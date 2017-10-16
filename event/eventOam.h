
/*----------------------------------------------------------------------------*/
#ifndef __INCeventOamh
#define __INCeventOamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define ALARM_REPORT		3
#define ALARM_RESPONSE		4


#define EVENT_TYPE_ONU_TEMPERATURE		2
#define  EVENT_TYPE_ONU_WARM_INSERT_AND_OUT   3
#define EVENT_TYPE_PON_UPLINK_TRAFFIC_CHECK		4
#define EVENT_TYPE_ONUETH_STATUS		10
#define EVENT_TYPE_ONUETH_LOOP			11
#define EVENT_TYPE_ONUETH_MON			20
#define EVENT_TYPE_ONUETH_TRAFFIC		21
#define EVENT_TYPE_ONUPORT_BROADCAST_CONTROL   22 /*add by shixh20090612*/
#define EVENT_TYPE_ONU_OPTICAL_PARA_ALM	25	/*added by wangxiaoyu 2008-7-24 12:09:18*/
#define EVENT_TYPE_ONU_STP				30
#define EVENT_TYPE_ONU_E1					40/*add by shixh@20090220*/
#define EVENT_TYPE_ONU_LOAD				50
#define EVENT_TYPE_ONU_LOADFILE                   70/*add by shixh@20080515*/
#define EVENT_TYPE_ONU_SWITCH				80
#define EVENT_TYPE_ONU_PON_MONITOR			90
#define EVENT_TYPE_ONU_DEVINFO			100


/*add by shixh@20080313*/
enum {
	onuAutoCfg_complete ,
	onuAutoCfg_failure
};

typedef struct {
	uchar_t type;
	uchar_t autocfgresult;   /*自动配置结果*/
} __attribute__((packed)) EventonuAutocfgMsg_t;


typedef struct {
	uchar_t type;
	uchar_t event_data[1500];
} __attribute__((packed)) commEventOamMsg_t;

/* 温度告警 */
enum {
	temperatureHighAlarm = 1,	/* 温度过高告警*/
	temperatureHighClear,		/* 温度过高告警恢复 */
	temperatureLowAlarm,		/* 温度过低告警 */
	temperatureLowClear			/* 温度过低告警恢复 */
};

typedef struct {
	uchar_t type;				/* 告警类型 */
	uchar_t flag;				/* 告警标志 */
	uchar_t temperature;		/* 当前温度 */
	uchar_t threshold;			/* 告警/恢复门限 */
} __attribute__((packed)) eventOnuTemperatureMsg_t;

/* ONU以太网端口状态告警 */
enum {
	ethPortStatus_linkup = 1,
	ethPortStatus_linkdown,
	ethPortStatus_testing,
	ethPortStatus_unknown
};

/*added by wangxiaoyu 2008-7-24 16:10:55
修改了OAM帧中端口ID的描述结构，由原来一个ULONG型数值改为一个结构
*/
typedef struct{
	uchar_t switchID;      /*add by shixh,ONU端口下挂交换机的ID*/
	uchar_t switchPort;    /*交换机的端口号*/
	uchar_t slot;
	uchar_t port;
} __attribute__((packed))oam_port_desc_t;
/*end*/

typedef struct {
	uchar_t type;				/* 告警类型 */
	oam_port_desc_t portId;				/* 端口号 */
	/*uchar_t adminStatus;*/	/* 管理状态 */
	uchar_t linkStatus;			/* 工作状态 */
} __attribute__((packed)) eventOnuEthStatusMsg_t;
enum {
	ethPortStatus_loopAlarm = 1,
	ethPortStatus_loopClear
};

typedef struct {
	uchar_t type;				/* 告警类型 */
	oam_port_desc_t portId;			/* 端口号 */
	uchar_t loopStatus;		/* 工作状态 */
	ushort_t vid;
	uchar_t   switchMac[6];          /*add by shixh20090610,ONU下挂交换机的MAC地址*/
    /*add by zhengyt for loop check @10-5-25*/
    uchar_t    loopInf;         /*环路信息标志:0-不带扩展信息1-带扩展信息*/
    uchar_t     oltType;    /*OLT类型:1-6100,2-6700*/
    uchar_t    oltMac[6];
    ULONG   onuLocal;       /*0+oltboardid+oltportid+onuid*/
    uchar_t  onuType;  /*add by duzhk*/
    uchar_t     onuMac[6];  
    ULONG   onuPortList;
    
} __attribute__((packed)) eventOnuEthLoopMsg_t;

enum {
	onuSwitch_newRegister = 1,
	onuSwitch_reregister,
	onuSwitch_notPresent,
	onuSwitch_ethIngressLimitExceed,
	onuSwitch_ethIngressLimitExceedClear,
	onuSwitch_ethEgressLimitExceed,
	onuSwitch_ethEgressLimitExceedClear
};

typedef struct {
	uchar_t	type;			/* 告警类型: 80 */
	oam_port_desc_t portId;	/* 端口号 */
	uchar_t	flag;			/* 告警标志: 1－第一次注册；2－重新注册；3－离线 */
	uchar_t   switchMac[6];	/* ONU下挂交换机mac地址 */
	uchar_t	reason;
} __attribute__((packed)) eventOnuSwitchMsg_t;

enum {
	onuBackupPon_alarm = 1,
	onuBackupPon_alarmClear = 2,
};

typedef struct {
	uchar_t	type;			/* 告警类型: 80 */
	ULONG portId;	/* 告警源的pon端口号 */
	uchar_t	flag;			/* 告警标志: 1－告警 2－恢复*/
	uchar_t  PonMac[6];	/* 双MAC ONU里，备用PON口的MAC地址；单MAC ONU，就是ONU的MAC地址*/
	ULONG reason;           /*不可用原因，0-未知 1-软件强制 2-光信号los 4-mac los 8-oam los 16-误码率越限 32-设备故障 0xffff-所有原因*/
} __attribute__((packed)) eventOnuPonMsg_t;


/* ONU以太网端口性能告警 */
enum {
	inFlrFlag_noAlarm = 0,
	inFlrFlag_alarm = 1,
	inFlrFlag_alarmClear = 2,
	outFlrFlag_noAlarm = 0,
	outFlrFlag_alarm = 1,
	outFlrFlag_alarmClear = 2,
	inFerFlag_noAlarm = 0,
	inFerFlag_alarm = 1,
	inFerFlag_alarmClear = 2
};
typedef struct {
	uchar_t type;				/* 告警类型 */
	oam_port_desc_t portId;				/* 端口号 */
	uchar_t inFlrFlag;			/* 接收丢帧告警标志 */
	ulong_t inFlr;				/* 接收丢帧率 */
	ulong_t inFlrThreshold;		/* 接收丢帧率告警（恢复）门限 */
	uchar_t outFlrFlag;			/* 发送丢帧告警标志 */
	ulong_t outFlr;				/* 发送丢帧率 */
	ulong_t outFlrThreshold;	/* 发送丢帧率告警（恢复）门限 */
	uchar_t inFerFlag;			/* 误帧告警标志 */
	ulong_t inFer;				/* 误帧率 */
	ulong_t inFerThreshold;		/* 误帧率（告警/恢复）门限 */
} __attribute__((packed)) eventOnuEthMonMsg_t;

/* ONU以太网端口业务中断告警 */
enum {
	ethTraffic_alarm = 1,
	ethTraffic_clear
};

typedef struct {
	uchar_t type;				/* 告警类型 */
	oam_port_desc_t portId;				/* 端口号 */
	uchar_t flag;				/* 接收丢帧告警标志 */
} __attribute__((packed)) eventOnuEthTrafficMsg_t;

enum{
	broadcastfloodcontrol_alarm = 1,
	broadcastfloodcontrol_clear
	};
/*ONU 端口广播风暴抑制告警*//*add by shixh20090612*/
typedef struct{
	uchar_t type;					/* 告警类型 */
	oam_port_desc_t portId;		/* 端口号 */
	uchar_t        flag;             		 /*告警标志*/
	uchar_t       control_status;	/*抑制状态*/
}__attribute__((packed)) eventOnuportBroadcastFloodControlMsg_t;
/* STP事件 */
enum {
	onuStp_topologyChange = 1,
	onuStp_newRoot
};

typedef struct {
	uchar_t type;				/* 告警类型 */
	uchar_t flag;				/* STP事件:1－topologyChange、2－newRoot */
} __attribute__((packed)) eventOnuStpMsg_t;

/* ONU设备信息修改事件 */
typedef struct {
	uchar_t type;				/* 告警类型 */
	uchar_t	nameLen;			/* 名称长度 */
	uchar_t	*pName;				/* 名称信息 */
	uchar_t	descLen;			/* 描述长度 */
	uchar_t	*pDesc;				/* 描述信息 */
	uchar_t	locationLen;		/* 位置长度 */
	uchar_t	*pLocation;			/* 位置信息 */
} __attribute__((packed)) eventOnuDevInfoMsg_t;

/* ONU升级告警 */
enum {
	onuLoad_result_software = 1,	/* */
	onuLoad_result_hardware,		/* */
	onuLoad_result_boot,			/* */
	onuLoad_result_cfgdata,			/* */
	onuLoad_result_voip,			/* */
	onuLoad_result_fpga,			/* */
	
	onuLoad_result_max			/* */
};

typedef struct {
	uchar_t type;				/* 告警类型 */
	uchar_t loadType;			/* 加载数据类型 */
	uchar_t result;				/* 加载结果 */
} __attribute__((packed)) eventOnuLoadMsg_t;

/*add by shixh@20080514*/
enum {
	onuLoadFile_upgrade_complete=0 ,
	onuLoadFile_transfer_complete,
	onuLoadFile_failure,
	onuLoadFile_timeout
};
/*ONU FTP数据加载结果上报事件*/
typedef struct {
	uchar_t alarmtype;				/* 告警类型 */
	ulong_t loadfileType;			/* 加载数据类型 */
	uchar_t loadfileresult;				/* 加载结果 */
	ulong_t  loadresultflag;                   /*加载结束标志*/
	uchar_t  loadDirect;				/*数据加载方向*/
} __attribute__((packed)) eventOnuLoadFileMsg_t;

typedef struct{
	uchar_t alarmtype;/*告警类型*/
	oam_port_desc_t  E1port;     /*端口号*/
	ushort_t alarmparttype;/*告警详细类型*/
}__attribute__((packed))  eventOnuE1Msg_t;

#define  ONULOAD_RESULT_CONFIG_FILE   0x01
#define  ONULOAD_RESULT_BOOT              	0x02
#define  ONULOAD_RESULT_EPON_APP         0x04
#define  ONULOAD_RESULT_HARDWARE        0x08
#define  ONULOAD_RESULT_VOIP_APP          0x10
#define  ONULOAD_RESULT_FPGA                  0x20
#define  ONULOAD_RESULT_ALL                     0xffffffff

/*added by wangxiaoyu 2008-7-24 16:28:21
重定义用于存放端口各告警位的类型，方便灵活的修改最大端口数
*/
typedef ulong_t port_alm_bmask_t;

typedef struct {
	ulong_t hTemperature:1;	/* 温度告警 */
	ulong_t lTemperature:1;	/* 温度告警 */
	ulong_t stpTopChg:1;	/* STP事件 */
	ulong_t stpNewRoot:1;	/* STP事件 */
	ulong_t loadSoft:1;		
	ulong_t loadHard:1;
	ulong_t loadBoot:1;
	ulong_t loadCfgData:1;
    
    /* B--added by liwei056@2009-10-14 for ONU's Optical Alarm */
#if 0    
	/*modified by wangxiaoyu 2008-07-24
	added opticalParaAlm, server bits width dec to 23
	*/
	ulong_t opticalParaAlm: 1
	ulong_t reserve:23;		/* 保留 */
#else
	ulong_t hOpticalPowerUp:1;
	ulong_t lOpticalPowerUp:1;
	ulong_t hOpticalPowerDown:1;
	ulong_t lOpticalPowerDown:1;
	ulong_t hWorkVoltage:1;
	ulong_t lWorkVoltage:1;
	ulong_t hBiasCurrent:1;
	ulong_t lBiasCurrent:1;
	ulong_t hOpticalTemperature:1;
	ulong_t lOpticalTemperature:1;
	ulong_t onuLaserAlwaysOn:1;
	ulong_t reserve:13;		/* 保留 */
#endif
    /* E--added by liwei056@2009-10-14 for ONU's Optical Alarm */
    
	/*ulong_t linkUp;*/			/* bit0-port1,... */
	/*ulong_t linkDown;	*/	/* bit0-port1,... */			/* removed by xieshl 20080506 */
	port_alm_bmask_t inFlr;			/* bit0-port1,... */
	port_alm_bmask_t outFlr;			/* bit0-port1,... */
	port_alm_bmask_t inFer;			/* bit0-port1,... */
	port_alm_bmask_t traffic;		/* bit0-port1,... */
} __attribute__((packed)) onuAlarmFlagRecord_t;

/*end*/

typedef  struct{
uchar_t 	alarmType;	
uchar_t  	alarmFlag;
uchar_t   	slot;
uchar_t   	pduData[EUQ_MAX_OAM_PDU];
} __attribute__((packed)) onusoltWarmInsertAndOut_t;

typedef struct{
	uchar_t almType;
	oam_port_desc_t portId;
	ushort_t paraType;
	uchar_t almFlag;
	ulong_t rtVal;
	ulong_t threshold;
}__attribute__((packed))onuOpticalParaAlmMsg_t;

enum {
	onuslot_inserted=1,
	onuslot_pull,
	onuslot_reset,
	onuslot_statuschange
};

enum{
	GT_EPON_B=1,
	GT_8POTS_A,
	GT_6FE,
	GT_8FE,
	GT_16FE,
	GT_8FXS_A,
	GT_8POTS_B,
	GT_8FXS_B
};

enum{
     none=1,
     doing_init,
     doing_update,
     running_status,
     abnormal_status
};

extern int   initAlarmOam( );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventOamh */
