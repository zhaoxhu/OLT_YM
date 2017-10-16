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
*	modified by wutw at 19 October
*		增加命令:show register info <slot/port>
*		19 October : 增加making test show run
*		19 October : 增加oam 模块err debug 开关，oam 接收数据包track debug开关
*					 增加oam 模块收发包
*		20 October : 增加pon端口的实时统计显示命令
*		24 October : 增加oam模块队列查看相关命令
*		2006/11/23 : pclint 检查，修改部分代码
*		2006/11/24 : 在oam debug 与undo oam debug命令增加sending选项
*		2006/12/27 : 增加show onu-llid mapping <slot/port> <onuid_list>命令
*
* 	1 modified by chenfj 2007-7-16
*		增加一个带宽比例因子,用于测试带宽精度
*				
*****************************************************************************/
#include "syscfg.h"
#include "../EPONV2R1/GwttOam/OAM_gw.h"
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
#include "mn_oam.h"

#include "ifm/ifm_debug.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"
#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"
#include "V2R1debug_cli.h"
#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"
#include "includeFromTk.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "tdm_comm/tdm_comm.h"
#endif

#define	CLI_EPON_PONMAX		(MAXPON-1)  /*19*/
#define	CLI_EPON_PONMIN		0
#define	CLI_EPON_CARDINSERT	CARDINSERT  /*1*/


#ifndef  CLI_EPON_ONUMAX
#define	CLI_EPON_ONUMIN						0
#define	CLI_EPON_ONUMAX						(MAXONUPERPON-1) /*63*/
#endif
/*extern short int GetPonPortIdxBySlot( short int slot, short  int port );*/

#define	CLI_EPON_ONUUP					ONU_OPER_STATUS_UP	/*1*/
#define	CLI_EPON_ONUDOWN                          ONU_OPER_STATUS_DOWN      /*2*/
/*
typedef enum{
	PONPORT_UP=1,
	PONPORT_DOWN,
	PONPORT_UNKNOWN,
	PONPORT_LOOP,
	PONPORT_UPDATE,
	PONPORT_INIT,
	PONPORT_DEL
}CLI_DEBUG_PONPORT_OPER;
*/
extern int CommOamTastQueueInfoVty(struct vty *vty);
extern int CommOltMsgQueueInfoVty(short int PonId,struct vty *vty);
extern int CommOltMsgRvcDebugTurnOnvty(unsigned char GwProId,struct vty *vty);
extern int CommOltMsgRvcDebugTurnOffvty(unsigned char GwProId,struct vty *vty);
extern int CommOltMsgRvcDebugTurnVtyShow(struct vty *vty);
extern int CommOltMsgSendingContentDebug(int Done);
extern int statOamCounterVty(struct vty *vty);
extern int OamDebugTrack(short int status);
extern int OamDebugErr(short int status);
extern int statOamclear(void);
extern int CommOamTaskQueueClear(unsigned char aM_uint8_QueIdx,UINT32  lM_uint32_Idx);
extern int ShowOnuLlidMappingByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern int ShowOnuLlidMappingByVty_1( short int PonPortIdx, short int OnuIdx, struct vty *vty );

extern LONG GetMacAddr(CHAR * szStr, CHAR * pucMacAddr);

extern LONG  lCli_SendbyOam( INT16 ponID, INT16 onuID,	UCHAR  *pClibuf,	USHORT length,cliPayload *stSessionIdPayload ,	struct vty * vty);
extern LONG  PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern int  CommOamCallbackInofVty( struct vty *vty);
extern int CommOamTaskTestingInfoGet(unsigned int *pSendCount, unsigned int *pRevCount);
extern int statDebugOnuRealTimeStatisticsVty(short int slotId, short int port, short int PonId,short int OnuId,struct vty *vty);
extern int statDebugOltRealTimeStatisticsVty(short int slotId, short int port, short int PonId,short int OnuId,struct vty *vty);
/*extern int EuqoamSend(unsigned long count);*/
/*
#define MAX_ONUID_NUMBER_IN_ONUID_LIST			MAXONUPERPON  
#define START_PORT								1
#define END_PORT								MAX_ONUID_NUMBER_IN_ONUID_LIST
*/

extern short int g_sLogPonSoftLLID;
extern short int g_sLogPonSoftOltID;
extern unsigned char  g_ucLogPonSoftEnable;



unsigned char *onu_emapper_string[] = 
{
	/* 0 -- 9 */
	"cookie      : ",
	"eramrd      : ",
	"eramwr      : ",
	"eramsz      : ",
	"fimage      : ",
	"simage      : ",
	"bmon        : ",
	"mansec      : ",
	"flashrd     : ",
	"flashwr     : ",
	/* 10 -19 */
	"flashvndr   : ",
	"sdraminfo   : ",
	"sdramref    : ",
	"sdramtcas   : ",
	"sdramtras   : ",
	"sdramwd     : ",
	"sdramrbc    : ",
	"",
	"",
	"",
	
	/*
	"rebootcount : ",
	"dgpol       : ",
	"lfpol       : ",
	"cepol       : ",
	*/
	/* 20 - 29 */ 	
	"eramty      : ",
	"bootmode    : ",
	"baud        : ",
	"ponsgnl     : ",
	"pontbc      : ",
	"lasertx     : ",
	"",
	"",
	"zbtu        : ",
	"zbtc        : ",
	/* 30 - 39 */
	"ponpwr      : ",
	"oui         : ",
	"mac         : ",
	"ip          : ",
	"netmask     : ",
	"laserton    : ",
	"lasertoff   : ",
	"adven       : ",
	"brg         : ",
	"autoneg     : ",
	/*40- 49 */
	"phyaddr     : ",
	"iftype      : ",
	"master      : ",
	"advmp       : ",
	"advgh       : ",
	"advgf       : ",
	"advpa       : ",
	"advpe       : ",
	"advt4       : ",
	"advhf       : ",
	/* 50 - 59 */
	"advhh       : ",
	"advtf       : ",
	"advth       : ",
	"thrsh       : ",
	"uthrsh      : ",
	"ptop        : ",
	"grant       : ",
	"laseronp    : ",
	"pontxdis    : ",
	"idle0       : ",
	/* 60 - 61 */
	"idle1       : ",
	"idle2       : ",
	"idle3       : ",
	"idlepre     : ",
	"user        : ",
	"passwd      : ",
	"gen1        : ",
	"gen2        : ",
	"ponfine     : ",
	"tsfec       : ",
	/* 70 - 79 */
	"arbdelta    : ",
	"txdly       : ",
	"rxdly       : ",
	"dgpol       : ",
	"lfpol       : ",
	"cepol       : "
	/*  GT812 中增加了这几项 
	"sdrambrlen  : "                                                           
	"sdrambrt    : "                                                           
	"sdramwbrm   : " 
	"tbisdpol     : "                                                  
	"tbcpol      : "
	*/
	

};

unsigned char *onu_emapper_string_list[] =
{
/* 0 */
"",
/* 1 -10 */
" 1 cookie      - EEPROM presence ID cookie,0xda-eeprom is present, other-eeprom is not present",
" 2 eramrd      - External RAM's read wait-states,range:0-15",
" 3 eramwr      - External RAM's write wait-states,range:0-15",
" 4 eramsz      - External RAM size(in MB for SDRAM and in KB for SRAM,Zero if not present)",
" 5 fimage      - First image start address in KB",
" 6 simage      - Second image start address in KB",
" 7 bmon        - Boot loader start address in KB",
" 8 mansec      - Management secs start address in KB",
" 9 flashrd     - FLASH read wait-states,number of bus clocks,range:0-15",
"10 flashwr     - FLASH write wait-states,number of bus clocks,range:0-15",
/* 11 - 20 */
"11 flashvndr   - FLASH vendor code,0 for AMD, 2 for CFI",
"12 sdraminfo   - SDRAM index of parameters,range:0-15",
"13 sdramref    - REFRESH cycle period [num of 16xHCLKs],range:0-1023",
"14 sdramtcas   - SDRAM tCAS,range:0-3",
"15 sdramtras   - SDRAM tRAS,range:0-3",
"16 sdramwd     - SDRAM Width Mode,range:0-3",
"17 sdramrbc    - SDRAM RBC or BRC addressing schem,0-RBC,1-BRC",
"18 rebootcount - Number of times before switch images,Note:not supported by current PAS-SOFT package",
"19 reserved",
"20 eramty      - External ram type in use (0=SDRAM, 1=SRAM)",
/* 21 - 30 */
"21 bootmode    - Image configuration,0-single image,1-dual image,3-enhanced,other-reserved",
"22 baud        - UART's baud-rate index(0-1200,1-2400,2-4800,3-9600,4-14000,5-19200,6-28800,7-38400,8-57600,9-115200,A-230400,B-460800)",
"23 reserved",
"24 ponsgnl     - PON loss signal polarity(0-base polarity,1-shifted 180 degrees)",
"25 pontbc      - PON TBC signal polarity(0-base polarity,1-shifted 180 degrees)",
"26 lasertx     - Laser TX enable polarity(0-base polarity,1-shifted 180 degrees)",
"27 dgpol       - DYING GASP polarity(Not used for PAS6201 A2)(0-active low,1-active high)",
"28 lfpol       - LINK FAULT polarity(Not used for PAS6201 A2)(0-active low,1-active high)",
"29 zbtu        - Use ZBT(Not used for PAS6201 A2)(0-no ZBT memory,1-use ZBT memory)",
"30 zbtc        - ZBT config (Not used for PAS6201 A2)(0=Flowtrhu, 1=Pipeline)",
/* 31 - 40 */
"31 ponpwr      - PON Connect on power up(0-dont connect on power up,1-connect on power up)",
"32 oui         - OUI value(24bit)",
"33 mac         - MAC address(as H.H.H)",
"34 ip          - IP Address(as x.x.x.x)",
"35 netmask     - Net Mask(as x.x.x.x)",
"36 laserton    - Laser on time in 16nSec quanta",
"37 lasertoff   - Laser off time in 16nSec quanta,should be bigger than zero to avoid laser kept always ON",
"38 adven       - UNI phy advertising enable(0-dont advertise,1-advertise)",
"39 brg         - UNI bridge enable(0-UNI link is always ON,1-UNI link status is according to PHY status register)",
"40 autoneg     - UNI auto-neg enable(0-disable,1-enable)",
/* 41 - 50 */
"41 phyaddr     - UNI MDIO external PHY address",
"42 iftype      - UNI MAC type (0-GMII or MII, 1-TBI)",
"43 master      - UNI Master mode (0=slave, 1=master)",
"44 advmp       - UNI advertise 1000t multi-port(0-dont advertise,1-advertise)",
"45 advgh       - UNI advertise 1000t half duplex(0-dont advertise,1-advertise)",
"46 advgf       - UNI advertise 1000t full duplex(0-dont advertise,1-advertise)",
"47 advpa       - UNI advertise pause asymmetric(0-dont advertise,1-advertise)",
"48 advpe       - UNI advertise pause enabled(0-dont advertise,1-advertise)",
"49 advt4       - UNI advertise 100t4(0-dont advertise,1-advertise)",
"50 advhf       - UNI advertise 100tx FD(0-dont advertise,1-advertise)",
/*  50 - 59 */
"51 advhh       - UNI advertise 100tx HD(0-dont advertise,1-advertise)",
"52 advtf       - UNI advertise 10tx FD(0-dont advertise,1-advertise)",
"53 advth       - UNI advertise 10tx HD(0-dont advertise,1-advertise)",
"54 thrsh       - threshhold mode (0=Disable, 1=mode 5, 2=mode 8)",
"55 uthrsh      - unify threshhold mode (0=not unify, 1=unify)",
"56 cepol       - CRITICAL EVENT polarity(Not used for PAS6201 A2)(0-active low,1-active high)",
"57 ptop        - Point to Point enable(0-disable,1-enable)",
"58 grant       - Granted always(0-disable,1-enable)",
"59 laseronp    - Laser on permanently(0-disable,1-enable)",
"60 pontxdis    - PON TX disable data format (1-idle code,0-zeros)",
/*  61 - 69 */
"61 idle0       - Idle Byte0 (8/10 coding) (0-Ctrl, 1-Data)",
"62 idle1       - Idle Byte1 (8/10 coding) (0-Ctrl, 1-Data)",
"63 idle2       - Idle Byte2 (8/10 coding) (0-Ctrl, 1-Data)",
"64 idle3       - Idle Byte3 (8/10 coding) (0-Ctrl, 1-Data)",
"65 idlepre     - Idle Preamble data",
"66 user        - 802.1X user name,max length=32byte",
"67 passwd      - 802.1X password,max length=32byte",
"68 gen1        - General purpose 1 data",
"69 gen2        - General purpose 2 data",
"70 ponfine     - PON clock fine tune delta,range is -16 to 15",
/* 71 - 80 */
"71 tsfec       - Timestamp delay for FEC delta,range is -16 to 15",
"72 arbdelta    - Arbitrator to PON timestamp delta,range is -16 to 15",

"73 txdly       - PON-clock Tx-calibration for encryption.,range is -16 to 15",
"74 rxdly       - PON-clock Rx-calibration for encryption.,range is -16 to 15",

/*  GT812 */
"sdrambrlen  - SDRAM burst length",
"sdrambrt    - SDRAM burst type (0=Seq, 1=Inter)",
"sdramwbrm   - SDRAM burst mode (0=program len, 1=Single Location)",
"tbisdpol    - TBI SD polarity",
"tbcpol      - TBI TBC polarity"

};

/*******Debug Point**************************/

/*******Register operate*****************************/

/*  deleted by chenfj 2008-7-21
DEFUN( read_port_statistic,
        read_port_statistic_cmd,
        "read port statistic <slotid/portid>",
        "Read information\n"
        "Read information of the port\n"
        "Read the statistic information fo the port\n"
        "Please input the slot ID and the portid\n"
        )
{
        
    return CMD_SUCCESS;
}

DEFUN( write_register_func,
        write_register_func_cmd,
        "write register <address> <value>",
        "Write information\n"
        "Write information to the register\n"
        "Please input the address of the register( e.g.: 0x1e2d3f4a )\n"
        "Please input the value\n"
        )
{
        
    return CMD_SUCCESS;
}

DEFUN( read_register_func,
        read_register_func_cmd,
        "read register <address>",
        "Read information\n"
        "Read information from the register\n"
        "Please input the address of the register( e.g.: 0x1e2d3f4a )\n"
        )
{
        
    return CMD_SUCCESS;
}
*/
/*******Entity management*****************/
/*
DEFUN( show_olt_entity_infomation,
        show_olt_entity_infomation_cmd,
        "show olt entity information",
        DescStringCommonShow
        "Show the olt\n"
        "Show the olt entity\n"
        "Show the information of olt entity\n"
        )
{
        
    return CMD_SUCCESS;
}

DEFUN( show_pon_entity_infomation,
        show_pon_entity_infomation_cmd,
        "show pon entity information <slot/port>",
        DescStringCommonShow
        "Show the pon\n"
        "Show the pon entity\n"
        "Show the information of the pon entity\n"
        "Please input the pon entity's slot and port(e.g.: 1/3) \n"
        )
{
        
    return CMD_SUCCESS;
}

DEFUN( show_onu_entity_infomation,
        show_onu_entity_infomation_cmd,
        "show onu entity information <onuid>",
        DescStringCommonShow
        "Show the onu\n"
        "Show the onu entity\n"
        "Show the information of the onu entity\n"
        "Please input the onu entity's onu ID\n"
        )
{
        
    return CMD_SUCCESS;
}
*/
DEFUN( oam_debug_on_config,
        oam_debug_on_config_cmd,
        "oam debug [cli|snmp|snmptrap|euqinfo|filetran|igmpauth|alarmlog|vconsole|all]",
        "config oam module debug on\n"
        "config oam module debug on\n"
        "config oam cli debug on\n"
        "config oam snmp debug on\n"
        "config oam snmptrap debug on\n"
        "config oam euq debug on\n"
        "config oam filetran debug on\n"
        "config oam igmpauth debug on\n"
        "config oam alarmlog debug on\n"
        "config oam vconsole(pty) debug on\n"
        "config oam all debug on\n"
        /*"config oam sending msg debug on\n"*/
        )
{
	unsigned char GwProId = 0;
	if (!VOS_StrCmp((CHAR *)argv[0], "cli"))
		 GwProId = GW_DEBUG_CLI;
	else if (!VOS_StrCmp((CHAR *)argv[0], "snmp"))
		GwProId = GW_DEBUG_SNMP;
	else if (!VOS_StrCmp((CHAR *)argv[0], "snmptrap"))
		GwProId = GW_DEBUG_SNMP_TRAP;
	else if (!VOS_StrCmp((CHAR *)argv[0], "euqinfo"))
		GwProId = GW_DEBUG_EUQINFO;
	else if (!VOS_StrCmp((CHAR *)argv[0], "filetran"))
		GwProId = GW_DEBUG_FILE;
	else if (!VOS_StrCmp((CHAR *)argv[0], "igmpauth"))
		GwProId = GW_DEBUG_IGMPAUTH;
	else if (!VOS_StrCmp((CHAR *)argv[0], "alarmlog"))
		GwProId = GW_DEBUG_ALARMORLOG;
	else if (!VOS_StrCmp((CHAR *)argv[0], "vconsole"))
		GwProId = GW_DEBUG_VCONSOLE;
	else if (!VOS_StrCmp((CHAR *)argv[0], "all"))
		GwProId = GW_DEBUG_ALL;
	else if (!VOS_StrCmp((CHAR *)argv[0], "sending"))
	{
		CommOltMsgSendingContentDebug( 1 );	
		return CMD_SUCCESS;
	}
	else
	{
		/*vty_out( vty,"  %% Parameter is error\r\n");*/
		return CMD_WARNING;
	}
	if(GwProId == GW_DEBUG_ALL)
		CommOltMsgSendingContentDebug( 1 );
	CommOltMsgRvcDebugTurnOnvty(GwProId,vty);
    return CMD_SUCCESS;
}


DEFUN( oam_debug_err_config,
        oam_debug_err_config_cmd,
        "oam err-debug",
        "config oam err info debug on\n"
        "config oam err info debug on\n"
        )
{
	OamDebugErr(1);
    return CMD_SUCCESS;
}


DEFUN( oam_debug_track_config,
        oam_debug_track_config_cmd,
        "oam track-debug",
        "config oam packet tracke info debug on\n"
        "config oam err info debug on\n"
        )
{
	OamDebugTrack(1);
	return CMD_SUCCESS;
}



DEFUN( oam_debug_err_off_config,
        oam_debug_err_off_config_cmd,
        "undo oam err-debug",
        NO_STR
        "config oam err info debug on\n"
        "config oam err info debug on\n"
        )
{
	OamDebugErr(0);
    return CMD_SUCCESS;
}

DEFUN( oam_debug_track_off_config,
        oam_debug_track_off_config_cmd,
        "undo oam track-debug",
        NO_STR
        "config oam packet tracke info debug on\n"
        "config oam err info debug on\n"
        )
{
	OamDebugTrack(0);
	return CMD_SUCCESS;
}

DEFUN(
		oam_stat_show,
        oam_stat_show_cmd,
        "show oam-statistics",
		DescStringCommonShow
		"show oam statistics info\n"
        )
{
	 
	statOamCounterVty(vty);
	return CMD_SUCCESS;
}

DEFUN(
		oam_Queue_show,
        oam_Queue_show_cmd,
        "show oam-frame-queue <slot/port>",
		DescStringCommonShow
		"show oam rev queue info\n"
		"Please int slot/port\n"
        )
{
	ULONG ulSlotId = 0;
	ULONG ulPort = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &ulSlotId, &ulPort);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlotId,ulPort,vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlotId) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ulSlotId,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlotId, ulPort);
		return(CMD_WARNING);
		}
	ponId = GetPonPortIdxBySlot( (short int)ulSlotId, (short  int)ulPort );
	if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;    	
		}
	CommOltMsgQueueInfoVty(ponId,vty);
	return CMD_SUCCESS;
}

extern int CommOltMsgQueueClear(short int PonId,short int OnuId);

DEFUN(
		oam_frame_Queue_clear,
        oam_frame_Queue_clear_cmd,
        "clear oam-frame-queue <slot/port>",
		"clear config\n"
		"clear Rev queue of oam Frame\n"
		"Please int slot/port\n"
        )
{
	ULONG ulSlotId = 0;
	ULONG ulPort = 0;
	short int ponId = 0;
	short int OnuId = 0;
	
	sscanf( argv[0], "%d/%d", &ulSlotId, &ulPort);
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlotId,ulPort,vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlotId) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ulSlotId,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlotId, ulPort);
		return(CMD_WARNING);
		}
	ponId = GetPonPortIdxBySlot( (short int)ulSlotId, (short  int)ulPort );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	/* modified by chenfj 2007-12-20
		此处应该调用清除函数，而不是显示函数
	CommOltMsgQueueInfoVty(ponId,vty);
	*/
	for( OnuId = 0; OnuId <= CLI_EPON_ONUMAX; OnuId ++)
		CommOltMsgQueueClear(ponId, OnuId );
	
	return CMD_SUCCESS;
}



DEFUN(
		oam_Task_Queue_show,
        oam_Task_Queue_show_cmd,
        "show oam-msg-queue",
		DescStringCommonShow
		"show oam task queue info\n"
        )
{
	CommOamTastQueueInfoVty(vty);
	return CMD_SUCCESS;
}

DEFUN(
		oam_Task_Queue_clear,
        oam_Task_Queue_clear_cmd,
        "clear oam-msg-queue [send|rev] <QueueId>",
		"clear config"
		"clear oam task queue\n"
		"clear oam task queue of send or rev\n"
		"Please input QueueId\n"
        )
{
	unsigned char msgTyep = 0;
	unsigned int queueId = 0;
	if ( !VOS_StrCmp( argv[ 0 ], "send" ) )
    {
        msgTyep = 0;
    }
	else if ( !VOS_StrCmp( argv[ 0 ], "rev" ) )
    {
        msgTyep = 1;
    }
	 queueId = ( ULONG ) VOS_AtoL( argv[ 1 ] );
	if(VOS_OK != CommOamTaskQueueClear(msgTyep,queueId))
	{
		vty_out( vty, " QueueId err! \r\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(
		oam_stat_clear,
        oam_stat_clear_cmd,
        "clear oam-statistics",
		"clear config\n"
		"clear oam statistics\n"
        )
{
	statOamclear();
	return CMD_SUCCESS;
}


DEFUN( oam_debug_off_config,
        oam_debug_off_config_cmd,
        "undo oam debug [cli|snmp|snmptrap|euqinfo|filetran|igmpauth|alarmlog|vconsole|all]",
		NO_STR
		"config oam module debug off\n"
        "config oam module debug off\n"
        "config oam cli debug off\n"
        "config oam snmp debug off\n"
        "config oam snmptrap debug off\n"
        "config oam euq debug off\n"
        "config oam filetran debug off\n"
        "config oam igmpauth debug off\n"
        "config oam alarmlog debug off\n"
        "config oam vconsole(pty) debug off\n"
        "config oam all debug off\n"
        /*"config oam sending msg debug off\n"*/
        )
{
	unsigned char GwProId = 0;
	if (!VOS_StrCmp((CHAR *)argv[0], "cli"))
		 GwProId = GW_DEBUG_CLI;
	else if (!VOS_StrCmp((CHAR *)argv[0], "snmp"))
		GwProId = GW_DEBUG_SNMP;
	else if (!VOS_StrCmp((CHAR *)argv[0], "snmptrap"))
		GwProId = GW_DEBUG_SNMP_TRAP;
	else if (!VOS_StrCmp((CHAR *)argv[0], "euqinfo"))
		GwProId = GW_DEBUG_EUQINFO;
	else if (!VOS_StrCmp((CHAR *)argv[0], "filetran"))
		GwProId = GW_DEBUG_FILE;
	else if (!VOS_StrCmp((CHAR *)argv[0], "igmpauth"))
		GwProId = GW_DEBUG_IGMPAUTH;
	else if (!VOS_StrCmp((CHAR *)argv[0], "alarmlog"))
		GwProId = GW_DEBUG_ALARMORLOG;
	else if (!VOS_StrCmp((CHAR *)argv[0], "vconsole"))
		GwProId = GW_DEBUG_VCONSOLE;
	else if (!VOS_StrCmp((CHAR *)argv[0], "all"))
		GwProId = GW_DEBUG_ALL;
	else if (!VOS_StrCmp((CHAR *)argv[0], "sending"))
		{
			CommOltMsgSendingContentDebug( 0 );
			return CMD_SUCCESS;
		}
	else
		{
		/*vty_out( vty,"  %% Parameter is error\r\n");*/
		return CMD_WARNING;
		}
	if(GwProId == GW_DEBUG_ALL)
		CommOltMsgSendingContentDebug( 0 );
	CommOltMsgRvcDebugTurnOffvty(GwProId,vty);
    return CMD_SUCCESS;
}




DEFUN(
		oam_debug_show,
        oam_debug_show_cmd,
        "show oam debug-status",
		DescStringCommonShow
		"show oam module debug status\n"
        "show oam module debug status\n"
        )
{
	 
	CommOltMsgRvcDebugTurnVtyShow(vty);
	return CMD_SUCCESS;
}




DEFUN( oam_comm_debug_off_config,
        oam_comm_debug_off_config_cmd,
        "undo oam-comm debug",
		NO_STR
		"config oam comm debug off\n"
		"config oam comm debug off\n"
        )
{
	 SetOamCommFlag( 2 );
    return CMD_SUCCESS;
}

DEFUN( oam_comm_debug_on_config,
        oam_comm_debug_on_config_cmd,
        "oam-comm debug",
		"config oam comm debug on\n"
		"config oam comm debug on\n"
        )
{
	 SetOamCommFlag( 1 );
    return CMD_SUCCESS;
}
/*________________________________________________________________*/


DEFUN(
	 event_flag_show,
        event_flag_show_cmd,
        "show event trace-flag",
		"show all event trace flag \n"
		"show all event trace flag \n"
        "show all event trace flag \n"
        )
{
	 
	ShowAllDebugFlag(vty);
	return CMD_SUCCESS;
}

DEFUN( phy_debug_on_config,
        phy_debug_on_config_cmd,
        "pon phy-level trace <slot/port>",
        "config pon phy-level trace enable\n"
        "config pon phy-level trace enable\n"
        "config pon phy-level trace enable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 1;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;

	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);

	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	if(0 != SetPonPhyDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }

	return CMD_SUCCESS;
	
		
}


DEFUN( phy_debug_off_config,
        phy_debug_off_config_cmd,
        "undo pon phy-level trace <slot/port>",
        NO_STR
        "config pon phy-level trace disable\n"
        "config pon phy-level trace disable\n"
        "config pon phy-level trace disable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 2;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	if (0 != SetPonPhyDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }		
	return CMD_SUCCESS;
		
}


DEFUN( msg_debug_on_config,
        msg_debug_on_config_cmd,
        "pon phy-level msg trace <slot/port>",
        "config pon phy-level msg trace enable\n"
        "config pon phy-level msg trace enable\n"
        "config pon phy-level msg trace enable\n"
        "config pon phy-level msg trace enable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 1;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;

	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);

	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	if(0 != SetPonMsgDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }

	return CMD_SUCCESS;
	
		
}


DEFUN( msg_debug_off_config,
        msg_debug_off_config_cmd,
        "undo pon phy-level msg trace <slot/port>",
        NO_STR
        "config pon phy-level msg trace disable\n"
        "config pon phy-level msg trace disable\n"
        "config pon phy-level msg trace disable\n"
        "config pon phy-level msg trace disable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 2;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	if (0 != SetPonMsgDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }		
	return CMD_SUCCESS;
		
}

DEFUN( oam_msg_debug_on_config,
        oam_msg_debug_on_config_cmd,
        "pon oam-level msg trace <slot/port>",
        "config pon oam msg trace enable\n"
        "config pon oam msg trace enable\n"
        "config pon oam msg trace enable\n"
        "config pon oam msg trace enable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 1;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;

	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	if(0 != SetPonOamMsgDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }

	return CMD_SUCCESS;
	
		
}


DEFUN( oam_msg_debug_off_config,
        oam_msg_debug_off_config_cmd,
        "undo pon oam-level msg trace <slot/port>",
        NO_STR
        "config pon oam msg trace disable\n"
        "config pon oam msg trace disable\n"
        "config pon oam msg trace disable\n"
        "config pon oam msg trace disable\n"
        "Please input slot/port"
        )
{
	int Debug_flag = 2;
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
			
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	if (0 != SetPonOamMsgDebug(ponId, Debug_flag))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }		
	return CMD_SUCCESS;
		
}

DEFUN( pon_msg_count_show,
        pon_msg_count_show_cmd,
        "show pon comm-count <slot/port>",
        DescStringCommonShow
        "Show pon`s Rev and Send msg count\n"
        "Show pon`s Rev and Send msg count\n"
        "Please input slot/port"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
		return(ShowMsgCountByVty( ponId ,vty));
		
}


DEFUN( pon_msg_count_clear,
        pon_msg_count_clear_cmd,
        "clear pon comm-count <slot/port>",
        "clear info\n"
        "clear pon`s Rev and Send msg count\n"
        "clear pon`s Rev and Send msg count\n"
        "Please input slot/port"
        )
{

	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	ClearMsgCount(ponId);
	return CMD_SUCCESS;
}

DEFUN( event_debug_delete_authentry,
        event_debug_delete_authentry_cmd,
        "auto-delete onu-authentry {[enable|disable]}",
        "auto-delete"
		"onu-authentry"
		"enable or disable"
        )
{
	if(argc == 1)
	{
		if (!VOS_StrCmp((CHAR *)argv[0], "enable"))
		{
			Timeout_delete_authentry =1;
		}
		else if (!VOS_StrCmp((CHAR *)argv[0], "disable"))
		{
			Timeout_delete_authentry =0;
		}
	}
	else
	{
		if(Timeout_delete_authentry)
		{
			sys_console_printf("The auto-delete onu-authentry is enable!\r\n");
		}
		else
		{
			sys_console_printf("The auto-delete onu-authentry is disable!\r\n");
		}
	}
	/* SetGeneralEvent( debugFlag ); */
    return CMD_SUCCESS;
}

DEFUN( event_debug_associate_share_conf,
        event_debug_associate_share_conf_cmd,
        "auto-associate onu-profile {[enable|disable]}",
        "auto-associate"
		"onu-profile"
		"enable or disable"
        )
{
	if(argc == 1)
	{
		if (!VOS_StrCmp((CHAR *)argv[0], "enable"))
		{
			conf_associate_share =1;
		}
		else if (!VOS_StrCmp((CHAR *)argv[0], "disable"))
		{
			conf_associate_share =0;
		}
	}
	else
	{
		if(conf_associate_share )
		{
			sys_console_printf("The auto-associate onu-profile is enable!\r\n");
		}
		else
		{
			sys_console_printf("The auto-associate onu-profile is disable!\r\n");
		}
	}
	/* SetGeneralEvent( debugFlag ); */
    return CMD_SUCCESS;
}

DEFUN( event_debug_off_config,
        event_debug_off_config_cmd,
        "undo event trace [general|encrypt|register|alarm|reset|activate-pon|switch-pon|update-onu-file|rpc|remote]",
		NO_STR
		"config event trace flag\n"
		"config event trace flag\n"
        "config general event trace off\n"
        "config encrypt event trace off\n"
        "config register event trace off\n"
        "config alarm event trace off\n"
        "config reset event trace off\n"
        "config activate ponchip event trace off\n"
        "config switch pon event trace off\n"
        "config update onu file trace off\n"
        "config rpc trace off\n"
        "config remote trace off\n"
        )
{
	int debugMode = V2R1_DISABLE;
	int debugFlags = 0;

	if (!VOS_StrCmp((CHAR *)argv[0], "general"))
        debugFlags = PON_DEBUGFLAG_DEBUG;
		/* SetGeneralEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "encrypt"))
        debugFlags = PON_DEBUGFLAG_ENCRYPT;
		/* SetEncryptEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "register"))
        debugFlags = PON_DEBUGFLAG_REGISTER;
		/* SetRegisterEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "alarm"))
        debugFlags = PON_DEBUGFLAG_ALARM;
		/* SetAlarmEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "reset"))
        debugFlags = PON_DEBUGFLAG_RESET;
		/*  SetResetEvent(debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "activate-pon"))
        debugFlags = PON_DEBUGFLAG_ADD;
		/* SetAddPonEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "switch-pon"))
        debugFlags = PON_DEBUGFLAG_SWITCH;
		/* SetSwitchPonEvent( debugFlag ); */
	else if(!VOS_StrCmp((CHAR *)argv[0],"update-onu-file"))
        debugFlags = PON_DEBUGFLAG_FILE;
		/* EVENT_UPDATE_ONU_FILE = V2R1_DISABLE; */
	else if (!VOS_StrCmp((CHAR *)argv[0], "rpc"))
        debugFlags = PON_DEBUGFLAG_RPC;
		/* SetRpcEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "remote"))
        debugFlags = PON_DEBUGFLAG_REMOTE;
		/* SetRemoteEvent( debugFlag ); */
	else
		{
		/*vty_out( vty,"  %% Executing error\r\n");*/
		return CMD_WARNING;
		}
    
    OLT_SetDebugMode(OLT_ID_ALL, debugFlags, debugMode);

    return CMD_SUCCESS;
}



DEFUN( event_debug_on_config,
        event_debug_on_config_cmd,
        "event trace [general|encrypt|register|alarm|reset|activate-pon|switch-pon|update-onu-file|rpc|remote]",
		"config event trace flag\n"
		"config event trace flag\n"
        "config general event trace on\n"
        "config encrypt event trace on\n"
        "config register event trace on\n"
        "config alarm event trace on\n"
        "config reset event trace on\n"
        "config activate ponchip event trace on\n"
        "config switch pon event trace on\n"
        "config update onu file trace on\n"
        "config rpc trace on\n"
        "config remote trace on\n"
        )
{
	int debugMode = V2R1_ENABLE;
	int debugFlags = 0;

	if (!VOS_StrCmp((CHAR *)argv[0], "general"))
        debugFlags = PON_DEBUGFLAG_DEBUG;
		/* SetGeneralEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "encrypt"))
        debugFlags = PON_DEBUGFLAG_ENCRYPT;
		/* SetEncryptEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "register"))
        debugFlags = PON_DEBUGFLAG_REGISTER;
		/* SetRegisterEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "alarm"))
        debugFlags = PON_DEBUGFLAG_ALARM;
		/* SetAlarmEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "reset"))
        debugFlags = PON_DEBUGFLAG_RESET;
		/*  SetResetEvent(debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "activate-pon"))
        debugFlags = PON_DEBUGFLAG_ADD;
		/* SetAddPonEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "switch-pon"))
        debugFlags = PON_DEBUGFLAG_SWITCH;
		/* SetSwitchPonEvent( debugFlag ); */
	else if(!VOS_StrCmp((CHAR *)argv[0],"update-onu-file"))
        debugFlags = PON_DEBUGFLAG_FILE;
		/* EVENT_UPDATE_ONU_FILE = V2R1_DISABLE; */
	else if (!VOS_StrCmp((CHAR *)argv[0], "rpc"))
        debugFlags = PON_DEBUGFLAG_RPC;
		/* SetRpcEvent( debugFlag ); */
	else if (!VOS_StrCmp((CHAR *)argv[0], "remote"))
        debugFlags = PON_DEBUGFLAG_REMOTE;
		/* SetRemoteEvent( debugFlag ); */
	else
		{
		/*vty_out( vty,"  %% Executing error\r\n");*/
		return CMD_WARNING;
		}
    
    OLT_SetDebugMode(OLT_ID_ALL, debugFlags, debugMode);

    return CMD_SUCCESS;
}


#if 0

DEFUN( add_pon_config,
        add_pon_config_cmd,
        "add pon-port <slot/port>",
        "add pon port\n"
        "add pon port\n"
        "Please input slot/port"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(slotId) != ROK )
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
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	if( PonPortIsWorking(ponId) == TRUE)
		{
		vty_out( vty,"  %% the pon port is working already.\r\n");
		return CMD_SUCCESS;
		}
	
	if ( ROK != Add_PonPort( ponId ))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }
	return CMD_SUCCESS;
}


DEFUN( del_pon_config,
        del_pon_config_cmd,
        "del pon-port <slot/port>",
        "del pon port\n"
        "del pon port\n"
        "Please input slot/port"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(slotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(slotId) != ROK )
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

	ponId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	if( PonPortIsWorking(ponId) != TRUE)
		{
		vty_out( vty,"  %% the pon port is not working.\r\n");
		return CMD_SUCCESS;
		}
	
	if ( ROK != Del_PonPort( ponId ))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }
	return CMD_SUCCESS;

		
}
#endif

#if 0
DEFUN( del_onu_pending,
        del_onu_pending_cmd,
        "del pending-onu <slot/port/onu>",
        "del pending-onu port\n"
        "del pending-onu port\n"
        "Please input slot/port/onu"
        )
{
	short int ponId = 0;
	unsigned long slotId = 0;
	unsigned long port = 0;
	unsigned long onuId = 0;
	sscanf( argv[0], "%d/%d/%d", &slotId, &port, &onuId);
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	onuId = onuId - 1;
	if ( ROK  != DelOneNodeFromGetEUQ( ponId , onuId))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	return 0;

		
}


DEFUN( add_onu_pending,
        add_onu_pending_cmd,
        "add pending-onu <slot/port/onu>",
        "add pending-onu port\n"
        "add pending-onu port\n"
        "Please input slot/port/onu"
        )
{
	short int ponId = 0;
	unsigned long slotId = 0;
	unsigned long port = 0;
	unsigned long onuId = 0;
	sscanf( argv[0], "%d/%d/%d", &slotId, &port, &onuId);
	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }
	
	onuId = onuId - 1;
	if ( 0 != AddOneNodeToGetEUQ( ponId , onuId))
    {
	    vty_out( vty, "  %% Executing error.\r\n" );
	    return CMD_WARNING;    	
    }
	return 0;

		
}
#endif

#ifdef  __PENDING_ONU__
#endif
extern void PendingOnuListHeadByVty( struct vty *vty);
DEFUN( pending_onu_list_show,
        pending_onu_list_show_cmd,
        "show pending-onu {<slot/port>}*1",
        DescStringCommonShow
        "show pending-pon list\n"
       /* "show pending-onu(no Authentication) list\n"*/
        "Please input slot/port\n"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int PonPortIdx = 0;
	short int count = 0;

	if(argc == 1 )
	{
		sscanf( argv[0], "%d/%d", &slotId, &port);
		
		if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
			return(CMD_WARNING);
		
		if(SlotCardIsPonBoard(slotId) != ROK)
			{
			vty_out(vty," %% slot%d is not pon card\r\n", slotId);
			return(CMD_WARNING);
			}
		
		if(getPonChipInserted(slotId, port) != PONCHIP_EXIST)
			{
			vty_out(vty," %% pon%d/%d is not inserted\r\n", slotId, port);
			return(CMD_WARNING);
			}
		
		PonPortIdx = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );

		CHECK_PON_RANGE;

		/*if(PonPortIsWorking(PonPortIdx) != TRUE)
			{
			vty_out(vty,"pon%d/%d is not working\r\n", slotId, port);
			return(RERROR);
			}*/
			
		PendingOnuListHeadByVty(vty);
		PendingOnuListByVty( PonPortIdx, &count, vty);
	}
	else
	{
		PendingOnuListHeadByVty(vty);
		for(PonPortIdx=0;PonPortIdx < MAXPON; PonPortIdx++)
		{
			if( PonPortIsWorking(PonPortIdx) )
			/*if((PonPortTable[PonPortIdx].PendingOnu.Next != NULL)||(PonPortTable[PonPortIdx].PendingOnu_Conf.Next != NULL))*//*问题单11280*/
			{
				PendingOnuListByVty( PonPortIdx, &count, vty);
			}
		}
	}
	if( 0 == count )
	{
		vty_out(vty, "  No pending onu\r\n"); 
	}
	
	return( CMD_SUCCESS );
		
}

DEFUN( active_pending_onu,
        active_pending_onu_cmd,
        "active pending-onu <slot/port> <H.H.H>",
        "active pending onu\n"
        "active pending onu\n"
        "Please input slot/port\n"
        "please input mac address\n"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int PonPortIdx = 0;
	unsigned char MacAddress[BYTES_IN_MAC_ADDRESS]={0};
	
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
			return(CMD_WARNING);
	
	if(SlotCardIsPonBoard(slotId) != ROK)
		{
		vty_out(vty," %% slot%d is not pon card\r\n", slotId);
		return(CMD_WARNING);
		}
	
	if(getPonChipInserted(slotId, port) != PONCHIP_EXIST)
		{
		vty_out(vty," %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx) != TRUE)
		{
		vty_out(vty,"pon%d/%d is not working\r\n", slotId, port);
		return(RERROR);
		}
	
	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddress ) != VOS_OK )
		{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
		}

	OnuAuth_ActivatePendingOnuByMacAddrMsg(PonPortIdx, MacAddress);
	ActivatePendingOnuMsg_conf(PonPortIdx, MacAddress);

	return(CMD_SUCCESS);	

}

DEFUN( active_Gpon_pending_onu,
        active_Gpon_pending_onu_cmd,
        "active gpending-onu <slot/port> <SN>",
        "active gpending onu\n"
        "active gpending onu\n"
        "Please input slot/port\n"
        "please input SN\n"
        )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int PonPortIdx = 0;
	unsigned char SN[GPON_ONU_SERIAL_NUM_STR_LEN]={0};
	
	sscanf( argv[0], "%d/%d", &slotId, &port);

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
			return(CMD_WARNING);
	
	if(SlotCardIsPonBoard(slotId) != ROK)
	{
		vty_out(vty," %% slot%d is not pon card\r\n", slotId);
		return(CMD_WARNING);
	}
	
	if(getPonChipInserted(slotId, port) != PONCHIP_EXIST)
	{
		vty_out(vty," %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
	}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		vty_out(vty,"pon%d/%d is not working\r\n", slotId, port);
		return(RERROR);
	}
	if( VOS_StrLen(argv[1]) != 16 ) 
	{
		vty_out(vty,"  %%  serial number %s is invalid\r\n", (unsigned char *)argv[1] );
		return(CMD_WARNING );
	}
	VOS_MemCpy( SN, argv[1], GPON_ONU_SERIAL_NUM_STR_LEN );
	OnuAuth_ActivatePendingOnuBySnMsg(PonPortIdx, SN);
	ActivateGponPendingOnuMsg_conf(PonPortIdx, SN);

	return(CMD_SUCCESS);	

}
DEFUN( active_Gponpon_pending_onu,
        active_Gponpon_pending_onu_cmd,
        "active gpending-onu <SN>",
        "active gpending onu\n"
        "active gpending onu\n"
        "Please input slot/port\n"
        "please input SN\n"
        )
{
	unsigned long  ul_slot = 0, ul_port = 0, ulOnuid ;
	short int PonPortIdx = 0;
	unsigned char SN[GPON_ONU_SERIAL_NUM_STR_LEN]={0};
	
	if( parse_pon_command_parameter( vty, &ul_slot, &ul_port , &ulOnuid, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ul_slot, (short  int)ul_port );
	CHECK_PON_RANGE

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		vty_out(vty,"pon%d/%d is not working\r\n", ul_slot, ul_port);
		return(RERROR);
	}
	if( VOS_StrLen(argv[0]) != 16 ) 
	{
		vty_out(vty,"  %%  serial number %s is invalid\r\n", (unsigned char *)argv[0] );
		return(CMD_WARNING );
	}
	VOS_MemCpy( SN, argv[0], GPON_ONU_SERIAL_NUM_STR_LEN );
	OnuAuth_ActivatePendingOnuBySnMsg(PonPortIdx, SN);
	ActivateGponPendingOnuMsg_conf(PonPortIdx, SN);

	return(CMD_SUCCESS);	

}


/*  2008-8-23 chenfj 
     实现在指定PON口下ONU注册绑定
    */
DEFUN( make_test_on_config,
        make_test_on_config_cmd,
        "onu-pon binding [enable|disable]",
        "onu register binding\n"
        "onu register binding\n"
        "enable\n"
        "disable\n"
        )
{
	unsigned long debugFlag = V2R1_ENABLE;

	if(!VOS_StrCmp((CHAR *)argv[0], "enable"))
		debugFlag = V2R1_DISABLE;
	else /*if(!VOS_StrCmp((CHAR *)argv[0], "disable"))*/
		debugFlag = V2R1_ENABLE;

	SetMakeingTestEvent( debugFlag );

	return 0;	
}

/*
DEFUN( make_test_off_config,
        make_test_off_config_cmd,
        "undo making test",
        NO_STR
        "Config making test disable\n"
        "Config making test disable\n"
        )
{
	int debugFlag = 2;
	SetMakeingTestEvent( debugFlag );
	return 0;	
}
*/

DEFUN( make_test_status_show,
        make_test_status_show_cmd,
        "show onu-pon binding",
        DescStringCommonShow
        "show onu register binding\n"
        "show onu register binding\n"
        )
{
	int debugFlag = -1;
	debugFlag = GetMakeingTestFlag(  );
	if (debugFlag == (-1))
    {
	    vty_out( vty, "  %% onu-pon binding error\r\n" );
	    return CMD_WARNING;    	
    }	
	if (debugFlag == V2R1_ENABLE)
		vty_out( vty, "  onu-pon binding disable\r\n");
	else if (debugFlag == V2R1_DISABLE)
		vty_out( vty, "  onu-pon binding enable\r\n");
	else
    {
	    vty_out( vty, "  %% onu-pon binding error.\r\n" );
	    return CMD_WARNING;    	
    }			
	return 0;	
}
/*
5.9.8	制造测试用标志 (此参数需show run 保存)
	  接口函数：int SetMakeingTestEvent(int debugFlag )
      初始值获取函数：int GetMakeingTestFlagDefault()
      当前值获取函数：int GetMakeingTestFlag ()
*/


DEFUN  (
    pon_mpcp_register_show,
    pon_mpcp_register_show_cmd,
    "show onu register info <slot/port>",
    DescStringCommonShow
    "show onu register info\n"
    "show MPCP information of register\n"
    "show MPCP information of register\n"
    "please input slot/port"
    )
{
	ULONG ulSlotId = 0;
	ULONG ulPort = 0;
	short int ponId = 0;
	sscanf( argv[0], "%d/%d", &ulSlotId, &ulPort);

	if(PonCardSlotPortCheckWhenRunningByVty(ulSlotId,ulPort,vty) != ROK)
		return(CMD_WARNING);

	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(ulSlotId) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	/* 2 pon 板类型检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA_EPON )*/
	if(SlotCardIsPonBoard(ulSlotId) != ROK )
		{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlotId);
		return( CMD_WARNING );
		}
	if ( CLI_EPON_CARDINSERT != GetOltCardslotInserted( ulSlotId-1 ))
		{
		vty_out(vty," %% slot %d is not registered\r\n", ulSlotId);
		return CMD_WARNING;
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)ulSlotId,(unsigned char)ulPort) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlotId, ulPort);
		return(CMD_WARNING);
		}

	ponId = GetPonPortIdxBySlot( (short int)ulSlotId, (short  int)ulPort );
    if (ponId == VOS_ERROR)
    {
        return VOS_ERROR;
    }

	ShowOnuRegisterCounterInfoByVty(ponId ,vty);
    return CMD_SUCCESS;
}

/* deleted by chenfj 2008-7-21 */
#if 0
DEFUN  (
    pon_oam_rev_test,
    pon_oam_rev_test_cmd,
    "oam pkt-rev-test <count>",
    "oam rev testing for oam-task\n"
    "oam rev testing for oam-task\n"
    "please input testing packet count\n"
    )
{
	ULONG count = 0;
	count = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	EuqoamSend(count);
    return CMD_SUCCESS;
	
}


DEFUN  (
    pon_oam_send_test,
    pon_oam_send_test_cmd,
    "oam pkt-send-test <count>",
    "oam send testing for oam-task\n"
    "oam send testing for oam-task\n"
    "please input testing packet count\n"
    )
{

	ULONG count = 0;
	count = ( ULONG ) VOS_AtoL( argv[ 0 ] );
	/*EuqoamSend(count);*/
    return CMD_SUCCESS;
}
#endif





#ifdef __OLT_PON_AND_ONU_STAT_
/*  modified by chenfj  2008-8-11
    这两个命令并没有挂到节点下,所以此处先将函数注释掉
    */
DEFUN  (
    debug_olt_stat,
    debug_olt_stat_cmd,
    "stat pon <slotid/port/onuId>",    
    "sample olt`s pon statistics \n"
    "please input sloltid/port/onuId\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;

	sscanf( argv[0], "%d/%d/%d", &slotId, &port, &onuId );

	if(PonCardSlotPortCheckWhenRunningByVty(slotId,port,vty) != ROK)
		return(CMD_WARNING);

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
		return( CMD_WARNING );
		}
	if ( CLI_EPON_CARDINSERT != GetOltCardslotInserted( slotId-1 ))
		{
		vty_out(vty," %% slot %d is not registered\r\n", slotId);
		return CMD_WARNING;
		}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)slotId,(unsigned char)port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}

	if ( (onuId > MAXONUPERPON ) || (onuId < 1) )
    {
       vty_out( vty, "  %% Error onuId %d.\r\n", onuId );
		return CMD_WARNING;
    }
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == VOS_ERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",slotId,port);
		return CMD_WARNING;
	}

	userOnuId = onuId - 1;
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
       {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
   	   vty_out( vty, "  %%  onu%d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
       }
	else if (lRet == (-1))
	{
		vty_out( vty, "  %% Parameter error: %d/%d/%d \r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}

	
	statDebugOltRealTimeStatisticsVty(slotId, port, phyPonId,(onuId-1),vty);
	
	return CMD_SUCCESS;
}


DEFUN  (
    debug_onu_stat,
    debug_onu_stat_cmd,
    "stat onu-pon <slotid/port/onuId>",    
    "sample onu`s pon statistics \n"
    "please input sloltid/port/onuId\n"
    )
{
    LONG lRet = VOS_OK;
    ULONG slotId = 0;
    ULONG onuId = 0; 
    ULONG port = 0;
    INT16 phyPonId = 0;
    UINT16 userOnuId = 0;
	
	/*if(0 == memcmp("/?",argv[0],2))
	{
		vty_out(vty, "  %%Please input the slot/port/onuIdx\r\n");
		return CMD_WARNING;
	}*/
	
	sscanf( argv[0], "%d/%d/%d", &slotId, &port, &onuId );

	if(PonCardSlotPortCheckByVty(slotId, port,vty)!=ROK)
		return CMD_WARNING;

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
		return( CMD_WARNING );
	}
	/* pon chip is inserted  */
	if(getPonChipInserted((unsigned char)slotId,(unsigned char)port) != PONCHIP_EXIST)
		{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", slotId, port);
		return(CMD_WARNING);
		}
	
	if ( (onuId > MAXONUPERPON) || (onuId < 1) )
    {
       vty_out( vty, "  %% Error onuId %d/%d/%d.\r\n", slotId,port,onuId );
		return CMD_WARNING;
    }
    phyPonId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if (phyPonId == VOS_ERROR)
    {
	#ifdef CLI_EPON_DEBUG
	    vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
	#endif   
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == VOS_ERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",slotId,port);
		return CMD_WARNING;
	}

	userOnuId = onuId - 1;
     /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
    lRet = 	GetOnuOperStatus( phyPonId, userOnuId);
    if ( CLI_EPON_ONUDOWN == lRet)
    {
       #ifdef CLI_EPON_DEBUG
          vty_out( vty, "  %% DEBUG: phyPonId %d, userOnuId %d.\r\n",phyPonId, userOnuId) ;
       #endif
   	   vty_out( vty, "  %% %d/%d/%d is off-line.\r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
    }
	else if (lRet == (-1))
	{
		vty_out( vty, "  %% Parameter error: %d/%d/%d \r\n",slotId,  port, onuId) ;
		return CMD_WARNING;
	}

	
	statDebugOnuRealTimeStatisticsVty(slotId, port, phyPonId,onuId-1,vty);
	
	return CMD_SUCCESS;
}
#endif

DEFUN  (
    oam_testing_show,
    oam_testing_show_cmd,
    "show oam-testing-info",
    DescStringCommonShow
    "show oam testing result\n"
    )
{
	unsigned int sendCount = 0;
	unsigned int revCount = 0;
	CommOamTaskTestingInfoGet( &sendCount, &revCount);
	vty_out( vty, "\r\n  Testing send packet num(Revice by oam-task): %d\r\n",sendCount);
	vty_out( vty, "\r\n  Testing Rev packet num(Revice by oam-task): %d\r\n",revCount);	
	return CMD_SUCCESS;
}


DEFUN  (
    oam_callback_show,
    oam_callback_show_cmd,
    "show oam-callback-info",
    DescStringCommonShow
    "show oam testing result\n"
    )
{
	CommOamCallbackInofVty( vty );
	return CMD_SUCCESS;
}

/*----------------------------------------------------------------------------------------------------------*
 启动/ 禁止 ONU 文件传输debug 开关
*-----------------------------------------------------------------------------------------------------------*/
extern long             glTransFileDebug;

DEFUN( debug_oam_transfile_func,
           debug_oam_transfile_cmd,
           "debug oam_transfile",
           DEBUG_STR
           "oam_transfile information\n" )
{
      glTransFileDebug = 1;
      return VOS_OK;
}

DEFUN( no_debug_oam_transfile_func,
           no_debug_oam_transfile_cmd,
           "undo debug oam_transfile",
           NO_STR
           DEBUG_STR
           "oam_transfile information\n" )
{
      glTransFileDebug = 0;
      return VOS_OK;
}


DEFUN( onu_llid_mapping,
           onu_llid_mapping_cmd,
           "show onu-llid mapping <slot/port> <onuid_list>",
           DescStringCommonShow
           "Show onu and llid information\n"
           "Show onu to llid mapping\n"
           "Please input slot/port\n"
           OnuIDStringDesc
           )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	unsigned long ulOnuId = 0;
	short int ponId = 0;
	int countFirst = 0;
	
	sscanf( argv[0], "%d/%d", &slotId, &port);
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
	{
		countFirst++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	if ( 1 == countFirst )
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
		{
			ShowOnuLlidMappingByVty( ponId, (ulOnuId-1), vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
	}
	else
	{
		vty_out( vty, "\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], ulOnuId )
		{
			ShowOnuLlidMappingByVty_1( ponId, (ulOnuId-1), vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
		vty_out( vty, "\r\n");
	}
	

      return VOS_OK;
}


DEFUN( onu_llid_mapping_pon,
           onu_llid_mapping_pon_cmd,
           "show onu-llid mapping <onuid_list>",
           DescStringCommonShow
           "Show onu and llid information\n"
           "Show onu to llid mapping\n"
           OnuIDStringDesc
           )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	unsigned long ulOnuId = 0;
	short int ponId = 0;
	int countFirst = 0;
	ULONG ulIfIndex;
	LONG lRet;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &slotId, &port, &ulOnuId );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
    if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
    {
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;    	
    }

	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
		countFirst++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	if ( 1 == countFirst )
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
		{
			ShowOnuLlidMappingByVty( ponId, (ulOnuId-1), vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
		vty_out( vty, "\r\n");
	}
	else
	{
		vty_out( vty, "\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
		{
			ShowOnuLlidMappingByVty_1( ponId, (ulOnuId-1), vty );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();	
		vty_out( vty, "\r\n");
	}
	

      return VOS_OK;
}
    
/* added by chenfj 2007-7-3 */
#ifdef  PON_PORT_CNI_PARAMETER
#endif
DEFUN( set_pon_cni,
		set_pon_cni_cmd,
		"pon cni-para <slot/port> mdio-phy <0-31> auto-neg <0-1> pause <0-1> <1-100> <1-100> advertise <0-1> <0-1> <0-1> <0-1> <0-1> <0-1>",
		"set pon cni parameter\n"
		"set pon cni parameter\n"
		"please input the slot/port\n"
		"the phy mdio address\n"
		"please input the phy mdio address\n"
		"auto-negotiatiion\n"
		"input:1-enable,0-disable\n"
		"pause\n"
		"input:1-enable,0-disable\n"
		"input the set threshold,for pas5001,unit:KB.note:this value should be >= 6 than release threshold. for pas5201,unit:%\n"
		"input pause release threshold,for pas5001,unit:KB.note:this value should be <= 6 than the set threshold.for pas5201,unit:%\n"
		"advertise option\n"
		"advertise:1-enable,0-disable\n"
		"advertise tx-half:1-enable,0-disable\n"
		"advertise tx-full:1-enable,0-disable\n"
		"advertise port type:1-master,0-slave\n"
		"advertiase pause:1-enable,0-disable\n"
		"advertise asymmetric pause:1-enable,0-disable\n"
           )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	int PonChipType ;
	short int ret;
	long threshold;

	PON_olt_cni_port_mac_configuration_t  cni_mac_config;
	
	sscanf( argv[0], "%d/%d", &slotId, &port);
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
	 {
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;    	
	 }

	PonChipType = V2R1_GetPonchipType( ponId );
	if(( PonChipType != PONCHIP_PAS5001 ) && (PonChipType != PONCHIP_PAS5201 ))
	{
		vty_out( vty, " pon chip type err\r\n");
		return(CMD_WARNING );
	}
	
	cni_mac_config.master = PON_MASTER;
	cni_mac_config.mdio_phy_address = ( unsigned char ) VOS_AtoL( argv[ 1 ] );
	
	/* auto-negotiation */
	if ( VOS_AtoL(argv[2]) == 1 )
	{
		cni_mac_config.auto_negotiation = TRUE;
	}
	else
	{
		cni_mac_config.auto_negotiation = FALSE;
	}

	/* pause */
	if ( VOS_AtoL(argv[3]) == 1 )
	{
		cni_mac_config.pause= ENABLE;
	}
	else
	{
		cni_mac_config.pause= DISABLE;
	}
	
	/* pause-thrd */

	threshold = ( long ) VOS_AtoL( argv[ 4 ] );
	cni_mac_config.pause_thresholds.pause_set_threshold = threshold;

	if(PonChipType == PONCHIP_PAS5001)
	{
		if ( !(PON_IS_PAUSE_SET_THRESHOLD_VALUE_LEGAL(threshold )))
#if 0
		if(( threshold != PON_PAUSE_SET_THRESHOLD_24K ) && (threshold != PON_PAUSE_SET_THRESHOLD_26K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_32K ) && ( threshold != PON_PAUSE_SET_THRESHOLD_34K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_40K ) && ( threshold != PON_PAUSE_SET_THRESHOLD_42K ) )
#endif
		{
			vty_out(vty," pause set threshold err\r\n");
			return( CMD_WARNING );
		}
	}
	if( PonChipType != PONCHIP_PAS5001 ) 
	{
		if( !(PON_IS_PAUSE_RELEASE_THRESHOLD_PERCENTS_VALUE_LEGAL(threshold ) ) )
#if 0
		if(( threshold != PON_PAUSE_SET_THRESHOLD_24K ) && (threshold != PON_PAUSE_SET_THRESHOLD_26K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_32K ) && ( threshold != PON_PAUSE_SET_THRESHOLD_34K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_40K ) && ( threshold != PON_PAUSE_SET_THRESHOLD_42K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_48K ) && ( threshold != PON_PAUSE_SET_THRESHOLD_56K )
			&& ( threshold != PON_PAUSE_SET_THRESHOLD_60K ) )
#endif
		{
			vty_out(vty," pause set threshold err\r\n");
			return( CMD_WARNING );
		}
	}

	threshold = ( long ) VOS_AtoL( argv[ 5 ] );
	cni_mac_config.pause_thresholds.pause_release_threshold = threshold;
		
	if(PonChipType == PONCHIP_PAS5001)
	{
		if ( !(PON_IS_PAUSE_RELEASE_THRESHOLD_VALUE_LEGAL(threshold )))
#if 0
		if(( threshold != PON_PAUSE_RELEASE_THRESHOLD_16K ) && (threshold != PON_PAUSE_RELEASE_THRESHOLD_20K )
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_24K ) && ( threshold != PON_PAUSE_RELEASE_THRESHOLD_28K )
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_32K ) && ( threshold != PON_PAUSE_RELEASE_THRESHOLD_36K ) )
#endif
		{
			vty_out(vty," pause set threshold err\r\n");
			return( CMD_WARNING );
		}
	}
	if( PonChipType != PONCHIP_PAS5001 ) 
	{
		if( !(PON_IS_PAUSE_RELEASE_THRESHOLD_PERCENTS_VALUE_LEGAL( threshold ) ))
#if 0		
		if(( threshold != PON_PAUSE_RELEASE_THRESHOLD_16K ) && (threshold != PON_PAUSE_RELEASE_THRESHOLD_20K )
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_24K ) && ( threshold != PON_PAUSE_RELEASE_THRESHOLD_28K )
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_32K ) && ( threshold != PON_PAUSE_RELEASE_THRESHOLD_36K ) 
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_40K ) && ( threshold != PON_PAUSE_RELEASE_THRESHOLD_48K )
			&& ( threshold != PON_PAUSE_RELEASE_THRESHOLD_52K ) )
#endif
		{
			vty_out(vty," pause set threshold err\r\n");
			return( CMD_WARNING );
		}
	}

	if( PonChipType == PONCHIP_PAS5001 )
		{
	
	 	if(( threshold + 6 ) > cni_mac_config.pause_thresholds.pause_set_threshold )
	 		{
			vty_out(vty," release threshold shoule be <= 6 than set threshold\r\n");
			return( CMD_WARNING );
	 		}
		}
	
	/* advertise */
	if ( VOS_AtoL(argv[6]) == 1 )
	{
		cni_mac_config.advertise = ENABLE;
	}
	else
	{
		cni_mac_config.advertise = DISABLE;
	}

	/* advertise-tx-half  */
	if ( VOS_AtoL(argv[7]) == 1 )
	{
		cni_mac_config.advertisement_details._1000base_tx_half_duplex = TRUE;
	}
	else
	{
		cni_mac_config.advertisement_details._1000base_tx_half_duplex = FALSE;
	}

	/* advertise-rx_full  */
	if ( VOS_AtoL(argv[8]) == 1 )
	{
		cni_mac_config.advertisement_details._1000base_tx_full_duplex = TRUE;
	}
	else
	{
		cni_mac_config.advertisement_details._1000base_tx_full_duplex = FALSE;
	}

	/*advertise-port-type  */
	if ( VOS_AtoL(argv[9]) == 1 )
	{
		cni_mac_config.advertisement_details.preferable_port_type = PON_MASTER;
	}
	else
	{
		cni_mac_config.advertisement_details.preferable_port_type = PON_SLAVE;
	}

	/*  advertiase-pause */
	if ( VOS_AtoL(argv[10]) == 1 )
	{
		cni_mac_config.advertisement_details.pause = TRUE;
	}
	else
	{
		cni_mac_config.advertisement_details.pause = FALSE;
	}


	 /* advertise-asymmetric-pause */
	if ( VOS_AtoL(argv[11]) == 1 )
	{
		cni_mac_config.advertisement_details.asymmetric_pause = TRUE;
	}
	else
	{
		cni_mac_config.advertisement_details.asymmetric_pause = FALSE;
	}

	if(OLT_PONCHIP_ISPAS(PonChipType)) 
	{
	    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		ret = Pon_SetCniPortMacConfiguration( ponId, PonChipType, &cni_mac_config );
		if( ret == PAS_EXIT_OK )
			return( CMD_SUCCESS);
		else return( CMD_WARNING );
	}

      return( CMD_WARNING );
}


DEFUN( show_pon_cni,
		show_pon_cni_cmd,
		"show pon cni-para <slot/port>",
		"show pon cni parameter\n"
		"show pon cni parameter\n"
		"show pon cni parameter\n"
		"input slot/port\n"		
           )
{
	unsigned long slotId = 0;
	unsigned long port = 0;
	short int ponId = 0;
	int PonChipType ;
	short int ret;

	PON_olt_cni_port_mac_configuration_t  cni_mac_config;
	
	sscanf( argv[0], "%d/%d", &slotId, &port);
	ponId = GetPonPortIdxBySlot( (short int)slotId, (short  int)port );
	if ((ponId < CLI_EPON_PONMIN) || (ponId > CLI_EPON_PONMAX))
	 {
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;    	
	 }

	if( PonPortIsWorking(ponId) != TRUE ) 
	{	
		vty_out( vty, " pon%d/%d is not working\r\n", slotId, port );
		return( CMD_WARNING);
	}
	
	PonChipType = V2R1_GetPonchipType( ponId );
    /*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
	if(OLT_PONCHIP_ISPAS(PonChipType)) 
	{
	   
		ret = Pon_GetCniPortMacConfiguration( ponId, PonChipType, &cni_mac_config );
		if( ret != PAS_EXIT_OK ) 
			return( CMD_WARNING );

		vty_out( vty, "  master = %s\r\n", ((cni_mac_config.master ==  PON_MASTER) ? "master" : "slave") );

		/* pause */
		vty_out(vty,"  pause = %s\r\n", ((cni_mac_config.pause ==  ENABLE) ? "enable" : "disable") );
		vty_out( vty, "  mdio phy address = %d\r\n", cni_mac_config.mdio_phy_address );

		/* auto-negotiation */
		vty_out(vty,"  auto-negotiation = %s\r\n", ((cni_mac_config.auto_negotiation == ENABLE) ? "enable" : "disable") );

		/* pause-thrd */

		if( PonChipType == PONCHIP_PAS5001 )
		{
			vty_out(vty,"  pause_set_threshold = %d\r\n", cni_mac_config.pause_thresholds.pause_set_threshold);	
			vty_out(vty,"  pause_release_threshold = %d\r\n", cni_mac_config.pause_thresholds.pause_release_threshold);
		}
		else
		{
			vty_out(vty,"  pause_set_threshold = %d\r\n", cni_mac_config.pause_thresholds.pause_set_threshold);	
			vty_out(vty,"  pause_release_threshold = %d\r\n", cni_mac_config.pause_thresholds.pause_release_threshold);
		}	
		
		/* advertise */
		vty_out(vty,"  advertise = %s\r\n", ((cni_mac_config.advertise == ENABLE) ? "enable" : "disable") );

		/* advertise-tx-half  */
		vty_out(vty,"  advertise-tx-half = %s\r\n", ((cni_mac_config.advertisement_details._1000base_tx_half_duplex == ENABLE) ? "enable" : "disable") );

		/* advertise-rx_full  */
		vty_out(vty,"  advertise-tx_full = %s\r\n", ((cni_mac_config.advertisement_details._1000base_tx_full_duplex == ENABLE) ? "enable" : "disable") );

		/*advertise-port-type  */
		vty_out(vty,"  advertise-port-type = %s\r\n", ((cni_mac_config.advertisement_details.preferable_port_type == PON_MASTER) ? "master" : "slave") );
		
		/*  advertiase-pause */
		vty_out(vty,"  advertiase-pause = %s\r\n", ((cni_mac_config.advertisement_details.pause == ENABLE) ? "enable" : "disable") );


		 /* advertise-asymmetric-pause */
		vty_out(vty,"  advertise-asymmetric-pause = %s\r\n", ((cni_mac_config.advertisement_details.asymmetric_pause == ENABLE) ? "enable" : "disable") );
	}
	else {
		vty_out( vty, "  pon chip type err\r\n");
		return( CMD_WARNING);
	}

      return( CMD_SUCCESS );
}

	/* modified by chenfj 2007-7-16
		增加一个带宽比例因子,用于测试带宽精度
	*/
				
DEFUN(
	uplink_bandwidth_BataRatio_set,
	uplink_bandwidth_BataRatio_set_cmd, 
	"uplink bandwidth bata ratio <500-1500>",
	"set uplink bandwidth ratio\n"
	"set uplink bandwidth ratio\n"
	"set uplink bandwidth ratio\n"
	"set uplink bandwidth ratio\n"
	"input the ratio, unit:\n"
	)
{
	int ratio = VOS_AtoI( argv[ 0 ] );

    SetOnuBwParams(ratio, -1);

	return CMD_SUCCESS;
}

DEFUN(
	uplink_bandwidth_BataRatio_show,
	uplink_bandwidth_BataRatio_show_cmd, 
	"show uplink bandwidth bata ratio", 
	"show uplink bandwidth ratio\n"
	"show uplink bandwidth ratio\n"
	"show uplink bandwidth ratio\n"
	"show uplink bandwidth ratio\n"
	"show uplink bandwidth ratio\n"
	)
{
    int ratio = 1000;

	GetOnuBwParams(&ratio, NULL);
	vty_out( vty, "  %%uplink bandwidth bata ratio=%d\r\n", ratio);
		 
	return CMD_SUCCESS;
}

DEFUN(
	onu_eeprom_set,
	onu_eeprom_set_cmd, 
	"set onu eeprom threshold mode <slot/port/onuid> <0-10>", 
	"set onu eeprom threshold mode\n"
	"set onu eeprom threshold mode\n"
	"set onu eeprom threshold mode\n"
	"set onu eeprom threshold mode\n"
	"set onu eeprom threshold mode\n"
	"input the slot/port/onuid\n"
	"input the mode,(0=Disable, 1=mode 5, 2=mode 8)\n"
	)
{

	short int ret;
	short int phyPonId, OnuIdx, llid;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	unsigned char thrld;
	unsigned long size=1;
	int OnuVendorType;
	

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{ 
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
		
#if 0    
	OnuVendorType = GetOnuVendorType(phyPonId, (ulOnuid-1));
#endif

	thrld = ( unsigned char ) VOS_AtoL( argv[ 1 ] );

	vty_out(vty, "pon %d onu %d llid %d thrld %d\r\n", phyPonId, ulOnuid,llid, thrld );
	
#if 1
    ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, EEPROM_MAPPER_THRESHOLD_MODE, &thrld, size );
#else
	ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, EEPROM_MAPPER_THRESHOLD_MODE, &thrld, size );
#endif

	if( ret != PAS_EXIT_OK ) 
		{
		vty_out(vty, "  %% set onu %d/%d/%d threshold err\r\n",ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	return(CMD_SUCCESS);
}


DEFUN(
	onu_eeprom1_set,
	onu_eeprom1_set_cmd, 
	"set onu eeprom unify-threshold mode <slot/port/onuid> <0-10>", 
	"set onu eeprom unify-threshold mode\n"
	"set onu eeprom unify-threshold mode\n"
	"set onu eeprom unify-threshold mode\n"
	"set onu eeprom unify-threshold mode\n"
	"set onu eeprom unify-threshold mode\n"
	"input the slot/port/onuid\n"
	"input the mode(0=not unify, 1=unify)\n"
	)
{
	short int ret;
	short int phyPonId, OnuIdx, llid;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	unsigned char thrld;
	unsigned long size=1;


	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if( PonCardSlotPortCheckByVty(ulSlot,ulPort, vty) != ROK)
		return(CMD_WARNING);
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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n",phyPonId);
#endif   
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;    
	if( GetOnuOperStatus( phyPonId, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	thrld = ( unsigned char ) VOS_AtoL( argv[ 1 ] );
	vty_out(vty, "pon %d onu %d llid %d thrld %d\r\n", phyPonId, ulOnuid,llid, thrld );

#if 1
    ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, EEPROM_MAPPER_UNIFY_THRESHOLD_MODE, &thrld, size );
#else
	ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, EEPROM_MAPPER_UNIFY_THRESHOLD_MODE, &thrld, size );
#endif

	if( ret != PAS_EXIT_OK ) 
		{
		vty_out(vty, "  %% set onu %d/%d/%d unify-threshold err\r\n",ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	return(CMD_SUCCESS);
}

DEFUN(
	onu_eeprom_show,
	onu_eeprom_show_cmd, 
	"show onu eeprom threshold mode <slot/port/onuid>", 
	"show onu eeprom threshold mode\n"
	"show onu eeprom threshold mode\n"
	"show onu eeprom threshold mode\n"
	"show onu eeprom threshold mode\n"
	"show onu eeprom threshold mode\n"
	"input the slot/port/onuid\n"
	)
{

	short int ret;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, OnuIdx /* llid */;
	unsigned char thrld, unify_thrld;
	unsigned long size=1;
	

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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
		
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

#if 0
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
#endif
	
#if 1
    ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, EEPROM_MAPPER_THRESHOLD_MODE, &thrld, &size );
#else
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_THRESHOLD_MODE, &thrld, &size );
#endif
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "  %% get onu %d/%d/%d threshold err\r\n",ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
#if 1
    ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, EEPROM_MAPPER_UNIFY_THRESHOLD_MODE, &unify_thrld, &size );
#else
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNIFY_THRESHOLD_MODE, &unify_thrld, &size );
#endif
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "  %% get onu %d/%d/%d unify-threshold err\r\n",ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	vty_out(vty,"  onu %d/%d/%d eeprom mapper para:\r\n", ulSlot, ulPort, ulOnuid );
	vty_out(vty,"  threshold:%d\r\n",thrld);
	vty_out(vty,"  unify threshold:%d\r\n", unify_thrld);
			 
	return CMD_SUCCESS;
}

void ExchangeWord( unsigned short int *x)
{
	unsigned short int y, z;

	y = ((*x) & 0xff ) << 8;
	z = ((*x )& 0xff00) >> 8;

	*x = y + z;
}

void ExchangeInt( unsigned int *x)
{

	unsigned short int y=0, z=0;
	
	y = ((*x) & 0xffff ) ;
	z = ((*x) & 0xffff0000) >> 16;

	ExchangeWord( (unsigned short int *) &y );
	ExchangeWord( (unsigned short int *) &z);

	*x = y << 16;
	*x +=  z;
}

char  Handle5BitData( char *BitData_5)
{
	/*  modified by chenfj 2009-6-8
		ONU(固件版本V1.4.2.4)返回的参数格式改变了，跟之前onu版本V1.3.11.1 不一致；
		如若实际值为-15, 那么在之前版本时，返回值为0x11;  版本V1.4.2.4 则返回值
		为0xf1
		*/		
	if(*BitData_5 >= 0xf0 )
		{
		*BitData_5 = 0x100 - *BitData_5 ;
		return( 1 );
		}
	else if( *BitData_5 >= 0x10 )
		{
		*BitData_5 = 0x20 - *BitData_5 ;
		return( 1 );
		}
	else{
		return( 0 );
		}
}

#if  0
DEFUN(
	onu_emapper_set1,
	onu_emapper_set1_cmd, 
	"set onu emapper1 <slot/port/onuid> [cookie|eramrd|eramwr|eramsz|fimage|simage|bmon|mansec|flashrd|flashwr|flashvndr|sdraminfo|sdramref|sdramtcas|sdramtras|sdramwd|sdramrbc|eramty|ponsgnl|pontbc|lasertx] <value>", 
	"set onu emapper\n"
	"set onu emapper\n"
	"set onu emapper\n"
	"input the slot/port/onuid\n"
	"      -EEPROM presence ID cookie;0xda-eeprom is present, other-eeprom is not present\n"
	"      - External RAM's read wait-states,range:0-15\n"
	"      - External RAM's write wait-states,range:0-15\n"
	"      - External RAM size (in MB for SDRAM and in KB for SRAM,Zero if not present)\n"
	"      - First image start address in KB\n"
	"      - Second image start address in KB\n"
	"        - Boot monitor start address in KB\n"
	"      - Management secs start address in KB\n"
	"     - FLASH read wait-states,number of bus clocks,range:0-15\n"
	"     - FLASH write wait-states,number of bus clocks,range:0-15\n"
	"   - FLASH vendor code,0 for AMD, 2 for CFI\n"
	"   - SDRAM index of parameters,range:0-15\n"
	"    - REFRESH cycle period [num of 16xHCLKs],range:0-1023\n"
	"   - SDRAM tCAS,range:0-3\n"
	"   - SDRAM tRAS,range:0-3\n"
	"     - SDRAM Width Mode,range:0-3\n"
	"    - SDRAM RBC or BRC addressing schem,0-RBC,1-BRC\n"
	"      - External ram to use (0=SDRAM, 1=SRAM)\n"
	"     - PON loss signal polarity(0-base polarity,1-shifted 180 degrees)\n"
	"      - PON TBC signal polarity(0-base polarity,1-shifted 180 degrees)\n"
	"     - Laser TX enable polarity(0-base polarity,1-shifted 180 degrees)\n"

	"the emapper value\n"

	)
{

	short int ret;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, llid;
	int OnuType;
	int result;
	char  One_byte;
	unsigned short int Two_byte;
	int Four_byte;
	char N_bytes[64];
	unsigned long size = 0xff;
	EEPROM_mapper_param_t  emapper_Macro = 0xff;
	char *pToken;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	/*IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );*/

	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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
		
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

	if( GetOnuOperStatus( phyPonId, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

	GetOnuType( phyPonId, (ulOnuid - 1 ), &OnuType );
	if((OnuType < V2R1_ONU_GT811 ) || ( OnuType > V2R1_DEVICE_LAST))
		{
		vty_out(vty, "  %% onu type err\r\n");
		return	( CMD_WARNING );
		}

	if(  argc != 3 )
		return( CMD_WARNING );

	if (!VOS_StrCmp((CHAR *)argv[1], "cookie"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_COOKIE_ID;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "eramrd"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_EXT_RAM_READ_WS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "eramwr"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_EXT_RAM_WRITE_WS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "eramsz"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_EXT_RAM_SIZE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "fimage"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_FIRST_FW_START_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "simage"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_SECOND_FW_START_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "bmon"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_BOOT_MONITOR_START_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "mansec"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_MANAGE_SECS_START_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "flashrd"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_EXT_FLASH_READ_WS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "flashwr"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_EXT_FLASH_WRITE_WS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "flashvndr"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_FLASH_VENDOR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdraminfo"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_SDRAM_INFO_INDEX;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdramref"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_SDRAM_REFRESH_CYCLE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdramtcas"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_SDRAM_TCAS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdramtras"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_SDRAM_TRAS;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdramwd"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_SDRAM_WIDTH_MODE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "sdramrbc"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_SDRAM_RBC_OR_BRC;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "eramty"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "ponsgnl"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_PON_LOSS_SIGNAL_POLARITY;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "pontbc"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_PON_TBC_SIGNAL_POLARITY;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "lasertx"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_LASER_TX_ENABLE_POLARITY;
		}                        
	else {
		vty_out(vty,"  %% parameter err\r\n");
		}
		


	if( size == 1 )
		{
		if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
			{
			One_byte = ( char )VOS_StrToUL( argv[2], &pToken, 16 );
			sys_console_printf(" One_byte 0x%x\r\n", One_byte );
			}
		else {
			One_byte = (char )strtol(argv[2], NULL, 10 );
			sys_console_printf(" One_byte %d\r\n", One_byte );
			}

		ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &One_byte, size );
		if( ret != PAS_EXIT_OK ) 
			{
			vty_out(vty,"  %% Executing err\r\n");	
			return( CMD_WARNING );
			}
		}
	else if (size == 2 )
		{
		if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
			{
			Two_byte = ( short int )VOS_StrToUL( argv[2], &pToken, 16 );
			sys_console_printf(" Two_byte 0x%x\r\n", Two_byte );
			}
		else {
			Two_byte = (short int )strtol(argv[2], NULL, 10 );
			sys_console_printf(" Two_byte %d\r\n", Two_byte );
			}
		/*if( emapper_Macro != EEPROM_MAPPER_SDRAM_REFRESH_CYCLE )*/
		ExchangeWord(&Two_byte );
		ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &Two_byte, size );
		if( ret != PAS_EXIT_OK ) 
			{
			vty_out(vty,"  %% Executing err\r\n");	
			return( CMD_WARNING );
			}
		}

	return( CMD_SUCCESS);	

}

DEFUN(
	onu_emapper_set2,
	onu_emapper_set2_cmd, 
	"set onu emapper2 <slot/port/onuid> [bootmode|baud|dgpol|lfpol|cepol|zbtu|zbtc|ponpwr|oui|mac|ip|netmask|laserton|lasertoff] <value>", 
	"set onu emapper\n"
	"set onu emapper\n"
	"set onu emapper\n"
	"input the slot/port/onuid\n"
	"    - Image configuration,0-single image,1-dual image,3-enhanced,other-reserved\n"
	"        - UART's baud-rate index(0-1200,1-2400,2-4800,3-9600,4-14000,5-19200,6-28800,7-38400,8-57600,9-115200,A-230400,B-460800)\n"
	"       - DYING GASP polarity(0-active low,1-active high)\n"
	"       - LINK FAULT polarity(0-active low,1-active high)\n"
	"       - CRITICAL EVENT polarity(0-active low,1-active high)\n"
	"        - Use ZBT(0-no ZBT memory,1-use ZBTmemory)\n"
	"        - ZBT config (0=Flowtrhu, 1=Pipeline)\n"
	"      - PON Connect on power up(0-dont connect on power up,1-connect on power up)\n"
	"         - OUI address\n"
	"         - MAC address (xxxx.xxxx.xxxx)\n"
	"          - IP Address (x.x.x.x)\n"
	"     - Net Mask (x.x.x.x)\n"
	"    - Laser on time in 16nSec quanta\n"
	"   - Laser off time in 16nSec quanta\n"

	"the emapper value\n"
	)
{

	short int ret;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, llid;
	int OnuType;
	int result;
	char  One_byte;
	unsigned short int Two_byte;
	int Four_byte;
	char N_bytes[64];
	unsigned long size = 0xff;
	EEPROM_mapper_param_t  emapper_Macro = 0xff;
	char *pToken;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	/*IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );*/

	if ((ulSlot<4)  || (ulSlot>8))
		{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
		}
	
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if ( CLI_EPON_CARDINSERT != GetOltCardslotInserted( ulSlot-1 ))
			{
			vty_out( vty, "  %% Can not find slot %d. \r\n",ulSlot);
			return CMD_WARNING;
			}
	
		if (ulSlot == 4)
			{
			if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
				{
				vty_out( vty, "  %% slot%d is %s\r\n", ulSlot, MODULE_E_EPON3_SW_NAME_STR );
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
			vty_out( vty, "  %% Parameter is error.\r\n" );
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

		if( GetOnuOperStatus( phyPonId, (ulOnuid-1)) != ONU_OPER_STATUS_UP )
			{
			vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
			return( CMD_WARNING);
			}
		llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
		if( llid == INVALID_LLID ) 
			{
			vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
			return( CMD_WARNING);
			}
		}
	else return( CMD_WARNING );

	GetOnuType( phyPonId, (ulOnuid - 1 ), &OnuType );
	if((OnuType < V2R1_ONU_GT811 ) || ( OnuType > V2R1_DEVICE_LAST))
		{
		vty_out(vty, "  %% onu type err\r\n");
		return	( CMD_WARNING );
		}

	if(  argc != 3 )
		return( CMD_WARNING );                               
                                                           
	if (!VOS_StrCmp((CHAR *)argv[1], "bootmode"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_BOOT_LOADER_MODE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "baud"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_UART_BAUD_INDEX;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "dgpol"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_DYING_GASP_POLARITY;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "lfpol"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_LINK_FAULT_POLARITY;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "cepol"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_CRITICAL_EVENT_POLARITY;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "zbtu"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_ZBT_USE;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "zbtc"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_ZBT_CONFIG;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "ponpwr"))
		{
		size = 1;
		emapper_Macro = EEPROM_MAPPER_PON_CONNECT_ON_POWER_UP;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "oui"))
		{
		size = 3;
		emapper_Macro = EEPROM_MAPPER_OUI_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "mac"))
		{
		size = 6;
		emapper_Macro = EEPROM_MAPPER_EEPROM_MAC_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "ip"))
		{
		size = 4;
		emapper_Macro = EEPROM_MAPPER_EEPROM_IP_ADDR;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "netmask"))
		{
		size = 4;
		emapper_Macro = EEPROM_MAPPER_EEPROM_NET_MASK;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "laserton"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_LASER_TON;
		}
	else if (!VOS_StrCmp((CHAR *)argv[1], "lasertoff"))
		{
		size = 2;
		emapper_Macro = EEPROM_MAPPER_LASER_TOFF;
		}
	else {
		vty_out(vty,"  %% parameter err\r\n");
		}

/*
adven       - UNI phy advertising enable                                        
brg         - bridge enable                                                     
autoneg     - auto-neg enable                                                   
phyaddr     - PHY address                                                       
iftype      - MAC type (0-GMII or MII, 1-TBI)                                   
master      - UNI Master mode (0=slave, 1=master)                               
advmp       - advertise 1000t multi-port                                        
advgh       - advertise 1000t half duplex                                       
advgf       - advertise 1000t full duplex                                       
advpa       - advertise pause asymmetric                                        
advpe       - advertise pause enabled                                           
advt4       - advertise 1000t 100t4                                             
advhf       - advertise 100tx FD                                                
advhh       - advertise 100tx HD                                                
advtf       - advertise 10tx FD                                                 
advth       - advertise 10tx HD                                                 
thrsh       - threshhold mode (0=Disable, 1=mode 5, 2=mode 8)                   
uthrsh      - unify threshhold mode (0=not unify, 1=unify)                      
ptop        - Point to Point enable                                             
grant       - Granted always                                                    
laseronp    - Laser on permanently                                              
pontxdis    - PON TX disable data format (idle code or zeros)                   
idle0       - Idle Byte0 (8/10 coding) (0-Ctrl, 1-Data)                         
idle1       - Idle Byte1 (8/10 coding) (0-Ctrl, 1-Data)                         
idle2       - Idle Byte2 (8/10 coding) (0-Ctrl, 1-Data)                         
idle3       - Idle Byte3 (8/10 coding) (0-Ctrl, 1-Data)                         
idlepre     - Idle Preamble data                                                
user        - 802.1X user name                                                  
passwd      - 802.1X password                                                   
gen1        - General purpose 1 data                                            
gen2        - General purpose 2 data                                            
ponfine     - PON clock fine tune delta                                         
tsfec       - Timestamp delay for FEC delta                                     
arbdelta    - Arbitrator2PON timestamp delta                                    
txdly       - PON clock clib Tx                                                 
rxdly       - PON clock clib Rx  
*/
	if( size == 1 )
		{
		if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
			{
			One_byte = ( char )VOS_StrToUL( argv[2], &pToken, 16 );
			sys_console_printf(" One_byte 0x%x\r\n", One_byte );
			}
		else {
			One_byte = (char )strtol(argv[2], NULL, 10 );
			sys_console_printf(" One_byte %d\r\n", One_byte );
			}

		ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &One_byte, size );
		if( ret != PAS_EXIT_OK ) 
			{
			vty_out(vty,"  %% Executing err\r\n");	
			return( CMD_WARNING );
			}
		}
	else if (size == 2 )
		{
		if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
			{
			Two_byte = ( short int )VOS_StrToUL( argv[2], &pToken, 16 );
			sys_console_printf(" Two_byte 0x%x\r\n", Two_byte );
			}
		else {
			Two_byte = (short int )strtol(argv[2], NULL, 10 );
			sys_console_printf(" Two_byte %d\r\n", Two_byte );
			}
		
		ExchangeWord(&Two_byte );
		ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &Two_byte, size );
		if( ret != PAS_EXIT_OK ) 
			{
			vty_out(vty,"  %% Executing err\r\n");	
			return( CMD_WARNING );
			}
		}
	else if( size == 3 )  /* oui */
		{
		if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
			{
			Four_byte = ( int )VOS_StrToUL( argv[2], &pToken, 16 );
			sys_console_printf(" Two_byte 0x%x\r\n", Four_byte );
			}
		else {
			Four_byte = ( int )strtol(argv[2], NULL, 10 );
			sys_console_printf(" Two_byte %d\r\n", Four_byte );
			}

		}
	else if( size == 4 ) /* ip addr & ip mask */
		{

		}
	else if( size == 6 )  /* mac addr */
		{

		}
	

	return( CMD_SUCCESS);	

}
#endif 

DEFUN(
	onu_emapper_set,
	onu_emapper_set_cmd, 
	"set onu emapper <slot/port/onuid> <1-74> <value>", 
	"config onu emapper\n"
	"config onu emapper\n"
	"config onu emapper\n"
	"input the slot/port/onuid\n"
	"input the onu emapper parameter index,range:1-74\n"
	"the emapper value\n"
	)
{

	short int ret;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, OnuIdx /* llid */;
	int OnuType=V2R1_DEVICE_UNKNOWN;
	char  One_byte;
	unsigned short int Two_byte;
	int Four_byte;
	char N_bytes[64];
	unsigned long size=1;
	EEPROM_mapper_param_t  emapper_Macro = 0xff;
	char Bits5Flag = 0;
	char *pToken;
	char *Ptr;

	if( argc != 3 )
		{
		/*vty_out(vty,"  %% parameter err\r\n");*/
		return( CMD_WARNING );
		}

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

#if 0
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
#endif
		
	GetOnuType( phyPonId, OnuIdx, &OnuType );
	if((OnuType < V2R1_ONU_GT811 ) || ( OnuType >= V2R1_ONU_MAX)) return( CMD_WARNING );


	emapper_Macro =  (UINT32 ) VOS_AtoL( argv[1 ] );
	
	/* 18,19,23 不支持*/
	if( emapper_Macro == 18 )
		{
		vty_out(vty, "onu emapper index %d is not supported\r\n",emapper_Macro);
		return( CMD_WARNING );
		}
	if(( emapper_Macro == 19 ) || ( emapper_Macro == 23 ))
		{
		vty_out(vty, "onu emapper index %d is reserved\r\n",emapper_Macro);
		return( CMD_WARNING );
		}
	
	switch (emapper_Macro)
		{
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 13:
		case 36:
		case 37:
		case 68:
		case 69:
			size = 2;
			break;
			
		case 32:
			size = 3;
			break;
			
		case 33:
			size = 6;
			break;
			
		case 34:
		case 35:
		case 65:
			size = 4;
			break;
			
		case 66:
		case 67:
			size = 32;
			break;
			
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
			Bits5Flag = 1;
			size = 1;
			break;
			
		default:
			size = 1;			

		}
	/* 
	27 变换为73
	28 变换为74
	56变换为75
	*/		
	if(  emapper_Macro == 27 )
		emapper_Macro = EEPROM_MAPPER_DYING_GASP_POLARITY;
	else if ( emapper_Macro == 28 )
		emapper_Macro = EEPROM_MAPPER_LINK_FAULT_POLARITY;
	else if ( emapper_Macro == 56 )
		emapper_Macro = EEPROM_MAPPER_CRITICAL_EVENT_POLARITY;

	/*	(1-17), (24-26),(29 -55) 需减1*/
	else if(( emapper_Macro < EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE )
		||((emapper_Macro >=24) && ( emapper_Macro <= 26))
		||((emapper_Macro >= 29) && ( emapper_Macro <= 55 )))
		emapper_Macro  -= 1;
	/* 57 － 74需减2 */
	else if((emapper_Macro >= 57 ) && ( emapper_Macro <= 74))
		emapper_Macro -= 2;
	/* 其他值(20,21,22 )保持不变*/
	
	/*y_out(vty," emapper_Macro=%d,size=%d, Bits5Flag =%d\r\n", emapper_Macro, size,Bits5Flag);*/
	

	switch( size )
		{
		case 1:  /* 单字节参数*/
			
			if( Bits5Flag == 1 )
				{
				char NegativeFlag=0;

				Ptr = &N_bytes[0];
				VOS_MemSet( Ptr, 0, 64 );
				
				VOS_MemCpy( N_bytes, argv[2], VOS_StrLen(argv[2]));

				if( VOS_MemCmp( argv[2], "-", 1) == 0)  /* 输入值为负*/
					NegativeFlag = 1;
					
				if( NegativeFlag  == 1 )
					{
					Ptr += 1;
					}
				
				if(( VOS_MemCmp(Ptr, "0x", 2) == 0) || ( VOS_MemCmp( Ptr, "0X", 2) == 0) )
					{
					One_byte = ( char )VOS_StrToUL( Ptr, &pToken, 16 );
					}
				else {
					One_byte = (char )strtol(Ptr, NULL, 10 );
					}

				if((( One_byte > 16 ) && ( NegativeFlag == 1)) ||(( One_byte > 15 ) && ( NegativeFlag == 0)))
					{
					vty_out(vty,"  %% parameter out of range");
					if(NegativeFlag  == 1 )
						vty_out(vty,"-");
					vty_out(vty,"%d\r\n", One_byte );
					
					return( CMD_WARNING );
					}

				if( NegativeFlag  == 1 )
					{
					One_byte = 0x20 - One_byte;
					}
				
				}
			
			else {
				if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
					{
					One_byte = ( char )VOS_StrToUL( argv[2], &pToken, 16 );
					}
				else {
					One_byte = (char )strtol(argv[2], NULL, 10 );
					}
				}
			
#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &One_byte, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter( phyPonId, llid, emapper_Macro, &One_byte, size );
#endif
			
			if( ret != PAS_EXIT_OK)
				{
				vty_out(vty, "set parameter err\r\n");
				}
			break;

		case 2:
			if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
				{
				Two_byte = ( short int )VOS_StrToUL( argv[2], &pToken, 16 );
				}
			else {
				Two_byte = (short int )strtol(argv[2], NULL, 10 );
				}
			
			ExchangeWord(&Two_byte );
			
#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &Two_byte, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &Two_byte, size );
#endif
			if( ret != PAS_EXIT_OK ) 
				{
				vty_out(vty, "set parameter err\r\n");
				}
		
			break;

		case 3:   /* OUI */
			{
				
			if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
				{
				Four_byte = ( int )VOS_StrToUL( argv[2], &pToken, 16 );
				}
			else {
				Four_byte = ( int )strtol(argv[2], NULL, 10 );
				}

			if( Four_byte > 0xffffff ) 
				{
				vty_out(vty,"  %% OUI len is 3 byts(24bit),out of range\r\n");
				return( CMD_WARNING );
				}

			ExchangeInt( &Four_byte );

#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &Four_byte, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter(phyPonId, llid, emapper_Macro, &Four_byte, size );
#endif
			if( ret != PAS_EXIT_OK ) 
				{
				vty_out(vty, "set parameter err\r\n");
				}
			}
			
			break;

		case 4:
			Four_byte = 0;
			
			if(emapper_Macro == EEPROM_MAPPER_IDLE_PREAMBLE_DATA )
				{				
				if(( VOS_MemCmp( argv[2], "0x", 2) == 0) || ( VOS_MemCmp( argv[2], "0X", 2) == 0) )
					{
					Four_byte = ( int )VOS_StrToUL( argv[2], &pToken, 16 );
					}
				else {
					Four_byte = ( int )strtol(argv[2], NULL, 10 );
					}
				
				ExchangeInt( &Four_byte );				
				}

			else if((emapper_Macro == EEPROM_MAPPER_EEPROM_IP_ADDR ) || (emapper_Macro == EEPROM_MAPPER_EEPROM_NET_MASK ) )
				{
				int IpAddr[4] = {0};

				sscanf( &argv[2][0], "%d.%d.%d.%d", &IpAddr[0], &IpAddr[1], &IpAddr[2], &IpAddr[3] );
				Four_byte = ((IpAddr[0] & 0xff) << 24) | ((IpAddr[1] & 0xff) << 16) | ((IpAddr[2] & 0xff) << 8) | (IpAddr[3] & 0xff);
				
				ExchangeInt(&Four_byte);
				}
			
			else {
				vty_out(vty,"  %% Parameter err\r\n");
				return( CMD_WARNING );
				}
			
#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &Four_byte, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter( phyPonId, llid, emapper_Macro, &Four_byte, size );
#endif
	
			if( ret != PAS_EXIT_OK)
				{
				vty_out(vty, "set parameter err\r\n");
				}
				
			break;

		case 6:  /* mac address */
			VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
			 if ( GetMacAddr( ( char* ) argv[ 2 ], N_bytes ) != VOS_OK )
				{
				vty_out( vty, "  %% Invalid MAC address.\r\n" );
				return CMD_WARNING;
				}
			
#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter( phyPonId, llid, emapper_Macro, N_bytes, size );
#endif

			if( ret != PAS_EXIT_OK ) 
				{
				vty_out(vty, "set parameter err\r\n");
				}
			
			break;
		
		case 32:
			VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
			VOS_MemCpy(N_bytes, (char *) argv[ 2 ],  32) ;				
			
#if 1
            ret = OnuMgt_SetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, size );
#else
			ret = REMOTE_PASONU_eeprom_mapper_set_parameter( phyPonId, llid, emapper_Macro, N_bytes, size );
#endif

			if( ret != PAS_EXIT_OK ) 
				{
				vty_out(vty, "set parameter err\r\n");
				}
			
			break;
			
		default:
			vty_out(vty, "  %% Executing error\r\n");				

		}
	
	return CMD_SUCCESS;
}

DEFUN(
	onu_emapper_show,
	onu_emapper_show_cmd, 
	"show onu emapper <slot/port/onuid> {<1-74>}*1", 
	"show onu emapper\n"
	"show onu emapper\n"
	"show onu emapper\n"
	"input the slot/port/onuid\n"
	"input the onu emapper parameter index,range:1-74\n"
	)
{
	short int ret;
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, OnuIdx /* llid */;
	int OnuType =V2R1_DEVICE_UNKNOWN;
	unsigned short int Two_byte;
	int Four_byte;
	char One_byte;
	char Bits5Flag;
	char N_bytes[64];
	unsigned long size;
	EEPROM_mapper_param_t  emapper_Macro, emapper_curr;
	EEPROM_mapper_param_t  emapper_begin, emapper_end;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
	}

	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
	{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
	}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

#if 0
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
#endif

	GetOnuType( phyPonId, OnuIdx, &OnuType );
	if((OnuType < V2R1_ONU_GT811 ) || ( OnuType >= V2R1_ONU_MAX)) return( CMD_WARNING );

	if( argc == 1 )
	{
#if 0
	size = 1;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_COOKIE_ID, &One_byte, &size );

	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_COOKIE_ID]);
	
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_RAM_READ_WS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_RAM_READ_WS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_RAM_WRITE_WS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_RAM_WRITE_WS]);	
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	size = 2;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_RAM_SIZE, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_RAM_SIZE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_FIRST_FW_START_ADDR, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_FIRST_FW_START_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SECOND_FW_START_ADDR, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SECOND_FW_START_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else{
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_BOOT_MONITOR_START_ADDR, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_BOOT_MONITOR_START_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_MANAGE_SECS_START_ADDR, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_MANAGE_SECS_START_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	size = 1;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_FLASH_READ_WS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_FLASH_READ_WS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_FLASH_WRITE_WS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_FLASH_WRITE_WS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_FLASH_VENDOR, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_FLASH_VENDOR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_INFO_INDEX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_INFO_INDEX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	size = 2;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_REFRESH_CYCLE, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_REFRESH_CYCLE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord( &Two_byte);
		vty_out(vty,"%d\r\n", Two_byte);
		}

	size = 1;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_TCAS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_TCAS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_TRAS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_TRAS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_WIDTH_MODE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_WIDTH_MODE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_SDRAM_RBC_OR_BRC, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_SDRAM_RBC_OR_BRC]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_BOOT_LOADER_MODE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_BOOT_LOADER_MODE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

/*  rebootcount
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, 18, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[18]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);
*/
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UART_BAUD_INDEX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UART_BAUD_INDEX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_LOSS_SIGNAL_POLARITY, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_LOSS_SIGNAL_POLARITY]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_TBC_SIGNAL_POLARITY, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_TBC_SIGNAL_POLARITY]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_LASER_TX_ENABLE_POLARITY, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_LASER_TX_ENABLE_POLARITY]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	/* GT812 中没有这三项*/
	if( OnuType  != V2R1_ONU_GT812 )
	{
		ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_DYING_GASP_POLARITY, &One_byte, &size );
		vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_DYING_GASP_POLARITY]);
		if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
		else  vty_out(vty,"%d\r\n", One_byte); 

		ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_LINK_FAULT_POLARITY, &One_byte, &size );
		vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_LINK_FAULT_POLARITY]);
		if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
		else  vty_out(vty,"%d\r\n", One_byte);

		ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_CRITICAL_EVENT_POLARITY, &One_byte, &size );
		vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_CRITICAL_EVENT_POLARITY]);
		if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
		else  vty_out(vty,"%d\r\n", One_byte);
	}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_ZBT_USE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_ZBT_USE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_ZBT_CONFIG, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_ZBT_CONFIG]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_CONNECT_ON_POWER_UP, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_CONNECT_ON_POWER_UP]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);

	size = 3;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_OUI_ADDR, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_OUI_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		vty_out(vty,"0x");
		vty_out(vty,"%02x%02x%02x\r\n",N_bytes[2],N_bytes[1],N_bytes[0]);
		/*vty_out(vty,"%02x%02x%02x%02x\r\n",N_bytes[0],N_bytes[1],N_bytes[2],N_bytes[3]);*/
		}

	size = 6;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EEPROM_MAC_ADDR, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EEPROM_MAC_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		/*vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x\r\n", N_bytes[1], N_bytes[0],N_bytes[3],N_bytes[2],N_bytes[5],N_bytes[4]);*/
		vty_out(vty,"%02x%02x.%02x%02x.%02x%02x\r\n", N_bytes[0], N_bytes[1],N_bytes[2],N_bytes[3],N_bytes[4],N_bytes[5]);
		}

	size = 4;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EEPROM_IP_ADDR, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EEPROM_IP_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		vty_out(vty,"%d.%d.%d.%d\r\n", N_bytes[3], N_bytes[2],N_bytes[1],N_bytes[0]);
		}

	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_EEPROM_NET_MASK, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_EEPROM_NET_MASK]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		vty_out(vty,"%d.%d.%d.%d\r\n", N_bytes[3], N_bytes[2],N_bytes[1],N_bytes[0]);
		}

	size = 2;
	Two_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_LASER_TON, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_LASER_TON]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord(&Two_byte);
		vty_out(vty,"0x%x\r\n", Two_byte);
		}

	Two_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_LASER_TOFF, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_LASER_TOFF]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord(&Two_byte);
		vty_out(vty,"0x%x\r\n", Two_byte);
		}

	size = 1;
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_PHY_ADVERTISING_ENABLE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_PHY_ADVERTISING_ENABLE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_BRIDGE_ENABLE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_BRIDGE_ENABLE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_AUTONEG_ENABLE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_AUTONEG_ENABLE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_MDIO_EXTERN_PHY_ADDR, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_MDIO_EXTERN_PHY_ADDR]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_MAC_TYPE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_MAC_TYPE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_MASTER_MODE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_MASTER_MODE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_1000T_MULTI_PORT, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_1000T_MULTI_PORT]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_1000T_HALF_DUPLEX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_1000T_HALF_DUPLEX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_1000T_FULL_DUPLEX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_1000T_FULL_DUPLEX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ASYMMETRIC, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ASYMMETRIC]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ENABLED, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ENABLED]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_100T4, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_100T4]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_100TX_FD, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_100TX_FD]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_100TX_HD, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_100TX_HD]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_10TX_FD, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_10TX_FD]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);
		

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNI_ADVERTISE_10TX_HD, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNI_ADVERTISE_10TX_HD]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else  vty_out(vty,"%d\r\n", One_byte);	
	
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_THRESHOLD_MODE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_THRESHOLD_MODE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte);

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_UNIFY_THRESHOLD_MODE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_UNIFY_THRESHOLD_MODE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_POINT_TO_POINT_ENABLE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_POINT_TO_POINT_ENABLE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_GRANTED_ALWAYS, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_GRANTED_ALWAYS]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_LASER_ON_PERMANENTLY, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_LASER_ON_PERMANENTLY]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_TX_DISABLE_DATA_FORMAT, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_TX_DISABLE_DATA_FORMAT]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_IDLE_BYTE0, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_IDLE_BYTE0]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_IDLE_BYTE1, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_IDLE_BYTE1]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_IDLE_BYTE2, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_IDLE_BYTE2]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_IDLE_BYTE3, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_IDLE_BYTE3]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%d\r\n", One_byte );

	size = 4;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_IDLE_PREAMBLE_DATA, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_IDLE_PREAMBLE_DATA]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else{
		Four_byte = N_bytes[3]<< 24 ;
		Four_byte += N_bytes[2] << 16;
		Four_byte += N_bytes[1] << 8;
		Four_byte += N_bytes[0];
		vty_out(vty,"%d (0x%02x%02x%02x%02x)\r\n", Four_byte, N_bytes[3],N_bytes[2], N_bytes[1],N_bytes[0]);
		}

	size = 32;	
	VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_USER_NAME_802_1X, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_USER_NAME_802_1X]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%s\r\n", N_bytes);

	size = 32;	
	VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PASSWORD_802_1X, N_bytes, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PASSWORD_802_1X]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else vty_out(vty,"%s\r\n", N_bytes);

	size = 2;
	Two_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_GENERAL_PURPOSE_FIELD1, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_GENERAL_PURPOSE_FIELD1]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord(&Two_byte);
		vty_out(vty,"%d (0x%x)\r\n", Two_byte,Two_byte);
		}

	Two_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_GENERAL_PURPOSE_FIELD2, &Two_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_GENERAL_PURPOSE_FIELD2]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		ExchangeWord(&Two_byte);
		vty_out(vty,"%d (0x%x)\r\n", Two_byte, Two_byte);
		}

	size = 1;
	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_CLOCK_FINE_TUNE, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_CLOCK_FINE_TUNE]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else{
		if( Handle5BitData ( &One_byte) == 1 )
			vty_out(vty,"-");
		vty_out(vty,"%d \r\n", One_byte);
		}

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_TIMESTAMP_DELAY_FEC, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_TIMESTAMP_DELAY_FEC]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else{
		if( Handle5BitData ( &One_byte) == 1 )
			vty_out(vty,"-");
		vty_out(vty,"%d \r\n", One_byte);
		}

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		if( Handle5BitData ( &One_byte) == 1 )
			vty_out(vty,"-");
		vty_out(vty,"%d \r\n", One_byte);
		}

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_CLK_CALIB_TX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_CLK_CALIB_TX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else {
		if( Handle5BitData ( &One_byte) == 1 )
			vty_out(vty,"-");
		vty_out(vty,"%d \r\n", One_byte);
		}

	One_byte = 0;
	ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, EEPROM_MAPPER_PON_CLK_CALIB_RX, &One_byte, &size );
	vty_out(vty,"%-12s", onu_emapper_string[EEPROM_MAPPER_PON_CLK_CALIB_RX]);
	if( ret != PAS_EXIT_OK)
		{
		vty_out(vty, "err\r\n");
		}
	else{
		if( Handle5BitData ( &One_byte) == 1 )
			vty_out(vty,"-");
		vty_out(vty,"%d \r\n", One_byte);
		}
#else        
        emapper_begin = 1;
        emapper_end = 74;
#endif    
	}
	
	else if( argc == 2 )  /* 显示单一参数*/
	{
#if 0
		emapper_Macro =  (UINT32 ) VOS_AtoL( argv[1 ] );
#else
		emapper_begin =  (UINT32 ) VOS_AtoL( argv[1 ] );
        emapper_end = emapper_begin;
#endif    
    }
    else
    {
        VOS_ASSERT(0);
        emapper_begin = 0;
        emapper_end = -1;
    }
    
#if 1
    for ( emapper_curr = emapper_begin; emapper_curr <= emapper_end; ++emapper_curr )
    {
        Bits5Flag = 0;
		switch (emapper_Macro = emapper_curr)
		{
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 13:
			case 36:
			case 37:
			case 68:
			case 69:
				size = 2;
				break;
				
			case 32:
				size = 3;
				break;
				
			case 33:
				size = 6;
				break;
				
			case 34:
			case 35:
			case 65:
				size = 4;
				break;
				
			case 66:
			case 67:
				size = 32;
				break;
				
			case 70:
			case 71:
			case 72:
			case 73:
			case 74:
				Bits5Flag = 1;
				size = 1;
				break;
                
    		/* 18,19,23 不支持*/
			case 18:
    		    if ( emapper_begin == emapper_end )
                {
        			vty_out(vty, "onu emapper index %d is not supported\r\n",emapper_Macro);
        			return( CMD_WARNING );
                }      
                else
                {
                    continue;
                }

                break;
			case 19:
			case 23:
    		    if ( emapper_begin == emapper_end )
                {
        			vty_out(vty, "onu emapper index %d is reserved\r\n",emapper_Macro);
        			return( CMD_WARNING );
                }      
                else
                {
                    continue;
                }

                break;
			default:
				size = 1;			
		}
        
		/* 
		27 变换为73
		28 变换为74
		56变换为75
		*/		
		if(  emapper_Macro == 27 )
			emapper_Macro = EEPROM_MAPPER_DYING_GASP_POLARITY;
		else if ( emapper_Macro == 28 )
			emapper_Macro = EEPROM_MAPPER_LINK_FAULT_POLARITY;
		else if ( emapper_Macro == 56 )
			emapper_Macro = EEPROM_MAPPER_CRITICAL_EVENT_POLARITY;

		/*	(1-17), (24-26),(29 -55) 需减1*/
		else if(( emapper_Macro < EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE )
			||((emapper_Macro >=24) && ( emapper_Macro <= 26))
			||((emapper_Macro >= 29) && ( emapper_Macro <= 55 )))
			emapper_Macro  -= 1;
		/* 57 － 74需减2 */
		else if((emapper_Macro >= 57 ) && ( emapper_Macro <= 74))
			emapper_Macro -= 2;
		/* 其他值(20,21,22 )保持不变*/
		
		/*vty_out(vty," emapper_Macro=%d,size=%d, Bits5Flag =%d\r\n", emapper_Macro, size,Bits5Flag);*/		

		switch( size )
		{
			case 1:			
				One_byte = 0;
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &One_byte, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, &One_byte, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
                {
					if( Bits5Flag == 1 )
					{
						if( Handle5BitData ( &One_byte) == 1 )
							vty_out(vty,"-");
					}
					vty_out(vty,"%d \r\n", One_byte);
				}
				break;

			case 2:
				Two_byte = 0;
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &Two_byte, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, &Two_byte, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
                {
					ExchangeWord(&Two_byte);				
					vty_out(vty,"%d \r\n", Two_byte);
				}
				break;

			case 3:
				VOS_MemSet( N_bytes, 0, sizeof(N_bytes));
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, N_bytes, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
                {
					vty_out(vty,"0x");
					vty_out(vty,"%02x%02x%02x\r\n",N_bytes[2],N_bytes[1],N_bytes[0]);
				}
				break;

			case 4:
				VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
				Four_byte = 0;
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, N_bytes, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
                {
					if(emapper_Macro == EEPROM_MAPPER_IDLE_PREAMBLE_DATA )
					{
						Four_byte = N_bytes[3]<< 24 ;
						Four_byte += N_bytes[2] << 16;
						Four_byte += N_bytes[1] << 8;
						Four_byte += N_bytes[0];
						vty_out(vty,"%d (0x%02x%02x%02x%02x)\r\n", Four_byte, N_bytes[3],N_bytes[2], N_bytes[1],N_bytes[0]);
					}
					else /*if((emapper_Macro == EEPROM_MAPPER_EEPROM_IP_ADDR ) || (emapper_Macro == EEPROM_MAPPER_EEPROM_NET_MASK ) )*/
					{
						vty_out(vty,"%d.%d.%d.%d\r\n", N_bytes[3], N_bytes[2],N_bytes[1],N_bytes[0]);
					}
				}
				break;

			case 6:
				VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, N_bytes, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
                {
					vty_out(vty,"%02x%02x.%02x%02x.%02x%02x\r\n", N_bytes[0], N_bytes[1],N_bytes[2],N_bytes[3],N_bytes[4],N_bytes[5]);
				}
				break;
			
			case 32:
				VOS_MemSet( N_bytes, 0, sizeof( N_bytes));
#if 1
                ret = OnuMgt_GetOnuI2CInfo( phyPonId, OnuIdx, emapper_Macro, &N_bytes, &size );
#else
				ret = REMOTE_PASONU_eeprom_mapper_get_parameter( phyPonId, llid, emapper_Macro, N_bytes, &size );
#endif
				vty_out(vty,"%-12s", onu_emapper_string[emapper_Macro]);
				if( ret != PAS_EXIT_OK)
				{
					vty_out(vty, "err\r\n");
				}
				else
				{
                    vty_out(vty,"%s\r\n", N_bytes);
				}
				break;
				
			default:
				vty_out(vty, "  %% Executing error\r\n");				

		}
    }
#endif    
		
	
	return CMD_SUCCESS;
}

DEFUN(
	onu_emapper_list_show,
	onu_emapper_list_show_cmd, 
	"show onu emapper list {<1-74>}*1", 
	"show onu emapper list\n"
	"show onu emapper list\n"
	"show onu emapper list\n"
	"show onu emapper list\n"
	"input the onu emapper parameter index,range:1-74\n"
	)
{

	int size;
	
	if( argc == 0)
		{
		for ( size= 1; size <= 74; size ++ )
			vty_out(vty, "%s\r\n", onu_emapper_string_list[size] );
		}
	else {
		size = ( UINT32 ) VOS_AtoL( argv[ 0 ] );
		vty_out(vty, "%s\r\n", onu_emapper_string_list[size] );
		}
	return( CMD_SUCCESS );

}


DEFUN(
	onu_timesync_gap_set,
	onu_timesync_gap_cmd, 
	"set onu time-sync interval <30-3600>", 
	"set onu time-sync interval\n"
	"set onu time-sync interval\n"
	"set onu time-sync interval\n"
	"set onu time-sync interval\n"
	"input the interval,unit:S\n"
	)
{


	if( argc != 1 ) 
		{
		/*vty_out(vty, " %% Parameter err\r\n");*/
		return( CMD_WARNING );
		}	

	V2R1_SYS_TIME_PERIOD = ( unsigned int ) VOS_AtoL( argv[ 0 ] );

	return(CMD_SUCCESS);
}

DEFUN(
	onu_timesync_gap_show,
	onu_timesync_gap_show_cmd, 
	"show onu time-sync interval", 
	"show onu time-sync interval\n"
	"show onu time-sync interval\n"
	"show onu time-sync interval\n"
	"show onu time-sync interval\n"
	)
{


	vty_out(vty," onu time-sync msg interval is %d(s)\r\n", V2R1_SYS_TIME_PERIOD );

	return(CMD_SUCCESS);
}

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
extern ULONG g_ctcSaveDebFlag;
DEFUN(
	ctc_save_config_debug,
	ctc_save_config_debug_cmd,
	"ctccfgsave debug",
	"show or configure ctc onu configuration data instructions\n"
	"set ctc onu configuration debug flag\n"
)
{
	if( 1 == g_ctcSaveDebFlag )
		vty_out( vty, "ctc config debug flag was already turned on!\r\n" );
	else
	{
		g_ctcSaveDebFlag = 1;
		vty_out( vty, "ctc config debug flag has  turned on!\r\n" );

	}
	return (CMD_SUCCESS );
}

DEFUN(
	undo_ctc_save_config_debug,
	undo_ctc_save_config_debug_cmd,
	"undo ctccfgsave debug",
	NO_STR
	"show or configure ctc onu configuration data instructions\n"
	"set ctc onu configuration debug flag\n"
)
{
	if( 0 == g_ctcSaveDebFlag )
		vty_out( vty, "ctc config debug flag was already turned off!\r\n" );
	else
	{
		g_ctcSaveDebFlag = 0;
		vty_out( vty, "ctc config debug flag has  turned off!\r\n" );
	}
	
	return (CMD_SUCCESS );
}
#endif

#ifdef ETH_LOOP_DEB
#endif

extern ULONG g_ethLoopbackPrint;
DEFUN(
	eth_loop_debug,
	eth_loop_debug_cmd,
	"ethlpb debug",
	"ethernet port loopback\n"
	"set loopback debug flag\n"
)
{
	/*if( 1 == g_ethLoopbackPrint )
		vty_out( vty, "eth port loopback deug flag was already turned on!\r\n" );
	else*/
	{
		g_ethLoopbackPrint = 1;
		/*vty_out( vty, "eth port loopback deug flag has turned on!\r\n"  );*/
	}
	return (CMD_SUCCESS );
}

DEFUN(
	undo_eth_loop_debug,
	undo_eth_loop_debug_cmd,
	"undo ethlpb debug",
	NO_STR
	"ethernet port loopback\n"
	"set loopback debug flag\n"
)
{
	/*if( 0 == g_ethLoopbackPrint )
		vty_out( vty, "eth port loopback deug flag was already turned off!\r\n" );
	else*/
	{
		g_ethLoopbackPrint = 0;
		/*vty_out( vty, "eth port loopback deug flag has turned off!\r\n"  );*/
	}
	return (CMD_SUCCESS );
}

DEFUN(
	set_pon_discard_unlearned_addr_func,
	set_pon_discard_unlearned_addr_cmd,
	"pon-port <slot/port> discard-unlearned-sa [enable|disable] discard-unknown-da [uplink|downlink|all-direction|no-direction]",
	"set pon forward rule\n"
	"input the slot/port\n"
	"whether discard unlearned src address\n"
	"enable:discard unlearned address\n"
	"disable:not discard unlearned address\n"
	"whether discard unknown dest address(this parameter is ignored in pas5001)\n"
	"uplink:uplink direction discard unknown dest address\n"
	"downlink:downlink direction discard unknown dest address\n"
	"all-direction:uplink & downlink direction discard unknown dest address\n"
	"no-direction:no direction discard unknown dest address\n"
	)
	
{
	short int PonPortIdx;
	short int PonChipType;
	short int ret;
	unsigned long ulSlot, ulPort;
	#if 1/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_addr_table_cfg_t addrtbl_cfg;
	#else
	PON_address_table_config_t address_table_config;
	#endif

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}

	PonChipType = V2R1_GetPonchipType(PonPortIdx);

	if ( !( VOS_StrCmp( argv[ 1 ] , "enable" ) ) )
		{
		V2R1_discard_llid_unlearned_sa = TRUE;
		}
	
	else{
		V2R1_discard_llid_unlearned_sa = FALSE;
		V2R1_discard_unknown_da = PON_DIRECTION_NO_DIRECTION;
		}

	if( !( VOS_StrCmp( argv[ 2 ] , "uplink" ) ) )
		{
		V2R1_discard_unknown_da = PON_DIRECTION_UPLINK;
		}
	else if( !( VOS_StrCmp( argv[ 2 ] , "downlink" ) ) )
		{
		V2R1_discard_unknown_da = PON_DIRECTION_DOWNLINK;
		}
	else if( !( VOS_StrCmp( argv[ 2 ] , "all-direction" ) ) )
		{
		V2R1_discard_unknown_da = PON_DIRECTION_UPLINK_AND_DOWNLINK;
		}
	else /*if( !( VOS_StrCmp( argv[ 2 ] , "no-direction" ) ) )*/
		{
		V2R1_discard_unknown_da = PON_DIRECTION_NO_DIRECTION;
		}
	
    #if 1
	VOS_MemZero( &addrtbl_cfg, sizeof(addrtbl_cfg) );
	ret = OLT_GetAddressTableConfig(PonPortIdx, &addrtbl_cfg );
	if( ret != PAS_EXIT_OK ) 
		{
		vty_out(vty,"  %% Execute error\r\n");
		return( CMD_WARNING );
		}
	
	addrtbl_cfg.discard_llid_unlearned_sa = V2R1_discard_llid_unlearned_sa;
	addrtbl_cfg.discard_unknown_da = V2R1_discard_unknown_da;

	ret =  OLT_SetAddressTableConfig(PonPortIdx, &addrtbl_cfg );
	#else
	ret = PAS_get_address_table_configuration(PonPortIdx, &address_table_config );
	if( ret != PAS_EXIT_OK ) 
		{
		vty_out(vty,"  %% Execute error\r\n");
		return( CMD_WARNING );
		}
	
	address_table_config.discard_llid_unlearned_sa = V2R1_discard_llid_unlearned_sa;
	address_table_config.discard_unknown_da = V2R1_discard_unknown_da;

	ret =  PAS_set_address_table_configuration(PonPortIdx, address_table_config );
    #endif
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    if ( OLT_CALL_ISERROR(ret) )
    {
        switch (ret)
        {
            case OLT_ERR_NOTSUPPORT:
                vty_out( vty, "  %% %s not support this command\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
            	return ( CMD_WARNING );
            default:
        		vty_out(vty,"set err %d\r\n", ret);
            	return ( CMD_WARNING );
        }
    }

	return( CMD_SUCCESS );

}


DEFUN(
	show_pon_discard_unlearned_addr_func,
	show_pon_discard_unlearned_addr_cmd,
	"show pon-port address table config <slot/port>",
	"show pon-port address table config info\n"
	"show pon-port address table config info\n"
	"show pon-port address table config info\n"
	"show pon-port address table config info\n"
	"show pon-port address table config info\n"
	"input the slot/port\n"
	)
	
{
	short int PonPortIdx, PonChipType;
	unsigned long ulSlot, ulPort;
#if 0    
	short int ret;
	PON_address_table_config_t address_table_config;
#else
	int ret;
    OLT_addr_table_cfg_t address_table_config;
#endif

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}

#if 1
    ret = OLT_GetAddressTableConfig(PonPortIdx, &address_table_config);
    if ( OLT_CALL_ISERROR(ret) )
    {
        switch (ret)
        {
            case OLT_ERR_NOTSUPPORT:
        		vty_out(vty, "  %% this is %s, this cli is not supported\r\n", OLTAdv_GetChipTypeName(PonPortIdx));
                break;
            case OLT_ERR_NOTEXIST:
        		vty_out(vty,"  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
                break;
            default:
        		vty_out(vty,"  %% Execute error\r\n");
        }

		return( CMD_WARNING);
    }
#else	
	PonChipType = V2R1_GetPonchipType(PonPortIdx);
	
	if( PonChipType != PONCHIP_PAS5201)
		{
		vty_out(vty," pon chip type is not supported by this cmd\r\n");
		return( CMD_WARNING);
		}
	
	ret = PAS_get_address_table_configuration(PonPortIdx, &address_table_config );
	if( ret != PAS_EXIT_OK ) 
		{
		vty_out(vty,"  %% Execute error\r\n");
		return( CMD_WARNING );
		}
#endif

	vty_out(vty, "  pon %d/%d address table config info\r\n", ulSlot, ulPort );

	vty_out(vty, "  address aging time:%d(s)\r\n", (address_table_config.aging_timer/SECOND_1 ));
	if( address_table_config.removed_when_aged == TRUE ) vty_out(vty, "  removed_when_aged:true\r\n");
	else vty_out(vty, "  removed_when_aged:false\r\n");
	if( address_table_config.allow_learning > 3 )
		address_table_config.allow_learning = 3;
	vty_out(vty, "  address learning from %s", PonLinkDirection_s[address_table_config.allow_learning]);
	if( PonChipType == PONCHIP_PAS5001)
		vty_out(vty,"(this parameter is ignored in pas5001)\r\n");
	else vty_out(vty,"\r\n");
	vty_out(vty, "  discard unlearned sa:");
	if( address_table_config.discard_llid_unlearned_sa  == TRUE ) vty_out(vty, "true\r\n");
	else vty_out(vty, "false\r\n");
	if( address_table_config.discard_unknown_da > 3 )
		address_table_config.discard_unknown_da = 3;
	vty_out(vty, "  discard unlearned da:%s", PonLinkDirection_s[address_table_config.discard_unknown_da]);
	if( PonChipType == PONCHIP_PAS5001)
		vty_out(vty,"(this parameter is ignored in pas5001)\r\n");
	else vty_out(vty,"\r\n");
	
	return( CMD_SUCCESS );

}

#if 0	/* removed by xieshl 20101210, 问题单11503 */
/* added by chenfj 2007-10-23
    在debug命令节点下，增加通过FTP 升级所有PON口下所有在线ONU程序的命令
*/
DEFUN(ftpc_download_ftp_phenixos_master_func_for_all_pons,
 ftpc_download_ftp_phenixos_master_for_all_pons_cmd,
 "download ftp [os] <A.B.C.D> <user> <pass> <filename>",
 /*"Download GW OS\n"*/
 "Download OS\n"
 "Download file using ftp protocol\n"
 "Download new GROS image\n"
 "Please input ftp server's IP address\n"
 "Please input user name\n" 
 "Please input the password\n" 
 "Please input the file name\n"
 )
{
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	/*ULONG   ulIfIndex = 0;*/
	ULONG   ulSlot, ulPort;
	USHORT length = 0;
	cliPayload *stPayload=NULL;
	/*int OnuType;*/

	if( argc != 5 )
		{
		/*vty_out(vty, " %%Param err\r\n");*/
		return( CMD_WARNING );
		}
	/*
	ulIfIndex =(ULONG) vty->index;
	
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
	
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
	*/
	for(ulSlot = PONCARD_FIRST; ulSlot <= PONCARD_LAST; ulSlot ++)
		{
		for( ulPort = 1; ulPort <= PONPORTPERCARD; ulPort++ )			
			{
			PonPortIdx = GetPonPortIdxBySlot( ulSlot, ulPort);
			
			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
				{				
				 /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
				lRet = GetOnuOperStatus( PonPortIdx, OnuIdx );
				if ( CLI_EPON_ONUUP != lRet) continue;

				/*
				if( GetOnuType( PonPortIdx, OnuIdx, &OnuType) != ROK )
					continue;

				if(( OnuType != V2R1_ONU_GT810 ) && ( OnuType != V2R1_ONU_GT816 ) && ( OnuType != V2R1_ONU_GT811_A) && ( OnuType != V2R1_ONU_GT812_A))
					continue;
				*/
				
				VOS_MemCpy( pBuff, vty->buf, vty->length);
				length = vty->length;	
				stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
				stPayload->fd = vty->fd;

				lRet=lCli_SendbyOam( PonPortIdx, (OnuIdx+1), pBuff, length, stPayload, vty);
				}
			
			}

		}
	
    return CMD_SUCCESS;
}
#endif

DEFUN(onu_pas_pq_occupancy,
 onu_pas_pq_occupancy_cmd,
 "pq occupancy <slot/port/onuid> [up|down|p2c|c2p|u2c]", 
 "Priority queue occupancy status\n"
 "Show priority queue occupancy status\n"
 "please input sloltid/port/onuId\n"
 "Direction: Upstream\n"
 "Direction: Downstream\n"
 "Direction: Pon to Cpu\n"
 "Direction: Cpu to Pon\n"
 "Direction: Uni to Cpu\n"
 )
{
	LONG lRet;
	CHAR pBuff[256]={0};   
	short int PonPortIdx =0;
	short int OnuIdx;
	/*ULONG   ulIfIndex = 0;*/
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	/*ULONG ulRet;*/
	cliPayload *stPayload=NULL;
	/*ULONG ulOnuFeid;*/
	
/******************/
if(( argc !=  2 ) )
		{
		/*vty_out(vty, " %% Parameter err\r\n");*/
		return( CMD_WARNING );
		}
	
	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	IFM_ParseSlotPort( argv[0], (ULONG *)&ulSlot, (ULONG *)&ulPort );
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(ulSlot) != ROK )
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (PonPortIdx == VOS_ERROR)
		{
#ifdef CLI_EPON_DEBUG
		vty_out(vty, "  %% DEBUG: phyPonId %d.\r\n", PonPortIdx);
#endif   
		vty_out( vty, "  %% pon port Parameter is error.\r\n" );

		return CMD_WARNING;
		}

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

	VOS_Sprintf(pBuff,"pq occupancy ");
	length = strlen(pBuff);
	VOS_Sprintf(&(pBuff[length])," %s", argv[1] );
	length = strlen(pBuff);

	stPayload = VOS_Malloc( sizeof(struct cli_payload), MODULE_RPU_IFM);
	stPayload->fd = vty->fd;

	lRet=lCli_SendbyOam( PonPortIdx, ulOnuid, pBuff, length, stPayload, vty);
	if(lRet != VOS_OK)
		{
		vty_out(vty, "  %% atu limit failed!\r\n");	
		return CMD_WARNING;
		}
    return CMD_SUCCESS;
}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
DEFUN( debug_tdm_api_func,
           debug_tdm_api_func_cmd,
           "debug tdm [info|rxmsg|rxraw|txmsg|txraw]",
           DEBUG_STR
           "tdm api function information\n"
           "general debug output information\n"
           "received tdm message\n"
           "received tmd comm raw data\n"
           "sent tdm message\n"
           "sent tdm comm raw data\n"
           "output data line width\n")
{
  	if( VOS_StriCmp( argv[0], "info" ) == 0 )
		setTdmDebugFlag( TDM_DEBUG_INFO );
	else if( VOS_StriCmp( argv[0], "rxmsg" ) == 0 )
		setTdmDebugFlag( TDM_DEBUG_RECV_MSG );
	else if( VOS_StriCmp( argv[0], "rxraw" ) == 0 )
		setTdmDebugFlag( TDM_DEBUG_RECV_RAW );
	else if( VOS_StriCmp( argv[0], "txmsg" ) == 0 )
		setTdmDebugFlag( TDM_DEBUG_SEND_MSG );
	else if( VOS_StriCmp( argv[0], "txraw" ) == 0 )
		setTdmDebugFlag( TDM_DEBUG_SEND_RAW );
      else
	  	return CMD_WARNING;
      	return CMD_SUCCESS;
}

DEFUN( no_debug_tdm_api_func,
           no_debug_tdm_api_func_cmd,
           "undo debug tdm [info|rxmsg|rxraw|txmsg|txraw]",
           DEBUG_STR
           "tdm api function information\n"
           "general debug output information\n"
           "received tdm message\n"
           "received tmd comm raw data\n"
           "sent tdm message\n"
           "sent tdm comm raw data\n")
{
	if( VOS_StriCmp( argv[0], "info" ) == 0 )
		undoSetTdmDebugFlag( TDM_DEBUG_INFO );
	else if( VOS_StriCmp( argv[0], "rxmsg" ) == 0 )
		undoSetTdmDebugFlag( TDM_DEBUG_RECV_MSG );
	else if( VOS_StriCmp( argv[0], "rxraw" ) == 0 )
		undoSetTdmDebugFlag( TDM_DEBUG_RECV_RAW );
	else if( VOS_StriCmp( argv[0], "txmsg" ) == 0 )
		undoSetTdmDebugFlag( TDM_DEBUG_SEND_MSG );
	else if( VOS_StriCmp( argv[0], "txraw" ) == 0 )
		undoSetTdmDebugFlag( TDM_DEBUG_SEND_RAW );
	else
	  	return CMD_WARNING;
		
	return CMD_SUCCESS;
}

DEFUN( debug_ETH_port_isolate,
           debug_ETH_port_isolate_cmd,
           "isolate src-prot <1-20> dst-port <1-20>",
           "isolate two eth port\n"
           "the src port\n"
           "input port num\n"
           "the dst port\n"
           "input port num\n"
      )
{
	unsigned int src_port, dst_port;
      if( argc == 2 )
      {
	  	src_port =  ( unsigned int ) VOS_AtoL( argv[ 0 ] );
		dst_port =  ( unsigned int ) VOS_AtoL( argv[ 1 ] );
		test1EthPortIsolate(src_port, dst_port,1);
	      	return VOS_OK;
      }
      else
	  	return CMD_WARNING;
}

DEFUN( undo_debug_ETH_port_isolate,
           undo_debug_ETH_port_isolate_cmd,
           "undo isolate src-prot <1-20> dst-port <1-20>",
           "undo isolate two eth port\n"
           "undo isolate two eth port\n"
           "the src port\n"
           "input port num\n"
           "the dst port\n"
           "input port num\n"
      )
{
	unsigned int src_port, dst_port;
      if( argc == 2 )
      {
	  	src_port =  ( unsigned int ) VOS_AtoL( argv[ 0 ] );
		dst_port =  ( unsigned int ) VOS_AtoL( argv[ 1 ] );
		test1EthPortIsolate(src_port, dst_port,0);
	      	return VOS_OK;
      }
      else
	  	return CMD_WARNING;
}
#endif

/*  测试OLT PON芯片外部BUF ；当前仅支持PAS5201 */
DEFUN( test_pon_ext_buf,
           test_pon_ext_buf_cmd,
           "test pon external-data-buffer <slot/port>",
           "test pon port\n"
           "test pon port\n"
           "test pon port external data buffer\n"
           "input slot/port\n"
      )
{
	short int PonPortIdx, PonChipType;
	short int ret1,ret2;
	unsigned long ulSlot, ulPort;
	PON_olt_response_parameters_t pon_updated_parameters;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (PonPortIdx == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	PonChipType = V2R1_GetPonchipType(PonPortIdx);

	if( OLT_PONCHIP_ISPAS5001(PonChipType))
	{
		vty_out(vty," pon%d/%d has no external data buffer\r\n",ulSlot, ulPort);
		return( CMD_WARNING);
	}

	CHECK_PON_RANGE

	if( OLTAdv_IsExist(PonPortIdx) != TRUE )
	{
		vty_out(vty,"pon%d/%d is not working\r\n",ulSlot, ulPort );
		return( CMD_WARNING );
	}
	 /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret1 = OLT_GetOltParameters( PonPortIdx, &pon_updated_parameters );
	if( ret1 != PAS_EXIT_OK )
	{
		vty_out(vty,"Execute err\r\n");
		return (CMD_WARNING );
	}
	if(pon_updated_parameters.external_downlink_buffer_size == PON_EXTERNAL_DOWNLINK_BUFFER_0MB )
	{
		vty_out(vty,"pon%d/%d has no external data buffer\r\n", ulSlot, ulPort);
		return( CMD_WARNING );
	}
		
	ret1 = PAS_test_downlink_buffer(PonPortIdx, PON_TEST_DOWNLINK_BUFFER_DATA);
	if( ret1 != PAS_EXIT_OK )
	{
		vty_out(vty," pon%d/%d external data buffer DATA-BUS err\r\n", ulSlot, ulPort );
	}
	ret2 = PAS_test_downlink_buffer(PonPortIdx, PON_TEST_DOWNLINK_BUFFER_ADDRESS);
	if( ret2 != PAS_EXIT_OK )
	{
		vty_out(vty," pon%d/%d external data buffer ADDRESS-BUS err\r\n", ulSlot, ulPort );
	}
	if((ret1 == PAS_EXIT_OK) && ( ret2 == PAS_EXIT_OK ))
		vty_out(vty,"test pon%d/%d external data buffer ok\r\n",ulSlot, ulPort );

	return(CMD_SUCCESS) ;

}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#ifdef	TDM_DEBUG_CLI
#endif

	/*begin
	added by wangxiaoyu 2008-01-24
	end*/
	
#define    reports(a,b)    vty_out(vty, "\r\n%-40s%s", a, b )
#define    reportd(a,b)    vty_out(vty, "\r\n%-40s%ld", a, b )

extern tdm_msg_statistic_t tdmCommStatis;

DEFUN(
	tdm_comm_queue_show,
	tdm_comm_queue_show_cmd,
	"show tdm queue {[stats|mem]}*1",
	"show instructions\n"
	"show debug information about tdm\n"
	"show tdm communication queue information\n"
	"show statistic about tdm queue only\n"
	"show memory of tdm queue\n"
)
{
	if(argc > 1)
	{
		/*vty_out(vty, "too many parameter!\r\n");*/
		return (CMD_WARNING);
	}

	if(argc == 1 && VOS_StriCmp(argv[0],"mem") == 0)
	{
		reports("tdm queue memory show:", "");
		showTdmQueue(vty, MAX_TDM_QUEUE_LEN);
	}
	else if(argc == 1 && VOS_StriCmp(argv[0],"stats") == 0)
	{
		reports("tdm statistic show:", "");
		reports("", "");		
		reportd("queue item used:", tdmCommStatis.ulQueUsedCount);
		reportd("send OK events:", tdmCommStatis.ulQueSendTotalCount);
		reportd("recieved OK events:", tdmCommStatis.ulQueRecvTotalCount);
		reportd("full queue events:", tdmCommStatis.ulQueFullCount);
		reportd("receive fail events:", tdmCommStatis.ulQueRecvTimeoutCount);
		reportd("send fail events:", tdmCommStatis.lQueSendErrCount);
		reports("", "\r\n");
	}
	else
	{
		reports("tdm statistic show:", "");
		reports("", "");		
		reportd("queue item used:", tdmCommStatis.ulQueUsedCount);
		reportd("send OK events:", tdmCommStatis.ulQueSendTotalCount);
		reportd("recieved OK events:", tdmCommStatis.ulQueRecvTotalCount);
		reportd("full queue events:", tdmCommStatis.ulQueFullCount);
		reportd("receive fail events:", tdmCommStatis.ulQueRecvTimeoutCount);
		reportd("send fail events:", tdmCommStatis.lQueSendErrCount);
		reports("", "");

		reports("tdm queue memory show:", "");
		showTdmQueue(vty, MAX_TDM_QUEUE_LEN);
	}

	
	return (CMD_SUCCESS );
}

#endif
/*added by wutw 2006/12/27*/


/* added by chenfj 2008-4-18 
     增加交换板上板间通道接口重启动命令
     */
extern void ReInitCtrlChannel();
DEFUN(
	restart_sw_ctrlchannel,
	restart_sw_ctrlchanne_cmd,
	"restart mgmt-channel",
	"restart sw-pon mgmt channel\n"
)
{
	ReInitCtrlChannel();
	return (CMD_SUCCESS);
}

DEFUN(
	onu_encrypt_update_flag,
	onu_encrypt_update_flag_cmd,
	"onu-encrypt-update [enable|disable]",
	"onu encrypt update flag\n"
	"enable\n"
	"disable\n"
)
{
	if(VOS_StriCmp(argv[0],"enable") == 0)
		UpdateKeyFlag = V2R1_ENABLE;
	else /*if(VOS_StriCmp(argv[0],"disable") == 0)*/
		UpdateKeyFlag = V2R1_DISABLE;
	return (CMD_SUCCESS);
}
#if 0
DEFUN(
	show_ctrlchanne_stat,
	show_ctrlchanne_stat_cmd,
	"show mgmt-channel-stat",
	"show sw-pon mgmt channel statistics\n"
)
{
	extern unsigned long iPacketsReceivedPerSecond;
	extern unsigned long iPacketsTransmitPerSecond;
	extern unsigned long iPacketsReceivedPerSecondMax;
	extern unsigned long iPacketsTransmitPerSecondMax;
	extern unsigned long iPacketsReceivedPerSecondAvg;
	extern unsigned long iPacketsTransmitPerSecondAvg;

	vty_out(vty, "                 Current        Max            Average\r\n");
	vty_out(vty," ----------------------------------------------------------------------\r\n");
	VOS_TaskLock();
	vty_out(vty,"  Received       %-15d%-15d%-15d\r\n", 
				iPacketsReceivedPerSecond,iPacketsReceivedPerSecondMax, iPacketsReceivedPerSecondAvg);
	vty_out(vty,"  Transmitted    %-15d%-15d%-15d\r\n", 
				iPacketsTransmitPerSecond,iPacketsTransmitPerSecondMax, iPacketsTransmitPerSecondAvg);
	VOS_TaskUnlock();
	vty_out(vty," ----------------------------------------------------------------------\r\n");

	return (CMD_SUCCESS);
}
#endif

/* B--added by liwei056@2011-3-4 for OnuBw's FailedFor-1007 */
extern int g_iOnuBwSetFirstFailed;
extern int g_iOnuBwSetSecondFailed;
extern int g_iOnuBwSetFailedDelay;

DEFUN(
	debug_onubw_failed,
	debug_onubw_failed_cmd,
	"onu-bw-failed {reset}*1",
	"trace onu's bw failed times\n"
	"reset onu's bw failed times\n"
)
{
    if ( 0 == argc )
    {
    	vty_out(vty,"  FailedDelay(%dms), FirstUplinkBWFailedTimes(%d), SecondUplinkFailedTimes(%d).\r\n", 
    				g_iOnuBwSetFailedDelay, g_iOnuBwSetFirstFailed,g_iOnuBwSetSecondFailed);
    }
    else
    {
        g_iOnuBwSetFirstFailed  = 0;
        g_iOnuBwSetSecondFailed = 0;
    }

	return (CMD_SUCCESS);
}

DEFUN(
	set_onubw_faildelay,
	set_onubw_faildelay_cmd,
	"onu-bw-failed delay <0-1000>",
	"trace onu's bw failed times\n"
	"set onu's bw re-set delaytime after firstly failed\n"
)
{
    g_iOnuBwSetFailedDelay = VOS_AtoI(argv[0]);

	return (CMD_SUCCESS);
}
/* E--added by liwei056@2011-3-4 for OnuBw's FailedFor-1007 */


/* B--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */
DEFUN(
	test_onu_abnormal_report,
	test_onu_abnormal_report_cmd,
	"test onu reg-abnormal-report <0-1000>",
	"test funcs\n"
	"test onu funcs\n"
	"test onu's register abnormal-report funcs\n"
	"test times\n"
)
{
    TestAbnormalOnuReport(VOS_AtoI(argv[0]));

	return (CMD_SUCCESS);
}
/* E--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */

/* B--added by liwei056@2013-5-10 for [813 ONU OAM HearBeat's Clear] */
DEFUN(
	onu_oamrate_set,
	onu_oamrate_set_cmd, 
	"onu oam-rate-limit <slot/port/onuid> {[enable|diable]}*1", 
	"config onu oam's rate\n"
	"config onu oam's rate\n"
	"input the slot/port/onuid\n"
	"open the oam's rate-limiter\n"
	"close the oam's rate-limiter\n"
	)
{
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, OnuIdx /* llid */;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}

	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

#if 0
	llid = GetLlidByOnuIdx(phyPonId, (ulOnuid-1));
	if( llid == INVALID_LLID ) 
		{
		vty_out(vty, " %% onu %d/%d/%d LLID err\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}
#endif

    if ( 1 == argc )
    {
        bool enable_val;
    
        if ( 0 == (lRet = GetOnuOamSlowProtocolLimit(phyPonId, OnuIdx, &enable_val)) )
        {
    		vty_out(vty, " onu %d/%d/%d's oam-rate-limit is %s\r\n", ulSlot, ulPort, ulOnuid, (enable_val) ? "enabled" : "disabled");
        }
        else
        {
    		vty_out(vty, " onu %d/%d/%d is failed(%d) to get\r\n", ulSlot, ulPort, ulOnuid, lRet);
    		return(CMD_WARNING);
        }
    }
    else
    {
        if ( 'e' == argv[1][0] )
        {
            lRet = EnableOnuOamSlowProtocolLimit(phyPonId, OnuIdx);
        }
        else
        {
            lRet = DisableOnuOamSlowProtocolLimit(phyPonId, OnuIdx);
        }
        
        if ( 0 != lRet )
        {
    		vty_out(vty, " onu %d/%d/%d is failed(%d) to set\r\n", ulSlot, ulPort, ulOnuid, lRet);
    		return(CMD_WARNING);
        }
    }

	return (CMD_SUCCESS);
}
/* E--added by liwei056@2013-5-10 for [813 ONU OAM HearBeat's Clear] */

/* B--added by liwei056@2013-7-5 for [TkOlt&TkOnu's 2.5G DownlinkBW's test] */
DEFUN(
	olt_2g_set,
	olt_2g_set_cmd, 
	"olt 2g-downlink-bw <slot/port> {[enable|disable]}*1", 
	"config olt 2g's rate\n"
	"config olt 2g's rate\n"
	"input the slot/port\n"
	"open the 2g's rate-support\n"
	"close the 2g's rate-limiter\n"
	)
{
	ULONG   ulSlot, ulPort;
	int iRlt;
	short int phyPonId;

	IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );

	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}

	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

    if ( 1 == argc )
    {
        int enable_val;

        if ( 0 == (iRlt = OLT_GetOnuB2PMode(phyPonId, &enable_val)) )
        {
    		vty_out(vty, " olt %d/%d's 2g-downlink-bw is %s\r\n", ulSlot, ulPort, (enable_val) ? "enabled" : "disabled");
        }
        else
        {
    		vty_out(vty, " olt %d/%d is failed(%d) to get\r\n", ulSlot, ulPort, iRlt);
    		return(CMD_WARNING);
        }
    }
    else
    {
        if ( 'e' == argv[1][0] )
        {
            iRlt = OLT_SetOnuB2PMode(phyPonId, TRUE);
        }
        else
        {
            iRlt = OLT_SetOnuB2PMode(phyPonId, FALSE);
        }
        
        if ( 0 != iRlt )
        {
    		vty_out(vty, " olt %d/%d is failed(%d) to set\r\n", ulSlot, ulPort, iRlt);
    		return(CMD_WARNING);
        }
    }

	return (CMD_SUCCESS);
}


DEFUN(
	onu_2g_set,
	onu_2g_set_cmd, 
	"onu 2g-downlink-bw <slot/port/onuid> {[enable|disable]}*1", 
	"config onu 2g's rate\n"
	"config onu 2g's rate\n"
	"input the slot/port/onuid\n"
	"open the 2g's rate-support\n"
	"close the 2g's rate-limiter\n"
	)
{
	ULONG   ulSlot, ulPort, ulOnuid;
	LONG lRet;
	short int phyPonId, OnuIdx /* llid */;

	lRet = PON_ParseSlotPortOnu( argv[0], &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
    		return CMD_WARNING;
	
	if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
		return CMD_WARNING;

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

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
	    	return CMD_WARNING;
		}

	if( PonPortIsWorking(phyPonId) != TRUE )
		{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	if ((ulOnuid<1) || (ulOnuid>MAXONUPERPON))
		{
		vty_out( vty, "  %% onuid error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

    OnuIdx = ulOnuid - 1;
	if( GetOnuOperStatus( phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuid );
		return( CMD_WARNING);
		}

    if ( 1 == argc )
    {
        int enable_val;

        if ( 0 == (lRet = OnuMgt_GetOnuB2PMode(phyPonId, OnuIdx, &enable_val)) )
        {
    		vty_out(vty, " onu %d/%d/%d's 2g-downlink-bw is %s\r\n", ulSlot, ulPort, ulOnuid, (enable_val) ? "enabled" : "disabled");
        }
        else
        {
    		vty_out(vty, " onu %d/%d/%d is failed(%d) to get\r\n", ulSlot, ulPort, ulOnuid, lRet);
    		return(CMD_WARNING);
        }
    }
    else
    {
        if ( 'e' == argv[1][0] )
        {
            lRet = OnuMgt_SetOnuB2PMode(phyPonId, OnuIdx, TRUE);
        }
        else
        {
            lRet = OnuMgt_SetOnuB2PMode(phyPonId, OnuIdx, FALSE);
        }
        
        if ( 0 != lRet )
        {
    		vty_out(vty, " onu %d/%d/%d is failed(%d) to set\r\n", ulSlot, ulPort, ulOnuid, lRet);
    		return(CMD_WARNING);
        }
    }

	return (CMD_SUCCESS);
}
/* E--added by liwei056@2013-7-5 for [TkOlt&TkOnu's 2.5G DownlinkBW's test] */


/* added by xieshl 20120512, 针对青岛长宽和上海鹏博士发生的部分ONU不注册的问题，怀疑
    跟PON底层通信有关，增加统计查询命令*/
extern ULONG pon_comm_rx_over_size_pkts;
extern ULONG pon_comm_rx_que_over_drop_pkts;
extern ULONG pon_comm_rx_task_busy_drop_pkts;
extern ULONG pon_comm_rx_total_pkts;
extern ULONG pon_comm_rx_error_pkts;
DEFUN(
	show_pon_comm_statistic,
	show_pon_comm_statistic_cmd,
	"show pon-comm",
	"show pon low level comm statistic\n"
	"show pon low level comm statistic\n"
)
{
	vty_out( vty, "recv-total-pkts:%d\r\n", pon_comm_rx_total_pkts );
	vty_out( vty, "over-size-pkts:%d\r\n", pon_comm_rx_over_size_pkts );
	vty_out( vty, "que-over-drop:%d\r\n", pon_comm_rx_que_over_drop_pkts );
	vty_out( vty, "task-busy-drop:%d\r\n", pon_comm_rx_task_busy_drop_pkts );
	vty_out( vty, "emac1-error:%d\r\n", pon_comm_rx_error_pkts );
	return (CMD_SUCCESS);
}

DEFUN(
	clear_pon_comm_statistic,
	clear_pon_comm_statistic_cmd,
	"clear pon-comm",
	"clear pon low level comm statistic\n"
	"clear pon low level comm statistic\n"
)
{
	pon_comm_rx_over_size_pkts = 0;
	pon_comm_rx_que_over_drop_pkts = 0;
	pon_comm_rx_task_busy_drop_pkts = 0;
	pon_comm_rx_total_pkts = 0;
	pon_comm_rx_error_pkts = 0;
	return (CMD_SUCCESS);
}

/*B--added by liyang@2015-4-16 for syslog olt&onu filter */
DEFUN(
	config_syslog_pon_soft_filter,
	config_syslog_pon_soft_filter_cmd, 
	"config syslog pon-soft  [general|<slot/port>]  [general|<1-1024>]", 
	DescStringCommonConfig
    "Config syslog's setting\n"
	"Config pon-soft syslog\n"
	"Enable irrelevant to pon port syslog\n"
	"Enable one pon port syslog\n"
	"Enable irrelevant to llid syslog\n"
	"Enable one llid syslog\n"
	)
{
	ULONG   ulSlot, ulPort;
	LONG lRet;
	short int phyPonId;

	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	{
		if(argv[0][0] == 'g')
		{
			g_sLogPonSoftOltID = -1;/*irrelevant to pon port*/
		}
		else
		{
			lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
			if( lRet != VOS_OK )
				return CMD_WARNING;
		

			if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
					return CMD_WARNING;

			
			phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
			if (phyPonId == VOS_ERROR)
			{
				vty_out( vty, "  %% Parameter is error.\r\n" );
			    	return CMD_WARNING;
			}

			g_sLogPonSoftOltID = phyPonId;
		}

		if(argv[1][0] == 'g')
		{
			g_sLogPonSoftLLID = -1;/*irrelevant to llid/onu*/
		}
		else
		{
			g_sLogPonSoftLLID = VOS_AtoL(argv[1]);
		}
	}

	return (CMD_SUCCESS);
}

DEFUN(
	config_syslog_pon_soft_switch,
	config_syslog_pon_soft_switch_cmd, 
	"config syslog pon-soft [on|off]", 
	DescStringCommonConfig
	"Config syslog's setting\n"
	"Config pon-soft syslog\n"
	"Turn on pon-soft syslog\n"
	"Turn off pon-soft syslog\n"
	)
{
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	{
		if(!VOS_StrCmp(argv[0], "on"))
		{
			g_ucLogPonSoftEnable = TRUE;
		}
		else
		{
			g_ucLogPonSoftEnable = FALSE;
		}

	}
	return CMD_SUCCESS;
}


/*E--added by liyang@2015-4-16 for syslog olt&onu filter */

DEFUN(
	show_onu_tk_onu_table,
	show_onu_tk_onu_table_cmd, 
	"show onu-tk-table <slot/port> [all|<1-128>]", 
	"show onu tk tabel\n"
	"show onu record table\n"
	"input slot and port\n"
	"indicate all onus\n"
	"input one onu id\n"
	
	)
{

	int olt_id, onu_id;
	ULONG   ulSlot, ulPort;
	LONG lRet;
	short int phyPonId;
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	{
		
		lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			return CMD_WARNING;
	

		if(PonCardSlotPortCheckByVty(ulSlot, ulPort,vty)!=ROK)
				return CMD_WARNING;

		
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
		    	return CMD_WARNING;
		}

		olt_id = phyPonId;
		
		
		if(argv[1][0] == 'a')
		{
			onu_id = 129;
		}
		else
		{
			onu_id = VOS_AtoL(argv[1]);
		}

		ShowTkOnuTable(olt_id, onu_id,vty);
	}

	
	return (CMD_SUCCESS);
}

LONG EPON_DEBUG_CommandInstall()
{
    install_element ( CONFIG_NODE, &make_test_on_config_cmd );
    /*install_element ( DEBUG_HIDDEN_NODE, &make_test_off_config_cmd );*/
    install_element ( CONFIG_NODE, &make_test_status_show_cmd );

	/* modified by chenfj 2009-2-27
		将这个命令移到config 节点下;并增加激活pending ONU 的命令
	*/
    install_element ( CONFIG_NODE, &pending_onu_list_show_cmd ); 
    install_element ( CONFIG_NODE, &active_pending_onu_cmd);
	install_element ( CONFIG_NODE, &active_Gpon_pending_onu_cmd);
    install_element ( VIEW_NODE, &pending_onu_list_show_cmd ); 
	install_element ( PON_PORT_NODE, &active_Gponpon_pending_onu_cmd);
	
    /*install_element ( DEBUG_HIDDEN_NODE, &add_onu_pending_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &del_onu_pending_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &del_pon_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &add_pon_config_cmd );*/
    
	/*added by wutw 2006/12/27*/
	/* 问题单3752: 建议调整到pon节点下*/
	install_element ( CONFIG_NODE, &onu_llid_mapping_cmd);
	install_element ( PON_PORT_NODE, &onu_llid_mapping_pon_cmd);	
	install_element ( VIEW_NODE, &onu_llid_mapping_cmd);

	/* added by chenfj 2007-7-16 */
	install_element ( DEBUG_HIDDEN_NODE, &uplink_bandwidth_BataRatio_set_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &uplink_bandwidth_BataRatio_show_cmd );

	/* added by chenfj 2007-9-17, ONU emapper 参数显示、设置*/
	/*
	install_element ( DEBUG_HIDDEN_NODE, &onu_emapper_set1_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_emapper_set2_cmd );
	*/
	install_element ( DEBUG_HIDDEN_NODE, &onu_emapper_set_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_emapper_show_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_emapper_list_show_cmd );
    
	install_element ( DEBUG_HIDDEN_NODE, &onu_eeprom_set_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_eeprom1_set_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_eeprom_show_cmd );

	install_element ( CONFIG_NODE, &event_debug_delete_authentry_cmd );
	install_element ( CONFIG_NODE, &event_debug_associate_share_conf_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &event_debug_off_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &event_debug_on_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &event_flag_show_cmd);

    install_element ( DEBUG_HIDDEN_NODE, &onu_oamrate_set_cmd);
    install_element ( DEBUG_HIDDEN_NODE, &olt_2g_set_cmd);
    install_element ( DEBUG_HIDDEN_NODE, &onu_2g_set_cmd);

    /* B--added by liwei056@2011-11-29 for D14051 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    /* E--added by liwei056@2011-11-29 for D14051 */
    {
   /* install_element ( DEBUG_HIDDEN_NODE, &read_port_statistic_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &write_register_func_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &read_register_func_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &show_olt_entity_infomation_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &show_pon_entity_infomation_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &show_onu_entity_infomation_cmd );
    */
	/*at by wutw at 14 October*/
    install_element ( DEBUG_HIDDEN_NODE, &oam_debug_off_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &oam_debug_on_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &oam_debug_show_cmd ); 
    install_element ( DEBUG_HIDDEN_NODE, &phy_debug_on_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &phy_debug_off_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &msg_debug_on_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &msg_debug_off_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &oam_msg_debug_on_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &oam_msg_debug_off_config_cmd );

    install_element ( DEBUG_HIDDEN_NODE, &oam_comm_debug_off_config_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &oam_comm_debug_on_config_cmd );

	install_element ( DEBUG_HIDDEN_NODE, &oam_debug_err_config_cmd); 
	install_element ( DEBUG_HIDDEN_NODE, &oam_debug_track_config_cmd); 
	install_element ( DEBUG_HIDDEN_NODE, &oam_debug_err_off_config_cmd); 
	install_element ( DEBUG_HIDDEN_NODE, &oam_debug_track_off_config_cmd); 
	install_element ( DEBUG_HIDDEN_NODE, &oam_Queue_show_cmd); 
	
	install_element ( DEBUG_HIDDEN_NODE, &oam_stat_show_cmd); 
	install_element ( DEBUG_HIDDEN_NODE, &oam_stat_clear_cmd); 

/*	install_element ( DEBUG_HIDDEN_NODE, &debug_onu_stat_cmd); */
	/*install_element ( DEBUG_HIDDEN_NODE, &debug_olt_stat_cmd);*/
	install_element ( DEBUG_HIDDEN_NODE, &oam_frame_Queue_clear_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &oam_Task_Queue_show_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &oam_Task_Queue_clear_cmd);
	/*
	install_element ( DEBUG_HIDDEN_NODE, &pon_oam_send_test_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &pon_oam_rev_test_cmd);	
	*/
	install_element ( DEBUG_HIDDEN_NODE, &oam_testing_show_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &oam_callback_show_cmd);

    /* add for onu file transfer debug flag */
	install_element ( DEBUG_HIDDEN_NODE, &debug_oam_transfile_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &no_debug_oam_transfile_cmd);


    install_element ( DEBUG_HIDDEN_NODE, &pon_msg_count_show_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &pon_msg_count_clear_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &pon_mpcp_register_show_cmd); 

	/* added by chenfj 2007-7-3 */
	install_element ( DEBUG_HIDDEN_NODE, &set_pon_cni_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_pon_cni_cmd);

    
	install_element ( DEBUG_HIDDEN_NODE, &onu_timesync_gap_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &onu_timesync_gap_show_cmd );

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
	install_element( DEBUG_HIDDEN_NODE, &ctc_save_config_debug_cmd);
	install_element( DEBUG_HIDDEN_NODE, &undo_ctc_save_config_debug_cmd);
#endif
	install_element( DEBUG_HIDDEN_NODE, &eth_loop_debug_cmd);
	install_element( DEBUG_HIDDEN_NODE, &undo_eth_loop_debug_cmd);

	install_element ( DEBUG_HIDDEN_NODE, &set_pon_discard_unlearned_addr_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_pon_discard_unlearned_addr_cmd );

	/*install_element ( DEBUG_HIDDEN_NODE, &ftpc_download_ftp_phenixos_master_for_all_pons_cmd );*/

	install_element ( DEBUG_HIDDEN_NODE, &onu_pas_pq_occupancy_cmd );
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	install_element( DEBUG_HIDDEN_NODE, &debug_tdm_api_func_cmd);
	install_element( DEBUG_HIDDEN_NODE, &no_debug_tdm_api_func_cmd);
	install_element( DEBUG_HIDDEN_NODE, &debug_ETH_port_isolate_cmd );
	install_element( DEBUG_HIDDEN_NODE, &undo_debug_ETH_port_isolate_cmd );
#endif

	install_element( DEBUG_HIDDEN_NODE, &test_pon_ext_buf_cmd );

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/*begin
	added by wangxiaoyu 2008-01-24
	end*/
	install_element( DEBUG_HIDDEN_NODE, &tdm_comm_queue_show_cmd);
#endif

	/* added by zhangxinhui 2009-02-02
	** display the control channel packets statistics
	
	install_element( DEBUG_HIDDEN_NODE, &show_ctrlchanne_stat_cmd );
	*/
	install_element( DEBUG_HIDDEN_NODE, &restart_sw_ctrlchanne_cmd );

	install_element( DEBUG_HIDDEN_NODE, &onu_encrypt_update_flag_cmd );
    
    /* B--added by liwei056@2011-3-4 for OnuBw's FailedFor-1007 */
	install_element( DEBUG_HIDDEN_NODE, &debug_onubw_failed_cmd );
	install_element( DEBUG_HIDDEN_NODE, &set_onubw_faildelay_cmd );
    /* E--added by liwei056@2011-3-4 for OnuBw's FailedFor-1007 */

    /* B--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */
	install_element( DEBUG_HIDDEN_NODE, &test_onu_abnormal_report_cmd );
    /* E--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */

	/*B--added by liyang@2015-4-16 for syslog olt&onu filter */
	install_element( DEBUG_HIDDEN_NODE, &config_syslog_pon_soft_filter_cmd );
	install_element( DEBUG_HIDDEN_NODE, &config_syslog_pon_soft_switch_cmd );
	/*E--added by liyang@2015-4-16 for syslog olt&onu filter */

    }

    /* added by xieshl 20120512, 针对现场发生了多起部分ONU不注册的问题，怀疑跟PON底层通信有关，
        增加统计查询命令*/
    if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        install_element( DEBUG_HIDDEN_NODE, &show_pon_comm_statistic_cmd );
        install_element( DEBUG_HIDDEN_NODE, &clear_pon_comm_statistic_cmd );
    }

	install_element(DEBUG_HIDDEN_NODE, &show_onu_tk_onu_table_cmd);

    return VOS_OK;
}


#ifdef	__cplusplus
}
#endif/* __cplusplus */
