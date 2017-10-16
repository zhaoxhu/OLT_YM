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
extern char  OltBoardTypeString[][BOARDTYPESTRINGLEN+1] ;  /* �忨��������*/
extern char  OltBoardTypeStringShort[][BOARDTYPESTRINGLEN_SHORT+1] ;	/* �忨�������Ƽ�д*/
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

#define  ETHPORTPERCARD  (OltChassis.UplinkPortNumPerCard)   /* ÿ��������������Eth�˿���*/
#define  MAXPONCHIP  (OltChassis.PonPortMaxNum)    /* ϵͳ��֧�ֵ����PON оƬ�� */
#define  PONPORTPERCARD  (OltChassis.PonPortNumPerCard)   /* ÿ��PON�������PON оƬ��*/
#define  MAXPON  (OltChassis.PonPortMaxNum)   /* ϵͳ��֧�ֵ����PON �˿��� */
#define  MAX_PON_BOARD_NUM  (OltChassis.PonCardNum)
#define  MAX_PON_BOARD_DEVNUM  (OltChassis.PonCardDevNum)
#define  MAXETH  (OltChassis.EthPortMaxNum)   /* ϵͳ��֧�ֵ����ETH �˿��� */
#define  MAXGLOBALPON  (MAX_PON_BOARD_DEVNUM * PONPORTPERCARD)   /* ϵͳ��֧�ֵ����PON �˿��� */

#define  PONCARD_FIRST (OltChassis.PonCardFirst)   /* ϵͳ�пɲ���PON ��ĵ�һ����λ��*/
#define  PONCARD_LAST  (OltChassis.PonCardLast)    /* ϵͳ�пɲ���PON ������һ����λ��*/
#define  TDMCARD_FIRST (OltChassis.TdmCardFirst)   /* ϵͳ�пɲ���TDM ��ĵ�һ����λ��*/
#define  TDMCARD_LAST  (OltChassis.TdmCardLast)    /* ϵͳ�пɲ���TDM ������һ����λ��*/
#define  SWCARD_FIRST  (OltChassis.SwCardFirst)
#define  SWCARD_LAST    (OltChassis.SwCardLast)
#define  UPCARD_FIRST   (OltChassis.UplinkCardFirst)   /* ϵͳ�пɲ���������ĵ�һ����λ��*/
#define  UPCARD_LAST    (OltChassis.UplinkCardLast)    /* ϵͳ�пɲ�������������һ����λ��*/
#define  OLTCARD_LAST   (OltChassis.OltCardLast)
#define  OLTCARD_FIRST  (OltChassis.OltCardFirst)
#define  MAX_PONPORT_PER_BOARD  (OltChassis.PonPortNumPerCard)
#define  OLTPORTCARD_FIRST (OltChassis.OltPortCardFirst)
#define  OLTPORTCARD_LAST (OltChassis.OltPortCardLast)
#define  OLTPORTPERCARD  (OltChassis.OltPortPerCardMax)   /* ���������˿���*/


/********  for  ����onu *************/
#define ONU_POTS_NUM_MIN   1    
#define ONU_POTS_NUM_MAX  32  /* ONU ֧�ֵ����POTS ����*/
#define ONU_POTS_ALL  0xffffffff 

#define  MAXONUPERSG  256    /*  ÿ��TDM �ӿڿ�֧�ֵ��������ONU ��*/

#define MAXIMUM_LOGICPORT 2047  /*  ÿ��TDM �ӿڿ�֧�ֵ����POTS ������*/
#define MINIMUM_LOGICPORT  0

#define  TDM_FPGA_PORT_MAX   3     /* ÿTDM �������FPGA ����, ֻ������������*/
#define TDM_FPGA_MIN   1
#define TDM_FPGA_MAX  (OltChassis.TdmPortPerCard)           /* ÿ��TDM ���Ͽ�֧�ֵ����fpga  �˿��� */

#define MAX_E1_PER_FPGA  (OltChassis.E1NumPerTdmPort)  /* ÿfpga ��֧�ֵ�E1	�˿���  */
extern int MAX_E1_PORT_NUM;                          /* ÿTDM ����֧�ֵ����E1 �˿��� */

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
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ�
     Ϊ������(��GFA6700,SW��. ��GFA6100,SW��)
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ����ΪSW��
                        RERROR -- ���岻��SW��
*********************************************************************************************/
extern int  SlotCardIsSwBoard(int CardIndex);

/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ�
     ����PON оƬ
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ������PON оƬ; ��GFA6700, ��ΪPON ��; ��GFA6100, ΪSW���PON��
                        RERROR -- ����û��PON оƬ,��û�в��
*********************************************************************************************/
extern int  SlotCardIsPonBoard(int CardIndex);
/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ����
     ����PON оƬ(��GFA6700,PON��. ��GFA6100, PON���SW��)
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ���Ͽ�����PON оƬ; ��GFA6700, ��Ϊ(��δ��ʼ��I2C ��)PON��,��ղ�λ;
                                    ��GFA6100, ΪSW���PON��,��δ��ʼ��I2C ��PON��, ��ղ�λ
                        RERROR -- ����û��PON оƬ
*********************************************************************************************/
extern int  SlotCardMayBePonBoardByVty(int CardIndex, struct vty *vty);
/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ�
     ΪTDM ��
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM ��
                        RERROR -- ����TDM ��,��û�в��
*********************************************************************************************/
extern int  SlotCardIsTdmBoard( int CardIndex );
/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ����
     ΪTDM ��
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM -sg ��,��δ��ʼ��I2C ��TDM��,��Ϊ�ղ�λ
                        RERROR -- ����TDM ��
*********************************************************************************************/
extern int SlotCardMayBeTdmBoardByVty( int CardIndex, struct vty *vty);

/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ�
     ΪTDM -SG ��;
     ע:  ���¸��嶼ͳ��ΪTDM -SG��
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     		
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM -sg ��
                        RERROR -- ����TDM -sg ��,��û�в��
*********************************************************************************************/
extern int SlotCardIsTdmSgBoard( int CardIndex );

/* ******************************************************************************************
	����CLI
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ����
     ΪTDM -sg ��
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM -sg ��,��δ��ʼ��I2C ��TDM��,��Ϊ�ղ�λ
                        RERROR -- ����TDM -sg ��
*********************************************************************************************/
extern int SlotCardMayBeTdmSgBoardByVty( int CardIndex , struct vty *vty);

/* ******************************************************************************************
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ�
     ΪTDM -e1 ��;
     ע:  ���¸��嶼ͳ��ΪTDM -e1��
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     	ע: E1 ����ܻ�������֣�	ÿFPGA ��8��E1 �İ忨����ÿFGPA ��16 ��E1 �İ忨
     		   ���ﲻ��ϸ�֣�ͳ��ΪTDM-e1 �壻ÿFPGA���Ĳ�ͬ��E1 �������ھ���Ӧ��
     		   ϸ��������	
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM -e1��
                        RERROR -- ����TDM -e1 ��,��û�в��
*********************************************************************************************/
extern int SlotCardIsTdmE1Board( int CardIndex );

/* ******************************************************************************************
	����CLI
     ���ݲ�Ʒ���͵Ĳ�ͬ( GFA6700, or GFA6100 ), �жϵ�ǰ��λ�еİ忨�Ƿ����
     ΪTDM -e1 ��
     �������:  unsigned int CardIndex, ȡֵ: ��1 ��ʼ
     ����ֵ: ROK -- ��TDM -e1g ��,��δ��ʼ��I2C ��TDM��,��Ϊ�ղ�λ
                        RERROR -- ����TDM -e1 ��
*********************************************************************************************/
extern int SlotCardMayBeTdmE1BoardByVty( int CardIndex , struct vty *vty);


/* ********************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��tdm ��Slot ��Port ��
    ��Χ�����ж�, ������Χ����ʾ����
   �������: slotIdx -- ��1 ��ʼ
                            PortIdx -- ��1 ��ʼ
   ����ֵ:  ROK
                       RERROR
**********************************************************************/
extern int TdmCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);

/* ************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��TDM ��Slot ��
    ��Χ�����ж�, ������Χ����ʾ����
   �������: slotIdx -- ��1 ��ʼ
   ����ֵ:  ROK
                       RERROR
**************************************************************/
extern int TdmCardSlotRangeCheck(unsigned long SlotIdx);
/* ************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��TDM ��port��
    ��Χ�����ж�, ������Χ����ʾ����
   �������: slotIdx -- ��1 ��ʼ
   ����ֵ:  ROK
                       RERROR
**************************************************************/
extern int TdmPortRangeCheck(unsigned long PortIdx);

/* ************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��TDM ��E1 ��
    ��Χ�����ж�, ������Χ����ʾ����
   �������: E1Num -- ��1 ��ʼ-- MAX_E1_PORT_NUM
   ����ֵ:  ROK
                       RERROR
**************************************************************/
extern int TdmE1NumRangeCheck(unsigned long E1Num);

/* ************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��TDM ��ÿ�˿�֧�ֵ�E1 
    ��Χ�����ж�, ������Χ����ʾ����
   �������: E1NumPerFpga -- ��1 ��ʼ-- MAX_E1_PER_FPGA
   ����ֵ:  ROK
                       RERROR
**************************************************************/
extern int TdmFpgaE1NumRangeCheck(unsigned long E1NumPerPort);

extern int  InitGlobalVariable();
/* ********************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), ��PON��Slot ��Port ��
    ��Χ�����ж�, ������Χ����ʾ������Ҫ����CLI 
   �������: slotIdx -- ȡֵ: ��1 ��ʼ
                            PortIdx -- 
   ����ֵ:  ROK
                       RERROR
**********************************************************************/
extern int PonCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);
extern int PonCardSlotPortCheckWhenRunningByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty);

extern int PonCardSlotRangeCheckByVty(unsigned long SlotIdx,struct vty *vty);
/*extern int PonCardSlotPortCheck(unsigned long SlotIdx, unsigned long PortIdx);*/

/********* ********************************************************************
    ���ݲ�Ʒ���Ͳ�ͬ( GFA6700, or GFA6100 ), �Ͳ�λ�ϲ���İ忨��ͬ,
    ��̬�޸İ忨 ����( �忨�İγ�/����)
   �������: slotIdx -- ȡֵ: ��1 ��ʼ
   ����ֵ:  ROK
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
	Bit3��0��ʾ��ģ�������̱�ţ�
	1��Fibrexon��2��Hisense��3��PHOTON��4��WTD��5��GWD
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

