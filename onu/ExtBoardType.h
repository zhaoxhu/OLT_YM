#ifndef EXT_BOARD_TYPE_H

#include "V2R1_Product.h"
#define EXT_BOARD_TYPE_H
/*#include  "OltGeneral.h"
#include  "PonGeneral.h"*/
/* begin: added by jianght 20090223  */
#include "OnuGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "e1/E1_MIB.h"
#endif
/* end: added by jianght 20090223 */

#define  MAX_ONU_BRD_NUM	5
#define  MIN_ONU_BRD_NUM	1
/*#define	MAXPON  20*/
/*#define  MAXDEVDESCLEN   128
#define  MAXDEVNAMELEN  64
#define  MAXVERINFOLEN    32
#define  MAXSWVERLEN     32
#define  MAXHWVERLEN   32
#define  MAXBTVERLEN    32
#define  MAXFWVERLEN     32
#define  MAXMFLEN   128
#define  MAXSNLEN  32*/	/* removed by xieshl 20101228, 统一采用OLTGeneral.h中的宏定义 */
#define  GET_ONU_EXT_SYS_INFO_SUCCESS  1
#define  MAXREADCOMMLEN  16
#define  MAXWRITECOMMLEN 16
#define  AGENTIPLEN  4
/*#define  MAXPRODUCTDATELEN  32*/


#define MAX_PON_CHIP  MAXPONCHIP
/*0－空、1－GT-EPON-B；2－GT-8POTS-A（RJ11接口）；3－GT-6FE；4－GT-8FE；5－GT-16FE；
6－GT-8FXS-A（RJ11接口）；7－GT-8POTS-B（RJ21接口）；8－GT-8FXS-B（RJ21接口）；
9－GT-8POTSO-A（RJ11接口）；10－GT-8POTSO-B*/
enum {
	OAM_ONU_SLOT_NULL = 0,
	OAM_ONU_SLOT_GT_EPON_B,
	OAM_ONU_SLOT_GT_8POTS_A,
	OAM_ONU_SLOT_GT_6FE,
	OAM_ONU_SLOT_GT_8FE,
	OAM_ONU_SLOT_GT_16FE,
	OAM_ONU_SLOT_GT_8FXS_A,
	OAM_ONU_SLOT_GT_8POTS_B,
	OAM_ONU_SLOT_GT_8FXS_B,
	OAM_ONU_SLOT_GT_8POTSO_A,
	OAM_ONU_SLOT_GT_8POTSO_B,
/* begin: added by jianght 20090417  */
	OAM_ONU_SLOT_GT_4E1,			/* 正式版本不用此宏 */
	OAM_ONU_SLOT_GT_4E1_120ohm,
	OAM_ONU_SLOT_GT_4E1_75ohm,
/* end: added by jianght 20090417 */
	OAM_ONU_SLOT_TYPE_MAX
};

#define ONU_BRD_IDX_IS_VALID(brdIdx) (((brdIdx) <= MAX_ONU_BRD_NUM) && ((brdIdx) >= MIN_ONU_BRD_NUM))
#define ONU_BRD_TYPE_IS_VALID(brdType) (((brdType) < OAM_ONU_SLOT_TYPE_MAX) && ((brdType) != OAM_ONU_SLOT_NULL))


#define MAX_BRD_DESC_LEN	32
#define MAX_BRD_VER_LEN	20
#define MAX_BRD_MANU_LEN	32
#define MAX_BRD_STR_LEN	16
#define MAX_BRD_COMM_LEN	16

typedef struct {
	/*char DeviceDesc[MAX_BRD_DESC_LEN+1];*/
	UCHAR brdType;
	UCHAR OperatStatus;
	char SwVersion[MAX_BRD_VER_LEN+1];
	char HwVersion[MAX_BRD_VER_LEN+1];
	char BootVersion[MAX_BRD_VER_LEN+1];
	char FwVersion[MAX_BRD_VER_LEN+1];
	char Manufacture[MAX_BRD_MANU_LEN+1];
	char SerialNum[MAX_BRD_STR_LEN+1];
	char ProductTime[MAX_BRD_STR_LEN+1];
	char ReadCommunity[MAX_BRD_COMM_LEN+1];
	char WriteCommunity[MAX_BRD_COMM_LEN+1];
	UCHAR SupportSnmpAgent;
	ULONG AgentIp;
	ULONG ChangeTime;
	/*UCHAR Reserve;*/
}__attribute__((packed)) onu_ext_brd_t;


typedef enum {
	mib_brd_type_null = 1,			/* 1 - NULL */
	mib_brd_type_unknown,			/* 2 - Unknown, */
	mib_brd_type_sw,				/* 3 - SW (OLT), master control and switch board. */
	mib_brd_type_epon,				/* 4 - EPON (OLT), slave epon board. */
	mib_brd_type_gpon,				/* 5 - GPON (OLT), slave gpon board. */
	mib_brd_type_get,				/* 6 - GET (OLT, 4GE-TX), using category 5 UTP interface. */
	mib_brd_type_geo,				/* 7 - GEO (OLT, 4GE-XX), using X fiber over PMT interface. */
	mib_brd_type_tdm,				/* 8 - TDM (OLT), slave tdm board. */
	mib_brd_type_stm1,				/* 9 - STM1 (OLT), slave stm1 board. */
	mib_brd_type_pwu48,			/* 10 - PWR48, power board with input of -48V(DC). */
	mib_brd_type_pwu220,			/* 11 - PWR220, power board with input of 220V(AC). */
	mib_brd_type_smb,				/* 12 - SMB (ONU), the simple mainboard of box device. */
	mib_brd_type_gem,				/* 13 - GEM (OLT), slave board. */
	mib_brd_type_sig,				/* 14 - SIG (OLT), slave board. */
	mib_brd_type_onuEponB,			/* 15 - EPON-B (ONU). */
	mib_brd_type_onu6FeC,			/* 16 - 6FE_C (ONU), RJ45 connector. */
	mib_brd_type_onu8FeD,			/* 17 - 8FE_D (ONU), SCSI68 connector. */
	mib_brd_type_onu16FeD,		/* 18 - 16FE_D (ONU), SCSI68 connector. */
	mib_brd_type_onu8PotsA,		/* 19 - 8POTS-A (ONU), RJ11 connector. */
	mib_brd_type_onu8PotsB,		/* 20 - 8POTS-B (ONU), RJ21 connector. */
	mib_brd_type_onu8FxsA,			/* 21 - 8FXS-A (ONU), RJ11 connector. */
	mib_brd_type_onu8FxsB,			/* 22 - 8FXS-B (ONU), RJ21 connector. */
	
	
	mib_brd_type_6100_main =23,	/*23- 6100 main*/
	mib_brd_type_6100_epon,		/*24- 6100 epon*/

	mib_brd_type_e1,				/*25- 6700 e1*/
	mib_brd_type_6100_e1,			/*26- 6100 e1*/

	mib_brd_type_onu4PotsA,		/*27- onu e1 120*/	/*add by zhengyt 09-04-22*/
	mib_brd_type_onu4PotsB,		/*28- onu e1 75*/
#if 0
	mib_brd_type_6100_gem,             /*29- 6100 4GE*/
	mib_brd_type_6100_pwu48,		/*30- 6100 DC48V*/
	mib_brd_type_6100_pwu220,		/*31- 6100 AC220V*/
#endif
	/*6900*/
	mib_brd_type_6900_sw=29,
	mib_brd_type_6900_4epon,/*30*/
	mib_brd_type_6900_8epon,/*31*/
	mib_brd_type_6900_12epon,/*32*/
	mib_brd_type_6900_gem_4ge,/*33*/
	mib_brd_type_6900_gem_10ge,/*34*/
	mib_brd_type_6900_fan,/*35*/
	mib_brd_type_6900_pwu48,/*36*/
	mib_brd_type_6900_pwu220,/*37*/
	mib_brd_type_6900_4epon_4ge,/*38*/
	mib_brd_type_6900m_pwu220,/*39*/
	mib_brd_type_6900s_fan,/*40*/
	mib_brd_type_6900s_pwu48,/*41*/
	mib_brd_type_6900s_pwu220,/*42*/
	mib_brd_type_6900_12epon_b0,/*43*/
	
	mib_brd_type_6900_16epon_b1,/*44*/

	/*8000*/
	mib_brd_type_8000_sw=45,
	mib_brd_type_8000_4epon,/*46*/
	mib_brd_type_8000_4epon_4ge,/*47*/
	mib_brd_type_8000_12epon,/*48*/
	mib_brd_type_8000_12epon_b0,/*49*/
	mib_brd_type_8000_16epon_b1,/*50*/
	mib_brd_type_8000_10g_8epon,/*51*/
	
	mib_brd_type_8000_gem_4ge,/*52*/
	mib_brd_type_8000_gem_10ge,/*53*/
	mib_brd_type_8000_fan,/*54*/
	mib_brd_type_8000_pwu48,/*55*/
	mib_brd_type_8000_pwu220,/*56*/
	mib_brd_type_8000m_pwu220,/*57*/
	mib_brd_type_8000s_fan,/*58*/
	mib_brd_type_8000s_pwu48,/*59*/
	mib_brd_type_8000s_pwu220,/*60*/
	mib_brd_type_8000_8xet=61,/*61*/
	mib_brd_type_8000_4xet,/*62*/
	mib_brd_type_8000_16gpon_b0,/*63*/

/*8100*/
/*added by wangying*/
	 mib_brd_type_8100_16epon,  /*64*/
	 mib_brd_type_8100_fan,   	/*65*/
	 mib_brd_type_8100_pwu220,  /*66*/
	 mib_brd_type_8100_pwu48,	/*67*/
	 mib_brd_type_8100_16gpon,  /*68*/

	mib_brd_type_max
} mib_board_type_t;

#define mib_brd_support_bit_sw			0x80000000
#define mib_brd_support_bit_epon		0x40000000
#define mib_brd_support_bit_gpon		0x20000000
#define mib_brd_support_bit_get			0x10000000
#define mib_brd_support_bit_geo			0x08000000
#define mib_brd_support_bit_tdm			0x04000000
#define mib_brd_support_bit_stm1		0x02000000
#define mib_brd_support_bit_pwu48		0x01000000
#define mib_brd_support_bit_pwu220		0x00800000
#define mib_brd_support_bit_smb			0x00400000
#define mib_brd_support_bit_gem			0x00200000
#define mib_brd_support_bit_sig			0x00100000
#define mib_brd_support_bit_onuEponB	0x00080000
#define mib_brd_support_bit_onu6FeC		0x00040000
#define mib_brd_support_bit_onu8FeD		0x00020000
#define mib_brd_support_bit_onu16FeD	0x00010000
#define mib_brd_support_bit_onu8PotsA	0x00008000
#define mib_brd_support_bit_onu8PotsB	0x00004000
#define mib_brd_support_bit_onu8FxsA	0x00002000
#define mib_brd_support_bit_onu8FxsB	0x00001000

typedef struct {
	UCHAR MsgType;
	UCHAR Result;
	UCHAR CurBrdNum;
	/*UCHAR QueryFlag;*/
	unsigned char  ExtData[EUQ_MAX_OAM_PDU];	/* 1500 */
}__attribute__((packed)) onu_ext_pdu_t;

typedef struct OnuType_Ext{
	onu_ext_brd_t  DeviceInfo; 
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/* begin: added by jianght 20090223  */
	e1PortTable_t onuE1PortTable[MAX_ONU_BOARD_E1];
       /* end: added by jianght 20090223 */
#endif
}__attribute__((packed)) onu_ext_brd_table_t;

typedef struct onu_ext_table_s {
	USHORT PonPortIdx;
	UCHAR OnuIdx;
	UCHAR QueryFlag;
	onu_ext_brd_table_t  BrdMgmtTable[MAX_ONU_BRD_NUM];
	struct onu_ext_table_s *next;
}__attribute__((packed)) onu_ext_table_t;


typedef struct  ONU_SYNC_EXT_MSG{
	onu_sync_msg_head_t   OnuSyncMsgHead;
	onu_ext_pdu_t ExtPdu;
}__attribute__((packed)) onu_sync_ext_msg_t;


extern LONG OnuExtMgmt_SyncRecv_Callback( VOID *pMsg );
extern LONG OnuExtMgmt_SyncReqRecv_Callback( VOID *pMsg );


#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern LONG e1OnuTakeOff(ULONG onuDevIdx);
#endif

extern LONG getOnuExtMgmtBrdType( ULONG devIdx, ULONG brdIdx, ULONG *pultype );
extern LONG getOnuExtMgmtBrdDeviceDesc( ULONG devIdx, ULONG brdIdx, char *pDesc );
extern LONG setOnuExtMgmtBrdDeviceDesc( ULONG devIdx, ULONG brdIdx, char *pDesc );
extern LONG getOnuExtMgmtBrdOperStatus( ULONG devIdx, ULONG brdIdx, ULONG *pulOperStatus );
extern LONG getOnuExtMgmtBrdChangeTime( ULONG devIdx, ULONG brdIdx, ULONG *pulChangeTime );
extern LONG getOnuExtMgmtBrdSupportType( ULONG devIdx, ULONG brdIdx, ULONG *pultype );
extern LONG getOnuExtMgmtBrdSwVer( ULONG devIdx, ULONG brdIdx, char *pSwVer );
extern LONG getOnuExtMgmtBrdFwVer( ULONG devIdx, ULONG brdIdx, char *pFwVer );
extern LONG getOnuExtMgmtBrdHwVer( ULONG devIdx, ULONG brdIdx, char *pHwVer );
extern LONG getOnuExtMgmtBrdBootVer( ULONG devIdx, ULONG brdIdx, char *pBootVer );
extern LONG getOnuExtMgmtBrdProductTime( ULONG devIdx, ULONG brdIdx, char *pProductTime );
extern LONG getOnuExtMgmtBrdReadCommnity( ULONG devIdx, ULONG brdIdx, char *pReadComm );
extern LONG getOnuExtMgmtBrdWriteCommnity( ULONG devIdx, ULONG brdIdx, char *pWriteComm );
extern LONG setOnuExtMgmtBrdWriteCommnity( ULONG devIdx, ULONG brdIdx, char *pCommunity );
extern LONG getOnuExtMgmtBrdAgentIp( ULONG devIdx, ULONG brdIdx, ULONG *ulAgentIp );
extern LONG setOnuExtMgmtBrdAgentIp( ULONG devIdx, ULONG brdIdx, ULONG *ulAgentIp );
extern LONG setOnuExtMgmtBrdReadCommnity( ULONG devIdx, ULONG brdIdx, char *pCommunity );
extern LONG getOnuExtMgmtBrdHasSnmpAgent( ULONG devIdx, ULONG brdIdx, ULONG *pAgent );
extern LONG getOnuExtMgmtBrdSerialNum( ULONG devIdx, ULONG brdIdx, char *pSerialNum );
extern BOOL onuExtMgmt_IsSupport( ULONG devIdx );
extern BOOL onu_ext_dev_support( short int PonPortIdx, short int OnuIdx );

#endif
