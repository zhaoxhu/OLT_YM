/****************************************************************************
*
*     Copyright (c) 2005 Mellon Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Mellon Corporation
*
*     All information contained in this document is Mellon Corporation
*     company private, proprietary, and trade secret.
*
*	  modified wutw at 18 October
*		增加对PON节点的范围限制,有效PON 卡号为4-7,其中4如果为主控卡,则同样无效
*	  modified wutw at 19 October
*		增加show run 中add onu <1-64> <H.H.H>的命令
*		针对增加板卡是否在位的判断语句,判断port 是否为工作状态
*		增加showrun代码,补充add onu 命令
*	  modified wutw at 20 October
*		修改no命令为undo
*	  modified by wutw at 1 November
*		将debug节点下的downlink policer等命令移至pon节点，并在show run中增加相应的代码
*	  modified by wutw at 8 November
*	 	将showrun某处onu的错误数量6改为64.此处代码原为调试代码,之后没能检查出来,需以后
*		使用对比软件进行对比
*	  modified by wutw at 10 November
*		增加代码,一支持onu列表的输入,在update onu file <onuIdList> 参数onuIdList
*		输入可为:1,3,5,64-60,10等增加代码显示onu下载文件结果与进度;
*	  modified by wutw at 12 November
*		修改max-mac <1-128> logical-link <1-64>与max-mac <1-128> logical-link all命令,
*		将number参数改为1-128;
*		增加show max－mac logiclink <1-64>命令.并修改显示max-mac数量,使得显示正常.
*		分别增加历史统计设置15分钟与24小时的接口(原来的命令为同时使能15分钟与24小时),
*		与mib同步,并在show run添加相应的代码;
*		在pon_cli.c节点下,update onu file [<1-64>|all] 不变,使用onuIdList
*		由于隋平礼所提供的函数有误,需要调试,目前暂不提供;
*	  modified by wutw 2006/11/21
*		增加以下命令:
*		onu software auto-update <onuid_list>
*		undo onu software auto-update <onuid_list>
*		show onu software auto-update {[enable|disable]}*1(没有在showrun中增加代码)
*		修改undo encrypt命令下的调用函数为SetPonPortEncrypt()
*		注释max-mac， mac-learning，aps 相关命令，并在show run中注释掉相关
*		代码
*		增加show onu-version <onuid_list>
*		增加show onu-log <1-64>
*		增加upload onu-log <H.H.H.H> <user> <pass> <onuid_list>
*	  modified by wutw 2006/11/23
*		pclint检查，修改部分代码
*		增加 show onu-version <onuid_list>
*		增加onu software auto-update <onuIdx list>与
*		undo onu software auto-update <onuIdx list>
*		增加show onu software auto-update [enabled|disable]
*	  modified by wutw 2006/11/29
*		修改<onuli_list>只能输入1－63，而不能输入1－64的问题
*		补充增加有<onuid_list>参数的命令，对输入单个onu进行判断
*	  modified wutw 2006/12/05
*		增加查看处于环回状态的onu信息
*	  modified wutw 2006/12/15
*		增加p2p命令，设置,删除,显示命令
*	  modified wutw 2006/12/28
*		修改onu p2p <onuid_list> <onuid_list>与undo onu p2p <onuid_list> <onuid_list>
*		代码中,两个onuid_list之间多重执行p2p设置的问题
*		同时在show run的相关代码中修改,也避免多重操作设置p2p操作.
*	  modified wutw 2007/1/8
*		修改range [20km|40km]与show range命令的相关说明与帮助说明
*	  modified wutw 2007/1/17
*		增加命令show statistic [hostmsg|pon|cni],用于显示pon端口，hostmsg消息，cni端口
*		的数据统计
*	  modified by wutw 2007/01/23
*		增加olt pon端口,cni端口的统计命令
*      modified by chenfj 2007/04/26
*         问题单#4298 修改命令show onu-version <onu-list>，没有ONU参数时，显示当前PON端口下所有所有ONU版本
*      modified by chenfj  2007/04/26 
*         问题单#4299: P2P第二个命令有待改进,将广播包转发功能隐含到配置p2p链接的命令中去， 而未知单播包的转发则可以另外配置
*
*      added by chenfj 2007-5-23
*        增加 ONU 能支持的最大MAC 数设置/显示/恢复
*      added by chenfj  2007-6-1
*        增加ONU 上行数据包原MAC过滤
*     added by chenfj 2007-6-8 
*        增加ONU 数据流IP/PORT过滤
*    added by chenfj 2007-6-12 
*       增加ONU 数据流vlan id 过滤
*    added by chenfj 2007-6-15 
*       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤
*    added by chenfj 2007-9-24
*		问题单#5396:
*		在进入已设置了保护切换，且当前状态为passive 的PON节点或
*		ONU节点时增加提示信息，对用户配置passive端口时作出限制提示
*	modified by chenfj 2007-9-25
*		ONU加密，密钥更新时间在show run中保存时保存了错误的
*		字符串"encrypt-keytime"（不是配置更新密钥时间的命令），
*		导致配置数据未能恢复
*   modified by chenfj 2008-7-9
*         增加GFA6100 产品支持; 
*****************************************************************************/
#include "syscfg.h"


#ifdef	__cplusplus
extern "C"
{
#endif


#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "vos/vospubh/vos_ctype.h"
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
#include "mn_oam.h"

#include "ifm/ifm_debug.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"

/*added by wutw at 11 september*/
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"

/*added by wutw 2006/11/10*/
#include "pon_cli.h"
#include "file_oam/TransFile.h"
/*#include "statistics/statistics.h"*/
/*#include "../monitor/monitor.h"*/

#include "lib_gwEponOnuMib.h"
#include "gwEponSys.h"

#include "E1DataSave.h"

#include "onu/onuconfmgt.h"

#define	CLI_EPON_VOS_RESERVE		1
#define	CLI_EPON_STATS_PONOBJ_NULL_ERR		(-9006)
#define	CLI_EPON_CARDINSERT		CARDINSERT
#define	CLI_EPON_CARDNOTINSERT	CARDNOTINSERT
#define  	CLI_EPON_ONULIMIT_ENABLE     V2R1_ENABLE
#define  	CLI_EPON_ONULIMIT_DISABLE		V2R1_DISABLE


#define	EPON_SBA	1
#define	EPON_DBA	2
#define	CLI_EPON_UPLINK 		1
#define	CLI_EPON_DOWNLINK	      2
#define	CLI_EPON_SINGLELLID	1
#define	CLI_EPON_ALLLLID		2
#define	CLI_EPON_DISABLE		1
#define	CLI_EPON_AUTO			2
#define	CLI_EPON_FORCE		3
#define	CLI_EPON_PONDEV		1
#define	CLI_EPON_ONUDEV		2
#define	CLI_EPON_STATS15MIN	1
#define	CLI_EPON_STATS24HOUR	2
#define CLI_EPON_HOURS_ONE_DAY  24
#define	CLI_EPON_ONUMIN		0
#define	CLI_EPON_ONUMAX		(MAXONUPERPON-1) /* 63*/
#define	CLI_EPON_PONMIN		0
#define	CLI_EPON_PONMAX		(MAXPON-1) /*19*/
#define	CLI_EPON_DELALL		1
#define	CLI_EPON_DELONE		2
#define	CLI_EPON_DEFAULTLLID	0
#define	CLI_EPON_ONUUP		ONU_OPER_STATUS_UP /*1*/
#define	CLI_EPON_ONUDOWN	      ONU_OPER_STATUS_DOWN /*2*/
#define CLI_EPON_P2P_ENABLE		V2R1_ENABLE
#define CLI_EPON_P2P_DISABLE		V2R1_DISABLE
#define  CLI_EPON_ENCRYPT_NONE     PON_ENCRYPTION_PURE /*1*/
#define  CLI_EPON_ENCRYPT_DOWN    PON_ENCRYPTION_DIRECTION_DOWN /*2*/
#define  CLI_EPON_ENCRYPT_ALL       PON_ENCRYPTION_DIRECTION_ALL  /*3*/

#define  CLI_EPON_LEARNING_ENABLE  1
#define  CLI_EPON_LEARNING_DISABLE 2 
#define  CLI_EPON_BER			1
#define  CLI_EPON_FER			2
#define  CLI_EPON_ALARM_ENABLE		1
#define  CLI_EPON_ALARM_DISABLE	2
#define  CLI_EPON_MONITOR_ENABLE		1
#define  CLI_EPON_MONITOR_DISABLE	2
#define  CLI_EPON_BANDWIDTH_MIN		64
#define  CLI_EPON_DOWN_MAX_BANDWIDTH	500000

#define	CLI_EPON_PON_EN		1
#define	CLI_EPON_PON_DIS		2

extern unsigned int MaxMACDefault;

extern STATUS	isHaveAuthMacAddress( ULONG slot, ULONG pon, const char* mac,  ULONG *onu );
extern STATUS	getOnuAuthEnable(ULONG slot, ULONG port, ULONG *enable );
extern STATUS	setOnuAuthEnable(ULONG slot, ULONG port, ULONG	enable );
extern STATUS 	setOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf );
extern STATUS 	getOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu, ULONG *st );
extern STATUS 	setOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu,  ULONG st );
extern STATUS CliRealTimeStatsOnuUpLinkBer( short int ponId, short int onuId, struct vty* vty );
extern STATUS CliRealTimeOnuUpLinkFer( short int ponId, short int onuId, struct vty* vty );
extern bool Olt_exists ( const short int olt_id );
extern STATUS getNextOnuAuthEntry( ULONG brd, ULONG port, ULONG usr, ULONG *nextbrd, ULONG *nextport, ULONG* nextusr );
extern STATUS getOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf, ULONG *len );
extern int CTC_GetLlidFecCapability( short int PonPortIdx, short int OnuIdx,  int *value );
extern LONG  lCli_SendbyOam( INT16 ponID, INT16 onuID,	UCHAR  *pClibuf,	USHORT length,cliPayload *stSessionIdPayload ,	struct vty * vty);
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);

extern STATUS CliRealTimeOltDownlinkBer( short int ponId, short int onuId, struct vty* vty  );
extern  int  ClearOltPonPortStatisticCounter(short int PonPortIdx );
extern int onu_auth_loidlist_showrun(struct vty * vty, ULONG brdIdx, ULONG portIdx, int flag);
extern int search_onu_vlan(short int PonPortIdx, short int OnuIdx, short int vid);
extern int get_onu_allvlanstr(short int PonPortIdx, short int OnuIdx, UCHAR *vlanstr, int *str_len);
extern int GetPonQinQRuleOnuList(short int PonPortIdx, short int * OnuListNum, unsigned char * OnuList);
extern int g_SystemLoadConfComplete;
unsigned char *v2r1EthType[] = 
{
	"0x8100",
	"0x9100",
	"0x88a8",
	"configable"
};

char *fileTranErrStr[]= {
	"NULL",
	"  % Error! System busy\r\n",
	"  % Error! System resource is not enough.\r\n",
	"  % Error! Processing err.\r\n",
	"  % Error! Flowing err.\r\n",
	"  % Error! Not exist file.\r\n",/*5*/
	"  % Error! File is too long.\r\n",/*6*/
	"  % Error! File is too short\r\n",/*7*/
	"  % Error! Length or offset err.\r\n",/*8*/
	"  % Error! FCS err.\r\n",/*9*/
	"  % Error! Save err.\r\n",/*a*/
	"  % Error! Memory is not enough.\r\n",/*b*/
	"  % Error! Semaphores are not enough.\r\n",/*c*/
	"  % Error! Sent msg failed.\r\n",/*d*/
	"  % Error! Time out.\r\n",/*e*/
	"  % Error! Sent file time out.\r\n",/*f*/
	"  % Error! Reveice file time out.\r\n",/*0x11*/
	"  % Error! No Ack.\r\n",/*0x12*/
	"  % Error! Requestint reject.\r\n",/*0x13*/
	"  % Error! No end Ack.\r\n",/*0x14*/
	"  % Error! End Ack err.\r\n",/*0x15*/
	"  % Error! Disable start backet.\r\n",/*0x16*/
	"  % Error! Transmitting status of the onuId.\r\n",/*0x17*/
	"  % Error! Reading file failed.\r\n",/*0x18*/
	};


static char cliUserIp[64];
static char cliUserName[64];
static char cliUserPassWord[64];
static struct vty *file_vty = NULL;

/* Alarm types */
/* not used, commented by chenfj 2008-7-9
typedef enum
{  
	CLI_PON_ALARM_BER,
	CLI_PON_ALARM_FER,
	CLI_PON_ALARM_SOFTWARE_ERROR,
	CLI_PON_ALARM_LOCAL_LINK_FAULT,	
	CLI_PON_ALARM_DYING_GASP,	
	CLI_PON_ALARM_CRITICAL_EVENT,
	CLI_PON_ALARM_REMOTE_STABLE,
	CLI_PON_ALARM_LOCAL_STABLE,
	CLI_PON_ALARM_OAM_VENDOR_SPECIFIC,
	CLI_PON_ALARM_ERRORED_SYMBOL_PERIOD,
	CLI_PON_ALARM_ERRORED_FRAME,
	CLI_PON_ALARM_ERRORED_FRAME_PERIOD,
	CLI_PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY,
	CLI_PON_ALARM_ONU_REGISTRATION_ERROR,
	CLI_PON_ALARM_OAM_LINK_DISCONNECTION,
	CLI_PON_ALARM_BAD_ENCRYPTION_KEY,
	CLI_PON_ALARM_LLID_MISMATCH,
	CLI_PON_ALARM_TOO_MANY_ONU_REGISTERING,
	CLI_PON_ALARM_LAST_ALARM
} CLI_PON_alarm_t;
*/

#define CLI_SLOT_SYSCLE    for(slotId = PONCARD_FIRST;slotId <= PONCARD_LAST; slotId ++)
#define CLI_PORT_SYSCLE	for(port = 1; port <= PONPORTPERCARD; port ++)
#define CLI_ONU_SYSCLE	for(onuId = 0; onuId <= CLI_EPON_ONUMAX; onuId++)
/*#undef	CLI_EPON_DEBUG*/

/*extern int  SetOnuEncryptKeyExchagetime(short int PonPortIdx, short int OnuIdx, unsigned int time);*/
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
extern short int GetPonPortIdxBySlot( short int slot, short  int port );
extern int GetOltCardslotInserted( int CardIndex);
extern int  GetOnuOperStatus( short int PonPortIdx, short int OnuIdx );
static INT16 cliPonIdCheck(ULONG ulIfIndex);
/*extern int HisStatsPonStatsStart (short int ponId, BOOL Done);*/
extern long send_by_ftp_Api( CHAR *ModeType, CHAR *ipaddress, CHAR *username, 
									CHAR *password,CHAR *filename, CHAR *filebuffer, 
									CHAR *filelen);
extern ULONG IFM_PON_CREATE_INDEX( ULONG ulSlot, ULONG ulPort , ULONG ulOnuId, ULONG ulOnuFeId);
extern ULONG ulIfindex_2_userSlot_userPort(ULONG ulIfIndex, ULONG *ulUserslot, ULONG *ulUserport);
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern int ShowPonMacLearningByVty( short int PonPortIdx, short int SingleOnuIdx, struct vty *vty );
extern int DeleteOnuMacAddrAll( short int PonPortIdx, short int OnuIdx, short int LlidIdx);
extern int DeleteOnuMacAddr( short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned char *MacAddr );
extern int HisStatsPonStatsStart (short int ponId, BOOL Done);
extern int HisStatsOnuStatsStart  (short int ponId, short int onuId, BOOL Done);
extern STATUS HisStats15MinMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue);
extern STATUS HisStats15MinMaxRecordGet(unsigned int *pValue);
extern STATUS HisStatsDefaultRecordGet(unsigned int *pDefBucket_15M, unsigned int *pDefBucket_24H); 
extern STATUS HisStatsPon15MModified (short int ponId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsPon24HModified (short int ponId, unsigned int bucketNum,  BOOL flag24H);
extern STATUS HisStatsOnu15MModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsOnu24HModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag24H);
extern STATUS CliHisStatsOnu15MinDataVty(unsigned short ponId,  unsigned short onuId, unsigned int bucket_num, struct vty* vty);
extern STATUS CliHisStatsOnu24HourDataVty(unsigned short ponId, unsigned short onuId, unsigned int bucket_num, struct vty* vty);
extern STATUS CliHisStats15MinDataVty(unsigned short ponId, unsigned int bucket_num, struct vty* vty);
extern STATUS CliHisStats24HourDataVty(unsigned short ponId, unsigned int bucket_num, struct vty* vty);
extern STATUS HisStatsPonRawClear(short int ponId);
extern STATUS HisStatsPon15MinRawClear(short int ponId);
extern STATUS HisStatsPon24HourRawClear(short int ponId);
extern STATUS HisStatsOnu15MinRawClear(short int ponId, short int onuId);
extern STATUS HisStatsOnu24HourRawClear(short int ponId, short int onuId);
extern STATUS CliHisStatsPonCtrlGet( short int ponId, struct vty* vty ) ;
extern STATUS CliHisStatsPonStatusGet(short int ponId,unsigned int *pStatus15m,unsigned int *pStatus24h) ;
extern char *OnuTypeToString( int type );
extern short int monStatusSet(short int  status);
extern short int monStatusGet(short int  *pStatus);
extern short int monPonBerAlmEnSet(unsigned short oltId, unsigned int berAlmEn);
extern short int monPonBerAlmEnGet(unsigned short oltId, unsigned int *pBerAlmEn);
extern short int monPonFerAlmEnSet(unsigned short oltId, unsigned int ferAlmEn);
extern short int monOnuFerAlmEnGet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int *pFerAlmEn);
extern short int monPonFerAlmEnGet(unsigned short oltId, unsigned int *pFerAlmEn);
extern int SetOnuPeerToPeer(short int PonPortIdx, short int OnuIdx1, short int OnuIdx2);
extern int DiscOnuPeerToPeer(short int PonPortIdx, short int OnuIdx1, short int OnuIdx2);
extern int ShowOnuPeerToPeerByVty( short int PonPortIdx, short int OnuIdx , struct vty *vty);
extern int ShowOnuPeerToPeerByVty_1( short int PonPortIdx, short int OnuIdx , struct vty *vty);
/*extern int GetOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1,short int OnuIdx2 );*/
extern int SetPonRange(short int PonPortIdx , int range);
extern int GetPonRange( short int PonPortIdx, int *range );
extern int  GetPonRangeDefault( int *rangeDefault );
extern int  ShowOnuBandwidthByVty_1(short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern STATUS CliRealTimeStatsPon( short int ponId, struct vty* vty );
extern STATUS CliRealTimeStatsCNI( short int ponId ,struct vty* vty);
extern STATUS CliRealTimeStatsPonForGpon( short int ponId, struct vty* vty );
extern STATUS CliRealTimeStatsCNIForGpon( short int ponId ,struct vty* vty);
extern STATUS CliRealTimeStatsGemForGpon( short int ponId , short int gemId, struct vty* vty);

extern STATUS CliRealTimeStatsHostMsg( short int ponId, struct vty* vty );
extern int ShowPonPortOffLineOnuByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );
extern VOID show_onu_version_banner(struct vty * vty);

ULONG * V2R1_Parse_OnuId_List( CHAR * pcOnuId_List );
enum match_type V2R1_Check_OnuId_List( char * onuid_list ); 
int CliOnuLogFileTranResult(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode);
int CliOnuLogFileTranData(unsigned short usPonID, unsigned short usOnuID, 
                                char *pFileName, char *pFileBuf, long lFileLen);

int CliOnuLogFileTUploadResult(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                      					char *pFileName, void *pArgVoid, long lFileLen, 
                      					long lSucLen, long lErrCode);

int CliOnuLogFileUpload(unsigned short usPonID, unsigned short usOnuID, 
                                char *pFileName, char *pFileBuf, long lFileLen);

/*added by liyang @2015-03-30 for pon statistics*/
extern STATUS CliRealTimeStatsPonDatapath(short int ponId, struct vty * vty);
extern STATUS CliRealTimeStatsCniDatapath(short int ponId, struct vty * vty);
extern STATUS  CliRealTimeStatsUplink(short int ponId, struct vty * vty);
extern STATUS  CliRealTimeStatsDownlink(short int ponId, struct vty * vty);
extern STATUS CliRealTimeStatsUplinkForGpon( short int ponId, struct vty* vty );
extern STATUS CliRealTimeStatsDownlinkForGpon( short int ponId, struct vty* vty );
extern STATUS CliRealTimeStatsPonDatapathForGpon( short int ponId, struct vty* vty );
int cliCheckOnuMacValid( short int PonPortIdx, short int OnuIdx )
{
	UCHAR *pMac;
	int ret = VOS_OK;
	ONU_MGMT_SEM_TAKE;
	/*if( (CompTwoMacAddress( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr , Invalid_Mac_Addr ) == ROK) ||
		(CompTwoMacAddress( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr , Invalid_Mac_Addr1 ) == ROK) ) 
		ret = V2R1_ONU_NOT_EXIST;*/
	pMac = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr;
	if( MAC_ADDR_IS_INVALID(pMac) )
		ret = V2R1_ONU_NOT_EXIST;

	ONU_MGMT_SEM_GIVE;
	return ret;
}

DEFUN  (
    into_epon_pon_node,
    into_epon_pon_node_cmd,
    "pon <slot/port>",
    "Select a pon to config\n"
    "Specify pon interface's slot and port\n")
{
    CHAR    ifName[IFM_NAME_SIZE + 1];
    ULONG   ulIFIndex = 0;
    CHAR    prompt[64] = { 0 };
    ULONG  	ulSlot = 0, ulPort = 0 ;
	short int phyPonId = 0;

    
    /*modified by wutw at 11 september*/
    IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
	{
		vty_out( vty, "  %% pon %s is not exist\r\n", argv[0] );	/* 问题单10955 */
		return(CMD_WARNING );
	}
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
	{
		vty_out( vty, "  %% slot %d is not PON board\r\n", ulSlot );	/* 问题单10955 */
		return(CMD_WARNING);
	}
    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	/*modified by wangjiah@2017-05-11 to automatically switch from passive pon port to active pon port*/
	if(( PonPortSwapEnableQuery( phyPonId ) == V2R1_PON_PORT_SWAP_ENABLE ) &&
		( PonPortHotStatusQuery( phyPonId ) == V2R1_PON_PORT_SWAP_PASSIVE ) &&
		( PON_SWAPMODE_ISOLT( GetPonPortHotSwapMode( phyPonId ))))/*for onu swap by jinhl@2013-02-22*/
	{
		int ret;
		unsigned int PartnerSlot, PartnerPort;
		ret = PonPortAutoProtectPortQuery(ulSlot, ulPort, &PartnerSlot, &PartnerPort );
		if(!OLT_SLOT_ISVALID(PartnerSlot))
		{
			vty_out(vty,"\r\nNOTE:Can't goto pon%d/%d. Because it's PASSIVE and ACTIVE pon%d/%d is a logical slot.\r\n", ulSlot, ulPort, PartnerSlot, PartnerPort );
			return CMD_WARNING;
		}
		if( ret == ROK )
			vty_out(vty,"\r\nNOTE:pon%d/%d is PASSIVE now. It's automatically switched into ACTIVE pon%d/%d CLI node\r\n", ulSlot, ulPort, PartnerSlot, PartnerPort );
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", PartnerSlot, PartnerPort);
		ulSlot = PartnerSlot;
		ulPort = PartnerPort;
	}
	else
	{
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", "pon", argv[0] );
	}

    ulIFIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, 0, 0);
    if ( ulIFIndex == 0 )
    {
        vty_out( vty, "  %% Can not find interface %s.\r\n", ifName );
        return CMD_WARNING;
    }	
    
    vty->node = PON_PORT_NODE;
    vty->index = ( VOID * ) ulIFIndex;
    
    if(SYS_MODULE_IS_GPON(ulSlot))  
    {
        VOS_StrCpy( prompt, "%s(gpon-" );
        VOS_StrCat( prompt, ifName );
        VOS_StrCat( prompt, ")#" );
        vty_set_prompt( vty, prompt );
    }
    else
    {
        VOS_StrCpy( prompt, "%s(epon-" );
        VOS_StrCat( prompt, ifName );
        VOS_StrCat( prompt, ")#" );
        vty_set_prompt( vty, prompt );
	}
	
    return CMD_SUCCESS;
}

/*
5.9.9	ONU下行带宽是否使能policer(此参数需show run 保存)
      接口函数： int SetOnuPolicerFlag( int flag)
                
初始值获取函数：int GetOnuPolicerFlagDefault()
      当前值获取函数：int GetOnuPolicerFlag ()
*/
#ifdef _V2R1_ONU_DOWNLINK_POLICER_
#endif
DEFUN( bandwidth_onu_on_config,
        bandwidth_onu_on_cmd,
        "onu downlink-policer",
        "Config the onu software\n"
        "Config onu downlink-policer enable\n"
        )
{
    int iRet;
	int enFlag = V2R1_ENABLE;

    if ( PON_PORT_NODE == vty->node )
    {
    	unsigned long ulSlot, ulPort, ulOnuId;
        short int iOltID;
        
    	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &iOltID) != VOS_OK )
    		return CMD_WARNING;

    	if( PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK )
    		return(CMD_WARNING);

        iRet = SetOltPolicerFlag( iOltID, enFlag );
    }
    else
    {
        iRet = SetOnuPolicerFlag( enFlag );
    }
	if (0 != iRet)
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		

	return CMD_SUCCESS;	
}


DEFUN( bandwidth_onu_of_config,
        bandwidth_onu_off_cmd,
        "undo onu downlink-policer",
        NO_STR
        "Config the onu software\n"
        "Config onu downlink-policer disable\n"
        )
{
    int iRet;
	int enFlag = V2R1_DISABLE;

    if ( PON_PORT_NODE == vty->node )
    {
    	unsigned long ulSlot, ulPort, ulOnuId;
        short int iOltID;
        
    	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &iOltID) != VOS_OK )
    		return CMD_WARNING;

    	if( PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK )
    		return(CMD_WARNING);

        iRet = SetOltPolicerFlag( iOltID, enFlag );
    }
    else
    {
        iRet = SetOnuPolicerFlag( enFlag );
    }
	if (0 != iRet)
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		

	return CMD_SUCCESS;	
}

DEFUN( bandwidth_onu_off_show,
        bandwidth_onu_off_show_cmd,
        "show onu downlink-policer",
        DescStringCommonShow
        "Show the onu software\n"
        "Show onu downlink-policer status\n"
        )
{
	int debugFlag = (-1);

    if ( PON_PORT_NODE == vty->node )
    {
    	unsigned long ulSlot, ulPort, ulOnuId;
        short int iOltID;
        
    	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &iOltID) != VOS_OK )
    		return CMD_WARNING;

    	if( PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK )
    		return(CMD_WARNING);

        debugFlag = GetOltPolicerFlag(iOltID);
    }
    else
    {
        debugFlag = GetOnuPolicerFlag();
    }
	if ((-1) == debugFlag)
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }	
	if (debugFlag == V2R1_ENABLE)
		vty_out( vty ,"  Onu downlink-policer enable\r\n");
	else if (debugFlag == V2R1_DISABLE)
		vty_out( vty ,"  Onu downlink-policer disable \r\n");
	else
    {
	    vty_out( vty, "  %% Unknowed status of onu downlink-policer.\r\n" );
	    return CMD_WARNING;    	
    }
	
	return CMD_SUCCESS;	
}

/* 支持予配置,不判断PON 板及芯片是否在位*/
/* 增加onu */
DEFUN  (
    pon_add_onu_config,
    pon_add_onu_config_cmd,
	"add onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <H.H.H> {<devicename>}*1",
    "add onu to the pon port\n"
    "add onu\n"
    "please input onu index\n"
    "please input onu mac address\n"
    "please input onu name\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    ULONG ulIfIndex = 0;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
    CHECK_CMD_ONU_RANGE(vty, userOnuId);
    
	if(userOnuId >= (GetMaxOnuByPonPort(phyPonId)&0xff))
	{
		vty_out(vty, " This port only allowed %d onu to register!\r\n", GetMaxOnuByPonPort(phyPonId)&0xff);
		return CMD_WARNING;
	}
	/* 不论是否给onu为注册的onu，都允许增加*/    	
#if 0
    lRet = cliOnuIdCheck(phyPonId, userOnuId);
    if (lRet != VOS_OK)
	{
	#ifdef CLI_EPON_DEBUG
	   vty_out( vty, "  %% phyPonid %d userOnuId %d\r\n",phyPonId, userOnuId);
	#endif
	   vty_out( vty, "  %% Parameter is error.\r\n");
    
	   return CMD_WARNING;
	}
#endif

    if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }
	
	if(GetOnuOperStatus_1(phyPonId, userOnuId) != ONU_OPER_STATUS_DOWN)
	{
        vty_out( vty, "  %% Onu%d/%d/%d has been online.\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), userOnuId+1);
        return CMD_WARNING;
	}
    lRet = AddOnuToPonPort(phyPonId, userOnuId, MacAddr );
    switch (lRet)
    {
        case 2:
            vty_out( vty, "  %% the newer Onu mac address is existed in other pon port.\r\n");
            return CMD_WARNING;
        case 1:
            vty_out( vty, "  %% valid Mac address has existed in this OnuIdx \r\n");
            return CMD_WARNING;
        case 3:
            vty_out(vty, "  %% the newer Onu Mac address is existed in other Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        case 4:
            vty_out(vty, "  %% the Onu Mac address is existed in this Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        default:
            if (lRet != VOS_OK)
            {
#ifdef CLI_EPON_DEBUG
                vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d.\r\n",lRet, phyPonId, userOnuId);
#endif     
                vty_out( vty, "  %% Executing error.\r\n");

                return CMD_WARNING;
            }
    }
    /*begin: added by liub 2017-05-24*/
	if(argc == 3)
	{
		UCHAR len;
		len = strlen(argv[2]);	
		if( len > MAXDEVICENAMELEN )
    	{
       		vty_out( vty, "  %% Err:name is too long.\r\n" );
	   		return CMD_WARNING;
		}
		OnuMgt_SetOnuDeviceName( phyPonId, userOnuId,argv[2],len);
	}
	/*end*/

    
    return CMD_SUCCESS;
}
DEFUN  (
    olt_add_onu_config,
    olt_add_onu_config_cmd,
		"add onu <slot/port> <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <H.H.H> {<devicename>}*1",
    "add onu to the pon port\n"
    "add onu\n"
	"please input pon port slot/port\n"  
    "please input onu index\n"
    "please input onu mac address\n"
    "please input onu name\n"
    )
{
    LONG lRet = VOS_OK;
	short int ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );	
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );    
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	ulOnuId = (short int)(VOS_AtoL( argv[1] ) - 1);
    CHECK_CMD_ONU_RANGE(vty, ulOnuId);
	
	if(ulOnuId >= (GetMaxOnuByPonPort(phyPonId)&0xff))
	{
		vty_out(vty, " This port only allowed %d onu to register!\r\n", GetMaxOnuByPonPort(phyPonId)&0xff);
		return CMD_WARNING;
	}

    if ( GetMacAddr( ( CHAR* ) argv[ 2 ], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }
	
	if(GetOnuOperStatus_1(phyPonId, ulOnuId) != ONU_OPER_STATUS_DOWN)
	{
        vty_out( vty, "  %% Onu%d/%d/%d has been online.\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), ulOnuId+1);
        return CMD_WARNING;
	}
		
    lRet = AddOnuToPonPort(phyPonId, ulOnuId, MacAddr );
    switch (lRet)
    {
        case 2:
            vty_out( vty, "  %% the newer Onu mac address is existed in other pon port.\r\n");
            return CMD_WARNING;
        case 1:
            vty_out( vty, "  %% valid Mac address has existed in this OnuIdx \r\n");
            return CMD_WARNING;
        case 3:
            vty_out(vty, "  %% the newer Onu Mac address is existed in other Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        case 4:
            vty_out(vty, "  %% the Onu Mac address is existed in this Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        default:
            if (lRet != VOS_OK)
            {
#ifdef CLI_EPON_DEBUG
                vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d.\r\n",lRet, phyPonId, ulOnuId);
#endif     
                vty_out( vty, "  %% Executing error.\r\n");

                return CMD_WARNING;
            }
    } 
	/*begin: added by liub 2017-05-24*/
	if(argc == 4)
	{
		UCHAR len;
		len = strlen(argv[3]);	
		if( len > MAXDEVICENAMELEN )
    		{
       		vty_out( vty, "  %% Err:name is too long.\r\n" );
	   		return CMD_WARNING;
		}
		OnuMgt_SetOnuDeviceName( phyPonId, ulOnuId,argv[3],len);
	}  
	/*end*/

    return CMD_SUCCESS;
}

DEFUN  (
    pon_add_gonu_config,
    pon_add_gonu_config_cmd,
	"add gonu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <sn> {<devicename>}*1",
    "add onu to the pon port\n"
    "add onu\n"
    "please input onu index\n"
    "please input onu serial number(16 Bytes)\n"
    "please input onu device name(1-128 Bytes)\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    ULONG ulIfIndex = 0;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
    CHECK_CMD_ONU_RANGE(vty, userOnuId);
    
	if(userOnuId >= (GetMaxOnuByPonPort(phyPonId)&0xff))
	{
		vty_out(vty, " This port only allowed %d onu to register!\r\n", GetMaxOnuByPonPort(phyPonId)&0xff);
		return CMD_WARNING;
	}

    if(VOS_StrLen(argv[1]) != 16)
    {
        vty_out(vty, "The serial number of GPON ONU must be 16 bytes!\r\n");
        return CMD_WARNING;
    }
	if(GetOnuOperStatus_1(phyPonId, userOnuId) != ONU_OPER_STATUS_DOWN)
	{
        vty_out( vty, "  %% Onu%d/%d/%d has been online.\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), userOnuId+1);
        return CMD_WARNING;
	}
   	AddGponOnuToPonPort(phyPonId, userOnuId, argv[1]);
	if(argc == 3)
	{
		UCHAR len;
		len = strlen(argv[2]);	
		if( len > MAXDEVICENAMELEN )
    	{
       		vty_out( vty, "  %% Err:name is too long.\r\n" );
	   		return CMD_WARNING;
		}
		OnuMgt_SetOnuDeviceName( phyPonId, userOnuId,argv[2],len);
	}
        
    
    return CMD_SUCCESS;
}

DEFUN (
	olt_add_gonu_config,
	olt_add_gonu_config_cmd,
	"add gonu <slot/port> <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <sn> {<devicename>}*1",
    "add onu to the pon port\n"
    "add gpon onu\n"
	"please input pon port slot/port\n"  
    "please input onu index\n"
    "please input onu serial number(16 Bytes)\n"
    "please input onu device name(1-128 Bytes)\n"    
    )
{
    LONG lRet = VOS_OK;
	short int ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );	
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );    
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	ulOnuId = (short int)(VOS_AtoL( argv[1] ) - 1);
    CHECK_CMD_ONU_RANGE(vty, ulOnuId);
	
	if(ulOnuId >= (GetMaxOnuByPonPort(phyPonId)&0xff))
	{
		vty_out(vty, " This port only allowed %d onu to register!\r\n", GetMaxOnuByPonPort(phyPonId)&0xff);
		return CMD_WARNING;
	}

    if(VOS_StrLen(argv[2]) != 16)
    {
        vty_out(vty, "The serial number of GPON ONU must be 16 bytes!\r\n");
        return CMD_WARNING;
    }

	if(GetOnuOperStatus_1(phyPonId, ulOnuId) != ONU_OPER_STATUS_DOWN)
	{
        vty_out( vty, "  %% Onu%d/%d/%d has been online.\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId), ulOnuId+1);
        return CMD_WARNING;
	}
		
    lRet = AddGponOnuToPonPort(phyPonId, ulOnuId, argv[2] );
    switch (lRet)
    {
        case 2:
            vty_out( vty, "  %% the newer Onu mac address is existed in other pon port.\r\n");
            return CMD_WARNING;
        case 1:
            vty_out( vty, "  %% valid Mac address has existed in this OnuIdx \r\n");
            return CMD_WARNING;
        case 3:
            vty_out(vty, "  %% the newer Onu Mac address is existed in other Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        case 4:
            vty_out(vty, "  %% the Onu Mac address is existed in this Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        default:
            if (lRet != VOS_OK)
            {
#ifdef CLI_EPON_DEBUG
                vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d.\r\n",lRet, phyPonId, ulOnuId);
#endif     
                vty_out( vty, "  %% Executing error.\r\n");

                return CMD_WARNING;
            }
    }    
	if(argc == 4)
	{
		UCHAR len;
		len = strlen(argv[3]);	
		if( len > MAXDEVICENAMELEN )
    	{
       		vty_out( vty, "  %% Err:name is too long.\r\n" );
	   		return CMD_WARNING;
		}
		OnuMgt_SetOnuDeviceName( phyPonId, ulOnuId,argv[3],len);
	}
    return CMD_SUCCESS;
}


DEFUN  (
    pon_modi_onu_config,
    pon_modi_onu_config_cmd,
	"modify onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <H.H.H>",
    "modify onu to the pon port\n"
    "modify onu\n"
    "please input onu index\n"
    "please input onu mac address\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    ULONG ulIfIndex = 0;
    int status = ONU_OPER_STATUS_DOWN;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
    CHECK_CMD_ONU_RANGE(vty, userOnuId);

	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

    status = GetOnuOperStatus(phyPonId, userOnuId);
    if(status != ONU_OPER_STATUS_DOWN)
    {
	    vty_out( vty, "  %% Do not support modified online onu.\r\n" );
	    return CMD_WARNING;    	
    }
    
    lRet = ModifyOnuToPonPort(phyPonId, userOnuId, MacAddr );
    switch (lRet)
    {
        case 2:
            vty_out( vty, "  %% the newer Onu mac address is existed in other pon port.\r\n");
            return CMD_WARNING;
        case 1:
            vty_out( vty, "  %% valid Mac address has existed in this OnuIdx \r\n");
            return CMD_WARNING;
        case 3:
            vty_out(vty, "  %% the newer Onu Mac address is existed in other Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        default:
            if (lRet != VOS_OK)
            {
#ifdef CLI_EPON_DEBUG
                vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d.\r\n",lRet, phyPonId, userOnuId);
#endif     
                vty_out( vty, "  %% Executing error.\r\n");

                return CMD_WARNING;
            }
    }
        
    
    return CMD_SUCCESS;
}

DEFUN  (
    olt_modi_onu_config,
    olt_modi_onu_config_cmd,
		"modify onu <slot/port> <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"> <H.H.H>",
    "modify onu to the pon port\n"
    "modify onu\n"
	"please input pon port slot/port\n"  
    "please input onu index\n"
    "please input onu mac address\n"
    )
{
    LONG lRet = VOS_OK;
	short int ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
    unsigned char MacAddr[6] = {0,0,0,0,0,0};
    int status = ONU_OPER_STATUS_DOWN;
    
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );	
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );    
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	ulOnuId = (short int)(VOS_AtoL( argv[1] ) - 1);
    CHECK_CMD_ONU_RANGE(vty, ulOnuId);
	
	if ( GetMacAddr( ( CHAR* ) argv[ 2 ], MacAddr ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
    
    status = GetOnuOperStatus(phyPonId, ulOnuId);
    if(status != ONU_OPER_STATUS_DOWN)
    {
	    vty_out( vty, "  %% Do not support modified online onu.\r\n" );
	    return CMD_WARNING;    	
    }
    lRet = ModifyOnuToPonPort(phyPonId, ulOnuId, MacAddr );
    switch (lRet)
    {
        case 2:
            vty_out( vty, "  %% the newer Onu mac address is existed in other pon port.\r\n");
            return CMD_WARNING;
        case 1:
            vty_out( vty, "  %% valid Mac address has existed in this OnuIdx \r\n");
            return CMD_WARNING;
        case 3:
            vty_out(vty, "  %% the newer Onu Mac address is existed in other Onu entry of this pon port.\r\n");
            return CMD_WARNING;
        default:
            if (lRet != VOS_OK)
            {
#ifdef CLI_EPON_DEBUG
                vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d userOnuId %d.\r\n",lRet, phyPonId, ulOnuId);
#endif     
                vty_out( vty, "  %% Executing error.\r\n");

                return CMD_WARNING;
            }
    }            
    return CMD_SUCCESS;
}

int checkVtyPonIsValid( struct vty *vty, ULONG ulSlot, ULONG ulPort )
{
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return VOS_ERROR;
	}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return VOS_ERROR;
	}
	/* 3 PON 芯片在位检查*/
	if( getPonChipInserted( (ulSlot), (ulPort)) != PONCHIP_EXIST )
	{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
		return VOS_ERROR;
	}
	return VOS_OK;
}

int parse_pon_command_parameter( struct vty *vty, ULONG *pulSlot, ULONG *pulPort , ULONG *pulOnuId, INT16 *pi16PonId )
{
	if( (pulSlot == NULL) || (pulPort == NULL) || (pulOnuId == NULL) || (pi16PonId == NULL) )
		return VOS_ERROR;
	if( PON_GetSlotPortOnu((ULONG)(vty->index), pulSlot, pulPort, pulOnuId) == VOS_ERROR )
	{
		return VOS_ERROR;
	}
	*pi16PonId = GetPonPortIdxBySlot( (short int)(*pulSlot), (short  int)(*pulPort) );
	if ((*pi16PonId < CLI_EPON_PONMIN) || (*pi16PonId > CLI_EPON_PONMAX))
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return VOS_ERROR;    	
	}
	return VOS_OK;
}

DEFUN  (
    pon_del_offline_onu_config,
    pon_del_offline_onu_config_cmd,
    "delete offline-onu",
    "delete onu from the pon port\n"
    "delete offline-onu from the pon port\n"
    )
{

	INT16 phyPonId = 0;

	/*ULONG ulIfIndex = 0;*/
	ULONG ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    int OnuEntry = 0;
    int onu_status = 0;
    int onu_is_valid = 0;
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	
    for(ulOnuId=0; ulOnuId < MAXONUPERPON; ulOnuId++)
    {	
    	/*问题单12908，只针对有onu注册信息的onuid进行检查。2012-8-2 by luh*/
		if(ThisIsValidOnu(phyPonId, ulOnuId) != ROK )
            continue; 
        
        OnuEntry = phyPonId*MAXONUPERPON+ ulOnuId;
        			
        ONU_MGMT_SEM_TAKE;
        onu_status = OnuMgmtTable[OnuEntry].OperStatus;
        ONU_MGMT_SEM_GIVE;

        if((onu_status != ONU_OPER_STATUS_UP) && (onu_status != ONU_OPER_STATUS_PENDING) && (onu_status != ONU_OPER_STATUS_DORMANT) )
        {
            DelOnuFromPonPort( phyPonId, ulOnuId );
        }
    }

    return CMD_SUCCESS;
}

/* modified by xieshl 20110901, 增加删除ONU认证表选项，需求11832 */
/* 支持予配置,不判断PON 板及芯片是否在位*/
/*删除onu*/
DEFUN  (
    pon_del_onu_config,
    pon_del_onu_config_cmd,
    "delete onu <onuid_list> {[auth-mac]}*1",
    "delete onu from the pon port\n"
    "delete onu from the pon port\n"
    OnuIDStringDesc
    "delete the onu's authentication mac address at the same time\n"
    )
{
	LONG lRet = VOS_OK;
	INT16 phyPonId = 0;
	INT16 userOnuId = 0;
	/*ULONG ulIfIndex = 0;*/
	ULONG ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	int count = 0;
	int empty_count = 0;
	int error_count = 0;
	int slot = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	
		
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
 	{
 		count++;
        
 		userOnuId = (short int)(ulOnuId - 1);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		slot = get_gfa_tdm_slotno();
		if(slot)
		{
			if(SlotCardIsTdmSgBoard(slot) == ROK)
			{
				unsigned char tdm_slot, tdm_sg;
				unsigned short int logidOnuId;
				
				if(GetOnuBelongToSG( MAKEDEVID(ulSlot, ulPort, ulOnuId), &tdm_slot, &tdm_sg, &logidOnuId) == ROK )
				{
					vty_out(vty,"onu%d/%d/%d is config to sig%d/%d, Please delete voice-onu from sig first\r\n", ulSlot,ulPort,ulOnuId,tdm_slot,tdm_sg);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
			}
			else if(SlotCardIsTdmE1Board(slot) == ROK)
			{
				OnuE1Info  pOnuE1Info;
				VOS_MemSet(&pOnuE1Info, 0, sizeof(OnuE1Info));
				if((GetOnuE1Info(MAKEDEVID(ulSlot, ulPort, ulOnuId), &pOnuE1Info) == ROK)
					&& (pOnuE1Info.onuValidE1Count != 0 ))
				{
					vty_out(vty,"onu%d/%d/%d has e1-link, Please delete e1-link first\r\n",  ulSlot,ulPort,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
			}					
		}
#endif
		if( argc == 2 )
		{
			UCHAR onuMacAddr[6];
			int len = 0;
			ULONG authIdx = 0;
			if( GetOnuMacAddr(phyPonId, userOnuId, onuMacAddr, &len) == ROK )
			{
				if( isHaveAuthMacAddress(ulSlot, ulPort, onuMacAddr, &authIdx ) == VOS_OK )
			 	{
					if( authIdx <= MAXONUPERPON )
				 	{
						setOnuAuthStatus( ulSlot, ulPort, authIdx, V2R1_ENTRY_DESTORY );
				 	}
			 	}
			}
		}
		
		lRet = DelOnuFromPonPort(phyPonId, userOnuId);
        if ( ROK != lRet )
        {
            error_count++;
    		if ( lRet == V2R1_ONU_NOT_EXIST )
    		{
                empty_count++;
    		}
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}
DEFUN  (
    olt_del_onu_config,
    olt_del_onu_config_cmd,
    "delete onu <slot/port> <onuid_list> {[auth-mac]}*1",
    "delete onu from the pon port\n"
    "delete onu from the pon port\n"
	"please input pon port slot/port\n"      
    OnuIDStringDesc
    "delete the onu's authentication mac address at the same time\n"
    )
{
	LONG lRet = VOS_OK;
	INT16 phyPonId = 0;
	INT16 userOnuId = 0;
	/*ULONG ulIfIndex = 0;*/
	ULONG ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	int count = 0;
	int empty_count = 0;
	int error_count = 0;
	int slot = 0;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );	
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);
    
    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }	
		
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], ulOnuId )
 	{
 		count++;
        
 		userOnuId = (short int)(ulOnuId - 1);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		slot = get_gfa_tdm_slotno();
		if(slot)
		{
			if(SlotCardIsTdmSgBoard(slot) == ROK)
			{
				unsigned char tdm_slot, tdm_sg;
				unsigned short int logidOnuId;
				
				if(GetOnuBelongToSG( MAKEDEVID(ulSlot, ulPort, ulOnuId), &tdm_slot, &tdm_sg, &logidOnuId) == ROK )
				{
					vty_out(vty,"onu%d/%d/%d is config to sig%d/%d, Please delete voice-onu from sig first\r\n", ulSlot,ulPort,ulOnuId,tdm_slot,tdm_sg);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
			}
			else if(SlotCardIsTdmE1Board(slot) == ROK)
			{
				OnuE1Info  pOnuE1Info;
				VOS_MemSet(&pOnuE1Info, 0, sizeof(OnuE1Info));
				if((GetOnuE1Info(MAKEDEVID(ulSlot, ulPort, ulOnuId), &pOnuE1Info) == ROK)
					&& (pOnuE1Info.onuValidE1Count != 0 ))
				{
					vty_out(vty,"onu%d/%d/%d has e1-link, Please delete e1-link first\r\n",  ulSlot,ulPort,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
			}					
		}
#endif
		if( argc == 3 )
		{
			UCHAR onuMacAddr[6];
			int len = 0;
			ULONG authIdx = 0;
			if( GetOnuMacAddr(phyPonId, userOnuId, onuMacAddr, &len) == ROK )
			{
				if( isHaveAuthMacAddress(ulSlot, ulPort, onuMacAddr, &authIdx ) == VOS_OK )
			 	{
					if( authIdx <= MAXONUPERPON )
				 	{
						setOnuAuthStatus( ulSlot, ulPort, authIdx, V2R1_ENTRY_DESTORY );
				 	}
			 	}
			}
		}
		
		lRet = DelOnuFromPonPort(phyPonId, userOnuId);
        if ( ROK != lRet )
        {
            error_count++;
    		if ( lRet == V2R1_ONU_NOT_EXIST )
    		{
                empty_count++;
    		}
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}

#if 0 
extern  STATUS t_changeOnuStatus( short int ponPortIdx, short int onuIdx,  short int actcode );
 /* chenfj 2008-1-22, 下面两个命令被废弃了, 将源代码也注释掉*/
/*限制onu*/
DEFUN  (
    pon_onu_regiseter_limit_config,
    pon_onu_regiseter_limitconfig_cmd,
    "register limit-onu ",
    "onu register config\n"
    "onu limit register config\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    ULONG ulIfIndex = 0;
    int  cliflag	 = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	cliflag = CLI_EPON_ONULIMIT_ENABLE;
    lRet = SetOnuRegisterLimitFlag( phyPonId, cliflag );	
    if (lRet != VOS_OK)
         {
           #ifdef CLI_EPON_DEBUG
       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
           #endif     
           	vty_out( vty, "  %% Executing error.\r\n");
       	return CMD_WARNING;
         }
	
    return CMD_SUCCESS;
}

DEFUN  (
    pon_active_onu_config,
    pon_active_onu_config_cmd,
    "activate onu <1-64> [1|2]",
    "activate onu of ponIdx\n"
    "activate onu\n"
    "Please input onu ID\n"
    "activate onu\n"
    "deactivate onu\n"
    )
{
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    short int  actcode = 0; /*1: active 2: deactive*/
    ULONG ulIfIndex = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

        userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;

  	actcode = VOS_AtoL( argv[1] );

	if( t_changeOnuStatus( phyPonId, userOnuId , actcode ) == VOS_ERROR )
	{
		vty_out(vty,"\r\nchange onu status fail!\r\n");
		return CMD_WARNING;
	}
	else
		vty_out(vty,"\r\nchange onu status ok!\r\n");
	
    return CMD_SUCCESS;
}
#endif

#if 0
DEFUN  (
    pon_change_onu_type_config,
    pon_change_onu_type_config_cmd,
    "set onu type <1-64> [1|2]",
    "set onu of ponIdx\n"
    "set onu\n"
    "modify onu type\n"
    "Please input onu ID\n"
    "CTC onu\n"
    "GW onu\n"
    )
{
    LONG lRet = VOS_OK;
    
    short int  actcode = 0; /*1: active 2: deactive*/
    ULONG ulIfIndex = 0;

    ULONG slot=0,port=0,onuid=0, devIdx=  0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    
	PON_GetSlotPortOnu( ulIfIndex, &slot, &port , &onuid );
	
        onuid = (( INT16 ) VOS_AtoL( argv[ 0 ] )) ;

  	actcode = VOS_AtoL( argv[1] );

	devIdx = slot*10000+port*1000+onuid;

	vty_out( vty, "\r\nset device index=%d", devIdx );

	setOnuType( devIdx, actcode ); 
	
    return CMD_SUCCESS;
}

#endif
/***PON Point************************/
/***PON Management********/

#ifdef  PON_ENCRYPT_CONFIG
#endif
/* modified by chenfj 2008-12-24
	问题单7806, 将ONU 加密命令设置在PON节点下;
	分析后,修改为:
	不删除ONU节点下相关配置命令; 但在PON 节点下增加相应命令
	同时将启动加密及密钥更新时间合并为一条命令
	将此命令的执行发送到OLT 管理任务中
	*/

DEFUN  (
    pon_encrypt_config,
    pon_encrypt_config_cmd,
    "encrypt <onuid_list> direction [up-down|down] update-key-interval [default|<5-10000>]",
    "Config pon's encrypt attribute\n"
    "OnuIdx,e.g 1,3-5,7, etc. the range is 1-"INT_TO_STR(MAXONUPERPON)"\n"
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
    /*ULONG ulIfIndex = 0;*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
	short int OnuIdx;
	unsigned int count=0/*, EncryptStatus*/;
	int		empty_count=0;
	int		support_count=0;
	int		error_count=0;
	unsigned int UpdateKeyTime=0;
    /*ulIfIndex = ( ULONG ) ( vty->index ) ;*/


    if ( !VOS_StrCmp( argv[ 1 ], "up-down" ) )
    {
        ulEncrypt = PON_ENCRYPTION_DIRECTION_ALL;
    }
    else /*if ( !VOS_StrCmp( argv[ 1 ], "down" ) )*/
    {
        ulEncrypt = PON_ENCRYPTION_DIRECTION_DOWN;
    }
    /*else
    {
        vty_out( vty, "  %% direction Parameter error\r\n" );
        return CMD_WARNING;
    }*/
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if(VOS_StrCmp( argv[ 2 ], "default" ) == 0 )
		UpdateKeyTime = GetOnuEncryptKeyExchagetimeDefault();
	else 
		UpdateKeyTime = VOS_AtoL( argv[2] )*SECOND_1;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], onuId )
	{
		count++;

		OnuIdx = onuId-1;
		if( ThisIsValidOnu((short int)phyPonId, OnuIdx ) != ROK )
		{
		    error_count++;
		    empty_count++;

            continue;
		}
		
		if( EncryptIsSupported((short int)phyPonId, OnuIdx ) != ROK )
		{
		    error_count++;
		    support_count++;
            
			continue;
		}

		/* 之前已配置,且加密已启动
		if((GetOnuEncryptStatus((short int)phyPonId, OnuIdx , &EncryptStatus) == ROK )
			&& (EncryptStatus == V2R1_STARTED))
			{
			if( OnuMgmtTable[phyPonId * MAXONUPERPON + OnuIdx].EncryptDirection != ulEncrypt )
				{
				vty_out(vty,"onu%d/%d/%d encrypt is started, direction is %s; if want to start %s encrypt, should stop encrypt first\r\n", slotId, port, onuId, v2r1EncryptDirection[OnuMgmtTable[phyPonId * MAXONUPERPON + OnuIdx].EncryptDirection], v2r1EncryptDirection[ulEncrypt]);
				if(counter == 1 )
					return( CMD_WARNING );
				else continue;
				}
			else{
				SetOnuEncryptKeyExchagetime((short int)phyPonId, OnuIdx, UpdateKeyTime);
				continue;
				}
			}	
		*/
		lRet = OnuEncryptionOperation( (short int) phyPonId, OnuIdx, ulEncrypt);
		if(lRet != VOS_OK)
		{
		    error_count++;
            
			continue;
		}
		SetOnuEncryptKeyExchagetime((short int)phyPonId, OnuIdx, UpdateKeyTime);
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (support_count == count)
            || ((support_count + empty_count) == count) )
        {
			vty_out(vty, "  %%  onu PON chip mismatch, encrypt is failed\r\n");
        }
        else
        {
			vty_out( vty, "  %%  encrypt Executing error.\r\n");
        }
        
	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}


DEFUN  (
    pon_no_encrypt_config,
    pon_no_encrypt_config_cmd,
    "undo encrypt <onuid_list>",
    NO_STR
    "Config pon's encrypt attribute\n"
    "OnuIdx,e.g 1,3-5,7, etc. the range is 1-"INT_TO_STR(MAXONUPERPON)"\n"
   /* &g_Olt_Queue_Id*/
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
    short int OnuIdx;
    unsigned int ulEncrypt = PON_ENCRYPTION_PURE;
    unsigned int count=0;
    int		empty_count=0;
	int		support_count=0;
    int		error_count=0;
    unsigned int UpdateKeyTime=0;
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	UpdateKeyTime = GetOnuEncryptKeyExchagetimeDefault();
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], onuId )
	{
		count++;
        
		OnuIdx = onuId-1;
		if( ThisIsValidOnu((short int)phyPonId, OnuIdx ) != ROK )
		{
		    error_count++;
		    empty_count++;

			continue;
		}
		
		SetOnuEncryptKeyExchagetime(phyPonId, OnuIdx, UpdateKeyTime);
		
		if( EncryptIsSupported((short int)phyPonId, OnuIdx ) != ROK )
		{
		    error_count++;
		    support_count++;
            
			continue;
		}
		
		if(OnuEncryptionOperation( (short int) phyPonId, OnuIdx, ulEncrypt) != ROK)
		{
		    error_count++;
            
			continue;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (support_count == count)
            || ((support_count + empty_count) == count) )
        {
			vty_out(vty, "  %%  onu PON chip mismatch, encrypt is failed\r\n");
        }
        else
        {
			vty_out( vty, "  %%  undo-encrypt Executing error.\r\n");
        }
        
	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}

/* 支持予配置,不判断PON 板及芯片是否在位*/
/* modified by chenfj 2008-12-25 
	增加ONU 设备索引, 可对多个ONU 同时操作
	保留此命令, 虽然在启动加密的命令中, 已有设置更新密钥时间的参数;
	*/
DEFUN  (
    pon_encrypt_keytime_config,
    pon_encrypt_keytime_config_cmd,
    "encrypt <onuid_list> update-key-interval [default|<30-10000>]",
    "Config pon's encrypt\n"
		"OnuIdx,e.g 1,3-5,7, etc. the range is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
    "encrypt key update interval\n"
    "restore to deault value\n"
    "Please input the time length(second)\n"
    )
{
    /*ULONG ulIfIndex = 0;*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    unsigned int  time_len = 0;
	unsigned int count = 0;
	int		empty_count=0;
	int		error_count=0;
	short int OnuIdx;

	if(VOS_StrCmp( argv[1], "default" ) == 0 )
		time_len = GetOnuEncryptKeyExchagetimeDefault();
	else 
		time_len = VOS_AtoL( argv[1] )*SECOND_1;
	
    if (time_len <= 0)
    {
#ifdef CLI_EPON_DEBUG
    	vty_out(vty, "  %% DEBUG: time_len %d  cmd encrypt-keytime failed\r\n",time_len);
#endif         
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_SUCCESS;
    }
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], onuId )
	{
		count++;
		OnuIdx = onuId-1;
		if( ThisIsValidOnu( phyPonId, OnuIdx ) == ROK )
        {
			SetOnuEncryptKeyExchagetime( phyPonId, OnuIdx, time_len);
        }      
		else
        {
            error_count++;
            empty_count++;    
        }      
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}
 /* 支持予配置,不判断PON 板及芯片是否在位*/
QDEFUN  (
    pon_show_encrypt_config,
    pon_show_encrypt_config_cmd,
    "show encrypt information",
    DescStringCommonShow
    "Show pon's encrypt attribute\n"
    "Show pon's encrypt information\n",
	&g_Olt_Queue_Id
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	lRet = ShowPonPortEncryptInfoByVty(phyPonId, vty );
	if (lRet != VOS_OK)
	    {
	       #ifdef CLI_EPON_DEBUG
	       	  vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d onuId %d.\r\n",lRet, phyPonId);
	        #endif     
	           vty_out( vty, "  %% Executing error.\r\n");
	       	return CMD_WARNING;
	    }		
    /*ShowOnuEncryptInfo( (short int )phyPonId, (short int )onuId ) ;*/

    return CMD_SUCCESS;
}

/* chenfj 2008-1-22, 下面这个命令被废弃了, 将源代码也注释掉*/
#if 0
DEFUN  (
    pon_bandwidth_alloc_mode_config,
    pon_bandwidth_alloc_mode_config_cmd,
    "bandwidth alloc-mode [dba|sba]",
    "Config pon's bandwidth\n"
    "Config pon's bandwidth alloc mode\n"
    "DBA( Dynamic bandwidth alloc mode )\n"
    "SBA( Static bandwidth alloc mode )\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    int dba_mode = 0;	
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;
    if ( !VOS_StrCmp( argv[ 0 ], "sba" ) )
    {
        dba_mode = EPON_SBA;
        vty_out( vty, "  %% sba is not supply.\r\n" );
        return CMD_WARNING;		
    }
    else if ( !VOS_StrCmp( argv[ 0 ], "dba" ) )
    {
         dba_mode = EPON_DBA;
		 
    }
    else
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% cmd bandwidth alloc-mode failed\r\n");
    #endif         
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot((short int)slotId, (short  int)port);
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    lRet = SetPonPortBWMode( (short int )phyPonId, dba_mode );
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d dba_mode %d.\r\n",lRet, phyPonId, dba_mode);
    #endif
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }		
    return CMD_SUCCESS;
}
#endif

/*设置mac 地址表项的最大数目

增加了onuId 参数，llid为每个onu 的逻辑链路,目前每个onu的逻辑连接
为1，为了扩展以后，这里定义为1-8 

modified by wutw at 11 september
缺少
返回值:
onu_up	1
onu_down	2
error 	-1
*/

#ifdef  ONU_MAX_MAC_AND_FEC
#endif 

DEFUN  (
    onu_default_max_mac_config,
    onu_default_max_mac_config_cmd,
    "onu default-max-mac [0|<1-8192>]",
    "Config onu info.\n"
    "Config the default max-mac number supported by per onu\n"
    "0 indicate no mac-learning limit\n"
    "Please input the max-mac number supported,the default value is 128\n"
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    UINT32 number = 0;
	int count = 0;
	int		empty_count=0;
	int		error_count=0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
    INT16   llidIndex = 0;
	short int PonChipType;

    number = ( ULONG ) VOS_AtoL( argv[ 0 ] );

    if ( OLT_CALL_ISERROR(OLT_SetOnuDefaultMaxMac(OLT_ID_ALL, number)) )
    {
        return RERROR;
    }


	llidIndex = CLI_EPON_DEFAULTLLID;
	for(phyPonId=0;phyPonId<MAXPON;phyPonId++)      
	{
    	PonChipType = V2R1_GetPonchipType( phyPonId );

    	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
    	if( PonChipType == PONCHIP_PAS5001 )
    		continue;
        
    	for(userOnuId=0;userOnuId<MAXONUPERPON;userOnuId++)
    	{
            SetOnuMaxMacNum((short int)phyPonId, (short int)userOnuId, llidIndex, number);
    	}
	}

    return CMD_SUCCESS;
}

DEFUN  (
    undo_onu_default_max_mac_config,
    undo_onu_default_max_mac_config_cmd,
    "undo onu default-max-mac",
    "clear config\n"
    "clear the current max-mac config\n"
    "restore the default value(128)\n"
    )
{
    INT16   llidIndex = 0;
	unsigned int number = ONU_DEFAULT_MAX_MAC;
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;	
	short int PonChipType;

    if ( OLT_CALL_ISERROR(OLT_SetOnuDefaultMaxMac(OLT_ID_ALL, number)) )
    {
        return RERROR;
    }


	llidIndex = CLI_EPON_DEFAULTLLID;
	for(phyPonId=0;phyPonId<MAXPON;phyPonId++)      
	{
    	PonChipType = V2R1_GetPonchipType( phyPonId );

    	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
    	if( PonChipType == PONCHIP_PAS5001 )
    		continue;
        
    	for(userOnuId=0;userOnuId<MAXONUPERPON;userOnuId++)
    	{
            SetOnuMaxMacNum((short int)phyPonId, (short int)userOnuId, llidIndex, number);
    	}
	}

    return CMD_SUCCESS;
}
DEFUN  (
    onu_default_max_mac_show,
    onu_default_max_mac_show_cmd,
    "show onu default-max-mac",
    DescStringCommonShow
    "Show the default max-mac number\n"
    "Show the default max-mac number\n"
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/

	vty_out(vty, "Onu default max-mac is: %d.\r\n", MaxMACDefault);
    return CMD_SUCCESS;
}

/* the following cli cmd  is added by chenfj  2007-5-23 */
/* 支持予配置,不判断PON 板及芯片是否在位*/
DEFUN  (
    onu_max_mac_config,
    onu_max_mac_config_cmd,
    "onu max-mac [0|<1-8192>] <onuid_list>",
    "Config onu info.\n"
    "Config the max-mac number supported by per onu\n"
    "0 indicate no mac-learning limit\n"
    "Please input the max-mac number supported,the default value is 128\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    UINT32 number = 0;
	int count = 0;
	int		empty_count=0;
	int		error_count=0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
    INT16   llidIndex = 0;
	short int PonChipType;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	PonChipType = V2R1_GetPonchipType( phyPonId );
#if 0
	if(PonChipType == PONCHIP_PAS5001 )
		vty_out(vty,"Note:this is PAS5001,setting limitation of address learned by an onu,the MAC table can lose entries after long runs of traffic and frequent aging,and also possible Data path loss\r\n");
#else
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( PonChipType == PONCHIP_PAS5001 )
	{
		vty_out( vty, "  %% this is pas5001, max mac number is not supported\r\n");
		return( CMD_WARNING );
	}

#endif

    number = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    number |= ONU_NOT_DEFAULT_MAX_MAC_FLAG;
	llidIndex = CLI_EPON_DEFAULTLLID;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	{
    	count++;
		userOnuId = (short int)(ulOnuId - 1);
        lRet=SetOnuMaxMacNum((short int)phyPonId, (short int)userOnuId, llidIndex, number);
    	if ( lRet == V2R1_ONU_NOT_EXIST )	/* modified by xieshl 20100325, 问题单9978 */
		{
		    error_count++;
        	empty_count++;
		}
    	else if( lRet == RERROR ) 
		{
		    error_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% set onu max mac err\r\n");
        }

	   	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}
/* 支持予配置,不判断PON 板及芯片是否在位*/
DEFUN  (
    undo_onu_max_mac_config,
    undo_onu_max_mac_config_cmd,
    "undo onu max-mac <onuid_list>",
    "clear config\n"
    "clear the current max-mac config\n"
    "restore the default value(128)\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
    INT16   llidIndex = 0;
	int count = 0;
	int		empty_count=0;
	int		error_count=0;
	unsigned int number = MaxMACDefault;
	number |= ONU_UNDO_MAX_MAC_FLAG; 
	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

#if 0
	if(GetPonChipTypeByPonPort(phyPonId) == PONCHIP_PAS5001 )
		vty_out(vty,"Note:this is PAS5001,setting limitation of address learned by an onu,the MAC table can lose entries after long runs of traffic and frequent aging,and also possible Data path loss\r\n");
#endif

	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if(OLTAdv_IsExist( phyPonId ))
	{
		if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
			vty_out( vty, "  %% this is pas5001, max mac number is not supported\r\n");
			return( CMD_WARNING );
		}
	}

#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, max mac number is not supported\r\n");
		return( CMD_SUCCESS );
		}
#endif

	llidIndex = CLI_EPON_DEFAULTLLID;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
    	count++;
 		userOnuId = (short int)(ulOnuId - 1);
        lRet=SetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, llidIndex, number);
		if ( lRet == V2R1_ONU_NOT_EXIST )	/* modified by xieshl 20100325, 问题单9978 */
		{
		    error_count++;
        	empty_count++;
		}
    	else if( lRet == RERROR ) 
		{
		    error_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% clear onu max mac err\r\n");
        }

	   	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}
/* 支持予配置,不判断PON 板及芯片是否在位*/
/* this cli cmd is modified by chenfj 2007-5-23 */
DEFUN  (
    onu_max_mac_show,
    onu_max_mac_show_cmd,
    "show onu max-mac <onuid_list>",
    DescStringCommonShow
    "Show the max-mac number\n"
    "Show the max-mac number\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    UINT32 number = 0;
	int count = 0;
	int		empty_count=0;
	int		error_count=0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	
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

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], onuId )
 	{
 		count++;
 		userOnuId = (short int)(onuId - 1);
		lRet = GetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, &number);
		if( lRet == ROK ) 
		{
			vty_out( vty, "   onu%d/%d/%d max-mac supported---%d\r\n",slotId,port,onuId,number);
		}
        else
        {
            error_count++;
			if ( lRet == V2R1_ONU_NOT_EXIST )
			{
                empty_count++;
			}
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% get onu max mac err\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}

DEFUN  (
    onu_max_mac_show_by_pon,
    onu_max_mac_show_by_pon_cmd,
    "show onu max-mac <slot/port> <onuid_list>",
    DescStringCommonShow
    "Show the max-mac number\n"
    "Show the max-mac number\n"
   	"input pon port slot/port\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    short int onuid = 0;
	short int ulOnuId = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
    unsigned int number = 0;
    int error_count = 0;
    int empty_count = 0;
    int count = 0;
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );	
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );    
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }


	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], onuid )
 	{
 		count++;
 		ulOnuId = (short int)(onuid - 1);
		lRet = GetOnuMaxMacNum((short int )phyPonId, (short int)ulOnuId, &number);
		if( lRet == ROK ) 
		{
			vty_out( vty, "   onu%d/%d/%d max-mac supported---%d\r\n",ulSlot,ulPort,onuid,number);
		}
        else
        {
            error_count++;
			if ( lRet == V2R1_ONU_NOT_EXIST )
			{
                empty_count++;
			}
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% get onu max mac err\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}
#ifdef  ONU_FEC_CONFIG
#endif
/* 支持予配置,不判断PON 板及芯片是否在位*/
DEFUN  (
    onu_fec_config_pon,
    onu_fec_config_pon_cmd,
    "fec-mode [enable|disable] <onuid_list>",
    "config onu fec\n"
    "enable onu fec\n"
    "disable onu fec\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int count = 0;
	int		empty_count=0;
	int		offline_count=0;
	int		error_count=0;
	int fec_mode = STD_FEC_MODE_ENABLED ;/*CLI_EPON_ALARM_ENABLE + 1;*/

    
	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	
	
#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this is PAS5001, FEC is not supported\r\n" );
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	/* 此处应增加对ONU 类型的判断，只有PAS6301 ONU 才能支持FEC 设置*/
	
	if( VOS_StrCmp((CHAR *)argv[0], "enable") == 0 )
		fec_mode = STD_FEC_MODE_ENABLED;
	else if( VOS_StrCmp((CHAR *)argv[0], "disable") == 0 )
		fec_mode = STD_FEC_MODE_DISABLED;
	else{
		vty_out(vty, "  %% param err\r\n");
		return( CMD_WARNING );
		}
		
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	{
 		count++;
 		userOnuId = (short int)(ulOnuId - 1);
		lRet = SetOnuFecMode((short int )phyPonId, (short int)userOnuId, fec_mode );
        if ( ROK != lRet )
        {
     		error_count++;
            switch (lRet)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_OFF_LINE:
                    offline_count++;
                    break;
                default:
    				vty_out( vty, "  %% set onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (offline_count == count)
                || ((offline_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% onu is off-line.\r\n");
        }

	   	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}

DEFUN  (
    undo_onu_fec_config_pon,
    undo_onu_fec_config_pon_cmd,
    "undo onu fec <onuid_list>",
    "disable onu fec config\n"
    "disable onu fec config\n"
    "disable onu fec config\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int count = 0;
	int		empty_count=0;
	int		offline_count=0;
	int		error_count=0;
	int Done = STD_FEC_MODE_UNKNOWN ; /*CLI_EPON_ALARM_DISABLE + 1;*/

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	/* 此处应增加对ONU 类型的判断，只有PAS6301 ONU 才能支持此设置*/
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
 		count++;
 		userOnuId = (short int)(ulOnuId - 1);
		lRet = SetOnuFecMode((short int )phyPonId, (short int)userOnuId, Done );
        if ( ROK != lRet )
        {
     		error_count++;
            switch (lRet)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_OFF_LINE:
                    offline_count++;
                    break;
                default:
    				vty_out( vty, "  %% set onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (offline_count == count)
                || ((offline_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% onu is off-line.\r\n");
        }

	   	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}


DEFUN  (
    show_onu_fec_config_pon,
    show_onu_fec_config_pon_cmd,
    "show fec-mode <onuid_list>",
    "show onu fec mode\n"
    "show onu fec mode\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	int count = 0;
	int fec_mode = STD_FEC_MODE_UNKNOWN;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

#if 0
	/* PAS5001不支持此设置*/
	if( V2R1_GetPonchipType( phyPonId ) == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( V2R1_GetPonchipType( phyPonId ) != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	/* 此处应增加对ONU 类型的判断，只有PAS6301 ONU 才能支持此设置*/

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
	if ( count == 1 )  /*参数中只有一个ONU */
		{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	 		{
	 		userOnuId = (short int)(ulOnuId - 1);
			if( GetOnuOperStatus(phyPonId, userOnuId )  != ONU_OPER_STATUS_UP )
				{
				vty_out( vty, "FEC Mode : unknown\r\n" );
				RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
			
			if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)
				{
				lRet = GetOnuFecMode((short int )phyPonId, (short int)userOnuId, &fec_mode );
				if ( lRet == V2R1_ONU_NOT_EXIST )
					{
					vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
					}
				if( lRet == RERROR ) 
					{
					vty_out( vty, "  %% get onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
					}
				}
			
			else{
				lRet = CTC_GetLlidFecMode( phyPonId, userOnuId, &fec_mode);
				if ( lRet == V2R1_ONU_NOT_EXIST )
					{
					vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
					}
				if( lRet == V2R1_ONU_OFF_LINE )
					{
					vty_out(vty,"  %% onu %d/%d/%d is off-line\r\n",slotId,port,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
					}
				if( lRet == RERROR ) 
					{
					vty_out( vty, "  %% get onu%d/%d/%d FEC err\r\n",slotId,port,ulOnuId);
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
					}
				}

			if( fec_mode == STD_FEC_MODE_UNKNOWN )
				vty_out( vty, "FEC Mode : unknown\r\n" );
			else if( fec_mode == STD_FEC_MODE_DISABLED )
				vty_out( vty, "FEC Mode : Disable\r\n" );
			else if( fec_mode == STD_FEC_MODE_ENABLED)
				vty_out( vty, "FEC Mode : Enable\r\n" );
			else
				vty_out( vty, "FEC Mode : Invalid value\r\n" );
			}
	
		END_PARSE_ONUID_LIST_TO_ONUID();
		}
	
	else /*  参数中有多个ONU */
		{
		vty_out(vty, "  OnuIdx            FEC mode\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	 		{
	 		userOnuId = (short int)(ulOnuId - 1);

			if(ThisIsValidOnu(phyPonId, userOnuId ) != ROK )
				continue;

			if( GetOnuOperStatus(phyPonId, userOnuId )  != ONU_OPER_STATUS_UP )
				{
				vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
				continue;
				}
			
			if( GetOnuVendorType(phyPonId, userOnuId) ==  ONU_VENDOR_GW)
				{
				lRet = GetOnuFecMode((short int )phyPonId, (short int)userOnuId, &fec_mode );
				if( lRet != ROK )
					continue;
				if ( lRet == V2R1_ONU_NOT_EXIST )
					{
					vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
					continue;
					}
				if( lRet == RERROR ) 
					{
					vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
					continue;
					}
				}
			
			else{
				lRet = CTC_GetLlidFecMode( phyPonId, userOnuId, &fec_mode);
				if( lRet != ROK )
					continue;
				if ( lRet == V2R1_ONU_NOT_EXIST )
					{
					vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
					continue;
					}
				if( lRet == V2R1_ONU_OFF_LINE )
					{
					vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
					continue;
					}
				if( lRet == RERROR ) 
					{
					vty_out( vty, "  %d/%d/%d             unknown\r\n", slotId, port,ulOnuId);
					continue;
					}
				}

			vty_out( vty, "  %d/%d/%d             ", slotId, port,ulOnuId);
			if( fec_mode == STD_FEC_MODE_UNKNOWN )
				vty_out( vty, "Unknown\r\n" );
			else if( fec_mode == STD_FEC_MODE_DISABLED )
				vty_out( vty, "Disable\r\n" );
			else if( fec_mode == STD_FEC_MODE_ENABLED)
				vty_out( vty, "Enable\r\n" );
			else
				vty_out( vty, "Invalid value\r\n" );
					
			}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
		}

    return CMD_SUCCESS;
}

/* 显示当前及时值,需判断PON 板及芯片是否在位*/
DEFUN(
	show_fec_ability_pon,
	show_fec_ability_pon_cmd,
	"show fec-ability <onuid_list>",
	SHOW_STR
	"Display fec ability\n"
	OnuIDStringDesc
	)
{
	PON_olt_id_t	olt_id;
	unsigned long ulSlot, ulPort, ulOnu;
	short int ulOnuId;
	short int PonChipType;
	CTC_STACK_standard_FEC_ability_t	fec_ability;
	int count = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnu, &olt_id) != VOS_OK )
		return CMD_WARNING;    	
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK)
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 3 PON 芯片在位检查*/
	if( getPonChipInserted( (ulSlot), (ulPort)) != PONCHIP_EXIST )
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
		return (CMD_WARNING );
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	if( PonPortIsWorking(olt_id) != TRUE )
		{
		vty_out(vty,"\r\n Pon%d/%d is not working\r\n", ulSlot, ulPort );
		return (CMD_WARNING );
		}
	
	PonChipType = V2R1_GetPonchipType( olt_id );

#if 0
	
	/* PAS5001不支持此设置*/
	if( PonChipType == PONCHIP_PAS5001 )
		{
		vty_out( vty, "  FEC is Unsupported\r\n" );
		/*vty_out( vty, "  %% this pon chip is PAS5001, FEC is not supported\r\n");*/
		return( CMD_SUCCESS );
		}
	
	/* 当前只有PAS5201支持此设置；其他类型待以后增加*/
	if( PonChipType != PONCHIP_PAS5201 )
		{
		vty_out( vty, "  %% FEC is not supported\r\n");
		return( CMD_WARNING );
		}
#endif

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();


	if ( count == 1 )  /*参数中只有一个ONU */
		{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnu )
			{
			ulOnuId = ulOnu-1;

			/* 问题单#6139 : 对ONU 是否合法检查有误*/
			if(ThisIsValidOnu( olt_id, ulOnuId) != ROK )
				{
				vty_out(vty, "  %% onu %d/%d/%d not exist\r\n",ulSlot, ulPort, ulOnu);
				RETURN_PARSE_ONUID_LIST_TO_ONUID(CMD_WARNING );
				}
			
			if(GetOnuOperStatus(olt_id, ulOnuId) != ONU_OPER_STATUS_UP )
				{
				vty_out(vty,"  %% onu %d/%d/%d/is off-line\r\n", ulSlot, ulPort, ulOnu);
				RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}

			if( PonChipType == PONCHIP_PAS5001 )
			{
				fec_ability = STD_FEC_ABILITY_UNSUPPORTED;
			}
			else
			{
				if( GetOnuVendorType( olt_id, ulOnuId) == ONU_VENDOR_CT )
				{
					if( CTC_GetLlidFecCapability( olt_id, ulOnuId, (int *)&fec_ability) != ROK) 
					{
						fec_ability = STD_FEC_ABILITY_UNKNOWN;
					}
				}
				else /*if( GetOnuVendorType( olt_id, ulOnuId) == ONU_VENDOR_GW )*/
				{
					fec_ability = STD_FEC_ABILITY_SUPPORTED;
				}
			}
			if( (fec_ability < STD_FEC_ABILITY_UNKNOWN) || (fec_ability > STD_FEC_ABILITY_UNSUPPORTED) )
				fec_ability = STD_FEC_ABILITY_UNKNOWN;
			vty_out(vty,"  FEC Ability : %s\r\n", Fec_Ability[fec_ability]);
				RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_SUCCESS );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}

	else {   /* 参数中有多个ONU*/
		vty_out(vty, "  OnuIdx            FEC ability\r\n");
		
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnu )
			{
			ulOnuId = ulOnu-1;
			if(ThisIsValidOnu( olt_id, ulOnuId) != ROK )
				continue;
			/*
			if(ThisIsValidOnu( olt_id, ulOnuId) == V2R1_ONU_NOT_EXIST )
				{
				continue;
				}
			*/
			if(GetOnuOperStatus(olt_id, ulOnuId) != ONU_OPER_STATUS_UP )
				continue;
			
			if( PonChipType == PONCHIP_PAS5001 )
				{
				fec_ability = STD_FEC_ABILITY_UNSUPPORTED;
				}
			else 
				{
				if( GetOnuVendorType( olt_id, ulOnuId) == ONU_VENDOR_CT )
					{
					if( CTC_GetLlidFecCapability( olt_id, ulOnuId, (int *)&fec_ability) != ROK) 
						{
						fec_ability = STD_FEC_ABILITY_UNKNOWN;
						}
					}
				else /*if( GetOnuVendorType( olt_id, ulOnuId) == ONU_VENDOR_GW )*/
					{
					fec_ability = STD_FEC_ABILITY_SUPPORTED;
					}
				}
				if( (fec_ability < STD_FEC_ABILITY_UNKNOWN) || (fec_ability > STD_FEC_ABILITY_UNSUPPORTED) )
					fec_ability = STD_FEC_ABILITY_UNKNOWN;
				vty_out( vty, "  %d/%d/%d             %s\r\n",ulSlot, ulPort,ulOnu, Fec_Ability[fec_ability]);
			}
		END_PARSE_ONUID_LIST_TO_ONUID();
		}

	return CMD_WARNING;
}

/******************   modified end ****************************/ 

#ifdef  ONU_UPSTREAM_SA_MAC_FILTER

/* the following cli is add by chenfj 2007-6-1 
	设置ONU 上行数据包原目的MAC 过滤
*/
DEFUN  (
	onu_SA_mac_filter_config,
	onu_SA_mac_filter_config_cmd,
	"onu src-mac filter <onuid_list> <H.H.H>",
	"Config onu upstream source mac filter\n"
	"Config onu upstream source mac filter\n"
	"Config onu upstream source mac filter\n"
	OnuIDStringDesc
	"Please input the source mac to be filtered\n"
	)
{
	LONG lRet = VOS_OK;
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int count = 0;
	int	empty_count = 0;
	int	invalid_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	unsigned char MacAddr[6] = {0,0,0,0,0,0};

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
	{
       	 vty_out( vty, "  %% Invalid MAC address.\r\n" );
       	 return CMD_WARNING;
	}
		
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
 		count++;
        
 		userOnuId = (short int)(ulOnuId - 1);
		lRet = AddOnuSAMacFilter((short int )phyPonId, (short int)userOnuId, MacAddr);
        if ( ROK != lRet )
        {
		    error_count++;
            switch (lRet)
            {
                case V2R1_ONU_NOT_EXIST:
    			    empty_count++;
                    break;
                case V2R1_ONU_FILTER_SA_MAC_NOT_VALID:
    			    invalid_count++;
                    break;
                case V2R1_ONU_FILTER_SA_MAC_EXIST:
    			    exist_count++;
                    break;
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (invalid_count == count)
                || ((invalid_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the mac address is invalid.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the mac address is already in filter table.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% set onu source mac filter err\r\n");
        }

	   	return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
}

DEFUN  (
	undo_onu_SA_mac_filter_config,
	undo_onu_SA_mac_filter_config_cmd,
	"undo onu src-mac filter <onuid_list> {<H.H.H>}*1",
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	"clear onu upstream source mac filter\n"
	OnuIDStringDesc
	"Please input the source mac to be un-filtered\n"
	)
{
	LONG lRet = VOS_OK;
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int count = 0;
	int	empty_count = 0;
	int	invalid_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	unsigned char MacAddr[6] = {0,0,0,0,0,0};

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 2 )
    {
    	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }   
		
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
	    count++;
        
 		userOnuId = (short int)(ulOnuId - 1);
		if( argc == 2 )
		{
			lRet = ClearOnuSAMacFilter((short int )phyPonId, (short int)userOnuId, MacAddr);
            if ( ROK != lRet )
            {
    		    error_count++;
                switch (lRet)
                {
                    case V2R1_ONU_NOT_EXIST:
        			    empty_count++;
                        break;
                    case V2R1_ONU_FILTER_SA_MAC_NOT_VALID:
        			    invalid_count++;
                        break;
                    case V2R1_ONU_FILTER_SA_MAC_NOT_EXIST:
        			    exist_count++;
                        break;
                }
            }
		}
		else if( argc == 1 ) /* 没有MAC 地址参数，删除全部过滤*/
		{
			ClearOnuSAMacFilterAll( (short int )phyPonId, (short int)userOnuId );
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (invalid_count == count)
                || ((invalid_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the mac address is invalid.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the mac address isn't in filter table.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% clear onu source mac filter err\r\n");
        }

	   	return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
}

DEFUN  (
	show_onu_SA_mac_filter_config,
	show_onu_SA_mac_filter_config_cmd,
	"show onu src-mac filter {<onuid_list>}*1",
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	"show onu upstream source mac filter\n"
	OnuIDStringDesc
	)
{
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;
	int count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 0 ) /* 显示PON端口下所有ONU 原MAC过滤表*/
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId ++ )
			ShowOnuFilterSAMacByVty( phyPonId, userOnuId, vty );
	}
	else
    { 
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
		{
			userOnuId = (short int)(ulOnuId - 1);
			ShowOnuFilterSAMacByVty1( phyPonId, userOnuId, vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}

	return( CMD_SUCCESS );

}
#endif

#ifdef  ONU_IP_AND_PORT_FILTER

/*   added by chenfj 2007-6-8 
        增加ONU 数据流IP/PORT过滤 */

DEFUN( 
	onu_Ip_filter_config,
	onu_Ip_filter_config_cmd,
	"onu [src-ip|dst-ip] filter <onuid_list> <A.B.C.D>",
	"Create a filter rule\n"
	"onu packet ip Address option:source ip\n"
	"onu packet ip Address option:destination ip\n"
	"onu packet ip address filter\n"
	OnuIDStringDesc
	"Please input the IP address\n"
	/*"any: any destination IP address\n"*/
	)
    {
    	/*
	LONG FilterFlag ;
	*/
	unsigned int  IpFilter= 0 ;
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	int count = 0;	
	int count = 0;
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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
	else if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( V2R1_GetLongFromIpdotstring( argv[2], &IpAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}

	if( argc == 3 ) 
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
 			count++;
	
			userOnuId = (short int)(ulOnuId - 1);
			if ( IpFilter ==  V2R1_ONU_FILTER_IP_SOURCE  )
				ret = AddOnuSIpFilter( phyPonId, userOnuId , IpAddr );
			else if ( IpFilter ==  V2R1_ONU_FILTER_IP_DEST)
				ret = AddOnuDIpFilter( phyPonId, userOnuId , IpAddr );
			if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_IP_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}			
		END_PARSE_ONUID_LIST_TO_ONUID();
        
        if ( error_count == count )
        {
            if ( empty_count == count )
            {
        		vty_out( vty, "  %% onu not exist.\r\n");
            }
            else if ( (exist_count == count)
                    || ((exist_count + empty_count) == count) )
            {
        		vty_out( vty, "  %% the ipAddr 0x%8x is already in filter table.\r\n", IpAddr);
            }
            else
            {
        		vty_out( vty, "  %% set onu ip filter err\r\n");
            }

    	   	return CMD_WARNING;
        }
	}
#if 0
	else {  /* 作用于所有ONU */
		if( FilterFlag == V2R1_ENABLE )
			{
			if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
				{
				for ( i = 0; i< MAXONUPERPON; i++)
					AddOnuSIpFilter( phyPonId, i , IpAddr );
				}
			else if( IpFilter == V2R1_ONU_FILTER_IP_DEST )
				{
				for ( i = 0; i< MAXONUPERPON; i++)
					AddOnuDIpFilter( phyPonId, i , IpAddr );
				}
			}
		
		else if( FilterFlag == V2R1_DISABLE )
			{

			}
		}
#endif	
	return  CMD_SUCCESS;
}

DEFUN( 
	undo_onu_Ip_filter_config,
	undo_onu_Ip_filter_config_cmd,
	/*"access-list [permit|deny] ip [dip|sip] [<A.B.C.D>|any] {<onu_list>}*1",*/
	"undo onu [src-ip|dst-ip] filter <onuid_list> {<A.B.C.D>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"onu packet ip Address option:source ip\n"
	"onu packet ip Address option:destination ip\n"
	"onu packet ip address filter\n"
	OnuIDStringDesc
	"Please input the IP address\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpFilter= 0 ;
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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
	else if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )
		IpFilter =  V2R1_ONU_FILTER_IP_DEST ; 
	else 
		{
		vty_out( vty, " %% src-ip|dst-ip Parameter input err\r\n");
		return( CMD_WARNING);
		}

	if( argc == 3 )
		{
		if( V2R1_GetLongFromIpdotstring( argv[2], &IpAddr )  != ROK )
			{
			vty_out( vty, " %% IpAddr Parameter err\r\n");
			return( CMD_WARNING);
			}
		}

	if( argc == 3 ) /* 作用于ONU 上ACL 列表中指定节点*/
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
 			count++;
		
			userOnuId = (short int)(ulOnuId - 1);
			if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
				ret = ClearOnuSIpFilter( phyPonId, userOnuId , IpAddr );
			else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
				ret = ClearOnuDIpFilter( phyPonId, userOnuId , IpAddr );
			if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_IP_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}			
		END_PARSE_ONUID_LIST_TO_ONUID();
        
        if ( error_count == count )
        {
            if ( empty_count == count )
            {
        		vty_out( vty, "  %% onu not exist.\r\n");
            }
            else if ( (exist_count == count)
                    || ((exist_count + empty_count) == count) )
            {
        		vty_out( vty, "  %% the ipAddr 0x%8x is not in filter table.\r\n", IpAddr);
            }
            else
            {
        		vty_out( vty, "  %% clear onu ip filter err\r\n");
            }

    	   	return CMD_WARNING;
        }
	}
	else
    {  
        /* 作用于ONU 上ACL 列表中所有节点*/
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
			userOnuId = (short int)(ulOnuId - 1);
			if( IpFilter == V2R1_ONU_FILTER_IP_SOURCE )
				ret = ClearOnuSIpFilterAll( phyPonId, userOnuId );
			else if( IpFilter == V2R1_ONU_FILTER_IP_DEST)
				ret = ClearOnuDIpFilterAll( phyPonId, userOnuId );
		}			
		END_PARSE_ONUID_LIST_TO_ONUID();
	}

	return  CMD_SUCCESS;
}

DEFUN( 
	onu_SIpUdp_filter_config,
	onu_SIpUdp_filter_config_cmd,
	"onu udp filter <onuid_list> src-port <0-65535>",
	"Create a filter rule\n"
	"create source udp port filter\n"
	"onu packet source udp port filter\n"
	OnuIDStringDesc
	/*
	"source ip address\n"
	"please input source IP address\n"
	*/
	"source udp port\n"
	"please input source udp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int udp_port;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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
#endif

	udp_port = ( unsigned short int ) VOS_AtoL(argv[1]);
	
	IpAddr = INVALID_IP;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
        count++;
    
		userOnuId = (short int)(ulOnuId - 1);
		ret = AddOnuSIpUdpFilter( phyPonId, userOnuId , IpAddr, udp_port );				
		if ( 0 != ret )
        {
            error_count++;
            switch(ret)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_FILTER_IP_UDP_EXIST:
                    exist_count++;
                    break;
            }
        }         
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the source udp-port %d is already in filter table.\r\n", udp_port);
        }
        else
        {
    		vty_out( vty, "  %% set onu source udp port filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_SIpUdp_filter_config,
	undo_onu_SIpUdp_filter_config_cmd,
	/*"undo access-list [permit|deny] <onuid_list> udp src-ip {<A.B.C.D> src-port <0-65535>}*1",*/
	"undo onu udp filter <onuid_list> {src-port <0-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear source udp port filter\n"
	"onu packet source udp port filter\n"
	OnuIDStringDesc
	/*
	"source ip address\n"
	"please input source IP address\n"
	*/
	"source udp port\n"
	"please input source udp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int udp_port;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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

	if( argc == 2 )
	{
		udp_port = ( unsigned short int ) VOS_AtoL(argv[1]);
	}

	IpAddr = INVALID_IP;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
        count++;
    
		userOnuId = (short int)(ulOnuId - 1);
		if( argc == 1 )   /* 参数中没有特定的UDP 端口*/
		{
			ret = ClearOnuSIpUdpFilterAll( phyPonId, userOnuId );
		}	
		else if( argc ==2)  /* 参数中有特定的UDP 端口*/
		{
			ret = ClearOnuSIpUdpFilter(phyPonId,  userOnuId, IpAddr, udp_port);
    		if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_IP_UDP_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}				
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the source udp-port %d is not in filter table.\r\n", udp_port);
        }
        else
        {
    		vty_out( vty, "  %% clear onu source udp port filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN( 
	onu_SIpTcp_filter_config,
	onu_SIpTcp_filter_config_cmd,
	"onu tcp filter <onuid_list> src-port <0-65535>",
	"Create a filter rule\n"
	"create source tcp port filter\n"
	"onu packet source tcp port filter\n"
	OnuIDStringDesc
	/*
	"source ip address\n"
	"please input source IP address\n"
	*/
	"source tcp port\n"
	"please input source tcp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int tcp_port;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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
#endif

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[1]);
	
	IpAddr = INVALID_IP;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
        count++;
    
		userOnuId = (short int)(ulOnuId - 1);
		ret = AddOnuSIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port );				
		if ( 0 != ret )
        {
            error_count++;
            switch(ret)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_FILTER_IP_TCP_EXIST:
                    exist_count++;
                    break;
            }
        }         
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the source tcp-port %d is already in filter table.\r\n", tcp_port);
        }
        else
        {
    		vty_out( vty, "  %% set onu source tcp port filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return  CMD_SUCCESS;
}

DEFUN( 
	undo_onu_SIpTcp_filter_config,
	undo_onu_SIpTcp_filter_config_cmd,
	/*"undo access-list [permit|deny] <onuid_list> udp src-ip {<A.B.C.D> src-port <0-65535>}*1",*/
	"undo onu tcp filter <onuid_list> {src-port <0-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear source tcp port filter\n"
	"onu packet source tcp port filter\n"
	OnuIDStringDesc
	/*
	"source ip address\n"
	"please input source IP address\n"
	*/
	"source tcp port\n"
	"please input source tcp port\n"
	)
    {
	/*LONG FilterFlag ;*/
	unsigned int  IpAddr;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int tcp_port;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

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

	if( argc == 2 )
		{
		tcp_port = ( unsigned short int ) VOS_AtoL(argv[1]);
		}

	IpAddr = INVALID_IP;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
        count++;
    
		userOnuId = (short int)(ulOnuId - 1);

		if( argc == 1 )   /* 参数中没有特定的TCP 端口*/
		{
			ret = ClearOnuSIpTcpFilterAll( phyPonId, userOnuId );
		}
		else if( argc ==2)  /* 参数中有特定的TCP 端口*/
		{
			ret = ClearOnuSIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
    		if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_IP_TCP_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}				
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the source tcp-port %d is not in filter table.\r\n", tcp_port);
        }
        else
        {
    		vty_out( vty, "  %% clear onu source tcp port filter err\r\n");
        }

	   	return CMD_WARNING;
    }	

	return CMD_SUCCESS;
}

DEFUN( 
	show_onu_IPUDPTCP_filter_config,
	show_onu_IPUDPTCP_filter_config_cmd,
	"show onu [src-ip|dst-ip|src-udp|src-tcp|vlanid|ethtype|iptype] filter {<onuid_list>}*1",
	"show onu filter table\n"
	"show onu filter table\n"
	"show onu src-ip filter table\n"
	"show onu dst-ip filter table\n"
	"show onu src-udp filter table\n"
	"show onu src-tcp filter table\n"
	"show onu vlanId filter table\n"
	"show onu ether type filter table\n"
	"show onu ip protocol type filter table\n"
	"show onu filter table\n"
	OnuIDStringDesc
	)
    {
	LONG FilterFlag ;

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int count = 0;
	int OnuBaseEntry;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if ( !( VOS_StrCmp( argv[ 0 ] , "src-ip" ) ) )
		{
		FilterFlag = 1 ; 
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "dst-ip" ) ) )
		{
		FilterFlag = 2;
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "src-udp" ) ) )
		{
		FilterFlag = 3;
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "src-tcp" ) ) )
		{
		FilterFlag = 4;
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "vlanid" ) ) )
		{
		FilterFlag = 5;
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "ethtype" ) ) )
		{
		FilterFlag = 6;
		}
	else if ( !( VOS_StrCmp( argv[ 0 ] , "iptype" ) ) )
		{
		FilterFlag = 7;
		}
	else {
		vty_out( vty, " %% Parameter err\r\n");
		return( CMD_WARNING);
		}

	OnuBaseEntry = phyPonId * MAXONUPERPON;
	if( argc == 1 )   /* 显示所有ONU过滤*/
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId++)
		{
			if( ThisIsValidOnu( phyPonId, userOnuId ) !=  ROK ) 
				continue;
		
			if( FilterFlag == 1 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp )
					ShowOnuSIpFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 2 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_DIp )
					ShowOnuDIpFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 3 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp_udp )
					ShowOnuUdpFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 4 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp_tcp )
					ShowOnuTcpFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 5 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Vid )
					ShowOnuVlanIdFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 6 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Vid )
					ShowOnuEtherTypeFilterByVty( phyPonId, userOnuId, vty );
			}
			else if( FilterFlag == 7 )
			{
				if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Vid )
					ShowOnuIpTypeFilterByVty( phyPonId, userOnuId, vty );
			}			
		}
	}
	else if( argc ==2)  /* 显示指定ONU过滤*/
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
 			{
 			count++;
			}	
		END_PARSE_ONUID_LIST_TO_ONUID();

		if( count == 1 )  /* 参数中只有一个ONU */
		{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
			{
				userOnuId = (short int)(ulOnuId - 1);
				
				if( ThisIsValidOnu( phyPonId, userOnuId ) != ROK ) 
				{
					vty_out(vty, "  onu %d/%d/%d not exist\r\n", slotId, port, ulOnuId );
					RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
				}
				if( FilterFlag == 1 )
				{
					ShowOnuSIpFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 2 )
				{
					ShowOnuDIpFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 3 )
				{
					ShowOnuUdpFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 4 )
				{
					ShowOnuTcpFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 5 )
				{
					ShowOnuVlanIdFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 6 )
				{
					ShowOnuEtherTypeFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 7 )
				{
					ShowOnuIpTypeFilterByVty( phyPonId, userOnuId, vty );
				}
			}
			END_PARSE_ONUID_LIST_TO_ONUID();			
		}
		else 
		{  /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
			{
				userOnuId = (short int)(ulOnuId - 1);
				if( ThisIsValidOnu( phyPonId, userOnuId ) != ROK ) continue;
				if( FilterFlag == 1 )
				{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp )
						ShowOnuSIpFilterByVty( phyPonId, userOnuId, vty );
				}
				else if( FilterFlag == 2 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_DIp )
						ShowOnuDIpFilterByVty( phyPonId, userOnuId, vty );
					}
				else if( FilterFlag == 3 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp_udp )
						ShowOnuUdpFilterByVty( phyPonId, userOnuId, vty );
					}
				else if( FilterFlag == 4 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_SIp_tcp )
						ShowOnuTcpFilterByVty( phyPonId, userOnuId, vty );
					}
				else if( FilterFlag == 5 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Vid )
						ShowOnuVlanIdFilterByVty( phyPonId, userOnuId, vty );
					}
				else if( FilterFlag == 6 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Ether_Type )
						ShowOnuEtherTypeFilterByVty( phyPonId, userOnuId, vty );
					}
				else if( FilterFlag == 7 )
					{
					if( OnuMgmtTable[OnuBaseEntry + userOnuId ].Filter_Ip_Type == NULL ) continue;
						ShowOnuIpTypeFilterByVty( phyPonId, userOnuId, vty );
					}
			}
			END_PARSE_ONUID_LIST_TO_ONUID();
		}		
	}	
	return CMD_SUCCESS;
}
#endif

#if 0

DEFUN( 
	onu_DIpUdp_filter_config,
	onu_DIpUdp_filter_config_cmd,
	"access-list [permit|deny] <onuid_list> udp dst-ip <A.B.C.D> dst-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
	"IP protocol type is udp\n"
	"Configure destination IP address\n"
	"Please input the IP address\n"
	"Please input the destination udp port\n"
	"Please input the destination udp port\n"
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc != 4 )/* ||( argc > 5)*/)
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
		vty_out( vty, " %% permit|deny Parameter err\r\n");
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

	udp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		
		else {  /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
				AddOnuDIpUdpFilter( phyPonId, userOnuId , IpAddr, udp_port);
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_DIpUdp_filter_config,
	undo_onu_DIpUdp_filter_config_cmd,
	"undo access-list [permit|deny] <onuid_list> udp dst-ip {<A.B.C.D> dst-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc  != 2 ) && ( argc !=4) )
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
	
	if( argc == 4 )
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
		
		udp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及UDP 端口*/
					{
					ret = ClearOnuDIpUdpFilterAll( phyPonId, userOnuId );
					}
				
				else if( argc ==4 )  /* 参数中有特定的IP地址及UDP 端口*/
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}

		else {
			 /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及UDP 端口*/
					{
					ret = ClearOnuDIpUdpFilterAll( phyPonId, userOnuId );					
					}
				
				else if( argc == 4 )
					{
					ret = ClearOnuDIpUdpFilter(phyPonId,  userOnuId, IpAddr, udp_port);
					}
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	onu_SIpTcp_filter_config,
	onu_SIpTcp_filter_config_cmd,
	"access-list [permit|deny] <onuid_list> tcp src-ip <A.B.C.D> src-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc != 4 )/* ||( argc > 5)*/)
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
		vty_out( vty, " %% permit|deny Parameter err\r\n");
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

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		
		else {  /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
				AddOnuSIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port);
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_SIpTcp_filter_config,
	undo_onu_SIpTcp_filter_config_cmd,
	"undo access-list [permit|deny] <onuid_list> tcp src-ip {<A.B.C.D> src-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc  != 2 ) && ( argc !=4) )
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
		vty_out( vty, " %% permit|deny Parameter err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 4 )
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
		
		tcp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及TCP 端口*/
					{
					ret = ClearOnuSIpTcpFilterAll( phyPonId, userOnuId );
					}
				
				else if( argc ==4 )  /* 参数中有特定的IP地址及TCP 端口*/
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}

		else {
			 /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及TCP 端口*/
					{
					ret = ClearOnuSIpTcpFilterAll( phyPonId, userOnuId );					
					}
				
				else if( argc == 4 )
					{
					ret = ClearOnuSIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
					}
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	onu_DIpTcp_filter_config,
	onu_DIpTcp_filter_config_cmd,
	"access-list [permit|deny] <onuid_list> tcp dst-ip <A.B.C.D> dst-port <0-65535>",
	"Create an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc != 4 )/* ||( argc > 5)*/)
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
		vty_out( vty, " %% permit|deny Parameter err\r\n");
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

	tcp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )  /* permit / deny */
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		
		else {  /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);
				AddOnuDIpTcpFilter( phyPonId, userOnuId , IpAddr, tcp_port);
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
			}
		}
	
	else if( FilterFlag == V2R1_DISABLE )
		{

		}

	return lRet;
}

DEFUN( 
	undo_onu_DIpTcp_filter_config,
	undo_onu_DIpTcp_filter_config_cmd,
	"undo access-list [permit|deny] <onuid_list> tcp dst-ip {<A.B.C.D> dst-port <0-65535>}*1",
	"clear an access-list\n"
	"clear an access-list\n"
	"Input permit to allow the packets matching the conditions to pass\n"
	"Input deny not to allow the packets matching the conditions to pass\n"
	OnuIDStringDesc
	"IP protocol type is TCP\n"
	"Configure source IP address\n"
	"Please input the IP address\n"
	"Configure dest tcp port\n"
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

	int count = 0;	

	ulIfIndex = ( ULONG ) ( vty->index ) ;	   

	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)(slotId-1), (short  int)(port-1) );
	
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif      
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}	

        if ( ( argc  != 2 ) && ( argc !=4) )
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
		vty_out( vty, " %% permit|deny Parameter err\r\n");
		return( CMD_WARNING);
		}
	
	if( argc == 4 )
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
		
		tcp_port = ( unsigned short int ) VOS_AtoL(argv[3]);
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
		{
		count++;
		}	
	END_PARSE_ONUID_LIST_TO_ONUID();

	if( FilterFlag == V2R1_ENABLE )
		{
		if( count == 1 )  /* 参数中只有一个ONU */
			{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1 ], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及TCP 端口*/
					{
					ret = ClearOnuDIpTcpFilterAll( phyPonId, userOnuId );
					}
				
				else if( argc ==4 )  /* 参数中有特定的IP地址及TCP 端口*/
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
			END_PARSE_ONUID_LIST_TO_ONUID();
			}

		else {
			 /* 参数中有多个ONU */
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], ulOnuId )
				{
				userOnuId = (short int)(ulOnuId - 1);

				if( argc == 2 )   /* 参数中没有有特定的IP地址及TCP 端口*/
					{
					ret = ClearOnuDIpTcpFilterAll( phyPonId, userOnuId );					
					}
				
				else if( argc == 4 )
					{
					ret = ClearOnuDIpTcpFilter(phyPonId,  userOnuId, IpAddr, tcp_port);
					}
				}
			END_PARSE_ONUID_LIST_TO_ONUID();
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
	onu_vlanId_filter_config,
	onu_vlanId_filter_config_cmd,
	"onu vlanid filter <onuid_list> <1-4095>",
	"Create a filter rule\n"
	"create vlanid filter\n"
	"onu packet vlanid  filter\n"
	OnuIDStringDesc
	"please input vid value\n"
	)
    {

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int vlanId;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	vlanId = ( unsigned short int ) VOS_AtoL(argv[1]);
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
        count++;
    
		userOnuId = (short int)(ulOnuId - 1);
		ret = AddOnuVlanIdFilter( phyPonId, userOnuId , vlanId );				
		if ( 0 != ret )
        {
            error_count++;
            switch(ret)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_FILTER_VLAN_ID_EXIST:
                    exist_count++;
                    break;
            }
        }         
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the vlanId %d is already in filter table.\r\n", vlanId);
        }
        else
        {
    		vty_out( vty, "  %% set onu vlanId filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_vlanId_filter_config,
	undo_onu_vlanId_filter_config_cmd,
	"undo onu vlanid  filter <onuid_list> {<0-4095>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear vlanid filter\n"
	"onu packet vlanid filter\n"
	OnuIDStringDesc
	"please input vid value\n"
	)
    {
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int vlanId;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 2 )
		{
		vlanId = ( unsigned short int ) VOS_AtoL(argv[1]);
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
		count++;
	
		userOnuId = (short int)(ulOnuId - 1);
		if( argc == 1 )   /* 参数中没有特定的vid */
		{
			ret = ClearOnuVlanIdFilterAll( phyPonId, userOnuId );
		}
		else if( argc ==2)  /* 参数中有特定的vid */
		{
			ret = ClearOnuVlanIdFilter(phyPonId,  userOnuId, vlanId );
    		if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_VLAN_ID_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}				
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the vlanId %d is not in filter table.\r\n", vlanId);
        }
        else
        {
    		vty_out( vty, "  %% clear onu vlanId filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}
#endif
#ifdef  ONU_ETHER_TYPE_and_IP_PROTOCOL_FILTER

/** added by chenfj 2007-6-15 
       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤*/

DEFUN( 
	onu_EtherType_filter_config,
	onu_EtherType_filter_config_cmd,
	"onu ethertype filter <onuid_list> <eth_type>",
	"Create a filter rule\n"
	"create ether type filter\n"
	"onu packet ether type filter\n"
	OnuIDStringDesc
	"please input ether type value,note: 0xabcd(hex valule),abcd(decimal value)\n"
	)
    {

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int  EtherType;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	/* modified by chenfj 2007-8-20
		urTracker问题单#4819:
		对于基于EtherType过滤数据【源MAC地址后的类型域】，
		建议命令参数改成16进制
		*/
	
	/*EtherType = ( unsigned short int ) VOS_AtoL(argv[1]);	*/

	if(( VOS_MemCmp( argv[1], "0x", 2) == 0) || ( VOS_MemCmp( argv[1], "0X", 2) == 0) )
		{
		EtherType = ( unsigned short int )VOS_StrToUL( argv[1], NULL, 16 );
		/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
		}
	else {
		EtherType = (unsigned short int )strtol(argv[1], NULL, 10 );
		/*sys_console_printf(" eth type 0x%x\r\n", EtherType );*/
		}
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
		count++;
	
		userOnuId = (short int)(ulOnuId - 1);
		ret = AddOnuEtherTypeFilter( phyPonId, userOnuId , EtherType );				
		if ( 0 != ret )
        {
            error_count++;
            switch(ret)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_FILTER_ETHER_TYPE_EXIST:
                    exist_count++;
                    break;
            }
        }         
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the EtherType 0x%x is already in filter table.\r\n", EtherType);
        }
        else
        {
    		vty_out( vty, "  %% set onu EtherType filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_EtherType_filter_config,
	undo_onu_EtherType_filter_config_cmd,
	"undo onu ethertype filter <onuid_list> {<eth_type>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear ether type filter\n"
	"onu packet ehter type filter\n"
	OnuIDStringDesc
	"please input ether type value,note: 0xabcd(hex valule),abcd(decimal value)\n"
	)
    {
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int EtherType;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 2 )
		{
		/* modified by chenfj 2007-8-20
		urTracker问题单#4819:
		对于基于EtherType过滤数据【源MAC地址后的类型域】，
		建议命令参数改成16进制
		*/
	
		/*EtherType = ( unsigned short int ) VOS_AtoL(argv[1]);	*/

		if(( VOS_MemCmp( argv[1], "0x", 2) == 0) || ( VOS_MemCmp( argv[1], "0X", 2) == 0) )
			{
			EtherType = ( unsigned short int )VOS_StrToUL( argv[1], NULL, 16 );
			}
		else {
			EtherType = (unsigned short int )strtol(argv[1], NULL, 10 );
			}
	
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
		count++;
	
		userOnuId = (short int)(ulOnuId - 1);
		if( argc == 1 )   /* 参数中没有特定的Ether Type */
		{
			ret = ClearOnuEtherTypeFilterAll( phyPonId, userOnuId );
		}
		else if( argc ==2)  /* 参数中有特定的EtherType */
		{
			ret = ClearOnuEtherTypeFilter(phyPonId,  userOnuId, EtherType );
    		if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_ETHER_TYPE_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}				
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the EtherType 0x%x is not in filter table.\r\n", EtherType);
        }
        else
        {
    		vty_out( vty, "  %% clear onu EtherType filter err\r\n");
        }

	   	return CMD_WARNING;
    }
    
	return CMD_SUCCESS;
}

DEFUN( 
	onu_IpType_filter_config,
	onu_IpType_filter_config_cmd,
	"onu iptype filter <onuid_list> <1-65535>",
	"Create a filter rule\n"
	"create ip protocol type filter\n"
	"onu packet ip protocol type filter\n"
	OnuIDStringDesc
	"please input ip protocol type\n"
	)
    {

	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int  IpType;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	IpType = ( unsigned short int ) VOS_AtoL(argv[1]);
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
		count++;
	
		userOnuId = (short int)(ulOnuId - 1);
		ret = AddOnuIpTypeFilter( phyPonId, userOnuId , IpType );				
		if ( 0 != ret )
        {
            error_count++;
            switch(ret)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;
                    break;
                case V2R1_ONU_FILTER_IP_PROT_EXIST:
                    exist_count++;
                    break;
            }
        }         
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the Ip protocol type 0x%x is already in filter table.\r\n", IpType);
        }
        else
        {
    		vty_out( vty, "  %% set onu IP protocol type filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN( 
	undo_onu_IpType_filter_config,
	undo_onu_IpType_filter_config_cmd,
	"undo onu iptype filter <onuid_list> {<1-65535>}*1",
	"clear a filter rule\n"
	"clear a filter rule\n"
	"clear ip protocol type filter\n"
	"onu packet ip protocol filter\n"
	OnuIDStringDesc
	"please input ip protocol type\n"
	)
    {
	/*ULONG ulIfIndex = 0;	*/
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	UINT16 userOnuId = 0;

	int ret;
	unsigned short int IpType;

	int count = 0;	
	int	empty_count = 0;
	int	exist_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 2 )
		{
		IpType = ( unsigned short int ) VOS_AtoL(argv[1]);
		}

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0 ], ulOnuId )
	{
		count++;
	
		userOnuId = (short int)(ulOnuId - 1);
		if( argc == 1 )   /* 参数中没有特定的IP  protocol type */
		{
			ret = ClearOnuIpTypeFilterAll( phyPonId, userOnuId );
		}
		else if( argc ==2)  /* 参数中有特定的IP protocol type */
		{
			ret = ClearOnuIpTypeFilter(phyPonId,  userOnuId, IpType );
    		if ( 0 != ret )
            {
                error_count++;
                switch(ret)
                {
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;
                        break;
                    case V2R1_ONU_FILTER_IP_PROT_NOT_EXIST:
                        exist_count++;
                        break;
                }
            }         
		}				
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();
	
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% the Ip protocol type 0x%x is not in filter table.\r\n", IpType);
        }
        else
        {
    		vty_out( vty, "  %% clear onu IP protocol type filter err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}
#endif
/******************* added end ************************/

#if  0
DEFUN  (
    pon_max_mac_llid_config,
    pon_max_max_llid_config_cmd,
    "max-mac <1-128> logical-link <1-64>",
    "Config the max-mac number of the pon\n"
    "Please input the max-mac number of the pon\n"
    "Config the logical link\n"
    "Please input the  onuId \n"
    "Please input the logical linkID of the onu \n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
    INT16   llidIndex = 0;
    UINT32 number = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    number = ( ULONG ) VOS_AtoL( argv[ 0 ] );

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
       userOnuId = (( UINT16 ) VOS_AtoL( argv[ 1 ] )) - 1;
	if(( userOnuId > CLI_EPON_ONUMAX) /*|| ( userOnuId < CLI_EPON_ONUMIN)*/ )
	{
       #ifdef CLI_EPON_DEBUG
   	   vty_out(vty, "  %% DEBUG: phyPonId %d  userOnuId %d\r\n",phyPonId, userOnuId);
       #endif 	
	    vty_out( vty, "  %% onu Parameter is error.\r\n" );
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
   	   vty_out( vty, "  %% slot/port/onu %d/%d/%d is not exist.\r\n",slotId,phyPonId, userOnuId) ;
   	return CMD_WARNING;
       }
       else if (VOS_ERROR == lRet)
       {
          vty_out( vty, "  %% Parameter is error.\r\n" );
   	   return CMD_WARNING;
       }
	#endif
	llidIndex = CLI_EPON_DEFAULTLLID;	
	if (llidIndex != CLI_EPON_DEFAULTLLID)
	{
	   vty_out( vty, "  %% Parameter is error. llid must be 1\r\n");
	   return CMD_WARNING;
	}
	 lRet = SetOnuMaxMacNum( (short int )phyPonId, (short int )userOnuId, (short int )llidIndex, number);	
        if (lRet != VOS_OK)
        {
            vty_out( vty, "  %% Executing error.\r\n" );
    	    return CMD_WARNING;
        }	

    return CMD_SUCCESS;
}

DEFUN  (
    pon_max_mac_config,
    pon_max_max_config_cmd,
    "max-mac <1-128> logical-link all",
    "Config the max-mac number of the pon\n"
    "Please input the max-mac number of the pon\n"
    "Config the logical link\n"
    "All the logical link\n"
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
    number = ( ULONG ) VOS_AtoL( argv[ 0 ] );

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
	 for (userOnuId = 0; userOnuId <= CLI_EPON_ONUMAX; userOnuId++)
	 	{
		#if 0
       	    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
       	 lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
       	 if ( CLI_EPON_ONUUP== lRet)
 		   continue;  			
		#endif
		 lRet = SetOnuMaxMacNum( (short int )phyPonId, (short int )userOnuId, (short int )0, number);
	       /* lRet = SetOnuMaxMacNumAll( (short int )phyPonId , number);*/
	        if (lRet != VOS_OK)
	          {
	        #ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d  number %d\r\n",phyPonId, number);
	        #endif           
	            vty_out( vty, "  %% Executing error.\r\n" );
	    	    return CMD_WARNING;
	          }	
	 	}
    return CMD_SUCCESS;
}

/*使能/去使能地址学习功能此处函数
SetPonPortMacAutoLearningCtrl() 有问题*/
DEFUN  (
    pon_mac_learn_config,
    pon_mac_learn_config_cmd,
    "mac-learning [enable|disable]",
    "Config the mac learning ability of the pon\n"
    "Enable the mac learning ability\n"
    "Disable the mac learning ability\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    unsigned int CtrlValue = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ; 
	
    if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
    {
        CtrlValue = CLI_EPON_LEARNING_ENABLE;
    }
    else if ( !VOS_StrCmp( argv[ 0 ], "disable" ) )
    {
         CtrlValue = CLI_EPON_LEARNING_DISABLE;
    }
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    lRet = SetPonPortMacAutoLearningCtrl( (short int )phyPonId,  CtrlValue );	
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d\r\n",phyPonId);
    #endif        
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}

#endif

/* 设置老化时间*/
DEFUN  (
    pon_mac_age_config,
    pon_mac_age_config_cmd,
    "aging-mac-time <5-86400>",
    "Config the mac aging time of the pon\n"
    "Aging time(second),Value should be multiple of 5 seconds\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;	
    ULONG onuId = 0; 
    INT16 phyPonId = 0;	
    UINT32 AgeingTime = 0;

	
    AgeingTime = ( UINT32 ) VOS_AtoL( argv[ 0 ] );
    if ((AgeingTime<5) || (AgeingTime > 86400))
	{
    	vty_out( vty, "  %% Parameter is error.\r\n" );
    	return CMD_WARNING;
	}
	if((AgeingTime % 5 ) != 0 )
    {
    	vty_out( vty, "  %% aging-time should be multiple of 5 seconds\r\n" );
    	return CMD_WARNING;
    }

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

    lRet = SetPonPortMacAgeingTime( (short int )phyPonId,  AgeingTime*1000 ) ;
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
    	return CMD_WARNING;
    }

    return CMD_SUCCESS;
}

/*显示PON 地址表老化时间*/
DEFUN  (
    pon_show_mac_age_config,
    pon_show_mac_age_config_cmd,
    "show aging-mac-time",
    DescStringCommonShow
    "show the mac aging time(s) of the pon\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;	
    ULONG onuId = 0; 
    INT16 phyPonId = 0;	
    UINT32 AgeingTime = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

    lRet = GetPonPortMacAgeingTime( phyPonId,  &AgeingTime );
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
    	return CMD_WARNING;
    }
    
    vty_out(vty, "  aging-mac-time %d ( s)\r\n\r\n", AgeingTime/1000);

    return CMD_SUCCESS;
}

#if 0
DEFUN  (
    pon_show_onu_regiseter_limit_config,
    pon_show_onu_regiseter_limit_config_cmd,
    "show register limit-onu ",
    DescStringCommonShow
    "onu register config\n"
    "onu limit register config\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    ULONG ulIfIndex = 0;
    int  cliflag	 = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	
	lRet = GetOnuRegisterLimitFlag(phyPonId, &cliflag );
    if (lRet == VOS_ERROR)
         {
           #ifdef CLI_EPON_DEBUG
       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
           #endif     
           	vty_out( vty, "  %% Executing error.\r\n");
       	return CMD_WARNING;
         }
	
	if (cliflag == CLI_EPON_ONULIMIT_ENABLE)
	{
		vty_out( vty, "\r\n  register limit-onu is enable\r\n");
	}
	else if (cliflag == CLI_EPON_ONULIMIT_DISABLE)
	{
		vty_out( vty, "\r\n  register limit-onu is disable\r\n");
	}
	else
         {
           #ifdef CLI_EPON_DEBUG
       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
           #endif     
           	vty_out( vty, "  %% Executing error.\r\n");
       	return CMD_WARNING;
         }
	
    return CMD_SUCCESS;
}

/*已经注释该cli 命令，不提供
增加fdb*/
DEFUN  (
    pon_fdb_mac_config,
    pon_fdb_mac_config_cmd,
    "fdbentry mac <H.H.H> logical-link <1-64>",
    "Create a permanent FDB entry \n"
    "MAC address\n"
    "Please input MAC address \n"
    "Logical link associated with MAC address \n"
    "Please input the onuId \n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    UCHAR MacAddr[6] = {0,0,0,0,0,0};
	
    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }
    userOnuId = (( INT16 ) VOS_AtoL( argv[ 1 ] )) - 1;

    ulIfIndex = ( ULONG ) ( vty->index ) ;  
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
    #endif      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
    #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif
	vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, userOnuId+1) ;
	return CMD_WARNING;
    }
    else if (VOS_ERROR == lRet)
    {
       vty_out( vty, "  %% Parameter is error.\r\n" );
	 return CMD_WARNING;
    }	

    lRet = SetOnuStaticMacAddr( phyPonId, userOnuId, MacAddr);
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d userOnuId %d.\r\n",phyPonId,userOnuId);
    #endif       
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }
          
    return CMD_SUCCESS;
}
#endif

#define CLIE_BANDWITHDELAY_LOW	1
#define CLIE_BANDWITHDELAY_HIGH	2
/* 对PLATO DBA V3, 增加fixed-bw参数*/
#ifdef  PLATO_DBA_V3
DEFUN  (
    pon_bandwidth_config,
    pon_bandwidth_config_cmd,
    /*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
    "bandwidth class <0-7> delay [high|low] {fixed-bw <0-10000000>}*1 assured-bw <64-10000000> best-effort-bw <64-10000000> [up|down] <onuid_list>",
    "Config onu's bandwidth\n"
    "Config onu`s bandwidth class\n"
    "Please input class\n"
    "Config onu`s bandwidth delay\n"
    "Delay high\n"
    "Delay low\n"
    "Config onu`s fixed bandwidth\n"
    "Please input the bandwidth(unit:Kbit/s),when <1000,should be 64*n\n"
    "Config onu`s assured bandwidth\n"
    "Please input the bandwidth(unit:Kbit/s),when <1000,should be 64*n;down direction,for pas5001,Max BW is 500000kbit/s,for pas5201,max BW is 1000000kbit/s\n"
    "Config onu`s best-effort bandwidth\n"
    "Please input the bandwidth(unit:Kbit/s), when <1000,should be 64*n;and should be no smaller than assured-bw.down direction,for pas5001,Max BW is 500000kbit/s,for pas5201,max BW is 1000000kbit/s\n"
    "The up direction\n"
    "down direction\n"
    OnuIDStringDesc
    )
#else
DEFUN  (
    pon_bandwidth_config,
    pon_bandwidth_config_cmd,
    "bandwidth class <0-7> delay [high|low] assured-bw <64-1000000> best-effort-bw <64-1000000> [up|down] <onuid_list>",
    "Config onu's bandwidth\n"
    "Config onu`s bandwidth class\n"
    "Please input class\n"
    "Config onu`s bandwidth delay\n"
    "Delay high\n"
    "Delay low\n"
    "Config onu`s assured bandwidth\n"
    "Please input the bandwidth(unit:Kbit/s),when <1000,should be 64*n;down direction,for pas5001,Max BW is 500000kbit/s,for pas5201,max BW is 1000000kbit/s\n"
    "Config onu`s best-effort bandwidth\n"
    "Please input the bandwidth(unit:Kbit/s), when <1000,should be 64*n;and should be no smaller than assured-bw.down direction,for pas5001,Max BW is 500000kbit/s,for pas5201,max BW is 1000000kbit/s\n"
    "The up direction\n"
    "down direction\n"
    OnuIDStringDesc
    )

#endif
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    unsigned int direction = 0;
    short int chipType = 0;
	short int PonPortIdx = 0;
	unsigned int fixedBw = 0;
	unsigned int assBw = 0;
	unsigned int bestEfBw = 0;
	unsigned int bwClass = 0;
	unsigned int bwDelay = CLIE_BANDWITHDELAY_LOW;
	int		count;
	int		empty_count;
	int		error_count;
	int fixedBw_Exist = 0;
	int exceedBw_Exist = 0;

	ULONG defUBw = DEFAULT_UP_BW, defDBw = DEFAULT_DOWN_BW;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 7 )
		fixedBw_Exist = 1;
	
	bwClass = ( unsigned int ) VOS_AtoL( argv[ 0 ] );
	
    if ( !VOS_StrCmp( argv[ 1 ], "high" ) )
    {
        bwDelay = CLIE_BANDWITHDELAY_HIGH;
    }
    else /*if ( !VOS_StrCmp( argv[ 1 ], "low" ) )*/
    {
        bwDelay = CLIE_BANDWITHDELAY_LOW;
    }
	
	
	if( fixedBw_Exist == 1 )
	{
		fixedBw = ( UINT32 ) VOS_AtoL( argv[ 2 ] );
		if( fixedBw < 1000 )
        {
            if (0 != (fixedBw % CLI_EPON_BANDWIDTH_MIN))
            {
            	vty_out( vty, "  %% when fixed-bw < 1000, should be 64*n\r\n" );
            	return CMD_WARNING;
            }
        }		 
	}
   	
    assBw = ( UINT32 ) VOS_AtoL( argv[ 2+fixedBw_Exist ] );
	
    if (assBw < CLI_EPON_BANDWIDTH_MIN)
    {
    	vty_out( vty, "  %% Parameter is error.assured-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
    }
	/* 该带宽粒度为256的倍数*/

	if(assBw<1000)
	{
		if (0 != (assBw % CLI_EPON_BANDWIDTH_MIN))
	    {
	    	vty_out( vty, "  %% when Assured-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
	    }
	}

	bestEfBw = ( UINT32 ) VOS_AtoL( argv[ 3+fixedBw_Exist ] );
	
#ifdef PLATO_DBA_V3
	if(bestEfBw < (assBw+fixedBw))
#else
	if(bestEfBw < assBw )
#endif
	{
		vty_out( vty, "  %% Parameter error. Best-effort-bw must be greater than or equal to ");
#ifdef PLATO_DBA_V3
		vty_out(vty,"fixed-bw+");
#endif
		vty_out(vty,"assured-bw\r\n");
		return CMD_WARNING;
	}
		
    if (bestEfBw < CLI_EPON_BANDWIDTH_MIN)
    {
    	vty_out( vty, "  %% Parameter is error.Best-effort-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
    }
	/* 该带宽粒度为256的倍数*/
	if(bestEfBw<1000)
	{
		if (0 != (bestEfBw % CLI_EPON_BANDWIDTH_MIN) )
	    {
	    	vty_out( vty, "  %% when Best-effort-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
	    }
	}
	
    if ( !VOS_StrCmp(argv[4+fixedBw_Exist], "up"))
    {
    	direction = CLI_EPON_UPLINK;
    }
    else 
    {
    	direction = CLI_EPON_DOWNLINK;
    }

	/* added by xieshl 20151225,  上下行不对称时，应分开检查，问题单28412 */
	PonPortIdx = GetPonPortIdxBySlot(slotId, port);
	if (PonPortIdx == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	Pon_GetSupportMaxBw(PonPortIdx, &defUBw, &defDBw);
	if( ((direction == CLI_EPON_UPLINK) && ((fixedBw > defUBw) || (assBw > defUBw) || (bestEfBw > defUBw))) ||
		((direction == CLI_EPON_DOWNLINK) && ((fixedBw > defDBw) || (assBw > defDBw) || (bestEfBw > defDBw))) )
	{
	    vty_out( vty, "  %% Parameter is error. The %s-bindwidth should not exceed %dkbit/s.\r\n", 
			argv[4+fixedBw_Exist], ((direction == CLI_EPON_UPLINK) ? defUBw : defDBw) );
		return CMD_WARNING;
	}

	count = 0;
	empty_count = 0;
	error_count = 0;
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 5+fixedBw_Exist ], onuId )
 	{
 		count++;
        
 		userOnuId = ( short int )(onuId - 1);
		if (direction == CLI_EPON_UPLINK)
		{
#ifdef PLATO_DBA_V3
			lRet = SetOnuUplinkBW_2( phyPonId, userOnuId, bwClass, bwDelay, fixedBw,assBw, bestEfBw );           
#else
 			lRet = SetOnuUplinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );
#endif
		}
		else if(direction == CLI_EPON_DOWNLINK)
		{
			lRet = SetOnuDownlinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );
			
		}
		
        /* B--modified by liwei056@2009-1-13 for D9425 */
        if ( lRet != VOS_OK )
        {
            error_count++;
            switch (lRet)
            {
                case V2R1_EXCEED_RANGE:
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
                    if ( CLI_EPON_DOWNLINK == direction )
                    {
                        --error_count;
                        if ( 0 == exceedBw_Exist++ )
                        {
            	 			vty_out( vty, "  Warning: PON%d/%d's max-downlink-bandwidth is exceeded from the beginning of onu %d/%d/%d.\r\n",slotId,port,slotId,port,onuId);
                        }
                    }
                    else
#endif
                    {
        	 			vty_out( vty, "  %% Not enough bandwidth.\r\n" );
        	 		   	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
                    }

                    break;
				/*Begin:for onu swap by jinhl@2013-04-27*/
				case V2R1_Parter_EXCEED_RANGE:
					--error_count;
					#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
                    if ( CLI_EPON_DOWNLINK == direction )
                    {
                        --error_count;
                        if ( 0 == exceedBw_Exist++ )
                        {
            	 			vty_out( vty, "  Warning: The partern's port max-downlink-bandwidth is exceeded from the beginning .\r\n");
                        }
                    }
                    else
                    #endif
                    {
        	 			vty_out( vty, "  %% The partern's port has not enough bandwidth.\r\n" );
        	 		   	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
                    }

                    break;
				/*End:for onu swap by jinhl@2013-04-27*/
                case V2R1_ONU_NOT_EXIST:
                    empty_count++;    
                    break;
                case PLATO3_ECODE_ILLEGAL_FIXED_BW:
    	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal fixed-bandwidth.\r\n",slotId,port,onuId);
                    break;
				case TK_DBA_ECODE_FIXED_BW:
					vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA, BCM55524 card's fixed-bandwidth should be no greater than %dkbit/s.\r\n",slotId,port,onuId,uplinkBWPacketUnitSize*32);
                    break;
                case PLATO3_ECODE_ILLEGAL_GR_BW:
    	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal assured-bandwidth.\r\n",slotId,port,onuId);
                    break;
                case PLATO3_ECODE_ILLEGAL_BE_BW:
    	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal best-effort-bandwidth.\r\n",slotId,port,onuId);
                    break;
                default:
    	 		    vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current executing error(%d).\r\n",slotId,port,onuId, lRet);
            }
        }
        /* E--modified by liwei056@2009-1-13 for D9425 */
	}
	END_PARSE_ONUID_LIST_TO_ONUID();	

    /* B--added by liwei056@2010-11-18 for D11184 */
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }

	   	return CMD_WARNING;
    }
    /* E--added by liwei056@2010-11-18 for D11184 */

	chipType = GetPonChipTypeByPonPort( phyPonId );
	if( OLT_PONCHIP_ISPAS(chipType) ) 
	{
    	if ( direction == CLI_EPON_DOWNLINK )
    	{
#ifdef PLATO_DBA_V3
    		if( fixedBw_Exist == 1 )
    			vty_out( vty, "  Note: class, delay, fixed-bw, best-effort-bw on the onu is inactive.\r\n");
    		else
                vty_out( vty, "  Note: class, delay, best-effort-bw on the onu is inactive.\r\n");
#else 
    		vty_out( vty, "  Note: class, delay, best-effort-bw on the onu is inactive.\r\n");
#endif
    	}
	}
    else if ( OLT_PONCHIP_ISTK(chipType) )
    {
    	if ( direction == CLI_EPON_DOWNLINK )
    	{
    		if( fixedBw_Exist == 1 )
    			vty_out( vty, "  Note: delay, fixed-bw on the onu is inactive.\r\n");
    		else
                vty_out( vty, "  Note: delay on the onu is inactive.\r\n");
    	}
    }
    else
    {
        /* 主控板启动时，进入此处，因为其未执行PON板的插入操作所致 */
        /* VOS_ASSERT(0); */
    }

	return CMD_SUCCESS;
}

DEFUN  (
    undo_pon_bandwidth_config,
    undo_pon_bandwidth_config_cmd,
    "undo bandwidth [up|down] {<onuid_list>}*1",
    "delete onu's information\n"
    "delete onu`s bandwidth config information\n"
    "The up direction\n"
    "down direction\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    unsigned int direction = 0;
	unsigned int fixedBw = 0;
	unsigned int assBw = 0;
	unsigned int bestEfBw = 0;
	unsigned int bwClass = 2;
	unsigned int bwDelay = CLIE_BANDWITHDELAY_LOW;
	unsigned int exceedBw_Exist = 0;
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

    if ( !VOS_StrCmp( argv[0], "up" ) )
    {
        direction = CLI_EPON_UPLINK;
    }
    else /*if ( !VOS_StrCmp( argv[0], "down" ) )*/
    {
        direction = CLI_EPON_DOWNLINK;
    }
    /*else
        {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
        }*/
	
	if( argc == 2)
	{
        int		count;
        int		empty_count;
        int		error_count;

        count = 0;
        empty_count = 0;
        error_count = 0;

        BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], onuId )
        {
            count++;
        
        	userOnuId = ( short int )(onuId - 1);
        if (direction == CLI_EPON_UPLINK)
#ifdef PLATO_DBA_V3
        	lRet = SetOnuUplinkBW_2( phyPonId, userOnuId, bwClass, bwDelay, fixedBw,assBw, bestEfBw );
#else
    		lRet = SetOnuUplinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );
#endif
        else if(direction == CLI_EPON_DOWNLINK)
        	lRet = SetOnuDownlinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );

            if ( lRet != VOS_OK )
            {
                error_count++;
                switch (lRet)
                {
                    case V2R1_EXCEED_RANGE:
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
                    if ( CLI_EPON_DOWNLINK == direction )
                    {
                        --error_count;
                        if ( 0 == exceedBw_Exist++ )
                        {
            	 			vty_out( vty, "  Warning: PON%d/%d's max-downlink-bandwidth is exceeded from the beginning of onu %d/%d/%d.\r\n",slotId,port,slotId,port,onuId);
                        }
                    }
                    else
#endif
                    {
        	 			vty_out( vty, "  %% Not enough bandwidth.\r\n" );
        	 		   	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
                    }

                    break;
					/*Begin:for onu swap by jinhl@2013-04-27*/
					case V2R1_Parter_EXCEED_RANGE:
						--error_count;
						#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
	                    if ( CLI_EPON_DOWNLINK == direction )
	                    {
	                        --error_count;
	                        if ( 0 == exceedBw_Exist++ )
	                        {
	            	 			vty_out( vty, "  Warning: The partern's port max-downlink-bandwidth is exceeded from the beginning .\r\n");
	                        }
	                    }
	                    else
	                    #endif
	                    {
	        	 			vty_out( vty, "  %% The partern's port has not enough bandwidth.\r\n" );
	        	 		   	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
	                    }

	                    break;
				/*End:for onu swap by jinhl@2013-04-27*/
                    case V2R1_ONU_NOT_EXIST:
                        empty_count++;    
                        break;
                    case PLATO3_ECODE_ILLEGAL_FIXED_BW:
        	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal fixed-bandwidth.\r\n",slotId,port,onuId);
                        break;
                    case PLATO3_ECODE_ILLEGAL_GR_BW:
        	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal assured-bandwidth.\r\n",slotId,port,onuId);
                        break;
                    case PLATO3_ECODE_ILLEGAL_BE_BW:
        	 			vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current illegal best-effort-bandwidth.\r\n",slotId,port,onuId);
                        break;
                    default:
        	 		    vty_out( vty, "  %% onu %d/%d/%d failed to setup DBA for the current executing error.\r\n",slotId,port,onuId);
                }
            }

        }
        END_PARSE_ONUID_LIST_TO_ONUID();	
    
        /* B--added by liwei056@2010-11-18 for D11184 */
        if ( error_count == count )
        {
            if ( empty_count == count )
            {
        		vty_out( vty, "  %% onu not exist.\r\n");
            }

    	   	return CMD_WARNING;
        }
        /* E--added by liwei056@2010-11-18 for D11184 */
	}
	else
    {
    	for(userOnuId=0;userOnuId<MAXONUPERPON;userOnuId++)
    	{
    		if(ThisIsValidOnu(phyPonId, userOnuId) == ROK )
    		{
    			if (direction == CLI_EPON_UPLINK)
#ifdef PLATO_DBA_V3
    				lRet = SetOnuUplinkBW_2( phyPonId, userOnuId, bwClass, bwDelay, fixedBw,assBw, bestEfBw );
#else
    	 			lRet = SetOnuUplinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );
#endif
    			else if(direction == CLI_EPON_DOWNLINK)
    				lRet = SetOnuDownlinkBW_1( phyPonId, userOnuId, bwClass, bwDelay, assBw, bestEfBw );
    		
		            if ( lRet != VOS_OK )
		            {
		                switch (lRet)
		                {
		                    case V2R1_EXCEED_RANGE:
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
                            if ( CLI_EPON_DOWNLINK == direction )
                            {
                                if ( 0 == exceedBw_Exist++ )
                                {
                    	 			vty_out( vty, "  Warning: PON%d/%d's max-downlink-bandwidth is exceeded from the beginning of onu %d/%d/%d.\r\n",slotId,port,slotId,port,onuId);
                                }
                            }
                            else
#endif
                            {
                	 			vty_out( vty, "  %% Not enough bandwidth.\r\n" );
                	 		   	return CMD_WARNING;
                            }

                            break;
		                    case V2R1_ONU_NOT_EXIST:
		        				vty_out( vty, "  %% onu %d/%d/%d not exist.\r\n",slotId,port,onuId);
		                        break;
		                    case PLATO3_ECODE_ILLEGAL_FIXED_BW:
		        	 			vty_out( vty, "  %% Failed to setup DBA for the current illegal fixed-bandwidth.\r\n" );
		                        break;
		                    case PLATO3_ECODE_ILLEGAL_GR_BW:
		        	 			vty_out( vty, "  %% Failed to setup DBA for the current illegal assured-bandwidth.\r\n" );
		                        break;
		                    case PLATO3_ECODE_ILLEGAL_BE_BW:
		        	 			vty_out( vty, "  %% Failed to setup DBA for the current illegal best-effort-bandwidth.\r\n" );
		                        break;
		                    default:
		        	 		    vty_out( vty, "  %% Failed to setup DBA for the current executing error.\r\n");
		                }
		            }
    			}
    	}
    	
    }	
    
	return CMD_SUCCESS;
}



#if 0



/*该命令的输入参数需要进行修改,看隋平理礼提供*/
DEFUN  (
    pon_bandwidth_config,
    pon_bandwidth_config_cmd,
    "bandwidth <256-1000000> [up|down] <onuid_list>",
    "Config pon's bandwidth\n"
    "Please input the bandwidth(Kbit/s, 256, 512, >1000),but max downlink bandwidth is 500000Kbit/s\n"
    "The up direction Max bandwidth is 1000000kbit/s\n"
    "The down direction Max bandwidth is 500000kbit/s\n"
    "Please input the onuId\n"
    "All onus\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    INT16 userOnuId = 0;
    INT32	direction = 0;
    INT32	bandWidth = 0;
	int		count = 0;

	ulIfIndex = ( ULONG ) ( vty->index ) ; 
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d userOnuId %d direction %d bandWidth %d.\r\n",phyPonId, userOnuId, direction, bandWidth);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	
    bandWidth = ( UINT32 ) VOS_AtoL( argv[ 0 ] );
	
    if (bandWidth < CLI_EPON_BANDWIDTH_MIN)
    	{
    	vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
    	}
	/* 该带宽粒度为256的倍数*/

	if(bandWidth<1000)
	{
		if (0 != bandWidth % CLI_EPON_BANDWIDTH_MIN)
	    	{
	    	vty_out( vty, "  %% bandwidth must be 256, 512, >1000.\r\n" );
			return CMD_WARNING;
	    	}
	}
    if ( !VOS_StrCmp(argv[1], "up"))
    	{
    	direction = CLI_EPON_UPLINK;
    	}
    else if ( !VOS_StrCmp(argv[1], "down"))
    	{
    	if(bandWidth > CLI_EPON_DOWN_MAX_BANDWIDTH)
    		{
		vty_out( vty, "  %% Parameter error. Max downlink bandwidth is 500000kbit/s\r\n" );
		return( CMD_WARNING );
    		}
    	direction = CLI_EPON_DOWNLINK;
    	}
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 2 ], onuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
	if (count == 1)
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 2 ], onuId )
	 	{
			userOnuId = ( short int )(onuId - 1);
			if (direction == CLI_EPON_UPLINK)
	 			lRet = SetOnuUplinkBW( phyPonId, userOnuId, bandWidth );
			else if(direction == CLI_EPON_DOWNLINK)
				lRet = SetOnuDownlinkBW( phyPonId, userOnuId, bandWidth );
			
			if (lRet == CLI_EPON_VOS_RESERVE)
	 		{
	 			vty_out( vty, "  %% Not enough bandwidth.\r\n" );
	 			return CMD_WARNING;
	 		}	
			else if (lRet == VOS_ERROR)
	 		{
	 		    vty_out( vty, "  %% Executing error.\r\n");
	 		   	return CMD_WARNING;
	 		}
			else if ( lRet == 3 )
			{
				vty_out( vty, "  %% onu %d/%d/%d not exist.\r\n",slotId,port,onuId);
				return CMD_WARNING;
			}
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		return CMD_SUCCESS;
	}
	else
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 2 ], onuId )
	 	{
	 		userOnuId = ( short int )(onuId - 1);
			if (direction == CLI_EPON_UPLINK)
	 			lRet = SetOnuUplinkBW( phyPonId, userOnuId, bandWidth );
			else if(direction == CLI_EPON_DOWNLINK)
				lRet = SetOnuDownlinkBW( phyPonId, userOnuId, bandWidth );
			
			if (lRet == CLI_EPON_VOS_RESERVE)
		 	{
		 		vty_out( vty, "  %% Not enough bandwidth.\r\n" );
		 		return CMD_WARNING;
		 	}	
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
		return CMD_SUCCESS;
	}

	#if 0
    if ( !VOS_StrCmp( argv[ 2 ], "all" ) )
    {
	obj = CLI_EPON_ALLLLID;	
    }
    else 
    {
       obj = CLI_EPON_SINGLELLID;
	    userOnuId = (( INT16 ) VOS_AtoL( argv[ 2 ] )) -1;
		if( (userOnuId > (CLI_EPON_ONUMAX)) || (userOnuId < (CLI_EPON_ONUMIN)))
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d userOnuId %d direction %d bandWidth %d.\r\n",phyPonId, userOnuId, direction, bandWidth);
		#endif
		    vty_out( vty, "  %% Parameter is error.\r\n" );
		    return CMD_WARNING;
		}
    }
	
    if (CLI_EPON_UPLINK == direction)
    {
    	if (CLI_EPON_ALLLLID == obj)
    	{
    	    /*lRet = SetOnuUpLinkBWAll( (short int )phyPonId, bandWidth);*/
	    for (OnuIdx = 0; OnuIdx<= CLI_EPON_ONUMAX; OnuIdx++)
	    	{
	    	    lRet = SetOnuUplinkBW( phyPonId, OnuIdx, bandWidth );
				if (lRet == CLI_EPON_VOS_RESERVE)
	 		    {
	 		        vty_out( vty, "  %% Not enough bandwidth.\r\n" );
	 		    	 return CMD_WARNING;
	 		    }	
			   else if (lRet == VOS_ERROR)
	 		    {
	 		        vty_out( vty, "  %% %s/port%d onu%d setting uplink bw is wrong.\r\n",CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId),(OnuIdx+1) );
	 		    	/* return CMD_WARNING; deleted by chenfj 2006-10-31 */
	 		    }	
	    	}			
    	}

		if (CLI_EPON_SINGLELLID ==obj )
		{
		    lRet = SetOnuUplinkBW( (short int )phyPonId, (short int )userOnuId, bandWidth );
			if (lRet == CLI_EPON_VOS_RESERVE)
		 		{
		 		    vty_out( vty, "  %% Not enough bandwidth.\r\n" );
		 		    return CMD_WARNING;
		 		}	
			else if (lRet == VOS_ERROR)
		 		{
		 		    vty_out( vty, "  %% Executing error.\r\n" );
		 		    return CMD_WARNING;
		 		}				
		}	
   }
    else if (CLI_EPON_DOWNLINK == direction)
    {
    	if (CLI_EPON_ALLLLID == obj)
    	{
    	    /*lRet = SetOnuDownLinkBWAll( (short int )phyPonId, bandWidth);*/
	    for (OnuIdx = 0; OnuIdx<= CLI_EPON_ONUMAX; OnuIdx++)
	    	{
	    	    /*if (CLI_EPON_ONUUP != GetOnuOperStatus( phyPonId, OnuIdx))
					continue;*/
	    	    lRet = SetOnuDownlinkBW( phyPonId, OnuIdx, bandWidth );
				if (lRet == CLI_EPON_VOS_RESERVE)
			 		{
			 		    vty_out( vty, "  %% Not enough bandwidth.\r\n" );
			 		    return CMD_WARNING;
			 		}	
				else if (lRet == VOS_ERROR)
			 		{
			 		    vty_out( vty, "  %% %s/port%d onu%d setting downlink bw is wrong.\r\n",CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId),(OnuIdx+1) );
			 		    /*return CMD_WARNING; deleted by chenfj 2006-10-31 */
			 		}	
	    	}
    	}

		if (CLI_EPON_SINGLELLID ==obj )
		{
		    lRet = SetOnuDownlinkBW( (short int )phyPonId, (short int )userOnuId, bandWidth );
			if (lRet == CLI_EPON_VOS_RESERVE)
		 		{
		 		    vty_out( vty, "  %% Not enough bandwidth.\r\n" );
		 		    return CMD_WARNING;
		 		}	
			else if (lRet == VOS_ERROR)
		 		{
		 		    vty_out( vty, "  %% Executing error.\r\n" );
		 		    return CMD_WARNING;
		 		}				
		}
   }
	#endif

}

DEFUN  (
    pon_aps_config,
    pon_aps_config_cmd,
    "aps [auto|forced|disable]",
    "Config pon's aps attribute\n"
    "Auto asp\n"
    "Forced asp\n"
    "Disable asp\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT32 apsDone = 0;

    if ( !VOS_StrCmp(argv[0], "auto"))
    	{
    	apsDone = CLI_EPON_AUTO;
    	}
    else if ( !VOS_StrCmp(argv[0], "forced"))
    	{
    	apsDone = CLI_EPON_FORCE;
    	}
    else if ( !VOS_StrCmp(argv[0], "disable"))
    	{
    	apsDone = CLI_EPON_DISABLE;
    	}	
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }       

    ulIfIndex = ( ULONG ) ( vty->index ) ;      
	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d apsDone %d.\r\n",phyPonId, apsDone);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    lRet = SetPonPortApsCtrl( (short int )phyPonId, apsDone );	
	
    if (lRet != VOS_OK)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d apsDone %d.\r\n",phyPonId, apsDone);
	#endif    
        vty_out( vty, "  %% Executing error.\r\n" );
    	 return CMD_WARNING;
    }		
    return CMD_SUCCESS;
}


/*说明: 该命令暂不使用,已被注销*/
DEFUN  (
    pon_onu_register_mac_config,
    pon_onu_register_mac_config_cmd,
    "onu-register-mac <H.H.H> [valid|invalid]",
    "Config pon's onu mac\n"
    "Please input the mac address\n"
    "The mac is valid\n"
    "The mac is invalid\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG ponId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    CHAR  MacAddr[6] = {0};
    ulIfIndex = ( ULONG ) ( vty->index ) ;          

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
    {
    #ifdef CLI_EPON_DEBUG
	 vty_out(vty, "  %% DEBUG: phyPonId %d  mac : 0x%x.",phyPonId, MacAddr[0]);
	{
		int i = 1;
		for(; i < 6;i++)
			vty_out(vty, "-0x%x",MacAddr[i]);
		vty_out(vty, ".\r\n");
	}
    #endif
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }  
	
    if ( !VOS_StrCmp(argv[1], "invalid"))
    {
    	lRet = AddInvalidOnuMac( ponId, MacAddr );
    }
    else if ( !VOS_StrCmp(argv[1], "valid"))
    {
    	lRet = DelInvalidOnuMac( ponId, MacAddr);
    }
    else
    {
    #ifdef CLI_EPON_DEBUG
	 vty_out(vty, "  %% DEBUG: phyPonId %d  parameter %s",phyPonId, argv[1]);
    #endif    
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    } 
		
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
	 vty_out(vty, "  %% DEBUG: phyPonId %d  mac : 0x%x.",phyPonId, MacAddr[0]);
	{
		int i = 1;
		for(; i < 6;i++)
			vty_out(vty, "-0x%x",MacAddr[i]);
		vty_out(vty, ".\r\n");
	}
    #endif    
        vty_out( vty, "  %% Executing error.\r\n" );
    	 return CMD_WARNING;
    }	
	
    return CMD_SUCCESS;
}


/*=======================================*/

/*该命令已被屏蔽,pon上不支持没链路最大mac地址设置*/
DEFUN  (
    pon_no_max_mac_config,
    pon_no_max_mac_config_cmd,
    "undo max-mac logical-link [<1-64>|all]",
    NO_STR
    "Config the max-mac number of the pon\n"
    "Config the logical link\n"
    "Please input the  ID of the logical link\n"
    "All the logical link\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    INT32	obj = 0;
	
    if ( !VOS_StrCmp( argv[ 0 ], "all" ) )
    {
	obj = CLI_EPON_ALLLLID;	
    }
    else 
    {
       obj = CLI_EPON_SINGLELLID;
       userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
    }        

    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
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
    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
    #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif
	vty_out( vty, "  %% slot/port/onu %d/%d/%d is not exist.\r\n",slotId, phyPonId, userOnuId) ;
	return CMD_WARNING;
    }
    else if (VOS_ERROR == lRet)
    {
    #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif    
       vty_out( vty, "  %% Parameter is error.\r\n" );
	 return CMD_WARNING;
    }	
	#endif
    if (CLI_EPON_ALLLLID == obj)
    {
       lRet = CancelOnuMaxMacNumAll( (short int )phyPonId );
    }
    else if ( CLI_EPON_SINGLELLID == obj)
    {
      	lRet = CancelOnuMaxMacNum( (short int )phyPonId, (short int )userOnuId);
    }
     if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
       vty_out( vty, "  %% phyPonId %d userOnuId %d", phyPonId, userOnuId);
    #endif
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	    	
	
    return CMD_SUCCESS;
}


DEFUN  (
    pon_no_onu_regiseter_limit_config,
    pon_no_onu_regiseter_limitconfig_cmd,
    "undo register limit-onu ",
    NO_STR
    "onu register config\n"
    "Set onu auto register config\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    ULONG ulIfIndex = 0;
    int  cliflag	 = 0;

    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	   

	cliflag = CLI_EPON_ONULIMIT_DISABLE;
    lRet = SetOnuRegisterLimitFlag( phyPonId, cliflag );	
    if (lRet != VOS_OK)
         {
           #ifdef CLI_EPON_DEBUG
       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
           #endif     
           	vty_out( vty, "  %% Executing error.\r\n");
       	return CMD_WARNING;
         }
	
    return CMD_SUCCESS;
}





/*该命令目前不提供,已被注销*/
DEFUN  (
    pon_no_onu_register_mac_config,
    pon_no_onu_register_mac_config_cmd,
    "undo onu-register-mac <H.H.H>",
    NO_STR
    "Config pon's onu mac\n"
    "Please input the mac address\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }  

    lRet = DelInvalidOnuMac( phyPonId, MacAddr);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	      
    return CMD_SUCCESS;
}
#endif

#ifdef  __PON_MAC_TABLE_ENTRY
#endif
/*显示pon 的mac地址*/
DEFUN  (
    pon_show_fdb_mac_config,
    pon_show_fdb_mac_config_cmd,
		"show fdbentry mac {onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">}*1",
    DescStringCommonShow
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    "the specific onu\n"
    "the onu id\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    /*ULONG ulIfIndex = 0;	*/
    INT16 phyPonId = 0;
	LONG lRet = VOS_OK;
		
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	if( argc == 0 )
	{
		lRet = ShowPonMacLearningByVty( phyPonId, MAXONUPERPON, vty );
		/*if (CMD_SUCCESS != lRet)
		 vty_out(vty, "  %% Executing error.\r\n");*/
	}
	else
	{
		onuId = (ULONG )VOS_AtoL( argv[ 0 ] ) -1;
        CHECK_CMD_ONU_RANGE(vty, onuId);		
		lRet = ShowPonMacLearningByVty( phyPonId, onuId, vty );
		/*if (CMD_SUCCESS != lRet)
		 vty_out(vty, "  %% Executing error.\r\n");*/
	}
	/*ShowPonPortOnLineOnuByVty( phyPonId, vty );*/
	return CMD_SUCCESS;
		
}

/* 
	增加命令, 用于显示某个指定的MAC 地址是从哪个PON 口的哪个ONU上学到的.
 	此命令在config , view 和pon 节点下都注册
 */
extern LONG GetMacAddr1( CHAR * szStr, CHAR * pucMacAddr );
DEFUN  (
	pon_show_fdb_mac_form_which_onu,
	pon_show_fdb_mac_form_which_onu_cmd,
	"show fdbentry mac <H.H.H>",
	DescStringCommonShow
	"Show fdbentry \n"
	"Show fdbentry MAC address\n"
	"the specific mac address\n"
	)
{
	ULONG slotId = 0;
	ULONG onuId = 0; 
	ULONG port = 0;	
	/*ULONG ulIfIndex = 0;	*/
	INT16 PonPortIdx = 0;
	unsigned char MacAddress[BYTES_IN_MAC_ADDRESS];

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;    	

	if(GetMacAddr1( (unsigned char * )argv[ 0 ], MacAddress) != VOS_OK )
	{
		vty_out(vty,"input mac error\n");
		return(CMD_WARNING);
	}

	ShowPonMacLearningFromWhichOnuByVty( PonPortIdx,MacAddress,vty );

	return CMD_SUCCESS;

}

DEFUN  (
	config_show_fdb_mac_form_which_onu,
	config_show_fdb_mac_form_which_onu_cmd,
	"show fdbentry mac <H.H.H>",
	DescStringCommonShow
	"Show fdbentry \n"
	"Show fdbentry MAC address\n"
	"the specific mac address\n"
	)
{
	unsigned char MacAddress[BYTES_IN_MAC_ADDRESS];
 
	if(GetMacAddr1( (unsigned char * )argv[ 0 ], MacAddress) != VOS_OK )
	{
		vty_out(vty,"input mac error\n");
		return(CMD_WARNING);
	}

	ShowPonMacLearningFromWhichOnuByVtyAll(MacAddress,vty );

	return CMD_SUCCESS;

}

/*显示pon 学习到的mac地址总数*/
DEFUN  (
    pon_show_fdb_mac_counter,
    pon_show_fdb_mac_counter_cmd,
    "show fdbentry mac counter",
    DescStringCommonShow
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    "show mac entry counter\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    /*ULONG ulIfIndex = 0;	*/
    INT16 phyPonId = 0;
		
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	

	/*deleted by chenfj 2008-8-7 */
	ShowPonMacLearningCounterByVty( phyPonId, vty );
	/*ShowPonPortOnLineOnuByVty( phyPonId, vty );*/
    return CMD_SUCCESS;
		
}

/* added by chenfj 2008-2-28
     在config 节点下, 增加显示PON 端口学习到的MAC 地址
     */
DEFUN(
	config_show_fdb_mac_config,
	config_show_fdb_mac_config_cmd,
		"show fdbentry mac <slot/port> {onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">}*1",
	DescStringCommonShow
	"Show fdbentry \n"
	"Show fdbentry MAC address\n"
	"input pon port slot/port\n"
	"the specific onu\n"
	"the onu id\n"
    )
{
	ULONG ulSlot = 0;
	ULONG onuId = 0; 
	ULONG ulPort = 0;	
	/*ULONG ulIfIndex = 0;	*/
	INT16 phyPonId = 0;


	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	/*
	if ((ulSlot<4)  || (ulSlot>8))
		{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
		}

	if ( (ulPort < 1) || (ulPort > 4) )
		{
		vty_out( vty, "  %% not exist port %d/%d. \r\n",ulSlot, ulPort);
		return CMD_WARNING;
		}
	if(!(( SYS_MODULE_TYPE(ulSlot) ==  MODULE_E_GFA_EPON ) ||( SYS_MODULE_TYPE(ulSlot) <=  MODULE_TYPE_UNKNOW )))
		{
		vty_out(vty,"  %% Error slot %d\r\n", ulSlot );
		return CMD_WARNING;
		}
	*/
	
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return CMD_WARNING;
	
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% PonPortidx error.\r\n" );
		return CMD_WARNING;
	}

	if( argc == 1 )
	{
		/*deleted by chenfj 2008-8-7*/
		ShowPonMacLearningByVty( phyPonId, MAXONUPERPON, vty );
	}
	else
	{
		onuId = (ULONG )VOS_AtoL( argv[ 1 ] ) -1;
        CHECK_CMD_ONU_RANGE(vty, onuId);		
		/*deleted by chenfj 2008-8-7*/
		ShowPonMacLearningByVty( phyPonId, onuId, vty );
	}

	return CMD_SUCCESS;

}

/* B--added by liwei056@2011-3-14 for 武汉长宽 */
/* 显示pon 学习到的onu mac地址数目*/
DEFUN  (
    pon_show_onu_fdb_num,
    pon_show_onu_fdb_num_cmd,
    "show fdbentry onu counter {threshold <1-1000>}*1",
    DescStringCommonShow
    "Show fdbentry \n"
    "Show onu's fdbentry address\n"
    "Show onu's fdbentry address number\n"
    "the minimal onu's mac address number to display\n"
    "input the number of onu's mac address\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
	LONG lRet = VOS_OK;
    short int sOltId;
    short int sNumLimit;
		

    if ( PON_PORT_NODE == vty->node )
    {
    	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &sOltId) != VOS_OK )
    		return CMD_WARNING;    	

    	if( PonPortIsWorking(sOltId) == FALSE ) 
		{
    		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slotId,port);
    		return( ROK );
		}
    }
    else if ( CONFIG_NODE == vty->node || VIEW_NODE == vty->node)
    {
        sOltId = OLT_ID_ALL;
    }
    else
    {
        VOS_ASSERT(0);
    }
    
	if( argc == 0 )
	{
	    sNumLimit = 1;
	}
	else
    {
		sNumLimit = VOS_AtoI( argv[ 0 ] );
	}
    
	lRet = ShowPonMacLearningCounterListByVty( sOltId, sNumLimit, vty );
    
    return CMD_SUCCESS;
}
/* E--added by liwei056@2011-3-14 for 武汉长宽 */
/*找出所有在线GT812中EMAPPER 中的不符合项*/
/*config节点下和pon节点下*/

#if 1	 /* modified by xieshl 20120210, 为支持对812等E2参数在初次注册时的自动更正，统一函数接口*/
extern LONG checkOnuEmapperParameter( short int PonPortIdx, short int OnuIdx, int resetflag );
DEFUN  (
    set_onu_emapper_restore,
    set_onu_emapper_restore_cmd,
    /*"onu-eeprom-mapper-restore {<arbdelta> <txdly> <rxdly>}*1 {[reset]}*1",*/
    "onu-eeprom-mapper-restore {[reset]}*1",
    "set default onu eeprom mapper\n"
   /* "arbdelta range -127~127\n"
    "txdly range -127~127\n"
    "rxdly range -127~127\n"*/
    "reset onu\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    short int sOltId;

    int resetflag=0;  /*不复位*/
    int setcount_total = 0, setcount_ok = 0, setcount_err = 0;
	
    short int OltStartID, OltEndID;
    short int OnuIdx,llid;
    short int OltSlot,OltPort;
    int type=0;
    LONG rc;

    if ( PON_PORT_NODE == vty->node )
    {
    	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &sOltId) != VOS_OK )
    		return CMD_WARNING;    	

    	if( PonPortIsWorking(sOltId) == FALSE ) 
		{
    		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slotId,port);
    		return( ROK );
		}
    }
    else if ( CONFIG_NODE == vty->node )
    {
        sOltId = OLT_ID_ALL;
    }
    else
    {
        VOS_ASSERT(0);
	 return CMD_WARNING;
    }

    if ( OLT_ID_ALL == sOltId )
    {
        OltStartID  = 0;
        OltEndID    = MAXPON;
    }
    else
    {
        OltStartID  = sOltId;
        OltEndID    = sOltId + 1;
    }

    if(argc==1)
    {
        resetflag=1;
    }

    for ( ; OltStartID < OltEndID ; OltStartID++ )
    {
        OltSlot = GetCardIdxByPonChip(OltStartID);
        OltPort = GetPonPortByPonChip(OltStartID);
        
        if( PonPortIsWorking(OltStartID) == FALSE ) 
		{
    		continue;
		}

        for (OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
        {
            llid = GetLlidByOnuIdx(OltStartID, OnuIdx);
	     if( llid == INVALID_LLID ) 
		{
		continue;
		}
            /*else
                {
                   vty_out(vty, "onu  LLID =%d\r\n",llid );
                }*/
    
        	if( GetOnuOperStatus( OltStartID, OnuIdx ) == ONU_OPER_STATUS_UP )
               {
	            setcount_total++;
			   
		     /* GT812A/GT815/GT815_B/GT871/GT871P/GT871R/GT872/GT872P/GT872R/GT873/GT873P/GT873R */
		     /* GT816/812PB/815PB */
                   /*GetOnuType( OltStartID, OnuIdx, &type );
                   if( (type != V2R1_ONU_GT812_A) && (type != V2R1_ONU_GT815) && (type != V2R1_ONU_GT815_B) &&
			   (type != V2R1_ONU_GT871) && (type != V2R1_ONU_GT871_R) && (type != V2R1_ONU_GT872) &&
			   (type != 38) && (type != 39) && (type != V2R1_ONU_GT873) && (type != 41) && (type != 42) && (type != 43) &&
			   (type != V2R1_ONU_GT816) && (type !=V2R1_ONU_GT812_B) && (type != V2R1_ONU_GT815_B) )
                        continue;*/

			rc = checkOnuEmapperParameter( OltStartID, OnuIdx, resetflag );
			if( rc == 0 )
			{
                        vty_out(vty, "set onu %d/%d/%d eeprom mapper sucess!\r\n", OltSlot, OltPort, OnuIdx + 1);
			   setcount_ok++;
			}
			else if( rc == -1 )
			{
                        	setcount_err++;
			}
			else if( rc == -2 )
			{
                        	setcount_err++;
                            vty_out(vty,"onu %d/%d/%d eeprom mapper set failure!\r\n", OltSlot, OltPort, OnuIdx + 1);
			}
			else if( rc == -3 )
			{
				v2r1_printf(vty,"reset onu %d/%d/%d failure\r\n", OltSlot, OltPort, OnuIdx + 1);
			}
                }
            }   
        }

	if( setcount_total )
	{
		if(setcount_total < (setcount_ok + setcount_err) )
			setcount_total = setcount_ok + setcount_err;
		vty_out( vty, "\r\n restore success:%d", setcount_ok );
		if( setcount_err )	vty_out( vty, " failure:%d", setcount_err );
		if( setcount_total > (setcount_ok + setcount_err) )	vty_out( vty, " ignore:%d", (setcount_total-setcount_ok-setcount_err) );
		vty_out( vty, "\r\n check onu eeprom mapper finished\r\n\r\n" );
	}
	else
	{
		vty_out( vty, "not find inline onu\r\n" );
	}
	return CMD_SUCCESS;
}
#else
#define ONU_EPROM_CHECK(irlt)   if ( 0 != (irlt) )  {vty_out(vty,"set onu %d/%d/%d eeprom mapper fail!\r\n",OltSlot, OltPort, OnuIdx + 1);continue;}
DEFUN  (
    set_onu_emapper_restore,
    set_onu_emapper_restore_cmd,
    /*"onu-eeprom-mapper-restore {<arbdelta> <txdly> <rxdly>}*1 {[reset]}*1",*/
    "onu-eeprom-mapper-restore {[reset]}*1",
    "set default onu eeprom mapper\n"
   /* "arbdelta range -127~127\n"
    "txdly range -127~127\n"
    "rxdly range -127~127\n"*/
    "reset onu\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;	
    short int sOltId;

    int resetflag=0;  /*不复位*/
    UCHAR arbdelta=4;
    UCHAR txdly=-6;
    UCHAR rxdly=-1;
	
    UCHAR timestamp_delta,clk_calib_tx,clk_calib_rx;
    ULONG  size=1;
    int setflag = 0;
    int setcount_total = 0, setcount_ok = 0, setcount_err = 0;
	
    short int OltStartID, OltEndID;
    short int OnuIdx,llid;
    short int OltSlot,OltPort;
    int type=0;

    if ( PON_PORT_NODE == vty->node )
    {
    	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &sOltId) != VOS_OK )
    		return CMD_WARNING;    	

    	if( PonPortIsWorking(sOltId) == FALSE ) 
		{
    		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slotId,port);
    		return( ROK );
		}
    }
    else if ( CONFIG_NODE == vty->node )
    {
        sOltId = OLT_ID_ALL;
    }
    else
    {
        VOS_ASSERT(0);
	 return CMD_WARNING;
    }

    if ( OLT_ID_ALL == sOltId )
    {
        OltStartID  = 0;
        OltEndID    = MAXPON;
    }
    else
    {
        OltStartID  = sOltId;
        OltEndID    = sOltId + 1;
    }

    /* if(argc==0)
    {

    }
   else if(argc==3)
    {
        arbdelta=VOS_AtoI( argv[0]);
        txdly=VOS_AtoI( argv[1]);
        rxdly=VOS_AtoI( argv[2]);
    }
    else if(argc==4)
    {
        resetflag=1;
        arbdelta=VOS_AtoI( argv[0]);
        txdly=VOS_AtoI( argv[1]);
        rxdly=VOS_AtoI( argv[2]);
    }
    else  */if(argc==1)
    {
        resetflag=1;
    }

    for ( ; OltStartID < OltEndID ; OltStartID++ )
    {
        OltSlot = GetCardIdxByPonChip(OltStartID);
        OltPort = GetPonPortByPonChip(OltStartID);
        
        if( PonPortIsWorking(OltStartID) == FALSE ) 
		{
    		continue;
		}

        for (OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
        {
            llid = GetLlidByOnuIdx(OltStartID, OnuIdx);
	     if( llid == INVALID_LLID ) 
		{
		continue;
		}
            /*else
                {
                   vty_out(vty, "onu  LLID =%d\r\n",llid );
                }*/
    
        	if( GetOnuOperStatus( OltStartID, OnuIdx ) == ONU_OPER_STATUS_UP )
               {
	            setcount_total++;
			   
                   GetOnuType( OltStartID, OnuIdx, &type );
		     /* GT812A/GT815/GT815_B/GT871/GT871P/GT871R/GT872/GT872P/GT872R/GT873/GT873P/GT873R */
		     /* GT816/812PB/815PB */
                   if( (type != V2R1_ONU_GT812_A) && (type != V2R1_ONU_GT815) && (type != V2R1_ONU_GT815_B) &&
			   (type != V2R1_ONU_GT871) && (type != V2R1_ONU_GT871_R) && (type != V2R1_ONU_GT872) &&
			   (type != 38) && (type != 39) && (type != V2R1_ONU_GT873) && (type != 41) && (type != 42) && (type != 43) &&
			   (type != V2R1_ONU_GT816) && (type !=V2R1_ONU_GT812_B) && (type != V2R1_ONU_GT815_B) )
                        continue;

			timestamp_delta = 0;
		  	clk_calib_tx = 0;
 			clk_calib_rx = 0;
			setflag = 0;
			
                   OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA , &timestamp_delta, &size );
                    if(timestamp_delta!=arbdelta)
                    	{
                        if( OnuMgt_SetOnuI2CInfo(OltStartID, OnuIdx,EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA,&arbdelta,1) != 0 )
                        {
                        	setcount_err++;
				continue;
                        }
			   setflag = 1;
                    	}
                   OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX , &clk_calib_tx, &size );
                   if(clk_calib_tx!=txdly)
                   	{
                        if( OnuMgt_SetOnuI2CInfo(OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX,&txdly,1) != 0 )
                        {
                        	setcount_err++;
				continue;
                        }
			   setflag = 1;
                   	}
                   OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX , &clk_calib_rx, &size );
                   if(clk_calib_rx!=rxdly)
                   	{
                        if( OnuMgt_SetOnuI2CInfo(OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX,&rxdly,1) != 0 )
                        {
                        	setcount_err++;
				continue;
                        }
			   setflag = 1;
                    	}

                   if(resetflag==1)
                   {
                    		timestamp_delta = 0;
		  		clk_calib_tx = 0;
 				clk_calib_rx = 0;

                           OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA , &timestamp_delta, &size );
                           OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_TX , &clk_calib_tx, &size );
                           OnuMgt_GetOnuI2CInfo( OltStartID, OnuIdx, EEPROM_MAPPER_PON_CLK_CALIB_RX , &clk_calib_rx, &size );
				if((timestamp_delta!=arbdelta)&&(clk_calib_tx!=txdly)&&(clk_calib_rx!=rxdly))
                            {
                                  vty_out(vty,"onu %d/%d/%d eeprom mapper set failure!\r\n", OltSlot, OltPort, OnuIdx + 1);
                        	      setcount_err++;
                                  continue;
                            }
                            else
                            {
                                if( setflag )
                                {
                                /*复位ONU*/
                                   if( OnuMgt_ResetOnu( OltStartID, OnuIdx) != 0 )
                            	vty_out(vty,"reset onu %d/%d/%d failure\r\n", OltSlot, OltPort, OnuIdx + 1);
                                }
                            }
                    }
                    if( setflag )
                    {
                        vty_out(vty, "set onu %d/%d/%d eeprom mapper sucess!\r\n", OltSlot, OltPort, OnuIdx + 1);
			   setcount_ok++;
                    }
                }
            }   
        }

	if( setcount_total )
	{
		vty_out( vty, "\r\n check onu eeprom mapper finished\r\n" );
		if(setcount_total < (setcount_ok + setcount_err) )
			setcount_total = setcount_ok + setcount_err;
		vty_out( vty, " restore success:%d, failure:%d, ignore:%d\r\n\r\n", setcount_ok, setcount_err, (setcount_total-setcount_ok-setcount_err) );
	}
	else
	{
		vty_out( vty, "not find inline onu\r\n" );
	}
	return CMD_SUCCESS;
}
#undef  ONU_EPROM_CHECK    
#endif

#if 0
/* 该cli命令已被注释，不提供
增加了onuid 参数，并修改帮助说明。目前我们的onu 只支持一个链路llid*/
DEFUN  (
    pon_no_fdb_mac_config,
    pon_no_fdb_mac_config_cmd,
    "undo fdbentry mac [<H.H.H>|all] logical-link <1-64>",
    NO_STR
    "Config a permanent FDB entry \n"
    "MAC address\n"
    "Please input MAC address \n"
    "All mac\n"
    "Logical link associated with MAC address \n"
    "Please input the OnuId\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    INT16 llidIndex = 0;
    INT16 Done = 0;
    CHAR MacAddr[6] = {0};
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }


    Done = CLI_EPON_DELONE;	
    if ( !VOS_StrCmp(argv[0], "all"))
    {
    	Done = CLI_EPON_DELALL;
    }
    else if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }   
	
    userOnuId = (( INT16 ) VOS_AtoL( argv[ 1 ] )) - 1;

    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
    #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif
	vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, userOnuId+1) ;
	return CMD_WARNING;
    }
    else if (VOS_ERROR == lRet)
    {
    #ifdef CLI_EPON_DEBUG
    	vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
    #endif    
       vty_out( vty, "  %% Parameter is error.\r\n" );
	 return CMD_WARNING;
    }	
	
    llidIndex = CLI_EPON_DEFAULTLLID;
    if (llidIndex != CLI_EPON_DEFAULTLLID)
	{
	   vty_out( vty, "  %% Parameter is error. llid must be 1\r\n");
	   return CMD_WARNING;
	}
	
    if ( CLI_EPON_DELALL == Done )	
    {
    	lRet = DeleteOnuMacAddrAll( phyPonId, userOnuId, llidIndex);
    }
    else if ( CLI_EPON_DELONE == Done )
    {
    	lRet = DeleteOnuMacAddr( phyPonId, userOnuId, llidIndex, MacAddr );
    }
	
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
       vty_out( vty, "  %% phyPonId %d userOnuId %d llidIndex %d mac %x",phyPonId, userOnuId, llidIndex,MacAddr[0] );
	{
		int i = 1;
		for(; i < 6;i++)
			vty_out(vty, "-0x%x",MacAddr[i]);
		vty_out(vty, ".\r\n");
	}
    #endif	
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}

/* 目前不支持,已被注销*/
DEFUN  (
    pon_reset_config,
    pon_reset_config_cmd,
    "reset",
    "Reset the PON\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;

	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	

    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    /*if (VOS_OK != PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&ponId , (ULONG *)&onuId))
	return CMD_WARNING; */
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
	lRet = GetPonPortOperStatus(  phyPonId );
	if(( lRet == PONPORT_DOWN )|| ( lRet == PONPORT_UNKNOWN )||(lRet == PONPORT_INIT)||( lRet == RERROR )) 
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",slotId,port);
		return CMD_WARNING;
	}
    lRet = RestartPonPort( (short int) phyPonId);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	
	
    return CMD_SUCCESS;
}
#endif

DEFUN  (
    pon_update_config,
    pon_update_config_cmd,
    "update pon file",
    "file update command\n"
    "pon file update. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
    "update the pon file. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;    	
	/*
	lRet = GetPonPortOperStatus(  phyPonId );
	if(( lRet == PONPORT_DOWN )|| ( lRet == PONPORT_UNKNOWN )||(lRet == PONPORT_INIT)||( lRet == RERROR )) 
	*/
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slotId) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", slotId);
		return( CMD_WARNING );		}

	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)(slotId),(unsigned char)(port)) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;
#endif

	if(OLTAdv_IsExist(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",slotId,port);
		return CMD_WARNING;
	}
    lRet = UpdatePonFirmware( (short int) phyPonId);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	       
    return CMD_SUCCESS;
}

DEFUN  (
    config_pon_update_config,
    config_pon_update_config_cmd,
    "update pon file <slot/port>",
    "file update command\n"
    "pon file update. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
    "update the pon file. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	"input the slot/port\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;
    INT16 phyPonId = 0;

	VOS_Sscanf(argv[0], "%d/%d", &slotId, &port);

	if( PonCardSlotPortCheckWhenRunningByVty(slotId, port,vty) != ROK )
		return(CMD_WARNING );
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slotId) != ROK)
		{
		vty_out(vty," %% slot %d is not pon card\r\n", slotId);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)(slotId),(unsigned char)(port)) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;
#endif
	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	/*
	lRet = GetPonPortOperStatus(  phyPonId );
	if(( lRet == PONPORT_DOWN )|| ( lRet == PONPORT_UNKNOWN )||(lRet == PONPORT_INIT)||( lRet == RERROR )) 
	*/
	if(OLTAdv_IsExist(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",slotId,port);
		return CMD_WARNING;
	}
    lRet = UpdatePonFirmware( (short int) phyPonId);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	       
    return CMD_SUCCESS;
}


DEFUN  (
    pon_show_bandwidth_config,
    pon_show_bandwidth_config_cmd,
    "show bandwidth",
    DescStringCommonShow
    "Show bandwidth information\n"
    )
{
    /*ULONG ulIfIndex = 0;*/
	ULONG slotId = 0, port = 0, onuId = 0;
    INT16 phyPonId = 0;	

    /*ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }*/
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	 ShowPonPortBWInfoByVty(phyPonId , vty);
    /*ShowPonPortBWInfo( (short int )phyPonId );*/
	
    return CMD_SUCCESS;
}

/*     修改此处,无需输入ponid
DEFUN  (
    pon_show_port_info_config,
    pon_show_port_info_config_cmd,
    "show pon port information <ponid>",
    DescStringCommonShow
    "Show pon information\n"
    "Show pon port information\n"
    "Show pon port information\n"
    "Please input pon ID\n",
    &g_ulIFMQue )
*/


/* modified by chenfj 2008-3-18
     问题单: #6449
     在config节点下，查看PON端口信息时，建议只显示up的PON口

	增加判断:
	   1  对应板是否在位
	   2  是否为PON板?
	   2  PON芯片是否在位
	   3  PON芯片工作状态
 */
DEFUN  (
    pon_show_port_info_config,
    pon_show_port_info_config_cmd,
    "show pon port information",
    DescStringCommonShow
    "Show pon information\n"
    "Show pon port information\n"
    "Show pon port information\n"
    )
{
    INT16 phyPonId = 0;
    /*ULONG ulIfIndex = 0;*/

    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slotId) != ROK)
		{
		vty_out(vty," %% slot %d is not pon card\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 3 PON 芯片在位检查*/
	if( getPonChipInserted( (slotId), (port)) != PONCHIP_EXIST )
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", slotId, port );
		return (CMD_WARNING );
		}
#else
	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;
#endif

	/* 4 PON 芯片工作状态检查*/
	if( GetPonPortAdminStatus(phyPonId) == V2R1_DISABLE )
		{
		vty_out(vty,"\r\n Pon(slot%d)/port%d administrator status is down\r\n", slotId, port );
		return (CMD_WARNING );
		}
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty,"\r\n Pon(slot%d)/port%d is not working\r\n", slotId, port );
		return (CMD_WARNING );
		}

    ShowPonPortInfoByVty( phyPonId,vty );
    return CMD_SUCCESS;
}

DEFUN  (
    olt_show_port_info_config,
    olt_show_port_info_config_cmd,
    "show olt information",
    DescStringCommonShow
    "Olt\n"
    "Show pon port information\n"
    )
{
    INT16 phyPonId = 0;
    /*ULONG ulIfIndex = 0;*/

    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    show_olt_info_banner(vty);
    
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        for(slotId=1;slotId<=PONCARD_LAST;slotId++)
        {
            if(!SYS_MODULE_IS_PON(slotId))
                continue;

            SYS_SLOT_LOOP_PON_PORT_BEGIN(slotId, port)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(slotId, port);
                if(PonPortIdx == RERROR)
                    continue;           

                ShowOltPonInfoByVty( PonPortIdx,vty );
            }
            SYS_SLOT_LOOP_PON_PORT_END()
        }
    }
    else
    {
        SYS_SLOT_LOOP_PON_PORT_BEGIN(SYS_LOCAL_MODULE_SLOTNO, port)
        {
            short int PonPortIdx = GetPonPortIdxBySlot(SYS_LOCAL_MODULE_SLOTNO, port);
            if(PonPortIdx == RERROR)
                continue;           

            ShowOltPonInfoByVty( PonPortIdx,vty );
        }
        SYS_SLOT_LOOP_PON_PORT_END()
    }
    vty_out(vty, "\r\n");
    return CMD_SUCCESS;
}
/*  added by chenfj 2008-2-28
     在CONFIG 节点下增加显示PON 端口信息的命令
     */
DEFUN  (
    show_pon_port_info_config,
    show_pon_port_info_config_cmd,
    "show pon port information {<slot/port>}*1",
    DescStringCommonShow
    "Show pon information\n"
    "Show pon port information\n"
    "Show pon port information\n"
    "Please input pon slot/port\n"
    )
{
	short int phyPonId = 0;
	unsigned long ulSlot, ulPort;

	if( argc == 1 )
	{
		VOS_Sscanf(argv[0], "%d/%d", &ulSlot, &ulPort);
		
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
			return(CMD_WARNING );
#if 0
		/* 1 板在位检查*/
		if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
			{
			vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
			return( CMD_WARNING );
			}
		/* 2 pon 板类型检查*/
		/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
		if(SlotCardIsPonBoard(ulSlot) != ROK )
			{
			vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
			return( CMD_WARNING );
			}
		/* 3 PON 芯片在位检查*/
		if( getPonChipInserted( (ulSlot), (ulPort)) != PONCHIP_EXIST )
			{
			vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
			return (CMD_WARNING );
			}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

		phyPonId = GetPonPortIdxBySlot(ulSlot, ulPort);
		if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
			{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;    	
			}
		
		/* 4 PON 芯片工作状态检查*/
		if( GetPonPortAdminStatus(phyPonId) == V2R1_DISABLE )
			{
			vty_out(vty,"\r\n Pon(slot%d)/port%d administrator status is down\r\n", ulSlot, ulPort );
			return (CMD_WARNING );
			}
		if( PonPortIsWorking(phyPonId) != TRUE )
			{
			vty_out(vty,"\r\n Pon%d/%d is not working\r\n", ulSlot, ulPort );
			return (CMD_WARNING );
			}
	
		ShowPonPortInfoByVty( phyPonId,vty );
	}
	
	else {
		/*for( ulSlot = (PON5+1); ulSlot <= (PON1+1); ulSlot ++)*/
		for(ulSlot = PONCARD_FIRST; ulSlot <= PONCARD_LAST; ulSlot++)
			{
			if(SlotCardIsPonBoard(ulSlot) != ROK) continue;
#if 0
			if ((ulSlot < 4)  || (ulSlot > 8)) continue;
			
			if ((ulSlot == 4) && (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) ))
				{
				if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
					{
					continue;
					}
				}
			/* 1 板在位检查*/
			if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	continue;
			/* 2 pon 板类型检查*/
			if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON ) continue;
#endif		
			for( ulPort = 1; ulPort <= PONPORTPERCARD; ulPort ++)
				{
				/* 3 PON 芯片在位检查*/
				if( getPonChipInserted( (ulSlot), (ulPort)) != PONCHIP_EXIST ) continue;

				phyPonId = GetPonPortIdxBySlot(ulSlot, ulPort);
				if( phyPonId == RERROR ) continue;
				/* 4 PON 芯片工作状态检查*/
				if( GetPonPortAdminStatus(phyPonId) == V2R1_DISABLE )
					{
					vty_out(vty,"\r\n Pon(slot%d)/port%d administrator status is down\r\n", ulSlot, ulPort );
					continue;
					}
				if( PonPortIsWorking(phyPonId) != TRUE )
					{
					vty_out(vty,"\r\n Pon(slot%d)/port%d is not working\r\n", ulSlot, ulPort );
					continue;
					}
				
				ShowPonPortInfoByVty( phyPonId,vty );
				}
			}
		}
    return CMD_SUCCESS;
}
 

int  ShowSingleOnuBandwidthbyVty( short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	short int OnuEntry; 
	short int PonChipType, PonChipVer;
	unsigned int UplinkDelay, UplinkClass, Uplink_gr, Uplink_be;
	unsigned int DownlinkDelay, DownlinkClass, Downlink_gr, Downlink_be;
	
	CHECK_ONU_RANGE
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;

	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) 
		/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr) == ROK ) ||
		( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1) == ROK ))
		*/
	{
		/*vty_out (vty, "\r\n  %s/port%d Onu%d not provisioned\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		*/
		return( V2R1_ONU_NOT_EXIST );	/* modified by xieshl 20100325, 问题单9978 */
	}	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	/*vty_out (vty, "\r\n  %s/port%d Onu%d bandwidth Information(unit:kbit/s) \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	*/
	GetOnuUplinkBW_1(PonPortIdx, OnuIdx, &UplinkClass, &UplinkDelay, &Uplink_gr, &Uplink_be );
	GetOnuDownlinkBW_1(PonPortIdx, OnuIdx, &DownlinkClass, &DownlinkDelay, &Downlink_gr, &Downlink_be );

	/*added by wutw*/
	vty_out (vty, "\r\n  OnuIdx   direction  class   delay   assured-bw   best-effort-bw\r\n");
	
	vty_out (vty, "  %2d       Uplink   ", (OnuIdx + 1 ));

	/*class show*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		vty_out(vty, "  %-2d   ", UplinkClass);
	else vty_out(vty, "  --   ");
	
	/*vty_out(vty, "delay:");*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		if( UplinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   low  ");
		else vty_out(vty, "   high ");
	}
	else vty_out(vty, "   --   ");
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		vty_out(vty, "   %-6d    ", Uplink_gr);
	else vty_out(vty, "   --        ");

	if( OLT_PONCHIP_ISPAS(PonChipType) )
		vty_out(vty, "   %-6d\r\n", Uplink_be );
	else vty_out(vty, "   --    \r\n");

	/*========================================*/
	/*downlink*/
	vty_out(vty, "           Downlink ");
	
	if( PonChipType == PONCHIP_TYPE_MAX )
		vty_out(vty, "  --   ");
	/*其他待定*/
	else vty_out(vty, "  --   ");/*vty_out(vty, "  %2d   ", DownlinkClass);*/
	
	/*vty_out(vty, "delay:");*/
	/*if( DownlinkDelay == V2R1_DELAY_LOW)
		vty_out(vty, "   low  ");
	else vty_out( vty, "   high ");*/
	if( PonChipType == PONCHIP_TYPE_MAX )
	{
		if( DownlinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   --   ");
		else vty_out( vty, "   --   ");	
	}
	else 
	{
		if( DownlinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   --   ");
		else vty_out( vty, "   --   ");	
	}
	
	/*vty_out(vty, "          assured bw:");*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		if(PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
		{
			vty_out(vty, "   No policer");
		}
		else vty_out(vty, "   %-6d    ", Downlink_gr);
	}
	else vty_out(vty, "   %-6d    ", Downlink_gr);

	/*vty_out(vty, "best-efort bw:"  );*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
		{
			vty_out(vty, "   No policer\r\n");
		}
		else vty_out( vty, "   --\r\n", Downlink_be );
	}
	else vty_out(vty, "   %-6d\r\n", Downlink_be );
	
	return( ROK );
}
int  ShowOnulistBandwidthbyVty( short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	short int OnuEntry; 
	short int PonChipType, PonChipVer;
	unsigned int UplinkDelay, UplinkClass, Uplink_gr, Uplink_be;
	unsigned int DownlinkDelay, DownlinkClass, Downlink_gr, Downlink_be;
	
	CHECK_ONU_RANGE
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;

	if( ThisIsValidOnu(PonPortIdx,OnuIdx ) != ROK )
		/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr) == ROK ) ||
		( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1) == ROK ))
		*/
	{
		/*vty_out (vty, "\r\n  %s/port%d Onu%d not provisioned\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , (OnuIdx+1));
		*/
		return( V2R1_ONU_NOT_EXIST );	/* modified by xieshl 20100325, 问题单9978 */
	}	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	if( GetOltCardslotInserted(GetCardIdxByPonChip(PonPortIdx)) != CARDINSERT )
		PonChipType = PONCHIP_PAS;
	
	/*vty_out (vty, "\r\n  %s/port%d Onu%d bandwidth Information(unit:kbit/s) \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , (OnuIdx+1));
	*/
#if 0	
	GetOnuUplinkBW_1(PonPortIdx, OnuIdx, &UplinkClass, &UplinkDelay, &Uplink_gr, &Uplink_be );
	GetOnuDownlinkBW_1(PonPortIdx, OnuIdx, &DownlinkClass, &DownlinkDelay, &Downlink_gr, &Downlink_be );
#else
	GetOnuRunningUplinkBW(PonPortIdx, OnuIdx, &UplinkClass, &UplinkDelay, &Uplink_gr, &Uplink_be );
	GetOnuRunningDownlinkBW(PonPortIdx, OnuIdx, &DownlinkClass, &DownlinkDelay, &Downlink_gr, &Downlink_be );
#endif
	/*added by wutw*/
	vty_out (vty, "  %2d       Uplink   ", (OnuIdx + 1 ));

	/*class show*/
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "  %-2d   ", UplinkClass);
	else
        vty_out(vty, "  --   ");
	
	/*vty_out(vty, "delay:");*/
#ifndef PLATO_DBA_V3
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		if( UplinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   low  ");
		else
            vty_out(vty, "   high ");
	}
#else
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
	{
		/* if( UplinkDelay == V2R1_DELAY_LOW) */ /* remmed by liwei056@2011-4-11 for ShowBug */
			vty_out(vty, "   %5d", /*OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed*/OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed);
	}
#endif
	
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "   %-6d            ", Uplink_gr);
	else
        vty_out(vty, "   --                ");

	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "  %-6d\r\n", Uplink_be );
	else
        vty_out(vty, "  --    \r\n");

	/*========================================*/
	/*downlink*/
	vty_out(vty, "           Downlink ");
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		vty_out(vty, "  --   ");
	/*其他待定*/
	else
        vty_out(vty, "  %2d   ", DownlinkClass);
	
	/*vty_out(vty, "delay:");*/
	/*if( DownlinkDelay == V2R1_DELAY_LOW)
		vty_out(vty, "   low  ");
	else vty_out( vty, "   high ");*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		if( DownlinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   --   ");
		else vty_out( vty, "   --   ");	
	}
	else 
	{
		if( DownlinkDelay == V2R1_DELAY_LOW)
			vty_out(vty, "   --   ");
		else vty_out( vty, "   --   ");	
	}
	
	/*vty_out(vty, "          assured bw:");*/
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
	{
		if(PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
		{
			vty_out(vty, "   No policer        ");
		}
		else vty_out(vty, "   %-6d            ", Downlink_gr);
	}
	else vty_out(vty, "   %-6d            ", Downlink_gr);

	/*vty_out(vty, "best-efort bw:"  );*/
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
	{
		vty_out(vty, "  --\r\n");
		/*vty_out(vty, "  No policer\r\n");*/
	}
    else
    {
    	if( OLT_PONCHIP_ISPAS(PonChipType) )
    	{
    	    vty_out( vty, "  --\r\n", Downlink_be );
    	}
    	else
            vty_out(vty, "  %-6d\r\n", Downlink_be );
    }
	
	return( ROK );
}
#if 0
{
	short int OnuEntry;
	short int LlidIdx = 0;
	
	CHECK_PON_RANGE
/*
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) 
		{
		vty_out(vty, "\r\n  %s/port%d not working\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
*/
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		/* add by chenfj 2006-10-30*/
		if( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr) == ROK ) return (RERROR);
		if( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1) == ROK ) return (RERROR);
		
		
		vty_out (vty, "  %2d", (OnuIdx + 1 ));

		vty_out (vty, "      %6d         %6d", OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].UplinkBandwidth_gr,OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].DownlinkBandwidth_gr);
	
		if( OnuMgmtTable[OnuEntry].OperStatus == ONU_OPER_STATUS_UP )
			{
			vty_out (vty, "          %6d", OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveUplinkBandwidth );

			if(downlinkBWlimit == V2R1_DISABLE )
			{
			vty_out (vty, "        No policer\r\n");
			}
			else{
			vty_out (vty, "        %6d\r\n",OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveDownlinkBandwidth );
			}
			/*
			vty_out (vty, " \t%d",( downlinkBWlimit == V2R1_DISABLE ) ? PonPortTable[PonPortIdx].DownlinkActiveBw : OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveDownlinkBandwidth);
			if( downlinkBWlimit == V2R1_DISABLE ) vty_out (vty, "(no policeing)\r\n");
			else vty_out (vty, "\r\n");
			*/
			}
		else{
			vty_out (vty, "               0" );
			vty_out (vty, "             0\r\n");
			}
	
	return ( ROK );
}

#endif
/* 取值1-64;当此参数缺省时,则为显示全部ONU带宽 */
DEFUN  (
    pon_show_llid_bandwith_config,
    pon_show_llid_bandwith_config_cmd,
    "show bandwidth logical-link <onuid_list>",
    DescStringCommonShow
    "Show bandwidth information\n"
    "Show bandwidth information of logical link\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
	ULONG slotId, port, ulOnuId = 0;
    INT16 phyPonId = 0;
    INT16   userOnuId = 0;  
	int count = 0;
    /*ULONG ulIfIndex = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }*/
	if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	if (count == 1)
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	 	{
	 		userOnuId = (short int )(ulOnuId - 1);
			/*lRet = ShowSingleOnuBandwidthbyVty( phyPonId, userOnuId, vty );*/
			/*lRet = ShowOnuBandwidthByVty(phyPonId, userOnuId, vty );*/
			lRet = ShowOnuBandwidthByVty_1(phyPonId, userOnuId, vty );
			if ( V2R1_ONU_NOT_EXIST == lRet )	/* modified by xieshl 20100325, 问题单9978 */
			{
				vty_out( vty, "  %% Onu %d is not exist.\r\n", ulOnuId);
			}
	    	else if (lRet != VOS_OK)
	         {
	           	vty_out( vty, "  %% Executing error.\r\n");
	       	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
	         }
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
		return CMD_SUCCESS;
	}
	else
	{
		/*vty_out (vty, "\r\n  %s/port%d Onu bandwidth Information list \r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId));
		vty_out (vty, "  OnuIdx   provisioned Bandwidth          Activated Bandwidth\r\n");
		vty_out (vty, "          Uplink         Downlink        Uplink        Downlink(Unit: kbit/s)\r\n");
		*/
#ifdef PLATO_DBA_V3
		vty_out (vty, "\r\n  OnuIdx   direction  class   fixbw   assured-bw(kbit/s)  best-effort-bw(kbit/s)\r\n");
#else
		vty_out (vty, "\r\n  OnuIdx   direction  class   delay   assured-bw(kbit/s)  best-effort-bw(kbit/s)\r\n");
#endif	
		/*vty_out (vty, "\r\n  %s/port%d Onu%d bandwidth Information(unit:kbit/s) \r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId), (OnuIdx+1));
		*/
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	 	{
	 		userOnuId = (short int)(ulOnuId -1);
			ShowOnulistBandwidthbyVty( phyPonId, userOnuId, vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
		return CMD_SUCCESS;
	}
    
}



#if 0

/* 取值1-64;当此参数缺省时,则为显示全部ONU带宽 */
DEFUN  (
    pon_show_llid_bandwith_config,
    pon_show_llid_bandwith_config_cmd,
    "show bandwidth logical-link [<1-64>|all]",
    DescStringCommonShow
    "Show bandwidth information\n"
    "Show bandwidth information of logical link\n"
    "Please input onuId\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    INT16   userOnuId = 0;   
    ULONG ulIfIndex = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

    /*显示所有的onu的带宽分配情况*/	
    if ( !VOS_StrCmp(argv[0], "all"))
    {
      ShowOnuBandwidthAllByVty(phyPonId, vty );
			/*
		for(userOnuId = CLI_EPON_ONUMIN; userOnuId <= CLI_EPON_ONUMAX; userOnuId++)
		{
				lRet = ShowOnuBandwidthByVty(phyPonId, userOnuId, vty );
			    if (lRet != VOS_OK)
			         {
			           #ifdef CLI_EPON_DEBUG
			       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d onuId %d.\r\n",lRet, phyPonId, userOnuId);
			           #endif     
			           	vty_out( vty, "  %% Executing error.\r\n");
			       	return CMD_WARNING;
			         }				
		}*/
    }
    else
    {
    	
        userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;

		lRet = ShowOnuBandwidthByVty(phyPonId, userOnuId, vty );
	    if (lRet != VOS_OK)
	         {
	           #ifdef CLI_EPON_DEBUG
	       	       vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d onuId %d.\r\n",lRet, phyPonId, userOnuId);
	           #endif     
	           	vty_out( vty, "  %% Executing error.\r\n");
	       	return CMD_WARNING;
	         }
    }

    return CMD_SUCCESS;
}



/*该命令已被注释*/
DEFUN  (
    pon_show_onu_register_mac,
    pon_show_onu_register_mac_cmd,
    "show onu-register-mac",
    DescStringCommonShow
    "Show onu register mac information\n"
    )
{
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	ShowPonPortOnuMacAddrByVty( phyPonId, NULL, vty);
    /*ShowPonPortOnuMacAddr( phyPonId ) ;*/
    return CMD_SUCCESS;
}
#endif
/* modified by chenfj 2008-3-20 #6349
     在显示在线ONU  列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
DEFUN  (
    pon_show_online_onu,
    pon_show_online_onu_cmd,
    "show online-onu {[type] <typestring>}*1",
    DescStringCommonShow
    "show current online onu list\n"
    "onu type\n"
    "the specific onu type-string\n"
    /*"the specific onu type-string,e.g "DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n"*/
    )
{
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	unsigned long slotId, port, onuId;
	
    /*ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;*/

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slotId) != ROK)
		{
		vty_out(vty," %% slot %d is not pon card\r\n", slotId);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)slotId,(unsigned char)port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;
#endif
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty,"\r\n pon%d/%d is not working\r\n", slotId, port );
		return (CMD_WARNING );
		}
	
	if( argc == 0 )
		ShowPonPortOnLineOnuByVty( phyPonId, NULL, vty );
	else if( argc == 2 )
		{
		/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
		if(SearchOnuType(argv[1]) == RERROR)
			{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
			}
		ShowPonPortOnLineOnuByVty( phyPonId, argv[1], vty );
		}
	
    return CMD_SUCCESS;
}

/* added by xieshl 20101123 问题单11179 */
DEFUN( pon_show_offline_onu,
        pon_show_offline_onu_cmd,
         "show offline-onu {[type] <typestring>}*1",
        DescStringCommonShow
        "show current offline onu list\n"
         "onu type\n"
        "the specific onu type-string\n" /*DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n"*/
        "show offline onu count\n"
        )
{
	INT16 phyPonId = 0;
	ULONG slotId, port, onuId;
	UCHAR *pOnuTypeString = NULL;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;
	if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
		return CMD_WARNING;
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty,"\r\n pon%d/%d is not working\r\n", slotId, port );
		return (CMD_WARNING );
	}

	if( argc == 2 )
	{
		pOnuTypeString = argv[1];
		if(SearchOnuType(pOnuTypeString) == RERROR)
		{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
		}
	}
	ShowPonPortOffLineOnuByVty( phyPonId, pOnuTypeString, vty );
	
	return CMD_SUCCESS;
}

/* modified by chenfj 2008-2-27 #6349
     在显示ONU 列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
DEFUN  (
    pon_show_list_onu,
    pon_show_list_onu_cmd,
    "show onu-list {[type] <typestring>}*1",
    DescStringCommonShow
    "Show current  onu list information\n"
    "onu type\n"
    "the specific onu type-string\n"
    /*"the specific onu type-string,e.g "DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n" */
    )
{
    /*ULONG ulIfIndex = 0;*/
	ULONG slotId = 0, port = 0, onuId = 0;
    INT16 phyPonId = 0;
	
    /*ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }*/
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( argc == 0 )
		{
		ShowPonPortOnuMacAddrByVty( phyPonId, NULL, vty);
		return CMD_SUCCESS;
		}
	else if( argc == 2 )
		{
		/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
		if(SearchOnuType(argv[1]) == RERROR)
			{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
			}
		ShowPonPortOnuMacAddrByVty( phyPonId, argv[1], vty);
		}
    return CMD_SUCCESS;
}

#ifdef MAC_LEARNING_SHOW
/*使能/去使能地址学习功能此处函数
SetPonPortMacAutoLearningCtrl() 有问题*/
DEFUN  (
    pon_show_mac_learn_config,
    pon_show_mac_learn_config_cmd,
    "show mac-learning",
    DescStringCommonShow
    "Show the mac learning ability of the pon\n"
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    unsigned int CtrlValue = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

    lRet = GetPonPortMacAutoLearningCtrl( (short int )phyPonId,  &CtrlValue );	
    if (lRet != VOS_OK)
    {
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: phyPonId %d\r\n",phyPonId);
    #endif        
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }
	if (CLI_EPON_LEARNING_ENABLE == CtrlValue)
		vty_out( vty, "  mac-learning enable\r\n");
	else if (CLI_EPON_LEARNING_DISABLE == CtrlValue)
		vty_out( vty, "  mac-learning disable\r\n");
    return CMD_SUCCESS;
}
#endif

#ifdef  PON_APS_1
DEFUN  (
    pon_show_aps_config,
    pon_show_aps_config_cmd,
    "show aps",
    DescStringCommonShow
    "Show pon's aps attribute\n"
    )
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
    int apsDone = 0;
     
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	apsDone = GetPonPortApsCtrl( phyPonId );
    if (apsDone == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d apsDone %d.\r\n",phyPonId, apsDone);
	#endif    
        vty_out( vty, "  %% Executing error.\r\n" );
    	 return CMD_WARNING;
    }	
/* [auto|forced|disable]*/
	if ( CLI_EPON_DISABLE == apsDone )
		vty_out( vty, "  aps disable\r\n");
	else if ( CLI_EPON_FORCE == apsDone )
		vty_out( vty, "  aps forced\r\n");
	else if ( CLI_EPON_AUTO == apsDone )
		vty_out( vty, "  aps auto\r\n");
	else
		return CMD_WARNING;
    return CMD_SUCCESS;
}



/*设置mac 地址表项的最大数目

增加了onuId 参数，llid为每个onu 的逻辑链路,目前每个onu的逻辑连接
为1，为了扩展以后，这里定义为1-8 

modified by wutw at 11 september
缺少
返回值:
onu_up	1
onu_down	2
error 	-1
*/



/*设置mac 地址表项的最大数目

增加了onuId 参数，llid为每个onu 的逻辑链路,目前每个onu的逻辑连接
为1，为了扩展以后，这里定义为1-8 

modified by wutw at 11 september
缺少
返回值:
onu_up	1
onu_down	2
error 	-1
*/

DEFUN  (
    pon_show_max_mac_llid_config,
    pon_show_max_max_llid_config_cmd,
		"show max-mac logical-link <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
    DescStringCommonShow
    "Show the max-mac number of the pon`s logical link\n"
    "Show the max-mac number of the pon`s special logical link\n"
    "Please input the logical linkID of the onu \n"
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    UINT32 number = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
	return CMD_WARNING;

	onuId = (( INT16 ) VOS_AtoL( argv[ 0 ] ));
	userOnuId = onuId - 1;

    CHECK_CMD_ONU_RANGE(vty, userOnuId);

	lRet = GetOnuMaxMacNum( (short int )phyPonId, (short int )userOnuId, &number);
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
    }	
	vty_out( vty, "  %d/%d/%d max-mac %d\r\n",slotId,port,onuId,number);
	return CMD_SUCCESS;
}
#endif

#ifdef  PON_STATISTIC_HISTORY_
#endif
/*使能pon或者llid的的历史统计
修改了参数，和帮助说明*/
#if 0
DEFUN  (
    pon_start_statistic_config,
    pon_start_statistic_config_cmd,
    "statistic-history [pon|<1-64>]",
    "Set the history statistic attribute\n"
    "Set pon's history statistic \n"
    "Set onu's history statistic \n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;	
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16   phyPonId = 0;
    INT16   userOnuId = 0;
    ULONG statDevType = 0;	

    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

    if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
    {
    	statDevType = CLI_EPON_PONDEV;
    }
    else 
    {
    	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] ) - 1);  
      /* if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
       {
       #ifdef CLI_EPON_DEBUG
			 vty_out( vty, "phyPonId %d userOnuId %d \r\n",phyPonId, userOnuId);
		#endif
   	    vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, userOnuId);
   	    return CMD_WARNING;    	
       }	*/	
	statDevType = CLI_EPON_ONUDEV;
    }
   
    /*lRet = HisStatsPonStatsStart (phyPonId, TRUE) ;*/
   
    if (CLI_EPON_PONDEV == statDevType)
    {
	lRet = HisStatsPonStatsStart (phyPonId, TRUE)  ;
    }
    else if ( CLI_EPON_ONUDEV == statDevType )
    {
    	lRet = HisStatsOnuStatsStart ( phyPonId,  userOnuId, TRUE);
    }
    else
       {
   	    vty_out( vty, "  %% Parameter is error.\r\n" );
   	    return CMD_WARNING;    	
       }			
	
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}
#endif

DEFUN  (
	pon_start_statistic_sycle_config,
	pon_start_statistic_sycle_config_cmd,
		"statistic-history [pon|<1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">] {[15m|24h]}*1",
	"start the history statistic attribute\n"
	"start pon's history statistics \n"
	"start onu's history statistics \n"
	"start 15 minutes sycle history statistics\n"
	"start 24 hours sycle history statistics\n"
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;	*/
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16   phyPonId = 0;
    INT16   userOnuId = 0;
    ULONG statDevType = 0;	

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

    if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
	{
		statDevType = CLI_EPON_PONDEV;
	}
	else
	{
		userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] ) - 1);
        CHECK_CMD_ONU_RANGE(vty, userOnuId);
		
		/* if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
		 {
		 #ifdef CLI_EPON_DEBUG
		 vty_out( vty, "phyPonId %d userOnuId %d \r\n",phyPonId, userOnuId);
		 #endif
   	    vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, userOnuId);
   	    return CMD_WARNING;    	
       }	*/	
	statDevType = CLI_EPON_ONUDEV;
    }
   
    /*lRet = HisStatsPonStatsStart (phyPonId, TRUE) ;*/
   
    if (CLI_EPON_PONDEV == statDevType)
    {
    	if ( argc < 2 )
    	{
    		lRet = HisStatsPonStatsStart (phyPonId, TRUE);
    	}
    	else if (!VOS_StrCmp((CHAR *)argv[1], "15m"))
		{
			unsigned int bucketNum15M = 0;
			HisStats15MinMaxRecordGet(&bucketNum15M);
			lRet = HisStatsPon15MModified( phyPonId, bucketNum15M, TRUE);
		}
		else if (!VOS_StrCmp((CHAR *)argv[1], "24h"))
		{
			unsigned int bucketNum24h = 0;
			HisStats24HoursMaxRecordGet( &bucketNum24h);
			lRet = HisStatsPon24HModified( phyPonId, bucketNum24h, TRUE);
		}
    }
    else if ( CLI_EPON_ONUDEV == statDevType )
    {
    	if ( argc < 2 )
    	{
    		lRet = HisStatsOnuStatsStart(phyPonId, userOnuId, TRUE);
    	}
    	else if (!VOS_StrCmp((CHAR *)argv[1], "15m"))
		{
			unsigned int bucketNum15M = 0;
			HisStats15MinMaxRecordGet(&bucketNum15M);
			lRet = HisStatsOnu15MModified( phyPonId, userOnuId, bucketNum15M, TRUE);
		}
		else if (!VOS_StrCmp((CHAR *)argv[1], "24h"))
		{
			unsigned int bucketNum24h = 0;
			HisStats24HoursMaxRecordGet( &bucketNum24h);
			lRet = HisStatsOnu24HModified( phyPonId, userOnuId, bucketNum24h, TRUE);
		}
    }
    else
       {
   	    vty_out( vty, "  %% Parameter is error.\r\n" );
   	    return CMD_WARNING;    	
       }			
	
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}


#if 0
DEFUN  (
    pon_no_statistic_config,
    pon_no_statistic_config_cmd,
    "undo statistic-history [pon|<1-64>]",
    NO_STR
    "Set the history statistic attribute\n"
    "Set pon's history statistic \n"
    "Set onu's history statistic \n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16   phyPonId = 0;
    INT16   userOnuId = 0;
    ULONG statDevType = 0;	

    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    phyPonId = cliPonIdCheck(ulIfIndex);
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
    if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
    {
    	statDevType = CLI_EPON_PONDEV;	
    }
    else 
    {
    	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;  
       /*if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
       {
   	    vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, onuId);
   	    return CMD_WARNING;    	
       }*/		
	statDevType = CLI_EPON_ONUDEV;
    }

    /*lRet = HisStatsPonStatsStart (phyPonId, FALSE) ;*/

    if (CLI_EPON_PONDEV == statDevType)
    {
	lRet = HisStatsPonStatsStart (phyPonId, FALSE)  ;
    }
    else if ( CLI_EPON_ONUDEV == statDevType )
    {
    	lRet = HisStatsOnuStatsStart ( phyPonId,  userOnuId, FALSE);
    }
    else
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	if (lRet == CLI_EPON_STATS_PONOBJ_NULL_ERR)
		{
			vty_out( vty, "  %% %d/%d/%d is off-line.\r\n" ,slotId, port, userOnuId+1);
			return CMD_WARNING;
		}
    else if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	        
    return CMD_SUCCESS;
}
#endif

DEFUN  (
	pon_no_statistic_sycle_config,
	pon_no_statistic_sycle_config_cmd,
		"undo statistic-history [pon|<1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">] {[15m|24h]}*1",
	NO_STR
	"undo the history statistic attribute\n"
	"undo pon's history statistic \n"
	"undo onu's history statistic \n"
	"undo 15 minutes sycle history statistics\n"
	"undo 24 hours sycle history statistics\n"    
    )
{
    LONG lRet = VOS_OK;
    /*ULONG ulIfIndex = 0;*/
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16   phyPonId = 0;
    INT16   userOnuId = 0;
    ULONG statDevType = 0;	

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

    if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
	{
		statDevType = CLI_EPON_PONDEV;
	}
	else
	{
		userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
        CHECK_CMD_ONU_RANGE(vty, userOnuId);
		
		/*if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
		 {
		 vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, onuId);
		 return CMD_WARNING;    	
		 }*/
	statDevType = CLI_EPON_ONUDEV;
    }


    if (CLI_EPON_PONDEV == statDevType)
    {
    	if ( argc < 2 )
    	{
    		lRet = HisStatsPonStatsStart( phyPonId, FALSE);
    	}
    	else if (!VOS_StrCmp((CHAR *)argv[1], "15m"))
		{
			unsigned int bucketNum15M = 0;
			HisStats15MinMaxRecordGet(&bucketNum15M);
			lRet = HisStatsPon15MModified( phyPonId, bucketNum15M, FALSE);
		}
		else if (!VOS_StrCmp((CHAR *)argv[1], "24h"))
		{
			unsigned int bucketNum24h = 0;
			HisStats24HoursMaxRecordGet( &bucketNum24h);
			lRet = HisStatsPon24HModified( phyPonId, bucketNum24h, FALSE);
		}
    }
    else if ( CLI_EPON_ONUDEV == statDevType )
    {
    	if ( argc < 2 )
    	{
    		lRet = HisStatsOnuStatsStart(phyPonId, userOnuId, FALSE);
    	}
    	else if (!VOS_StrCmp((CHAR *)argv[1], "15m"))
		{
			unsigned int bucketNum15M = 0;
			HisStats15MinMaxRecordGet(&bucketNum15M);
			lRet = HisStatsOnu15MModified( phyPonId, userOnuId, bucketNum15M, FALSE);
		}
		else if (!VOS_StrCmp((CHAR *)argv[1], "24h"))
		{
			unsigned int bucketNum24h = 0;
			HisStats24HoursMaxRecordGet( &bucketNum24h);
			lRet = HisStatsOnu24HModified( phyPonId, userOnuId, bucketNum24h, FALSE);
		}
    }
    else
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	if (lRet == CLI_EPON_STATS_PONOBJ_NULL_ERR)
		{
			vty_out( vty, "  %% %d/%d/%d is off-line.\r\n" ,slotId, port, userOnuId+1);
			return CMD_WARNING;
		}
    else if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	        
    return CMD_SUCCESS;
}


#if 0
DEFUN  (
    pon_show_statistic_bucket,
    pon_show_statistic_bucket_cmd,
    "show statistic-history bucket-num",
    DescStringCommonShow
    "Show the history statistic\n"
    "bucket number of the history statistic\n"
    )
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

/* modified by chenfj 2008-7-10
     将以下几个命令合并到一起
     */
/* begin: added by jianght 20090917 */
extern unsigned short 	gHis15MinMaxRecord;
extern unsigned short 	gHis24HoursMaxRecord;
/* end: added by jianght 20090917 */
DEFUN  (
	pon_show_statistic_15m_24h_data,
	pon_show_statistic_15m_24h_data_cmd,
	"show statistic-history data [15m|24h] {<1-1440>}*1",	/* modified by xieshl 20091021, 问题单8761 */
	DescStringCommonShow
	"list pon history statistic data\n"
	"list pon history statistic data\n"
	"list pon 15 minute history statistic data\n"
	"list pon 24 hour history statistic data\n"
	"input hours number, the max hours of 15m is 50, the max hours of 24h is 1440\n"
    )
{
	/*ULONG ulIfIndex = 0;*/
	LONG lRet = VOS_OK;
	ULONG slotId = 0;
	ULONG onuId = 0; 
	ULONG port = 0;
	INT16 phyPonId = 0;	
	unsigned int  time_len = 0;
	unsigned int  bucket_num = 0;
	unsigned char flag =0;
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( VOS_StrCmp( argv[0], "15m" ) == 0 )
		flag = CLI_EPON_STATS15MIN;
	else if(VOS_StrCmp( argv[0], "24h" ) == 0 )
		flag = CLI_EPON_STATS24HOUR;
	else return CMD_WARNING;
	
	if(argc ==  2)
		{
		time_len = (( unsigned int  ) VOS_AtoL( argv[ 1 ] ));
		/* begin: modified by jianght 20090917 */
		/* 问题单:8761 */
		if( flag == CLI_EPON_STATS15MIN )
		{
			bucket_num = time_len*60/15;

			if (bucket_num < 0  || bucket_num > gHis15MinMaxRecord)
			{
				vty_out( vty, "  %% Sorry, the current bucket num of 15m configed is %d, the max hours value is %d accordingly.\r\n", gHis15MinMaxRecord, (gHis15MinMaxRecord + 3) /4 );
			}
		}
		else if(flag == CLI_EPON_STATS24HOUR )
		{
			if(time_len < CLI_EPON_HOURS_ONE_DAY)
				bucket_num = 1;
			else if((time_len%CLI_EPON_HOURS_ONE_DAY) == 0)
				bucket_num = time_len/CLI_EPON_HOURS_ONE_DAY;
			else
				bucket_num = time_len/CLI_EPON_HOURS_ONE_DAY + 1;
			
			if (bucket_num < 0  || bucket_num > gHis24HoursMaxRecord)
			{
				vty_out( vty, "  %% Sorry, the current bucket num of 24h configed is %d, the max hours value is %d accordingly.\r\n", gHis24HoursMaxRecord, gHis24HoursMaxRecord * CLI_EPON_HOURS_ONE_DAY );
			}
		}
		/* end: modified by jianght 20090917 */
		}
	else{ 
		if(flag == CLI_EPON_STATS15MIN )
			HisStats15MinMaxRecordGet(&bucket_num);
		else if(flag == CLI_EPON_STATS24HOUR )
			HisStats24HoursMaxRecordGet(&bucket_num);
		}

	if(flag == CLI_EPON_STATS15MIN )
		lRet = CliHisStats15MinDataVty(phyPonId, bucket_num, vty);
	else if(flag == CLI_EPON_STATS24HOUR )
		lRet = CliHisStats24HourDataVty(phyPonId, bucket_num, vty);	
	
	if (lRet != VOS_OK)
		{
		vty_out( vty, "  %% Executing error.\r\n" );
		return CMD_WARNING;
		}	  	

	return CMD_SUCCESS;
}

#if 0
DEFUN  (
    pon_show_statistic_15mall,
    pon_show_statistic_15mall_cmd,
    "show statistic-history 15m-data",
    DescStringCommonShow
    "Show pon`s history statistic\n"
    "Show pon`s history statistic of 15 minute`s cycle\n"
    )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	unsigned int bucket_num = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    { 
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	HisStats15MinMaxRecordGet(&bucket_num);
	lRet = CliHisStats15MinDataVty(phyPonId, bucket_num, vty);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  	
	
    return CMD_SUCCESS;
}



DEFUN  (
    pon_show_statistic_24hdata,
    pon_show_statistic_24hdata_cmd,
    "show statistic-history 24h-data <time_len>",
    DescStringCommonShow
    "Show pon`s history statistic\n"
    "Show pon`s history statistic of 24 hour`s cycle\n"
    "bucket number of the history statistic\n"
    )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	unsigned int  time_len = 0;
	unsigned int  bucket_num = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	time_len = (( unsigned int  ) VOS_AtoL( argv[ 0 ] ));

	if(time_len < 24)
		bucket_num = 1;
	else if(time_len%24 == 0)
		bucket_num = time_len/24;
	else
		bucket_num = time_len/24 + 1;
	
	lRet = CliHisStats24HourDataVty(phyPonId, bucket_num, vty);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );;
	return CMD_WARNING;
    }	  	
	
    return CMD_SUCCESS;
}



DEFUN  (
    pon_show_statistic_24hall,
    pon_show_statistic_24hall_cmd,
    "show statistic-history 24h-data",
    DescStringCommonShow
    "Show pon`s history statistic\n"
    "Show pon`s history statistic of 24 hour`s cycle\n"
    "bucket number of the history statistic\n"
    )
{
    ULONG ulIfIndex = 0;
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	unsigned int bucket_num = 0;
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    { 
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
		
	HisStats24HoursMaxRecordGet(&bucket_num);
	
	lRet = CliHisStats24HourDataVty(phyPonId, bucket_num, vty);
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  	
	
    return CMD_SUCCESS;
}

#endif

/* modified by chenfj 2008-7-10
     将以下几个命令合并到一起
     */
DEFUN  (
	pon_clear_statistic_pon_onu_data,
	pon_clear_statistic_pon_onu_data_cmd,
		"undo statistic-history data [pon|<1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">] {[15m|24h]}*1",
	NO_STR
	"Clear the history statistic data\n"   
	"Clear the history statistic data\n"
	"Clear pon history statistic data\n"
	"Clear onu history statistic data\n"
	"Clear 15 minute history statistic data\n"
	"Clear 24 hour history statistic data\n"
    )
{
	LONG lRet = VOS_OK; 
	/*ULONG ulIfIndex = 0;*/
	ULONG slotId = 0;
	ULONG onuId = 0; 
	ULONG port = 0;		
	INT16 phyPonId = 0, userOnuId = 0;
	unsigned int Device_flag = CLI_EPON_PONDEV;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if(VOS_StrCmp((CHAR *)argv[0], "pon") == 0)
	Device_flag = CLI_EPON_PONDEV;
	else {
		Device_flag = CLI_EPON_ONUDEV;
		userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;
        CHECK_CMD_ONU_RANGE(vty, userOnuId);		
	}

	if( Device_flag == CLI_EPON_PONDEV )
	{
		if(argc == 1 )
		{
			lRet = HisStatsPon15MinRawClear(phyPonId);
			if(lRet != VOS_OK)
				{
				vty_out( vty, "  %% pon 15m-obj isn`t in history-stats list.\r\n" );
				return CMD_WARNING;
				}
			lRet = HisStatsPon24HourRawClear(phyPonId);
			if(lRet != VOS_OK)
				{
				vty_out( vty, "  %% pon 24h-obj isn`t in history-stats list.\r\n" );
				return CMD_WARNING;
				}
			}
		else{
			if(VOS_StrCmp((CHAR *)argv[1], "15m") == 0)
				{
				lRet = HisStatsPon15MinRawClear(phyPonId);
				if(lRet != VOS_OK)
					{
					vty_out( vty, "  %% pon 15m-obj isn`t in history-stats list.\r\n" );
					return CMD_WARNING;
					}
				}
			else  if(VOS_StrCmp((CHAR *)argv[1], "24h") == 0)
				{
				lRet = HisStatsPon24HourRawClear(phyPonId);
				if(lRet != VOS_OK)
					{
					vty_out( vty, "  %% pon 24h-obj isn`t in history-stats list.\r\n" );
					return CMD_WARNING;
					}
				}
			else
				{
				vty_out( vty, "  %% Parameter is error.\r\n" );
				return CMD_WARNING;    	
				}
			}
		}
	else if(Device_flag == CLI_EPON_ONUDEV)
		{
		if(argc == 1 )
			{
			lRet = HisStatsOnu15MinRawClear(phyPonId, userOnuId) ;	
			if(lRet != VOS_OK)
				{
				vty_out( vty, "  %% onu%d/%d/%d 15m-obj isn`t in history-stats list.\r\n",slotId, port,(userOnuId+1));
				return CMD_WARNING;
				}
			lRet = HisStatsOnu24HourRawClear(phyPonId, userOnuId);
			if(lRet != VOS_OK)
				{
				vty_out( vty, "  %% onu%d/%d/%d 24h-obj isn`t in history-stats list.\r\n",slotId, port,(userOnuId+1));
				return CMD_WARNING;
				}
			}
		else{
			if(VOS_StrCmp((CHAR *)argv[1], "15m") == 0)
				{
				lRet = HisStatsOnu15MinRawClear(phyPonId, userOnuId) ;
				if(lRet != VOS_OK)
					{
					vty_out( vty, "  %% onu%d/%d/%d 15m-obj isn`t in history-stats list.\r\n",slotId, port,(userOnuId+1));
					return CMD_WARNING;
					}
				}
			else  if(VOS_StrCmp((CHAR *)argv[1], "24h") == 0)
				{
				lRet = HisStatsOnu24HourRawClear(phyPonId, userOnuId);
				if(lRet != VOS_OK)
					{
					vty_out( vty, "  %% onu%d/%d/%d 24h-obj isn`t in history-stats list.\r\n",slotId, port,(userOnuId+1));
					return CMD_WARNING;
					}
				}
			}
		}
	return CMD_SUCCESS;
}

#if 0
DEFUN  (
    pon_clear_statistic_pon_all,
    pon_clear_statistic_pon_all_cmd,
    "undo statistic-history data pon",
    NO_STR
    "Clear the history statistic\n"   
    "Clear the history statistic data\n"
    "Clear pon`s all of history statistic data\n"
    )
{
    LONG lRet = VOS_OK; 
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16 phyPonId = 0;	
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );		
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
		
	lRet = HisStatsPon15MinRawClear(phyPonId);
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% obj isn`t in history-stats list.\r\n" );
		return CMD_WARNING;
    }	 
		 
	lRet = HisStatsPon24HourRawClear(phyPonId);
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% obj isn`t in history-stats list.\r\n" );
		return CMD_WARNING;
    }	  	
    return CMD_SUCCESS;
}




DEFUN  (
    pon_clear_statistic_onu_data,
    pon_clear_statistic_onu_data_cmd,
    "undo statistic-history data <1-64> [15m|24h]",
    NO_STR
    "Clear the history statistic\n"   
    "Clear onu`s history statistic data\n"
    "Please input onuId\n"
    "Clear onu`s history statistic data of 15 minute`s cycle\n"
    "Clear onu`s history statistic data of 24 hour`s cycle\n"
    )
{
    LONG lRet = VOS_OK; 
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		  
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;

	
    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );		
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	

    userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;  
   /* if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
    {
   	    vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, userOnuId+1 );
   	    return CMD_WARNING;    	
    }	*/	
		
    if (!VOS_StrCmp((CHAR *)argv[1], "15m"))
    {
    	lRet = HisStatsOnu15MinRawClear(phyPonId, userOnuId) ;
    }
    else  if (!VOS_StrCmp((CHAR *)argv[1], "24h"))
    {
    	lRet = HisStatsOnu24HourRawClear(phyPonId, userOnuId);
    }

     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% obj isn`t in history-stats list.\r\n" );
		return CMD_WARNING;
    }	  	

    return CMD_SUCCESS;
}



DEFUN  (
    pon_clear_statistic_onu_all,
    pon_clear_statistic_onu_all_cmd,
    "undo statistic-history data <1-64>",
    NO_STR
    "Clear the history statistic\n"   
    "Clear onu`s all of history statistic data\n"
    "Please input onuId\n"
    )
{
    LONG lRet = VOS_OK; 
    ULONG ulIfIndex = 0;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;		
    INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;		
    PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId);	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );		
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	

    userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1;  
    /*if ( VOS_OK != cliOnuIdCheck(phyPonId, userOnuId))
    {
   	    vty_out( vty, "  %% %d/%d/%d is not exist.\r\n", slotId, port, userOnuId+1 );
   	    return CMD_WARNING;    	
    }	*/	

	lRet = HisStatsOnu15MinRawClear(phyPonId, userOnuId) ;	
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% obj isn`t in history-stats list.\r\n" );
		return CMD_WARNING;
    }	  	
	lRet = HisStatsOnu24HourRawClear(phyPonId, userOnuId);
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% obj isn`t in history-stats list.\r\n" );
		return CMD_WARNING;
    }	
		 
    return CMD_SUCCESS;
}
#endif

DEFUN  (
    pon_show_statistic,
    pon_show_statistic_cmd,
    "show statistic-history",
    DescStringCommonShow
    "Show pon`s history statistic\n"
    )
{
    /*ULONG ulIfIndex = 0;*/
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	
	
	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	lRet = CliHisStatsPonCtrlGet( phyPonId, vty );
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  	
	
    return CMD_SUCCESS;
}


#ifdef  PON_BER_FER_ALARM_
/***Alarm**/
DEFUN  (
    pon_alarm_config,
    pon_alarm_config_cmd,
    "alarm [ber|fer] [enable|disable]",
    "Set alarm\n"
    "ber\n"
    "fer\n"
    "Enable the alarm mask\n"
    "Disable the alarm mask\n"
    )
{
    LONG lRet = VOS_ERROR; 
    /*ULONG ulIfIndex = 0;*/
    ULONG slotId, port, onuId;
    INT16 phyPonId = 0;
    UINT32 alarmType = 0;    	
    UINT32 Done = 0;
	
    /*ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }*/
	if( parse_pon_command_parameter( vty, &slotId, &port, &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

    if (!VOS_StrCmp((CHAR *)argv[0], "ber"))
    {
    	alarmType = CLI_EPON_BER;
    }
    else /*if (!VOS_StrCmp((CHAR *)argv[0], "fer"))*/
    {
      alarmType = CLI_EPON_FER;
    }
    /*else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }*/

    if (!VOS_StrCmp((CHAR *)argv[1], "enable"))
    {
    	Done = CLI_EPON_ALARM_ENABLE;
    }
    else /*if (!VOS_StrCmp((CHAR *)argv[2], "disable"))*/
    {
      Done = CLI_EPON_ALARM_DISABLE;
    }
    /*else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }*/


    if (CLI_EPON_BER == alarmType)
    {
    	lRet = monPonBerAlmEnSet(phyPonId, Done);
    }
    else if (CLI_EPON_FER == alarmType)
    {
    	lRet = monPonFerAlmEnSet(phyPonId, Done);
    }
	if (lRet != VOS_OK)
	{
  		vty_out( vty, "  %% Executing error.\r\n" );
  		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}


/*该cli 命令暂不提供*/
DEFUN  (
    pon_alarm_threshold_config,
    pon_alarm_threshold_config_cmd,
    "alarm [ber|fer] threshold <threshold_value>",
    "Set alarm\n"
    "ber\n"
    "fer\n"
    "Set the alarm threshold\n"
    "Please input the threshold value(10e-7)\n"
    )
{
    LONG lRet = VOS_OK; 
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
    UINT32 alarmType = 0;    	
    INT32 thresholdValue = 0;
	ULONG slotId, port, onuId;
	
    /*ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }*/
	if( parse_pon_command_parameter( vty, &slotId, &port, &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;
	
	
    if (!VOS_StrCmp((CHAR *)argv[0], "ber"))
    {
    	alarmType = CLI_EPON_BER;
    }
    else /*if (!VOS_StrCmp((CHAR *)argv[0], "fer"))*/
    {
      alarmType = CLI_EPON_FER;;
    }
    /*else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }  */      

    thresholdValue = ( UINT32 ) VOS_AtoL( argv[ 1 ] );
    if ( (thresholdValue < 0) || (thresholdValue > 10000000) )
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }  		

    if (CLI_EPON_BER == alarmType)
    {
    	lRet = monponBERThrSet( thresholdValue, -1 );
    }
    else if (CLI_EPON_FER == alarmType)
    {
    	lRet = monponFERThrSet(thresholdValue, -1 );
    }

     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  

    return CMD_SUCCESS;
}


/*该cli 命令暂不提供*/
DEFUN  (
    pon_show_alarm_info,
    pon_show_alarm_info_cmd,
    "show alarm [ber|fer] information",
    DescStringCommonShow
    "Show alarm information\n"
    "Ber alarm  in the system\n"
    "Fer alarm  in the system\n"
    "Show alarm information\n"
    )
{
    LONG lRet = VOS_OK; 
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
    UINT32 alarmType = 0;    	
    INT32 thresholdValue = 0;
	unsigned int AlmEn = 0;
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;	
    phyPonId = cliPonIdCheck(ulIfIndex);
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
    
    if (!VOS_StrCmp((CHAR *)argv[0], "ber"))
    {
    	alarmType = CLI_EPON_BER;
    }
    else if (!VOS_StrCmp((CHAR *)argv[0], "fer"))
    {
      alarmType = CLI_EPON_FER;;
    }
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }   

    if (CLI_EPON_BER == alarmType)
    {
    	lRet = monponBERThrGet(&thresholdValue);
		lRet = monPonBerAlmEnGet(phyPonId, &AlmEn);			
    }
    else if (CLI_EPON_FER == alarmType)
    {
    	lRet = monponFERThrGet(&thresholdValue);
       lRet = monPonFerAlmEnGet(phyPonId, &AlmEn);				
    }

     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	  

    vty_out( vty, "  threshold     %d(10e-7)\r\n", thresholdValue);
    if ( AlmEn == CLI_EPON_ALARM_ENABLE)
    	vty_out( vty, "  mask          enable\r\n");
    else if ( AlmEn == CLI_EPON_ALARM_DISABLE)
	vty_out( vty, "  mask          disable\r\n");

    /**/
	/*
    vty_out( vty, "  status        normal\r\n");
    vty_out( vty, "  status        alarm\r\n");
	*/		
    return CMD_SUCCESS;
}
#endif

/*******Capability Monitor***************/

#ifdef  PON_OPTICAL_LINK_MONITOR_

DEFUN  (
    pon_optical_link_monitor,
    pon_optical_link_monitor_cmd,
    "optical-link [enable|disable]",
    "Monitor on optical link\n"
    "Enable the monitor on optical link\n"
    "Disable the monitor on optical link\n"
    )
{
    LONG lRet = VOS_OK; 
    short int  monStatus = 0;
    if (!VOS_StrCmp((CHAR *)argv[0], "ber"))
    {
    	monStatus = CLI_EPON_MONITOR_ENABLE ;
    }
    else if (!VOS_StrCmp((CHAR *)argv[0], "fer"))
    {
      monStatus = CLI_EPON_MONITOR_DISABLE;
    }
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }   

    lRet = monStatusSet(monStatus);
     if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	 		
		 
    return CMD_SUCCESS;
}




		
DEFUN  (
    pon_optical_link_ber_set,
    pon_optical_link_ber_set_cmd,
    "monitor optical-link ber <threshold_value>",
    "Set the Monitor\n"
    "Monitor on optical link\n"
    "Set the optical-link monitor ber\n"
    "Please input the threshold value of the optical-link monitor ber \n"
    )
{
        
    return CMD_SUCCESS;
}




DEFUN  (
    pon_show_optical_line_parameter,
    pon_show_optical_line_parameter_cmd,
    "show optical-line parameter",
    DescStringCommonShow
    "Show the information of optical-line\n"
    "Show optical-line parameter\n"
    )
{
    short int  monStatus = 0;
    monStatusGet(&monStatus) ;
    if ( CLI_EPON_MONITOR_ENABLE == monStatus)
	vty_out( vty, "  Optical line monitor is enable.\r\n" );
    else if ( CLI_EPON_MONITOR_DISABLE== monStatus)
	vty_out( vty, "  Optical line monitor is disable.\r\n" );
    else
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   
    return CMD_SUCCESS;
}
#endif

#if 0
char *glLogFileBuffer = NULL;
long   glLogFileLen    = 0;

void ReadOnuLogFileCallback(unsigned short usPonID, unsigned short usOnuID, 
                            char *pFileName, char *pFileBuf, long lFileLen) 
{
    glLogFileBuffer = pFileBuf;
    glLogFileLen     = lFileLen;
}


DEFUN(onu_logfile_show,
	onu_logfile_show_cmd,
	"show onu file <onu_no> <filename>",
	"Show onu file information\n"
	"Show onu file \n"
	"Show onu log file or config file \n"
	"Input onu number \n"
	"Input filename: log.txt or config.txt \n")
{
      /*ulong ulVlanIfindex;*/
    
      ulong  ulSlot   = 0;
      ulong  ulPort   = 0;
      ulong  OnuId   = 0;
      short int  PonId   = 0;
      char  FileName[256];

	/*ulVlanIfindex = (ULONG) vty->index;

	if( PON_GetSlotPortOnu( ulVlanIfindex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&OnuId) == VOS_ERROR )
    		return CMD_WARNING;
	
	PonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );*/
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &OnuId, &PonId) != VOS_OK )
		return CMD_WARNING;

       OnuId = strtol(argv[0],NULL, 10);
	   
	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	 if( CLI_EPON_ONUDOWN == GetOnuOperStatus( PonId, OnuId - 1))
       {
            vty_out( vty, "  %% onu %lu/%lu/%lu is off-line.\r\n", ulSlot,  ulPort, OnuId) ;
            return CMD_SUCCESS;   
       }	

	VOS_StrCpy(FileName, argv[1]);

	if((OnuId < 0) || (OnuId > MAXONUPERPON)) return CMD_WARNING;

	glLogFileBuffer = NULL;
	glLogFileLen    = 0;

       if(funOamTransFileLocReqApi(OAM_FILETX_REQ_READ, PonId, OnuId, "syslog", FileName, 
		                                          OAM_FILETX_CALL_BLOCK, NULL, NULL, ReadOnuLogFileCallback) != VOS_OK)
       {
	    vty_out(vty,"Receive file FAILE, onu %d/%d/%d, filename= %s\r\n",  ulSlot, ulPort, OnuId, FileName);
            return CMD_SUCCESS;    
       }

	if((glLogFileBuffer != NULL) && ((VOS_StrCmp(FileName,"log.txt") == 0)  || (VOS_StrCmp(FileName,"config.txt") == 0)) )
	{
	    glLogFileBuffer[glLogFileLen] = 0;
	    vty_big_out(vty, glLogFileLen + 8, "%s\r\n", glLogFileBuffer);
	}
	else
	{
	    vty_out(vty,"Receive file SUCCESS onu %d/%d/%d, filename= %s filelen = %d\r\n",  ulSlot, ulPort, OnuId, FileName, glLogFileLen);
	}

	/* 出现断言，修改之
		chenfj 2007-8-21 */
	if( glLogFileBuffer != NULL )
		free(glLogFileBuffer);
		/*VOS_Free(glLogFileBuffer);*/

	glLogFileBuffer = NULL;
       glLogFileLen    = 0;
	
	return CMD_SUCCESS;
}

/*  added by chenfj 2008-2-3
      在CONFIG节点下增加显示当前正在升级ONU的命令
      */


#if 0
DEFUN  (
    pon_log_show,
    pon_log_show_cmd,
    "show onu-log <1-64>",
    DescStringCommonShow
    "Show onu log\n"
    "Please input onuId\n"
    )
{
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	ULONG ulIfIndex = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	

	onuUpdateStatis_t updateInfo[64];
	
	memset(updateInfo, 0, sizeof(onuUpdateStatis_t)*64);
	
    ulIfIndex = ( ULONG ) ( vty->index ) ;
	file_vty = vty;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;
	}
	userOnuId = (( INT16 ) VOS_AtoL( argv[ 0 ] ));
	/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId-1);
    if ( CLI_EPON_ONUUP != lRet)
    {
   	   vty_out( vty, "  %% %lu/%lu/%lu is off-line.\r\n",ulSlot,  ulPort, userOnuId) ;
		return CMD_WARNING;
    }
	
	funOamTransFileLocReqApi((long)1/*read*/, phyPonId, userOnuId, "gt812", \
							"log.txt", (long)2/*block*/, vty, (POAMTRANSFILECALLBACK)CliOnuLogFileTranResult, \
							(POAMRECVFILECALLBACK)CliOnuLogFileTranData);

	return CMD_SUCCESS;
	
}
#endif



DEFUN  (
    pon_log_upload,
    pon_log_upload_cmd,
    "upload onu-log <H.H.H.H> <user> <pass> <onuid_list>",
    DescStringCommonShow
    "Show onu log\n"
    "Please input ftp server IP\n"
    "Please input user name on the ftp server\n"
    "Please input password on the ftp server\n"
    "Please input onu list(maximum 10)\n"
    )
{
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	/*ULONG ulIfIndex = 0;*/
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	
	int onuNum = 0;

	VOS_MemSet(cliUserIp, 0, 64);
	VOS_MemSet(cliUserName, 0, 64);
	VOS_MemSet(cliUserPassWord, 0, 64);
	
	VOS_Sprintf(cliUserIp, argv[0]);
	VOS_Sprintf(cliUserName, argv[1]);
	VOS_Sprintf(cliUserPassWord, argv[2]);
		
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	/*计算输入的onu个数,如果输入大于10,则*/
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 3 ], ulOnuId )
	{
		onuNum++;
		if (onuNum > 10)
		{
			vty_out( vty, "  %% Too much onu.Pleas limit to input ten onus\r\n");
			return CMD_WARNING;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	file_vty = vty;
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 3 ], ulOnuId )
	{	
		userOnuId = (INT16)(ulOnuId );/*文件传输部分会转换*/
		/*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
		sys_console_printf( "\r\n%-40s%d %d", "onu location info -- ponid, onuid", phyPonId, userOnuId-1 );
	    lRet = 	GetOnuOperStatus( phyPonId, userOnuId-1 );
	    if ( CLI_EPON_ONUUP != lRet)
	    {
	       continue;
	    }
		
		funOamTransFileLocReqApi((long)1/*read*/, phyPonId, userOnuId, "NULL", \
								"log.txt", (long)1/*block*/, NULL, (POAMTRANSFILECALLBACK)CliOnuLogFileTUploadResult, \
								(POAMRECVFILECALLBACK)CliOnuLogFileUpload);
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	return CMD_SUCCESS;
	
}
#endif


/* modified by chenfj 2008-2-27  #6349
     在显示ONU 版本信息时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */

DEFUN  (
    pon_onu_version_by_type_show,
    pon_onu_version_by_type_show_cmd,
    "show onu-version type <typestring>",
    DescStringCommonShow
    "Show onu version\n"
    "onu type\n"
    /*"the specific onu type-string,e.g "DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n"  */
    "the specific onu type-string\n"
    )

{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	/*ULONG ulIfIndex = 0;*/
	INT16 phyPonId = 0;	
	/*int count = 0;*/
	/*int DeviceTypeLen,i;*/
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/* modified by chenfj 2009-6-3
	增加显示信息，用于提示ONU 类型*/
	if(SearchOnuType(argv[0]) == RERROR)
		{
		NotificationOnuTypeString(vty);	
		return(CMD_WARNING);
		}
			
	vty_out( vty, "\r\n" );
	
	if( AllOnuCounter( phyPonId ) <= 0 ) 
		{
		vty_out(vty, " No onu exist in this pon port \r\n");
		return( CMD_SUCCESS );
		}
	
	/*DeviceTypeLen = GetDeviceTypeLength();
	vty_out(vty, "%s/port%d onu version info:\r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId));
	vty_out(vty, "Idx  type    ");
	for(i=0;i<(DeviceTypeLen - 5); i++)
		vty_out(vty," ");
	vty_out(vty,"range    HW-version  SW-version     userName\r\n\r\n");
	vty_out(vty, "----------------------------------------------------------------------\r\n");*/
	show_onu_version_banner(vty);

	for(ulOnuId=0; ulOnuId< MAXONUPERPON; ulOnuId ++ )
	 	{
	 	ShowOnuVersionInfoByVty_1( phyPonId, ulOnuId, argv[0], vty );
		}

	vty_out( vty, "\r\n");
	return CMD_SUCCESS;
}

DEFUN  (
    pon_onu_version_show,
    pon_onu_version_show_cmd,
    "show onu-version {<onuid_list>}*1",
    DescStringCommonShow
    "Show onu version\n"
    OnuIDStringDesc
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	/*ULONG ulIfIndex = 0;*/
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
	int count = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	vty_out( vty, "\r\n" );	
	if( argc != 0 )
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	 	{
	 		count ++;
		}
		END_PARSE_ONUID_LIST_TO_ONUID();

		if (count == 1)
		{
			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
		 	{
		 		userOnuId = (INT16)(ulOnuId - 1);
		 		ShowOnuVersionInfoByVty( phyPonId, userOnuId, vty );
			}
			END_PARSE_ONUID_LIST_TO_ONUID();
		}
		else if( count > 0 )
		{
			if( AllOnuCounter( phyPonId ) <= 0 ) 
			{
				vty_out(vty, " No onu exist in this pon port \r\n");
				return( CMD_SUCCESS );
			}
		
			show_onu_version_banner(vty);

			BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
		 	{
		 		userOnuId = (INT16)(ulOnuId - 1);
		 		ShowOnuVersionInfoByVty_1( phyPonId, userOnuId, NULL, vty );
			}
			END_PARSE_ONUID_LIST_TO_ONUID();
		}
	}
	/* modified by chenfj 2007/04/26
         问题单#4298 : 修改命令show onu-version <onu-list>，没有ONU参数时，显示当前PON端口下所有ONU版本
         */
	else {
		unsigned short  OnuIdx;
		if( AllOnuCounter( phyPonId ) <= 0 ) 
		{
			vty_out(vty, " No onu exist in this pon port \r\n");
			return( CMD_SUCCESS );
		}

		show_onu_version_banner(vty);

		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			if( ThisIsValidOnu(phyPonId, OnuIdx) == ROK )
				ShowOnuVersionInfoByVty_1( phyPonId, OnuIdx, NULL, vty );
		}
	}
	vty_out( vty, "\r\n");
	return CMD_SUCCESS;
}




DEFUN  (
    pon_onu_auto_update_enable,
    pon_onu_auto_update_enable_cmd,
    "onu software update enable <onuid_list>",
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update enable\n"
    "Config the onu software update enable\n"
    OnuIDStringDesc
    )
{
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    /*ULONG ulIfIndex = 0;*/
	ULONG ulOnusigle = 0;
	INT16 userOnuId = 0;	
    INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	EnableFlag = CLI_EPON_PON_EN;
	vty_out( vty, "\r\n" );
	if ( argc == 0 )
	{
		for( userOnuId = 0; userOnuId<= CLI_EPON_ONUMAX; userOnuId++)
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
	}
	else
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnusigle )
		{
			userOnuId = ulOnusigle - 1;
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
	}

    return CMD_SUCCESS;
}


DEFUN  (
    pon_onu_auto_update_disable,
    pon_onu_auto_update_disable_cmd,
    "undo onu software update enable <onuid_list>",
    NO_STR
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update enable\n"
    "Config the onu software update enable\n"
    OnuIDStringDesc
    )
{
	ULONG slotId = 0;
	ULONG port = 0;
	ULONG onuId = 0;
    /*ULONG ulIfIndex = 0;*/
	ULONG ulOnusigle = 0;
	INT16 userOnuId = 0;	
    INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	if( parse_pon_command_parameter( vty, &slotId, &port , &onuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	EnableFlag = CLI_EPON_PON_DIS;
	vty_out( vty, "\r\n" );
	if ( argc == 0 )
	{
		for( userOnuId = 0; userOnuId<= CLI_EPON_ONUMAX; userOnuId++)
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
	}
	else
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnusigle )
		{
			userOnuId = ulOnusigle - 1;
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
	}
	

    return CMD_SUCCESS;
}


/* modified by xieshl 20110117, 问题单11919 */
DEFUN  (
    pon_onu_auto_update_show,
    pon_onu_auto_update_show_cmd,
    "show onu software update",
    DescStringCommonShow
    "Show onu information\n"
    "Show the onu software\n"
    "Show the onu software auto update attribute\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	ShowOnuSoftwareUpdateByVty( phyPonId, vty);

    return CMD_SUCCESS;
}



#ifdef ONU_PEER_TO_PEER	

#define EPON_P2P_UNICAST_ENABLE 	1
#define EPON_P2P_UNICAST_DISABLE	2
#define EPON_P2P_BRDCAST_ENABLE 	1
#define EPON_P2P_BRDCAST_DISABLE	2

/*added by wutw 2006/12/13*/


DEFUN(
	peer_to_peer_add, 
	peer_to_peer_add_cmd,
	"onu p2p <onuid_list> <onuid_list>",
	"Config the onu\n"
	"Confine onu peer to peer onu\n"
	OnuIDStringDesc
	OnuIDStringDesc
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	ULONG ulScrOnuId = 0;
	ULONG ulDstOnuId = 0;
	ULONG CountFirst = 0;
	ULONG CountSecond = 0;
	ULONG scrUlOnuStr[MAXONUPERPONNOLIMIT] = {0};
	ULONG dstUlOnuStr[MAXONUPERPONNOLIMIT] = {0};
	int tempcount = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
		CountFirst++;
		scrUlOnuStr[ulOnuId-1] = ulOnuId;/*将源端口进行排队*/
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	{
		CountSecond++;
		dstUlOnuStr[ulOnuId-1] = ulOnuId;/*将目的端口进行排队*/
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	for (ulScrOnuId = 1;ulScrOnuId <= MAXONUPERPON;ulScrOnuId++)
	{
		if ( 0 == scrUlOnuStr[ulScrOnuId-1] )
			continue;
		tempcount ++;
		lRet = cliCheckOnuMacValid( phyPonId, (ulScrOnuId-1) );
		if (lRet != VOS_OK )
		{
			if (1 == CountFirst)
			{/*如果只有1个onu,且不存在该mac地址,则直接返回,否则取下一个onuid*/
				vty_out( vty, "  %% Onu %d not exist!\r\n", ulScrOnuId);
				return CMD_WARNING;
			}
			else 
				continue;
		}
		
		for (ulDstOnuId = 1; ulDstOnuId <= MAXONUPERPON; ulDstOnuId++)
		{
			if ( 0 == dstUlOnuStr[ulDstOnuId-1] )
				continue;
			if ( ulDstOnuId == ulScrOnuId )
				continue;
			if(tempcount != 1)
				/*当目的onuid与源onuid都存在时,则需要判断当前源onuid是否大于当前目的onuid
				如果大于,则不用执行以下代码*/
				if ((scrUlOnuStr[ulDstOnuId-1] != 0) && (ulScrOnuId > ulDstOnuId) )
					continue;
				
			lRet = cliCheckOnuMacValid( phyPonId, (ulDstOnuId-1) );
			if (lRet != VOS_OK )
			{				
				if (1 == CountSecond)
				{
				    /*如果只有1个onu,且不存在该mac地址,则直接返回,否则取下一个onuid*/
					vty_out( vty, "  %% Onu %d not exist!\r\n", ulDstOnuId);
					return CMD_WARNING;
				}
				else 
					continue;
			}
				
			lRet = SetOnuPeerToPeer( phyPonId, (short int)(ulScrOnuId-1), (short int)(ulDstOnuId-1));
			if ((CountFirst == 1) && (CountSecond == 1))
			{
				if (VOS_ERROR == lRet)
                {
					vty_out( vty, "  %% Executing error.\r\n");
                }    
                else
                {
                    ulScrOnuId = MAXONUPERPON + 1;
    				break;
                }
			}
			else 
				continue;
		}
	}

    /* 尝试使能P2P，预配置的失败是正常的 */
    (void)OLT_SetOnuP2PMode(phyPonId, TRUE);
	
	return CMD_SUCCESS;
}


/*added by wutw 2006/12/13*/
DEFUN(
	peer_to_peer_delete, 
	peer_to_peer_delete_cmd,
	"undo onu p2p <onuid_list> <onuid_list>",
	NO_STR
	"delete onu p2p\n"
	"delete peer to peer onu\n"
	OnuIDStringDesc
	OnuIDStringDesc
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	ULONG ulScrOnuId = 0;
	ULONG ulDstOnuId = 0;
	ULONG CountFirst = 0;
	ULONG CountSecond = 0;
	ULONG scrUlOnuStr[MAXONUPERPONNOLIMIT] = {0};
	ULONG dstUlOnuStr[MAXONUPERPONNOLIMIT] = {0};
	int tempcount = 0;	

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
		CountFirst++;
		scrUlOnuStr[ulOnuId-1] = ulOnuId;/*将源端口进行排队*/
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	{
		CountSecond++;
		dstUlOnuStr[ulOnuId-1] = ulOnuId;/*将目的端口进行排队*/
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	for (ulScrOnuId = 1;ulScrOnuId <= MAXONUPERPON;ulScrOnuId++)
	{
		if ( 0 == scrUlOnuStr[ulScrOnuId-1] )
			continue;
		tempcount ++;
		lRet = cliCheckOnuMacValid( phyPonId, (ulScrOnuId-1) );
		if (lRet != VOS_OK )
		{
			if (1 == CountFirst)
			{
			    /*如果只有1个onu,且不存在该mac地址,则直接返回,否则取下一个onuid*/
				vty_out( vty, "  %% Onu %d not exist!\r\n", ulScrOnuId);
				return CMD_WARNING;
			}
			else 
				continue;
		}
		
		for (ulDstOnuId = 1; ulDstOnuId <= MAXONUPERPON; ulDstOnuId++)
		{
			if ( 0 == dstUlOnuStr[ulDstOnuId-1] )
				continue;
			if ( ulDstOnuId == ulScrOnuId )
				continue;
			if(tempcount != 1)
				/*当目的onuid与源onuid都存在时,则需要判断当前源onuid是否大于当前目的onuid
				如果大于,则不用执行以下代码*/
				if ((scrUlOnuStr[ulDstOnuId-1] != 0) && (ulScrOnuId > ulDstOnuId) )
					continue;
				
			/*vty_out( vty, "%d, ",ulDstOnuId);*/ 
			lRet = cliCheckOnuMacValid( phyPonId, (ulDstOnuId-1) );
			if (lRet != VOS_OK )
			{				
				if (1 == CountSecond)
				{
    				/*如果只有1个onu,且不存在该mac地址,则直接返回,否则取下一个onuid*/
					vty_out( vty, "  %% Onu %d not exist!\r\n", ulDstOnuId);
					return CMD_WARNING;
				}
				else 
					continue;
			}
			lRet = DiscOnuPeerToPeer( phyPonId, (short int)(ulScrOnuId-1), (short int)(ulDstOnuId-1) );
 			if ((CountFirst == 1) && (CountSecond == 1))
			{
				if (VOS_ERROR == lRet)
                {
					vty_out( vty, "  %% Executing error.\r\n");
                }            
                else
                {
                    ulScrOnuId = MAXONUPERPON + 1;
    				break;
                }
			}
			else 
				continue;
		}
	}

    /* 尝试关闭P2P，预配置的失败是正常的 */
    (void)OLT_SetOnuP2PMode(phyPonId, TRUE);
	
	return CMD_SUCCESS;
}

/*modified by chenfj  2007/04/26 
      问题单#4299: P2P第二个命令有待改进,将广播包转发功能隐含到配置p2p链接的命令中去， 而未知单播包的转发则可以另外配置
*/

DEFUN(
	peer_to_peer_forward_rule, 
	peer_to_peer_forward_rule_cmd,
	"onu p2p <onuid_list> forward address-not-found [enable|disable]",
	/*"onu p2p <onuid_list> forward address-not-found [enable|disable] broadcast [enable|disable]",*/
	"Config the onu\n"
	"Confine onu peer to peer onu\n"
	OnuIDStringDesc
	"Config onu peer to peer forward rule\n"
	"Config address-not-found frame transfers\n"
	"Enable address-not-found frame transfers\n"
	"Disable address-not-found frame transfers\n"
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
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	ULONG ulScrOnuId = 0;
	int count = 0;
	int	empty_count = 0;
	int	error_count = 0;
	int unicastFlag = 0;
	int brdFlag = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if (!VOS_StrCmp((CHAR *)argv[1], "enable"))
		unicastFlag = EPON_P2P_UNICAST_ENABLE;
	else /*if (!VOS_StrCmp((CHAR *)argv[1], "disable"))*/
		unicastFlag = EPON_P2P_UNICAST_DISABLE;
	/*else 
	{
		vty_out( vty, "  %% Parameter error!\r\n");
		return CMD_WARNING;
	}*/

	brdFlag = EPON_P2P_BRDCAST_ENABLE;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulScrOnuId )
	{	
	    count++;
        
		lRet = cliCheckOnuMacValid( phyPonId, (ulScrOnuId-1) );
		if (lRet != VOS_OK )
		{
		    error_count++;
            empty_count++;
            
			continue;
		}
		
		lRet = (LONG)SetOnuPeerToPeerForward( phyPonId, (short int)(ulScrOnuId-1), unicastFlag, brdFlag );
		if (lRet != VOS_OK)
		{
		    error_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% set onu p2p forward err\r\n");
        }

	   	return CMD_WARNING;
    }
		
	return CMD_SUCCESS;
}


/*added by wutw 2006/12/13*/
DEFUN(
	peer_to_peer_show, 
	peer_to_peer_show_cmd,
	"show onu p2p <onuid_list>",
	DescStringCommonShow
	"Show onu information\n"
	"Show the onu p2p information\n"
	OnuIDStringDesc
	)
{
	LONG lRet = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	ULONG ulScrOnuId = 0;
	int count = 0;
	int	empty_count = 0;
	int	noset_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulScrOnuId )
	{	
	    count++;
		lRet = (LONG)GetOnuIsSetPeerToPeer( phyPonId, (short int)(ulScrOnuId-1) );
		if (V2R1_ONU_SETING_P2P/*CLI_EPON_ONUCFG_P2P*/ == lRet)	/* modified by xieshl 20100325, 问题单9978 */
		{
			ShowOnuPeerToPeerByVty( phyPonId, (short int)(ulScrOnuId-1) , vty);
		}
		else
        {
            error_count++;
            switch(lRet)
            {
                case V2R1_ONU_NOT_EXIST:
                    empty_count++; /* modified by xieshl 20100325, 问题单9978 */
                    break;
                case V2R1_ONU_NOSETING_P2P:
                    noset_count++;
                    break;
            }
        }      
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (noset_count == count)
                || ((noset_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% onu no p2p setting.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% Executing error!\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

#endif

/* modified by xieshl 20160421, 支持超长光纤需求 */
#define ONU_RANGE_WINDOW_PON_CLI
#ifdef  ONU_RANGE_WINDOW_PON_CLI
DEFUN(
	pon_range_set,
	pon_range_set_cmd, 
	"onu-register window [20km|40km|60km|80km|close]", 
	"set onu register max window\n"
	"set onu register max window\n"
	"onu register window 20km\n"
	"onu register window 40km\n"
	"onu register window 60km\n"
	"onu register window 80km\n"
	"onu register window close\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	int iRes = 0;
	int range = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if (!VOS_StrCmp((CHAR *)argv[0], "20km"))
		range = PON_RANGE_20KM;
	else if (!VOS_StrCmp((CHAR *)argv[0], "40km"))
		range = PON_RANGE_40KM;
	else if (!VOS_StrCmp((CHAR *)argv[0], "60km"))
		range =PON_RANGE_60KM;
	else if (!VOS_StrCmp((CHAR *)argv[0], "80km"))
		range =PON_RANGE_80KM;
	else if(!VOS_StrCmp((CHAR *)argv[0], "close")) 
		range = PON_RANGE_CLOSE;
	else
	{
		/*vty_out( vty, "  %% Parameter error\r\n");*/
		return CMD_WARNING;
	}

	/*PonPortTable[phyPonId].range = range;*/
	/*配置恢复在ready状态开始 by jinhl@2017.04.24*/
	if (TRUE  == SYS_MODULE_IS_READY(SYS_LOCAL_MODULE_SLOTNO) )
	{
		if(OLTAdv_IsExist( phyPonId ) != TRUE )
		{
		
			vty_out(vty, "  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING );
		}
	}
	else/*if else 格式更方便阅读by jinhl*/
	{
		vty_out(vty, "  %% slot %d is not working\r\n", ulSlot );
		return( CMD_WARNING );
	}
	
	iRes = SetPonRange( phyPonId, range );
	if (iRes != VOS_OK)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(
	pon_range_show,
	pon_range_show_cmd, 
	"show onu-register window",
	DescStringCommonShow
	"show onu-register window\n"
	"show onu-register window\n"	
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	int range = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	iRes = GetPonRange( phyPonId, &range );
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "  pon %d/%d onu-register window is ", ulSlot, ulPort);
	if( PON_RANGE_20KM == range )
		vty_out( vty, "20km\r\n" );
	else if( PON_RANGE_40KM == range )
		vty_out( vty, "40km\r\n" );
	else if( PON_RANGE_60KM == range )		
		vty_out( vty, "60km\r\n" );
	else if( PON_RANGE_80KM == range )		
		vty_out( vty, "80km\r\n" );
	else if( PON_RANGE_CLOSE == range )
		vty_out( vty, "closed\r\n");
	else
	{
		vty_out( vty, "Unknown\r\n");
	}
	
	return CMD_SUCCESS;
}
#endif

DEFUN(
	pon_port_statstics_show,
	pon_port_statstics_show_cmd, 
	"show statistic pon",
	DescStringCommonShow
	"Show statstics information\n"
	"Show pon port statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int PonChipType = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	if( PonPortIsWorking(phyPonId) != TRUE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
	}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		iRes = CliRealTimeStatsPonForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsPon( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}


DEFUN(
	pon_gem_statstics_show,
	pon_gem_statstics_show_cmd, 
	"show statistic pon gem <0-4095>",
	DescStringCommonShow
	"Show statstics information\n"
	"Show pon port statstics\n"
	"Show gem port statstics\n"
	"gem val\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	INT16 gemId = 0;
	short int PonChipType = 0;
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	gemId = atoi(argv[0]);

	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	if( PonPortIsWorking(phyPonId) != TRUE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
	}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		iRes = CliRealTimeStatsGemForGpon( phyPonId, gemId, vty );
	}
	else
	{
		iRes=VOS_ERROR;
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	CNI_statstics_show,
	CNI_statstics_show_cmd, 
	"show statistic CNI",
	DescStringCommonShow
	"Show statstics information\n"
	"Show CNI statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int PonChipType = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	if( PonPortIsWorking(phyPonId) != TRUE ) 
		{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
		}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		iRes = CliRealTimeStatsCNIForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsCNI( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	pon_port_statstics_pon_datapath_show,
	pon_port_statstics_pon_datapath_show_cmd, 
	"show statistic pon datapath",
	DescStringCommonShow
	"Show statstics information\n"
	"Show pon port statstics\n"
	"Pon business traffic statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int PonChipType;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	if( PonPortIsWorking(phyPonId) != TRUE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
	}

	PonChipType = V2R1_GetPonchipType( phyPonId );
	if ( OLT_PONCHIP_ISGPON(PonChipType) )
	{
		iRes = CliRealTimeStatsPonDatapathForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsPonDatapath( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	CNI_statstics_cni_datapath_show,
	CNI_statstics_cni_datapath_show_cmd, 
	"show statistic CNI datapath",
	DescStringCommonShow
	"Show statstics information\n"
	"Show CNI statstics\n"
	"Cni business traffic statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
	short int PonChipType = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	if( PonPortIsWorking(phyPonId) != TRUE ) 
		{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
		}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if ( OLT_PONCHIP_ISGPON(PonChipType) )
	{
		/*CNI与CNI datapath数据一致*/
		iRes = CliRealTimeStatsCNIForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsCniDatapath( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	pon_statstics_datapath_uplink_show,
	pon_statstics_datapath_uplink_show_cmd, 
	"show statistic datapath uplink",
	DescStringCommonShow
	"Show statstics information\n"
	"Datapath statstics\n"
	"Uplink business traffic statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    INT16 phyPonId = 0;
	short int PonChipType = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	if( PonPortIsWorking(phyPonId) != TRUE ) 
		{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
		}

	PonChipType = V2R1_GetPonchipType( phyPonId );
	if ( OLT_PONCHIP_ISGPON(PonChipType) )
	{
		iRes = CliRealTimeStatsUplinkForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsUplink( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	pon_statstics_datapath_downlink_show,
	pon_statstics_datapath_downlink_show_cmd, 
	"show statistic datapath downlink",
	DescStringCommonShow
	"Show statstics information\n"
	"Datapath statstics\n"
	"Downlink business traffic statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int PonChipType = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	if( PonPortIsWorking(phyPonId) != TRUE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
	}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if ( OLT_PONCHIP_ISGPON(PonChipType) )
	{
		iRes = CliRealTimeStatsDownlinkForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsDownlink( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}


DEFUN(
	hostMsg_statstics_show,
	hostMsg_statstics_show_cmd, 
	"show statistic hostmsg",
	DescStringCommonShow
	"Show statstics information\n"
	"Show host msg statstics\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	if( PonPortIsWorking(phyPonId) != TRUE ) 
		{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[ulSlot], ulPort );
		return( ROK );
		}
	
	iRes = CliRealTimeStatsHostMsg( phyPonId, vty );
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

/* modified by chenfj 2009-6-10 将显示ONU 上、下行BER计数合并到一个命令中*/
DEFUN(
	onu_uplink_ber_show,
	onu_uplink_ber_show_cmd, 
		"show onu-ber <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	DescStringCommonShow
	"Show onu-ber\n"
	"Please input onuId\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int llid = 0;
    short int PonChipType;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
	return CMD_WARNING;

	ulOnuId = (( unsigned int ) VOS_AtoL( argv[ 0 ] ));

    CHECK_CMD_ONU_RANGE(vty, ulOnuId-1);

	/* 1 板在位检查
	 if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
	 {
	 vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
	 return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	if(ThisIsValidOnu(phyPonId,(ulOnuId-1)) == V2R1_ONU_NOT_EXIST)
		{
		vty_out( vty, "  %% onu%d/%d/%d is not exist\r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
		}
	if(GetOnuOperStatus(phyPonId,(ulOnuId-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out( vty, "  %% onu%d/%d/%d is off-line \r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
		}

	llid = GetLlidByOnuIdx( phyPonId, (ulOnuId-1));
	if (INVALID_LLID == llid ) 
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line \r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
	}
	
	PonChipType = V2R1_GetPonchipType( phyPonId );

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
	{
    	iRes = CliRealTimeStatsOnuUpLinkBer( phyPonId, (ulOnuId-1), vty );
    	if ( VOS_OK != iRes)
    	{	
    		vty_out( vty, "  %% uplink-ber Executing error\r\n");
    		/*return CMD_WARNING;*/
    	}
    	iRes = CliRealTimeOltDownlinkBer( phyPonId, (short int)(ulOnuId-1), vty );
    	if ( VOS_OK != iRes)
    	{	
    		vty_out( vty, "  %% downlink-ber Executing error\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(phyPonId));
    }
    
	return CMD_SUCCESS;
}


	
DEFUN(
	onu_uplink_fer_show,
	onu_uplink_fer_show_cmd, 
		"show onu-fer uplink <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	DescStringCommonShow
	"Show onu uplink information\n"
	"Show onu uplink fer\n"
	"Please input onuId\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    /*ULONG ulIfIndex = 0;*/
    INT16 phyPonId = 0;
	short int llid = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
	return CMD_WARNING;

	ulOnuId = (( unsigned int ) VOS_AtoL( argv[ 0 ] ));
    CHECK_CMD_ONU_RANGE(vty, ulOnuId-1);

	/* 1 板在位检查
	 if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
	 {
	 vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
	 return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	if(ThisIsValidOnu(phyPonId,(ulOnuId-1)) == V2R1_ONU_NOT_EXIST)
		{
		vty_out( vty, "  %% %d/%d/%d is not exist\r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
		}
	if(GetOnuOperStatus(phyPonId,(ulOnuId-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out( vty, "  %% %d/%d/%d is off-line \r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
		}
	
	llid = GetLlidByOnuIdx( phyPonId, (ulOnuId-1));
	if (INVALID_LLID == llid ) 
	{
		vty_out( vty, "  %% %d/%d/%d is off-line\r\n",ulSlot, ulPort, ulOnuId) ;
		return CMD_WARNING;
	}
	
	iRes = CliRealTimeOnuUpLinkFer( phyPonId, (ulOnuId-1), vty );
	if ( VOS_OK != iRes)
	{	
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}


#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )

DEFUN (
	set_ponport_hot_swap,
	set_ponport_hot_swap_cmd,
	"auto-protect partner "EPON_PON_SWAP_PORTRANGE,
	"set a pon auto-protection,default mode is slowness\n"
	"the partner auto-protect pon port\n"
	"please input the pon port\n"
	)
{
	unsigned long  ul_desSlot =0, ul_desPort = 0;
	unsigned long  ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Des_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	/* the dest pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[0], &ul_desSlot, &ul_desPort );
#else
	IFM_ParseSlotPort( argv[0], &ul_desSlot, &ul_desPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_desSlot, ul_desPort,vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(ul_desSlot,vty) != ROK )
		return(CMD_WARNING);

	Des_PonPort = GetPonPortIdxBySlot( (short int)ul_desSlot, (short int)ul_desPort );
	if (Des_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% 2nd pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	ret = EnablePonPortAutoProtect(ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort, V2R1_PON_PORT_SWAP_AUTO);
	if (ret != VOS_OK)
    {
        switch (ret)
        {
            case V2R1_IS_HOT_SWAP_PORT:
        		vty_out( vty, "  %% port protection already setting with this two port\r\n");
        		return( CMD_SUCCESS );
            case V2R1_PORT1_HAS_OTHER_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d already has protection setting with other port \r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_PORT2_HAS_OTHER_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d already has protection setting with other port \r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISDISABLED:
        		vty_out(vty,  "  %% pon auto-protect is disabled\r\n");
                break;
            case V2R1_SWAP_PORT_ALL_REMOTE:
        		vty_out(vty,  "  %% pon auto-protect can't set two remote pon\r\n");
                break;
            case V2R1_SWAP_PORT_ALL_ONE:
        		vty_out(vty,  "  %% two pon port are same\r\n");
                break;
            case V2R1_SWAP_RPCMODE_NOTSUPPORT:
        		vty_out(vty,  "  %% please exec cmd:[cdp sync mode async] in hidden node\r\n");
                break;
            default:
        		vty_out( vty, "  %% The pair pons failed to set \r\n");
        }
        
		return( CMD_WARNING );
    }   
	
    return CMD_SUCCESS;
}
/*Begin:for onu swap by jinhl@2013-02-22*/
extern int do_protect_onuswap(struct vty *vty);
#if ( (EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES) || (EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES) )
DEFUN (
	set_ponport_hot_swap_full,
	set_ponport_hot_swap_full_cmd,
	"auto-protect partner "EPON_PON_SWAP_PORTRANGE" mode ["EPON_PON_SWAP_MODE"]", /* <gslot/gport> */
	"set a pon auto-protection\n"
	"the partner auto-protect "EPON_PON_SWAP_PORTTYPE"port\n"
	"please input the "EPON_PON_SWAP_PORTTYPE"port\n"
	"the auto-protect's work mode\n"
	EPON_PON_SWAP_MODE_DESC
	)
{
	unsigned long  ul_desSlot =0, ul_desPort = 0;
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Des_PonPort;
    int iSwapMode;
	int ret;
	ULONG ulOnuId = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	/* the dest pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[0], &ul_desSlot, &ul_desPort );
#else
	IFM_ParseSlotPort( argv[0], &ul_desSlot, &ul_desPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_desSlot, ul_desPort, vty) != ROK)
		return(CMD_WARNING);
	Des_PonPort = GetPonPortIdxBySlot( (short int)ul_desSlot, (short int)ul_desPort );
	if (Des_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %%  pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}
   
    if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_SLOWLY_STR, argv[1]) )
    {
        iSwapMode = V2R1_PON_PORT_SWAP_SLOWLY;
    }
    else
    {
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_QUICKLY_STR, argv[1]) )
        {
            iSwapMode = V2R1_PON_PORT_SWAP_QUICKLY;
        }
        else
#endif
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_ONU_OPTIC_STR, argv[1]) )
        {
            iSwapMode = V2R1_PON_PORT_SWAP_ONU;
			
        }
        else
#endif
        {
    		vty_out( vty, "  %% mode Parameter is error.\r\n" );

            VOS_ASSERT(0);
    		return CMD_WARNING;
        }
    }

	ret = EnablePonPortAutoProtect(ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort, iSwapMode);
	if (ret != VOS_OK)
    {
        switch (ret)
        {
            case V2R1_IS_HOT_SWAP_PORT:
        		vty_out( vty, "  %% port protection already setting with this two port\r\n");
        		return( CMD_SUCCESS );
            case V2R1_PORT1_HAS_OTHER_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d already has protection setting with other port \r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_PORT2_HAS_OTHER_HOT_SWAP_PORT:
                /* B--modified by liwei056@2010-10-13 for D10860 */
        		vty_out( vty, "  %% pon %d/%d already has protection setting with other port \r\n", ul_desSlot, ul_desPort );
                /* E--modified by liwei056@2010-10-13 for D10860 */
                break;
            case V2R1_SWAP_PORT_ISDISABLED:
        		vty_out(vty,  "  %% pon auto-protect is disabled\r\n");
                break;
            case V2R1_SWAP_PORT_ALL_REMOTE:
        		vty_out(vty,  "  %% pon auto-protect can't set two remote pon\r\n");
                break;
            case V2R1_SWAP_PORT_ALL_ONE:
        		vty_out(vty,  "  %% two pon port are same\r\n");
                break;
            case V2R1_SWAP_RPCMODE_NOTSUPPORT:
        		vty_out(vty,  "  %% please exec cmd:[cdp sync mode async] in hidden node\r\n");
                break;
			/*for onu swap by jinhl@2013-04-27*/
			case V2R1_SWAP_PORT_HASONU:
				vty_out( vty, "  %% There are onus on the pon ports. We will deregister all onus automatically if you confirm to do this config. Are you sure to continue? [Y/N]\r\n" );
                vty->prev_node = vty->node;
				vty->node = CONFIRM_ACTION_NODE;
				vty->action_func = do_protect_onuswap;
		        VTY_CONFIRM_CB_SETPARAM1(vty, Src_PonPort);
	            VTY_CONFIRM_CB_SETPARAM2(vty, Des_PonPort);
	    		return CMD_SUCCESS;
            default:
                if (V2R1_PON_PORT_SWAP_SLOWLY < iSwapMode )
                {
            		vty_out( vty, "  %% The pair pons don't support the %s auto-protect mode. \r\n", argv[2]);
                }
                else
                {
            		vty_out( vty, "  %% The pair pons failed to set \r\n");
                }
        }
        
		return( CMD_WARNING );
    }   

    return CMD_SUCCESS;
}
#endif
/*End:for onu swap by jinhl@2013-02-22*/
/*for onu swap by jinhl@2013-04-27*/
extern int undo_protect_onuswap(struct vty *vty);
DEFUN (
	undo_set_ponport_hot_swap,
	undo_set_ponport_hot_swap_cmd,
	"undo auto-protect",
	"clear a pon auto-protection\n"
	"clear a pon auto-protection\n"
	)
{

	unsigned long ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	ret = DisablePonPortAutoProtect(ul_srcSlot, ul_srcPort );
	
	if( ret ==  V2R1_PORT_HAS_NO_HOT_SWAP_PORT ) 
	{
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );
		return( CMD_WARNING );
	}
	/*Begin:for onu swap by jinhl@2013-02-22*/
	else if( ret == V2R1_SWAP_PORT_HASONU)
	{
	    vty_out( vty, "  %% There are onus on the pon ports. We will deregister all onus automatically if you confirm to do this config. Are you sure to continue? [Y/N]\r\n" );
        vty->prev_node = vty->node;
		vty->node = CONFIRM_ACTION_NODE;
		vty->action_func = undo_protect_onuswap;
        VTY_CONFIRM_CB_SETPARAM1(vty, Src_PonPort);
        
		return CMD_SUCCESS;
	}
	/*End:for onu swap by jinhl@2013-02-22*/
	else if ( ret == RERROR ) 
	{
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
	}  
        
    return CMD_SUCCESS;
}

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
DEFUN (
	op_trigger_ponport_hot_swap,
	op_trigger_ponport_hot_swap_cmd,
	"trigger-switch optical-power",
	"config pon switch's trigger\n"
	"trigger pon switch at optical-power's abnormal\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_OPTICPOWER )
    {
		vty_out(vty, "  %% The trigger have been enabled.\r\n");
		return( CMD_WARNING );
    }
    else
    {
        iCurrTriggers |= PROTECT_SWITCH_TRIGGER_OPTICPOWER;
    }

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }
        
    return CMD_SUCCESS;
}

DEFUN (
	op_untrigger_ponport_hot_swap,
	op_untrigger_ponport_hot_swap_cmd,
	"undo trigger-switch optical-power",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"untrigger pon switch at optical-power's abnormal\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_OPTICPOWER )
    {
        iCurrTriggers &= ~PROTECT_SWITCH_TRIGGER_OPTICPOWER;
    }
    else
    {
		vty_out(vty, "  %% The trigger have been disabled.\r\n");
		return( CMD_WARNING );
    }

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }
        
    return CMD_SUCCESS;
}
#endif

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
DEFUN (
	oe_trigger_ponport_hot_swap,
	oe_trigger_ponport_hot_swap_cmd,
	"trigger-switch data-error",
	"config pon switch's trigger\n"
	"trigger pon switch at optical-data's received abnormal\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_OPTICERROR )
    {
		vty_out(vty, "  %% The trigger have been enabled.\r\n");
		return( CMD_WARNING );
    }
    else
    {
        iCurrTriggers |= PROTECT_SWITCH_TRIGGER_OPTICERROR;
    }

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }
        
    return CMD_SUCCESS;
}

DEFUN (
	oe_untrigger_ponport_hot_swap,
	oe_untrigger_ponport_hot_swap_cmd,
	"undo trigger-switch data-error",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"untrigger pon switch at optical-data's received abnormal\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_OPTICERROR )
    {
        iCurrTriggers &= ~PROTECT_SWITCH_TRIGGER_OPTICERROR;
    }
    else
    {
		vty_out(vty, "  %% The trigger have been disabled.\r\n");
		return( CMD_WARNING );
    }

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }
        
    return CMD_SUCCESS;
}
#endif

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
DEFUN (
	ud_trigger_ponport_hot_swap,
	ud_trigger_ponport_hot_swap_cmd,
	"trigger-switch uplink-down <slot/port>",
	"config pon switch's trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"trigger pon switch at uplink-port's link-down\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	unsigned long ul_desSlot = 0, ul_desPort = 0;
	unsigned long ulIfindex;
	short int  Src_PonPort, Partner_PonPort;
	int iCurrTriggers;
	int iRlt;
	unsigned char ucSlot, ucPort;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

	if( IFM_ParseSlotPort( argv[0], &ul_desSlot, &ul_desPort ) != VOS_OK )
		return CMD_WARNING;
	if( olt_eth_idx_check(ul_desSlot, ul_desPort, vty) == VOS_ERROR )
		return CMD_WARNING;

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
    {
		vty_out(vty, "  %% The trigger have been enabled.\r\n");
		return( CMD_WARNING );
    }
    else
    {
        iCurrTriggers |= PROTECT_SWITCH_TRIGGER_UPLINKDOWN;
    }

    ulIfindex = userSlot_userPort_2_Ifindex(ul_desSlot, ul_desPort);
    if ( VOS_OK == IFM_GetIfPartnerApi(ulIfindex, &ucSlot, &ucPort) )
    {
        /* 正向从Eth查询PON保护配置 */
        if ( (0 != ucSlot) || (0 != ucPort) )
        {
    		vty_out( vty, " %% eth%d/%d have been binded to pon%d/%d, please release it at first.\r\n", ul_desSlot, ul_desPort, ucSlot, ucPort );
    		return VOS_ERROR;
        }
    }
    else
    {
        short int EthPortIdx;
        short int PonPortIdx;
    
        /* 反向从PON查询Eth保护配置 */
        EthPortIdx = OLT_DEVICE_ID(ul_desSlot, ul_desPort);
        if ( ROK == PonPortProtectedPortLocalQuery(EthPortIdx, &PonPortIdx) )
        {
            ul_srcSlot = GetCardIdxByPonChip(PonPortIdx);
            ul_srcPort = GetPonPortByPonChip(PonPortIdx);
    		vty_out( vty, " %% eth%d/%d have been binded to pon%d/%d, please release it at first.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
    		return VOS_ERROR;
        }
    }

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }

    if ( VOS_OK != IFM_SetIfPartnerApi(ulIfindex, (UCHAR)ul_srcSlot, (UCHAR)ul_srcPort) )
    {
        if ( PonPortIsWorking(Src_PonPort) )
        {
        	sendPonSwitchEventMsg( Src_PonPort, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, PROTECT_SWITCH_STATUS_PASSIVE, PonPortSwapTimesQuery(Src_PonPort) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
    		vty_out(vty, "  %% eth%d/%d is not existed, pon%d/%d is stoping work now.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort);
        }
    }
    else
    {
        if ( PonPortIsWorking(Src_PonPort) )
        {
            ULONG ulPortIsLinkUp;
            
            if ( VOS_OK == IFM_GetIfStatusApi(ulIfindex, &ulPortIsLinkUp) )
            {
                if ( ulPortIsLinkUp )
                {
                	sendPonSwitchEventMsg( Src_PonPort, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_ACTIVE, PonPortSwapTimesQuery(Src_PonPort) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
            		vty_out(vty, "  %% eth%d/%d is linkup, pon%d/%d is working normal.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort);
                }
                else
                {
                	sendPonSwitchEventMsg( Src_PonPort, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, PROTECT_SWITCH_STATUS_PASSIVE, PonPortSwapTimesQuery(Src_PonPort) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
            		vty_out(vty, "  %% eth%d/%d is linkdown, pon%d/%d is stoping work now.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort);
                }
            }
        }
    }

    return CMD_SUCCESS;
}

DEFUN (
	ud_untrigger_ponport_hot_swap,
	ud_untrigger_ponport_hot_swap_cmd,
	"undo trigger-switch uplink-down",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"untrigger pon switch at uplink-port's link-down\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0, ulOnuId = 0;
	unsigned long ul_desSlot = 0, ul_desPort = 0;
    unsigned long ulIfindex;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif    

    iCurrTriggers = GetPonPortHotSwapTriggers(Src_PonPort);
    if ( iCurrTriggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
    {
        iCurrTriggers &= ~PROTECT_SWITCH_TRIGGER_UPLINKDOWN;
    }
    else
    {
		vty_out(vty, "  %% The trigger have been disabled.\r\n");
		return( CMD_WARNING );
    }

    (void)GetPonPortHotProtectedPort(Src_PonPort, &ul_desSlot, &ul_desPort);
    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, -1, -1) )
    {
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
    }

    if ( (ul_desSlot != 0) && (ul_desPort != 0) )
    {
        ulIfindex = userSlot_userPort_2_Ifindex(ul_desSlot, ul_desPort);
        if ( VOS_OK != IFM_SetIfPartnerApi(ulIfindex, (UCHAR)0, (UCHAR)0) )
        {
            if ( PonPortIsWorking(Src_PonPort) )
            {
            	sendPonSwitchEventMsg( Src_PonPort, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_ACTIVE, PonPortSwapTimesQuery(Src_PonPort) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
        		vty_out(vty, "  %% eth%d/%d is not existed, pon%d/%d is working normal.\r\n", ul_desSlot, ul_desPort);
            }
        }
        else
        {
            if ( PonPortIsWorking(Src_PonPort) )
            {
                ULONG ulPortIsLinkUp;
                
                if ( VOS_OK == IFM_GetIfStatusApi(ulIfindex, &ulPortIsLinkUp) )
                {
                    if ( ulPortIsLinkUp )
                    {
                		vty_out(vty, "  %% eth%d/%d is linkup, pon%d/%d is worked normal.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort);
                    }
                    else
                    {
                    	sendPonSwitchEventMsg( Src_PonPort, PROTECT_SWITCH_EVENT_NOTIFY, PROTECT_SWITCH_REASON_UPLINKCNG, PROTECT_SWITCH_EVENT_SRC_HARDWARE, 0, V2R1_PON_PORT_SWAP_ACTIVE, PonPortSwapTimesQuery(Src_PonPort) + 1, PROTECT_SWITCH_EVENT_FLAGS_NONE );
                		vty_out(vty, "  %% eth%d/%d is linkdown, pon%d/%d is working normal.\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort);
                    }
                }
            }
        }
    }
        
    return CMD_SUCCESS;
}

#endif

/*for onu swap by jinhl@2013-04-27*/
/*****************************************************************
pon口上有onu注册，又确定要配成onu保护倒换模式，
或者从onu保护倒换模式切换到其它模式，
强制关闭光口，待所有onu离线后，进行配置。
*****************************************************************/
int do_changeto_swapmode(struct vty *vty)
{
	short int  PonPortIdx = 0, PartnerPonPortIdx = 0;
    int swapmode = 0;
	int ret = 0;
	
    PonPortIdx = VTY_CONFIRM_CB_GETPARAM1(vty);
	swapmode = VTY_CONFIRM_CB_GETPARAM2(vty);
	PonPortSwapPortQuery( PonPortIdx, &PartnerPonPortIdx );

	OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_SWITCH);
	OLTAdv_SetOpticalTxMode2(PartnerPonPortIdx, FALSE, PONPORT_TX_SWITCH);
	
    while(HasOnuOnlineOrRegistering(PonPortIdx) || HasOnuOnlineOrRegistering(PartnerPonPortIdx))
	{
	    VOS_TaskDelay(VOS_TICK_SECOND/2);
	}
	
	/*这里再加个延时，若不延时，即使通过PAS_get_onu_parameters获取
	onu的数目是0，pas_soft设置冗余配置PAS_set_redundancy_config还是会出错*/
	VOS_TaskDelay(VOS_TICK_SECOND*5);
	
	ret = ActivePonHotSwapPort( PonPortIdx, PartnerPonPortIdx, V2R1_PON_PORT_SWAP_DISABLED, V2R1_PON_PORT_SWAP_SYNC);
	if(EXIT_OK == ret)
	{
	    ret = ActivePonHotSwapPort( PonPortIdx, PartnerPonPortIdx, swapmode, V2R1_PON_PORT_SWAP_SYNC);
	}
	if( V2R1_PON_PORT_SWAP_ONU == swapmode)
	{
		OLTAdv_SetOpticalTxMode2(PonPortIdx, TRUE, PONPORT_TX_SWITCH);
		OLTAdv_SetOpticalTxMode2(PartnerPonPortIdx, TRUE, PONPORT_TX_SWITCH);
	}
	/*for onu swap by jinhl@2013-06-14*/
	else 
	{
	    if(PonPortHotStatusQuery( PonPortIdx ) == V2R1_PON_PORT_SWAP_ACTIVE)
    	{
    	    OLTAdv_SetOpticalTxMode2(PonPortIdx, TRUE, PONPORT_TX_SWITCH);
    	}
		else if(PonPortHotStatusQuery( PartnerPonPortIdx ) == V2R1_PON_PORT_SWAP_ACTIVE)
    	{
    	    OLTAdv_SetOpticalTxMode2(PartnerPonPortIdx, TRUE, PONPORT_TX_SWITCH);
    	}
		
	}
	if (ret != VOS_OK)
    {
        vty_out(vty, "  %% executing err\r\n");
        return( CMD_WARNING );
    }
    
	return CMD_SUCCESS;
}
/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#if ( (EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES) || (EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES) )
DEFUN (
	set_ponport_hot_swap_mode,
	set_ponport_hot_swap_mode_cmd,
	"auto-protect mode ["EPON_PON_SWAP_MODE"]",
	"set a pon auto-protection\n"
	"the work mode auto-protect pon port\n"
	EPON_PON_SWAP_MODE_DESC
	)
{
	ULONG ul_srcSlot, ul_srcPort;
	ULONG ulOnuId;
	short int  Src_PonPort;
	int ret;
    int iSwapMode;

	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

    if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_SLOWLY_STR, argv[0]) )
    {
        iSwapMode = V2R1_PON_PORT_SWAP_SLOWLY;
    }
    else
    {
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_QUICKLY_STR, argv[0]) )
        {
            iSwapMode = V2R1_PON_PORT_SWAP_QUICKLY;
        }
        else
#endif
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_ONU_OPTIC_STR, argv[0]) )
        {
            iSwapMode = V2R1_PON_PORT_SWAP_ONU;
        }
        else
#endif
        {
    		vty_out( vty, "  %% mode Parameter is error.\r\n" );

            VOS_ASSERT(0);
    		return CMD_WARNING;
        }
    }

	ret = SetPonPortAutoProtectMode(ul_srcSlot, ul_srcPort, iSwapMode);
	if (ret != VOS_OK)
    {
        switch (ret)
        {
            case V2R1_PORT_HAS_NO_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISNOT_WORKING:
            case V2R1_SWAP_PORT_ISUNSUPPORTED:
        		vty_out( vty, "  %% The pair pons don't support the %s mode. \r\n", argv[0]);
                break;
			/*Begin:for onu swap by jinhl@2013-02-22*/
			case V2R1_SWAP_MODE_ALL_ONE:
				vty_out( vty, "  %% The mode is the same as current mode, no change. \r\n");
                break;
			case V2R1_SWAP_PORT_HASONU:
				vty_out( vty, "  %% There are onus on the pon ports. We will deregister all onus automatically if you confirm to do this config. Are you sure to continue? [Y/N]\r\n" );
                vty->prev_node = vty->node;
				vty->node = CONFIRM_ACTION_NODE;
				/*Begin:for onu swap by jinhl@2013-06-14*/
				vty->action_func = do_changeto_swapmode;
		        VTY_CONFIRM_CB_SETPARAM1(vty, Src_PonPort);
				VTY_CONFIRM_CB_SETPARAM2(vty, iSwapMode);
	            /*End:for onu swap by jinhl@2013-06-14*/
	    		return CMD_SUCCESS;
			/*End:for onu swap by jinhl@2013-02-22*/
            default:
        		vty_out(vty, "  %% executing err\r\n");
        }
        
		return( CMD_WARNING );
    }    
        
    return CMD_SUCCESS;
}

DEFUN (
	show_ponport_hot_swap_mode,
	show_ponport_hot_swap_mode_cmd,
	"show auto-protect mode",
	"show pon auto-protect switching mode\n"
	"show pon auto-protect switching mode\n"
	"show pon auto-protect switching mode\n"
	)
{
	ULONG ul_srcSlot, ul_srcPort;
	ULONG ulOnuId;
	int ret;
	ULONG ulIfIndex;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	ret = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ul_srcSlot, (ULONG *)&ul_srcPort , (ULONG *)&ulOnuId);	
	if (ret != VOS_OK)
	{
    	vty_out( vty, "  %% Executing error\r\n");
    	return CMD_WARNING;
	}

	ret = ShowPonPortAutoProtectMode(vty, ul_srcSlot, ul_srcPort );
	if( ret ==  V2R1_PORT_HAS_NO_HOT_SWAP_PORT ) 
	{
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );
		return( CMD_WARNING );
	}
	else if ( ret == RERROR ) 
	{
		vty_out(vty, "  %% executing err\r\n");
		return( CMD_WARNING );
	}  
        
    return CMD_SUCCESS;
}

#endif

#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
DEFUN (
	show_ponport_hot_swap_status,
	show_ponport_hot_swap_status_cmd,
	"show auto-protect status",
	"show pon auto-protect\n"
	"show pon auto-protect\n"
	"show pon auto-protect's current status\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned long ul_desSlot = 0, ul_desPort=0;
	unsigned long ul_MSlot, ul_MPort;
	unsigned long ul_SSlot, ul_SPort;
	short int  Src_PonPort, Partner_PonPort;
	short int  M_PonPort, S_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	short int iProtectMode = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	ret = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
	if( ret != ROK ) 
	{
		vty_out(vty, "  %% pon %d/%d has no protect port\r\n", ul_srcSlot, ul_srcPort);
		return( CMD_WARNING );
	}

	Partner_PonPort = GetPonPortIdxBySlot( (short int)ul_desSlot, (short int)ul_desPort );
	if (Partner_PonPort == VOS_ERROR)
	{
		vty_out(vty, "  %% pon %d/%d has no protect port\r\n", ul_srcSlot, ul_srcPort);
		return CMD_WARNING;
	}
    /*Begin:for onu swap by jinhl@2013-02-22*/
    iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
	if(V2R1_PON_PORT_SWAP_ONU == iProtectMode)
	{
		vty_out( vty, "  ACTIVE PON: %d/%d ", ul_srcSlot, ul_srcPort);
		if ( PonPortIsWorking(Src_PonPort) )
		{
			vty_out( vty, "is working.");
		}
		else
		{
			vty_out( vty, "is not working.");
		}
		vty_out( vty, "\r\n  ACTIVE PON: %d/%d ", ul_desSlot, ul_desPort);
	    if ( PonPortIsWorking(Partner_PonPort) )
	    {
        	vty_out( vty, "is working.");
	    }
	    else
	    {
	    	vty_out( vty, "is not working.");
	    }
		vty_out( vty, "\r\n");
	}
	else
	{
	    if ( PonPortHotStatusQuery( Src_PonPort ) == V2R1_PON_PORT_SWAP_ACTIVE )
	    {
	        M_PonPort = Src_PonPort;
	        S_PonPort = Partner_PonPort;

	        ul_MSlot = ul_srcSlot;
	        ul_MPort = ul_srcPort;
	        ul_SSlot = ul_desSlot;
	        ul_SPort = ul_desPort;
	    }
	    else
	    {
	        M_PonPort = Partner_PonPort;
	        S_PonPort = Src_PonPort;

	        ul_MSlot = ul_desSlot;
	        ul_MPort = ul_desPort;
	        ul_SSlot = ul_srcSlot;
	        ul_SPort = ul_srcPort;
	    }
	    
		vty_out( vty, "  ACTIVE PON: %d/%d ", ul_MSlot, ul_MPort);
	    if ( PonPortIsWorking(M_PonPort) )
	    {
	    	vty_out( vty, "is working.");
	    }
	    else
	    {
	    	vty_out( vty, "is not working.");
	    }

		vty_out( vty, "\r\n  PASSIVE PON: %d/%d ", ul_SSlot, ul_SPort);
	    if ( PonPortIsWorking(S_PonPort) )
	    {
	        if ( OLTAdv_RdnIsReady(S_PonPort) )
	        {
	        	vty_out( vty, "is ready to switch.");
	        }
	        else
	        {
	        	vty_out( vty, "is working, but not ready to switch.");
	        }
	    }
	    else
	    {
	    	vty_out( vty, "is not working.");
	    }
		vty_out( vty, "\r\n");
	}
    /*End:for onu swap by jinhl@2013-02-22*/
	return( CMD_SUCCESS );
}
#endif
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */


DEFUN (
	force_ponport_hot_swap,
	force_ponport_hot_swap_cmd,
	"force-switching",
	"force pon switching to this pon port\n"
	)
{

	ULONG ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ul_srcSlot, ul_srcPort) != VOS_OK )
		return CMD_WARNING;

	ret = ForcePonPortAutoProtect(ul_srcSlot, ul_srcPort );
   	if (ret != VOS_OK)
    {
        switch (ret)
        {
            case V2R1_PORT_HAS_NO_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISNOT_WORKING:
        		vty_out(vty, "  %% pon %d/%d is not working\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_BUT_NOT_ACTIVE:
        		vty_out(vty, "  %% pon %d/%d is not ready to switch\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISDISABLED:
        		vty_out(vty, "  %% pon auto-protect is disabled\r\n");
                break;
            case RERROR:
        		vty_out(vty, "  %% executing err\r\n");
                break;
            default:
        		vty_out(vty, "  %% executing err\r\n");
        }
        
        return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
}

/* B--added by liwei056@2011-12-14 for AddManualSyncCmd */
DEFUN (
	sync_ponport_hot_swap,
	sync_ponport_hot_swap_cmd,
	"syncfg",
	"sync the master pon's configs to the slave pon\n"
	)
{
	ULONG ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	short int iProtectMode = 0;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ul_srcSlot, ul_srcPort) != VOS_OK )
		return CMD_WARNING;
    /*Begin:for onu swap by jinhl@2013-02-22*/
    iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
	if(V2R1_PON_PORT_SWAP_ONU == iProtectMode) 
	{
		vty_out( vty, "  %% The pon port is onu protect mode,err.\r\n" );

		return CMD_WARNING;
	}
	/*End:for onu swap by jinhl@2013-02-22*/
	ret = SyncPonPortAutoProtect(ul_srcSlot, ul_srcPort);
   	if (ret != VOS_OK)
    {
        switch (ret)
        {
            case V2R1_PORT_HAS_NO_HOT_SWAP_PORT:
        		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISNOT_WORKING:
        		vty_out(vty, "  %% pon %d/%d is not working\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_BUT_NOT_ACTIVE:
        		vty_out(vty, "  %% pon %d/%d is not ready to sync\r\n", ul_srcSlot, ul_srcPort );
                break;
            case V2R1_SWAP_PORT_ISDISABLED:
        		vty_out(vty, "  %% pon auto-protect is disabled\r\n");
                break;
            case RERROR:
        		vty_out(vty, "  %% executing err\r\n");
                break;
            default:
        		vty_out(vty, "  %% executing err\r\n");
        }
        
        return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
}

DEFUN (
	show_ponport_hot_swap,
	show_ponport_hot_swap_cmd,
	"show auto-protect",
	"show pon auto-protect\n"
	"show pon auto-protect\n"
	)
{
#ifdef __CTC_TEST
extern int  pon_force_switch_enable;
extern VOID display_pon_aps_status(struct vty *vty, short int  Src_PonPort, ULONG ul_srcSlot, ULONG ul_srcPort, ULONG ul_desSlot, ULONG ul_desPort);
#endif

	unsigned long ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned long ul_desSlot = 0, ul_desPort=0;
	short int  Src_PonPort;
	int ret;
	ULONG ulOnuId = 0;
	short int iProtectMode = 0;
    int state = 0;
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;

	ret = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
	if( ret != ROK ) 
	{
		vty_out(vty, "  %% pon %d/%d has no protect port\r\n", ul_srcSlot, ul_srcPort);
		return( CMD_WARNING );
	}

    /*Begin:for onu swap by jinhl@2013-02-22*/
	iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
	if(V2R1_PON_PORT_SWAP_ONU != iProtectMode)
	{
		vty_out( vty, "  ACTIVE PON       PASSIVE PON \r\n");

#ifdef __CTC_TEST
		display_pon_aps_status( vty, Src_PonPort, ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort);
#else
	    /* B--modified by liwei056@2010-4-6 for D9997 */
		if( PonPortHotStatusQuery( Src_PonPort ) == V2R1_PON_PORT_SWAP_ACTIVE )
			vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
		else 
			vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
	    /* E--modified by liwei056@2010-4-6 for D9997 */
#endif
	}
	else
	{
	    vty_out( vty, "  ODD PON           EVEN PON \r\n");
	    	
		ret = OLT_GetRdnState(Src_PonPort, &state);
		if((EXIT_OK == ret) && (PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID == state))
		{
			vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
		}
		else if((EXIT_OK == ret) && (PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID == state))
		{
			vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
		}
		else
		{
			vty_out( vty, "   %2d/%-2d              %2d/%-2d   unknowed llid-method\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
		}
	}
	/*End:for onu swap by jinhl@2013-02-22*/

	return( CMD_SUCCESS );
}

/* B--added by liwei056@2012-8-7 for D14913 */
DEFUN (
	show_ponport_trigger_swap,
	show_ponport_trigger_swap_cmd,
	"show trigger-switch",
	"show pon switch's trigger\n"
	"show pon switch's trigger\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0 ;
	ULONG ulOnuId = 0;
	short int  Src_PonPort;
	
	if( parse_pon_command_parameter( vty, &ul_srcSlot, &ul_srcPort , &ulOnuId, &Src_PonPort) != VOS_OK )
		return CMD_WARNING;
    
    ShowPonPortAutoProtectTrigger(vty, ul_srcSlot, ul_srcPort);
        
	return( CMD_SUCCESS );
}
/* E--added by liwei056@2012-8-7 for D14913 */
#endif


#ifdef  PON_PORT_ADMIN_STATUS
#endif

/* added by chenfj 2007-6-27  
     PON 端口管理使能: up / down  */
DEFUN (
	shutdown_ponport,
	shutdown_ponport_cmd,
	"shutdown pon port",
	"shutdown the pon port\n"
	"shutdown the pon port\n"
	"shutdown the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	int iRes = 0;*/

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	ShutdownPonPort( phyPonId );

	return CMD_SUCCESS;
}

DEFUN (
	undo_shutdown_ponport,
	undo_shutdown_ponport_cmd,
	"undo shutdown pon port",
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
   	INT16 phyPonId = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/*
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
		return( CMD_WARNING);
		}
	*/
	StartupPonPort( phyPonId );	
	return CMD_SUCCESS;
}
DEFUN (
	deregister_onu,
	deregister_onu_cmd,
	"deregister onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	"deregister the onu\n"
	"deregister the onu\n"
	"please input the onu's llid\n"	
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
    ULONG llid = VOS_StrToUL( argv[0], NULL, 10 );
    
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if(VOS_OK != OLT_DeregisterLLID( phyPonId, (short int )llid, 0))
    {
        vty_out(vty, " %%Execute command failed!\r\n");
        return CMD_WARNING;
    }
	    

	return CMD_SUCCESS;
}

#if 0
DEFUN (
	reset_ponport,
	reset_ponport_cmd,
	"reset",
	"reset the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    	INT16 phyPonId = 0;
	ULONG ulIfIndex=0;
	int iRes = 0;


	if( argc != 0 ) 
		{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
		}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	iRes = PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuId);	
	if (iRes != VOS_OK)
	{
		vty_out( vty, "  %% getting parameter error\r\n");
		return CMD_WARNING;
	}
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
	    vty_out( vty, "  %% pon parameter error\r\n");
	    return CMD_WARNING;
	}
			
	ResetPonPort( phyPonId );							

	return CMD_SUCCESS;
}
#endif

#ifdef  ONU_DEVICE_DEACTIVATE 
#endif
	/* added by chenfj 2007-6-27 
	    ONU	 device  de-active */

/* added by xieshl 20111011, 统一去激活ONU命令实现；不管CTC协议栈是否打开，去激活ONU时都不再依据ONU认证功能实现，
    而是直接将ONU放入pending队列，以避免ONU反复离线注册。问题单13516 */
extern LONG cli_set_onu_traffic_service_enable( struct vty *vty, ULONG ulSlot, ULONG ulPort, ULONG ulOnuid, LONG enable );
DEFUN (
	pon_node_deactivate_onu_device,
	pon_node_deactivate_onu_device_cmd,
	"deactivate onu traffic service <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"please input the onu\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet;*/
	int result = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
		{
		vty_out( vty, "  %% onuidx error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

	if(ThisIsValidOnu( phyPonId, (ulOnuid-1))  != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
		}

	/*if( V2R1_CTC_STACK )*/
	if(GetOnuVendorType( phyPonId, (ulOnuid - 1 ) ) == ONU_VENDOR_CT )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			vty_out(vty, " ONU authentication should be enable first\r\n");
			return( CMD_WARNING );
			}		
		}
	
	result = SetOnuTrafficServiceEnable( phyPonId, (ulOnuid - 1 ), V2R1_DISABLE );
	if( result ==  V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	/*
	else if(  result == ONU_OPER_STATUS_DOWN  ) 
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	*/
	else if( result == RERROR )
		{
		vty_out( vty, "  %% Executing error \r\n");
		return( CMD_WARNING );
		}

	return CMD_SUCCESS;
#else
	if( PON_GetSlotPortOnu((ULONG)(vty->index), &ulSlot, &ulPort , &ulOnuid) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    CHECK_CMD_ONU_RANGE(vty, ulOnuid-1);
	
	return cli_set_onu_traffic_service_enable( vty, ulSlot, ulPort, ulOnuid, V2R1_DISABLE );
#endif
}

DEFUN (
	undo_pon_node_deactivate_onu_device,
	undo_pon_node_deactivate_onu_device_cmd,
		"undo deactivate onu traffic service <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	NO_STR
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"deactivate onu traffic service\n"
	"please input the onu\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
    	/*INT16 phyPonId = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	int result = 0;*/

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
	ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
		{
		vty_out( vty, "  %% onuidx error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

	if(ThisIsValidOnu( phyPonId, (ulOnuid-1))  != ROK )
		{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
		}

	/*if( V2R1_CTC_STACK )*/
	if(GetOnuVendorType( phyPonId, (ulOnuid - 1 ) ) == ONU_VENDOR_CT )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(&auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			vty_out(vty, " ONU authentication should be enable first\r\n");
			return( CMD_WARNING );
			}		
		}

	result = SetOnuTrafficServiceEnable( phyPonId, (ulOnuid - 1 ), V2R1_ENABLE );

	if( result ==  V2R1_ONU_NOT_EXIST )
		{
		vty_out( vty, "  %% onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	/*
	else if(  result == ONU_OPER_STATUS_DOWN  ) 
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING );
		}
	*/
	else if( result == RERROR )
		{
		vty_out( vty, "  %% Executing error \r\n");
		return( CMD_WARNING );
		}

	return CMD_SUCCESS;
#else
	if( PON_GetSlotPortOnu((ULONG)(vty->index), &ulSlot, &ulPort , &ulOnuid) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    CHECK_CMD_ONU_RANGE(vty, ulOnuid-1);
	
	return cli_set_onu_traffic_service_enable( vty, ulSlot, ulPort, ulOnuid, V2R1_ENABLE );
#endif
}

DEFUN (
	show_deactivate_onu_device1,
	show_deactivate_onu_device1_cmd,
	"show deactivate onu",
	"show deactivate onu\n"
	"show deactivate onu\n"
	"show deactivate onu\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
	/*ULONG ulIfIndex;
   	LONG lRet;*/
 	INT16 phyPonId = 0;
	unsigned char i, flag=0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	i = 0;
	for( ulOnuid = 0; ulOnuid< MAXONUPERPON; ulOnuid++)
		{
		if( GetOnuTrafficServiceEnable( phyPonId, ulOnuid ) == V2R1_DISABLE )
			{
			if( flag == 0 )
				{
				flag =1 ;
				vty_out(vty, "  pon %d/%d deactivate onu list\r\n", ulSlot, ulPort );
				}
			vty_out(vty, "  %2d ", (ulOnuid+1) );
			i++;
			if(( i % 16 ) == 0 ) vty_out(vty,"\r\n");
			}
		}
	
	if( flag == 0 ) vty_out(vty, "  pon %d/%d has no deactivate onu\r\n", ulSlot, ulPort );	
	else if(( i % 8 ) !=  0 ) vty_out(vty,"\r\n");
	return( CMD_SUCCESS );
	
}


#ifdef  ONU_AUTHENTICATION_CONTROL
#endif

/* 	 added by chenfj 2007-7-6
*	     for onu authenticaion by PAS5001 & PAS5201
*         注: 在PAS5001上，onu authentication是由软件来实现的；在PAS5201上，若启动CTC协议栈，onu authentication是由PON芯片硬件实现的；若没有启动CTC协议栈，onu authentication 也是由 软件来实现
*/

DEFUN (
	show_onuauthentication_entry,
	show_onuauthentication_entry_cmd,
	"show onu-register authentication entry",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication mac table\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	ShowPonPortAuthOnuEntry( PonPortIdx,  vty );

	return( CMD_SUCCESS );
}


DEFUN (
	add_onuauthentication_entry,
	add_onuauthentication_entry_cmd,
	"add onu-register authentication entry <H.H.H>",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mac table\n"
	"please input the onu mac address\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	unsigned long  TableIdx;
	short int  PonPortIdx;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
	STATUS ret;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsInvalidMacAddr(MacAddr) == TRUE ) 
	{
		vty_out(vty,"  %%  mac address %s is invalid\r\n", (unsigned char *)argv[0] );
		return(CMD_WARNING );
	}

	ret = isHaveAuthMacAddress(ul_slot, ul_port, MacAddr, &TableIdx );
	if( ret == VOS_OK )
	{
		vty_out(vty, "  %% This MAC addr is already exist\r\n");
		return( CMD_SUCCESS );
	}
	if( (TableIdx < 1 ) ||( TableIdx > MAXONUPERPON ))
	{
		vty_out( vty, "  %% Execting err\r\n");
		return( CMD_WARNING );
	}

	ret = setOnuAuthStatus( ul_slot, ul_port, TableIdx, V2R1_ENTRY_CREATE_AND_GO);
	if( ret != VOS_OK )	
	{
		vty_out( vty, "  %% Execting err\r\n");
		return( CMD_WARNING );
	}
#if 0
	vty_out(vty, " pon %d/%d TableIdx = %d\r\n", ul_slot, ul_port, TableIdx );
	vty_out(vty, " mac addr: %02x%02x.%02x%02x.%02x%02x\r\n",MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5] );
#endif	
	ret = setOnuAuthMacAddress( ul_slot, ul_port, TableIdx, MacAddr );
	if( ret == VOS_OK )	
		return( CMD_SUCCESS ); 	
	else {
		vty_out( vty, "  %% Execting err\r\n");
		return( CMD_WARNING );
	}

	return( CMD_SUCCESS );

}


DEFUN (
	delete_onuauthentication_entry,
	delete_onuauthentication_entry_cmd,
	"delete onu-register authentication entry {<H.H.H>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mac table\n"
	"please input the onu mac address\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid;
	unsigned long  TableIdx;
	short int  PonPortIdx;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
	STATUS ret;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	if( argc == 1 )
	{
		if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
		{
			vty_out( vty, "  %% Invalid MAC address.\r\n" );
			return CMD_WARNING;
		}

		ret = isHaveAuthMacAddress(ul_slot, ul_port, MacAddr, &TableIdx );

		if( ret != VOS_OK )
		{
			vty_out(vty, "  %% this mac addr is not in authentication onu table\r\n");
			return( CMD_SUCCESS );
		}
		if( (TableIdx < 0 ) ||( TableIdx > MAXONUPERPON ))
		{
			vty_out( vty, "  %% Execting err\r\n");
			return( CMD_WARNING );
		}

		ret = setOnuAuthStatus( ul_slot, ul_port, TableIdx, V2R1_ENTRY_DESTORY );
		if( ret == VOS_OK )	
			return( CMD_SUCCESS ); 	
		else {
			vty_out( vty, "  %% Execting err\r\n");
			return( CMD_WARNING );
		}
	}

	else if( argc == 0 )
	{
		DeletePonPortAuthEntryAll( PonPortIdx );
		return( CMD_SUCCESS );
	}

	return( CMD_SUCCESS );

}
#if 1
DEFUN (
	show_gonuauthentication_entry,
	show_gonuauthentication_entry_cmd,
	"show gonu-register authentication entry",
	SHOW_STR
	"show gonu-register authentication information\n"
	"show gonu-register authentication information\n"
	"show gonu-register authentication serial number table\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid = 0;
	short int  PonPortIdx;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	ShowPonPortAuthGponOnuEntry(vty, ul_slot, ul_port);

	return( CMD_SUCCESS );
}


DEFUN (
	add_gonuauthentication_entry,
	add_gonuauthentication_entry_cmd,
	"add gonu-register authentication entry <sn> {<pwd>}*1", /*将密码改为可选参数，mod by liuyh, 2017-5-11*/
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication entry\n"
	"please input the gonu serial number\n"
	"please input the gonu passward\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	unsigned long  TableIdx = 0;
	short int  PonPortIdx = 0;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;		
	STATUS ret = 0;
    UCHAR SN[GPON_ONU_SERIAL_NUM_STR_LEN];
	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		vty_out(vty,"  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
    
	if( VOS_StrLen(argv[0]) != 16 ) 
	{
		vty_out(vty,"  %%  serial number %s is invalid\r\n", (unsigned char *)argv[0] );
		return(CMD_WARNING );
	}
	else
	{
	    VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
		VOS_MemZero(SN,GPON_ONU_SERIAL_NUM_STR_LEN);
	    VOS_MemCpy(entry.authEntry.serial_no, argv[0], GPON_ONU_SERIAL_NUM_STR_LEN-1);	    	    
		VOS_MemCpy(SN, argv[0], GPON_ONU_SERIAL_NUM_STR_LEN-1);
	}
	
    if(argc == 2)
    {
        if(VOS_StrLen(argv[1]) >= GPON_ONU_PASSWARD_STR_LEN)
        {
            vty_out(vty,"  %%  passward %s is invalid\r\n", (unsigned char *)argv[1] );
            return(CMD_WARNING );
        }
        else
        {
            VOS_MemCpy(entry.authEntry.password, argv[1], GPON_ONU_PASSWARD_STR_LEN-1);              
        }
    }
    
    pAuthData = gonu_auth_entry_list_seach(ul_slot, ul_port, entry.authEntry.serial_no);
    if(pAuthData != NULL)
    {
		vty_out(vty, "  %% This serial number is already exist\r\n");
		return( CMD_SUCCESS );
    }
    
    TableIdx = gonu_auth_entry_free_idx_get(ul_slot, ul_port);
	if( (TableIdx < 1 ) ||( TableIdx > MAXONUPERPON ))
	{
		vty_out( vty, "  %% Entry full\r\n");
		return( CMD_WARNING );
	}

    entry.authIdx = TableIdx;
    entry.authRowStatus = RS_CREATEANDGO;
    ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
    if(ret != VOS_OK)
    {
        vty_out( vty, "  %% Execting err\r\n");
        return( CMD_WARNING );
    }
	/*b-pon节点下add认证表，默认active,by zhaoxh 问题单38678*/
    OnuAuth_ActivatePendingOnuBySnMsg(PonPortIdx,SN);
	ActivateGponPendingOnuMsg_conf(PonPortIdx,SN);
	/*e-pon节点下add认证表，默认active*/
    return( CMD_SUCCESS);
}


DEFUN (
	delete_gonuauthentication_entry,
	delete_gonuauthentication_entry_cmd,
	"delete gonu-register authentication entry {<sn>}",
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication mac table\n"
	"please input the gonu serial number\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	unsigned long  TableIdx = 0;
	short int  PonPortIdx = 0;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;		
	STATUS ret = 0;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		vty_out(vty,"  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
    if(argc == 0)
	{
		ULONG brdIdx,portIdx,Index=0,NextBrdIdx,NextPortIdx,NextIndex=0;
		brdIdx = ul_slot;
		portIdx = ul_port;
		do{
			
			Index = NextIndex;

			ret =  mn_getNextGponOnuAuthEntryIdx(brdIdx,portIdx,Index, &NextBrdIdx, &NextPortIdx, &NextIndex);
			entry.authIdx = Index;
			entry.authRowStatus = RS_DESTROY;
			if(Index!=0)
			ret=OLT_SetGponAuthEntry(PonPortIdx,  entry );
		}while(( ret == VOS_OK ) && ( brdIdx == NextBrdIdx ) && ( portIdx == NextPortIdx ) );
	}
    else if(argc == 1)
    {
	    VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
	    VOS_MemCpy(entry.authEntry.serial_no, argv[0], GPON_ONU_SERIAL_NUM_STR_LEN-1);	    	    
    
    
	    pAuthData = gonu_auth_entry_list_seach(ul_slot, ul_port, entry.authEntry.serial_no);
	    if(pAuthData == NULL)
	    {
			vty_out(vty, "  %% This serial number is not exist\r\n");
			return( CMD_WARNING );
	    }

	    entry.authRowStatus = RS_DESTROY;
	    ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
	    if(ret != VOS_OK)
	    {
	        vty_out( vty, "  %% Execting err\r\n");
	        return( CMD_WARNING );
	    }
	}
    return( CMD_SUCCESS);
}
#endif


#ifdef  PON_BER_FER_ALARM 
#endif
	/* added by chenfj 2007-7-31 
	    PON端口BER / FER 告警 */
DEFUN (
	onupon_ber_fer_alarm_enable,
	onupon_ber_fer_alarm_enable_cmd,
	"onu-pon [ber|fer] alarm [enable|disable]",
	"onu pon port\n"
	"set ber alarm\n"
	"set fer alarm\n"
	"onu pon port ber/fer alarm\n"
	"ber/fer alarm enable\n"
	"ber/fer alarm disable\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	ULONG Ber_Fer_flag, enable;
	short int ret;

	if( VOS_StrCmp( argv[0], "ber" ) == 0 )
	{
		Ber_Fer_flag = 1;
	}
	else
	{
		Ber_Fer_flag = 2;
	}
	if( VOS_StrCmp( argv[1], "enable" ) == 0 )
		enable = V2R1_ENABLE;
	else
		enable = V2R1_DISABLE;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

    
	if( Ber_Fer_flag == 1 )
		ret = monOnuBerAlmEnSet( phyPonId, enable );
	else ret = monOnuFerAlmEnSet( phyPonId, enable );

	if( ret != ROK ) 
	{
		vty_out(vty,"  %% Executing error \r\n");
		return( CMD_WARNING );
	}					

	return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20100928, 为了和showrun保持一致，去掉undo命令 */
DEFUN (
	onupon_ber_fer_alarm_disable,
	onupon_ber_fer_alarm_disable_cmd,
	"undo onu-pon [ber|fer] alarm enable",
	"onu pon port ber/fer alarm disable\n"
	"onu pon port ber/fer alarm disable\n"
	"ber alarm disable\n"
	"fer alarm disable\n"
	"onu pon port ber/fer alarm disable\n"
	"onu pon port ber/fer alarm disable\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	unsigned long Ber_Fer_flag = 0;
	short int ret;

	if( VOS_StrCmp( argv[0], "ber" ) == 0 )
		{
		Ber_Fer_flag = 1;
		}
	else /*if( VOS_StrCmp( argv[0], "fer" ) == 0 )*/
		{
		Ber_Fer_flag = 2;
		}
	/*else{
		vty_out(vty, "  %% parameter 1 err\r\n");
		return( CMD_WARNING );
		}*/

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/*
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING );
			}
		}
	*/
	if( Ber_Fer_flag == 1 )
		ret = monOnuBerAlmEnSet( phyPonId, V2R1_DISABLE );
	else ret = monOnuFerAlmEnSet( phyPonId, V2R1_DISABLE );

	if( ret != ROK ) 
		{
		vty_out(vty,"  %% Executing error \r\n");
		return( CMD_WARNING );
		}					

	return (CMD_SUCCESS);
}
#endif

DEFUN (
	show_onupon_ber_fer_alarm,
	show_onupon_ber_fer_alarm_cmd,
	"show onu-pon ber-fer alarm enable",
	"show info.\n"
	"show onu pon port info.\n"
	"show onu pon port ber/fer alarm config\n"
	"show onu pon port ber/fer alarm config\n"
	"show onu pon port ber/fer alarm config\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid=0;
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	unsigned int Ber_flag = V2R1_DISABLE;
	unsigned int Fer_flag = V2R1_DISABLE;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING);
			}
		}

	monOnuBerAlmEnGet( phyPonId, &Ber_flag );
	monOnuFerAlmEnGet( phyPonId, &Fer_flag );

	 if( (Ber_flag != V2R1_ENABLE) && (Ber_flag != V2R1_DISABLE) )		/* modified by xieshl 20091217 */
	 	Ber_flag = 0;
	 if( (Fer_flag != V2R1_ENABLE) && (Fer_flag != V2R1_DISABLE) )
	 	Fer_flag = 0;
	vty_out(vty, "  onu pon-port %d/%d  ber %s, fer %s\r\n", ulSlot, ulPort, v2r1Enable[Ber_flag], v2r1Enable[Fer_flag]);					


	return CMD_SUCCESS;
}


DEFUN (
	oltpon_ber_fer_alarm_enable,
	oltpon_ber_fer_alarm_enable_cmd,
	"olt-pon [ber|fer] alarm [enable|disable]",
	"olt pon port\n"
	"ber alarm enable\n"
	"fer alarm enable\n"
	"olt pon port ber/fer alarm\n"
	"ber/fer alarm enable\n"
	"ber/fer alarm disable\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	ULONG Ber_Fer_flag, enable;
	short int ret;

	if( VOS_StrCmp( argv[0], "ber" ) == 0 )
		{
		Ber_Fer_flag = 1;
		}
	else /*if( VOS_StrCmp( argv[0], "fer" ) == 0 )*/
		{
		Ber_Fer_flag = 2;
		}
	/*else{
		vty_out(vty, "  %% parameter 1 err\r\n");
		return( CMD_WARNING );
		}*/

	if( VOS_StrCmp( argv[1], "enable" ) == 0 )
		enable = V2R1_ENABLE;
	else
		enable = V2R1_DISABLE;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;


	if( Ber_Fer_flag == 1 )
		ret = monPonBerAlmEnSet( phyPonId, enable );
	else ret = monPonFerAlmEnSet( phyPonId, enable );

	if( ret != ROK ) 
	{
		vty_out(vty,"  %% Executing error \r\n");
		return( CMD_WARNING );
	}					

	return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20100928, 为了和showrun保持一致，去掉undo命令 */
DEFUN (
	oltpon_ber_fer_alarm_disable,
	oltpon_ber_fer_alarm_disable_cmd,
	"undo olt-pon [ber|fer] alarm enable",
	"olt pon port ber/fer alarm disable\n"
	"olt pon port ber/fer alarm disable\n"
	"ber alarm disable\n"
	"fer alarm disable\n"
	"olt pon port ber/fer alarm disable\n"
	"olt pon port ber/fer alarm disable\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	unsigned long Ber_Fer_flag = 0;
	short int ret;

	if( VOS_StrCmp( argv[0], "ber" ) == 0 )
		{
		Ber_Fer_flag = 1;
		}
	else /*if( VOS_StrCmp( argv[0], "fer" ) == 0 )*/
		{
		Ber_Fer_flag = 2;
		}
	/*else{
		vty_out(vty, "  %% parameter 1 err\r\n");
		return( CMD_WARNING );
		}*/

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	/*
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING );
			}
		}
	*/
	if( Ber_Fer_flag == 1 )
		ret = monPonBerAlmEnSet( phyPonId, V2R1_DISABLE );
	else ret = monPonFerAlmEnSet( phyPonId, V2R1_DISABLE );

	if( ret != ROK ) 
		{
		vty_out(vty,"  %% Executing error \r\n");
		return( CMD_WARNING );
		}					

	return (CMD_SUCCESS);
}
#endif

DEFUN (
	show_oltpon_ber_fer_alarm,
	show_oltpon_ber_fer_alarm_cmd,
	"show olt-pon ber-fer alarm enable",
	"show olt pon port ber/fer alarm enable\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid=0;
    	INT16 phyPonId = 0;
	/*ULONG ulIfIndex=0;
	LONG lRet = 0;*/
	unsigned int Ber_flag = V2R1_DISABLE;
	unsigned int Fer_flag = V2R1_DISABLE;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;
	/*
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING);
			}
		}
	*/
	monPonBerAlmEnGet( phyPonId, &Ber_flag );
	monPonFerAlmEnGet( phyPonId, &Fer_flag );

	 if( (Ber_flag != V2R1_ENABLE) && (Ber_flag != V2R1_DISABLE) )		/* modified by xieshl 20091217 */
	 	Ber_flag = 0;
	 if( (Fer_flag != V2R1_ENABLE) && (Fer_flag != V2R1_DISABLE) )
	 	Fer_flag = 0;
	vty_out(vty, "  olt pon-port %d/%d  ber %s, fer %s\r\n", ulSlot, ulPort, v2r1Enable[Ber_flag], v2r1Enable[Fer_flag]);					


	return CMD_SUCCESS;
}


#ifdef  RESET_PON_CHIP
#endif
extern int do_olt_reset(struct vty *vty);
extern int do_olt_reset_chip(struct vty *vty);

DEFUN (
	reset_ponchip,
	reset_ponchip_cmd,
	"reset pon",
	"reset pon\n"
	"reset pon. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid= 0;
	int PonChipPonNum;    
	int PonChipType;    
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot,ulPort,vty) != ROK)
		return(CMD_WARNING);
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif


    PonChipType = V2R1_GetPonchipType(phyPonId);
	/*gpon暂时屏蔽此功能 by jinhl*/
	if( PonChipType == PONCHIP_GPON)
	{
		vty_out(vty, "  %% this is gpon card, resetting pon port is not supported\r\n");
		return CMD_WARNING;
	}
    if ( 1 >= (PonChipPonNum = GetPonChipPonPorts(phyPonId, phyPonIds)) )
    {
    	if( PonChipType == PONCHIP_PAS5001 )
    	{
    		vty_out(vty, "  %% this is %s card, reset single pon chip isnot supported\r\n", pon_chip_type2name(PonChipType));
    		return( CMD_WARNING );	
    	}
    }
    else
    {
        int i, n, l;
        char szTitleStr[256];
    
		/*--------modified by wangjiah@2016-09-14 8xep reset pon port software: begin------------*/
        /* 板上只有一个PON芯片，则不支持端口的复位 */
        if ( 1 == GetSlotCardPonChipNum(0, ulSlot) )
        {
			if(PonChipType != PONCHIP_BCM55538)
			{
				vty_out(vty, "  %% this is %s %d-pon-card, reset single pon chip isnot supported\r\n", pon_chip_type2name(PonChipType), PonChipPonNum);
				return( CMD_WARNING );	
			}
        }
   
		if( PonChipType == PONCHIP_BCM55538	)
		{
			vty_out(vty, "  %% are you sure to reset the pon port(%d/%d)? [Y/N]", ulSlot, ulPort);
		}
		else
		{
			n = 0;
		   	ulPort -= phyPonId - phyPonIds[0]; /* 同一PON芯片上的端口号连续 */
			for ( i = 0; i < PonChipPonNum; i++ )
		   	{
			  	l = VOS_Sprintf(&szTitleStr[n], "%d/%d, ", ulSlot, ulPort++);
			   	n += l;    
		   	}
		   	szTitleStr[n-2] = '\0';
			/* PON芯片第一个端口的复位，才会硬件复位芯片 */
			phyPonId = phyPonIds[0];
			vty_out(vty, "  %% this is %s card, reset single pon port is not supported\r\n", pon_chip_type2name(PonChipType));
			vty_out(vty, "  %% are you sure to reset the pon chip's all of pon ports(%s)? [Y/N]", szTitleStr);
		}
		/*--------modified by wangjiah@2016-09-14 8xep reset pon port software: end------------*/
		vty->prev_node = vty->node;
		vty->node = CONFIRM_ACTION_NODE;
		vty->action_func = do_olt_reset;
        VTY_CONFIRM_CB_SETPARAM1(vty, phyPonId);
        
		return CMD_SUCCESS;    
    }


#if 0
	if(Olt_exists(phyPonId) == TRUE )
		Remove_olt( phyPonId, FALSE, FALSE);	
	Hardware_Reset_olt1(ulSlot, ulPort, 1, 0 );

	vty_out(vty,"  reset %s/port%d complete\r\n", CardSlot_s[ulSlot],ulPort);
	Hardware_Reset_olt2(ulSlot, ulPort, 1, 0 );

	ClearPonRunningData( phyPonId );
	SetPonChipResetFlag(ulSlot, ulPort);
	Add_PonPort( phyPonId );		
#else
    if ( OLT_CALL_ISOK( OLT_ResetPon(phyPonId) ) )
    {
    	vty_out(vty,"  reset pon%d/%d complete\r\n", ulSlot,ulPort);
    }
    else
    {
		vty_out(vty, "  %% pon%d/%d is resetting, please wait a little.\r\n", ulSlot, ulPort);
    }
#endif


	return CMD_SUCCESS;
}

DEFUN (
	pon_port_reset_10gepon_chip,
	pon_port_reset_10gepon_chip_cmd,
	"reset 10gepon-chip", 
	"reset 10gepon-chip\n"
	"reset 10gepon-chip. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 1;
	ULONG ulOnuid = 0;
	int PonChipPonNum;    
	int PonChipType;    
	/*LONG lRet = 0;*/
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)
	{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
	}

    PonChipType = V2R1_GetPonchipType(phyPonId);
	PonChipPonNum = GetPonChipPonPorts(phyPonId, phyPonIds);

	if( PonChipType != PONCHIP_BCM55538)
	{
		vty_out(vty, "  %% this is not 10g epon card, resetting pon chip is not supported\r\n");
		return CMD_WARNING;
	}
        
	int i, n, l;
	char szTitleStr[256];
	n = 0;
	ulPort -= phyPonId - phyPonIds[0]; /* 同一PON芯片上的端口号连续 */
	for ( i = 0; i < PonChipPonNum; i++ )
	{
		l = VOS_Sprintf(&szTitleStr[n], "%d/%d, ", ulSlot, ulPort++);
		n += l;    
	}
	szTitleStr[n-2] = '\0';

	vty_out(vty, "  %% this is %s card, reset single pon chip will reset all pon ports!\r\n", pon_chip_type2name(PonChipType));
	vty_out(vty, "  %% are you sure to reset the pon chip's all of pon ports(%s)? [Y/N]", szTitleStr);

	vty->prev_node = vty->node;
	vty->node = CONFIRM_ACTION_NODE;
	vty->action_func = do_olt_reset_chip;
	VTY_CONFIRM_CB_SETPARAM1(vty, phyPonId);

	return CMD_SUCCESS;    
   
}
/* added by wangjiah@2016-08-12 to support pon port and pon chip reset under pon node : end*/


/* added by chenfj 2007-8-31 
	PAS-SOFT软件包中定义的CLI命令*/
#ifdef PAS_CLI_DEFINED
#endif

#define  REMOVED_WHEN_FULL   1
#define  KEEPED_WHEN_FULL   0

DEFUN(set_pon_address_table_configuration,
	set_pon_address_table_configuration_cmd,
	"pon address-table-config discard-unlearned-sa [true|false] mac-table-full [remove-entry|keep-entry] {discard-unknown-da [uplink|downlink|all-direction|no-direction]}*1",
	"config running system information\n"
	"pon address table config\n"
	"specifies whether discard unlearned SAframes(uplink direction) when originating ONU's address entries limit is reached\n"
	"discard unlearned source mac\n"
	"forward unlearned source mac\n"
	"the handling mode when add a new mac address and the mac address full\n"
	"remove entry\n"
	"keep entry,entry is removed only by aged\n"
	"Specifies whether unknown DAframes are discarded from uplink and/or downlink, or disabled\n"
	"uplink all unknown da frames(including broadcast and multicast) are discarded\n"
	"downlink unknown da frames(only unknown unicast) are discarded\n"
	"unknown da frames are discarded from both uplink and downlink\n"
	"unknown da frames are not discarded from either uplink or downlink\n"
	)
{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	OLT_addr_table_cfg_t stCfg;
  	int  iRlt;
	/* modified by xieshl 20101028, 问题单10956、10844 */
	UCHAR full_mode;
	UCHAR unlearned_mode;
	UCHAR discard_unlearned_da;
	UCHAR unlearned_sa_mode;
    
	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	full_mode = PonPortTable[PonPortIdx].table_full_handle_mode;
	unlearned_mode = PonPortTable[PonPortIdx].discard_unlearned_sa;
	discard_unlearned_da = PonPortTable[PonPortIdx].discard_unlearned_da;

    if ( 'r' == argv[1][0] )
    {
        full_mode = REMOVED_WHEN_FULL;
    }
    else
    {
        full_mode = KEEPED_WHEN_FULL;
    }

    if ( 't' == argv[0][0] )
    {
        unlearned_sa_mode = REMOVED_WHEN_FULL;
    }
    else
    {
        unlearned_sa_mode = KEEPED_WHEN_FULL;
    }

	if ( 3 == argc )
	{
	    switch (argv[2][0])
        {
            case 'u':
    			discard_unlearned_da = PON_DIRECTION_UPLINK;
                break;
            case 'd':
    			discard_unlearned_da = PON_DIRECTION_DOWNLINK;
                break;
            case 'a':
    			discard_unlearned_da = PON_DIRECTION_UPLINK_AND_DOWNLINK;
                break;
            case 'n':
    			discard_unlearned_da = PON_DIRECTION_NO_DIRECTION;
                break;
            default:
                VOS_ASSERT(0);
            	return CMD_WARNING;
        }   
	}

    stCfg.removed_when_full = full_mode;
    stCfg.discard_llid_unlearned_sa = unlearned_sa_mode;
    stCfg.discard_unknown_da = discard_unlearned_da;
    stCfg.aging_timer = -1;
	/* modified by xieshl 20101028, 不管什么情况上行学习使能都打开，暂不修改命令行 */
    stCfg.allow_learning = V2R1_ENABLE;
    iRlt = OLT_SetAddressTableConfig(PonPortIdx, &stCfg);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        switch (iRlt)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
            	return ( CMD_WARNING );
            default:
        		vty_out(vty,"set err %d\r\n", iRlt);
            	return ( CMD_WARNING );
        }
    }

    return ( CMD_SUCCESS );
}
/*add by yangzl@2016-6-3*/
DEFUN(set_pon_address_table_configuration_enable,
	set_pon_address_table_configuration_enable_cmd,
	"pon address-table-config [enable|disable]",
	"config running system information\n"
	"pon address table config\n"
	"enable pon address table config\n"
	"disable pon address table config\n"
	)
{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	OLT_addr_table_cfg_t stCfg;
  	int  iRlt;
    
	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;
	
	iRlt = OLT_GetAddressTableConfig( PonPortIdx, &stCfg );
	if( iRlt != PAS_EXIT_OK ) 
		{
			vty_out(vty,"  %% Execute error\r\n");
			return( CMD_WARNING );
		}

	if ( 0 == VOS_StrCmp( argv[ 0 ], "enable" ) )
	{
		stCfg.allow_learning = V2R1_ENABLE;
	}
	else if ( 0 == VOS_StrCmp( argv[ 0 ], "disable" ) )
	{
		stCfg.allow_learning = V2R1_DISABLE;
	}
		
    iRlt = OLT_SetAddressTableConfig(PonPortIdx, &stCfg);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        switch (iRlt)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
            	return ( CMD_WARNING );
            default:
        		vty_out(vty,"set err %d\r\n", iRlt);
            	return ( CMD_WARNING );
        }
    }

    return ( CMD_SUCCESS );
}


DEFUN(show_oam_information,
	show_oam_information_cmd,
	"show oam information <slot/port/onuid>",
	"Show running system information\n"
	"OAM protocol configuration\n"
	"Get OAM information of an ONU device.\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)

{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	short int  PonChipType;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	PON_onu_id_t             onu_id;
	short int                return_result;
	PON_oam_information_t    oam_param;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ul_slot, ul_port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short  int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	
	onu_id = GetLlidByOnuIdx( PonPortIdx, (ulOnuid-1));
	if (INVALID_LLID == onu_id ) 
	{
		vty_out( vty, "  %% %d/%d/%d is not exist or not onu working.\r\n",ul_slot, ul_port, ulOnuid) ;
		return CMD_WARNING;
	}

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(OLT_PONCHIP_ISPAS(PonChipType) ) 
		{
		/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		return_result = OLT_GetOamInformation (PonPortIdx, onu_id, &oam_param );

		if(return_result == PAS_EXIT_OK )
		{
			vty_out(vty,"OAM and UNI port information for ONU %d of %s/port%d:%s",
				    ulOnuid, CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), VTY_NEWLINE);
			CLI_Print_oam_information (vty, &oam_param);
		    	g_free(oam_param.oam_standard_information);
		}else
			CLI_ERROR_PRINT
		}
	else{
		vty_out(vty,"pon chip type is not supported\r\n");
		}
	return CMD_SUCCESS;
}

#ifndef  SHOW_PON_PORT_VERSION
#endif
DEFUN (show_ponport_version,
	show_ponport_version_cmd,
	"show pon port version",
	"show running system information\n"
	"pon port version\n"
	"pon port version\n"
	"pon port version\n"
	/*"please input the slot/port\n"*/
	)
{
	PON_device_versions_t   device_versions_struct;
	short int    return_result;
	short int  PonPortIdx;
	/*short int  PonChipType;*/
	char	device_s[30];
	unsigned long  ul_slot = 0, ul_port = 0 , ulOnuid = 0;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

#if 0		
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ul_slot, ul_port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

#if 1
    return_result = OLT_GetVersion(PonPortIdx, &device_versions_struct);
    if ( OLT_CALL_ISOK(return_result) )
#else    
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if((PonChipType == PONCHIP_PAS5001 ) || (PonChipType == PONCHIP_PAS5201 ) )
	{
		if(Olt_exists(PonPortIdx) != TRUE )
		{
			vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
			return(CMD_WARNING);
		}
		
		PonPortFirmwareAndDBAVersionMatchByVty(PonPortIdx, vty);	
			
		if ( (return_result = ( PAS_get_olt_versions ( PonPortIdx,
													  &device_versions_struct ))) == PAS_EXIT_OK)
#endif
		{
			vty_out(vty,"%s",VTY_NEWLINE);
			strcpy (device_s,"OLT");
		        /*WRITE_USHORT_PARAMETER   (PonPortIdx,OLT id)*/
			/*vty_out(vty,"pon id: %s/port%d\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );*/
		      WRITE_STRING_PARAMETER   (device_s, device)

			strcpy (device_s,"ponchip version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");

            if ( device_versions_struct.pon_vendors & PON_VENDOR_PMC )
            {
                switch((unsigned short)device_versions_struct.hardware_major) 
                {
                case 0xabcd:
                    vty_out(vty,"PAS5001-A");
                	break;
                case 0x5001:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x2):
                		vty_out(vty,"PAS5001-N");
                		break;
                	case(0x3):
                		vty_out(vty,"PAS5001-N M3");
                		break;
                	case(0x4):
                		vty_out(vty,"PAS5001-N M4");
                		break;
                	default:
                		vty_out(vty,"PAS5001 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0x5201:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x1):
                		vty_out(vty,"PAS5201");
                		break;
                	default:
                		vty_out(vty,"PAS5201 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0x5204:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x1):
                		vty_out(vty,"PAS5204");
                		break;
                	default:
                		vty_out(vty,"PAS5204 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                default:
                    vty_out(vty,"PMC Hardware major version not supported (0x%hx)",device_versions_struct.hardware_major);
                }
            }
            else if ( device_versions_struct.pon_vendors & PON_VENDOR_TEKNOVUS )
            {
                switch((unsigned short)device_versions_struct.hardware_major) 
                {
                case 0xa524:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x0):
                		vty_out(vty,"BCM55524-A0");
                		break;
                	case(0x1):
                		vty_out(vty,"BCM55524-B1");
                		break;
                	default:
                		vty_out(vty,"BCM55524 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0xa538:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x0):
                		vty_out(vty,"BCM55538-A0");
                		break;
                	case(0x1):
                		vty_out(vty,"BCM55538-B1");
                		break;
                	default:
                		vty_out(vty,"BCM55538 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                default:
                    vty_out(vty,"BCM Hardware major version not supported (0x%hx)",device_versions_struct.hardware_major);
                }
            }
            else
            {
                vty_out(vty,"PON Hardware ChipVendor(0x%x) not supported (0x%hx, 0x%hx)", device_versions_struct.pon_vendors, device_versions_struct.hardware_major, device_versions_struct.hardware_minor);
            }
			vty_out(vty,"%s",VTY_NEWLINE);
			
			/*
		        WRITE_STRING_PARAMETER   (PON_mac_s[device_versions_struct.system_mac],System port MAC)
			if( device_versions_struct.ports_supported > MAXONUPERPON )
				device_versions_struct.ports_supported = MAXONUPERPON;
		        WRITE_USHORT_PARAMETER   (device_versions_struct.ports_supported,LLIDs (ports) supported)
			*/
			strcpy (device_s,"firmware version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"%s%s",PonChipMgmtTable[PonPortIdx].FirmwareVer, VTY_NEWLINE);

			strcpy (device_s,"pon-soft version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"%s%s",PonChipMgmtTable[PonPortIdx].HostSwVer, VTY_NEWLINE);
/*
			vty_out(vty,"V%d.%d.%d.%d%s",
						device_versions_struct.firmware_major,
						device_versions_struct.firmware_minor,
						device_versions_struct.build_firmware,
						device_versions_struct.maintenance_firmware,
						VTY_NEWLINE);
						
			vty_out(vty,"V%d.%d.%d%s",
						device_versions_struct.host_major,
						device_versions_struct.host_minor,
						device_versions_struct.host_compilation,
						VTY_NEWLINE);
*/			
			GetDBAInfo(PonPortIdx);
			vty_out(vty, "%s version : V%s\r\n", DBA_name, DBA_Version );

			vty_out(vty,"\r\n");

		}
        else
        {
            switch ( return_result )
            {
                case OLT_ERR_NOTEXIST:
            		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
                    break;
                case OLT_ERR_NOTSUPPORT:
            		vty_out(vty,"  %% pon%d/%d is not supported\r\n", ul_slot, ul_port);
                    break;
                default:
        			CLI_ERROR_PRINT
            }

            return CMD_WARNING;
        }
#if 0	
	}
	else
    {
        vty_out(vty,"pon chip type is not supported\r\n");
    }
#endif


	return CMD_SUCCESS;
}

DEFUN (config_show_ponport_version,
	config_show_ponport_version_cmd,
	"show pon port version <slot/port>",
	"show running system information\n"
	"pon port version\n"
	"pon port version\n"
	"pon port version\n"
	"please input the slot/port\n"
	)
{
	PON_device_versions_t   device_versions_struct;
	short int    return_result;
	char	device_s[30];
	unsigned long  ul_slot = 0, ul_port = 0;
	short int  PonPortIdx;
	/*short int  PonChipType;*/
	/*unsigned long ulIfIndex;*/
	LONG lRet;
	
	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;		
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
	}
    
#if 1
    return_result = OLT_GetVersion(PonPortIdx, &device_versions_struct);
    if ( OLT_CALL_ISOK(return_result) )
#else    
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if((PonChipType == PONCHIP_PAS5001 ) || (PonChipType == PONCHIP_PAS5201 ) )
	{
    	if( Olt_exists(PonPortIdx) != TRUE )
    	{
    		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
    		return(CMD_WARNING);
    	}
        
		PonPortFirmwareAndDBAVersionMatchByVty(PonPortIdx, vty);
		
		if ( (return_result = ( PAS_get_olt_versions ( PonPortIdx,
													  &device_versions_struct ))) == PAS_EXIT_OK)
#endif
		{
			vty_out(vty,"%s",VTY_NEWLINE);
			strcpy (device_s,"OLT");
		        /*WRITE_USHORT_PARAMETER   (PonPortIdx,OLT id)*/
			/*vty_out(vty,"pon id: %s/port%d\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );*/
		      WRITE_STRING_PARAMETER   (device_s, device)

			strcpy (device_s,"ponchip version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");

            if ( device_versions_struct.pon_vendors & PON_VENDOR_PMC )
            {
                switch((unsigned short)device_versions_struct.hardware_major) 
                {
                case 0xabcd:
                    vty_out(vty,"PAS5001-A");
                	break;
                case 0x5001:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x2):
                		vty_out(vty,"PAS5001-N");
                		break;
                	case(0x3):
                		vty_out(vty,"PAS5001-N M3");
                		break;
                	case(0x4):
                		vty_out(vty,"PAS5001-N M4");
                		break;
                	default:
                		vty_out(vty,"PAS5001 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0x5201:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x1):
                		vty_out(vty,"PAS5201");
                		break;
                	default:
                		vty_out(vty,"PAS5201 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0x5204:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x1):
                		vty_out(vty,"PAS5204");
                		break;
                	default:
                		vty_out(vty,"PAS5204 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                default:
                    vty_out(vty,"PMC Hardware major version not supported (0x%hx)",device_versions_struct.hardware_major);
                }
            }
            else if ( device_versions_struct.pon_vendors & PON_VENDOR_TEKNOVUS )
            {
                switch((unsigned short)device_versions_struct.hardware_major) 
                {
                case 0xa524:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x0):
                		vty_out(vty,"BCM55524-A0");
                		break;
                	case(0x1):
                		vty_out(vty,"BCM55524-B1");
                		break;
                	default:
                		vty_out(vty,"BCM55524 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                case 0xa538:
                	switch(device_versions_struct.hardware_minor)
                	{
                	case(0x0):
                		vty_out(vty,"BCM55538-A0");
                		break;
                	case(0x1):
                		vty_out(vty,"BCM55538-B1");
                		break;
                	default:
                		vty_out(vty,"BCM55538 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
                		break;
                	}
                	break;
                default:
                    vty_out(vty,"BCM Hardware major version not supported (0x%hx)",device_versions_struct.hardware_major);
                }
            }
            else
            {
                vty_out(vty,"PON Hardware ChipVendor(0x%x) not supported (0x%hx, 0x%hx)", device_versions_struct.pon_vendors, device_versions_struct.hardware_major, device_versions_struct.hardware_minor);
            }
			
			vty_out(vty,"%s",VTY_NEWLINE);
			
			/*
		        WRITE_STRING_PARAMETER   (PON_mac_s[device_versions_struct.system_mac],System port MAC)
			if( device_versions_struct.ports_supported > MAXONUPERPON )
				device_versions_struct.ports_supported = MAXONUPERPON;
		        WRITE_USHORT_PARAMETER   (device_versions_struct.ports_supported,LLIDs (ports) supported)
			*/
			strcpy (device_s,"firmware version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"%s%s",PonChipMgmtTable[PonPortIdx].FirmwareVer, VTY_NEWLINE);
			
			strcpy (device_s,"pon-soft version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"%s%s",PonChipMgmtTable[PonPortIdx].HostSwVer, VTY_NEWLINE);
/*
			strcpy (device_s,"firmware version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"V%d.%d.%d.%d%s",
						device_versions_struct.firmware_major,
						device_versions_struct.firmware_minor,
						device_versions_struct.build_firmware,
						device_versions_struct.maintenance_firmware,
						VTY_NEWLINE);
			
			strcpy (device_s,"pas-soft version");
			vty_out(vty,"%-17s", device_s);
			vty_out(vty,": ");
			vty_out(vty,"V%d.%d.%d%s",
						device_versions_struct.host_major,
						device_versions_struct.host_minor,
						device_versions_struct.host_compilation,
						VTY_NEWLINE);
*/			
			GetDBAInfo(PonPortIdx);
			vty_out(vty, "%s version : V%s\r\n", DBA_name, DBA_Version );

			vty_out(vty,"\r\n");	
		}
        else
        {
            switch ( return_result )
            {
                case OLT_ERR_NOTEXIST:
            		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
                    break;
                case OLT_ERR_NOTSUPPORT:
            		vty_out(vty,"  %% pon%d/%d is not supported\r\n", ul_slot, ul_port);
                    break;
                default:
        			CLI_ERROR_PRINT
            }

            return CMD_WARNING;
        }
#if 0	
	}
	else
    {
        vty_out(vty,"pon chip type is not supported\r\n");
    }
#endif

	return CMD_SUCCESS;
}

#if 0
DEFUN (show_onu_version,
	show_onu_version_cmd,
	"show onu version <slot/port/onuid>",
	"Show running system information\n"
	"ONU configuration\n"
	"Display onu device version\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)
{
	PAS_device_versions_t   device_versions_struct;
	short int               return_result;
	short int				onu_id;
	char					device_s[10];

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	short int  PonChipType;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	if(( argc !=  1 ) )
		{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
		}

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;

	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if ((ul_slot < 4)  || (ul_slot > 8))
		{
	   vty_out( vty, "  %% Error slot %d.\r\n", ul_slot );
		return CMD_WARNING;
		}

	if (ul_slot == 4)
		{
		if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ul_slot))
			{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", ul_slot, MODULE_E_EPON3_SW_NAME_STR );
			return CMD_WARNING;
			}
		}
	
	if ( (ul_port < 1) || (ul_port > 4) )
		{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ul_slot, ul_port);
		return CMD_WARNING;
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/

	onu_id = GetLlidByOnuIdx( PonPortIdx, (ulOnuid-1));
	if ((-1) == onu_id ) 
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ul_slot, ul_port, ulOnuid) ;
		return CMD_WARNING;
	}

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if((PonChipType == PONCHIP_PAS5001 ) || (PonChipType == PONCHIP_PAS5201 ) )
		{
		if ( (return_result = ( PAS_get_device_versions_v4 ( PonPortIdx,
														  onu_id,
														  &device_versions_struct ))) == PAS_EXIT_OK)
		{
		       /* WRITE_USHORT_PARAMETER   (olt_id,OLT id)*/
			vty_out(vty,"OLT id: %s/port%d\r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
			sprintf((device_s), "ONU %d", ulOnuid);
		        WRITE_STRING_PARAMETER   (device_s,Device)
				vty_out(vty,"Hardware version: ");
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
				
				vty_out(vty,"%s",VTY_NEWLINE);
				vty_out(vty,"Firmware version: %d.%d.%d.%d%s",
						device_versions_struct.firmware_major,
						device_versions_struct.firmware_minor,
						device_versions_struct.build_firmware,
						device_versions_struct.maintenance_firmware,
						VTY_NEWLINE);
				if ((device_versions_struct.host_major == 0) && (device_versions_struct.host_minor == 0) &&
					(device_versions_struct.host_compilation == 0) )
					vty_out(vty,"Host version    : Not available%s",VTY_NEWLINE);
				else
					vty_out(vty,"Host version    : %d.%d.%d%s",
							device_versions_struct.host_major,
							device_versions_struct.host_minor,
							device_versions_struct.host_compilation,
							VTY_NEWLINE);
		}else
			CLI_ERROR_PRINT
		}
	else{
		vty_out(vty,"pon chip type is not supported\r\n");
		}

	return CMD_SUCCESS;
}
#endif

DEFUN(show_olt_capabilities,
	show_olt_capabilities_cmd,
	"show pon capabilities <slot/port>",
	"Show running system information\n"
	"PON configuration\n"
	"Get the physical capabilities of an OLT\n"
	"please input the slot/port\n"
	)
{
#if 1
    OLT_optical_capability_t optical_capabilities;
	int  return_result;
#else
	PAS_physical_capabilities_t  device_capabilities;
	short int  return_result;
#endif
	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid*/;
	short int  PonPortIdx/*, PonChipType*/;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	if(PonPortIsWorking(PonPortIdx) != TRUE )
	{
		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
		return(CMD_WARNING);
	}

#if 1
    return_result = OLT_GetOpticalCapability(PonPortIdx, &optical_capabilities);
    if ( OLT_CALL_ISOK(return_result) )
    {
		vty_out (vty,"OLT %s/port%d:%s", CardSlot_s[ul_slot], ul_port, VTY_NEWLINE);
		vty_out (vty,"AGC lock time: %d TQs%s", optical_capabilities.agc_lock_time, VTY_NEWLINE);
		vty_out (vty,"CDR lock time: %d TQs%s", optical_capabilities.cdr_lock_time, VTY_NEWLINE);
		vty_out (vty,"PON TX signal: %s%s", optical_capabilities.pon_tx_signal == ENABLE ? "ENABLE" : "DISABLE",VTY_NEWLINE);
    }
    else
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
        		vty_out(vty, "pon chip type is not supported\r\n");
                break;
            case OLT_ERR_NOTEXIST:
        		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
                break;
            default:
    			CLI_ERROR_PRINT
        }
		return(CMD_WARNING);
    }
#else    
    PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    if((PonChipType == PONCHIP_PAS5001) ||(PonChipType == PONCHIP_PAS5201))
    {
		return_result = PAS_get_device_capabilities_v4( PonPortIdx,
										     PON_OLT_ID,
										     &device_capabilities );

		if ( return_result == PAS_EXIT_OK )
			{
			vty_out (vty,"OLT %s/port%d:%s", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), VTY_NEWLINE);
			vty_out (vty,"AGC lock time: %d TQs%s", device_capabilities.agc_lock_time, VTY_NEWLINE);
			vty_out (vty,"CDR lock time: %d TQs%s", device_capabilities.cdr_lock_time, VTY_NEWLINE);
			vty_out (vty,"PON TX signal: %s%s", device_capabilities.pon_tx_signal == ENABLE ? "ENABLE" : "DISABLE",VTY_NEWLINE);
			}
		else
			CLI_ERROR_PRINT
	}
	else
    {
		vty_out(vty,"pon chip type is not supported\r\n");
	}
#endif

    return CMD_SUCCESS;
}

DEFUN(show_llic_capabilities,
	show_llid_capabilities_cmd,
	"show onu capabilities <slot/port/onuid>",
	"Show running system information\n"
	"PON configuration\n"
	"Get the physical capabilities of a onu\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)
{
#if 1    
    ONU_optical_capability_t     optical_capability;
	int  return_result;
#else
	PAS_physical_capabilities_t  device_capabilities;
	short int                    return_result = EXIT_OK;
	short int  PonChipType;
	PON_onu_id_t                 onu_id;
#endif
	short int PonPortIdx;
	short int OnuIdx;
	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	/*unsigned long ulIfIndex;*/
	LONG lRet;


	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	if(PonPortIsWorking(PonPortIdx) != TRUE )
		{
		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
		return(CMD_WARNING);
		}
	
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}

#if 1
    return_result = OnuMgt_GetOpticalCapability(PonPortIdx, OnuIdx, &optical_capability);
    if ( OLT_CALL_ISOK(return_result) )
    {
		vty_out (vty,"onu %d/%d/%d:%s", ul_slot, ul_port, ulOnuid,  VTY_NEWLINE);
		vty_out (vty,"Laser on time:     %d TQs%s", optical_capability.laser_on_time, VTY_NEWLINE);
		vty_out (vty,"Laser off time:    %d TQs%s", optical_capability.laser_off_time, VTY_NEWLINE);
		vty_out (vty,"Grants fifo depth: %d grants%s", optical_capability.onu_grant_fifo_depth, VTY_NEWLINE);
    }
    else
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
        		vty_out(vty, "pon chip type is not supported\r\n");
                break;
            case OLT_ERR_NOTEXIST:
        		vty_out(vty,"  %% onu %d/%d/%d is not online\r\n", ul_slot, ul_port, ulOnuid);
                break;
            default:
    			CLI_ERROR_PRINT
        }
		return(CMD_WARNING);
    }
#else
	onu_id = GetLlidByOnuIdx( PonPortIdx, (ulOnuid - 1));
	if( onu_id == INVALID_LLID)
		{
		vty_out(vty, "  %% Executing error\r\n");
		return( CMD_WARNING );
		}

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
		{
		return_result = PAS_get_device_capabilities_v4( PonPortIdx ,
												     onu_id,
												     &device_capabilities );

		if ( return_result == PAS_EXIT_OK )
			{
			vty_out (vty,"onu %d/%d/%d:%s", ul_slot, ul_port, ulOnuid,  VTY_NEWLINE);
			vty_out (vty,"Laser on time:     %d TQs%s", device_capabilities.laser_on_time, VTY_NEWLINE);
			vty_out (vty,"Laser off time:    %d TQs%s", device_capabilities.laser_off_time, VTY_NEWLINE);
			vty_out (vty,"Grants fifo depth: %d grants%s", device_capabilities.onu_grant_fifo_depth, VTY_NEWLINE);

			}
		else
			CLI_ERROR_PRINT
		}
	else {
		vty_out(vty,"pon chip type is not supported\r\n");
		}
#endif

	return CMD_SUCCESS;
}

#if 0
DEFUN(show_statistics,
	show_onu_statistics_cmd,
	"show onu statistics <1-64> statistics-type [olt-ber|olt-fer]",
	"Show running system information\n"
	"PON configuration\n"
	"The command gets one statistics data\n"
	OnuIDString
	"The type of the statistics\n"  
	"Average OLT bit error rate (downstream) as measured by the ONU, computed by software\n"
	"Average OLT frames error rate (downstream) as measured by the ONU, computed by software\n")
{
	short int		   collector = 0;
	short int		   statistic_param = 0;
	PON_onu_id_t       onu_id;
	PON_statistics_t   statistics_type;
	short int          return_result;
	char               stati_type[MAX_PARAM_LENGTH];
	long double        statistics_data;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	unsigned int  PonPortIdx;
	unsigned long ulIfIndex;
	LONG lRet;

	if(( argc !=  2) )
		{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
		}
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if ((ul_slot < 4)  || (ul_slot > 8))
		{
	   vty_out( vty, "  %% Error slot %d.\r\n", ul_slot );
		return CMD_WARNING;
		}

	if (ul_slot == 4)
		{
		if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ul_slot))
			{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", ul_slot, MODULE_E_EPON3_SW_NAME_STR );
			return CMD_WARNING;
			}
		}
	
	if ( (ul_port < 1) || (ul_port > 4) )
		{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ul_slot, ul_port);
		return CMD_WARNING;
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	if( GetOnuOperStatus( PonPortIdx, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}

	onu_id = GetLlidByOnuIdx( PonPortIdx, (ulOnuid - 1));
	if( onu_id == ( -1 ))
		{
		vty_out(vty, "  %% Executing err\r\n");
		return( CMD_WARNING );
		}
	
	PARSE_STATISTICS_TYPE_COMMAND_PARAMETER( argv[1], statistics_type, statistics_type);
	
	if ( (statistics_type == PON_STAT_ONU_BER) || (statistics_type == PON_STAT_ONU_FER) )
	{
		collector = PON_OLT_ID;
		statistic_param = onu_id;
	}else if ( (statistics_type == PON_STAT_OLT_BER) || (statistics_type == PON_STAT_OLT_FER) )
	{
		collector = onu_id;
		statistic_param = 0;
	}

	if ( (return_result = ( PAS_get_statistics ( PonPortIdx, 
												 collector, 
												 statistics_type, 
												 statistic_param, 
												 &statistics_data ))) == PAS_EXIT_OK)
		vty_out(vty,"Get statistics result: %s = %Lg %s", stati_type, statistics_data, VTY_NEWLINE);
	else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}
#endif

DEFUN(show_olt_address_table_configuration,
	show_olt_address_table_configuration_cmd,
	"show pon address-table configuration",
	"PON configuration\n"
	"the command gets pon address config\n"
	"address table\n"
	"the PON's address table configuration\n"
	)
{
	short int    olt_id;
#if 0    
	short int       return_result;
	PON_address_table_config_t      address_table_config;
	bool	remove_entry_when_table_full;
#else
	int       return_result;
    OLT_addr_table_cfg_t address_table_config;
#endif
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid;
	short int  PonPortIdx/*, PonChipType*/;

	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	olt_id = PonPortIdx;
#if 1
    return_result = OLT_GetAddressTableConfig(olt_id, &address_table_config);
    if ( OLT_CALL_ISOK(return_result) )
    {
		WRITE_ENABLE_DISABLE_PARAMETER(address_table_config.removed_when_aged ,removed when aged)
		WRITE_ULONG_PARAMETER((address_table_config.aging_timer/SECOND_1),aging time)
		WRITE_PON_NETWORK_TRAFFIC_DIRECTION_PARAMETER(address_table_config.allow_learning,allow learning)
		WRITE_TRUE_FALSE_PARAMETER(address_table_config.discard_llid_unlearned_sa ,discard llid unlearned sa)
		if( address_table_config.discard_unknown_da > 3 )
			address_table_config.discard_unknown_da = 0;
		vty_out(vty,"discard unknown da:%s\r\n",PonLinkDirection_s[address_table_config.discard_unknown_da]);

        WRITE_FULL_MODE_BEHAVIOR_PARAMETER(address_table_config.removed_when_full, full_mode)
    }
    else
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
        		vty_out(vty, "  %% this is %s, this cli is not supported\r\n", OLTAdv_GetChipTypeName(olt_id));
                break;
            case OLT_ERR_NOTEXIST:
        		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
                break;
            default:
    			CLI_ERROR_PRINT
        }

		return( CMD_WARNING);
    }
#else    
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,this cli is not supported\r\n");
		return( CMD_WARNING);
		}

	return_result = PAS_get_address_table_configuration
                                                     (PonPortIdx,
                                                      &address_table_config);

	vty_out(vty,"\r\n");
	if ( return_result ==  PAS_EXIT_OK)
	{
		/*WRITE_OLT_PARAMETER*/
		WRITE_ENABLE_DISABLE_PARAMETER(address_table_config.removed_when_aged ,removed when aged)
		WRITE_ULONG_PARAMETER((address_table_config.aging_timer/SECOND_1),aging time)
		WRITE_PON_NETWORK_TRAFFIC_DIRECTION_PARAMETER(address_table_config.allow_learning,allow learning)
		WRITE_TRUE_FALSE_PARAMETER(address_table_config.discard_llid_unlearned_sa ,discard llid unlearned sa)
		if( address_table_config.discard_unknown_da > 3 )
			address_table_config.discard_unknown_da = 0;
		vty_out(vty,"discard unknown da:%s\r\n",PonLinkDirection_s[address_table_config.discard_unknown_da]);
		/*WRITE_PON_NETWORK_TRAFFIC_DIRECTION_FALSE_PARAMETER(address_table_config.discard_unknown_da,discard unknown da)*/
	}
	else
	{
		CLI_ERROR_PRINT
	}


	return_result = PAS_get_address_table_full_handling_mode
	                                                 (PonPortIdx,
	                                                  &remove_entry_when_table_full);

	if ( return_result ==  PAS_EXIT_OK)
	{
		/*WRITE_OLT_PARAMETER*/
		WRITE_FULL_MODE_BEHAVIOR_PARAMETER(remove_entry_when_table_full ,full_mode)
	}
	else
	{
		CLI_ERROR_PRINT
	}
#endif
    
	vty_out(vty,"\r\n");
	
	return CMD_SUCCESS;
}

DEFUN(show_onu_parameters,
	show_onu_parameters_cmd,
	"show onu parameters <slot/port>",
	"Show running system information\n"
	"PON configuration\n"
	"Get a report containing various parameters of all the ONUs registered to OLT\n"
	"please input the slot/port\n"
	)
{
	short int			  olt_id;
#if 0
	short int			  number, counter=0;
	short int			  return_result;
	PAS_onu_parameters_t  onu_parameters;
#else
	int			     number, counter;
	int			     return_result;
    extern OLT_onu_table_t  olt_onu_global_table;
#endif
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
#if 1
    return_result = OLT_GetAllOnus(olt_id, &olt_onu_global_table);
    if ( OLT_CALL_ISOK(return_result) )
    {
        number = olt_onu_global_table.onu_num;
    
		/*vty_out( vty, "ONUs list of pon%d/%d%s------------------------------------------------------------%s",
	     ul_slot, ul_port,VTY_NEWLINE,VTY_NEWLINE);*/
		vty_out( vty, "ONUs list of pon%d/%d\r\n", ul_slot, ul_port);

		for (counter=0; counter < number; counter++)
		{
			ulOnuid = GetOnuIdxByLlid( PonPortIdx, olt_onu_global_table.onus[counter].llid) + 1;
			vty_out( vty, "ONU %d/%d/%2d |MAC: ", ul_slot, ul_port, ulOnuid );
			{PON_MAC_ADDRESS_CLI_OUT(olt_onu_global_table.onus[counter].mac_address)}
			vty_out( vty, "| RTT: %4u TQ%s", olt_onu_global_table.onus[counter].rtt, VTY_NEWLINE );
		}
		/*vty_out( vty, "------------------------------------------------------------%sTotal: %d ONUs registered%s",VTY_NEWLINE, number, VTY_NEWLINE);*/
		vty_out( vty, "\r\n Total: %d ONUs registered\r\n", number );
    }
#else    
	return_result = PAS_get_onu_parameters ( olt_id,
								 &number,
								  onu_parameters );
	if ( return_result == PAS_EXIT_OK )
	{
		vty_out( vty, "ONUs list of pon%d/%d%\r\n", ul_slot, ul_port);

		for (counter=0; counter < number; counter++)
		{
			ulOnuid = GetOnuIdxByLlid( PonPortIdx, onu_parameters[counter].llid) + 1;
			vty_out( vty, "ONU %d/%d/%2d |MAC: ", ul_slot, ul_port, ulOnuid );
			{PON_MAC_ADDRESS_CLI_OUT(onu_parameters[counter].mac_address)}
			vty_out( vty, "| RTT: %4u TQ%s", onu_parameters[counter].rtt, VTY_NEWLINE );
		}
		/*vty_out( vty, "------------------------------------------------------------%sTotal: %d ONUs registered%s",VTY_NEWLINE, number, VTY_NEWLINE);*/
		vty_out( vty, "\r\n Total: %d ONUs registered\r\n", number );
	}
#endif
	else
		CLI_ERROR_PRINT
		
	return (return_result);
}

DEFUN(show_dba_information,
	show_dba_information_cmd,
	"show dba version <slot/port>",
	"Show running system information\n"
	"Query current DBA information\n"
	"Query current DBA information\n"
	"please input the slot/port\n"
	)
{
	PON_olt_id_t    olt_id;
#if 1
	int       return_result      = EXIT_OK;
	OLT_DBA_version_t stDBAinfo;
	char		    *dba_name, *dba_version;
#else
	short int       return_result      = EXIT_OK;
	char		    dba_name[DBA_INFO_BUFFER_SIZE], dba_version[DBA_INFO_BUFFER_SIZE];
#endif
	/*
	CLI_dba_mode_t  current_dba_mode;
	CLI_dba_t		current_dba_loaded;
	*/

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid*/;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	olt_id = PonPortIdx;

	if (!OLTAdv_IsExist ( olt_id ))
	{
		vty_out(vty, "OLT doesn't exist.%s",
				      VTY_NEWLINE);
		return (CMD_SUCCESS);
	} 

#if 1
    return_result = OLT_GetDBAVersion(olt_id, &stDBAinfo);
	if (return_result != 0)
	{
	 	vty_out(vty, "  %% get dba info err\r\n");
		return return_result;
	}
    else
    {
        dba_name    = stDBAinfo.szDBAname;
        dba_version = stDBAinfo.szDBAversion;
    }
#else
	/*
	current_dba_mode = CLI_get_olt_dba_mode(olt_id);

	if(current_dba_mode == CLI_STATIC_GRANTING_DBA_MODE) 
	{
	    vty_out (vty, "DBA information OLT:%d/%d%sName: Static granting%s", 
				 ul_slot, ul_port, VTY_NEWLINE, VTY_NEWLINE);
	}
	else if (current_dba_mode == CLI_INTERNAL_DBA_MODE)
	{
	    vty_out (vty, "DBA information OLT:%d/%d%sName: Internal DBA%s", 
				 ul_slot, ul_port, VTY_NEWLINE, VTY_NEWLINE);
	}
	else
	*/	
	{
		/*
		current_dba_loaded = CLI_get_olt_loaded_dba(olt_id);
		switch(current_dba_loaded)
		{
		case CLI_PLATO_DBA:
			return_result = PLATO_get_info( olt_id, 
											dba_name , DBA_INFO_BUFFER_SIZE,
											dba_version, DBA_INFO_BUFFER_SIZE );
			if (return_result != PLATO_ERR_OK)
			{
			    PLATO_CLI_ERROR_PRINT
				return return_result;
			}
			break;
		case CLI_PLATO2_DBA:
			*/
#ifdef PLATO_DBA_V3
			return_result = PLATO3_get_info( olt_id, 
											 dba_name , DBA_INFO_BUFFER_SIZE,
											 dba_version, DBA_INFO_BUFFER_SIZE );
			if (return_result != PLATO2_ERR_OK)
			{
			 	vty_out(vty, "  %% get dba info err\r\n");
				return return_result;
			}

#else
			return_result = PLATO2_get_info( olt_id, 
											 dba_name , DBA_INFO_BUFFER_SIZE,
											 dba_version, DBA_INFO_BUFFER_SIZE );
			if (return_result != PLATO2_ERR_OK)
			{
			 	vty_out(vty, "  %% get dba info err\r\n");
				return return_result;
			}
#endif
			/*
			break;
		case CLI_ARCHIMEDES_DBA:
			return_result = ARCHIMEDES_get_info( olt_id, 
											 dba_name , DBA_INFO_BUFFER_SIZE,
											 dba_version, DBA_INFO_BUFFER_SIZE );
			if (return_result != PLATO_ERR_OK)
			{
			    PLATO_CLI_ERROR_PRINT
				return return_result;
			}
			break;
		default:
			vty_out(vty,"Loaded DBA wasn't found%s", VTY_NEWLINE);
			return_result = EXIT_ERROR;
			
		}
		*/
	}
#endif
		if (return_result == EXIT_OK)
			vty_out (vty, "DBA information pon:%d/%d%sName: %s, Version: %s%s", 
				 ul_slot, ul_port, VTY_NEWLINE, dba_name, dba_version, VTY_NEWLINE);
		else
			CLI_ERROR_PRINT


	return CMD_SUCCESS;
}


DEFUN(show_dba_sla,
	show_dba_sla_cmd,
	"show dba sla <slot/port/onuid>",
	"Show running system information\n"
	"Query current DBA\n"
	"Gets the SLA parameters for a specific LLID\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)
{
	PON_olt_id_t        olt_id;
#if 1
    ONU_SLA_INFO_t SLA_Info;
#else
	PLATO3_SLA_t   plato_SLA3;
	PLATO2_SLA_t   plato_SLA2;
	PON_llid_t          llid;
	/*
	INTERNAL_DBA_SLA_t  internal_sla;
	PLATO_SLA_EXT_t		plato_SLA;	
	ARCHIMEDES_SLA_t    archimedes_SLA;
	*/
#endif
	short				DBA_error_code;
	/*
	CLI_dba_mode_t		current_dba_mode;
	CLI_dba_t			current_dba_loaded;
	*/
	short int           return_result  = EXIT_OK;
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	char dba_flag = 0;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if( GetOnuOperStatus( PonPortIdx, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}

#if 1
    if ( 0 == (return_result = OnuMgt_GetOnuSLA(PonPortIdx, ulOnuid - 1, &SLA_Info)) )
    {
        dba_flag = (char)SLA_Info.SLA_Ver;
        DBA_error_code = SLA_Info.DBA_ErrCode;
    }    
#else    
	llid = GetLlidByOnuIdx( PonPortIdx, (ulOnuid - 1));
	if( llid == INVALID_LLID)
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}
	
	/*
	current_dba_mode = CLI_get_olt_dba_mode(olt_id);
	if( current_dba_mode == CLI_STATIC_GRANTING_DBA_MODE )
	{
		vty_out(vty, "show sla command isn't supported in current running DBA%s", VTY_NEWLINE);
		return EXIT_ERROR;
	}
	else if ( current_dba_mode == CLI_INTERNAL_DBA_MODE )
	{
		return_result = INTERNAL_DBA_get_sla( olt_id, llid, &internal_sla );
	    if ( return_result == INTERNAL_DBA_EXIT_OK)
		    vty_out (vty, "Internal DBA get SLA to LLID %d of OLT %d succeeded. Min BW: %d, Max BW: %d%s", 
				     llid, olt_id, internal_sla.min_bw, internal_sla.max_bw, VTY_NEWLINE);
	    else 
		    INTERNAL_DBA_CLI_ERROR_PRINT
	}
	else
	{
		current_dba_loaded = CLI_get_olt_loaded_dba(olt_id);
		switch(current_dba_loaded)
		{
		case CLI_PLATO_DBA:
			return_result = PLATO_get_SLA_ext( olt_id, 
											  (unsigned char)llid,
											  &plato_SLA, 
											  &DBA_error_code );
			if ( (return_result == PLATO_ERR_OK) )
			{
				if( DBA_error_code == PLATO_ECODE_NO_ERROR )
				{
			   	    vty_out (vty, "PLATO get SLA from LLID %d of OLT %d:%s",llid, olt_id, VTY_NEWLINE);
		            WRITE_USHORT_PARAMETER(plato_SLA.class          , class          )
	                WRITE_USHORT_PARAMETER(plato_SLA.max_gr_bw      , max_gr_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA.max_gr_bw_fine , max_gr_bw_fine )
					WRITE_USHORT_PARAMETER(plato_SLA.max_be_bw      , max_be_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA.max_be_bw_fine , max_be_bw_fine )
				}
				else
					PLATO_CLI_ERROR_DBA_PRINT
			}else
				PLATO_CLI_ERROR_PRINT
			break;
		case CLI_PLATO2_DBA:
		*/
#ifdef PLATO_DBA_V3
		dba_flag = 3;
#else
		dba_flag = 2;
#endif
#endif
		if( dba_flag == 2 )
		{
        	PLATO2_SLA_t *plato_SLA2;

#if 0
			return_result = PLATO2_get_SLA( olt_id, 
									       (unsigned char)llid,
									       &plato_SLA2, 
									       &DBA_error_code );
#endif
			if ( (return_result == 0) )
			{
			    plato_SLA2 = &SLA_Info.SLA.SLA2;
				if( DBA_error_code == PLATO2_ECODE_NO_ERROR )
				{
	   				vty_out (vty, "PLATO2 get SLA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
					WRITE_USHORT_PARAMETER(plato_SLA2->class          , class          )
					WRITE_USHORT_PARAMETER(plato_SLA2->delay          , delay          )
					WRITE_USHORT_PARAMETER(plato_SLA2->max_gr_bw      , max_gr_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA2->max_gr_bw_fine , max_gr_bw_fine )
					WRITE_USHORT_PARAMETER(plato_SLA2->max_be_bw      , max_be_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA2->max_be_bw_fine , max_be_bw_fine )
				} 
			}
			else
            {
				vty_out(vty, "  %% get SLA from onu %d/%d/%d err\r\n", ul_slot, ul_port, ulOnuid );
				return( CMD_WARNING );
			}
		}
		else if( dba_flag == 3 )
		{
        	PLATO3_SLA_t   *plato_SLA3;
		
#if 0
			return_result = PLATO3_get_SLA( olt_id, 
									       (unsigned char)llid,
									       &plato_SLA3, 
									       &DBA_error_code );
#endif
			if ( (return_result == 0) )
			{
			    plato_SLA3 = &SLA_Info.SLA.SLA3;
				if( DBA_error_code == PLATO3_ECODE_NO_ERROR )
				{
	   				vty_out (vty, "PLATO3 get SLA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
					WRITE_USHORT_PARAMETER(plato_SLA3->class          , class          )
					/*WRITE_USHORT_PARAMETER(plato_SLA2.delay          , delay          )*/
					WRITE_USHORT_PARAMETER(plato_SLA3->fixed_packet_size      , fixed_packet_size      )
					WRITE_USHORT_PARAMETER(plato_SLA3->fixed_bw, fixed_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA3->fixed_bw_fine, fixed_bw_fine      )
					WRITE_USHORT_PARAMETER(plato_SLA3->max_gr_bw      , max_gr_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA3->max_gr_bw_fine , max_gr_bw_fine )
					WRITE_USHORT_PARAMETER(plato_SLA3->max_be_bw      , max_be_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA3->max_be_bw_fine , max_be_bw_fine )
				} 
			}
			else
            {
				vty_out(vty, "  %% get SLA from onu %d/%d/%d err\r\n", ul_slot, ul_port, ulOnuid );
				return( CMD_WARNING );
			}
		}
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		else if( dba_flag == 4 )
		{
            PLATO4_SLA_t   *plato_SLA4;
			
			if ( (return_result == PLATO4_ERR_OK) )
			{
			    plato_SLA4 = &SLA_Info.SLA.SLA4;
				if( DBA_error_code == PLATO4_ECODE_NO_ERROR )
				{
	   				vty_out (vty, "PLATO4 get SLA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
					WRITE_USHORT_PARAMETER(plato_SLA4->class          , class          )
					/*WRITE_USHORT_PARAMETER(plato_SLA2.delay          , delay          )*/
					WRITE_USHORT_PARAMETER(plato_SLA4->fixed_packet_size      , fixed_packet_size      )
					WRITE_USHORT_PARAMETER(plato_SLA4->fixed_bw, fixed_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA4->fixed_bw_fine, fixed_bw_fine      )
					WRITE_USHORT_PARAMETER(plato_SLA4->max_gr_bw      , max_gr_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA4->max_gr_bw_fine , max_gr_bw_fine )
					WRITE_USHORT_PARAMETER(plato_SLA4->max_be_bw      , max_be_bw      )
					WRITE_USHORT_PARAMETER(plato_SLA4->max_be_bw_fine , max_be_bw_fine )
				} 
			}
			else{
				vty_out(vty, "  %% get SLA from onu %d/%d/%d err\r\n", ul_slot, ul_port, ulOnuid );
				return( CMD_WARNING );
				}
		}
        /*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        else if ( dba_flag == 5 )
        {
            TkLinkSlaDbaInfo *TkLinkSLA;
            TkSlaShaperParams *TkSlaMin;
            TkSlaShaperParams *TkSlaMax;
            TkSlaDbaParams *TkLinkDBA;

		    TkLinkSLA = &SLA_Info.SLA.SLA5;
            TkSlaMin  = &TkLinkSLA->sla.min;
            TkSlaMax  = &TkLinkSLA->sla.max;
            TkLinkDBA = &TkLinkSLA->dba;
            
			if( DBA_error_code == 0 )
			{
   				vty_out (vty, "TKDBA get SLA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
				WRITE_ULONG_PARAMETER(TkSlaMin->bandwidth, min_gr_bw      )
				WRITE_USHORT_PARAMETER(TkSlaMin->burst, min_gr_burst_size )
				WRITE_USHORT_PARAMETER(TkSlaMin->schedulerLevel, min_gr_schedulerLevel )
				WRITE_USHORT_PARAMETER(TkSlaMin->weight, min_gr_weight )
				
				WRITE_ULONG_PARAMETER(TkSlaMax->bandwidth, max_be_bw      )
				WRITE_USHORT_PARAMETER(TkSlaMax->burst, max_be_burst_size )
				WRITE_USHORT_PARAMETER(TkSlaMax->schedulerLevel, max_be_schedulerLevel )
				WRITE_USHORT_PARAMETER(TkSlaMax->weight, max_be_weight )
				
   				vty_out (vty, "TKDBA get DBA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
				WRITE_LONG_HEX_PARAMETER(TkLinkDBA->dbaOptions          , dbaOptions          )
				WRITE_USHORT_PARAMETER(TkLinkDBA->pollingLevel, pollingLevel          )
				WRITE_ULONG_PARAMETER(TkLinkDBA->tdmRate, tdmRate          )
				WRITE_ULONG_PARAMETER(TkLinkDBA->tdmGrantLength, tdmGrantLength          )
			} 
        }
        else if ( dba_flag == 6 )
        {
            BcmLinkSlaDbaInfo *BcmLinkSLA;
            bcmEmmiSchedulerSolicited *BcmSlaMin;
            bcmEmmiSchedulerSolicited *BcmSlaMax;
            BcmSlaDbaParams *BcmLinkDBA;

		    BcmLinkSLA = &SLA_Info.SLA.SLA6;
            BcmSlaMin  = &BcmLinkSLA->sla.min;
            BcmSlaMax  = &BcmLinkSLA->sla.max;
            BcmLinkDBA = &BcmLinkSLA->dba;
            
			if( DBA_error_code == 0 )
			{
   				vty_out (vty, "BCMDBA get SLA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
				WRITE_ULONG_PARAMETER(BcmSlaMin->bandwidth, min_gr_bw      )
				WRITE_USHORT_PARAMETER(BcmSlaMin->burstSize, min_gr_burst_size )
				WRITE_USHORT_PARAMETER(BcmSlaMin->level, min_gr_schedulerLevel )
				WRITE_USHORT_PARAMETER(BcmSlaMin->weight, min_gr_weight )
				
				WRITE_ULONG_PARAMETER(BcmSlaMax->bandwidth, max_be_bw      )
				WRITE_USHORT_PARAMETER(BcmSlaMax->burstSize, max_be_burst_size )
				WRITE_USHORT_PARAMETER(BcmSlaMax->level, max_be_schedulerLevel )
				WRITE_USHORT_PARAMETER(BcmSlaMax->weight, max_be_weight )
				
   				vty_out (vty, "BCMDBA get DBA from onu%d/%d/%d:%s", ul_slot, ul_port, ulOnuid, VTY_NEWLINE);
				WRITE_LONG_HEX_PARAMETER(BcmLinkDBA->dba_flags, dbaOptions          )
				WRITE_USHORT_PARAMETER(BcmLinkDBA->polling_level, pollingLevel          )
				WRITE_ULONG_PARAMETER(BcmLinkDBA->token_size, token_size      )
				WRITE_ULONG_PARAMETER(BcmLinkDBA->tdm.interval, tdmInterval(96us)          )
				WRITE_USHORT_PARAMETER(BcmLinkDBA->tdm.length, tdmGrantLength          )
				WRITE_USHORT_PARAMETER(BcmLinkDBA->tdm.extraGrantLength, extraGrantLength          )
			} 
        }
        else
        {
			vty_out(vty, "  %% get SLA from onu %d/%d/%d err\r\n", ul_slot, ul_port, ulOnuid );
			return( CMD_WARNING );
        }
		/*	
			break;

		case CLI_ARCHIMEDES_DBA:
			return_result = ARCHIMEDES_get_SLA( olt_id, 
									  (unsigned char)llid,
									  &archimedes_SLA, 
									  &DBA_error_code );
			if ( return_result == ARCHIMEDES_ERR_OK )
			{
				if( DBA_error_code == ARCHIMEDES_ECODE_NO_ERROR )
				{
		   			pas_vty_out (vty, "ARCHIMEDES get SLA from LLID %d of OLT %d:%s",llid, olt_id, VTY_NEWLINE);
					WRITE_USHORT_PARAMETER(archimedes_SLA.priority_class          , class          )
					WRITE_USHORT_PARAMETER(archimedes_SLA.fixed_packet_size, fixed_packet_size      )
					WRITE_USHORT_PARAMETER(archimedes_SLA.fixed_bw       , fixed_bw      )
					WRITE_USHORT_PARAMETER(archimedes_SLA.fixed_bw_fine  , fixed_bw_fine )
					WRITE_USHORT_PARAMETER(archimedes_SLA.max_gr_bw      , max_gr_bw      )
					WRITE_USHORT_PARAMETER(archimedes_SLA.max_gr_bw_fine , max_gr_bw_fine )
					WRITE_USHORT_PARAMETER(archimedes_SLA.max_be_bw      , max_be_bw      )
					WRITE_USHORT_PARAMETER(archimedes_SLA.max_be_bw_fine , max_be_bw_fine )
				}
				else
					ARCHIMEDES_CLI_ERROR_DBA_PRINT
			}else
				ARCHIMEDES_CLI_ERROR_PRINT
			break;
		default:
			pas_vty_out(vty,"Loaded DBA wasn't found%s", VTY_NEWLINE);
			return_result = EXIT_ERROR;
		}
	}
	*/
	
	return CMD_SUCCESS;
}

DEFUN(show_uni_port_config,
	show_uni_port_config_cmd,
	"show onu uni-port-mac-configuration <slot/port/onuid>",
	"Show running system information\n"
	"ONU configuration\n"
	"Query ONU UNI port mac configuration\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)
{
	PON_olt_id_t						  olt_id;
	PON_onu_id_t						  onu_id;
	short int							  return_result;
	PON_oam_uni_port_mac_configuration_t  mac_configuration;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	
#if 0	
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if( GetOnuOperStatus( PonPortIdx, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}

	onu_id = GetLlidByOnuIdx( PonPortIdx, (ulOnuid - 1));
	if( onu_id == INVALID_LLID)
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}


    return_result = PAS_get_onu_uni_port_mac_configuration_v4( olt_id, 
												            onu_id, 
														    &mac_configuration );
#endif		
onu_id=ulOnuid - 1;
return_result = OnuMgt_GetOnuUniMacCfg( olt_id, onu_id, &mac_configuration );

    if( return_result == PAS_EXIT_OK )
    {
		mac_configuration.advertise = ENABLE;
		WRITE_MAC_TYPE_PARAMETER(mac_configuration.mac_type, MAC type )
        WRITE_SHORT_PARAMETER(mac_configuration.mii_type_rate, MII type rate)
        WRITE_DUPLEX_PARAMETER(mac_configuration.mii_type_duplex, MII type duplex)
        WRITE_ENABLE_DISABLE_PARAMETER(mac_configuration.autonegotiation, Autonegotiation)
        WRITE_MASTER_SLAVE_PARAMETER(mac_configuration.master, Maste/Slave)
       /*WRITE_ENABLE_DISABLE_PARAMETER(mac_configuration.advertise, advertise-not supported)*/
        WRITE_ENABLE_DISABLE_PARAMETER(mac_configuration.advertise, Advertise)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement._10base_tx_half_duplex,  MII phy advertisement 10base tx half duplex)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement._10base_tx_full_duplex,  MII phy advertisement 10base tx full duplex)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement._100base_tx_half_duplex, MII phy advertisement 100base tx half duplex)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement._100base_tx_full_duplex, MII phy advertisement 100base tx full duplex)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement._100base_t4, MII phy advertisement 100base t4)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement.pause, MII phy advertisement pause)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.mii_phy_advertisement.asymmetric_pause, MII phy advertisement asymmetric pause)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.gmii_phy_advertisement._1000base_tx_half_duplex, GMII phy advertisement 1000base tx half duplex)
        WRITE_TRUE_FALSE_PARAMETER(mac_configuration.advertisement_details.gmii_phy_advertisement._1000base_tx_full_duplex, GMII phy advertisement 1000base tx full duplex)
        WRITE_MASTER_SLAVE_PARAMETER(mac_configuration.advertisement_details.gmii_phy_advertisement.preferable_port_type, GMII phy advertisement preferable port type)
       /*WRITE_UCHAR_PARAMETER(mac_configuration.mdio_phy_address, mdio_phy_address-not supported)*/
        WRITE_SHORT_PARAMETER(0, MDIO phy address-not supported)
	}
	else
		CLI_ERROR_PRINT	
	return (CMD_SUCCESS);
}	  

DEFUN(show_optics_parameters,
	show_optics_parameters_cmd,
	"show pon optics <slot/port>",
	"Show running system information\n"
	"Get pon optics parameters\n"
	"show pon optics parameters\n"
	"please input the slot/port\n"
	)
{
	short int							 olt_id;
	short int							 return_result;
	PON_olt_optics_configuration_t       optics_configuration;
    OLT_optics_detail_t                  optics_detail;  
	bool                                 pon_tx_signal;

	unsigned long  ul_slot = 0, ul_port = 0 ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	olt_id = PonPortIdx;

#if 1
    return_result = OLT_GetOpticsDetail(olt_id, &optics_detail);
    if ( OLT_CALL_ISOK(return_result) )
    {
        optics_configuration = optics_detail.pon_optics_params;
        pon_tx_signal        = optics_detail.pon_tx_signal;

        return_result = 0;
    }
#else		
    return_result = PAS_get_olt_optics_parameters ( olt_id,
                                                    &optics_configuration,
                                                    &pon_tx_signal );
#endif
   if ( return_result == 0)
   {
  
	    WRITE_OLT_PARAMETER
        WRITE_SHORT_HEX_PARAMETER(optics_configuration.agc_lock_time, AGC lock time)	
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.agc_reset_configuration.gate_offset, AGC reset configuration gate offset)
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.agc_reset_configuration.discovery_offset, AGC reset configuration discovery offset)
		WRITE_SHORT_PARAMETER(optics_configuration.agc_reset_configuration.duration, AGC reset configuration duration)
		WRITE_POLARITY_PARAMETER(optics_configuration.agc_reset_configuration.polarity, AGC reset configuration polarity)
 
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.cdr_lock_time, CDR lock time)	
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.cdr_reset_configuration.gate_offset, CDR reset configuration offset)
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.cdr_reset_configuration.discovery_offset, CDR reset configuration discovery offset)
		WRITE_SHORT_PARAMETER(optics_configuration.cdr_reset_configuration.duration, CDR reset configuration duration)
		WRITE_POLARITY_PARAMETER(optics_configuration.cdr_reset_configuration.polarity, CDR reset configuration polarity)
                                                         
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.cdr_end_of_grant_reset_configuration.offset, CDR end of grant reset configuration offset)
		WRITE_SHORT_PARAMETER(optics_configuration.cdr_end_of_grant_reset_configuration.duration, CDR end of grant reset configuration duration)
		WRITE_POLARITY_PARAMETER(optics_configuration.cdr_end_of_grant_reset_configuration.polarity, CDR end of grant reset configuration polarity)

		WRITE_SHORT_HEX_PARAMETER(optics_configuration.optics_end_of_grant_reset_configuration.offset, Optics end of grant reset configuration offset)
		WRITE_SHORT_PARAMETER(optics_configuration.optics_end_of_grant_reset_configuration.duration, Optics end of grant reset configuration duration)
		WRITE_POLARITY_PARAMETER(optics_configuration.optics_end_of_grant_reset_configuration.polarity, Optics end of grant reset configuration polarity)

		WRITE_ENABLE_DISABLE_PARAMETER(optics_configuration.discovery_re_locking_enable, Discovery re-locking enable)
		WRITE_POLARITY_PARAMETER(optics_configuration.discovery_laser_rx_loss_polarity, Discovery laser Rx loss polarity)
		WRITE_POLARITY_PARAMETER(optics_configuration.pon_tx_disable_line_polarity, PON Tx disable line polarity)
		WRITE_SHORT_HEX_PARAMETER(optics_configuration.optics_dead_zone, Optics dead zone)
		WRITE_TRUE_FALSE_PARAMETER(optics_configuration.use_optics_signal_loss, Use optics signal loss)

		WRITE_POLARITY_PARAMETER(optics_configuration.polarity_configuration.pon_port_link_indication_polarity, pon port link indication polarity)
		WRITE_POLARITY_PARAMETER(optics_configuration.polarity_configuration.cni_port_link_indication_polarity, CNI port link indication polarity)
		WRITE_POLARITY_PARAMETER(optics_configuration.polarity_configuration.pon_tbc_polarity                 , TBC port link indication polarity)

        WRITE_SHORT_HEX_PARAMETER(optics_configuration.discovery_laser_on_time , Discovery laser On time)
	    WRITE_SHORT_HEX_PARAMETER(optics_configuration.discovery_laser_off_time, Discovery laser Off time)

        WRITE_ENABLE_DISABLE_PARAMETER(pon_tx_signal,pon_tx_signal)
       
    }
   else
	    CLI_ERROR_PRINT
	return (return_result);
}

DEFUN(show_oam_limited,
	show_oam_limited_cmd,
	"show oam rate-limit <slot/port>",
	"Show running system information\n"
	"PON configuration\n"
	"Retrieve OAM frames rate limit\n"
	"please input the slot/port\n"
	)
{
	bool                         ignore_oam_limit; 
	short int                    return_result;

	unsigned long  ul_slot = 0, ul_port = 0 ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

#if 1
    return_result = OLT_OamIsLimit(PonPortIdx, &ignore_oam_limit);
#else        
	return_result = PAS_get_oam_configuration (PonPortIdx, &ignore_oam_limit);
#endif
	if ( OLT_CALL_ISOK(return_result) ){
		if(ignore_oam_limit == ENABLE)
			WRITE_STRING_PARAMETER(enum_s(DISABLE), OAM rate limit)
		else
			WRITE_STRING_PARAMETER(enum_s(ENABLE), OAM rate limit)
	}else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}

DEFUN(show_llid_vlan_mode,
	show_llid_vlan_mode_cmd,
	"show onu vlan-mode <slot/port/onuid>",
	"Show running system information\n"	
	"Query the VLAN handling mode for uplink frames arriving from the specified onu\n"
	"VLAN's\n"
	"please input the slot/port/onuid\n"
	/*OnuIDString*/
	)
{
	PON_olt_id_t			        olt_id;
	PON_llid_t				        llid;
	short int				        return_result;
	PON_olt_vlan_uplink_config_t    vlan_uplink_config ;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	/*short int  PonChipType;*/
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
    
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}
	olt_id = PonPortIdx;
#if 0
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,this cli is not support\r\n");
		return( CMD_WARNING);
		}
	
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if( GetOnuOperStatus( PonPortIdx, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ul_slot, ul_port, ulOnuid );
		return( CMD_WARNING );
		}

	llid = GetLlidByOnuIdx( PonPortIdx, (ulOnuid - 1));
	if( llid == ( -1 ))
		{
		vty_out(vty, "  %% Executing error\r\n");
		return( CMD_WARNING );
		}

	olt_id = PonPortIdx;
	return_result = PAS_get_llid_vlan_mode  ( olt_id, 
											  llid, 
											  &vlan_uplink_config );
#endif
llid=ulOnuid - 1;
return_result=OnuMgt_GetOnuVlanMode(olt_id,llid, &vlan_uplink_config);


	if ( return_result == PAS_EXIT_OK )
	{
		WRITE_OLT_PARAMETER
		WRITE_LLID_PARAMETER
        WRITE_UNTAGGED_FRAMES_AUTHENTICATION_VID_PARAMETER(vlan_uplink_config.untagged_frames_authentication_vid,Untagged frames authentication vid)
        WRITE_AUTHENTICATED_VID_WITH_ALL_FRAMES_AUTHENTICATE(vlan_uplink_config.authenticated_vid     ,Authenticated vid)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_untagged               ,Untagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_tagged                 ,Tagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_null_tagged            ,Null tagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_nested                 ,Nested frames)
        WRITE_VLAN_UPLINK_MANIPULATION_PARAMETER(vlan_uplink_config.vlan_manipulation                 ,Vlan manipulation)
        switch(vlan_uplink_config.vlan_manipulation) 
        {                                                                  
        case PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION              : 
        case PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED           : 
            break;
        case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED     : 
        case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED : 
            WRITE_USHORT_PARAMETER(vlan_uplink_config.new_vlan_tag_id,New vlan tag id)
            WRITE_VLAN_ETHERNET_TYPE_PARAMETER(vlan_uplink_config.vlan_type,Vlan type)
            WRITE_VLAN_PRIORITY_WITH_ORIGINAL_PRIORITY(vlan_uplink_config.vlan_priority,Vlan priority)
    	    break;
        default:
            break;
        }
	}else
    {
		CLI_ERROR_PRINT
    }

	return (return_result);
}

DEFUN(show_llid_encryption,
	show_llid_encryption_cmd,
	"show onu encryption <slot/port/onuid>",
	"Show running system information\n"
	"onu encrypt\n"
	"show onu encryption status\n"
	"please input the slot/port/onuid\n"
	/*"onu index\n"*/
	)
{
	PON_olt_id_t			olt_id;
	PON_llid_t				llid;
	PON_llid_parameters_t   llid_parameters;
	short int				return_result;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	short int OnuIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
    
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
#if 0	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	OnuIdx  = ulOnuid -1;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) !=  ROK ) 
		{
		vty_out(vty, "  %% onu%d/%d/%d is not exist\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP)
		{
		vty_out(vty, "  %% onu%d/%d/%d is off-line\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}
	
	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == (-1)) 
		{
		vty_out(vty,"  %% Executing error.\r\n" );
		return( CMD_WARNING );
		}

	/* modified by shixh20101216*/
#if 0
	return_result = PAS_get_llid_parameters ( olt_id, 
											  llid, 
											  &llid_parameters );
#else
    return_result = OnuMgt_GetLLIDParams(PonPortIdx, OnuIdx,  &llid_parameters);
#endif    

	if ( return_result == 0 )
	{
		WRITE_OLT_PARAMETER
		WRITE_LLID_PARAMETER
        WRITE_ENCRYPTION_MODE_PARAMETER(llid_parameters.encryption_mode, Encryption mode)

	}else
		CLI_ERROR_PRINT
	
	return (return_result);
}


#if 0
DEFUN(show_llid_authorize,
	show_llid_authorize_cmd,
	"show onu-llid authorization <slot/port/onuid>",
	"Show running system information\n"
	"LLID configuration\n"
	"Query onu-LLID authorization status\n"
	"please input the slot/port/onuid\n"
	)
{
	PON_olt_id_t			olt_id;
	PON_llid_t				llid;
	PON_llid_parameters_t   llid_parameters;
	short int				return_result;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx;
	short int OnuIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	OnuIdx  = ulOnuid -1;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) !=  ROK ) 
		{
		vty_out(vty, "  %% onu%d/%d/%d is not exist\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}

	if(Mac_address_exist( olt_id, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, &llid) != TRUE )
		{
		vty_out(vty, "  %% onu%d/%d/%d is off-line\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}

	return_result = PAS_get_llid_parameters ( olt_id, 
											  llid, 
											  &llid_parameters );

	if ( return_result == PAS_EXIT_OK )
	{

		WRITE_OLT_PARAMETER
		WRITE_LLID_PARAMETER
	    WRITE_AUTHORIZE_MODE_PARAMETER(llid_parameters.authorization_mode,Authorization mode)

	}else
		CLI_ERROR_PRINT
	
	return (return_result);
}

DEFUN(show_llid_fec,
	show_llid_fec_cmd,
	"show llid fec <slot/port/onuid>",
	"Show running system information\n"
	"LLID configuration\n"
	"Query LLID fec status\n"
	"please input the slot/port/onuid\n"
	)
{
	PON_olt_id_t			olt_id;
	PON_llid_t				llid;
	short int				return_result;
	bool                    downlink_fec; 
	bool                    uplink_fec;
	bool                    last_uplink_frame_fec;

	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int  PonPortIdx, PonChipType;
	short int OnuIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port , (ULONG *)&ulOnuid);	
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ul_slot, (ULONG *)&ul_port );
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	/*ulOnuid = ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	OnuIdx  = ulOnuid -1;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) !=  ROK ) 
		{
		vty_out(vty, "  %% onu%d/%d/%d is not exist\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP)
		{
		vty_out(vty, "  %% onu%d/%d/%d is off-line\r\n", ul_slot, ul_port,  ulOnuid );
		return( CMD_WARNING );
		}

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx );
	if( llid == (-1))
		{
		vty_out(vty,"  %% Executing error.\r\n" );
		return( CMD_WARNING );
		}

	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001, not support this cli\r\n");
		return( CMD_WARNING );
		}

	if( PonChipType == PONCHIP_PAS5201)
	{
		return_result = PAS_get_llid_fec_mode ( olt_id, 
											    llid, 
	                                            &downlink_fec, 
	                                            &uplink_fec,
	                                            &last_uplink_frame_fec );


		if ( return_result == PAS_EXIT_OK )
		{
			WRITE_OLT_PARAMETER
			WRITE_LLID_PARAMETER
	        WRITE_ENABLE_DISABLE_PARAMETER(downlink_fec,Downlink FEC);
	        WRITE_ENABLE_DISABLE_PARAMETER(uplink_fec,Uplink FEC);
	        WRITE_ENABLE_DISABLE_PARAMETER(last_uplink_frame_fec,Last uplink frame FEC);
		}else
			CLI_ERROR_PRINT
		return( CMD_SUCCESS );
	}
	else{

		}
	
	
	return (CMD_WARNING);
}

DEFUN(show_gpio,
	show_gpio_cmd,
	"show pon <slot/port> gpio line-number <0-3>",
	"Show running system information\n"
	"OLT configuration\n"
	"please input the slot/port\n"
	"Read a GPIO (General purpose IO) line\n"
	"GPIO line number controlled\n"
	"Setting line number\n")
{
	PON_olt_id_t            olt_id;
	PON_gpio_lines_t        line_number;
	PON_gpio_line_io_t      set_direction;
	short int               set_value;
	short int               return_result;
	PON_gpio_line_io_t	    direction;
	bool				    value;	

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid*/;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	olt_id = PonPortIdx;
	
	line_number = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	set_direction = PON_GPIO_LINE_INPUT;
	set_value     = PON_GPIO_LINE_NOT_CHANGED;

	return_result = PAS_gpio_access_extended_v4 ( olt_id,
										       line_number,
										       set_direction,
										       set_value,
										       &direction,
										       &value );
    if( return_result == PAS_EXIT_OK )
    {
		vty_out(vty,"GPIO input line: %d%s",line_number, VTY_NEWLINE);
		vty_out(vty,"Value          : %d%s",value, VTY_NEWLINE);
	}else
		CLI_ERROR_PRINT
    return (return_result);
}

DEFUN (show_olt_mdio,
	show_olt_mdio_cmd,
	"show pon <slot/port> mdio phy <0-31> register <0-31>",
	"Show running system information\n"
	"PON configuration\n"
	"please input the slot/port\n"
	"Display MDIO register value from an OLT device\n"
	"input PHY address\n"
	"Physical address\n"
	"input register address\n"
	"Register value\n")
{
	short int                phy_address;
	short int                reg_address;
	short int				 return_result;
	unsigned short int		 value;

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid*/;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	phy_address = ( ULONG ) VOS_AtoL( argv[ 1 ] );
	reg_address = ( ULONG ) VOS_AtoL( argv[ 2 ] );
	
	return_result = (short int)PAS_read_mdio_register(PonPortIdx,phy_address,reg_address,&value);

	if ( return_result ==  PAS_EXIT_OK)
	    vty_out ( vty, "read MDIO register: OLT: %s/port%d, phy_address: %d, register address: %d, value: %d%s",
			      CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) , phy_address, reg_address, value, VTY_NEWLINE);
	else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}

DEFUN (show_olt_eeprom,
	show_olt_eeprom_cmd,
	"show pon <slot/port> eeprom register <0-4096>",
	"Show running system information\n"
	"PON configuration\n"
	"please input the slot/port\n"
	"Read an OLT EEPROM device register content.\n"
	"EEPROM register number\n"
	"Setting register address\n")
{
	short int                reg_address;
	unsigned short int		 data;
	short int				 return_result;

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid */;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	reg_address = ( ULONG ) VOS_AtoL( argv[ 1 ] );
	return_result = PAS_read_olt_eeprom_register(PonPortIdx,reg_address,&data);

	if ( return_result ==  PAS_EXIT_OK)
		vty_out ( vty,"Read OLT eeprom register:%sOLT: %s/port%d%sEEPROM register address: 0x%x%sData: 0x%hx%s",
                  VTY_NEWLINE,
				  CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), 
                  VTY_NEWLINE,
				  reg_address, 
                  VTY_NEWLINE,
				  data, 
				  VTY_NEWLINE);
	else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}

DEFUN (show_olt_register,
	show_olt_register_cmd,
	"show pon <slot/port> register address <register_addr>",
	"Show running system information\n"
	"PON configuration\n"
	"please input the slot/port\n"
	"Read an OLT register value.\n"
	"Register address\n"
	"input register address,Hex format\n")
{

	unsigned long            register_address;
	short int				 return_result;
	unsigned long   		 data;

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid */;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

	register_address = ( ULONG ) VOS_StrToUL( argv[1], NULL, 16 );
	
	return_result = PAS_get_olt_register(PonPortIdx,register_address,&data);


	if ( return_result ==  PAS_EXIT_OK)
	{

	    WRITE_LONG_HEX_PARAMETER(data,data);
	}
	else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}

DEFUN(show_olt_cni_management_mode,
	show_olt_cni_management_mode_cmd,
	"show pon <slot/port> cni management-mode",
	"Show running system information\n"
	"PON configuration\n"
	"please input the slot/port\n"
	"CNI configuration\n"
	"Query OLT CNI management mode\n"
	 )
{
	short int			return_result;
	bool                     cni_management_mode;

	unsigned long  ul_slot = 0, ul_port = 0/*, ulOnuid*/ ;
	short int  PonPortIdx;
	/*unsigned long ulIfIndex;*/
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	/*
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	*/
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ul_slot, ul_port );
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}
        
	return_result = PAS_get_cni_management_mode(PonPortIdx,&cni_management_mode);

	if ( return_result ==  PAS_EXIT_OK)
	{
		vty_out ( vty,"Read olt cni management mode  of %s/port%d succeeded.%s",
	              CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), VTY_NEWLINE);
	    WRITE_ENABLE_DISABLE_PARAMETER(cni_management_mode,cni_management_mode);
	}
	else
		CLI_ERROR_PRINT
	return CMD_SUCCESS;
}
#endif

DEFUN (show_passoft_system_param,
	show_passoft_system_param_cmd,
	"show pas-soft system-param",
	"show running system information\n"
	"show pas-soft running system information\n"
	"pas-soft system parameters\n" )
{
    PAS_system_parameters_t   system_parameters;
    short int                 return_result = PAS_EXIT_OK;
    bool                      statistics_sampling_cycle      = FALSE; 
    bool                      monitoring_cycle               = FALSE; 
    bool                      host_olt_msgs_timeout          = FALSE; 
    bool                      olt_reset_timeout              = FALSE; 
    bool    	              automatic_authorization_policy = FALSE; 

	/*
    if (argc == 1) 
    {
        if ( strcmp(argv[0],"automatic-authorization") == 0 ) 
        {
            automatic_authorization_policy = TRUE;
        } else
        if ( strcmp(argv[0],"statistics-cycle") == 0 ) 
        {
            statistics_sampling_cycle = TRUE;
        } else
        if ( strcmp(argv[0],"monitoring-cycle") == 0 ) 
        {
            monitoring_cycle = TRUE;
        } else
        if ( strcmp(argv[0],"host-msg-timeout") == 0 ) 
        {
            host_olt_msgs_timeout = TRUE;
        } else
        if ( strcmp(argv[0],"olt-reset-timeouts") == 0 ) 
        {
            olt_reset_timeout = TRUE;
        } 
    }
    else
		*/
    {
        statistics_sampling_cycle      = TRUE;
        monitoring_cycle               = TRUE;
        host_olt_msgs_timeout          = TRUE;
        olt_reset_timeout              = TRUE;
        automatic_authorization_policy = TRUE;
    }

#if 0    
    return_result = PAS_get_system_parameters(&system_parameters);
#else
    system_parameters = PAS_system_parameters;
#endif	

	if ( return_result == PAS_EXIT_OK )
	{
        if (statistics_sampling_cycle) 
        {
            WRITE_LONG_PARAMETER  (system_parameters.statistics_sampling_cycle   , Statistics sampling cycle             )        
        }
        if (monitoring_cycle) 
        {
            WRITE_LONG_PARAMETER  (system_parameters.monitoring_cycle            , Monitoring cycle                      )
        }
        if (host_olt_msgs_timeout) 
        {
            WRITE_SHORT_PARAMETER (system_parameters.host_olt_msgs_timeout       , Host OLT msgs timeout                 )
        }
        if (olt_reset_timeout) 
        {
            WRITE_SHORT_PARAMETER (system_parameters.olt_reset_timeout           , host-pon timeout                     )
        }
        if (automatic_authorization_policy) 
        {
            WRITE_ENABLE_DISABLE_PARAMETER(system_parameters.automatic_authorization_policy , Automatic authorization policy  )
        }
	}else
		CLI_ERROR_PRINT


    return(return_result);
}


#if 0	/* removed by xieshl 20110124, 问题单11987 */
/* added by chenfj 2007-10-23
    在PON命令节点下，增加通过FTP 升级多个ONU程序的命令
*/
DEFUN(ftpc_download_ftp_phenixos_master_func_for_pon_onus,
 ftpc_download_ftp_phenixos_master_for_pon_onus_cmd,
 "download ftp [os] <A.B.C.D> <user> <pass> <filename> <onuid_list>",
 /*"Download GW OS\n"*/
 "Download OS\n"
 "Download file using ftp protocol\n"
 "Download new GROS image\n"
 "Please input ftp server's IP address\n"
 "Please input user name\n" 
 "Please input the password\n" 
 "Please input the file name\n"
 OnuIDStringDesc
 )
{
	LONG lRet;
	CHAR pBuff[256];   
	short int PonPortIdx =0;
	short int OnuIdx;
	/*ULONG   ulIfIndex = 0;
	ULONG ulRet;*/
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length = 0;
	cliPayload *stPayload=NULL;
	int OnuType;
	int count=0;
	int	empty_count = 0;
	int	exist_count = 0;
	int	type_count = 0;
	int	error_count = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	length = 0;
	VOS_MemCpy( &pBuff[length],"download ftp ", 13);
	length = 13;
	sprintf( &pBuff[length],"%s ",argv[0]);
	length += VOS_StrLen(argv[0])+1;
	sprintf( &pBuff[length],"%s ",argv[1]);
	length += VOS_StrLen(argv[1])+1;
	sprintf( &pBuff[length],"%s ", argv[2] );
	length += VOS_StrLen(argv[2])+1;
	sprintf( &pBuff[length],"%s ", argv[3] );
	length += VOS_StrLen(argv[3])+1;
	sprintf( &pBuff[length],"%s ", argv[4] );
	length += VOS_StrLen(argv[4])+1;
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[5], ulOnuid )
	{
		count++;
	
		OnuIdx = (short int)(ulOnuid - 1);

		CHECK_ONU_RANGE

        if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) )
        {
            error_count++;
            empty_count++;

            continue;
        }
		
		 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
		lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
		if ( CLI_EPON_ONUUP != lRet)
		{
            error_count++;
            exist_count++;

            continue;
		}

		if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
		{
            error_count++;
            type_count++;

            continue;
		}
#if 0
		if(( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT811_A) && ( OnuType != V2R1_ONU_GT812_A)
			&&( OnuType != V2R1_ONU_GT812_B )&&( OnuType != V2R1_ONU_GT866 ) &&( OnuType != V2R1_ONU_GT865 ) &&( OnuType != V2R1_ONU_GT861 ) &&( OnuType != V2R1_ONU_GT815 ))
			{
			vty_out( vty, "  %% unknown command to onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuid );
			return( CMD_WARNING );
			}
#endif
		stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
		stPayload->fd = vty->fd;

		lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
		if(lRet != VOS_OK)
		{
            error_count++;
		}			
	}			
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == count )
    {
        if ( empty_count == count )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (exist_count == count)
                || ((exist_count + empty_count) == count) )
        {
    		vty_out( vty, "  %% onu %s off-line.\r\n", (1 == count) ? "is" : "are all");
        }
        else if ( (type_count == count)
                || ((type_count + empty_count + exist_count) == count) )
        {
    		vty_out( vty, "  %% Get onu type err.\r\n");
        }
        else
        {
    		vty_out( vty, "  %% ftp download to onu failed!\r\n");
        }

	   	return CMD_WARNING;
    }
	
    return CMD_SUCCESS;
}
#endif

/* added by chenfj 2007-12-19
	增加用于设置PON 下行优先级与队列之间映射*/
extern unsigned char PON_priority_queue_mapping[];
extern unsigned char PON_priority_queue_mapping_default[];
DEFUN( downlink_qos_priority_mapping_func,
        downlink_qos_priority_mapping_cmd,
        "onu downlink priority-queue-mapping <priority_list> queue [high|low]",
        "Config the onu\n"
        "onu downlink bandwidth\n"
        "onu downlink priority mapping; priority is mapping to specific queue, other priority-queue mapping is not changed\n"
        "onu downlink priority list;<priority_list> is mapping to the defined queue, other priority-queue mapping is not changed\n"
        "onu downlink priority queue;<priority_list> is mapping to the defined queue, other priority-queue mapping is not changed\n"
        "onu donwlink high priority queue\n"
        "onu downlink low priority queue\n"
        )
{
	ULONG priority;
	unsigned char PriorityQueue = V2R1_LOW;
	
	if( strcmp(argv[1],"high") == 0 )
		PriorityQueue = V2R1_HIGH;
	else /*if( strcmp(argv[1],"low") == 0 )*/
		PriorityQueue = V2R1_LOW;
	/*else {
		vty_out(vty, "Parameter err\r\n");
		return( CMD_WARNING );
		}*/
	
	BEGIN_PARSE_PRIORITY_LIST_TO_PRIORITY( argv[0], priority )
	{
		PON_priority_queue_mapping[priority] = PriorityQueue;
	}	
	END_PARSE_PRIORITY_LIST_TO_PRIORITY();

	SetOnuDownlinkQosMapping();
	vty_out(vty,"\r\nNote:this config is only valid to PAS5201\r\n");
	
	return CMD_SUCCESS;	
}

DEFUN( show_downlink_qos_priority_mapping_func,
      show_downlink_qos_priority_mapping_cmd,
      "show onu downlink priority-queue-mapping",
      "show onu info\n"
      "show onu info\n"
      "show onu downlink frame priority and queue mapping"
	"show onu downlink frame priority and queue mapping"
        )
{
	int priority;

	vty_out(vty,"\r\nonu downlink frame priority and queue mapping:\r\n");
	for(priority = 0; priority <= MAX_PRIORITY_QUEUE; priority++)
	{
		vty_out(vty,"priority %d -- queue %s\r\n",priority, PonQueuePriority_s[PON_priority_queue_mapping[priority]] );
	}	
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;	
}

/* 设置PON芯片支持的最大帧长度*/
extern unsigned int PON_Jumbo_frame_length_default;
extern unsigned int PON_Jumbo_frame_length ;

extern int SetPas5201frame_size_limits(short int PonPortIdx, unsigned int jumbo_length);
extern int GetPas5201frame_size_limits(short int PonPortIdx, unsigned int *jumbo_length);
/*  modified by chenfj 2008-5-23
	问题单6463
	PMC一直没有答复;既然不能支持1600字节,先将命令改为jumbo receive length <1518-1692>
	*/
DEFUN( set_pon_jumbo_frame_length,
           set_pon_jumbo_frame_length_cmd,
           "pon jumbo length <64-1596>",
           "set pon port\n"
           "set pon port jumbo frame length;note:jumbo frame setting is only supported by PAS5201 currently\n"
           "set pon port jumbo frame length;note:jumbo frame setting is only supported by PAS5201 currently\n"
           "input frame length,including FCS\n"
      )
{
	short int PonPortIdx/*, PonChipType*/;
    int iRlt;
    UINT32 jumbo_length;

	jumbo_length = ( UINT32 ) VOS_AtoL( argv[ 0 ] );   
	if((jumbo_length < PON_MIN_HW_ETHERNET_FRAME_SIZE )
		||(jumbo_length > PON_MAX_HW_ETHERNET_FRAME_SIZE ))
		return( RERROR );
    
	PON_Jumbo_frame_length = jumbo_length;
	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
	{
		if( OLTAdv_IsExist(PonPortIdx) != TRUE )
		{
			continue;
		}

#if 0        
		PonChipType = V2R1_GetPonchipType(PonPortIdx);
		if( PonChipType == PONCHIP_PAS5001)
		{			
			/*chipflag = 1;*/
			continue;
		}
#endif

	    iRlt = SetPas5201frame_size_limits(PonPortIdx, jumbo_length);
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            switch (iRlt)
            {
                case OLT_ERR_NOTSUPPORT:
        			vty_out(vty," pon%d/%d is not supported\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
                    break;
                default:    
        			vty_out(vty," pon%d/%d set err\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
            }
        }
	}

	return( CMD_SUCCESS );	
}

DEFUN( show_pon_jumbo_frame_length,
           show_pon_jumbo_frame_length_cmd,
           "show pon jumbo length",
           "show pon port info\n"
           "show pon port info\n"
           "show pon port jumbo frame length\n"
           "show pon port jumbo frame length\n"
      )
{
	short int PonPortIdx, PonChipType;

	unsigned int frame_size = 0;
	int ret = RERROR;

	for(PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx ++)
	{
		PonChipType = V2R1_GetPonchipType(PonPortIdx);

		if( PonChipType == PONCHIP_PAS5001)	continue;

		if( OLTAdv_IsExist(PonPortIdx) != TRUE )continue;

		ret = GetPas5201frame_size_limits(PonPortIdx, &frame_size );
		if( ret == ROK )
		{
			vty_out(vty,"\r\npon(%s) jumbo frame length:%d\r\n", OLTAdv_GetChipTypeName(PonPortIdx), frame_size );
			return( CMD_SUCCESS );
		}
	}

	vty_out(vty,"note:jumbo frame is not supported by PAS5001, and PAS5001 all in this chasis\r\n");

	return( CMD_WARNING );	
}

#ifdef  __CTC_TEST

#ifdef _ONU_UPLINK_VLAN_PRIORITY_
#endif
/* added by chenfj 2008-3-25
     用于在OLT PON芯片上设置ONU 上行数据优先级
     */
DEFUN(
	set_onu_vlan_priority,
	set_onu_vlan_priority_cmd,
	/*"onu-uplink-priority <onuid_list> vlan-id <1-4094> vlan-type [0x8100|0x9100|0x88a8] priority <0-7>",*/
	"onu uplink-priority <onuid_list> vlan-id <1-4094> priority <0-7>",
	"config onu\n" 
	"uplink priority\n"
	OnuIDStringDesc
	"vlan id\n"
	"specific vlan tag id 1 - 4094\n"
	/*
	"vlan type for the new tag\n"
	"vlan type 0x8100\n"
	"vlan type 0x9100\n"
	"vlan type 0x88a8\n"
	*/
	"priority for the new tag\n"
	"new priority\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx, OnuLlid;
	short int PonChipType;
	unsigned char OnuCounter=0;
	int	empty_count = 0;
	int	offline_count = 0;
	int	error_count = 0;

	/*unsigned long ulIfIndex;*/
	long lRet;
	short int return_result;
	
	PON_olt_vlan_uplink_config_t    vlan_uplink_config ;
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_vlan_qinq_t vlan_qinq_config;

	VOS_MemSet(&vlan_uplink_config, 0, sizeof(PON_olt_vlan_uplink_config_t));
	VOS_MemSet(&vlan_qinq_config, 0, sizeof(vlan_qinq_config));

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty,"\r\n this is PAS5001, cli is not supported\r\n");
		return( CMD_WARNING );
		}

	vlan_uplink_config.untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	vlan_uplink_config.authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	vlan_uplink_config.discard_untagged = FALSE;
	vlan_uplink_config.discard_tagged = FALSE;
	vlan_uplink_config.discard_null_tagged = FALSE;
	vlan_uplink_config.discard_nested = FALSE;
	vlan_uplink_config.vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED;
	vlan_uplink_config.new_vlan_tag_id = (PON_vlan_tag_t)VOS_AtoL( argv[1] );
	/*
	if(VOS_MemCmp(argv[2],"0x8100",6) == 0)
		vlan_uplink_config.vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	else if(VOS_MemCmp(argv[2],"0x9100",6) == 0)
		vlan_uplink_config.vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_9100;
	else if(VOS_MemCmp(argv[2],"0x88a8",6) == 0)
		vlan_uplink_config.vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_88A8;
	else return( CMD_WARNING );
	*/
	vlan_uplink_config.vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	/*
	if(VOS_MemCmp(argv[3],"original",8) == 0)
		vlan_uplink_config.vlan_priority = PON_VLAN_ORIGINAL_PRIORITY ;
	else*/
	vlan_uplink_config.vlan_priority = VOS_AtoL( argv[2] );

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
		OnuCounter++;
        
		OnuIdx = ulOnuId-1;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			SetOnuUplinkVlanPriority(PonPortIdx, OnuIdx, V2R1_UP, vlan_uplink_config);
			if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
			{
                error_count++;
			    offline_count++
			}
			else
            {
				OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( OnuLlid != INVALID_LLID )
				{
				    #if 1
					vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
					vlan_qinq_config.qinq_objectid = OnuIdx;
					vlan_qinq_config.qinq_cfg.up_cfg = vlan_uplink_config;
					return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
					#else
					return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
					#endif/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					if( return_result != PAS_EXIT_OK ) 
                    {
                        error_count++;
                    }               
				}
                else
                {
                    error_count++;
                }
			}
		}
		else
        {
            error_count++;
            empty_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == OnuCounter )
    {
        if ( empty_count == OnuCounter )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (offline_count == OnuCounter)
                || ((offline_count + empty_count) == OnuCounter) )
        {
    		vty_out( vty, "  %% onu %s off-line.\r\n", (1==OnuCounter) ? "is" : "are all");
        }
        else
        {
    		vty_out( vty, "  %% set onu uplink-priority err\r\n");
        }

	   	return CMD_WARNING;
    }

	return (CMD_SUCCESS);
}

DEFUN(
	undo_set_onu_vlan_priority,
	undo_set_onu_vlan_priority_cmd,
	"undo onu uplink-priority <onuid_list>",
	"undo onu config\n"
	"undo uplink priority\n"
	"uplink priority\n"
	OnuIDStringDesc
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx, OnuLlid;
	short int PonChipType;
	unsigned char OnuCounter=0;
	int	empty_count = 0;
	int	offline_count = 0;
	int	error_count = 0;

	/*unsigned long ulIfIndex;*/
	long lRet;
	short int return_result;
	
	PON_olt_vlan_uplink_config_t    vlan_uplink_config ;
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_vlan_qinq_t vlan_qinq_config;

	VOS_MemSet(&vlan_uplink_config, 0, sizeof(PON_olt_vlan_uplink_config_t));
	VOS_MemSet(&vlan_qinq_config, 0, sizeof(vlan_qinq_config));

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty,"\r\n this is PAS5001, cli is not supported\r\n");
		return( CMD_WARNING );
		}

	vlan_uplink_config.untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	vlan_uplink_config.authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	vlan_uplink_config.discard_untagged = FALSE;
	vlan_uplink_config.discard_tagged = FALSE;
	vlan_uplink_config.discard_null_tagged = FALSE;
	vlan_uplink_config.discard_nested = FALSE;
	vlan_uplink_config.vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
	vlan_uplink_config.vlan_priority = PON_VLAN_ORIGINAL_PRIORITY;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
		OnuCounter++;
		OnuIdx = ulOnuId-1;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		{
			SetOnuUplinkVlanPriority(PonPortIdx, OnuIdx, V2R1_DOWN, vlan_uplink_config);
			if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
			{
                error_count++;
			    offline_count++
			}
			else
            {
				OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( OnuLlid != INVALID_LLID )
				{
				    #if 1
					vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
					vlan_qinq_config.qinq_objectid = OnuIdx;
					vlan_qinq_config.qinq_cfg.up_cfg = vlan_uplink_config;
					return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
					#else
					return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
					#endif/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					if( return_result != PAS_EXIT_OK ) 
                    {
                        error_count++;
                    }               
				}
                else
                {
                    error_count++;
                }
			}
		}
		else
        {
            error_count++;
            empty_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == OnuCounter )
    {
        if ( empty_count == OnuCounter )
        {
    		vty_out( vty, "  %% onu not exist.\r\n");
        }
        else if ( (offline_count == OnuCounter)
                || ((offline_count + empty_count) == OnuCounter) )
        {
    		vty_out( vty, "  %% onu %s off-line.\r\n", (1==OnuCounter) ? "is" : "are all");
        }
        else
        {
    		vty_out( vty, "  %% undo onu uplink-priority err\r\n");
        }

	   	return CMD_WARNING;
    }

	return (CMD_SUCCESS);
}
#endif

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
/* added by chenfj 2008-7-9
     用于在OLT PON芯片上对ONU 上行数据增加VLAN TAG
     */
DEFUN(
	set_onu_uplink_vlan_tpid,
	set_onu_uplink_vlan_tpid_cmd,
	"pon-vlantpid user-defined [<0x0001-0xffff>|<1-65535>]",
	"config pon vlan tpid\n" 
	"pon vlan tpid user defined\n"
	"input the hex number of vlan tpid\n"
	"input the decimal number of vlan tpid\n"
	)
{
    unsigned long ulSlot, ulPort, ulOnuId;
    unsigned long vlan_tpid;
    short int PonPortIdx;
    /*short int PonChipType;*/
    char *pToken;
    int iRlt;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

#if 0
    /* B--modified by liwei056@2010-5-25 for D10157 */
    PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, vlan-tpid is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	if((VOS_StrnCmp(argv[0],"0x", 2) == 0 ) || (VOS_StrnCmp(argv[0],"0X", 2) == 0 ))
		vlan_tpid = VOS_StrToUL( argv[0], &pToken, 16 );
	else 
		vlan_tpid = VOS_AtoL( argv[0] );
	if((vlan_tpid < 1) || ( vlan_tpid > 65535))
	{
		vty_out(vty, " vlan-tpid is out of range, should be 1-65535\r\n");
		return( CMD_WARNING);
	}

	if((iRlt = SetPonPortVlanTpid(PonPortIdx,(unsigned short int)vlan_tpid)) != ROK)
    {
        if ( OLT_ERR_NOTSUPPORT == iRlt )
        {
            vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
        }
        else
        {
    		vty_out(vty,"config pon vlan-tpid err\r\n");
        }
 		return( CMD_WARNING);
    }   

	return(CMD_SUCCESS);
}

DEFUN(
	show_onu_uplink_vlan_tpid,
	show_onu_uplink_vlan_tpid_cmd,
	"show pon-vlantpid user-defined",
	"show pon vlan tpid\n" 
	"show pon vlan tpid user defined\n"
	"show pon vlan tpid user defined\n"
	)
{
    int result;
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	unsigned short vlan_tpid;

 	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	if((result = GetPonPortVlanTpid(PonPortIdx)) != RERROR)
    {
        vlan_tpid = (unsigned short)result;
		vty_out(vty,"user defined vlan tpid:0x%04x\r\n", vlan_tpid);
    }   
	
	return(CMD_SUCCESS);

}

DEFUN(
	config_set_onu_uplink_vlan_tpid,
	config_set_onu_uplink_vlan_tpid_cmd,
	"pon-vlantpid user-defined <portlist> [<0x0001-0xffff>|<1-65535>]",
	"config pon vlan tpid\n" 
	"pon vlan tpid user defined\n"
	"specify pon port list(e.g. 3/2 or 3/1-2)\n"
	"input the hex number of vlan tpid\n"
	"input the decimal number of vlan tpid\n"
	)
{
	unsigned long ulSlot, ulPort;
	short int PonPortIdx;
	/*short int PonChipType;*/
	unsigned long ulIfIndex;
    unsigned long vlan_tpid;
	char *pToken;
    int iRlt, iRltOkNum;

	if((VOS_StrnCmp(argv[1],"0x", 2) == 0 ) || (VOS_StrnCmp(argv[1],"0X", 2) == 0 ))
		vlan_tpid = VOS_StrToUL( argv[1], &pToken, 16 );
	else 
		vlan_tpid = VOS_AtoL( argv[1] );
	if((vlan_tpid < 1) || ( vlan_tpid > 65535))
	{
		vty_out(vty, " pon vlan tpid is out of range, should be 1-65535\r\n");
		return( CMD_WARNING);
	}

    iRltOkNum = 0;
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( (ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK) ||
			(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK) ||
			(RERROR == (PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort ))) )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			

#if 0
        /* B--modified by liwei056@2010-5-25 for D10157 */
        PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
        /* E--modified by liwei056@2010-5-25 for D10157 */
		{
			vty_out(vty,"\r\n pon%d/%d this is not PAS5201, pon vlan-tpid is not supported\r\n", ulSlot, ulPort);
			return( CMD_WARNING );
		}
#endif

    	if((iRlt = SetPonPortVlanTpid(PonPortIdx,(unsigned short int)vlan_tpid)) != ROK)
        {
            if ( OLT_ERR_NOTSUPPORT == iRlt )
            {
                vty_out( vty, "  %% pon%d/%d(%s) not support this command\r\n", ulSlot, ulPort, OLTAdv_GetChipTypeName(PonPortIdx));
            }
            else
            {
        		vty_out(vty,"  %% pon%d/%d config pon vlan-tpid err\r\n", ulSlot, ulPort);
            }
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
        }  
        else
        {
            iRltOkNum++;
        }
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()


	return (0 < iRltOkNum) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(
	config_show_onu_uplink_vlan_tpid,
	config_show_onu_uplink_vlan_tpid_cmd,
	"show pon-vlantpid user-defined <portlist>",
	"show pon vlan tpid\n" 
	"show pon vlan tpid user defined\n"
	"show pon vlan tpid user defined\n"
	"specify pon port list(e.g. 3/2 or 3/1-2)\n"
	)
{
	unsigned long ulSlot, ulPort;
	unsigned long ulIfIndex;
    int result;
	short int PonPortIdx;
	/*short int PonChipType;*/
	unsigned short vlan_tpid;
	
	vty_out(vty,"user defined vlan tpid\r\n");

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( (ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK) ||
			(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK) ||
			(RERROR == (PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort))) )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			

#if 0
        /* B--modified by liwei056@2010-5-25 for D10157 */
        PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
        /* E--modified by liwei056@2010-5-25 for D10157 */
		{
			vty_out(vty,"\r\n pon%d/%d this is not PAS5201, pon vlan-tpid is not supported\r\n", ulSlot, ulPort);
			return( CMD_WARNING );
		}
#endif

		result = GetPonPortVlanTpid(PonPortIdx);
		if(result != RERROR)
        {
            vlan_tpid = (unsigned short)result;
			vty_out(vty,"pon%d/%d	--  tpid 0x%04x\r\n", ulSlot, ulPort, vlan_tpid);
        }      
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		
	return(CMD_SUCCESS);
}

/*  ONU 上行数据增加VLAN 标签*/
DEFUN(
	set_onu_vlan_add_tag,
	set_onu_vlan_add_tag_cmd,
	"uplink vlan-tag-add <onuid_list> inter-vid [all|<1-4094>] outer-vid <1-4094> priority [<0-7>|original] tpid [0x8100|0x9100|0x88a8|user-defined]",
	"config pon\n" 
	"uplink add tag\n"
	OnuIDStringDesc
	"inter vlan id\n"
	"all frames\n"
	/*"untagged frames\n"*/
	"specific vlan tag id 1 - 4094\n"
	"outer vlan id\n"
	"specific vlan tag id 1 - 4094\n"
	"priority for the new tag\n"
	"new priority\n"
	"original priority\n"
	"tpid\n"
	"vlan tpid is 0x8100\n"
	"vlan tpid is 0x9100\n"
	"vlan tpid is 0x88a8\n"
	"vlan tpid is user defined\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx/*, OnuLlid*/;
	/*short int PonChipType;
	unsigned char OnuCounter=1;*/
	int count=0;
	int	error_count = 0;
	int return_result;
	OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t *vlan_uplink_config;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER != EPON_MODULE_YES)
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
    {
        count++;
    }   
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( 1 == count )
    {
		OnuIdx = ulOnuId - 1;
        if ( ONU_IDX_ISVALID(OnuIdx) )
        {
            /* B--added by liwei056@2012-5-22 for D10708 */    
            GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);
            if ( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
            {
        		vty_out(vty,"  %% onu%d/%d/%d have uplink qinq config, please undo it at first.\r\n", ulSlot, ulPort, ulOnuId);
        		return( CMD_WARNING );
            }
            /* E--added by liwei056@2012-5-22 for D10708 */    
        }
    }
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_uplink_config = &(vlan_qinq_config.qinq_cfg.up_cfg);
    
	vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	if( VOS_MemCmp(argv[1],"all", 3 ) == 0 )
		vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	else if( VOS_MemCmp(argv[1],"untag", 5 ) == 0 )
		vlan_uplink_config->authenticated_vid = VLAN_UNTAGGED_ID;
	else 
		vlan_uplink_config->authenticated_vid = (PON_vlan_tag_t)VOS_AtoL( argv[1] );
	vlan_uplink_config->discard_untagged = FALSE;
	vlan_uplink_config->discard_tagged = FALSE;
	vlan_uplink_config->discard_null_tagged = FALSE;
	vlan_uplink_config->discard_nested = FALSE;
	vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED;
	vlan_uplink_config->new_vlan_tag_id = (PON_vlan_tag_t)VOS_AtoL( argv[2] );

	/*vlan_uplink_config.vlan_type = VOS_AtoL( argv[4] ); PON_OLT_VLAN_ETHERNET_TYPE_8100;*/
	if(VOS_StrCmp(argv[3], "original") == 0 )
		vlan_uplink_config->vlan_priority = PON_VLAN_ORIGINAL_PRIORITY;
	else
		vlan_uplink_config->vlan_priority = VOS_AtoL( argv[3] );

	if( VOS_StrCmp(argv[4], "0x8100") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	else if( VOS_StrCmp(argv[4], "0x9100") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_9100;
	else if( VOS_StrCmp(argv[4], "0x88a8") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_88A8;
	else if( VOS_StrCmp(argv[4], "user-defined") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE;
	else
		return CMD_WARNING;

#if 1    
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER == EPON_MODULE_YES)
        count++;
#endif

		OnuIdx = ulOnuId - 1;
        vlan_qinq_config.qinq_objectid = OnuIdx;

        return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
        if ( OLT_CALL_ISERROR(return_result) )
        {
            error_count++;
            switch (return_result)
            {
                case OLT_ERR_NOTSUPPORT:
                    vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                    break;
                case OLT_ERR_NORESC:
                    vty_out(vty,"\r\n Failed to set onu%d/%d/%d uplink add tag for uplink vlan-add table full\r\n", ulSlot, ulPort, ulOnuId);
                    break;
                default:
            		vty_out(vty,"\r\n set onu %d/%d/%d uplink add tag err(code=%d)\r\n",ulSlot, ulPort, ulOnuId,return_result);
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
	   	return CMD_WARNING;
    }
#else    
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
	{
		if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
			SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			if(OnuCounter == 1 )
				vty_out(vty,"\r\n  onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);
		}
		else
        {
			OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if( OnuLlid != INVALID_LLID )
			{
				return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
				if( return_result != PAS_EXIT_OK ) 
					vty_out(vty,"\r\n set onu %d/%d/%d uplink add tag err(code=%d)\r\n",ulSlot, ulPort, ulOnuId,return_result);
				else 
					SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			}
		}
	}
	else
    {
		SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
		vty_out(vty,"\r\n onu%d/%d/%d not exist\r\n",ulSlot, ulPort, ulOnuId);
	}
#endif

	return (CMD_SUCCESS);
}

DEFUN(
	uplink_set_pon_vlan_trans_tag,
	uplink_set_pon_vlan_trans_tag_cmd,
	"uplink vlan-tag-exchange <onuid_list> [all|null-tagged|<1-4094>] <1-4094> [original|<0-7>] [0x8100|0x9100|0x88a8|user-defined]",
	"pon uplink\n"
	"pon uplink vlan exchange\n"
	OnuIDStringDesc
	"all tagged-frames\n"
	"null-tagged frames\n"
	"original vlan-id\n"
	"new vlan id\n"
	"original VLAN priority is copied\n"
	"new priority\n"
	"vlan tpid is 0x8100\n"
	"vlan tpid is 0x9100\n"
	"vlan tpid is 0x88a8\n"
	"vlan tpid is user defined\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx/*, OnuLlid*/;
	/*short int PonChipType;
	unsigned char OnuCounter=1;*/
	short int return_result;
	int count=0;
	int	error_count = 0;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t *vlan_uplink_config ;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0 
#if 0 
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER != EPON_MODULE_YES)
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
    {
        count++;
    }   
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( 1 == count )
    {
		OnuIdx = ulOnuId - 1;
        if ( ONU_IDX_ISVALID(OnuIdx) )
        {
            /* B--added by liwei056@2012-5-22 for D10708 */    
            GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);
            if ( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
            {
        		vty_out(vty,"  %% onu%d/%d/%d have uplink qinq config, please undo it at first.\r\n", ulSlot, ulPort, ulOnuId);
        		return( CMD_WARNING );
            }
            /* E--added by liwei056@2012-5-22 for D10708 */    
        }
    }
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_uplink_config = &(vlan_qinq_config.qinq_cfg.up_cfg);

	vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	if( VOS_MemCmp(argv[1],"null-tagged", 11 ) == 0 )
		vlan_uplink_config->authenticated_vid = NULL_VLAN;
	else if( VOS_MemCmp(argv[1],"untagged", 8 ) == 0 )
		vlan_uplink_config->authenticated_vid = VLAN_UNTAGGED_ID;
	else if( VOS_MemCmp(argv[1],"all", 3 ) == 0 )
		vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	else 
		vlan_uplink_config->authenticated_vid = (PON_vlan_tag_t)VOS_AtoL( argv[1] );

	vlan_uplink_config->discard_untagged = FALSE;
	vlan_uplink_config->discard_tagged = FALSE;
	vlan_uplink_config->discard_null_tagged = FALSE;
	vlan_uplink_config->discard_nested = FALSE;
	vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED;
	vlan_uplink_config->new_vlan_tag_id = (PON_vlan_tag_t)VOS_AtoL( argv[2] );

    if (vlan_uplink_config->authenticated_vid == vlan_uplink_config->new_vlan_tag_id)
    {
		vty_out(vty,"\r\n param is error, the two vlan-id is equal.\r\n");
		return( CMD_WARNING );
    }

	if( VOS_MemCmp(argv[3],"original", 8) == 0)
		vlan_uplink_config->vlan_priority = PON_VLAN_ORIGINAL_PRIORITY;
	else 
		vlan_uplink_config->vlan_priority = VOS_AtoL( argv[3] );

	if( VOS_StrCmp(argv[4], "0x8100") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	else if( VOS_StrCmp(argv[4], "0x9100") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_9100;
	else if( VOS_StrCmp(argv[4], "0x88a8") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_88A8;
	else if( VOS_StrCmp(argv[4], "user-defined") == 0)
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE;
	else
		return CMD_WARNING;

#if 1
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER == EPON_MODULE_YES)
        count++;
#endif

		OnuIdx = ulOnuId - 1;
        vlan_qinq_config.qinq_objectid = OnuIdx;

        return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
        if ( OLT_CALL_ISERROR(return_result) )
        {
            error_count++;
            switch (return_result)
            {
                case OLT_ERR_NOTSUPPORT:
                    vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                    break;
                case OLT_ERR_NORESC:
                    vty_out(vty,"\r\n Failed to set onu%d/%d/%d uplink add tag for uplink vlan-add table full\r\n", ulSlot, ulPort, ulOnuId);
                    break;
                default:
            		vty_out(vty,"\r\n set onu %d/%d/%d uplink add tag err(code=%d)\r\n",ulSlot, ulPort, ulOnuId,return_result);
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
	   	return CMD_WARNING;
    }
#else    
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
	{
		if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
			SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			if(OnuCounter == 1 )
				vty_out(vty,"\r\n  onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);
		}
		else
        {
			OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if( OnuLlid != INVALID_LLID )
			{
				return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
				if( return_result != PAS_EXIT_OK ) 
				{
					if( OnuCounter == 1 )
    					vty_out(vty,"\r\n set onu %d/%d/%d uplink vlan-translation err(code=%d)\r\n",ulSlot, ulPort, ulOnuId, return_result);
				}
				else
					SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			}
		}
	}
	else
    {
		SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
		vty_out(vty,"\r\n onu%d/%d/%d not exist\r\n",ulSlot, ulPort, ulOnuId);
	}
#endif

	return (CMD_SUCCESS);
}

DEFUN(
	uplink_set_pon_vlan_filter_tag,
	uplink_set_pon_vlan_filter_tag_cmd,
	"uplink vlan-tag-filter <onuid_list> [all|<1-4094>] {[0x8100|0x9100|0x88a8|user-defined]}*1",
	"pon uplink\n"
	"pon uplink vlan filter\n"
	OnuIDStringDesc
	"all frames\n"
	"specific vlan-id\n"
	"vlan tpid is 0x8100\n"
	"vlan tpid is 0x9100\n"
	"vlan tpid is 0x88a8\n"
	"vlan tpid is user defined\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx/*, OnuLlid*/;
	/*short int PonChipType;
	unsigned char OnuCounter=1;*/
	short int return_result;
	int count=0;
	int	error_count = 0;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t *vlan_uplink_config ;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER != EPON_MODULE_YES)
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
    {
        count++;
    }   
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( 1 == count )
    {
		OnuIdx = ulOnuId - 1;
        if ( ONU_IDX_ISVALID(OnuIdx) )
        {
            /* B--added by liwei056@2012-5-22 for D10708 */    
            GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);
            if ( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
            {
        		vty_out(vty,"  %% onu%d/%d/%d have uplink qinq config, please undo it at first.\r\n", ulSlot, ulPort, ulOnuId);
        		return( CMD_WARNING );
            }
            /* E--added by liwei056@2012-5-22 for D10708 */    
        }
    }
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_uplink_config = &(vlan_qinq_config.qinq_cfg.up_cfg);

	vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	if( VOS_MemCmp(argv[1],"all", 3 ) == 0 )
		vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	else if( VOS_MemCmp(argv[1],"untagged", 8 ) == 0 )
		vlan_uplink_config->authenticated_vid = VLAN_UNTAGGED_ID;
	else 
		vlan_uplink_config->authenticated_vid = (PON_vlan_tag_t)VOS_AtoL( argv[1] );
	
	vlan_uplink_config->discard_untagged = FALSE;
	vlan_uplink_config->discard_tagged = FALSE;
	vlan_uplink_config->discard_null_tagged = FALSE;
	vlan_uplink_config->discard_nested = FALSE;
	vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED;
	vlan_uplink_config->new_vlan_tag_id = 0;
	vlan_uplink_config->vlan_priority = 0;

    if ( argc > 2 )
    {
    	if( VOS_StrCmp(argv[2], "0x8100") == 0)
    		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
    	else if( VOS_StrCmp(argv[2], "0x9100") == 0)
    		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_9100;
    	else if( VOS_StrCmp(argv[2], "0x88a8") == 0)
    		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_88A8;
    	else if( VOS_StrCmp(argv[2], "user-defined") == 0)
    		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE;
    	else
    		return CMD_WARNING;
    }
    /* B--added by liwei056@2012-3-19 for D14753 */
    else
    {
		vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
    }
    /* E--added by liwei056@2012-3-19 for D14753 */

#if 1
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER == EPON_MODULE_YES)
        count++;
#endif

		OnuIdx = ulOnuId - 1;
        vlan_qinq_config.qinq_objectid = OnuIdx;
        return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
        if ( OLT_CALL_ISERROR(return_result) )
        {
            error_count++;
            switch (return_result)
            {
                case OLT_ERR_NOTSUPPORT:
                    vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                    break;
                case OLT_ERR_NORESC:
                    vty_out(vty,"\r\n Failed to set onu%d/%d/%d uplink add tag for uplink vlan-add table full\r\n", ulSlot, ulPort, ulOnuId);
                    break;
                default:
            		vty_out(vty,"\r\n set onu %d/%d/%d uplink add tag err(code=%d)\r\n",ulSlot, ulPort, ulOnuId,return_result);
            }
        }
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
	   	return CMD_WARNING;
    }
#else    
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
	{
		if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
			SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			if(OnuCounter == 1 )
				vty_out(vty,"\r\n  onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);
		}
		else
        {
			OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if( OnuLlid != INVALID_LLID )
			{
				return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
				if( return_result != PAS_EXIT_OK ) 
				{
					if( OnuCounter == 1 )
					vty_out(vty,"\r\n set onu %d/%d/%d uplink vlan-filter err(code=%d)\r\n",ulSlot, ulPort, ulOnuId, return_result);
				}
				else
					SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			}
		}
	}
	else
    {
		SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
		vty_out(vty,"\r\n onu%d/%d/%d not exist\r\n",ulSlot, ulPort, ulOnuId);
	}
#endif

	return (CMD_SUCCESS);
}

DEFUN(
	undo_uplink_set_pon_vlan_trans_tag,
	undo_uplink_set_pon_vlan_trans_tag_cmd,
	"undo uplink vlan-tag-manipulation <onuid_list>",  /*{[any-tagged|<1-4094>]}*1*/
	"undo pon config\n" 
	"undo pon uplink\n"
	"undo pon uplink vlan manipulation\n"
	OnuIDStringDesc
	/*
	"all tagged-frames\n"
	"uplink original vlan-id\n"*/
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	short int OnuIdx/*, OnuLlid*/;
	/*short int PonChipType;*/
	int count=0;
	int	empty_count = 0;
	int	error_count = 0;
	short int return_result;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vlan_uplink_config_t *vlan_uplink_config ;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_uplink_config = &(vlan_qinq_config.qinq_cfg.up_cfg);
	vlan_uplink_config->untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
	/*
	if( VOS_MemCmp(argv[1],"any-tagged", 10 ) == 0 )
		vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	else 
		vlan_uplink_config->authenticated_vid = (PON_vlan_tag_t)VOS_AtoL( argv[1] );*/
	vlan_uplink_config->authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
	vlan_uplink_config->discard_untagged = FALSE;
	vlan_uplink_config->discard_tagged = FALSE;
	vlan_uplink_config->discard_null_tagged = FALSE;
	vlan_uplink_config->discard_nested = FALSE;
	vlan_uplink_config->vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
	vlan_uplink_config->new_vlan_tag_id = 1;

	vlan_uplink_config->vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;
	vlan_uplink_config->vlan_priority = PON_VLAN_ORIGINAL_PRIORITY;

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
	{
	    count++;
		OnuIdx = ulOnuId - 1;


        /* B--added by liwei056@2012-5-22 for D10708 */    
#if 1
        GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);
        if ( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
#else
 		if(ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
#endif
        /* E--added by liwei056@2012-5-22 for D10708 */    
		{
#if 1
            vlan_qinq_config.qinq_objectid  = OnuIdx;
            vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
            return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
            if ( OLT_CALL_ISERROR(return_result) )
            {
                error_count++;
                switch (return_result)
                {
                    case OLT_ERR_NOTSUPPORT:
                        vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                    	RETURN_PARSE_ONUID_LIST_TO_ONUID (CMD_WARNING);
                }
            }
#else    
			SavePonUplinkVlanManipulation(PonPortIdx, OnuIdx, &vlan_uplink_config);
			/*SetOnuUplinkVlanPriority(PonPortIdx, OnuIdx, V2R1_UP, vlan_uplink_config);*/
			if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
				{
				/*if(OnuCounter == 1 )
					vty_out(vty,"\r\n  onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);*/
				}
			else{
				OnuLlid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
				if( OnuLlid != INVALID_LLID )
					{
					return_result = PAS_set_llid_vlan_mode(PonPortIdx, OnuLlid, vlan_uplink_config);
					if( return_result != PAS_EXIT_OK ) 
						if( OnuCounter == 1 )
						vty_out(vty,"\r\n undo onu %d/%d/%d uplink vlan-translation err\r\n",ulSlot, ulPort, ulOnuId);
					}
				}
#endif
		}
		else
        {
            error_count++;
            empty_count++;
		}
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
    
    if ( error_count == count )
    {
        if ( empty_count == count )
        {
#if 1
    		vty_out( vty, "  %% uplink qinq config not exist.\r\n");
#else
    		vty_out( vty, "  %% onu not exist.\r\n");
#endif
        }
        else
        {
    		vty_out(vty,  "  %% undo onu uplink vlan-manipulation err\r\n");
        }

	   	return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

/* ONU 下行数据剥离vlan 标签*/
DEFUN(
	set_onu_vlan_strip_tag,
	set_onu_vlan_strip_tag_cmd,
	"downlink vlan-tag-striped <1-4094>",
	"config onu\n" 
	"downlink striped vlan tag\n"
	"vlan id will be striped,range:1-4094\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	/*short int PonChipType;*/
	int return_result =PAS_EXIT_ERROR;
	PON_vlan_tag_t  original_vlan;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vid_downlink_config_t   *vlan_downlink_config ;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	original_vlan = VOS_AtoL( argv[0] );

#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER != EPON_MODULE_YES)
    /* B--added by liwei056@2012-5-22 for D10708 */    
    GetPonDownlinkVlanQinQ(PonPortIdx, original_vlan, &vlan_qinq_policy);
    if ( PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
    {
		vty_out(vty,"  %% pon%d/%d have vlan%d's downlink qinq config, please undo it at first.\r\n", ulSlot, ulPort, original_vlan);
		return( CMD_WARNING );
    }
    /* E--added by liwei056@2012-5-22 for D10708 */    
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_downlink_config = &(vlan_qinq_config.qinq_cfg.down_cfg);

	vlan_downlink_config->discard_nested = FALSE;
	vlan_downlink_config->destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
	vlan_downlink_config->llid = 0;
	vlan_downlink_config->vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG;
	vlan_downlink_config->new_vid = 0;
	vlan_downlink_config->new_priority = 0;

#if 1
    vlan_qinq_config.qinq_objectid  = original_vlan;
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_DOWNLINK;
    return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
    if ( OLT_CALL_ISERROR(return_result) )
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                break;
            case OLT_ERR_NORESC:
                vty_out(vty,"\r\n Failedto set pon%d/%d for downlink vlan strip table full\r\n", ulSlot, ulPort);
                break;
            default:
				vty_out(vty,"\r\n set pon%d/%d downlink vlan strip err:%d\r\n",ulSlot, ulPort, return_result);
        }
        
    	return (CMD_WARNING);
    }
#else
	if(SavePonDownlinkVlanManipulation(PonPortIdx, original_vlan, &vlan_downlink_config) == ROK)
	{
		if(Olt_exists(PonPortIdx))
		{
			return_result = PAS_set_vid_downlink_mode(PonPortIdx, original_vlan, vlan_downlink_config);
			if( return_result != PAS_EXIT_OK)
            {
				vty_out(vty,"\r\n set pon%d/%d downlink vlan strip err:%d\r\n",ulSlot, ulPort, return_result);
            	return (CMD_WARNING);
            }         
		}
	}
	else
    {
        vty_out(vty,"\r\n set pon%d/%d downlink vlan strip table full\r\n",ulSlot, ulPort);
    	return (CMD_WARNING);
    }   
#endif
	
	return (CMD_SUCCESS);
}

#if 0	/* 问题单11611 */
/*add by shixh20101207 for ctc onu test*/
extern long CTC_StackSupported ;
DEFUN(
	onu_set_ctc_enable_state,
	onu_set_ctc_enable_state_cmd,
	"ctc-onu [enable|disable]",
	"config ctc onu enable state\n"
	"enable\n"
	"disable\n")
{
	unsigned long Slot, Port;
        short int PonPortIdx;
	unsigned long state;


	if( VOS_StrCmp( argv[ 0 ] , "enable" )  == 0 )
		state = 1;
	else
		state = 0;
    
        for(Slot=1;Slot<=21/*SYS_MAX_PON_CARDNUM*/;Slot++)
        {
            if(SlotCardIsPonBoard(Slot)!=ROK)
                continue;
            
                for(Port=1;Port<=4;Port++)
                    {
                        PonPortIdx=GetPonPortIdxBySlot(Slot,Port);
                        
                    	if(OLT_SetCtcOnuEnable(PonPortIdx, state) == ROK)
                    		return CMD_SUCCESS;
                    	else
                    		return CMD_WARNING;
                    }
        }
        return CMD_WARNING;

}

DEFUN(
	onu_set_ctc_enable_show,
	onu_set_ctc_enable_show_cmd,
	"show ctc-onu enable",
	"show information\n"
	"config ctc onu enable state\n"
	"enable\n")
{

    if(CTC_StackSupported==1)
            vty_out(vty,"support ctc-onu!\r\n");
     else
            vty_out(vty,"not support ctc-onu!\r\n");
	
	return CMD_SUCCESS;
}
#endif

/* added by chenfj 2008-7-15
     用于在OLT PON芯片上, ONU 下行数据转换vlan 标签
     */
DEFUN(
	downlink_set_pon_vlan_trans_tag,
	downlink_set_pon_vlan_trans_tag_cmd,
	"downlink vlan-tag-exchange <1-4094> <1-4094> [original|<0-7>]",
	"pon downlink\n"
	"pon downlink vlan exchange\n"
	/*"null-taged frames\n" PMC note: this option is useless */
	/*"un-taged frames\n"  PMC note: this option is not supported*/
	"original vlan-id\n"
	"new vlan id\n"
	"original VLAN priority is copied\n"
	"new priority\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	/*short int PonChipType;*/
	short int return_result =PAS_EXIT_ERROR;
	PON_vlan_tag_t  original_vlan;
	PON_vlan_tag_t  dest_vlan;
    OLT_vlan_qinq_t vlan_qinq_config;
	PON_olt_vid_downlink_config_t   *vlan_downlink_config ;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	if( VOS_MemCmp(argv[0],"null-tag", 8 ) == 0 )
		original_vlan = NULL_VLAN;  /* = 0 */
	else if(VOS_MemCmp(argv[0],"untag", 5 ) == 0 )
		original_vlan = VLAN_UNTAGGED_ID;  /* = 4095 */
	else original_vlan = VOS_AtoL( argv[0] );

    dest_vlan = VOS_AtoL( argv[1] );
    if (original_vlan == dest_vlan)
    {
		vty_out(vty,"\r\n param is error, the two vlan-id is equal.\r\n");
		return( CMD_WARNING );
    }

#if (EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER != EPON_MODULE_YES)
    /* B--added by liwei056@2012-5-22 for D10708 */    
    GetPonDownlinkVlanQinQ(PonPortIdx, original_vlan, &vlan_qinq_policy);
    if ( PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION != vlan_qinq_policy.vlan_manipulation )
    {
		vty_out(vty,"  %% pon%d/%d have vlan%d's downlink qinq config, please undo it at first.\r\n", ulSlot, ulPort, original_vlan);
		return( CMD_WARNING );
    }
    /* E--added by liwei056@2012-5-22 for D10708 */    
#endif

	VOS_MemSet(&vlan_qinq_config, 0, sizeof(OLT_vlan_qinq_t));
    vlan_downlink_config = &(vlan_qinq_config.qinq_cfg.down_cfg);

	vlan_downlink_config->discard_nested = FALSE;
	vlan_downlink_config->destination = PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE;
	vlan_downlink_config->llid = 0;
	vlan_downlink_config->new_vid = dest_vlan; 
	if( VOS_MemCmp(argv[2],"original", 8) == 0)
	{
		vlan_downlink_config->new_priority = PON_VLAN_ORIGINAL_PRIORITY;
		vlan_downlink_config->vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID;
	}
	else
    { 
		vlan_downlink_config->new_priority = VOS_AtoL( argv[2] );
		vlan_downlink_config->vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY;
	}

#if 1
    vlan_qinq_config.qinq_objectid  = original_vlan;
    vlan_qinq_config.qinq_direction = OLT_CFG_DIR_DOWNLINK;
    return_result = OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config);
    if ( OLT_CALL_ISERROR(return_result) )
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                break;
            case OLT_ERR_NORESC:
                vty_out(vty,"\r\n Failedto set pon%d/%d for downlink vlan-translation table full\r\n", ulSlot, ulPort);
                break;
            default:
				vty_out(vty,"\r\n set pon%d/%d downlink vlan-translation err:%d\r\n", ulSlot, ulPort, return_result);
        }
        
    	return (CMD_WARNING);
    }
#else
	if(SavePonDownlinkVlanManipulation(PonPortIdx, original_vlan, &vlan_downlink_config) == ROK)
	{
		if(Olt_exists(PonPortIdx))
		{
			return_result = PAS_set_vid_downlink_mode(PonPortIdx, original_vlan, vlan_downlink_config);		
			if( return_result != PAS_EXIT_OK)
            {
				vty_out(vty,"\r\n set pon%d/%d downlink vlan-translation err:%d\r\n",ulSlot, ulPort, return_result);
            	return (CMD_WARNING);
            }         
		}
	}
	else
    {
        vty_out(vty,"\r\n set pon%d/%d downlink vlan-translation table full\r\n", ulSlot, ulPort);
    	return (CMD_WARNING);
    }   
#endif

	return (CMD_SUCCESS);
}

DEFUN(
	undo_downlink_set_pon_vlan_trans_tag,
	undo_downlink_set_pon_vlan_trans_tag_cmd,
	"undo downlink vlan-tag-manipulation <1-4094>",
	"undo pon config\n"
	"undo pon downlink\n"
	"undo pon downlink vlan manipulation\n"
	/*"null-taged frames\n"*/
	/*"un-taged frames\n"*/
	"original vlan-id\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	/*short int PonChipType;*/
	short int return_result;
	PON_vlan_tag_t  original_vlan;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	if( VOS_MemCmp(argv[0],"null-tag", 8 ) == 0 )
		original_vlan = NULL_VLAN;
	else if(VOS_MemCmp(argv[0],"untag", 5 ) == 0 )
		original_vlan = VLAN_UNTAGGED_ID;
	else
        original_vlan = VOS_AtoL( argv[0] );

    /* B--added by liwei056@2012-5-22 for D10708 */    
    GetPonDownlinkVlanQinQ(PonPortIdx, original_vlan, &vlan_qinq_policy);
    if ( PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION == vlan_qinq_policy.vlan_manipulation )
    {
		vty_out(vty,"  %% pon%d/%d have not any vlan%d's downlink qinq config.\r\n", ulSlot, ulPort, original_vlan);
		return( CMD_WARNING );
    }
    /* E--added by liwei056@2012-5-22 for D10708 */    

#if 1
    return_result = ClearPonDownlinkVlanManipulation(PonPortIdx, original_vlan);
    if ( OLT_CALL_ISERROR(return_result) )
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                break;
            default:
				vty_out(vty,"\r\n delete pon%d/%d downlink vlan-manipulation err:%d\r\n", ulSlot, ulPort, return_result);
        }
        
    	return (CMD_WARNING);
    }
#else
	DeletePonDownlinkVlanManipulation(PonPortIdx, original_vlan);
	if(Olt_exists(PonPortIdx))
	{
		return_result = PAS_delete_vid_downlink_mode(PonPortIdx, original_vlan);
		if( return_result != PAS_EXIT_OK)
			vty_out(vty,"\r\n delete pon%d/%d downlink vid= %d vlan manipulation err:%d\r\n",ulSlot, ulPort,original_vlan, return_result);
	}
#endif

	return (CMD_SUCCESS);
}

DEFUN(show_pon_vlan_trans_uplink,
	show_pon_vlan_trans_uplink_cmd,
		"show uplink vlan-tag-manipulation <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	"show pon config\n"	
	"pon vlan manipulation\n"
	"uplink direction\n"
	"onuIdx\n"
	)
{
	/*PON_olt_id_t			        olt_id;
	PON_llid_t				        llid;*/
	short int				        return_result;
	/*PON_olt_vlan_uplink_config_t    vlan_uplink_config ;*/

	unsigned long  ulSlot = 0, ulPort = 0, ulOnuId ;
	short int  PonPortIdx;
	short int  OnuIdx;
	/*short int  PonChipType;*/
	Olt_llid_vlan_manipulation vlan_qinq_policy;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	/*
	if( PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		vty_out(vty, "\r\n %% pon%d/%d not working\r\n", ulSlot, ulPort );
		return( ROK );
		}
	*/

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );
	}
#endif

	ulOnuId = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	OnuIdx = ulOnuId - 1;
    CHECK_CMD_ONU_RANGE(vty, OnuIdx);
	
#if 0
	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK )
	{
		vty_out(vty, "  %% onu %d/%d/%d not exist\r\n", ulSlot, ulPort, ulOnuId );
		return( CMD_WARNING );
	}
#endif

    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);
	
#if 0
	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
#endif
	{
        switch ( vlan_qinq_policy.vlan_manipulation )
        {
            case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED:
			{
    			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
    			vty_out(vty,"uplink vlan manipulation: vlan-tag-exchanged\r\n");
    			vty_out(vty,"original vid: ");
    			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
    				vty_out(vty,"all-frame\r\n");
    			else if(vlan_qinq_policy.original_vlan_id == NULL_VLAN)
    				vty_out(vty,"null-tagged\r\n");
    			else 
    				vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
    			vty_out(vty,"new vid: %d\r\n",vlan_qinq_policy.new_vlan_id);
    			vty_out(vty,"vlan priority: ");
    			if(vlan_qinq_policy.new_priority == PON_VLAN_ORIGINAL_PRIORITY )
    				vty_out(vty,"original priority\r\n");
    			else vty_out(vty,"%d\r\n",vlan_qinq_policy.new_priority);
    			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
    				vty_out(vty,"vlan tpid: 0x8100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
    				vty_out(vty,"vlan tpid: 0x9100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
    				vty_out(vty,"vlan tpid: 0x88a8\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
    				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
			}
            break;
            case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED:
			{
    			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
    			vty_out(vty,"uplink vlan manipulation: vlan-tag-add\r\n");
    			vty_out(vty,"inter vid: ");
    			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
    				vty_out(vty,"all frame\r\n");
    			else vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
    			vty_out(vty,"outer vid: %d\r\n",vlan_qinq_policy.new_vlan_id);
    			vty_out(vty,"vlan priority: ");
    			if(vlan_qinq_policy.new_priority == PON_VLAN_ORIGINAL_PRIORITY )
    				vty_out(vty,"original priority\r\n");
    			else vty_out(vty,"%d\r\n",vlan_qinq_policy.new_priority);
    			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
    				vty_out(vty,"vlan tpid: 0x8100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
    				vty_out(vty,"vlan tpid: 0x9100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
    				vty_out(vty,"vlan tpid: 0x88a8\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
    				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
			}
            break;
            case PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED:
            {
    			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
    			vty_out(vty,"uplink vlan manipulation: vlan-tag-filter\r\n");
    			vty_out(vty,"original vid: ");
    			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
    				vty_out(vty,"all frame\r\n");
    			else vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
    			
    			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
    				vty_out(vty,"vlan tpid: 0x8100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
    				vty_out(vty,"vlan tpid: 0x9100\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
    				vty_out(vty,"vlan tpid: 0x88a8\r\n");
    			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
    				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
			}
            break;
            default:
            {
    			vty_out(vty,"uplink vlan manipulation: no manipulation\r\n");
    			/*vty_out(vty,"onu%d/%d/%d uplink no vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
            }
        }
    }
   
	return( CMD_SUCCESS);

#if 0
	llid = GetLlidByOnuIdx( PonPortIdx, (ulOnuId - 1));
	if( llid == INVALID_LLID)
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId );
		return( CMD_WARNING );
		}

	olt_id = PonPortIdx;
	return_result = PAS_get_llid_vlan_mode  ( olt_id, 
											  llid, 
											  &vlan_uplink_config );

	if ( return_result == PAS_EXIT_OK )
	{
	/*
		WRITE_OLT_PARAMETER
		WRITE_LLID_PARAMETER
        WRITE_UNTAGGED_FRAMES_AUTHENTICATION_VID_PARAMETER(vlan_uplink_config.untagged_frames_authentication_vid,Untagged frames authentication vid)
        WRITE_AUTHENTICATED_VID_WITH_ALL_FRAMES_AUTHENTICATE(vlan_uplink_config.authenticated_vid     ,Authenticated vid)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_untagged               ,Untagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_tagged                 ,Tagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_null_tagged            ,Null tagged frames)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vlan_uplink_config.discard_nested                 ,Nested frames)
		
        WRITE_VLAN_UPLINK_MANIPULATION_PARAMETER(vlan_uplink_config.vlan_manipulation                 ,Vlan manipulation)*/
        switch(vlan_uplink_config.vlan_manipulation) 
        {                                                                  
        case PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION              : 
		vty_out(vty,"uplink vlan manipulation: no manipulation\r\n");
		break;
        case PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED           : 
		vty_out(vty,"uplink vlan manipulation: vlan-tag-filter\r\n");
		if(vlan_uplink_config.authenticated_vid == PON_ALL_FRAMES_AUTHENTICATE )
				vty_out(vty,"original vid: all frames\r\n");
		else if(vlan_uplink_config.authenticated_vid == NULL_VLAN )
				vty_out(vty,"original vid: null-tagged\r\n");
		else if(vlan_uplink_config.authenticated_vid == VLAN_UNTAGGED_ID )
				vty_out(vty,"original vid: untagged\r\n");
		else WRITE_USHORT_PARAMETER(vlan_uplink_config.authenticated_vid,original vid)

		 if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
			vty_out(vty,"vlan tpid: 0x8100\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
			vty_out(vty,"vlan tpid: 0x9100\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
			vty_out(vty,"vlan tpid: 0x88a8\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
			{
			unsigned short int vlan_tpid=0;
			PAS_get_vlan_recognizing(PonPortIdx, &vlan_tpid);
			vty_out(vty,"vlan tpid(user defined):0x%x\r\n",vlan_tpid);
			}
            break;
        case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED : 
			vty_out(vty,"uplink vlan manipulation: vlan-tag-exchanged\r\n");
			if(vlan_uplink_config.authenticated_vid == PON_ALL_FRAMES_AUTHENTICATE )
				vty_out(vty,"original vid: any-tagged\r\n");
			else if(vlan_uplink_config.authenticated_vid == NULL_VLAN )
				vty_out(vty,"original vid: null-tagged\r\n");
			else WRITE_USHORT_PARAMETER(vlan_uplink_config.authenticated_vid,original vid)
			 WRITE_USHORT_PARAMETER(vlan_uplink_config.new_vlan_tag_id,new vid)
			 WRITE_VLAN_PRIORITY_WITH_ORIGINAL_PRIORITY(vlan_uplink_config.vlan_priority,vlan priority)
			 if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
				vty_out(vty,"vlan tpid: 0x8100\r\n");
			else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
				vty_out(vty,"vlan tpid: 0x9100\r\n");
			else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
				vty_out(vty,"vlan tpid: 0x88a8\r\n");
			else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
				{
				unsigned short int vlan_tpid=0;
				PAS_get_vlan_recognizing(PonPortIdx, &vlan_tpid);
				vty_out(vty,"vlan tpid(user defined):0x%x\r\n",vlan_tpid);
				}
			 break; 
			 
	case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED     : 
		vty_out(vty,"uplink vlan manipulation: vlan-tag-add\r\n");
		if(vlan_uplink_config.authenticated_vid == PON_ALL_FRAMES_AUTHENTICATE)
			vty_out(vty,"inter vid: all frame\r\n");
		else if(vlan_uplink_config.authenticated_vid == NULL_VLAN )
			vty_out(vty,"inter vid: null-tagged\r\n");
		else 
			WRITE_USHORT_PARAMETER(vlan_uplink_config.authenticated_vid,inter vid)
            WRITE_USHORT_PARAMETER(vlan_uplink_config.new_vlan_tag_id,out vid)
            /*WRITE_VLAN_ETHERNET_TYPE_PARAMETER(vlan_uplink_config.vlan_type,Vlan type)*/
            WRITE_VLAN_PRIORITY_WITH_ORIGINAL_PRIORITY(vlan_uplink_config.vlan_priority,vlan priority)
            	if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
			vty_out(vty,"vlan tpid: 0x8100\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
			vty_out(vty,"vlan tpid: 0x9100\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
			vty_out(vty,"vlan tpid: 0x88a8\r\n");
		else if(vlan_uplink_config.vlan_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
			{
			unsigned short int vlan_tpid=0;
			PAS_get_vlan_recognizing(PonPortIdx, &vlan_tpid);
			vty_out(vty,"vlan tpid(user defined):0x%x\r\n",vlan_tpid);
			}
    	    break;
        default:
            break;
        }
	}else
    {
		CLI_ERROR_PRINT
    }

	return (return_result);
#endif
}

DEFUN(show_pon_vlan_trans_uplink1,
	show_pon_vlan_trans_uplink1_cmd,
		"show uplink vlan-tag-manipulation <slot/port> <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
	"show pon config\n"	
	"pon vlan manipulation\n"
	"uplink direction\n"
	"Please input slot/port\n"
	"onuIdx\n"
	)
{
	short int				        return_result;
	unsigned long  ulSlot = 0, ulPort = 0, ulOnuId ;
	short int  PonPortIdx;
	short int  OnuIdx;
	Olt_llid_vlan_manipulation vlan_qinq_policy;

    IFM_ParseSlotPort( argv[0], &ulSlot, &ulOnuId );	

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulOnuId, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
		return(CMD_WARNING);
	
    PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulOnuId );
    if (PonPortIdx == VOS_ERROR)
    {  
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	ulOnuId = ( ULONG ) VOS_AtoL( argv[ 1 ] );
	OnuIdx = ulOnuId - 1;
    CHECK_CMD_ONU_RANGE(vty, OnuIdx);
	
	GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);

	switch ( vlan_qinq_policy.vlan_manipulation )
	{
		case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED:
		{
			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
			vty_out(vty,"uplink vlan manipulation: vlan-tag-exchanged\r\n");
			vty_out(vty,"original vid: ");
			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
				vty_out(vty,"all-frame\r\n");
			else if(vlan_qinq_policy.original_vlan_id == NULL_VLAN)
				vty_out(vty,"null-tagged\r\n");
			else 
				vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
			vty_out(vty,"new vid: %d\r\n",vlan_qinq_policy.new_vlan_id);
			vty_out(vty,"vlan priority: ");
			if(vlan_qinq_policy.new_priority == PON_VLAN_ORIGINAL_PRIORITY )
				vty_out(vty,"original priority\r\n");
			else vty_out(vty,"%d\r\n",vlan_qinq_policy.new_priority);
			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
				vty_out(vty,"vlan tpid: 0x8100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
				vty_out(vty,"vlan tpid: 0x9100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
				vty_out(vty,"vlan tpid: 0x88a8\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
		}
        break;
        case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED:
		{
			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
			vty_out(vty,"uplink vlan manipulation: vlan-tag-add\r\n");
			vty_out(vty,"inter vid: ");
			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
				vty_out(vty,"all frame\r\n");
			else vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
			vty_out(vty,"outer vid: %d\r\n",vlan_qinq_policy.new_vlan_id);
			vty_out(vty,"vlan priority: ");
			if(vlan_qinq_policy.new_priority == PON_VLAN_ORIGINAL_PRIORITY )
				vty_out(vty,"original priority\r\n");
			else vty_out(vty,"%d\r\n",vlan_qinq_policy.new_priority);
			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
				vty_out(vty,"vlan tpid: 0x8100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
				vty_out(vty,"vlan tpid: 0x9100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
				vty_out(vty,"vlan tpid: 0x88a8\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
		}
        break;
        case PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED:
        {
			/*vty_out(vty,"onu%d/%d/%d uplink vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
			vty_out(vty,"uplink vlan manipulation: vlan-tag-filter\r\n");
			vty_out(vty,"original vid: ");
			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
				vty_out(vty,"all frame\r\n");
			else vty_out(vty,"%d\r\n",vlan_qinq_policy.original_vlan_id);
			
			if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
				vty_out(vty,"vlan tpid: 0x8100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
				vty_out(vty,"vlan tpid: 0x9100\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
				vty_out(vty,"vlan tpid: 0x88a8\r\n");
			else if(vlan_qinq_policy.ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE)
				vty_out(vty,"vlan tpid: 0x%x\r\n",PonPortTable[PonPortIdx].vlan_tpid);
		}
        break;
        default:
        {
			vty_out(vty,"uplink vlan manipulation: no manipulation\r\n");
			/*vty_out(vty,"onu%d/%d/%d uplink no vlan manipulation\r\n",ulSlot, ulPort, ulOnuId );*/
        }
    }
    
   
	return( CMD_SUCCESS);
}

DEFUN(show_onu_vlan_trans_uplink,
	show_onu_vlan_trans_uplink_cmd,
	"show onu-uplink vlan-tag-configuation <H.H.H>",
	"Show onu config\n"	
	"Onu uplink direction\n"
	"Qinq vlan config\n"
	"Please input mac address\n"
	)
{
	short int				        return_result;
	unsigned long  ulSlot = 0, ulPort = 0, ulOnuId = 0;
	short int  PonPortIdx = 0;
	short int  OnuIdx = 0;
    short int OnuEntry = 0;
	Olt_llid_vlan_manipulation vlan_qinq_policy;
    USHORT vlan[64] = {0};
    int vlan_num = 0;
    UCHAR MacAddress[6] = {0};
	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddress ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
	if(ThisIsInvalidMacAddr(MacAddress) == TRUE )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}
    
    OnuEntry = GetOnuEntryByMac(MacAddress);
    if(OnuEntry == RERROR)
    {
        vty_out(vty, " %Can not find the specified onu!\r\n");
        return CMD_WARNING;
    }
    PonPortIdx = OnuEntry/MAXONUPERPON;
    OnuIdx = OnuEntry%MAXONUPERPON;
    ulSlot = GetCardIdxByPonChip(PonPortIdx);
    ulPort = GetPonPortByPonChip(PonPortIdx);
    ulOnuId = OnuIdx + 1;
    VOS_MemZero(&vlan_qinq_policy, sizeof(Olt_llid_vlan_manipulation));
    
    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
    switch ( vlan_qinq_policy.vlan_manipulation )
    {            
        case PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED:
            vty_out(vty, "\r\nPon%d/%d:\r\n", ulSlot, ulPort);
            vty_out(vty, " %5s %14s %7s %20s\r\n", "OnuId", " Mac address  ", "Out-vid", "   Inner-vid        ");
            vty_out(vty, " %5s %14s %7s %20s\r\n", "=====", "==============", "=======", " ===================");
			if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
			{
                if(get_onu_allvlanstr(PonPortIdx, OnuIdx, vlan, &vlan_num) == VOS_OK && vlan_num)
                {
                    int lineCount= 5;
                    int loop_x = vlan_num/lineCount;
                    int loop_y = vlan_num%lineCount;
                    int i = 0;
                    int j = 0;
                    
                    for(i=0;i<loop_x+1;i++)
                    {
                        UCHAR vlanstr[32] = {0};
                        int len = 0;
                        if(vlan_num == 1)
                        {
                            len += VOS_Sprintf(vlanstr, "%d", vlan[0]);                    
                        }
                        else
                        {
                            len += VOS_Sprintf(vlanstr, "%d", vlan[i*lineCount]);                                        
                            for(j=1;j<lineCount;j++)
                            {
                                if(vlan[i*lineCount+j])
                                    len += VOS_Sprintf(vlanstr+len, ",%d", vlan[i*lineCount+j]);                    
                            }
                        }
                        if(i)
                            vty_out(vty, " %5s %14s %7s    %-20s\r\n", "", "", "", vlanstr);
                        else                                
                            vty_out(vty, " %5d %14s %7d    %-20s\r\n", ulOnuId, argv[0], vlan_qinq_policy.new_vlan_id, vlanstr);                                                
                    }
                    vty_out(vty, "\r\n");
                }
			}
            else
            {
                if(search_onu_vlan(PonPortIdx, OnuIdx, vlan_qinq_policy.original_vlan_id) == VOS_OK)
                    vty_out(vty, " %5d %14s %7d    %-20d\r\n", ulOnuId, argv[0], vlan_qinq_policy.new_vlan_id, vlan_qinq_policy.original_vlan_id);
                else
                {
                    vty_out(vty, " %5d %14s %7d    %-20d\r\n", ulOnuId, argv[0], vlan_qinq_policy.new_vlan_id, vlan_qinq_policy.original_vlan_id);                        
                    vty_out(vty, "WARNING: Can not find the specified inner vlan in Onu(%s)!\r\n", argv[0]);
                    vty_out(vty, "         Please config this onu!\r\n");
                }
                vty_out(vty, "\r\n");                
            }
            break;
        default:
            vty_out(vty, "The specified onu has no qinq vlan config\r\n");
            break;
    }
	return( CMD_SUCCESS);
}
DEFUN(show_pon_vlan_trans_downlink,
		show_pon_vlan_trans_downlink_cmd,
		"show downlink vlan-tag-manipulation {<1-4094>}*1",
		"show pon config\n"		
		"downlink direction\n"
		"pon vlan manipulation\n"		
		/*"null-taged frames\n"*/
		/*"un-taged frames\n"*/
		"downlink original vlan-id\n"
	)


{
	short int                       olt_id;
	/*short int                       return_result;*/
	PON_vlan_tag_t                  vlanid;
	/*PON_olt_vid_downlink_config_t   vid_downlink_config;*/
	
	unsigned long  ulSlot = 0, ulPort = 0, ulOnuId ;
	short int  PonPortIdx;
	/*short int  PonChipType;*/

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/* 支持予配置,所以不需对槽位及PON 芯片在位检查*/
#if 0
#if 0
	/* 1 板在位检查	*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	/*
	if( PonPortIsWorking(PonPortIdx) != TRUE ) 
		{
		vty_out(vty, "\r\n %% pon%d/%d not working\r\n", ulSlot, ulPort );
		return( ROK );
		}
	*/

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    /* B--modified by liwei056@2010-5-25 for D10157 */
	if( (PonChipType != RERROR) && (PonChipType != PONCHIP_PAS5201) )
    /* E--modified by liwei056@2010-5-25 for D10157 */
	{
		vty_out(vty,"\r\n this is not PAS5201, cli is not supported\r\n");
		return( CMD_WARNING );		
	}
#endif

	olt_id = PonPortIdx;

	if(argc == 0)
	{
		ShowPonDownlinkVlanManipulation(PonPortIdx, -1, vty);
		return(CMD_SUCCESS);
	}

	if( VOS_MemCmp(argv[0],"null-tag", 8 ) == 0 )
		vlanid = NULL_VLAN;
	else if(VOS_MemCmp(argv[0],"untag", 5 ) == 0 )
		vlanid = VLAN_UNTAGGED_ID;
	else vlanid = VOS_AtoL( argv[0] );    

#if 0
	if(!(OLTAdv_IsExist(olt_id ) )) 
#endif
	{
#if 1
		ShowPonDownlinkVlanManipulation(PonPortIdx, vlanid, vty);
#else
		vty_out(vty," pon%d/%d is not working\r\n", ulSlot, ulPort);
		return CMD_WARNING;
#endif
	}

#if 0
    return_result = PAS_get_vid_downlink_mode ( olt_id,
                                                vlanid,
                                                &vid_downlink_config );

	if ( return_result ==  PAS_EXIT_OK)
    {
		vty_out(vty,"pon%d/%d:\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id));
		if( vlanid == NULL_VLAN)
			vty_out(vty, "vid: null-tag\r\n");
		else if(vlanid == VLAN_UNTAGGED_ID)
			vty_out(vty," vid: untagged\r\n");
		else 
        		WRITE_VID_PARAMETER(vlanid,vid)
        WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(vid_downlink_config.discard_nested,Nested frames)
        WRITE_VLAN_DESTINATION_PARAMETER(vid_downlink_config.destination,Destination)
        if ( (vid_downlink_config.destination == PON_OLT_VLAN_DESTINATION_LLID_PARAMETER) ||
             (vid_downlink_config.destination == PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE_AND_LLID_PARAMETER) 
           )
        {
            WRITE_SHORT_PARAMETER(vid_downlink_config.llid,LLID)
        }

        WRITE_VLAN_DOWNLINK_MANIPULATION_PARAMETER(vid_downlink_config.vlan_manipulation,VLAN manipulation)
        switch(vid_downlink_config.vlan_manipulation) 
        {
        case PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION            :
        case PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG            :
            break;
        case PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID               :
            WRITE_USHORT_PARAMETER(vid_downlink_config.new_vid,New vid)
            break;
        case PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY  :
            WRITE_USHORT_PARAMETER(vid_downlink_config.new_vid,New vid)
            WRITE_USHORT_PARAMETER(vid_downlink_config.new_priority,New priority)
            break;

        default:
            break;
        }

    }
	else
    {
		CLI_ERROR_PRINT
    }
#endif

	return CMD_SUCCESS;
}
DEFUN(show_pon_vlan_trans_downlink1,
		show_pon_vlan_trans_downlink1_cmd,
		"show downlink vlan-tag-manipulation <slot/port> {<1-4094>}*1",
		"show pon config\n"		
		"downlink direction\n"
		"pon vlan manipulation\n"		
		/*"null-taged frames\n"*/
		/*"un-taged frames\n"*/
        "input the slot/port\n"
		"downlink original vlan-id\n"
	)


{
	PON_vlan_tag_t  vlanid = 0;	
	unsigned long   ulSlot = 0, ulPort = 0;
	short int       PonPortIdx = 0;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return(CMD_WARNING);

    PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
	if(argc == 1)
	{
		ShowPonDownlinkVlanManipulation(PonPortIdx, -1, vty);
		return(CMD_SUCCESS);
	}

	if( VOS_MemCmp(argv[1],"null-tag", 8 ) == 0 )
		vlanid = NULL_VLAN;
	else if(VOS_MemCmp(argv[1],"untag", 5 ) == 0 )
		vlanid = VLAN_UNTAGGED_ID;
	else vlanid = VOS_AtoL( argv[1] );    

	ShowPonDownlinkVlanManipulation(PonPortIdx, vlanid, vty);
	return CMD_SUCCESS;
}


#endif

/* added by chenfj 2008-4-23
     删除OLT PON 端口学到的MAC 地址表项
     */
    /* modified by chenfj 2008-5-20
    问题单: 6712. PON节点下delete fdbentry mac 为ONU的MAC时无错误提示 
    这个命令的本意,是想删除从ONU上学到数据MAC,而非ONU设备MAC
    对CLI,若想删除ONU设备MAC,则给出提示,不删除
    对SNMP, 暂不相应接口
    */
DEFUN(
	delete_olt_pon_address_entry,
	delete_olt_pon_address_entry_cmd,
	"delete fdbentry mac <H.H.H>",
	"delete pon address entry\n" 
	"delete pon address entry\n"
	"the mac address\n"
	"the mac address learned from onu or cni\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuId;
	/*unsigned long ulIfIndex;
	long lRet;*/
	short int ret;
	short int record_num = 0;

	short int PonPortIdx/*, PonChipType*/;
	unsigned char MacAddress[BYTES_IN_MAC_ADDRESS] = {0};
	int i, flag = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
		}
	
	if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;
#endif
	
#if 0
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	if((PonChipType != PONCHIP_PAS5201) && (PonChipType != PONCHIP_PAS5001))
	{
		vty_out(vty,"\r\nthis is not PAS-chip, delete mac entry is not supported currently\r\n");
		return( CMD_WARNING );
	}
#endif

	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddress ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
	if(ThisIsInvalidMacAddr(MacAddress) == TRUE )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}
	if(GetOnuIdxByMacPerPon(PonPortIdx, MacAddress) != RERROR )
	{
		vty_out(vty,"this is a onu device mac, can't delete it\r\n");
		return (CMD_WARNING );
	}

#if 1
    OLT_GetMacAddrTbl(PonPortIdx, &record_num, Mac_addr_table);
#else
	PAS_get_address_table(PonPortIdx, &record_num, Mac_addr_table );
#endif
	for(i=0; i< record_num; i++)
	{
		if( MAC_ADDR_IS_EQUAL(MacAddress, Mac_addr_table[i].mac_address) )
		{
			flag = 0xa5;
			break;
		}
	}
	if(flag != 0xa5 )
	{
		vty_out(vty, "can't find fdbentry mac %02x%02x.%02x%02x.%02x%02x\r\n",MacAddress[0],MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);
		return( CMD_WARNING );
	}

#if 1
    ret = OLT_RemoveMac(PonPortIdx, MacAddress);
#else
	ret = PAS_remove_address_table_record(PonPortIdx, MacAddress);
#endif
	if( OLT_CALL_ISERROR(ret) )
		vty_out(vty, "%%  Executing error\r\n");

	return( CMD_SUCCESS);
}

/* B--added by liwei056@2011-2-18 for Test-StaticMac */
DEFUN(
	add_olt_pon_static_address_func,
	add_olt_pon_static_address_cmd,
	"olt-mac add <H.H.H> to cni",
	"config olt's static mac address\n"
	"add a static mac address\n"
	"please input the mac address\n"
	"select the mac address's src-port\n"
	"the mac address is comming from CNI port\n"
)
{
    int ret;
    PON_address_record_t stMac;
    short int iOltID;

    if ( PON_PORT_NODE == vty->node )
    {
    	unsigned long ulSlot, ulPort, ulOnuId;
        
    	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &iOltID) != VOS_OK )
    		return CMD_WARNING;

    	if( PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK )
    		return(CMD_WARNING);
    }
    else
    {
        iOltID = OLT_ID_ALL;
    }
    
    if ( VOS_ERROR == GetMacAddr(argv[0], stMac.mac_address) )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( ThisIsInvalidMacAddr(stMac.mac_address) == TRUE )
    {
        vty_out(vty,"This is a Invalid mac address\r\n");
        return (CMD_WARNING);
    }

    /* B--added by liwei056@2011-5-27 for D12828 */
    if ( PonStaticMacTableIsFull(stMac.mac_address) == TRUE )
    {
        vty_out( vty, "  %% Olt's static MAC table is full.\r\n" );
        return (CMD_WARNING);
    }
    /* E--added by liwei056@2011-5-27 for D12828 */
    
    stMac.type = ADDR_STATIC;
#if 1
    stMac.logical_port = PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID;    
#else
    if ( 'c' == argv[1][0] )
    {
        stMac.logical_port = PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID;    
    }
    else
    {
        stMac.logical_port = VOS_AtoI(argv[1]);    
    }
#endif

    ret = OLT_AddMacAddrTbl(iOltID, 1, &stMac);
    if ( OLT_CALL_ISERROR(ret) )
    {
        switch (ret)
        {
            case OLT_ERR_NOTEXIST:
                vty_out( vty, "  %% Olt is not working.\r\n" );
                break;
            case OLT_ERR_NORESC:
                vty_out( vty, "  %% Olt's static MAC table is full.\r\n" );
                break;
            default:    
                vty_out( vty, "  %% Setting failed(%d).\r\n", ret );
        }
        
    	return CMD_WARNING;
    }
   
    return CMD_SUCCESS;	
}

DEFUN(
	del_olt_pon_address_func,
	del_olt_pon_address_cmd,
	"olt-mac delete <H.H.H>",
	"config olt's static mac address\n"
	"delete a static mac address\n"
	"please input the mac address\n"
)
{
	int ret;
    PON_address_record_t stMac;
    short int iOltID;

    if ( PON_PORT_NODE == vty->node )
    {
    	unsigned long ulSlot, ulPort, ulOnuId;
        
    	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &iOltID) != VOS_OK )
    		return CMD_WARNING;

    	if( PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK )
    		return(CMD_WARNING);
    }
    else
    {
        iOltID = OLT_ID_ALL;
    }
	
	if (VOS_ERROR == GetMacAddr(argv[0], stMac.mac_address))
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );

    	return CMD_WARNING;
	}

	if ( ThisIsInvalidMacAddr(stMac.mac_address) == TRUE )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}

    stMac.logical_port = PON_ALL_LLIDS;
    stMac.type = ADDR_DYNAMIC_AND_STATIC;
    ret = OLT_DelMacAddrTbl(iOltID, 1, &stMac);
    if ( OLT_CALL_ISERROR(ret) )
    {
        switch (ret)
        {
            case OLT_ERR_NOTEXIST:
        		vty_out( vty, "  %% Olt is not working.\r\n" );
                break;
            default:    
        		vty_out( vty, "  %% Setting failed(%d).\r\n", ret );
        }
        
    	return CMD_WARNING;
    }
   
	return CMD_SUCCESS;	
}

DEFUN(
	show_olt_pon_address_func,
	show_olt_pon_address_cmd,
	"show olt-mac",
    DescStringCommonShow
    "Show OLT's global static mac table\n"
)
{
    int i;

    if ( GlobalPonAddrTblNum > 0 )
    {
        unsigned char *pbMacAddr;

		vty_out( vty, "total added mac counter = %d\r\n\r\n", GlobalPonAddrTblNum);        
    	vty_out(vty, "   Mac addr         type      by OnuIdx\r\n");
    	vty_out(vty, "-----------------------------------------\r\n");
        for (i=0; i<GlobalPonAddrTblNum; i++)
        {
            pbMacAddr = GlobalPonStaticMacAddr[i];
            
    		vty_out( vty, "%02x%02x.%02x%02x.%02x%02x     static      CNI\r\n", pbMacAddr[0], pbMacAddr[1], pbMacAddr[2], pbMacAddr[3], pbMacAddr[4], pbMacAddr[5]);
        }
		vty_out( vty, "\r\n" );
    }
    else
    {
		vty_out( vty, "  %% Olt static mac-table is empty.\r\n" );
    }
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2011-2-18 for Test-StaticMac */


		
#ifdef _NORMAL_USER_PROPERTITY_
#endif
/*
    added by chenfj 2008-4-29
    扩展normal 用户的查询权限
    以下命令由PON 节点扩展到view 节点
    */
#if 0
DEFUN  (
    view_show_encrypt_config,
    view_show_encrypt_config_cmd,
    "show encrypt information <slot/port>",
    DescStringCommonShow
    "Show pon's encrypt attribute\n"
    "Show pon's encrypt information\n"
    "input the slot/port\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;
    INT16 phyPonId = 0;	

   IFM_ParseSlotPort( argv[0], &slotId, &port );

	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	lRet = ShowPonPortEncryptInfoByVty(phyPonId, vty );
	if (lRet != VOS_OK)
	    {  
	           vty_out( vty, "  %% Executing error.\r\n");
	       	return CMD_WARNING;
	    }		

    return CMD_SUCCESS;
}


DEFUN  (
    view_onu_max_mac_show,
    view_onu_max_mac_show_cmd,
    "show onu max-mac <slot/port> <onuid_list>",
    DescStringCommonShow
    "Show the max-mac number\n"
    "Show the max-mac number\n"
    "input the slot/port\n"
    OnuIDStringDesc
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG onuId = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    UINT32 number = 0;
	int count = 0;

     IFM_ParseSlotPort( argv[0], &slotId, &port );	

	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {      
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], onuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	
	if ( count == 1 )  /*参数中只有一个ONU */
		{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], onuId )
	 		{
	 		userOnuId = (short int)(onuId - 1);
			lRet = GetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, &number);
			if ( lRet == V2R1_ONU_NOT_EXIST )
				{
				vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",slotId,port,onuId);
				return CMD_WARNING;  
				}
			if( lRet == RERROR ) 
				{
				vty_out( vty, "  %% get onu%d/%d/%d max mac err\r\n",slotId,port,onuId);
				return CMD_WARNING;  
				}
			vty_out( vty, "  onu%d/%d/%d supported max-mac number is %d\r\n",slotId,port,onuId,number);
			}
		END_PARSE_ONUID_LIST_TO_ONUID();
		}
	
	else /*  参数中有多个ONU */
		{
		
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], onuId )
	 		{
	 		userOnuId = (short int)(onuId - 1);
			lRet = GetOnuMaxMacNum((short int )phyPonId, (short int)userOnuId, &number);
			if( lRet == ROK ) 
				{
				vty_out( vty, "   onu%d/%d/%d max-mac supported---%d\r\n",slotId,port,onuId,number);
				}
			}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out(vty, "\r\n");
		}
	
    return CMD_SUCCESS;
}
#endif

/*显示PON 地址表老化时间*/
DEFUN  (
    view_show_pon_mac_age,
    view_show_pon_mac_age_cmd,
    "show aging-mac-time <slot/port>",
    DescStringCommonShow
    "show the mac aging time(s) of the pon\n"
    "input the slot/port\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;	

    INT16 phyPonId = 0;	
    UINT32 AgeingTime = 0;


    IFM_ParseSlotPort( argv[0], &slotId, &port );	

	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(slotId, vty) != ROK )
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {  
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
    lRet = GetPonPortMacAgeingTime( phyPonId,  &AgeingTime );
    if (lRet != VOS_OK)
    {  
    	vty_out( vty, "  %% Executing error.\r\n" );
    	return CMD_WARNING;
    }
    vty_out(vty, "  aging-mac-time %d ( s)\r\n\r\n",AgeingTime/1000);	


    return CMD_SUCCESS;
}

DEFUN  (
    view_show_bandwidth_config,
    view_show_bandwidth_config_cmd,
    "show bandwidth <slot/port>",
    DescStringCommonShow
    "Show bandwidth information\n"
    "input the slot/port\n"
    )
{
    INT16 phyPonId = 0;	
	ULONG slotId=0, port=0;
	
	 IFM_ParseSlotPort( argv[0], &slotId, &port );	

	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(slotId, vty) != ROK )
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

    ShowPonPortBWInfoByVty(phyPonId , vty);
    /*ShowPonPortBWInfo( (short int )phyPonId );*/
	
    return CMD_SUCCESS;
}

DEFUN  (
    view_show_llid_bandwith_config,
    view_show_llid_bandwith_config_cmd,
    "show bandwidth logical-link <slot/port> <onuid_list>",
    DescStringCommonShow
    "Show bandwidth information\n"
    "Show bandwidth information of logical link\n"
    "input the slot/port\n"
    OnuIDStringDesc
    )
{
	LONG lRet = VOS_OK;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;
	INT16   userOnuId = 0;  
	int count = 0;
	ULONG slotId=0, port=0;

	IFM_ParseSlotPort( argv[0], &slotId, &port );
	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(slotId, vty) != ROK )
		return(CMD_WARNING);
		
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((phyPonId < CLI_EPON_PONMIN) || (phyPonId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	if (count == 1)
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	 	{
	 		userOnuId = (short int )(ulOnuId - 1);
			/*lRet = ShowSingleOnuBandwidthbyVty( phyPonId, userOnuId, vty );*/
			/*lRet = ShowOnuBandwidthByVty(phyPonId, userOnuId, vty );*/
			lRet = ShowOnuBandwidthByVty_1(phyPonId, userOnuId, vty );
			if ( V2R1_ONU_NOT_EXIST == lRet )	/* modified by xieshl 20100325, 问题单9978 */
			{
				vty_out( vty, "  %% Onu %d is not exist.\r\n", ulOnuId);
			}
	    	else if (lRet != VOS_OK)
	         {
	           	vty_out( vty, "  %% Executing error.\r\n");
	       	RETURN_PARSE_ONUID_LIST_TO_ONUID( CMD_WARNING );
	         }
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
		return CMD_SUCCESS;
	}
	else
	{
		/*vty_out (vty, "\r\n  %s/port%d Onu bandwidth Information list \r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId));
		vty_out (vty, "  OnuIdx   provisioned Bandwidth          Activated Bandwidth\r\n");
		vty_out (vty, "          Uplink         Downlink        Uplink        Downlink(Unit: kbit/s)\r\n");
		*/
#ifdef PLATO_DBA_V3
		vty_out (vty, "\r\n  OnuIdx   direction  class   fixbw   assured-bw(kbit/s)  best-effort-bw(kbit/s)\r\n");
#else
		vty_out (vty, "\r\n  OnuIdx   direction  class   delay   assured-bw(kbit/s)  best-effort-bw(kbit/s)\r\n");
#endif	
		/*vty_out (vty, "\r\n  %s/port%d Onu%d bandwidth Information(unit:kbit/s) \r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId), (OnuIdx+1));
		*/
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	 	{
	 		userOnuId = (short int)(ulOnuId -1);
			ShowOnulistBandwidthbyVty( phyPonId, userOnuId, vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
		return CMD_SUCCESS;
	}
    
}

#if 1
DEFUN(
		view_pon_port_statstics_show,
		view_pon_port_statstics_show_cmd,
		"show statistic pon <slot/port>",
		DescStringCommonShow
		"Show statstics information\n"
	"Show pon port statstics\n"
	"input the slot/port\n"
	)
{
	int iRes = 0;
	ULONG slotId = 0;
	ULONG port = 0;
	INT16 phyPonId = 0;
	short int PonChipType = 0;

	IFM_ParseSlotPort( argv[0], &slotId, &port );
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);	
	/* 1 板在位检查
	 if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
	 {
	 vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
	 return CMD_WARNING;
	 }
	 */
#if 0
    if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
    {
        vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
        return( CMD_WARNING );
    }
    /* 2 pon 板类型检查*/
    if(SlotCardIsPonBoard(ulSlot) != ROK )
    {
        vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
        return( CMD_WARNING );
    }

    /* pon chip is inserted ? */
    if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
    {
        vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
        return(CMD_WARNING);
    }
#else
    if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
        return CMD_WARNING;
#endif
	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}

	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[slotId], port );
		return( CMD_WARNING );
	}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		iRes = CliRealTimeStatsPonForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsPon( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}

DEFUN(
	view_CNI_statstics_show,
	view_CNI_statstics_show_cmd, 
	"show statistic CNI <slot/port>",
	DescStringCommonShow
	"Show statstics information\n"
	"Show pon-CNI statstics\n"
	"input the slot/port\n"
	)
{
	int iRes = 0;
	ULONG slotId = 0;
	ULONG port = 0;


    INT16 phyPonId = 0;
	short int PonChipType = 0;

	 IFM_ParseSlotPort( argv[0], &slotId, &port );
	 
	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查
	 if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
	 {
	 vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
	 return CMD_WARNING;
	 }
	 */
#if 0
    if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
    {
        vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
        return( CMD_WARNING );
    }
    /* 2 pon 板类型检查*/
    if(SlotCardIsPonBoard(ulSlot) != ROK )
    {
        vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
        return( CMD_WARNING );
    }

    /* pon chip is inserted ? */
    if(getPonChipInserted((unsigned char)ulSlot,(unsigned char)ulPort) != PONCHIP_EXIST)
    {
        vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
        return(CMD_WARNING);
    }
#else
    if( checkVtyPonIsValid(vty, slotId, port) != VOS_OK )
        return CMD_WARNING;
#endif

	phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}

	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[slotId], port );
		return( CMD_WARNING );
	}
	PonChipType = V2R1_GetPonchipType( phyPonId );
	if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		iRes = CliRealTimeStatsCNIForGpon( phyPonId, vty );
	}
	else
	{
		iRes = CliRealTimeStatsCNI( phyPonId, vty );
	}
	if ( VOS_OK != iRes)
	{
		vty_out( vty, "  %% Executing error\r\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;
}
/*整个gpon 口下使能所有onu by jinhl@2016.11.25*/
extern int PalDrvEnableOnuByLink(uint32 deviceId, uint32 linkId);
DEFUN(
		gpon_port_allOnu_en,
		gpon_port_allOnu_en_cmd,
		"enableOnu  pon <slot/port>",
		"enable onu"
		"pon\n"
		"input the slot/port\n"
	)
{
	ULONG slotId = 0;
	ULONG port = 0;
	int ret = 0;

	IFM_ParseSlotPort( argv[0], &slotId, &port );
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK)
		return(CMD_WARNING);	

	if(!SYS_LOCAL_MODULE_TYPE_IS_GPON)
	{
		vty_out(vty," %% slot %d is not gpon card\r\n", slotId);
        return( CMD_WARNING );
	}
	ret =PalDrvEnableOnuByLink(0,port-1);
	if(VOS_OK != ret)
	{
		vty_out(vty," %% enable all onu for pon %d err\r\n", port);
        return( CMD_WARNING );
	}
	
	return CMD_SUCCESS;
}
#endif

DEFUN(view_show_olt_address_table_configuration,
	view_show_olt_address_table_configuration_cmd,
	"show pon address-table configuration <slot/port>",
	"PON configuration\n"
	"the command gets pon address config\n"
	"address table\n"
	"the PON's address table configuration\n"
	"input the slot/port\n"
	)
{
#if 0
	short int       return_result;
	PON_address_table_config_t      address_table_config;
	bool	remove_entry_when_table_full;
#else
	int       return_result;
    OLT_addr_table_cfg_t address_table_config;
#endif
	unsigned long  ul_slot = 0, ul_port = 0;
	short int  PonPortIdx/*, PonChipType*/;
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ul_slot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return CMD_WARNING;
		}
	*/
#if 0
	if( __SYS_MODULE_TYPE__(ul_slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ul_slot);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ul_slot) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ul_slot);
		return( CMD_WARNING );
		}

	/* pon chip is inserted */
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ul_slot, ul_port);
		return(CMD_WARNING);
		}
#else
	if( checkVtyPonIsValid(vty, ul_slot, ul_port) != VOS_OK )
		return CMD_WARNING;
#endif

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
	}

#if 1
    return_result = OLT_GetAddressTableConfig(PonPortIdx, &address_table_config);
    if ( OLT_CALL_ISOK(return_result) )
    {
		WRITE_ENABLE_DISABLE_PARAMETER(address_table_config.removed_when_aged ,removed when aged)
		WRITE_ULONG_PARAMETER((address_table_config.aging_timer/SECOND_1),aging time)
		WRITE_PON_NETWORK_TRAFFIC_DIRECTION_PARAMETER(address_table_config.allow_learning,allow learning)
		WRITE_TRUE_FALSE_PARAMETER(address_table_config.discard_llid_unlearned_sa ,discard llid unlearned sa)
		if( address_table_config.discard_unknown_da > 3 )
			address_table_config.discard_unknown_da = 0;
		vty_out(vty,"discard unknown da:%s\r\n",PonLinkDirection_s[address_table_config.discard_unknown_da]);

        WRITE_FULL_MODE_BEHAVIOR_PARAMETER(address_table_config.removed_when_full, full_mode)
    }
    else
    {
        switch (return_result)
        {
            case OLT_ERR_NOTSUPPORT:
        		vty_out(vty, "  %% this is %s, this cli is not supported\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                break;
            case OLT_ERR_NOTEXIST:
        		vty_out(vty,"  %% pon%d/%d is not working\r\n", ul_slot, ul_port);
                break;
            default:
    			CLI_ERROR_PRINT
        }

		return( CMD_WARNING);
    }
#else	
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,this cli cmd is not supported\r\n");
		return( CMD_WARNING);
		}
	
	return_result = PAS_get_address_table_configuration
                                                     (PonPortIdx,
                                                      &address_table_config);

	vty_out(vty,"\r\n");
	if ( return_result ==  PAS_EXIT_OK)
	{
		/*WRITE_OLT_PARAMETER*/
		WRITE_ENABLE_DISABLE_PARAMETER(address_table_config.removed_when_aged ,removed when aged)
		WRITE_ULONG_PARAMETER((address_table_config.aging_timer/SECOND_1),aging time)
		WRITE_PON_NETWORK_TRAFFIC_DIRECTION_PARAMETER(address_table_config.allow_learning,allow learning)
		WRITE_TRUE_FALSE_PARAMETER(address_table_config.discard_llid_unlearned_sa ,discard llid unlearned sa)
		if( address_table_config.discard_unknown_da > 3 )
			address_table_config.discard_unknown_da = 0;
		vty_out(vty,"discard unknown da:%s\r\n",PonLinkDirection_s[address_table_config.discard_unknown_da]);
		/*WRITE_PON_NETWORK_TRAFFIC_DIRECTION_FALSE_PARAMETER(address_table_config.discard_unknown_da,discard unknown da)*/
	}
	else
	{
		CLI_ERROR_PRINT
	}


	return_result = PAS_get_address_table_full_handling_mode
	                                                 (PonPortIdx,
	                                                  &remove_entry_when_table_full);

	if ( return_result ==  PAS_EXIT_OK)
	{
		/*WRITE_OLT_PARAMETER*/
		WRITE_FULL_MODE_BEHAVIOR_PARAMETER(remove_entry_when_table_full ,full_mode)
	}
	else
	{
		CLI_ERROR_PRINT
	}
#endif    
	vty_out(vty,"\r\n");
	
	return CMD_SUCCESS;
}

DEFUN( host_pon_timeout_config,
	host_pon_timeout_config_cmd,
	"host-pon timeout <1-20000>",
	"config\n"
	"config host-pon communication timeout\n"
	"timeout length, stated in times\n"
	)
{
	short int Times;
	short int result;
	Times = (short int )VOS_AtoL(argv[0]);
	result = pon_set_system_parameters(-1, -1, -1, Times);
	if(result != ROK ) vty_out(vty,"  %% Executing error.\r\n" );
	return( CMD_SUCCESS );
}

DEFUN( host_pon_timeout_show,
	host_pon_timeout_show_cmd,
	"show host-pon timeout",
	"show system info\n"
	"show host-pon communication\n"
	"show host-pon communication timeout,stated in times\n"
	)
{
	short int Times;
	short int result;

	result = Pon_Get_System_parameters(NULL,NULL,NULL, &Times);
	if( result == ROK )
		vty_out(vty,"host - pon communication timeout is %d(stated in times)\r\n", Times);
	else vty_out(vty,"  %% Executing error.\r\n" );
	return( CMD_SUCCESS );
}

#if 0
DEFUN(
	link_test_num_of_frames_pon,
	link_test_num_of_frames_pon_cmd,
	"onu-link-test <1-64> number-of-frmaes <1-32766> size <56-1514> delay-measurement [disable|enable] {vlan enable vlan-priority <0-7> vlan-tag <1-4094>}*1",
	"onu link test\n"
	"onu idx\n"
	"Number of frames\n"
	"Send exact number of measurement frames\n"
	"Frame size measured in bytes\n"
	"Measurement frames data size, measured in bytes\n"
	"Link delay measurement, the difference between sent and received times\n"
	"Link test not includes a delay measurement\n"
	"Link test includes delay measurement\n"
	"VLAN tag in link test frames\n"
	"Enable VLAN tag in link test frames\n"
	"VLAN priority\n"
	"Setting VLAN priority\n"
	"VLAN tag id\n"
	"Setting VLAN tag\n"
	)
{
	short int PonPortIdx, OnuIdx;
	unsigned long ulSlot, ulPort, ulOnuid;
	unsigned long ulIfIndex;
	LONG lRet;
	
	PON_llid_t                           llid;
	short int                            number_of_frames;
	short int                            frame_size;
	bool                                 link_delay_measurement;
	PON_link_test_vlan_configuration_t   vlan_configuration;
	PON_link_test_results_t				 test_results;
	short int                            return_result;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	OnuIdx = (( INT16 ) VOS_AtoL( argv[ 0 ] )) - 1; 
	CHECK_ONU_RANGE

	if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
		{
		vty_out(vty,"onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, (OnuIdx+1));
		return(CMD_WARNING);
		}
	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(llid == INVALID_LLID )
		{
		vty_out(vty,"onu%d/%d/%d is off-line\r\n", ulSlot, ulPort, (OnuIdx+1));
		return(CMD_WARNING);
		}
	
	number_of_frames = (INT16 ) VOS_AtoL( argv[ 1 ] );
	frame_size = (INT16 ) VOS_AtoL( argv[ 2 ] );
	if( VOS_StrCmp( argv[3], "disable" ) == 0 )
		link_delay_measurement = FALSE;
	else link_delay_measurement = TRUE;

	if(argc > 4)
	{
	   vlan_configuration.vlan_frame_enable = ENABLE;
	   vlan_configuration.vlan_priority = (signed char)VOS_AtoL( argv[ 4 ] );
	   vlan_configuration.vlan_tag = (short int )VOS_AtoL( argv[ 5 ] );
	}else
	{
		vlan_configuration.vlan_frame_enable = DISABLE;
		vlan_configuration.vlan_priority = 0;
		vlan_configuration.vlan_tag = 0;		   
	}
    return_result = PAS_link_test ( PonPortIdx, llid, number_of_frames, frame_size, link_delay_measurement,
								    &vlan_configuration, &test_results );

	if( return_result == PAS_EXIT_OK)
	{
		if ( link_delay_measurement == ENABLE ) 
		{
			vty_out (vty,"Link test results:%s",VTY_NEWLINE);
			vty_out (vty,"Sent frames    : %u%s", test_results.number_of_sent_frames, VTY_NEWLINE);
			vty_out (vty,"Returned frames: %u%s", test_results.number_of_returned_frames, VTY_NEWLINE);
			vty_out (vty,"Errored frames : %u%s", test_results.number_of_errored_return_frames, VTY_NEWLINE);
			vty_out (vty,"Minimal delay  : %u TQ%s", test_results.minimal_delay, VTY_NEWLINE);
			vty_out (vty,"Mean delay     : %u TQ%s", test_results.mean_delay, VTY_NEWLINE);
			vty_out (vty,"Maximal delay  : %u TQ%s", test_results.maximal_delay, VTY_NEWLINE);
		}
		else {
			vty_out (vty,"Link test results:%s",VTY_NEWLINE);
			vty_out (vty,"Sent frames    : %u%s", test_results.number_of_sent_frames, VTY_NEWLINE);
			vty_out (vty,"Returned frames: %u%s", test_results.number_of_returned_frames, VTY_NEWLINE);
			vty_out (vty,"Errored frames : %u%s", test_results.number_of_errored_return_frames, VTY_NEWLINE);
		}
	}else
	    CLI_ERROR_PRINT
    return (return_result);
}
#endif

#ifdef  __PENDING_ONU__
#endif

void PendingOnuListHeadByVty( struct vty *vty)
{
	vty_out(vty," PON     LLID     macAddress/SN       reason\r\n");
	vty_out(vty,"----------------------------------------------\r\n");
}

DEFUN( pending_onu_list_show_pon,
        pending_onu_list_show_pon_cmd,
        "show pending-onu",
        DescStringCommonShow
        "show pending-pon list\n"
        )
{
	unsigned long ulSlot = 0;
	unsigned long ulPort = 0;
	unsigned long ulOnuid=0;
	short int PonPortIdx = 0;
	short int count = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	/*if(PonPortIsWorking(PonPortIdx) != TRUE)
		{
		vty_out(vty,"pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
		}*/
	PendingOnuListHeadByVty( vty );
		
	PendingOnuListByVty( PonPortIdx, &count, vty);

	if( count == 0 )
		vty_out(vty, "  No pending onu\r\n"); 

	return( CMD_SUCCESS );
		
}

DEFUN( active_pending_onu_pon,
        active_pending_onu_pon_cmd,
        "active pending-onu <H.H.H>",
        "active pending onu\n"
        "active pending onu\n"
        "please input mac address\n"
        )
{
	unsigned long ulSlot = 0;
	unsigned long ulPort = 0;
	unsigned long ulOnuid=0;
	short int PonPortIdx = 0;
	/*unsigned long ulIfIndex;
	long lRet;*/
	unsigned char MacAddress[BYTES_IN_MAC_ADDRESS]={0};
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	if(PonPortIsWorking(PonPortIdx) != TRUE)
		{
		vty_out(vty,"pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
		}
	
	if ( GetMacAddr( ( CHAR* ) argv[0], MacAddress ) != VOS_OK )
		{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
		}

	OnuAuth_ActivatePendingOnuByMacAddrMsg(PonPortIdx, MacAddress);
	ActivatePendingOnuMsg_conf(PonPortIdx, MacAddress);

	return(CMD_SUCCESS);	

}


/*  added by chenfj 2008-6-13
     PON 口光接收信号检测相关
     */
#if  0
#ifdef  _PON_VIRTUAL_OPTICAL_SCOPE_
/*  1:  显示PON-adc 配置参数 */
DEFUN(show_olt_adc_config,
	show_olt_adc_config_cmd,
	"show adc-config <slot/port>",
	"show config\n"
	"show pon adc-config\n"
	"input the slot/port\n"
	)
{
	unsigned long  ul_slot = 0, ul_port = 0;
	short int  PonPortIdx, PonChipType;
	LONG lRet;
	
	short int return_result;
	PON_adc_config_t  adc_config;
	
	lRet = IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	/*
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not inserted\r\n", ul_slot, ul_port );
		return CMD_WARNING;
		}
	*/
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,adc config is not supported\r\n");
		return( CMD_WARNING);
		}
	

	return_result = PAS_get_virtual_scope_adc_config(PonPortIdx, &adc_config);

	vty_out(vty,"\r\n");
	if ( return_result ==  PAS_EXIT_OK)
	{
		WRITE_ULONG_PARAMETER(adc_config.adc_bits_number,adc bits number)
		WRITE_ULONG_PARAMETER(adc_config.adc_vdd,adc vdd)
		WRITE_ULONG_PARAMETER(adc_config.adc_number_of_leading_zeros,adc number of leading zeros)
		WRITE_ULONG_PARAMETER(adc_config.adc_number_of_trailing_zeros,adc number of trailing zeros)
		WRITE_ULONG_PARAMETER(adc_config.adc_clock,adc clock)
		WRITE_ADC_POLARITY_PARAMETER(adc_config.clock_polarity,clock polarity);
		WRITE_ADC_POLARITY_PARAMETER(adc_config.cs_polarity,cs polarity);
		WRITE_ULONG_PARAMETER(adc_config.adc_measurement_start,measurement start);
		WRITE_ULONG_PARAMETER(adc_config.adc_measurement_time,measurement time);
		if(adc_config.adc_interface == PON_ADC_INTERFACE_SPI )
			vty_out(vty,"adc interface: SPI\r\n");
		else  vty_out(vty,"adc interface: I2C\r\n");
		
	}
	else
	{
		CLI_ERROR_PRINT
	}

	vty_out(vty,"\r\n");
	return( CMD_SUCCESS );

}

DEFUN(show_olt_adc_config_pon,
	show_olt_adc_config_pon_cmd,
	"show adc-config",
	"show adc-config\n"
	"show pon adc-config\n"
	)
{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid=0;
	unsigned long ulIfIndex;
	
	short int  PonPortIdx, PonChipType;
	LONG lRet;
	
	short int return_result;
	PON_adc_config_t  adc_config;
	

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	/*
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not inserted\r\n", ul_slot, ul_port );
		return CMD_WARNING;
		}
	*/
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,adc config is not supported\r\n");
		return( CMD_WARNING);
		}
	

	return_result = PAS_get_virtual_scope_adc_config(PonPortIdx, &adc_config);

	vty_out(vty,"\r\n");
	if ( return_result ==  PAS_EXIT_OK)
	{
		WRITE_ULONG_PARAMETER(adc_config.adc_bits_number,adc bits number)
		WRITE_ULONG_PARAMETER(adc_config.adc_vdd,adc vdd)
		WRITE_ULONG_PARAMETER(adc_config.adc_number_of_leading_zeros,adc number of leading zeros)
		WRITE_ULONG_PARAMETER(adc_config.adc_number_of_trailing_zeros,adc number of trailing zeros)
		WRITE_ULONG_PARAMETER(adc_config.adc_clock,adc clock)
		WRITE_ADC_POLARITY_PARAMETER(adc_config.clock_polarity,clock polarity);
		WRITE_ADC_POLARITY_PARAMETER(adc_config.cs_polarity,cs polarity);
		WRITE_ULONG_PARAMETER(adc_config.adc_measurement_start,measurement start);
		WRITE_ULONG_PARAMETER(adc_config.adc_measurement_time,measurement time);
		if(adc_config.adc_interface == PON_ADC_INTERFACE_SPI )
			vty_out(vty,"adc interface: SPI\r\n");
		else  vty_out(vty,"adc interface: I2C\r\n");
	}
	else
	{
		CLI_ERROR_PRINT
	}

	vty_out(vty,"\r\n");
	return( CMD_SUCCESS );

}

/* 2: 设置PON-adc 参数*/
DEFUN(set_olt_adc_config,
	  	set_olt_adc_config_cmd,
		"adc-config bits-number <2-31> vdd <10-50> number-of-leading-zeros <0-31> number-of-trailing-zeros <0-31> adc-clock <2-100> clock-polarity [positive|negative] cs-polarity [positive|negative] start-time <1-1000> time-len <1-50>",
		"Set ADC configuration\n"
		"Number of bits in ADC\n"
		"Setting number of bits in ADC\n"
		"VDD of ADC\n"
		"Setting VDD of ADC, stated in 0.1V\n"
		"ADC number of leading zeros\n"
		"Setting ADC number of leading zeros, stated in bits\n"
		"ADC number of trailing zeros\n"
		"Setting ADC number of trailing zeros, stated in bits\n"
		"ADC clock\n"
		"Setting ADC Clock, stated in MHz\n"
		"Clock polarity\n"
		"Positive clock polarity\n"
		"Negative clock polarity\n"
		"CS polarity\n"
		"Positive CS polarity\n"
		"Negative CS polarity\n"
		"adc measurement start time\n"
		"measurement start time, stated in TQ\n"
		"adc measurement time length\n"
		"measurement time length, stated in TQ\n"
		)
{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid=0;
	unsigned long ulIfIndex;
	
	short int  PonPortIdx, PonChipType;
	LONG lRet;
	
	PON_adc_config_t  adc_config;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ul_slot, &ul_port, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port, vty) != ROK)
		return(CMD_WARNING);
	/*
	if(getPonChipInserted((unsigned char)ul_slot,(unsigned char)ul_port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon %d/%d is not inserted\r\n", ul_slot, ul_port );
		return CMD_WARNING;
		}
	*/
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
		return CMD_WARNING;
		}
	
	PonChipType = V2R1_GetPonchipType(PonPortIdx );
	if( PonChipType == PONCHIP_PAS5001)
		{
		vty_out(vty, "  %% this is pas5001,adc config is not supported\r\n");
		return( CMD_WARNING);
		}

	adc_config.adc_bits_number = (unsigned char )VOS_AtoL(argv[0]);
	adc_config.adc_vdd = (unsigned char )VOS_AtoL(argv[1]);
	adc_config.adc_number_of_leading_zeros = (unsigned char )VOS_AtoL(argv[2]);
	adc_config.adc_number_of_trailing_zeros = (unsigned char )VOS_AtoL(argv[3]);
	adc_config.adc_clock = (unsigned char )VOS_AtoL(argv[4]);

	if(VOS_StrCmp(argv[5],"positive") == 0)
		adc_config.clock_polarity = PON_POLARITY_POSITIVE;
	else if(VOS_StrCmp(argv[5],"negative") == 0)
		adc_config.clock_polarity = PON_POLARITY_NEGATIVE;

	if(VOS_StrCmp(argv[6],"positive") == 0)
		adc_config.cs_polarity = PON_POLARITY_POSITIVE;
	else if(VOS_StrCmp(argv[6],"negative") == 0)
		adc_config.cs_polarity = PON_POLARITY_NEGATIVE;

	adc_config.adc_measurement_start = (unsigned short int )VOS_AtoL(argv[7]);
	adc_config.adc_measurement_time = (unsigned short int )VOS_AtoL(argv[8]);;
	adc_config.adc_interface = PON_ADC_INTERFACE_SPI;
		
	if(AdcConfig(PonPortIdx) == ROK ) 
		return( CMD_SUCCESS );
	else{
		vty_out(vty,"pon%d/%d config adc executing err\r\n", ul_slot, ul_port);
		return( CMD_WARNING );
		}
}

/* 3:  */
#endif
#endif

#ifdef  PRODUCT_EPON3_GFA6100_TEST
DEFUN  (
	config_add_pon_port,
	config_add_pon_port_cmd,
	"download pon-port <slot/port>",
	DescStringCommonConfig
	"add pon-port to system\n"
	"input the slot/port\n"
    )
{
	unsigned long ulSlot, ulPort;
	short int PonPortIdx;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	vty_out(vty,"download pon%d/%d...\r\n", ulSlot, ulPort);
	PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
	CHECK_PON_RANGE;
	downloadPonByVty(PonPortIdx,vty);
	return(CMD_SUCCESS);
}

DEFUN  (
	config_remove_pon_port,
	config_remove_pon_port_cmd,
	"remove pon-port <slot/port>",
	DescStringCommonConfig
	"remove pon-port from system\n"
	"input the slot/port\n"
    )
{
	unsigned long ulSlot, ulPort;
	short int PonPortIdx;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	vty_out(vty,"remove pon%d/%d...\r\n", ulSlot, ulPort);
	PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
	CHECK_PON_RANGE;
	RemovePonByVty(PonPortIdx,vty);
	return(CMD_SUCCESS);
}

DEFUN  (
	config_add_pon_card,
	config_add_pon_card_cmd,
	"reset pon-card <slot>",
	DescStringCommonConfig
	"add pon-card to system\n"
	"input the slot\n"
    )
{
	unsigned long ulSlot;

	ulSlot = (unsigned long )VOS_AtoL(argv[0]);
	if(PonCardSlotRangeCheckByVty(ulSlot,vty) == ROK )
	{
		if(ulSlot == (PON[0]))
		{
			PonCardPull(__SYS_MODULE_TYPE__(ulSlot),ulSlot);
			PonCardInserted(__SYS_MODULE_TYPE__(ulSlot),ulSlot);
			/*PonCardActivated(ulSlot);*/
		}
		else if(ulSlot == (PON[1]))
		{
			short int PonPortIdx, port;
			PonChipActivatedFlag[ulSlot] = TRUE;
			if( GetOltCardslotInserted(ulSlot) != CARDINSERT )
			{
				PonCardPull(__SYS_MODULE_TYPE__(ulSlot),ulSlot);
				PonCardInserted(__SYS_MODULE_TYPE__(ulSlot),ulSlot);
			}
			else
            {
				for(port=1; port<=PONPORTPERCARD; port++)
				{
					PonPortIdx = GetPonPortIdxBySlot(ulSlot, port);
                    
					PonChipMgmtTable[PonPortIdx].operStatus    = PONCHIP_ERR;
					PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
				}
			}
		}
	}

    return(CMD_SUCCESS);
}

DEFUN  (
	config_remove_pon_card,
	config_remove_pon_card_cmd,
	"remove pon-card <slot>",
	DescStringCommonConfig
	"remove pon-card from system\n"
	"input the slot\n"
    )
{
	unsigned long ulSlot;

	ulSlot = (unsigned long )VOS_AtoL(argv[0]);
	if(PonCardSlotRangeCheckByVty(ulSlot,vty) == ROK )
		{
		PonCardPulled(__SYS_MODULE_TYPE__(ulSlot),ulSlot);
		}
	return(CMD_SUCCESS);
}


/*硬复位整个pon芯片 by jinhl*/
DEFUN (
	reset_pon_chipHard,
	reset_pon_chipHard_cmd,
	"reset gpon-chip <slot>",
	"reset gpon-chip \n"
	"reset gpon-chip \n"
	"input the slot \n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 1;
	int PonChipPonNum;    
	int PonChipType;    
	/*LONG lRet = 0;*/
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	ulSlot = (unsigned long )VOS_AtoL(argv[0]);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}

	
	
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	
	if (phyPonId == VOS_ERROR)
	{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif   
		vty_out( vty, "  %% Parameter is error.\r\n" );

		return CMD_WARNING;
	}

    PonChipType = V2R1_GetPonchipType(phyPonId);

	if( PonChipType != PONCHIP_GPON)
	{
		vty_out(vty, "  %% this is %s card, reset pon chip is not supported\r\n", pon_chip_type2name(PonChipType));
		return( CMD_WARNING );	
	}
    
	if ( 0 != OLT_ResetPonChip(phyPonId) )
    {
		vty_out(vty, "  %% slot %d is resetting pon chip, please wait a little.\r\n", GetCardIdxByPonChip(phyPonId));
		return( CMD_WARNING );	
    }
        
	return CMD_SUCCESS;
}

/* B--added by wangjiah@2017-04-14 for ONU GT524_B*/
extern int DBA_POLLING_LEVEL;
DEFUN(
		dba_polling_level_func,
		dba_polling_level_cmd,
		"polling-level [default|factory]",
		"set dba polling level\n"
		"set dba polling level as default\n"
		"set dba polling level for factory\n"
	 )
{
	if('d' == argv[0][0])
	{
		DBA_POLLING_LEVEL = POLLING_LEVEL_DEFAULT;
	}
	else if('f' == argv[0][0])
	{
		DBA_POLLING_LEVEL = POLLING_LEVEL_FACTORY;
	}
	return CMD_SUCCESS;
}
/* E--added by wangjiah@2017-04-14 for ONU GT524_B*/

/* B--added by liwei056@2010-5-7 for 5001-Test */
static LONG s_lUpPonCardSlot = 0;
void UpdatePonCard_TimerCB(void* pData)
{
	unsigned long ulSlot = s_lUpPonCardSlot;
	unsigned long ulPort;
    short int phyPonId;

	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		return;
	}

    for (ulPort=1; ulPort<=PONPORTPERCARD; ulPort++)
    {
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
    	if(OLTAdv_IsExist(phyPonId) == TRUE )
    	{
    	    sendPonSoftUpdateMsg(phyPonId);
    	}
    }
}

DEFUN  (
	config_test_pon_card,
	config_test_pon_card_cmd,
	"update pon-card <slot> <0-100>",
	"update system\n"
	"update pon-card\n"
	"input the slot\n"
	"input the time(minutes)\n"
    )
{
    static LONG s_lUpPonCardTimerId = 0;
    /*int iRlt;*/
	unsigned long ulSlot;
	unsigned long ulTimeMS;

	ulSlot   = (unsigned long )VOS_AtoL(argv[0]);
	ulTimeMS = (unsigned long )VOS_AtoL(argv[1]) * 60000;
    
	if(PonCardSlotRangeCheckByVty(ulSlot,vty) != ROK )
    {
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return VOS_ERROR;
    }

	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return VOS_ERROR;
	}
    
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return VOS_ERROR;
	}

    s_lUpPonCardSlot = ulSlot;
    if (ulTimeMS > 0)
    {
        if (0 == s_lUpPonCardTimerId)
        {
            s_lUpPonCardTimerId = VOS_TimerCreate( MODULE_PONUPDATE, 0, ulTimeMS, UpdatePonCard_TimerCB, &ulSlot, VOS_TIMER_LOOP );
        }
        else
        {
            VOS_TimerChange(MODULE_PONUPDATE, s_lUpPonCardTimerId, ulTimeMS);                
        }
    }
    else
    {
        if (0 < s_lUpPonCardTimerId)
        {
    		if ( VOS_OK == VOS_TimerDelete( MODULE_PONUPDATE, s_lUpPonCardTimerId ) )
            {
                s_lUpPonCardTimerId = 0;
            }      
        }

        UpdatePonCard_TimerCB(&ulSlot);
    }

	return(CMD_SUCCESS);
}
/* E--added by liwei056@2010-5-7 for 5001-Test */

#endif

/*begin : add by zhengyt 09-06-03,clear pon port statistic data*/
DEFUN(clear_pon_statistic,
	clear_pon_statistic_cmd,
	"clear pon statistic",
	SHOW_STR
	"pon statistic data\n"
	"Clear pon statistic data\n"
	)
{
	/*int iRes = 0;*/
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
    	/*ULONG ulIfIndex = 0;*/
	short int phyPonId=0;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	return(ClearOltPonPortStatisticCounter(phyPonId));
		
}
/*end*/


#ifdef  _OEM_TYPE_CLI_
void  pon_init_information(void)
{
	/*show online-onu {[type] <typestring>}*1*/
	VOS_StrCpy(pon_online_onu,"Show running system information\nshow current online onu list\nonu type\nthe specific onu type-string,e.g etc\nthe specific onu type-string,e.g");
	VOS_StrCat(pon_online_onu,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(pon_online_onu,",");
	VOS_StrCat(pon_online_onu,GetDeviceTypeString(V2R1_ONU_GT831_A_CATV));
	VOS_StrCat(pon_online_onu,"etc\n");

	/*convert onu file <onuid_list>*/
	VOS_StrCpy(convert_onu1_file,"convert onu file command\nconvert onu file\nif running");
	VOS_StrCat(convert_onu1_file,&ProductCorporationName_AB[0]);
	VOS_StrCat(convert_onu1_file,"mode, then update to CTC file; and if running CTC mode, then update to ");
	VOS_StrCat(convert_onu1_file,"&ProductCorporationName_AB[0]");
	VOS_StrCat(convert_onu1_file,"file\nPlease input onuId list,e.g 1,3-5,7, etc. the range for onuId is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n");

	/*show onu-version type <typestring>*/
	VOS_StrCpy(onu_version_type,"Show running system information\nShow onu version\nonu type\nthe specific onu type-string,e.g");
	VOS_StrCat(onu_version_type,GetDeviceTypeString(V2R1_ONU_GT816));
	VOS_StrCat(onu_version_type,",");
	VOS_StrCat(onu_version_type,GetDeviceTypeString(V2R1_ONU_GT831_A_CATV));
	VOS_StrCat(onu_version_type,"etc\n");
		
}
#endif

LONG PON_show_run( ULONG ulIfindex, VOID * p )
{
	int iRes = VOS_OK; 
	short int slotId = 0;
	short int port = 0;
	short int onuId = 0;
	short int phyPonId = 0;
	int point_flag = 0;
	struct vty          *vty = p;
	unsigned char *onulist = NULL;
/*#ifdef _DISTRIBUTE_PLATFORM_
	ULONG ifIndex ;
	ULONG slot ;
	ULONG port ;
#endif*/

	vty_out( vty, "!Pon port config\r\n" );

	/* 问题单#5368  modified by chenfj 2007-9-27
	  建议将PON口保护倒换的配置保存到PON口配置下
	*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	/* added by chenfj 2007-6-25  增加PON 端口保护切换: 配置数据保存*/
	{
	unsigned int  ul_desSlot =0, ul_desPort = 0;
	unsigned int   ul_srcSlot = 0, ul_srcPort = 0;
	short int  Src_PonPort, Des_PonPort;
    int iProtectMode;
    int iSwapTriggers;
	int ret;
	
	for( ul_srcSlot = PONCARD_FIRST; ul_srcSlot <= PONCARD_LAST; ul_srcSlot ++)
		{
		if(SlotCardMayBePonBoardByVty(ul_srcSlot,NULL) != ROK ) continue;
		
		for( ul_srcPort = 1; ul_srcPort <= PONPORTPERCARD; ul_srcPort ++ )
			{

			Src_PonPort = GetPonPortIdxBySlot(ul_srcSlot, ul_srcPort);	
			if( Src_PonPort == RERROR ) continue;

            if ( V2R1_PON_PORT_SWAP_TRIGGER != (iSwapTriggers = GetPonPortHotSwapTriggers(Src_PonPort)) )
            {
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
                if ( iSwapTriggers & PROTECT_SWITCH_TRIGGER_OPTICPOWER )
                {
					vty_out( vty, " trigger-switch pon %d/%d optical-power\r\n", ul_srcSlot, ul_srcPort );
                }
#endif

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
                if ( iSwapTriggers & PROTECT_SWITCH_TRIGGER_OPTICERROR )
                {
					vty_out( vty, " trigger-switch pon %d/%d data-error\r\n", ul_srcSlot, ul_srcPort );
                }
#endif

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
                if ( iSwapTriggers & PROTECT_SWITCH_TRIGGER_UPLINKDOWN )
                {
                    if ( ROK == GetPonPortHotProtectedPort(Src_PonPort, &ul_desSlot, &ul_desPort) )
                    {
    					vty_out( vty, " trigger-switch pon %d/%d uplink-down %d/%d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
                    }
                }
#endif
            }

			ret = PonPortSwapEnableQuery( Src_PonPort );		
			if( ret == V2R1_PON_PORT_SWAP_ENABLE ) 
				{
				ret  = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
				if( ret == ROK )
					{
					Des_PonPort = GetPonPortIdxBySlot((short int)ul_desSlot, (short int)ul_desPort);
					if( Des_PonPort != RERROR ) 
						{
                        iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
                        
						if( OLT_ISREMOTE(Des_PonPort)
                            || (Src_PonPort < Des_PonPort) )
                        {
                            if (V2R1_PON_PORT_SWAP_SLOWLY == iProtectMode)
                            {
    							vty_out( vty, " auto-protect pon %d/%d partner %d/%d mode slowness\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
                            }
							/*Begin:for onu swap by jinhl@2013-02-22*/
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
                            else if(V2R1_PON_PORT_SWAP_ONU == iProtectMode)
                        	{
                        	    vty_out( vty, " auto-protect pon %d/%d partner %d/%d mode onu-optic\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
                        	}
#endif
                            /*End:for onu swap by jinhl@2013-02-22*/
                            else
                            {
    							vty_out( vty, " auto-protect pon %d/%d partner %d/%d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
                            }
                        }        
						}	
					}
				}
			}
		}

	if( V2R1_AutoProtect_Timer !=  V2R1_PON_PORT_SWAP_TIMER )
		vty_out(vty, " set pon auto-protect time %d\r\n", V2R1_AutoProtect_Timer );
	
	}
#endif

    /* 静态MAC地址表  */
    {
        int i;
        unsigned char *pbMacAddr;

        for (i=0; i<GlobalPonAddrTblNum; i++)
        {
            pbMacAddr = GlobalPonStaticMacAddr[i];
    		vty_out(vty," olt-mac add %02x%02x.%02x%02x.%02x%02x to cni\r\n", pbMacAddr[0], pbMacAddr[1], pbMacAddr[2], pbMacAddr[3], pbMacAddr[4], pbMacAddr[5]);
        }
    }

	/* 增加ONU 默认带宽设置*/
	{/*normal 1/1g*/
	if((OnuConfigDefault.UplinkBandwidth!= ONU_DEFAULT_BW) ||
		(OnuConfigDefault.UplinkBandwidthBe != ONU_DEFAULT_BE_BW) ||   
		(OnuConfigDefault.DownlinkBandwidth!= ONU_DEFAULT_BW) ||
		(OnuConfigDefault.DownlinkBandwidthBe != ONU_DEFAULT_BE_BW))  
		{
		vty_out(vty," onu default-bandwidth uplink %d %d", OnuConfigDefault.UplinkBandwidth, OnuConfigDefault.UplinkBandwidthBe);
		if((OnuConfigDefault.DownlinkBandwidth != ONU_DEFAULT_BW) ||
			(OnuConfigDefault.DownlinkBandwidthBe != ONU_DEFAULT_BE_BW))
			vty_out(vty, " downlink %d %d", OnuConfigDefault.DownlinkBandwidth, OnuConfigDefault.DownlinkBandwidthBe);
		vty_out(vty, "\r\n");
		}

	}

	{/*asym 10/1g*/
	if((OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM!= ONU_DEFAULT_BW) ||
		(OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM != ONU_DEFAULT_BE_BW) ||   
		(OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM!= GW10G_ONU_DEFAULT_BW) ||
		(OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM != GW10G_ONU_DEFAULT_BE_BW))  
		{
		vty_out(vty," onu default-bandwidth uplink %d %d", OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM, OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM);
		if((OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM != GW10G_ONU_DEFAULT_BW) ||
			(OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM != GW10G_ONU_DEFAULT_BE_BW))
			vty_out(vty, " downlink %d %d", OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM, OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM);
		vty_out(vty, " 10g/1g\r\n");
		}

	}

	{/*sym 10/10g*/
	if((OnuConfigDefault.UplinkBandwidth_XGEPON_SYM != GW10G_ONU_DEFAULT_BW) ||
		(OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM != GW10G_ONU_DEFAULT_BE_BW) ||   
		(OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM != GW10G_ONU_DEFAULT_BW) ||
		(OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM != GW10G_ONU_DEFAULT_BE_BW))  
		{
		vty_out(vty," onu default-bandwidth uplink %d %d", OnuConfigDefault.UplinkBandwidth_XGEPON_SYM, OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM);
		if((OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM != GW10G_ONU_DEFAULT_BW) ||
			(OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM != GW10G_ONU_DEFAULT_BW))
			vty_out(vty, " downlink %d %d", OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM, OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM);
		vty_out(vty, " 10g/10g\r\n");
		}

	}

	{/*gpon onu*/
	if((OnuConfigDefault.UplinkBandwidth_GPON != ONU_DEFAULT_BW) ||
		(OnuConfigDefault.UplinkBandwidthBe_GPON != ONU_DEFAULT_BE_BW) ||   
		(OnuConfigDefault.DownlinkBandwidth_GPON != GPON_ONU_DEFAULT_BW) ||
		(OnuConfigDefault.DownlinkBandwidthBe_GPON != GPON_ONU_DEFAULT_BE_BW))  
		{
		vty_out(vty," onu default-bandwidth uplink %d %d", OnuConfigDefault.UplinkBandwidth_GPON, OnuConfigDefault.UplinkBandwidthBe_GPON);
		if((OnuConfigDefault.DownlinkBandwidth_GPON != GPON_ONU_DEFAULT_BW) ||
			(OnuConfigDefault.DownlinkBandwidthBe_GPON != GPON_ONU_DEFAULT_BE_BW))
			vty_out(vty, " downlink %d %d", OnuConfigDefault.DownlinkBandwidth_GPON, OnuConfigDefault.DownlinkBandwidthBe_GPON);
		vty_out(vty, " gpon\r\n");
		}

	}

	/* added by chenfj 2007-12-19
		增加用于设置PON 下行优先级与队列之间映射*/

	{
	unsigned char changedFalg = 0;
	unsigned char i;
	unsigned char ToHigh[MAX_PRIORITY_QUEUE+1] = { 0 };
	unsigned char ToLow[MAX_PRIORITY_QUEUE+1] = { 0 };
	unsigned char ToHighFlag = 0;
	unsigned char ToLowFlag = 0;

	for( i = 0; i <= MAX_PRIORITY_QUEUE; i++)
		{
		if( PON_priority_queue_mapping_default[i]  !=  PON_priority_queue_mapping[i] )
			{
			changedFalg = 1;
			break;
			}
		}

	if( changedFalg == 1 )
		{
		for( i = 0; i <= MAX_PRIORITY_QUEUE; i++ )
			{
			if( PON_priority_queue_mapping_default[i]  !=  PON_priority_queue_mapping[i] )
				{
					if(PON_priority_queue_mapping[i] == V2R1_HIGH )
						{
						ToHigh[i] = 1;
						ToHighFlag = 1;
						}
					else {
						ToLow[i] = 1;
						ToLowFlag = 1;
						}
				}
			}
		}
	
	if( ToLowFlag == 1 )
		{
		changedFalg = 0;
		vty_out(vty," onu downlink priority-queue-mapping");
		for( i = 0; i <= MAX_PRIORITY_QUEUE; i++ )
			{
			if( ToLow[i] == 1 )
				{
				if( changedFalg == 0 )
					{
					vty_out(vty," %d", i);
					changedFalg = 1;
					}
				else vty_out(vty,",%d", i);
				}
			}
		vty_out(vty, " queue low\r\n");
		}

	if( ToHighFlag == 1 )
		{
		changedFalg = 0;
		vty_out(vty," onu downlink priority-queue-mapping");
		for( i = 0; i <= MAX_PRIORITY_QUEUE; i++ )
			{
			if( ToHigh[i] == 1 )
				{
				if( changedFalg == 0 )
					{
					vty_out(vty," %d", i);
					changedFalg = 1;
					}
				else vty_out(vty,",%d", i);
				}
			}
		vty_out(vty, " queue high\r\n");
		}			
		
	}

	/*
		added by chenfj 2008-1-9
			用于设置PON 支持的最大帧长度
	*/
	{
	if( PON_Jumbo_frame_length != PON_Jumbo_frame_length_default )
		vty_out(vty," pon jumbo length %d\r\n",PON_Jumbo_frame_length );
	}

#if ( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
    /* 恢复全局CMC设置 */
    {
        int iVlanId;
    
        GetCmcSVlanID(&iVlanId);
        if ( CMC_CFG_SVLAN_ID_DEFAULT != iVlanId )
        {
			vty_out(vty, " cmc svlan %d\r\n", iVlanId);
        }
    }
#endif


/*#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )	removed by xieshl 20110829,独立定义showrun模块
	 optical_power_show_run(vty );
#endif*/
	/*此处用来取出pon 的配置数据与默认数据之间进行比较，有差异打印出来
	注意打印格式其实是相应的设置命令。因为PON的数目多,需要一个遍历过程,
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

#if 0 /*removed by wangjiah@2017-05-11 to solve issue:38234*/
			  /* added by chenfj 2007-8-1 
				 若PON端口使能了保护倒换，在save数据时，应将主用PON 口的数据同步到备用PON口中*/
			{	
				short int PonPort_Swap;

				if( PonPortSwapPortQuery(phyPonId, &PonPort_Swap ) == ROK )
				{
					if ( OLT_ISLOCAL(PonPort_Swap) )
                    {
    					if(( PonPortHotStatusQuery(phyPonId ) == V2R1_PON_PORT_SWAP_PASSIVE ) && ( PonPortHotStatusQuery(PonPort_Swap ) == V2R1_PON_PORT_SWAP_ACTIVE ))
    						CopyAllOnuDataFromPon1ToPon2( PonPort_Swap, phyPonId );
					}
                }
			}
#endif

            /* B--added by liwei056@2012-8-8 for D15615 */
			{/* pon shutdown */
				int pon_admin;

                if ( PONPORT_DISABLE == (pon_admin = GetPonPortAdminStatus(phyPonId)) )
                {
					if(point_flag == 0)
					{	
						point_flag = 1;
						vty_out( vty, " pon %d/%d\r\n",slotId, port);
					}
					vty_out( vty, "  shutdown pon port\r\n");
                }
			}
            /* E--added by liwei056@2012-8-8 for D15615 */
		
            /*PON 端口最大允许注册onu 数目*/
        	{
                int max_onu = GetMaxOnuByPonPort(phyPonId) & 0xff;  
                if(MaxOnuDefault != max_onu)
                {
            		if (point_flag == 0)
            		{
            			point_flag = 1;
            			vty_out( vty, " pon %d/%d\r\n", slotId,port);
            		}	                                
                    vty_out(vty, "  config max-onu %d\r\n", max_onu);
                }
        	}            
			{/*onu register limit*/
				int def_flag = 0;
				int run_flag = 0;

				if( (VOS_OK == GetOnuRegisterLimitFlagDufault( phyPonId, &def_flag )) \
					&&(VOS_OK == GetOnuRegisterLimitFlag( phyPonId, &run_flag )))
				{
					
					if ( def_flag != run_flag)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						
						if (run_flag == 1)
						{
							vty_out( vty, "  register limit-onu\r\n");
						}
						else if(run_flag == 2)
						{
							vty_out( vty, "  undo register limit-onu\r\n");
						}
					}	
				}
			}
			{
			if((PonPortTable[phyPonId].discard_unlearned_sa != FALSE ) || (PonPortTable[phyPonId].table_full_handle_mode != FALSE )
				|| (PonPortTable[phyPonId].discard_unlearned_da != PON_DIRECTION_NO_DIRECTION ))
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n", slotId,port);
					}
				vty_out(vty,"  pon address-table-config discard-unlearned-sa");
				if(PonPortTable[phyPonId].discard_unlearned_sa == TRUE )
					vty_out(vty," true");
				else vty_out(vty," false");
				if(PonPortTable[phyPonId].table_full_handle_mode == TRUE )
					vty_out(vty," mac-table-full remove-entry");
				else vty_out(vty," mac-table-full keep-entry");
				if(PonPortTable[phyPonId].discard_unlearned_da == PON_DIRECTION_UPLINK)
					vty_out(vty," discard-unknown-da uplink");
				else if(PonPortTable[phyPonId].discard_unlearned_da == PON_DIRECTION_DOWNLINK)
					vty_out(vty," discard-unknown-da downlink");
				else if(PonPortTable[phyPonId].discard_unlearned_da == PON_DIRECTION_UPLINK_AND_DOWNLINK)
					vty_out(vty," discard-unknown-da all-direction");
				vty_out(vty,"\r\n");
				}

			}
		
			{/*added onu cli*/
				char runMacAddr[6] = {0};
				int runlen = 0;
				char defMacAddr[6] = {0};
				int deflen = 0;
				iRes = GetOnuMacAddrDefault( defMacAddr, &deflen);
				if (iRes == VOS_ERROR)
					continue;
				/*vty_out(vty,"  defmacaddr: 	%x.%x.%x.%x.%x.%x\r\n",defMacAddr[0],defMacAddr[1],defMacAddr[2],defMacAddr[3],	
				defMacAddr[4],defMacAddr[5]);*/
				/*CLI_ONU_SYSCLE*/
				CLI_ONU_SYSCLE
				{
                    if(ThisIsGponOnu(phyPonId, onuId) == TRUE)
                    {
                        char sn[GPON_ONU_SERIAL_NUM_STR_LEN] = {0};
						char g_localname[MAXDEVICENAMELEN];
                        int len = 0,namelen=0;
						VOS_MemZero(g_localname,MAXDEVICENAMELEN);
                        GetOnuSerialNo(phyPonId, onuId, sn, &len);
                    	GetOnuDeviceName(phyPonId, onuId,g_localname, &namelen);
							
                        if (point_flag == 0)
                        {
                            point_flag = 1;
                            vty_out( vty, " pon %d/%d\r\n", slotId,port);
                        }
                    
                        if(len == (GPON_ONU_SERIAL_NUM_STR_LEN-1))
							{vty_out(vty, " add gonu %d %16s %s\r\n", onuId+1, sn ,g_localname);}
                    }
                    else if ( (VOS_OK == GetOnuMacAddr( (short int )phyPonId, onuId, runMacAddr,&runlen)) \
                    && (VOS_OK != memcmp(runMacAddr, defMacAddr,6)))
                    {
                    	int len = 0,namelen=0;
						char localname[MAXDEVICENAMELEN];
						VOS_MemZero(localname,MAXDEVICENAMELEN);
                    	GetOnuDeviceName(phyPonId, onuId,localname, &namelen);

                        if (point_flag == 0)
                        {
                            point_flag = 1;
                            vty_out( vty, " pon %d/%d\r\n", slotId,port);
                        }

		   						vty_out (vty, "  add onu %d %02x%02x.%02x%02x.%02x%02x %s\r\n",onuId+1,\
                        runMacAddr[0],runMacAddr[1],runMacAddr[2],runMacAddr[3],\
                        				runMacAddr[4],runMacAddr[5],localname);
#ifdef CLI_OLT_DEBUG
                       				sys_console_printf("  add onu %d %d%d.%d%d.%d%d.%d%d %s\r\n",onuId+1,\
                        runMacAddr[0],runMacAddr[1],runMacAddr[2],runMacAddr[3],\
                       				 runMacAddr[4],runMacAddr[5],localname);
		   						#endif
					 	}
				}/*end of CLI_ONU_SYSCLE*/			
			}
			
			
			/*	modified by chenfj 2007-9-25
				ONU加密，密钥更新时间在show run中保存时保存了错误的
				字符串"encrypt-keytime"（不是配置更新密钥时间的命令），
				导致配置数据未能恢复
			*/
			/*设置密钥交换时间
			{
				unsigned int def_timeLen = 0;
				unsigned int run_timeLen = 0;
				if ((VOS_OK == GetPonPortEncryptKeyTimeDefault(&def_timeLen)) \
					&& (VOS_OK == GetPonPortEncryptKeyTime(phyPonId, &run_timeLen)) )
				{

					if (def_timeLen != run_timeLen)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						vty_out( vty, "  encrypt update-key-time %d\r\n",run_timeLen);
						#ifdef CLI_OLT_DEBUG
						sys_console_printf("  def_timeLen != run_timeLen\r\n");
						sys_console_printf("  run_timeLen: %s\r\n", run_timeLen);
						sys_console_printf("  def_timeLen: %s\r\n", def_timeLen);
						#endif
					}
				}
			}
			*/
			#if 0
			/*encrypt direiction*/
			{
				unsigned int def_cry = 0;
				unsigned int run_cry = 0;
				if ((VOS_OK == GetPonPortEncryptDefault( &def_cry)) \
					&& (VOS_OK == GetPonPortEncrypt(phyPonId, &run_cry)) )
				{

					if (def_cry != run_cry)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						if (PON_ENCRYPTION_DIRECTION_ALL == run_cry)
							vty_out( vty, "  encrypt up-down\r\n");
						else if (PON_ENCRYPTION_DIRECTION_DOWN == run_cry )
							vty_out( vty, "  encrypt down\r\n");
						/*
						else if (PON_ENCRYPTION_PURE == run_cry )
							vty_out( vty, "  encrypt disable\r\n");
						*/
					} 
				}
			}
			#endif

 			
			#if 0
			/*PON端口带宽分配方式 */
			{
				int def_dbaMode = 0;
				int run_dbaMode = 0;
				
				if ((VOS_OK == GetPonPortBWModeDefault( &def_dbaMode)) \
					&& (VOS_OK == GetPonPortBWMode(phyPonId, &run_dbaMode)))
				{
					if( def_dbaMode != run_dbaMode)
					{
					if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
					if ( EPON_SBA == run_dbaMode )
						vty_out( vty, "  bandwidth alloc-mode sba\r\n");
					else if ( EPON_DBA == run_dbaMode)
						vty_out( vty, "  bandwidth alloc-mode dba\r\n");
					}
				}
			} 
			#endif

			#if 0
			/*3.2.5	设置PON端口下某个/所有逻辑链接支持的最大MAC数*/
			{
				unsigned int def_maxMac = 0;
				unsigned int run_maxMac = 0;
				def_maxMac = (unsigned int)MaxMACDefault;
						
				CLI_ONU_SYSCLE
				{
				 if (VOS_OK == GetOnuMaxMacNum( (short int )phyPonId, onuId,  &run_maxMac)) 
			        {
					  if ( def_maxMac != run_maxMac )
					  	{
							if(point_flag == 0)
							{	
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
							vty_out( vty, "  max-mac %d logical-link %d\r\n",run_maxMac, (onuId+1));
					  	}
				  }
				}/*end of CLI_ONU_SYSCLE*/
			}
			#endif
			#if 0
			/*3.2.6	配置PON端口下动态MAC地址自学习使能*/
			{
				unsigned int def_ctrlValue = 0;
				unsigned int run_ctrlValue = 0;
				if ((VOS_OK == GetPonPortMacAutoLearningCtrDefaultl( &def_ctrlValue)) \
					&& (VOS_OK == GetPonPortMacAutoLearningCtrl(phyPonId, &run_ctrlValue)) )
				{
					if (def_ctrlValue != run_ctrlValue)
					{	
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						if (run_ctrlValue == CLI_EPON_LEARNING_ENABLE)
						{
							vty_out( vty, "  mac-learning enable\r\n");
						}
						else if ( run_ctrlValue == CLI_EPON_LEARNING_DISABLE)
						{
							vty_out (vty, "  mac-learning disable\r\n");
						}
					}
				}
			}
			#endif
			/*3.2.7	设置PON端口下动态MAC地址老化时间*/
			{
				unsigned int def_ageingTime = 0;
				unsigned int run_ageingTime = 0;
					
				if ((VOS_OK == GetPonPortMacAgeingTimeDefault(&def_ageingTime)) \
					&& (VOS_OK == GetPonPortMacAgeingTime(phyPonId,&run_ageingTime)) )
				{
					if (def_ageingTime != run_ageingTime)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						vty_out( vty,"  aging-mac-time %d\r\n", run_ageingTime/1000);
					}	
				}
			}
			
			/*3.2.8	设置PON端口下某个LLID允许接入的静态MAC */
			{

			}

        	{/*设置downlink policer*/
        		int def_flag = 0;
        		int run_flag = 0;
        		def_flag = GetOnuPolicerFlag();
        		run_flag = GetOltPolicerFlag(phyPonId);
        		if ( def_flag != run_flag )
        		{			
					if(point_flag == 0)
					{	
						point_flag = 1;
						vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
                    
        			if ( run_flag == V2R1_ENABLE )
        			{
        				vty_out( vty, "  onu downlink-policer\r\n" );
        			}
        			else if ( run_flag == V2R1_DISABLE )
        			{
        				vty_out( vty, "  undo onu downlink-policer\r\n" );
        			}
        		}     
        	}
			
			/*3.2.9	设置PON端口下某个/所有ONU带宽*/
			{
#if 0
				unsigned int def_downlinkClass;
				unsigned int def_uplinkClass;
				unsigned int def_downlinkdelay;
				unsigned int def_uplinkdelay;
				unsigned int def_downlinkasBw;
				unsigned int def_uplinkasBw;
				unsigned int def_downlink_beBw;
				unsigned int def_uplink_beBw;
#endif                
				unsigned int run_downlinkClass;
				unsigned int run_uplinkClass;
				
				unsigned int run_downlinkdelay;
				unsigned int run_uplinkdelay;
				
				unsigned int run_uplinkfxBw;
				unsigned int run_uplinkasBw;
				unsigned int run_uplink_beBw;
                
				unsigned int run_downlinkasBw;
				unsigned int run_downlink_beBw;

				int BandWidthIsDefault;

#if 0
				if (VOS_OK != GetOnuUplinkBWDefault_1(&def_uplinkClass, &def_uplinkdelay,
					&def_uplinkasBw, &def_uplink_beBw))
					continue;
				if(VOS_OK != GetOnuDownlinkBWDefault_1(&def_downlinkClass, &def_downlinkdelay,
					&def_downlinkasBw, &def_downlink_beBw))
					continue;
#endif                
			
				CLI_ONU_SYSCLE
				{
                    if(ThisIsGponOnu(phyPonId, onuId) == TRUE)
                    {
                        /*do nothing*/
                    }
					else if (VOS_OK != cliCheckOnuMacValid( phyPonId, onuId ))
						continue;

					BandWidthIsDefault = Onu_IsDefaultBWSetting( phyPonId, onuId );
                    
					/*downlink*/
                    /* B--modified by liwei056@2011-1-27 for DefaultBW */
#if 1					
					if ( !(OLT_CFG_DIR_DOWNLINK & BandWidthIsDefault) )
#else
					if ((run_downlinkClass != def_downlinkClass) || 
						(run_downlinkdelay != def_downlinkdelay) ||
						(run_downlinkasBw != def_downlinkasBw) ||
						(run_downlink_beBw != def_downlink_beBw))
#endif
                    /* E--modified by liwei056@2011-1-27 for DefaultBW */
					{
    					if (VOS_OK == GetOnuDownlinkBW_1(phyPonId, onuId,&run_downlinkClass, &run_downlinkdelay,
    													&run_downlinkasBw, &run_downlink_beBw))
						{
    						if( !((run_downlinkasBw == 0) 
                                || (GetOltPolicerFlag(phyPonId) == V2R1_DISABLE)) )
    						{
    							if(point_flag == 0)
    							{	
    								point_flag = 1;
    								vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
    							}
    							
    							if (CLIE_BANDWITHDELAY_HIGH == run_downlinkdelay)
    								vty_out( vty, "  bandwidth class %d delay high assured-bw %d best-effort-bw %d down %d\r\n",
    										run_downlinkClass, run_downlinkasBw, run_downlink_beBw, (onuId+1) );
    							else if (CLIE_BANDWITHDELAY_LOW == run_downlinkdelay)
    								vty_out( vty, "  bandwidth class %d delay low assured-bw %d best-effort-bw %d down %d\r\n",
    										run_downlinkClass, run_downlinkasBw, run_downlink_beBw, (onuId+1) );
    						}
						}
					}

					/*uplink*/
#ifndef PLATO_DBA_V3
                    /* B--modified by liwei056@2011-1-27 for DefaultBW */
#if 1					
					if ( !(OLT_CFG_DIR_UPLINK & BandWidthIsDefault) )
#else
					if ((run_uplinkClass != def_uplinkClass) || 
						(run_uplinkdelay != def_uplinkdelay) ||
						(run_uplinkasBw != def_uplinkasBw) ||
						(run_uplink_beBw != def_uplink_beBw))
#endif
                    /* E--modified by liwei056@2011-1-27 for DefaultBW */
					{
    					if (VOS_OK == GetOnuUplinkBW_1(phyPonId, onuId,&run_uplinkClass, &run_uplinkdelay,
    													&run_uplinkasBw, &run_uplink_beBw))
						{
							if(point_flag == 0)
							{	
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
							if (CLIE_BANDWITHDELAY_HIGH == run_uplinkdelay)
								vty_out( vty, "  bandwidth class %d delay high assured-bw %d best-effort-bw %d up %d\r\n",
										run_uplinkClass, run_uplinkasBw, run_uplink_beBw, (onuId+1) );
							else if (CLIE_BANDWITHDELAY_LOW == run_uplinkdelay)
								vty_out( vty, "  bandwidth class %d delay low assured-bw %d best-effort-bw %d up %d\r\n",
										run_uplinkClass, run_uplinkasBw, run_uplink_beBw, (onuId+1) );
						}													
					}
#else
                    /* B--modified by liwei056@2011-1-27 for DefaultBW */
#if 1					
					if ( !(OLT_CFG_DIR_UPLINK & BandWidthIsDefault) )
#else
					if ((run_uplinkClass != def_uplinkClass) || 
						(run_uplinkdelay != 0) ||
						(run_uplinkasBw != def_uplinkasBw) ||
						(run_uplink_beBw != def_uplink_beBw))
#endif
                    /* E--modified by liwei056@2011-1-27 for DefaultBW */
					{
                        /* B--modified by liwei056@2011-12-15 for D14182 */
#if 0
        				if (VOS_OK == GetOnuUplinkBW_2(phyPonId, onuId,&run_uplinkClass, &run_uplinkdelay,
        													&run_uplinkasBw, &run_uplink_beBw))
#else
        				if (VOS_OK == GetOnuUplinkBW_3(phyPonId, onuId,&run_uplinkClass, &run_uplinkdelay,
        													&run_uplinkfxBw, &run_uplinkasBw, &run_uplink_beBw))
#endif
                        /* E--modified by liwei056@2011-12-15 for D14182 */
						{
							if(point_flag == 0)
							{	
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
                            
                            /* B--modified by liwei056@2011-12-15 for D14182 */
#if 0
							vty_out( vty, "  bandwidth class %d delay low fixed-bw %d assured-bw %d best-effort-bw %d up %d\r\n",
										run_uplinkClass, run_uplinkdelay, run_uplinkasBw, run_uplink_beBw, (onuId+1) );
#else
							if (CLIE_BANDWITHDELAY_HIGH == run_uplinkdelay)
								vty_out( vty, "  bandwidth class %d delay high fixed-bw %d assured-bw %d best-effort-bw %d up %d\r\n",
										run_uplinkClass, run_uplinkfxBw, run_uplinkasBw, run_uplink_beBw, (onuId+1) );
							else if (CLIE_BANDWITHDELAY_LOW == run_uplinkdelay)
								vty_out( vty, "  bandwidth class %d delay low fixed-bw %d assured-bw %d best-effort-bw %d up %d\r\n",
										run_uplinkClass, run_uplinkfxBw, run_uplinkasBw, run_uplink_beBw, (onuId+1) );
#endif
                            /* E--modified by liwei056@2011-12-15 for D14182 */
						}													
					}
#endif
				}/*end of CLI_ONU_SYSCLE*/
			}
			#if 0
			{
				unsigned int def_uplinkBw = 0;
				unsigned int run_uplinkBw = 0;
				unsigned int def_downlinkBw = 0;
				unsigned int run_downlinkBw = 0;
				if(VOS_OK != GetOnuUplinkBWDefault(&def_uplinkBw))
				{
					if (point_flag == 1)
						{
						point_flag = 0;
						vty_out( vty, " exit\r\n" );
						}
					continue;
				}
				if (VOS_OK != GetOnuDownlinkBWDefault(&def_downlinkBw))
				{
					if (point_flag == 1)
						{
						point_flag = 0;
						vty_out( vty, " exit\r\n" );
						}
					continue;
				}
				
				CLI_ONU_SYSCLE
				{
					if(OnuMgmtTable[phyPonId*MAXONUPERPON+onuId].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )
						continue;
						
					if (VOS_OK != GetOnuDownlinkBW(phyPonId, onuId, &run_downlinkBw))
					{
						#if 0/*因此处无节点切换,固以下代码可删除,否则会有错误*/
						if (point_flag == 1)
						{
						point_flag = 0;
						/*vty_out( vty, " exit\r\n" );deleted by chenfj 2006/10/31*/
						}
						#endif
						goto UplinkBw;
					}
					else
					{
						if (def_uplinkBw != run_downlinkBw)
						{
							if(point_flag == 0)
							{	

							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
							vty_out( vty, "  bandwidth %d down %d\r\n",run_downlinkBw, (onuId+1));
						}
					}
					
				UplinkBw:					
					if (VOS_OK != GetOnuUplinkBW(phyPonId, onuId, &run_uplinkBw))
					{
						#if 0/*因此处无节点切换,固以下代码可删除,否则会有错误*/
						if (point_flag == 1)
						{
						point_flag = 0;
						/*vty_out( vty, " exit\r\n" );deleted by chenfj 2006/10/31 */
						}
						#endif
						continue;
					}
					
					if (def_downlinkBw != run_uplinkBw)
					{
						if(point_flag == 0)
						{	

							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						vty_out( vty, "  bandwidth %d up %d\r\n",run_uplinkBw,(onuId+1));
					}	
				}/*end of CLI_ONU_SYSCLE*/
			}
			#endif
			#if 0
			/*3.2.10	配置PON芯片EPON接口保护倒换使能*/
			{
				unsigned char def_aps = 0;
				unsigned char run_aps = 0;
					
				def_aps = GetPonPortApsCtrlDefault();
				run_aps = GetPonPortApsCtrl(phyPonId);
				if (def_aps != run_aps)
				{
					if(point_flag == 0)
					{	
						point_flag = 1;
						vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
					if (CLI_EPON_AUTO == run_aps)
					{
						vty_out( vty, "  aps auto\r\n");
					}
					else if (CLI_EPON_FORCE == run_aps)
					{
						vty_out( vty, "  aps forced\r\n");
					}
					else if (CLI_EPON_DISABLE == run_aps)						{
						vty_out( vty, "  aps disable\r\n");
					}
				}
			}
			#endif
			#if 0
			/*历史统计的周期统计数目*/
			{
				unsigned int def_bucket_15m = 0;
				unsigned int def_bucket_24h = 0;
				unsigned int run_bucket_15m = 0;
				unsigned int run_bucket_24h = 0;
				if (VOS_OK == HisStatsDefaultRecordGet(&def_bucket_15m, &def_bucket_24h))
				{

					HisStats24HoursMaxRecordGet(&run_bucket_24h);
					HisStats15MinMaxRecordGet(&run_bucket_15m);

					if (def_bucket_15m != run_bucket_15m)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						vty_out( vty, "  statistic-history bucket-num 15m %d\r\n",run_bucket_15m);
					}
					if (def_bucket_24h != run_bucket_24h)
					{
						if(point_flag == 0)
						{	
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						vty_out( vty, "  statistic-history bucket-num 24h %d\r\n",run_bucket_24h);
					}
				}
			}
			#endif
			{/*onu auto update cmd*/
			
			}
			
			{/*history-statistics*/
				unsigned int status15m = 0;
				unsigned int status24h = 0;
				iRes = CliHisStatsPonStatusGet( phyPonId, &status15m, &status24h);
				if (iRes == VOS_OK)
				{
					if(point_flag == 0)
					{	
						point_flag = 1;
						vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
					if ((status15m == 1) && (status24h == 1))
						vty_out( vty, "  statistic-history pon\r\n");
					else 
					{
						if (status15m == 1)
						vty_out( vty, "  statistic-history pon 15m\r\n");

						if (status24h == 1)
						vty_out( vty, "  statistic-history pon 24h\r\n");
					}
				}
			}
			 
			/*fe be告警门限*/
				
			/*{
				unsigned int run_berAlmEn = 0;
				unsigned int run_ferAlmEn = 0;
				if (VOS_OK != monPonBerAlmEnGet(phyPonId, &run_berAlmEn))
					return VOS_ERROR;
				if (VOS_OK != monPonFerAlmEnGet(phyPonId, 0, &run_ferAlmEn))
			}*/




#ifdef ONU_PEER_TO_PEER		
			{/*peer to peer*/
				short int srcOnuId = 0;
				short int dstOnuId = 0;
				int iRes = 0;
				for( srcOnuId = 0; srcOnuId < MAXONUPERPON; srcOnuId++)
				{
					for( dstOnuId = 0; dstOnuId < MAXONUPERPON; dstOnuId++)
					{
						if (srcOnuId == dstOnuId)
							continue;
						if ( srcOnuId > dstOnuId )
							continue;
						iRes = GetOnuPeerToPeer( phyPonId, srcOnuId, dstOnuId );
						if ((CLI_EPON_P2P_ENABLE != iRes) && (CLI_EPON_P2P_DISABLE != iRes))
							continue;

						if (point_flag == 0)
						{
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
						if (CLI_EPON_P2P_ENABLE == iRes)
							vty_out( vty, "  onu p2p %d %d\r\n", (srcOnuId+1), (dstOnuId+1) );
						/*else if (CLI_EPON_P2P_DISABLE == iRes)
							vty_out( vty, "  undo onu p2p %d %d\r\n", (srcOnuId+1), (dstOnuId+1) );*/
						else 
							continue;
					}
				}
			}
#endif	

/* modified by xieshl 20160421, 支持超长光纤需求 */
#ifdef ONU_RANGE_WINDOW_PON_CLI
			{/*messuring distance*/
				int run_range = 0;
				int def_range = 0;
				char *str = NULL;
				if( (VOS_OK == GetPonRange( phyPonId, &run_range ))&& \
					(VOS_OK == GetPonRangeDefault( &def_range )) )
				{
					if (run_range != def_range)
					{
						if (run_range == PON_RANGE_20KM)
							str = "20km";
						else if (run_range == PON_RANGE_40KM)
							str = "40km";
						else if (run_range == PON_RANGE_60KM)
							str = "60km";
						else if (run_range == PON_RANGE_80KM)
							str = "80km";
						else if( run_range == PON_RANGE_CLOSE )
							str = "close";
						if( str )
						{
							if (point_flag == 0)
							{
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
							vty_out( vty, "  onu-register window %s\r\n", str);
						}
					}
				}
			}				
#endif

#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
			/* added by chenfj 2007-6-25  增加PON 端口保护切换: 配置数据保存*/
#if 0
			{
			unsigned int  ul_desSlot =0, ul_desPort = 0;
			unsigned int   ul_srcSlot = 0, ul_srcPort = 0;
			short int  Src_PonPort, Des_PonPort;
			int ret;

			Src_PonPort = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
			
			ret = PonPortSwapEnableQuery( Src_PonPort );
			if( ret == V2R1_PON_PORT_SWAP_ENABLE ) 
				{
				ul_srcSlot = slotId; 
				ul_srcPort = port; 
			
				ret  = PonPortAutoProtectPortQuery( ul_srcSlot, ul_srcPort, &ul_desSlot, &ul_desPort );
				if( ret == ROK )
					{
					Des_PonPort = GetPonPortIdxBySlot((short int)ul_desSlot, (short int)ul_desPort);
					if( Des_PonPort != RERROR ) 
						{						
						if (point_flag == 0)
							{
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
						vty_out(vty,"  auto-protect partner %d/%d\r\n", ul_desSlot, ul_desPort );
						}	
					}
				}
			
			}
#endif
#endif
			/* added by chenfj 2007-6-27  
		     PON 端口管理使能: up / down  

			{
			unsigned long Slot, Port;

			Slot = GetCardIdxByPonChip( phyPonId );		
			Port = GetPonPortByPonChip(phyPonId);

			if( PonPortTable[phyPonId].PortAdminStatus == V2R1_DISABLE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",Slot, Port) ;
					}
				vty_out( vty, "  shutdown pon port\r\n");
				}
			}
			*/

			/* added by chenfj 2007-7-9
			   ONU  认证使能*/
			{
			unsigned long Slot, Port, Entry=0;
			unsigned long NextSlot, NextPort, NextEntry;
			unsigned long length;
			unsigned char MacAddr[6];
            ULONG mode = 0;
			STATUS ret;

			Slot = GetCardIdxByPonChip( phyPonId );		
			Port = GetPonPortByPonChip(phyPonId);
            mn_getCtcOnuAuthMode(Slot, Port, &mode);            
            if(mode != mn_ctc_auth_mode_mac)
            {
                if (point_flag == 0)
                {
                    point_flag = 1;
                    vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
                }
                vty_out( vty, "  onu-register authentication mode %s\r\n", onu_auth_mode_to_str(mode));
            }
            ret = getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
	
			if (( ret == VOS_OK ) && ((Slot == NextSlot ) && ( Port == NextPort )))
				{			
				do{
					if (point_flag == 0)
						{
						point_flag = 1;
						vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
						}
					Entry = NextEntry;
					ret = getOnuAuthMacAddress( Slot, Port, Entry, MacAddr, &length );
					if( ThisIsInvalidMacAddr(MacAddr) != TRUE )
						vty_out( vty, "  add onu-register authentication entry %02x%02x.%02x%02x.%02x%02x\r\n", MacAddr[0],MacAddr[1],MacAddr[2],MacAddr[3],MacAddr[4],MacAddr[5] );
					ret =  getNextOnuAuthEntry( Slot, Port, Entry, &NextSlot, &NextPort, &NextEntry );
					
					}while(( ret == VOS_OK ) && ( Slot == NextSlot ) && ( Port == NextPort ) );
				}

			}
            /*增加loid 认证表showrun ; added by luh 2012-2-10*/
            {
                /*modi by luh@2016-6-1 for Q.29825*/
                if(onu_auth_loidlist_showrun(vty, slotId, port, point_flag))
                    point_flag = 1;
            }

            /*gpon onu认证表show run*/
            {
                /*modi by luh@2016-6-1 for Q.29825*/            
                if(gpon_onu_auth_showrun(vty, slotId, port, point_flag))
                    point_flag = 1;
            }
			/* added by chenfj 2007-8-1 
			     PON 端口BER/FER告警设置*/
			{
			unsigned int BerAlarmEnable, FerAlarmEnable;
			
			monOnuBerAlmEnGet( phyPonId, &BerAlarmEnable );
			if( BerAlarmEnable != V2R1_ENABLE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
				vty_out(vty, "  onu-pon ber alarm disable\r\n");
				}

			monOnuFerAlmEnGet( phyPonId, &FerAlarmEnable );
			if( FerAlarmEnable != V2R1_ENABLE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
				vty_out(vty, "  onu-pon fer alarm disable\r\n");
				}

			monPonBerAlmEnGet( phyPonId, &BerAlarmEnable );
			if( BerAlarmEnable != V2R1_ENABLE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
				vty_out(vty, "  olt-pon ber alarm disable\r\n");
				}

			monPonFerAlmEnGet( phyPonId, &FerAlarmEnable );
			if( FerAlarmEnable != V2R1_ENABLE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
				vty_out(vty, "  olt-pon fer alarm disable\r\n");
				}
			}

			/* added by chenfj 2007-9-5 
				增加PAS5201芯片在MAC地址满后，新进来的MAC地址的处理模式
			{
			unsigned char  Handling_mode;
			Handling_mode = PonPortTable[phyPonId].table_full_handle_mode;

			if( Handling_mode == TRUE )
				{
				if (point_flag == 0)
					{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
					}
				
				vty_out(vty, "  pon address-table full-handling-mode remove-entry\r\n");
				}
			}*/			

			/* added by chenfj 2008-3-28
			   保存ONU 上行数据帧优先级设置
			  */
			{
			PON_olt_vlan_uplink_config_t    vlan_uplink_config ;
			unsigned char ConfigFlag= V2R1_DOWN;
			
			CLI_ONU_SYSCLE
				{
				if(ThisIsValidOnu(phyPonId, onuId) != ROK ) continue;
				if(GetOnuUplinkVlanPriority(phyPonId, onuId, &ConfigFlag, &vlan_uplink_config) == ROK )
					{
					if(ConfigFlag == V2R1_UP )
						{
						if (point_flag == 0)
							{
							point_flag = 1;
							vty_out( vty, " pon %d/%d\r\n",slotId, port) ;
							}
						/*vty_out(vty,  "  onu-uplink-priority %d vlan-id %d vlan-type %s priority %d\r\n",(onuId+1),vlan_uplink_config.new_vlan_tag_id,v2r1EthType[vlan_uplink_config.vlan_type],vlan_uplink_config.vlan_priority);*/
						vty_out(vty,  "  onu uplink-priority %d vlan-id %d priority %d\r\n",(onuId+1),vlan_uplink_config.new_vlan_tag_id,vlan_uplink_config.vlan_priority);
						}	
					}
				}
			}

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
			/* added by chenfj 2009-6-18
				PON 端口用户自定义vlan-tpid*/
			if(PonPortTable[phyPonId].vlan_tpid != DEFAULT_PON_VLAN_TPID)
			{
				if (point_flag == 0)
				{
					point_flag = 1;
					vty_out( vty, " pon %d/%d\r\n", slotId,port);
				}
				vty_out(vty, "  pon-vlantpid user-defined 0x%x\r\n", PonPortTable[phyPonId].vlan_tpid);
			}
			
			/*  added by chenfj 2008-8-26
				上行ONU 业务VLAN 转换*/
			{
				#define ONULISTLEN 256
				unsigned char tempBuf[512] = {0};
				unsigned char *tempPtr;
				short int length;
				short int vlan_mgt;
				Olt_llid_vlan_manipulation  *CurrentPtr;
				short int OnuListNum = 0;
				short int groupId = 0;

				

				/*modified by liyang @2015-06-03 for pon uplink qinq combine onu ID*/
#if 1
				if(onulist == NULL)
				{
					onulist = ( VOID * )VOS_Malloc(MAXONUPERPON*ONULISTLEN, MODULE_RPU_CLI);
				}
				
#endif			
				if(onulist != NULL)
				{
					VOS_MemSet(onulist, 0, MAXONUPERPON*ONULISTLEN);
				
					GetPonQinQRuleOnuList(phyPonId,&OnuListNum, onulist);
					
					tempPtr = &tempBuf[0];
					for(groupId =0; groupId < OnuListNum; groupId++)
					{
						onuId =  *(onulist + groupId*ONULISTLEN) - 1;

						VOS_MemSet(tempPtr, 0, sizeof(tempBuf));

						if( PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION == (vlan_mgt = PonPortTable[phyPonId].Uplink_vlan_manipulation[onuId].vlan_manipulation) ) continue;

		                CurrentPtr = &PonPortTable[phyPonId].Uplink_vlan_manipulation[onuId];

						
						if(PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED == vlan_mgt)
						{
							if (point_flag == 0)
							{
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n", slotId,port);
							}
							tempBuf[0] = 0;
							VOS_Sprintf(&tempBuf[0],"  uplink vlan-tag-exchange %s", (onulist + groupId*ONULISTLEN + 1));
							length = VOS_StrLen(tempBuf);
							
							if(CurrentPtr->original_vlan_id == NULL_VLAN)
								VOS_Sprintf(&tempBuf[length]," null-tagged %d", CurrentPtr->new_vlan_id);
							else if(CurrentPtr->original_vlan_id == VLAN_UNTAGGED_ID)
								VOS_Sprintf(&tempBuf[length]," untagged %d", CurrentPtr->new_vlan_id);
							else if(CurrentPtr->original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
								VOS_Sprintf(&tempBuf[length]," all %d", CurrentPtr->new_vlan_id);
							else 
								VOS_Sprintf(&tempBuf[length]," %d %d", CurrentPtr->original_vlan_id, CurrentPtr->new_vlan_id);
							length = VOS_StrLen(tempBuf);
							
							if(CurrentPtr->new_priority == PON_VLAN_ORIGINAL_PRIORITY)
								VOS_Sprintf(&tempBuf[length]," original");
							else 
								VOS_Sprintf(&tempBuf[length]," %d", CurrentPtr->new_priority);
							length = VOS_StrLen(tempBuf);
							
							if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
								VOS_Sprintf(&tempBuf[length]," 0x8100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
								VOS_Sprintf(&tempBuf[length]," 0x9100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
								VOS_Sprintf(&tempBuf[length]," 0x88a8\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE )
								VOS_Sprintf(&tempBuf[length]," user-defined\r\n");
							
							vty_out(vty,"%s",tempPtr);
						}
						
						else if(PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED == vlan_mgt)
						{
							if (point_flag == 0)
							{
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n", slotId,port);
							}
							tempBuf[0] = 0;

							
							VOS_Sprintf(&tempBuf[0],"  uplink vlan-tag-add %s", (onulist + groupId*ONULISTLEN + 1));
							length = VOS_StrLen(tempBuf);
			
							if(CurrentPtr->original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
								VOS_Sprintf(&tempBuf[length]," inter-vid all");
							else 
								VOS_Sprintf(&tempBuf[length]," inter-vid %d", CurrentPtr->original_vlan_id);
							length = VOS_StrLen(tempBuf);

							VOS_Sprintf(&tempBuf[length]," outer-vid %d priority", CurrentPtr->new_vlan_id);
							length = VOS_StrLen(tempBuf);

							if(CurrentPtr->new_priority == PON_VLAN_ORIGINAL_PRIORITY)
								VOS_Sprintf(&tempBuf[length]," original tpid");
							else VOS_Sprintf(&tempBuf[length]," %d tpid", CurrentPtr->new_priority);
							length = VOS_StrLen(tempBuf);
							
							if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
								VOS_Sprintf(&tempBuf[length]," 0x8100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
								VOS_Sprintf(&tempBuf[length]," 0x9100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
								VOS_Sprintf(&tempBuf[length]," 0x88a8\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE )
								VOS_Sprintf(&tempBuf[length]," user-defined\r\n");
							
							vty_out(vty,"%s",tempPtr);
							
						}
						
						else if(PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED == vlan_mgt)
						{
							if (point_flag == 0)
							{
								point_flag = 1;
								vty_out( vty, " pon %d/%d\r\n", slotId,port);
							}
							tempBuf[0] = 0;
							VOS_Sprintf(&tempBuf[0],"  uplink vlan-tag-filter %s", (onulist + groupId*ONULISTLEN + 1));
							length = VOS_StrLen(tempBuf);
							
							if(CurrentPtr->original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE)
								VOS_Sprintf(&tempBuf[length]," all");
							else 
								VOS_Sprintf(&tempBuf[length]," %d", CurrentPtr->original_vlan_id);

		                    /* B--remmed by liwei056@2012-3-19 for D14753 */
#if 0
							length = VOS_StrLen(tempBuf);
							if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_8100 )
								VOS_Sprintf(&tempBuf[length]," 0x8100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_9100 )
								VOS_Sprintf(&tempBuf[length]," 0x9100\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_88A8)
								VOS_Sprintf(&tempBuf[length]," 0x88a8\r\n");
							else if(CurrentPtr->ethernet_type == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE )
								VOS_Sprintf(&tempBuf[length]," user-defined");
#endif
		                    /* E--remmed by liwei056@2012-3-19 for D14753 */
							
							vty_out(vty,"%s\r\n",tempPtr);
						}
					}
				}
				
				#undef ONULISTLEN 256

			}

			/* 下行ONU 业务VLAN 转换*/
			{
			unsigned char ucVlanManIdx;
			unsigned char tempBuf[255];
			unsigned char *tempPtr;
			short int length;
			Olt_llid_vlan_manipulation  *CurrentPtr;
			short int Entry, m;
			
			tempPtr = &tempBuf[0];

            m = PonPortTable[phyPonId].Downlink_vlan_Cfg_Max;
			for(Entry=0; Entry<=m;Entry++)
			{
        		if( 0 != (ucVlanManIdx = PonPortTable[phyPonId].Downlink_vlan_Config[Entry]) )
        		{
    				CurrentPtr = &PonPortTable[phyPonId].Downlink_vlan_manipulation[ucVlanManIdx];
    				if((CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY)
    					||(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID))
					{
    					if (point_flag == 0)
						{
    						point_flag = 1;
    						vty_out( vty, " pon %d/%d\r\n", slotId,port);
						}
    					length = 0;
    					VOS_Sprintf(&tempBuf[length], "  downlink vlan-tag-exchange");
    					length = VOS_StrLen(tempBuf);

    					if(Entry == NULL_VLAN )
    						VOS_Sprintf(&tempBuf[length], " null-tagged %d", CurrentPtr->new_vlan_id );
    					else if(Entry == VLAN_UNTAGGED_ID )
    						VOS_Sprintf(&tempBuf[length], " untagged %d", CurrentPtr->new_vlan_id );
    					else 
    						VOS_Sprintf(&tempBuf[length], " %d %d", Entry, CurrentPtr->new_vlan_id );
    					length = VOS_StrLen(tempBuf);

    					if(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY) 
    						VOS_Sprintf(&tempBuf[length], " %d\r\n", CurrentPtr->new_priority);
    					else 
    						VOS_Sprintf(&tempBuf[length], " original\r\n");
    					
    					vty_out(vty,"%s",tempPtr);
					}
    				else if(CurrentPtr->vlan_manipulation == PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
					{
    					if (point_flag == 0)
						{
    						point_flag = 1;
    						vty_out( vty, " pon %d/%d\r\n", slotId,port);
						}
    					
    					vty_out(vty,"  downlink vlan-tag-striped %d\r\n", Entry);
					}
                }
			}

            
			}	
#endif			
			/*exit*/
			if (point_flag == 1)
			{
				point_flag = 0;
				vty_out( vty, " exit\r\n" );
			}						
						
		}/*end of CLI_PORT_SYSCLE*/ 
	}/*end of CLI_SLOT_SYSCLE*/

	vty_out( vty, "! \r\n\r\n");

	if(onulist != NULL)
		VOS_Free(onulist);
	
    return VOS_OK;
}


DEFUN  (
    onu_conf_associate,
    onu_conf_associate_cmd,
    "onu-profile associate <onuid_list> <name> {[reset]}*1",
    "Config onu info.\n"
    "associate profile\n"
    OnuIDStringDesc
    "profile name\n"
    "erase onu local configuration and reset"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG port = 0;
    ULONG ulOnuId = 0;

    INT16 phyPonId = 0;

    if( parse_pon_command_parameter( vty, &slotId, &port , &ulOnuId, &phyPonId) != VOS_OK )
        return CMD_WARNING;


    if(argc == 2)
    {
        BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
        {
            /*ULONG ulIfidx = IFM_PON_CREATE_INDEX(slotId, port, ulOnuId, 0);*/
			if(checkOnuIsIntoOnuNode(slotId, port, ulOnuId) == VOS_ERROR)
			{
				vty_out(vty, " %d/%d/%d is used by other client now!\r\n", slotId, port, ulOnuId);
				continue;
			}
            onu_profile_associate_by_index(vty, slotId, port, ulOnuId, argv[1]);
        }
        END_PARSE_ONUID_LIST_TO_ONUID();
    }
    else
    {
        BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
        {
    		short int to_slot = device_standby_master_slotno_get();
            /*onuconfAssociateOnuId(phyPonId, ulOnuId-1, argv[1]);*/
            OnuProfile_Action_ByCode(OnuMap_Update, 0, phyPonId, ulOnuId-1, argv[1], NULL, NULL);
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
                OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, slotId, phyPonId, 0, NULL, NULL, NULL);
                if(to_slot)
                    OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, phyPonId, 0, NULL, NULL, NULL);
            }
            OnuMgt_SetMgtReset(phyPonId, ulOnuId-1, 1);
        }
        END_PARSE_ONUID_LIST_TO_ONUID();
    }

    return CMD_SUCCESS;
}

DEFUN  (
    onu_conf_associate_global,
    onu_conf_associate_global_cmd,
    /*"onu associate-profile <slot/port> <onuid_list> <name>",*/
    "onu-profile associate <slot/port> <onuid_list> <name> {[reset]}*1",
    "Config onu info\n"
    "associate profile\n"
    "pon port parameter\n"
    OnuIDStringDesc
    "profile name\n"
    "erase onu local configuration and reset"
    )
{

    ULONG slot,pon,onu;

    short ponid;

    VOS_Sscanf(argv[0], "%d/%d", &slot, &pon);
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_WARNING;
    
    ponid = GetPonPortIdxBySlot(slot, pon);
    /*modi by luh 2013-1-18*/
    if(ponid == VOS_ERROR /*|| (SlotCardIsPonBoard(slot)!=VOS_OK) */|| pon > MAX_PONPORT_PER_BOARD)
    {
        vty_out(vty, "invalid pon port parameter!\r\n");
        return CMD_WARNING;
    }

    if(!isOnuConfExist(argv[2]))
    {
        vty_out(vty, "\r\nfile %s doesn't exist! please check the file name you entered!\r\n", argv[2]);
        return CMD_WARNING;
    }

	if(argc == 3)
	{
	    BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], onu )
	    {
            if(g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_)
            {
    	        /*ULONG ulIfidx = IFM_PON_CREATE_INDEX(slot, pon, onu, 0);*/
				if(checkOnuIsIntoOnuNode(slot, pon, onu) == VOS_ERROR)
				{
					vty_out(vty, " %d/%d/%d is used by other client now!\r\n", slot, pon, onu);
					continue;
				}
				
    	        onu_profile_associate_by_index(vty, slot, pon, onu, argv[2]);
            }
            else/*冷启动时，只恢复主控板本地的映射关系；2012-10-19 added by luh*/
            {
                OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onu-1, argv[2], NULL, NULL);
            }
	    }
	    END_PARSE_ONUID_LIST_TO_ONUID();
	}
	else
	{

	    /*私有配置文件只能关联给一个ONU*/
	    if(!onuConfIsSharedByName(argv[2]) && onuconfHaveAssociatedOnu(argv[2]))
	    {
	        vty_out(vty, "\r\nassociation with onu fail!\r\nthe private config file has been associated with one ONU!\r\n");
	        return CMD_WARNING;
	    }

	    BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], onu )
	    {
    		short int to_slot = device_standby_master_slotno_get();
            
            OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onu-1, argv[2], NULL, NULL);
			/*onuconfAssociateOnuId(ponid, onu-1, argv[2]);*/
			if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
                OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, slot, ponid, 0, NULL, NULL, NULL);
                if(to_slot)
                    OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, ponid, 0, NULL, NULL, NULL);
			}
			OnuMgt_SetMgtReset(ponid, onu-1, 1);
	    }
	    END_PARSE_ONUID_LIST_TO_ONUID();		
	}

    return CMD_SUCCESS;
}

DEFUN  (
    onu_conf_apply_global,
    onu_conf_apply_global_cmd,
    "onu apply profile <slot/port> <onuid_list> {reset}*1",
    "Config onu info\n"
    "apply the profile\n"
    "Config the profile \n"
    "pon port parameter\n"
    OnuIDStringDesc
    "reset onu\n"
    )
{

    ULONG slot,pon,onu;

    short ponid, reset = 0;

    if(VOS_StrStr(vty->buf, "reset"))
        reset = 1;

    VOS_Sscanf(argv[0], "%d/%d", &slot, &pon);

    ponid = GetPonPortIdxBySlot(slot, pon);

    if((!SYS_MODULE_IS_PON(slot)) || pon > MAXPONPORT_PER_BOARD || ponid == VOS_ERROR )
    {
        vty_out(vty, "invalid pon port parameter!\r\n");
        return CMD_WARNING;
    }

    BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], onu )
    {
        if(reset)
        {
            OnuMgt_SetMgtReset(ponid, onu-1, 0);
        }
        else
            sendOnuConfExecuteMsg(slot, pon, onu);
    }
    END_PARSE_ONUID_LIST_TO_ONUID();

    return CMD_SUCCESS;
}

DEFUN(
        onu_profile_info_inpon_show,
        onu_profile_info_inpon_show_cmd,
        "show onu-profile list",
        SHOW_STR
        "profile infor\n"
        "file list\n"
        )
{

    ULONG slot, pon, onu;
    short int lpon;

    if(parse_pon_command_parameter(vty, &slot, &pon, &onu, &lpon) != VOS_OK)
    {
        vty_out(vty, "parameters error!\r\n");
        return CMD_WARNING;
    }
    else
    {
        if(SYS_MODULE_IS_PON(slot) && pon <= /*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD)
            return show_onu_conf_list_by_pon(vty, lpon);
        else
        {
            vty_out(vty, "parameters error!\r\n");
            return CMD_WARNING;
        }
    }
}
#if 1
DEFUN  (
    config_default_max_onu_counter,
    config_default_max_onu_counter_cmd,
		"config default-max-onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
		"Config onu info.\n"
		"Config the default max-onu number per pon\n"
		"Please input the max-onu number,the default value is "INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
)
{
	UINT32 number = (ULONG) VOS_AtoL( argv[ 0 ] );
	INT16 phyPonId = 0;
	
    CHECK_CMD_ONU_RANGE(vty, number-1);

	number |= CONFIG_DEFAULT_MAX_ONU_FLAG|CONFIG_CODE_ALL_PORT;

	
	if ( OLT_CALL_ISERROR(OLT_SetMaxOnu(OLT_ID_ALL, number)) )
	{
		return RERROR;
	}

    return CMD_SUCCESS;
}

DEFUN  (
    undo_default_max_onu_counter,
		undo_default_max_onu_counter_cmd,
		"undo default-max-onu",
		"clear config\n"
		"clear the current default-max-onu config\n"
)
{
	unsigned int number = DAFAULT_MAX_ONU;
	if ( OLT_CALL_ISERROR(OLT_SetMaxOnu(OLT_ID_ALL, number | CONFIG_DEFAULT_MAX_ONU_FLAG | CONFIG_CODE_ALL_PORT)) )
	{
		return RERROR;
	}

	return CMD_SUCCESS;
}
DEFUN  (
    show_default_max_onu_counter,
    show_default_max_onu_counter_cmd,
    "show default-max-onu",
    DescStringCommonShow
    "Show the current default-max-onu config\n"
    )
{
	vty_out(vty, "The default max-onu per pon is: %d.\r\n", MaxOnuDefault);
    return CMD_SUCCESS;
}
DEFUN  (
    config_max_onu_counter,
		config_max_onu_counter_cmd,
		"config max-onu <1-"INT_TO_STR(MAXONUPERPONNOLIMIT)">",
    "Config onu info.\n"
    "Config the default max-onu number per pon\n"
		"Please input the max-onu number,the default value is "INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
    )
{
    UINT32 number = 0;
    INT16 phyPonId = 0;
	ULONG ulSlot = 0, ulPort = 0, ulOnuid = 0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
	return CMD_WARNING;

	number = ( ULONG ) VOS_AtoL( argv[ 0 ] );
    CHECK_CMD_ONU_RANGE(vty, number-1);
	
    if ( OLT_CALL_ISERROR(OLT_SetMaxOnu(phyPonId, number)) )
    {
        return RERROR;
    }

    return CMD_SUCCESS;
}

DEFUN  (
    undo_max_onu_counter,
    undo_max_onu_counter_cmd,
    "undo max-onu",
    "clear config\n"
    "clear the current default-max-onu config\n"
    )
{
	unsigned int number = MaxOnuDefault | CONFIG_DEFAULT_MAX_ONU_FLAG;
    INT16 phyPonId = 0;
    ULONG ulSlot = 0, ulPort = 0, ulOnuid = 0;
    
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;
    
    if ( OLT_CALL_ISERROR(OLT_SetMaxOnu(phyPonId, number)) )
    {
        return RERROR;
    }

    return CMD_SUCCESS;
}
DEFUN  (
    show_onu_max_onu_counter,
    show_onu_max_onu_counter_cmd,
    "show max-onu",
    DescStringCommonShow
    "Show the max-onu config\n"
    )
{
    LONG lRet = VOS_OK;
    INT16 phyPonId = 0;
    ULONG ulSlot = 0, ulPort = 0, ulOnuid = 0;
    
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuid, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	vty_out(vty, "The default max-onu per pon is: %d.\r\n", GetMaxOnuByPonPort(phyPonId)&0xff);
    return CMD_SUCCESS;
}
#endif

extern short int PonOamDebugOltId;
extern short int PonOamDebugLlid;
extern short int PonOamDebugSend;
extern short int PonOamDebugRec;
extern unsigned char OnuMacAddrDebug[BYTES_IN_MAC_ADDRESS];
DEFUN (
		debug_onu_oam,
		debug_onu_oam_cmd,
		"debug oam {[tx|rx|all] <slot/port/onuid>}*1",
		"Debug\n"
		"Config oam module debug on\n"
		"Debug Tx OAM infomation\n"
		"Debug Rx OAM infomation\n"
		"Debug both Tx and Rx OAM infomation\n"
		"Please input onuId\n"
)
{
	LONG lRet = VOS_OK;
	ULONG ulSlot = 0, ulPort = 0, ulOnuid = 0;
	int len = 0;
    if(argc)
    {
    	short int PonPortIdx = 0;
		short int Llid = 0;
    	lRet = PON_ParseSlotPortOnu( argv[1], (ULONG *)&ulSlot, (ULONG *)&ulPort , (ULONG *)&ulOnuid);
    	if( lRet != VOS_OK )
        	return CMD_WARNING;
		
        PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
		if(PonPortIdx == VOS_ERROR)
		{
			return CMD_WARNING;
		}
		
        Llid = GetLlidByOnuIdx(PonPortIdx, ulOnuid-1);
		if(INVALID_LLID == Llid)
		{
			return CMD_WARNING;
		}
		
		/*确保onu在线情况下再打开调试开关*/
		PonOamDebugOltId = PonPortIdx;
		PonOamDebugLlid = Llid;
		lRet = GetOnuMacAddr(PonPortIdx, ulOnuid-1, OnuMacAddrDebug, &len);
		if( lRet != VOS_OK )
        	return CMD_WARNING;
	
        if(VOS_StrnCmp(argv[0], "t", 1) == 0)
            PonOamDebugSend = 1;
        else if(VOS_StrnCmp(argv[0], "r", 1) == 0)
            PonOamDebugRec = 1;
        else
        {
            PonOamDebugSend = 1;
            PonOamDebugRec = 1;
        }

	}
    else
    {
        if(PonOamDebugSend || PonOamDebugRec)
        {
            vty_out(vty, "\r\nPon%d/%d OAM DEBUG:\r\n", GetCardIdxByPonChip(PonOamDebugOltId), GetPonPortByPonChip(PonOamDebugOltId));
            vty_out(vty, "ONU %d: Send( %s )/ Recieve (%s)\r\n", GetOnuIdxByLlid(PonOamDebugOltId, PonOamDebugLlid)+1, PonOamDebugSend?"OPEN":"CLOSE", PonOamDebugRec?"OPEN":"CLOSE");
        }
        else
        {
            vty_out(vty, "No debug config!\r\n");
        }
    }
    return CMD_SUCCESS;
}
DEFUN (
		nodebug_onu_oam,
		nodebug_onu_oam_cmd,
		"undo debug oam",
		"Negate a command or set its defaults\n"
		"Debug\n"
		"Config oam module debug on\n"
)
{
    PonOamDebugOltId = 0;
    PonOamDebugLlid = 0;
    PonOamDebugSend = 0;
    PonOamDebugRec = 0;
	VOS_MemSet(OnuMacAddrDebug,0,BYTES_IN_MAC_ADDRESS);
    return CMD_SUCCESS;
}
/*added by liyang @2015-06-24 for Tk physical msg debug*/
extern unsigned short int TkPonPhyMsgCmdOpFirst;
extern unsigned short int TkPonPhyMsgCmdOpSecond;
extern unsigned short int TkPonPhyMsgEventOp;
DEFUN (
		debug_tk_pon_phy_msg,
		debug_tk_pon_phy_msg_cmd,
		"debug tk-physical-msg cmd <0-872> <0-872> event <32768-32822>",
		"Debug\n"
		"Config tk pon physical msg debug on\n"
		"Debug two cmd opcde\n"
		"Please input first cmd opcode\n"
		"Please input second cmd opcode\n"
		"Debug one event opcode\n"
		"Please input event opcode\n"
)
{
	
	TkPonPhyMsgCmdOpFirst = VOS_AtoL( argv[ 0 ] );
	TkPonPhyMsgCmdOpSecond = VOS_AtoL( argv[ 1 ] );
	TkPonPhyMsgEventOp = VOS_AtoL( argv[ 2 ] );

    return CMD_SUCCESS;
}


int pon_init_func()
{
    return VOS_OK;
}


#define STATE_PS 1
#define STATE_PE 2
#define MAX_ONUID_NUMBER_IN_ONUID_LIST			MAXONUPERPONNOLIMIT /*MAXONUPERPON*/  /*64*/
#define START_PORT								1
#define END_PORT								MAX_ONUID_NUMBER_IN_ONUID_LIST
ULONG * V2R1_Parse_Id_List( CHAR * pcId_List, ULONG ulIdMin, ULONG ulIdMax, ULONG ulIdNumMax )
{
    ULONG ulState = STATE_PS;
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_ONUID_NUMBER_IN_ONUID_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    ULONG ulListLen = 0;
    CHAR * list;

    if ( ulIdNumMax > ARRAY_SIZE(ulInterfaceList) )
    {
        VOS_ASSERT(0);
    }

    VOS_MemZero( ulInterfaceList, sizeof(ulInterfaceList) );
    ulListLen = VOS_StrLen( pcId_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcId_List, ulListLen + 1 );
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

                    if ( ulPortS > (ulIdMax) )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > (ulIdNumMax) )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    /*ulState = STATE_S;*/
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
                        if ( i<ulIdMin || i > ulIdMax )
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > (ulIdNumMax) )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    /*ulState = STATE_S;*/
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

ULONG * V2R1_Parse_OnuId_List( CHAR * pcOnuId_List )
{
    return V2R1_Parse_Id_List(pcOnuId_List, START_PORT, END_PORT, MAX_ONUID_NUMBER_IN_ONUID_LIST);
}

LONG V2R1_Check_OnuId_ListValid( const char * pcOnuId_List )
{
    ULONG * p = NULL;

    p = V2R1_Parse_OnuId_List( ( CHAR * ) pcOnuId_List );

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


enum match_type V2R1_Check_OnuId_List( char * onuid_list )
{
    int len = VOS_StrLen( onuid_list );
    ULONG interface_list[ MAX_ONUID_NUMBER_IN_ONUID_LIST ];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulOnuId=0;

    char *plistbak = NULL;

    if ( ( !onuid_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_ONUID_NUMBER_IN_ONUID_LIST * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, onuid_list );

    BEGIN_PARSE_ONUID_LIST_TO_ONUID( plistbak, ulOnuId )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulOnuId )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                RETURN_PARSE_ONUID_LIST_TO_ONUID( no_match );  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulOnuId;
        if_num ++;
        if ( if_num > (MAX_ONUID_NUMBER_IN_ONUID_LIST) )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            RETURN_PARSE_ONUID_LIST_TO_ONUID( no_match );
        }
        ret = 1;
    }
    END_PARSE_ONUID_LIST_TO_ONUID();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}

/* modified by chenfj 2008-3-11
    问题单6405: 
    修改优先级解析函数, 将优先级范围从1-8 修改为0-7
    */
#define MAX_PRIORITY_LIST  7

ULONG * V2R1_Parse_Priority_List( CHAR * priority_list )
{
    ULONG ulState = STATE_PS;
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_PRIORITY_LIST+1 ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    ULONG ulListLen = 0;
    CHAR * list;

	for(temp_i=0; temp_i<(MAX_PRIORITY_LIST+1); temp_i++)
		{
		ulInterfaceList[temp_i] = MAX_PRIORITY_LIST+1;
		}
	temp_i =0;
	
    /*VOS_MemZero( ulInterfaceList, (MAX_PRIORITY_LIST+1) * 4 );*/
    ulListLen = VOS_StrLen( priority_list );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, priority_list, ulListLen + 1 );
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

                    if ( ulPortS > (MAX_PRIORITY_LIST) )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 <= ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > (MAX_PRIORITY_LIST+1) )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    /*ulState = STATE_S;*/
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
                        if (/* i<START_PORT ||*/ i > MAX_PRIORITY_LIST )
                        {
                            goto error;
                        }

                        if ( 0 <= i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > (MAX_PRIORITY_LIST+1) )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    /*ulState = STATE_S;*/
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
	  *(int *)&list[iflist_i * 4] = 0xaa55;
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}

enum match_type V2R1_Check_Priority_List( char * priority_list )
{
    int len = VOS_StrLen( priority_list );
    ULONG interface_list[ MAX_PRIORITY_LIST+1 ];
    int j, if_num = 0;
    int ret = 0;
    ULONG priority=0;

    char *plistbak = NULL;

    if ( ( !priority_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

	for(j=0; j<(MAX_PRIORITY_LIST+1); j++)
		interface_list[j] = MAX_PRIORITY_LIST+1;
    /*VOS_MemZero( ( char * ) interface_list, (MAX_PRIORITY_LIST+1) * sizeof( ULONG ) );*/
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, priority_list );

    BEGIN_PARSE_PRIORITY_LIST_TO_PRIORITY( plistbak, priority )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == priority )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = priority;
        if_num ++;
        if ( if_num > (MAX_PRIORITY_LIST+1) )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_PARSE_PRIORITY_LIST_TO_PRIORITY();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_V2R1_OnuId_List_Check =
{
    "<onuid_list>",
    V2R1_Check_OnuId_List,
    0
};

CMD_NOTIFY_REFISTER_S stCMD_V2R1_Priority_List_Check =
{
    "<priority_list>",
    V2R1_Check_Priority_List,
    0
};

/***************************************/

int pon_showrun( struct vty * vty )
{
    /*IFM_READ_LOCK;*/
    PON_show_run( 0, vty );
    /*IFM_READ_UNLOCK;*/
    
    return VOS_OK;
}


int pon_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node pon_port_node =
{
    PON_PORT_NODE,
    NULL,
    1
};

LONG pon_node_install()
{
    install_node( &pon_port_node, pon_config_write);
    pon_port_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_PON);
    if ( !pon_port_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( PON_PORT_NODE );
    return VOS_OK;
}

LONG pon_module_init()
{
    struct cl_cmd_module * pon_module = NULL;

    pon_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_PON);
    if ( !pon_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) pon_module, sizeof( struct cl_cmd_module ) );

    pon_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_PON );
    if ( !pon_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( pon_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( pon_module->module_name, "pon" );

    pon_module->init_func = pon_init_func;
    pon_module->showrun_func = pon_showrun;
    pon_module->next = NULL;
    pon_module->prev = NULL;
/*************/
	if ( cmd_rugular_register( &stCMD_V2R1_OnuId_List_Check ) == no_match )
	{
        ASSERT( 0 );
	}

	if ( cmd_rugular_register( &stCMD_V2R1_Priority_List_Check ) == no_match )
	{
        ASSERT( 0 );
	}
/***************/
    cl_install_module( pon_module );

    return VOS_OK;
}

extern struct cmd_element show_running_config_cmd;

void  Pon_EncryptCLI()
{
	install_element ( PON_PORT_NODE, &pon_no_encrypt_config_cmd);	
	install_element ( PON_PORT_NODE, &pon_encrypt_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_encrypt_config_cmd);
}

LONG PON_CommandInstall()
{
	/*install_element( PON_PORT_NODE, &pon_active_onu_config_cmd );*/
	/*	install_element( PON_PORT_NODE, &pon_change_onu_type_config_cmd ); */
	install_element ( CONFIG_NODE, &into_epon_pon_node_cmd);
	/*install_element ( PON_PORT_NODE, &pon_bandwidth_alloc_mode_config_cmd);*/

	/*2006/11/20 by wutw
	install_element ( PON_PORT_NODE, &pon_max_max_config_cmd);
	install_element ( PON_PORT_NODE, &pon_mac_learn_config_cmd);*/
	install_element ( VIEW_NODE, &bandwidth_onu_off_show_cmd );
	
	install_element ( CONFIG_NODE, &bandwidth_onu_on_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_off_show_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_off_cmd ); 

	install_element ( PON_PORT_NODE, &bandwidth_onu_on_cmd );
	install_element ( PON_PORT_NODE, &bandwidth_onu_off_show_cmd );
	install_element ( PON_PORT_NODE, &bandwidth_onu_off_cmd ); 

	/* commented by chenfj 2007-8-8 
		将CTC模式下的FEC 与NTT模式FEC命令合并，取消PON口下的命令*/ 
#if 1
	install_element( PON_PORT_NODE, &onu_fec_config_pon_cmd );
	/*install_element( PON_PORT_NODE, &undo_onu_fec_config_cmd );*/
	install_element( PON_PORT_NODE, &show_onu_fec_config_pon_cmd );
	install_element( PON_PORT_NODE, &show_fec_ability_pon_cmd );
#endif

	install_element ( PON_PORT_NODE, &pon_mac_age_config_cmd);
	/*install_element ( PON_PORT_NODE, &pon_fdb_mac_config_cmd);*/
	install_element ( PON_PORT_NODE, &pon_bandwidth_config_cmd);
	install_element ( PON_PORT_NODE, &undo_pon_bandwidth_config_cmd);/*add by shixh20100120*/
	/*2006/11/20 by wutw*/
	/*install_element ( PON_PORT_NODE, &pon_aps_config_cmd);*/


	/*install_element ( PON_PORT_NODE, &pon_onu_register_mac_config_cmd);*/
	
	install_element ( PON_PORT_NODE, &pon_no_encrypt_config_cmd);	
	install_element ( PON_PORT_NODE, &pon_encrypt_config_cmd);	
	
	install_element ( PON_PORT_NODE, &pon_show_encrypt_config_cmd);
	install_element ( PON_PORT_NODE, &pon_encrypt_keytime_config_cmd);


	/*2006/11/20 by wutw*/
	/*install_element ( PON_PORT_NODE, &pon_no_max_mac_config_cmd);*/


	/*install_element ( PON_PORT_NODE, &pon_no_onu_register_mac_config_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_no_fdb_mac_config_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_reset_config_cmd);*/
	install_element ( PON_PORT_NODE, &pon_update_config_cmd);
	install_element ( CONFIG_NODE, &config_pon_update_config_cmd );

	/* install_element ( PON_PORT_NODE, &pon_show_fdb_llid_config_cmd);*/
	install_element ( PON_PORT_NODE, &pon_show_bandwidth_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_port_info_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_llid_bandwith_config_cmd);
	/*install_element ( PON_PORT_NODE, &pon_show_onu_register_mac_cmd);*/
	install_element ( PON_PORT_NODE, &pon_show_online_onu_cmd);
	install_element ( PON_PORT_NODE, &pon_show_offline_onu_cmd);
	install_element ( PON_PORT_NODE, &pon_show_list_onu_cmd);
	/*install_element ( PON_PORT_NODE, &pon_start_statistic_config_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_no_statistic_config_cmd);*/

	/*begin:
	commented by wangxiaoyu 2008-01-22 
	end*/
	/*install_element ( PON_PORT_NODE, &pon_show_statistic_bucket_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_alarm_config_cmd);*/
	/* install_element ( PON_PORT_NODE, &pon_alarm_threshold_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_alarm_info_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_optical_link_monitor_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_optical_link_ber_set_cmd);*/
	/*  install_element ( PON_PORT_NODE, &pon_show_optical_line_parameter_cmd);
	install_element ( PON_PORT_NODE, &pon_loop_mode_set_cmd);
	install_element ( PON_PORT_NODE, &pon_show_loop_mode_cmd);*/

	/*2006/11/20 by wutw*/
	/*added by wutw at 12 september */
	/*install_element ( PON_PORT_NODE, &pon_max_max_llid_config_cmd);*/


	install_element ( PON_PORT_NODE, &pon_show_mac_age_config_cmd);	
	install_element ( PON_PORT_NODE, &pon_add_onu_config_cmd);		
	install_element ( PON_PORT_NODE, &pon_del_onu_config_cmd);	
	install_element ( PON_PORT_NODE, &pon_add_gonu_config_cmd);		
	
#if 0    /*moved by luh 2013-08-06*/
	install_element ( PON_PORT_NODE, &pon_modi_onu_config_cmd);		
#endif		
	install_element ( VIEW_NODE, &olt_add_onu_config_cmd);		
	install_element ( VIEW_NODE, &olt_del_onu_config_cmd);	
	install_element ( VIEW_NODE, &olt_add_gonu_config_cmd);			
#if 0    
	install_element ( VIEW_NODE, &olt_modi_onu_config_cmd);	
	install_element ( CONFIG_NODE, &olt_modi_onu_config_cmd);	
#endif    
	install_element ( PON_PORT_NODE, &pon_del_offline_onu_config_cmd);/*add by luh 2012-7-6*/	
	install_element ( PON_PORT_NODE, &pon_show_fdb_mac_config_cmd);	
	install_element( PON_PORT_NODE,&pon_show_fdb_mac_counter_cmd );

	install_element ( PON_PORT_NODE, &pon_clear_statistic_pon_onu_data_cmd);	
	/*
	install_element ( PON_PORT_NODE, &pon_clear_statistic_pon_all_cmd);		
	install_element ( PON_PORT_NODE, &pon_clear_statistic_pon_data_cmd);		
	install_element ( PON_PORT_NODE, &pon_clear_statistic_onu_all_cmd);			
	install_element ( PON_PORT_NODE, &pon_clear_statistic_onu_data_cmd);
	*/
	install_element ( PON_PORT_NODE, &pon_show_statistic_15m_24h_data_cmd);
	/*
	install_element ( PON_PORT_NODE, &pon_show_statistic_15mdata_cmd);
	install_element ( PON_PORT_NODE, &pon_show_statistic_15mall_cmd);
	install_element ( PON_PORT_NODE, &pon_show_statistic_24hdata_cmd);
	install_element ( PON_PORT_NODE, &pon_show_statistic_24hall_cmd);
	*/
	/*2006/11/20 by wutw
	install_element ( PON_PORT_NODE, &pon_show_mac_learn_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_aps_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_max_max_config_cmd);
	install_element ( PON_PORT_NODE, &pon_show_max_max_llid_config_cmd);*/

#if 0 /* deleted by chenfj 2007-7-26 */
	install_element ( PON_PORT_NODE, &pon_show_onu_regiseter_limit_config_cmd);
	install_element ( PON_PORT_NODE, &pon_no_onu_regiseter_limitconfig_cmd);
	install_element ( PON_PORT_NODE, &pon_onu_regiseter_limitconfig_cmd);
#endif

	/*install_element ( PON_PORT_NODE, &onu_logfile_show_cmd);*/  /* add by suxq */

	/*added by wutw 2006/11/12*/
	install_element ( PON_PORT_NODE, &pon_start_statistic_sycle_config_cmd);
	install_element ( PON_PORT_NODE, &pon_no_statistic_sycle_config_cmd);

	/*added by wutw 2006/11/14*/
	install_element ( PON_PORT_NODE, &pon_show_statistic_cmd);


	/*added by wutw 2006/11/21*/
	install_element ( PON_PORT_NODE, &pon_onu_version_show_cmd);
	install_element ( PON_PORT_NODE, &pon_onu_version_by_type_show_cmd );
	install_element ( PON_PORT_NODE, &pon_onu_auto_update_enable_cmd);
	install_element ( PON_PORT_NODE, &pon_onu_auto_update_disable_cmd);
	install_element ( PON_PORT_NODE, &pon_onu_auto_update_show_cmd);


	/*install_element ( PON_PORT_NODE, &pon_log_show_cmd);*/
	/*install_element ( PON_PORT_NODE, &pon_log_upload_cmd);*/

#ifdef ONU_PEER_TO_PEER	
	/*added by wutw 2006/12/14*/
	install_element ( PON_PORT_NODE, &peer_to_peer_add_cmd);
	install_element ( PON_PORT_NODE, &peer_to_peer_delete_cmd);
	install_element ( PON_PORT_NODE, &peer_to_peer_show_cmd);
	install_element ( PON_PORT_NODE, &peer_to_peer_forward_rule_cmd);
#endif

	/* 
	问题单6222: 建议将onu-register window命令调整到隐藏节点下
	*/
#ifdef  ONU_RANGE_WINDOW_PON_CLI
	install_element ( PON_PORT_NODE, &pon_range_set_cmd);
	install_element ( PON_PORT_NODE, &pon_range_show_cmd);
#endif
	
	/*added by wutw 2007/01/17*/
	install_element ( PON_PORT_NODE, &pon_port_statstics_show_cmd);
	install_element ( PON_PORT_NODE, &CNI_statstics_show_cmd);
	install_element ( PON_PORT_NODE, &hostMsg_statstics_show_cmd);
	install_element ( PON_PORT_NODE, &pon_gem_statstics_show_cmd);

	/****************/
	/*install_element ( PON_PORT_NODE, &test_onuid_list_cmd );*/
	/******************/

	/* deleted by chenf 2007-8-1 
	  这两个命令执行出错；调用PMC API时参数赋值错误；修正之后再放开
	  */
	install_element ( PON_PORT_NODE, &onu_uplink_ber_show_cmd);
	install_element ( PON_PORT_NODE, &onu_uplink_fer_show_cmd);
	

	/* add by chenfj 2007-5-23 */

	install_element( PON_PORT_NODE, &onu_max_mac_config_cmd );
	install_element( PON_PORT_NODE, &onu_max_mac_show_cmd );
	install_element( PON_PORT_NODE, &undo_onu_max_mac_config_cmd );
	install_element( CONFIG_NODE, &onu_default_max_mac_config_cmd );
	install_element( CONFIG_NODE, &undo_onu_default_max_mac_config_cmd );
	install_element( CONFIG_NODE, &onu_default_max_mac_show_cmd );
	install_element( VIEW_NODE, &onu_default_max_mac_show_cmd );
	install_element( VIEW_NODE, &onu_max_mac_show_by_pon_cmd );
	install_element( CONFIG_NODE, &onu_max_mac_show_by_pon_cmd );
#if  0		/* for CNC */
	/* add by chenfj 2007-6-1 */
	install_element( PON_PORT_NODE, &onu_SA_mac_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_SA_mac_filter_config_cmd );
	install_element( PON_PORT_NODE, &show_onu_SA_mac_filter_config_cmd );

	/* add by chenfj  2007-6-12 */
	install_element( PON_PORT_NODE, &onu_Ip_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_Ip_filter_config_cmd );
	install_element( PON_PORT_NODE, &onu_SIpUdp_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_SIpUdp_filter_config_cmd );	
	install_element( PON_PORT_NODE, &onu_SIpTcp_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_SIpTcp_filter_config_cmd );
	install_element( PON_PORT_NODE, &show_onu_IPUDPTCP_filter_config_cmd );
	/*
	install_element( PON_PORT_NODE, &onu_DIpUdp_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_DIpUdp_filter_config_cmd );
	install_element( PON_PORT_NODE, &onu_DIpTcp_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_DIpTcp_filter_config_cmd);
	*/

	/* added by chenfj 2007-6-14 */
	install_element( PON_PORT_NODE, &onu_vlanId_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_vlanId_filter_config_cmd );

	/* added by chenfj 2007-6-15 */
	install_element( PON_PORT_NODE, &onu_EtherType_filter_config_cmd );
	install_element( PON_PORT_NODE, &undo_onu_EtherType_filter_config_cmd );
	install_element( PON_PORT_NODE, &onu_IpType_filter_config_cmd);
	install_element( PON_PORT_NODE, &undo_onu_IpType_filter_config_cmd);
#endif

	/* added by chenfj 2007-6-21  增加PON 端口保护切换*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	install_element ( PON_PORT_NODE, &set_ponport_hot_swap_cmd);
    install_element ( PON_PORT_NODE, &set_ponport_hot_swap_full_cmd);
	install_element ( PON_PORT_NODE, &undo_set_ponport_hot_swap_cmd );
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
	install_element ( PON_PORT_NODE, &op_trigger_ponport_hot_swap_cmd );
	install_element ( PON_PORT_NODE, &op_untrigger_ponport_hot_swap_cmd );
#endif
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
	install_element ( PON_PORT_NODE, &oe_trigger_ponport_hot_swap_cmd );
	install_element ( PON_PORT_NODE, &oe_untrigger_ponport_hot_swap_cmd );
#endif
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
	install_element ( PON_PORT_NODE, &ud_trigger_ponport_hot_swap_cmd );
	install_element ( PON_PORT_NODE, &ud_untrigger_ponport_hot_swap_cmd );
#endif
	install_element ( PON_PORT_NODE, &force_ponport_hot_swap_cmd );
	install_element ( PON_PORT_NODE, &sync_ponport_hot_swap_cmd );
	install_element ( PON_PORT_NODE, &show_ponport_hot_swap_cmd );
/* B--added by liwei056@2012-8-7 for D14913 */
	install_element ( PON_PORT_NODE, &show_ponport_trigger_swap_cmd );
/* E--added by liwei056@2012-8-7 for D14913 */
/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#if ( (EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES) || (EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES) )
	install_element ( PON_PORT_NODE, &set_ponport_hot_swap_mode_cmd );
	install_element ( PON_PORT_NODE, &show_ponport_hot_swap_mode_cmd );
#endif
#if ( EPON_SUBMODULE_PON_SWAP_MONITOR == EPON_MODULE_YES )
	install_element ( PON_PORT_NODE, &show_ponport_hot_swap_status_cmd );
#endif
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */

#endif

	/* added by chenfj 2007-6-27  
     PON 端口管理使能: up / down  , reset 
     ONU 激活、去激活操作*/
	install_element ( PON_PORT_NODE, &shutdown_ponport_cmd );
	install_element ( PON_PORT_NODE, &undo_shutdown_ponport_cmd );
	install_element ( PON_PORT_NODE, &deregister_onu_cmd ); /*added by luh @2016-6-21 for deregister one onu*/
	/*install_element ( PON_PORT_NODE, &reset_ponport_cmd );*/
	install_element ( PON_PORT_NODE, &pon_node_deactivate_onu_device_cmd );
	install_element ( PON_PORT_NODE, &undo_pon_node_deactivate_onu_device_cmd );
	install_element ( PON_PORT_NODE, &show_deactivate_onu_device1_cmd );

#if 1
	/* added by chenfj 2007-7-9
		for onu authenticaion by PAS5001 & PAS5201 */
	install_element ( PON_PORT_NODE, &show_onuauthentication_entry_cmd );
	install_element ( PON_PORT_NODE, &add_onuauthentication_entry_cmd );
	install_element ( PON_PORT_NODE, &delete_onuauthentication_entry_cmd );
#endif

#if 1
    install_element ( PON_PORT_NODE, &show_gonuauthentication_entry_cmd );
    install_element ( PON_PORT_NODE, &add_gonuauthentication_entry_cmd );
    install_element ( PON_PORT_NODE, &delete_gonuauthentication_entry_cmd );
#endif
	/* added by chenfj 2007-7-31 
	    PON端口BER / FER 告警 */
	install_element ( PON_PORT_NODE, &onupon_ber_fer_alarm_enable_cmd );
	/*install_element ( PON_PORT_NODE, &onupon_ber_fer_alarm_disable_cmd );*/
	install_element ( PON_PORT_NODE, &show_onupon_ber_fer_alarm_cmd );
	install_element ( PON_PORT_NODE, &oltpon_ber_fer_alarm_enable_cmd );
	/*install_element ( PON_PORT_NODE, &oltpon_ber_fer_alarm_disable_cmd );*/
	install_element ( PON_PORT_NODE, &show_oltpon_ber_fer_alarm_cmd );	
	
		/* PON芯片复位命令*/
	install_element ( PON_PORT_NODE, &reset_ponchip_cmd );

	/*install_element ( PON_PORT_NODE, &set_pon_address_table_full_handling_mode_cmd );*/
	install_element ( PON_PORT_NODE, &set_pon_address_table_configuration_cmd );
	install_element ( PON_PORT_NODE, &set_pon_address_table_configuration_enable_cmd );/*add by yangzl@2016-5-6*/
	install_element ( PON_PORT_NODE, &show_olt_address_table_configuration_cmd );
	
	install_element (PON_PORT_NODE, &onu_conf_associate_cmd);
	install_element (PON_PORT_NODE, &onu_profile_info_inpon_show_cmd);

#if 1
	/* added by chenfj 2007-8-31 
	PAS-SOFT软件包中定义的CLI命令*/	
	install_element ( DEBUG_HIDDEN_NODE, &show_oam_information_cmd );
	install_element ( PON_PORT_NODE, &show_ponport_version_cmd );
	/*install_element ( DEBUG_HIDDEN_NODE, &show_onu_version_cmd );*/
	install_element ( DEBUG_HIDDEN_NODE, &show_olt_capabilities_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_llid_capabilities_cmd );
	/*install_element ( DEBUG_HIDDEN_NODE, &show_onu_statistics_cmd );	
	install_element ( DEBUG_HIDDEN_NODE, &show_olt_address_table_configuration_cmd );*/
	install_element ( DEBUG_HIDDEN_NODE, &show_onu_parameters_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_dba_information_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_dba_sla_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_uni_port_config_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_optics_parameters_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_oam_limited_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_llid_vlan_mode_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_llid_encryption_cmd );
	
	/*
	install_element ( PON_PORT_NODE, &show_olt_mdio_cmd );
	install_element ( PON_PORT_NODE, &show_olt_eeprom_cmd );
	install_element ( PON_PORT_NODE, &show_olt_register_cmd );
	install_element ( PON_PORT_NODE, &show_olt_cni_management_mode_cmd );
	install_element ( PON_PORT_NODE, &show_gpio_cmd );
	install_element ( PON_PORT_NODE, &show_llid_authorize_cmd );
	install_element ( PON_PORT_NODE, &show_llid_fec_cmd );
	*/
	install_element ( CONFIG_NODE, &show_passoft_system_param_cmd );
	install_element ( VIEW_NODE, &show_passoft_system_param_cmd );
#endif	

	/*install_element ( PON_PORT_NODE, &ftpc_download_ftp_phenixos_master_for_pon_onus_cmd );*/

	/* added by chenfj 2007-12-19
	增加用于设置PON 下行优先级与队列之间映射*/
	/*
	install_element ( CONFIG_NODE, &test_priority_cmd );*/
	install_element ( CONFIG_NODE, &downlink_qos_priority_mapping_cmd );
	install_element ( CONFIG_NODE, &show_downlink_qos_priority_mapping_cmd );
	install_element ( VIEW_NODE, &show_downlink_qos_priority_mapping_cmd );
	install_element ( CONFIG_NODE, &show_pon_jumbo_frame_length_cmd );
	install_element ( VIEW_NODE, &show_pon_jumbo_frame_length_cmd );
	install_element ( CONFIG_NODE, &set_pon_jumbo_frame_length_cmd );

	install_element (PON_PORT_NODE,&show_running_config_cmd);

	/* 在CONFIG节点下增加显示PON 端口信息的命令*/
	install_element ( CONFIG_NODE, &show_pon_port_info_config_cmd );
	install_element ( CONFIG_NODE, &olt_show_port_info_config_cmd );    
	install_element ( VIEW_NODE, &olt_show_port_info_config_cmd );    
	install_element ( CONFIG_NODE, &config_show_ponport_version_cmd );
	install_element ( CONFIG_NODE, &config_show_fdb_mac_config_cmd );

    /* B--added by liwei056@2011-3-14 for 武汉长宽 */
	install_element ( CONFIG_NODE, &pon_show_onu_fdb_num_cmd );
	install_element ( PON_PORT_NODE,&pon_show_onu_fdb_num_cmd );
    /* E--added by liwei056@2011-3-14 for 武汉长宽 */

    /*added by luh@2014-10-27*/
	install_element ( VIEW_NODE, &pon_show_onu_fdb_num_cmd );

    /*B--add by shixh@2011-12-13*/
    install_element ( CONFIG_NODE, &set_onu_emapper_restore_cmd );
      install_element ( PON_PORT_NODE, &set_onu_emapper_restore_cmd );
    /*E--add by shixh@2011-12-13*/
    

	/* 用于以ONU 为单位,设置业务优先级*/
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	install_element (PON_PORT_NODE,&downlink_set_pon_vlan_trans_tag_cmd );
	install_element (PON_PORT_NODE,&undo_downlink_set_pon_vlan_trans_tag_cmd);
	install_element (PON_PORT_NODE,&uplink_set_pon_vlan_trans_tag_cmd);
	install_element (PON_PORT_NODE,&undo_uplink_set_pon_vlan_trans_tag_cmd);

	install_element (PON_PORT_NODE,&uplink_set_pon_vlan_filter_tag_cmd);

	install_element (PON_PORT_NODE,&set_onu_vlan_add_tag_cmd );
	install_element (PON_PORT_NODE,&set_onu_vlan_strip_tag_cmd );

       /*add by shixh20101207 for ctc onu test*/
       /* install_element (CONFIG_NODE,&onu_set_ctc_enable_state_cmd );
        install_element (CONFIG_NODE,&onu_set_ctc_enable_show_cmd );*/
	install_element (PON_PORT_NODE,&set_onu_uplink_vlan_tpid_cmd );
	install_element (PON_PORT_NODE,&show_onu_uplink_vlan_tpid_cmd );
	install_element ( CONFIG_NODE, &config_show_onu_uplink_vlan_tpid_cmd );
	install_element ( CONFIG_NODE, &config_set_onu_uplink_vlan_tpid_cmd);
	
	/*
	install_element (PON_PORT_NODE,&set_onu_vlan_priority_cmd );
	install_element (PON_PORT_NODE,&undo_set_onu_vlan_priority_cmd );*/
	
	install_element (PON_PORT_NODE,&show_pon_vlan_trans_downlink_cmd);
	install_element (VIEW_NODE,&show_pon_vlan_trans_downlink1_cmd);/*added by luh@2014-10-29*/
	install_element (PON_PORT_NODE,&show_pon_vlan_trans_uplink_cmd );
	install_element (VIEW_NODE,&show_pon_vlan_trans_uplink1_cmd ); /*added by luh@2014-10-29*/
#endif	


	/* 删除PON 地址表项*/
	install_element (PON_PORT_NODE,&delete_olt_pon_address_entry_cmd );
    /* B--added by liwei056@2011-3-9 for Loop-StaticMac */
	install_element ( CONFIG_NODE, &add_olt_pon_static_address_cmd);
	install_element ( CONFIG_NODE, &del_olt_pon_address_cmd);
	install_element ( CONFIG_NODE, &show_olt_pon_address_cmd);
    /* E--added by liwei056@2011-3-9 for Loop-StaticMac */

    /*added by luh@2014-10-27*/
	install_element ( VIEW_NODE, &show_olt_pon_address_cmd);
    
	/*  added by chenfj 2008-4-29
	  11. 命令行作用户权限分级（徐州提的需求），改进部分。（不增加用户权限，扩展normal用户的查询权限，即在VIEW节点下增加查询命令）
	  */
	 /* 以下命令由pon 节点扩展到view  节点*/
	install_element ( VIEW_NODE, &config_show_fdb_mac_config_cmd );
	install_element ( VIEW_NODE, &view_show_bandwidth_config_cmd );
	install_element ( VIEW_NODE, &view_show_llid_bandwith_config_cmd );
	/*install_element ( VIEW_NODE, &view_show_encrypt_config_cmd );
	 install_element ( VIEW_NODE, &view_onu_max_mac_show_cmd );*/
	install_element(VIEW_NODE, &view_show_pon_mac_age_cmd);
	install_element(VIEW_NODE, &show_pon_port_info_config_cmd);
	install_element(VIEW_NODE, &config_show_ponport_version_cmd);
	
    install_element ( VIEW_NODE, &view_pon_port_statstics_show_cmd );
    install_element ( VIEW_NODE, &view_CNI_statstics_show_cmd );
    install_element ( CONFIG_NODE, &view_pon_port_statstics_show_cmd );
    install_element ( CONFIG_NODE, &view_CNI_statstics_show_cmd );
	install_element(VIEW_NODE, &view_show_olt_address_table_configuration_cmd);
	/*整个gpon 口下使能所有onu by jinhl@2016.11.25*/
	install_element(DEBUG_HIDDEN_NODE, &gpon_port_allOnu_en_cmd);

	install_element(CONFIG_NODE, &host_pon_timeout_config_cmd);
	install_element(CONFIG_NODE, &host_pon_timeout_show_cmd);
	install_element ( VIEW_NODE, &host_pon_timeout_show_cmd );

	/*install_element ( PON_PORT_NODE, &link_test_num_of_frames_pon_cmd );*/
	install_element ( VIEW_NODE, &show_onu_vlan_trans_uplink_cmd);
	install_element ( CONFIG_NODE, &show_onu_vlan_trans_uplink_cmd);
	
#if  0
#ifdef  _PON_VIRTUAL_OPTICAL_SCOPE_
	install_element ( CONFIG_NODE, &show_olt_adc_config_cmd );
	install_element ( PON_PORT_NODE, &show_olt_adc_config_pon_cmd );
	install_element ( PON_PORT_NODE, &set_olt_adc_config_cmd );
#endif
#endif

#ifdef  PRODUCT_EPON3_GFA6100_TEST
	install_element ( CONFIG_NODE, &config_add_pon_port_cmd);
	install_element ( CONFIG_NODE, &config_remove_pon_port_cmd);
	#if 0/*此命令功能实现不正确，暂屏蔽该命令by jinhl@2016.07.19*/
	install_element ( CONFIG_NODE, &config_add_pon_card_cmd );
	#endif
	install_element ( CONFIG_NODE, &config_remove_pon_card_cmd );
	install_element ( CONFIG_NODE, &config_test_pon_card_cmd );
#endif

/*#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	PonPortMeteringCliInit();
#endif*/
	install_element (PON_PORT_NODE, &clear_pon_statistic_cmd);	/*add by zhengyt@09-06-03*/
	install_element ( PON_PORT_NODE, &pon_show_fdb_mac_form_which_onu_cmd );
	install_element ( CONFIG_NODE, &config_show_fdb_mac_form_which_onu_cmd);
	install_element ( VIEW_NODE, &config_show_fdb_mac_form_which_onu_cmd);

	install_element ( PON_PORT_NODE, &active_pending_onu_pon_cmd);
	install_element ( PON_PORT_NODE, &pending_onu_list_show_pon_cmd);

	install_element( CONFIG_NODE, &onu_conf_associate_global_cmd);
	/*install_element( CONFIG_NODE, &onu_conf_apply_global_cmd);*/

	install_element ( PON_PORT_NODE, &config_max_onu_counter_cmd);
	install_element ( PON_PORT_NODE, &undo_max_onu_counter_cmd);
	install_element ( PON_PORT_NODE, &show_onu_max_onu_counter_cmd);
	install_element( CONFIG_NODE, &config_default_max_onu_counter_cmd);
	install_element( CONFIG_NODE, &undo_default_max_onu_counter_cmd);
	install_element( CONFIG_NODE, &show_default_max_onu_counter_cmd);

	install_element ( PON_PORT_NODE, &CNI_statstics_cni_datapath_show_cmd);
	install_element ( PON_PORT_NODE, &pon_port_statstics_pon_datapath_show_cmd);
	install_element ( PON_PORT_NODE, &pon_statstics_datapath_uplink_show_cmd);
	install_element ( PON_PORT_NODE, &pon_statstics_datapath_downlink_show_cmd);

    /*added by luh @2015-6-19*/
	if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
	{
        install_element( DEBUG_HIDDEN_NODE, &debug_onu_oam_cmd);	
        install_element( DEBUG_HIDDEN_NODE, &nodebug_onu_oam_cmd);
		/*added by liyang @2015-6-23*/
		install_element(DEBUG_HIDDEN_NODE, &debug_tk_pon_phy_msg_cmd);
	}

	/*added by jinhl@2016.07.19*/
	install_element ( CONFIG_NODE, &reset_pon_chipHard_cmd );
	/*added by wangjiah@2016-08-31*/
	//install_element ( PON_PORT_NODE, &pon_port_reset_10gepon_chip_cmd );
	/*added by wangjiah@2016-08-31*/
	if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
	{
		install_element ( DEBUG_HIDDEN_NODE, &dba_polling_level_cmd);
	}
	return VOS_OK;
}

LONG PON_CliInit()
{  
    pon_node_install();
    pon_module_init();
    PON_CommandInstall();
    
    return VOS_OK;
}

static INT16 cliPonIdCheck(ULONG ulIfIndex)
{
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;	

    if( PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotId, (ULONG *)&port , (ULONG *)&onuId) == VOS_ERROR )
    		return CMD_WARNING;

    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if (phyPonId == VOS_ERROR)
    {
        return VOS_ERROR;
    }
	return phyPonId;
}



#if 0
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



/*added by wutw*/
#if 0
                                                 /* 执行完管理层发起的读/写请求后后的回调函数,用于通知执行结果 */
                                                 /* lReqType 请求类型, pFileName 文件名, lErrCode 错误代码*/
                                                 /* lFileLen 文件长度，lSucLen 当前进度*/
                                                 /* 当 lFileLen = 0(失败的传输) 或 lFileLen > 0 且 lSucLen = lFileLen 时, 表示文件传送完毕 最后一次回调*/
typedef void (*POAMTRANSFILECALLBACK)(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode); 
                                                 /* 执行从ONU读取文件请求时, 当正确接收完文件后, 执行的回调函数 */
												 /* pFileType 文件类型, pFileName 文件名, pFileBuf 文件的接收缓存 lFileLen 文件长度*/
												 /* pFileBuf 由请求发起者释放, 释放函数 free() */ 
typedef void (*POAMRECVFILECALLBACK)(unsigned short usPonID, unsigned short usOnuID, 
                                     char *pFileName, char *pFileBuf, long lFileLen); 

#endif


int fileTranErrParse(long lErrCode,struct vty *vty)
{
	if (lErrCode == 0)
	{}
	else if ( (lErrCode > 0x18) || (lErrCode < 1) )
		vty_out( vty, "  %% Unkowned errcode!\r\n");
	else
		vty_out( vty,"%s",fileTranErrStr[lErrCode]);
	return VOS_OK;

}
int CliOnuLogFileTranResult(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode)
{
	if (lErrCode != 0)
	{
		vty_out( file_vty, "\r\n  %% %s, length %d" ,pFileName,lFileLen);
		fileTranErrParse(lErrCode, file_vty);
		
	}
	return 0;
}





int CliOnuLogFileTranData(unsigned short usPonID, unsigned short usOnuID, 
                                char *pFileName, char *pFileBuf, long lFileLen)
{
	char fileBuf[65540] = {0};
	long length = 0;
	
	if (pFileBuf == NULL)
		return (-1);
	
	if( lFileLen >  65535 )
		{
		vty_out ( file_vty, "  %% log.txt is too long!\r\n");
		length = 65535;
		}
	else length = lFileLen;

	memcpy(fileBuf,pFileBuf,length);
	fileBuf[length] = '\0';
	fileBuf[length+1] = '\0';

	/*
	if (lFileLen < 65536)
	{
		memcpy(fileBuf,pFileBuf,lFileLen);
		fileBuf[lFileLen] = '\0';
		fileBuf[lFileLen+1] = '\0';
	}
	else
	{
		vty_out ( file_vty, "  %% log.txt is too long!\r\n");
		VOS_Free(pFileBuf);
	}	
	length = strlen(fileBuf);
	*/
	vty_out( file_vty, "\r\n  %s  length %d(byte) :\r\n",pFileName,lFileLen);
	vty_big_out (file_vty, length, "%s\r\n\r\n",fileBuf);
	
	VOS_Free(pFileBuf);
	pFileBuf = NULL;
	return 0;
}


int CliOnuLogFileTUploadResult(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	if (lErrCode != 0)
	{	/*
		if( usPonID < 4 ) { ulSlot = 8; }
		else if(( usPonID < 8 ) && ( usPonID >= 4 )) { ulSlot = 7; }
		else if(( usPonID < 12 ) && ( usPonID >= 8 )) { ulSlot = 6; }
		else if(( usPonID < 16 ) && ( usPonID >= 12 )) { ulSlot = 5; }
		else if(( usPonID < 20 ) && ( usPonID >= 16 )) { ulSlot = 4; }

	 	if(usPonID >= 20)
			return( RERROR );
		
		ulPort = ( usPonID % 4 + 1 );
		*/
		ulSlot = GetCardIdxByPonChip(usPonID);
		ulPort = GetPonPortByPonChip(usPonID);
	
		vty_out( file_vty, "\r\n  %% Upload %d/%d/%d log.txt failed! \r\n",ulSlot, ulPort, usOnuID );
		fileTranErrParse( lErrCode, file_vty);
		return lErrCode;
		
	}
	return 0;
}
int CliOnuLogFileUpload(unsigned short usPonID, unsigned short usOnuID, 
                                char *pFileName, char *pFileBuf, long lFileLen)
{

	
	/*sys_console_printf(  "buf ptr = %d, len=%d\r\n", (int )pFileBuf, lFileLen);*/
	
	return (send_by_ftp_Api( "ftp", cliUserIp, cliUserName, 
							cliUserPassWord, pFileName, pFileBuf, 
							(char *)(&lFileLen)) );
	return 0;
}



#ifdef	__cplusplus
}
#endif/* __cplusplus */
