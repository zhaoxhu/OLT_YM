#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "CT_RMan_Main.h"

#ifdef __CT_EXTOAM_SUPPORT


/*----------------------------------------------------------------------------*/

int CT_RMan_VoipPortEbl_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR VoipPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pVoipPortEbl )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_Voip_Req_t *pSendPduData;
	CT_ExtOam_PduData_Voip_Resp_t *pRecvPduData = NULL;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pVoipPortEbl== NULL )
			return VOS_ERROR;
	}

	pSendPduData = (CT_ExtOam_PduData_Voip_Req_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->instanceBranch = 0x36;
	pSendPduData->instanceLeaf = 0x0001;
	pSendPduData->instanceWidth = 1;
	pSendPduData->instanceValue = VoipPort;
	pSendPduData->variableBranch = 0xc7;
	pSendPduData->variableLeaf = 0x0014;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_Voip_Req_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "Voip : get, CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL)  )
			{
				sys_console_printf( "Voip_get RECV\r\n" );
				if( recvPduDataLen > sizeof(CT_ExtOam_PduData_Voip_Resp_t) )
					recvPduDataLen = sizeof(CT_ExtOam_PduData_Voip_Resp_t);
				
				if( (pRecvPduData->enable == CT_Rman_Voip_Luck) || (pRecvPduData->enable == CT_Rman_Voip_Unluck) )
				{
					sys_console_printf( "Voip_get OK\r\n" );
					*pVoipPortEbl = pRecvPduData->enable;
				}
				else
				{
					rc = CT_Ext_OAM_Send_Resp_Err;
				}
			}
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
	}
	else
	{
		sys_console_printf( "Voip : get, CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;

	return VOS_ERROR;
}

int CT_RMan_VoipPortEbl_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR VoipPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR VoipPortEbl )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_Voip_Resp_t *pSendPduData;
	CT_ExtOam_PduData_Voip_Resp_t *pRecvPduData = NULL;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( (VoipPortEbl != CT_Rman_Voip_Luck) && (pRecvPduData->enable != CT_Rman_Voip_Unluck)  )
			return VOS_ERROR;

	pSendPduData = (CT_ExtOam_PduData_Voip_Resp_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->descriptor.OUI, 3 );
	pSendPduData->descriptor.extOpcode = OAM_ExtOpcode_Set_Request;
	pSendPduData->descriptor.instanceBranch = 0x36;
	pSendPduData->descriptor.instanceLeaf = 0x0001;
	pSendPduData->descriptor.instanceWidth = 1;
	pSendPduData->descriptor.instanceValue = VoipPort;
	pSendPduData->descriptor.variableBranch = 0xc7;
	pSendPduData->descriptor.variableLeaf = 0x0014;
	pSendPduData->variableWidth = 1;
	pSendPduData->enable = VoipPortEbl;
	

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_Voip_Resp_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "Voip : set, CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL)  )
			{
				/*sys_console_printf( "Voip_set RECV\r\n" );
				if( recvPduDataLen > sizeof(CT_ExtOam_PduData_Voip_Resp_t) )
					recvPduDataLen = sizeof(CT_ExtOam_PduData_Voip_Resp_t);*/
				
				if( pRecvPduData->variableWidth == 0x80 )
				{
					sys_console_printf( "Voip : set  OK\r\n" );
				}
				else
				{
					sys_console_printf( "Voip : set response value ERROR\r\n" );
					rc = CT_Ext_OAM_Send_Resp_Err;
				}
			}
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
	}
	else
	{
		sys_console_printf( "Voip : get, CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;

	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

#endif /*__CT_EXTOAM_SUPPORT*/


