
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

/*TDM数据在flash中的最大存储空间*/
#define TDM_CFG_FILE_LEN	(sizeof(nvm_tdmNvmDataHead_t) + \
                                                   sizeof(nvm_eponSgTable_t) + \
                                                   sizeof(nvm_eponTdmOnuTable_t) + \
                                                   sizeof(nvm_eponPotsLinkTable_t) )	

typedef struct {
	UCHAR		nvmFlag[3]; 				/* 数据有效性标志，固定值"TDM"*/
	UCHAR		nvmVersion[9];			/* 软件版本号，格式"VxRyyBzzz"*/
	ULONG		nvmDataSize; 			/* 存储数据长度，不包括头自身长度 */
}PACKED  nvm_tdmNvmDataHead_t;

typedef struct {
	UCHAR eponSgIfIndex;				/* 信令网关索引 */
	UCHAR eponSgHdlcMasterE1;			/* 主HDLC所用时隙对应E1编号 */
	UCHAR eponSgHdlcSlaveE1;			/* 从HDLC所用时隙对应E1编号 */
	UCHAR eponSgHdlcE1Clk;				/* 时钟跟踪E1编号 */
	UCHAR eponSgVlanEnable;				/* 使能VLAN */
	USHORT eponSgVlanId;				/* VLAN_ID */
	UCHAR eponSgVlanPri;				/* VLAN优先级 */
} PACKED  nvm_eponSgEntry_t;	/* SG表项信息 */

#define SG_NUM_MAX		3
typedef struct {
	USHORT	eponSgEntry_Num;			/* SG数，最大＝3 */
	nvm_eponSgEntry_t eponSgEntry[SG_NUM_MAX];		/* SG表项 */
} PACKED nvm_eponSgTable_t;

typedef struct {
	ULONG eponTdmOnuDevIndex;			/* ONU设备索引 */
	UCHAR eponTdmOnuSgIfIndex;			/* 信令网关索引 */
	USHORT eponTdmOnuLogicalOnuIndex;	/* ONU逻辑编号 */
	UCHAR eponTdmOnuRowStatus;			/*表行状态*/
} PACKED  nvm_eponTdmOnuEntry_t;

#define TDMONU_NUM_MAX	(3*256)
typedef struct {
	USHORT	eponTdmOnuEntry_Num;	/* ONU逻辑编号表条目数，最大＝3×256 */
	nvm_eponTdmOnuEntry_t eponTdmOnuEntry[TDMONU_NUM_MAX];/* 语音ONU逻辑编号数据表项 */
} PACKED  nvm_eponTdmOnuTable_t;

typedef struct {
	UCHAR eponPotsLinkSgIfIndex;			/* 信令网关索引 */
	USHORT eponPotsLinkSgPortIndex;		/* 逻辑端口号 */
	ULONG eponPotsLinkOnuDev;			/* ONU设备索引 */
	UCHAR eponPotsLinkOnuBoard;			/* ONU上槽位号 */
	UCHAR eponPotsLinkOnuPots;			/* ONU上POTS口索引 */
	ULONG eponPotsLinkPhoneCode;		/* 电话号码 */
	UCHAR eponPotsLinkDesc[32];			/* 连接的描述 */
	UCHAR eponPotsLinkRowStatus;			/* 表行状态 */
}PACKED  nvm_eponPotsLinkEntry_t;

#define POTSLINK_NUM_MAX	(3*2048) 
typedef struct {
	USHORT	eponPotsLinkEntry_Num;	/* 语音连接数据条目数，最大＝3×2048 */
	nvm_eponPotsLinkEntry_t eponPotsLinkEntry[POTSLINK_NUM_MAX];	/* 语音连接数据表项 */
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
     USHORT epone1portentry_Num;/*最大的E1端口数目*/
     nvm_e1portentry_t   epone1portentry[E1PORT_NUM_MAX];/*E1 端口的数据比表项*/
}PACKED  nvm_eponE1PortTable_t;
/*end shixh@20080402*/

typedef struct {
	nvm_tdmNvmDataHead_t tdmDataHead; 	/* 存储数据头 */
	nvm_eponSgTable_t *eponSgTable;		/* SG基本信息 */
	nvm_eponTdmOnuTable_t *eponTdmOnuTable;	/* 语音ONU逻辑编号数据 */
	nvm_eponPotsLinkTable_t *eponPotsLinkTable;	/* 语音连接数据 */
	nvm_eponE1PortTable_t *eponE1PortTable;    /*E1端口数据*/
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





