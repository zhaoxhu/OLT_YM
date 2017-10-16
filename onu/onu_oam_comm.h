
#define OAM_NOTI_VIA_NULL 0
#define OAM_NOTI_VIA_QUEUE 1
#define OAM_NOTI_VIA_FUNC  2

#define OAM_SESSIOM_MAX_QNUM 100
#define FC_OAM_SESSION_TIMEOUT 0x0001

#define MAX_OAM_EVENT 20
#define MAX_OAM_SESSION_NUM 200

extern int debug_oam_session_enable;
#define DEBUG_OAM_SESSION_PRINTF if(debug_oam_session_enable)sys_console_printf

typedef int ( *OAMRECIEVECALLBACK)(short int PonPortIdx, short int OnuLlid, short int len, unsigned char *pDataBuf, unsigned char  *pSessionField);

enum
{
    OAM_SYNC = 0,
    OAM_ASYNC  
};

typedef unsigned char OamSessionfield[8];

typedef struct 
{
    UCHAR ulvalid;
    UCHAR ulmode;
    UCHAR ulflag;
    UCHAR MsgType;
    ULONG TimeCount;
    
    OamSessionfield sessionid;
    void *TxBuff;
    void *RxBuff;
    short int *RxBuffLen;
    ULONG ulSynSem;
    ULONG ulQueId;
    OAMRECIEVECALLBACK func;
}Oam_Session_table_t;

typedef Oam_Session_table_t Oam_Session_tables_t[MAX_OAM_SESSION_NUM];

extern int Oam_Session_Send(
    short int PonPortIdx, 
    short int OnuIdx, 
    char  MsgType, 
    UCHAR mode, 
    ULONG ulFlag, 
    ULONG QueId, 
    OAMRECIEVECALLBACK func, 
    unsigned char *pDataBuf,
    short int len,
    unsigned char *pRecieveBuf,
    short int *pRecieveBuflen);

extern int Oam_Session_RecieveCallBack(
    short int PonPortIdx, 
    short int OnuId, 
    short int llid, 
    short int len,         
    unsigned char *pDataBuf, 
    unsigned char  *pSessionField);


