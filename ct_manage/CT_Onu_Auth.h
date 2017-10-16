
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_Onu_AuthH
#define __INCCT_Onu_AuthH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CTC_AUTH_MODE_DISABLE		5
#define CTC_AUTH_MODE_ERROR		6

typedef struct __ctc_onu_auth_t {
	UCHAR authIdx;
	UCHAR authRowStatus;
	UCHAR authOnuIdx;
	UCHAR authOnuLlid;
	CTC_STACK_auth_loid_data_t loidData;
	struct __ctc_onu_auth_t *next;
} ctc_onu_auth_t;

typedef struct {
	UCHAR authIdx;
	UCHAR authRowStatus;
	UCHAR authOnuIdx;
	UCHAR authOnuLlid;
	CTC_STACK_auth_loid_data_t loidData;
} __attribute__ ((packed)) ctc_onu_auth_data_t;

typedef struct {
	UCHAR subCode;
	UCHAR result;
	UCHAR slotno;
	UCHAR portno;
	ctc_onu_auth_data_t authData;
} __attribute__ ((packed)) ctc_onu_auth_msg_t;

#define CTC_MSG_CODE_ONU_AUTH_MODE		1
#define CTC_MSG_CODE_ONU_AUTH_LOID		2
#define CTC_MSG_CODE_ONU_TX_POWER		3
#define GPON_MSG_CODE_ONU_AUTH_ENTRY    4

#define CTC_MSG_SUBCODE_AUTH_MODE_MAC			1
#define CTC_MSG_SUBCODE_AUTH_MODE_LOID			2
#define CTC_MSG_SUBCODE_AUTH_MODE_HYBRID		3
#define CTC_MSG_SUBCODE_AUTH_MODE_LOID_NO_PWD	4
#define CTC_MSG_SUBCODE_AUTH_MODE_HYB_NO_PWD	5
#define CTC_MSG_SUBCODE_AUTH_MODE_DISABLE		6

#define CTC_MSG_SUBCODE_AUTH_LOID_CREATE	1
#define CTC_MSG_SUBCODE_AUTH_LOID_ADD		2
#define CTC_MSG_SUBCODE_AUTH_LOID_DEL		3
#define CTC_MSG_SUBCODE_AUTH_LOID_REP		4
#define CTC_MSG_SUBCODE_AUTH_LOID_DEL_ALL   5

#define CTC_MSG_SUBCODE_ONU_TX_POWER_CTRL		1
#define CTC_MSG_SUBCODE_ALL_TX_POWER_DISABLE	2
#define CTC_MSG_SUBCODE_ALL_TX_POWER_ENABLE	3
#define CTC_MSG_SUBCODE_ONU_MAC_AGING			4
#define CTC_MSG_SUBCODE_ALL_MAC_AGING			5
#define CTC_MSG_SUBCODE_ONU_MAC_LIMIT			6

extern LONG onu_auth_sync_msg_send ( ULONG dstSlotNo, ULONG msgCode, ctc_onu_auth_msg_t *pSendMsg );

extern LONG mn_getCtcOnuAuthMode( ULONG brdIdx, ULONG portIdx, ULONG *pMode );
extern LONG mn_setCtcOnuAuthMode( ULONG brdIdx, ULONG portIdx, ULONG mode );
extern LONG mn_getFirstCtcOnuAuthModeIdx( ULONG *pBrdIdx, ULONG *pPortIdx );
extern LONG mn_getNextCtcOnuAuthModeIdx( ULONG brdIdx, ULONG portIdx, ULONG *pNextBrdIdx, ULONG *pNextPortIdx );

extern LONG mn_addCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd );
extern LONG mn_delCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid );
extern LONG mn_createCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx );
extern LONG mn_showCtcOnuAuthLoid( struct vty * vty, ULONG brdIdx, ULONG portIdx );
extern LONG mn_getFirstCtcOnuAuthLoidIdx( ULONG *pBrdIdx, ULONG *pPortIdx, ULONG *pLoidIdx );
extern LONG mn_getNextCtcOnuAuthLoidIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pNextBrdIdx, ULONG *pNextPortIdx, ULONG *pNextLoidIdx );
extern LONG mn_getCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd );
extern LONG mn_setCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid );
extern LONG mn_setCtcOnuAuthLoidPassword( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pPwd );
extern LONG mn_getCtcOnuAuthLoidOnuDevIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pOnuDevIdx );
extern LONG mn_getCtcOnuAuthLoidRowStatus( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pStatus );
extern LONG mn_setCtcOnuAuthLoidRowStatus( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG status );
extern int DeleteGponAuthBySn(int PonPortIdx ,const *SN );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_Onu_AuthH */
