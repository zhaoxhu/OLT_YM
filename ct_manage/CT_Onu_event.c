#include "OltGeneral.h"
#include "gwEponSys.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"
#include "eventOam.h"
#include "lib_ethPortMib.h"
#include	"lib_gwEponMib.h"
#include  "../cli/Olt_cli.h"
#include "CT_Onu_event.h" 
#include "V2R1General.h"
#include "../MIB/lib_eponOpticPowerMon.h"
#include "vos/vospubh/cdp_syn.h"
#include "vos/vospubh/vos_sysmsg.h"
#include "vos/vospubh/cdp_pub.h" 

int sfun_CTCONU_SetOnuAlarmThrehold(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, ULONG alarm, ULONG clear);
int sfun_CTCONU_SetPonAlarmThrehold(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, ULONG alarm, ULONG clear);
long getCtcAlarmflagByAlarmId(CTC_STACK_alarm_id_t code);
long TranslateOnuReportValue(CTC_STACK_alarm_id_t code,long value);

ULONG g_CtcOnuEventSemId = 0;
ULONG ctcEventQueId = 0;
VOS_HANDLE ctcEventTaskId = NULL;
int ReportDebugflag = 0;
int ConfigDebugflag = 0;
#if 1
int Is811cOnu(short int PonPortIdx, short int OnuIdx)
{
    ULONG model = 0;
    int onuType = 0;
	int onuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    
	ONU_MGMT_SEM_TAKE;
	onuType = OnuMgmtTable[onuEntry].DeviceInfo.type;
	model = OnuMgmtTable[onuEntry].onu_model;
	ONU_MGMT_SEM_GIVE;

    if(onuType == V2R1_ONU_CTC && model == 0x30000000)
        return VOS_OK;
    else
        return VOS_ERROR;
        
}
int Is831cOnu(short int PonPortIdx, short int OnuIdx)
{
    ULONG model = 0;
    int onuType = 0;
	int onuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    
	ONU_MGMT_SEM_TAKE;
	onuType = OnuMgmtTable[onuEntry].DeviceInfo.type;
	model = OnuMgmtTable[onuEntry].onu_model;
	ONU_MGMT_SEM_GIVE;

    if(onuType == V2R1_ONU_CTC && model == 0x38333143)
        return VOS_OK;
    else
        return VOS_ERROR;
        
}

int CTC_TranslateTemperature( long oldTemp, long *newTemp)/*oldTemp 取值范围是 -128~128 'C
                                                                                                                  转换后的取值范围是
                                                                                                                   -32768~32768*/
{
    *newTemp = oldTemp*256;
	return VOS_OK; 
}
int CTC_TranslateBiasCurrent( long oldBiasCurrent, long *newBiasCurrent)/*oldBiasCurrent 取值范围0~131mA*/
{
    *newBiasCurrent = oldBiasCurrent*1000/2; 
	return VOS_OK;
}
long CTC_TranslateWorkVoltage(long WorkVoltage, long *newWorkVoltage)/*WorkVoltage 取值范围0~65.5(100mV)*/
{
    *newWorkVoltage = WorkVoltage*1000;
	return VOS_OK;
}
long CTC_TranslateOpticalPower( long OpticalPower, long *newOpticalPower)/*OpticalPower 取值范围-400~82(0.1dBm)*/
{
	*newOpticalPower = 10000.0*(double)pow((double)10.0,(double)((double)OpticalPower - 0.5)/100.0);
	return VOS_OK;
}

static int sfun_CTCONU_SetAlarmEnable(CTC_STACK_alarm_id_t code, int enable)
{
    int ret = VOS_OK;
        switch(code)
        {
/*Onu*/
            case EQUIPMENT_ALARM:
                CTCOnuAlarmConfig_onu.EquipmentAlarm= enable;
                break;
            case POWERING_ALARM:
                CTCOnuAlarmConfig_onu.PowerAlarm= enable;
                break;
            case BATTERY_MISSING:
            case BATTERY_FAILURE:
            case BATTERY_VOLT_LOW:
                CTCOnuAlarmConfig_onu.BatteryAlarm= enable;
                break;
            case PHYSICAL_INTRUSION_ALARM:
                CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm= enable;
                break;
            case ONU_SELF_TEST_FAILURE:
                CTCOnuAlarmConfig_onu.OnuSelfTestFailure= enable;
                break;
            case ONU_TEMP_HIGH_ALARM:
            case ONU_TEMP_LOW_ALARM:
                CTCOnuAlarmConfig_onu.OnuTempAlarm= enable;
                break;
            case IAD_CONNECTION_FAILURE:
                CTCOnuAlarmConfig_onu.IadConnectFailure= enable;
                break;
            case PON_IF_SWITCH:
                CTCOnuAlarmConfig_onu.PonSwitch= enable;
                break;           
/*Eth port*/
            case ETH_PORT_AUTO_NEG_FAILURE:
                CTCOnuAlarmConfig_port.EthAutoNegFailure= enable;
                break;
            case ETH_PORT_LOS:
                CTCOnuAlarmConfig_port.EthLos= enable;
                break;
            case ETH_PORT_FAILURE:
                CTCOnuAlarmConfig_port.EthAutoNegFailure= enable;
                break;
            case ETH_PORT_LOOPBACK:
                /*CTCOnuAlarmConfig_port.EthLoopback= enable;*/
                break;
            case ETH_PORT_CONGESTION:
                CTCOnuAlarmConfig_port.EthCongestion= enable;
                break;
            default:
                ret = VOS_ERROR;
                break;
        }
    return ret;
}
int CTCONU_SetAlarmEnable(CTC_STACK_alarm_id_t code, int enable)
{
    int ret = VOS_ERROR;
    eventCtcSyncCfgMsg_t cfgMsg;
    
    ONU_EVENT_CONF_SEM_TAKE
    {
        ret = sfun_CTCONU_SetAlarmEnable(code, enable);
    }
    ONU_EVENT_CONF_SEM_GIVE
        
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        CTCONU_ConfAlarmData(code, 0, enable, 0, 0);
    }    
    /*else */
    if (SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( &cfgMsg, sizeof(eventCtcSyncCfgMsg_t) );
        cfgMsg.subType = FC_EVENT_CFG_SYNC_ENABLE;
		cfgMsg.syncEnable = enable;
		cfgMsg.Code = code;
        ret = CTC_eventSync_configData2AllSlave((VOID *)&cfgMsg, sizeof(eventCtcSyncCfgMsg_t));
	}
    return ret;
}

static int sfun_CTCONU_GetAlarmEnable(CTC_STACK_alarm_id_t code, int *enable)
{
    int ret = VOS_OK;
        switch(code)
        {
/*Onu*/            
            case EQUIPMENT_ALARM:
                *enable = CTCOnuAlarmConfig_onu.EquipmentAlarm;
                break;
            case POWERING_ALARM:
                *enable = CTCOnuAlarmConfig_onu.PowerAlarm;
                break;
            case BATTERY_MISSING:
            case BATTERY_FAILURE:
            case BATTERY_VOLT_LOW:
                *enable = CTCOnuAlarmConfig_onu.BatteryAlarm;
                break;
            case PHYSICAL_INTRUSION_ALARM:
                *enable = CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm;
                break;
            case ONU_SELF_TEST_FAILURE:
                *enable = CTCOnuAlarmConfig_onu.OnuSelfTestFailure;
                break;
            case ONU_TEMP_HIGH_ALARM:
            case ONU_TEMP_LOW_ALARM:
                *enable = CTCOnuAlarmConfig_onu.OnuTempAlarm;
                break;
            case IAD_CONNECTION_FAILURE:
                *enable = CTCOnuAlarmConfig_onu.IadConnectFailure;
                break;
            case PON_IF_SWITCH:
                *enable = CTCOnuAlarmConfig_onu.PonSwitch;
                break;
/*Eth port*/
            case ETH_PORT_AUTO_NEG_FAILURE:
                *enable = CTCOnuAlarmConfig_port.EthAutoNegFailure;
                break;
            case ETH_PORT_LOS:
                *enable = CTCOnuAlarmConfig_port.EthLos;
                break;
            case ETH_PORT_FAILURE:
                *enable = CTCOnuAlarmConfig_port.EthFailure;
                break;
            case ETH_PORT_LOOPBACK:
                /**enable = CTCOnuAlarmConfig_port.EthLoopback;*/
                break;
            case ETH_PORT_CONGESTION:
                *enable = CTCOnuAlarmConfig_port.EthCongestion;
                break;
            default:
                ret = VOS_ERROR;
                break;
        }
    return ret;
}
int CTCONU_GetAlarmEnable(CTC_STACK_alarm_id_t code, int *enable)
{
    int ret = VOS_ERROR;
    ONU_EVENT_CONF_SEM_TAKE
    {
        ret = sfun_CTCONU_GetAlarmEnable(code, enable);
    }
    ONU_EVENT_CONF_SEM_GIVE
    return ret;
}
static int sfun_CTCONU_SetOnuAlarmThreshold(CTC_STACK_alarm_id_t code, LONG alarm, LONG clear)
{
    int ret = VOS_OK;
        switch(code)
        {
/*Onu*/            
            case BATTERY_VOLT_LOW:
                CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr = alarm;
                CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr = clear;
                break;
            case ONU_TEMP_HIGH_ALARM:
                CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr = alarm;
                CTCOnuAlarmConfig_onu.OnuTempHighClearThr = clear;
                break;
            case ONU_TEMP_LOW_ALARM:
                CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr = alarm;
                CTCOnuAlarmConfig_onu.OnuTempLowClearThr = clear;
                break;
            default:
                ret = VOS_ERROR;
        }
    return ret;
}
int CTCONU_SetAlarmThreshold(CTC_STACK_alarm_id_t code, int alarm, int clear)
{
    int ret = VOS_ERROR;
    eventCtcSyncCfgMsg_t cfgMsg;

    ONU_EVENT_CONF_SEM_TAKE
    {
        ret = sfun_CTCONU_SetOnuAlarmThreshold(code, alarm, clear);
    }
    ONU_EVENT_CONF_SEM_GIVE

    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        CTCONU_ConfAlarmData(code, 1, 0, alarm, clear);
    }
    /*else*/
    if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( &cfgMsg, sizeof(eventCtcSyncCfgMsg_t) );
        cfgMsg.subType = FC_EVENT_CFG_SYNC_THRESHOLD;
		cfgMsg.threshold = alarm;
		cfgMsg.Clearthreshold = clear;
		cfgMsg.Code = code;
		ret = CTC_eventSync_configData2AllSlave( (VOID *)&cfgMsg, sizeof(eventCtcSyncCfgMsg_t) );
	}  
    return ret;
}

static int sfun_CTCONU_GetOnuAlarmThreshold(CTC_STACK_alarm_id_t code, LONG *alarm, LONG *clear)
{
    int ret = VOS_OK;
        switch(code)
        {
/*Onu*/            
            case BATTERY_VOLT_LOW:
                *alarm = CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr;
                *clear = CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr;
                break;
            case ONU_TEMP_HIGH_ALARM:
                *alarm = CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr;
                *clear = CTCOnuAlarmConfig_onu.OnuTempHighClearThr;
                break;
            case ONU_TEMP_LOW_ALARM:
                *alarm = CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr;
                *clear = CTCOnuAlarmConfig_onu.OnuTempLowClearThr;
                break;
            default:
                ret = VOS_ERROR;
        }
    return ret;
}
int CTCONU_GetAlarmThreshold(CTC_STACK_alarm_id_t code, LONG *alarm, LONG *clear)
{
    int ret = VOS_ERROR;

    ONU_EVENT_CONF_SEM_TAKE
    {
        ret = sfun_CTCONU_GetOnuAlarmThreshold(code, alarm, clear);
    }
    ONU_EVENT_CONF_SEM_GIVE

    return ret;
}
int SetDebugEnable(int flag, int enable)
{
    int ret = VOS_OK;
   
    switch(flag)
    {
        case Configflag:
            ConfigDebugflag = enable;
            break;
        case Reportflag:
            ReportDebugflag = enable;
            break;
        default:
            ret = VOS_ERROR;
            break;
    } 
    return ret;
}
int getDebugEnable(int flag, int *enable)
{
    int ret = VOS_OK;
    switch(flag)
    {
        case Configflag:
            *enable = ConfigDebugflag;
            break;
        case Reportflag:
            *enable = ReportDebugflag;
            break;
        default:
            ret = VOS_ERROR;
            break;
    } 
    return ret;
}


int eventSync_SetAlarmData(eventCtcSyncCfgMsg_t* pRecvMsg )
{
    int ret = VOS_ERROR;
    if( pRecvMsg == NULL )
	{
		VOS_ASSERT(0);
		return ret;
	}
    switch(pRecvMsg->subType)
    {
        case FC_EVENT_CFG_SYNC_ENABLE:
            ret = CTCONU_SetAlarmEnable(pRecvMsg->Code, pRecvMsg->syncEnable);
            break;
        case FC_EVENT_CFG_SYNC_THRESHOLD:
            ret = CTCONU_SetAlarmThreshold(pRecvMsg->Code, pRecvMsg->threshold, pRecvMsg->Clearthreshold); 
            break;
        default:
            break;
    }
    return ret;
}

int CTC_eventSync_configData2Slave( ULONG slotno, VOID *pCfgData, ULONG dataLen )
{
	LONG rc;
	SYS_MSG_S   *pstMsg;
	eventCtcSyncCfgMsg_t *pSndCfgMsg;
	ULONG msgLen;

	if( (pCfgData == NULL) || (dataLen == 0) || (dataLen >sizeof(eventCtcSyncCfgMsg_t)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	msgLen = sizeof(SYS_MSG_S) + dataLen;
	
	if(  SYS_MODULE_IS_READY(slotno)  )
	{
		pstMsg = CDP_AllocMsg( msgLen, MODULE_EVENT );
		if ( NULL == pstMsg )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		pSndCfgMsg = (eventCtcSyncCfgMsg_t *)(pstMsg + 1);

		VOS_MemZero( pstMsg, msgLen );
		pstMsg->ulSrcModuleID = MODULE_EVENT;
		pstMsg->ulDstModuleID = MODULE_EVENT;
		pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
		pstMsg->ulDstSlotID = slotno;/*目的slot*/
		pstMsg->ucMsgType = MSG_NOTIFY;
		pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
		pstMsg->ptrMsgBody = (VOID *)pSndCfgMsg;
		pstMsg->usFrameLen = dataLen;
		pstMsg->usMsgCode = FC_EVENT_CFG_SYNC_CTC;

		VOS_MemCpy( pSndCfgMsg, pCfgData, dataLen );
				
		rc = CDP_Send( RPU_TID_CDP_EVENT_CTC, slotno,  RPU_TID_CDP_EVENT_CTC,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_EVENT );
		if( rc !=  VOS_OK )
		{
			VOS_ASSERT(0);
			CDP_FreeMsg( (void *) pstMsg );
			return VOS_ERROR;
		}	
	}

	return VOS_OK;
}
int CTC_eventSync_configData2AllSlave( VOID *pCfgData, ULONG dataLen )
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
        if((slotno != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slotno))&&(SYS_MODULE_SLOT_ISHAVECPU(slotno)))
    		CTC_eventSync_configData2Slave( slotno, pCfgData, dataLen );
	}
	return VOS_OK;
}

#endif

int CTCONU_ethloopport(ULONG devIdx, ULONG brdIdx, ULONG portIdx, int status)
{

    int  ponslot,ponport,onuid,llid=0,ponId;
    eventOnuEthLoopMsg_t *pEthLoopMsg ;
    
    pEthLoopMsg = ( eventOnuEthLoopMsg_t * )VOS_Malloc( sizeof(eventOnuEthLoopMsg_t), MODULE_EVENT );
    if ( NULL == pEthLoopMsg )
	{
	    return VOS_ERROR;
	}
  
    ponslot=GET_PONSLOT(devIdx);
    ponport=GET_PONPORT(devIdx);
    onuid=GET_ONUID(devIdx);
    
    ponId = GetPonPortIdxBySlot(ponslot,ponport);
    
     
    if( INVALID_LLID == (llid = GetLlidActivatedByOnuIdx(ponId, onuid-1)) )
     {
        return  VOS_ERROR;
     }
    
    VOS_MemSet(pEthLoopMsg, 0, sizeof(eventOnuEthLoopMsg_t));
    
    pEthLoopMsg->portId.slot = brdIdx;
    pEthLoopMsg->portId.port = portIdx;
    pEthLoopMsg->loopStatus = status;
    pEthLoopMsg->onuLocal = devIdx;

    ethLoopOamRecvCallback(ponId,onuid ,pEthLoopMsg);
        
    return VOS_OK;
    
}


#if 1/*对CTC  接口*/
int sfun_CTCONU_SetOnuAlarmStatus(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, bool flagenable)
{
    int ret = VOS_ERROR;
    short int llid;
    CTC_management_object_t management_object;
    CTC_management_object_index_t *management_object_index;
    CONFIG_DEBUG_PRINTF("\r\rOLTid = %d ONUid = %d \r\n",olt_id,onu_id);
    CONFIG_DEBUG_PRINTF("Enable = %d Alarm_id = %0x\r\n",flagenable,code);
    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            management_object.leaf = CTC_MANAGEMENT_OBJECT_LEAF_NONE;
            management_object_index = &management_object.index;
        	management_object_index->frame_number = 0;
        	management_object_index->port_number = 0xFFFF;
        	management_object_index->slot_number = 63;
        	management_object_index->port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
            #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			ret = OnuMgt_SetAlarmAdminState(olt_id, onu_id, &management_object, code, flagenable);
			#else
            ret = CTC_STACK_set_alarm_admin_state(olt_id, llid, management_object, code, flagenable);
			#endif
        }
    }  
    
    CONFIG_DEBUG_PRINTF("test_final:ret = %d\r\n",ret);
    return ret;
}



int sfun_CTCONU_SetPonAlarmStatus(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, bool flagenable)
{
    int ret = VOS_ERROR;
    short int llid;
    CTC_management_object_t management_object;
    CTC_management_object_index_t *management_object_index;
    CONFIG_DEBUG_PRINTF("\r\nOLTid = %d ONUid = %d \r\n",olt_id,onu_id);
    CONFIG_DEBUG_PRINTF("Enable = %d Alarm_id = %0x\r\n",flagenable,code);
    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            management_object.leaf = CTC_MANAGEMENT_OBJECT_LEAF_PON_IF;
            management_object_index = &management_object.index;
        	management_object_index->frame_number = 0;
        	management_object_index->port_number = 0;
        	management_object_index->slot_number = 0;
        	management_object_index->port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE;
            #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			ret = OnuMgt_SetAlarmAdminState(olt_id, onu_id, &management_object, code, flagenable);
			#else
            ret = CTC_STACK_set_alarm_admin_state(olt_id, llid, management_object, code, flagenable);
			#endif
        }
    }  
    CONFIG_DEBUG_PRINTF("test_final:ret = %d\r\n",ret);
    return ret;
}

int CTCONU_ConfAlarmStatus(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t alarm_id ,int enable)
{
    int ret = VOS_OK;
    bool flagenable = 0;
    long alarm_temp = 0;
    long clear_temp = 0;
    switch(alarm_id)
    {	/* modified by xieshl 20161009, 避免重复配置，解决现场810H不注册问题 */
#if 0
        case BATTERY_MISSING:
        case BATTERY_FAILURE:
        case BATTERY_VOLT_LOW:
            ret = sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_MISSING, enable);
            ret |= sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_FAILURE, enable);
            ret |= sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_VOLT_LOW, enable);
            if(CTCONU_GetAlarmThreshold(BATTERY_VOLT_LOW,&alarm_temp,&clear_temp)!=VOS_ERROR && enable)
                ret |= sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, BATTERY_VOLT_LOW,alarm_temp,clear_temp);
            break;
        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
            sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ONU_TEMP_HIGH_ALARM, enable);
            sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ONU_TEMP_LOW_ALARM, enable);
            if(CTCONU_GetAlarmThreshold(ONU_TEMP_HIGH_ALARM,&alarm_temp,&clear_temp)!=VOS_ERROR && enable)
                sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, ONU_TEMP_HIGH_ALARM,alarm_temp,clear_temp);
            if(CTCONU_GetAlarmThreshold(ONU_TEMP_LOW_ALARM,&alarm_temp,&clear_temp)!=VOS_ERROR && enable)
                sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, ONU_TEMP_LOW_ALARM,alarm_temp,clear_temp);
            break;
        case EQUIPMENT_ALARM:
        case POWERING_ALARM:
        case PHYSICAL_INTRUSION_ALARM:
        case ONU_SELF_TEST_FAILURE:
        case IAD_CONNECTION_FAILURE:
	    case PON_IF_SWITCH:
        case SELF_TEST_FAILURE:
        case ETH_PORT_AUTO_NEG_FAILURE:
        case ETH_PORT_LOS:
        case ETH_PORT_FAILURE:
        case ETH_PORT_CONGESTION:
        case POTS_PORT_FAILURE:
        case ETH_PORT_LOOPBACK:
            sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, alarm_id, enable);
            break;
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM:
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
        case TX_BIAS_HIGH_ALARM: 
        case TX_BIAS_LOW_ALARM:
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM: 
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            flagenable = GetOnuOpticalPowerEnable() == V2R1_ENABLE?1:0;
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, RX_POWER_HIGH_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, RX_POWER_LOW_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_POWER_HIGH_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_POWER_LOW_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_BIAS_HIGH_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_BIAS_LOW_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, VCC_HIGH_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, VCC_LOW_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TEMP_HIGH_ALARM, flagenable);
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TEMP_LOW_ALARM, flagenable); 
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_recv_oppower_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_HIGH_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_recv_oppower_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_LOW_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_trans_oppower_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_HIGH_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_trans_oppower_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_LOW_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_cur_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_cur_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_HIGH_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_cur_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_cur_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_LOW_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_vol_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_vol_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_HIGH_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_vol_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_vol_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_LOW_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_tempe_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_tempe_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_HIGH_ALARM, alarm_temp, clear_temp);

                alarm_temp = getOpticalPowerThreshold(field_pon_tempe_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_tempe_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_LOW_ALARM, alarm_temp, clear_temp);
            }
            break;
        default:
            ret = VOS_ERROR;
            break;
#else
        case BATTERY_MISSING:
        case BATTERY_FAILURE:
        case BATTERY_VOLT_LOW:
            ret = sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, alarm_id, enable);
            if(CTCONU_GetAlarmThreshold(BATTERY_VOLT_LOW,&alarm_temp,&clear_temp)!=VOS_ERROR && enable)
                ret |= sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, alarm_id,alarm_temp,clear_temp);
            break;

        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
            sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, alarm_id, enable);
            if(CTCONU_GetAlarmThreshold(alarm_id,&alarm_temp,&clear_temp)!=VOS_ERROR && enable)
                sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, alarm_id,alarm_temp,clear_temp);
            break;

        case EQUIPMENT_ALARM:
        case POWERING_ALARM:
        case PHYSICAL_INTRUSION_ALARM:
        case ONU_SELF_TEST_FAILURE:
        case IAD_CONNECTION_FAILURE:
	    case PON_IF_SWITCH:
        case SELF_TEST_FAILURE:
        case ETH_PORT_AUTO_NEG_FAILURE:
        case ETH_PORT_LOS:
        case ETH_PORT_FAILURE:
        case ETH_PORT_CONGESTION:
        case POTS_PORT_FAILURE:
        case ETH_PORT_LOOPBACK:
            sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, alarm_id, enable);
            break;

        case RX_POWER_HIGH_ALARM:
        
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_recv_oppower_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case RX_POWER_LOW_ALARM:
         
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_recv_oppower_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TX_POWER_HIGH_ALARM:
          
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_trans_oppower_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TX_POWER_LOW_ALARM:
        
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_trans_oppower_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TX_BIAS_HIGH_ALARM: 
          
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_cur_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_cur_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TX_BIAS_LOW_ALARM:
           
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_cur_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_cur_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case VCC_HIGH_ALARM:
        
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_vol_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_vol_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case VCC_LOW_ALARM: 
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_vol_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_vol_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TEMP_HIGH_ALARM:
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable);
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_tempe_high, 0);
                clear_temp = alarm_temp - getOpticalPowerDeadZone(field_tempe_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
			break;
        case TEMP_LOW_ALARM:
            sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, alarm_id, flagenable); 
            if(flagenable)
            {
                alarm_temp = getOpticalPowerThreshold(field_pon_tempe_low, 0);
                clear_temp = alarm_temp + getOpticalPowerDeadZone(field_tempe_dead_zone);
                sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            }
            break;
        default:
            ret = VOS_ERROR;
            break;
#endif
    }
    return ret; 
}
long TranslateOnuConfigValue(CTC_STACK_alarm_id_t code,long value)
{
    long ret = VOS_ERROR;
    long temp = 0;
    switch(code)
    {
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM: 
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
            if(CTC_TranslateOpticalPower(value, &temp) == VOS_OK)
                ret = temp;
            break;
        case TX_BIAS_HIGH_ALARM:
        case TX_BIAS_LOW_ALARM:
            if(CTC_TranslateBiasCurrent(value, &temp) == VOS_OK)
                ret = temp;
            break;
        case BATTERY_VOLT_LOW:
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM:
            if(CTC_TranslateWorkVoltage(value, &temp) == VOS_OK)
                ret = temp;
            break;
        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            if(CTC_TranslateTemperature(value, &temp) == VOS_OK)
                ret = temp;
            break;
        default:
            VOS_ASSERT(0);
            break;
    }
    return ret;
}

int sfun_CTCONU_SetOnuAlarmThrehold(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, ULONG alarm, ULONG clear)
{
    int ret = VOS_ERROR;
    short int llid;
    CTC_management_object_t management_object;
    CTC_management_object_index_t *management_object_index;
    ULONG alarm_temp = 0, clear_temp = 0;
    alarm_temp = TranslateOnuConfigValue(code, alarm);
    clear_temp = TranslateOnuConfigValue(code, clear);

    CONFIG_DEBUG_PRINTF("\r\nOLTid = %d ONUid = %d alarm_id = %0x\r\n",olt_id,onu_id,code);    
    CONFIG_DEBUG_PRINTF("alarm = %d clear = %d alarm_temp = %d clear_temp = %d \r\n",alarm,clear,alarm_temp,clear_temp);
    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            management_object.leaf = CTC_MANAGEMENT_OBJECT_LEAF_NONE;
            management_object_index = &management_object.index;
        	management_object_index->frame_number = 0;
        	management_object_index->port_number = 0xFFFF;
        	management_object_index->slot_number = 63;
        	management_object_index->port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
            /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
            ret = OnuMgt_SetAlarmThreshold(olt_id, onu_id, &management_object, code, alarm_temp, clear_temp);
        }
    }
    CONFIG_DEBUG_PRINTF("test_final:ret = %d\r\n",ret);
    return ret;
}
int sfun_CTCONU_SetPonAlarmThrehold(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t code, ULONG alarm, ULONG clear)
{
    int ret = VOS_ERROR;
    short int llid;
    CTC_management_object_t management_object;
    CTC_management_object_index_t *management_object_index;
    ULONG alarm_temp = 0, clear_temp = 0;
    alarm_temp = TranslateOnuConfigValue(code, alarm);
    clear_temp = TranslateOnuConfigValue(code, clear);
    
    CONFIG_DEBUG_PRINTF("\r\nOLTid = %d ONUid = %d alarm_id = %0x\r\n",olt_id,onu_id,code);
    CONFIG_DEBUG_PRINTF("alarm = %d clear = %d alarm_temp = %d clear_temp = %d %d %d\r\n",alarm,clear,alarm_temp,clear_temp,TranslateOnuReportValue(code,alarm_temp),TranslateOnuReportValue(code,clear_temp));
    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            management_object.leaf = CTC_MANAGEMENT_OBJECT_LEAF_PON_IF;
            management_object_index = &management_object.index;
        	management_object_index->frame_number = 0;
        	management_object_index->port_number = 0;
        	management_object_index->slot_number = 0;
        	management_object_index->port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE;
            /*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
            ret = OnuMgt_SetAlarmThreshold(olt_id, onu_id, &management_object, code, alarm_temp, clear_temp);
        }
    }    
    CONFIG_DEBUG_PRINTF("test_final:ret = %d\r\n",ret);
    return ret;
}

int CTCONU_ConfAlarmThrehold(short int olt_id, short int onu_id, CTC_STACK_alarm_id_t alarm_id, ULONG alarm, ULONG clear)
{
    int ret = VOS_ERROR;
    long alarm_temp, clear_temp;
    switch(alarm_id)
    {
#if 0	/* modified by xieshl 20161009, 避免重复配置，解决现场810H不注册问题 */
        case BATTERY_VOLT_LOW:
        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
            ret = sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, alarm_id, alarm, clear);
            break;
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM:   
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_recv_oppower_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_HIGH_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_recv_oppower_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_LOW_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_trans_oppower_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_HIGH_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_trans_oppower_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_LOW_ALARM, alarm_temp, clear_temp);
            break;
        case TX_BIAS_HIGH_ALARM: 
        case TX_BIAS_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_cur_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_cur_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_HIGH_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_pon_cur_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_cur_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_LOW_ALARM, alarm_temp, clear_temp);
            break;
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM: 
            alarm_temp = getOpticalPowerThreshold(field_pon_vol_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_vol_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_HIGH_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_pon_vol_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_vol_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_LOW_ALARM, alarm_temp, clear_temp);
            break;
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_tempe_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_tempe_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_HIGH_ALARM, alarm_temp, clear_temp);

            alarm_temp = getOpticalPowerThreshold(field_pon_tempe_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_tempe_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_LOW_ALARM, alarm_temp, clear_temp);
            break;
        default:
            ret = VOS_ERROR;
#else
        case BATTERY_VOLT_LOW:
        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
            ret = sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, alarm_id, alarm, clear);
            break;
        case RX_POWER_HIGH_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_recv_oppower_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case RX_POWER_LOW_ALARM:   
            alarm_temp = getOpticalPowerThreshold(field_recv_oppower_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case TX_POWER_HIGH_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_trans_oppower_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case TX_POWER_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_trans_oppower_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_power_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            break;
        case TX_BIAS_HIGH_ALARM: 
            alarm_temp = getOpticalPowerThreshold(field_pon_cur_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_cur_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case TX_BIAS_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_cur_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_cur_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            break;
        case VCC_HIGH_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_vol_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_vol_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case VCC_LOW_ALARM: 
            alarm_temp = getOpticalPowerThreshold(field_pon_vol_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_vol_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            break;
        case TEMP_HIGH_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_tempe_high, 0);
            clear_temp = alarm_temp - getOpticalPowerDeadZone(field_tempe_dead_zone);
            ret = sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
			break;
        case TEMP_LOW_ALARM:
            alarm_temp = getOpticalPowerThreshold(field_pon_tempe_low, 0);
            clear_temp = alarm_temp + getOpticalPowerDeadZone(field_tempe_dead_zone);
            ret |= sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, alarm_id, alarm_temp, clear_temp);
            break;
        default:
            ret = VOS_ERROR;
#endif
    }
    return ret;
}

int CTCONU_ConfAlarmData(CTC_STACK_alarm_id_t alarm_id, int flag, int enable, ULONG alarm, ULONG clear)
{
    int i = 0,j = 0;
    int type;
    int ret = VOS_ERROR;
    ULONG status = 0;
    int move_flag = getCtcAlarmflagByAlarmId(alarm_id);
    for(j=0;j<MAXPON;j++)
    {
        /*added by luh 2013-1-23*/
        if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && GetCardIdxByPonChip(j) != SYS_LOCAL_MODULE_SLOTNO)
            continue; 
        
        for(i=0;i<MAXONUPERPON;i++)
        {
            if(GetOnuOperStatus(j, i) == ONU_OPER_STATUS_UP)
            {
                if(GetOnuType(j, i, &type) == ROK && type == V2R1_ONU_CTC)
                {
                    if(flag)
                        ret = CTCONU_ConfAlarmThrehold(j, i, alarm_id, alarm, clear);
                    else
                    {
                        if(enable == CTC_ALARM_DISABLE &&move_flag != VOS_ERROR &&move_flag != VOS_OK)
                        {
                            status = GetOnuAlarmStatus(j,i)&(~move_flag);
                            SetOnuAlarmStatus(j,i,status);
                        }
                        ret = CTCONU_ConfAlarmStatus(j,i,alarm_id,enable);
                    }
                }
            }
        }

    }
    return ret;
}
/*ONU 注册恢复数据 add by luh 2011-9-18*/
int CTCONU_RecoverAlarmData(short int olt_id, short int onu_id)
{
    int type = 0;
    ULONG status = 0;
    int alarm_id = 0;
    int i = 0;
    int slot = GetCardIdxByPonChip( olt_id );
	int port = GetPonPortByPonChip( olt_id );
	int devIdx = MAKEDEVID(slot,port,onu_id+1);

    if(GetOnuOperStatus(olt_id, onu_id) != ONU_OPER_STATUS_UP)
        return VOS_OK;
    if(GetOnuType(olt_id, onu_id, &type) == ROK && type != V2R1_ONU_CTC)
        return VOS_OK;
    /*CTC-ONU 告警上报标识复位，并上报相应的告警清除消息*/
    status = GetOnuAlarmStatus(olt_id,onu_id);
    for(i=0;i<CTC_ALARM_MAX_STATE;i++)
    {
        if(status & (1<<i))
        {
            alarm_id = getCtcAlarmIdByFlag(1<<i);
            if(alarm_id != ETH_PORT_LOS)
            {
                CTC_Onu_AllReportAPI(alarm_id, CLEAR_REPORT_ALARM, devIdx, 0, 0, 0);
            }
        }
    }
    SetOnuAlarmStatus(olt_id,onu_id,0);
    CONFIG_DEBUG_PRINTF("OnuMgmtTable[%d].ctcAlarmstatus = %d\r\n",olt_id * MAXONUPERPON + onu_id,OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].ctcAlarmstatus);
    /*CCT-ONU 告警配置使能、门限的恢复*/
    if(CTCOnuAlarmConfig_onu.EquipmentAlarm)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, EQUIPMENT_ALARM, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_onu.PowerAlarm)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, POWERING_ALARM, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_onu.BatteryAlarm)
    {
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_MISSING, CTC_ALARM_ENABLE);   
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_FAILURE, CTC_ALARM_ENABLE);   
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, BATTERY_VOLT_LOW, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, BATTERY_VOLT_LOW, CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr, CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr);
    }
    if(CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, PHYSICAL_INTRUSION_ALARM, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_onu.OnuSelfTestFailure)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ONU_SELF_TEST_FAILURE, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_onu.OnuTempAlarm)
    {
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ONU_TEMP_HIGH_ALARM, CTC_ALARM_ENABLE);
        sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, ONU_TEMP_HIGH_ALARM, CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr, CTCOnuAlarmConfig_onu.OnuTempHighClearThr);

        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ONU_TEMP_LOW_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetOnuAlarmThrehold(olt_id, onu_id, ONU_TEMP_LOW_ALARM, CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr, CTCOnuAlarmConfig_onu.OnuTempLowClearThr);
    }
    if(CTCOnuAlarmConfig_onu.IadConnectFailure)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, IAD_CONNECTION_FAILURE, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_onu.PonSwitch)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, PON_IF_SWITCH, CTC_ALARM_ENABLE);  
    /*CTC-ONU 光模块告警使能、门限恢复*/
    if(GetOnuOpticalPowerEnable() == V2R1_ENABLE)
    {
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, RX_POWER_HIGH_ALARM, CTC_ALARM_ENABLE);
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_HIGH_ALARM, eponOpticalPowerThresholds.recvOPhigh, eponOpticalPowerThresholds.recvOPhigh - eponOpticalPowerDeadZone.powerVarDeadZone);
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, RX_POWER_LOW_ALARM, CTC_ALARM_ENABLE);   
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, RX_POWER_LOW_ALARM, eponOpticalPowerThresholds.recvOPlow, eponOpticalPowerThresholds.recvOPlow + eponOpticalPowerDeadZone.powerVarDeadZone);

        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_POWER_HIGH_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_HIGH_ALARM, eponOpticalPowerThresholds.tranOPhigh, eponOpticalPowerThresholds.tranOPhigh - eponOpticalPowerDeadZone.powerVarDeadZone);
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_POWER_LOW_ALARM, CTC_ALARM_ENABLE);  
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_POWER_LOW_ALARM, eponOpticalPowerThresholds.tranOPlow, eponOpticalPowerThresholds.tranOPlow + eponOpticalPowerDeadZone.powerVarDeadZone);
        
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_BIAS_HIGH_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_HIGH_ALARM, eponOpticalPowerThresholds.ponBiasCurHigh, eponOpticalPowerThresholds.ponBiasCurHigh - eponOpticalPowerDeadZone.curVarDeadZone);
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TX_BIAS_LOW_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TX_BIAS_LOW_ALARM, eponOpticalPowerThresholds.ponBiasCurLow, eponOpticalPowerThresholds.ponBiasCurLow + eponOpticalPowerDeadZone.curVarDeadZone);

        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, VCC_HIGH_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_HIGH_ALARM, eponOpticalPowerThresholds.ponVolHigh, eponOpticalPowerThresholds.ponVolHigh - eponOpticalPowerDeadZone.volVarDeadZone);
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, VCC_LOW_ALARM, CTC_ALARM_ENABLE);
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, VCC_LOW_ALARM, eponOpticalPowerThresholds.ponVolLow, eponOpticalPowerThresholds.ponVolLow + eponOpticalPowerDeadZone.volVarDeadZone);

        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TEMP_HIGH_ALARM, CTC_ALARM_ENABLE);
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_HIGH_ALARM, eponOpticalPowerThresholds.ponTemHigh, eponOpticalPowerThresholds.ponTemHigh - eponOpticalPowerDeadZone.temVarDeadZone);
        sfun_CTCONU_SetPonAlarmStatus(olt_id, onu_id, TEMP_LOW_ALARM, CTC_ALARM_ENABLE); 
        sfun_CTCONU_SetPonAlarmThrehold(olt_id, onu_id, TEMP_LOW_ALARM, eponOpticalPowerThresholds.ponTemLow, eponOpticalPowerThresholds.ponTemLow + eponOpticalPowerDeadZone.temVarDeadZone);

    }
    /*以太网口告警使能恢复*/
    if(CTCOnuAlarmConfig_port.EthAutoNegFailure)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ETH_PORT_AUTO_NEG_FAILURE, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_port.EthLos)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ETH_PORT_LOS, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_port.EthFailure)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ETH_PORT_FAILURE, CTC_ALARM_ENABLE);   
    if(CTCOnuAlarmConfig_port.EthCongestion)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ETH_PORT_CONGESTION, CTC_ALARM_ENABLE);   
    /*if(getEthLoopEnable() == ENABLE)
        sfun_CTCONU_SetOnuAlarmStatus(olt_id, onu_id, ETH_PORT_LOOPBACK, CTC_ALARM_ENABLE);*/  

    return VOS_OK;
}


/*Pon板掉电恢复add by luh 2011-9-22*/
int  Build_recoverData_func(eventCtcSyncRecoverMsg_t * alconfig)
{
    int ret = VOS_ERROR;
    if(alconfig)
    {
        
        if(CTCOnuAlarmConfig_onu.EquipmentAlarm)
            alconfig->onudata.EquipmentAlarm = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_onu.PowerAlarm)
            alconfig->onudata.PowerAlarm = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_onu.BatteryAlarm)
        {
            alconfig->onudata.BatteryAlarm = CTC_ALARM_ENABLE;
            alconfig->onudata.BatteryVoltLowAlarmThr = CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr;
            alconfig->onudata.BatteryVoltLowClearThr = CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr;
        }
        if(CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm)
            alconfig->onudata.PhysicalIntrusionAlarm = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_onu.OnuSelfTestFailure)
            alconfig->onudata.OnuSelfTestFailure = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_onu.OnuTempAlarm)
        {
            alconfig->onudata.OnuTempAlarm = CTC_ALARM_ENABLE;
            alconfig->onudata.OnuTempHighAlarmThr = CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr;
            alconfig->onudata.OnuTempHighClearThr = CTCOnuAlarmConfig_onu.OnuTempHighClearThr;
            alconfig->onudata.OnuTempLowAlarmThr = CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr;
            alconfig->onudata.OnuTempLowClearThr = CTCOnuAlarmConfig_onu.OnuTempLowClearThr;
        }
        if(CTCOnuAlarmConfig_onu.IadConnectFailure)
            alconfig->onudata.IadConnectFailure = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_onu.PonSwitch)
            alconfig->onudata.PonSwitch = CTC_ALARM_ENABLE;
        /*CTC-ONU 光模块告警使能、门限恢复*/
        if(GetOnuOpticalPowerEnable() == V2R1_ENABLE)
        { 
            alconfig->ponthresholddata.OltMonitorEnable = CTC_ALARM_ENABLE;
            alconfig->ponthresholddata.ponBiasCurHigh = eponOpticalPowerThresholds.ponBiasCurHigh;
            alconfig->ponthresholddata.ponBiasCurLow = eponOpticalPowerThresholds.ponBiasCurLow;
            alconfig->ponthresholddata.ponTemHigh = eponOpticalPowerThresholds.ponTemHigh;
            alconfig->ponthresholddata.ponTemLow = eponOpticalPowerThresholds.ponTemLow;
            alconfig->ponthresholddata.ponVolHigh = eponOpticalPowerThresholds.ponVolHigh;
            alconfig->ponthresholddata.ponVolLow = eponOpticalPowerThresholds.ponVolLow;
            alconfig->ponthresholddata.tranOPhigh = eponOpticalPowerThresholds.tranOPhigh;
            alconfig->ponthresholddata.tranOPlow= eponOpticalPowerThresholds.tranOPlow;
            alconfig->ponthresholddata.recvOPhigh = eponOpticalPowerThresholds.recvOPhigh;
            alconfig->ponthresholddata.recvOPlow= eponOpticalPowerThresholds.recvOPlow;
            
            alconfig->pondeadzonedata.curVarDeadZone = eponOpticalPowerDeadZone.curVarDeadZone;
            alconfig->pondeadzonedata.powerVarDeadZone = eponOpticalPowerDeadZone.powerVarDeadZone;
            alconfig->pondeadzonedata.temVarDeadZone = eponOpticalPowerDeadZone.temVarDeadZone;
            alconfig->pondeadzonedata.volVarDeadZone = eponOpticalPowerDeadZone.volVarDeadZone;
        }
        /*以太网口告警使能恢复*/
        if(CTCOnuAlarmConfig_port.EthAutoNegFailure)
            alconfig->portdata.EthAutoNegFailure = CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_port.EthLos)
            alconfig->portdata.EthLos= CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_port.EthFailure)
            alconfig->portdata.EthFailure= CTC_ALARM_ENABLE;
        if(CTCOnuAlarmConfig_port.EthCongestion)
            alconfig->portdata.EthCongestion= CTC_ALARM_ENABLE;
        /*if(getEthLoopEnable() == CTC_ALARM_ENABLE)
            alconfig->portdata.EthLoopback = CTC_ALARM_ENABLE;*/
        ret = VOS_OK;
    }
    return ret;
}
int Parse_recoverData_func(eventCtcSyncRecoverMsg_t *alconfig)
{
    if(alconfig == NULL)
        return VOS_ERROR;
    if(alconfig->onudata.EquipmentAlarm)
        CTCOnuAlarmConfig_onu.EquipmentAlarm = CTC_ALARM_ENABLE;
    if(alconfig->onudata.PowerAlarm)
        CTCOnuAlarmConfig_onu.PowerAlarm = CTC_ALARM_ENABLE;   
    if(alconfig->onudata.BatteryAlarm)
    {
        CTCOnuAlarmConfig_onu.BatteryAlarm = CTC_ALARM_ENABLE;   
        CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr = alconfig->onudata.BatteryVoltLowAlarmThr;
        CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr = alconfig->onudata.BatteryVoltLowClearThr;
    }
    if(alconfig->onudata.PhysicalIntrusionAlarm)
        CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm= CTC_ALARM_ENABLE;   
    if(alconfig->onudata.OnuSelfTestFailure)
        CTCOnuAlarmConfig_onu.OnuSelfTestFailure= CTC_ALARM_ENABLE;   
    if(alconfig->onudata.OnuTempAlarm)
    {
        CTCOnuAlarmConfig_onu.OnuTempAlarm= CTC_ALARM_ENABLE;   
        CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr = alconfig->onudata.OnuTempHighAlarmThr;   
        CTCOnuAlarmConfig_onu.OnuTempHighClearThr= alconfig->onudata.OnuTempHighClearThr;   
        CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr = alconfig->onudata.OnuTempLowAlarmThr;   
        CTCOnuAlarmConfig_onu.OnuTempLowClearThr = alconfig->onudata.OnuTempLowClearThr;   
    }
    if(alconfig->onudata.IadConnectFailure)
        CTCOnuAlarmConfig_onu.IadConnectFailure= CTC_ALARM_ENABLE;   
    if(alconfig->onudata.PonSwitch)
        CTCOnuAlarmConfig_onu.PonSwitch= CTC_ALARM_ENABLE;   
    /*CTC-ONU 光模块告警使能、门限恢复*/
    if(alconfig->ponthresholddata.OltMonitorEnable)
    {
        onu_OpticalPower_Enable = V2R1_ENABLE;
        eponOpticalPowerThresholds.ponBiasCurHigh = alconfig->ponthresholddata.ponBiasCurHigh;
        eponOpticalPowerThresholds.ponBiasCurLow = alconfig->ponthresholddata.ponBiasCurLow;
        eponOpticalPowerThresholds.ponTemHigh = alconfig->ponthresholddata.ponTemHigh;
        eponOpticalPowerThresholds.ponTemLow = alconfig->ponthresholddata.ponTemLow;
        eponOpticalPowerThresholds.ponVolHigh = alconfig->ponthresholddata.ponVolHigh;
        eponOpticalPowerThresholds.ponVolLow = alconfig->ponthresholddata.ponVolLow;
        eponOpticalPowerThresholds.recvOPhigh = alconfig->ponthresholddata.recvOPhigh;
        eponOpticalPowerThresholds.recvOPlow = alconfig->ponthresholddata.recvOPlow;
        eponOpticalPowerThresholds.tranOPhigh = alconfig->ponthresholddata.tranOPhigh;
        eponOpticalPowerThresholds.tranOPlow = alconfig->ponthresholddata.tranOPlow;
        
        eponOpticalPowerDeadZone.curVarDeadZone = alconfig->pondeadzonedata.curVarDeadZone;
        eponOpticalPowerDeadZone.temVarDeadZone = alconfig->pondeadzonedata.temVarDeadZone;
        eponOpticalPowerDeadZone.volVarDeadZone = alconfig->pondeadzonedata.volVarDeadZone;
        eponOpticalPowerDeadZone.powerVarDeadZone = alconfig->pondeadzonedata.powerVarDeadZone;
    }
    /*以太网口告警使能恢复*/
    if(alconfig->portdata.EthAutoNegFailure)
        CTCOnuAlarmConfig_port.EthAutoNegFailure = CTC_ALARM_ENABLE;
    if(alconfig->portdata.EthCongestion)
        CTCOnuAlarmConfig_port.EthCongestion = CTC_ALARM_ENABLE;
    if(alconfig->portdata.EthFailure)
        CTCOnuAlarmConfig_port.EthFailure = CTC_ALARM_ENABLE;
    if(alconfig->portdata.EthLos)
        CTCOnuAlarmConfig_port.EthLos = CTC_ALARM_ENABLE;
    if(alconfig->portdata.EthLoopback)
        setOnuEthLoopEnable(CTC_ALARM_ENABLE);  
    return VOS_OK;
}
#endif

#if 1
int getCtcAlarmIdByFlag(int flag)
{
    int ret = VOS_OK;
    switch(flag)
    {
        case CTC_ALARM_STATE_EQUIPMENT:
            ret = EQUIPMENT_ALARM;
            break;
        case CTC_ALARM_STATE_POWERING_ALARM :
            ret = POWERING_ALARM;
            break;
        case CTC_ALARM_STATE_BATTERY_MISSING:
            ret = BATTERY_MISSING;
            break;
        case CTC_ALARM_STATE_BATTERY_FAILURE:
            ret = BATTERY_FAILURE;
            break;
        case CTC_ALARM_STATE_BATTERY_VOLT_LOW:
            ret = BATTERY_VOLT_LOW;
            break;
        case CTC_ALARM_STATE_PHYSICAL_INTRUSION_ALARM:
            ret = PHYSICAL_INTRUSION_ALARM;
            break;
        case CTC_ALARM_STATE_ONU_SELF_TEST_FAILURE:
            ret = ONU_SELF_TEST_FAILURE;
            break;
        case CTC_ALARM_STATE_ONU_TEMP_HIGH_ALARM:
            ret = ONU_TEMP_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_ONU_TEMP_LOW_ALARM:
            ret = ONU_TEMP_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_IAD_CONNECTION_FAILURE:
            ret = IAD_CONNECTION_FAILURE;
            break;
        case CTC_ALARM_STATE_PON_IF_SWITCH:
            ret = PON_IF_SWITCH;
            break;
        case CTC_ALARM_STATE_RX_POWER_HIGH_ALARM:
            ret = RX_POWER_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_RX_POWER_LOW_ALARM:
            ret = RX_POWER_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_TX_POWER_HIGH_ALARM:
            ret = TX_POWER_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_TX_POWER_LOW_ALARM:
            ret = TX_POWER_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_TX_BIAS_HIGH_ALARM:
            ret = TX_BIAS_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_TX_BIAS_LOW_ALARM:
            ret = TX_BIAS_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_VCC_HIGH_ALARM:
            ret = VCC_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_VCC_LOW_ALARM:
            ret = VCC_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_TEMP_HIGH_ALARM:
            ret = TEMP_HIGH_ALARM;
            break;
        case CTC_ALARM_STATE_TEMP_LOW_ALARM:
            ret = TEMP_LOW_ALARM;
            break;
        case CTC_ALARM_STATE_ETH_PORT_AUTO_NEG_FAILURE:
            ret = ETH_PORT_AUTO_NEG_FAILURE;
             break;
        case CTC_ALARM_STATE_ETH_PORT_LOS:
            ret = ETH_PORT_LOS;
            break;
        case CTC_ALARM_STATE_ETH_PORT_FAILURE:
            ret = ETH_PORT_FAILURE;
            break;
        case CTC_ALARM_STATE_ETH_PORT_LOOPBACK: 
            ret = ETH_PORT_LOOPBACK;
            break;
        case CTC_ALARM_STATE_ETH_PORT_CONGESTION:
            ret = ETH_PORT_CONGESTION;
            break;
        default:
            ret = VOS_ERROR;
            break;      
    }
    return ret;
}

long getCtcAlarmflagByAlarmId(CTC_STACK_alarm_id_t code)
{
    long ret = VOS_OK;
    switch(code)
    {
        case EQUIPMENT_ALARM:
            ret = CTC_ALARM_STATE_EQUIPMENT;
            break;
        case POWERING_ALARM:
            ret = CTC_ALARM_STATE_POWERING_ALARM;
            break;
        case BATTERY_MISSING:
            ret = CTC_ALARM_STATE_BATTERY_MISSING;
            break;
        case BATTERY_FAILURE:
            ret = CTC_ALARM_STATE_BATTERY_FAILURE;
            break;
        case BATTERY_VOLT_LOW:
            ret = CTC_ALARM_STATE_BATTERY_VOLT_LOW;
            break;
        case PHYSICAL_INTRUSION_ALARM:
            ret = CTC_ALARM_STATE_PHYSICAL_INTRUSION_ALARM;
            break;
        case ONU_SELF_TEST_FAILURE:
            ret = CTC_ALARM_STATE_ONU_SELF_TEST_FAILURE;
            break;
        case ONU_TEMP_HIGH_ALARM:
            ret = CTC_ALARM_STATE_ONU_TEMP_HIGH_ALARM;
            break;
        case ONU_TEMP_LOW_ALARM:
            ret = CTC_ALARM_STATE_ONU_TEMP_LOW_ALARM;
            break;
        case IAD_CONNECTION_FAILURE:
            ret = CTC_ALARM_STATE_IAD_CONNECTION_FAILURE;
            break;
        case PON_IF_SWITCH:
            ret = CTC_ALARM_STATE_PON_IF_SWITCH;
            break;
        case RX_POWER_HIGH_ALARM:
            ret = CTC_ALARM_STATE_RX_POWER_HIGH_ALARM;
            break;
        case RX_POWER_LOW_ALARM:
            ret = CTC_ALARM_STATE_RX_POWER_LOW_ALARM;
            break;
        case TX_POWER_HIGH_ALARM:
            ret = CTC_ALARM_STATE_TX_POWER_HIGH_ALARM;
            break;
        case TX_POWER_LOW_ALARM:
            ret = CTC_ALARM_STATE_TX_POWER_LOW_ALARM;
            break;
        case TX_BIAS_HIGH_ALARM:
            ret = CTC_ALARM_STATE_TX_BIAS_HIGH_ALARM;
            break;
        case TX_BIAS_LOW_ALARM:
            ret = CTC_ALARM_STATE_TX_BIAS_LOW_ALARM;
            break;
        case VCC_HIGH_ALARM:
            ret = CTC_ALARM_STATE_VCC_HIGH_ALARM;
            break;
        case VCC_LOW_ALARM:
            ret = CTC_ALARM_STATE_VCC_LOW_ALARM;
            break;
        case TEMP_HIGH_ALARM:
            ret = CTC_ALARM_STATE_TEMP_HIGH_ALARM;
            break;
        case TEMP_LOW_ALARM:
            ret = CTC_ALARM_STATE_TEMP_LOW_ALARM;
            break;
        case ETH_PORT_AUTO_NEG_FAILURE:
        case ETH_PORT_LOS:
        case ETH_PORT_FAILURE:
        case ETH_PORT_LOOPBACK: 
        case ETH_PORT_CONGESTION:
            break;
        default:
            ret = VOS_ERROR;
            break;      
    }
    return ret;
}
int getCtcAlarmEnable(CTC_STACK_alarm_id_t code, int *enable)
{
    int ret = VOS_OK;
    switch(code)
    {
        case EQUIPMENT_ALARM:
        case POWERING_ALARM:
        case BATTERY_MISSING:
        case BATTERY_FAILURE:
        case BATTERY_VOLT_LOW:
        case PHYSICAL_INTRUSION_ALARM:
        case ONU_SELF_TEST_FAILURE:
        case ONU_TEMP_HIGH_ALARM:
        case ONU_TEMP_LOW_ALARM:
        case IAD_CONNECTION_FAILURE:
        case PON_IF_SWITCH:
        case ETH_PORT_AUTO_NEG_FAILURE:
        case ETH_PORT_LOS:
        case ETH_PORT_FAILURE:
        case ETH_PORT_CONGESTION:
            ret = CTCONU_GetAlarmEnable(code,enable);
            break;
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM:
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
        case TX_BIAS_HIGH_ALARM:
        case TX_BIAS_LOW_ALARM:
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM:
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            *enable = GetOnuOpticalPowerEnable() == V2R1_ENABLE?CTC_ALARM_ENABLE:CTC_ALARM_DISABLE;
            break;
        /*case ETH_PORT_LOOPBACK: 
            *enable = getEthLoopEnable()?CTC_ALARM_ENABLE:CTC_ALARM_DISABLE;
            break;*/
        default:
            ret = VOS_ERROR;
            break;      
    }
    return ret;
}

int CTC_Onu_AllReportAPI(CTC_STACK_alarm_id_t code, CTC_STACK_alarm_state_t state, ULONG devIdx, ULONG brdIdx, ULONG portIdx, long value)
{
    int ret = VOS_OK;
    short int PonPortIdx = GetPonPortIdxBySlot(GET_PONSLOT(devIdx), GET_PONPORT(devIdx));
    short int OnuIdx = GET_ONUID(devIdx)-1;
    switch(code)
    {
        case EQUIPMENT_ALARM:
            if(state == REPORT_ALARM)
                ctcOnuEquipmentAlarm_EventReport(devIdx);
            else
                ctcOnuEquipmentAlarmClear_EventReport(devIdx);
            break;
        case POWERING_ALARM:
            if(state == REPORT_ALARM)
                devPowerOff_EventReport(devIdx);
            else
                devPowerOn_EventReport(devIdx);
            break;
        case BATTERY_MISSING:
            if(state == REPORT_ALARM)
                ctcOnuBatteryMissing_EventReport(devIdx);
            else
                ctcOnuBatteryMissingClear_EventReport(devIdx);
            break;
        case BATTERY_FAILURE:
            if(state == REPORT_ALARM)
                ctcOnuBatteryFailure_EventReport(devIdx);
            else
                ctcOnuBatteryFailureClear_EventReport(devIdx);
            break;
        case BATTERY_VOLT_LOW:
            if(state == REPORT_ALARM)
                ctcOnuBatteryVoltLow_EventReport(devIdx, value);
            else
                ctcOnuBatteryVoltLowClear_EventReport(devIdx, value);
            break;
        case PHYSICAL_INTRUSION_ALARM:
            if(state == REPORT_ALARM)
                ctcOnuPhysicalIntrusionAlarm_EventReport(devIdx);
            else
                ctcOnuPhysicalIntrusionAlarmClear_EventReport(devIdx);
            break;
        case SELF_TEST_FAILURE:
            if(state == REPORT_ALARM)
                ctcOnuSelfTestFailure_EventReport(devIdx);
            else
                ctcOnuSelfTestFailureClear_EventReport(devIdx);
            break;
        case ONU_TEMP_HIGH_ALARM:
            if(state == REPORT_ALARM)
                ctcOnuTempHigh_EventReport(devIdx, value);
            else
                ctcOnuTempHighClear_EventReport(devIdx, value);
            break;
        case ONU_TEMP_LOW_ALARM:
            if(state == REPORT_ALARM)
                ctcOnuTempLow_EventReport(devIdx, value);
            else
                ctcOnuTempLowClear_EventReport(devIdx, value);
            break;
        case IAD_CONNECTION_FAILURE:
            if(state == REPORT_ALARM)
                ctcOnuIADConnectionFailure_EventReport(devIdx);
            else
                ctcOnuIADConnectionFailureClear_EventReport(devIdx);
            break;
        case PON_IF_SWITCH:
            if(state == REPORT_ALARM)
                ctcOnuPonIfSwitch_EventReport(devIdx);
            else
                ctcOnuPonIfSwitchClear_EventReport(devIdx);
            break;
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM:
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
        case TX_BIAS_HIGH_ALARM:
        case TX_BIAS_LOW_ALARM:
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM:
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            if(state == REPORT_ALARM)
                onuOpticalParaAlm_EventReport(code , 1, devIdx, brdIdx, portIdx, value);
            else
                onuOpticalParaAlm_EventReport(code , 0, devIdx, brdIdx, portIdx, value);
            break;
            /*modified by duzhk 2011-11-23
            暂且先注释掉，另外下次放开时须注意onuOpticalParaAlm_EventReport函数使用的有问题*/
        case ETH_PORT_AUTO_NEG_FAILURE:
            if(state == REPORT_ALARM)
                ethAutoNegFailure_EventReport(devIdx ,brdIdx, portIdx);
            else
                ethAutoNegFailureClear_EventReport(devIdx ,brdIdx, portIdx);
            break;
        case ETH_PORT_LOS:
            if(state == REPORT_ALARM)
            {
                if(Is811cOnu(PonPortIdx, OnuIdx)==VOS_OK
                    ||Is831cOnu(PonPortIdx, OnuIdx)==VOS_OK)
                    ethLinkdown_EventReport(devIdx, brdIdx, portIdx );  
                else
                    ethLinkup_EventReport(devIdx ,brdIdx, portIdx);
            }
            else
            {
                if(Is811cOnu(PonPortIdx, OnuIdx)==VOS_OK
                    ||Is831cOnu(PonPortIdx, OnuIdx)==VOS_OK)
                    ethLinkup_EventReport(devIdx ,brdIdx, portIdx);
                else
                    ethLinkdown_EventReport(devIdx, brdIdx, portIdx );                      
            }
            break;
        case ETH_PORT_FAILURE:
            if(state == REPORT_ALARM)
                ethFailure_EventReport(devIdx ,brdIdx, portIdx);
            else
                ethFailureClear_EventReport(devIdx ,brdIdx, portIdx);
            break;
        case ETH_PORT_LOOPBACK: 
            if(state == REPORT_ALARM) 
                CTCONU_ethloopport(devIdx ,brdIdx, portIdx,ethPortStatus_loopAlarm);
            else
                CTCONU_ethloopport(devIdx ,brdIdx, portIdx,ethPortStatus_loopClear);
            break;
        case ETH_PORT_CONGESTION:
            if(state == REPORT_ALARM)
                ethCongestion_EventReport(devIdx ,brdIdx, portIdx);
            else
                ethCongestionClear_EventReport(devIdx ,brdIdx, portIdx);
            break;
        default:
            ret = VOS_ERROR;
            break;      
    }
    return ret;
}
long TranslateOnuReportValue(CTC_STACK_alarm_id_t code,long value)
{
    long ret = VOS_OK;
    switch(code)
    {
        case RX_POWER_HIGH_ALARM:
        case RX_POWER_LOW_ALARM: 
        case TX_POWER_HIGH_ALARM:
        case TX_POWER_LOW_ALARM:
            ret = TranslateOpticalPower(value);
            break;
        case TX_BIAS_HIGH_ALARM:
        case TX_BIAS_LOW_ALARM:
            ret = TranslateBiasCurrent(value);
            break;
        case BATTERY_VOLT_LOW:
        case VCC_HIGH_ALARM:
        case VCC_LOW_ALARM:
            ret = TranslateWorkVoltage(value);
            break;
        case ONU_TEMP_HIGH_ALARM: 
        case ONU_TEMP_LOW_ALARM:
        case TEMP_HIGH_ALARM:
        case TEMP_LOW_ALARM:
            ret = TranslateTemperature(value);
            break;
        default:
            break;
    }
    return ret;
}

int ctc30_alarm_debug = 0;  /*CTC3.0告警翻转调试开关*/

int CTCONU_AlarmHandler(short int olt_id, short int llid, CTC_STACK_event_value_t *event)
{
	ULONG devIdx = 0;
    int slot = 0, port = 0;
	int brdIdx = 0, portIdx = 0;
    long monitorValue = 0;
    long alarmflag = 0;
    ULONG status = 0;
    short int PonPortIdx = 0;
    short int OnuIdx = 0;
    unsigned char version = 0;
 
	PonPortIdx = olt_id;
	OnuIdx = GetOnuIdxByLlid(PonPortIdx, llid);
	OnuGen_Get_CtcVersion( olt_id, OnuIdx, &version);

	/*b-解决onu协商成c1也告警也翻转，问题单40011.added by zhaoxh*/
	if(0x30 == version && 1 == ctc30_alarm_debug)
	{
		event->alarm_state = ! event->alarm_state;
	}
    /*e-解决onu协商成c1也告警也翻转，问题单40011*/

	if( OnuIdx == RERROR )	/* modified by xieshl 20111202, 如果娶不到LLID，说明ONU已不存在了，应直接返回，否则会造成后续一串断言，问题单14034 */
		return VOS_OK;

    slot = GetCardIdxByPonChip( PonPortIdx );
	port = GetPonPortByPonChip( PonPortIdx );
	devIdx = MAKEDEVID(slot,port,OnuIdx+1);
    REPORT_DEBUG_PRINTF("slot = %d  port = %d devid = %d\r\n",slot,port,devIdx);
	brdIdx = event->management_object.index.slot_number;
    if(brdIdx == 0)   /*问题单13942*/
        brdIdx += 1;

	portIdx = event->management_object.index.port_number;
    alarmflag = getCtcAlarmflagByAlarmId(event->alarm_id);
    monitorValue = TranslateOnuReportValue(event->alarm_id,event->alarm_info.alarm_info_long);
    status = GetOnuAlarmStatus(PonPortIdx,OnuIdx);

    if(event->alarm_state == REPORT_ALARM)
    {
        REPORT_DEBUG_PRINTF("test:Alarm!!!\r\n");
        REPORT_DEBUG_PRINTF("test:report %0x success!!! Pon_id = %d Onu_id = %d brdIdx = %d PortIdx = %d\r\n",event->alarm_id,PonPortIdx,OnuIdx,brdIdx,portIdx);
        if((status & alarmflag) == 0 ||alarmflag == VOS_OK)
        {
            REPORT_DEBUG_PRINTF("Value = %d or %d\r\n",monitorValue,event->alarm_info.alarm_info_long);
            CTC_Onu_AllReportAPI(event->alarm_id, REPORT_ALARM, devIdx, brdIdx, portIdx, monitorValue);
            status |= alarmflag; 
            SetOnuAlarmStatus(PonPortIdx,OnuIdx,status);
        }
    }
    else if(event->alarm_state == CLEAR_REPORT_ALARM)
    {
        REPORT_DEBUG_PRINTF("test:Alarm Clear!!!\r\n");
        REPORT_DEBUG_PRINTF("test:report %0x success!!! Pon_id = %d Onu_id = %d brdIdx = %d PortIdx = %d\r\n",event->alarm_id,PonPortIdx,OnuIdx,brdIdx,portIdx);
        if(status & alarmflag ||alarmflag == VOS_OK)
        {
            REPORT_DEBUG_PRINTF("Value = %d or %d\r\n",monitorValue,event->alarm_info.alarm_info_long);
            CTC_Onu_AllReportAPI(event->alarm_id, CLEAR_REPORT_ALARM, devIdx, brdIdx, portIdx, monitorValue);
            status &= (~ alarmflag);
            SetOnuAlarmStatus(PonPortIdx,OnuIdx,status);
        }
    }
            
    return VOS_OK;
}

int CTC_eventMsgSend( short int olt_id, short int llid, CTC_STACK_event_value_t *event )
{
	int rc = VOS_ERROR;
	ULONG ulMsg[4] = {MODULE_EVENT, FC_EVENT_CFG_CTC_EVENT_HANDLE, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen, dataLen;

	if( event == NULL )
		return rc;

    dataLen = sizeof(CTC_STACK_event_value_t);
    msgLen = sizeof(SYS_MSG_S) + dataLen;
	pstMsg = (SYS_MSG_S *)VOS_Malloc( msgLen, MODULE_EVENT ); 
	if( pstMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  
	pstMsg->ptrMsgBody = (VOID *)(pstMsg + 1);
	pstMsg->usFrameLen = dataLen;
	pstMsg->usMsgCode = FC_EVENT_CFG_CTC_EVENT_HANDLE;

	VOS_MemCpy( pstMsg->ptrMsgBody, event, dataLen );

	ulMsg[3] = (ULONG)pstMsg;
	ulMsg[2] = (ULONG)((olt_id << 16) | (llid & 0xFFFF));  
	rc = VOS_QueSend( ctcEventQueId, ulMsg, NO_WAIT, MSG_PRI_NORMAL);
	if( rc != VOS_OK )
	{
		VOS_Free((void*)pstMsg);
	}
	return rc;	
}


int CTC_STACK_event_notification_handler(const PON_olt_id_t   olt_id, 
					                                const PON_onu_id_t  onu_id, 
					                                const CTC_STACK_event_value_t   event_value )
{
	if( EVENT_ALARM == V2R1_ENABLE)
	{
		sys_console_printf("CTC_EVENT:llid(%d/%d), alarm_id %d, alarm_state %d\r\n", olt_id, onu_id
			, event_value.alarm_id, event_value.alarm_state );
	}

	if( !OLT_LOCAL_ISVALID(olt_id) )
	{
		VOS_ASSERT(0);
		return ( RERROR );
	}
    
	if( !LLID_ISVALID(onu_id) )
	{
		sys_console_printf(" Error LLID for onu ctc-event\r\n");
		return( RERROR );
	}

    if ( 0 == ctcEventQueId )
    {
		sys_console_printf(" Not Start CTC Event Report Service\r\n");
		return( RERROR );
    }

    return CTC_eventMsgSend(olt_id, onu_id, &event_value);
}
#endif



DEFUN(alarm_onu_enable_config,
	alarm_onu_enable_config_cmd,
	"alarm-onu [equipment|power|intrusion|selftest|iadconnect|ponswitch|battery|temp] {[enable|disable]}*1",
	"Config onu alarm enable\n"
	"equipment alarm\n"
	"power alarm\n"
	"physical intrusion alarm\n"
	"onu self test failure alarm\n"
	"onu IAD ConnectionFailure\n"
	"pon switch\n"
	"battery missing,failure or voltage exceed threshold alarm\n"
	"onu temperature high or low alarm\n"
	"Enable\n"
    "Disable\n"
	)
{
    CTC_STACK_alarm_id_t code;
    int enable = 0;
	if( VOS_StriCmp(argv[0], "equipment") == 0 )
		code = EQUIPMENT_ALARM;
	else if( VOS_StriCmp(argv[0], "power") == 0 )
		code = POWERING_ALARM;
	else if( VOS_StriCmp(argv[0], "intrusion") == 0 )
		code = PHYSICAL_INTRUSION_ALARM;
	else if( VOS_StriCmp(argv[0], "selftest") == 0 )
		code = ONU_SELF_TEST_FAILURE;
	else if( VOS_StriCmp(argv[0], "iadconnect") == 0 )
		code = IAD_CONNECTION_FAILURE;
	else if( VOS_StriCmp(argv[0], "ponswitch") == 0 )
		code = PON_IF_SWITCH;
	else if( VOS_StriCmp(argv[0], "battery") == 0 )
        code = BATTERY_MISSING;
	else if( VOS_StriCmp(argv[0], "temp") == 0 )
	    code = ONU_TEMP_HIGH_ALARM;
	if(argc ==2)
	{
        int enable_temp = 0;
        enable = VOS_StriCmp( argv[1], "enable")==0?CTC_ALARM_ENABLE:CTC_ALARM_DISABLE;
        if(CTCONU_GetAlarmEnable(code, &enable_temp) == VOS_OK && enable_temp == enable)
        {
            vty_out(vty, "The %s alarm has been %s!\r\n\r\n",argv[0],argv[1]);
		    return CMD_SUCCESS;
        }
        else
        {
            if(CTCONU_SetAlarmEnable(code, enable) == ERROR)
                vty_out(vty, "The %s alarm enable set fail!\r\n",argv[0]);
        }
	}
    else
    {
        if(CTCONU_GetAlarmEnable(code, &enable) == VOS_OK)
            vty_out(vty, "The %s alarm is %s\r\n",argv[0],enable?"enable":"disable");
        else
            vty_out(vty, "The %s alarm Configuration get ERROR!\r\n",argv[0]);
    }
	return  CMD_SUCCESS;
}

DEFUN(alarm_Port_enable_config,
	alarm_Port_enable_config_cmd,
	"alarm-ethport [autoneg|los|failure|congestion] {[enable|disable]}*1",
	"config onu eth-port alarm enable\n"
	"ethernet port autoneg failure\n"
	"ethernet port LOS\n"
	"ethernet port failure\n"
	"ethernet port congestion\n"
	"Enable\n"
    "Disable\n"
	)
{
    CTC_STACK_alarm_id_t code;
    int enable = 0; 
    
	if(VOS_StriCmp(argv[0], "autoneg") == 0 )
        code = ETH_PORT_AUTO_NEG_FAILURE;
    else if(VOS_StriCmp(argv[0], "los")==0)
        code = ETH_PORT_LOS;
	else if(VOS_StriCmp(argv[0], "failure")==0)
        code = ETH_PORT_FAILURE;
	else if(VOS_StriCmp(argv[0], "congestion")==0)
        code = ETH_PORT_CONGESTION;
    if(argc ==2)
	{
        int enable_temp = 0;
        enable = VOS_StrCmp( argv[1], "enable")==0?CTC_ALARM_ENABLE:CTC_ALARM_DISABLE;

        if(CTCONU_GetAlarmEnable(code, &enable_temp) == VOS_OK && enable_temp == enable)
        {
            vty_out(vty, "The ethernet port %s enable has been %s!\r\n\r\n",argv[0],argv[1]);
		    return CMD_SUCCESS;
        }
        else
        {
            if(CTCONU_SetAlarmEnable(code, enable) == ERROR)
                vty_out(vty, "The ethernet port %s enable set fail!\r\n",argv[0]);
        }
	}
    else
    {
        if(CTCONU_GetAlarmEnable(code, &enable) == VOS_OK)
            vty_out(vty, "The ethernet port %s alarm is %s\r\n",argv[0],enable?"enable":"disable");
        else
            vty_out(vty, "The ethernet port %s alarm Configuration get ERROR!\r\n",argv[0]);
    }
	return  CMD_SUCCESS;
}

DEFUN(alarm_onu_battery_threshold_config,
	alarm_onu_battery_threshold_config_cmd,
    "alarm-onu batterylow <alarm_threshold> <clear_threshold>",
    "config onu alarm threshold\n"
	"battery voltage exceed threshold alarm\n"
	"input alarm threshold 0~65.5(100mv)\n"
	"input alarm clear threshold 0~65.5(100mv)\n"
	)
{
    CTC_STACK_alarm_id_t code;
    LONG threshold = 0;
    LONG clearthreshold = 0;
    
    threshold = VOS_AtoL(argv[0]);
    clearthreshold = VOS_AtoL(argv[1]);
    code = BATTERY_VOLT_LOW;
    
    CHECK_ALARM_VCC(threshold);
    CHECK_ALARM_VCC(clearthreshold);
    
    if(threshold > clearthreshold)
    {
        vty_out(vty, "The battery low threshold needs to be less than its clear threshold!\r\n");
        return CMD_WARNING;
    }
    if(CTCONU_SetAlarmThreshold(code, threshold, clearthreshold) == VOS_ERROR)
        vty_out(vty, "The batterylow threshold set fail!\r\n");

	return  CMD_SUCCESS;
}

DEFUN(alarm_onu_temperature_threshold_config,
	alarm_onu_temperature_threshold_config_cmd,
    "alarm-onu [temphigh|templow] <alarm_threshold> <clear_threshold>",
    "config onu alarm threshold\n"
	"temperature high alarm\n"
	"temperature low alarm\n"
	"input alarm threshold -128~128 (℃)\n"
	"input alarm clear threshold -128~128 (℃)\n"
	)
{
    CTC_STACK_alarm_id_t code;
    LONG threshold = 0, threshold_temp = 0;
    LONG clearthreshold = 0, clearthreshold_temp = 0;
    
    threshold = VOS_AtoL(argv[1]);
    clearthreshold = VOS_AtoL(argv[2]);

    CHECK_ALARM_TEMP(threshold);
    CHECK_ALARM_TEMP(clearthreshold);
    
    if( VOS_StriCmp(argv[0], "temphigh") == 0 )
	{
        code = ONU_TEMP_HIGH_ALARM;
        if(CTCONU_GetAlarmThreshold(ONU_TEMP_LOW_ALARM, &threshold_temp, &clearthreshold_temp) == VOS_OK)
        {
            if(threshold < threshold_temp)
            {
                vty_out(vty, "The temperature high threshold needs to be more than the low threshold!\r\n");
                return CMD_WARNING;
            }
        }
        if(threshold < clearthreshold)
        {
            vty_out(vty, "The temperature alarm threshold needs to be more than its clear threshold!\r\n");
            return CMD_WARNING;
        }     
	}
	else if( VOS_StriCmp(argv[0], "templow") == 0 )
	{
        code = ONU_TEMP_LOW_ALARM;	
        if(CTCONU_GetAlarmThreshold(ONU_TEMP_HIGH_ALARM, &threshold_temp, &clearthreshold_temp) == VOS_OK)
        {
            if(threshold > threshold_temp)
            {
                vty_out(vty, "The temperature low threshold needs to be less than the high threshold!\r\n");
                return CMD_WARNING;
            }
        }
        if(threshold > clearthreshold)
        {
            vty_out(vty, "The temperature alarm threshold needs to be less than its clear threshold!\r\n");
            return CMD_WARNING;
        }     
	}
    
    if(CTCONU_SetAlarmThreshold(code, threshold, clearthreshold) == VOS_ERROR)
          vty_out(vty, "The %s threshold set fail!\r\n",argv[0]);

	return  CMD_SUCCESS;
}

DEFUN(alarm_onu_threshold_show,
	alarm_onu_threshold_show_cmd,
    "show alarm-onu [batterylow|temphigh|templow]",
    "Show running system information\n"
    "onu alarm\n"
	"battery voltage exceed threshold alarm\n"
	"temperature high alarm\n"
	"temperature low alarm\n"
	)
{
    CTC_STACK_alarm_id_t code;
    ULONG threshold = 0;
    ULONG clearthreshold = 0;
    int enable = 0;
	if( VOS_StriCmp(argv[0], "batterylow") == 0 )
        code = BATTERY_VOLT_LOW;
	else if( VOS_StriCmp(argv[0], "temphigh") == 0 )
        code = ONU_TEMP_HIGH_ALARM;
	else if( VOS_StriCmp(argv[0], "templow") == 0 )
        code = ONU_TEMP_LOW_ALARM;	
    
    if(CTCONU_GetAlarmThreshold(code, &threshold, &clearthreshold) == VOS_ERROR ||
        CTCONU_GetAlarmEnable(code, &enable) == VOS_ERROR)
        vty_out(vty, "The %s threshold get fail!\r\n",argv[0]);
    else
    {
        vty_out(vty, "\r\nThe %s alarm Configuration\r\n",argv[0]);
        vty_out(vty, "%30s : %s\r\n","alarm_enable", enable?"enable":"disable");
        if(code == BATTERY_VOLT_LOW)
        {
            vty_out(vty, "%30s : %d.%d V\r\n","alarm_threshold", decimal2_integer_part(threshold), decimal2_fraction_part(threshold));
            vty_out(vty, "%30s : %d.%d V\r\n\r\n","clear_threshold", decimal2_integer_part(clearthreshold), decimal2_fraction_part(clearthreshold));
        }
        else
        {
            vty_out(vty, "%30s : %d\r\n","alarm_threshold", threshold);
            vty_out(vty, "%30s : %d\r\n\r\n","clear_threshold", clearthreshold);
        }
    }
	return  CMD_SUCCESS;
}
DEFUN(alarm_onu_config_show,
	alarm_onu_config_show_cmd,
    "show alarm-onu [onu|ponif|port]",
    "Show running system information\n"
    "all onu alarm\n"
	"onu alarm\n"
	"pon_if alarm\n"
	"eth port alarm\n"
	)
{
    int enable = 0;
    long threshold = 0;
    long clearthreshold = 0;
    int flag = 0;
    char *outputstr[4] = {"NULL", "Onu", "Onu Pon_If", "Onu Etherport"};
	if( VOS_StriCmp(argv[0], "onu") == 0 )
        flag = ONU_Alarm_FLAG;
	else if( VOS_StriCmp(argv[0], "ponif") == 0 )
        flag = PON_Alarm_FLAG;
	else if( VOS_StriCmp(argv[0], "port") == 0 )
        flag = ETHPORT_Alarm_FLAG;	
    vty_out(vty, "\r\nThe %s alarm Configuration:\r\n",outputstr[flag]);
    vty_out(vty, "-------------------------------------------------------------------\r\n");
    switch(flag)
    {
        case ONU_Alarm_FLAG:
            vty_out(vty, "%-24s %-8s %-16s %-16s\r\n","Alarm Type","Status","Alarm_threshold","Clear_threshold");
            if(CTCONU_GetAlarmEnable(EQUIPMENT_ALARM,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","equipment", enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(POWERING_ALARM,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","power", enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(BATTERY_MISSING,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","battery missing",enable? "Enable":"Disable");
            if(CTCONU_GetAlarmEnable(BATTERY_FAILURE,&enable)==VOS_OK)
                vty_out(vty,  "%-24s %-8s\r\n","battery failure", enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(BATTERY_VOLT_LOW,&enable)==VOS_OK)
            {
                if(CTCONU_GetAlarmThreshold(BATTERY_VOLT_LOW, &threshold, &clearthreshold) == VOS_OK)
                    vty_out(vty, "%-24s %-8s %10d.%-1d V %10d.%-1d V\r\n","battery_voltage low", enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));
            }
            if(CTCONU_GetAlarmEnable(PHYSICAL_INTRUSION_ALARM,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","physical intrusion",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(ONU_SELF_TEST_FAILURE,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","self test failure",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(ONU_TEMP_HIGH_ALARM,&enable)==VOS_OK)
            {
                if(CTCONU_GetAlarmThreshold(ONU_TEMP_HIGH_ALARM, &threshold, &clearthreshold) == VOS_OK)
                    vty_out(vty, "%-24s %-8s %12d   %12d    \r\n","temperature high",enable?"Enable":"Disable",threshold,clearthreshold);
            }
            if(CTCONU_GetAlarmEnable(ONU_TEMP_LOW_ALARM,&enable)==VOS_OK)
            {
                if(CTCONU_GetAlarmThreshold(ONU_TEMP_LOW_ALARM, &threshold, &clearthreshold) == VOS_OK)
                    vty_out(vty, "%-24s %-8s %12d   %12d    \r\n","temperature low",enable?"Enable":"Disable",threshold,clearthreshold);
            }
            if(CTCONU_GetAlarmEnable(IAD_CONNECTION_FAILURE,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","IAD connection failure",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(PON_IF_SWITCH,&enable)==VOS_OK)
                vty_out(vty, "%-24s %-8s\r\n","PON_IF switch",enable?"Enable":"Disable");
            break;
         case PON_Alarm_FLAG:
            vty_out(vty, "%-24s %-8s %-16s %-16s\r\n","Alarm Type","Status","Alarm_threshold","Clear_threshold");
            enable = GetOnuOpticalPowerEnable() == V2R1_ENABLE?ENABLE:DISABLE;
            threshold = getOpticalPowerThreshold(field_recv_oppower_high, 0);
            clearthreshold = threshold - getOpticalPowerDeadZone(field_power_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d dBm %8d.%-1d dBm\r\n","RX power high", enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));

            threshold = getOpticalPowerThreshold(field_recv_oppower_low, 0);
            clearthreshold = threshold + getOpticalPowerDeadZone(field_power_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d dBm %8d.%-1d dBm\r\n","RX power low", enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));

            threshold = getOpticalPowerThreshold(field_trans_oppower_high, 0);
            clearthreshold = threshold - getOpticalPowerDeadZone(field_power_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d dBm %8d.%-1d dBm\r\n","TX power high",enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));

            threshold = getOpticalPowerThreshold(field_trans_oppower_low, 0);
            clearthreshold = threshold + getOpticalPowerDeadZone(field_power_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d dBm %8d.%-1d dBm\r\n","TX power low",enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));

            threshold = getOpticalPowerThreshold(field_pon_cur_high, 0);
            clearthreshold = threshold - getOpticalPowerDeadZone(field_cur_dead_zone);
            vty_out(vty, "%-24s %-8s %12d mA%12d mA\r\n","TX bias high",enable?"Enable":"Disable",threshold,clearthreshold);
 
            threshold = getOpticalPowerThreshold(field_pon_cur_low, 0);
            clearthreshold = threshold + getOpticalPowerDeadZone(field_cur_dead_zone);
            vty_out(vty, "%-24s %-8s %12d mA%12d mA\r\n","TX bias low",enable?"Enable":"Disable",threshold,clearthreshold);

            threshold = getOpticalPowerThreshold(field_pon_vol_high, 0);
            clearthreshold = threshold - getOpticalPowerDeadZone(field_vol_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d V %10d.%-1d V\r\n","voltage high",enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));
            
            threshold = getOpticalPowerThreshold(field_pon_vol_low, 0);
            clearthreshold = threshold + getOpticalPowerDeadZone(field_vol_dead_zone);
            vty_out(vty, "%-24s %-8s %10d.%-1d V %10d.%-1d V\r\n","voltage low",enable?"Enable":"Disable",decimal2_integer_part(threshold),decimal2_fraction_part(threshold),decimal2_integer_part(clearthreshold),decimal2_fraction_part(clearthreshold));

            threshold = getOpticalPowerThreshold(field_pon_tempe_high,0);
            clearthreshold = threshold - getOpticalPowerDeadZone(field_tempe_dead_zone);
            vty_out(vty, "%-24s %-8s %12d   %12d    \r\n","temperature high",enable?"Enable":"Disable",threshold,clearthreshold);

            threshold = getOpticalPowerThreshold(field_pon_tempe_low, 0);
            clearthreshold = threshold + getOpticalPowerDeadZone(field_tempe_dead_zone);
            vty_out(vty, "%-24s %-8s %12d   %12d    \r\n","temperature low",enable?"Enable":"Disable",threshold,clearthreshold);
            break;
         case ETHPORT_Alarm_FLAG:
            vty_out(vty, "%-28s %-8s \r\n","Alarm Type","Status");
            if(CTCONU_GetAlarmEnable(ETH_PORT_AUTO_NEG_FAILURE,&enable)==VOS_OK)
                vty_out(vty, "%-28s %-8s\r\n","ethport auto neg failure",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(ETH_PORT_LOS,&enable)==VOS_OK)
                vty_out(vty, "%-28s %-8s\r\n","eth port los",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(ETH_PORT_FAILURE,&enable)==VOS_OK)
                vty_out(vty, "%-28s %-8s\r\n","ethport failure",enable?"Enable":"Disable");
            enable = getEthLoopEnable();
                vty_out(vty, "%-28s %-8s\r\n","ethport loopback",enable?"Enable":"Disable");
            if(CTCONU_GetAlarmEnable(ETH_PORT_CONGESTION,&enable)==VOS_OK)
                vty_out(vty, "%-28s %-8s\r\n","ethport congestion",enable?"Enable":"Disable");
            break;
         default:
            break;
    }
    vty_out(vty, "-------------------------------------------------------------------\r\n\r\n");
	return  CMD_SUCCESS;
}

DEFUN(alarm_onu_debug_enable,
	alarm_onu_debug_enable_cmd,
    "debug onu-alarm [config|report] {[enable|disable]}*1",
    "debug enable\n"
    "onu alarm\n"
    "config information\n"
    "report information\n"
	"Enable\n"
    "Disable\n"	
    )
{
    int flag = 0;
    int enable = 0;
    
    flag = VOS_StrCmp( argv[0], "config")==0?Configflag:Reportflag;
    if(argc ==2)
    {
        enable = VOS_StrCmp( argv[1], "enable")==0?CTC_ALARM_ENABLE:CTC_ALARM_DISABLE;
        if(SetDebugEnable(flag, enable) != VOS_OK)
            vty_out(vty, "The %s Debug Enable set fail!\r\n",argv[0]);
    }
    else
    {
        if(getDebugEnable(flag, &enable) != VOS_OK)
            vty_out(vty, "The %s Debug Enable get error!\r\n",argv[0]);
        else
            vty_out(vty, "The %s Debug Configuration is %s!\r\n",argv[0],enable?"Enable":"Disable");
    }
	return  CMD_SUCCESS;
}

LONG OnuEventCtc_init()
{
    int rc = VOS_ERROR;
	if( g_CtcOnuEventSemId == 0 )
		g_CtcOnuEventSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);
    
	if( ctcEventQueId == 0 ) 
		ctcEventQueId = VOS_QueCreate( CTC_EVENT_QUEUE_LENGTH , VOS_MSG_Q_PRIORITY);
	if( ctcEventQueId  == 0 )
	{
		VOS_ASSERT( 0 );
		return rc;
	}

    ctcEventTaskId = ( VOS_HANDLE )VOS_TaskCreate("tctcEvent", TASK_PRIORITY_NORMAL, ctcEventProcTask, NULL );
	if( ctcEventTaskId == NULL )
	{
		VOS_ASSERT( 0 );
		return rc;
	}

	VOS_QueBindTask( ctcEventTaskId, ctcEventQueId );

    if( VOS_OK != CDP_Create(RPU_TID_CDP_EVENT_CTC, CDP_NOTI_VIA_QUEUE, ctcEventQueId, NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

    onuAlarmStatusInit();
	return VOS_OK;
}

void ctcEventProcTask()
{
    ULONG ulRcvMsg[4];
	long result;
	SYS_MSG_S *pMsg;
    eventCtcSyncCfgMsg_t *syncCfgMsg;
    CtcEventCfgData_t *ctcEventCfgData;
    CTC_STACK_event_value_t *ctcEventData;
    
	while( 1 )
	{
		result = VOS_QueReceive( ctcEventQueId, ulRcvMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_TaskDelay(100);
			ASSERT(0);
			continue;
		}

		pMsg = (SYS_MSG_S *)ulRcvMsg[3];
        
		if( NULL == pMsg )
		{
			VOS_ASSERT(pMsg);
			VOS_TaskDelay(20);
			continue;
		}
        
        if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pMsg ); 	 
			continue;
		}
        
		switch( pMsg->usMsgCode )
		{
            case FC_EVENT_CFG_SYNC_CTC:
                syncCfgMsg = (eventCtcSyncCfgMsg_t*)(pMsg+1);
                eventSync_SetAlarmData(syncCfgMsg);
                break;
            case FC_EVENT_CFG_PON_TO_ONU:
                ctcEventCfgData = (CtcEventCfgData_t*)(pMsg+1);
                CTCONU_ConfAlarmData(ctcEventCfgData->alarm_id,ctcEventCfgData->flag,0,0,0);
                break;                
            case FC_EVENT_CFG_CTC_EVENT_HANDLE:
                ctcEventData = (CTC_STACK_event_value_t*)(pMsg+1);
                CTCONU_AlarmHandler((short int)(ulRcvMsg[2] >> 16),(short int)(ulRcvMsg[2] & 0xFFFF),ctcEventData);
                break;                
            default:               
                break;
                                
		}
		if(SYS_MSG_SRC_SLOT(pMsg) != (SYS_LOCAL_MODULE_SLOTNO))
		{
			CDP_FreeMsg( pMsg );  
		}
		else
		{
			VOS_Free(pMsg); 
		}
		pMsg = NULL;
	}
}

LONG  CTC_AlarmCmd_Init()
{    
    install_element ( DEBUG_HIDDEN_NODE, &alarm_onu_debug_enable_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_enable_config_cmd);
	install_element ( CONFIG_NODE, &alarm_Port_enable_config_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_battery_threshold_config_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_temperature_threshold_config_cmd);    
    install_element ( CONFIG_NODE, &alarm_onu_threshold_show_cmd);
    install_element ( CONFIG_NODE, &alarm_onu_config_show_cmd);
    return VOS_OK;
}

/*初始化时，告警使能置0，告警上报标志清零2011-9-22 */
int onuAlarmStatusInit()
{
    CTCOnuAlarmConfig_onu.BatteryAlarm = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.BatteryVoltLowAlarmThr = 0;
    CTCOnuAlarmConfig_onu.BatteryVoltLowClearThr = 0;
    CTCOnuAlarmConfig_onu.EquipmentAlarm = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.IadConnectFailure = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.OnuSelfTestFailure = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.OnuTempAlarm = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.OnuTempHighAlarmThr = 128;
    CTCOnuAlarmConfig_onu.OnuTempHighClearThr = 128;
    CTCOnuAlarmConfig_onu.OnuTempLowAlarmThr = -128;
    CTCOnuAlarmConfig_onu.OnuTempLowClearThr = -128;
    CTCOnuAlarmConfig_onu.PhysicalIntrusionAlarm = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.PonSwitch = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_onu.PowerAlarm = CTC_ALARM_DISABLE;
    
    CTCOnuAlarmConfig_port.EthAutoNegFailure = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_port.EthCongestion = CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_port.EthFailure= CTC_ALARM_DISABLE;
    CTCOnuAlarmConfig_port.EthLos = CTC_ALARM_DISABLE;
	return VOS_OK;
}

LONG ctcOnu_alarm_showrun( struct vty * vty )
{
    int alarm_enable = 0;
    int i = 0;
    LONG alarmThreshold=0,alarmClearThreshold=0;

    vty_out(vty,"!ctc onu alarm config\r\n");     
    for(i=0x0001;i<=0x000c;i++)
    {
        if((getCtcAlarmEnable(i,&alarm_enable) != VOS_OK )|| (alarm_enable != CTC_ALARM_ENABLE))
            continue;
        switch(i)
        {
            case  EQUIPMENT_ALARM:
                vty_out(vty,"  alarm-onu equipment enable\r\n");
                break;                                                       
            case  POWERING_ALARM:
                vty_out(vty,"  alarm-onu power enable\r\n");
                break;
            case  BATTERY_MISSING:
            case  BATTERY_FAILURE:
            case  BATTERY_VOLT_LOW:
                vty_out(vty,"  alarm-onu battery enable\r\n"/*, alarmEnable[alarm_enable]*/);
                if(CTCONU_GetAlarmThreshold(i,&alarmThreshold,&alarmClearThreshold) == VOS_OK)
                {
                    if((alarmThreshold>MIN_ALARM_VCC)&&(alarmThreshold<MAX_ALARM_VCC)&&(alarmClearThreshold>MIN_ALARM_VCC)&&(alarmClearThreshold<MAX_ALARM_VCC)) 
                    {
                        vty_out(vty,"  alarm-onu batterylow %d %d\r\n",alarmThreshold,alarmClearThreshold);  
                    }
                }
                break;
            case  PHYSICAL_INTRUSION_ALARM:
                vty_out(vty,"  alarm-onu intrusion enable\r\n");
                break;
            case ONU_SELF_TEST_FAILURE:
                vty_out(vty,"  alarm-onu selftest enable\r\n");  
                break;
            case  ONU_TEMP_HIGH_ALARM:
                if(CTCONU_GetAlarmThreshold(i,&alarmThreshold,&alarmClearThreshold)==VOS_OK)
                {
                    if((alarmThreshold>MIN_ALARM_TEMP)&&(alarmThreshold<MAX_ALARM_TEMP)&&(alarmClearThreshold>MIN_ALARM_TEMP)&&(alarmClearThreshold<MAX_ALARM_TEMP)) 
                    {
                        vty_out(vty,"  alarm-onu temp enable\r\n"/*, alarmEnable[alarm_enable]*/);
                        vty_out(vty,"  alarm-onu temphigh %d %d\r\n",alarmThreshold,alarmClearThreshold);
                    }
                }
                break;
            case ONU_TEMP_LOW_ALARM:
                if(CTCONU_GetAlarmThreshold(i,&alarmThreshold,&alarmClearThreshold)==VOS_OK)
                {
                    if(((alarmThreshold>MIN_ALARM_TEMP)&&(alarmThreshold<MAX_ALARM_TEMP))&&((alarmClearThreshold>MIN_ALARM_TEMP)&&(alarmClearThreshold<MAX_ALARM_TEMP))) 
                    {
                        vty_out(vty,"  alarm-onu temp enable\r\n"/*, alarmEnable[alarm_enable]*/);
                        vty_out(vty,"  alarm-onu templow %d %d\r\n",alarmThreshold,alarmClearThreshold);
                    }
                }
                break;
            case IAD_CONNECTION_FAILURE:
                vty_out(vty,"  alarm-onu iadconnect enable\r\n");
                break;
            case  PON_IF_SWITCH:
                vty_out(vty,"  alarm-onu ponswitch enable\r\n");
                break;
            default:
                break;
        }     
    }

    for(i=0x0301;i<=0x0305;i++)
    {
        if((getCtcAlarmEnable(i,&alarm_enable) != VOS_OK )|| (alarm_enable != CTC_ALARM_ENABLE))
            continue;
        switch(i)
        {
            case ETH_PORT_AUTO_NEG_FAILURE:
                vty_out(vty,"  alarm-ethport autoneg enable\r\n");
                break;
            case ETH_PORT_LOS:
                vty_out(vty,"  alarm-ethport los enable\r\n");
                break;
            case ETH_PORT_FAILURE:
                vty_out(vty,"  alarm-ethport failure enable\r\n");
                break;
              /* 环路告警的showrun 维持原状;
                         * case ETH_PORT_LOOPBACK:
                         * vty_out(vty,"loop-detection enable\r\n");
                         * break;*/
            case ETH_PORT_CONGESTION:
                vty_out(vty,"  alarm-ethport congestion enable\r\n");
                break;
            default:
                break;
        }
    } 
    /*光功率告警的showrun 维持原状*/
    #if 0    
    if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
    {
        vty_out( vty, "!\r\n\r\n" );
        return VOS_OK;
    }
    vty_out(vty,"optical-power enable\r\n");  /*光功率有可能是不使能*/
        {
                Rxopticallowthreshold=getOpticalPowerThreshold(field_recv_oppower_low);                               
                Rxopticalhighthreshold=getOpticalPowerThreshold(field_recv_oppower_high);
                Txopticallowthreshold=getOpticalPowerThreshold(field_trans_oppower_low);
                Txopticalhighthreshold=getOpticalPowerThreshold(field_trans_oppower_high);
                if((Rxopticallowthreshold<MAX_ALARM_POWER)&&(Rxopticallowthreshold>MIN_ALARM_POWER)&&(Rxopticalhighthreshold<MAX_ALARM_POWER)&&(Rxopticalhighthreshold>MIN_ALARM_POWER)
                    &&(Txopticallowthreshold<MAX_ALARM_POWER)&&(Txopticallowthreshold>MIN_ALARM_POWER)&&(Txopticalhighthreshold<MAX_ALARM_POWER)&&(Txopticalhighthreshold>MIN_ALARM_POWER))
                    vty_out(vty,"optical-power alarm-threshold onu %d %d %d %d\r\n",Txopticalhighthreshold,Txopticallowthreshold,Rxopticalhighthreshold,Rxopticallowthreshold);
              
                biasLow=getOpticalPowerThreshold(field_pon_cur_low);
                BiasHigh=getOpticalPowerThreshold(field_pon_cur_high);
                if((BiasHigh<MAX_ALARM_BIAS)&&(BiasHigh>MIN_ALARM_BIAS)&&(biasLow<MAX_ALARM_BIAS)&&(biasLow>MIN_ALARM_BIAS))
                    vty_out(vty,"optical-current onu %d %d\r\n",BiasHigh,biasLow);
             
                VccLow=getOpticalPowerThreshold(field_pon_vol_low);
                VccHigh=getOpticalPowerThreshold(field_pon_vol_high);
                if((VccHigh<MAX_ALARM_VCC)&&(VccHigh>MIN_ALARM_VCC)&&(VccLow<MAX_ALARM_BIAS)&&(VccLow>MIN_ALARM_VCC))
                    vty_out(vty,"optical-voltage onu %d %d\r\n",VccHigh,VccLow);
               
                PonTempLow=getOpticalPowerThreshold(field_pon_tempe_low);
                PonTempHigh=getOpticalPowerThreshold(field_pon_tempe_high);
                if((PonTempHigh<MAX_ALARM_TEMP)&&(PonTempHigh>MIN_ALARM_TEMP)&&(PonTempLow<MAX_ALARM_TEMP)&&(PonTempLow>MIN_ALARM_TEMP))
                    vty_out(vty,"optical-current onu %d %d\r\n",PonTempHigh,PonTempLow);   
        }
    #endif
    vty_out( vty, "!\r\n\r\n" );
    return VOS_OK;
}
