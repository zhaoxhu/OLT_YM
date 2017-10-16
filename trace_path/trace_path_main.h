#ifndef TRACE_PATH_MAIN_H
#define TRACE_PATH_MAIN_H

#ifdef __cplusplus
extern "C"{
#endif


#define USER_ID_RESOLVING_TIMES	30
#define TRACE_PATH_TRAP_RATE_DEFAULT		5
#define TRACE_PATH_RESOLVE_RATE_DEFAULT	10
#define TRACE_PATH_AUTO_SYNC_TIME  (2*60*60)

#define TRACE_PATH_MSG_CODE_TIMER		501
#define TRACE_PATH_MSG_CODE_AUTO_SYNC_TIMER		502
#define TRACE_PATH_MSG_TYPE_RESOLVE_REQ	1
#define TRACE_PATH_MSG_TYPE_RESOLVE_RSP	2
#define TRACE_PATH_MSG_TYPE_SNOOP_NEW	3
#define TRACE_PATH_MSG_TYPE_OAM_REQ		4
#define TRACE_PATH_MSG_TYPE_OAM_RSP		5


typedef struct{
	UCHAR  msgType;
	UCHAR  userId[USER_ID_MAXLEN];
	/*UCHAR  userMac[USER_MACADDR_LEN];
	UCHAR  resolvingStatus;*/
	user_loc_t  userLocation;
}__attribute__((packed)) trace_path_sync_msg_t;

typedef unsigned char trace_path_mac[6];
typedef struct{
    UCHAR macNum;
    trace_path_mac mac[64];
}__attribute__((packed)) trace_path_mac_t;
trace_path_mac_t *g_trace_path_mac_wait[200];

typedef struct{
    UCHAR msgtype;
    UCHAR macnum;
    USHORT slotno;
    USHORT portno;
    trace_path_mac mac[64];
}__attribute__((packed)) trace_path_mac_sync_msg_t;
extern ULONG tracePathQId;


extern int trace_path_debug;
#define trace_path_debug_out  if( trace_path_debug ) sys_console_printf

extern CHAR *trace_path_mac_addr_2_str( UCHAR *pUserMac );


#ifdef __cplusplus
}
#endif

#endif
