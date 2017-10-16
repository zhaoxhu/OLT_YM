#define ONU_REGISTER_DEBUG  if(EVENT_REGISTER == V2R1_ENABLE )sys_console_printf

typedef enum
{
    ONU_REGISTER_STATUS_IDLE = 0,
    ONU_REGISTER_STATUS_DISCOVERY,
    ONU_REGISTER_STATUS_IN_PROGRESS,
    ONU_REGISTER_STATUS_FINISH,
    ONU_REGISTER_STATUS_FAIL,
}g_Onu_register_status_t;

typedef  struct
{
	short int  PonPortIdx;
	short int  OnuIdx;
	short int  llid;
    UCHAR      onu_mac[6];
} OnuEventData_s;

typedef  struct
{
	short int  PonPortIdx;
	short int  OnuIdx;
	short int  LLIDIdx;
	short int  reverse;
	int  Data;
    int* DataBuffer;
} OnuClientData_s;

#define    ONU_IDLE                 0
#define    ONU_DISCOVERY            1
#define    ONU_GET_EQU              2
#define    ONU_IN_PROGRESS          3 
#define    ONU_FINISH               4

#define    CTC_DISCOVERY_COMPLETE   1
#define    CTC_DISCOVERY_FAIL       2

enum
{
    ONU_EVENT_CODE_MIN = 0,      /*注册完成后，回调函数标识，由Pon 板调用*/
    ONU_EVENT_CODE_REGISTER,      /*注册完成后，回调函数标识，由Pon 板调用*/
    ONU_EVENT_CODE_DEREGISTER,     /*收到去注册事件时，回调函数标识，由Pon 板调用*/
    ONU_EVENT_CODE_REMOVE,     /*Reset pon/Reboot card时，在主控上由pon模块通知Onu模块做相应处理。
                                                                  *同时，onu模块遍历event list，通知其他模块有onu离线时间发生*/
    ONU_EVENT_CODE_MAX
};

typedef int ( *g_OnuEventFuction_callback ) ( OnuEventData_s data );

typedef struct OnuEventList
{
    int code;       /*ONU_EVENT_CODE_REGISTER/ONU_EVENT_CODE_DEREGISTER/ONU_EVENT_CODE_REMOVE*/
	g_OnuEventFuction_callback func;
	struct OnuEventList *next;
}OnuEventList_t;

struct Onu_EventClient_S 
{
	UCHAR	ucFsmState;			/*当前状态*/
	UCHAR	ucFsmNext;			/*下一个状态*/
	USHORT	usMsgCode;			/*收到的消息码*/
	int	(* OnuEventAction)(OnuClientData_s);	/*该状态执行的动作*/
};

enum
{
    ONU_AUTH_FAIL = 0,
    ONU_AUTH_SUCESS
};
#if 1
extern int OnuEvent_Get_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx);
extern int OnuEvent_Set_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, int TimeOut);
extern int OnuEvent_Set_RegisterStatus(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, int status);
extern int OnuEvent_Get_RegisterStatus(short int PonPortIdx, short int OnuIdx, short int LLIDIdx);
extern int OnuEvent_Clear_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx);
extern UCHAR OnuEvent_Get_WaitOnuEUQMsgFlag(short int PonPortIdx, short int OnuIdx);
extern int OnuEvent_Set_WaitOnuEUQMsgFlag(short int PonPortIdx, short int OnuIdx, UCHAR status);
#endif
#if 1
extern int OnuEvent_OK(OnuClientData_s OnuClientData);
extern int OnuEvent_ERROR(OnuClientData_s OnuClientData);
extern int OnuEvent_Register_Discovery(OnuClientData_s OnuClientData);
extern int OnuEvent_Register_Finish(OnuClientData_s OnuClientData);
extern int OnuEvent_Register_GetEQUInfo(OnuClientData_s OnuClientData);
extern int OnuEvent_Deregister(OnuClientData_s OnuClientData);
extern int OnuEvent_IDLE_TimeOutHandler(OnuClientData_s OnuClientData);
extern int OnuEvent_DISCOVERY_TimeOutHandler(OnuClientData_s OnuClientData);
extern int OnuEvent_IN_PROGRESS_TimeOutHandler(OnuClientData_s OnuClientData);
extern int OnuEvent_FINISH_TimeOutHandler(OnuClientData_s OnuClientData);
/*extern int OnuEvent_SetBandWidth(OnuClientData_s OnuClientData);*/
#endif
extern void OnuEvent_init();
extern int OnuEvent_TimeOutHandler();
extern void OnuEvent_EQU_handler(ULONG aulMsg[4]);
extern int GetOnuEUQInfoHandle_New(short int PonPortIdx, short int OnuIdx, char *pRxOamMsg);
extern int OnuEvent_InprogressMsg_Send(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, char *pdata);
extern int EQU_SendMsgToOnu_New(short int PonPortIdx, short int OnuIdx, char  MsgType, unsigned char *pBuf, int length);
extern int OnuEvent_Clear_OnuRunningDataBySlot(ULONG aulMsg[4]);
extern int OnuEvent_Clear_OnuRunningDataByPort(ULONG aulMsg[4]);

