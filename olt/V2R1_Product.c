/***************************************************************
*
*						Module Name:  V2R1_Product.c
*
*                       (c) COPYRIGHT  by 
*                        GWTT Com. Ltd.
*                        All rights reserved.
*
*     This software is confidential and proprietary to gwtt Com, Ltd. 
*     No part of this software may be reproduced,
*     stored, transmitted, disclosed or used in any form or by any means
*     other than as expressly provided by the written Software function 
*     Agreement between gwtt and its licensee
*
*   Date: 			2008/07/08
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
** ---- ----- |----------- |------------------ 
**  2008.7.8     |   chenfj         |    creation
**
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include   "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "ponEventHandler.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "Tdm_comm.h"
#endif
#include "Typesdb_product.h"
#include "Device_flash.h"
#include "ppc405EP.h"
#include "bsp_cpld.h"
#include "sysDcr.h"


/* added by chenfj 2008-6-25 
     以下代码用于区分OLT 产品类型, GFA6700 / GFA6100 
     将宏定义变换为全局变量*/
OltDeviceAttr_t  OltChassis;
SlotAttr_t  SlotAttr[PRODUCT_MAX_TOTAL_SLOTNUM+1];

int  FLASH_BASE = 0xfe000000;

int UPLINK[SYS_MAX_UPLINK_CARDNUM];
int SW[SYS_MAX_SW_CARDNUM];
int PON[SYS_MAX_PON_CARDNUM];
int PWU[SYS_MAX_PWU_CARDNUM];
int FAN[SYS_MAX_FAN_CARDNUM];
int MAX_E1_PORT_NUM = 24;

/* int  MAX_ONU_NUM = SYS_MAX_PON_PORTNUM * MAXONUPERPON; */ /*OLT 最大能接的ONU数目，GFA6700为1280，GFA6100为256,add by shixh@20080805*/
/*unsigned int TDM_SG_MAX=3;*/

Gpio_signal CtrlChannel_Gpio;
extern int WDI;
extern int bcm_cpu_port;
extern unsigned int  MaxOnuDefault;

static unsigned char CardSlot_s_1[PRODUCT_MAX_TOTAL_SLOTNUM+1][20] = {
	"slot0",
	"slot1",
   	"slot2",
   	"slot3",
   	"slot4",
   	"slot5",
   	"slot6",
   	"slot7",
   	"slot8",
   	"slot9",
   	"slot10",
   	"slot11",
   	"slot12",
   	"slot13",
   	"slot14",
   	"slot15",
   	"slot16",
   	"slot17",
   	"slot18",
   	"slot19",
   	"slot20",
   	"slot21"
	};
unsigned char *CardSlot_s[PRODUCT_MAX_TOTAL_SLOTNUM+1] = { 0 };

/* PON端口与交换芯片物理端口对应表*/
/* B--modified by liwei056@2011-11-22 for D13980 */
#if 0
static unsigned int  PonPortToSwPort_6700[] =
{
	19,18,17,16,
	15,14,13,12,
	11,10,9,8,
	7,6,5,4,
	3,2,1,0	
};
#else
static unsigned int  PonPortToSwPort_6700[] =
{
	3,2,1,0,	
	7,6,5,4,
	11,10,9,8,
	15,14,13,12,
	19,18,17,16
};
#endif
/* E--modified by liwei056@2011-11-22 for D13980 */

static unsigned int  PonPortToSwPort_6100[] =
{
	4,5,6,7	
};

#if ( CARD_MAX_PON_PORTNUM == 16 )
#define PonPortUnknownSwport    0,0,0,0, 0,0,0,0, 0,0,0,0,   
#elif ( CARD_MAX_PON_PORTNUM == 12 )
#define PonPortUnknownSwport    0,0,0,0, 0,0,0,0,  
#elif ( CARD_MAX_PON_PORTNUM == 8 )
#define PonPortUnknownSwport    0,0,0,0,   
#elif ( CARD_MAX_PON_PORTNUM == 4 )
#define PonPortUnknownSwport
#else
#error "CARD_MAX_PON_PORTNUM is only: 4 8 12 16"
#endif

static unsigned int  PonPortToSwPort_6900[] =
{
    35,36,46,47, PonPortUnknownSwport
    41,42,48,49, PonPortUnknownSwport
    37,38,39,40, PonPortUnknownSwport
    14,15,16,17, PonPortUnknownSwport
    8,9,10,11,   PonPortUnknownSwport
    2,3,4,5,     PonPortUnknownSwport
    6,7,18,19,   PonPortUnknownSwport
    12,13,20,21, PonPortUnknownSwport
    22,23,24,25, PonPortUnknownSwport
    26,32,33,34, PonPortUnknownSwport
    50,51,52,53, PonPortUnknownSwport
    27,43,44,45, PonPortUnknownSwport
};

/* B--added by liwei056@2012-3-9 for PonCardWithPP */
static unsigned int  PonPortToSwPort_16PONCARD[] =
{
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
};
/* E--added by liwei056@2012-3-9 for PonCardWithPP */


/* PON端口与交换芯片逻辑端口对应表*/
/* B--modified by liwei056@2011-11-22 for D13980 */
#if 0
static unsigned int  PonPortToSwLogPort_6700[] =
{
	17,18,19,20,
	13,14,15,16,
	9,10,11,12,
	5,6,7,8,
	1,2,3,4	
};
#else
static unsigned int  PonPortToSwLogPort_6700[] =
{
	1,2,3,4,	
	5,6,7,8,
	9,10,11,12,
	13,14,15,16,
	17,18,19,20
};
#endif
/* E--modified by liwei056@2011-11-22 for D13980 */

static unsigned int  PonPortToSwLogPort_6100[] =
{
	5,6,7,8	
};

static unsigned int  PonPortToSwLogPort_6900[] =
{
    1,2,3,4,     0,0,0,0, 0,0,0,0, 0,0,0,0,

    5,6,7,8,     0,0,0,0, 0,0,0,0, 0,0,0,0,

    9,10,11,12,  0,0,0,0, 0,0,0,0, 0,0,0,0,

    13,14,15,16, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    17,18,19,20, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    21,22,23,24, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    25,26,27,28, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    29,30,31,32, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    33,34,35,36, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    37,38,39,40, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    41,42,43,44, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    45,46,47,48, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

/* B--added by liwei056@2012-3-9 for PonCardWithPP */
static unsigned int  PonPortToSwLogPort_16PONCARD[] =
{
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
};
/* E--added by liwei056@2012-3-9 for PonCardWithPP */

/* B--added by liwei056@2013-11-20 for PonCardWithPPbcm56334 */
void PonPortToSwReMap(int map_offset)
{
    int i;

    for ( i = 0; i < ARRAY_SIZE(PonPortToSwPort_16PONCARD); i++ )
    {
        PonPortToSwPort_16PONCARD[i] += map_offset;
    }
}
/* E--added by liwei056@2013-11-20 for PonCardWithPPbcm56334 */

int  InitGlobalVariable()
{
	int i;
	int Product_type;
	
	for(i=0; i<=PRODUCT_MAX_TOTAL_SLOTNUM; i++)
	{
		CardSlot_s[i] = CardSlot_s_1[i];
		SlotAttr[i].SupportedModuleTypeList = 0;
	}

	if(xflash_sysfile_exist() != VOS_OK)
	{
		/*sys_console_printf("\r\ndefault sysfile.ini is used to config product info\r\n");*/
		InitOltMgmtInfoByDefault();
		InitOnuMgmtInfoByDefault();
	}
	else
    {
		InitMgmtInfoBySysfile(NULL);
	}
	
	InitProductTypeArray();

    VOS_MemZero(SW,  sizeof(SW));
    VOS_MemZero(UPLINK,  sizeof(UPLINK));
    VOS_MemZero(PON, sizeof(PON));
    VOS_MemZero(PWU, sizeof(PWU));
    VOS_MemZero(FAN, sizeof(FAN));

    Product_type = GetOltType();
	if(V2R1_PRODUCT_IS_6700Series(Product_type))
	{
		FLASH_BASE = 0xfe000000;		

		UPLINK[0] = 1;
		UPLINK[1] = 2;

        SW[0] = 3;
		SW[1] = 4;

#if 1
		PON[0] = 4;
        PON[1] = 5;
		PON[2] = 6;
		PON[3] = 7;
		PON[4] = 8;
#else
		PON[4] = 4;
        PON[3] = 5;
		PON[2] = 6;
		PON[1] = 7;
		PON[0] = 8;
#endif

        PWU[0] = 9;
		PWU[1] = 10;
		PWU[2] = 11;

		OltChassis.EthPortMaxNum     = 24;
		OltChassis.PonPortMaxNum     = 20;
		OltChassis.PonPortNumPerCard = 4;
		OltChassis.PonCardNum        = 5;
		OltChassis.PonCardFirst      = 4;
		OltChassis.PonCardLast       = 8;
        OltChassis.PonCardDevNum     = 5;

        OltChassis.TdmCardFirst = 4;
		OltChassis.TdmCardLast  = 8;

        OltChassis.SwCardFirst = 3;
		OltChassis.SwCardLast  = 4;

        OltChassis.UplinkCardFirst = 1;
		OltChassis.UplinkCardLast = 1;
        OltChassis.UplinkPortNumPerCard = 4;
        
        if ( SYS_CHASSIS_IS_V2_8GE )
        {
    		OltChassis.UplinkCardLast = 2;
            
    		OltChassis.PonCardNum     = 4;
    		OltChassis.PonCardDevNum  = 4;
    		OltChassis.PonPortMaxNum  = 16;
    		OltChassis.PonCardFirst   = 5;

            OltChassis.TdmCardFirst   = 5;

            PON[0] = 5;
    		PON[1] = 6;
    		PON[2] = 7;
    		PON[3] = 8;
        }
        
		OltChassis.OltCardLast      = 11;
		OltChassis.OltCardFirst     = 1;
		OltChassis.OltPortCardFirst = 1;
		OltChassis.OltPortCardLast  = 8;
		OltChassis.OltPortPerCardMax = 4;

            OltChassis.OltMaxOnuPerPon = 64;
            
		for(i=PONCARD_FIRST;i<=PONCARD_LAST;i++)
			SlotAttr[i].SupportedModuleTypeList = (V2R1_GFA6700_EPON_MODULE | V2R1_GFA6700_TDM_MODULE);

		TDM_FPGA_MAX = TDM_FPGA_PORT_MAX;
		MAX_E1_PER_FPGA = 8;
		MAX_E1_PORT_NUM = TDM_FPGA_MAX * MAX_E1_PER_FPGA;/*add by shixh@20080725*/
		/* MAX_ONU_NUM = 1280; */ /*add byshixh@20080805*/
		
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(V2R1_OLT_GFA6700));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(V2R1_OLT_GFA6700));

		{
			VOS_StrCpy(CardSlot_s[1],"UPLINK(slot1)");
			VOS_StrCpy(CardSlot_s[2],"UPLINK(slot2)");
			VOS_StrCpy(CardSlot_s[3],"SW(slot3)");
			VOS_StrCpy(CardSlot_s[4],"SW(slot4)");
			VOS_StrCpy(CardSlot_s[5],"Pon(slot5)");
			VOS_StrCpy(CardSlot_s[6],"Pon(slot6)");
			VOS_StrCpy(CardSlot_s[7],"Pon(slot7)");
			VOS_StrCpy(CardSlot_s[8],"Pon(slot8)");
			VOS_StrCpy(CardSlot_s[9],"PWU(slot9)");
			VOS_StrCpy(CardSlot_s[10],"PWU(slot10)");
			VOS_StrCpy(CardSlot_s[11],"PWU(slot11)");			
		}

		/* PON端口与交换芯片物理端口对应表*/
        PonPortToSwPort = PonPortToSwPort_6700;

		/* PON端口与交换芯片逻辑端口对应表*/
        PonPortToSwLogPort = PonPortToSwLogPort_6700;

		SCK  = 0;
		SS   = 1;
		MOSI = 3;
		MISO = 2;

		WDI = 6;

	}
	else if(V2R1_PRODUCT_IS_6100Series(Product_type))
	{
		FLASH_BASE = 0xfe000000;		

		UPLINK[0] = 1;

        SW[0] = 2;

        PON[0] = 2;
		PON[1] = 3;

        PWU[0] = 4;
		PWU[1] = 5;

		OltChassis.EthPortMaxNum     = 8;
		OltChassis.PonPortMaxNum     = 4;
		OltChassis.PonPortNumPerCard = 2;
		OltChassis.PonCardNum        = 2;
		OltChassis.PonCardFirst      = 2;
		OltChassis.PonCardLast       = 3;
        OltChassis.PonCardDevNum     = 2;

		OltChassis.TdmCardFirst = 3;
		OltChassis.TdmCardLast  = 3;
        
		OltChassis.SwCardFirst = 2;
		OltChassis.SwCardLast  = 2;

        OltChassis.UplinkCardFirst = 1;
		OltChassis.UplinkCardLast  = 1;
		OltChassis.UplinkPortNumPerCard = 4;

		OltChassis.OltCardLast      = 5;
		OltChassis.OltCardFirst     = 1;
		OltChassis.OltPortCardFirst = 1;
		OltChassis.OltPortCardLast  = 3;
		OltChassis.OltPortPerCardMax = 4;
		
            OltChassis.OltMaxOnuPerPon = 64;
            
		for(i=PONCARD_FIRST;i<=PONCARD_LAST;i++)
		{
			if(i == PONCARD_FIRST)
				SlotAttr[i].SupportedModuleTypeList = V2R1_GFA6100_EPON_MODULE;
			else 
				SlotAttr[i].SupportedModuleTypeList = (V2R1_GFA6100_EPON_MODULE | V2R1_GFA6100_TDM_MODULE);
		}

		TDM_FPGA_MAX = 1;
		MAX_E1_PER_FPGA = 8;
		MAX_E1_PORT_NUM = TDM_FPGA_MAX * MAX_E1_PER_FPGA;/*add by shixh@20080725*/
		/* MAX_ONU_NUM = 256; */ /*add byshixh@20080805*/
		
		{
			VOS_StrCpy(CardSlot_s[1],"SW(slot1)");
			VOS_StrCpy(CardSlot_s[2],"Pon(slot2)");
			VOS_StrCpy(CardSlot_s[3],"Pon(slot3)");
			VOS_StrCpy(CardSlot_s[4],"PWU(slot4)");
			VOS_StrCpy(CardSlot_s[5],"PWU(slot5)");			
		}
	
		/*
		主板PON1=56300_5，主板PON2=56300_6,
		子板PON1=56300_7，主板PON2=56300_8，
		*/
		/* PON端口与交换芯片物理端口对应表*/
        PonPortToSwPort = PonPortToSwPort_6100;

		/* PON端口与交换芯片逻辑端口对应表*/
        PonPortToSwLogPort = PonPortToSwLogPort_6100;
		
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(V2R1_OLT_GFA6100));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(V2R1_OLT_GFA6100));

		SCK = 8;
		SS= 9;
		MOSI = 14;
		MISO = 15;

		WDI = 16;

	}
	else if(V2R1_PRODUCT_IS_HL_Series(Product_type))
    {
		FLASH_BASE = 0xfe000000;		

		UPLINK[0]  = 1;
		UPLINK[1]  = 2;
		UPLINK[2]  = 3;
		UPLINK[3]  = 4;
		UPLINK[4]  = 5;
		UPLINK[5]  = 6;
		UPLINK[6]  = 9;
		UPLINK[7]  = 10;
		UPLINK[8]  = 11;
		UPLINK[9]  = 12;
		UPLINK[10] = 13;
		UPLINK[11] = 14;
        
		SW[0] = 7;
		SW[1] = 8;

        PON[0]  = 1;
        PON[1]  = 2;
        PON[2]  = 3;
        PON[3]  = 4;
        PON[4]  = 5;
        PON[5]  = 6;
        PON[6]  = 9;
        PON[7]  = 10;
        PON[8]  = 11;
        PON[9]  = 12;
        PON[10] = 13;
        PON[11] = 14;

        FAN[0] = 15;
        FAN[1] = 16;
        FAN[2] = 17;
        
		PWU[0] = 18;
		PWU[1] = 19;
		PWU[2] = 20;
		PWU[3] = 21;

        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
        {
    		OltChassis.PonCardNum    = 1;
    		OltChassis.PonPortMaxNum = CARD_MAX_PON_PORTNUM; 
    		OltChassis.PonCardFirst  = SYS_LOCAL_MODULE_SLOTNO;
    		OltChassis.PonCardLast   = SYS_LOCAL_MODULE_SLOTNO;

            if ( SYS_LOCAL_MODULE_TYPE_IS_SWITCH_PON )
            {
        		/* PON端口与交换芯片物理端口对应表*/
                PonPortToSwPort = PonPortToSwPort_16PONCARD;

        		/* PON端口与交换芯片逻辑端口对应表*/
                PonPortToSwLogPort = PonPortToSwLogPort_16PONCARD;

                if ( 0 == bcm_cpu_port )
                {
                    PonPortToSwReMap(2);
                }
            }
            else
            {
                /* ---防止错误访问越界--- */
                
        		/* PON端口与交换芯片物理端口对应表*/
                PonPortToSwPort = PonPortToSwPort_6900;

        		/* PON端口与交换芯片逻辑端口对应表*/
                PonPortToSwLogPort = PonPortToSwLogPort_6900;
            }
        }
        else
        {
    		OltChassis.PonCardNum    = 12;
    		OltChassis.PonPortMaxNum = 12 * CARD_MAX_PON_PORTNUM;
    		OltChassis.PonCardFirst  = 1;
    		OltChassis.PonCardLast   = 14;
	
    		/* PON端口与交换芯片物理端口对应表*/
            PonPortToSwPort = PonPortToSwPort_6900;

    		/* PON端口与交换芯片逻辑端口对应表*/
            PonPortToSwLogPort = PonPortToSwLogPort_6900;
        }
        OltChassis.PonCardDevNum     = 12;
		OltChassis.PonPortNumPerCard = CARD_MAX_PON_PORTNUM;
        
		OltChassis.EthPortMaxNum = CARD_MAX_PON_PORTNUM * 12;/*MODIFY BY ZHAOZHG FOR 12EPON*/
        
		OltChassis.SwCardFirst = 7;
		OltChassis.SwCardLast  = 8;

        OltChassis.UplinkCardFirst = 1;
		OltChassis.UplinkCardLast  = 14;
		OltChassis.UplinkPortNumPerCard = 4;
        
		OltChassis.TdmCardFirst = 1;
		OltChassis.TdmCardLast  = 14;

		OltChassis.OltCardFirst = 1;
        OltChassis.OltCardLast  = 21;
        
		OltChassis.OltPortCardFirst = 1;
		OltChassis.OltPortCardLast  = 14;
		OltChassis.OltPortPerCardMax = CARD_MAX_PON_PORTNUM;

        if (V2R1_PRODUCT_IS_6900Series(Product_type))
            OltChassis.OltMaxOnuPerPon = 64;
        else
        {        
            OltChassis.OltMaxOnuPerPon = 128;
            /*-begin-根据设备类型设置每pon口允许的最大onu注册数量，武汉需求*/
            /*MaxOnuDefault = 128; */
            /*-end-*/            
        }      
		for(i=0;i<12;i++)
		{
			SlotAttr[PON[i]].SupportedModuleTypeList = V2R1_GFA6900_EPON_MODULE | V2R1_GFA6900_UPLINK_MODULE;
		}

		TDM_FPGA_MAX = 1;
		MAX_E1_PER_FPGA = 8;
		MAX_E1_PORT_NUM = TDM_FPGA_MAX * MAX_E1_PER_FPGA;/*add by shixh@20080725*/
		/* MAX_ONU_NUM = OltChassis.PonPortMaxNum * MAXONUPERPON; */ /*add byshixh@20080805*/

		{
			VOS_StrCpy(CardSlot_s[1],"Pon(slot1)");
			VOS_StrCpy(CardSlot_s[2],"Pon(slot2)");
			VOS_StrCpy(CardSlot_s[3],"Pon(slot3)");
			VOS_StrCpy(CardSlot_s[4],"Pon(slot4)");
			VOS_StrCpy(CardSlot_s[5],"Pon(slot5)");
			VOS_StrCpy(CardSlot_s[6],"Pon(slot6)");
			VOS_StrCpy(CardSlot_s[7],"SW(slot7)");
			VOS_StrCpy(CardSlot_s[8],"SW(slot8)");
			VOS_StrCpy(CardSlot_s[9],"Pon(slot9)");
			VOS_StrCpy(CardSlot_s[10],"Pon(slot10)");
			VOS_StrCpy(CardSlot_s[11],"Pon(slot11)");
			VOS_StrCpy(CardSlot_s[12],"Pon(slot12)");
			VOS_StrCpy(CardSlot_s[13],"Pon(slot13)");
			VOS_StrCpy(CardSlot_s[14],"Pon(slot14)");
			VOS_StrCpy(CardSlot_s[15],"FAN(slot15)");
			VOS_StrCpy(CardSlot_s[16],"FAN(slot16)");			
			VOS_StrCpy(CardSlot_s[17],"FAN(slot17)");			
			VOS_StrCpy(CardSlot_s[18],"PWU(slot18)");
			VOS_StrCpy(CardSlot_s[19],"PWU(slot19)");
			VOS_StrCpy(CardSlot_s[20],"PWU(slot20)");
			VOS_StrCpy(CardSlot_s[21],"PWU(slot21)");			
		}
	
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(Product_type));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(Product_type));
		/*
		SCK = 0;
		SS= 1;
		MOSI = 3;
		MISO = 2;
		*/
		WDI = 6;
    } 
	else if(V2R1_PRODUCT_IS_HM_Series(Product_type))
    {
		FLASH_BASE = 0xfe000000;		

		UPLINK[0]  = 1;
		UPLINK[1]  = 2;
		UPLINK[2]  = 3;
        
		SW[0] = 1;
#if 0
		SW[1] = 2;
		SW[2] = 3;
#endif

        PON[0] = 1;
        PON[1] = 2;
        PON[2] = 3;

        FAN[0] = 4;
        
		PWU[0] = 5;
		PWU[1] = 6;

        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
    		OltChassis.PonCardNum    = 3;
    		OltChassis.PonPortMaxNum = 3 * CARD_MAX_PON_PORTNUM;
    		OltChassis.PonCardFirst  = 1;
    		OltChassis.PonCardLast   = 3;
        }
        else
        {
    		OltChassis.PonCardNum    = 1;
    		OltChassis.PonPortMaxNum = CARD_MAX_PON_PORTNUM; 
    		OltChassis.PonCardFirst  = SYS_LOCAL_MODULE_SLOTNO;
    		OltChassis.PonCardLast   = SYS_LOCAL_MODULE_SLOTNO;
        }

        OltChassis.PonCardDevNum     = 3;
		OltChassis.PonPortNumPerCard = CARD_MAX_PON_PORTNUM;
        
		OltChassis.EthPortMaxNum = CARD_MAX_PON_PORTNUM * 3;/*MODIFY BY ZHAOZHG FOR 12EPON*/
        
		OltChassis.SwCardFirst = 1;
		OltChassis.SwCardLast  = 1;

        OltChassis.UplinkCardFirst = 1;
		OltChassis.UplinkCardLast  = 3;
		OltChassis.UplinkPortNumPerCard = 4;
        
		OltChassis.TdmCardFirst = 0;
		OltChassis.TdmCardLast  = 0;

		OltChassis.OltCardFirst = 1;
        OltChassis.OltCardLast  = 6;
        
		OltChassis.OltPortCardFirst = 1;
		OltChassis.OltPortCardLast  = 3;
		OltChassis.OltPortPerCardMax = CARD_MAX_PON_PORTNUM;

            OltChassis.OltMaxOnuPerPon = 64;
            
		for(i=0;i<3;i++)
		{
			SlotAttr[PON[i]].SupportedModuleTypeList = V2R1_GFA6900_EPON_MODULE | V2R1_GFA6900_UPLINK_MODULE;
		}

		TDM_FPGA_MAX = 0;
		MAX_E1_PER_FPGA = 0;
		MAX_E1_PORT_NUM = TDM_FPGA_MAX * MAX_E1_PER_FPGA;/*add by shixh@20080725*/
		/* MAX_ONU_NUM = OltChassis.PonPortMaxNum * MAXONUPERPON; *//*add byshixh@20080805*/

		{
			VOS_StrCpy(CardSlot_s[1],"Pon(slot1)");
			VOS_StrCpy(CardSlot_s[2],"Pon(slot2)");
			VOS_StrCpy(CardSlot_s[3],"Pon(slot3)");
			VOS_StrCpy(CardSlot_s[4],"FAN(slot4)");			
			VOS_StrCpy(CardSlot_s[5],"PWU(slot5)");
			VOS_StrCpy(CardSlot_s[6],"PWU(slot6)");
		}
	
		/* PON端口与交换芯片物理端口对应表*/
        PonPortToSwPort = PonPortToSwPort_16PONCARD;

		/* PON端口与交换芯片逻辑端口对应表*/
        PonPortToSwLogPort = PonPortToSwLogPort_16PONCARD;

        if ( 0 == bcm_cpu_port )
        {
            PonPortToSwReMap(2);
        }
	
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(Product_type));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(Product_type));
		/*
		SCK = 0;
		SS= 1;
		MOSI = 3;
		MISO = 2;
		*/
		WDI = 6;
    }   
	else if(V2R1_PRODUCT_IS_HS_Series(Product_type))
    {
		FLASH_BASE = 0xfe000000;		

		SW[0] = 1;

        PON[0] = 1;

        FAN[0] = 2;
        
		PWU[0] = 3;
		PWU[1] = 4;

        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
        {
    		OltChassis.PonCardNum    = 1;
    		OltChassis.PonPortMaxNum = CARD_MAX_PON_PORTNUM; 
    		OltChassis.PonCardFirst  = SYS_LOCAL_MODULE_SLOTNO;
    		OltChassis.PonCardLast   = SYS_LOCAL_MODULE_SLOTNO;
        }
        else
        {
            VOS_ASSERT(0);
        }

        OltChassis.PonCardDevNum     = 1;
		OltChassis.PonPortNumPerCard = CARD_MAX_PON_PORTNUM;
        
		OltChassis.EthPortMaxNum = CARD_MAX_PON_PORTNUM * 1;/*MODIFY BY ZHAOZHG FOR 12EPON*/
        
		OltChassis.SwCardFirst = 1;
		OltChassis.SwCardLast  = 1;

        OltChassis.UplinkCardFirst = 1;
		OltChassis.UplinkCardLast  = 1;
		OltChassis.UplinkPortNumPerCard = 4;
        
		OltChassis.TdmCardFirst = 0;
		OltChassis.TdmCardLast  = 0;

		OltChassis.OltCardFirst = 1;
        OltChassis.OltCardLast  = 4;
        
		OltChassis.OltPortCardFirst = 1;
		OltChassis.OltPortCardLast  = 1;
		OltChassis.OltPortPerCardMax = CARD_MAX_PON_PORTNUM;

		if(V2R1_PRODUCT_IS_8100Series(Product_type))
			OltChassis.OltMaxOnuPerPon = 128;
		else
            OltChassis.OltMaxOnuPerPon = 64;
		for(i=0;i<1;i++)
		{
			SlotAttr[PON[i]].SupportedModuleTypeList = V2R1_GFA6900_EPON_MODULE;
		}

		TDM_FPGA_MAX = 0;
		MAX_E1_PER_FPGA = 0;
		MAX_E1_PORT_NUM = TDM_FPGA_MAX * MAX_E1_PER_FPGA;/*add by shixh@20080725*/
		/* MAX_ONU_NUM = OltChassis.PonPortMaxNum * MAXONUPERPON; *//*add byshixh@20080805*/

		{
			VOS_StrCpy(CardSlot_s[1],"Pon(slot1)");
			VOS_StrCpy(CardSlot_s[4],"FAN(slot2)");			
			VOS_StrCpy(CardSlot_s[5],"PWU(slot3)");
			VOS_StrCpy(CardSlot_s[6],"PWU(slot4)");
		}
	
		/* PON端口与交换芯片物理端口对应表*/
        PonPortToSwPort = PonPortToSwPort_16PONCARD;

		/* PON端口与交换芯片逻辑端口对应表*/
        PonPortToSwLogPort = PonPortToSwLogPort_16PONCARD;

        if ( 0 == bcm_cpu_port )
        {
            PonPortToSwReMap(2);
        }
	
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(Product_type));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(Product_type));
		/*
		SCK = 0;
		SS= 1;
		MOSI = 3;
		MISO = 2;
		*/
		WDI = 6;
    }   
    else
    {
        VOS_ASSERT(0);
    }

    if (MAXONUPERPON < OltChassis.OltMaxOnuPerPon)
    {
        VOS_ASSERT(0);
        sys_console_printf("%s:%d Please change the MARCRO  'MAXONUPERPON_CONS'.\r\n", __FILE__, __LINE__);
    }

    if (0 >= OltChassis.OltMaxOnuPerPon)
    {
        VOS_ASSERT(0);
        sys_console_printf("%s:%d Please change the OltChassis.OltMaxOnuPerPon'.\r\n", __FILE__, __LINE__);
    }
	return( ROK );
}

extern int manageSwChipType;
void InitSPI(void)
{
    if( PRODUCT_IS_6100Series(SYS_PRODUCT_TYPE) )
	{
	       sys_console_printf("\r\nInitializing SPI interface ...");
                   SCK = 8;
                   SS= 9;
                   MOSI = 14;
                   MISO = 15;
                  manageSwChipType=1; 
	       sys_console_printf("OK\r\n");
	} 
    else if ( PRODUCT_IS_EPON(SYS_PRODUCT_TYPE) )
    {
	       sys_console_printf("\r\nInitializing SPI interface ...");
		SCK = 0;
		SS= 1;
		MOSI = 3;
		MISO = 2;
	       sys_console_printf("OK\r\n");
    }
    
	return;
}

int  SlotCardIsNullBoard(int CardIndex)
{
    if ( __SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW )
    {
        return(ROK);
    }
    
    return(RERROR );
}

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为交换板(对GFA6700,SW板. 对GFA6100,SW板)
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 本板为SW板
                        RERROR -- 本板不是SW板
*********************************************************************************************/
int  SlotCardIsSwBoard(int CardIndex)
{
	int Product_type = GetOltType();
	
	if(Product_type == V2R1_OLT_GFA6700)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SW )
			return(ROK);
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_MAIN )
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA6900)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_SW )
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA8000)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8000_SW )
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA8100)
	{
		if(( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8100_16EPONB0)||( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8100_16GPONB0))
			return(ROK);
	}
    
	return(RERROR );
}

/* begin: added by jianght 20090708 */
/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为交换板(对GFA6700,SW板. 对GFA6100,SW板)，是带有10G以太接口的交换板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 本板为SW板
                        RERROR -- 本板不是SW板
*********************************************************************************************/
int  SlotCardIsSwGTBoard(int CardIndex)
{
	int Product_type = GetOltType();
	
	if(Product_type == V2R1_OLT_GFA6700)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SWTG )
			return(ROK);
	}
	else if(Product_type == V2R1_OLT_GFA6900)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE )
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA8000)
	{
		if( __SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE ) /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
			return(ROK);
	}	

	return(RERROR );
}
/* end: added by jianght 20090708 */

int SlotCardIsUplinkBoard(int CardIndex)
{
	int Product_type=GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
	{
#ifdef _SUPPORT_8GE_6700_
		/* wangysh mod 20110307 6700 support slot2 uplink */
		if(CardIndex != 1 && !SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version < 2*/ ) 
		    return( RERROR);
		if(SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version == 2*/ && CardIndex != 1 && CardIndex != 2 )
			return( RERROR);
#endif
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEM)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEO)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GET))
			return (ROK);
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if(CardIndex != 1 ) return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEM ) )
			return( ROK );
	}
	else if(Product_type == V2R1_OLT_GFA6900)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_GE))
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA8000)  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_GE) ||
			(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8000_4XET)||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA8000_8XETA1))
			return(ROK);
	}	

	return( RERROR );
}

int  SlotCardMayBeUplinkBoardByVty(int CardIndex, struct vty *vty)
{
	int Product_type=GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
	{
#ifdef _SUPPORT_8GE_6700_
		/* wangysh mod 20110307 6700 support slot2 uplink */
		if(CardIndex != 1 && !SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version < 2*/ ) 
		    return( RERROR);
		if(SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version == 2*/ && CardIndex != 1 && CardIndex != 2 )
			return( RERROR);
#endif
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEM)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEO)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GET))
			return (ROK);
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if(CardIndex != 1 ) return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_GEM) )
			return( ROK );
	}
	else if(Product_type == V2R1_OLT_GFA6900)
	{
	    if( (7 == CardIndex) || (8 == CardIndex) )
			return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_GE))
			return(ROK);
	}
	else if(Product_type == V2R1_OLT_GFA6900M || Product_type == V2R1_OLT_GFA6900S)
	{
	    if( SYS_LOCAL_MODULE_SLOTNO == CardIndex)
			return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE))
			return(ROK);
	}	
	else if(Product_type == V2R1_OLT_GFA8000)
	{
	    if( (7 == CardIndex) || (8 == CardIndex) )
			return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE)
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_GE))
			return(ROK);
	}
	else if(Product_type == V2R1_OLT_GFA8000M || Product_type == V2R1_OLT_GFA8000S)
	{
	    if( SYS_LOCAL_MODULE_SLOTNO == CardIndex)
			return( RERROR);
		if((__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW)  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
            ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6900_GEM_10GE))
			return(ROK);
	}	

	return( RERROR );
}

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     带有PON 芯片(对GFA6700,PON板. 对GFA6100, PON板和SW板)
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 板上有PON 芯片; 对GFA6700, 即为PON 板; 对GFA6100, 为SW板或PON板
                        RERROR -- 板上没有PON 芯片,或没有插板
*********************************************************************************************/
int  SlotCardIsPonBoard(int CardIndex)
{
	int Product_type = GetOltType();

    if(CardIndex>PONCARD_LAST || CardIndex<PONCARD_FIRST)
        return RERROR;
    
	if(Product_type == V2R1_OLT_GFA6700)
	{
#ifdef _SUPPORT_8GE_6700_
		if( (CardIndex == 4) && SYS_CHASSIS_IS_V2_8GE)
		    return ( RERROR );
#endif
		if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_EPON )
			return( ROK );
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_MAIN ) || (__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_EPON ))
			return( ROK );
	}
	else if(V2R1_PRODUCT_IS_6900Series(Product_type))
	{		
		if(SYS_MODULE_IS_6900_EPON(CardIndex))
			return( ROK );
	}
	else if(V2R1_PRODUCT_IS_8000Series(Product_type))
	{		
	    /*modi by luh@2015-04-09,8000设备中应该包含所有的cpu pon板*/
		if(SYS_MODULE_IS_CPU_PON(CardIndex)) 
			return( ROK );
	}
	else if(V2R1_PRODUCT_IS_8100Series(Product_type))
	{		
		if(SYS_MODULE_IS_8100_PON(CardIndex))
			return( ROK );
	}
    
	return( RERROR );
}

 /* modified by xieshl 20120413, PON口数是根据板卡类型推算出的，在快速板卡复位时，
   板卡类型可能存在unknown的情况，此时没有PON口，就会发生PON板无法启动和
   加载的情况，属于小概率事件*/
int  GetSlotCardPonPortRange(int module_type, int CardIndex)
{
    int iPortNum = 0;
    int Product_type = GetOltType();

    if ( module_type <= 0 )
    {
        module_type = __SYS_MODULE_TYPE__(CardIndex);
    }

    if(Product_type == V2R1_OLT_GFA6700)
    {
#ifdef _SUPPORT_8GE_6700_
        if( (CardIndex == 4) && SYS_CHASSIS_IS_V2_8GE)
            return ( 0 );
#endif
		if(module_type == MODULE_E_GFA_EPON )
            return( 4 );
    }
    else if(Product_type == V2R1_OLT_GFA6100)
    {
        if((module_type == MODULE_E_GFA6100_MAIN ) || (module_type == MODULE_E_GFA6100_EPON ))
            return( 2 );
    }
    else if(V2R1_PRODUCT_IS_6900Series(Product_type))
    {
        switch (module_type)
        {
            case MODULE_E_GFA6900_4EPON:
                iPortNum = 4;
                break;
            case MODULE_E_GFA6900_8EPON:
            case MODULE_E_GFA6900_4EPON_4GE:
                iPortNum = 8;
                break;
            case MODULE_E_GFA6900_12EPON:
			case MODULE_E_GFA6900_12EPON_M:	
            case MODULE_E_GFA6900_12EPONB0:
            case MODULE_E_GFA6900_12EPONB0_M:
            case MODULE_E_GFA6900_16EPONB1:
            case MODULE_E_GFA6900_16EPONB1_M:
                iPortNum = 16;
                break;
#if defined(_EPON_10G_PMC_SUPPORT_)            
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            case MODULE_E_GFA6900_10G_EPON:
            case MODULE_E_GFA6900_10G_EPON_M:
                iPortNum = 6;
				/*sys_console_printf("GetSlotCardPonPortRange,the module_type is MODULE_E_GFA6900_10G_EPON\r\n");*/
                break;
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
            default:
                VOS_ASSERT(0);
        }   
        return( iPortNum );
    }
    else if(V2R1_PRODUCT_IS_8000Series(Product_type))
    {
        switch (module_type)
        {
            case MODULE_E_GFA6900_4EPON:/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
                iPortNum = 4;
                break;
            case MODULE_E_GFA8000_4EPON_4GE:
            case MODULE_E_GFA8000_10G_8EPON:
            case MODULE_E_GFA8000_10G_8EPON_M:
                iPortNum = 8;
                break;
            case MODULE_E_GFA6900_12EPON:/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
			case MODULE_E_GFA8000_12EPON_M:	
            case MODULE_E_GFA8000_12EPONB0:
            case MODULE_E_GFA8000_12EPONB0_M:
            case MODULE_E_GFA6900_16EPONB1:  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
            case MODULE_E_GFA8000_16EPONB1_M:
			case MODULE_E_GFA8000_16GPONB0:
                iPortNum = 16;
                break;
            default:
                VOS_ASSERT(0);
        }   
        return( iPortNum );
    }
	else if(V2R1_PRODUCT_IS_8100Series(Product_type))
	{
		switch (module_type)
        {
            case MODULE_E_GFA8100_16EPONB0:
			case MODULE_E_GFA8100_16GPONB0:
                iPortNum = 16;
                break;
			default:
				VOS_ASSERT(0);
				break;
		}
		return( iPortNum );
	}
    
    return( 0 );
}

int  GetUplinkPortRange(int module_type)
{
    int iPortNum = 0;

    switch(module_type)
    {
	    case MODULE_E_GFA6900_GEM_GE:
	    case MODULE_E_GFA8000_4XET:
	    case MODULE_E_GFA_GEM:
	    case MODULE_E_GFA6100_GEM:
			iPortNum = 4;
			break;
	    case MODULE_E_GFA6900_GEM_10GE:
			iPortNum = 5;
			break;
	    case MODULE_E_GFA8000_8XET:
	    		iPortNum = 8;
			break;
    }

    return iPortNum;
}

int GetSlotCardPonPortNum(int module_type, int CardIndex)
{
    if ( module_type <= 0 )
    {
        module_type = __SYS_MODULE_TYPE__(CardIndex);
    }

    if ( SYS_MODULE_TYPE_IS_12EPON(module_type) )
    {
        return 12;
    }
	else if ( SYS_MODULE_TYPE_IS_4EPON_4GE(module_type) )
    {
        return 4;
    }
#if defined(_EPON_10G_PMC_SUPPORT_)            
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	else if ( SYS_MODULE_TYPE_IS_6900_10G_EPON(module_type) )
    {
        return 1;
    }
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif

    return GetSlotCardPonPortRange(module_type, CardIndex);
}

int GetSlotPonPortPhyRange(int CardIndex, short int *sOltStartPort, short int *sOltEndPort)
{
    int iPortNum = 0;
	int Product_type = GetOltType();

    *sOltStartPort = 1;
    switch(Product_type)
    {
        case V2R1_OLT_GFA6700:
            iPortNum = 4;
            break;
        case V2R1_OLT_GFA6100:
            iPortNum = 2;
            break;
        case V2R1_OLT_GFA6900:
		case V2R1_OLT_GFA6900M:
		case V2R1_OLT_GFA6900S:
        case V2R1_OLT_GFA8000:
		case V2R1_OLT_GFA8000M:
		case V2R1_OLT_GFA8000S:
            if ( SYS_MODULE_IS_12EPON(CardIndex)  )
            {
                iPortNum = 12;
                *sOltStartPort = 5;
            }
			else if ( SYS_MODULE_IS_4EPON_4GE(CardIndex) )
            {
                iPortNum = 4;
                *sOltStartPort = 5;
            }
#if defined(_EPON_10G_PMC_SUPPORT_)            
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			else if ( SYS_MODULE_IS_6900_10G_EPON(CardIndex) )
            {
                iPortNum = 1;
                *sOltStartPort = 6;
            }
			/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
            else
            {
                iPortNum = GetSlotCardPonPortRange( 0, CardIndex );
            }
            break;
		case V2R1_OLT_GFA8100:
			if(SYS_MODULE_IS_8100_PON(CardIndex))
			{
				iPortNum = GetSlotCardPonPortRange( 0, CardIndex );
			}
			break;
        default:
            VOS_ASSERT(0);
    }
    *sOltEndPort = *sOltStartPort + iPortNum - 1;
        
    return iPortNum;
}

int GetLocalSlotPonPortRange(short int *sOltStartPort, short int *sOltEndPort)
{
    return GetSlotPonPortPhyRange(SYS_LOCAL_MODULE_SLOTNO, sOltStartPort, sOltEndPort);
}

int  GetSlotCardPonChipNum(int module_type, int CardIndex)
{
    int iPonChipNum = 0;

    if ( module_type <= 0 )
    {
        module_type = __SYS_MODULE_TYPE__(CardIndex);
    }

    iPonChipNum = GetSlotCardPonPortNum(module_type, CardIndex);
    if ( SYS_MODULE_TYPE_IS_TK_PONCARD_MANAGER(module_type) )
    {
        if ( SYS_MODULE_TYPE_IS_BCM_PONCARD_MANAGER(module_type) )
        {
            iPonChipNum = iPonChipNum >> 3;
        }
        else
        {
            iPonChipNum = iPonChipNum >> 2;
        }
    }

    return iPonChipNum;
}

int GetLocalSlotPonChipNum()
{
    return GetSlotCardPonChipNum(0, SYS_LOCAL_MODULE_SLOTNO);
}

int  GetPonChipPonPorts(short int PonPortIdx, short int PonPortIdxs[MAXOLTPERPONCHIP])
{
    int iPonChipPonNum;
    int PonChipType;

    iPonChipPonNum = 1;
    PonPortIdxs[0] = PonPortIdx;

    PonChipType = V2R1_GetPonchipType(PonPortIdx);
    if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
        switch ( PonChipType )
        {
            case PONCHIP_BCM55538:
                iPonChipPonNum = 8;

                PonPortIdxs[0] = PonPortIdx & 0xFFF8;
                PonPortIdxs[1] = PonPortIdxs[0] + 1;
                PonPortIdxs[2] = PonPortIdxs[1] + 1;
                PonPortIdxs[3] = PonPortIdxs[2] + 1;
                PonPortIdxs[4] = PonPortIdxs[3] + 1;
                PonPortIdxs[5] = PonPortIdxs[4] + 1;
                PonPortIdxs[6] = PonPortIdxs[5] + 1;
                PonPortIdxs[7] = PonPortIdxs[6] + 1;

                break;
            case PONCHIP_BCM55524:
                iPonChipPonNum = 4;

                PonPortIdxs[0] = PonPortIdx & 0xFFFC;
                PonPortIdxs[1] = PonPortIdxs[0] + 1;
                PonPortIdxs[2] = PonPortIdxs[1] + 1;
                PonPortIdxs[3] = PonPortIdxs[2] + 1;

                break;
            case PONCHIP_TK3723:
                iPonChipPonNum = 2;

                PonPortIdxs[0] = PonPortIdx & 0xFFFE;
                PonPortIdxs[1] = PonPortIdxs[0] + 1;

                break;
            default:
                VOS_ASSERT(0);
        }
    }
	else if(OLT_PONCHIP_ISGPON(PonChipType))
	{
		
        iPonChipPonNum = 16;

        PonPortIdxs[0] = PonPortIdx & 0xFFF0;
        PonPortIdxs[1] = PonPortIdxs[0] + 1;
        PonPortIdxs[2] = PonPortIdxs[1] + 1;
        PonPortIdxs[3] = PonPortIdxs[2] + 1;
        PonPortIdxs[4] = PonPortIdxs[3] + 1;
        PonPortIdxs[5] = PonPortIdxs[4] + 1;
        PonPortIdxs[6] = PonPortIdxs[5] + 1;
        PonPortIdxs[7] = PonPortIdxs[6] + 1;
		PonPortIdxs[8] = PonPortIdxs[7] + 1;
        PonPortIdxs[9] = PonPortIdxs[8] + 1;
        PonPortIdxs[10] = PonPortIdxs[9] + 1;
        PonPortIdxs[11] = PonPortIdxs[10] + 1;
        PonPortIdxs[12] = PonPortIdxs[11] + 1;
        PonPortIdxs[13] = PonPortIdxs[12] + 1;
        PonPortIdxs[14] = PonPortIdxs[13] + 1;
		PonPortIdxs[15] = PonPortIdxs[14] + 1;

               
	}

    return iPonChipPonNum;
}


#if 1
#define PON_INT_BYEDGE      0
#define PON_INT_BYUPEDGE    1
#define PON_INT_BYDOWNEDGE  0

#define PON_INT_BYLEVEL     1
#define PON_INT_BYUPLEVEL   1
#define PON_INT_BYDOWNLEVEL 0

#define PON_INT_ISNEEDRESET  1
#define PON_INT_NOTNEEDRESET 0

static int s_aiIntLocks[16];
static int s_iCPLDIntTriggerType = 0;
static int s_iCPLDIntTriggerValue = 0;
int s_iCPLDIntIsNeedReset = 0;

static volatile UCHAR *s_CPLDRegPonIntSwitch[2] = {0x21, 0x23};
static volatile UCHAR *s_CPLDRegPonIntState[2]  = {0x20, 0x22};

static int s_CPLDRegPonBitRange[2] = {8, 8};
static int s_CPLDRegPonByteBase = 0;
static int s_CPLDRegPonByteNum  = 16;
static short int s_aCPLDRegAllOltIds[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

static int (* s_PF_GetLocalOltIdByInt)(int iIntIdx, short int aOltIds[16]);


int GetLocalOltIntRange(unsigned char aIntIds[16])
{
    int IntNum = 0;
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

    if ( SYS_LOCAL_MODULE_TYPE_IS_CPU_PON )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_4EPON_0GE )
        {
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
            aIntIds[0] = pabIntIrqs[0];    
            aIntIds[1] = pabIntIrqs[1];    
            aIntIds[2] = pabIntIrqs[2];   
            aIntIds[3] = pabIntIrqs[3];    
#else
            aIntIds[0] = INT_VEC_EXT_IRQ_0;    
            aIntIds[1] = INT_VEC_EXT_IRQ_1;    
            aIntIds[2] = INT_VEC_EXT_IRQ_2;    
            aIntIds[3] = INT_VEC_EXT_IRQ_3;    
#endif            

            IntNum = 4;
        }
        else
        {
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
            aIntIds[0] = pabIntIrqs[0];    
#else
            aIntIds[0] = INT_VEC_EXT_IRQ_0;    
#endif            
            
            IntNum = 1;
        }
    }
    
    return IntNum;
}

static int GetLocalOltIdBySelfInt(int iIntIdx, short int aOltIds[16])
{
    aOltIds[0] = iIntIdx;
    return 1;
}

static int GetLocalOltIdBySingleIntV1(int iIntIdx, short int aOltIds[16])
{
    static int a[2] = {8, 4};
    int OltNum = 0;
    int i, j, n, m;
    UCHAR PonIntRegs[2];

    ReadCPLDReg((volatile UCHAR *)06, &PonIntRegs[0]);
    ReadCPLDReg((volatile UCHAR *)07, &PonIntRegs[1]);

    if ( 0xFF == PonIntRegs[0] )
    {
        n = 1;
    }
    else
    {
        n = 0;
    }

    if ( 0xF == (0xF & PonIntRegs[1]) )
    {
        m = 1;
    }
    else
    {
        m = 2;
    }

    for (; n<m; ++n)
    {
        j = 4 + (n << 3);
        for (i=0; i<a[n]; ++i)
        {
            if ( 0 == (PonIntRegs[n] & (1 << i)) )
            {
                aOltIds[OltNum++] = j + i;
            }
        }
    }

    /* logMsg("\r\n GetLocalOltIdBySingleIntV1(%d, %d, %d, %d)", iIntIdx, (int)PonIntRegs[0], (int)PonIntRegs[1], OltNum,0,0); */
#if 1
    if ( 0 == OltNum )
    {
        static short int aAllOltIds[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    
        /* 只有中断，而CPLD并无指明具体OLT */
        OltNum = -12;
        VOS_MemCpy(aOltIds, aAllOltIds, 12);
    }
#endif

    return OltNum;
}

static int GetLocalOltIdBySingleIntV2_16(int iIntIdx, short int aOltIds[16])
{
    int OltNum = 0;
    int i, b, r, n, m;
    UCHAR PonIntRegs[2];
    UCHAR PonIntVals[2];

    ReadCPLDReg(s_CPLDRegPonIntState[0], &PonIntRegs[0]);
    ReadCPLDReg(s_CPLDRegPonIntState[1], &PonIntRegs[1]);

    if ( 0 == PonIntRegs[0] )
    {
        n = 1;
    }
    else
    {
        n = 0;
    }

    if ( 0 == PonIntRegs[1] )
    {
        m = 1;
    }
    else
    {
        m = 2;
    }

    for (; n<m; ++n)
    {
        b = s_CPLDRegPonByteBase + (n << 3);
        r = s_CPLDRegPonBitRange[n];
        
        for (i=0; i<r; ++i)
        {
            if ( PonIntRegs[n] & (1 << i) )
            {
                aOltIds[OltNum++] = b + i;
            }
        }
    
        /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x55 );*/
        CPLD_WRITE_LOCK(CPLD_PROTECT_6900M);
#if 0
        ReadCPLDReg ( s_CPLDRegPonIntState[n], &PonIntVals[n] );
        WriteCPLDReg( s_CPLDRegPonIntState[n], PonIntVals[n] & (~PonIntRegs[n]) );
#else
        WriteCPLDReg( s_CPLDRegPonIntState[n], 0 );
#endif
        /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x0 );*/
        CPLD_WRITE_UNLOCK(CPLD_PROTECT_6900M);
    }

    /* logMsg("\r\n GetLocalOltIdBySingleIntV2_16(%d, %d, %d, %d, %d, %d)", iIntIdx, (int)PonIntRegs[0], (int)PonIntRegs[1], OltNum, aOltIds[0], aOltIds[1]); */
#if 1
    if ( 0 == OltNum )
    {
        /* 只有中断，而CPLD并无指明具体OLT */
        OltNum = -s_CPLDRegPonByteNum;
        VOS_MemCpy(aOltIds, &s_aCPLDRegAllOltIds[s_CPLDRegPonByteBase], s_CPLDRegPonByteNum);
    }
#endif

    return OltNum;
}

static int GetLocalOltIdBySingleIntV1_16(int iIntIdx, short int aOltIds[16])
{
    int OltNum = 0;
    int i, n;
    UCHAR PonIntRegs;
    UCHAR PonIntVals;

    ReadCPLDReg(s_CPLDRegPonIntState[0], &PonIntRegs);

    if ( 0 != PonIntRegs )
    {
        n = s_CPLDRegPonByteNum >> 1;
        for (i = 0; i < n; ++i)
        {
            if ( PonIntRegs & (1 << i) )
            {
                aOltIds[OltNum++] = s_CPLDRegPonByteBase + (i << 2);
                aOltIds[OltNum++] = aOltIds[OltNum-1] + 2;
            }
        }
            
        /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x55 );*/
        CPLD_WRITE_LOCK(CPLD_PROTECT_6900M);
#if 0
        ReadCPLDReg ( s_CPLDRegPonIntState[0], &PonIntVals );
        WriteCPLDReg( s_CPLDRegPonIntState[0], PonIntVals & (~PonIntRegs) );
#else
        WriteCPLDReg( s_CPLDRegPonIntState[0], 0 );
#endif
        /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x0 );*/
        CPLD_WRITE_UNLOCK(CPLD_PROTECT_6900M);
    }

    /* logMsg("GetLocalOltIdBySingleIntV1_16(%d, %d, %d, %d, %d, %d)\r\n", iIntIdx, (int)PonIntRegs, OltNum, aOltIds[0], aOltIds[1], aOltIds[2]); */
#if 1
    if ( 0 == OltNum )
    {
        /* 只有中断，而CPLD并无指明具体OLT */
        OltNum = -s_CPLDRegPonByteNum;
        VOS_MemCpy(aOltIds, &s_aCPLDRegAllOltIds[s_CPLDRegPonByteBase], s_CPLDRegPonByteNum);
    }
#endif

    return OltNum;
}

int GetLocalOltIdByInt(int iIntIdx, short int aOltIds[16])
{
    return (*s_PF_GetLocalOltIdByInt)(iIntIdx, aOltIds);
}


#ifndef USE_BSP_PPC405
/* 配置cpu侧PON中断模式*/
#define uic_reset(bit_offset, reg) \
do{\
   reg &= ~(1<<(31-bit_offset));\
}\
while(0)

#define uic_set(bit_offset, reg) \
do{\
   reg |= (1<<(31-bit_offset));\
}\
while(0)
#endif

#ifndef USE_BSP_1010
/* 配置cpu侧PON中断模式*/
#define uic_reset(bit_offset, reg) \
do{\
   reg &= ~(1<<(31-bit_offset));\
}\
while(0)

#define uic_set(bit_offset, reg) \
do{\
   reg |= (1<<(31-bit_offset));\
}\
while(0)
#endif


extern int intLock (void);
extern void intUnlock( int lockKey );

int debug_stub()
{
    static int x = 0;

    sys_console_printf("\r\n debug_stub(%d).\r\n", x);

    vos_print_my_taskstack();

    VOS_TaskDelay(10);

    return ++x;
}



#if 1
int test_cpld(int cpld_reg, int iTickNum, int iTestNum)
{
    int ReadNum;
    int WriteNum;
    unsigned char bCpldValue;

    ReadNum  = 0;
    WriteNum = 0;
    do{
        do{
            WriteCPLDReg( (volatile UCHAR*)0x0B, 0x55 );
            WriteCPLDReg( (volatile UCHAR*)cpld_reg, 0 );
            WriteCPLDReg( (volatile UCHAR*)0x0B, 0 );
            ReadCPLDReg ( (volatile UCHAR*)cpld_reg, &bCpldValue );
            ++WriteNum;
        } while( (0 != bCpldValue) && (WriteNum < iTestNum) );
        sys_console_printf("\r\ntest_cpld(0x%x, %d, %d)'s writenum(%d).\r\n", cpld_reg, iTickNum, iTestNum, WriteNum);

        VOS_TaskDelay(iTickNum);

        ReadCPLDReg ( (volatile UCHAR*)cpld_reg, &bCpldValue );

        if ( ++ReadNum == iTestNum )
        {
            break;
        }
    }while( 0 != bCpldValue );
    sys_console_printf("\r\ntest_cpld(0x%x, %d, %d)'s readnum(%d).\r\n", cpld_reg, iTickNum, iTestNum, ReadNum);
    
    return ReadNum; 
}

int test_cpld2(int cpld_reg, int iWriteNum, int iReadNum)
{
    int ReadNum;
    int WriteNum;
    unsigned char bCpldValue;

    ReadNum  = 0;
    do{
        WriteNum = iWriteNum + ReadNum;
        do{
            WriteCPLDReg( (volatile UCHAR*)0x0B, 0x55 );
            WriteCPLDReg( (volatile UCHAR*)cpld_reg, 0 );
            WriteCPLDReg( (volatile UCHAR*)0x0B, 0 );
        } while( 0 < --WriteNum );
        sys_console_printf("\r\ntest_cpld2(0x%x, %d, %d)'s writenum(%d).\r\n", cpld_reg, iWriteNum, iReadNum, iWriteNum);

        VOS_TaskDelay(1);

        ReadCPLDReg ( (volatile UCHAR*)cpld_reg, &bCpldValue );

        if ( ++ReadNum == iReadNum )
        {
            break;
        }
    }while( 0 != bCpldValue );
    sys_console_printf("\r\ntest_cpld2(0x%x, %d, %d)'s readnum(%d).\r\n", cpld_reg, iWriteNum, iReadNum, ReadNum);
    
    return ReadNum; 
}
#endif


void pon_int_config(void)
{
    UINT32 uic0_tr,uic0_cr,uic0_pr;
    UINT32 uiIntReg;
    int lockKey;
    int i, IntNum;
    unsigned char aIntIds[16];

    VOS_TaskLock();
    lockKey = intLock();

    /* 12PON板不能使用电平触发，因为复位一个PON会造成中断低电平触发，严重影响其它PON的中断 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_SWITCH_PON )
    {
        unsigned char bCPLDVer = 0;
        
        /* 根据CPLD版本，采用对应的触发方式 */
        s_iCPLDIntTriggerType  = PON_INT_BYEDGE;
        s_iCPLDIntTriggerValue = PON_INT_BYDOWNEDGE;
        s_iCPLDIntIsNeedReset  = PON_INT_ISNEEDRESET;

        if ( (MODULE_E_GFA6900_12EPON == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO))
            || (MODULE_E_GFA6900_12EPON_M == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO)) )
        {
            ReadCPLDReg((volatile UCHAR*)0, &bCPLDVer);
            
            if ( bCPLDVer >= 3 )
            {
                bCPLDVer = 5;
            }
            else
            {
                ReadCPLDReg((volatile UCHAR*)1, &bCPLDVer);
            }
        }
        else
        {
            bCPLDVer = 5;
        }

        if ( 4 <= bCPLDVer )
        {
            if ( 4 < bCPLDVer )
            {
                s_CPLDRegPonIntState[0] = 0x20;
                s_CPLDRegPonIntState[1] = 0x22;

                s_CPLDRegPonIntSwitch[0] = 0x21;
                s_CPLDRegPonIntSwitch[1] = 0x23;
            }
            else
            {
                /* v4的0x1f总是被人不断地写为0x55 */
                s_CPLDRegPonIntState[0] = 0x1f;
                s_CPLDRegPonIntState[1] = 0x21;

                s_CPLDRegPonIntSwitch[0] = 0x20;
                s_CPLDRegPonIntSwitch[1] = 0x22;
            }

            /*WriteCPLDReg( (volatile UCHAR*)0x0b, 0x55 );*/
            CPLD_WRITE_LOCK(CPLD_PROTECT_6900M);
            WriteCPLDReg( (volatile UCHAR*)s_CPLDRegPonIntState[0], 0 );
            WriteCPLDReg( (volatile UCHAR*)s_CPLDRegPonIntState[1], 0 );
            /*WriteCPLDReg( (volatile UCHAR*)0x0b, 0x0 );*/
            CPLD_WRITE_UNLOCK(CPLD_PROTECT_6900M);

            {
            	short int olt_port_start;
            	short int olt_port_end;
                
                s_CPLDRegPonByteNum  = GetLocalSlotPonPortRange(&olt_port_start, &olt_port_end);
                s_CPLDRegPonByteBase = olt_port_start - 1;
                
                VOS_ASSERT(0 <= s_CPLDRegPonByteBase);
                VOS_ASSERT(0 < s_CPLDRegPonByteNum);
                VOS_ASSERT(s_CPLDRegPonByteBase + s_CPLDRegPonByteNum <= 16);
            }

            if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
            {
                s_CPLDRegPonByteNum >>= 1;
                IntNum = s_CPLDRegPonByteBase + s_CPLDRegPonByteNum - 1;
            
                for ( i = s_CPLDRegPonByteBase; i < IntNum; i++ )
                {
                    s_aCPLDRegAllOltIds[i+1] = s_aCPLDRegAllOltIds[i] + 2; 
                }
            
                s_PF_GetLocalOltIdByInt = GetLocalOltIdBySingleIntV1_16;

                /* BCM PON中断，暂时不支持 */
#ifndef BCM_PON_INT_SUPPORT
                s_iCPLDIntIsNeedReset = PON_INT_NOTNEEDRESET;
#endif
            }
            else
            {
                switch ( s_CPLDRegPonByteNum )
                {
                    case 4:
                        s_CPLDRegPonBitRange[0] = 4;
                        s_CPLDRegPonBitRange[1] = 0;
                        break;
                    case 8:
                        s_CPLDRegPonBitRange[0] = 8;
                        s_CPLDRegPonBitRange[1] = 0;
                        break;
                    case 12:
                        s_CPLDRegPonBitRange[0] = 8;
                        s_CPLDRegPonBitRange[1] = 4;
                        break;
                    case 16:
                        s_CPLDRegPonBitRange[0] = 8;
                        s_CPLDRegPonBitRange[1] = 8;
                        break;
                    default:
                        VOS_ASSERT(0);
                }
            
                s_PF_GetLocalOltIdByInt = GetLocalOltIdBySingleIntV2_16;
            }
        }
        else
        {
            s_CPLDRegPonIntState[0] = 0x06;
            s_CPLDRegPonIntState[1] = 0x07;

            s_PF_GetLocalOltIdByInt = GetLocalOltIdBySingleIntV1;
            sys_console_printf("\r\nPlease update the PON-Card's CPLD to V5 at least(now is V%d).\r\n", bCPLDVer);
        }
    }
    else
    {
        /* 默认采用上升沿触发 */
        s_iCPLDIntTriggerType  = PON_INT_BYEDGE;
        s_iCPLDIntTriggerValue = PON_INT_BYUPEDGE;

        s_PF_GetLocalOltIdByInt = GetLocalOltIdBySelfInt;
    }
    
    IntNum = GetLocalOltIntRange(aIntIds);
    for (i=0; i<IntNum; ++i)
    {
        uiIntReg = aIntIds[i];

        if ( PON_INT_BYEDGE == s_iCPLDIntTriggerType )
        {
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
            if ( PON_INT_BYUPEDGE == s_iCPLDIntTriggerValue )
            {
                cpuIrqConfig(uiIntReg, 1, 1);
            }
            else if ( PON_INT_BYDOWNEDGE == s_iCPLDIntTriggerValue )
            {
                cpuIrqConfig(uiIntReg, 1, 0);
            }
            else
            {
                VOS_ASSERT(0);
            }
#else
            uic0_tr = sysDcrInLong((volatile UINT32)UIC0_TR);
            uic_set(uiIntReg, uic0_tr);
            sysDcrOutLong((volatile UINT32)UIC0_TR, uic0_tr);
            
            uic0_cr = sysDcrInLong((volatile UINT32)UIC0_CR);
            uic_reset(uiIntReg, uic0_cr);
            sysDcrOutLong((volatile UINT32)UIC0_CR, uic0_cr);

            uic0_pr = sysDcrInLong((volatile UINT32)UIC0_PR);
            if ( PON_INT_BYUPEDGE == s_iCPLDIntTriggerValue )
            {
                uic_set(uiIntReg, uic0_pr);
            }
            else if ( PON_INT_BYDOWNEDGE == s_iCPLDIntTriggerValue )
            {
                uic_reset(uiIntReg, uic0_pr);
            }
            else
            {
                VOS_ASSERT(0);
            }
            sysDcrOutLong((volatile UINT32)UIC0_PR, uic0_pr);
#endif
        }
        else if ( PON_INT_BYLEVEL == s_iCPLDIntTriggerType )
        {
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
            if ( PON_INT_BYUPLEVEL == s_iCPLDIntTriggerValue )
            {
                cpuIrqConfig(uiIntReg, 0, 1);
            }
            else if ( PON_INT_BYDOWNLEVEL == s_iCPLDIntTriggerValue )
            {
                cpuIrqConfig(uiIntReg, 0, 0);
            }
            else
            {
                VOS_ASSERT(0);
            }
#else
            uic0_cr = sysDcrInLong((volatile UINT32)UIC0_CR);
            uic_reset(uiIntReg, uic0_cr);
            sysDcrOutLong((volatile UINT32)UIC0_CR, uic0_cr);

            uic0_pr = sysDcrInLong((volatile UINT32)UIC0_PR);
            if ( PON_INT_BYUPLEVEL == s_iCPLDIntTriggerValue )
            {
                uic_set(uiIntReg, uic0_pr);
            }
            else if ( PON_INT_BYDOWNLEVEL == s_iCPLDIntTriggerValue )
            {
                uic_reset(uiIntReg, uic0_pr);
            }
            else
            {
                VOS_ASSERT(0);
            }
            sysDcrOutLong((volatile UINT32)UIC0_PR, uic0_pr);
            
            uic0_tr = sysDcrInLong((volatile UINT32)UIC0_TR);
            uic_reset(uiIntReg, uic0_tr);
            sysDcrOutLong((volatile UINT32)UIC0_TR, uic0_tr);
#endif
        }
        else
        {
            VOS_ASSERT(0);
        }
    }

    intUnlock(lockKey);
    VOS_TaskUnlock();

    return;
}

/* CPLD开中断*/
void PonIntOpen(int iOltIdx)
{
    if ( s_iCPLDIntIsNeedReset )
    {
        int iRegID;
        UCHAR bRegValue, bRegBitSet;

        VOS_ASSERT((iOltIdx >= s_CPLDRegPonByteBase) && (iOltIdx < 16));

        if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
        {
            iRegID = 0;
            iOltIdx = (iOltIdx - s_CPLDRegPonByteBase) >> 2;
            bRegBitSet = 1 << iOltIdx;
            
            VOS_ASSERT(iOltIdx < 4);
        }
        else
        {
            if ( (iOltIdx -= s_CPLDRegPonByteBase) < 8 )
            {
                iRegID = 0;
                bRegBitSet = 1 << iOltIdx;
            }
            else
            {
                iRegID = 1;
                bRegBitSet = 1 << (iOltIdx - 8);
            }
        }

        ReadCPLDReg ( s_CPLDRegPonIntSwitch[iRegID], &bRegValue);
        if ( !(bRegValue & bRegBitSet) )
        {
            /* 先清中断，再开中断 */
            /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x55 );*/
            CPLD_WRITE_LOCK(CPLD_PROTECT_6900M);
            ReadCPLDReg ( s_CPLDRegPonIntState[iRegID], &bRegValue );
            WriteCPLDReg( s_CPLDRegPonIntState[iRegID], bRegValue & (~bRegBitSet) );
            ReadCPLDReg ( s_CPLDRegPonIntSwitch[iRegID], &bRegValue);
            WriteCPLDReg( s_CPLDRegPonIntSwitch[iRegID], bRegValue | bRegBitSet );
            /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x0 );*/
            CPLD_WRITE_UNLOCK(CPLD_PROTECT_6900M);
        }
    }

    return;
}

/* CPLD关中断*/
void PonIntClose(int iOltIdx)
{
    if ( s_iCPLDIntIsNeedReset )
    {
        int iRegID;
        UCHAR bRegValue, bRegBitSet;

        VOS_ASSERT((iOltIdx >= s_CPLDRegPonByteBase) && (iOltIdx < 16));

        if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
        {
            iRegID = 0;
            iOltIdx = (iOltIdx - s_CPLDRegPonByteBase) >> 2;
            bRegBitSet = 1 << iOltIdx;
            
            VOS_ASSERT(iOltIdx < 4);
        }
        else
        {
            if ( (iOltIdx -= s_CPLDRegPonByteBase) < 8 )
            {
                iRegID = 0;
                bRegBitSet = 1 << iOltIdx;
            }
            else
            {
                iRegID = 1;
                bRegBitSet = 1 << (iOltIdx - 8);
            }
        }

        ReadCPLDReg ( s_CPLDRegPonIntSwitch[iRegID], &bRegValue);
        if ( bRegValue & bRegBitSet )
        {
            /* 先关中断，再清中断 */
            /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x55 );*/
            CPLD_WRITE_LOCK(CPLD_PROTECT_6900M);
            ReadCPLDReg ( s_CPLDRegPonIntSwitch[iRegID], &bRegValue);
            WriteCPLDReg( s_CPLDRegPonIntSwitch[iRegID], bRegValue & (~bRegBitSet) );
            ReadCPLDReg ( s_CPLDRegPonIntState[iRegID], &bRegValue );
            WriteCPLDReg( s_CPLDRegPonIntState[iRegID], bRegValue & (~bRegBitSet) );
            /*WriteCPLDReg( (volatile UCHAR *)0x0B, 0x0 );*/
            CPLD_WRITE_UNLOCK(CPLD_PROTECT_6900M);
        }
    }

    return;
}

/* CPU关中断*/
void PonIntDisable(int iIntIdx)
{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

    cpuIrqDisable(pabIntIrqs[iIntIdx]);
    cpuIrqClear(pabIntIrqs[iIntIdx]);
#else
    UINT32 uic0_er,uic0_sr;
    UINT32 uiIntReg;

    uiIntReg = INT_VEC_EXT_IRQ_0 + iIntIdx;

    uic0_er = sysDcrInLong(UIC0_ER);
    uic_reset(uiIntReg, uic0_er);
    sysDcrOutLong(UIC0_ER, uic0_er);

    uic0_sr = sysDcrInLong(UIC0_SR);
    uic_set(uiIntReg, uic0_sr);
    sysDcrOutLong(UIC0_SR, uic0_sr);
#endif

    return;
}

/* CPU开中断*/
void PonIntEnable(int iIntIdx)
{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

    cpuIrqEnable(pabIntIrqs[iIntIdx]);
#else
    UINT32 uic0_er,uic0_sr;
    UINT32 uiIntReg;

    uiIntReg = INT_VEC_EXT_IRQ_0 + iIntIdx;

    uic0_er = sysDcrInLong(UIC0_ER);
    uic_set(uiIntReg, uic0_er);
    sysDcrOutLong(UIC0_ER, uic0_er);
#endif

    return;
}

/* CPU清中断*/
void PonIntClear(int iIntIdx)
{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

    cpuIrqClear(pabIntIrqs[iIntIdx]);
#else
    UINT32 uic0_er,uic0_sr;
    UINT32 uiIntReg;

    uiIntReg = INT_VEC_EXT_IRQ_0 + iIntIdx;
    
    uic0_sr = sysDcrInLong(UIC0_SR);
    uic_set(uiIntReg, uic0_sr);
    sysDcrOutLong(UIC0_SR, uic0_sr);
#endif

    return;
}

/* CPU锁中断*/
void PonIntLock(int iIntIdx)
{
    if ( iIntIdx < 0 || iIntIdx >= PRODUCT_MAX_EXT_INT_NUM )
    {
        iIntIdx = 0;
    }
    
    s_aiIntLocks[iIntIdx] = intLock();
    if ( PON_INT_BYEDGE == s_iCPLDIntTriggerType )
    {
        PonIntClear(iIntIdx);
    }
    else
    {
        PonIntDisable(iIntIdx);
    }

    return;
}

/* CPU解锁中断*/
void PonIntUnLock(int iIntIdx)
{
    if ( iIntIdx < 0 || iIntIdx >= PRODUCT_MAX_EXT_INT_NUM )
    {
        iIntIdx = 0;
    }

    intUnlock(s_aiIntLocks[iIntIdx]);
    if ( PON_INT_BYLEVEL == s_iCPLDIntTriggerType )
    {
        PonIntEnable(iIntIdx);
    }

    return;
}
#endif


/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     带有PON 芯片(对GFA6700,PON板. 对GFA6100, PON板和SW板)
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 板上可能有PON 芯片; 对GFA6700, 即为(或未初始化I2C 的)PON板,或空槽位;
                                    对GFA6100, 为SW板或PON板,或未初始化I2C 的PON板, 或空槽位
                        RERROR -- 板上没有PON 芯片
*********************************************************************************************/
int  SlotCardMayBePonBoardByVty(int CardIndex, struct vty *vty)
{
	int Product_type;

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_MODULE_IS_REMOTE(CardIndex) )
    {
        if ( !SYS_SLOT_IS_VALID_REMOTE(CardIndex) )
        {
    		if(EVENT_DEBUG == V2R1_ENABLE)
        		v2r1_printf( vty, "  %% Logical PON slot range is %d-%d\r\n",SYS_MIN_VIRTUAL_SLOT, SYS_MAX_VIRTUAL_SLOT);

            return RERROR;
        }

    	return( ROK);
    }
#endif

	if ( (CardIndex < PONCARD_FIRST) || (CardIndex > PONCARD_LAST))
	{
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf( vty, "  %% PON slot range is %d-%d\r\n",PONCARD_FIRST, PONCARD_LAST);

        return(RERROR);
	}

    Product_type = GetOltType();
	if(Product_type == V2R1_OLT_GFA6700)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_EPON ) || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
#ifdef _SUPPORT_8GE_6700_
			/* wangysh add 20110301 support 6700 slot2 uplink */
			if( SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version == 2*/ && CardIndex == 4 )
				return (RERROR);
			/* end */
#endif
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not %s board\r\n", CardIndex, BOARD_TYPE_GFA6700_EPON_STR );
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_MAIN ) || (__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_EPON ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not %s board\r\n", CardIndex, BOARD_TYPE_GFA6100_EPON_STR );
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6900)
	{		
		if ((CardIndex == SW[0]) || (CardIndex == SW[1]))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", CardIndex, typesdb_module_type2name(MODULE_E_GFA6900_SW));
			return RERROR;
		}	
    
		if(SYS_MODULE_IS_6900_EPON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 6900 pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6900M)
	{		
		if(SYS_MODULE_IS_6900_EPON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 6900M pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6900S)
	{		
		if(SYS_MODULE_IS_6900_EPON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 6900S pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8000)
	{		
		if ((CardIndex == SW[0]) || (CardIndex == SW[1]))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", CardIndex, typesdb_module_type2name(MODULE_E_GFA8000_SW));
			return RERROR;
		}	
    
		if(SYS_MODULE_IS_CPU_PON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 8000 pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8000M)
	{		
		if(SYS_MODULE_IS_CPU_PON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 8000M pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8000S)
	{		
		if(SYS_MODULE_IS_PON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not 8000S pon board\r\n", CardIndex);
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8100)
	{
		if(SYS_MODULE_IS_8100_PON(CardIndex))
		{
			return(ROK);
		}
		else {
			return(RERROR);
		}
	}

	return( RERROR );
}
int  SlotCardMayBePonBoard(int CardIndex)
{
	int Product_type;

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_MODULE_IS_REMOTE(CardIndex) )
    {
        if ( !SYS_SLOT_IS_VALID_REMOTE(CardIndex) )
        {
            return RERROR;
        }

    	return( ROK);
    }
#endif

	if ( (CardIndex < PONCARD_FIRST) || (CardIndex > PONCARD_LAST))
	{
        return(RERROR);
	}

    Product_type = GetOltType();
	if(Product_type == V2R1_OLT_GFA6700)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_EPON ) || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
#ifdef _SUPPORT_8GE_6700_
			/* wangysh add 20110301 support 6700 slot2 uplink */
			if( SYS_CHASSIS_IS_V2_8GE/*SYS_CHASSIS_VER.b_version == 2*/ && CardIndex == 4 )
				return (RERROR);
			/* end */
#endif
			return(ROK);
		}
		else
        {
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_MAIN ) || (__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_EPON ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else {
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA6900)
	{		
		if ((CardIndex == SW[0]) || (CardIndex == SW[1]))
		{
			return RERROR;
		}	
    
		if(SYS_MODULE_IS_6900_EPON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			return(RERROR);
		}
	}
	else if( (Product_type == V2R1_OLT_GFA6900M) || (Product_type == V2R1_OLT_GFA6900S) )
	{		
		if(SYS_MODULE_IS_6900_EPON(CardIndex)
           || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8000)
	{		
		if ((CardIndex == SW[0]) || (CardIndex == SW[1]))
		{
			return RERROR;
		}	
    
		if(SYS_MODULE_IS_PON(CardIndex) || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			return(RERROR);
		}
	}
	else if( (Product_type == V2R1_OLT_GFA8000M) || (Product_type == V2R1_OLT_GFA8000S) )
	{		
		if(SYS_MODULE_IS_PON(CardIndex) || (__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
		{
			return(ROK);
		}
		else
        {
			return(RERROR);
		}
	}
	else if(Product_type == V2R1_OLT_GFA8100)
	{
		if(SYS_MODULE_IS_8100_PON(CardIndex))
		{
			return(ROK);
		}
		else {
			return(RERROR);
		}
	}

	return( RERROR );
}
/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM 板;
     注:  以下各板都统称为TDM 板
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     		
    注: TDM 板以后还会有新的类型出现，如支持V5 接口的TDM-v5 板; 留待以后扩展
    
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM 板
                        RERROR -- 不是TDM 板,或没有插板
*********************************************************************************************/
int SlotCardIsTdmBoard( int CardIndex )
{
	if((SlotCardIsTdmSgBoard(CardIndex) == ROK) || (SlotCardIsTdmE1Board(CardIndex) == ROK))
		 return(ROK);
	else return(RERROR);
}

/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM 板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM 板
*********************************************************************************************/
int SlotCardMayBeTdmBoardByVty( int CardIndex , struct vty *vty)
{
	int Product_type = GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SIG ) ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_E1 ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM board\r\n", CardIndex );
			return( RERROR );
		}		
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_SIG) ||(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_E1 ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM board\r\n", CardIndex );
			return( RERROR );
		}
	}
	return( RERROR );
}

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM -SG 板;
     注:  以下各板都统称为TDM -SG板
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     		
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM-sg 板
                        RERROR -- 不是TDM -sg 板,或没有插板
*********************************************************************************************/
int SlotCardIsTdmSgBoard( int CardIndex )
{
	int Product_type =GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
		{
		if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SIG )
			return( ROK );
		}
	else if(Product_type == V2R1_OLT_GFA6100)
		{
		if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_SIG)
			return( ROK );
		}
	return( RERROR );
}

/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM -sg 板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -sg 板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM -sg 板
*********************************************************************************************/
int SlotCardMayBeTdmSgBoardByVty( int CardIndex , struct vty *vty)
{
	int Product_type = GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SIG ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM-sig board\r\n", CardIndex );
			return( RERROR );
		}		
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_SIG) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM-sig board\r\n", CardIndex );
			return( RERROR );
		}
	}
	return( RERROR );
}

/* ******************************************************************************************
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否
     为TDM -e1 板;
     注:  以下各板都统称为TDM -E1板
     		MODULE_E_GFA_TDM
     		MODULE_E_GFA_SIG
     		MODULE_E_GFA6100_TDM
     	注: E1 板可能还会分两种，	每FPGA 出8个E1 的板卡，和每FGPA 出16 个E1 的板卡
     		   这里不再细分，统称为TDM-e1 板；每FPGA出的不同的E1 个数可在具体应用
     		   细节上区分
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM-e1 板
                        RERROR -- 不是TDM -e1 板,或没有插板
*********************************************************************************************/
int SlotCardIsTdmE1Board( int CardIndex )
{
	int Product_type =GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
		{
		if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_E1)
			return( ROK );
		}
	else if(Product_type == V2R1_OLT_GFA6100)
		{
		if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_E1)
			return( ROK );
		}
	return( RERROR );
}

 
/* ******************************************************************************************
	用于CLI
     根据产品类型的不同( GFA6700, or GFA6100 ), 判断当前槽位中的板卡是否可能
     为TDM -e1板
     输入参数:  unsigned int CardIndex, 取值: 从1 开始
     返回值: ROK -- 是TDM -e1板,或未初始化I2C 的TDM板,或为空槽位
                        RERROR -- 不是TDM -e1 板
*********************************************************************************************/
int SlotCardMayBeTdmE1BoardByVty( int CardIndex , struct vty *vty)
{
	int Product_type = GetOltType();

	if(Product_type == V2R1_OLT_GFA6700)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_E1 ) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM-E1 board\r\n", CardIndex );
			return( RERROR );
		}		
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		if((__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA6100_E1) ||(__SYS_MODULE_TYPE__(CardIndex) <= MODULE_TYPE_UNKNOW))
			return( ROK );
		else{
			if(EVENT_DEBUG == V2R1_ENABLE)
				v2r1_printf( vty, "  Slot %d is not TDM-E1 board\r\n", CardIndex );
			return( RERROR );
		}
	}
	return( RERROR );
}

/* ********************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对PON板Slot 和Port 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
                            PortIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**********************************************************************/
int PonCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty)
{
	if ( (SlotIdx < PONCARD_FIRST) || (SlotIdx > PONCARD_LAST))
	{
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf( vty, "  %% PON slot range is %d-%d\r\n",PONCARD_FIRST, PONCARD_LAST);
		return(RERROR);
	}
	if ((PortIdx > PONPORTPERCARD ) || (PortIdx < 1))
	{
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf( vty, "  %% PON port range is 1-%d\r\n", PONPORTPERCARD);
		return(RERROR);
	}
	return(ROK);
}

int PonCardSlotPortCheckWhenRunningByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty)
{
	int Product_type;
	int Card_type;

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
    if ( SYS_MODULE_IS_REMOTE(SlotIdx) )
    {
        if ( !SYS_SLOT_IS_VALID_REMOTE(SlotIdx) )
        {
    		vty_out( vty, "  %% Logical PON slot range is %d-%d\r\n",SYS_MIN_VIRTUAL_SLOT, SYS_MAX_VIRTUAL_SLOT);
    		return RERROR;
        }
        
        if ( !SYS_PORT_IS_VALID_REMOTE(PortIdx) )
        {
    		vty_out( vty, "  %% Logical PON port range is %d-%d\r\n",1, SYS_MAX_VIRTUAL_PORTNUM);
    		return RERROR;
        }

    	return( ROK);
    }
#endif

	if ( (SlotIdx < PONCARD_FIRST) || (SlotIdx > PONCARD_LAST))
	{
		vty_out( vty, "  %% PON slot range is %d-%d\r\n",PONCARD_FIRST, PONCARD_LAST);
		return RERROR;
	}
	if ((PortIdx > PONPORTPERCARD ) || (PortIdx < 1))
	{
		vty_out( vty, "  %% PON port range is 1-%d\r\n", PONPORTPERCARD);
		return RERROR;
	}

    Product_type = GetOltType();
	if( Product_type == V2R1_OLT_GFA6700 )
	{		
		if (SlotIdx == (SW[2]))
		{
			if(MODULE_E_GFA_SW == __SYS_MODULE_TYPE__(SlotIdx))
			{
				vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(MODULE_E_GFA_SW)/*MODULE_E_EPON3_SW_NAME_STR*/);
				return RERROR;
			}
		}	
	}
    else if ( Product_type == V2R1_OLT_GFA6900 )
    {
		if ((SlotIdx == SW[0]) || (SlotIdx == SW[1]))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(MODULE_E_GFA6900_SW));
			return RERROR;
		}	
        
	    Card_type = __SYS_MODULE_TYPE__(SlotIdx);
		if((MODULE_E_GFA6900_GEM_GE == Card_type)||(MODULE_E_GFA6900_GEM_10GE == Card_type))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(Card_type));
			return RERROR;
		}
    }
	else if ( Product_type == V2R1_OLT_GFA6900M || Product_type == V2R1_OLT_GFA6900S)
    {
	    Card_type = __SYS_MODULE_TYPE__(SlotIdx);
		if(MODULE_E_GFA6900_GEM_10GE == Card_type)
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(Card_type));
			return RERROR;
		}

        /* B--added by liwei056@2014-3-12 for 16PON-Card */
        if ( SYS_MODULE_TYPE_IS_UPLINK_PON(Card_type) )
        /* E--added by liwei056@2014-3-12 for 16PON-Card */
        {
            /* B--added by liwei056@2013-3-19 for D16712 */
           	if ( PortIdx <= ETHPORTPERCARD )
            {
        		vty_out( vty, "  %% PON port range is %d-%d\r\n", ETHPORTPERCARD + 1, PONPORTPERCARD);
        		return RERROR;
            }
            /* E--added by liwei056@2013-3-19 for D16712 */
        }
    }
    else if ( Product_type == V2R1_OLT_GFA8000 )
    {
		if ((SlotIdx == SW[0]) || (SlotIdx == SW[1]))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(MODULE_E_GFA8000_SW));
			return RERROR;
		}	
        
	    Card_type = __SYS_MODULE_TYPE__(SlotIdx);  /*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
		if((MODULE_E_GFA6900_GEM_GE == Card_type)||(MODULE_E_GFA6900_GEM_10GE == Card_type))
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(Card_type));
			return RERROR;
		}
    }
	else if ( Product_type == V2R1_OLT_GFA8000M || Product_type == V2R1_OLT_GFA8000S)
    {
	    Card_type = __SYS_MODULE_TYPE__(SlotIdx);
		if(MODULE_E_GFA6900_GEM_10GE == Card_type)
		{
			vty_out( vty, "  %% Error slot %d. This slot is %s borad\r\n", SlotIdx, typesdb_module_type2name(Card_type));
			return RERROR;
		}

        /* B--added by liwei056@2014-3-12 for 16PON-Card */
        if ( SYS_MODULE_TYPE_IS_UPLINK_PON(Card_type) )
        /* E--added by liwei056@2014-3-12 for 16PON-Card */
        {
            /* B--added by liwei056@2013-3-19 for D16712 */
           	if ( PortIdx <= ETHPORTPERCARD )
            {
        		vty_out( vty, "  %% PON port range is %d-%d\r\n", ETHPORTPERCARD + 1, PONPORTPERCARD);
        		return RERROR;
            }
            /* E--added by liwei056@2013-3-19 for D16712 */
        }
    }
    
	return( ROK);
}

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对PON板Slot 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**************************************************************/
int PonCardSlotRangeCheckByVty(unsigned long SlotIdx, struct vty *vty)
{
	if ( (SlotIdx < PONCARD_FIRST) || (SlotIdx > PONCARD_LAST))
	{
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf(vty,"  %% PON slot range is %d-%d\r\n",PONCARD_FIRST, PONCARD_LAST);
		return(RERROR);
	}
	return(ROK);
}

/* ********************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对tdm 板Slot 和Port 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
                            PortIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**********************************************************************/
int TdmCardSlotPortCheckByVty(unsigned long SlotIdx, unsigned long PortIdx, struct vty *vty)
{
	if ((SlotIdx < TDMCARD_FIRST) || (SlotIdx > TDMCARD_LAST))
	{
		/*if(vty != NULL )
			{
			vty_out( vty, "  %% Parameter error.slot range is %d-%d\r\n",TDMCARD_FIRST, TDMCARD_LAST);
			}
		else {
			if(EVENT_DEBUG == V2R1_ENABLE )
				sys_console_printf("  %% Parameter error.slot range is %d-%d\r\n",TDMCARD_FIRST, TDMCARD_LAST);
			}*/
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf( vty, "  %% TDM slot range is %d-%d\r\n",TDMCARD_FIRST, TDMCARD_LAST);
		return(RERROR);
	}
	if ((PortIdx > TDM_FPGA_MAX ) || (PortIdx < TDM_FPGA_MIN))
	{
		/*if(vty != NULL )
			{
			vty_out( vty, "  %% Parameter error.port range is 1-%d\r\n", TDM_FPGA_MAX);
			}
		else {
			if(EVENT_DEBUG == V2R1_ENABLE )
				sys_console_printf("  %% Parameter error.port range is 1-%d\r\n", TDM_FPGA_MAX);
			}*/
		if(EVENT_DEBUG == V2R1_ENABLE)
			v2r1_printf( vty, "  %% TDM port range is 1-%d\r\n", TDM_FPGA_MAX);
		return(RERROR);
	}
	return(ROK);
}
/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板Slot 的
    范围进行判断, 超出范围的提示错误
   输入参数: slotIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**************************************************************/
int TdmCardSlotRangeCheck(unsigned long SlotIdx)
{
	if ( (SlotIdx < TDMCARD_FIRST) || (SlotIdx > TDMCARD_LAST))
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
			sys_console_printf("  %% TDM slot range is %d-%d\r\n",TDMCARD_FIRST, TDMCARD_LAST);
		return(RERROR);
	}
	return(ROK);
}

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板port的
    范围进行判断, 超出范围的提示错误
   输入参数: PortIdx -- 从1 开始
   返回值:  ROK
                       RERROR
**************************************************************/
int TdmPortRangeCheck(unsigned long PortIdx)
{
	if ( (PortIdx < TDM_FPGA_MIN) || (PortIdx > TDM_FPGA_MAX))
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
		sys_console_printf("  %% TDM fpga-port range is %d-%d\r\n",TDM_FPGA_MIN, TDM_FPGA_MAX);
		return(RERROR);
	}
	return(ROK);
}

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板E1 的
    范围进行判断, 超出范围的提示错误
   输入参数: E1Num -- 从1 开始-- MAX_E1_PORT_NUM
   返回值:  ROK
                       RERROR
**************************************************************/
int TdmE1NumRangeCheck(unsigned long E1Num)
{
	if ( (E1Num < 1) || (E1Num > MAX_E1_PORT_NUM))
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
		sys_console_printf("  %% E1-num range is 1-%d\r\n", MAX_E1_PORT_NUM);
		return(RERROR);
	}
	return(ROK);
}

/* ************************************************************
    根据产品类型不同( GFA6700, or GFA6100 ), 对TDM 板每端口支持的E1 
    范围进行判断, 超出范围的提示错误
   输入参数: E1NumPerPort -- 从1 开始-- MAX_E1_PER_FPGA
   返回值:  ROK
                       RERROR
**************************************************************/
int TdmFpgaE1NumRangeCheck(unsigned long E1NumPerPort)
{
	if ( (E1NumPerPort < 1) || (E1NumPerPort > MAX_E1_PER_FPGA))
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
		sys_console_printf("  %% E1 port range is 1-%d\r\n", MAX_E1_PER_FPGA);
		return(RERROR);
	}
	return(ROK);
}

unsigned char * GetGFA6xxxTdmNameString()
{
	int Product_type = GetOltType();
	int CardIndex = get_gfa_tdm_slotno();
	
	if(Product_type == V2R1_OLT_GFA6700 )
		{
		if(SlotCardIsTdmSgBoard(CardIndex) == ROK)
			return(typesdb_module_type2name(MODULE_E_GFA_SIG));
		else if(SlotCardIsTdmE1Board(CardIndex) == ROK)
			return(typesdb_module_type2name(MODULE_E_GFA_E1));
		}
	else if(Product_type == V2R1_OLT_GFA6100)
		{
		if(SlotCardIsTdmSgBoard(CardIndex) == ROK)
			return(typesdb_module_type2name(MODULE_E_GFA6100_SIG));
		else if(SlotCardIsTdmE1Board(CardIndex) == ROK)
			return(typesdb_module_type2name(MODULE_E_GFA6100_E1));
		}

	return(BOARD_TYPE_GFA6700_SIG_STR);
}


void  ModifyCardNameBySlot(unsigned long CardIdx)
{
	unsigned char CardTypeString[50];
	
	VOS_Sprintf(CardTypeString, "%s(slot%d)", GetOltBoardString/*typesdb_module_type2name*/(__SYS_MODULE_TYPE__(CardIdx)),CardIdx);
	VOS_StrCpy(CardSlot_s[CardIdx],CardTypeString);
	return;	
}

#if 0
extern int GetOltSWVersion(char * Version, int * len);
extern void PonCardInserted( ULONG CardIndex );
extern void PonCardActivated(unsigned long CardIndex );
extern int PonCardStatusCheck(unsigned long CardIndex );
extern void PonCardPull ( unsigned long CardIndex );		/* added by xieshl 20061009, 支持EPON板拔插和复位 */
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
/* tdm 管理与设备管理消息交互*/
extern void  TdmCardPulled ( unsigned long CardIndex );
extern void  TdmCardReset ( unsigned long CardIndex );
extern void  TdmCardActivated(unsigned long CardIndex );
extern int  TdmCardStatusQuery(unsigned long TdmSlot);
extern int  TdmUpdateAppfile(unsigned long CardIndex);
extern int  TdmUpdateAppfileComp(unsigned long CardIndex, unsigned int result );
extern int  TdmUpdateFpgafile(unsigned long CardIndex);
extern int  TdmUpdateFpgafileComp(unsigned long CardIndex, unsigned int result );
#endif
#endif 
extern ULONG userSlot_userPort_2_Ifindex( ULONG ulUserSlot, ULONG ulUserPort );
extern LONG IFM_admin_down( ULONG ulIfindex, VOID * pData, CHAR ** szErrInfo );
extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern void WriteCPLDReg( unsigned char * RegAddr, unsigned char ucIntData );
extern LONG IFM_admin_up( ULONG ulIfindex, VOID * pData, CHAR ** szErrInfo );
extern STATUS tdmReadVersion( UCHAR calltype, ULONG callnotifier, UCHAR * softwarever, 
								USHORT *sfverlen, UCHAR *fpgaver, USHORT *fpgaverlen );

/* added by xieshl 20081230, 针对6100 虚拟板卡复位 */
LONG device_device_board_dummy_reset( ULONG module_type, ULONG slotno )
{
	LONG reset_dummy_flag = 0;
    
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
	{
		if( module_type == MODULE_E_GFA6100_GEM )
		{
			ULONG cpld_reg = 6;
			UCHAR cpld_val = 0;
			int i;
			ULONG ifIndex, val;
			reset_dummy_flag = 1;

			for( i=1; i<=4; i++ )
			{
				val = 0;
				ifIndex = userSlot_userPort_2_Ifindex( 1, i );
				IFM_admin_down( ifIndex, &val, NULL );
			}

			ReadCPLDReg( (UCHAR*)cpld_reg, &cpld_val );
			cpld_val &= 0xfb;
			WriteCPLDReg( (UCHAR*)cpld_reg, cpld_val );
			VOS_TaskDelay(20);
			cpld_val |= 0x04;
			WriteCPLDReg( (UCHAR*)cpld_reg, cpld_val );
			ponBoardReset_EventReport ( 1, slotno );
			
			for( i=1; i<=4; i++ )
			{
				val = 1;
				ifIndex = userSlot_userPort_2_Ifindex( 1, i );
				IFM_admin_up( ifIndex, &val, NULL );
			}
		}
		else if( module_type ==MODULE_E_GFA6100_MAIN )
		{
			reset_dummy_flag = 1;
			/*PonCardPulled(slotno);*/	/* modified by xieshl 20090119, 问题单9683 */
			PonCardPull(module_type, slotno);
			PonCardInserted(module_type, slotno);
			PonCardActivated(slotno);
			ponBoardReset_EventReport( 1, slotno );
		}
	}
	return reset_dummy_flag;
}

/* added by xieshl 20080327, 加载固件, 增加板类型参数 */
LONG epon_device_board_inserted( ULONG module_type, ULONG slotno )
{
	if(SlotCardIsPonBoard(slotno) == ROK)
	{
		/*VOS_Sprintf(CardSlot_s[slotno], "Pon(slot%d)", slotno);*/
		PonCardInserted( module_type, slotno );
	}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	else if(SlotCardIsTdmBoard(slotno) == ROK)
	{
		/*VOS_Sprintf(CardSlot_s[slotno], "Tdm(slot%d)", slotno);*/
		TdmCardInserted( slotno );
	}
#endif
	else if(SlotCardIsUplinkBoard(slotno) == ROK)
	{
		/*VOS_Sprintf(CardSlot_s[slotno], "Uplink(slot%d)", slotno);*/
		UplinkCardInserted( slotno );
	}
	/*else
	{
		sys_console_printf(" \r\n slot%d module_type=%d", slotno, module_type);
		return VOS_ERROR;
	}*/
	return( VOS_OK );
}

LONG epon_device_board_pulled( ULONG module_type, ULONG slotno )
{
	if(SlotCardIsPonBoard(slotno) == ROK)
	{
		PonCardPull( module_type, slotno );
	}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	else if(SlotCardIsTdmBoard(slotno) == ROK)
	{		
		TdmCardPulled( slotno );
	}
#endif
	return( VOS_OK);
}

LONG epon_device_board_activated( ULONG module_type, ULONG slotno )
{
	if(SlotCardIsPonBoard(slotno) == ROK)
	{
		PonCardActivated( slotno );
	}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	else if(SlotCardIsTdmBoard(slotno) == ROK)
	{	
		TdmCardActivated( slotno );
	}
#endif
	return( VOS_OK);
}

LONG  epon_device_board_status_check( ULONG module_type, ULONG slotno )
{
	if(SlotCardIsPonBoard(slotno) == ROK)
	{
		return(PonCardStatusCheck( module_type, slotno ));
	}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	else if(SlotCardIsTdmBoard(slotno) == ROK)
	{		
		return(TdmCardStatusQuery( slotno ));
	}
#endif
	return 0;
}

/*vvvv(ULONG slotno )
{
	sys_version_no *pVer;

	pVer = &devsm_module[slotno].software_version;
	sys_console_printf( "\r\n gros: V%dR%02dB%02d%d\r\n", pVer->v_version, pVer->r_version, 
		pVer->b_version, pVer->d_version);

	pVer = &devsm_module[slotno].pub_info.hardware_version;
	sys_console_printf( "\r\n hardware: V%d.%d%d\r\n", 
		pVer->b_version, pVer->d_version, pVer->sp_version);

	pVer = &devsm_module[slotno].pub_info.bootloader_version;
	sys_console_printf( "\r\n boot: V%d.%02d.%d\r\n",  
		pVer->b_version, pVer->d_version, pVer->sp_version);
}*/

extern LONG getLocalModuleBspVersion( char * version );
extern LONG epon_device_slot_board_version_get(ULONG slotno, char *boot, char *sw, char *fw, char *cpld);
extern LONG epon_device_8000_slot_board_version_get(ULONG slotno, char *boot, char *sw, char *fw, char *cpld,char *fpga);
extern LONG getLocalFirmwareVersion( char * version );

LONG epon_device_local_board_version_get(char *boot, char *sw, char *fw, char *cpld)
{
	LONG rc = VOS_ERROR;
	int swLen/*, fwLen*/;
	if( sw )
		rc = GetOltSWVersion(sw, &swLen);
	if( boot )
		rc = getLocalModuleBspVersion( boot );
	if( fw )
		rc = getLocalFirmwareVersion( fw );
    if( cpld )
		rc = getLocalCPLDVersion( cpld );
	return rc;
}

LONG epon_device_8000_board_version_get( ULONG slotno, char *boot_ver, char *sw_ver, char *fw_ver , char *cpld_ver,char *fpga_ver)
{
	char boot[32] = {0}, sw[32] = {0}, fw[32] = {0}, cpld[32] = {0},fpga[32] = {0};
	USHORT swLen = 0, fwLen = 0;
	LONG rc = VOS_ERROR;
	/*sys_version_no *pVer;*/
	
	if( SlotCardIsSwBoard(slotno) == ROK )
	{
		/* modified by xieshl 20080416, #6616 */
		if( slotno == SYS_LOCAL_MODULE_SLOTNO )
		{
			rc = epon_device_local_board_version_get(boot, sw, fw, cpld);
			
			if(fpga)
				rc = getLocalFPGAVersion( fpga );
		}
		else
		{
			rc = epon_device_8000_slot_board_version_get(slotno, boot, sw, fw, cpld,fpga);
		}
	}
	else if( SlotCardIsPonBoard(slotno) == ROK )
	{
		if ( SYS_MODULE_SLOT_ISHAVECPU(slotno))
		{
			if( slotno == SYS_LOCAL_MODULE_SLOTNO )
			{
				rc = epon_device_local_board_version_get(boot, sw, fw, cpld);
			}
			else
			{

				rc = epon_device_8000_slot_board_version_get(slotno, boot, sw, fw, cpld,fpga);
			}
		}      
		else
		{
			rc = GetPonCardVersion( slotno, fw, sw, 0, boot );
		}      
	}
	else if( SlotCardIsTdmBoard(slotno) == ROK )
	{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		/* modified by xieshl 20090927, 规避TDM板boot版本错误问题 */
		VOS_StrCpy( boot, "V1.00.1" );
		rc = tdmReadVersion(ENUM_CALLTYPE_SYN, 0, sw, &swLen, fw, &fwLen);
		if( rc == VOS_OK )
		{
			if( VOS_StrCmp(sw, "V1R01B051") > 0 )
				VOS_StrCpy( boot, "V1.01.1" );
		}
#endif
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET || __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET || __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		/*added by mengxsh@21050610 目前主要是为了获取8xet的CPLD的版本信息 */
		rc = getOLTNoCpuBoardVersion( slotno, fpga, cpld );
	}
	
	if( rc == VOS_ERROR )
	{
		VOS_StrCpy( boot, "-" );
		VOS_StrCpy( sw, "-" );
		VOS_StrCpy( fw, "-" );
		VOS_StrCpy( cpld, "-" );
		VOS_StrCpy( fpga, "-" );
	}

	if( boot_ver )
		VOS_StrCpy( boot_ver, boot );
	if( sw_ver )
		VOS_StrCpy( sw_ver, sw );
	if( fw_ver )
		VOS_StrCpy( fw_ver, fw );
	if( cpld_ver )
		VOS_StrCpy( cpld_ver, cpld );
	if( fpga_ver )
		VOS_StrCpy( fpga_ver, fpga );
	
	
	return VOS_OK;
}

LONG epon_device_board_version_get( ULONG slotno, char *boot_ver, char *sw_ver, char *fw_ver , char *cpld_ver)
{
	char boot[32] = {0}, sw[32] = {0}, fw[32] = {0}, cpld[32] = {0};
	USHORT swLen = 0, fwLen = 0;
	LONG rc = VOS_ERROR;
	/*sys_version_no *pVer;*/
	
	if( SlotCardIsSwBoard(slotno) == ROK )
	{
		/* modified by xieshl 20080416, #6616 */
		if( slotno == SYS_LOCAL_MODULE_SLOTNO )
		{
			rc = epon_device_local_board_version_get(boot, sw, fw, cpld);
		}
		else
		{
			rc = epon_device_slot_board_version_get(slotno, boot, sw, fw, cpld);
		}
	}
	else if( SlotCardIsPonBoard(slotno) == ROK )
	{
		if ( SYS_MODULE_SLOT_ISHAVECPU(slotno))
		{
			if( slotno == SYS_LOCAL_MODULE_SLOTNO )
			{
				rc = epon_device_local_board_version_get(boot, sw, fw, cpld);
			}
			else
			{
				rc = epon_device_slot_board_version_get(slotno, boot, sw, fw, cpld);
			}
		}      
		else
		{
			rc = GetPonCardVersion( slotno, fw, sw, 0, boot );
		}      
	}
	else if( SlotCardIsTdmBoard(slotno) == ROK )
	{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		/* modified by xieshl 20090927, 规避TDM板boot版本错误问题 */
		VOS_StrCpy( boot, "V1.00.1" );
		rc = tdmReadVersion(ENUM_CALLTYPE_SYN, 0, sw, &swLen, fw, &fwLen);
		if( rc == VOS_OK )
		{
			if( VOS_StrCmp(sw, "V1R01B051") > 0 )
				VOS_StrCpy( boot, "V1.01.1" );
		}
#endif
	}
	if( rc == VOS_ERROR )
	{
		VOS_StrCpy( boot, "-" );
		VOS_StrCpy( sw, "-" );
		VOS_StrCpy( fw, "-" );
		VOS_StrCpy( cpld, "-" );
	}

	if( boot_ver )
		VOS_StrCpy( boot_ver, boot );
	if( sw_ver )
		VOS_StrCpy( sw_ver, sw );
	if( fw_ver )
		VOS_StrCpy( fw_ver, fw );
	if( cpld_ver )
		VOS_StrCpy( cpld_ver, cpld );
	
	return VOS_OK;
}

#ifdef __cplusplus

}
#endif

