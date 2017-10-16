
/**************************************************************
*
*    PonGeneral.h -- PON management high level Application functions General header
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	     |    Change 	|    Author	  
*   --------------|------------------|------------------|------------
*	1.00	  |   14/04/2006     |   Creation	| chen fj
*
***************************************************************/
#ifndef _PONGERNRAL_H
#define  _PONGERNRAL_H

#include "OltGeneral.h"
#include "PonLog.h"
#include "PonEvent.h"


#ifndef TO_STR
#define TO_STR(m) #m
#endif

#ifndef INT_TO_STR
#define INT_TO_STR(m) TO_STR(m)
#endif


#define MAXOLTPERPONCHIP  16/*8*/
/*数组初始化、全局变量初始化、命令行参数及提示需使用设备最大能力，MAXONUPERPONNOLIMIT应该大于等于MAXONUPERPON*/
#define MAXONUPERPONNOLIMIT 128 
/*根据不同设备定义不同的能力，目前8000支持128分路比，其他老旧设备均为64，代码中的判断依据应该按照此宏进行判断*/
#define MAXONUPERPON       (OltChassis.OltMaxOnuPerPon) /*128*/

#define MAXONU  (MAXONUPERPON*MAXPON )

#define MAXOLTADDRNUM     16 
#define OLT_ADDR_BYTELEN  BYTES_IN_MAC_ADDRESS + 1

#define SYS_MAX_PON_ONUNUM   (SYS_MAX_PON_PORTNUM * MAXONUPERPON)  
/*#define MAXMACNUM 128*/


#define PONPORT_TYPE 1

#define SYS_MAX_PON_CARDNUM      12  
#define CARD_MAX_PON_PORTNUM     16  /*   4   8   12   16  */  
#define SYS_MAX_PON_PORTNUM      (SYS_MAX_PON_CARDNUM * CARD_MAX_PON_PORTNUM)  
#define SYS_MAX_PON_PORT_BITNUM  ((SYS_MAX_PON_PORTNUM >> 3) + 1)   

/*#define DONT_SUPORT_EXT_BOARD_MGT   (CARD_MAX_PON_PORTNUM > 4)*/

/*#define MAXPON  MAXPONCHIP*/
#define MAXLLID  258
/*#define MAXPONPORTPERCARD  4*/

/*#define MAC_ADDRESS_SIZE  6

#define LLID_UP 1
#define LLID_DOWN 2
*/
#define MAX_PON_ALARM_NUM  32

/* B--added by liwei056@2012-3-19 for D14748 */
#define DEFAULT_PON_VLAN_TPID              0x8100      
/* E--added by liwei056@2012-3-19 for D14748 */

/* B--added by liwei056@2010-1-20 for D9624 */
#define MIN_PON_BER_THRESHOLD_RATIO        8      
#define DEFAULT_PON_BER_THRESHOLD_RATIO    4      
#define DEFAULT_PON_BER_THRESHOLD_MINBYTE  10000

#define MIN_PON_FER_THRESHOLD_RATIO        10      
#define DEFAULT_PON_FER_THRESHOLD_RATIO    5      
#define DEFAULT_PON_FER_THRESHOLD_MINFRAME 1000

/* 应该至少大于2分钟，因为PON口每2分钟才上报一次BER\FER 报警 */
#define SECOND_PON_BER_ALERM_CLEAR_TIME   300
#define SECOND_PON_FER_ALERM_CLEAR_TIME   300
/* E--added by liwei056@2010-1-20 for D9624 */

#define ONU_AUTO_AUTHENTICATION      TRUE 
#define ONU_MANUAL_AUTHENTICATION    FALSE

#define OLT_DBA_UNKNOWN      0
#define OLT_INTERNAL_DBA     1
#define OLT_EXTERNAL_DBA     2


/* PON rate define */
#define PON_RATE_DOWNLINK_1G     0x01
#define PON_RATE_DOWNLINK_2G     0x02
#define PON_RATE_DOWNLINK_10G    0x04

#define PON_RATE_UPLINK_1G       0x10
#define PON_RATE_UPLINK_10G      0x40

#define PON_RATE_NORMAL_1G       (PON_RATE_UPLINK_1G  | PON_RATE_DOWNLINK_1G)
#define PON_RATE_1_2G            (PON_RATE_UPLINK_1G  | PON_RATE_DOWNLINK_2G)
#define PON_RATE_1_10G           (PON_RATE_UPLINK_1G  | PON_RATE_DOWNLINK_10G)
#define PON_RATE_10_10G          (PON_RATE_UPLINK_10G | PON_RATE_DOWNLINK_10G)


/* PON range define */
#define  PON_RANGE_CLOSE  0
#define  PON_RANGE_20KM   1 
#define  PON_RANGE_40KM   2 
#define  PON_RANGE_60KM   3
#define  PON_RANGE_80KM   4

/* pon encrypt */
#define  PON_ENCRYPTION_ENABLE        TRUE
#define  PON_ENCRYPTION_DISABLE      FALSE 
#define  PON_ENCRYPTION_PURE             1
#define  PON_ENCRYPTION_DIRECTION_DOWN   2
#define  PON_ENCRYPTION_DIRECTION_ALL    3
/*
#define  PON_MAC_LEARNING_ENABLE TRUE
#define  PON_MAC_LEARNING_DISABLE FALSE 

#define  PON_MAC_AUTOLEARNING_ENABLE  TRUE
#define  PON_MAC_AUTOLEARNING_DISABLE  FALSE
*/

#define BITS_PER_SECOND_1M   (1000 )

#define DEFAULT_UP_BW      (1000*BITS_PER_SECOND_1M)
#define DEFAULT_DOWN_BW    (1000*BITS_PER_SECOND_1M)
/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#define GW10G_DEFAULT_UP_BW      (10000*BITS_PER_SECOND_1M)
#define GW10G_DEFAULT_DOWN_BW    (10000*BITS_PER_SECOND_1M)
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/

#define GW2G_DEFAULT_UP_BW      (2500*BITS_PER_SECOND_1M)
#define GW2G_DEFAULT_DOWN_BW    (2500*BITS_PER_SECOND_1M)

// set epon 1g/1g onu, epon 10g/1g onu downlink and gpon onu uplink default bw_gr=7.5M, 
// assumed supportting 128 ONUs per port: 128 * 7.5M = 960M
#define ONU_DEFAULT_BW     ( BITS_PER_SECOND_1M * 15 / 2) 
#define ONU_DEFAULT_BE_BW  ( BITS_PER_SECOND_1M * 1000)
#define ONU_MIN_BW         64

/*default bw of GPON onu 2.5G/1.25G */
// set gpon onu downlink default bw_gr=15M, 
// assumed supportting 128 ONUs per port: 128 * 15M = 1920M
#define GPON_ONU_DEFAULT_BW     ( BITS_PER_SECOND_1M * 30 / 2) 
#define GPON_ONU_DEFAULT_BE_BW  ( BITS_PER_SECOND_1M * 1000)

/*default bw of 10G epon onu 10G/1G or 10G/10G */
// set gpon onu downlink default bw_gr=75M, 
// assumed supportting 128 ONUs per port: 128 * 75M = 9600M 
#define GW10G_ONU_DEFAULT_BW     ( BITS_PER_SECOND_1M * 150 / 2) 
#define GW10G_ONU_DEFAULT_BE_BW  ( BITS_PER_SECOND_1M * 1000)

#define BITS_PER_SECOND_64K  64
#define MAX_DOWNLINK_SPEED  (500*BITS_PER_SECOND_1M)
#define PAS5201_MAX_DOWNLINK_SPEED  (999994)

#define DEFAULT_MAC_AGING_TIME  30000   /* modified 300000 by liwei056@2011-2-16 for D12054 */ 
#define SECOND_1 1000
#define SECOND_5  5000
#define SECOND_10 10000
#define MINUTE_1  (SECOND_1 * 60 )
#define MINUTE_5  ( MINUTE_1 * 5  )
#define PONPORTTESTCYCLE  (SECOND_1*2 )
#define V2R1_TIMERTICK SECOND_1
#define V2R1_TIMERTICK_50MS  50

#define ONE_DAY   86400000  /* unit: ms */

#define DEFAULT_RTT 13524

#define  DEFAULT_COUNTER  3
#define  PRIVATE_TIME  60  /* unit: s */

#define V2R1_HIGH  1
#define V2R1_LOW  0

#define SFP_ONLINE  2/*added by yanjy,2017-6-14*/

/*
#define  STATISTIC_SAMPLING_CYCLE  4000
#define  MONITORING_CYCLE  4000 
#define  HOST_OLT_MSGS_TIMEOUT   600 
#define  OLT_RESET_TIMEOUT   20 

#define  MAXMACNUMBER  32
*/

#define  ADD_INVALID_ONU     1
#define  DELETE_INVALID_ONU  2

#define PON_LED_ON   1
#define PON_LED_OFF  0

/* pon port status : ON / OFF */
#define PON_ONLINE  1
#define PON_OFFLINE 0

/* whether pon OAM frame is limited */
#define  NO_OAM_LIMIT 1
#define  OAM_IS_LIMIT 0

typedef enum
{
    OLT_PROTECT_TYPE_NONE = 0,
    OLT_PROTECT_TYPE_A = 1,
    OLT_PROTECT_TYPE_B = 2,
    OLT_PROTECT_TYPE_C = 3,
    OLT_PROTECT_TYPE_D = 4
} OLT_PRROTECT_TYPE;

typedef enum
{
    ONU_PROTECT_TYPE_NONE = 0,
    ONU_PROTECT_TYPE_C = 1,
    ONU_PROTECT_TYPE_D = 2
} ONU_PRROTECT_TYPE;

/* pon port switchover */
#define  PROTECT_SWITCH_CTRL_DISABLE  1
#define  PROTECT_SWITCH_CTRL_AUTO     2 
#define  PROTECT_SWITCH_CTRL_FORCE    3

#define  PROTECT_SWITCH_STATUS_UNKNOWN 1
#define  PROTECT_SWITCH_STATUS_ACTIVE  2
#define  PROTECT_SWITCH_STATUS_PASSIVE 3

#define  PROTECT_SWITCHONU_EVENT_OVER  3
#define  PROTECT_SWITCHONU_EVENT_START 2
#define  PROTECT_SWITCH_EVENT_START   1
#define  PROTECT_SWITCH_EVENT_OVER    0
#define  PROTECT_SWITCH_EVENT_NOTIFY -1

#define  PROTECT_SWITCH_EVENT_SRC_HARDWARE   1
#define  PROTECT_SWITCH_EVENT_SRC_SOFTWARE   0
#define  PROTECT_SWITCH_EVENT_SRC_REMOTE     2

#define  PROTECT_SWITCH_REASON_HOSTREQUEST   0
#define  PROTECT_SWITCH_REASON_PONLOSS       1
#define  PROTECT_SWITCH_REASON_LLID_FAIL     2
#define  PROTECT_SWITCH_REASON_OLTREMOVE     3
#define  PROTECT_SWITCH_REASON_ONUNONE       4
#define  PROTECT_SWITCH_REASON_OPTICPOWER    5
#define  PROTECT_SWITCH_REASON_OPTICERROR    6
#define  PROTECT_SWITCH_REASON_UPLINKCNG     7
#define  PROTECT_SWITCH_REASON_ETHLINKCNG    8
#define  PROTECT_SWITCH_REASON_MAX           8

#define  PROTECT_SWITCH_RESULT_SUCCEED   0
#define  PROTECT_SWITCH_RESULT_FAILED    1
#define  PROTECT_SWITCH_RESULT_MAX       1

#define  PROTECT_SWITCH_EVENT_SEQ_NONE         0

#define  PROTECT_SWITCH_EVENT_FLAGS_NONE       0
#define  PROTECT_SWITCH_EVENT_FLAGS_NEEDRESUME 1

#define  PROTECT_SWITCH_TRIGGER_MANUAL       1
#define  PROTECT_SWITCH_TRIGGER_OPTICLOSS    2
#define  PROTECT_SWITCH_TRIGGER_PORTLOSS     4
#define  PROTECT_SWITCH_TRIGGER_OPTICPOWER   8 
#define  PROTECT_SWITCH_TRIGGER_OPTICERROR   16 
#define  PROTECT_SWITCH_TRIGGER_UPLINKDOWN   32 
#define  PROTECT_SWITCH_TRIGGER_ETHLINKCNG   64 
#define  V2R1_PON_PORT_SWAP_TRIGGER         (PROTECT_SWITCH_TRIGGER_MANUAL | PROTECT_SWITCH_TRIGGER_OPTICLOSS | PROTECT_SWITCH_TRIGGER_PORTLOSS)

#define   V2R1_PON_PORT_SWAP_DISABLE   1
#define   V2R1_PON_PORT_SWAP_ENABLE    2 
#define   V2R1_PON_PORT_SWAP_FORCED    3

#define   V2R1_PON_PORT_SWAP_UNKNOWN   1
#define   V2R1_PON_PORT_SWAP_ACTIVE    2
#define   V2R1_PON_PORT_SWAP_PASSIVE   3 

#define   OLT_SWAP_STATUS_SWITCHOVER(status)  ( (V2R1_PON_PORT_SWAP_ACTIVE == (status)) ? V2R1_PON_PORT_SWAP_PASSIVE : V2R1_PON_PORT_SWAP_ACTIVE )
#define   PON_SWAPSTATUS_ISVALID(s)  ( ((s) >= V2R1_PON_PORT_SWAP_UNKNOWN) && ((s) <= V2R1_PON_PORT_SWAP_PASSIVE) )
#define   PON_SWAPSTATUS_ASSERT(s)   VOS_ASSERT(PON_SWAPSTATUS_ISVALID(s))

#define   V2R1_PON_PORT_SWAP_SYNC   1
#define   V2R1_PON_PORT_SWAP_NOSYNC 0

#define   V2R1_IS_HOT_SWAP_PORT               1
#define   V2R1_ISNOT_HOT_SWAP_PORT            2
#define   V2R1_PORT1_HAS_OTHER_HOT_SWAP_PORT  3
#define   V2R1_PORT2_HAS_OTHER_HOT_SWAP_PORT  4
#define   V2R1_PORT_HAS_NO_HOT_SWAP_PORT      5
#define   V2R1_SWAP_PORT_BUT_NOT_ACTIVE       6
#define   V2R1_SWAP_PORT_ISNOT_WORKING        7
#define   V2R1_SWAP_PORT_ISDISABLED           8
#define   V2R1_SWAP_PORT_ISUNSUPPORTED        9
#define   V2R1_SWAP_PORT_MODE_REPEAT         10
#define   V2R1_SWAP_PORT_ALL_REMOTE          11
#define   V2R1_SWAP_PORT_ALL_ONE             12
#define   V2R1_SWAP_RPCMODE_NOTSUPPORT       13
/*Begin:for onu swap by jinhl@2013-02-22*/
#define   V2R1_SWAP_MODE_ALL_ONE             14
#define   V2R1_SWAP_PORT_HASONU              15
/*End:for onu swap by jinhl@2013-02-22*/


#define   V2R1_SWAPPING_NO    1
#define   V2R1_SWAPPING_NOW   2

#define   V2R1_PON_PORT_SWAP_AUTO          0
#define   V2R1_PON_PORT_SWAP_DISABLED      1
#define   V2R1_PON_PORT_SWAP_SLOWLY        2 
#define   V2R1_PON_PORT_SWAP_QUICKLY       3
#define   V2R1_PON_PORT_SWAP_ONU           4

#define   PON_SWAPMODE_ISVALID(m)  ( ((m) >= V2R1_PON_PORT_SWAP_AUTO) && ((m) <= V2R1_PON_PORT_SWAP_ONU) )
#define   PON_SWAPMODE_ASSERT(m)   VOS_ASSERT(PON_SWAPMODE_ISVALID(m))

#define   PON_SWAPMODE_ISOLT(m)    ( ((m) >= V2R1_PON_PORT_SWAP_SLOWLY) && ((m) <= V2R1_PON_PORT_SWAP_QUICKLY) )
#define   PON_SWAPMODE_ISNOTOLT(m) ( ((m) < V2R1_PON_PORT_SWAP_SLOWLY) || ((m) > V2R1_PON_PORT_SWAP_QUICKLY) )
#define   PON_SWAPMODE_ISONU(m)    ( V2R1_PON_PORT_SWAP_ONU == (m) )
#define   PON_SWAPMODE_ISNOTONU(m) ( V2R1_PON_PORT_SWAP_ONU != (m) )


#define   PON_SWAP_CFGTYPE_SOFTWARE        0x01
#define   PON_SWAP_CFGTYPE_HARDWARE        0x02
#define   PON_SWAP_CFGTYPE_INBOARD         0x04
#define   PON_SWAP_CFGTYPE_OUTBOARD        0x08
#define   PON_SWAP_CFGTYPE_INDEV           0x10
#define   PON_SWAP_CFGTYPE_OUTDEV          0x20


#define   V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MAJOR        5 
#define   V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_MINOR        2 
#define   V2R1_PON_5201_SWAP_QUICKLY_SUPPORT_FIRMWAREVER_BUILD        44 

#define   V2R1_PON_5201_SWAP_ONU_SUPPORT_FIRMWAREVER_MAJOR            5 
#define   V2R1_PON_5201_SWAP_ONU_SUPPORT_FIRMWAREVER_MINOR            2 
#define   V2R1_PON_5201_SWAP_ONU_SUPPORT_FIRMWAREVER_BUILD            56 

#define   V2R1_PON_PORT_SWAP_TIMER      10  /* unit: S  */
#define   V2R1_PON_PORT_SWAP_TIMER_MIN   3  /* unit: S  */
#define   V2R1_PON_PORT_SWAP_INTERVAL	60 /* unit: S */

#define  DEFAULT_ONU_MAC_THRESHOLD  128 /*add by sxh20110201*/

/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
typedef struct PonInitParam{
    unsigned short host_manage_iftype;
    unsigned short host_manage_address;
}PonInitParam_S;
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

extern struct CLI_return_code_str_t  PAS_CLI_return_code_str[];

extern unsigned int V2R1_AutoProtect_Timer;
extern unsigned int V2R1_AutoProtect_Trigger;
/*extern unsigned int V2R1_TimeCounter;*/	/* removed by xieshl 20100730 */

extern unsigned int *PonPortToSwPort;
extern unsigned int *PonPortToSwLogPort;

extern int           GlobalPonAddrTblNum;
extern unsigned char GlobalPonStaticMacAddr[MAXOLTADDRNUM][OLT_ADDR_BYTELEN];	    

extern PON_olt_init_parameters_t	PAS_initialization_parameters_5001;
extern PON_olt_initialization_parameters_t  PAS_initialization_parameters_5201;
extern PON_olt_cni_port_mac_configuration_t  PAS_port_cni_parameters;
extern PAS_pon_initialization_parameters_t PAS_init_para;
extern PON_olt_update_parameters_t  PAS_updated_parameters_5001;
extern PON_update_olt_parameters_t  PAS_updated_parameters_5201;
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern GW10G_PON_update_olt_parameters_t PAS_updated_parameters_8411 /*= { 0 }*/;
extern PAS_system_parameters_t  PAS_system_parameters;
extern PAS_pon_initialization_parameters_t  PAS_init_para;
/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
extern PonInitParam_S PON_init_params;
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

extern unsigned char *PonFileType[];
/*extern unsigned char *PonAlarmArray[];*/

extern unsigned long OnuPendingDataSemId ;

extern bool  CTC_STACK_Init;

extern int pon_swap_switch_enable; 

extern bool V2R1_discard_llid_unlearned_sa;
extern PON_pon_network_traffic_direction_t  V2R1_discard_unknown_da ;

typedef struct{
unsigned char  dev_type[16];  /* 文件所属类型，字符串,16字节 */
unsigned int   loc_offset;    /* 文件在FLASH段内偏移 */
unsigned int  file_len;       /* 文件在FLASH段内长度*/
unsigned int  compress_flag;  /* 文件压缩标志，0：表示不压缩，0x000000ff ：表示压缩 */
unsigned char  reserve[8];    /* 保留，8字节 */
unsigned char  file_ver[92];  /* 版本号，字符串，92字节 */
}__attribute__((packed))driver_file_desc_t;

#define  PON_FIRMWARE_FLASH_FILE_ID  7
#define  PON_DBA_FLASH_FILE_ID   10

typedef enum{
	PONPORT_ENABLE=1,
	PONPORT_DISABLE
}PONPORT_ADMIN;
/*
typedef enum {
	PONPORT_WORKING,
	PONPORT_TESTING,
	PONPORT_UPDATE,
	PONPORT_NOTWORKING,
	PONPORT_OFFLINE,
	PONPORT_RESETING,
	PONPORT_FAULT
}PONPORT_OPER;
*/
typedef enum{
	PONPORT_UP=1,
	PONPORT_DOWN,
	PONPORT_UNKNOWN,
	PONPORT_LOOP,
	PONPORT_UPDATE,
	PONPORT_INIT,
	PONPORT_DEL,
	PONPORT_OPER_MAX
}PONPORT_OPER;

typedef enum {
	PONLLID_UP =1,
	PONLLID_DOWN
}PONLLID_STATUS;

typedef enum{
    PONPORT_TX_DIRECT   = 0,
    PONPORT_TX_ACTIVED  = 1,
    PONPORT_TX_SHUTDOWN = 2,
    PONPORT_TX_SFPCHK   = 4,
    PONPORT_TX_VERCHK   = 8,
    PONPORT_TX_SWITCH   = 128,
    PONPORT_TX_ALL      = 255
}PONPORT_TX_REASON;

/* B--added by liwei056@2010-1-25 for Pon-BackupMonitor */
typedef enum{
	PONBACKUP_NONE = 0,
	PONBACKUP_DOWN,
	PONBACKUP_NO_ONU,
	PONBACKUP_NOT_SAMELINE,
	PONBACKUP_MAX
}PONBACKUP_STATUS;
/* E--added by liwei056@2010-1-25 for Pon-BackupMonitor */

typedef struct If_Index {
    unsigned int  type: 6;      /* 接口类型 */
    unsigned int  subcard : 3;
    unsigned int  slot : 5;        /* 槽位号 */
    unsigned int  port : 6;      /* 端口号 */
    unsigned int  subNo : 8;  /* 子端口号 */
    unsigned int  Onu_FE : 4; 
}IF_Index_S;

typedef struct {
    unsigned char  MAC[BYTES_IN_MAC_ADDRESS];
    unsigned int  nextMac;
}MacTable_S;

typedef struct {
	unsigned int ipAddr;
	unsigned int NextIp;
}IpTable_S;

typedef struct {
	unsigned int  ipAddr;
	unsigned short int  port;
	unsigned int Next_Ip_Port;
}Ip_Port_Table_S;

typedef struct {
	unsigned short int  Vid;
	unsigned int  NextVid;
}VlanId_Table_S;

typedef struct{
	unsigned  short int EtherType;
	unsigned  int Next_EthType;
}Ether_Type_Table_S;

typedef struct{
	unsigned short int  IpType;
	unsigned  int Next_IpType;
}Ip_Type_Table_S;

typedef enum
{
    PENDING_REASON_CODE_MIN = 0,
    PENDING_REASON_CODE_ONU_FULL,
    PENDING_REASON_CODE_ONU_CONFICT,
    PENDING_REASON_CODE_TRAFFIC_DISABLE,
    PENDING_REASON_CODE_StdOAM_FAIL,	
    PENDING_REASON_CODE_ExtOAM_FAIL,
    PENDING_REASON_CODE_GET_ONU_REGISTER_INFO_FAIL,
    PENDING_REASON_CODE_GET_ONU_EQU_INFO_FAIL,
    PENDING_REASON_CODE_PON_PASSIVE,
    PENDING_REASON_CODE_ONU_AUTH_FAIL,
    PENDING_REASON_CODE_ONU_AUOR_FAIL,
    PENDING_REASON_CODE_SET_BW_FAIL,
    PENDING_REASON_CODE_REQUEST_BW_FAIL,
    PENDING_REASON_CODE_TIMEOUT,
    PENDING_REASON_CODE_GET_ONUVER_FAIL,	   
	PENDING_REASON_CODE_ONU_DENY,
    PENDING_REASON_CODE_MAX
}PendingReason_S;
#define GPON_ONU_SERIAL_NUM_STR_LEN (16+1)
#define GPON_ONU_PASSWARD_STR_LEN (20+1)

typedef enum
{
	PENDINGONU_GPON=1,
	PENDINGONU_EPON=2
}Onu_type;

typedef union OnuOnlyMar{
	unsigned char serial_no[GPON_ONU_SERIAL_NUM_STR_LEN];
	UCHAR MacAddr[BYTES_IN_MAC_ADDRESS];
	}OnuOnlyMark;
typedef struct onutype{
	UCHAR pendingOnutype;
	OnuOnlyMark OnuMark;
}onu_type_t;

typedef struct pendingOnu{
	short int otherPonIdx;
	short int Llid;
	onu_type_t OnuMarkInfor;
	short int counter;
    PendingReason_S code;
	struct pendingOnu  *Next;
}pendingOnu_S;

typedef struct {
	USHORT otherPonIdx;
	USHORT Llid;
    PendingReason_S code;    
	onu_type_t OnuMarkInfor;
} pendingOnuNode_t;
typedef struct {
	UCHAR pendingCount;
	UCHAR conflictCount;
	pendingOnuNode_t pendingList[MAXONUPERPONNOLIMIT];
	pendingOnuNode_t conflictList[MAXONUPERPONNOLIMIT];
} __attribute__((packed)) pendingOnuList_t;

/********************declear for gpon(begin)******************************/


typedef struct  
{
    unsigned char                   serial_no[GPON_ONU_SERIAL_NUM_STR_LEN];
    unsigned char                   password[GPON_ONU_PASSWARD_STR_LEN];
}GPON_onu_auth_entry_t;

typedef struct __gpon_onu_auth_t {
	UCHAR authIdx;
	UCHAR authRowStatus;
	UCHAR authOnuIdx;
	UCHAR authOnuLlid;
	GPON_onu_auth_entry_t authEntry;
	struct __gpon_onu_auth_t *next;
} gpon_onu_auth_t;

typedef struct {
	UCHAR authIdx;
	UCHAR authRowStatus;
	UCHAR authOnuIdx;
	UCHAR authOnuLlid;
	GPON_onu_auth_entry_t authEntry;
} __attribute__ ((packed)) gpon_onu_auth_entry_t;
/********************declear for gpon(end)******************************/
extern int GW_GetPendingOnu(short int olt_id, pendingOnuList_t *onuList);
extern int OLT_GetPendingOnuList(short int olt_id, pendingOnuList_t *onuList);
extern int GetPendingOnu( short int PonPortIdx, pendingOnuList_t *onuList );

/* modified by xieshl 20110110, 为了便于在板间使用， 问题单11796 */

#define COMMNAME_LEN        16

#define ONU_UPD_RESULT_TYPE_UNKNOWN		0
#define ONU_UPD_RESULT_TYPE_WAITING		1
#define ONU_UPD_RESULT_TYPE_TIME			2
#define ONU_UPD_RESULT_TYPE_RATE			3
typedef struct{
	char onuid;    /*range: 0-63*/
	char type;
	char version[COMMNAME_LEN];
	char swversion[COMMNAME_LEN];
	/*char result[20];*/
	char result_type;
	long result_data;
} __attribute__((packed)) onuUpdateStatus_t;

typedef struct {
	char num;
	onuUpdateStatus_t updStat[MAXONUPERPONNOLIMIT];
} __attribute__((packed)) onuUpdateStatusList_t;

extern int GW_GetUpdatingOnu(short int olt_id, onuUpdateStatusList_t *onuList);
extern int OLT_GetUpdatingOnuList(short int olt_id, onuUpdateStatusList_t *onuList);
extern int GW_GetUpdatedOnu(short int olt_id, onuUpdateStatusList_t *onuList);
extern int OLT_GetUpdatedOnuList(short int olt_id, onuUpdateStatusList_t *onuList);
extern int getOnuOamUpdatedStatus( ulong_t PonPortIdx, onuUpdateStatusList_t *pbuf );
extern int getOnuOamUpdatingStatus( ulong_t PonPortIdx, onuUpdateStatusList_t *pbuf );


#define    MAX_UPDATE_ONU_NUM   2   /*可同时更新的ONU个数限制*/
#define    MAX_UPDATE_GPON_ONU_NUM   5   /*可同时更新的ONU个数限制*/
typedef struct {
	USHORT ponid_list[MAX_UPDATE_ONU_NUM+1];
	USHORT onuid_list[MAX_UPDATE_ONU_NUM+1];
	USHORT count;
	USHORT reserved;
}onu_updating_counter_t;
extern int GW_GetOnuUpdatingStatusLocal( short int olt_id, onu_updating_counter_t *pList );
extern int OLT_GetOnuUpdatingStatusBySlot( short int olt_id, onu_updating_counter_t *pList );

typedef struct {
	ULONG upd_mode;
	ULONG msg_type;
	ULONG onuDevIdx;
	ULONG file_type;
	UCHAR reserved[20];		/* ONU_TYPE_LEN+4  */	/* modified by xieshl 20110802, 问题单11878 */
}onu_update_msg_t;
extern int GW_SetOnuUpdateMsg( short int olt_id, onu_update_msg_t *pMsg );
extern int OLT_SetOnuUpdateMsg( short int olt_id, onu_update_msg_t *pMsg );

typedef struct {
	USHORT wait_onu[MAXONUPERPONNOLIMIT+1];
	USHORT reserved;
}onu_update_waiting_t;
extern int GW_GetOnuUpdateWaiting( short int olt_id, onu_update_waiting_t *pList );
extern int OLT_GetOnuUpdateWaiting( short int olt_id, onu_update_waiting_t *pList );
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int GW_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag);
extern int GW_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress);
extern int GW_BroadCast_CliCommand(short int olt_id, int action_code);

/*End:for onu swap by jinhl@2013-04-27*/
#define MAX_VID_DOWNLINK 256

typedef struct PonConfigInfo{
	unsigned int   PortAdminStatus;
	PON_rtt_t      MaxRtt;
	unsigned short MaxOnu;
	unsigned short MaxMacNum;
	unsigned int   DefaultOnuBW;
	unsigned int   DefaultXGOnuBW; /*10GEPON onu deafult BW*/
	unsigned int   DefaultGPONOnuBW; /*GPON onu deafult BW*/
	unsigned int   MaxBW;
	unsigned long  AlarmMask/*[MAX_PON_ALARM_NUM/8]*/; 
	unsigned int   MACAgeingTime;
	unsigned char  MacSelfLearningCtrl;
	unsigned char  ApsCtrl;
	double   BERThreshold_gen;
	double   BERThreshold_dis;
	double   FERThreshold_gen;
	double   FERThreshold_dis;
	unsigned int  DBA_mode;	
	int testTimer;
	int testRemain;

	unsigned int AutoRegisterFlag;

	unsigned int  EncryptType;   	  /* 加密类型 */
	unsigned int  EncryptEnable;	  /* 加密使能 */
	/* moved  to onu management table */
	unsigned int  EncryptDirerction ; /* 加密方向*/
	unsigned int  EncryptKeyTime;     /* 密钥交互时间*/

	int  range;

	MacTable_S *InvalidOnuTablePtr;

	unsigned char  CTC_EncryptKeyUpdateTime;
	unsigned char  CTC_EncryptTimeingThreshold;
	unsigned short int   CTC_EncryptKeyNoReplyTime;
	
}PonConfigInfo_S;

typedef struct PonLLIDConfigInfo{
	unsigned char  MaxMAC; /* not supported by PAS5001 currently */

}PonLLIDConfigInfo_S;

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
typedef struct Olt_llid_vlan_manipulation_{
	short int  vlan_manipulation;
	short int  original_vlan_id;
	short int  new_vlan_id;
	short int new_priority;
	PON_olt_vlan_uplink_type_t  ethernet_type;
	/*struct  Olt_llid_vlan_trans_ *Next_vlan_Trans;*/
}__attribute__((packed))Olt_llid_vlan_manipulation;
#endif

typedef struct PonPortmeteringTable{
	CHAR sfpMeteringSupport; /* PON 口是否支持光功率检测*/
	CHAR SFPInitType;
	CHAR powerMeteringSupport;
	CHAR SFPTypeMismatchAlarm;  /* pon口光模块类型不匹配告警*/
	LONG transOpticalPower;	/*发送光功率, unit 0.1dbm */
	LONG ponTemperature;	/*模块温度, unit C */
	LONG ponVoltageApplied;	/*模块电压,unit 0.1V */
	LONG ponBiasCurrent;	/*偏置电流,unit 1mA */
	LONG recvOpticalPower[MAXONUPERPONNOLIMIT];	/*接收光功率*/
	UCHAR recvPowerFlag[MAXONUPERPONNOLIMIT];
	USHORT txAlarmStatus;
	UCHAR rxAlarmStatus[MAXONUPERPONNOLIMIT];
	UCHAR sfp_type_invalid_counter;
}PonPortmeteringInfo_S;
typedef struct UplinkPortMeteringTable{
	CHAR powerMeteringSupport;
	CHAR LosAlarmFlag;
	
	CHAR SFPType;
	CHAR resved;
	ULONG AlarmStatus;
	LONG recvOpticalPower;				/*接收光功率*/
	LONG transOpticalPower;			/*发送光功率, unit 0.1dbm */
	LONG Temperature;					/*模块温度, unit C */
	LONG Voltage;		/*模块电压,unit 0.1V */
	LONG BiasCurrent;			/*偏置电流,unit 1mA */	
}UplinkPortMeteringInfo_S;

typedef struct{
	LONG recvOPlow;
	LONG recvOPhigh;
	LONG OltRecvOPlow[2];
	LONG OltRecvOPhigh[2];
	LONG tranOPlow;
	LONG tranOPhigh;
	LONG OltTranOPlow[2];
	LONG OltTranOPhigh[2];
	LONG ponTemLow;
	LONG ponTemHigh;
	LONG OltTemLow[2];
	LONG OltTemHigh[2];
	LONG ponVolLow;
	LONG ponVolHigh;
	LONG OltVolLow[2];
	LONG OltVolHigh[2];
	LONG ponBiasCurLow;
	LONG ponBiasCurHigh;
	LONG OltBiasCurLow[2];
	LONG OltBiasCurHigh[2];
	LONG OltLaserAlwaysOnThresh;
	LONG OltMonLaserAlwaysOnThresh;
	LONG OltMonitorEnable;
	LONG OltMonInterval;
	LONG ponOnPower;
	LONG upporttransOPlow[2]; /* the second for 10GE */
	LONG upporttransOPhigh[2];
	LONG upportrecvOPlow[2];
	LONG upportrecvOPhigh[2];
	LONG upportTemlow[2];
	LONG upportTemhigh[2];
	LONG upportVollow[2];
	LONG upportVolhigh[2];
	LONG upportCurlow[2];
	LONG upportCurhigh[2];
	LONG UplinkMonInterval;
}eponOpticalPowerThresholds_t;
typedef struct{
	LONG powerVarDeadZone;
	LONG volVarDeadZone;
	LONG curVarDeadZone;
	LONG temVarDeadZone;
}eponOpticalPowerDeadZone_t;

typedef enum{
	XFP_TYPE_UNKNOWN = 0,
	XFP_TYPE_SYM_10_10 = 1,
	XFP_TYPE_ASYM_10_1 = 2
} XgeponXfpType_e;

/* B--added by liwei056@2010-5-11 for OLT-API-IF */
#if 1
/* -------------------OLT基本API------------------- */
typedef int (* ctss_olt_is_exist_f)(short int olt_id, bool *status); 
typedef int (* ctss_olt_get_chip_typeid_f)(short int olt_id, int *type); 
typedef int (* ctss_olt_get_chip_typename_f)(short int olt_id, char typename[OLT_CHIP_NAMELEN]); 
typedef int (* ctss_olt_reset_pon_f)(short int olt_id);
typedef int (* ctss_olt_remove_olt_f)(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt);

typedef int (* ctss_olt_copy_olt_f)(short int olt_id, short int dst_olt_id, int copy_flags);
typedef int (* ctss_olt_is_support_cmd_f)(short int olt_id, short int *cmd);
typedef int (* ctss_olt_set_debug_mode_f)(short int olt_id, int debug_flags, int debug_mode);
typedef int (* ctss_olt_set_init_params_f)(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address);
typedef int (* ctss_olt_set_system_params_f)(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout);

typedef int (* ctss_olt_set_pon_i2c_extinfo_f)(short int olt_id, eeprom_gfa_epon_ext_t *pon_info);
typedef int (* ctss_olt_get_pon_i2c_extinfo_f)(short int olt_id, eeprom_gfa_epon_ext_t *pon_info);
typedef int (* ctss_olt_set_card_i2c_info_f)(short int olt_id, board_sysinfo_t *board_info);
typedef int (* ctss_olt_get_card_i2c_info_f)(short int olt_id, board_sysinfo_t *board_info);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_write_mdio_register_f)(short int olt_id, short int phy_address, short int reg_address, unsigned short int value );

typedef int (* ctss_olt_read_mdio_register_f)(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value );
typedef int (* ctss_olt_read_i2c_register_f)(short int olt_id, short int device, short int register_address, short int *data );
typedef int (* ctss_olt_gpio_access_f)(short int olt_id, short int line_number, PON_gpio_line_io_t set_direction, short int set_value, PON_gpio_line_io_t *direction, bool *value);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_gpio_read_f)(short int olt_id, int func_id, bool *value);
typedef int (* ctss_olt_gpio_write_f)(short int olt_id, int func_id, bool value);

typedef int (* ctss_olt_send_chip_cli_f)(short int olt_id, unsigned short size, unsigned char *command);
typedef int (* ctss_olt_set_device_name_f)(short int olt_id, char* pValBuf, int valLen);
/* olt_id为每块pon芯片的第一个pon口 for reset pon chip by jinhl*/
typedef int (* ctss_olt_reset_pon_chip_f)(short int olt_id);
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
typedef int (* ctss_olt_get_version_f)(short int olt_id, PON_device_versions_t *device_versions); 
typedef int (* ctss_olt_get_dba_version_f)(short int olt_id, OLT_DBA_version_t *dba_version);
typedef int (* ctss_olt_chk_version_f)(short int olt_id, bool *is_compatibled);
typedef int (* ctss_olt_chk_dba_version_f)(short int olt_id, bool *is_compatibled);
typedef int (* ctss_olt_get_cni_link_status_f)(short int olt_id, bool *status); 

typedef int (* ctss_olt_get_pon_workstatus_f)(short int olt_id, int *work_status);
typedef int (* ctss_olt_set_admin_status_f)(short int olt_id, int admin_status);
typedef int (* ctss_olt_get_admin_status_f)(short int olt_id, int *admin_status);
typedef int (* ctss_olt_set_vlan_tpid_f)(short int olt_id, unsigned short int vlan_tpid);
typedef int (* ctss_olt_set_vlan_qinq_f)(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config);

typedef int (* ctss_olt_set_pon_frame_sizelimit_f)(short int olt_id, short int jumbo_length);
typedef int (* ctss_olt_get_pon_frame_sizelimit_f)(short int olt_id, short int *jumbo_length);
typedef int (* ctss_olt_oam_is_limit_f)(short int olt_id, bool *oam_limit);
typedef int (* ctss_olt_update_pon_params_f)(short int olt_id, int max_range, int mac_agetime);
typedef int (* ctss_olt_set_pppoerelay_mode_f)(short int olt_id, int set_mode, int relay_mode);

typedef int (* ctss_olt_set_pppoerelay_params_f)(short int olt_id, short int param_id, int param_value1, int param_value2);
typedef int (* ctss_olt_set_dhcprelay_mode_f)(short int olt_id, int set_mode, int relay_mode);
typedef int (* ctss_olt_set_igmp_auth_mode_f)(short int olt_id, int auth_mode);
typedef int (* ctss_olt_send_frame_2_pon_f)(short int olt_id, short int llid, void *eth_frame, int frame_len);
typedef int (* ctss_olt_send_frame_2_cni_f)(short int olt_id, short int llid, void *eth_frame, int frame_len);

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_get_vid_downlink_mode_f)(short int olt_id, PON_vlan_tag_t vid, PON_olt_vid_downlink_config_t *vid_downlink_config); 
typedef int (* ctss_olt_delete_vid_downlink_mode_f)(short int olt_id, PON_vlan_tag_t vid);
typedef int (* ctss_olt_get_olt_parameters_f)(short int olt_id, PON_olt_response_parameters_t *olt_parameters);
typedef int (* ctss_olt_set_olt_igmp_snooping_mode_f)(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration);
typedef int (* ctss_olt_get_olt_igmp_snooping_mode_f)(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration);

typedef int (* ctss_olt_set_olt_mld_forwarding_mode_f)(short int olt_id, disable_enable_t mode);
typedef int (* ctss_olt_get_olt_mld_forwarding_mode_f)(short int olt_id, disable_enable_t *mode);
typedef int (* ctss_olt_set_dba_report_format_f)(short int olt_id, PON_DBA_report_format_t report_format);
typedef int (* ctss_olt_get_dba_report_format_f)(short int olt_id, PON_DBA_report_format_t *report_format);   
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
/*Begin:for onu swap by jinhl@2013-04-27*/
typedef int (* ctss_olt_update_prov_bwinfo_f)(short int olt_id);
/*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
typedef int (* ctss_olt_llid_is_exist_f)(short int olt_id, short int llid, bool *status);
typedef int (* ctss_olt_deregister_llid_f)(short int olt_id, short int llid, bool iswait);
typedef int (* ctss_olt_get_llid_mac_f)(short int olt_id, short int llid, mac_address_t onu_mac);
typedef int (* ctss_olt_get_llid_reginfo_f)(short int olt_id, short int llid, onu_registration_info_t *onu_info);
typedef int (* ctss_olt_auth_llid_f)(short int olt_id, short int llid, bool auth_mode);

typedef int (* ctss_olt_set_llid_sla_f)(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA);
typedef int (* ctss_olt_get_llid_sla_f)(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA);
typedef int (* ctss_olt_set_llid_police_f)(short int olt_id, short int llid, LLID_POLICE_INFO_t *police);
typedef int (* ctss_olt_get_llid_police_f)(short int olt_id, short int llid, LLID_POLICE_INFO_t *police);
typedef int (* ctss_olt_set_llid_dba_type_f)(short int olt_id, short int llid, int dba_type, short int *dba_error);

typedef int (* ctss_olt_get_llid_dba_type_f)(short int olt_id, short int llid, int *dba_type, short int *dba_error);
typedef int (* ctss_olt_set_llid_dba_flags_f)(short int olt_id, short int llid, unsigned short dba_flags);
typedef int (* ctss_olt_get_llid_dba_flags_f)(short int olt_id, short int llid, unsigned short *dba_flags);
typedef int (* ctss_olt_get_llid_heart_oam_f)(short int olt_id, short int llid, unsigned short *send_period, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_timeout, unsigned short *recv_size, unsigned char *recv_data, bool *recv_IgnoreTrailingBytes);
typedef int (* ctss_olt_set_llid_heart_oam_f)(short int olt_id, short int llid, unsigned short send_period, unsigned short send_size, unsigned char *send_data, unsigned short recv_timeout, unsigned short recv_size, unsigned char *recv_data, bool recv_IgnoreTrailingBytes);
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
typedef int (* ctss_olt_get_all_onunum_f)(short int olt_id, int onu_flags, int *onu_number);
typedef int (* ctss_olt_get_all_onu_f)(short int olt_id, OLT_onu_table_t *onu_table);
typedef int (* ctss_olt_clear_all_onu_f)(short int olt_id);
typedef int (* ctss_olt_resume_all_onu_status_f)(short int olt_id, int resume_reason, int resume_mode);
typedef int (* ctss_olt_set_onu_auth_mode_f)(short int olt_id, int auth_mode);
typedef int (* ctss_olt_set_auth_entry_f)(short int olt_id, int code);
typedef int (* ctss_olt_set_gpon_auth_entry_f)(short int olt_id, gpon_onu_auth_entry_t *entry);
typedef int (* ctss_olt_set_onu_default_max_mac_f)(short int olt_id, int max_mac);

typedef int (* ctss_olt_set_onu_downlink_police_mode_f)(short int olt_id, int police_mode);
typedef int (* ctss_olt_set_onu_downlink_police_param_f)(short int olt_id, int BwBurstSize, int BwPreference, int BwWeightSize);
typedef int (* ctss_olt_set_onu_uplink_dba_param_f)(short int olt_id, int BwFixedPktSize, int BwBurstSize, int BwWeightSize);

typedef int (* ctss_olt_set_onu_bind_mode_f)(short int olt_id, int bind_mode);
typedef int (* ctss_olt_chk_onu_reg_ctrl_f)(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id);
typedef int (* ctss_olt_set_onu_downlink_pri2cos_f)(short int olt_id, OLT_pri2cosqueue_map_t *map);
typedef int (* ctss_olt_active_pending_onus_f)(short int olt_id);
typedef int (* ctss_olt_active_pending_onu_f)(short int olt_id, UCHAR *mac);

typedef int (* ctss_olt_active_conflict_pending_onus_f)(short int olt_id, short int conf_olt_id);
typedef int (* ctss_olt_active_conflict_pending_onu_f)(short int olt_id, short int conf_olt_id, UCHAR *mac);
typedef int (* ctss_olt_get_pending_onu_f)(short int olt_id, pendingOnuList_t *onuList );
typedef int (* ctss_olt_get_update_onu_f)(short int olt_id, onuUpdateStatusList_t *onuList );
typedef int (* ctss_olt_get_updating_status_f)(short int olt_id, onu_updating_counter_t * );

typedef int (* ctss_olt_set_update_onu_msg_f)(short int olt_id, onu_update_msg_t * );
typedef int (* ctss_olt_get_update_waiting_f)(short int olt_id, onu_update_waiting_t * );
typedef int (* ctss_olt_set_onu_bw_params_f)(short int olt_id, int uplink_bwradio, int dwlink_bwradio);
typedef int (* ctss_olt_set_p2p_mode_f)(short int olt_id, int p2p_mode);
typedef int (* ctss_olt_get_p2p_mode_f)(short int olt_id, int *p2p_mode);
typedef int	(* ctss_olt_set_onu_TxPowerSupplyControl_f)(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter);

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_get_onu_mode_f)(short int olt_id, short int onu_id);
typedef int (* ctss_olt_get_mac_address_authentication_f)(short int olt_id, unsigned char	*number_of_mac_address, mac_addresses_list_t mac_addresses_list);
typedef int (* ctss_olt_set_authorize_mac_according_list_f)(short int olt_id, bool	authentication_according_to_list);
typedef int (* ctss_olt_get_authorize_mac_according_list_f)(short int olt_id, bool	*authentication_according_to_list);
typedef int (* ctss_olt_get_downlink_buffer_config_f)(short int olt_id, PON_downlink_buffer_priority_limits_t *priority_limits);

typedef int (* ctss_olt_get_oam_information_f)(short int olt_id, short int llid, PON_oam_information_t  *oam_information);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

/*Begin:for onu swap by jinhl@2013-02-22*/
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
typedef int (* ctss_olt_resume_llid_status_f)(short int olt_id, short int llid, int resume_reason, int resume_mode);
#endif
typedef int (* ctss_olt_search_free_onuIdx_f)(short int olt_id, unsigned char *MacAddress, short int *reg_flag);
typedef int (* ctss_olt_get_act_onuIdx_byMac_f)(short int olt_id, unsigned char *MacAddress);
/*End:for onu swap by jinhl@2013-02-22*/
typedef int (* ctss_olt_broadcast_clicommand_f)(short int olt_id, int action_code);
typedef int (* ctss_olt_set_onu_mac_auth_f)(short int olt_id, int mode, mac_address_t mac);
typedef int (* ctss_olt_set_onu_default_bw_f)(short int olt_id, ONU_bw_t *default_bw);
typedef int (* ctss_olt_set_max_onu_f)(short int olt_id, int max_mac);
typedef int (* ctss_olt_get_onu_conf_del_status_f)(short int olt_id, char* name, int *status);
typedef int (* ctss_olt_set_onu_port_stats_timeout_f)(short int olt_id,ONU_PORTSTATS_TIMER_NAME_E timer_type,LONG timeout);
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
typedef int (* ctss_olt_set_encrypt_mode_f)(short int olt_id, int encrypt_mode);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_set_encryption_preamble_mode_f)(short int olt_id, bool encrypt_mode);
typedef int (* ctss_olt_get_encryption_preamble_mode_f)(short int olt_id, bool *encrypt_mode);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_get_llid_encrypt_mode_f)(short int olt_id, short int llid, bool *encrypt_mode);
typedef int (* ctss_olt_start_llid_encrypt_f)(short int olt_id, short int llid);

typedef int (* ctss_olt_finish_llid_encrypt_f)(short int olt_id, short int llid, short int status);
typedef int (* ctss_olt_stop_llid_encrypt_f)(short int olt_id, short int llid);
typedef int (* ctss_olt_set_llid_encrypt_key_f)(short int olt_id, short int llid, PON_encryption_key_t key);
typedef int (* ctss_olt_finish_llid_encrypt_key_f)(short int olt_id, short int llid, short int status);
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
typedef int (* ctss_olt_set_mac_agingtime_f)(short int olt_id, int aging_time);
typedef int (* ctss_olt_set_addrtbl_config_f)(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg);
typedef int (* ctss_olt_get_addrtbl_config_f)(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg);
typedef int (* ctss_olt_get_mac_addrtbl_f)(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl);
typedef int (* ctss_olt_add_mac_addrtbl_f)(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);

typedef int (* ctss_olt_del_mac_addrtbl_f)(short int olt_id, short int addr_num, PON_address_table_t addr_tbl);
typedef int (* ctss_olt_remove_mac_f)(short int olt_id, mac_address_t mac);
typedef int (* ctss_olt_reset_mac_addrtbl_f)(short int olt_id, short int llid, int addr_type);
typedef int (* ctss_olt_set_mac_threshold_f)(short int olt_id,  ULONG mac_threshold);
typedef int (* ctss_olt_set_mac_check_enable_f)(short int olt_id,  ULONG enable);

typedef int (* ctss_olt_set_mac_check_timer_f)(short int olt_id,ULONG period);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_set_addr_tab_full_handling_mode_f)(short int olt_id, bool remove_entry_when_table_full);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_get_llid_by_user_mac_address)(short int olt_id, mac_address_t mac, short int *llid);
typedef int (* ctss_olt_get_mac_addrVlantbl_f)(short int olt_id, short int *addr_num, PON_address_vlan_table_t addr_tbl);/*for GPON by jinhl*/
#endif

#if 1
/* -------------------OLT 光路管理API----------- */
typedef int (* ctss_olt_get_optical_capability_f)(short int olt_id, OLT_optical_capability_t *optical_capability);
typedef int (* ctss_olt_get_optics_detail_f)(short int olt_id, OLT_optics_detail_t *optics_params);
typedef int (* ctss_olt_set_pon_range_f)(short int olt_id, unsigned int max_range, unsigned int max_rtt);
typedef int (* ctss_olt_set_optical_tx_mode_f)(short int olt_id, int tx_mode);
typedef int (* ctss_olt_get_optical_tx_mode_f)(short int olt_id, int *tx_mode);

typedef int (* ctss_olt_set_virtual_scope_adc_config_f)(short int olt_id, PON_adc_config_t *adc_config);                   
typedef int (* ctss_olt_get_virtual_scope_measurement_f)(short int olt_id, short int onu_id, PON_measurement_type_t measurement_type, 	void *configuration, short int config_len, void *result, short int res_len); 
typedef int (* ctss_olt_get_virtual_scope_rssi_measurement_f)(short int olt_id, short int llid, PON_rssi_result_t *rssi_result);                   
typedef int (* ctss_olt_get_virtual_scope_onuvoltage_f)(PON_olt_id_t olt_id, short int llid, float *voltage,unsigned short int *sample, float *dbm);
typedef int (* ctss_olt_set_virtual_llid_f)(short int olt_id, short int llid, PON_virtual_llid_operation_t operation);

typedef int (* ctss_olt_set_optical_tx_mode2_f)(short int olt_id, int tx_mode, int tx_reason);
#endif

#if 1
/* -------------------OLT 监控统计管理API---- */
typedef int (* ctss_olt_get_raw_statistics_f)(short int olt_id, OLT_raw_stat_item_t *stat_item);
typedef int (* ctss_olt_reset_counters_f)(short int olt_id);
typedef int (* ctss_olt_set_ber_alarm_config_f)(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes);
typedef int (* ctss_olt_set_fer_alarm_config_f)(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames);
typedef int (* ctss_olt_set_ber_alarm_params_f)(short int olt_id, int alarm_thresold, int alarm_min_error_bytes);

typedef int (* ctss_olt_set_fer_alarm_params_f)(short int olt_id, int alarm_thresold, int alarm_min_error_frames);
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
typedef int (* ctss_olt_set_alarm_config_f)(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length);
typedef int (* ctss_olt_get_alarm_config_f)(short int olt_id, short int source, PON_alarm_t type, bool *activate, void	*configuration);
typedef int (* ctss_olt_get_statistics_f)(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data);
typedef int (* ctss_olt_olt_self_test_f)(short int olt_id);

typedef int (* ctss_olt_link_test_f)(short int olt_id, PON_onu_id_t onu_id, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results);
typedef int (* ctss_olt_set_llid_fec_mode_f)(short int olt_id, short int llid, bool downlink_fec);
typedef int (* ctss_olt_get_llid_fec_mode_f)(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec);
typedef int (* ctss_olt_sys_dump_f)(short int olt_id, short int llid, int dump_type);
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------OLT 倒换API---------------- */
typedef int (* ctss_olt_get_hotswap_cap_f)(short int olt_id, int *swap_cap);
typedef int (* ctss_olt_get_hotswap_mode_f)(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status);
typedef int (* ctss_olt_set_hotswap_mode_f)(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags);
typedef int (* ctss_olt_force_hotswap_f)(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags);
typedef int (* ctss_olt_set_hotswap_param_f)(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2);

typedef int (* ctss_olt_rdn_onu_register_f)(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info);
typedef int (* ctss_olt_set_rdn_config_f)(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable);
typedef int (* ctss_olt_set_rdn_switch_f)(short int olt_id);
typedef int (* ctss_olt_rdn_is_exist_f)(short int olt_id, bool *status);
typedef int (* ctss_olt_reset_rdn_record_f)(short int olt_id, int rdn_state);

typedef int (* ctss_olt_get_rdn_state_f)(short int olt_id, int *state);
typedef int (* ctss_olt_set_rdn_state_f)(short int olt_id, int state);
typedef int (* ctss_olt_get_rdn_addrtbl_f)(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl);
typedef int (* ctss_olt_remove_rdn_olt_f)(short int olt_id);
typedef int (* ctss_olt_get_llid_rdn_db_f)(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db);

typedef int (* ctss_olt_set_llid_rdn_db_f)(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db);
typedef int (* ctss_olt_remove_remote_olt_f)(short int olt_id, short int partner_olt_id);
typedef int (* ctss_olt_swap_remote_olt_f)(short int olt_id, short int partner_olt_id);
typedef int (* ctss_olt_add_remote_olt_f)(short int olt_id, short int partner_olt_id);
typedef int (* ctss_olt_loose_remote_olt_f)(short int olt_id, short int partner_olt_id);

/*Begin:for onu swap by jinhl@2013-02-22*/
typedef int (*ctss_olt_rdn_llid_add_f)(short int olt_id, short int llid);
typedef int (*ctss_olt_rdn_get_Rdnllid_mode_f)(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t* mode);
typedef int (*ctss_olt_rdn_set_Rdnllid_mode_f)(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t mode);
typedef int (*ctss_olt_rdn_set_Rdnllid_stdby_act_f)(short int olt_id, short int llid_n, short int* llid_list_marker);
typedef int (*ctss_olt_rdn_set_Rdnllid_rtt_f)(short int olt_id, short int llid, PON_rtt_t rtt);
/*End:for onu swap by jinhl@2013-02-22*/

typedef int (* ctss_olt_rdn_is_ready_f)(short int olt_id, int *iIsReady);
typedef int (* ctss_olt_get_llid_rdn_reginfo_f)(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo);
#endif

#if 1
/* --------------------OLT CMC协议管理API------------------- */
typedef int (* ctss_olt_dump_all_cmc_f)(short int olt_id, char *dump_buf, unsigned short *dump_len);
typedef int (* ctss_olt_set_all_cmc_svid_f)(short int olt_id, int svlan);

#endif



/* 注意:顺序与数目必须与OLT_CMD_ID一一对应 */
typedef struct stOltMgmtIFs {
#if 1
/* -------------------OLT基本API------------------- */
    ctss_olt_is_exist_f                         IsExist;
    ctss_olt_get_chip_typeid_f                  GetChipTypeID;
    ctss_olt_get_chip_typename_f                GetChipTypeName;
    ctss_olt_reset_pon_f                        ResetPon;
    ctss_olt_remove_olt_f                       RemoveOlt;

    ctss_olt_copy_olt_f                         CopyOlt;
    ctss_olt_is_support_cmd_f                   CmdIsSupported;
    ctss_olt_set_debug_mode_f                   SetDebugMode;
    ctss_olt_set_init_params_f                  SetInitParams;    
    ctss_olt_set_system_params_f                SetSystemParams;    

    ctss_olt_set_pon_i2c_extinfo_f              SetPonI2CExtInfo;
    ctss_olt_get_pon_i2c_extinfo_f              GetPonI2CExtInfo;
    ctss_olt_set_card_i2c_info_f                SetCardI2CInfo;
    ctss_olt_get_card_i2c_info_f                GetCardI2CInfo;
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_olt_write_mdio_register_f              WriteMdioRegister;

	ctss_olt_read_mdio_register_f               ReadMdioRegister;
	ctss_olt_read_i2c_register_f                ReadI2CRegister;
	ctss_olt_gpio_access_f                      GpioAccess;
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_olt_gpio_read_f                        ReadGpio;
	ctss_olt_gpio_write_f                       WriteGpio;

	ctss_olt_send_chip_cli_f                    SendChipCli;
    ctss_olt_set_device_name_f                  SetDeviceName;
	ctss_olt_reset_pon_chip_f                   ResetPonChip;/*by jinhl*/
#endif
    
#if 1
/* -------------------OLT PON管理API--------------- */
    ctss_olt_get_version_f                      GetVersion;
    ctss_olt_get_dba_version_f                  GetDBAVersion;
    ctss_olt_chk_version_f                      ChkVersion;
    ctss_olt_chk_dba_version_f                  ChkDBAVersion;
    ctss_olt_get_cni_link_status_f              GetCniLinkStatus;    
    
    ctss_olt_get_pon_workstatus_f               GetPonWorkStatus;
    ctss_olt_set_admin_status_f                 SetAdminStatus;
    ctss_olt_get_admin_status_f                 GetAdminStatus;
    ctss_olt_set_vlan_tpid_f                    SetVlanTpid;
    ctss_olt_set_vlan_qinq_f                    SetVlanQinQ;

    ctss_olt_set_pon_frame_sizelimit_f          SetPonFrameSizeLimit;
    ctss_olt_get_pon_frame_sizelimit_f          GetPonFrameSizeLimit;
    ctss_olt_oam_is_limit_f                     OamIsLimit;
    ctss_olt_update_pon_params_f                UpdatePonParams;
    ctss_olt_set_pppoerelay_mode_f              SetPPPoERelayMode;    

    ctss_olt_set_pppoerelay_params_f            SetPPPoERelayParams;    
    ctss_olt_set_dhcprelay_mode_f               SetDhcpRelayMode;    
    ctss_olt_set_igmp_auth_mode_f               SetIgmpAuthMode;
    ctss_olt_send_frame_2_pon_f                 SendFrame2PON;
    ctss_olt_send_frame_2_cni_f                 SendFrame2CNI;

	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_olt_get_vid_downlink_mode_f            GetVidDownlinkMode; 
	ctss_olt_delete_vid_downlink_mode_f         DelVidDownlinkMode;
	ctss_olt_get_olt_parameters_f               GetOltParameters;
    ctss_olt_set_olt_igmp_snooping_mode_f       SetOltIgmpSnoopingMode;
    ctss_olt_get_olt_igmp_snooping_mode_f       GetOltIgmpSnoopingMode;

    ctss_olt_set_olt_mld_forwarding_mode_f      SetOltMldForwardingMode;
    ctss_olt_get_olt_mld_forwarding_mode_f      GetOltMldForwardingMode;
    ctss_olt_set_dba_report_format_f            SetDBAReportFormat;
    ctss_olt_get_dba_report_format_f            GetDBAReportFormat;
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	/*Begin:for onu swap by jinhl@2013-04-27*/
    ctss_olt_update_prov_bwinfo_f               UpdateProvBWInfo;
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    ctss_olt_llid_is_exist_f                    LLIDIsExist;
    ctss_olt_deregister_llid_f                  DeregisterLLID;
    ctss_olt_get_llid_mac_f                     GetLLIDMac;
    ctss_olt_get_llid_reginfo_f                 GetLLIDRegisterInfo;
    ctss_olt_auth_llid_f                        AuthorizeLLID;

    ctss_olt_set_llid_sla_f                     SetLLIDSLA;
    ctss_olt_get_llid_sla_f                     GetLLIDSLA;
    ctss_olt_set_llid_police_f                  SetLLIDPolice;
    ctss_olt_get_llid_police_f                  GetLLIDPolice;
    ctss_olt_set_llid_dba_type_f                SetLLIDdbaType;
    
    ctss_olt_get_llid_dba_type_f                GetLLIDdbaType;
    ctss_olt_set_llid_dba_flags_f               SetLLIDdbaFlags;
    ctss_olt_get_llid_dba_flags_f               GetLLIDdbaFlags;
    ctss_olt_get_llid_heart_oam_f               GetLLIDHeartbeatOam;
    ctss_olt_set_llid_heart_oam_f               SetLLIDHeartbeatOam;
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    ctss_olt_get_all_onunum_f                   GetOnuNum;
    ctss_olt_get_all_onu_f                      GetAllOnus;
    ctss_olt_clear_all_onu_f                    ClearAllOnus;
    ctss_olt_resume_all_onu_status_f            ResumeAllOnuStatus;
    ctss_olt_set_onu_auth_mode_f                SetAllOnuAuthMode;   

    ctss_olt_set_onu_auth_mode_f                SetOnuAuthMode;
    ctss_olt_set_onu_mac_auth_f                 SetMacAuth;
    ctss_olt_set_onu_bind_mode_f                SetAllOnuBindMode;   
    ctss_olt_chk_onu_reg_ctrl_f	                ChkOnuRegisterControl;
    ctss_olt_set_onu_default_bw_f               SetAllOnuDefaultBW; 
    
    ctss_olt_set_onu_downlink_police_mode_f     SetAllOnuDownlinkPoliceMode;
    ctss_olt_set_onu_downlink_police_mode_f     SetOnuDownlinkPoliceMode;
    ctss_olt_set_onu_downlink_police_param_f    SetAllOnuDownlinkPoliceParam;
    ctss_olt_set_onu_uplink_dba_param_f         SetAllOnuUplinkDBAParam;
    ctss_olt_set_onu_downlink_pri2cos_f         SetOnuDownlinkPri2CoSQueueMap;
    
    ctss_olt_active_pending_onus_f              ActivePendingOnu;
    ctss_olt_active_pending_onu_f               ActiveOnePendingOnu;
    ctss_olt_active_conflict_pending_onus_f     ActiveConfPendingOnu;
    ctss_olt_active_conflict_pending_onu_f      ActiveOneConfPendingOnu;
    ctss_olt_get_pending_onu_f                  GetPendingOnuList;
    
    ctss_olt_get_update_onu_f                   GetUpdatingOnuList;
    ctss_olt_get_update_onu_f	                GetUpdatedOnuList;
    ctss_olt_get_updating_status_f              GetUpdatingOnuStatus;
    ctss_olt_set_update_onu_msg_f	            SetUpdateOnuMsg;
    ctss_olt_get_update_waiting_f               GetUpdateWaiting;

    ctss_olt_set_onu_auth_mode_f                SetAllOnuAuthMode2;
    ctss_olt_set_onu_bw_params_f                SetAllOnuBWParams;
    ctss_olt_set_p2p_mode_f                     SetOnuP2PMode;
    ctss_olt_get_p2p_mode_f                     GetOnuB2PMode;
    ctss_olt_set_p2p_mode_f                     SetOnuB2PMode;
	
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_olt_get_onu_mode_f                     GetOnuMode;
	ctss_olt_get_mac_address_authentication_f   GetMACAddressAuthentication;
	ctss_olt_set_authorize_mac_according_list_f SetAuthorizeMacAddressAccordingListMode;
	ctss_olt_get_authorize_mac_according_list_f GetAuthorizeMacAddressAccordingListMode;
	ctss_olt_get_downlink_buffer_config_f       GetDownlinkBufferConfiguration;

	ctss_olt_get_oam_information_f              GetOamInformation;
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	/*Begin:for onu swap by jinhl@2013-02-22*/
    ctss_olt_resume_llid_status_f               ResumeLLIDStatus;
    ctss_olt_search_free_onuIdx_f               SearchFreeOnuIdx;
    ctss_olt_get_act_onuIdx_byMac_f             GetActOnuIdxByMac;
    /*End:for onu swap by jinhl@2013-02-22*/
    ctss_olt_broadcast_clicommand_f             BroadCastCliCommand;    

    ctss_olt_set_auth_entry_f                   SetAuthEntry;
    ctss_olt_set_gpon_auth_entry_f              SetGponOnuAuthEntry;
    ctss_olt_set_onu_default_max_mac_f          SetOnuDefaultMaxMac;    

	ctss_olt_set_onu_port_stats_timeout_f     	SetOnuPortStatsTimeOut;
    ctss_olt_set_max_onu_f                      SetMaxOnu;
    ctss_olt_get_onu_conf_del_status_f          GetOnuConfDelStatus;
	ctss_olt_set_onu_TxPowerSupplyControl_f     SetOnuTxPowerSupplyControl;

#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    ctss_olt_set_encrypt_mode_f                 SetEncryptMode;
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ctss_olt_set_encryption_preamble_mode_f     SetEncryptionPreambleMode;
    ctss_olt_get_encryption_preamble_mode_f     GetEncryptionPreambleMode;
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ctss_olt_get_llid_encrypt_mode_f            GetLLIDEncryptMode;
    ctss_olt_start_llid_encrypt_f               StartLLIDEncrypt;
    
    ctss_olt_finish_llid_encrypt_f              FinishLLIDEncrypt;
    ctss_olt_stop_llid_encrypt_f                StopLLIDEncrypt;
    ctss_olt_set_llid_encrypt_key_f             SetLLIDEncryptKey;
    ctss_olt_finish_llid_encrypt_key_f          FinishLLIDEncryptKey;
#endif
    
#if 1
/* -------------------OLT 地址表管理API-------- */
    ctss_olt_set_mac_agingtime_f                SetMacAgingTime;
    ctss_olt_set_addrtbl_config_f               SetAddrTblCfg;
    ctss_olt_get_addrtbl_config_f               GetAddrTblCfg;
    ctss_olt_get_mac_addrtbl_f                  GetMacAddrTbl;
    ctss_olt_add_mac_addrtbl_f                  AddMacAddrTbl;

    ctss_olt_del_mac_addrtbl_f                  DelMacAddrTbl;
    ctss_olt_remove_mac_f                       RemoveMac;
    ctss_olt_reset_mac_addrtbl_f                ResetAddrTbl;
    ctss_olt_set_mac_threshold_f                SetOnuMacThreshold;
    ctss_olt_set_mac_check_enable_f             SetOnuMacCheckEnable;
    
    ctss_olt_set_mac_check_timer_f              SetOnuMacCheckPeriod;
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ctss_olt_set_addr_tab_full_handling_mode_f  SetAddressTableFullHandlingMode;
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    ctss_olt_get_llid_by_user_mac_address       GetLlidByUserMacAddress;
	ctss_olt_get_mac_addrVlantbl_f                  GetMacAddrVlanTbl;
#endif
    
#if 1
/* -------------------OLT 光路管理API----------- */
    ctss_olt_get_optical_capability_f           GetOpticalCapability;
    ctss_olt_get_optics_detail_f                GetOpticsDetail;
    ctss_olt_set_pon_range_f                    SetPonRange;
    ctss_olt_set_optical_tx_mode_f              SetOpticalTxMode;
    ctss_olt_get_optical_tx_mode_f              GetOpticalTxMode;
	
	ctss_olt_set_virtual_scope_adc_config_f     SetVirtualScopeAdcConfig;
    ctss_olt_get_virtual_scope_measurement_f    GetVirtualScopeMeasurement;
    ctss_olt_get_virtual_scope_rssi_measurement_f GetVirtualScopeRssiMeasurement;
	ctss_olt_get_virtual_scope_onuvoltage_f     GetVirtualScopeOnuVoltage;
	ctss_olt_set_virtual_llid_f                 SetVirtualLLID;
    
    ctss_olt_set_optical_tx_mode2_f             SetOpticalTxMode2;
#endif
    
#if 1
/* -------------------OLT 监控统计管理API---- */
    ctss_olt_get_raw_statistics_f               GetRawStatistics;
    ctss_olt_reset_counters_f                   ResetCounters;
    ctss_olt_set_ber_alarm_config_f             SetBerAlarm;
    ctss_olt_set_fer_alarm_config_f             SetFerAlarm;
    ctss_olt_set_ber_alarm_config_f             SetPonBerAlarm;

    ctss_olt_set_fer_alarm_config_f             SetPonFerAlarm;
    ctss_olt_set_ber_alarm_params_f             SetBerAlarmParams;    
    ctss_olt_set_fer_alarm_params_f             SetFerAlarmParams; 
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ctss_olt_set_alarm_config_f                 SetAlarmConfig; 
	ctss_olt_get_alarm_config_f                 GetAlarmConfig;
	
	ctss_olt_get_statistics_f                   GetStatistics;
	ctss_olt_olt_self_test_f                    OltSelfTest;
	ctss_olt_link_test_f                        LinkTest;
    ctss_olt_set_llid_fec_mode_f                SetLLIDFecMode;
    ctss_olt_get_llid_fec_mode_f                GetLLIDFecMode;

	ctss_olt_sys_dump_f                         SysDump;
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------OLT 倒换API---------------- */
    ctss_olt_get_hotswap_cap_f                  GetHotSwapCapability;
    ctss_olt_get_hotswap_mode_f                 GetHotSwapMode;
    ctss_olt_set_hotswap_mode_f                 SetHotSwapMode;
    ctss_olt_force_hotswap_f                    ForceHotSwap;
    ctss_olt_set_hotswap_param_f                SetHotSwapParam;
    
    ctss_olt_rdn_onu_register_f                 RdnOnuRegister;
    ctss_olt_set_rdn_config_f                   SetRdnConfig;
    ctss_olt_set_rdn_switch_f                   RdnSwitchOver;
    ctss_olt_rdn_is_exist_f                     RdnIsExist;
    ctss_olt_reset_rdn_record_f                 ResetRdnRecord;

    ctss_olt_get_rdn_state_f                    GetRdnState;
    ctss_olt_set_rdn_state_f                    SetRdnState;
    ctss_olt_get_rdn_addrtbl_f                  GetRdnAddrTbl;
    ctss_olt_remove_rdn_olt_f                   RemoveRdnOlt;
    ctss_olt_get_llid_rdn_db_f                  GetLLIDRdnDB;

    ctss_olt_set_llid_rdn_db_f                  SetLLIDRdnDB;
    ctss_olt_remove_remote_olt_f                RdnRemoveOlt;
    ctss_olt_swap_remote_olt_f                  RdnSwapOlt;
    ctss_olt_add_remote_olt_f                   AddSwapOlt;
    ctss_olt_loose_remote_olt_f                 RdnLooseOlt;

	/*Begin:for onu swap by jinhl@2013-02-22*/
    ctss_olt_rdn_llid_add_f                     RdnLLIDAdd;
    ctss_olt_rdn_get_Rdnllid_mode_f             GetRdnLLIDMode;
    ctss_olt_rdn_set_Rdnllid_mode_f             SetRdnLLIDMode;
	ctss_olt_rdn_set_Rdnllid_stdby_act_f        SetRdnLLIDStdbyToAct;
	ctss_olt_rdn_set_Rdnllid_rtt_f              SetRdnLLIDRtt;
    /*End:for onu swap by jinhl@2013-02-22*/
    
    ctss_olt_rdn_is_ready_f                     RdnIsReady;
    ctss_olt_get_llid_rdn_reginfo_f             GetLLIDRdnRegisterInfo;
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC协议管理API------------------- */
    ctss_olt_dump_all_cmc_f                     DumpAllCmc;
    ctss_olt_set_all_cmc_svid_f                 SetCmcServiceVID;
#endif

    ctss_olt_reset_pon_f                        LastFun;
}OltMgmtIFs;
/* E--added by liwei056@2010-5-11 for OLT-API-IF */

typedef struct PonMgmtInfo {   
	OltMgmtIFs  *OltIFs;    /* OLT的操作接口 */
	unsigned int type;

	IF_Index_S  IfIndex;   /* PON 端口索引 */
	/* PON端口管理状态 enable(1)/disable(2)*/
	unsigned int   PortAdminStatus; 
	/* PON端口工作状态
                      0 ------工作（WORKING ）
                      1 ------ 不在线（ OFF_LINE ）
                      2 ------ 复位（RESETING ）
                      3 ------ 测试 （TESTING ）
                      4 ------ 故障（ FAULT ）  
                      5 ------ 更新frimware */
 	unsigned int  PortWorkingStatus; 

	/* added by chenfj 2007/4/28 用于标识PON 端口是否支持CTC ONU, 这个信息来自于I2C */
	unsigned char CTC_Supported;
	unsigned char AutoRegisterFlag;
	unsigned char SignalLossFlag;
	unsigned char TxCtrlFlag;    /* added by liwei056@2014-4-24 for Tx's Control */
	
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
	
	unsigned long  AlarmStatus/*[MAX_PON_ALARM_NUM/8]*/;   /* 告警状态*/
	unsigned long  AlarmMask/*[MAX_PON_ALARM_NUM/8]*/;     /*告警屏蔽*/

	PON_olt_monitoring_configuration_t  AlarmConfigInfo;

	unsigned int MACAgeingTime;     /*PON端口动态MAC老化时间 */
 	unsigned char MacSelfLearningCtrl;
	unsigned char discard_unlearned_sa ;
	unsigned char discard_unlearned_da;
	unsigned char table_full_handle_mode;

	unsigned short int MaxOnu;		     /* 支持最大ONU数 */
	unsigned short int CurrOnu;		     /* 当前注册ONU数 */
	unsigned char  OnuCurStatus[8];         /*当前ONU在线状态 */

	unsigned short int  MaxLLID;           /* PON端口最大支持的LLID数 */
	unsigned short int  CurLLID;           /* PON端口当前已使用的LLID数 */
	unsigned short int  MaxMacNum;
	/* unsigned short int  MaxMacPerLlid[MAXLLID];  */
		
	unsigned int  MaxRTT; 
	unsigned int  CurrRTT;

	unsigned int  EncryptType;	      /* 加密类型 */   
	unsigned int  EncryptEnable;	  /* 加密使能 */
	/* moved  to onu management table */
	unsigned int  EncryptDirection ;  /* 加密方向*/
	unsigned int  EncryptKeyTime;     /* 密钥交互时间*/

	/* added by chenfj 2006/12/12
	     PON MAX RANGE */
	int range;
	
#if 0
	double   BERThreshold;
	double   FERThreshold;
#endif
	unsigned int   PortSpeed;   /* PON 端口当前流量,以bytes/s为单位 */
	unsigned int   PortBER;      /* PON端口当前误码率，以Bits/s为单位 */
	unsigned int   PortPER;      /* PON端口当前误码率，以packets/s为单位 */
	unsigned char  BerFlag;
	unsigned char  FerFlag;

	MacTable_S *InvalidOnuTablePtr;
	int  InvalidOnuNum;

	unsigned int PendingOnuCounter ;
	pendingOnu_S PendingOnu; /* 由于条件限制而未能注册的ONU */
	pendingOnu_S PendingOnu_Conf; /* 由于冲突而未能注册的ONU*/
	
	unsigned int  DBA_mode;
	int  external_DBA_used;	
	
	int testCounter;
	int testTimer;
	int testRemain;

	int  OnuRegisterMsgCounter;
	int  OnuDeregisterMsgCounter;
	int  OnuAuthMode;
	Date_S LastOnuRegisterTime;
	char LastRegisterMacAddr[BYTES_IN_MAC_ADDRESS];	

	unsigned int  TooManyOnuRegisterLastAlarmTime;
	unsigned int  LastBerAlarmTime;
	unsigned int  LastFerAlarmTime;

	unsigned char  CTC_EncryptKeyUpdateTime;
	unsigned char  CTC_EncryptTimeingThreshold;

	unsigned short int  CTC_EncryptKeyNoReplyTime;

	/* added by chenfj 2007-6-19  增加PON 端口保护切换*/
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
	unsigned char  swap_flag;
	unsigned char  swap_use;
	unsigned char  swapping;
/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
	unsigned char  swap_mode;	
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
	unsigned char  swap_slot;
	unsigned char  swap_port;
/* B--added by liwei056@2012-3-8 for Uplink's PonMonitorSwitch */
	unsigned char  protect_slot;
	unsigned char  protect_port;
/* E--added by liwei056@2012-3-8 for Uplink's PonMonitorSwitch */
	unsigned char  swap_timer;
	unsigned char  swapHappened;
/* B--added by liwei056@2010-4-7 for D9997 */
	unsigned char  swap_reason;	
/* E--added by liwei056@2010-4-7 for D9997 */
/* B--added by liwei056@2010-1-25 for Pon-BackupMonitor */
	unsigned char  swap_monitor;
/* E--added by liwei056@2010-1-25 for Pon-BackupMonitor */
/* B--added by liwei056@2011-12-22 for RdnSwitchStat */
	unsigned short swap_times;
/* E--added by liwei056@2011-12-22 for RdnSwitchStat */
/* B--added by liwei056@2010-12-2 for RdnReport */
	unsigned short swap_status;	
/* E--added by liwei056@2010-12-2 for RdnReport */
/* B--added by liwei056@2012-2-28 for Pon-SwicthTrigger */
	unsigned int   swap_triggers;	
/* E--added by liwei056@2012-2-28 for Pon-SwicthTrigger */
	unsigned int   swap_interval; //added by wangjiah@2017-05-15 to avoid hotswap frequently
#else
#error "EPON_MODULE_PON_PORT_HOT_SWAP is not opened!"    
#endif

/*begin: added by wangxiaoyu 2008-7-23 17:15:23
每个PON端口添加了如下变量用于增加对PON端口光功率的监视功能*/
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	PonPortmeteringInfo_S PonPortmeteringInfo;
#else
#error "EPON_MODULE_PON_OPTICAL_POWER is not opened!"    
#endif
#if 0
	short int powerMeteringSupport; /* PON 口是否支持光功率检测*/
	short int SFPTypeMismatchAlarm;  /* pon口光模块类型不匹配告警*/
	int recvOpticalPower[64];	/*接收光功率*/
	int transOpticalPower;	/*发送光功率, unit 0.1dbm */
	int ponTemperature;	/*模块温度, unit C */
	unsigned int ponVoltageApplied;	/*模块电压,unit 0.1V */
	unsigned int ponBiasCurrent;	/*偏置电流,unit 1mA */

	unsigned short int OpticalPowerAlarm;
	unsigned short int OpticalPowerAlarm_onu[64];
#endif
	/* 有个别时候，读光模块参数会出现偏差；当出现这种情况时，增加容错处理:
	     检查两次读的结果，若两次都是异常，则产生告警，或清楚告警*/
	/*unsigned short int  OpticalPowerAlarm_gen;
	unsigned short int  OpticalPowerAlarm_clr;
	
	unsigned short int OpticalPowerAlarm_gen_onu[64];
	unsigned short int OpticalPowerAlarm_clr_onu[64];*/
	
#if 0    /* 控制功能属于全局变量，对每个PON口都一样*/
	long ponAlwaysOnEnable;	/*允许PON端口突发模式监视*/
	long ponSignalMonEnable;	/*光功率监视使能*/
	long ponSignalMonInterval;	/*监视周期*/
#endif
/*end*/	
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	Olt_llid_vlan_manipulation Uplink_vlan_manipulation[MAXONUPERPONNOLIMIT];
	Olt_llid_vlan_manipulation Downlink_vlan_manipulation[MAX_VID_DOWNLINK];
	unsigned char  Downlink_vlan_Config[4096];
	unsigned short vlan_tpid;
	unsigned short Downlink_vlan_Cfg_Max;
	unsigned char  Downlink_vlan_Man_Max;
#else
#error "EPON_MODULE_PON_LLID_VLAN is not opened!"    
#endif

	unsigned char firmwareMismatchAlarm;
	unsigned char DBAMismatchAlarm;
	/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
	unsigned char PortFullAlarm;
	unsigned char AbnormalFlag;    /* added by liwei056@2011-11-25 for D13166 */

	unsigned char ApsStatus;		/* 保护倒换状态 */
	unsigned char ApsCtrl;			/* 保护倒换控制 */

#if 0
    unsigned char StaticMacAddr[MAXOLTADDRNUM][OLT_ADDR_BYTELEN];
    int AddrTblNum;
#endif
}PonPortMgmtInfo_S;


typedef struct PonLLID_Attr {
	
	IF_Index_S  LLIDIfIndex; /* LLID Index */
	unsigned char LLID_id;                       /* LLID allocated */
     
	unsigned int  LLIDEntryStatus; /* LLID Current status */
	unsigned int  LLIDUplinkBandwidth;  /* LLID bandwidth, unit: 256kbit/s*/
	unsigned int  LLIDDownlinkBandwidth ; 
   
	IF_Index_S  ONUIndex;      /* LLID对应的ONU索引*/
	unsigned char  ONUMAC[BYTES_IN_MAC_ADDRESS];                  /* LLID对应的ONU MAC*/
	/* LLID对应的ONU所能挂接的最大MAC数*/
	unsigned char  LLIDMaxMAC;	
	/* LLID动态MAC学习使能
	unsigned char  LLIDAutoLearningEnable ; 	 */
	unsigned short LLIDSpeed; /* 统计的LLID当前流量, 以bytes/为单位 */
	
} PonLLIDAttr_S;

typedef struct PonUpdateInfo{
	short int PonPortIdx;
	short int PonChipType;
}PonUpdateInfo_S;

typedef  struct  file_Info{
	void * location;
	int size;
}file_Info_S;

#if 0
extern PonLLIDAttr_S  PonLLIDTable[SYS_MAX_PON_PORTNUM][MAXLLID];
extern PonPortMgmtInfo_S  PonPortTable[];
#else
/* extern PonLLIDAttr_S    (*PonLLIDTable)[MAXLLID]; */
extern PonPortMgmtInfo_S *PonPortTable;
#endif
extern PonConfigInfo_S  PonPortConfigDefault;
extern unsigned long  PonLLIDDataSemId ;
extern unsigned long  PonPortDataSemId;
extern PON_olt_monitoring_configuration_t PonDefaultAlarmConfiguration;
/*
extern file_Info_S Pon_Firmware_image[];
extern file_Info_S Pon_DBA_image[];
*/
extern unsigned char *PonPortOperStatus_s[];
extern unsigned char *PonPortAdminStatus_s[];
extern unsigned char *PonPortApsCtrl_s[] ;
extern unsigned char *PonLinkDirection_s[];
extern unsigned char *PonQueuePriority_s[];

extern unsigned char *StartEncryptRetCode[];
extern unsigned char *StopEncryptRetCode[] ;
extern unsigned char *UpdateEncryptKeyRetCode[];
extern unsigned char *OAMVersion_s[];

#ifdef PONPORT_GENERAL
#endif
extern int  GetPonLlidEntryStatus( short int PonPortIdx,short int Llid );
#if 0
extern int  SetPonLlidBW( short int PonPortIdx, short int  Llid, unsigned int UplinkBw, unsigned int DownLinkBw);
extern int  GetPonLlidBW( short int PonPortIdx, short int  Llid, unsigned int UplinkBw, unsigned int DownLinkBw);
#endif
/*
extern int  SetPonLlidMaxMac( short int PonPortIdx, short int LLIDMaxMAC );
extern char  GetPonLlidMaxMac( short int PonPortIdx );
extern int  SetPonLlidMacAutoLearningCtrl( short int PonPortIdx, short int Llid,  unsigned int CtrlValue );
extern int  GetPonLlidMacAutoLearningCtrl( short int PonPortIdx, short int Llid );
*/
extern int  SetPonPortMacAutoLearningCtrl( short int PonPortIdx,  unsigned int CtrlValue );
extern int  GetPonPortMacAutoLearningCtrl( short int PonPortIdx,  unsigned int *CtrlValue );
extern int  GetPonPortMacAutoLearningCtrDefaultl( unsigned int *CtrlValue );
extern int  SetPonPortMacAgeingTime( short int PonPortIdx,  unsigned int AgeingTime );
extern int  GetPonPortMacAgeingTime( short int PonPortIdx,  unsigned int *AgeingTime );
extern int  GetPonPortMacAgeingTimeDefault( unsigned int *AgeingTime );
/*
extern int  GetPonLlidCurrentSpeed ( short int PonPortIdx, short int  Llid );
*/
extern int  InitPonLLIDTable();
extern int  InitOnePonLLIDTable( short int PonPortIdx );

extern int  ClearPonPortTable( short int PonPortIdx );
extern int  InitPonPortConfigDefault();
extern int  InitPonPortTableByDefault( short int PonPortIdx );
extern int  InitPonPortTableByFlash( short int PonPortIdx );

extern int  PonPortIsWorking( short int PonPortIdx );

extern int  ClearPonPortRunningData( short int PonPortIdx );
extern int  DecreasePonPortRunningData( short int PonPortIdx, short int OnuIdx );

extern int  GetPonPortType(short int PonPortIdx );
extern int  SetPonPortType( short int PonPortIdx, unsigned int type );
extern int  SetPonPortAdminStatus( short int PonPortIdx, unsigned int AdminStatus);
extern int  GetPonPortAdminStatus( short int PonPortIdx);
extern int  SetPonPortOperStatus( short int PonPortIdx, unsigned int OperStatus);
extern int  GetPonPortAdminStatusDefault( short int PonPortIdx);
extern int  ShutdownPonPort( short int PonPortIdx );
extern int  StartupPonPort( short int PonPortIdx );
extern int  GetPonPortOperStatus( short int PonPortIdx);
extern int  GetPonPortCurrentOnuNum( short int PonPortIdx);
extern int  SetPonPortMaxOnuNum( short int PonPortIdx, unsigned char MaxOnuNum );
extern int  GetPonPortMaxOnuNum( short int PonPortIdx); 
extern int  OnLineOnuCounter(short int PonPortIdx );
extern int  AllOnuCounter(short int PonPortIdx);
extern int  AllOnuCounterByMac( short int PonPortIdx, char *mac_address);
extern int  SoftwareUpdateEnableCounter( short int PonPortIdx );
extern int  SoftwareUpdateDisableCounter( short int PonPortIdx );

#if 0
extern int  GetPonPortCurrentOnuStatus( short int PonPortIdx, unsigned char *CurrentOnuStatus );

/* 还未实现
extern int  SetPonPortOnuDefaultBW( short int PonPortIdx, unsigned int DefaultBW );
extern int  GetPonPortOnuDefaultBW( short int PonPortIdx );
*/
extern int  SetPonPortBWMode( short int PonPortIdx, int  dba_mode );

extern int  GetPonPortActBw(short int PonPortIdx);
#endif
extern int  GetPonPortBWMode ( short int PonPortIdx, int *dba_mode );
extern int  GetPonPortBWModeDefault( int *dba_mode );
extern int  GetPonPortMaxBw( short int PonPortIdx );
extern int  GetPonPortRemainBW( short int PonPortIdx );
extern int  GetPonPortProvisionedBw(short int PonPortIdx);
extern int  UpdateProvisionedBWInfo( short int PonPortIdx );
extern int  ActivePonPort( short int PonPortIdx );

/*
int  SetPonPortAlarmMask( short int PonPortIdx, unsigned int AlarmId, unsigned char MaskFlag );
int  GetPonPortAlarmMask( short int PonPortIdx, unsigned int AlarmId );
*/
extern int GetPonPortCurrentAlarmStatus( short int PonPortIdx , unsigned long *AlarmNum, unsigned long *CurAlarmStatus );
#if 0
extern  int  GetPonPortCurrentAlarmRank( short int PonPortIdx );
extern int GetOltPonPortAlarmMask( short int PonPortIdx , unsigned long *pMask );
extern int SetOltPonPortAlarmMask( short int PonPortIdx , unsigned long mask );

/*extern int  SetPonPortMacAgingTime( short int PonPortIdx, unsigned int AgeingTime );
extern int  GetPonPortMacAgingTime( short int PonPortIdx );*/
extern int  GetPonPortMaxLLIDNum( short int PonPortIdx );
extern short int  GetPonPortCurrentLLIDUsed ( short int PonPortIdx );

extern int  SetPonPortMaxRTT( short int PonPortIdx, unsigned int MaxRTT  );
extern int  GetPonPortMaxRTT( short int PonPortIdx );

extern int  SetPonPortApsCtrl( short int PonPortIdx, unsigned int CtrlValue );
extern int  GetPonPortApsCtrl( short int PonPortIdx );
extern int  GetPonPortApsCtrlDefault();
extern int  GetPonPortApsStatus( short int PonPortIdx );
/* extern int ChangePonPortApsStatus(short int PonPortIdx, int status ); */

extern int  SetPonPortBerThreshold( short int PonPortIdx, long double Threshold );
extern long double  GetPonPortBerThreshold( short int PonPortIdx );
extern int  SetPonPortFerThreshold( short int PonPortIdx, long  double Threshold );
extern long double  GetPonPortFerThreshold( short int PonPortIdx );
#endif

#if 0
extern int  SetPonPortEncrypt( short int PonPortIdx, /*unsigned int CtrlValue,*/unsigned int  cryptionDirectioin );
extern int  GetPonPortEncrypt ( short int PonPortIdx , unsigned int *cryptionDirection );
extern int  SetPonPortEncryptKeyTime( short int PonPortIdx, unsigned int Timelen);
extern int  GetPonPortEncryptKeyTime( short int PonPortIdx, unsigned int *Time );
#endif
extern int  GetPonPortEncryptDefault ( unsigned int *cryptionDirection);
extern int  GetPonPortEncryptKeyTimeDefault( unsigned int *TimeLen );

/* 还未实现
extern int  GetPonPortCurrentSpeed( short int PonPortIdx );
extern int  GetPonPortCurrentBer( short int PonPortIdx );
extern int  GetPonPortCurrentFer( short int PonPortIdx );
*/
extern int  CheckIsInvalidOnu( short int PonPortIdx, unsigned char *OnuMac );
extern int  SetPonPortInvalidOnu( short int PonPortIdx,  unsigned char *OnuMac ,unsigned int flag);
extern int  GetPonPortInvalidOnuList( short int PonPortIdx, MacTable_S *InvalidOnuList );

extern int  AddPendingOnu( short int PonPortIdx, short int OnuIdx, short int Llid, unsigned char  *macAddress, PendingReason_S code);
/*extern int  DelPendingOnu( short int PonPortIdx, short int Llid );
extern int  DelAllPendingOnu( short int PonPortIdx );*/
extern int  ActivatePendingOnu( short int PonPortIdx );
extern int  ActivatePendingOnuMsg( short int PonPortIdx );

extern int  ActivateOnePendingOnu( short int PonPortIdx, unsigned char *MacAddr );

extern int  AddPendingOnu_conf( short int PonPortIdx, short int Llid, unsigned char  *macAddress, short int PonPortIdx_conf );
/*extern int  DelPendingOnu_conf( short int PonPortIdx, short int Llid );
extern int  PendingOnuList_conf( short int PonPortIdx );
extern int  DelAllPendingOnu_conf( short int PonPortIdx );*/
extern int  ActivatePendingOnu_conf( short int PonPortIdx, unsigned char *macAddress );
extern int  ActivatePendingOnu_conf_all( short int PonPortIdx );
extern int  ActivatePendingOnuMsg_conf( short int PonPortIdx, unsigned char *macAddress );
extern int  ActivatePendingOnuMsg_conf_all( short int PonPortIdx );

extern int UpdatePendingOnu( short int PonPortIdx, short int llid, char *onuMac );
extern int UpdatePendingConfOnu( short int PonPortIdx, short int llid, char *onuMac );
extern int UpdateAllPendingOnu( short int PonPortIdx );
extern int UpdateAllPendingConfOnu( short int PonPortIdx );

extern short int  GetPonPortIdx( short int PonChipIdx );
extern short int  GetPonChipIdx( short int PonPortIdx );
 

extern int  GetOnuMacAddrFromLlid( short int PonPortIdx, short int Llid , unsigned char *MacAddr );
extern int  GetOnuLlidListFromMacAddr( unsigned char *MacAddr, short int *PonPortIdx,  short int  *Llid/*, unsigned char Llid_Num */);

extern int  setUpdateParamToDufault( PON_olt_update_parameters_t updated_parameters );


extern int FindGlobalPonMacAddr( unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] );
extern int PonStaticMacTableIsFull( unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] );


#ifdef PON_INIT
#endif
extern int  pon_init(void);
extern int  Add_PonPort( short int PonPortIdx );
extern int  Del_PonPort( short int PonPortIdx );
extern int  pon_resume_olt(short int PonPortIdx);
extern int  pon_add_olt(const short int PonPortIdx );
extern int  pon_add_oltEx2(int SlotId, int PortId, int PonPortIdx, int PonChipType, int ReloadBase, int IsNeedReset, int IsNewThread);
extern int  pon_test_oltEx2(struct vty *vty, int SlotId, int PortId, int TestDelay, int TestTimes, int IsNewThread);
extern int  pon_terminate();
extern void  pon_cni_port_init(void);
extern void  get_cni_port_configuration(void);
extern void  pon_initialization_parameters_init(void);
extern int  GetPonOpticsParam( short int PonPortIdx );
extern void  pon_set_update_parameters();
extern short int  update_Pon_parameters_5001(short int PonPortIdx);
extern short int  update_Pon_parameters_5201(short int PonPortIdx,short int grant_filtering );
/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
extern short int update_Pon_parameters_8411(short int PonPortIdx, short int  grant_filtering );
extern void  set_image_src( short int PonChipIdx,  PON_binary_source_t  src, PON_binary_t *pInit, unsigned char version[100] );
extern int  pon_start_default_dba(const short int PonPortIdx );
extern int  pon_set_system_parameters(long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout);
extern int  Pon_Get_System_parameters(short int *statistics_sampling_cycle,short int *monitoring_cycle,short int *host_olt_msg_timeout, short int  *olt_reset_timeout);

/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern PON_STATUS GW_CTC_STACK_assign_handler_function( const CTC_STACK_handler_index_t handler_function_index, const void (*handler_function)(),
				  unsigned short *handler_id );
extern int  GW8411_Init_CTC_STACK(void );
extern int  GW5201_Init_CTC_STACK(void );
extern int GW8411_pon_init();
extern int GW5201_pon_init();
extern int GW5001_pon_add_olt(short int PonPortIdx, short int PonChipVer);
extern int GW8411_pon_add_olt(short int PonPortIdx, short int PonChipVer);
int Pon_InitDefaultMaxBW(short int PonPortIdx);
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#if 0
extern int  SetPonFetchIGMPMsgToHostPath( short int PonPortIdx );
extern int  SetPonFetchIGMPMsgToDataPath(short int PonPortIdx);
extern int  SetPonFetchIGMPMsgToAllPath(short int PonPortIdx);
extern int  DisablePonFetchIGMPMsg(short int PonPortIdx);
#endif

/*extern int  pon_add_olt1(const short int PonPortIdx);*/
extern int  pon_remove_olt(const short int PonPortIdx );
extern int  pon_reset_olt(short int PonPortIdx);
extern int  ClearPonRunningData(short int PonPortIdx);
#if 0
extern int  Hardware_Reset_olt( unsigned long ulSlot, unsigned long port);
#endif
extern int  Hardware_Reset_olt1( unsigned long ulSlot, unsigned long port, unsigned long wait_delay, unsigned long force);
extern int  Hardware_Reset_olt2( unsigned long ulSlot, unsigned long port, unsigned long wait_delay, unsigned long force);
extern int  ShutDown_olt( short int PonPortIdx );
extern int  StartUp_olt( short int PonPortIdx );
extern int  PONTx_Disable( short int PonPortIdx, int tx_reason );
extern int  PONTx_Enable( short int PonPortIdx, int tx_reason );
extern int  UpdatePonFirmware( short int PonPortIdx );
extern int  pon_assign_handler_5001();

extern int  pon_load_external_dba_file(short int PonPortIdx );

#ifdef PAS_CHIP_5201_
#endif
extern int  pon_assign_handler_5201();
extern int  redundancy_assign_handler();
extern int  pon_assign_handler_tk2pas5201();
extern int  pon_assign_handler_tk3723();
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern int pon_assign_handler_8411();
extern int GW5201_redundancy_assign_handler();
extern int GW8411_redundancy_assign_handler();

/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
extern short int SetPonCniPatameter(short int PonPortIdx );
extern short int GetPonCniPatameter(short int PonPortIdx );

extern short int  SetOltIgmpSnooping( short int PonPortIdx, short int enable_flag );
extern short int  GetOltIgmpSnootping(short int PonPortIdx);

#if 1
extern int CopyOltAddressTableConfig(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags);
extern int ResumeOltAddressTableConfig(short int PonPortIdx);
#else
extern short int  ConfigOltAddressTable(short int PonPortIdx /*, short int direction, short int limit_entry*/ );
#endif
extern int  GetOltAddressTable( short int PonPortIdx );

extern short int  ConfigOltARPFilter(short int PonPortIdx);
extern short int  GetOltARPFilter( short int PonPortIdx );
extern short int  ConfigOltDhcpFilter(short int PonPortIdx);
	
extern short int  ConfigOltVLAN( short int PonPortIdx );
extern short int GetOltVlanConfig_Uplink(short int PonPortIdx, short int OnuIdx  );
extern short int GetOltVlanConfig_Downlink(short int PonPortIdx, short int OnuIdx  );

extern int  SetPonEncryptMode( short int PonPortIdx, int  Mode );
extern int  GetPonEncryptMode( short int PonPortIdx, int  *Mode );
extern int  SetPonDBAReportMode( short int  PonPortIdx,  int Report_Mode );
extern int  GetPonDBAReportMode( short int  PonPortIdx,  int  *Report_Mode );
/*extern short int  SetONUDefaultConfigMode();
extern int  CTC_AddSupportedOUI( unsigned char *OUI, unsigned char version );
extern int  CTC_GetSupportedOUI();*/

extern int  CTC_SetPonEncryptTimeParam(unsigned char UpdateTime, unsigned short  int  NoReplyTime );
extern int  CTC_GetPonEncryptTimeParam(unsigned char *UpdateTime, unsigned short  int  *NoReplyTime );
extern int  CTC_SetPonEncryptTimingThreshold( int TimingThreshold );
extern int  CTC_GetPonEncryptTimingThreshold( int  *TimingThreshold );
extern int  CTC_SetPonEncryptKeyUpdateTime( int UpdateTime );
extern int  CTC_GetPonEncryptKeyUpdateTime( int *UpdateTime );
extern int  CTC_SetPonEncryptNoReplyTime( int  NoReplyTime );
extern int  CTC_GetPonEncryptNoReplyTime( int *NoReplyTime );

extern int  Init_PAS_CTC_STACK(void);
extern int  Init_TK_CTC_STACK(void);

#ifdef PAS_CHIP_5201_
#endif


extern int  AddInvalidOnuMac( short int PonPortIdx, unsigned char *OnuMac );
extern int  DelInvalidOnuMac( short int PonPortIdx, unsigned char *OnuMac );

extern int  SetCniTbc( short int PonPortIdx, unsigned long tbe);
extern unsigned int GetCniTbc( short int PonPortIdx );
extern int  SetPonPortTBC(  short int PonPortIdx, PON_polarity_t  tbc );
extern int  SetPonGpioPin(short int PonPortIdx, PON_gpio_lines_t gpio , PON_gpio_line_io_t gpio_dir, short int value );
extern int  V2r1PonWatchdog( short int PonPortIdx, short int value );
extern int  SetPonNotRunningFlag( short int PonPortIdx/*, PON_gpio_line_io_t direction, bool value*/);
extern int  SetPonRunningFlag( short int PonPortIdx/*,PON_gpio_line_io_t direction, bool value*/);

#ifdef PON_HANDLER
#endif

extern int  PonBERAlarmConfigurationIsDefault(short int PonPortIdx);
extern int  PonFERAlarmConfigurationIsDefault(short int PonPortIdx );
extern int  SetPonBerAlarm( short int PonPortIdx );
extern int  SetPonFerAlarm( short int PonPortIdx );
extern int  SetPASSoftwareErrorAlarm();
extern int  SetPonLocalLinkAlarm( short int PonPortIdx );
extern int  SetPonDyingGaspAlarm( short int PonPortIdx );
extern int  SetPonCriticalEventAlarm( short int PonPortIdx );
extern int  SetPonRemoteStableAlarm( short int PonPortIdx );
extern int  SetPonLocalStableAlarm( short int PonPortIdx );
extern int  SetPonOamVendorAlarm( short int PonPortIdx );
extern int  SetPonErrorSymbolAlarm( short int PonPortIdx );
extern int  SetPonErrorFrameAlarm( short int PonPortIdx );
extern int  SetPonErrorFramePeriodAlarm( short int PonPortIdx );
extern int  SetPonErrorFrameSecondsAlarm( short int PonPortIdx );
extern int  SetPonRegisterErrorAlarm( short int PonPortIdx );
extern int  SetPonOamLinkDisconnectAlarm( short int PonPortIdx );
extern int  SetPonBadEncryptKeyAlarm( short int PonPortIdx );
extern int  SetPonLlidMismatchAlarm( short int PonPortIdx );
extern int  SetPonTooManyOnuAlarm( short int PonPortIdx );
extern int  SetPonAlarmConfig( short int PonPortIdx );
extern int  InitPonAlarmConfigDefault();


extern int  ShowPonMacLearningCounterByVty( short int PonPortIdx, struct vty *vty );

extern  int  ShowPonPortStatusByVty( short int PonPortIdx, struct vty *vty );
extern  int  ShowPonPortStatusAllByVty(struct vty *vty);
extern  int  ShowPonPortBWInfoByVty( short int PonPortIdx, struct vty *vty );
extern  int  ShowPonPortEncryptInfoByVty (short int PonPortIdx, struct vty *vty );
extern  int  ShowPonPortOnuMacAddrByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty);
/*extern  int  ShowPonPortOnuMacAddrByVty_1( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty);*/
extern  int  ShowPonPortOnuMacAddrByVtyAll( unsigned char *OnuTypeString, struct vty *vty);
extern  int  ShowPonPortOnuMacAddrCounterByVty(short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty);
extern  int  ShowPonPortOnuMacAddrCounterByVtyAll(unsigned char *OnuTypeString, struct vty *vty);
	
extern  int  ShowPonPortOnLineOnuByVty( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );
/*extern int   ShowPonPortOnLineOnuByVty_1( short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );*/
extern  int  ShowPonPortOnLineOnuByVtyAll( unsigned char *OnuTypeString, struct vty *vty );
extern  int  ShowPonPortOnLineOnuCounterByVty(short int PonPortIdx, unsigned char *OnuTypeString, struct vty *vty );
extern  int  ShowPonPortOnLineOnuCounterByVtyAll(unsigned char *OnuTypeString,struct vty *vty );
extern  int  ShowPonMacLearningFromWhichOnuByVty( short int PonPortIdx, unsigned char *MacAddress, struct vty *vty );
extern  int  ShowPonMacLearningFromWhichOnuByVtyAll(unsigned char *MacAddress, struct vty *vty );

extern  int ShowPonPortInfoByVty(short int PonPortIdx , struct vty *vty);  
extern int  ShowPonPortOpticalParaByVty( short int PonPortIdx , struct vty *vty);
extern int  PendingOnuListByVty( short int PonPortIdx , short int * count, struct vty *vty);
extern int  ShowOnuRegisterCounterInfoByVty(short int PonPortIdx ,struct vty *vty);

extern int  ShowOnuSoftwareUpdateByVty( short int PonPortIdx, struct vty *vty);
extern int  ShowOnuSoftwareUpdateAllByVty(struct vty *vty);

extern int  UpdatePonPortCurrRTT(short int PonPortIdx, short int OnuIdx );

extern int  ScanPonPortMgmtTableTimer();
extern int  ScanPonPortMgmtTableBerFer();
extern int  RestartPonPort( short int PonPortIdx );

#ifdef ADD_ONU
#endif

extern int  AddOnuToPonPort(short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr );
extern int  DelOnuFromPonPort(short int PonPortIdx, short int OnuIdx);
extern int  SetOnuRegisterLimitFlag( short int PonPortIdx, int flag );
extern int  GetOnuRegisterLimitFlag(short int PonPortIdx, int *flag );
extern int  GetOnuRegisterLimitFlagDufault( short int PonPortIdx, int *DefaultFlag );

/*extern int  OnuRegisterHandler_EUQ( short int PonPortIdx, short int OnuIdx );*/

extern int  SetPonRange(short int PonPortIdx , int range);
extern int  GetPonRange( short int PonPortIdx, int *range );
extern int  GetPonRangeDefault( int *rangeDefault );
extern int	IsPonPortPairOnSameChip(short int PonPortIdx1, short int PonPortIdx2);

#include "PON_CTC_STACK_defines.h"
#include "PON_CTC_STACK_variable_descriptor_defines_expo.h"

typedef struct ExtOAMDiscovery{
	unsigned int   event_flags;
	PON_olt_id_t   olt_id;
	PON_onu_id_t   llid;
	CTC_STACK_discovery_state_t  result;
	unsigned char Number_of_records; 
	CTC_STACK_oui_version_record_t  onu_version_records_list[MAX_OUI_RECORDS];
}ExtOAMDiscovery_t;

extern int CTC_stack_initialize();
extern int pon_assign_pashandler_CTC_STACK();
extern int pon_assign_tkhandler_CTC_STACK();
extern short  int  Onu_ExtOAMDiscovery_Handler(PON_olt_id_t olt_id,  PON_llid_t llid, CTC_STACK_discovery_state_t result,  unsigned char Number_of_records,  CTC_STACK_oui_version_record_t *onu_version_records_list );
extern short  int  OnuExtOAMDiscoveryHandler(ExtOAMDiscovery_t  *ExtOAMDiscovery_Data);

#ifdef  FLASH_FILE
#endif
extern  int  GetPonChipFWImageInfo( short int PonChipType, int *location, int  *length, unsigned char version[100] );
extern  int  GetPonChipDBAFileInfo( short int PonChipType,  int *location, int  *length, unsigned char version[100] );

#define  OLT_COPYFLAGS_CONFIG       0x0000
#define  OLT_COPYFLAGS_WITHINFO     0x0001
#define  OLT_COPYFLAGS_WITHLLID     0x0002
#define  OLT_COPYFLAGS_GETSTATUS    0x0004
#define  OLT_COPYFLAGS_WITHSTATUS   0x0008

#define  OLT_COPYFLAGS_ONLYNEW      0x0100
#define  OLT_COPYFLAGS_CHECK        0x0200
#define  OLT_COPYFLAGS_COVER        0x0400

#define  OLT_COPYFLAGS_NORMAL       OLT_COPYFLAGS_CONFIG | OLT_COPYFLAGS_CHECK
#define  OLT_COPYFLAGS_RESUME       OLT_COPYFLAGS_CONFIG | OLT_COPYFLAGS_ONLYNEW
#define  OLT_COPYFLAGS_CMPSYNC      OLT_COPYFLAGS_CONFIG | OLT_COPYFLAGS_CHECK
#define  OLT_COPYFLAGS_COVERSYNC    OLT_COPYFLAGS_CONFIG | OLT_COPYFLAGS_COVER
#define  OLT_COPYFLAGS_SYNC         OLT_COPYFLAGS_RESUME

#define  OLT_COPYFLAGS_ALL          -1

extern int CopyOlt(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags);
extern int CopyOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags);
extern int CopyOneOnu(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags);

#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
extern  int   SetPonPortSwapEnable( short int PonPortIdx, unsigned int  SwapFlag );
extern  int   SetPonPortSwapStatus( short int PonPortIdx, unsigned  int SwapStatus );
extern  int   PonPortSwapEnableQuery( short int PonPortIdx );
extern  int   PonPortSwapPortLocalQuery( short int PonPortIdx1, short int *PonPortIdx_Swap );
extern  int   PonPortSwapPortQuery( short int PonPortIdx, short int *PonPortIdx_Swap  );
extern  int   PonPortSwapSlotQuery( short int PonPortIdx1, int *PonPortIdx2_Slot, int *PonPortIdx2_Port );
extern  int   PonPortHotStatusQuery( short int PonPortIdx );
extern  int   PonPortIsSwaping( short int PonPortIdx );
extern  int   PonPortIsNotSwaping( short int PonPortIdx );
extern  int   PonPortSwapTimesQuery( short int PonPortIdx );
extern  int   ThisIsHotSwapPort(short int PonPortIdx1, short int PonPortIdx2);
extern  int   GetPonPortHotSwapMode(short int PonPortIdx);
extern  int   GetPonPortHotSwapTriggers(short int PonPortIdx);
extern  int   GetPonPortHotProtectedPort(short int PonPortIdx, unsigned long *ulSlot, unsigned long *ulPort);
extern  int   PonPortProtectedPortLocalQuery( short int PonPortIdx1, short int *PonPortIdx_Protected );

extern	void  ProcessHotSwap(short int olt_id, short int partner_olt_id, int reason);
extern  int   ScanPonPortHotSwap();
extern  int   DisablePonPortHotSwap(short int PonPortIdx );
#if 0
extern  int   HotSwapPort1ToPort2( short int PonPortIdx1 , short int PonPortIdx2);
extern  int   PonPortSwapEnablePort1DisablePort2(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode ); 
#endif
extern  int   HotSwapPort( short int PonPortIdx );
extern  int   SyncSwapPort( short int PonPortIdx );
extern  int   ClearHotSwapActivePonRunningData(short int PonPortIdx);
extern  int   ClearHotSwapPassivePonRunningData(short int PonPortIdx);


extern int  EnablePonPortAutoProtect( unsigned int slot, unsigned int port, unsigned int PartnerSlot, unsigned int PartnerPort, int iProtectMode );
extern int  DisablePonPortAutoProtect( unsigned int slot, unsigned int port );
extern int  ForcePonPortAutoProtect( unsigned int slot, unsigned int port );
extern int  SyncPonPortAutoProtect( unsigned int slot, unsigned int port );
extern int  ShowPonPortAutoProtectTrigger(struct vty* vty, unsigned int slot, unsigned int port );

/* B--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
extern  int ActivePonFastHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx);
extern  int ActivePonGpioHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx);
extern  int InActivePonFastHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx);
extern  int HotSwapRedundancyPon(short int oldActivePonPortIdx, short int newActivePonPortIdx);
extern  int SetPonPortAutoProtectMode( unsigned int slot, unsigned int port, int swapMode );
extern  int ShowPonPortAutoProtectMode(struct vty* vty, unsigned int slot, unsigned int port );
#endif
#if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
extern  int ActivePonOnuHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx);
extern  int InActivePonOnuHotSwapSetup(short int ActivePonPortIdx, short int PassivePonPortIdx);
extern int ShowAllPonPortAutoProtect(struct vty* vty );
extern int ShowAllPonPortOnuProtect(struct vty* vty );
#endif

extern void BindOnuConfOnSmallPonPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode);//added by wangjiah@2017-03-08
extern int EnablePonPortHotSwapPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode);
extern int ActivePonHotSwapPort(short int PonPortIdx1, short int PonPortIdx2, int iProtectMode, int iSyncMode);
/* E--added by liwei056@2010-1-25 for Pon-FastSwicthHover & Pon-BackupMonitor */

extern int  PonPortAutoProtectStatusQuery(unsigned int slot, unsigned int port );
extern int  PonPortAutoProtectEnableQuery(unsigned int slot, unsigned int port );
extern int ThisIsAutoProtectPort(unsigned int slot, unsigned int port, unsigned int PartnerSlot, unsigned int PartnerPort );
extern int  PonPortAutoProtectPortQuery( unsigned int slot, unsigned int port, unsigned int *PartnerSlot, unsigned int *PartnerPort );
extern int  EnablePonPortAutoProtectByOnePort(short int PonPortIdx );
#if 0
extern  int   DisablePonPortHotSwap_bk(short int PonPortIdx1, short int PonPortIdx2 );
#endif
/*Begin:for onu swap by jinhl@2013-04-27*/
extern int ActivePonPort_RegVirtual(short int PonPortIdx);
/*End:for onu swap by jinhl@2013-04-27*/

extern int  ClearAllOnuDataByOnePon(short int PonPortIdx);
extern int  CopyPonConfigDataFromPon1ToPon2( short int PonPortIdx1, short int PonPortIdx2);
extern int  CopyAllOnuListFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2);
extern int  CopyAllOnuDeviceInfoFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2);
extern int  CopyAllOnuConfigDataFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2);
extern int  CopyAllOnuDataFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2 );
extern void CopyAllConfigFromPon1ToPon2(short int PonPortIdx1, short int PonPortIdx2, int CopyFlags);
extern void CopyOnuConfigFromPon1ToPon2(short int PonPortIdx1, short int OnuIdx1, short int PonPortIdx2, short int OnuIdx2, int CopyFlags);
extern int  HasOnuOnlineOrRegistering( short int PonPortIdx );
extern int  ScanAuthOnuCounter(short int PonPortIdx );

/*extern void ponSwitchCtcOnuCfgData( const short int srcPon, const short int desPon );*/
extern ULONG ctcOnuIgmpAuthInfoSwitch( const short int srcPonId, const short int desPonId );

#define _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
#ifdef _RECORD_ACTIVE_PON_FOR_AUTO_PROTECT
#define  AUTO_PROTECT_AND_ACTIVE   3
#define  AUTO_PROTECT_AND_PASSIVE  2
#define  NO_AUTO_PROTECT  0

extern int SetPonPortAutoProtectAcitve(short int PonPortIdx);
extern int SetPonPortAutoProtectPassive(short int PonPortIdx);
extern int GetPonPortAutoProtectStatus(short int PonPortIdx);
extern int ClearPonPortAutoPoetect(short int PonPortIdx);

#endif
#endif

extern int  PonPortAdminStatusEnable( short int PonPortIdx );
extern int  PonPortAdminStatusDisable( short int PonPortIdx );
extern int  ResetPonPort( short int PonPortIdx );

extern int  ShowPonPortAuthOnuEntry( short int PonPortIdx,  struct vty *vty );
extern int  ShowPonPortAuthOnuEntryALL( struct vty *vty );
extern int  AddMacAuthenticationAll( short int PonPortIdx );
extern int  DeletePonPortAuthEntryAll( short int PonPortIdx );
extern int  OnuAuth_ActivatePendingOnuByMacAddrMsg( short int PonPortIdx, unsigned char *MacAddr );
extern int  OnuAuth_ActivateOnePendingOnu( unsigned long slot, unsigned long port, unsigned char *MacAddr );
extern int  OnuAuth_DeregisterOnu(unsigned long slot, unsigned long port, unsigned char *MacAddr );
extern int  AddRegisterOnuToAuthEntry( short int PonPortIdx );
extern int  OnuAuth_DeregisterAllOnu( short int PonPortIdx );


extern int CLI_Print_oam_information (struct vty *vty, const PON_oam_information_t   *oam_information);
extern const char *error_string_lookup(short int error_code);
/* this command free the memory of statistics_data after printing */
extern short int Print_raw_statistics_buffers ( const short int			  olt_id, 
										 const short int			  collector_id,
										 const PON_raw_statistics_t   raw_statistics_type,
										 const short int			  statistics_parameter,
										       void				     *statistics_data,
										 const PON_timestamp_t		  timestamp,
										 struct  vty					 *vty );


#define CLI_ERROR_PRINT\
         vty_out( vty,"Command return error.%s%s (error code : %d)%s" \
		, VTY_NEWLINE, error_string_lookup(return_result), return_result, VTY_NEWLINE); 

/* added by chenfj 2007-9-5 
	增加PAS5201芯片在MAC地址满后，新进来的MAC地址的处理模式*/
extern int SetPonPortAddressTableFullHandlingMode(short int  PonPortIdx, unsigned char Handling_mode);
/* added by chenfj 2007-12-19
	增加用于设置PON 下行优先级与队列之间映射*/
extern int SetOnuDownlinkQosMapping(void);
extern int SetPas5201frame_size_limits(short int PonPortIdx, unsigned int jumbo_length);
extern int GetPas5201frame_size_limits(short int PonPortIdx, unsigned int *jumbo_length);

extern int AllOnuCounterByType( short int PonPortIdx, unsigned char *OnuTypeString );
extern int OnLineOnuCounterByType(short int PonPortIdx, unsigned char *OnuTypeString );

#if 0
/* 清除PON 端口MAC 地址表*/
extern int  ResetPonPortMacTable(short int PonPortIdx );
extern int  SetPonPortMACLearning(short int PonPortIdx, int LearningFlag );
#endif
extern int  PonAbnormalHandler(short int PonPortIdx, int ErrCode);

extern 	void  InitPonMsgDebugFlag(void);
extern 	void  ClearPonDownloadCounter(void);
extern 	void  ClearPonActivatedFlag(void);

extern int ScanAuthenticationPendingOnu(short int PonPortIdx);
extern int ScanAuthenticationPendingOnuAll();


#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )

#define  ONU_LASER_ALWAYS_ON_DEFAULT   3    /* 1.2V */
#define  ONU_LASER_DEGRADATION_DEFAULT  3    /* 0.3V  */
#define  ONU_EOL_THRESHOLD_DEFAULT  4          /* 4dBM */

extern int AdcConfig(short int PonPortIdx);
extern int ConfigOnuAlwaysOnAlarm( short int PonPortIdx, bool enable_flag );
extern int ConfigOnuDegradationAlarm( short int PonPortIdx, bool enable_flag );
/*extern int ReadOnuOpticalPower(short int PonPortIdx, short int OnuIdx, struct vty *vty);*/
/*extern int OltPonPowerMetering(void);*/
extern int Main_OltPonpowermetering(void);

#endif


#ifdef  PRODUCT_EPON3_GFA6100_TEST
extern int downloadPonByVty(short int PonPortIdx, struct vty *vty);
extern int RemovePonByVty(short int PonPortIdx, struct vty *vty);
#endif

typedef struct _XCVR_DATA_s{
	BYTE cAddr;
	UCHAR * pcName;
	BYTE cLen;
	BYTE cType;
}_XCVR_DATA_;

#define   A0H_1010000X    0
#define   A2H_1010001X    1

extern _XCVR_DATA_ XcvrDiagArr[] ;
extern _XCVR_DATA_ XcvrInfoArr[];

#define SFPOUITYPEBASEADDR  0x80
#define SFPOUITYPE_GWD  "GWD"		/*"GWDelight"*/	/* modified by xieshl 20091102 */
#define SFPOUITYPELEN  3				/*9*/
#define SFPOUITYPE_HIS  "Hisense"	/*"Hisense"*/  	/* added by liwei056 20140325 */
#define SFPOUITYPELEN_HIS  7		/*9*/

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
typedef struct SFP_temperature_data_S{
/*short int alarm_high;
short int alarm_low;
short int warning_high;
short int warning_low;*/
short int current_value; /* 范围: -128C ~ + 128C; 最高位为符号位，0 为正值，1 为负值。
							其它各比特代表温度值，LSB ＝1/256 C，依次类推*/
}__attribute__ ((packed)) SFP_temperature_data;

typedef struct SFP_voltage_data_S{
/*short int alarm_high;
short int alarm_low;
short int warning_high;
short int warning_low;*/
short int current_value; /* 光模块工作电压，范围: (0 ~ 65535), unit: 0.1mV */
}__attribute__ ((packed)) SFP_voltage_data;

typedef struct SFP_bias_data_S{
/*short int alarm_high;
short int alarm_low;
short int warning_high;
short int warning_low;*/
short int current_value; /* 光模块偏置电流，范围: 0 ~ 131 mA */
}__attribute__ ((packed)) SFP_bias_data;

typedef struct SFP_power_data_S{
/*short int alarm_high;
short int alarm_low;
short int warning_high;
short int warning_low;*/
short int current_value;/* 光模块接收ONU 光功率，范围: 0 ~ 65535, unit 0.1iW (~ -40 to +8.2 dBm).*/
}__attribute__ ((packed)) SFP_power_data;

extern int  ponSfp_IsSupportRSSI(int slot, int port, struct vty *vty);

extern int  ponSfp_IsOnline(short int PonPortIdx);
extern int  ponSfp_IsGwdType(short int PonPortIdx, struct vty *vty);

/*extern void CheckPonSFPSupportRSSI();*/

#if 0
extern int  ReadSFPTemperature(short int PonPortIdx /*, SFP_temperature_data *temperature*/);
extern int  ReadSFPVoltage(short int PonPortIdx/*, SFP_voltage_data  *voltage*/);
extern int  ReadSFPBias(short int PonPortIdx/*, SFP_bias_data *biasCurrent*/);
extern int  ReadSFPTxPower(short int PonPortIdx/*, SFP_power_data *TxPowerData*/);
#endif
extern ULONG showSFPTemperature(short int PonPortIdx, struct vty *vty);
extern ULONG showSFPVoltage(short int PonPortIdx, struct vty *vty);
extern ULONG showSFPBias(short int PonPortIdx,struct vty *vty);
extern ULONG showSFPPower(short int PonPortIdx,struct vty *vty);

#endif

extern int localOnuRegisterControl( short int pon_id, short int llid, unsigned char *OnuMac, short int  *ret_PonPort );

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
extern int  GetPonUplinkVlanQinQ(short int PonPortIdx, short int OnuIdx, Olt_llid_vlan_manipulation *vlan_uplink_config);
extern int  SetPonUplinkVlanQinQ(short int PonPortIdx, short int OnuIdx, Olt_llid_vlan_manipulation *vlan_uplink_config);
extern int  GetPonDownlinkVlanQinQ(short int PonPortIdx, short int DownVlanId, Olt_llid_vlan_manipulation *vlan_downlink_config);
extern int  SetPonDownlinkVlanQinQ(short int PonPortIdx, short int DownVlanId, Olt_llid_vlan_manipulation *vlan_downlink_config);
extern int  SavePonUplinkVlanManipulation(short int PonPortIdx, short int OnuIdx,  PON_olt_vlan_uplink_config_t  *vlan_uplink_config);
extern int  SavePonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t  original_vlan, PON_olt_vid_downlink_config_t  *vlan_Downlink_config);
extern int  DeletePonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t  original_vlan);
extern int  ShowPonDownlinkVlanManipulation(short int PonPortIdx, short int VlanId, struct vty *vty);
extern int  RestoreUplinkVlanManipulationToPon(short int PonPortIdx, short int OnuIdx);
extern int  RestoreDownllinkVlanManipulationToVlan(short int PonPortIdx, unsigned short VlanIdx);
extern int  RestoreDownlinkVlanManipulationToPon(short int PonPortIdx);
extern int  ClearPonUplinkVlanManipulation(short int PonPortIdx, short int OnuIdx);
extern int  ClearPonDownlinkVlanManipulation(short int PonPortIdx, PON_vlan_tag_t original_vlan);
extern int  GetPonPortVlanTpid(short int PonPortIdx);
extern int  SetPonPortVlanTpid(short int PonPortIdx, unsigned short int vlan_tpid);
#endif
extern void OltOamPtyOnuLoseNoti(long lPonNo, long lOnuNo);

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )

#define PON_OPTICAL_POWER_TRANSMIT_DEFAULT 30

extern short int  Optical_Scope_PonPortIdx ;
extern unsigned int Optical_Scope_Sample_Default;

/*extern unsigned char OamMsgDstMac[];
extern unsigned char OamMsgSrcMac[];*/

typedef struct{
	unsigned char DA[6];
	unsigned char SA[6];
	unsigned short int Type;
	unsigned char SubType;
	unsigned short int Flag;
	unsigned char Code;
	
	unsigned char OUI[3];
	unsigned char GwOpcode;
	unsigned long SendSerNo;
	unsigned short int WholePktLen;
	unsigned short int PayLoadOffset;
	unsigned short int PayLoadLength;
	unsigned char SessionID[8];
}__attribute__ ((packed))GwOamMsgHeader_t;

typedef struct{
	unsigned char subGwOpcode;
	unsigned char Get_reserved[5];
}__attribute__ ((packed))OpticalScope_GetMsgBody_t;

typedef struct{
	unsigned char subGwOpcode;
	unsigned char Rsp_reserved[4];
	unsigned char result;
	unsigned short int sendPower;
	unsigned short int recvPower;
	unsigned short int temperature;
	unsigned short int workVoltage;
	unsigned short int biasCurrent;
}__attribute__ ((packed))OpticalScope_RspMsgBody_t;

typedef struct{
	GwOamMsgHeader_t OamHeader;
	OpticalScope_GetMsgBody_t  OamBody;		
	UCHAR    reserved[60-sizeof(GwOamMsgHeader_t)-sizeof(OpticalScope_GetMsgBody_t)];
} __attribute__ ((packed)) OpticalScope_GetMsg_t;
typedef struct{
	GwOamMsgHeader_t OamHeader;
	OpticalScope_RspMsgBody_t  OamBody;		
	UCHAR    reserved[60-sizeof(GwOamMsgHeader_t)-sizeof(OpticalScope_RspMsgBody_t)];
} __attribute__ ((packed)) OpticalScope_RspMsg_t;

extern int PAS_PowerMeteringAlarmConfig(short int PonPortIdx);

extern int GetPonPortTransOpticalPower(short int PonPortIdx);
extern int GetPonPortRecvOpticalPower(short int PonPortIdx, short int OnuIdx);
extern int GetPonPortTemperature(short int PonPortIdx);
extern int GetPonPortWorkVoltage(short int PonPortIdx);
extern int GetPonPortBiasCurrent(short int PonPortIdx);

extern LONG GetPonPortOpticalMonitorInterval(void);
extern LONG GetPonPortOpticalMonitorEnable(void);
extern LONG GetPonPortOpticalAlwaysOnEnable(void);

/*extern LONG SetPonPortOpticalMonitorInterval(int val);*/
/*extern LONG SetPonPortOpticalMonitorEnable(long val);
extern LONG SetPonPortOpticalAlwaysOnEnable(long val);*/

extern LONG GetPonPortRecvOpticalPowerLowThrd(int flag);
extern LONG GetPonPortRecvOpticalPowerHighThrd(int flag);
extern LONG GetPonPortTransOpticalPowerLowThrd(int flag);
extern LONG GetPonPortTransOpticalPowerHighThrd(int flag);
extern LONG GetPonPortTemperatureLowThrd(int flag);
extern LONG GetPonPortTemperatureHighThrd(int flag);
extern LONG GetPonPortWorkVoltageLowThrd(int flag);
extern LONG GetPonPortWorkVoltageHighThrd(int flag);
extern LONG GetPonPortBiasCurrentHighThrd(int flag);
extern LONG GetPonPortBiasCurrentLowThrd(int flag);

extern short int GetPonPortOpticalPowerCurrentAlarm(short int PonPortIdx);
extern int  ClearPonPortOpticalPowerAlarmFlag(short int PonPortIdx, short int AlarmType );
extern int  SetPonPortOpticalPowerAlarmFlag(short int PonPortIdx, short int AlarmType );
extern short int GetPonPortRecvOpticalPowerCurrentAlarm(short int PonPortIdx,short int OnuIdx);
extern int  ClearPonPortRecvOpticalPowerAlarmFlag(short int PonPortIdx, short int OnuIdx, short int AlarmType );
extern int  SetPonPortRecvOpticalPowerAlarmFlag(short int PonPortIdx, short int OnuIdx,short int AlarmType);
extern int ClearAllOpticalPowerAlarmWhenOnuDeregister(short int PonPortIdx,short int OnuIdx);
extern int ClearOpticalPowerAlamWhenOnuOffline(short int PonPortIdx,short int OnuIdx);

extern LONG SetPonPortRecvOpticalPowerLowThrd(long val,int flag);
extern LONG SetPonPortRecvOpticalPowerHighThrd(long val,int flag);
extern LONG SetPonPortTransOpticalPowerLowThrd(long val,int flag);
extern LONG SetPonPortTransOpticalPowerHighThrd(long val,int flag);
extern LONG SetPonPortTemperatureLowThrd(long val,int flag);
extern LONG SetPonPortTemperatureHighThrd(long val,int flag);
extern LONG SetPonPortWorkVoltageLowThrd(long val,int flag);
extern LONG SetPonPortWorkVoltageHighThrd(long val,int flag);
extern LONG SetPonPortBiasCurrentLowThrd(long val,int flag);
extern LONG SetPonPortBiasCurrentHighThrd(long val,int flag);


extern int BroadcastOamMsg_GetOpticalPowerToOnu(short int PonPortIdx);
extern int GwOamMsg_OpticalScope_Handler(short int PonPortIdx, short int OnuIdx, short int Length, unsigned char *content);
extern short int FindOpticalScopePonPort();
extern void  PollingOnePonPortOpticalScope();
extern void  PollingPonPortOpticalScope(short int PonPortIdx);
/*extern void  ModifiedPonPortSampleInterval( long val);*/

extern int CheckPonPortTransOpticalPowerNeedTrap(short int PonPortIdx, int TransOpticalPower_cur);
extern int CheckPonPortRecvOpticalPowerNeedTrap(short int PonPortIdx, short int OnuIdx, int RecvOpticalPower_cur);
extern int CheckPonPortTemperatureNeedTrap(short int PonPortIdx, int Temperature_cur);
extern int CheckPonPortWorkVoltageNeedTrap(short int PonPortIdx, int workVoltage_cur);
extern int CheckPonPortBiasCurrentNeedTrap(short int PonPortIdx, int BiasCurrent_cur);

/*extern int PonPortMeteringCliInit(void);*/
extern short int GetOnuOpticalPowerCurrentAlarm(short int PonPortIdx,short int OnuIdx);
extern int BroadcastOamMsg_GetOpticalScopeToOnu(short int PonPortIdx);
extern int UnicastOamMsg_GetOpticalScopeToOnu(short int PonPortIdx, short int OnuIdx);
extern int  optical_power_show_run( struct vty * vty );

extern int ClearOpticalPowerAlarmWhenPortDown(short int PonPortIdx);

#endif
/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
extern int GW_set_ddr_configuration(const PON_olt_id_t olt_id);
extern int GW_get_ddr_configuration(const PON_olt_id_t olt_id);
extern int GW_set_ref_clock(const PON_olt_id_t olt_id);
extern int GW_get_ref_clock(const PON_olt_id_t olt_id);
extern int GW_set_olt_ne_mode(const PON_olt_id_t olt_id);
extern int GW_get_olt_ne_mode(const PON_olt_id_t olt_id);
extern int GW_set_olt_flavor_edk_type(const PON_olt_id_t olt_id);
extern int GW_set_gpio_group_mapping(const PON_olt_id_t olt_id, const PON_gpio_grp_type_e gpio_grp_type, const GW10G_PON_optics_module_type_e module_type);
extern int GW_set_gpio_group_mapping_test(const PON_olt_id_t olt_id, const PON_gpio_grp_level_e	 gpio_grp_pon_level, const int type, const int gpio,const int val);

extern int GW_Set_TMSchedMode(const PON_olt_id_t	olt_id,const TM_sched_model_e mode);
extern int GW_TMOperation_CommitSched(const PON_olt_id_t	olt_id);
extern int GW_TMOperation_TrafficEnable(const PON_olt_id_t	olt_id);
extern int GW_set_olt_device_optics( const PON_olt_id_t olt_id, const PON_pon_type optics_type);
extern int GW_set_CNI_interface_type(const PON_olt_id_t olt_id);
extern int GW_get_olt_device_version(const PON_olt_id_t olt_id);

extern int GW_Set_GPIO_Access( const PON_olt_id_t			olt_id, 
							    const GW10G_PON_gpio_lines_t		line_number,
							    const PON_gpio_line_io_t    set_direction,
							    const short int		  	    set_value
							    );
extern int GW_get_address_table_countbymac(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,const INT32U max_count,
INT8U MAC0,INT8U MAC1,INT8U MAC2,INT8U MAC3,INT8U MAC4,INT8U MAC5);
extern int GW_get_address_table_countbyall(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,const INT32U max_count);
extern int GW_get_address_table_bymac(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index,INT8U MAC0,INT8U MAC1,INT8U MAC2,INT8U MAC3,INT8U MAC4,INT8U MAC5);
extern int GW_get_address_table_byall(const PON_olt_id_t olt_id,const INT32U start_index,const INT32S end_index);
extern int GW_add_address_table_record(const PON_olt_id_t olt_id);
extern int GW_Set_GPIO_AccessInit(const PON_olt_id_t	olt_id);
extern int GW_reset_olt_counters(const PON_olt_id_t olt_id);
extern int GW_reset_olt_statistic(const PON_olt_id_t olt_id, INT32U count, STAT_olt_entity_e	entity,INT32U entity_index, INT32U statistic_index );
extern int GW_get_olt_statistic(const PON_olt_id_t olt_id,	INT32U count,	STAT_olt_entity_e	entity,INT32U entity_index, INT32U statistic_index);
extern int GW_get_olt_statistic_PON10G(const PON_olt_id_t olt_id, INT32U entity_index);
extern int GW_get_olt_statistic_PON1G(const PON_olt_id_t olt_id, INT32U entity_index);
extern int GW_get_olt_statistic_CNI(const PON_olt_id_t olt_id, INT32U entity_index);
extern int GW_get_olt_statistic_llid_general(const PON_olt_id_t olt_id, INT32U entity_index);
extern int GW_get_cni_link_status ( const PON_olt_id_t	olt_id);
extern int GW_set_CNI_MAC_mode(const PON_olt_id_t olt_id, const int cni_port, const int cni_conf_type, const int cni_conf_value);
extern int GW_get_CNI_MAC_mode(const PON_olt_id_t olt_id, const int cni_port, const int cni_conf_type);
extern int GW_get_olt_pon_transmission(const PON_olt_id_t olt_id, const PON_pon_type pon_type);
extern int GW_set_olt_pon_transmission(const PON_olt_id_t olt_id, const PON_pon_type pon_type, const int status);
extern int GW_set_tranparent_mode(const PON_olt_id_t olt_id);
extern int GW_set_tranparent_mode_downpon1(const PON_olt_id_t olt_id, const int port_cni);
extern int GW_set_tranparent_mode_downpon0(const PON_olt_id_t olt_id, const int port_cni);
extern int pas_10g_transparent_mode(uint32 olt_device_id);
extern int GW_Set_address_table_config(const PON_olt_id_t olt_id, const NE_addr_config_e config_type, const INT32U address_table_config);
extern int GW_Get_address_table_config(const PON_olt_id_t olt_id, const NE_addr_config_e config_type);
extern int GW_set_fanspeed_fast();
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
extern int Pon_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt);
extern int Pon_SendCliCommand(short int olt_id, short int chip_type, unsigned short size, unsigned char *command);
extern int Pon_SetCniPortMacConfiguration(short int olt_id, short int chip_type, PON_olt_cni_port_mac_configuration_t *olt_cni_port_mac_configuration);
extern int Pon_GetCniPortMacConfiguration(short int olt_id, short int chip_type, PON_olt_cni_port_mac_configuration_t *olt_cni_port_mac_configuration);
extern int Pon_LoadOltBinary(short int olt_id, short int chip_type, PON_indexed_binary_t  *olt_binary);
extern int Pon_StartDbaAlgorithm(short int olt_id, short int chip_type, bool use_default_dba, short int initialization_data_size, void *initialization_data);
extern int Pon_HostRecovery(short int olt_id, PON_olt_mode_t *olt_mode, PON_olt_initialization_parameters_t *initialization_configuration,
	unsigned short int *dba_mode, PON_llid_t *link_test, PON_active_llids_t *active_llids);
extern int Pon_GetOltModeQuery(short int olt_id, PON_olt_mode_t *olt_mode, mac_address_t mac_address,
	PON_olt_init_parameters_t *initialization_configuration, unsigned short int *dba_mode, PON_llid_t *link_test, PON_active_llids_t *active_llids);
extern int Pon_COMMInitOlt ( short int  olt_id);
extern int Pon_GetDeviceVersions(short int olt_id, short int device_id, PAS_device_versions_t *device_versions,
	PON_remote_mgnt_version_t *remote_mgnt_version, unsigned long int *critical_events_counter);
extern void Pon_MonitorEnable(bool status);
extern char *Pon_BooleanString ( const bool  parameter );
extern int Pon_ReceivePollingThreadFunc1(void *lpPacketRecBuf,unsigned int buf_data_size);
extern int Pon_SetClassificationRule(short int olt_id, PON_pon_network_traffic_direction_t direction, PON_olt_classification_t classification_entity, void *classification_parameter, PON_olt_classifier_destination_t destination);
extern int Pon_GetClassificationRule(short int olt_id, PON_pon_network_traffic_direction_t direction, PON_olt_classification_t classification_entity, void *classification_parameter, PON_olt_classifier_destination_t *destination);
extern int Pon_SetTransparentMode(short int PonPortIdx, short int chip_type);
extern int Pon_PLATO_ErrRet(int val, int PonPortIdx);
extern void Pon_Show_SLAInfo(short int PonPortIdx, ONU_SLA_INFO_t SLA_Info);
extern int Pon_SetLLIDSLA(short int olt_id, short int llid, unsigned short gr_fine, unsigned short be_fine, short int *dba_error);
extern int CTC_GetExtendedOamDiscoveryTiming(unsigned short int  *discovery_timeout);
extern int CTC_SetExtendedOamDiscoveryTiming(unsigned short int discovery_timeout);
extern int CTC_GetEncryptionTiming(unsigned char *update_key_time, unsigned short int  *no_reply_timeout);
extern int CTC_SetEncryptionTiming(unsigned char update_key_time, unsigned short int  no_reply_timeout);
extern int CTC_GetEncryptionTimingThreshold(unsigned char *start_encryption_threshold);
extern int CTC_SetEncryptionTimingThreshold(unsigned char start_encryption_threshold);

/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */


#if 1
/* PON PP Handler */

int InitPonPPHandler();

int pon_pp_add_olt(short int olt_id);
int pon_pp_remove_olt(short int olt_id);

int pon_pp_add_link(short int olt_id, short int llid, unsigned long up_tunnel, unsigned long down_tunnel, unsigned long flags);
int pon_pp_remove_link(short int olt_id, short int llid);

int pon_pp_get_link_sla(short int olt_id, short int llid, int bridge_path, BcmSlaQueueParams *sla);
int pon_pp_set_link_sla(short int olt_id, short int llid, int bridge_path, BcmSlaQueueParams *sla);

int pon_pp_get_olt_addrs(short int olt_id, short int *active_records, PON_address_table_t address_table);
int pon_pp_add_olt_addrs(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int pon_pp_del_olt_addrs(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int pon_pp_del_olt_addr(short int olt_id, mac_address_t mac_addr);

int pon_pp_get_link_addrs(short int olt_id, short int llid, short int *active_records, PON_address_table_t address_table);
int pon_pp_clr_link_addrs(short int olt_id, short int llid, PON_address_aging_t address_type);

int pon_pp_get_olt_vlan_tpid(short int olt_id, unsigned short int *tpid_outer, unsigned short int *tpid_inner);
int pon_pp_set_olt_vlan_tpid(short int olt_id, unsigned short int tpid_outer, unsigned short int tpid_inner);
int pon_pp_set_link_vlan_uplink(short int olt_id, short int llid, PON_olt_vlan_uplink_config_t *vlan_uplink_config);
int pon_pp_set_link_vlan_downlink(short int olt_id, PON_vlan_tag_t vlan_id, PON_olt_vid_downlink_config_t *vid_downlink_config);

int pon_pp_set_class_rule(short int olt_id, const PON_pon_network_traffic_direction_t direction, const PON_olt_classification_t classification_entity, const void *classification_parameter, const PON_olt_classifier_destination_t destination);
#endif


#endif  /* _PONGERNRAL_H */




