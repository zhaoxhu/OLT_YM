#ifdef __cplusplus
extern "C"{
#endif

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"
#include "Cdp_pub.h"
#include "V2R1_product.h"
#pragma pack(1)
#include "trace_path_lib.h"
#include "trace_path_main.h"


typedef struct {
	USHORT	tpid;
	USHORT	tag_ctrl;	/* Tag control */
} dot1q_tag_t;


typedef struct {
	USHORT  frame_type;
	USHORT  version;
	USHORT  session_id;
	USHORT  payload_len;
	USHORT  ppp_auth_type;	
	UCHAR   ppp_auth_code;
	UCHAR   ppp_auth_id;		/*  */
	USHORT  ppp_auth_len;
	UCHAR   val_len;
	/*UCHAR   peer_id[] or name[];	*/
}__attribute__((packed)) pppoe_body_t;

typedef struct {
	UCHAR  dmac[6];
	UCHAR  smac[6];
	dot1q_tag_t  tag;
	pppoe_body_t body;
} __attribute__((packed)) pppoe_frm_t;

#define ETH_TYPE_PPPOE_SESSION	0x8864
#define PPP_AUTH_TYPE_PAP			0xc023
#define PPP_AUTH_TYPE_CHAP		0xc223

#define PPP_AUTH_CODE_REQUEST		1
#define PPP_AUTH_CODE_RESPONSE	2
#define PPP_AUTH_CODE_SUCCESS		3

#define PPP_PAYLOAD_MAXLEN		(USER_ID_MAXLEN * 2)
#define PPP_AUTH_DATA_MAXLEN		(PPP_PAYLOAD_MAXLEN - 2)
#define PPP_USER_ID_MAXLEN			USER_ID_MAXLEN



#define  TRACE_PATH_OAM_TYPE_REQ	200
#define  TRACE_PATH_OAM_TYPE_RSP	200
#define  TRACE_PATH_OAM_MAX_MAC 64
#if 0
typedef struct {
	UCHAR type;
	UCHAR result;
	UCHAR mode;
	UCHAR macNum;
	UCHAR userMac[USER_MACADDR_LEN];
	ULONG onuEthPort;
	UCHAR swFlag;
	UCHAR swMacAddr[USER_MACADDR_LEN];
	ULONG swPortIdx;
}__attribute__((packed)) trace_path_oam_pdu_t;
#else

typedef struct {
	UCHAR userMac[USER_MACADDR_LEN];
	ULONG onuEthPort;
	UCHAR swFlag;
	UCHAR swMacAddr[USER_MACADDR_LEN];
	ULONG swPortIdx;
}__attribute__((packed)) trace_path_oam_rsp_data_t;

typedef struct {
	UCHAR type;
	UCHAR result;
	UCHAR mode;
	UCHAR macNum;
	int   data[0];
}__attribute__((packed)) trace_path_oam_pdu_t;
#endif

typedef struct{
	GwOamMsgHeader_t oamHeader;
	trace_path_oam_pdu_t oamPdu;
	UCHAR reserved;
} __attribute__ ((packed)) trace_path_oam_t;

typedef struct{
	short int PonPortIdx;
	short int OnuIdx;
	short int oamLength;
	trace_path_oam_t oamContent;
} trace_path_oam_msg_t;

typedef struct{
	short int PonPortIdx;
	short int OnuIdx;
	short int payloadLength;
    unsigned char sessionid[8];
	trace_path_oam_pdu_t oamPdu;
} trace_path_oam_msg1_t;


extern LONG (*trace_path_pppoe_snoop_hookrtn)( int, int, unsigned char * );
extern LONG devsm_sys_is_switchhovering();
extern LONG trace_path_resolving_local( UCHAR *pUserId, UCHAR *pUserMac, UCHAR slotno, UCHAR portno );

ULONG traceOamBSemId = 0;
LONG tracePathOamFlag = 0;
user_loc_t userLocationRecvBuf;


ULONG pppoe_pap_counter = 0;
ULONG pppoe_chap_counter = 0;
ULONG pppoe_total_counter = 0;

ULONG traceSnoopQId = 0;
VOS_HANDLE traceSnoopTId = NULL;


LONG trace_path_pppoe_snoop_callback( int unit, int l2Port, unsigned char *packet )
{
	pppoe_frm_t * pFrm;
	dot1q_tag_t * pTag;
	pppoe_body_t *pBody;
	ULONG i;
	UCHAR *pVal;
	ULONG slotno, portno;
	user_trace_snoop_t snoopMsg;
		
	if( packet == NULL )
		return VOS_ERROR;

	pFrm = (pppoe_frm_t *)packet;

	pTag = (dot1q_tag_t *)&(pFrm->tag);
	for( i=0; i<=2; i++ )
	{
		if( pTag->tpid == ETH_TYPE_PPPOE_SESSION )
			break;
		pTag++;
	}

	pBody = (pppoe_body_t *)pTag;
	if( pBody->frame_type != ETH_TYPE_PPPOE_SESSION )
	{
		return 1;
	}

	pppoe_total_counter++;
	slotno = SWITCH_PORT_2_USER_SLOT( unit, l2Port );
	portno = SWITCH_PORT_2_USER_PORT( unit, l2Port );
	trace_path_debug_out( "TRACE-PATH:Rx PPPoE session frame port%d/%d %d\r\n", slotno, portno, pppoe_total_counter );

	if( pBody->payload_len > PPP_PAYLOAD_MAXLEN )
	{
		trace_path_debug_out( "TRACE-PATH:PPPoE frame length error %d\r\n", pBody->payload_len );
		return VOS_ERROR;
	}
	if( pBody->ppp_auth_len > PPP_AUTH_DATA_MAXLEN )
	{
		trace_path_debug_out( "TRACE-PATH:PPPoE auth data length error %d\r\n", pBody->ppp_auth_len );
		return VOS_ERROR;
	}
	
	if( pBody->ppp_auth_type == PPP_AUTH_TYPE_PAP )
	{
		if( pBody->ppp_auth_code != PPP_AUTH_CODE_REQUEST )	/* 暂时只处理上行帧 */
		{
			return VOS_OK;
		}

		pppoe_pap_counter++;
		trace_path_debug_out( "TRACE-PATH:Rx PPPoE PAP(%d) frame (peer-len=%d)\r\n", pppoe_pap_counter, pBody->val_len );

		i = pBody->val_len;
		if( i >= PPP_USER_ID_MAXLEN )
		{
			i = PPP_USER_ID_MAXLEN - 1;
		}

		pVal = (UCHAR *)(pBody + 1);
		/*pVal = (UCHAR *)(&(pBody->val_len) + 1);*/
		VOS_MemZero( &snoopMsg, sizeof(user_trace_snoop_t) );
#if 0
		VOS_StrnCpy( snoopMsg.userId, pVal, i );
#else
        VOS_Sprintf(snoopMsg.userId, "%02x%02x.%02x%02x.%02x%02x", pFrm->smac[0], pFrm->smac[1], pFrm->smac[2], 
        pFrm->smac[3], pFrm->smac[4], pFrm->smac[5]);
#endif
		MAC_ADDR_CPY( snoopMsg.userMac, pFrm->smac );
		snoopMsg.oltBrdIdx = slotno;
		snoopMsg.oltPortIdx = portno;

		trace_path_add_notify( &snoopMsg );
	}
	else if( pBody->ppp_auth_type == PPP_AUTH_TYPE_CHAP )
	{
		if( pBody->ppp_auth_code != PPP_AUTH_CODE_RESPONSE )	/* 暂时只处理上行帧 */
		{
			return VOS_OK;
		}

		pppoe_chap_counter++;
		trace_path_debug_out( "TRACE-PATH:Rx PPPoE CHAP(%d) frame (data-val len=%d)\r\n", pppoe_chap_counter, pBody->val_len );
#if 0
		i = pBody->val_len;
		if( i > 256/*PPP_USER_ID_MAXLEN*/ )
		{
			return VOS_ERROR;
		}

#endif
		pVal = (UCHAR *)(pBody + 1);
		/*pVal = (UCHAR *)(&(pBody->val_len) + 1);*/
		pVal += i;
		VOS_MemZero( &snoopMsg, sizeof(user_trace_snoop_t) );
#if 0
		VOS_StrnCpy( snoopMsg.userId, pVal, PPP_USER_ID_MAXLEN -2 );
#else
        VOS_Sprintf(snoopMsg.userId, "%02x%02x.%02x%02x.%02x%02x", pFrm->smac[0], pFrm->smac[1], pFrm->smac[2], 
        pFrm->smac[3], pFrm->smac[4], pFrm->smac[5]);
#endif
		MAC_ADDR_CPY( snoopMsg.userMac, pFrm->smac );
		snoopMsg.oltBrdIdx = slotno;
		snoopMsg.oltPortIdx = portno;

		trace_path_add_notify( &snoopMsg );
	}
	else
	{
		trace_path_debug_out( "TRACE-PATH:PPPoE auth type=%d ERR\r\n", pBody->ppp_auth_type );
		return VOS_ERROR;
	}

	return VOS_OK;
}


/*  向ONU 广播trace OAM 消息*/
extern int BroadcastOamFrameToOnu( short int PonPortIdx, int length, unsigned char *content );
LONG trace_path_req_oam_broadcast( ULONG slotno, ULONG portno, ULONG hash_idx, UCHAR *pUserMac )
{
    UCHAR send_buf[1024]={0};
    int   send_len = 0;
	trace_path_oam_t  *txOam;
	unsigned char OamMsgDstMac[BYTES_IN_MAC_ADDRESS] = {0x01,0x80,0xc2,0x00,0x00,0x02};
	unsigned char OamMsgSrcMac[BYTES_IN_MAC_ADDRESS] = {0x00,0x0f,0xe9,0x00,0x00,0x00};
	SHORT PonPortIdx;
    short int payloadlen = 0;
	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return VOS_ERROR;

	PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
	
	if( !OLT_LOCAL_ISVALID(PonPortIdx) )
		return VOS_ERROR;
	if( !PonPortIsWorking(PonPortIdx) )
		return VOS_ERROR;

	trace_path_debug_out( "TRACE-PATH:pon%d/%d broadcast mac %s oam REQ\r\n", slotno, portno, trace_path_mac_addr_2_str(pUserMac) );

    txOam = (trace_path_oam_t  *)send_buf;
    payloadlen = 6+5;
	/*VOS_MemZero( &txOam, sizeof(trace_path_oam_t));*/
	VOS_MemCpy( txOam->oamHeader.DA, OamMsgDstMac, 6 );
	VOS_MemCpy( txOam->oamHeader.SA, OamMsgSrcMac, 6 );
	txOam->oamHeader.Type = 0x8809;
	txOam->oamHeader.SubType = 0x03;
	txOam->oamHeader.Flag = 0x0050;
	txOam->oamHeader.Code = 0xfe;
	
	txOam->oamHeader.OUI[0] = 0x00;
	txOam->oamHeader.OUI[1] = 0x0f;
	txOam->oamHeader.OUI[2] = 0xe9;
	txOam->oamHeader.GwOpcode = GW_OPCODE_EUQ_INFO_REQUESET;
	txOam->oamHeader.SendSerNo = 0;
	txOam->oamHeader.WholePktLen = payloadlen/*sizeof(trace_path_oam_t)*/;
	txOam->oamHeader.PayLoadOffset = 0;
	txOam->oamHeader.PayLoadLength = payloadlen/*sizeof(trace_path_oam_t)*/;
	VOS_MemCpy( txOam->oamHeader.SessionID, &hash_idx, sizeof(int) );
	txOam->oamPdu.type = TRACE_PATH_OAM_TYPE_REQ;
	txOam->oamPdu.mode = 1;
	txOam->oamPdu.macNum = 1;
	VOS_MemCpy(txOam->oamPdu.data, pUserMac, 6);
	send_len = sizeof(GwOamMsgHeader_t) + payloadlen;
    if(send_len<64)
        send_len = 64;
    
	BroadcastOamFrameToOnu( PonPortIdx, send_len, send_buf);

	return VOS_OK;
}

LONG trace_path_req_oam_broadcast_perpon( short int PonPortIdx, UCHAR mac_num, UCHAR *pUserMac )
{
    UCHAR send_buf[1024] = {0};
    int   send_len = 0; 
	trace_path_oam_t  *txOam = (trace_path_oam_t  *)send_buf;
	unsigned char OamMsgDstMac[BYTES_IN_MAC_ADDRESS] = {0x01,0x80,0xc2,0x00,0x00,0x02};
	unsigned char OamMsgSrcMac[BYTES_IN_MAC_ADDRESS] = {0x00,0x0f,0xe9,0x00,0x00,0x00};
	short int slotno = GetCardIdxByPonChip(PonPortIdx);
    short int portno = GetPonPortByPonChip(PonPortIdx);
    short int payloadlen = 0;
    
	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return VOS_ERROR;
	
	if( !OLT_LOCAL_ISVALID(PonPortIdx) )
		return VOS_ERROR;
	if( !PonPortIsWorking(PonPortIdx) )
		return VOS_ERROR;

	trace_path_debug_out( "TRACE-PATH:pon%d/%d broadcast %d mac oam REQ\r\n", slotno, portno, mac_num);
    if(mac_num>64)
        mac_num=64;
    payloadlen = 6*mac_num+5;
	VOS_MemCpy( txOam->oamHeader.DA, OamMsgDstMac, 6 );
	VOS_MemCpy( txOam->oamHeader.SA, OamMsgSrcMac, 6 );
	txOam->oamHeader.Type = 0x8809;
	txOam->oamHeader.SubType = 0x03;
	txOam->oamHeader.Flag = 0x0050;
	txOam->oamHeader.Code = 0xfe;
	
	txOam->oamHeader.OUI[0] = 0x00;
	txOam->oamHeader.OUI[1] = 0x0f;
	txOam->oamHeader.OUI[2] = 0xe9;
	txOam->oamHeader.GwOpcode = GW_OPCODE_EUQ_INFO_REQUESET;
	txOam->oamHeader.SendSerNo = 0;
	txOam->oamHeader.WholePktLen = payloadlen;
	txOam->oamHeader.PayLoadOffset = 0;
	txOam->oamHeader.PayLoadLength = payloadlen;
    
	txOam->oamPdu.type = TRACE_PATH_OAM_TYPE_REQ;
	txOam->oamPdu.mode = 1;

	txOam->oamPdu.macNum = mac_num;
	VOS_MemCpy( txOam->oamPdu.data, pUserMac, 6*mac_num);
	send_len = sizeof(GwOamMsgHeader_t)+payloadlen;
    if(send_len<64)
        send_len = 64;
	BroadcastOamFrameToOnu( PonPortIdx, send_len, send_buf);

	return VOS_OK;
}

int  ReceivedFrameIsGwOAM_UserTrace_Rsp( unsigned char *content )
{
	trace_path_oam_t  *pRxOam;
	
	if( content == NULL )
		return (FALSE );
	
	pRxOam = (trace_path_oam_t *)content;
	if((pRxOam->oamHeader.GwOpcode == GW_OPCODE_EUQ_INFO_RESPONSE) && (pRxOam->oamPdu.type == TRACE_PATH_OAM_TYPE_REQ))
		return(TRUE);

	return(FALSE);
}
int  ReceivedFrameIsGwOAM_UserTrace_Rsp1( unsigned char *content )
{
	trace_path_oam_pdu_t  *pRxOam;
	
	if( content == NULL )
		return (FALSE );
	
	pRxOam = (trace_path_oam_pdu_t *)content;
	if((pRxOam->type == TRACE_PATH_OAM_TYPE_REQ))
		return(TRUE);

	return(FALSE);
}
int GwOamMsg_UserTrace_Handler(short int PonPortIdx, short int OnuIdx, short int Length, unsigned char *content)
{
	int rc = VOS_ERROR;
	ULONG aulMsg[4] = {RPU_TID_CDP_USERTRACE, 0, 0, 0};
	trace_path_oam_msg_t *pOamMsg;
	SYS_MSG_S * pstMsg = NULL;
    UCHAR *pMac = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + /*sizeof(trace_path_oam_msg_t)*/Length+3*sizeof(short int);

	if((Length == 0) ||(content == NULL )) 
		return rc;

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( msgLen, RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return VOS_ERROR;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_OAM;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
	pstMsg->ptrMsgBody = (VOID *)(pstMsg+1);
	pstMsg->usFrameLen = sizeof(trace_path_oam_msg_t);

	pOamMsg = (trace_path_oam_msg_t *)pstMsg->ptrMsgBody;

	pOamMsg->PonPortIdx = PonPortIdx;
	pOamMsg->OnuIdx = OnuIdx;
	pOamMsg->oamLength = Length;
	VOS_MemCpy( &(pOamMsg->oamContent), content, /*sizeof(trace_path_oam_t) */Length);
	aulMsg[3] = (ULONG)pstMsg;
	
	if( (rc = VOS_QueSend(traceSnoopQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL)) != VOS_OK )
	{
        VOS_Free(pMac);
		VOS_Free( pOamMsg );
	}
	
	return rc;
}

int GwOamMsg_UserTrace_Handler1(short int PonPortIdx, short int OnuIdx, unsigned char *sessionid, short int Length, unsigned char *content)
{
	int rc = VOS_ERROR;
	ULONG aulMsg[4] = {RPU_TID_CDP_USERTRACE, 0, 0, 0};
	trace_path_oam_msg1_t *pOamMsg;
	SYS_MSG_S * pstMsg = NULL;
    UCHAR *pMac = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + /*sizeof(trace_path_oam_msg_t)*/Length+3*sizeof(short int);

	if((Length == 0) ||(content == NULL )) 
		return rc;

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( msgLen, RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return VOS_ERROR;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_OAM;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
	pstMsg->ptrMsgBody = (VOID *)(pstMsg+1);
	pstMsg->usFrameLen = sizeof(trace_path_oam_msg1_t);

	pOamMsg = (trace_path_oam_msg1_t *)pstMsg->ptrMsgBody;

	pOamMsg->PonPortIdx = PonPortIdx;
	pOamMsg->OnuIdx = OnuIdx;
	pOamMsg->payloadLength = Length;
    VOS_MemCpy(pOamMsg->sessionid, sessionid, 8);
	VOS_MemCpy( &(pOamMsg->oamPdu), content, /*sizeof(trace_path_oam_t) */Length);
	aulMsg[3] = (ULONG)pstMsg;
	
	if( (rc = VOS_QueSend(traceSnoopQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL)) != VOS_OK )
	{
        VOS_Free(pMac);
		VOS_Free( pOamMsg );
	}
	
	return rc;
}
LONG trace_path_oam_req_cdp_recv(trace_path_sync_msg_t  *pSyncMsg)
{
	if( pSyncMsg == NULL )
		return VOS_ERROR;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( pSyncMsg->userLocation.oltBrdIdx != SYS_LOCAL_MODULE_SLOTNO )
		return VOS_OK;
	
	trace_path_debug_out( "TRACE-PATH:pon%d/%d recv mac %s oam REQ\r\n", pSyncMsg->userLocation.oltBrdIdx, pSyncMsg->userLocation.oltPortIdx,
		trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac) );

	trace_path_req_oam_broadcast( pSyncMsg->userLocation.oltBrdIdx, pSyncMsg->userLocation.oltPortIdx, TRACE_USERID_HASH_BUCKET, pSyncMsg->userLocation.userMac );

	return VOS_OK;
}

LONG trace_path_oam_rsp_cdp_recv(trace_path_sync_msg_t  *pSyncMsg)
{
	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return VOS_OK;
	}

	if( pSyncMsg->userLocation.oltBrdIdx == SYS_LOCAL_MODULE_SLOTNO )
		return VOS_OK;
	
	if( tracePathOamFlag )
	{
		VOS_MemCpy( &userLocationRecvBuf, &(pSyncMsg->userLocation), sizeof(user_loc_t) );
		VOS_SemGive(traceOamBSemId);
	}
	trace_path_debug_out( "TRACE-PATH:recv mac %s oam RSP\r\n", trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac) );

	return VOS_OK;
}
LONG trace_path_resolving_req_cdp_recv(trace_path_sync_msg_t  *pSyncMsg)
{
	/*trace_path_sync_msg_t  sendMsg;*/
	ULONG status;

	if( pSyncMsg == NULL )
		return VOS_ERROR;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( pSyncMsg->userLocation.oltBrdIdx != SYS_LOCAL_MODULE_SLOTNO )
		return VOS_OK;
	
	trace_path_debug_out( "TRACE-PATH:recv mac %s resolving REQ\r\n", trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac) );

	status = pSyncMsg->userLocation.resolvingStatus;
	if( status == USER_TRACE_RESOLVED_OK )
	{
		trace_userid_location_set( pSyncMsg->userId, pSyncMsg->userLocation.userMac, &(pSyncMsg->userLocation) );
	}
	else
	{
		trace_userid_tbl_insert( pSyncMsg->userId, pSyncMsg->userLocation.userMac, pSyncMsg->userLocation.oltBrdIdx, pSyncMsg->userLocation.oltPortIdx );
		trace_path_resolving_local( pSyncMsg->userId, pSyncMsg->userLocation.userMac, pSyncMsg->userLocation.oltBrdIdx, pSyncMsg->userLocation.oltPortIdx );
	}

	return VOS_OK;
}

LONG trace_path_resolving_rsp_cdp_send( trace_path_sync_msg_t  *pSyncMsg )
{
	LONG rc = VOS_ERROR;
	ULONG to_slotno = SYS_MASTER_ACTIVE_SLOTNO;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(trace_path_sync_msg_t);
	SYS_MSG_S * pstMsg = NULL;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return rc;
	}
	if( pSyncMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( (pSyncMsg->userLocation.oltBrdIdx == 0) || (pSyncMsg->userLocation.oltBrdIdx == to_slotno) )
		return rc;

	if( !(SYS_MODULE_IS_READY(to_slotno) || devsm_sys_is_switchhovering()) )
		return rc;

	pstMsg = ( SYS_MSG_S * ) CDP_AllocMsg( msgLen, MODULE_RPU_TRACEPATH );              
	if(pstMsg == NULL)
	{
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulDstModuleID = MODULE_RPU_TRACEPATH;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = to_slotno;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)(pstMsg + 1);
	pstMsg->usFrameLen = sizeof(trace_path_sync_msg_t);
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_CODE_SYNC;
	
	VOS_MemCpy( (VOID *)(pstMsg + 1), pSyncMsg, sizeof(trace_path_sync_msg_t) );

	trace_path_debug_out( "TRACE-PATH:sync new mac %s to slot %d\r\n", trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac), to_slotno );

	rc = CDP_Send( RPU_TID_CDP_USERTRACE, to_slotno,  RPU_TID_CDP_USERTRACE,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_TRACEPATH );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
		VOS_ASSERT(0);
	}

	return rc;
}

LONG trace_path_resolving_rsp_cdp_recv(trace_path_sync_msg_t  *pSyncMsg)
{
	LONG status;
    trace_mac_table_t *mac_location = NULL;
	UCHAR user_id[USER_ID_MAXLEN];

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return VOS_OK;
	}

	if( pSyncMsg->userLocation.oltBrdIdx == SYS_LOCAL_MODULE_SLOTNO )
		return VOS_OK;
	
	/*status = trace_userid_status_get(pSyncMsg->userId);*/
	status = trace_mac_status_get( pSyncMsg->userLocation.userMac );
	
	if( status == USER_TRACE_RESOLVED_OK )
	{
#if 1        
       mac_location = trace_mac_tbl_search(pSyncMsg->userLocation.userMac);
       if(mac_location)
       {
            if(trace_mac_userid_get(mac_location->userIdHashIdx, pSyncMsg->userLocation.userMac, user_id) == VOS_OK)
        		trace_userid_location_set(user_id, pSyncMsg->userLocation.userMac, &(pSyncMsg->userLocation) );
       }        
#else
	   trace_userid_location_set( pSyncMsg->userId, pSyncMsg->userLocation.userMac, &(pSyncMsg->userLocation) );
#endif
	}
	else if( status == USER_TRACE_RESOLVING_WAIT )
	{
#if 1        
       mac_location = trace_mac_tbl_search(pSyncMsg->userLocation.userMac);
       if(mac_location)
       {
            if(trace_mac_userid_get(mac_location->userIdHashIdx, pSyncMsg->userLocation.userMac, user_id) == VOS_OK)
        		trace_userid_location_set(user_id, pSyncMsg->userLocation.userMac, &(pSyncMsg->userLocation) );
       }        
#else
	   trace_userid_location_set( pSyncMsg->userId, pSyncMsg->userLocation.userMac, &(pSyncMsg->userLocation) );
#endif
	}
	else
	{
        if(pSyncMsg->msgType == TRACE_PATH_MSG_TYPE_SNOOP_NEW)
    		trace_userid_tbl_insert( pSyncMsg->userId, pSyncMsg->userLocation.userMac, pSyncMsg->userLocation.oltBrdIdx, pSyncMsg->userLocation.oltPortIdx );
	}
	
	trace_path_debug_out( "TRACE-PATH:recv mac %s new join (%d)\r\n", trace_path_mac_addr_2_str(pSyncMsg->userLocation.userMac), status );

	return VOS_OK;
}



LONG trace_path_add_notify( user_trace_snoop_t *pSnoopMsg )
{
	ULONG aulMsg[4] = {MODULE_RPU_TRACEPATH, 0, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(user_trace_snoop_t);

	if( pSnoopMsg == NULL )
		return VOS_ERROR;

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( msgLen, RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return VOS_ERROR;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_TYPE_ADD;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
	pstMsg->ptrMsgBody = (VOID *)(pstMsg+1);
	pstMsg->usFrameLen = sizeof(user_trace_snoop_t);

	VOS_MemCpy( pstMsg->ptrMsgBody, pSnoopMsg, pstMsg->usFrameLen );
	
	aulMsg[3] = (ULONG)pstMsg;

	trace_path_debug_out( "TRACE-PATH:add %s %s notify\r\n", pSnoopMsg->userId, trace_path_mac_addr_2_str(pSnoopMsg->userMac) );

	if( VOS_QueSend( traceSnoopQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_Free( pstMsg );
		return VOS_ERROR;
	}
	
	return VOS_OK;
}

LONG trace_path_del_notify( UCHAR *pUserId, UCHAR *pUserMac )
{
	ULONG aulMsg[4] = {MODULE_RPU_TRACEPATH, 0, 0, 0};
	user_trace_snoop_t *pSnoopMsg;
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(user_trace_snoop_t);

	if( (pUserId == NULL) || (pUserMac == NULL) )
		return VOS_ERROR;

	trace_path_debug_out( "TRACE-PATH:del %s %s notify\r\n", pUserId, trace_path_mac_addr_2_str(pUserMac) );

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( msgLen, RPU_TID_CDP_USERTRACE );
	if ( NULL == pstMsg )
	{
		return VOS_ERROR;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulDstModuleID = RPU_TID_CDP_USERTRACE;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = TRACE_SNOOP_MSG_TYPE_DEL;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
	pstMsg->ptrMsgBody = (VOID *)(pstMsg+1);
	pstMsg->usFrameLen = sizeof(user_trace_snoop_t);

	pSnoopMsg = (user_trace_snoop_t *)pstMsg->ptrMsgBody;
	
	VOS_StrnCpy( pSnoopMsg->userId, pUserId, USER_ID_MAXLEN );
	VOS_MemCpy( pSnoopMsg->userMac, pUserMac, USER_MACADDR_LEN );
	
	aulMsg[3] = (ULONG)pstMsg;

	if( VOS_QueSend( traceSnoopQId, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_Free( pstMsg );
		return VOS_ERROR;
	}
	
	return VOS_OK;
}


VOID trace_path_add_notify_proc( user_trace_snoop_t *pUserSnoop )
{
	trace_path_sync_msg_t  syncMsg;
	
	if( pUserSnoop == NULL )
		return;

	if( trace_userid_tbl_insert(pUserSnoop->userId, pUserSnoop->userMac, pUserSnoop->oltBrdIdx, pUserSnoop->oltPortIdx) == VOS_OK )
	{
		if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		{
			VOS_MemZero( &syncMsg, sizeof(syncMsg) );

			syncMsg.msgType = TRACE_PATH_MSG_TYPE_SNOOP_NEW;
			VOS_StrnCpy( syncMsg.userId, pUserSnoop->userId, USER_ID_MAXLEN );
			MAC_ADDR_CPY( syncMsg.userLocation.userMac, pUserSnoop->userMac );
			syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVING_WAIT;
			syncMsg.userLocation.oltBrdIdx = pUserSnoop->oltBrdIdx;
			syncMsg.userLocation.oltPortIdx = pUserSnoop->oltPortIdx;
			/*syncMsg.userLocation.locFlag = USER_TRACE_FLAG_DYNAMIC;*/

			trace_path_resolving_rsp_cdp_send( &syncMsg );
		}
	}
}

VOID trace_path_del_notify_proc( user_trace_snoop_t *pUserSnoop )
{
	if( pUserSnoop == NULL )
		return;

	/*trace_userid_tbl_delete( pUserSnoop->userId );*/
	trace_mac_userid_tbl_delete( pUserSnoop->userId, pUserSnoop->userMac );
}

LONG trace_path_oam_recv( trace_path_oam_msg_t *pOamMsg )
{
	LONG rc = VOS_ERROR;
	short int PonPortIdx, OnuIdx;
	int loop_mac = 0;
	trace_path_oam_t  *pRxOam;
	ULONG hash_idx = 0;
	trace_path_sync_msg_t  syncMsg;
	UCHAR user_id[USER_ID_MAXLEN];
	ULONG slotno, portno;
    trace_path_oam_rsp_data_t *data;
	if( pOamMsg == NULL ) 
		return rc;
	
	pRxOam = (trace_path_oam_t *)&(pOamMsg->oamContent);
	PonPortIdx = pOamMsg->PonPortIdx;
	OnuIdx = pOamMsg->OnuIdx;

	if( (pOamMsg->oamLength == 0) || (pRxOam == NULL) )
		return rc;    
    data = (trace_path_oam_rsp_data_t *)pRxOam->oamPdu.data;
   
	slotno = GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);
	trace_path_debug_out("TRACE-PATH:Rx user trace oam from onu%d/%d/%d\r\n", slotno, portno, OnuIdx+1 );

	if((pRxOam->oamHeader.GwOpcode == GW_OPCODE_EUQ_INFO_RESPONSE) && 
		(pRxOam->oamPdu.type == TRACE_PATH_OAM_TYPE_REQ) &&
		(pRxOam->oamPdu.result == 1) )
	{
        for(loop_mac=0;loop_mac<pRxOam->oamPdu.macNum;loop_mac++)
        {                        
            trace_mac_table_t *mac_location = NULL;
            if(data == NULL)
                continue;
            
    		VOS_MemZero( &syncMsg, sizeof(syncMsg) );
    		syncMsg.userLocation.oltBrdIdx = slotno;
    		syncMsg.userLocation.oltPortIdx = portno;
    		syncMsg.userLocation.onuId = OnuIdx+1;
    		syncMsg.userLocation.onuBrdIdx = (data->onuEthPort >> 8) & 0xff;
    		syncMsg.userLocation.onuPortIdx = (data->onuEthPort & 0xff);
    		/*syncMsg.userLocation.locFlag = USER_TRACE_FLAG_DYNAMIC;*/
    		trace_path_debug_out("  mac num=%d,mac1=%s,eth%d/%d\r\n", pRxOam->oamPdu.macNum, 
    					trace_path_mac_addr_2_str(data->userMac),
    					syncMsg.userLocation.onuBrdIdx, syncMsg.userLocation.onuPortIdx );
    		if( data->swFlag )
    		{
    			MAC_ADDR_CPY( (syncMsg.userLocation.swMacAddr), (data->swMacAddr) );
    			syncMsg.userLocation.swPortIdx = data->swPortIdx;

    			trace_path_debug_out("  switch/port=%s /%d\r\n",
    					trace_path_mac_addr_2_str(data->swMacAddr), data->swPortIdx);
    		}
#if 0
    		VOS_MemCpy( &hash_idx, pRxOam->oamHeader.SessionID, sizeof(ULONG) );

    		if( hash_idx >= TRACE_USERID_HASH_BUCKET )
    		{
    			syncMsg.msgType = TRACE_PATH_MSG_TYPE_OAM_RSP;
    			/*VOS_StrnCpy( syncMsg.userId, pRxOam->oamHeader.SessionID, 8 );*/
    			MAC_ADDR_CPY( syncMsg.userLocation.userMac, pRxOam->oamPdu.userMac );
    			syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK;

    			if( tracePathOamFlag )
    			{
    				tracePathOamFlag = 0;
    				VOS_MemCpy( &userLocationRecvBuf, &(syncMsg.userLocation), sizeof(user_loc_t) );
    				VOS_SemGive(traceOamBSemId);
    			}

    			if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
    			{
    				/*VOS_MemCpy( &userLocationRecvBuf, &(syncMsg.userLocation), sizeof(user_loc_t) );
    				if( tracePathOamFlag )
    					VOS_SemGive(traceOamBSemId);*/
    			}
    			else
    			{
    				trace_path_resolving_rsp_cdp_send( &syncMsg );
    			}
    			return VOS_OK;
    		}
#endif
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
                mac_location = trace_mac_tbl_search(data->userMac);
                if(mac_location)
                {
            		if( (rc = trace_mac_userid_get(mac_location->userIdHashIdx, data->userMac, user_id)) == VOS_ERROR )
            		{
            			trace_path_debug_out("TRACE-PATH: hash idx=%d ERR\r\n", mac_location->userIdHashIdx );
            		}
            		else
            		{
                        syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK; 	
                       
            			if( trace_userid_location_set(user_id, data->userMac, &(syncMsg.userLocation)) == VOS_OK )
            			{	      
#if 0                            
                            syncMsg.msgType = TRACE_PATH_MSG_TYPE_RESOLVE_RSP;
                			VOS_StrnCpy( syncMsg.userId, user_id, USER_ID_MAXLEN );
                			MAC_ADDR_CPY( syncMsg.userLocation.userMac, data->userMac);
            			
            				trace_path_resolving_rsp_cdp_send( &syncMsg );
#endif                            
            			}
            		}
                }
            }
            else
            {
                syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK; 	                
                syncMsg.msgType = TRACE_PATH_MSG_TYPE_RESOLVE_RSP;
    			MAC_ADDR_CPY( syncMsg.userLocation.userMac, data->userMac);                
				trace_path_resolving_rsp_cdp_send( &syncMsg );
            }
            data += 1;
	    }
	}
    
	return rc;
}
LONG trace_path_oam_recv1( trace_path_oam_msg1_t *pOamMsg )
{
	LONG rc = VOS_ERROR;
	short int PonPortIdx, OnuIdx;
	int loop_mac = 0;
	ULONG hash_idx = 0;
	trace_path_sync_msg_t  syncMsg;
    trace_path_oam_pdu_t *pRxOam;
	UCHAR user_id[USER_ID_MAXLEN];
	ULONG slotno, portno;
    trace_path_oam_rsp_data_t *data;
	if( pOamMsg == NULL ) 
		return rc;
	
	PonPortIdx = pOamMsg->PonPortIdx;
	OnuIdx = pOamMsg->OnuIdx;
    pRxOam = &(pOamMsg->oamPdu);
	if( (pOamMsg->payloadLength == 0) || (pRxOam == NULL) )
		return rc;    
    data = (trace_path_oam_rsp_data_t *)pRxOam->data;
   
	slotno = GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);
	trace_path_debug_out("TRACE-PATH:Rx user trace oam from onu%d/%d/%d\r\n", slotno, portno, OnuIdx+1 );

	if(pRxOam->result == 1)
	{
        for(loop_mac=0;loop_mac<pRxOam->macNum;loop_mac++)
        {                        
            trace_mac_table_t *mac_location = NULL;
            if(data == NULL)
                continue;
            
    		VOS_MemZero( &syncMsg, sizeof(syncMsg) );
    		syncMsg.userLocation.oltBrdIdx = slotno;
    		syncMsg.userLocation.oltPortIdx = portno;
    		syncMsg.userLocation.onuId = OnuIdx+1;
    		syncMsg.userLocation.onuBrdIdx = (data->onuEthPort >> 8) & 0xff;
    		syncMsg.userLocation.onuPortIdx = (data->onuEthPort & 0xff);
    		/*syncMsg.userLocation.locFlag = USER_TRACE_FLAG_DYNAMIC;*/
    		trace_path_debug_out("  mac num=%d,mac1=%s,eth%d/%d\r\n", pRxOam->macNum, 
    					trace_path_mac_addr_2_str(data->userMac),
    					syncMsg.userLocation.onuBrdIdx, syncMsg.userLocation.onuPortIdx );
    		if( data->swFlag )
    		{
    			MAC_ADDR_CPY( (syncMsg.userLocation.swMacAddr), (data->swMacAddr) );
    			syncMsg.userLocation.swPortIdx = data->swPortIdx;

    			trace_path_debug_out("  switch/port=%s /%d\r\n",
    					trace_path_mac_addr_2_str(data->swMacAddr), data->swPortIdx);
    		}
#if 1
    		VOS_MemCpy( &hash_idx, pOamMsg->sessionid, sizeof(ULONG) );

    		if( hash_idx >= TRACE_USERID_HASH_BUCKET )
    		{
    			syncMsg.msgType = TRACE_PATH_MSG_TYPE_OAM_RSP;
    			/*VOS_StrnCpy( syncMsg.userId, pRxOam->oamHeader.SessionID, 8 );*/
    			MAC_ADDR_CPY( syncMsg.userLocation.userMac, data->userMac);
    			syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK;

    			if( tracePathOamFlag )
    			{
    				tracePathOamFlag = 0;
    				VOS_MemCpy( &userLocationRecvBuf, &(syncMsg.userLocation), sizeof(user_loc_t) );
    				VOS_SemGive(traceOamBSemId);
    			}

    			if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
    			{
    				/*VOS_MemCpy( &userLocationRecvBuf, &(syncMsg.userLocation), sizeof(user_loc_t) );
    				if( tracePathOamFlag )
    					VOS_SemGive(traceOamBSemId);*/
    			}
    			else
    			{
    				trace_path_resolving_rsp_cdp_send( &syncMsg );
    			}
    			return VOS_OK;
    		}
#endif
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
                mac_location = trace_mac_tbl_search(data->userMac);
                if(mac_location)
                {
            		if( (rc = trace_mac_userid_get(mac_location->userIdHashIdx, data->userMac, user_id)) == VOS_ERROR )
            		{
            			trace_path_debug_out("TRACE-PATH: hash idx=%d ERR\r\n", mac_location->userIdHashIdx );
            		}
            		else
            		{
                        syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK; 	
                       
            			if( trace_userid_location_set(user_id, data->userMac, &(syncMsg.userLocation)) == VOS_OK )
            			{	      
#if 0                            
                            syncMsg.msgType = TRACE_PATH_MSG_TYPE_RESOLVE_RSP;
                			VOS_StrnCpy( syncMsg.userId, user_id, USER_ID_MAXLEN );
                			MAC_ADDR_CPY( syncMsg.userLocation.userMac, data->userMac);
            			
            				trace_path_resolving_rsp_cdp_send( &syncMsg );
#endif                            
            			}
            		}
                }
            }
            else
            {
                syncMsg.userLocation.resolvingStatus = USER_TRACE_RESOLVED_OK; 	                
                syncMsg.msgType = TRACE_PATH_MSG_TYPE_RESOLVE_RSP;
    			MAC_ADDR_CPY( syncMsg.userLocation.userMac, data->userMac);                
				trace_path_resolving_rsp_cdp_send( &syncMsg );
            }
            data += 1;
	    }
	}
    
	return rc;
}
LONG trace_path_sync_msg_cdp_recv( trace_path_sync_msg_t  *pSyncMsg )
{
	if( pSyncMsg == NULL )
	{
		return VOS_ERROR;
	}
	switch( pSyncMsg->msgType )
	{
		case TRACE_PATH_MSG_TYPE_RESOLVE_REQ:
			trace_path_resolving_req_cdp_recv(pSyncMsg);
			break;
		case  TRACE_PATH_MSG_TYPE_RESOLVE_RSP:
		case TRACE_PATH_MSG_TYPE_SNOOP_NEW:
			trace_path_resolving_rsp_cdp_recv(pSyncMsg);
			break;

		case TRACE_PATH_MSG_TYPE_OAM_REQ:
			trace_path_oam_req_cdp_recv(pSyncMsg);
			break;
		case TRACE_PATH_MSG_TYPE_OAM_RSP:
			trace_path_oam_rsp_cdp_recv(pSyncMsg);
			break;
				
		default:
			break;	
	}
	/*CDP_FreeMsg((void*)pSyncMsg);*/
	return VOS_OK;
}

LONG trace_path_mac_sync_msg_cdp_recv( trace_path_mac_sync_msg_t  *pSyncMsg )
{
    short int PonPortIdx = 0;
	if( pSyncMsg == NULL )
	{
		return VOS_ERROR;
	}
    
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( pSyncMsg->slotno != SYS_LOCAL_MODULE_SLOTNO )
		return VOS_OK;

    PonPortIdx = GetPonPortIdxBySlot(pSyncMsg->slotno, pSyncMsg->portno);
	trace_path_debug_out( "TRACE-PATH:pon%d/%d recv mac oam REQ\r\n", pSyncMsg->slotno, pSyncMsg->portno);

	trace_path_req_oam_broadcast_perpon(PonPortIdx, pSyncMsg->macnum, pSyncMsg->mac[0]);

	return VOS_OK;
}

void trace_path_snoop_task()
{
	LONG result;
	ULONG ulRcvMsg[4];
	SYS_MSG_S *pstMsg;

	while( 1 )
	{
		result = VOS_QueReceive( traceSnoopQId, ulRcvMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_TaskDelay(100);
			ASSERT(0);
			continue;
		}

		pstMsg = (SYS_MSG_S *)ulRcvMsg[3];
		if( NULL == pstMsg )
		{
			VOS_ASSERT(pstMsg);
			VOS_TaskDelay(20);
			continue;
		}

		if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pstMsg );
			continue;
		}

		switch( pstMsg->usMsgCode )
		{
			case TRACE_SNOOP_MSG_TYPE_ADD:
				trace_path_add_notify_proc( (user_trace_snoop_t *)(pstMsg + 1) );
				break;

			case TRACE_SNOOP_MSG_TYPE_DEL:
				trace_path_del_notify_proc( (user_trace_snoop_t *)(pstMsg + 1) );
				break;

			case TRACE_SNOOP_MSG_CODE_OAM:
#if 0                
				trace_path_oam_recv((trace_path_oam_msg_t *)(pstMsg + 1));
#else
                trace_path_oam_recv1((trace_path_oam_msg1_t *)(pstMsg + 1));
#endif
				break;

			case TRACE_SNOOP_MSG_CODE_SYNC:
				trace_path_sync_msg_cdp_recv((trace_path_sync_msg_t *)(pstMsg+1));
				break;
            case TRACE_SNOOP_MSG_CODE_BROADCAST:
                trace_path_mac_sync_msg_cdp_recv((trace_path_mac_sync_msg_t *)(pstMsg+1));
                break;
			default:
				break;
		}
		
		if(SYS_MSG_SRC_SLOT(pstMsg) != (SYS_LOCAL_MODULE_SLOTNO))
		{
			CDP_FreeMsg( pstMsg );    /*CDP消息释放*/
		}
		else
		{
			VOS_Free(pstMsg);         /*本板消息释放(包括PDU消息)*/
		}
		pstMsg = NULL;
	}
}



LONG trace_path_snoop_init()
{
	if( traceOamBSemId != 0 )
		return VOS_OK;
	
	traceOamBSemId = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY );

	traceSnoopQId = VOS_QueCreate( 2048 , VOS_MSG_Q_PRIORITY);
	if( traceSnoopQId  == 0 )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	traceSnoopTId = ( VOS_HANDLE )VOS_TaskCreate("tTraceSnoop", 160, trace_path_snoop_task, NULL );
	if( traceSnoopTId == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	VOS_QueBindTask( traceSnoopTId, traceSnoopQId );

	CDP_Create(RPU_TID_CDP_USERTRACE, CDP_NOTI_VIA_QUEUE, traceSnoopQId, NULL);


	if ( SYS_LOCAL_MODULE_ISHAVEPP() )
	{
		trace_path_pppoe_snoop_hookrtn = trace_path_pppoe_snoop_callback;
	}
	return VOS_OK;
}

#pragma pack()


#ifdef __cplusplus
}
#endif

