
/*----------------------------------------------------------------------------*/
#ifndef __INCeventLogh
#define __INCeventLogh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct {
	ULONG  year		:6;
	ULONG  month	:4;
	ULONG  day		:5;
	ULONG  hour		:5;
	ULONG  minute	:6;
	ULONG  second	:6;
}__attribute__((packed)) nvramLogDateAndTime_t;


typedef struct {
	UCHAR  alarmType;
	UCHAR  alarmId;
	UCHAR  alarmSrcData[MAXLEN_EVENT_DATA];
	nvramLogDateAndTime_t  alarmTime;
}__attribute__((packed)) nvramEventLogData_t;

typedef struct {
	ULONG	eventLogFlag;				/* ��Ч��־ */
	ULONG 	eventLogCurIndex;			/* ��־���� */
	USHORT 	eventLogHeadSuffix;			/* ��һ��ָʾ�±� */
	USHORT 	eventLogCurSuffix;			/* ��ǰ��ָʾ�±� */
	ULONG	reserve;
}__attribute__((packed)) nvramEventLogHead_t;

enum {
	alarmType_mib2 = 1,
	alarmType_bridge = 2,
	alarmType_private = 3,
	alarmType_bcmcmcctrl = 4,
	alarmType_other = 0xff
};

enum{
	trap_mib2_min = 0,
	trap_coldStart = 1,
	trap_warmStart = 2,
	trap_linkDown = 3,
	trap_linkUp = 4,
	trap_authenticationFailure = 5,
	trap_mib2_max
};

enum{
	trap_bridge_min = 0,
	trap_newRoot = 1,
	trap_topologyChange = 2,
	trap_bridge_max
};

enum{
	trap_cmcctrl_min = 0,
	trap_cmcArrival = 1,    
	trap_cmcReady = 2,
	trap_cmcHeartbeat = 3,
	trap_cmArrival = 4,
	trap_cmReady = 5,
	trap_cmDeparture = 6,
	trap_cmcctrl_max
};

enum {
	other_eventlog_min = 0,
	other_diagnosis_success = 1,
	other_diagnosis_fail = 2,
	other_onustp_topologychange,
	other_onustp_newroot,
	other_ctrlchan_success,		/* added by xieshl 20080421, ���ӿ���ͨ���жϼ�¼�澯��־ */
	other_ctrlchan_fail,
	other_reserve1,				/*other_pon_fw_ver_mismatch,*/
	other_reserve2,				/*other_pon_fw_ver_match,*/
	other_pon_cni_ber,			/*other_pon_DBA_ver_mismatch,*/
	other_pon_cni_ber_clear,	/*other_pon_DBA_ver_match,*/
	other_pon_update_file,
	other_pon_slave_ok,			
	other_pon_slave_fail,
	other_snmp_reset_dev,
	other_snmp_reset_brd,
	other_snmp_reset_pon,
	other_snmp_save_config,
	other_snmp_erase_config,
	other_eventlog_max
};

typedef enum{
	ALM_LEV_NULL = 0,
	ALM_LEV_VITAL = 1,
	ALM_LEV_MAJOR,
	ALM_LEV_MINOR,
	ALM_LEV_WARN,
	ALM_LEV_CLEAR,
	ALM_LEV_MAX
} alarmLevel_t;


#define ALM_PRI_LOW	0	/* ����¼��Ҫ�澯��־*/ /* MSG_PRI_NORMAL */
#define ALM_PRI_HIGH	1	/* ��¼��Ҫ�澯��־ */ /* MSG_PRI_URGENT */
#define ALM_SYN_NO		0	/* ��ͬ�� */
#define ALM_SYN_INS		1	/* ����һ��ͬ����Ϣ */
#define ALM_SYN_DEL		2	/* �鲢һ��ͬ����Ϣ */

#define ALM_NVRAM_YES	1	/* �澯��־д��nvram */
#define ALM_NVRAM_NO	0	/* �澯��־��д��nvram */

typedef struct {
	UCHAR alarmId;			/* �澯��ʶ:����ӳ��澯���� */
	UCHAR alarmLevel;		/* �澯����:��5�������ڿ���ָʾ�� */
	UCHAR priority;			/* �������ȼ�:0�������ȼ���1�������ȼ� */
	UCHAR synFlag;			/* �澯ͬ����־:0����ͬ����1��ͬ����2���鲢 */
	UCHAR partnerId;		/* ���alarmId */
	UCHAR almSrcType;		/* �澯Դ�������ͣ����ڻ�ȡ�澯���� */
	CHAR *pEventLogDesc;	/* ���� */
} eventInfoMapTbl_t;

#define MAXLEN_EVENT_DESC	255
#define EVENTLOG_ENABLE		1
#define EVENTLOG_DISABLE	2


#if 0
typedef struct {
    ULONG eventLogIndex;						/* ��־���� */
    nvramLogDateAndTime_t eventLogTime;			/* ʱ�� */
    UCHAR eventLogDesc[MAXLEN_EVENT_DESC];		/* ���� */
}__attribute__((packed)) eventLogTbl_t;
#endif

#define NVRAM_GetYearOfDate(ulDate)  (((((ulDate) >> 16) > 2000) ? (((ulDate) >> 16) - 2000) : 06) & 0x3f)
#define NVRAM_GetMonthOfDate(ulDate) (((ulDate) >> 8) & 0x0f)
#define NVRAM_GetDayOfDate(ulDate)   ((ulDate) & 0x1f)
#define NVRAM_GetHourOfTime(ulTime)   (((ulTime) >> 16) & 0x1f)
#define NVRAM_GetMinuteOfTime(ulTime) (((ulTime) >> 8) & 0x3f)
#define NVRAM_GetSecondOfTime(ulTime) ((ulTime) & 0x3f)

extern LONG initEventLogTbl();
extern LONG eraseAllEventLog();
extern LONG eraseAllEventImportLog();
extern LONG eraseDevEventLog( ULONG devIdx );

extern LONG saveEventLog( UCHAR alarmType, UCHAR alarmId, alarmSrc_t *pAlarmSrc);

extern LONG getEventLogEnable( ULONG *pEnable );
extern LONG setEventLogEnable( ULONG enable );
extern LONG getEventLogLastIndex( ULONG *pIndex );
extern LONG getEventLogIndex( ULONG logIdx, ULONG *pLogIdx );
extern LONG getEventLogSysTime( ULONG logIdx, sysDateAndTime_t *pLogTime );
extern LONG getEventLogDesc( ULONG logIdx, UCHAR *pLogDesc );

extern LONG getFirstEventLogTblIndex ( ULONG *pLogIdx );
extern LONG getNextEventLogTblIndex (ULONG logIdx, ULONG *pNextLogIdx );
extern LONG checkEventLogTblIndex ( ULONG logIdx );

extern ULONG getEventPriority( ULONG alarmType, ULONG alarmId );
extern ULONG getEventAlarmLevel( ULONG alarmType, ULONG alarmId );
extern ULONG getEventSynFlag( ULONG alarmType, ULONG alarmId );
extern ULONG getPartnerTrapId( ULONG alarmType, ULONG trapId );

extern LONG showEventLogCmd( struct vty *vty, ULONG devIdx, ULONG level, ULONG alarmId,
	sysDateAndTime_t *pStartTime, sysDateAndTime_t *pEndTime );

#if 0
enum
{
    DEREG_REPORT_TIMEOUT = 1,		/* Timeout has expired since the last MPCP REPORT */
                            		/* frame received from the ONU */
    DEREG_OAM_LINK_DISCONNECTION,	/* ONU didn't reply to several OAM Information */
                            		/* frames, hence OAM link is disconnected */
    DEREG_HOST_REQUEST,           	/* Response to previous PAS_deregister_onu or */
                            		/* PAS_shutdown_onu API calls */
    DEREG_DOUBLE_REGISTRATION,    	/* ONU registered twice without deregistering */
                           		 	/* This is an error handling for extrem cases. */
                            		/* If the ONU should be registered, the following */
                            		/* REGISTER_REQ will cause a correct registration. */
    DEREG_TIMESTAMP_DRIFT,        	/* when a timestamp drift of 16nsec is identified by the HW */
    DEREG_UNKNOWN,                	/* unknown reason, (recovery after missing event) */
    DEREG_LAST_CODE
};
#endif

/* added by xieshl 20080424, �澯��־���� */
#define  ALM_FILTER_DEFAULT			0x00000000
#define  ALM_FILTER_ONU_ETH_LINK		0x00000001
#define  ALM_FILTER_ALL					(ALM_FILTER_ONU_ETH_LINK)

extern ULONG getEventLogDataFilter();
extern LONG setEventLogDataFilter( ULONG filter );
extern LONG addEventLogDataFilterIterm( ULONG filter_iterm );
extern LONG delEventLogDataFilterIterm( ULONG filter_iterm );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventLogh */
