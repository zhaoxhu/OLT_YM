
#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"

#include "mn_e1.h"
#include "E1_MIB.h"
#include "e1_apis.h"
#include "e1_oam.h"
#include "onu/ExtBoardType.h"
#include "OnuGeneral.h"

/* debug开关 */
ULONG debugE1 = 0;

const unsigned char E1MAC[6] = {0x00, 0x0F, 0xE9, 0x04, 0x03, 0x00};


e1PortTable_t tdmE1PortTable[MAX_SGIF_E1 * MAX_SGIF_ID + 1] = 
{ 
    {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
	{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
	{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0} 
};

/* ONU设备索引是否在范围内 */
STATUS onuDevIdxIsRight(ULONG onuDevId)
{
	ULONG PonPortId, onuId, ponSlotIdx;

	if ( -1 == parseOnuIndexFromDevIdx(onuDevId, &PonPortId, &onuId) )
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsRight()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
		return VOS_ERROR;
	}
/*
	if(ThisIsValidOnu(PonPortId,onuId) != ROK)
	{
		E1_ERROR_INFO_PRINT("onu%d/%d/%d is not exist\r\n", GetCardIdxByPonChip(PonPortId), GetPonPortByPonChip(PonPortId),(onuId+1));
		return VOS_ERROR;
	}
*/
	/*
	chenfj
	对ONU 设备索引的检查，用ThisIsValidOnu() 一个函数就可以了*/
	
	ponSlotIdx = GET_PONSLOT(onuDevId)/*onuDevId / 10000*/;
	if ( (ponSlotIdx < PONCARD_FIRST) || (ponSlotIdx > PONCARD_LAST) )
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsRight()::ponSlotIdx:%d < PONCARD_FIRST:%d    or   ponSlotIdx:%d > PONCARD_LAST:%d       error! \r\n", ponSlotIdx, PONCARD_FIRST, ponSlotIdx, PONCARD_LAST));*/
		return VOS_ERROR;
	}
	
	if (PonPortId >= MAXPON)
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsRight()::PonPortId:%d >= MAXPON:%d  error! \r\n", PonPortId, MAXPON));*/
		return VOS_ERROR;
	}

	if (onuId >= MAXONUPERPON)
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsRight()::onuId:%d >= MAXONUPERPON:%d  error! \r\n", onuId, MAXONUPERPON));*/
		return VOS_ERROR;
	}
	
	return VOS_OK;
}

/* ONU设备索引是否支持E1 */
STATUS onuDevIdxIsSupportE1(ULONG onuDevId)
{
	int onuType = V2R1_DEVICE_UNKNOWN;
	ULONG PonPortId, onuId;

	if (VOS_OK != onuDevIdxIsRight(onuDevId))
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsSupportE1()::onuDevIdxIsRight()    error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( -1 == parseOnuIndexFromDevIdx(onuDevId, &PonPortId, &onuId) )
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsSupportE1()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if( ThisIsValidOnu( PonPortId, onuId ) != ROK )
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsSupportE1()::ThisIsValidOnu()   error!\r\n"));*/
		return VOS_ERROR;
	}

	GetOnuType(PonPortId, onuId, &onuType);

	if ( (onuType == V2R1_ONU_GT861) || ( (onuType == V2R1_OTHER) && (GetOnuOperStatus(PonPortId, onuId) != ONU_OPER_STATUS_UP) ) )
	{
		return VOS_OK;
	}
	else 
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsSupportE1()::onuType=%d   OperStatus=%d    line=%d error!\r\n", onuType, GetOnuOperStatus(PonPortId, onuId), __LINE__));*/
		return VOS_ERROR;
	}

       /*E1_ERROR_INFO_PRINT(("onuDevIdxIsSupportE1()::onuType=%d   OperStatus=%d    line=%d error!\r\n", onuType, GetOnuOperStatus(PonPortId, onuId), __LINE__));*/
	return VOS_ERROR;
}

/* ONU E1索引是否在范围内 */
STATUS checkOnuE1IsRight(ULONG onuDevId, UCHAR onuE1SlotId, UCHAR onuE1Id)
{
	if (VOS_OK != onuDevIdxIsRight(onuDevId))
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsRight()::onuDevIdxIsRight()    error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( (onuE1SlotId < MIN_ONU_E1_SLOT_ID) || (onuE1SlotId > MAX_ONU_E1_SLOT_ID) )
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsRight()  onu e1 slot index=%d  error! \r\n", onuE1SlotId));*/
		return VOS_ERROR;
	}

	if ( onuE1Id > MAX_ONU_BOARD_E1 )
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsRight()  onu e1 port index=%d  error! \r\n", onuE1Id));*/
		return VOS_ERROR;
	}

	return VOS_OK;
}

/* ONU E1索引是否支持E1 */
STATUS checkOnuE1IsSupportE1(ULONG onuDevId, UCHAR onuE1SlotId, UCHAR onuE1Id)
{
	ULONG PonPortId, onuId;

	if (VOS_OK != checkOnuE1IsRight(onuDevId, onuE1SlotId, onuE1Id))
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsSupportE1()::checkOnuE1IsRight()    error! \r\n"));*/
		return VOS_ERROR;
	}

	if (VOS_OK != onuDevIdxIsSupportE1(onuDevId))
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsSupportE1()::onuDevIdxIsSupportE1()    error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( -1 == parseOnuIndexFromDevIdx(onuDevId, &PonPortId, &onuId) )
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsSupportE1()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( (ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1SlotId].brdType != OAM_ONU_SLOT_GT_4E1_120ohm)  && 
		(ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1SlotId].brdType != OAM_ONU_SLOT_GT_4E1_75ohm) )
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsSupportE1()::onu:%d   board:%d     is not E1 board  error! \r\n", onuDevId, onuE1SlotId));
		E1_ERROR_INFO_PRINT(("ExtBrdMgmtTable[%ld][%ld].BrdMgmtTable[%d].brdType=%d \r\n", PonPortId, onuId + 1, onuE1SlotId, ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1SlotId].brdType));*/
		return VOS_ERROR;
	}

	return VOS_OK;
}

/* OnuE1Index中的变量是否支持E1 */
STATUS check_OnuE1Index_IsSupportE1(OnuE1Index *pOnuE1Index)
{
	if (NULL == pOnuE1Index)
	{
		/*E1_ERROR_INFO_PRINT(("checkOnuE1IsSupportE1()::   pOnuE1Index=NULL    error! \r\n"));*/
		return VOS_ERROR;
	}

	return checkOnuE1IsSupportE1(pOnuE1Index->onuDevId, (UCHAR)pOnuE1Index->onuE1SlotId, (UCHAR)pOnuE1Index->onuE1Id);
}




/* 输入ONU e1索引，返回下一个ONU e1索引 */
STATUS findNextOnuE1Idx( OnuE1Index *pOnuE1Index )
{
	ULONG PonPortId, onuId, i;
	OnuE1Index OnuE1Index_tmp;
	ULONG slot_i1, port_i1, onuId_i1;

	if (NULL == pOnuE1Index)
	{
		/*E1_ERROR_INFO_PRINT(("findNextOnu()  pOnuE1Index=NULL! \r\n"));*/
		return VOS_ERROR;
	}

	if ( -1 == parseOnuIndexFromDevIdx( pOnuE1Index->onuDevId, &PonPortId, &onuId ) )
	{
		/*E1_ERROR_INFO_PRINT(("findNextOnuE1Idx()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
		return VOS_ERROR;
	}

	memcpy((UCHAR *)&OnuE1Index_tmp, (UCHAR *)pOnuE1Index, sizeof(OnuE1Index));

	if ( (pOnuE1Index->onuE1Id < MAX_ONU_BOARD_E1) && ( VOS_OK == onuDevIdxIsSupportE1(pOnuE1Index->onuDevId) ) && 
		( (ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[pOnuE1Index->onuE1SlotId].brdType == OAM_ONU_SLOT_GT_4E1_120ohm) ||
		(ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[pOnuE1Index->onuE1SlotId].brdType == OAM_ONU_SLOT_GT_4E1_75ohm) ) )
	{
		return VOS_OK;
	}

	pOnuE1Index->onuE1Id = 0;

	if ( VOS_OK == onuDevIdxIsSupportE1(pOnuE1Index->onuDevId) )
	{
		for (i = pOnuE1Index->onuE1SlotId + 1; i <= MAX_ONU_E1_SLOT_ID; i++)
		{
			/*E1_ERROR_INFO_PRINT(("E1BoardSlot=%d      Type:%d \r\n", i, ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[i].brdType));*/
			if ( (ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[i].brdType == OAM_ONU_SLOT_GT_4E1_120ohm) || 
				(ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[i].brdType == OAM_ONU_SLOT_GT_4E1_75ohm) )
			{
				pOnuE1Index->onuE1SlotId = i;
				return VOS_OK;
			}
		}
	}

	slot_i1 = GET_PONSLOT(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId / 10000*/;
	port_i1 =GET_PONPORT(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId % 10000 / 1000*/;
	onuId_i1 = GET_ONUID(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId % 1000*/;

	pOnuE1Index->onuDevId = findNextOnu(slot_i1, port_i1, onuId_i1, ALL_ONU);

	while ( 0 != pOnuE1Index->onuDevId  )
	{
		if ( VOS_OK == onuDevIdxIsSupportE1(pOnuE1Index->onuDevId) )
		{
			if ( -1 == parseOnuIndexFromDevIdx( pOnuE1Index->onuDevId, &PonPortId, &onuId ) )
			{
				/*E1_ERROR_INFO_PRINT(("findNextOnuE1Idx()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
				return VOS_ERROR;
			}
			
			for (i = MIN_ONU_E1_SLOT_ID; i <= MAX_ONU_E1_SLOT_ID ; i++)
			{
				if ( (ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[i].brdType == OAM_ONU_SLOT_GT_4E1_120ohm) || 
					(ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[i].brdType == OAM_ONU_SLOT_GT_4E1_75ohm) )
				{
					pOnuE1Index->onuE1SlotId = i;
					return VOS_OK;
				}
			}
		}

		if ( -1 == parseOnuIndexFromDevIdx( pOnuE1Index->onuDevId, &PonPortId, &onuId ) )
		{
			/*E1_ERROR_INFO_PRINT(("findNextOnuE1Idx()::parseOnuIndexFromDevIdx()  error! \r\n"));*/
			return VOS_ERROR;
		}

		slot_i1 =GET_PONSLOT(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId / 10000*/;
		port_i1 =GET_PONPORT(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId % 10000 / 1000*/;
		onuId_i1 = GET_ONUID(pOnuE1Index->onuDevId)/*pOnuE1Index->onuDevId % 1000*/;
		pOnuE1Index->onuDevId = findNextOnu(slot_i1, port_i1, onuId_i1, ALL_ONU);
	}

	/*E1_ERROR_INFO_PRINT(("findNextOnuE1Idx()::last Onu E1,no more Onu E1  error! \r\n"));*/
	memcpy((UCHAR *)&pOnuE1Index, (UCHAR *)&OnuE1Index_tmp, sizeof(OnuE1Index));/* 恢复参数数据 */
	return VOS_ERROR;
}


STATUS checkE1TableIndex(ULONG *idxs)
{

	if (NULL == idxs)
	{
		/*E1_ERROR_INFO_PRINT(("checkE1PortTableIndex()  idxs=NULL! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		/* TDM侧 */
		if ( (idxs[1] < TDMCARD_FIRST) || (idxs[1] > TDMCARD_LAST) )
		{
			/*E1_ERROR_INFO_PRINT(("checkE1PortTableIndex()  tdm slot index=%d  error! \r\n", idxs[1]));*/
			return VOS_ERROR;
		}

		if ( idxs[2] > (MAX_E1_PER_FPGA * TDM_FPGA_MAX)/* 24 */ )
		{
			/*E1_ERROR_INFO_PRINT(("checkE1PortTableIndex()  tdm e1 port index=%d  error! \r\n", idxs[2]));*/
			return VOS_ERROR;
		}
	} 
	else
	{
		/* ONU侧 */
		if (VOS_OK != checkOnuE1IsRight(idxs[0], (UCHAR)idxs[1], (UCHAR)idxs[2]))
		{
			/*E1_ERROR_INFO_PRINT(("checkE1PortTableIndex()::checkOnuE1IsRight()     error! \r\n"));*/
			return VOS_ERROR;
		}
	}

	return VOS_OK;
}

/* leafIdx==0检查所有leLeafValue */
STATUS checkE1PortTableLeafValue( UCHAR leafIdx, UCHAR  eponE1Loop, UCHAR  eponE1TxClock )
{
	if ( leafIdx > LEAF_eponE1PortTxClock )
	{
		/*E1_ERROR_INFO_PRINT(("checkE1PortTableLeafValue()  leafIdx=%d  error! \r\n", leafIdx));*/
		return VOS_ERROR;
	}

	if ( (0 == leafIdx) || (LEAF_eponE1PortLoop == leafIdx) )
	{
		if (eponE1Loop & 0xFF00)
		{
			/*E1_ERROR_INFO_PRINT(("checkE1PortTableLeafValue()  Loop=0x%02x  error! \r\n", eponE1Loop));*/
			return VOS_ERROR;
		}
	}

	if ( (0 == leafIdx) || (LEAF_eponE1PortTxClock == leafIdx) )
	{
		if (eponE1TxClock > E1_TX_CLOCK_CRYS)
		{
			/*E1_ERROR_INFO_PRINT(("checkE1PortTableLeafValue()  TxClock=0x%02x  error! \r\n", eponE1TxClock));*/
			return VOS_ERROR;
		}
	}

	return VOS_OK;
}



STATUS sw_e1PortTable_get( ULONG *idxs, e1PortTable_t *pE1PortTable )
{
	ULONG PonPortId, onuId;
	UCHAR *ptr = NULL;

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_get()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		if (idxs[2] >= (MAX_E1_PER_FPGA * TDM_FPGA_MAX))
		{
			/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_get()::olt e1 port index:%d  error! \r\n", idxs[2]));*/
			return VOS_ERROR;
		}

		memcpy( (UCHAR *)pE1PortTable, (UCHAR *)&tdmE1PortTable[idxs[2]], sizeof(e1PortTable_t) );
	} 
	else
	{
		parseOnuIndexFromDevIdx( idxs[0], &PonPortId, &onuId );

		ptr = (UCHAR *)&( ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]] );

		memcpy( (UCHAR *)pE1PortTable, (UCHAR *)ptr, sizeof(e1PortTable_t) );
	}

	return VOS_OK;
}

STATUS sw_e1PortTable_getNext( ULONG *idxs, e1PortTable_t *pE1PortTable )
{
	ULONG PonPortId, onuId;
	UCHAR *ptr = NULL;
	OnuE1Index onuE1Index;

	idxs[2]++;/*getNext*/

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_getNext()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		if ( idxs[2] > (MAX_E1_PER_FPGA * TDM_FPGA_MAX - 1) )/* 从本端查找远端 */
		{
			onuE1Index.onuDevId    = findFirstOnu( ALL_ONU );
			onuE1Index.onuE1SlotId = MIN_ONU_E1_SLOT_ID;
			onuE1Index.onuE1Id     = 0;

			if ( VOS_ERROR == findNextOnuE1Idx( &onuE1Index ) )
			{
				/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_getNext()::findNextOnuE1Idx()  error! \r\n"));*/
				return VOS_ERROR;
			}

			idxs[0] = onuE1Index.onuDevId;
			idxs[1] = onuE1Index.onuE1SlotId;
			idxs[2] = onuE1Index.onuE1Id;

			parseOnuIndexFromDevIdx( idxs[0], &PonPortId, &onuId );
			ptr = (UCHAR *)&( ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]] );
			memcpy( (UCHAR *)pE1PortTable, (UCHAR *)ptr, sizeof(e1PortTable_t) );
		} 
		else
		{
			memcpy( (UCHAR *)pE1PortTable, (UCHAR *)&tdmE1PortTable[idxs[2]], sizeof(e1PortTable_t) );
		}
	}
	else
	{
		onuE1Index.onuDevId    = idxs[0];
		onuE1Index.onuE1SlotId = idxs[1];
		onuE1Index.onuE1Id     = idxs[2];

		if ( VOS_ERROR == findNextOnuE1Idx( &onuE1Index ) )
		{
			/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_getNext()::findNextOnuE1Idx()  error! \r\n"));*/
			return VOS_ERROR;
		}

		idxs[0] = onuE1Index.onuDevId;
		idxs[1] = onuE1Index.onuE1SlotId;
		idxs[2] = onuE1Index.onuE1Id;

		parseOnuIndexFromDevIdx( idxs[0], &PonPortId, &onuId );
		ptr = (UCHAR *)&( ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]] );
		memcpy( (UCHAR *)pE1PortTable, (UCHAR *)ptr, sizeof(e1PortTable_t) );
	}

	return VOS_OK;
}

STATUS sw_e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval )
{
	ULONG PonPortId, onuId;

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( VOS_ERROR == checkE1PortTableLeafValue( leafIdx, (UCHAR)setval, (UCHAR)setval ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::checkE1PortTableLeafValue()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		switch(leafIdx)
		{
		case LEAF_eponE1PortAlarmStatus:
			tdmE1PortTable[idxs[2]].eponE1PortAlarmStatus = setval;
			break;
		case LEAF_eponE1PortAlarmMask:
			tdmE1PortTable[idxs[2]].eponE1PortAlarmMask = setval;
			break;
		case LEAF_eponE1PortLoop:
			tdmE1PortTable[idxs[2]].eponE1Loop = (UCHAR)setval;
			break;
		case LEAF_eponE1PortTxClock:
			tdmE1PortTable[idxs[2]].eponE1TxClock = (UCHAR)setval;
		    break;
		default:
			/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::leafIdx=%d  error! \r\n", leafIdx));*/
			return VOS_ERROR;
		}
	} 
	else
	{
		parseOnuIndexFromDevIdx( idxs[0], &PonPortId, &onuId );
		/*E1_ERROR_INFO_PRINT(("%%leafIdx=0x%02x    setval=0x%04x\r\n", leafIdx, setval));*/

		switch(leafIdx)
		{
		case LEAF_eponE1PortAlarmStatus:
			ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1PortAlarmStatus = setval;
			break;
		case LEAF_eponE1PortAlarmMask:
			ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1PortAlarmMask = setval;
			break;
		case LEAF_eponE1PortLoop:
			ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1Loop = (UCHAR)setval;
			break;
		case LEAF_eponE1PortTxClock:
			ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1TxClock = (UCHAR)setval;
			break;
		default:
			/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::leafIdx=%d  error! \r\n", leafIdx));*/
			return VOS_ERROR;
		}
	}

	return VOS_OK;
}

STATUS sw_e1PortTable_rowset( ULONG  *idxs, USHORT eponE1PortAlarmMask, UCHAR  eponE1Loop, UCHAR  eponE1TxClock )
{
	ULONG PonPortId, onuId;

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( VOS_ERROR == checkE1PortTableLeafValue( 0, eponE1Loop, eponE1TxClock ) )
	{
		/*E1_ERROR_INFO_PRINT(("sw_e1PortTable_set()::checkE1PortTableLeafValue()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		tdmE1PortTable[idxs[2]].eponE1PortAlarmMask = eponE1PortAlarmMask;
		tdmE1PortTable[idxs[2]].eponE1Loop          = eponE1Loop;
		tdmE1PortTable[idxs[2]].eponE1TxClock       = eponE1TxClock;
	} 
	else
	{
		parseOnuIndexFromDevIdx( idxs[0], &PonPortId, &onuId );

		ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1PortAlarmMask = eponE1PortAlarmMask;
		ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1Loop          = eponE1Loop;
		ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].onuE1PortTable[idxs[2]].eponE1TxClock       = eponE1TxClock;
	}

	return VOS_OK;
}



STATUS e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval )
{
	OAM_OnuE1Clock oam_OnuE1Clock;
	OAM_OnuE1AlarmMask oam_OnuE1AlarmMask;
	ULONG SeachonuIdx[3], i = 0;
	onuE1Info OnuE1Info;
	e1PortTable_row_entry e1Port;
	USHORT tdmLoopValue;

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( VOS_ERROR == checkE1PortTableLeafValue( leafIdx, (UCHAR)setval, (UCHAR)setval ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::checkE1PortTableLeafValue()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		if (LEAF_eponE1PortLoop == leafIdx)
		{
			if (VOS_OK != tdm_e1PortTable_get( idxs, &e1Port ))
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_e1PortTable_get() error! index=%d\r\n", i));*/
				return VOS_ERROR;
			}

			e1Port.eponE1Loop &= TDM_E1_NO_LOOP;
			tdmLoopValue = (e1Port.eponE1Loop | setval);

			if ( VOS_OK != tdm_e1PortTable_set( leafIdx, idxs, tdmLoopValue ) )
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_e1PortTable_set()  error! \r\n"));*/
				return VOS_ERROR;
			}
		}
		else
		{
			if ( VOS_OK != tdm_e1PortTable_set( leafIdx, idxs, setval ) )
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_e1PortTable_set()  error! \r\n"));*/
				return VOS_ERROR;
			}
		}
	}
	else
	{
		switch(leafIdx)
		{
		case LEAF_eponE1PortAlarmMask:/* 发送到TDM,不发OAM */
			memset( (UCHAR *)&oam_OnuE1AlarmMask, 0, sizeof(OAM_OnuE1AlarmMask) );
			oam_OnuE1AlarmMask.MsgType = SET_ONU_E1_ALARMMASK_REQ;
			oam_OnuE1AlarmMask.E1PortTotalCount = 1;
			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1PortIdx = idxs[2] + 1;
			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1SlotIdx = idxs[1];
			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].AlarmMask = setval;

			if ( VOS_OK != SetOnuE1AlarmMask(idxs[0], &oam_OnuE1AlarmMask) )
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::SetOnuE1AlarmMask()  error! \r\n"));*/
				/*return VOS_ERROR;*/
			}
			break;

		case LEAF_eponE1PortLoop:/* 不发OAM，直接设置TDM上的fpga reg */

			SeachonuIdx[0] = 1;
			SeachonuIdx[1] = get_gfa_e1_slotno();
			SeachonuIdx[2] = 0xFFFFFFFF;

			if (VOS_OK != tdm_getOnuE1Info(SeachonuIdx, idxs[0], &OnuE1Info))
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_getOnuE1Info()  error! \r\n"));*/
				return VOS_ERROR;
			}

			for (i = 0; i < OnuE1Info.onuValidE1Count; i++)
			{
				if ( (OnuE1Info.onuEachE1Info[i].onuE1Slot == idxs[1]) && (OnuE1Info.onuEachE1Info[i].onuE1Index == idxs[2]) )
				{
					SeachonuIdx[2] = MAX_E1_PER_FPGA * OnuE1Info.onuEachE1Info[i].tdmSgIfId + OnuE1Info.onuEachE1Info[i].tdmE1Id;
				}
			}

			if (0xFFFFFFFF != SeachonuIdx[2])
			{
				if (VOS_OK != tdm_e1PortTable_get( SeachonuIdx, &e1Port ))
				{
					/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_e1PortTable_get() error! index=%d\r\n", i));*/
					return VOS_ERROR;
				}

				e1Port.eponE1Loop &= ONU_E1_NO_LOOP;
				tdmLoopValue = (e1Port.eponE1Loop | setval);

				if ( VOS_OK != tdm_e1PortTable_set( leafIdx, SeachonuIdx, tdmLoopValue ) )
				{
					/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::tdm_e1PortTable_set()  error! \r\n"));*/
					return VOS_ERROR;
				}
			}
			break;

		case LEAF_eponE1PortTxClock:
			memset( (UCHAR *)&oam_OnuE1Clock, 0, sizeof(OAM_OnuE1Clock) );
			oam_OnuE1Clock.MsgType = SET_ONU_E1_CLOCK_REQ;
			oam_OnuE1Clock.E1PortTotalCount = 1;
			oam_OnuE1Clock.oam_OnuE1Clock[0].E1PortIdx = idxs[2] + 1;
			oam_OnuE1Clock.oam_OnuE1Clock[0].E1SlotIdx = idxs[1];
			oam_OnuE1Clock.oam_OnuE1Clock[0].ClockControl = (UCHAR)setval;
			/*E1_ERROR_INFO_PRINT("ClockControl=0x%02x    setval=0x%04x%\r\n", oam_OnuE1Clock.oam_OnuE1Clock[0].ClockControl, setval);*/

			if ( VOS_OK != SetOnuE1Clock(idxs[0], &oam_OnuE1Clock) )
			{
				/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::SetOnuE1Clock()  error! \r\n"));*/
				/*return VOS_ERROR;*/
			}
			break;
		default:
			/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::leafIdx=%d  error! \r\n", leafIdx));*/
			return VOS_ERROR;
		}
	}

	if ( VOS_OK != sw_e1PortTable_set( leafIdx, idxs, setval ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_set()::sw_e1PortTable_set()  error! \r\n"));*/
		return VOS_ERROR;
	}

	return VOS_OK;
}

/* 先别调用此函数 */
STATUS e1PortTable_rowset( ULONG *idxs, USHORT eponE1PortAlarmMask, UCHAR eponE1Loop, UCHAR eponE1TxClock )
{
	OAM_OnuE1Clock oam_OnuE1Clock;
	OAM_OnuE1AlarmMask oam_OnuE1AlarmMask;

	if ( VOS_ERROR == checkE1TableIndex( idxs ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( VOS_ERROR == checkE1PortTableLeafValue( 0, eponE1Loop, eponE1TxClock ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::checkE1PortTableLeafValue()  error! \r\n"));*/
		return VOS_ERROR;
	}

	if (1 == idxs[0])
	{
		if ( VOS_OK != tdm_e1PortTable_rowset( idxs, eponE1PortAlarmMask, eponE1Loop, eponE1TxClock ) )
		{
			/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::tdm_e1PortTable_rowset()  error! \r\n"));*/
			return VOS_ERROR;
		}
	} 
	else
	{
		memset( (UCHAR *)&oam_OnuE1AlarmMask, 0, sizeof(OAM_OnuE1AlarmMask) );
		oam_OnuE1AlarmMask.MsgType = SET_ONU_E1_ALARMMASK_REQ;
		oam_OnuE1AlarmMask.E1PortTotalCount = 1;
		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1PortIdx = idxs[2] + 1;
		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1SlotIdx = idxs[1];
		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].AlarmMask = eponE1PortAlarmMask;

		if ( VOS_OK != SetOnuE1AlarmMask(idxs[0], &oam_OnuE1AlarmMask) )
		{
			/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::SetOnuE1AlarmMask()  error! \r\n"));*/
			/*return VOS_ERROR;*/
		}			

/*    onu端的环回，通过tdm侧的fpga设置，不发oam了
		memset( (UCHAR *)&oam_OnuE1Loop, 0, sizeof(OAM_OnuE1Loop) );
		oam_OnuE1Loop.MsgType     = SET_ONU_E1_LOOP_REQ;
		oam_OnuE1Loop.E1PortIdx   = idxs[2] + 1;
		oam_OnuE1Loop.E1SlotIdx   = idxs[1];
		oam_OnuE1Loop.LoopControl = eponE1Loop;

		if ( VOS_OK != SetOnuE1Loop(idxs[0], &oam_OnuE1Loop) )
		{
			E1_ERROR_INFO_PRINT("e1PortTable_rowset()::SetOnuE1Loop()  error! \r\n");
			return VOS_ERROR;
		}
*/
		memset( (UCHAR *)&oam_OnuE1Clock, 0, sizeof(OAM_OnuE1Clock) );
		oam_OnuE1Clock.MsgType = SET_ONU_E1_CLOCK_REQ;
		oam_OnuE1Clock.E1PortTotalCount = 1;
		oam_OnuE1Clock.oam_OnuE1Clock[0].E1PortIdx = idxs[2]  + 1;
		oam_OnuE1Clock.oam_OnuE1Clock[0].E1SlotIdx = idxs[1];
		oam_OnuE1Clock.oam_OnuE1Clock[0].ClockControl = eponE1TxClock;

		if ( VOS_OK != SetOnuE1Clock(idxs[0], &oam_OnuE1Clock) )
		{
			/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::SetOnuE1Clock()  error! \r\n"));*/
			/*return VOS_ERROR;*/
		}
	}

	if ( VOS_OK != sw_e1PortTable_rowset( idxs, eponE1PortAlarmMask, eponE1Loop, eponE1TxClock ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_rowset()::sw_e1PortTable_rowset()  error! \r\n"));*/
		return VOS_ERROR;
	}

	return VOS_OK;
}

STATUS tdm_e1_alarm_set(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType)
{
	UCHAR i;
	ULONG idxs[3];
	e1PortTable_t e1PortTable;

	/*E1_ERROR_INFO_PRINT("Enter tdm_e1_alarm_set()   fpgaIdx=%d  e1PortIdx=%d  alarmType=0x%04x", fpgaIdx, e1PortIdx, alarmType);*/

	if ( !get_gfa_e1_slotno() )
	{
		return VOS_OK;
	}

	if ( fpgaIdx >= TDM_FPGA_MAX )
	{
		/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::fpgaIdx=%d   error! \r\n", fpgaIdx));*/
		return VOS_ERROR;
	}

	if ( e1PortIdx >= MAX_E1_PER_FPGA )
	{
		/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::e1PortIdx=%d   error! \r\n", e1PortIdx));*/
		return VOS_ERROR;
	}

	/*sys_console_printf("\r\n&&&& fpgaIdx=%d e1PortIdx=%d alarmType=0x%04x\r\n", fpgaIdx, e1PortIdx, alarmType);*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = fpgaIdx * MAX_E1_PER_FPGA + e1PortIdx;

	for (i = 0; i < 16; i++)
	{
		if ( e1PortTable.eponE1PortAlarmMask & (0x01 << i) )
		{
			continue;
		}
		else
		{
			if ( alarmType & (0x01 << i) )
			{
				if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
				{
					/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::sw_e1PortTable_get()  error! \r\n"));*/
					return VOS_ERROR;
				}
				
				if ( e1PortTable.eponE1PortAlarmStatus & (0x01 << i) )
				{
					continue;
				} 
				else
				{
					e1PortTable.eponE1PortAlarmStatus |= 0x01 << i;

					if ( VOS_OK != sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, e1PortTable.eponE1PortAlarmStatus ) )
					{
						/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::sw_e1PortTable_set()  error! \r\n"));*/
						return VOS_ERROR;
					}
				}
			}
		}
	}

	return VOS_OK;
}

STATUS tdm_e1_alarm_clear(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType)
{
	UCHAR i;
	ULONG idxs[3];
	e1PortTable_t e1PortTable;

	if ( !get_gfa_e1_slotno() )
	{
		return VOS_OK;
	}

	/*E1_ERROR_INFO_PRINT("Enter tdm_e1_alarm_clear()   fpgaIdx=%d  e1PortIdx=%d  alarmType=0x%04x", fpgaIdx, e1PortIdx, alarmType);*/

	if ( fpgaIdx >= TDM_FPGA_MAX )
	{
		/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::fpgaIdx=%d   error! \r\n", fpgaIdx));*/
		return VOS_ERROR;
	}

	if ( e1PortIdx >= MAX_E1_PER_FPGA )
	{
		/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::e1PortIdx=%d   error! \r\n", e1PortIdx));*/
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = fpgaIdx * MAX_E1_PER_FPGA + e1PortIdx;

	for (i = 0; i < 16; i++)
	{
		if ( alarmType & (0x01 << i) )
		{
			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::sw_e1PortTable_get()  error! \r\n"));*/
				return VOS_ERROR;
			}
			
			if ( e1PortTable.eponE1PortAlarmStatus & (0x01 << i) )
			{
				e1PortTable.eponE1PortAlarmStatus &= ~(0x01 << i);

				if ( VOS_OK != sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, e1PortTable.eponE1PortAlarmStatus ) )
				{
					/*E1_ERROR_INFO_PRINT(("tdm_e1_alarm_set()::sw_e1PortTable_set()  error! \r\n"));*/
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}

static BOOL onuDevIdxIsExist(ULONG newDevIdx, UCHAR onuCnt, ULONG *onuDevIdx)
{
	UCHAR i;

	if (NULL == onuDevIdx)
	{
		/*E1_ERROR_INFO_PRINT(("onuDevIdxIsExist():: onuDevIdx=NULL   error! \r\n"));*/
		return VOS_ERROR;
	}

	for (i = 0; i < onuCnt; i++)
	{
		if (onuDevIdx[i] == newDevIdx)
		{
			return FALSE;
		}
	}

	return TRUE;
}

STATUS getOneFpgaOnuDevIdx(UCHAR fpgaIdx/*0-2*/, UCHAR *onuCnt, ULONG *onuDevIdx)
{
	ULONG idxs[2], i;
	AllE1Info allE1Info;

	if ( (NULL == onuCnt) || (NULL == onuDevIdx) )
	{
		/*E1_ERROR_INFO_PRINT(("getOneFpgaOnuDevIdx()::onuCnt=NULL or onuDevIdx=NULL   error! \r\n"));*/
		return VOS_ERROR;
	}

	if ( fpgaIdx >= TDM_FPGA_MAX )
	{
		/*E1_ERROR_INFO_PRINT(("getOneFpgaOnuDevIdx()::fpgaIdx=%d   error! \r\n", fpgaIdx));*/
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getAllE1Info( idxs, fpgaIdx, &allE1Info ) )
	{
		/*E1_ERROR_INFO_PRINT(("getOneFpgaOnuDevIdx()::tdm_getAllE1Info()  error! \r\n"));*/
		return VOS_ERROR;
	}

	*onuCnt = 0;

	for (i = 0; i < allE1Info.fpgaE1Info[0].fpgaValidE1Count && i < MAX_E1_PER_FPGA; i++)
	{
		if (MIB_E1_ENABLE == allE1Info.fpgaE1Info[0].eachE1Info[i].eponE1LocalEnable)
		{
			if (onuDevIdxIsExist(allE1Info.fpgaE1Info[0].eachE1Info[i].onuDevId, *onuCnt, onuDevIdx))
			{
				onuDevIdx[++(*onuCnt)] = allE1Info.fpgaE1Info[0].eachE1Info[i].onuDevId;
			}
		}
	}

	return VOS_OK;
}

/* 0:不显示ONU的E1端口数据    1:显示在线的861(在线肯定是注册过的) */
STATUS printSwE1PortTable( struct vty *vty, UCHAR showOnu)
{
	OnuE1Index onuE1Index;
	ULONG PonPortId, onuId, i;

	if (showOnu > 1)
	{
		/*E1_ERROR_INFO_PRINT(("printSwE1PortTable()::showOnu=%d  error! \r\n", showOnu));*/
		return VOS_ERROR;
	}

	vty_out( vty, "\r\n E1 Port\r\n" );
	vty_out( vty, "Idx        AlarmStat       AlarmMask         Loop           TxClock\r\n" );

	switch(showOnu)
	{
	case 1:
		for (i = 0; i < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); i++)
		{
			vty_out( vty, "%05d      0x%04x          0x%04x            0x%02x           %d\r\n", i, 
				tdmE1PortTable[i].eponE1PortAlarmStatus, tdmE1PortTable[i].eponE1PortAlarmMask, 
				tdmE1PortTable[i].eponE1Loop, tdmE1PortTable[i].eponE1TxClock );
		}

		break;
	case 2:
		onuE1Index.onuDevId    = findFirstOnu( ALL_ONU );
		onuE1Index.onuE1SlotId = MIN_ONU_E1_SLOT_ID;
		onuE1Index.onuE1Id     = 0;

		while (VOS_OK == findNextOnuE1Idx( &onuE1Index ))
		{
			parseOnuIndexFromDevIdx( onuE1Index.onuDevId, &PonPortId, &onuId );

			vty_out( vty, "%05d      0x%04x          0x%04x            0x%02x           %d\r\n", onuE1Index.onuDevId, 
				ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1Index.onuE1SlotId].onuE1PortTable[onuE1Index.onuE1Id].eponE1PortAlarmStatus, 
				ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1Index.onuE1SlotId].onuE1PortTable[onuE1Index.onuE1Id].eponE1PortAlarmMask, 
				ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1Index.onuE1SlotId].onuE1PortTable[onuE1Index.onuE1Id].eponE1Loop, 
				ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[onuE1Index.onuE1SlotId].onuE1PortTable[onuE1Index.onuE1Id].eponE1TxClock );
		}

		break;
	}

	vty_out( vty, "\r\n" );
	return VOS_OK;
}

/* 供MIB设置环回的接口，其中加了2个判断条件：1.没配置链接的端口不能设置环回；2.设置了一端（OLT或者ONU），不能设置另外一端 */
STATUS e1PortTable_setLoopBack(ULONG *idxs, USHORT setval)
{
	AllE1Info allE1Info;
	ULONG index[3], i, j;
	e1PortTable_t e1PortTable;

	if ( VOS_ERROR == checkE1TableIndex(idxs) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::checkE1TableIndex()  error! \r\n"));*/
		return VOS_ERROR;
	}

	index[0] = 1;
	index[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getAllE1Info(index, 0xFF, &allE1Info) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::tdm_getAllE1Info()  error! \r\n"));*/
		return VOS_ERROR;
	}

	/* 判断要设置的环回值 */
	if (1 == idxs[0])
	{
		if (TDM_E1_ALL_LOOP != setval && 0 != setval)
		{
			/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::setval=0x%04x  error! Line=%d\r\n", setval, __LINE__));*/
			return VOS_ERROR;
		}
	}
	else
	{
		if (ONU_E1_ALL_LOOP != setval && 0 != setval)
		{
			/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::setval=0x%04x  error! Line=%d\r\n", setval, __LINE__));*/
			return VOS_ERROR;
		}
	}

	if (1 == idxs[0])
	{
		/* OLT索引是否配置链接 */
		for (i = 0; i < TDM_FPGA_MAX; i++)
		{
			for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				if ( ( (idxs[2] / MAX_E1_PER_FPGA) == i) && ( (idxs[2] % MAX_E1_PER_FPGA) == allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index ) )
				{
					/* 找到对端ONU索引 */
					index[0] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId;
					index[1] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId;
					index[2] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id;

					goto next;
				}
			}
		}

		return VOS_ERROR;/* 没配置 */
	} 
	else
	{
		/* ONU索引是否配置链接 */
		for (i = 0; i < TDM_FPGA_MAX; i++)
		{
			for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				if ( (idxs[0] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId) 
					&& (idxs[1] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId) 
					&& (idxs[2] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id) )
				{
					/* 找到对端OLT索引 */
					index[0] = 1;
					index[1] = get_gfa_e1_slotno();
					index[2] = i * MAX_E1_PER_FPGA + allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index;

					goto next;
				}
			}
		}

		return VOS_ERROR;/* 没配置 */
	}

next:
	/*E1_ERROR_INFO_PRINT(("\r\nMIB Set:%d/%d/%d    value=0x%04x\r\n", idxs[0], idxs[1], idxs[2], setval));*/

	if ( VOS_OK != sw_e1PortTable_get(index, &e1PortTable) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::sw_e1PortTable_get()   error! \r\n"));*/
		return VOS_ERROR;
	}

	if (0 != e1PortTable.eponE1Loop)
	{
		/* 已经配置了一端，就不再配置 */
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::eponE1Loop=0x%02x   error! Line=%d\r\n", e1PortTable.eponE1Loop, __LINE__));*/
		return VOS_ERROR;
	}

	if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, setval ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::e1PortTable_set()   error! \r\n"));*/
		return VOS_ERROR;
	}

	return VOS_OK;
}

/* 在CLI中判断，没配置链接的，已经设置了一端系统环回的，不能再设置环回 */
/* 能设置环回，返回true，否则返回false */
BOOL checkLoopBack(ULONG *idxs, USHORT setval)
{
	AllE1Info allE1Info;
	ULONG index[3], i, j;
	e1PortTable_t e1PortTable;

	if ( VOS_ERROR == checkE1TableIndex(idxs) )
	{
		/*E1_ERROR_INFO_PRINT(("checkLoopBack()::checkE1TableIndex()  error! \r\n"));*/
		return FALSE;
	}

	index[0] = 1;
	index[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getAllE1Info(index, 0xFF, &allE1Info) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::tdm_getAllE1Info()  error! \r\n"));*/
		return FALSE;
	}

	if (1 == idxs[0])
	{
		/* OLT索引是否配置链接 */
		for (i = 0; i < TDM_FPGA_MAX; i++)
		{
			for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				if ( ( (idxs[2] / MAX_E1_PER_FPGA) == i) && ( (idxs[2] % MAX_E1_PER_FPGA) == allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index ) )
				{
					/* 找到对端ONU索引 */
					index[0] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId;
					index[1] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId;
					index[2] = allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id;

					goto next1;
				}
			}
		}

		return FALSE;/* 没配置 */
	} 
	else
	{
		/* ONU索引是否配置链接 */
		for (i = 0; i < TDM_FPGA_MAX; i++)
		{
			for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				if ( (idxs[0] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId) 
					&& (idxs[1] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId) 
					&& (idxs[2] == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id) )
				{
					/* 找到对端OLT索引 */
					index[0] = 1;
					index[1] = get_gfa_e1_slotno();
					index[2] = i * MAX_E1_PER_FPGA + allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index;

					goto next1;
				}
			}
		}

		return FALSE;/* 没配置 */
	}

next1:
	/*E1_ERROR_INFO_PRINT(("\r\nMIB Set:%d/%d/%d    value=0x%04x\r\n", idxs[0], idxs[1], idxs[2], setval));*/

	if ( VOS_OK != sw_e1PortTable_get(index, &e1PortTable) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::sw_e1PortTable_get()   error! \r\n"));*/
		return FALSE;
	}

	if (1 == idxs[0])
	{
		if ( (ONU_E1_SYSTEM_LOOP & e1PortTable.eponE1Loop) && (TDM_E1_SYSTEM_LOOP & setval) )
		{
			/* 已经配置了一端的系统环回，就不再配置另一端系统环回 */
			/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::eponE1Loop=0x%02x   error! Line=%d\r\n", e1PortTable.eponE1Loop, __LINE__));*/
			return FALSE;
		}
	} 
	else
	{
		if ( (TDM_E1_SYSTEM_LOOP & e1PortTable.eponE1Loop) && (ONU_E1_SYSTEM_LOOP & setval) )
		{
			/* 已经配置了一端的系统环回，就不再配置另一端系统环回 */
			/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::eponE1Loop=0x%02x   error! Line=%d\r\n", e1PortTable.eponE1Loop, __LINE__));*/
			return FALSE;
		}
	}

	if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, setval ) )
	{
		/*E1_ERROR_INFO_PRINT(("e1PortTable_setLoopBack()::e1PortTable_set()   error! \r\n"));*/
		return FALSE;
	}

	return TRUE;
}

/* 当OLT侧的E1板卡被拔出，清除SW上的E1表项数据 */
void initSwE1Data(void)
{
	/*ULONG i, j, k, l;*/
	ULONG i;

	for (i = 0; i < (MAX_SGIF_E1 * MAX_SGIF_ID); i++)
	{
		memset((char *)&tdmE1PortTable[i], 0, sizeof(e1PortTable_t));
	}

#if 0
	for (i = 0; i < MAXPON; i++)
	{
		for (j = 0; j < MAXONUPERPON; j++)
		{
			for (k = 0; k < MAX_ONU_E1_SLOT_ID; k++)
			{
				for (l = 0; l < MAX_ONU_BOARD_E1; l++)
				{
					/*memset((char *)&(ExtBrdMgmtTable[i][j + 1].BrdMgmtTable[k].onuE1PortTable[l]), 0, sizeof(e1PortTable_t));*/

					ExtBrdMgmtTable[i][j + 1].BrdMgmtTable[k].onuE1PortTable[l].eponE1Loop = 0;
				}
			}
		}
	}
#endif

}


#endif



