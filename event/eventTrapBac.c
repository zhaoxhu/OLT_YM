#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "../../mib/gwEponSys.h"
#include "snmp/sn_a_trp.h"
#include "snmp/sn_snmp.h"
#include "Vos_time.h"

#include "eventMain.h"


/* �澯ͬ�� */
static ULONG 		trap_bac_list_header = 0;				/*��һ��ָʾ�±�*/
static ULONG 		trap_bac_list_idx = 0;					/*��ǰ��ָʾ�±�*/
static ULONG 		trap_bac_enable = TRAP_BAC_ENABLE;		/* �澯trap����ʹ�� */
static trapBackupList_t	trap_bac_list[TRAP_BAC_LIST_SIZE];	/* �澯trap���ݹ���� */

extern INT32 code_snmp_trap_packet( SNMPTRAP * SnmpTrap, UCHAR * packet,  size_t *out_length );
extern STATUS bindTrapVar( SNMPTRAP *trapvar, ULONG trapId, ULONG *pVarlist, const ULONG varNum );
extern LONG devsm_get_slocal_time( SLOCAL_TIME * time );
extern LONG alm_status_src_idx_comp( ULONG alarmType, ULONG alarmId, alarmSrc_t *pSrc1, alarmSrc_t *pSrc2 );

/*----------------------------------------------------------------------------*/

/* �澯ͬ�� */

/*----------------------------------------------------------------------------*/

/* �澯ͬ�����ݳ�ʼ����������0 */
LONG initAlarmTrapBac()
{
	trap_bac_list_header = 0;
	trap_bac_list_idx = 0;
	trap_bac_enable = TRAP_BAC_ENABLE;
	VOS_MemZero( trap_bac_list, TRAP_BAC_LIST_SIZE * sizeof(trapBackupList_t) );

	/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Init OK\r\n") );*/
	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* ���ݴ洢���±�͸澯ͬ���������Ķ�Ӧ */
LONG trapBacListSuffix2Index( ULONG suffix, ULONG *pIdxs )
{
	if( suffix >= TRAP_BAC_LIST_SIZE || pIdxs == NULL )
		return VOS_ERROR;
	*pIdxs = trap_bac_list[suffix].bacIdx;
	return VOS_OK;
}

/*----------------------------------------------------------------------------
* ����: ���澯ͬ�������ǰ�е����ݴ洢ָ��
* �������: synIdx����ǰ�е�ͬ������
* �������: 
* ����ֵ: ��ȷ���ص�ǰ�����ݵĴ洢ָ�룬���󷵻�NULL */
static trapBackupList_t* seekTrapBacListByIndex( ULONG idx )
{
	LONG i;
	trapBackupList_t *pEntry = NULL;
	for( i=0; i<TRAP_BAC_LIST_SIZE; i++ )
	{
		if( trap_bac_list[i].bacIdx == idx )
		{
			pEntry = &trap_bac_list[i];
			break;
		}
	}
	return pEntry;
}

static trapBackupList_t* seekTrapBacListByAlarmSrc( ULONG alarmType, ULONG trapId, alarmSrc_t *pAlarmSrc )
{
	trapBackupList_t *pEntry = NULL;
	LONG i = trap_bac_list_idx;
	while( i < TRAP_BAC_LIST_SIZE )
	{
		if( (trap_bac_list[i].alarmType == alarmType) && (trap_bac_list[i].trapId == trapId)  )
		{
			if( alm_status_src_idx_comp( alarmType, trapId, &trap_bac_list[i].alarmSrc, pAlarmSrc) == 0 )
			{
				pEntry = &trap_bac_list[i];
			}
			
#if 0
			switch( alarmType )
			{
				case alarmType_mib2:
				case alarmType_bridge:
				case alarmType_other:
					break;
				case alarmType_private:
					switch( trapId )
					{
						case trap_onuNotPresent:
						case trap_onuNewRegSuccess:
						case trap_onuReregSuccess:
						case trap_devPowerOff:
						case trap_devPowerOn:
						case trap_sysfileUploadsuccess:/*add by shixh20090604*/
						case trap_sysfileUploadfailure:
						case trap_sysfileDownloadsuccess:
						case trap_sysfileDownloadfailure:
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
						case trap_onuAutoLoadConfigSuccess:/*add by shixh@20070312*/
						case trap_onuAutoLoadConfigFailure:
						case trap_onuAutoLoadUpgradeSuccess:/*add by shixh@20070312*/
						case trap_onuAutoLoadUpgradeFailure:
#endif
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
						case trap_deviceTemperatureHigh:
						case trap_deviceTemperatureHighClear:
						case trap_deviceTemperatureLow:
						case trap_deviceTemperatureLowClear:
#endif
							if( pAlarmSrc->devAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.devAlarmSrc.devIdx )
							{
								pEntry = &trap_bac_list[i];
							}
							break;
							
						case trap_cfgDataSaveSuccess:
						case trap_cfgDataSaveFail:
						case trap_flashClearSuccess:
						case trap_flashClearFail:
						case trap_softwareUpdateSuccess:
						case trap_softwareUpdateFail:
						case trap_firmwareUpdateSuccess:
						case trap_firmwareUpdateFail:
						case trap_cfgDataBackupSuccess:
						case trap_cfgDataBackupFail:
						case trap_cfgDataRestoreSuccess:
						case trap_cfgDataRestoreFail:
						case trap_cpuUsageFactorHigh:
						case trap_dbaUpdateSuccess:
						case trap_dbaUpdateFailure:
						case trap_onuSoftwareLoadSuccess:
						case trap_onuSoftwareLoadFailure:
						case trap_onuRegisterConflict:
						
						case trap_bootUpdateSuccess:
						case trap_bootUpdateFailure:
						case trap_batfileBackupSuccess:
						case trap_batfileBackupFailure:
						case trap_batfileRestoreSuccess:
						case trap_batfileRestoreFailure:

						case trap_deviceColdStart:
						case trap_deviceWarmStart:
						case trap_deviceExceptionRestart:
							break;

						/* deviceIndex, boardIndex */
						case trap_swBoardProtectedSwitch:
						case trap_boardTemperatureHigh:
						case trap_boardTemperatureHighClear:
						case trap_ponBoardReset:
								
							break;
							
							/* deviceIndex, boardIndex *//*add by shixh@200808031*/
						case trap_boardCpuUsageAlarm:   
						case trap_boardCpuUsageAlarmClear:
						case trap_boardMemoryUsageAlarm:
						case trap_boardMemoryUsageAlarmClear:
							if( (pAlarmSrc->brdAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.brdAlarmSrc.devIdx) &&
								(pAlarmSrc->brdAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.brdAlarmSrc.brdIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						/* deviceIndex, boardIndex, curBoardType */
						case trap_devBoardInterted:
						case trap_devBoardPull:
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
						/*case trap_PWUPowerOff:
						case trap_PWUPowerOn:*/
#endif
						/* begin: added by jianght 20090520 */
							if( (pAlarmSrc->brdAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.brdAlarmSrc.devIdx) &&
								(pAlarmSrc->brdAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.brdAlarmSrc.brdIdx) &&
								(pAlarmSrc->brdAlarmSrc.brdType == trap_bac_list[i].alarmSrc.brdAlarmSrc.brdType) )
							{
								pEntry = &trap_bac_list[i];
							}
						/* end: added by jianght 20090520 */

							break;

						/* deviceIndex, boardIndex */
						case trap_powerOffAlarm:
						case trap_powerOnAlarm:
							if( (pAlarmSrc->devAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.devAlarmSrc.devIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						/* deviceIndex, devFanIndex */
						case trap_devFanAlarm:
						case trap_devFanAlarmClear:
							if( (pAlarmSrc->fanAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.fanAlarmSrc.devIdx) &&
								(pAlarmSrc->fanAlarmSrc.fanIdx == trap_bac_list[i].alarmSrc.fanAlarmSrc.fanIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponPortBER *//*modyfied by shixh@20080830*/
						case trap_ponPortBERAlarm:
						case trap_ponPortBERAlarmClear:
						case trap_ponPortFERAlarm:
						case trap_ponPortFERAlarmClear:
						case trap_ponToEthLinkdown:	
						case trap_ponToEthLinkup:
							
						/* added by xieshl 20080812 */	
						case trap_ponReceiverPowerTooLow:
						case trap_ponReceiverPowerTooLowClear:
						case trap_ponReceiverPowerTooHigh:
						case trap_ponReceiverPowerTooHighClear:
						case trap_ponTransmissionPowerTooLow:
						case trap_ponTransmissionPowerTooLowClear:
						case trap_ponTransmissionPowerTooHigh:
						case trap_ponTransmissionPowerTooHighClear:
						case trap_ponAppliedVoltageTooHigh:
						case trap_ponAppliedVoltageTooHighClear:
						case trap_ponAppliedVoltageTooLow:
						case trap_ponAppliedVoltageTooLowClear:
						case trap_ponBiasCurrentTooHigh:
						case trap_ponBiasCurrentTooHighClear:
						case trap_ponBiasCurrentTooLow:
						case trap_ponBiasCurrentTooLowClear:
						/* end 20080812 */
						case trap_ponTemperatureTooHigh:
						case trap_ponTemperatureTooHighClear:
						case trap_ponTemperatureTooLow:
						case trap_ponTemperatureTooLowClear:
							/*add by shixh20090507*/
						case trap_ponPortlosAlarm:       /*add by shixh20090626*/
						case trap_ponPortlosAlarmClear:
						case trap_ponFWVersionMismatch:/*add by shixh20090710*/
					       case trap_ponFWVersionMatch:
					      	case trap_ponDBAVersionMismatch:
					      	case trap_ponDBAVersionMatch:
					      	case trap_ponSFPTypeMismatch:      
					     	case trap_ponSFPTypeMitch:
							if( (pAlarmSrc->monAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.monAlarmSrc.devIdx) &&
								(pAlarmSrc->monAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.monAlarmSrc.brdIdx) &&
								(pAlarmSrc->monAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.monAlarmSrc.portIdx)/*&&
								(pAlarmSrc->monAlarmSrc.monValue == trap_bac_list[i].alarmSrc.monAlarmSrc.monValue)*/)
							{
								pEntry = &trap_bac_list[i];
							}
							break;
						case trap_uplinkReceiverPowerTooLow:
						case trap_uplinkReceiverPowerTooLowClear:
						case trap_uplinkReceiverPowerTooHigh:
						case trap_uplinkReceiverPowerTooHighClear:
						case trap_uplinkTransmissionPowerTooLow:
						case trap_uplinkTransmissionPowerTooLowClear:
						case trap_uplinkTransmissionPowerTooHigh:
						case trap_uplinkTransmissionPowerTooHighClear:
						case trap_uplinkAppliedVoltageTooHigh:
						case trap_uplinkAppliedVoltageTooHighClear:
						case trap_uplinkAppliedVoltageTooLow:
						case trap_uplinkAppliedVoltageTooLowClear:
						case trap_uplinkBiasCurrentTooHigh:
						case trap_uplinkBiasCurrentTooHighClear:
						case trap_uplinkBiasCurrentTooLow:
						case trap_uplinkBiasCurrentTooLowClear:
						case trap_uplinkTemperatureTooHigh:
						case trap_uplinkTemperatureTooHighClear:
						case trap_uplinkTemperatureTooLow:
						case trap_uplinkTemperatureTooLowClear:
							if( (pAlarmSrc->uplinkAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.uplinkAlarmSrc.devIdx) &&
								(pAlarmSrc->uplinkAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.uplinkAlarmSrc.brdIdx) &&
								(pAlarmSrc->uplinkAlarmSrc.ethIdx == trap_bac_list[i].alarmSrc.uplinkAlarmSrc.ethIdx)/*&&
								(pAlarmSrc->monAlarmSrc.monValue == trap_bac_list[i].alarmSrc.monAlarmSrc.monValue)*/)
							{
								pEntry = &trap_bac_list[i];
							}
							break;
							
						case trap_ponportBRASAlarm:     /*add by shixh20090715*/
						case trap_ponportBRASAlarmClear:
							if( (pAlarmSrc->commAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.commAlarmSrc.devIdx) &&
								(pAlarmSrc->commAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.commAlarmSrc.brdIdx) &&
								(pAlarmSrc->commAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.commAlarmSrc.portIdx)&&
								VOS_MemCmp(pAlarmSrc->commAlarmSrc.data,trap_bac_list[i].alarmSrc.commAlarmSrc.data,6)==0)
							{
								pEntry = &trap_bac_list[i];
							}
							break;
							/*add by shixh@20080831*/
						case trap_oltPonReceiverPowerTooLow:
	                                   case trap_oltPonReceiverPowerTooLowClear:
	                                   case trap_oltPonReceiverPowerTooHigh:
	                                   case trap_oltPonReceiverPowerTooHighClear:
							if( (pAlarmSrc->oltRxpowerAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.oltRxpowerAlarmSrc.brdIdx) &&
								(pAlarmSrc->oltRxpowerAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.oltRxpowerAlarmSrc.portIdx) &&
								(pAlarmSrc->oltRxpowerAlarmSrc.onuIdx == trap_bac_list[i].alarmSrc.oltRxpowerAlarmSrc.onuIdx)/*&&
								(pAlarmSrc->oltRxpowerAlarmSrc.oltrxValue == trap_bac_list[i].alarmSrc.oltRxpowerAlarmSrc.oltrxValue)*/)
							{
								pEntry = &trap_bac_list[i];
							}
										break;
						/* deviceIndex, ponPortBrdIndex, ponPortIndex, ponLlidIndex */
						case trap_llidActBWExceeding:
						case trap_llidActBWExceedingClear:
							if( (pAlarmSrc->llidAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.llidAlarmSrc.devIdx) &&
								(pAlarmSrc->llidAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.llidAlarmSrc.brdIdx) &&
								(pAlarmSrc->llidAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.llidAlarmSrc.portIdx) &&
								(pAlarmSrc->llidAlarmSrc.llidIdx == trap_bac_list[i].alarmSrc.llidAlarmSrc.llidIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						/* deviceIndex, boardIndex, ponPortIndex */
						case trap_autoProtectSwitch:
						/*case trap_ponPortAbnormal:*/
						case trap_firmwareLoadSuccess:
						case trap_firmwareLoadFailure:
						case trap_dbaLoadSuccess:
						case trap_dbaLoadFailure:
						case trap_ponLaserAlwaysOnAlarm:		/* added by xieshl 20080812 */
						case trap_ponLaserAlwaysOnAlarmClear:
						case trap_PonPortFullAlarm:
						case trap_ponPortAbnormalClear:

							break;
						
						/* devIndex, boardIndex, portIndex */
						case trap_ethFlrAlarm:
						case trap_ethFlrAlarmClear:
						case trap_ethFerAlarm:
						case trap_ethFerAlarmClear:
						case trap_ethTranmittalIntermitAlarm:
						case trap_ethTranmittalIntermitAlarmClear:
						case trap_ethLinkdown:
						case trap_ethLinkup:
						case trap_ponPortAbnormal:	/*add by shixh20090625*/
							if( (pAlarmSrc->portAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.devIdx) &&
								(pAlarmSrc->portAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.brdIdx) &&
								(pAlarmSrc->portAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.portIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						case trap_onuRegAuthFailure:
							break;
							
							/*add by shixh@20070929*/
						/*devIdx,  brdIdx,   portIdx*/
					       case trap_e1LosAlarm:
					       case trap_e1LosAlarmClear:
					       case trap_e1LofAlarm:
					       case trap_e1LofAlarmClear:
					       case trap_e1AisAlarm:
					       case trap_e1AisAlarmClear:
					       case trap_e1RaiAlarm:
					       case trap_e1RaiAlarmClear:
					       case trap_e1SmfAlarm:
					       case trap_e1SmfAlarmClear:
					       case trap_e1LomfAlarm:
					       case trap_e1LomfAlarmClear:
					       case trap_e1Crc3Alarm:
					       case trap_e1Crc3AlarmClear:
					       case trap_e1Crc6Alarm:
					       case trap_e1Crc6AlarmClear:
						if( (pAlarmSrc->e1AlarmSrc.devIdx == trap_bac_list[i].alarmSrc.e1AlarmSrc.devIdx) &&
							(pAlarmSrc->e1AlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.e1AlarmSrc.brdIdx) &&
							(pAlarmSrc->e1AlarmSrc.portIdx == trap_bac_list[i].alarmSrc.e1AlarmSrc.portIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;
					      case trap_tdmServiceAbortAlarm:	
					      case trap_tdmServiceAbortAlarmClear:	
						case  trap_E1OutOfService:
						case  trap_E1OutOfServiceClear:
						if( (pAlarmSrc->e1AlarmSrc.devIdx == trap_bac_list[i].alarmSrc.e1AlarmSrc.devIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;
						/*end add by shixh@20070929*/

						/*added by xieshl 20080116*/
						case trap_ethLoopAlarm:
						case trap_ethLoopAlarmClear:
							if( (pAlarmSrc->portAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.devIdx) &&
								(pAlarmSrc->portAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.brdIdx) &&
								(pAlarmSrc->portAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.portIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;
						case trap_onuLoopAlarm:
						case trap_onuLoopAlarmClear:
							if( pAlarmSrc->devAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.devAlarmSrc.devIdx )
							{
								pEntry = &trap_bac_list[i];
							}
							/* end 20080116 */
						case trap_SwitchEthPortLoop:/*add by shixh20090520*/
						case trap_SwitchEthPortLoopClear:
						case trap_switchNewRegSuccess:
						case trap_switchReregSuccess:
						case trap_switchNotPresent:
							if( (pAlarmSrc->onuSwitchAlarmSrc.brdId == trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.brdId) &&
								(pAlarmSrc->onuSwitchAlarmSrc.ponId == trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.ponId) &&
								(pAlarmSrc->onuSwitchAlarmSrc.onuId== trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.onuId) &&
								(pAlarmSrc->onuSwitchAlarmSrc.onuBrdId== trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.onuBrdId)&&
								(pAlarmSrc->onuSwitchAlarmSrc.onuPortId== trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.onuPortId)&&
								(pAlarmSrc->onuSwitchAlarmSrc.reason== trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.reason)/*&&
								(pAlarmSrc->onuSwitchAlarmSrc.switchMacAddr== trap_bac_list[i].alarmSrc.onuSwitchAlarmSrc.switchMacAddr)*/)
							{
								pEntry = &trap_bac_list[i];
							}
							break;
							
						case trap_backboneEthLinkdown:  /*add by shixh@20080215*/
						case trap_backboneEthLinkup:
						case trap_ethPortBroadCastFloodControl:/*add by shixh20090612*/
      					case  trap_ethPortBroadCastFloodControlClear:
							if( (pAlarmSrc->portAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.devIdx) &&
								(pAlarmSrc->portAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.brdIdx) &&
								(pAlarmSrc->portAlarmSrc.portIdx == trap_bac_list[i].alarmSrc.portAlarmSrc.portIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;	
							
						/*added by shixh20080202*/
						case trap_tdmToEthLinkdown:	
						case trap_tdmToEthLinkup:
							if( (pAlarmSrc->tdmAlarmSrc.devIdx == trap_bac_list[i].alarmSrc.tdmAlarmSrc.devIdx) &&
								(pAlarmSrc->tdmAlarmSrc.brdIdx == trap_bac_list[i].alarmSrc.tdmAlarmSrc.brdIdx) &&
								(pAlarmSrc->tdmAlarmSrc.tdmIdx == trap_bac_list[i].alarmSrc.tdmAlarmSrc.tdmIdx) )
							{
								pEntry = &trap_bac_list[i];
							}
							break;

						default:
							break;
					}
					break;
				default:
					break;
			}
#endif
		}

		if( (pEntry != NULL) || (i == trap_bac_list_header) )
			break;
		
		if( i == 0 )
			i = TRAP_BAC_LIST_SIZE-1;
		else
			i--;
	}
	return pEntry;
}

int eventGetCurTime( sysDateAndTime_t *pDateTime )
{
	SLOCAL_TIME stTime;
	LONG rc = VOS_ERROR;

	if( pDateTime == NULL )
		return rc;

	if( (rc = devsm_get_slocal_time(&stTime)) == VOS_OK )
	{
		pDateTime->year = stTime.usYear;
		pDateTime->month = stTime.usMonth;
		pDateTime->day = stTime.usDay;
		pDateTime->hour = stTime.usHour;
		pDateTime->minute = stTime.usMinute;
		pDateTime->second = stTime.usSecond;
		pDateTime->reserver[0] = 0;
		pDateTime->reserver[1] = 0;
		pDateTime->reserver[2] = 0;
		pDateTime->reserver[3] = 0;
	}
	return rc;
}

/*----------------------------------------------------------------------------
* ����: �ڴ洢��������һ���µĴ洢�ռ䣬����洢�ռ��Ѿ����꣬�򸲸ǾɵĴ洢�ռ�
* �������: 
* �������: 
* ����ֵ: �洢�ռ��ַָ�� */
static trapBackupList_t* allocTrapBacItem()
{
	ULONG idx;

	if( trap_bac_list_idx >= TRAP_BAC_LIST_SIZE )
	{
		VOS_ASSERT	(0);
		return NULL;
	}

	idx = trap_bac_list[trap_bac_list_idx].bacIdx;
	if( idx != 0 )
	{
		if( trap_bac_list_idx >= TRAP_BAC_LIST_SIZE-1 )
			trap_bac_list_idx = 0;
		else
			trap_bac_list_idx++;

		if( trap_bac_list_header == trap_bac_list_idx )
		{
			if( trap_bac_list_header >= TRAP_BAC_LIST_SIZE-1 )
				trap_bac_list_header = 0;
			else
				trap_bac_list_header++;
		}
	}
	trap_bac_list[trap_bac_list_idx].bacIdx = idx+1;
	trap_bac_list[trap_bac_list_idx].alarmType = 0;
	trap_bac_list[trap_bac_list_idx].trapId = 0;
	VOS_MemZero( trap_bac_list[trap_bac_list_idx].alarmSrc.alarmSrcData, MAXLEN_EVENT_DATA );
	trap_bac_list[trap_bac_list_idx].alarmClearFlag = TRAP_FLAG_ALARM;
	eventGetCurTime( &(trap_bac_list[trap_bac_list_idx].alarmBeginTime) );
	eventGetCurTime( &(trap_bac_list[trap_bac_list_idx].alarmEndTime) );
	
	return &trap_bac_list[trap_bac_list_idx];
}

#if 0
static trapBackupList_t* freeAlarmSyn( trapBackupList_t* pAlarmSynTbl )
{
	if( pAlarmSynTbl == NULL )
		return pAlarmSynTbl;
	
	if( (pAlarmSynTbl->bacIdx != 0) &&
		(pAlarmSynTbl->bacIdx == trap_bac_list[trap_bac_list_idx].bacIdx) &&
		(trap_bac_list_header != trap_bac_list_idx) )
	{
		trap_bac_list[trap_bac_list_idx].bacIdx = 0;
		trap_bac_list[trap_bac_list_idx].alarmClearFlag = TRAP_FLAG_CLEAR;

		if( trap_bac_list_idx == 0 )
			trap_bac_list_idx = TRAP_BAC_LIST_SIZE-1;
		else
			trap_bac_list_idx--;

		/*if( trap_bac_list_header == trap_bac_list_idx )
		{
			if( trap_bac_list_header >= TRAP_BAC_LIST_SIZE-1 )
				trap_bac_list_header = 0;
			else
				trap_bac_list_header++;
		}*/
	}
	else
	{
		sys_console_printf("TRAP BAC: freeAlarmSyn error: bacIdx=%d\n", 
				pAlarmSynTbl->bacIdx );
		return NULL;
	}
		
	return &trap_bac_list[trap_bac_list_idx];
}
#endif

/*void eventOctetStringPrintf( uchar_t *pOctBuf, ULONG bufLen )
{
	ULONG i;
	for( i=0; i<bufLen; i++ )
	{
		if( i % 20 == 0 )
			sys_console_printf("\r\n");
		sys_console_printf( "%02x ", pOctBuf[i] );
	}
	sys_console_printf("\r\n");
}*/
/*----------------------------------------------------------------------------
* ����: ���ɸ澯ͬ������鲢�澯ͬ��
* �������: 
* �������: 
* ����ֵ: ��ȷ����OK�����󷵻�ERROR */
extern STATUS bindTrapVar( SNMPTRAP *trapvar, ULONG trapId, ULONG *pVarlist, const ULONG varNum );
extern LONG bindBridgeTrapVar( SNMPTRAP *trapvar, ULONG trapId, ULONG *pVarlist, const ULONG varNum );
extern LONG bindMib2TrapVar( SNMPTRAP *trapvar, ULONG trapId, ULONG *pVarlist, const ULONG varNum );

LONG saveTrapBacData( ULONG alarmType, ULONG trapId, 
			ULONG *pVarlist, ULONG varNum,
			alarmSrc_t *pAlarmSrc )
{
	LONG rc = VOS_ERROR;
	ULONG  synFlag;
	ULONG  partnerId;
	trapBackupList_t *pNewEntry;

	SNMPTRAP trapvar;
	uchar_t pSnmpPkt[512];
	size_t snmpPktLen = 512;

	if( (pVarlist == NULL) || (pAlarmSrc == NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if( trap_bac_enable == TRAP_BAC_DISABLE )
	{
		return rc;
	}
	
	synFlag = getEventSynFlag( alarmType, trapId );
	if( synFlag == ALM_SYN_NO )
	{
		/*return rc;*/
	}
	else if( synFlag == ALM_SYN_INS )
	{
		VOS_MemZero( pSnmpPkt, sizeof(pSnmpPkt) );
		if( alarmType == alarmType_private )
		{
			rc = bindTrapVar( &trapvar, trapId, pVarlist, varNum );
		}
		else if( alarmType == alarmType_bridge )
		{
			rc = bindBridgeTrapVar( &trapvar, trapId, pVarlist, varNum );
		}
		else if( alarmType == alarmType_mib2 )
		{
			rc = bindMib2TrapVar( &trapvar, trapId, pVarlist, varNum );
		}
		else if( alarmType == alarmType_bcmcmcctrl )
		{
			rc = bindCmcCtrlTrapVar( &trapvar, trapId, pVarlist, varNum );
		}
		
		if( rc == VOS_OK )
		{
			rc = code_snmp_trap_packet( &trapvar, pSnmpPkt, &snmpPktLen );
			if( rc == VOS_ERROR )
				sys_console_printf("\r\nTRAP BAC: Trap encoding error, trap_id=%d\r\n", trapId );
			if( snmpPktLen > TRAP_PDU_MAXLEN )
			{
				sys_console_printf("\r\nTRAP BAC: Trap packet is too long to save, trapPktLen=%d trap_id=%d\r\n", snmpPktLen, trapId );
				rc = VOS_ERROR;
			}
		}
		else
			sys_console_printf("\r\nTRAP BAC: Trap varlist error, trap_id=%d\r\n", trapId );

		if( rc == VOS_OK )
		{
			VOS_SemTake( eventSemId, WAIT_FOREVER );

			pNewEntry = allocTrapBacItem();

			if( pNewEntry )
			{
				pNewEntry->alarmType = (uchar_t)alarmType;
				pNewEntry->trapId = (uchar_t)trapId;
				VOS_MemCpy( &pNewEntry->alarmSrc, pAlarmSrc, sizeof(alarmSrc_t) );
				VOS_MemCpy( pNewEntry->trapPdu, pSnmpPkt, snmpPktLen );
				pNewEntry->trapPduLen = snmpPktLen;
				if( eventDebugSwitch & EVENT_DEBUGSWITCH_SYN )
				{
					sys_console_printf( "\r\nTRAP BAC: alarmType=%d, trapId=%d\r\n", alarmType, trapId );
					/*eventOctetStringPrintf( pSnmpPkt, snmpPktLen );*/
					pktDataPrintf( pSnmpPkt, snmpPktLen );

				}
			}
			VOS_SemGive( eventSemId );
		}
		else
		{
			/*sys_console_printf("TRAP BAC: Save trap packet error, trap_id=%d\r\n", trapId );*/
		}
	}
	else if( synFlag == ALM_SYN_DEL )
	{
		partnerId = getPartnerTrapId( alarmType, trapId );
		if( partnerId != 0 )
		{
			VOS_SemTake( eventSemId, WAIT_FOREVER );

			pNewEntry = seekTrapBacListByAlarmSrc( alarmType, partnerId, pAlarmSrc );
			if( pNewEntry != NULL )
			{
				rc = VOS_OK;
				pNewEntry->alarmClearFlag = TRAP_FLAG_CLEAR;
				/*pNewEntry->alarmEndTime = tickGet();*/
				eventGetCurTime( &pNewEntry->alarmEndTime );
				
				eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Alarm trap merger OK, trap=%d\r\n", trapId) );
			}
			else
				eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Alarm trap merger ERR, trap=%d\r\n", trapId) );

			VOS_SemGive( eventSemId );
		}
	}

	return rc;
}

/*----------------------------------------------------------------------------
* ����: �澯����ͬ��
* �������: 
* �������: 
* ����ֵ: ��ȷ����OK�����󷵻�ERROR */
/*LONG mergerAlarmSyn( ULONG alarmType, ULONG trapId, alarmSrc_t *pAlarmSrc )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pNewEntry;

	if( trap_bac_enable == TRAP_BAC_DISABLE )
		return rc;

	pNewEntry = seekTrapBacListByAlarmSrc( alarmType, trapId, pAlarmSrc );
	if( pNewEntry != NULL )
	{
		rc = VOS_OK;
		pNewEntry->alarmClearFlag = TRAP_FLAG_CLEAR;
		eventGetCurTime( &pNewEntry->alarmEndTime );
		
		eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Alarm trap merger OK, trap=%d\r\n", trapId) );
	}
	else
		eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Alarm trap merger ERR, trap=%d\r\n", trapId) );

	return rc;
}*/

/*----------------------------------------------------------------------------
* ����: ����澯ͬ��
* �������: 
* �������: 
* ����ֵ: ��ȷ����OK�����󷵻�ERROR */
LONG eraseTrapBacData()
{
	LONG rc = VOS_OK;

	memset( trap_bac_list, 0, TRAP_BAC_LIST_SIZE * sizeof(trapBackupList_t) );
	trap_bac_list_header = 0;
	trap_bac_list_idx = trap_bac_list_header;

	/*eventDbgPrintf( EVENT_DEBUGSWITCH_SYN, ("TRAP BAC: Alarm trap packet erase OK\r\n") );*/
	return rc;
}

/*----------------------------------------------------------------------------
* ����: ͨ������ɾ��һ��ONUʱ����澯ͬ����ϢҲɾ��
* �������: devIdx��ONU�豸����
* ����ֵ: ������ڷ���OK�������ڷ���ERROR */
LONG eraseTrapBacDataByDevIdx( ULONG devIdx )
{
	/*add by shixh@20070929*/
	LONG rc = VOS_OK;
	ULONG synIdx;
	LONG i;
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	for( i=0; i<TRAP_BAC_LIST_SIZE; i++ )
	{
		if( trap_bac_list[i].alarmSrc.portAlarmSrc.devIdx == devIdx )
	   	{
	   		synIdx=trap_bac_list[i].bacIdx;
			memset( &trap_bac_list[i], 0,  sizeof(trapBackupList_t) );
	   		trap_bac_list[i].bacIdx = synIdx;
		}
	}
	VOS_SemGive( eventSemId );
	return rc;
}

/* ���澯ͬ��ʹ�� */
LONG getTrapBacEnable( ULONG *pEnable )
{
	if( pEnable == NULL )
		return VOS_ERROR;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	*pEnable = trap_bac_enable;
	VOS_SemGive( eventSemId );

	return VOS_OK;
}

extern int eventSync_configEbl_2AllSlave( ULONG subType, ULONG subCode );
/* ���ø澯ͬ��ʹ�� */
LONG setTrapBacEnable( ULONG enable )
{
	if( (enable != TRAP_BAC_ENABLE) && (enable != TRAP_BAC_DISABLE) )
		return VOS_ERROR;

	if( trap_bac_enable != enable )
	{
		VOS_SemTake( eventSemId, WAIT_FOREVER );
		trap_bac_enable = enable;

		if( enable == TRAP_BAC_ENABLE )
		{
			eraseTrapBacData();
   		}	
		VOS_SemGive( eventSemId );

		eventSync_configEbl_2AllSlave( 2, enable );
   	}

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* ���澯ͬ������ */
LONG getTrapBacIndex( ULONG idx, ULONG *pIdx )
{
	trapBackupList_t *pEntry;
	if( pIdx == NULL )
		return VOS_ERROR;
	pEntry = seekTrapBacListByIndex( idx );
	if( pEntry == NULL )
		return VOS_ERROR;
	*pIdx = idx;

	return VOS_OK;
}
/*----------------------------------------------------------------------------*/

/* ���澯ͬ��ʱ�� */
LONG getTrapBacBeginTime( ULONG idx, sysDateAndTime_t *pAlarmTime )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pEntry;
	if( pAlarmTime == NULL )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekTrapBacListByIndex( idx );
	if( pEntry )
	{
		VOS_MemCpy( pAlarmTime, &pEntry->alarmBeginTime, sizeof(sysDateAndTime_t) );
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*----------------------------------------------------------------------------*/

/* ���澯ͬ��������Ϣ */
LONG getTrapBacPduData( ULONG idx, uchar_t *pSynData, ULONG *pDataLen )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pEntry;
	if( (pSynData == NULL) || (pDataLen == NULL) )
		return rc;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekTrapBacListByIndex( idx );
	if( pEntry )
	{
		if( pEntry->trapPduLen >= TRAP_PDU_MAXLEN )
			pEntry->trapPduLen = TRAP_PDU_MAXLEN-1;
		VOS_MemCpy( pSynData, pEntry->trapPdu, pEntry->trapPduLen );
		*pDataLen = pEntry->trapPduLen;
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*----------------------------------------------------------------------------*/

/* ���澯ͬ����־ */
LONG getTrapBacClearFlag( ULONG idx, ULONG *pSynFlag )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pEntry;
	if( pSynFlag == NULL )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekTrapBacListByIndex( idx );
	if( pEntry )
	{
		*pSynFlag = pEntry->alarmClearFlag;
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*----------------------------------------------------------------------------*/

/* ���澯ͬ��ʱ�� */
LONG getTrapBacClearTime( ULONG idx, sysDateAndTime_t *pClearTime )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pEntry;
	if( pClearTime == NULL )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	pEntry = seekTrapBacListByIndex( idx );
	if( pEntry )
	{
		VOS_MemCpy( pClearTime, &pEntry->alarmEndTime, sizeof(sysDateAndTime_t) );
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*----------------------------------------------------------------------------*/

/*modefy  by shixh@20070929*/
/*----------------------------------------------------------------------------
* ����: �澯ͬ��������һ������
* �������: 
* �������: pLogIdx����һ�е�ͬ������
* ����ֵ: ������ڷ���OK�������ڷ���ERROR */
LONG getTrapBacFirstIndex ( ULONG *pIdx )
{
	LONG rc = VOS_ERROR;
	/*LONG  i;*/
	if( pIdx == NULL )
		return rc;

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	/*for( i=0; i<TRAP_BAC_LIST_SIZE; i++ )*/
	{
		if( (trap_bac_list[trap_bac_list_header].bacIdx != 0) && (trap_bac_list[trap_bac_list_header].trapId != 0) )
		{
			*pIdx = trap_bac_list[trap_bac_list_header].bacIdx;
			rc = VOS_OK;
		}
		/*else 
			break;*/
	}
	VOS_SemGive( eventSemId );

	return rc;
}

/*----------------------------------------------------------------------------
* ����: �澯ͬ���������һ������
* �������: logIdx����ǰ��־����
* �������: pNextLogIdx����һ�е�ͬ������
* ����ֵ: ������ڷ���OK�������ڷ���ERROR */
LONG getTrapBacNextIndex (ULONG idx, ULONG *pNextIdx )
{
	LONG rc = VOS_ERROR;
	trapBackupList_t *pEntry;
	if( pNextIdx == NULL )
		return rc;

	if( trap_bac_list_idx >= TRAP_BAC_LIST_SIZE )
	{
		VOS_ASSERT	(0);
		return rc;
	}
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	while( idx < trap_bac_list[trap_bac_list_idx].bacIdx )
	{
		idx++;
		if( (pEntry = seekTrapBacListByIndex(idx)) == NULL )
			break;
		if( pEntry->trapId != 0 )
		{
			*pNextIdx = idx;
			rc = VOS_OK;
			break;
		}
	}
	VOS_SemGive( eventSemId );

	return rc;
}
/*end modefy  by shixh@20070929*/

/*----------------------------------------------------------------------------
* ����: ��������Ƿ���ȷ
* �������: logIdx����ǰͬ������
* �������: 
* ����ֵ: ��ȷ����OK�����󷵻�ERROR */
LONG checkTrapBacIndex ( ULONG idx )
{
	LONG rc = VOS_ERROR;
	if( (trap_bac_list_idx >= TRAP_BAC_LIST_SIZE) || (trap_bac_list_header >= TRAP_BAC_LIST_SIZE) )
	{
		VOS_ASSERT	(0);
		return rc;
	}

	VOS_SemTake( eventSemId, WAIT_FOREVER );
	if( idx != 0 && 
		idx >= trap_bac_list[trap_bac_list_header].bacIdx &&
		idx <= trap_bac_list[trap_bac_list_idx].bacIdx )
	{
		rc = VOS_OK;
	}
	VOS_SemGive( eventSemId );

	return rc;
}

#if 0
LONG testEthPortLinkStatus( ULONG trapId, ULONG devIdx, ULONG brdIdx, ULONG portIdx )
{
	LONG rc;
	ULONG varList[4] = { 0, 0, 0, 0 };
	alarmSrc_t alarmSrc;

	/*LONG i = 0;
	SNMPTRAP  trapvar;
	ULONG trapOid[] = {1,3,6,1,4,1,10072,2,20,1,1,6};
	ULONG oidLen = sizeof(trapOid)/sizeof(trapOid[0]); 
	ULONG ethDevIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,1};
	ULONG ethBrdIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,2};
	ULONG ethPortIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,3};

	VOS_MemCpy( trapvar.TrapOid, trapOid, sizeof(trapOid));
	trapvar.TrapOid[oidLen] = trapId;
	trapvar.TrapOidLength = oidLen+1;
	
	VOS_MemCpy( trapvar.VarOid[i], ethDevIdxOid, sizeof( ethDevIdxOid ) );
	trapvar.VarOidLength[i] = sizeof(ethDevIdxOid)/sizeof(oid);
	*(ULONG*)trapvar.Var[i] = devIdx;
	trapvar.VarLength[i] = sizeof(ULONG);
	trapvar.VarType[i] = ASN_INTEGER;

	i++;
	VOS_MemCpy( trapvar.VarOid[i], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
	trapvar.VarOidLength[i] = sizeof(ethBrdIdxOid)/sizeof(oid);
	*(ULONG*)trapvar.Var[i] = brdIdx;
	trapvar.VarLength[i] = sizeof(ULONG);
	trapvar.VarType[i] = ASN_INTEGER;

	i++;
	VOS_MemCpy( trapvar.VarOid[i], ethPortIdxOid, sizeof( ethPortIdxOid ) );
	trapvar.VarOidLength[i] = sizeof(ethPortIdxOid)/sizeof(oid);
	*(ULONG*)trapvar.Var[i] = portIdx;
	trapvar.VarLength[i] = sizeof(ULONG);
	trapvar.VarType[i] = ASN_INTEGER;	

	mn_send_msg_trap( &trapvar, MSG_PRI_NORMAL );*/

	varList[0] = devIdx;
	varList[1] = brdIdx;
	varList[2] = portIdx;

	alarmSrc.portAlarmSrc.devIdx = devIdx;
	alarmSrc.portAlarmSrc.brdIdx = brdIdx;
	alarmSrc.portAlarmSrc.portIdx = portIdx;

	rc = saveTrapBacData(alarmType_private, trapId, varList, 3, &alarmSrc );

	return rc;
}

static ULONG loopCounter = 0;
LONG testEthPortLinkStatusLoop( ULONG num )
{
	ULONG i, trapId, devIdx, brdIdx, portIdx;

	sys_console_printf("alarm syn. test begin time = %s\n", (char*)getClkTimeStrings() );

	loopCounter++;
	
	for( i=0; i<num; i++ )
	{
		if( loopCounter == 0 )
			break;
		
		devIdx = 1;
		for( trapId=trap_ethLinkdown; trapId<=trap_ethLinkup; trapId++ )
		for( portIdx=1; portIdx<=24; portIdx++ )
		{
			testEthPortLinkStatus( trapId, devIdx, brdIdx, portIdx );
		}
	}
		
	sys_console_printf("alarm syn. end time = %s\n", (char*)getClkTimeStrings() );

	loopCounter--;
	
	return VOS_OK;
}

void alarmSynTest( LONG num )
{
	if( (num == 0) && (loopCounter != 0) )
	{
		loopCounter = 0;
		return;
	}
	if( loopCounter > 0 )
	{
		sys_console_printf("alarm syn. is testing now.\n" );
	}
	else
	{
		VOS_TaskCreate_Ex_X( "tASynTest", 
					(FUNCPTR)testEthPortLinkStatusLoop,
					254, 
					8000, 
					num,0,0,0,0,0,0,0,0,0);
	}
}
#endif

