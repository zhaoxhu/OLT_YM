#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "onuConfMgt.h"
#include  "V2R1_product.h"
#include  "ponEventHandler.h"
#include  "ct_manage/CT_RMan_EthPort.h"
#include  "Onu_manage.h"

LONG OnuEvent_TimerId = 0;
#if 1
static int OnuEvent_TimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ONU_EVENT_TIMEOUT, 0, 0};

	/* modified by xieshl 20160523, 解决ONU提前注册问题，tOnu任务忙时，暂停超时处理，防止其它消息丢失 */
    if(VOS_QueNum(g_Onu_Queue_Id) > 3/*(MAXOLTMSGNUM/2)*/)
        return VOS_ERROR;
    
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
			VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  );
		}
	}
    return VOS_OK;
}
static int OnuRegister_TimerCallback(short int PonPortIdx, short int OnuIdx, short int LLIDIdx)
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ONU_REGISTER_TIMEOUT, 0, 0};
    aulMsg[2] = PonPortIdx;
    aulMsg[3] = (OnuIdx << 8) | (LLIDIdx & 0xFF);

    /*if(VOS_QueNum(g_Onu_Queue_Id)>(MAXOLTMSGNUM/2))
        return VOS_ERROR;*/
    
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
			VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  );
		}
	}
    return VOS_OK;
}
int OnuEvent_TimeOutHandler()
{
    int OnuEntry;
    short int PonPortIdx, OnuIdx, LLIDIdx, LLIDNum;    
    OnuLLIDTable_S *pLLID;
    
    for(PonPortIdx=0;PonPortIdx<MAXPON;PonPortIdx++)
    {
        OnuEntry = PonPortIdx*MAXONUPERPON;
        
        for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
        {
            ONU_MGMT_SEM_TAKE;
            if( 0 < (LLIDNum = OnuMgmtTable[OnuEntry].llidNum) )
            {
                pLLID = OnuMgmtTable[OnuEntry].LlidTable;
                for(LLIDIdx=0;LLIDIdx<MAXLLIDPERONU;LLIDIdx++)
                {
                    if ( 0 < pLLID->OnuEventRegisterTimeout )
                    {
                        if( 0 == --pLLID->OnuEventRegisterTimeout )
                        {
                            OnuRegister_TimerCallback(PonPortIdx, OnuIdx, LLIDIdx);
                        }
                    }

                    pLLID++;
                }
            }
            ONU_MGMT_SEM_GIVE;

            OnuEntry++;
        }
    }
    
    return VOS_OK;
}
#endif
#if 1
int OnuEvent_IDLE_TimeOutHandler(OnuClientData_s OnuClientData)
{
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    int llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);
    char local_mac[6]={0};
	char SN[17]={0};

    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, 17);
    ONU_MGMT_SEM_GIVE;
    
    ONU_REGISTER_DEBUG("\r\nOnuEvent_IDLE_TimeOut(%d, %d, %d), llid = %d\r\n", PonPortIdx, OnuIdx, LLIDIdx, llid);       
    if( llid == INVALID_LLID ) 
        return VOS_ERROR;   


    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		 AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_TIMEOUT);
	else
	{
		if( MAC_ADDR_IS_INVALID(local_mac))
		return VOS_ERROR;
    	AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_TIMEOUT);
	}
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);  
    
    return VOS_OK;
}
int OnuEvent_DISCOVERY_TimeOutHandler(OnuClientData_s OnuClientData)
{
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
    char local_mac[6]={0};
	char SN[17]={0};

    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, 17);
    ONU_MGMT_SEM_GIVE;
    ONU_REGISTER_DEBUG("\r\nOnuEvent_DISCOVERY_TimeOut(%d, %d, %d)", PonPortIdx, OnuIdx, LLIDIdx);  
    if( llid == INVALID_LLID ) 
        return VOS_ERROR;   

    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_TIMEOUT);
	else
	{
		if( MAC_ADDR_IS_INVALID(local_mac))
		return VOS_ERROR;
    	AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_TIMEOUT);
	}
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);  
    
    return VOS_OK;
}
int OnuEvent_IN_PROGRESS_TimeOutHandler(OnuClientData_s OnuClientData)
{
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    int flag = OnuEvent_Get_WaitOnuEUQMsgFlag(PonPortIdx, OnuIdx);
    int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
    char local_mac[6];    
	char SN[17]={0};

    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, 17);
    ONU_MGMT_SEM_GIVE;
    
    ONU_REGISTER_DEBUG("\r\nOnuEvent_IN_PROGRESS_TimeOut(%d, %d, %d)", PonPortIdx, OnuIdx, LLIDIdx);       
#if 0    
    if(flag == V2R1_ONU_EUQ_MSG_WAIT)
    {
        if( llid == INVALID_LLID ) 
            return VOS_ERROR;   

    	if( MAC_ADDR_IS_INVALID(local_mac))
    		return VOS_ERROR;
        
        AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_TIMEOUT);
    }
#else
    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_TIMEOUT);
	else
    	AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_TIMEOUT);
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);  
#endif
    return VOS_OK;
}
int OnuEvent_FINISH_TimeOutHandler(OnuClientData_s OnuClientData)
{
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);  
    ONU_REGISTER_DEBUG("\r\nOnuEvent_FINISH_TimeOut(%d, %d, %d)", PonPortIdx, OnuIdx, LLIDIdx);       
    return VOS_OK;
}
#endif
int OnuEvent_TimerCreate()
{
	OnuEvent_TimerId = VOS_TimerCreate( MODULE_OLT, 0, SECOND_1, OnuEvent_TimerCallback, NULL, VOS_TIMER_LOOP );
	return OnuEvent_TimerId;
}

#ifdef __cplusplus
}
#endif
