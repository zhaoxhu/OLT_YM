#ifndef TRACE_PATH_LIB_H
#define TRACE_PATH_LIB_H

#ifdef __cplusplus
extern "C"{
#endif


#define TRACE_MAC_HASH_BUCKET		(8*1024)
#define TRACE_USERID_HASH_BUCKET		(4*1024)


#define USER_ID_CAPACITY_MAX			(64*1024)
#define USER_ID_CAPACITY_LOW_DEF		(16*1024)
#define USER_ID_CAPACITY_HIGH_DEF		(32*1024)

extern ULONG tracePathUserIdDefaultNum;
extern ULONG tracePathUserIdSupportNum;
#define USER_ID_SUPPORT_MAXNUM	tracePathUserIdSupportNum
#define USER_ID_DEFAULT_MAXNUM	tracePathUserIdDefaultNum


#define USER_TRACE_CASCADED_YES		1
#define USER_TRACE_CASCADED_NO		0

#define USER_TRACE_RESOLVING_WAIT	0
#define USER_TRACE_RESOLVED_OK		1
#define USER_TRACE_RESOLVING_NON		2

#define USER_TRACE_FLAG_DYNAMIC		1
#define USER_TRACE_FLAG_STATIC		2

#define USER_ID_MAXLEN				28
#define USER_MACADDR_LEN			6
#define USER_MACADDR_NUM			2

#define USER_MAC_SUPPORT_MAXNUM	(USER_ID_SUPPORT_MAXNUM * USER_MACADDR_NUM)

typedef struct {
	UCHAR userMac[USER_MACADDR_LEN];
	UCHAR oltBrdIdx;
	UCHAR oltPortIdx;
	UCHAR onuId;		/* ´Ó1¿ªÊ¼±àºÅ */
	UCHAR onuBrdIdx:3;
	UCHAR onuPortIdx:5;
	UCHAR swMacAddr[USER_MACADDR_LEN];
	UCHAR swPortIdx:7;
	/*UCHAR locFlag;*/
	UCHAR updateFlag:1;
	UCHAR resolvingTimer:6;
	UCHAR resolvingStatus:2;
}__attribute__((packed)) user_loc_t;


typedef struct user_mac_table_s {
	UCHAR userMac[USER_MACADDR_LEN];
	ULONG userIdHashIdx;
	struct user_mac_table_s *next;
}__attribute__((packed)) trace_mac_table_t;


typedef struct user_loc_table_s {
	UCHAR userId[USER_ID_MAXLEN];
	UCHAR userCurIdx;
	/*UCHAR userMacList[USER_MACADDR_NUM][USER_MACADDR_LEN];*/
	user_loc_t  userLocList[USER_MACADDR_NUM];
	struct user_loc_table_s *next;
}__attribute__((packed)) trace_userid_table_t;


#define TRACE_SNOOP_MSG_TYPE_ADD		201
#define TRACE_SNOOP_MSG_TYPE_DEL		202
#define TRACE_SNOOP_MSG_CODE_OAM	203
#define TRACE_SNOOP_MSG_CODE_SYNC	204
#define TRACE_SNOOP_MSG_CODE_BROADCAST	205
typedef struct{
	UCHAR  userId[USER_ID_MAXLEN];
	UCHAR  userMac[USER_MACADDR_LEN];
	UCHAR  oltBrdIdx;
	UCHAR  oltPortIdx;
} user_trace_snoop_t;

typedef struct{
    short int flag;
    short int svlan;
    short int cvlan;
    short int OltBrd;
    short int OltPort;
    short int OnuId;
    short int OnuBrd;
    short int OnuPort;
}trace_vlan_data_t;

extern ULONG tracePathMSemId;
extern ULONG traceSnoopQId;

extern ULONG trace_userid_num_get();
extern ULONG trace_userid_hash_idx( const UCHAR *pUserId );
extern LONG trace_userid_status_get( UCHAR *pUserId );
extern LONG trace_userid_tbl_insert( UCHAR *pUserId, UCHAR *pUserMac, UCHAR oltBrdIdx, UCHAR oltPortIdx );
extern LONG trace_userid_tbl_delete( UCHAR *pUserId );
extern LONG trace_userid_update_over( ULONG userid_hash_idx, UCHAR *pUserMac );
extern LONG trace_userid_location_get( UCHAR *pUserId, user_loc_t* pLoc );
extern LONG trace_userid_location_set( UCHAR *pUserId, UCHAR *pUserMac, user_loc_t* pNewLoc );
extern LONG trace_userid_counter_get( ULONG *pUserIdCount, ULONG *pResolvedCount, ULONG *pResolvingCount );

extern LONG trace_userid_tbl_first_get( trace_userid_table_t *pFirstTbl );
extern LONG trace_userid_tbl_next_get( UCHAR *pUserId, trace_userid_table_t *pNextTbl );
/*extern LONG trace_userid_tbl_aging( UCHAR *pUserId );*/

extern LONG trace_mac_location_get( UCHAR *pUserMac, user_loc_t* pLoc );
extern LONG trace_mac_userid_get( ULONG user_loc_hash_idx, UCHAR *pUserMac, UCHAR* pUserId );
extern LONG trace_mac_userid_location_get( ULONG userid_hash_idx, UCHAR *pUserMac, UCHAR* pUserId, user_loc_t* pLoc );
extern LONG trace_mac_status_get( UCHAR *pUserMac );
extern LONG trace_mac_tbl_first_get( trace_mac_table_t *pFirstTbl );
extern LONG trace_mac_tbl_next_get( UCHAR *pUserMac, trace_mac_table_t *pNextTbl );
extern LONG trace_mac_userid_tbl_delete( UCHAR *pUserId, UCHAR *pUserMac );

extern LONG trace_path_add_notify( user_trace_snoop_t *pSnoopMsg );
extern LONG trace_path_del_notify( UCHAR *pUserId, UCHAR *pUserMac );


/*#define MAC_ADDR_CPY(DA, SA)	VOS_MemCpy(DA,SA,USER_MACADDR_LEN)
#define MAC_ADDR_ZERO(DA)		VOS_MemZero(DA,USER_MACADDR_LEN)	*/
#define MAC_ADDR_CPY(DA, SA)	*(ULONG*)(DA) = *(ULONG*)(SA);	*(USHORT*) (&((DA)[4])) = *(USHORT*) (&((SA)[4]))
#define MAC_ADDR_ZERO(DA)		*(ULONG*)(DA) = 0;	*(USHORT*) (&((DA)[4])) = 0


#ifdef __cplusplus
}
#endif

#endif	/* TRACE_PATH_LIB_H */
