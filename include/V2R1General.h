/**************************************************************
*
*    V2R1Generall.h -- PON management high level Application functions General header
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	     |    Change			     	|    Author	  
*   ---------|-----------|---------------------|------------
*	1.00	  | 19/05/2006 |   Creation				| chen fj
*
***************************************************************/
#ifndef _USERGERNRAL_H
#define  _USERGERNRAL_H

/*#define V2R1_GFA6700
#define V2R1_GFA6100*/


#ifndef ROK
#define ROK  0
#endif

#ifndef RERROR
#define RERROR (-1)
#endif

#define BYTE_FF  0xff
#define WORD_FF  0xffff
#define INT_FF  0xffffffff

extern unsigned long  getCtcStackSupported();
/* 功能:      判断指定PON是否支持外部数据buf
	输入参数:       slot_id：PON板槽位号3～7（0开始编号）
	pon_id：PON端口号0～3（从0开始编号?
	返回值:   0－不支持、1－支持16M */
extern unsigned long getPonExtSdramSupported( unsigned char slot_id,  unsigned char pon_id );

/* support CTC STACK */
#define  V2R1_CTC_STACK  	(getCtcStackSupported() == 1)
#define  V2R1_NOT_CTC_STACK  ( getCtcStackSupported() != 1)



/* device type */
typedef enum {
	
	V2R1_DEVICE_UNKNOWN = 0,
	V2R1_OTHER = 1,
	V2R1_OLT_GFA6700 = 2,
	V2R1_OLT_GFA6100 = 3,
	V2R1_ONU_GT811 = 4,	
	V2R1_ONU_GT831 = 5,
	V2R1_ONU_GT831_CATV = 6,	
 	V2R1_ONU_GT812 =7,
	V2R1_ONU_GT813 = 8,
	V2R1_ONU_GT881  =9,
	V2R1_ONU_GT861 = 10,
	V2R1_ONU_GT891  =11,
	V2R1_ONU_GT810  =12,
	V2R1_ONU_GT863  =13,
	V2R1_ONU_CTC  =  14,
	V2R1_ONU_GT865 = 15,
	V2R1_ONU_GT816 = 16,
	/* PAS6301 ONU */
	V2R1_ONU_GT811_A = 17,
	V2R1_ONU_GT812_A = 18,
	V2R1_ONU_GT831_A = 19,
	V2R1_ONU_GT831_A_CATV = 20,

	V2R1_ONU_GT815 = 21,
	V2R1_ONU_GT812_B = 22,

	V2R1_ONU_GT831_B = 23,
	V2R1_ONU_GT866 = 24,

	V2R1_ONU_GT811_B = 25,		/*25*/
	/*V2R1_ONU_GT851,*/			/*26*/  /*del by luh 2013-5-24*/
	V2R1_ONU_GT813_B,			/*27*/  /*modi by luh 2013-5-24*/
	V2R1_ONU_GT862,			/*28*/
	V2R1_ONU_GT892,			/*29*/
	
	V2R1_ONU_GT835 = 30,		/*30*/
	V2R1_ONU_GT831_B_CATV,	/*31*/
	V2R1_ONU_GT815_B,			/*32*/
	V2R1_ONU_GT871,			/* 两个GT811_A 合并到一个盒子中, 互为备用*/
	V2R1_ONU_GT871_R,  		/*add by shixh20100122*/
	
	V2R1_ONU_GT872=37,
	V2R1_ONU_GT872_P,			/* added by xieshl 20120302 */
	V2R1_ONU_GT872_R,
	V2R1_ONU_GT873 = 40,
	V2R1_ONU_GT873_P,
	V2R1_ONU_GT873_R,
	V2R1_ONU_GT871_P,			/* 43 */
	V2R1_ONU_GT870,
	V2R1_ONU_GT811_C,

    V2R1_ONU_GPON = 60,
	V2R1_ONU_CMC  =  80,
	/*V2R1_ONU_MAX=95,*//*change by yanjy2017-2*/
	V2R1_OLT_GFA6900M = 96,
	V2R1_OLT_GFA6900S = 97,
	V2R1_OLT_GFA6900 = 99,
	V2R1_OLT_GFA8000M = 100,
	V2R1_OLT_GFA8000S = 101,
	V2R1_OLT_GFA8000 = 102,
	V2R1_OLT_GFA8100 = 103,
	V2R1_OLT_GFA8100GP = 104, /*此类型只用于提供给网管区分8100-16EP/16GP类型 */
	V2R1_ONU_MAX=300,
	V2R1_DEVICE_LAST = 301
}V2R1_DeviceType;

#define V2R1_PRODUCT_IS_6100Series(type)  ((type) == V2R1_OLT_GFA6100)
#define V2R1_PRODUCT_IS_6700Series(type)  ((type) == V2R1_OLT_GFA6700)
#define V2R1_PRODUCT_IS_6900Series(type)  (((type) >= V2R1_OLT_GFA6900M) && ((type) <= V2R1_OLT_GFA6900))
#define V2R1_PRODUCT_IS_8000Series(type)  (((type) >= V2R1_OLT_GFA8000M) && ((type) <= V2R1_OLT_GFA8000))
#define V2R1_PRODUCT_IS_8100Series(type)  ((type) == V2R1_OLT_GFA8100)

#define V2R1_PRODUCT_IS_H_Series(type)    ((((type) >= V2R1_OLT_GFA6900M) && ((type) <= V2R1_OLT_GFA8000))||((type) == V2R1_OLT_GFA8100))
#define V2R1_PRODUCT_IS_O_Series(type)    (((type) >= V2R1_OLT_GFA6700) || ((type) <= V2R1_OLT_GFA6100))

#define V2R1_PRODUCT_IS_EPON(type)        (V2R1_PRODUCT_IS_H_Series(type) || V2R1_PRODUCT_IS_O_Series(type))

#define V2R1_PRODUCT_IS_L_Series(type)    (((type) == V2R1_OLT_GFA6900) || ((type) == V2R1_OLT_GFA8000) || ((type) == V2R1_OLT_GFA6700))
#define V2R1_PRODUCT_IS_M_Series(type)    (((type) == V2R1_OLT_GFA6900M) || ((type) == V2R1_OLT_GFA8000M))
#define V2R1_PRODUCT_IS_S_Series(type)    (((type) == V2R1_OLT_GFA6900S) || ((type) == V2R1_OLT_GFA8000S) || ((type) == V2R1_OLT_GFA6100)||((type) == V2R1_OLT_GFA8100))

#define V2R1_PRODUCT_IS_HL_Series(type)    (((type) == V2R1_OLT_GFA6900)  || ((type) == V2R1_OLT_GFA8000))
#define V2R1_PRODUCT_IS_HM_Series(type)    V2R1_PRODUCT_IS_M_Series(type)
#define V2R1_PRODUCT_IS_HS_Series(type)    (((type) == V2R1_OLT_GFA6900S) || ((type) == V2R1_OLT_GFA8000S) || ((type) == V2R1_OLT_GFA8100))

#define V2R1_PRODUCT_IS_CentSeries(type)  (V2R1_PRODUCT_IS_O_Series(type) || V2R1_PRODUCT_IS_S_Series(type))
#define V2R1_PRODUCT_IS_DistSeries(type)  (V2R1_PRODUCT_IS_H_Series(type) && !V2R1_PRODUCT_IS_HS_Series(type))

#if 0
#define  V2R1_UNKNOWN 0
#define  V2R1_OTHER  1
#define  V2R1_OLT_GFA6700  2
#define  V2R1_OLT_GFA6100  3
#define  V2R1_ONU_GT811  4
#define  V2R1_ONU_GT821  5
#define  V2R1_ONU_GT831  6
#define  V2R1_ONU_GT812  7
#define  V2R1_ONU_GT813  8
#define  V2R1_ONU_GT881  9
#define  V2R1_ONU_GT861  10
#define  V2R1_ONU_GT891  11
#define  V2R1_ONU_GT810  12
#define  V2R1_ONU_GT863  13
#define  V2R1_ONU_CTC    14
#define  V2R1_ONU_GT865  15
#endif
/* device type by Daya onu
#define DAYA_ONU_GT811  1   */

#define MAXVENDORINFOLEN 128
#define MAXDEVICENAMELEN 128
#define MAXDEVICEDESCLEN 128 
#define MAXLOCATIONLEN 128
#define MAXV2r1CopyrightLEN 64
#define MAXV2R1SeriesLEN  64
#define MAXCorporationNameLEN  64
#define MAXCorporationNameLENShort  32
#define MAXCorporationNameLenAB  16

#define  MAXProductSoftwareDescLen  128

#define  BOARDTYPESTRINGLEN  32
#define  BOARDTYPESTRINGLEN_SHORT 16

#define SYS_MAX_SW_CARDNUM      2  
#define SYS_MAX_UPLINK_CARDNUM  12  
#define SYS_MAX_TDM_CARDNUM     5  
#define SYS_MAX_PWU_CARDNUM     4  
#define SYS_MAX_FAN_CARDNUM     3  

extern unsigned char DeviceDesc_1[V2R1_DEVICE_LAST][MAXDEVICEDESCLEN+1];
extern unsigned char DeviceType_1[V2R1_DEVICE_LAST][16+1];
extern unsigned char OnuAppType_1[V2R1_ONU_MAX][16+1];
extern unsigned char OnuVoiceType_1[V2R1_ONU_MAX][16+1];
extern unsigned char OnuFpgaType_1[V2R1_ONU_MAX][16+1];
extern int OnuCtcRegisterId[V2R1_ONU_MAX];

/*extern unsigned char DeviceDesc[][];*/

extern unsigned char  *Fec_Ability[];

#define ONU_APP_NAME  "appimage.bin"
#define ONU_LOG_NAME  "syslog"
#define ONU_CONFIG_NAME "config.txt"
#define ONU_VOICE_NAME  "appvoice.bin"
#define ONU_FPGA_NAME "appfpga.bin"
#define OnuDefaultConfigName "gen-onu.config"
#define SYSFILE_INI   "sysfile.ini"   /*add by shixh20090518*/

#define  V2R1_UPDATE_ONU_IMAGE_GW   1
#define  V2R1_UPDATE_ONU_IMAGE_CTC  2
/*
#define  V2R1_CTC_ONU_IMAGE_UCOS_NAME  "_CTC_UCOS"
#define  V2R1_CTC_ONU_IMAGE_LINUX_NAME  "_CTC_LINUX"
*/
#define V2R1_CTC_ONU   "GT_CTC"
#define V2R1_CTC_ONU_4FE   "GT_CTC_4FE"
#define V2R1_CTC_ONU_4FE_2VOIP   "GT_CTC_4FE_2VOIP"

#define  V2R1_CTC_ONU_IMAGE_UCOS  1
#define  V2R1_CTC_ONU_IMAGE_LINUX 2 

/* 文件在FLASH中存放*/
typedef struct{
	char dev_type[16];
	ULONG loc_offset;
	ULONG file_len;
	ULONG compress_flag;
	char  reserve[8];
	char  file_ver[92];
}__attribute__((packed))app_desc_t;

#define  ONU_FLASH_FILE_ID  5
#define  OLT_FLASH_FILE_ID  11

#define  PON_FW_FLASH_ID  7
#define  PON_DBA_FLASH_ID   10

#define  V2R1_VERSION_CONSISTENT   1
#define  V2R1_VERSION_NO_CONSISTENT  2

#define V2R1_NO_WAIT_RETURN   1
#define V2R1_WAIT_RETURN  2

#define V2R1_DATA_NOT_CHANGED  0
#define V2R1_DATA_CHANGED 1

#define V2R1_LOAD_FILE_SUCCESS  1
#define V2R1_LOAD_FILE_FAILURE  2


/* olt 定时广播系统时间周期*/
extern unsigned int  V2R1_SYS_TIME_PERIOD;

#ifndef MAX_ONU_ETHPORT
#define MAX_ONU_ETHPORT	 32/*24*/    /*modi by luh@2015-1-6, GT524等ONU 多于24口，需要扩充能力*/
#endif
#ifndef MAX_ETH_PORT_NUM
#define MAX_ETH_PORT_NUM	24
#endif


#define FIRSTPONPORTPERCARD  1

#define CARDINSERT        1
#define CARDNOTINSERT  0

#define  V2R1_REGISTER_WINDOW_CLOSE  0
/*#define  PON_MAX_RTT_40KM  12500*/
/* modified by chenfj 2009-6-3
     PON 默认ONU 注册窗口由20KM 调整到25KM 左右*/
#define  PON_MAX_RTT_40KM  15800
#define  PON_MAX_RTT_80KM  27500
#define  PON_MAX_RTT_120KM  41000
#define  PON_MAX_RTT_160KM  51000

#define V2R1_PON_SAMPLING_CYCLE  4000
#define V2R1_PON_MONITORING_CYCLE 4000
#define V2R1_PON_HOST_MSG_TIMEOUT 1600
#define V2R1_PON_HOST_MSG_OUT 100

/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
#define V2R1_PON_HOST_MANAGE_BY_ETH  0
#define V2R1_PON_HOST_MANAGE_BY_BUS  1
#define V2R1_PON_HOST_MANAGE_BY_URT  2
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

/* B--added by liwei056@2012-5-6 for 6900-12PON-B2 */
#define V2R1_PON_HOST_MANAGE_ADDRESS_BASE0   0xFA00
#define V2R1_PON_HOST_MANAGE_ADDRESS_BASE1   0xF810
/* E--added by liwei056@2012-5-6 for 6900-12PON-B2 */

/* alarm ID */
#define OLTALARMNO  0xa0
#define PONALARMNO  0xa1
#define ONUALARMNO  0xa2
#define PWUALARMID  0xa3
#define FANALARMID  0xa4


#define MAXOLTMSGNUM				1000
#define MAXPONMSGNUM				1000
#define MAXLOWLEVELMSGNUM		1000	/*500*/
#define MAXPONUPDATEMSGNUM		1000
#define MAXONUMSGNUM			(2048*4)	/*1280*/
#define MAXONUAUTOCONFIGMSGNUM	1000
#define MAXTDMMGMTMSGNUM			100

/*** module id , 由软件平台统一分派***/
/* modified by xieshl 20100804 */
#define MODULE_PON			MODULE_RPU_PON
#define MODULE_PONUPDATE	MODULE_RPU_PON_UPDATE
#define MODULE_OLT  		MODULE_RPU_OLT
#define MODULE_ONU			MODULE_RPU_ONU

#define MODULE_TDM_CARD_MGMT		MODULE_RPU_TDM_MGMT
#define MODULE_TDM_ONU_SERVICE	MODULE_RPU_TDM_SERVICE

/*begin add by wangxy 2007-6-15 13:54:05*/
#define MODULE_TDM_COMM	MODULE_RPU_TDM_COMM

#define MODULE_OAM			MODULE_RPU_OAM
#define MODULE_STATSTICS	MODULE_RPU_STATSTICS
#define MODULE_MON			MODULE_RPU_PON_MON
#define MODULE_LOOPBACK	MODULE_RPU_LOOPBACK

#define MODULE_EVENT		MODULE_RPU_EVENT
#define MODULE_IGMPAUTH	MODULE_RPU_IGMPAUTH
#define MODULE_TRANSFILE	MODULE_RPU_TRANSFILE     /*  OAM文件传递 模块ID   */
/* 20100804 */

/*** msg function code ***/
#define FC_ONU_REGISTER		0xc0
#define FC_ONU_PARTNER_REG	0x10c0
/*for onu swap by jinhl@2013-04-27*/
#define FC_ONUSWAP_PARTNER_REG	0x10c1/*支持onu倒换的对端虚注册事件*/
#define FC_ONU_REGISTER_INPROCESS 0x11c0
#define FC_ONU_REGISTER_SETBANDWIDTH 0x11c2
#define FC_ONU_EVENT_TIMEOUT 0x0020
#define FC_ONU_REGISTER_TIMEOUT 0x0021
#define FC_ONU_EVENT_PONCARD_RESET 0x0022
#define FC_ONU_EVENT_PONPORT_RESET 0x0023
#define FC_ONU_DEREGISTER		0xc1
#define FC_PON_ALARM			0xc2
#define FC_ONU_AUTH			0xc3 
#define FC_PON_RESET			0xc4 
#define FC_PON_FILE_LOADED		0xc5 
#define FC_START_ENCRYPT		0x10c6
#define FC_STOP_ENCRYPT		0x10c7
#define FC_ENCRYPT_KEY			0x10c8

#define FC_STARTENCRYPTION_COMP 0xc6
#define FC_STOPENCRYPTION_COMP 0xc7
#define FC_UPDATEENCRYPTIONKEY_COMP 0xc8

#define FC_PONUPDATE  0xc9 
#define FC_ONUUPDATE  0xca
#define FC_ONU_EUQ_INFO  0xcb
#define FC_ONU_TDM_SERVICE 0xcf
#define FC_ONU_TDM_SERVICE_ALL 0x1cf
#define FC_V2R1_TIMEOUT  0xcc
#define FC_V2R1_TIMEOUT_UPLINKLOS 0x1cc
#define FC_V2R1_START_ENCRYPT 0xcd
#define FC_V2R1_ONU_AUTO_CONFIG  0xce
#define FC_ONU_AGINGTIME  0x2cc
#define FC_ONU_STATUS_SYNC_TIMEOUT  0x3cc

#define FC_CARD_INSERTED  0xd0
#define FC_CARD_PULLED   0xd1
#define FC_CARD_ACTIVATED  0xd2
#define FC_PONPORT_ACTIVATED 0xd3
#define FC_ADD_PONPORT  0xd4
#define FC_DEL_PONPORT  0xd5
#define FC_ACTIVE_PENDING_ONU 0xd6
#define FC_ACTIVE_PENDING_ONU_CONF 0xd7
#define FC_ACTIVE_PENDING_ONU_CONF_ALL 0xd8
#define FC_ACTIVE_FENDING_ONU_BY_MAC  0xd9
#define FC_PONDEV_RESET 0xda/*by jinhl*/
#define FC_PONDEV_DISCONNECT 0xdb
#define FC_ACTIVE_FENDING_ONU_BY_SN  0xdc

#define FC_LOWLEVEL_COMM 0xe0
#define FC_FTP_LOAD    0xe1
#define FC_LPB_SEND    0xe2
/*begin add by wangxy 2007-6-15 13:54:05*/
#define FC_TDM_COMM	0xe3
/*end*/
#define  FC_OAM_COMM_RECV  0xe8

#define  FC_EXTOAMDISCOVERY  0xf0
#define  FC_PONLOSS  0xf1
#define  FC_ONUDENIED  0xf2
#define  FC_UPDATE_ONU_APP_IMAGE  0xf3
#define  FC_PONSWITCH  0xf4
#define  FC_ONUMGTSYNCDATE  0xf5        /*add by shixh20100525*/
#define  FC_ONURECEIVENOTIFY       0xf6
#define  FC_ONUSWITCH  0xf7
#define  FC_SFPLOSS  0xf8

#define  FC_TEST_PONPORT  0xff


/* task priority defined */
#define  TASK_PRIORITY_OLT   95
#define  TASK_PRIORITY_PON   58
#define  TASK_PRIORITY_PONUPDATE  56
#define  TASK_PRIORITY_LOWLEVELCOMM 45
#define  TASK_PRIORITY_ONU  59
#define  TASK_PRIORITY_TDM_MGMT 60
#define  TASK_PRIORITY_GW_FTP  100  /*added by zhengyt 09-04-09*/
#define  TASK_PRIORITY_ONU_MGT_SYNC  96  /*add by shixh20100525*/

/* task Id */
extern LONG  g_Olt_Task_Id ; 
extern LONG  g_Pon_Task_Id;	
extern LONG  g_PonUpdate_Task_Id;
extern LONG  g_Onu_Task_Id;
extern LONG  g_LowLevelComm_Task_Id;
extern LONG  g_Tdm_mgmt_Task_Id;

/* Queue id */
extern unsigned long  g_Olt_KPALIVE_Queue_Id ;
extern unsigned long  g_Olt_Queue_Id ;
extern unsigned long  g_Pon_Queue_Id;
extern unsigned long  g_PonUpdate_Queue_Id;
extern unsigned long  g_Onu_Queue_Id;
extern unsigned long  g_LowLevelComm_Queue_Id;
extern unsigned long g_Tdm_mgmt_Queue_Id;
/* flag */
extern unsigned int  Olt_Task_flag;
extern unsigned int  Pon_Task_flag;
extern unsigned int  PonUpdate_Task_flag;
extern unsigned int  Onu_Task_flag;
extern unsigned int  LowLevelComm_Task_flag;
extern unsigned int  Tdm_mgmt_Task_flag;
	
extern int  SYS_INIT_COMP;

extern int VOS_Ticks_Per_Seconds;

#define  V2R1_ONE_MINUTE  (60)
#define  V2R1_ONE_HOUR  (60*V2R1_ONE_MINUTE)
#define  V2R1_ONE_DAY  (24*V2R1_ONE_HOUR)

typedef struct
{
 unsigned char bMacAdd[6];

}Enet_MACAdd_Infor1;  

extern int rand (void);
/*extern void v2r1_srand( );*/

/* OnStatus -1 表示olt 在线;0 - 表示掉线 */
extern STATUS StatsMsgPonOnSend(unsigned short oltId, unsigned char OnStatus);
extern STATUS StatsMsgOnuOnSend(unsigned short oltId, unsigned short onuId, unsigned char OnStatus);
extern Enet_MACAdd_Infor1 *funReadMacAddFromNvram( Enet_MACAdd_Infor1 *);

extern int  EVENT_DEBUG;
extern int  EVENT_ENCRYPT;
extern int  EVENT_REGISTER;
extern int  EVENT_ALARM;
extern int  EVENT_RESET;
extern int  EVENT_PONADD;
extern int  MAKEING_TEST_FLAG;
extern int  EVENT_OAM_DEBUG;
extern int  EVENT_UPDATE_ONU_FILE;
extern int  EVENT_TESTPONPORT;
extern int  EVENT_TRAP;
extern int  EVENT_EUQ;
extern int  MAKEING_TEST_FLAG_DEFAULT;


extern int  downlinkBWlimit;
extern int  downlinkBWlimitBurstSize;
extern int  downlinkBWlimitPreference;
extern int  downlinkBWlimitDefault;
extern int  uplinkBWPacketUnitSize;
extern int  uplinkBWlimitBurstSize;
extern int  uplinkBWWeight;
extern int  downlinkBWWeight;


#define  PONid_ONUMAC_BINDING (MAKEING_TEST_FLAG == V2R1_ENABLE ? V2R1_DISABLE : V2R1_ENABLE)


extern int v2r1_printf( struct vty *vty, const char * format, ... );
extern void pktDataPrintf( uchar_t *pOctBuf, ulong_t bufLen );
extern ULONG * V2R1_Parse_OnuId_List( CHAR * pcOnuId_List );
extern ULONG * V2R1_Parse_Id_List( CHAR * pcId_List, ULONG ulIdMin, ULONG ulIdMax, ULONG ulIdNumMax );

#define BEGIN_PARSE_ONUID_LIST_TO_ONUID(onuid_list, onuid) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_OnuId_List(onuid_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            onuid = _pulIfArray[_i];\
            if(onuid>MAXONUPERPON) continue;

#define END_PARSE_ONUID_LIST_TO_ONUID() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}

#define RETURN_PARSE_ONUID_LIST_TO_ONUID(x) { \
       VOS_Free(_pulIfArray);\
	return (x); \
}


#define BEGIN_PARSE_ID_LIST_TO_ID(id_list, id, id_min, id_max, id_num) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_Id_List(id_list, id_min, id_max, id_num);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            id = _pulIfArray[_i];

#define END_PARSE_ID_LIST_TO_ID() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}

#define RETURN_PARSE_ID_LIST_TO_ID(x) { \
       VOS_Free(_pulIfArray);\
	return (x); \
}

/*Begin: added by wangjiah@2017-05-15
 * swap olt_id or slot/port of passive pon port to active one
 * to avoid configuration on passive pon port
 * SWAP_TO_ACTIVE_PON_PORT_INDEX/SWAP_TO_ACTIVE_PON_PORT DON'T support remote olt 
 * To support remote olt, use  SWAP_TO_ACTIVE_PON_PORT_INDEX_R/SWAP_TO_ACTIVE_PON_PORT_R
 */

extern int SWAP_PASSIVE_ID_DEBUG;
#define DEBUG_SWAP_PASSIVE if(1 == SWAP_PASSIVE_ID_DEBUG) sys_console_printf
#define SWAP_TO_ACTIVE_PON_PORT_INDEX(olt_id) { \
	short int _partner_olt_id = 0; \
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& VOS_OK == PonPortSwapPortQuery(olt_id, &_partner_olt_id) \
			&& V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(olt_id)) \
	{ \
		if(OLT_ISLOCAL(_partner_olt_id)) \
		{olt_id = _partner_olt_id;DEBUG_SWAP_PASSIVE("\r\n swap to active olt_id: %d\r\n", olt_id);} \
		else \
		{DEBUG_SWAP_PASSIVE("\r\n Don't support remote olt config\r\n");return VOS_ERROR;} \
	} \
}


#define SWAP_TO_ACTIVE_PON_PORT(slot, port)	{ \
	int _partner_slot = 0; \
	int _partner_port = 0; \
	short int _pon_port_idx = GetPonPortIdxBySlot(slot, port); \
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& VOS_OK == PonPortSwapSlotQuery(_pon_port_idx, &_partner_slot, &_partner_port) \
			&& V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(_pon_port_idx)) \
   	{ \
		if(OLT_SLOT_ISVALID(_partner_slot)) \
		{slot = _partner_slot; port = _partner_port;sys_console_printf("\r\n swap to active slot: %d port: %d\r\n", slot, port);}\
		else \
		{DEBUG_SWAP_PASSIVE("\r\n Don't support remote olt config\r\n");return VOS_ERROR;} \
	} \
}

#define SWAP_TO_ACTIVE_PON_PORT_INDEX_R(olt_id) { \
	short int _partner_olt_id = 0; \
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& VOS_OK == PonPortSwapPortQuery(olt_id, &_partner_olt_id) \
			&& V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(olt_id)) \
	{olt_id = _partner_olt_id;DEBUG_SWAP_PASSIVE("\r\n swap to active olt_id: %d\r\n", olt_id);}}


#define SWAP_TO_ACTIVE_PON_PORT_R(slot, port)	{ \
	int _partner_slot = 0; \
	int _partner_port = 0; \
	short int _pon_port_idx = GetPonPortIdxBySlot(slot, port); \
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE \
			&& VOS_OK == PonPortSwapSlotQuery(_pon_port_idx, &_partner_slot, &_partner_port) \
			&& V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(_pon_port_idx)) \
	{slot = _partner_slot; port = _partner_port;DEBUG_SWAP_PASSIVE("\r\n swap to active slot: %d port: %d\r\n", slot, port);}}

/*End: added by wangjiah@2017-05-15*/

#endif

