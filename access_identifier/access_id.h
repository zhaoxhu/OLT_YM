/*--------------------------------------------------------------------*\
 *  File name:  access_iden.h 
 *  Author:     wugang
 *  Version:    V1.0
 *  Date:       2009-06-04
 *  Description: 定义或声明ONU线路接入的相关宏、结构体、函数原型等
\*--------------------------------------------------------------------*/

#include "vos_typevx.h"
#include "vos_types.h"
#include "cli/cli.h"

#define RELAY_TYPE_PPPOE 1
#define RELAY_TYPE_DHCP 2

#define ACCESS_ID_MODULE_FREE 0
#define ACCESS_ID_MODULE_BUSY 1

#define PPPOE_RELAY_GWD_PRIVATE_MODE 1
#define PPPOE_RELAY_DSL_RORUM_MODE 2
#define PPPOE_RELAY_FPT_PRIVATE_MODE 3

#define PPPOE_RELAY_PARAMID_STRHEAD 0

#define PPPOE_RELAY_DISABLE 0
#define PPPOE_RELAY_ENABLE 1

#define DHCP_RELAY_CTC_MODE 1
#define DHCP_RELAY_STD_MODE 2

#define DHCP_RELAY_DISABLE 0
#define DHCP_RELAY_ENABLE 1

#define ACCESS_ID_OAM 8/*用户线路接入标识*/

#define MAX_RETRY_COUNT 3/*允许底层重复发送报文的次数*/

#define ONU_ACCESS_ID_SUCCESS 1/*ONU对OAM报文返回值为成功*/
#define ONU_ACCESS_ID_FAIL    2/*ONU对OAM报文返回值为失败*/

extern char *PPPOE_Relay_Maual_String_head;
extern char g_PPPOE_relay;
extern char g_PPPOE_relay_mode;
extern int PPPOE_Relay_Maual_String_Len;

extern char g_DHCP_relay;
extern char g_DHCP_relay_mode;

typedef struct ACCESS_ID_oam_t
{
  char msg_type;
  char result;/*ONU返回的结果，OLT下发时为0*/
  char relay_type;/*relay种类*/
  char relay_mode;/*relay 模式,如果为0，表示relay disable*/
  unsigned char relay_str[64];
}__attribute__((packed)) ACCESS_ID_OAM_s;

STATUS PPPOE_RELAY_Oam_Pkt(ULONG PonPortIdx, ULONG OnuIdx);
STATUS DHCP_RELAY_Oam_Pkt(ULONG PonPortIdx, ULONG OnuIdx);
STATUS Onu_Access_Msg_Oam_pkt(ULONG PonPortIdx, ULONG OnuIdx, char MsgType,
        unsigned char *pBuf, int length, unsigned char *pRxBuf, short int*RxBufLen);
void ACCESS_id_CommandInstall(void);
STATUS SEND_Access_Str_Oam(ULONG PonPortIdx, ULONG Relay_type);
STATUS PON_Port_Send_Oam(ULONG Relay_type, ULONG ulArg2,
        ULONG ulArg3, ULONG ulArg4,
        ULONG ulArg5, ULONG ulArg6,
        ULONG ulArg7, ULONG ulArg8,
        ULONG ulArg9, ULONG ulArg10 );
LONG ACCESS_Id_Cli_Init(void);
STATUS onuIdx_Is_Support_Relay(ULONG PonPortIdx, ULONG OnuIdx, ULONG Relay_type);
