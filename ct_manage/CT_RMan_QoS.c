#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "CT_RMan_Main.h"

int qosRuleFieldValue_ValToStr_parase( unsigned char select, CTC_STACK_value_t *pMatchVal, unsigned char *pMatchStr );

#ifdef __CT_EXTOAM_SUPPORT
#include   "gwEponSys.h"
#include  "../cli/Olt_cli.h"



/*----------------------------------------------------------------------------*/

int CT_RMan_PduData_To_Class( CT_RMan_Classification_t *pDClass, 
		CT_ExtOam_PduData_Classification_Resp_t *pPduData, ULONG pduDataLen )
{
	int i,j;
	ULONG curLength = 0;
	CT_RMan_rule_t *pRule;
	/*CT_RMan_rule_class_entry *pRuleEntry;*/
	UCHAR *pBuf;
	
	if( (pDClass == NULL) || (pPduData == NULL) || (pduDataLen == 0) )
		return VOS_ERROR;

	if( (pPduData->class.number_of_rules == 0) || (pPduData->class.number_of_rules > CT_MAX_CLASS_RULES_NUM) )
	{
		sys_console_printf("Classification&Marking : parse pdu,  number of rules is %d ERR\r\n", pPduData->class.number_of_rules);
		return VOS_ERROR;
	}

	pBuf = (UCHAR *)&pPduData->class;
	pDClass->number_of_rules = *pBuf;

	*pBuf++;
	curLength++;
	for( i=0; i<pDClass->number_of_rules; i++ )
	{
		pRule = (CT_RMan_rule_t *)pBuf;
		if( (pRule->number_of_entries == 0) || (pRule->number_of_entries > CT_MAX_CLASS_RULE_ENTRY_NUM) )
		{
			sys_console_printf("Classification&Marking : parse pdu,  number of entries is %d ERR\r\n", pRule->number_of_entries);
			return VOS_ERROR;
		}
		
		pDClass->rules[i].precedence = *pBuf++;
		pDClass->rules[i].length = *pBuf++;
		pDClass->rules[i].queue_Mapped = *pBuf++;
		pDClass->rules[i].ethernet_Priority = *pBuf++;
		pDClass->rules[i].number_of_entries = *pBuf++;

		curLength += ( sizeof(CT_RMan_rule_t) - CT_MAX_CLASS_RULE_ENTRY_NUM * sizeof(CT_RMan_rule_class_entry) );
			
		for( j=0; j<pDClass->rules[i].number_of_entries; j++ )
		{
			pDClass->rules[i].entries[j].field_select = *pBuf++;
			VOS_MemCpy( pDClass->rules[i].entries[j].match_value, pBuf, 6 );
			pBuf += 6;
			pDClass->rules[i].entries[j].validation_operator = *pBuf++;
		}
		curLength += ( pDClass->rules[i].number_of_entries * sizeof(CT_RMan_rule_class_entry) );
	}
	
	return VOS_OK;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_Class_To_PduData( CT_ExtOam_PduData_Classification_Resp_t *pPduData, ULONG *pPduDataLen,
			CT_RMan_Classification_t *pClass )
{
	int i,j;
	ULONG curLength = 0;
	CT_RMan_rule_t *pRule;
	/*CT_RMan_rule_class_entry *pRuleEntry;*/
	UCHAR *pBuf;
	
	if( (pClass == NULL) || (pPduData == NULL) || (pPduDataLen == NULL) )
		return VOS_ERROR;

	if( (pClass->number_of_rules == 0) || (pClass->number_of_rules > CT_MAX_CLASS_RULES_NUM) )
	{
		sys_console_printf("Classification&Marking : make PDU, number of rules is %d ERR\r\n", pClass->number_of_rules);
		return VOS_ERROR;
	}

	pBuf = (UCHAR *)&pPduData->class;

	pBuf[curLength++] = pClass->number_of_rules;
	
	for( i=0; i<pClass->number_of_rules; i++ )
	{
		pRule = (CT_RMan_rule_t *)&pClass->rules[i];
		if( (pRule->number_of_entries == 0) || (pRule->number_of_entries > CT_MAX_CLASS_RULE_ENTRY_NUM) )
		{
			sys_console_printf("Classification&Marking : make PDU, number of entries is %d ERR\r\n", pRule->number_of_entries);
			return VOS_ERROR;
		}
		
		pBuf[curLength++] = pRule->precedence;
		pBuf[curLength++] = pRule->length;
		pBuf[curLength++] = pRule->queue_Mapped;
		pBuf[curLength++] = pRule->ethernet_Priority;
		pBuf[curLength++] = pRule->number_of_entries;

		for( j=0; j<pRule->number_of_entries; j++ )
		{
			pBuf[curLength++] = pRule->entries[j].field_select;
			VOS_MemCpy( &pBuf[curLength], pRule->entries[j].match_value, 6 );
			curLength += 6;
			pBuf[curLength++] = pRule->entries[j].validation_operator;
		}
	}
	
	*pPduDataLen = curLength;
	
	return VOS_OK;
}

/*----------------------------------------------------------------------------*/

int CT_RMan_Classification_Marking_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_Classification_Req_t *pSendPduData;
	CT_ExtOam_PduData_Classification_Resp_t *pRecvPduData = NULL;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( call_flag == call_flag_sync )
	{
		if( pClass == NULL )
			return VOS_ERROR;
	}

	pSendPduData = (CT_ExtOam_PduData_Classification_Req_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->OUI, 3 );
	pSendPduData->extOpcode = OAM_ExtOpcode_Get_Request;
	pSendPduData->instanceBranch = 0x36;
	pSendPduData->instanceLeaf = 0x0001;
	pSendPduData->instanceWidth = 1;
	pSendPduData->instanceValue = ethPort;
	pSendPduData->variableBranch = 0xc7;
	pSendPduData->variableLeaf = 0x0031;
	pSendPduData->variableWidth = 1;
	pSendPduData->action = CT_Class_Action_List;

	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sizeof(CT_ExtOam_PduData_Classification_Req_t),
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		if( CT_ExtOamDebugSwitch & 2 )
			sys_console_printf( "Classification&Marking : get, CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL)  )
			{
				if( CT_ExtOamDebugSwitch & 2 )
					sys_console_printf( "Classification_Marking_get RECV\r\n" );
				
				if( recvPduDataLen > sizeof(CT_ExtOam_PduData_Classification_Resp_t) )
					recvPduDataLen = sizeof(CT_ExtOam_PduData_Classification_Resp_t);
				
				if( CT_RMan_PduData_To_Class( pClass, pRecvPduData, recvPduDataLen ) == VOS_OK )
				{
					if( CT_ExtOamDebugSwitch & 2 )
						sys_console_printf( "Classification_Marking_get OK\r\n" );
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
		sys_console_printf( "Classification&Marking : get, CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );

		CT_ExtOam_Free( (VOID*)pSendPduData );
	}

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;

	return VOS_ERROR;
}


int CT_RMan_Classification_Marking_del( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_Classification_Resp_t *pSendPduData;
	CT_ExtOam_PduData_Classification_Resp_t *pRecvPduData = NULL;
	ULONG sendPduDataLen = 0;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	pSendPduData = (CT_ExtOam_PduData_Classification_Resp_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->descriptor.OUI, 3 );
	pSendPduData->descriptor.extOpcode = OAM_ExtOpcode_Set_Request;
	pSendPduData->descriptor.instanceBranch = 0x36;
	pSendPduData->descriptor.instanceLeaf = 0x0001;
	pSendPduData->descriptor.instanceWidth = 1;
	pSendPduData->descriptor.instanceValue = ethPort;
	pSendPduData->descriptor.variableBranch = 0xc7;
	pSendPduData->descriptor.variableLeaf = 0x0031;
	pSendPduData->descriptor.variableWidth = 1;

	if( (pClass == NULL) || (pClass->number_of_rules == 0) )
	{
		sendPduDataLen = sizeof(CT_ExtOam_PduData_Classification_Req_t);
		pSendPduData->descriptor.action = CT_Class_Action_Destroy;
	}
	else	
	{
		if( CT_RMan_Class_To_PduData( pSendPduData, &sendPduDataLen, pClass ) == VOS_OK )
		{
			pSendPduData->descriptor.action = CT_Class_Action_Delete;
		}
		else
		{
			/* 在参数错误的情况下，则全部删除，暂时保留 */
			sendPduDataLen = sizeof(CT_ExtOam_PduData_Classification_Req_t);
			pSendPduData->descriptor.action = CT_Class_Action_Destroy;
		}
	}
	
	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sendPduDataLen,
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		if( CT_ExtOamDebugSwitch & 2 )
			sys_console_printf( "Classification&Marking : del, CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL)  )
			{
				if( pRecvPduData->descriptor.variableWidth == 0x80 )
				{
					if( CT_ExtOamDebugSwitch & 2 )
						sys_console_printf( "Classification&Marking : del  OK\r\n" );
				}
				else
				{
					rc = CT_Ext_OAM_Send_Resp_Err;
					sys_console_printf( "Classification&Marking : del response ERROR\r\n" );
				}
			}
			else
			{
				rc = CT_Ext_OAM_Send_Resp_Err;
				sys_console_printf( "Classification&Marking : del, CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
			}
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "Classification&Marking : del, CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );
	}

	CT_ExtOam_Free( (VOID*)pSendPduData );

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;
	
	return VOS_ERROR;
}

int CT_RMan_Classification_Marking_add( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass )
{
	int rc;
	UCHAR ponid, onuid;
	CT_ExtOam_PduData_Classification_Resp_t *pSendPduData;
	CT_ExtOam_PduData_Classification_Resp_t *pRecvPduData = NULL;
	ULONG sendPduDataLen = 0;
	ULONG recvPduDataLen = 0;

	if( (slotno == 0) || (slotno >= SYS_CHASSIS_SLOTNUM) ||
		(pon == 0) || (pon >= PONPORTPERCARD) ||
		(onu == 0) || (onu >= MAXONUPERPON) )
		return VOS_ERROR;

	if( pClass == NULL )
		return VOS_ERROR;
	if( pClass->number_of_rules == 0 )
		return VOS_ERROR;

	pSendPduData = (CT_ExtOam_PduData_Classification_Resp_t *)CT_ExtOam_Alloc();
	if( pSendPduData == NULL )
		return VOS_ERROR;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	onuid = GetLlidByOnuIdx(ponid, onu-1 );

	VOS_MemZero( pSendPduData->descriptor.OUI, 3 );
	pSendPduData->descriptor.extOpcode = OAM_ExtOpcode_Set_Request;
	pSendPduData->descriptor.instanceBranch = 0x36;
	pSendPduData->descriptor.instanceLeaf = 0x0001;
	pSendPduData->descriptor.instanceWidth = 1;
	pSendPduData->descriptor.instanceValue = ethPort;
	pSendPduData->descriptor.variableBranch = 0xc7;
	pSendPduData->descriptor.variableLeaf = 0x0031;
	pSendPduData->descriptor.variableWidth = 1;
	pSendPduData->descriptor.action = CT_Class_Action_Add;

	if( CT_RMan_Class_To_PduData( pSendPduData, &sendPduDataLen, pClass ) == VOS_ERROR )
	{
		CT_ExtOam_Free( (VOID*)pSendPduData );
		return VOS_ERROR;
	}
	
	rc = CT_ExtOAM_Send( ponid, onuid, 
				call_flag, call_notify, 
				(VOID*)pSendPduData, sendPduDataLen,
				(VOID*)&pRecvPduData, &recvPduDataLen );
	if( rc == CT_Ext_OAM_Send_OK )
	{
		if( CT_ExtOamDebugSwitch & 2 )
			sys_console_printf( "Classification&Marking : add, CT_ExtOAM_Send is OK\r\n" );

		if( call_flag == call_flag_sync )
		{
			if( (recvPduDataLen != 0) && (pRecvPduData != NULL)  )
			{
				if( pRecvPduData->descriptor.variableWidth == 0x80 )
				{
					if( CT_ExtOamDebugSwitch & 2 )
						sys_console_printf( "Classification&Marking : add  OK\r\n" );
				}
				else
				{
					rc = CT_Ext_OAM_Send_Resp_Err;
					sys_console_printf( "Classification&Marking : add response ERROR\r\n" );
				}
			}
			else
			{
				rc = CT_Ext_OAM_Send_Resp_Err;
				sys_console_printf( "Classification&Marking : add, CT_ExtOAM_Send: recvPduDataLen = %d\r\n", recvPduDataLen );
			}
			
			CT_ExtOam_Free( (VOID*)pSendPduData );
		}
		return VOS_OK;
	}
	else
	{
		sys_console_printf( "Classification&Marking : add, CT_ExtOAM_Send is ERROR, ERR code = %d\r\n", rc );
	}

	CT_ExtOam_Free( (VOID*)pSendPduData );

	if( rc == CT_Ext_OAM_Send_OK )
		return VOS_OK;
	
	return VOS_ERROR;
}



/*----------------------------------------------------------------------------*/

typedef struct {
	UCHAR  precedence;
	UCHAR  length;
	UCHAR  queue_Mapped;
	UCHAR  ethernet_Priority;
	UCHAR  number_of_entries;
	CT_RMan_rule_class_entry entries[2];
} __attribute__((packed)) test_CT_RMan_rule_t;

typedef struct {
	UCHAR number_of_rules;
	test_CT_RMan_rule_t rules[2];
} __attribute__((packed))  test_CT_RMan_Classification_t;


typedef struct {
	CT_ExtOam_PduData_Classification_Req_t	descriptor;
	test_CT_RMan_Classification_t			class;
} __attribute__((packed))  test_CT_ExtOam_PduData_Classification_Resp_t;

#include "Vos_string.h"
void testExtOam_QoS_Pdu( UCHAR port, UCHAR *pdu, ULONG *pLen )
{
	ULONG length;
	test_CT_ExtOam_PduData_Classification_Resp_t *pRespPduData;
	CT_ExtOamPdu_t *pOamPdu = (CT_ExtOamPdu_t*)pdu;

	VOS_MemZero( pdu, sizeof(test_CT_ExtOam_PduData_Classification_Resp_t) );
	
	VOS_MemCpy( pOamPdu->DA, ct_extoam_da_mac, 6 );
	VOS_MemCpy( pOamPdu->SA, ct_extoam_sa_mac, 6 );
	pOamPdu->OamType = 0x8809;
	pOamPdu->SubType = 3;
	pOamPdu->Flag = 0/*0x50*/;
	pOamPdu->Code = 0xfe;

	VOS_MemZero( pRespPduData->descriptor.OUI, 3 );
	pRespPduData = (test_CT_ExtOam_PduData_Classification_Resp_t *)pOamPdu->PduData;
	pRespPduData->descriptor.extOpcode = OAM_ExtOpcode_Get_Response;
	pRespPduData->descriptor.instanceBranch = 0x36;
	pRespPduData->descriptor.instanceLeaf = 0x0001;
	pRespPduData->descriptor.instanceWidth = 1;
	pRespPduData->descriptor.instanceValue = port;
	pRespPduData->descriptor.variableBranch = 0xc7;
	pRespPduData->descriptor.variableLeaf = 0x0031;
	pRespPduData->descriptor.variableWidth = sizeof(test_CT_RMan_Classification_t);
	pRespPduData->descriptor.action = CT_Class_Action_List;

	pRespPduData->class.number_of_rules = 2;
	pRespPduData->class.rules[0].precedence = 1;
	pRespPduData->class.rules[0].length = sizeof(test_CT_RMan_rule_t);
	pRespPduData->class.rules[0].queue_Mapped = 2;
	pRespPduData->class.rules[0].ethernet_Priority = 2;
	pRespPduData->class.rules[0].number_of_entries = 2;
	pRespPduData->class.rules[0].entries[0].field_select = 2;
	pRespPduData->class.rules[0].entries[0].match_value[5] = 2;
	pRespPduData->class.rules[0].entries[0].validation_operator = 1;
	pRespPduData->class.rules[0].entries[1].field_select = 3;
	pRespPduData->class.rules[0].entries[1].match_value[5] = 3;
	pRespPduData->class.rules[0].entries[1].validation_operator = 1;

	pRespPduData->class.rules[1].precedence = 2;
	pRespPduData->class.rules[1].length = sizeof(test_CT_RMan_rule_t);
	pRespPduData->class.rules[1].queue_Mapped = 4;
	pRespPduData->class.rules[1].ethernet_Priority = 4;
	pRespPduData->class.rules[1].number_of_entries = 2;
	pRespPduData->class.rules[1].entries[0].field_select = 2;
	pRespPduData->class.rules[1].entries[0].match_value[5] = 3;
	pRespPduData->class.rules[1].entries[0].validation_operator = 1;
	pRespPduData->class.rules[1].entries[1].field_select = 2;
	pRespPduData->class.rules[1].entries[1].match_value[5] = 4;
	pRespPduData->class.rules[1].entries[1].validation_operator = 1;
	
	length = sizeof(test_CT_ExtOam_PduData_Classification_Resp_t) + CT_EXTOAM_PDUDATA_OFFSET;
	if( length < MIN_CT_EXTOAM_FRAME_SIZE )
		length = MIN_CT_EXTOAM_FRAME_SIZE;
	
	*pLen = length;
	
}

VOID showQoS_sync( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR port )
{
	CT_RMan_Classification_t class;
	
	/*VOS_MemZero( &sn, sizeof(CT_RMan_OnuSN_t) );*/
	if( CT_RMan_Classification_Marking_get( slotno, pon, onu, port, call_flag_sync, 0, &class ) == VOS_OK )
	{
		sys_console_printf("ONU%d/%d/%d port%d QoS : \r\n", slotno, pon, onu, port );
	}
	else
		sys_console_printf("ERR\r\n");
}

/*----------------------------------------------------------------------------*/

#endif /*__CT_EXTOAM_SUPPORT*/
#if 0
STATUS	getEthPortQosSetSel( ULONG devIdx, ULONG brdIdx, ULONG ethIdx, ULONG *idx );
STATUS	getFirstQosRuleEntry( ULONG *devIdx, ULONG * pqos_set_index, ULONG *pRuleIdx, QOS_RULE_ENTRY ** pEntry );
STATUS	getNextQosRuleEntry( UCHAR idxCount, ULONG *idxs,  QOS_RULE_ENTRY **ppEntry );

STATUS	getFirstQosRuleFieldEntry( ULONG *pDevIdx, ULONG *pQosRuleSetIdx, ULONG *pQosRuleIdx, ULONG *pQosRuleFieldIdx, QOS_FIELD_ENTRY **ppFieldEntry );
STATUS	getNextQosRuleFieldEntry( ULONG idxCount, ULONG *idxs,  QOS_FIELD_ENTRY **ppFieldEntry );
typedef	struct {
	UCHAR	qos_rule_index;
	UCHAR	qos_rule_queue_mapped;
	UCHAR	qos_rule_pri_mask;
	UCHAR	qos_rule_entry_status;
	QOS_FIELD_ENTRY	qos_rule_fields[MAX_FIELDS_PER_RULE];
}__attribute__((packed))QOS_RULE_ENTRY;
typedef struct{
	UCHAR	qos_field_index;
	UCHAR	qos_field_sel;
	UCHAR	qos_filed_operator;
	UCHAR	qos_field_entry_status;
	UCHAR	qos_filed_match_val[6];
}__attribute__((packed)) QOS_FIELD_ENTRY;
#endif

char * match_val_to_str( ulong_t select, uchar_t *pval )
{
	CTC_STACK_value_t match_val;
	static uchar_t match_str[20];
	VOS_MemZero( match_str, sizeof(match_str) );
	VOS_MemCpy( match_val.mac_address, pval, 6 );
	qosRuleFieldValue_ValToStr_parase( select-1, &match_val, match_str );
	return match_str;
}

#ifndef CTC_OBSOLETE		/* removed by xieshl 20120607 */
char * qos_rule_field_sel_str ( int field_sel )
{
	char *pStr;
	switch( field_sel )
	{
		case 1:
			pStr = "d-mac";
			break;
		case 2:
			pStr = "s-mac";
			break;
		case 3:
			pStr = "cos";
			break;
		case 4:
			pStr = "vlan";
			break;
		case 5:
			pStr = "eth-type";
			break;
		case 6:	   
			pStr = "d-ip";
			break;
		case 7:	
			pStr = "s-ip";
			break;
		case 8:
			pStr = "ip-type";
			break;
		case 9:
			pStr = "ip4-tos-dscp";
			break;
		case 10:
			pStr = "ip6-precedence";
			break;
		case  11:
			pStr = "s-l4-port";
			break;
		case  12:
			pStr = "d-l4-port";
			break;
		case  13:
			pStr = "other";
			break;
		default:
			pStr = "-";
			break;
	}
	return pStr;
}

int select_field_to_int( char *str )
{
	/*int val = 0;*/
	int sel;

	for( sel=1; sel<13; sel++ )
	{
	    char *pstr = qos_rule_field_sel_str(sel);
		if( VOS_StrCmp(str, pstr) == 0 )
			return sel;
	}
	/*if( VOS_MemCmp(str, "d-mac", 2) == 0 )
		val = 1;
	else if( VOS_MemCmp(str, "s-mac", 2) == 0 )
		val = 2;
       else  if( VOS_MemCmp(str, "cos", 2) == 0 )
		val = 3;
	else  if( VOS_MemCmp(str, "vlan ", 2) == 0 )
		val = 4;
	else  if( VOS_MemCmp(str, "eth-type", 2) == 0 )
		val = 5;
	else  if( VOS_MemCmp(str, "d-ip", 2) == 0 )
		val = 6;
	else  if( VOS_MemCmp(str, "s-ip", 2) == 0 )
		val = 7;
       else  if( VOS_MemCmp(str, "ip-type", 2) == 0 )
		val = 8;
	else  if( VOS_MemCmp(str, "ip4-tos-dscp", 2) == 0 )
		val = 9;
	else  if( VOS_MemCmp(str, "ip6-precedence ", 2) == 0 )
		val = 10;
	else  if( VOS_MemCmp(str, "s-l4-port ", 2) == 0 )
		val = 11;
	else  if( VOS_MemCmp(str, "d-l4-port", 2) == 0 )
		val = 12;
	else 
		 val = 0;*/

	return 0;
}			

char * qos_rule_field_operator_str( int opr )
{
	char *pStr;
	switch( opr )
	{
		case 1:
			pStr = "never";
			break;
		case 2:
			pStr = "equal";
	 		break;
		case 3:
			pStr = "not-equal";
			break;
		case 4:
			pStr = "less-equal";
			break;
		case 5:
			pStr = "greater-equal";
			break;
		case 6:
			pStr = "exist";
			break;
		case 7:
			pStr = "not-exist";
			break;
		case 8:
			pStr = "always";
		       break;
		default:
			pStr = "-";
			break;
	}
	return pStr;
}

int operator_field_to_int( char *str )
{
	int opr;
	for( opr=1; opr<=8; opr++ )
	{
		if( VOS_StrCmp(str, qos_rule_field_operator_str(opr)) == 0 )
			return opr;
	}
	return 0;
/*        int val = 0;
	if( VOS_MemCmp(str, "never", 2) == 0 )
		val = 1;
       else if( VOS_MemCmp(str, "equal", 2) == 0 )
		val = 2;
        else if( VOS_MemCmp(str, "equal", 2) == 0 )
		val = 3;
	else if( VOS_MemCmp(str, "not-equal", 2) == 0 )
		val = 4; 
	else if( VOS_MemCmp(str, " less-equal", 2) == 0 )
		val = 5;
	else if( VOS_MemCmp(str, "greater-equal", 2) == 0 )
		val = 6;
	else if( VOS_MemCmp(str, "exist", 2) == 0 )
		val = 7;
	else if( VOS_MemCmp(str, "not-exist", 2) == 0 )
		val = 8;
	else if( VOS_MemCmp(str, "always", 2) == 0 )
		val = 9;
	else
		val = 0;

	return val;*/
 }

int qosRuleFieldValue_StrToVal_parase( unsigned char select, unsigned char *pMatchStr, CTC_STACK_value_t *pMatchVal )
{
	int rc_err = VOS_ERROR;
	/*int i;
	char	*token;
	unsigned long val;*/
	char	*pToken;
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

#else
/*add by shixh@2007/08/24*/
/*show policy-map*/
DEFUN(
	show_policy_map,
	show_policy_map_cmd,
	"show policy-map {port <port_list>}*1",
	SHOW_STR
	"show QoS map information\n"
	"set port<1-24>\n")
{
       ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
	ULONG     idx;
       ULONG         pqos_set_index;
       ULONG                 pRuleIdx;
	QOS_RULE_ENTRY      * pEntry;

	/*UCHAR   idxCount;*/
	ULONG   idxs[4];
	QOS_RULE_ENTRY      *ppEntry = NULL;
	 ULONG      pDevIdx;
	 ULONG  pQosRuleSetIdx;
	 ULONG  pQosRuleIdx;
	 ULONG    pQosRuleFieldIdx;
	 QOS_FIELD_ENTRY     *ppFieldEntry;
	   
	ULONG     portNum = 0;   
	ULONG     pon;
	ULONG     onu;

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	if (argc==0)
	{
		for( ethIdx = 1; ethIdx <=portNum; ethIdx++ )
		{
		
		if(getEthPortQosSetSel(devIdx,1,ethIdx ,&idx)==VOS_OK)
		 	vty_out( vty, "port%d QoS policy-index=%d\r\n", ethIdx,idx );
		else
		{
				/*vty_out( vty, "get port%d  QoS policy-index fail!!\r\n", ethIdx);*/
			continue;
		}
		if(getFirstQosRuleEntry(&devIdx,&pqos_set_index,&pRuleIdx,&pEntry)==VOS_OK)
		{
			idxs[0]=devIdx;
			idxs[1]=pqos_set_index;
			idxs[2]=pRuleIdx;
	
			
			/*vty_out( vty, "get port%d first policy_index is:%d\r\n", ethIdx,pqos_set_index);*/
			vty_out( vty, " class_idx=%d", pRuleIdx );
			/*vty_out( vty, " class_idx=%d", pEntry->qos_rule_index);*/
			vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
			vty_out( vty, ",priority_mask=%d\r\n", pEntry->qos_rule_pri_mask );
			/*vty_out( vty, "get port%d first qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     while(getNextQosRuleEntry(3, &idxs, ppEntry)==VOS_OK)
                     {
                     	if(idxs[1]==0)
				   break;
				vty_out( vty, " class_idx=%d", pEntry->qos_rule_index);
				vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
				vty_out( vty, ",priority_mask=%d\r\n", pEntry->qos_rule_pri_mask );
				/*vty_out( vty, "get port%d next qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     }	 	
					 	
		}
		
		if(getFirstQosRuleFieldEntry(&pDevIdx,&pQosRuleSetIdx,&pQosRuleIdx,&pQosRuleFieldIdx,&ppFieldEntry)==VOS_OK)
			{
			
			idxs[0]=pDevIdx;
			idxs[1]=pQosRuleSetIdx;
			idxs[2]=pQosRuleIdx;
			idxs[3]=pQosRuleFieldIdx;
			
			/*vty_out( vty, "get first pDevIdx is:%d\r\n", pDevIdx);
			vty_out( vty, "qos_rule_set=%d\r\n", pQosRuleSetIdx);*/
			vty_out( vty, " class_idx=%d\r\n", pQosRuleIdx);
			/*vty_out( vty, "  rule_field_idx=%d\r\n", pQosRuleFieldIdx);*/
			vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index);
			vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel);
			vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator);
			/*vty_out( vty, "get first qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
			vty_out( vty, "get first qos_filed_match_val is:%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[4]);*/
			vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
			
			while(getNextQosRuleFieldEntry(5,&idxs,ppFieldEntry)==VOS_OK)
			{
				if(idxs[1]==0||idxs[2]==0)
					break;
				vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index);
				vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel);
				vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator);
				/*vty_out( vty, "get next qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
				vty_out( vty, "get next qos_filed_match_val is:%d%d%d%d%d%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[5]);*/
				vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
				}
			}
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if(ethIdx>portNum)
		{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();*/	/* removed by xieshl 20120906, 解决内存丢失问题，同时非法端口不处理，但不报错，下同 */
			
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
			 if(getEthPortQosSetSel(devIdx,1,ethIdx ,&idx)==VOS_OK)
		 		vty_out( vty, "port%d QoS policy-index=%d\r\n", ethIdx,idx);
		       /*else
				vty_out( vty, "get port%d  QoS policy-index fail!!\r\n", ethIdx);*/

		       if(getFirstQosRuleEntry(&devIdx,&pqos_set_index,&pRuleIdx,&pEntry)==VOS_OK)
			{
			idxs[0]=devIdx;
			idxs[1]=pqos_set_index;
			idxs[2]=pRuleIdx;
	
			
			/*vty_out( vty, "get port%d first policy_index is:%d\r\n", ethIdx,pqos_set_index);*/
			vty_out( vty, " class_idx=%d", pRuleIdx);
			/*vty_out( vty, " class_idx=%d", pEntry->qos_rule_index );*/
			vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
			vty_out( vty, ",priority_mask=%d\r\n", ethIdx,pEntry->qos_rule_pri_mask );
			/*vty_out( vty, "get port%d first qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     while(getNextQosRuleEntry(3,&idxs,ppEntry)==VOS_OK)
                     	{
                     	if(idxs[1]==0)
				   break;
				vty_out( vty, " class_idx=%d", pEntry->qos_rule_index );
				vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
				vty_out( vty, ",priority_mask=%d\r\n", pEntry->qos_rule_pri_mask );
				/*vty_out( vty, "get port%d next qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     	}	 	
					 	
			}
		
		if(getFirstQosRuleFieldEntry(&pDevIdx,&pQosRuleSetIdx,&pQosRuleIdx,&pQosRuleFieldIdx,&ppFieldEntry)==VOS_OK)
			{
			
			idxs[0]=pDevIdx;
			idxs[1]=pQosRuleSetIdx;
			idxs[2]=pQosRuleIdx;
			idxs[3]=pQosRuleFieldIdx;
			
			/*vty_out( vty, "get first pDevIdx is:%d\r\n", pDevIdx);
			vty_out( vty, "QosRuleSetIdx=%d\r\n", pQosRuleSetIdx);*/
			vty_out( vty, " class_idx=%d\r\n", pQosRuleIdx);
			/*vty_out( vty, "  QosRuleFieldIdx=%d\r\n", pQosRuleFieldIdx);*/
			vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index);
			vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel);
			vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator);
			/*vty_out( vty, "get first qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
			vty_out( vty, "get first qos_filed_match_val is:%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[4]);*/
			vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
			
			while(getNextQosRuleFieldEntry(5,&idxs,ppFieldEntry)==VOS_OK)
				{
				if(idxs[1]==0||idxs[2]==0)
					break;
				vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index );
				vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel );
				vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator );
				/*vty_out( vty, "get next qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
				vty_out( vty, "get next qos_filed_match_val is:%d%d%d%d%d%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[5]);*/
				vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
				}
			
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
			#if 0	
			ethIdx = VOS_AtoI( argv[0] );
			if(getEthPortQosSetSel(devIdx,1,ethIdx ,&idx)==VOS_OK)
		 		vty_out( vty, "port%d QoS policy-index=%d\r\n", ethIdx,idx);
		       /*else
				vty_out( vty, "get port%d  QoS policy-index fail!!\r\n", ethIdx);*/

		       if(getFirstQosRuleEntry(&devIdx,&pqos_set_index,&pRuleIdx,&pEntry)==VOS_OK)
			{
			idxs[0]=devIdx;
			idxs[1]=pqos_set_index;
			idxs[2]=pRuleIdx;
	
			
			/*vty_out( vty, "get port%d first policy_index is:%d\r\n", ethIdx,pqos_set_index);*/
			vty_out( vty, " class_idx=%d", pRuleIdx);
			/*vty_out( vty, " class_idx=%d", pEntry->qos_rule_index );*/
			vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
			vty_out( vty, ",priority_mask=%d\r\n", ethIdx,pEntry->qos_rule_pri_mask );
			/*vty_out( vty, "get port%d first qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     while(getNextQosRuleEntry(4,&idxs,ppEntry)==VOS_OK)
                     	{
                     	if(idxs[1]==0)
				   break;
				vty_out( vty, " class_idx=%d", pEntry->qos_rule_index );
				vty_out( vty, ",queue_map=%d", pEntry->qos_rule_queue_mapped );
				vty_out( vty, ",priority_mask=%d\r\n", pEntry->qos_rule_pri_mask );
				/*vty_out( vty, "get port%d next qos_rule_entry_status is:%d\r\n", ethIdx,pEntry->qos_rule_entry_status);*/
                     	}	 	
					 	
			}
		
		if(getFirstQosRuleFieldEntry(&pDevIdx,&pQosRuleSetIdx,&pQosRuleIdx,&pQosRuleFieldIdx,&ppFieldEntry)==VOS_OK)
			{
			
			idxs[0]=pDevIdx;
			idxs[1]=pQosRuleSetIdx;
			idxs[2]=pQosRuleIdx;
			idxs[3]=pQosRuleFieldIdx;
			
			/*vty_out( vty, "get first pDevIdx is:%d\r\n", pDevIdx);
			vty_out( vty, "QosRuleSetIdx=%d\r\n", pQosRuleSetIdx);*/
			vty_out( vty, " class_idx=%d\r\n", pQosRuleIdx);
			/*vty_out( vty, "  QosRuleFieldIdx=%d\r\n", pQosRuleFieldIdx);*/
			vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index);
			vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel);
			vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator);
			/*vty_out( vty, "get first qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
			vty_out( vty, "get first qos_filed_match_val is:%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[4]);*/
			vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
			
			while(getNextQosRuleFieldEntry(5,&idxs,ppFieldEntry)==VOS_OK)
				{
				if(idxs[1]==0||idxs[2]==0)
					break;
				vty_out( vty, "  field_idx=%d", ppFieldEntry->qos_field_index );
				vty_out( vty, ",selete=%d", ppFieldEntry->qos_field_sel );
				vty_out( vty, ",operator=%d", ppFieldEntry->qos_filed_operator );
				/*vty_out( vty, "get next qos_field_entry_status is:%d\r\n", ppFieldEntry->qos_field_entry_status);
				vty_out( vty, "get next qos_filed_match_val is:%d%d%d%d%d%d\r\n", ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[0],ppFieldEntry->qos_filed_match_val[1],ppFieldEntry->qos_filed_match_val[2],ppFieldEntry->qos_filed_match_val[3],ppFieldEntry->qos_filed_match_val[4],ppFieldEntry->qos_filed_match_val[5]);*/
				vty_out( vty, ",match_val=%s\r\n", match_val_to_str(ppFieldEntry->qos_field_sel, ppFieldEntry->qos_filed_match_val) );
				}
			
			}
		#endif 
		}
	return CMD_SUCCESS;
}


							

/*add by shixh@2007/08/17*/
/*set service-policy*/
DEFUN(set_service_policy,
	  set_service_policy_cmd,
         "service-policy port [all|<port_list>] policy <policy_index>",
	  "set QoS policy\n"
	  "set QoS policy port\n"
	  "All port\n"
	  "set  port\n"
	  "<policy-index>\n" )
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 ULONG   ethIdx;
 	 ULONG   idx;
  	 
         ULONG  pon;
         ULONG  onu;
	 
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
	
	
	if(argc==2) 
		{
		if(VOS_StriCmp(argv[0], "all")==0)
			for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		        {
		        idx=VOS_AtoI( argv[1] );
			if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,idx)  == CTC_STACK_EXIT_ERROR ) 
				vty_out( vty, "set port%d QoS policy-index fail!\r\n", ethIdx);
			}
		else
		{
			idx=VOS_AtoI( argv[1] );
			/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
			}
			END_PARSE_PORT_LIST_TO_PORT();*/
			 
			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if( ethIdx <= portNum )
			{
				if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,idx)  == CTC_STACK_EXIT_ERROR) 
			  		vty_out( vty, "set port%d QoS policy-index fail!\r\n", ethIdx);
			}
			END_PARSE_PORT_LIST_TO_PORT();
			#if 0
			ethIdx = VOS_AtoI( argv[0] );
			 idx=VOS_AtoI( argv[1] );
		     if( ethIdx <25&&ethIdx > 0 )
		     	{
		      if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,idx)  == CTC_STACK_EXIT_OK ) 
			
			vty_out( vty, "set port%d  QoS policy-index is%d\r\n", ethIdx,idx);
			else
			  	vty_out( vty, "set port%d  QoS policy-index fail!!\r\n", ethIdx);
		     	}
			 else
			 	vty_out( vty, "out of range<1~24>\r\n", ethIdx);
			 #endif
			}
		}
	/*else
		vty_out( vty, " PARA error!!\r\n" );*/

	      return CMD_SUCCESS;
}


/*add by shixh@2007/08/17*/
/*undo service-policy*/
DEFUN(undo_service_policy,
	  undo_service_policy_cmd,
         "undo service-policy port [all|<port_list>]",
         "undo operation\n"
	  "undo QoS policy\n"
	  "All port\n"
	  "set  port\n" )
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 ULONG   ethIdx;
 
         ULONG  pon;
         ULONG  onu;
	 
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
	
	
	if(argc==1) 
		{
		if(VOS_StriCmp(argv[0], "all")==0)
			for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		        {
			if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,0)  == CTC_STACK_EXIT_ERROR ) 
			
			vty_out( vty, "undo QoS policy-index is fail!\r\n", ethIdx);
			}
		else
		{
			/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
			}
			END_PARSE_PORT_LIST_TO_PORT();*/
			
			BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if( ethIdx <= portNum )
			{
				 if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,0)  == CTC_STACK_EXIT_ERROR ) 
					vty_out( vty, "undo QoS policy-index is fail!\r\n", ethIdx);
			}
			END_PARSE_PORT_LIST_TO_PORT();
			#if 0
			ethIdx = VOS_AtoI( argv[0] );
		    	 if( ethIdx <25&&ethIdx > 0 )
		       	{ 
		     		 if( setEthPortQosSetSel(devIdx,brdIdx, ethIdx,0)  == CTC_STACK_EXIT_OK ) 
			
				vty_out( vty, "set port%d  QoS policy-index is already undo\r\n", ethIdx);
		     		}
			 else
			 	vty_out( vty, "out of range<1~24>\r\n", ethIdx);
			 #endif
			}
		}
	/*else
		vty_out( vty, " PARA error!!\r\n" );*/

	      return CMD_SUCCESS;
}


/*add by shixh@2007/08/17*/
/*set policy-map*/
DEFUN(set_policy_map,
	  set_policy_map_cmd,
         "policy-map <policy_index> <class_map_index> queue-map <0-7> priority-mark <0-7>",
	  "set policy map\n"
	  "set policy index\n"
	  "set class map index\n"
	  "<0-7>\n"
	  "priority-mark\n"
	  "<0-7>\n" )
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 /*ULONG   ethIdx;*/
 	 ULONG   qosset;
	 ULONG   qosrule;
	 ULONG   QueueMap;
	 ULONG   PriorityMark;
	 	
         ULONG  pon;
         ULONG  onu;

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
	if(argc==4) 
		{
		qosset = VOS_AtoI( argv[0] );
		qosrule = VOS_AtoI( argv[1] );
		QueueMap = VOS_AtoI( argv[2] );
		PriorityMark = VOS_AtoI( argv[3] );
		
		if(QueueMap<8&&QueueMap>=0&&PriorityMark<8&&PriorityMark>=0)
			{
			if(setQosRuleEntry(devIdx,qosset,qosrule,QueueMap,PriorityMark)==CTC_STACK_EXIT_ERROR)
				vty_out( vty, " set QoS map fail!\r\n");
			}
		else
			vty_out( vty, " out of range<0~7>\r\n");
		}
	/*else
		vty_out( vty, " PARA error!!\r\n");*/
	       return CMD_SUCCESS;
}
		

/*add by shixh@2007/08/17*/
/*undo policy-map*/
DEFUN(undo_policy_map,
	  undo_policy_map_cmd,
         "undo policy-map <policy_index> <class_map_index> ",
	  "undo operation\n"
	  "undo QoS map\n"
	  "policy-index\n"
	  "class-map-index\n")
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 /*ULONG   ethIdx;*/
 	 ULONG   qosset;
	 ULONG   qosrule;
	 
         ULONG  pon;
         ULONG  onu;
	 
	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
	if(argc==2) 
		{
		qosset = VOS_AtoI( argv[0] );
		qosrule = VOS_AtoI( argv[1] );

		if(delQosRuleEntry(devIdx,qosset,qosrule)==CTC_STACK_EXIT_ERROR)
			vty_out( vty, " undo QoS map fail!\r\n");
		}
	/*else
		vty_out( vty, " PARA error!!\r\n");*/
	       return CMD_SUCCESS;
}
				     

/*add by shixh@2007/08/17*/
/*set class-match*/


DEFUN(set_class_match,
	  set_class_match_cmd,
         "class-match <policy_index> <class_map_index> <match_index> select [d-mac|s-mac|cos|vlan|eth-type|d-ip|s-ip|ip-type|ip4-tos-dscp|ip6-precedence|s-l4-port|d-l4-port] <match_value> {operator [never|equal|not-equal|less-equal|greater-equal|exist|not-exist|always]}*1 ",
	   "set class match\n" 
	  "policy-index\n" 
	  "class-map-index\n" 
	  "match-index\n" 
	  "d-mac addr\n"
	  "s-mac addr\n"
	  "cos\n"
	  "vlan id\n"
	  "eth-type\n"
	  "d-ip addr\n"
	  "s-ip addr\n"
	  "ip-type\n"
	  "ip4-tos-dscp\n"
	  "ip6-precedence\n"
	  "s-l4-port"
	  "d-l4-port\n" 
	  "match-value\n" 
	  "operator\n"
	  "never\nequal\nnot-equal\nless-equal\ngreater-equaln\existn\not-existn\always\n" )
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 /*ULONG   ethIdx;*/
 	 ULONG   qosfieldset;
	 ULONG   qosfieldrule;
	 ULONG   qosfield;
	 ULONG   fieldSelect;
	 uchar    *matchval;
	 ULONG    fieldOperator = 2;
	 
         ULONG  pon;
         ULONG  onu;

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
	qosfieldset = VOS_AtoI( argv[0] );
	qosfieldrule = VOS_AtoI( argv[1] );
	qosfield = VOS_AtoI( argv[2] );
	fieldSelect = select_field_to_int( argv[3] );
	matchval = argv[4];
	if( argc == 6 ) 
		fieldOperator = operator_field_to_int( argv[5] );
	if(setQosRuleFieldEntry(devIdx,qosfieldset,qosfieldrule,qosfield,fieldSelect,matchval,fieldOperator)==CTC_STACK_EXIT_ERROR)
		vty_out( vty, " set class map fail!\r\n");

       return CMD_SUCCESS;
}


/*add by shixh@2007/08/17*/
/*undo class-match*/
DEFUN(undo_class_match,
	  undo_class_match_cmd,
         "undo class-match <policy_index> <class_map_index> <match_index> ",
	  "undo operation\n"
	  "undo class match \n"
	  "policy-index\n"
	  "class-map-index\n"
	   "match-index\n")
{	 	
        ULONG   devIdx;
	 ULONG   brdIdx;
	 /*ULONG   ethIdx;*/
 	 ULONG   qosset;
	 ULONG   qosrule;
	 ULONG    qosfield;
  	 
         ULONG  pon;
         ULONG  onu;
	 
	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
	
	if(argc==3) 
	{
		qosset = VOS_AtoI( argv[0] );
		qosrule = VOS_AtoI( argv[1] );
              qosfield=VOS_AtoI( argv[2] );
		if(delQosRuleFieldEntry(devIdx,qosset,qosrule,qosfield)==CTC_STACK_EXIT_ERROR)
			vty_out( vty, " undo class match fail!\r\n");
	}
	/*else
		vty_out( vty, " PARA error!!\r\n");*/
	return CMD_SUCCESS;
}


#ifdef  ONU_QOS_CLASSIFICATION_MARKING
/* added by chenfj 2007-7-19
    增加ONU QOS 命令*/

DEFUN (
	onu_qos_destMac_mapping_func,
	onu_qos_destMac_mapping_cmd,
	"qos-destMac-mapping ethport <1-64> dest-mac <H.H.H> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos DA MAC mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the destination mac address\n"
	"input the mac address\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	/*int result = 0;*/
	LONG lRet;
	unsigned char port_number;
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] = {0,0,0,0,0,0};
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
	}

    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 		
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosDMacMapping(phyPonId, OnuIdx, port_number, MacAddr, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}

	return( CMD_SUCCESS );
}

DEFUN (
	onu_qos_srcMac_mapping_func,
	onu_qos_srctMac_mapping_cmd,
	"qos-srcMac-mapping ethport <1-64> src-mac <H.H.H> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos SA MAC mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the source mac address\n"
	"input the mac address\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	/*int result = 0;*/
	LONG lRet;
	unsigned char port_number;
	unsigned char MacAddr[BYTES_IN_MAC_ADDRESS] = {0,0,0,0,0,0};
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		{
		vty_out(vty, "  %d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
		}
	*/
	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx error. \r\n",ulSlot, ulPort);
		return CMD_WARNING;	
	}

    OnuIdx = ulOnuid - 1;
    if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	if ( GetMacAddr( ( CHAR* ) argv[ 1 ], MacAddr ) != VOS_OK )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );
/*
	vty_out(vty, "port_number = %d\r\n",  port_number );
	vty_out(vty, "mapping priority = %d\r\n",  mapping_priority );
	vty_out(vty, "mapping_queue = %d\r\n",  mapping_queue );
	vty_out(vty, "Match_condition = %d\r\n",  Match_condition );
*/
	if(SetOnuEthPortQosSMacMapping(phyPonId, OnuIdx, port_number, MacAddr, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %%  %% Executing error\r\n");
		return	( CMD_WARNING );
	}

	return( CMD_SUCCESS );
}


DEFUN (
	onu_qos_priority_mapping_func,
	onu_qos_priority_mapping_cmd,
	"qos-pri-mapping ethport <1-64> pri <0-7> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos priority mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the initial priority\n"
	"input the priority\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned char priority;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	

	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}
	
	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
    
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	priority = ( unsigned char ) VOS_AtoL( argv[ 1 ] );
	
	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosPriMapping(phyPonId, OnuIdx, port_number, priority, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}

DEFUN (
	onu_qos_vlanId_mapping_func,
	onu_qos_vlanId_mapping_cmd,
	"qos-vid-mapping ethport <1-64> vid <1-4094> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos vlan id mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the vlan id\n"
	"input vlan id\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int vlanId;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
    
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	vlanId = ( ULONG ) VOS_AtoL( argv[ 1 ] );
	
	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosVidMapping(phyPonId, OnuIdx, port_number, vlanId, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %%  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}

DEFUN (
	onu_qos_ethtype_mapping_func,
	onu_qos_ethtype_mapping_cmd,
	"qos-ethtype-mapping ethport <1-64> ether-type <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos ether type mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the ether type\n"
	"input ether type\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int  ethType;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx%d%d/%d error. \r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;	
	}

    OnuIdx = ulOnuid - 1；
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
    
	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	ethType = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );

	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosEthTypeMapping(phyPonId, OnuIdx, port_number, ethType, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}



DEFUN (
	onu_qos_destIp_mapping_func,
	onu_qos_destIp_mapping_cmd,
	"qos-destIp-mapping ethport <1-64> dest-Ip <A.B.C.D> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos destionation ip address mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the destination ip address\n"
	"input ip address\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queue id is 0-3\n"
	"input the queue id,for PMC onu,the queue id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int ipAddr;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	

	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	if( V2R1_GetLongFromIpdotstring( argv[1], &ipAddr )  != ROK )
	{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
	}

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosDipMapping(phyPonId, OnuIdx, port_number, ipAddr, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %%  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}


DEFUN (
	onu_qos_srcIp_mapping_func,
	onu_qos_srcIp_mapping_cmd,
	"qos-srcIp-mapping ethport <1-64> src-Ip <A.B.C.D> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos source ip address mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the source ip address\n"
	"input ip address\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int ipAddr;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	if( V2R1_GetLongFromIpdotstring( argv[1], &ipAddr )  != ROK )
		{
		vty_out( vty, " %% IpAddr Parameter err\r\n");
		return( CMD_WARNING);
		}
	
	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 	
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosSipMapping(phyPonId, OnuIdx, port_number, ipAddr, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}


DEFUN (
	onu_qos_Iptype_mapping_func,
	onu_qos_Iptype_mapping_cmd,
	"qos-Iptype-mapping ethport <1-64> ip-type <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos mapping\n"
	"set onu qos ip protocol type mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the ip-protocol type\n"
	"any ip protocol type\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int ipType;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );

	if (!VOS_StrCmp((CHAR *)argv[1], "IP"))	
		ipType = 0x800;
	else if (!VOS_StrCmp((CHAR *)argv[1], "ICMP"))
		ipType = 1;
	else if (!VOS_StrCmp((CHAR *)argv[1], "IGMP"))
		ipType = 2;
	else ipType = ( int ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosIpTypeMapping(phyPonId, OnuIdx, port_number, ipType, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %%  Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}


DEFUN (
	onu_qos_Iptos_mapping_func,
	onu_qos_Iptos_mapping_cmd,
	"qos-Ipv4tos-mapping ethport <1-64> ip-tos <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos ipv4 tos mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the ip tos(dscp)\n"
	"tos(dscp) value\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int tos;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid< (CLI_EPON_ONUMIN+1)) || (ulOnuid> (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	tos = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosIpv4DSCPMapping(phyPonId, OnuIdx, port_number, tos, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}



DEFUN (
	onu_qos_Ipprecedence_mapping_func,
	onu_qos_Ipprecedence_mapping_cmd,
	"qos-Ipv6precedence-mapping ethport <1-64> ip-precedence <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos ipv6 precedence mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the ipv6 precedence\n"
	"precedence value\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int precedence;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	precedence = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosIpv6PrecMapping(phyPonId, OnuIdx, port_number,  precedence, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}

DEFUN (
	onu_qos_L4srcport_mapping_func,
	onu_qos_L4srcport_mapping_cmd,
	"qos-srcport-mapping ethport <1-64> src-port <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos source port mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the L4 source port\n"
	"the L4 source port\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int port;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	port = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosIpSrcPortMapping(phyPonId, OnuIdx, port_number,  port, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}


DEFUN (
	onu_qos_l4dstport_mapping_func,
	onu_qos_l4dstport_mapping_cmd,
	"qos-dstport-mapping ethport <1-64> dst-port <0-65535> mapping-priority [<0-7>|original] mapping-queue <0-7> match-condition [0|1|2|3|4|5|6|7]",
	"set onu qos destination port mapping\n"
	"the ether port number\n"
	"input the port number\n"
	"the L4 destination port\n"
	"the L4 destination port\n"
	"the priority mapping to\n"
	"input the priority\n"
	"indicates that the priority field isn't re-marked,but this isn't supported by PMC onu\n"
	"the queue mapping to,for PMC onu,the queu id is 0-3\n"
	"input the queue id,for PMC onu,the queu id is 0-3\n"
	"macth condition\n"
	"never match\n"
	"field equal to vaule\n"
	"field not equal to value\n"
	"field less than or equal to value\n"
	"field greater than or equal to value\n"
	"true if field exists(value ignored)\n"
	"true if field does not exists(value ignored)\n"
	"always match\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
   	INT16 phyPonId = 0;
   	INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;
	unsigned int port;
	unsigned char mapping_priority;
	unsigned char mapping_queue;
	int Match_condition;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
		*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );
	port = ( ULONG ) VOS_AtoL( argv[ 1 ] );

	if ( !( VOS_StrCmp( argv[ 2 ] , "original" ) ) )
		mapping_priority = 255;
	else 
		mapping_priority = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	mapping_queue = ( unsigned char ) VOS_AtoL( argv[ 3 ] );
	Match_condition = ( int ) VOS_AtoL( argv[ 4 ] );

	if(SetOnuEthPortQosIpDestPortMapping(phyPonId, OnuIdx, port_number,  port, mapping_priority, mapping_queue , Match_condition) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );
	
}


DEFUN (
	onu_qos_mapping_clear_func,
	onu_qos_mapping_clear_cmd,
	"clear qos-mapping ethport <1-64>",
	"clear onu qos mapping\n"
	"clear onu qos mapping\n"
	"the ether port number\n"
	"input the port number\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
    INT16 phyPonId = 0;
    INT16 OnuIdx = 0;
	ULONG ulIfIndex=0;
	LONG lRet;
	unsigned char port_number;

	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}	
	/*
	result = GetPonPortOperStatus(  phyPonId );
	if(( result == PONPORT_DOWN )|| ( result == PONPORT_UNKNOWN )||(result == PONPORT_INIT)||( result == RERROR )) 
	*/
	if( PonPortIsWorking(phyPonId) != TRUE )
	{
		vty_out(vty, "  pon%d/%d is not Working \r\n",ulSlot,ulPort);
		return CMD_WARNING;
	}

	/*ulOnuid =  ( ULONG ) VOS_AtoL( argv[ 0 ] );*/
	if ((ulOnuid < (CLI_EPON_ONUMIN+1)) || (ulOnuid > (CLI_EPON_ONUMAX+1)))
	{
		vty_out( vty, "  %% onuidx %d/%d/%d error. \r\n",ulSlot, ulPort, ulOnuid);
		return CMD_WARNING;	
	}
	
    OnuIdx = ulOnuid - 1;
	if(ThisIsValidOnu( phyPonId, OnuIdx)  != ROK )
	{
		vty_out( vty, "  %% onu%d/%d/%d not exist.\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}
	if(GetOnuOperStatus(phyPonId, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "  %% onu%d/%d/%d is off-line\r\n",ulSlot, ulPort,ulOnuid);
		return CMD_WARNING;  
	}

	port_number = ( unsigned char ) VOS_AtoL( argv[ 0 ] );

	if(DelOnuEthPortQosRule(phyPonId, OnuIdx, port_number) != ROK )
	{
		vty_out(vty, "  %% Executing error\r\n");
		return	( CMD_WARNING );
	}
	else
        return( CMD_SUCCESS );

}
#endif

LONG CT_RMan_ONU_QoS_Init()
{
	install_element ( ONU_CTC_NODE, &show_policy_map_cmd);
       install_element ( ONU_CTC_NODE, &set_service_policy_cmd);
	install_element ( ONU_CTC_NODE, &undo_service_policy_cmd);
	install_element ( ONU_CTC_NODE, &set_policy_map_cmd);
	install_element ( ONU_CTC_NODE, &undo_policy_map_cmd);
	install_element ( ONU_CTC_NODE, &set_class_match_cmd);
	install_element ( ONU_CTC_NODE, &undo_class_match_cmd);

	/* added  by chenfj 2007-7-19 */
#ifdef  ONU_QOS_CLASSIFICATION_MARKING
	install_element ( ONU_CTC_NODE, &onu_qos_destMac_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_srctMac_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_priority_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_vlanId_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_ethtype_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_destIp_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_srcIp_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_Iptype_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_Iptos_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_Ipprecedence_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_L4srcport_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_l4dstport_mapping_cmd );
	install_element ( ONU_CTC_NODE, &onu_qos_mapping_clear_cmd );
#endif

	return VOS_OK;
}
#endif


