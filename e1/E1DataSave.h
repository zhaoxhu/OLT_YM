
#ifndef  _E1_DATA_SAVE_H_
#define  _E1_DATA_SAVE_H_


#ifndef GW_EPON_SYS_H
#include	"../../mib/gwEponSys.h"
#endif

#include "cli/cli.h"
#include "e1_apis.h"
#include "E1_MIB.h"
#include "onu/ExtBoardType.h"

#ifndef	PACKED
#define	PACKED	__attribute__((packed))
#endif


#define E1_NODE ATM_SUBIF_NODE


/* E1������flash�е����洢�ռ� */
#define E1_CFG_FILE_LEN	( sizeof(nvm_e1NvmDataHead_t) + sizeof(nvm_e1LinkTable_t) + sizeof(nvm_e1PortTable_t) + sizeof(nvm_e1VlanTable_t) + sizeof(nvm_sw_onu_e1PortTable_t) )	


typedef struct {
	UCHAR		nvmFlag[3]; 				/* ������Ч�Ա�־���̶�ֵ"E1"*/
	UCHAR		nvmVersion[9];			/* ����汾�ţ���ʽ"VxRyyBzzz"*/
	ULONG		nvmDataSize; 			/* �洢���ݳ��ȣ�������ͷ������ */
}PACKED  nvm_e1NvmDataHead_t;



typedef struct {
	ULONG  onuDevId;
	UCHAR  onuE1SlotId;
	UCHAR  onuE1Id;

	UCHAR  eponE1LocalEnable;
	UCHAR  eponE1Description[E1_DESCRIPTION_MAX + 1];
} PACKED  nvm_e1LinkEntry_t;	/* Link������Ϣ */

typedef struct {
	USHORT	e1LinkTableEntry_Num;			/* Link�������24 */
	nvm_e1LinkEntry_t e1LinkEntry[MAX_SGIF_E1 * MAX_SGIF_ID];/* Link���� */
} PACKED nvm_e1LinkTable_t;



typedef struct {
	USHORT eponE1PortAlarmStatus;
	USHORT eponE1PortAlarmMask;
	UCHAR  eponE1Loop;
	UCHAR  eponE1TxClock;
} PACKED  nvm_e1PortEntry_t;	/* Port������Ϣ */

typedef struct {
	USHORT	e1PortTableEntry_Num;			/* Port�������24 */
	nvm_e1PortEntry_t e1PortEntry[MAX_SGIF_E1 * MAX_SGIF_ID];/* Port���� */
} PACKED nvm_e1PortTable_t;

typedef struct/* 50KB���� */
{
	USHORT  alarmmask[0]; /* Port����(MAX_PON_CHIP * MAXONUPERPON * MAX_ONU_E1_SLOT_ID * MAX_ONU_BOARD_E1) */
} PACKED nvm_sw_onu_e1PortTable_t;/* ��������ONU E1�澯���� */


typedef struct {
	UCHAR  eponVlanEnable;
	UCHAR  eponVlanPri;
	USHORT eponVlanId;
} PACKED  nvm_e1VlanEntry_t;	/* Vlan������Ϣ */

typedef struct {
	USHORT	e1VlanTableEntry_Num;			/* Vlan�������3 */
	nvm_e1VlanEntry_t e1VlanEntry[MAX_SGIF_ID];/* Vlan���� */
} PACKED nvm_e1VlanTable_t;


extern LONG e1_DataSave_CommandInstall(void);

#endif





