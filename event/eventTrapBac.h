
/*----------------------------------------------------------------------------*/
#ifndef __INCeventTrapBach
#define __INCeventTrapBach

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* �澯ͬ�� */
#define TRAP_BAC_LIST_SIZE		1000		/* ֧�ָ澯ͬ��������� */
#define TRAP_BAC_ENABLE		1
#define TRAP_BAC_DISABLE	2

#define TRAP_FLAG_ALARM		1
#define TRAP_FLAG_CLEAR		2

#define TRAP_PDU_MAXLEN	255


typedef struct {
	ULONG	bacIdx;					/* ��־���� */
	UCHAR	alarmType;
	UCHAR	trapId;
	alarmSrc_t alarmSrc;						/* �澯Դ */
	USHORT	trapPduLen;
	UCHAR	trapPdu[TRAP_PDU_MAXLEN];/* trap pdu data */
	UCHAR	alarmClearFlag;					/* �澯������־ */
	sysDateAndTime_t alarmBeginTime;			/* �澯����ʱ�� */
	sysDateAndTime_t alarmEndTime;				/* �澯����ʱ�� */
}__attribute__((packed)) trapBackupList_t;


extern LONG saveTrapBacData( ULONG alarmType, ULONG trapId, ULONG *pVarlist, ULONG varNum, alarmSrc_t *pAlarmSrc );
extern LONG eraseTrapBacData();
extern LONG eraseTrapBacDataByDevIdx( ULONG devIdx );

extern LONG getTrapBacEnable( ulong_t *pEnable );
extern LONG setTrapBacEnable( ulong_t enable );
extern LONG getTrapBacFirstIndex ( ulong_t *pIdx );
extern LONG getTrapBacNextIndex (ulong_t idx, ulong_t *pNextIdx );
extern LONG checkTrapBacIndex ( ulong_t idx );
extern LONG getTrapBacBeginTime( ulong_t idx, sysDateAndTime_t *pAlarmTime );
extern LONG getTrapBacPduData( ulong_t idx, uchar_t *pSynData, ulong_t *pDataLen );
extern LONG getTrapBacClearFlag( ulong_t idx, ulong_t *pSynFlag );
extern LONG getTrapBacClearTime( ulong_t idx, sysDateAndTime_t *pClearTime );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventTrapBach */
