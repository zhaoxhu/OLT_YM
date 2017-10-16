/**************************************************************
*
*   OltGeneral.h -- Olt management high level Application functions General header
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ---------|--- -------|---------------------|------------
*	1.00	  | 15/04/2006 | Creation				| chen fj
*
***************************************************************/

#ifndef _OLTGERNRAL_H
#define  _OLTGERNRAL_H

/*
#include "PON_expo.h"
*/
/*#include "OSSRV_expo.h"*/
/*#include "PAS_expo.h" */

#define  TBC_ADDR  0xC000B0
/*#define  PAS_SOFT_V4162 */
#define  V2R1_PRODUCT_TRAP

#define  STATISTIC_TASK_MODULE

#define  ONU_VERSION_COMPARE 

#define  UPDATE_ONU_IMAGE_BY_OAM 

#define  ONU_PEER_TO_PEER  /* 支持ONU peer-to-peer 通信*/

/*#define  PON_ENCRYPTION_HANDLER */

#define  V2R1_WATCH_DOG

#define  V2R1_SWITCH_5325E

#define  V2R1_PAS_SOFT_V4 

#define  V2R1_PAS_SOFT_V5

/*#define  TDM_VOICE_SERVICE*/

#define  ONU_PPPOE_RELAY

#define  ONU_DHCP_RELAY
/*#define  GONGJIN_VERSION*/


/*  added by chenfj 2007-02-07
     增加ONU自动配置*/
/* #define  V2R1_ONU_AUTO_CONFIG  */

#define EPON_MODULE_YES	1
#define EPON_MODULE_NO	0

#define EPON_MODULE_ENVIRONMENT_MONITOR	    EPON_MODULE_YES	/* added by xieshl 20090204, OLT温度和风扇监控 */
#define EPON_MODULE_ONU_AUTO_LOAD			EPON_MODULE_YES	/* added by xieshl 20090205, ONU自动升级和配置 */
#define EPON_MODULE_MASTER_FILE_SYNC		EPON_MODULE_YES	/* added by xieshl 20090302, 主备文件同步 */
#define EPON_MODULE_POWEROFF_INT_ISR		EPON_MODULE_YES	/* added by jianght 20090515, 电源板卡掉电中断ISR */
#define EPON_MODULE_SYS_DIAGNOSE			EPON_MODULE_YES	/* added by xieshl 20090701, 主要针对5001 PON上行业务检查，断时自动恢复 */
#define EPON_MODULE_EPON_5325E_MDIO			EPON_MODULE_NO		/* added by xieshl 20090904, GFA-EPON板bcm5325e端口隔离 */
#define EPON_MODULE_PON_LLID_VLAN			EPON_MODULE_YES
#define      EPON_SUBMODULE_PON_LLID_VLAN_CLI_CFG_COVER	   EPON_MODULE_YES /* added by liwei056@2014-7-17 for D10708 */
#define EPON_MODULE_PON_SFP_CHECK			EPON_MODULE_YES
#define EPON_MODULE_PON_OPTICAL_POWER		EPON_MODULE_YES
#define EPON_MODULE_QOS_ACL_QINQ_MIB		EPON_MODULE_YES
#define EPON_MODULE_OLT_QINQ_MIB            EPON_MODULE_YES     /* added by liwei056@2014-3-26 for 四川广电的MIB需求 */  
#define EPON_MODULE_ONU_REG_FILTER			EPON_MODULE_NO
#define EPON_MODULE_PON_ABNORMAL_HANDLE	    EPON_MODULE_NO		/* added by xieshl 20100805 */
#define EPON_MODULE_ONU_LOOP				EPON_MODULE_NO
#define EPON_MODULE_ONU_MAC_OVERFLOW_CHECK  EPON_MODULE_YES	
#define EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT	EPON_MODULE_NO	    /* added by liwei056@2012-8-21 */
#define EPON_MODULE_TDM_SERVICE				EPON_MODULE_NO		/* added by xieshl 20120821, 支持GT861，但不再支持TDM管理 */
#define EPON_MODULE_ONU_EXT_BOARD			EPON_MODULE_YES  	/* added by xieshl 20120821, 支持GT861 */
#define EPON_MODULE_USER_TRACE				EPON_MODULE_YES	/* added by xieshl 20121219, 支持基于用户MAC地址或用户认证账号的用户定位功能 */

#define EPON_MODULE_DOCSIS_MANAGE           EPON_MODULE_NO    /* added by liwei056@2013-1-28 for DOCSIS业务支持 */  
#define EPON_MODULE_DOCSIS_MANAGE_MIB       EPON_MODULE_NO    /* added by liwei056@2013-4-25 for DOCSIS业务支持 */  
#define EPON_MODULE_DOCSIS_PROFILE          EPON_MODULE_NO    /* added by liwei056@2013-5-15 for DOCSIS业务支持 */  


#include "syscfg.h"
#include "vos.h"
#include "vos_base.h"
#include "vos_task.h"
#include "vos_que.h"
#include "vos_sem.h"
#include "vos_mem.h"
#include "vos_time.h"
#include "vos_string.h"
#include "vos_io.h"
#include "vos_timer.h"
#include "sys/console/sys_console.h"
#include "cli/cli.h"
#include "sys/main/sys_main.h"
#include "device_hardware.h"

#include "V2R1General.h"
#include "V2R1_product.h"
#include "includeFromPas.h"
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#include "includeFromPas8411.h"
#include "includeFromTk.h"
#include "includeFromCmc.h"
#include "includeFromBcm.h"
#if defined(_GPON_BCM_SUPPORT_)  
#include "includeFromGpon.h"
#endif
#include "PON_CTC_STACK_expo.h"
#include "OnuPmcRemoteManagement.h"
#include "OnuPmcApi.h"
#include "event/eventMain.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "TdmService.h"
#endif


#define EPON_MODULE_PON_REMOTE_MANAGE       RPU_MODULE_LOGICAL_SLOT 

#define EPON_MODULE_PON_PORT_HOT_SWAP       EPON_MODULE_YES

/* B--added by liwei056@2012-3-6 for 国电ONU倒换功能 */
#define EPON_MODULE_ONU_HOT_SWAP            EPON_MODULE_YES
/* E--added by liwei056@2012-3-6 for 国电ONU倒换功能 */

/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
#define   EPON_SUBMODULE_PON_FAST_SWAP      EPON_MODULE_YES
#define   EPON_SUBMODULE_PON_FAST_SWAP_V1   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_FAST_SWAP_V2   EPON_MODULE_YES
#define   EPON_SUBMODULE_PON_SWAP_MONITOR   EPON_MODULE_YES
#define   EPON_SUBMODULE_PON_DEVICE_SWAP    EPON_MODULE_PON_REMOTE_MANAGE 
#define   EPON_SUBMODULE_PON_ETH_SWAP       EPON_MODULE_NO 
/* B--added by liwei056@2012-7-4 for 国电ONU倒换功能 */
#if ( EPON_MODULE_ONU_HOT_SWAP == EPON_MODULE_YES )
#define   EPON_SUBMODULE_ONU_OPTIC_SWAP     EPON_MODULE_YES
#else
#define   EPON_SUBMODULE_ONU_OPTIC_SWAP     EPON_MODULE_NO
#endif
/* E--added by liwei056@2012-7-4 for 国电ONU倒换功能 */

/* B--added by liwei056@2012-2-28 for 国电新倒换触发条件 */
#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER   EPON_MODULE_YES
#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR   EPON_MODULE_YES
/* E--added by liwei056@2012-2-28 for 国电新倒换触发条件 */
/* B--added by liwei056@2012-3-7 for 国电上联绑定PON倒换功能 */
#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN   EPON_MODULE_YES
/* E--added by liwei056@2012-3-7 for 国电上联绑定PON倒换功能 */
#else
#define   EPON_SUBMODULE_PON_FAST_SWAP      EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_FAST_SWAP_V1   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_FAST_SWAP_V2   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_SWAP_MONITOR   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_DEVICE_SWAP    EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_ETH_SWAP       EPON_MODULE_NO 
#define   EPON_SUBMODULE_ONU_OPTIC_SWAP     EPON_MODULE_NO

#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR   EPON_MODULE_NO
#define   EPON_SUBMODULE_PON_SWAP_TRIGGER_UPLINKDOWN   EPON_MODULE_NO
#endif


#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
#define EPON_PON_SWAP_PORTRANGE "<gslot/gport>"
#else
#define EPON_PON_SWAP_PORTRANGE "<slot/port>"
#endif

#if ( EPON_SUBMODULE_PON_ETH_SWAP == EPON_MODULE_YES )
#define EPON_PON_SWAP_PORTTYPE ""
#else
#define EPON_PON_SWAP_PORTTYPE "pon "
#endif

#define EPON_PON_SWAP_MODE_OLT_SLOWLY_STR    "slowness"
#define EPON_PON_SWAP_MODE_OLT_QUICKLY_STR   "sensitive"
#define EPON_PON_SWAP_MODE_ONU_OPTIC_STR     "onu-optic"

#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
#define EPON_PON_SWAP_MODE_OLT      EPON_PON_SWAP_MODE_OLT_QUICKLY_STR"|"EPON_PON_SWAP_MODE_OLT_SLOWLY_STR
#define EPON_PON_SWAP_MODE_OLT_DESC	"the switchhover is sensitive and real-time\n""the switchhover is slow and compatible\n"
#else
#define EPON_PON_SWAP_MODE_OLT      EPON_PON_SWAP_MODE_OLT_SLOWLY_STR
#define EPON_PON_SWAP_MODE_OLT_DESC	"the switchhover is slow and compatible\n"
#endif

#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
#define EPON_PON_SWAP_MODE_ONU      EPON_PON_SWAP_MODE_ONU_OPTIC_STR
#define EPON_PON_SWAP_MODE_ONU_DESC	"the switchhover is only for onu-optic protected\n"

#define EPON_PON_SWAP_MODE          EPON_PON_SWAP_MODE_OLT"|"EPON_PON_SWAP_MODE_ONU
#else
#define EPON_PON_SWAP_MODE_ONU      ""
#define EPON_PON_SWAP_MODE_ONU_DESC	""

#define EPON_PON_SWAP_MODE          EPON_PON_SWAP_MODE_OLT
#endif
#define EPON_PON_SWAP_MODE_DESC     EPON_PON_SWAP_MODE_OLT_DESC""EPON_PON_SWAP_MODE_ONU_DESC
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */

#define PON_BOARD_ENSURE \
	if ( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER ) \
	{ \
		VOS_ASSERT(0); \
		break; \
	}
/*Begin:for onu swap by jinhl@2013-04-27*/
#define PON_TOP_MANAGER_ENSURE \
	if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER ) \
	{ \
		VOS_ASSERT(0); \
		break; \
	}
/*End:for onu swap by jinhl@2013-04-27*/
#ifndef ERROR
#define ERROR ( -1 )
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE( x ) (sizeof(x)/sizeof((x)[0]))
#endif

#define ONU_TRANSMISSION_ENABLE 1
#define ONU_TRANSMISSION_DISABLE 0

#if 0
extern STATUS AlarmDispatch (
					unsigned long *AlarmSource, 
					unsigned int Len, 
					unsigned int TrapId, 
					unsigned int AlarmClass, 
					char *AlarmDec);
#endif

extern unsigned long  getPonChipInserted( unsigned char slotno, unsigned char portno );
extern unsigned long  getPonChipType(unsigned char slotno, unsigned char portno );
extern unsigned long  getPonCtcStackSupported( unsigned char slotno,  unsigned char ponno);
extern unsigned long  getCtcStackSupported();

#ifdef V2R1_WATCH_DOG
/*
extern void write_gpio(ulong_t GPIOx, uchar_t bit);
extern int read_gpio(int GPIOx);
*/
extern int V2R1_watchdog_init_flag;
extern int V2R1_watchdog_startup_flag;	/* added by xieshl 20090123 */
extern int OnuTransmission_flag; /*added by luh 2011-9-24*/

extern  void  V2R1_enable_watchdog(void);
extern  void  V2R1_disable_watchdog(void);
extern  void  V2R1_disable_watchdog_forced(void);
extern  void  V2R1_feed_watchdog(void);
extern  void  V2R1_init_watchdog(void);
#endif

#ifdef  V2R1_SWITCH_5325E
extern int BrdChannelBcm5325eInit(void);
extern LONG BCM5325E_DEBUG_CommandInstall(void);
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE( x ) (sizeof(x)/sizeof((x)[0]))
#endif

typedef enum
{ 
	ONU_GETDATA_TIMER = 0,
	ONU_WAKEUP_TIMER,
	ONU_PORTSTATS_TASKSTATUS,
	ONU_PORTSTATS_ENABLE
}ONU_PORTSTATS_TIMER_NAME_E;

/* B--added by liwei056@2010-6-1 for 6900-Prj */
/* 注意:顺序与数目必须与OltMgmtIFs一一对应 */
typedef enum{
#if 1
/* -------------------OLT基本API------------------- */
    OLT_CMD_SPLIT_LINE_FRAME_BEGIN = 0,
    
    OLT_CMD_OLT_IS_EXIST_1,
    OLT_CMD_GET_CHIPTYPEID_2,
    OLT_CMD_GET_CHIPTYPENAME_3,
    OLT_CMD_RESET_PON_4,
    OLT_CMD_REMOVE_OLT_5,

    OLT_CMD_COPY_OLT_6,
    OLT_CMD_IS_SUPPORT_CMD_7,
    OLT_CMD_SET_PON_DEBUG_MODE_8,
    OLT_CMD_SET_INIT_PARAMS_9,
    OLT_CMD_SET_SYSTEM_PARAMS_10,
    
    OLT_CMD_SET_PON_I2C_EXTINFO_11,
    OLT_CMD_GET_PON_I2C_EXTINFO_12,
    OLT_CMD_SET_CARD_I2C_INFO_13,
    OLT_CMD_GET_CARD_I2C_INFO_14,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_WRITE_MDIO_REGISTER_15,
    
    OLT_CMD_READ_MDIO_REGISTER_16,
    OLT_CMD_READ_I2C_REGISTER_17,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_GPIO_ACCESS_18,
    OLT_CMD_READ_GPIO_19,
    OLT_CMD_WRITE_GPIO_20,
    
    OLT_CMD_SEND_CHIP_CLI_21,
    OLT_CMD_SET_DEVICE_NAME_22,
    OLT_CMD_RESET_PON_CHIP_23,

    OLT_CMD_SPLIT_LINE_FRAME_END,
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    OLT_CMD_SPLIT_LINE_PON_FUN_BEGIN = 100,
    
    OLT_CMD_GET_VERSION_1,
    OLT_CMD_GET_DBA_VERSION_2,
    OLT_CMD_CHK_VERSION_3,
    OLT_CMD_CHK_DBA_VERSION_4,
    OLT_CMD_GET_CNI_LINK_STATUS_5,
    
    OLT_CMD_GET_PON_WORKSTATUS_6,
    OLT_CMD_SET_ADMIN_STATUS_7,
    OLT_CMD_GET_ADMIN_STATUS_8,
    OLT_CMD_SET_VLAN_TPID_9,
    OLT_CMD_SET_VLAN_QINQ_10,
    
    OLT_CMD_SET_FRAME_SIZELIMIT_11,
    OLT_CMD_GET_FRAME_SIZELIMIT_12,
    OLT_CMD_OAM_IS_LIMIT_13,
    OLT_CMD_UPDATE_PON_PARAMS_14,
    OLT_CMD_SET_PPPOE_RELAY_MODE_15,

    OLT_CMD_SET_PPPOE_RELAY_PARAMS_16,
    OLT_CMD_SET_DHCP_RELAY_MODE_17,
    OLT_CMD_SET_IGMP_AUTH_MODE_18,
    OLT_CMD_SEND_FRAME_2_PON_19,
    OLT_CMD_SEND_FRAME_2_CNI_20,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_GET_VID_DOWNLINK_MODE_21,
    OLT_CMD_DEL_VID_DOWNLINK_MODE_22,
    OLT_CMD_GET_OLT_PARAMETERS_23,
    OLT_CMD_SET_OLT_IGMP_SNOOPING_MODE_24,
    OLT_CMD_GET_OLT_IGMP_SNOOPING_MODE_25,
    
    OLT_CMD_SET_OLT_MLD_FORWARDING_MODE_26,
    OLT_CMD_GET_OLT_MLD_FORWARDING_MODE_27,
    OLT_CMD_SET_DBA_REPORT_FORMAT_28,
    OLT_CMD_GET_DBA_REPORT_FORMAT_29,
    OLT_CMD_UPDATE_PROV_BWINFO_30,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    
    OLT_CMD_SPLIT_LINE_PON_FUN_END,
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    OLT_CMD_SPLIT_LINE_LLID_FUN_BEGIN = 200,
    
    OLT_CMD_LLID_IS_EXIST_1,
    OLT_CMD_LLID_DEREGISTER_2,
    OLT_CMD_GET_LLID_MAC_3,
    OLT_CMD_GET_LLID_REGINFO_4,
    OLT_CMD_AUTHORIZE_LLID_5,

    OLT_CMD_SET_LLID_SLA_6,
    OLT_CMD_GET_LLID_SLA_7,
    OLT_CMD_SET_LLID_POLICE_8,
    OLT_CMD_GET_LLID_POLICE_9,
    OLT_CMD_SET_LLID_DBA_TYPE_10,
    
    OLT_CMD_GET_LLID_DBA_TYPE_11,
    OLT_CMD_SET_LLID_DBA_FLAG_12,
    OLT_CMD_GET_LLID_DBA_FLAG_13,
    OLT_CMD_GET_LLID_HEART_OAM_14,
    OLT_CMD_SET_LLID_HEART_OAM_15,
    
    OLT_CMD_SPLIT_LINE_LLID_FUN_END,
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    OLT_CMD_SPLIT_LINE_ONU_FUN_BEGIN = 300,
    
    OLT_CMD_GET_ALL_ONUNUM_1,
    OLT_CMD_GET_ALL_ONU_2,
    OLT_CMD_CLEAR_ALL_ONU_3,
    OLT_CMD_RESUME_ALL_ONU_STATUS_4,
    OLT_CMD_SET_ALLONU_AUTH_MODE_5,
    
    OLT_CMD_SET_ONU_AUTH_MODE_6,
    OLT_CMD_SET_ONU_MAC_AUTH_7,
    OLT_CMD_SET_ALLONU_BIND_MODE_8,
    OLT_CMD_CHK_ONU_REG_CTRL_9,
    OLT_CMD_SET_ALLONU_DEFAULT_BW_10,
    
    OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_MODE_11,   
    OLT_CMD_SET_ONU_DOWNLINK_POLICE_MODE_12,   
    OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_PARAM_13,
    OLT_CMD_SET_ALLONU_UPLINK_DBA_PARAM_14,
    OLT_CMD_SET_ONU_DOWNLINK_PRI2COS_15,
    
    OLT_CMD_ACTIVE_PENDING_ONU_16,
    OLT_CMD_ACTIVE_ONE_PENDING_ONU_17,
    OLT_CMD_ACTIVE_CONFLICT_PENDING_ONU_18,
    OLT_CMD_ACTIVE_ONE_CONFLICT_PENDING_ONU_19,
    OLT_CMD_GET_PENDING_ONU_20,

    OLT_CMD_GET_UPDATING_ONU_21,	/* added by xieshl 20110110, 问题单11796 */
    OLT_CMD_GET_UPDATED_ONU_22,    
    OLT_CMD_GET_UPDATING_COUNTER_23,
    OLT_CMD_SET_UPDATE_ONU_MSG_24,
    OLT_CMD_GET_UPDATE_WAITING_25,
    
    OLT_CMD_SET_ALLONU_AUTH_MODE2_26,
    OLT_CMD_SET_ALLONU_BW_PARAM_27,
    OLT_CMD_SET_ONU_P2P_MODE_28,
    OLT_CMD_GET_ONU_B2P_MODE_29,
    OLT_CMD_SET_ONU_B2P_MODE_30,
    
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_GET_ONU_MODE_31,
    OLT_CMD_GET_MAC_ADDRESS_AUTHENTICATION_32,
    OLT_CMD_SET_AUTHORIZE_MAC_Aaccording_LIST_33,
    OLT_CMD_GET_AUTHORIZE_MAC_Aaccording_LIST_34,
    OLT_CMD_GET_DOWNLINK_BUFFER_CONFIG_35,

    OLT_CMD_GET_OAM_INFORMATION_36,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-02-22*/
    OLT_CMD_RESUME_LLID_STATUS_37,
    OLT_CMD_SEARCH_FREE_ONUIDX_38,
    OLT_CMD_GET_ONUIDX_BYMAC_39,
    /*End:for onu swap by jinhl@2013-02-22*/
    OLT_CMD_BROADCAST_CLICOMMAND_40,

    OLT_CMD_SET_AUTH_Entry_41,
    OLT_CMD_SET_ONU_DEFAULT_MAX_MAC_42,
	OLT_CMD_CTC_SET_PORT_STATS_TIME_OUT_43,
    OLT_CMD_SET_MAX_ONU_44,
    OLT_CMD_GET_ONU_CONFIG_DEL_STATUS_45,
    OLT_CMD_SET_ONU_TX_PORWER_SUPPLY_CONTROL_46,
    OLT_CMD_SET_GPON_AUTH_Entry_47,
    OLT_CMD_SPLIT_LINE_ONU_FUN_END,
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    OLT_CMD_SPLIT_LINE_ENCRYPT_FUN_BEGIN = 400,
    
    OLT_CMD_SET_ENCRYPT_MODE_1,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_SET_ENCRYPTION_PREAMBLE_MODE_2,
    OLT_CMD_GET_ENCRYPTION_PREAMBLE_MODE_3,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_GET_LLID_ENCRYPT_MODE_4,
    OLT_CMD_START_LLID_ENCRYPT_5,
    
    OLT_CMD_FINISH_LLID_ENCRYPT_6,
    OLT_CMD_STOP_LLID_ENCRYPT_7,
    OLT_CMD_SET_LLID_ENCRYPT_KEY_8,
    OLT_CMD_FINISH_LLID_ENCRYPT_KEY_9,

    OLT_CMD_SPLIT_LINE_ENCRYPT_FUN_END,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    OLT_CMD_SPLIT_LINE_ADDRTBL_FUN_BEGIN = 500,
    
    OLT_CMD_SET_MAC_AGINGTIME_1,
    OLT_CMD_SET_ADDR_TBL_CFG_2,
    OLT_CMD_GET_ADDR_TBL_CFG_3,
    OLT_CMD_GET_ADDR_TBL_4,
    OLT_CMD_ADD_ADDR_TBL_5,
    
    OLT_CMD_DEL_ADDR_TBL_6,
    OLT_CMD_REMOVE_MAC_7,
    OLT_CMD_RESET_ADDR_TBL_8,
    OLT_CMD_SET_ONU_MAC_THRESHOLD_9,
    OLT_CMD_SET_ONU_MAC_CHECK_ENABLE_10,
    
    OLT_CMD_SET_ONU_MAC_CHECK_PERIOD_11,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_SET_ADDRESS_TABLE_FULL_HANDLING_MODE_12,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    
    OLT_CMD_GET_LLID_BY_USER_MAC_ADDRESS_13,
    OLT_CMD_GET_ADDR_VLAN_TBL_14,/*for GPON by jinhl*/
    OLT_CMD_SPLIT_LINE_ADDRTBL_FUN_END,
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
    OLT_CMD_SPLIT_LINE_OPTICS_FUN_BEGIN = 600,
    
    OLT_CMD_GET_OPTICAL_CAPABILITY_1,
    OLT_CMD_GET_OPTICS_DETAIL_2,
    OLT_CMD_SET_MAX_RTT_3,
    OLT_CMD_SET_OPTICAL_TX_MODE_4,
    OLT_CMD_GET_OPTICAL_TX_MODE_5,
    
    OLT_CMD_SET_VIRTUAL_SCOPE_ADC_CONFIG_6,
    OLT_CMD_GET_VIRTUAL_SCOPE_MEASUREMENT_7,
    OLT_CMD_GET_VIRTUAL_SCOPE_RSSI_MEASUREMENT_8,
    OLT_CMD_GET_VIRTUAL_SCOPE_ONU_VOLTAGE_9,
    OLT_CMD_SET_VIRTUAL_LLID_10,
    
    OLT_CMD_SET_OPTICAL_TX_MODE2_11,

    OLT_CMD_SPLIT_LINE_OPTICS_FUN_END,
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
    OLT_CMD_SPLIT_LINE_MONITOR_FUN_BEGIN = 700,
    
    OLT_CMD_GET_RAW_STAT_1,
    OLT_CMD_RESET_COUNTER_2,
    OLT_CMD_SET_BER_ALARM_3,
    OLT_CMD_SET_FER_ALARM_4,
    OLT_CMD_SET_PON_BER_ALARM_5,
    
    OLT_CMD_SET_PON_FER_ALARM_6,
    OLT_CMD_SET_BER_ALARM_PARAMS_7,
    OLT_CMD_SET_FER_ALARM_PARAMS_8,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_SET_ALARM_Config_9,
    OLT_CMD_GET_STATISTICS_10,

    OLT_CMD_OLT_SELF_TEST_11,
    OLT_CMD_LINK_TEST_12,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    OLT_CMD_SET_LLID_FEC_MODE_13,
    OLT_CMD_GET_LLID_FEC_MODE_14,
    
    OLT_CMD_SPLIT_LINE_MONITOR_FUN_END,
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    OLT_CMD_SPLIT_LINE_HOTSWAP_FUN_BEGIN = 800,
    
    OLT_CMD_GET_HOTSWAP_CAP_1,
    OLT_CMD_GET_HOTSWAP_MODE_2,
    OLT_CMD_SET_HOTSWAP_MODE_3,
    OLT_CMD_FORCE_HOTSWAP_PON_4,
    OLT_CMD_SET_HOTSWAP_PARAM_5,
    
    OLT_CMD_RDN_ONU_REGISTER_6,
    OLT_CMD_SET_RDN_CONFIG_7,
    OLT_CMD_RDN_SWITCHOVER_8,
    OLT_CMD_RDN_OLT_ISEXIST_9,
    OLT_CMD_RESET_OLT_RDN_RECORD_10,
    
    OLT_CMD_GET_RDN_OLT_STATE_11,
    OLT_CMD_SET_RDN_OLT_STATE_12,
    OLT_CMD_GET_RDN_ADDR_TBL_13,
    OLT_CMD_REMOVE_RDN_OLT_14,
    OLT_CMD_GET_RDN_LLID_DB_15,
    
    OLT_CMD_SET_RDN_LLID_DB_16,
    OLT_CMD_REMOVE_RDN_OLT_17,
    OLT_CMD_SWAP_RDN_OLT_18,
    OLT_CMD_ADD_RDN_OLT_19,
    OLT_CMD_LOOSE_RDN_OLT_20,

    /*Begin:for onu swap by jinhl@2013-02-22*/
    OLT_CMD_RDN_LLID_ADD_21,
    OLT_CMD_GET_RDNLLID_MODE_22,
    OLT_CMD_SET_RDNLLID_MODE_23,
    OLT_CMD_SET_RDNLLID_STDBYTOACT_24,
    OLT_CMD_SET_RDNLLID_RTT_25,
    /*End:for onu swap by jinhl@2013-02-22*/
    
    OLT_CMD_RDN_IS_READY_26,
    OLT_CMD_GET_RDN_LLID_REGINFO_27,

    OLT_CMD_SPLIT_LINE_HOTSWAP_FUN_END,
#endif

#if 1
/* --------------------OLT CMC管理API------------------- */
    OLT_CMD_SPLIT_LINE_CMC_FUN_BEGIN = 1000,

    OLT_CMD_DUMP_ALL_CMC_1,
    OLT_CMD_SET_ALL_CMC_SVLAN_2,
    
    OLT_CMD_SPLIT_LINE_CMC_FUN_END,
#endif

    OLT_CMD_MAX
}OLT_CMD_ID;

#define OLT_ID_ALL       -1
#define OLT_ID_INVALID   -1
#define OLT_ID_NULL      0xFF
/* B--added by liwei056@2010-9-29 for OLT's NetManage */
#define UNKNOWN_OLT_ID   0xFF
#define OLT_NET_ID(remote_slot, remote_port)    (((remote_slot) << 8) | ((remote_port) & 0xFF))
#define OLT_DEVICE_ID(local_slot, local_port)   (((local_slot) << 8) | ((local_port) & 0xFF))
#define OLT_SLOT_ID(olt_id)                     ((olt_id) >> 8)
#define OLT_PORT_ID(olt_id)                     ((olt_id) & 0xFF)

#define OLT_ISNET(olt_id)            SYS_MODULE_IS_REMOTE(OLT_SLOT_ID(olt_id))
#define OLT_ISREMOTE(olt_id)         ((olt_id) > 0xFF)
#define OLT_ISLOCAL(olt_id)          ((olt_id) < 0xFF)
/* E--added by liwei056@2010-9-29 for OLT's NetManage */

#define OLT_SLOT_ISVALID(slotid)         (((slotid) >= PONCARD_FIRST) && ((slotid) <= PONCARD_LAST))
#define OLT_PORT_ISVALID(portid)         (((portid) > 0) && ((portid) <= PONPORTPERCARD))

#define OLT_LOCAL_ISVALID(oltid)         (((oltid) >= 0) && ((oltid) < MAXPON))
#define OLT_GLOBAL_ISVALID(oltid)        (((oltid) >= 0) && ((oltid) < MAXGLOBALPON))
#define OLT_NET_ISVALID(oltid)           ( SYS_SLOT_IS_VALID_REMOTE(OLT_SLOT_ID(oltid)) && SYS_PORT_IS_VALID_REMOTE(OLT_PORT_ID(oltid)) )
#define OLT_REMOTE_ISVALID(oltid)        ( SYS_MODULE_IS_LOCAL(OLT_SLOT_ID(oltid)) ? OLT_PORT_ISVALID(OLT_PORT_ID(oltid)) : OLT_NET_ISVALID(oltid) )

#define OLT_ISVALID(oltid)               (OLT_ISLOCAL(oltid) ? OLT_LOCAL_ISVALID(oltid) : OLT_REMOTE_ISVALID(oltid))
#define OLT_ID_ISVALID(oltid)            (OLT_ISLOCAL(oltid) ? OLT_GLOBAL_ISVALID(oltid) : OLT_REMOTE_ISVALID(oltid))

#define ONU_IDX_ISVALID(onuidx)          ( ((onuidx) >= 0) && ((onuidx) < MAXONUPERPON) )
#define ONU_ID_ISVALID(onuid)            ( ((onuid) > 0) && ((onuid) <= MAXONUPERPON) )

#define LLID_IDX_ISVALID(llid_idx)       ( ((llid_idx) >= 0) && ((llid_idx) < MAXLLIDPERONU) )
#if 0
/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#define LLID_ISVALID(llid)               ( ((llid) >= -1) && ((llid) <= 0xFFF) )
#else
#define LLID_ISVALID(llid)               ( ((llid) >= -1) && ((llid) <= MAXLLID) )
#endif
#define LLID_ISUNICAST(llid)             ( ((llid) > 0) && ((llid) < MAXLLID) )

#define VLAN_ID_ISVALID(vlanid)          ( ((vlanid) > 0) && ((vlanid) < 4095) )

#define OLT_LOCAL_ASSERT(oltid)          VOS_ASSERT( OLT_LOCAL_ISVALID(oltid) )
#define OLT_REMOTE_ASSERT(oltid)         VOS_ASSERT( OLT_REMOTE_ISVALID(oltid) )
#define OLT_NET_ASSERT(oltid)            VOS_ASSERT( OLT_NET_ISVALID(oltid) )
#define OLT_ASSERT(oltid)                VOS_ASSERT( OLT_ISVALID(oltid) )
#define OLT_ID_ASSERT(oltid)             VOS_ASSERT( OLT_ID_ISVALID(oltid) )
#define OLT_ALLID_ASSERT(oltid)          VOS_ASSERT( OLT_ID_ALL == oltid )
#define OLT_LOCALID_ASSERT(oltid)        VOS_ASSERT( (OLT_ID_ALL == oltid) || OLT_LOCAL_ISVALID(oltid) )

#define PONCARD_ASSERT(slotid)           VOS_ASSERT( (slotid >= PONCARD_FIRST) && (slotid <= PONCARD_LAST) )
#define OLT_CMD_ASSERT(olt_cmd)          VOS_ASSERT( ((olt_cmd) > 0) && ((olt_cmd) < OLT_CMD_MAX) )
#define VLAN_ASSERT(vlan_id)             VOS_ASSERT( VLAN_ID_ISVALID(vlan_id) )
#define LLID_ASSERT(llid)                VOS_ASSERT( LLID_ISVALID(llid) )
#define LLID_ID_ASSERT(llid_id)          VOS_ASSERT( LLID_IDX_ISVALID(llid_id) )


extern int  EVENT_RPC;
#define OLT_RPC_SND_TITLE     "\r\n[OLT-RPC-SND:]"
#define OLT_RPC_RCV_TITLE     "\r\n[OLT-RPC-RCV:]"
#define OLT_RPC_DEBUG         if (V2R1_ENABLE == EVENT_RPC) sys_console_printf  

extern int  EVENT_NULL;
#define OLT_NULL_TITLE        "\r\n[OLT-NULL:]"
#define OLT_NULL_DEBUG        if (V2R1_ENABLE == EVENT_NULL) sys_console_printf  

extern int  EVENT_PAS;
#define OLT_PAS_TITLE         "\r\n[OLT-PAS:]"
#define OLT_PAS_DEBUG         if (V2R1_ENABLE == EVENT_PAS) sys_console_printf  

extern int  EVENT_TK;
#define OLT_TK_TITLE         "\r\n[OLT-TK:]"
#define OLT_TK_DEBUG         if (V2R1_ENABLE == EVENT_TK) sys_console_printf  

extern int  EVENT_BCM;
#define OLT_BCM_TITLE         "\r\n[OLT-BCM:]"
#define OLT_BCM_DEBUG         if (V2R1_ENABLE == EVENT_BCM) sys_console_printf  

#if defined(_GPON_BCM_SUPPORT_)
extern int  EVENT_GPON;
extern int  EVENT_GPON_UPDATE;
#define OLT_GPON_TITLE         "\r\n[OLT-GPON:]"
#define OLT_GPON_DEBUG         if (V2R1_ENABLE == EVENT_GPON) sys_console_printf  
#define OLT_GPON_UPDATE_DEBUG         if (V2R1_ENABLE == EVENT_GPON_UPDATE) sys_console_printf  
#endif

extern int  EVENT_GW;
#define OLT_GW_TITLE          "\r\n[OLT-GW:]"
#define OLT_GW_DEBUG          if (V2R1_ENABLE == EVENT_GW) sys_console_printf  

extern int EVENT_SYSLOG;
extern int DebugOnuDeviceIdx;
#define OLT_SYSLOG_DEBUG    if((V2R1_ENABLE == EVENT_SYSLOG) && (DebugOnuDeviceIdx == 0xFFFFFFFF || deviceIndex == DebugOnuDeviceIdx)) VOS_SysLog
extern int  EVENT_SYN;
#define OLT_SYNC_TITLE        "\r\n[OLT-SYNC:]"
#define OLT_SYNC_SND_TITLE    "\r\n[OLT-SYNC-SND:]"
#define OLT_SYNC_RCV_TITLE    "\r\n[OLT-SYNC-RCV:]"
#define OLT_SYNC_DEBUG        if (V2R1_ENABLE == EVENT_SYN) sys_console_printf  

extern int EVENT_REGISTER;
#define OLT_REG_TITLE          "\r\n[OLT%d-REG:]"
#define OLT_REG_TAIL           , olt_id
#define OLT_REG_ISDEBUG        (V2R1_ENABLE == EVENT_REGISTER) 
#define OLT_REG_DEBUG          if (OLT_REG_ISDEBUG) sys_console_printf  

extern int  EVENT_PONADD;
#define OLT_ADD_TITLE         "\r\n[OLT-ADD:]"
#define OLT_ADD_DEBUG         if (V2R1_ENABLE == EVENT_PONADD) sys_console_printf  

extern int  EVENT_PONSWITCH;
#define OLT_SWITCH_TITLE      "\r\n[OLT-SWITCH:]"
#define OLT_SWITCH_DEBUG      if (V2R1_ENABLE == EVENT_PONSWITCH) sys_console_printf  

/*for onu swap by jinhl@2013-06-14*/
extern int  EVENT_ONUSWITCH;
#define ONU_SWITCH_DEBUG      if (V2R1_ENABLE == EVENT_ONUSWITCH) sys_console_printf

extern int  EVENT_REMOTE;
#define OLT_REMOTE_TITLE      "\r\n[OLT-REMOTE:]"
#define OLT_REMOTE_SND_TITLE  "\r\n[OLT-REMOTE-SND:]"
#define OLT_REMOTE_RCV_TITLE  "\r\n[OLT-REMOTE-RCV:]"
#define OLT_REMOTE_DEBUG      if (V2R1_ENABLE == EVENT_REMOTE) sys_console_printf  


#define OLT_CHIP_NAMELEN      16
#define OLT_CHIP_UNKNOWN_NAME "PON-unknown"

typedef enum
{
    OLT_ERR_NOTNEED_CFG = 2,
    OLT_ERR_REMOTE_OK = 1,
    OLT_ERR_OK = 0,
    OLT_ERR_UNKNOEWN = -1,
    OLT_ERR_TIMEOUT = -2,
    OLT_ERR_NOTSUPPORT = -3,
    OLT_ERR_PARAM = -4,
    OLT_ERR_NOTEXIST = -5,
    OLT_ERR_MALLOC = -6,
    OLT_ERR_NORESC = -7,
    OLT_ERR_PARTOK = -8,
    OLT_ERR_DATA = -9,

    OLT_ERR_DBA_BEGIN = -10,
    OLT_ERR_DBA_END = -30,

    OLT_ERR_UPGRADE_FAILED = -49,
    OLT_ERR_UPGRADE_BEGIN = -50,
    OLT_ERR_UPGRADE_INBUSY = -51,
    OLT_ERR_UPGRADE_VERION = -52,
    OLT_ERR_UPGRADE_NOFILE = -53,
    OLT_ERR_UPGRADE_FORBIDDED = -54,
    OLT_ERR_UPGRADE_END = -70,
   
    
    OLT_ERR_MAX = -100
}OLT_ERROR_CODE;

#define OLT_CALL_ISOK(iRlt)     (0 <= (iRlt))
#define OLT_CALL_ISERROR(iRlt)  (0 > (iRlt)) 
#define OLT_CALL_ISFAIL(iRlt)   (OLT_CALL_ISERROR(iRlt) && (OLT_ERR_NOTEXIST != (iRlt))) 

#define OLT_RPC_CALLFLAG_MANUALFREERLT 0x01
#define OLT_RPC_CALLFLAG_OUTPUTASINPUT 0x02
#define OLT_RPC_CALLFLAG_LOOPBACK      0x04
#define OLT_RPC_CALLFLAG_ASYNC         0x08


typedef enum
{
    OLT_ADAPTER_NULL   = 0,
    OLT_ADAPTER_GLOBAL = 1,    
    OLT_ADAPTER_RPC    = 2,

    OLT_ADAPTER_PAS5001,    
    OLT_ADAPTER_PAS5201,    
    OLT_ADAPTER_PAS5204,    
    OLT_ADAPTER_PAS8411,    

    OLT_ADAPTER_TK3723,
    OLT_ADAPTER_BCM55524,
    OLT_ADAPTER_BCM55538,
    
    OLT_ADAPTER_GW,   
	#if defined(_GPON_BCM_SUPPORT_)
	OLT_ADAPTER_GPON,
	#endif
    OLT_ADAPTER_MAX    
}OLT_ADAPTER_TYPE;

typedef enum
{
    OLT_GPIO_NONE = 0,

    OLT_GPIO_LED_OK,
    OLT_GPIO_LED_RUN,
    OLT_GPIO_LED_ALARM,

    OLT_GPIO_PON_LOSS,
    OLT_GPIO_SFP_LOSS,
    
    OLT_GPIO_PON_WATCHDOG_SWITCH,
    OLT_GPIO_PON_WATCHDOG,
    
    OLT_GPIO_MAX    
}OLT_GPIO_FUNC_ID;

typedef enum
{
    OLT_LED_NONE  = 0,
    OLT_LED_OK    = OLT_GPIO_LED_OK,
    OLT_LED_RUN   = OLT_GPIO_LED_RUN,
    OLT_LED_ALM   = OLT_GPIO_LED_ALARM,
    OLT_LED_LOSS  = OLT_GPIO_PON_LOSS,
    OLT_LED_MAX    
}OLT_LED_ID;

typedef enum
{
    OLT_CFG_DIR_UPLINK   = 0x01,
    OLT_CFG_DIR_DOWNLINK = 0x02,
    OLT_CFG_DIR_BOTH     = 0x03,
    OLT_CFG_DIR_UNDO     = 0x04,
    OLT_CFG_DIR_INACTIVE = 0x08,
    OLT_CFG_DIR_FORCE    = 0x10,
    OLT_CFG_DIR_BY_ONUID   = 0x20,   
    OLT_CFG_DIR_DO_ACTIVE  = 0x40,
    OLT_CFG_DIR_MAX    
}OLT_CFG_DIRECTION;

typedef enum PON_VENDOR_ID
{
    PON_VENDOR_UNKNOWN         = 0,
    PON_VENDOR_TEKNOVUS        = 1,    /* < Teknovus */
    PON_VENDOR_INTEROP         = 2,    /* < Interop */
    PON_VENDOR_KTEXTINTEROP    = 4,    /* < KT extended interop */
    PON_VENDOR_DASAN           = 8,    /* < Dasan */
    PON_VENDOR_PMC             = 16    /* < PMC */
} PON_VENDOR_ID;

typedef enum OnuVendorTypes
{
    OnuVendorTypesUnknown                                 = 0,
    OnuVendorTypesTeknovus                                = 1,    /* < Teknovus */
    OnuVendorTypesInterop                                 = 2,    /* < Interop */
    OnuVendorTypesKTExtInterop                            = 4,    /* < KT extended interop */
    OnuVendorTypesDasan                                   = 8,    /* < Dasan */
    OnuVendorTypesPmc                                     = 16    /* < PMC */
} OnuVendorTypes;

typedef struct
{
    OAM_standard_version_t  oam_version;
    PON_rtt_t				rtt;
    OnuVendorTypes          vendorType;
    unsigned short          productCode;   
    unsigned short          productVersion;
    unsigned short 		    laser_on_time;	/* Measured in TQ, range: 0 - (2^16-1) */
    unsigned short 		    laser_off_time; /* Measured in TQ, range: 0 - (2^16-1) */
    unsigned char           max_links_support;
    unsigned char           curr_links_num;
    unsigned char           max_cm_support;
    unsigned char           pon_rate_flags;
} onu_registration_info_t;

typedef struct OLT_vlan_qinq_t
{
    short int qinq_direction;
    short int qinq_objectid;
    union{
        PON_olt_vlan_uplink_config_t   up_cfg;
        PON_olt_vid_downlink_config_t  down_cfg;
    }qinq_cfg;
}OLT_vlan_qinq_t;

typedef struct ONU_bw_t
{
    unsigned int  bw_direction;
	unsigned int  bw_rate;
    unsigned int  bw_gr;  /* bandwidth, unit: Kbit/s*/
    unsigned int  bw_be;  /* unit: Kbit/s */
    unsigned int  bw_fixed;
    unsigned int  bw_actived;  
    unsigned int  bw_class;
    unsigned int  bw_delay;
}ONU_bw_t;

typedef struct ONU_SLA_INFO_t
{
    short int SLA_Ver;
    short int DBA_ErrCode;
    union{
        BcmLinkSlaDbaInfo SLA6;
        TkLinkSlaDbaInfo SLA5;
        PLATO4_SLA_t SLA4;/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        PLATO3_SLA_t SLA3;
        PLATO2_SLA_t SLA2;
    }SLA;
}ONU_SLA_INFO_t;

typedef struct OLT_raw_stat_item_t
{
    short int collector_id;
    short int statistics_parameter;
    PON_raw_statistics_t raw_statistics_type;

    PON_timestamp_t timestam;
    unsigned int statistics_data_size;
    void *statistics_data;
}OLT_raw_stat_item_t;

#define PON_OPTICAL_CAPABILITY_LOCKTIME     0x01
#define PON_OPTICAL_CAPABILITY_LASERTIME    0x02
typedef struct OLT_optical_capability_t
{
    int optical_capabilitys;
    bool pon_tx_signal;
    short int agc_lock_time;
    short int cdr_lock_time;
    unsigned short int  laser_on_time;	     /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
    unsigned short int  laser_off_time;      /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
}OLT_optical_capability_t;

typedef struct OLT_optics_detail_t
{
    PON_olt_optics_configuration_t pon_optics_params;
    bool pon_tx_signal;
}OLT_optics_detail_t;

typedef struct OLT_addr_table_cfg_t
{
    int removed_when_full;
    int removed_when_aged;
    int discard_llid_unlearned_sa;
    int discard_unknown_da;
    int allow_learning;
    int aging_timer;
}OLT_addr_table_cfg_t;

typedef struct OLT_onu_table_t
{
    int  onu_num;
    PON_onu_parameters_t onus;
}OLT_onu_table_t;

typedef struct OLT_pri2cosqueue_map_t  
{
    unsigned char priority[MAX_PRIORITY_QUEUE + 1];
} OLT_pri2cosqueue_map_t;

typedef struct LLID_SLA_INFO_t
{
    short int SLA_Ver;
    short int DBA_ErrCode;
    union{
		#if defined(_GPON_BCM_SUPPORT_)
		GPON_SLA_t SLA7;
		#endif
        BcmLinkSlaDbaInfo SLA6;
        TkLinkSlaDbaInfo SLA5;
        PLATO4_SLA_t SLA4;/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        PLATO3_SLA_t SLA3;
        PLATO2_SLA_t SLA2;
    }SLA;
}LLID_SLA_INFO_t;

typedef struct LLID_POLICE_INFO_t
{
    int  path;
    bool enable;
    PON_policing_parameters_t params;
}LLID_POLICE_INFO_t;

/*Begin:for onu swap by jinhl@2013-04-27*/
typedef struct PonPortBWInfo {
    unsigned char BWMode;        /* PON端口带宽分配方式 */
	unsigned char DownlinkPoliceMode; /* PON端口下行带宽限速模式 */ /* added by liwei056@2011-4-7 for GuestNeed */
	unsigned char BWExceedFlag;
	unsigned char DownlinkBWExceedFlag;
	unsigned int  DefaultOnuBW;	/* ONU连接默认带宽 */
	unsigned int  MaxBW;             /* PON端口最大可分配带宽 */
	unsigned int  ProvisionedBW; /* PON 端口已分配带宽*/
	unsigned int  ActiveBW;        /* PON端口当前使用带宽*/
	unsigned int  RemainBW;     /* PON端口剩余可分配带宽 */
	unsigned int  DownlinkMaxBw;
	unsigned int  DownlinkProvisionedBW;
	unsigned int  DownlinkRemainBw;
	unsigned int  DownlinkActiveBw;

}PonPortBWInfo_S;

typedef struct ONUBWInfo
{
	unsigned int  FinalUplinkBandwidth_gr;  /* bandwidth, unit: Kbit/s*/
	unsigned int  FinalUplinkBandwidth_be;  /* unit: Kbit/s */
    unsigned int  FinalUplinkBandwidth_fixed;        
	unsigned int  FinalDownlinkBandwidth_gr; 
	unsigned int  FinalDownlinkBandwidth_be; 
}ONUBWInfo_S;
/*End:for onu swap by jinhl@2013-04-27*/

#define OLT_ONUFLAG_ONLINE        0x01
#define OLT_ONUFLAG_OFFLINE       0x02
#define OLT_ONUFLAG_ONREG         0x04
#define OLT_ONUFLAG_OFFREG        0x08
#define OLT_ONUFLAG_VALID         0x10

#define OLT_RESUMEREASON_NULL     0x00
#define OLT_RESUMEREASON_UNKNOWN  PON_ONU_DEREGISTRATION_UNKNOWN
#define OLT_RESUMEREASON_SYNC     PON_ONU_DEREGISTRATION_HOST_REQUEST

#define OLT_RESUMEMODE_SYNCHARD   0x00
#define OLT_RESUMEMODE_SYNCSOFT   0x01
#define OLT_RESUMEMODE_FORCEDOWN  0x02
#define OLT_RESUMEMODE_FORCEUP    0x03
#define OLT_RESUMEMODE_ACTIVEHARD 0x04

#define PON_DEBUGFLAG_DEBUG       0x0001
#define PON_DEBUGFLAG_ENCRYPT     0x0002
#define PON_DEBUGFLAG_REGISTER    0x0004
#define PON_DEBUGFLAG_ALARM       0x0008
#define PON_DEBUGFLAG_RESET       0x0010
#define PON_DEBUGFLAG_ADD         0x0020
#define PON_DEBUGFLAG_SWITCH      0x0040
#define PON_DEBUGFLAG_FILE        0x0080
#define PON_DEBUGFLAG_RPC         0x0100
#define PON_DEBUGFLAG_REMOTE      0x0200


/* -------------------OLT API的安装及初始化接口---------- */
#if 1
struct stOltMgmtIFs;
extern void OLT_API_Init();
extern void OLT_RegisterAdapter(OLT_ADAPTER_TYPE adater_type, struct stOltMgmtIFs *olt_ifs);
extern int  OLT_SetupIFs(short int olt_id, OLT_ADAPTER_TYPE adater_type);
extern int  OLT_SetupIFsByChipType(short int olt_id, int chip_type);
extern int  OLT_GetIFType(short int olt_id);
extern int  OLT_GetIFChipType(short int olt_id);
#endif


#if 1
/* -------------------OLT 基本API的GW实现------------------- */
extern int GW_GetChipTypeID(short int olt_id, int *type);
extern int GW_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN]);
extern int GW_CmdIsSupported(short int olt_id, short int *cmd);
extern int GW_CopyOlt(short int olt_id, short int dst_olt_id, int copy_flags);
extern int GW_ResetPon(short int olt_id);
extern int GW_SetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *data);
extern int GW_GetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info);
extern int GW_SetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info);
extern int GW_GetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info);
#endif


#if 1
/* -------------------OLT PON管理API的GW实现--------------- */
extern int GW_GetAdminStatus(short int olt_id, int *admin_status);
extern int GW_GetPonWorkStatus(short int olt_id, int *work_status);
extern int GW_SetPPPoERelayMode(short int olt_id, int set_mode, int relay_mode);
extern int GW_SetPPPoERelayParams(short int olt_id, short int param_id, int param_value1, int param_value2);
extern int GW_SetDhcpRelayMode(short int olt_id, int set_mode, int relay_mode);
extern int  GW_OnuMacCheckEnable(short int olt_id,  ULONG enable);
extern int  GW_OnuMacCheckPeriod(short int olt_id,  ULONG enable);
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int GW_UpdateProvBWInfo( short int olt_id );
/*End:for onu swap by jinhl@2013-04-27*/
extern int GW_ChkDBAVersion(short int olt_id,bool * is_compatible);
#endif


#if 1
/* -------------------OLT 光路管理API------------------- */
int GW_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason);
int GW_GPONTxEnable(int olt_id, int tx_mode );
#endif


#if 1
/* -------------------OLT ONU管理API的GW实现-------------- */
extern int GW_GetOnuNum(short int olt_id, int onu_flags, int *onu_number);
extern int GW_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id);
#endif


#if 1
/* -------------------OLT 地址表管理API的GW实现----------- */
extern int GW_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);
extern int GW_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);
#endif


#if 1
/* -------------------OLT 倒换API的GW实现----------------- */
extern int GW_SetHotSwapParam(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2);
extern int GW_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status);
extern int GW_RdnIsReady(short int olt_id, int *iIsReady);
#endif



#if 1
/* -------------------OLT 基本API------------------- */
extern int OLT_IsExist(short int olt_id, bool *status);
extern int OLT_GetChipTypeID(short int olt_id, int *type);
extern int OLT_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN]);
extern int OLT_ResetPon(short int olt_id);
extern int OLT_ResetPonChip(short int olt_id);
extern int OLT_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt);

extern int OLT_CopyOlt(short int olt_id, short int dst_olt_id, int copy_flags);
extern int OLT_CmdIsSupported(short int olt_id, short int *cmd);
extern int OLT_SetDebugMode(short int olt_id, int debug_flags, int debug_mode);
extern int OLT_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address);
extern int OLT_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout);

extern int OLT_SetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info);
extern int OLT_GetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info);
extern int OLT_SetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info);
extern int OLT_GetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value );

extern int OLT_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value );
extern int OLT_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data );
extern int OLT_GpioAccess(short int olt_id, short int line_number, PON_gpio_line_io_t set_direction, short int set_value, PON_gpio_line_io_t *direction, bool *value);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_ReadGpio(short int olt_id, int func_id, bool *value);
extern int OLT_WriteGpio(short int olt_id, int func_id, bool value);

extern int OLT_SendChipCli(short int olt_id, unsigned short size, unsigned char *command);
extern int OLT_SetDeviceName(short int olt_id, char* pValBuf, int valLen);
#endif


#if 1
/* -------------------OLT PON管理API--------------- */
extern int OLT_GetVersion(short int olt_id, PON_device_versions_t *device_versions);
extern int OLT_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version);
extern int OLT_ChkVersion(short int olt_id, bool *is_compatibled);
extern int OLT_ChkDBAVersion(short int olt_id, bool *is_compatibled);
extern int OLT_GetCniLinkStatus(short int olt_id, bool *status);

extern int OLT_GetPonWorkStatus(short int olt_id, int *work_status);
extern int OLT_SetAdminStatus(short int olt_id, int admin_status);
extern int OLT_GetAdminStatus(short int olt_id, int *admin_status);
extern int OLT_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid);
extern int OLT_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_uplink_config);

extern int OLT_SetPonFrameSizeLimit(short int olt_id, short int jumbo_length);
extern int OLT_GetPonFrameSizeLimit(short int olt_id, short int *jumbo_length);
extern int OLT_OamIsLimit(short int olt_id, bool *oam_limit);
extern int OLT_UpdatePonParams(short int olt_id, int max_range, int mac_agetime);
extern int OLT_SetPPPoERelayMode(short int olt_id, int set_mode, int relay_mode);

extern int OLT_SetPPPoERelayParams(short int olt_id, short int param_id, int param_value1, int param_value2);
extern int OLT_SetDhcpRelayMode(short int olt_id, int set_mode, int relay_mode);
extern int OLT_SetIgmpAuthMode(short int olt_id, int auth_mode);
extern int OLT_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len);
extern int OLT_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len);

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_GetVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid, PON_olt_vid_downlink_config_t *vid_downlink_config);
extern int OLT_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid);
extern int OLT_GetOltParameters(short int olt_id, PON_olt_response_parameters_t *olt_parameters);
extern int OLT_SetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration);
extern int OLT_GetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration);

extern int OLT_SetOltMldForwardingMode(short int olt_id, disable_enable_t mode);
extern int OLT_GetOltMldForwardingMode(short int olt_id, disable_enable_t *mode);
extern int OLT_SetDBAReportFormat(short int olt_id, PON_DBA_report_format_t report_format);
extern int OLT_GetDBAReportFormat(short int olt_id, PON_DBA_report_format_t *report_format);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int OLT_UpdateProvBWInfo( short int olt_id );
/*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
extern int OLT_LLIDIsExist(short int olt_id, short int llid, bool *status);
extern int OLT_DeregisterLLID(short int olt_id, short int llid, bool iswait);
extern int OLT_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac);
extern int OLT_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info);
extern int OLT_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode);

extern int OLT_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA);
extern int OLT_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA);
extern int OLT_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police);
extern int OLT_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police);
extern int OLT_SetLLIDdbaType(short int olt_id, short int llid, int dba_type, short int *dba_error);

extern int OLT_GetLLIDdbaType(short int olt_id, short int llid, int *dba_type, short int *dba_error);
extern int OLT_SetLLIDdbaFlags(short int olt_id, short int llid, unsigned short dba_flags);
extern int OLT_GetLLIDdbaFlags(short int olt_id, short int llid, unsigned short *dba_flags);
extern int OLT_GetLLIDHeartbeatOam(short int olt_id, short int llid, unsigned short *send_period, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_timeout, unsigned short *recv_size, unsigned char *recv_data, bool *recv_IgnoreTrailingBytes);
extern int OLT_SetLLIDHeartbeatOam(short int olt_id, short int llid, unsigned short send_period, unsigned short send_size, unsigned char *send_data, unsigned short recv_timeout, unsigned short recv_size, unsigned char *recv_data, bool recv_IgnoreTrailingBytes);
#endif

#if 1
/* -------------------OLT ONU管理API-------------- */
extern int OLT_GetOnuNum(short int olt_id, int onu_flags, int *onu_number);
extern int OLT_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table);
extern int OLT_ClearAllOnus(short int olt_id);
extern int OLT_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode);
extern int OLT_SetAllOnuAuthMode(short int olt_id, int auth_mode);

extern int OLT_SetOnuAuthMode(short int olt_id, int auth_mode);
extern int OLT_SetMacAuth(short int olt_id, int mode, mac_address_t mac);
extern int OLT_SetAllOnuBindMode(short int olt_id, int bind_mode);
extern int OLT_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id);
extern int OLT_SetAllOnuDefaultBW(short int olt_id, ONU_bw_t *default_bw);

extern int OLT_SetAllOnuDownlinkPoliceMode(short int olt_id, int police_mode);
extern int OLT_SetOnuDownlinkPoliceMode(short int olt_id, int police_mode);
extern int OLT_SetAllOnuDownlinkPoliceParam(short int olt_id, int BwBurstSize, int BwPreference, int BwWeightSize);
extern int OLT_SetAllOnuUplinkDBAParam(short int olt_id, int BwFixedPktSize, int BwBurstSize, int BwWeightSize);
extern int OLT_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map);

extern int OLT_ActivePendingOnu(short int olt_id);
extern int OLT_ActiveOnePendingOnu(short int olt_id, unsigned char*mac);
extern int OLT_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id);
extern int OLT_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, unsigned char *mac);
extern int OLT_SetAllOnuAuthMode2(short int olt_id, int auth_mode);

extern int OLT_SetAllOnuBWParams(short int olt_id, int uplink_bwradio, int dwlink_bwradio);
extern int OLT_SetOnuP2PMode(short int olt_id, int p2p_mode);
extern int OLT_GetOnuB2PMode(short int olt_id, int *b2p_mode);
extern int OLT_SetOnuB2PMode(short int olt_id, int b2p_mode);

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_GetOnuMode( short int olt_id, short int llid );
extern int OLT_GetMACAddressAuthentication(short int olt_id, unsigned char	*number_of_mac_address, mac_addresses_list_t mac_addresses_list);	
extern int OLT_SetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	authentication_according_to_list);
extern int OLT_GetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	*authentication_according_to_list);
extern int OLT_GetDownlinkBufferConfiguration(short int olt_id, PON_downlink_buffer_priority_limits_t *priority_limits);

extern int OLT_GetOamInformation(short int olt_id, short int llid, PON_oam_information_t  *oam_information);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-02-22*/
extern int OLT_ResumeLLIDStatus(short int olt_id, short int llid, int resume_reason, int resume_mode);
extern int OLT_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag);
extern int OLT_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress);
/*End:for onu swap by jinhl@2013-02-22*/
extern int OLT_BroadCast_CliCommand(short int olt_id, int action_code);

extern int OLT_SetAuthEntry(short int olt_id, int code);
extern int OLT_SetOnuDefaultMaxMac(short int olt_id, int max_mac);
extern int OLT_SetCTCOnuPortStatsTimeOut(short int olt_id, ONU_PORTSTATS_TIMER_NAME_E timer_name, LONG timeout);

extern int OLT_SetCTCONUTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t * parameter);

#endif

#if 1
/* -------------------OLT 加密管理API---------- */
extern int OLT_SetEncryptMode(short int olt_id, int encrypt_mode);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_SetEncryptionPreambleMode(short int olt_id, bool encrypt_mode);
extern int OLT_GetEncryptionPreambleMode(short int olt_id, bool *encrypt_mode);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode);
extern int OLT_StartLLIDEncrypt(short int olt_id, short int llid);

extern int OLT_FinishLLIDEncrypt(short int olt_id, short int llid, short int status);
extern int OLT_StopLLIDEncrypt(short int olt_id, short int llid);
extern int OLT_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key);
extern int OLT_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status);
#endif


#if 1
/* -------------------OLT 地址表管理API-------- */
extern int OLT_SetMacAgingTime(short int olt_id, int aging_time);
extern int OLT_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg);
extern int OLT_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg);
extern int OLT_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl);
extern int OLT_GetMacAddrVlanTbl(short int olt_id, short int * addr_num, PON_address_vlan_table_t addr_tbl);
extern int OLT_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);

extern int OLT_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);
extern int OLT_RemoveMac(short int olt_id, mac_address_t mac);
extern int OLT_ResetAddrTbl(short int olt_id, short int llid, int addr_type);
extern int OLT_SetOnuMacThreshold(short int olt_id,  ULONG mac_threshold);
extern int OLT_SetOnuMacCheckEnable(short int olt_id,ULONG enable); 

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_SetAddressTableFullHandlingMode(short int olt_id, bool remove_entry_when_table_full);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 光路管理API------------- */
extern int OLT_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability);
extern int OLT_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params);
extern int OLT_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt);
extern int OLT_SetOpticalTxMode(short int olt_id, int tx_mode);
extern int OLT_GetOpticalTxMode(short int olt_id, int *tx_mode);

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
extern int OLT_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config);
extern int OLT_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len);
extern int OLT_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result);
extern int OLT_GetVirtualScopeOnuVoltage(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm);
extern int OLT_SetVirtualLLID(short int olt_id, short int llid, PON_virtual_llid_operation_t operation);
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */

extern int OLT_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason);
#endif


#if 1
/* -------------------OLT 监控管理API----------- */
extern int OLT_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item);
extern int OLT_ResetCounters(short int olt_id);
extern int OLT_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes);
extern int OLT_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames);
extern int OLT_SetPonBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes);

extern int OLT_SetPonFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames);
extern int OLT_SetBerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_bytes);
extern int OLT_SetFerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_frames);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OLT_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length);
extern int OLT_GetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration);

extern int OLT_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data);
extern int OLT_OltSelfTest(short int olt_id);
extern int OLT_LinkTest(short int olt_id, short int llid, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results);
extern int OLT_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec);
extern int OLT_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec);

extern int OLT_SysDump(short int olt_id, short int llid, int dump_type);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------OLT 倒换API---------------- */
#define OLT_SWAP_FLAGS_NONE                 0

#define OLT_SWAP_FLAGS_OLT_NOTWORKING       0x01
#define OLT_SWAP_FLAGS_PARTNER_NOTWORKING   0x02
#define OLT_SWAP_FLAGS_OLT_SINGLESETTING    0x04
#define OLT_SWAP_FLAGS_OLT_RESUMEMODE       0x08

#define OLT_SWAP_FLAGS_NOTSAVECFG           0x10
#define OLT_SWAP_FLAGS_NOTREPORT            0x20

#define OLT_SWAP_FLAGS_NOTALLWORKING        (OLT_SWAP_FLAGS_OLT_NOTWORKING | OLT_SWAP_FLAGS_PARTNER_NOTWORKING)
#define OLT_SWAP_FLAGS_ONLYSETTING          (OLT_SWAP_FLAGS_NOTSAVECFG | OLT_SWAP_FLAGS_NOTREPORT)

extern int OLT_GetHotSwapCapability(short int olt_id, int *swap_cap);
extern int OLT_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status);
extern int OLT_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags);
extern int OLT_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags);
extern int OLT_SetHotSwapParam(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2);

extern int OLT_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info);
extern int OLT_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable);
extern int OLT_RdnSwitchOver(short int olt_id);
extern int OLT_RdnIsExist(short int olt_id, bool *status);
extern int OLT_ResetRdnRecord(short int olt_id, int rdn_state);

extern int OLT_GetRdnState(short int olt_id, int *state);
extern int OLT_SetRdnState(short int olt_id, int state);
extern int OLT_RemoveRdnOlt(short int olt_id);
extern int OLT_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db);
extern int OLT_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db);

extern int OLT_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl);
extern int OLT_RdnRemoveOlt(short int olt_id, short int partner_olt_id);
extern int OLT_RdnSwapOlt(short int olt_id, short int partner_olt_id);
extern int OLT_AddSwapOlt(short int olt_id, short int partner_olt_id);
extern int OLT_RdnLooseOlt(short int olt_id, short int partner_olt_id);

/*Begin:for onu swap by jinhl@2013-02-22*/
extern int OLT_RdnLLIDAdd(short int olt_id, short int llid);
extern int OLT_GetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t* mode);
extern int OLT_SetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t mode);
extern int OLT_SetRdnLLIDStdbyToAct(short int olt_id, short int llid_n, short int* llid_list_marker);
extern int OLT_SetRdnLLIDRtt(short int olt_id, short int llid, PON_rtt_t rtt);
/*End:for onu swap by jinhl@2013-02-22*/

extern int OLT_RdnIsReady(short int olt_id, int *iIsReady);
extern int OLT_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo);
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
extern int OLT_DumpAllCmc(short int olt_id, char *dump_buf, unsigned short *dump_len);
extern int OLT_SetCmcServiceVID(short int olt_id, int svlan);
#endif



/* -------------------OLT高级API------------------- */
extern int         OLTAdv_CmdIsSupported(short int olt_id, int cmd);
extern int         OLTAdv_TestOltID(short int olt_id, void *vty);
extern int         OLTAdv_IsExist(short int olt_id);
extern int         OLTAdv_GetChipTypeID(short int olt_id);
extern const char* OLTAdv_GetChipTypeName(short int olt_id);
extern int         OLTAdv_SetCardI2CInfo(short int olt_id, int boardtype, char *boardsn, char *boardver, char *boarddate);
extern int         OLTAdv_GetCardI2CInfo(short int olt_id, int *boardtype, char *boardsn, char *boardver, char *boarddate);

extern bool        OLTAdv_ChkVersion(short int olt_id);
extern bool        OLTAdv_ChkDBAVersion(short int olt_id);
extern int         OLTAdv_GetAdminStatus(short int olt_id);
extern int         OLTAdv_GetPonWorkStatus(short int olt_id);

extern int         OLTAdv_SetIgmpAuthMode(short int olt_id, int auth_mode);

extern int         OLTAdv_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id);

extern int         OLTAdv_GetOpticalTxMode(short int olt_id);
extern int         OLTAdv_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason);

extern int         OLTAdv_HotSwapSelectMaster(short int *olt_id, short int *partner_id, short int *master_onunum, short int *slave_onunum);
extern int         OLTAdv_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int sync_slave);
extern int         OLTAdv_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status);
extern int         OLTAdv_ForceHotSwap(short int new_master_olt, short int new_slave_olt);
extern int         OLTAdv_RdnIsExist(short int olt_id);
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern int         OLTAdv_PLATO3_SetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error);
extern int         OLTAdv_PLATO3_GetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error);
extern int         OLTAdv_PLATO4_SetLLIDSLA(short int olt_id, short int llid, PLATO4_SLA_t *SLA, short int *dba_error);
extern int         OLTAdv_PLATO4_GetLLIDSLA(short int olt_id, short int llid, PLATO4_SLA_t *SLA, short int *dba_error);
extern int         OLTAdv_GetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error);
extern int         OLTAdv_SetLLIDSLA(short int olt_id, short int llid, PLATO3_SLA_t *SLA, short int *dba_error);
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern int         OLTAdv_SetLLIDPolice(short int olt_id, short int llid, int police_path, bool enable, PON_policing_parameters_t police_params);
extern int         OLTAdv_GetLLIDPolice(short int olt_id, short int llid, int police_path, bool *enable, PON_policing_parameters_t *police_params);
extern int         OLTAdv_LLIDIsExist(short int olt_id, short int llid);
extern int         OLTAdv_LLIDIsOnline(short int olt_id, short int llid);
extern int         OLTAdv_RdnIsReady(short int olt_id);
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int OLTAdv_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress);
extern int OLTAdv_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag);
/*End:for onu swap by jinhl@2013-04-27*/

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
extern int         OLTRM_GetRemoteOltID(int logical_slot, int logical_port);
extern int         OLTRM_GetRemoteLogicalOltID(short int remote_olt_id);
extern int         OLTRM_RemoteOltIsValid(short int olt_id);

#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
extern int         OLTRM_GetRemoteOltPartner(short int remote_olt_id, short int *local_partner_id);
extern int         OLTRM_SetRemoteOltPartner(short int remote_olt_id, short int local_partner_id);
#endif
#endif
/* E--added by liwei056@2010-6-1 for 6900-Prj */

/* B--added by liwei056@2010-8-18 for 6900-Prj */
#ifdef OLT_SYNC_SUPPORT
#define OLT_SYNC_EVENT_UPDATE_ATONCE   0x01
#define OLT_SYNC_EVENT_UPDATE_FORBID   0x02
#define OLT_SYNC_EVENT_UPDATE_CLEAR    0x04

typedef int  (*PF_OLT_SYNC_SUBMIT_OFFER)(void **sync_data, unsigned long *sync_datalen);
typedef int  (*PF_OLT_SYNC_NOTIFY_COLLECT)(void *sync_data, unsigned long sync_datalen);

typedef struct stOltSyncObj
{
    short int            dst_slot;
    short int            dst_port;
    short int            olt_id;
    short int            obj_id;

    PF_OLT_SYNC_SUBMIT_OFFER   pfOffer;
    PF_OLT_SYNC_NOTIFY_COLLECT pfCollect;
}OLT_SYNC_OBJECT;

extern void OLT_SYNC_Collector_Init();
extern void OLT_SYNC_Offeror_Init();
extern int OLT_SYNC_Add_Offeror(OLT_SYNC_OBJECT *pstOfferor, UINT16 nCircleTimeSec);
extern int OLT_SYNC_Add_Collector(OLT_SYNC_OBJECT *pstCollector, UINT16 nUpdateTimeSec);
extern int OLT_SYNC_Notify_Offeror(int sync_id, unsigned int sync_event);
extern int OLT_SYNC_Notify_Collector(int sync_id, unsigned int sync_event);
extern int OLT_SYNC_Remove_Offeror(int sync_id);
extern int OLT_SYNC_Remove_Collector(int sync_id);
#else
extern void OLT_SYNC_Init();
extern int OLT_SYNC_Data(short int olt_id, int sync_cmd, void *sync_data, unsigned long sync_datalen);
#endif
/* E--added by liwei056@2010-8-18 for 6900-Prj */


#define PONCHIP_EXIST  1
#define PONCHIP_NOT_EXIST  0

#define  V2R1_FILE_FOUND  1
#define  V2R1_FILE_NOT_FOUND  2

/* the flag value */
#define V2R1_ENABLE    1
#define V2R1_DISABLE   2
#define V2R1_COMPLETE  3

/*ctc onu authentication value 2012-2 added by luh */
#define  V2R1_ONU_AUTHENTICATION_DISABLE                0x1
#define  V2R1_ONU_AUTHENTICATION_ENABLE                0x2
#define  V2R1_ONU_AUTHENTICATION_NEW_ONLY               0x3
#define  V2R1_ONU_AUTHENTICATION_ALL                    0x4

extern unsigned char *v2r1Enable[];

extern unsigned char *v2r1EthType[];

#define  V2R1_STARTED 1
#define  V2R1_NOTSTARTED 2

#define  V2R1_CTC_ENCRYPT_NOOP   1
#define  V2R1_CTC_ENCRYPT_START   2
#define  V2R1_CTC_ENCRYPT_STOP  3
#define  V2R1_CTC_ENCRYPT_ENCRYPTING  4

extern unsigned char *v2r1Start[];

#define V2R1_OPERATION_NO  1
#define V2R1_OPERATION_RESET  2

/*
#define  ENCRYPT_NONE 1
#define  ENCRYPT_DOWN 2
#define  ENCRYPT_ALL 3
*/
extern unsigned char *v2r1EncryptDirection[];

#define V2R1_NONE 0
#define NOT_KNOWN  0

#define V2R1_UNKNOWN 0
#define V2R1_UP  1
#define V2R1_DOWN 2
#define V2R1_PENDING 3


#define ALARMMASK    0
#define ALARMOPEN    1
#define ALARMEXIST   1
#define ALARMNOEXIST 0

#define ALLALARMNUM  100

#define TEMPERATUREALARMID   6
#define FIRSTPWUALARM_BIT     0
#define SECONDPWUALARM_BIT  1
#define THIRDPWUALARM_BIT     2
#define FANALARM_BIT                3
#define PWUTEMPERATUREALARM_BIT   4
#define OTHERTEMPERATUREALARM_BIT  5

/* the power unit output voltage */
#define PWR_NORMAL   0
#define PWR_LOW        1 
#define PWR_HIGH       2
#define PWR_NOTPRESENT      3

/* power trap */
#define V2R1_POWER_ON   1
#define V2R1_POWER_OFF  2

#define V2R1_POWER_OFF_WAIT  5 

/* BER trap */
#define  V2R1_BER_ALARM  1
#define  V2R1_BER_CLEAR  2

/* FER trap */
#define  V2R1_FER_ALARM  1
#define  V2R1_FER_CLEAR  2

#define  V2R1_TRAP_SEND  1
#define  V2R1_TRAP_NOT_SEND  2
#define  V2R1_TRAP_CLEAR  2

/* the fan working status */
#define FAN_NORMAL  1
#define FAN_ABNORAML 0

#define   V2R1_ENTRY_UNKNOWN 0
#define   V2R1_ENTRY_ACTIVE  1
#define   V2R1_ENTRY_NOT_IN_ACTIVE  2
#define   V2R1_ENTRY_NOT_READY  3
#define   V2R1_ENTRY_CREATE_AND_GO   4
#define   V2R1_ENTRY_CREATE_AND_WAIT  5
#define   V2R1_ENTRY_DESTORY    6

#define ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT  8388480        /* 0~16777215 */
#define ONU_UPLINK_POLICE_BURSESIZE_DEFAULT    256            /* 0~256 */

/* B--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
#define ONU_UPLINK_DBA_PACKETSIZE_DEFAULT      256        /* 0~1522 */
/* E--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */

/* B--added by liwei056@2013-8-2 for TK-BW-Fix */
#define ONU_UPLINK_DBA_WEIGHT_MIN              2 
#define ONU_UPLINK_DBA_WEIGHT_MAX              32 
#define ONU_UPLINK_DBA_WEIGHT_DEFAULT          32 

#define ONU_DOWNLINK_DBA_WEIGHT_MIN            2 
#define ONU_DOWNLINK_DBA_WEIGHT_MAX            256 
#define ONU_DOWNLINK_POLICE_WEIGHT_DEFAULT     2
/* E--added by liwei056@2013-8-2 for TK-BW-Fix */

/* B--added by wangjiah@2017-04-14 for ONU GT524_B*/
#define POLLING_LEVEL_DEFAULT 1
#define POLLING_LEVEL_FACTORY 0 
/* E--added by wangjiah@2017-04-14 for ONU GT524_B*/

/* pon chip type */
#ifndef PON_CHIP_TYPE
#define  PON_CHIP_TYPE 
typedef enum{
    PONCHIP_TYPE_MIN = 0,
    PONCHIP_UNKNOWN = 0,

    PONCHIP_PAS = 1,
    PONCHIP_PAS5001 = 1,
    PONCHIP_PAS5201 = 2,
    PONCHIP_PAS5204 = 3,
    PONCHIP_PAS8411 = 4,
    PONCHIP_PASLAST = 6,

    PONCHIP_TK = 7,
    PONCHIP_TK3723 = 7,
    PONCHIP_BCM55524 = 8,
    PONCHIP_BCM55538 = 9,
    PONCHIP_TKLAST = 11,

    PONCHIP_CT = 12,
    PONCHIP_CTLAST = 14,

    PONCHIP_GW = 15,
    PONCHIP_GWLAST = 15,

	PONCHIP_GPON = 16,
    PONCHIP_TYPE_MAX = 17
}PonChipType_e;

#define OLT_PONCHIP_ISVALID(chip_type)   ( (0 < (chip_type)) && ((chip_type) < PONCHIP_TYPE_MAX) )
#define OLT_PONCHIP_ISINVALID(chip_type) ( (0 >= (chip_type)) || (PONCHIP_TYPE_MAX <= (chip_type)) )
#define OLT_PONCHIP_ASSERT(chip_type)    VOS_ASSERT(OLT_PONCHIP_ISVALID(chip_type));

#define OLT_PONCHIP_ISPAS(chip_type) ( (PONCHIP_PASLAST >= (chip_type)) && (0 < (chip_type)) )
/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
#define OLT_PONCHIP_ISPAS5001(chip_type) ( (PONCHIP_PAS5001 == (chip_type)) ) 
#define OLT_PONCHIP_ISPAS1G(chip_type) ( (PONCHIP_PAS5204 >= (chip_type)) && (0 < (chip_type)) )
#define OLT_PONCHIP_ISPAS10G(chip_type) ( (PONCHIP_PASLAST >= (chip_type)) && (PONCHIP_PAS8411 <= (chip_type)) )
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
#define OLT_PONCHIP_ISTK(chip_type)  ( (PONCHIP_TKLAST >= (chip_type)) && (PONCHIP_PASLAST < (chip_type)) )
#define OLT_PONCHIP_ISBCM(chip_type) (PONCHIP_BCM55538 == (chip_type))
#define OLT_PONCHIP_ISCT(chip_type)  ( (PONCHIP_CTLAST >= (chip_type)) && (PONCHIP_TKLAST < (chip_type)) )
#define OLT_PONCHIP_ISGW(chip_type)  ( PONCHIP_GW == (chip_type) )

#if defined(_GPON_BCM_SUPPORT_)
#define OLT_PONCHIP_ISGPON(chip_type) (PONCHIP_GPON == (chip_type))
#endif



/*
#define  ONUCHIP_PAS6201  0x6201
#define  ONUCHIP_PAS6301  0x6301

#define  I2C_REG_MODULE_PON_CHIP_TYPE  0x35   pon芯片类型地址偏移量
#define  I2C_REG_MODULE_PON_CHIP_TYPE_LEN 1

#define  PAS_CHIP_5001   0
#define  PAS_CHIP_5201   2
*/
#define  PONCHIP_TYPE_STRING_PAS5001   "PON_PAS5001"
#define  PONCHIP_TYPE_STRING_PAS5201   "PON_PAS5201"
#define  PONCHIP_TYPE_STRING_PAS8411   "PON_PAS8411"

#define  PONCHIP_TYPE_STRING_BCM55524  "PON_BCM55524"

#define  PONCHIP_TYPE_STRING_BCM55538  "PON_BCM55538"

#define  PONCHIP_DBA_TYPE_PAS5001      "DBA_PAS5001"
#define  PONCHIP_DBA_TYPE_PAS5201      "DBA_PAS5201"
#define  PONCHIP_DBA_TYPE_PAS5201_CTC  "DBA_PAS5201_CTC"

#define  PONCHIP_DBA_TYPE_PAS8411      "DBA_PAS8411"
#define  PONCHIP_DBA_TYPE_PAS8411_CTC  "DBA_PAS8411_CTC"

#if defined(_GPON_BCM_SUPPORT_)
#define  PONCHIP_TYPE_STRING_GPON  "PON_GPON"
#endif

#endif


#ifndef PON_CHIP_VENDOR
#define  PON_CHIP_VENDOR 
typedef enum{
    PONCHIP_VENDOR_MIN = 0,
    PONCHIP_VENDOR_UNKNOWN = 0,

    PONCHIP_VENDOR_PAS = 1,
    PONCHIP_VENDOR_TK = 2,
    PONCHIP_VENDOR_BCM = 3,
    PONCHIP_VENDOR_CT = 4,
    PONCHIP_VENDOR_GW = 5,
    
    PONCHIP_VENDOR_MAX
}PonChipVendor_e;
#endif



/* pon interface type */
enum PONPORTTYPE
{
PONPORTTYPEUNKNOWN = 0,
EPONMAUTYPE1000BASEPXOLT, /*1*/
EPONMAUTYPE1000BASEPXONU, /*2*/

EPONMAUTYPE1000BASEPX10DOLT, /*3*/
EPONMAUTYPE1000BASEPX10DONU, /*4*/

EPONMAUTYPE1000BASEPX10UOLT, /*5*/
EPONMAUTYPE1000BASEPX10UONU, /*6*/

EPONMAUTYPE1000BASEPX20DOLT, /*7*/
EPONMAUTYPE1000BASEPX20DONU, /*8*/

EPONMAUTYPE1000BASEPX20UOLT, /*9*/
EPONMAUTYPE1000BASEPX20UONU, /*10*/

EPONMAUTYPE10GBASEPRX30UONU, /*11*/
EPONMAUTYPE10GBASEPRX30DOLT, /*12*/

EPONMAUTYPE10GBASEPR30UONU,/*13*/
EPONMAUTYPE10GBASEPR30DOLT,/*14*/

GPONTYPE2G1GDOLT, /*15*/
GPONTYPE2G1GUONU, /*16*/

GPONTYPE2G2GDOLT, /*17*/
GPONTYPE2G2GUONU, /*18*/

/*stay consistent with PonPortType_s[] in PonHandler.c*/
PONPORTTYPEMAX /*19*/
};

extern unsigned char *PonPortType_s[];

/* device type define 
#define DEVICE_OTHER 1
#define EPV2R1_OLT 2
#define EPV2R1_ONU1 3
#define EPV2R1_ONU2 4
#define EPV2R1_ONU3 5
#define EPV2R1_ONU4 6
*/

/*  the software version consistent */
#define VERSION_CONSISTENT   1
#define VERSION_CONSISTENT_NOT  0

#define DEFAULT_DIS_PWU  50
#define DEFAULT_GEN_PWU 50
#define DEFAULT_DIS 50
#define DEFAULT_GEN 50

#ifndef  ETHERNET_FRAME_SOURCE_ADDRESS_BEGINNING_PLACE
#define ETHERNET_FRAME_SOURCE_ADDRESS_BEGINNING_PLACE 6
#endif


#define CHECK_PON_RANGE \
	if( !OLT_LOCAL_ISVALID(PonPortIdx) ){\
		sys_console_printf("\r\n PonId=%d out of range\r\n", PonPortIdx);\
		/*sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );*/\
		VOS_ASSERT(0);\
		return ( RERROR );\
		}

#define CHECK_LLID_RANGE\
	{\
	if( !OLT_LOCAL_ISVALID(PonPortIdx) || !LLID_ISVALID(Llid) ){\
		sys_console_printf("\r\n PonId=%d llid=%d out of range\r\n", PonPortIdx, Llid);\
		/*sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );*/\
		VOS_ASSERT(0);\
		return ( RERROR );\
		}\
	}
	/*if(( Llid < 0 ) || ( Llid >= MAXLLID )) { \
		sys_console_printf("\r\n PON%d/%d:llid=%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (Llid+1) );\
		sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );\
		ASSERT(0);\
		return ( RERROR );\
		}\ */
	
#define CHECK_ONU_RANGE \
	{ \
	if( !OLT_LOCAL_ISVALID(PonPortIdx) || !ONU_IDX_ISVALID(OnuIdx) ){\
		sys_console_printf("\r\n %s,%d,PonId=%d onuId=%d out of range\r\n", __FILE__, __LINE__, PonPortIdx, (OnuIdx+1));\
		/*sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );*/\
		VOS_ASSERT(0);\
		return ( RERROR );\
		}\
	}
	/* if(( OnuIdx < 0 ) || ( OnuIdx >= MAXONUPERPON )) {\
		sys_console_printf("\r\n PON%d/%d:onuId=%d out of range\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );\
		sys_console_printf("file:%s, line:%d\r\n",__FILE__, __LINE__ );\
		ASSERT(0);\
		return ( RERROR );\
		}\ */

#define CHECK_CMD_ONU_RANGE(vty, OnuIdx) \
	{ \
	if(OnuIdx && OnuIdx >= MAXONUPERPON){\	
	    vty_out(vty, "Onu index must be less than %d!\r\n", MAXONUPERPON);\
		return ( CMD_WARNING );\
		}\
	} 
/*{up(1) ,testing(2),down(3),unknown (4) , dormant(5),notPresent(6)*/
typedef enum {
	PONCHIP_UP = 1,	
	PONCHIP_ONLINE,
	PONCHIP_TESTING,
	PONCHIP_INIT,
	PONCHIP_DOWN,
	/*PONCHIP_UNKNOWN,*/
	PONCHIP_DORMANT,
	PONCHIP_NOTPRESENT,
	PONCHIP_DEL,
	PONCHIP_ERR
}PonChipWorkingStatus;

#define PONCHIP_NOTLOADING  PONCHIP_ONLINE
#define PONCHIP_LOADING   PONCHIP_INIT
#define PONCHIP_LOADCOMP PONCHIP_DORMANT
#define PONCHIP_ACTIVATED PONCHIP_UP
#define PONCHIP_ERRED  PONCHIP_ERR

/* pon card status */
#define PONCARD_NOTLOADING  1
#define PONCARD_LOADING 2
#define PONCARD_LOADCOMP 3
#define PONCARD_ACTIVATED 4
#define PONCARD_ERROR 5
#define PONCARD_DEL  6

extern unsigned char *CardStatus[];
extern unsigned char PonCardStatus[];

/* date defined */
#if 0
typedef struct Date{
	unsigned short year;
	unsigned char   month;
	unsigned char   day;
	unsigned short   hour;
	unsigned char   minute;
	unsigned char   second;
	unsigned long   MilliSecond;
} Date_S;
#endif
typedef struct {
	unsigned short int  year;
	unsigned char  month;
	unsigned char  day;
	unsigned char  hour;
	unsigned char  minute;
	unsigned char  second;
	unsigned long  MilliSecond;
}__attribute__((packed))Date_S;

/*
发件人: KD解世立 
发送时间: 2007年10月31日 19:21
收件人: KD刘冬; KD陈福军; KD张新辉; RJ田忠勇
抄送: KD盖鹏飞
主题: 修改GW OAM--告警实现方案和消息定义

11页：修改ONU设备信息查询OAM，增加ONU序列号和生产日期信息，同时对部分信息的最大长度限制由255B改为128B
*/
/* general Device Info */
/*#define MAXVENDORINFOLEN  128*/
#if 0
#define MAXVENDORLOCATIONLEN 40
#define MAXVENDORCONTACTLEN 40 
#define MAXCUSTOMERNAMELEN 40
#define MAXCUSTOMERCONTACTLEN 40
#define MAXCUSTOMERLOCATIONLEN 40
#endif
/*#define MAXDEVICENAMELEN 128
#define MAXDEVICEDESCLEN 128 */
/*#define MAXLOCATIONLEN 128*/
#if 0
#define MAXDEVICETYPELEN 20
#endif
#define MAXSWVERSIONLEN 32
#define MAXHWVERSIONLEN 32
#define MAXBOOTVERSIONLEN 32
#define MAXFWVERSIONLEN 32

#define MAXSNLEN		32
#define MAXDATELEN		32
#define MAXEQUIPMENTIDLEN 41
#define MAXDEVICEPWDLEN 21
/* modified by xieshl 20101228, 为了避免板间同步报文分片，压缩部分变量定义 */
#define MAXLLIDDESCLEN 128

typedef struct DeviceInfo_x{
	short int type;
	/* B--added by liwei056@2011-2-23 for OnuPonChipType */
	USHORT PonChipVendor;
	USHORT PonChipType; 
	/* E--added by liwei056@2011-2-23 for OnuPonChipType */
    
	UCHAR MacAddr[BYTES_IN_MAC_ADDRESS];
	UCHAR OUI[3];
	UCHAR AutoConfigFlag;

	char VendorInfo[MAXVENDORINFOLEN]; 
	char DeviceName[MAXDEVICENAMELEN];
	char DeviceDesc[MAXDEVICEDESCLEN];
	char Location[MAXLOCATIONLEN];
	UCHAR VendorInfoLen;
	UCHAR DeviceNameLen;
	UCHAR DeviceDescLen;
	UCHAR LocationLen;
	
	char SwVersion[MAXSWVERSIONLEN];
	char HwVersion[MAXHWVERSIONLEN];
	char BootVersion[MAXBOOTVERSIONLEN];
	char FwVersion[MAXFWVERSIONLEN];
	UCHAR SwVersionLen;
	UCHAR HwVersionLen;
	UCHAR BootVersionLen;
	UCHAR FwVersionLen;

	char SwVersion1[MAXSWVERSIONLEN];
	char DeviceSerial_No[MAXSNLEN];
	char MadeInDate[MAXDATELEN];
	UCHAR  DeviceSerial_NoLen;
	UCHAR MadeInDateLen;

    CHAR equipmentID[MAXEQUIPMENTIDLEN];
    UCHAR equipmentIDLen;    
    CHAR DevicePassward[MAXDEVICEPWDLEN];
    UCHAR DevicePasswardLen;

	Date_S  SysLaunchTime ;/*开通时间：格式：年、月、日、时、分、秒 */
    Date_S  SysOfflineTime;/*Onu 离线时间*/
	ULONG  SysUptime ; /*最近一次运行状态改变时的当前秒数*/
	ULONG  RelativeUptime; /* 状态最近一次变化时距离系统启动时的TICK	数*/
#if 0
	/* the follow field maybe be not used */
	char CustomerName[MAXCUSTOMERNAMELEN];
	int CustomerNameLen;
	char CustomerContact[MAXCUSTOMERCONTACTLEN];
	int CustomerContactLen;
	char CustomerLocation[MAXCUSTOMERLOCATIONLEN];
	int CustomerLocationLen;
#endif	
}__attribute__((packed))DeviceInfo_S;

typedef struct OltConfigInfo{
#if 0
	char VendorInfoDefault[MAXVENDORINFOLEN]; 
	int  VendorInfoLen;	
	char VendorLocationDefault[MAXVENDORLOCATIONLEN] ;
	int  VendorLocationLen;
	char VendorContactDefault[MAXVENDORCONTACTLEN];
	int VendorContactLen;
	char DeviceTypeDefault[MAXDEVICETYPELEN];
	int DeviceTypeLen;
#endif
	char DeviceNameDefault[MAXDEVICENAMELEN+1];
	int DeviceNameLen;
	char DeviceDesc[MAXDEVICEDESCLEN+1];
	int DeviceDescLen;
	
	char LocationDefault[MAXLOCATIONLEN+1];
	int LocationLen;
	unsigned char  MacAddrDefault[6];
#if 0
	int type;

	unsigned char  operStatusDefault ;
	unsigned char  adminStatusDefault ;
#endif
	unsigned char  PWUtemperatureThreshold_gen_Default ;  
	unsigned char  PWUtemperatureThreshold_dis_Default;   
	unsigned char  temperatureThreshold_gen_Default;  
	unsigned char  temperatureThreshold_dis_Default ;  
	unsigned long  AlarmMaskDefault ;
	unsigned char  MaxPonPortDefault;

} OltConfigInfo_S;


/* Alarm Info defined */
typedef struct AlarmInfo{
	unsigned int  ModuleId;
	unsigned int  AlarmId;
	unsigned int  AlarmClass;
	/*
	unsigned char AlarmStatus;
	*/
	unsigned long  AlarmMask; 
	char  AlarmDesc[30];
}AlarmInfo_S;

/* The define of the Alarm Class */
typedef enum {
	NOALARM = 0,
	VITAL,
	MAJOR,
	MINOR,
	WARNING,
	CLEAR
}AlarmClass;

/* olt working environment defined */
typedef struct  OltEnvironment {
	/*  ＝ 00时表示电源正常
		＝ 01时表示电源异常（电压底） 
		＝ 10 时表示电源异常（电压高）
		＝ 11时表示电源异常（ 掉电） */		
	unsigned char   CurrentPwu1 ;
	unsigned char   CurrentPwu2 ;
	unsigned char   CurrentPwu3 ;
	
	/* 表示风扇工作情况，1正常，0异常*/
	unsigned char   CurrentPan;

       /* 以下各参数分别表示各板卡的当前温度*/
	
	unsigned char   UpLink0Temperature;
	unsigned char   UpLink1Temperature;
	unsigned char   SW1Temperature;
	unsigned char   SW2Temperature;
	unsigned char   PON1Temperature;
	unsigned char   PON2Temperature;
	unsigned char   PON3Temperature;
	unsigned char   PON4Temperature;
	unsigned char   PWU1Temperature;
	unsigned char   PWU2Temperature;
	unsigned char   PWU3Temperature; /* 各板上有几个温度检测，待定 */
	
}OltEnvironment_S;  

   
typedef struct  OltMgmtInfo{
	DeviceInfo_S  DeviceInfo;
	int  adminStatus ;  /* enable(1) ,disable(2) */
	int  operStatus ; 

	unsigned char  PWUtemperatureThreshold_gen;  /*温度告警产生门限*/
	unsigned char  PWUtemperatureThreshold_dis;   /* 温度告警消失门限*/
	unsigned char  temperatureThreshold_gen;  /*温度告警产生门限*/
	unsigned char  temperatureThreshold_dis;   /* 温度告警消失门限*/
	
	OltEnvironment_S  environment;
	/*unsigned char AlarmMask;*/
	
	/* bit 2-0 分别表示当前三个电源是否有输出电压告警；
		bit 3 表示风扇当前是否有告警
		bit 6-4 表示当前三个电源温度是否有告警 
		bit 15-8 表示当前各板卡温度是否有告警
	*/
	unsigned long  AlarmStatus;
	/* power ( 0 ) , fan ( 1 ) , cpu ( 2 ) , temperature ( 3 ) , register ( 4 ) , present ( 5 ) */
	unsigned long  AlarmMask;
	/* bit 4-0 用来指示5块PON板卡是否插入
	     ＝1 插入，＝0 未插入		
	*/
	unsigned short int  InsertedPonCard;
	unsigned int  InsertedPonCardNum;
	unsigned char   MaxPonPort;
	unsigned char   CurrentPonPort;

	unsigned short int  InsertedTdmCard;
	bool IsSystemStart;
	unsigned int InsertedTdmCardNum;
	unsigned char CurrentTdmFPGA;

	OltConfigInfo_S  OltConfigInfoDefault;
	  
} OLTMgmtInfo_S;


/* PON chip 5001 Epon and CNI interface working parameters */
typedef  struct PonChipWorkingMode {
	/*EPON接口光模块参数：*/
	short int    laserOnTime;
	short int    laserOffTime;
	short int    AGCTime;
	short int    CDRTime;
    
	unsigned short int  MTU;                       /*最大传送单元*/
	unsigned short int  usReverse;                       
    
	unsigned char   bus_mode;             /* TBI or GMII */
	unsigned char   MAC_mode;            /* MAC or PHY */
	unsigned char   flowCtrlEnable;      /* 流控使能*/
	unsigned char   cReverse;      
	/*unsigned char   PortAdmniStatus ;  ENABLE/DISABLE */
    
       /*  SNI接口工作参数*/
	PON_mac_t  MacType;
}PonChipWorkingMode_S;


/* the PON chip 5001 management Info  */
typedef struct PONChipInfo{
	/*IF_Index  PonIfIndex;    PON 端口索引 */
	unsigned char   Type;                        /* PON芯片类型*/
	unsigned char   SFDType; 		/* 光模块类型*/
	unsigned char   Err_counter;
	unsigned char   ucReverse;
	unsigned char   TypeName[OLT_CHIP_NAMELEN];  /* PON芯片类型名字*/
	unsigned char   MgmtPathMAC[6];              /* PON芯片管理通信MAC地址*/
	unsigned char   DataPathMAC[6]; /* PON 芯片数据路径MAC地址*/
	unsigned char   FirmwareVer[32];   /* PON芯片固件版本 */
	unsigned char   HostSwVer[32];
	unsigned char   HardVer[32]; 

	/*
	PON_mac_t  MacType;
	short int LaserOnTime;
	short int LaserOffTime;
	short int AGC_time;
	short int CDR_time; */
	short int  version;   /* PON 芯片版本*/
    short int  sReverse;

	/*{up(1) ,down(2),testing(3),unknown (4) , dormant(5),notPresent(6)*/
	unsigned int   adminStatus;  /* PON芯片管理状态*/
	PonChipWorkingStatus  operStatus;      /* PON芯片工作状态*/
    
    PonChipWorkingMode_S WorkingMode;     /* PON芯片工作模式*/
}__attribute__((packed)) PONChipInfo_S;

#define MAX_ERR_CONTER 2


extern AlarmInfo_S AlarmInfoTable[];

#if 0
extern PONChipInfo_S  PonChipMgmtTable[];
#else
extern PONChipInfo_S  *PonChipMgmtTable;
#endif

extern OLTMgmtInfo_S  OLTMgmt;

extern unsigned char *PONchipStatus[];
/*extern unsigned char PAS_default_Mac[];
extern unsigned char Host_Default_Mac[];*/
extern unsigned char PAS_Data_Mac[];
extern unsigned char *PonChipType_s[];
extern unsigned char *PonInterfaceType[];
extern int PonChipActivatedFlag[];
extern unsigned char PonChipDownloadCounter[];
extern unsigned char PonBoardDownloadCounter[];
extern unsigned int PON_DOWNLOAD_MAX_COUNTER;

extern int PonPhyDebug[];
extern int PonMsgDebug[];
extern int PonOamMsgDebug[];

extern unsigned long  PonChipMgmtDataSemId;
extern unsigned long OltMgmtDataSemId;
#if 0
extern char V2R1_VERSION[];
extern char OltVendorInfo[];
extern char OltVendorLocation[];

extern char OltVendorLocationDefault[MAXVENDORLOCATIONLEN] ;
extern char OltVendorContactDefault[MAXVENDORCONTACTLEN];
extern char OltDeviceTypeDefault[MAXDEVICETYPELEN];
#endif
extern char OltDeviceNameDefault[];
extern char OltDeviceDescriptionDefault[];
extern char OltLocationDefault[];
extern unsigned char  OltMacAddrDefault[];

/*extern char V2R1Series[];
extern char V2R1Copyright[];*/
extern char ProductCorporationName[];
extern char ProductCorporationName_Short[];
extern char ProductCorporationName_AB[];

extern unsigned char  OltOperStatusDefault ;
extern unsigned char  OltAdminStatusDefault ;
extern unsigned char  OltPWUtemperatureThreshold_gen_Default ;  
extern unsigned char  OltPWUtemperatureThreshold_dis_Default;   
extern unsigned char  OltTemperatureThreshold_gen_Default;  
extern unsigned char  OltTemperatureThreshold_dis_Default ;  
extern unsigned long  OltAlarmMaskDefault ;
extern unsigned char   OltMaxPonPortDefault;

extern long V2r1TimerId;

extern int GetSysCurrentTime( Date_S *CurrentTime );

extern int GetCardIdxByPonChip( short int PonChipIdx );
extern int GetPonPortByPonChip( short int PonChipIdx );
extern short int GetPonPortIdxBySlot( short int slot, short  int port );
extern short int GetGlobalPonPortIdxBySlot( short int slot, short int port );
extern short int GetGlobalPonPortIdx( short int PonPortIdx );
extern int GetGlobalCardIdxByPonChip( short int PonPortGlobalIdx );
extern int GetGlobalPonPortByPonChip( short int PonPortGlobalIdx );
extern short int GetLocalPonPortIdx( short int PonPortIdx );
extern short int GetBasePonPortIdx( short int PonPortBaseIdx, short int PonPortIdx );
extern int PonPortIsLocal( short int PonChipIdx );

extern short int  V2R1_GetPonchipType( short int PonChipIdx );
extern short int  V2R1_GetPonChipVersion( short int PonChipIdx );
extern short int  GetPonChipTypeByPonPort( short int PonPortIdx);
extern int  GetPonchipMgmtMacAddr( short int PonChipIdx, unsigned char *MacAddr );
/*extern int  GetPonchipFirmwareVer( short int PonChipIdx, unsigned char *FirmwareVer );
extern int  GetPonchipHostSwVer(short int PonPonIdx, unsigned char *HostSwVer);*/

extern int GetPonCardVersion( unsigned int slot, unsigned char *FirmwareVer, unsigned char *HostSwVer, unsigned char *DBA_type, unsigned char *DBA_ver);

/* 还未实现
extern int  SetPonchipEPONWorkingMode ( short int PonChipIdx, unsigned char *EponParam );
extern int  SetPonchipCNIWorkingMode ( short int PonChipIdx, unsigned char *CNIParam );
*/
#ifdef __PON_CNI_WORK_MODE__
extern int  GetPonchipEPONWorkingMode ( short int PonChipIdx, short int *EponParam );
extern int  GetPonchipCNIWorkingMode ( short int PonChipIdx, unsigned char *CNIParam );
extern int  GetPonchipWorkingMode ( short int PonChipIdx, PonChipWorkingMode_S *WorkingParam );
#endif
extern int  GetPonchipWorkingStatus( short int PonChipIdx );
/*extern int  GetPonCardWorkingStatus( short int CardSlot );*/
extern int  GetPonchipWorkingStatusAll();
/*extern int  ClearPonChipMgmtInfo(short int PonChipIdx);*/
extern int  ClearPonChipMgmtInfoAll();
/*extern int  PonChipMgmtInfoInit(short int PonChipIdx );*/
extern int  PonChipFWVersionCompare( short int PonChipIdx );
extern int  SetPonChipDataPathMac( short int PonPortIdx );
extern int  SetPonchipMgmtPathMac( short int PonPortIdx );
extern int SetPonchipMgmtPathMac2( short int PonPortIdx, unsigned char mac_addr[BYTES_IN_MAC_ADDRESS] );
extern int *GetPonChipMgmtPathMac(short int PonPortIdx );
	
extern void GetPonChipInfo( short int PonCard, short int  port );
extern int  GetPonDeviceVersion(short int PonPortIdx);
extern int  GetPonChipFWVersion( char *version, int *len);
extern int  GetPonDeviceCapabilities( short int PonPortIdx );

extern int sendPonSoftUpdateMsg( short int PonPortIdx);
	
extern int  GetOltType();
extern int  SetOltDeviceName(  char *Name, int len );
extern int  GetOltDeviceName( char *Name , int *NameLen);
extern int  GetOltDeviceNameDefault(  char *Name ,  int *NameLen);
extern int  SetOltDeviceDesc( char *Desc, int len );
extern int  GetOltDeviceDesc( char *Desc, int *len );
extern int  GetOltDeviceDescDefault( char *Desc, int *len);
extern int  SetOltLocation( char *Location, int len );
extern int  GetOltLocation( char *Location, int *len);
extern int  GetOltLocationDefault( char *Location, int *len );
extern int  GetOltVendorInfo( char *Info, int *len);
extern int  GetOltFWVersion( char *Version, int *len );
extern int  GetOltSWVersion( char *Version, int *len);
extern int  GetOltHWVersion( char *Version, int *len);
extern int  GetOltOperStatus( void );
extern int  GetOltAdminStatus(void );
extern int  GetOltCurrenAlarmStatus();
extern int  SetOltMgmtMacAddr( char *MacAddr, int len);
extern int  GetOltMgmtMAC( char *MacAddr, int *len );
extern int  GetOltSysUpTime( void );




/*extern AlarmClass  GetOltCurrentAlarmRank( void ); */
extern int  SetOltVendorInfo( char *Info, int len );

/*
extern int  SetOltVendorLocation( unsigned char *Location, int len  );
extern int  GetOltVendorLocation( unsigned char *Location , int *len );
extern int  SetOltVendorContact( unsigned char *Telenum, int len );
extern int  GetOltVendorContact( unsigned char *Telenum, int *len );

extern int  SetOltCustomerName( unsigned char *Name, int len );
extern int  GetOltCustomerName( unsigned char *Name, int *len );
extern int  SetOltCustomerContact( unsigned char *Telenum, int len );
extern int  GetOltCustomerContact( unsigned char *Telenum, int *len );

extern int  SetOltDeviceType( unsigned char *Type, int len );
extern int  GetOltDeviceType( unsigned char *Type, int *len);
*/
extern int  SetOltSWVersion( char *SwVersion, int len );
extern int  SetOltHWVersion( char *HwVersion, int len );
extern int  SetOltFWVersion( char *FwVersion, int len );	
extern int  GetDeviceInfoFromHardware();

extern int  RecordSysLaunchTime();
extern int  GetSysLaunchTime( Date_S *date );
extern int  RecordOLtSysUpTime (void);
/*extern int  GetOltSysUpLastTime(void);*/

/*  is it need to set more threshold for different cards ? */
extern int SetOltTemperatureThreshold_gen( unsigned char  AlarmThreshold );
extern int GetOltTemperatureThreshold_gen( unsigned char  *AlarmThreshold );
extern int SetOltTemperatureThreshold_dis( unsigned char  AlarmThreshold );
extern int GetOltTemperatureThreshold_dis( unsigned char  *AlarmThreshold );

extern int  SetAlarmMask( unsigned int alarmIdx,  unsigned long alarmMask );
extern unsigned long  GetAlarmMask ( unsigned int alarmIdx );
extern int GetOltAlarmMask( unsigned long *pMask );
extern int SetOltAlarmMask( unsigned long mask );

extern int  GetOltEnvironment( OltEnvironment_S *CurEnvirnoment ) ;
extern int  RecordOltCurrentTemperature( int whichCard, unsigned char  value );
extern int  RecordOltPWUValue( int whichPWUCard, unsigned char  vlaue );
extern int  RecordOltFanStatus( unsigned char  status  );

extern unsigned short  GetPonSlotInsertedAll( void );
extern int  GetOltCardslotInserted( int whichCard);
extern int SetOltCardslotInserted( int  whichCard );
extern int SetOltCardslotPulled(int whichCard);


extern  unsigned int GetPonFwVersionFromFlash ( short int PonChipType, unsigned char ver[100] );

extern int  ClearOltMgmtInfo();
extern int  OltMgmtInfoDefault();
extern int  InitProductTypeVar();

/*extern void ShowPonChipStatusAll();
extern void ShowPonChipStatus( short int PonChipIdx );*/

extern int  V2r1TimerStart();
extern int  V2r1TimerCallback();

extern void  UpdatePonPortNum();
extern int  ShowOltInfo();
extern int  ShowOltInfoByVty(struct vty *vty);
extern int  RestoreToDefaultData();

#if 0
extern int  OltMgmgInfoFromFlash();
#endif

/*** general define ***/

typedef struct  GeneralMsgBody{
	int  Module_Id;
	int  function_code;
	int  length;
	int  DataPonter;
}GeneralMsgBody_S;

/*extern void  SearchAndInsertPonCard();*/
/* modified by xieshl 20120409, 主备倒换状态只能通过参数传入，如果在函数中读取可能是不准确的 */
extern void  PonCardInsert(unsigned long module_type, unsigned long CardIndex, unsigned long switchhovering);
extern void  PonCardPull(unsigned long module_type, unsigned long CardIndex );
extern void  PonCardActivate(short int CardIndex, short int switchover_flag );

/* 与设备管理接口函数*/
extern void  PonCardInserted( unsigned long module_type, unsigned long CardIndex );
extern void  PonCardPulled ( unsigned long module_type, unsigned long CardIndex );
extern void  PonCardActivated(unsigned long  CardIndex );
extern void  PonPortActivated( short int PonPortIdx );
extern void PonDevReset(short int PonPortIdx );
extern int  PonCardStatusCheck(unsigned long module_type, unsigned long CardIndex );

/*TRAP 接口函数*/

extern int  Trap_OnuRegister( short int PonPortIdx, short int OnuIdx );
extern int  Trap_OnuDeregister( short int PonPortIdx, short int OnuIdx ,unsigned long reason, int forcibly );
extern int  Trap_OnuRegisterConflict(short int PonPortIdx, short int OnuIdx /*, short int PonPort, unsigned char * MacAddr*/);
extern int  Trap_FirmwareLoad(short int PonPortIdx, int Result );
extern int  Trap_DBALoad(short int PonPortIdx, int Result );
/*extern int  Trap_OnuAppImageUpdate(short int PonPortIdx, short int OnuIdx, int Result );*/
extern int  Trap_PonPortAbnormal( short int PonPortIdx,  int ResetCode );
extern int  Trap_PonPortNormal( short int PonPortIdx );
extern int  Trap_onuPowerOff(short int PonPortIdx, short int OnuIdx);
extern int  Trap_OnuPowerAlarm(short int PonPortIdx, short int OnuIdx, unsigned long Power_OnOff);
/*extern int  Trap_OnuSoftwareUpdate(short int PonPortIdx, short int OnuIdx, unsigned long Result );*/
extern int  Trap_LlidBandwidthExceeding(short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned long Result );
extern int  Trap_PonPortProtectSwitch(short int PonPortIdx );
extern int  Trap_PonPortBER(short int PonPortIdx, unsigned int Ber, int result );
extern int  Trap_OnuPonPortBER(short int PonPortIdx, short int OnuIdx, unsigned int Ber, int result );
extern int  Trap_PonPortFER(short int PonPortIdx, unsigned int Fer, int result );
extern int  Trap_OnuPonPortFER(short int PonPortIdx, short int OnuIdx, unsigned int Fer, int result );
extern int  Trap_OnuAuthFailure( short int  PonPortIdx,  unsigned char  *MacAddr );
extern int  Trap_PonAutoProtectSwitch( short int PonPortIdx);
/* debug cli 命令接口函数*/

extern int  SetPonPhyDebug(short int PonPortIdx, int Debug_flag);
extern int  SetPonMsgDebug(short int PonPortIdx, int Debug_flag);
extern int  SetPonOamMsgDebug(short int PonPortIdx, int Debug_flag);
extern int  ClearMsgCount(short int PonPortIdx);
extern int  ShowMsgCountByVty(short int PonPortIdx, struct vty *vty);

extern int  SetGeneralEvent(int debugFlag);
extern int  SetEncryptEvent(int debugFlag);
extern int  SetRegisterEvent(int debugFlag);
extern int  SetAlarmEvent(int debugFlag);
extern int  SetResetEvent(int debugFlag);
extern int  SetAddPonEvent(int debugFlag);
extern int  SetRpcEvent( int debugFlag );
extern int  SetSwitchPonEvent( int debugFlag );
extern int  SetRemoteEvent( int debugFlag );

extern int  SetOamCommFlag( int flag );
extern int  SetMakeingTestEvent(int debugFlag );
extern int  SetOamCommFlag(int flag );

extern int  GetMakeingTestFlagDefault();
extern int  GetMakeingTestFlag();
extern int  GetOnuPolicerFlagDefault();
extern int  GetOnuPolicerFlag();
extern int  ShowAllDebugFlag(struct vty *vty);
extern int  SetOnuPolicerFlag( int flag );
extern int  SetOnuPolicerParam(int BwBurstSize, int BwPreference, int BwWeightSize);
extern int  GetOnuPolicerParam(int *BwBurstSize, int *BwPreference, int *BwWeightSize );
extern int  SetOnuDbaParam(int BwFixedPktSize, int BwBurstSize, int BwWeightSize);
extern int  GetOnuDbaParam(int *BwFixedPktSize, int *BwBurstSize, int *BwWeightSize);
extern int  SetOnuBwParams(int uplink_bwradio, int dwlink_bwradio);
extern int  GetOnuBwParams(int *uplink_bwradio, int *dwlink_bwradio);
extern int  SetOnuDefaultBW(ONU_bw_t *default_bw, struct vty *vty);

extern int V2r1TimerHandler();

extern short int monOnuBerAlmEnSet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int berAlmEn);
extern short int monOnuBerAlmEnGet(unsigned short oltId, /*unsigned short onuId, */unsigned int *pBerAlmEn);
extern short int monOnuFerAlmEnSet(unsigned short oltId, /*unsigned short onuId, */unsigned int ferAlmEn);
extern short int monOnuFerAlmEnGet(unsigned short oltId, /*unsigned short onuId,*/ unsigned int *pFerAlmEn);

extern short int monponBERThrSet(int berFallThr, int berNum);
extern short int monponFERThrSet(int ferFallThr, int ferNum);
extern short int monponBERThrGet(unsigned int *pBerFallThr, unsigned int *pBerNum);
extern short int monponFERThrGet(unsigned int *pFerFallThr, unsigned int *pFerNum);

#if 0
extern short int monponBERNumSet(unsigned int berNum);
extern short int monponFERNumSet(unsigned int ferNum);
extern short int monponBERNumGet(unsigned int *pBerNum);
extern short int monponFERNumGet(unsigned int *pFerNum);
#endif

extern short int monPonBerAlmEnGet(unsigned short oltId, unsigned int *pBerAlmEn);
extern short int monPonBerAlmEnSet(unsigned short oltId, unsigned int BerAlmEn);
extern short int monPonFerAlmEnGet(unsigned short oltId, unsigned int *pFerAlmEn);
extern short int monPonFerAlmEnSet(unsigned short oltId, unsigned int FerAlmEn);

extern int GetDeviceTypeLength();
/*extern int ConvertCharCase(unsigned char *CharString );*/

extern void  DoubleDataPrintf(double *Data, unsigned int precision, struct vty *vty);

/*
extern float ReadTemp(int channel);
extern void Read_Temperature(int channel);
*/

extern ULONG get_gfa_tdm_slotno();
extern ULONG get_gfa_e1_slotno();
extern ULONG get_gfa_sg_slotno();

/* included from Device_hardware.c */
extern UCHAR * typesdb_module_type2name( ULONG mtype );
extern char *pon_chip_typename( unsigned long pon_type );
extern char *pon_chip_type2name( unsigned long pon_type );
extern char *pon_sfpVendor2name(unsigned long sfpVendor);
extern char *pon_ext_buffer2name( unsigned long ext_buf );
extern char *tdm_chip_type2name( unsigned long tdm_type );
extern unsigned long getTdmChipType( unsigned char slot_id,  unsigned char tdm_id );
extern unsigned long getTdmChipInserted( unsigned char slot_id, unsigned char tdm_id );

extern int GetHavePonChipReset(unsigned long CardIndex );
extern int GetPonChipResetFlag(int slot, int port);
extern int ClearPonChipResetFlag(int slot, int port );
extern int SetPonChipResetFlag(int slot, int port );
extern unsigned int PonPortFirmwareAndDBAVersionMatch(short int PonPortIdx);
extern int PonPortFirmwareAndDBAVersionMatchByVty(short int PonPortIdx, struct vty *vty);
extern void AllPonPortFirmwareAndDBAVersionMatchByVty(struct vty *vty);

extern void  SysLog_PonPortAbnormal(short int PonPortIdx, int type);
extern void  FlashAlarm_PonPortVersionMismatch(short int PonPortIdx, int VersionType, int MismatchFlag);
extern void  Trap_PonPortSFPTypeMismatch(short int PonPortIdx, int  MismatchFlag);
extern void  Trap_PonPortSignalLoss(short int PonPortIdx, int SignalLoss);
/*extern short int  SendTstMsgToPonPort(short int PonPortIdx);*/

extern int  Trap_PonPortFull(short int PonPortIdx);
extern int Trap_PonPortFullClear(short int PonPortIdx);


#endif /* _OLTGERNRAL_H*/

