/**************************************************************
*
*    OnuGeneral.h -- Onu management high level Application functions General header
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   --------|--- --------|---------------------|------------
*	1.00	 | 14/04/2006 | Creation				| chen fj
*
* 1  added by chenfj 2006-12-21 
*     设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参数
* 2  2007-02-07 增加ONU自动配置
*
***************************************************************/
#ifndef _ONUGERNRAL_H
#define  _ONUGERNRAL_H

#include  "GwttOam/OAM_gw.h"
#include  "tftp/Tftp_down.h"
#include  "V2R1_product.h"
#include  "V2R1General.h"
#include  "OltGeneral.h"
#include  "PonGeneral.h"
#if 0
#ifdef CTC_EXT_OID
#include "ct_manage/CT_Rman_cli.h"
#endif
#endif

#ifndef  CTC_EXT_OID
#define CTC_EXT_OID
#endif

#ifndef  ETH_TYPE_0x8100
#define  ETH_TYPE_0x8100   1
#define  ETH_TYPE_0x9100   2
#define  ETH_TYPE_0x8a88   3
#endif

extern unsigned char *OnuImageType[];

#define ONU_NOT_DEFAULT_MAX_MAC_FLAG 0x8000
#define ONU_UNDO_MAX_MAC_FLAG 0x4000
#define ONU_DEFAULT_MAX_MAC      128

#define DAFAULT_MAX_ONU          128/*MAXONUPERPON*/
#define CONFIG_DEFAULT_MAX_ONU_FLAG     0x8000
#define CONFIG_CODE_ALL_PORT            0x4000
extern unsigned int  MaxOnuDefault;
extern ULONG conf_associate_share;
extern ULONG Timeout_delete_authentry;


#define ONU_CMD_ERROR_STR  "%% Excute command error or Maybe the onu is recovering config!\r\n"

/*
其中：
 
act:操作码， 0 -- 升级单个ONU；1 -- 升级全部ONU
para: 操作数， 当act为0时，该值为要升级的ONU索引，当act为 1 时，该值为0
*/
#define  CTC_ONU_MAX_ETHPORT  32/*24*/	/*64*/	/* modified by xieshl 20081118, 64 to 24 */

/* added  by chenfj 2007-8-1
	    增加GT813/GT816 作为CTC ONU注册，但仍作为私有ONU来 管理
	    以下两个定义用于识别CTC模式下GT813/GT816的注册信息*/
#define CTC_REGISTER_ID_PRE  0x4754
#define  CTC_GT813_MODEL  0x47548130
#define  CTC_GT816_MODEL  0x47548160
#define  CTC_GT810_MODEL  0x47548100	/* added by xieshl 20070930 */

#define  CTC_GT811_MODEL  0x4754811A
#define  CTC_GT812_MODEL  0x4754812A
#define  CTC_GT812B_MODEL  0x4754812B

#define  CTC_GT831_A_MODEL  0x4754831A
#define  CTC_GT831_A_CATV_MODEL  0x4754831B
#define  CTC_GT815_MODEL  0x47548150
#define  CTC_GT861_MODEL  0x47548610
#define  CTC_GT866_MODEL  0x47548660
#define  CTC_GT831B_MODEL  0x4754831B

#define  CTC_GT811B_MODEL  0x4754811B
#define  CTC_GT851_MODEL  0x47548510
#define  CTC_GT813B_MODEL  0x4754813B
#define  CTC_GT862_MODEL  0x47548620
#define  CTC_GT863_MODEL  0x47548630
#define  CTC_GT892_MODEL  0x47548920
#define  CTC_GT835_MODEL  0x47548350
#define  CTC_GT831_B_CATV_MODEL  0x4754831C
#define  CTC_GT815_B_MODEL  0x4754815B

#define  CTC_GT811_C_MODEL  0x30000000

#define CTC_ONU_VENDORID "GWDL"

#define  OnuIDStringDesc  "Please input onuId list,e.g 1,3-5,7, etc. the range for onuId is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
#define  INVALID_IP  0xffffffff
#define  OnuIDString  "Please input onuId,the range is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"

#define MAX_GETEUQINFO_COUNTER 2

#define  ONU_VENDOR_CT    1
#define  ONU_VENDOR_GW      2

#define  ONU_EXTOAM_STATUS_NONE      0
#define  ONU_EXTOAM_STATUS_SUCCESS   1
#define  ONU_EXTOAM_STATUS_FAIL      2
#define  ONU_EXTOAM_STATUS_DOING     4
#define  ONU_EXTOAM_STATUS_UNKNOWN   8

#define  ONU_EXTOAMNEGO_ISOVER(s)    (3 & (s))

#define HWVERSIONLEN  10
#define SWVERSIONLEN  14
#define  AUTO_CONFIG_NAME_LEN  32

#define  V2R1_ERROR_BASE			1000
#define  V2R1_EXCEED_RANGE			(V2R1_ERROR_BASE+1)
#define  V2R1_NOT_EXCEED_RANGE	(V2R1_ERROR_BASE+2)
#define  V2R1_ONU_NOT_EXIST		(V2R1_ERROR_BASE+3)
#define  V2R1_ONU_OFF_LINE			(V2R1_ERROR_BASE+4)
#define  V2R1_ONU_SETING_P2P		(V2R1_ERROR_BASE+5)
#define  V2R1_ONU_NOSETING_P2P	(V2R1_ERROR_BASE+6)
/*for onu swap by jinhl@2013-04-27*/
#define  V2R1_Parter_EXCEED_RANGE   (V2R1_ERROR_BASE+7)

#define  V2R1_ONU_FILTER_SA_MAC_NOT_VALID  (V2R1_ERROR_BASE+8)
#define  V2R1_ONU_FILTER_SA_MAC_EXIST  		(V2R1_ERROR_BASE+9)
#define  V2R1_ONU_FILTER_SA_MAC_NOT_EXIST  (V2R1_ERROR_BASE+10)

#define  V2R1_ONU_FILTER_IP_NOT_VALID  (V2R1_ERROR_BASE+11)
#define  V2R1_ONU_FILTER_IP_EXIST   (V2R1_ERROR_BASE+12) 
#define  V2R1_ONU_FILTER_IP_NOT_EXIST  (V2R1_ERROR_BASE+13)

#define  V2R1_ONU_FILTER_IP_UDP_EXIST  (V2R1_ERROR_BASE+14)
#define  V2R1_ONU_FILTER_IP_UDP_NOT_EXIST  (V2R1_ERROR_BASE+15)

#define V2R1_ONU_FILTER_IP_TCP_EXIST  (V2R1_ERROR_BASE+16)
#define V2R1_ONU_FILTER_IP_TCP_NOT_EXIST  (V2R1_ERROR_BASE+17)

#define  V2R1_ONU_FILTER_VLAN_ID_EXIST   (V2R1_ERROR_BASE+18)
#define  V2R1_ONU_FILTER_VLAN_ID_NOT_EXIST  (V2R1_ERROR_BASE+19)

#define  V2R1_ONU_FILTER_ETHER_TYPE_EXIST  (V2R1_ERROR_BASE+20)
#define  V2R1_ONU_FILTER_ETHER_TYPE_NOT_EXIST (V2R1_ERROR_BASE+21)

#define  V2R1_ONU_FILTER_IP_PROT_EXIST  (V2R1_ERROR_BASE+22)
#define  V2R1_ONU_FILTER_IP_PROT_NOT_EXIST  (V2R1_ERROR_BASE+23)


#define  V2R1_ONU_FILTER_IP_SOURCE   1
#define  V2R1_ONU_FILTER_IP_DEST  2

#define  V2R1_ONU_FILTER_IP_UDP_SOURCE  3
#define  V2R1_ONU_FILTER_IP_UDP_DEST  4

#define  V2R1_ONU_FILTER_IP_TCP_SOURCE  5
#define  V2R1_ONU_FILTER_IP_TCP_DEST  6


#define MAX_ONU_ALARM_NUM  32

/* ONU software update ctrl & status */
#define V2R1_UPDATE_SIGNAL_ONU   0
#define V2R1_UPDATE_ALL_ONU   1

#define ONU_SW_UPDATE_ENABLE   1
#define ONU_SW_UPDATE_DISABLE   2

#define ONU_SW_UPDATE_ACTION_UPDATE  2  /* this is an action */
#define ONU_SW_UPDATE_STATUS_NOOP  1  /* the three others are status */
#define ONU_SW_UPDATE_STATUS_INPROGRESS  3
#define ONU_SW_UPDATE_STATUS_FORBIDDEN  4

/*ONU update API ret code*/
#define ONU_UPDATE_ERR   (-1)
#define ONU_UPDATE_OK    0
#define ONU_UPDATE_INPROCESS   1
#define ONU_VERSION_IDENTICAL  2
#define ONU_FILE_NOEXIST       3
#define ONU_UPDATE_FORBIDDED   4

/* onu admin & operation status */
#define ONU_ADMIN_STATUS_UP  V2R1_ENABLE
#define ONU_ADMIN_STATUS_DOWN  V2R1_DISABLE

/*  modified by chenfj 2008-2-20
      扩展ONU 操作状态,增加调电和休眠
     { up(1), down(2), unknown(3) }，改为{ up(1), down(2), unknown(3), dormant(4), powerDown(5) }
*/

#define ONU_OPER_STATUS_MIN			0
#define ONU_OPER_STATUS_UP			1	/*V2R1_UP*/
#define ONU_OPER_STATUS_DOWN			2	/*V2R1_DOWN*/
#define ONU_OPER_STATUS_PENDING		3	/*V2R1_PENDING*/
#define ONU_OPER_STATUS_DORMANT		4
#define ONU_OPER_STATUS_POWERDOWN	5 
#define ONU_OPER_STATUS_MAX			6

/* onu first register flag */
#define ONU_FIRST_REGISTER_FLAG   0x5a
#define NOT_ONU_FIRST_REGISTER_FLAG  0xa5

#define ONU_USED_FLAG  0x5a
#define NOT_ONU_USED_FLAG  0xa5

#ifdef   V2R1_ENABLE
#define  ONU_REGISTER_TRAP  V2R1_ENABLE
#define  NO_ONU_REGISTER_TRAP  V2R1_DISABLE
#endif

/* onu battery & capability */

#define POWER_AND_BATTERY  1
#define POWER_NO_BATTERY  2

#define POWER_STATUS_RUNNING   1
#define POWER_STATUS_OFF  2

/* onu mgmt table entry row status */
#define   LLID_ENTRY_UNKNOWN 0
#define   LLID_ENTRY_ACTIVE  1
#define   LLID_ENTRY_NOT_IN_ACTIVE  2
#define   LLID_ENTRY_NOT_READY  3
#define   LLID_ENTRY_CREATE_AND_GO   4
#define   LLID_ENTRY_CREATE_AND_WAIT  5
#define   LLID_ENTRY_DESTORY    6

#define  MAXLLIDPERONU  1
#define  INVALID_LLID (-1)
#define  UNKNOWN_LLID   0

#define  LLIDTYPE_UNKNOWN 0
#define  LLIDTYPE_ETHLINK  1

#define  ADDRESS_STATIC  1
#define  ADDRESS_DYNAMIC  0

#define  V2R1_DELAY_HIGH 2
#define  V2R1_DELAY_LOW 1

#define  PRECEDENCE_OF_FLOW_0   0
#define  PRECEDENCE_OF_FLOW_1   1 
#define  PRECEDENCE_OF_FLOW_2   2
#define  PRECEDENCE_OF_FLOW_3   3
#define  PRECEDENCE_OF_FLOW_4   4
#define  PRECEDENCE_OF_FLOW_5   5
#define  PRECEDENCE_OF_FLOW_6   6
#define  PRECEDENCE_OF_FLOW_7   7

/* 2007-02-07 增加ONU自动配置*/
#ifdef  V2R1_ONU_AUTO_CONFIG
#define  V2R1_ONU_USING_AUTO_CONFIG  1
#define  V2R1_ONU_USING_LOCAL_DATA  2

#define  V2R1_ONU_INITIAL_STATUS  1
#define  V2R1_ONU_STARTUP_AUTO_CONFIG  2
#endif

#define  V2R1_ONU_AUTO_CONFIG_FLAG  1
#define  NO_V2R1_ONU_AUTO_CONFIG_FLAG  0

/* added by zhuyf, 2007.3.18 */
#define SUPPORTING     1
#define NOT_SUPPORTING 0

#define FE_INTERFACE 1
#define GE_INTERFACE 2
#define UNKNOW_INTERFACE 0

#define RPC_CMD_BUF_LEN 512

/* end */
#ifndef MAX_QUEUE_NUMBER
#define MAX_QUEUE_NUMBER		8
#endif

/*Begin:for onu swap by jinhl@2013-04-27*/
#define VIRREG_FAILTIMES        10   /*虚注册添加失败的最大次数*/
/*End:for onu swap by jinhl@2013-04-27*/


extern int EVENT_UPDATE_ONU_FILE;
#define ONU_UPDATE_TITLE      "\r\n[ONU-UPDATE:]"
#define ONU_UPDATE_DEBUG      if (V2R1_ENABLE == EVENT_UPDATE_ONU_FILE) sys_console_printf  

/*added by liyang 2014-11-19 for onu fixed bw range*/
#define TK_DBA_ECODE_FIXED_BW                 20
#define TKCOMM_FIXED_BW_ERROR                -30 
#if 0
typedef struct
{
    char password[OG_CMAPI_PASSWORD_LEN];
    int enablePm;
    uint32 flowProfileIndex;
    int tcontVirtualPortBindingType;
    uint32 tcontVirtualPortBindingProfileIndex;
    uint32 berIntervalConfiguration;
    int powerLevel;
    uint8 vendorID[OG_CMAPI_ONU_VENDOR_ID_LEN];
    uint8 version[OG_CMAPI_ONU_VERSION_LEN];
    uint8 serialNumber[OG_CMAPI_ONU_SERIAL_NUMBER_LEN];
    int batteryBackup;
    int adminState;
    int operationalState;
    uint8 equipmentID[OG_CMAPI_ONU_EQUIPMENT_ID_LEN];
    int omccVersion;
    uint32 onuHardwareType;
    uint32 onuHardwareRevision;
    uint32 totalPriorityQueueNumber;
    uint32 totalTrafficSchedulerNumber;
    uint32 totalGEMPortNumber;
    uint32 totalTCONTNumber;
    uint32 totalEthernetUNINumber;
    uint32 totalPOTSUNInumber;
    uint32 sysUpTime;
    uint8 onuImageInstance0Version[OG_CMAPI_ONU_IMAGE_INSTANCE0_VERSION];
    int onuImageInstance0Valid;
    int onuImageInstance0Activate;
    int onuImageInstance0Commit;
    uint8 onuImageInstance1Version[OG_CMAPI_ONU_IMAGE_INSTANCE0_VERSION];
    int onuImageInstance1Valid;
    int onuImageInstance1Activate;
    int onuImageInstance1Commit;
    uint8 onuMacAddress[OG_CMAPI_MAC_ADDR_HEX_LEN];
    int onuDHCPMode;
    uint32 onuIPAddress;
    uint32 onuIPMask;
    uint32 onuDefaultGateway;
    int onuReset;
    uint32 macLimit;
    uint32 fecTxEnable;
    uint32 onuFastLeaveCapability;
    uint32 rxpower;
    uint32 txpower;
    uint32 distance; /* Read-only Field. In KMs */
}tGponOnuConfig;
#endif

typedef struct
{
    unsigned char number_of_entries;
    CTC_STACK_voip_pots_status_array_t pots_status_array;
}CTC_STACK_voip_pots_status_array;
typedef struct
{
    unsigned char number_of_entries;
    CTC_STACK_h248_user_tid_config_array_t h248_user_tid_array;
}CTC_STACK_h248_user_tid_config_array;
typedef struct
{
    unsigned char number_of_entries;
    CTC_STACK_sip_user_param_config_array_t sip_user_param_array;
}CTC_STACK_sip_user_param_config_array;


/* B--added by shixh@2010-5-17 for ONUmgt-Api */
typedef enum{
    ONU_MANAGE_UNKNOWN = 0,

    ONU_MANAGE_CTC,
    ONU_MANAGE_GW,    
    ONU_MANAGE_GPON,
    ONU_MANAGE_MAX    
}ONU_MANAGE_TYPE;

typedef enum{
    ONU_ADAPTER_NULL = 0,
    ONU_ADAPTER_RPC  = 1,

    ONU_ADAPTER_PAS5001_GW,    
    ONU_ADAPTER_PAS5201_GW,    
    ONU_ADAPTER_PAS5204_GW,    
    ONU_ADAPTER_PAS8411_GW,    

    ONU_ADAPTER_PAS5201_CTC,    
    ONU_ADAPTER_PAS5204_CTC, 
    ONU_ADAPTER_PAS8411_CTC,    

    ONU_ADAPTER_TK3723_GW,
    ONU_ADAPTER_TK3723_CTC,
    ONU_ADAPTER_BCM55524_GW,
    ONU_ADAPTER_BCM55524_CTC,
    ONU_ADAPTER_BCM55538_GW,
    ONU_ADAPTER_BCM55538_CTC,

    ONU_ADAPTER_BCM_GPON,       /*added by luh for 16GPON support*/
    
    ONU_ADAPTER_GW,    
   
    ONU_ADAPTER_MAX    
}ONU_ADAPTER_TYPE;

typedef enum {
	mn_ctc_auth_mode_min,
	mn_ctc_auth_mode_mac = 1,
	mn_ctc_auth_mode_loid,
	mn_ctc_auth_mode_hybrid,
	mn_ctc_auth_mode_loid_no_pwd,
	mn_ctc_auth_mode_hybrid_no_pwd,
	mn_ctc_auth_mode_disable,
	mn_gpon_auth_mode_sn,
	mn_gpon_auth_mode_sn_and_pwd,
	mn_ctc_auth_mode_error,
	mn_ctc_auth_mode_max
} mn_ctc_auth_mode_t;

#define TOTAL_NUMBER_MAC_ADDRESS_ENTRY 100
typedef struct OnuPortLacationEntry
{
	unsigned char  result_get;
	unsigned char mac[6];
	short int vlan_id;
	unsigned char port_id;
	unsigned char mac_type;
}OnuPortLacationEntry_S;

typedef struct OnuPortLacationInfor
{
	unsigned int number_entry;
	OnuPortLacationEntry_S OnuPortMacTable[TOTAL_NUMBER_MAC_ADDRESS_ENTRY];
}OnuPortLacationInfor_S;
 
 typedef struct OnuPortStorm
 {
	 unsigned char storm_type;
	 unsigned char storm_mode;
	 unsigned int storm_rate;
 }OnuPortStorm_S;


/********************declear for gw_onu 2013-5-14*********************************/
#define MAX_CLASS_RULES_FOR_SINGLE_PATH 8 
#define MAX_PAS_RULES_FOR_SINGLE_PATH 8 

#define QOS_MAX_RULE_COUNT_PATH 2
#define QOS_RULE_UP_DIRECTION   0
#define QOS_RULE_DOWN_DIRECTION 1

#define QOS_MODE_PRIO_TRANS 0
#define QOS_MODE_PRIO_VID   1

#define BASE_ON_ETHER_TYPE 1
#define BASE_ON_IP_PROTOCAL 2
#define BASE_ON_VLAN_ID 3

#define PAS_MAX_RULE_COUNT_PATH 2
#define PAS_RULE_UP_DIRECTION   0
#define PAS_RULE_DOWN_DIRECTION 1

#define PAS_RULE_PRIO_SOURCE_CLASSIFIER  1
#define PAS_RULE_PRIO_SOURCE_ORIGINAL    2

#define PAS_RULE_ACTION_NONE     0
#define PAS_RULE_ACTION_ATTACH   1
#define PAS_RULE_ACTION_EXCHANGE 2

#define SET_QOS_RULE 1
#define CLR_QOS_RULE 2
#define SET_PAS_RULE 3
#define CLR_PAS_RULE 4

typedef struct
{
    unsigned char                           valid;
    unsigned char                           mode;
    unsigned char                           queue_mapped;
    unsigned char                           priority_mark;
    
    ULONG                                   value;
}__attribute__((packed)) gw_qos_classification_rule_t;

typedef struct
{    
    unsigned char                           valid;
    unsigned char                           mode;
    unsigned char                           action;
    unsigned char                           prio_source;

    unsigned short                          vlan_type;       
    unsigned short                          new_vid;    
    
    ULONG                                   value;      
}__attribute__((packed)) gw_pas_rule_t;

typedef  struct
{
    unsigned char                           priority_mode;
    unsigned char                           num_of_class; 
    gw_qos_classification_rule_t            class_entry[MAX_CLASS_RULES_FOR_SINGLE_PATH];    
}__attribute__((packed)) gw_qos_classification_rules_t;

typedef struct
{
    int                                     num_of_rule;            
    gw_pas_rule_t                           rule_entry[MAX_PAS_RULES_FOR_SINGLE_PATH];    
}__attribute__((packed)) gw_pas_rules_t;

typedef struct
{
    gw_qos_classification_rule_t            qos_rule;            
    gw_pas_rule_t                           pas_rule;    
}__attribute__((packed)) gw_rule_t;

/************************ end declear for gw_onu2013-5-14 *******************************/


/* 注意:顺序与数目必须与OnuMgmtIFs一一对应 */
typedef enum{
#if 1
/* -------------------ONU基本API------------------- */
    ONU_CMD_SPLIT_LINE_FRAME_BEGIN = 0,
    ONU_CMD_ONU_IS_VALID_1,
    ONU_CMD_ONU_IS_ONLINE_2,
    ONU_CMD_ADD_ONU_MANURAL_3,
    ONU_CMD_DEL_ONU_MANURAL_4,
    ONU_CMD_IS_SUPPORT_CMD_5,
    ONU_CMD_COPY_ONU_6,
    ONU_CMD_GET_IFTYPE_7,
    ONU_CMD_SET_IFTYPE_8,
    ONU_CMD_MODIFY_ONU_MANURAL_9,    
    ONU_CMD_ADD_GPON_ONU_MANURAL_10,
    ONU_CMD_SPLIT_LINE_FRAME_END,
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    ONU_CMD_SPLIT_LINE_AUTH_FUN_BEGIN = 100,
    ONU_CMD_DEREGISTERT_ONU_1,       /* ONU 去注册*/
    ONU_CMD_SET_MAC_AUTH_MODE_2,       
    ONU_CMD_DEL_BIND_ONU_3,          /* 删除绑定在其他端口的ONU */
    ONU_CMD_ADD_PENDING_ONU_4,       /* 添加pending onu */
    ONU_CMD_DEL_PENDING_ONU_5,       /* 删除pending onu */
    ONU_CMD_DEL_CONF_PENDING_ONU_6,  /*  删除注册冲突的ONU  */
    ONU_CMD_AUTHORIZE_ONU_7,
    ONU_CMD_SPLIT_LINE_AUTH_FUN_END,
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    ONU_CMD_SPLIT_LINE_SERVICE_FUN_BEGIN = 200,
    ONU_CMD_SET_TRAFFIC_SERVICE_MODE_1,
    ONU_CMD_SET_ONU_PEER_TO_PEER_2,
    ONU_CMD_SET_ONU_PEER_TO_PEER_FORWARD_3,
    ONU_CMD_SET_ONU_BW_4,
    ONU_CMD_GET_ONU_SLA_5,

    ONU_CMD_SET_FEC_MODE_6,
    ONU_CMD_GET_ONU_VLAN_MODE_7,     /* 获取ONU VLAN 模式*/
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    ONU_CMD_SET_UNI_PORT_8,
    ONU_CMD_SET_SLOW_PROTOCOL_LIMIT_9,
    ONU_CMD_GET_SLOW_PROTOCOL_LIMIT_10,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    ONU_CMD_GET_ONU_BWINFO_11,
    ONU_CMD_GET_B2P_MODE_12,
    ONU_CMD_SET_B2P_MODE_13,

    ONU_CMD_SPLIT_LINE_SERVICE_FUN_END,
#endif

#if 1
/* -------------------ONU 监控统计管理API------------------- */
    ONU_CMD_SPLIT_LINE_MONITOR_FUN_BEGIN = 300,
    ONU_CMD_RESET_COUNTER_1,
    ONU_CMD_SET_PON_LOOPBACK_2,
    ONU_CMD_SPLIT_LINE_MONITOR_FUN_END,
#endif

#if 1
/* -------------------ONU 加密管理API----------- */
    ONU_CMD_SPLIT_LINE_ENCRYPT_FUN_BEGIN = 400,
    ONU_CMD_GET_LLID_PARAMS_1,       /*  获取ONU 加密模式*/
    ONU_CMD_START_ENCRYPTION_2,      /* ONU 开始加密*/
    ONU_CMD_STOP_ENCRYPTION_3,       /* ONU 停止加密*/
    ONU_CMD_SET_ONU_ENCRYPT_PARAMS_4,/* 设置ONU 加密参数*/
    ONU_CMD_GET_ONU_ENCRYPT_PARAMS_5,/* 获取ONU 加密参数*/
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ONU_CMD_UPDATE_ENCRYPTION_KEY_6,/* 更新加密KEY*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ONU_CMD_SPLIT_LINE_ENCRYPT_FUN_END,
#endif

#if 1
/* -------------------ONU 地址表管理API-------- */
    ONU_CMD_SPLIT_LINE_ADDRTBL_FUN_BEGIN = 500,
    ONU_CMD_GET_ONU_MAC_ADDTBL_1,    /* ONU 的MAC 学习地址*/
    ONU_CMD_GET_OLT_MAC_ADDTBL_2,    /* ONU 在OLT上的MAC 学习地址*/
    ONU_CMD_SET_MAX_MAC_3,           /* ONU 最大MAC地址数*/
    ONU_CMD_GET_ONU_UNI_CONFIG_4,    /* 获取ONU UNI 的端口配置*/
    ONU_CMD_GET_ONU_MAC_CHECK_FLAG_5,
    ONU_CMD_GET_ONU_ALL_PORT_MAC_COUNTER_6,
    ONU_CMD_GET_OLT_MAC_VLAN_ADDTBL_7,    /* ONU 在OLT上的MAC 学习地址,用于GPON，获取mac+vid by jinhl*/
    ONU_CMD_SPLIT_LINE_ADDRTBL_FUN_END,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    ONU_CMD_SPLIT_LINE_OPTICS_FUN_BEGIN = 600,
    ONU_CMD_GET_ONU_DISTANCE_1,
    ONU_CMD_GET_OPTICAL_CAPABILITY_2,
    ONU_CMD_SPLIT_LINE_OPTICS_FUN_END,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    ONU_CMD_SPLIT_LINE_HOTSWAP_FUN_BEGIN = 700,
    ONU_CMD_SET_ONU_LLID_1,
    ONU_CMD_SPLIT_LINE_HOTSWAP_FUN_END,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    ONU_CMD_SPLIT_LINE_DEVICE_FUN_BEGIN = 800,
    ONU_CMD_GET_ONU_VER_1,           /* 获取ONU的版本*/
    ONU_CMD_GET_PON_VER_2,           /* 获取ONU 的PON VER */
    ONU_CMD_GET_ONU_INFO_3,
    ONU_CMD_GET_ONU_I2C_INFO_4,
    ONU_CMD_SET_ONU_I2C_INFO_5,
    
    ONU_CMD_RESET_ONU_6,
    ONU_CMD_SET_ONU_SW_UPDATE_MODE_7,
    ONU_CMD_ONU_SW_UPDATE_8,
    ONU_CMD_GW_CTC_SW_CONVERT_9,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    ONU_CMD_Get_Burn_Image_Complete_10,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    
    ONU_CMD_SET_ONU_DEVICE_NAME_11,
    ONU_CMD_SET_ONU_DEVICE_DESC_12,
    ONU_CMD_SET_ONU_DEVICE_LOCATION_13,
    ONU_CMD_GET_ONU_ALL_PORT_STATISTIC_DATA_14,    
    ONU_CMD_SPLIT_LINE_DEVICE_FUN_END,
#endif

#if 1
/* --------------------ONU CTC协议管理API------------------- */
    ONU_CMD_SPLIT_LINE_CTC_FUN_BEGIN = 900,
    ONU_CMD_CTC_GET_CTC_VERSION_1,
    ONU_CMD_CTC_GET_FIRMWARE_VERSION_2,
    ONU_CMD_CTC_GET_SERIAL_NUMBER_3,
    ONU_CMD_CTC_GET_CHIPSET_ID_4,

    ONU_CMD_CTC_GET_ONU_CAP1_5,
    ONU_CMD_CTC_GET_ONU_CAP2_6,
    ONU_CMD_CTC_GET_ONU_CAP3_7,

    ONU_CMD_CTC_UPDATE_FIRMWARE_8,
    ONU_CMD_CTC_ACTIVE_FIRMWARE_9,
    ONU_CMD_CTC_COMMIT_FIRMWARE_10,
    
    ONU_CMD_CTC_START_ENCRYPTION_11,
    ONU_CMD_CTC_STOP_ENCRYPTION_12,
    
    ONU_CMD_CTC_GET_ETHPORT_LINKSTATE_13,
    ONU_CMD_CTC_GET_ETHPORT_ADMIN_STATUS_14,
    ONU_CMD_CTC_SET_ETHPORT_ADMIN_STATUS_15,
    
    ONU_CMD_CTC_GET_ETHPORT_PAUSE_16,
    ONU_CMD_CTC_SET_ETHPORT_PAUSE_17,
    
    ONU_CMD_CTC_GET_ETHPORT_AUTO_NEGOTIATION_ADMIN_18,
    ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_ADMIN_19,
    ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_RESTART_20,
    ONU_CMD_CTC_GET_ETHPORT_AN_LOCAL_TECABILITY_21,
    ONU_CMD_CTC_GET_ETHPORT_AN_ADVERTISED_TECABILITY_22,
    
    ONU_CMD_CTC_GET_ETHPORT_POLICING_23,
    ONU_CMD_CTC_SET_ETHPORT_POLICING_24,
    ONU_CMD_CTC_GET_ETHPORT_DOWNSTREAM_POLICING_25,
    ONU_CMD_CTC_SET_ETHPORT_DOWNSTREAM_POLICING_26,
    
    ONU_CMD_CTC_GET_ETHPORT_VCONF_27,
    ONU_CMD_CTC_SET_ETHPORT_VCONF_28,
    ONU_CMD_CTC_GET_ALLPORT_VCONF_29,

    ONU_CMD_CTC_GET_ETHPORT_CLASSIFICATIOANDMARK_30,
    ONU_CMD_CTC_SET_ETHPORT_CLASSIFICATIOANDMARK_31,
    ONU_CMD_CTC_CLR_ETHPORT_CLASSIFICATIOANDMARK_32,
    
    ONU_CMD_CTC_GET_ETHPORT_MULTICAST_VLAN_33,
    ONU_CMD_CTC_SET_ETHPORT_MULTICAST_VLAN_34,
    ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_VLAN_35,
    
    ONU_CMD_CTC_GET_ETHPORT_MULTICAST_MAX_GROUP_36,
    ONU_CMD_CTC_SET_ETHPORT_MULTICAST_MAX_GROUP_37,
    
    ONU_CMD_CTC_GET_ETHPORT_MULTICAST_TAG_STRIP_38,
    ONU_CMD_CTC_SET_ETHPORT_MULTICAST_TAG_STRIP_39,
    ONU_CMD_CTC_GET_ALLPORT_MULTICAST_TAG_STRIP_40,
    
    ONU_CMD_CTC_GET_ETHPORT_IGMP_TAG_OPER_41,
    ONU_CMD_CTC_SET_ETHPORT_IGMP_TAG_OPER_42,
    ONU_CMD_CTC_SET_OBJ_IGMP_TAG_OPER_43,
    
    ONU_CMD_CTC_GET_ETHPORT_MULTICAST_CONTROL_44,
    ONU_CMD_CTC_SET_ETHPORT_MULTICAST_CONTROL_45,
    ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_CONTROL_46,
    
    ONU_CMD_CTC_GET_MULTICAST_SWITCH_47,
    ONU_CMD_CTC_SET_MULTICAST_SWITCH_48,
    
    ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_ABILITY_49,
    ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_50,
    ONU_CMD_CTC_SET_MULTICAST_FASTLEAVE_51,
    
    ONU_CMD_CTC_GET_PORT_STATISTIC_DATA_52,
    ONU_CMD_CTC_GET_PORT_STATISTIC_STATE_53,
    ONU_CMD_CTC_SET_PORT_STATISTIC_STATE_54,

    ONU_CMD_CTC_SET_ALARM_ADMIN_STATE_55,
    ONU_CMD_CTC_SET_ALARM_THRESOLD_56,
    ONU_CMD_CTC_GET_DBA_REPORT_THRESOLD_57,
    ONU_CMD_CTC_SET_DBA_REPORT_THRESOLD_58,

    ONU_CMD_CTC_GET_MNG_GLOBAL_CONFIG_59,
    ONU_CMD_CTC_SET_MNG_GLOBAL_CONFIG_60,
    ONU_CMD_CTC_GET_MNG_SNMP_CONFIG_61,
    ONU_CMD_CTC_SET_MNG_SNMP_CONFIG_62,

    ONU_CMD_CTC_GET_HOLDOVER_63,
    ONU_CMD_CTC_SET_HOLDOVER_64,
    ONU_CMD_CTC_GET_OPTIC_TRAN_DIAG_65,
    ONU_CMD_CTC_SET_ONU_POWER_SUPPLY_CONTROL_66,

    ONU_CMD_CTC_GET_ONU_FEC_ABILITY_67,
    
    ONU_CMD_GET_IADINFO_68,
    ONU_CMD_GET_VOIP_IAD_OPER_69,
    ONU_CMD_SET_VOIP_IAD_OPER_70,
    ONU_CMD_GET_VOIP_GLOBAL_CONFIG_71,
    ONU_CMD_SET_VOIP_GLOBAL_CONFIG_72,
    ONU_CMD_GET_VOIP_FAX_CONFIG_73,
    ONU_CMD_SET_VOIP_FAX_CONFIG_74,
    
    ONU_CMD_GET_VOIP_POTS_STATUS_75,
    ONU_CMD_GET_VOIP_PORT_OPER_76,
    ONU_CMD_SET_VOIP_PORT_OPER_77,
    ONU_CMD_GET_VOIP_PORT2_OPER_78,
    ONU_CMD_SET_VOIP_PORT2_OPER_79,

    ONU_CMD_GET_H248_CONFIG_80,
    ONU_CMD_SET_H248_CONFIG_81,
    ONU_CMD_GET_H248_USERTID_CONFIG_82,
    ONU_CMD_SET_H248_USERTID_CONFIG_83,
    ONU_CMD_GET_H248_RTPTID_CONFIG_84,
    ONU_CMD_SET_H248_RTPTID_CONFIG_85,
    
    ONU_CMD_GET_SIP_CONFIG_86,
    ONU_CMD_SET_SIP_CONFIG_87,
    ONU_CMD_SET_SIP_DIGITMAP_88,
    ONU_CMD_GET_SIP_USERTID_CONFIG_89,
    ONU_CMD_SET_SIP_USERTID_CONFIG_90,

	ONU_CMD_CTC_GET_PORT_STATS_DATA_91,

    ONU_CMD_SPLIT_LINE_CTC_FUN_END,
#endif


#if 1
/* -------------------ONU 远程管理API------------------- */
    ONU_CMD_SPLIT_LINE_REMOTE_FUN_BEGIN = 1200,
    ONU_CMD_CLI_CALL_1,
    ONU_CMD_SET_MGT_RESET_2,
    ONU_CMD_SET_MGT_CONFIG_3,
    ONU_CMD_SET_MGT_LASER_4,
    ONU_CMD_SET_TEMPERATURE_5,
    ONU_CMD_SET_PAS_FLUSH_6,
    
    ONU_CMD_SET_ATU_AGING_TIME_7,
    ONU_CMD_SET_ATU_LIMIT_8,

    ONU_CMD_SET_PORT_LINK_MON_9,
    ONU_CMD_SET_PORT_MODE_MON_10,
    ONU_CMD_SET_PORT_ISOLATE_11,
    
    ONU_CMD_SET_VLAN_ENABLE_12,
    ONU_CMD_SET_VLAN_MODE_13,
    ONU_CMD_ADD_VLAN_14,
    ONU_CMD_DEL_VLAN_15,
    ONU_CMD_SET_PORT_PVID_16,
    
    ONU_CMD_ADD_VLAN_PORT_17,
    ONU_CMD_DEL_VLAN_PORT_18,
    ONU_CMD_SET_VLAN_TRANSATION_19, /*VLAN transation add by shixh20110609*/
    ONU_CMD_DEL_VLAN_TRANSATION_20,
    ONU_CMD_SET_VLAN_AGGREGATION_21,/*VLAN aggregation*/
    ONU_CMD_DEL_VLAN_AGGREGATION_22,
    
    ONU_CMD_SET_QINQ_ENABLE_23,
    ONU_CMD_ADD_QINQ_TAG_24,
    ONU_CMD_DEL_QINQ_TAG_25,
    
    ONU_CMD_SET_PORT_VLAN_FRAME_ACC_26,
    ONU_CMD_SET_PORT_INGRESS_VLAN_FILTER_27,

    ONU_CMD_SET_PORT_MODE_28,
    ONU_CMD_SET_PORT_FC_MODE_29,
    ONU_CMD_SET_PORT_ATU_LEARN_30,
    ONU_CMD_SET_PORT_ATU_FLOOD_31,
    ONU_CMD_SET_PORT_LOOP_DETECT_32,
    ONU_CMD_SET_PORT_STAT_FLUSH_33,

    ONU_CMD_SET_INGRESS_RATE_LIMIT_BASE_34,
    ONU_CMD_SET_PORT_INGRESS_RATE_35,
    ONU_CMD_SET_PORT_EGRESS_RATE_36,
    
    ONU_CMD_SET_QOS_CLASS_37,
    ONU_CMD_CLR_QOS_CLASS_38,
    ONU_CMD_SET_QOS_RULE_39,
    ONU_CMD_CLR_QOS_RULE_40,
    
    ONU_CMD_SET_PORT_QOS_RULE_41,
    ONU_CMD_CLR_PORT_QOS_RULE_42,
    ONU_CMD_SET_PORT_QOS_RULE_TYPE_43,
    
    ONU_CMD_SET_PORT_DEF_PRIORITY_44,
    ONU_CMD_SET_PORT_NEW_PRIORITY_45,
    ONU_CMD_SET_QOS_PRIORITY_TO_QUEUE_46,
    ONU_CMD_SET_QOS_DSCP_TO_QUEUE_47,
    
    ONU_CMD_SET_PORT_QOS_USERPRI_ENABLE_48,
    ONU_CMD_SET_PORT_QOS_IPPRI_ENABLE_49,
    ONU_CMD_SET_QOS_ALGORITHM_50,
    ONU_CMD_SET_QOS_MODE_51,
    ONU_CMD_SET_RULE_52,
    
    ONU_CMD_SET_IGMP_ENABLE_53,
    ONU_CMD_SET_IGMP_AUTH_54,
    ONU_CMD_SET_IGMP_HOST_AGE_55,
    ONU_CMD_SET_IGMP_GROUP_AGE_56,
    ONU_CMD_SET_IGMP_MAX_RESTIME_57,
    
    ONU_CMD_SET_IGMP_MAX_GROUP_58,
    ONU_CMD_ADD_IGMP_GROUP_59,
    ONU_CMD_DEL_IGMP_GROUP_60,
    ONU_CMD_SET_PORT_MULTICAST_FASTLEAVE_61,
    ONU_CMD_SET_PORT_MULTICAST_VLAN_62,
    
    ONU_CMD_SET_PORT_MIRROR_FROM_63,
    ONU_CMD_SET_PORT_MIRROR_TO_64, 
    ONU_CMD_DEL_MIRROR_65,

    ONU_CMD_SPLIT_LINE_REMOTE_FUN_END,
#endif


#if 1
/* --------------------ONU CMC协议管理API------------------- */
    ONU_CMD_SPLIT_LINE_CMC_FUN_BEGIN = 1500,

    ONU_CMD_CMC_REGISTER_CMC_1,
    ONU_CMD_CMC_UNREGISTER_CMC_2,
    ONU_CMD_CMC_DUMP_CMC_3,
    ONU_CMD_CMC_DUMP_ALARM_4,
    ONU_CMD_CMC_DUMP_LOG_5,
    
    ONU_CMD_CMC_RESET_CMC_BOARD_6,
    ONU_CMD_CMC_GET_CMC_VERSION_7,
    ONU_CMD_CMC_GET_CMC_MAXMULTICAST_8,
    ONU_CMD_CMC_GET_CMC_MAXCM_9,
    ONU_CMD_CMC_SET_CMC_MAXCM_10,
    
    ONU_CMD_CMC_GET_CMC_TIME_11,
    ONU_CMD_CMC_SET_CMC_TIME_12,
    ONU_CMD_CMC_LOCAL_CMC_TIME_13,
    ONU_CMD_CMC_SET_CMC_CUSTOM_CONFIG_14,
    ONU_CMD_CMC_DUMP_CMC_CUSTOM_CONFIG_15,
    
    ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_16,
    ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_17,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODE_18,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODE_19,
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_MODE_20,
    
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_MODE_21,
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_D30MODE_22,
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_D30MODE_23,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_FREQ_24,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_FREQ_25,
    
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_FREQ_26,
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_FREQ_27,
    ONU_CMD_CMC_GET_CMC_DOWN_AUTO_FREQ_28,
    ONU_CMD_CMC_SET_CMC_DOWN_AUTO_FREQ_29,
    ONU_CMD_CMC_GET_CMC_UP_AUTO_FREQ_30,
    
    ONU_CMD_CMC_SET_CMC_UP_AUTO_FREQ_31,
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_WIDTH_32,
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_WIDTH_33,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_ANNEXMODE_34,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_ANNEXMODE_35,
    
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_TYPE_36,
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_TYPE_37,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODULATION_38,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODULATION_39,
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_PROFILE_40,
    
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_PROFILE_41,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_INTERLEAVER_42,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_INTERLEAVER_43,
    ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_POWER_44,
    ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_POWER_45,
    
    ONU_CMD_CMC_GET_CMC_UP_CHANNEL_POWER_46,
    ONU_CMD_CMC_SET_CMC_UP_CHANNEL_POWER_47,
    ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_POWER_48,
    ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_SIGNAL_QUALITY_49,
    ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_UTILITY_50,
    
    ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_STATISTICS_51,
    ONU_CMD_CMC_DUMP_CMC_IF_MAC_STATISTICS_52,
    ONU_CMD_CMC_DUMP_CMC_IF_ALL_STATISTICS_53,
    ONU_CMD_CMC_DUMP_CMC_GROUP_ALL_54,
    ONU_CMD_CMC_DUMP_CMC_GROUP_55,
    
    ONU_CMD_CMC_DUMP_GROUP_DOWN_CHANNEL_56,
    ONU_CMD_CMC_DUMP_GROUP_UP_CHANNEL_57,
    ONU_CMD_CMC_DUMP_GROUP_DYNAMIC_CONFIG_58,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_METHOD_59,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_PERIOD_60,
    
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_WEIGHT_PERIOD_61,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_OVERLOAD_THRESOLD_62,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_DIFF_THRESOLD_63,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_MAX_MOVE_64,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_MIN_HOLD_65,
    
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_RANGE_MODE_66,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DCC_TECH_67,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DCC_TECH_68,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DBC_TECH_69,
    ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DBC_TECH_70,
    
    ONU_CMD_CMC_CREATE_GROUP_71,
    ONU_CMD_CMC_DESTROY_GROUP_72,
    ONU_CMD_CMC_ADD_GROUP_DOWN_CHANNEL_73,
    ONU_CMD_CMC_RMV_GROUP_DOWN_CHANNEL_74,
    ONU_CMD_CMC_ADD_GROUP_UP_CHANNEL_75,
    
    ONU_CMD_CMC_RMV_GROUP_UP_CHANNEL_76,
    ONU_CMD_CMC_ADD_GROUP_MODEM_77,
    ONU_CMD_CMC_RMV_GROUP_MODEM_78,
    ONU_CMD_CMC_ADD_GROUP_EXCLUDE_MODEM_79,
    ONU_CMD_CMC_RMV_GROUP_EXCLUDE_MODEM_80,
    
    ONU_CMD_CMC_DUMP_GROUP_MODEM_81,
    ONU_CMD_CMC_DUMP_GROUP_ACTIVE_MODEM_82,
    ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_MODEM_83,
    ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_ACTIVE_MODEM_84,
    ONU_CMD_CMC_RSV_GROUP_85,
    
    ONU_CMD_CMC_DUMP_ALL_MODEM_86,
    ONU_CMD_CMC_DUMP_MODEM_87,
    ONU_CMD_CMC_DUMP_ALL_MODEM_HISTORY_88,
    ONU_CMD_CMC_DUMP_MODEM_HISTORY_89,
    ONU_CMD_CMC_CLEAR_ALL_MODEM_HISTORY_90,
    
    ONU_CMD_CMC_RESET_MODEM_91,
    ONU_CMD_CMC_DUMP_MODEM_DOWN_CHANNEL_92,
    ONU_CMD_CMC_DUMP_MODEM_UP_CHANNEL_93,
    ONU_CMD_CMC_SET_MODEM_DOWN_CHANNEL_94,
    ONU_CMD_CMC_SET_MODEM_UP_CHANNEL_95,
    
    ONU_CMD_CMC_CREATE_MODEM_SERVICE_FLOW_96,
    ONU_CMD_CMC_MODIFY_MODEM_SERVICE_FLOW_97,
    ONU_CMD_CMC_DESTROY_MODEM_SERVICE_FLOW_98,
    ONU_CMD_CMC_DUMP_MODEM_CLASSIFIER_99,
    ONU_CMD_CMC_DUMP_MODEM_SERVICE_FLOW_100,
    
    ONU_CMD_CMC_DUMP_CMC_CLASSIFIER_101,
    ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_102,
    ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_STATISTICS_103,
    ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_GROUP_104,
    ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_GROUP_105,
    
    ONU_CMD_CMC_CREATE_SERVICE_FLOW_CLASS_NAME_106,
    ONU_CMD_CMC_DESTROY_SERVICE_FLOW_CLASS_NAME_107,
    ONU_CMD_CMC_GET_CMC_MAC_ADDTBL_108,
    ONU_CMD_CMC_GET_CM_MAC_ADDTBL_109,
    ONU_CMD_CMC_RESET_CM_MAC_ADDTBL_110,
   
    ONU_CMD_SPLIT_LINE_CMC_FUN_END,
#endif
	ONU_CMD_CMC_Set_Multicast_Template_112,
	ONU_CMD_CMC_Get_Multicast_Template_113,
	ONU_CMD_CMC_Set_Mcast_OperProfile_114,
	ONU_CMD_CMC_Get_Mcast_OperProfile_115,
	ONU_CMD_CMC_SetUniPortAssociateMcastProf_116,
	ONU_CMD_CMC_GetUniPortAssociateMcastProf_117,
	ONU_CMD_Set_Port_Isolation_118,
	ONU_CMD_Get_Port_Isolation_119,
	ONU_CMD_Get_Onu_Mac_Entry_120,
	ONU_CMD_Set_Onu_Port_Save_Config_121,
	ONU_CMD_Set_Onu_Loop_DetectionTime_122,
	ONU_CMD_Get_Onu_Loop_DetectionTime_123,
	ONU_CMD_Set_Onu_Port_Mode_124,
	ONU_CMD_Get_Onu_Port_Mode_125,
	ONU_CMD_Set_Onu_Port_StormStatus_126,
	ONU_CMD_Get_Onu_Port_StormStatus_127,
	ONU_CMD_Set_Onu_DeviceLocation_128,
	ONU_CMD_Get_Onu_DeviceLocation_129,
	ONU_CMD_Set_Onu_DeviceDescription_130,
	ONU_CMD_Get_Onu_DeviceDescription_131,
	ONU_CMD_Set_Onu_DeviceName_132,
	ONU_CMD_Get_Onu_DeviceName_133,
	ONU_CMD_Get_Onu_PortMacNumber_134,
	ONU_CMD_Get_Onu_PortLocationByMAC_135,
	ONU_CMD_Get_Onu_ExtendAttribute_136,

#if 1
/* --------------------ONU DOCSIS应用管理API------------------- */
    ONU_CMD_SPLIT_LINE_DOCSIS_FUN_BEGIN = 2000,

    ONU_CMD_SPLIT_LINE_DOCSIS_FUN_END,
#endif

#if 1
/*----------------------GPON OMCI--------------------------*/
    ONU_CMD_SPLIT_LINE_OMCI_FUN_BEGIN = 3000,
    ONU_CMD_SPLIT_LINE_OMCI_GET_ONU_CFG,
    ONU_CMD_SPLIT_LINE_OMCI_SET_ONU_CFG,
    ONU_CMD_SPLIT_LINE_OMCI_FUN_END,

#endif
/* --------------------END------------------------------- */

    ONU_CMD_MAX
}ONU_CMD_ID;

typedef struct
{
	INT64U SendStreamOctets;  /*发送字节数*/
	INT64U ReceiveStreamOctets; /*接收字节数*/
}OnuPortStats_ST; 

typedef struct
{
    ULONG OnuPortNum;
    ULONG MacNum[32];
}OnuEthPortCounter_t;

#define ONU_ASSERT(onuid)       VOS_ASSERT( ((onuid) >= 0) && ((onuid) < MAXONUPERPON) )
#define ONU_CMD_ASSERT(onu_cmd) VOS_ASSERT( ((onu_cmd) >= 0) && ((onu_cmd) < ONU_CMD_MAX) )

struct stOnuMgmtIFs;
extern void ONU_RegisterAdapter(ONU_ADAPTER_TYPE adater_type, struct stOnuMgmtIFs *onu_ifs);
/* E--added by shixh@2010-5-17 for ONUmgt-Api */

typedef struct {
	short int PonPortIdx;
	short int OnuIdx;
	int file_len;
	unsigned char file_name[20];
}__attribute__((packed))fileName_t;

typedef struct 
{
    UCHAR ucOltType;  /* 0x61、0x67、0x69、0x80 、0x81*/
    UCHAR ucOltSlot;
    UCHAR ucOltPort;
    UCHAR ucOnuId;
}onu_local_info_t;

typedef struct 
{
    unsigned short int info_type;
    UCHAR info_Id;
    unsigned short int info_len;
    onu_local_info_t info;
}__attribute__ ((packed))onu_local_info_all_t;

typedef struct OnuStatisticDataPerPort
{
	INT64U downStreamOctets;
	INT64U upStreamOctets;
}OnuStatisticDataPerPort_S;

typedef struct OnuStatisticData
{
    int portNum;
	OnuStatisticDataPerPort_S data[33];
}OnuStatisticData_S;
#if 0 
#ifndef __sysDateAndTime_t	
#define __sysDateAndTime_t
typedef struct {
	ushort_t year;
	uchar_t  month;
	uchar_t  day;
	uchar_t  hour;
	uchar_t  minute;
	uchar_t  second;
	uchar_t  reserver[4];
}__attribute__((packed)) sysDateAndTime_t;
#endif
#endif

typedef struct OnuConfigInfo{
	char VendorInfo[MAXVENDORINFOLEN]; 
	char DeviceName[MAXDEVICENAMELEN];
	char DeviceDesc[MAXDEVICEDESCLEN];
	char Location[MAXLOCATIONLEN];
	UCHAR VendorInfoLen;
	UCHAR DeviceNameLen;
	UCHAR DeviceDescLen;
	UCHAR LocationLen;
	UCHAR MacAddr[6];

	unsigned short int OnuIdx;
	unsigned char  MaxMAC;

	unsigned char  AdminStatus;
	unsigned char  OperStatus;
	unsigned char SoftwareUpdateCtrl; 
	unsigned char  SoftwareUpdateFlag;

	/*1g/1g epon onu*/
	unsigned int  UplinkBandwidth;	
	unsigned int  DownlinkBandwidth;
	/* added by chenfj 2006-12-21 */
	unsigned int  UplinkBandwidthBe;
	unsigned int  DownlinkBandwidthBe;

	/*10g/1g asymmetric epon onu*/
	unsigned int  UplinkBandwidth_XGEPON_ASYM;	
	unsigned int  UplinkBandwidthBe_XGEPON_ASYM;
	unsigned int  DownlinkBandwidth_XGEPON_ASYM;
	unsigned int  DownlinkBandwidthBe_XGEPON_ASYM;

	/*10g/10g symmetric epon onu*/
	unsigned int  UplinkBandwidth_XGEPON_SYM;	
	unsigned int  UplinkBandwidthBe_XGEPON_SYM;
	unsigned int  DownlinkBandwidth_XGEPON_SYM;
	unsigned int  DownlinkBandwidthBe_XGEPON_SYM;

	/*gpon onu*/
	unsigned int  UplinkBandwidth_GPON;	
	unsigned int  UplinkBandwidthBe_GPON;
	unsigned int  DownlinkBandwidth_GPON;
	unsigned int  DownlinkBandwidthBe_GPON;

	UCHAR  UplinkClass;
	UCHAR  UplinkDelay;
	UCHAR  DownlinkClass;	
	UCHAR  DownlinkDelay;
	
	UCHAR  EncryptEnable;	
	UCHAR  EncryptStatus;
	UCHAR  EncryptDirection ; 
	unsigned int  EncryptKeyTime;   

	unsigned long AlarmMask;

	/* 2007/6/1 增加ONU 数据包原MAC 过滤*/
	MacTable_S  *StaticMac;	
	MacTable_S  *FilterSaMac;

	/* 2007/6/8 增加ONU 数据包IP/port 过滤*/
	IpTable_S   *Filter_SIp;
	IpTable_S   *Filter_DIp;
	Ip_Port_Table_S  *Filter_SIp_udp;
	Ip_Port_Table_S  *Filter_DIp_udp;
	Ip_Port_Table_S  *Filter_SIp_tcp;
	Ip_Port_Table_S  *Filter_DIp_tcp;

	/* 2007/6/12 增加ONU数据包VID过滤*/
	VlanId_Table_S  *Filter_Vid;

	/* 2007-6-14   增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤 */
	Ether_Type_Table_S  *Filter_Ether_Type;
	Ip_Type_Table_S  *Filter_Ip_Type;
	

	UCHAR AutoConfig;

	UCHAR  CTC_EncryptCtrl;   /* 2 - start, 3 - stop */ 
	UCHAR  CTC_EncryptStatus;  /* 1 - noop,  4 - encrypting */

	UCHAR FEC_ability;
	UCHAR FEC_mode;

}OnuConfigInfo_S;

typedef struct OnuMap{
	short int OnuIdx;
	unsigned char MacAddr[6];	
}OnuMap_S;

#ifdef	CTC_EXT_OID
/*声明CTC扩充的每个LLID上的DBA设置描述*/
typedef struct{
	ULONG	DbaReportBitMap;
	ULONG    DbaQueueThreathold[8];
	/*
	ULONG	DbaQueue0Threathold;
	ULONG	DbaQueue1Threathold;
	ULONG	DbaQueue2Threathold;
	ULONG	DbaQueue3Threathold;
	ULONG	DbaQueue4Threathold;
	ULONG	DbaQueue5Threathold;
	ULONG	DbaQueue6Threathold;
	ULONG	DbaQueue7Threathold;
	*/
}__attribute__((packed)) _queset_t;

#endif

/* modified by xieshl 20101228, 为了避免板间同步报文分片，压缩部分变量定义 */
typedef struct OnuLLIDTable{

	UCHAR  EntryStatus;
	UCHAR  BandWidthIsDefault;
	UCHAR  OnuEventRegisterTimeout;
	UCHAR  OnuhandlerStatus;        /* ONU 注册事件状态机控制变量2012-5-2*/
	    
#if 0    
	UCHAR  LlidDescLen;
	UCHAR  LlidDesc[MAXLLIDDESCLEN];
#endif
	USHORT Llid;
	USHORT MaxMAC;

	ULONG  UplinkBandwidth_gr;  /* bandwidth, unit: Kbit/s*/
	ULONG  UplinkBandwidth_be;  /* unit: Kbit/s */
	ULONG  DownlinkBandwidth_gr ; 
	ULONG  DownlinkBandwidth_be ; 
#ifdef  PLATO_DBA_V3
	ULONG  UplinkBandwidth_fixed;
#endif
	/* added by chenfj 2006-12-21*/
	UCHAR  UplinkClass;
	UCHAR  UplinkDelay;
	UCHAR  DownlinkClass;
	UCHAR  DownlinkDelay;
	
	ULONG  ActiveUplinkBandwidth;
	ULONG  ActiveDownlinkBandwidth;

	UCHAR  llidOltBoard;
	UCHAR  llidOltPort;
	UCHAR  llidOnuBoard;
	UCHAR  llidOnuPort;
    
	UCHAR  LlidType;
#ifdef	CTC_EXT_OID
/*	added by wangxy 2007-03-19  */
	UCHAR  llidCtcFecAbility;
	UCHAR  llidCtcFecMode;
	UCHAR  llidCtcEncrypCtrl;
#if 0    
	UCHAR  llidCtcDbaQuesetNum;
	UCHAR  llidCtcDbaQuesetCfgStatus;
	
	_queset_t llidCtcDbaQueset[MAX_QUEUE_NUMBER];
#endif	
#endif	

}__attribute__((packed))OnuLLIDTable_S;

typedef union{
	struct{
		int result:1;
		int year:5;
		int month:4;
		int day:5;
		int hour:5;
		int minute:6;
		int second:6;
		}st_clk;
	long clk;
}onuupdateclk_t;

/* added by chenfj 2008-3-25 */
typedef struct  Onu_vlan_priority
{	
	unsigned char row_status;
	short int vid;
	unsigned char EthType;
	unsigned char Priority;	
}__attribute__((packed))Onu_vlan_priority_t;

/* Device capabilities structure */
typedef struct ONU_optical_capability_t
{
	unsigned short int  laser_on_time;	     /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	unsigned short int  laser_off_time;      /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	unsigned char		onu_grant_fifo_depth;/* Max number of grant records an ONU can store,		   */
										     /* relevant to ONU only, range: 1 - MAXUCHAR			   */
	short int			agc_lock_time;		 /* PON upstream data AGC lock time, measured in TQ,	   */
											 /* relevant to OLT only, range: 1 - 2^15				   */
	short int			cdr_lock_time;		 /* PON RX signal synchronization time, measured in TQ,	   */
											 /* relevant to OLT only, range: 1 - 2^15				   */
	short int			pon_tx_signal;		 /* PON TX signal                                          */
} ONU_optical_capability_t;

/*  added by chenfj 2008-6-12
	扩展多子卡ONU设备管理,增加子卡版本/状态等管理信息
*/
#define  MAX_ONU_SUBBOARD_VERSION_LEN   16
typedef struct  Onu_SubBoard_Info
{
	unsigned char Inserted;   /* 子板插入状态*/
	unsigned char Status;       /* 子板运行状态*/
	unsigned char subType;   /* 自板类型*/
	unsigned char HwVersionLen;
	unsigned char HwVersion[MAX_ONU_SUBBOARD_VERSION_LEN];
	unsigned char BootVersionLen;
	unsigned char BootVersion[MAX_ONU_SUBBOARD_VERSION_LEN];
	unsigned char SwVersionLen;
	unsigned char SwVersion[MAX_ONU_SUBBOARD_VERSION_LEN];
	unsigned char FwVersionLen;
	unsigned char FwVersion[MAX_ONU_SUBBOARD_VERSION_LEN];
	unsigned char VendorInfoLen;
	unsigned char VendorInfo[MAXVENDORINFOLEN];

	unsigned char DeviceSerial_No[MAXSNLEN];
	unsigned char DeviceSerial_NoLen;
	unsigned char MadeInDate[MAXDATELEN];
	unsigned char MadeInDateLen;	
}__attribute__((packed))Onu_SubBoard_Info_t;

typedef struct ONUMeteringTable{
	long recvOpticalPower;	/*接收光功率*/
	long transOpticalPower;	/*发送光功率*/
	long ponTemperature;	/*模块温度*/
	long ponVoltageApplied;	/*模块电压*/
	long ponBiasCurrent;	/*偏置电流*/
#if 0
	long uplink_delta;  /* ONU 上行光功率衰减修正值*/
	long downlink_delta;  /* ONU 下行光功率衰减修正值*/
	USHORT OpticalPowerAlarm;
#endif
	USHORT AlarmStatus;
	char onu_power_support_flag;
}__attribute__((packed))ONUMeteringTable_S;

typedef struct
{
    unsigned int multicastGemPortNo;
    unsigned int multicastVLANID;
    unsigned int sourceIPAddress;
    unsigned int multicastGroupAddressStart;
    unsigned int multicastGroupAddressStop;
    unsigned int imputedGroupBandwidth;
}CTC_GPONADP_ONU_Multicast_Prof_t;

typedef struct
{
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_IGMP_VER 0x00000002
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_MULT_CONTROL_MODE 0x00000004
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_FAST_LEAVE_MODE 0x00000008
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_UPSTREAM_IGMP_TCI 0x00000010
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_UPSTREAM_IGMP_TAG_CONTROL 0x00000020
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_MAX_SIMUL_GROUP 0x00000040
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_OLT_TAG_STRIP 0x00000080
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_DS_IGMP_MCAST_TCI 0x00000100
#define GPONADP_ONU_SERV_PROTO_PROF_MASK_DS_IGMP_MCAST_TAG_CONTROL 0x00000200

    unsigned int bitMask;
    unsigned int igmpVersion;
    unsigned int multicastControlMode;
    unsigned int fastLeaveMode;
    unsigned int upstreamIGMPTCI;
    unsigned int upstreamIGMPTagControl;
    unsigned int maxSimultaneousGroup;
    unsigned char oltTagStrip;  /*True: olt strip outer tag, False: transparent in OLT.*/
    unsigned short downstreamIgmpMcastTCI;
    /* downstreamIgmpMcastTagControl value: 
       ** 0: transparent
       ** 1: strip outer tag
       ** 2: add tag, tag specified by downstreamIgmpMcastTCI
       ** 3: replace outer tag, include vid and pbit.
       ** 4: replace outer tag, only vid. */
    unsigned char downstreamIgmpMcastTagControl;

}__attribute__((packed)) CTC_GPONADP_ONU_McastOper_Prof_t;
typedef struct
{
     int  staticProfIdx;
     int  dynamicProfIdx;
     int  operProfIdx;
}CTC_GPONADP_ONU_Profile_t;
#define ONU_CONFIG_NAME_LEN 21
#define ONU_CONFIG_NAME_LEN_CLI (ONU_CONFIG_NAME_LEN-6)

/*begin: 支持管理IP的配置保存，mod by liuyh, 2017-5-5*/
typedef struct
{
    CTC_STACK_mxu_mng_global_parameter_config_t mxu_mng;
    unsigned char needSaveOlt;
} RPC_CTC_mxu_mng_global_parameter_config_t;
/*end: mod by liuyh, 2017-5-5*/

/* B--added by shixh@2010-5-13 for ONUMGT-API-IF */
#if 1

#if 1
/* -------------------ONU 基本API的GW实现------------------- */
extern int GWONU_OnuIsValid(short int olt_id, short int onu_id, int *status);
extern int GWONU_CmdIsSupported(short int olt_id, short int onu_id, short int *cmd);
extern int GWONU_CopyOnu(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int CopyFlags);
extern int GWONU_SetIFType(short int olt_id, short int onu_id, int chip_type, int remote_type);
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
extern int GWONU_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW);
#endif

#if 1
/* -------------------ONU加密管理API的GW实现------------------- */
extern int GWONU_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status);
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
extern int GWONU_GetOnuMacCheckFlag(short int olt_id, short int onu_id, ULONG  *flag);
#endif

#if 1
/* -------------------ONU 版本管理API------------------- */
extern int GPONONU_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype);
extern int GWONU_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype);
extern int CTCONU_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype);
extern int GWONU_OnuGwCtcSwConvert(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4]);
extern int GWONU_SetOnuDeviceName(short int olt_id, short int onu_id, char *Name, int NameLen);
extern int GWONU_SetOnuDeviceDesc(short int olt_id, short int onu_id, char *Desc, int DescLen);
extern int GWONU_SetOnuDeviceLocation(short int olt_id, short int onu_id, char *Location, int LocationLen);
extern int GWONU_GetOnuAllPortStatisticData(short int olt_id, short int onu_id, OnuStatisticData_S* data);
extern int GWONU_GetAllEthPortMacCounter(short int olt_id, short int onu_id, OnuEthPortCounter_t *data);
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
extern int GWONU_SetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int enable);
extern int GWONU_SetEthPortPause(short int olt_id, short int onu_id, int port_id, int enable);
extern int GWONU_SetEthPortAutoNegotiationRestart(short int olt_id, short int onu_id, int port_id);
extern int GWONU_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num);
extern int GWONU_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int tag);
extern int GWONU_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw);

extern int GWONU_CliCall(short int olt_id, short int onu_id, char *str, short int len, char **pRbuf, unsigned short int *plen);
extern int GWONU_SetMgtReset(short int olt_id, short int onu_id, int en);
extern int GWONU_SetMgtConfig(short int olt_id, short int onu_id, int save);
extern int GWONU_SetMgtLaser(short int olt_id, short int onu_id, int en);
extern int GWONU_SetTemperature(short int olt_id, short int onu_id, int temp);
extern int GWONU_SetPasFlush(short int olt_id, short int onu_id, int act);

extern int GWONU_SetAtuAgingTime(short int olt_id, short int onu_id, int aging);
extern int GWONU_SetAtuLimit(short int olt_id, short int onu_id, int limit);

extern int GWONU_SetPortLinkMon(short int olt_id, short int onu_id, int mon);
extern int GWONU_SetPortModeMon(short int olt_id, short int onu_id, int mon);
extern int GWONU_SetPortIsolate(short int olt_id, short int onu_id, int enable);

extern int GWONU_SetVlanEnable(short int olt_id, short int onu_id, int enable);
extern int GWONU_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode);
extern int GWONU_AddVlan(short int olt_id, short int onu_id, int vid);
extern int GWONU_DelVlan(short int olt_id, short int onu_id, int vid);
extern int GWONU_SetPortPvid(short int olt_id, short int onu_id, int port_id, int pvid);

extern int GWONU_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag);
extern int GWONU_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist);
extern int GWONU_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid);
extern int GWONU_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid);
extern int GWONU_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid);
extern int GWONU_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid);

extern int GWONU_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable);
extern int GWONU_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan);
extern int GWONU_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan);

extern int GWONU_SetPortVlanFrameTypeAcc(short int olt_id, short int onu_id, int port_id, int acc);
extern int GWONU_SetPortIngressVlanFilter(short int olt_id, short int onu_id, int port_id, int enable);

extern int GWONU_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode);
extern int GWONU_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int fc);
extern int GWONU_SetPortAtuLearn(short int olt_id, short int onu_id, int portlist, int enable);
extern int GWONU_SetPortAtuFlood(short int olt_id, short int onu_id, int portlist, int enable);
extern int GWONU_SetPortLoopDetect(short int olt_id, short int onu_id, int portlist, int enable);
extern int GWONU_SetPortStatFlush(short int olt_id, short int onu_id, int portlist, int enable);

extern int GWONU_SetIngressRateLimitBase(short int olt_id, short int onu_id, int uv);
extern int GWONU_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode);
extern int GWONU_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate);

extern int GWONU_SetQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid, int field, int oper, char *val, int val_len);
extern int GWONU_ClrQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid);
extern int GWONU_SetQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid, int queue, int prio);
extern int GWONU_ClrQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid);

extern int GWONU_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid);
extern int GWONU_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid);
extern int GWONU_SetPortQosRuleType(short int olt_id, short int onu_id, int port_id, int mode);

extern int GWONU_SetPortDefPriority(short int olt_id, short int onu_id, int port_id, int prio);
extern int GWONU_SetPortNewPriority(short int olt_id, short int onu_id, int port_id, int oldprio, int newprio);
extern int GWONU_SetQosPrioToQueue(short int olt_id, short int onu_id, int prio, int queue);
extern int GWONU_SetQosDscpToQueue(short int olt_id, short int onu_id, int dscpnum, int queue);
extern int GWONU_SetPortUserPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode);
extern int GWONU_SetPortIpPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode);
extern int GWONU_SetQosAlgorithm(short int olt_id, short int onu_id, int uv);
extern int GWONU_SET_QosMode(short int olt_id, short int onu_id, int direct, unsigned char mode);
extern int GWONU_SET_Rule(short int olt_id, short int onu_id, int direct, int code, gw_rule_t rule);

extern int GWONU_SetIgmpEnable(short int olt_id, short int onu_id, int enable);
extern int GWONU_SetIgmpAuth(short int olt_id, short int onu_id, int en);
extern int GWONU_SetIgmpHostAge(short int olt_id, short int onu_id, int age);
extern int GWONU_SetIgmpGroupAge(short int olt_id, short int onu_id, int age);
extern int GWONU_SetIgmpMaxResTime(short int olt_id, short int onu_id, int tm);

extern int GWONU_SetIgmpMaxGroup(short int olt_id, short int onu_id, int number);
extern int GWONU_AddIgmpGroup(short int olt_id, short int onu_id, int portlist, ULONG addr, ULONG vid);
extern int GWONU_DeleteIgmpGroup(short int olt_id, short int onu_id, ULONG addr);
extern int GWONU_SetPortIgmpFastLeave(short int olt_id, short int onu_id, int port_id, int mode);
extern int GWONU_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid);

extern int GWONU_SetPortMirrorFrom(short int olt_id, short int onu_id, int port_id, int mode, int type);
extern int GWONU_SetPortMirrorTo(short int olt_id, short int onu_id, int port_id, int type);
extern int GWONU_DeleteMirror(short int olt_id, short int onu_id, int type);
#endif


#if 1
/* -------------------ONU基本API------------------- */
extern int OnuMgt_OnuIsValid(short int olt_id, short int onu_id, int *status);
extern int OnuMgt_OnuIsOnline(short int olt_id, short int onu_id, int *status);
extern int OnuMgt_AddOnuByManual(short int olt_id, short int onu_id , unsigned char *MacAddr);
extern int OnuMgt_DelOnuByManual(short int olt_id, short int onu_id);
extern int OnuMgt_CmdIsSupported(short int olt_id, short int onu_id, short int *cmd);

extern int OnuMgt_CopyOnu(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int copy_flags);
extern int OnuMgt_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type);
extern int OnuMgt_SetIFType(short int olt_id, short int onu_id, int chip_type, int remote_type);
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
extern int OnuMgt_DeregisterOnu(short int olt_id, short int onu_id);
extern int OnuMgt_SetMacAuthMode(short int olt_id, short int onu_id, int auth_mode, mac_address_t auth_mac);
extern int OnuMgt_DelBindingOnu(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code, int code_param);
#if 0
extern int OnuMgt_AddPendingOnu(short int olt_id, short int onu_id, unsigned char *mac_address);
extern int OnuMgt_DelPendingOnu(short int olt_id, short int onu_id);
extern int OnuMgt_DelConfPendingOnu(short int olt_id, short int onu_id);
#endif
extern int OnuMgt_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode);
/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
extern int OnuMgt_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response);
extern int OnuMgt_AuthSuccess ( short int olt_id, short int onu_id);
extern int OnuMgt_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type );
#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
extern int OnuMgt_SetOnuTrafficServiceMode(short int olt_id,short int onu_id, int enable);
extern int OnuMgt_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2,short int enable);
extern int OnuMgt_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast);
extern int OnuMgt_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW);
extern int OnuMgt_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info);

extern int OnuMgt_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode);
extern int OnuMgt_GetOnuVlanMode(short int olt_id, short int onu_id, PON_olt_vlan_uplink_config_t *vlan_uplink_config);
extern int OnuMgt_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath);
extern int OnuMgt_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable);
extern int OnuMgt_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable);

extern int OnuMgt_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW);
extern int OnuMgt_GetOnuB2PMode(short int olt_id, short int onu_id, int *b2p_mode);
extern int OnuMgt_SetOnuB2PMode(short int olt_id, short int onu_id, int b2p_mode);
#endif


#if 1
/* -------------------ONU 监控统计管理API--------------- */
extern int OnuMgt_ResetCounters(short int olt_id, short int onu_id);
#endif


#if 1
/* -------------------ONU加密管理API-------------------- */
extern int OnuMgt_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters);
extern int OnuMgt_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir);
extern int OnuMgt_StopEncryption(short int olt_id, short int onu_id);
extern int OnuMgt_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time);
extern int OnuMgt_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
extern int OnuMgt_UpdateEncryptionKey(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU 地址管理API------------------- */
extern int OnuMgt_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table);
extern int OnuMgt_GetOltMacAddrTbl(short int olt_id,short int onu_id,short int * EntryNum,PON_address_table_t addr_table);
extern int OnuMgt_GetOltMacAddrVlanTbl(short int olt_id,short int onu_id,short int * EntryNum,PON_address_vlan_table_t addr_table);
extern int OnuMgt_SetOnuMaxMac(short int olt_id, short int onu_id, short int llid_id, unsigned int *val);
extern int OnuMgt_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config);
extern int OnuMgt_GetOnuMacCheckFlag(short int olt_id, short int onu_id, ULONG  *flag);
#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
extern int OnuMgt_GetOnuDistance(short int olt_id, short int onu_id, int *rtt);
extern int OnuMgt_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability);
#endif


#if 1
/* -------------------ONU 倒换API---------------- */
extern int OnuMgt_SetOnuLLID(short int olt_id, short int onu_id, short int llid);
#endif


#if 1
/* -------------------ONU 设备管理API------------------- */
extern int OnuMgt_GetOnuVer(short int olt_id, short int onu_id, PON_onu_versions *onu_versions);
extern int OnuMgt_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions);
extern int OnuMgt_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info);
extern int OnuMgt_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size);
extern int OnuMgt_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size);

extern int OnuMgt_ResetOnu(short int olt_id, short int onu_id);
extern int OnuMgt_SetOnuSWUpdateMode(short int olt_id, short int onu_id, int update_mode);
extern int OnuMgt_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype);
extern int OnuMgt_OnuGwCtcSwConvert(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4]);
extern int OnuMgt_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete);

extern int OnuMgt_SetOnuDeviceName(short int olt_id, short int onu_id, char *Name, int NameLen);
extern int OnuMgt_SetOnuDeviceDesc(short int olt_id, short int onu_id, char *Desc, int DescLen);
extern int OnuMgt_SetOnuDeviceLocation(short int olt_id, short int onu_id, char *Location, int LocationLen);
#endif
extern int OnuMgt_DrPengSetOnuPortSaveConfig(short int olt_id, short int onu_id, short int port_id,unsigned char action );
extern int OnuMgt_DrPengSetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char state);
extern int OnuMgt_DrPengGetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char * state);
extern int OnuMgt_DrPengSetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times );
extern int OnuMgt_DrPengGetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times );
extern int OnuMgt_DrPengSetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode );
extern int OnuMgt_DrPengGetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char *mode );
extern int OnuMgt_DrPengSetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status );
extern int OnuMgt_DrPengGetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status );
extern int OnuMgt_DrPengSetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char	device_location_len );
extern int OnuMgt_DrPengGetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location );
extern int OnuMgt_DrPengSetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name,unsigned char	name_len );
extern int OnuMgt_DrPengGetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name );
extern int OnuMgt_DrPengSetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name,unsigned char	device_name_len );
extern int OnuMgt_DrPengGetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name);
extern int OnuMgt_DrPengGetOnuPortMacNumber(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number );
extern int OnuMgt_DrPengGetOnuPortLocationByMAC(short int olt_id, short int onu_id, mac_address_t mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor );
extern int OnuMgt_DrPengGetOnuExtendAttribute(short int olt_id, short int onu_id, char *SupportAttribute);
#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
extern int OnuMgt_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version );
extern int OnuMgt_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version );
extern int OnuMgt_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number );
extern int OnuMgt_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id );

extern int OnuMgt_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps );
extern int OnuMgt_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps );
extern int OnuMgt_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps );

extern int OnuMgt_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name );
extern int OnuMgt_ActiveOnuFirmware( short int olt_id, short int onu_id );
extern int OnuMgt_CommitOnuFirmware( short int olt_id, short int onu_id );

extern int OnuMgt_StartEncrypt(short int olt_id, short int onu_id);
extern int OnuMgt_StopEncrypt(short int olt_id, short int onu_id);

extern int OnuMgt_GetEthPortLinkState(short int olt_id, short int onu_id, int port_id, int *link);
extern int OnuMgt_GetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int *enable);
extern int OnuMgt_SetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int enable);

extern int OnuMgt_GetEthPortPause(short int olt_id, short int onu_id, int port_id, int *enable);
extern int OnuMgt_SetEthPortPause(short int olt_id, short int onu_id, int port_id, int enable);

extern int OnuMgt_GetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int *an);
extern int OnuMgt_SetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int an);
extern int OnuMgt_SetEthPortAutoNegotiationRestart(short int olt_id, short int onu_id, int port_id);
extern int OnuMgt_GetEthPortAnLocalTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability);
extern int OnuMgt_GetEthPortAnAdvertisedTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability);

extern int OnuMgt_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing);
extern int OnuMgt_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing);

extern int OnuMgt_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing);
extern int OnuMgt_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing);

extern int OnuMgt_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port_id, CTC_STACK_port_vlan_configuration_t *vconf);
extern int OnuMgt_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf);
extern int OnuMgt_GetAllPortVlanConfig(short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_vlan_configuration_ports_t ports_info);

extern int OnuMgt_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port_id,	CTC_STACK_classification_rules_t cam);
extern int OnuMgt_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, int mode, CTC_STACK_classification_rules_t cam);
extern int OnuMgt_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id);

extern int OnuMgt_GetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port_id,	CTC_STACK_multicast_vlan_t *mv );
extern int OnuMgt_SetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv);
extern int OnuMgt_ClearEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id);

extern int OnuMgt_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int *num);
extern int OnuMgt_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num);

extern int OnuMgt_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *tag);
extern int OnuMgt_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int tag);
extern int OnuMgt_GetAllPortMulticastTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info );

extern int OnuMgt_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw);
extern int OnuMgt_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw);
extern int OnuMgt_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw);

extern int OnuMgt_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc );
extern int OnuMgt_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc);
extern int OnuMgt_ClearMulticastControl(short int olt_id, short int onu_id);

extern int OnuMgt_GetMulticastSwitch(short int olt_id, short int onu_id, int *sw);
extern int OnuMgt_SetMulticastSwitch(short int olt_id, short int onu_id, int sw);

extern int OnuMgt_GetMulticastFastleaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability);
extern int OnuMgt_GetMulticastFastleave(short int olt_id, short int onu_id, int *fl);
extern int OnuMgt_SetMulticastFastleave(short int olt_id, short int onu_id, int fl);

extern int OnuMgt_GetOnuPortStatisticData(short int olt_id, short int onu_id, int port_id,CTC_STACK_statistic_data_t *data);
extern int OnuMgt_GetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id,CTC_STACK_statistic_state_t *state);
extern int OnuMgt_SetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id,CTC_STACK_statistic_state_t *state);

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
extern int OnuMgt_SetAlarmAdminState(short int olt_id, short int onu_id,  CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable);
extern int OnuMgt_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold );
extern int OnuMgt_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );
extern int OnuMgt_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );

extern int OnuMgt_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng);
extern int OnuMgt_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, RPC_CTC_mxu_mng_global_parameter_config_t *mxu_mng);
extern int OnuMgt_GetMxuMngSnmpConfig( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter);
extern int OnuMgt_SetMxuMngSnmpConfig(short int olt_id, short int onu_id,CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter);

extern int OnuMgt_GetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover);
extern int OnuMgt_SetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover);
extern int OnuMgt_GetOptTransDiag ( short int olt_id, short int onu_id, CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );
extern int OnuMgt_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter);
extern int OnuMgt_GetCTCOnuPortStatsData(short int olt_id, short int onu_id, int port_id,OnuPortStats_ST *data);

extern int OnuMgt_GetFecAbility(short int olt_id, short int onu_id,  CTC_STACK_standard_FEC_ability_t  *fec_ability);
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */

#if 1
extern int OnuMgt_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t* iad_info);
extern int OnuMgt_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status);
extern int OnuMgt_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type);
extern int OnuMgt_GetVoipGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param);
extern int OnuMgt_SetVoipGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param);
extern int OnuMgt_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax);
extern int OnuMgt_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax);

extern int OnuMgt_GetVoipPortStatus(short int olt_id, short int onu_id, int port_id, CTC_STACK_voip_pots_status_array *pots_status_array);
extern int OnuMgt_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state);
extern int OnuMgt_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state);
extern int OnuMgt_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state);
extern int OnuMgt_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state);

extern int OnuMgt_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param);
extern int OnuMgt_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param);
extern int OnuMgt_GetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array);
extern int OnuMgt_SetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_t *user_tid_config);
extern int OnuMgt_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info);
extern int OnuMgt_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info);

extern int OnuMgt_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param);
extern int OnuMgt_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param);
extern int OnuMgt_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map);
extern int OnuMgt_GetSipUserConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_sip_user_param_config_array *sip_user_param_array);
extern int OnuMgt_SetSipUserConfig(short int olt_id, short int onu_id, int port_id, int code, CTC_STACK_sip_user_param_config_t *sip_user_param);
#endif
#endif


#if 1
/* -------------------ONU 远程管理API------------------- */
extern int OnuMgt_CliCall(short int olt_id, short int onu_id, char *cli_str, const int cli_len, char **rlt_str, unsigned short int *rlt_len);

extern int OnuMgt_SetMgtReset(short int olt_id, short int onu_id, int lv);
extern int OnuMgt_SetMgtConfig(short int olt_id, short int onu_id, int lv);
extern int OnuMgt_SetMgtLaser(short int olt_id, short int onu_id, int lv);
extern int OnuMgt_SetTemperature(short int olt_id, short int onu_id, int temp);
extern int OnuMgt_SetPasFlush(short int olt_id, short int onu_id, int act);

extern int OnuMgt_SetAtuAgingTime(short int olt_id, short int onu_id, int aging);
extern int OnuMgt_SetAtuLimit(short int olt_id, short int onu_id, int limit);

extern int OnuMgt_SetPortLinkMon(short int olt_id, short int onu_id, int mon);
extern int OnuMgt_SetPortModeMon(short int olt_id, short int onu_id, int mon);
extern int OnuMgt_SetPortIsolate(short int olt_id, short int onu_id, int enable);

extern int OnuMgt_SetVlanEnable(short int olt_id, short int onu_id, int enable);
extern int OnuMgt_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode);
extern int OnuMgt_AddVlan(short int olt_id, short int onu_id, int vid);
extern int OnuMgt_DelVlan(short int olt_id, short int onu_id, int vid);
extern int OnuMgt_SetPortPvid(short int olt_id, short int onu_id, int port_id, int pvid);

extern int OnuMgt_AddVlanPort(short int olt_id, short int onu_id, int vid, int portmask, int tag);
extern int OnuMgt_DelVlanPort(short int olt_id, short int onu_id, int vid, int portmask);
extern int OnuMgt_SetEthPortVlanTran(short int olt_id, short int onu_id, int port_id,ULONG inVid, ULONG newVid);
extern int OnuMgt_DelEthPortVlanTran(short int olt_id, short int onu_id, int port_id,ULONG inVid);
extern int OnuMgt_SetEthPortVlanAgg(short int olt_id, short int onu_id, int port_id,USHORT inVid[8], USHORT targetVid);
extern int OnuMgt_DelEthPortVlanAgg(short int olt_id, short int onu_id, int port_id,ULONG targetVid);

extern int OnuMgt_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable );
extern int OnuMgt_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG  cvlan,ULONG svlan);
extern int OnuMgt_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id,ULONG svlan);

extern int OnuMgt_SetPortVlanFrameTypeAcc(short int olt_id, short int onu_id, int port_id, int acc);
extern int OnuMgt_SetPortIngressVlanFilter(short int olt_id, short int onu_id, int port_id, int enable);

extern int OnuMgt_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode);
extern int OnuMgt_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int fc);
extern int OnuMgt_SetPortAtuLearn(short int olt_id, short int onu_id, int portlist, int enable);
extern int OnuMgt_SetPortAtuFlood(short int olt_id, short int onu_id, int portlist, int enable);
extern int OnuMgt_SetPortLoopDetect(short int olt_id, short int onu_id, int portlist, int enable);
extern int OnuMgt_SetPortStatFlush(short int olt_id, short int onu_id, int portlist, int enable);

extern int OnuMgt_SetIngressRateLimitBase(short int olt_id, short int onu_id, int uv);
extern int OnuMgt_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode);
extern int OnuMgt_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate);

extern int OnuMgt_SetQosClass(short int olt_id, short int onu_id, int qossetid,  int ruleid, int classid, int field, int oper, char *val, int len);
extern int OnuMgt_ClrQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid);
extern int OnuMgt_SetQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid, int queue, int prio);
extern int OnuMgt_ClrQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid);

extern int OnuMgt_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid);
extern int OnuMgt_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid);
extern int OnuMgt_SetPortQosRuleType(short int olt_id, short int onu_id, int port_id, int mode);

extern int OnuMgt_SetPortDefPriority(short int olt_id, short int onu_id, int port_id, int prio);
extern int OnuMgt_SetPortNewPriority(short int olt_id, short int onu_id, int port_id, int oldprio, int newprio);
extern int OnuMgt_SetQosPrioToQueue(short int olt_id, short int onu_id, int prio, int queue);
extern int OnuMgt_SetQosDscpToQueue(short int olt_id, short int onu_id, int dscpnum,int queue);
extern int OnuMgt_SetPortUserPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode);
extern int OnuMgt_SetPortIpPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode);
extern int OnuMgt_SetQosAlgorithm(short int olt_id, short int onu_id, int uv);

extern int OnuMgt_SetIgmpEnable(short int olt_id, short int onu_id, int en);
extern int OnuMgt_SetIgmpAuth(short int olt_id, short int onu_id, int en);
extern int OnuMgt_SetIgmpHostAge(short int olt_id, short int onu_id, int age);
extern int OnuMgt_SetIgmpGroupAge(short int olt_id, short int onu_id, int age);
extern int OnuMgt_SetIgmpMaxResTime(short int olt_id, short int onu_id, int tm);

extern int OnuMgt_SetIgmpMaxGroup(short int olt_id, short int onu_id, int number);
extern int OnuMgt_AddIgmpGroup(short int olt_id, short int onu_id, int portlist, ULONG addr, ULONG vid);
extern int OnuMgt_DeleteIgmpGroup(short int olt_id, short int onu_id, ULONG addr);
extern int OnuMgt_SetPortIgmpFastLeave(short int olt_id, short int onu_id, int port_id, int mode);
extern int OnuMgt_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid);

extern int OnuMgt_SetPortMirrorFrom(short int olt_id, short int onu_id, int port_id,int mode, int type);
extern int OnuMgt_SetPortMirrorTo(short int olt_id, short int onu_id, int port_id, int type);
extern int OnuMgt_DeleteMirror(short int olt_id, short int onu_id, int type);
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */

#if 1
/* --------------------CMC管理API------------------- */
extern int OnuMgt_RegisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac);
extern int OnuMgt_UnregisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac);
extern int OnuMgt_DumpCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcAlarms(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLogs(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);

extern int OnuMgt_ResetCmcBoard(short int olt_id, short int onu_id, mac_address_t cmc_mac);
extern int OnuMgt_GetCmcVersion(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len);
extern int OnuMgt_GetCmcMaxMulticasts(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_multicasts);
extern int OnuMgt_GetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_cm);
extern int OnuMgt_SetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short max_cm);

extern int OnuMgt_GetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time);
extern int OnuMgt_SetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time);
extern int OnuMgt_LocalCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac);
extern int OnuMgt_SetCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len);
extern int OnuMgt_DumpCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, char *dump_buf, unsigned short *dump_len);
#endif

#if 1
/* --------------------CMC频道管理API------------------- */
extern int OnuMgt_DumpCmcDownChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcUpChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_GetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode);
extern int OnuMgt_SetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode);
extern int OnuMgt_GetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode);

extern int OnuMgt_SetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode);
extern int OnuMgt_GetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *docsis30_mode);
extern int OnuMgt_SetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char docsis30_mode);
extern int OnuMgt_GetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq);
extern int OnuMgt_SetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq);

extern int OnuMgt_GetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq);
extern int OnuMgt_SetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq);
extern int OnuMgt_SetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode);
extern int OnuMgt_SetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode);
extern int OnuMgt_GetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_width);

extern int OnuMgt_SetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_width);
extern int OnuMgt_GetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *annex_mode);
extern int OnuMgt_SetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char annex_mode);
extern int OnuMgt_GetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_type);
extern int OnuMgt_SetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_type);

extern int OnuMgt_GetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *modulation_type);
extern int OnuMgt_SetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char modulation_type);
extern int OnuMgt_GetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_profile);
extern int OnuMgt_SetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_profile);
extern int OnuMgt_GetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *interleaver);

extern int OnuMgt_SetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char interleaver);
extern int OnuMgt_GetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power);
extern int OnuMgt_SetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power);
extern int OnuMgt_GetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power);
extern int OnuMgt_SetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power);

extern int OnuMgt_DumpCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcUpChannelSignalQuality(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcInterfaceUtilization(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcInterfaceStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcMacStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);

extern int OnuMgt_DumpCmcAllInterface(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
#endif

#if 1
/* --------------------CMC频道组管理API------------------- */
extern int OnuMgt_DumpCmcAllLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);

extern int OnuMgt_DumpCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingDynConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_SetCmcLoadBalancingDynMethod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char method);
extern int OnuMgt_SetCmcLoadBalancingDynPeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period);

extern int OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period);
extern int OnuMgt_SetCmcLoadBalancingDynOverloadThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent);
extern int OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent);
extern int OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long max_move);
extern int OnuMgt_SetCmcLoadBalancingDynMinHoldTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long hold_time);

extern int OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char range_mode);
extern int OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id);
extern int OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id);
extern int OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id);
extern int OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id);

extern int OnuMgt_CreateCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char grp_method);
extern int OnuMgt_DestroyCmcLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id);
extern int OnuMgt_AddCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids);
extern int OnuMgt_RemoveCmcLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids);
extern int OnuMgt_AddCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids);

extern int OnuMgt_RemoveCmcLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids);
extern int OnuMgt_AddCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end);
extern int OnuMgt_RemoveCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end);
extern int OnuMgt_AddCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end);
extern int OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end);

extern int OnuMgt_DumpCmcLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingGrpActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
#endif

#if 1
/* --------------------CM管理API------------------- */
extern int OnuMgt_DumpAllCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_ClearAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac);

extern int OnuMgt_ResetCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac);
extern int OnuMgt_DumpCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_SetCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids);
extern int OnuMgt_SetCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids);

extern int OnuMgt_CreateCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len);
extern int OnuMgt_ModifyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len);
extern int OnuMgt_DestroyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid);
extern int OnuMgt_DumpCmClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
#endif

#if 1
/* --------------------QoS管理API------------------- */
extern int OnuMgt_DumpCmcClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcServiceFlowStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcDownChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);
extern int OnuMgt_DumpCmcUpChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);

extern int OnuMgt_CreateCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len);
extern int OnuMgt_DestroyCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name);
#endif

#if 1
/* --------------------地址管理API------------------- */
extern int OnuMgt_GetCmcMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table);
extern int OnuMgt_GetCmMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table);
extern int OnuMgt_ResetCmAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type);
#endif

#endif


#if 1
/* --------------------ONU DOCSIS应用管理API------------------- */

#endif


/* -------------------ONU高级API------------------- */
extern int    OnuMgtAdv_IsValid(short int olt_id, short int onu_id);
extern int    OnuMgtAdv_IsOnline(short int olt_id, short int onu_id);

extern int    OnuMgtAdv_GetBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW);


#if 1
/* -------------------ONU基本API------------------- */
typedef int (*ctss_Onu_Set_MulticastTemplate_f)(short int olt_id, short int onu_id,  int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag);
typedef int (*ctss_Onu_Get_MulticastTemplate_f)(short int olt_id, short int onu_id,  int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag);
typedef int (*ctss_Onu_SetMcastOperProfile_f)( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter);
typedef int (*ctss_Onu_GetMcastOperProfile_f)( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter);
typedef int (*ctss_Onu_SetUniPortAssociateMcastPro_f)( short int olt_id, short int onu_id,short int portid ,unsigned char stateflag,int profIdx);
typedef int (*ctss_Onu_GetUniPortAssociateMcastPro_f)( short int olt_id, short int onu_id, short int port_id, CTC_GPONADP_ONU_Profile_t *ProfIdx);
typedef int (*ctss_Onu_SetOnuPortIsolation)( short int olt_id, short int onu_id,unsigned char state);
typedef int (*ctss_Onu_GetOnuPortIsolation)( short int olt_id, short int onu_id,unsigned char * state);
typedef int (*ctss_Onu_GetOnuMacEntry)(short int olt_id, short int onu_id, ULONG mactype ,OnuPortLacationInfor_S *table);
typedef int (*ctss_Onu_SetOnuPortSaveConfig)(short int olt_id, short int onu_id, short int port_id,unsigned char action );
typedef int (*ctss_Onu_SetOnuLoopDetectionTime)(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times );
typedef int (*ctss_Onu_GetOnuLoopDetectionTime)(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times );
typedef int (*ctss_Onu_SetOnuPortMode)(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode );
typedef int (*ctss_Onu_GetOnuPortMode)(short int olt_id, short int onu_id, unsigned short port_id,unsigned char *mode );
typedef int (*ctss_Onu_SetOnuDeviceName)(short int olt_id, short int onu_id, unsigned char* device_name,unsigned char  device_name_len );
typedef int (*ctss_Onu_GetOnuDeviceName)(short int olt_id, short int onu_id, unsigned char* device_name );
typedef int (*ctss_Onu_SetOnuPortStormStatus)(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status );
typedef int (*ctss_Onu_GetOnuPortMacNumber)(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number );
typedef int (*ctss_Onu_GetOnuPortLocationByMAC)(short int olt_id, short int onu_id, mac_address_t mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor );
typedef int (*ctss_Onu_GetOnuSupportExtendAtt)(short int olt_id, short int onu_id, unsigned char* mac );
typedef int (*ctss_onu_is_valid_f)(short int olt_id, short int onu_id, int *status); 
typedef int (*ctss_onu_is_online_f)(short int olt_id, short int onu_id, int *status); 
typedef int (*ctss_onu_add_onu_manual_f)(short int olt_id, short int onu_id, unsigned char *MacAddr);
typedef int (*ctss_onu_modify_onu_manual_f)( short int olt_id, short int onu_id , unsigned char *MacAddr );
typedef int (*ctss_onu_del_onu_manual_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_add_gpon_onu_manual_f)(short int olt_id, short int onu_id, unsigned char *sn);
typedef int (*ctss_onu_is_support_cmd_f)(short int olt_id, short int onu_id, short int *cmd);

typedef int (*ctss_onu_copy_onu_f)(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int copy_flags);
typedef int (*ctss_onu_get_iftype_f)(short int olt_id, short int onu_id, int *chip_type, int *remote_type);
typedef int (*ctss_onu_set_iftype_f)(short int olt_id, short int onu_id, int chip_type, int remote_type);
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
typedef int (*ctss_onu_deregister_onu_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_set_mac_auth_f)(short int olt_id, short int onu_id, int auth_mode, mac_address_t auth_mac);
typedef int (*ctss_onu_del_bind_onu_f)(short int olt_id, short int onu_id , PON_onu_deregistration_code_t deregistration_code, int code_param);
#if 0
typedef int (*ctss_onu_add_onu_pending_f)( short int olt_id, short int onu_id , unsigned char *MacAddr );
typedef int (*ctss_onu_del_onu_pending_f)( short int olt_id, short int onu_id);
typedef int (*ctss_onu_del_onu_pending_conf_f)( short int olt_id, short int onu_id);
#endif
typedef int (*ctss_onu_authorize_onu_f)(short int olt_id, short int onu_id, bool auth_mode);
typedef int (*ctss_onu_auth_request_f) ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response);
typedef int (*ctss_onu_auth_success_f) ( short int olt_id, short int onu_id);
typedef int (*ctss_onu_auth_failure_f) (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type );
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
typedef int (*ctss_onu_set_traffic_service_f)(short int olt_id, short int onu_id, int enable);
typedef int (*ctss_onu_set_onu_peer_to_peer_f)(short int olt_id, short int onu_id1, short int onu_id2, short int enable);
typedef int (*ctss_onu_set_onu_peer_to_peer_forward_f)( short int olt_id, short int onu_id , int address_not_found, int broadcast);
typedef int (*ctss_onu_set_onu_bw_f)(short int olt_id, short int onu_id, ONU_bw_t *BW);
typedef int (*ctss_onu_get_onu_sla_f)(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info);

typedef int (*ctss_onu_set_fec_mode_f)(short int olt_id, short int onu_id, int fec_mode);
typedef int (*ctss_onu_vlan_mode_f)(short int olt_id, short int onu_id , PON_olt_vlan_uplink_config_t  *vlan_uplink_config );
typedef int (*ctss_onu_set_uni_port_f)(short int olt_id,short int onu_id, bool enable_cpu, bool enable_datapath);
typedef int (*ctss_onu_set_slow_protocol_limit_f)(short int olt_id,short int onu_id, bool enable);
typedef int (*ctss_onu_get_slow_protocol_limit_f)(short int olt_id,short int onu_id, bool *enable);

typedef int (*ctss_onu_get_bw_f)(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW);
typedef int (*ctss_onu_get_mode_f)(short int olt_id, short int onu_id, int *mode);
typedef int (*ctss_onu_set_mode_f)(short int olt_id, short int onu_id, int mode);
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
typedef int (*ctss_onu_reset_counters_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_pon_loopback_f)(short int olt_id, short int onu_id, int enable );
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
typedef int (*ctss_onu_get_llid_params_f)(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters);
typedef int (*ctss_onu_start_encryption_f)(short int olt_id, short int onu_id, int *encrypt_dir);
typedef int (*ctss_onu_stop_encryption_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_set_encrypt_params_f)(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time);
typedef int (*ctss_onu_get_encrypt_params_f)(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *);

typedef int (*ctss_onu_update_encryption_key_f)(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method);
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
typedef int (*ctss_onu_get_onu_mac_addrtbl_f)(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table);
/*-------modified by wangjiah@2016-07-14 begin--------------*/
//typedef int (*ctss_onu_get_olt_mac_addrtbl_f)(short int olt_id, short int onu_id, long *EntryNum, PON_address_table_t addr_table);
typedef int (*ctss_onu_get_olt_mac_addrtbl_f)(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table);
/*-------modified by wangjiah@2016-07-14 end --------------*/
typedef int (*ctss_onu_get_olt_mac_addrtbl_vlan_f)(short int olt_id, short int onu_id, short int *EntryNum, PON_address_vlan_table_t addr_table);
typedef int (*ctss_onu_set_onu_max_mac_f)(short int olt_id, short int onu_id, short int llid_id, unsigned int *val);
typedef int (*ctss_onu_uni_port_config_ver_f)(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config);
typedef int (*ctss_onu_mac_check_flag_f)(short int olt_id, short int onu_id, ULONG  *flag);
typedef int (*ctss_onu_get_all_eth_port_mac_counter_f)(short int olt_id, short int onu_id, OnuEthPortCounter_t *data);
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
typedef int (*ctss_onu_get_distance_f)(short int olt_id, short int onu_id, int *rtt);
typedef int (*ctss_onu_get_opticap_f)(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability);
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
typedef int (*ctss_onu_set_onu_llid_f)(short int olt_id, short int onu_id, short int llid);
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
typedef int (*ctss_onu_get_onu_ver_f)(short int olt_id, short int onu_id, PON_onu_versions *onu_versions);
typedef int (*ctss_onu_get_pon_ver_f)(short int olt_id, short int onu_id, PAS_device_versions_t *device_versions);
typedef int (*ctss_onu_get_onu_reginfo_f)(short int olt_id, short int onu_id, onu_registration_info_t *onu_info);
typedef int (*ctss_onu_get_onu_i2c_info_f)(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size);
typedef int (*ctss_onu_set_onu_i2c_info_f)(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size);

typedef int (*ctss_onu_reset_onu_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_set_onu_update_mode_f)(short int olt_id, short int onu_id, int update_mode);
typedef int (*ctss_onu_update_onu_software_f)(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype);
typedef int (*ctss_onu_gw_ctc_sw_convert_f)(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4]);
typedef int (*ctss_onu_get_burn_image_complete_f)(short int olt_id,short int onu_id, bool *complete);

typedef int (*ctss_onu_set_onu_device_name_f)(short int olt_id, short int onu_id, char *Name, int NameLen);
typedef int (*ctss_onu_set_onu_device_desc_f)(short int olt_id, short int onu_id, char *desc, int DescLen);
typedef int (*ctss_onu_set_onu_device_location_f)(short int olt_id, short int onu_id, char *Location, int LocationLen);
typedef int (*ctss_onu_get_onu_all_port_statistic_data_f)(short int olt_id, short int onu_id, OnuStatisticData_S* data);
#endif

#if 1
/* -------------------ONU CTC-PROTOCOL API----------------- */
typedef int (*ctss_onu_get_ctc_version_f)(short int olt_id, short int onu_id, unsigned char *version);
typedef int (*ctss_onu_get_firmware_version_f)(short int olt_id, short int onu_id, unsigned short int *version);
typedef int (*ctss_onu_get_serial_f)(short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number);
typedef int (*ctss_onu_get_chipset_f)(short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id);

typedef int (*ctss_onu_get_onu_cap1_f)( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps );
typedef int (*ctss_onu_get_onu_cap2_f)( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps );
typedef int (*ctss_onu_get_onu_cap3_f)( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps );

typedef int (*ctss_onu_update_onu_firmware_f)( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name );
typedef int (*ctss_onu_active_onu_firmware_f)( short int olt_id, short int onu_id );
typedef int (*ctss_onu_commit_onu_firmware_f)( short int olt_id, short int onu_id );

typedef int (*ctss_onu_start_encrypt_f)(short int olt_id, short int onu_id);
typedef int (*ctss_onu_stop_encrypt_f)(short int olt_id, short int onu_id);

typedef int (*ctss_onu_get_port_link_state_f)(short int olt_id, short int onu_id, int port, int *ls );
typedef int (*ctss_onu_get_port_admin_f)(short int olt_id, short int onu_id, int port, int *enable );
typedef int (*ctss_onu_set_port_admin_f)(short int olt_id, short int onu_id, int port, int enable );

typedef int (*ctss_onu_get_port_pause_f)(short int olt_id, short int onu_id, int port, int *enable );
typedef int (*ctss_onu_set_port_pause_f)(short int olt_id, short int onu_id, int port, int enable );

typedef int (*ctss_onu_get_port_auto_negotiation_f)(short int olt_id, short int onu_id, int port, int *an );
typedef int (*ctss_onu_set_port_auto_negotiation_f)(short int olt_id, short int onu_id, int port, int an );
typedef int (*ctss_onu_set_port_auto_negotiation_restart_f)(short int olt_id, short int onu_id, int port );
typedef int (*ctss_onu_get_port_auto_negotiation_tecability_f)(short int olt_id, short int onu_id, int port, CTC_STACK_auto_negotiation_technology_ability_t *ablity);

typedef int (*ctss_onu_get_port_policing_f)(short int olt_id, short int onu_id, int port, CTC_STACK_ethernet_port_policing_entry_t *policing);
typedef int (*ctss_onu_set_port_policing_f)(short int olt_id, short int onu_id, int port, CTC_STACK_ethernet_port_policing_entry_t *policing);
typedef int (*ctss_onu_get_port_downstream_policing_f)(short int olt_id, short int onu_id, int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing);
typedef int (*ctss_onu_set_port_downstream_policing_f)(short int olt_id, short int onu_id, int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing);

typedef int (*ctss_onu_get_port_vlan_config_f)(short int olt_id, short int onu_id, int port, CTC_STACK_port_vlan_configuration_t *config );
typedef int (*ctss_onu_set_port_vlan_config_f)(short int olt_id, short int onu_id, int port, CTC_STACK_port_vlan_configuration_t *config );
typedef int (*ctss_onu_get_vlan_allport_configuration_f)(short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_vlan_configuration_ports_t ports_info);

typedef int (*ctss_onu_get_port_classification_and_mark_f)(short int olt_id, short int onu_id, int port, CTC_STACK_classification_rules_t config );
typedef int (*ctss_onu_set_port_classification_and_mark_f)(short int olt_id, short int onu_id, int port, int mode, CTC_STACK_classification_rules_t config );
typedef int (*ctss_onu_clr_port_classification_and_mark_f)(short int olt_id, short int onu_id, int port);

typedef int (*ctss_onu_get_port_multicast_vlan_f)(short int olt_id, short int onu_id, int port, CTC_STACK_multicast_vlan_t *mv );
typedef int (*ctss_onu_set_port_multicast_vlan_f)(short int olt_id, short int onu_id, int port, CTC_STACK_multicast_vlan_t *mv );
typedef int (*ctss_onu_clr_port_multicast_vlan_f)(short int olt_id, short int onu_id, int port );

typedef int (*ctss_onu_get_port_multicast_max_group_f)(short int olt_id, short int onu_id, int port, int *num );
typedef int (*ctss_onu_set_port_multicast_max_group_f)(short int olt_id, short int onu_id, int port, int num );

typedef int (*ctss_onu_get_port_multicast_tag_strip_f)(short int olt_id, short int onu_id, int port, int *strip );
typedef int (*ctss_onu_set_port_multicast_tag_strip_f)(short int olt_id, short int onu_id, int port, int strip);
typedef int (*ctss_onu_get_all_multicast_tag_strip_f) ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info );

typedef int (*ctss_onu_get_port_igmp_tag_oper_f)(short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw);
typedef int (*ctss_onu_set_port_igmp_tag_oper_f)(short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw);
typedef int (*ctss_onu_set_obj_igmp_tag_oper_f) ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw);

typedef int (*ctss_onu_get_multicast_control_f)(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc );
typedef int (*ctss_onu_set_multicast_control_f)(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc );
typedef int (*ctss_onu_clr_multicast_control_f)(short int olt_id, short int onu_id );

typedef int (*ctss_onu_get_multicast_switch_f)(short int olt_id, short int onu_id, int *sw );
typedef int (*ctss_onu_set_multicast_switch_f)(short int olt_id, short int onu_id, int sw);

typedef int (*ctss_onu_get_multicast_fast_leave_ability_f)(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability );
typedef int (*ctss_onu_get_multicast_fast_leave_f)(short int olt_id, short int onu_id, int *fl );
typedef int (*ctss_onu_set_multicast_fast_leave_f)(short int olt_id, short int onu_id, int fl);

typedef int (*ctss_onu_get_onu_port_statistic_data_f)(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data);
typedef int (*ctss_onu_get_onu_port_statistic_state_f)(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state);
typedef int (*ctss_onu_set_onu_port_statistic_state_f)(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state);

typedef int (*ctss_onu_set_obj_alarm_adminstate_f)(short int olt_id, short int onu_id,  CTC_management_object_t *management_object, CTC_STACK_alarm_id_t alarm_id, bool enable);
typedef int (*ctss_onu_set_obj_alarm_threshold_f) (short int olt_id, short int onu_id, CTC_management_object_t *management_object, CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long clear_threshold );
typedef int (*ctss_onu_get_dbareport_thresholds_f) ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t *queues_sets_thresholds );
typedef int (*ctss_onu_set_dbareport_thresholds_f) ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t *queues_sets_thresholds );

typedef int (*ctss_onu_get_mxu_mng_global_config_f)(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng);
typedef int (*ctss_onu_set_mxu_mng_global_config_f)(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng);
typedef int (*ctss_onu_get_mxu_mng_snmp_config_f) ( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter);
typedef int (*ctss_onu_set_mxu_mng_snmp_config_f)(short int olt_id, short int onu_id,CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter);

typedef int (*ctss_onu_get_holdover_f)(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover);
typedef int (*ctss_onu_set_holdover_f)(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover);
typedef int (*ctss_onu_get_optical_transceiver_diagnosis_f) ( short int olt_id, short int onu_id, CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );
typedef int (*ctss_onu_set_tx_power_supply_control_f) (short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter);

typedef int (*ctss_onu_get_fec_ability_f)(short int olt_id,short int onu_id,CTC_STACK_standard_FEC_ability_t  *fec_ability);

#if 1
typedef int (*ctss_onu_get_iad_info)(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t *iad_info);
typedef int (*ctss_onu_get_voip_iad_oper_status)(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status);
typedef int (*ctss_onu_set_voip_iad_operation)(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type);
typedef int (*ctss_onu_get_GlobalParameterConfig)(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param);
typedef int (*ctss_onu_set_GlobalParameterConfig)(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param);
typedef int (*ctss_onu_get_voip_fax_config)(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax);
typedef int (*ctss_onu_set_voip_fax_config)(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax);

typedef int (*ctss_onu_get_voip_pots_status)(short int olt_id, short int onu_id, int port, CTC_STACK_voip_pots_status_array *pots_status_array);
typedef int (*ctss_onu_set_voip_port)(short int olt_id, short int onu_id, int port_number, CTC_STACK_on_off_state_t   port_state);
typedef int (*ctss_onu_get_voip_port)(short int olt_id, short int onu_id, int port_number, CTC_STACK_on_off_state_t  *port_state);
typedef int (*ctss_onu_set_voip_port2)(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state);
typedef int (*ctss_onu_get_voip_port2)(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state);

typedef int (*ctss_onu_get_H248ParameterConfig)(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param);
typedef int (*ctss_onu_set_H248ParameterConfig)(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param);
typedef int (*ctss_onu_get_H248UserTidInfo)(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array);
typedef int (*ctss_onu_set_H248UserTidConfig)(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_t *user_tid_config);
typedef int (*ctss_onu_get_H248RtpTidConfig)(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info);
typedef int (*ctss_onu_set_H248RtpTidConfig)(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info);

typedef int (*ctss_onu_get_sip_param_config)(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param);
typedef int (*ctss_onu_set_sip_param_config)(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param);
typedef int (*ctss_onu_set_sip_digit_map)(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map);
typedef int (*ctss_onu_get_sip_user_param_config)(short int olt_id, short int onu_id, int port, CTC_STACK_sip_user_param_config_array *sip_user_param_array);
typedef int (*ctss_onu_set_sip_user_param_config)(short int olt_id, short int onu_id, int port, int code, CTC_STACK_sip_user_param_config_t *sip_user_param);
typedef int (*ctss_onu_get_onu_port_stats_f )(short int olt_id, short int onu_id, int port_id, OnuPortStats_ST *data);
#endif
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
typedef int (*ctss_onu_cli_call_f)(short int olt_id, short int onu_id, char *cli_str, const int cli_len, char **rlt_str, unsigned short int *rlt_len);

typedef int (*ctss_onu_set_long_f)(short int olt_id, short int onu_id, int lv);
typedef int (*ctss_onu_set_port_long_f)(short int olt_id, short int onu_id, int port, int lv);
typedef int (*ctss_onu_set_vlan_port_add_f)(short int olt_id, short int onu_id, int vid, int portlist, int tag);
typedef int (*ctss_onu_set_vlan_port_del_f)(short int olt_id, short int onu_id, int vid, int portlist);
typedef int (*ctss_onu_set_qos_class_f)(short int olt_id, short int onu_id, int qossetid, int ruleid,int classid, int field, int oper, char *val, int len);
typedef int (*ctss_onu_set_qos_rule_f)(short int olt_id, short int onu_id, int qossetid, int ruleid, UCHAR queue, UCHAR prio);
typedef int (*ctss_onu_clr_qos_class_f)(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid);
typedef int (*ctss_onu_clr_qos_rule_f)(short int olt_id, short int onu_id, int qossetid, int ruleid);
typedef int (*ctss_onu_qos_prioreplace_f)(short int olt_id, short int onu_id, int port, int oldprio, int newprio);
typedef int (*ctss_onu_setqosrulemode_f)(short int olt_id, short int onu_id, int direct, unsigned char mode);
typedef int (*ctss_onu_setrule_f)(short int olt_id, short int onu_id, int direct, int code, gw_rule_t rule);
typedef int (*ctss_onu_get_onuver_f)(short int olt_id, short int onu_id, char *ver);
typedef int (*ctss_onu_set_vlan_transation_f)(short int olt_id, short int onu_id, int port, ULONG inVid, ULONG newVid);
typedef int (*ctss_onu_set_vlan_transation_del_f)(short int olt_id, short int onu_id, int port, ULONG inVid);
typedef int (*ctss_onu_set_vlan_agg_f)(short int olt_id, short int onu_id, int port, USHORT inVid[8], USHORT targetVid);
typedef int (*ctss_onu_set_vlan_agg_del_f)(short int olt_id, short int onu_id, int port, ULONG targetVid);
typedef int (*ctss_onu_set_portmirror_long_f)(short int olt_id, short int onu_id, int port, int lv, char type);
typedef int (*ctss_onu_add_igmpgroup_f)(short int olt_id, short int onu_id, int port, ULONG addr,ULONG vid);
typedef int (*ctss_onu_delete_igmpgroup_f)(short int olt_id, short int onu_id, ULONG addr);
typedef int (*ctss_onu_get_igmpgroup_f)(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mv);
typedef int (*ctss_onu_set_vlan_qinq_enable_f)(short int olt_id,short int onu_id, int port ,int enable );
typedef int (*ctss_onu_set_vlan_qinq_add_f)(short int olt_id, short int onu_id, int port, ULONG  cvlan,ULONG svlan);
typedef int (*ctss_onu_set_vlan_qinq_del_f)(short int olt_id, short int onu_id, int port, ULONG  cvlan);
typedef int (*ctss_onu_set_port_ingress_rate_f)(short int olt_id, short int onu_id, int port, int ratetype, int limit, int action, int burstmode);

#endif

#if 1
/* --------------------ONU CMC协议管理API------------------- */
typedef int (*ctss_onu_op_cmc_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac);
typedef int (*ctss_onu_dump_cmc_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len);

typedef int (*ctss_onu_get_cmc_ver_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len);
typedef int (*ctss_onu_get_cmc_u32_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long *out_u32);
typedef int (*ctss_onu_set_cmc_u32_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long in_u32);
typedef int (*ctss_onu_get_cmc_u16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *out_u16);
typedef int (*ctss_onu_set_cmc_u16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short in_u16);
typedef int (*ctss_onu_get_cmc_u8_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *out_u8);
typedef int (*ctss_onu_set_cmc_u8_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char in_u8);

typedef int (*ctss_onu_get_cmc_tm_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time);
typedef int (*ctss_onu_set_cmc_tm_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time);
typedef int (*ctss_onu_set_cmc_cfg_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len);

typedef int (*ctss_onu_dump_cmc_channel_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
typedef int (*ctss_onu_dump_cmc_channel2_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
typedef int (*ctss_onu_get_cmc_channel_u8_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *out_u8);
typedef int (*ctss_onu_set_cmc_channel_u8_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char in_u8);
typedef int (*ctss_onu_get_cmc_channel_u16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned short *out_u16);
typedef int (*ctss_onu_set_cmc_channel_u16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned short in_u16);
typedef int (*ctss_onu_get_cmc_channel_s16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *out_s16);
typedef int (*ctss_onu_set_cmc_channel_s16_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int in_s16);
typedef int (*ctss_onu_get_cmc_channel_u32_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *out_u32);
typedef int (*ctss_onu_set_cmc_channel_u32_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long in_u32);

typedef int (*ctss_onu_get_cmc_freq_auto_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long *base_freq, unsigned long *step_freq, unsigned char *step_mode);
typedef int (*ctss_onu_set_cmc_freq_auto_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode);

typedef int (*ctss_onu_dump_cmc_group_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
typedef int (*ctss_onu_set_cmc_group_u8_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char in_u8);
typedef int (*ctss_onu_set_cmc_group_channel_list_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids);
typedef int (*ctss_onu_set_cmc_group_modem_list_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end);
typedef int (*ctss_onu_set_cmc_modem_list_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end);

typedef int (*ctss_onu_op_cm_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac);
typedef int (*ctss_onu_dump_cm_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len);
typedef int (*ctss_onu_set_cm_channel_list_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids);

typedef int (*ctss_onu_create_cm_sf_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len);
typedef int (*ctss_onu_modify_cm_sf_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len);
typedef int (*ctss_onu_destroy_cm_sf_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid);

typedef int (*ctss_onu_dump_cmc_sfid_list_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len);

typedef int (*ctss_onu_create_cmc_sf_calsssname_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len);
typedef int (*ctss_onu_destroy_cmc_sf_calsssname_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name);

typedef int (*ctss_onu_get_cmc_mactbl_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table);
typedef int (*ctss_onu_get_cm_mactbl_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table);
typedef int (*ctss_onu_reset_cm_mactbl_f)(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type);
#endif

#endif

#if 1
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
/*------------------------OMCI----------------------------*/
typedef int(*ctss_onu_get_onu_cfg_f)(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig);
typedef int(*ctss_onu_set_onu_cfg_f)(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig);

#endif

typedef struct stOnuMgmtIFs {
#if 1
/* -------------------ONU基本API------------------- */
    ctss_onu_is_valid_f                       OnuIsValid;
    ctss_onu_is_online_f                      OnuIsOnline;
    ctss_onu_add_onu_manual_f	              AddOnuByManual;
    ctss_onu_modify_onu_manual_f              ModifyOnuByManual;
    ctss_onu_del_onu_manual_f	              DelOnuByManual;
    ctss_onu_add_gpon_onu_manual_f            AddGponOnuByManual;
    ctss_onu_is_support_cmd_f                 CmdIsSupported;

    ctss_onu_copy_onu_f                       CopyOnu;
    ctss_onu_get_iftype_f                     GetIFType;
    ctss_onu_set_iftype_f                     SetIFType;
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    ctss_onu_deregister_onu_f                 DeregisterOnu;
    ctss_onu_set_mac_auth_f                   SetMacAuthMode;
    ctss_onu_del_bind_onu_f                   DelBindingOnu;
#if 0
    ctss_onu_add_onu_pending_f                AddPendingOnu;
    ctss_onu_del_onu_pending_f                DelPendingOnu;
    ctss_onu_del_onu_pending_conf_f	          DelConfPendingOnu;
#endif
    ctss_onu_authorize_onu_f                  AuthorizeOnu;
    ctss_onu_auth_request_f                   AuthRequest;   
    ctss_onu_auth_success_f                   AuthSucess;  
    ctss_onu_auth_failure_f                   AuthFail;
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    ctss_onu_set_traffic_service_f            SetOnuTrafficServiceMode;
    ctss_onu_set_onu_peer_to_peer_f           SetOnuPeerToPeer;
    ctss_onu_set_onu_peer_to_peer_forward_f	  SetOnuPeerToPeerForward;
    ctss_onu_set_onu_bw_f	                  SetOnuBW;
    ctss_onu_get_onu_sla_f                    GetOnuSLA;

    ctss_onu_set_fec_mode_f                   SetOnuFecMode;
    ctss_onu_vlan_mode_f                      GetOnuVlanMode;
	/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ctss_onu_set_uni_port_f                   SetUniPort;
	ctss_onu_set_slow_protocol_limit_f        SetSlowProtocolLimit;
	ctss_onu_get_slow_protocol_limit_f        GetSlowProtocolLimit;
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    
    ctss_onu_get_bw_f                         GetBWInfo;
    ctss_onu_get_mode_f                       GetOnuB2PMode;
    ctss_onu_set_mode_f                       SetOnuB2PMode;
#endif

#if 1
/* -------------------ONU 监控统计管理API------------------- */
    ctss_onu_reset_counters_f                 ResetCounters;
    ctss_onu_pon_loopback_f                   SetPonLoopback;
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    ctss_onu_get_llid_params_f                GetLLIDParams;
    ctss_onu_start_encryption_f               StartEncryption;
    ctss_onu_stop_encryption_f                StopEncryption;
    ctss_onu_set_encrypt_params_f             SetOnuEncryptParams;
    ctss_onu_get_encrypt_params_f             GetOnuEncryptParams;
    
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_onu_update_encryption_key_f          UpdateEncryptionKey;
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    ctss_onu_get_onu_mac_addrtbl_f            GetOnuMacAddrTbl;
    ctss_onu_get_olt_mac_addrtbl_f            GetOltMacAddrTbl;
	ctss_onu_get_olt_mac_addrtbl_vlan_f       GetOltMacAddrVlanTbl;/*for gpon 获取mac+vid by jinhl*/
    ctss_onu_set_onu_max_mac_f                SetOnuMaxMac;
    ctss_onu_uni_port_config_ver_f            GetOnuUniMacCfg;
    ctss_onu_mac_check_flag_f                 GetOnuMacCheckFlag;
    ctss_onu_get_all_eth_port_mac_counter_f   GetAllEthPortMacCounter;    
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    ctss_onu_get_distance_f                   GetOnuDistance;
    ctss_onu_get_opticap_f                    GetOpticalCapability;
#endif

#if 1
/* -------------------ONU 倒换API------------------------- */
    ctss_onu_set_onu_llid_f                   SetOnuLLID;
#endif
    
#if 1
/* -------------------ONU 设备管理API------------------- */
    ctss_onu_get_onu_ver_f                    GetOnuVer;
    ctss_onu_get_pon_ver_f		              GetOnuPonVer;
    ctss_onu_get_onu_reginfo_f                GetOnuRegisterInfo;
    ctss_onu_get_onu_i2c_info_f               GetOnuI2CInfo;
    ctss_onu_set_onu_i2c_info_f               SetOnuI2CInfo;
    
    ctss_onu_reset_onu_f                      ResetOnu;
    ctss_onu_set_onu_update_mode_f            SetOnuSWUpdateMode;
    ctss_onu_update_onu_software_f            OnuSwUpdate;
    ctss_onu_gw_ctc_sw_convert_f              OnuGwCtcSwConvert;
	ctss_onu_get_burn_image_complete_f        GetBurnImageComplete;
    
    ctss_onu_set_onu_device_name_f            SetOnuDeviceName;
    ctss_onu_set_onu_device_desc_f            SetOnuDeviceDesc;
    ctss_onu_set_onu_device_location_f        SetOnuDeviceLocation;
    ctss_onu_get_onu_all_port_statistic_data_f      GetOnuAllPortStatisticData;
#endif

#if 1
/* -------------------ONU CTC-PROTOCOL API----------------- */
    ctss_onu_get_ctc_version_f                GetCtcVersion;
    ctss_onu_get_firmware_version_f           GetFirmwareVersion;
    ctss_onu_get_serial_f                     GetSerialNumber;
    ctss_onu_get_chipset_f                    GetChipsetID;
    
    ctss_onu_get_onu_cap1_f                   GetOnuCap1;
    ctss_onu_get_onu_cap2_f                   GetOnuCap2;
    ctss_onu_get_onu_cap3_f                   GetOnuCap3;
    
    ctss_onu_update_onu_firmware_f            UpdateOnuFirmware;
    ctss_onu_active_onu_firmware_f            ActiveOnuFirmware;
    ctss_onu_commit_onu_firmware_f            CommitOnuFirmware;
    
    ctss_onu_start_encrypt_f				  StartEncrypt;
    ctss_onu_stop_encrypt_f					  StopEncrypt;
    
    ctss_onu_get_port_link_state_f			  GetEthPortLinkState;
    ctss_onu_get_port_admin_f				  GetEthPortAdminStatus;
    ctss_onu_set_port_admin_f 				  SetEthPortAdminStatus;
    
    ctss_onu_get_port_pause_f				  GetEthPortPause;
    ctss_onu_set_port_pause_f 				  SetEthPortPause;
    
    ctss_onu_get_port_auto_negotiation_f	  GetEthPortAutoNegotiation;
    ctss_onu_set_port_auto_negotiation_f 	  SetEthPortAutoNegotiation;
    ctss_onu_set_port_auto_negotiation_restart_f 	SetEthPortAutoNegotiationRestart;
    ctss_onu_get_port_auto_negotiation_tecability_f GetEthPortAnLocalTecAbility;
    ctss_onu_get_port_auto_negotiation_tecability_f GetEthPortAnAdvertisedTecAbility;

    ctss_onu_get_port_policing_f			  GetEthPortPolicing;
    ctss_onu_set_port_policing_f			  SetEthPortPolicing;

    ctss_onu_get_port_downstream_policing_f   GetEthPortDsPolicing;
    ctss_onu_set_port_downstream_policing_f   SetEthPortDsPolicing;

    ctss_onu_get_port_vlan_config_f 		  GetEthPortVlanConfig;
    ctss_onu_set_port_vlan_config_f 		  SetEthPortVlanConfig;
    ctss_onu_get_vlan_allport_configuration_f GetAllPortVlanConfig;
   
    ctss_onu_get_port_classification_and_mark_f	GetEthPortClassificationAndMark;
    ctss_onu_set_port_classification_and_mark_f	SetEthPortClassificationAndMark;
    ctss_onu_clr_port_classification_and_mark_f	ClearEthPortClassificationAndMarking;

    ctss_onu_get_port_multicast_vlan_f		  GetEthPortMulticatVlan;
    ctss_onu_set_port_multicast_vlan_f		  SetEthPortMulticatVlan;
    ctss_onu_clr_port_multicast_vlan_f        ClearEthPortMulticastVlan;

    ctss_onu_get_port_multicast_max_group_f	  GetEthPortMulticastGroupMaxNumber;
    ctss_onu_set_port_multicast_max_group_f   SetEthPortMulticastGroupMaxNumber;
    
    ctss_onu_get_port_multicast_tag_strip_f	  GetEthPortMulticastTagStrip;
    ctss_onu_set_port_multicast_tag_strip_f	  SetEthPortMulticastTagStrip;
    ctss_onu_get_all_multicast_tag_strip_f    GetAllPortMulticastTagStrip;
    
    ctss_onu_get_port_igmp_tag_oper_f         GetEthPortMulticastTagOper;
    ctss_onu_set_port_igmp_tag_oper_f         SetEthPortMulticastTagOper;
    ctss_onu_set_obj_igmp_tag_oper_f          SetObjMulticastTagOper;

    ctss_onu_get_multicast_control_f 	      GetMulticastControl;
    ctss_onu_set_multicast_control_f	      SetMulticastControl;
    ctss_onu_clr_multicast_control_f          ClearMulticastControl;
    
    ctss_onu_get_multicast_switch_f			  GetMulticatSwitch;
    ctss_onu_set_multicast_switch_f			  SetMulticatSwitch;
    
    ctss_onu_get_multicast_fast_leave_ability_f GetMulticastFastLeaveAbility;
    ctss_onu_get_multicast_fast_leave_f		  GetMulticastFastLeave;
    ctss_onu_set_multicast_fast_leave_f		  SetMulticastFastLeave;
    
    ctss_onu_get_onu_port_statistic_data_f    GetOnuPortStatisticData;
    ctss_onu_get_onu_port_statistic_state_f   GetOnuPortStatisticState;
    ctss_onu_set_onu_port_statistic_state_f   SetOnuPortStatisticState;

    ctss_onu_set_obj_alarm_adminstate_f       SetObjAlarmAdminState;
    ctss_onu_set_obj_alarm_threshold_f        SetObjAlarmThreshold;
    ctss_onu_get_dbareport_thresholds_f       GetDbaReportThreshold;
    ctss_onu_set_dbareport_thresholds_f       SetDbaReportThreshold;
    
    ctss_onu_get_mxu_mng_global_config_f      GetMngGlobalConfig;
    ctss_onu_set_mxu_mng_global_config_f      SetMngGlobalConfig;
    ctss_onu_get_mxu_mng_snmp_config_f        GetMngSnmpConfig;  
    ctss_onu_set_mxu_mng_snmp_config_f        SetMngSnmpConfig;
    
    ctss_onu_get_holdover_f                   GetHoldOver;
    ctss_onu_set_holdover_f                   SetHoldOver;
    ctss_onu_get_optical_transceiver_diagnosis_f   GetOptTransDiag;      
    ctss_onu_set_tx_power_supply_control_f    SetTxPowerSupplyControl;    
    
    ctss_onu_get_fec_ability_f                GetFecAbility;

    ctss_onu_get_iad_info                     GetIADInfo;
    ctss_onu_get_voip_iad_oper_status         GetVoipIadOperation;   
    ctss_onu_set_voip_iad_operation           SetVoipIadOperation;
    ctss_onu_get_GlobalParameterConfig        GetVoipGlobalConfig;
    ctss_onu_set_GlobalParameterConfig        SetVoipGlobalConfig;
    ctss_onu_get_voip_fax_config              GetVoipFaxConfig;
    ctss_onu_set_voip_fax_config              SetVoipFaxConfig;
    
    ctss_onu_get_voip_pots_status             GetVoipPotsStatus;
    ctss_onu_get_voip_port                    GetVoipPort;
    ctss_onu_set_voip_port                    SetVoipPort;      
    ctss_onu_get_voip_port2                   GetVoipPort2;
    ctss_onu_set_voip_port2                   SetVoipPort2;
    
    ctss_onu_get_H248ParameterConfig          GetH248Config;
    ctss_onu_set_H248ParameterConfig          SetH248Config;
    ctss_onu_get_H248UserTidInfo              GetH248UserTidConfig;
    ctss_onu_set_H248UserTidConfig            SetH248UserTidConfig;
    ctss_onu_get_H248RtpTidConfig             GetH248RtpTidConfig;
    ctss_onu_set_H248RtpTidConfig             SetH248RtpTidConfig;
    
    ctss_onu_get_sip_param_config             GetSipConfig;
    ctss_onu_set_sip_param_config             SetSipConfig;
    ctss_onu_set_sip_digit_map                SetSipDigitMap;
    ctss_onu_get_sip_user_param_config        GetSipUserConfig;
    ctss_onu_set_sip_user_param_config        SetSipUserConfig;
    ctss_onu_get_onu_port_stats_f             GetOnuPortStatsData;
	
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    ctss_onu_cli_call_f                       CliCall;

    ctss_onu_set_long_f                       SetMgtReset;
    ctss_onu_set_long_f                       SetMgtConfig;
    ctss_onu_set_long_f                       SetMgtLaser;
    ctss_onu_set_long_f                       SetTemperature;
    ctss_onu_set_long_f                       SetPasFlush;

    ctss_onu_set_long_f                       SetAtuAgingTime;
    ctss_onu_set_long_f                       SetAtuLimit;

    ctss_onu_set_long_f                       SetPortLinkMon;
    ctss_onu_set_long_f                       SetPortModeMon;
    ctss_onu_set_long_f                       SetPortIsolate;

    ctss_onu_set_long_f                       SetVlanEnable;
    ctss_onu_set_port_long_f                  SetVlanMode;
    ctss_onu_set_long_f                       AddVlan;
    ctss_onu_set_long_f                       DelVlan;
    ctss_onu_set_port_long_f                  SetPortPvid;

    ctss_onu_set_vlan_port_add_f              AddVlanPort;
    ctss_onu_set_vlan_port_del_f              DelVlanPort;
    ctss_onu_set_vlan_transation_f            SetEthPortVlanTran;
    ctss_onu_set_vlan_transation_del_f        DelEthPortVlanTran;
    ctss_onu_set_vlan_agg_f                   SetEthPortVlanAgg;
    ctss_onu_set_vlan_agg_del_f               DelEthPortVlanAgg;    
    
    ctss_onu_set_vlan_qinq_enable_f           SetPortQinQEnable;
    ctss_onu_set_vlan_qinq_add_f              AddQinQVlanTag;
    ctss_onu_set_vlan_qinq_del_f              DelQinQVlanTag;   

    ctss_onu_set_port_long_f                  SetPortVlanFrameTypeAcc;
    ctss_onu_set_port_long_f                  SetPortIngressVlanFilter;

    ctss_onu_set_port_long_f                  SetPortMode;
    ctss_onu_set_port_long_f                  SetPortFcMode;
    ctss_onu_set_port_long_f                  SetPortAtuLearn;
    ctss_onu_set_port_long_f                  SetPortAtuFlood;
    ctss_onu_set_port_long_f                  SetPortLoopDetect;
    ctss_onu_set_port_long_f                  SetPortStatFlush;
    
    ctss_onu_set_long_f                       SetIngressRateLimitBase;
	ctss_onu_set_port_ingress_rate_f          SetPortIngressRate;
    ctss_onu_set_port_long_f                  SetPortEgressRate;
    
    ctss_onu_set_qos_class_f                  SetQosClass;
    ctss_onu_clr_qos_class_f                  ClrQosClass;
    ctss_onu_set_qos_rule_f                   SetQosRule;
    ctss_onu_clr_qos_rule_f                   ClrQosRule;
    
    ctss_onu_set_port_long_f                  SetPortQosRule;
    ctss_onu_set_port_long_f                  ClrPortQosRule;
    ctss_onu_set_port_long_f                  SetPortQosRuleType;
    
    ctss_onu_set_port_long_f                  SetPortDefPriority;
    ctss_onu_qos_prioreplace_f                SetPortNewPriority;
    ctss_onu_set_vlan_port_del_f              SetQosPrioToQueue;
    ctss_onu_set_vlan_port_del_f              SetQosDscpToQueue;
    
    ctss_onu_set_port_long_f                  SetPortUserPriorityEnable;
    ctss_onu_set_port_long_f                  SetPortIpPriorityEnable;
    ctss_onu_set_long_f                       SetQosAlgorithm;
    ctss_onu_setqosrulemode_f                 SetQosRuleMode;   /*for gw onu*/
    ctss_onu_setrule_f                        SetRule;          /*for gw onu*/

    ctss_onu_set_long_f                       SetIgmpEnable;
    ctss_onu_set_long_f                       SetIgmpAuth;
    ctss_onu_set_long_f                       SetIgmpHostAge;
    ctss_onu_set_long_f                       SetIgmpGroupAge;	
    ctss_onu_set_long_f                       SetIgmpMaxResTime;
    
    ctss_onu_set_long_f                       SetIgmpMaxGroup;
    ctss_onu_add_igmpgroup_f                  AddIgmpGroup;
    ctss_onu_delete_igmpgroup_f               DeleteIgmpGroup;  /*new added by luh       2011.7.13*/
    ctss_onu_set_port_long_f	              SetPortIgmpFastLeave;
    ctss_onu_set_port_long_f                  SetPortMulticastVlan;

    ctss_onu_set_portmirror_long_f            SetPortMirrorFrom;/*new added by luh     2011.7.13*/
    ctss_onu_set_port_long_f                  SetPortMirrorTo;
    ctss_onu_set_long_f                       DeleteMirror;
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    ctss_onu_op_cmc_f                         RegisterCmc;
    ctss_onu_op_cmc_f                         UnregisterCmc;
    ctss_onu_dump_cmc_f                       DumpCmc;
    ctss_onu_dump_cmc_f                       DumpCmcAlarms;
    ctss_onu_dump_cmc_f                       DumpCmcLogs;

    ctss_onu_op_cmc_f                         ResetCmcBoard;
    ctss_onu_get_cmc_ver_f                    GetCmcVersion;
    ctss_onu_get_cmc_u16_f                    GetCmcMaxMulticasts;
    ctss_onu_get_cmc_u16_f                    GetCmcMaxCm;
    ctss_onu_set_cmc_u16_f                    SetCmcMaxCm;

    ctss_onu_get_cmc_tm_f                     GetCmcTime;
    ctss_onu_set_cmc_tm_f                     SetCmcTime;
    ctss_onu_op_cmc_f                         LocalCmcTime;
    ctss_onu_set_cmc_cfg_f                    SetCmcCustomConfig;
    ctss_onu_dump_cmc_channel_f               DumpCmcCustomConfig;

    ctss_onu_dump_cmc_channel_f               DumpCmcDownChannel;
    ctss_onu_dump_cmc_channel_f               DumpCmcUpChannel;
    ctss_onu_get_cmc_channel_u8_f             GetCmcDownChannelMode;
    ctss_onu_set_cmc_channel_u8_f             SetCmcDownChannelMode;
    ctss_onu_get_cmc_channel_u8_f             GetCmcUpChannelMode;
    
    ctss_onu_set_cmc_channel_u8_f             SetCmcUpChannelMode;
    ctss_onu_get_cmc_channel_u8_f             GetCmcUpChannelD30Mode;
    ctss_onu_set_cmc_channel_u8_f             SetCmcUpChannelD30Mode;
    ctss_onu_get_cmc_channel_u32_f            GetCmcDownChannelFreq;
    ctss_onu_set_cmc_channel_u32_f            SetCmcDownChannelFreq;
   
    ctss_onu_get_cmc_channel_u32_f            GetCmcUpChannelFreq;
    ctss_onu_set_cmc_channel_u32_f            SetCmcUpChannelFreq;
    ctss_onu_get_cmc_freq_auto_f              GetCmcDownAutoFreq;
    ctss_onu_set_cmc_freq_auto_f              SetCmcDownAutoFreq;
    ctss_onu_get_cmc_freq_auto_f              GetCmcUpAutoFreq;

    ctss_onu_set_cmc_freq_auto_f              SetCmcUpAutoFreq;
    ctss_onu_get_cmc_channel_u32_f            GetCmcUpChannelWidth;
    ctss_onu_set_cmc_channel_u32_f            SetCmcUpChannelWidth;
    ctss_onu_get_cmc_channel_u8_f             GetCmcDownChannelAnnexMode;
    ctss_onu_set_cmc_channel_u8_f             SetCmcDownChannelAnnexMode;

    ctss_onu_get_cmc_channel_u8_f             GetCmcUpChannelType;
    ctss_onu_set_cmc_channel_u8_f             SetCmcUpChannelType;
    ctss_onu_get_cmc_channel_u8_f             GetCmcDownChannelModulation;
    ctss_onu_set_cmc_channel_u8_f             SetCmcDownChannelModulation;
    ctss_onu_get_cmc_channel_u8_f             GetCmcUpChannelProfile;
    
    ctss_onu_set_cmc_channel_u8_f             SetCmcUpChannelProfile;
    ctss_onu_get_cmc_channel_u8_f             GetCmcDownChannelInterleaver;
    ctss_onu_set_cmc_channel_u8_f             SetCmcDownChannelInterleaver;
    ctss_onu_get_cmc_channel_s16_f            GetCmcDownChannelPower;
    ctss_onu_set_cmc_channel_s16_f            SetCmcDownChannelPower;
    
    ctss_onu_get_cmc_channel_s16_f            GetCmcUpChannelPower;
    ctss_onu_set_cmc_channel_s16_f            SetCmcUpChannelPower;
    ctss_onu_dump_cmc_channel_f               DumpCmcUpChannelPower;
    ctss_onu_dump_cmc_channel_f               DumpCmcUpChannelSignalQuality;
    ctss_onu_dump_cmc_channel2_f              DumpCmcInterfaceUtilization;

    ctss_onu_dump_cmc_channel2_f              DumpCmcInterfaceStatistics;
    ctss_onu_dump_cmc_f                       DumpCmcMacStatistics;
    ctss_onu_dump_cmc_f                       DumpCmcAllInterface;
    ctss_onu_dump_cmc_f                       DumpCmcAllLoadBalancingGrp;
    ctss_onu_dump_cmc_group_f                 DumpCmcLoadBalancingGrp;

    ctss_onu_dump_cmc_group_f                 DumpCmcLoadBalancingGrpDownstream;
    ctss_onu_dump_cmc_group_f                 DumpCmcLoadBalancingGrpUpstream;
    ctss_onu_dump_cmc_f                       DumpCmcLoadBalancingDynConfig;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynMethod;
    ctss_onu_set_cmc_u32_f                    SetCmcLoadBalancingDynPeriod;

    ctss_onu_set_cmc_u32_f                    SetCmcLoadBalancingDynWeightedAveragePeriod;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynOverloadThresold;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynDifferenceThresold;
    ctss_onu_set_cmc_u32_f                    SetCmcLoadBalancingDynMaxMoveNumber;
    ctss_onu_set_cmc_u32_f                    SetCmcLoadBalancingDynMinHoldTime;

    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynRangeOverrideMode;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynAtdmaDccInitTech;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynScdmaDccInitTech;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynAtdmaDbcInitTech;
    ctss_onu_set_cmc_u8_f                     SetCmcLoadBalancingDynScdmaDbcInitTech;

    ctss_onu_set_cmc_group_u8_f               CreateCmcLoadBalancingGrp;
    ctss_onu_set_cmc_u8_f                     DestroyCmcLoadBalancingGrp;
    ctss_onu_set_cmc_group_channel_list_f     AddCmcLoadBalancingGrpDownstream;
    ctss_onu_set_cmc_group_channel_list_f     RemoveCmcLoadBalancingGrpDownstream;
    ctss_onu_set_cmc_group_channel_list_f     AddCmcLoadBalancingGrpUpstream;

    ctss_onu_set_cmc_group_channel_list_f     RemoveCmcLoadBalancingGrpUpstream;
    ctss_onu_set_cmc_group_modem_list_f       AddCmcLoadBalancingGrpModem;
    ctss_onu_set_cmc_group_modem_list_f       RemoveCmcLoadBalancingGrpModem;
    ctss_onu_set_cmc_modem_list_f             AddCmcLoadBalancingGrpExcludeModem;
    ctss_onu_set_cmc_modem_list_f             RemoveCmcLoadBalancingGrpExcludeModem;

    ctss_onu_dump_cmc_group_f                 DumpCmcLoadBalancingGrpModem;
    ctss_onu_dump_cmc_group_f                 DumpCmcLoadBalancingGrpActivedModem;
    ctss_onu_dump_cmc_f                       DumpCmcLoadBalancingGrpExcludeModem;
    ctss_onu_dump_cmc_f                       DumpCmcLoadBalancingGrpExcludeActivedModem;
    ctss_onu_op_cmc_f                         ReserveCmcLoadBalancingGrp;

    ctss_onu_dump_cmc_f                       DumpAllCm;
    ctss_onu_dump_cm_f                        DumpCm;
    ctss_onu_dump_cmc_f                       DumpAllCmHistory;
    ctss_onu_dump_cm_f                        DumpCmHistory;
    ctss_onu_op_cmc_f                         ClearAllCmHistory;

    ctss_onu_op_cm_f                          ResetCm;
    ctss_onu_dump_cm_f                        DumpCmDownstream;
    ctss_onu_dump_cm_f                        DumpCmUpstream;
    ctss_onu_set_cm_channel_list_f            SetCmDownstream;
    ctss_onu_set_cm_channel_list_f            SetCmUpstream;

    ctss_onu_create_cm_sf_f                   CreateCmServiceFlow;
    ctss_onu_modify_cm_sf_f                   ModifyCmServiceFlow;
    ctss_onu_destroy_cm_sf_f                  DestroyCmServiceFlow;   
    ctss_onu_dump_cm_f                        DumpCmClassifier;
    ctss_onu_dump_cm_f                        DumpCmServiceFlow;
    
    ctss_onu_dump_cmc_sfid_list_f             DumpCmcClassifier;
    ctss_onu_dump_cmc_sfid_list_f             DumpCmcServiceFlow;
    ctss_onu_dump_cmc_sfid_list_f             DumpCmcServiceFlowStatistics;
    ctss_onu_dump_cmc_f                       DumpCmcDownChannelBondingGroup;
    ctss_onu_dump_cmc_f                       DumpCmcUpChannelBondingGroup;

    ctss_onu_create_cmc_sf_calsssname_f       CreateCmcServiceFlowClassName;
    ctss_onu_destroy_cmc_sf_calsssname_f      DestroyCmcServiceFlowClassName;
    ctss_onu_get_cmc_mactbl_f                 GetCmcMacAddrTbl;
    ctss_onu_get_cm_mactbl_f                  GetCmMacAddrTbl;
    ctss_onu_reset_cm_mactbl_f                ResetCmAddrTbl;
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
/*---------------------GPON OMCI------------------------------*/
    ctss_onu_get_onu_cfg_f                    GetGponOnuCfg;
    ctss_onu_set_onu_cfg_f                    SetGponOnuCfg;  
#endif
    
   ctss_Onu_Set_MulticastTemplate_f  SetMulticastTemplate;
   ctss_Onu_Get_MulticastTemplate_f  GetMulticastTemplate;
   ctss_Onu_SetMcastOperProfile_f	SetMcastOperProfile;
   ctss_Onu_GetMcastOperProfile_f	GetMcastOperProfile;
   ctss_Onu_SetUniPortAssociateMcastPro_f  SetUniPortAssociateMcastProf;
   ctss_Onu_GetUniPortAssociateMcastPro_f GetUniPortAssociateMcastProf;
/**************Dr.Peng*****************/
   ctss_Onu_SetOnuPortIsolation SetOnuPortIsolation;
   ctss_Onu_GetOnuPortIsolation GetOnuPortIsolation;
   ctss_Onu_GetOnuMacEntry GetOnuMacEntry;
   ctss_Onu_SetOnuPortSaveConfig SetOnuPortSaveConfig;
   ctss_Onu_SetOnuLoopDetectionTime SetOnuLoopDetectionTime;
   
   ctss_Onu_GetOnuLoopDetectionTime GetOnuLoopDetectionTime;
   ctss_Onu_SetOnuPortMode SetOnuPortMode;
   ctss_Onu_GetOnuPortMode GetOnuPortMode;
   ctss_Onu_SetOnuPortStormStatus SetOnuPortStormStatus;
   ctss_Onu_SetOnuPortStormStatus GetOnuPortStormStatus;
   
   ctss_Onu_SetOnuDeviceName PengSetOnuDeviceLocation;
   ctss_Onu_GetOnuDeviceName GetOnuDeviceLocation;
   ctss_Onu_SetOnuDeviceName SetOnuDeviceDescription;
   ctss_Onu_GetOnuDeviceName GetOnuDeviceDescription;
   ctss_Onu_SetOnuDeviceName PengSetOnuDeviceName;
   
   ctss_Onu_GetOnuDeviceName PengGetOnuDeviceName;
   ctss_Onu_GetOnuPortMacNumber GetOnuPortMacNumber;
   ctss_Onu_GetOnuPortLocationByMAC GetOnuPortLocationByMAC;
   ctss_Onu_GetOnuSupportExtendAtt GetOnuSupportExtendAtt;
/* -------------------END---------------------------- */

    ctss_onu_del_onu_manual_f                 LastFun;
}OnuMgmtIFs;
/* E--added by shixh@2010-5-13 for ONUMGT-API-IF */

typedef struct DrPengSupportAttribute{
	unsigned char get_flag;	/*是否发送过支持属性的报文*/
	unsigned char support_flag;
	unsigned char  SupportAttribute[2];
}__attribute__((packed))DrPengSupportAttribute_S;
/*begin: 支持管理IP的配置保存，mod by liuyh, 2017-5-4*/
typedef struct ONUMngIPConf{
    ULONG ip;
    ULONG mask;
    ULONG gw;    
    USHORT cVlan;
    USHORT sVlan;
    UCHAR  pri;
}__attribute__((packed))ONUMngIPConf_t;
/*end: mod by liuyh, 2017-5-4*/

typedef struct  ONUTable{
 	const OnuMgmtIFs  *OnuIFs;    /* onu 管理的操作接口 */
	IF_Index_S  Index; 
	/*IF_Index_S LLIDIfIndex;*/
	short int PonIdx;
	short int LLID;
	
	UCHAR IsAutoDelete;			/*老化onu自动删除标志位，0为删除，1为不删除*/
	UCHAR IsGponOnu;                /*GPON ONU标识 1为gpon : 0为epon*/
	UCHAR TcontNum;
	UCHAR GEMPortNum;
	UCHAR OmccVersion;

	UCHAR PmEnable;
	UCHAR ImageValidIndex;      /*1: image 0       2: image 1*/
	USHORT GemPortId;
	ULONG BerInterval;
#if 0	
	short int Llid;
	unsigned int  LlidType;
	unsigned char LlidDesc[MAXLLIDDESCLEN];
	unsigned int LlidDescLen;
#endif

	DeviceInfo_S  DeviceInfo;
	UCHAR  PonType; 
	UCHAR  PonRate;         /*added by liwei056 2014-9-4*/

    /*int RecvMsgFromOnuLen;*/ /*del by luh 2012-10-31*/
    UCHAR WaitOnuEUQMsgFlag;
    UCHAR OnuAbility;         /*added by luh 2012-10-31*/
    ULONG  ctcAlarmstatus;          /*CTC ONU 告警状态 2011-8-29*/
	UCHAR  AdminStatus;             /* ONU 管理状态,enable/disable */
	UCHAR  OperStatus;              /* ONU 运行状态,UP/DOWN */
	UCHAR  TrafficServiceEnable;    /* ONU 去激活标志*/
	UCHAR  ExtOAM;                  /* ONU 扩展OAM ( CTC )是否发现完成*/
	UCHAR  SoftwareUpdateCtrl;      /* ONU 软件更新使能控制和状态*/
	UCHAR  SoftwareUpdateStatus;
	UCHAR  SoftwareUpdateType;
	UCHAR  NeedDeleteFlag ;
#if 0
	unsigned int  UplinkBandwidth_gr;  /* bandwidth, unit: Kbit/s*/
	unsigned int  UpLinkBandwidth_be;  /* unit: Kbit/s */
	unsigned int  DownlinkBandwidth_gr ; 
	unsigned int  DownlinkBandwidth_be ; 
#else
	unsigned int  FinalUplinkBandwidth_gr;  /* bandwidth, unit: Kbit/s*/
	unsigned int  FinalUplinkBandwidth_be;  /* unit: Kbit/s */
    unsigned int  FinalUplinkBandwidth_fixed;        
	unsigned int  FinalDownlinkBandwidth_gr; 
	unsigned int  FinalDownlinkBandwidth_be; 
    
	UCHAR  FinalUplinkClass;
	UCHAR  FinalUplinkDelay;
	UCHAR  FinalDownlinkClass;
	UCHAR  FinalDownlinkDelay;

	ULONG  BandWidthIsDefault;
	ULONG  ActiveUplinkBandwidth;
	ULONG  ActiveDownlinkBandwidth;
#endif
	/* modified by zhuyf, according to CTC STACK API, 2007-3-19 */

	UCHAR  EncryptEnable;	/* 加密使能 */
	UCHAR  EncryptStatus;
	UCHAR  EncryptDirection ; /* 加密方向*/
	UCHAR  EncryptFirstTime;  /* 是否为第一次启动加密 */

	unsigned int  EncryptKeyTime;   /*密钥交互时间*/
	unsigned short int EncryptNoReplyTimeout; /* 超时时间， 0 - 2550，单位为0.1s, 默认值30，即3s */
	/*unsigned int EncryptKeyCounter;    更新密钥次数*/
	unsigned int  EncryptCounter;
	PON_encryption_key_t  EncryptKey;
	
	/* end */

	unsigned char  GrantNumber; /* Max number of grant records an ONU can store*/
	unsigned int  RTT;                                 /* ONU Rount-Trip Time */
#if 0
	unsigned int  MaxMAC;
#endif

       /*AlarmInfo_S AlarmInfo[MAX_ONU_ALARM_NUM]; */
	
	unsigned long  AlarmStatus;	/*[MAX_ONU_ALARM_NUM / 8]*/
	unsigned long  AlarmMask;		/* ONU PON 告警屏蔽*/
	unsigned long  devAlarmMask;	/* added by xieshl 20070927, ONU设备告警屏蔽, 解决问题单#5457 */

	PON_olt_monitoring_configuration_t  AlarmConfigInfo;
	
	unsigned char  Surrounding [3];            /*ONU环境运行参数 */             

	MacTable_S  StaticMac;
	unsigned char StaticMacNum;
	
	/* 2007/6/1 增加ONU 数据包原MAC 过滤*/
	MacTable_S  *FilterSaMac;
	unsigned char  FilterSaMacNum;   

	/* 2007/6/8 增加ONU 数据包IP/port 过滤*/
	IpTable_S   *Filter_SIp;
	IpTable_S   *Filter_DIp;
	Ip_Port_Table_S  *Filter_SIp_udp;
	Ip_Port_Table_S  *Filter_DIp_udp;
	Ip_Port_Table_S  *Filter_SIp_tcp;
	Ip_Port_Table_S  *Filter_DIp_tcp;

	/* 2007/6/12 增加ONU数据包VID过滤*/
	VlanId_Table_S  *Filter_Vid;

	/* 2007-6-14   增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤 */
	Ether_Type_Table_S  *Filter_Ether_Type;
	Ip_Type_Table_S  *Filter_Ip_Type;
	
    /* B--added by liwei056@2012-7-5 for OnuOpticSwitch */
	unsigned char ProtectType;
    /* E--added by liwei056@2012-7-5 for OnuOpticSwitch */

	unsigned char PowerStatus;
	unsigned char HaveBattery;
	unsigned char BatteryCapability;
	/* OnuLoopData_S  LoopInfo;          Onu环回相关信息 */

	unsigned char  BerFlag;
	unsigned char  FerFlag;

	unsigned char UsedFlag ;
	unsigned char RegisterFlag;
	unsigned char RegisterTrapFlag;

	unsigned char  PowerOn;
	unsigned char  PowerOffCounter;

	unsigned char  SequenceNo[PON_AUTHENTICATION_SEQUENCE_SIZE];
	OAM_standard_version_t OAM_Ver;

	unsigned short int Laser_ON; /*Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	unsigned short int Laser_OFF;/*Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	/* follow two field have no meaning for onu */
	unsigned short int AGC_Time;/*PON upstream data AGC lock time, measured in TQ,*/
	unsigned short int CDR_Time;/*PON RX signal synchronization time, measured in TQ,*/
	
	OnuLLIDTable_S  LlidTable[MAXLLIDPERONU];
	unsigned char   llidNum;
	unsigned char   llidMax;
	unsigned char   cmMax;

	unsigned char BurningWait;
	unsigned int swFileLen;	/*ONU更新时要下载的文件的长度*/
	unsigned int transFileLen;	/*ONU更新时已经下载的文件长度*/
	unsigned int BurningFlash; 
	struct vty *vty;
	
	onuupdateclk_t updaterec;

	unsigned char  PeerToPeer[MAXONUPERPONNOLIMIT/8];/*modi by luh@2015-6-18,p2p的配置应该随着maxonu进行改变*/
	UCHAR address_not_found_flag;
	UCHAR broadcast_flag;

	/* 2007-02-07 added by chenfj */
#ifdef   V2R1_ONU_AUTO_CONFIG 
	int AutoConfig;
	unsigned char AutoConfigFile[AUTO_CONFIG_NAME_LEN];
	int AutoConfigFlag;
#endif
	unsigned char GetEUQflag;/*不需要*/
	unsigned char GetEUQCounter;

	unsigned int LastFerAlarmTime;
	unsigned int LastBerAlarmTime;
	/* ONU capabilities, 2007-3-17 added by zhuyf */

	unsigned char       device_vendor_id[4];
	unsigned long int   onu_model;
	unsigned char		chip_vendor_id[CTC_VENDOR_ID_LENGTH]; /* PMC-SIERRA value: ASCII 'E6' */
	unsigned short		chip_model;
	unsigned char		revision;
	unsigned char		date[3];
	unsigned char		hardware_version[8+1];
	unsigned char		software_version[16+1];
	unsigned short      firmware_version;
	unsigned char		extendedModel[16+1];

	unsigned char GE_supporting;                 /* 0 - not supporting, 1 - supporting */
	unsigned char FE_supporting;                 /* 0 - not supporting, 1 - supporting */
	unsigned char VoIP_supporting;               /* 0 - not supporting, 1 - supporting */
	unsigned char TDM_CES_supporting;           /* 0 - not supporting, 1 - supporting */
	
	unsigned char ctc_onu_type;
	unsigned char GE_Ethernet_ports_number;    /* 0 - 256 */
	unsigned char FE_Ethernet_ports_number;    /* 0 - 256 */
	unsigned char POTS_ports_number;            /* 0 - 256 */
	unsigned char E1_ports_number;              /* 0 - 256 */

	unsigned char ADSL_ports_number;
	unsigned char VDSL_ports_number;
	unsigned char WLAN_ports_number;
	unsigned char USB_ports_number;
	unsigned char CATV_ports_number;

	unsigned char  Ports_distribution[16];     /* 分别表示接口1-64的接口类型，每2bit代表一个端口，1 - FE, 2 - GE, 0 - unknow */
		                                          /* Ports_distribution[0]的bit0~1代表第1个端口，bit2~3代表第2个端口，依此类推 */

	unsigned char Upstream_queues_number;                 /* 1 - 256, 0代表256 */
	unsigned char Max_upstream_queues_per_port;          /* 1 - 256, 0代表256 */
	unsigned char Upstream_queues_allocation_increment; /* 1 - 256, 0代表256 */
	unsigned char Downstream_queues_number;               /* 1 - 256, 0代表256 */
	unsigned char Max_downstream_queues_per_port;        /* 1 - 256, 0代表256 */
	unsigned char multicastSwitch;  /* 01 - IGMP snooping, 02 - CTC */
	unsigned char onu_ctc_version;	/* added by xieshl 20110414 */

	unsigned char fastleaveControl;
	unsigned char fastleaveAbility;
	
	/* end */

	/*add by wangxy 2007-03-30*/
	UCHAR	stpEnable;
	/*end*/
	UCHAR  CTC_EncryptCtrl;   /* 2 - start, 3 - stop */ 
	UCHAR  CTC_EncryptStatus;  /* 1 - noop,  4 - encrypting */

	UCHAR  FEC_ability;
	UCHAR  FEC_Mode;
	UCHAR  LoopbackEnable;

	/* added by chenfj 2008-3-25 */
	Onu_vlan_priority_t vlan_priority;

	/* added by chenfj 2008-6-11 
	   扩展ONU 子卡管理
	unsigned char SubSlotNumMax;
	unsigned char SubSlotNum;
	Onu_SubBoard_Info_t  SubSlot[ONU_SUB_BOARD_MAX_NUMBER];
	*/
	
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	ONUMeteringTable_S ONUMeteringTable;
#endif
#if 0
	/*onu配置数据名称，通过对此字符串hash所得索引可获取该ONU的配置数据指针*/
	UCHAR configFileName[ONU_CONFIG_NAME_LEN];
	UCHAR configResult;
	UCHAR configRestoreFlag;
#endif
    /*Begin:for onu swap by jinhl@2013-04-27*/
    UCHAR virRegFailTimes;/*虚注册失败的次数*/
    /*End:for onu swap by jinhl@2013-04-27*/
	UCHAR needToRestoreConfFile;/*added by wangjiah@2017-01-18 flag to tag whether restoring config file is needed*/
	DrPengSupportAttribute_S  DrPengSupportAttribute;
    ONUMngIPConf_t mngIp;   /* 增加管理IP的配置保存, add by liuyh, 2017-5-12 */

   }__attribute__((packed))ONUTable_S;

/*add by shixh20100525*/
typedef enum{
    ONU_EVENT_IS_NOT_EXIST = 0,   
    ONU_EVENT_REGISTER = 1,    /*ONU注册*/
    ONU_EVENT_DEREGISTER = 2,  /*ONU去注册*/
	/* added by xieshl 20110728,  自动删除ONU 使能，ONU删除通知，ONU状态同步 */
    ONU_EVENT_DEL_ENABLE,
    ONU_EVENT_REPEATED_DEL,
    ONU_EVENT_STATUS_REPORT,
    ONU_EVENT_BW_BASED_MAC,
    ONU_EVENT_EXT_MGT_SYNC,
    ONU_EVENT_EXT_MGT_REQ,
    ONU_EVENT_ID_MAX
}ONU_EVENT_ID;

/* modified by xieshl 20101228, 为了避免板间同步报文分片，压缩部分变量定义，去掉了不需同步的可配数据变量 */
typedef struct{
	UCHAR  onuEventId;
	UCHAR  ponSlotIdx;
	UCHAR  portIdx;
	UCHAR  onuIdx;
} onu_sync_msg_head_t;

typedef struct  ONU_SYNC_REG_MSG{
	onu_sync_msg_head_t   OnuSyncMsgHead;
	   
	DeviceInfo_S  DeviceInfo;
	UCHAR  PonType; 
	UCHAR  PonRate; 

	UCHAR IsGponOnu;                /*GPON ONU标识 1为gpon : 0为epon*/
	UCHAR TcontNum;
	UCHAR GEMPortNum;
    UCHAR OmccVersion;

	UCHAR PmEnable;
	UCHAR ImageValidIndex;      /*1: image 0       2: image 1*/
	USHORT GemPortId;
	ULONG BerInterval;	
	
	/*int  AdminStatus;         */          /* ONU管理状态,enable/disable */
	UCHAR  OperStatus;                      /* ONU运行状态,UP/DOWN */
	/*UCHAR  TrafficServiceEnable;*/     /* onu 去激活标志*/
	UCHAR  ExtOAM;                            /* ONU 扩展OAM ( CTC )是否发现完成*/
	/*UCHAR  SoftwareUpdateCtrl;*/    /*ONU软件更新使能控制和状态*/
	/*UCHAR  SoftwareUpdateStatus;
	UCHAR  SoftwareUpdateType;*/
	UCHAR  NeedDeleteFlag ;
	UCHAR  OnuAbility; /*new added by luh 2012-11-1*/
        
	ULONG  AlarmStatus;	/*[MAX_ONU_ALARM_NUM / 8]*/
	/*unsigned long  AlarmMask;*/		/* ONU PON 告警屏蔽*/
	/*unsigned long  devAlarmMask;*/	/* added by xieshl 20070927, ONU设备告警屏蔽, 解决问题单#5457 */
    int    OnuhandlerStatus;        /* ONU 注册事件状态机控制变量2012-5-2*/
	UCHAR UsedFlag ;
	UCHAR RegisterFlag;
	UCHAR RegisterTrapFlag;
	/*unsigned char  Surrounding [3];   */         /*ONU环境运行参数 */    /*未使用*/     

	UCHAR  onuLlidRowStatus;
    
	UCHAR  SequenceNo[PON_AUTHENTICATION_SEQUENCE_SIZE];
	UCHAR  OAM_Ver;
    
	UCHAR  BerFlag;
	UCHAR  FerFlag;
    
	UCHAR  GrantNumber; /* Max number of grant records an ONU can store*/
	LONG   RTT;                                 /* ONU Rount-Trip Time */
#if 0
	unsigned int  MaxMAC;
#endif

	SHORT  onuLlid;
	UCHAR  llidNum;
	UCHAR  llidMax;
#if 0
	OnuLLIDTable_S  LlidTable[MAXLLIDPERONU];

	struct vty *vty;
	ULONG swFileLen;	/*ONU更新时要下载的文件的长度*/
	ULONG transFileLen;	/*ONU更新时已经下载的文件长度*/
	ULONG BurningFlash; 
	ULONG BurningWait;
	UCHAR  PeerToPeer[8];
	UCHAR address_not_found_flag;
	UCHAR broadcast_flag;
#endif

	/*onu capability*/
	USHORT Laser_ON; /*Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	USHORT Laser_OFF;/*Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	/* follow two field have no meaning for onu */
	USHORT AGC_Time;/*PON upstream data AGC lock time, measured in TQ,*/
	USHORT CDR_Time;/*PON RX signal synchronization time, measured in TQ,*/

	ULONG LastFerAlarmTime;
	ULONG LastBerAlarmTime;
	
	/*ctc*/
	UCHAR  device_vendor_id[4];
	ULONG  onu_model;
	UCHAR  chip_vendor_id[CTC_VENDOR_ID_LENGTH]; /* PMC-SIERRA value: ASCII 'E6' */
	USHORT chip_model;
	UCHAR  revision;
	UCHAR  date[3];
	UCHAR  hardware_version[8+1];
	UCHAR  software_version[16+1];
	USHORT firmware_version;
	UCHAR  extendedModel[16+1];

	UCHAR GE_supporting;                 /* 0 - not supporting, 1 - supporting */
	UCHAR FE_supporting;                 /* 0 - not supporting, 1 - supporting */
	UCHAR VoIP_supporting;               /* 0 - not supporting, 1 - supporting */
	UCHAR TDM_CES_supporting;           /* 0 - not supporting, 1 - supporting */
	
	UCHAR GE_Ethernet_ports_number;    /* 0 - 256 */
	UCHAR FE_Ethernet_ports_number;    /* 0 - 256 */
	UCHAR POTS_ports_number;            /* 0 - 256 */
	UCHAR E1_ports_number;              /* 0 - 256 */

	UCHAR ADSL_ports_number;
	UCHAR VDSL_ports_number;
	UCHAR WLAN_ports_number;
	UCHAR USB_ports_number;
	UCHAR CATV_ports_number;

	UCHAR  Ports_distribution[16];     /* 分别表示接口1-64的接口类型，每2bit代表一个端口，1 - FE, 2 - GE, 0 - unknow */
		                                          /* Ports_distribution[0]的bit0~1代表第1个端口，bit2~3代表第2个端口，依此类推 */

	UCHAR Upstream_queues_number;                 /* 1 - 256, 0代表256 */
	UCHAR Max_upstream_queues_per_port;          /* 1 - 256, 0代表256 */
	UCHAR Upstream_queues_allocation_increment; /* 1 - 256, 0代表256 */
	UCHAR Downstream_queues_number;               /* 1 - 256, 0代表256 */
	UCHAR Max_downstream_queues_per_port;        /* 1 - 256, 0代表256 */

	UCHAR ctc_version;
	UCHAR ProtectType;              /*for onu swap by jinhl@2013-04-27*/

#if 0
	UCHAR multicastSwitch;  /* 01 - IGMP snooping, 02 - CTC */
	UCHAR fastleaveControl;
	UCHAR fastleaveAbility;
	UCHAR stpEnable;
	UCHAR CTC_EncryptCtrl;   /* 2 - start, 3 - stop */ 
	UCHAR CTC_EncryptStatus;  /* 1 - noop,  4 - encrypting */
	UCHAR FEC_ability;
	UCHAR FEC_Mode;
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	ONUMeteringTable_S ONUMeteringTable;
#endif
#endif
}__attribute__((packed)) onu_sync_reg_msg_t;
/*end by shixh20100525*/

typedef struct  ONU_SYNC_DEREG_MSG{
	onu_sync_msg_head_t   OnuSyncMsgHead;
	int   OnuhandlerStatus;   
	UCHAR  OperStatus;			/* ONU运行状态,UP/DOWN */
	Date_S  SysLaunchTime ;		/*开通时间：格式：年、月、日、时、分、秒 */
    Date_S  SysOfflineTime;
	ULONG  SysUptime;			/*最近一次运行状态改变时的当前秒数*/
	ULONG  RelativeUptime;		/* 状态最近一次变化时距离系统启动时的TICK	数*/
}__attribute__((packed)) onu_sync_dereg_msg_t;

/* added by xieshl 20110728,  自动删除ONU 使能，ONU删除通知，ONU状态同步 */
typedef struct  {
	onu_sync_msg_head_t   OnuSyncMsgHead;
	ULONG syncValue;
}__attribute__((packed)) onu_sync_del_msg_t;

typedef  struct{
	onu_sync_msg_head_t   OnuSyncMsgHead;
	UCHAR  onuStatusList[CARD_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT];
} __attribute__((packed)) onu_sync_status_msg_t;




/*ONU绑定，add by shixh20100611*/
typedef struct OnuPonBanding{
	UCHAR slot;
	UCHAR port;
	UCHAR  mac_address[BYTES_IN_MAC_ADDRESS];
	USHORT pon_id;
	USHORT result;
}OnuPonBanding_t;

#define ONU_EVENTFLAG_REAL    0x01
#define ONU_EVENTFLAG_VIRTAL  0x02

typedef struct ONURegisterInfo{
	unsigned int  event_flags;
	PON_olt_id_t  olt_id;
	PON_onu_id_t	  onu_id; 
	unsigned char  mac_address[BYTES_IN_MAC_ADDRESS]; /* mac_address_t mac_address;*/
	unsigned char authenticatioin_sequence[PON_AUTHENTICATION_SEQUENCE_SIZE];
	/*PON_authentication_sequence_t  authentication_sequence;*/
	OAM_standard_version_t	  supported_oam_standard;
}ONURegisterInfo_S;

typedef struct gONURegisterInfo{
	unsigned int  event_flags;
	PON_olt_id_t  olt_id;
	PON_onu_id_t	  onu_id; 
	unsigned char  series_number[GPON_ONU_SERIAL_NUM_STR_LEN]; /* mac_address_t mac_address;*/
	unsigned char authenticatioin_sequence[PON_AUTHENTICATION_SEQUENCE_SIZE];
	/*PON_authentication_sequence_t  authentication_sequence;*/
	OAM_standard_version_t	  supported_oam_standard;
}gONURegisterInfo_S;

typedef struct ONUDeregistrationInfo{
	unsigned int  event_flags;
	PON_olt_id_t  olt_id;
	PON_onu_id_t  onu_id;
	PON_onu_deregistration_code_t  deregistration_code;
}ONUDeregistrationInfo_S;


typedef struct ONUStartEncryptionInfo{
	PON_olt_id_t olt_id;
	PON_onu_id_t llid;
	PON_start_encryption_acknowledge_codes_t ret_code;
}OnuStartEncryptionInfo_S;

typedef struct ONUStopEncryptionInfo{
	PON_olt_id_t olt_id;
	PON_onu_id_t  llid;
	PON_stop_encryption_acknowledge_codes_t ret_code;
}OnuStopEncryptionInfo_S;

typedef struct ONUUpdateEncryptkeyInfo{
	PON_olt_id_t  olt_id;
	PON_llid_t llid;
	PON_update_encryption_key_acknowledge_codes_t ret_code;	
}OnuUpdateEncryptkeyInfo_S;

typedef enum{
	ONU_OFFLINE=0,
	ONU_ONLINE  
}OnuStatus_S;


unsigned int Bata_ratio;

extern PON_address_record_t Mac_addr_table[PON_ADDRESS_TABLE_SIZE];
extern PON_address_vlan_record_t Mac_addr_vlan_table[PON_ADDRESS_TABLE_SIZE];
extern unsigned char GW_private_MAC[];

#if 0
extern  ONUTable_S  OnuMgmtTable[];  /* MAXONU */
#else
extern  ONUTable_S  *OnuMgmtTable;
#endif
/* RegisteredOnu[MAXPON][MAXONUPERPON];*/

extern unsigned long  OnuMgmtTableDataSemId;
extern int onu_mgmt_debug;

#define ONU_MGMT_SEM_TAKE		{\
	if( VOS_SemTake(OnuMgmtTableDataSemId, 5000) == VOS_ERROR )\
	{	if( onu_mgmt_debug ) sys_console_printf( "T_E:%s %d\r\n", __FILE__, __LINE__);} }
	/*else\
	{	if( onu_mgmt_debug ) sys_console_printf( "T_O:%s %d\r\n", __FILE__, __LINE__); }  }*/
	
#define ONU_MGMT_SEM_GIVE		{\
	if( VOS_SemGive(OnuMgmtTableDataSemId) == VOS_ERROR )\
	{	if( onu_mgmt_debug ) sys_console_printf( "G_E:%s %d\r\n", __FILE__, __LINE__);} }
	/*else\
	{	if( onu_mgmt_debug ) sys_console_printf( "G_O:%s %d\r\n", __FILE__, __LINE__); }  }*/

extern unsigned long  OnuEncryptSemId;
/*extern unsigned char Invalid_Mac_Addr[BYTES_IN_MAC_ADDRESS];
extern unsigned char Invalid_Mac_Addr1[BYTES_IN_MAC_ADDRESS];*/
/*extern unsigned char  *OnuDeregistrationReason[PON_ONU_DEREGISTRATION_LAST_CODE-1];
extern unsigned char *OnuActivatedMode[];*/
extern unsigned char *OnuCurrentStatus[] ;
extern unsigned char *v2r1AddrTableAgeingMode[];
extern unsigned char *OnuSwUpdateStatus[];
extern PON_olt_monitoring_configuration_t OnuDefaultAlarmConfiguration;

/*
extern unsigned char OnuUsedFlag[MAXONU];
extern unsigned char UsedFlagChange;
extern OnuMap_S OnuMapIdx[MAXONU];
extern unsigned char OnuMapChange;
*/
extern OnuConfigInfo_S OnuConfigDefault;
extern unsigned char *OnuCruuentStatus[];

/*extern char OnuVendorInfoDefault[];
extern char OnuDeviceNameDefault[];*/
#if 0
extern char OnuVendorLocationDefault[MAXVENDORLOCATIONLEN] ;
extern char OnuVendorContactDefault[MAXVENDORCONTACTLEN];
extern char OnuDeviceTypeDefault[MAXDEVICETYPELEN];
#endif
extern char OnuLocationDefault[ ];
extern char OnuDescDefault[];
/*extern unsigned char  OnuMacAddrDefault[6];*/
extern unsigned short int OnuIdxDefault;
extern unsigned int  MaxMACDefault;
extern unsigned int  UplinkBandwidthDefault;
extern unsigned int  DownlinkBandwidthDefault;	
extern unsigned int  EncryptTypeDefault;	
extern unsigned int  EncryptEnableDefault;	
extern unsigned int  EncryptStatusDefault;
extern unsigned int  EncryptDirectionDefault; 
extern unsigned int  EncryptKeyTimeDefault;   
extern unsigned char  CTC_EncryptKeyTimeDefault ;
extern unsigned char  CTC_EncryptTimeingThreshold;
extern unsigned short int  CTC_EncryptNoReplyTime;
extern unsigned long  AlarmMaskDefault;
extern MacTable_S  *StaticMacDefault;	
extern unsigned char  OnuOperStatusDefault ;
extern unsigned char  OnuAdminStatusDefault ;
extern unsigned char  OnuSoftwareUpdateCtrl;
extern unsigned char  OnuSoftwareUpdateFlag;

extern unsigned char DBA_name[];
extern unsigned char NameSize;
extern unsigned char DBA_Version[];
extern unsigned char VersionSize;
 
extern int  ThisIsValidOnu( short int PonPortIdx, short int OnuIdx );
extern int ThisIsInvalidMacAddr(unsigned char * MacAddr);

/* onu management Info */
/*extern int  ClearOnuMgmtTable();
extern int  ClearOneOnuByMAC( unsigned char *MacAddress );
extern int  ClearOneOnuByName( unsigned char *Name );
extern int  ClearOneOnuByIdx( short int OnuIdx );*/
extern int  ClearOnuLlidRunningData( short int PonPortIdx, short int OnuIdx, short int LLIDIdx );
extern int  ClearOnuRunningData( short int PonPortIdx, short int OnuIdx, int ClearFlags );
extern int  InitOnuConfigDefault();
extern int  InitOnuMgmtTableByDefault();
extern int  InitOneOnuByDefault( short int PonPortIdx, short int OnuIdx );
extern short int SearchFreeOnuIdxByPonPort(short int PonPort_id, short int *rep_flag);
extern int  SearchValidOnuEntry( short int PonPortIdx );

extern int  UpdateKeyFlag;
/*extern int  InitOnuMgmtTableFromFlash();*/


/*added by wangjiah@2017-01-18 to get/set needToRestoreConfFile flag*/
extern int	SetNeedToRestoreConfFile(short int PonPortIdx, short int OnuIdx, unsigned char isNeeded);
extern int	GetNeedToRestoreConfFile(short int PonPortIdx, short int OnuIdx, unsigned char *isNeeded);
/*added by wangjiah end*/
extern int  GetOnuType( short int PonPortIdx, short int OnuIdx, int *type );
extern int  GetOnuTypeString(short int PonPortIdx, short int OnuIdx, char *TypeString, int *len );
extern int  GetOnuOUI( short int PonPortIdx, short int OnuIdx, unsigned char *OUI );
extern int  SetOnuDeviceName( short int PonPortIdx, short int OnuIdx, char *Name, int  NameLen );
extern int  SetOnuDeviceNameMsg( short int PonPortIdx, short int OnuIdx,  char *Name,  int  NameLen );
extern int  SetOnuDeviceName_1( short int PonPortIdx, short int OnuIdx, char *Name, int  NameLen );
extern int  GetOnuDeviceName( short int PonPortIdx, short int OnuIdx,  char *Name , int *NameLen);

extern int  GetOnuDeviceNameByIdx( short int slot, short int port, short int  OnuIdx, char *OnuName, int *len  );
extern int  GetOnuDeviceIdxByName_OnePon( char *OnuName, int len, short  int slot, short int port, short int *OnuIdx1);
extern int  GetOnuDeviceIdxByName( char *OnuName, int len, short  int *slot, short int *port, short int *OnuIdx1);

extern int  SetOnuDeviceDesc( short int PonPortIdx, short int OnuIdx,  char *Desc,  int  len );
extern int  SetOnuDeviceDescMsg( short int PonPortIdx, short int OnuIdx,  char *Desc,  int  len );
extern int  SetOnuDeviceDesc_1( short int PonPortIdx, short int OnuIdx,  char *Desc,  int  len );
extern int  GetOnuDeviceDesc( short int PonPortIdx, short int OnuIdx,  char *Desc, int *len );
extern int  SetOnuLocation( short int PonPortIdx, short int OnuIdx,  char *Location, int len );
extern int  SetOnuLocationMsg( short int PonPortIdx, short int OnuIdx,  char *Location, int len );
extern int  SetOnuLocation_1( short int PonPortIdx, short int OnuIdx,  char *Location, int len );
extern int  GetOnuLocation( short int PonPortIdx, short int OnuIdx, char *Location , int *len);
extern int  GetOnuVendorInfo( short int PonPortIdx, short int OnuIdx, char *Info, int *InfoLen );
extern int  GetOnuFWVersion( short int PonPortIdx, short int OnuIdx, char *Version, int *len);
extern int  GetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *SwVersion, int *len );
extern int  GetOnuHWVersion( short int PonPortIdx, short int OnuIdx, char *HwVersion, int *len );
extern int  GetOnuBootVersion( short int PonPortIdx, short int OnuIdx, char *BootVersion, int *len);
extern int  GetOnuOperStatusByMacAddr( short int PonPortIdx, short int *OnuIdx, unsigned char *MacAddr );
extern int  GetOnuOperStatus( short int PonPortIdx, short int OnuIdx );
extern ULONG GetOnuAlarmStatus(short int PonPortIdx, short int OnuIdx );/*add by luh 2011-9-22*/
extern int SetOnuAlarmStatus(short int PonPortIdx, short int OnuIdx ,ULONG status);

extern int  GetOnuOperStatus_Ext(short int PonPortIdx, short int OnuIdx );
extern int  GetOnuOperStatus_1(short int PonPortIdx, short int OnuIdx );
extern int  GetOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx);
extern int  SetOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx);
extern int  ClearOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx);
extern int  GetOnuCurStatus( int slot , int port , int OnuId );
extern int  GetOnuCurrentAlarm( short int PonPortIdx, short int OnuIdx,  unsigned long *CurrentAlarmStatus );
/*extern int  GetOnuCurrentAlarmRank(short int PonPortIdx, short int OnuIdx, int *CurAlarm);*//* removed 20070705 */
extern int  GetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned long *pMask );
extern int  SetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned long mask );

extern int  SetOnuMacAddr( short int PonPortIdx, short int OnuIdx, char *MacAddr , int len );
extern int  GetOnuMacAddr( short int PonPortIdx, short int OnuIdx, char *MacAddr, int *len );
extern int  GetOnuMacAddrDefault( char *MacAddr, int *len );
extern int  GetOnuUpTime ( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuDistance( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuMaxMacNum( short int PonPortIdx, short int OnuIdx, unsigned int  *pValBuf );
extern int  SetOnuMaxMacNum( short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned int val );
extern int  SetOnuMaxMacNumAll( short int PonPortIdx, unsigned int Number);
extern int  CancelOnuMaxMacNumAll(short int PonPortIdx);
extern int  CancelOnuMaxMacNum(short int PonPortIdx, short int OnuIdx);
extern int  GetOnuPonType( short int PonPortIdx, short int OnuIdx, int *type );
extern int  GetOnuPonRate( short int PonPortIdx, short int OnuIdx, int  *rate );


extern int  SetOnuPonType( short int PonPortIdx, short int OnuIdx, int type );
extern int  SetOnuVendorInfo( short int PonPortIdx, short int OnuIdx, char *Info, int  len );
extern int  SetOnuSerialNo(short int PonPortIdx, short int OnuIdx, char * Info, int len);
extern int  GetOnuSerialNo(short int PonPortIdx, short int OnuIdx, char * Info, int * len);
extern int  SetOnuProduceDate(short int PonPortIdx, short int OnuIdx, char * Info, int len);
extern int  GetOnuProduceDate(short int PonPortIdx, short int OnuIdx, char * Info, int * len);
/*
extern int  SetOnuVendorLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location, unsigned char len  );
extern int  GetOnuVendorLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location );
extern int  SetOnuVendorContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len );
extern int  GetOnuVendorContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum );

extern int  SetOnuCustomerName( short int PonPortIdx, short int OnuIdx, unsigned char *Name, unsigned char len );
extern int  GetOnuCustomerName( short int PonPortIdx, short int OnuIdx, unsigned char *Name );
extern int  SetOnuCustomerContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len );
extern int  GetOnuCustomerContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum );

extern int  SetOnuDeviceType( short int PonPortIdx, short int OnuIdx, unsigned char *Type, unsigned char len );
extern int  GetOnuDeviceType( short int PonPortIdx, short int OnuIdx, unsigned char *Type);
*/
extern int  SetOnuType( short int PonPortIdx, short int OnuIdx, int type );
extern int  SetOnuOUI( short int PonPortIdx, short int OnuIdx, unsigned char *OUI );
extern int  OnuIsCMC( short int PonPortIdx, short int OnuIdx );

extern int  SetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *Version, int len);
extern int  SetOnuHWVersion( short int PonPortIdx, short int OnuIdx, char *SwVersion, int len );
extern int  SetOnuFWVersion( short int PonPortIdx, short int OnuIdx, char *SwVersion, int len );
extern int  SetOnuBootVersion( short int PonPortIdx, short int OnuIdx, char *BootVersion, int len);
	
extern int  RecordOnuLaunchTime ( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuLaunchTime ( short int PonPortIdx, short int OnuIdx, Date_S *date );
extern int  GetOnuUpLastTime( short int PonPortIdx, short int OnuIdx );

extern int  SetOnuAdminStatus( short int PonPortIdx, short int OnuIdx, unsigned char adminStatus );
extern int  GetOnuAdminStatus( short int PonPortIdx, short int OnuIdx );


/* onu running parameters */

extern int  SetOnuOperStatus( short int PonPortIdx, short int OnuIdx, int OperStatus );

extern int  updateOnuOperStatusAndUpTime( ULONG OnuEntry, int OperStatus );/* added by xieshl 20101220 */
extern int  RecordOnuUpTime ( ULONG OnuEntry );

extern int  SetOnuSWUpdateCtrl( short int PonPortIdx, short int OnuIdx, unsigned char EnableFlag );
extern int  GetOnuSWUpdateCtrl( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuSWUpdateCtrlDefault();
extern int  GetOnuSWUpdateStatus( short int PonPortIdx, short int OnuIdx );
extern int  SetOnuSwUpdateStatus( short int PonPortIdx, short int OnuIdx, int status, unsigned int fileType);

extern int  SetOnuUplinkBW( short int PonPortIdx, short int OnuIdx, unsigned int UplinkBw );

extern int GetOnuDefaultBWByPonRate(short int PonPortIdx, short int OnuIdx, char direction, unsigned int * bw_gr, unsigned int * bw_be);

extern int  SetOnuUplinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int UplinkDelay, unsigned int assured_bw, unsigned int best_effort_bw );
#ifdef PLATO_DBA_V3
extern int  SetOnuUplinkBW_2(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int fixed_bw, unsigned int assured_bw, unsigned int best_effort_bw );
#endif
extern int  SetOnuUpLinkBWAll( short int PonPortIdx, unsigned int UplinkBw);
extern int  GetOnuUplinkBW( short int PonPortIdx, short int OnuIdx , unsigned  int *UplinkBw );
extern int  GetOnuUplinkBW_1( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw );
#ifdef PLATO_DBA_V3
extern int  GetOnuUplinkBW_2( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *fixed, unsigned int *assured_bw, unsigned int *best_effort_bw );
extern int  GetOnuUplinkBW_3( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *delay, unsigned int *fixed, unsigned int *assured_bw, unsigned int *best_effort_bw );
#endif
extern int  GetOnuUplinkBWDefault(  unsigned int *UplinkBw );
extern int  GetOnuUplinkBWDefault_1(  unsigned int *UplinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw );
extern int  SetOnuDownlinkBW( short int PonPortIdx, short int OnuIdx, unsigned int DownlinkBw );
extern int  SetOnuDownlinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int DownlinkClass, unsigned int  DownlinkDelay, unsigned int assured_bw, unsigned int best_effort_bw );
extern int  SetOnuDownLinkBWAll( short int PonPortIdx, unsigned int UplinkBw);
extern int  Onu_IsDefaultBWSetting(short int PonPortIdx, short int OnuIdx);
extern int  GetOnuDownlinkBW( short int PonPortIdx, short int OnuIdx , unsigned  int *DownlinkBw );
extern int  GetOnuDownlinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int *DownlinkClass, unsigned int *DownlinkDelay, unsigned int *assured_bw, unsigned int *best_effort_bw );
extern int  GetOnuDownlinkBWDefault(  unsigned int *DownlinkBw );
extern int  GetOnuDownlinkBWDefault_1(  unsigned int *DownlinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw );
extern int  ActiveOnuUplinkBW( short int PonPortIdx, short int OnuIdx );
extern int  ActiveOnuDownlinkBW( short int PonPortIdx, short int OnuIdx );
extern int  ActiveOnuDefaultBW( short int PonPortIdx, short int OnuIdx, unsigned int ActDir );

extern int  SetDefaultBwForAllOnu(short int PonPortIdx );
extern int  GetDBAInfo( short int PonPortIdx );
extern int  GetSLAInfo( short int PonPortIdx, short int OnuIdx );

extern int  SetOnuUsedFlag( short int PonPortIdx, short int OnuIdx );
/*
extern int  SetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned int AlarmId, unsigned char MaskFlag );
extern int  GetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned int AlarmId );
*/

extern int  SetOnuStaticMacAddr( short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr);
extern int  GetOnuStaticMacAddrList( short int PonPortIdx, short int OnuIdx, unsigned char *NumOfMacAddr, MacTable_S *MacAddr );
extern int  DelOnuStaticMacAddr( short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr );
/*
extern int  GetOnuIsHaveBattery( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuUsedFlag( short int PonPortIdx, short int OnuIdx );
*/
extern int  GetOnuTableEntryFromMacAddr( unsigned char MacAddr[6], unsigned int entryIdx );
extern int  GetOnuTableEntryFromIfidx( short int PonPortIdx, short int OnuIdx, unsigned int entryIdx );
extern int  GetOnuTableEntryFromCustomerName( unsigned char *OnuCustomerName, unsigned int entryIdx );
extern int  GetOnuTableEntryFromPASOnuId( unsigned char  PonIdx, short int OnuIdx, unsigned int entryIdx );

extern short int  GetLlidByLlidIdx( short int PonPortIdx, short int OnuIdx, short int LLIDIdx );
extern short int  GetOnuLLIDIdx( short int PonPortIdx, short int OnuIdx, short int llid );
extern short int  GetOnuLLIDNum( short int PonPortIdx, short int OnuIdx );
extern short int  GetOnuLLIDNumMax( short int PonPortIdx, short int OnuIdx );
extern short int  GetOnuCmNumMax( short int PonPortIdx, short int OnuIdx );
extern short int  GetOnuIdxByLlid( short int PonPortIdx, short int llid );
extern short int  GetOnuEntryByMac( unsigned char *MacAddress );
extern short int  GetOnuIdxByMacPerPon( short int PonPortIdx, unsigned char *MacAddress );
extern short int  GetOnuIdxByName( unsigned char *Name );
extern short int  GetOnuIdxByName_OnePon(unsigned char *Name, short int PonPortIdx );
extern short int  GetLlidByOnuIdx( short  int PonPortIdx,  short int  OnuIdx );
extern short int  GetLlidActivatedByOnuIdx( short int PonPortIdx, short int OnuIdx );

extern int  Onu_deregister( short int PonPortIdx, short int OnuIdx );
extern int  DeregisterAllOnu(short int PonPortIdx);
extern int  Onu_ShutDown( short int PonPortIdx, short int OnuIdx );
extern int  ResetOnu( short int PonPortIdx, short int OnuIdx );
extern int  OnuDeregisterEvent(short int PonPortIdx, short int OnuIdx, int reason_code, unsigned int event_flags);

extern int  V2R1_OnuStartEncrypt( short int PonPortIdx, short int OnuIdx );
/*extern int  StartOnuEncryption( short int PonPortIdx, short int OnuIdx, unsigned int cryptionDirection  );*/
extern int GetOnuEncryptStatus( short int PonPortIdx, short int OnuIdx, unsigned int *Status);
/*extern int  StopOnuEncrypt( short int PonPortIdx, short int OnuIdx );*/
extern int  OnuEncrypt( short int PonPortIdx, short int OnuIdx, unsigned int EncryptionDirection);
extern int  OnuEncryptionOperation(short int PonPortIdx, short int OnuIdx, unsigned int cryptionDirection);
extern void  UpdateOnuEncryptKey( short int PonPortIdx, short int OnuIdx /*, PON_encryption_key_t EncryptKey */ );
extern  int UpdateEncryptKey( short int PonPortIdx, short int OnuIdx);
/*extern int  SetOnuEncryptKey( short int PonPortIdx, short int OnuIdx, PON_encryption_key_t PON_encryption_key );*/
extern int  SetOnuEncryptKeyExchagetime(short int PonPortIdx, short int OnuIdx, unsigned int time);
extern int  GetOnuEncryptKeyExchagetime( short int PonPortIdx, short int OnuIdx, unsigned int  *timeLen );
extern unsigned int GetOnuEncryptKeyExchagetimeDefault(void);
extern unsigned int GetOnuEncryptDefault(void);

extern int  GetOnuDeviceVersion( short int PonPortIdx, short int OnuIdx);
extern int  GetOnuCapability(short int PonPortIdx, short int OnuIdx );
extern int  GetOnuOamInformation(short int PonPortIdx, short int OnuIdx);
extern int  GetOnuRegisterData( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuConfigPara(short int PonPortIdx, short int OnuIdx);

extern int  set_ONU_alarm_configuration(short int PonPort_id, short int onu_id ) ;

/*extern int  ShowOnuStatus( short int PonPortIdx, short int OnuIdx );
extern int  ShowOnuStatusAll( short int PonPortIdx );
extern int  ShowOnuRunningInfo( short int PonPortIdx, short int OnuIdx );
extern int  ShowOnuDeviceInfo(short int PonPortIdx, short int OnuIdx );
extern int  ShowOnuBandwidth( short  int PonPortIdx, short int OnuIdx );
extern int  ShowOnuBandwidthAll( short int PonPortIdx);
extern int  ShowLlidMacInfo( short int PonPortIdx, short int OnuIdx, short int LlidIdx );
extern int  ShowOnuEncryptInfo( short int PonPortIdx, short int OnuIdx );*/
extern int  ShowOnuBandwidthByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern int  ShowOnuBandwidthAllByVty( short int PonPortIdx, struct vty *vty );
extern int  ShowOnuSWUpdateInfoByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty );

extern int  ShowPAS6201MacLearningByVty(short int PonPortIdx, short int OnuIdx , struct vty *vty);
extern int  ShowOnuVersionInfoByVty_1(short int PonPortIdx, short int OnuIdx, unsigned char *OnuType, struct vty *vty);
extern int  ShowOnuVersionInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty);
extern int  ShowOnuVersionInfoByVtyAll(short int PonPortIdx, struct vty *vty);

/*extern int  CompTwoMacAddress( unsigned char *MacAddress1, unsigned char *MacAddress2 );*/
extern int  ScanOnuMgmtTableTimer( ULONG timer_counter );
extern int  ScanOnuMgmtTableBerFer();

/*extern int  setSeed();*/
extern int  GenerationRandom();
extern int  GenerationEncryptKey(short int PonPortIdx, short int OnuIdx );

extern int  ClearOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx );
extern int  CreateOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx );
extern int  InitOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx );

/***** from wutongwu ONU OAM communication ****/
/*=====================cli接口定义=========================*/

extern int  Comm_Cli_request_transmit( struct vty * vty,
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *sessionId );

 

/*=====================snmp接口定义=========================*/

extern int Comm_snmp_request_transmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				void *pSnmpbuf,
				const unsigned short Snmp_data_size,
				unsigned char *sessionId );

/*=================igmp 回应发送接口=========================*/

extern int Comm_IGMPAuth_Reponse(
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned char *pIgmpAuth,
				const unsigned short igmp_data_size,
				unsigned char *sessionId);

 

/*=====================设备信息传输接口定义=========================*/

extern int Comm_EUQ_info_request_transmit(
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned char *pEuqinfobuf,
				const unsigned short Euqinfo_data_size,
				unsigned char *sessionId);

 

/*========================oam 接收回调函数=====================================*/
/*
说明: 当其他  使用oam 模块发送数据时,必须向oam模块注册一个回收函数。
参数 ： oamopcode 待定，目前类型应分为：设备信息，告警，igmp，cli，snmp，文件传输。
         Oamopcode的值需要不同模块协商定义
*/
extern int  CommOltMsgRvcCallbackInit (unsigned char oamOpcode, void *Function);

 /*
说明: 当接收到一个完整的frame(经过重组后的完整的数据帧)后,调用该函数, 将数据帧送往目的模块进行处理。
关键参数说明： GwOpCode – 为不同模块的类型识别号，包括：设备信息，告警，igmp，cli，snmp，文件传输。
Oamopcode的值需要不同模块协商定义
                   Oltid －
                   Onuid －
                   Length － 
                   Pframe － 
 */
extern int CommOnuMsgCallback( const short int        OltId,
				const unsigned short     OnuId,
				const unsigned char GwOpCode,
				const unsigned short   Length,
				unsigned char   *pFrame);

typedef struct OnuEUQInfo{
	char SessionId[8];
	char MsgType;
	short int DeviceType;
	char OUI[3];
	char HwVerLen;
	char HwVer[10];
	char BootVerLen;
	char BootVer[15];
	char SwVerLen;
	char SwVer[15];
	char FwVerLen;
	char FwVer[20];
	char NameLen;
	char Name[10];
	char DescLen;
	char Desc[10];
	char LocationLen;
	char Location[10];
	char VendorInfoLen;
	char VendorInfo[20];
} __attribute__ ((packed)) OnuEUQInfo_S;

typedef struct  SetOnuSysInfo
{
	char NameLen;
	char Name[255];
	char DescLen;
	char Desc[255];
	char LocationLen;
	char Location[255];
}SetOnuSysInfo_S;

typedef struct OnuIdentifier
{
	short int PonPortIdx;
	short int OnuIdx;
	struct OnuIdentifier *PreNode;
	struct OnuIdentifier *NextNode;

}OnuIdentifier_S;

#define  ONU_OAM_COMM_WAIT_SEND  5
#define ONU_RESPONSE_TIMEOUT  (VOS_TICK_SECOND * 3 )
extern int ONU_FLASH_BURNING_WAIT ;

/* the struct of the GetOnuSysInfo */
/*
#define  OAMMSG_SESSIONIDSTART   0
#define  OAMMSG_SESSIONIDLEN 8
#define  OAMMSG_MSGTYPESTART  (OAMMSG_SESSIONIDSTART + OAMMSG_SESSIONIDLEN )
#define  OAMMSG_MSGTYPELEN  1
#define  OAMMSG_DEVICETYPESTRT (OAMMSG_MSGTYPESTART +  OAMMSG_MSGTYPELEN)
#define  OAMMSG_DEVICETYPELEN 2
#define  OAMMSG_OUISTART (OAMMSG_DEVICETYPESTRT +OAMMSG_DEVICETYPELEN)
#define  OAMMSG_OUILEN 3

#define  OAMMSG_GET_INFO_LEN_MIN  8
#define  OAMMSG_SET_INFO_LEN_MIN  3
*/
#if 0
#define  OAMMSG_SESSIONIDSTART   0
#define  OAMMSG_SESSIONIDLEN 8
#endif
#define  OAMMSG_MSGTYPESTART  0
#define  OAMMSG_MSGTYPELEN  1
#define  OAMMSG_DEVICETYPESTRT (OAMMSG_MSGTYPESTART +  OAMMSG_MSGTYPELEN)
#define  OAMMSG_DEVICETYPELEN 2
#define  OAMMSG_OUISTART (OAMMSG_DEVICETYPESTRT +OAMMSG_DEVICETYPELEN)
#define  OAMMSG_OUILEN 3

#define  OAMMSG_GET_INFO_LEN_MIN  8
#define  OAMMSG_SET_INFO_LEN_MIN  3
#define  EUQ_MAX_OAM_PDU 1500

#define V2R1_ONU_EUQ_MSG_OK  0
#define V2R1_ONU_EUQ_MSG_WAIT  1
#define V2R1_ONU_EUQ_MSG_GET 2

#define GET_ONU_SYS_INFO_REQ 0x01
#define GET_ONU_SYS_INFO_RSP 0x01

#define SET_ONU_SYS_INFO_REQ 0x02
#define SET_ONU_SYS_INFO_RSP 0x02
#define GET_EXT_ONU_SYS_INFO_REQ  0x05
#define GET_EXT_ONU_SYS_INFO_RSP  0x05

#define   SET_ONU_SYS_RESET_REQ   0x06
#define   SET_ONU_SYS_RESET_RSP   0x06

#define  SET_ONU_SYS_INFO_LOCATION 0x03
#define  SET_ONU_SYS_INFO_DESC 0x02
#define  SET_ONU_SYS_INFO_NAME  0x01

#define SYNC_ONU_SYS_TIME_REQ  0x03
#define SYNC_ONU_SYS_TIME_RSP  0x03

#define  GET_ONU_VIRTUAL_SCOPE_OPTICAL_REQ  0x07
#define  GET_ONU_VIRTUAL_SCOPE_OPTICAL_RSP  0x07

#define  SYNC_ONU_SYS_TIME_REQ_TYPE  0
#define  SYNC_ONU_SYS_TIME_REQ_TYPE_LEN  1
#define  SYNC_ONU_SYS_TIME_REQ_YEAR  ( SYNC_ONU_SYS_TIME_REQ_TYPE + SYNC_ONU_SYS_TIME_REQ_TYPE_LEN)
#define  SYNC_ONU_SYS_TIME_REQ_YEAR_LEN  2
#define  SYNC_ONU_SYS_TIME_REQ_MONTH  (SYNC_ONU_SYS_TIME_REQ_YEAR +  SYNC_ONU_SYS_TIME_REQ_YEAR_LEN )
#define  SYNC_ONU_SYS_TIME_REQ_MONTH_LEN  1
#define  SYNC_ONU_SYS_TIME_REQ_DAY  ( SYNC_ONU_SYS_TIME_REQ_MONTH +  SYNC_ONU_SYS_TIME_REQ_MONTH_LEN )
#define  SYNC_ONU_SYS_TIME_REQ_DAY_LEN  1
#define  SYNC_ONU_SYS_TIME_REQ_HOUR  (SYNC_ONU_SYS_TIME_REQ_DAY + SYNC_ONU_SYS_TIME_REQ_DAY_LEN )
#define  SYNC_ONU_SYS_TIME_REQ_HOUR_LEN  1
#define  SYNC_ONU_SYS_TIME_REQ_MINUTE  (SYNC_ONU_SYS_TIME_REQ_HOUR+SYNC_ONU_SYS_TIME_REQ_HOUR_LEN)
#define  SYNC_ONU_SYS_TIME_REQ_MINUTE_LEN  1
#define  SYNC_ONU_SYS_TIME_REQ_SECOND ( SYNC_ONU_SYS_TIME_REQ_MINUTE + SYNC_ONU_SYS_TIME_REQ_MINUTE_LEN)
#define  SYNC_ONU_SYS_TIME_REQ_SECOND_LEN   1 

#define GET_ONU_ALLPORT_STATISTIC  201
#define GET_ONU_ETH_PORT_MAC_NUM   210

#define UPLINK_BW_EQUALS_TO_DEFAULT(pon_rate,bw_gr,bw_be) (((pon_rate) == PON_RATE_NORMAL_1G \
			&& (bw_gr) == OnuConfigDefault.UplinkBandwidth && (bw_be) == OnuConfigDefault.UplinkBandwidthBe)\
		|| ((pon_rate) == PON_RATE_1_10G \
			&& (bw_gr) == OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM && (bw_be) == OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM)\
		|| ((pon_rate) == PON_RATE_10_10G \
			&& (bw_gr) == OnuConfigDefault.UplinkBandwidth_XGEPON_SYM && (bw_be) == OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM)\
		|| ((pon_rate) == PON_RATE_1_2G \
			&& (bw_gr) == OnuConfigDefault.UplinkBandwidth_GPON && (bw_be) == OnuConfigDefault.UplinkBandwidthBe_GPON))

#define DOWNLINK_BW_EQUALS_TO_DEFAULT(pon_rate,bw_gr,bw_be) (((pon_rate) == PON_RATE_NORMAL_1G \
			&& (bw_gr) == OnuConfigDefault.DownlinkBandwidth && (bw_be) == OnuConfigDefault.DownlinkBandwidthBe)\
		|| ((pon_rate) == PON_RATE_1_10G \
			&& (bw_gr) == OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM && (bw_be) == OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM)\
		|| ((pon_rate) == PON_RATE_10_10G \
			&& (bw_gr) == OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM && (bw_be) == OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM)\
		|| ((pon_rate) == PON_RATE_1_2G \
			&& (bw_gr) == OnuConfigDefault.DownlinkBandwidth_GPON && (bw_be) == OnuConfigDefault.DownlinkBandwidthBe_GPON))


extern OnuIdentifier_S  OnuWaitForGetEUQ;
extern unsigned long OnuEUQDataSemId ;
extern unsigned long OnuEUQCommSemId;
extern unsigned long OnuEUQRecvSemId ;
extern char RecvMsgFromOnu[];
extern char RecvMsgFromOnuSessionId[] ;
extern int  RecvMsgFromOnuLen ;
/*extern static unsigned int WaitOnuEUQMsgFlag;
extern short int ONUEUQMsgWaitPonPortIdx;
extern short int ONUEUQMsgWaitOnuIdx;
*/

extern int  OnuWaitForGetEUQInit();
extern int  AddOneNodeToGetEUQ( short int PonPortIdx, short int OnuIdx );
extern int  DelOneNodeFromGetEUQ( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuEUQInfo( short int PonPortIdx, short int OnuIdx );
extern int  SyncSysTimeToOnu( short int PonPortIdx, short int OnuIdx );
extern int  SyncSysTimeToOnuMsg(short int PonPortIdx, short int OnuIdx);
/*extern int  DisplayAllNode();
extern int  DisplayNode(short int PonPortIdx );*/
#if 0
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern int  V2R1_SendMsgToONU( short int PonPortIdx, short int OnuIdx, char MsgType, unsigned char *Buf, int len );	
#endif
#endif
extern int  EQU_SendMsgToOnu( short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length);
/*extern int  GetOnuEUQInfoHandle(short int PonPortIdx, short int OnuIdx);*/
#if 0
extern int  OnuEUQRecvMsgCallBack( short int PonPortIdx, short int OnuLlid, /*unsigned char GwProId, */short int len, unsigned char *pDataBuf, unsigned char  *pSessionField);
extern int  OnuEUQRecvMsgCallBack_New( short int PonPortIdx, short int OnuLlid, /*unsigned char GwProId, */short int len, unsigned char *pDataBuf, unsigned char  *pSessionField);
#endif
extern int   RecordOnuEUQInfo(short int PonPortIdx, short int OnuIdx, unsigned char *Info, int Len );

extern int OnuEvent_UpDate_RunningUpBandWidth( short int PonPortIdx, short int OnuIdx);
extern int OnuEvent_UpDate_RunningDownBandWidth(short int PonPortIdx, short int OnuIdx);

extern int ShowPAS6201MacLearningCounterByVty(short int PonPortIdx, short int OnuIdx , struct vty *vty);
extern long get_ONU_Info(char *onu_Type, int *offset,  int *file_len, int *compress_flag, int *file_fact_len, int * version);
extern long get_ONU_file(char *onu_Type, char *buffer, int *file_len, char *target_type);

#ifdef ONU_PEER_TO_PEER

extern int  RecordOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 );
extern int  ClearOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 );
extern int  GetOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2/*, int *address_not_foundFlag, int *broadcastFlag*/ );
extern int  GetOnuIsSetPeerToPeer( short int PonPortIdx, short int OnuIdx );
extern int  GetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx, int *address_not_foundFlag, int *broadcastFlag );
/*extern int  DisplayOnuPeerToPeer( short int PonPortIdx, short int OnuIdx);*/
extern int  DisableOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx );
extern int  EnableOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx /*, int address_not_found, int broadcast*/ );
#if 0
extern int  EnableOnuPeerToPeer_1(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2);
#endif
extern int  EnableOnuPeerToPeer(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2);
extern int  DisableOnuPeerToPeer(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2);
extern int  SetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx , int address_not_found, int broadcast );
extern int  SetOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 );
extern int  DiscOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 );
extern int  ShowOnuPeerToPeerByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty);
extern int  ShowOnuPeerToPeerByVty_1( short int PonPortIdx, short int OnuIdx, struct vty *vty);
#endif

extern int  GetOnuFecMode(short int PonPortIdx, short int OnuIdx, int  *mode );
extern int  SetOnuFecMode(short int PonPortIdx, short int OnuIdx, int  mode );

#ifdef  ONU_UPSTREAM_SA_MAC_FILTER 
#endif
/* add by chenfj 2007/5/28
   增加设置/清除ONU 上行数据包原MAC 过滤
*/
extern int  AddOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress );
extern int  ClearOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress );
extern int  ClearOnuSAMacFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  EnableOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx/*, mac_address_t MacAddress*/ );
extern int  DisableOnuSAMacFilter( short int PonPortIdx ,short int OnuIdx, mac_address_t MacAddress );
extern int  AddFilterSAMacToTable(short int PonPortIdx, short int OnuIdx, mac_address_t MacAddress );
extern int  DelFilterSAMacFromTable(short int PonPortIdx, short int OnuIdx, mac_address_t MacAddress );
extern int  ShowOnuFilterSAMacByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern int  ShowOnuFilterSAMacByVty1( short int PonPortIdx, short int OnuIdx, struct vty *vty );

#ifdef  ONU_ACCESS_CONTROL_LIST_FILTER
#endif
/** added by chenfj 2007-6-8 
       增加ONU 数据流IP/PORT  过滤*/
extern int  V2R1_GetLongFromIpdotstring( unsigned char * ipaddr, int  * pulMask );
extern int  AddOnuSIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  ClearOnuSIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  ClearOnuSIpFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  AddOnuDIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  ClearOnuDIpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  ClearOnuDIpFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  AddOnuSIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int  udp_port );
extern int  ClearOnuSIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port );
extern int  ClearOnuSIpUdpFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  AddOnuDIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port);
extern int  ClearOnuDIpUdpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port );
extern int  ClearOnuDIpUdpFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  AddOnuSIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port);
extern int  ClearOnuSIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port );
extern int  ClearOnuSIpTcpFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  AddOnuDIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port);
extern int  ClearOnuDIpTcpFilter( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port );
extern int  ClearOnuDIpTcpFilterAll( short int PonPortIdx, short int OnuIdx );


extern int  EnableOnuSIpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr );
extern int  EnableOnuSIpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuSIpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr );
extern int  DisableOnuSIpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  EnableOnuDIpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr );
extern int  EnableOnuDIpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuDIpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr );
extern int  DisableOnuDIpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  EnableOnuSIpUdpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  udp_port );
extern int  EnableOnuSIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuSIpUdpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int udp_port  );
extern int  DisableOnuSIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  EnableOnuDIpUdpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  udp_port );
extern int  EnableOnuDIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuDIpUdpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int udp_port  );
extern int  DisableOnuDIpUdpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  EnableOnuSIpTcpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  tcp_port );
extern int  EnableOnuSIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuSIpTcpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int tcp_port  );
extern int  DisableOnuSIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  EnableOnuDIpTcpFilter( short int PonPortIdx ,short int OnuIdx , unsigned int  IpAddr, unsigned short int  tcp_port );
extern int  EnableOnuDIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx );
extern int  DisableOnuDIpTcpFilter( short int PonPortIdx ,short int OnuIdx, unsigned int  IpAddr, unsigned short int tcp_port  );
extern int  DisableOnuDIpTcpFilterAll( short int PonPortIdx ,short int OnuIdx );

extern int  AddFilterSIpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  DelFilterSIpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr);
extern int  AddFilterDIpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr );
extern int  DelFilterDIpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr);
extern int  AddFilterSIpUdpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_Port );
extern int  DelFilterSIpUdpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port);
extern int  AddFilterDIpUdpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_Port );
extern int  DelFilterDIpUdpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int udp_port);
extern int  AddFilterSIpTcpToTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_Port );
extern int  DelFilterSIpTcpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port);
extern int  AddFilterDIpTcpToTable( short int PonPortIdx, short int OnuIdx,  unsigned int IpAddr, unsigned short int tcp_Port );
extern int  DelFilterDIpTcpFromTable( short int PonPortIdx, short int OnuIdx, unsigned int IpAddr, unsigned short int tcp_port);

extern int  ShowOnuSIpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty );
extern int  ShowOnuDIpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty );
extern int  ShowOnuUdpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty );
extern int  ShowOnuTcpFilterByVty( short int PonPortIdx, short int OnuIdx, struct  vty  *vty );

#ifdef  ONU_VLAN_ID_FILTER
#endif
/** added by chenfj 2007-6-12 
       增加ONU 数据流vlan id 过滤*/
extern int  AddOnuVlanIdFilter(short int PonPortIdx, short int OnuIdx, unsigned short int VlanId );
extern int  ClearOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId );
extern int  ClearOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  EnableOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId );
extern int  EnableOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  DisableOnuVlanIdFilter( short int PonPortIdx, short int OnuIdx, unsigned short int VlanId );
extern int  DisableOnuVlanIdFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  AddFilterVlanIdToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int VlanId);
extern int  DelFilterVlanIdFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int VlanId);
extern int  ShowOnuVlanIdFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty );

extern int  EnableOnuFilter(short int PonPortIdx, short int OnuIdx );

#ifdef  ONU_ETHER_TYPE_and_IP_PROTOCOL_FILTER
#endif
/** added by chenfj 2007-6-14 
       增加ONU 数据流ETHER TYPE /IP PROTOCOL 过滤*/

extern int  AddOnuEtherTypeFilter(short int PonPortIdx, short int OnuIdx, unsigned short int  EtherType);
extern int  ClearOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  EtherType );
extern int  ClearOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  EnableOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int EtherType );
extern int  EnableOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  DisableOnuEtherTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int EtherType );
extern int  DisableOnuEtherTypeFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  AddFilterEtherTypeToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  EthType);
extern int  DelFilterEtherTypeFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  EthType );
extern int  AddOnuIpTypeFilter(short int PonPortIdx, short int OnuIdx, unsigned short int  IpType);
extern int  ClearOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  IpType );
extern int  ClearOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx );
extern int  EnableOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int IpType );
extern int  EnableOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  DisableOnuIpTypeFilter( short int PonPortIdx, short int OnuIdx, unsigned short int  IpType );
extern int  DisableOnuIpTypeFilterAll( short int PonPortIdx, short int OnuIdx);
extern int  AddFilterIpTypeToTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  IpType);
extern int  DelFilterIpTypeFromTable( short int PonPortIdx, short int OnuIdx,  unsigned short int  IpType );
extern int  ShowOnuEtherTypeFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty );
extern int  ShowOnuIpTypeFilterByVty( short int PonPortIdx, short int OnuIdx , struct  vty  *vty );



#ifdef  V2R1_ONU_AUTO_CONFIG
extern int  GetOnuAutoConfig(short int PonPortIdx, short int OnuIdx );
extern int  SetOnuAutoConfig(short int PonPortIdx, short int OnuIdx, int AutoConfig );
extern int  GetOnuAutoConfigFlag(short int PonPortIdx, short int OnuIdx);
extern int  SetOnuAutoConfigFlag(short int PonPortIdx, short int OnuIdx, int AutoConfigStatus);
extern int  SendMsgToOnuAutoConfig(short int PonPortIdx, short int OnuIdx);
extern int  ClearOnuRunningConfig(short int PonPortIdx, short int OnuIdx );
extern int  AutoConfigResetOnu(short int PonPortIdx, short int OnuIdx );
extern int  AutoConfigOnu( short int PonPortIdx, short int OnuIdx );
#endif

#ifdef    CTC_EXT_OID

/*void    initLlidDbaQueset( short int pon, short int onu, CTC_STACK_onu_queue_set_thresholds_t *queset, UCHAR num );*/
STATUS	getLlidDbaQueset( short int pon, short int onu, short int queidx, _queset_t *data );

#endif

extern int  IsSupportCTCOnu(short int PonPortIdx );
extern int  GetOnuVendorType( short int PonPortIdx, short int OnuIdx );
extern int  CTC_SetOnuMulticastSwitchProtocol(short int PonPortIdx, short int OnuIdx );
extern int  CTC_GetOnuMulticastSwitchProtocol(short int PonPortIdx, short int OnuIdx, int *MulticastProtocol);

#if 0
extern int  CTC_GetOnuSN( short int PonPortIdx, short int OnuIdx );
#endif

extern int  CTC_StartLlidEncrypt( short int PonPortIdx, short int OnuIdx );
extern int  CTC_StopLlidEncrypt( short int PonPortIdx, short int OnuIdx );
extern int  CTC_GetLlidEncryptStatus( short int PonPortIdx, short int OnuIdx );

extern int  CTC_SetLlidFecMode( short int PonPortIdx, short int OnuIdx, int  mode );
extern int  CTC_GetLlidFecMode( short int PonPortIdx, short int OnuIdx, int  *mode );
extern int  CTC_GetLlidFecAbility( short int PonPortIdx, short int OnuIdx, int  *Fec_Ability );

#ifdef  ONU_DEVICE_DEACTIVE 
#endif
	/* added by chenfj 2007-6-27 
	    ONU	 device  de-active */
extern  int  DeactivateOnu( short int PonPortIdx, short int OnuIdx );
extern  int  ActivateOnu( short int PonPortIdx, short int OnuIdx );
extern  int  SetOnuTrafficServiceEnable( short int PonPortIdx, short int OnuIdx, int TrafficeServiceEnable );
extern  int  GetOnuTrafficServiceEnable( short int PonPortIdx, short int OnuIdx );


/* 设置ONU VLAN translation  and  QOS mapping */
extern int SetOnuEthPortVlanTranslation( short int PonPortIdx, short int OnuIdx, unsigned char port_number, unsigned long default_vid, int old_vlanId, int new_vlanId );
extern int SetOnuEthPortVlanTag( short int PonPortIdx, short int OnuIdx, unsigned char port_number, int Tag );
extern int SetOnuEthPortVlanTransparent( short int PonPortIdx, short int OnuIdx, unsigned char port_number);
extern int SetOnuEthPortQosPriMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char pri, unsigned char mapping_pri, unsigned char mapping_que, int Match_condition );
extern int SetOnuEthPortQosDMacMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char *MacAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosSMacMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned char *MacAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosVidMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Vid, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosEthTypeMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int EthType, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosDipMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int ipAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosSipMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int ipAddr, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosIpTypeMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Iptype, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosIpv4DSCPMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int Tos_Dscp, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosIpv6PrecMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  precedence, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosIpSrcPortMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  SrcPort, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int SetOnuEthPortQosIpDestPortMapping(short int PonPortIdx, short int OnuIdx, unsigned char port, unsigned int  DesPort, unsigned char mapping_pri, unsigned char mapping_Que, int Match_condition );
extern int DelOnuEthPortQosRule(short int PonPortIdx, short int OnuIdx, unsigned char port );

extern short int GetOnuDeviceChipId(short int PonPortIdx, short int OnuIdx, short int *ChipId);

/* 设置ONU  UNI 端口状态*/
extern int CTC_UpOnuOneEthPort(short int PonPortIdx, short int OnuIdx, unsigned char EthPort);
extern int CTC_DownOnuOneEthPort(short int PonPortIdx, short int OnuIdx, unsigned char EthPort);
extern int CTC_UpOnuAllEthPort(short int PonPortIdx, short int OnuIdx );
extern int CTC_DownOnuAllEthPort(short int PonPortIdx, short int OnuIdx );

/* added by chenfj 2007-8-14 
	通过PMC提供的API (PMC私有OAM通道) 来升级ONU文件*/
extern int  StartOnuSWUpdate_1( short int PonPortIdx, short int OnuIdx);
extern int  StartOnuSWUpdate_2( short int PonPortIdx, short int OnuIdx, int UpdateFileType);
extern int  SendUpdateOnuImageMsg(short int PonPortIdx, short int OnuIdx, unsigned char *onuTypeString, int length );
extern int  UpdateOnuFileImage(short int PonPortIdx, short int OnuIdx, unsigned char *onuTypeString, int length);
extern int  CheckOnuWhetherBurnFlashComplete( short int PonPortIdx, short int OnuIdx );
extern int  ScanOnuMgmtTableBurningFlash();
extern int  GetNumOfOnuUpdate();
extern LONG GetNumofOnuUpdatingStatusBySlot( short int olt_id );

/* added by chenfj 2007-9-13
	在对ONU加密时，判断ONU与OLT使用的PON芯片类型是否匹配*/
extern int  EncryptIsSupported( short int PonPortIdx, short int OnuIdx);

extern int IsPas6201Onu(short int PonPortIdx, short int OnuIdx);
extern int IsPas6301Onu(short int PonPortIdx, short int OnuIdx);
/*extern int IsSupportPotsService(short int PonPortIdx, short int OnuIdx);*/
extern int OnuIsSupportVoice(unsigned long OnuDeviceIdx, bool *SupportVoice);
extern int OnuEponAppHasVoiceApp(short int PonPortIdx, short int OnuIdx);
extern int OnuIsGT831(short int PonPortIdx, short int OnuIdx);
extern int OnuFirmwareHasFpgaApp(short int PonPortIdx, short int OnuIdx);
extern int GetOnuAppTypeString( short int PonPortIdx, short int OnuIdx, unsigned char *AppTypeString, int *len );
extern int GetOnuVoiceTypeString( short int PonPortIdx, short int OnuIdx, unsigned char *VoiceTypeString, int *len );
extern int GetOnuFPGATypeString( short int PonPortIdx, short int OnuIdx, unsigned char *FPGATypeString, int *len );

/*extern int  ModifyOnuVoiceVersion(short int PonPortIdx, short int OnuIdx );*/
extern int  SearchOnuByType(short int PonPortIdx, unsigned char *OnuTypeString );

extern int SetOnuAutoConfigFlag( short int PonPortIdx, short int OnuIdx, unsigned char AutoConfig );
extern int GetOnuAutoConfigFlag( short int PonPortIdx, short int OnuIdx );

extern int SetOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx, unsigned char flag, PON_olt_vlan_uplink_config_t    vlan_uplink_config );
extern int GetOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx, unsigned char *flag, PON_olt_vlan_uplink_config_t *vlan_uplink_config);
extern int RestoreOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx );

extern int ScanOnuPowerOffCounter();

/* added by chenfj 2008-6-11
      支持GT861 等多子卡ONU  */
extern int  ParsePrefixAndSuffixFromString(unsigned char *String, unsigned char *Prefix, unsigned char *len1, unsigned char *Suffix, unsigned char *len2);

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
#define  RecvOpticalPowerLowAlarm   0x1
#define  RecvOpticalPowerHighAlarm  0x2
#define  TransOpticalPowerLowAlarm  0x4
#define  TransOpticalPowerHighAlarm  0x8
#define  TemperatureLowAlarm 0x10
#define  TemperatureHighAlarm 0x20
#define  WorkVoltageLowAlarm 0x40
#define  WorkVoltageHighAlarm 0x80
#define  BiasCurrentLowAlarm  0x100
#define  BiasCurrentHighAlarm 0x200

extern int GetOnuTransOpticalPower(short int PonPortIdx,short int OnuIdx);
extern int GetOnuRecvOpticalPower(short int PonPortIdx, short int OnuIdx);
extern int GetOnuTemperature(short int PonPortIdx, short int OnuIdx);
extern int GetOnuWorkVoltage(short int PonPortIdx, short int OnuIdx);
extern int GetOnuBiasCurrent(short int PonPortIdx,short int OnuIdx);

extern LONG GetOnuRecvOpticalPowerLowThrd();
extern LONG GetOnuRecvOpticalPowerHighThrd();
extern LONG GetOnuTransOpticalPowerLowThrd();
extern LONG GetOnuTransOpticalPowerHighThrd();
extern LONG GetOnuTemperatureLowThrd();
extern LONG GetOnuTemperatureHighThrd();
extern LONG GetOnuWorkVoltageLowThrd();
extern LONG GetOnuWorkVoltageHighThrd();
extern LONG GetOnuBiasCurrentHighThrd();
extern LONG GetOnuBiasCurrentLowThrd();

extern LONG SetOnuRecvOpticalPowerLowThrd(long val);
extern LONG SetOnuRecvOpticalPowerHighThrd(long val);
extern LONG SetOnuTransOpticalPowerLowThrd(long val);
extern LONG SetOnuTransOpticalPowerHighThrd(long val);
extern LONG SetOnuTemperatureLowThrd(long val);
extern LONG SetOnuTemperatureHighThrd(long val);
extern LONG SetOnuWorkVoltageLowThrd(long val);
extern LONG SetOnuWorkVoltageHighThrd(long val);
extern LONG SetOnuBiasCurrentLowThrd(long val);
extern LONG SetOnuBiasCurrentHighThrd(long val);

extern int CheckOnuRecvOpticalPowerNeedTrap(short int PonPortIdx, short int OnuIdx, long RecvOpticalPower_cur);
extern int CheckOnuTransOpticalPowerNeedTrap(short int PonPortIdx, short int OnuIdx, long TransOpticalPower_cur);
extern int CheckOnuTemperatureNeedTrap(short int PonPortIdx, short int OnuIdx, long Temperature_cur);
extern int CheckOnuWorkVoltageNeedTrap(short int PonPortIdx, short int OnuIdx, long WorkVoltage_cur);
extern int CheckOnuBiasCurrentNeedTrap(short int PonPortIdx, short int OnuIdx, long BiasCurrent_cur);
#endif

/* added by chenfj 2008-12-9
	增加以下函数, 供ONU 自动升级和自动配置调用
*/
/*   读取ONU APP 程序版本 */
extern int  GetOnuDeviceAppVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength);
/*   读取ONU voice 程序版本 */
extern int  GetOnuDeviceVoiceVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength);
/*   读取ONU firmware 版本 */
extern int  GetOnuDeviceFrimwareVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength);
/*   读取ONU fpga 版本 */
extern int  GetOnuDeviceFpgaVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength);
/*   读取ONU boot 版本 */
extern int  GetOnuDeviceBootVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength);


extern int  GetOnuModel( short int PonPortIdx, short int OnuIdx, char *pModel, int *pLen );
extern int  AuthorizeOnuToNetwork(short int PonPortIdx, short int OnuIdx );
extern int  DenyOnuFromNetWork(short int PonPortIdx, short int OnuIdx );

extern int  EnableOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx);
extern int  DisableOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx);
extern int  GetOnuOamSlowProtocolLimit(short int PonPortIdx, short int OnuIdx, bool *enable);
/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
extern int CTC_GetOui ( unsigned long  *oui );
extern int CTC_SetOui ( const unsigned long  oui );
extern int CTC_GetVersion( unsigned char  *version );
extern int CTC_SetVersion( const unsigned char  version );
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
extern int OnuMgt_GetOnuMacEntry(short int olt_id, short int onu_id, ULONG mactype ,OnuPortLacationInfor_S *table);
extern int OnuMgt_SetMulticastTemplate( short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag);
extern int OnuMgt_GetMulticastTemplate(short int olt_id, short int onu_id, int prof1,  int prof2,CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag);
extern int OnuMgt_SetMcastOperProfile( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter);
extern int OnuMgt_GetMcastOperProfile( short int olt_id, short int onu_id, int prof,  CTC_GPONADP_ONU_McastOper_Prof_t *parameter);
extern int OnuMgt_SetUniPortAssociateMcastProf( short int olt_id, short int onu_id,short int portid ,unsigned char stateflag,int profIdx);
extern int  OnuMgt_GetUniPortAssociateMcastProf( short int olt_id, short int onu_id, short int port_id,CTC_GPONADP_ONU_Profile_t *ProfIdx);
/*extern void InitOnuRegisterFilterQueue();*/
#endif /* _ONUGERNRAL_H */
