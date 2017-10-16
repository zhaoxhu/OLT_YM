

#ifndef _MN_E1_H_
#define _MN_E1_H_

#include "e1_oam.h"
#include "e1_apis.h"

typedef struct 
{
	ULONG  onuDevId;
	UCHAR  onuE1SlotId;
	UCHAR  onuE1Id;

}__attribute__((packed)) OnuE1Index;



extern STATUS checkE1LinkIsExist(struct vty *vty, UCHAR tdmE1Index, ULONG onuDevId, UCHAR onuE1SlotId, UCHAR  onuE1Id);
extern STATUS AddE1Link( UCHAR tdmE1Index, ULONG onuDevId, UCHAR onuE1SlotId, UCHAR  onuE1Id, UCHAR *eponE1Description );
extern STATUS checkE1LinkIsNotExist(struct vty *vty, UCHAR tdmE1Index);
extern STATUS DelE1Link( UCHAR tdmE1Index );

extern STATUS GetE1VlanEnable(unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short *VlanId);

/* ȡ��һ��ONU����E1�忨����Ϣ����������дpOAM_OnuE1Info�ṹ */
extern STATUS GetOamOnuE1Info(ULONG onuIndex, OAM_OnuE1Info *pOAM_OnuE1Info);
/* ȡ��һ��ONU E1�忨����Ϣ */
extern STATUS GetOamOnuBoardE1Info(ULONG onuIndex, UCHAR slotIdx, OAM_OnuE1Info *pOAM_OnuE1Info);

/* һ��ONU������E1��Ϣ,����OLT���ONU�� */
typedef struct
{
	UCHAR  onuE1Slot;
	UCHAR  onuE1Index;
	UCHAR  tdmE1Enable;
	UCHAR  tdmSgIfId;/*0-2*/
	UCHAR  tdmE1Id;/*0-7*/
	UCHAR  tdmVlanEnable;
	USHORT tdmVlanTag;
	USHORT tdmAlarmStat;
	USHORT tdmAlarmMask;
	UCHAR  tdmE1Loop;
	UCHAR  tdmE1TxClk;
    /* onu��E1�˿���Ϣ */
	USHORT onuE1PortAlarmStatus;	/* (R) */
	USHORT onuE1PortAlarmMask;		/* (RW) */
	UCHAR  onuE1Loop;              /* (RW) */
	UCHAR  onuE1TxClock;           /* (RW) */
}__attribute__((packed)) OnuEachE1Info_t;

typedef struct
{
	UCHAR  onuValidE1Count;/* ˵��onuEachE1Info[]��Ч�� */
	OnuEachE1Info_t onuEachE1Info[MAX_SGIF_E1 * MAX_SGIF_ID];
}__attribute__((packed)) OnuE1Info;

/* ȡ��һ��ONU�ϵ�����E1������Ϣ */
extern STATUS GetOnuE1Info(ULONG onuIndex, OnuE1Info *pOnuE1Info);


#endif


