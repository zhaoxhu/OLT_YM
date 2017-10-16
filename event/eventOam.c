#include "gwEponSys.h"
#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "GwttOam/OAM_gw.h"
#include "ethLoopChk.h"
#include "eventOam.h"

#include "lib_gwEponOnuMib.h"
#include  "onu/ExtBoardType.h"
#include "onu/Onuatuocfg.h"
#include "onu/onuOamUpd.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "E1_MIB.h"
#endif
/*#include "Cdp_pub.h"*/

#define MAX_SUPPORTED_PONID			MAXPON
#define MAX_SUPPORTED_LLID_PERPON	64

/*#ifndef MAX_ONU_ETHPORT
#define MAX_ONU_ETHPORT 24
#endif*/

/* 定义ONU上报事件标志，主要用于防止重复上报，通过记录每个ONU的所有告警信
   息，以识别是否为第一次收到某告警OAM， 如果是则上报告警事件到告警任务中
   处理，并发送应答OAM，否则只发送应答OAM，不再处理 */

/* ONU温度告警事件标志 */
#define EVENT_HIGH_TEMPERATURE_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hTemperature == 1)
#define EVENT_HIGH_TEMPERATURE_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hTemperature = 1; }
#define EVENT_HIGH_TEMPERATURE_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hTemperature = 0; }

#define EVENT_LOW_TEMPERATURE_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lTemperature == 1)
#define EVENT_LOW_TEMPERATURE_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lTemperature = 1; }
#define EVENT_LOW_TEMPERATURE_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lTemperature = 0; }

/* ONU以太网端口link状态改变事件标志 */
/*#define EVENT_ETH_LINKUP_IS_REPEATED(PONID,LLID,PORT) \
			(((onuAlarmFlagRecord[PONID][LLID].linkUp & (1 << PORT)) != 0) && \
			((onuAlarmFlagRecord[PONID][LLID].linkDown & (1 << PORT)) == 0))
#define EVENT_ETH_LINKUP_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].linkUp |= (1 << PORT); \
			 EVENT_ETH_LINKDOWN_FLAG_CLEAR(PONID,LLID,PORT); }
#define EVENT_ETH_LINKUP_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].linkUp &= (~(1 << PORT));}

#define EVENT_ETH_LINKDOWN_IS_REPEATED(PONID,LLID,PORT) \
			(((onuAlarmFlagRecord[PONID][LLID].linkUp & (1 << PORT)) == 0) && \
			(((onuAlarmFlagRecord[PONID][LLID].linkDown & (1 << PORT)) != 0)))
#define EVENT_ETH_LINKDOWN_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].linkDown |= (1 << PORT); \
			 EVENT_ETH_LINKUP_FLAG_CLEAR(PONID,LLID,PORT); }
#define EVENT_ETH_LINKDOWN_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].linkDown &= (~(1 << PORT));}*/		/* removed by xieshl 20080506 */

/* ONU以太网性能告警事件标志 */
#define EVENT_ETH_INFLR_IS_REPEATED(PONID,LLID,PORT) \
			((onuAlarmFlagRecord[PONID][LLID].inFlr & (1 << PORT)) != 0)
#define EVENT_ETH_INFLR_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].inFlr |= (1 << PORT);}
#define EVENT_ETH_INFLR_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].inFlr &= (~(1 << PORT));}

#define EVENT_ETH_OUTFLR_IS_REPEATED(PONID,LLID,PORT) \
			((onuAlarmFlagRecord[PONID][LLID].outFlr & (1 << PORT)) != 0)
#define EVENT_ETH_OUTFLR_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].outFlr |= (1 << PORT);}
#define EVENT_ETH_OUTFLR_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].outFlr &= (~(1 << PORT));}

#define EVENT_ETH_INFER_IS_REPEATED(PONID,LLID,PORT) \
			((onuAlarmFlagRecord[PONID][LLID].inFer & (1 << PORT)) != 0)
#define EVENT_ETH_INFER_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].inFer |= (1 << PORT);}
#define EVENT_ETH_INFER_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].inFer &= (~(1 << PORT));}

/* ONU以太网业务中断告警事件标志 */
#define EVENT_ETH_TRAFFIC_IS_REPEATED(PONID,LLID,PORT) \
			((onuAlarmFlagRecord[PONID][LLID].traffic & (1 << PORT)) != 0)
#define EVENT_ETH_TRAFFIC_FLAG_SET(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].traffic |= (1 << PORT);}
#define EVENT_ETH_TRAFFIC_FLAG_CLEAR(PONID,LLID,PORT) \
			{onuAlarmFlagRecord[PONID][LLID].traffic &= (~(1 << PORT));}

/* ONU以太网STP事件标志 */
#define EVENT_STP_TOPCHG_IS_REPEATED(PONID,LLID) \
			((onuAlarmFlagRecord[PONID][LLID].stpTopChg != 0) && \
			 (onuAlarmFlagRecord[PONID][LLID].stpNewRoot == 0) )
#define EVENT_STP_TOPCHG_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].stpTopChg = 1; \
			 EVENT_STP_NEWROOT_FLAG_CLEAR(PONID,LLID); }
#define EVENT_STP_TOPCHG_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].stpTopChg = 0;}

#define EVENT_STP_NEWROOT_IS_REPEATED(PONID,LLID) \
			((onuAlarmFlagRecord[PONID][LLID].stpNewRoot != 0) && \
			 (onuAlarmFlagRecord[PONID][LLID].stpTopChg == 0) )
#define EVENT_STP_NEWROOT_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].stpNewRoot = 1; \
			 EVENT_STP_TOPCHG_FLAG_CLEAR(PONID,LLID); }
#define EVENT_STP_NEWROOT_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].stpNewRoot = 0;}

/* ONU设备信息修改事件标志 */
/*#define EVENT_DEV_INFO_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].devInfo != 0)
#define EVENT_DEV_INFO_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].devInfo = 1}
#define EVENT_DEV_INFO_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].devInfo = 0}*/

/* added by xieshl 20061108, ONU远程升级事件标志 */
#define EVENT_LOAD_SOFTWARE_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].loadSoft == 1)
#define EVENT_LOAD_SOFTWARE_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadSoft = 1; }
#define EVENT_LOAD_SOFTWARE_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadSoft = 0; }

#define EVENT_LOAD_HARDWARE_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].loadHard == 1)
#define EVENT_LOAD_HARDWARE_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadHard = 1; }
#define EVENT_LOAD_HARDWARE_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadHard = 0; }

#define EVENT_LOAD_BOOT_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].loadBoot == 1)
#define EVENT_LOAD_BOOT_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadBoot = 1; }
#define EVENT_LOAD_BOOT_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadBoot = 0; }

#define EVENT_LOAD_CFGDATA_IS_REPEATED(PONID,LLID) \
			(onuAlarmFlagRecord[PONID][LLID].loadCfgData == 1)
#define EVENT_LOAD_CFGDATA_FLAG_SET(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadCfgData = 1; }
#define EVENT_LOAD_CFGDATA_FLAG_CLEAR(PONID,LLID) \
			{onuAlarmFlagRecord[PONID][LLID].loadCfgData = 0; }
/* end 20061108 */

/* B--added by liwei056@2009-10-14 for ONU's Optical Report */
#if 0
/*begin: added by wangxiaoyu 2008-7-24 13:24:35*/
#define	EVENT_ONU_OPTICAL_PARA_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].opticalParaAlm == 1)
#define	EVENT_ONU_OPTICAL_PARA_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].opticalParaAlm = 1;}
#define	EVENT_ONU_OPTICAL_PARA_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].opticalParaAlm = 0;}		
#else
#define	EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hOpticalPowerUp == 1)
#define	EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalPowerUp = 1;}
#define	EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalPowerUp = 0;}		

#define	EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lOpticalPowerUp == 1)
#define	EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalPowerUp = 1;}
#define	EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalPowerUp = 0;}		

#define	EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hOpticalPowerDown == 1)
#define	EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalPowerDown = 1;}
#define	EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalPowerDown = 0;}		

#define	EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lOpticalPowerDown == 1)
#define	EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalPowerDown = 1;}
#define	EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalPowerDown = 0;}		


#define	EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hWorkVoltage == 1)
#define	EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hWorkVoltage = 1;}
#define	EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hWorkVoltage = 0;}		

#define	EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lWorkVoltage == 1)
#define	EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lWorkVoltage = 1;}
#define	EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lWorkVoltage = 0;}		


#define	EVENT_ONU_OPTICAL_BIAS_HIGH_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hBiasCurrent == 1)
#define	EVENT_ONU_OPTICAL_BIAS_HIGH_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hBiasCurrent = 1;}
#define	EVENT_ONU_OPTICAL_BIAS_HIGH_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hBiasCurrent = 0;}		

#define	EVENT_ONU_OPTICAL_BIAS_LOW_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lBiasCurrent == 1)
#define	EVENT_ONU_OPTICAL_BIAS_LOW_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lBiasCurrent = 1;}
#define	EVENT_ONU_OPTICAL_BIAS_LOW_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lBiasCurrent = 0;}		


#define	EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].hOpticalTemperature == 1)
#define	EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalTemperature = 1;}
#define	EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].hOpticalTemperature = 0;}		

#define	EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].lOpticalTemperature == 1)
#define	EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalTemperature = 1;}
#define	EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].lOpticalTemperature = 0;}		
#endif
/* E--added by liwei056@2009-10-14 for ONU's Optical Report */

#define	EVENT_ONU_LASER_ALWAYS_ON_ALM_IS_REPEATED(PONID, LLID) \
			(onuAlarmFlagRecord[PONID][LLID].onuLaserAlwaysOn == 1)
#define	EVENT_ONU_LASER_ALWAYS_ON_ALM(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].onuLaserAlwaysOn = 1;}
#define	EVENT_ONU_LASER_ALWAYS_ON_ALM_CLEAR(PONID, LLID) \
			{onuAlarmFlagRecord[PONID][LLID].onuLaserAlwaysOn = 0;}		


/*extern void eventOctetStringPrintf( uchar_t *pOctBuf, ulong_t bufLen );*/

extern ULONG LoopChkOnuIncludeExpandInfo;
#if 1
static onuAlarmFlagRecord_t (*onuAlarmFlagRecord)[MAX_SUPPORTED_LLID_PERPON];
#else
static onuAlarmFlagRecord_t onuAlarmFlagRecord[MAX_SUPPORTED_PONID][MAX_SUPPORTED_LLID_PERPON] /*= {{{0}}}*/;/*20100201*/
#endif

extern ULONG get_gfa_tdm_slotno();
extern int eventSync_onuData_2Master( ULONG devIdx, ULONG brdIdx, ULONG alarmFlag, UCHAR* pBrdPduData, LONG dataLen );
extern LONG onuExtMgmt_BrdPullCallback( ULONG devIdx, ULONG brdIdx);
extern int onuLaserAlwaysOnAlarmHandler( const ULONG onuDevIdx );
extern int onuLaserAlwaysOnAlarmClearHandler( const ULONG onuDevIdx );
/*extern int eventOamMsg_onuEthPortLoop_CDP( ushort_t ponId, ulong_t llId, eventOnuEthLoopMsg_t *pOamMsg );*/
extern int eventOamMsg_uplinkTraffic( ushort_t ponId, ulong_t llId, commEventOamMsg_t *pOamMsg );
int eventOamMsg_recvCallback( ushort_t ponId, ushort_t llId, ushort_t llid, 
                                ushort_t  length, uchar_t *pFrame, uchar_t *pSessionField );

/*----------------------------------------------------------------------------*/
int initAlarmOam()
{
	int rc = VOS_OK;
    ULONG ulSize;

#if 1
    ulSize = sizeof(onuAlarmFlagRecord_t) * MAX_SUPPORTED_LLID_PERPON * MAXPON;
    if ( NULL == (onuAlarmFlagRecord = VOS_Malloc(ulSize, MODULE_EVENT)) )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
	VOS_MemZero( onuAlarmFlagRecord, ulSize );
#else    
	/*int i,j;

	for( i=0; i<MAX_SUPPORTED_PONID; i++ )
	for( j=0; j<MAX_SUPPORTED_LLID_PERPON; j++ )
	{
		onuAlarmFlagRecord[i][j].hTemperature = 0;
		onuAlarmFlagRecord[i][j].lTemperature = 0;
		onuAlarmFlagRecord[i][j].stpTopChg = 0;
		onuAlarmFlagRecord[i][j].stpNewRoot = 0;
		onuAlarmFlagRecord[i][j].server = 0;
		onuAlarmFlagRecord[i][j].link = 0;	
		onuAlarmFlagRecord[i][j].inFlr = 0;	
		onuAlarmFlagRecord[i][j].outFlr = 0;	
		onuAlarmFlagRecord[i][j].inFer = 0;	
		onuAlarmFlagRecord[i][j].traffic = 0;		
	}*/
	VOS_MemZero( &onuAlarmFlagRecord[0][0], sizeof(onuAlarmFlagRecord) );
#endif

	if( PRODUCT_E_GFA6900 == SYS_PRODUCT_TYPE || SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000 )
	{
		if( SlotCardIsPonBoard(SYS_LOCAL_MODULE_SLOTNO) == VOS_OK )
			rc = CommOltMsgRvcCallbackInit(GW_CALLBACK_ALARMORLOG, (void *)eventOamMsg_recvCallback);
	}
	else
		rc = CommOltMsgRvcCallbackInit(GW_CALLBACK_ALARMORLOG, (void *)eventOamMsg_recvCallback);
	
	return rc;
}

/*----------------------------------------------------------------------------*/

int onuIdToOnuIndex( ushort_t ponId, ushort_t llId )
{
	int onuDevIdx = 0;
	int onu_idx, pon_idx, slot_idx;

	if( PON_NOT_USED_LLID != llId )
	{
		onu_idx = llId/*GetOnuIdxByLlid( ponId, llId )*/;
		pon_idx = GetPonPortByPonChip( ponId );
		slot_idx = GetCardIdxByPonChip( ponId );

		if( (onu_idx >= 0) && (pon_idx > 0) && (slot_idx > 0) )
			/*onuDevIdx = slot_idx * 10000 + pon_idx * 1000 + onu_idx;*/
                    onuDevIdx = MAKEDEVID(slot_idx,pon_idx,onu_idx);
	}
	return onuDevIdx;
}

int eventOamMsg_temperature( ushort_t ponId, ulong_t llId, eventOnuTemperatureMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR : temperature of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}
	
	switch( pOamMsg->flag )
	{
		case temperatureHighAlarm:	/* 温度过高告警*/
			if( EVENT_HIGH_TEMPERATURE_IS_REPEATED(ponId, llId) )
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d repeat alarm\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			else
			{
				deviceTemperatureHigh_EventReport( (ulong_t)onuDevIdx, pOamMsg->temperature, pOamMsg->threshold );
				EVENT_HIGH_TEMPERATURE_FLAG_SET( ponId, llId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d hAlarm\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			break;
			
		case temperatureHighClear:	/* 温度过高告警恢复 */
			if( EVENT_HIGH_TEMPERATURE_IS_REPEATED(ponId, llId) )
			{
				deviceTemperatureHighClear_EventReport( (ulong_t)onuDevIdx, pOamMsg->temperature, pOamMsg->threshold );
				EVENT_HIGH_TEMPERATURE_FLAG_CLEAR( ponId, llId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d hClear\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			else
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d repeat clear\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			break;

		case temperatureLowAlarm:		/* 温度过低告警 */
			if( EVENT_LOW_TEMPERATURE_IS_REPEATED(ponId, llId) )
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d repeat lAlarm\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			else
			{
				deviceTemperatureLow_EventReport( (ulong_t)onuDevIdx, pOamMsg->temperature, pOamMsg->threshold );
				EVENT_LOW_TEMPERATURE_FLAG_SET( ponId, llId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d lAlarm\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			break;

		case temperatureLowClear:		/* 温度过低告警恢复 */
			if( EVENT_LOW_TEMPERATURE_IS_REPEATED(ponId, llId) )
			{
				deviceTemperatureLowClear_EventReport( (ulong_t)onuDevIdx, pOamMsg->temperature, pOamMsg->threshold );
				EVENT_LOW_TEMPERATURE_FLAG_CLEAR( ponId, llId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d lClear\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			else
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d temperature=%d repeat lClear\r\n", onuDevIdx, pOamMsg->temperature) );
			}
			break;

		default:
			rc = VOS_ERROR;
			break;
	}
	
	return rc;
}

int eventOamMsg_onuEthPortStatus( ushort_t ponId, ulong_t llId, eventOnuEthStatusMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	int slot = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH link status of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}
	if( pOamMsg->portId.port > MAX_ONU_ETHPORT )
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: invalid eth port! DevIdx=%d port=%d\r\n", onuDevIdx, pOamMsg->portId.port) );
		return VOS_ERROR;
	}
	
	switch( pOamMsg->linkStatus )
	{
		case ethPortStatus_linkup:	
			/*if( EVENT_ETH_LINKUP_IS_REPEATED(ponId, llId, pOamMsg->portId) )
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: repeat ethPortStatus_linkup onuDevIdx=%d, port=%d\r\n", onuDevIdx, pOamMsg->portId) );
			}
			else
			{
				ethLinkup_EventReport( (ulong_t)onuDevIdx, 1, pOamMsg->portId );
				EVENT_ETH_LINKUP_FLAG_SET( ponId, llId, pOamMsg->portId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ethPortStatus_linkup onuDevIdx=%d, port=%d\r\n", onuDevIdx, pOamMsg->portId) );
			}*/		/* removed by xieshl 20080506, 不再过滤，直接上报 */
			
			ethLinkup_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port);
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d linkup\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
			break;

		case ethPortStatus_linkdown:	
			/*if( EVENT_ETH_LINKDOWN_IS_REPEATED(ponId, llId, pOamMsg->portId) )
			{
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: repeat ethPortStatus_linkdown onuDevIdx=%d, port=%d\r\n", onuDevIdx, pOamMsg->portId) );
			}
			else
			{
				ethLinkdown_EventReport( (ulong_t)onuDevIdx, 1, pOamMsg->portId );
				EVENT_ETH_LINKDOWN_FLAG_SET( ponId, llId, pOamMsg->portId );
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ethPortStatus_linkdown onuDevIdx=%d, port=%d\r\n", onuDevIdx, pOamMsg->portId) );
			}*/		/* removed by xieshl 20080506 */
			ethLinkdown_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d linkdown\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
			break;

		case ethPortStatus_testing: 	
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d testing\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
			break;

		case ethPortStatus_unknown: 	
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d unknown\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
			break;

		default:
			rc = VOS_ERROR;
			break;
	}
	
	return rc;
}
/* added by xieshl 20080506 */

extern ULONG loopChkDebugSwitch ;
extern UCHAR chk_config_frame_smac[];

int eventOamMsg_onuEthPortLoop( ushort_t ponId, ulong_t llId, eventOnuEthLoopMsg_t *pOamMsg )
{
	int rc = VOS_OK, vid = 0;
	UCHAR OtherOltFlag = 0;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	USHORT test = 0 ,oltboardid=0,oltportid=0,onuid=0;
	int slot = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
	int OltType = 0;
	int oltType[2] = {0};
	int slotno = 0, portno = 0;

	portno = GetPonPortByPonChip(ponId);
	slotno = GetCardIdxByPonChip(ponId);
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH loop detecting status of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->portId.port > MAX_ONU_ETHPORT)
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: eventOamMsg_onuEthPortLoop invalid port! onuDevIdx=%d port=%d\r\n", onuDevIdx, pOamMsg->portId.port) );
		return VOS_ERROR;
	}
/*如果传上来的OLT类型是0，那么不要with信息，新旧版本兼容*/
	if( pOamMsg->oltType  == 0)
	{
		pOamMsg->loopInf = 0;
	}	
	switch( pOamMsg->loopStatus )
	{
		case ethPortStatus_loopAlarm:
		
			if(loopChkDebugSwitch == 3)
			{
				test = (pOamMsg->onuLocal>>24)&0x000000ff;
				oltboardid = (pOamMsg->onuLocal>>16)&0x000000ff;
				oltportid=(pOamMsg->onuLocal>>8)&0x000000ff;
				onuid = (pOamMsg->onuLocal)&0x000000ff;
				sys_console_printf("Receive EthLoop Oam (Report Alarm). Device:%d ,Slot: %d ,Port:%d ,Alarm type is %d, Vlan is %d\r\n",
					onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port,pOamMsg->type, pOamMsg->vid);
				if(pOamMsg->portId.switchID != 0)
					sys_console_printf("switchID is %d, switchPort is %d\r\n",pOamMsg->portId.switchID,pOamMsg->portId.switchPort);
				if(pOamMsg->loopInf != 0)
				{
					sys_console_printf("oltType is %d\r\n",pOamMsg->oltType);
					sys_console_printf("onuLocal is %d\r\n",pOamMsg->onuLocal);
					sys_console_printf("It is %d, %d, %d, %d\r\n",test,oltboardid,oltportid,onuid);
					sys_console_printf("onuType is %d\r\n",pOamMsg->onuType);
					sys_console_printf("onuPortList is %d\r\n",pOamMsg->onuPortList);
				}
				sys_console_printf("\r\n");
			}

			if(loopChkDebugSwitch == 3)
			{
				if(oltboardid == 2)
				{
					sys_console_printf("onuLocal is %d\r\n",pOamMsg->onuLocal);
					sys_console_printf("It is %d, %d, %d, %d\r\n",test,oltboardid,oltportid,onuid);
				}
			}
		
			if(pOamMsg->portId.switchID==0)
			{
				 if(pOamMsg->loopInf==0)
				 {
    					/*addEthPortToLoopList( onuDevIdx, slot, pOamMsg->portId.port, pOamMsg->vid );*/
					addEthPortToLoopList2( onuDevIdx, slot, pOamMsg->portId.port, pOamMsg->vid,0,0,0,0,0,0,NULL);
    					eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d loop\r\n", 
    					onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
					LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d loop in vlan %d\r\n",
					GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), pOamMsg->portId.slot, pOamMsg->portId.port,pOamMsg->vid));
					check_mac_entry_delete(slotno,portno,onuDevIdx);
					
	                    	}
	                 	else if(pOamMsg->loopInf == 1) /*如果支持环路检测扩展消息*/
	                    {             
				    	ULONG brdIdx=0, portIdx=0;
					ULONG loopoltSlot=0, loopoltPort= 0, loopLlid= 0, loopdevIdx = 0, PonPortIdx = 0, looponuid = 0;
					ULONG loopportlist1 = 0,loopportlist2 = 0, ErrorFlag = 0;
			
					loopoltSlot=(pOamMsg->onuLocal>>16);
					loopoltPort=((pOamMsg->onuLocal>>8)&0x000000ff);
					loopLlid = ((pOamMsg->onuLocal)&0x000000ff);
										
					vid = pOamMsg->vid;

					if(pOamMsg->onuPortList != 0)
					{
						loopportlist1 =  (pOamMsg->onuPortList>>8)&0x000000ff;
						loopportlist2 = (pOamMsg->onuPortList)&0x000000ff;
					}

					brdIdx = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
				    	portIdx=pOamMsg->portId.port;
#if 0
					if(VOS_MemCmp(pOamMsg->oltMac, chk_config_frame_smac, ETH_LOOP_MAC_ADDR_LEN) != 0)
#else
					OltType=GetOltType();
					if(OltType==V2R1_OLT_GFA6100)
					{
						oltType[0] = 1;		/*对应发包中的规则1-6100，2-6700, 3-6900 */
						oltType[1] = 0x61;
					}
					else if(OltType == V2R1_OLT_GFA6900)
					{
						oltType[0] = 3;
						oltType[1] = 0x69;
					}
					else if(OltType == V2R1_OLT_GFA8000)  /*8000暂定为4*/
					{
						oltType[0] = 4;
						oltType[1] = 0x80;
					}
					else if(OltType == V2R1_OLT_GFA8100)  /*8100暂定为5*/
					{
						oltType[0] = 5;
						oltType[1] = 0x81;
					}
					else
					{
						oltType[0] = 2;
						oltType[1] = 0x67;
					}

					if( (oltType[0] != pOamMsg->oltType) && (oltType[1] != pOamMsg->oltType))
#endif
					{
						OtherOltFlag = 1;
					}
						
					if(pOamMsg->onuType == 0)
					{
						loopdevIdx = 1;
						addEthPortToLoopList2(onuDevIdx,  brdIdx,  portIdx,  vid,  loopdevIdx,  loopoltSlot,  loopoltPort,0, 
											OtherOltFlag, pOamMsg->oltType, pOamMsg->oltMac);
						LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d loop in vlan %d\r\n",
						GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), brdIdx, portIdx,vid));
						check_mac_entry_delete(slotno,portno,onuDevIdx);
						
					}
					else
					{
						ErrorFlag = 0;
						if(LoopChkOnuIncludeExpandInfo == 1)
						{
							if( loopLlid < 1 || loopLlid > MAXONUPERPON )
							{
								/*VOS_ASSERT(0);*/
								if(loopChkDebugSwitch == 3)
									sys_console_printf("\r\nIn the Oam received, Llid is %d\r\n",loopLlid );
								ErrorFlag = 1;
							}
						}
						else
						{
						PonPortIdx = GetPonPortIdxBySlot(loopoltSlot,loopoltPort);
						if( !OLT_LOCAL_ISVALID(PonPortIdx) || loopLlid < 1 || loopLlid > MAXONUPERPON )
						{
							/*VOS_ASSERT(0);*/
							if(loopChkDebugSwitch == 3)
								sys_console_printf("\r\nIn the Oam received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
							ErrorFlag = 1;
							}
						}
						if(ErrorFlag == 0)
						{
							if(LoopChkOnuIncludeExpandInfo == 1)
							{
								looponuid = loopLlid;	
							}
							else
							{
								looponuid = GetOnuIdxByLlid(PonPortIdx,loopLlid);
							}
							if(looponuid == VOS_ERROR)
							{
								/*VOS_ASSERT(0);*/
								if(loopChkDebugSwitch == 3)
								{
									sys_console_printf("\r\nIn the Oam received, Slot is %d,Port is %d,Llid is %d\r\n",loopoltSlot,loopoltPort,loopLlid );
									sys_console_printf("Fetch the onu_id error ,the PonPortIdx is %d and the Onu_id is %d.\r\n",PonPortIdx,looponuid);
								}
								ErrorFlag = 1;
							}
							else
							{
								if(LoopChkOnuIncludeExpandInfo != 1)
								{
									looponuid = looponuid +1;
								}
								loopdevIdx = MAKEDEVID(loopoltSlot,loopoltPort,looponuid)/*loopoltSlot*10000+loopoltPort*1000+looponuid*/;
							}
						}

						if(ErrorFlag == 1)
						{
							loopdevIdx = 0 ;
							loopportlist1 = 0 ;
							loopportlist2 = 0 ;
						}
						
						/*if(loopport == 0)
						{
							if(test_loop_detection_oam == 2)
								sys_console_printf("\r\nloopdevIdx is %d\r\n",loopdevIdx);
							return VOS_ERROR;
						}*/
						addEthPortToLoopList2(onuDevIdx,  brdIdx,  portIdx,  vid,  loopdevIdx,  loopportlist1,  loopportlist2,0,
											OtherOltFlag, pOamMsg->oltType, pOamMsg->oltMac);
						LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d loop in vlan %d\r\n",
						GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), brdIdx, portIdx,vid));
						check_mac_entry_delete(slotno,portno,onuDevIdx);
					}

	                    		/*addEthPortToLoopListNew( ponId,  llId, pOamMsg);*/
					/*if(test_loop_detection_oam == 1)
					{
						if(onuid > 0)
						{
							onuid = GetOnuIdxByLlid(ponId,onuid);
							onuid = onuid +1;
						}
						else
							onuid = 0;
					
						if(oltboardid == 1)
							sys_console_printf("ONU : %d, slot : %d,port %d and OLT %d/%d loop\r\n",
							onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port,oltboardid,oltportid);
						else
							sys_console_printf("ONU : %d, slot : %d,port %d and ONU %d/%d/%d loop\r\n",
							onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port,oltboardid,oltportid,onuid);
					}*/
	                    }
						
			}
			else
			{
				addEthPortToLoopList_Switch(onuDevIdx, slot, pOamMsg->portId.port, pOamMsg->portId.switchPort,pOamMsg->switchMac);
				LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d switch-port %d loop\r\n",
				GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), slot, pOamMsg->portId.port,pOamMsg->portId.switchPort));
				check_mac_entry_delete(slotno,portno,onuDevIdx);
			}
			break;

		case ethPortStatus_loopClear:	
			if(loopChkDebugSwitch == 3)
			{
				test = (pOamMsg->onuLocal>>24)&0x000000ff;
				oltboardid = (pOamMsg->onuLocal>>16)&0x000000ff;
				oltportid=(pOamMsg->onuLocal>>8)&0x000000ff;
				onuid = (pOamMsg->onuLocal)&0x000000ff;
				sys_console_printf("Receive EthLoop Oam (Clear Alarm). Device:%d ,Slot: %d ,Port:%d ,Alarm type is %d,  Vlan is %d\r\n",
					onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port,pOamMsg->type, pOamMsg->vid );
				if(pOamMsg->portId.switchID != 0)
					sys_console_printf("switchID is %d, switchPort is %d\r\n",pOamMsg->portId.switchID,pOamMsg->portId.switchPort);
				if(pOamMsg->loopInf != 0)
				{
					sys_console_printf("oltType is %d\r\n",pOamMsg->oltType);
					sys_console_printf("onuLocal is %d\r\n",pOamMsg->onuLocal);
					sys_console_printf("It is %d, %d, %d, %d\r\n",test,oltboardid,oltportid,onuid);
					sys_console_printf("onuType is %d\r\n",pOamMsg->onuType);
					sys_console_printf("onuPortList is %d\r\n",pOamMsg->onuPortList);
				}
				sys_console_printf("\r\n");
			}
			if(pOamMsg->portId.switchID==0)
			{
				delEthPortFromLoopListByVid( onuDevIdx, slot, pOamMsg->portId.port, pOamMsg->vid );
				/*ethLoopAlarmClear_EventReport( (ulong_t)onuDevIdx, 1, pOamMsg->portId );*/
				eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d loop clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
			}
			else
			{
				DelFromLoopListByDelFlag( onuDevIdx, slot, pOamMsg->portId.port, pOamMsg->portId.switchPort, pOamMsg->switchMac, 3 );
				LOOP_CHK_DEBUG(1,(LOG_LOOP_CHK,LOG_WARNING,"onu %d/%d/%d eth-port %d/%d switch-port %d loop clear\r\n",
				GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), slot, pOamMsg->portId.port,pOamMsg->portId.switchPort));
			}
			break;

		default:
			rc = VOS_ERROR;
			break;
	}
	
	return rc;
}

int eventOamMsg_onuSwitch( ushort_t ponId, ushort_t llId, eventOnuSwitchMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	int onuBrdIdx = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
	
	if( onuDevIdx == VOS_ERROR )
	{
		/*sys_console_printf( "\r\nRECV OAM ERR: onu switch of pon_id=%d onu_id=%d\r\n", ponId, llId );*/
		return onuDevIdx;
	}

	if( pOamMsg->portId.port > MAX_ONU_ETHPORT)
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: onu switch invalid port! devIdx=%d port=%d\r\n", onuDevIdx, pOamMsg->portId.port) );
		return VOS_ERROR;
	}

	eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d,switch mac:%s,flag=%d\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, macAddress_To_Strings(pOamMsg->switchMac), pOamMsg->flag) );
	
	switch( pOamMsg->flag )
	{
		case onuSwitch_newRegister:
			onuSwitchNewRegSuccess_EventReport(onuDevIdx, onuBrdIdx, pOamMsg->portId.port, pOamMsg->switchMac);
			break;

		case onuSwitch_reregister:	
			onuSwitchReregSuccess_EventReport(onuDevIdx, onuBrdIdx, pOamMsg->portId.port, pOamMsg->switchMac);
			break;

		case onuSwitch_notPresent:
			onuSwitchNotPresent_EventReport(onuDevIdx, onuBrdIdx, pOamMsg->portId.port, pOamMsg->switchMac, pOamMsg->reason );
			break;
		case onuSwitch_ethEgressLimitExceed:
		case onuSwitch_ethEgressLimitExceedClear:
		case onuSwitch_ethIngressLimitExceed:
		case onuSwitch_ethIngressLimitExceedClear:
			onuSwitchEthSpeedExceed_EventReport(onuDevIdx, onuBrdIdx, pOamMsg->portId.port, pOamMsg->switchMac, pOamMsg->reason, pOamMsg->flag);
			break;

		default:
			rc = VOS_ERROR;
			break;
	}
	
	return rc;
}

int eventOamMsg_onuBackupPon( ushort_t ponId, ushort_t llId, eventOnuPonMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	int PonPort = pOamMsg->portId;
	
	if( onuDevIdx == VOS_ERROR )
	{
		/*sys_console_printf( "\r\nRECV OAM ERR: onu switch of pon_id=%d onu_id=%d\r\n", ponId, llId );*/
		return onuDevIdx;
	}

	switch( pOamMsg->flag )
	{
		case onuBackupPon_alarm:
			onuBackupPonAlarm_EventReport(onuDevIdx, 1, PonPort, pOamMsg->PonMac, pOamMsg->reason);
			break;

		case onuBackupPon_alarmClear:	
			onuBackupPonAlarmClear_EventReport(onuDevIdx, 1, PonPort, pOamMsg->PonMac, 0);
			break;
			
		default:
			rc = VOS_ERROR;
			break;
	}
	
	return rc;
}

int eventOamMsg_onuEthMon( ushort_t ponId, ulong_t llId, eventOnuEthMonMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	int slot = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH MON of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->portId.port > MAX_ONU_ETHPORT)
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: eventOamMsg_onuEthMon invalid port onuDevIdx=%d port=%d\r\n", onuDevIdx, pOamMsg->portId.port) );
		return VOS_ERROR;
	}	

	if( pOamMsg->inFlrFlag == inFlrFlag_alarm )
	{
		if( EVENT_ETH_INFLR_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFlr=%d repeat alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFlr) );
		}
		else
		{
			ethFlrAlarm_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_INFLR_FLAG_SET( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFlr=%d Alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFlr) );
		}
	}
	else if( pOamMsg->inFlrFlag == inFlrFlag_alarmClear )
	{
		if( EVENT_ETH_INFLR_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			ethFlrAlarmClear_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_INFLR_FLAG_CLEAR( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFlr=%d Clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFlr) );
		}
		else
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFlr=%d repeat clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFlr) );
		}
	}

	if( pOamMsg->outFlrFlag == outFlrFlag_alarm )
	{
		if( EVENT_ETH_OUTFLR_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, outFlr=%d repeat alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->outFlr) );
		}
		else
		{
			ethFlrAlarm_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_OUTFLR_FLAG_SET( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, outFlr=%d Alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->outFlr) );
		}
	}
	else if( pOamMsg->outFlrFlag == outFlrFlag_alarmClear )
	{
		if( EVENT_ETH_OUTFLR_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			ethFlrAlarmClear_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_OUTFLR_FLAG_CLEAR( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, outFlr=%d Clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->outFlr) );
		}
		else
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, outFlr=%d repeat clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->outFlr) );
		}
	}

	if( pOamMsg->inFerFlag == inFerFlag_alarm )
	{
		if( EVENT_ETH_INFER_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFer=%d repeat alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFer) );
		}
		else
		{
			ethFerAlarm_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_INFER_FLAG_SET( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFer=%d Alarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFer) );
		}
	}
	else if( pOamMsg->inFerFlag == inFerFlag_alarmClear )
	{
		if( EVENT_ETH_INFER_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			ethFerAlarmClear_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_INFER_FLAG_CLEAR( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFer=%d Clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFer) );
		}
		else
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d, inFer=%d repeat clear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port, pOamMsg->inFer) );
		}
	}
	
	return rc;
}

int eventOamMsg_onuEthTraffic( ushort_t ponId, ulong_t llId, eventOnuEthTrafficMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	int slot = (pOamMsg->portId.slot <= 1)?1:pOamMsg->portId.slot;
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH MON of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->portId.port > MAX_ONU_ETHPORT )
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: eventOamMsg_onuEthTraffic invalid port! onuDevIdx=%d port=%d\r\n", onuDevIdx, pOamMsg->portId.port) );
		return VOS_ERROR;
	}	
		
	if( pOamMsg->flag == ethTraffic_alarm )
	{
		if( EVENT_ETH_TRAFFIC_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d repeat trafficAlarm \r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
		}
		else
		{
			ethTranmittalIntermitAlarm_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_TRAFFIC_FLAG_SET( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d TrafficAlarm\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
		}
	}
	else if( pOamMsg->flag == ethTraffic_clear )
	{
		if( EVENT_ETH_TRAFFIC_IS_REPEATED(ponId, llId, pOamMsg->portId.port) )
		{
			ethTranmittalIntermitAlarmClear_EventReport( (ulong_t)onuDevIdx, slot, pOamMsg->portId.port );
			EVENT_ETH_TRAFFIC_FLAG_CLEAR( ponId, llId, pOamMsg->portId.port );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d TrafficClear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
		}
		else
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d ETH%d/%d repeat TrafficClear\r\n", 
				onuDevIdx, pOamMsg->portId.slot, pOamMsg->portId.port) );
		}
	}
	else
		rc = VOS_ERROR;
	
	return rc;
}
/*add by shixh20090612*/
int eventOamMsg_onuportBroadcasrFloodControl( ushort_t ponId, ulong_t llId, eventOnuportBroadcastFloodControlMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );

	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: Broad cast flood Control event of pon_id=%d, onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}
	
	if(pOamMsg->flag==broadcastfloodcontrol_alarm)
		{
			/*if(pOamMsg->control_status==1)
			{
				sys_console_printf( "\r\nonu %d brd %d port %d rate limit to 64kbps!\r\n",onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);
			}
			else if(pOamMsg->control_status==2)
			{
				sys_console_printf( "\r\nonu %d brd %d port %d shut down!\r\n",onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);

			}*/
			onuportBroadCastFloodControl_EventReport(onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);
		}
	else if(pOamMsg->flag==broadcastfloodcontrol_clear)
		{
			/*if(pOamMsg->control_status==1)
			{
				sys_console_printf( "\r\nonu %d brd %d port %d rate limit to 64kbps!\r\n",onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);

			}
			else if(pOamMsg->control_status==2)
			{
				sys_console_printf( "\r\nonu %d brd %d port %d shut down!\r\n",onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);

			}*/
			onuportBroadCastFloodControlClear_EventReport(onuDevIdx,pOamMsg->portId.slot,pOamMsg->portId.port);
		}
	
	return rc;
}
int eventOamMsg_onuStp( ushort_t ponId, ulong_t llId, eventOnuStpMsg_t *pOamMsg )
{
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: STP event of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->flag == onuStp_topologyChange )
	{
		if( EVENT_STP_TOPCHG_IS_REPEATED(ponId, llId) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d repeat Stp_topologyChange\r\n", onuDevIdx) );
		}
		else
		{
			onuStp_EventReport( (ulong_t)onuDevIdx, pOamMsg->flag );
			EVENT_STP_TOPCHG_FLAG_SET( ponId, llId );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d Stp_topologyChange\r\n", onuDevIdx) );
		}
	}
	else if( pOamMsg->flag == onuStp_newRoot )
	{
		if( EVENT_STP_NEWROOT_IS_REPEATED(ponId, llId) )
		{
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d repeat Stp_newRoot\r\n", onuDevIdx) );
		}
		else
		{
			onuStp_EventReport( (ulong_t)onuDevIdx, pOamMsg->flag );
			EVENT_STP_NEWROOT_FLAG_SET( ponId, llId );
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d Stp_newRoot\r\n", onuDevIdx) );
		}
	}
	else
		rc = VOS_ERROR;

	return rc;
}

int eventOamMsg_onuDevInfo( ushort_t ponId, ulong_t llId, eventOnuDevInfoMsg_t *pOamMsg )
{
	char *pdata = (char*)pOamMsg;
/*	uchar_t nullStr[] = "";
	char *pOnuName = nullStr;
	char *pOnuDesc = nullStr;
	char *pOnuLoca = nullStr;
	uchar_t nameLen = 0, descLen = 0, locaLen = 0;*/
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: device info. of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	pdata++;

	/* modified by chenfj 2006/11/09 
	#3144:改变一个ONU的devicedesc,device name,device location影响其他*/

	RecordOnuEUQInfo( ponId, (llId-1), pdata, 5);
	return( onuDevIdx );
#if 0
	if( *pdata != 0 )
	{
		nameLen = *pdata;
		*pdata++;
		pOnuName = pdata;
		
		pdata += nameLen;
	}
	else
	{
		pdata++;
	}
	
	if( *pdata != 0 )
	{
		descLen = *pdata;
		pdata++;
		pOnuDesc = pdata;
		
		pdata += descLen;
	}
	else
		pdata++;

	if( *pdata != 0 )
	{
		locaLen = *pdata;
		pdata++;
		pOnuLoca = pdata;
	}

	if( setDeviceName(onuDevIdx, *pOnuName, nameLen) != VOS_OK )
	{
		*(pOnuName + nameLen) = 0;
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU NAME ERR: DevIdx=%d, name=%s", onuDevIdx, pOnuName));
	}
	else
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU NAME OK: DevIdx=%d, name=%s", onuDevIdx, pOnuName));
		
	if( setDeviceDesc(onuDevIdx, *pOnuDesc, descLen) != VOS_OK )
	{
		*(pOnuDesc + descLen) = 0;
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU DESC ERR: DevIdx=%d, desc=%s", onuDevIdx, pOnuDesc));
	}
	else
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU DESC OK: DevIdx=%d, desc=%s", onuDevIdx, pOnuDesc));
		
	if( setDeviceLocation(onuDevIdx, *pOnuLoca, locaLen) != VOS_OK )
	{
		*(pOnuLoca + locaLen) = 0;
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU LOCA ERR: DevIdx=%d, desc=%s", onuDevIdx, pOnuLoca));
	}
	else
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SET ONU LOCA OK: DevIdx=%d, desc=%s", onuDevIdx, pOnuLoca));

	return VOS_OK;
#endif
}

/* added by xieshl 20061108 */
/* 扩展GW OAM中增加ONU远程升级告警 */
int eventOamMsg_onuLoad( ushort_t ponId, ulong_t llId, eventOnuLoadMsg_t *pOamMsg )
{
	int result = 0;
	CHAR *result_str[] = { "failure", "success" };
	CHAR str[64];
	int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: LOAD event of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->loadType == onuLoad_result_software )
	{
		if( pOamMsg->result == 1 )
		{
			int OnuType;
			int ret ;

			ret = GetOnuType(ponId, llId -1, &OnuType );
			onuSoftwareLoadSuccess_EventReport( (ulong_t)onuDevIdx );
			/* modified by chenfj 2007-10-8  GT810/816 
			从GW文件升级到CT文件时,升级完成后，ONU不能自动复位重启*/
			if( !(( ret == ROK ) && ((OnuType == V2R1_ONU_GT810) ||(OnuType == V2R1_ONU_GT816))) )
				ResetOnu( ponId, llId-1 );	/* added by xieshl 20061205, 加载成功后直接复位ONU */
			result = pOamMsg->result;
		}
		else
		{
			onuSoftwareLoadFailure_EventReport( (ulong_t)onuDevIdx );
		}
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU %x software update %s\r\n", onuDevIdx, result_str[result]) );
	}
	else if( pOamMsg->loadType == onuLoad_result_hardware )
	{
	}
	else if( pOamMsg->loadType == onuLoad_result_boot )
	{
	}
	else if( pOamMsg->loadType == onuLoad_result_cfgdata )
	{
	}
	else if( pOamMsg->loadType == onuLoad_result_voip )
	{
		/*added by wangxy 2007-10-29 notify onu updating task 'onuDevIdx' flash save ok*/
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_SAVE_OK, onuDevIdx, IMAGE_TYPE_NONE );

		if( pOamMsg->result == 1 )
			result = pOamMsg->result;

		VOS_Sprintf( str, "%s update %s", onu_oam_upg_file_type_2_str(IMAGE_TYPE_VOIP), result_str[result] );
		onuOamUpdPrintf( GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), str );
		
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU %x voip update %s\r\n", onuDevIdx, result_str[result]) );
	}
	else if( pOamMsg->loadType == onuLoad_result_fpga )
	{
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_SAVE_OK, onuDevIdx, IMAGE_TYPE_NONE );

		if( pOamMsg->result == 1 )
			result = pOamMsg->result;
		
		VOS_Sprintf( str, "%s update %s", onu_oam_upg_file_type_2_str(IMAGE_TYPE_FPGA), result_str[result] );
		onuOamUpdPrintf( GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), str );
		
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU %x fpga update %s\r\n", onuDevIdx, result_str[result]) );
	}
	else
		rc = VOS_ERROR;

	return rc;
}

/*add by shixh@20080514*/
int eventOamMsg_onuLoadFile( ushort_t ponId, ulong_t llId, eventOnuLoadFileMsg_t *pOamMsg )
{
	int rc = VOS_OK;
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: LOAD event of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( pOamMsg->loadfileType & AUTO_LOAD_FILETYPE_CFG )
	{
		if( pOamMsg->loadDirect == 1 )/*上传方向*/
		{
			if(pOamMsg->loadfileresult == onuLoadFile_upgrade_complete)  /*0:加载完成；1:加载失败*/
			{
				onuLoadFileSuccess_EventReport( (ulong_t)onuDevIdx);
				onuAutoLoadOnuCompletedCallback( onuDevIdx, onuLoadFile_transfer_complete );
			}
			else 
			{
				onuLoadFileFailure_EventReport( (ulong_t)onuDevIdx );
				onuAutoLoadOnuCompletedCallback( onuDevIdx, onuLoadFile_failure );
			}
		}
		else
		{
			if(pOamMsg->loadfileresult == onuLoadFile_upgrade_complete)  /*0:加载完成；1:加载失败*/
			{
				onuLoadFileSuccess_EventReport( (ulong_t)onuDevIdx);
			}
			else if(pOamMsg->loadfileresult == onuLoadFile_transfer_complete)
			{
				onuAutoLoadOnuCompletedCallback( onuDevIdx, pOamMsg->loadfileresult );
			}
			else
			{
				onuLoadFileFailure_EventReport( (ulong_t)onuDevIdx );
				onuAutoLoadOnuCompletedCallback( onuDevIdx, onuLoadFile_failure );
			}
		}
	}
	else if( pOamMsg->loadfileType & AUTO_LOAD_FILETYPE_UPG )
	{
		if( pOamMsg->loadfileresult == onuLoadFile_upgrade_complete )  /*0:加载完成；1:加载失败*/
		{
			if( pOamMsg->loadresultflag != 0 )
				onuSoftwareLoadSuccess_EventReport( (ulong_t)onuDevIdx);
			else
				onuAutoLoadUpgradeSuccess_EventReport( (ulong_t)onuDevIdx);
		}
		else if(pOamMsg->loadfileresult == onuLoadFile_transfer_complete)
		{
			onuAutoLoadOnuCompletedCallback( onuDevIdx, pOamMsg->loadfileresult );
		}
		else
		{
			if( pOamMsg->loadresultflag != 0 )
				onuSoftwareLoadFailure_EventReport( (ulong_t)onuDevIdx );
			else
				onuAutoLoadUpgradeFailure_EventReport( (ulong_t)onuDevIdx );
				
			onuAutoLoadOnuCompletedCallback( onuDevIdx, onuLoadFile_failure );/*add by shixh20090605*/
		}
	}
	else
	{
		VOS_ASSERT(0);
		rc = VOS_ERROR;
	}
#endif
	return rc;
}

int eventOamMsg_onuE1Alarm( ushort_t ponId, ulong_t llId, eventOnuE1Msg_t *pOamMsg )
{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/*int rc = VOS_OK;
	ulong_t  brdidx;*/
	uchar_t  onue1port;
	long onuDevIdx = onuIdToOnuIndex( ponId, llId );

	UCHAR slot;
	ULONG  idxs[3];
	e1PortTable_t  pE1PortTable;
	onue1port=pOamMsg->E1port.port;
	slot = (pOamMsg->E1port.slot < MIN_ONU_E1_SLOT_ID) ? MIN_ONU_E1_SLOT_ID : pOamMsg->E1port.slot;

	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: LOAD event of pon_id=%d onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if(pOamMsg->E1port.port > MAX_ONU_BOARD_E1)	
	{
		eventDbgPrintf(EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: invalid E1 port num! onuDevIdx=%x,E1 port=%d\r\n", onuDevIdx, pOamMsg->E1port.port) );
		return VOS_ERROR;
	}

	idxs[0] = (ULONG)onuDevIdx;
	idxs[1] = (ULONG)pOamMsg->E1port.slot;
	idxs[2] = (ULONG)pOamMsg->E1port.port - 1;

	/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
	if(sw_e1PortTable_get( idxs, &pE1PortTable ) == VOS_ERROR)
	{
		/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
		return VOS_ERROR;
	}

	if(pOamMsg->alarmparttype & 0x0080)/* LOS */
	{
		/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
		if ( !(pE1PortTable.eponE1PortAlarmMask & 0x0080) )
		{
			/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
			if ( !(pE1PortTable.eponE1PortAlarmStatus & 0x0080) )
			{
				/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
				e1LosAlarm_EventReport(onuDevIdx,slot,onue1port);
				pE1PortTable.eponE1PortAlarmStatus |= 0x0080;
				sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
			}
		}
	}
	else
	{
		/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
		if (pE1PortTable.eponE1PortAlarmStatus & 0x0080)
		{
			/*sys_console_printf("LINE=%d\r\n", __LINE__);*/
			/* 原来有告警过 */
			pE1PortTable.eponE1PortAlarmStatus &= ~0x0080;
			e1LosAlarmClear_EventReport(onuDevIdx,slot,onue1port);
			sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
		}
	}
#if 0
	if(pOamMsg->alarmparttype & 0x0020)/* AIS */
	{
		if ( !(pE1PortTable.eponE1PortAlarmMask & 0x0020) )
		{
			if ( !(pE1PortTable.eponE1PortAlarmStatus & 0x0020) )
			{
				e1AisAlarm_EventReport(onuDevIdx,slot,onue1port);
				pE1PortTable.eponE1PortAlarmStatus |= 0x0020;
				sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
			}
		}
	}
	else
	{
		if (pE1PortTable.eponE1PortAlarmStatus & 0x0020)
		{
			/* 原来有告警过 */
			pE1PortTable.eponE1PortAlarmStatus &= ~0x0020;
			e1AisAlarmClear_EventReport(onuDevIdx,slot,onue1port);
			sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
		}
	}

	if(pOamMsg->alarmparttype & 0x8000)/* OOS */
	{
		if ( !(pE1PortTable.eponE1PortAlarmMask & 0x0100) )
		{
			if ( !(pE1PortTable.eponE1PortAlarmStatus & 0x0100) )
			{
				e1LosAlarm_EventReport(onuDevIdx,slot,onue1port);
				pE1PortTable.eponE1PortAlarmStatus |= 0x0100;
				sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
			}
		}
	}
	else
	{
		if (pE1PortTable.eponE1PortAlarmStatus & 0x0100)
		{
			/* 原来有告警过 */
			pE1PortTable.eponE1PortAlarmStatus &= ~0x0100;
			e1LosAlarmClear_EventReport(onuDevIdx,slot,onue1port);
			sw_e1PortTable_set( LEAF_eponE1PortAlarmStatus, idxs, pE1PortTable.eponE1PortAlarmStatus );
		}
	}
#endif
#endif
	return VOS_OK;
}


/*onu 板卡插拔告警，add by shixh@20080625*/
int eventOamMsg_onuHotPlugging( ushort_t ponId, ulong_t llId, onusoltWarmInsertAndOut_t *pOamMsg )
{
	int rc = VOS_OK;
#if( EPON_MODULE_ONU_EXT_BOARD == EPON_MODULE_YES )
	LONG len;

	int onuDevIdx = onuIdToOnuIndex( ponId, llId );

	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: onu slot warm inserted and out  event of pon_id=%d, onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}

	if( (onuslot_inserted == pOamMsg->alarmFlag) || (onuslot_statuschange == pOamMsg->alarmFlag) )
	{
		len = onuExtMgmt_BrdInsertCallback( onuDevIdx, pOamMsg->slot, pOamMsg->pduData );
		
		if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )	/* modified by xieshl 20111201, 本板是主控时就不需要发了 */
			eventSync_onuData_2Master( onuDevIdx, pOamMsg->slot, pOamMsg->alarmFlag, pOamMsg->pduData, len );

		if( onuslot_inserted == pOamMsg->alarmFlag )
		{
			devBoardInterted_EventReport(onuDevIdx, pOamMsg->slot, pOamMsg->pduData[0]);
		}
		/*else if( onuslot_statuschange == pOamMsg->alarmFlag )
		{
		}*/
	}
	else if( onuslot_pull == pOamMsg->alarmFlag )
	{
		devBoardPull_EventReport( onuDevIdx, pOamMsg->slot, pOamMsg->pduData[0] );
		
		onuExtMgmt_BrdPullCallback( onuDevIdx, (ulong_t)pOamMsg->slot );/*add by zhengyt 2008-11-14,861onu板拔出时清空信息*/
		
		if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		{
			len = 6;
			eventSync_onuData_2Master( onuDevIdx, pOamMsg->slot, pOamMsg->alarmFlag, pOamMsg->pduData, len);
		}
	}
#endif
	return  rc;
}

/*onu 光参数告警，added by wangxiaoyu 2008-7-24 15:31:22*/
int eventOamMsg_onuRssi( ushort_t ponId, ulong_t llId, onuOpticalParaAlmMsg_t  *pad )
{
	int rc = VOS_ERROR;
    int iRepeatType = 0;
	
	int onuIdx = onuIdToOnuIndex(ponId, llId);

	int almFlag = pad->almFlag;

    /* B--added by liwei056@2009-10-14 for ONU's Optical Report */
	switch(pad->paraType)
	{
		case 1:
			iRepeatType = EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM_IS_REPEATED(ponId, llId);
			break;
		case 2:
			iRepeatType = EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM_IS_REPEATED(ponId, llId);
			break;
		case 3:
			iRepeatType = EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM_IS_REPEATED(ponId, llId);
			break;
		case 4:
			iRepeatType = EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM_IS_REPEATED(ponId, llId);
			break;
		case 5:
			iRepeatType = EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM_IS_REPEATED(ponId, llId);
			break;
		case 6:
			iRepeatType = EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM_IS_REPEATED(ponId, llId);
			break;
		case 7:
			iRepeatType = EVENT_ONU_OPTICAL_BIAS_HIGH_ALM_IS_REPEATED(ponId, llId);
			break;
		case 8:
			iRepeatType = EVENT_ONU_OPTICAL_BIAS_LOW_ALM_IS_REPEATED(ponId, llId);
			break;
		case 9:
			iRepeatType = EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM_IS_REPEATED(ponId, llId);
			break;
		case 10:
			iRepeatType = EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM_IS_REPEATED(ponId, llId);
			break;
		case 100:
			iRepeatType = EVENT_ONU_LASER_ALWAYS_ON_ALM_IS_REPEATED(ponId, llId);
			break;
	}
    
	if( (iRepeatType && (almFlag == 1))
        || ((!iRepeatType) && (almFlag == 2)) )
	{
		eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: ONU%d optical parameter alarm repeated\r\n", onuIdx) );
	}
	else
	{
		int slot = (pad->portId.slot <= 1)?1:pad->portId.slot;	/*兼容以前的单板设备，所以，板卡报0和1均为板卡1*/
		
    		switch(pad->paraType)
	    	{
			case 1:
				if (almFlag == 1)
					EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_POWER_HIGH_UP_ALM_CLEAR(ponId, llId)
	                    
	    			break;
	    		case 2:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_POWER_LOW_UP_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 3:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_POWER_HIGH_DOWN_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 4:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_POWER_LOW_DOWN_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 5:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_VOLTAGE_HIGH_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 6:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_VOLTAGE_LOW_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 7:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_BIAS_HIGH_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_BIAS_HIGH_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 8:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_BIAS_LOW_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_BIAS_LOW_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 9:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_TEMPERATURE_HIGH_ALM_CLEAR(ponId, llId)

	    			break;
	    		case 10:
	    			if (almFlag == 1)
					EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM(ponId, llId)
				else
					EVENT_ONU_OPTICAL_TEMPERATURE_LOW_ALM_CLEAR(ponId, llId)
				
	    			break;
#ifdef _CTC_TEST
	    		case 100:
	    			/*if (almFlag == 1)
	    			{
	    				rc = onuLaserAlwaysOnAlarmHandler( onuIdx );
					EVENT_ONU_LASER_ALWAYS_ON_ALM(ponId, llId);
				}
				else
				{
	    				rc = onuLaserAlwaysOnAlarmClearHandler( onuIdx );
					EVENT_ONU_LASER_ALWAYS_ON_ALM_CLEAR(ponId, llId);
				}*/

	    			return rc;
#endif
			default:
				return rc;
	    	}

		rc = onuOpticalParaAlm_EventReport(pad->paraType, pad->almFlag, onuIdx, slot, pad->portId.port, pad->rtVal );
		/* Noted by duzhk 2011-11-23 该函数此时未使用，
		如果使用该函数时注意此处onuOpticalParaAlm_EventReport函数使用的有问题*/

	}
    /* E--added by liwei056@2009-10-14 for ONU's Optical Report */

	return rc;
	
}
#if 0
/*add by shixh@20080313*/
/*onu 自动配置告警*/
eventOamMsg_onuautocfg( ushort_t ponId, ulong_t llId, EventonuAutocfgMsg_t *pOamMsg )
{

int rc = VOS_OK;
	int onuDevIdx = onuIdToOnuIndex( ponId, llId );
	
	if( onuDevIdx == VOS_ERROR )
	{
		sys_console_printf( "\r\nRECV OAM ERR: ETH MON of pon_id=%d, onu_id=%d\r\n", ponId, llId );
		return onuDevIdx;
	}
		
	if( pOamMsg->autocfgresult == onuAutoCfg_complete )
	{
		
			onuAutoCfgSuccess_EventReport( (ulong_t)onuDevIdx);
			
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: onu_autocfgsuccess_alarm onuDevIdx=%d\r\n", onuDevIdx ));
		
	}
	else if( pOamMsg->autocfgresult == onuAutoCfg_failure )
	{
		       onuAutoCfgFail_EventReport( (ulong_t)onuDevIdx);
			
			eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("\r\nRECV ALM OAM: onu_autocfgfailure_alarm onuDevIdx=%d\r\n", onuDevIdx ));
	}
	else
		rc = VOS_ERROR;
	
	return rc;

}
#endif

/*----------------------------------------------------------------------------*/

/* 描述: OAM接收回调函数
* 输入参数:	ponId － 
*		llId － 
*		GwProId － 该函数注册ID，未用
*		*pFrame － 指向数据的指针,无论正确错误,均由调用者释放。
*		*pSessionField － 会话ID
* 返回值: 
*/
int eventOamMsg_recvCallback( ushort_t ponId, ushort_t llId, ushort_t llid,
                                ushort_t  length, uchar_t *pFrame, uchar_t *pSessionField )
{
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(eventOamMsg_t);
	eventOamMsg_t *pOamData;
	ULONG aulMsg[4] = { MODULE_EVENT, FC_EVENT_OAM_RX, 0, 0};

	if( (pFrame == NULL) ||(pSessionField == NULL) || (length == 0) )
	{
		if( pFrame ) 	VOS_Free( pFrame );
		if( pSessionField ) VOS_Free( pSessionField );
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = ( SYS_MSG_S* )VOS_Malloc( msgLen, MODULE_EVENT );
	if ( NULL == pstMsg )
	{
		VOS_Free( pFrame );
		VOS_Free( pSessionField );
		return VOS_ERROR;
	}
	pOamData = (eventOamMsg_t *)(pstMsg + 1);
	
	/*VOS_MemZero( pstMsg, msgLen );*/
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->usMsgCode = FC_EVENT_OAM_RX;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;
	pstMsg->ptrMsgBody = pOamData;
	pstMsg->usFrameLen = sizeof(eventOamMsg_t);

	pOamData ->ponId = ponId;
	pOamData->llId = llId;
	pOamData->length = length;
	pOamData->pFrame = pFrame ;
	pOamData->pSessionField = pSessionField;

	aulMsg[3] = (unsigned long )pstMsg;
	if( VOS_QueSend( eventQueId, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
	{
		/*VOS_ASSERT(0);
		sys_console_printf(" Event report is busy\r\n" );*/
		VOS_Free( pFrame );
		VOS_Free( pSessionField );
		VOS_Free( pstMsg );
		return VOS_ERROR;
	}
	return VOS_OK;
}

int eventOamMsg_recvProc( eventOamMsg_t *pMsgBuf )
{
	int rc = VOS_OK;
	ushort_t ponId, llId;
	ULONG  length;
	uchar_t *pSessionField;
	commEventOamMsg_t *pOamMsg;
	eventOnuEthLoopMsg_t *pEthLoopMsg ;
	if( NULL == pMsgBuf )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	ponId = pMsgBuf->ponId;
	llId = pMsgBuf->llId;
	length = pMsgBuf->length;
	pSessionField = pMsgBuf->pSessionField;
	pOamMsg = (commEventOamMsg_t *)pMsgBuf->pFrame;
	if( pOamMsg == NULL )
	{
		if( pSessionField )
			VOS_Free( pSessionField );
		VOS_ASSERT( 0 );
		return VOS_ERROR;	
	}

	if( eventDebugSwitch & EVENT_DEBUGSWITCH_OAM )
	{
		sys_console_printf( "Rx ALM OAM: ponId=%d, llId=%d, length=%d\r\n", ponId, llId, length );
		pktDataPrintf( (UCHAR *)pOamMsg, length );
	}

	switch( pOamMsg->type )
	{
		case EVENT_TYPE_ONU_TEMPERATURE:
			rc = eventOamMsg_temperature( ponId, llId, (eventOnuTemperatureMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONUETH_STATUS:
			rc = eventOamMsg_onuEthPortStatus( ponId, llId, (eventOnuEthStatusMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONUETH_LOOP:
			if(length < sizeof(eventOnuEthLoopMsg_t)) /*modefied by duzhk 2011.1.25*/
			{
				pEthLoopMsg = (eventOnuEthLoopMsg_t *)pOamMsg;
				pEthLoopMsg->loopInf = 0;
			}
				/*VOS_MemZero((void *)((u_char *)pOamMsg+length), sizeof(eventOnuEthLoopMsg_t)-length);*/
			/*if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
				rc = eventOamMsg_onuEthPortLoop_CDP(ponId, llId, (eventOnuEthLoopMsg_t *)pOamMsg);
			else*/
			pEthLoopMsg = ( eventOnuEthLoopMsg_t * )VOS_Malloc( sizeof(eventOnuEthLoopMsg_t), MODULE_EVENT );
			if ( NULL == pEthLoopMsg )
			{
				break;
			}
			VOS_MemZero(pEthLoopMsg, sizeof(eventOnuEthLoopMsg_t));

			VOS_MemCpy(pEthLoopMsg, pOamMsg, sizeof(eventOnuEthLoopMsg_t));
			
			ethLoopOamRecvCallback( ponId, llId, pEthLoopMsg );
			/*rc = eventOamMsg_onuEthPortLoop( ponId, llId, (eventOnuEthLoopMsg_t *)pOamMsg );*/
			break;
		case EVENT_TYPE_ONUETH_MON:
			rc = eventOamMsg_onuEthMon( ponId, llId, (eventOnuEthMonMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONUETH_TRAFFIC:
			rc = eventOamMsg_onuEthTraffic( ponId, llId, (eventOnuEthTrafficMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONUPORT_BROADCAST_CONTROL:
			rc=eventOamMsg_onuportBroadcasrFloodControl( ponId, llId, (eventOnuportBroadcastFloodControlMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_STP:
			rc = eventOamMsg_onuStp( ponId, llId, (eventOnuStpMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_DEVINFO:
			rc = eventOamMsg_onuDevInfo( ponId, llId, (eventOnuDevInfoMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_LOAD:	/* added by xieshl 20061108 */
			rc = eventOamMsg_onuLoad( ponId, llId, (eventOnuLoadMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_LOADFILE:/*add byshixh@20080515*/
			rc = eventOamMsg_onuLoadFile( ponId, llId, (eventOnuLoadFileMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_WARM_INSERT_AND_OUT:/*add byshixh@20080624*/
			rc = eventOamMsg_onuHotPlugging( ponId, llId, (onusoltWarmInsertAndOut_t *)pOamMsg );
			break;
		case EVENT_TYPE_ONU_OPTICAL_PARA_ALM:
			/*rc = eventOamMsg_onuRssi(ponId, llId, (onuOpticalParaAlmMsg_t*)pOamMsg);*/
			break;
		/* begin: added by jianght 20090319  */
		case EVENT_TYPE_ONU_E1:
			rc = eventOamMsg_onuE1Alarm(ponId, llId,(eventOnuE1Msg_t*)pOamMsg);
			break;
		/* end: added by jianght 20090319 */
		case EVENT_TYPE_ONU_SWITCH:
			rc = eventOamMsg_onuSwitch( ponId, llId, (eventOnuSwitchMsg_t *)pOamMsg );
			break;
		case EVENT_TYPE_PON_UPLINK_TRAFFIC_CHECK:
			rc = eventOamMsg_uplinkTraffic( ponId, llId, pOamMsg );
			if( pOamMsg )
				VOS_Free( pOamMsg );
			if( pSessionField )
				VOS_Free( pSessionField );

			return rc;
        case EVENT_TYPE_ONU_PON_MONITOR:
            rc = eventOamMsg_onuBackupPon(ponId, llId, (eventOnuPonMsg_t*)pOamMsg);
            break;
            
		default:
			rc = VOS_ERROR;
			break;
	}

	/*if( rc == VOS_OK )*/
	if( pSessionField == NULL )
	{
		pSessionField = VOS_Malloc( 8, MODULE_EVENT );
		if( pSessionField )
			VOS_MemZero( pSessionField, 8 );
	}
	if( pSessionField )
	{
		if(( rc= Comm_Alarm_Response_transmit(ponId, llId, (UCHAR *)pOamMsg, length, pSessionField))== 0 )
		{
			/*eventDbgPrintf( EVENT_DEBUGSWITCH_OAM, ("SEND ACK OAM ponId=%d, llId=%d, length=%d\r\n", ponId, llId, length) );*/
			if( eventDebugSwitch & EVENT_DEBUGSWITCH_OAM )
			{
				sys_console_printf( "Tx ALM OAM: ponId=%d, onu=%d, length=%d\r\n", ponId, llId, length );
				pktDataPrintf( (UCHAR *)pOamMsg, length );
			}
		}
		else
		{
			/*if( rc != OAM_ONU_DEREGISTER_ERR )
				sys_console_printf("Tx ALM OAM ERR: ponId=%d, onu=%d\r\n",ponId, llId );*/
		}
	}

	if( pOamMsg )
		VOS_Free( pOamMsg );
	if( pSessionField )
		VOS_Free( pSessionField );
	
	return rc;
}

