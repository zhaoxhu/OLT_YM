#ifdef	__cplusplus
extern "C"
{
#endif

#include  "OltGeneral.h"
#include  "gwEponSys.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includeFromPas.h"
#include	"lib_gwEponMib.h"
#include  "CT_RMan_Main.h"
#include  "CT_Onu_Auth.h"
#include "Cdp_pub.h"


#define CTC_AUTH_MODE_DISABLE		5
#define CTC_AUTH_MODE_ERROR		6

#define MAX_AUTH_PON_PER_SLOT	16

UCHAR g_onuAuthMode[PRODUCT_MAX_TOTAL_SLOTNUM][MAX_AUTH_PON_PER_SLOT+1];
ctc_onu_auth_t *pg_onuAuthList[PRODUCT_MAX_TOTAL_SLOTNUM][MAX_AUTH_PON_PER_SLOT+1];
gpon_onu_auth_t *pg_gpononuAuthList[PRODUCT_MAX_TOTAL_SLOTNUM][MAX_AUTH_PON_PER_SLOT+1];

int  EVENT_GPONAUTH = V2R1_DISABLE;
#define ONU_GPONAUTH_DEBUG  if(EVENT_GPONAUTH == V2R1_ENABLE )sys_console_printf

UCHAR CTC_AUTH_LOID_DATA_NOT_READY[CTC_AUTH_ONU_ID_SIZE];
static CTC_STACK_auth_loid_data_t mn_loid_data;



ULONG onuAuthDebugSwitch = 0;
ULONG onuAuthSemId = 0;
static ctc_onu_auth_t * onu_auth_loid_list_seach( ULONG brdIdx, ULONG portIdx, CHAR * pLoid );
VOID onu_auth_loid_cpy( char *d, char *s, int len );
LONG onu_auth_loid_onuid_set( ctc_onu_auth_msg_t *pAuthMsg );
int	DeletePonPortLoidEntryAll(short int PonPortIdx);

extern short int CTC_STACK_auth_request ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id,
                                         CTC_STACK_auth_response_t         *auth_response);
extern short int CTC_STACK_auth_success ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id);

extern short int CTC_STACK_auth_failure ( 
                                         const PON_olt_id_t                  olt_id, 
                                         const PON_onu_id_t                  onu_id,
                                         const CTC_STACK_auth_failure_type_t failure_type );


extern int onuIdToOnuIndex( ushort_t ponId, ushort_t llId );
extern int parse_pon_command_parameter( struct vty *vty, ULONG *pulSlot, ULONG *pulPort , ULONG *pulOnuId, INT16 *pi16PonId );
extern short int  onu_auth_ctc_event_handler( const short int	 olt_id, const PON_onu_id_t	 onu_id,
									CTC_STACK_auth_loid_data_t loid_data, unsigned char *auth_success, 
									const CTC_STACK_auth_failure_type_t  failure_type );
extern authentication_olt_database_t * CTC_STACK_get_auth_loid_database( const PON_olt_id_t olt_id );
extern PON_STATUS CTC_STACK_remove_auth_loid_data_by_index( const PON_olt_id_t olt_id, const unsigned char loid_id );
extern PON_STATUS CTC_STACK_add_auth_loid_data_by_index( const PON_olt_id_t olt_id, const unsigned char loid_id,
		const CTC_STACK_auth_loid_data_t loid_data);
char *onu_auth_result_to_str( int type );
VOID  onu_auth_ctc_sync_data_callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen);

#define ONU_AUTH_DEBUG_PRINTF if(onuAuthDebugSwitch)sys_console_printf

#if 1
int CTC_STACK_start_loid_authentication (short int  olt_id, short int  onu_id)
{
    short int                      result = -1;
    short int                      llid = GetLlidByOnuIdx(olt_id, onu_id);
    CTC_STACK_auth_response_t      auth_response;
    int                            ret = VOS_ERROR;    
    int brdIdx = GetCardIdxByPonChip(olt_id);
    int portIdx = GetPonPortByPonChip(olt_id);
    ctc_onu_auth_t *auth_ptr = NULL;
    ULONG AuthMode = 0;

    CTC_STACK_auth_failure_type_t failure_type; 
    CTC_STACK_auth_loid_data_t auth_loid_data_checked;
    
    VOS_MemZero(&auth_response, sizeof(CTC_STACK_auth_response_t));
    VOS_MemZero(&auth_loid_data_checked, sizeof(CTC_STACK_auth_loid_data_t));
    /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    result = OnuMgt_AuthRequest(olt_id, onu_id, &auth_response);  
    ONU_AUTH_DEBUG_PRINTF("\r\n%d = OnuMgt_AuthRequest(%d, %d, &auth_response)\r\n", result, olt_id, onu_id);
    if ( result != CTC_STACK_EXIT_OK )
        return ret;
    onu_auth_loid_cpy( auth_loid_data_checked.onu_id,  auth_response.loid_data.onu_id, CTC_AUTH_ONU_ID_SIZE );
    onu_auth_loid_cpy(auth_loid_data_checked.password, auth_response.loid_data.password, CTC_AUTH_PASSWORD_SIZE );
    ONU_AUTH_DEBUG_PRINTF("LOID = %s PWD = %s\r\n", auth_loid_data_checked.onu_id, auth_loid_data_checked.password);
    auth_ptr = onu_auth_loid_list_seach(brdIdx, portIdx, auth_loid_data_checked.onu_id);
    mn_getCtcOnuAuthMode(brdIdx, portIdx, &AuthMode);
    if(auth_ptr)
    {    
        if (AuthMode == mn_ctc_auth_mode_loid_no_pwd || AuthMode == mn_ctc_auth_mode_hybrid_no_pwd)
        {
            ret = VOS_OK;
        }
        else
        {
            if(VOS_StrCmp(auth_loid_data_checked.password, auth_ptr->loidData.password) == 0)            
                ret = VOS_OK;
            else
            {
                failure_type = CTC_AUTH_FAILURE_WRONG_PASSWORD;
            }
        }
    }
    else
    {
        failure_type = CTC_AUTH_FAILURE_ONU_ID_NOT_EXIST;
    }
    if(ret == VOS_OK)
    {   
        /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
        result = OnuMgt_AuthSuccess(olt_id, onu_id);
        ONU_AUTH_DEBUG_PRINTF("ret == VOS_OK, %d = OnuMgt_AuthSuccess(%d, %d)\r\n", result, olt_id, onu_id);
        if ( result != CTC_STACK_EXIT_OK )
            return VOS_ERROR;
    }
    else
    {
        /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
        result = OnuMgt_AuthFailure(olt_id, onu_id, failure_type);
        ONU_AUTH_DEBUG_PRINTF("ret == VOS_ERROR, %d = OnuMgt_AuthFailure(%d, %d, %d)\r\n", result, olt_id, onu_id, failure_type);            
        if ( result != CTC_STACK_EXIT_OK )
            return VOS_ERROR;
    }
    
    if(ret == VOS_OK)
    {
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
        /* B--modified by liwei056@2013-11-1 for D18216 */
#if 1        
		sndMsg.authData.authOnuIdx = onu_id;
		sndMsg.authData.authOnuLlid = llid;
#else
		sndMsg.authData.authOnuLlid = onu_id;
#endif
        /* E--modified by liwei056@2013-11-1 for D18216 */
		sndMsg.authData.authRowStatus = RS_ACTIVE;
		onu_auth_loid_cpy( sndMsg.authData.loidData.onu_id,  auth_response.loid_data.onu_id, CTC_AUTH_ONU_ID_SIZE );
		onu_auth_loid_cpy( sndMsg.authData.loidData.password, auth_response.loid_data.password, CTC_AUTH_PASSWORD_SIZE );
		
		onu_auth_loid_onuid_set( &sndMsg );        
    }
    return ret;
}
#endif
LONG onuAuthCtc_init()
{
	int i, j;

	if( onuAuthSemId == 0 )
		onuAuthSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);
	else
		return VOS_OK;
	
	VOS_MemSet( (VOID*)g_onuAuthMode, mn_ctc_auth_mode_mac, sizeof(g_onuAuthMode) );
	for( i=0; i<PRODUCT_MAX_TOTAL_SLOTNUM; i++ )
	{
		for( j=0; j<MAX_AUTH_PON_PER_SLOT; j++ )
		{
			pg_onuAuthList[i][j] = NULL;
		}		
	}

	for( i=0; i<PRODUCT_MAX_TOTAL_SLOTNUM; i++ )
	{
		for( j=0; j<MAX_AUTH_PON_PER_SLOT; j++ )
		{
			pg_gpononuAuthList[i][j] = NULL;
		}		
	}

	VOS_MemZero( &mn_loid_data, sizeof(mn_loid_data) );
	VOS_MemSet( CTC_AUTH_LOID_DATA_NOT_READY, '0', sizeof(CTC_AUTH_LOID_DATA_NOT_READY) );
	CTC_AUTH_LOID_DATA_NOT_READY[CTC_AUTH_ONU_ID_SIZE-1] = ' ';
	CTC_AUTH_LOID_DATA_NOT_READY[CTC_AUTH_ONU_ID_SIZE] = 0;

	if( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
	{
		unsigned short Handler_id  = 1;
		/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if( GW_CTC_STACK_assign_handler_function(CTC_STACK_HANDLER_AUTHORIZED_LOID, (void *)onu_auth_ctc_event_handler, &Handler_id) != PAS_EXIT_OK )
			sys_console_printf("\r\n register ctc auth loid handler error\r\n" );
	}

	if( VOS_OK != CDP_Create(RPU_TID_CDP_CTC, CDP_NOTI_VIA_FUNC, 0, onu_auth_ctc_sync_data_callback) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	return VOS_OK;
}

short int  onu_auth_ctc_event_handler( const short int	 olt_id, const PON_onu_id_t	 onu_id,
									CTC_STACK_auth_loid_data_t loid_data, unsigned char *auth_success, 
									const CTC_STACK_auth_failure_type_t  failure_type )
{
	ULONG brdIdx = GetCardIdxByPonChip(olt_id);
	ULONG ponIdx = GetPonPortByPonChip(olt_id);
	/*UCHAR onuMacAddr[6];*/
    if( onuAuthDebugSwitch & 1 )
	{
		sys_console_printf( "onu-auth:%s pon%d/%d llid%d,loid=%s,pwd=%s\r\n", onu_auth_result_to_str(failure_type), brdIdx, ponIdx, onu_id,
			loid_data.onu_id, loid_data.password );
	}
#if 0	
	if( CTC_AUTH_SUCCESS != failure_type )
	{
		VOS_MemZero( onuMacAddr, sizeof(onuMacAddr) );
        /*onuMacAddr[0] = failure_type;
		onuRegAuthFailure_EventReport( OLT_DEV_ID, brdIdx, ponIdx, onuMacAddr );*/
	}
	else
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.slotno = brdIdx;
		sndMsg.portno = ponIdx;
		sndMsg.authData.authOnuLlid = onu_id;
		sndMsg.authData.authRowStatus = RS_ACTIVE;
		onu_auth_loid_cpy( sndMsg.authData.loidData.onu_id, loid_data.onu_id, CTC_AUTH_ONU_ID_SIZE );
		onu_auth_loid_cpy( sndMsg.authData.loidData.password, loid_data.password, CTC_AUTH_PASSWORD_SIZE );
		
		onu_auth_loid_onuid_set( &sndMsg );
	}
#endif
	return (PAS_EXIT_OK);
}

VOID onu_ctc_loid_auth_process(ctc_onu_auth_msg_t *pAuthMsg)
{
	if( pAuthMsg == NULL )
	{
		VOS_ASSERT(0);
		return;
	}

	if( pAuthMsg->subCode == CTC_MSG_SUBCODE_AUTH_LOID_CREATE )
	{
		mn_createCtcOnuAuthLoid( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authIdx );
	}
	else if( pAuthMsg->subCode == CTC_MSG_SUBCODE_AUTH_LOID_ADD )
	{
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
            LONG to_slotno = device_standby_master_slotno_get();
            if( to_slotno != 0 )
            {
                ctc_onu_auth_msg_t sndMsg;
                VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
                sndMsg.subCode = pAuthMsg->subCode;
                sndMsg.slotno = pAuthMsg->slotno;
                sndMsg.portno = pAuthMsg->portno;
                sndMsg.authData.authIdx = pAuthMsg->authData.authIdx;
                sndMsg.authData.authRowStatus = RS_ACTIVE;
                if( pAuthMsg->authData.loidData.onu_id )
                    VOS_StrCpy( sndMsg.authData.loidData.onu_id, pAuthMsg->authData.loidData.onu_id );
                if( pAuthMsg->authData.loidData.password ) 
                    VOS_StrCpy( sndMsg.authData.loidData.password, pAuthMsg->authData.loidData.password );
                onu_auth_sync_msg_send ( to_slotno, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
            }
        }
               
		mn_addCtcOnuAuthLoid( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authIdx,
				pAuthMsg->authData.loidData.onu_id, pAuthMsg->authData.loidData.password );
	}
	else if( pAuthMsg->subCode == CTC_MSG_SUBCODE_AUTH_LOID_DEL )
	{
		mn_delCtcOnuAuthLoid( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authIdx, pAuthMsg->authData.loidData.onu_id );
	}
	else if( pAuthMsg->subCode == CTC_MSG_SUBCODE_AUTH_LOID_REP )
	{
		onu_auth_loid_onuid_set( pAuthMsg );
	}
    else if( pAuthMsg->subCode == CTC_MSG_SUBCODE_AUTH_LOID_DEL_ALL)
    {
        short int olt_id = GetPonPortIdxBySlot( pAuthMsg->slotno, pAuthMsg->portno); 
        if(olt_id != RERROR)
            DeletePonPortLoidEntryAll(olt_id);
    }
}

VOID  onu_auth_ctc_sync_data_callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	SYS_MSG_S *pMsg = NULL;
	ctc_onu_auth_msg_t *pAuthMsg;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
			pMsg = (SYS_MSG_S *)pData ;
			if( pMsg == NULL )
			{
				VOS_ASSERT( 0 );
				return ;
			}
			
			switch(SYS_MSG_MSG_CODE(pMsg))
			{
				case CTC_MSG_CODE_ONU_AUTH_MODE:
					pAuthMsg = (ctc_onu_auth_msg_t *)(pMsg + 1);
					if( pAuthMsg == NULL )
					{
						VOS_ASSERT(0);
						return;
					}
                    					    
                    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                    {
                		LONG to_slotno = device_standby_master_slotno_get();
                		if( to_slotno != 0 )
                		{
                    		ctc_onu_auth_msg_t sndMsg;
                    		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
                    		sndMsg.subCode = pAuthMsg->subCode;
                    		sndMsg.slotno = pAuthMsg->slotno;
                    		sndMsg.portno = pAuthMsg->portno;
                    		onu_auth_sync_msg_send ( to_slotno, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
                		}
                    }
                    mn_setCtcOnuAuthMode( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->subCode );    
					break;
				case CTC_MSG_CODE_ONU_AUTH_LOID:
					onu_ctc_loid_auth_process( (VOID*)(pMsg + 1) );
					break;
				case CTC_MSG_CODE_ONU_TX_POWER:
					onu_tx_power_control_process( (VOID *)(pMsg + 1) );
					break;

				default:
					ASSERT(0);
					break;
			}
			CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:
			CDP_FreeMsg(pData);
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

	return ;
}

#if 0
STATUS	CTC_setOnuAuthMode( ULONG mode )
{
	int i=0, j=0;
	short int ponId;

	g_onuAuthMode = mode;

	if( !(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) && SYS_LOCAL_MODULE_ISMASTERACTIVE) )
		return VOS_ERROR;
	

	for( i=PONCARD_FIRST; i<=PONCARD_LAST; i++ )
	{
		if(SlotCardIsPonBoard(i)==ROK)
		{
			for( j=0; j<MAX_PONPORT_PER_BOARD;j++ )
			{
				ponId = GetPonPortIdxBySlot( i, j );
				if( PonPortIsWorking( ponId ) != TRUE )
					continue;
						
				if(( mode <= CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD) )
				{
					if( CTC_STACK_set_auth_mode(ponId, mode) != CTC_STACK_EXIT_OK )
						sys_console_printf("\r\nset pon%d/%d auth mode Err!", i, j+1);
					else
					{
					/*OnuAuth_DeregisterAllOnu( ponId );*/
					}
				}
				else
				{
					if( SetOnuAuthenticationMode(ponId, FALSE) != VOS_OK )
					{
						sys_console_printf("\r\nset pon%d/%d auth disable Err!", i, j+1);
					}
					else
					{
						ActivatePendingOnuMsg( ponId );
					}
				}
			}
		}
	}

	return VOS_OK;
}
#endif

char *onu_auth_result_to_str( int type )
{
	char *pAuthStr;
	switch( type )
	{
		case CTC_AUTH_SUCCESS:
			pAuthStr = "OK";
			break;
		case CTC_AUTH_FAILURE_ONU_ID_NOT_EXIST:
			pAuthStr = "Loid is not exist";
			break;
		case CTC_AUTH_FAILURE_WRONG_PASSWORD:
			pAuthStr = "Pwd is wrong";
			break;
		case CTC_AUTH_FAILURE_ONU_ID_USED:
			pAuthStr = "Loid is already used";
			break;
		default:
			pAuthStr = "Unknown";
			break;
	}
	return pAuthStr;
}

VOID onu_auth_loid_cpy( char *d, char *s, int len )
{
	int i, endflag = 0;
	for( i=0; i<len; i++ )
	{
		if( *s != '\0' )
		{
			*d = *s;
			d++;
			endflag = 1;
		}
		else if( endflag )
			break;
		s++;
	}
	*d = 0;
}

char *onu_auth_mode_to_str( ULONG mode )
{
	char *pStr;
	switch( mode )
	{
		case mn_ctc_auth_mode_mac:
			pStr = "mac";
			break;
		case mn_gpon_auth_mode_sn:
			pStr = "sn";
			break;
		case mn_gpon_auth_mode_sn_and_pwd:
			pStr = "sn-pwd";
			break;
		case mn_ctc_auth_mode_loid:
			pStr = "loid";
			break;
		case mn_ctc_auth_mode_hybrid:
			pStr = "hybrid";
			break;
		case mn_ctc_auth_mode_loid_no_pwd:
			pStr = "loid-not-chk-pwd";
			break;
		case mn_ctc_auth_mode_hybrid_no_pwd:
			pStr = "hybrid-not-chk-pwd";
			break;
		case mn_ctc_auth_mode_disable:
			pStr = "disable";
			break;
		default:
			pStr = "Err";
			break;
	}
	return pStr;
}
char *onu_auth_mode_to_str1( ULONG mode )
{
	char *pStr;
	switch( mode )
	{
		case mn_ctc_auth_mode_mac:
			pStr = "    MAC      ";
			break;
		case mn_gpon_auth_mode_sn_and_pwd:
			pStr = "  SN-AND-PWD ";
			break;
		case mn_ctc_auth_mode_loid:
			pStr = "    LOID     ";
			break;
		case mn_ctc_auth_mode_hybrid:
			pStr = "   HYBRID    ";
			break;
		case mn_ctc_auth_mode_loid_no_pwd:
			pStr = " LOID-NO-PWD ";
			break;
		case mn_ctc_auth_mode_hybrid_no_pwd:
			pStr = "HYBRID-NO-PWD";
			break;
		default:
			pStr = "   None      ";
			break;
	}
	return pStr;
}

LONG onu_auth_sync_msg_send ( ULONG dstSlotNo, ULONG msgCode, ctc_onu_auth_msg_t *pSendMsg )
{
	unsigned int ulLen;
	SYS_MSG_S *pMsg;
	ctc_onu_auth_msg_t *pTmp;

	if( dstSlotNo == SYS_LOCAL_MODULE_SLOTNO )
	{
        /*moved by luh 2012-10-30*/
		/*VOS_ASSERT(0);*/
		return  VOS_ERROR;
	}
    /*如果没有插板子，则直接返回added by luh 2012-8-10*/
    if(!(SYS_MODULE_IS_READY(dstSlotNo))||!SYS_MODULE_SLOT_ISHAVECPU(dstSlotNo))/*modi 2013-3-21  Q.17131*/
        return VOS_ERROR;
    
	ulLen = sizeof(ctc_onu_auth_msg_t) + sizeof(SYS_MSG_S);
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

	if( NULL == pMsg )
	{
		VOS_ASSERT(0);
		return  VOS_ERROR;
	}
	VOS_MemZero((CHAR *)pMsg, ulLen );
	pTmp = (ctc_onu_auth_msg_t *)(pMsg + 1);

	SYS_MSG_SRC_ID( pMsg ) = MODULE_ONU;
	SYS_MSG_DST_ID( pMsg ) = MODULE_ONU;
	SYS_MSG_SRC_SLOT( pMsg ) = SYS_LOCAL_MODULE_SLOTNO;
	SYS_MSG_DST_SLOT( pMsg ) = dstSlotNo;
	SYS_MSG_MSG_TYPE( pMsg ) = MSG_NOTIFY;
	SYS_MSG_BODY_STYLE( pMsg ) = MSG_BODY_INTEGRATIVE;
	SYS_MSG_BODY_POINTER( pMsg ) = pTmp;
	SYS_MSG_FRAME_LEN( pMsg ) = sizeof(ctc_onu_auth_msg_t);
	SYS_MSG_MSG_CODE(pMsg) = msgCode;

	VOS_MemCpy((void *)pTmp, (void *)pSendMsg, sizeof(ctc_onu_auth_msg_t));

       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_CTC, dstSlotNo, RPU_TID_CDP_CTC, CDP_MSG_TM_ASYNC,
                 (VOID *)pMsg, ulLen, MODULE_ONU ) )
	{
		VOS_ASSERT(0); 
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}

	return VOS_OK;

}
/*added by wangying  34118*/
#if 1
LONG GetCtcOnuAuthMode_mib( ULONG brdIdx, ULONG portIdx, ULONG *pMode )
{
	mn_getCtcOnuAuthMode(brdIdx,portIdx,pMode);
	if (((MODULE_E_GFA8000_16GPONB0 == __SYS_MODULE_TYPE__( brdIdx ))
		||(MODULE_E_GFA8000_16GPONB0_M  == __SYS_MODULE_TYPE__( brdIdx ))
		||(MODULE_E_GFA8100_16GPONB0  == __SYS_MODULE_TYPE__( brdIdx )))&&(*pMode==1))
		*pMode=7;
	return VOS_OK;
}
#endif


LONG mn_getCtcOnuAuthMode( ULONG brdIdx, ULONG portIdx, ULONG *pMode )
{
	PON_olt_id_t ponId;

	if( pMode == NULL )
		return VOS_ERROR;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
	
	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}
	/*if( CTC_STACK_get_auth_mode(PonPortIdx, &mode) == CTC_STACK_EXIT_OK )
	{
		if( mode > CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD ) mode = CTC_AUTH_MODE_DISABLE;
		rc = VOS_OK;
	}
	else
	{
		mode = CTC_AUTH_MODE_ERROR;
		rc = VOS_ERROR;
	}*/

	VOS_SemTake( onuAuthSemId, WAIT_FOREVER );
	if( (g_onuAuthMode[brdIdx][portIdx] == mn_ctc_auth_mode_min) || (g_onuAuthMode[brdIdx][portIdx] >=mn_ctc_auth_mode_error) )/*changed by wangying*/ 
		g_onuAuthMode[brdIdx][portIdx] = mn_ctc_auth_mode_disable;

	*pMode = (ULONG)g_onuAuthMode[brdIdx][portIdx];
	VOS_SemGive( onuAuthSemId );

	return VOS_OK;
}

LONG mn_setCtcOnuAuthMode( ULONG brdIdx, ULONG portIdx, ULONG mode )
{
	LONG rc = CTC_STACK_EXIT_ERROR;
	PON_olt_id_t ponId, ponPartnerId = 0;
	short int brdPartnerIdx = 0, portPartnerIdx = 0;

	if( (mn_ctc_auth_mode_min == mode) || (mn_ctc_auth_mode_error <= mode) )/*changed by wangying*/
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		rc = CTC_STACK_EXIT_OK;        
		/*switch( mode )
		{
			case mn_ctc_auth_mode_mac:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_MAC);
				break;
			case mn_ctc_auth_mode_loid:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_LOID);
				break;
			case mn_ctc_auth_mode_hybrid:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_HYBRID);
				break;
			case mn_ctc_auth_mode_loid_no_pwd:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_LOID_NOT_CHK_PWD);
				break;
			case mn_ctc_auth_mode_hybrid_no_pwd:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD);
				break;
			case mn_ctc_auth_mode_disable:
				CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_MAC);
				rc = PAS_set_authorize_mac_address_according_list_mode( ponId, FALSE );
				ActivatePendingOnuMsg( ponId );
				break;
			default:
				break;
		}*/
		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: set auth-mode(%s) to pon%d/%d\r\n", onu_auth_mode_to_str(mode), brdIdx, portIdx );
		}
	}
	/*else */
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = mode;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
        if (PonPortIsWorking(ponId) == TRUE)
    		onu_auth_sync_msg_send ( brdIdx, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
		/*added by wangjiah@2017-05-06:begin
		 * to support config sync for pon protect
		 * */
		if(VOS_OK == PonPortSwapSlotQuery(ponId, &brdPartnerIdx, &portPartnerIdx) && PonPortIsWorking(ponPartnerId))
		{
			sndMsg.slotno = brdPartnerIdx;
			sndMsg.portno = portPartnerIdx;
    		onu_auth_sync_msg_send ( brdPartnerIdx, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
		}
		/*added by wangjiah@2017-05-06:end*/
		rc = CTC_STACK_EXIT_OK;

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: send auth-mode(%s) to pon%d/%d\r\n", onu_auth_mode_to_str(mode), brdIdx, portIdx );
		}
	}
    /*del by luh 2012-8-17*/
	/*if( rc == CTC_STACK_EXIT_OK )*/
	{
		VOS_SemTake( onuAuthSemId, WAIT_FOREVER );
		g_onuAuthMode[brdIdx][portIdx] = mode;
		VOS_SemGive( onuAuthSemId );
	}
	
	return VOS_OK;
}
LONG mn_setCtcOnuAuthMode1( ULONG brdIdx, ULONG portIdx, ULONG mode )
{
	LONG rc = CTC_STACK_EXIT_ERROR;
	PON_olt_id_t ponId;

	if( (mn_ctc_auth_mode_min == mode) || (mn_ctc_auth_mode_max <= mode) )
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		rc = CTC_STACK_EXIT_OK;        
		switch( mode )
		{
			case mn_ctc_auth_mode_mac:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_MAC);
				break;
			case mn_ctc_auth_mode_loid:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_LOID);
				break;
			case mn_ctc_auth_mode_hybrid:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_HYBRID);
				break;
			case mn_ctc_auth_mode_loid_no_pwd:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_LOID_NOT_CHK_PWD);
				break;
			case mn_ctc_auth_mode_hybrid_no_pwd:
				rc = CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD);
				break;
			case mn_ctc_auth_mode_disable:
				CTC_STACK_set_auth_mode(ponId, CTC_AUTH_MODE_MAC);
				rc = PAS_set_authorize_mac_address_according_list_mode( ponId, FALSE );
				ActivatePendingOnuMsg( ponId );
				break;
			default:
				break;
		}
		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: set auth-mode(%s) to pon%d/%d\r\n", onu_auth_mode_to_str(mode), brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_getFirstCtcOnuAuthModeIdx( ULONG *pBrdIdx, ULONG *pPortIdx )
{
	ULONG brdIdx, portIdx;
	int ponId;
	
	if( (NULL == pBrdIdx) || (NULL == pPortIdx) )
		return VOS_ERROR;
	
	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=/*MAX_PONPORT_PER_BOARD*/SYS_MODULE_SLOT_PORT_NUM(brdIdx); portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId >= 0) && (ponId < MAXPON) )
			{
				*pBrdIdx = brdIdx;
				*pPortIdx = portIdx;
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}

LONG mn_getNextCtcOnuAuthModeIdx( ULONG brdIdx, ULONG portIdx, ULONG *pNextBrdIdx, ULONG *pNextPortIdx )
{
	int ponId;
	
	if( (NULL == pNextBrdIdx) || (NULL == pNextPortIdx) )
		return VOS_ERROR;

	if( brdIdx == 0 )
		return mn_getFirstCtcOnuAuthModeIdx( pNextBrdIdx, pNextPortIdx );

	portIdx++;
	
	for( ; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( ; portIdx<=/*MAX_PONPORT_PER_BOARD*/SYS_MODULE_SLOT_PORT_NUM(brdIdx); portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId >= 0) && (ponId < MAXPON) )
			{
				*pNextBrdIdx = brdIdx;
				*pNextPortIdx = portIdx;
				return VOS_OK;
			}
		}
		portIdx = 1;
	}
	return VOS_ERROR;
}

static ctc_onu_auth_t * onu_auth_loid_list_get( ULONG brdIdx, ULONG portIdx, ULONG loidIdx )
{
	ctc_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( pItem->authIdx == loidIdx )
			{
				return pItem;
			}
			else if( pItem->authIdx > loidIdx )
			{
				break;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}
static ctc_onu_auth_t * onu_auth_loid_list_get_by_llid( ULONG brdIdx, ULONG portIdx, UCHAR llid )
{
	ctc_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD || 0 == llid )
		return NULL;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( pItem->authOnuLlid == llid )
			{
				return pItem;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}

static ctc_onu_auth_t * onu_auth_loid_list_seach( ULONG brdIdx, ULONG portIdx, CHAR * pLoid )
{
	ctc_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( VOS_StrCmp(pItem->loidData.onu_id, pLoid) == 0 )
			{
				return pItem;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}

static ULONG onu_auth_loid_free_idx_get( ULONG brdIdx, ULONG portIdx )
{
	ctc_onu_auth_t *pItem;
	ULONG loidIdx = 1;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( pItem->authIdx > loidIdx )
			break;

		pItem = pItem->next;
		loidIdx++;

		if( loidIdx >= 128 )
		{
			loidIdx = 0;
			break;
		}
	}
	
	return loidIdx;
}

static ctc_onu_auth_t * onu_auth_loid_list_new( ULONG brdIdx, ULONG portIdx, ULONG loidIdx )
{
	ctc_onu_auth_t *pItem;
	ctc_onu_auth_t *pPreItem;
	ctc_onu_auth_t *pNew;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	if( loidIdx == 0 )
		loidIdx = onu_auth_loid_free_idx_get( brdIdx, portIdx );
	if( loidIdx == 0 )
		return NULL;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	pPreItem = NULL;
	while( pItem )
	{
		if( pItem->authIdx == loidIdx )
		{
			if( pItem->authRowStatus == 1 )
				return NULL;
			else
				return pItem;
		}
		else if( pItem->authIdx > loidIdx )
		{
			break;
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	pNew = VOS_Malloc( sizeof(ctc_onu_auth_t), MODULE_ONU );
	if( pNew )
	{
		VOS_MemZero( pNew, sizeof(ctc_onu_auth_t) );
		pNew->authIdx = loidIdx;
		pNew->authRowStatus = RS_NOTREADY;
		pNew->next = pItem;

		if( pPreItem == NULL )
		{
			if( pItem == NULL )
			{
				pg_onuAuthList[brdIdx][portIdx] = pNew;
			}
			else
			{
				pg_onuAuthList[brdIdx][portIdx] = pNew;
			}
		}
		else
		{
			pPreItem->next = pNew;
		}
	}
	return pNew;
}

static LONG onu_auth_loid_list_free( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, CHAR *pLoid )
{
	ctc_onu_auth_t *pItem;
	ctc_onu_auth_t *pPreItem;

	if( (loidIdx == 0) && (pLoid == NULL) )	/* 不能同时无效 */
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
	
	pItem = pg_onuAuthList[brdIdx][portIdx];
	pPreItem = NULL;
	while( pItem )
	{
		if( loidIdx )
		{
			if( pItem->authIdx == loidIdx )
			{
				if( pPreItem == NULL )
				{
					pg_onuAuthList[brdIdx][portIdx] = pg_onuAuthList[brdIdx][portIdx]->next;
					VOS_Free( pItem );
				}
				else
				{
					pPreItem->next = pItem->next;
					VOS_Free( pItem );
				}
				return VOS_OK;
			}
			else if( pItem->authIdx > loidIdx )
			{
				break;
			}
		}
		if( pLoid )
		{
			if( VOS_StrCmp(pItem->loidData.onu_id, pLoid) == 0 )
			{
				if( pPreItem == NULL )
				{
					pg_onuAuthList[brdIdx][portIdx] = pg_onuAuthList[brdIdx][portIdx]->next;
					VOS_Free( pItem );
				}
				else
				{
					pPreItem->next = pItem->next;
					VOS_Free( pItem );
				}
				return VOS_OK;
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	return VOS_ERROR;
}


static LONG onu_auth_loid_list_add( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd )
{
	ctc_onu_auth_t *pList;

	if( pLoid == NULL )
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
    pList = onu_auth_loid_list_seach(brdIdx, portIdx, pLoid);
    if(pList == NULL)
	    pList = onu_auth_loid_list_new( brdIdx, portIdx, loidIdx );
    
    if(pList == NULL)
        pList = onu_auth_loid_list_get(brdIdx, portIdx, loidIdx);/*如果loid索引被其他的loid占用，则清空，再次写入modi by luh 2012-2-17*/
    if( pList )
    {
        pList->authRowStatus = RS_ACTIVE;
        onu_auth_loid_cpy( pList->loidData.onu_id, pLoid, CTC_AUTH_ONU_ID_SIZE );
        if( pPwd )
			onu_auth_loid_cpy( pList->loidData.password, pPwd, CTC_AUTH_PASSWORD_SIZE );

		return VOS_OK;
	}
	return VOS_ERROR;
}

LONG onu_auth_loid_onuid_set( ctc_onu_auth_msg_t *pAuthMsg )
{
	ctc_onu_auth_t *pItem;
	ctc_onu_auth_t *liv_authdata;
	if( pAuthMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    /*只有6900的PON板才需要向主控同步 modi by luh  2012-8-17*/
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && (!SYS_LOCAL_MODULE_ISMASTERACTIVE))
	{
		pItem = onu_auth_loid_list_seach( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.loidData.onu_id );
		if( pItem )
		{
			pItem->authOnuIdx = pAuthMsg->authData.authOnuIdx;
			pItem->authOnuLlid = pAuthMsg->authData.authOnuLlid;

			pAuthMsg->subCode = CTC_MSG_SUBCODE_AUTH_LOID_REP;
			pAuthMsg->authData.authIdx = pItem->authIdx;

			onu_auth_sync_msg_send ( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_LOID, pAuthMsg );

			if( onuAuthDebugSwitch & 1 )
			{
				sys_console_printf( "onu-auth:send llid to master pon%d/%d llid%d,loid=%s\r\n", pAuthMsg->slotno, pAuthMsg->portno, 
					pAuthMsg->authData.authOnuLlid, pAuthMsg->authData.loidData.onu_id );
			}
		}
	}
    
	{
        /*清空之前跟onu 配对的loid 的onu索引added by luh 2012-2-8*/
        VOS_ASSERT(pAuthMsg->authData.authOnuLlid > 0);
        liv_authdata = onu_auth_loid_list_get_by_llid(pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authOnuLlid);
        while(liv_authdata != NULL)
        {
            liv_authdata->authOnuLlid = 0;
            liv_authdata->authOnuIdx  = 0;
            liv_authdata = onu_auth_loid_list_get_by_llid(pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authOnuLlid);
        }

		pItem = onu_auth_loid_list_get( pAuthMsg->slotno, pAuthMsg->portno, pAuthMsg->authData.authIdx );
		if( pItem )
		{
			pItem->authOnuIdx = pAuthMsg->authData.authOnuIdx;
			pItem->authOnuLlid = pAuthMsg->authData.authOnuLlid;
			pItem->authRowStatus = RS_ACTIVE;
            /*modi by luh 2012-2-8*/
			/*onu_auth_loid_cpy( pItem->loidData.onu_id, pAuthMsg->authData.loidData.onu_id, CTC_AUTH_ONU_ID_SIZE );
			onu_auth_loid_cpy( pItem->loidData.password, pAuthMsg->authData.loidData.password, CTC_AUTH_PASSWORD_SIZE );*/
			if( onuAuthDebugSwitch & 1 )
			{
				sys_console_printf( "onu-auth:set llid pon%d/%d llid%d,loid=%s\r\n", pAuthMsg->slotno, pAuthMsg->portno, 
					pAuthMsg->authData.authOnuLlid, pAuthMsg->authData.loidData.onu_id );
			}
		}
	}
	return VOS_OK;
}
LONG CopyCtcOnuAuthMode(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int liv_slot = GetCardIdxByPonChip(SrcPonPortIdx);
    int liv_pon = GetPonPortByPonChip(SrcPonPortIdx);
    ULONG S_authmode = 0;
    ULONG D_authmode = 0;
    if(liv_slot == RERROR || liv_pon == RERROR)
        return VOS_ERROR;
	/*if( (SrcPonPortIdx < 0) || (SrcPonPortIdx >= MAXPON)||(DstPonPortIdx < 0) || (DstPonPortIdx >= MAXPON)  )
	{
		return VOS_ERROR;    	
	}*/

    mn_getCtcOnuAuthMode(liv_slot,liv_pon, &S_authmode);
    
    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        /* 自拷贝，应该是配置恢复 */
#if 0
        mn_setCtcOnuAuthMode(liv_slot, liv_pon, S_authmode);
#else
        /*问题单15973，此时pon口未激活，不能直接调用api恢复。
                 added by luh 2012-9-27*/
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)     
        {
            ctc_onu_auth_msg_t sndMsg;
            VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
            sndMsg.subCode = S_authmode;
            sndMsg.slotno = liv_slot;
            sndMsg.portno = liv_pon;
            onu_auth_sync_msg_send ( liv_slot, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
        }
#endif
        return VOS_OK;
    }
    else
    {
        liv_slot = GetCardIdxByPonChip(DstPonPortIdx);
        liv_pon = GetPonPortByPonChip(DstPonPortIdx);
        mn_getCtcOnuAuthMode(liv_slot,liv_pon, &D_authmode);
        
        if ( OLT_COPYFLAGS_COVER & CopyFlags || OLT_COPYFLAGS_ONLYNEW & CopyFlags)
        {
            if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)     
                mn_setCtcOnuAuthMode(liv_slot, liv_pon, S_authmode);
            else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
            {
        		ctc_onu_auth_msg_t sndMsg;
        		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
        		sndMsg.subCode = S_authmode;
        		sndMsg.slotno = liv_slot;
        		sndMsg.portno = liv_pon;
        		onu_auth_sync_msg_send ( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
            }            
        }
        else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
        {
            if(D_authmode != S_authmode)
            {
                if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)     
                    mn_setCtcOnuAuthMode(liv_slot, liv_pon, S_authmode);
                else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
                {
            		ctc_onu_auth_msg_t sndMsg;
            		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
            		sndMsg.subCode = S_authmode;
            		sndMsg.slotno = liv_slot;
            		sndMsg.portno = liv_pon;
            		onu_auth_sync_msg_send ( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_MODE, &sndMsg );
                }            
            }
        }	          
    }
  
	return VOS_OK;
}

LONG CopyCtcOnuAuthLoid(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    /*PON_olt_id_t ponId;*/
    /*INT16 ret;*/
    CTC_STACK_auth_loid_data_t loid_data;
    int liv_slot = GetCardIdxByPonChip(SrcPonPortIdx);
    int liv_pon = GetPonPortByPonChip(SrcPonPortIdx);
    ctc_onu_auth_t *Src_pItem = NULL;
    ctc_onu_auth_t *Dst_pItem = NULL;
    
    if(liv_slot == RERROR || liv_pon == RERROR)
        return VOS_ERROR;
	/*if( (SrcPonPortIdx < 0) || (SrcPonPortIdx >= MAXPON)||(DstPonPortIdx < 0) || (DstPonPortIdx >= MAXPON)  )
	{
		return VOS_ERROR;    	
	}*/
    /*if (PonPortIsWorking(PonPortIdx) != TRUE)
        return VOS_ERROR;*/
    Src_pItem = pg_onuAuthList[liv_slot][liv_pon];	
    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        /* 自拷贝，应该是配置恢复 do nothing  modi by luh 2012-3-5*/
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)     
        {
            while(Src_pItem)
            {
                ctc_onu_auth_msg_t sndMsg;
                VOS_MemZero( &loid_data, sizeof(loid_data) );        
                VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
                sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
                sndMsg.slotno = liv_slot;
                sndMsg.portno = liv_pon;
                sndMsg.authData.authIdx = Src_pItem->authIdx;
                sndMsg.authData.authRowStatus = RS_ACTIVE;
                if( Src_pItem->loidData.onu_id ) VOS_StrCpy( sndMsg.authData.loidData.onu_id, Src_pItem->loidData.onu_id );
                if( Src_pItem->loidData.password ) VOS_StrCpy( sndMsg.authData.loidData.password, Src_pItem->loidData.password );
                                            		
                onu_auth_sync_msg_send ( liv_slot, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
                Src_pItem = Src_pItem->next;            
            }
        }
        return VOS_OK;
    }
    else
    {
        liv_slot = GetCardIdxByPonChip(DstPonPortIdx);
        liv_pon = GetPonPortByPonChip(DstPonPortIdx);
        Dst_pItem = pg_onuAuthList[liv_slot][liv_pon];  
        if ( OLT_COPYFLAGS_COVER & CopyFlags || OLT_COPYFLAGS_ONLYNEW & CopyFlags)
        {
            while(Src_pItem)
            {
                ctc_onu_auth_msg_t sndMsg;
                VOS_MemZero( &loid_data, sizeof(loid_data) );        
                VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
                sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
                sndMsg.slotno = liv_slot;
                sndMsg.portno = liv_pon;
                sndMsg.authData.authIdx = Src_pItem->authIdx;
                sndMsg.authData.authRowStatus = RS_ACTIVE;
                if( Src_pItem->loidData.onu_id )
                    VOS_StrCpy( sndMsg.authData.loidData.onu_id, Src_pItem->loidData.onu_id );
                if( Src_pItem->loidData.password ) 
                    VOS_StrCpy( sndMsg.authData.loidData.password, Src_pItem->loidData.password );
                
                if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)   
                {
                    /*通过命令行修改，本地值应该同时修改2012-7-30 add by luh*/                    
	                mn_addCtcOnuAuthLoid( liv_slot, liv_pon, Src_pItem->authIdx, sndMsg.authData.loidData.onu_id, sndMsg.authData.loidData.password );
                }
                else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
                    onu_auth_sync_msg_send ( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
                
                Src_pItem = Src_pItem->next;            
            }
        }
        else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
        {
            while(Src_pItem)
        	{
        		ctc_onu_auth_msg_t sndMsg;
            	VOS_MemZero( &loid_data, sizeof(loid_data) );        
        		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
                if(Dst_pItem)
                {
                    if(VOS_StriCmp(Src_pItem->loidData.onu_id, Dst_pItem->loidData.onu_id)!=0 
                            || VOS_StriCmp(Src_pItem->loidData.password, Dst_pItem->loidData.password)!=0 
                            ||  Src_pItem->authIdx != Dst_pItem->authIdx)
                    {
                        sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
                        sndMsg.slotno = liv_slot;
                        sndMsg.portno = liv_pon;
                        sndMsg.authData.authIdx = Src_pItem->authIdx;
                        sndMsg.authData.authRowStatus = RS_ACTIVE;
                        if( Src_pItem->loidData.onu_id ) VOS_StrCpy( sndMsg.authData.loidData.onu_id, Src_pItem->loidData.onu_id );
                        if( Src_pItem->loidData.password ) VOS_StrCpy( sndMsg.authData.loidData.password, Src_pItem->loidData.password );
                                        		
                        if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)   
                        {
                            /*通过命令行修改，本地值应该同时修改2012-7-30 add by luh*/                            
        	                mn_addCtcOnuAuthLoid( liv_slot, liv_pon, Src_pItem->authIdx, sndMsg.authData.loidData.onu_id, sndMsg.authData.loidData.password );
                        }
                        else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
                            onu_auth_sync_msg_send( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
                    }
                    Dst_pItem = Dst_pItem->next;                                
                }
                else
                {
                    sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
                    sndMsg.slotno = liv_slot;
                    sndMsg.portno = liv_pon;
                    sndMsg.authData.authIdx = Src_pItem->authIdx;
                    sndMsg.authData.authRowStatus = RS_ACTIVE;
                    if( Src_pItem->loidData.onu_id ) VOS_StrCpy( sndMsg.authData.loidData.onu_id, Src_pItem->loidData.onu_id );
                    if( Src_pItem->loidData.password ) VOS_StrCpy( sndMsg.authData.loidData.password, Src_pItem->loidData.password );
                                                        		
                    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)   
                    {
                        /*通过命令行修改，本地值应该同时修改2012-7-30 add by luh*/
    	                mn_addCtcOnuAuthLoid( liv_slot, liv_pon, Src_pItem->authIdx, sndMsg.authData.loidData.onu_id, sndMsg.authData.loidData.password );
                    }
                    else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
                        onu_auth_sync_msg_send ( SYS_MASTER_ACTIVE_SLOTNO, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
                }
            }
            Src_pItem = Src_pItem->next;
        }	          
    }
  
	return VOS_OK;
}

LONG mn_addCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd )
{
	PON_olt_id_t                ponId;
	/*INT16                       ret;*/
	CTC_STACK_auth_loid_data_t  loid_data;
    LONG                        return_code;
	if( NULL == pLoid )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}
	VOS_MemZero( &loid_data, sizeof(loid_data) );
	if( VOS_StrLen(pLoid) > CTC_AUTH_ONU_ID_SIZE )
	{
		sys_console_printf( "Loid strings is too long\r\n" );
		return VOS_ERROR;
	}
	else
		VOS_StrCpy( loid_data.onu_id, pLoid );
	
	if( pPwd )
	{
		if( VOS_StrLen(pPwd) > CTC_AUTH_PASSWORD_SIZE )
		{
			sys_console_printf( "password strings is too long\r\n" );
			return VOS_ERROR;
		}
		else
			VOS_StrCpy( loid_data.password, pPwd );
	}

	if( loidIdx == 0 )
		loidIdx = onu_auth_loid_free_idx_get( brdIdx, portIdx );
	if( loidIdx == 0 )
	{
		sys_console_printf( " pon%d/%d auth-loid table is full\r\n", brdIdx, portIdx );
		return VOS_ERROR;
	}
	
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
        /*do nothing  modi by luh 2012-3-2*/
        /*if( (ret = CTC_STACK_add_auth_loid_data(ponId, loid_data)) != CTC_STACK_EXIT_OK )
        {
            sys_console_printf( "add loid data Err(ponId=%d,errcode=%d\r\n", ponId, ret );
            return VOS_ERROR;
        }
        if( onuAuthDebugSwitch & 1 )
        {
            sys_console_printf(" ONU-AUTH: set auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
        }*/
	}
    /*else */
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ctc_onu_auth_msg_t sndMsg;
        
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.authData.authIdx = loidIdx;
		sndMsg.authData.authRowStatus = RS_ACTIVE;
		if( pLoid )
            VOS_StrCpy( sndMsg.authData.loidData.onu_id, pLoid );
		if( pPwd ) 
            VOS_StrCpy( sndMsg.authData.loidData.password, pPwd );
        /*防止多次恢复，showrun时只恢复本地数据，
               *当需要激活pon口时才会调用本地数据配置pon芯片added by luh 2012-2-15 */        
        if (PonPortIsWorking(ponId) == TRUE)
		    onu_auth_sync_msg_send ( brdIdx, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: send auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
		}
	}
	
	return_code = onu_auth_loid_list_add( brdIdx, portIdx, loidIdx, pLoid, pPwd );
	
	return return_code;
}

LONG mn_delCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid )
{
    	INT16 ponId = 0;
	/*INT16 ret;*/
	/*CTC_STACK_auth_loid_data_t loid_data;*/
	/*static ctc_onu_auth_t *pItem;*/
	LONG return_code = 0;	
	if( (loidIdx == 0) && (pLoid == NULL) )	/* 不能同时无效 */
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	return_code = onu_auth_loid_list_free( brdIdx, portIdx, loidIdx, pLoid );

	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
        /*do nothing  modi by luh 2012-3-2*/
        
		/*VOS_MemZero( &loid_data, sizeof(loid_data) );
		if( pLoid )
		{
			if( VOS_StrLen(pLoid) > CTC_AUTH_ONU_ID_SIZE )
			{
				sys_console_printf( "Loid strings is too long\r\n" );
				return VOS_ERROR;
			}
			else
				VOS_StrnCpy( loid_data.onu_id, pLoid, CTC_AUTH_ONU_ID_SIZE );
		}
		else
		{
			pItem = onu_auth_loid_list_get( brdIdx, portIdx, loidIdx );
			if( pItem == NULL )
			{
				sys_console_printf( "Loid is not exist\r\n" );
				return VOS_ERROR;
			}
			VOS_StrnCpy( loid_data.onu_id, pItem->loidData.onu_id, CTC_AUTH_ONU_ID_SIZE );
		}

		if( (ret = CTC_STACK_remove_auth_loid_data(ponId, loid_data)) != CTC_STACK_EXIT_OK )
		{
			sys_console_printf( "delete loid data Err(ponId=%d,errcode=%d)\r\n", ponId, ret );
			return VOS_ERROR;
		}

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: del auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
		}*/
	}
    /*else */
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_DEL;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.authData.authIdx = loidIdx;
		sndMsg.authData.authRowStatus = RS_DESTROY;
		if( pLoid ) VOS_StrCpy( sndMsg.authData.loidData.onu_id, pLoid );
        
        if (PonPortIsWorking(ponId) == TRUE)
    		onu_auth_sync_msg_send ( brdIdx, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: send del auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
		}
	}
	
	return return_code;
}

LONG mn_createCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		if( onu_auth_loid_list_new( brdIdx, portIdx, loidIdx ) == NULL )
			return VOS_ERROR;

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: set new null row auth-loid(%d) to pon%d/%d\r\n", loidIdx, brdIdx, portIdx );
		}
	}
    /*else */
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_CREATE;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.authData.authIdx = loidIdx;
		sndMsg.authData.authRowStatus = RS_NOTREADY;
		
		onu_auth_sync_msg_send ( brdIdx, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );

		if( onuAuthDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AUTH: send new null row auth-loid(%d) to pon%d/%d\r\n", loidIdx, brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_showCtcOnuAuthLoid( struct vty * vty, ULONG brdIdx, ULONG portIdx )
{
	ctc_onu_auth_t *pItem;
	short int PonPortIdx, OnuIdx;

	pItem = pg_onuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			vty_out( vty, " %-4d%-26s%-14s", pItem->authIdx, pItem->loidData.onu_id, pItem->loidData.password );
			if( pItem->authOnuLlid == 0 )
				vty_out( vty, "-\r\n" );
			else
			{
				PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
				OnuIdx = GetOnuIdxByLlid( PonPortIdx, pItem->authOnuLlid );
				if( OnuIdx == RERROR )
					vty_out( vty, "-\r\n" );
				else
					vty_out( vty, "%d/%d/%d\r\n", brdIdx, portIdx, OnuIdx+1 );
			}
		}
		pItem = pItem->next;
	}
	return VOS_OK;
}

#if 0
int CTC_addCtcOnuAuthLoid_by_loidIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd )
{
	PON_olt_id_t ponId;
	CTC_STACK_auth_loid_data_t loid_data;

	if( NULL == pLoid )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	VOS_MemZero( &loid_data, sizeof(loid_data) );
	if( VOS_StrLen(pLoid) > CTC_AUTH_ONU_ID_SIZE )
	{
		sys_console_printf( "Loid strings is too long\r\n" );
		return VOS_ERROR;
	}
	else
		VOS_StrCpy( loid_data.onu_id, pLoid );
	
	if( pPwd )
	{
		if( VOS_StrLen(pPwd) > CTC_AUTH_PASSWORD_SIZE )
		{
			sys_console_printf( "password strings is too long\r\n" );
			return VOS_ERROR;
		}
		else
			VOS_StrCpy( loid_data.password, pPwd );
	}

	onu_auth_loid_list_add( brdIdx, portIdx, loidIdx, pLoid, pPwd );

	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		if( CTC_STACK_add_auth_loid_data_by_index(ponId, loidIdx-1, loid_data) != CTC_STACK_EXIT_OK )
		{
			sys_console_printf("\r\n ERR:pon%d/%d-%d,loid=%s,pwd=%s\r\n", brdIdx, portIdx, loidIdx, pLoid, pPwd );
			return VOS_ERROR;
		}
		sys_console_printf(" ONU-AUTH: set auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
	}
	else
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_ADD;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.authData.authIdx = loidIdx;
		if( pLoid )
		{
			sndMsg.authData.authRowStatus = RS_ACTIVE;
			VOS_StrCpy( sndMsg.authData.loidData.onu_id, pLoid );
		}
		else
		{
			sndMsg.authData.authRowStatus = RS_NOTREADY;
		}
		if( pPwd ) VOS_StrCpy( sndMsg.authData.loidData.password, pPwd );
		
		onu_auth_sync_msg_send ( CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );

		sys_console_printf(" ONU-AUTH: send auth-loid(%d-%s) to pon%d/%d\r\n", loidIdx, pLoid, brdIdx, portIdx );
	}
	
	return VOS_OK;
}

int CTC_delCtcOnuAuthLoid_by_loidIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx )
{
	PON_olt_id_t ponId;
	/*CTC_STACK_auth_loid_data_t loid_data;*/

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	onu_auth_loid_list_free( brdIdx, portIdx, loidIdx, NULL );

	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		if( CTC_STACK_remove_auth_loid_data_by_index(ponId, loidIdx-1) != CTC_STACK_EXIT_OK )
		{
			sys_console_printf("\r\n delete loid error\r\n");
			return VOS_ERROR;
		}
		sys_console_printf(" ONU-AUTH: del auth-loid(%d) to pon%d/%d\r\n", loidIdx, brdIdx, portIdx );
	}
	else
	{
		ctc_onu_auth_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_DEL;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.authData.authIdx = loidIdx;
		sndMsg.authData.authRowStatus = RS_DESTROY;
		
		onu_auth_sync_msg_send ( CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );

		sys_console_printf(" ONU-AUTH: send del auth-loid(%d) to pon%d/%d\r\n", loidIdx, brdIdx, portIdx );
	}
	
	return VOS_OK;
}
#endif

/*added by wangying(gpon)*/
#if 0
LONG getFirstGponOnuAuthSNIdx( ULONG *pBrdIdx, ULONG *pPortIdx, ULONG *pSnIdx )
{
	ULONG brdIdx, portIdx;
	PON_olt_id_t ponId;
	gpon_onu_auth_t *pList;
	

	if( (NULL == pBrdIdx) || (NULL == pPortIdx) || (NULL == pSnIdx) )
		return VOS_ERROR;


	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue;    	

			pList = pg_gpononuAuthList[brdIdx][portIdx];
			while( pList )
			{
				if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
				{
					*pBrdIdx = brdIdx;
					*pPortIdx = portIdx;
					*pSnIdx = pList->authIdx;
					return VOS_OK;
				}
				pList = pList->next;
			}

		}
	}

	return VOS_ERROR;
}

LONG getNextGponOnuAuthSNIdx( ULONG brdIdx, ULONG portIdx, ULONG SnIdx, ULONG *pNextBrdIdx, ULONG *pNextPortIdx, ULONG *pNextSnIdx )
{
	ULONG mode;
	PON_olt_id_t ponId;
	gpon_onu_auth_t *pList;

	if( (NULL == pNextBrdIdx) || (NULL == pNextPortIdx) || (NULL == pNextSnIdx) )
		return VOS_ERROR;

	if( brdIdx == 0 )
		return getFirstGponOnuAuthSNIdx( pNextBrdIdx, pNextPortIdx, pNextSnIdx );

	if( portIdx == 0 )
	{
		SnIdx = 1;
		portIdx++;
	}
	else
		SnIdx++;
	
	if( SnIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE )
	{
		portIdx++;
		SnIdx = 1;
	}
	if( portIdx > MAX_PONPORT_PER_BOARD )
	{
		brdIdx++;
		portIdx = 1;
	}

	if( brdIdx > SYS_CHASSIS_SLOTNUM )
		return VOS_ERROR;

	if( SnIdx == 0 ) SnIdx++;
	
	/*loidId = loidIdx-1;*/
	for( ; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( ; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
			{
				SnIdx = 1;
				continue;    	
			}
			
			if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
			{
				/*if( mode != mn_ctc_auth_mode_disable )*/
				{

					pList = pg_gpononuAuthList[brdIdx][portIdx];
					while( pList )
					{
						if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
						{
							if( pList->authIdx >= SnIdx )
							{
								*pNextBrdIdx = brdIdx;
								*pNextPortIdx = portIdx;
								*pNextSnIdx = pList->authIdx;
								return VOS_OK;
							}
						}
						pList = pList->next;
					}

				}
			}
			SnIdx = 1;
		}
		portIdx = 1;
	}

	return VOS_ERROR;
}
#endif
/*ended by wangying*/

#if 1/*for gpon auth support*/
 gpon_onu_auth_t * gonu_auth_entry_list_get( ULONG brdIdx, ULONG portIdx, ULONG Index )
{
	gpon_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( pItem->authIdx == Index )
			{
				return pItem;
			}
			else if( pItem->authIdx > Index )
			{
				break;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}
static gpon_onu_auth_t * gonu_auth_entry_list_get_by_llid( ULONG brdIdx, ULONG portIdx, UCHAR llid )
{
	gpon_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD || 0 == llid )
		return NULL;

	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( pItem->authOnuLlid == llid )
			{
				return pItem;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}

gpon_onu_auth_t * gonu_auth_entry_list_seach( ULONG brdIdx, ULONG portIdx, CHAR * serial_no )
{
	gpon_onu_auth_t *pItem;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
		{
			if( VOS_StrCmp(pItem->authEntry.serial_no, serial_no) == 0 )
			{
				return pItem;
			}
		}
		pItem = pItem->next;
	}
	
	return NULL;
}

ULONG gonu_auth_entry_free_idx_get( ULONG brdIdx, ULONG portIdx )
{
	gpon_onu_auth_t *pItem;
	ULONG index = 1;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;

	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	while( pItem )
	{
		if( pItem->authIdx > index )
			break;

		pItem = pItem->next;
		index++;

		if( index > 128 )
		{
			index = 0;
			break;
		}
	}
	
	return index;
}

static gpon_onu_auth_t * gonu_auth_entry_list_new( ULONG brdIdx, ULONG portIdx, ULONG index )
{
	gpon_onu_auth_t *pItem;
	gpon_onu_auth_t *pPreItem;
	gpon_onu_auth_t *pNew;

	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return NULL;

	if( index == 0 )
		index = onu_auth_loid_free_idx_get( brdIdx, portIdx );
	if( index == 0 )
		return NULL;

	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	pPreItem = NULL;
	while( pItem )
	{
		if( pItem->authIdx == index )
		{
			if( pItem->authRowStatus == 1 )
				return NULL;
			else
				return pItem;
		}
		else if( pItem->authIdx > index )
		{
			break;
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	pNew = VOS_Malloc( sizeof(gpon_onu_auth_t), MODULE_ONU );
	if( pNew )
	{
		VOS_MemZero( pNew, sizeof(gpon_onu_auth_t) );
		pNew->authIdx = index;
		pNew->authRowStatus = RS_NOTREADY;
		pNew->next = pItem;

		if( pPreItem == NULL )
		{
			if( pItem == NULL )
			{
				pg_gpononuAuthList[brdIdx][portIdx] = pNew;
			}
			else
			{
				pg_gpononuAuthList[brdIdx][portIdx] = pNew;
			}
		}
		else
		{
			pPreItem->next = pNew;
		}
	}
	return pNew;
}

LONG gonu_auth_entry_list_free( ULONG brdIdx, ULONG portIdx, ULONG index, CHAR *serial_no )
{
	gpon_onu_auth_t *pItem;
	gpon_onu_auth_t *pPreItem;

	if( (index == 0) && (serial_no == NULL) )	/* 不能同时无效 */
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
	
	pItem = pg_gpononuAuthList[brdIdx][portIdx];
	pPreItem = NULL;
	while( pItem )
	{
		if( index )
		{
			if( pItem->authIdx == index )
			{
				if( pPreItem == NULL )
				{
					pg_gpononuAuthList[brdIdx][portIdx] = pg_gpononuAuthList[brdIdx][portIdx]->next;
					VOS_Free( pItem );
				}
				else
				{
					pPreItem->next = pItem->next;
					VOS_Free( pItem );
				}
				return VOS_OK;
			}
			else if( pItem->authIdx > index )
			{
				break;
			}
		}
		if( serial_no )
		{
			if( VOS_StrCmp(pItem->authEntry.serial_no, serial_no) == 0 )
			{
				if( pPreItem == NULL )
				{
					pg_gpononuAuthList[brdIdx][portIdx] = pg_gpononuAuthList[brdIdx][portIdx]->next;
					VOS_Free( pItem );
				}
				else
				{
					pPreItem->next = pItem->next;
					VOS_Free( pItem );
				}
				return VOS_OK;
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	return VOS_ERROR;
}


LONG gonu_auth_entry_list_add( ULONG brdIdx, ULONG portIdx, ULONG index, char *serial_no, char *pPwd )
{
	gpon_onu_auth_t *pList;

	if( serial_no == NULL )
		return VOS_ERROR;
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
    pList = gonu_auth_entry_list_seach(brdIdx, portIdx, serial_no);
    if(pList == NULL)
	    pList = gonu_auth_entry_list_new( brdIdx, portIdx, index );
    
    if(pList == NULL)
        pList = gonu_auth_entry_list_get(brdIdx, portIdx, index);
    if( pList )
    {
        pList->authRowStatus = RS_ACTIVE;
        VOS_MemCpy( pList->authEntry.serial_no, serial_no, GPON_ONU_SERIAL_NUM_STR_LEN-1 );
        if( pPwd )
			VOS_MemCpy( pList->authEntry.password, pPwd, GPON_ONU_PASSWARD_STR_LEN );

		return VOS_OK;
	}
	return VOS_ERROR;
}

/*added by wangying */

LONG mn_getGponOnuAuthSeriaNoRowStatus( ULONG brdIdx, ULONG portIdx, ULONG snIdx, ULONG *pStatus )
{
	PON_olt_id_t ponId;
	UCHAR loid_id;

	if( NULL == pStatus )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(snIdx == 0) || (snIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}
      
	{
		gpon_onu_auth_t *pList = gonu_auth_entry_list_get( brdIdx, portIdx, snIdx );
		if( pList )
		{
			*pStatus = pList->authRowStatus;
			return VOS_OK;
		}
	}

	return VOS_ERROR;
}


LONG mn_setGponOnuAuthSeriaNoRowStatus( ULONG brdIdx, ULONG portIdx, ULONG snIdx, ULONG status )
{
	PON_olt_id_t ponId;
	static gpon_onu_auth_t *pList;
	int rc = VOS_OK;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(snIdx == 0) || (snIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

#if 0
	if( status == RS_CREATEANDWAIT )
	{
		pList = gonu_auth_entry_list_get( brdIdx, portIdx, snIdx );
		if( pList )
		{
			rc = VOS_ERROR;
		}
		else
		{
			VOS_MemZero( &mn_gpon_sn_auth, sizeof(mn_gpon_sn_auth) );
			onu_auth_loid_cpy( mn_gpon_sn_auth.serial_no, GPON_AUTH_ENTRY_NOT_READY, GPON_AUTH_SN_SIZE );
			rc =gonu_auth_entry_list_new( brdIdx, portIdx, snIdx );

		}
	}
	else if( status == RS_ACTIVE )
	{
		rc = gonu_auth_entry_list_add( brdIdx, portIdx, snIdx, mn_gpon_sn_auth.serial_no, mn_gpon_sn_auth.password );
		VOS_MemZero( &mn_gpon_sn_auth, sizeof(mn_gpon_sn_auth) );
		onu_auth_loid_cpy( mn_gpon_sn_auth.serial_no, GPON_AUTH_ENTRY_NOT_READY, GPON_AUTH_SN_SIZE );
	}
	else if( status == RS_DESTROY )
	{
		VOS_MemZero( &mn_gpon_sn_auth, sizeof(mn_gpon_sn_auth) );
		onu_auth_loid_cpy( mn_gpon_sn_auth.serial_no, GPON_AUTH_ENTRY_NOT_READY, GPON_AUTH_SN_SIZE );
		rc = gonu_auth_entry_list_free( brdIdx, portIdx, snIdx, NULL );
	}
	else
	{
		rc = VOS_ERROR;
	}
#endif

#if 1  /*changed by wangying*/
if(( status == RS_CREATEANDGO ) ||( status == RS_CREATEANDWAIT)||( status == RS_ACTIVE ))
{
		rc =addGponOnuAuthSeriaNo( brdIdx, portIdx, snIdx);
}

else if( status == RS_DESTROY )
{
	rc = DelGonuEntryByIdx( brdIdx, portIdx, snIdx);
}
else
{
	rc = VOS_ERROR;
}

#endif



	return rc;
}

/*end by wangying*/

LONG mn_getFirstGponOnuAuthEntryIdx( ULONG *pBrdIdx, ULONG *pPortIdx, ULONG *Index )
{
	ULONG brdIdx, portIdx/*, loidId*/;
	PON_olt_id_t ponId;
	/*authentication_olt_database_t *pLoidDatabase;*/
	gpon_onu_auth_t *pList;

	if( (NULL == pBrdIdx) || (NULL == pPortIdx) || (NULL == Index) )
		return VOS_ERROR;


	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue; 
				
			pList = pg_gpononuAuthList[brdIdx][portIdx];
			while( pList )
			{
				if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
				{
					*pBrdIdx = brdIdx;
					*pPortIdx = portIdx;
					*Index = pList->authIdx;
					return VOS_OK;
				}
				pList = pList->next;
			}
		}
	}

	return VOS_ERROR;
}

LONG mn_getNextGponOnuAuthEntryIdx( ULONG brdIdx, ULONG portIdx, ULONG Index, ULONG *pNextBrdIdx, ULONG *pNextPortIdx, ULONG *pNextIndex )
{
	/*ULONG loidId;*/
	ULONG mode;
	PON_olt_id_t ponId;
	/*authentication_olt_database_t *pLoidDatabase;*/
	gpon_onu_auth_t *pList;

	if( (NULL == pNextBrdIdx) || (NULL == pNextPortIdx) || (NULL == pNextIndex) )
		return VOS_ERROR;

	if( brdIdx == 0 )
		return mn_getFirstGponOnuAuthEntryIdx( pNextBrdIdx, pNextPortIdx, pNextIndex );

	if( portIdx == 0 )
	{
		Index = 1;
		portIdx++;
	}
	else
		Index++;
	
	if( Index > PON_ONU_ID_PER_OLT_ARRAY_SIZE +1 )
	{
		portIdx++;
		Index = 1;
	}
	if( portIdx > MAX_PONPORT_PER_BOARD )
	{
		brdIdx++;
		portIdx = 1;
	}

	if( brdIdx > SYS_CHASSIS_SLOTNUM )
		return VOS_ERROR;

	if( Index == 0 ) Index++;
	
	/*loidId = loidIdx-1;*/
	for( ; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		/*if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;*//*解决空槽位预配置无法删除gpon认证表项的问题*/

		for( ; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
			{
				Index = 1;
				continue;    	
			}
			
			pList = pg_gpononuAuthList[brdIdx][portIdx];
			while( pList )
			{
				if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
				{
					if( pList->authIdx >= Index )
					{
						*pNextBrdIdx = brdIdx;
						*pNextPortIdx = portIdx;
						*pNextIndex = pList->authIdx;
						return VOS_OK;
					}
				}
				pList = pList->next;
			}
			Index = 1;
		}
		portIdx = 1;
	}

	return VOS_ERROR;
}

LONG ShowPonPortAuthGponOnuEntry( struct vty * vty, ULONG brdIdx, ULONG portIdx )
{
    gpon_onu_auth_t *pItem;
    short int PonPortIdx, OnuIdx;
    int entry_counter = 0;
    pItem = pg_gpononuAuthList[brdIdx][portIdx];
    if(pItem)
    {
        vty_out(vty, "Pon%d/%d:\r\n", brdIdx, portIdx);
        vty_out( vty, " No. %-26s%-14s\r\n", "       SN", "Password");
        vty_out( vty, "-----------------------------------------------\r\n");
    }
    else
    {
        return VOS_OK;
    }
    while( pItem )
    {
        if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
        {
            vty_out( vty, " %-4d%-26s%-14s\r\n", pItem->authIdx, pItem->authEntry.serial_no, pItem->authEntry.password );
            entry_counter++;
        }
        pItem = pItem->next;
    }
    vty_out( vty, "---------------Total number(%d)------------------\r\n\r\n", entry_counter);
    return VOS_OK;
}
int  ShowPonPortAuthGponOnuEntryALL( struct vty *vty )
{
	unsigned long Slot, Port, Entry=0;
	STATUS ret;
	ULONG entryCounter = 0;
	
	for( Slot = PONCARD_FIRST; Slot <= PONCARD_LAST; Slot ++ )
    {
        for( Port = 1; Port <= PONPORTPERCARD; Port ++ )
        {
            ShowPonPortAuthGponOnuEntry(vty, Slot, Port);
        }
    }

	return( ROK );

}


LONG CopyGponOnuAuthEntry(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    GPON_onu_auth_entry_t auth_entry;
    int liv_slot = GetCardIdxByPonChip(SrcPonPortIdx);
    int liv_pon = GetPonPortByPonChip(SrcPonPortIdx);    
    int liv_dstSlot = GetCardIdxByPonChip(DstPonPortIdx);
    int liv_dstPort = GetPonPortByPonChip(DstPonPortIdx);
    gpon_onu_auth_t *Src_pItem = NULL;
    gpon_onu_auth_t *Dst_pItem = NULL;
    
    if(liv_slot == RERROR || liv_pon == RERROR)
        return VOS_ERROR;
	/*if( (SrcPonPortIdx < 0) || (SrcPonPortIdx >= MAXPON)||(DstPonPortIdx < 0) || (DstPonPortIdx >= MAXPON)  )
	{
		return VOS_ERROR;    	
	}*/
    /*if (PonPortIsWorking(PonPortIdx) != TRUE)
        return VOS_ERROR;*/
    if(!SYS_MODULE_IS_GPON(liv_slot) || !SYS_MODULE_IS_GPON(liv_dstSlot))   
        return VOS_ERROR;
        
    Src_pItem = pg_gpononuAuthList[liv_slot][liv_pon];	
    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        while(Src_pItem)
        {
            gpon_onu_auth_entry_t entry;
            VOS_MemZero( &entry, sizeof(gpon_onu_auth_entry_t) ); 
            VOS_MemCpy(&entry.authEntry, &Src_pItem->authEntry, sizeof(GPON_onu_auth_entry_t));
            entry.authIdx = Src_pItem->authIdx;
            entry.authRowStatus = RS_CREATEANDGO;
            OLT_SetGponAuthEntry(DstPonPortIdx, &entry);
            
            Src_pItem = Src_pItem->next;            
        }
    
    }
    else
    {
        Dst_pItem = pg_gpononuAuthList[liv_slot][liv_pon];  
        if ( OLT_COPYFLAGS_COVER & CopyFlags || OLT_COPYFLAGS_ONLYNEW & CopyFlags)
        {
            while(Src_pItem)
            {
                gpon_onu_auth_entry_t entry;
                VOS_MemZero( &entry, sizeof(gpon_onu_auth_entry_t) ); 
                VOS_MemCpy(&entry.authEntry, &Src_pItem->authEntry, sizeof(GPON_onu_auth_entry_t));
                entry.authIdx = Src_pItem->authIdx;
                entry.authIdx = Src_pItem->authRowStatus = RS_CREATEANDGO;
                OLT_SetGponAuthEntry(DstPonPortIdx, &entry);
                
                Src_pItem = Src_pItem->next;            
            }        
        }
        else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
        {
        }	          
    }
  
	return VOS_OK;
}
int gpon_onu_auth_showrun(struct vty * vty, ULONG brdIdx, ULONG portIdx, int flag)
{
    int ret = 0;
	gpon_onu_auth_t *pItem;
    pItem = pg_gpononuAuthList[brdIdx][portIdx];
    if(!flag && pItem)
    {
        ret = 1;
        vty_out( vty, " pon %d/%d\r\n",brdIdx, portIdx) ;
    }
    while( pItem )
    {
        if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
        {
            if(pItem->authEntry.serial_no)
            {
            	/*begin: BUG 38125, 将认证条目的密码改为可选参数，mod by liuyh, 2017-5-11*/
                if (VOS_StrLen(pItem->authEntry.password) > 0)
                {
                    vty_out( vty, "  add gonu-register authentication entry %s %s\r\n", pItem->authEntry.serial_no, pItem->authEntry.password);
                }                
                else
                {
                    vty_out( vty, "  add gonu-register authentication entry %s\r\n", pItem->authEntry.serial_no);
                }
                /*end: mod by liuyh, 2017-5-11*/
            }
        }
        pItem = pItem->next;
    }
    return ret;

}
int DeleteGponAuthBySn(int PonPortIdx ,const *SN )
{
	gpon_onu_auth_entry_t entry;
	
	if(SN == NULL)
	{
		return CMD_WARNING;
	}
	VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
	VOS_MemCpy(entry.authEntry.serial_no,SN, GPON_ONU_SERIAL_NUM_STR_LEN-1);
	entry.authRowStatus = RS_DESTROY;
	OLT_SetGponAuthEntry(PonPortIdx, &entry);
}
#endif

int getGponOnuAuthSeriaNoAndPwd(ULONG ul_slot,ULONG ul_port,ULONG TableIdx,UCHAR *SN,UCHAR *Password)
{
	UCHAR ret =CMD_SUCCESS;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;
	short int PonPortIdx;

	pAuthData = gonu_auth_entry_list_get(ul_slot,ul_port,TableIdx);
	if(pAuthData == NULL)
	{
		ONU_GPONAUTH_DEBUG("%% This is no this TableIdx \r\n");
		return( CMD_SUCCESS );
	}
	
	VOS_MemCpy( Password,pAuthData->authEntry.password, GPON_ONU_PASSWARD_STR_LEN-1); 
	VOS_MemCpy(	SN,pAuthData->authEntry.serial_no, GPON_ONU_SERIAL_NUM_STR_LEN-1);	
 
    return ret;
}
int localsetGponOnuAuthSeriaNoAndPwd(ULONG ul_slot,ULONG ul_port,ULONG TableIdx,UCHAR *SN,UCHAR *Password)
{
	UCHAR ret =CMD_SUCCESS;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;
	short int PonPortIdx;

	if(SN == NULL && Password == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
        return( CMD_WARNING );
	}
	pAuthData = gonu_auth_entry_list_get(ul_slot,ul_port,TableIdx);
	if(pAuthData == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% This is no this TableIdx \r\n");
		return( CMD_SUCCESS );
	}
	
	if(Password != NULL)
		VOS_MemCpy(pAuthData->authEntry.password, Password, GPON_ONU_PASSWARD_STR_LEN-1); 
	if(SN != NULL)
		VOS_MemCpy(pAuthData->authEntry.serial_no,SN, GPON_ONU_SERIAL_NUM_STR_LEN-1);	
 
    return ret;
}

int setGponOnuAuthSeriaNoAndPwd(ULONG ul_slot,ULONG ul_port,ULONG TableIdx,UCHAR *SN,UCHAR *Password)
{

	ONU_GPONAUTH_DEBUG("  %% setGponOnuAuthSeriaNoAndPwd ul_slot = %d,ul_port = %d, TableIdx = %d,\r\n",ul_slot,ul_port,TableIdx);
	UCHAR ret =VOS_ERROR;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;
	short int PonPortIdx;

	if(SN == NULL && Password == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
        return( CMD_WARNING );
	}
	pAuthData = gonu_auth_entry_list_get(ul_slot,ul_port,TableIdx);
	if(pAuthData == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% This is no this TableIdx \r\n");
		return( CMD_SUCCESS );
	}
	
	VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
	if(Password != NULL)
		VOS_MemCpy(entry.authEntry.password, Password, GPON_ONU_PASSWARD_STR_LEN-1); 
	else
		VOS_MemCpy(entry.authEntry.password, pAuthData->authEntry.password, GPON_ONU_PASSWARD_STR_LEN-1); 

	if(SN != NULL)
	{
		VOS_MemCpy(entry.authEntry.serial_no,SN, GPON_ONU_SERIAL_NUM_STR_LEN-1);
		//OLT_ActiveOnePendingOnu(PonPortIdx,SN);
	}
	else
		VOS_MemCpy(entry.authEntry.serial_no,pAuthData->authEntry.serial_no, GPON_ONU_SERIAL_NUM_STR_LEN-1);

	PonPortIdx = GetPonPortIdxBySlot(ul_slot,ul_port);
	if(PonPortIdx == VOS_ERROR)
    {   
		ONU_GPONAUTH_DEBUG("  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
	
	entry.authIdx = TableIdx;
    entry.authRowStatus = RS_NOTINSERVICE;
    ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
    if(ret != VOS_OK)
    {
        ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
        return( CMD_WARNING );
    }

    /*b-解决通过网管加认证表onu不上线，加active一下onu就OK。added by zhaoxh 问题单39337*/
    if (SN != NULL)
    {
        OnuAuth_ActivatePendingOnuBySnMsg(PonPortIdx,SN);
    	ActivateGponPendingOnuMsg_conf(PonPortIdx,SN);    
    }
	/*e-解决通过网管加认证表onu不上线，加active一下onu就OK。added by zhaoxh */

    return ret;
}
int addGponOnuAuthSeriaNo(ULONG ul_slot,ULONG ul_port,ULONG TableIdx)
{
	ONU_GPONAUTH_DEBUG("  addGponOnuAuthSeriaNo ul_slot = %d,ul_port = %d, TableIdx = %d\r\n",ul_slot,ul_port,TableIdx);
	UCHAR ret =VOS_ERROR;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;
	short int PonPortIdx;

	pAuthData = gonu_auth_entry_list_get(ul_slot,ul_port,TableIdx);
	if(pAuthData != NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% This TableIdx is already exist\r\n");
		return( CMD_SUCCESS );
	}
	PonPortIdx = GetPonPortIdxBySlot(ul_slot,ul_port);
	if(PonPortIdx == VOS_ERROR)
    {   
		ONU_GPONAUTH_DEBUG("  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
	VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
    entry.authIdx = TableIdx;
    entry.authRowStatus = RS_CREATEANDGO;
    ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
    if(ret != VOS_OK)
    {
        ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
        return( CMD_WARNING );
    }
    return ret;
}

int DelGonuEntryByIdx(ULONG ul_slot,ULONG ul_port,ULONG TableIdx)
{
	ONU_GPONAUTH_DEBUG("  DelGonuEntryByIdx ul_slot = %d,ul_port = %d, TableIdx = %d,  \r\n",ul_slot,ul_port,TableIdx);
	short int  PonPortIdx = 0;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;	
	STATUS ret = 0;
	short int onuidx;
	
	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		ONU_GPONAUTH_DEBUG("  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
	VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));    	    
    
	pAuthData = gonu_auth_entry_list_get(ul_slot, ul_port,TableIdx);
	if(pAuthData == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% This TableIdx is not exist\r\n");
		return( CMD_WARNING );
	}
	if(ERROR != (onuidx = GetOnuIdxBySnPerPon(PonPortIdx, pAuthData->authEntry.serial_no)))
	{
		OnuMgt_DeregisterOnu(PonPortIdx,onuidx);
	}
	entry.authIdx = TableIdx;
	entry.authRowStatus = RS_DESTROY;
	ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
	if(ret != VOS_OK)
	{
	    ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
	    return( CMD_WARNING );
	}
	return ret;
}

int DelGonuEntryBySN(ULONG ul_slot,ULONG ul_port,UCHAR *SN)
{
	ONU_GPONAUTH_DEBUG("  DelGonuEntryByIdx ul_slot = %d,ul_port = %d, SN = %s,  \r\n",ul_slot,ul_port,SN);
	unsigned long  TableIdx = 0;
	short int  PonPortIdx = 0;
	gpon_onu_auth_entry_t entry;
	gpon_onu_auth_t  *pAuthData = NULL;	
	STATUS ret = 0;
	
	PonPortIdx = GetPonPortIdxBySlot(ul_slot, ul_port);
    if(PonPortIdx == VOS_ERROR)
    {   
		ONU_GPONAUTH_DEBUG("  %%  %d/%d is invalid\r\n", ul_slot, ul_port);
		return(CMD_WARNING );
    }
	VOS_MemZero(&entry, sizeof(gpon_onu_auth_entry_t));
	VOS_MemCpy(entry.authEntry.serial_no,SN, GPON_ONU_SERIAL_NUM_STR_LEN-1);	    	    
    
	pAuthData = gonu_auth_entry_list_seach(ul_slot, ul_port, entry.authEntry.serial_no);
	if(pAuthData == NULL)
	{
		ONU_GPONAUTH_DEBUG("  %% This serial number is not exist\r\n");
		return( CMD_WARNING );
	}

	entry.authRowStatus = RS_DESTROY;
	ret = OLT_SetGponAuthEntry(PonPortIdx, &entry);	 
	if(ret != VOS_OK)
	{
	    ONU_GPONAUTH_DEBUG("  %% Execting err\r\n");
	    return( CMD_WARNING );
	}
	return ret;
}



BOOL check_loid_data_is_used( UCHAR *pLoid )
{
	UCHAR loid[CTC_AUTH_ONU_ID_SIZE+1];
	onu_auth_loid_cpy( loid, pLoid, CTC_AUTH_ONU_ID_SIZE );
	if( loid[0] && (VOS_StrCmp(loid, CTC_AUTH_LOID_DATA_NOT_USED) != 0) )
		return TRUE;
	return FALSE;
}

LONG mn_getFirstCtcOnuAuthLoidIdx( ULONG *pBrdIdx, ULONG *pPortIdx, ULONG *pLoidIdx )
{
	ULONG brdIdx, portIdx/*, loidId*/;
	PON_olt_id_t ponId;
	/*authentication_olt_database_t *pLoidDatabase;*/
	ctc_onu_auth_t *pList;

	if( (NULL == pBrdIdx) || (NULL == pPortIdx) || (NULL == pLoidIdx) )
		return VOS_ERROR;


	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue;    	
#if 0
			pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
			if( pLoidDatabase == NULL )
			{
				continue;
			}

			for( loidId=0; loidId< PON_ONU_ID_PER_OLT_ARRAY_SIZE; loidId++ )
			{
				if( check_loid_data_is_used(pLoidDatabase->loid[loidId].onu_id) )
				{
					*pBrdIdx = brdIdx;
					*pPortIdx = portIdx;
					*pLoidIdx = loidId + 1;
					return VOS_OK;
				}
			}
#else
			pList = pg_onuAuthList[brdIdx][portIdx];
			while( pList )
			{
				if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
				{
					*pBrdIdx = brdIdx;
					*pPortIdx = portIdx;
					*pLoidIdx = pList->authIdx;
					return VOS_OK;
				}
				pList = pList->next;
			}
#endif
		}
	}

	return VOS_ERROR;
}

LONG mn_getNextCtcOnuAuthLoidIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pNextBrdIdx, ULONG *pNextPortIdx, ULONG *pNextLoidIdx )
{
	/*ULONG loidId;*/
	ULONG mode;
	PON_olt_id_t ponId;
	/*authentication_olt_database_t *pLoidDatabase;*/
	ctc_onu_auth_t *pList;

	if( (NULL == pNextBrdIdx) || (NULL == pNextPortIdx) || (NULL == pNextLoidIdx) )
		return VOS_ERROR;

	if( brdIdx == 0 )
		return mn_getFirstCtcOnuAuthLoidIdx( pNextBrdIdx, pNextPortIdx, pNextLoidIdx );

	if( portIdx == 0 )
	{
		loidIdx = 1;
		portIdx++;
	}
	else
		loidIdx++;
	
	if( loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE )
	{
		portIdx++;
		loidIdx = 1;
	}
	if( portIdx > MAX_PONPORT_PER_BOARD )
	{
		brdIdx++;
		portIdx = 1;
	}

	if( brdIdx > SYS_CHASSIS_SLOTNUM )
		return VOS_ERROR;

	if( loidIdx == 0 ) loidIdx++;
	
	/*loidId = loidIdx-1;*/
	for( ; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( ; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
			{
				loidIdx = 1;
				continue;    	
			}
			
			if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
			{
				/*if( mode != mn_ctc_auth_mode_disable )*/
				{
#if 0
					pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
					if( pLoidDatabase == NULL )
					{
						continue;
					}

					for( ; loidId<PON_ONU_ID_PER_OLT_ARRAY_SIZE; loidId++ )
					{
						if( check_loid_data_is_used(pLoidDatabase->loid[loidId].onu_id) )
						{
							*pNextBrdIdx = brdIdx;
							*pNextPortIdx = portIdx;
							*pNextLoidIdx = loidId + 1;
							return VOS_OK;
						}
					}
					loidId = 0;
#else
					pList = pg_onuAuthList[brdIdx][portIdx];
					while( pList )
					{
						if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
						{
							if( pList->authIdx >= loidIdx )
							{
								*pNextBrdIdx = brdIdx;
								*pNextPortIdx = portIdx;
								*pNextLoidIdx = pList->authIdx;
								return VOS_OK;
							}
						}
						pList = pList->next;
					}
#endif
				}
			}
			loidIdx = 1;
		}
		portIdx = 1;
	}

	return VOS_ERROR;
}

LONG mn_getCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid, char *pPwd )
{
	PON_olt_id_t ponId;
	/*UCHAR loid_id;
	authentication_olt_database_t *pLoidDatabase;*/
	ctc_onu_auth_t *pList;

	if( NULL == pLoid )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

#if 0
	pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
	if( pLoidDatabase == NULL )
	{
		sys_console_printf( "get loid database error\r\n" );
		return VOS_ERROR;
	}

	loid_id = loidIdx - 1;

	if( check_loid_data_is_used(pLoidDatabase->loid[loid_id].onu_id) )
	{
		if( VOS_StrCmp(pLoidDatabase->loid[loid_id].onu_id, CTC_AUTH_LOID_DATA_NOT_READY) == 0 ) 
		{
			onu_auth_loid_cpy( pLoid, mn_loid_data.onu_id, CTC_AUTH_ONU_ID_SIZE );
			onu_auth_loid_cpy( pPwd, mn_loid_data.password, CTC_AUTH_PASSWORD_SIZE );
		}
		else
		{
			onu_auth_loid_cpy( pLoid, pLoidDatabase->loid[loid_id].onu_id, CTC_AUTH_ONU_ID_SIZE );
			onu_auth_loid_cpy( pPwd, pLoidDatabase->loid[loid_id].password, CTC_AUTH_PASSWORD_SIZE );
		}
		return VOS_OK;
	}
#else
	pList = pg_onuAuthList[brdIdx][portIdx];
	while( pList )
	{
		if( (pList->authRowStatus != 0) && (pList->authRowStatus < 6) )
		{
			if( pList->authIdx == loidIdx )
			{
				onu_auth_loid_cpy( pLoid, pList->loidData.onu_id, CTC_AUTH_ONU_ID_SIZE );
				if( pPwd )
					onu_auth_loid_cpy( pPwd, pList->loidData.password, CTC_AUTH_PASSWORD_SIZE );
				return VOS_OK;
			}
			else if( pList->authIdx > loidIdx )
				break;
		}
		pList = pList->next;
	}
	
	onu_auth_loid_cpy( pLoid, mn_loid_data.onu_id, CTC_AUTH_ONU_ID_SIZE );
	onu_auth_loid_cpy( pPwd, mn_loid_data.password, CTC_AUTH_PASSWORD_SIZE );
#endif

	return VOS_OK;
}

LONG mn_setCtcOnuAuthLoid( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pLoid )
{
	PON_olt_id_t ponId;

	if( NULL == pLoid )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	if( VOS_StrLen(pLoid) > CTC_AUTH_ONU_ID_SIZE )
	{
		sys_console_printf( "Loid strings is too long\r\n" );
		return VOS_ERROR;
	}
	else
	{
		VOS_StrCpy( mn_loid_data.onu_id, pLoid );
	}
	
	return VOS_OK;
}

LONG mn_setCtcOnuAuthLoidPassword( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, char *pPwd )
{
	PON_olt_id_t ponId;

	if( NULL == pPwd )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	if( VOS_StrLen(pPwd) > CTC_AUTH_PASSWORD_SIZE )
	{
		sys_console_printf( "password strings is too long\r\n" );
		return VOS_ERROR;
	}
	else
		VOS_StrCpy( mn_loid_data.password, pPwd );
	
	return VOS_OK;
}

LONG mn_getCtcOnuAuthLoidOnuDevIdx( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pOnuDevIdx )
{
	PON_olt_id_t ponId, onuId;
	UCHAR loid_id;
	authentication_olt_database_t *pLoidDatabase;

	if( NULL == pOnuDevIdx )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}
#if 0
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
		if( pLoidDatabase == NULL )
		{
			sys_console_printf( "get loid database error\r\n" );
			return VOS_ERROR;
		}

		loid_id = loidIdx - 1;
		
		if( check_loid_data_is_used(pLoidDatabase->loid[loid_id].onu_id) )
		{
			if( PON_NOT_USED_LLID == pLoidDatabase->onu_llid[loid_id] )
				*pOnuDevIdx = 0;
			else
				*pOnuDevIdx = onuIdToOnuIndex( ponId, pLoidDatabase->onu_llid[loid_id] );
			return VOS_OK;
		}
	}
	else
#endif        
	{
		ctc_onu_auth_t *pItem;

		*pOnuDevIdx = 0;
		pItem = onu_auth_loid_list_get( brdIdx, portIdx, loidIdx );
		if( pItem )
		{
			ponId = GetPonPortIdxBySlot( brdIdx, portIdx );
			onuId = GetOnuIdxByLlid( ponId, pItem->authOnuLlid );
			if( onuId != RERROR )
				*pOnuDevIdx = MAKEDEVID( brdIdx, portIdx, onuId+1 );
		}

		return VOS_OK;
	}

	return VOS_ERROR;
}

LONG mn_getCtcOnuAuthLoidRowStatus( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG *pStatus )
{
	PON_olt_id_t ponId;
	UCHAR loid_id;
	authentication_olt_database_t *pLoidDatabase;

	if( NULL == pStatus )
		return VOS_ERROR;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}
#if 0
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
		if( pLoidDatabase == NULL )
		{
			sys_console_printf( "get loid database error\r\n" );
			return VOS_ERROR;
		}

		loid_id = loidIdx - 1;
		
		if( check_loid_data_is_used(pLoidDatabase->loid[loid_id].onu_id) )
		{
			if( VOS_StrCmp(pLoidDatabase->loid[loid_id].onu_id, CTC_AUTH_LOID_DATA_NOT_READY) == 0 )
				*pStatus = RS_NOTREADY;
			else
				*pStatus = RS_ACTIVE;
			return VOS_OK;
		}
	}
	else
#endif        
	{
		ctc_onu_auth_t *pList = onu_auth_loid_list_get( brdIdx, portIdx, loidIdx );
		if( pList )
		{
			*pStatus = pList->authRowStatus;
			return VOS_OK;
		}
	}

	return VOS_ERROR;
}

LONG mn_setCtcOnuAuthLoidRowStatus( ULONG brdIdx, ULONG portIdx, ULONG loidIdx, ULONG status )
{
	PON_olt_id_t ponId;
	/*UCHAR loid_id;
	authentication_olt_database_t *pLoidDatabase;*/
	static ctc_onu_auth_t *pList;
	int rc = VOS_OK;

	if( ( SlotCardIsPonBoard(brdIdx) == VOS_ERROR ) || (portIdx == 0) || (portIdx > MAX_PONPORT_PER_BOARD) ||
		(loidIdx == 0) || (loidIdx > PON_ONU_ID_PER_OLT_ARRAY_SIZE) )
		return VOS_ERROR;

	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return VOS_ERROR;    	
	}

	/*pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
	if( pLoidDatabase == NULL )
	{
		sys_console_printf( "get loid database error\r\n" );
		return VOS_ERROR;
	}

	loid_id = loidIdx - 1;*/

	if( status == RS_CREATEANDWAIT )
	{
		pList = onu_auth_loid_list_get( brdIdx, portIdx, loidIdx );
		if( pList )
		/*if( check_loid_data_is_used(pLoidDatabase->loid[loid_id].onu_id) )*/
		{
			rc = VOS_ERROR;
		}
		else
		{
			VOS_MemZero( &mn_loid_data, sizeof(mn_loid_data) );
			onu_auth_loid_cpy( mn_loid_data.onu_id, CTC_AUTH_LOID_DATA_NOT_READY, CTC_AUTH_ONU_ID_SIZE );
			rc = mn_createCtcOnuAuthLoid( brdIdx, portIdx, loidIdx );
		}
	}
	else if( status == RS_ACTIVE )
	{
		rc = mn_addCtcOnuAuthLoid( brdIdx, portIdx, loidIdx, mn_loid_data.onu_id, mn_loid_data.password );
		VOS_MemZero( &mn_loid_data, sizeof(mn_loid_data) );
		onu_auth_loid_cpy( mn_loid_data.onu_id, CTC_AUTH_LOID_DATA_NOT_READY, CTC_AUTH_ONU_ID_SIZE );
	}
	else if( status == RS_DESTROY )
	{
		VOS_MemZero( &mn_loid_data, sizeof(mn_loid_data) );
		onu_auth_loid_cpy( mn_loid_data.onu_id, CTC_AUTH_LOID_DATA_NOT_READY, CTC_AUTH_ONU_ID_SIZE );
		rc = mn_delCtcOnuAuthLoid( brdIdx, portIdx, loidIdx, NULL );
	}
	else
	{
		rc = VOS_ERROR;
	}

	return rc;
}

DEFUN(
	show_pon_ctc_auth_mode,
	show_pon_ctc_auth_mode_cmd,
	"show onu-register authentication mode",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication mode\n"
	"please input the pon port\n"
    )
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuId = 0; 
	INT16  ponId = 0;
	ULONG mode = 0;


	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx , &onuId, &ponId) != VOS_OK )
		return CMD_WARNING;    	

	mn_getCtcOnuAuthMode( brdIdx, portIdx, &mode );
	if(SYS_MODULE_IS_GPON(brdIdx) && mode == mn_ctc_auth_mode_mac)
		vty_out( vty, "onu-register authentication mode is SN \r\n");
	else
		vty_out( vty, "onu-register authentication mode is %s \r\n", onu_auth_mode_to_str(mode) );

	return CMD_SUCCESS;
}

DEFUN(
	show_olt_ctc_auth_mode,
	show_olt_ctc_auth_mode_cmd,
	"show onu-register authentication mode <slot/port>",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication mode\n"
	"please input the pon port\n"
    )
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG mode = 0;

    VOS_Sscanf(argv[0], "%d/%d", &brdIdx, &portIdx);                		
	mn_getCtcOnuAuthMode( brdIdx, portIdx, &mode );
	if(SYS_MODULE_IS_GPON(brdIdx) && mode == mn_ctc_auth_mode_mac)
		vty_out( vty, "onu-register authentication mode is SN \r\n");
	else
		vty_out( vty, "onu-register authentication mode is %s \r\n", onu_auth_mode_to_str(mode) );

	return CMD_SUCCESS;
}

DEFUN(
	config_pon_ctc_auth_config,
	config_pon_ctc_auth_config_cmd,
	"ctc auth-config <slot/port> {[disable|mac|loid|hybrid|loid-not-chk-pwd|hybrid-not-chk-pwd]}*1",
	CTC_STR
	"Config CTC ONU auth config\n"
	"please input the pon port\n"	
	"disable auth\n"
	"auth based mac addr.\n"
	"auth based loid and password\n"
	"auth based loid or mac\n"
	"auth based loid but no password\n"
	"auth based hybrid but no password\n"
	)
{
	ULONG brdIdx = 0, brdPartnerIdx = 0;
	ULONG portIdx = 0, portPartnerIdx = 0;	
	ULONG mode = 0;
	ULONG enable = 0;
	short int PonPortIdx = 0;

    IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );
    if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
        return(CMD_WARNING);

	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
        return CMD_WARNING;
	}

    if(SYS_MODULE_IS_CPU_PON(brdIdx) && brdIdx != SYS_LOCAL_MODULE_SLOTNO)
    {
        return CMD_WARNING;
    }  
    
	PonPortIdx = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
    if(argc == 2)
    {
    	if( VOS_StrCmp(argv[1], "mac") == 0 )
    		mode = mn_ctc_auth_mode_mac;
    	else if( VOS_StrCmp(argv[1], "loid") == 0 )
    		mode = mn_ctc_auth_mode_loid;
    	else if( VOS_StrCmp(argv[1], "hybrid") == 0 )
    		mode = mn_ctc_auth_mode_hybrid;
    	else if( VOS_StrCmp(argv[1], "loid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_loid_no_pwd;
    	else if( VOS_StrCmp(argv[1], "hybrid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_hybrid_no_pwd;
    	else
    		mode = mn_ctc_auth_mode_disable;
        
    	if( mn_setCtcOnuAuthMode1(brdIdx, portIdx, mode) == VOS_OK )
    	{
			/*added by wangjiah@2017-05-06:begin
			 * to support config sync for pon protect
			 * */
			if(VOS_OK == PonPortSwapSlotQuery(PonPortIdx, &brdPartnerIdx, &portPartnerIdx))
			{
				if( mn_setCtcOnuAuthMode1(brdPartnerIdx, portPartnerIdx, mode) == VOS_OK )
				{
					vty_out( vty, "ctc-auth-config auto-protect partner:ERR\r\n" );
				}
			}
			/*added by wangjiah@2017-05-06:end*/
    		return CMD_SUCCESS;
    	}
    	vty_out( vty, "ctc-auth-config:ERR\r\n" );
    }
    else
    {
        ULONG enable = 0;
        short int PonPortIdx = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );

        vty_out(vty, " Pon %d/%d:\r\n", brdIdx, portIdx);
        if(PAS_get_authorize_mac_address_according_list_mode(PonPortIdx, &enable) == VOS_OK)
        {
            vty_out(vty, "  auth enable: %d\r\n", enable);
        }
        if(CTC_STACK_get_auth_mode(PonPortIdx, &mode) == VOS_OK)
        {
            vty_out(vty, "  auth mode: %d\r\n", mode);
        }
    }
	return CMD_WARNING;
}

#if 0
DEFUN(
	config_pon_ctc_auth_mode,
	config_pon_ctc_auth_mode_cmd,
	"ctc auth-mode [disable|mac|loid|hybrid|loid-not-chk-pwd|hybrid-not-chk-pwd]",
	CTC_STR
	"Config CTC ONU auth mode\n"
	"disable auth\n"
	"auth based mac addr.\n"
	"auth based loid and password\n"
	"auth based loid or mac\n"
	"auth based loid but no password\n"
	"auth based hybrid but no password\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuId = 0; 
	INT16  ponId = 0;
	ULONG mode = 0;

	if( VOS_StrCmp(argv[0], "mac") == 0 )
		mode = mn_ctc_auth_mode_mac;
	else if( VOS_StrCmp(argv[0], "loid") == 0 )
		mode = mn_ctc_auth_mode_loid;
	else if( VOS_StrCmp(argv[0], "hybrid") == 0 )
		mode = mn_ctc_auth_mode_hybrid;
	else if( VOS_StrCmp(argv[0], "loid-not-chk-pwd") == 0 )
		mode = mn_ctc_auth_mode_loid_no_pwd;
	else if( VOS_StrCmp(argv[0], "hybrid-not-chk-pwd") == 0 )
		mode = mn_ctc_auth_mode_hybrid_no_pwd;
	else
		mode = mn_ctc_auth_mode_disable;

	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx, &onuId, &ponId) != VOS_OK )
		return CMD_WARNING;    	
		
	if( mn_setCtcOnuAuthMode(brdIdx, portIdx, mode) == VOS_OK )
	{
		return CMD_SUCCESS;
	}
	vty_out( vty, "ctc-auth-mode:ERR\r\n" );
	
	return CMD_WARNING;
}
#else
DEFUN(
	config_pon_ctc_auth_mode,
	config_pon_ctc_auth_mode_cmd,
	"onu-register authentication mode {[mac|loid|hybrid|loid-not-chk-pwd|hybrid-not-chk-pwd|sn|sn-pwd]}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mode\n"
	"auth based mac addr\n"
	"auth based loid and password\n"
	"auth based loid or mac\n"
	"auth based loid but no password\n"
	"auth based hybrid but no password\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuId = 0; 
	INT16  ponId = 0;
	ULONG mode = 0;
    if(argc == 1)
    {
    	if( VOS_StrCmp(argv[0], "mac") == 0 )
    		mode = mn_ctc_auth_mode_mac;
    	else if( VOS_StrCmp(argv[0], "loid") == 0 )
    		mode = mn_ctc_auth_mode_loid;
    	else if( VOS_StrCmp(argv[0], "hybrid") == 0 )
    		mode = mn_ctc_auth_mode_hybrid;
    	else if( VOS_StrCmp(argv[0], "loid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_loid_no_pwd;
    	else if( VOS_StrCmp(argv[0], "hybrid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_hybrid_no_pwd;
    	else if( VOS_StrCmp(argv[0], "sn-pwd") == 0 )
    		mode = mn_gpon_auth_mode_sn_and_pwd;
		else if( VOS_StrCmp(argv[0], "sn") == 0 )
			mode = mn_ctc_auth_mode_mac;
		else
    		mode = mn_ctc_auth_mode_disable;

    	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx, &onuId, &ponId) != VOS_OK )
    		return CMD_WARNING;    	
    		
    	if( mn_setCtcOnuAuthMode(brdIdx, portIdx, mode) == VOS_OK )
    	{
    		return CMD_SUCCESS;
    	}
    	vty_out( vty, "ctc-auth-mode:ERR\r\n" );
    }
    else
    {
    	if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
    	{
        	vty_out( vty, "onu-register authentication mode is %s\r\n", onu_auth_mode_to_str(mode) );
    	}
    }
	return CMD_WARNING;
}
#endif
DEFUN(
	config_olt_ctc_gpon_auth_mode,
	config_olt_ctc_gpon_auth_mode_cmd,
	"onu-register authentication mode <slot/port> {[sn|sn-pwd]}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mode\n"
	"please input the pon port\n"
	"auth based on Serial_Number\n"
	"auth based on Serial_Number and password\n"
	)
{	 
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	/*ULONG onuId = 0; */
	/*INT16  ponId = 0;*/
	ULONG mode = 0;
    
    IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );
    if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
        return(CMD_WARNING);

    if(argc == 2)
    {
    	if( VOS_StrCmp(argv[1], "sn-pwd") == 0 )
    		mode = mn_gpon_auth_mode_sn_and_pwd;
		else if( VOS_StrCmp(argv[1], "sn") == 0 )
			mode = mn_ctc_auth_mode_mac;
    	else
    		mode = mn_ctc_auth_mode_disable;
    		
    	if( mn_setCtcOnuAuthMode(brdIdx, portIdx, mode) == VOS_OK )
    	{
    		return CMD_SUCCESS;
    	}
    	vty_out( vty, "Set onu-register authentication mode ERROR!\r\n" );
    }
    else
    {
    	if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
    	{
    		if(mode == mn_ctc_auth_mode_mac)
				vty_out( vty, "onu-register authentication mode is SN \r\n");
			else
        		vty_out( vty, "onu-register authentication mode is %s\r\n", onu_auth_mode_to_str(mode) );
    	}
    }
	return CMD_WARNING;
}
DEFUN(
	config_olt_ctc_epon_auth_mode,
	config_olt_ctc_epon_auth_mode_cmd,
	"onu-register authentication mode <slot/port> {[mac|loid|hybrid|loid-not-chk-pwd|hybrid-not-chk-pwd|sn|sn-pwd]}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication mode\n"
	"please input the pon port\n"
	"auth based on mac addr.\n"
	"auth based on loid\n"
	"auth based on loid or mac\n"
	"auth based on loid without password\n"
	"auth based on hybrid without password\n"
	)
{	 
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	/*ULONG onuId = 0; */
	/*INT16  ponId = 0;*/
	ULONG mode = 0;
    
    IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );
    if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
        return(CMD_WARNING);

    if(argc == 2)
    {
    	if( VOS_StrCmp(argv[1], "mac") == 0 )
    		mode = mn_ctc_auth_mode_mac;
    	else if( VOS_StrCmp(argv[1], "loid") == 0 )
    		mode = mn_ctc_auth_mode_loid;
    	else if( VOS_StrCmp(argv[1], "hybrid") == 0 )
    		mode = mn_ctc_auth_mode_hybrid;
    	else if( VOS_StrCmp(argv[1], "loid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_loid_no_pwd;
    	else if( VOS_StrCmp(argv[1], "hybrid-not-chk-pwd") == 0 )
    		mode = mn_ctc_auth_mode_hybrid_no_pwd;
		else if( VOS_StrCmp(argv[1], "sn-pwd") == 0 )
    		mode = mn_gpon_auth_mode_sn_and_pwd;
		else if( VOS_StrCmp(argv[1], "sn") == 0 )
			mode = mn_ctc_auth_mode_mac;
    	else
    		mode = mn_ctc_auth_mode_disable;
    		
    	if( mn_setCtcOnuAuthMode(brdIdx, portIdx, mode) == VOS_OK )
    	{
    		return CMD_SUCCESS;
    	}
    	vty_out( vty, "Set onu-register authentication mode ERROR!\r\n" );
    }
    else
    {
    	if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
    	{
        	vty_out( vty, "onu-register authentication mode is %s\r\n", onu_auth_mode_to_str(mode) );
    	}
    }
	return CMD_WARNING;
}

DEFUN(
	add_pon_ctc_auth_loid_data,
	add_pon_ctc_auth_loid_data_cmd,
	"add onu-register authentication loid <loid> {<pwd>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication loid table\n"
	"input Logical Onu ID, max len=24\n"
	"input CTC ONU auth password, max len=12\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	ULONG onuId;
	INT16 ponId = 0;
	int rc;

	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx, &onuId, &ponId) != VOS_OK )
		return CMD_WARNING;    	
    if( PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx, vty) != ROK )
        return(CMD_WARNING);
		
	if( argc == 2 )
	{
		rc = mn_addCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[0], argv[1] );
	}
	else
		rc = mn_addCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[0], NULL );

	if( rc == VOS_ERROR )
	{
		vty_out( vty, "add error!\r\n" );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(
	del_pon_ctc_auth_loid_data,
	del_pon_ctc_auth_loid_data_cmd,
	/*"ctc auth-loid delete <loid> {<pwd>}*1",*/
	"delete onu-register authentication loid <loid>",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication loid table\n"
	"input Logical Onu ID,max len=24\n"
	/*"input CTC ONU auth password\n"*/
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	ULONG onuId;
	INT16 ponId = 0;
	int rc;
		
	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx, &onuId, &ponId) != VOS_OK )
		return CMD_WARNING;    	
		
	rc = mn_delCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[0] );

	if( rc == VOS_ERROR )
	{
		vty_out( vty, "delete error!\r\n" );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(
	show_ctc_auth_loid_data,
	show_ctc_auth_loid_data_cmd,
	"show onu-register authentication loid",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication loid table\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	LONG onuId;
	INT16 ponId = 0;
	/*int i;*/

	/*UCHAR loid[CTC_AUTH_ONU_ID_SIZE+1];*/
	/*UCHAR password[CTC_AUTH_PASSWORD_SIZE+1];*/

	/*authentication_olt_database_t *pLoidDatabase;*/
	
	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx, (ULONG*)&onuId, &ponId) != VOS_OK )
		return CMD_WARNING;    	

	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return CMD_WARNING;    	
	}

	vty_out( vty, " No. %-26s%-14s%s\r\n", "LOID", "Password", "ONU" );
#if 0
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
		if( pLoidDatabase == NULL )
		{
			vty_out( vty, "Loid database error\r\n" );
			return CMD_WARNING;
		}

		for( i=0; i<PON_ONU_ID_PER_OLT_ARRAY_SIZE; i++ )
		{
			if( check_loid_data_is_used(pLoidDatabase->loid[i].onu_id) )
			{
				onu_auth_loid_cpy( loid, pLoidDatabase->loid[i].onu_id, CTC_AUTH_ONU_ID_SIZE );
				onu_auth_loid_cpy( password, pLoidDatabase->loid[i].password, CTC_AUTH_PASSWORD_SIZE );
				vty_out( vty, " %-4d%-26s%-14s", i+1, loid, password );
				if( (PON_NOT_USED_LLID == pLoidDatabase->onu_llid[i]) ||
					((onuId = GetOnuIdxByLlid( ponId, pLoidDatabase->onu_llid[i])) == VOS_ERROR) )
				{
					vty_out( vty, "-\r\n" );
				}
				else
				{
					vty_out( vty, "%d/%d/%d\r\n", brdIdx, portIdx, onuId+1 );
				}
			}
		}
	}
	else
#endif
	{
		mn_showCtcOnuAuthLoid( vty, brdIdx, portIdx );
	}
	
	return CMD_SUCCESS;
}
DEFUN(
	add_olt_ctc_auth_loid_data,
	add_olt_ctc_auth_loid_data_cmd,
	"add onu-register authentication loid <slot/port> <loid> {<pwd>}*1",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication loid table\n"
	"please input the pon port\n"
	"input Logical Onu ID, max len=24\n"
	"input CTC ONU auth password, max len=12\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;


	int rc;

    VOS_Sscanf(argv[0], "%d/%d", &brdIdx, &portIdx);                		
	if( argc == 3 )
	{
		rc = mn_addCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[1], argv[2] );
	}
	else
		rc = mn_addCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[1], NULL );

	if( rc == VOS_ERROR )
	{
		vty_out( vty, "add error!\r\n" );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(
	del_olt_ctc_auth_loid_data,
	del_olt_ctc_auth_loid_data_cmd,
	"delete onu-register authentication loid <slot/port> <loid>",
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication information\n"
	"Config onu-register authentication loid table\n"
	"please input the pon port\n"
	"input Logical Onu ID,max len=24\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;


	int rc;
		
    VOS_Sscanf(argv[0], "%d/%d", &brdIdx, &portIdx);                		
	rc = mn_delCtcOnuAuthLoid( brdIdx, portIdx, 0, argv[1] );

	if( rc == VOS_ERROR )
	{
		vty_out( vty, "delete error!\r\n" );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(
	show_olt_auth_loid_data,
	show_olt_auth_loid_data_cmd,
	"show onu-register authentication loid <slot/port>",
	SHOW_STR
	"show onu-register authentication information\n"
	"show onu-register authentication information\n"
	"show onu-register authentication loid table\n"
	"please input the pon port\n"	
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	/*LONG onuId;*/
	INT16 ponId = 0;
	/*int i;*/

	/*UCHAR loid[CTC_AUTH_ONU_ID_SIZE+1];*/
	/*UCHAR password[CTC_AUTH_PASSWORD_SIZE+1];*/

	/*authentication_olt_database_t *pLoidDatabase;*/
    VOS_Sscanf(argv[0], "%d/%d", &brdIdx, &portIdx);                		
 	
    ponId = GetPonPortIdxBySlot(brdIdx, portIdx);
	if( (ponId < 0) || (ponId >= MAXPON) )
	{
		return CMD_WARNING;    	
	}

	vty_out( vty, " No. %-26s%-14s%s\r\n", "LOID", "Password", "ONU" );
#if 0
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
		if( pLoidDatabase == NULL )
		{
			vty_out( vty, "Loid database error\r\n" );
			return CMD_WARNING;
		}

		for( i=0; i<PON_ONU_ID_PER_OLT_ARRAY_SIZE; i++ )
		{
			if( check_loid_data_is_used(pLoidDatabase->loid[i].onu_id) )
			{
				onu_auth_loid_cpy( loid, pLoidDatabase->loid[i].onu_id, CTC_AUTH_ONU_ID_SIZE );
				onu_auth_loid_cpy( password, pLoidDatabase->loid[i].password, CTC_AUTH_PASSWORD_SIZE );
				vty_out( vty, " %-4d%-26s%-14s", i+1, loid, password );
				if( (PON_NOT_USED_LLID == pLoidDatabase->onu_llid[i]) ||
					((onuId = GetOnuIdxByLlid( ponId, pLoidDatabase->onu_llid[i])) == VOS_ERROR) )
				{
					vty_out( vty, "-\r\n" );
				}
				else
				{
					vty_out( vty, "%d/%d/%d\r\n", brdIdx, portIdx, onuId+1 );
				}
			}
		}
	}
	else
#endif     
	{
		mn_showCtcOnuAuthLoid( vty, brdIdx, portIdx );
	}
	
	return CMD_SUCCESS;
}
DEFUN( onu_auth_debug,
	onu_auth_debug_cmd,
	"debug onu-ctc-auth",
	"Debug information\n"
	"CTC ONU auth debug switch\n")
{
	onuAuthDebugSwitch = 1;
	return CMD_SUCCESS;
}

DEFUN( undo_onu_auth_debug,
	undo_onu_auth_debug_cmd,
	"undo debug onu-ctc-auth",
	"Debug information\n"
	"CTC ONU auth debug switch\n")
{
	onuAuthDebugSwitch = 0;

	return CMD_SUCCESS;
}

/*DEFUN(
	show_ctc_auth_mode,
	show_ctc_auth_mode_cmd,
	"show ctc auth-mode <slot/port>",
	SHOW_STR
	CTC_STR
	"show CTC ONU auth mode\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	INT16 ponId = 0;
	ULONG mode = 0;

	IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );
	if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
		return CMD_WARNING;
	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if (ponId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	mn_getCtcOnuAuthMode( brdIdx, portIdx, &mode );
	vty_out( vty, "ctc-auth-mode:%s\r\n", onu_auth_mode_to_str(mode) );

	return CMD_SUCCESS;
}
DEFUN(
	add_ctc_auth_loid_data,
	add_ctc_auth_loid_data_cmd,
	"ctc auth-loid add <slot/port> <loid> {<pwd>}*1",
	CTC_STR
	"add loid data\n"
	"input PON slot and port\n"
	"input Logical Onu ID\n"
	"input CTC ONU auth password\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	INT16 ponId = 0;
	int ret;
		
	IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );

	if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
		return CMD_WARNING;
	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if (ponId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( argc == 3 )
	{
		ret = mn_addCtcOnuAuthLoid( brdIdx, portIdx, argv[1], argv[2] );
	}
	else
		ret = mn_addCtcOnuAuthLoid( brdIdx, portIdx, argv[1], NULL );
	
	if( ret == VOS_ERROR )
	{
		vty_out( vty, "add error\r\n" );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(
	del_ctc_auth_loid_data,
	del_ctc_auth_loid_data_cmd,
	"ctc auth-loid delete <slot/port> <loid> {<pwd>}*1",
	CTC_STR
	"delete loid data\n"
	"input PON slot and port\n"
	"input Logical Onu ID\n"
	"input CTC ONU auth password\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
    	INT16 ponId = 0;
	INT16 ret;
	CTC_STACK_auth_loid_data_t loid_data;
		

	IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );

	if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
		return CMD_WARNING;
	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if (ponId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	VOS_MemZero( &loid_data, sizeof(loid_data) );
	if( VOS_StrLen(argv[1]) > CTC_AUTH_ONU_ID_SIZE )
	{
		vty_out( vty, "Loid strings is too long\r\n" );
		return CMD_WARNING;
	}
	else
		VOS_StrCpy( loid_data.onu_id, argv[1] );
	
	if( argc == 3 )
	{
		if( VOS_StrLen(argv[2]) > CTC_AUTH_PASSWORD_SIZE )
		{
			vty_out( vty, "password strings is too long\r\n" );
			return CMD_WARNING;
		}
		else
			VOS_StrCpy( loid_data.password, argv[2] );
	}
	if( (ret = CTC_STACK_remove_auth_loid_data(ponId, loid_data)) != CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "delete loid Err(ponId=%d,errcode=%d)\r\n", ponId, ret );
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}
DEFUN(
	show_ctc_auth_loid_data,
	show_ctc_auth_loid_data_cmd,
	"show ctc auth-loid <slot/port>",
	SHOW_STR
	CTC_STR
	"show loid data\n"
	"input PON slot and port\n"
	)
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
    	INT16 ponId = 0;
	INT16 ret;
	UCHAR num_entry = PON_ONU_ID_PER_OLT_ARRAY_SIZE;
	CTC_STACK_auth_loid_data_t loid_data_entry[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
	PON_onu_id_t loid_llid_data_entry[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
	int i, n;

	IFM_ParseSlotPort( argv[0], &brdIdx, &portIdx );

	if(PonCardSlotPortCheckWhenRunningByVty(brdIdx, portIdx,vty) != ROK)
		return CMD_WARNING;
	ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
	if (ponId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	VOS_MemZero( &loid_data_entry, sizeof(loid_data_entry) );
	VOS_MemZero( &loid_llid_data_entry, sizeof(loid_llid_data_entry) );

	if( (ret = CTC_STACK_get_auth_loid_llid_data_table(ponId, &loid_llid_data_entry[0], &num_entry)) != CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "get loid llid Err(ponId=%d,errcode=%d\r\n", ponId, ret );
		return CMD_WARNING;
	}
	if( (ret = CTC_STACK_get_auth_loid_data_table(ponId, &loid_data_entry[0], &num_entry)) != CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "get loid Err(ponId=%d,errcode=%d\r\n", ponId, ret );
		return CMD_WARNING;
	}

	if( num_entry )
	{
		vty_out( vty, " No. %-26s%-12s%s\r\n", "LOID", "Password", "ONU" );
		for( i=0, n=0; i<num_entry; i++ )
		{
			if( loid_data_entry[i].onu_id[0] && (VOS_StrCmp(loid_data_entry[i].onu_id, CTC_AUTH_LOID_DATA_NOT_USED) != 0) )
			{
				n++;
				vty_out( vty, " %-4d%-26s%-12s", n, loid_data_entry[i].onu_id, loid_data_entry[i].password );
				if( PON_NOT_USED_LLID == loid_llid_data_entry[i] )
				{
					vty_out( vty, "  -\r\n" );
				}
				else
				{
					vty_out( vty, " %d\r\n", onuIdToOnuIndex( ponId, loid_llid_data_entry[i] ) );
				}
			}
		}
	}
	else
		vty_out( vty, "loid entries is NULL\r\n" );
	
	return CMD_SUCCESS;
}*/
int onu_auth_loidlist_showrun(struct vty * vty, ULONG brdIdx, ULONG portIdx, int flag)
{
    int ret = 0;
	ctc_onu_auth_t *pItem;
    pItem = pg_onuAuthList[brdIdx][portIdx];
    if(!flag && pItem)
    {
        ret = 1;
        vty_out( vty, " pon %d/%d\r\n",brdIdx, portIdx) ;
    }
    while( pItem )
    {
        if( (pItem->authRowStatus != 0) && (pItem->authRowStatus < 6) )
        {
            if(pItem->loidData.password)
                vty_out( vty, "  add onu-register authentication loid %s %s\r\n", pItem->loidData.onu_id, pItem->loidData.password );
            else
                vty_out( vty, "  add onu-register authentication loid %s\r\n", pItem->loidData.onu_id);
        }
        pItem = pItem->next;
    }
    return ret;
}
LONG onu_auth_cli_cmd_install()
{
	install_element ( PON_PORT_NODE, &show_pon_ctc_auth_mode_cmd);	
	/*install_element ( CONFIG_NODE, &show_ctc_auth_mode_cmd);*/
	install_element ( PON_PORT_NODE, &config_pon_ctc_auth_mode_cmd);
	
	install_element ( PON_PORT_NODE, &add_pon_ctc_auth_loid_data_cmd);
	install_element ( PON_PORT_NODE, &del_pon_ctc_auth_loid_data_cmd);
	install_element ( PON_PORT_NODE, &show_ctc_auth_loid_data_cmd);
    
	install_element ( CONFIG_NODE, &show_olt_ctc_auth_mode_cmd);   /*add by luh 2012-7-26*/         
    install_element ( CONFIG_NODE, &config_olt_ctc_epon_auth_mode_cmd );/*modi by luh 2012-2-8*/
	install_element ( CONFIG_NODE, &config_olt_ctc_gpon_auth_mode_cmd );
	install_element ( CONFIG_NODE, &add_olt_ctc_auth_loid_data_cmd);
	install_element ( CONFIG_NODE, &del_olt_ctc_auth_loid_data_cmd);
	install_element ( CONFIG_NODE, &show_olt_auth_loid_data_cmd);

	install_element ( VIEW_NODE, &show_olt_ctc_auth_mode_cmd);   /*add by luh 2012-7-26*/         
    install_element ( VIEW_NODE, &config_olt_ctc_epon_auth_mode_cmd );/*modi by luh 2012-2-8*/
	install_element ( VIEW_NODE, &config_olt_ctc_gpon_auth_mode_cmd );
	install_element ( VIEW_NODE, &add_olt_ctc_auth_loid_data_cmd);
	install_element ( VIEW_NODE, &del_olt_ctc_auth_loid_data_cmd);
	install_element ( VIEW_NODE, &show_olt_auth_loid_data_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &onu_auth_debug_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_onu_auth_debug_cmd);
	
	install_element ( DEBUG_HIDDEN_NODE, &config_pon_ctc_auth_config_cmd);
    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
    	install_element ( LIC_NODE, &config_pon_ctc_auth_config_cmd);
	return VOS_OK;
}

/*pon保护倒换时，应该调用该API 清掉目标pon上的相应配置
 * 数据，这里是对认证loid表的清空,期望由主控调用，先修
 * 主控板本地数据，再向pon发消息，清pon本地数据。最后，
 * 清PAS_SOFT 数据。added by luh 2012-2-17*/
int	DeletePonPortLoidEntryAll(short int PonPortIdx)
{
	ctc_onu_auth_t *pItem;
	ctc_onu_auth_t *pPreItem;
	/*UCHAR loid[CTC_AUTH_ONU_ID_SIZE+1];*/
	/*UCHAR password[CTC_AUTH_PASSWORD_SIZE+1];*/
    /*int loop = 0;*/
	/*authentication_olt_database_t *pLoidDatabase;*/
	/*CTC_STACK_auth_loid_data_t loid_data;*/
    
    int brdIdx = GetCardIdxByPonChip(PonPortIdx);
    int portIdx = GetPonPortByPonChip(PonPortIdx);
	if( brdIdx >= PRODUCT_MAX_TOTAL_SLOTNUM || portIdx > PONPORTPERCARD )
		return VOS_ERROR;
	
	pItem = pg_onuAuthList[brdIdx][portIdx];
	pPreItem = NULL;
	while( pItem )
	{
		if(pItem->authIdx || pItem->loidData.onu_id)
		{
    		pPreItem = pItem;
    		pItem = pItem->next;
            VOS_Free(pPreItem);
		}
	}
    pg_onuAuthList[brdIdx][portIdx] = NULL;
    
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && (!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER))
    {
		ctc_onu_auth_msg_t sndMsg;
        
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_auth_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_AUTH_LOID_DEL_ALL;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
        if (PonPortIsWorking(PonPortIdx) == TRUE)
		    onu_auth_sync_msg_send ( brdIdx, CTC_MSG_CODE_ONU_AUTH_LOID, &sndMsg );
    }
	return VOS_OK;
}
#if 0
LONG CTC_onu_auth_show_run( struct vty * vty )
{
	ULONG brdIdx, portIdx, loidIdx;
	/*ULONG nextBrdIdx, nextPortIdx, nextLoidIdx;*/
	ULONG mode;
	/*int count;*/
	int ponId;

	authentication_olt_database_t *pLoidDatabase;

	UCHAR loid[CTC_AUTH_ONU_ID_SIZE+1];
	UCHAR password[CTC_AUTH_PASSWORD_SIZE+1];

	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue;    	
			
			if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
			{
				if( mode != mn_ctc_auth_mode_disable )
				{
					vty_out( vty, "pon %d/%d\r\n", brdIdx, portIdx );
					vty_out( vty, "ctc auth-mode %s\r\n", onu_auth_mode_to_str(mode) );

					pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
					if( pLoidDatabase == NULL )
					{
						vty_out( vty, "Loid database error\r\n" );
						return CMD_WARNING;
					}

					for( loidIdx=0; loidIdx<PON_ONU_ID_PER_OLT_ARRAY_SIZE; loidIdx++ )
					{
						if( pLoidDatabase->loid[loidIdx].onu_id[0] && (VOS_StrCmp(pLoidDatabase->loid[loidIdx].onu_id, CTC_AUTH_LOID_DATA_NOT_USED) != 0) )
						{
							onu_auth_loid_cpy( loid, pLoidDatabase->loid[loidIdx].onu_id, CTC_AUTH_ONU_ID_SIZE );
							onu_auth_loid_cpy( password, pLoidDatabase->loid[loidIdx].password, CTC_AUTH_PASSWORD_SIZE );
							vty_out( vty, "ctc auth-loid add %s %s\r\n", loid, password );
						}
					}

					vty_out( vty, "exit\r\n" );
				}
			}
		}
	}
	return VOS_OK;
}
#endif

/*extern ULONG g_ctcSaveDebFlag;*/
typedef struct{
	USHORT  reserve;
	USHORT	auth_pon_num;
}__attribute__((packed))cfg_onu_auth_mode_entry_head;
typedef struct{
	UCHAR  brdIdx;
	UCHAR  ponIdx;
	UCHAR  auth_mode;
}__attribute__((packed))cfg_onu_auth_mode_entry;

typedef struct{
	USHORT  reserve;
	USHORT	auth_loid_num;
}__attribute__((packed))cfg_onu_auth_loid_entry_head;
typedef struct{
	UCHAR brdIdx;
	UCHAR ponIdx;
	UCHAR loidIdx;
	UCHAR auth_loid[CTC_AUTH_ONU_ID_SIZE+1];
	UCHAR auth_password[CTC_AUTH_PASSWORD_SIZE+1];
}__attribute__((packed))cfg_onu_auth_loid_entry;

ULONG saveCtcAuthModeCfgData( const ULONG addr )
{
#if 0
	ULONG brdIdx, portIdx;
	ULONG mode;
	int ponId;

	ULONG length = 0;

	cfg_onu_auth_mode_entry_head *pEntryHead = (cfg_onu_auth_mode_entry_head *)addr;
	cfg_onu_auth_mode_entry *pEntry = (cfg_onu_auth_mode_entry *)(pEntryHead + 1);

	pEntryHead->reserve = 0;
	pEntryHead->auth_pon_num = 0;
	length += sizeof(cfg_onu_auth_mode_entry_head);
	
	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue;
			
			if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) != VOS_OK )
			{
				mode = mn_ctc_auth_mode_disable;
			}
			pEntry->brdIdx = brdIdx;
			pEntry->ponIdx = portIdx;
			pEntry->auth_mode = mode;

			pEntryHead->auth_pon_num++;
/*sys_console_printf( "saveCtcAuthModeCfgData:pon%d/%d,auth_mode=%s\r\n", pEntry->brdIdx, pEntry->ponIdx, onu_auth_mode_to_str(mode) );*/

			pEntry++;
			length += sizeof(cfg_onu_auth_mode_entry);
		}
	}


	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcAuthModeCfgData:auth_pon_num=%d,len=%d\r\n", pEntryHead->auth_pon_num, length );
	}

	return length;
#else
	return 0;
#endif
}

ULONG retrieveCtcAuthModeCfgData( const ULONG addr )
{
#if 0
	ULONG length = 0;
	int ponId;
	int i;

	cfg_onu_auth_mode_entry_head *pEntryHead = (cfg_onu_auth_mode_entry_head *)addr;
	cfg_onu_auth_mode_entry *pEntry = (cfg_onu_auth_mode_entry *)(pEntryHead + 1);

	/*if( g_ctcRetrieveVer < CTC_CFG_DATA_VER2 )
		return length;*/
	
	length += sizeof(cfg_onu_auth_mode_entry_head);

	if( (pEntryHead->auth_pon_num == 0) || (pEntryHead->auth_pon_num > MAXPON) )
	{
	}
	else
	{
		for( i=0; i<pEntryHead->auth_pon_num; i++ )
		{
			if( pEntry == NULL )
			{
				VOS_ASSERT(0);
				break;
			}

			if( SlotCardIsPonBoard(pEntry->brdIdx) == VOS_OK )
			{
				ponId = GetPonPortIdxBySlot( (short int)pEntry->brdIdx, (short  int)pEntry->ponIdx );
				if( (ponId >= 0) && (ponId < MAXPON) )
				{
					mn_setCtcOnuAuthMode(pEntry->brdIdx, pEntry->ponIdx, pEntry->auth_mode );

					/*sys_console_printf( "retrieveCtcAuthModeCfgData:pon%d/%d mode=%s\r\n", pEntry->brdIdx, pEntry->ponIdx, onu_auth_mode_to_str(pEntry->auth_mode) );*/
				}
			}
			pEntry++;
			length += sizeof(cfg_onu_auth_mode_entry);
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcAuthModeCfgData:auth_pon_num=%d,len=%d\r\n", pEntryHead->auth_pon_num, length );
	}
	
	return length;
#else
	return 0;
#endif
}

ULONG saveCtcAuthLoidCfgData( const ULONG addr )
{
#if 0
	ULONG brdIdx, portIdx, loid_id;
	ULONG mode;
	int ponId;

	authentication_olt_database_t *pLoidDatabase;
	ULONG length = 0;

	cfg_onu_auth_loid_entry_head *pEntryHead = (cfg_onu_auth_loid_entry_head *)addr;
	cfg_onu_auth_loid_entry *pEntry = (cfg_onu_auth_loid_entry *)(pEntryHead + 1);

	pEntryHead->reserve = 0;
	pEntryHead->auth_loid_num = 0;
	length += sizeof(cfg_onu_auth_loid_entry_head);
	
	for( brdIdx=1; brdIdx<SYS_CHASSIS_SLOTNUM; brdIdx++ )
	{
		if( SlotCardIsPonBoard(brdIdx) == VOS_ERROR )
			continue;

		for( portIdx=1; portIdx<=MAX_PONPORT_PER_BOARD; portIdx++ )
		{
			ponId = GetPonPortIdxBySlot( (short int)brdIdx, (short  int)portIdx );
			if( (ponId < 0) || (ponId >= MAXPON) )
				continue;    	
			
			if( mn_getCtcOnuAuthMode(brdIdx, portIdx, &mode) == VOS_OK )
			{
				if( mode != mn_ctc_auth_mode_disable )
				{
					pLoidDatabase = CTC_STACK_get_auth_loid_database( ponId );
					if( pLoidDatabase == NULL )
					{
						sys_console_printf( " pon%d/%d Loid database error\r\n", brdIdx, portIdx );
						continue;
					}

					for( loid_id=0; loid_id<PON_ONU_ID_PER_OLT_ARRAY_SIZE; loid_id++ )
					{
						if( check_loid_data_is_used(pLoidDatabase->loid[loid_id].onu_id) )
						{
							pEntry->brdIdx = brdIdx;
							pEntry->ponIdx = portIdx;
							pEntry->loidIdx = loid_id+1;
							onu_auth_loid_cpy( pEntry->auth_loid, pLoidDatabase->loid[loid_id].onu_id, CTC_AUTH_ONU_ID_SIZE );
							onu_auth_loid_cpy( pEntry->auth_password, pLoidDatabase->loid[loid_id].password, CTC_AUTH_PASSWORD_SIZE );
		/*sys_console_printf( "saveCtcAuthLoidCfgData:pon%d/%d,loidIdx=%d\r\n", pEntry->brdIdx, pEntry->ponIdx, pEntry->loidIdx );*/

							pEntryHead->auth_loid_num++;
							pEntry++;
							length += sizeof(cfg_onu_auth_loid_entry);
						}
					}
				}
			}
		}
	}


	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcAuthLoidCfgData:auth_pon_num=%d,len=%d\r\n", pEntryHead->auth_loid_num, length );
	}

	return length;
#else
	return 0;
#endif
}

ULONG retrieveCtcAuthLoidCfgData( const ULONG addr )
{
#if 0
	ULONG length = 0;
	int i;

	cfg_onu_auth_loid_entry_head *pEntryHead = (cfg_onu_auth_loid_entry_head *)addr;
	cfg_onu_auth_loid_entry *pEntry = (cfg_onu_auth_loid_entry *)(pEntryHead + 1);

	/*if( g_ctcRetrieveVer < CTC_CFG_DATA_VER2 )
		return length;*/
	
	length += sizeof(cfg_onu_auth_loid_entry_head);

	if( (pEntryHead->auth_loid_num == 0) || (pEntryHead->auth_loid_num > 1024) )
	{
	}
	else
	{
		for( i=0; i<pEntryHead->auth_loid_num; i++ )
		{
			if( pEntry == NULL )
			{
				VOS_ASSERT(0);
				break;
			}
/*sys_console_printf( "retrieveCtcAuthLoidCfgData:pon%d/%d,loidIdx=%d", pEntry->brdIdx, pEntry->ponIdx, pEntry->loidIdx );*/
			CTC_addCtcOnuAuthLoid_by_loidIdx( pEntry->brdIdx, pEntry->ponIdx, pEntry->loidIdx, pEntry->auth_loid, pEntry->auth_password );

			pEntry++;
			length += sizeof(cfg_onu_auth_loid_entry);
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcAuthLoidCfgData:auth_loid_num=%d,len=%d\r\n", pEntryHead->auth_loid_num, length );
	}
	
	return length;
#else
	return 0;
#endif
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */

