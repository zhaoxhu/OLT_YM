
#ifndef _E1_MIB_H_
#define _E1_MIB_H_

#include "e1_apis.h"
#include "mn_e1.h"

extern const unsigned char E1MAC[6];
extern ULONG debugE1;

#define E1_ERROR_INFO      0x00000001/* ����ִ�еĴ�����Ϣ */
#define E1_TX_BOARD_MSG    0x00000002/* �����İ����Ϣ */
#define E1_RX_BOARD_MSG    0x00000004/* ���յİ����Ϣ */
#define E1_TX_OAM_MSG      0x00000008/* ������OAM */
#define E1_RX_OAM_MSG      0x00000010/* ���յ�OAM */

#define E1_ERROR_INFO_PRINT( x )  
/*#define E1_ERROR_INFO_PRINT( x )      if (debugE1 & E1_ERROR_INFO) {sys_console_printf x;}
#define E1_TX_BOARD_MSG_PRINT( x )    if (debugE1 & E1_TX_BOARD_MSG) {sys_console_printf (x);}
#define E1_RX_BOARD_MSG_PRINT( x )    if (debugE1 & E1_RX_BOARD_MSG) {sys_console_printf (x);}
#define E1_TX_OAM_MSG_PRINT( x )      if (debugE1 & E1_TX_OAM_MSG) {sys_console_printf (x);}
#define E1_RX_OAM_MSG_PRINT( x )      if (debugE1 & E1_RX_OAM_MSG) {sys_console_printf (x);}*/

/* TDM��E1���Ĳ�λ�� */
#define MAX_TDM_E1_SLOT_ID		8
/* TDM��E1��С�Ĳ�λ�� */
#define MIN_TDM_E1_SLOT_ID		4

/* TDM��E1�˿���������� */
#define MAX_TDM_E1_PORT_ID		23
/* TDM��E1�˿���С������ */
#define MIN_TDM_E1_PORT_ID		0

/* ONU��E1���Ĳ�λ�� */
#define MAX_ONU_E1_SLOT_ID		5
/* ONU��E1��С�Ĳ�λ�� */
#define MIN_ONU_E1_SLOT_ID		2
/*ÿ��ONU��E1��������E1��*/
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
#define TDM_E1_NO_LOOP              0xFC /* &������ */
#define TDM_E1_CIRCUIT_LOOP         0x01 /* &|��·���� */
#define TDM_E1_SYSTEM_LOOP          0x02 /* &|ϵͳ���� */
#define TDM_E1_ALL_LOOP             0x03 /* &|˫�򻷻� */
/* bit2 bit3 */
#define ONU_E1_NO_LOOP              0xF3 /* &������ */
#define ONU_E1_CIRCUIT_LOOP         0x04 /* &|��·���� */
#define ONU_E1_SYSTEM_LOOP          0x08 /* &|ϵͳ���� */
#define ONU_E1_ALL_LOOP             0x0C /* &|˫�򻷻� */

#define E1_TX_CLOCK_AUTO		0 /* ��ʾ����Ӧ�ָ�ʱ�� */
#define E1_TX_CLOCK_PICK		1 /* ��ʾE1���ն˿���ȡ��ʱ�� */
#define E1_TX_CLOCK_SPEC		2 /* ��ʾר�õ�2Mʱ�Ӷ�ʱ�ź� */
#define E1_TX_CLOCK_CRYS		3 /* ��ʾ���ؾ��������񵴵õ���ʱ�� */



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

/* ����SW�ϱ����TDM E1 PORT�� �� ONU E1 PORT�� */
extern STATUS sw_e1PortTable_get( ULONG *idxs, e1PortTable_t *pE1PortTable );
extern STATUS sw_e1PortTable_getNext( ULONG *idxs, e1PortTable_t *pE1PortTable );
extern STATUS sw_e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval );
extern STATUS sw_e1PortTable_rowset( ULONG  *idxs,
								     USHORT eponE1PortAlarmMask,
								     UCHAR  eponE1Loop,
								     UCHAR  eponE1TxClock );

/* ����SW�ϵ�E1�˿ڱ�TDM�ϵ�E1�˿ڱ�ONU�ϵ�E1�˿� */
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

/* TDM��澯д��ӿ� */
extern STATUS tdm_e1_alarm_set(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType);
extern STATUS tdm_e1_alarm_clear(UCHAR fpgaIdx, UCHAR e1PortIdx, USHORT alarmType);

/* ȡ��һ��fpga�����ӵ�����ONU�豸���� */
extern STATUS getOneFpgaOnuDevIdx(UCHAR fpgaIdx/*0-2*/, UCHAR *onuCnt, ULONG *onuDevIdx);

/* ����ģ�麯�������� */
extern BOOL onuIsOnLine( ULONG onuIdx );
extern short int parseOnuIndexFromDevIdx( const ULONG devIdx, ULONG * ponIdx, ULONG *onuIdx );

extern STATUS e1PortTable_setLoopBack(ULONG *idxs, USHORT setval);
extern BOOL checkLoopBack(ULONG *idxs, USHORT setval);
extern void initSwE1Data(void);/* û���� */

#endif


