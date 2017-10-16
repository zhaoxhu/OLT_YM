
#ifndef _E1_MIB_H_
#define _E1_MIB_H_

#include "e1_apis.h"
#include "mn_e1.h"

extern const unsigned char E1MAC[6];
extern ULONG debugE1;

#define E1_ERROR_INFO      0x00000001/* 函数执行的错误信息 */
#define E1_TX_BOARD_MSG    0x00000002/* 发出的板间消息 */
#define E1_RX_BOARD_MSG    0x00000004/* 接收的板间消息 */
#define E1_TX_OAM_MSG      0x00000008/* 发出的OAM */
#define E1_RX_OAM_MSG      0x00000010/* 接收的OAM */

#define E1_ERROR_INFO_PRINT( x )  
/*#define E1_ERROR_INFO_PRINT( x )      if (debugE1 & E1_ERROR_INFO) {sys_console_printf x;}
#define E1_TX_BOARD_MSG_PRINT( x )    if (debugE1 & E1_TX_BOARD_MSG) {sys_console_printf (x);}
#define E1_RX_BOARD_MSG_PRINT( x )    if (debugE1 & E1_RX_BOARD_MSG) {sys_console_printf (x);}
#define E1_TX_OAM_MSG_PRINT( x )      if (debugE1 & E1_TX_OAM_MSG) {sys_console_printf (x);}
#define E1_RX_OAM_MSG_PRINT( x )      if (debugE1 & E1_RX_OAM_MSG) {sys_console_printf (x);}*/

/* TDM上E1最大的槽位号 */
#define MAX_TDM_E1_SLOT_ID		8
/* TDM上E1最小的槽位号 */
#define MIN_TDM_E1_SLOT_ID		4

/* TDM上E1端口最大索引号 */
#define MAX_TDM_E1_PORT_ID		23
/* TDM上E1端口最小索引号 */
#define MIN_TDM_E1_PORT_ID		0

/* ONU上E1最大的槽位号 */
#define MAX_ONU_E1_SLOT_ID		5
/* ONU上E1最小的槽位号 */
#define MIN_ONU_E1_SLOT_ID		2
/*每个ONU上E1板子最大的E1数*/
#define MAX_ONU_BOARD_E1		4



#define MIB_E1_ENABLE			    1
#define MIB_E1_DISABLE		        2
#define MIB_E1_VLAN_ENABLE			1
#define MIB_E1_VLAN_DISABLE			2

#define VLAN_DEFAULT_ID		1
#define VLAN_DEFAULT_PRI	0
#define MAX_VLAN_ID			4094
#define MAX_VLAN_PRI		7


/* bit0 bit1 */
#define TDM_E1_NO_LOOP              0xFC /* &不环回 */
#define TDM_E1_CIRCUIT_LOOP         0x01 /* &|线路环回 */
#define TDM_E1_SYSTEM_LOOP          0x02 /* &|系统环回 */
#define TDM_E1_ALL_LOOP             0x03 /* &|双向环回 */
/* bit2 bit3 */
#define ONU_E1_NO_LOOP              0xF3 /* &不环回 */
#define ONU_E1_CIRCUIT_LOOP         0x04 /* &|线路环回 */
#define ONU_E1_SYSTEM_LOOP          0x08 /* &|系统环回 */
#define ONU_E1_ALL_LOOP             0x0C /* &|双向环回 */

#define E1_TX_CLOCK_AUTO		0 /* 表示自适应恢复时钟 */
#define E1_TX_CLOCK_PICK		1 /* 表示E1接收端口提取的时钟 */
#define E1_TX_CLOCK_SPEC		2 /* 表示专用的2M时钟定时信号 */
#define E1_TX_CLOCK_CRYS		3 /* 表示本地晶振自由振荡得到的时钟 */



/* nodeId = 1 (eponE1LinkTable) */
#define LEAF_eponE1PortOnuDevId			1
#define LEAF_eponE1PortOnuE1SlotId      2
#define LEAF_eponE1PortOnuE1Id          3
#define LEAF_eponE1PortLocalEnable		4
#define LEAF_eponE1Description          5

/* nodeId = 2 (eponE1PortTable_t) */
#define LEAF_eponE1PortAlarmStatus		1
#define LEAF_eponE1PortAlarmMask		2
#define LEAF_eponE1PortLoop             3
#define LEAF_eponE1PortTxClock			4

/* nodeId = 3 (eponE1VlanTable_t) */
#define LEAF_eponE1VlanEnable	1
#define LEAF_eponE1VlanPri		2
#define LEAF_eponE1VlanId		3


typedef struct
{
	unsigned short  eponE1PortAlarmStatus;	/* (R) */
	unsigned short  eponE1PortAlarmMask;		/* (RW) */
	unsigned char  eponE1Loop;              /* (RW) */
	unsigned char  eponE1TxClock;           /* (RW) */
} __attribute__((packed)) e1PortTable_t;

extern e1PortTable_t tdmE1PortTable[];
#define e1_Alarm_status(e1) (tdmE1PortTable[e1-1].eponE1PortAlarmStatus)
#define e1_Alarm_Mask(e1) (tdmE1PortTable[e1-1].eponE1PortAlarmMask)

/* 操作SW上保存的TDM E1 PORT表 和 ONU E1 PORT表 */
extern STATUS sw_e1PortTable_get( ULONG *idxs, e1PortTable_t *pE1PortTable );
extern STATUS sw_e1PortTable_getNext( ULONG *idxs, e1PortTable_t *pE1PortTable );
extern STATUS sw_e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval );
extern STATUS sw_e1PortTable_rowset( ULONG  *idxs,
								     USHORT eponE1PortAlarmMask,
								     UCHAR  eponE1Loop,
								     UCHAR  eponE1TxClock );

/* 设置SW上的E1端口表，TDM上的E1端口表，ONU上的E1端口 */
extern STATUS e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval );
extern STATUS e1PortTable_rowset( ULONG  *idxs,
								  USHORT eponE1PortAlarmMask,
							      UCHAR  eponE1Loop,
			                      UCHAR  eponE1TxClock );


extern STATUS onuDevIdxIsRight(ULONG onuDevId);
extern STATUS onuDevIdxIsSupportE1(ULONG onuDevId);
extern STATUS checkOnuE1IsRight(ULONG onuDevId, UCHAR onuE1SlotId, UCHAR onuE1Id/* 0-3 */);
extern STATUS checkOnuE1IsSupportE1(ULONG onuDevId, UCHAR onuE1SlotId, UCHAR onuE1Id);
extern STATUS check_OnuE1Index_IsSupportE1(OnuE1Index *pOnuE1Index);

/* TDM侧告警写入接口 */
extern STATUS tdm_e1_alarm_set(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType);
extern STATUS tdm_e1_alarm_clear(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType);

/* 取得一个fpga下链接的所有ONU设备索引 */
extern STATUS getOneFpgaOnuDevIdx(UCHAR fpgaIdx/*0-2*/, UCHAR *onuCnt, ULONG *onuDevIdx);

/* 其它模块函数的声明 */
extern BOOL onuIsOnLine( ULONG onuIdx );
extern short int parseOnuIndexFromDevIdx( const ULONG devIdx, ULONG * ponIdx, ULONG *onuIdx );

extern STATUS e1PortTable_setLoopBack(ULONG *idxs, USHORT setval);
extern BOOL checkLoopBack(ULONG *idxs, USHORT setval);
extern void initSwE1Data(void);/* 没调用 */

#endif


