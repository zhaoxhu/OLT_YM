#ifndef __INCeventStatush
#define __INCeventStatush

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*----------------------------------------------------------------------------*/

/*typedef nvramEventLogData_t alm_status_t;*/
typedef struct {
	UCHAR  alarmType;
	UCHAR  alarmId;
	alarmSrc_t alarmSrc;
	nvramLogDateAndTime_t  alarmTime;
} __attribute__((packed)) alm_status_t;

#define ALM_STA_CLS_MSG_CODE_DEV		1

typedef struct {
	UCHAR  msgCode;
	UCHAR  reserver[3];
	ULONG  devIdx;
} __attribute__((packed)) alm_status_cls_msg_t;


#define EVENT_MSG_SUBCODE_OLT_ALARM_MASK		1
#define EVENT_MSG_SUBCODE_ONU_ALARM_MASK		2
#define EVENT_MSG_SUBCODE_ONUTYPE_ALARM_MASK	3
#define EVENT_MSG_SUBCODE_PON_ALARM_MASK		4
#define EVENT_MSG_SUBCODE_ETH_ALARM_MASK		5
#define EVENT_MSG_SUBCODE_ALARM_CLEAR			6
#define EVENT_MSG_SUBCODE_ONUSWITCH_ALARM_MASK	7
typedef  struct{
	UCHAR  subType;
	UCHAR  subCode;
	alarmSrc_t alarmSrc;
	ULONG  syncValue;
} __attribute__((packed)) eventSyncCfgMsg_t;


extern LONG getAlarmLevel( ULONG alm_id, ULONG *level );
extern LONG setAlarmLevel( ULONG alm_id, ULONG level );
extern LONG compAlmStatusSource( ULONG alarmType, ULONG trapId, alarmSrc_t *pSrc1, alarmSrc_t *pSrc2 );
extern LONG getDeviceAlarmTopLevel( const ULONG devIdx );
extern LONG getDeviceAllAlarmLevel( const ULONG devIdx, ULONG *pVital, ULONG *pMajor, ULONG *pMinor );
extern LONG getEthPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ethIdx );
extern LONG getPonPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ponIdx );

extern LONG getEthPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ethIdx );
extern LONG getPonPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ponIdx );

extern LONG almStatusSetDeviceAlarmMask( ULONG devIdx, ULONG mask );
extern LONG almStatusSetOnuTypeAlarmMask( ULONG onuType, ULONG mask );
extern LONG almStatusSetPonPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG portIdx, const ULONG mask );
extern LONG almStatusSetEthPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG portIdx, const ULONG mask );
extern LONG eventSync_setAlarmMask( eventSyncCfgMsg_t *pMsg );
extern LONG eventSync_setAlarmStatus( eventSyncCfgMsg_t *pCfgMsg );

extern LONG clearDevAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx );
extern LONG showCurrentAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx );


/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventStatush */
