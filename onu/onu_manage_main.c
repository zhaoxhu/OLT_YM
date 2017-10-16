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
#include  "Cdp_pub.h"
#include  "Onu_manage.h"

extern unsigned long g_Onu_Queue_Id;
extern LONG  g_Onu_Task_Id;
extern unsigned int Onu_Task_flag;
void Oam_Session_Init();

struct Onu_EventClient_S g_ctc_stack_Onu_Client[ ] =
        {
          /*----------------------------------------------------------------------------------*/
          /* int			            Next int		         Input						Action	            */
          /*----------------------------------------------------------------------------------*/
          { ONU_IDLE,       ONU_DISCOVERY,    FC_ONU_REGISTER,              OnuEvent_OK},
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_REGISTER_TIMEOUT,      OnuEvent_IDLE_TimeOutHandler},
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_REGISTER_INPROCESS,    OnuEvent_ERROR},
          { ONU_DISCOVERY,  ONU_IN_PROGRESS,  FC_EXTOAMDISCOVERY,           OnuEvent_Register_Discovery},
          { ONU_DISCOVERY,  ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_DISCOVERY,  ONU_IDLE,         FC_ONU_REGISTER_TIMEOUT,      OnuEvent_DISCOVERY_TimeOutHandler},
          { ONU_IN_PROGRESS,ONU_FINISH,       FC_ONU_REGISTER_INPROCESS,    OnuEvent_Register_Finish},
          { ONU_IN_PROGRESS,ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_IN_PROGRESS,ONU_IDLE,         FC_ONU_REGISTER_TIMEOUT,      OnuEvent_IN_PROGRESS_TimeOutHandler},
          { ONU_FINISH,     ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_FINISH,     ONU_FINISH,       FC_ONU_REGISTER_TIMEOUT,      OnuEvent_FINISH_TimeOutHandler}
          /*{ ONU_FINISH,     ONU_FINISH,       FC_ONU_REGISTER_SETBANDWIDTH, OnuEvent_SetBandWidth}          */
        } ;
    
struct Onu_EventClient_S g_Onu_Client[ ] =
        {
          /*----------------------------------------------------------------------------------*/
          /* int			            Next int		         Input						Action	            */
          /*----------------------------------------------------------------------------------*/
          { ONU_IDLE,       ONU_IN_PROGRESS,  FC_ONU_REGISTER,              OnuEvent_Register_GetEQUInfo},
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_REGISTER_TIMEOUT,      OnuEvent_IDLE_TimeOutHandler},            
          { ONU_IDLE,       ONU_IDLE,         FC_ONU_REGISTER_INPROCESS,    OnuEvent_ERROR},            
          { ONU_IN_PROGRESS,ONU_FINISH,       FC_ONU_REGISTER_INPROCESS,    OnuEvent_Register_Finish},
          { ONU_IN_PROGRESS,ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_IN_PROGRESS,ONU_IDLE,         FC_ONU_REGISTER_TIMEOUT,      OnuEvent_IN_PROGRESS_TimeOutHandler},            
          { ONU_FINISH,     ONU_IDLE,         FC_ONU_DEREGISTER,            OnuEvent_Deregister},
          { ONU_FINISH,     ONU_FINISH,       FC_ONU_REGISTER_TIMEOUT,      OnuEvent_FINISH_TimeOutHandler}
          /*{ ONU_FINISH,     ONU_FINISH,       FC_ONU_REGISTER_SETBANDWIDTH, OnuEvent_SetBandWidth}                      */
        } ;

#define CTC_ONU_CLIENT_NUMBER	( sizeof(g_ctc_stack_Onu_Client) / sizeof(g_ctc_stack_Onu_Client[0]) )	/*状态机数量*/
#define ONU_CLIENT_NUMBER	( sizeof(g_Onu_Client) / sizeof(g_Onu_Client[0]) )	/*状态机数量*/

static int OnuClientFunctionCallback(ULONG aulMsg[4])
{
    int ret = VOS_OK;
    int init_status = 0;   
    int i = 0;   
    int action_code = 0;
    OnuClientData_s OnuClientData;
    int action_do = 0;

    VOS_MemZero((VOID*)&OnuClientData, sizeof(OnuClientData_s) );
    ret = parse_onu_event_data(aulMsg, &OnuClientData);
    if(ret != VOS_OK)
        return ret;

    if( !OLT_LOCAL_ISVALID(OnuClientData.PonPortIdx) )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    if( !ONU_IDX_ISVALID(OnuClientData.OnuIdx) ) 
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }  
    if( !LLID_IDX_ISVALID(OnuClientData.LLIDIdx) ) 
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }  
    
    init_status = OnuEvent_Get_RegisterStatus(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx); 
    action_code = aulMsg[1];
	ONU_REGISTER_DEBUG("\r\ninit:%d, act:0x%x \r\n",init_status, action_code);
    if(V2R1_CTC_STACK)
    {
        for(i = 0;i< CTC_ONU_CLIENT_NUMBER; i++)
        {
            if(init_status == g_ctc_stack_Onu_Client[i].ucFsmState && action_code == g_ctc_stack_Onu_Client[i].usMsgCode)
            {
                action_do = 1;
                if((*(g_ctc_stack_Onu_Client[i].OnuEventAction))(OnuClientData) == VOS_OK)
                    OnuEvent_Set_RegisterStatus(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx, g_ctc_stack_Onu_Client[i].ucFsmNext);
                else
                {
                    OnuEvent_Clear_RegisterTimer(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx);                          
                    OnuEvent_Set_RegisterStatus(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx, ONU_IDLE);
                }
            }
        }
        if(!action_do)
        {
            char *pdata = (char *)OnuClientData.DataBuffer;
            if(pdata != NULL)
            {
                VOS_Free(pdata);
            }        
        }
    }
    else
    {
        for(i = 0;i< ONU_CLIENT_NUMBER; i++)
        {
            if(init_status == g_Onu_Client[i].ucFsmState && action_code == g_Onu_Client[i].usMsgCode)
            {
                action_do = 1;                
                if((*(g_Onu_Client[i].OnuEventAction))(OnuClientData) == VOS_OK)
                    OnuEvent_Set_RegisterStatus(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx, g_Onu_Client[i].ucFsmNext);
                else
                {
                    OnuEvent_Clear_RegisterTimer(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx);                                              
                    OnuEvent_Set_RegisterStatus(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx, ONU_IDLE);
                }
            }
        }
        if(!action_do)
        {
            char *pdata = (char *)OnuClientData.DataBuffer;
            if(pdata != NULL)
            {
                VOS_Free(pdata);
            }        
        }
    }
    return ret;
}

static VOID NewOnuTask()
{
	unsigned long aulMsg[4];
	long result;
	short int PonPortIdx = 0, OnuIdx = 0;
	unsigned char MsgType;
	int length;
	unsigned char *pBuf;

	Onu_Task_flag = 1;
	while(1)
	{
		result = VOS_QueReceive( g_Onu_Queue_Id, aulMsg, WAIT_FOREVER);
		if( result == VOS_ERROR )
		{
			VOS_ASSERT(0);
			VOS_TaskDelay(20);
			continue;
		}
		if( result == VOS_NO_MSG ) continue; 
		switch(aulMsg[1])
		{
			case CDP_NOTI_FLG_RXDATA:             /*同步消息接收，add by shixh20100608*/
				recv_onuSyncMessage((VOID *)aulMsg[3]);
				break;

			case CDP_NOTI_FLG_SEND_FINISH:
				 if ( aulMsg[3] != 0)
					CDP_FreeMsg( (void*)aulMsg[3] );
				 else
				 	VOS_ASSERT(0);
				break;

			case FC_ONU_STATUS_SYNC_TIMEOUT:	/* added by xieshl 20111019, 主控和PON板间ONU状态同步 */
				OnuMgtSync_OnuStatusReport();
				break;

			case FC_V2R1_TIMEOUT:
				PON_BOARD_ENSURE;
#if( EPON_MODULE_ONU_REG_FILTER	 == EPON_MODULE_YES )
				HandlerRegisterFilterQueue();
#endif
				/*V2r1TimerHandler();*/
				break;

			case FC_ONU_EUQ_INFO:
				/*PON_BOARD_ENSURE;*/
				/* modified by xieshl 20111130, 扩展支持GT811_C读取 */
                OnuEvent_EQU_handler(aulMsg);
				break;

			case FC_ONU_TDM_SERVICE:
				PON_BOARD_ENSURE;
				OnuIdx = (short int )aulMsg[2] & 0xffff;
				PonPortIdx = (short int)(aulMsg[2] >> 16);
				pBuf = (unsigned char *)aulMsg[3];
                
				if( pBuf != NULL )
				{
					MsgType = FC_ONU_TDM_SERVICE;
					length  = *(int *) &pBuf[4];				
					EQU_SendMsgToOnu( PonPortIdx, OnuIdx, MsgType, &pBuf[8], length );
					VOS_Free((void *) pBuf );
					VOS_TaskDelay(30);
				}
				break;

			case FC_ONU_TDM_SERVICE_ALL:
				PON_BOARD_ENSURE;
				{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
    				ConfigOnuVoiceService(aulMsg[2]);
    				/*VOS_TaskDelay(30);*/
#endif
				}
				break;
                
            /*case FC_ONU_EVENT_PONCARD_RESET:
                OnuEvent_Clear_OnuRunningDataBySlot(aulMsg);                
                break;*/
                
            case FC_ONU_EVENT_PONPORT_RESET:
                OnuEvent_Clear_OnuRunningDataByPort(aulMsg);                
                break;
                
            case FC_ONU_EVENT_TIMEOUT:
                OnuEvent_TimeOutHandler();
                break;
			case FC_ONU_REGISTER:
			case FC_EXTOAMDISCOVERY:
            case FC_ONU_REGISTER_INPROCESS:
			case FC_ONU_DEREGISTER:
            case FC_ONU_REGISTER_TIMEOUT:
            case FC_ONU_REGISTER_SETBANDWIDTH:
                PON_BOARD_ENSURE;
                OnuClientFunctionCallback(aulMsg);
                break;
			default:
				break;
		}
	}
}

VOID OnuModule_init()
{
  
	sys_console_printf(" create the Onu task ...");
	g_Onu_Task_Id = VOS_TaskCreateEx( "tOnu", NewOnuTask, 63/*TASK_PRIORITY_ONU*/, 81920, NULL );
	/*g_Onu_Task_Id = VOS_TaskCreate("t_Onu", TASK_PRIORITY_ONU, OnuTask, NULL );*/
	while( Onu_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask(( VOS_HANDLE ) g_Onu_Task_Id,  g_Onu_Queue_Id );
	if( g_Onu_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");

	CDP_Create(RPU_TID_CDP_ONU, CDP_NOTI_VIA_QUEUE, g_Onu_Queue_Id, NULL);
    
    OnuEvent_init();
    OnuEvent_TimerCreate();   
    Oam_Session_Init();
    
}


#ifdef __cplusplus
}
#endif

