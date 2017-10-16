/**************************************************************
*
*   V2R1_product.h -- product type MACRO definition 
*
*  
*    Copyright (c)  2008.6 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   -------|--- -------|----------- -------|------------
*	1.00	  | 26/06/2008 | Create        			| chen fj
*
***************************************************************/
#ifndef __V2R1_PRODUCT_H__
#define __V2R1_PRODUCT_H__

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if 0 /*wangdp*/
extern char  OltBoardTypeString[][BOARDTYPESTRINGLEN+1] ;  /* 板卡类型名称*/
extern char  OltBoardTypeStringShort[][BOARDTYPESTRINGLEN_SHORT+1] ;	/* 板卡类型名称简写*/
#endif
typedef short int (*ONU_REGISTER_HANDLER_hook_t)( short int PonPortIdx, short int OnuIdx);
typedef short int (*ONU_DEREGISTER_HANDLER_hook_t)( short int PonPortIdx, short int OnuIdx);

#define  V2R1_GFA6700_EPON_MODULE    0x01
#define  V2R1_GFA6700_TDM_MODULE     0x02

#define  V2R1_GFA6100_EPON_MODULE    0x11
#define  V2R1_GFA6100_TDM_MODULE     0x12

#define  V2R1_GFA6900_EPON_MODULE    0x91
#define  V2R1_GFA6900_UPLINK_MODULE  0x92
#define  V2R1_GFA6900_TDM_MODULE     0x93


typedef struct {
	unsigned int PonPortMaxNum;
	unsigned int PonPortNumPerCard;
	unsigned int PonCardNum;
	unsigned int PonCardFirst;
	unsigned int PonCardLast;
	unsigned int PonCardDevNum;

	unsigned int SwCardFirst;
	unsigned int SwCardLast;

	unsigned int UplinkCardFirst;
	unsigned int UplinkCardLast;
	unsigned int UplinkPortNumPerCard;
	
	unsigned int TdmPortPerCard;
	unsigned int E1NumPerTdmPort;
	unsigned int TdmCardFirst;
	unsigned int TdmCardLast;

	unsigned int EthPortMaxNum;
	
	unsigned int OltCardFirst;
	unsigned int OltCardLast;
	unsigned int OltPortCardFirst;
	unsigned int OltPortCardLast;
	unsigned int OltPortPerCardMax;

       unsigned int OltMaxOnuPerPon;
}__attribute__((packed))OltDeviceAttr_t;

typedef struct{
	unsigned int  SupportedModuleTypeList;
	unsigned int  SupportedPortMax;
}__attribute__((packed))SlotAttr_t;

extern OltDeviceAttr_t  OltChassis;
extern SlotAttr_t  SlotAttr[];

#ifndef PRODUCT_MAX
#define	PRODUCT_MAX(a, b)       (((a)>(b))?(a):(b))
#endif
#ifndef PRODUCT_MIN
#define	PRODUCT_MIN(a, b)       (((a)<(b))?(a):(b))
#endif
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif

#define  ETHPORTPERCARD  (OltChassis.UplinkPortNumPerCard)   /* 每个上联板板上最大Eth端口数*/
#define  MAXPONCHIP  (OltChassis.PonPortMaxNum)    /* 系统可支持的最大PON 芯片数 */
#define  PONPORTPERCARD  (OltChassis.PonPortNumPerCard)   /* 每个PON板上最大PON 芯片数*/
#define  MAXPON  (OltChassis.PonPortMaxNum)   /* 系统可支持的最大PON 端口数 */
#define  MAX_PON_BOARD_NUM  (OltChassis.PonCardNum)
#define  MAX_PON_BOARD_DEVNUM  (OltChassis.PonCardDevNum)
#define  MAXETH  (OltChassis.EthPortMaxNum)   /* 系统可支持的最大ETH 端口数 */
#define  MAXGLOBALPON  (MAX_PON_BOARD_DEVNUM * PONPORTPERCARD)   /* 系统可支持的最大PON 端口数 */

#define  PONCARD_FIRST (OltChassis.PonCardFirst)   /* 系统中可插入PON 板的第一个槽位号*/
#define  PONCARD_LAST  (OltChassis.PonCardLast)    /* 系统中可插入PON 板的最后一个槽位号*/
#define  TDMCARD_FIRST (OltChassis.TdmCardFirst)   /* 系统中可插入TDM 板的第一个槽位号*/
#define  TDMCARD_LAST  (OltChassis.TdmCardLast)    /* 系统中可插入TDM 板的最后一个槽位号*/
#define  SWCARD_FIRST  (OltChassis.SwCardFirst)
#define  SWCARD_LAST    (OltChassis.SwCardLast)
#define  UPCARD_FIRST   (OltChassis.UplinkCardFirst)   /* 系统中可插入上联板的第一个槽位号*/
#define  UPCARD_LAST    (OltChassis.UplinkCardLast)    /* 系统中可插入上联板的最后一个槽位号*/
#define  OLTCARD_LAST   (OltChassis.OltCardLast)
#define  OLTCARD_FIRST  (OltChassis.OltCardFirst)
#define  MAX_PONPORT_PER_BOARD  (OltChassis.PonPortNumPerCard)
#define  OLTPORTCARD_FIRST (OltChassis.OltPortCardFirst)
#define  OLTPORTCARD_LAST (OltChassis.OltPortCardLast)
#define  OLTPORTPERCARD  (OltChassis.OltPortPerCardMax)   /* 单板上最大端口数*/


/********  for  语音onu *************/
#define ONU_POTS_NUM_MIN   1    
#define ONU_POTS_NUM_MAX  32  /* ONU 支持的最大POTS 口数*/
#define ONU_POTS_ALL  0xffffffff 

#define  MAXONUPERSG  256    /*  每个TDM 接口可支持的最大语音ONU 数*/

#define MAXIMUM_LOGICPORT 2047  /*  每个TDM 接口可支持的最大POTS 连接数*/
#define MINIMUM_LOGICPORT  0

#define  TDM_FPGA_PORT_MAX   3     /* 每TDM 板上最大FPGA 个数, 只用于数组声明*/
#define TDM_FPGA_MIN   1
#define TDM_FPGA_MAX  (OltChassis.TdmPortPerCard)           /* 每个TDM 板上可支持的最大fpga  端口数 */

#define MAX_E1_PER_FPGA  (OltChassis.E1NumPerTdmPort)  /* 每fpga 上支持的E1	端口数  */
extern int MAX_E1_PORT_NUM;                          /* 每TDM 板上支持的最大E1 端口数 */

extern int FLASH_BASE;


/* olt card slot No. defined */
extern int 	UPLINK[];
extern int  SW[];
extern int  PON[];
extern int 	PWU[];
extern int 	FAN[];
/*#define  MAX_SLOT_ID SYS_CHASSIS_SLOTNUM*/


extern unsigned char *CardSlot_s[];

/*extern unsigned char CardSlot_s_1[][];*/

typedef struct{
	int sck;
	int ss;
	int miso;
	int mosi;
}Gpio_signal; 
extern Gpio_signal CtrlChannel_Gpio;
#define SCK  (CtrlChannel_Gpio.sck)
#define SS  (CtrlChannel_Gpio.ss)
#define MISO (CtrlChannel_Gpio.miso)
#define MOSI  (CtrlChannel_Gpio.mosi)
 
extern void InitProductTypeArray();

/**********   onu  ******************/
#define  ONU_SUB_BOARD_MAX_NUMBER   8

enum  ONU_SUB_BOARD_TYPE
{
	SUB_BOARD_NULL			= 0,
	SUB_BOARD_GT_EPON_B		= 1,
	SUB_BOARD_GT_8POTS_A		= 2,
	SUB_BOARD_GT_6FE			= 3,
	SUB_BOARD_GT_8FE			= 4,
	SUB_BOARD_GT_16FE		= 5,
	SUB_BOARD_GT_8FXS_A		= 6,
	SUB_BOARD_GT_8POTS_B		= 7,
	SUB_BOARD_GT_8FXS_B		= 8,
	SUB_BOARD_GT_8POTSO_A	= 9,/* added by xieshl 20080815 */
	SUB_BOARD_GT_8POTSO_B	= 10,
	SUB_BOARD_4E1_75OHM  = 11,
	SUB_BOARD_4E1_120OHM  = 12,

	SUB_BOARD_GT_LAST
};

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为交换板(对GFA6700,SW板. 对GFA6100,SW板)
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 本板为SW板
                        RERROR -- 本板不是SW板
*********************************************************************************************/
extern int  SlotCardIsSwBoard(int CardIndex);

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     带有PON 芯片
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 板上有PON 芯片; 对GFA6700, 即为PON 板; 对GFA6100, 为SW板或PON板
                        RERROR -- 板上没有PON 芯片,或没有插板
*********************************************************************************************/
extern int  SlotCardIsPonBoard(int CardIndex);
/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     带有PON 芯片(对GFA6700,PON板. 对GFA6100, PON板和SW板)
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 板上可能有PON 芯片; 对GFA6700, 即为(或未初始化I2C 的)PON板,或空槽位;
                                    对GFA6100, 为SW板或PON板,或未初始化I2C 的PON板, 或空槽位
                        RERROR -- 板上没有PON 芯片
*********************************************************************************************/
extern int  SlotCardMayBePonBoardByVty(int CardIndex, struct vty *vty);
/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM 板
                        RERROR -- 不是TDM 板,或没有插板
*********************************************************************************************/
extern int  SlotCardIsTdmBoard( int CardIndex );
/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -sg 板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM 板
*********************************************************************************************/
extern int SlotCardMayBeTdmBoardByVty( int CardIndex, struct vty *vty);

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM -SG 板;
     注:  以下各板都统称为TDM -SG板
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     		
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -sg 板
                        RERROR -- 不是TDM -sg 板,或没有插板
*********************************************************************************************/
extern int SlotCardIsTdmSgBoard( int CardIndex );

/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM -sg 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -sg 板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM -sg 板
*********************************************************************************************/
extern int SlotCardMayBeTdmSgBoardByVty( int CardIndex , struct vty *vty);

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM -e1 板;
     注:  以下各板都统称为TDM -e1板
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     	注: E1 板可能还会分两种，	每FPGA 出8个E1 的板卡，和每FGPA 出16 个E1 的板卡
     		   这里不再细分，统称为TDM-e1 板；每FPGA出的不同的E1 个数可在具体应用
     		   细节上区分	
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -e1板
                        RERROR -- 不是TDM -e1 板,或没有插板
*********************************************************************************************/
extern int SlotCardIsTdmE1Board( int CardIndex );

/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM -e1 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -e1g 板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM -e1 板
*********************************************************************************************/
extern int SlotCardMayBeTdmE1BoardByVty( int CardIndex , struct vty *vty);


/* ********************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对tdm 板Slot 和Port 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
                            PortIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**********************************************************************/
extern int TdmCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板Slot 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**************************************************************/
extern int TdmCardSlotRangeCheck(unsigned long SlotIdx);
/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板port的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**************************************************************/
extern int TdmPortRangeCheck(unsigned long PortIdx);

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板E1 的
    范围进行判断, 超出范围的提示错误
   输入参数: E1Num -- 从1 开始-- MAX_E1_PORT_NUM
   返回值:  ROK
                       RERROR
**************************************************************/
extern int TdmE1NumRangeCheck(unsigned long E1Num);

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板每端口支持的E1 
    范围进行判断, 超出范围的提示错误
   输入参数: E1NumPerFpga -- 从1 开始-- MAX_E1_PER_FPGA
   返回值:  ROK
                       RERROR
**************************************************************/
extern int TdmFpgaE1NumRangeCheck(unsigned long E1NumPerPort);

extern int  InitGlobalVariable();
/* ********************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对PON板Slot 和Port 的
    范围进行判断, 超出范围的提示错误，主要用于CLI 
   输入参数: slotIdx -- 取值: 从1 开始
                            PortIdx -- 
   返回值:  ROK
                       RERROR
**********************************************************************/
extern int PonCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);
extern int PonCardSlotPortCheckWhenRunningByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);

extern int PonCardSlotRangeCheckByVty(unsigned long SlotIdx,struct vty *vty);
/*extern int PonCardSlotPortCheck(unsigned long SlotIdx, unsigned long PortIdx);*/

/********* ********************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 和槽位上插入的板卡不同,
    动态修改板卡 名称( 板卡的拔出/插入)
   输入参数: slotIdx -- 取值: 从1 开始
   返回值:  ROK
                       RERROR
******************************************************************************
extern void  ModifyCardNameBySlot(unsigned long CardIdx);*/

/*
extern unsigned char * GetGFA6xxxSwNameString();
extern unsigned char * GetGFA6xxxEponNameString();
*/
extern unsigned char * GetGFA6xxxTdmNameString();

extern void Pon_Monitor_Eth( void * pData, unsigned long event, void * pData2 );


/*
	board eeprom reg 36
	Bit3～0表示光模块制造商编号：
	1－Fibrexon、2－Hisense、3－PHOTON、4－WTD、5－GWD
*/
/*typedef enum{
	PON_SFP_TYPE_NULL =0,
	PON_SFP_TYPE_FIBREXON,
	PON_SFP_TYPE_HISENSE,
	PON_SFP_TYPE_PHOTON,
	PON_SFP_TYPE_WTD,
	PON_SFP_TYPE_GWD,
	PON_SFP_TYPE_MAX
}PonSFPType_e;*/

#define  SFP_TYPE_Vendor_Name_Hisense   "Hisense"
#define  SFP_TYPE_Vendor_Name_WTD  "WTD"
#define  SFP_TYPE_Vendor_Name_DEFAULT  "GWD"
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __V2R1_PRODUCT_H__ */

