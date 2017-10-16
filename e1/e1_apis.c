
#ifdef __cplusplus
extern"C"{
#endif

#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"

#include "tdm_apis.h"
#include "tdm_comm.h"
#include "onu/ExtBoardType.h"

#include "e1_apis.h"
#include "E1_MIB.h"

extern UCHAR g_tdmDebugFlag;

/* 如果链接ONU的PON口(此PON口号为参数输入)被保护(即和另外一个PON口关联了)，
就把物理索引号改成PON口号较小的ONU物理索引号，即返回值 */
extern int swapToMinOnuIdx( ULONG onuIdx );

/* 倒置ONU物质索引号，根据两个关联的被保护的PON口 */
extern int reverseSwapOnuIdx( ULONG onuIdx );


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1LinkIdx;
}PACKED e1LinkTable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1LinkIdx;
	ULONG setval;
}PACKED e1LinkTable_set_pdu;

typedef struct{
	UCHAR  tableIdx;
	UCHAR  leafIdx;
	UCHAR  devIdx;
	UCHAR  brdIdx;
	UCHAR  e1portIdx;

	ULONG  onuDevId;				/* (RW)ONU设备索引号 */
	UCHAR  onuE1SlotId;				/* (RW)ONU上的E1板卡的槽位号 0~6 */
	UCHAR  onuE1Id;					/* (RW)ONU上的E1板卡上的E1口索引号 0~3 */

	UCHAR  eponE1LocalEnable;		/* (RW) 1, 使能;  2, 不使能 */
	UCHAR  eponE1Description[E1_DESCRIPTION_MAX];/* (RW) 32Bytes */
}PACKED e1LinkTable_rowset_pdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
}PACKED e1PortTable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
	USHORT setval;
}PACKED e1PortTable_set_pdu;

typedef struct{
	UCHAR  tableIdx;
	UCHAR  leafIdx;
	UCHAR  devIdx;
	UCHAR  brdIdx;
	UCHAR  e1portIdx;

	USHORT eponE1PortAlarmMask;		/* (RW) */
	UCHAR  eponE1Loop;		        /* (RW) 1-4 */
	UCHAR  eponE1TxClock;			/* (RW)range: 0 - 3 */
}PACKED e1PortTable_rowset_pdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR vlanIdx;
}PACKED e1VlanTable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR vlanIdx;
	USHORT setval;
}PACKED e1VlanTable_set_pdu;

typedef struct{
	UCHAR  tableIdx;
	UCHAR  leafIdx;
	UCHAR  devIdx;
	UCHAR  brdIdx;
	UCHAR  vlanIdx;

	UCHAR  eponVlanEnable;		/* (RW) */
	UCHAR  eponVlanPri;	        /* (RW) */
	USHORT eponVlanId;			/* (RW)range */
}PACKED e1VlanTable_rowset_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR noContent;
	ULONG onuIndex;
}PACKED onuE1Info_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR noContent;
	UCHAR type;
}PACKED allE1Info_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR noContent;
}PACKED maxFrmGap_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR noContent;
	UCHAR eth2E1TxBuf[MAX_SGIF_E1 * MAX_SGIF_ID];
}PACKED eth2E1TxBuf_set_pdu;

/* 打印板间发送消息 */
extern void pktDataPrintf( uchar_t *pOctBuf, ulong_t bufLen );
static STATUS printTxBoardMsg(char *buf, ULONG len)
{
	/*int i;*/
	if( (buf == NULL) || (len == 0) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( len > 1500 )
	{
		sys_console_printf("E1 BOARD TX MSG:len=%d err\r\n", len);
		return VOS_ERROR;
	}
	if( debugE1 & (E1_ERROR_INFO | E1_TX_BOARD_MSG) )
		sys_console_printf("E1 BOARD TX MSG:len=%d\r\n", len);
	
	if( (debugE1 & (E1_ERROR_INFO | E1_TX_BOARD_MSG)) == (E1_ERROR_INFO | E1_TX_BOARD_MSG) )
	{
		/*for (i = 0; i < len; i++)
		{
			if ( 0 == (i % 16) )
			{
				sys_console_printf("\r\n");
			}
			sys_console_printf("%02x ", buf[i]);
		}
		sys_console_printf("\r\n");*/
		pktDataPrintf( buf, len );	/* modified by xieshl 20100202 */
	}
	return VOS_OK;
}

/* 打印板间接收消息 */
static STATUS printRxBoardMsg(char *buf, ULONG len)
{
	/*int i;*/
	if( (buf == NULL) || (len == 0) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( len > 1500 )
	{
		sys_console_printf("E1 BOARD RX MSG:len=%d err\r\n", len);
		return VOS_ERROR;
	}
	if( debugE1 & (E1_ERROR_INFO | E1_RX_BOARD_MSG) )
		sys_console_printf("E1 BOARD RX MSG:len=%d\r\n", len);
	
	if( (debugE1 & (E1_ERROR_INFO | E1_RX_BOARD_MSG)) == (E1_ERROR_INFO | E1_RX_BOARD_MSG) )
	{
		/*for (i = 0; i < len; i++)
		{
			if ( 0 == (i % 16) )
			{
				sys_console_printf("\r\n");
			}
			sys_console_printf("%02x ", buf[i]);
		}
		sys_console_printf("\r\n");*/
		pktDataPrintf( buf, len );
	}
	return VOS_OK;
}


static STATUS process_rtn_conf_e1LinkTable_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( pdata )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t*)pdata;
		const USHORT length = 10;

		if( (pdu->type == MSG_TYPE_CONFIG)
			&& ( pdu->subType == MSG_SUBTYPE_SET_RES || pdu->subType == MSG_SUBTYPE_SETROW_RES )
			&& (len >= length)
			&& (pdu->msgCode == 0) )
		{
			/*E1_ERROR_INFO_PRINT("ENTER process_rtn_conf_e1LinkTable_set()\r\n");*/
			rc = VOS_OK;
		}
	}
	else
	{
		VOS_ASSERT(0);
	}
	return rc;
}

static STATUS process_rtn_conf_e1LinkTable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( (pdata != NULL) && (ret_ptr != NULL) )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t *)pdata;
		const USHORT length = 8 + sizeof(e1LinkTable_row_entry) + 2/*uchar_t node; uchar_t leaf*/;

		if( (pdu->type == MSG_TYPE_CONFIG )
			&& ( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) 
			&& (len >= length)
			&& (pdu->msgCode == 0)
			&& (pdu->msgData[0] == E1LINKTABLE_INDEX) )
		{
			/*E1_ERROR_INFO_PRINT("@@@@ ENTER    process_rtn_conf_e1LinkTable_get(), len=%d, sizeof( e1LinkTable_row_entry )=%d\r\n", len, sizeof( e1LinkTable_row_entry ));*/

			VOS_MemCpy( ret_ptr, pdu->msgData + 2,  sizeof( e1LinkTable_row_entry ) );

			/*TODO 加\0*/
			rc = VOS_OK;
		}
		/*else
		{
			rc = VOS_ERROR;
		}*/
	}
	else
	{
		VOS_ASSERT(0);
	}
	return rc;
}

static STATUS process_rtn_conf_e1PortTable_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( pdata != NULL )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t*)pdata;
		const USHORT length = 10;

		if( (pdu->type == MSG_TYPE_CONFIG)
			&& (pdu->subType == MSG_SUBTYPE_SET_RES || pdu->subType == MSG_SUBTYPE_SETROW_RES)
			&& (len >= length)
			&& (pdu->msgCode == 0) )
		{
			/*E1_ERROR_INFO_PRINT("ENTER process_rtn_conf_e1PortTable_set()\r\n");*/
			rc = VOS_OK;
		}
	}
	else
	{
		VOS_ASSERT(0);
	}
	return rc;
}

static STATUS process_rtn_conf_e1PortTable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( (pdata != NULL) && (ret_ptr != NULL) )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t*)pdata;
		const USHORT length = 8 + sizeof(e1PortTable_row_entry) + 2/*uchar_t node; uchar_t leaf*/;

		if( (pdu->type == MSG_TYPE_CONFIG)
			&& ( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) 
			&& (len >= length)
			&& (pdu->msgCode == 0)
			&& (pdu->msgData[0] == E1PORTTABLE_INDEX_new) )
		{
			/*E1_ERROR_INFO_PRINT("@@@@ ENTER    process_rtn_conf_e1PortTable_get()\r\n");*/

			VOS_MemCpy( ret_ptr, pdu->msgData + 2,  sizeof( e1PortTable_row_entry ) );
			/*for (i = 0; i < 6; i++)
			{
			    E1_ERROR_INFO_PRINT("0x%02x", *(char *)(pdu->msgData + 2 + i) );
			}
			E1_ERROR_INFO_PRINT("\r\n");*/
			rc = VOS_OK;
		}
		/*else
		{
			rc = VOS_ERROR;
		}*/
	}
	else
	{
		VOS_ASSERT(0);
	}

	return rc;
}

static STATUS process_rtn_conf_e1VlanTable_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( pdata != NULL )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t*)pdata;
		const USHORT length = 10;

		if( pdu->type == MSG_TYPE_CONFIG 
			&& ( pdu->subType == MSG_SUBTYPE_SET_RES || pdu->subType == MSG_SUBTYPE_SETROW_RES )
			&& len >= length 
			&& pdu->msgCode == 0 )
		{
			/*E1_ERROR_INFO_PRINT("ENTER process_rtn_conf_e1VlanTable_set()\r\n");*/
			rc = VOS_OK;
		}
	}
	else
	{
		VOS_ASSERT(0);
	}

	return rc;
}

static STATUS process_rtn_conf_e1VlanTable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( (pdata != NULL) && (ret_ptr != NULL) )
	{
		tdm_pdu_t *pdu = (tdm_pdu_t*)pdata;
		const USHORT length = 8 + sizeof(e1VlanTable_row_entry) + 2/*uchar_t node; uchar_t leaf*/;

		if( pdu->type == MSG_TYPE_CONFIG 
			&& ( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) 
			&& len >= length 
			&& pdu->msgCode == 0
			&& pdu->msgData[0] == E1VLANTABLE_INDEX )
		{
			/*E1_ERROR_INFO_PRINT("@@@@ ENTER    process_rtn_conf_e1VlanTable_get()\r\n");*/

			VOS_MemCpy( ret_ptr, pdu->msgData + 2,  sizeof( e1VlanTable_row_entry ) );
			rc = VOS_OK;
		}
		/*else
		{
			rc = VOS_ERROR;
		}*/
	}
	else
	{
		VOS_ASSERT(0);
	}

	return rc;
}

void init_e1_process_function(void)
{
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1LINKTABLE_INDEX , process_rtn_conf_e1LinkTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1LINKTABLE_INDEX , process_rtn_conf_e1LinkTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1LINKTABLE_INDEX , process_rtn_conf_e1LinkTable_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1LINKTABLE_INDEX , process_rtn_conf_e1LinkTable_set );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX_new , process_rtn_conf_e1PortTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX_new , process_rtn_conf_e1PortTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX_new , process_rtn_conf_e1PortTable_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX_new , process_rtn_conf_e1PortTable_set );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1VLANTABLE_INDEX , process_rtn_conf_e1VlanTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1VLANTABLE_INDEX , process_rtn_conf_e1VlanTable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1VLANTABLE_INDEX , process_rtn_conf_e1VlanTable_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1VLANTABLE_INDEX , process_rtn_conf_e1VlanTable_set );
}

extern USHORT buildPduHead( tdm_pdu_t *ppdu,  int msgType, int msgSubType );


STATUS rpc_tdm_e1LinkTable_get( ULONG *idxs, e1LinkTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char   *pRecv = NULL;
	e1LinkTable_get_pdu * p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p            = (e1LinkTable_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1LinkIdx = idxs[2];
	p->tableIdx  = E1LINKTABLE_INDEX;/* 由于SW端表的序号为10，而TDM段为1，差9 */
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1LinkTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, sizeof(e1LinkTable_row_entry));
		/*E1_ERROR_INFO_PRINT("rpc_tdm_e1LinkTable_get()::tdmCommSendMsg()\r\n");*/
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1LINKTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			E1_ERROR_INFO_PRINT(("#@#tdm_e1LinkTable_get()::getRecvHandler() failed!!!!!!\r\n"));
			rc = VOS_ERROR;
		}
		/*E1_ERROR_INFO_PRINT("#@#rpc_tdm_e1LinkTable_get()::getRecvHandler() successed!!!!!!\r\n");*/
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("rpc_tdm_e1LinkTable_get()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS rpc_tdm_e1LinkTable_getNext( ULONG *idxs, e1LinkTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1LinkTable_get_pdu * p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p            = (e1LinkTable_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1LinkIdx = idxs[2];
	p->tableIdx  = E1LINKTABLE_INDEX;
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1LinkTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if ( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, sizeof(e1LinkTable_row_entry));
		rc = VOS_OK;
		/*E1_ERROR_INFO_PRINT("rpc_tdm_e1LinkTable_getNext()::tdmCommSendMsg()   OK!!\r\n");*/

		if ( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1LINKTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			rc = VOS_ERROR;
			E1_ERROR_INFO_PRINT(("rpc_tdm_e1LinkTable_getNext()::getRecvHandler()   ERROR!!\r\n"));
		}

		tdmCommMsgFree( pRecv );
	}

	return rc;
}

STATUS rpc_tdm_e1LinkTable_set( UCHAR leafIdx, ULONG *idxs, ULONG setval , UCHAR *description)
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1LinkTable_set_pdu *p = NULL;

	tdm_pdu_t *pdu = (tdm_pdu_t *)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p            = (e1LinkTable_set_pdu *)pdu->msgData;/* pdu和*p还是空的*/

	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1LinkIdx = idxs[2];
	p->tableIdx  = E1LINKTABLE_INDEX;
	p->leafIdx   = leafIdx;
	if (LEAF_eponE1Description == leafIdx)
	{
		if (NULL == description)
		{
			return VOS_ERROR;
		}
		/* Description每次都发32Bytes，不足32Bytes的，补0 */
		memset((UCHAR *)&p->setval, 0, E1_DESCRIPTION_MAX);

		if (strlen(description) < E1_DESCRIPTION_MAX)
		{
			memcpy((UCHAR *)&p->setval, description, strlen(description));
		} 
		else
		{
			E1_ERROR_INFO_PRINT(("rpc_tdm_e1LinkTable_set()::strlen(description)=%d    error!\r\n", strlen(description)));
		}

		/*E1_ERROR_INFO_PRINT("Send Description=%s, strlen=%d, sizeof(p->setval)=%d\r\n", (UCHAR *)&p->setval, strlen((UCHAR *)&p->setval), sizeof(p->setval));*/
		pdulen = headlen + sizeof( e1LinkTable_set_pdu ) - sizeof(ULONG) + E1_DESCRIPTION_MAX;
	}
	else
	{
		p->setval = setval;
		pdulen    = headlen + sizeof( e1LinkTable_set_pdu );
	}

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1LINKTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, NULL) )
		{
			rc = VOS_ERROR;
		}
		tdmCommMsgFree(pRecv);
	}
	return rc;
}

STATUS rpc_tdm_e1LinkTable_rowset( ULONG  *idxs,
							       ULONG  onuDevId,
							       UCHAR  onuE1SlotId,
							       UCHAR  onuE1Id,
							       UCHAR  eponE1LocalEnable,
							       UCHAR  *eponE1Description )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1LinkTable_rowset_pdu *p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();

	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	p            = (e1LinkTable_rowset_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1portIdx = idxs[2];
	p->tableIdx  = E1LINKTABLE_INDEX;
	p->leafIdx   = 0;

	p->onuDevId    = onuDevId;
	p->onuE1SlotId = onuE1SlotId;
	p->onuE1Id     = onuE1Id;

	p->eponE1LocalEnable = eponE1LocalEnable;
	/* Description每次都发32Bytes，不足32Bytes的，补0 */
	VOS_MemZero(p->eponE1Description, E1_DESCRIPTION_MAX);

	if (NULL != eponE1Description)
	{
		if (VOS_StrLen(eponE1Description) < E1_DESCRIPTION_MAX)
		{
			VOS_StrCpy(p->eponE1Description, eponE1Description);
		}
		else
		{
			E1_ERROR_INFO_PRINT(("rpc_tdm_e1LinkTable_rowset()::strlen(deeponE1Descriptionscription)=%d    error!\r\n", strlen(eponE1Description)));
		}
	}

	pdulen = headlen + sizeof( e1LinkTable_rowset_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1LINKTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;

		tdmCommMsgFree( pRecv );
	}

	return rc;
}



STATUS tdm_e1LinkTable_get( ULONG *idxs, e1LinkTable_row_entry *pEntry )
{
	if ( (NULL == idxs) || (NULL == pEntry) )
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_get()    idxs=NULL or pEntry=NULL   error!!\r\n"));
		return VOS_ERROR;
	}

	if (VOS_OK != rpc_tdm_e1LinkTable_get( idxs, pEntry ))
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_get()::rpc_tdm_e1LinkTable_get()   error!!\r\n"));
		return VOS_ERROR;
	}

	if ( (pEntry->onuDevId != 0xFFFFFFFF) && (pEntry->onuDevId != 0) )
	{
		pEntry->onuDevId = reverseSwapOnuIdx(pEntry->onuDevId);
	}

	return VOS_OK;
}

STATUS tdm_e1LinkTable_getNext( ULONG *idxs, e1LinkTable_row_entry *pEntry )
{
	if ( (NULL == idxs) || (NULL == pEntry) )
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_getNext()    idxs=NULL or pEntry=NULL   error!!\r\n"));
		return VOS_ERROR;
	}

	if (VOS_OK != rpc_tdm_e1LinkTable_getNext( idxs, pEntry ))
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_getNext()::rpc_tdm_e1LinkTable_getNext()   error!!\r\n"));
		return VOS_ERROR;
	}

	if ( (pEntry->onuDevId != 0xFFFFFFFF) && (pEntry->onuDevId != 0) )
	{
		pEntry->onuDevId = reverseSwapOnuIdx(pEntry->onuDevId);
	}

	return VOS_OK;
}

STATUS tdm_e1LinkTable_set( UCHAR leafIdx, ULONG *idxs, ULONG setval , UCHAR *description)
{
	if (leafIdx == LEAF_eponE1PortOnuDevId)
	{
		if ( (setval != 0xFFFFFFFF) && (setval != 0) )
		{
			setval = swapToMinOnuIdx(setval);
		}
	}

	if (VOS_OK != rpc_tdm_e1LinkTable_set( leafIdx, idxs, setval , description))
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_set()::rpc_tdm_e1LinkTable_set()   error!!\r\n"));
		return VOS_ERROR;
	}

	return VOS_OK;
}

STATUS tdm_e1LinkTable_rowset( ULONG  *idxs,
							  ULONG  onuDevId,
							  UCHAR  onuE1SlotId,
							  UCHAR  onuE1Id,
							  UCHAR  eponE1LocalEnable,
							  UCHAR  *eponE1Description )
{
	if ( (onuDevId != 0xFFFFFFFF) && (onuDevId != 0) )
	{
		onuDevId = swapToMinOnuIdx(onuDevId);
		/*E1_ERROR_INFO_PRINT("tdm_e1LinkTable_rowset()    swapToMinOnuIdx(onuDevId)=%d \r\n", onuDevId);*/
	}

	if (VOS_OK != rpc_tdm_e1LinkTable_rowset( idxs, onuDevId, onuE1SlotId, onuE1Id, eponE1LocalEnable, eponE1Description ))
	{
		E1_ERROR_INFO_PRINT(("tdm_e1LinkTable_rowset()::rpc_tdm_e1LinkTable_rowset()   error!!\r\n"));
		return VOS_ERROR;
	}

	return VOS_OK;
}

STATUS tdm_e1PortTable_get( ULONG *idxs, e1PortTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1PortTable_get_pdu * p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p            = (e1PortTable_get_pdu*)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1portIdx = idxs[2];
	p->tableIdx  = E1PORTTABLE_INDEX_new;/* 由于SW端表的序号为10，而TDM段为1，差9 */
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1PortTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_e1PortTable_get()::tdmCommSendMsg()\r\n");*/
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX_new ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			E1_ERROR_INFO_PRINT(("#@#tdm_e1PortTable_get()::getRecvHandler() failed!!!!!!\r\n"));
			rc = VOS_ERROR;
		}
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_e1PortTable_get()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS tdm_e1PortTable_getNext( ULONG *idxs, e1PortTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1PortTable_get_pdu * p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p            = (e1PortTable_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1portIdx = idxs[2];
	p->tableIdx  = E1PORTTABLE_INDEX_new;
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1PortTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if ( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;
		/*E1_ERROR_INFO_PRINT("tdm_e1PortTable_getNext()::tdmCommSendMsg()   OK!!\r\n");*/

		if ( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX_new ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			rc = VOS_ERROR;
			E1_ERROR_INFO_PRINT(("tdm_e1PortTable_getNext()::getRecvHandler()   ERROR!!\r\n"));
		}

		tdmCommMsgFree( pRecv );
	}

	return rc;
}

STATUS tdm_e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1PortTable_set_pdu *p = NULL;

	tdm_pdu_t *pdu = (tdm_pdu_t *)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p            = (e1PortTable_set_pdu *)pdu->msgData;/* pdu和*p还是空的*/

	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1portIdx = idxs[2];
	p->tableIdx  = E1PORTTABLE_INDEX_new;
	p->leafIdx   = leafIdx;
	p->setval    = setval;
	/*E1_ERROR_INFO_PRINT("tdm_e1PortTable_set()::setval=%ld, 0x%08lx\r\n",p->setval, p->setval);*/
	pdulen       = headlen + sizeof( e1PortTable_set_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX_new ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
		{
		       E1_ERROR_INFO_PRINT(("tdm_e1PortTable_set()::getRecvHandler()   error!\r\n"));
			rc = VOS_ERROR;
		}
		tdmCommMsgFree(pRecv);
	}
	return rc;
}

STATUS tdm_e1PortTable_rowset( ULONG  *idxs,
							   USHORT  eponE1PortAlarmMask,
							   UCHAR  eponE1Loop,
							   UCHAR  eponE1TxClock )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1PortTable_rowset_pdu *p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();

	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	p            = (e1PortTable_rowset_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->e1portIdx = idxs[2];
	p->tableIdx  = E1PORTTABLE_INDEX_new;
	p->leafIdx   = 0;

	p->eponE1PortAlarmMask = eponE1PortAlarmMask;
	p->eponE1Loop          = eponE1Loop;
	p->eponE1TxClock       = eponE1TxClock;

	pdulen = headlen + sizeof( e1PortTable_rowset_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX_new ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;

		tdmCommMsgFree( pRecv );
	}

	return rc;
}

STATUS tdm_e1VlanTable_get( ULONG *idxs, e1VlanTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1VlanTable_get_pdu *p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p            = (e1VlanTable_get_pdu*)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->vlanIdx = idxs[2];
	p->tableIdx  = E1VLANTABLE_INDEX;/* 由于SW端表的序号为10，而TDM段为1，差9 */
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1VlanTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_e1VlanTable_get()::tdmCommSendMsg()\r\n");*/
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1VLANTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			E1_ERROR_INFO_PRINT(("#@#tdm_e1VlanTable_get()::getRecvHandler() failed!!!!!!\r\n"));
			rc = VOS_ERROR;
		}
		/*E1_ERROR_INFO_PRINT("#@#tdm_e1VlanTable_get()::getRecvHandler() successed!!!!!!\r\n");*/
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_e1VlanTable_get()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS tdm_e1VlanTable_getNext( ULONG *idxs, e1VlanTable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1VlanTable_get_pdu * p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p            = (e1VlanTable_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->vlanIdx = idxs[2];
	p->tableIdx  = E1VLANTABLE_INDEX;
	p->leafIdx   = 0;
	pdulen       = headlen + sizeof( e1VlanTable_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if ( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;
		/*E1_ERROR_INFO_PRINT("tdm_e1VlanTable_getNext()::tdmCommSendMsg()   OK!!\r\n");*/

		if ( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1VLANTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, pEntry ) )
		{
			rc = VOS_ERROR;
			E1_ERROR_INFO_PRINT(("tdm_e1VlanTable_getNext()::getRecvHandler()   ERROR!!\r\n"));
		}

		tdmCommMsgFree( pRecv );
	}

	return rc;
}

STATUS tdm_e1VlanTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1VlanTable_set_pdu *p = NULL;

	tdm_pdu_t *pdu = (tdm_pdu_t *)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	/*E1_ERROR_INFO_PRINT("@@@@ENTER :: tdm_e1VlanTable_set(), sizeof(e1VlanTable_set_pdu)=%d\r\n", sizeof(e1VlanTable_set_pdu));*/

	headlen      = buildPduHead( pdu, MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p            = (e1VlanTable_set_pdu *)pdu->msgData;/* pdu和*p还是空的*/

	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->vlanIdx = idxs[2];
	p->tableIdx  = E1VLANTABLE_INDEX;
	p->leafIdx   = leafIdx;
	p->setval    = setval;
	/*E1_ERROR_INFO_PRINT("@@@@tdm_e1VlanTable_set()::setval=%ld, 0x%08lx\r\n",p->setval, p->setval);*/
	pdulen       = headlen + sizeof( e1VlanTable_set_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1VLANTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
		{
			rc = VOS_ERROR;
		}
		tdmCommMsgFree(pRecv);
	}
	return rc;
}

STATUS tdm_e1VlanTable_rowset( ULONG  *idxs,
							   UCHAR  eponVlanEnable,
							   UCHAR  eponVlanPri,
							   USHORT eponVlanId )
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	e1VlanTable_rowset_pdu *p = NULL;

	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc();

	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu, MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	p            = (e1VlanTable_rowset_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->vlanIdx = idxs[2];
	p->tableIdx  = E1VLANTABLE_INDEX;
	p->leafIdx   = 0;

	p->eponVlanEnable = eponVlanEnable;
	p->eponVlanPri    = eponVlanPri;
	p->eponVlanId     = eponVlanId;

	pdulen = headlen + sizeof( e1VlanTable_rowset_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		rc = VOS_OK;

		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1VLANTABLE_INDEX ))( pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;

		tdmCommMsgFree( pRecv );
	}

	return rc;
}

STATUS rpc_tdm_getOnuE1Info(ULONG *idxs, ULONG onuIndex, onuE1Info *pOnuE1Info)
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	onuE1Info_get_pdu *p = NULL;
	tdm_pdu_t *pdu = NULL, *receivePdu = NULL;
	onuE1Info *pOnuE1InfoTemp;

	if (NULL == pOnuE1Info)
	{
		E1_ERROR_INFO_PRINT(("tdm_getOnuE1Info() param error!  pOnuE1Info=NULL\r\n"));
		return rc;
	}

	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu,  MSG_TYPE_CONFIG, SUBMSG_ONU_E1_REQUEST );
	p            = (onuE1Info_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->noContent = 0;
	p->tableIdx  = 0;
	p->leafIdx   = 0;
	p->onuIndex  = onuIndex;
	pdulen       = headlen + sizeof( onuE1Info_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, (USHORT *)&recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_getOnuE1Info()::tdmCommSendMsg()\r\n");*/

		receivePdu = (tdm_pdu_t *)(pRecv + PDU_OFFSET_IN_MSG);

		if (receivePdu->subType != SUBMSG_ONU_E1_RESPONSE  || receivePdu->msgCode != 0)
		{
			E1_ERROR_INFO_PRINT(("tdm_getOnuE1Info() param error! receive pdu->subType=0x%02x, msgCode=0x%04x \r\n", receivePdu->subType, receivePdu->msgCode));
			tdmCommMsgFree( (char*)pRecv );
			return VOS_ERROR;
		}

		pOnuE1InfoTemp = (onuE1Info *)&receivePdu->msgData[2 + 1 + 1 + 1 + 4];
		recvlen = sizeof(pOnuE1InfoTemp->onuValidE1Count) + sizeof(OnuEachE1Info) * pOnuE1InfoTemp->onuValidE1Count;

		if (recvlen <= sizeof(onuE1Info))
		{
			VOS_MemCpy((char*)pOnuE1Info, (char*)pOnuE1InfoTemp, (ULONG)recvlen);
		} 
		else
		{
			E1_ERROR_INFO_PRINT(("rpc_tdm_getOnuE1Info()::recvlen=%d    too large!\r\n", recvlen));
		}

		rc = VOS_OK;
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_getOnuE1Info()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS tdm_getOnuE1Info(ULONG *idxs, ULONG onuIndex, onuE1Info *pOnuE1Info)
{
	if ( (onuIndex != 0xFFFFFFFF) && (onuIndex != 0) )
	{
		onuIndex = swapToMinOnuIdx(onuIndex);
	}

	if (VOS_OK != rpc_tdm_getOnuE1Info(idxs, onuIndex, pOnuE1Info))
	{
		E1_ERROR_INFO_PRINT(("tdm_getOnuE1Info()::rpc_tdm_getOnuE1Info()   error!\r\n"));
		return VOS_ERROR;
	}

	return VOS_OK;
}

STATUS rpc_tdm_getAllE1Info(ULONG *idxs, UCHAR type, AllE1Info *pAllE1Info)
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	allE1Info_get_pdu *p = NULL;
	tdm_pdu_t *pdu = NULL, *receivePdu = NULL;
	AllE1Info *pAllE1InfoTemp;

	if (NULL == pAllE1Info)
	{
		E1_ERROR_INFO_PRINT(("tdm_getAllE1Info() param error!  pAllE1Info=NULL\r\n"));
		return rc;
	}

	if ( (type >= MAX_SGIF_ID) && (0xFF != type) )
	{
		E1_ERROR_INFO_PRINT(("tdm_getAllE1Info() param error!  type=%d\r\n", type));
		return rc;
	}

	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu, MSG_TYPE_CONFIG, SUBMSG_ALL_E1_REQUEST );
	p            = (allE1Info_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->noContent = 0;
	p->tableIdx  = 0;
	p->leafIdx   = 0;
	p->type      = type;
	pdulen       = headlen + sizeof( allE1Info_get_pdu );
/*
	E1_ERROR_INFO_PRINT("\r\nSend Data: ");
	for (i = 0; i < 32; i++)
	{
		if ( !(i % 8) )
		{
			E1_ERROR_INFO_PRINT("\r\n");
		}
		E1_ERROR_INFO_PRINT( "0x%02x ", *(char *)((char *)pdu + i) );
	}
	E1_ERROR_INFO_PRINT("\r\n");
*/
	printTxBoardMsg((char *)pdu, (ULONG)pdulen);
	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_getAllE1Info()::tdmCommSendMsg()\r\n");*/

		receivePdu = (tdm_pdu_t *)(pRecv + PDU_OFFSET_IN_MSG);

		if (receivePdu->subType != SUBMSG_ALL_E1_RESPONSE  || receivePdu->msgCode != 0)
		{
			E1_ERROR_INFO_PRINT(("tdm_getAllE1Info() param error! receive pdu->subType=0x%02x, msgCode=0x%04lx\r\n", receivePdu->subType, receivePdu->msgCode));
			tdmCommMsgFree( (char*)pRecv );
			return VOS_ERROR;
		}

		pAllE1InfoTemp = (AllE1Info *)&receivePdu->msgData[sizeof(allE1Info_get_pdu) - 1/*type*/];
		VOS_MemCpy((char *)pAllE1Info, (char *)pAllE1InfoTemp, sizeof(AllE1Info));
/*
		E1_ERROR_INFO_PRINT("sizeof(AllE1Info)=%d\r\n", sizeof(AllE1Info));
		for (i = 0; i < sizeof(AllE1Info); i++)
		{
			if (i % 32 == 0)
			{
				E1_ERROR_INFO_PRINT("\r\n");
			}
			E1_ERROR_INFO_PRINT( "%02x ", *(UCHAR *)((UCHAR *)pAllE1InfoTemp + i) );
		}
		E1_ERROR_INFO_PRINT("\r\nEND!!\r\n");
*/
		rc = VOS_OK;
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_getAllE1Info()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS tdm_getAllE1Info(ULONG *idxs, UCHAR type, AllE1Info *pAllE1Info)
{
	UCHAR i, j;

	if (VOS_OK != rpc_tdm_getAllE1Info(idxs, type, pAllE1Info))
	{
		E1_ERROR_INFO_PRINT(("tdm_getAllE1Info()::rpc_tdm_getAllE1Info()   error!\r\n"));
		return VOS_ERROR;
	}

	if (0xFF == type)
	{
		for (i = 0; (i < TDM_FPGA_MAX) && (i < TDM_FPGA_PORT_MAX); i++)
		{
			for (j = 0; j < pAllE1Info->fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				if ( (pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId != 0xFFFFFFFF) && (pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId != 0) )
				{
					pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId = reverseSwapOnuIdx(pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId);
				}
			}
		}
	} 
	else
	{
		for (i = 0; i < pAllE1Info->fpgaE1Info[0].fpgaValidE1Count; i++)
		{
			if ( (pAllE1Info->fpgaE1Info[0].eachE1Info[i].onuDevId != 0xFFFFFFFF) && (pAllE1Info->fpgaE1Info[0].eachE1Info[i].onuDevId != 0) )
			{
				pAllE1Info->fpgaE1Info[0].eachE1Info[i].onuDevId = reverseSwapOnuIdx(pAllE1Info->fpgaE1Info[0].eachE1Info[i].onuDevId);
			}
		}
	}

	return VOS_OK;
}


/* char *maxFrmGapArray是24路E1间隔寄存器的数组，数组元素必须大于等于24 */
STATUS tdm_getMaxFrameGap(ULONG *idxs, char *maxFrmGapArray)
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	maxFrmGap_get_pdu *p = NULL;
	tdm_pdu_t *pdu = NULL, *receivePdu = NULL;

	if (NULL == idxs)
	{
		E1_ERROR_INFO_PRINT(("tdm_getMaxFrameGap() param error!  idxs=NULL\r\n"));
		return rc;
	}

	if (NULL == maxFrmGapArray)
	{
		E1_ERROR_INFO_PRINT(("tdm_getMaxFrameGap() param error!  maxFrmGapArray=NULL\r\n"));
		return rc;
	}

	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu, MSG_TYPE_CONFIG, SUBMSG_MAX_FRAME_GAP_REQUEST );
	p            = (maxFrmGap_get_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->noContent = 0;
	p->tableIdx  = 0;
	p->leafIdx   = 0;

	pdulen       = headlen + sizeof( maxFrmGap_get_pdu );

	printTxBoardMsg((char *)pdu, (ULONG)pdulen);

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_getAllE1Info()::tdmCommSendMsg()\r\n");*/

		receivePdu = (tdm_pdu_t *)(pRecv + PDU_OFFSET_IN_MSG);

		if (receivePdu->subType != SUBMSG_MAX_FRAME_GAP_RESPONSE  || receivePdu->msgCode != 0)
		{
			E1_ERROR_INFO_PRINT(("tdm_getMaxFrameGap() param error! receive pdu->subType=0x%02x, msgCode=0x%04lx\r\n", receivePdu->subType, receivePdu->msgCode));
			tdmCommMsgFree( (char*)pRecv );
			return VOS_ERROR;
		}

		VOS_MemCpy((char *)maxFrmGapArray, (char *)&receivePdu->msgData[sizeof(maxFrmGap_get_pdu)], MAX_E1_PORT_NUM);
		/*
		E1_ERROR_INFO_PRINT("sizeof(AllE1Info)=%d\r\n", sizeof(AllE1Info));
		for (i = 0; i < sizeof(AllE1Info); i++)
		{
		if (i % 32 == 0)
		{
		E1_ERROR_INFO_PRINT("\r\n");
		}
		E1_ERROR_INFO_PRINT( "%02x ", *(UCHAR *)((UCHAR *)pAllE1InfoTemp + i) );
		}
		E1_ERROR_INFO_PRINT("\r\nEND!!\r\n");
		*/
		rc = VOS_OK;
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_getMaxFrameGap()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

STATUS tdm_setEth2E1TxBuffer(ULONG *idxs, char *eth2E1TxBufRegArray)
{
	STATUS rc = VOS_ERROR;
	USHORT pdulen = 0, recvlen = 0, headlen = 0;
	char *pRecv = NULL;
	eth2E1TxBuf_set_pdu *p = NULL;
	tdm_pdu_t *pdu = NULL, *receivePdu = NULL;

	if (NULL == idxs)
	{
		E1_ERROR_INFO_PRINT(("tdm_setEth2E1TxBuffer() param error!  idxs=NULL\r\n"));
		return rc;
	}

	if (NULL == eth2E1TxBufRegArray)
	{
		E1_ERROR_INFO_PRINT(("tdm_setEth2E1TxBuffer() param error!  eth2E1TxBufRegArray=NULL\r\n"));
		return rc;
	}

	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	PDU_CHECK(pdu);

	headlen      = buildPduHead( pdu, MSG_TYPE_CONFIG, SUBMSG_SET_ETH2E1_BUF_REQUEST );
	p            = (eth2E1TxBuf_set_pdu *)pdu->msgData;
	p->devIdx    = idxs[0];
	p->brdIdx    = idxs[1];
	p->noContent = 0;
	p->tableIdx  = 0;
	p->leafIdx   = 0;

	memcpy(p->eth2E1TxBuf, eth2E1TxBufRegArray, MAX_E1_PORT_NUM);

	pdulen       = headlen + sizeof( eth2E1TxBuf_set_pdu );
	/*
	E1_ERROR_INFO_PRINT("\r\nSend Data: ");
	for (i = 0; i < 32; i++)
	{
	if ( !(i % 8) )
	{
	E1_ERROR_INFO_PRINT("\r\n");
	}
	E1_ERROR_INFO_PRINT( "0x%02x ", *(char *)((char *)pdu + i) );
	}
	E1_ERROR_INFO_PRINT("\r\n");
	*/
	printTxBoardMsg((char *)pdu, (ULONG)pdulen);

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char *)pdu, pdulen, &pRecv, &recvlen ) )
	{
		printRxBoardMsg(pRecv + PDU_OFFSET_IN_MSG, recvlen - PDU_OFFSET_IN_MSG);
		/*E1_ERROR_INFO_PRINT("tdm_getAllE1Info()::tdmCommSendMsg()\r\n");*/

		receivePdu = (tdm_pdu_t *)(pRecv + PDU_OFFSET_IN_MSG);

		if (receivePdu->subType != SUBMSG_SET_ETH2E1_BUF_RESPONSE  || receivePdu->msgCode != 0)
		{
			E1_ERROR_INFO_PRINT(("tdm_setEth2E1TxBuffer() param error! receive pdu->subType=0x%02x, msgCode=0x%04lx\r\n", receivePdu->subType, receivePdu->msgCode));
			tdmCommMsgFree( (char*)pRecv );
			return VOS_ERROR;
		}
		/*
		E1_ERROR_INFO_PRINT("sizeof(AllE1Info)=%d\r\n", sizeof(AllE1Info));
		for (i = 0; i < sizeof(AllE1Info); i++)
		{
		if (i % 32 == 0)
		{
		E1_ERROR_INFO_PRINT("\r\n");
		}
		E1_ERROR_INFO_PRINT( "%02x ", *(UCHAR *)((UCHAR *)pAllE1InfoTemp + i) );
		}
		E1_ERROR_INFO_PRINT("\r\nEND!!\r\n");
		*/
		rc = VOS_OK;
		tdmCommMsgFree( (char*)pRecv );
	}
	else
	{
		E1_ERROR_INFO_PRINT(("tdm_setEth2E1TxBuffer()::tdmCommSendMsg()::Failed !!!\r\n"));
	}

	return rc;
}

#endif	/* EPON_MODULE_TDM_SERVICE */

#ifdef __cplusplus
}
#endif







