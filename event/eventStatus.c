#ifdef __cplusplus
extern "C" {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "lib_gwEponMib.h"
#include "gwEponSys.h"
#include  "gwEponMibData.h"
#include "Cdp_pub.h"
#include "eventStatus.h"
#include "ethLoopChk.h"

#define boardno_is_valid(brd) ((brd != 0) && (brd <= SYS_CHASSIS_SLOTNUM))
#define eth_portno_is_valid(port) ((port != 0) && (port <= MAX_ETH_PORT_NUM))
#define pon_portno_is_valid(port) ((port != 0) && (port <= SYS_MAX_PON_PORTNUM))


typedef struct _alarm_status_list_t{
	alm_status_t alm_item;
	struct _alarm_status_list_t *next;
} __attribute__((packed)) alm_status_list_t;

alm_status_list_t *alm_status_list = NULL;
const ULONG alm_status_list_size = 2048;
LONG alm_status_list_len = 0;


extern LONG getEventLogTime( nvramLogDateAndTime_t *dt );
extern CHAR *eventLogDataToStrings( nvramEventLogData_t *pEntry, CHAR *logDesc );
extern ULONG getEventAlarmSrcType( ULONG alarmType, ULONG alarmId );
extern LONG setEventAlarmLevel( ULONG alarmType, ULONG alarmId, ULONG level );
extern LONG getOnuEthPortAlarmMask( ulong_t *mask );
extern LONG setOnuEthPortAlarmMask( ulong_t mask );
extern int OltEthPortAlarmMaskSet( ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx, ulong_t mask );
extern int OnuEthPortAlarmMaskSet( ULONG mask );
extern int DeviceAlarmMaskSet(DeviceIndex_S  DevIdx,  unsigned long mask );
extern LONG getOnuTypeAlarmMask(ulong_t onu_type,ulong_t  *maskvalue);
/*extern int send_onuSyncMessage_Register( short int PonPortIdx, short int OnuIdx );*/

LONG alm_status_src_idx_comp( ULONG alarmType, ULONG trapId, alarmSrc_t *pSrc1, alarmSrc_t *pSrc2 );
LONG eventSync_configData_2Slave( ULONG slotno, VOID *pCfgData, ULONG dataLen );
LONG eventSync_configData_2AllSlave( VOID *pCfgData, ULONG dataLen );
static LONG alm_status_src_idx_get( alm_status_t *pAlmItem, ULONG *pIdxList, ULONG *pIdxNum );

/* modified by xieshl 20120502, 问题单15084 */
static void __alm_status_debug( alm_status_list_t *pItem, char * pPrompt )
{
	CHAR pAlmStr[MAXLEN_EVENT_DESC+1];
	pAlmStr[0] = 0;
	eventLogDataToStrings( (VOID*)&pItem->alm_item, pAlmStr );
	if( pAlmStr[0] )
	{
		sys_console_printf( "ALM_STA:(%s)", pPrompt );
		sys_console_printf( "%s\r\n", pAlmStr );
	}
}

#define ALM_STATUS_LIST_ITEM_INSERT(pItem) \
{\
	pItem->next = alm_status_list;\
	alm_status_list = pItem;\
	alm_status_list_len++;\
	if( eventDebugSwitch & EVENT_DEBUGSWITCH_STA )\
	{\
		__alm_status_debug( pItem, "ADD" );\
	}\
}

#define ALM_STATUS_LIST_ITEM_DESTROY(pPreItem, pItem) \
{\
	if( eventDebugSwitch & EVENT_DEBUGSWITCH_STA )\
	{\
		__alm_status_debug( pItem, "DEL" );\
	}\
	if( alm_status_list_len > 0 )\
		alm_status_list_len--;\
	if( pPreItem == NULL )\
	{\
		alm_status_list = alm_status_list->next;\
		VOS_Free( pItem );\
		pItem = alm_status_list;\
		continue;\
	}\
	else\
	{\
		pPreItem->next = pItem->next; \
		VOS_Free( pItem );\
		pItem = pPreItem;\
	}\
}

/*在当前告警表中查找是否存在，并且返回找到的告警条目地址
根据告警的type和ID匹配*/
static alm_status_list_t * alm_status_list_get( eventMsg_t *pAlmMsg )
{
	alm_status_list_t *pItem = NULL;

	if( pAlmMsg == NULL )
		return pItem;
	
	pItem = alm_status_list;
	while( pItem )
	{
		if( (pItem->alm_item.alarmType == pAlmMsg->alarmType) && (pItem->alm_item.alarmId == pAlmMsg->alarmId) )
		{
			if( alm_status_src_idx_comp(pAlmMsg->alarmType, pAlmMsg->alarmId, &pItem->alm_item.alarmSrc, &pAlmMsg->alarmSrc) == 0 )
				break;
		}
		pItem = pItem->next;
	}
	return pItem;
}
/*创建新的告警条目*/
static LONG alm_status_list_new( eventMsg_t *pAlmMsg )
{
	alm_status_list_t *pItem;
	ULONG level;
	
	if( pAlmMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( alm_status_list_len >= alm_status_list_size )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	level = getEventAlarmLevel( pAlmMsg->alarmType, pAlmMsg->alarmId);
	if( (level == ALM_LEV_VITAL) || (level == ALM_LEV_MAJOR) || (level == ALM_LEV_MINOR) )
	{
		pItem = alm_status_list_get( pAlmMsg );
		if( pItem )
		{
			if( eventDebugSwitch & EVENT_DEBUGSWITCH_STA )
			{
				CHAR pAlmStr[MAXLEN_EVENT_DESC+1];
				eventLogDataToStrings( (VOID*)&pItem->alm_item, pAlmStr );
				if( pAlmStr[0] )
				{
					sys_console_printf( "ALM_STA: ADD repeat:" );
					sys_console_printf( "%s\r\n", pAlmStr );
				}
			}
			return VOS_OK;
		}
		
		pItem = VOS_Malloc( sizeof(alm_status_list_t), MODULE_EVENT );
		if( pItem == NULL )
			return VOS_ERROR;

		VOS_MemZero( pItem, sizeof(alm_status_list_t) );
		pItem->alm_item.alarmType = pAlmMsg->alarmType;
		pItem->alm_item.alarmId = pAlmMsg->alarmId;
		VOS_MemCpy( &pItem->alm_item.alarmSrc, &pAlmMsg->alarmSrc, sizeof(alarmSrc_t) );
		getEventLogTime( &pItem->alm_item.alarmTime );

		ALM_STATUS_LIST_ITEM_INSERT(pItem);
	}
	return VOS_OK;
}
/*删除对应的告警条目*/
static LONG alm_status_list_free( eventMsg_t *pAlmMsg )
{
	alm_status_list_t *pItem, *pPreItem;
	ULONG partnerAlmId = 0;
	
	if( pAlmMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	partnerAlmId = getPartnerTrapId( pAlmMsg->alarmType, pAlmMsg->alarmId );
    /*moved by luh 2013-7-11, 取不到partneralmid 也需要删掉告警*/
	/*if( partnerAlmId == 0 )
		return VOS_OK;*/

	pItem = alm_status_list;
	pPreItem = NULL;
	
	while( pItem )
	{
		if( (pItem->alm_item.alarmType == pAlmMsg->alarmType) && 
			((pItem->alm_item.alarmId == partnerAlmId) || (pItem->alm_item.alarmId == pAlmMsg->alarmId)) )
		{
			if( alm_status_src_idx_comp(pAlmMsg->alarmType, pAlmMsg->alarmId, &pItem->alm_item.alarmSrc, &pAlmMsg->alarmSrc) == 0 )
			{
				ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	return VOS_OK;
}

static LONG alm_status_src_dev_idx_get( alm_status_t *pAlmItem )
{
	LONG devIdx = VOS_ERROR;
	alarmSrc_t *pAlmSrc;

	if( pAlmItem == NULL )
		return devIdx;
	if( pAlmItem->alarmType != alarmType_private )
		return devIdx;

	pAlmSrc = &pAlmItem->alarmSrc;

	switch( getEventAlarmSrcType(pAlmItem->alarmType, pAlmItem->alarmId) )
	{
		case ALM_SRC_T_PON_PWR:
			devIdx = OLT_DEV_ID;		/* pAlmItem->alarmSrc.oltRxpowerAlarmSrc.onuIdx;*/
			break;
		case ALM_SRC_T_ONU_LASER:
			devIdx = OLT_DEV_ID;/*MAKEDEVID( pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.brdIdx, 
								pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.portIdx, 
								pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.onuIdx );*/
			break;

		case ALM_SRC_T_COMM:
			devIdx = OLT_DEV_ID;		/*pAlmSrc->commAlarmSrc.devIdx;*/
			break;

		case ALM_SRC_T_ONU_SWITCH:
			devIdx =	MAKEDEVID( pAlmSrc->onuSwitchAlarmSrc.brdId, pAlmSrc->onuSwitchAlarmSrc.ponId, pAlmSrc->onuSwitchAlarmSrc.onuId );
			break;

		default:
			devIdx = pAlmSrc->devAlarmSrc.devIdx;
			break;
	}
	
	return devIdx;
}

static LONG alm_status_src_brd_idx_get( alm_status_t *pAlmItem, ULONG *pDevIdx, ULONG *pBrdIdx )
{
	LONG rc = VOS_ERROR;
	ULONG idxList[6], idxNum = 0;

	if( (pAlmItem == NULL) || (pDevIdx == NULL) || (pBrdIdx == NULL) )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( (rc = alm_status_src_idx_get(pAlmItem, idxList, &idxNum)) == VOS_OK )
	{
		*pDevIdx = idxList[0];
		if( idxNum == 1 )
		{
			if( idxList[0] == OLT_DEV_ID )
				*pBrdIdx = SYS_MASTER_ACTIVE_SLOTNO;
			else
				*pBrdIdx = 1;
		}
		else if( idxNum > 1 )
		{
			*pBrdIdx = idxList[1];
		}
	}
	return rc;
}

static LONG alm_status_src_idx_get( alm_status_t *pAlmItem, ULONG *pIdxList, ULONG *pIdxNum )
{
	LONG rc = VOS_ERROR;
	alarmSrc_t *pAlmSrc;
	LONG num = 0;

	if( (pAlmItem == NULL) || (pIdxList == NULL) || (pIdxNum == NULL) )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( pAlmItem->alarmType != alarmType_private )
		return rc;

	pAlmSrc = &pAlmItem->alarmSrc;

	rc = VOS_OK;
	switch( getEventAlarmSrcType(pAlmItem->alarmType, pAlmItem->alarmId) )
	{
		/* deviceIndex */
		case ALM_SRC_T_DEV:
			pIdxList[num++] = pAlmSrc->devAlarmSrc.devIdx;
			break;

		/* deviceIndex, boardIndex */
		case ALM_SRC_T_BRD:
			pIdxList[num++] = pAlmSrc->brdAlarmSrc.devIdx;
			pIdxList[num++] = pAlmSrc->brdAlarmSrc.brdIdx;
			break;
		/* deviceIndex, devFanIndex */
		case ALM_SRC_T_FAN:
			pIdxList[num++] = pAlmSrc->fanAlarmSrc.devIdx;
			pIdxList[num++] = pAlmSrc->fanAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->fanAlarmSrc.fanIdx;
			break;

		/* deviceIndex, ponPortBrdIndex, ponPortIndex */
		case ALM_SRC_T_PORT:
		case ALM_SRC_T_MON:
		case ALM_SRC_T_ETH_PWR:
		/*case ALM_SRC_T_E1:
		case ALM_SRC_T_TDM:*/
			pIdxList[num++] = pAlmSrc->portAlarmSrc.devIdx;
			pIdxList[num++] = pAlmSrc->portAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->portAlarmSrc.portIdx;
			break;

		/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
		case ALM_SRC_T_LLID:
			pIdxList[num++] = pAlmSrc->llidAlarmSrc.devIdx;
			pIdxList[num++] = pAlmSrc->llidAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->llidAlarmSrc.portIdx;
			pIdxList[num++] = pAlmSrc->llidAlarmSrc.llidIdx;
			break;

		/* deviceIndex, ponPortBrdIndex, ponPortIndex, onuIndex,oltPonReceiverPower*/
		case ALM_SRC_T_PON_PWR:
			pIdxList[num++] = pAlmItem->alarmSrc.oltRxpowerAlarmSrc.onuIdx;
			pIdxList[num++] = pAlmItem->alarmSrc.oltRxpowerAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmItem->alarmSrc.oltRxpowerAlarmSrc.portIdx;
			break;

		case ALM_SRC_T_ONU_LASER:
			pIdxList[num++] = OLT_DEV_ID;	/*pAlmItem->alarmSrc.oltRxpowerAlarmSrc.onuIdx;*/
								/*MAKEDEVID( pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.brdIdx, 
								pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.portIdx, 
								pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.onuIdx );*/
			pIdxList[num++] = pAlmItem->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmItem->alarmSrc.oltRxpowerAlwaysOnAlarmSrc.portIdx;
			break;

		case ALM_SRC_T_COMM:
			pIdxList[num++] = OLT_DEV_ID;
			pIdxList[num++] = pAlmSrc->commAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->commAlarmSrc.portIdx;
			if( pAlmItem->alarmId == trap_onuDeletingNotify )
				pIdxList[num++] = pAlmSrc->commAlarmSrc.onuIdx;
			break;

		case ALM_SRC_T_ONU_SWITCH:
			pIdxList[num++] = MAKEDEVID( pAlmSrc->onuSwitchAlarmSrc.brdId, pAlmSrc->onuSwitchAlarmSrc.ponId, pAlmSrc->onuSwitchAlarmSrc.onuId );
			pIdxList[num++] = pAlmSrc->onuSwitchAlarmSrc.onuBrdId;
			pIdxList[num++] = pAlmSrc->onuSwitchAlarmSrc.onuPortId;
			break;

		default:
			rc = VOS_ERROR;
			break;
	}

	*pIdxNum = num;
	
	return rc;
}

static LONG alm_status_src_associated_idx_get( alm_status_t *pAlmItem, ULONG *pIdxList, ULONG *pIdxNum )
{
	LONG rc = VOS_ERROR;
	alarmSrc_t *pAlmSrc;
	LONG num = 0;

	if( (pAlmItem == NULL) || (pIdxList == NULL) || (pIdxNum == NULL) )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( pAlmItem->alarmType != alarmType_private )
		return rc;

	pAlmSrc = &pAlmItem->alarmSrc;

	switch( getEventAlarmSrcType(pAlmItem->alarmType, pAlmItem->alarmId) )
	{
		/* deviceIndex */
		case ALM_SRC_T_DEV:
			pIdxList[num++] = OLT_DEV_ID;
			if( pAlmSrc->devAlarmSrc.devIdx != OLT_DEV_ID )
			{
				pIdxList[num++] = GET_PONSLOT(pAlmSrc->devAlarmSrc.devIdx);
				pIdxList[num++] = GET_PONPORT(pAlmSrc->devAlarmSrc.devIdx);
			}
			rc = VOS_OK;
			break;

		/* deviceIndex, boardIndex */
		case ALM_SRC_T_BRD:
		/* deviceIndex, devFanIndex */
		case ALM_SRC_T_FAN:
			pIdxList[num++] = OLT_DEV_ID;
			if( pAlmSrc->fanAlarmSrc.devIdx == OLT_DEV_ID )
			{
				pIdxList[num++] = pAlmSrc->fanAlarmSrc.brdIdx;
				pIdxList[num++] = pAlmSrc->fanAlarmSrc.fanIdx;
			}
			else
			{
				pIdxList[num++] = GET_PONSLOT(pAlmSrc->devAlarmSrc.devIdx);
				pIdxList[num++] = GET_PONPORT(pAlmSrc->devAlarmSrc.devIdx);
			}
			rc = VOS_OK;
			break;

		/* deviceIndex, ponPortBrdIndex, ponPortIndex */
		/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
		case ALM_SRC_T_PORT:
		case ALM_SRC_T_MON:
		case ALM_SRC_T_ETH_PWR:
		case ALM_SRC_T_LLID:
		/*case ALM_SRC_T_E1:
		case ALM_SRC_T_TDM:*/
			pIdxList[num++] = OLT_DEV_ID;
			if( pAlmSrc->portAlarmSrc.devIdx == OLT_DEV_ID )
			{
				pIdxList[num++] = pAlmSrc->portAlarmSrc.brdIdx;
				pIdxList[num++] = pAlmSrc->portAlarmSrc.portIdx;
			}
			else
			{
				pIdxList[num++] = GET_PONSLOT(pAlmSrc->portAlarmSrc.devIdx);
				pIdxList[num++] = GET_PONPORT(pAlmSrc->portAlarmSrc.devIdx);
			}
			rc = VOS_OK;
			break;

		/* deviceIndex, ponPortBrdIndex, ponPortIndex, onuIndex,oltPonReceiverPower*/
		case ALM_SRC_T_PON_PWR:
		case ALM_SRC_T_ONU_LASER:
			pIdxList[num++] = OLT_DEV_ID;
			pIdxList[num++] = pAlmSrc->oltRxpowerAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->oltRxpowerAlarmSrc.portIdx;
			rc = VOS_OK;
			break;

		case ALM_SRC_T_COMM:
			pIdxList[num++] = OLT_DEV_ID;
			pIdxList[num++] = pAlmSrc->commAlarmSrc.brdIdx;
			pIdxList[num++] = pAlmSrc->commAlarmSrc.portIdx;
			if( pAlmItem->alarmId == trap_onuDeletingNotify )
				pIdxList[num++] = pAlmSrc->commAlarmSrc.onuIdx;
			rc = VOS_OK;
			break;

		case ALM_SRC_T_ONU_SWITCH:
			pIdxList[num++] = OLT_DEV_ID;
			pIdxList[num++] = pAlmSrc->onuSwitchAlarmSrc.brdId;
			pIdxList[num++] = pAlmSrc->onuSwitchAlarmSrc.ponId;
			rc = VOS_OK;
			break;


		default:
			break;
	}

	*pIdxNum = num;
	
	return rc;
}

LONG alm_status_src_idx_comp( ULONG alarmType, ULONG alarmId, alarmSrc_t *pSrc1, alarmSrc_t *pSrc2 )
{
	LONG ret = -1;

	if( (pSrc1 == NULL) || (pSrc2 == NULL) )
		return ret;
	
	switch( alarmType )
	{
		case alarmType_mib2:
		case alarmType_bridge:
		case alarmType_other:
			break;
		case alarmType_private:
			switch( getEventAlarmSrcType(alarmType, alarmId) )
			{
				/* deviceIndex */
				case ALM_SRC_T_DEV:
					if( pSrc1->devAlarmSrc.devIdx == pSrc2->devAlarmSrc.devIdx )
					{
						ret = 0;
					}
					break;

				/* deviceIndex, boardIndex */
				case ALM_SRC_T_BRD:
					if( (pSrc1->brdAlarmSrc.devIdx == pSrc2->brdAlarmSrc.devIdx) &&
						(pSrc1->brdAlarmSrc.brdIdx == pSrc2->brdAlarmSrc.brdIdx)  )
					{
						ret = 0;
					}
					break;
				case ALM_SRC_T_FAN:
					if( (pSrc1->fanAlarmSrc.devIdx == pSrc2->fanAlarmSrc.devIdx) &&
						(pSrc1->fanAlarmSrc.brdIdx == pSrc2->fanAlarmSrc.brdIdx) &&
						(pSrc1->fanAlarmSrc.fanIdx == pSrc2->fanAlarmSrc.fanIdx) )
					{
						ret = 0;
					}
					break;

				case ALM_SRC_T_PORT:
				case ALM_SRC_T_MON:
				case ALM_SRC_T_ETH_PWR:
				/*case ALM_SRC_T_E1:
				case ALM_SRC_T_TDM:*/
					if( (pSrc1->portAlarmSrc.devIdx == pSrc2->portAlarmSrc.devIdx) &&
						(pSrc1->portAlarmSrc.brdIdx == pSrc2->portAlarmSrc.brdIdx) &&
						(pSrc1->portAlarmSrc.portIdx == pSrc2->portAlarmSrc.portIdx) )
					{
						ret = 0;
					}
					break;
					
				case ALM_SRC_T_COMM:
					/*if( (pSrc1->commAlarmSrc.devIdx == pSrc2->commAlarmSrc.devIdx) &&
						(pSrc1->commAlarmSrc.brdIdx == pSrc2->commAlarmSrc.brdIdx) &&
						(pSrc1->commAlarmSrc.portIdx == pSrc2->commAlarmSrc.portIdx)&&
						(VOS_MemCmp(pSrc1->commAlarmSrc.data, pSrc2->commAlarmSrc.data, 6) == 0) &&
						(pSrc1->commAlarmSrc.onuIdx == pSrc2->commAlarmSrc.onuIdx) )*/
					if( VOS_MemCmp(&pSrc1->commAlarmSrc, &pSrc2->commAlarmSrc, MAXLEN_EVENT_DATA) == 0 )
					{
						ret = 0;
					}
					break;

				case ALM_SRC_T_PON_PWR:
				case ALM_SRC_T_ONU_LASER:
					if( (pSrc1->oltRxpowerAlarmSrc.brdIdx == pSrc2->oltRxpowerAlarmSrc.brdIdx) &&
						(pSrc1->oltRxpowerAlarmSrc.portIdx == pSrc2->oltRxpowerAlarmSrc.portIdx) &&
						(pSrc1->oltRxpowerAlarmSrc.onuIdx == pSrc2->oltRxpowerAlarmSrc.onuIdx) )
					{
						ret = 0;
					}
					break;

				/*case ALM_SRC_T_ONU_LASER:
					if( (pSrc1->oltRxpowerAlwaysOnAlarmSrc.brdIdx == pSrc2->oltRxpowerAlwaysOnAlarmSrc.brdIdx) &&
						(pSrc1->oltRxpowerAlwaysOnAlarmSrc.portIdx == pSrc2->oltRxpowerAlwaysOnAlarmSrc.portIdx) &&
						(pSrc1->oltRxpowerAlwaysOnAlarmSrc.onuIdx == pSrc2->oltRxpowerAlwaysOnAlarmSrc.onuIdx) )
					{
						ret = 0;
					}
					break;*/

				/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
				case ALM_SRC_T_LLID:
					if( (pSrc1->llidAlarmSrc.devIdx == pSrc2->llidAlarmSrc.devIdx) &&
						(pSrc1->llidAlarmSrc.brdIdx == pSrc2->llidAlarmSrc.brdIdx) &&
						(pSrc1->llidAlarmSrc.portIdx == pSrc2->llidAlarmSrc.portIdx) &&
						(pSrc1->llidAlarmSrc.llidIdx == pSrc2->llidAlarmSrc.llidIdx) )
					{
						ret = 0;
					}
					break;

				case ALM_SRC_T_ONU_SWITCH:
					if( (pSrc1->onuSwitchAlarmSrc.brdId == pSrc2->onuSwitchAlarmSrc.brdId) &&
						(pSrc1->onuSwitchAlarmSrc.ponId == pSrc2->onuSwitchAlarmSrc.ponId) &&
						(pSrc1->onuSwitchAlarmSrc.onuId== pSrc2->onuSwitchAlarmSrc.onuId) &&
						(pSrc1->onuSwitchAlarmSrc.onuBrdId== pSrc2->onuSwitchAlarmSrc.onuBrdId)&&
						(pSrc1->onuSwitchAlarmSrc.onuPortId== pSrc2->onuSwitchAlarmSrc.onuPortId)&&
						(pSrc1->onuSwitchAlarmSrc.reason== pSrc2->onuSwitchAlarmSrc.reason)/*&&
						(pSrc1->onuSwitchAlarmSrc.switchMacAddr== pSrc2->onuSwitchAlarmSrc.switchMacAddr)*/)
					{
						ret = 0;
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	return ret;
}


static LONG del_alm_status_list_by_devIdx( ULONG devIdx )
{
	alm_status_list_t *pItem, *pPreItem;
	ULONG idxList[6], idxNum;

	pItem = alm_status_list;
	pPreItem = NULL;

	while( pItem )
	{
		if( devIdx )
		{
			idxNum = 0;
			if( alm_status_src_idx_get(&pItem->alm_item, idxList, &idxNum) != VOS_OK )
				idxList[0] = 0;
		}		
		if( (devIdx == 0) || ((idxList[0] == devIdx) && (idxNum != 0)) )
		{
			ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}
/*功能:根据devidx将当前告警表中的告警项删除*/
/*调用:在主备倒换时清除pon板告警；删除不想要的告警项*/
/*输入参数:devIdx 设备索引;**********************
*********level    告警严重级别*/
static LONG del_alm_status_list_by_devIdx_level( ULONG devIdx, ULONG level )
{
	alm_status_list_t *pItem, *pPreItem;
	ULONG idxList[6], idxNum;

	pItem = alm_status_list;
	pPreItem = NULL;

	if( ALM_LEV_MAX <= level )
		level = ALM_LEV_NULL;
	/*这个idxlist数组只是中间传递的作用，起一个判断的作用，实际的输出并没有用到*/
	while( pItem )
	{
		if( devIdx )
		{
			idxNum = 0;
			if( alm_status_src_idx_get(&pItem->alm_item, idxList, &idxNum) != VOS_OK )
				idxList[0] = 0;
		}
		if( level != ALM_LEV_NULL )
		{
			idxList[1] = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
		}
		if( ((devIdx == 0) || ((idxList[0] == devIdx) && (idxNum != 0))) &&
			((level == ALM_LEV_NULL) || (level == idxList[1])) )
		{
			ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}
/*根据告警ID将告警清除*/
static LONG del_alm_status_list_by_dev_alarmId( ULONG alarmId, ULONG devIdx )
{
	alm_status_list_t *pItem, *pPreItem;

	pItem = alm_status_list;
	pPreItem = NULL;

	if( devIdx == OLT_DEV_ID )
		devIdx = 0;

	while( pItem )
	{
		if( alarmId == pItem->alm_item.alarmId )
		{
			if( (devIdx == 0) || (devIdx == alm_status_src_dev_idx_get(&pItem->alm_item)) )
			{
				ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}

static LONG del_alm_status_list_by_onuType_alarmId( ULONG alarmId, ULONG onuType )
{
	ULONG devIdx, devType;
	ULONG idxList[6], idxNum;
	alm_status_list_t *pItem, *pPreItem;

	
	if( (V2R1_ONU_GT811 > onuType) || (onuType >= V2R1_ONU_MAX) )
		return VOS_ERROR;

	pItem = alm_status_list;
	pPreItem = NULL;

	while( pItem )
	{
		if( alarmId == pItem->alm_item.alarmId )
		{
			idxNum = 0;
			if( alm_status_src_idx_get(&pItem->alm_item, idxList, &idxNum) == VOS_OK )
				devIdx = idxList[0];
			else
				devIdx = 0;
			if( (devIdx == OLT_DEV_ID) || (idxNum == 0) )
				devIdx = 0;
			
			if( devIdx ) 
			{
				devType = 0;
				if( getDeviceType(devIdx, &devType) == VOS_OK)
				{
					if( devType == onuType )
					{
						ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
					}
				}
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}


static LONG del_alm_status_list_by_src_alarmId( ULONG alarmId,  alarmSrc_t *pSrc )
{
	alm_status_list_t *pItem, *pPreItem;

	if( pSrc == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pItem = alm_status_list;
	pPreItem = NULL;

	while( pItem )
	{
		if( alarmId == pItem->alm_item.alarmId )
		{
			if( alm_status_src_idx_comp(alarmType_private, alarmId, pSrc, &pItem->alm_item.alarmSrc) == 0 )
			{
				ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}

static LONG del_alm_status_list_by_src_associated_idx( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	alm_status_list_t *pItem, *pPreItem;
	ULONG almDevIdx, almBrdIdx, almPortIdx;
	ULONG idxList[8], idxNum;

	pItem = alm_status_list;
	pPreItem = NULL;

	while( pItem )
	{
		idxNum = 0;
		VOS_MemZero(idxList, 3*sizeof(ULONG)/*sizeof(idxList)*/ );
		
		if( alm_status_src_associated_idx_get(&pItem->alm_item, idxList, &idxNum) == VOS_OK )
		{
			almDevIdx = idxList[0];
			almBrdIdx = idxList[1];
			almPortIdx = idxList[2];
			
			if( ((devIdx == 0) || (devIdx == almDevIdx)) && ((brdIdx == 0) || (brdIdx == almBrdIdx)) && ((portIdx == 0) || (portIdx == almPortIdx)) )
			{
				ALM_STATUS_LIST_ITEM_DESTROY( pPreItem, pItem );
			}
		}
		pPreItem = pItem;
		pItem = pItem->next;
	}
	
	return VOS_OK;
}

static LONG get_dev_alm_top_level( ULONG devIdx )
{
	LONG level;
	LONG top_level = ALM_LEV_MAX;
	alm_status_list_t *pItem = NULL;

	pItem = alm_status_list;
	while( pItem )
	{
		if( (devIdx == 0) || (alm_status_src_dev_idx_get(&pItem->alm_item) == devIdx) )
		{
			level = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
			if( level == ALM_LEV_VITAL )
			{
				top_level = level;
				break;
			}
			if( (level != ALM_LEV_NULL) && (level < top_level) )
				top_level = level;
		}
		pItem = pItem->next;
	}
	if( top_level == ALM_LEV_MAX )
		top_level = ALM_LEV_NULL;

	return top_level;
}

static LONG get_brd_alm_top_level( ULONG devIdx, ULONG brdIdx )
{
	LONG level;
	LONG top_level = ALM_LEV_MAX;
	alm_status_list_t *pItem = NULL;
	ULONG alm_devIdx, alm_brdIdx;

	pItem = alm_status_list;
	while( pItem )
	{
		if( alm_status_src_brd_idx_get(&pItem->alm_item, &alm_devIdx, &alm_brdIdx) == VOS_OK )
		{
			if( (alm_devIdx == devIdx) && (alm_brdIdx == brdIdx) )
			{
				level = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
				if( level == ALM_LEV_VITAL )
				{
					top_level = level;
					break;
				}
				if( (level != ALM_LEV_NULL) && (level < top_level) )
					top_level = level;
			}
		}
		pItem = pItem->next;
	}
	if( top_level == ALM_LEV_MAX )
		top_level = ALM_LEV_NULL;

	return top_level;
}

static LONG get_dev_alm_all_level( ULONG devIdx, ULONG *pVital, ULONG *pMajor, ULONG *pMinor, ULONG *pWarn )
{
	ULONG level;
	alm_status_list_t *pItem = NULL;

	if( (pVital == NULL) || (pMajor == NULL) || (pMinor == NULL) ||(pWarn == NULL) )
		return VOS_ERROR;

	*pVital = ALM_LEV_NULL;
	*pMajor = ALM_LEV_NULL;
	*pMinor = ALM_LEV_NULL;
	*pWarn = ALM_LEV_NULL;

	pItem = alm_status_list;
	while( pItem )
	{
		if( (devIdx == 0 ) || (alm_status_src_dev_idx_get(&pItem->alm_item) == devIdx) )
		{
			level = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
			if( level == ALM_LEV_VITAL )
			{
				*pVital = level;
			}
			else if( level == ALM_LEV_MAJOR )
			{
				*pMajor = level;
			}
			else if( level == ALM_LEV_MINOR )
			{
				*pMinor = level;
			}
			else if( level == ALM_LEV_WARN )
			{
				*pWarn = level;
			}
		}
		pItem = pItem->next;
	}

	return VOS_OK;
}

static LONG get_sys_alm_top_level()
{
	return get_dev_alm_top_level( 0 );
}

static LONG get_sys_alm_all_level( ULONG *pVital, ULONG *pMajor, ULONG *pMinor, ULONG *pWarn )
{
	return get_dev_alm_all_level( 0, pVital, pMajor, pMinor, pWarn );
}

static ULONG get_dev_alm_status( ULONG devIdx )
{
	return 0;
}

static ULONG get_brd_alm_status( ULONG devIdx, ULONG brdIdx )
{
	return 0;
}

static ULONG get_eth_alm_status( ULONG devIdx, ULONG brdIdx, ULONG ethIdx )
{
	ULONG status = 0;
	alm_status_list_t *pItem = NULL;
	alarmSrc_t *pAlmSrc;
	
	pItem = alm_status_list;
	while( pItem )
	{
		if( pItem->alm_item.alarmType == alarmType_private )
		{
			pAlmSrc = &pItem->alm_item.alarmSrc;
			if( (devIdx != pAlmSrc->portAlarmSrc.devIdx ) || (brdIdx != pAlmSrc->portAlarmSrc.brdIdx) ||
				(ethIdx != pAlmSrc->portAlarmSrc.portIdx) )
			{
				pItem = pItem->next;
				continue;
			}
		
			switch( pItem->alm_item.alarmId )
			{
				case trap_backboneEthLinkdown:
				case trap_ethLinkdown:
					status |= EVENT_MASK_ETH_LINK;
					break;
				case trap_ethFerAlarm:
					status |= EVENT_MASK_ETH_FER;
					break;
				case trap_ethFlrAlarm:
					status |= EVENT_MASK_ETH_FLR;
					break;
				case trap_ethTranmittalIntermitAlarm:
					status |= EVENT_MASK_ETH_TI;
					break;
				case trap_ethLoopAlarm:
					status |= EVENT_MASK_ETH_LOOP;
					break;
				case trap_ethPortBroadCastFloodControl:
					status |= EVENT_MASK_ETH_BCFC;
					break;

				/*case trap_ethPortBroadCastFloodControl:
				case trap_uplinkReceiverPowerTooLow:
				case trap_uplinkReceiverPowerTooHigh:
				case trap_uplinkTransmissionPowerTooLow:
				case trap_uplinkTransmissionPowerTooHigh:
				case trap_uplinkAppliedVoltageTooHigh:
				case trap_uplinkAppliedVoltageTooLow:
				case trap_uplinkBiasCurrentTooHigh:
				case trap_uplinkBiasCurrentTooLow:
				case trap_uplinkTemperatureTooHigh:
				case trap_uplinkTemperatureTooLow:*/
				default:
					break;
			}
		}
		
		pItem = pItem->next;
	}
	return status;
}

static ULONG get_pon_alm_status( ULONG devIdx, ULONG brdIdx, ULONG ponIdx )
{
	ULONG status = 0;
	alm_status_list_t *pItem = NULL;
	alarmSrc_t *pAlmSrc;
	
	pItem = alm_status_list;
	while( pItem )
	{
		if( pItem->alm_item.alarmType == alarmType_private )
		{
			pAlmSrc = &pItem->alm_item.alarmSrc;
		
			switch( pItem->alm_item.alarmId )
			{
				case trap_ponPortBERAlarm:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_BER;/*0x80000000;*/
					break;
				case trap_ponPortFERAlarm:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_FER;/*0x40000000;*/
					break;
				case trap_ponPortAbnormal:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_ABNORMAL;/*0x20000000;*/
					break;
				case trap_autoProtectSwitch:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_APS;/*0x10000000;*/
					break;
				case trap_ponToEthLinkdown:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_LINK;/*0x08000000;*/
					break;
				case trap_ponLaserAlwaysOnAlarm:
					if( (devIdx == OLT_DEV_ID/*pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.devIdx*/ ) && 
						(brdIdx == pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->oltRxpowerAlwaysOnAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_LASER_ON;/*0x04000000;*/
					break;
				case trap_ponPortlosAlarm:
					if( (devIdx == pAlmSrc->portAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->portAlarmSrc.brdIdx) && (ponIdx == pAlmSrc->portAlarmSrc.portIdx) )
						status |= EVENT_MASK_PON_LOS;/*0x008000000;*/
					break;
					
				/*case trap_ponReceiverPowerTooLow:
				case trap_ponReceiverPowerTooHigh:
				case trap_ponTransmissionPowerTooLow:
				case trap_ponTransmissionPowerTooHigh:
				case trap_ponAppliedVoltageTooHigh:
				case trap_ponAppliedVoltageTooLow:
				case trap_ponBiasCurrentTooHigh:
				case trap_ponBiasCurrentTooLow:
				case trap_ponTemperatureTooHigh:
				case trap_ponTemperatureTooLow:

				case trap_oltPonReceiverPowerTooLow:
				case trap_oltPonReceiverPowerTooHigh:

				case trap_firmwareLoadFailure:
				case trap_dbaLoadFailure:
				case trap_PonPortFullAlarm:
				case trap_ponFWVersionMismatch:
		      		case trap_ponDBAVersionMismatch:
		      		case trap_ponSFPTypeMismatch:      

				case trap_ponportBRASAlarm:
				case trap_onuRegAuthFailure:*/
			}
		}

		pItem = pItem->next;
	}

	return status;
}

/* added by xieshl 20121211, 问题单16144 */
static ULONG get_fan_alm_status( ULONG devIdx, ULONG brdIdx, ULONG fanIdx )
{
	ULONG status = 0;
	alm_status_list_t *pItem = NULL;
	alarmSrc_t *pAlmSrc;
	
	pItem = alm_status_list;
	while( pItem )
	{
		if( pItem->alm_item.alarmType == alarmType_private )
		{
			pAlmSrc = &pItem->alm_item.alarmSrc;
		
			if( pItem->alm_item.alarmId == trap_devFanAlarm )
			{
				if( (devIdx == pAlmSrc->fanAlarmSrc.devIdx ) && (brdIdx == pAlmSrc->fanAlarmSrc.brdIdx) &&
					(fanIdx == pAlmSrc->fanAlarmSrc.fanIdx) )
				{
					status = EVENT_MASK_DEV_FAN;
					break;
				}
			}
		}
		
		pItem = pItem->next;
	}
	return status;
}

VOID initAlmStatus()
{
	return;
}
/*功能:记录当前告警状态*/
LONG eventStatusProc( eventMsg_t *pAlmMsg )
{
	LONG rc = VOS_ERROR;
	LONG alm_level;
	ULONG devIdx;
	
	if( pAlmMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}

	/* added by xieshl 20110526, 关联告警处理，问题单12909 */
	/*12909具体实现:
	ONU离线时，自动删除所有跟该ONU相关的告警
	ONU掉电时，自动删除除掉电外的所有跟该ONU相关的告警
	ONU被删除时，自动删除所有跟该ONU相关的告警*/
	switch( pAlmMsg->alarmType )
	{
		case alarmType_mib2:
		case alarmType_bridge:
		case alarmType_bcmcmcctrl:
			break;
		case alarmType_private:
			if( pAlmMsg->alarmId < trap_private_max )
			{
				rc = VOS_OK;
				
				switch( pAlmMsg->alarmId )
				{
					case trap_deviceColdStart:
					case trap_deviceWarmStart:
					case trap_deviceExceptionRestart:
						break;
					case trap_onuNewRegSuccess:
					case trap_onuReregSuccess:
					case trap_devPowerOn:
					case trap_devPowerOff:
					case trap_onuNotPresent:
						devIdx = pAlmMsg->alarmSrc.devAlarmSrc.devIdx;
						if( devIdx != OLT_DEV_ID )
						{
							/*clearDeviceAlarmStatus( devIdx );*/
							VOS_SemTake( eventSemId, WAIT_FOREVER );
							rc = del_alm_status_list_by_devIdx( devIdx );
							VOS_SemGive( eventSemId );
							return rc;
						}
						break;
					case trap_onuDeletingNotify:
						devIdx = MAKEDEVID( pAlmMsg->alarmSrc.commAlarmSrc.brdIdx, 
											pAlmMsg->alarmSrc.commAlarmSrc.portIdx,
											pAlmMsg->alarmSrc.commAlarmSrc.onuIdx );
						/*clearDeviceAlarmStatus( devIdx );*/
						VOS_SemTake( eventSemId, WAIT_FOREVER );
						rc = del_alm_status_list_by_devIdx( devIdx );
						VOS_SemGive( eventSemId );
						return rc;
					case trap_ponBoardReset:
					case trap_devBoardPull:
					case trap_devBoardInterted:
                    case trap_boardLosAlarm:
                    case trap_boardLosAlarmClear:
						break;

					/* deviceIndex, boardIndex, ponPortIndex */
					case trap_autoProtectSwitch:
					case trap_ponPortAbnormalClear:
					case trap_ponPortAbnormal:
					case trap_ponPortlosAlarm:
					case trap_ponPortlosAlarmClear:
					case trap_ponFWVersionMismatch:
			     		case trap_ponFWVersionMatch:
			      		case trap_ponDBAVersionMismatch:
			      		case trap_ponDBAVersionMatch:
			      		case trap_ponSFPTypeMismatch:      
			     		case trap_ponSFPTypeMitch:
						break;

					case trap_pwuStatusAbnoarmal:
					case trap_pwuStatusAbnoarmalClear:
						break;

					default:
						rc = VOS_ERROR;
						break;
				}

			}
			break;
		case alarmType_other:
			break;
		default:
			break;
	}

	/* 记录告警状态 */
	alm_level = getEventAlarmLevel( pAlmMsg->alarmType, pAlmMsg->alarmId );
	switch( alm_level )
	{
		case ALM_LEV_NULL:
			rc = VOS_OK;
			break;
		case ALM_LEV_VITAL:
		case ALM_LEV_MAJOR:
		case ALM_LEV_MINOR:
		case ALM_LEV_WARN:
			VOS_SemTake( eventSemId, WAIT_FOREVER );
			rc = alm_status_list_new( pAlmMsg );
			VOS_SemGive( eventSemId );
			break;
		case ALM_LEV_CLEAR:
			VOS_SemTake( eventSemId, WAIT_FOREVER );
			rc = alm_status_list_free( pAlmMsg );
			VOS_SemGive( eventSemId );
			break;
		default:
			break;
	}
	return rc;
}

/* added by xieshl 20110602, ONU离线或被删除时，在告警任务中清除其告警状态 */
LONG eventStatusClearProc( alm_status_cls_msg_t *pStatusMsg )
{
	LONG rc = VOS_ERROR;

	if( pStatusMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}

	if( pStatusMsg->msgCode == ALM_STA_CLS_MSG_CODE_DEV )
	{
		/*eventDbgPrintf(EVENT_DEBUGSWITCH_STA, ("ALM STA:Clear onu %d alarm status\r\n", pStatusMsg->devIdx) );*/
		
		VOS_SemTake( eventSemId, WAIT_FOREVER );
		rc = del_alm_status_list_by_devIdx( pStatusMsg->devIdx );
		VOS_SemGive( eventSemId );
	}
	
	return rc;
}



LONG getAlarmLevel( ULONG alm_id, ULONG *level )
{
	/*short int i;
	STATUS rc = VOS_ERROR;
	if( level == NULL )
		return rc;
	for( i=0; i<MAX_ALARM_LEVEL_ITEM; i++ )
	{
	    if( galmLevels[i].alm_id == 0 )
			break;
	    if( galmLevels[i].alm_id == idx )
	    {
	        *level = galmLevels[i].alm_level;
			rc = VOS_OK;
			break;
	    }
	}
	return rc*/
	if( level == NULL )
		return VOS_ERROR;
	*level = getEventAlarmLevel( alarmType_private, alm_id );
	return VOS_OK;
}

LONG setAlarmLevel( ULONG alm_id, ULONG level )
{
    /*short int i = 0;
    STATUS rc = VOS_ERROR;

	for( i=0; i<MAX_ALARM_LEVEL_ITEM;i++ )
	{
	    if( galmLevels[i].alm_id == 0 )
			break;
		if( galmLevels[i].alm_id == idx )
		{
		    galmLevels[i].alm_level = level;
			rc = VOS_OK;
			break;
		}
	}
	return rc;*/
	return setEventAlarmLevel( alarmType_private, alm_id, level );
}

LONG compAlmStatusSource( ULONG alarmType, ULONG trapId, alarmSrc_t *pSrc1, alarmSrc_t *pSrc2 )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = alm_status_src_idx_comp( alarmType, trapId, pSrc1, pSrc2 );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getDeviceAlarmTopLevel( const ULONG devIdx )
{
	LONG ret;
	/*ULONG status = 0;*/
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	if( devIdx == OLT_DEV_ID )
		ret = get_sys_alm_top_level();
	else
		ret = get_dev_alm_top_level( devIdx );
	VOS_SemGive( eventSemId );
#if 0
	if( (ret >= ALM_LEV_MAX) || (ret <= ALM_LEV_NULL) )	/* modified by xieshl 20110517, 设备在线时不能返回ALM_LEV_NULL，问题单12738 */
	{
		if( getDeviceOperStatus(devIdx, &status) == VOS_OK )
		{
			if( status == ONU_OPER_STATUS_UP )
				ret = ALM_LEV_CLEAR;
			else
				ret = ALM_LEV_NULL;
		}
	}
#endif
	return ret;
}

LONG getDeviceAllAlarmLevel( const ULONG devIdx, ULONG *pVital, ULONG *pMajor, ULONG *pMinor )
{
	LONG rc;
	ULONG warn = 0;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	if( devIdx == OLT_DEV_ID )
		rc = get_sys_alm_all_level( pVital, pMajor, pMinor, &warn );
	else
		rc = get_dev_alm_all_level( devIdx, pVital, pMajor, pMinor, &warn );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getBoardAlarmTopLevel( const ULONG devIdx, const ULONG brdIdx )
{
	LONG top_level;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	top_level = get_brd_alm_top_level( devIdx, brdIdx );
	VOS_SemGive( eventSemId );

	if( (top_level >= ALM_LEV_MAX) || (top_level == ALM_LEV_NULL) )
		top_level = ALM_LEV_CLEAR;

	return top_level;
}

LONG getDeviceAlarmStatusBitList( const ULONG devIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = get_dev_alm_status( devIdx );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getBoardAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = get_brd_alm_status( devIdx, brdIdx );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getEthPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ethIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = get_eth_alm_status( devIdx, brdIdx, ethIdx );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getPonPortAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG ponIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = get_pon_alm_status( devIdx, brdIdx, ponIdx );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG getFanAlarmStatusBitList( const ULONG devIdx, ULONG brdIdx, ULONG fanIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = get_fan_alm_status( devIdx, brdIdx, fanIdx );
	VOS_SemGive( eventSemId );
	return rc;
}
/* 基于OLT设备的告警屏蔽，需要删除相应告警状态 */
#if 0
LONG almStatusOltDevAlmMaskCallback( ULONG mask )
{
	ULONG devIdx = OLT_DEV_ID;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( mask & EVENT_MASK_DEV_POWER )
	{
		del_alm_status_list_by_dev_alarmId( trap_devPowerOff, devIdx );
		del_alm_status_list_by_dev_alarmId( trap_powerOffAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_FAN )
	{
		del_alm_status_list_by_dev_alarmId( trap_devFanAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_CPU )
	{
		/*del_alm_status_list_by_dev_alarmId( trap_cpuUsageFactorHigh, devIdx );*/
		del_alm_status_list_by_dev_alarmId( trap_boardCpuUsageAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_TEMPERATURE )
	{
		del_alm_status_list_by_dev_alarmId( trap_boardTemperatureHigh, devIdx );
	}
	if( mask & EVENT_MASK_DEV_REGISTER )
	{
		/*del_alm_status_list_by_dev_alarmId( trap_onuNewRegSuccess, devIdx );*/
		del_alm_status_list_by_dev_alarmId( trap_onuReregSuccess, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PRESENT )
	{
		del_alm_status_list_by_dev_alarmId( trap_onuNotPresent, devIdx );
	}
	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/* 基于ONU设备的告警屏蔽，需要删除相应告警状态 */
LONG almStatusOnuDevAlmMaskCallback( ULONG devIdx, ULONG mask )
#endif
/* 基于设备的告警屏蔽，需要删除相应告警状态 */
LONG almStatusDevAlmMaskCallback( ULONG devIdx, ULONG mask )
{
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( mask & EVENT_MASK_DEV_POWER )
	{
		del_alm_status_list_by_dev_alarmId( trap_devPowerOff, devIdx );
		del_alm_status_list_by_dev_alarmId( trap_powerOffAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_FAN )
	{
		del_alm_status_list_by_dev_alarmId( trap_devFanAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_CPU )
	{
		/*del_alm_status_list_by_dev_alarmId(trap_cpuUsageFactorHigh, devIdx);*/
		del_alm_status_list_by_dev_alarmId( trap_boardCpuUsageAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_TEMPERATURE )
	{
		del_alm_status_list_by_dev_alarmId( trap_boardTemperatureHigh, devIdx );
	}
	if( mask & EVENT_MASK_DEV_REGISTER )
	{
		/*del_alm_status_list_by_dev_alarmId(trap_onuNewRegSuccess, devIdx);*/
		del_alm_status_list_by_dev_alarmId( trap_onuReregSuccess, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PRESENT )
	{
		del_alm_status_list_by_dev_alarmId( trap_onuNotPresent, devIdx );
	}

	if( mask & EVENT_MASK_DEV_ETH_LINK )
	{
		del_alm_status_list_by_dev_alarmId( trap_ethLinkdown, devIdx );
	}
	if( mask & EVENT_MASK_DEV_ETH_FER )
	{
		del_alm_status_list_by_dev_alarmId( trap_ethFerAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_ETH_FLR )
	{
		del_alm_status_list_by_dev_alarmId( trap_ethFlrAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_ETH_TI )
	{
		del_alm_status_list_by_dev_alarmId( trap_ethTranmittalIntermitAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_ETH_LOOP )
	{
		del_alm_status_list_by_dev_alarmId( trap_ethLoopAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_BER )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponPortBERAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_FER )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponPortFERAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponPortAbnormal, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_ABS )
	{
		del_alm_status_list_by_dev_alarmId( trap_autoProtectSwitch, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_LINK )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponToEthLinkdown, devIdx );
	}
	if( mask & EVENT_MASK_DEV_ONU_LASER_ON )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponLaserAlwaysOnAlarm, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_POWER_L )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponReceiverPowerTooLow, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PON_POWER_H )
	{
		del_alm_status_list_by_dev_alarmId( trap_ponReceiverPowerTooHigh, devIdx );
	}
	if( mask & EVENT_MASK_DEV_PWU_STATUS)
	{
		del_alm_status_list_by_dev_alarmId( trap_pwuStatusAbnoarmal, devIdx ); /*add by @muqw 2017-4-26*/
	}

	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/* 基于ONU设备类型的告警屏蔽，需要删除相应告警状态 */
LONG almStatusOnuTypeAlmMaskCallback( ULONG onuType, ULONG mask )
{
	ULONG oldMask = 0;
	
	if( (V2R1_ONU_GT811 > onuType) || (onuType >= V2R1_ONU_MAX) )
		return VOS_ERROR;
	
	mask &= EVENT_MASK_DEV_ALL;
	if( mask )
	{
		if( getOnuTypeAlarmMask(onuType, &oldMask) == VOS_OK )
		{
			if( mask == oldMask )
				return VOS_OK;

			VOS_SemTake( eventSemId, WAIT_FOREVER );

			if( mask & EVENT_MASK_DEV_POWER )
			{
				del_alm_status_list_by_onuType_alarmId( trap_devPowerOff, onuType );
				del_alm_status_list_by_onuType_alarmId( trap_powerOffAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_FAN )
			{
				del_alm_status_list_by_onuType_alarmId( trap_devFanAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_CPU )
			{
				/*del_alm_status_list_by_dev_alarmId(trap_cpuUsageFactorHigh, onuType);*/
				del_alm_status_list_by_onuType_alarmId( trap_boardCpuUsageAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_TEMPERATURE )
			{
				del_alm_status_list_by_onuType_alarmId( trap_boardTemperatureHigh, onuType );
			}
			if( mask & EVENT_MASK_DEV_REGISTER )
			{
				/*del_alm_status_list_by_dev_alarmId(trap_onuNewRegSuccess, onuType);*/
				del_alm_status_list_by_onuType_alarmId( trap_onuReregSuccess, onuType );
			}
			if( mask & EVENT_MASK_DEV_PRESENT )
			{
				del_alm_status_list_by_onuType_alarmId( trap_onuNotPresent, onuType );
			}

			if( mask & EVENT_MASK_DEV_ETH_LINK )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ethLinkdown, onuType );
			}
			if( mask & EVENT_MASK_DEV_ETH_FER )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ethFerAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_ETH_FLR )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ethFlrAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_ETH_TI )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ethTranmittalIntermitAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_ETH_LOOP )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ethLoopAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_BER )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponPortBERAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_FER )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponPortFERAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponPortAbnormal, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_ABS )
			{
				del_alm_status_list_by_onuType_alarmId( trap_autoProtectSwitch, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_LINK )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponToEthLinkdown, onuType );
			}
			if( mask & EVENT_MASK_DEV_ONU_LASER_ON )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponLaserAlwaysOnAlarm, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_POWER_L )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponReceiverPowerTooLow, onuType );
			}
			if( mask & EVENT_MASK_DEV_PON_POWER_H )
			{
				del_alm_status_list_by_onuType_alarmId( trap_ponReceiverPowerTooHigh, onuType );
			}
			
			VOS_SemGive( eventSemId );
		}
	}
	return VOS_ERROR;
}

/* 基于PON口的告警屏蔽，需要删除相应告警状态 */
LONG almStatusPonPortAlmMaskCallback( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG mask )
{
	alarmSrc_t almSrc;
	
	VOS_MemZero( &almSrc, sizeof(alarmSrc_t) );
	almSrc.portAlarmSrc.devIdx = devIdx;
	almSrc.portAlarmSrc.brdIdx = brdIdx;
	almSrc.portAlarmSrc.portIdx = portIdx;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( mask & EVENT_MASK_PON_BER )
	{
		del_alm_status_list_by_src_alarmId( trap_ponPortBERAlarm, &almSrc );
	}
	if( mask & EVENT_MASK_PON_FER )
	{
		del_alm_status_list_by_src_alarmId( trap_ponPortFERAlarm, &almSrc );
	}
	if( mask & EVENT_MASK_PON_ABNORMAL )
	{
		del_alm_status_list_by_src_alarmId( trap_ponPortAbnormal, &almSrc );
	}
	if( mask & EVENT_MASK_PON_APS )
	{
		del_alm_status_list_by_src_alarmId( trap_autoProtectSwitch, &almSrc );
	}
	if( mask & EVENT_MASK_PON_LINK )
	{
		del_alm_status_list_by_src_alarmId( trap_ponToEthLinkdown, &almSrc );
	}

	VOS_SemGive( eventSemId );

	return VOS_OK;
}

/* 基于ETH口的告警屏蔽，需要删除相应告警状态 */
LONG almStatusEthPortAlmMaskCallback( ULONG devIdx, ULONG brdIdx, ULONG portIdx, ULONG mask )
{
	alarmSrc_t almSrc;
	
	VOS_MemZero( &almSrc, sizeof(alarmSrc_t) );
	almSrc.portAlarmSrc.devIdx = devIdx;
	almSrc.portAlarmSrc.brdIdx = brdIdx;
	almSrc.portAlarmSrc.portIdx = portIdx;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );

	if( mask & EVENT_MASK_ETH_LINK )
	{
		if( devIdx == OLT_DEV_ID )
			del_alm_status_list_by_src_alarmId( trap_backboneEthLinkdown, &almSrc );
		else
			del_alm_status_list_by_src_alarmId( trap_ethLinkdown, &almSrc );
	}
	if( mask & EVENT_MASK_ETH_FER )
	{
		del_alm_status_list_by_src_alarmId( trap_ethFerAlarm, &almSrc );
	}
	if( mask & EVENT_MASK_ETH_FLR )
	{
		del_alm_status_list_by_src_alarmId( trap_ethFlrAlarm, &almSrc );
	}
	if( mask & EVENT_MASK_ETH_TI )
	{
		del_alm_status_list_by_src_alarmId( trap_ethTranmittalIntermitAlarm, &almSrc );
	}

	VOS_SemGive( eventSemId );

	return VOS_OK;
}



/* 清除告警状态记录，主要用于特殊告警消除 */
LONG clearAlarmStatus( ULONG alarmId, alarmSrc_t *pAlmSrc )
{
	LONG rc;
	eventMsg_t almMsg;

	if( pAlmSrc == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	/*VOS_MemZero( &almMsg, sizeof(eventMsg_t) );*/
	almMsg.alarmType = alarmType_private;
	almMsg.alarmId = alarmId;
	VOS_MemCpy( &almMsg.alarmSrc, pAlmSrc, sizeof(alarmSrc_t) );
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = alm_status_list_free( &almMsg );
	VOS_SemGive( eventSemId );
	return rc;
}

/* 清除指定设备的告警状态记录，主要用于设备离线处理 */
LONG clearDeviceAlarmStatus( ULONG devIdx )
{
	/* modified by xieshl 20110602, ONU离线或被删除时，在告警任务中清除其告警状态 */
	/* 之前在ONU任务中清除时，存在信号量死锁问题，用64个ONU在不同的PON上反复注册可复现 */
	int rc = VOS_ERROR;
	ULONG ulMsg[4] = {MODULE_EVENT, FC_EVENT_STA_CLS, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	alm_status_cls_msg_t *pStatusMsg;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(alm_status_cls_msg_t);

	pstMsg = (SYS_MSG_S *)VOS_Malloc( msgLen, MODULE_EVENT ); 
	if( pstMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	pStatusMsg = (alm_status_cls_msg_t *)(pstMsg + 1);
	
	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (VOID *)pStatusMsg;
	pstMsg->usFrameLen = sizeof(alm_status_cls_msg_t);
	pstMsg->usMsgCode = FC_EVENT_STA_CLS;

	pStatusMsg->msgCode = ALM_STA_CLS_MSG_CODE_DEV;
	pStatusMsg->devIdx = devIdx;

	ulMsg[3] = (ULONG)pstMsg;
	rc = VOS_QueSend( eventQueId, ulMsg, NO_WAIT, ALM_PRI_LOW );
	if( rc != VOS_OK )
	{
		VOS_Free((void*)pstMsg);
	}
	return rc;	
}

ULONG ClearDeviceAndLevelAlarmStatusCallback(devsm_switchhover_notifier_event ulEvent )
{
	LONG rc = 0;
    
	if(ulEvent != switchhover_notify_start)
		return VOS_ERROR;

	if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
		return VOS_ERROR;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = del_alm_status_list_by_devIdx_level( 0, 0 );
	VOS_SemGive( eventSemId );

	return rc;
}

LONG clearDeviceAndLevelAlarmStatus( ULONG devIdx, ULONG level )
{
	LONG rc;
	eventSyncCfgMsg_t cfgMsg;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = del_alm_status_list_by_devIdx_level( devIdx, level );
	VOS_SemGive( eventSemId );

	/* 需要同步到PON板上 */
	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
		cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_STATUS;
		cfgMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
		cfgMsg.syncValue = level;
		cfgMsg.subCode = EVENT_MSG_SUBCODE_ALARM_CLEAR;

		rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
	}
	return rc;
}

/* 清除OLT上指定板卡的所有告警状态记录，主要用于热拔插 */
LONG clearOltBrdAlarmStatus( ULONG devIdx, ULONG brdIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = del_alm_status_list_by_src_associated_idx( devIdx, brdIdx, 0 );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG clearOltPortAlarmStatus( ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	LONG rc;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	rc = del_alm_status_list_by_src_associated_idx( devIdx, brdIdx, portIdx );
	VOS_SemGive( eventSemId );
	return rc;
}

LONG clearIllegalOnuAlarmStatus( ULONG onuDevIdx )
{
	LONG rc;
	UCHAR onuMacAddr[6];
	ULONG macAddrLen = 6;
	if( (rc = getDeviceMacAddr(onuDevIdx, onuMacAddr, &macAddrLen)) == VOS_OK )
	{
		alarmSrc_t almSrc;
		/*VOS_MemZero( &almSrc, sizeof(alarmSrc_t) );*/
		almSrc.commAlarmSrc.devIdx = OLT_DEV_ID;
		almSrc.commAlarmSrc.brdIdx = GET_PONSLOT(onuDevIdx);
		almSrc.commAlarmSrc.portIdx = GET_PONPORT(onuDevIdx);
		almSrc.commAlarmSrc.onuIdx = 0;
		VOS_MemCpy(almSrc.commAlarmSrc.data, onuMacAddr, 6);
		rc = clearAlarmStatus( trap_onuRegAuthFailure, &almSrc );
	}
	return rc;
}

LONG checkAlarmStatus( eventMsg_t *pAlmMsg  )
{
	LONG rc = VOS_OK;
	if( pAlmMsg->alarmType == alarmType_private )
	{
		/*if( pAlmMsg->alarmId == trap_onuRegAuthFailure )*/  /*add for PR38562 by @muqw 2017-6-1*/
		{
			VOS_SemTake( eventSemId, WAIT_FOREVER );
			if( alm_status_list_get(pAlmMsg) )	/* 该告警已存在 */
				rc = VOS_ERROR;
			VOS_SemGive( eventSemId );
		}
	}
	return rc;
}

LONG almStatusSetDeviceAlarmMask( ULONG devIdx, ULONG mask )
{
	LONG rc = VOS_ERROR;
	eventSyncCfgMsg_t cfgMsg;
	LOCATIONDES lct = {0,0,0};
	
	if(getLocation( devIdx, &lct, CONV_YES ) == VOS_OK )
	{
		/*VOS_SemTake( eventSemId, WAIT_FOREVER );*/
		rc = DeviceAlarmMaskSet(LOC2DevIdx(&lct), mask);
		/*VOS_SemGive( eventSemId );*/
	}
	if( rc != VOS_OK )
		return rc;

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
		cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_MASK;
		cfgMsg.alarmSrc.devAlarmSrc.devIdx = devIdx;
		cfgMsg.syncValue = mask;
			
		if( devIdx == OLT_DEV_ID )
		{
			cfgMsg.subCode = EVENT_MSG_SUBCODE_OLT_ALARM_MASK;
		}
		else
		{
			cfgMsg.subCode = EVENT_MSG_SUBCODE_ONU_ALARM_MASK;
		}
		
		/* modified by xieshl 20111116, 指定槽位的配置不需要广播，问题单13868 */
		if( devIdx == OLT_DEV_ID )
			rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
		else
			rc = eventSync_configData_2Slave( GET_PONSLOT(devIdx), (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
	}

	/*if( devIdx == OLT_DEV_ID )
	{
		almStatusOltDevAlmMaskCallback( mask );		
	}
	else
	{
		almStatusOnuDevAlmMaskCallback( devIdx, mask );
	}*/
	almStatusDevAlmMaskCallback( devIdx, mask );

	return rc;
}

LONG almStatusSetOnuTypeAlarmMask( ULONG onuType, ULONG mask )
{
	LONG rc;
	eventSyncCfgMsg_t cfgMsg;

	if( (V2R1_ONU_GT811 > onuType) || (onuType >= V2R1_ONU_MAX) )
		return VOS_ERROR;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	onu_type_mask[onuType] = mask;
	VOS_SemGive( eventSemId );

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
		cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_MASK;
		cfgMsg.subCode = EVENT_MSG_SUBCODE_ONUTYPE_ALARM_MASK;
		cfgMsg.alarmSrc.devAlarmSrc.devData = onuType;
		cfgMsg.syncValue = mask;

		rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
	}
	almStatusOnuTypeAlmMaskCallback( onuType, mask );

	return rc;
}

LONG almStatusSetPonPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG portIdx, const ULONG mask )
{
	LONG rc = VOS_ERROR;
	LOCATIONDES lct;
	eventSyncCfgMsg_t cfgMsg;

	if( getLocation( devIdx, &lct, CONV_YES ) == VOS_OK )
	{
		rc = PonPortAlarmMaskSet( LOC2DevIdx(&lct), brdIdx, portIdx, mask );
		if( rc == ROK )
		{
			if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
			{
				VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
				cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_MASK;
				cfgMsg.subCode = EVENT_MSG_SUBCODE_PON_ALARM_MASK;
				cfgMsg.alarmSrc.portAlarmSrc.devIdx = devIdx;
				cfgMsg.alarmSrc.portAlarmSrc.brdIdx = brdIdx;
				cfgMsg.alarmSrc.portAlarmSrc.portIdx = portIdx;
				cfgMsg.syncValue = mask;

				/* modified by xieshl 20111116, 指定槽位的配置不需要广播，问题单13868 */
				if( SYS_SLOTNO_IS_BROADCAST_SLOTNO(brdIdx) )
					rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
				else
					rc = eventSync_configData_2Slave( brdIdx, (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t)  );
			}
			almStatusPonPortAlmMaskCallback( devIdx, brdIdx, portIdx, mask );
		}
	}
	return rc;
}

LONG almStatusSetEthPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG portIdx, const ULONG mask )
{
	LONG rc = VOS_ERROR;
	eventSyncCfgMsg_t cfgMsg;

	if( devIdx == OLT_DEV_ID )
		rc = OltEthPortAlarmMaskSet( devIdx, brdIdx, portIdx, mask );
	else
		rc = OnuEthPortAlarmMaskSet(mask);

	if( rc == VOS_OK )
	{
		if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
			cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_MASK;
			cfgMsg.subCode = EVENT_MSG_SUBCODE_ETH_ALARM_MASK;
			cfgMsg.alarmSrc.portAlarmSrc.devIdx = devIdx;
			cfgMsg.alarmSrc.portAlarmSrc.brdIdx = brdIdx;
			cfgMsg.alarmSrc.portAlarmSrc.portIdx = portIdx;
			cfgMsg.syncValue = mask;

			rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
		}
		almStatusEthPortAlmMaskCallback( devIdx, brdIdx, portIdx, mask );		
	}
	return rc;
}
LONG almStatusSetOnuSwitchAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG portIdx, const ULONG mask )
{
	LONG rc = VOS_ERROR;
	eventSyncCfgMsg_t cfgMsg;

	if( devIdx == OLT_DEV_ID )
	{
		return VOS_OK;
	}
	else
		rc = OnuSwitchAlarmMaskSet(mask);

	if( rc == VOS_OK )
	{
		if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		{
			VOS_MemZero( &cfgMsg, sizeof(eventSyncCfgMsg_t) );
			cfgMsg.subType = EVENT_MSG_SUBTYPE_ALARM_MASK;
			cfgMsg.subCode = EVENT_MSG_SUBCODE_ONUSWITCH_ALARM_MASK;
			cfgMsg.alarmSrc.portAlarmSrc.devIdx = devIdx;
			cfgMsg.alarmSrc.portAlarmSrc.brdIdx = brdIdx;
			cfgMsg.alarmSrc.portAlarmSrc.portIdx = portIdx;
			cfgMsg.syncValue = mask;

			rc = eventSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(eventSyncCfgMsg_t) );
		}
		/*almStatusEthPortAlmMaskCallback( devIdx, brdIdx, portIdx, mask );		*/
	}
	return rc;
}

LONG eventSync_configData_2Slave( ULONG slotno, VOID *pCfgData, ULONG dataLen )
{
	LONG rc;
	SYS_MSG_S   *pstMsg;
	eventSyncCfgMsg_t *pSndCfgMsg;
	ULONG msgLen;

	if( (pCfgData == NULL) || (dataLen == 0) || (dataLen >sizeof(eventSyncCfgMsg_t)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( !SYS_MODULE_SLOT_ISHAVECPU(slotno) )
		return VOS_OK;

	msgLen = sizeof(SYS_MSG_S) + dataLen;
	
	if(  SYS_MODULE_IS_READY(slotno) && (slotno != SYS_LOCAL_MODULE_SLOTNO) )
	{
		pstMsg = CDP_AllocMsg( msgLen, MODULE_EVENT );
		if ( NULL == pstMsg )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		pSndCfgMsg = (eventSyncCfgMsg_t *)(pstMsg + 1);

		VOS_MemZero( pstMsg, msgLen );
		pstMsg->ulSrcModuleID = MODULE_EVENT;
		pstMsg->ulDstModuleID = MODULE_EVENT;
		pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
		pstMsg->ulDstSlotID = slotno;/*目的slot*/
		pstMsg->ucMsgType = MSG_NOTIFY;
		pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
		pstMsg->ptrMsgBody = (VOID *)pSndCfgMsg;
		pstMsg->usFrameLen = dataLen;
		pstMsg->usMsgCode = FC_EVENT_CFG_SYNC;

		VOS_MemCpy( pSndCfgMsg, pCfgData, dataLen );
				
		rc = CDP_Send( RPU_TID_CDP_EVENT, slotno,  RPU_TID_CDP_EVENT,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_EVENT );
		if( rc !=  VOS_OK )
		{
			VOS_ASSERT(0);
			CDP_FreeMsg( (void *) pstMsg );
			return VOS_ERROR;
		}	
	}

	return VOS_OK;
}
LONG eventSync_configData_2AllSlave( VOID *pCfgData, ULONG dataLen )
{
	ULONG slotno;

	if( pCfgData == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return VOS_OK;
	}

	for( slotno = 1; slotno <= SYS_CHASSIS_SLOTNUM; slotno++ )
	{
		eventSync_configData_2Slave( slotno, pCfgData, dataLen );
	}
	return VOS_OK;
}
/*设置告警屏蔽使能*/
LONG eventSync_setAlarmMask( eventSyncCfgMsg_t *pCfgMsg )
{
	LONG rc = VOS_ERROR;
	
	if( pCfgMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	
	switch( pCfgMsg->subCode )
	{
		case EVENT_MSG_SUBCODE_OLT_ALARM_MASK:
			rc = almStatusSetDeviceAlarmMask( pCfgMsg->alarmSrc.devAlarmSrc.devIdx, pCfgMsg->syncValue );
			break;
		case EVENT_MSG_SUBCODE_ONU_ALARM_MASK:
			rc = almStatusSetDeviceAlarmMask( pCfgMsg->alarmSrc.devAlarmSrc.devIdx, pCfgMsg->syncValue );
			break;
		case EVENT_MSG_SUBCODE_ONUTYPE_ALARM_MASK:
			rc = almStatusSetOnuTypeAlarmMask( pCfgMsg->alarmSrc.devAlarmSrc.devData, pCfgMsg->syncValue );
			break;
		case EVENT_MSG_SUBCODE_PON_ALARM_MASK:
			rc = almStatusSetPonPortAlarmMask( pCfgMsg->alarmSrc.portAlarmSrc.devIdx, pCfgMsg->alarmSrc.portAlarmSrc.brdIdx, 
				pCfgMsg->alarmSrc.portAlarmSrc.portIdx, pCfgMsg->syncValue );
			break;
		case EVENT_MSG_SUBCODE_ETH_ALARM_MASK:
			rc = almStatusSetEthPortAlarmMask( pCfgMsg->alarmSrc.portAlarmSrc.devIdx, pCfgMsg->alarmSrc.portAlarmSrc.brdIdx, 
				pCfgMsg->alarmSrc.portAlarmSrc.portIdx, pCfgMsg->syncValue );
			break;
        case EVENT_MSG_SUBCODE_ONUSWITCH_ALARM_MASK:
			rc = almStatusSetOnuSwitchAlarmMask( pCfgMsg->alarmSrc.portAlarmSrc.devIdx, pCfgMsg->alarmSrc.portAlarmSrc.brdIdx, 
				pCfgMsg->alarmSrc.portAlarmSrc.portIdx, pCfgMsg->syncValue );
			break;
		default:
			break;
	}
	return rc;
}

LONG eventSync_setAlarmStatus( eventSyncCfgMsg_t *pCfgMsg )
{
	LONG rc = VOS_ERROR;

	if( pCfgMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}
	if( pCfgMsg->subCode == EVENT_MSG_SUBCODE_ALARM_CLEAR )
	{
		rc = clearDeviceAndLevelAlarmStatus( pCfgMsg->alarmSrc.devAlarmSrc.devIdx, pCfgMsg->syncValue );
	}
	return rc;
}
/*功能:将告警严重程度转化为字符串，用于命令行打印输出*/
CHAR *alm_level_2_str( ULONG level )
{
	CHAR *pStr = "-";
	switch( level )
	{
		case ALM_LEV_VITAL:
			pStr = "VITAL";
			break;
		case ALM_LEV_MAJOR:
			pStr = "MAJOR";
			break;
		case ALM_LEV_MINOR:
			pStr = "MINOR";
			break;
		case ALM_LEV_WARN:
			pStr = "WARN";
			break;
		case ALM_LEV_CLEAR:
			pStr = "CLEAR";
			break;
		default:
			break;
	}
	return pStr;
}

LONG showCurrentAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx )
{
	CHAR pAlmStr[MAXLEN_EVENT_DESC+1];
	alm_status_list_t *pItem;
	LONG count = 0;
	ULONG level;

#if 1
	vty_out( vty, "\r\n" );

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pItem = alm_status_list;
	while( pItem )
	{
		level = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
		if( ((almLevel == 0) || (level == almLevel)) &&
			((devIdx == 0) || (alm_status_src_dev_idx_get(&pItem->alm_item) == devIdx)) )
		{
			eventLogDataToStrings( (VOID*)&pItem->alm_item, pAlmStr );
			if( pAlmStr[0] )
			{
				/*if( count == 0 )
				{
					vty_out( vty, " L1-vital,L2-major,L3-minor,L4-warning\r\n" )
					vty_out( vty, " -------------------------------------\r\n" );
				}*/
					
				if( almLevel )
					vty_out( vty, "%s\r\n", pAlmStr );
				else
					vty_out( vty, "[%s]%s\r\n", alm_level_2_str(level), pAlmStr );
				count++;
			}
		}
		pItem = pItem->next;
	}
	VOS_SemGive( eventSemId );
#else
	alm_status_list_t *p_temp_item, *p_temp_list = NULL;

	vty_out( vty, "\r\n" );

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pItem = alm_status_list;
	while( pItem )
	{
		level = getEventAlarmLevel( pItem->alm_item.alarmType, pItem->alm_item.alarmId );
		if( ((almLevel == 0) || (level == almLevel)) &&
			((devIdx == 0) || (alm_status_src_dev_idx_get(&pItem->alm_item) == devIdx)) )
		{
			p_temp_item = VOS_Malloc( sizeof(alm_status_list_t), MODULE_EVENT );
			if( p_temp_item == NULL )
			{
				break;
			}
			VOS_MemCpy( p_temp_item, pItem, sizeof(alm_status_list_t) );
			p_temp_item->next = p_temp_list;
			p_temp_list = p_temp_item;
		}
		pItem = pItem->next;
	}
	VOS_SemGive( eventSemId );

	pItem = p_temp_list;
	while( pItem )
	{
		pAlmStr = eventLogDataToStrings( (VOID*)&pItem->alm_item );
		if( pAlmStr )
		{
			if( almLevel )
				vty_out( vty, "%s\r\n", pAlmStr );
			else
				vty_out( vty, "[%s]%s\r\n", alm_level_2_str(level), pAlmStr );
			count++;
		}
		p_temp_item = pItem;
		pItem = pItem->next;

		VOS_Free( p_temp_item );
	}
#endif

	if( count == 0 )
		vty_out( vty, " Not find alarm source\r\n\r\n" );

	return CMD_SUCCESS;
}

LONG clearDevAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx )
{
	clearDeviceAndLevelAlarmStatus( devIdx, almLevel );

	return CMD_SUCCESS;
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */

