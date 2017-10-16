
/*#ifndef	CTC_CFGDATASACE_H
#define	CTC_CFGDATASACE_H*/
#ifndef  TDMDATASAVE_H
#define   TDMDATASAVE_H

#ifndef GW_EPON_SYS_H
#include	"../../mib/gwEponSys.h"
#endif

#include "cli/cli.h"

#ifndef	PACKED
#define	PACKED	__attribute__((packed))
#endif

/*TDM������flash�е����洢�ռ�*/
#define TDM_CFG_FILE_LEN	(sizeof(nvm_tdmNvmDataHead_t) + \
                                                   sizeof(nvm_eponSgTable_t) + \
                                                   sizeof(nvm_eponTdmOnuTable_t) + \
                                                   sizeof(nvm_eponPotsLinkTable_t) )	

typedef struct {
	UCHAR		nvmFlag[3]; 				/* ������Ч�Ա�־���̶�ֵ"TDM"*/
	UCHAR		nvmVersion[9];			/* ����汾�ţ���ʽ"VxRyyBzzz"*/
	ULONG		nvmDataSize; 			/* �洢���ݳ��ȣ�������ͷ������ */
}PACKED  nvm_tdmNvmDataHead_t;

typedef struct {
	UCHAR eponSgIfIndex;				/* ������������ */
	UCHAR eponSgHdlcMasterE1;			/* ��HDLC����ʱ϶��ӦE1��� */
	UCHAR eponSgHdlcSlaveE1;			/* ��HDLC����ʱ϶��ӦE1��� */
	UCHAR eponSgHdlcE1Clk;				/* ʱ�Ӹ���E1��� */
	UCHAR eponSgVlanEnable;				/* ʹ��VLAN */
	USHORT eponSgVlanId;				/* VLAN_ID */
	UCHAR eponSgVlanPri;				/* VLAN���ȼ� */
} PACKED  nvm_eponSgEntry_t;	/* SG������Ϣ */

#define SG_NUM_MAX		3
typedef struct {
	USHORT	eponSgEntry_Num;			/* SG�������3 */
	nvm_eponSgEntry_t eponSgEntry[SG_NUM_MAX];		/* SG���� */
} PACKED nvm_eponSgTable_t;

typedef struct {
	ULONG eponTdmOnuDevIndex;			/* ONU�豸���� */
	UCHAR eponTdmOnuSgIfIndex;			/* ������������ */
	USHORT eponTdmOnuLogicalOnuIndex;	/* ONU�߼���� */
	UCHAR eponTdmOnuRowStatus;			/*����״̬*/
} PACKED  nvm_eponTdmOnuEntry_t;

#define TDMONU_NUM_MAX	(3*256)
typedef struct {
	USHORT	eponTdmOnuEntry_Num;	/* ONU�߼���ű���Ŀ�������3��256 */
	nvm_eponTdmOnuEntry_t eponTdmOnuEntry[TDMONU_NUM_MAX];/* ����ONU�߼�������ݱ��� */
} PACKED  nvm_eponTdmOnuTable_t;

typedef struct {
	UCHAR eponPotsLinkSgIfIndex;			/* ������������ */
	USHORT eponPotsLinkSgPortIndex;		/* �߼��˿ں� */
	ULONG eponPotsLinkOnuDev;			/* ONU�豸���� */
	UCHAR eponPotsLinkOnuBoard;			/* ONU�ϲ�λ�� */
	UCHAR eponPotsLinkOnuPots;			/* ONU��POTS������ */
	ULONG eponPotsLinkPhoneCode;		/* �绰���� */
	UCHAR eponPotsLinkDesc[32];			/* ���ӵ����� */
	UCHAR eponPotsLinkRowStatus;			/* ����״̬ */
}PACKED  nvm_eponPotsLinkEntry_t;

#define POTSLINK_NUM_MAX	(3*2048) 
typedef struct {
	USHORT	eponPotsLinkEntry_Num;	/* ��������������Ŀ�������3��2048 */
	nvm_eponPotsLinkEntry_t eponPotsLinkEntry[POTSLINK_NUM_MAX];	/* �����������ݱ��� */
} PACKED  nvm_eponPotsLinkTable_t;

/*add byshixh@20070402*/
typedef struct{
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
	UCHAR almmask;
	UCHAR crcenable;
	UCHAR reserved;
}PACKED nvm_e1portentry_t;

#define E1PORT_NUM_MAX	24 
typedef struct{
     USHORT epone1portentry_Num;/*����E1�˿���Ŀ*/
     nvm_e1portentry_t   epone1portentry[E1PORT_NUM_MAX];/*E1 �˿ڵ����ݱȱ���*/
}PACKED  nvm_eponE1PortTable_t;
/*end shixh@20080402*/

typedef struct {
	nvm_tdmNvmDataHead_t tdmDataHead; 	/* �洢����ͷ */
	nvm_eponSgTable_t *eponSgTable;		/* SG������Ϣ */
	nvm_eponTdmOnuTable_t *eponTdmOnuTable;	/* ����ONU�߼�������� */
	nvm_eponPotsLinkTable_t *eponPotsLinkTable;	/* ������������ */
	nvm_eponE1PortTable_t *eponE1PortTable;    /*E1�˿�����*/
} PACKED  nvm_tdmData_t;


/*ULONG  savetdmNvmDataHead(const ULONG addr, const UCHAR nvmFlag[3], const UCHAR nvmVersion[9], const ULONG size);
ULONG  retrievetdmNvmDataHead(const ULONG addr, UCHAR nvmFlag[3],  UCHAR nvmVersion[9], ULONG *size);
ULONG savetdmNvmeponSgTable(const ULONG addr,USHORT  eponSgNum, nvm_eponSgEntry_t  eponSgEntry[3]);
ULONG retrievetdmNvmeponSgTable(const ULONG addr,USHORT  *eponSgNum, nvm_eponSgEntry_t  eponSgEntry[3]);
ULONG savetdmNvmeponOnuTable(const ULONG addr,USHORT  eponTdmOnuNum, nvm_eponTdmOnuEntry_t  eponTdmOnuEntry[3*256]);
ULONG retrievetdmNvmeponOnuTable(const ULONG addr,USHORT  *eponTdmOnuNum, nvm_eponTdmOnuEntry_t  eponTdmOnuEntry[3*256]);
ULONG  savetdmNvmeponPotsLInkTable( const ULONG addr,USHORT  eponpotsNum, nvm_eponPotsLinkEntry_t  eponpotsEntry[3*2048]);
ULONG  retrievetdmNvmeponPotsLInkTable( const ULONG addr,USHORT  *eponpotsNum, nvm_eponPotsLinkEntry_t  eponpotsEntry[3*2048]);*/


#endif





