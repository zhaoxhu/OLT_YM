
#ifndef	TDM_COMM_H
#define	TDM_COMM_H

#ifdef	__cplusplus
extern "C"{
#endif

/*定义处理函数的指针类型*/
typedef	VOID (*FUNC_PRC_CALLBACK)( const char*, USHORT );

#define	TDM_DEBUG_INFO	0x01
#define	TDM_DEBUG_RECV_MSG	0x02
#define	TDM_DEBUG_RECV_RAW	0x04
#define	TDM_DEBUG_SEND_MSG	0x08
#define	TDM_DEBUG_SEND_RAW	0x10

#define	MAX_TDM_PDU_LEN	1490
#define	MAX_TDM_MSG_LEN	1500
#define	PDU_OFFSET_IN_MSG	14
#define	MIN_PDU_LEN		(60-PDU_OFFSET_IN_MSG)


#define	MAX_TDM_QUEUE_LEN	50
#define	MAX_COMM_TRY_NUM	6

#define	TASK_TDM_COMM_PRI		58
#define	TASK_TDM_COMM_NAME		"tdmComm"

/*定义TDM板间通信消息类型*/
enum{
	TDM_MSG_TYPE_DEVMAN = 0x10,
	TDM_MSG_TYPE_TDMCONF = 0x20,
	TDM_MSG_TYPE_EVENT = 0x30,
	TDM_MSG_TYPE_FILETRANS = 0x40,
	TDM_MSG_TYPE_TRACEDEB = 0x50
};

enum{
	MSG_SUBTYPE_GETONU = 0x20,
	MSG_SUBTYPE_GETPOT
};

enum{
	MSG_SUBTYPE_READVER = 0,
	MSG_SUBTYPE_LOADFPGA,
	MSG_SUBTYPE_LOADFPGA_DONE,
	MSG_SUBTYPE_RUN,
	MSG_SUBTYPE_QUERY,
	MSG_SUBTYPE_RESET,
	MGMT_MSG_SUBTYPE_LAST
};

/*begin:
added by wangxiaoyu 2008-01-10
event message subtype enumeration
end*/
enum{
	MSG_SUBTYPE_E1_ALM,
	MSG_SUBTYPE_E1_ALMCLR,
	MSG_SUBTYPE_VOICE_OOS,
	MSG_SUBTYPE_VOICE_IS,
	MSG_SUBTYPE_ALM_SIG_RESET,	/* added by xieshl 20080324 */
	
	MSG_SUBTYPE_ALM_MAX
};

enum{
	MSG_SUBTYPE_SET_REQ = 0,
	MSG_SUBTYPE_GET_REQ,
	MSG_SUBTYPE_GETNEXT_REQ,
	MSG_SUBTYPE_SETROW_REQ,
	MSG_SUBTYPE_SET_RES,
	MSG_SUBTYPE_GET_RES,
	MSG_SUBTYPE_GETNEXT_RES,
	MSG_SUBTYPE_SETROW_RES,

	/* add by chenfj */
	MSG_SUBTYPE_GET_ALLONU = 0x20,
	MSG_SUBTYPE_GET_ALLPOTS,
	MSG_SUBTYPE_E1_LVL_SET,
	MSG_SUBTYPE_E1_LVL_GET,
};

enum{
	MSG_SUBTYPE_UPDATE_REQ=0,
	MSG_SUBTYPE_UPDATE_RES,
	MSG_SUBTYPE_TRANS_REQ,
	MSG_SUBTYPE_TRANS_RES,
	MSG_SUBTYPE_TRANS_END_REQ,
	MSG_SUBTYPE_TRANS_END_RES,
	MSG_SUBTYPE_UPDATE_OK
};

enum{
	MSG_SUBTYPE_DEBUG_REQ=0,
	MSG_SUBTYPE_DEBUG_RES,
	MSG_SUBTYPE_DEBUG_SET_REQ,
	MSG_SUBTYPE_DEBUG_SET_RES,
	MSG_SUBTYPE_DEBUG_INFO
};

/*定义消息队列动作码*/
enum{
	TDM_MSG_SEND_CODE = 1,
	TDM_MSG_RECV_CODE,
	TDM_MSG_TIMER_CODE,
	TDM_MSG_RECV_TIMEOUT
};

/*定义API调用类型*/
enum{
	ENUM_CALLTYPE_SYN=1,
	ENUM_CALLTYPE_MSG,
	ENUM_CALLTYPE_FUNC,
	ENUM_CALLTYPE_NOACK
};

typedef	struct{
	ULONG	reserved;
	UCHAR	type;
	UCHAR	subType;
	USHORT	msgCode;
	char		msgData[MAX_TDM_PDU_LEN];
}__attribute__((packed)) tdm_pdu_t;

typedef	struct{
	char DA[6];
	char SA[6];
	USHORT type;
	tdm_pdu_t pdu;	
}__attribute__((packed)) tdm_comm_msg_t;

typedef	struct{
	ULONG	ulQueUsedCount;	/*当前待处理的消息数*/
	ULONG	ulQueFullCount;	/*因队列满而放弃的消息数*/
	ULONG	ulQueSendTotalCount;	/*已经发送的消息数*/
	ULONG	ulQueRecvTotalCount;	/*已经成功接收的消息数*/
	ULONG	lQueSendErrCount;		/*发送错误消息数*/
	ULONG	ulQueRecvTimeoutCount;	/*接收超时消息数*/
}tdm_msg_statistic_t;

/*FUNCTION DECLARATION*/

USHORT buildPduHead( tdm_pdu_t *ppdu,  int msgType, int msgSubType );

void tdm_recv_frame_handler( const void *, UINT );
FUNC_PRC_CALLBACK registRtnProcesser( int , int , FUNC_PRC_CALLBACK  );

STATUS initTdmComm( void );
void * tdmCommMsgAlloc( void );
void tdmCommMsgFree( char* pdu );
int tdmCommSendMsg( UCHAR callflag, ULONG callNotify, char* pSendData, USHORT sendDataLen, char** ppRecvMsgPtr, USHORT *pRecvMsgLen );

void setTdmDebugFlag( UCHAR );
void undoSetTdmDebugFlag( UCHAR );

#ifdef	__cplusplus
}
#endif

#endif

