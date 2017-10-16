
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
	uchar_t autocfgresult;   /*�Զ����ý��*/
} __attribute__((packed)) EventonuAutocfgMsg_t;


typedef struct {
	uchar_t type;
	uchar_t event_data[1500];
} __attribute__((packed)) commEventOamMsg_t;

/* �¶ȸ澯 */
enum {
	temperatureHighAlarm = 1,	/* �¶ȹ��߸澯*/
	temperatureHighClear,		/* �¶ȹ��߸澯�ָ� */
	temperatureLowAlarm,		/* �¶ȹ��͸澯 */
	temperatureLowClear			/* �¶ȹ��͸澯�ָ� */
};

typedef struct {
	uchar_t type;				/* �澯���� */
	uchar_t flag;				/* �澯��־ */
	uchar_t temperature;		/* ��ǰ�¶� */
	uchar_t threshold;			/* �澯/�ָ����� */
} __attribute__((packed)) eventOnuTemperatureMsg_t;

/* ONU��̫���˿�״̬�澯 */
enum {
	ethPortStatus_linkup = 1,
	ethPortStatus_linkdown,
	ethPortStatus_testing,
	ethPortStatus_unknown
};

/*added by wangxiaoyu 2008-7-24 16:10:55
�޸���OAM֡�ж˿�ID�������ṹ����ԭ��һ��ULONG����ֵ��Ϊһ���ṹ
*/
typedef struct{
	uchar_t switchID;      /*add by shixh,ONU�˿��¹ҽ�������ID*/
	uchar_t switchPort;    /*�������Ķ˿ں�*/
	uchar_t slot;
	uchar_t port;
} __attribute__((packed))oam_port_desc_t;
/*end*/

typedef struct {
	uchar_t type;				/* �澯���� */
	oam_port_desc_t portId;				/* �˿ں� */
	/*uchar_t adminStatus;*/	/* ����״̬ */
	uchar_t linkStatus;			/* ����״̬ */
} __attribute__((packed)) eventOnuEthStatusMsg_t;
enum {
	ethPortStatus_loopAlarm = 1,
	ethPortStatus_loopClear
};

typedef struct {
	uchar_t type;				/* �澯���� */
	oam_port_desc_t portId;			/* �˿ں� */
	uchar_t loopStatus;		/* ����״̬ */
	ushort_t vid;
	uchar_t   switchMac[6];          /*add by shixh20090610,ONU�¹ҽ�������MAC��ַ*/
    /*add by zhengyt for loop check @10-5-25*/
    uchar_t    loopInf;         /*��·��Ϣ��־:0-������չ��Ϣ1-����չ��Ϣ*/
    uchar_t     oltType;    /*OLT����:1-6100,2-6700*/
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
	uchar_t	type;			/* �澯����: 80 */
	oam_port_desc_t portId;	/* �˿ں� */
	uchar_t	flag;			/* �澯��־: 1����һ��ע�᣻2������ע�᣻3������ */
	uchar_t   switchMac[6];	/* ONU�¹ҽ�����mac��ַ */
	uchar_t	reason;
} __attribute__((packed)) eventOnuSwitchMsg_t;

enum {
	onuBackupPon_alarm = 1,
	onuBackupPon_alarmClear = 2,
};

typedef struct {
	uchar_t	type;			/* �澯����: 80 */
	ULONG portId;	/* �澯Դ��pon�˿ں� */
	uchar_t	flag;			/* �澯��־: 1���澯 2���ָ�*/
	uchar_t  PonMac[6];	/* ˫MAC ONU�����PON�ڵ�MAC��ַ����MAC ONU������ONU��MAC��ַ*/
	ULONG reason;           /*������ԭ��0-δ֪ 1-���ǿ�� 2-���ź�los 4-mac los 8-oam los 16-������Խ�� 32-�豸���� 0xffff-����ԭ��*/
} __attribute__((packed)) eventOnuPonMsg_t;


/* ONU��̫���˿����ܸ澯 */
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
	uchar_t type;				/* �澯���� */
	oam_port_desc_t portId;				/* �˿ں� */
	uchar_t inFlrFlag;			/* ���ն�֡�澯��־ */
	ulong_t inFlr;				/* ���ն�֡�� */
	ulong_t inFlrThreshold;		/* ���ն�֡�ʸ澯���ָ������� */
	uchar_t outFlrFlag;			/* ���Ͷ�֡�澯��־ */
	ulong_t outFlr;				/* ���Ͷ�֡�� */
	ulong_t outFlrThreshold;	/* ���Ͷ�֡�ʸ澯���ָ������� */
	uchar_t inFerFlag;			/* ��֡�澯��־ */
	ulong_t inFer;				/* ��֡�� */
	ulong_t inFerThreshold;		/* ��֡�ʣ��澯/�ָ������� */
} __attribute__((packed)) eventOnuEthMonMsg_t;

/* ONU��̫���˿�ҵ���жϸ澯 */
enum {
	ethTraffic_alarm = 1,
	ethTraffic_clear
};

typedef struct {
	uchar_t type;				/* �澯���� */
	oam_port_desc_t portId;				/* �˿ں� */
	uchar_t flag;				/* ���ն�֡�澯��־ */
} __attribute__((packed)) eventOnuEthTrafficMsg_t;

enum{
	broadcastfloodcontrol_alarm = 1,
	broadcastfloodcontrol_clear
	};
/*ONU �˿ڹ㲥�籩���Ƹ澯*//*add by shixh20090612*/
typedef struct{
	uchar_t type;					/* �澯���� */
	oam_port_desc_t portId;		/* �˿ں� */
	uchar_t        flag;             		 /*�澯��־*/
	uchar_t       control_status;	/*����״̬*/
}__attribute__((packed)) eventOnuportBroadcastFloodControlMsg_t;
/* STP�¼� */
enum {
	onuStp_topologyChange = 1,
	onuStp_newRoot
};

typedef struct {
	uchar_t type;				/* �澯���� */
	uchar_t flag;				/* STP�¼�:1��topologyChange��2��newRoot */
} __attribute__((packed)) eventOnuStpMsg_t;

/* ONU�豸��Ϣ�޸��¼� */
typedef struct {
	uchar_t type;				/* �澯���� */
	uchar_t	nameLen;			/* ���Ƴ��� */
	uchar_t	*pName;				/* ������Ϣ */
	uchar_t	descLen;			/* �������� */
	uchar_t	*pDesc;				/* ������Ϣ */
	uchar_t	locationLen;		/* λ�ó��� */
	uchar_t	*pLocation;			/* λ����Ϣ */
} __attribute__((packed)) eventOnuDevInfoMsg_t;

/* ONU�����澯 */
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
	uchar_t type;				/* �澯���� */
	uchar_t loadType;			/* ������������ */
	uchar_t result;				/* ���ؽ�� */
} __attribute__((packed)) eventOnuLoadMsg_t;

/*add by shixh@20080514*/
enum {
	onuLoadFile_upgrade_complete=0 ,
	onuLoadFile_transfer_complete,
	onuLoadFile_failure,
	onuLoadFile_timeout
};
/*ONU FTP���ݼ��ؽ���ϱ��¼�*/
typedef struct {
	uchar_t alarmtype;				/* �澯���� */
	ulong_t loadfileType;			/* ������������ */
	uchar_t loadfileresult;				/* ���ؽ�� */
	ulong_t  loadresultflag;                   /*���ؽ�����־*/
	uchar_t  loadDirect;				/*���ݼ��ط���*/
} __attribute__((packed)) eventOnuLoadFileMsg_t;

typedef struct{
	uchar_t alarmtype;/*�澯����*/
	oam_port_desc_t  E1port;     /*�˿ں�*/
	ushort_t alarmparttype;/*�澯��ϸ����*/
}__attribute__((packed))  eventOnuE1Msg_t;

#define  ONULOAD_RESULT_CONFIG_FILE   0x01
#define  ONULOAD_RESULT_BOOT              	0x02
#define  ONULOAD_RESULT_EPON_APP         0x04
#define  ONULOAD_RESULT_HARDWARE        0x08
#define  ONULOAD_RESULT_VOIP_APP          0x10
#define  ONULOAD_RESULT_FPGA                  0x20
#define  ONULOAD_RESULT_ALL                     0xffffffff

/*added by wangxiaoyu 2008-7-24 16:28:21
�ض������ڴ�Ŷ˿ڸ��澯λ�����ͣ����������޸����˿���
*/
typedef ulong_t port_alm_bmask_t;

typedef struct {
	ulong_t hTemperature:1;	/* �¶ȸ澯 */
	ulong_t lTemperature:1;	/* �¶ȸ澯 */
	ulong_t stpTopChg:1;	/* STP�¼� */
	ulong_t stpNewRoot:1;	/* STP�¼� */
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
	ulong_t reserve:23;		/* ���� */
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
	ulong_t reserve:13;		/* ���� */
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
