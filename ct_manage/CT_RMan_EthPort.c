#include  "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#if 0
#include  "gwEponSys.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "CT_RMan_EthPort.h"
#include "CT_RMan_Main.h"

#include "gwEponSys.h"
#include "../cli/Olt_cli.h"

/*#ifdef __CT_EXTOAM_SUPPORT*/
#if 0

/*----------------------------------------------------------------------------*/

int CT_RMan_EthLinkState_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pEthLinkPause )
{
	pdu_head_t pdu;

	short int ponid;

	char* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;
	
	char *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( &pdu, 0, sizeof(pdu) );
	pdu.opcode = EXT_VAR_REQ_CODE;

	pdu.iit.branch = INSTANCE_INDEX_VAL;
	pdu.iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu.iit.width = 0x01;
	pdu.iit.value = ethPort;

	pdu.vd.branch = CT_EXT_ATTRIBUTE;
	pdu.vd.leaf = CT_EXT_ATTR_LEAF_ETHLINSTATE;

	VOS_MemCpy( ppdu+4, &pdu, sizeof(pdu) );

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, sizeof(pdu), pRecvData, &recvlen ) == 0 )
	{
		const int pdulen = sizeof( pdu_head_t );
		pdu_head_t *p = (pdu_head_t*)pRecvData;

		if( p->opcode == EXT_VAR_RES_CODE &&
			p->vd.branch == 0xc7 &&
			p->vd.leaf == 0x11 &&
			p->iit.value == ethPort )
			{
				*pEthLinkPause = *(pRecvData+pdulen+1);
				ret = VOS_OK;
			}
	}

	CT_ExtOam_Free( ppdu );

	return ret;
	
}

int CT_RMan_EthLinkState_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort,
			UCHAR call_flag, ULONG call_notify, UCHAR ethLinkState )
{
	int ret = VOS_ERROR;
	pdu_head_t *pdu = NULL;
	pdu_ack_t	*pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen  = 0;
	USHORT	ponid = 0;

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return ret;

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_ETHLINSTATE;

	headlen = sizeof(pdu_head_t);

   *(((char*)pdu)+headlen) = 1;
   *(((char*)pdu)+headlen+1) = ethLinkState;

	ponid = GetPonPortIdxBySlot( slotno, pon );
   if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData,
   	                    &recvlen ) == VOS_OK && pRecvPduData->ack == SET_REQUEST_RET_OK )
   	                    ret = VOS_OK;

   CT_ExtOam_Free( pdu );
   return ret;
	
}

int CT_RMan_EthPortPolicing_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_EthPortPolicing_t* pEthPortPolicing )
{
	pdu_head_t *pdu = NULL;

	char *pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen = 0;
	USHORT ponid = 0;
	int ret = VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	if( (pdu=CT_ExtOam_Alloc()) == NULL )
		return ret;

	headlen = sizeof(pdu_head_t);
	VOS_MemSet( (char*)pdu, 0, headlen );


	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_ETHPORTPOLICY;

	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		char *p = ((char*)pdu)+headlen;
		int width = *p;
		pEthPortPolicing->portpolicyope = *(p+1);
		if( width == 10 )
		{
			ULONG ut = 0;
			ut = *(ULONG*)(p+2);
			pEthPortPolicing->ppCIR = ut>>8;
			ut = *(ULONG*)(p+5);
			pEthPortPolicing->ppCBS = ut>>8;
			ut = *(ULONG*)(p+8);
			pEthPortPolicing->ppEBS = ut>>8;
		}
		ret = VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return ret;
}

int CT_RMan_EthPortPolicing_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_EthPortPolicing_t* pEthPortPolicing )
{
	pdu_head_t *pdu = NULL;

	pdu_ack_t *pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen = 0;
	USHORT ponid = 0;
	int ret = VOS_ERROR;
	int width = 0;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	if( (pdu=CT_ExtOam_Alloc()) == NULL )
		return ret;

	headlen = sizeof(pdu_head_t);
	VOS_MemSet( (char*)pdu, 0, headlen );


	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_ETHPORTPOLICY;

	if( pEthPortPolicing->portpolicyope == 0 )
	{
		char *p = ((char*)pdu)+headlen;
		*p = 1;
		width = 1;
		*(p+1)=0;
	}
	else
	{
		char *p = ((char*)pdu)+headlen;
		*p = 10;
		width = 10;
		*(p+1)=pEthPortPolicing->portpolicyope;
		VOS_MemCpy( p+2, ((char*)(pEthPortPolicing->ppCIR&0xffffff))+1, 3  );
		VOS_MemCpy( p+5, ((char*)(pEthPortPolicing->ppCBS&0xffffff))+1, 3  );
		VOS_MemCpy( p+8, ((char*)(pEthPortPolicing->ppEBS&0xffffff))+1, 3  );
	}

	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+width+1, &pRecvPduData, &recvlen ) == VOS_OK &&
		pRecvPduData->ack == SET_REQUEST_RET_OK )
	{
		ret = VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return ret;
}


int CT_RMan_PhyAdminState_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pAdminState )
{

	short int ponid;

	char* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;
	
	pdu_head_t *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( ppdu, 0, sizeof(pdu_head_t) );
	ppdu->opcode = EXT_VAR_REQ_CODE;

	ppdu->iit.branch = INSTANCE_INDEX_VAL;
	ppdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	ppdu->iit.width = 0x01;
	ppdu->iit.value = ethPort;

	ppdu->vd.branch = STANDARD_ATTRIBUTE;
	ppdu->vd.leaf = STANDARD_ATTR_LEAF_PHYADMINSTATUS;

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, ppdu, sizeof(pdu_head_t), &pRecvData, &recvlen ) == 0 )
	{
		const int pdulen = sizeof( pdu_head_t );
		int width = *(pRecvData+pdulen);
		if( width == 4 )
		{
			ULONG val = *(ULONG*)(pRecvData+pdulen+1);
			*pAdminState = val&0xff;
			ret = VOS_OK;
		}
	}

	CT_ExtOam_Free( ppdu );

	return ret;
	
}
#if 0
int CT_RMan_EthLinkState_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort,
			UCHAR call_flag, ULONG call_notify, UCHAR ethLinkState )
{
	int ret = VOS_ERROR;
	pdu_head_t *pdu = NULL;
	pdu_ack_t	*pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen  = 0;
	USHORT	ponid = 0;

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return ret;

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_ETHLINSTATE;

	headlen = sizeof(pdu_head_t);

   *(((char*)pdu)+headlen) = 4;
   *(ULONG*)(((char*)pdu)+headlen+1) = (ULONG)ethLinkState;

	ponid = GetPonPortIdxBySlot( slotno, pon );
   if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+5, &pRecvPduData,
   	                    &recvlen ) == VOS_OK && pRecvPduData->ack == SET_REQUEST_RET_OK )
   	                    ret = VOS_OK;

   CT_ExtOam_Free( pdu );
   return ret;
}

#endif

int CT_RMan_PhyAdminControl_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort,
			UCHAR call_flag, ULONG call_notify, UCHAR adminCtrl )
{
	short int ponid;

	pdu_ack_t* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;

	int headlen = sizeof(pdu_head_t);
	
	pdu_head_t *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( ppdu, 0, sizeof(pdu_head_t) );
	ppdu->opcode = EXT_VAR_REQ_CODE;

	ppdu->iit.branch = INSTANCE_INDEX_VAL;
	ppdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	ppdu->iit.width = 0x01;
	ppdu->iit.value = ethPort;

	ppdu->vd.branch = STANDARD_OPERATION;
	ppdu->vd.leaf = STANDARD_ATTR_LEAF_PHYADMINCONTROL;

	*(((char*)ppdu)+headlen) = 4;
	*(ULONG*)(((char*)ppdu)+headlen+1) = adminCtrl;

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, ppdu, headlen+5, &pRecvData, &recvlen ) ==  VOS_OK &&
		pRecvData->ack == SET_REQUEST_RET_OK )
	{
		ret =VOS_OK;
	}

	CT_ExtOam_Free( ppdu );

	return ret;
}

int CT_RMan_AutoNegAdminState_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pAutoNegState )
{
	short int ponid;

	char* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;
	
	pdu_head_t *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( ppdu, 0, sizeof(pdu_head_t) );
	ppdu->opcode = EXT_VAR_REQ_CODE;

	ppdu->iit.branch = INSTANCE_INDEX_VAL;
	ppdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	ppdu->iit.width = 0x01;
	ppdu->iit.value = ethPort;

	ppdu->vd.branch = STANDARD_ATTRIBUTE;
	ppdu->vd.leaf = STANDARD_ATTR_LEAF_AUTONEGADMINSTATE;

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, ppdu, sizeof(pdu_head_t), &pRecvData, &recvlen ) == 0 )
	{
		const int pdulen = sizeof( pdu_head_t );
		int width = *(pRecvData+pdulen);
		if( width == 4 )
		{
			ULONG val = *(ULONG*)(pRecvData+pdulen+1);
			*pAutoNegState = val&0xff;
			ret = VOS_OK;
		}
	}

	CT_ExtOam_Free( ppdu );

	return ret;

}

int CT_RMan_AutoNegLocalTechnologyAbility_get( UCHAR slotno, UCHAR pon, UCHAR onu, 
			UCHAR ethPort, UCHAR call_flag, ULONG call_notify, CT_RMan_EthAutoNegAbility_t* pAbility )
{
	pdu_head_t *pdu = NULL;
	char*	pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen = 0,width=0, ret=VOS_ERROR;
	USHORT ponid = 0;

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return ret;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = STANDARD_ATTRIBUTE;
	pdu->vd.leaf = STANDARD_ATTR_LEAF_AUTOLOCALTECHABILITY;

	headlen = sizeof(pdu_head_t);

	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
		{
			char* p = pRecvPduData+headlen;
			width = *p;
			VOS_MemCpy( pAbility, p+1, width );
			ret = VOS_OK;
		}
	CT_ExtOam_Free( pdu );
	return ret;
}

int CT_RMan_AutoNegAdvertisedTechnologyAbility_get( UCHAR slotno, UCHAR pon, UCHAR onu, 
			UCHAR ethPort, UCHAR call_flag, ULONG call_notify, UCHAR* pAbility )
{
	pdu_head_t *pdu = NULL;
	char*	pRecvPduData = NULL;
	ULONG	recvlen = 0;

	int headlen = 0,width=0, ret=VOS_ERROR;
	USHORT ponid = 0;

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return ret;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 1;
	pdu->iit.value = ethPort;

	pdu->vd.branch = STANDARD_ATTRIBUTE;
	pdu->vd.leaf = STANDARD_ATTR_LEAF_AUTONEGADVERTISETECHABILITY;

	headlen = sizeof(pdu_head_t);

	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
		{
			char* p = pRecvPduData+headlen;
			int width = *p;
			VOS_MemCpy( pAbility, p+1, width );
			ret = VOS_OK;
		}
	CT_ExtOam_Free( pdu );
	return ret;
}

int CT_RMan_AutoNegRestartAutoConfig_set( UCHAR slotno, UCHAR pon, UCHAR onu, 
			UCHAR ethPort, UCHAR call_flag, ULONG call_notify, UCHAR restart )
{
	short int ponid;

	pdu_ack_t* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;

	int headlen = sizeof(pdu_head_t);
	
	pdu_head_t *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( ppdu, 0, sizeof(pdu_head_t) );
	ppdu->opcode = EXT_VAR_REQ_CODE;

	ppdu->iit.branch = INSTANCE_INDEX_VAL;
	ppdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	ppdu->iit.width = 0x01;
	ppdu->iit.value = ethPort;

	ppdu->vd.branch = STANDARD_OPERATION;
	ppdu->vd.leaf = STANDARD_ATTR_LEAF_AUTONEGRESTARTAUTOCONFIG;

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, ppdu, headlen, &pRecvData, &recvlen ) ==  VOS_OK &&
		pRecvData->ack == SET_REQUEST_RET_OK )
	{
		ret =VOS_OK;
	}

	CT_ExtOam_Free( ppdu );

	return ret;

}

int CT_RMan_AutoNegAdminControl_set( UCHAR slotno, UCHAR pon, UCHAR onu, 
			UCHAR ethPort, UCHAR call_flag, ULONG call_notify, UCHAR ctrl )
{
	short int ponid;

	pdu_ack_t* pRecvData = NULL;

	int ret = VOS_ERROR;

	ULONG recvlen = 0;

	int headlen = sizeof(pdu_head_t);
	
	pdu_head_t *ppdu = CT_ExtOam_Alloc();

	if( ppdu == NULL )
	{
	    sys_console_printf("Alloc fail!\r\n");
		return ret;
	}

    VOS_MemSet( ppdu, 0, sizeof(pdu_head_t) );
	ppdu->opcode = EXT_VAR_REQ_CODE;

	ppdu->iit.branch = INSTANCE_INDEX_VAL;
	ppdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	ppdu->iit.width = 0x01;
	ppdu->iit.value = ethPort;

	ppdu->vd.branch = STANDARD_OPERATION;
	ppdu->vd.leaf = STANDARD_ATTR_LEAF_AUTONEGADMINCONTROL;

	*(((char*)ppdu)+headlen) = 4;
	*(ULONG*)(((char*)ppdu)+headlen+1) = ctrl;

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if(CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, ppdu, headlen+5, &pRecvData, &recvlen ) ==  VOS_OK &&
		pRecvData->ack == SET_REQUEST_RET_OK )
	{
		ret =VOS_OK;
	}

	CT_ExtOam_Free( ppdu );

	return ret;

}


/*----------------------------------------------------------------------------*/

#endif /*__CT_EXTOAM_SUPPORT*/

/*----------------------------------------------------------------------------*/

/* CLI */

#if 0
/* modified by chenfj 2007-7-24 */
DEFUN(show_ctc_ethernet_link_state,
	  show_ctc_ethernet_link_state_cmd,
      	  "show ctc ethernet-link-state port [all|<1-64>]",
	  SHOW_STR
	  CTC_STR
	  "display ethernet port link state\n"
	  "onu ethport\n"
	  "all ether ports\n"
	  "port number\n")
{

	int result;
	short int PonPortIdx;
	ULONG ulSlot, ulPort, OnuIdx;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char EthPort = 0;
	short int llid;
	bool  Status;
	short int ret;
	

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &OnuIdx );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if ((ulSlot<4)  || (ulSlot>8))
		{
		vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
		return CMD_WARNING;
		}
	
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if ( CARDINSERT  != GetOltCardslotInserted( ulSlot-1 ))
			{
			vty_out( vty, "  %% Can not find slot %d. \r\n",ulSlot);
			return CMD_WARNING;
			}

		if (ulSlot == 4)
			{
			if(MODULE_E_GFA_SW == SYS_MODULE_TYPE(ulSlot))
				{
				vty_out( vty, "  %% slot %d is SW board.\r\n", ulSlot );
				return CMD_WARNING;
				}
			}
		}
	
	if ( (ulPort < 1) || (ulPort > 4) )
		{
		vty_out( vty, "  %% no exist port %d/%d. \r\n",ulSlot, ulPort);
		return CMD_WARNING;
		}

	PonPortIdx = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (PonPortIdx == VOS_ERROR)
		{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
		}	
	
	result = GetPonPortOperStatus(  PonPortIdx );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((OnuIdx< 1) || (OnuIdx> 64))
		{
		vty_out( vty, "  %% onuidx error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
		}

	if( GetOnuOperStatus(PonPortIdx, (OnuIdx-1)) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, OnuIdx );
		return( CMD_WARNING );
		}

	llid = GetLlidByOnuIdx(PonPortIdx, (OnuIdx-1));
	if(llid == INVALID_LLID) return( CMD_WARNING );
	
	if ( !( VOS_StrCmp( argv[ 0 ] , "all" ) ) )
		EthPort = 0;
	else  EthPort = ( unsigned char )VOS_AtoL( argv[ 0 ] );
		
	if( EthPort != 0 ) /*  指定UNI 端口*/
		{
		ret = CTC_STACK_get_phy_admin_state( PonPortIdx, llid, EthPort, &Status );
		if( ret == PAS_EXIT_OK ) 
			{
			vty_out(vty, "  onu %d/%d/%d ether-port %d state is ", ulSlot, ulPort, OnuIdx, EthPort );
			if( Status == TRUE ) vty_out(vty,"UP\r\n");
			else vty_out(vty,"Down\r\n");
			return( CMD_SUCCESS);
			}
		else {
			vty_out( vty, "%% Executing error\r\n" );
			return( CMD_WARNING );
			}
		}
	
	else{  /*  所有UNI 端口*/

		/* 当对TK ONU 调用CTC_STACK_get_ethernet_all_port_link_state() 时，出错*/
		/* */

		int retVal;
		int value;
		short int OnuEntry;
		unsigned char EthPortNum, EthPortIdx, PortCount=0;

		retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, (OnuIdx-1),  &value );
		if( retVal != ROK ) return( CMD_WARNING );

		if( value > CTC_ONU_MAX_ETHPORT ) 
			{
			vty_out(vty, "  onu %d/%d/%d capability err,eht port num=%d;try again\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx),value);
			CTC_GetOnuCapability( PonPortIdx, (OnuIdx-1) );
			}
		retVal = CTC_getDeviceCapEthPortNum( PonPortIdx, (OnuIdx-1),  &value );
		if( retVal != ROK ) return( CMD_WARNING );
		if( value > CTC_ONU_MAX_ETHPORT ) 
			{
			vty_out(vty, "  onu %d/%d/%d capability err,eht port num=%d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx),value);
			return( CMD_WARNING );
			}

		OnuEntry = PonPortIdx * MAXONUPERPON + (OnuIdx-1) ;
		EthPortNum = OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number;

		if( EthPortNum != 0 )
			{
			PortCount = 0;
			vty_out(vty, "  onu %d/%d/%d FE-port state\r\n", ulSlot, ulPort, OnuIdx );
			
			for( EthPortIdx = 0; EthPortIdx < (CTC_ONU_MAX_ETHPORT); EthPortIdx ++ )
				{
				if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == FE_INTERFACE )
					{
					PortCount++;
					ret = CTC_STACK_get_phy_admin_state( PonPortIdx, llid, (EthPortIdx+1), &Status );
					if( ret == PAS_EXIT_OK )
						{
						vty_out(vty, "  port%d ---", (EthPortIdx+1));
						if( Status == TRUE ) vty_out(vty, "UP\r\n");
						else vty_out(vty, "Down\r\n");
						}
					
					if( PortCount == EthPortNum ) break;
					}
				}
			}
		
		EthPortNum = OnuMgmtTable[OnuEntry].GE_Ethernet_ports_number;
		if( EthPortNum != 0 )
			{
			PortCount = 0;
			vty_out(vty, "  onu %d/%d/%d GE-port state\r\n", ulSlot, ulPort, OnuIdx );
			
			for( EthPortIdx = 0; EthPortIdx < (CTC_ONU_MAX_ETHPORT); EthPortIdx ++ )
				{
				if(((OnuMgmtTable[OnuEntry].Ports_distribution[EthPortIdx/4] >> ( ( EthPortIdx%4 )*2 ) ) & 3 ) == GE_INTERFACE)
					{
					PortCount++;
					ret = CTC_STACK_get_phy_admin_state( PonPortIdx, llid, (EthPortIdx+1), &Status );
					if( ret == PAS_EXIT_OK )
						{
						vty_out(vty, "  port%d ---", (EthPortIdx+1));
						if( Status == TRUE ) vty_out(vty, "UP\r\n");
						else vty_out(vty, "Down\r\n");
						}
					
					if( PortCount == EthPortNum ) break;
					}
				}
			}


		}
	
#if 0
	if( parse_onu_command_parameter( vty, &olt_id, &onu_id) == VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	if( VOS_MemCmp( argv[0], "all",3 ) == 0 )
	{
		/* return_result = CTC_STACK_get_ethernet_all_port_link_state(olt_id, onu_id, &number_of_entries, port_link_state); */
	}
	else 
	{
		/* return_result = CTC_STACK_get_ethernet_link_state(olt_id, onu_id, port_number, &link_state);*/
	}
#endif

	return CMD_SUCCESS;
}
#endif

/*
DEFUN(show_ctc_ethernet_pause,
	  show_ctc_ethernet_pause_cmd,
      "show ctc ethernet-pause {[port] [all | <1-24>]}*1	",
	  SHOW_STR
	  CTC_STR
	  "Display ethernet port pause state\n"
	  "set port\n"
	  "All port\n"
	  "port number\n")
{
	CTC_ports_pause_t			port_pause; 
	return CMD_SUCCESS;
}*/


/*----------------------------------------------------------------------------*/
/* added by xieshl 20070321 */
/*----------------------------------------------------------------------------*/
#if 0
CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];		/* added by xieshl 20070321 */
#else
CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPON][MAX_ONU_ETHPORT];
#endif
ULONG onuEthPortSemId = 0;
/*----------------------------------------------------------------------------*/
/* end 20070321 */
/*----------------------------------------------------------------------------*/
extern LONG CT_EthCli_Init();
extern LONG CT_VlanCli_Init();

LONG CT_RMan_EthPort_Init( void )
{
	int i, j, k;
	CT_Onu_EthPortItem_t *pItem;
    ULONG ulSize;

#if 1
#ifdef g_malloc
#undef g_malloc
#endif

    ulSize = sizeof(CT_Onu_EthPortItem_t) * MAXPON * MAXONUPERPON * MAX_ONU_ETHPORT;
    if ( NULL == (onuEthPort = g_malloc(ulSize)) )
    {
        VOS_ASSERT(0);
        
        return VOS_ERROR;
    }
	VOS_MemZero( (VOID *)onuEthPort, ulSize );
#else  
	VOS_MemZero( (VOID *)&onuEthPort[0][0][0], sizeof(onuEthPort) );
#endif

	for( i=0; i<MAXPON; i++ )
	{
		for( j=0; j<MAXONUPERPON; j++ )
		{
			for( k=0; k<MAX_ONU_ETHPORT; k++ )
			{
				pItem = &onuEthPort[i][j][k];
				pItem->ethPortPolicingCIR = 4836;				/*	承诺平均速率，范围64～1000000Kbps*/
				pItem->ethPortPolicingCBS = 64022;				/*	承诺突发尺寸，范围0～10000000byte */
				pItem->ethPortPolicingEBS = 1514;				/*	最大突发尺寸，范围0～10000000byte */
				pItem->ethPortPolicingEnable = 2;				/*	Policing使能*/

				pItem->ethPortVlanTagTpid = 0;					/*	只对ethPortVlanMode!=1有效，默认0x8100 */
				pItem->ethPortVlanTagPri_Cfi_Vid = 1;			/*	只对ethPortVlanMode!=1时有效，默认1*/
				pItem->ethPortVlanMode = 0;					/*	0-transparent、1-tag、2-translation*/
#if 0 /*marked by wangxy 2007-05-22*/
				pItem->ethPortQoSQueueMapped = 0;			/*	本规则所映射的优先级队列*/
				pItem->ethPortQoSPriMark = 0;					/*	本规则的优先级标记*/
				pItem->ethPortQoSRuleEntriesSelect = 13;			/*	{da_mac(1),sa_mac(2),pri(3),vlan(4), thType(5),d_ip(6), s_ip(7), ipType(8), ip4_tos_dscp(9), ip6_precedence(10), s_l4_port(11),d_l4_port(12), other(13) }	规则匹配条件*/
				VOS_MemZero( pItem->ethPortQoSRuleEntriesMatchVal, sizeof(pItem->ethPortQoSRuleEntriesMatchVal));/*匹配值*/
				pItem->ethPortQoSRuleMatchOperator = ethPortQoSRuleMatchOperator_never;
				pItem->ethPortQoSRuleAction = 1;				/* 1-noop, 2-delete, 3-add, 4-delAll, 5-get, 6-processing */


				pItem->ethPortMulticastVlanVid = 1;				/* 临时定义，一个端口只能设置一个组播vlan*/

				pItem->ethPortAutoNegLocalTecAbility = 0;		/*  { bOther ( 0 ) , b10baseT ( 1 ) , b10baseTFD ( 2 ) , b100baseT4 ( 3 ) , b100baseTX ( 4 ) , b100baseTXFD ( 5 ) , b100baseT2 ( 6 ) , b100baseT2FD ( 7 ) , bfdxPause ( 8 ) , bfdxAPause ( 9 ) , bfdxSPause ( 10 ) , bfdxBPause ( 11 ) , b1000baseX ( 12 ) , b1000baseXFD ( 13 ) , b1000baseT ( 14 ) , b1000baseTFD ( 15 ) } */
				pItem->ethPortAutoNegAdvTechAbility = 0;		/*  { bOther ( 0 ) , b10baseT ( 1 ) , b10baseTFD ( 2 ) , b100baseT4 ( 3 ) , b100baseTX ( 4 ) , b100baseTXFD ( 5 ) , b100baseT2 ( 6 ) , b100baseT2FD ( 7 ) , bFdxPause ( 8 ) , bFdxAPause ( 9 ) , bFdxSPause ( 10 ) , bFdxBPause ( 11 ) , b1000baseX ( 12 ) , b1000baseXFD ( 13 ) , b1000baseT ( 14 ) , b1000baseTFD ( 15 ) } */
				pItem->ethPortAutoNegRestart = 1;				/* { noop（1）， restart（2） }	重新协商*/
#endif
				/*BEGIN: 	added by zhengyt  2008-6-27*/
				pItem->ethPortDSPolicingCIR=4836;
				pItem->ethPortDSPolicingPIR=4836;
				pItem->ethPortDSPolicingEnable=2;
				/*END*/
				pItem->ethPortDefaultMauType = 16;		/* added by xieshl 20080704 */
				pItem->ethPortAdminStatus = 1;
				pItem->ethPortAutoNegAdminStatus = 1;
				pItem->ethPortPauseAdminMode = 1;
				pItem->ethPortMultiTagStriped = 1;
			}
		}
	}


	onuEthPortSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);

	return VOS_OK;
}

/*----------------------------------------------------------------------------*/

#define CHECK_ONU_ETHPORT(PON, ONU, PORT)	\
		if( (PON >= MAXPON) || (ONU >= MAXONUPERPON) || (PORT >= MAX_ONU_ETHPORT) )   return VOS_ERROR

/*#define __ETHPORT_VAL_GET( devIdx, portIdx, pVal, NAME ) \
{ \
	PON_olt_id_t olt_id; \
	PON_onu_id_t onu_id; \
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) ) \
		return VOS_ERROR; \
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR ) \
		return VOS_ERROR; \
	portIdx--; \
	*pVal = onuEthPort[olt_id][onu_id][portIdx].NAME; \
	return VOS_OK; \
}*/

int CTC_ethPortDSPolicingEnable_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingEnable;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_ethPortDSPolicingPIR_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal=onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingPIR;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}


int CTC_ethPortDSPolicingCIR_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
#ifndef  PAS_SOFT_VERSION_V5_3_5
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal=onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingCIR;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
#else
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	port_ds_rate_limiting;
	
	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if(CTC_STACK_get_ethernet_port_ds_rate_limiting ( olt_id, llid, portIdx+1, &port_ds_rate_limiting) != CTC_STACK_EXIT_OK)
		    return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingCIR = port_ds_rate_limiting.CIR;
	onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingPIR = port_ds_rate_limiting.PIR;
	
	if( port_ds_rate_limiting.state== CTC_STACK_STATE_ACTIVATE )
		onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingEnable = 1;/* enable */
	else
		onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingEnable = 2;/* disable */

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingCIR;
	VOS_SemGive( onuEthPortSemId );
	
	return VOS_OK;
#endif
}

/*	承诺平均速率，范围64～1000000Kbps*/	
int CTC_ethPortPolicingCIR_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
#if 0	/* xieshl 20070328, 读该对象时刷新所有policing数据 */
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR;
	return VOS_OK;
#else
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_ethernet_port_policing_entry_t  port_policing;
	
	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if( CTC_STACK_get_ethernet_port_policing(olt_id, llid, portIdx+1, &port_policing) != CTC_STACK_EXIT_OK )
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	if( port_policing.cir >= 64 )
		onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR = port_policing.cir;
	else
		onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR = 4836;

	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCBS = port_policing.bucket_depth;
	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEBS = port_policing.extra_burst_size;
	
	if( port_policing.operation == CTC_STACK_STATE_ACTIVATE )
		onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEnable = 1;/* enable */
	else
		onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEnable = 2;/* disable */

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR;
	VOS_SemGive( onuEthPortSemId );
	
	return VOS_OK;
#endif
}

int CTC_ethPortPolicingCIR_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( (val < 64) || (val > 1000000) )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/*BEGIN: added by zhengyt 2008-6-26*/
int CTC_ethPortDSPolicingCIR_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if(   val > 1000000)
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingCIR= val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int  CTC_ethPortDSPolicingPIR_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if(  val > 1000000 )
		return VOS_ERROR;

	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingPIR= val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_ethPortDSPolicingEnable_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
#ifdef  PAS_SOFT_VERSION_V5_3_5
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	port_ds_rate_limiting;
	

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( (val < 1) || (val > 2) )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	if( val == 2 )		/* disable */
	{
		port_ds_rate_limiting.state = CTC_STACK_STATE_DEACTIVATE;
	}
	else if( val == 1 ) /* enable */
	{
		port_ds_rate_limiting.state = CTC_STACK_STATE_ACTIVATE;
	}
	else
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	port_ds_rate_limiting.CIR= onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingCIR;
	port_ds_rate_limiting.PIR= onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingPIR;
	VOS_SemGive( onuEthPortSemId );

	port_ds_rate_limiting.port_number = portIdx+1;

	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
		/*sys_console_printf( "  GetOnuOperStatus error\r\n");*/
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if( CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, portIdx+1, &port_ds_rate_limiting) != CTC_STACK_EXIT_OK )
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortDSPolicingEnable= val;
	VOS_SemGive( onuEthPortSemId );
#endif
	return VOS_OK;
}
/*END*/
/*	承诺突发尺寸，范围0～10000000byte */
int CTC_ethPortPolicingCBS_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCBS;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int CTC_ethPortPolicingCBS_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCBS = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/*	最大突发尺寸，范围0～10000000byte */
int CTC_ethPortPolicingEBS_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEBS;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int CTC_ethPortPolicingEBS_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEBS = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/*	Policing使能0-deactivate, 1-active */
int CTC_ethPortPolicingEnable_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEnable;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int CTC_ethPortPolicingEnable_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_ethernet_port_policing_entry_t  port_policing;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	if( val == 2 )		/* disable */
	{
		port_policing.operation = CTC_STACK_STATE_DEACTIVATE;
	}
	else if( val == 1 ) /* enable */
	{
		port_policing.operation = CTC_STACK_STATE_ACTIVATE;
	}
	else
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	port_policing.cir = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCIR;
	port_policing.bucket_depth = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingCBS;
	port_policing.extra_burst_size = onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEBS;
	VOS_SemGive( onuEthPortSemId );

	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
		/*sys_console_printf( "  GetOnuOperStatus error\r\n");*/
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if( CTC_STACK_set_ethernet_port_policing(olt_id, llid, portIdx+1, port_policing) != CTC_STACK_EXIT_OK )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortPolicingEnable = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}



int qosRuleFieldValue_StrToVal_parase( unsigned char select, unsigned char *pMatchStr, CTC_STACK_value_t *pMatchVal )
{
	int rc_err = VOS_ERROR;
	int i;
	char	*token, *pToken;
	unsigned long val;
	char tmpStr[20];

	if( (pMatchStr == NULL) || (pMatchVal == NULL) )
		return rc_err;

	VOS_MemZero( pMatchVal, sizeof(CTC_STACK_value_t) );
	VOS_StrnCpy( tmpStr, pMatchStr, 17 );
	tmpStr[17] = 0;

	switch( select )
	{
		case CTC_FIELD_SEL_DA_MAC:
		case CTC_FIELD_SEL_SA_MAC:
			/*token = strtok( tmpStr,  " " );
			if( token == NULL )
				return rc_err;
			for( i=0; i<sizeof(mac_address_t); i++ )
			{
				if( token == NULL )
					break;

				pMatchVal->mac_address[i] = VOS_StrToUL( token, &pToken, 16 );
				
				token = strtok( NULL, " " );
			}*/
			Qos_Get_Mac_Address_By_Str( pMatchVal->mac_address, tmpStr );
			break;
			
		case CTC_FIELD_SEL_ETHERNET_PRIORITY:
		case CTC_FIELD_SEL_VLAN_ID:
			pMatchVal->match_value = VOS_AtoL( tmpStr );
			break;
			
		case CTC_FIELD_SEL_ETHER_TYPE:
			pMatchVal->match_value = VOS_StrToUL( tmpStr, &pToken, 16 );
			break;

		case CTC_FIELD_SEL_DEST_IP:
		case CTC_FIELD_SEL_SRC_IP:
			pMatchVal->match_value = get_long_from_ipdotstring( tmpStr );
			/*token = strtok( tmpStr,  "." );
			if( token == NULL )
				return rc_err;
			for( i=0; i<4; i++ )
			{
				if( token == NULL )
					break;
				
				val = VOS_AtoL( token );
				pMatchVal->match_value |= ((val & 0xff) << ((3-i) * 8));
				
				token = strtok( NULL, "." );
			}*/
			break;

		case CTC_FIELD_SEL_IP_PROTOCOL_TYPE:
			pMatchVal->match_value = VOS_StrToUL( tmpStr, &pToken, 16 );
			break;
			
		case CTC_FIELD_SEL_IPV4_TOS_DSCP:
		case CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS:
		case CTC_FIELD_SEL_L4_SRC_PORT:
		case CTC_FIELD_SEL_L4_DEST_PORT:
			pMatchVal->match_value = VOS_AtoL( tmpStr );
			break;
			
		default:
			return rc_err;
	}
	return VOS_OK;
}
int qosRuleFieldValue_ValToStr_parase( unsigned char select, CTC_STACK_value_t *pMatchVal, unsigned char *pMatchStr )
{
	int rc_err = VOS_ERROR;

	if( (pMatchStr == NULL) || (pMatchVal == NULL) )
		return rc_err;

	switch( select )
	{
		case CTC_FIELD_SEL_DA_MAC:
		case CTC_FIELD_SEL_SA_MAC:
			VOS_Sprintf( pMatchStr, "%02x%02x.%02x%02x.%02x%02x",
					pMatchVal->mac_address[0], pMatchVal->mac_address[1],
					pMatchVal->mac_address[2], pMatchVal->mac_address[3],
					pMatchVal->mac_address[4], pMatchVal->mac_address[5] );
			break;

		case CTC_FIELD_SEL_ETHERNET_PRIORITY:
		case CTC_FIELD_SEL_VLAN_ID:
			VOS_Sprintf( pMatchStr, "%d", pMatchVal->match_value );
			break;

		case CTC_FIELD_SEL_ETHER_TYPE:
			VOS_Sprintf( pMatchStr, "%04x", pMatchVal->match_value );
			break;

		case CTC_FIELD_SEL_DEST_IP:
		case CTC_FIELD_SEL_SRC_IP:
			VOS_Sprintf( pMatchStr, "%d.%d.%d.%d",
					((pMatchVal->match_value >> 24) & 0xff),
					((pMatchVal->match_value >> 16) & 0xff),
					((pMatchVal->match_value >> 8) & 0xff), 
					(pMatchVal->match_value & 0xff) );
			break;

		case CTC_FIELD_SEL_IP_PROTOCOL_TYPE:
			VOS_Sprintf( pMatchStr, "%02x", pMatchVal->match_value );
			break;

		case CTC_FIELD_SEL_IPV4_TOS_DSCP:
		case CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS:
		case CTC_FIELD_SEL_L4_SRC_PORT:
		case CTC_FIELD_SEL_L4_DEST_PORT:
			VOS_Sprintf( pMatchStr, "%d", pMatchVal->match_value );
			break;
			
		default:
			return rc_err;
	}
	return VOS_OK;
}


#if 0 /*marked by wangxy 2007-05-22*/
/*	本规则所映射的优先级队列*/
int CTC_ethPortQoSQueueMapped_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSQueueMapped;
	return VOS_OK;
}
int CTC_ethPortQoSQueueMapped_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 7 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSQueueMapped = val;

	return VOS_OK;
}

/*	本规则的优先级标记*/
int CTC_ethPortQoSPriMark_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark;
	return VOS_OK;
}
int CTC_ethPortQoSPriMark_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 7 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark = val;

	return VOS_OK;
}

/*	{da_mac(1),sa_mac(2),pri(3),vlan(4), thType(5),d_ip(6), s_ip(7), ipType(8), ip4_tos_dscp(9), ip6_precedence(10), s_l4_port(11),d_l4_port(12), other(13) }	规则匹配条件*/
int CTC_ethPortQoSRuleEntriesSelect_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	if( onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect == 0 ||
		onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect > 13 )
		*pVal = 13;
	else
		*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect;

	return VOS_OK;
}
int CTC_ethPortQoSRuleEntriesSelect_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 12 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect = val;

	return VOS_OK;
}

/* 匹配值*/
int CTC_ethPortQoSRuleEntriesMatchVal_get( ULONG devIdx, ULONG portIdx,  UCHAR *pVal, ULONG *pValLen)
{
	ULONG strlen;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	strlen = VOS_StrLen(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal);
	if( strlen >= sizeof(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal) )
	{
		strlen = sizeof(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal) - 1;
		onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal[strlen] = 0;
	}
	else if( strlen == 0 )
	{
		onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal[strlen++] = '0';
	}
	
	VOS_StrnCpy( pVal, onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal, strlen );
	*pValLen = strlen;
	
	return VOS_OK;
}
int CTC_ethPortQoSRuleEntriesMatchVal_set( ULONG devIdx, ULONG portIdx,  UCHAR *pVal, ULONG valLen )
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( valLen >= sizeof(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal)  )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_StrnCpy( onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal, pVal, valLen );
	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal[valLen] = 0;

	return VOS_OK;
}

/* 1-noop, 2-delete, 3-add, 4-delAll, 5-get, 6-processing */
int CTC_ethPortQoSRuleAction_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	

	*pVal = 1;/*onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleAction;*/
	return VOS_OK;
}

UCHAR qosRuleFieldMatchOperator( UCHAR mibOperator )
{
	UCHAR operator;
	switch( mibOperator )
	{
	 	case ethPortQoSRuleMatchOperator_never:
			operator = CTC_VALIDATION_OPERS_NEVER_MATCH;
			break;
	 	case ethPortQoSRuleMatchOperator_equal:
			operator = CTC_VALIDATION_OPERS_EQUAL;
			break;
	 	case ethPortQoSRuleMatchOperator_not_equal:
			operator = CTC_VALIDATION_OPERS_NOT_EQUAL;
			break;
	 	case ethPortQoSRuleMatchOperator_less_equal:
			operator = CTC_VALIDATION_OPERS_LESS_THAN_OR_EQUAL;
			break;
	 	case ethPortQoSRuleMatchOperator_greater_equal:
			operator = CTC_VALIDATION_OPERS_GREATER_THAN_OR_EQUAL;
			break;
	 	case ethPortQoSRuleMatchOperator_exist:
			operator = CTC_VALIDATION_OPERS_FIELD_EXIST;
			break;
	 	case ethPortQoSRuleMatchOperator_not_exist:
			operator = CTC_VALIDATION_OPERS_FIELD_NOT_EXIST;
			break;
	 	case ethPortQoSRuleMatchOperator_always:
			operator = CTC_VALIDATION_OPERS_ALWAYS_MATCH;
			break;
		default:
			operator = CTC_VALIDATION_OPERS_NEVER_MATCH;
			break;
	}
	return operator;
}
int CTC_ethPortQoSRuleAction_set( ULONG devIdx,  ULONG portIdx,  ULONG val)
{
	CTC_STACK_classification_rules_t  rules;
	CTC_STACK_classification_rule_t	  *pRule;
	CTC_STACK_classification_rule_mode_t   mode;
	int i = 1, j = 0;
	CTC_STACK_value_t match_value;
	
	int rc = CTC_STACK_EXIT_ERROR;
	
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;

	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;
	
	portIdx--;

	if( val == 1 )		/* noop */
	{
	}
	else if( val == 2 ) /* delete */
	{
		VOS_MemZero( (VOID*)&rules, sizeof(CTC_STACK_classification_rules_t) );
		for( i=0; i<CTC_MAX_CLASS_RULES_COUNT; i++ )
		{
			pRule = &rules[i];
			pRule->valid = FALSE;
		}

		onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark = val;
		i = 0;

		VOS_MemZero( (VOID*)&match_value, sizeof(match_value) );
		if( qosRuleFieldValue_StrToVal_parase( onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect-1,
				onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal,
				&match_value ) == VOS_OK )
		{
			pRule = &rules[i];
			pRule->valid = TRUE;
			pRule->queue_mapped = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSQueueMapped;
			pRule->priority_mark = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark;
			pRule->num_of_entries = 1;
			pRule->entries[j].field_select = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect - 1;
			/*VOS_MemCpy( &pRule->entries[j].value, &match_value, sizeof(CTC_STACK_value_t) );*/
			pRule->entries[j].value.match_value = match_value.match_value;
			VOS_MemCpy( pRule->entries[j].value.mac_address, match_value.mac_address, 6 );
			pRule->entries[j].validation_operator = qosRuleFieldMatchOperator(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator);

			mode = CTC_CLASSIFICATION_DELETE_RULE;
			rc = CTC_STACK_set_classification_and_marking( olt_id, llid, portIdx+1, mode, rules );
		}
		else
			return VOS_ERROR;
	}
	else if( val == 3 )	/* add */
	{
		VOS_MemZero( (VOID*)&rules, sizeof(CTC_STACK_classification_rules_t) );
		for( i=0; i<CTC_MAX_CLASS_RULES_COUNT; i++ )
		{
			pRule = &rules[i];
			pRule->valid = FALSE;
		}
		i = 1;
		
		VOS_MemZero( (VOID*)&match_value, sizeof(match_value) );
		if( qosRuleFieldValue_StrToVal_parase( onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect-1,
				onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal,
				&match_value ) == VOS_OK )
		{
			pRule = &rules[i];
			pRule->valid = TRUE;
			pRule->queue_mapped = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSQueueMapped;
			pRule->priority_mark = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark;
			pRule->num_of_entries = 1;
			pRule->entries[j].field_select = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect - 1;
			/*VOS_MemCpy( &pRule->entries[j].value, &match_value, sizeof(CTC_STACK_value_t) );*/
			pRule->entries[j].value.match_value = match_value.match_value;
			VOS_MemCpy( pRule->entries[j].value.mac_address, match_value.mac_address, 6 );
			pRule->entries[j].validation_operator = qosRuleFieldMatchOperator(onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator);

			mode = CTC_CLASSIFICATION_ADD_RULE;
			rc = CTC_STACK_set_classification_and_marking( olt_id, llid, portIdx+1, mode, rules );
		}
		else
			return VOS_ERROR;
	}
	else if( val == 4 ) /* delAll */
	{
		rc = CTC_STACK_delete_classification_and_marking_list( olt_id, llid, portIdx+1 );
	}
	else if( val == 5 ) /* get */
	{
		pRule = NULL;
		rc = CTC_STACK_get_classification_and_marking( olt_id, llid, portIdx+1, rules );
		if( rc == CTC_STACK_EXIT_OK )
		{
			/* 检索第一条规则 */
			for( i=0; i<CTC_MAX_CLASS_RULES_COUNT; i++ )
			{
				pRule = &rules[i];
				if( pRule->valid == TRUE )
					break;
			}

			/* 仅处理第一条规则 */
			if( i < CTC_MAX_CLASS_RULES_COUNT )
			{
				onuEthPort[olt_id][onu_id][portIdx].ethPortQoSQueueMapped = pRule->queue_mapped;
				onuEthPort[olt_id][onu_id][portIdx].ethPortQoSPriMark = pRule->priority_mark;
				onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesSelect = pRule->entries[j].field_select + 1;

				rc = qosRuleFieldValue_ValToStr_parase(pRule->entries[j].field_select, 
						&pRule->entries[j].value,
						onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleEntriesMatchVal );
				switch( pRule->entries[j].validation_operator )
				{
				 	case CTC_VALIDATION_OPERS_NEVER_MATCH:
				 	case CTC_VALIDATION_OPERS_EQUAL:
				 	case CTC_VALIDATION_OPERS_NOT_EQUAL:
				 	case CTC_VALIDATION_OPERS_LESS_THAN_OR_EQUAL:
				 	case CTC_VALIDATION_OPERS_GREATER_THAN_OR_EQUAL:
				 	case CTC_VALIDATION_OPERS_FIELD_EXIST:
				 	case CTC_VALIDATION_OPERS_FIELD_NOT_EXIST:
				 	case CTC_VALIDATION_OPERS_ALWAYS_MATCH:
						onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator = 
									pRule->entries[j].validation_operator + 1;
						break;
					default:
						onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator = ethPortQoSRuleMatchOperator_never;
						break;
				}
			}
		}
	}
	/*else
		return VOS_ERROR;*/

	if( rc != CTC_STACK_EXIT_OK )
		return VOS_ERROR;
	
	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleAction = val;
	return VOS_OK;
}

int CTC_ethPortQoSRuleMatchOperator_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator;
	return VOS_OK;
}
int CTC_ethPortQoSRuleMatchOperator_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 8 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	

	onuEthPort[olt_id][onu_id][portIdx].ethPortQoSRuleMatchOperator = val;

	return VOS_OK;
}
#endif

#if 1
/* 临时定义，一个端口只能设置一个组播vlan*/
int CTC_ethPortMulticastVlanVid_get( ulong_t devIdx, ulong_t portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_multicast_vlan_t  m_vlan;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT)|| portIdx == 0 )
		return VOS_ERROR;

	if( parse_llid_command_parameter( devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;
	
	m_vlan.vlan_operation = CTC_MULTICAST_VLAN_OPER_CLEAR;

	if( CTC_STACK_get_multicast_vlan( olt_id, llid, portIdx, &m_vlan) != CTC_STACK_EXIT_OK )
		return VOS_ERROR;

	if( m_vlan.num_of_vlan_id == 0 )
	{
		/*onuEthPort[olt_id][onu_id][port_id].ethPortMulticastVlanVid = 0;*/
		*pVal = 0;
	}
	else
	{
		/*onuEthPort[olt_id][onu_id][port_id].ethPortMulticastVlanVid = m_vlan.vlan_id[0];*/
		if( m_vlan.vlan_id[0] <= 4094 )
			*pVal = m_vlan.vlan_id[0];
		else
			*pVal = 0;
	}
	return VOS_OK;
}
int CTC_ethPortMulticastVlanVid_set( ulong_t devIdx, ulong_t portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_STACK_multicast_vlan_t  m_vlan;

	if( (val > 4094) || (portIdx > MAX_ONU_ETHPORT)|| portIdx == 0 )
		return VOS_ERROR;

	if( parse_llid_command_parameter( devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;

	/*VOS_MemZero( m_vlan.vlan_id, sizeof(m_vlan.vlan_id) );*/

	if( val == 0 )		/* clear */
	{
		if( CTC_STACK_clear_multicast_vlan(olt_id, llid, portIdx) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}
	else if( val <= 4094 ) /* add */
	{
		m_vlan.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
		m_vlan.vlan_id[0] = (USHORT)val;
		m_vlan.num_of_vlan_id = 1;
		if( CTC_STACK_set_multicast_vlan(olt_id, llid, portIdx, m_vlan) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}
	else
		return VOS_ERROR;

	/*onuEthPort[olt_id][onu_id][port_id].ethPortMulticastVlanVid = val;*/
	return VOS_OK;
}
#endif
enum {
	portAutoNegAbilities_Other			= 0x80000000,
	portAutoNegAbilities_b10baseT		= 0x40000000,
	portAutoNegAbilities_b10baseTFD		= 0x20000000,
	portAutoNegAbilities_b100baseT4		= 0x10000000, 
	portAutoNegAbilities_b100baseTX		= 0x08000000, 
	portAutoNegAbilities_b100baseTXFD	= 0x04000000,
	portAutoNegAbilities_b100baseT2		= 0x02000000,
	portAutoNegAbilities_b100baseT2FD	= 0x01000000,
	portAutoNegAbilities_bfdxPause		= 0x00800000,
	portAutoNegAbilities_bfdxAPause		= 0x00400000,
	portAutoNegAbilities_bfdxSPause		= 0x00200000,
	portAutoNegAbilities_bfdxBPause		= 0x00100000,
	portAutoNegAbilities_b1000baseX		= 0x00080000,
	portAutoNegAbilities_b1000baseXFD	= 0x00040000,
	portAutoNegAbilities_b1000baseT		= 0x00020000,
	portAutoNegAbilities_b1000baseTFD	= 0x00010000
};

ULONG autoNegTechAbility_To_Bits( ULONG number_of_abilities, CTC_STACK_technology_ability_t *pTechAbilitiesList )
{
	int i;
	ULONG bits = 0;

	if( number_of_abilities > MAX_TECHNOLOGY_ABILITY )
		return 0;
		
	for( i=0; i<number_of_abilities; i++ )
	{
		switch( pTechAbilitiesList[i] )
		{
			case CTC_STACK_TECHNOLOGY_ABILITY_UNDEFINED:
			case CTC_STACK_TECHNOLOGY_ABILITY_UNKNOWN:
				bits |= portAutoNegAbilities_Other;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_T:
				bits |= portAutoNegAbilities_b10baseT;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_TFD:
				bits |= portAutoNegAbilities_b10baseTFD;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T4:
				bits |= portAutoNegAbilities_b100baseT4;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TX:
				bits |= portAutoNegAbilities_b100baseTX;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TXFD:
				bits |= portAutoNegAbilities_b100baseTXFD;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_FDX_PAUSE:
				bits |=portAutoNegAbilities_bfdxPause ;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_FDX_A_PAUSE:
				bits |= portAutoNegAbilities_bfdxAPause;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_FDX_S_PAUSE:
				bits |= portAutoNegAbilities_bfdxSPause;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_FDX_B_PAUSE:
				bits |= portAutoNegAbilities_bfdxBPause;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2:
				bits |= portAutoNegAbilities_b100baseT2;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2FD:
				bits |= portAutoNegAbilities_b100baseT2FD;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_X	:
				bits |= portAutoNegAbilities_b1000baseX;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_XFD:
				bits |= portAutoNegAbilities_b1000baseXFD;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_T:
				bits |= portAutoNegAbilities_b1000baseT;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_TFD:
				bits |= portAutoNegAbilities_b1000baseTFD;
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT1:
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT2:
				break;
			case CTC_STACK_TECHNOLOGY_ABILITY_ISO_ETHERNET:
				break;
			default:
				break;
		}
	}
	return bits;
}

/*  { bOther ( 0 ) , b10baseT ( 1 ) , b10baseTFD ( 2 ) , b100baseT4 ( 3 ) , b100baseTX ( 4 ) , b100baseTXFD ( 5 ) , b100baseT2 ( 6 ) , b100baseT2FD ( 7 ) , bfdxPause ( 8 ) , bfdxAPause ( 9 ) , bfdxSPause ( 10 ) , bfdxBPause ( 11 ) , b1000baseX ( 12 ) , b1000baseXFD ( 13 ) , b1000baseT ( 14 ) , b1000baseTFD ( 15 ) } */
int CTC_ethPortAutoNegLocalTecAbility_get( ULONG devIdx, ULONG portIdx,  UCHAR *pVal,  ULONG *pLen)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	
	CTC_STACK_auto_negotiation_technology_ability_t  abilities;
	ULONG bits = 0;

	if( (portIdx > MAX_ONU_ETHPORT) || (pVal == NULL) )
		return VOS_ERROR;
	if( parse_llid_command_parameter( devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;

	if( CTC_STACK_get_auto_negotiation_local_technology_ability(olt_id, llid, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	{
		bits = autoNegTechAbility_To_Bits( abilities.number_of_abilities, abilities.technology);
		
		/*onuEthPort[olt_id][onu_id][port_id].ethPortAutoNegLocalTecAbility = bits;*/
		*pVal = (bits >> 16);
		*pLen = 2;
		return VOS_OK;
	}

	pVal[0] = 0x60;		
	pVal[1] = 0x0f;		
	*pLen = 2;
	
	return VOS_OK;
}
/*  { bOther ( 0 ) , b10baseT ( 1 ) , b10baseTFD ( 2 ) , b100baseT4 ( 3 ) , b100baseTX ( 4 ) , b100baseTXFD ( 5 ) , b100baseT2 ( 6 ) , b100baseT2FD ( 7 ) , bFdxPause ( 8 ) , bFdxAPause ( 9 ) , bFdxSPause ( 10 ) , bFdxBPause ( 11 ) , b1000baseX ( 12 ) , b1000baseXFD ( 13 ) , b1000baseT ( 14 ) , b1000baseTFD ( 15 ) } */
int CTC_ethPortAutoNegAdvTechAbility_get( ULONG devIdx, ULONG portIdx,  UCHAR *pVal,  ULONG *pLen )
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	
	CTC_STACK_auto_negotiation_technology_ability_t  abilities;
	ULONG bits = 0;

	if( (portIdx > MAX_ONU_ETHPORT) || (pVal == NULL) )
		return VOS_ERROR;
	if( parse_llid_command_parameter( devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;

	if( CTC_STACK_get_auto_negotiation_advertised_technology_ability(olt_id, llid, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	{
		bits = autoNegTechAbility_To_Bits( abilities.number_of_abilities, abilities.technology);

		/*onuEthPort[olt_id][onu_id][port_id].ethPortAutoNegAdvTechAbility = bits;*/
		*pVal = (bits>>16);
		*pLen = 2;
		return VOS_OK;
	}
	pVal[0] = 0x20;		
	pVal[1] = 0x0f;		
	*pLen = 2;
	
	return VOS_OK;
}
#if 0
/* { noop（1）， restart（2） }	重新协商*/
int CTC_ethPortAutoNegRestart_get( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG *pVal)
{
	CHECK_ONU_ETHPORT( olt_id, onu_id, port_id );
	if( pVal == NULL )
		return VOS_ERROR;

	*pVal = onuEthPort[olt_id][onu_id][port_id].ethPortAutoNegRestart;
	return VOS_OK;
}
int CTC_ethPortAutoNegRestart_set( PON_olt_id_t olt_id,  PON_onu_id_t onu_id,  unsigned char port_id,  ULONG val)
{
	CHECK_ONU_ETHPORT( olt_id, onu_id, port_id );
	if( val > 12 )
		return VOS_ERROR;

	if( CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, onu_id, port_id)	 == CTC_STACK_EXIT_OK )
		return VOS_OK;

	return VOS_ERROR;
}
#endif
#if 1
STATUS set_ctc_vlan( ulong_t devIdx, ulong_t portIdx, ulong_t vid )
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	CTC_STACK_port_vlan_configuration_t  port_vlan;
	
	if( parse_llid_command_parameter(devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;
	
	if( vid == 0 )		/* transparent */
	{
		port_vlan.mode = CTC_VLAN_MODE_TRANSPARENT;
		port_vlan.default_vlan = 1;
		VOS_MemZero( port_vlan.vlan_list, sizeof(port_vlan.vlan_list) );
		port_vlan.number_of_entries = 0;

		if( CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx, port_vlan) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}
	else /* tag */
	{
		port_vlan.mode = CTC_VLAN_MODE_TAG;
		port_vlan.default_vlan = 1;
		VOS_MemZero( port_vlan.vlan_list, sizeof(port_vlan.vlan_list) );
		port_vlan.number_of_entries = 1;
		port_vlan.vlan_list[0] = vid;

		if( CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx, port_vlan) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}

	return VOS_OK;
}

int set_ctc_mctrl( ulong_t devIdx, ulong_t portIdx, ulong_t vid, uchar_t mac, ulong_t userId )
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	CTC_STACK_multicast_control_t  multicast_control;
	short ret;
	
	if( parse_llid_command_parameter(devIdx, &olt_id, &onu_id, &llid) == VOS_ERROR )
		return VOS_ERROR;

	multicast_control.control_type = CTC_STACK_MULTICAST_CONTROL_TYPE_DA_VID;
	multicast_control.action = CTC_MULTICAST_CONTROL_ACTION_ADD;
	multicast_control.num_of_entries = 1;
	multicast_control.entries[0].da[0] = 0x01;
	multicast_control.entries[0].da[1] = 0x00;
	multicast_control.entries[0].da[2] = 0x5e;
	multicast_control.entries[0].da[3] = 0x01;
	multicast_control.entries[0].da[4] = 0x01;
	multicast_control.entries[0].da[5] = mac;
	multicast_control.entries[0].vid = vid;
	multicast_control.entries[0].user_id = userId;
	
	if( (ret = CTC_STACK_set_multicast_control( olt_id, llid, multicast_control )) == 0 )
	{
		sys_console_printf("multicast_control OK\r\n");
		return VOS_OK;
	}
	sys_console_printf("multicast_control ERROR, ret = %d\r\n", ret);
	return VOS_ERROR;
}
#endif
#if 0
/*add by shixh@2007/07/16*/
/*show ctc ethernet-link-state*/
DEFUN(
	show_ctc_ethernet_link_state,
	show_ctc_ethernet_link_state_cmd,
	"show ctc ethernet-link-state {port [all | <1-24>]}*1",
	  SHOW_STR
	  CTC_STR
	"Display ethernet port link state\n"
	"Display ethernet port link state\n"
	"Set port\n"
	"All ports number\n"
	"Port number\n"
            )
{
	PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;
	

	unsigned char		number_of_entries = 0;
	CTC_STACK_ethernet_ports_link_state_t    ports_link_state;
	char *linkstatus[] = {"linkdown", "linkup"};

	int ret = CMD_SUCCESS;

	if( parse_onu_command_parameter( vty,  &olt_id, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		ret =  CMD_WARNING;
	}

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
		if( CTC_STACK_get_ethernet_all_port_link_state(olt_id, onu_id, &number_of_entries, ports_link_state)  == CTC_STACK_EXIT_OK ) 
		{
			int i=0;
			vty_out( vty, "total ports is: %d\r\n", number_of_entries );
			for( i=0; i<number_of_entries; i++ )
			{
				if( CTC_STACK_LINK_STATE_DOWN == ports_link_state[i].link_state ||
					ports_link_state[i].link_state == CTC_STACK_LINK_STATE_UP )
					vty_out( vty, "port %d link state is: %s\r\n", i+1 ,linkstatus[ports_link_state[i].link_state]);
			}
			ret = CMD_SUCCESS;
		}
		else
		{
			vty_out( vty, " link state get fail!" );
			ret =  CMD_WARNING;
		}
	}
	else
	{
		UCHAR port = 0;
		CTC_STACK_link_state_t   link_state;
		
		port = VOS_AtoI( argv[0] );
 		
		if( CTC_STACK_get_ethernet_link_state(olt_id, onu_id, port, &link_state)== CTC_STACK_EXIT_OK )
		{
			vty_out( vty, "port %d link state is:%s\r\n", port, linkstatus[link_state]);
			ret = CMD_SUCCESS;
		}
		else
		{
			vty_out( vty, "ethernet link state get fail!" );
			ret =  CMD_WARNING;
		}
	}

	return ret;
}
#endif
#if 0
/*add by shixh@2007/07/16*/
/*show ctc ethernet-pause*/
DEFUN(show_ctc_ethernet_pause,
	  show_ctc_ethernet_pause_cmd,
      "show ctc ethernet-pause {port [all | <1-24>]}*1",
	  SHOW_STR
	  CTC_STR
	  "Display ethernet port pause \n"
	  "set port\n"
	  "All port\n"
	  "port number\n")
{

       PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;
	
 	 ulong_t   devIdx;
	 ulong_t   brdIdx;
	 ulong_t   ethIdx;
 	 ulong_t   status;
  	 ulong_t   pauseState ;
         ulong_t  pon;
         ulong_t  onu;
	 
	int ret = CMD_SUCCESS;
	 /*   ULONG ulIfIndex = 0;	*/

	 char * pauseStatus[] = {"error","disable", "enable"};
	 
  	ULONG portNum = 0;   

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > 24 )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	vty_out(vty, " devIdx=%d, portNum = %d", devIdx,portNum);
	
	/*if( parse_onu_command_parameter( vty,  &olt_id, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		ret =  CMD_WARNING;
	}*/
	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
		for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		{
			if( getEthPortPauseAdminStatus(devIdx, brdIdx, ethIdx,&status)  == CTC_STACK_EXIT_OK ) 
			{
			vty_out( vty, "test state is: %d\r\n", status);/*for test*/
				if( status > 2 )
					status = 2;
				
				vty_out( vty, "port %d ethernet pause state is: %s\r\n", ethIdx,pauseStatus[status]);
				
				ret = CMD_SUCCESS;
			}
			else
			{
				/*vty_out( vty, " phy  admin state get fail!" );*/
				ret =  CMD_WARNING;
				break;
			}
		}
	}

	else
	{
		
		ethIdx = VOS_AtoI( argv[0] );
		if( ethIdx == 0 || ethIdx > portNum )
		{
			vty_out( vty, " ethernet port error!" );
			return CMD_WARNING;
		}
		
		if( getEthPortPauseAdminStatus(devIdx, brdIdx, ethIdx,&status)  == CTC_STACK_EXIT_OK )
		{
				if( status > 2 )
					status = 2;
			vty_out( vty, "port %d ethernet pause state is:%s\r\n", ethIdx,pauseStatus[status]);
			ret = CMD_SUCCESS;
		}
		else
		{
			vty_out( vty, " ethernet pause state get fail!" );
			ret =  CMD_WARNING;
		}
	}

	return ret;
}
#endif

/*add by shixh@2007/08/02*/
/*set ethernet-pause*/
DEFUN(ethernet_pause_set,
	  ethernet_pause_set_cmd,
         "ethernet-pause port [<port_list>|all] [enable|disable]",
	  "set ethernet information\n"
	  "set ctc ethernet information\n "
	  "set ctc ethernet pause port\n"
	  "set port\n"
	  "all port\n"
	  "port number\n"
	  "port enable\n"
	  "port disable\n"
	  ) 
{
ulong_t devIdx;
ulong_t brdIdx;
ulong_t ethIdx;
ulong_t status;
ulong_t portNum;
ulong_t pon;
ulong_t onu;

 /*PON_olt_id_t		       olt_id;
 PON_onu_id_t		onu_id;*/

/*char * pauseStatus[] = {"error","disable", "enable"};*/

if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if(argc==2)	
	{
		if (VOS_StriCmp(argv[1], "enable")==0)
		{
			status=2;
		}
		else
		{
			status=1;
		}
		if ( 0 != VOS_StrCmp( argv[ 0 ], "all" ) )
		{
			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
			END_PARSE_PORT_LIST_TO_PORT();

			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			{
				if(setEthPortPauseAdminStatus( devIdx, brdIdx, ethIdx,  status )==VOS_ERROR)
					vty_out(vty, " port=%d pause status set fail!\r\n", ethIdx);	
			}
			END_PARSE_PORT_LIST_TO_PORT();
              }
		else
		{
			for( ethIdx=1; ethIdx<= portNum; ethIdx++ )
			{
				if(setEthPortPauseAdminStatus( devIdx, brdIdx, ethIdx,  status )==VOS_ERROR)
					vty_out(vty, " port=%d pause status set fail!\r\n", ethIdx);
			}
		}
	}	

	return  CMD_SUCCESS;
}

/*add by shixh@2007/08/02*/
/*show  ethernet-policing*/
DEFUN(show_ethernet_policing,
	  show_ethernet_policing_cmd,
         "show ethernet-policing {port [all|<port_list>]}*1",
	  SHOW_STR
	  "show ethernet policing\n"
	  "set port\n"
	  "All port\n"
	  "port number\n"
	  )
{
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t  ethIdx;
	ulong_t   pvalue;
       ulong_t     pon;
	ulong_t     onu;

	ULONG portNum = 0;  
	   

	int   ret = CMD_SUCCESS;
       char * pauseStatus[] = {"error","enable", "disable"};
	
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	 if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	         {
		  vty_out(vty, "no ethernet port\r\n");
		  return CMD_WARNING;
	          }
	    
	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
	       for( ethIdx=1; ethIdx<= portNum; ethIdx++ )
		{
			vty_out( vty, " ethernet port%d policing:", ethIdx);
			if(CTC_ethPortPolicingCIR_get(devIdx, ethIdx, &pvalue )== VOS_OK )
				vty_out( vty, " CIR:%d", pvalue);
			
			if( CTC_ethPortPolicingCBS_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK)
				vty_out( vty, " CBS:%d", pvalue);

			if( CTC_ethPortPolicingEBS_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " EBS:%d", pvalue);

			if( CTC_ethPortPolicingEnable_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " %s\r\n", pauseStatus[pvalue]);
			
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
			{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			vty_out( vty, " ethernet port%d policing:", ethIdx);
			if(CTC_ethPortPolicingCIR_get(devIdx, ethIdx, &pvalue)== VOS_OK )
				vty_out( vty, " CIR is:%d", pvalue);
			
			if( CTC_ethPortPolicingCBS_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " CBS:%d", pvalue);

			if( CTC_ethPortPolicingEBS_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " EBS:%d", pvalue);

			if( getEthPortPolcingEnable(devIdx, 1, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " %s\r\n", ethIdx,pauseStatus[pvalue]);
			
		}
         END_PARSE_PORT_LIST_TO_PORT();
		}
             		
		return   ret;
	}

	
#if 0
DEFUN  (
    olt_pon_auto_updated_show,
    olt_pon_auto_updated_show_cmd,
    "show onu software auto-update [enable|disable] {pon <slotId/port>}*1",
    DescStringCommonShow
    "Show the onu information\n"
    "Show the onu information\n"
    "Show onu auto-update information\n"
    "Show enabled onu auto-update information\n"
    "Show disabled onu auto-update information\n"
    "Show pon`s onu auto-update information\n"
    "Please input slotId/port\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;	
    INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;
	vty_out( vty, "" );


	if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
		EnableFlag = 1;
	else if ( !VOS_StrCmp( argv[ 0 ], "disable" ) )
		EnableFlag = 2;
	else
	{
	    vty_out( vty, "  %% Parameter is error.\r\n" );
	    return CMD_WARNING;
	}

	if (argc == 1)
	{
	    if ( EnableFlag == 1 )
				ShowOnuSoftwareAutoUpdateAllEnabledByVty( vty );
		else if ( EnableFlag == 2 )
			ShowOnuSoftwareAutoUpdateAllDisabledByVty( vty );
	}
	else if (argc == 2)
	{
		VOS_Sscanf(argv[1], "%d/%d", &ulSlot, &ulPort);
	    phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if(phyPonId == (VOS_ERROR))
		{
			vty_out( vty, "  %% Parameter error.Please input right slotId/port.\r\n");
			return CMD_WARNING;
		}
		if (EnableFlag == 1)
			ShowOnuSoftwareAutoUpdatePonEnabledByVty( phyPonId, vty );
		else if (EnableFlag == 2)
			ShowOnuSoftwareAutoUpdatePonDisabledByVty( phyPonId, vty );		
	}

    return CMD_SUCCESS;
}
#endif

 
/*add by shixh@2007/08/02*/
/*set ethernet-policing*/
DEFUN(ethernet_policing_set,
	  ethernet_policing_set_cmd,
     "ethernet-policing port [all|<port_list>] cir <1-1000000> {[cbs] <0-1000000>}*1 {[ebs] <0-1000000>}*1",
     "set ethernet-policing\n"
     "set ethernet-policing port\n"
     "all port\n"
     "port number<1-24>\n"
     "committed information rate\n"
     "cir value, unit:kbps\n"
     "depth of this token bucket to tolerant the certain burst\n"
     "cbs value, unit:byte\n"
     "the extra token to permet the forwarding engine to finish the packet being sent\n"
     "ebs value, unit:byte\n") 
{
	
       int   i;
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t   ethIdx;
	ulong_t      pon;
       ulong_t      onu;
	ulong_t     Cbs = 1600;
	ulong_t     Ebs = 0;
	ulong_t   Cir = 0;

	ulong_t    portNum;
	
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
			
	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		(portNum > MAX_ETH_PORT_NUM) )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

       Cir=VOS_AtoI( argv[1] );
	if( Cir > 1000000 )
	{
		vty_out(vty, " CIR set error\r\n");	 
		return CMD_WARNING;
	}
	for(i=2;i<argc;i++)
	{
		if( VOS_StriCmp( argv[i], "cbs") == 0 )
		{
			Cbs=VOS_AtoI(argv[i+1]);
			if( Cbs > 100000 )
			{
				vty_out(vty, " CBS set error\r\n");	 
				return CMD_WARNING;
			}
		}
		else if( VOS_StriCmp(argv[i], "ebs") == 0 )
		{
			Ebs=VOS_AtoI(argv[i+1]);
			if( Ebs > 100000 )
			{
				vty_out(vty, " EBS set error\r\n");	 
				return CMD_WARNING;
			}
		}
		else
		{
			vty_out(vty, " para error!!\r\n");	
		}
		i++;           
	}

	if (VOS_StriCmp(argv[0], "all")==0)
	{
   		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
    		{
			if(setEthPortPolcingCIR( devIdx, 1, ethIdx, Cir )==VOS_ERROR)
				vty_out(vty, " Cir set error\r\n");	 

			if(setEthPortPolcingCBS( devIdx, 1, ethIdx, Cbs )==VOS_ERROR)
		 		 vty_out( vty, "set port %d policing CBS fail!\r\n",ethIdx);

			if(setEthPortPolcingEBS( devIdx, 1, ethIdx, Ebs )==VOS_ERROR)
		 		vty_out( vty, "set port %d policing EBS fail!\r\n",ethIdx);

			if(setEthPortPolcingEnable( devIdx, brdIdx, ethIdx, 1 )==VOS_ERROR)
		    		vty_out( vty, "set port %d policing enable fail!\r\n",ethIdx);	
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		
		if(ethIdx>portNum)
		{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
				return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
			
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(setEthPortPolcingCIR( devIdx, 1, ethIdx, Cir )==VOS_ERROR)
				vty_out( vty, "set port %d policing CIR fail!\r\n",ethIdx );

		      if(setEthPortPolcingCBS( devIdx, 1, ethIdx, Cbs )==VOS_ERROR)
		 	      vty_out( vty, "set port %d policing CBS fail!\r\n",ethIdx );

			if(setEthPortPolcingEBS( devIdx, 1, ethIdx, Ebs )==VOS_ERROR)
		 		vty_out( vty, "set port %d policing EBS fail!\r\n",ethIdx);

			if(setEthPortPolcingEnable( devIdx, brdIdx, ethIdx, 1 )==VOS_ERROR)
		    		vty_out( vty, "set port %d policing enable fail!\r\n",ethIdx);	
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	return  CMD_SUCCESS;					 
}
	
/*add by shixh@2007/08/02*/
/*undo ethernet-policing*/
DEFUN(undo_ethernet_policing,
	  undo_ethernet_policing_cmd,
     "undo ethernet-policing port [all|<port_list>]",
     "undo operation\n"
     "undo port  ethernet-policing\n"
     "all port\n"
     "port number<1-24>\n") 
{
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t   ethIdx;
	ulong_t      pon;
       ulong_t      onu;
	   
	ulong_t    portNum;         
	
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
     if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if(argc==1)
		{
	  if (VOS_StriCmp(argv[0], "all")==0)
	     for(ethIdx=1;ethIdx<=portNum;ethIdx++)
	    	{
	    	if(setEthPortPolcingEnable( devIdx, brdIdx, ethIdx, 2 )==VOS_ERROR)
	          vty_out( vty, "set port %d policing Enable fail!\r\n",ethIdx);
	    	}
	 else
	 	{
	 	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
	 	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
		if(setEthPortPolcingEnable( devIdx, brdIdx, ethIdx, 2 )==VOS_ERROR)
	    		vty_out( vty, "set port %d policing enable fail!\r\n",ethIdx);	
		}
		END_PARSE_PORT_LIST_TO_PORT();
	 	}
		}	 	
	   /*else
		vty_out(vty, " the number of para error!!\r\n");*/
		return   CMD_WARNING;
 	}
               
#if 0
/*add by shixh@2007/07/17*/
/*show ctc phy-admin-state*/
DEFUN(
	show_ctc_phy_admin_state,
	show_ctc_phy_admin_state_cmd,
	"show ctc phy-admin-state {port [all | <1-24>]}*1",
	  SHOW_STR
	  CTC_STR
	"Display phy admin state\n"
	"Display phy admin  state\n"
	"Set port\n"
	"All ports number\n"
	"Port number\n"
            )
{
 	 ULONG devIdx, slotno;
	 ULONG pon;
	 ULONG onu;
	 ULONG ethPort;
 	 ULONG adminState ;
	 ULONG  portNum;
	 
	    ULONG ulIfIndex = 0;	

	 char *adminstatus[] = {"error","enable", "disable"};
	 
	int ret = CMD_SUCCESS;
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if(parse_onu_devidx_command_parameter( /*ulIfIndex*/vty, &devIdx, &slotno, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > 24 )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	vty_out(vty, " devIdx=%d, portNum = %d", devIdx,portNum);
	

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{

	
		
		for(ethPort=1; ethPort<=portNum; ethPort++ )
		{
			if( getEthPortAdminStatus(devIdx, slotno, ethPort,&adminState)  == CTC_STACK_EXIT_OK ) 
			{
				/*vty_out( vty, "total ports is: %d\r\n", ethPort );*/
				{
					vty_out( vty, "port %d phy state is: %s\r\n", ethPort,adminstatus[adminState]);
				}
				ret = CMD_SUCCESS;
			}
			else
			{
				/*vty_out( vty, " phy  admin state get fail!" );*/
				ret =  CMD_WARNING;
				break;
			}
		}
	}

	else
	{
		
		ethPort = VOS_AtoI( argv[0] );
		
		
		if( getEthPortAdminStatus(devIdx, slotno, ethPort,&adminState)  == CTC_STACK_EXIT_OK )
		{
			vty_out( vty, "port %d phy admin state is:%s\r\n", ethPort,adminstatus[adminState]);
			ret = CMD_SUCCESS;
		}
		else
		{
			vty_out( vty, " phy admin state get fail!" );
			ret =  CMD_WARNING;
		}
	}

	return ret;
}
#endif

/*add by shixh@2007/08/06*/
/*set ethernet shutdown*/
DEFUN(ethernet_shutdown,
	  ethernet_shutdown_cmd,
     "ethernet shutdown port <port_list>",
    "set ethernet\n"
     "set ctc ethernet-shutdown port\n"
     "port number\n") 
{
	ulong_t devIdx;
	ulong_t brdIdx;
	ulong_t ethIdx = 0;
	ulong_t   pon;
	ulong_t   onu;
	ulong_t   portNum;
	

	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	

	if(argc==1)
		{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
		if (setEthPortAdminStatus(devIdx,brdIdx,ethIdx,2)==VOS_ERROR)
		   	vty_out(vty, "set  port%d admin state fail!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
		#if 0
		 ethIdx = VOS_AtoI( argv[0] );
		 if (ethIdx<25&&ethIdx>0)
		 	{
	            if (setEthPortAdminStatus(devIdx,brdIdx,ethIdx,2)==VOS_OK)
		   	vty_out(vty, " port%d admin state is down!\r\n", ethIdx);
		 	}
		 else
		 	vty_out(vty, " out of the range<1~24>\r\n ");
		 #endif
		}
	/*else
		vty_out(vty, " the number of para error!!\r\n ", ethIdx);*/
	return    CMD_SUCCESS;
		
}


/*add by shixh@2007/08/06*/
/*undo ethernet shutdown*/
DEFUN(undo_ethernet_shutdown,
	  undo_ethernet_shutdown_cmd,
     "undo ethernet shutdown port <port_list>",
     "undo operation\n"
     "undo ctc ethernet-shutdown port\n"
     "port number\n") 
{
	ulong_t devIdx;
	ulong_t brdIdx;
	ulong_t ethIdx = 0;
	ulong_t   pon;
	ulong_t   onu;
	ulong_t   portNum;
	

	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	

	if(argc==1)
		{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
		 if (setEthPortAdminStatus(devIdx,brdIdx,ethIdx,1)==VOS_ERROR)
		   	vty_out(vty, "set port%d admin state fail!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
		#if 0
		ethIdx = VOS_AtoI( argv[0] );
	        if (ethIdx<25&&ethIdx>0)
		 	{
	            if (setEthPortAdminStatus(devIdx,brdIdx,ethIdx,1)==VOS_OK)
		   	vty_out(vty, " port%d admin state is up!\r\n", ethIdx);
		 	}
		 else
		 	vty_out(vty, " out of the range<1~24>\r\n ");
		 #endif
		}
	/*else
		vty_out(vty, " the number of para error!!\r\n ", ethIdx);*/
		 return    CMD_SUCCESS;
		
}
		
#if 0
/*add by shixh@2007/07/19*/
/*show ctc auto-negotiation-admin-state*/
DEFUN(
	show_ctc_auto_negotiation_admin_state,
	show_ctc_auto_negotiation_admin_state_cmd,
	"show ctc auto-negotiation-admin-state {port [all | <1-24>]}*1",
	  SHOW_STR
	  CTC_STR
	"Display auto-negotiation admin state\n"
	"Display auto-negotiation admin  state\n"
	"Set port\n"
	"All ports number\n"
	"Port number\n"
            )
{
 	 ulong_t devIdx;
	 ulong_t brdIdx;
	 ulong_t ethIdx;
	 ulong_t  status;
	 ulong_t  portNum;
	 
	 
	 ULONG pon;
	 ULONG onu;
	 ULONG ethPort;
 	
	 
	    ULONG ulIfIndex = 0;	

	 char *ANtatus[] = {"error","enable", "disable"};
	 
	int ret = CMD_SUCCESS;
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if(parse_onu_devidx_command_parameter( /*ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	/*( parse_onu_command_parameter( vty,  &olt_id, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		ret =  CMD_WARNING;
	}*/

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > 24 )
	         {
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	          }
	     vty_out(vty, " devIdx=%d, portNum = %d\r\n", devIdx,portNum);

		
		for( ethIdx=1; ethIdx<=portNum;ethIdx++ )
		{
			if( getEthPortAnAdminStatus(devIdx, brdIdx, ethIdx,&status)  == CTC_STACK_EXIT_OK ) 
			{
				/*vty_out( vty, "total ports is: %d\r\n", ethPort );*/
				{
					vty_out( vty, "port %d AN state is: %s\r\n",ethIdx,ANtatus[status]);
				}
				ret = CMD_SUCCESS;
			}
			else
			{
				/*vty_out( vty, " phy  admin state get fail!" );*/
				ret =  CMD_WARNING;
				break;
			}
		}
	}

	else
	{
		
		ethPort = VOS_AtoI( argv[0] );
		
		
		if(getEthPortAnAdminStatus(devIdx, brdIdx, ethPort,&status)== CTC_STACK_EXIT_OK )
		{
			vty_out( vty, "port %d AN admin state is:%s\r\n", ethPort,ANtatus[status]);
			ret = CMD_SUCCESS;
		}
		else
		{
			vty_out( vty, " AN admin state get fail!" );
			ret =  CMD_WARNING;
		}
	}

	return ret;
}
#endif

/*add by shixh@2007/08/13*/
/*show ctc  auto-negotiation-local-technology-ability*/
DEFUN(show_auto_negotiation_technology_ability,
	  show_auto_negotiation_technology_ability_cmd,
         "show auto-technology-ability {port <port_list>}*1",
	  SHOW_STR
	  "show auto-negotiation local technology ability\n"
	  "set port\n"
	  "port number\n")
{
      	 ulong_t   devIdx;
	 ulong_t   brdIdx;	 
	 ulong_t   ethIdx;
	 char       pstr;
	 ulong_t  pLen;
	 
         ulong_t  pon;
         ulong_t  onu;
	   int   i;

	 char * str[] = {	"bOther(0)", "b10baseT(1)" , "b10baseTFD(2)" , "b100baseT4(3)" , "b100baseTX(4)" , "b100baseTXFD(5)", 
	 				"b100baseT2(6)" ,"b100baseT2FD(7)" , "bfdxPause(8)" , "bfdxAPause(9)", "bfdxSPause(10)" ,
	 				"bfdxBPause(11)" ,"b1000baseX(12)", "b1000baseXFD(13)" , "b1000baseT(14)" ,"b1000baseTFD(15)"};
	 
  	ULONG portNum = 0;   

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
		for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		{
			if(  getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx,&pstr, &pLen,1) == CTC_STACK_EXIT_OK ) 
			{
			     vty_out( vty, "port%d auto negotication ability list:\r\n", ethIdx);
				 
			     for(i=0;i<16;i++)
				 	if((pstr>>i)&1)
				vty_out( vty, " %s\r\n", str[i]);
										
			}
			else
				vty_out( vty, " auto negotication ability list get fail!\r\n" );
			
			if(  getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx, &pstr, &pLen,0) == CTC_STACK_EXIT_OK ) 
			{
			     vty_out( vty, "port%d auto negotication advertised ability list:\r\n", ethIdx);
				
			     for(i=0;i<16;i++)
				 	if((pstr>>i)&1)
				vty_out( vty, " %s\r\n", str[i]);
			}
			else
				vty_out( vty, " auto negotication ability advertised  list get fail!\r\n" );
		}
	}

	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
		 if( getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx, &pstr, &pLen,1) == CTC_STACK_EXIT_OK ) 
				{
			     vty_out( vty, "port%d auto negotication ability list:\r\n", ethIdx);
			     for(i=0;i<16;i++)
				if((pstr>>i)&1)
				vty_out( vty, " %s\r\n", str[i]);
										
				}
			else
				vty_out( vty, " auto negotication ability list get fail!\r\n" );
			
			if(  getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx, &pstr, &pLen,0) == CTC_STACK_EXIT_OK ) 
				{
			     vty_out( vty, "port%d auto negotication advertised ability list:\r\n", ethIdx);
			     for(i=0;i<16;i++)
				if((pstr>>i)&1)
				vty_out( vty, " %s\r\n", str[i]);
										
				}
			else
				vty_out( vty, " auto negotication ability advertised  list get fail!\r\n" );
		}
		END_PARSE_PORT_LIST_TO_PORT();
		#if 0
		ethIdx = VOS_AtoI( argv[0] );
		if( ethIdx == 0 || ethIdx > portNum )
		{
			vty_out( vty, " ethernet port error!" );
			return CMD_WARNING;
		}
		
			if( getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx, &pstr, &pLen,1) == CTC_STACK_EXIT_OK ) 
				{
			     vty_out( vty, "port%d auto negotication ability list is: \r\n", ethIdx);
			     for(i=0;i<16;i++)
				if((pstr>>i)&1==1)
				vty_out( vty, "%s\r\n", str[i]);
										
				}
			else
				vty_out( vty, " auto negotication ability list get fail!\r\n" );
			
			if(  getEthPortAutoNegLocalTecAbility( devIdx, 1, ethIdx, &pstr, &pLen,0) == CTC_STACK_EXIT_OK ) 
				{
			     vty_out( vty, "port%d auto negotication advertised ability list is: \r\n", ethIdx);
			     for(i=0;i<16;i++)
				if((pstr>>i)&1==1)
				vty_out( vty, "%s\r\n", str[i]);
										
				}
			else
				vty_out( vty, " auto negotication ability advertised  list get fail!\r\n" );
			#endif
		}
	       return  CMD_SUCCESS;
	}		


/*add by shixh@2007/08/06*/
/*set ctc  auto-negotiation-restart*/
DEFUN(set_auto_negotiation_restart,
	  set_auto_negotiation_restart_cmd,
      "ethernet auto port <port_list> restart",
	   "set ethernet\n"
	  "set port\n"
	  "port number\n")
{
       
	unsigned char        portNum;
	 ULONG   devIdx;
	 ULONG   brdIdx;	 
	 ULONG   portIdx;
	 ULONG    pon;
	 ULONG    onu;
	
	
       
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
				
     if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

     vty_out(vty, "port num is %d\r\n", portNum);
	if(argc==1)
		{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], portIdx )
	
		if(portIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], portIdx )
		{
		  if(setEthPortAutoNegRestart(devIdx,brdIdx,portIdx,2)==VOS_ERROR)
		vty_out(vty, "set port=%d AN restart fail!\r\n", portIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
		#if 0
		portIdx = VOS_AtoI( argv[0] );
		if (portIdx<25&&portIdx>0)
			{
	       if(setEthPortAutoNegRestart(devIdx,brdIdx,portIdx,2)==VOS_OK)
		vty_out(vty, " port=%d is set AN restart!\r\n", portIdx);
			}
		else
			vty_out(vty, " out of range<1~24>\r\n");
		#endif
		} 
	/*else
		vty_out(vty, " the number of para error!!\r\n");*/
	       return  CMD_SUCCESS;
}



/*add by shixh@2007/08/06*/
/*set auto-negotiation*/
DEFUN(set_auto_negotiation,
	  set_auto_negotiation_cmd,
         "ethernet auto port <port_list> [enable|disable]",
	  "set ethernet\n"
	  "set auto-negotiation\n"
	  "set port\n"
	  "port number\n"
	  "set AN enable\n"
	  "set AN disable\n")
{
ulong_t devIdx;
ulong_t brdIdx;
ulong_t ethIdx;
ulong_t status;

ulong_t portNum;
ulong_t pon;
ulong_t onu;

 
	/*char * AnAdminStatus[] = {"error","enable", "disable"};*/

	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if(argc==2)
	{
		if (VOS_StriCmp(argv[1], "enable")==0)
		{
		        status=1;
		}
	       else
       	{
	       	status=2;
       	}

		if(VOS_StriCmp(argv[0], "all")==0)
		{
	   		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
	   		{
				if(setEthPortAnAdminStatus( devIdx, brdIdx, ethIdx,  status )==VOS_ERROR)
					vty_out(vty, "set port=%d AN fail!\r\n", ethIdx);	
	   		}
		}
		else
	  	{
	  		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
			END_PARSE_PORT_LIST_TO_PORT();

	  		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			{
				if(setEthPortAnAdminStatus( devIdx, brdIdx, ethIdx,  status )==VOS_ERROR)
					vty_out(vty, "set port=%d AN fail!\r\n", ethIdx);	
			}
			END_PARSE_PORT_LIST_TO_PORT();
	  	}
	}
	return   CMD_SUCCESS;
}


/*add by shixh@2007/08/06*/
/*show ethernet*/
DEFUN(show_ethernet,
	  show_ethernet_cmd,
      "show ethernet {port <port_list>}*1",
	  SHOW_STR
	  "show ethernet information\n"
	  "set port\n"
	  "All port\n"
	  "port number\n"
	  "Display ethernet port link status\n"
	  "Display ethernet port administrator status\n"
	  "Display ethernet port auto-negotiation\n"
	  "Display ethernet port pause\n")
{
	ulong_t devIdx;
	ulong_t brdIdx;
	ulong_t ethIdx;
	ulong_t status;

	ulong_t portNum;
	ulong_t pon;
	ulong_t onu;

       PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;
	
	unsigned char		number_of_entries = 0;
	CTC_STACK_ethernet_ports_link_state_t    ports_link_state;
	CTC_STACK_link_state_t   link_state;
	char *linkstatus[] = {"linkdown", "linkup"};
       char *adminstatus[] = {"error","up", "down"};
	char *ANstatus[] = {"error","enable", "disable"};
	char * pauseStatus[] = {"error","disable", "enable"};
	
	int ret = CMD_SUCCESS;
	
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if( parse_onu_command_parameter( vty,  &olt_id, 0, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		ret =  CMD_WARNING;
	}

	VOS_MemZero( &ports_link_state[0], sizeof(ports_link_state) );

	vty_out( vty, " port    link-state   admin-state    auto-neg    pause\r\n");

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
		int i=0;
		if( CTC_STACK_get_ethernet_all_port_link_state(olt_id, onu_id, &number_of_entries, ports_link_state)  == CTC_STACK_EXIT_OK ) 
		{
		}
			/*vty_out( vty, "total ports is: %d\r\n", portNum );*/
		for( i=0; i<portNum; i++ )
		{
			ethIdx = i+1;		
			if( ports_link_state[i].link_state != CTC_STACK_LINK_STATE_UP )
				ports_link_state[i].link_state = CTC_STACK_LINK_STATE_DOWN;
			vty_out( vty, " %4d%12s", ethIdx ,linkstatus[ports_link_state[i].link_state]);

			getEthPortAdminStatus(devIdx,brdIdx,ethIdx,&status);
			if( status > 2 ) status = 0;
			vty_out( vty, "%12s", adminstatus[status] );

			getEthPortAnAdminStatus(devIdx,brdIdx,ethIdx,&status);
			if( status > 2 ) status = 0;
			vty_out( vty, "%12s", ANstatus[status]);

			getEthPortPauseAdminStatus(devIdx,brdIdx,ethIdx,&status);
		        if( (status <= 4) && (status > 1) ) 
				status = 2;
			else
				status = 1;
			vty_out( vty, "%12s\r\n", pauseStatus[status]);
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
		 	CTC_STACK_get_ethernet_link_state(olt_id, onu_id, ethIdx, &link_state);
			if( link_state != CTC_STACK_LINK_STATE_UP )
				link_state = CTC_STACK_LINK_STATE_DOWN;
			vty_out( vty, " %4d%12s", ethIdx ,linkstatus[link_state]);

			getEthPortAdminStatus(devIdx,brdIdx,ethIdx,&status);
			if( status > 2 ) status = 0;
			vty_out( vty, "%12s", adminstatus[status] );

			getEthPortAnAdminStatus(devIdx,brdIdx,ethIdx,&status);
			if( status > 2 ) status = 0;
			vty_out( vty, "%12s", ANstatus[status]);

			getEthPortPauseAdminStatus(devIdx,brdIdx,ethIdx,&status);
		        if( status > 2 ) status = 0;
			vty_out( vty, "%12s\r\n", pauseStatus[status]);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return CMD_SUCCESS;
}


DEFUN(show_ethernet_ds_ratelimiting,
	  show_ethernet_ds_ratelimiting_cmd,
         "show ds-ratelimiting {port [all|<port_list>]}*1",
	  SHOW_STR
	  "show ethernet port downstream rate limiting\n"
	  "set port\n"
	  "All port\n"
	  "port number\n"
	  )
{
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t  ethIdx;
	ulong_t   pvalue;
       ulong_t     pon;
	ulong_t     onu;

	ULONG portNum = 0;  
	   

	int   ret = CMD_SUCCESS;
       char * pauseStatus[] = {"error","enable", "disable"};
	
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	 if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		  vty_out(vty, "no ethernet port\r\n");
		  return CMD_WARNING;
	}
	    
	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
	       for( ethIdx=1; ethIdx<= portNum; ethIdx++ )
		{
			vty_out( vty, " port%d downstream rate limiting:", ethIdx);
			if(CTC_ethPortDSPolicingCIR_get(devIdx, ethIdx, &pvalue )== VOS_OK )
				vty_out( vty, " CIR:%d", pvalue);
			
			if( CTC_ethPortDSPolicingPIR_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK)
				vty_out( vty, " PIR:%d", pvalue);

			if( CTC_ethPortDSPolicingEnable_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " %s\r\n", pauseStatus[pvalue]);
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			vty_out( vty, " port%d downstream rate limiting:", ethIdx);
			if(CTC_ethPortDSPolicingCIR_get(devIdx, ethIdx, &pvalue )== VOS_OK )
				vty_out( vty, " CIR:%d", pvalue);
			
			if( CTC_ethPortDSPolicingPIR_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " PIR:%d", pvalue);

			if( CTC_ethPortDSPolicingEnable_get(devIdx, ethIdx, &pvalue)== CTC_STACK_EXIT_OK )
				vty_out( vty, " %s\r\n", pauseStatus[pvalue]);
			
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	return   ret;
}

DEFUN(ethernet_ds_ratelimiting_set,
	  ethernet_ds_ratelimiting_set_cmd,
     "ds-ratelimiting port [all|<port_list>] cir <1-1000000> {[pir] <0-10000000>}*1",
     "set ethernet port downstream rate limiting\n"
     "set ethernet-policing port \n"
     "all port\n"
     "port number<1-24>\n"
     "committed information rate\n"
     "cir value, unit:kbps\n"
     "Peak Information Rate\n"
     "pir value, unit:kbps\n") 
{
	
       /*int   i;*/
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t   ethIdx;
	ulong_t      pon;
       ulong_t      onu;
	ulong_t     pir = 0;
	ulong_t   cir = 0;
	ulong_t    portNum;
	
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
			
	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	cir=VOS_AtoI( argv[1] );
	if( cir > 1000000 )
	{
		vty_out(vty, " CIR set error\r\n");	 
	}

	if( argc > 3 )
	{
		if( VOS_StriCmp( argv[3], "pir") == 0 )
		{
			pir=VOS_AtoI(argv[4]);
			if( pir > 1000000 )
			{
				vty_out(vty, " PIR set error\r\n");	 
			}
		}
	}
	if (VOS_StriCmp(argv[0], "all")==0)
	{
   		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
    		{
			if(CTC_ethPortDSPolicingCIR_set( devIdx, ethIdx, cir ) != VOS_OK)
				vty_out(vty, " CIR set error\r\n");	 

			if(CTC_ethPortDSPolicingPIR_set( devIdx, ethIdx, pir ) != VOS_OK)
				vty_out(vty, " PIR set error\r\n");	 

			if( CTC_ethPortDSPolicingEnable_set(devIdx, ethIdx, 1) != VOS_OK)
				vty_out(vty, " downstream rate limiting set error\r\n");
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
			
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(CTC_ethPortDSPolicingCIR_set( devIdx, ethIdx, cir ) != VOS_OK)
				vty_out(vty, " CIR set error\r\n");	

             		if(CTC_ethPortDSPolicingPIR_set( devIdx, ethIdx, pir ) != VOS_OK)
				vty_out(vty, " PIR set error\r\n");

			if( CTC_ethPortDSPolicingEnable_set(devIdx, ethIdx, 1) != VOS_OK)
				vty_out(vty, " downstream rate limiting set error\r\n");
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	return  CMD_SUCCESS;					 
}
	
/*add by shixh@2007/08/02*/
/*undo ethernet-policing*/
DEFUN(undo_ethernet_ds_ratelimiting,
	  undo_ethernet_ds_ratelimiting_cmd,
     "undo ds-ratelimiting port [all|<port_list>]",
     "undo operation\n"
     "undo ethernet port downstream rate limiting\n"
     "all port\n"
     "port number<1-24>\n") 
{
	ulong_t   devIdx;
	ulong_t   brdIdx;
	ulong_t   ethIdx;
	ulong_t      pon;
       ulong_t      onu;

	ulong_t    portNum;         
	
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
			
	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if (VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(CTC_ethPortDSPolicingEnable_set( devIdx, ethIdx, 2 )==VOS_ERROR)
	          		vty_out( vty, "set port %d policing Enable fail!\r\n",ethIdx);
	    	}
	}
	else
	{
	 	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
	
		if(ethIdx>portNum)
		{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
	 	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(CTC_ethPortDSPolicingEnable_set( devIdx, ethIdx, 2 )==VOS_ERROR)
	    			vty_out( vty, "set port %d policing enable fail!\r\n",ethIdx);	
		}
		END_PARSE_PORT_LIST_TO_PORT();
 	}
	return   CMD_WARNING;
}
               

DEFUN(ethernet_loop_detect_set,
	  ethernet_loop_detect_set_cmd,
         "ethernet-loop-detect port [<port_list>|all] [enable|disable]",
	  "set ethernet information\n"
	  "set loop detect port\n"
	  "all port\n"
	  "port number\n"
	  "detect enable\n"
	  "detect disable\n"
	  ) 
{
	ulong_t devIdx;
	ulong_t brdIdx;
	ulong_t ethIdx;
	ulong_t pon;
	ulong_t onu;

	ulong_t portNum;
	ulong_t status;
	
	PON_olt_id_t	olt_id; 
	PON_onu_id_t	onu_id;
	PON_llid_t	llid;
	short ret;
	CTC_management_object_index_t midx;

	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

       if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if (VOS_StriCmp(argv[1], "enable")==0)
		status = CTC_STACK_PORT_LOOP_DETECT_STATE_ACTIVATED;
	else
		status = CTC_STACK_PORT_LOOP_DETECT_STATE_DEACTIVATED;

	midx.frame_number = 0;
	midx.slot_number = 0;
	midx.port_number = 1;
	midx.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if ( 0 != VOS_StrCmp( argv[ 0 ], "all" ) )
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			midx.port_number = ethIdx;
			if( (ret = CTC_STACK_set_management_object_port_loop_detect(olt_id, llid, midx, status)) != CTC_STACK_EXIT_OK)
				vty_out( vty, " port=%d loop detect set fail(%d)!\r\n", ethIdx, ret );	
		}
		END_PARSE_PORT_LIST_TO_PORT();
       }
	else
	{
		for( ethIdx=1; ethIdx<= portNum; ethIdx++ )
		{
			if( (ret = CTC_STACK_set_management_object_port_loop_detect(olt_id, llid, midx, status)) != CTC_STACK_EXIT_OK)
				vty_out(vty, " port=%d loop detect set fail(%d)!\r\n", ethIdx, ret);
		}
	}

	return  CMD_SUCCESS;
}



LONG  CT_EthCli_Init()
{
	/*install_element ( ONU_CTC_NODE, &show_ctc_ethernet_link_state_cmd );*/
	/*install_element ( ONU_CTC_NODE, &show_ctc_phy_admin_state_cmd );*/
	/*install_element ( ONU_CTC_NODE, &show_ctc_ethernet_pause_cmd );*/
	install_element ( ONU_CTC_NODE, &ethernet_pause_set_cmd );
	
	install_element ( ONU_CTC_NODE, &show_ethernet_policing_cmd );
	install_element ( ONU_CTC_NODE, &ethernet_policing_set_cmd );
	install_element ( ONU_CTC_NODE, &undo_ethernet_policing_cmd );


	install_element ( ONU_CTC_NODE, &ethernet_shutdown_cmd );
	install_element ( ONU_CTC_NODE, &undo_ethernet_shutdown_cmd );
	
	/*install_element ( ONU_CTC_NODE, &show_ctc_auto_negotiation_admin_state_cmd );*/
	install_element ( ONU_CTC_NODE, &show_auto_negotiation_technology_ability_cmd );

	install_element ( ONU_CTC_NODE, &set_auto_negotiation_restart_cmd );
	install_element ( ONU_CTC_NODE, &set_auto_negotiation_cmd );
	install_element ( ONU_CTC_NODE, &show_ethernet_cmd );

#ifdef PAS_SOFT_VERSION_V5_3_5
	install_element ( ONU_CTC_NODE, &show_ethernet_ds_ratelimiting_cmd);
	install_element ( ONU_CTC_NODE, &ethernet_ds_ratelimiting_set_cmd);
	install_element ( ONU_CTC_NODE, &undo_ethernet_ds_ratelimiting_cmd);
#endif

	install_element ( ONU_CTC_NODE, &ethernet_loop_detect_set_cmd);

	return VOS_OK;
}

#endif
#endif



	
