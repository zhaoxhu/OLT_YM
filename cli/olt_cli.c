/****************************************************************************
*
*     Copyright (c) 2006 GWTT Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           GWTT Corporation
*
*     All information contained in this document is GWTT Corporation
*     company private, proprietary, and trade secret.
*
*	  modified :wutw 2006-10-18
*			修改因为配置命令改变:devicename 改为device name等而showrun代码
*			中没有对这些命令进行修改.
*	  modified :wutw 2006-11-10
*			增加由隋平礼提供的,用于解析端口列表参数与onuid列表参数的相关代码
*	  modified :wutw 2006/11/17
*			增加show onu-list命令
*	  modified :wutw 2006/11/20
*			增加以下代码
*			show onu-list {count}*1
*			show onu-list pon <slot/port> {count}*1
*			show online {count}*1
*			show onuline pon <slot/port> {count}*1	
*			onu software auto-update与undo onu software auto-update
*			onu software auto-update <slot/port> { <onuid-list >}*1
*			与onu software auto-update <slot/port> { <onuid-list >}*1
*			show onu software auto-update 
*			show onu software auto-update [enable|disable]
*			show onu software auto-update [enable|disable] pon <slot/port>
*	  modified :wutw 2006/11/24
*			使用pclint检查，修改代码。没有告警与错误
*	  modified wutw 2006/12/05
*			增加查看处于环回状态的onu信息
*	  modified wutw 2006/12/15
*			增加设置测量距离，显示测量距离的命令
*     modified by chenfj 2007/04/26
*         问题单#4298 : 在config节点下添加命令show onu-version <slot/port> <onu-list>，没有<slot/port>参数时，显示所有ONU版本。没有ONU参数时，显示当前PON端口下所有ONU版本
*     added by chenfj 2007-6-21  
*       增加PON 端口保护切换  
* 	 added by chenfj 2007-7-6
*	     for onu authenticaion by PAS5001 & PAS5201
*         注: 在PAS5001上，onu authentication是由软件来实现的；在PAS5201上，若启动CTC协议栈，onu authentication是由PON芯片硬件实现的；若没有启动CTC协议栈，onu authentication 也是由 软件来实现。
*     added by chenfj 2007-6-27  
*          PON 端口管理使能: up / down  
*     added by chenfj 2007-7-31 
*	    PON端口BER / FER 告警
*     问题单#5385: modified by chenfj 2007-9-19
*		在CONFIG节点下，使用命令show auto-protect pon slot/port 显示当前处于激活状态的
*		PON端口错误，处于激活状态的总是命令中输入的端口     
*	  问题单#5379  added by chenfj 2007-9-20 
*        没有专门的命令查看当前的PON口保护倒换检测时间值
*      modified by chenfj 2008-7-8
*          增加产品类型识别(GFA6700/GFA6100),
*****************************************************************************/


#ifdef	__cplusplus
extern "C"
{
#endif

#include "syscfg.h"


#include "vos_global.h"
#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "vos/vospubh/vos_ctype.h"
#include "vos/vospubh/vos_byteorder.h"
#include "vos/vosprvh/vos_page.h"
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

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
/*#include "sys/main/sys_main.h"*/
#include "cpi/ctss/ctss_ifm.h"

/*#include "V2R1General.h"*/
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
/*#include "V2R1_product.h"
#include "eventMain.h"*/
#include "olt_cli.h"
#include "V2R1debug_cli.h"

#include "gwEponSys.h"
#include "CT_gwIgmpSnoopAuth.h"

#include "Typesdb_product.h"
#include "Typesdb_module.h"

#include "device_hardware.h"
#include "Device_flash.h"
#include "../onu/OnuPortStatsMgt.h"

#if ((defined ONU_PPPOE_RELAY)||(defined ONU_DHCP_RELAY))
#include "access_identifier/access_id.h"/*wugang add for pppoe relay*/
#endif
#include "i2c.h"
/*#include "../statistics/statistics.h"*/
/*#include "../include/OnuGeneral.h"*/
/*#include "pon_cli.h"*/
/*#define CLI_OLT_NAME_MAXLEN	255 */
#define START_VLAN 1
#define END_VLAN   4094
#define CLI_EPON_ETH					4
#define CLI_EPON_LLID					3
#define CLI_EPON_ONU					2
#define CLI_EPON_PON					1
#define CLI_EPON_PONMIN			0
#define CLI_EPON_PONMAX			(MAXPON-1)  /* 19 */
#define CLI_EPON_ONUMAX			(MAXONUPERPON-1)  /*63*/
#define CLI_EPON_ONUMIN			0
#define	CLI_EPON_PON_EN		V2R1_ENABLE  /*1*/
#define	CLI_EPON_PON_DIS		V2R1_DISABLE /*2*/

#define	CLI_EPON_CARDINSERT			CARDINSERT   /*1*/
#define CLI_SOFTWARE_UPDATE_IMMEDIATELY 	1
#define CLI_SOFTWARE_UPDATE_DELAY    2
/*#define ONU_TRANSMISSION_ENABLE 1
#define ONU_TRANSMISSION_DISABLE 0*/
#define UPDATE_PENDING_QUEUE_TIME 120
extern int cl_vty_user_limit;
int OnuTransmission_flag = ONU_TRANSMISSION_ENABLE;/*added by luh 2011-9-24 GW-ONU 透传标志*/
int UpdatePendingQueueTime = UPDATE_PENDING_QUEUE_TIME; /*added by luh 2013-7-3*/
/*#undef CLI_OLT_DEBUG*/ 
/*
#define  ONU_FLASH_FILE_ID  5
#define  OLT_FLASH_FILE_ID  11

#define  PON_FW_FLASH_ID  7
#define  PON_DBA_FLASH_ID   10
*/
/*
#ifndef MAXONUPERPON 
#define  MAXONUPERPON  64
#endif
*/
#define MIP_Descrition "Please input group-ip(224.0.1.0-239.255.255.255)\n"

/*extern ULONG * applyforaddress( struct vty * vty ) ;
extern void judgePonId( ULONG addr , ULONG idx , struct vty * vty );
extern void printCtcCfgData( struct vty * vty , uchar *addr );*/
extern ULONG * V2R1_Parse_OnuId_List( CHAR * pcOnuId_List );
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern LONG TDM_CliInit();
/* begin: added by jianght 20090223  */
extern LONG E1_CliInit();
/* end: added by jianght 20090223 */
#endif
extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern void WriteCPLDReg( unsigned char * RegAddr, unsigned char ucIntData );
extern int vos_inet_aton ( const char *cp, struct in_addr *inaddr );
extern ulong_t getFtpTaskQueId(void);
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );

extern int AllOnuCounter( short int PonPortIdx );

extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
extern STATUS	isHaveAuthMacAddress( ULONG slot, ULONG pon, const char* mac,  ULONG *onu );
extern STATUS	getOnuAuthEnable(ULONG slot, ULONG port, ULONG *enable );
extern STATUS	setOnuAuthEnable(ULONG slot, ULONG port, ULONG	 enable );
extern STATUS	setOnuAuthEnableForSinglePon(ULONG slot, ULONG port, ULONG enable );
extern STATUS 	setOnuAuthMacAddress( ULONG slot, ULONG pon, ULONG onu, CHAR *macbuf );
extern STATUS 	getOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu, ULONG *st );
extern STATUS 	setOnuAuthStatus( ULONG slot, ULONG pon, ULONG onu,  ULONG st );

extern LONG getCtcStackOui( ULONG slotno, UCHAR *p_oui, UCHAR *p_oui_ver );	/* modified by xieshl 20110721 */
extern LONG setCtcStackOui( ULONG slotno, UCHAR *p_oui, UCHAR oui_ver );
extern char *tdm_vendor_id2name( unsigned long vendor_id );
extern char *tdm_e1_impedance2name( unsigned long impedance );
extern int portListLongToString(ULONG list, char *str);    /*added by luh 2012- 3-12*/
extern int ShowPonPortOffLineOnuCounterByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );
extern int ShowPonPortOffLineOnuCounterByVtyAll(unsigned char *OnuTypeString, struct vty *vty );
extern int ShowPonPortOffLineOnuByVtyAll(unsigned char *OnuTypeString, struct vty *vty );
extern int ShowPonPortOffLineOnuByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );

extern long device_sysfile_read_flash_to_mem_deflate( CHAR* memfile, LONG * memfile_length );

extern char *pon_ctc_support2name( unsigned long ctc_support );
/*
typedef struct{
	char dev_type[16];
	ULONG loc_offset;
	ULONG file_len;
	ULONG compress_flag;
	char  reserve[8];
	char  file_ver[92];
}__attribute__((packed))app_desc_t;
*/
/*extern void CommOlt_HadleCli(unsigned short   PonId,
                       unsigned short	OnuId,
                       unsigned short   Length,
                       unsigned char	*pFrame,
                       unsigned char	*pSessionField);*/
extern LONG EPON_DEBUG_CommandInstall();
extern LONG ONU_CliInit();
extern LONG PON_CliInit();
extern int  GetOltSWVersion( char *Version, int *len);
extern STATUS setActivateTime( const char *val, const ulong_t valLen );
extern STATUS setActInstanterEnable( const ulong_t val );
extern STATUS getActInstanterEnable( ulong_t *pValBuf );
extern STATUS getActivateTime( char *pValBuf, ulong_t *pValLen );
extern STATUS addIgmpSnoopAuthItem( ulong_t onuid, ulong_t vlanid, ulong_t groupip, 
										uchar_t usrmac[], ulong_t mcastVid );
extern STATUS delIgmpSnoopAuthItem( ulong_t onuid, ulong_t vlanid, ulong_t groupip, uchar_t usrmac[] );
extern int ShowOnuDeviceInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern short int  GetOnuIdxByName( unsigned char *Name );
extern LONG userport_is_pon (int slot, int port);
#if 0
extern STATUS checkGwIgmpSnoopAuthEntry( ulong_t onuIdx, ulong_t vlanid, ulong_t groupip, uchar_t mac[] );
extern STATUS setGwIgmpSnoopAuthStatus( ulong_t onuIdx, ulong_t vlanid, ulong_t groupip, uchar_t mac[], ulong_t status );
extern STATUS getFirstGwIgmpSnoopAuthEntry( ulong_t *onuIdx, ulong_t *vlanid, ulong_t *groupip, uchar_t mac[] );
extern STATUS getGwIgmpSnoopAuthUserIP( ulong_t onuIdx, ulong_t vlanid, ulong_t groupip, uchar_t mac[], ulong_t *usrip );
extern STATUS getGwIgmpSnoopAuthStatus( ulong_t onuIdx, ulong_t vlanid, ulong_t groupip, uchar_t mac[], ulong_t *status );
extern STATUS getNextGwIgmpSnoopAuthEntry( ulong_t onuIdx, ulong_t vlanid, ulong_t groupip, uchar_t mac[],
	                                          ulong_t *nextonuIdx, ulong_t *nextvlanid, ulong_t *nextgroupip, uchar_t nextmac[]);
#endif

extern STATUS checkIgmpSnoopEntry( ULONG, ULONG, ULONG, ULONG, ULONG );
extern STATUS getFirstIgmpSnoopAuthEntry( ULONG *, ULONG *, ULONG*, ULONG *, ULONG * );
extern STATUS getNextIgmpSnoopAuthEntry( ULONG, ULONG, ULONG, ULONG, ULONG, ULONG*, ULONG*, ULONG*, ULONG*, ULONG* );


extern int SetOnuRegisterLimitFlagAll( int flag );
extern int  GetOnuRegisterLimitFlag(short int PonPortIdx, int *flag );
extern STATUS CliHisStatsPonCtrlAllGet(struct vty* vty) ; 
extern STATUS CliHisStatsONUCtrlAllGet(struct vty* vty) ; 
extern STATUS CliHisStatsPonCtrlGet( short int ponId, struct vty* vty ) ;
extern STATUS CliHisStatsONUCtrlGet(short int ponId, short int onuId, struct vty* vty) ; 
extern STATUS HisStats15MinMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue);
extern STATUS HisStats15MinMaxRecordGet(unsigned int *pValue);
extern STATUS HisStatsDefaultRecordGet(unsigned int *pDefBucket_15M, unsigned int *pDefBucket_24H); 
/*extern  SYS_MODULE_TYPE(slotno);*/
extern int  SetOnuSWUpdateCtrl(short int PonPortIdx, short int OnuIdx, unsigned char EnableFlag);
/*extern LONG event_show_run( struct vty * vty );*/
extern ULONG ulIfindex_2_userSlot_userPort( ULONG ulIfIndex, ULONG *ulUserslot, ULONG *ulUserport );
extern int SetPonRange(short int PonPortIdx , int range);
extern int GetPonRange( short int PonPortIdx, int *range );
extern int i2c_data_set( UINT slot_id, UINT type, UINT reg, UCHAR *pdata, UINT len);
extern int Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TIMER_NAME_E timer_name,ULONG * timeout);
extern LONG Onustats_EnableIs();   
extern void show_run_onu_model( struct vty * vty );
extern STATUS getCtcIgmpSnoopAuthItem( UCHAR slot, UCHAR pon, USHORT onu, USHORT cvlan, ULONG gda, IGMP_AUTH_INFO *pItem );

/*
#if( RPU_MODULE_IGMP_TVM == RPU_YES )
extern STATUS CliIgmpTvmShowRun(struct vty *vty);
#endif
*/
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
extern LONG trace_path_show_run( struct vty * vty );
#endif

/*extern int eventGetCurTime( sysDateAndTime_t *pDateTime );*/
ULONG * V2R1_Parse_OnuId_List_1( CHAR * pcOnuId_List );
enum match_type V2R1_Check_OnuId_List_1( char * onuid_list );

STATUS cliGetLocation( ulong_t deviceIndex, ulong_t *ulSlot,  ulong_t *ulPort, ulong_t *ulOnuId)
{ 
	STATUS rc = VOS_ERROR;
	if ( (NULL == ulSlot) || (NULL == ulPort) || (NULL == ulOnuId) )
		{
		return rc;
		}

	*ulSlot = GET_PONSLOT(deviceIndex);
	*ulPort = GET_PONPORT(deviceIndex);
	*ulOnuId = GET_ONUID(deviceIndex);

	if(PonCardSlotPortCheckByVty(*ulSlot,*ulPort, 0) != ROK )
		return(rc);
	if(*ulOnuId <= MAXONUPERPON)
		return(VOS_OK);
	else return(rc);
	/*
	if( (*ulSlot<=8)&&(*ulPort<=4)&&(*ulOnuId<=64) )
		{
		return VOS_OK;
		}
	else
		return rc;
	*/
}

/* added by xieshl 20110707, 检查输入的端口号是否正确，以解决当pon portno过大时，会操作到其它槽位上的PON，问题单13231 */
LONG olt_pon_idx_check( ULONG slotno, ULONG portno, struct vty *vty )
{
    short int sOltStartPort;
    short int sOltEndPort;

	if( SlotCardIsPonBoard(slotno) != VOS_OK )
	{
		if( vty ) vty_out( vty, "  %% Error slot %d is not PON board.\r\n", slotno  );
		return VOS_ERROR;
	}

    (void)GetSlotPonPortPhyRange(slotno, &sOltStartPort, &sOltEndPort);
    
	if( (portno == 0)
/* B--modified by liwei056@2014-2-27 for 16PON-Card */
#if 1
        || (portno > sOltEndPort)
#else
/* B--modified by liwei056@2012-2-22 for D14487 */
#if 1
        || (portno > GetSlotCardPonPortRange(0, slotno))
#else
        || (portno > GetSlotCardPonPortNum(slotno))
#endif
/* E--modified by liwei056@2012-2-22 for D14487 */
#endif
/* E--modified by liwei056@2014-2-27 for 16PON-Card */
        )
	{
		if( vty ) vty_out( vty, " %% Error pon %d/%d is not exist.\r\n", slotno, portno  );
		return VOS_ERROR;
	}

    /* B--added by liwei056@2012-3-23 for D14620 */
    /* B--modified by liwei056@2014-2-27 for 16PON-Card */
#if 1
    if ( portno < sOltStartPort )
#else
    if ( SYS_MODULE_IS_SWITCH_PON(slotno) && (portno < 5) )
#endif
    /* E--modified by liwei056@2014-2-27 for 16PON-Card */
    {
		if( vty ) vty_out( vty, "  %% Error port %d/%d is ETH port.\r\n", slotno, portno  );
		return VOS_ERROR;
    }
    /* E--added by liwei056@2012-3-23 for D14620 */

	return VOS_OK;
}



DEFUN (
    show_olt_sysfile_ver,
    show_olt_sysfile_ver_cmd,
    "show sysfile-version",
    DescStringCommonShow
    "Display sysfile version\n"
    )
{
    LONG lRet = VOS_OK;
    char ver[100] = {0}; 
    GetOltSysfileVer(ver);
    vty_out(vty, " Olt current sysfile version : %s\r\n\r\n", ver);
    return CMD_SUCCESS;
}


DEFUN (
    olt_device_name,
    olt_device_name_cmd,
    "device name <name>",
    "Config olt device info\n"
    "Config olt name\n"
    "Please input olt name(no more than 127 characters)\n"
    )
{
    LONG lRet = VOS_OK;
    int  len = strlen(argv[0]);

    if(len <= MAXDEVICENAMELEN )
    	{/*modi by luh 2014-1-10*/
#if 0            
		lRet = SetOltDeviceName( argv[0], len );
#else
		lRet = OLT_SetDeviceName(OLT_ID_ALL, argv[0], len );
#endif
    	}
	else
        {
            vty_out( vty, "  %% Device name is too long\r\n" );
    	    return CMD_WARNING;
        }		
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
	
    return CMD_SUCCESS;
}

DEFUN (
    undo_olt_vendor_name,
    undo_olt_vendor_name_cmd,
    "undo device name",
    "restore device name to default\n"
    "restore olt device info\n"
    "restore olt name to default\n"
    )
{
    LONG lRet = VOS_OK;
    int len = 0;
    char DeviceName[MAXDEVICENAMELEN];

	VOS_MemZero(DeviceName, sizeof(DeviceName) );
    GetOltDeviceNameDefault(DeviceName, &len);
    if(len > MAXDEVICENAMELEN )
		len = MAXDEVICENAMELEN;

#if 0            
    lRet = SetOltDeviceName( DeviceName, len );
#else
	lRet = OLT_SetDeviceName(OLT_ID_ALL, DeviceName, len );
#endif
   	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
	
    return CMD_SUCCESS;
}


/*已被注释*/
#if 0
DEFUN (
    olt_vendor_addr,
    olt_vendor_add_cmd,
    "device address <address>",
    "Config olt device info\n"
    "Config olt address\n"
    "Please input olt address(no more than 255 characters)\n"
    )
{
    LONG lRet = VOS_OK;
    int  len = 0;
    len = strlen(argv[0]);
    if ( len > CLI_OLT_NAME_MAXLEN )
        {
            vty_out( vty, "  %% Parameter is error. Character string is too long\r\n" );
    	    return CMD_WARNING;
        }	
		
	lRet = SetOltDeviceDesc( argv[0], len );
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Excuting error.\r\n" );
	return CMD_WARNING;
    }	   	
     
    return CMD_SUCCESS;
}

/*已被注释*/
/*The cli cmd had been deleted*/
DEFUN (
    olt_vendor_contac,
    olt_vendor_contac_cmd,
    "vendorcontact <contac>",
    "Config vendor contact phone\n"
    "Please input the vendor contact phone(e.g.: 010-62961177), no longer than 255 characters\n"
    )
{


    return CMD_SUCCESS;
}
#endif

DEFUN (
    olt_device_location,
    olt_device_location_cmd,
    "device location <location>",
    "Config olt device info\n"
    "Config olt location\n"
    "Please input olt location( no more than 128 characters)\n" )
{
    LONG lRet = VOS_OK;
    int  len = strlen(argv[0]);

    if ( len <= MAXLOCATIONLEN)
    	{
		lRet = SetOltLocation( argv[0], len );
    	}
	else
        {
            vty_out( vty, "  %% Device location is too long\r\n" );
    	    return CMD_WARNING;
        }			
	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
        
    return CMD_SUCCESS;
}

DEFUN (
    undo_olt_device_location,
    undo_olt_device_location_cmd,
    "undo device location",
    "restore device location to default\n"
    "restore olt device info\n"
    "restore olt location to default\n"
	)
{
    LONG lRet = VOS_OK;
    int  len = 0;
    char DeviceLoaction[MAXLOCATIONLEN]={'\0'};

    GetOltLocationDefault(DeviceLoaction, &len);
	
    if ( len> MAXLOCATIONLEN)
		len = MAXLOCATIONLEN;
    	
    lRet = SetOltLocation( DeviceLoaction, len );	
	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
        
    return CMD_SUCCESS;
}

DEFUN (
    olt_device_description,
    olt_device_description_cmd,
    "device description <description>",
    "Config olt device info\n"
    "Config olt description\n"
    "Please input olt description( no more than 128 characters)\n"
    )
{
    LONG lRet = VOS_OK;
    int  len = 0;
    len = strlen(argv[0]);

    if ( len <= MAXDEVICEDESCLEN)
    	{
    	#ifdef CLI_OLT_DEBUG
		vty_out( vty, "  cli : devicedescription %s  len = %d\r\n",argv[0], len);
		#endif
		lRet = SetOltDeviceDesc( argv[0], len );
    	}
	else
        {
            vty_out( vty, "  %% Parameter is error. Character string is too long\r\n" );
    	    return CMD_WARNING;
        }			
	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
        
    return CMD_SUCCESS;
}

DEFUN (
    undo_olt_device_description,
    undo_olt_device_description_cmd,
    "undo device description",
    "restore device description\n"
    "restore olt device info\n"
    "restore olt description to default\n"
    )
{
    LONG lRet = VOS_OK;
    int  len = 0;
    char DeviceDesc[MAXDEVICEDESCLEN] = {'\0'};

    GetOltDeviceDescDefault(DeviceDesc,&len);

    if ( len > MAXDEVICEDESCLEN)
		len = MAXDEVICEDESCLEN;
	
    lRet = SetOltDeviceDesc( DeviceDesc, len );		
	
    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
        
    return CMD_SUCCESS;
}


/*This command can be used under view point*/
DEFUN (
    show_olt_device_info,
    show_olt_device_info_cmd,
    "show device information",
    DescStringCommonShow
    "Show olt device in the system\n"
    "Device information\n"
    )
{
    ShowOltInfoByVty(vty);
    return CMD_SUCCESS;
}

#if 0
/*olt 的运行环境*/
DEFUN (
    show_olt_surrounding_info,
    show_olt_surrounding_info_cmd,
    "show olt surrounding [board|pwu|fan]",
    DescStringCommonShow
    "Show olt device in the system\n"
    "Show olt device surrounding\n"
    "Board information\n"
    "Pwu information\n"
    "Fan information\n"
    )
{
        
    return CMD_SUCCESS;
}



/*This cli cmd had been moved to node of debug.
This command can be used under view point*/
/*此命令最好拆作两条，否则的话，在shell底下显示有些别扭。*/

QDEFUN (
    show_onu_llid_mapping_info,
    show_onu_llid_mapping_info_cmd,
    "show onu-llid-mapping [onu|llid] [<onuid>|<llid>]",
    DescStringCommonShow
    "Show onu-llid-mapping in the system\n"
    "Show all llid to the given onu in the system\n"
    "Show onu to the given llid in the system\n"
    "Please input the Onu ID\n"
    "Please input the Llid\n",
    &g_ulIFMQue )
{
        
    return CMD_SUCCESS;
}

#endif





DEFUN (
    show_history_stats_table_info,
    show_history_stats_table_info_cmd,
    "show statistic-history table [pon|onu]",
    DescStringCommonShow
    "Show history-stats information\n"
    "Show the table in the system\n"
    "Pon type\n"
    "Onu type\n"
    )
{
    /*LONG lRet = VOS_OK; */
    LONG devType = 0;	
	/*INT32	slotId = 0;
	short int port = 0;
	unsigned short ponId = 0;
	unsigned short onuId = 0;*/
	
    if (!VOS_StrCmp((CHAR *)argv[0], "pon"))
    {
    	devType = CLI_EPON_PON;
    }
    else /*if (!VOS_StrCmp((CHAR *)argv[0], "onu"))*/
    {
      devType = CLI_EPON_ONU;
    }
    /*else if (!VOS_StrCmp((CHAR *)argv[0], "llid"))
    {
      devType = CLI_EPON_LLID;
    }
    else if (!VOS_StrCmp((CHAR *)argv[0], "ethernet"))
    {
      devType = CLI_EPON_ETH;
    }
    else
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;	
    }   */

	switch(devType)
	{
	    case CLI_EPON_PON:
			vty_out(vty, "\r\n  History statisitics table of pon`s port(slot/port):\r\n");
			CliHisStatsPonCtrlAllGet(vty);
			vty_out(vty, "\r\n");	
			break;
		case CLI_EPON_ONU:
			vty_out(vty, "\r\n  History statisitics table of onu(slot/port/onu):\r\n");
			CliHisStatsONUCtrlAllGet(vty) ;
			vty_out(vty, "\r\n");	
			break;
		case CLI_EPON_LLID:
			break;
		case CLI_EPON_ETH:
			break;
		default:
			return CMD_WARNING;
	}
        
    return CMD_SUCCESS;
}

#if 0
/*告警与门限类的已被注释.不在该olt节点之下*/
DEFUN (
    alarm_enable,
    alarm_enable_cmd,
    "alarm <alarm_name> [enable|disable]",
    "Set alarm\n"
    "Alarm name in the system\n"
    "Enable/disable the alarm mask\n"
    "Disable/disable the alarm mask\n"
    )
{
        
    return CMD_SUCCESS;
}

/*the threshold is unknown*/
DEFUN (
    alarm_threshold_set,
    alarm_threshold_set_cmd,
    "alarm <alarm_name> threshold <threshold_value>",
    "Set alarm\n"
    "Alarm name in the system\n"
    "Set alarm threshold(1-?)\n"
    "Threshold value\n"
    )
{
        
    return CMD_SUCCESS;
}

DEFUN (
    alarm_temperature_threshold_set,
    alarm_temperature_threshold_set_cmd,
    "alarm board [pwu|pon|sw|up-link] temperature threshold <raising_value> <falling_value>",
    "Set alarm\n"
    "Alarm board in the system\n"
    "Board is PWU type\n"
    "Board is PON type\n"
    "Board is SW type\n"
    "Board is UP-LINK type\n"
    "Temperature alarm\n"
    "Set temperature threshold\n"
    "Up limit value\n"
    "Down limit value\n"
    )
{
        
    return CMD_SUCCESS;
}

DEFUN (
    show_alarm_info,
    show_alarm_info_cmd,
    "show alarm <alarm_name> information",
    DescStringCommonShow
    "Show alarm\n"
    "Alarm name in the system\n"
    "Alarm information\n"
    )
{
        
    return CMD_SUCCESS;
}

DEFUN (
    show_last_alarm_info,
    show_last_alarm_info_cmd,
    "show last alarm",
    DescStringCommonShow
    "Show the last one\n"
    "Show alarm\n")
{
        
    return CMD_SUCCESS;
}

DEFUN (
    show_alarm_log,
    show_alarm_log_cmd,
    "show alarm log {time <start_time> <end_time>}*1 {alarm-class <level>}*1",
    DescStringCommonShow
    "Show alarm\n"
    "Show alarm log\n"
    "Alarm time\n"
    "Start time\n"
    "End time\n"
    "The alarm class \n"
    "The alarm class level(1-5)\n"
    )
{
        
    return CMD_SUCCESS;
}


/*此命令最好拆作两条，否则的话，在shell底下显示有些别扭。*/

DEFUN (
    start_self_test_board,
    start_self_test_board_cmd,
    "self-test [board|module] [<slotid>|<moduleid>]",
    "Self test\n"
    "Start self test board\n"
    "Start self test module\n"
    "Please input self test board's slot ID\n"
    "Please input self test module ID\n"
    )
{
        
    return CMD_SUCCESS;
}


/*已注释*/
DEFUN (
    clear_config,
    clear_config_cmd,
    "clear configure",
    CLEAR_STR
    "The configuration value\n"
    )
{
        
    return CMD_SUCCESS;
}

DEFUN (
    ftp_auto_download,
    ftp_auto_download_cmd,
    "ftp auto-download [enable|disable]",
    "Ftp client command\n"
    "Ftp client auto download\n"
    "Ftp client auto download enable\n"
    "Ftp client auto download disable\n"
    )
{
        
    return CMD_SUCCESS;
}

DEFUN (
    ftp_auto_download_time_set,
    ftp_auto_download_time_set_cmd,
    "ftp auto-download [enable|disable] time <year> <month> <day> <hour> <minute> <second>",
    "Ftp client command\n"
    "Ftp client auto download\n"
    "Ftp client auto download enable\n"
    "Ftp client auto download disable\n"
    "The auto-download time\n"
    "Year(e.g.:2006)\n"
    "Month(e.g: 01, 11)\n"
    "Day(e.g.: 01, 11)\n"
    "Hour(e.g.: 01, 11)\n"
    "Minute(e.g.: 01, 11)\n"
    "Second(e.g.: 01, 11)\n"
    )
{
        
    return CMD_SUCCESS;
}
#endif


#if 0
  问题单8226 
DEFUN (
    olt_software_auto_update_enable,
    olt_software_auto_update_enable_cmd,
    "software auto-update time <1980-2079> <1-12> <1-31> <0-23> <0-59> <0-59>",
    "Config software auto-update\n"
    "Config software auto-update time\n"
    "Config software auto-update time\n"
    "Year(e.g.:2006)\n"
    "Month(e.g: 01, 11)\n"
    "Day(e.g.: 01, 11)\n"
    "Hour(e.g.: 01, 11)\n"
    "Minute(e.g.: 01, 11)\n"
    "Second(e.g.: 01, 11)\n"
    )
{
	LONG lRet = VOS_OK;
	unsigned long year = 0;
	unsigned long month = 0;
	unsigned long day = 0;
	unsigned long hour = 0;
	unsigned long minute = 0;
	unsigned long second = 0;
	char timeval[8] = {0};
	unsigned long valLen= 8;
	
	/*unsigned long ulRetDate = 0;
	unsigned long ulRetTime = 0;
	unsigned long ulRetMillSec = 0;*/
	
	unsigned short current_year = 0;
	unsigned char current_month = 0;
	unsigned char current_day = 0;
	unsigned char current_hour = 0;
	unsigned char current_minute = 0;
	unsigned char current_second = 0;
	sysDateAndTime_t dateTime;
	memset(&dateTime, 0, sizeof(sysDateAndTime_t));
	


	year = VOS_AtoL( argv[ 0 ] ); 
	month = VOS_AtoL( argv[ 1 ] ); 
	day = VOS_AtoL( argv[ 2 ] ); 
	hour = VOS_AtoL( argv[ 3 ] ); 
	minute = VOS_AtoL( argv[ 4 ] );
	second = VOS_AtoL( argv[ 5 ] ); 
	eventGetCurTime( &dateTime );
	current_year = dateTime.year;
	current_month = dateTime.month;
	current_day = dateTime.day;
	current_hour = dateTime.hour;
	current_minute = dateTime.minute;
	current_second = dateTime.second;
	
	/*VOS_GetCurrentTime( &ulRetDate, &ulRetTime, &ulRetMillSec );*/
	/*current_year =  ((ulRetDate & 0xffff0000)>>16);*/
	/*系统时间是否正确*/
	/*if ( current_year < 2006 )
	{
		vty_out( vty, "  %% System`s time is err!\r\n");
		return CMD_WARNING; 
	}*/
	if ( year < current_year )
	{
		vty_out( vty, "  %% Invalid time! System`s time : %d-%d-%d %d:%d:%d\r\n",\
				current_year,current_month,current_day,current_hour,
				current_minute,current_second);
		return CMD_WARNING; 
	}
	/*current_month =  ((ulRetDate & 0xff00)>>8);*/
	if ( (current_year == year) && (month < current_month ) )
	{
		vty_out( vty, "  %% Invalid time! System`s time : %d-%d-%d %d:%d:%d\r\n",\
				current_year,current_month,current_day,current_hour,
				current_minute,current_second);
		return CMD_WARNING; 
	}
	/*current_day =   (ulRetDate & 0xff);*/
	if ( (current_year == year) && (current_month == month ) && (day < current_day ) )
	{
		vty_out( vty, "  %% Invalid time! System`s time : %d-%d-%d %d:%d:%d\r\n",\
				current_year,current_month,current_day,current_hour,
				current_minute,current_second);
		return CMD_WARNING; 
	}		
	/*current_hour =  ((ulRetTime & 0xffff0000)>>16);*/
	if ( (current_year == year) && (current_month == month ) && (current_day == day) \
		&& (hour < current_hour ) )
	{
		vty_out( vty, "  %% Invalid time! System`s time : %d-%d-%d %d:%d:%d\r\n",\
				current_year,current_month,current_day,current_hour,
				current_minute,current_second);
		return CMD_WARNING; 
	}
	/*需设置5分钟后的时间有效*/
	/*current_minute =  ((ulRetTime & 0xff00)>>8);*/
	
	/*有效时间设为大于等于当前系统时间5minutes*/
	if ((current_year == year) && (current_month == month ) && (current_day == day) \
	&&(current_hour == hour ) && (minute < (current_minute+5 )))
	{
		vty_out( vty, "  %% Invalid time! More than five minutes of system`s time.\r\n");
		vty_out( vty, "  %% System`s time : %d-%d-%d %d:%d:%d\r\n",\
				current_year,current_month,current_day,current_hour,
				current_minute,current_second);
		return CMD_WARNING; 
	}

	*(unsigned short*)timeval = (unsigned short)year;
	timeval[2] = (char)month;
	timeval[3] = (char)day;
	timeval[4] = (char)hour;
	timeval[5] = (char)minute;
	timeval[6] = (char)second;
	
	lRet = setActivateTime( timeval,  valLen );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING; 
	}
	/*启动定时更新*/
	lRet = setActInstanterEnable( CLI_SOFTWARE_UPDATE_DELAY );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING; 
	}
	vty_out( vty, " software auto-update next time : %d-%d-%d %d:%d:%d\r\n",year,month,day,
			hour,minute,second);	
    return CMD_SUCCESS;
}



DEFUN (
    olt_software_auto_update_disable,
    olt_software_auto_update_disable_cmd,
    "undo software auto-update time",
    NO_STR
    "delete software auto-update time\n"
    "delete software auto-update time\n"
    "delete software auto-update time\n"
    )
{
	LONG lRet = VOS_OK;
    /*设置立即更新*/
	lRet = setActInstanterEnable( CLI_SOFTWARE_UPDATE_IMMEDIATELY);
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING; 
	}	   
    return CMD_SUCCESS;
}

DEFUN (
    olt_software_auto_update_show,
    olt_software_auto_update_show_cmd,
    "show software auto-update time",
	DescStringCommonShow    
	"show software auto-update time\n"
	"show software auto-update time\n"
	"show software auto-update time\n"
    )
{
	LONG lRet = VOS_OK;
	unsigned long valBuf = 0;
	char timeBuf[8] = {0};
	unsigned long timeBufLen = 0;
	short int year = 0;
    /*设置立即更新*/
	lRet = getActInstanterEnable( &valBuf );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING; 
	}
	if ( CLI_SOFTWARE_UPDATE_IMMEDIATELY == valBuf )
	{
		vty_out( vty, "  Software update immediately.\r\n");
		return CMD_SUCCESS;
	}
	else if ( CLI_SOFTWARE_UPDATE_DELAY == valBuf )
	{
		vty_out( vty, "  Software update ");
		lRet = getActivateTime( timeBuf, &timeBufLen );
		if ( lRet != VOS_OK )
		{
			vty_out( vty, "  %% Executing error.\r\n");
			return CMD_WARNING; 
		}
		
		year = *(short int *)timeBuf;
		vty_out( vty, "time : %d-%d-%d %d:%d:%d\r\n",year,timeBuf[2],timeBuf[3],\
				timeBuf[4],timeBuf[5],timeBuf[6],timeBuf[7]);		
	}
	else
	{
		vty_out( vty, "  %% Software update`s status is unknowing.");
		return CMD_WARNING; 
	}	
	
	#if 0
	lRet = getActivateTime( timeBuf, &timeBufLen );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING; 
	}
	
	year = *(short int *)timeBuf;
	vty_out( vty, "Time : %d-%d-%d %d:%d:%d\r\n",year,timeBuf[2],timeBuf[3],\
			timeBuf[4],timeBuf[5],timeBuf[6],timeBuf[7]);
	#endif
    return CMD_SUCCESS;
}
#endif

/*added by wutw at 27 september*/
/*added by wutw at 23 september
show onu device information by mac*/
DEFUN (
    olt_show_device_mac_information,
    olt_show_device_mac_information_cmd,
    "show onu information mac <H.H.H>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by mac\n"
    "Please input Mac address\n"
    )
{
   /* LONG lRet = VOS_OK;*/
    LONG onuId = 0; 	
    UINT16 userOnuId = 0;
    INT16 phyPonId = 0;	
    CHAR MacAddr[6] = {0,0,0,0,0,0};
 
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	    {
	        vty_out( vty, "  %% Invalid MAC address.\r\n" );
	        return CMD_WARNING;
	    }  

	onuId = GetOnuEntryByMac( MacAddr);
	if ((-1) == onuId)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: onuId %d.\r\n",onuId);
	#endif   
        vty_out( vty, "  %% The mac is not exist.\r\n" );
        return CMD_WARNING;
    }
	
	phyPonId = onuId/MAXONUPERPON;
	/*此处onu 的值是0-63*/
	userOnuId = onuId % MAXONUPERPON;	
	#ifdef CLI_OLT_DEBUG
	vty_out(vty, "  %% phyPonId = %d userOnuId = %d\r\n",phyPonId, userOnuId);
	#endif	
	/*ShowOnuDeviceInfo( phyPonId, userOnuId );	*/	
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}


/*show onu device information onuIdx*/
DEFUN (
    olt_show_device_onuidx_information,
    olt_show_device_onuidx_information_cmd,
    "show onu information <slot/port/onuIdx>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information by index\n"
    "Please input the slot/port/onuIdx\n"
    )
{
    /*LONG lRet = VOS_OK;*/
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;

	/*if(0 == memcmp("/?",argv[0],2))
	{
		vty_out(vty, "  %% Please input the slot/port/onuIdx\r\n");
		return CMD_WARNING;
	}*/
	
	sscanf( argv[0], "%d/%d/%d", &slotId, &port, &onuId );
	
	/*增加槽位号，端口号，onuid的范围判断*/
	if(PonCardSlotPortCheckWhenRunningByVty(slotId, port, vty) != ROK )
		return(CMD_WARNING);
	
    #ifdef CLI_EPON_DEBUG
	vty_out(vty, "  %% DEBUG: slotId/port %d/%d.\r\n",slotId, port);
    #endif  

	
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	/*问题单11135*/
	if ((onuId<(CLI_EPON_ONUMIN+1)) || (onuId > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% Parameter is error.onuId is 1-%d\r\n", (CLI_EPON_ONUMAX+1));
		 return CMD_WARNING;
	}
	userOnuId = onuId - 1;
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}


DEFUN (
    onu_show_device_name_information,
    onu_show_device_name_information_cmd,
    "show onu information name <devicename>",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu device information\n"
    "Show the onu device information\n"
    "Show the onu device information by name\n"
    "Please input the device name\n"
    )
{
    /*LONG lRet = VOS_OK;
    ULONG slotId = 0;*/
    LONG onuId = 0; 
    /*ULONG port = 0;*/

    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
    /*CHAR MacAddr[6] = {0,0,0,0,0,0};*/
    int  len = VOS_StrLen(argv[0]);
	if (len > MAXDEVICENAMELEN )
	{
		vty_out( vty, "  %% Parameter is error. Character string is too long\r\n" );	
		return CMD_WARNING;
	}
	
	onuId = GetOnuIdxByName(argv[0] );
	if ((RERROR) == onuId)
    {
        vty_out( vty, "  %% The name is not exist.\r\n" );
        return CMD_WARNING;
    }
	phyPonId = (short int)(onuId/MAXONUPERPON);
	userOnuId = (short int)(onuId % MAXONUPERPON);
	
	ShowOnuDeviceInfoByVty(phyPonId, userOnuId, vty );
    return CMD_SUCCESS;
}

#ifdef IGMP_AUTH_FUNC
#endif

DEFUN (
    igmpsnooping_auth_gw_Item_add,
    igmpsnooping_auth_gw_Item_add_cmd,
    "igmp-auth-gw <slotId/port/onuId> <1-4094> <A.B.C.D> <H.H.H> <1-4094>",
    "Config igmp authorization item based user mac\n"
    "Please input slotId/port/onuId\n"
    "Please input vlanId(1-4094)\n"
    "Please input group-ip(224.0.1.0-239.255.255.255)\n"
    "Please input user mac\n"
    "Please input multicast vlan id\n"
    )
{
    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	unsigned int  mulIp = 0;
	unsigned int  mcastVid = 0;	
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};


	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe00000ff ) || (mulIp > 0xefffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
	
	/*argv[3]*/
	if ( GetMacAddr( ( CHAR* ) argv[ 3 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	
	/*argv[4]*/
	/*	
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[4][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    userIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}	
	*/
	mcastVid = ( ULONG ) VOS_AtoL( argv[ 4 ] );
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	
	lRet = addIgmpSnoopAuthItem( dexIdx, vlanId, mulIp, MacAddr, mcastVid);
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;
}



DEFUN (
    igmpsnooping_auth_gw_Item_del,
    igmpsnooping_auth_gw_Item_del_cmd,
    "undo igmp-auth-gw <slotId/port/onuId> <1-4094> <A.B.C.D> <H.H.H>",
    NO_STR
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input vlanId(1-4094)\n"
	MIP_Descrition    
	"Please input user mac\n"
    )
{
    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	ULONG mulIp = 0;
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};

	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
	
	/*argv[3]*/
	if ( GetMacAddr( ( CHAR* ) argv[ 3 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	/*lRet = delIgmpSnoopAuthItem( dexIdx, vlanId, mulIp, MacAddr );*/
	lRet = setGwIgmpSnoopAuthStatus( dexIdx, vlanId, mulIp, MacAddr, 3 );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;
}

DEFUN (
    igmpsnooping_auth_gw_status_set,
    igmpsnooping_auth_gw_status_set_cmd,
    "igmp-auth-gw status <slotId/port/onuId> <1-4094> <A.B.C.D> <H.H.H> <authstatus>",
    "Config igmp auth item\n"
    "Config igmp auth enable\n"
    "Please input slotId/port/onuId\n"
    "Please input vlanId(1-4094)\n"
	MIP_Descrition    
    "Please input user mac\n"
    "Please input the status will be changed to: 1:active 2:deactive 3:delete"
    )
{
    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	unsigned int  mulIp = 0;
	/*unsigned int  userIp = 0;	*/
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
	ULONG status = 0;


	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
	
	
	/*argv[3]*/
	if ( GetMacAddr( ( CHAR* ) argv[ 3 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}

	VOS_Sscanf( argv[4], "%d", &status );
	if( status <1 || status > 3 )
	{
		vty_out( vty, "invalid status value!\r\n" );
		return CMD_WARNING;
	}
	
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	/*modified the auth statu to enable ( 1 )*/
	lRet = checkGwIgmpSnoopAuthEntry( dexIdx, vlanId, mulIp, MacAddr );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% The item is not exist!\r\n");
		return CMD_WARNING;
	}
	lRet = setGwIgmpSnoopAuthStatus( dexIdx, vlanId, mulIp, MacAddr, status );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% Executing error.\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;
}

DEFUN(
	igmpsnooping_auth_gw_Item_del_all,
	igmpsnooping_auth_gw_Item_del_all_cmd,
	"igmp-auth-gw delete all",
	"Config igmp authorization item based user mac\n"
	"delete operation\n"
	"all objects in the authentication table\n"
	)
{
    /*LONG lRet = VOS_OK;*/
	ULONG devIdx = 0;
    /*ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;*/
	ULONG vlanId = 0;
	ULONG gda =0 ;
	ULONG counter = 0;
    	CHAR MacAddr[6] = {0,0,0,0,0,0};



	while (VOS_OK == getFirstGwIgmpSnoopAuthEntry( &devIdx, &vlanId, &gda, MacAddr ) )
	{
		counter++;
		setGwIgmpSnoopAuthStatus( devIdx, vlanId, gda, MacAddr, 3 );
	}
	if ( !counter )
	{
		vty_out( vty, "  %% NULL\r\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out( vty, "deleted %d items\r\n", counter );
		vty_out( vty, "  \r\n");
    		return CMD_SUCCESS;			
	}
}

DEFUN (
    igmpsnooping_auth_gw_Item_show,
    igmpsnooping_auth_gw_Item_show_cmd,
    "show igmp-auth-gw",
    DescStringCommonShow
    "Show igmp auth item base mac\n"
    )
{
    LONG lRet = VOS_OK;
	ULONG devIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	ULONG vlanId = 0;
	ULONG gda =0 ;
	ULONG nextDevIdx = 0;	
	ULONG nextVlanId = 0;	
	/*ULONG nextslot = 0;
	ULONG nextport = 0;
	ULONG netxonu = 0;*/
	ULONG nextgda = 0;
	
	ULONG authStatus = 0;
	/*unsigned long  mulIp = 0;*/
	unsigned long  mvid = 0;	
	/*unsigned long  nextMulIp = 0;*/
	/*unsigned int  nextUserIp = 0;	*/
	unsigned int authItemCounter = 0;
	int macLen = 6;
    	CHAR MacAddr[6] = {0,0,0,0,0,0};
	CHAR nextMacAddr[6] = {0,0,0,0,0,0};

	ULONG oltm = 0, olst = 0;

	char authbuf[128]="";


	vty_out( vty, "\r\n  No.  vlan  mvlan  s/p/o    group           user-mac        status    online    online-time\r\n\r\n");
	/*vty_out( vty, "  ------------------------------------------------------------------------------------------\r\n");*/


	lRet = getFirstGwIgmpSnoopAuthEntry( &devIdx, &vlanId, &gda, MacAddr );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% NULL\r\n");
		return CMD_SUCCESS;
	}
	
	lRet = getGwIgmpSnoopAuthMulticastVid( devIdx, vlanId, gda,  MacAddr, &mvid );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% get user IP error\r\n");
		return CMD_WARNING;
	}
	
	lRet = getGwIgmpSnoopAuthStatus( devIdx, vlanId, gda,  MacAddr, &authStatus );
	if ( lRet != VOS_OK )
	{
		vty_out( vty, "  %% get auth status error\r\n");
		return CMD_WARNING;
	}

	if( VOS_ERROR == getGwIgmpSnoopAuthOnLineState( devIdx, vlanId, gda, MacAddr, &olst ) )
	{
		vty_out( vty, " %%get igmp auth online status error\n\n" );
		return CMD_WARNING;
	}

	if( VOS_ERROR == getGwIgmpSnoopAuthOnLineTime( devIdx, vlanId, gda, MacAddr, &oltm ) )
	{
		vty_out( vty, " %%get igmp auth online time error\n\n" );
		return CMD_WARNING;
	}
	
	authItemCounter++;

	ulslotId = GET_PONSLOT( devIdx );
	ulport = GET_PONPORT( devIdx );
	ulonuId = GET_ONUID( devIdx );

	/*显示认证内容*/
	vty_out( vty, "  %-4d %-4d  %-5d  %d/%d/%-3d",authItemCounter,vlanId, mvid, ulslotId,ulport,ulonuId);
	sprintf( authbuf, "%d.%d.%d.%d", (gda>>24)&0xff, (gda>>16)&0xff, 
			(gda>>8)&0xff,gda&0xff );
	vty_out( vty, "  %-15s", authbuf );
	
	VOS_MemZero( authbuf, 128 );
	sprintf( authbuf, "%02x%02x.%02x%02x.%02x%02x", MacAddr[0], MacAddr[1], MacAddr[2],
			MacAddr[3], MacAddr[4], MacAddr[5] );
	vty_out( vty, " %-14s", authbuf );

	if( authStatus == 1 )
		vty_out( vty, "  %-8s", "active" );
	else if( authStatus == 2 )
		vty_out( vty, "  %-8s", "deactive" );
	else
		vty_out( vty, "  %-8s", "unkown" );

	if( olst == 1)
		vty_out( vty, "  %-8s", "online" );
	else
		vty_out( vty, "  %-8s", "offline" );

	vty_out( vty, "  %-11d\r\n", oltm );
	
#if 0
	/*显示auth item   serno.  vid  group  usermac  userip  authstatus*/
	if(authStatus == 1)
		vty_out( vty,"                                                                   active\r");  
	else if (authStatus == 2)
		vty_out( vty, "                                                                  deactive\r");  
	else
		vty_out( vty, "                                                                  unknow\r");  

	vty_out( vty, "%54d.%d.%d.%d\r",(userIp>>24)&0xff, (userIp>>16)&0xff, 
			(userIp>>8)&0xff,userIp&0xff);

	vty_out( vty, "%37.2x%02x.%02x%02x.%02x%02x\r",MacAddr[0], MacAddr[1], MacAddr[2],
			MacAddr[3], MacAddr[4], MacAddr[5]);
	vty_out( vty, "%22d.%d.%d.%d\r",(gda>>24)&0xff, (gda>>16)&0xff, 
			(gda>>8)&0xff,gda&0xff);
	vty_out( vty, "  %11d/%d/%d\r",ulslotId,ulport,ulonuId);
	vty_out( vty, "  %-4d %-4d\r\n",authItemCounter,vlanId);	
	/*end of show auth item*/
#endif


	
	while( VOS_OK == getNextGwIgmpSnoopAuthEntry( devIdx, vlanId, gda, MacAddr, &nextDevIdx, &nextVlanId, &nextgda, 
												(unsigned char*)&nextMacAddr ) )
	{
		authItemCounter++;
		lRet = getGwIgmpSnoopAuthMulticastVid( nextDevIdx, nextVlanId, nextgda, nextMacAddr, &mvid );
		if ( lRet != VOS_OK )
		{
			vty_out( vty, "  %% get user IP error\r\n");
			return CMD_WARNING;
		}
		lRet = getGwIgmpSnoopAuthStatus( nextDevIdx, nextVlanId, nextgda, nextMacAddr, &authStatus );
		if ( lRet != VOS_OK )
		{
			vty_out( vty, "  %% get auth status error\r\n");
			return CMD_WARNING;
		}

		if( VOS_ERROR == getGwIgmpSnoopAuthOnLineState( nextDevIdx, nextVlanId, nextgda, nextMacAddr, &olst ) )
		{
			vty_out( vty, " %%get igmp auth online status error\n\n" );
			return CMD_WARNING;
		}

		if( VOS_ERROR == getGwIgmpSnoopAuthOnLineTime( nextDevIdx, nextVlanId, nextgda, nextMacAddr, &oltm ) )
		{
			vty_out( vty, " %%get igmp auth online time error\n\n" );
			return CMD_WARNING;
		}		

		ulslotId = GET_PONSLOT( nextDevIdx );
		ulport = GET_PONPORT( nextDevIdx );
		ulonuId = GET_ONUID( nextDevIdx );

#if 0
		/*显示auth item   serno.  vid  group  usermac  userip  authstatus*/
		if(authStatus == 1)
			vty_out( vty, "                                                                   active\r");
		else if (authStatus == 2)
			vty_out( vty, "                                                                   deactive\r");
		else
			vty_out( vty, "                                                                   unknow\r");

		vty_out( vty, "%54d.%d.%d.%d\r",(userIp>>24)&0xff, (userIp>>16)&0xff, 
				(userIp>>8)&0xff,userIp&0xff);
		
		vty_out( vty, "%37.2x%02x.%02x%02x.%02x%02x\r",nextMacAddr[0], nextMacAddr[1], nextMacAddr[2],
				nextMacAddr[3], nextMacAddr[4], nextMacAddr[5]);
		
		vty_out( vty, "%22d.%d.%d.%d\r",(nextgda>>24)&0xff, (nextgda>>16)&0xff, 
				(nextgda>>8)&0xff,nextgda&0xff);
		
		vty_out( vty, "  %11d/%d/%d\r",ulslotId,ulport,ulonuId);	

		vty_out( vty, "  %-4d %-4d\r\n",authItemCounter,vlanId);			
		/*end of show auth item*/
#endif

		/*显示认证内容*/
		vty_out( vty, "  %-4d %-4d  %-5d  %d/%d/%-3d",authItemCounter,nextVlanId, mvid, ulslotId,ulport,ulonuId);
		sprintf( authbuf, "%d.%d.%d.%d", (nextgda>>24)&0xff, (nextgda>>16)&0xff, 
				(nextgda>>8)&0xff,nextgda&0xff );
		vty_out( vty, "  %-15s", authbuf );
		
		VOS_MemZero( authbuf, 128 );
		sprintf( authbuf, "%02x%02x.%02x%02x.%02x%02x", nextMacAddr[0], nextMacAddr[1], nextMacAddr[2],
				nextMacAddr[3], nextMacAddr[4], nextMacAddr[5] );
		vty_out( vty, " %-14s", authbuf );

		if( authStatus == 1 )
			vty_out( vty, "  %-8s", "active" );
		else if( authStatus == 2 )
			vty_out( vty, "  %-8s", "deactive" );
		else
			vty_out( vty, "  %-8s", "unkown" );

		if( olst == 1)
			vty_out( vty, "  %-8s", "online" );
		else
			vty_out( vty, "  %-8s", "offline" );

		vty_out( vty, "  %-11d\r\n", oltm );
		
		devIdx = nextDevIdx;
		vlanId = nextVlanId;
		gda = nextgda;
		VOS_MemCpy( MacAddr, nextMacAddr, macLen );

	}
	vty_out( vty, "  \r\n");
    return CMD_SUCCESS;
}


DEFUN (
    igmpsnooping_auth_Item_add,
    igmpsnooping_auth_Item_add_cmd,
    "igmp-auth <slotId/port/onuId> <1-24> <A.B.C.D> <1-3> <1-4094>",
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input the port id which port with it has connected to user's device\n"
	MIP_Descrition    
    "Please input user security 1:permit, 2:preview, 3 forbid\n"
    "Please input multicast vlan\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	/*unsigned int  mulIp = 0;*/
	unsigned int  userIp = 0;	
	unsigned short vlanId = 0;
	ULONG security = 0;
	ULONG mvid = 0;
    INT16 phyPonId = 0;
	igmp_auth_list_t *pAuthInfo = NULL;
    /*CHAR MacAddr[6] = {0,0,0,0,0,0};*/

	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
		int cli_ip[4] = { 0, 0, 0, 0 };
		sscanf( argv[2], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
	    	userIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
		if( userIp < 0xe0000100 || userIp > 0xeeffffff )
		{
			vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n" );
			return CMD_WARNING;
		}
	}	
#else
    userIp = get_long_from_ipdotstring( argv[ 2 ] );
	userIp = VOS_NTOHL(userIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif

	security = VOS_AtoL( argv[3] );
	
	lRet = addIgmpAuthEntry( ulslotId, ulport, ulonuId, vlanId, userIp, &pAuthInfo );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% the entry had been added or parameters are wrong(%d).\r\n", lRet);
		return CMD_WARNING;
	}

	mvid = VOS_AtoL( argv[4] );


	setCtcIgmpSnoopAuthStatus( ulslotId, ulport, ulonuId, vlanId, userIp, security );
	setCtcIgmpSnoopAuthMvlanId( ulslotId, ulport, ulonuId, vlanId, userIp, mvid );

    return CMD_SUCCESS;
}

DEFUN (
    igmpsnooping_auth_Item_add_groups,
    igmpsnooping_auth_Item_add_groups_cmd,
    "igmp-auth <slotId/port/onuId> <1-24> <A.B.C.D> <A.B.C.D> <1-3> <1-4094>",
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input the port id which port with it has connected to user's device\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n"
    "Please input user security 1:permit, 2:preview, 3 forbid\n"
    "Please input multicast vlan\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	/*unsigned int  mulIp = 0;*/
	unsigned int  userIp = 0, userIp_St=0, userIp_End=0;	
	unsigned short vlanId = 0;
	ULONG security = 0;
	ULONG mvid = 0;
    INT16 phyPonId = 0;
	igmp_auth_list_t *pAuthInfo = NULL;
    /*CHAR MacAddr[6] = {0,0,0,0,0,0};*/
	ULONG flagt=0; /*这里i 须为4 字节大小*/
	
	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
#if 0
	{
		int cli_ip[4] = { 0, 0, 0, 0 };
		sscanf( argv[2], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
	    	userIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
		if( userIp < 0xe0000100 || userIp > 0xeeffffff )
		{
			vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n" );
			return CMD_WARNING;
		}
	}	
#else
    userIp = get_long_from_ipdotstring( argv[ 2 ] );
	userIp = VOS_NTOHL(userIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
#else
	userIp_St= get_long_from_ipdotstring( argv[ 2 ] );
	userIp_St = VOS_NTOHL(userIp_St);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_St ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 2] );
		return CMD_WARNING;
    	}

	userIp_End = get_long_from_ipdotstring( argv[ 3 ] );
	userIp_End = VOS_NTOHL(userIp_End);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_End ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[3] );
		return CMD_WARNING;
    	}
	if(userIp_St > userIp_End)
	{
		vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}	
#endif

	security = VOS_AtoL( argv[4] );
	mvid = VOS_AtoL( argv[5] );

	/* modified by xieshl 20170427, 太啰嗦 */
	/*flagt = 0;
	for( i = userIp_St ; i <= userIp_End; i++)
		flagt++;
	if(flagt > 1000 )*/
	if( (userIp_End - userIp_St) > 1000 )
	{
		vty_out(vty," You cann't add more  one thousand groups one time!\r\n");
		return CMD_WARNING;
	}

	flagt = 0;
	
	for( userIp = userIp_St ; userIp <= userIp_End; userIp++)
	{
		pAuthInfo = NULL;
		
		lRet = addIgmpAuthEntry( ulslotId, ulport, ulonuId, vlanId, userIp, &pAuthInfo );
#if 0
		if (lRet != VOS_OK)
		{
			vty_out( vty, "  %% the entry had been added or parameters are wrong(%d).\r\n", lRet);
			return CMD_WARNING;
		}
#else
		if (lRet != VOS_OK)
		{
			/*if( flagt%10 == 0 )
			{
				if( flagt == 0)
					vty_out( vty, "  The groups had been added or parameters are wrong.");
				
				vty_out(vty,"\r\n");
				
				vty_out(vty, "  0x%x", userIp );
			}
			else
				vty_out(vty, ", 0x%x", userIp );*/

			flagt++;

		}
#endif

		setCtcIgmpSnoopAuthStatus( ulslotId, ulport, ulonuId, vlanId, userIp, security );
		setCtcIgmpSnoopAuthMvlanId( ulslotId, ulport, ulonuId, vlanId, userIp, mvid );
	}

	if(flagt > 0)
		vty_out( vty, " Some groups had been added or parameters are wrong.\r\n");
		/*vty_out(vty,"\r\n");*/
	
    	return CMD_SUCCESS;

}

DEFUN (
    igmpsnooping_auth_gw_Item_add_groups,
    igmpsnooping_auth_gw_Item_add_groups_cmd,
    "igmp-auth-gw <slotId/port/onuId> <1-4094> <A.B.C.D> <A.B.C.D> <H.H.H> <1-4094>",
    "Config igmp authorization item based user mac\n"
    "Please input slotId/port/onuId\n"
    "Please input vlanId(1-4094)\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n"
    "Please input user mac\n"
    "Please input multicast vlan id\n"
    )
{
    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	unsigned int  mulIp = 0;
	unsigned int  mcastVid = 0;	
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
    ULONG userIp_St=0, userIp_End=0;
    ULONG flagt=0;
	
	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);
	
    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe00000ff ) || (mulIp > 0xefffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
#else
	userIp_St= get_long_from_ipdotstring( argv[ 2 ] );
	userIp_St = VOS_NTOHL(userIp_St);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_St ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 2] );
		return CMD_WARNING;
    	}

	userIp_End = get_long_from_ipdotstring( argv[ 3 ] );
	userIp_End = VOS_NTOHL(userIp_End);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_End ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[3] );
		return CMD_WARNING;
    	}
	if(userIp_St > userIp_End)
	{
		vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}	
#endif

	if( (userIp_End - userIp_St) > 1000 )
	{
		vty_out(vty," You cann't add more  one thousand groups one time!\r\n");
		return CMD_WARNING;
	}
	
	/*argv[3]*/
	if ( GetMacAddr( ( CHAR* ) argv[ 4 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	
	/*argv[4]*/
	/*	
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[4][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    userIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}	
	*/
	mcastVid = ( ULONG ) VOS_AtoL( argv[ 5 ] );
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);

		flagt = 0;
	
	for( mulIp = userIp_St ; mulIp <= userIp_End; mulIp++)
	{
	
		lRet = addIgmpSnoopAuthItem( dexIdx, vlanId, mulIp, MacAddr, mcastVid);
		if (lRet != VOS_OK)
		{
			/*	vty_out( vty, "  %% Executing error.\r\n");
			return CMD_WARNING;*/
			flagt++;
		}
	}

	if(flagt > 0)
		vty_out( vty, " Some groups had been added or parameters are wrong.\r\n");

	return CMD_SUCCESS;
	
}


DEFUN (
    igmpsnooping_auth_Item_del,
    igmpsnooping_auth_Item_del_cmd,
    "undo igmp-auth <slotId/port/onuId> <1-24> <A.B.C.D> ",
    NO_STR
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input the port id which port with it has connected to user's device\n"
	MIP_Descrition    
    "Please input user mac\n"
    )
{
    LONG lRet = VOS_OK;
	/*ULONG dexIdx = 0;*/
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	ULONG mulIp = 0;
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    /*CHAR MacAddr[6] = {0,0,0,0,0,0};*/


	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( argv[2], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    	mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
	
	lRet = delIgmpAuthEntry( ulslotId, ulport, ulonuId, vlanId, mulIp );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% the entry does't exist or had been deleted.\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;
}

DEFUN (
    igmpsnooping_auth_gw_Item_groups_del,
    igmpsnooping_auth_gw_Item_del_groups_cmd,
    "undo igmp-auth-gw <slotId/port/onuId> <1-4094> <A.B.C.D> <A.B.C.D> <H.H.H>",
    NO_STR
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input vlanId(1-4094)\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n"
    "Please input user mac\n"
    )
{
    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
    ULONG mulIp = 0, flagt=0;
    unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    CHAR MacAddr[6] = {0,0,0,0,0,0};
    ULONG userIp_St=0, userIp_End=0;
	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

#if 0
	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
#else
	userIp_St= get_long_from_ipdotstring( argv[ 2 ] );
	userIp_St = VOS_NTOHL(userIp_St);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_St ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 2] );
		return CMD_WARNING;
    	}

	userIp_End = get_long_from_ipdotstring( argv[ 3 ] );
	userIp_End = VOS_NTOHL(userIp_End);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_End ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[3] );
		return CMD_WARNING;
    	}
	if(userIp_St > userIp_End)
	{
		vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}	
#endif

	if( (userIp_End - userIp_St) > 1000 )
	{
		vty_out(vty," You cann't delete more one thousand groups one time!\r\n");
		return CMD_WARNING;
	}
	
	/*argv[3]*/
	if ( GetMacAddr( ( CHAR* ) argv[ 4 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	/*lRet = delIgmpSnoopAuthItem( dexIdx, vlanId, mulIp, MacAddr );*/

	flagt = 0;
	
	for( mulIp = userIp_St ; mulIp <= userIp_End; mulIp++)
	{
		lRet = setGwIgmpSnoopAuthStatus( dexIdx, vlanId, mulIp, MacAddr, 3 );
		if (lRet != VOS_OK)
		{
			/*vty_out( vty, "  %% Executing error.\r\n");
			return CMD_WARNING;*/
			flagt++;
		}
	}

	if(flagt > 0)
		vty_out( vty, " Some groups does't exist. Please check it!\r\n");
	
    	return CMD_SUCCESS;

}

DEFUN (
    igmpsnooping_auth_Item_del_groups,
    igmpsnooping_auth_Item_del_groups_cmd,
    "undo igmp-auth <slotId/port/onuId> <1-24> <A.B.C.D> <A.B.C.D>",
    NO_STR
    "Config igmp auth item\n"
    "Please input slotId/port/onuId\n"
    "Please input the port id which port with it has connected to user's device\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n"
    )
{
    LONG lRet = VOS_OK;
	/*ULONG dexIdx = 0;*/
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0, flagt=0;
	ULONG mulIp = 0, userIp_St=0, userIp_End=0;
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
    /*CHAR MacAddr[6] = {0,0,0,0,0,0};*/

	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

#if 0
	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( argv[2], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    	mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
#else
	userIp_St= get_long_from_ipdotstring( argv[ 2 ] );
	userIp_St = VOS_NTOHL(userIp_St);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_St ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 2] );
		return CMD_WARNING;
    	}

	userIp_End = get_long_from_ipdotstring( argv[ 3 ] );
	userIp_End = VOS_NTOHL(userIp_End);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( userIp_End ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[3] );
		return CMD_WARNING;
    	}
	if(userIp_St > userIp_End)
	{
		vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}	
#endif

	if( (userIp_End - userIp_St) > 1000 )
	{
		vty_out(vty," You cann't delete more one thousand groups one time!\r\n");
		return CMD_WARNING;
	}

	flagt = 0;
	
	for( mulIp = userIp_St ; mulIp <= userIp_End; mulIp++)
	{
		lRet = delIgmpAuthEntry( ulslotId, ulport, ulonuId, vlanId, mulIp );
		if (lRet != VOS_OK)
		{
			/*if( flagt%10 == 0 )
			{
				if( flagt == 0)
					vty_out( vty, " The groups does't exist. ");
				
				vty_out(vty,"\r\n");
				
				vty_out(vty, "  0x%x", mulIp );
			}
			else
				vty_out(vty, ", 0x%x", mulIp );*/

			flagt++;
	
			/*return CMD_WARNING;*/
		}
	}

	if(flagt > 0)
		vty_out( vty, " Some groups does't exist. Please check it!\r\n");
		/*vty_out(vty,"\r\n");*/

	return CMD_SUCCESS;
}

DEFUN (
    igmpsnooping_auth_status_set,
    igmpsnooping_auth_status_set_cmd,
    "igmp-auth status <slotId/port/onuId> <1-24> <A.B.C.D> <level> {[preview-time|preview-interval|preview-counter|used-counter] <set_value>}*1",
    "Config igmp auth item\n"
    "Config igmp auth enable\n"
    "Please input onu's location information slotId/port/onuId\n"
    "Please input the port id which port with it has connected to user's device\n"
	MIP_Descrition    
    "Please input new security level(1:permit;2:preview;3:forbid\n"
    "Set preview time length, 1-3600 (second)\n" 
    "Set interval time before retring join, 1-600 (second)\n"
    "Set the max try of joining by preview-user, 1-10 \n"
    "Set the used try number by preview-user, 1-10\n"
    "Please input the value needed\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	unsigned int  mulIp = 0;
	/*unsigned int  userIp = 0;	*/
	unsigned short vlanId = 0;
    INT16 phyPonId = 0;
	int security = 0;

	/*argv[0]*/
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	SWAP_TO_ACTIVE_PON_PORT(ulslotId, ulport)

	if(PonCardSlotPortCheckWhenRunningByVty(ulslotId, ulport,vty) != ROK)
		return(CMD_WARNING);

    phyPonId = GetPonPortIdxBySlot( (short int)ulslotId, (short  int)ulport );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}	


	/*argv[1]*/
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	/*argv[2]*/
#if 0
	{
	int cli_ip[4] = { 0, 0, 0, 0 };
	sscanf( &argv[2][0], "%d.%d.%d.%d", &cli_ip[0], &cli_ip[1], &cli_ip[2], &cli_ip[3] );
    mulIp = ((cli_ip[0] & 0xff) << 24) | ((cli_ip[1] & 0xff) << 16) | ((cli_ip[2] & 0xff) << 8) | (cli_ip[3] & 0xff);
	}
	
	if ( (mulIp < 0xe0000100 ) || (mulIp > 0xeeffffff))
	{
		vty_out( vty, "  %% Group IP error. Input group IP : 224.0.1.0-238.255.255.255\r\n");
		return CMD_WARNING;
	}
#else
    mulIp = get_long_from_ipdotstring( argv[ 2 ] );
	mulIp = VOS_NTOHL(mulIp);    /* add by suxq 2007-06-19 */
   /*group address check ,add by sxj 060627*/
    if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIp ) )
    {
        vty_out( vty, "%% Group address %s invalid . \r\n", argv[ 2] );
#if 0
    	VOS_SysLog( LOG_TYPE_IGMP, LOG_ALERT, "Fail to report IGMP receive: Group address invalid. 0x%-8x ", ulGroup );
        IGMP_SNOOP_REPORT_DEBUG( ( "Igmp_Recv_Report: group address invalid. 0x%-8x \r\n", ulGroup ) );
#endif
    	return VOS_ERROR;
    }
#endif
	
	
	/*argv[3]*/
	VOS_Sscanf( (CHAR*)argv[3], "%d", &security );
	if ( security<1 || security>3 )
	{
	        vty_out( vty, "  %% Invalid security level.\r\n" );
	        return CMD_WARNING;
	}
	
	/*modified the auth status*/

	lRet = checkIgmpSnoopEntry( ulslotId, ulport, ulonuId, vlanId, mulIp );
	if (lRet != VOS_OK)
	{
		vty_out( vty, "  %% The item is not exist!\r\n");
		return CMD_WARNING;
	}

	if( setCtcIgmpSnoopAuthStatus( ulslotId, ulport, ulonuId, vlanId, mulIp, security ) == VOS_ERROR )
	{
		vty_out( vty, "set auth status error!\r\n" );
		return CMD_WARNING;
	}

	if( VOS_StrCmp( (char*)argv[4], "preview-time") == 0 )
	{
		ULONG runvar = 0;
#if 0
		VOS_Sscanf( (char*)argv[5], "%d", &runvar );
#else
		lRet = VOS_AtoL_2(argv[5], &runvar);
		if(lRet != VOS_OK)
		{
			vty_out( vty, " Parameter error, please check it !\n" );
			return CMD_WARNING;
		}
		else
		{
			if( runvar <=0 || runvar > 3600)
			{
				vty_out(vty, " The Parameter for preview-time should be between 1 and 3600 (unit: second).\r\n");
				return CMD_WARNING;
			}
		}
#endif
		if( setCtcIgmpSnoopAuthPreViewTime( ulslotId, ulport, ulonuId, vlanId, mulIp, runvar ) == VOS_ERROR )
		{
			vty_out( vty, "set auth preview time error!\r\n" );
			return CMD_WARNING;
		}
	}

	if( VOS_StrCmp( (char*)argv[4], "preview-interval") == 0 )
	{
		ULONG runvar = 0;
#if 0
		VOS_Sscanf( (char*)argv[5], "%d", &runvar );
#else
		lRet = VOS_AtoL_2(argv[5], &runvar);
		if(lRet != VOS_OK)
		{
			vty_out( vty, " Parameter error, please check it !\n" );
			return CMD_WARNING;
		}
		else
		{
			if( runvar <=0 || runvar > 600)
			{
				vty_out(vty, " The Parameter for preview-interval should be between 1 and 600 (unit: second).\r\n");
				return CMD_WARNING;
			}
		}
#endif
		if( setCtcIgmpSnoopAuthPreViewInter( ulslotId, ulport, ulonuId, vlanId, mulIp, runvar ) == VOS_ERROR )
		{
			vty_out( vty, "set auth preview interval error!\r\n" );
			return CMD_WARNING;
		}
	}	

	if( VOS_StrCmp( (char*)argv[4], "preview-counter") == 0 )
	{
		ULONG runvar = 0;
#if 0
		VOS_Sscanf( (char*)argv[5], "%d", &runvar );
#else
		lRet = VOS_AtoL_2(argv[5], &runvar);
		if(lRet != VOS_OK)
		{
			vty_out( vty, " Parameter error, please check it !\n" );
			return CMD_WARNING;
		}
		else
		{
			if( runvar <=0 || runvar > 10)
			{
				vty_out(vty, " The Parameter for preview-counter should be between 1 and 10.\r\n");
				return CMD_WARNING;
			}
		}
#endif
		if( setCtcIgmpSnoopAuthPreViewCount( ulslotId, ulport, ulonuId, vlanId, mulIp, runvar ) == VOS_ERROR )
		{
			vty_out( vty, "set auth preview-counter error!\r\n" );
			return CMD_WARNING;
		}
	}	

	if( VOS_StrCmp( (char*)argv[4], "used-counter") == 0 )
	{
		ULONG runvar = 0;
#if 0
		VOS_Sscanf( (char*)argv[5], "%d", &runvar );
#else
		lRet = VOS_AtoL_2(argv[5], &runvar);
		if(lRet != VOS_OK)
		{
			vty_out( vty, " Parameter error, please check it !\n" );
			return CMD_WARNING;
		}
		else
		{
			if( runvar <=0 || runvar > 10)
			{
				vty_out(vty, " The Parameter for used-counter should be between 1 and 10.\r\n");
				return CMD_WARNING;
			}
		}
#endif
		if( setCtcIgmpSnoopAuthPreViewUseCou( ulslotId, ulport, ulonuId, vlanId, mulIp, runvar ) == VOS_ERROR )
		{
			vty_out( vty, "set auth used-counter error!\r\n" );
			return CMD_WARNING;
		}
	}	
	
    return CMD_SUCCESS;
}

DEFUN(
	igmpsnooping_auth_Item_del_all,
	igmpsnooping_auth_Item_del_all_cmd,
	"igmp-auth delete all",
	"Config igmp auth item\n"
	"delete operation\n"
	"all objectes in the authentication table\n"
	)
{
/*    LONG lRet = VOS_OK;
	ULONG dexIdx = 0;*/
    ULONG ulslotId = 0;
    ULONG ulonuId = 0; 
    ULONG ulport = 0;
	ULONG vlanId = 0;
	ULONG gda =0 ;
	ULONG counter = 0;
	
	while( getFirstIgmpSnoopAuthEntry( &ulslotId, &ulport, &ulonuId, &vlanId, &gda ) == VOS_OK )
	{
		delIgmpAuthEntry( ulslotId, ulport, ulonuId, vlanId, gda );
		counter++;
	}
	
	if ( counter == 0 )
	{
		vty_out( vty, "  %% NULL\r\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty, "deleted %d items\r\n", counter );
		vty_out( vty, "  \r\n");
	    	return CMD_SUCCESS;
	}
}	

/* added by xieshl 20170425 问题单19778，同一onu端口的认证表项以范围段显示 */
static CHAR *igmp_auth_ip_2_str( ULONG ip )
{
	static CHAR str[24];
	UCHAR *p_ip = (UCHAR *)&ip;
	VOS_MemZero( str, sizeof(str) );
	VOS_Sprintf( str, "%d.%d.%d.%d", p_ip[0], p_ip[1], p_ip[2], p_ip[3] );
	return str;
}
static CHAR *igmp_auth_mac_2_str( UCHAR *p_mac )
{
	static CHAR str[16];
	VOS_MemZero( str, sizeof(str) );
	VOS_Sprintf( str, "%02X%02X.%02X%02X.%02X%02X", p_mac[0], p_mac[1], p_mac[2], p_mac[3], p_mac[4], p_mac[5] );
	return str;
}
static CHAR *igmp_auth_onu_2_str( ULONG slot, ULONG port, ULONG onuid )
{
	static CHAR str[16];
	VOS_MemZero( str, sizeof(str) );
	sprintf( str, "%d/%d/%d", (slot&0x1f), (port&0x1f), (onuid&0xff) );	
	return str;
}
static CHAR *igmp_auth_status_2_str( ULONG status )
{
	CHAR *pStr = "unknown";
	if( status == IGMP_SECURITY_PERMIT )
		pStr = "permit";
	else if( status == IGMP_SECURITY_PREVIEW )
		pStr = "preview";
	else if( status == IGMP_SECURITY_FORBID )
		pStr = "forbid";
	return pStr;
}
static VOID igmp_auth_item_print( struct vty *vty, LONG itemCounter, IGMP_AUTH_INFO *pItem, ULONG mergeGda, LONG mergeCount )
{
	CHAR tmp[36];
	vty_out( vty,"%-4d%-6d%-9s%-5d", itemCounter, pItem->usrmvlan,
		igmp_auth_onu_2_str(pItem->slot,pItem->pon,pItem->onu), pItem->cvlan );

	if( mergeCount == 1 )
	{
		vty_out( vty,"%-32s", igmp_auth_ip_2_str(pItem->gda) );
	}
	else
	{
		VOS_Sprintf( tmp, "%s-", igmp_auth_ip_2_str(mergeGda) );
		VOS_StrCat( tmp, igmp_auth_ip_2_str(pItem->gda) );
		vty_out( vty,"%-32s", tmp );
	}
	vty_out( vty, "%-9s", igmp_auth_status_2_str(pItem->securitylevel) );

	if( pItem->onlinestatus == 0 )
		vty_out( vty, "%-10s", "OFF" );
	else
	{
		vty_out( vty, "%-10d", pItem->onlinetime );
	}

	if( pItem->securitylevel == IGMP_SECURITY_PREVIEW )
	{
		VOS_Sprintf( tmp, "%d/%d", pItem->previewtime, pItem->previewinterval );
		vty_out( vty, "%-13s%d/%d\r\n", tmp, pItem->usedpreviewcount, pItem->previewcount );
	}
	else
	{
		vty_out( vty, "%-13s%s\r\n", "-", "-" );
	}
	return;
}
/* end 20170425 */

DEFUN (
    igmpsnooping_auth_Item_show,
    igmpsnooping_auth_Item_show_cmd,
    "show igmp-auth",
    DescStringCommonShow
    "Show igmp auth item\n"
    )
{
	/* modified by xieshl 20170425 问题单19778，同一onu端口的认证表项以范围段显示，
	    注意: 修改后，预览相关参数显示做简化处理，只以最后一个为准。后续如果预览功能有需求，可以另扩展命令显示 */
    LONG lRet = VOS_OK;
    ULONG ulSlot = 0, ulNextSlot = 0;
    ULONG ulPort = 0, ulNextPort = 0;
    ULONG ulOnuId = 0, ulNextOnuId = 0; 
	ULONG vlanId = 0, nextVlanId = 0;
	ULONG gda =0, nextgda = 0;
	ULONG mergeGda = 0, mergeCount = 0;

	IGMP_AUTH_INFO item, nextItem;
	LONG itemCounter = 0;
	CHAR tmp[36];

	VOS_MemZero( &item, sizeof(item) );

	for( lRet = getFirstIgmpSnoopAuthEntry(&ulNextSlot, &ulNextPort, &ulNextOnuId, &nextVlanId, &nextgda);
		lRet == VOS_OK;
		lRet = getNextIgmpSnoopAuthEntry(ulSlot, ulPort, ulOnuId, vlanId, gda, &ulNextSlot, &ulNextPort, &ulNextOnuId, &nextVlanId, &nextgda) )
	{
		if( getCtcIgmpSnoopAuthItem(ulNextSlot, ulNextPort, ulNextOnuId, nextVlanId, nextgda, &nextItem) == VOS_OK )
		{
			if( itemCounter == 0 )
			{
				vty_out( vty, " preview=preview times/interval, counter=preview used counter/max\r\n\r\n" );
				vty_out( vty, "%-4s%-6s%-9s%-5s%-32s%-9s%-10s%-13s%s\r\n",
					"No", "mvlan", "onuIdx", "port", "group", "status", "online(s)", "preview", "counter" );
				itemCounter++;
			}
			/*
			vty_out( vty,"  %-5d%-6d%-8s%-6d%-33s", itemCounter, nextItem.usrmvlan,
				igmp_auth_onu_2_str(ulNextSlot,ulNextPort,ulNextOnuId), nextVlanId, igmp_auth_ip_2_str(nextgda) );
			
			vty_out( vty, "%-9s", igmp_auth_status_2_str(nextItem.securitylevel) );

			if( nextItem.onlinestatus == 0 )
				vty_out( vty, "%-10s", "-" );
			else
			{
				vty_out( vty, "%-10d", nextItem.onlinetime );
			}
			
			VOS_Sprintf( tmp, "%d/%d", nextItem.previewtime, nextItem.previewinterval );
			vty_out( vty, "%-13s%d/%d\r\n", tmp, nextItem.usedpreviewcount, nextItem.previewcount );
			*/

			if( mergeGda == 0 )
			{
				mergeCount = 1;
				mergeGda = nextItem.gda;
			}
			else
			{
				if( (vlanId == nextVlanId) && (item.usrmvlan == nextItem.usrmvlan) &&
					(ulSlot == ulNextSlot && ulPort == ulNextPort && ulOnuId == ulNextOnuId) &&
					(item.onlinestatus == nextItem.onlinestatus) &&
					(item.gda + 1 == nextItem.gda) )
				{
					mergeCount++;
				}
				else
				{
					igmp_auth_item_print( vty, itemCounter, &item, mergeGda, mergeCount );
					itemCounter++;

					mergeCount = 1;
					mergeGda = nextItem.gda;
				}
			}

			VOS_MemCpy( &item, &nextItem, sizeof(item) );
		}

        ulSlot = ulNextSlot;
		ulPort = ulNextPort;
		ulOnuId = ulNextOnuId;
		vlanId = nextVlanId;
		gda = nextgda;
	}

	if( itemCounter != 0 )
	{
		if( mergeCount != 0 )
			igmp_auth_item_print( vty, itemCounter, &item, mergeGda, mergeCount );

		vty_out( vty, "\r\n");
	}
	else
	{
		vty_out( vty, "\r\n  igmp auth entry is NULL\r\n");
	}

    return CMD_SUCCESS;
}

#if IGMP_AUTH_FUNC_END
#endif

/* B--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */
#if 1
DEFUN( bandwidth_onu_police_param_config,
        bandwidth_onu_police_param_cmd,
        "onu downlink-policer-param burst <0-16777215> preference [enable|disable] {weight <2-256>}*1",
        "Config the onu software\n"
        "Config onu downlink-policer's param\n"
        "Config onu downlink-policer's burst\n"
        "Please input the burst value(unit:byte)\n"
        "Config onu downlink-policer's preference\n"
        "Enable the preference setting\n"
        "Close the preference setting\n"
        "Config onu downlink-policer's weight\n"
        "Please input the weight value(unit:kbit)\n"
        )
{
	int iBurst;
	int iPre;
	int iWeight;

    iBurst = VOS_AtoI(argv[0]);
    iPre   = ('e' == argv[1][0]) ? ENABLE : DISABLE;
    if ( argc > 2 )
    {
        iWeight = VOS_AtoI(argv[2]);
    }
    else
    {
        iWeight = -1;
    }
    
	if (0 != SetOnuPolicerParam( iBurst, iPre, iWeight ))
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		
    
	return CMD_SUCCESS;	
}


DEFUN( bandwidth_onu_police_param_undo,
        bandwidth_onu_police_param_undo_cmd,
        "undo onu downlink-policer-param",
        NO_STR
        "Config the onu software\n"
        "Config onu downlink-policer's default param\n"
        )
{
	if (0 != SetOnuPolicerParam( -1, -1, -1 ))
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		
	return CMD_SUCCESS;	
}

DEFUN( bandwidth_onu_police_param_show,
        bandwidth_onu_police_param_show_cmd,
        "show onu downlink-policer-param",
        DescStringCommonShow
        "Show the onu software\n"
        "Show onu downlink-policer's params\n"
        )
{
	int debugFlag;
	int iBurst;
	int iPre;
	int iWeight;

	debugFlag = GetOnuPolicerParam( &iBurst, &iPre, &iWeight );
	if ((-1) == debugFlag)
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }	
    
	vty_out( vty ,"  Onu downlink-policer-param: burst=%d Bytes\r\n", iBurst);
	vty_out( vty ,"  Onu downlink-policer-param: preference is %s\r\n", (iPre == ENABLE) ? "enable" : "disable");
	vty_out( vty ,"  Onu downlink-policer-param: weight=%d kb\r\n", iWeight);
	
	return CMD_SUCCESS;	
}
#endif
/* E--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */

/* B--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
#if 1
DEFUN( bandwidth_onu_dba_param_config,
        bandwidth_onu_dba_param_cmd,
        "onu uplink-dba-param pktsize <84-1522> burst <1-256> {weight <2-32>}*1",
        "Config the onu software\n"
        "Config onu uplink-dba's param\n"
        "Config onu uplink-dba's fixed packet size\n"
        "Please input the packet size(unit:byte)\n"
        "Config onu uplink-dba's burst\n"
        "Please input the burst value(unit:byte)\n"
        "Config onu uplink-dba's weight\n"
        "Please input the weight value(unit:kbit)\n"
        )
{
	int iPktSize;
	int iBurst;
	int iWeight;

    iPktSize = VOS_AtoI(argv[0]);
    iBurst = VOS_AtoI(argv[1]);
    if ( argc > 2 )
    {
        iWeight = VOS_AtoI(argv[2]);
    }
    else
    {
        iWeight = -1;
    }
    
	if (0 != SetOnuDbaParam( iPktSize, iBurst, iWeight ))
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		
    
	return CMD_SUCCESS;	
}

DEFUN( bandwidth_onu_dba_param_undo,
        bandwidth_onu_dba_param_undo_cmd,
        "undo onu uplink-dba-param",
        NO_STR
        "Config the onu software\n"
        "Config onu uplink-dba's default param\n"
        )
{
	if (0 != SetOnuDbaParam(-1, -1, -1))
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }		
	return CMD_SUCCESS;	
}

DEFUN( bandwidth_onu_dba_param_show,
        bandwidth_onu_dba_param_show_cmd,
        "show onu uplink-dba-param",
        DescStringCommonShow
        "Show the onu software\n"
        "Show onu uplink-dba's params\n"
        )
{
	int debugFlag;
	int iPktSize;
	int iBurst;
	int iWeight;

	debugFlag = GetOnuDbaParam( &iPktSize, &iBurst, &iWeight );
	if ((-1) == debugFlag)
    {
	    vty_out( vty, "  %% Executing failed.\r\n" );
	    return CMD_WARNING;    	
    }	
    
	vty_out( vty ,"  Onu uplink-dba-param: fixedPktSize=%d Bytes, burst=%d Bytes, weight=%d kb\r\n", iPktSize, iBurst, iWeight);
	
	return CMD_SUCCESS;	
}
#endif
/* E--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */



/*限制onu*/
/* deleted by chenfj 2008-3-31 */
#ifdef  V2R1_ONU_REGISTER_LIMIT_
DEFUN  (
    pon_onu_regiseter_limit_all_config,
    pon_onu_regiseter_limit_all_config_cmd,
    "register limit-onu ",
    "onu register config\n"
    "onu limit register config\n"
    )
{
    LONG lRet = VOS_OK;
    int  cliflag	 = 0;

	cliflag = V2R1_ENABLE;
    lRet = SetOnuRegisterLimitFlagAll( cliflag );	
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
    pon_no_onu_regiseter_limit_all_config,
    pon_no_onu_regiseter_limitconfig_all_cmd,
    "undo register limit-onu ",
    NO_STR
    "onu register config\n"
    "Set onu auto register config\n"
    )
{
    LONG lRet = VOS_OK;
    int  cliflag	 = 0;

    cliflag = V2R1_DISABLE;
	
    lRet = SetOnuRegisterLimitFlagAll( cliflag );	
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
    pon_show_onu_regiseter_limit_all_config,
    pon_show_onu_regiseter_limit_all_config_cmd,
    "show register limit-onu ",
    DescStringCommonShow
    "Show all onu register information\n"
    "Show all onu limit register infomation\n"
    )
{
	LONG lRet = VOS_OK;
	INT16 phyPonId = 0;
	INT16 slotId = 0;
	int  cliflag	 = 0;
	vty_out(vty, "     Pon Idx        limit flag \r\n");
	for(slotId=PONCARD_FIRST; slotId<=PONCARD_LAST; slotId++)
		{
		for(phyPonId=1; phyPonId<=PONPORTPERCARD ; phyPonId ++)
			{		
			if( getPonChipInserted(slotId, phyPonId) == PONCHIP_NOT_EXIST ) continue;

			lRet = GetOnuRegisterLimitFlag(phyPonId, &cliflag );
			vty_out(vty, "   %s/%d", CardSlot_s[slotId], phyPonId );

			if (lRet == VOS_ERROR)
				{
#ifdef CLI_EPON_DEBUG
				vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
#endif     
				vty_out( vty, "  %% Getting is wrong.\r\n");
				return CMD_WARNING;
				}

			if (cliflag == V2R1_ENABLE)
				{
				vty_out( vty, "      enable\r\n");
				}
			else if (cliflag == V2R1_DISABLE)
				{
				vty_out( vty, "      disable\r\n");
				}
			else
				{
#ifdef CLI_EPON_DEBUG
				vty_out(vty, "  %% DEBUG: lRet %d  phyPonId %d  cliflag %d.\r\n",lRet, phyPonId,cliflag);
#endif     
				vty_out( vty, "     unKnown\r\n");
				return CMD_WARNING;
				}
			}
		}	
    return CMD_SUCCESS;
}
#endif

/*该历史统计的时间设置为全局变量
不可针对每一个对象进行设置*/
DEFUN  (
    olt_history_15m_time_interval_config,
    olt_history_15m_time_interval_config_cmd,
    "statistic-history bucket-num 15m <1-200>",
    /*"history-statistic [pon|<llid>] time-interval [15|24] <bucket>",*/
    "Set the history statistic attribute\n"
    "bucket number\n"
    "The statistic time-interval is 15 minutes\n"
    "Please input the bucket number of statistic cycles\n"
    )
{
    LONG lRet = VOS_OK;
    /*INT16 phyPonId = 0;	*/
    /*UINT32 statType = 0;*/
    UINT32 timeStat = 0;

    timeStat = ( UINT16 ) VOS_AtoL( argv[ 0 ] );  
	lRet = HisStats15MinMaxRecordSet(timeStat);

    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}


/*该历史统计的时间设置为全局变量
不可针对每一个对象进行设置*/
DEFUN  (
    olt_history_24H_time_interval_config,
    olt_history_24H_time_interval_config_cmd,
    "statistic-history bucket-num 24h <1-60>",
    /*"history-statistic [pon|<llid>] time-interval [15|24] <bucket>",*/
    "Set the history statistic attribute\n"
    "bucket number\n"
    "The statistic time-interval is 24 hours\n"
    "Please input the bucket number of statistic cycles\n"
    )
{
    LONG lRet = VOS_OK;
    UINT32 timeStat = 0;

    timeStat = ( UINT16 ) VOS_AtoL( argv[ 0 ] );  
    lRet = HisStats24HoursMaxRecordSet(timeStat);

    if (lRet != VOS_OK)
    {
    	vty_out( vty, "  %% Executing error.\r\n" );
	return CMD_WARNING;
    }	   	
    return CMD_SUCCESS;
}

DEFUN  (
    olt_show_statistic,
    olt_show_statistic_cmd,
    "show statistic-history bucket-num",
    DescStringCommonShow
    "Show history-stats information\n"
    "bucket number of the history statistic\n"
    )
{

    unsigned int syscle_time24H = 0;
    unsigned int syscle_time15M = 0;
    HisStats24HoursMaxRecordGet( &syscle_time24H );
    HisStats15MinMaxRecordGet( &syscle_time15M );
    vty_out( vty, "\r\n%10s%12s\r\n", "interval", "bucket" );	/* modified by xieshl 20101025，问题单10579 */
    vty_out( vty, " %7s%11d\r\n", "15Min", syscle_time15M);
    vty_out( vty, " %7s%11d\r\n", "24h", syscle_time24H);

    return CMD_SUCCESS;
}

/* modified by chenfj 2008-2-27 #6349
     在显示ONU 列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */

DEFUN  (
    olt_show_list_onu_count,
    olt_show_list_onu_count_cmd,
    "show onu-list {[type] <typestring>}*1 {[pon] <slot/port>}*1 {[count]}*1",
    DescStringCommonShow
    "Show current onu list information\n"
    "onu type\n"
    /* the specific onu type-string,e.g"DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n" */
    "the specific onu type-string\n"
    "Show pon`s onu list information\n"
    "Please input slot/port\n"
    "Show onu list count\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int phyPonId = 0;
	int OptionFlag = 0;
	/*int i;*/
	/*int stringLen;*/

	 if( (argc & 1) == 1 )
		{
		OptionFlag = 1;	
		argc--;
		}

	 if( argc == 0 )
	 	{
		if( OptionFlag == 1 )
			ShowPonPortOnuMacAddrCounterByVtyAll(NULL, vty );
		else
			ShowPonPortOnuMacAddrByVtyAll(NULL, vty );
	 	}
	
	else if( argc == 2 )
		{
		
		if(VOS_StrCmp(argv[0],"type") == 0)
			{
			/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
			if(SearchOnuType(argv[1]) == RERROR)
				{
				NotificationOnuTypeString(vty);	
				return(CMD_WARNING);
				}
			
			if( OptionFlag == 1 )
				ShowPonPortOnuMacAddrCounterByVtyAll(argv[1], vty );
			else 
				ShowPonPortOnuMacAddrByVtyAll(argv[1], vty );			
			}
		
		else if(VOS_StrCmp(argv[0],"pon") == 0)
			{
			sscanf( argv[1], "%d/%d", &ulSlot, &ulPort);
			if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
				return(CMD_WARNING);
			phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
			if (phyPonId == VOS_ERROR)
				{ 
				    vty_out( vty, "  %% Parameter is error.\r\n" );
				    return CMD_WARNING;
				}
			if( OptionFlag == 1 )
				ShowPonPortOnuMacAddrCounterByVty( phyPonId, NULL, vty );
			else 
				ShowPonPortOnuMacAddrByVty( phyPonId, NULL, vty );
			}
		}
	
	else if( argc == 4 )
		{
		/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
		if(SearchOnuType(argv[1]) == RERROR)
			{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
			}
			
		sscanf( argv[3], "%d/%d", &ulSlot, &ulPort);
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
			{
			    vty_out( vty, "  %% Parameter is error.\r\n" );
			    return CMD_WARNING;
			}
		
		if( OptionFlag == 1 )
			ShowPonPortOnuMacAddrCounterByVty( phyPonId, argv[1], vty );
		else 
			ShowPonPortOnuMacAddrByVty( phyPonId, argv[1], vty );
		}
	
    return CMD_SUCCESS;
}

DEFUN  (
    olt_show_list_onu_by_mac,
    olt_show_list_onu_by_mac_cmd,
    "show onu-list mac <H.H.H>",
    DescStringCommonShow
    "Show current onu list information\n"
    "Onu mac address\n"
    "the specific onu mac address\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int PonPortIdx = 0;
	int OptionFlag = 0;
    char MacAddr[6] = {0,0,0,0,0,0};
    short int counter = 0;
	
    if ( GetMacAddr( ( CHAR* ) argv[0], MacAddr ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }
	
	for( PonPortIdx = 0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		counter += AllOnuCounterByMac(PonPortIdx, MacAddr);
	}
	
	if(counter == 0)
	{
		return CMD_SUCCESS;
	}
	
	vty_out( vty, "\r\n[TOTAL ONU COUNTER = %d]\r\n", counter);
    show_onu_list_banner( vty );
    
    for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
    {
        if( SearchValidOnuEntry(PonPortIdx) == ROK )
        {
            short int OnuIdx = 0;  
			counter = AllOnuCounterByMac(PonPortIdx, MacAddr);

			if(counter == 0)
				continue;
			
            vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), counter);
            
            /*  ONU_MGMT_SEM_TAKE;*/
            for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
            {
                unsigned char name[MAXDEVICENAMELEN+1];
                int nameLen = 0;           
                int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
                int status = 0;
                if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
                {                    
                    char local_mac[6];
                    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);

                    if(!MacAddr[3] && !MacAddr[4] && !MacAddr[5])
                    {
                        /*可以根据oui进行筛选*/
                        if((*(USHORT*) (&MacAddr[0]) != *(USHORT*) (&local_mac[0])) || (MacAddr[2] != local_mac[2]))
                            continue;
                    }
                    else
                    {
                        /*如果后三个字节不全为0，即需要全匹配*/
                        if(MAC_ADDR_IS_UNEQUAL(local_mac, MacAddr))
                            continue;
                    }
                    vty_out( vty, "%2d  ", (OnuIdx+1));
                    vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", local_mac[0], local_mac[1],
                                                                local_mac[2], local_mac[3],
                                                               local_mac[4], local_mac[5]);
        
                    if( (GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK) || (nameLen == 0) || (nameLen > MAXDEVICENAMELEN) )
                    {
                        VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );    
                    }
                    vty_out( vty, "  %-12s  ", name );  
        
                    status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
                    if( (status < 0) || (status > 5) )
                        status = 0;
                    vty_out(vty," %s ", OnuCurrentStatus[status]);
        
                    if(OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year)
                    {
                        vty_out(vty,"%04d/%02d/%02d,%02d:%02d:%02d", 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.month, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.day, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.hour,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.minute,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
                    }
                    else
                        vty_out(vty,"%04s/%02s/%02s,%02s:%02s:%02s", 
                            "----", "--", "--", "--", "--", "--");
                    
                    if(OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year)
                    {
                        vty_out(vty,"  %04d/%02d/%02d,%02d:%02d:%02d", 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.month, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.day, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.hour,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.minute,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
                    }
                    else
                        vty_out(vty,"  %04s/%02s/%02s,%02s:%02s:%02s", 
                            "----", "--", "--", "--", "--", "--");
                    
                    nameLen = 0;
                    GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
                    if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
                    name[nameLen] = '\0';
                    vty_out(vty, /*"   valid*/"  %s\r\n", name); 
                }
            }
            /*  ONU_MGMT_SEM_GIVE;*/
            vty_out(vty, "\r\n");
        }
    }
    

    return CMD_SUCCESS;
}
DEFUN  (
    olt_show_list_onu_by_sn,
    olt_show_list_onu_by_sn_cmd,
    "show onu-list sn <sn>",
    DescStringCommonShow
    "Show current onu list information\n"
    "Onu serial number\n"
    "the specific onu serial number\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int PonPortIdx = 0;
	int OptionFlag = 0;
    UCHAR SN[GPON_ONU_SERIAL_NUM_STR_LEN] = {0};
	UCHAR SNLen=0;
    short int counter = 0;
	VOS_MemZero(SN,GPON_ONU_SERIAL_NUM_STR_LEN);

	if( (SNLen = VOS_StrLen(argv[0])) > 16 ) 
	{
		vty_out(vty,"  %%  serial number %s is invalid\r\n", (unsigned char *)argv[0] );
		return(CMD_WARNING );
	}
	else
	{
		VOS_MemCpy(SN, argv[0], SNLen);
	}
	
	for( PonPortIdx = 0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		counter += AllOnuCounterBySn(PonPortIdx, SN);
	}
	
	if(counter == 0)
	{
		return CMD_SUCCESS;
	}
	
	vty_out( vty, "\r\n[TOTAL ONU COUNTER = %d]\r\n", counter);
    show_onu_list_banner( vty );
    
    for(PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
    {
        if( SearchValidOnuEntry(PonPortIdx) == ROK )
        {
            short int OnuIdx = 0;  
			counter = AllOnuCounterBySn(PonPortIdx, SN);

			if(counter == 0)
				continue;
			
            vty_out(vty, "pon%d/%d [onu counter = %d]\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), counter);
            
            /*  ONU_MGMT_SEM_TAKE;*/
            for(OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx++)
            {
                unsigned char name[MAXDEVICENAMELEN+1];
                int nameLen = 0;           
                int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
                int status = 0;
                if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
                {                    
                    char local_sn[GPON_ONU_SERIAL_NUM_STR_LEN];
					VOS_MemZero(local_sn,GPON_ONU_SERIAL_NUM_STR_LEN);
                    VOS_MemCpy(local_sn, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN-1);
					if(OnuMgmtTable[OnuEntry].IsGponOnu == 0)
						continue;
                    if(strstr(local_sn,SN) == 0)
						continue;
                    vty_out( vty, "%2d  ", (OnuIdx+1));
                    vty_out(vty, "%s",local_sn);
        
                    if( (GetOnuModel(PonPortIdx, OnuIdx, name, &nameLen) != ROK) || (nameLen == 0) || (nameLen > MAXDEVICENAMELEN) )
                    {
                        VOS_StrCpy( name, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );    
                    }
                    vty_out( vty, "  %-12s  ", name );  
        
                    status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
                    if( (status < 0) || (status > 5) )
                        status = 0;
                    vty_out(vty," %s ", OnuCurrentStatus[status]);
        
                    if(OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year)
                    {
                        vty_out(vty,"%04d/%02d/%02d,%02d:%02d:%02d", 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.month, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.day, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.hour,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.minute,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
                    }
                    else
                        vty_out(vty,"%04s/%02s/%02s,%02s:%02s:%02s", 
                            "----", "--", "--", "--", "--", "--");
                    
                    if(OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year)
                    {
                        vty_out(vty,"  %04d/%02d/%02d,%02d:%02d:%02d", 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.month, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.day, 
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.hour,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.minute,
                            OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.second);    /* modified by xieshl 20100319, GWD网上问题9966 */
                    }
                    else
                        vty_out(vty,"  %04s/%02s/%02s,%02s:%02s:%02s", 
                            "----", "--", "--", "--", "--", "--");
                    
                    nameLen = 0;
                    GetOnuDeviceName( PonPortIdx, OnuIdx, name, &nameLen);
                    if(nameLen > MAXDEVICENAMELEN ) nameLen = MAXDEVICENAMELEN;
                    name[nameLen] = '\0';
                    vty_out(vty, /*"   valid*/"  %s\r\n", name); 
                }
            }
            /*  ONU_MGMT_SEM_GIVE;*/
            vty_out(vty, "\r\n");
        }
    }
    

    return CMD_SUCCESS;
}

/* removed by xieshl 20110117, 与下面命令合并，问题单11919 */
/*DEFUN  (
    olt_all_auto_updated_show,
    olt_all_auto_updated_show_cmd,
    "show onu software update",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu information\n"
    "Show onu update information\n"
    )
{
	vty_out( vty, "\r\n" );
	if ( argc == 0 )
		ShowOnuSoftwareAutoUpdateAllByVty( vty );
#if 0
	else if (argc == 1)
	{
	    if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
			EnableFlag = V2R1_ENABLE;
		else if ( !VOS_StrCmp( argv[ 0 ], "disable" ) )
			EnableFlag = V2R1_DISABLE;
		else
	    {
	        vty_out( vty, "  %% Parameter is error.\r\n" );
	        return CMD_WARNING;
	    }
	}
	if (argc == 1)
	{
	    if ( EnableFlag == V2R1_ENABLE )
				ShowOnuSoftwareAutoUpdateAllEnabledByVty( vty );
		else if ( EnableFlag == V2R1_DISABLE )
			ShowOnuSoftwareAutoUpdateAllDisabledByVty( vty );
	}
	else if (argc == 2)
	{
		VOS_Sscanf(argv[1], "%d/%d", &ulSlot, &ulPort);
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);
	    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if(phyPonId == (VOS_ERROR))
		{
			vty_out( vty, "  %% Parameter error.Please input right slotId/port.\r\n");
			return CMD_WARNING;
		}
		if (EnableFlag == V2R1_ENABLE)
			ShowOnuSoftwareAutoUpdatePonEnabledByVty( phyPonId, vty );
		else if (EnableFlag == V2R1_DISABLE)
			ShowOnuSoftwareAutoUpdatePonDisabledByVty( phyPonId, vty );		
	}
#endif
    return CMD_SUCCESS;
}*/


/*
DEFUN( onu_online_show,
        onu_online_show_cmd,
        "show online-onu {[count]}*1",
        DescStringCommonShow
        "Show current online onu information\n"
        "Show current online onu count\n"
        )
{
	if (argc == 0)
		ShowPonPortOnLineOnuByVtyAll( vty );
	else
	{
		ShowPonPortOnLineOnuCounterByVtyAll( vty );
	}
	return CMD_SUCCESS;	
}
*/

/* modified by chenfj 2008-3-20 #6349
     在显示在线ONU  列表时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */

DEFUN( olt_onu_online_count_show,
        olt_onu_online_show_count_cmd,
         "show online-onu {[type] <typestring>}*1 {[pon] <slot/port>}*1 {[count]}*1",
        /*"show online-onu pon <slot/port> {[count]}*1",*/
        DescStringCommonShow
        "show current online onu list\n"
         "onu type\n"
         /* the specific onu type-string,e.g "DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n" */
        "the specific onu type-string\n"
        "show pon`s online onu information\n"
        "Please input slot/port\n"
        "show online onu count\n"
        )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int phyPonId = 0;
	int OptionFlag = 0;
	/*int i;
	int stringLen;*/

	 if( (argc & 1) == 1 )
		{
		OptionFlag = 1;	
		argc--;
		}

	 if( argc == 0 )
	 	{
		if( OptionFlag == 1 )
			ShowPonPortOnLineOnuCounterByVtyAll(NULL, vty );
		else
			ShowPonPortOnLineOnuByVtyAll(NULL, vty );
	 	}
	
	else if( argc == 2 )
		{
		
		if(VOS_MemCmp(argv[0],"type",4) == 0)
			{
			/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
			if(SearchOnuType(argv[1]) == RERROR)
				{
				NotificationOnuTypeString(vty);	
				return(CMD_WARNING);
				}
			
			if( OptionFlag == 1 )
				ShowPonPortOnLineOnuCounterByVtyAll(argv[1], vty );
			else 
				ShowPonPortOnLineOnuByVtyAll(argv[1], vty );			
			}
		
		else if(VOS_MemCmp(argv[0],"pon",4) == 0)
			{
			sscanf( argv[1], "%d/%d", &ulSlot, &ulPort);
			if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
				return(CMD_WARNING);
			phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
			if (phyPonId == VOS_ERROR)
				{ 
				    vty_out( vty, "  %% Parameter is error.\r\n" );
				    return CMD_WARNING;
				}
			if( OptionFlag == 1 )
				ShowPonPortOnLineOnuCounterByVty( phyPonId, NULL, vty );
			else 
				ShowPonPortOnLineOnuByVty( phyPonId, NULL, vty );
			}
		}
	
	else if( argc == 4 )
		{
		/* modified by chenfj 2009-6-3
		增加显示信息，用于提示ONU 类型*/
		if(SearchOnuType(argv[1]) == RERROR)
			{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
			}
			
		sscanf( argv[3], "%d/%d", &ulSlot, &ulPort);
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
			{
			    vty_out( vty, "  %% Parameter is error.\r\n" );
			    return CMD_WARNING;
			}
		
		if( OptionFlag == 1 )
			ShowPonPortOnLineOnuCounterByVty( phyPonId, argv[1], vty );
		else 
			ShowPonPortOnLineOnuByVty( phyPonId, argv[1], vty );
		}
	
    return CMD_SUCCESS;
}
/*add byshixh20090429*/
DEFUN( olt_onu_offline_count_show,
        olt_onu_offline_show_count_cmd,
         "show offline-onu {[type] <typestring>}*1 {[pon] <slot/port>}*1 {[count]}*1",
        DescStringCommonShow
        "show current offline onu list\n"
         "onu type\n"
        "the specific onu type-string\n" /*DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n"*/
        "show pon`s offline onu information\n"
        "Please input slot/port\n"
        "show offline onu count\n"
        )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int phyPonId = 0;
	int OptionFlag = 0;
	/*int i;
	int stringLen;*/

	 if( (argc & 1) == 1 )
		{
		OptionFlag = 1;	
		argc--;
		}

	 if( argc == 0 )
	 	{
		if( OptionFlag == 1 )
			ShowPonPortOffLineOnuCounterByVtyAll(NULL, vty );
		else
			ShowPonPortOffLineOnuByVtyAll(NULL, vty );
	 	}
	
	else if( argc == 2 )
		{
		
		if(VOS_MemCmp(argv[0],"type",4) == 0)
			{
			/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
			if(SearchOnuType(argv[1]) == RERROR)
				{
				NotificationOnuTypeString(vty);	
				return(CMD_WARNING);
				}
			
			if( OptionFlag == 1 )
				ShowPonPortOffLineOnuCounterByVtyAll(argv[1], vty );
			else 
				ShowPonPortOffLineOnuByVtyAll(argv[1], vty );			
			}
		
		else if(VOS_MemCmp(argv[0],"pon",4) == 0)
			{
			sscanf( argv[1], "%d/%d", &ulSlot, &ulPort);
			if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
				return(CMD_WARNING);
			phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
			if (phyPonId == VOS_ERROR)
				{ 
				    vty_out( vty, "  %% Parameter is error.\r\n" );
				    return CMD_WARNING;
				}
			if( OptionFlag == 1 )
				ShowPonPortOffLineOnuCounterByVty( phyPonId, NULL, vty );
			else 
				ShowPonPortOffLineOnuByVty( phyPonId, NULL, vty );
			}
		}
	
	else if( argc == 4 )
		{
		/* modified by chenfj 2009-6-3
			增加显示信息，用于提示ONU 类型*/
		if(SearchOnuType(argv[1]) == RERROR)
			{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
			}
			
		sscanf( argv[3], "%d/%d", &ulSlot, &ulPort);
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
			{
			    vty_out( vty, "  %% Parameter is error.\r\n" );
			    return CMD_WARNING;
			}
		
		if( OptionFlag == 1 )
			ShowPonPortOffLineOnuCounterByVty( phyPonId, argv[1], vty );
		else 
			ShowPonPortOffLineOnuByVty( phyPonId, argv[1], vty );
		}
	
    return CMD_SUCCESS;
}

DEFUN (
		olt_del_offline_onu_config,
		olt_del_offline_onu_config_cmd,
		"delete offline-onu",
		"delete onu from all pon port\n"
		"delete offline-onu from all pon port\n"
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

    for(phyPonId=0; phyPonId<MAXPON; phyPonId++)
    {	
    	if(V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(phyPonId)) continue;
		
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
    }
	return CMD_SUCCESS;
}


DEFUN( olt_onu_infomation_show,
        olt_onu_infomation_cmd,
        "show onu information",
        DescStringCommonShow
        "Onu config\n"
        "Show onu information\n"
        )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	short int PonPortIdx = 0;
	short int OnuIdx = 0;
    short int online_gw[V2R1_DEVICE_LAST+1];/*以类型为下标*/
    short int offline_gw[V2R1_DEVICE_LAST+1];/*以类型为下标*/
    short int online_other[ONU_MAX_PORT+1];/*以端口为下标*/
    short int offline_other[ONU_MAX_PORT+1];/*以端口为下标*/
    short int online = 0;
    short int offline = 0;
    short int i = 0, j = 1;
    VOS_MemZero(online_gw, (V2R1_DEVICE_LAST+1)*2);		 
    VOS_MemZero(offline_gw, (V2R1_DEVICE_LAST+1)*2);		 
    VOS_MemZero(online_other, (ONU_MAX_PORT+1)*2);		 
    VOS_MemZero(offline_other, (ONU_MAX_PORT+1)*2);		 

	for(PonPortIdx = 0; PonPortIdx<MAXPON; PonPortIdx++)
	{
	    for(OnuIdx = 0; OnuIdx<MAXONUPERPON; OnuIdx++)
	    {
	        if(VOS_OK != ThisIsValidOnu(PonPortIdx, OnuIdx))
	            continue;
	        else
	        {
	            int onu_status = GetOnuOperStatus(PonPortIdx, OnuIdx);
	            int onu_type = 0;
	            
	            GetOnuType(PonPortIdx, OnuIdx, &onu_type);
	            if(onu_type == V2R1_ONU_CTC)
	            {
                    ULONG module = 0, length = 0;
                    char onu_module[80] = "";
                    ULONG liv_typeid = 0;
	            
    		        ONU_MGMT_SEM_TAKE
    		        module = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].onu_model;
    		        ONU_MGMT_SEM_GIVE
    		        if(CTC_OnuModel_Translate(module, &liv_typeid, onu_module, &length) == VOS_OK)
    		        {
    		        	onu_type = liv_typeid;
    		        }
	            }

	            if(onu_type != V2R1_ONU_CTC)
	            {
	                if(onu_type > V2R1_DEVICE_LAST)
	                    continue;
	                    
            	    if(onu_status == ONU_OPER_STATUS_DOWN)
            	    {
            	        offline_gw[onu_type]++;
            	    }
            	    else if(onu_status == ONU_OPER_STATUS_UP)
            	    {
            	        online_gw[onu_type]++;
            	    }
        	    }
        	    else
        	    {
        	        short int portNum = getOnuEthPortNum(PonPortIdx, OnuIdx);
        	        if(portNum > ONU_MAX_PORT)
            	        continue;
            	        
            	    if(onu_status == ONU_OPER_STATUS_DOWN)
            	    {
            	        offline_other[portNum]++;
            	    }
            	    else if(onu_status == ONU_OPER_STATUS_UP)
            	    {
            	        online_other[portNum]++;
            	    }
        	    }
    	    }
	    }
	}

    vty_out(vty, "\r\n%2s %12s  %14s  %14s  %15s\r\n", "No", "DEVICE TYPE", "TOTAL NUMBER", "ONLINE NUMBER", "OFFLINE NUMBER");
    vty_out(vty, "-----------------------------------------------------------------------\r\n");
	for(i=0;i<=V2R1_ONU_MAX;i++)
	{   
	    if(i == 0)
	    {
    	    if(online_gw[i] || offline_gw[i])
    	    {
                vty_out(vty, "%2d %12s  %14d  %14d  %15d\r\n", j++, "UNKNOWN", online_gw[i]+offline_gw[i], online_gw[i], offline_gw[i]);
                online += online_gw[i];
                offline += offline_gw[i];
    	    }	  
	    }
	    else if(i == 1)
	    {
    	    if(online_gw[i] || offline_gw[i])
    	    {
                vty_out(vty, "%2d %12s  %14d  %14d  %15d\r\n", j++, "-", online_gw[i]+offline_gw[i], online_gw[i], offline_gw[i]);
                online += online_gw[i];
                offline += offline_gw[i];
    	    }	  
	    }
	    else if(i>=V2R1_ONU_GT811)
	    {
    	    if(online_gw[i] || offline_gw[i])
    	    {
                vty_out(vty, "%2d %12s  %14d  %14d  %15d\r\n", j++, onutypeint_to_str(i), online_gw[i]+offline_gw[i], online_gw[i], offline_gw[i]);
                online += online_gw[i];
                offline += offline_gw[i];
    	    }
	    }
	}
	for(i=0;i<=ONU_MAX_PORT;i++)
	{
	    if(online_other[i] || offline_other[i])
	    {
	        char onu_str[10] = {0};
	        VOS_Sprintf(onu_str, "OTHER-%d", i);
            vty_out(vty, "%2d %12s  %14d  %14d  %15d\r\n", j++, onu_str, online_other[i]+offline_other[i], online_other[i], offline_other[i]);
            online += online_other[i];
            offline += offline_other[i];
	    }
	}
    vty_out(vty, "-----------------------------------------------------------------------\r\n");
    vty_out(vty, "%2s %12s  %14d  %14d  %15d\r\n\r\n", " ", "", online+offline, online, offline);

    return CMD_SUCCESS;
}

#if 0	/* removed by xieshl 20160421, 移到PON口下定义 */
/*DEFUN(
	olt_loop_info_show, 
	olt_loop_info_show_cmd,
	"show pon-loop information",
	DescStringCommonShow

/*added by wutw 2006/12/14
#define ONU_RANGE_20KM	1
#define ONU_RANGE_40KM	2
#define ONU_RANGE_60KM	3
*/
DEFUN(
	olt_range_set,
	olt_range_set_cmd, 
	"onu-register window pon <portlist> [20km|40km|close]", 
	"set onu register max window\n"
	"set onu register max window\n"
	"the pon port specific\n"
	"Please input slot/port\n"
	"onu register window 20km\n"
	"onu register window 40km\n"
	"onu register window close\n"
	/*"30km"*/
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
	int count = 0;
	int range = 0;
	ULONG ulIfIndex=0;
	int iRes = 0;
	LONG lRet = 0;
		
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
	
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	if (!VOS_StrCmp((CHAR *)argv[1], "20km"))
		range = PON_RANGE_20KM;
	else if (!VOS_StrCmp((CHAR *)argv[1], "40km"))
		range = PON_RANGE_40KM;
	else if (!VOS_StrCmp((CHAR *)argv[1], "60km"))
		range = PON_RANGE_60KM;
	else if(!VOS_StrCmp((CHAR *)argv[1], "close"))
		range = PON_RANGE_CLOSE;
	else if (1 == count)
		{
		vty_out( vty, "  %% Parameter error\r\n");
		return CMD_WARNING;
		}		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
				continue;
		}	
		
		PonPortTable[phyPonId].range = range;

		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
			if(OLTAdv_IsExist( phyPonId ) != TRUE )
				{
				vty_out(vty, "  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
				continue;
				}
		
		iRes = SetPonRange( phyPonId, range );
		if (iRes != VOS_OK)
		{
			if(1==count)
			{
				vty_out( vty, "  %% Executing error\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		}		
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		
	return CMD_SUCCESS;
}

DEFUN(
	olt_range_show,
	olt_range_show_cmd, 
	"show onu-register window pon <portlist>",
	DescStringCommonShow
	"show onu-register window\n"
	"show onu-register window\n"
	"the pon port specific\n"
	"input the slot/port\n"
	)
{
	int iRes = 0;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    ULONG ulIfIndex = 0;
    INT16 phyPonId = 0;
	int range = 0;
	int count = 0;

	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		iRes = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( iRes != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
	
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()


	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		iRes = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( iRes != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif   
			if(1==count)
			{
		    	vty_out( vty, "  %% Executing error\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
				continue;
		}	
		iRes = GetPonRange( phyPonId, &range );
		if ( VOS_OK != iRes)
		{	
			if(1==count)
			{
				vty_out( vty, "  %% Executing error\r\n");
				RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
				continue;
		}
		vty_out(vty, "  pon %d/%d onu-register window is ", ulSlot, ulPort);
		if(PON_RANGE_20KM == range )
			vty_out( vty, "20km\r\n" );
		else if( PON_RANGE_40KM == range )
			vty_out( vty, "40km\r\n" );
		else if( PON_RANGE_60KM == range )		
			vty_out( vty, "60km\r\n" );
		else if( PON_RANGE_CLOSE == range )
			vty_out( vty, "closed\r\n" );
		else if (1==count)
		{
			vty_out( vty, "unknown\r\n");
			/*return CMD_WARNING;*/
		}
		/*else
			continue;*/
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()	

	return CMD_SUCCESS;
}
#endif


/*added by suipl 2006/11/14 for testing
DEFUN (
    olt_test_onuid_list,
    olt_test_onuid_list_cmd,
    "test onuid_list <onuid_list>",
    "Test \n"
    "Test port_list\n"
   OnuIDStringDesc
)
{
	ULONG ulOnuId=0;

	
	if ( !V2R1_Check_OnuId_List_1( argv[ 0 ] ) )
	{
		vty_out( vty, "%% Invalid onuid list <%s>. \r\n", argv[ 0 ] );
		return CMD_SUCCESS;
	}

	vty_out( vty, "Input onuid is :" );

	BEGIN_PARSE_ONUID_LIST_TO_ONUID_1( argv[ 0 ], ulOnuId )
	{
		vty_out( vty, "%d  ", ulOnuId );
	}
	END_PARSE_ONUID_LIST_TO_ONUID_1();

	vty_out( vty, "\r\n");

	return CMD_SUCCESS;
}
*/
/*	UCHAR file[20480] = "";*/
/*add by wangxy 2007-03-12*/

int CheckOltCurAndFlashVerIsTheSame()
{
    int ret = VOS_OK;
	int size = 128;
	UCHAR file[128] = {0};	
	app_desc_t* pdesc = NULL;
	UCHAR *p = NULL;

	VOS_MemSet(file, 0, size );
	xflash_file_read( OLT_FLASH_FILE_ID, &file[0], &size );
	
	pdesc = (app_desc_t*)file;    		
    if(VOS_StrCmp(V2R1_VERSION, pdesc->file_ver) != 0)
        ret = VOS_ERROR;
    return ret;
}
DEFUN(
	app_file_show,
	app_file_show_cmd,
	"show app-file [olt|olt-10gepon|olt-gpon|onu]",
	"Show  information\n"
	DescStringCommonShow
	/*"Show app file information\n"*/
	"the app file for olt\n"
	"the app file for olt-10gepon\n"
	"the app file for olt-gpon\n"
	"the app file for onu\n"
)
{
	if( VOS_StrCmp( argv[0], "olt" ) == 0 )
	{
		int size = 128;
		UCHAR file[128] = {0};	
		app_desc_t* pdesc = NULL;
		UCHAR *p = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( OLT_FLASH_FILE_ID, &file[0], &size );
		
		pdesc = (app_desc_t*)file;

		p = (UCHAR*)&pdesc->file_len;
		
		pdesc->file_len = MAKELONG( MAKEWORD(*p,*(p+1)), MAKEWORD(*(p+2), *(p+3)) );
		
		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		/*if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID))) == 0)
			&& (pdesc->file_len !=  0))*/
		if((CompareOltAppKeyId(&pdesc->dev_type[0]) == ROK) && (pdesc->file_len !=  0))
		/*if((pdesc->file_len !=  0) && (pdesc->dev_type[0] != 0 ))*/
		vty_out( vty, "\r\n\t%-16s\t%-10u\t%s\r\n", typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID)/*pdesc->dev_type*/, pdesc->file_len, pdesc->file_ver );
				
	}
	else if( VOS_StrCmp( argv[0], "olt-10gepon" ) == 0 )
	{
		int size = 128;
		UCHAR file[128] = {0};	
		app_desc_t* pdesc = NULL;
		UCHAR *p = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( FLASH_APP_CODE_ARAD, &file[0], &size );
		
		pdesc = (app_desc_t*)file;

		p = (UCHAR*)&pdesc->file_len;
		
		pdesc->file_len = MAKELONG( MAKEWORD(*p,*(p+1)), MAKEWORD(*(p+2), *(p+3)) );
		
		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		/*if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID))) == 0)
			&& (pdesc->file_len !=  0))*/
		if((CompareOltAppKeyId(&pdesc->dev_type[0]) == ROK) && (pdesc->file_len !=  0))
		/*if((pdesc->file_len !=  0) && (pdesc->dev_type[0] != 0 ))*/
		vty_out( vty, "\r\n\t%-16s\t%-10u\t%s\r\n", "OLTAPP_ARAD"/*pdesc->dev_type*/, pdesc->file_len, pdesc->file_ver );
				
	}
	else if( VOS_StrCmp( argv[0], "olt-gpon" ) == 0 )
	{
		int size = 128;
		UCHAR file[128] = {0};	
		app_desc_t* pdesc = NULL;
		UCHAR *p = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( FLASH_APP_CODE_GPON, &file[0], &size );
		
		pdesc = (app_desc_t*)file;

		p = (UCHAR*)&pdesc->file_len;
		
		pdesc->file_len = MAKELONG( MAKEWORD(*p,*(p+1)), MAKEWORD(*(p+2), *(p+3)) );
		
		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		/*if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID))) == 0)
			&& (pdesc->file_len !=  0))*/
		if((CompareOltAppKeyId(&pdesc->dev_type[0]) == ROK) && (pdesc->file_len !=  0))
		/*if((pdesc->file_len !=  0) && (pdesc->dev_type[0] != 0 ))*/
		vty_out( vty, "\r\n\t%-16s\t%-10u\t%s\r\n", "OLTAPP_GPON"/*pdesc->dev_type*/, pdesc->file_len, pdesc->file_ver );
				
	}
	else if( VOS_StrCmp( argv[0], "onu" ) == 0 )
	{
		int size = 2048, i=0;
		UCHAR file[2048]="";
		app_desc_t *pdesc = NULL;
		extern CHAR GPON_ONUAPP_FILE_HAED[];

		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		{
			pdesc = (app_desc_t*)GPON_ONUAPP_FILE_HAED; 
		}
		else
		{	
			VOS_MemSet(file, 0, size );
			xflash_file_read( ONU_FLASH_FILE_ID, file, &size );

			pdesc = (app_desc_t*)file;
		}


		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		for( i=0; i<(2048/128); i++ )
		{
			char *p = (char*)&pdesc->file_len;
			/* modified by chenfj 
			问题单:6712.PON节点下delete fdbentry mac 为ONU的MAC时无错误提示 
			增加通过文件名和文件长度检查文件有效性
			*/
			/*
			if( pdesc->dev_type[0] == 0 )
				break;.
			*/	
			pdesc->file_len = MAKELONG( MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) );
			if((pdesc->file_len != 0) &&
				(VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_ONU_EPONAPP_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_ONU_EPONAPP_KEY_ID))) == 0))
			vty_out( vty, "\r\n%d\t%-16s\t%-10u\t%s", i+1, pdesc->dev_type, pdesc->file_len, pdesc->file_ver );
			pdesc++;
			
		}
		vty_out(vty, "\r\n" );
	}
	else
		return CMD_WARNING;
	
	return CMD_SUCCESS;	
}

DEFUN(
	driver_file_show,
	driver_file_show_cmd,
	"show driver-file [EPON|GPON|DBA]",
	"Show  information\n"
	DescStringCommonShow
	/*"Show app file information\n"*/
	"the epon chip Firmware Image\n"
	"the gpon chip Firmware Image\n"
	"the pon chip DBA Image\n"
)
{
	int FileId = 0;
	if( VOS_StrCmp( argv[0], "epon" ) == 0 )
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			FileId = 0;
		}
		else 
		{
			FileId = PON_FW_FLASH_ID;
		}

	}
	else if( VOS_StrCmp( argv[0], "gpon" ) == 0 )
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			FileId = PON_FW_FLASH_ID;
		}
		else
		{
			FileId = FLASH_FIRMWARE_GPON;
		}
	}
	else if( VOS_StrCmp( argv[0], "dba" ) == 0 )
	{
		FileId = PON_DBA_FLASH_ID;
	}
	


	if( VOS_StrCmp( argv[0], "epon" ) == 0 )
	{
		int size = 2048, i=0;
		UCHAR file[2048]="";
		app_desc_t *pdesc = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( FileId, file, &size );

		pdesc = (app_desc_t*)file;


		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		for( i=0; i<(2048/128); i++ )
		{
			char *p = (char*)&pdesc->file_len;
			/*
			if( pdesc->dev_type[0] == 0 )
				break;
			*/
			
			pdesc->file_len = MAKELONG( MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) );
			
			if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_PON_FIRMWARE_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_PON_FIRMWARE_KEY_ID))) == 0)
			&& (pdesc->file_len !=  0))
			vty_out( vty, "\r\n%d\t%-16s\t%-10u\t%s", i+1, pdesc->dev_type, pdesc->file_len, pdesc->file_ver );
			pdesc++;
			
		}
		vty_out(vty, "\r\n" );
	}
	else if( VOS_StrCmp( argv[0], "gpon" ) == 0 )
	{
		int size = 128;
		UCHAR file[128] = {0};	
		app_desc_t* pdesc = NULL;
		UCHAR *p = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( FileId, &file[0], &size );
		
		pdesc = (app_desc_t*)file;

		p = (UCHAR*)&pdesc->file_len;
		
		pdesc->file_len = MAKELONG( MAKEWORD(*p,*(p+1)), MAKEWORD(*(p+2), *(p+3)) );
		
		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		if((pdesc->file_len !=  0))
			vty_out( vty, "\r\n\t%-16s\t%-10u\t%s\r\n", pdesc->dev_type, pdesc->file_len, pdesc->file_ver );
				
	}
	else if( VOS_StrCmp( argv[0], "dba" ) == 0 ){
		int size = 2048, i=0;
		UCHAR file[2048]="";
		app_desc_t *pdesc = NULL;

		VOS_MemSet(file, 0, size );
		xflash_file_read( FileId, file, &size );

		pdesc = (app_desc_t*)file;


		vty_out( vty, "\r\n\t%-16s\t%-10s\t%s\r\n", "type", "length", "version" );

		for( i=0; i<(2048/128); i++ )
		{
			char *p = (char*)&pdesc->file_len;
			/*
			if( pdesc->dev_type[0] == 0 )
				break;
			*/
			pdesc->file_len = MAKELONG( MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) );
			if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_PON_DBA_KEY_ID), &pdesc->dev_type[0], VOS_StrLen(typesdb_product_file_key_name(PRODUCT_PON_DBA_KEY_ID))) == 0)
			&& (pdesc->file_len !=  0))
			vty_out( vty, "\r\n%d\t%-16s\t%-10u\t%s", i+1, pdesc->dev_type, pdesc->file_len, pdesc->file_ver );
			pdesc++;
			
		}
		vty_out(vty, "\r\n" );
	}
	else
		return CMD_WARNING;
	
	return CMD_SUCCESS;	
}

VOID show_onu_version_banner(struct vty * vty)
{
	int i;
	vty_out(vty, "%-8s%-16s%-8s HW-version  SW-version     userName\r\n", "Idx", "type", "range" );
	vty_out(vty,"");
	for( i=0; i<70; i++ ) vty_out(vty, "-");
	vty_out(vty, "\r\n");
}
/*  added by chenfj 2007/04/26
     问题单#4298 : 在config节点下添加命令show onu-version <slot/port> <onu-list>，没有<slot/port>参数时，显示所有ONU版本。没有ONU参数时，显示当前PON端口下所有ONU版本
	*/
/* modified by chenfj 2008-2-27 #6349
     在显示ONU 版本信息时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
DEFUN (
    onu_version_show,
    onu_version_show_cmd,
    "show onu-version {[type] <typestring>}*1 {[pon] <slotId/port>}*1",
    DescStringCommonShow
    "Show onu version\n"
    "onu type\n"
    /*the specific onu type-string,e.g DEVICE_TYPE_NAME_GT816_STR","DEVICE_TYPE_NAME_GT831A_CATV_STR" etc\n"*/
    "the specific onu type-string\n"
    "Please input pon slot/port\n"
    "Please input onuId\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	ULONG ulIfIndex = 0;
	INT16 phyPonId = 0;	
	char  OnuFlag = 0;
	unsigned int DeviceTypeLen = 0;
	/*int i;*/
	unsigned char onuTypeString[ONU_TYPE_LEN+2];
	unsigned char OptionFlag = 0;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;

	onuTypeString[0]='\0';
	if( argc == 2 ) 
		{
		if(VOS_MemCmp(argv[0], "pon", 3) == 0 )
			{
			VOS_Sscanf(argv[1], "%d/%d", &ulSlot, &ulPort);
			if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
				return(CMD_WARNING);
			OptionFlag = 1;
			}
		else if(VOS_MemCmp(argv[0],"type", 4) == 0 )
			{
			DeviceTypeLen = VOS_StrLen(argv[1]);
			if(DeviceTypeLen > ONU_TYPE_LEN ) 
				{
				DeviceTypeLen = ONU_TYPE_LEN;
				argv[1][DeviceTypeLen] = '\0';
				}
			VOS_StrCpy(onuTypeString, argv[1]);
			OptionFlag = 2;
			}
		}
	else if( argc == 4 )
		{
		if(VOS_MemCmp(argv[0],"type", 4) == 0 )
			{
			DeviceTypeLen = VOS_StrLen(argv[1]);
			if(DeviceTypeLen > ONU_TYPE_LEN ) 
				{
				DeviceTypeLen = ONU_TYPE_LEN;
				argv[1][DeviceTypeLen] = '\0';
				}
			VOS_StrCpy(onuTypeString, argv[1]);
			OptionFlag = 2;
			}
		if(VOS_MemCmp(argv[2], "pon", 3) == 0 )
			{
			VOS_Sscanf(argv[3], "%d/%d", &ulSlot, &ulPort);
			if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
				return(CMD_WARNING);
			OptionFlag += 1;
			}
		}	
	/*
	if( onuTypeString != NULL)
		{
		for(i=0; i< DeviceTypeLen; i++)
			onuTypeString[i] = VOS_ToUpper(onuTypeString[i]);
		}
	*/
	/*DeviceTypeLen = GetDeviceTypeLength();*/
	
	/* modified by chenfj 2009-6-3
		增加显示信息，用于提示ONU 类型*/
	if((OptionFlag >= 2 ) && (SearchOnuType(onuTypeString) == RERROR))
		{
		NotificationOnuTypeString(vty);
		return(CMD_WARNING);
		}
	
	/* 命令中没有参数, 或者只有一个参数, 为ONU 类型*/
	if(( argc == 0 ) || (( argc == 2 ) && (OptionFlag == 2 )))
	{
		for(ulSlot=PONCARD_FIRST; ulSlot<=PONCARD_LAST; ulSlot++)
		{
			if(PonCardSlotRangeCheckByVty(ulSlot, NULL) != ROK ) continue;
			
			for( ulPort = 1; ulPort <= PONPORTPERCARD; ulPort++ )
			{
				phyPonId = GetPonPortIdxBySlot(ulSlot, ulPort);
				if( phyPonId == RERROR ) continue;
				
				if( AllOnuCounter( phyPonId ) <= 0 ) continue;
				if( OnuFlag == 0 )
				{
					/*vty_out(vty, "\r\nonu version info\r\n");
					vty_out(vty, "Idx  type    ");
					for(i=0;i<(DeviceTypeLen-5);i++)
						vty_out(vty," ");
					vty_out(vty,"range    HW-version  SW-version     userName\r\n\r\n");
					vty_out(vty, "----------------------------------------------------------------------\r\n");*/
					show_onu_version_banner(vty);
					OnuFlag = 1;
				}
				if( SearchOnuByType(phyPonId,onuTypeString) == ROK )
				{
					/*vty_out(vty, "pon %d/%d:\r\n", ulSlot, ulPort );*/
					DeviceTypeLen = 0;
					for(ulOnuId=0; ulOnuId<MAXONUPERPON; ulOnuId++)
					{
						if( ThisIsValidOnu(phyPonId, ulOnuId) == ROK )
						{
							ShowOnuVersionInfoByVty_1( phyPonId, ulOnuId, (OptionFlag>=2)?onuTypeString:NULL, vty );
							DeviceTypeLen++;
						}
					}
					if( DeviceTypeLen )
						vty_out(vty, "\r\n");
				}
			}
		}
		if( OnuFlag == 0 )
			{
			vty_out( vty, "No onu exist\r\n");
			}
		vty_out(vty, "\r\n");
		return CMD_SUCCESS;
	}

	/* 命令中有两个或四个参数, 都指定了PON端口*/
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;
	}
	vty_out( vty, "\r\n" );
	
	{
		unsigned short  OnuIdx;
		if( AllOnuCounter( phyPonId ) <= 0 ) 
			{
			vty_out(vty, " No onu exist in this pon\r\n");
			return( CMD_SUCCESS );
			}
		/*vty_out(vty, "%s/port%d onu version info:\r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId));
		vty_out(vty, "Idx  type    ");
		for(i=0;i<(DeviceTypeLen-5);i++)
			vty_out(vty," ");
		vty_out(vty,"range    HW-version  SW-version     userName\r\n\r\n");
		vty_out(vty, "----------------------------------------------------------------------\r\n");*/
		show_onu_version_banner(vty);

		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			if( ThisIsValidOnu(phyPonId, OnuIdx) == ROK )
				ShowOnuVersionInfoByVty_1( phyPonId, OnuIdx, (OptionFlag>=2)?onuTypeString:NULL, vty );
		}
	}
	vty_out(vty, "\r\n");
	return CMD_SUCCESS;
}

DEFUN (
    pononu_version_show,
    pononu_version_show_cmd,
    "show onu-version pon <slotId/port> <onuid_list>",
    DescStringCommonShow
    "Show onu version\n"
    "please input the pon port\n"
    "Please input pon slot/port\n"
    OnuIDStringDesc
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	ULONG ulIfIndex = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;
	int count = 0;


	ulIfIndex = ( ULONG ) ( vty->index ) ;
	
	VOS_Sscanf(argv[0], "%d/%d", &ulSlot, &ulPort);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return(CMD_WARNING);
	
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;
	}
	vty_out( vty, "\r\n" );

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], ulOnuId )
	{
		count ++;
		#if 0
		userOnuId = (INT16)(ulOnuId - 1);
		ShowOnuVersionInfoByVty( phyPonId, userOnuId, vty );
		#endif
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	
	if (count == 1)
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], ulOnuId )
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
    		vty_out(vty, " No onu exist\r\n\r\n");
    		return( CMD_SUCCESS );
		}
	/*DeviceTypeLen = GetDeviceTypeLength();
	vty_out(vty, "%s/port%d onu version info:\r\n", CardSlot_s[GetCardIdxByPonChip(phyPonId)], GetPonPortByPonChip(phyPonId));
	vty_out(vty, "Idx  type    ");
	for(i=0; i<(DeviceTypeLen - 5);i++)
		vty_out(vty," ");
	vty_out(vty,"range    HW-version  SW-version    userName\r\n\r\n");
	vty_out(vty, "----------------------------------------------------------------------\r\n");*/
		show_onu_version_banner(vty);

		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	 	{
	 		userOnuId = (INT16)(ulOnuId - 1);
	 		ShowOnuVersionInfoByVty_1( phyPonId, userOnuId, NULL, vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}
	
	vty_out(vty, "\r\n");
	return CMD_SUCCESS;
}

/*extern void set_board_i2cinfo( UCHAR slot_id, USHORT boardtype, UCHAR *boardsn, UCHAR *boardver, UCHAR *boarddate );
extern void set_epon_i2cinfo( UCHAR slot_id, UCHAR pon_inserteds, UCHAR pon_type, UCHAR ctc_support, UCHAR ext_buf );*/
extern ULONG device_soft_modtype_2_hw_modtype( ULONG board_type );
extern int get_i2c_board_info( UCHAR slotno, USHORT *p_boardtype, UCHAR *p_boardsn, UCHAR *p_boardver, UCHAR *p_boarddate );
extern int set_i2c_board_info( UCHAR slotno, USHORT boardtype, UCHAR *p_boardsn, UCHAR *p_boardver, UCHAR *p_boarddate );
extern int updateSlotEepromInfo( ULONG slotno );	/* added by xieshl 20080307 */

/*modified by shixh@20080124*/
/*"Input board type(e.g.: gfa-sw,gfa-epon,gfa-get,gfa-geo,gfa-gem,gfa-sig,gfa-pwu48,gfa-pwu220,...\n"*/

/* modified by xieshl 20111129, 问题单14032 */
LONG check_eeprom_access_limit( struct vty *vty, ULONG slotno )
{
	if( SYS_SLOTNO_IS_ILLEGAL(slotno) )
	{
		vty_out( vty, "  Slot number error\r\n");
		return VOS_ERROR;
	}
	/* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY || SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
	{
		if( slotno != SYS_LOCAL_MODULE_SLOTNO )
		{
			vty_out( vty, "  This board is standby, only config itself\r\n" );
			return VOS_ERROR;
		}
	}
	else
	{
	       /* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
	       if ( (slotno != SYS_LOCAL_MODULE_SLOTNO) && SYS_MODULE_ISMASTERSTANDBY(slotno) )
	       {
	                vty_out( vty, "  This board is standby, only support config itself locally\r\n" );
	                return VOS_ERROR;
	      }
	}
	return VOS_OK;
}

DEFUN (
    set_board_eeprom,
    set_board_eeprom_cmd,
    "board-eeprom <1-21> {[type] <type>}*1 {[sn] <sn>}*1 {[version] <ver>}*1 {[date] <date>}*1",
    "Config board EEPROM info.\n"
    "Specify slot no.\n"
    "Specify board type\n"
    /*Input board type,e.g.:BOARD_TYPE_GFA6700_SW_STR","BOARD_TYPE_GFA6700_EPON_STR","BOARD_TYPE_GFA6700_GET_STR","BOARD_TYPE_GFA6700_GEO_STR","BOARD_TYPE_GFA6700_GEM_STR","BOARD_TYPE_GFA6700_SIG_STR","BOARD_TYPE_GFA6700_E1_STR","BOARD_TYPE_GFA6700_PWU48_STR","BOARD_TYPE_GFA6700_PWU220_STR",...\n"*/
    "Input board type\n" 
    "Specify board serial number\n"
    "Input serial number\n"
    "Specify board hardware version\n"
    "Input version\n"
    "Specify manufacture date\n"
    "Input board manufacture date\n"
    )
{
	UCHAR slotno;
	USHORT boardtype;
	UCHAR p_boardsn[17],p_boardver[17],p_boarddate[11];
	int i;
	/*UCHAR *p_boardsn, *p_boardver, *p_boarddate;*/
      int iProductType = GetOltType();
    
	slotno = VOS_AtoI( argv[0] );
#if 0
	if( SYS_SLOTNO_IS_ILLEGAL(slotno) )
	{
		vty_out( vty, "  Slot number error\r\n");
		return CMD_WARNING;
	}
	/*vty_out( vty, "slot number is:%d\r\n", slotno);*/
	/* added by xieshl 20071108, 如果在备SW板上执行，只能设置本板信息 */
	/* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY || SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
	{
		if( slotno != SYS_LOCAL_MODULE_SLOTNO )
		{
			vty_out( vty, "  This board is standby, only config itself\r\n" );
			return CMD_WARNING;
		}
	}
	else
	{
	       /* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
	       if ( (slotno != SYS_LOCAL_MODULE_SLOTNO) && SYS_MODULE_ISMASTERSTANDBY(slotno) )
	       {
	                vty_out( vty, "  This board is standby, only support config itself locally\r\n" );
	                return CMD_WARNING;
	      }
	}
#else
	if( check_eeprom_access_limit(vty, slotno) == VOS_ERROR )
		return CMD_WARNING;
#endif

	get_i2c_board_info( slotno, &boardtype, p_boardsn, p_boardver, p_boarddate );
/* vty_out( vty, "first slotno:%d,boardtype:%d,p_boardsn:%s,p_boardver:%s,p_boarddate:%s\r\n", slotno,boardtype,p_boardsn,p_boardver,p_boarddate);*/	
	for(i=1;i<argc;i++)
	{
		if( VOS_StriCmp( argv[i], "type") == 0 )
		{
			/* modified by xieshl 20111118,  不支持6100了，6100/6700一样，这里没必要区分产品类型, 问题单13924 */
			/*if ( V2R1_OLT_GFA6700 == iProductType )
			{*/
				if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_SW_STR) == 0 )
					boardtype = MODULE_E_GFA_SW;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_EPON_STR) == 0 )
					boardtype = MODULE_E_GFA_EPON;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_GET_STR) == 0 )
					boardtype = MODULE_E_GFA_GET;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_GEO_STR) == 0 )
					boardtype = MODULE_E_GFA_GEO;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA_PWU220;
				else if( (VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_SIG_STR) == 0) ||
						( VOS_StriCmp(argv[i+1], "gfa-tdm") == 0) )
					boardtype = MODULE_E_GFA_SIG;
				else if( (VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_E1_STR) == 0) ||
						( VOS_StriCmp(argv[i+1], "gfa-e1") == 0) )
					boardtype = MODULE_E_GFA_E1;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_GEM_STR) == 0 )
					boardtype = MODULE_E_GFA_GEM;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6700_SWTG_STR) == 0 )
					boardtype = MODULE_E_GFA_SWTG;
	    			/*else
	    			{
	    				vty_out( vty, "  board type error, board type should be %s, %s, %s, %s, %s, %s, %s, %s, %s etc\r\n", BOARD_TYPE_GFA6700_SW_STR,BOARD_TYPE_GFA6700_EPON_STR, BOARD_TYPE_GFA6700_GET_STR, BOARD_TYPE_GFA6700_GEO_STR, BOARD_TYPE_GFA6700_GEM_STR, BOARD_TYPE_GFA6700_SIG_STR, BOARD_TYPE_GFA6700_E1_STR, BOARD_TYPE_GFA6700_PWU48_STR, BOARD_TYPE_GFA6700_PWU220_STR);
	    				return CMD_WARNING;
	    			}	
			}      
			else if ( V2R1_OLT_GFA6900 == iProductType )
			{*/
#if 1/*GFA6900*/
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_SW_STR) == 0 )
					boardtype = MODULE_E_GFA6900_SW;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_4EPON_STR) == 0 )
					boardtype = MODULE_E_GFA6900_4EPON;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_8EPON_STR) == 0 )
					boardtype = MODULE_E_GFA6900_8EPON;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_12EPON_STR) == 0 )
					boardtype = MODULE_E_GFA6900_12EPON;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_4EPON_4GE_STR) == 0 )
					boardtype = MODULE_E_GFA6900_4EPON_4GE;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_12EPONB0_STR) == 0 )
					boardtype = MODULE_E_GFA6900_12EPONB0;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_16EPONB1_STR) == 0 )
					boardtype = MODULE_E_GFA6900_16EPONB1;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_GEM_STR) == 0 )
					boardtype = MODULE_E_GFA6900_GEM_GE;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_10GEM_STR) == 0 )
					boardtype = MODULE_E_GFA6900_GEM_10GE;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_FAN_STR) == 0 )
					boardtype = MODULE_E_GFA6900_FAN;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA6900_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA6900_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900M_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA6900M_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900S_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA6900S_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900S_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA6900S_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA6900S_FAN_STR) == 0 )
					boardtype = MODULE_E_GFA6900S_FAN;
#endif
#if 1/*GFA8000*/
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_SW_STR) == 0 )
					boardtype = MODULE_E_GFA8000_SW;
/*				
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_4EPON_STR) == 0 )
					boardtype = MODULE_E_GFA8000_4EPON;
*/
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_4EPON_4GE_STR) == 0 )
					boardtype = MODULE_E_GFA8000_4EPON_4GE;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_10G_8EPON_STR) == 0 )
					boardtype = MODULE_E_GFA8000_10G_8EPON;
/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/	
/*				
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_12EPON_STR) == 0 )
					boardtype = MODULE_E_GFA8000_12EPON;
*/				
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_12EPONB0_STR) == 0 )
					boardtype = MODULE_E_GFA8000_12EPONB0;

/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
/*
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_16EPONB1_STR) == 0 )
					boardtype = MODULE_E_GFA8000_16EPONB1;

				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_GEM_STR) == 0 )
					boardtype = MODULE_E_GFA8000_GEM_GE;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_10GEM_STR) == 0 )
					boardtype = MODULE_E_GFA8000_GEM_10GE;
*/

				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_FAN_STR) == 0 )
					boardtype = MODULE_E_GFA8000_FAN;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA8000_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA8000_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000M_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA8000M_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000S_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA8000S_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000S_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA8000S_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8000S_FAN_STR) == 0 )
					boardtype = MODULE_E_GFA8000S_FAN;
#endif

#if 1 /*GFA8100*/

				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8100_16EPONB0_STR) == 0 )
					boardtype = MODULE_E_GFA8100_16EPONB0;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8100_PWU48_STR) == 0 )
					boardtype = MODULE_E_GFA8100_PWU48;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8100_PWU220_STR) == 0 )
					boardtype = MODULE_E_GFA8100_PWU220;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8100_FAN_STR) == 0 )
					boardtype = MODULE_E_GFA8100_FAN;
				else if( VOS_StriCmp(argv[i+1], BOARD_TYPE_GFA8100_16GPONB0_STR) == 0 )
					boardtype = MODULE_E_GFA8100_16GPONB0;
#endif
				else
				{
					if( V2R1_OLT_GFA6900 == iProductType )
					{
						vty_out( vty, "  board type error, board type should be %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s etc\r\n", 
							BOARD_TYPE_GFA6900_SW_STR, BOARD_TYPE_GFA6900_4EPON_STR, BOARD_TYPE_GFA6900_4EPON_4GE_STR, BOARD_TYPE_GFA6900_8EPON_STR,
							BOARD_TYPE_GFA6900_12EPON_STR, BOARD_TYPE_GFA6900_12EPONB0_STR, BOARD_TYPE_GFA6900_16EPONB1_STR,
							BOARD_TYPE_GFA6900_GEM_STR, BOARD_TYPE_GFA6900_10GEM_STR, BOARD_TYPE_GFA6900_FAN_STR, BOARD_TYPE_GFA6900_PWU48_STR, 
							BOARD_TYPE_GFA6900_PWU220_STR);
					}
					else if( V2R1_OLT_GFA8000 == iProductType )
					{
						vty_out( vty, "  board type error, board type should be %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s etc\r\n", 
							BOARD_TYPE_GFA8000_SW_STR, BOARD_TYPE_GFA8000_4EPON_STR, BOARD_TYPE_GFA8000_4EPON_4GE_STR, BOARD_TYPE_GFA8000_10G_8EPON_STR,
							BOARD_TYPE_GFA8000_12EPON_STR, BOARD_TYPE_GFA8000_12EPONB0_STR, BOARD_TYPE_GFA6900_16EPONB1_STR, /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
							BOARD_TYPE_GFA8000_GEM_STR, BOARD_TYPE_GFA8000_10GEM_STR, BOARD_TYPE_GFA8000_FAN_STR, BOARD_TYPE_GFA8000_PWU48_STR, 
							BOARD_TYPE_GFA8000_PWU220_STR);
					}
					else if( V2R1_OLT_GFA8100 == iProductType )
					{
						vty_out( vty, "  board type error, board type should be %s, %s, %s, %s etc\r\n", 
							BOARD_TYPE_GFA8100_16EPONB0_STR, BOARD_TYPE_GFA8100_FAN_STR, BOARD_TYPE_GFA8100_PWU48_STR, BOARD_TYPE_GFA8100_PWU220_STR);
					}
					else
					{
						vty_out( vty, "  board type error, board type should be %s, %s, %s, %s, %s, %s, %s, %s, %s etc\r\n", 
							BOARD_TYPE_GFA6700_SW_STR,BOARD_TYPE_GFA6700_EPON_STR, BOARD_TYPE_GFA6700_GET_STR, BOARD_TYPE_GFA6700_GEO_STR, 
							BOARD_TYPE_GFA6700_GEM_STR, BOARD_TYPE_GFA6700_SIG_STR, BOARD_TYPE_GFA6700_E1_STR, BOARD_TYPE_GFA6700_PWU48_STR, 
							BOARD_TYPE_GFA6700_PWU220_STR);
					}
					return CMD_WARNING;
				}	
			/*}
			else
			{
				vty_out( vty, "  product type %d is error\r\n", iProductType);
				return CMD_WARNING;
			}*/
		}
		else if( VOS_StriCmp(argv[i], "sn") == 0 )
		{
			if(VOS_StrLen(argv[i+1])<=16)
				VOS_StrCpy(p_boardsn,argv[i+1]);
			else
			{
            			vty_out(vty, "Serial number is too long, chopped to 16 bytes\r\n");	
				VOS_StrnCpy(p_boardsn, argv[i+1], 16);
			}
		}
		else if( VOS_StriCmp(argv[i], "version") == 0 )
		{
			if(VOS_StrLen(argv[i+1])<=16)
				VOS_StrCpy(p_boardver,argv[i+1]);
			else
			{
            			vty_out(vty, "Version string is too long, chopped to 16 bytes\r\n");	
				VOS_StrnCpy(p_boardver, argv[i+1], 16);
			}
		}
		else if( VOS_StriCmp(argv[i], "date") == 0 )
		{
			if(VOS_StrLen(argv[i+1])<=10)
				VOS_StrCpy(p_boarddate,argv[i+1]);
			else
			{
            			vty_out(vty, "Date string is too long, chopped to 10 bytes\r\n");	
				VOS_StrnCpy(p_boarddate, argv[i+1], 10);
			}
		}
		else
		{
            		vty_out(vty, "Parameter error!!\r\n");	
		}
		i++;           
	}

	/*vty_out( vty, "second slotno:%d ,boardtype:%d,p_boardsn:%s,p_boardver:%s,p_boarddate:%d\r\n", slotno,boardtype,p_boardsn,p_boardver,p_boarddate);*/

    /* B--modified by liwei056@2010-12-10 for Dead-BUG */
	if( NULL != CDSMS_CHASSIS_SLOT_INSERTED_FUN )
    {
    	if( CDSMS_CHASSIS_SLOT_INSERTED(slotno) != TRUE )
		{
    		vty_out(vty,"  slot %d is empty\r\n", slotno);
    		return CMD_WARNING;
		}
    }
    else
    {
        if ( slotno != SYS_LOCAL_MODULE_SLOTNO )
		{
    		vty_out(vty,"  slot %d is empty\r\n", slotno);
    		return CMD_WARNING;
		}
    }
    /* E--modified by liwei056@2010-12-10 for Dead-BUG */
 
	if( set_i2c_board_info(slotno, boardtype, p_boardsn, p_boardver, p_boarddate) == TRUE )
		vty_out( vty, " set success!\r\n" );
	else
		vty_out( vty, " board-eeprom error!\r\n" );
	
	return CMD_SUCCESS;
}


/* modified by xieshl 20091217, 修改命令格式，便于扩展 */
DEFUN (
    set_pon_ext_eeprom,
    set_pon_ext_eeprom_cmd,
    "pon-eeprom-ext <portlist> <0-15> <0-1> <0-1> <1-9>",
    "Config pon card eeprom Ext info.\n"
    "Specify pon's port list(e.g. 3/2 or 3/1-2)\n"
    "PON chip type:0-NoChip,1-PAS5001,2-PAS5201,3-PAS5204,4-PAS8411;7-TK3723,8-BCM55524,9-BCM55538 10-BCM68622\n"
    "CTC protocol:0-Not support,1-Support\n"
    "Support extend buffer:0-Not support,1-16M\n"
    "SFP type:1-Fibrexon,2-Hisense,3-PHOTON,4-WTD,5-GWD,6-Delta,7-Other\n"
    )
{
    int chip_type;
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulIfIndex = 0;
	USHORT pon_ins[16];
	USHORT pon_ins_count[16];
	
	UCHAR pon_inserted = 1;
	UCHAR pon_type = 1;
	UCHAR ctc_support = 0;
	UCHAR ext_buf = 0;
	UCHAR sfpVendor = 2;


	VOS_MemZero( pon_ins, sizeof(pon_ins) );
	VOS_MemZero( pon_ins_count, sizeof(pon_ins_count) );
	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		pon_ins[ulSlot] |= (1<<(16 - ulPort));
		pon_ins_count[ulSlot]++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

    chip_type = VOS_AtoI(argv[1]);
	if( 0 == chip_type )
	{
		pon_inserted = 0;
		pon_type = 0;
		ctc_support = 0;
		ext_buf = 0;
	}
	else
	{
        if ( chip_type > 0 && chip_type < 16 )
        {
            pon_type = (UCHAR)chip_type;
        }
        else
        {
        	return CMD_WARNING;
        }

		if( argv[2][0] == '1' )
			ctc_support = 1;
		if( argv[3][0] == '1' )
			ext_buf = 1;
		sfpVendor = (UCHAR ) VOS_AtoL(argv[4]);
	}
	/*for( ulSlot=4; ulSlot<=8; ulSlot++ ) */
	for( ulSlot= PONCARD_FIRST; ulSlot<= PONCARD_LAST; ulSlot++)
		{
		/*if( PonCardSlotRangeCheckByVty(ulSlot,0) != ROK ) continue;*/
		
		if( pon_ins_count[ulSlot] != 0 )
		{
			/*
			if( (__SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON) && (__SYS_MODULE_TYPE__(ulSlot) != MODULE_TYPE_UNKNOW) )
			{
				vty_out( vty, "  Slot %d is not %s board\r\n", ulSlot, BOARD_TYPE_GFA6700_EPON_STR );
				continue;
			}
			*/

            /* B--modified by liwei056@2010-12-10 for Dead-BUG */
        	if( NULL != CDSMS_CHASSIS_SLOT_INSERTED_FUN )
            {
            	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) != TRUE )
        		{
            		vty_out(vty,"  slot %d is empty\r\n", ulSlot);
    				continue;
        		}
            }
            else
            {
                if ( ulSlot != SYS_LOCAL_MODULE_SLOTNO )
        		{
            		vty_out(vty,"  slot %d is empty\r\n", ulSlot);
    				continue;
        		}
            }
            /* E--modified by liwei056@2010-12-10 for Dead-BUG */

			if(SlotCardMayBePonBoardByVty(ulSlot,vty) != ROK )
				continue;
			/*set_epon_i2cinfo( LOGICAL_2_PHYSICAL_SLOT(ulSlot), pon_ins[ulSlot], pon_type, ctc_support, ext_buf );*/
			for( ulPort=0; ulPort<PONPORTPERCARD; ulPort++ )
			{
				if( pon_ins[ulSlot] & (1<<(15 - ulPort)) )
				{
					set_i2c_pon_ext_info( ulSlot, ulPort+1, pon_inserted, pon_type, ctc_support, ext_buf, sfpVendor);
				}
			}
			vty_out( vty, "  Slot %d: PON-list=%x; Type-%s; CTC-%s; Ext_Buf-%s; %s\r\n", ulSlot, pon_ins[ulSlot],
						pon_chip_type2name(pon_type),
						pon_ctc_support2name(ctc_support),
						pon_ext_buffer2name(ext_buf),
						pon_sfpVendor2name(sfpVendor) );
		}
	}

	return CMD_SUCCESS;
}

DEFUN (
    undo_pon_ext_eeprom,
    undo_pon_ext_eeprom_cmd,
    "undo pon-eeprom-ext <portlist>",
    NO_STR
   /* "Delete "BOARD_TYPE_GFA6700_EPON_STR" EEPROM ext. info.\n"*/
    "Delete pon board EEPROM ext. info.\n"
    "Specify pon's port list(e.g.: 5/1; 5/3, 6/1-4)\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	USHORT pon_ins[16];
	USHORT pon_ins_count[16];
	
	ULONG ulIfIndex = 0;

	VOS_MemZero( pon_ins, sizeof(pon_ins) );
	VOS_MemZero( pon_ins_count, sizeof(pon_ins_count) );
	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		pon_ins[ulSlot] |= (1<<(16 - ulPort));
		pon_ins_count[ulSlot]++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	/*for( ulSlot=4; ulSlot<=8; ulSlot++ )*/
	for( ulSlot= PONCARD_FIRST; ulSlot<= PONCARD_LAST; ulSlot++)
		{
		if( PonCardSlotRangeCheckByVty(ulSlot, NULL) != ROK ) continue;
	
		if( pon_ins_count[ulSlot] != 0 )
		{
            /* B--modified by liwei056@2010-12-10 for Dead-BUG */
        	if( NULL != CDSMS_CHASSIS_SLOT_INSERTED_FUN )
            {
            	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) != TRUE )
        		{
            		vty_out(vty,"  slot %d is empty\r\n", ulSlot);
    				continue;
        		}
            }
            else
            {
                if ( ulSlot != SYS_LOCAL_MODULE_SLOTNO )
        		{
            		vty_out(vty,"  slot %d is empty\r\n", ulSlot);
    				continue;
        		}
            }
            /* E--modified by liwei056@2010-12-10 for Dead-BUG */

			if(SlotCardMayBePonBoardByVty(ulSlot,vty) != ROK )
				continue;
			/*set_epon_i2cinfo( LOGICAL_2_PHYSICAL_SLOT(ulSlot), pon_ins[ulSlot], pon_type, ctc_support, ext_buf );*/
			for( ulPort=0; ulPort<PONPORTPERCARD; ulPort++ )
			{
				if( pon_ins[ulSlot] & (1<<(15 - ulPort)) )
				{
					set_i2c_pon_ext_info( ulSlot, ulPort+1, 0, 0, 0, 0, 0 );
				}
			}
		}
	}

	return CMD_SUCCESS;
}

DEFUN(set_chassis_board,
		set_chassis_board_cmd,
		"chassis-eeprom <version> <date> <sn>",
		"set chassis e2prom\n"
		"set hardware ver\n"
		"set product date\n"
		"set serial no\n")
{
	int slotno;
	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	    return CMD_SUCCESS;
	if(!(argv[0]&&argv[1]&&argv[2]))
	{
		vty_out(vty,"\r\n  %% Para error");
	    return CMD_SUCCESS;	
	}
	if(SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900 || SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900M||
	   SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900S ||(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000))
	{
		slotno = 22;
	}
	else
	{
		vty_out(vty,"  \r\n%% The command is only for 6900");
		return CMD_SUCCESS;
	}
	if( VOS_YES != i2c_data_set(slotno,I2C_BASE_E2PROM,I2C_REG_MODULE_VER,argv[0],I2C_REG_MODULE_VER_LEN))
	{
		vty_out(vty,"\r\n  %% Set product ver failed");
	}
	if( VOS_YES != i2c_data_set(slotno,I2C_BASE_E2PROM,I2C_REG_MODULE_DATE,argv[1],I2C_REG_MODULE_DATE_LEN))
	{
		vty_out(vty,"\r\n  %% Set card date failed");
	}
	if( VOS_YES != i2c_data_set(slotno,I2C_BASE_E2PROM,I2C_REG_MODULE_SN,argv[2],I2C_REG_MODULE_SN_LEN))
	{
		vty_out(vty,"\r\n  %% Set card sn failed");
	}
	
	return CMD_SUCCESS;
}
DEFUN (
    show_board_eeprom,
    show_board_eeprom_cmd,
    "show board-eeprom <1-21>",
    "show eeprom info.\n"
    "show board and pon eeprom info.\n"
    "Specify slot no.\n"
    )
{
	UCHAR slotno;
	USHORT boardtype = 0;
	UCHAR p_boardsn[20], p_boardver[20], p_boarddate[20];/* modified by xieshl 20070904, 动态申请内存输出时会打印多余的乱码信息 */

	VOS_MemZero(p_boardsn, sizeof(p_boardsn));
	VOS_MemZero(p_boardver, sizeof(p_boardver));
	VOS_MemZero(p_boarddate, sizeof(p_boarddate));
	
	slotno = VOS_AtoI( argv[0] );
    /* B--modified by liwei056@2010-12-10 for Dead-BUG */
	if( NULL != CDSMS_CHASSIS_SLOT_INSERTED_FUN )
    {
    	if( CDSMS_CHASSIS_SLOT_INSERTED(slotno) != TRUE )
		{
    		vty_out(vty,"  slot %d is empty\r\n", slotno);
    		return CMD_WARNING;
		}
    }
    else
    {
        if ( slotno != SYS_LOCAL_MODULE_SLOTNO )
		{
    		vty_out(vty,"  slot %d is empty\r\n", slotno);
    		return CMD_WARNING;
		}
    }
    /* E--modified by liwei056@2010-12-10 for Dead-BUG */
    
#if 0
	if( SYS_SLOTNO_IS_ILLEGAL(slotno) ||
		(__SYS_MODULE_TYPE__(slotno) >= MODULE_TYPE_MAX) ||
		(__SYS_MODULE_TYPE__(slotno) <= MODULE_TYPE_UNKNOW) )
	{
		vty_out( vty, "  Slot number error\r\n" );
		return CMD_WARNING;
	}

	/* added by xieshl 20071108, 如果在备SW板上执行，只能查看本板信息 */
	/* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY || SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
	{
		if( slotno != SYS_LOCAL_MODULE_SLOTNO )
		{
			vty_out( vty, "  This board is standby or slave, only show itself\r\n" );
			return CMD_WARNING;
		}
	}
	else
	{
		/* added by zhangxinhui 20101227,支持从主控查看和设置PON板EEPROM */
		/* if ( (slotno != SYS_LOCAL_MODULE_SLOTNO) && SYS_MODULE_SLOT_ISHAVECPU(slotno) )
		{
			vty_out( vty, "  The specified slot has its own CPU, so please swith to that card and try.\r\n");
			return CMD_WARNING;
		} */
	}
#else
	if( check_eeprom_access_limit(vty, slotno) == VOS_ERROR )
		return CMD_WARNING;
#endif

	/*updateSlotEepromInfo( slotno );*/	/* added by xieshl 20080307, 仅用于制造 */
	if( get_i2c_board_info(slotno, &boardtype, p_boardsn, p_boardver, p_boarddate) )
	{
		vty_out( vty, "slot%d: %s\r\n", slotno, typesdb_module_type2name(boardtype) );
		vty_out( vty, "  sn: %s\r\n", p_boardsn );
		vty_out( vty, "  ver: %s\r\n", p_boardver );
		vty_out( vty, "  date: %s\r\n", p_boarddate );
		/*if( SYS_LOCAL_MODULE_SLOTNO == slotno )*/
		if( SYS_MODULE_SLOT_ISHAVEPP(slotno) && (SYS_LOCAL_MODULE_SLOTNO == slotno) )	/* modified by xieshl 20121119, 参见I2C_REG_SYS_MAC定义，sysmac偏移有误，问题单13924 */
		{
			extern ULONG g_PhysicalSlot[];
			char mac[6];
			UINT sys_mac_reg;

			sys_mac_reg = I2C_REG_SYS_MAC;
				
			if( i2c_data_get(g_PhysicalSlot[slotno], I2C_BASE_E2PROM, sys_mac_reg, mac, I2C_REG_SYS_MAC_LEN) == TRUE)	/* 单板MAC地址*/
			{
				vty_out( vty, "  mac: %02X%02X.%02X%02X.%02X%02X\r\n", 
					mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			}
		}
	}
	else
	{
		vty_out( vty, "  Can't get slot%d EEPROM info.\r\n", slotno );
		return CMD_WARNING;
	}	

	if(SlotCardIsSwBoard(slotno) == ROK )
	/*if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_SW )*/
	{
		UCHAR oui[3];
		UCHAR oui_ver;
		if( getCtcStackOui(slotno, oui, &oui_ver) == VOS_OK )
		{
			vty_out( vty, "  EXT-OAM OUI: %02x%02x%02x, version: %02x\r\n", oui[0], oui[1], oui[2], oui_ver );
		}
	}
	if(SlotCardIsPonBoard(slotno) == ROK )
	/*else if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_EPON )*/
	{
		UCHAR inserted, chip_type, ctc_support, ext_buf, sfpVendor;
		UCHAR ponid;
		UCHAR ponbegin, ponend;
        /* B--added by liwei056@2012-3-23 for D14620 */
        short int sOltPortStart, sOltPortEnd;

        if ( 0 < GetSlotPonPortPhyRange(slotno, &sOltPortStart, &sOltPortEnd) )
        {
            ponbegin = (UCHAR)sOltPortStart;
            ponend = (UCHAR)sOltPortEnd;
        }
        else
        /* E--added by liwei056@2012-3-23 for D14620 */
        {
            ponbegin = 1;
            ponend = PONPORTPERCARD;
        }
        
		for( ponid=ponbegin; ponid<=ponend; ponid++ )
		{
			if( get_i2c_pon_ext_info(slotno, ponid, &inserted, &chip_type, &ctc_support, &ext_buf, &sfpVendor) )
			{
				if( inserted == 1 )
				{
					vty_out( vty, "  PON%d: %s; CTC-%s; Ext_Buf-%s; %s\r\n", ponid,
							pon_chip_type2name(chip_type),
							pon_ctc_support2name(ctc_support),
							pon_ext_buffer2name(ext_buf), 
							pon_sfpVendor2name(sfpVendor) );
				}
				else
				{
					vty_out( vty, "  PON%d: NULL\r\n", ponid );
				}
			}
		}
	}
	else if(SlotCardIsTdmBoard(slotno) == ROK )
	/*else if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_SIG )*/
	{
		/* added by xieshl 20071108 for TDM */
		UCHAR tdmno;
		UCHAR vendor_id, chip_type, e1_impedance;
		for( tdmno=TDM_FPGA_MIN; tdmno<=TDM_FPGA_MAX; tdmno++ )
		{
			if( get_i2c_tdm_ext_info(slotno, tdmno, &chip_type, &vendor_id, &e1_impedance) )
			{
				if( chip_type != 0 )
					vty_out( vty, "  TDM%d: Chipset-%s %s, E1-impedance-%s\r\n", tdmno,
								tdm_chip_type2name(chip_type),
								tdm_vendor_id2name(vendor_id),
								tdm_e1_impedance2name(e1_impedance) );
				else
					vty_out( vty, "  TDM%d: NULL\r\n", tdmno );
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN (
    set_sw_ext_eeprom,
    set_sw_ext_eeprom_cmd,
    "sw-eeprom-ext <oui_hexvalue> <oui_version>",
    /*"Config GFA-SW EEPROM ext. info.\n"*/
    "Config SW EEPROM ext. info.\n"
    "OUI of OAM extension, eg. 111111 or 0x111111\n"
    "OUI version of OAM Extension, eg. 21 or 0x21\n"
    )
{
	UCHAR oui[3] = { 0, 0, 0 };
	UCHAR oui_ver = 0;
	ULONG val = 0;
	ULONG slotno;
	char *pToken;

	slotno = SYS_LOCAL_MODULE_SLOTNO;

	val = VOS_StrToUL( argv[0], &pToken, 16 );
	oui[2] = (UCHAR)(val & 0xff);
	oui[1] = (UCHAR)((val >> 8) & 0xff);
	oui[0] = (UCHAR)((val >> 16) & 0xff);
		
	/* modified by xieshl 20110721 */
	oui_ver = (UCHAR)(VOS_StrToUL(argv[1], &pToken, 16) & 0xff);
	if( setCtcStackOui(slotno, oui, oui_ver) == VOS_OK )
	{
		/*vty_out( vty, "  Slot %d: OUI=%02x%02x%02x; version=%d\r\n", slotno, oui[0], oui[1], oui[2], oui_ver );*/
		vty_out( vty, " The new configuration do not take effect until you restart system\r\n" );
		return CMD_SUCCESS;
	}
	else
	{
		vty_out( vty, "  Slot %d: Can't write EEPROM\r\n", slotno );
	}
	
	return CMD_WARNING;
}

DEFUN (
    undo_sw_ext_eeprom,
    undo_sw_ext_eeprom_cmd,
    "undo sw-eeprom-ext",
    NO_STR
    /*"Delete GFA-SW EEPROM ext. info.\n"*/
    "Delete SW EEPROM ext. info.\n"
    )
{
	UCHAR oui[3] = { 0, 0, 0 };
	UCHAR oui_ver = 0;
	ULONG slotno = SYS_LOCAL_MODULE_SLOTNO;

	/* modified by xieshl 20110721 */
	if( setCtcStackOui(slotno, oui, oui_ver) == VOS_OK )
	{
		/*vty_out( vty, "  Slot %d: OUI erase ok\r\n", slotno );*/
		vty_out( vty, " The new configuration do not take effect until you restart system\r\n" );
		return CMD_SUCCESS;
	}
	else
	{
		vty_out( vty, "  Slot %d: Can't erase EEPROM\r\n", slotno);
	}

	return CMD_WARNING;
}

/* added by xieshl 20110714, 增加OLT可支持的OAM OUI 及其版本号*/
DEFUN(
	show_ctc_oui_ver_rec,
	show_ctc_oui_ver_rec_cmd,
	"show oam-oui-version",
	"show oam oui info.\n"
	"show oam oui and version records list\n"
	)
{
	extern CTC_STACK_oui_version_record_t PAS_oui_version_records_list[MAX_OUI_RECORDS];
	extern int PAS_oui_version_records_num;
	int i;
	CTC_STACK_oui_version_record_t *pList;
	CHAR oui_str[16];
	vty_out( vty, "%-6s%-10s%s\r\n", "No.", "OUI", "Version" );
	for( i=0; i<PAS_oui_version_records_num; i++ )
	{
		pList = &PAS_oui_version_records_list[i];
		VOS_Sprintf( oui_str, "0x%02x%02x%02x", pList->oui[0], pList->oui[1], pList->oui[2] );
		vty_out( vty, "%-5d%-12s0x%02x\r\n", i+1, oui_str, pList->version );
	}
	vty_out( vty, "%-5d%-12s%s\r\n", ++i, "0X000FE9", "0x01" );
	return CMD_SUCCESS;
}

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
/* modified by xieshl 20080529, 增加E1阻抗设置项，问题单#6730 */
DEFUN (
    set_tdm_ext_eeprom,
    set_tdm_ext_eeprom_cmd,
    "tdm-eeprom-ext <portlist> <0-2> <0-2> <0-1>",
    "Config TDM EEPROM ext. info.\n"
    "Specify tdm's port list(e.g.: 5/1 or 5/1-3)\n"
    "TDM chip type:0-null,1-sig,2-8e1\n"
    "TDM chip vendor ID:0-null,1-gw,2-other\n"
    "TDM E1 impedance:0-unbalanced,1-balanced\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	UCHAR tdm_ins[12];
	UCHAR tdm_ins_count[12];
	
	UCHAR tdm_chip_type = 1;	/* sig */
	UCHAR tdm_vendor_id = 1;	/* gw */
	UCHAR tdm_e1_impedance = 0;	/* 75 ou */

	ULONG ulIfIndex = 0;

	VOS_MemZero( tdm_ins, sizeof(tdm_ins) );
	VOS_MemZero( tdm_ins_count, sizeof(tdm_ins_count) );
	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		if(TdmCardSlotPortCheckByVty(ulSlot, ulPort,vty) != ROK )
		{
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
		/*
		if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}
		*/
		if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) != TRUE )
			{
			vty_out(vty,"  slot %d is empty\r\n", ulSlot);
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}		

		/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_SIG )*/
		if(SlotCardIsTdmBoard(ulSlot) != ROK )		
		{
			vty_out( vty, "  %% Parameter error.slot module must be tdm\r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		
		tdm_ins[ulSlot] |= (1<<(8 - ulPort));
		tdm_ins_count[ulSlot]++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	tdm_chip_type = argv[1][0] - '0';
	tdm_vendor_id = argv[2][0] - '0';
	tdm_e1_impedance = argv[3][0] - '0';

	for( ulSlot=TDMCARD_FIRST; ulSlot<=TDMCARD_LAST; ulSlot++ )
	{
		if( tdm_ins_count[ulSlot] != 0 )
		{
			/*if( (__SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_SIG) && (__SYS_MODULE_TYPE__(ulSlot) != MODULE_TYPE_UNKNOW) )*/
			if(SlotCardMayBeTdmBoardByVty(ulSlot, vty) != ROK )
			{
				/*vty_out( vty, "  Slot %d is not %s board\r\n", ulSlot, BOARD_TYPE_GFA6700_SIG_STR );*/
				continue;
			}
			for( ulPort=TDM_FPGA_MIN; ulPort<=TDM_FPGA_MAX; ulPort++ )
			{
				if( tdm_ins[ulSlot] & (1<<(8 - ulPort)) )
				{
					set_i2c_tdm_ext_info( ulSlot, ulPort, tdm_chip_type, tdm_vendor_id, tdm_e1_impedance );
				}
			}
			vty_out( vty, "  Slot %d: TDM-list=%x; Chipset-%s %s, E1-impedance-%s\r\n", ulSlot, tdm_ins[ulSlot],
						tdm_chip_type2name(tdm_chip_type),
						 tdm_vendor_id2name(tdm_vendor_id),
						 tdm_e1_impedance2name(tdm_e1_impedance) );
		}
	}

	return CMD_SUCCESS;
}

DEFUN (
    undo_tdm_ext_eeprom,
    undo_tdm_ext_eeprom_cmd,
    "undo tdm-eeprom-ext <portlist>",
    NO_STR
    /*Delete BOARD_TYPE_GFA6700_SIG_STR" EEPROM ext. info.\n"*/
    "Delete tdm eeprom ext. info.\n"
    "Specify tdm's port list(e.g.: 5/1 or 5/1-3)\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	UCHAR tdm_ins[12];
	UCHAR tdm_ins_count[12];
	
	ULONG ulIfIndex = 0;

	VOS_MemZero( tdm_ins, sizeof(tdm_ins) );
	VOS_MemZero( tdm_ins_count, sizeof(tdm_ins_count) );
	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( ulIfindex_2_userSlot_userPort(ulIfIndex, &ulSlot, &ulPort)  != VOS_OK )
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		/*if ( (ulSlot <4) || (ulSlot > 8) )*/
		if(TdmCardSlotPortCheckByVty(ulSlot,ulPort,vty) != ROK )
		{
			/*vty_out( vty, "  %% Parameter error.slot number must be %d-%d\r\n", TDMCARD_FIRST, TDMCARD_LAST);*/
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
		if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) != TRUE )
			{
			vty_out(vty,"  slot %d is empty\r\n", ulSlot);
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
		
		tdm_ins[ulSlot] |= (1<<(8 - ulPort));
		tdm_ins_count[ulSlot]++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	for( ulSlot=TDMCARD_FIRST; ulSlot<=TDMCARD_LAST; ulSlot++ )
	{
		if( tdm_ins_count[ulSlot] != 0 )
		{
			/*if( (__SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_SIG) && (__SYS_MODULE_TYPE__(ulSlot) != MODULE_TYPE_UNKNOW) )*/
			if(SlotCardMayBeTdmBoardByVty(ulSlot, vty) != ROK )
			{
				/*vty_out( vty, "  Slot %d is not %s board\r\n", ulSlot, BOARD_TYPE_GFA6700_SIG_STR);*/
				continue;
			}
			for( ulPort=0; ulPort<TDM_FPGA_MAX; ulPort++ )
			{
				if( tdm_ins[ulSlot] & (1<<(7 - ulPort)) )
				{
					set_i2c_tdm_ext_info( ulSlot, ulPort+1, 0, 0, 0 );
				}
			}
		}
	}

	return CMD_SUCCESS;
}

#endif	/* EPON_MODULE_TDM_SERVICE */

#define STATE_PS 1
#define STATE_PE 2
#define MAX_PORT_NUMBER_IN_PORT_LIST	64
#define START_PORT			1
#define END_PORT			MAX_PORT_NUMBER_IN_PORT_LIST
ULONG * V2R1_Parse_Port_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_PORT_NUMBER_IN_PORT_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

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
ULONG * V2R1_Parse_Vlan_List( char * pcvlan_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_PORT_NUMBER_IN_PORT_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

    VOS_MemZero( ulInterfaceList, MAX_PORT_NUMBER_IN_PORT_LIST * 4 );
    ulListLen = VOS_StrLen( pcvlan_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcvlan_List, ulListLen + 1 );
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

                    if ( ulPortS > END_VLAN )
                    {
                        goto error;
                    }
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
                        if ( i<START_VLAN || i > END_VLAN )
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

LONG V2R1_Check_Port_ListValid( const char * pcPort_List )
{
    ULONG * p = NULL;

    p = V2R1_Parse_Port_List( ( CHAR * ) pcPort_List );

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

/*************************************************/

/*使用举例，可以参照此命令函数内的使用方法
DEFUN (
    test_port_list,
    test_port_list_cmd,
    "test port_list <port_list>",
    "Test \n"
    "Test port_list\n"
    "Please input the port_list. e.g.1,3-4\n" )
{
	ULONG ulPort=0;

	
	if ( !V2R1_Check_Port_ListValid( argv[ 0 ] ) )
	{
		vty_out( vty, "%% Invalid port list <%s>. \r\n", argv[ 0 ] );
		return CMD_SUCCESS;
	}

	vty_out( vty, "Input port is :" );

	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0 ], ulPort )
	{
		vty_out( vty, "%d  ", ulPort );
	}
	END_PARSE_PORT_LIST_TO_PORT();

	vty_out( vty, "\r\n");

	return CMD_SUCCESS;
}*/
/* 
DEFUN (
    test_onuidlist,
    test_onuidlist_cmd,
    "test onuIdList <onuIdList>",
    "Test \n"
    "Test onuIdList\n"
    "Please input the onuIdList\n" )
{
	ULONG ulOnuId=0;

	
	if ( !V2R1_Check_Port_ListValid( argv[ 0 ] ) )
	{
		vty_out( vty, "%% Invalid port list <%s>. \r\n", argv[ 0 ] );
		return CMD_SUCCESS;
	}

	vty_out( vty, "Input onuid is :" );
 
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
		vty_out( vty, "%d  ", ulOnuId );
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

	vty_out( vty, "\r\n");

	return CMD_SUCCESS;
}
*/
enum match_type V2R1_Check_Port_List( char * port_list )
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

    VOS_MemZero( ( char * ) interface_list, MAX_PORT_NUMBER_IN_PORT_LIST * 
sizeof( ULONG ) );
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
                RETURN_PARSE_PORT_LIST_TO_PORT( no_match );  /* 写重复的端口认为是错误的语法  */
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


CMD_NOTIFY_REFISTER_S stCMD_V2R1_Port_List_Check =
{
    "<port_list>",
    V2R1_Check_Port_List,
    0
};


#if 0
#endif
#define MAX_ONUID_NUMBER_IN_ONUID_LIST			MAXONUPERPONNOLIMIT /*MAXONUPERPON*/ /*64*/
ULONG * V2R1_Parse_OnuId_List_1( CHAR * pcOnuId_List )
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

    VOS_MemZero( ulInterfaceList, MAX_ONUID_NUMBER_IN_ONUID_LIST * 4 );
    ulListLen = VOS_StrLen( pcOnuId_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcOnuId_List, ulListLen + 1 );
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

                    if ( ulPortS > MAX_ONUID_NUMBER_IN_ONUID_LIST )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > MAX_ONUID_NUMBER_IN_ONUID_LIST )
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
                        if ( i<START_PORT || i > END_PORT )
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > MAX_ONUID_NUMBER_IN_ONUID_LIST )
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

LONG V2R1_Check_OnuId_ListValid_1( const char * pcOnuId_List )
{
    ULONG * p = NULL;

    p = V2R1_Parse_OnuId_List_1( ( CHAR * ) pcOnuId_List );

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


enum match_type V2R1_Check_OnuId_List_1( char * onuid_list )
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

    BEGIN_PARSE_ONUID_LIST_TO_ONUID_1( plistbak, ulOnuId )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulOnuId )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulOnuId;
        if_num ++;
        if ( if_num > MAX_ONUID_NUMBER_IN_ONUID_LIST )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_PARSE_ONUID_LIST_TO_ONUID_1();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}




CMD_NOTIFY_REFISTER_S stCMD_V2R1_OnuId_List_Check_1 =
{
    "<onuid_list>",
    V2R1_Check_OnuId_List_1,
    0
};



enum match_type V2R1_Check_OUI( char * oui_str )
{
    int i;
    int len = VOS_StrLen( oui_str );

    if ( ( !oui_str ) || ( len != 6 ) )
    {
        return incomplete_match;
    }

    for ( i = 0; i < 6; i++ )
    {
        if ( !vos_isxdigit(oui_str[i]) )
        {
            return incomplete_match;
        }
    }

    return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_V2R1_OUI_Check =
{
    "<OUI>",
    V2R1_Check_OUI,
    0
};

#if 0
#else
/*添加igmpauth表项，并判断其状态，如果其状态为disable，则修改其状态*/
int CliIgmpAuthShowRun(struct vty *vty)
{
	/* modified by xieshl 20170425 问题单19778，同一onu端口的认证表项以范围段显示 */
	LONG lRet = VOS_OK;
	ULONG ulDevIdx=0, ulNextDevIdx= 0;
	ULONG ulSlot = 0, ulNextSlot = 0;
	ULONG ulPort = 0, ulNextPort = 0;
	ULONG ulOnuId = 0, ulNextOnuId = 0; 
	ULONG vlanId = 0, nextVlanId = 0;
	ULONG mulIp = 0, nextMulIp = 0, mergeMulIp = 0;

	ULONG authStatus = 0, nextAuthStatus = 0;
	ULONG mvid = 0, nextMvid = 0;

	CHAR mac[6]={0}, nextmac[6]={0};
	int macLen = 6;

	ULONG mip_c = 0;

	for( lRet = getFirstIgmpSnoopAuthEntry(&ulNextSlot, &ulNextPort, &ulNextOnuId, &nextVlanId, &nextMulIp);
		lRet == VOS_OK;
		lRet = getNextIgmpSnoopAuthEntry( ulSlot, ulPort , ulOnuId, vlanId, mulIp, &ulNextSlot,
							&ulNextPort, &ulNextOnuId, &nextVlanId, &nextMulIp) )
	{
		getCtcIgmpSnoopAuthMvlanId( ulNextSlot, ulNextPort, ulNextOnuId, nextVlanId, nextMulIp, &nextMvid );
		getCtcIgmpSnoopAuthStatus( ulNextSlot, ulNextPort, ulNextOnuId, nextVlanId, nextMulIp, &nextAuthStatus );

		if( mergeMulIp == 0 )
		{
			mergeMulIp = nextMulIp;
			mip_c = 1;
		}
		else
		{
			if( (mulIp + 1 == nextMulIp) &&
				(ulSlot == ulNextSlot) && (ulPort == ulNextPort) && (ulOnuId == ulNextOnuId) &&
				(vlanId == nextVlanId ) && (mvid == nextMvid) && (authStatus == nextAuthStatus) )
			{
				mip_c++;
			}
			else
			{
				vty_out( vty, "  igmp-auth %d/%d/%d %d %s ", ulSlot, ulPort, ulOnuId, vlanId, igmp_auth_ip_2_str(mergeMulIp) );
				if( mip_c == 1 )
				{
					vty_out( vty, "%d %d\r\n", authStatus, mvid );
				}
				else
				{
					vty_out( vty, "%s %d %d\r\n", igmp_auth_ip_2_str(mulIp), authStatus, mvid );
				}
				
				mergeMulIp = nextMulIp;
				mip_c = 1;
			}
		}
		ulSlot = ulNextSlot;
		ulPort = ulNextPort;
		ulOnuId = ulNextOnuId;
		vlanId = nextVlanId;
		mulIp = nextMulIp;
		mvid = nextMvid;
		authStatus = nextAuthStatus;
	}

	if( mip_c != 0 )
	{
		vty_out( vty, "  igmp-auth %d/%d/%d %d %s ", ulSlot, ulPort, ulOnuId, vlanId, igmp_auth_ip_2_str(mergeMulIp) );
		if( mip_c == 1 )
			vty_out( vty, "%d %d\r\n", authStatus, mvid );
		else if( mip_c >= 2 )
		{
			vty_out( vty, "%s %d %d\r\n", igmp_auth_ip_2_str(mulIp), authStatus, mvid );
		}
	}


	for( lRet = getFirstGwIgmpSnoopAuthEntry(&ulNextDevIdx, &nextVlanId, &nextMulIp, nextmac);
		lRet == VOS_OK;
		lRet = getNextGwIgmpSnoopAuthEntry(ulDevIdx, vlanId, mulIp, mac, &ulNextDevIdx, &nextVlanId, &nextMulIp, nextmac) )
	{
		ulSlot = GET_PONSLOT(ulNextDevIdx);
		ulPort = GET_PONPORT(ulNextDevIdx);
		ulOnuId = GET_ONUID(ulNextDevIdx);
		getGwIgmpSnoopAuthStatus( ulNextDevIdx,nextVlanId, nextMulIp, nextmac, &authStatus );
		getGwIgmpSnoopAuthMulticastVid( ulNextDevIdx, nextVlanId, nextMulIp, nextmac, &mvid );

		vty_out( vty, "  igmp-auth-gw %d/%d/%d %d %s %s %d\r\n",
			ulSlot, ulPort, ulOnuId, nextVlanId, igmp_auth_ip_2_str(nextMulIp), igmp_auth_mac_2_str(nextmac), mvid );
		vty_out( vty, "  igmp-auth-gw status %d/%d/%d %d %s %s %d\r\n",
			ulSlot, ulPort, ulOnuId, nextVlanId, igmp_auth_ip_2_str(nextMulIp), igmp_auth_mac_2_str(nextmac), authStatus );
		
		ulDevIdx = ulNextDevIdx;
		vlanId = nextVlanId;
		mulIp = nextMulIp;
		VOS_MemCpy( mac, nextmac, macLen );
	}
	
	return CMD_SUCCESS;
}
#endif

/* added by chenfj 2007-6-21  增加PON 端口保护切换*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
DEFUN (
	set_pon_port_hot_swap,
	set_pon_port_hot_swap_cmd,
	"auto-protect pon <slot/port> partner "EPON_PON_SWAP_PORTRANGE,
	"set a pon auto-protection,default mode is slowness\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"the partner auto-protect "EPON_PON_SWAP_PORTTYPE"port\n"
	"please input the "EPON_PON_SWAP_PORTTYPE"port\n"
	)
{

	unsigned long  ul_desSlot =0, ul_desPort = 0;
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Des_PonPort;
	int ret;

	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );

	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(ul_srcSlot,vty) != ROK )
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% 1st pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	/* the dest pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[1], &ul_desSlot, &ul_desPort );
#else
	IFM_ParseSlotPort( argv[1], &ul_desSlot, &ul_desPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_desSlot, ul_desPort, vty) != ROK)
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
            default:
        		vty_out( vty, "  %% The pair pons failed to set \r\n");
        }
        
		return( CMD_WARNING );
    }   
        
    return CMD_SUCCESS;
}

/*for onu swap by jinhl@2013-04-27*/
/*****************************************************************
pon口上有onu注册，又确定要配成onu保护倒换模式，
强制关闭光口，待所有onu离线后，进行配置。
*****************************************************************/
int do_protect_onuswap(struct vty *vty)
{
	short int  Src_PonPort = 0, Des_PonPort = 0;
    
	int ret = 0;
	OLT_onu_table_t onu_tableSrc;
	OLT_onu_table_t onu_tableDes;
	int i = 0;

	VOS_MemSet((void *)&onu_tableSrc, 0, sizeof(OLT_onu_table_t));
	VOS_MemSet((void *)&onu_tableDes, 0, sizeof(OLT_onu_table_t));
    Src_PonPort = VTY_CONFIRM_CB_GETPARAM1(vty);
	Des_PonPort = VTY_CONFIRM_CB_GETPARAM2(vty);

	OLTAdv_SetOpticalTxMode2(Src_PonPort, FALSE, PONPORT_TX_SWITCH);
	OLTAdv_SetOpticalTxMode2(Des_PonPort, FALSE, PONPORT_TX_SWITCH);

	for(i = 0; i < MAXONUPERPON; i++)
	{
	    OLT_DeregisterLLID(Src_PonPort, i, 0);
		OLT_DeregisterLLID(Des_PonPort, i, 0);
	}
    while(HasOnuOnlineOrRegistering(Src_PonPort) || HasOnuOnlineOrRegistering(Des_PonPort))
	{
	    VOS_TaskDelay(VOS_TICK_SECOND/2);
	}

    /*这里再加个延时，若不延时，即使通过PAS_get_onu_parameters获取
	onu的数目是0，pas_soft设置冗余配置PAS_set_redundancy_config还是会出错*/
	VOS_TaskDelay(VOS_TICK_SECOND*5);
	#if 0
	ret = OLT_GetAllOnus(Src_PonPort, &onu_tableSrc);
	ret = OLT_GetAllOnus(Src_PonPort, &onu_tableDes);
	vty_out( vty, "  %% srcnum:%d,desnum:%d \r\n",onu_tableSrc.onu_num,onu_tableDes.onu_num);
	#endif
	ret = EnablePonPortHotSwapPort(Src_PonPort, Des_PonPort, V2R1_PON_PORT_SWAP_ONU);
	OLTAdv_SetOpticalTxMode2(Src_PonPort, TRUE, PONPORT_TX_SWITCH);
	OLTAdv_SetOpticalTxMode2(Des_PonPort, TRUE, PONPORT_TX_SWITCH);
	if (ret != VOS_OK)
    {
        vty_out( vty, "  %% The pair pons failed to set \r\n");
        return( CMD_WARNING );
    }
    
	return CMD_SUCCESS;
}

/*for onu swap by jinhl@2013-04-27*/
/*****************************************************************
pon口上有onu注册，又确定要去配onu保护倒换模式，
强制关闭光口，待所有onu离线后，进行去配置。
*****************************************************************/
int undo_protect_onuswap(struct vty *vty)
{
	short int  PonPortIdx = 0, PonPortIdx_Swap = 0;
    int i = 0;
	int ret = 0;
	
    PonPortIdx = VTY_CONFIRM_CB_GETPARAM1(vty);
	ret = PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap);

	OLTAdv_SetOpticalTxMode2(PonPortIdx, FALSE, PONPORT_TX_SWITCH);
	OLTAdv_SetOpticalTxMode2(PonPortIdx_Swap, FALSE, PONPORT_TX_SWITCH);

	for(i = 0; i < MAXONUPERPON; i++)
	{
	    OLT_DeregisterLLID(PonPortIdx, i, 0);
		OLT_DeregisterLLID(PonPortIdx_Swap, i, 0);
	}
    while(HasOnuOnlineOrRegistering(PonPortIdx) || HasOnuOnlineOrRegistering(PonPortIdx_Swap))
	{
	    VOS_TaskDelay(VOS_TICK_SECOND/2);
	}
	
	/*这里再加个延时，若不延时，即使通过PAS_get_onu_parameters获取
	onu的数目是0，pas_soft设置冗余配置PAS_set_redundancy_config还是会出错*/
	VOS_TaskDelay(VOS_TICK_SECOND*5);
	ret = DisablePonPortHotSwap(PonPortIdx);
	OLTAdv_SetOpticalTxMode2(PonPortIdx, TRUE, PONPORT_TX_SWITCH);
	OLTAdv_SetOpticalTxMode2(PonPortIdx_Swap, TRUE, PONPORT_TX_SWITCH);
	if (ret != VOS_OK)
    {
        vty_out( vty, "  %% The pair pons failed to set \r\n");
        return( CMD_WARNING );
    }
    
	return CMD_SUCCESS;
}

/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#if ( (EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES) || (EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES) )
DEFUN (
	set_pon_port_hot_swap_full,
	set_pon_port_hot_swap_full_cmd,
	"auto-protect pon <slot/port> partner "EPON_PON_SWAP_PORTRANGE" mode ["EPON_PON_SWAP_MODE"]", /* <gslot/gport> */
	"set a pon auto-protection\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
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

	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(ul_srcSlot,vty) != ROK )
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% 1st pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	/* the dest pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[1], &ul_desSlot, &ul_desPort );
#else
	IFM_ParseSlotPort( argv[1], &ul_desSlot, &ul_desPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_desSlot, ul_desPort, vty) != ROK)
		return(CMD_WARNING);
	Des_PonPort = GetPonPortIdxBySlot( (short int)ul_desSlot, (short int)ul_desPort );
	if (Des_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% 2nd pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}
   
    if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_SLOWLY_STR, argv[2]) )
    {
        iSwapMode = V2R1_PON_PORT_SWAP_SLOWLY;
    }
    else
    {
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_OLT_QUICKLY_STR, argv[2]) )
        {
            iSwapMode = V2R1_PON_PORT_SWAP_QUICKLY;
        }
        else
#endif
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
        if ( 0 == VOS_StrCmp(EPON_PON_SWAP_MODE_ONU_OPTIC_STR, argv[2]) )
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

DEFUN (
	show_pon_port_hot_swap_mode,
	show_pon_port_hot_swap_mode_cmd,
	"show auto-protect mode pon <slot/port>",
	"show pon auto-protect switching mode\n"
	"show pon auto-protect switching mode\n"
	"show pon auto-protect switching mode\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

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
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */


DEFUN (
	undo_set_pon_port_hot_swap,
	undo_set_pon_port_hot_swap_cmd,
	"undo auto-protect pon <slot/port>",
	"clear a pon auto-protection\n"
	"clear a pon auto-protection\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	)
{

	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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
	op_trigger_pon_port_hot_swap,
	op_trigger_pon_port_hot_swap_cmd,
	"trigger-switch pon <slot/port> optical-power",
	"config pon switch's trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"trigger pon switch at optical-power's abnormal\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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
	op_untrigger_pon_port_hot_swap,
	op_untrigger_pon_port_hot_swap_cmd,
	"undo trigger-switch pon <slot/port> optical-power",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"untrigger pon switch at optical-power's abnormal\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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
	oe_trigger_pon_port_hot_swap,
	oe_trigger_pon_port_hot_swap_cmd,
	"trigger-switch pon <slot/port> data-error",
	"config pon switch's trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"trigger pon switch at optical-data's received abnormal\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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
	oe_untrigger_pon_port_hot_swap,
	oe_untrigger_pon_port_hot_swap_cmd,
	"undo trigger-switch pon <slot/port> data-error",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"untrigger pon switch at optical-data's received abnormal\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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
	ud_trigger_pon_port_hot_swap,
	ud_trigger_pon_port_hot_swap_cmd,
	"trigger-switch pon <slot/port> uplink-down <slot/port>",
	"config pon switch's trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"trigger pon switch at uplink-port's link-down\n"
	"please input the uplink port\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned long ul_desSlot = 0, ul_desPort = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
    ULONG ulIfindex;
    unsigned char ucSlot, ucPort;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ul_srcSlot, ul_srcPort );

		return CMD_WARNING;
    }
#endif

	if( IFM_ParseSlotPort( argv[1], &ul_desSlot, &ul_desPort ) != VOS_OK )
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

    if ( 0 != OLT_SetHotSwapParam(Src_PonPort, -1, -1, -1, iCurrTriggers, (int)ul_desSlot, (int)ul_desPort) )
    {
		vty_out(vty, "  %% pon executing err\r\n");
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
	ud_untrigger_pon_port_hot_swap,
	ud_untrigger_pon_port_hot_swap_cmd,
	"undo trigger-switch pon <slot/port> uplink-down",
	"clear a pon auto-switch trigger\n"
	"clear a pon auto-switch trigger\n"
	"the auto-protect pon port\n"
	"please input the pon port\n"
	"untrigger pon switch at uplink-port's link-down\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned long ul_desSlot = 0, ul_desPort = 0;
	short int  Src_PonPort, Partner_PonPort;
    int iCurrTriggers;
    ULONG ulIfindex;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

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


DEFUN (
	force_pon_port_hot_swap,
	force_pon_port_hot_swap_cmd,
	"force-switching pon "EPON_PON_SWAP_PORTRANGE,
	"force pon switching\n"
	"the switching to pon port\n"
	"please input the pon port switching to\n"
	)
{

	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	
	/* source pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
#else
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	ret = ForcePonPortAutoProtect(ul_srcSlot, ul_srcPort);
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
	sync_pon_port_hot_swap,
	sync_pon_port_hot_swap_cmd,
	"syncfg pon "EPON_PON_SWAP_PORTRANGE,
	"sync the master pon's configs to the slave pon\n"
	"the slave pon port\n"
	"please input the pon port to sync\n"
	)
{
	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	int ret;
	short int iProtectMode = 0;
	
	/* source pon port */
#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
    IFM_ParseGlobalSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
#else
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
#endif

	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

    /*Begin:for onu swap by jinhl@2013-02-22*/
    iProtectMode = GetPonPortHotSwapMode(Src_PonPort);
	if(V2R1_PON_PORT_SWAP_ONU == iProtectMode) 
	{
		vty_out( vty, "  %% The pon port is onu protect mode.\r\n" );

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
/* E--added by liwei056@2011-12-14 for AddManualSyncCmd */


#ifdef __CTC_TEST
extern int CheckPonPortSignal_time_delay;
DEFUN (
	pon_force_switch_config,
	pon_switch_config_cmd,
	"pon auto-protect [enable|disable] {<0-2000>}*1",
	"pon port config\n"
	"auto switching\n"
	"enable\n"
	"disable\n" 
	"pon aps waiting interval, unit:10ms\n"
	)
{
	if( argc == 2 )
		CheckPonPortSignal_time_delay = (int)VOS_AtoL( argv[1]);
	
	if( argv[0][0] == 'e' )
	{
		pon_force_switch_enable = V2R1_ENABLE;
	}
	else
	{
		pon_force_switch_enable = V2R1_DISABLE;
		V2R1CheckPonSignalLoss();
	}
			
	return CMD_SUCCESS;
}

DEFUN (
	show_pon_force_switch_config,
	show_force_switch_config_cmd,
	"show pon auto-switch-config",
	"show config information\n"
	"show pon force switch config\n"
	"show pon force switch config\n"
	)
{
	vty_out(vty, "\r\npon signal-loss check:%s\r\n", (V2R1_ENABLE == pon_force_switch_enable)? "enabled":"disabled");
	vty_out(vty,"delay time=%dms\r\n",CheckPonPortSignal_time_delay);
	return CMD_SUCCESS;
}
#endif


DEFUN (
	show_pon_port_hot_swap,
	show_pon_port_hot_swap_cmd,
	"show auto-protect {pon <slot/port>}*1",
	"show pon auto-protect\n"
	"show pon auto-protect\n"
	"the specific pon port\n"
	"please input the pon port\n"
	)
{

	unsigned long   ul_srcSlot = 0, ul_srcPort = 0 ;
	unsigned int   ul_desSlot = 0, ul_desPort=0;
	short int  Src_PonPort, Des_PonPort;
	int ret;
	int flag =0;
	int iProtectMode = 0;
	int state = 0;
	
    if( argc == 1 )
    {
    	/* source pon port */
    	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
    	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
    		return(CMD_WARNING);
    	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
    	if (Src_PonPort == VOS_ERROR)
    	{
    		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

    		return CMD_WARNING;
    	}

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
	    	vty_out( vty, "PON Automatic Protection Switching\r\n");
	    	vty_out( vty, "-----------------------------------\r\n");
	    	vty_out( vty, "  ACTIVE PON       PASSIVE PON \r\n");
	    	/* 问题单#5385: modified by chenfj 2007-9-19
	    		在CONFIG节点下，使用命令show auto-protect pon slot/port 显示当前处于激活状态的
	    		PON端口错误，处于激活状态的总是命令中输入的端口*/
#ifdef __CTC_TEST
	    	display_pon_aps_status( vty, Src_PonPort, ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort);
#else
	    	if( PonPortHotStatusQuery( Src_PonPort ) == V2R1_PON_PORT_SWAP_ACTIVE )
	        	vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_srcSlot, ul_srcPort, ul_desSlot, ul_desPort );
	    	else
	            vty_out( vty, "   %2d/%-2d              %2d/%-2d\r\n", ul_desSlot, ul_desPort, ul_srcSlot, ul_srcPort );
#endif  
		}
		else
		{
		    vty_out( vty, "PON ONU Protection Switching\r\n");
	    	vty_out( vty, "-----------------------------------\r\n");
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

    	return( CMD_SUCCESS );

    }

	ShowAllPonPortAutoProtect(vty);
	ShowAllPonPortOnuProtect(vty);
        
    return CMD_SUCCESS;
}



DEFUN (
	set_pon_port_hot_swap_timer,
	force_pon_port_hot_swap_timer_cmd,
	"set pon auto-protect time <3-100>",
	"set pon auto-protect switching time\n"
	"set pon auto-protect switching time\n"
	"set pon auto-protect switching time\n"
	"set pon auto-protect switching time\n"
	"please input the time,unit:S\n"
	)
{
    OLT_SetHotSwapParam(OLT_ID_ALL, -1, VOS_AtoL( argv[ 0 ] ), -1, -1, -1, -1);
        
    return CMD_SUCCESS;
}

/*  added by chenfj 2007-9-20 
	问题单#5379 
    没有专门的命令查看当前的PON口保护倒换检测时间值
*/
DEFUN (
	show_pon_port_hot_swap_timer,
	show_force_pon_port_hot_swap_timer_cmd,
	"show pon auto-protect time",
	"show pon auto-protect switching time\n"
	"show pon auto-protect switching time\n"
	"show pon auto-protect switching time\n"
	"show pon auto-protect switching time\n"
	)
{

	vty_out(vty,"pon auto-protect time %d(s)\r\n", V2R1_AutoProtect_Timer );
        
    return CMD_SUCCESS;
}

/* B--added by liwei056@2012-8-7 for D14913 */
DEFUN (
	show_pon_port_trigger_swap,
	show_pon_port_trigger_swap_cmd,
	"show trigger-switch pon <slot/port>",
	"show pon switch's trigger\n"
	"show pon switch's trigger\n"
	"the specific pon port\n"
	"please input the pon port\n"
	)
{
	unsigned long ul_srcSlot = 0, ul_srcPort = 0 ;
	short int  Src_PonPort;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_srcSlot, &ul_srcPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ul_srcSlot, ul_srcPort, vty) != ROK)
		return(CMD_WARNING);
	Src_PonPort = GetPonPortIdxBySlot( (short int)ul_srcSlot, (short int)ul_srcPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}
    
    ShowPonPortAutoProtectTrigger(vty, ul_srcSlot, ul_srcPort);
        
	return( CMD_SUCCESS );
}
/* E--added by liwei056@2012-8-7 for D14913 */

/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
int pon_swap_switch_enable = V2R1_ENABLE;
DEFUN (
	pon_swap_switch_config,
	pon_swap_switch_config_cmd,
	"auto-protect pon [enable|disable]",
	"auto switching\n"
	"pon port auto switching config\n"
	"enable pon auto-protect service\n"
	"disable pon auto-protect service\n" 
	)
{
    int iEnabledVal;
        
	if( argv[0][0] == 'e' )
	{
	    if (V2R1_ENABLE == pon_swap_switch_enable)
        {
        	vty_out(vty,"pon auto-protect have been enabled.\r\n" );
        	return CMD_SUCCESS;
        }

        iEnabledVal = V2R1_ENABLE;
	}
	else
	{
	    if (V2R1_DISABLE == pon_swap_switch_enable)
        {
        	vty_out(vty,"pon auto-protect have been disabled.\r\n" );
        	return CMD_SUCCESS;
        }

        iEnabledVal = V2R1_DISABLE;
	}

    OLT_SetHotSwapParam(OLT_ID_ALL, iEnabledVal, -1, -1, -1, -1, -1);
			
	return CMD_SUCCESS;
}
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#endif

/* B--added by liwei056@2012-3-6 for 国电ONU倒换功能 */
#if ( EPON_MODULE_ONU_HOT_SWAP == EPON_MODULE_YES )
DEFUN (
	force_pon_onu_hot_swap,
	force_pon_onu_hot_swap_cmd,
	"force-switching onu <slot/port/onuid>",
	"force pon switching\n"
	"force onu switching\n"
	"please input the onu to switch\n"
	)
{
	ULONG ulSlot;
	ULONG ulPort;
	ULONG ulOnuid;
    INT16 Src_PonPort;
    INT16 Partner_PonPort;
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ulSlot, ulPort );

		return CMD_WARNING;
    }
#endif

	if ( ROK != PONTx_Disable(Src_PonPort, PONPORT_TX_ACTIVED) )
    {
		vty_out(vty, "  %% executing err\r\n");

        return CMD_WARNING;
    }   
    
    return CMD_SUCCESS;
}

DEFUN (
	resume_pon_onu_hot_swap,
	resume_pon_onu_hot_swap_cmd,
	"resume-switching onu <slot/port/onuid>",
	"resume pon switching\n"
	"resume onu switching\n"
	"please input the onu resume to\n"
	)
{
	ULONG ulSlot;
	ULONG ulPort;
	ULONG ulOnuid;
    INT16 Src_PonPort;
    INT16 Partner_PonPort;
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(CMD_WARNING);

	Src_PonPort = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (Src_PonPort == VOS_ERROR)
	{
		vty_out( vty, "  %% The pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

#if 0
    if ( ROK != PonPortSwapPortQuery(Src_PonPort, &Partner_PonPort) )
    {
		vty_out( vty, "  %% pon %d/%d has no auto-protect partner port\r\n", ulSlot, ulPort );

		return CMD_WARNING;
    }
#endif

	if ( ROK != PONTx_Enable(Src_PonPort, PONPORT_TX_SWITCH) )
    {
		vty_out(vty, "  %% executing err\r\n");

        return CMD_WARNING;
    }   
    
    return CMD_SUCCESS;
}
#endif
/* E--added by liwei056@2012-3-6 for 国电ONU倒换功能 */



#ifdef  ONU_AUTHENTICATION_CONTROL
#endif

/* 	 added by chenfj 2007-7-6
*	     for onu authenticaion by PAS5001 & PAS5201
*         注: 在PAS5001上，onu authentication是由软件来实现的；在PAS5201上，若启动CTC协议栈，onu authentication是由PON芯片硬件实现的；若没有启动CTC协议栈，onu authentication 也是由 软件来实现
*/

DEFUN (
	set_onu_authentication,
	set_onu_authentication_cmd,
	"onu-register authentication enable {[auth-all]}*1 {[pon] <slot/port>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"authentication only to onu registered lately\n"
	"authentication to all onu(include onu registered currently)\n" 
	"authentication to pon port\n"
	"please input the slot/port\n"
	)
{
	unsigned long enable = 0, old_mode = 0;
    ULONG ulsslot = 0, ulsport = 0, lRet = 0;
    ULONG slot = 0, port = 0, ulIfIndex = 0;
    ULONG liv_oportist = 0, liv_iportlist = 0;
    short int pon_id = 0;
    int flag = 0;
    char portlist_str[80]="";
    short int PonPortIdx = 0;

    if (argc == 2)
		IFM_ParseSlotPort( argv[1], &ulsslot, &ulsport );
        
    if (argc == 3)
		IFM_ParseSlotPort( argv[2], &ulsslot, &ulsport );
            
    if (argc == 0 || argc == 2)
        enable = V2R1_ONU_AUTHENTICATION_NEW_ONLY;
    else if (argc == 1 || argc == 3)
        enable = V2R1_ONU_AUTHENTICATION_ALL;     
    
    if(argc == 2 || argc == 3)
    {
		SWAP_TO_ACTIVE_PON_PORT(ulsslot, ulsport)

    	if(PonCardSlotPortCheckWhenRunningByVty(ulsslot, ulsport, vty) != ROK)
    		return(CMD_WARNING);

		PonPortIdx = GetPonPortIdxBySlot( (short int)ulsslot, (short int)ulsport );
		if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
			vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
			vty_out( vty, "  %% pon port Parameter is error.\r\n" );

			return CMD_WARNING;
		}
            
        getOnuAuthEnable(ulsslot, ulsport, &old_mode);
        if(old_mode != V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            vty_out(vty, "  %% PON %d/%d onu authentication enabled already\r\n", ulsslot, ulsport);
            return CMD_SUCCESS;
        }
        setOnuAuthEnable(ulsslot, ulsport, enable);
    }
    else
    {
        getOnuAuthEnable(0, 0, &old_mode);
        if(old_mode != V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            vty_out(vty, "  %% onu authentication enabled already\r\n");
            return( CMD_SUCCESS );
        }
        setOnuAuthEnable(0, 0, enable);
    }
	return( CMD_SUCCESS ); 	
#if 0
	if ( !( VOS_StrCmp( argv[ 0 ] , "new-register-only" ) ) )
		enable = V2R1_ONU_AUTHENTICATION_NEW_ONLY;
	else if ( !( VOS_StrCmp( argv[ 0 ] , "auth-all" ) ) )
		enable = V2R1_ONU_AUTHENTICATION_ALL; 
	else 
		{
		vty_out( vty, " %% [new-only|all] Parameter input err\r\n");
		return( CMD_WARNING);
		} 
#endif
}


DEFUN (
	undo_set_onu_authentication,
	undo_set_onu_authentication_cmd,
	"undo onu-register authentication enable {<slot/port>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication enable\n"
	"please input the slot/port\n"
	)
{
	unsigned long enable;
    ULONG ulsslot = 0, ulsport = 0, lRet = 0;
    ULONG ulIfIndex = 0;
    ULONG liv_oportist = 0, liv_iportlist = 0;
    char portlist_str[80]="";
    short int PonPortIdx = 0;
    
    if(argc == 1)
    {
		IFM_ParseSlotPort( argv[0], &ulsslot, &ulsport );

		SWAP_TO_ACTIVE_PON_PORT(ulsslot, ulsport)
        
        if(PonCardSlotPortCheckWhenRunningByVty(ulsslot, ulsport, vty) != ROK)
            return(CMD_WARNING);
        
		PonPortIdx = GetPonPortIdxBySlot( (short int)ulsslot, (short int)ulsport );
		if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
    		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
    		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
    		return CMD_WARNING;
		}
        
        if(getOnuAuthEnable(ulsslot, ulsport, &enable) == VOS_OK 
            && enable == V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            vty_out(vty, "  %% PON %d/%d onu authentication disabled already\r\n", ulsslot, ulsport);
            return CMD_SUCCESS;                
        }
        setOnuAuthEnable(ulsslot, ulsport, V2R1_ONU_AUTHENTICATION_DISABLE);
    }
    else
    {
        setOnuAuthEnable(0, 0, V2R1_ONU_AUTHENTICATION_DISABLE);
    }
	return( CMD_SUCCESS ); 		
}
DEFUN (
	show_onu_authentication,
	show_onu_authentication_cmd,
	"show onu-register authentication enable {<slot/port>}*1",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication enable\n"
	"please input the slot/port\n"
	)
{
	unsigned long enable;
    ULONG ulsslot, ulsport, lRet = 0;
    ULONG ulIfIndex = 0;
    short int PonPortIdx = 0;
    if(argc == 1)
    {
		IFM_ParseSlotPort( argv[0], &ulsslot, &ulsport );
        if(PonCardSlotPortCheckWhenRunningByVty(ulsslot, ulsport, vty) != ROK)
            return(CMD_WARNING);

		PonPortIdx = GetPonPortIdxBySlot( (short int)ulsslot, (short int)ulsport );
		if (PonPortIdx == VOS_ERROR)
			{
#ifdef CLI_EPON_DEBUG
			vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
			vty_out( vty, "  %% pon port Parameter is error.\r\n" );

			return CMD_WARNING;
			}

        
        if(getOnuAuthEnable(ulsslot, ulsport, &enable) == VOS_OK) 
        {
            if(enable != V2R1_ONU_AUTHENTICATION_DISABLE)
                vty_out(vty, " Pon %d/%d's onu-register authentication is Enable\r\n", ulsslot, ulsport);
            else
                vty_out(vty, " Pon %d/%d's onu-register authentication is Disable\r\n", ulsslot, ulsport);
        }
    }
    else
    {
        if(getOnuAuthEnable(0, 0, &enable) == VOS_OK && enable != V2R1_ONU_AUTHENTICATION_DISABLE) 
        {
            vty_out(vty, " onu-register authentication is Enable\r\n", ulsslot, ulsport);
        }
        else
        {
            for(ulsslot=PONCARD_FIRST;ulsslot<=PONCARD_LAST;ulsslot++)
            {
                if(SlotCardMayBePonBoard(ulsslot) == ROK)
                {
                    for(ulsport=1;ulsport<=PONPORTPERCARD;ulsport++)
                    {
                		PonPortIdx = GetPonPortIdxBySlot( (short int)ulsslot, (short int)ulsport );
                		if (PonPortIdx == VOS_ERROR)
                            continue;
                        
                            getOnuAuthEnable(ulsslot, ulsport, &enable);
                            if(enable != V2R1_ONU_AUTHENTICATION_DISABLE)
                                vty_out(vty, " Pon %d/%d's onu-register authentication is Enable\r\n", ulsslot, ulsport);	
                            else
                                vty_out(vty, " Pon %d/%d's onu-register authentication is Disable\r\n", ulsslot, ulsport);
                    }
                }
            }
        }
    }
	return( CMD_SUCCESS );
}

DEFUN (
	show_onu_authentication_entry,
	show_onu_authentication_entry_cmd,
	"show onu-register authentication entry {<slot/port>}*1",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication mac table\n"
	"please input the slot/port\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0 ;
	unsigned int  PonPortIdx;
	
	if( argc == 1 )
		{
		/* source pon port */
		IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
		if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port,vty) != ROK)
			return(CMD_WARNING);

		PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
		if (PonPortIdx == VOS_ERROR)
			{
#ifdef CLI_EPON_DEBUG
			vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
			vty_out( vty, "  %% pon port Parameter is error.\r\n" );

			return CMD_WARNING;
			}

		ShowPonPortAuthOnuEntry( PonPortIdx,  vty );
		return( CMD_SUCCESS );
		
		}

	else if ( argc == 0 )
		{
		ShowPonPortAuthOnuEntryALL( vty );
		return( CMD_SUCCESS );
		}

	return( CMD_WARNING );

	
}

DEFUN (
	reorganize_onu_authentication_entry,
	reorganize_onu_authentication_entry_cmd,
	"reorganize onu-register authentication entry {<slot/port>}*1",
	"Reorganize onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mac table\n"
	"please input the pon port\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0 ;
	unsigned int  PonPortIdx;
	int ret;
	
	if(argc == 1)
	{
    	IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
    	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port,vty) != ROK)
    		return(CMD_WARNING);

    	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
    	if (PonPortIdx == VOS_ERROR)
		{
    		vty_out( vty, "  %% pon port Parameter is error.\r\n" );
    		return CMD_WARNING;
		}
    	ret = OLT_SetAuthEntry( PonPortIdx, 1);
    	if( ret != VOS_OK )	
    	{
    		vty_out( vty, "  %% Execting err\r\n");
    		return( CMD_WARNING );
     	}               
	}
    else
    {
        for(ul_slot=1;ul_slot<=PONCARD_LAST;ul_slot++)
        {
            for(ul_port=1;ul_port<=PONPORTPERCARD;ul_port++)
            {
                PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
                if(PonPortIdx == VOS_ERROR)
                    continue;
            	OLT_SetAuthEntry(PonPortIdx, 1);                            
            }
        }
    }
			 			 
	 return( CMD_SUCCESS );
	 	
}

DEFUN (
	add_onu_authentication_entry,
	add_onu_authentication_entry_cmd,
	"add onu-register authentication entry <slot/port> <H.H.H>",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mac table\n"
	"please input the pon port\n"
	"please input the onu mac address\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0 ;
	unsigned long  TableIdx;
	unsigned int  PonPortIdx;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
	int ret;
	
	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );

	SWAP_TO_ACTIVE_PON_PORT(ul_slot, ul_port)

	if(SYS_MODULE_IS_RUNNING(ul_slot))
	{
		if(SYS_MODULE_IS_GPON(ul_slot))
		{
			vty_out(vty,"  %%  slot %d is Gpon, you can't add epon entry!\r\n", ul_slot);
			return (CMD_WARNING);
		}
	}
	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port,vty) != ROK)
		return(CMD_WARNING);

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
	}

	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
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
		vty_out(vty, "  %% this mac addr is already in authentication onu table\r\n");
		return( CMD_SUCCESS );
	}

	if( (TableIdx < 1 ) ||( TableIdx > MAXONUPERPON ))
	{
		vty_out(vty,"pon %d/%d is full!",ul_slot,ul_port);/*问题单8825*/
		/*vty_out( vty, "  %% Execting err\r\n");*/
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
	delete_onu_authentication_entry,
	delete_onu_authentication_entry_cmd,
	"delete onu-register authentication entry <slot/port> {<H.H.H>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mac table\n"
	"please input the pon port\n"
	"please input the onu mac address\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0 ;
	unsigned long  TableIdx;
	unsigned int  PonPortIdx;
	unsigned char MacAddr[6] = {0,0,0,0,0,0};
	int ret;

	/* source pon port */
	IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );

	SWAP_TO_ACTIVE_PON_PORT(ul_slot, ul_port)

	if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port,vty) != ROK)
		return(CMD_WARNING);

	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
	if (PonPortIdx == VOS_ERROR)
	{
		vty_out( vty, "  %% pon %s is error.\r\n", argv[0] );

		return CMD_WARNING;
	}

	if( argc == 2 )
	{
		if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
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

	else if( argc == 1 )
	{
		DeletePonPortAuthEntryAll( PonPortIdx );
		return( CMD_SUCCESS );
	}

	return( CMD_SUCCESS );

}

#if 1
DEFUN (
	show_gonu_authentication_entry,
	show_gonu_authentication_entry_cmd,
	"show gonu-register authentication entry {<slot/port>}*1",
	SHOW_STR
	"show gonu-register authentication information\n"
	"show gonu-register authentication information\n"
	"show gonu-register authentication serial number table\n"
	"please input the slot/port\n"	
	)
{	
    unsigned long  ul_slot = 0, ul_port = 0 ;
    unsigned int  PonPortIdx;

    if( argc == 1 )
    {
        IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );
        if(PonCardSlotPortCheckWhenRunningByVty(ul_slot, ul_port,vty) != ROK)
            return(CMD_WARNING);

        PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short int)ul_port );
        if (PonPortIdx == VOS_ERROR)
        {
            vty_out( vty, "  %% pon port Parameter is error.\r\n" );
            return CMD_WARNING;
        }
        
        ShowPonPortAuthGponOnuEntry(vty, ul_slot, ul_port);
        return( CMD_SUCCESS );
    }
    else if ( argc == 0 )
    {
        ShowPonPortAuthGponOnuEntryALL( vty );
        return( CMD_SUCCESS );
    }

    return( CMD_WARNING );
}


DEFUN (
	add_gonu_authentication_entry,
	add_gonu_authentication_entry_cmd,
	"add gonu-register authentication entry <slot/port> <sn> {<pwd>}*1", /*将密码改为可选参数，mod by liuyh, 2017-5-11*/
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication entry\n"
	"please input the slot/port\n"		
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
	
	IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );

	if(SYS_MODULE_IS_RUNNING(ul_slot))
	{
		if(SYS_MODULE_IS_GPON(ul_slot) == 0)
		{
			vty_out(vty,"  %%  slot %d is Epon, you can't add gpon entry.\r\n", ul_slot);
			return (CMD_WARNING);
		}
	}
	
	
	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		vty_out(vty,"  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
    
	if( VOS_StrLen(argv[1]) != 16 ) 
	{
		vty_out(vty,"  %%  serial number %s is invalid\r\n", (unsigned char *)argv[1] );
		return(CMD_WARNING );
	}
	else
	{
	    VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
		VOS_MemZero(SN,GPON_ONU_SERIAL_NUM_STR_LEN);
	    VOS_MemCpy(entry.authEntry.serial_no, argv[1], GPON_ONU_SERIAL_NUM_STR_LEN-1);
		VOS_MemCpy(SN, argv[1], GPON_ONU_SERIAL_NUM_STR_LEN-1);	
	}
	
    if(argc == 3)
    {
        if(VOS_StrLen(argv[2]) >= GPON_ONU_PASSWARD_STR_LEN)
        {
            vty_out(vty,"  %%  passward %s is invalid\r\n", (unsigned char *)argv[2] );
            return(CMD_WARNING );
        }
        else
        {
            VOS_MemCpy(entry.authEntry.password, argv[2], GPON_ONU_PASSWARD_STR_LEN-1);              
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
	OnuAuth_ActivatePendingOnuBySnMsg(PonPortIdx,SN);
	ActivateGponPendingOnuMsg_conf(PonPortIdx,SN);
    
    return( CMD_SUCCESS);
}


DEFUN (
	delete_gonu_authentication_entry,
	delete_gonu_authentication_entry_cmd,
	"delete gonu-register authentication entry <slot/port> {<sn>}*1",
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication information\n"
	"Config gonu-register authentication entry\n"
	"please input the slot/port\n"			
	"please input the gonu serial number\n"
	)
{	
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	unsigned long  TableIdx = 0;
	short int  PonPortIdx = 0;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;	
	STATUS ret = 0;

	IFM_ParseSlotPort( argv[0], &ul_slot, &ul_port );

	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		vty_out(vty,"  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
    	
    if(argc == 2)
    {
	    VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
	    VOS_MemCpy(entry.authEntry.serial_no, argv[1], GPON_ONU_SERIAL_NUM_STR_LEN-1);	    	    
    
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
	else if(argc == 1)
	{
		ULONG brdIdx,portIdx,Index=0,NextBrdIdx,NextPortIdx,NextIndex=0;
		brdIdx = ul_slot;
		portIdx = ul_port;
		do{
			
			Index = NextIndex;

			mn_getNextGponOnuAuthEntryIdx(brdIdx,portIdx,Index, &NextBrdIdx, &NextPortIdx, &NextIndex);
			entry.authIdx = Index;
			entry.authRowStatus = RS_DESTROY;
			if(Index!=0)
			ret=OLT_SetGponAuthEntry(PonPortIdx,  entry );
		}while(( ret == VOS_OK ) && ( brdIdx == NextBrdIdx ) && ( portIdx == NextPortIdx ) );
	}
	else
	{
	/*do nothing*/
	}
    return( CMD_SUCCESS);
}
#endif
#ifdef  PON_PORT_ADMIN_STATUS
#endif

/* added by chenfj 2007-6-27  
     PON 端口管理使能: up / down  reset */
/*  modified by chenfj 2008-5-23
     PON端口的禁止和使能,不再判断PON端口的当前状态;可直接设置,
     即支持予配置(shutdown pon port / undo shutdown pon port)
    */
DEFUN (
	shutdown_pon_port,
	shutdown_pon_port_cmd,
	"shutdown pon port <portlist>",
	"shutdown the pon port\n"
	"shutdown the pon port\n"
	"shutdown the pon port\n"
	"please input the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;

	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	  	 RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	  	 continue;
		}
		/*
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( PonPortIsWorking( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		ShutdownPonPort( phyPonId );
					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}


DEFUN (
	undo_shutdown_pon_port,
	undo_shutdown_pon_port_cmd,
	"undo shutdown pon port <portlist>",
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	"no shutdown the pon port\n"
	"please input the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;

	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
	
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	   	RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	        continue;
		}
		/* 
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( getPonChipInserted((unsigned char)(ulSlot),(unsigned char) (ulPort) ) != PONCHIP_EXIST )
			{
			vty_out(vty,"  %% pon %d/%d is not exist\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		StartupPonPort( phyPonId );
					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		


	return CMD_SUCCESS;
}

#if 0
DEFUN (
	reset_pon_port,
	reset_pon_port_cmd,
	"reset pon port <portlist>",
	"reset the pon port\n"
	"reset the pon port\n"
	"reset the pon port\n"
	"please input the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;

	if( argc != 1 ) 
		{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
		}
		
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			return CMD_WARNING;
	
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		if ( (ulSlot <4) || (ulSlot > 8) )
		{
			vty_out( vty, "  %% Parameter error.slot range is 4-8\r\n");
			return CMD_WARNING;
		}
		if ((ulPort > 4) || (ulPort < 1))
		{
			vty_out( vty, "  %% Parameter error.port range is 1-4\r\n");
			return CMD_WARNING;
		}
		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			return CMD_WARNING;
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
		    	return CMD_WARNING;
			}
			else
				continue;
		}

		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
			{
			if( PonPortIsWorking( phyPonId ) != TRUE )
				{
				vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
				continue;
				}
			}
		
		ResetPonPort( phyPonId );							
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}
#endif

#ifdef  ONU_DEVICE_DEACTIVATE 
#endif
	/* added by chenfj 2007-6-27 
	    ONU	 device  de-active */

/* added by xieshl 20111011, 统一去激活ONU命令实现；不管CTC协议栈是否打开，去激活ONU时都不再依据ONU认证功能实现，
    而是直接将ONU放入pending队列，以避免ONU反复离线注册。问题单13516 */
DEFUN (
	deactivate_onu_device,
	deactivate_onu_device_cmd,
	"deactivate onu traffic service <slot/port/onuid>",
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
    	/*INT16 phyPonId = 0;
	int result = 0;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );

	SWAP_TO_ACTIVE_PON_PORT(ulSlot, ulPort)

	if( lRet != VOS_OK )
		return CMD_WARNING;

/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
	/*added by wutongwu at 18 October*/
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return CMD_WARNING;
#if 0
	if ((ulSlot<4)  || (ulSlot>8))
		{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
		}
	
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		/*
		if ( CLI_EPON_CARDINSERT != GetOltCardslotInserted( ulSlot-1 ))
			{
			vty_out( vty, "  %% Can not find slot %d. \r\n",ulSlot);
			return CMD_WARNING;
			}
		*/

		if (ulSlot == 4)
			{
			if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
				{
				vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", ulSlot, MODULE_E_EPON3_SW_NAME_STR);
				return CMD_WARNING;
				}
			}
		}
	
	if ( (ulPort < 1) || (ulPort > 4) )
		{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
		return CMD_WARNING;
		}
#endif
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}	
	/*
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( PonPortIsWorking( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING);
			}
		}
	*/
	if(ThisIsValidOnu(phyPonId,(ulOnuid -1)) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
		}

	/*if( V2R1_CTC_STACK )*/
	if(GetOnuVendorType( phyPonId, (ulOnuid -1)) == ONU_VENDOR_CT )
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
	undo_deactivate_onu_device,
	undo_deactivate_onu_device_cmd,
	"undo deactivate onu traffic service <slot/port/onuid>",
	NO_STR
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
    	/*INT16 phyPonId = 0;
	int result = 0;*/
	LONG lRet;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );

	SWAP_TO_ACTIVE_PON_PORT(ulSlot, ulPort)

	if( lRet != VOS_OK )
		return CMD_WARNING;


/* modified by xieshl 20111011, 统一去激活ONU命令实现，问题单13516 */
#if 0
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return CMD_WARNING;
#if 0
	if ((ulSlot<4)  || (ulSlot>8))
		{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
		}
	
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		/*
		if ( CLI_EPON_CARDINSERT != GetOltCardslotInserted( ulSlot-1 ))
			{
			vty_out( vty, "  %% Can not find slot %d. \r\n",ulSlot);
			return CMD_WARNING;
			}
		*/

		if (ulSlot == 4)
			{
			if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
				{
				vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", ulSlot, MODULE_E_EPON3_SW_NAME_STR);
				return CMD_WARNING;
				}
			}
		}
	
	if ( (ulPort < 1) || (ulPort > 4) )
		{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
		return CMD_WARNING;
		}
#endif
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}	
	/*
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( PonPortIsWorking( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			return( CMD_WARNING);
			}
		}
	*/
	if(ThisIsValidOnu(phyPonId,(ulOnuid -1)) != ROK )
		{
		vty_out( vty, "  %% onuid%d/%d/%d is not exist. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
		}

	/*if( V2R1_CTC_STACK )*/
	if(GetOnuVendorType( phyPonId, (ulOnuid -1)) == ONU_VENDOR_CT )
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
	show_deactivate_onu_device,
	show_deactivate_onu_device_cmd,
	"show deactivate onu {<slot/port>}*1",
	"show deactivate onu\n"
	"show deactivate onu\n"
	"show deactivate onu\n"
	"please input the slot/port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    INT16 phyPonId = 0;
	short int PonPortIdx=0;

	unsigned char i, flag, total_flag;

	if( argc == 1 )
	{
		IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
        
		/* 对Slot的范围进行判断，超出范围内的提示错误，返回。*/
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;
#if 0
		if ((ulSlot<4)  || (ulSlot>8))
			{
			vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
			return CMD_WARNING;
			}
		
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
			{
			if (ulSlot == 4)
				{
				if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
					{
					vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", ulSlot, MODULE_E_EPON3_SW_NAME_STR);
					return CMD_WARNING;
					}
				}
			}
		
		if ( (ulPort < 1) || (ulPort > 4) )
			{
			vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
			return CMD_WARNING;
			}
#endif
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}

		if(PonPortIsWorking(phyPonId) != TRUE)
		{
			vty_out(vty,"pon%d/%d is not working\r\n", ulSlot, ulPort);
			return(RERROR);
		}
			
		i = 0;
        flag = 0;
		for( ulOnuid = 0; ulOnuid< MAXONUPERPON; ulOnuid++)
		{
			if( GetOnuTrafficServiceEnable( phyPonId, ulOnuid ) == V2R1_DISABLE )
			{
				if( flag == 0 )
				{
					flag = 1 ;
					vty_out(vty, "  pon %d/%d deactivate onu list\r\n", ulSlot, ulPort );
				}
				vty_out(vty, "  %2d ", (ulOnuid+1) );
				i++;
				if(( i % 16 ) == 0 ) vty_out(vty,"\r\n");
			}
		}
		
		if( flag == 0 ) vty_out(vty, "  pon %d/%d has no deactivate onu\r\n", ulSlot, ulPort );
		else if(( i % 16 ) !=  0 ) vty_out(vty,"\r\n");
		
	}
	else                         /*问题单11280*/
	{
	    total_flag = 0;
		for(ulSlot = PONCARD_FIRST; ulSlot <= PONCARD_LAST;ulSlot ++)
		{
			if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK)
				continue;
			for( ulPort = 1; ulPort <= PONPORTPERCARD; ulPort ++)
			{
				i = 0;
				flag = 0;
				
				PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
				if (PonPortIdx == VOS_ERROR) continue;
				
				if(PonPortIsWorking(PonPortIdx) != TRUE)
				{
					continue;
				}
				
				for( ulOnuid = 0; ulOnuid< MAXONUPERPON; ulOnuid++)
				{
					if( GetOnuTrafficServiceEnable( PonPortIdx, ulOnuid ) == V2R1_DISABLE )
					{
						if( flag == 0 )
						{
						    total_flag = 1;
                            
							flag = 1 ;
							vty_out(vty, "  pon %d/%d deactivate onu list\r\n", ulSlot, ulPort );
						}
						vty_out(vty, "  %2d ", (ulOnuid+1) );
						i++;
						if(( i % 16 ) == 0 ) vty_out(vty,"\r\n");
					}
				}
				if(( i % 16) != 0 ) vty_out(vty,"\r\n");
			}		
		}	

		if( total_flag == 0 ) vty_out(vty, "  no deactivate onu\r\n");
	}

    return CMD_SUCCESS;
}

/* added by xieshl 20110519, 自动删除不在线ONU PR11109 */
extern ULONG g_onu_agingtime;
extern const ULONG g_onu_agingtime_default;
extern LONG onuAgingTimerId;
DEFUN  (
    olt_onu_agingtime_config,
    olt_onu_agingtime_config_cmd,
    "config onu-agingtime [0|<10-525600>]",
    "config offline onu\n"
    "offline onu agingtime\n"
    "never be aged\n"
    "range 10~525600 minutes, default 21600 minutes (15 days)\n"
    )
{
	LONG time_len = VOS_AtoL(argv[0]);

	if( (time_len == 0) || ((time_len >= 10) && (time_len <= 525600)) )		/* modified by xieshl 20111118, 默认使能打开，时间改为15天 */
	{
		time_len = time_len * 60;
		/*if( time_len != 0 )
		{
			if( VOS_TimerChange( MODULE_OLT, onuAgingTimerId, time_len * MINUTE_1 ) == VOS_ERROR )
			{
				vty_out( vty, "Config onu agingtime ERR\r\n" );
				return CMD_WARNING;
			}
		}*/
		g_onu_agingtime = time_len;
	}
	return CMD_SUCCESS;
}
/*  需求26975 */
DEFUN  (
    olt_onu_agingtime_day_config,
    olt_onu_agingtime_day_config_cmd,
    "config onu-agingtime days <1-365>",
    "config offline onu\n"
    "offline onu agingtime\n"
    "agingtime unit:day\n"
    "input the num of days.\n"
    )
{
	LONG time_len = VOS_AtoL(argv[0]);

	if( (time_len >= 1)  && (time_len <= 365) )		
	{
		g_onu_agingtime = time_len * (24 * 60 * 60);
	}
	return CMD_SUCCESS;
}

DEFUN  (
    olt_onu_agingtime_show,
    olt_onu_agingtime_show_cmd,
    "show onu-agingtime",
    "show offline onu agingtime\n"
    "show offline onu agingtime\n"
    )
{
	vty_out( vty, " aging time: " );
	if( g_onu_agingtime )
	{
		if( g_onu_agingtime % (24 * 60 * 60) == 0 )
			vty_out( vty, "%d days\r\n", g_onu_agingtime / (24 * 60 * 60) );
		else
			vty_out( vty, "%d minutes\r\n", g_onu_agingtime / 60 );
	}
	else
	{
		vty_out( vty, "0 (never)\r\n" );
	}
	return CMD_SUCCESS;
}

/* added by xieshl 20110602, 自动删除mac地址相同的离线ONU 增加使能开关 */
extern ULONG g_onu_repeated_del_enable;
DEFUN  (
    olt_onu_repeated_del_config,
    olt_onu_repeated_del_config_cmd,
    "config onu-repeated-del [enable|disable]",
    "config onu-repeated\n"
    "config onu-repeated delete enable\n"
    "delete repeated onu\n"
    "don't delete repeated onu\n"
    )
{
	ULONG enable;
	UCHAR *str[] = { "disable", "enable" };
	
	if( argv[0][0] == 'e' )
		enable = 1;
	else
		enable = 0;
	
	if( enable != g_onu_repeated_del_enable )
	{
		OnuMgtSync_OnuRepeatedDelEnable(enable);
	}
	else
	{
		vty_out( vty, "onu-repeated delete has already %s\r\n", str[enable] );
	}
	return CMD_SUCCESS;
}
DEFUN  (
    olt_onu_repeated_del_show,
    olt_onu_repeated_del_show_cmd,
    "show onu-repeated-del",
    "show onu-repeated\n"
    "show onu-repeated delete enable\n"
    )
{
	vty_out( vty, " onu-repeated delete %s\r\n", g_onu_repeated_del_enable ? "enable" : "disable" );
	return CMD_SUCCESS;
}

#ifdef  PON_BER_FER_ALARM 
#endif
	/* added by chenfj 2007-7-31 
	    PON端口BER / FER 告警 */

DEFUN (
	pon_port_ber_alarm_threshold,
	pon_port_ber_alarm_threshold_cmd,
	"pon-port ber alarm threshold <0-8> min-error-bytes <1-20000>",
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	"ber measurement threshold(value as:1.00e-0=1, 1.00e-1=0.1, 1.00e-8=0.00000001)\n"
	"minimum error bytes threshold\n"
	"the minimal number of error bytes required for ber alarm\r\n"
	)
{
	unsigned int ber_Threshold;
	unsigned int error_bytes;

	ber_Threshold = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	error_bytes = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	monponBERThrSet(ber_Threshold, error_bytes);

	return( CMD_SUCCESS );
}

/* B--added by liwei056@2012-8-6 for D15528 */
DEFUN (
	pon_port_ber_alarm_threshold_no,
	pon_port_ber_alarm_threshold_no_cmd,
	"undo pon-port ber alarm threshold",
    NO_STR
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	"pon port ber alarm threshold\n"
	)
{
	unsigned int ber_Threshold = 0;
	unsigned int error_bytes = 0;

    monponBERThrGet(&ber_Threshold, &error_bytes);
    if ( (DEFAULT_PON_BER_THRESHOLD_RATIO != ber_Threshold)
        || (DEFAULT_PON_BER_THRESHOLD_MINBYTE != error_bytes) )
    {
    	monponBERThrSet(DEFAULT_PON_BER_THRESHOLD_RATIO, DEFAULT_PON_BER_THRESHOLD_MINBYTE);
    }
    else
    {
    	vty_out( vty, "  %% No config to clear.\r\n");
    	return( CMD_WARNING );
    }

	return( CMD_SUCCESS );
}
/* E--added by liwei056@2012-8-6 for D15528 */

DEFUN (
	pon_port_fer_alarm_threshold,
	pon_port_fer_alarm_threshold_cmd,
	"pon-port fer alarm threshold <0-10> min-error-frames <1-2000>",
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	"fer measurement threshold(value as:1.00e-0=1, 1.00e-1=0.1, 1.00e-10=0.0000000001)\n"
	"minimum error frames threshold\n"
	"the minimal number of error frames required for fer alarm\r\n"
	)
{
	unsigned int fer_Threshold;
	unsigned  int error_frames;

	fer_Threshold = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	error_frames = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	monponFERThrSet(fer_Threshold, error_frames);

	return( CMD_SUCCESS );
	
}

/* B--added by liwei056@2012-8-6 for D15528 */
DEFUN (
	pon_port_fer_alarm_threshold_no,
	pon_port_fer_alarm_threshold_no_cmd,
	"undo pon-port fer alarm threshold",
    NO_STR
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	"pon port fer alarm threshold\n"
	)
{
	unsigned int fer_Threshold = 0;
	unsigned int error_bytes = 0;

    monponFERThrGet(&fer_Threshold, &error_bytes);
    if ( (DEFAULT_PON_FER_THRESHOLD_RATIO != fer_Threshold)
        || (DEFAULT_PON_FER_THRESHOLD_MINFRAME != error_bytes) )
    {
    	monponFERThrSet(DEFAULT_PON_FER_THRESHOLD_RATIO, DEFAULT_PON_FER_THRESHOLD_MINFRAME);
    }
    else
    {
    	vty_out( vty, "  %% No config to clear.\r\n");
    	return( CMD_WARNING );
    }

	return( CMD_SUCCESS );
}
/* E--added by liwei056@2012-8-6 for D15528 */

DEFUN (
	show_pon_port_berfer_alarm_threshold,
	show_pon_port_berfer_alarm_threshold_cmd,
	"show pon-port ber-fer alarm threshold",
	"show pon port ber/fer alarm threshold\n"
	"show pon port ber/fer alarm threshold\n"
	"show pon port ber/fer alarm threshold\n"
	"show pon port ber/fer alarm threshold\n"
	"show pon port ber/fer alarm threshold\n"
	)
{
	unsigned int fer_Threshold = 0;
	unsigned  int error_frames = 0;
	unsigned int ber_Threshold = 0;
	unsigned  int error_bytes = 0;
	short int ret;

	ret = monponBERThrGet( &ber_Threshold, &error_bytes );
	ret = monponFERThrGet( &fer_Threshold, &error_frames );

#if 0
	vty_out(vty, " ber threshold=%d(unit:1.00e-9), minimun error bytes=%d\r\n", ber_Threshold, error_bytes);
	vty_out(vty, " fer threshold=%d(unit:1.00e-11), minimun error frames=%d\r\n", fer_Threshold,error_frames);
#else
	vty_out(vty, " ber threshold=1.00e-%d, minimun error bytes=%d\r\n", ber_Threshold, error_bytes);
	vty_out(vty, " fer threshold=1.00e-%d, minimun error frames=%d\r\n", fer_Threshold,error_frames);
#endif

	return( CMD_SUCCESS );

}




DEFUN (
	onu_pon_ber_fer_alarm_enable,
	onu_pon_ber_fer_alarm_enable_cmd,
	"onu-pon [ber|fer] alarm [enable|disable] <portlist>",
	"onu pon port\n"
	"ber alarm\n"
	"fer alarm\n"
	"onu pon port ber/fer alarm\n"
	"ber/fer alarm enable\n"
	"ber/fer alarm disable\n"
	"please input the pon port,ie:7/1-3,5/1 etc.\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;
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

	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[2], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );

		SWAP_TO_ACTIVE_PON_PORT(ulSlot, ulPort)

		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );

		}
			
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
#if 0		
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		if ( (ulSlot <4) || (ulSlot > 8) )
		{
			vty_out( vty, "  %% Parameter error.slot range is 4-8\r\n");
			return CMD_WARNING;
		}
		if ((ulPort > 4) || (ulPort < 1))
		{
			vty_out( vty, "  %% Parameter error.port range is 1-4\r\n");
			return CMD_WARNING;
		}
#endif
		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[2], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );

		SWAP_TO_ACTIVE_PON_PORT(ulSlot, ulPort)

		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	   continue;
		}
		/*
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		if( Ber_Fer_flag == 1 )
			ret = monOnuBerAlmEnSet( phyPonId, enable );
		else ret = monOnuFerAlmEnSet( phyPonId, enable );

		if(( ret != ROK ) && ( count == 1))
			{
			vty_out(vty,"  %% Executing error \r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}


DEFUN (
	show_onu_pon_ber_fer_alarm,
	show_onu_pon_ber_fer_alarm_cmd,
	"show onu-pon ber-fer alarm enable <portlist>",
	"show onu pon port ber/fer alarm enable\n"
	"show onu pon port ber/fer alarm config\n"
	"show onu pon port ber/fer alarm config\n"
	"show onu pon port ber/fer alarm config\n"
	"show onu pon port ber/fer alarm config\n"
	"please input the pon port,ie:7/1-3,5/1 etc\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;
	unsigned int Ber_flag = V2R1_DISABLE;
	unsigned int Fer_flag = V2R1_DISABLE;

	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			

		/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
#if 0
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		if ( (ulSlot <4) || (ulSlot > 8) )
		{
			vty_out( vty, "  %% Parameter error.slot range is 4-8\r\n");
			return CMD_WARNING;
		}
		if ((ulPort > 4) || (ulPort < 1))
		{
			vty_out( vty, "  %% Parameter error.port range is 1-4\r\n");
			return CMD_WARNING;
		}
#endif
		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	   continue;
		}
		/*
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		monOnuBerAlmEnGet( phyPonId, &Ber_flag );
		monOnuFerAlmEnGet( phyPonId, &Fer_flag );
		if( Ber_flag > 2 ) Ber_flag = 0;
		if( Fer_flag > 2 ) Fer_flag = 0;
		vty_out(vty, "  onu pon-port %d/%d  ber %s, fer %s\r\n", ulSlot, ulPort, v2r1Enable[Ber_flag], v2r1Enable[Fer_flag]);					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}

DEFUN (
	olt_pon_ber_fer_alarm_enable,
	olt_pon_ber_fer_alarm_enable_cmd,
	"olt-pon [ber|fer] alarm [enable|disable] <portlist>",
	"olt pon port\n"
	"ber alarm\n"
	"fer alarm\n"
	"olt pon port ber/fer alarm\n"
	"ber/fer alarm enable\n"
	"ber/fer alarm disable\n"
	"please input the pon port,ie:7/1-3,5/1 etc.\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;
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

	
	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[2], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			

		/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
#if 0
		/*对Slot的范围进行判断，超出范围内的提示错误，返回。Port的范围就不必判断了，平台里已经做好了。*/
		if ( (ulSlot <4) || (ulSlot > 8) )
		{
			vty_out( vty, "  %% Parameter error.slot range is 4-8\r\n");
			return CMD_WARNING;
		}
		if ((ulPort > 4) || (ulPort < 1))
		{
			vty_out( vty, "  %% Parameter error.port range is 1-4\r\n");
			return CMD_WARNING;
		}
#endif

		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[2], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	   continue;
		}
		/*
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		if( Ber_Fer_flag == 1 )
			ret = monPonBerAlmEnSet( phyPonId, enable );
		else ret = monPonFerAlmEnSet( phyPonId, enable );

		if(( ret != ROK ) && ( count == 1))
			{
			vty_out(vty,"  %% Executing error \r\n");
			RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}

DEFUN (
	show_olt_pon_ber_fer_alarm,
	show_olt_pon_ber_fer_alarm_cmd,
	"show olt-pon ber-fer alarm enable <portlist>",
	"show olt pon port ber/fer alarm enable\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	"show olt pon port ber/fer alarm config\n"
	"please input the pon port,ie:7/1-3,5/1 etc\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
    	INT16 phyPonId = 0;
	int count = 0;
	ULONG ulIfIndex=0;
	LONG lRet = 0;
	unsigned int Ber_flag = V2R1_DISABLE;
	unsigned int Fer_flag = V2R1_DISABLE;

	/*ulPort : 1-4*/
	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}

		/*对Slot的范围进行判断，超出范围内的提示错误，返回。*/
		/*if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return CMD_WARNING;*/
		if( olt_pon_idx_check(ulSlot, ulPort, vty) == VOS_ERROR )		/* modified by xieshl 20110707, 问题单13231 */
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
 		count++;
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
		

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		lRet = ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
		{
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
		}
			
	
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		#ifdef CLI_EPON_DEBUG
		    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
		#endif  
			if(1 == count)
			{
		    	vty_out( vty, "  %% Executing error \r\n");
         	   RETURN_PARSE_PORT_LIST_TO_IFINDEX_ETH( CMD_WARNING );
			}
			else
         	   continue;
		}
		/*
		if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if( OLTAdv_IsExist( phyPonId ) != TRUE )
			{
			vty_out(vty,"  %% pon %d/%d is not working\r\n", ulSlot, ulPort );
			continue;
			}
		}
		*/
		monPonBerAlmEnGet( phyPonId, &Ber_flag );
		monPonFerAlmEnGet( phyPonId, &Fer_flag );
		vty_out(vty, "  olt pon-port %d/%d  ber %s, fer %s\r\n", ulSlot, ulPort, v2r1Enable[Ber_flag], v2r1Enable[Fer_flag]);					
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()

	return CMD_SUCCESS;
}

/*----added by wangjiah@2016-08-12 to support 8xep pon chip reset : begin---*/
int do_olt_reset_chip(struct vty *vty)
{
	INT16 phyPonId;

    phyPonId = VTY_CONFIRM_CB_GETPARAM1(vty);
    if ( OLT_LOCAL_ISVALID(phyPonId) )
    {
        if ( 0 == OLT_ResetPonChip(phyPonId) )
        {
        	vty_out(vty,"  reset is complete\r\n");
        }
        else
        {
    		vty_out(vty, "  %% pon%d is resetting. please wait a little.\r\n", GetCardIdxByPonChip(phyPonId) );
    		return( CMD_WARNING );	
        }
    }
    else
    {
		vty_out(vty, "  %% invalid pon-id(%d).\r\n", phyPonId);
		return( CMD_WARNING );	
    }

	return CMD_SUCCESS;
}
/*----added by wangjiah@2016-08-12 : end---*/

int do_olt_reset(struct vty *vty)
{
	INT16 phyPonId;

    phyPonId = VTY_CONFIRM_CB_GETPARAM1(vty);
    if ( OLT_LOCAL_ISVALID(phyPonId) )
    {
        if ( 0 == OLT_ResetPon(phyPonId) )
        {
        	vty_out(vty,"  reset is complete\r\n");
        }
        else
        {
    		vty_out(vty, "  %% pon%d/%d is resetting, please wait a little.\r\n", GetCardIdxByPonChip(phyPonId), GetPonPortByPonChip(phyPonId));
    		return( CMD_WARNING );	
        }
    }
    else
    {
		vty_out(vty, "  %% invalid pon-id(%d).\r\n", phyPonId);
		return( CMD_WARNING );	
    }

	return CMD_SUCCESS;
}

/*extern int updateSlotEepromInfo( ULONG slotno );*/
DEFUN (
	reset_pon_chip,
	reset_pon_chip_cmd,
	"reset pon <slot/port>",
	"reset pon\n"
	"reset pon. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	"input the slot/port. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	int PonChipPonNum;    
	int PonChipType;    
	/*LONG lRet = 0;*/
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	/* 1 板在位检查
	if( CDSMS_CHASSIS_SLOT_INSERTED(ulSlot) ) != TRUE )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return CMD_WARNING;
		}
	*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
		}*/
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)
	{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
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

		if(PonChipType == PONCHIP_BCM55538)
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
	Hardware_Reset_olt1(ulSlot, ulPort, 1, 0);	

	vty_out(vty,"  reset pon%d/%d complete\r\n", ulSlot, ulPort);
	Hardware_Reset_olt2(ulSlot, ulPort, 1, 0);

	ClearPonRunningData( phyPonId );
	SetPonChipResetFlag(ulSlot, ulPort);
	Add_PonPort( phyPonId );	
#else
    if ( 0 != OLT_ResetPon(phyPonId) )
    {
		vty_out(vty, "  %% pon%d/%d is resetting, please wait a little.\r\n", ulSlot, ulPort);
		return( CMD_WARNING );	
    }
#endif
	
	return CMD_SUCCESS;
}

DEFUN (
	reset_10gepon_chip,
	reset_10gepon_chip_cmd,
	"reset 10gepon-chip <slot>",
	"reset 10gepon-chip\n"
	"reset 10gepon-chip. NOTE:if this pon is configed to auto-protect and status currently is passive,the fiber shoule be pulled off;otherwise ONUs will be deregistered\n"
	"input the slot\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 1;
	int PonChipPonNum;    
	int PonChipType;    
	/*LONG lRet = 0;*/
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	ulSlot = (ULONG)VOS_AtoL(argv[0]);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)
	{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
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

#if 0 /*merge this cmd into reset pon <slot/port>*/
/* modified by wangjiah@2016-08-12 to support pon port reset issue-23072: begin*/
DEFUN (
	reset_pon_port,
	reset_pon_port_cmd,
	"reset pon port <slot/port>",
	"reset the pon port\n"
	"reset the pon port\n"
	"reset the pon port\n"
	"please input the pon port\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	int PonChipPonNum;    
	int PonChipType;    
	/*LONG lRet = 0;*/
	INT16 phyPonId = 0;
	INT16 phyPonIds[MAXOLTPERPONCHIP];

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}

	/* pon chip is inserted ? */
	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)
	{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(CMD_WARNING);
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
		if(PonChipType == PONCHIP_BCM55538)
		{
			vty_out(vty, "  %% are you sure to reset the pon port(%d/%d)? [Y/N]", ulSlot, ulPort);
		}
		else
		{
			vty_out(vty, "  %% this is %s card, reset single pon port is not supported\r\n", pon_chip_type2name(PonChipType));
			return CMD_WARNING;
		}

		vty->prev_node = vty->node;
		vty->node = CONFIRM_ACTION_NODE;
		vty->action_func = do_olt_reset;
        VTY_CONFIRM_CB_SETPARAM1(vty, phyPonId);
        
		return CMD_SUCCESS;    
    }
}
/* modified by wangjiah@2016-08-12 to support pon port reset : end*/
#endif

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
DEFUN (
	upgrade_tdm,
	upgrade_tdm_cmd,
	"upgradetdm ftp [app|fpga] <A.B.C.D> <username> <password> <filename>",
	"software or firmware download instruction\n"
	"download by ftp protocol\n"
	"download software for TDM\n"
	"download FPGA for TDM\n"
	"ftp server ip address\n"
	"user name on the ftp server\n"
	"password for the user\n"
	"filename of the file to be download\n"
	)
{
	static ftpActTerm term;
	struct in_addr ia;
	ULONG msgItem[4]={0};
	long msgId = getFtpTaskQueId();

	if( get_gfa_tdm_slotno() == 0 )
	{
		vty_out( vty, "Sorry, not find the %s board\r\n", GetGFA6xxxTdmNameString());
		return CMD_WARNING;
	}

	VOS_MemZero( &term, sizeof(ftpActTerm));
	VOS_StrCpy(term.loadtype, argv[0]);
	VOS_StrCpy(term.userName, argv[2]);
	VOS_StrCpy(term.userpassword, argv[3]);
	VOS_StrCpy(term.filename, argv[4]);
	VOS_StrCpy(term.loadtarget, "tdm");

	vos_inet_aton(argv[1], &ia);
	term.ipAddr = ia.s_addr;
	term.actCode = 2;

	msgItem[0] = MODULE_RPU_TDM_MGMT;
	msgItem[1] = FC_FTP_LOAD;
	msgItem[2] = (ULONG)&term;
	msgItem[3] = (ulong)vty;

	if(VOS_QueSend(msgId, msgItem, 200, MSG_PRI_NORMAL) == VOS_OK)	
		return CMD_SUCCESS;
	else
		return CMD_WARNING;
	
}
#endif
#ifdef ONU_UPDATE
#endif
/*add by shixh20090518*/
#if 0	/* removed by xieshl 20120629, 和app等文件采用一致的处理方式，在ftpclient中实现，问题单14828 */
/*问题单11480*/
DEFUN (
	download_sysfile,
	download_sysfile_cmd,
	"download ftp [sysfile] <A.B.C.D> <user> <pass> <filename>",
	"software or firmware download instruction\n"
	"download by ftp protocol\n"
	"download software for systerm file\n"
	"ftp server ip address\n"
	"user name on the ftp server\n"
	"password for the user\n"
	"filename of the file to be download\n"
	)
{
	static ftpActTerm term;
	struct in_addr ia;
	ULONG msgItem[4]={0};
	long msgId = getFtpTaskQueId();
	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return CMD_SUCCESS;

	/*if( get_gfa_tdm_slotno() == 0 )
	{
		vty_out( vty, "Sorry, not find the %s board\r\n", GetGFA6xxxTdmNameString());
		return CMD_WARNING;
	}*/
       
	VOS_MemZero( &term, sizeof(ftpActTerm));
    
       if(VOS_StrCmp(argv[0],"sysfile")==0)
	    VOS_StrCpy(term.loadtype, SYSFILE_INI);
        
	VOS_StrCpy(term.userName, argv[2]);
	VOS_StrCpy(term.userpassword, argv[3]);
	VOS_StrCpy(term.filename, argv[4]);
	VOS_StrCpy(term.loadtarget, SYSFILE_INI);

	vos_inet_aton(argv[1], &ia);
	term.ipAddr = ia.s_addr;
	term.actCode = 2;

	msgItem[0] = MODULE_RPU_TDM_MGMT;
	msgItem[1] = FC_FTP_LOAD;
	msgItem[2] = (ULONG)&term;
	msgItem[3] = (ulong)vty;

	if(VOS_QueSend(msgId, msgItem, 200, MSG_PRI_NORMAL) == VOS_OK)	
		return CMD_SUCCESS;
	else
		return CMD_WARNING;
	
}
#endif
#if 0   /*此命令放到ftpclient.c文件中实现*/
extern int uploadSysfile(char *host,char *user,char *pwd, char *file_type,struct vty *vty);

DEFUN (
	upload_sysfile,
	upload_sysfile_cmd,
	"upload ftp sysfile <A.B.C.D> <username> <password> <filetype>",
	"software or firmware download instruction\n"
	"download by ftp protocol\n"
	"download software for systerm file\n"
	"ftp server ip address\n"
	"user name on the ftp server\n"
	"password for the user\n"
	"sys file name\n"
	)
{
	/*struct in_addr ia;

	vos_inet_aton(argv[0], &ia);*/

	if(uploadSysfile(argv[0], argv[1], argv[2],argv[3],vty)==VOS_ERROR)
	 	return VOS_ERROR;
	 
	return  VOS_OK;
}
#endif


/* 运行此命令，则会启动当前flash 中的系统文件sysfile  */
DEFUN (
	active_sysfile,
	active_sysfile_cmd,
	"active sysfile",
	"config sysfile to running database\n"
	"config sysfile to running database\n")
{
	
	if(xflash_sysfile_exist() != VOS_OK)
	{
		vty_out(vty, "\r\nthere is no sysfile in flash\r\n"); 
		return CMD_WARNING;
	}
	
	InitMgmtInfoBySysfile(vty);
	InitProductTypeArray();

       Sysfile_Active_SYNC_All();
       
	return CMD_SUCCESS;
}

DEFUN (
	show_sysfile,
	show_sysfile_cmd,
	"show sysfile",
	"display sysfile\n"
	"display sysfile\n")
{
	LONG  length=0;
	int file_head_len=0;
	char *pfile;
	
	if(xflash_sysfile_exist() != VOS_OK)
	{
		vty_out(vty, "\r\nthere is no sysfile in flash\r\n"); 
		return CMD_WARNING;
	}

	pfile = g_malloc(DEVICE_FLASH_SYNINFO_MAXLEN+10);
	if(pfile == NULL)
	{
		vty_out(vty, "\r\nmalloc mem for file-buffer err\r\n");
		return(CMD_WARNING);
	}
	memset(pfile, 0x0, DEVICE_FLASH_SYNINFO_MAXLEN+10);
	
	device_sysfile_read_flash_to_mem_deflate(pfile, &length);

	file_head_len =  sizeof(app_desc_t);
	if(length > file_head_len)
	{
		vty_out(vty, "\r\n");
		length -= file_head_len;
		vty_big_out(vty, length, "%s", &pfile[file_head_len]);
		vty_out(vty, "\r\n");
	}

	g_free(pfile);
	
	return CMD_SUCCESS;
}



/* added by xieshl 20090212, 看门狗使能默认不启动，只能通过命令启动 */
DEFUN( v2r1_watchdog_enable_fun,
       v2r1_watchdog_enable_cmd,
       "config watchdog [enable|disable]",
       DescStringCommonConfig
       "Config watchdog's setting\n"
	"Set watchdog enable\n"
	"Set watchdog disable\n"
       )
{
	/*if( V2R1_watchdog_init_flag != V2R1_ENABLE )
	{
		vty_out( vty, " Watchdog is not inited\r\n" );
		return CMD_WARNING;
	}*/
	if( VOS_StrCmp( argv[0], "enable") == 0 )
	{
		if( V2R1_watchdog_startup_flag == V2R1_ENABLE )
		{
			vty_out( vty, " Watchdog is enabled already\r\n" );
		}
		else
		{
			VOS_SemTake( OltMgmtDataSemId, WAIT_FOREVER );
			V2R1_watchdog_startup_flag = V2R1_ENABLE;
			VOS_SemGive( OltMgmtDataSemId );
			
			/* 在配置数据恢复时不能使能看门狗 */
			if( SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
				V2R1_enable_watchdog();
		}
	}
	else
	{
		V2R1_disable_watchdog_forced();

		if( V2R1_watchdog_startup_flag == V2R1_DISABLE )
		{
			vty_out( vty, " Watchdog is disabled already\r\n" );
		}
		else
		{
			VOS_SemTake( OltMgmtDataSemId, WAIT_FOREVER );
			V2R1_watchdog_startup_flag = V2R1_DISABLE;
			VOS_SemGive( OltMgmtDataSemId );
		}
	}
	return CMD_SUCCESS;
}

DEFUN( v2r1_watchdog_show_fun,
       v2r1_watchdog_show_cmd,
       "show watchdog",
       DescStringCommonShow
       "Show watchdog's setting\n"
       )
{
	if( V2R1_watchdog_startup_flag == V2R1_ENABLE )
		vty_out( vty, " Watchdog is enable\r\n" );
	else	
		vty_out( vty, " Watchdog is disable\r\n" );
	return CMD_SUCCESS;
}

DEFUN( v2r1_cpld_debug_show,
       v2r1_cpld_debug_show_cmd,
       "show cpld-register <1-32>",
       DescStringCommonShow
       "Show cpld's setting\n"
       "Input cpld register address\n"
       )
{
	unsigned char data = 0;
	ULONG reg = VOS_AtoL(argv[0]);
	ReadCPLDReg((unsigned char *)reg, &data);
	vty_out( vty, " REG(%d):0x%02x\r\n", reg, data );
	return CMD_SUCCESS;
}

DEFUN( v2r1_ONU_default_bandwidth,
       v2r1_ONU_default_bandwidth_cmd,
       "onu default-bandwidth uplink <64-10000000> <64-10000000> {downlink [0|<64-10000000>] [0|<64-10000000>]}*1 {[10g/1g|10g/10g|gpon]}*1",	/* 问题单10768 */
       DescStringCommonConfig
       "onu default bandwidth\n"
       "uplink direction\n"
       "assured-bw\n"
       "best-effort-bw\n"
       "downlink direction\n"
       "not occupy assured-bw\n"
       "assured-bw\n"
       "no policer\n"       
       "best-effort-bw\n"       
	   "set 10G/1G asymmetric onu default-bandwidth\n"
	   "set 10G/10G symmetric onu default-bandwidth\n"
	   "set GPON onu default-bandwidth\n"
       )
{
	int  assured_bw, best_effort_bw;
    ONU_bw_t default_bw;
	unsigned char *mode = NULL;
    
	assured_bw = VOS_AtoL(argv[0]);
	best_effort_bw = VOS_AtoL(argv[1]);
	
	if(assured_bw > best_effort_bw)
	{
		vty_out(vty, " uplink best-effort-bw must be greater than or equal to assured-bw\r\n");
		return(CMD_WARNING);
	}

    VOS_MemZero(&default_bw, sizeof(ONU_bw_t));
	default_bw.bw_rate = PON_RATE_NORMAL_1G;
    default_bw.bw_direction = OLT_CFG_DIR_UPLINK;
    default_bw.bw_gr = assured_bw;
    default_bw.bw_be = best_effort_bw;

	if( 4 <= argc)
	{
		assured_bw = VOS_AtoL(argv[2]);
		best_effort_bw = VOS_AtoL(argv[3]);
	
		if(assured_bw > best_effort_bw)
		{
			vty_out(vty, " downlink best-effort-bw must be greater than or equal to assured-bw\r\n");
			return(CMD_WARNING);
		}

        default_bw.bw_direction = OLT_CFG_DIR_BOTH;
        default_bw.bw_fixed   = assured_bw;
        default_bw.bw_actived = best_effort_bw;
	}	

	if(3 == argc || 5 == argc)//for 10g/1g and GPON onu limit
	{
		if(3 == argc)
		{
			mode = (unsigned char *)argv[2];
		}
		else if(5 == argc)
		{
			mode = (unsigned char *)argv[4];
		}

		if(0 == VOS_StrCmp(mode,"10g/10g"))
		{
			default_bw.bw_rate = PON_RATE_10_10G;
		}
		else if(0 == VOS_StrCmp(mode, "10g/1g"))
		{
			default_bw.bw_rate = PON_RATE_1_10G;
			if(default_bw.bw_be > 1000000)
			{
				vty_out(vty, " 10g/1g uplink best-effort-bw can't excced 1000M\r\n");
				return (CMD_WARNING);
			}
		}
		else if(0 == VOS_StrCmp(mode, "gpon"))
		{
			default_bw.bw_rate = PON_RATE_1_2G;
			if(default_bw.bw_be > 1000000)
			{
				vty_out(vty, " GPON uplink best-effort-bw can't excced 1000M\r\n");
				return (CMD_WARNING);
			}

			if(default_bw.bw_actived > 2000000)
			{
				vty_out(vty, " GPON downlink best-effort-bw can't excced 2000M\r\n");
				return (CMD_WARNING);
			}
		}
	
	}
	else // for 1g/1g onu limit 
	{
		if(default_bw.bw_be > 1000000)
		{
			vty_out(vty, " uplink best-effort-bw can't excced 1000M\r\n");
			return (CMD_WARNING);
		}

		if(default_bw.bw_actived > 1000000)
		{
			vty_out(vty, " downlink best-effort-bw can't excced 1000M\r\n");
			return (CMD_WARNING);
		}
	}

    SetOnuDefaultBW(&default_bw, vty);
    
	return CMD_SUCCESS;
}

DEFUN( v2r1_ONU_default_bandwidth2,
       v2r1_ONU_default_bandwidth_undo,
       "undo onu default-bandwidth {[10g/1g|10g/10g|gpon]}*1",
        NO_STR
        "Config the onu software\n"
        "Config onu default bandwidth\n"
        "Config 10G/1G onu default bandwidth\n"
        "Config 10G/10G onu default bandwidth\n"
        "Config GPON onu default bandwidth\n"
       )
{
    ONU_bw_t default_bw;
    
    VOS_MemZero(&default_bw, sizeof(ONU_bw_t));
    default_bw.bw_direction = OLT_CFG_DIR_BOTH;
   	if(1 == argc)
	{
		if(0 == VOS_StrCmp(argv[0], "10g/1g"))
		{
			default_bw.bw_gr = ONU_DEFAULT_BW; //7.5M
			default_bw.bw_be = ONU_DEFAULT_BE_BW; //1000M
			default_bw.bw_fixed   = GW10G_ONU_DEFAULT_BW; //75M
			default_bw.bw_actived = GW10G_ONU_DEFAULT_BE_BW; //1000M
			default_bw.bw_rate = PON_RATE_1_10G;
		}
		else if(0 == VOS_StrCmp(argv[0], "10g/10g"))
		{
			default_bw.bw_gr = GW10G_ONU_DEFAULT_BW;
			default_bw.bw_be = GW10G_ONU_DEFAULT_BE_BW;
			default_bw.bw_fixed   = GW10G_ONU_DEFAULT_BW;
			default_bw.bw_actived = GW10G_ONU_DEFAULT_BE_BW;
			default_bw.bw_rate = PON_RATE_10_10G;
		}
		else if(0 == VOS_StrCmp(argv[0], "gpon"))
		{
			default_bw.bw_gr = ONU_DEFAULT_BW;
			default_bw.bw_be = ONU_DEFAULT_BE_BW;
			default_bw.bw_fixed   = GPON_ONU_DEFAULT_BW; //15M
			default_bw.bw_actived = GPON_ONU_DEFAULT_BE_BW; //1000M
			default_bw.bw_rate = PON_RATE_1_2G;
		}
	}
	else
	{
		default_bw.bw_gr = ONU_DEFAULT_BW;
		default_bw.bw_be = ONU_DEFAULT_BE_BW;
		default_bw.bw_fixed   = ONU_DEFAULT_BW;
		default_bw.bw_actived = ONU_DEFAULT_BE_BW;
		default_bw.bw_rate = PON_RATE_NORMAL_1G;
	}

    SetOnuDefaultBW(&default_bw, vty);

	return CMD_SUCCESS;
}

DEFUN( v2r1_ONU_default_bandwidth3,
       v2r1_ONU_default_bandwidth_show,
       "show onu default-bandwidth",
       DescStringCommonShow
       "show onu default bandwidth\n"
       "show onu default bandwidth\n"
       )
{
	char string1[32], string2[32];
	vty_out(vty, "epon 1g/1g onu default-bandwidth(kbit/s):\r\n");
	VOS_Sprintf(string1,"    uplink: assured-bw:%d,", OnuConfigDefault.UplinkBandwidth);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.UplinkBandwidthBe);
	vty_out(vty,"%-30s%s\r\n", string1, string2);
	VOS_Sprintf(string1,"  downlink: assured-bw:%d,", OnuConfigDefault.DownlinkBandwidth);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.DownlinkBandwidthBe);
	vty_out(vty,"%-30s%s\r\n", string1, string2);

	vty_out(vty, "epon 10G/1G asymmetric onu default-bandwidth(kbit/s):\r\n");
	VOS_Sprintf(string1,"    uplink: assured-bw:%d,", OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM);
	vty_out(vty,"%-30s%s\r\n", string1, string2);
	VOS_Sprintf(string1,"  downlink: assured-bw:%d,", OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM);
	vty_out(vty,"%-30s%s\r\n", string1, string2);

	vty_out(vty, "epon 10G/10G symmetric onu default-bandwidth(kbit/s):\r\n");
	VOS_Sprintf(string1,"    uplink: assured-bw:%d,", OnuConfigDefault.UplinkBandwidth_XGEPON_SYM);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM);
	vty_out(vty,"%-30s%s\r\n", string1, string2);
	VOS_Sprintf(string1,"  downlink: assured-bw:%d,", OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM);
	vty_out(vty,"%-30s%s\r\n", string1, string2);

	vty_out(vty, "GPON onu default-bandwidth(kbit/s):\r\n");
	VOS_Sprintf(string1,"    uplink: assured-bw:%d,", OnuConfigDefault.UplinkBandwidth_GPON);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.UplinkBandwidthBe_GPON);
	vty_out(vty,"%-30s%s\r\n", string1, string2);
	VOS_Sprintf(string1,"  downlink: assured-bw:%d,", OnuConfigDefault.DownlinkBandwidth_GPON);
	VOS_Sprintf(string2," best-effort-bw:%d", OnuConfigDefault.DownlinkBandwidthBe_GPON);
	vty_out(vty,"%-30s%s\r\n", string1, string2);

	return CMD_SUCCESS;
}

DEFUN(
	wind_view_start_func,
	wind_view_start_cmd,
	"windview start <1-3>",
	"Wind view.\n"
	"Start wind view facility.\n"
	"Wind view record leve. 1: Context Switch. 2: Task State Transition. 3: Additional Instrumention.\n"
)
{
	int iLevel,iClass;
	extern STATUS wvUsrInit (unsigned long startAddr,unsigned long len, int iClass) ;
	extern int wvIsRunning();
	if(wvIsRunning())
	{
		vty_out(vty, "Wind view has been started.\r\n");
		return CMD_SUCCESS;	
	}
	iLevel = VOS_AtoL(argv[0]);
	switch (iLevel)
	{
		case 1:
			iClass=1;/*WV_CLASS_1*/
			break;
		case 2:
			iClass=3;/*WV_CLASS_2*/
			break;
		case 3:
			iClass=7;/*WV_CLASS_3*/
			break;
		default:
			iClass=3;
			break;
	}
	if(wvUsrInit(USER_MEM_WIND_VIEW_START_ADDR,USER_MEM_WIND_VIEW,iClass)!=0)
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;	
}

DEFUN(
	wind_view_stop_func,
	wind_view_stop_cmd,
	"windview stop",
	"Wind view.\n"
	"Stop wind view facility and save data to \"/ram/eventLog.wvr\".\n"
)
{
	extern STATUS wvStopAndUpload(char *fname);
#if VX_VERSION==55
	if(wvStopAndUpload("/ram/eventLog.wvr")!=0)
#else
	if(wvStopAndUpload("/ram:0/eventLog.wvr")!=0)
#endif
		vty_out(vty, "Stop wind view Error.\r\n");
	return CMD_SUCCESS;	
}

DEFUN(
	wind_view_show_func,
	wind_view_show_cmd,
	"show windview",
	 DescStringCommonShow
	"Show wind view information.\n"
)
{
	if(wvIsRunning())
		{
			vty_out(vty, "Wind view is running.\r\n");
		}
	else
		{
			vty_out(vty, "Wind view is not running.\r\n");
		}
	return CMD_SUCCESS;	
}

/* removed by xieshl 20110629, 问题单13073 */
#if 0
int nme_shutdown_flag = 0 ;
DEFUN(
	nme_shutdown_func,
	nme_shutdown_cmd,
	"nme shutdown",
	"NME config.\n"
	"Shutdown NME.\n"
)
{
	muxDevStopAll(0);
	nme_shutdown_flag=1;
	return CMD_SUCCESS;	
}
DEFUN(
	undo_nme_shutdown_func,
	undo_nme_shutdown_cmd,
	"undo nme shutdown",
	NO_STR
	"NME config.\n"
	"Undo shutdown NME.\n"
)
{
	muxDevStartAll();
	nme_shutdown_flag=0;
	return CMD_SUCCESS;	
}
#endif

/* B--added by liwei056@2010-12-9 for SYS_LOG&OLT_ID_MAP DUMP */
DEFUN(
	test_syslog_olt_func,
	test_syslog_olt_cmd,
	"test syslog olt [<0x0101-0x1910>|<0-255>] {cmd <1-5000>}*1",
	"test\n"
	"test syslog function\n"
	"test olt's id-map function\n"
	"please input the olt's devid(0x|slot|port)\n"
	"please input the olt's id\n"
)
{
    int iOltID;

    if ( ('0' == argv[0][0]) && (('x' == argv[0][1])
            || ('X' == argv[0][1])) )
    {
        iOltID = VOS_StrToUL( argv[0], NULL, 16 );
    }
    else
    {
        iOltID = VOS_StrToUL( argv[0], NULL, 10 );
    }
	OLTAdv_TestOltID(iOltID, vty);
    
    if ( argc > 1 )
    {
        int iCmdID;
        int iIsSupported;

        iCmdID = VOS_AtoI(argv[1]);
        iIsSupported = OLTAdv_CmdIsSupported(iOltID, iCmdID);
        vty_out(vty, "OLTAdv_CmdIsSupported(olt:%d, cmd:%d)'s result(%d).\r\n", iOltID, iCmdID, iIsSupported);
        VOS_SysLog(LOG_TYPE_OLT, LOG_WARNING, "OLTAdv_CmdIsSupported(olt:%d, cmd:%d)'s result(%d).", iOltID, iCmdID, iIsSupported);
    }
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2010-12-9 for SYS_LOG&OLT_ID_MAP DUMP */

/* B--added by liwei056@2011-6-22 for RPC DelayTest */
DEFUN(
	test_delay_olt_func,
	test_delay_olt_cmd,
	"test [rpc-delay|cdp-delay] olt [<0x0101-0x4410>|<0-255>]",
	"test\n"
	"test rpc's time delay\n"
	"test cdp's time delay\n"
	"please input the olt's devid(0x|slot|port)\n"
	"please input the olt's id\n"
    )
{
    int iOltID;

    if ( ('0' == argv[1][0]) && (('x' == argv[1][1])
            || ('X' == argv[1][1])) )
    {
        iOltID = VOS_StrToUL( argv[1], NULL, 16 );
    }
    else
    {
        iOltID = VOS_StrToUL( argv[1], NULL, 10 );
    }

    if ( OLT_ISVALID(iOltID) )
    {
    	struct timeval rpc_starttime;
    	struct timeval rpc_endtime;
        UINT32 delay_time_ms;

    	gettimeofday(&rpc_starttime, NULL);
        if ( 'r' == argv[0][0] )
        {
        	OLTAdv_IsExist(iOltID);
        }
        else
        {
            OLT_SYNC_Data(iOltID, FC_TEST_PONPORT, NULL, 0);
        }
    	gettimeofday(&rpc_endtime, NULL);
    	delay_time_ms = calculate_time_diff_ms(&rpc_endtime, &rpc_starttime);

        vty_out(vty, "the olt id's %s is %d(ms).\r\n", argv[0], delay_time_ms);
    }
    else
    {
        vty_out(vty, "the olt id is invalid.\r\n");
    }
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2011-6-22 for RPC DelayTest */

/* B--added by liwei056@2011-11-11 for PonCardTest */
DEFUN(
	test_card_func,
	test_card_cmd,
	"test card <1-14> {delay <0-10000> times <0-100000>}*1",
	"test\n"
	"test pon-card's access to all of the pon-chips\n"
	"please input the slot number\n"
	"test delay\n"
	"please input the delay-time(Unit:ms)\n"
	"test times\n"
	"please input the times(0-test forever)\n"
    )
{
    int iRlt;
    int iSlotID;

    iSlotID = VOS_AtoI(argv[0]);
    if ( ROK == SlotCardIsPonBoard(iSlotID) )
    {
        int iPortNum;
        int iDelayTime;
        int iTestTimes;
        short int sOltStartPort, sOltEndPort;

        if ( argc > 2 )
        {
            iDelayTime = VOS_AtoI(argv[1]);
            iTestTimes = VOS_AtoI(argv[2]);
        }
        else
        {
            iDelayTime = 0;
            iTestTimes = 100;
        }
        
        if ( 0 < (iPortNum = GetSlotPonPortPhyRange(iSlotID, &sOltStartPort, &sOltEndPort)) )
        {
            vty_direct_out(vty, "\r\npon_card%d<%d-%d> test start...\r\n", iSlotID, sOltStartPort, sOltEndPort);

            for ( ; sOltStartPort <= sOltEndPort; ++sOltStartPort  )
            {
                iRlt = pon_test_oltEx2(vty, iSlotID, sOltStartPort, iDelayTime, iTestTimes, TRUE);
                VOS_TaskDelay(VOS_TICK_SECOND/10);
            }
        }
        else
        {
            vty_out(vty, "  %% slot%d's have not any pon port.\r\n", iSlotID);
        }
    }
    else
    {
        vty_out(vty, "  %% the slot%d is not pon card.\r\n", iSlotID);
    	return CMD_WARNING;	
    }   
   
	return CMD_SUCCESS;	
}

DEFUN(
	test_port_func,
	test_port_cmd,
	"test port <portlist> {delay <0-10000> times <0-100000>}*1",
	"test\n"
	"test pon-chips' access\n"
	"please input the port list\n"
	"test delay\n"
	"please input the delay-time(Unit:ms)\n"
	"test times\n"
	"please input the times(0-test forever)\n"
    )
{
    int iRlt;
    ULONG ulIfIndex;
    ULONG ulSlot, ulPort;
    int iDelayTime;
    int iTestTimes;

    if ( argc > 2 )
    {
        iDelayTime = VOS_AtoI(argv[1]);
        iTestTimes = VOS_AtoI(argv[2]);
    }
    else
    {
        iDelayTime = 0;
        iTestTimes = 100;
    }

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( VOS_OK == ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort ) )
        {
            if ( ROK == SlotCardIsPonBoard(ulSlot) )
            {
            	if( ROK == PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) )
                {
                    iRlt = pon_test_oltEx2(vty, ulSlot, ulPort, iDelayTime, iTestTimes, TRUE);
                    VOS_TaskDelay(VOS_TICK_SECOND/10);
                }
                else
            	{
            		vty_out( vty, "\r\n  %% pon%d/%d is not exist\r\n", ulSlot, ulPort );
            	}
            }
            else
            {
                vty_out( vty, "\r\n  %% the slot%d is not pon card, pon%d/%d is not exist.\r\n", ulSlot, ulSlot, ulPort );
            }   
        }      
        else
        {
            vty_out( vty, "\r\n  %% the port%d/%d is unknowned.\r\n", ulSlot, ulPort );
        }   
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2011-11-11 for PonCardTest */


/* B--added by liwei056@2011-12-22 for PonPortTxTest */
DEFUN(
	show_pontx_func,
	show_pontx_cmd,
	"show pon-tx <portlist>",
	DescStringCommonShow
	"pon's optical tx status\n"
	"please input the port list\n"
    )
{
    int iRlt;
    ULONG ulIfIndex;
    ULONG ulSlot, ulPort;
    short int PonPortIdx;

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( VOS_OK == ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort ) )
        {
            if ( ROK == SlotCardIsPonBoard(ulSlot) )
            {
            	if( ROK == PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) )
                {
                    if ( RERROR != (PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort)) )
                    {
                        iRlt = OLTAdv_GetOpticalTxMode(PonPortIdx);
                        if ( iRlt > 0 )
                        {
                    		vty_out( vty, "\r\n  %% pon%d/%d is txing ray\r\n", ulSlot, ulPort );
                        }
                        else if ( 0 == iRlt )
                        {
                    		vty_out( vty, "\r\n  %% pon%d/%d is stoping tx ray\r\n", ulSlot, ulPort );
                        }
                        else
                        {
                    		vty_out( vty, "\r\n  %% pon%d/%d is not exist\r\n", ulSlot, ulPort );
                        }
                    }
                }
                else
            	{
            		vty_out( vty, "\r\n  %% pon%d/%d is not exist\r\n", ulSlot, ulPort );
            	}
            }
            else
            {
                vty_out( vty, "\r\n  %% the slot%d is not pon card, pon%d/%d is not exist.\r\n", ulSlot, ulSlot, ulPort );
            }   
        }      
        else
        {
            vty_out( vty, "\r\n  %% the port%d/%d is unknowned.\r\n", ulSlot, ulPort );
        }   
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2011-12-22 for PonPortTxTest */

/* B--added by liwei056@2012-8-23 for PonIfTypeTest */
DEFUN(
	show_ponif_func,
	show_ponif_cmd,
	"show olt-if <portlist>",
	DescStringCommonShow
	"olt's if type\n"
	"please input the port list\n"
    )
{
    static char* s_cszIfTypeNames[] = { "OLT_ADAPTER_NULL",
                                        "OLT_ADAPTER_GLOBAL",    
                                        "OLT_ADAPTER_RPC",

                                        "OLT_ADAPTER_PAS5001",    
                                        "OLT_ADAPTER_PAS5201",    
                                        "OLT_ADAPTER_PAS5204",    
                                        "OLT_ADAPTER_PAS8411",    
                                        
                                        "OLT_ADAPTER_TK3723",    
                                        "OLT_ADAPTER_BCM55524",    
                                        "OLT_ADAPTER_BCM55538",    

                                        "OLT_ADAPTER_GW",                                           
                                        "OLT_ADAPTER_MAX" };    
    int iIfTypeID;
    ULONG ulIfIndex;
    ULONG ulSlot, ulPort;
    short int PonPortIdx;

	BEGIN_PARSE_PORT_LIST_TO_IFINDEX_ETH( argv[0], ulIfIndex )
	{
		if( VOS_OK == ulIfindex_2_userSlot_userPort( ulIfIndex, &ulSlot, &ulPort ) )
        {
        	if( ROK == PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) )
            {
                if ( RERROR != (PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort)) )
                {
                    iIfTypeID = OLT_GetIFType(PonPortIdx);
                    if ( (iIfTypeID >= 0) && (iIfTypeID < ARRAY_SIZE(s_cszIfTypeNames)) )
                    {
                		vty_out( vty, "\r\n  %% pon%d/%d's IF is [%s].\r\n", ulSlot, ulPort, s_cszIfTypeNames[iIfTypeID] );
                    }
                    else
                    {
                		vty_out( vty, "\r\n  %% pon%d/%d's IF(%d) is unknowned.\r\n", ulSlot, ulPort, iIfTypeID );
                    }
                }
            }
            else
        	{
        		vty_out( vty, "\r\n  %% pon%d/%d is not exist\r\n", ulSlot, ulPort );
        	}
        }      
        else
        {
            vty_out( vty, "\r\n  %% the port%d/%d is unknowned.\r\n", ulSlot, ulPort );
        }   
	}
	END_PARSE_PORT_LIST_TO_IFINDEX_ETH()
   
	return CMD_SUCCESS;	
}

DEFUN(
	show_onuif_func,
	show_onuif_cmd,
	"show onu-if <slot/port> <onuid_list>",
	DescStringCommonShow
	"onu's if type\n"
	"please input the pon port\n"
	"please input the onu list\n"
    )
{
    static char* s_cszIfTypeNames[] = { "ONU_ADAPTER_NULL",
                                        "ONU_ADAPTER_RPC",    

                                        "ONU_ADAPTER_PAS5001_GW",    
                                        "ONU_ADAPTER_PAS5201_GW",    
                                        "ONU_ADAPTER_PAS5204_GW",    
                                        "ONU_ADAPTER_PAS8411_GW",    

                                        "ONU_ADAPTER_PAS5201_CTC",    
                                        "ONU_ADAPTER_PAS5204_CTC",
                                        "ONU_ADAPTER_PAS8411_CTC",

                                        "ONU_ADAPTER_TK3723_GW",    
                                        "ONU_ADAPTER_TK3723_CTC",
                                        "ONU_ADAPTER_BCM55524_GW",    
                                        "ONU_ADAPTER_BCM55524_CTC",
                                        "ONU_ADAPTER_BCM55538_GW",    
                                        "ONU_ADAPTER_BCM55538_CTC",

                                        "ONU_ADAPTER_GW",                                           
                                        "ONU_ADAPTER_MAX" };    
    int iIfTypeID;
    ULONG ulSlot, ulPort;
    ULONG ulOnuId;
    short int PonPortIdx;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty ) != ROK )
		return(CMD_WARNING);

	if( ROK == PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) )
    {
        if ( RERROR != (PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort)) )
        {
            BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[1], ulOnuId )
            {
                iIfTypeID = ONU_GetIFType(PonPortIdx, ulOnuId - 1);
                if ( (iIfTypeID >= 0) && (iIfTypeID < ARRAY_SIZE(s_cszIfTypeNames)) )
                {
            		vty_out( vty, "\r\n  %% onu%d/%d/%d's IF is [%s].\r\n", ulSlot, ulPort, ulOnuId, s_cszIfTypeNames[iIfTypeID] );
                }
                else
                {
            		vty_out( vty, "\r\n  %% onu%d/%d/%d's IF(%d) is unknowned.\r\n", ulSlot, ulPort, ulOnuId, iIfTypeID );
                }
            }
            END_PARSE_ONUID_LIST_TO_ONUID();
        }
    }
    else
	{
		vty_out( vty, "\r\n  %% pon%d/%d is not exist\r\n", ulSlot, ulPort );
	}
   
	return CMD_SUCCESS;	
}
/* E--added by liwei056@2012-8-23 for PonIfTypeTest */

/* B--added by liwei056@2010-11-29 for PAS_SOFT DUMP */
#ifdef SYS_DUMP
DEFUN(
	pon_dump_func,
	pon_dump_cmd,
	"dump pon <slot/port> llid [all|<0-"INT_TO_STR(MAXONUPERPONNOLIMIT)">] type <0-6>",
	"Dump the system\n"
	"Dump pon information\n"
	"please input the pon port\n"
	"Dump onu information"
	"all onu\n"
	"please input the onu\n"
	"Dump information's type.\n"
	"The Type of dump information(0-General&Olt&Onu; 1-Olt; 2-Onu; 3-General&Olt; 4-General&Onu; 5-Olt&Onu; 6-General)\n"
)
{
    int iRlt;
    ULONG ulSlot, ulPort;
    int iDumpType;
    int aiDumpTypeMap[7] = {PON_SYS_DUMP_GENERAL_CONFIG | PON_SYS_DUMP_OLT_CONFIG | PON_SYS_DUMP_ONU_CONFIG,
                            PON_SYS_DUMP_OLT_CONFIG,
                            PON_SYS_DUMP_ONU_CONFIG,
                            PON_SYS_DUMP_GENERAL_CONFIG | PON_SYS_DUMP_OLT_CONFIG,
                            PON_SYS_DUMP_GENERAL_CONFIG | PON_SYS_DUMP_ONU_CONFIG,
                            PON_SYS_DUMP_OLT_CONFIG | PON_SYS_DUMP_ONU_CONFIG,
                            PON_SYS_DUMP_GENERAL_CONFIG
                            };
    short int olt_id, onu_id;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return(CMD_WARNING);
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (olt_id == VOS_ERROR)
	{
    	vty_out( vty, "  %% pon port Parameter is error.\r\n" );

    	return CMD_WARNING;
	}

    if ( 'a' == argv[1][0] )
    {
        onu_id = PON_SYS_DUMP_ALL;
    }
    else
    {
        onu_id = VOS_AtoI(argv[1]);
        CHECK_CMD_ONU_RANGE(vty, onu_id-1);        
    }

    iDumpType = aiDumpTypeMap[VOS_AtoI(argv[2])];
    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	if( 0 == (iRlt = OLT_SysDump(olt_id, onu_id, iDumpType)) )
	{
		vty_out(vty, "Dump is OK.\r\n");
	}
	else
	{
        switch (iRlt)
        {
            case PAS_OLT_NOT_EXIST:
        		vty_out(vty, "Olt%s is not existed.\r\n", argv[0]);
                break;
            case PAS_ONU_NOT_AVAILABLE:
        		vty_out(vty, "Onu%s is not available.\r\n", argv[1]);
                break;
            default:
        		vty_out(vty, "Dump is Failed(%d).\r\n", iRlt);
        }
    
        return CMD_WARNING;
	}
    
	return CMD_SUCCESS;	
}
#endif
/* E--added by liwei056@2010-11-29 for PAS_SOFT DUMP */
extern int g_gwonu_pty_flag;      
extern int SetPrivatePtyEnable(int enable);
DEFUN (
	config_ctc_onu_telnet_valid_client,
	config_ctc_onu_telnet_valid_client_cmd,
	"private-pty {[enable|disable]}*1",
	"Private-onu pty vconsole client\n"
	"Enable\n"
	"Disable\n"
	)
{
    short int slotno = 0;
    if(argc==1)
    {
        int enable = VOS_StrCmp(argv[0], "enable")==0?1:0;
        SetPrivatePtyEnable(enable);
    }
    else
    {
        vty_out(vty, "Private-pty is %s\r\n", g_gwonu_pty_flag?"enable":"disable");
    }
    return CMD_SUCCESS;
}

extern int setOnuTransmission_flag(int enable);
extern int getOnuTransmission_flag();
extern ULONG get_ctcOnuOtherVendorAccept();
extern ULONG set_ctcOnuOtherVendorAccept(const ULONG v);
/*add by luh 2011-9-24 增加对GW-ONU 完全透传使能配置(默认使能) ，当使能*/
DEFUN(onu_completely_transmission_enable,
	onu_completely_transmission_enable_cmd,
	"onu transmission-flag {[enable|disable]}*1",
	"GW Onu config\n"
	"transmission flag config\n"
	"Enable\n"
	"Disable\n"
	)
{
    int enable  = 0;
    if(argc == 1)
    {
        enable = VOS_StriCmp(argv[0],"enable")==0?ONU_TRANSMISSION_ENABLE:ONU_TRANSMISSION_DISABLE;
        if(OnuTransmission_flag == enable)
            vty_out(vty,"onu transmission-flag has been %s \r\n",argv[0]);
        else
    	    setOnuTransmission_flag(enable);
    }
    else
        vty_out(vty,"onu transmission-flag is %s\r\n",OnuTransmission_flag?"enable":"disable");
    return CMD_SUCCESS;
}
DEFUN(set_pendingQueue_time,
	set_pendingQueue_time_cmd,
	"pending-queue active-period {<60-3600>}*1",
	"Set Pending-Queue Config\n"
	"Set active period\n"
	"the period(seconds),default is 120s\n"
	)
{
    int period  = 0;
    if(argc == 1)
    {
        period = VOS_StrToUL(argv[0], NULL, 10);
        if(UpdatePendingQueueTime == period)
            vty_out(vty,"Onu-Pending Queue Update period has been %ds.\r\n", period);
        else
    	    SetUpdatePendingTime(period);
    }
    else
        vty_out(vty,"Onu-Pending Queue Update period is %ds.\r\n", UpdatePendingQueueTime);
    return CMD_SUCCESS;
}

DEFUN(onu_accept_other_vendor_ctc,
        onu_accept_othre_vendor_ctc_cmd,
        "onu other-ctc registration [enable|disable]",
        "onu config\n"
        "CTC onus of other vendors\n"
        "registration policy\n"
        "enable onu registration\n"
        "disable onu registration\n")
{

    ULONG v = VOS_StriCmp(argv[0], "disable")?1:0;

    set_ctcOnuOtherVendorAccept(v);

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && (!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER))
    {
        setOtherCtcOnuRegistrationForAllPonCard(v);
    }
    return CMD_SUCCESS;    
}

DEFUN(show_onu_accept_other_vendor_ctc,
        show_onu_accept_other_vendor_ctc_cmd,
        "show other-ctc registration",
        SHOW_STR
        "CTC onus of other vendors\n"
        "registration policy\n")
{
    vty_out(vty, "the registration policy of other ctc onus is %s\r\n", get_ctcOnuOtherVendorAccept()?"enable":"disable");
    return CMD_SUCCESS;    
}

int gAutoClearArpflag = 0;
DEFUN(clear_arp_by_onu_enable,
        clear_arp_by_onu_enable_cmd,
        "config auto-clear-arp {[0|1]}*1",
        "Config system's setting\n"
        "Clear arp when onu not present\n"
        "Disable auto-clear-arp\n"
        "Enable auto-clear-arp\n")
{
    ULONG enable = 0;
    if(argc == 1)
    {
        enable = VOS_StrToUL(argv[0], NULL, 10);
        gAutoClearArpflag = enable;
    }
    else
    {
        vty_out(vty, "\r\n  Auto clear arp when onu not present: %s\r\n", gAutoClearArpflag?"Enable":"Disable");
    }
    return CMD_SUCCESS;
}
extern int BootLog_ShowFlash(struct vty *vty);
extern int Bootlog_EraseFlash();

DEFUN(show_olt_reset_info,
        show_olt_reset_info_cmd,
        "show local reset-info",
        SHOW_STR
        "Local board type\n"
        "Local board reset information\n")
{
    BootLog_ShowFlash(vty);
    return CMD_SUCCESS;
}
DEFUN(erase_olt_reset_info,
        erase_olt_reset_info_cmd,
        "erase local reset-info",
        "Erase information\n"
        "Local board type\n"
        "Local board reset information\n")
{
    Bootlog_EraseFlash();
    return CMD_SUCCESS;
}
extern int ep1000_read_reg(char phy_slot,char reg,char *retBuff);
extern int ep1000_write_reg(char phy_slot,char reg,char *buff);
extern unsigned char ep1000_reg_len(char reg);
extern int rs485Get(int array[]);
extern int rs485IsValid(char phySlot);


DEFUN(show_olt_power_info,
        show_olt_power_info_cmd,
        "show power {<0-3>}*1",
        SHOW_STR
        "Show power information\n"
        "Chose slot\n")
{
    char buf[32];
	int array[4];
	int slot;
	int len=0;
	int rs485cnt=0;
	int i;
	float val;
	unsigned short vol,cap;
	unsigned long tmp1,tmp2;

	if(argc==1)
	{
		slot = VOS_AtoI(argv[0]);
		if(rs485IsValid(slot)==FALSE)
		{
			vty_out(vty,"  %% Invalid slot %d.\r\n",slot);
			return CMD_SUCCESS;
		}
		rs485cnt = 1;
		array[0] = slot;
	}
	else
	{
		rs485cnt = rs485Get(array);
	}
	if(rs485cnt == 0)
	{
		vty_out(vty,"  %% No valid slot exist.\r\n");
		return CMD_SUCCESS;
	}
	for(i=0;i<rs485cnt;i++)
	{
		slot = array[i];
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(slot,0x20,buf);
		if(len==2)
		{
			vol=*((unsigned short *)buf);
			val =vol;
			val = val/400;
			tmp1=val;
			tmp2=(val-tmp1)*100;
			vty_out(vty,"%d Current Voltage: %d.%02dV \r\n",
				slot,tmp1,tmp2);
		}
		else
		{
			vty_out(vty,"%d Current Voltage: unknown\r\n",slot);
		}

		VOS_MemZero(buf,32);
		len = ep1000_read_reg(slot,0xa,buf);
		if(len==2)
		{
			cap=*((unsigned short *)buf);
			val =cap;
			val = val/10;
			tmp1=val;
			tmp2=(val-tmp1)*10;

			vty_out(vty,"%d Current Capacity: %d.%01dA \r\n",
				slot,tmp1,tmp2);
		}
		else
		{
			vty_out(vty,"%d Current Capacity: unknown\r\n",slot);
		}
	}

    return CMD_SUCCESS;
}

DEFUN(show_olt_power_reg,
        show_olt_power_reg_cmd,
        "power <0-3> read <reg>",
        "Communite power with rs485\n"
        "Chose slot\n"
        "Read reg\n"
        "Read reg\n")
{
    char buf[32];
	int len=0;

	int slot;
	char reg;
	int i;
	slot = VOS_AtoI(argv[0]);
	if(rs485IsValid(slot)==FALSE)
	{
		vty_out(vty,"  %% Invalid slot %d.\r\n",slot);
		return CMD_SUCCESS;
	}

	reg = VOS_AtoI(argv[1]);
	if(reg==0)
		reg = VOS_StrToUL(argv[1], NULL, 16);

	
	VOS_MemZero(buf,32);
	len = ep1000_read_reg(slot,reg,buf);

	if(len<=0)
	{
		vty_out(vty,"  %% Get reg 0x%02x error.\r\n",reg);
		return CMD_SUCCESS;
	}

	vty_out(vty,"Reg 0x%02x(len %d): 0x",reg,len);
	for(i=0;i<len;i++)
	{
		vty_out(vty,"%02x",buf[i]);
	}
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;
}

DEFUN(write_olt_power_reg,
        write_olt_power_reg_cmd,
        "power <0-3> write <reg> <val>",
        "Communite power with rs485\n"
        "Chose slot\n"
        "Write reg\n"
        "Write reg\n"
        "Reg value\n")
{
	char *pVal=NULL;
	unsigned long val;
	int len=0;

	int slot;
	char reg;
	char reg_len=0;
	int i;
	slot = VOS_AtoI(argv[0]);
	if(rs485IsValid(slot)==FALSE)
	{
		vty_out(vty,"  %% Invalid slot %d.\r\n",slot);
		return CMD_SUCCESS;
	}

	reg = VOS_AtoI(argv[1]);
	if(reg==0)
		reg = VOS_StrToUL(argv[1], NULL, 16);


	reg_len = ep1000_reg_len(reg);

	if(reg_len>4)
	{
		vty_out(vty,"  %% Reg 0x%02x size is too long %d.\r\n",reg,reg_len);
		return CMD_SUCCESS;
	}
	else if(reg_len<=0)
	{
		vty_out(vty,"  %% Unsupport Reg 0x%02x.\r\n",reg);
		return CMD_SUCCESS;
	}
	pVal = argv[2];
	if((pVal[0]=='0')&&((pVal[1]=='x')||(pVal[1]=='X')))
		val = VOS_StrToUL(pVal, NULL, 16);
	else
		val = VOS_StrToUL(pVal, NULL, 10);

	val = val<<((4-reg_len)*8);

	
	/*vty_out(vty,"Set reg 0x%02x val 0x%08x len %d.\r\n",reg,val,reg_len);*/
	
	len = ep1000_write_reg(slot,reg,&val);

	

	if(len<=0)
	{
		vty_out(vty,"  %% Set reg 0x%02x error.\r\n",reg);
		return CMD_SUCCESS;
	}

	return CMD_SUCCESS;
}

#if 0
extern int TestFlag;
extern void ReadPonPortVer(short int PonPortIdx,unsigned long arg2,unsigned long arg3,unsigned longarg4,unsigned long arg5,unsigned long arg6,unsigned long arg7,unsigned long arg8,unsigned long arg9,unsigned long arg10);
extern void ReadPonPortVer1(void);
DEFUN (
	start_pon_port_test,
	start_pon_port_test_cmd,
	"pon port self-test [all|<slot/pon>] priority <1-255>",
	"start pon port self-test\n"
	"start pon port self-test\n"
	"start pon port self-test\n"
	"all pon port\n"
	"pon port Idx\n"
	"priority\n"
	"range is 1-255\n"
	)
{
	unsigned long slot, pon;
	short int PonPortIdx;
	unsigned long priority;
	static unsigned char TaskName[VOS_NAME_MAX_LENGTH] = {0};
	static unsigned long TaskArg[VOS_MAX_TASK_ARGS]={0};

	if( VOS_StrCmp( argv[0], "all" ) != 0 )
		{		
		IFM_ParseSlotPort(argv[0], &slot, &pon);	
		priority = ( ULONG ) VOS_AtoL( argv[1] );
		
		PonPortIdx = GetPonPortIdxBySlot(slot, pon);
		VOS_MemZero(TaskName,VOS_NAME_MAX_LENGTH);
		VOS_MemSet(TaskArg,0,VOS_MAX_TASK_ARGS*sizeof(unsigned long));

		CHECK_PON_RANGE

		TestFlag = V2R1_ENABLE;
		sprintf(&TaskName[0], "PonTest%d/%d",slot,pon);
		TaskArg[0] =(unsigned long)PonPortIdx;
		VOS_TaskCreate ( &TaskName[0], priority, ReadPonPortVer ,  &TaskArg[0] ); 
		}
	else{
		TestFlag = V2R1_ENABLE;
		priority = ( ULONG ) VOS_AtoL( argv[1] );
		VOS_TaskCreate("PonTestAll",priority, ReadPonPortVer1,0);
		}
	return(CMD_SUCCESS);	

}

DEFUN (
	stop_pon_port_test,
	stop_pon_port_test_cmd,
	"undo pon port self-test",
	"undo pon port self-test\n"
	"undo pon port self-test\n"
	"undo pon port self-test\n"
	"undo pon port self-test\n"
	/*"pon port Idx\n"*/
	)
{
	/*
	unsigned long slot, pon;
	short int PonPortIdx;
	
	IFM_ParseSlotPort(argv[0], &slot, &pon);	
	
	PonPortIdx = GetPonPortIdxBySlot(slot, pon);

	CHECK_PON_RANGE
	*/
	TestFlag = V2R1_DISABLE;
	 
	return(CMD_SUCCESS);	

}

extern unsigned int Delay_time;
extern unsigned int Buffer_TestFlag;
extern void DestortyPASSendBuffer(void);

DEFUN (
	start_pas_buffer_test,
	start_pas_buffer_test_cmd,
	"pas-buffer test priority <1-255> Gaps <1-1000>",
	"start pas-buffer test\n"
	"start pas-buffer test\n"
	"priority\n"
	"range is 1-255\n"
	"modified buffer interval\n"
	"unit:10ms\n"
	)
{
	unsigned long priority;

	priority = ( ULONG ) VOS_AtoL( argv[0] );
	Delay_time = ( ULONG ) VOS_AtoL( argv[1] );
	Buffer_TestFlag = V2R1_ENABLE;

	VOS_TaskCreate ("PasBuffTest", priority, DestortyPASSendBuffer,  NULL); 
		
	return(CMD_SUCCESS);	

}

DEFUN (
	stop_pas_buffer_test,
	stop_pas_buffer_test_cmd,
	"undo pas-buffer test",
	"undo pas-buffer test\n"
	"undo pas-buffer test\n"
	"undo pas-buffer test\n"
	)
{
	Buffer_TestFlag = V2R1_DISABLE;
	return(CMD_SUCCESS);	
}


extern void SetPonMsgLog(PON_log_flag_t  LogLevel, unsigned char EnableFlag);
DEFUN (
	start_pas_log,
	start_pas_log_cmd,
	"pas-log level <0-15> [enable|disable]",
	"start pas-logt\n"
	"log level\n"
	"log level\n"
	"enable log\n"
	"disable log\n"
	)
{
	unsigned long LogLevel;
	unsigned char EnableFlag;

	LogLevel = ( ULONG ) VOS_AtoL( argv[0] );

	if( VOS_StrCmp( argv[0], "enable" ) != 0 )
		EnableFlag = ENABLE;
	else 
		EnableFlag = DISABLE;

	SetPonMsgLog ((PON_log_flag_t)LogLevel, EnableFlag); 
		
	return(CMD_SUCCESS);	

}
#endif
#if 0
/* added by chenfj 2007-8-17 */
DEFUN (
	update_onu_file_func,
	update_onu_file_cmd,
	"update onu file <slot/port/onuid>",
	"update onu file\n"
	"update onu file\n"
	"update onu file\n"
	"input the slot/port/onuid\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId;
    	INT16 phyPonId = 0;
	int ret = 0;
	short int PonChipType;
	int OnuType;

	if( argc != 1 ) 
		{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
		}

	if( PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuId ) != VOS_OK )
		return (CMD_WARNING );
	
	if ((ulSlot<4)  || (ulSlot>8))
	{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
	}

	if (ulSlot == 4)
	{
		if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
		{
			vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
			return CMD_WARNING;
		}
	}
	
	if ( (ulPort < 1) || (ulPort > 4) )
	{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
		return CMD_WARNING;
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


	OnuType =  GetOnuVendorType(phyPonId, (ulOnuId-1));
	
	if( OnuType == ONU_VENDOR_CT )
		{
		StartOnuSWUpdate_1(phyPonId, (ulOnuId-1));
		}

	else if( OnuType ==  ONU_VENDOR_GW)
		{
		StartOnuSWUpdate(phyPonId, (ulOnuId-1), V2R1_NO_WAIT_RETURN);
		}
		
	/*
	if(  ret == ROK )
		{
		vty_out(vty, "  transfer file to onu %d/%d/%d complete\r\n",  ulSlot, ulPort, ulOnuId );
		return( CMD_SUCCESS );
		}
	else {
		vty_out(vty, "  transfer file to onu %d/%d/%d failed\r\n",  ulSlot, ulPort, ulOnuId );
		return( CMD_WARNING );
		}
	*/
	return CMD_SUCCESS;
}

#endif
int AuthModeGetPortList(unsigned short slot, char *portlist, int  length)
{
	char portlist_str[256], tmp[16];
	ULONG Svalue = 0, Evalue = 0,counter = 0;
	ULONG y=0;
    int ulsport, enable;
	if(length <256)
		return VOS_ERROR;
	
	VOS_MemZero( portlist_str, sizeof(portlist_str) );
	VOS_MemZero( tmp, sizeof(tmp) );
    for(ulsport=1;ulsport<=PONPORTPERCARD;ulsport++)
    {	
        counter = 0;
        y = ulsport;
        while(userport_is_pon(slot,y) == VOS_YES
            && getOnuAuthEnable(slot, y, &enable) == VOS_OK 
            && enable != V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            counter++ ;
            if(counter == 1)
                Svalue = y;
            				
            Evalue = y;
            if(y < PONPORTPERCARD)
                y++ ;
            else if( y == PONPORTPERCARD)
                break;
        }
        if(counter == 0)
        {
        }
        else if(counter == 1)
        {
            VOS_Sprintf( tmp, "%s%d/%d", ((portlist_str[0] == 0) ? "" : ","), slot, ulsport );
            VOS_StrCat( portlist_str, tmp );
        }
        else
        {
            VOS_Sprintf( tmp, "%s%d/%d-%d", ((portlist_str[0] == 0) ? "" : ","), slot, Svalue, Evalue);
            VOS_StrCat( portlist_str, tmp );
        }
        ulsport = y;
    }
	VOS_MemCpy(portlist,  portlist_str, 256);
	return VOS_OK;
}
LONG OLT_show_run( struct vty * vty )
{
	/*int iRes = VOS_OK; */
	
	vty_out( vty, "!OLT information config\r\n" );

#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
	trace_path_show_run( vty );
#endif

	/*此处用来取出olt 的配置数据与默认数据之间进行比较，有差异打印出来
	注意打印格式其实是相应的设置命令。*/

    if(gAutoClearArpflag)
	    vty_out( vty, " config auto-clear-arp 1\r\n");
    
	if(!get_ctcOnuOtherVendorAccept())
	    vty_out( vty, " onu other-ctc registration disable\r\n");

    if(MaxMACDefault != MAXONUPERPON)
        vty_out ( vty, " onu default-max-mac %d\r\n", MaxMACDefault);/*GW ONU 透传使能2011-9-26*/
	if(OnuTransmission_flag == ONU_TRANSMISSION_DISABLE)
        vty_out ( vty, " onu transmission-flag disable\r\n");/*GW ONU 透传使能2011-9-26*/
    if(UpdatePendingQueueTime != UPDATE_PENDING_QUEUE_TIME)
        vty_out ( vty, " pending-queue active-period %d\r\n", UpdatePendingQueueTime);/*2013-7-3*/        
        
    if(!g_gwonu_pty_flag)
        vty_out(vty, " private-pty disable\r\n");
    if(0 == cl_vty_user_limit)
        vty_out(vty, " vty user limit no\r\n");

    if(MaxOnuDefault != DAFAULT_MAX_ONU)
        vty_out(vty, " config default-max-onu %d\r\n", MaxOnuDefault);
	if(Timeout_delete_authentry != 0)
		vty_out(vty, " auto-delete onu-authentry enable\r\n");
	if(conf_associate_share != 0)
		vty_out(vty, " auto-associate onu-profile enable\r\n");

	show_run_onu_model(vty);
        
	{
		char defname[256] = {0};
		int defnameLen = 0;
		char runname[256] = {0};
		int runLen = 0;	

		if(VOS_OK != GetOltDeviceNameDefault(defname, &defnameLen))
		{
			return (VOS_ERROR);
		}
		if(defnameLen >  255 ) return(VOS_ERROR);
		defname[defnameLen] = '\0';
		
   		if(VOS_OK != GetOltDeviceName(runname,&runLen))
		{
			return (VOS_ERROR);
		}
		if(runLen > 255)return(VOS_ERROR);
		runname[runLen] = '\0';
		
		if (VOS_OK != strcmp(defname, runname))
		/*if (VOS_OK != strcmp(defname, runname))*/
		{
			vty_out ( vty, " device name %s\r\n",runname);
			/*vty_out (vty, "  devicename %s\r\n",runname);*/	
		}
	}

	/*device description*/

	{
		char def_desc[256] = {0};
		int len_desc = 0;
		int len_rundesc = 0;
		char runDesc[256] = {0};
		if (VOS_OK != GetOltDeviceDescDefault(def_desc , &len_desc))
		{
			return (VOS_ERROR);
		}
		if(len_desc > 255) return(VOS_ERROR);
		def_desc[len_desc] = '\0';
		
		if (VOS_OK != GetOltDeviceDesc(runDesc, &len_rundesc))
		{
			return (VOS_ERROR);
		}
		if(len_rundesc > 255) return(VOS_ERROR);
		runDesc[len_rundesc] = '\0';
		
		if (VOS_OK != strcmp(def_desc, runDesc))
		{
			vty_out (vty, " device description %s\r\n",runDesc);	
		}
	}

	{
		char def_location[256] = {0};
		int lo_len = 0;
		char run_location[256] = {0};
		int run_lonlen = 0;
		if (VOS_OK != GetOltLocationDefault(def_location, &lo_len))
		{
			return CMD_WARNING;
		}
		if(lo_len > 255) return(VOS_ERROR);
		def_location[lo_len] = '\0';
		
		if (VOS_OK != GetOltLocation( run_location, &run_lonlen))
		{
			return CMD_WARNING;
		}
		if(run_lonlen > 255 ) return(VOS_ERROR);
		run_location[run_lonlen] = '\0';
		
		if(VOS_OK != strcmp(def_location, run_location))
		{
			vty_out( vty, " device location %s\r\n", run_location);
		}

	}
	
	{/*设置downlink policer*/
		int def_flag = 0;
		int run_flag = 0;
		def_flag = GetOnuPolicerFlagDefault();
		run_flag = GetOnuPolicerFlag();
		if ( def_flag != run_flag )
		{			
			if ( run_flag == V2R1_ENABLE )
			{
				vty_out( vty, " onu downlink-policer\r\n" );
			}
			else if ( run_flag == V2R1_DISABLE )
			{
				vty_out( vty, " undo onu downlink-policer\r\n" );
			}
		}     
	}
    

/* B--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */
	{/*设置downlink policer param*/
    	int iBurst;
    	int iPre;
    	int iWeight;
        
        GetOnuPolicerParam( &iBurst, &iPre, &iWeight );
		if ( (iBurst != ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT)
            || (iPre != DISABLE)
            || (iWeight != ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT) )
		{			
		    if ( iWeight != ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT )
            {
    			vty_out( vty, " onu downlink-policer-param burst %d preference %s weight %d \r\n", iBurst, (iPre == ENABLE) ? "enable" : "disable", iWeight);
            }
            else
            {
    			vty_out( vty, " onu downlink-policer-param burst %d preference %s \r\n", iBurst, (iPre == ENABLE) ? "enable" : "disable");
            }
		}     
	}
/* E--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */

/* B--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
	{/*设置uplink dba param*/
    	int iPktSize;
    	int iBurst;
    	int iWeight;
        
        GetOnuDbaParam( &iPktSize, &iBurst, &iWeight );
		if ( (iPktSize != ONU_UPLINK_DBA_PACKETSIZE_DEFAULT)
            || (iBurst != ONU_UPLINK_POLICE_BURSESIZE_DEFAULT)
            || (iWeight != ONU_UPLINK_DBA_WEIGHT_DEFAULT) )
		{			
		    if ( iWeight != ONU_UPLINK_DBA_WEIGHT_DEFAULT )
            {
    			vty_out( vty, " onu uplink-dba-param pktsize %d burst %d weight %d\r\n", iPktSize, iBurst, iWeight);
            }
            else
            {
    			vty_out( vty, " onu uplink-dba-param pktsize %d burst %d \r\n", iPktSize, iBurst);
            }
		}     
	}
/* E--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */

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
				vty_out( vty, " statistic-history bucket-num 15m %d\r\n",run_bucket_15m);
			}
			if (def_bucket_24h != run_bucket_24h)
			{
				vty_out( vty, " statistic-history bucket-num 24h %d\r\n",run_bucket_24h);
			}
		}
	}

	
	CliIgmpAuthShowRun(vty);
/*		
#if( RPU_MODULE_IGMP_TVM == RPU_YES )
	CliIgmpTvmShowRun(vty);
#endif
*/
#ifdef ONU_PPPOE_RELAY
	{

		if ( '\0' != *PPPOE_Relay_Maual_String_head)
			vty_out(vty," pppoe relay circuitid-set %s%s", PPPOE_Relay_Maual_String_head, VTY_NEWLINE);

		if (PPPOE_RELAY_DISABLE != g_PPPOE_relay)
		{
			if (PPPOE_RELAY_GWD_PRIVATE_MODE == g_PPPOE_relay_mode)
				vty_out(vty," pppoe relay enable gwd\r\n");
			else if(PPPOE_RELAY_DSL_RORUM_MODE == g_PPPOE_relay_mode)
				vty_out(vty," pppoe relay enable dsl-forum\r\n");
            else
				vty_out(vty," pppoe relay enable fpt\r\n");
		}
	}
#endif
#ifdef ONU_DHCP_RELAY
	{
		if (DHCP_RELAY_CTC_MODE != g_DHCP_relay_mode)
		{
			vty_out(vty, " dhcp relay mode standerd\r\n");
		}
		if (DHCP_RELAY_DISABLE != g_DHCP_relay)
		{
			vty_out(vty, " dhcp relay enable\r\n");
		}
	}
#endif

	/* added by chenfj 2007-7-9  ONU  认证使能*/
	{
        ULONG old_mode;
    	ULONG ulsslot;
        ULONG ulsport;
        short int PonPortIdx = 0;
        char portlist_str[256]="";
        
        if(getOnuAuthEnable(0, 0, &old_mode) == VOS_OK && old_mode != V2R1_ONU_AUTHENTICATION_DISABLE)
        {
            vty_out(vty," onu-register authentication enable\r\n");	
        }
        else
        {
            for(ulsslot=PONCARD_FIRST;ulsslot<=PONCARD_LAST;ulsslot++)
            {
                if(SlotCardMayBePonBoard(ulsslot) != ROK)
                    continue;
                for(ulsport=1;ulsport<=PONPORTPERCARD;ulsport++)
                {
            		PonPortIdx = GetPonPortIdxBySlot( (short int)ulsslot, (short int)ulsport );
            		if (PonPortIdx == VOS_ERROR)
                        continue;
                    
                    if(getOnuAuthEnable(ulsslot, ulsport, &old_mode) == VOS_OK && old_mode != V2R1_ONU_AUTHENTICATION_DISABLE)
                        vty_out(vty," onu-register authentication enable pon %d/%d\r\n", ulsslot, ulsport);
                }
            }
        }    
	}

	/* added by chenfj 2007-8-1 
     PON 端口BER/FER告警设置*/
	{
	unsigned int threshold, Num;

	monponBERThrGet(&threshold, &Num );
    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	if(( threshold != 10 ) || ( Num != 10000))
		vty_out(vty, "  pon-port ber alarm threshold %d min-error-bytes %d\r\n", threshold, Num );
#else
	if(( threshold != DEFAULT_PON_BER_THRESHOLD_RATIO ) || ( Num != DEFAULT_PON_BER_THRESHOLD_MINBYTE))
		vty_out(vty, " pon-port ber alarm threshold %d min-error-bytes %d\r\n", threshold, Num );
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */

	monponFERThrGet(&threshold, &Num );
    /* B--modified by liwei056@2010-1-20 for D9624 */
#if 0
	if(( threshold != 10 ) ||( Num != 1000))
		vty_out(vty, "  pon-port fer alarm threshold %d min-error-bytes %d\r\n", threshold, Num );	
	}
#else
	if(( threshold != DEFAULT_PON_FER_THRESHOLD_RATIO ) || ( Num != DEFAULT_PON_FER_THRESHOLD_MINFRAME))
		vty_out(vty, " pon-port fer alarm threshold %d min-error-frames %d\r\n", threshold, Num );	
	}
#endif
    /* E--modified by liwei056@2010-1-20 for D9624 */

	/* added by chenfj 2008-8-25
	     PON 口与ONU 注册绑定
	     */
	{
	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
		vty_out(vty," onu-pon binding enable\r\n");
	}
	
	if( V2R1_watchdog_startup_flag == V2R1_ENABLE )	/* added by xieshl 20090212 */
	{
		vty_out( vty, " config watchdog enable\r\n" );
	}

	{	/* added by xieshl 20081204 */
		short int Times = 0;
		if( Pon_Get_System_parameters(NULL,NULL,NULL, &Times) == ROK )
		{
			if( Times != V2R1_PON_HOST_MSG_OUT )
				vty_out( vty, " host-pon timeout  %d\r\n", Times );
		}
	}
    
	{	/* added by wangdp 20110212 */
		/*if( nme_shutdown_flag==1)
		{
				vty_out( vty, " nme shutdown\r\n");
		}*/
	}	
	#if 0
	{/* for making test flag(V2R1debug_cli.c) */
		int runFlag = 0;
		int defFlag = 0;
		runFlag = GetMakeingTestFlag();
		defFlag = GetMakeingTestFlagDefault();
		if (runFlag != defFlag)
		{
			vty_out( vty, " doorback\r\n");
			if (runFlag == 1)
				vty_out( vty, " making test\r\n");
			else 
				vty_out( vty, " no making test\r\n");
			vty_out( vty, " exit\r\n");
		}
	#if 0
	  接口函数：int SetMakeingTestEvent(int debugFlag )
      初始值获取函数：int GetMakeingTestFlagDefault()
      当前值获取函数：int GetMakeingTestFlag ()

	#endif
	}
	#endif

	if( g_onu_agingtime != g_onu_agingtime_default )
		vty_out( vty, " config onu-agingtime %d\r\n", g_onu_agingtime / 60 );
	if( g_onu_repeated_del_enable )
		vty_out( vty, " config onu-repeated-del enable\r\n" );

	/*onu relay cli mode set*/
	{
	    int type;
	    vty_out(vty, " undo onu cli-relay ALL\r\n");
	    for(type = V2R1_ONU_GT811; type < V2R1_ONU_MAX; type++)
	    {
	        if(isOnuCliRelayMode(type))
	        {
	            char * typestring = getCliRelayOnuTypeString(type);
	            if(typestring)
	                vty_out(vty, " onu cli-relay %s\r\n", typestring);
	        }
	    }
	}

	onuMAC_check_showrun( vty );
	
	{
		ULONG timeout = 0;

		Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_TASKSTATUS,&timeout);
		if(timeout != 0)
			vty_out(vty," ctc-onu port-stats task-start\r\n");	

		Onustats_GetPortStatsTimeOut(ONU_PORTSTATS_ENABLE,&timeout);
		if(timeout != 0)
			vty_out(vty," ctc-onu port-stats enable\r\n");	

		timeout = 0;
		Onustats_GetPortStatsTimeOut(ONU_GETDATA_TIMER,&timeout);
		if(timeout != ONU_GETDATA_TIMER_INTERVAL)
			vty_out(vty," ctc-onu port-stats data-timeout %d\r\n",timeout);	

		Onustats_GetPortStatsTimeOut(ONU_WAKEUP_TIMER,&timeout);
		if(timeout != ONU_WAKEUP_TIMER_INTERVAL)
			vty_out(vty," ctc-onu port-stats wakeup-timeout %d\r\n",timeout/60);	
		
	}
	vty_out( vty, "!\r\n\r\n" );
	return VOS_OK;
}


int olt_init_func()
{
    return VOS_OK;
}

int olt_showrun( struct vty * vty )
{
    /*IFM_READ_LOCK;*/
    OLT_show_run( vty );
    /*IFM_READ_UNLOCK;*/

	/*event_show_run(vty);*/
    return VOS_OK;
}


int olt_config_write ( struct vty * vty )
{
    return VOS_OK;
}

/*struct cmd_node olt_node =
{
    OLT_NODE,
    NULL,
    1
};

LONG olt_node_install()
{
    install_node( &olt_node, olt_config_write);
    olt_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_OLT );
    if ( !olt_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( OLT_NODE );
    return VOS_OK;
}*/

LONG olt_module_init()
{
    struct cl_cmd_module * olt_module = NULL;

    olt_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_OLT );
    if ( !olt_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) olt_module, sizeof( struct cl_cmd_module ) );

    olt_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_OLT );
    if ( !olt_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( olt_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( olt_module->module_name, "olt" );

    olt_module->init_func = olt_init_func;
    olt_module->showrun_func = olt_showrun;
    olt_module->next = NULL;
    olt_module->prev = NULL;
/******Added by suipl 2006/11/10 **********/
	if ( cmd_rugular_register( &stCMD_V2R1_Port_List_Check ) == no_match )
	{
		ASSERT( 0 );
	}
/*	if ( cmd_rugular_register( &stCMD_V2R1_OnuId_List_Check ) == no_match )
	{
		ASSERT( 0 );
	}*/
	if ( cmd_rugular_register( &stCMD_V2R1_OUI_Check ) == no_match )
	{
		ASSERT( 0 );
	}
/****************************************/
    cl_install_module( olt_module );

    return VOS_OK;
}

LONG EPON_show_run( ULONG ulIfindex, VOID * p )
{
	return VOS_OK;
}

/* modified by xieshl 20071108 有些命令在备GFA-SW板上也应该安装，需要单独初始化 */
LONG OLT_CommandInstall_for_standby()
{
	install_element ( DEBUG_HIDDEN_NODE, &set_board_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_board_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &set_sw_ext_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_sw_ext_eeprom_cmd);
	if( (PRODUCT_E_EPON3 != SYS_PRODUCT_TYPE) && (PRODUCT_E_GFA6100 != SYS_PRODUCT_TYPE) )	/* modified by xieshl 20111125, 问题单13994 */
	{
		install_element ( DEBUG_HIDDEN_NODE, &set_chassis_board_cmd);
	}

	/* To support set & read from Master */
	install_element ( LIC_NODE, &set_board_eeprom_cmd);
	install_element ( LIC_NODE, &show_board_eeprom_cmd);
	
	return VOS_OK;
}

LONG OLT_CommandInstall()
{
    install_element ( CONFIG_NODE, &clear_arp_by_onu_enable_cmd);
    install_element ( CONFIG_NODE, &show_olt_reset_info_cmd);
    install_element ( CONFIG_NODE, &erase_olt_reset_info_cmd);
    install_element ( DEBUG_HIDDEN_NODE, &show_olt_reset_info_cmd);
    install_element ( DEBUG_HIDDEN_NODE, &erase_olt_reset_info_cmd);
    if (SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
    {
        install_element ( LIC_NODE, &show_olt_reset_info_cmd);
        install_element ( LIC_NODE, &erase_olt_reset_info_cmd);
    }
    install_element ( CONFIG_NODE, &onu_accept_othre_vendor_ctc_cmd);
    install_element ( CONFIG_NODE, &show_onu_accept_other_vendor_ctc_cmd);
    install_element ( VIEW_NODE, &show_onu_accept_other_vendor_ctc_cmd);
    install_element ( CONFIG_NODE, &config_ctc_onu_telnet_valid_client_cmd);    
    install_element ( CONFIG_NODE, &onu_completely_transmission_enable_cmd);/*added by luh 2011-9-26*/
    install_element ( CONFIG_NODE, &set_pendingQueue_time_cmd);/*added by luh 2013-7-3*/
#if (RPU_MODULE_HOT ==  RPU_YES )
	/*install_element ( CONFIG_NODE, &olt_swboard_switchover_cmd );*/
#endif
 
	/*install_element ( DEBUG_HIDDEN_NODE, &set_board_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_board_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &set_sw_ext_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_sw_ext_eeprom_cmd);*/
	install_element ( DEBUG_HIDDEN_NODE, &set_pon_ext_eeprom_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_pon_ext_eeprom_cmd);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
    if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
    {
    	install_element ( DEBUG_HIDDEN_NODE, &set_tdm_ext_eeprom_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &undo_tdm_ext_eeprom_cmd);
    }
#endif
	/*    install_element ( EPON_NODE, &into_epon_olt_node_cmd);*/
	install_element ( CONFIG_NODE, &olt_device_name_cmd);
	install_element ( CONFIG_NODE, &show_olt_sysfile_ver_cmd);
	install_element ( VIEW_NODE, &show_olt_sysfile_ver_cmd);

	install_element ( CONFIG_NODE, &undo_olt_vendor_name_cmd );
	/*install_element ( CONFIG_NODE, &olt_vendor_add_cmd);*/
	/*install_element ( CONFIG_NODE, &olt_vendor_contac_cmd);*/
	install_element ( CONFIG_NODE, &olt_device_location_cmd);
	install_element ( CONFIG_NODE, &undo_olt_device_location_cmd );
	install_element ( CONFIG_NODE, &show_olt_device_info_cmd);
	/*install_element ( CONFIG_NODE, &show_olt_surrounding_info_cmd);*/
	/*install_element ( CONFIG_NODE, &show_onu_llid_mapping_info_cmd);*/
	install_element ( CONFIG_NODE, &show_history_stats_table_info_cmd);
	/*install_element ( CONFIG_NODE, &alarm_enable_cmd);
	install_element ( CONFIG_NODE, &alarm_threshold_set_cmd);
	install_element ( CONFIG_NODE, &alarm_temperature_threshold_set_cmd);
	install_element ( CONFIG_NODE, &show_alarm_info_cmd);
	install_element ( CONFIG_NODE, &show_last_alarm_info_cmd);
	install_element ( CONFIG_NODE, &show_alarm_log_cmd);*/
	/*install_element ( CONFIG_NODE, &start_self_test_board_cmd);*/
	/*install_element ( CONFIG_NODE, &clear_config_cmd);*/
	/*install_element ( CONFIG_NODE, &ftp_auto_download_cmd);*/
	/*install_element ( CONFIG_NODE, &ftp_auto_download_time_set_cmd);*/

	/*added by wutw at 27 september*/	
	install_element ( CONFIG_NODE, &olt_device_description_cmd);
	install_element ( CONFIG_NODE, &undo_olt_device_description_cmd);
	install_element ( CONFIG_NODE, &olt_show_device_mac_information_cmd);	
	install_element ( CONFIG_NODE, &olt_show_device_onuidx_information_cmd);	
	install_element ( CONFIG_NODE, &onu_show_device_name_information_cmd);	

	install_element ( CONFIG_NODE, &igmpsnooping_auth_Item_add_cmd);	
	install_element ( CONFIG_NODE, &igmpsnooping_auth_Item_add_groups_cmd);	
	
	install_element ( CONFIG_NODE, &igmpsnooping_auth_Item_show_cmd);		
	install_element ( CONFIG_NODE, &igmpsnooping_auth_Item_del_cmd);	
	install_element ( CONFIG_NODE, &igmpsnooping_auth_Item_del_groups_cmd);	
	
	install_element ( CONFIG_NODE, &igmpsnooping_auth_status_set_cmd);	

	/*BEGIN: add by wangxy 2007-06-25*/
	install_element( CONFIG_NODE, &igmpsnooping_auth_gw_Item_add_cmd );
	install_element( CONFIG_NODE, &igmpsnooping_auth_gw_Item_add_groups_cmd );
	install_element ( CONFIG_NODE, &igmpsnooping_auth_gw_Item_del_cmd);
	install_element ( CONFIG_NODE, &igmpsnooping_auth_gw_Item_del_groups_cmd);
	install_element ( CONFIG_NODE, &igmpsnooping_auth_gw_status_set_cmd);	
	install_element( CONFIG_NODE, &igmpsnooping_auth_gw_Item_show_cmd );
	
	/*END*/

	/*BEGIN: add by wangxy 2007-11-19*/
	install_element(CONFIG_NODE, &igmpsnooping_auth_gw_Item_del_all_cmd );
	install_element( CONFIG_NODE, &igmpsnooping_auth_Item_del_all_cmd );
	/*END*/
	
#if 1    
	install_element ( CONFIG_NODE, &bandwidth_onu_police_param_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_police_param_show_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_police_param_undo_cmd ); 
#endif
#if 1    
	install_element ( CONFIG_NODE, &bandwidth_onu_dba_param_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_dba_param_show_cmd );
	install_element ( CONFIG_NODE, &bandwidth_onu_dba_param_undo_cmd ); 
#endif
	/*install_element ( CONFIG_NODE, &olt_software_auto_update_enable_cmd );
	deleted by chenfj 2009-6-3
	与问题单8226 相关:  去掉了设置命令，那么undo 和show 也就没有必要了
	install_element ( CONFIG_NODE, &olt_software_auto_update_disable_cmd );
	install_element ( CONFIG_NODE, &olt_software_auto_update_show_cmd ); */

	/*added by wutw at 9 November
	install_element ( CONFIG_NODE, &onu_online_show_cmd ); */

/* deleted by chenfj 2007-7-26 */
#if 0
	install_element ( CONFIG_NODE, &pon_onu_regiseter_limit_all_config_cmd);
	install_element ( CONFIG_NODE, &pon_no_onu_regiseter_limitconfig_all_cmd);
	install_element ( CONFIG_NODE, &pon_show_onu_regiseter_limit_all_config_cmd);
#endif
	
	/*added by wutw at 2006/11/14*/
	install_element ( CONFIG_NODE, &olt_show_statistic_cmd);
	install_element ( CONFIG_NODE, &olt_history_15m_time_interval_config_cmd);
	install_element ( CONFIG_NODE, &olt_history_24H_time_interval_config_cmd);

	/*added by wutw at 2006/11/17*/
	/*install_element ( CONFIG_NODE, &olt_show_list_onu_cmd);*/

	/*install_element ( CONFIG_NODE, &test_port_list_cmd );*/
	/*install_element ( CONFIG_NODE, &olt_all_auto_updated_show_cmd);*/
	
	install_element ( CONFIG_NODE, &olt_show_list_onu_count_cmd);
	install_element ( VIEW_NODE, &olt_show_list_onu_count_cmd);	
	install_element ( CONFIG_NODE, &olt_onu_infomation_cmd);
	install_element ( VIEW_NODE, &olt_onu_infomation_cmd);    
	install_element ( CONFIG_NODE, &olt_onu_online_show_count_cmd);
	install_element ( VIEW_NODE, &olt_onu_online_show_count_cmd);	
	install_element ( CONFIG_NODE, &olt_onu_offline_show_count_cmd);   
	install_element ( VIEW_NODE, &olt_onu_offline_show_count_cmd);
	install_element ( CONFIG_NODE, &olt_del_offline_onu_config_cmd);
	install_element ( VIEW_NODE, &olt_show_list_onu_by_mac_cmd);
	install_element ( VIEW_NODE, &olt_show_list_onu_by_sn_cmd);
	install_element ( CONFIG_NODE, &olt_show_list_onu_by_mac_cmd);
	install_element ( CONFIG_NODE, &olt_show_list_onu_by_sn_cmd);
    

	/*install_element ( CONFIG_NODE, &olt_loop_info_show_cmd);*/

	/*added by wutw 2006/11/15*/
	/* 
	问题单6222: 建议将onu-register window命令调整到隐藏节点下
	*/
	/*install_element ( DEBUG_HIDDEN_NODE, &olt_range_set_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &olt_range_show_cmd);*/

	/*added by wangxy 2007-03-12*/
	install_element ( CONFIG_NODE, &app_file_show_cmd);
	install_element ( CONFIG_NODE, &driver_file_show_cmd );
	install_element ( CONFIG_NODE, &pononu_version_show_cmd);
	install_element ( CONFIG_NODE, &onu_version_show_cmd);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
    if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
    {
    	/*added by wangxiaoyu 2008-01-15*/
    	install_element ( CONFIG_NODE, &upgrade_tdm_cmd);
    }
#endif
/*add by shixh for systerm file upload and download*/
	/*install_element ( CONFIG_NODE, &download_sysfile_cmd);*/
	/*install_element ( CONFIG_NODE, &upload_sysfile_cmd);*/
	/* 增加激活和显示sysfil命令 chenfj */
	install_element ( CONFIG_NODE, &active_sysfile_cmd);
	/*install_element ( CONFIG_NODE, &show_sysfile_cmd);*/
	install_element ( DEBUG_HIDDEN_NODE, &show_sysfile_cmd);	
	install_element ( VIEW_NODE, &show_sysfile_cmd);

	/* added by chenfj 2007-6-21  增加PON 端口保护切换*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
	install_element ( DEBUG_HIDDEN_NODE, &pon_swap_switch_config_cmd );
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
	install_element ( CONFIG_NODE, &set_pon_port_hot_swap_cmd);
#if ( (EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES) || (EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES) )
	install_element ( CONFIG_NODE, &set_pon_port_hot_swap_full_cmd);
	install_element ( CONFIG_NODE, &show_pon_port_hot_swap_mode_cmd);
#endif
	install_element ( CONFIG_NODE, &undo_set_pon_port_hot_swap_cmd );
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &op_trigger_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &op_untrigger_pon_port_hot_swap_cmd );
#endif
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &oe_trigger_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &oe_untrigger_pon_port_hot_swap_cmd );
#endif
#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &ud_trigger_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &ud_untrigger_pon_port_hot_swap_cmd );
#endif
	install_element ( CONFIG_NODE, &force_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &sync_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &show_pon_port_hot_swap_cmd );
	install_element ( CONFIG_NODE, &force_pon_port_hot_swap_timer_cmd );
	install_element ( CONFIG_NODE, &show_force_pon_port_hot_swap_timer_cmd );
/* B--added by liwei056@2012-8-7 for D14913 */
	install_element ( CONFIG_NODE, &show_pon_port_trigger_swap_cmd );
/* E--added by liwei056@2012-8-7 for D14913 */
#ifdef __CTC_TEST
	install_element ( CONFIG_NODE, &pon_switch_config_cmd );
	install_element ( CONFIG_NODE, &show_force_switch_config_cmd );
#endif
#endif

/* B--added by liwei056@2012-3-6 for 国电ONU倒换功能 */
#if ( EPON_MODULE_ONU_HOT_SWAP == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &force_pon_onu_hot_swap_cmd );
	install_element ( CONFIG_NODE, &resume_pon_onu_hot_swap_cmd );
#endif
/* E--added by liwei056@2012-3-6 for 国电ONU倒换功能 */


	/* added by chenfj 2007-7-6
     		for onu authenticaion by PAS5001 & PAS5201 */
	install_element ( CONFIG_NODE, &set_onu_authentication_cmd );
	install_element ( CONFIG_NODE, &undo_set_onu_authentication_cmd );
	install_element ( CONFIG_NODE, &show_onu_authentication_cmd );
	install_element ( CONFIG_NODE, &add_onu_authentication_entry_cmd );
	install_element ( CONFIG_NODE, &delete_onu_authentication_entry_cmd );
	install_element ( CONFIG_NODE, &show_onu_authentication_entry_cmd );
	install_element ( CONFIG_NODE, &reorganize_onu_authentication_entry_cmd );

	install_element ( CONFIG_NODE, &add_gonu_authentication_entry_cmd );
	install_element ( CONFIG_NODE, &delete_gonu_authentication_entry_cmd );
	install_element ( CONFIG_NODE, &show_gonu_authentication_entry_cmd );

	install_element ( VIEW_NODE, &set_onu_authentication_cmd );
	install_element ( VIEW_NODE, &undo_set_onu_authentication_cmd );
	install_element ( VIEW_NODE, &add_onu_authentication_entry_cmd );
	install_element ( VIEW_NODE, &delete_onu_authentication_entry_cmd );
	install_element ( VIEW_NODE, &add_gonu_authentication_entry_cmd );
	install_element ( VIEW_NODE, &delete_gonu_authentication_entry_cmd );
	install_element ( VIEW_NODE, &show_gonu_authentication_entry_cmd );

	/* added by chenfj 2007-6-27  
     PON 端口管理使能: up / down , reset 
     ONU 激活、去激活操作*/
	install_element ( CONFIG_NODE, &shutdown_pon_port_cmd );
	install_element ( CONFIG_NODE, &undo_shutdown_pon_port_cmd );
	/* added by wangjiah@2016-08-12 : begin*/
	//install_element ( CONFIG_NODE, &reset_pon_port_cmd );/*merge into reset_pon_chip_cmd*/
	//install_element ( CONFIG_NODE, &reset_10gepon_chip_cmd);
	/* added by wangjiah@2016-08-12 : end*/
	install_element ( CONFIG_NODE, &deactivate_onu_device_cmd );
	install_element ( CONFIG_NODE, &undo_deactivate_onu_device_cmd );
	install_element ( CONFIG_NODE, &show_deactivate_onu_device_cmd );

	install_element ( CONFIG_NODE, &olt_onu_agingtime_config_cmd);
	install_element ( CONFIG_NODE, &olt_onu_agingtime_day_config_cmd);
	install_element ( CONFIG_NODE, &olt_onu_agingtime_show_cmd);
    /*added by luh@2014-10-27*/
	install_element ( VIEW_NODE, &olt_onu_agingtime_show_cmd);
	
	install_element ( CONFIG_NODE, &olt_onu_repeated_del_config_cmd);
	install_element ( CONFIG_NODE, &olt_onu_repeated_del_show_cmd);

	/* added by chenfj 2007-7-31 
	    PON端口BER / FER 告警 */
	install_element ( CONFIG_NODE, &pon_port_ber_alarm_threshold_cmd );
	install_element ( CONFIG_NODE, &pon_port_ber_alarm_threshold_no_cmd );
	install_element ( CONFIG_NODE, &pon_port_fer_alarm_threshold_cmd );
	install_element ( CONFIG_NODE, &pon_port_fer_alarm_threshold_no_cmd );
	install_element ( CONFIG_NODE, &show_pon_port_berfer_alarm_threshold_cmd );
	install_element ( CONFIG_NODE, &onu_pon_ber_fer_alarm_enable_cmd );
	/*install_element ( CONFIG_NODE, &onu_pon_ber_fer_alarm_disable_cmd );*/
	install_element ( CONFIG_NODE, &show_onu_pon_ber_fer_alarm_cmd );
	install_element ( CONFIG_NODE, &olt_pon_ber_fer_alarm_enable_cmd );
	/*install_element ( CONFIG_NODE, &olt_pon_ber_fer_alarm_disable_cmd );*/
	install_element ( CONFIG_NODE, &show_olt_pon_ber_fer_alarm_cmd );

	/* PON芯片硬件复位命令*/
	install_element ( CONFIG_NODE, &reset_pon_chip_cmd );
	/*install_element ( CONFIG_NODE, &update_onu_file_cmd );*/

	/*  added by chenfj 2008-4-29
	  11. 命令行作用户权限分级（徐州提的需求），改进部分。（不增加用户权限，扩展normal用户的查询权限，即在VIEW节点下增加查询命令）
	  */
	  /* 以下命令由config 节点扩展到view 节点*/
	install_element ( VIEW_NODE, &show_olt_device_info_cmd);
	install_element ( VIEW_NODE, &show_history_stats_table_info_cmd);
	/*deleted by chenfj 2009-6-3
	与问题单8226 相关:  去掉了设置命令，那么show 也就没有必要了
	install_element ( VIEW_NODE, &olt_software_auto_update_show_cmd );*/

	install_element ( VIEW_NODE, &olt_show_device_mac_information_cmd);	
	install_element ( VIEW_NODE, &olt_show_device_onuidx_information_cmd);	
	install_element ( VIEW_NODE, &onu_show_device_name_information_cmd);

	install_element ( VIEW_NODE, &igmpsnooping_auth_gw_Item_show_cmd );
	install_element ( VIEW_NODE, &igmpsnooping_auth_Item_show_cmd);
	/*install_element ( VIEW_NODE, &olt_show_statistic_cmd);*/

	/*install_element ( VIEW_NODE, &olt_loop_info_show_cmd);*/
	/*install_element ( VIEW_NODE, &olt_range_show_cmd);*/

	install_element ( VIEW_NODE, &app_file_show_cmd);
	install_element ( VIEW_NODE, &driver_file_show_cmd );

	install_element ( VIEW_NODE, &onu_version_show_cmd);
	install_element ( VIEW_NODE, &pononu_version_show_cmd);
	
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	install_element ( VIEW_NODE, &show_pon_port_hot_swap_cmd );
#endif

	install_element ( VIEW_NODE, &show_onu_authentication_cmd );
	install_element ( VIEW_NODE, &show_onu_authentication_entry_cmd );
	install_element ( VIEW_NODE, &show_deactivate_onu_device_cmd );

	install_element ( CONFIG_NODE, &v2r1_watchdog_enable_cmd);
	install_element ( CONFIG_NODE, &v2r1_watchdog_show_cmd);
	install_element ( VIEW_NODE, &v2r1_watchdog_show_cmd);

	if( SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6100 )
		install_element ( DEBUG_HIDDEN_NODE, &v2r1_cpld_debug_show_cmd);

	install_element ( CONFIG_NODE, &v2r1_ONU_default_bandwidth_cmd);
	install_element ( CONFIG_NODE, &v2r1_ONU_default_bandwidth_undo);
	install_element ( CONFIG_NODE, &v2r1_ONU_default_bandwidth_show);
    /*added by luh@2014-10-27*/
	install_element ( VIEW_NODE, &v2r1_ONU_default_bandwidth_show);
    
	install_element ( DEBUG_HIDDEN_NODE, &wind_view_start_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &wind_view_stop_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &wind_view_show_cmd);
	if (SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
	{
		install_element ( LIC_NODE, &wind_view_start_cmd );
		install_element ( LIC_NODE, &wind_view_stop_cmd );
		install_element ( LIC_NODE, &wind_view_show_cmd );
	}
	/*install_element ( CONFIG_NODE, &nme_shutdown_cmd);
	install_element ( CONFIG_NODE, &undo_nme_shutdown_cmd);*/
	install_element ( DEBUG_HIDDEN_NODE, &test_syslog_olt_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &test_delay_olt_cmd);
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
    	install_element ( DEBUG_HIDDEN_NODE, &test_card_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &test_port_cmd);
    }
	install_element ( DEBUG_HIDDEN_NODE, &show_pontx_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_ponif_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_onuif_cmd);

	install_element( CONFIG_NODE, &show_ctc_oui_ver_rec_cmd );

	if (SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
	{
		install_element ( DEBUG_HIDDEN_NODE, &show_olt_power_info_cmd);
		install_element ( DEBUG_HIDDEN_NODE, &show_olt_power_reg_cmd);
		install_element ( DEBUG_HIDDEN_NODE, &write_olt_power_reg_cmd);
	}
	
#ifdef SYS_DUMP
	install_element ( DEBUG_HIDDEN_NODE, &pon_dump_cmd);
#endif
	dosFs_CommandInstall();
	
#if 0
	install_element ( VIEW_NODE, &start_pon_port_test_cmd);
	install_element ( VIEW_NODE, &stop_pon_port_test_cmd);

	install_element ( VIEW_NODE, &start_pas_buffer_test_cmd);
	install_element ( VIEW_NODE, &stop_pas_buffer_test_cmd);

	install_element ( CONFIG_NODE, &start_pas_log_cmd);
#endif
    return VOS_OK;
}

#if 0
struct cmd_node epon_node =
{
    EPON_NODE,
    NULL,
    1
};

 
int epon_init_func()
{
    return VOS_OK;
}

int epon_showrun( struct vty * vty )
{
	int iRes = VOS_OK;
	{
	char defname[255] = {0};
	int defnameLen = 0;
	char runname[255] = {0};
	int runLen = 0;
	
	if(VOS_OK != GetOltDeviceNameDefault(&defname, &defnameLen))
		{
			return (-1);
		}
   	iRes = GetOltDeviceName(&runname,&runLen);
		{
			return (-1);
		}
	if (VOS_OK != strcmp(defname, runname))
		{
			/*vty_out (vty, "  devicename runname\r\n");*/
		}
	/*对比两个字符串*/
	}

	return VOS_OK;
}


int epon_config_write ( struct vty * vty )
{
    return VOS_OK;
}
LONG epon_node_install()
{
    install_node( &epon_node, epon_config_write);
    epon_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_EPON);
    if ( !epon_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( EPON_NODE );
    return VOS_OK;
}

LONG epon_module_init()
{
    struct cl_cmd_module * epon_module = NULL;

    epon_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_EPON);
    if ( !epon_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) epon_module, sizeof( struct cl_cmd_module ) );

    epon_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_EPON );
    if ( !epon_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( epon_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( epon_module->module_name, "epon" );

    epon_module->init_func = epon_init_func;
    epon_module->showrun_func = epon_showrun;
    epon_module->next = NULL;
    epon_module->prev = NULL;

    cl_install_module( epon_module );

    return VOS_OK;
}

LONG EPON_CommandInstall()
{
    install_element ( CONFIG_NODE, &into_epon_node_cmd);    
        
    return VOS_OK;
}

LONG EPON_CliInit()
{  
    epon_node_install();
    epon_module_init();
    EPON_CommandInstall();
    
    return VOS_OK;
}

#endif

extern LONG (*cdsms_epv2r1_ONU_CliInit)( VOID );
extern void authenticationModeInit();
extern int cl_osinfo_cmd_init();

LONG EPON_V2R1_CliInit()
{
    /*EPON_CliInit();

    olt_node_install();*/  
    
	/*shell cmd init for pon card*/
	if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE) 
	{
		cl_osinfo_cmd_init();
	}
    olt_module_init();
    OLT_CommandInstall();
    authenticationModeInit();/*new added by luh 2012-2-13*/
    PON_CliInit();
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
    if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
    {
        /*if(SlotCardIsTdmSgBoard(get_gfa_tdm_slotno()) == ROK)*/
        TDM_CliInit();
        /*else if(SlotCardIsTdmE1Board(get_gfa_tdm_slotno()) == ROK)*/
        /* begin: added by jianght 20090223  */
       	E1_CliInit();
        /* end: added by jianght 20090223 */
    }
#endif
    /* ONU_CliInit(); */
    if (cdsms_epv2r1_ONU_CliInit != NULL)
		(*cdsms_epv2r1_ONU_CliInit) ( );
	
#if ((defined ONU_PPPOE_RELAY)||(defined ONU_DHCP_RELAY))
    ACCESS_Id_Cli_Init();
#endif
    /*ONUFE_CliInit();*/
    EPON_DEBUG_CommandInstall();
	/*CommOltMsgRvcCallbackInit(1, (void *)CommOlt_HadleCli);*/
    
    onuMAC_check_init();  /*add by sxh20020201*/

    return VOS_OK;
}



        
#ifdef	__cplusplus
}
#endif/* __cplusplus */
