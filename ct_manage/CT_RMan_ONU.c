#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include "Cdp_pub.h"
#include  "includeFromPas.h"
#include "CT_RMan_Main.h"
#include  "CT_Onu_Auth.h"
#include   "gwEponSys.h"
#include "onu/onuconfmgt.h"

ULONG onuTxPowerDebugSwitch = 0;

extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG ctss_fibsyn_func( void * pEntry, ULONG ulDumpCard, USHORT usCmdType );
extern LONG ctss_l3_np_routemodule_change_hook( VOID * arg1);
extern VOID nmi_cpu_usage_start(VOID) ;
extern LONG ctss_vrrp_hook( void *pstVrrpInfo ) ;
extern LONG ctss_arpsyn_func( void * pEntry,ULONG unIfIndex,void  *pstArpParam,USHORT usCmdType) ;


#if 0
void __Ttttt( void )
{
	PON_GetSlotPortOnu( 0, NULL, NULL, NULL  );
	ctss_fibsyn_func( 0,0,0 );
	ctss_l3_np_routemodule_change_hook( 0 );
	nmi_cpu_usage_start();
	ctss_vrrp_hook(0);
	ctss_arpsyn_func( 0,0,0,0 );
}



/*----------------------------------------------------------------------------*/

int CT_RMan_OnuSN_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag, 
			ULONG call_notify, CT_RMan_OnuSN_t* pSN )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_OnuSN_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pSN == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0xc7;
	pSendPduData->leaf = 0x0001;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( recvPduDataLen < 38 )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d, less than 38\r\n", recvPduDataLen );
				recvPduDataLen = 38;
			}
			
			VOS_MemCpy( pSN, (VOID*)&pRecvPduData->SN, recvPduDataLen );
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_FirmwareVer_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_FirmwareVer_t* pFirmwareVer )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_FirmwareVer_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pFirmwareVer == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0xc7;
	pSendPduData->leaf = 0x0002;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen == 0) || (recvPduDataLen >= 32) )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
				if( recvPduDataLen >= 32 )
					recvPduDataLen = 31;
			}
			
			VOS_MemCpy( pFirmwareVer, (VOID*)&pRecvPduData->FirmwareVer, recvPduDataLen );
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_ChipsetId_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_ChipsetId_t* pChipsetId )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_ChipsetId_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pChipsetId == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0xc7;
	pSendPduData->leaf = 0x0003;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen == 0) || (recvPduDataLen > 8) )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
				if( recvPduDataLen > 8 )
					recvPduDataLen = 8;
			}
			
			VOS_MemCpy( pChipsetId, (VOID*)&pRecvPduData->ChipsetId, recvPduDataLen );
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_Capabilities_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_Capabilities_t* pCapabilities )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_Capabilities_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pCapabilities == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0xc7;
	pSendPduData->leaf = 0x0004;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen == 0) || (recvPduDataLen > 0x1a) )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
				if( recvPduDataLen > 0x1a )
					recvPduDataLen = 0x1a;
			}
			
			VOS_MemCpy( pCapabilities, (VOID*)&pRecvPduData->Capabilities, recvPduDataLen );
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_FECAbility_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag, 
			ULONG call_notify, UCHAR *pFECAbility )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_FECAbility_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pFECAbility == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0x07;
	pSendPduData->leaf = 0x0139;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( recvPduDataLen != 1 )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
				if( recvPduDataLen > 0x1a )
					recvPduDataLen = 0x1a;
			}
			
			*pFECAbility = pRecvPduData->FECAbility;
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_FECMode_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR *pFECMode )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_ONU_GetReq_t *pSendPduData;
	CT_ExtOam_PduData_FECMode_GetResp_t *pRecvPduData;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pFECMode == NULL )
			return VOS_ERROR;
	}
	pSendPduData = (CT_ExtOam_PduData_ONU_GetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->branch = 0x07;
	pSendPduData->leaf = 0x013a;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_ONU_GetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( recvPduDataLen != 0 )
			{
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
				if( recvPduDataLen > 0x1a )
					recvPduDataLen = 0x1a;
			}
			
			*pFECMode = pRecvPduData->FECMode;
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}
	return VOS_ERROR;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_FECMode_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR FECMode )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_FECMode_SetReq_t *pSendPduData;
	CT_ExtOam_PduData_FECMode_SetResp_t *pRecvPduData = NULL;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( FECMode == 0 || FECMode > 3 )
		return VOS_ERROR;
	
	pSendPduData = (CT_ExtOam_PduData_FECMode_SetReq_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->descriptor.OUI, 3 );
	pSendPduData->descriptor.extOpcode = OAM_ExtOpcode_Set_Request;
	pSendPduData->descriptor.branch = 0x07;
	pSendPduData->descriptor.leaf = 0x013a;
	pSendPduData->variable_Width = 1;
	pSendPduData->FECMode = FECMode;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_FECMode_SetReq_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		sys_console_printf( "CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL) )
			{
				if( pRecvPduData->variable_Width == 0x80 )
				{
					sys_console_printf( "FECMode set OK\r\n" );
				}
				else
				{
					rc = CT_Ext_OAM_Send_Resp_Err;
					sys_console_printf( "FECMode set response ERROR\r\n" );
				}
			}
			else
			{
				rc = CT_Ext_OAM_Send_Resp_Err;
				sys_console_printf( "CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
			}
		}
	}
	else
	{
		sys_console_printf( "CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );
	}

	CT_ExtOam_Free( (VOID*)pSendPduData );

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;
	
	return VOS_ERROR;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#include "Vos_string.h"
void testExtOam_ONU_SN_Pdu( UCHAR *pdu, ULONG *pLen )
{
	ULONG length;
	CT_ExtOam_PduData_OnuSN_GetResp_t *pRespPdu;
	CT_ExtOamPdu_t *pOamPdu = (CT_ExtOamPdu_t*)pdu;
		
	VOS_MemCpy( pOamPdu->DA, ct_extoam_da_mac, 6 );
	VOS_MemCpy( pOamPdu->SA, ct_extoam_sa_mac, 6 );
	pOamPdu->OamType = 0x8809;
	pOamPdu->SubType = 3;
	pOamPdu->Flag = 0/*0x50*/;
	pOamPdu->Code = 0xfe;

	pRespPdu = (CT_ExtOam_PduData_OnuSN_GetResp_t *)pOamPdu->PduData;
	pRespPdu->descriptor.extOpcode = OAM_ExtOpcode_Get_Response;
	pRespPdu->descriptor.branch = 0xc7;
	pRespPdu->descriptor.leaf = 0x0001;
	pRespPdu->variable_Width = 38;

	VOS_StrCpy( pRespPdu->SN.vendor_Id, "vvv" );
	pRespPdu->SN.ONU_Model = 12;
	VOS_MemCpy(pRespPdu->SN.ONU_ID, ct_extoam_sa_mac, 6);
	VOS_StrCpy( pRespPdu->SN.hardwareVersion, "v1.r03" );
	VOS_StrCpy( pRespPdu->SN.softwareVersion, "V1.R02" );

	length = sizeof(CT_ExtOam_PduData_OnuSN_GetResp_t) + CT_EXTOAM_PDUDATA_OFFSET;
	if( length < MIN_CT_EXTOAM_FRAME_SIZE )
		length = MIN_CT_EXTOAM_FRAME_SIZE;
	
	*pLen = length;
	
}

VOID showOnuSN_sync( UCHAR slotno, UCHAR pon, UCHAR onu )
{
	CT_RMan_OnuSN_t sn;
	
	VOS_MemZero( &sn, sizeof(CT_RMan_OnuSN_t) );
	if( CT_RMan_OnuSN_get( slotno, pon, onu, call_flag_sync, 0, &sn ) == VOS_OK )
	{
		sys_console_printf( "\r\n ONU SN :\r\n" );
		sys_console_printf( "  vendor_Id = %s\r\n", sn.vendor_Id );
		sys_console_printf( "  ONU_Model = %d\r\n", sn.ONU_Model );
		sys_console_printf( "  ONU_ID = %s\r\n", macAddress_To_Strings(sn.ONU_ID));
		sys_console_printf( "  hardwareVersion = %s\r\n", sn.hardwareVersion );
		sys_console_printf( "  softwareVersion = %s\r\n", sn.softwareVersion );
	}
	else
		sys_console_printf("ERR\r\n");
}

int prnOnuSN(UCHAR ponid, UCHAR onuid, VOID *pPduData, USHORT pduDataLen )
{
	CT_ExtOam_PduData_OnuSN_GetResp_t *pSN_Resp = (CT_ExtOam_PduData_OnuSN_GetResp_t *)pPduData;
	if( (pSN_Resp == NULL) || (pduDataLen == 0) )
		return VOS_ERROR;

	sys_console_printf( "\r\n ONU SN :\r\n" );
	sys_console_printf( "  vendor_Id = %s\r\n", pSN_Resp->SN.vendor_Id );
	sys_console_printf( "  ONU_Model = %d\r\n", pSN_Resp->SN.ONU_Model );
	sys_console_printf( "  ONU_ID = %s\r\n", macAddress_To_Strings(pSN_Resp.SN.ONU_ID));
	sys_console_printf( "  hardwareVersion = %s\r\n", pSN_Resp->SN.hardwareVersion );
	sys_console_printf( "  softwareVersion = %s\r\n", pSN_Resp->SN.softwareVersion );

	CT_ExtOam_Buffer_Free( pPduData );
}

VOID showOnuSN_asyn( UCHAR slotno, UCHAR pon, UCHAR onu )
{
	/*CT_RMan_OnuSN_t sn;
	
	VOS_MemZero( &sn, sizeof(CT_RMan_OnuSN_t) );*/
	if( CT_RMan_OnuSN_get( slotno, pon, onu, call_flag_asyn_callback, (ULONG)prnOnuSN, 0 ) == VOS_OK )
	{
		sys_console_printf("ok\r\n");
	}
	else
		sys_console_printf("err\r\n");
}

#endif /*__CT_EXTOAM_SUPPORT*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#if 0
DEFUN  (
	ctc_onu_reset,
	ctc_onu_reset_cmd,
	"reset",
	/*"Reset operations\n"*/
	"Reset the onu device\n"
	)
{
	LONG lRet = VOS_OK;	
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	ULONG ulSlot, ulPort, ulOnu;

	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == VOS_ERROR)
	{
		CTC_STACK_CLI_ERROR_PRINT;
       	return CMD_WARNING;
	}

	/*	modified by chenfj 2007-8-7 	
	onu_id = ulOnu - 1;
	*/
	onu_id = GetLlidByOnuIdx( olt_id, (short int ) ( ulOnu-1));
	if( onu_id  == INVALID_LLID ) 
		{
		vty_out(vty," onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnu );
		return( CMD_WARNING );
		}

	/*lRet = GetOnuOperStatus( phyPonId, userOnuId);
	if ( CLI_EPON_ONUDOWN == lRet)
	{
		vty_out( vty, "  %% slot/port/onu %d/%d/%d is off-line.\r\n",slotId, port, onuId) ;
		return CMD_WARNING;	
	}*/		/* 即使ONU不在线，仍然尝试复位 */
	
	lRet = CTC_STACK_reset_onu( olt_id, onu_id );
		
	if( lRet != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}	
		
	return CMD_SUCCESS;
}
#endif

/*----------------------------------------------------------------------------*/

typedef struct{
	UCHAR PortBranch;
	USHORT PortLeaf;
	UCHAR PortVarWidth;
	ULONG Portlist;
	UCHAR LimitBranch;
	USHORT LimitLeaf;
	UCHAR LimitVarWidth;
	USHORT LimitNum;
}__attribute__ ((packed))ctcOamOnuPortMacLimit_t;

typedef struct{
	UCHAR Branch;
	USHORT Leaf;
	UCHAR VarWidth;
	ULONG AgingTime;
}__attribute__ ((packed))ctcOamOnuMacAging_t;

typedef struct{
	UCHAR Branch;
	USHORT Leaf;
	UCHAR VarWidth;
	USHORT Action;
	UCHAR ONUID[6];
	ULONG ExtValue;
	UCHAR OnuIdx;
}__attribute__ ((packed))ctcOamOnuTxPowerControl_t;

typedef struct {
	UCHAR subCode;
	UCHAR result;
	UCHAR slotno;
	UCHAR portno;
	ctcOamOnuTxPowerControl_t ctrlData;
} __attribute__ ((packed)) ctc_onu_tx_power_msg_t;

LONG onu_tx_power_control_msg_send ( ULONG dstSlotNo, ULONG msgCode, ctc_onu_tx_power_msg_t *pSendMsg )
{
	unsigned int ulLen;
	SYS_MSG_S *pMsg;
	ctc_onu_tx_power_msg_t *pTmp;

	if( dstSlotNo == SYS_LOCAL_MODULE_SLOTNO )
	{
		VOS_ASSERT(0);
		return  VOS_ERROR;
	}

	ulLen = sizeof(ctc_onu_tx_power_msg_t) + sizeof(SYS_MSG_S);
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_LOOPBACK);

	if( NULL == pMsg )
	{
		VOS_ASSERT(0);
		return  VOS_ERROR;
	}
	VOS_MemZero((CHAR *)pMsg, ulLen );
	pTmp = (ctc_onu_tx_power_msg_t *)(pMsg + 1);

	SYS_MSG_SRC_ID( pMsg ) = MODULE_ONU;
	SYS_MSG_DST_ID( pMsg ) = MODULE_ONU;
	SYS_MSG_SRC_SLOT( pMsg ) = SYS_LOCAL_MODULE_SLOTNO;
	SYS_MSG_DST_SLOT( pMsg ) = dstSlotNo;
	SYS_MSG_MSG_TYPE( pMsg ) = MSG_NOTIFY;
	SYS_MSG_BODY_STYLE( pMsg ) = MSG_BODY_INTEGRATIVE;
	SYS_MSG_BODY_POINTER( pMsg ) = pTmp;
	SYS_MSG_FRAME_LEN( pMsg ) = sizeof(ctc_onu_tx_power_msg_t);
	SYS_MSG_MSG_CODE(pMsg) = msgCode;

	VOS_MemCpy((void *)pTmp, (void *)pSendMsg, sizeof(ctc_onu_tx_power_msg_t));

       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_CTC, dstSlotNo, RPU_TID_CDP_CTC, CDP_MSG_TM_ASYNC,
                 (VOID *)pMsg, ulLen, MODULE_ONU ) )
	{
		VOS_ASSERT(0); 
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}

	return VOS_OK;

}

typedef struct{
	UCHAR DA[6];
	UCHAR SA[6];
	USHORT Type;
	UCHAR SubType;
	USHORT Flags;
	UCHAR Code;
	UCHAR OUI[3];
	UCHAR ExtOpcode;
}__attribute__ ((packed))ctcOamPduHeader_t;

typedef struct{
	ctcOamPduHeader_t pduHead;
	ctcOamOnuPortMacLimit_t ctrlData;
}__attribute__ ((packed))ctcOamOnuPortMacLimitPdu_t;

typedef struct{
	ctcOamPduHeader_t pduHead;
	ctcOamOnuMacAging_t ctrlData;
}__attribute__ ((packed))ctcOamOnuAgingTimePdu_t;

typedef struct{
	ctcOamPduHeader_t pduHead;
	ctcOamOnuTxPowerControl_t ctrlData;
}__attribute__ ((packed))ctcOamOnuTxPowerControlPdu_t;


LONG CTC_oamPduHeaderCreate( SHORT PonPortIdx, ctcOamPduHeader_t * pPduHeader )
{
	if( pPduHeader == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	CHECK_PON_RANGE;

	pPduHeader->DA[0] = 0x01;
	pPduHeader->DA[1] = 0x80;
	pPduHeader->DA[2] = 0xc2;
	pPduHeader->DA[3] = 0x00;
	pPduHeader->DA[4] = 0x00;
	pPduHeader->DA[5] = 0x02;
	VOS_MemCpy( pPduHeader->SA, (UCHAR*)GetPonChipMgmtPathMac(PonPortIdx), 6 );
	
	pPduHeader->Type = 0x8809;
	pPduHeader->SubType = 0x03;
	pPduHeader->Flags = 0x0050;
	pPduHeader->Code = 0xfe;
	
	pPduHeader->OUI[0] = 0x11;
	pPduHeader->OUI[1] = 0x11;
	pPduHeader->OUI[2] = 0x11;
	pPduHeader->ExtOpcode = 3;

	return VOS_OK;
}

static LONG CTC_onuPortMacLimitSet( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, ULONG portNo, ULONG num )
{
	SHORT PonPortIdx, OnuIdx;
	ctcOamOnuPortMacLimitPdu_t  oamPdu;
	SHORT pduLen = sizeof(ctcOamOnuPortMacLimitPdu_t);
	SHORT llid = PON_LLID_NOT_AVAILABLE;

	if( (onuIdx == 0) || (portNo > 24) )
		return VOS_ERROR;
		
	PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
	OnuIdx = onuIdx - 1;
	CHECK_ONU_RANGE;
	
	VOS_MemZero( &oamPdu, pduLen );
	CTC_oamPduHeaderCreate( PonPortIdx, &oamPdu.pduHead );
	oamPdu.ctrlData.PortBranch = 0xC7;
	oamPdu.ctrlData.PortLeaf = 0x0E;
	oamPdu.ctrlData.PortVarWidth = 4;
	if( portNo == 0 )
		oamPdu.ctrlData.Portlist = 0x0100ffff;
	else
		oamPdu.ctrlData.Portlist = ((1 << (portNo-1)) | 0x01000000);

	oamPdu.ctrlData.LimitBranch = 0xC7;
	oamPdu.ctrlData.LimitLeaf = 0x1C;
	oamPdu.ctrlData.LimitVarWidth = 2;
	oamPdu.ctrlData.LimitNum = num;
		

	llid = GetLlidByOnuIdx( PonPortIdx,OnuIdx );
	if( llid == INVALID_LLID )
	{
		return VOS_ERROR;
	}

	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
	{
		sys_console_printf("   onu %d/%d/%d:send MAC limit OAM frame-len=%d\r\n", brdIdx, portIdx, onuIdx, pduLen);
		pktDataPrintf( (UCHAR*)&oamPdu, pduLen );
	}

#if 0
	if( PAS_send_frame( PonPortIdx, pduLen, PON_PORT_SYSTEM, llid, 0, (UCHAR*)&oamPdu ) == PAS_EXIT_OK )
		return VOS_OK;
#else
	if( OLT_SendFrame2CNI( PonPortIdx, llid, (void*)&oamPdu, (int)pduLen ) == 0 )
		return VOS_OK;
#endif    

	return VOS_ERROR;
}

static LONG CTC_onuMacAgingTimeSet( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, ULONG aging )
{
	SHORT PonPortIdx, OnuIdx;
	ctcOamOnuAgingTimePdu_t  oamPdu;
	SHORT pduLen = sizeof(ctcOamOnuAgingTimePdu_t);
	SHORT llid = PON_LLID_NOT_AVAILABLE;

	PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
	OnuIdx = onuIdx - 1;
	CHECK_ONU_RANGE;
	
	VOS_MemZero( &oamPdu, pduLen );
	CTC_oamPduHeaderCreate( PonPortIdx, &oamPdu.pduHead );
	oamPdu.ctrlData.Branch = 0xC7;
	oamPdu.ctrlData.Leaf = 0x0E;
	oamPdu.ctrlData.VarWidth = 4;
	oamPdu.ctrlData.AgingTime = aging;

	llid = GetLlidByOnuIdx( PonPortIdx,OnuIdx );
	if( llid == INVALID_LLID )
	{
		return VOS_ERROR;
	}

	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
	{
		sys_console_printf("   pon %d/%d:send MAC aging time OAM frame-len=%d\r\n", brdIdx, portIdx, pduLen);
		pktDataPrintf( (UCHAR*)&oamPdu, pduLen );
	}

#if 0
	if( PAS_send_frame( PonPortIdx, pduLen, PON_PORT_SYSTEM, llid, 0, (UCHAR*)&oamPdu ) == PAS_EXIT_OK )
		return VOS_OK;
#else
	if( OLT_SendFrame2CNI( PonPortIdx, llid, (void*)&oamPdu, (int)pduLen ) == 0 )
		return VOS_OK;
#endif    

	return VOS_ERROR;
}

/* 临时用于CNC测试 */
static LONG CTC_onuAllMacAgingTimeSet( ULONG brdIdx, ULONG portIdx, ULONG aging )
{
	ctcOamOnuAgingTimePdu_t  oamPdu;
	SHORT pduLen = sizeof(ctcOamOnuAgingTimePdu_t);
	SHORT llid = PON_LLID_NOT_AVAILABLE;
	SHORT PonPortIdx, OnuIdx;

	PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
	CHECK_PON_RANGE;
	
	VOS_MemZero( &oamPdu, pduLen );
	CTC_oamPduHeaderCreate( PonPortIdx, &oamPdu.pduHead );
	oamPdu.ctrlData.Branch = 0xC7;
	oamPdu.ctrlData.Leaf = 0x0E;
	oamPdu.ctrlData.VarWidth = 4;
	oamPdu.ctrlData.AgingTime = aging;

	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
	{
		llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
		if(llid == INVALID_LLID)
			continue;

#if 0
		PAS_send_frame( PonPortIdx, pduLen, PON_PORT_SYSTEM, llid, 0, (UCHAR*)&oamPdu );
#else
	    OLT_SendFrame2CNI( PonPortIdx, llid, (void*)&oamPdu, (int)pduLen );
#endif    

		if(EVENT_OAM_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("   onu %d/%d/%d:send MAC aging time OAM\r\n", brdIdx, portIdx, OnuIdx+1 );
			pktDataPrintf( (UCHAR*)&oamPdu, pduLen );
		}
	}
	return VOS_OK;
}

static LONG CTC_onuOpticalTxPowerControl( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, USHORT action, UCHAR onuID[6], ULONG opticalTransmitterId )
{
	SHORT PonPortIdx, OnuIdx;
	ctcOamOnuTxPowerControlPdu_t  oamPdu;
	SHORT pduLen = sizeof(ctcOamOnuTxPowerControlPdu_t) - 1;	/*  */
	SHORT llid = PON_LLID_NOT_AVAILABLE;
	SHORT broadcast_llid = PON_EVERY_LLID;

	PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
	CHECK_PON_RANGE;
	OnuIdx = onuIdx - 1;
	
	
	VOS_MemZero( &oamPdu, pduLen );
	CTC_oamPduHeaderCreate( PonPortIdx, &oamPdu.pduHead );
	oamPdu.ctrlData.Branch = 0xC7;
	oamPdu.ctrlData.Leaf = 0xA1;
	oamPdu.ctrlData.VarWidth = 12;
	oamPdu.ctrlData.Action = action;
	VOS_MemCpy( oamPdu.ctrlData.ONUID, onuID, 6 );
	oamPdu.ctrlData.ExtValue = opticalTransmitterId;

	if( OnuIdx == -1 )	/* 广播 */
	{
	}
	else
	{
		llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
		if(llid == INVALID_LLID)
		{
			llid = PON_LLID_NOT_AVAILABLE;	/* 广播 */
		}
		else
		{
			broadcast_llid = 0;		/* 单播 */
		}
	}
	if(EVENT_OAM_DEBUG == V2R1_ENABLE)
	{
		sys_console_printf("\r\n pon %d/%d:Broadcast OAM frame-len=%d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), pduLen);
		pktDataPrintf( (UCHAR*)&oamPdu, pduLen );
	}

#if 0
	if( PAS_send_frame( PonPortIdx, pduLen, PON_PORT_SYSTEM, llid, broadcast_llid, (UCHAR*)&oamPdu ) == PAS_EXIT_OK )
		return VOS_OK;
#else
	if( OLT_SendFrame2CNI( PonPortIdx, llid, (void*)&oamPdu, (int)pduLen ) == 0 )
		return VOS_OK;
#endif    

	return VOS_ERROR;
}

/* 临时用于CNC测试 */
static LONG CTC_onuAllOpticalTxPowerControl( ULONG brdIdx, ULONG portIdx, USHORT action )
{
	ctcOamOnuTxPowerControlPdu_t  oamPdu;
	SHORT pduLen = sizeof(ctcOamOnuTxPowerControlPdu_t);
	SHORT llid = PON_LLID_NOT_AVAILABLE;
	SHORT broadcast_llid = PON_EVERY_LLID;
	SHORT PonPortIdx, OnuIdx;
	int len;

	PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
	CHECK_PON_RANGE;
	
	VOS_MemZero( &oamPdu, pduLen );
	CTC_oamPduHeaderCreate( PonPortIdx, &oamPdu.pduHead );
	oamPdu.ctrlData.Branch = 0xC7;
	oamPdu.ctrlData.Leaf = 0xA1;
	oamPdu.ctrlData.VarWidth = 12;
	oamPdu.ctrlData.Action = action;
	oamPdu.ctrlData.ExtValue = 0;

	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++ )
	{
		VOS_MemZero( oamPdu.ctrlData.ONUID, 6 );
		if( GetOnuMacAddr( PonPortIdx, OnuIdx, oamPdu.ctrlData.ONUID, &len) == VOS_ERROR )
    			continue;
		if( MAC_ADDR_IS_ZERO(oamPdu.ctrlData.ONUID) || MAC_ADDR_IS_BROADCAST(oamPdu.ctrlData.ONUID) )
			continue;
#if 0
		PAS_send_frame( PonPortIdx, pduLen, PON_PORT_SYSTEM, llid, broadcast_llid, (UCHAR*)&oamPdu );
#else
	    OLT_SendFrame2CNI( PonPortIdx, llid, (void*)&oamPdu, (int)pduLen );
#endif    

		if(EVENT_OAM_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("   onu %d/%d/%d:Broadcast optical tx power control OAM\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1 );
			pktDataPrintf( (UCHAR*)&oamPdu, pduLen );
		}
	}
	return VOS_OK;
}

LONG mn_onuPortMacLimitSet( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, ULONG onuEthIdx, ULONG num )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuPortMacLimitSet( brdIdx, portIdx, onuIdx, onuEthIdx, num );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ONU_MAC_AGING;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.OnuIdx = onuIdx;
		sndMsg.ctrlData.ONUID[0] = onuEthIdx;
		sndMsg.ctrlData.ExtValue = num;
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AGING: send MAC aging time to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_onuMacAgingTimeSet( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, ULONG aging )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuMacAgingTimeSet( brdIdx, portIdx, onuIdx, aging );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ONU_MAC_AGING;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.OnuIdx = onuIdx;
		sndMsg.ctrlData.ExtValue = aging;
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AGING: send MAC aging time to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_onuAllMacAgingTimeSet( ULONG brdIdx, ULONG portIdx, ULONG aging )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuAllMacAgingTimeSet( brdIdx, portIdx, aging );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ALL_MAC_AGING;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.ExtValue = aging;
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-AGING: broadcast MAC aging time to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_onuOpticalTxPowerControl( ULONG brdIdx, ULONG portIdx, ULONG onuIdx, USHORT action, UCHAR onuID[6], ULONG opticalTransmitterId )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuOpticalTxPowerControl( brdIdx, portIdx, onuIdx, action, onuID, opticalTransmitterId );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ONU_TX_POWER_CTRL;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.Action = action;
		sndMsg.ctrlData.OnuIdx = onuIdx;
		sndMsg.ctrlData.ExtValue = opticalTransmitterId;
		VOS_MemCpy( sndMsg.ctrlData.ONUID, onuID, 6 );
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-TXPOWER: broadcast Optical tx power control to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_onuAllOpticalTxPowerDisable( ULONG brdIdx, ULONG portIdx )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuAllOpticalTxPowerControl( brdIdx, portIdx, 65535 );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ALL_TX_POWER_DISABLE;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.Action = 65535;
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-TXPOWER: broadcast Optical tx power disable to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

LONG mn_onuAllOpticalTxPowerEnable( ULONG brdIdx, ULONG portIdx )
{
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		return CTC_onuAllOpticalTxPowerControl( brdIdx, portIdx, 0 );
	}
	else
	{
		ctc_onu_tx_power_msg_t sndMsg;
		VOS_MemZero( &sndMsg, sizeof(ctc_onu_tx_power_msg_t) );
		sndMsg.subCode = CTC_MSG_SUBCODE_ALL_TX_POWER_ENABLE;
		sndMsg.slotno = brdIdx;
		sndMsg.portno = portIdx;
		sndMsg.ctrlData.Action = 0;
		
		onu_tx_power_control_msg_send( brdIdx, CTC_MSG_CODE_ONU_TX_POWER, &sndMsg );

		if( onuTxPowerDebugSwitch & 1 )
		{
			sys_console_printf(" ONU-TXPOWER: broadcast Optical tx power enable to pon%d/%d\r\n", brdIdx, portIdx );
		}
	}
	return VOS_OK;
}

VOID onu_tx_power_control_process( ctc_onu_tx_power_msg_t *pCtrlMsg )
{
	if( pCtrlMsg == NULL )
	{
		VOS_ASSERT(0);
		return;
	}

	if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ONU_TX_POWER_CTRL )
	{
		mn_onuOpticalTxPowerControl( pCtrlMsg->slotno, pCtrlMsg->portno, pCtrlMsg->ctrlData.OnuIdx, 
			pCtrlMsg->ctrlData.Action, pCtrlMsg->ctrlData.ONUID, pCtrlMsg->ctrlData.ExtValue );
	}
	else if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ALL_TX_POWER_DISABLE )
	{
		mn_onuAllOpticalTxPowerDisable( pCtrlMsg->slotno, pCtrlMsg->portno );
	}
	else if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ALL_TX_POWER_ENABLE )
	{
		mn_onuAllOpticalTxPowerEnable( pCtrlMsg->slotno, pCtrlMsg->portno );
	}
	else if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ONU_MAC_AGING )
	{
		mn_onuMacAgingTimeSet( pCtrlMsg->slotno, pCtrlMsg->portno, pCtrlMsg->ctrlData.OnuIdx, pCtrlMsg->ctrlData.ExtValue );
	}
	else if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ALL_MAC_AGING )
	{
		mn_onuAllMacAgingTimeSet( pCtrlMsg->slotno, pCtrlMsg->portno, pCtrlMsg->ctrlData.ExtValue );
	}
	else if( pCtrlMsg->subCode == CTC_MSG_SUBCODE_ONU_MAC_LIMIT )
	{
		mn_onuPortMacLimitSet( pCtrlMsg->slotno, pCtrlMsg->portno, pCtrlMsg->ctrlData.OnuIdx, pCtrlMsg->ctrlData.ONUID[0], pCtrlMsg->ctrlData.ExtValue );
	}
}

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
DEFUN(
	config_onu_mac_limit,
	config_onu_mac_limit_cmd,
	"ctc onu-mac-limit [all|<1-24>] <0-8192>",
	CTC_STR
	"config onu mac limit\n"
	"all port\n"
	"input port number\n"
	"input limit value\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuIdx = 0;
	ULONG ethIdx;

	if( PON_GetSlotPortOnu( (ULONG)(vty->index), &brdIdx, &portIdx, &onuIdx) == VOS_ERROR )
    		return CMD_WARNING;

	if( VOS_StrCmp(argv[0], "all") == 0 )
		ethIdx = 0;
	else
		ethIdx = VOS_AtoL(argv[0]);
	mn_onuPortMacLimitSet( brdIdx, portIdx, onuIdx, ethIdx, VOS_AtoL(argv[1]) );
	
	return CMD_SUCCESS;
}

DEFUN(
	config_onu_mac_aging,
	config_onu_mac_aging_cmd,
	"ctc onu-mac-aging [0|<10-1000000>]",
	CTC_STR
	"config onu mac aging time\n"
	"input aging time value\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuIdx = 0; 

	if( PON_GetSlotPortOnu( (ULONG)(vty->index), &brdIdx, &portIdx, &onuIdx) == VOS_ERROR )
    		return CMD_WARNING;

	mn_onuMacAgingTimeSet( brdIdx, portIdx, onuIdx, VOS_AtoL(argv[0]) );
	
	return CMD_SUCCESS;
}

DEFUN(
	config_all_onu_mac_aging,
	config_all_onu_mac_aging_cmd,
	"ctc onu-mac-aging [0|<10-1000000>]",
	CTC_STR
	"config onu mac aging time\n"
	"input aging time value\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	ULONG onuIdx = 0;
	INT16 PonPortIdx = 0;

	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx , &onuIdx, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;    	

	mn_onuAllMacAgingTimeSet( brdIdx, portIdx, VOS_AtoL(argv[0]) );
	
	return CMD_SUCCESS;
}

DEFUN(
	config_onu_tx_power,
	config_onu_tx_power_cmd,
	"ctc onu-tx-power-control <0-65535>",
	CTC_STR
	"shut onu optical tx-power suply\n"
	"0:re-enable TX power supply;1-65534:duration shutdown time,unit:second;65535:permanently shutdown\n")
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;	
	ULONG onuIdx = 0; 
	INT16 PonPortIdx = 0;
	unsigned char ONUID[8];
	int len;

	if( PON_GetSlotPortOnu( (ULONG)(vty->index), &brdIdx, &portIdx, &onuIdx) == VOS_ERROR )
    		return CMD_WARNING;
	PonPortIdx = GetPonPortIdxBySlot( (short int)brdIdx, (short int)portIdx );
	if (PonPortIdx == VOS_ERROR)
	{
		return CMD_WARNING;
	}

	VOS_MemZero( ONUID, 6 );
	if( GetOnuMacAddr( PonPortIdx, onuIdx-1, ONUID, &len) == VOS_ERROR )
    		return CMD_WARNING;
	if( MAC_ADDR_IS_ZERO(ONUID) || MAC_ADDR_IS_BROADCAST(ONUID) )
    		return CMD_WARNING;

	mn_onuOpticalTxPowerControl( brdIdx, portIdx, 0, VOS_AtoL(argv[0]), ONUID, 0 );
	
	return CMD_SUCCESS;
}

DEFUN(
	disable_all_onu_tx_power,
	disable_all_onu_tx_power_cmd,
	"ctc onu-tx-power-control",
	CTC_STR
	"shut onu optical tx-power suply\n" )
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	ULONG onuIdx = 0;
	INT16 PonPortIdx = 0;

	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx , &onuIdx, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;    	

	mn_onuAllOpticalTxPowerDisable( brdIdx, portIdx );
	
	return CMD_SUCCESS;
}

DEFUN(
	enable_all_onu_tx_power,
	enable_all_onu_tx_power_cmd,
	"undo ctc onu-tx-power-control",
	"Delete configuration\n"
	CTC_STR
	"re-enable onu optical tx-power suply\n" )
{
	ULONG brdIdx = 0;
	ULONG portIdx = 0;
	ULONG onuIdx = 0;
	INT16 PonPortIdx = 0;

	if( parse_pon_command_parameter( vty, &brdIdx, &portIdx , &onuIdx, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;    	

	mn_onuAllOpticalTxPowerEnable( brdIdx, portIdx );
	
	return CMD_SUCCESS;
}
#endif

DEFUN(
	show_onu_serial_number,
	show_onu_serial_number_cmd,
	"show onu serial-number",
	SHOW_STR
	ONU_STR
	"Display onu serial number\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
	/*short int	return_result;
	CTC_STACK_onu_serial_number_t	onu_serial_number;*/
	unsigned char tmp_str[20];
	ONUTable_S *pOnuEntry;

	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
#if 0
	return_result = CTC_STACK_get_onu_serial_number(PonPortIdx, onu_id, &onu_serial_number);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		unsigned char tmp_str[20]={0};

		VOS_StrnCpy( tmp_str, onu_serial_number.vendor_id, 4 );	
		vty_out( vty, "  Vendor id: %s\r\n", tmp_str );
		vty_out( vty, "  ONU Model: %x\r\n", onu_serial_number.onu_model );
		vty_out( vty, "  ONU ID: %s\r\n", macAddress_To_Strings(onu_serial_number.onu_id) );
		VOS_StrnCpy( tmp_str, onu_serial_number.hardware_version, 8 );	
		vty_out( vty, "  Hardware version: %s\r\n", tmp_str );
		VOS_StrnCpy( tmp_str, onu_serial_number.software_version, 16 );	
		vty_out( vty, "  Software version: %s\r\n", tmp_str );
		
		return CMD_SUCCESS;
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;
#endif
	pOnuEntry = &OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx];

	VOS_MemZero( tmp_str, sizeof(tmp_str) );
	VOS_MemCpy( tmp_str, pOnuEntry->device_vendor_id, 4 );
	vty_out( vty, "  Vendor id: %s\r\n", tmp_str );
	vty_out( vty, "  ONU Model: %x\r\n", pOnuEntry->onu_model );
	vty_out( vty, "  ONU ID: %s\r\n", macAddress_To_Strings(pOnuEntry->DeviceInfo.MacAddr) );

	VOS_MemZero( tmp_str, sizeof(tmp_str) );
	VOS_StrnCpy( tmp_str, pOnuEntry->hardware_version, 8 );	
	vty_out( vty, "  Hardware version: %s\r\n", tmp_str );
	VOS_MemZero( tmp_str, sizeof(tmp_str) );
	VOS_StrnCpy( tmp_str, pOnuEntry->software_version, 16 );	
	vty_out( vty, "  Software version: %s\r\n", tmp_str );

	return CMD_SUCCESS;
}

DEFUN(
	show_firmware_version,
	show_firmware_version_cmd,
	"show ctc firmware-version",
	SHOW_STR
	CTC_STR
	"Display firmware version\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
	/*short int		return_result;
	unsigned short firmware_version;*/
	
	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
#if 0	
	return_result = CTC_STACK_get_firmware_version(olt_id, onu_id, &firmware_version);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "Firmware version: %x\r\n", firmware_version );
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;
#endif
	vty_out( vty, "Firmware version: %x\r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].firmware_version );

	return CMD_SUCCESS;
}

DEFUN(
	show_chipset_id,
	show_chipset_id_cmd,
	"show ctc chipset-id",
	SHOW_STR
	CTC_STR
	"Display chipset id\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
	/*short int		return_result;
	CTC_STACK_chipset_id_t	chipset_id;*/
	ONUTable_S *pOnuEntry;
	char tmp_str[CTC_VENDOR_ID_LENGTH+2];
	char tmp_str1[5] = {0,0,0,0,0};
	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
#if 0
	return_result = CTC_STACK_get_chipset_id(olt_id, onu_id, &chipset_id);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		unsigned char tmp_str[16]={0};

		VOS_StrnCpy(tmp_str, chipset_id.vendor_id, 2);
		vty_out( vty, "  Vendor ID: %s\r\n", tmp_str );
		
		vty_out( vty, "  Chip Model: %d\r\n", chipset_id.chip_model );
		vty_out( vty, "  Revision: %x\r\n", chipset_id.revision );  
		vty_out( vty, "  Date: (YY)%02d (MM)%d (DD)%d\r\n",chipset_id.date[0], chipset_id.date[1], chipset_id.date[2] );
		
	}else
		CTC_STACK_CLI_ERROR_PRINT;
#endif
	pOnuEntry = &OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx];
	VOS_MemCpy( tmp_str, ( void *)pOnuEntry->chip_vendor_id, CTC_VENDOR_ID_LENGTH );
	tmp_str[CTC_VENDOR_ID_LENGTH] = 0;
	vty_out( vty, "  Vendor ID: %s\r\n", tmp_str );
	vty_out( vty, "  Onu Model: 0x%08x\r\n", pOnuEntry->onu_model);
	VOS_MemCpy(tmp_str1, pOnuEntry->device_vendor_id, 4);
	vty_out( vty, "  Device vendor: %s\r\n", tmp_str1);
	vty_out( vty, "  Chip Model: %d\r\n", pOnuEntry->chip_model );
	vty_out( vty, "  Revision: %x\r\n", pOnuEntry->revision );  
	vty_out( vty, "  Date: %02d-%d -%d\r\n",pOnuEntry->date[0], pOnuEntry->date[1], pOnuEntry->date[2] );
	if(OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].onu_ctc_version == 0x30)
		vty_out( vty, "  Device extendedModel: %s\r\n", pOnuEntry->extendedModel);
		
	return CMD_SUCCESS;
}


DEFUN(
	show_onu_capabilities,
	show_onu_capabilities_cmd,
	"show ctc onu-capabilities",
	SHOW_STR
	CTC_STR
	"Display onu capabilities\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
	char tmp_str[20] = {0};
	ONUTable_S *pOnuEntry;

	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
#if 0
	short int		return_result;
	CTC_STACK_onu_capabilities_t	onu_capabilities;

	VOS_MemZero( &onu_capabilities, sizeof(CTC_STACK_onu_capabilities_t) );
	return_result = CTC_STACK_get_onu_capabilities(PonPortIdx, onu_id, &onu_capabilities);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		if( onu_capabilities.service_supported & 0x01 )
			VOS_StrCpy( tmp_str, "Gbit " );
		if( onu_capabilities.service_supported & 0x02 )
			VOS_StrCat( tmp_str, "FE " ); 
		if( onu_capabilities.service_supported & 0x01 )
			VOS_StrCpy( tmp_str, "VoIP " );
		if( onu_capabilities.service_supported & 0x01 )
			VOS_StrCpy( tmp_str, "TDM " );
	
		vty_out( vty, "  Service Supported: %s\r\n", tmp_str );
		
		vty_out( vty, "  GE Number: %d\r\n", onu_capabilities.ge_ethernet_ports_number );  
		vty_out( vty, "  GE Bitmap: %s\r\n", bitmap_To_Strings((UCHAR*)&onu_capabilities.ge_ports_bitmap, 8) );

		vty_out( vty, "  FE Number: %d\r\n", onu_capabilities.fe_ethernet_ports_number );  
		vty_out( vty, "  FE Bitmap: %s\r\n",  bitmap_To_Strings((UCHAR*)&onu_capabilities.fe_ports_bitmap, 8) );

		vty_out( vty, "  POTS Number: %d\r\n", onu_capabilities.pots_ports_number );  
		vty_out( vty, "  E1 Number: %d\r\n", onu_capabilities.e1_ports_number );  

		vty_out( vty, "  Number of upstream queues: %d\r\n", onu_capabilities.upstream_queues_number );  
		vty_out( vty, "  Max queues per port upstream: %d\r\n", onu_capabilities.max_upstream_queues_per_port );  

		vty_out( vty, "  Number of downstream queues: %d\r\n", onu_capabilities.downstream_queues_number );  
		vty_out( vty, "  Max queues per port downstream: %d\r\n", onu_capabilities.max_downstream_queues_per_port );  
		vty_out( vty, "  Battery backup: %s\r\n", (onu_capabilities.battery_backup==1) ? "Supported" : "Unsupported" );  

		return CMD_SUCCESS;
	}
	else
	{
		CTC_STACK_CLI_ERROR_PRINT;
	}
	return CMD_WARNING;
#else
	pOnuEntry = &OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx];
		
	if( SUPPORTING == pOnuEntry->GE_supporting )
		VOS_StrCpy( tmp_str, "Gbit " );
	if( SUPPORTING == pOnuEntry->FE_supporting )
		VOS_StrCat( tmp_str, "FE " ); 
	if( SUPPORTING == pOnuEntry->VoIP_supporting )
		VOS_StrCat( tmp_str, "VoIP " );
	if( SUPPORTING == pOnuEntry->TDM_CES_supporting )
		VOS_StrCat( tmp_str, "TDM " );

	vty_out( vty, "  Service Supported: %s\r\n", tmp_str );
	if( pOnuEntry->GE_Ethernet_ports_number )
		vty_out( vty, "  GE Number: %d\r\n", pOnuEntry->GE_Ethernet_ports_number );  
	if( pOnuEntry->FE_Ethernet_ports_number )
		vty_out( vty, "  FE Number: %d\r\n", pOnuEntry->FE_Ethernet_ports_number );  
	if( pOnuEntry->POTS_ports_number )
		vty_out( vty, "  POTS Number: %d\r\n", pOnuEntry->POTS_ports_number );  
	if( pOnuEntry->E1_ports_number )
		vty_out( vty, "  E1 Number: %d\r\n", pOnuEntry->E1_ports_number );  

	if( pOnuEntry->ADSL_ports_number )
		vty_out( vty, "  ADSL Number: %d\r\n", pOnuEntry->ADSL_ports_number );  
	if( pOnuEntry->VDSL_ports_number )
		vty_out( vty, "  VDSL Number: %d\r\n", pOnuEntry->VDSL_ports_number );  
	if( pOnuEntry->WLAN_ports_number )
		vty_out( vty, "  WLAN Number: %d\r\n", pOnuEntry->WLAN_ports_number );  
	if( pOnuEntry->USB_ports_number )
		vty_out( vty, "  USB Number: %d\r\n", pOnuEntry->USB_ports_number );  
	if( pOnuEntry->CATV_ports_number )
		vty_out( vty, "  CATV Number: %d\r\n", pOnuEntry->CATV_ports_number );  

	vty_out( vty, "  Battery backup: %s\r\n", (pOnuEntry->HaveBattery == 1) ? "Supported" : "Unsupported" );  

	return CMD_SUCCESS;
#endif
}

DEFUN(
	show_onu_ctc_version,
	show_onu_ctc_version_cmd,
	"show ctc version",
	SHOW_STR
	CTC_STR
	"Display ctc version\n")
{
	PON_olt_id_t	PonPortIdx,OnuIdx;
	PON_onu_id_t onu_id;
	short int		return_result;
	unsigned char ver = 0;
	
	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
#if 0
		return_result = CTC_STACK_get_onu_version( PonPortIdx, onu_id, &ver );
#else
		return_result = OnuMgt_GetCtcVersion(PonPortIdx, OnuIdx, &ver );
#endif
	}
	else
	{
		ver = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].onu_ctc_version;
		return_result = CTC_STACK_EXIT_OK;
	}
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  CTC v%d.%d (%02X)\r\n", ((ver>>4) & 0x0f), (ver & 0x0f), ver );
		return CMD_SUCCESS;
	}
	
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
}

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
DEFUN(
	show_onu_holdover,
	show_onu_holdover_cmd,
	"show ctc holdover",
	SHOW_STR
	CTC_STR
	"Display onu holdover parameter\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id, llid;
	short int		return_result;
	/*CTC_STACK_holdover_state_t para;*/
	ULONG tm;
	
	if( parse_onu_command_parameter(vty, &olt_id, &onu_id, &llid) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	/*
	VOS_MemZero( &para, sizeof(para) );
	return_result = CTC_STACK_get_holdover_state( olt_id, onu_id, &para );*/
	return_result = getOnuConfSimpleVar(olt_id, onu_id, sv_enum_onu_holdover_time, &tm);

	if ( return_result == CTC_STACK_EXIT_OK )
	{
		/*vty_out( vty, "Holdover state:%s,holdover time:%d\r\n", ((para.holdover_state == CTC_STACK_HOLDOVER_STATE_ACTIVATED) ? "active" : "deactive"), para.holdover_time );*/
	    vty_out( vty, "Holdover state:%s,holdover time:%d\r\n", ((tm) ? "active" : "deactive"), tm );
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
}
#endif

DEFUN(
	config_onu_holdover,
	config_onu_holdover_cmd,
	"ctc holdover time {<0-1000>}*1",
	CTC_STR
	"onu holdover parameter\n"
	"input holdover time,0-deactive;other-active\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id, llid;
	short int		return_result;
	CTC_STACK_holdover_state_t para;
	ULONG suffix;
	
	if(!IsProfileNodeVty(vty->index, &suffix))
	{
        if( parse_onu_command_parameter(vty, &olt_id, &onu_id, &llid) == VOS_ERROR )
        {
            CTC_STACK_CLI_ERROR_PRINT;
                return CMD_WARNING;
        }

        if(argc == 1)
        {
            para.holdover_time = VOS_AtoL( argv[0] );
            if( para.holdover_time )
                para.holdover_state = CTC_STACK_HOLDOVER_STATE_ACTIVATED;
            else
                para.holdover_state = CTC_STACK_HOLDOVER_STATE_DEACTIVATED;

            /*return_result = CTC_STACK_set_holdover_state( olt_id, onu_id, para );*/
                    return_result = OnuMgt_SetHoldOver(olt_id, onu_id, &para);

            if ( return_result == CTC_STACK_EXIT_OK )
            {
                return CMD_SUCCESS;
            }
            vty_out(vty, ONU_CMD_ERROR_STR);

        }
        else
        {
            CTC_STACK_holdover_state_t holdover;
            return_result = OnuMgt_GetHoldOver(olt_id, onu_id, &holdover);
            if(return_result)
            {
#if 0                
                vty_out(vty, "Holdover state get fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
            else
                vty_out( vty, "Holdover state:%s,holdover time:%d\r\n", ((holdover.holdover_state == CTC_STACK_HOLDOVER_STATE_ACTIVATED) ? "active" : "deactive"), holdover.holdover_time );
        }

        return CMD_WARNING;
	}
	else
	{
	    ULONG holdover = 0;
	    if(argc == 1)
	        setOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_onu_holdover_time, VOS_AtoL( argv[0] ));
	    else
	    {
	        if(getOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_onu_holdover_time, &holdover) == VOS_OK)
	            vty_out( vty, "Holdover state:%s,holdover time:%d\r\n", ((holdover) ? "active" : "deactive"), holdover );
	    }
	    return CMD_SUCCESS;
	}

	return CMD_SUCCESS;

}

#if 0
DEFUN(
	show_fec_ability,
	show_fec_ability_cmd,
	"show fec-ability",
	SHOW_STR

	"show fec ability\n")
{
	/* modified by chenfj 2007-8-8 */
	PON_olt_id_t	olt_id;
	unsigned long ulSlot, ulPort, ulOnu;
	PON_onu_id_t	onu_id;
	short int		return_result;
	CTC_STACK_standard_FEC_ability_t	fec_ability;

	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	/*
	if( parse_onu_command_parameter( vty, &olt_id, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	*/
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
		{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return CMD_WARNING;
		}
	
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == (-1) )
		{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return CMD_WARNING;
		}

	if( GetOnuOperStatus(olt_id, (ulOnu-1)) != 1 )
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnu);
    		return CMD_WARNING;
		}

	onu_id = GetLlidByOnuIdx( olt_id, (ulOnu-1));
	if( onu_id == (-1)) return( CMD_WARNING );
		
	return_result = CTC_STACK_get_fec_ability(olt_id, onu_id, &fec_ability);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  FEC Ability:" );
		if( fec_ability == STD_FEC_ABILITY_UNKNOWN )
			vty_out( vty, "Unknown\r\n" );
		else if( fec_ability == STD_FEC_ABILITY_SUPPORTED )
			vty_out( vty, "Supported\r\n" );
		else if( fec_ability == STD_FEC_ABILITY_UNSUPPORTED )
			vty_out( vty, "Unsupported\r\n" );
		else	
			vty_out( vty, "Invalid value\r\n" );

		return CMD_SUCCESS;
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;

	return CMD_WARNING;
}

DEFUN(
	show_fec_mode,
	show_fec_mode_cmd,
	"show  fec-mode",
	SHOW_STR

	"Display fec mode\n")
{
	/* modified by chenfj 2007-8-8 */
	PON_olt_id_t	olt_id;
	unsigned long ulSlot, ulPort, ulOnu;
	PON_onu_id_t	onu_id;
	short int		return_result;
	CTC_STACK_standard_FEC_mode_t	fec_mode;
	
	/*if( argc != 0 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	/*
	if( parse_onu_command_parameter( vty, &olt_id, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	*/

	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
		{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return CMD_WARNING;
		}
	
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == (-1) )
		{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return CMD_WARNING;
		}

	if( GetOnuOperStatus(olt_id, (ulOnu-1)) != 1 )
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnu);
    		return CMD_WARNING;
		}

	onu_id = GetLlidByOnuIdx( olt_id, (ulOnu-1));
	if( onu_id == (-1)) return( CMD_WARNING );
	
	return_result = CTC_STACK_get_fec_mode(olt_id, onu_id, &fec_mode);
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "  FEC Mode:" );
		if( fec_mode == STD_FEC_MODE_UNKNOWN )
			vty_out( vty, "Unknown\r\n" );
		else if( fec_mode == STD_FEC_MODE_DISABLED )
			vty_out( vty, "Disable\r\n" );
		else if( fec_mode == STD_FEC_MODE_ENABLED)
			vty_out( vty, "Enable\r\n" );
		else
			vty_out( vty, "Invalid value\r\n" );

		return CMD_SUCCESS;
	}
	else
		CTC_STACK_CLI_ERROR_PRINT;

	return CMD_WARNING;
}

DEFUN(
	ctc_set_fec_mode,
	ctc_set_fec_mode_cmd,
	"fec-mode [enable|disable]",
	"Set fec mode\n"
	"fec mode disable fec\n"
	"fec mode enable fec\n")
{

	/* modified by chenfj 2007-8-8 */
	
	PON_olt_id_t	olt_id;
	unsigned long ulSlot, ulPort, ulOnu;
	/*PON_onu_id_t	llid;*/
	short int		return_result;
	CTC_STACK_standard_FEC_mode_t	fec_mode;

	/*if( argc != 1 )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}*/
	/*
	if( parse_onu_command_parameter( vty, &olt_id, &llid) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	*/
	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
		{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return CMD_WARNING;
		}
	
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == (-1) )
		{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return CMD_WARNING;
		}

	if( VOS_StrCmp(argv[0], "enable") == 0 )
		fec_mode = STD_FEC_MODE_ENABLED;
	else if( VOS_StrCmp(argv[0], "disable") == 0 )
		fec_mode = STD_FEC_MODE_DISABLED;
	else
		return( CMD_WARNING );

	if( GetOnuOperStatus(olt_id, (ulOnu-1)) != 1 )
		{
		vty_out( vty, "  %% onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnu);
		OnuMgmtTable[olt_id * MAXONUPERPON + (ulOnu-1)].FEC_Mode = fec_mode;
    		return CMD_WARNING;
		}
	
	return_result = CTC_SetLlidFecMode( olt_id,(ulOnu-1), fec_mode); 	
	/*return_result = CTC_STACK_set_fec_mode(olt_id, llid, fec_mode);*/
	
	if( return_result != CTC_STACK_EXIT_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
#endif

DEFUN(
	show_onu_ctc_mxu_mng_global,
	show_onu_ctc_mxu_mng_global_cmd,
	"show ctc mxu-mng",
	SHOW_STR
	CTC_STR
	"Display onu manage parameter\n")
{
   	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id, llid;
	short int		return_result;
	CTC_STACK_mxu_mng_global_parameter_config_t para;
	
	if( parse_onu_command_parameter(vty, &olt_id, &onu_id, &llid) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}
	return_result = OnuMgt_GetMxuMngGlobalConfig( olt_id, onu_id, &para );
	if ( return_result == VOS_OK )
	{
		vty_out( vty, "Manage Global Parameter\r\n" );
		vty_out( vty, " mng_ip:%d.%d.%d.%d\r\n", ((para.mng_ip >> 24) & 0xff), ((para.mng_ip >> 16) & 0xff), ((para.mng_ip >> 8) & 0xff), (para.mng_ip & 0xff) );
		vty_out( vty, " mng_mask:%d.%d.%d.%d\r\n", ((para.mng_mask >> 24) & 0xff), ((para.mng_mask >> 16) & 0xff), ((para.mng_mask >> 8) & 0xff), (para.mng_mask & 0xff) );
		vty_out( vty, " mng_gw:%d.%d.%d.%d\r\n", ((para.mng_gw >> 24) & 0xff), ((para.mng_gw >> 16) & 0xff), ((para.mng_gw >> 8) & 0xff), (para.mng_gw & 0xff) );
		vty_out( vty, " data_cvlan:%d\r\n", para.data_cvlan);
		vty_out( vty, " data_svlan:%d\r\n", para.data_svlan);
		vty_out( vty, " data_priority:%d\r\n", para.data_priority );
		
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
    #if 0
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	short int		return_result;
	CTC_STACK_mxu_mng_global_parameter_config_t para;
	
	if( parse_onu_command_parameter(vty, &olt_id, 0, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	VOS_MemZero( &para, sizeof(para) );
	return_result = CTC_STACK_get_mxu_mng_global_parameter_config( olt_id, onu_id, &para );
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "Manage Global Parameter\r\n" );
		vty_out( vty, " mng_ip:%d.%d.%d.%d\r\n", ((para.mng_ip >> 24) & 0xff), ((para.mng_ip >> 16) & 0xff), ((para.mng_ip >> 8) & 0xff), (para.mng_ip & 0xff) );
		vty_out( vty, " mng_mask:%d.%d.%d.%d\r\n", ((para.mng_mask >> 24) & 0xff), ((para.mng_mask >> 16) & 0xff), ((para.mng_mask >> 8) & 0xff), (para.mng_mask & 0xff) );
		vty_out( vty, " mng_gw:%d.%d.%d.%d\r\n", ((para.mng_gw >> 24) & 0xff), ((para.mng_gw >> 16) & 0xff), ((para.mng_gw >> 8) & 0xff), (para.mng_gw & 0xff) );
		vty_out( vty, " data_cvlan:%d\r\n", para.data_cvlan );
		vty_out( vty, " data_svlan:%d\r\n", para.data_svlan );
		vty_out( vty, " data_priority:%d\r\n", para.data_priority );
		
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
    #endif
}
DEFUN(
	config_onu_ctc_mxu_mng_global,
	config_onu_ctc_mxu_mng_global_cmd,
	"ctc mxu-mng <A.B.C.D/M> <A.B.C.D> <1-4094> <0-4094> <0-7>",
	CTC_STR
	"config onu manage global parameter\n"
	"Ip addr. and mask\n"
	"Gateway ip addr.\n"
	"manage data CVlan\n"
	"manage data SVlan\n"
	"manage data priority\n"
	)
{
    int ponID = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
    ULONG ulRet, suffix;
	short int		return_result;
    RPC_CTC_mxu_mng_global_parameter_config_t para;
	
	if(!IsProfileNodeVty(vty->index, &suffix))
	{
    	/*begin:支持管理IP的配置保存，mod by liuyh, 2017-5-4*/
        ulRet = PON_GetSlotPortOnu( vty->index, &ulSlot, &ulPort, &ulOnuid );
        if ( ulRet !=VOS_OK )
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }

        ponID = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
        if(ponID == (VOS_ERROR))
        {
            vty_out( vty, "  %% Parameter error\r\n");
            return CMD_WARNING;
        }
        /*end: mod by liuyh, 2017-5-4*/
        
        if ( VOS_OK != IpListToIp( argv[0], &para.mxu_mng.mng_ip, &para.mxu_mng.mng_mask ) )
    	{
    		vty_out( vty, "%% Invalid IP address.\r\n" );
    		return CMD_SUCCESS;
    	}
        
    	para.mxu_mng.mng_gw = get_long_from_ipdotstring( argv[1] );
    	para.mxu_mng.data_cvlan = VOS_AtoL( argv[2] );
    	para.mxu_mng.data_svlan = VOS_AtoL( argv[3] );
    	para.mxu_mng.data_priority = VOS_AtoL( argv[4] );
        para.needSaveOlt = TRUE;    /* 需要保存OLT配置, by liuyh 2017-5-5 */        
        
        if (OnuMgt_SetMxuMngGlobalConfig(ponID, ulOnuid-1, &para) != VOS_OK)
        {
            vty_out(vty, ONU_CMD_ERROR_STR);    	
        }
	}

    return CMD_SUCCESS;
    
#if 0
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	short int		return_result;
	CTC_STACK_mxu_mng_global_parameter_config_t para;
	
	if( parse_onu_command_parameter(vty, &olt_id, 0, &onu_id) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	if ( VOS_OK != IpListToIp( argv[0], &para.mng_ip, &para.mng_mask ) )
	{
		vty_out( vty, "%% Invalid IP address.\r\n" );
		return CMD_SUCCESS;
	}
	para.mng_gw = get_long_from_ipdotstring( argv[1] );
	para.data_cvlan = VOS_AtoL( argv[2] );
	para.data_svlan = VOS_AtoL( argv[3] );
	para.data_priority = VOS_AtoL( argv[4] );
		
	return_result = CTC_STACK_set_mxu_mng_global_parameter_config( olt_id, onu_id, para );
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
#endif
}

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
DEFUN(
	show_onu_ctc_mxu_mng_snmp,
	show_onu_ctc_mxu_mng_snmp_cmd,
	"show ctc mxu-mng-snmp",
	SHOW_STR
	CTC_STR
	"Display onu manage snmp parameter\n")
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	short int llid;
	short int		return_result;
	CTC_STACK_mxu_mng_snmp_parameter_config_t para;
	
	if( parse_onu_command_parameter(vty, &olt_id, &onu_id, &llid) == VOS_ERROR )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	VOS_MemZero( &para, sizeof(para) );
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	return_result = OnuMgt_GetMxuMngSnmpConfig( olt_id, onu_id, &para );
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		vty_out( vty, "Manage SNMP Parameter\r\n" );
		vty_out( vty, " snmp_ver:%d\r\n", para.snmp_ver );
		vty_out( vty, " trap_host_ip:%d.%d.%d.%d\r\n", ((para.trap_host_ip >> 24) & 0xff), ((para.trap_host_ip >> 16) & 0xff), ((para.trap_host_ip >> 8) & 0xff), (para.trap_host_ip & 0xff) );
		vty_out( vty, " trap_port:%d\r\n", para.trap_port );
		vty_out( vty, " snmp_port:%d\r\n", para.snmp_port );
		vty_out( vty, " security_name:%s\r\n", para.security_name );
		vty_out( vty, " community_for_read:%s\r\n", para.community_for_read );
		vty_out( vty, " community_for_write:%s\r\n", para.community_for_write );
		
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
}
DEFUN(
	config_onu_ctc_mxu_mng_snmp,
	config_onu_ctc_mxu_mng_snmp_cmd,
	"ctc mxu-mng-snmp [v1|v2c|v3] <A.B.C.D> <1-65535> <1-65535> <security> <readc> <writec>",
	CTC_STR
	"config onu snmp parameter\n"
	"snmp version v1\n"
	"snmp version v2c\n"
	"snmp version v3\n"
	"trap ip addr.\n"
	"trap port\n"
	"snmp port\n"
	"security name\n"
	"read community\n"
	"write community\n"
	)
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	short int llid;
	short int		return_result;
	CTC_STACK_mxu_mng_snmp_parameter_config_t para;
	
	if( VOS_ERROR == parse_onu_command_parameter(vty, &olt_id, &onu_id, &llid) )
	{
		CTC_STACK_CLI_ERROR_PRINT;
    		return CMD_WARNING;
	}

	VOS_MemZero( &para, sizeof(para) );
	
	if( '1' == argv[0][1] )
		para.snmp_ver = 1;
	else if( '3' == argv[0][1] )
		para.snmp_ver = 3;
	else
		para.snmp_ver = 2;

	para.trap_host_ip = get_long_from_ipdotstring( argv[1] );
	
	para.trap_port = VOS_AtoL( argv[2] );
	para.snmp_port = VOS_AtoL( argv[3] );

	if( SECURITY_NAME_SIZE <= VOS_StrLen(argv[4]) )
	{
		vty_out( vty, "Err:security name is too long\r\n" );
		return CMD_WARNING;
	}
	VOS_StrCpy( para.security_name, argv[4] );
	if( (SECURITY_NAME_SIZE <= VOS_StrLen(argv[5])) || (SECURITY_NAME_SIZE <= VOS_StrLen(argv[6])) )
	{
		vty_out( vty, "Err:community is too long\r\n" );
		return CMD_WARNING;
	}
	VOS_StrCpy( para.community_for_read, argv[5] );
	VOS_StrCpy( para.community_for_write, argv[6] );
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/	
	return_result = OnuMgt_SetMxuMngSnmpConfig( olt_id, onu_id, &para );
	if ( return_result == CTC_STACK_EXIT_OK )
	{
		return CMD_SUCCESS;
	}
	CTC_STACK_CLI_ERROR_PRINT;
	
	return CMD_WARNING;
}
#endif

#if 1/*GPON ONU command*/
DEFUN(
	show_gonu_capabilities,
	show_gonu_capabilities_cmd,
	"show gpon onu-capabilities",
	SHOW_STR
	"gpon onu operations\n"
	"Display onu capabilities\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;

	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	
    OnuEvent_ShowCapabilities(vty, PonPortIdx, OnuIdx);
	return CMD_SUCCESS;
}
DEFUN(
	show_gonu_device_information,
	show_gonu_device_information_cmd,
	"show device information",
	SHOW_STR
	"Gpon onu operations\n"
	"Display onu device information\n")
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;

	if( parse_onu_command_parameter( vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

    OnuEvent_ShowDeviceInfomation(vty, PonPortIdx, OnuIdx);
	return CMD_SUCCESS;
}

#endif
extern int CTC_onu_init();
LONG CT_RMan_ONU_Init(enum node_type  node)
{

	/*modified by liyang 2014-10-27 for question 21706*/
	if(node == ONU_CTC_NODE)
		CTC_onu_init();

	/*install_element ( ONU_CTC_NODE, &ctc_onu_reset_cmd);*/
	/*
	 * 当前CTC标准不支持
	install_element ( ONU_CTC_NODE, &config_onu_mac_limit_cmd);
	install_element ( ONU_CTC_NODE, &config_onu_mac_aging_cmd);
	install_element ( PON_PORT_NODE, &config_all_onu_mac_aging_cmd);	
	install_element ( ONU_CTC_NODE, &config_onu_tx_power_cmd);
	install_element ( PON_PORT_NODE, &disable_all_onu_tx_power_cmd);	
	install_element ( PON_PORT_NODE, &enable_all_onu_tx_power_cmd);	
	*/
	
	install_element ( node, &show_onu_serial_number_cmd);
	install_element ( node, &show_firmware_version_cmd);
	install_element ( node, &show_chipset_id_cmd);
	install_element ( node, &show_onu_capabilities_cmd);

	install_element ( node, &show_onu_ctc_version_cmd);

	/* deleted by chenfj 2007-8-8 */
#if 0  
	install_element ( ONU_CTC_NODE, &show_fec_ability_cmd);
	install_element ( ONU_CTC_NODE, &show_fec_mode_cmd);
	install_element ( ONU_CTC_NODE, &ctc_set_fec_mode_cmd);
#endif

	/*install_element ( ONU_CTC_NODE, &show_onu_holdover_cmd);*/
	install_element ( node, &config_onu_holdover_cmd);
    if ( ONU_CTC_NODE == node )
    {
    	install_element ( ONU_PROFILE_NODE, &config_onu_holdover_cmd);
    }

	/*暂时屏蔽这些设置，一时用不到*/
	install_element ( node, &show_onu_ctc_mxu_mng_global_cmd);
	install_element ( node, &config_onu_ctc_mxu_mng_global_cmd);
	/*begin: mod by liuyh, 2017-5-5*/
    if ( ONU_CTC_NODE == node )
    {
        /* 同时在通用ONU节点安装，以实现配置加载 */
        install_element ( ONU_NODE, &show_onu_ctc_mxu_mng_global_cmd);
        install_element ( ONU_NODE, &config_onu_ctc_mxu_mng_global_cmd);
    }
    /*end: mod by liuyh, 2017-5-5*/
    
    /*
	install_element ( ONU_CTC_NODE, &show_onu_ctc_mxu_mng_snmp_cmd);
	install_element ( ONU_CTC_NODE, &config_onu_ctc_mxu_mng_snmp_cmd);
	*/

	return VOS_OK;
}

void GPON_onu_init()
{    
    install_element ( ONU_GPON_NODE, &show_gonu_capabilities_cmd);
    install_element ( ONU_GPON_NODE, &show_gonu_device_information_cmd);

    /*begin: 增加配置管理IP相关命令，mod by liuyh, 2017-4-27*/
    install_element ( ONU_GPON_NODE, &show_onu_ctc_mxu_mng_global_cmd);
	install_element ( ONU_GPON_NODE, &config_onu_ctc_mxu_mng_global_cmd);
	/*end: mod by liuyh, 2017-4-27*/
}
#if 0	/* __test_ctc*/

UCHAR ct_extoam_da_mac[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };
UCHAR ct_extoam_sa_mac[6] = { 0x00, 0x0f, 0x00, 0x00, 0x11, 0x22 };

PON_STATUS CTC_STACK_get_onu_serial_number( const PON_olt_id_t	olt_id, 
			const PON_onu_id_t onu_id, CTC_STACK_onu_serial_number_t *onu_serial_number )
{
	VOS_StrCpy( onu_serial_number->vendor_id, "abc");
	onu_serial_number->onu_model = 1;
	VOS_MemCpy(onu_serial_number->onu_id, ct_extoam_sa_mac, 6);
	VOS_StrCpy( onu_serial_number->hardware_version, "V1R0" );
	VOS_StrCpy( onu_serial_number->software_version, "V1R04B101");
	return 0;
}

PON_STATUS CTC_STACK_get_firmware_version( const PON_olt_id_t	 olt_id, 
			const PON_onu_id_t  onu_id, unsigned short int *version )
{
	*version = 0x12;
	return 0;
}
PON_STATUS CTC_STACK_get_chipset_id( const PON_olt_id_t  olt_id, 
			const PON_onu_id_t  onu_id, CTC_STACK_chipset_id_t 	*chipset_id )
{
	chipset_id->vendor_id[0] = 'E'; /* PMC-SIERRA value: ASCII 'E6' */
	chipset_id->vendor_id[1] = '6';
	chipset_id->chip_model = 0x5201;
	chipset_id->revision = 0;
	chipset_id->date[0] = 07;
	chipset_id->date[1] = 03;
	chipset_id->date[2] = 15;
	return 0;
}

PON_STATUS CTC_STACK_get_onu_capabilities ( const PON_olt_id_t  olt_id, 
			const PON_onu_id_t  onu_id,  CTC_STACK_onu_capabilities_t  *onu_capabilities )
{
	onu_capabilities->service_supported = 0x02;
	onu_capabilities->ge_ethernet_ports_number = 0;
	/*onu_capabilities->ge_ports_bitmap[8];*/
	onu_capabilities->fe_ethernet_ports_number = 4;
	onu_capabilities->fe_ports_bitmap[0] = 0x0f;
	onu_capabilities->pots_ports_number = 0;
	onu_capabilities->e1_ports_number = 0;
	onu_capabilities->upstream_queues_number = 4;
	onu_capabilities->max_upstream_queues_per_port = 4;
	onu_capabilities->downstream_queues_number = 4;
	onu_capabilities->max_downstream_queues_per_port = 4;
	onu_capabilities->battery_backup = 0;
	return 0;
}

PON_STATUS CTC_STACK_get_fec_ability( const PON_olt_id_t	olt_id, 
			const PON_onu_id_t  onu_id,  CTC_STACK_standard_FEC_ability_t  *fec_ability )
{
	*fec_ability = STD_FEC_ABILITY_SUPPORTED;
	return 0;
}

static CTC_standard_FEC_mode_t __ctc_fec_mode = STD_FEC_MODE_ENABLED;
PON_STATUS CTC_STACK_get_fec_mode( 	const PON_olt_id_t	olt_id, 
			const PON_onu_id_t  onu_id,  CTC_STACK_standard_FEC_mode_t  *fec_mode )
{
	*fec_mode = __ctc_fec_mode;
	return 0;
}
PON_STATUS CTC_STACK_set_fec_mode( const PON_olt_id_t	olt_id, 
			const PON_onu_id_t  onu_id,  const CTC_STACK_standard_FEC_mode_t  fec_mode )
{
	__ctc_fec_mode = fec_mode;
	return 0;
}

#endif /*__test_ctc*/

/* modified by xieshl 20120607, 下面的2个函数需要封装，先暂时放这里 */
#ifndef CTC_OBSOLETE		/* removed by xieshl 20120607 */
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

    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_GetEthPortAnLocalTecAbility(olt_id, onu_id, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	#else
	if( CTC_STACK_get_auto_negotiation_local_technology_ability(olt_id, llid, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	#endif
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
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_GetEthPortAnAdvertisedTecAbility(olt_id, onu_id, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	#else
	if( CTC_STACK_get_auto_negotiation_advertised_technology_ability(olt_id, llid, portIdx, &abilities) == CTC_STACK_EXIT_OK )
	#endif
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
#endif

