/***************************************************************
*
*						Module Name:  V2R1_Product_type.c
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
*   Date: 			2008/03/31
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  08/03/31  |   chenfj          |     create 
**----------|-----------|------------------
**  09/04/22  |   chenfj        |  modified for all OEM and GW 
**
***************************************************************/
#ifdef __cplusplus
extern"C"{
#endif

#include  "vos/vospubh/vos_base.h"
#include  "sys/main/sys_main.h"
#include  "cpi/typesdb/typesdb_product.h"
#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "gweponSys.h"
#include  "Device_flash.h"
/*#include "cpi/typesdb/typesdb_module.h"*/

/*#include "syscfg.h" */
/*  MIB 设备类型定义
unknown ( 1 ) , 
gfa6700 ( 2 ) , 
gfa6100 ( 3 ) , 
gt811 ( 4 ) , 
gt821 ( 5 ) , 
gt831 ( 6 ) , 
gt812 ( 7 ) , 
gt813 ( 8 ) , 
gt881 ( 9 ) , 
gt861 ( 10 ) , 
gt891 ( 11 ) , 
gt810 ( 12 ) , 
gt863 ( 13 ) , 
other ( 14 ) , 
gt865 ( 15 ) , 
gt816 ( 16 )
*/

/*#define  _EPON_TYPE_GDC_OEM_
#define  _EPON_TYPE_GW_ */
/*(_EPON_TYPE_GDC_OEM_ ) ||(_EPON_TYPE_RTC_OEM_) || (_EPON_TYPE_HDM_OEM_)
  _EPON_TYPE_OEM_ */

char  V2R1_VERSION[] = "V1R15B287";
char  PAS_VERSION[] = "V5R03B013";
char  TK_VERSION[] =  "V1R02B001";
char  BCM_VERSION[] =  "V1R01B001";
char  GPON_VERSION[] =  "V2R00B000";

char * app_creationDate   = __TIME__ " " __DATE__;	/* added by xieshl 20091023, 区分产品软件build日期 */

 /* 定义GW EPON-V2R1  产品类型*/

/*------------------------------------------------------------------*/
/* 系统信息 */

char OltVendorInfo[MAXVENDORINFOLEN+1] /*=  OLT_VENDOR_INFO_DEFAULT_default*/;
char OltVendorLocation[MAXLOCATIONLEN+1]/*= OLT_VENDOR_LOCATION_default*/;
char ProductCorporationName_AB[MAXCorporationNameLenAB+1] /*= PRODUCT_CORPORATION_AB_NAME_default*/;
char ProductCorporationName_Short[MAXCorporationNameLENShort+1] /*= PRODUCT_CORPORATION_SHORT_NAME_default*/;
char ProductCorporationName[MAXCorporationNameLEN+1] /*= PRODUCT_CORPORATION_NAME_default*/;

static char V2R1Series[MAXV2R1SeriesLEN+1] /*= EPON_V2R1_SERIES_default*/;
static char V2R1Copyright[MAXV2r1CopyrightLEN+1] /*=EPON_V2R1_COPYRIGHT_default*/;
/* 产品软件描述 */
char  ProductSoftwareDesc[MAXProductSoftwareDescLen+1] /*= PRODUCT_OLT_SOFTWARE_DESC_STR_default*/;

char OltLocationDefault[MAXLOCATIONLEN+1] /*= OLT_LOCATION_DEFAULT_default*/;
char OltDeviceNameDefault[MAXDEVICENAMELEN+1];
char OltDeviceDescriptionDefault[MAXDEVICEDESCLEN+1];

char OLTSwAppKeyName[8]=PRODUCT_OLT_SWAPP_KEY_NAME;
char OltTdmAppkeyName[8] = PRODUCT_OLT_SIGAPP_KEY_NAME;

char  OltBoardTypeString[MODULE_TYPE_MAX+1][BOARDTYPESTRINGLEN+1] ;  /*  OLT 板卡类型名称*/
char  OltBoardTypeStringShort[MODULE_TYPE_MAX+1][BOARDTYPESTRINGLEN_SHORT+1] ;	/* olt 板卡类型名称简写*/

/* onu 信息*/
char OnuVendorInfoDefault[MAXVENDORINFOLEN+1] /*= ONU_VENDOR_INFO_DEFAULT_default*/;
char OnuDeviceNameDefault[MAXDEVICENAMELEN+1] /*= ONU_DEVICE_NAME_DEFAULT_default*/;
char OnuLocationDefault[MAXLOCATIONLEN+1] /*= ONU_LOCATION_DEFAULT_default*/;
char OnuDescDefault[MAXDEVICEDESCLEN+1] /*= ONU_DESC_DEFAULT_default*/;
	
char OnuAppKeyName[ONU_TYPE_LEN+1] ;
char OnuBoardTypeString[SUB_BOARD_GT_LAST+1][BOARDTYPESTRINGLEN+1] ;  /*  ONU 板卡类型名称*/
/*------------------------------------------------------------------*/

unsigned char DeviceDesc_1[V2R1_DEVICE_LAST][MAXDEVICEDESCLEN+1];
/*unsigned char DeviceDesc[V2R1_DEVICE_LAST][MAXDEVICEDESCLEN+1];*/
unsigned char DeviceType_1[V2R1_DEVICE_LAST][ONU_TYPE_LEN+1];
unsigned char OnuAppType_1[V2R1_ONU_MAX][ONU_TYPE_LEN+1];
unsigned char OnuVoiceType_1[V2R1_ONU_MAX][ONU_TYPE_LEN+1];
unsigned char OnuFpgaType_1[V2R1_ONU_MAX][ONU_TYPE_LEN+1];
int OnuCtcRegisterId[V2R1_ONU_MAX]/*={0}*/;
OnuUniPort_t  OnuUniPort[V2R1_ONU_MAX];

UCHAR *typesdb_product_copyright()
{
	return (&V2R1Copyright[0]);
}

UCHAR *typesdb_product_series()
{
	return (&V2R1Series[0]);
}

UCHAR *typesdb_product_default_sysname()
{
	return (&V2R1Series[0]);
}

char * typesdb_product_olt_vendor_location()
{
	return(&OltVendorLocation[0]);
}

UCHAR *typesdb_pruduct_default_onu_vendor_info()
{
	return(&OnuVendorInfoDefault[0]);
}


char *GetDeviceDescString(int type)
{
	if((type >= V2R1_DEVICE_LAST) || (type < V2R1_OTHER))
		return DEVICE_TYPE_DESC_UNKNOWN_STR;
	
	return(&(DeviceDesc_1[type][0]));
}

char *GetDeviceTypeString(int type)
{
	if((type >= V2R1_DEVICE_LAST) || (type < V2R1_OTHER))
		return DEVICE_TYPE_NAME_UNKNOWN_STR;
	
	return(&(DeviceType_1[type][0]));
}

char *GetDeviceDescString_6700()
{
	return(GetDeviceDescString(V2R1_OLT_GFA6700));
}
char *GetDeviceTypeString_6700()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA6700));
}
char *GetDeviceDescString_6100()
{
	return(GetDeviceDescString(V2R1_OLT_GFA6100));
}
char *GetDeviceTypeString_6100()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA6100));
}
/* wangysh add 6900 */
char *GetDeviceDescString_6900()
{
	return(GetDeviceDescString(V2R1_OLT_GFA6900));
}
char *GetDeviceTypeString_6900()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA6900));
}
char *GetDeviceDescString_6900M()
{
	return(GetDeviceDescString(V2R1_OLT_GFA6900M));
}

char *GetDeviceTypeString_6900M()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA6900M));
}

char *GetDeviceDescString_6900S()
{
	return(GetDeviceDescString(V2R1_OLT_GFA6900S));
}

char *GetDeviceTypeString_6900S()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA6900S));
}

char *GetDeviceDescString_8000()
{
	return(GetDeviceDescString(V2R1_OLT_GFA8000));
}
char *GetDeviceTypeString_8000()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA8000));
}
char *GetDeviceDescString_8000M()
{
	return(GetDeviceDescString(V2R1_OLT_GFA8000M));
}

char *GetDeviceTypeString_8000M()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA8000M));
}

char *GetDeviceDescString_8000S()
{
	return(GetDeviceDescString(V2R1_OLT_GFA8000S));
}

char *GetDeviceTypeString_8000S()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA8000S));
}

char *GetDeviceDescString_8100()
{
	return(GetDeviceDescString(V2R1_OLT_GFA8100));
}

char *GetDeviceTypeString_8100()
{
	return(GetDeviceTypeString(V2R1_OLT_GFA8100));
}



char *GetOnuEponAppString(int type)
{
	if((type >= V2R1_ONU_MAX) || (type < V2R1_ONU_GT811))
		return DEVICE_APP_TYPE_UNKNOWN;
	
	return(&(OnuAppType_1[type][0]));
}

char *GetOnuVoiceAppString(int type)
{
	if((type >= V2R1_ONU_MAX) || (type < V2R1_ONU_GT811))
		return DEVICE_APP_TYPE_UNKNOWN;
	
	return(&(OnuVoiceType_1[type][0]));
}

char *GetOnuFpgaAppString(int type)
{
	if((type >= V2R1_ONU_MAX) || (type < V2R1_ONU_GT811))
		return DEVICE_APP_TYPE_UNKNOWN;
	
	return(&(OnuFpgaType_1[type][0]));
}


char *GetOltProductSoftwareDesc()
{
	return(&ProductSoftwareDesc[0]);
}

char *GetOltDeviceNameString()
{
	return(&(OLTMgmt.DeviceInfo.DeviceName[0]));
}
char *GetOltDeviceDescString()
{
	return(&(OLTMgmt.DeviceInfo.DeviceDesc[0]));
}

/* OLT 板卡类型*/
/* sw*/
char *Get6700SwBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_SW][0]));
}
/*added by wangdp 20100113 支持10G*/
char *Get6700SwtgBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_SWTG][0]));
}

char *Get6100SwBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_MAIN][0]));
}
char *Get6700SwBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_SW][0]));
}

/*added by wangdp 20100113 支持10G*/
char *Get6700SwtgBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_SWTG][0]));
}
char *Get6100SwBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_MAIN][0]));
}

char *GetSwBoardNameString()
{
	int type;
	type = GetOltType();
	if(type == V2R1_OLT_GFA6700)
		{
		/*modify by wangdp 20100113,支持10G*/
#if 0		
		return(Get6700SwBoardNameString());
#else

		if(MODULE_E_GFA_SW == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO))
			return(Get6700SwBoardNameString());
		else if(MODULE_E_GFA_SWTG == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO))
			return(Get6700SwtgBoardNameString());
			
#endif			
		}
	else if(type == V2R1_OLT_GFA6100)
		return(Get6100SwBoardNameString());

	return BOARD_TYPE_UNKNOW_STR;
}
char *GetSwBoardNameStringShort()
{
	int type = GetOltType();
	if(type == V2R1_OLT_GFA6700)
	{
	/*modify by wangdp 20100113,支持10G*/
#if 0		
		return(Get6700SwBoardNameStringShort());
#else

		if(MODULE_E_GFA_SW == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO))
			return(Get6700SwBoardNameStringShort());
		else if(MODULE_E_GFA_SWTG == __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO))
			return(Get6700SwtgBoardNameStringShort());
			
#endif	
	}	
	else if(type == V2R1_OLT_GFA6100)
	{
		return(Get6100SwBoardNameStringShort());
	}

	return BOARD_TYPE_UNKNOW_SHORT_STR;
}


/* PON */
char *Get6700PonBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_EPON][0]));
}
char *Get6100PonBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_EPON][0]));
}
char *Get6700PonBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_EPON][0]));
}
char *Get6100PonBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_EPON][0]));
}

char *GetPonBoardNameString()
{
	int type;
	type = GetOltType();
	if(type == V2R1_OLT_GFA6700)
		return(Get6700PonBoardNameString());
	else if(type == V2R1_OLT_GFA6100)
		return(Get6100PonBoardNameString());
	else return BOARD_TYPE_UNKNOW_STR;
}
char *GetPonBoardNameStringShort()
{
	int type;
	type = GetOltType();
	if(type == V2R1_OLT_GFA6700)
		return(Get6700PonBoardNameStringShort());
	else if(type == V2R1_OLT_GFA6100)
		return(Get6100PonBoardNameStringShort());
	else return BOARD_TYPE_UNKNOW_SHORT_STR;
}

/* tdm */
char *Get6700SigBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_SIG][0]));
}
char *Get6100SigBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_SIG][0]));
}

char *Get6700SigBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_SIG][0]));
}
char *Get6100SigBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_SIG][0]));
}

char *GetSigBoardNameString()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(Get6700SigBoardNameString());
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(Get6100SigBoardNameString());
	
	return(MODULE_TYPE_UNKNOW_NAME_STR);	
}
char *GetSigBoardNameStringShort()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(Get6700SigBoardNameStringShort());
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(Get6100SigBoardNameStringShort());
	
	return(MODULE_TYPE_UNKNOW_SHORT_NAME_STR);	
}

char *Get6700E1BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_E1][0]));
}
char *Get6100E1BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_E1][0]));
}

char *Get6700E1BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_E1][0]));
}
char *Get6100E1BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_E1][0]));
}

char *GetE1BoardNameString()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(Get6700E1BoardNameString());
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(Get6100E1BoardNameString());
	
	return(MODULE_TYPE_UNKNOW_NAME_STR);	
}
char *GetE1BoardNameStringShort()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(Get6700E1BoardNameStringShort());
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(Get6100E1BoardNameStringShort());
	
	return(MODULE_TYPE_UNKNOW_SHORT_NAME_STR);	
}

/* 上联板*/
char *GetGeoBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_GEO][0]));
}
char *GetGeoBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_GEO][0]));
}
char *GetGetBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_GET][0]));
}
char *GetGetBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_GET][0]));
}
char *GetGemBoardNameString()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(&(OltBoardTypeString[MODULE_E_GFA_GEM][0]));
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(&(OltBoardTypeString[MODULE_E_GFA6100_GEM][0]));
	
	return(MODULE_TYPE_UNKNOW_NAME_STR);	
}
char *GetGemBoardNameStringShort()
{
	if(GetOltType() == V2R1_OLT_GFA6700)
		return(&(OltBoardTypeStringShort[MODULE_E_GFA_GEM][0]));
	else if(GetOltType() == V2R1_OLT_GFA6100)
		return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_GEM][0]));
	
	return(MODULE_TYPE_UNKNOW_SHORT_NAME_STR);	
}


/* 电源板 */
char *Get6700Pwu48NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_PWU48][0]));
}
char *Get6700Pwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_PWU48][0]));
}
char *Get6700Pwu220NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA_PWU220][0]));
}
char *Get6700Pwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA_PWU220][0]));
}

char *Get6100Pwu48NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_PWU48][0]));
}
char *Get6100Pwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_PWU48][0]));
}
char *Get6100Pwu220NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6100_PWU220][0]));
}
char *Get6100Pwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6100_PWU220][0]));
}
/* wangysh add 6900*/
#if 1
char *Get6900SwBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_SW][0]));
}
char *Get6900SwBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_SW][0]));
}
/* pon */
char *Get6900_4EponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_4EPON][0]));
}
char *Get6900_4EponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_4EPON][0]));
}
char *Get6900_8EponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_8EPON][0]));
}
char *Get6900_8EponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_8EPON][0]));
}
char *Get6900_12EponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_12EPON][0]));
}
char *Get6900_12EponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_12EPON][0]));
}

char *Get6900_12EponB0BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_12EPONB0][0]));
}
char *Get6900_12EponB0BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_12EPONB0][0]));
}

char *Get6900_16EponB1BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_16EPONB1][0]));
}
char *Get6900_16EponB1BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_16EPONB1][0]));
}

#if defined(_EPON_10G_PMC_SUPPORT_)            
/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
char *Get6900_10GEponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_10G_EPON][0]));
}
char *Get6900_10GEponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_10G_EPON][0]));
}
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#endif

char *Get6900_4Epon4geBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_4EPON_4GE][0]));
}
char *Get6900_4Epon4geBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_4EPON_4GE][0]));
}

/* wangysh add 6900 gem*/
char *Get6900GemBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_GE][0]));	
}
char *Get6900GemBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_GE][0]));
}

char *Get6900_10GemBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_10GE][0]));	
}
char *Get6900_10GemBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_10GE][0]));
}

char *Get6900Pwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900_PWU220][0]));
}

char *Get6900Pwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_PWU220][0]));
}

char *Get6900MPwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900M_PWU220][0]));
}

char *Get6900MPwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900M_PWU220][0]));
}

char *Get6900SPwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900S_PWU220][0]));
}

char *Get6900SPwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_PWU220][0]));
}
char *Get6900Pwu48NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900_PWU48][0]));
}

char *Get6900Pwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_PWU48][0]));
}

char *Get6900SPwu48NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900S_PWU48][0]));
}

char *Get6900SPwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_PWU48][0]));
}


char *Get6900FanNameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900_FAN][0]));
}

char *Get6900FanNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900_FAN][0]));
}

char *Get6900SFanNameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA6900S_FAN][0]));
}

char *Get6900SFanNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_FAN][0]));
}
#endif

#if 1
char *Get8000SwBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_SW][0]));
}
char *Get8000SwBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_SW][0]));
}
/* pon */
char *Get8000_4EponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_4EPON][0]));
}
char *Get8000_4EponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_4EPON][0]));
}
char *Get8000_12EponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_12EPON][0]));
}
char *Get8000_12EponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_12EPON][0]));
}

char *Get8000_12EponB0BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_12EPONB0][0]));
}
char *Get8000_12EponB0BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_12EPONB0][0]));
}

char *Get8000_16EponB1BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_16EPONB1][0]));
}
char *Get8000_16EponB1BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_16EPONB1][0]));
}

/*add for gpon @muqw*/
char *Get8000_16GponBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_16GPONB0][0]));
}
char *Get8000_16GponBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_16GPONB0][0]));
}
char *Get8000_4Epon4geBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_4EPON_4GE][0]));
}
char *Get8000_4Epon4geBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_4EPON_4GE][0]));
}

char *Get8000GemBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_GEM_GE][0]));	
}
char *Get8000GemBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_GEM_GE][0]));
}

char *Get8000_10GemBoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8000_GEM_10GE][0]));	
}
char *Get8000_10GemBoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_GEM_10GE][0]));
}

char *Get8000Pwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000_PWU220][0]));
}

char *Get8000Pwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_PWU220][0]));
}

char *Get8000MPwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000M_PWU220][0]));
}

char *Get8000MPwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000M_PWU220][0]));
}

char *Get8000SPwu220NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000S_PWU220][0]));
}

char *Get8000SPwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_PWU220][0]));
}
char *Get8000Pwu48NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000_PWU48][0]));
}

char *Get8000Pwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_PWU48][0]));
}

char *Get8000SPwu48NameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000S_PWU48][0]));
}

char *Get8000SPwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_PWU48][0]));
}

char *Get8000FanNameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000_FAN][0]));
}

char *Get8000FanNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000_FAN][0]));
}

char *Get8000SFanNameString()
{
    return(&(OltBoardTypeString[MODULE_E_GFA8000S_FAN][0]));
}

char *Get8000SFanNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_FAN][0]));
}
#endif

char *Get8100_16EponB0BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8100_16EPONB0][0]));
}
char *Get8100_16EponB0BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8100_16EPONB0][0]));
}

char *Get8100Pwu48NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8100_PWU48][0]));
}
char *Get8100Pwu48NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8100_PWU48][0]));
}


char *Get8100Pwu220NameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8100_PWU220][0]));
}
char *Get8100Pwu220NameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8100_PWU220][0]));
}

char *Get8100FanNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8100_FAN][0]));
}
char *Get8100FanNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8100_FAN][0]));
}


char *Get8100_16GponB0BoardNameString()
{
	return(&(OltBoardTypeString[MODULE_E_GFA8100_16GPONB0][0]));
}
char *Get8100_16GponB0BoardNameStringShort()
{
	return(&(OltBoardTypeStringShort[MODULE_E_GFA8100_16GPONB0][0]));
}





char *GetOltBoardString(int BoardType)
{
	if(BoardType >= MODULE_TYPE_MAX) return "unknown" ;
	return(&(OltBoardTypeString[BoardType][0]));
}

char *GetOltBoardStringShort(int BoardType)
{
	if(BoardType >= MODULE_TYPE_MAX) return "unknown" ;
	return(&(OltBoardTypeStringShort[BoardType][0]));
}


char *GetBoardNameString(int type)/*****add by   mengxsh  20140718 *****/
{
	if (type ==MODULE_TYPE_NULL)
	{
		return BOARD_TYPE_NULL_STR;
	}
	else if (type ==MODULE_TYPE_UNKNOW || type >= MODULE_TYPE_MAX)
	{
		return BOARD_TYPE_UNKNOW_STR;
	}
	if (type == MODULE_E_GFA_PWU48 && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_PWU48;
	}
	else if (type == MODULE_E_GFA_PWU220 && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_PWU220;
	}
	else if (type == MODULE_E_GFA_GEM && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_GEM;
	}
	return(&(OltBoardTypeString[type][0])); /*  在该函数  InitOltBoardStringInfoByDefault 中对该数组初始化 */
}	

char *GetBoardNameStringShort(int type)/*****add by   mengxsh  20140718 *****/
{
	if (type ==MODULE_TYPE_NULL)
	{
		return BOARD_TYPE_NULL_SHORT_STR;
	}
	else if (type ==MODULE_TYPE_UNKNOW || type >= MODULE_TYPE_MAX)
	{
		return BOARD_TYPE_UNKNOW_SHORT_STR;
	}
	if (type == MODULE_E_GFA_PWU48 && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_PWU48;
	}
	else if (type == MODULE_E_GFA_PWU220 && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_PWU220;
	}
	else if (type == MODULE_E_GFA_GEM && SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		type = MODULE_E_GFA6100_GEM;
	}
	return(&(OltBoardTypeStringShort[type][0])); /*  在该函数  InitOltBoardStringInfoByDefault 中对该数组初始化 */
}



/************  ONU ****************************/

/*   多子板ONU, 如 GT861 , 各子板类型定义
   0－空、
   1－GT-EPON-B、
   2－GT-8POTS-A（RJ11接口）、
   3－GT-6FE、
   4－GT-8FE、
   5－GT-16FE、
   6－GT-8FXS-A（RJ11接口），
   7－GT-8POTS-B（RJ21接口）；
   8－GT-8FXS-B（RJ21接口），

   注：
   1 - GT-EPON-B 为ONU主控板，其它为业务板。
   2 - GT-8POTS只是按接口类型划分（RJ11和RJ21），目前没有区分oem的板卡和我们公司的板卡。
*/

/* added by chenfj 2008-6-11
      支持GT861 等多子卡ONU  */

char *GetOnuSubBoardString(int SubType)
{
	if((SubType <0) || (SubType>= SUB_BOARD_GT_LAST ))
		return (&(OnuBoardTypeString[SUB_BOARD_NULL][0]));
	return(&(OnuBoardTypeString[SubType][0]));
}

char *GetOnuBoardStringEpon()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_EPON_B][0]));
}

char *GetOnuBoardString8Pots_A()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8POTS_A][0]));
}

char *GetOnuBoardString6Fe()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_6FE][0]));
}

char *GetOnuBoardString8Fe()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8FE][0]));
}

char *GetOnuBoardString16Fe()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_16FE][0]));
}

char *GetOnuBoardString8Fxs_A()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8FXS_A][0]));
}
char *GetOnuBoardString8Pots_B()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8POTS_B][0]));
}
char *GetOnuBoardString8Fxs_B()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8FXS_B][0]));
}
char *GetOnuBoardString8Potso_A()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8POTSO_A][0]));
}
char *GetOnuBoardString8Potso_B()
{
	return(&(OnuBoardTypeString[SUB_BOARD_GT_8POTSO_B][0]));
}

char *GetOnuBoardString4E1_75()
{
	return(&(OnuBoardTypeString[SUB_BOARD_4E1_75OHM][0]));
}
char *GetOnuBoardString4E1_120()
{
	return(&(OnuBoardTypeString[SUB_BOARD_4E1_120OHM][0]));
}

char *GetGT813TypeString()
{
	return(GetDeviceTypeString(V2R1_ONU_GT813));
}

char *GetGT813DescString()
{
	return(GetDeviceDescString(V2R1_ONU_GT813));
}

char *GetGT831TypeString()
{
	return(GetDeviceTypeString(V2R1_ONU_GT831));
}

char *GetGT831DescString()
{
	return(GetDeviceDescString(V2R1_ONU_GT831));
}

char *GetGT865TypeString()
{
	return(GetDeviceTypeString(V2R1_ONU_GT865));
}
char *GetGT865DescString()
{
	return(GetDeviceDescString(V2R1_ONU_GT865));
}

/*add by shixh20090828*/
char *GetGT863TypeString()
{
  return (GetDeviceTypeString(V2R1_ONU_GT863));
}
char *GetGT863DescString()
{
return  (GetDeviceDescString(V2R1_ONU_GT863));
}

#ifdef  OLT_MGMT_INFO_BY_DEFAULT_
#endif
static void  ClearOltDeviceDescArray(void)
{
#if 0
	int i;
	for(i=0; i<= V2R1_OLT_GFA6100; i++)
	{
		VOS_StrCpy((char *)&(DeviceDesc_1[i][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	}
#else
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA6700][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);    
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA6100][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);    
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA6900][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA6900M][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA6900S][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA8000][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA8000M][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA8000S][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceDesc_1[V2R1_OLT_GFA8100][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
#endif
	return;
}

static void  InitOltDeviceDescByDefault(void)
{
	ClearOltDeviceDescArray();
	
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA6700][0]), 	DEVICE_TYPE_DESC_GFA6700_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA6100][0]), 	DEVICE_TYPE_DESC_GFA6100_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA6900][0]), 	DEVICE_TYPE_DESC_GFA6900_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA6900M][0]), 	DEVICE_TYPE_DESC_GFA6900M_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA6900S][0]), 	DEVICE_TYPE_DESC_GFA6900S_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA8000][0]), 	DEVICE_TYPE_DESC_GFA8000_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA8000M][0]), 	DEVICE_TYPE_DESC_GFA8000M_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA8000S][0]), 	DEVICE_TYPE_DESC_GFA8000S_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_OLT_GFA8100][0]), 	DEVICE_TYPE_DESC_GFA8100_STR_default);
	return;
};

static void  ClearOltDeviceTypeArray(void)
{
#if 0
	int i;
	for(i=0; i< V2R1_OLT_GFA6100; i++)
		{
		VOS_StrCpy((char *)&(DeviceType_1[i][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
		}
#else
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA6100][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA6700][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA6900][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA6900M][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA6900S][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA8000][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA8000M][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA8000S][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
	VOS_StrCpy((char *)&(DeviceType_1[V2R1_OLT_GFA8100][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);

#endif
	return;
}
static void  InitOltDeviceTypeByDefault(void)
{
	ClearOltDeviceTypeArray();
	
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA6700][0]),	DEVICE_TYPE_NAME_GFA6700_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA6100][0]), 	DEVICE_TYPE_NAME_GFA6100_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA6900][0]), 	DEVICE_TYPE_NAME_GFA6900_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA6900M][0]), 	DEVICE_TYPE_NAME_GFA6900M_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA6900S][0]), 	DEVICE_TYPE_NAME_GFA6900S_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA8000][0]), 	DEVICE_TYPE_NAME_GFA8000_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA8000M][0]), 	DEVICE_TYPE_NAME_GFA8000M_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA8000S][0]), 	DEVICE_TYPE_NAME_GFA8000S_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_OLT_GFA8100][0]), 	DEVICE_TYPE_NAME_GFA8100_STR_default);
	return;
}

static void InitOltDeviceInfoByDefault()
{
	VOS_StrCpy((char *)&OltVendorInfo[0], OLT_VENDOR_INFO_DEFAULT_default);
	VOS_StrCpy((char *)&OltVendorLocation[0], OLT_VENDOR_LOCATION_default);
	VOS_StrCpy((char *)&ProductCorporationName_AB[0], PRODUCT_CORPORATION_AB_NAME_default);
	VOS_StrCpy((char *)&ProductCorporationName_Short[0], PRODUCT_CORPORATION_SHORT_NAME_default);
	VOS_StrCpy((char *)&ProductCorporationName[0], PRODUCT_CORPORATION_NAME_default);

	VOS_StrCpy((char *)&V2R1Series[0], EPON_V2R1_SERIES_default);
	VOS_StrCpy((char *)&V2R1Copyright[0], EPON_V2R1_COPYRIGHT_default);
	VOS_StrCpy((char *)&ProductSoftwareDesc[0], PRODUCT_OLT_SOFTWARE_DESC_STR_default);

	return;
}

static void  InitOltBoardStringInfoByDefault(void)
{	
	int i;

	for(i=0; i<MODULE_TYPE_MAX; i++)
		VOS_StrCpy(&(OltBoardTypeString[i][0]), "unknown");
		
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_SW][0]),"GFA-SW");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_SW][0]),"SW");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_EPON][0]),"GFA-EPON");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_EPON][0]),"EPN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_GET][0]),"GFA-GET" );
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_GET][0]),"GET" );

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_GEO][0]),"GFA-GEO" );
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_GEO][0]),"GEO" );

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_SIG][0]),"GFA-SIG" );
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_SIG][0]),"SIG" );

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_PWU48][0]),"GFA-PWU48" );
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_PWU48][0]),"P48" );

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_PWU220][0]),"GFA-PWU220" );
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_PWU220][0]),"P22" );

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_MAIN][0]),"GFA6100-MAIN");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_MAIN][0]),"MAN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_EPON][0]),"GFA6100-EPON");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_EPON][0]),"EPN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_SIG][0]),"GFA6100-SIG");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_SIG][0]),"SIG");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_GEM][0]),"GFA-GEM");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_GEM][0]),"GEM");
	
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_GEM][0]),"GFA6100-GEM");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_GEM][0]),"GEM");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_E1][0]),"GFA-TDM");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_E1][0]),"TDM");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_E1][0]),"GFA6100-TDM");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_E1][0]),"E1");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_PWU48][0]),"GFA6100-PWR48");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6100_PWU220][0]),"GFA6100-PWR220");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6100_PWU220][0]),"P22");

/*wangdp 20100113 支持10G*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA_SWTG][0]),"GFA-SWTG");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA_SWTG][0]),"SWTG");

#if 1
/* wangysh 6900*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_SW][0]),"GFA6900-SW-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_SW][0]),"SW");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_8EPON][0]),"GFA6900-8EPON-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_8EPON][0]),"8-EPN");
	
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_12EPONB0][0]),"GFA6900-12EPON-B0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_12EPONB0][0]),"12-EPN");
/******为了8000 产品中兼容 以下单板   add by   mengxsh  20141028  *****/
	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)
	{
		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_4EPON][0]),"GFA6900-4EPON-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_4EPON][0]),"4-EPN");
		
		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_12EPON][0]),"GFA6900-12EPON-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_12EPON][0]),"12-EPN");


		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_16EPONB1][0]),"GFA6900-16EPON-B1");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_16EPONB1][0]),"16-EPN");

		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_GE][0]),"GFA6900-4GE-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_GE][0]),"GEM-GE");

		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_10GE][0]),"GFA6900-10GE-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_10GE][0]),"GEM-10GE");
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
	{	
		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_4EPON][0]),"GFA6900-4EPON-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_4EPON][0]),"4-EPN");
				
		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_12EPON][0]),"GFA6900-12EPON-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_12EPON][0]),"12-EPN");
		
		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_16EPONB1][0]),"GFA6900-16EPON-B1");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_16EPONB1][0]),"16-EPN");

		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_GE][0]),"GFA6900-4GE-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_GE][0]),"GEM-GE");

		VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_GEM_10GE][0]),"GFA6900-10GE-A0");
		VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_GEM_10GE][0]),"GEM-10GE");
	}


	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_4EPON_4GE][0]),"GFA6900-4EPON-B0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_4EPON_4GE][0]),"4-EPN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_PWU48][0]),"GFA6900-PWU-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_PWU220][0]),"GFA6900-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900M_PWU220][0]),"GFA6900M-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900M_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_FAN][0]),"GFA6900-FAN-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_FAN][0]),"FAN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900S_PWU48][0]),"GFA6900S-PWU-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900S_PWU220][0]),"GFA6900S-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900S_FAN][0]),"GFA6900S-FAN-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900S_FAN][0]),"FAN");

#if defined(_EPON_10G_PMC_SUPPORT_)            
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA6900_10G_EPON][0]),"GFA6900-10G-EPON");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA6900_10G_EPON][0]),"10G-EPN");
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif	
#endif	

#if 1
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_SW][0]),"GFA8000-SW-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_SW][0]),"SW");
/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
/*
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_4EPON][0]),"GFA8000-4EPON-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_4EPON][0]),"4-EPN");
*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_4EPON_4GE][0]),"GFA8000-4EPON-B0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_4EPON_4GE][0]),"4-EPN");

/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
/*
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_12EPON][0]),"GFA8000-12EPON-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_12EPON][0]),"12-EPN");
*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_12EPONB0][0]),"GFA8000-12EPON-B0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_12EPONB0][0]),"12-EPN");
/*****为了去掉8000 产品中多余的宏定义而临时 修改 add by   mengxsh  20141028  sheng *****/
/*
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_16EPONB1][0]),"GFA8000-16EPON-B1");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_16EPONB1][0]),"16-EPN");
*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_10G_8EPON][0]),"GFA8000-8XEP-B1");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_10G_8EPON][0]),"8XEP");
/*
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_GEM_GE][0]),"GFA8000-4GE-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_GEM_GE][0]),"GEM-GE");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_GEM_10GE][0]),"GFA8000-10GE-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_GEM_10GE][0]),"GEM-10GE");
*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_PWU48][0]),"GFA8000-PWU-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_PWU220][0]),"GFA8000-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000M_PWU220][0]),"GFA8000M-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000M_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_FAN][0]),"GFA8000-FAN-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_FAN][0]),"FAN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000S_PWU48][0]),"GFA8000S-PWU-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000S_PWU220][0]),"GFA8000S-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000S_FAN][0]),"GFA8000S-FAN-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000S_FAN][0]),"FAN");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_8XET][0]),"GFA8000-8XET-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_8XET][0]),"8XET");
	
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_4XET][0]),"GFA8000-4XET-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_4XET][0]),"4XET");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_8XETA1][0]),"GFA8000-8XET-A1");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_8XETA1][0]),"8XET");
	
	 /*EEPROM 发布文件中和系统显示的16GPON-B1  在软件中板卡类型定义为16GPON-B0*/
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8000_16GPONB0][0]),"GFA8000-16GPON-B1");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8000_16GPONB0][0]),"16-GPN");
#endif	

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8100_16EPONB0][0]),"GFA8100-16EP");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8100_16EPONB0][0]),"16-EP");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8100_PWU48][0]),"GFA8100-PWU-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8100_PWU48][0]),"P48");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8100_PWU220][0]),"GFA8100-PWU220-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8100_PWU220][0]),"P220");

	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8100_FAN][0]),"GFA8100-FAN-A0");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8100_FAN][0]),"FAN");
	VOS_StrCpy(&(OltBoardTypeString[MODULE_E_GFA8100_16GPONB0][0]),"GFA8100-16GP");
	VOS_StrCpy(&(OltBoardTypeStringShort[MODULE_E_GFA8100_16GPONB0][0]),"16-GP");

	return;
}

/* 使用默认值初始化OLT 管理信息*/
void InitOltMgmtInfoByDefault(void)
{
	InitOltDeviceDescByDefault();
	InitOltDeviceTypeByDefault();
	InitOltDeviceInfoByDefault();
	InitOltBoardStringInfoByDefault();
	
	return;
}

/* 清空OLT 管理表中所有信息*/
void  OltMgmtInfoClear()
{
	int i;
	
	ClearOltDeviceDescArray();
	ClearOltDeviceTypeArray();

	VOS_MemZero((char *)&OltVendorInfo[0], sizeof(OltVendorInfo));
	VOS_MemZero((char *)&OltVendorLocation[0], sizeof(OltVendorLocation));
	VOS_MemZero((char *)&ProductCorporationName_AB[0], sizeof(ProductCorporationName_AB));
	VOS_MemZero((char *)&ProductCorporationName_Short[0], sizeof(ProductCorporationName_Short));
	VOS_MemZero((char *)&ProductCorporationName[0], sizeof(ProductCorporationName));

	VOS_MemZero((char *)&V2R1Series[0], sizeof(V2R1Series));
	VOS_MemZero((char *)&V2R1Copyright[0], sizeof(V2R1Copyright));
	VOS_MemZero((char *)&ProductSoftwareDesc[0], sizeof(ProductSoftwareDesc));

	for(i=0; i<MODULE_TYPE_MAX; i++)
		VOS_StrCpy(&(OltBoardTypeString[i][0]), "unknown");
}

/***   以下函数用于使用系统文件来初始化OLT 设备信息*/
#ifdef  OLT_MGMT_INFO_BY_SYSFILE_
#endif

/* 设置vendor 信息；对应关键字[VENDOR_INFO] */
void  InitOltVendorInfoBySysfile(char *VendorInfo, int len)
{
	if(VendorInfo == NULL)
		return;
	
	if(len > MAXVENDORINFOLEN)
		len = MAXVENDORINFOLEN;
	
	SetOltVendorInfo( VendorInfo, len);	
	return;
}

/* 设置vendor地址：对应关键字[VENDOR_LOCATION] */
void InitOltVendorLocationBySysfile(char *VendorLocation)
{
	int  length;

	if(VendorLocation == NULL)
		return;
	
	length = VOS_StrLen(VendorLocation);
	if(length > MAXLOCATIONLEN)
		length = MAXLOCATIONLEN;	
	
	VOS_StrnCpy(&OltVendorLocation[0],VendorLocation, length);
	OltVendorLocation[length] = '\0';
	
	return;
}

/* 设置vendor 版权信息，对应关键字[VENDOR_COPYRIGHT] */
void InitOltVendorCopyrightBySysfile(char  *Copyright )
{
	int  length;

	if(Copyright == NULL)
		return;
	
	length = VOS_StrLen(Copyright);
	if(length > MAXV2r1CopyrightLEN)
		length = MAXV2r1CopyrightLEN;	
	
	VOS_StrnCpy(&V2R1Copyright[0],Copyright, length);
	V2R1Copyright[length] = '\0';
	
	return;
}

/* 设置 vendor 名称，对应关键字[VENDOR_NAME] */
void InitOltVendorNameBySysfile(char *Name)
{
	int  length;

	if(Name == NULL)
		return;
	
	length = VOS_StrLen(Name);
	if(length > MAXCorporationNameLEN)
		length = MAXCorporationNameLEN;	
	
	VOS_StrnCpy(&ProductCorporationName[0],Name, length);
	ProductCorporationName[length] = '\0';
	
	return;
}

/* 设置 vendor 名称简写，对应关键字[VENDOR_NAME_SHORT] */
void InitOltVendorShortNameBySysfile(char *Name)
{
	int  length;

	if(Name == NULL)
		return;
	
	length = VOS_StrLen(Name);
	if(length > MAXCorporationNameLENShort)
		length = MAXCorporationNameLENShort;	
	
	VOS_StrnCpy(&ProductCorporationName_Short[0],Name, length);
	ProductCorporationName_Short[length] = '\0';
	
	return;
}

/* 设置 vendor 名称缩写，对应关键字[VENDOR_NAME_AB] */
void InitOltVendorABNameBySysfile(char *Name)
{
	int  length;

	if(Name == NULL)
		return;

	length = VOS_StrLen(Name);
	if(length > MAXCorporationNameLenAB)
		length = MAXCorporationNameLenAB;
	
	VOS_StrnCpy(&ProductCorporationName_AB[0],Name, length);
	ProductCorporationName_AB[length] = '\0';
	
	return;
}

/* 设置产品系列，对应关键字 [PRODUCT_SERIES] */
void InitOltProductSeriesBySysfile(char *Series)
{
	int  length;

	if(Series == NULL)
		return;

	length = VOS_StrLen(Series);
	if(length > MAXV2R1SeriesLEN)
		length = MAXV2R1SeriesLEN;
	
	VOS_StrnCpy(&V2R1Series[0],Series, length);
	V2R1Series[length] = '\0';
	
	return;
}

/* 设置产品软件，对应关键字[PRODUCT_SOFTWARE]  */
void InitOltProductSoftwareBySysfile(char *Software)
{
	int  length;

	if(Software == NULL)
		return;

	length = VOS_StrLen(Software);
	if(length > MAXProductSoftwareDescLen)
		length = MAXProductSoftwareDescLen;
	
	VOS_StrnCpy(&ProductSoftwareDesc[0],Software, length);
	ProductSoftwareDesc[length] = '\0';
		
	return;
}

/*  OLT 产品类型，对应关键字[PRODUCT_TYPE_6700], [PRODUCT_TYPE_6100] */
void InitOltDeviceTypeStringBySysfile(int DeviceType, char *TypeString)
{
	int length;
	
	if((DeviceType >= V2R1_DEVICE_LAST) || (DeviceType < V2R1_OTHER))
		return;
	if(TypeString == NULL)
		return;
	
	length = VOS_StrLen(TypeString);
	if(length > ONU_TYPE_LEN )
		length = ONU_TYPE_LEN;
	
	VOS_StrnCpy(&(DeviceType_1[DeviceType][0]), TypeString, length);
	DeviceType_1[DeviceType][length] = '\0';

	return;
}
/*  olt 产品描述，对应关键字[PRODUCT_DESCRIPTION_6700], [PRODUCT_DESCRIPTION_6100] */
void InitOltDeviceDescStringBySysfile(int DeviceType, char *DescString)
{
	int length;
	
	if((DeviceType >= V2R1_DEVICE_LAST) || (DeviceType < V2R1_OTHER))
		return;
	if(DescString == NULL)
		return;

	length = VOS_StrLen(DescString);
	if(length > MAXDEVICEDESCLEN )
		length = MAXDEVICEDESCLEN;
	
	VOS_StrnCpy(&(DeviceDesc_1[DeviceType][0]), DescString, length);
	DeviceDesc_1[DeviceType][length] = '\0';
	
	return;
}

/* OLT 侧板卡名称，对应关键字OLT_BOARD */
void InitOltBoardTypeStringBySysfile(int  BoardType, char *TypeString)
{
	int length;

	if((BoardType < MODULE_E_GFA_SW ) || (BoardType >= MODULE_TYPE_MAX )) 
		return;
	if(TypeString == NULL)
		return;


	length = VOS_StrLen(TypeString);
	if(length > BOARDTYPESTRINGLEN )
		length = BOARDTYPESTRINGLEN;

	VOS_StrnCpy(&(OltBoardTypeString[BoardType][0]), TypeString, length);
	OltBoardTypeString[BoardType][length] = '\0';
	
	return;
}

/* OLT 侧板卡名称，对应关键字OLT_BOARD */
void InitOltBoardTypeStringShortBySysfile(int  BoardType, char *TypeString)
{
	int length;

	if((BoardType < MODULE_E_GFA_SW ) || (BoardType >= MODULE_TYPE_MAX )) 
		return;
	if(TypeString == NULL)
		return;

	length = VOS_StrLen(TypeString);
	if(length > BOARDTYPESTRINGLEN_SHORT ) length = BOARDTYPESTRINGLEN_SHORT;

	VOS_StrnCpy(&(OltBoardTypeStringShort[BoardType][0]), TypeString, length);
	OltBoardTypeStringShort[BoardType][length] = '\0';
	
	return;
}

#ifdef OLT_MGMT_INFO_API_
#endif
/* OLT 侧板卡类型关键字与板卡类型转换*/
int  OltBoardTypeKeywordToInteger(char *TypeKeyword)
{
	if(TypeKeyword == NULL)
		return( MODULE_TYPE_UNKNOW);

#if 1
	/* 6900 */
	if((VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6900", VOS_StrLen("BOARD_MAIN_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6900_SHORT", VOS_StrLen("BOARD_MAIN_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_SW;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_6900", VOS_StrLen("BOARD_4_EPON_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_6900_SHORT", VOS_StrLen("BOARD_4_EPON_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_4EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_8_EPON_6900", VOS_StrLen("BOARD_8_EPON_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_8_EPON_6900_SHORT", VOS_StrLen("BOARD_8_EPON_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_8EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_6900", VOS_StrLen("BOARD_12_EPON_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_6900_SHORT", VOS_StrLen("BOARD_12_EPON_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_12EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_B0_6900", VOS_StrLen("BOARD_12_EPON_B0_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_B0_6900_SHORT", VOS_StrLen("BOARD_12_EPON_B0_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_12EPONB0;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B1_6900", VOS_StrLen("BOARD_16_EPON_B1_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B1_6900_SHORT", VOS_StrLen("BOARD_16_EPON_B1_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_16EPONB1;
#if defined(_EPON_10G_PMC_SUPPORT_)            
	/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_10G_EPON_6900", VOS_StrLen("BOARD_10G_EPON_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_10G_EPON_6900_SHORT", VOS_StrLen("BOARD_10G_EPON_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_10G_EPON;
	/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#endif
	if((VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_4GE_6900", VOS_StrLen("BOARD_4_EPON_4GE_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_4GE_6900_SHORT", VOS_StrLen("BOARD_4_EPON_4GE_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_4EPON_4GE;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6900", VOS_StrLen("BOARD_UPLINK_M_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6900_SHORT", VOS_StrLen("BOARD_UPLINK_M_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_GEM_GE;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_10_M_6900", VOS_StrLen("BOARD_UPLINK_10_M_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_10_M_6900_SHORT", VOS_StrLen("BOARD_UPLINK_10_M_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_GEM_10GE;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6900", VOS_StrLen("BOARD_AC220_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6900_SHORT", VOS_StrLen("BOARD_AC220_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_M6900", VOS_StrLen("BOARD_AC220_M6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_M6900_SHORT", VOS_StrLen("BOARD_AC220_M6900_SHORT")) == 0))
		return MODULE_E_GFA6900M_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_S6900", VOS_StrLen("BOARD_AC220_S6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_S6900_SHORT", VOS_StrLen("BOARD_AC220_S6900_SHORT")) == 0))
		return MODULE_E_GFA6900S_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6900", VOS_StrLen("BOARD_DC48_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6900_SHORT", VOS_StrLen("BOARD_DC48_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_PWU48;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_S6900", VOS_StrLen("BOARD_DC48_S6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_S6900_SHORT", VOS_StrLen("BOARD_DC48_S6900_SHORT")) == 0))
		return MODULE_E_GFA6900S_PWU48;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_FAN_6900", VOS_StrLen("BOARD_FAN_6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_FAN_6900_SHORT", VOS_StrLen("BOARD_FAN_6900_SHORT")) == 0))
		return MODULE_E_GFA6900_FAN;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_FAN_S6900", VOS_StrLen("BOARD_FAN_S6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_FAN_S6900_SHORT", VOS_StrLen("BOARD_FAN_S6900_SHORT")) == 0))
		return MODULE_E_GFA6900S_FAN;
#endif
#if 1
	/* 8000 */
	if((VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_8000", VOS_StrLen("BOARD_MAIN_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_8000_SHORT", VOS_StrLen("BOARD_MAIN_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_SW;
/*****8000 产品中兼容MODULE_E_GFA6900_4EPON  板子   add by   mengxsh  20141028  *****/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_6900_8000", VOS_StrLen("BOARD_4_EPON_6900_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_6900_8000_SHORT", VOS_StrLen("BOARD_4_EPON_6900_8000_SHORT")) == 0))
		return MODULE_E_GFA6900_4EPON;
	
/*****8000 产品中兼容MODULE_E_GFA6900_12EPON  板子	add by	 mengxsh  20141028	*****/	
	if((VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_6900_8000", VOS_StrLen("BOARD_12_EPON_6900_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_6900_8000_SHORT", VOS_StrLen("BOARD_12_EPON_6900_8000_SHORT")) == 0))
		return MODULE_E_GFA6900_12EPON;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_B0_8000", VOS_StrLen("BOARD_12_EPON_B0_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_12_EPON_B0_8000_SHORT", VOS_StrLen("BOARD_12_EPON_B0_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_12EPONB0;
/******8000 产品中兼容MODULE_E_GFA6900_16EPONB1  板子   add by   mengxsh  20141028  *****/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B1_6900_8000", VOS_StrLen("BOARD_16_EPON_B1_6900_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B1_6900_8000_SHORT", VOS_StrLen("BOARD_16_EPON_B1_6900_8000_SHORT")) == 0))
		return MODULE_E_GFA6900_16EPONB1;
	
	if((VOS_StrnCmp(TypeKeyword, "BOARD_10G_8EPON_8000", VOS_StrLen("BOARD_10G_8EPON_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_10G_8EPON_8000_SHORT", VOS_StrLen("BOARD_10G_8EPON_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_10G_8EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_4GE_8000", VOS_StrLen("BOARD_4_EPON_4GE_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_4_EPON_4GE_8000_SHORT", VOS_StrLen("BOARD_4_EPON_4GE_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_4EPON_4GE;
	
/******8000 产品中兼容MODULE_E_GFA6900_GEM_GE  板子   add by   mengxsh  20141028  *****/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6900_8000", VOS_StrLen("BOARD_UPLINK_M_6900_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6900_8000_SHORT", VOS_StrLen("BOARD_UPLINK_M_6900_8000_SHORT")) == 0))
		return MODULE_E_GFA6900_GEM_GE;
	
/******8000 产品中兼容MODULE_E_GFA6900_GEM_10GE  板子   add by   mengxsh  20141028  *****/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_10_M_6900_8000", VOS_StrLen("BOARD_UPLINK_10_M_6900_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_10_M_6900_8000_SHORT", VOS_StrLen("BOARD_UPLINK_10_M_6900_8000_SHORT")) == 0))
		return MODULE_E_GFA6900_GEM_10GE;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_8000", VOS_StrLen("BOARD_AC220_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_8000_SHORT", VOS_StrLen("BOARD_AC220_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_M8000", VOS_StrLen("BOARD_AC220_M8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_M8000_SHORT", VOS_StrLen("BOARD_AC220_M8000_SHORT")) == 0))
		return MODULE_E_GFA8000M_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_S6900", VOS_StrLen("BOARD_AC220_S6900")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_S6900_SHORT", VOS_StrLen("BOARD_AC220_S6900_SHORT")) == 0))
		return MODULE_E_GFA6900S_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_8000", VOS_StrLen("BOARD_DC48_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_8000_SHORT", VOS_StrLen("BOARD_DC48_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_PWU48;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_S8000", VOS_StrLen("BOARD_DC48_S8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_S8000_SHORT", VOS_StrLen("BOARD_DC48_S8000_SHORT")) == 0))
		return MODULE_E_GFA8000S_PWU48;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_FAN_8000", VOS_StrLen("BOARD_FAN_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_FAN_8000_SHORT", VOS_StrLen("BOARD_FAN_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_FAN;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_FAN_S8000", VOS_StrLen("BOARD_FAN_S8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_FAN_S8000_SHORT", VOS_StrLen("BOARD_FAN_S8000_SHORT")) == 0))
		return MODULE_E_GFA8000S_FAN;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_8XET_8000", VOS_StrLen("BOARD_UPLINK_8XET_8000")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_8XET_8000_SHORT", VOS_StrLen("BOARD_UPLINK_8XET_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_8XET;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_4XET_8000", VOS_StrLen("BOARD_UPLINK_4XET_8000")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_4XET_8000_SHORT", VOS_StrLen("BOARD_UPLINK_4XET_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_4XET;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_8XET_A1_8000", VOS_StrLen("BOARD_UPLINK_8XET_A1_8000")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_8XET_A1_8000_SHORT", VOS_StrLen("BOARD_UPLINK_8XET_A1_8000_SHORT")) == 0))
		return MODULE_E_GFA8000_8XETA1;

    if((VOS_StrnCmp(TypeKeyword, "BOARD_16_GPON_B1_8000", VOS_StrLen("BOARD_16_GPON_B1_8000")) == 0)
    ||(VOS_StrnCmp(TypeKeyword, "BOARD_16_GPON_B1_8000_SHORT", VOS_StrLen("BOARD_16_GPON_B1_8000_SHORT")) == 0))
        return MODULE_E_GFA8000_16GPONB0;

	
	/*8100*/
	if((VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B0_8100", VOS_StrLen("BOARD_16_EPON_B0_8100")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_16_EPON_B0_8100_SHORT", VOS_StrLen("BOARD_16_EPON_B0_8100_SHORT")) == 0))
		return MODULE_E_GFA8100_16EPONB0;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_FAN_8100", VOS_StrLen("BOARD_FAN_8100")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_FAN_8100_SHORT", VOS_StrLen("BOARD_FAN_8100_SHORT")) == 0))
		return MODULE_E_GFA8100_FAN;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_8100", VOS_StrLen("BOARD_AC220_8100")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_8100_SHORT", VOS_StrLen("BOARD_AC220_8100_SHORT")) == 0))
		return MODULE_E_GFA8100_PWU220;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_8100", VOS_StrLen("BOARD_DC48_8100")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_8100_SHORT", VOS_StrLen("BOARD_DC48_8100_SHORT")) == 0))
		return MODULE_E_GFA8100_PWU48;

	if((VOS_StrnCmp(TypeKeyword, "BOARD_16_GPON_B0_8100", VOS_StrLen("BOARD_16_GPON_B0_8100")) == 0)
	||(VOS_StrnCmp(TypeKeyword, "BOARD_16_GPON_B0_8100_SHORT", VOS_StrLen("BOARD_16_GPON_B0_8100_SHORT")) == 0))
		return MODULE_E_GFA8100_16GPONB0;
		
#endif
	/* 6700 */
	if((VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6700", VOS_StrLen("BOARD_MAIN_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6700_SHORT", VOS_StrLen("BOARD_MAIN_6700_SHORT")) == 0))
		return MODULE_E_GFA_SW;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_EPON_6700", VOS_StrLen("BOARD_EPON_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_EPON_6700_SHORT", VOS_StrLen("BOARD_EPON_6700_SHORT")) == 0))
		return MODULE_E_GFA_EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_TDM_E1_6700", VOS_StrLen("BOARD_TDM_E1_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_TDM_E1_6700_SHORT", VOS_StrLen("BOARD_TDM_E1_6700_SHORT")) == 0))
		return MODULE_E_GFA_E1;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_TDM_SIG_6700", VOS_StrLen("BOARD_TDM_SIG_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_TDM_SIG_6700_SHORT", VOS_StrLen("BOARD_TDM_SIG_6700_SHORT")) == 0))
		return MODULE_E_GFA_SIG;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_T_6700", VOS_StrLen("BOARD_UPLINK_T_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_T_6700_SHORT", VOS_StrLen("BOARD_UPLINK_T_6700_SHORT")) == 0))
		return MODULE_E_GFA_GET;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_O_6700", VOS_StrLen("BOARD_UPLINK_O_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_O_6700_SHORT", VOS_StrLen("BOARD_UPLINK_O_6700_SHORT")) == 0))
		return MODULE_E_GFA_GEO;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6700", VOS_StrLen("BOARD_UPLINK_M_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6700_SHORT", VOS_StrLen("BOARD_UPLINK_M_6700_SHORT")) == 0))
		return MODULE_E_GFA_GEM;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6700", VOS_StrLen("BOARD_AC220_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6700_SHORT", VOS_StrLen("BOARD_AC220_6700_SHORT")) == 0))
		return MODULE_E_GFA_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6700", VOS_StrLen("BOARD_DC48_6700")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6700_SHORT", VOS_StrLen("BOARD_DC48_6700_SHORT")) == 0))
		return MODULE_E_GFA_PWU48;

	/* 6100 */
	if((VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6100", VOS_StrLen("BOARD_MAIN_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_MAIN_6100_SHORT", VOS_StrLen("BOARD_MAIN_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_MAIN;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_EPON_6100", VOS_StrLen("BOARD_EPON_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_EPON_6100_SHORT", VOS_StrLen("BOARD_EPON_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_EPON;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_TDM_E1_6100", VOS_StrLen("BOARD_TDM_E1_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_TDM_E1_6100_SHORT", VOS_StrLen("BOARD_TDM_E1_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_E1;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_TDM_SIG_6100", VOS_StrLen("BOARD_TDM_SIG_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_TDM_SIG_6100_SHORT", VOS_StrLen("BOARD_TDM_SIG_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_SIG;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6100", VOS_StrLen("BOARD_UPLINK_M_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_UPLINK_M_6100_SHORT", VOS_StrLen("BOARD_UPLINK_M_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_GEM;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6100", VOS_StrLen("BOARD_AC220_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_AC220_6100_SHORT", VOS_StrLen("BOARD_AC220_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_PWU220;
	if((VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6100", VOS_StrLen("BOARD_DC48_6100")) == 0)
		||(VOS_StrnCmp(TypeKeyword, "BOARD_DC48_6100_SHORT", VOS_StrLen("BOARD_DC48_6100_SHORT")) == 0))
		return MODULE_E_GFA6100_PWU48;

	return MODULE_TYPE_NULL;
}

/* 本函数用于兼容OLT 应用程序头标识中的关键字*/
int CompareOltAppKeyId(char *AppKeyId)
{
	if((VOS_MemCmp(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID), AppKeyId, VOS_StrLen(typesdb_product_file_key_name(PRODUCT_OLT_SWAPP_KEY_ID))) == 0)
		||(VOS_MemCmp(PRODUCT_OLT_SWAPP_KEY_NAME_GW,AppKeyId,VOS_StrLen(PRODUCT_OLT_SWAPP_KEY_NAME_GW)) == 0)
		||(VOS_MemCmp(PRODUCT_OLT_SWAPP_KEY_NAME_GDC,AppKeyId,VOS_StrLen(PRODUCT_OLT_SWAPP_KEY_NAME_GDC)) == 0)
		||(VOS_MemCmp(PRODUCT_OLT_SWAPP_KEY_NAME_RTC,AppKeyId,VOS_StrLen(PRODUCT_OLT_SWAPP_KEY_NAME_RTC)) == 0)
		||(VOS_MemCmp(PRODUCT_OLT_SWAPP_KEY_NAME_HDM,AppKeyId,VOS_StrLen(PRODUCT_OLT_SWAPP_KEY_NAME_HDM)) == 0) )
		return ROK;
	else return RERROR;		
}


/***  使用默认值初始化onu 管理信息*/

#ifdef ONU_MGMT_INFO_BY_DEFAULT_
#endif

/***  使用默认值初始化onu 板卡类型*/
static void InitOnuBoardStringInfoByDefault(void)
{
	int i;

	for(i=0; i<SUB_BOARD_GT_LAST; i++)
		VOS_StrCpy(&(OnuBoardTypeString[i][0]), "unknown");

	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_EPON_B][0]), (char *)"GT-EPON-B");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8POTS_A][0]), (char *)"GT-8POTS_A");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_6FE][0]), (char *)"GT-6FE");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8FE][0]), (char *)"GT-8FE");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_16FE][0]), (char *)"GT-16FE");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8FXS_A][0]), (char *)"GT-8FXS-A");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8POTS_B][0]), (char *)"GT-8POTS-B");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8FXS_B][0]), (char *)"GT-8FXS-B");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8POTSO_A][0]), (char *)"GT-8POTSO_A");	/* added by xieshl 20080815 */
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_GT_8POTSO_B][0]), (char *)"GT-8POTSO_B");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_4E1_75OHM][0]), (char *)"GT-4E1-120");
	VOS_StrCpy(&(OnuBoardTypeString[SUB_BOARD_4E1_120OHM][0]), (char *)"GT-4E1-75");

};

/* 使用默认值初始化ONU 设备UNI 端口数*/
static void InitOnuUniPortByDefault(void)
{
	VOS_MemZero(&OnuUniPort[0], sizeof(OnuUniPort));
	
	InitOnuUniPortBySysfile(V2R1_ONU_GT811, 1, 4, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831, 1, 4, 0, 0, 2, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831_CATV, 1, 4, 0, 0, 2, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT812, 1, 8, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT813, 1,24, 0, 0, 0, 0);
	/*InitOnuUniPortBySysfile(V2R1_ONU_GT881, 1, 8, 0, 0, 0, 0);*/
	InitOnuUniPortBySysfile(V2R1_ONU_GT861, 5, 8, 0, 0, 0, 0);
	/*InitOnuUniPortBySysfile(V2R1_ONU_GT891, 1, 8, 0, 0, 0, 0);*/
	InitOnuUniPortBySysfile(V2R1_ONU_GT810, 1, 1, 0, 0, 0, 0);
	/*InitOnuUniPortBySysfile(V2R1_ONU_GT863, 1, 1, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_CTC, 1, 1, 0, 0, 0, 0);*/
	InitOnuUniPortBySysfile(V2R1_ONU_GT865, 1, 16, 0, 16, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT816, 1, 1, 1, 0, 0, 0);

	InitOnuUniPortBySysfile(V2R1_ONU_GT811_A, 1, 4, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT812_A, 1, 8, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831_A, 1, 4, 0, 0, 2, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831_A_CATV, 1, 4, 0, 0, 2, 0);

	InitOnuUniPortBySysfile(V2R1_ONU_GT815, 1, 16, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT812_B, 1, 8, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831_B, 1, 4, 0, 0, 2, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT866, 1, 16, 0, 16, 0, 0);

	InitOnuUniPortBySysfile(V2R1_ONU_GT811_B, 1, 4, 0, 0, 0, 0);
	/*InitOnuUniPortBySysfile(V2R1_ONU_GT851, 1, 4, 0, 0, 0, 0);*/
	InitOnuUniPortBySysfile(V2R1_ONU_GT813_B, 1, 24, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT862, 1, 8, 0, 8, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT892, 1, 2, 0, 0, 0, 4);

	InitOnuUniPortBySysfile(V2R1_ONU_GT835, 1, 4, 0, 2, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT831_B_CATV, 1, 4, 0, 2, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT815_B, 1, 16, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT871, 1, 4, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT873, 1, 4, 0, 0, 0, 0);/*add by shixh20100122*/
	InitOnuUniPortBySysfile(V2R1_ONU_GT871_R, 1, 4, 0, 0, 0, 0);
	InitOnuUniPortBySysfile(V2R1_ONU_GT872, 1, 4, 0, 0, 0, 0);
	
	return;	
}

static void  ClearOnuDeviceDescArray(void)
{
	int i;
	for(i=V2R1_ONU_GT811; i< V2R1_ONU_MAX; i++)
		{
		VOS_StrCpy((char *)&(DeviceDesc_1[i][0]),		DEVICE_TYPE_DESC_UNKNOWN_STR);
		}
	return;
}

static void  InitOnuDeviceDescByDefault(void)
{
	ClearOnuDeviceDescArray();

	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT811][0]), 	DEVICE_TYPE_DESC_GT811_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831][0]), 	DEVICE_TYPE_DESC_GT831_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831_CATV][0]), 	DEVICE_TYPE_DESC_GT831_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT812][0]), 	DEVICE_TYPE_DESC_GT812_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT813][0]),	 	DEVICE_TYPE_DESC_GT813_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT881][0]), 	DEVICE_TYPE_DESC_GT881_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT861][0]), 	DEVICE_TYPE_DESC_GT861_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT891][0]), 	DEVICE_TYPE_DESC_GT891_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT810][0]), 	DEVICE_TYPE_DESC_GT810_STR_default);
	VOS_StrCpy( (char *)&(DeviceDesc_1[V2R1_ONU_GT863][0]), 	DEVICE_TYPE_DESC_GT863_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_CTC][0]), 		DEVICE_TYPE_DESC_GTCTC_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT865][0]), 	DEVICE_TYPE_DESC_GT865_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT816][0]), 	DEVICE_TYPE_DESC_GT816_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT811_A][0]), 	DEVICE_TYPE_DESC_GT811A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT812_A][0]), 	DEVICE_TYPE_DESC_GT812A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831_A][0]), 	DEVICE_TYPE_DESC_GT831A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831_A_CATV][0]), 	DEVICE_TYPE_DESC_GT831A_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT815][0]), 	DEVICE_TYPE_DESC_GT815_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT812_B][0]), 	DEVICE_TYPE_DESC_GT812B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831_B][0]), 	DEVICE_TYPE_DESC_GT831B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT866][0]), 	DEVICE_TYPE_DESC_GT866_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT811_B][0]), 	DEVICE_TYPE_DESC_GT811B_STR_default);
	/*VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT851][0]), 	DEVICE_TYPE_DESC_GT851_STR_default);*/
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT813_B][0]), 	DEVICE_TYPE_DESC_GT813B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT862][0]), 	DEVICE_TYPE_DESC_GT862_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT892][0]), 	DEVICE_TYPE_DESC_GT892_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT835][0]), 	DEVICE_TYPE_DESC_GT835_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT831_B_CATV][0]), 	DEVICE_TYPE_DESC_GT831B_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT815_B][0]), 			DEVICE_TYPE_DESC_GT815B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT871][0]), 			DEVICE_TYPE_DESC_GT871_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT871_R][0]), 			DEVICE_TYPE_DESC_GT871_STR_default);/*add by shixh20100122*/
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT872][0]), 			DEVICE_TYPE_DESC_GT871_STR_default);
	VOS_StrCpy(	(char *)&(DeviceDesc_1[V2R1_ONU_GT873][0]), 			DEVICE_TYPE_DESC_GT871_STR_default);
	return;
};

static void  ClearOnuDeviceTypeArray(void)
{
	int i;
	
	for(i=V2R1_ONU_GT811; i< V2R1_ONU_MAX; i++)
		{
		VOS_StrCpy((char *)&(DeviceType_1[i][0]),	DEVICE_TYPE_NAME_UNKNOWN_STR);
		}

	return;
}

static void  InitOnuDeviceTypeByDefault(void)
{
	ClearOnuDeviceTypeArray();
	
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT811][0]), 	DEVICE_TYPE_NAME_GT811_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831][0]), 	DEVICE_TYPE_NAME_GT831_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831_CATV][0]), 	DEVICE_TYPE_NAME_GT831_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT812][0]), 	DEVICE_TYPE_NAME_GT812_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT813][0]), 	DEVICE_TYPE_NAME_GT813_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT881][0]), 	DEVICE_TYPE_NAME_GT881_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT861][0]), 	DEVICE_TYPE_NAME_GT861_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT891][0]), 	DEVICE_TYPE_NAME_GT891_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT810][0]), 	DEVICE_TYPE_NAME_GT810_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT863][0]), 	DEVICE_TYPE_NAME_GT863_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_CTC][0]), 		DEVICE_TYPE_NAME_GTCTC_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT865][0]), 	DEVICE_TYPE_NAME_GT865_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT816][0]), 	DEVICE_TYPE_NAME_GT816_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT811_A][0]), 	DEVICE_TYPE_NAME_GT811A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT812_A][0]), 	DEVICE_TYPE_NAME_GT812A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831_A][0]), 	DEVICE_TYPE_NAME_GT831A_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831_A_CATV][0]), 		DEVICE_TYPE_NAME_GT831A_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT815][0]), 		DEVICE_TYPE_NAME_GT815_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT812_B][0]), 	DEVICE_TYPE_NAME_GT812B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831_B][0]), 	DEVICE_TYPE_NAME_GT831B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT866][0]), 		DEVICE_TYPE_NAME_GT866_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT811_B][0]), 	DEVICE_TYPE_NAME_GT811B_STR_default);
	/*VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT851][0]), 		DEVICE_TYPE_NAME_GT851_STR_default);*/
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT813_B][0]), 	DEVICE_TYPE_NAME_GT813B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT862][0]), 		DEVICE_TYPE_NAME_GT862_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT892][0]), 		DEVICE_TYPE_NAME_GT892_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT835][0]), 		DEVICE_TYPE_NAME_GT835_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT831_B_CATV][0]), 	DEVICE_TYPE_NAME_GT831B_CATV_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT815_B][0]), 		DEVICE_TYPE_NAME_GT815B_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT871][0]), 		DEVICE_TYPE_NAME_GT871_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT871_R][0]), 		DEVICE_TYPE_NAME_GT871R_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT872][0]), 		DEVICE_TYPE_NAME_GT872_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT871_R][0]), 		DEVICE_TYPE_NAME_GT872R_STR_default);
	VOS_StrCpy(	(char *)&(DeviceType_1[V2R1_ONU_GT873][0]), 		DEVICE_TYPE_NAME_GT873_STR_default);/*add by shixh20100122*/
	return;
}


static void  ClearOnuAppTypeArray(void)
{
	int i;

	for(i=0; i< V2R1_ONU_MAX; i++)
		{
		VOS_StrCpy((char *)&(OnuAppType_1[i][0]),	DEVICE_APP_TYPE_UNKNOWN);
		}

	return;
}

static void InitOnuAppTypeByDefault(void)
{
	ClearOnuAppTypeArray();

	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT811][0]), 	DEVICE_APP_TYPE_GT811_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831][0]), 	DEVICE_APP_TYPE_GT831_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831_CATV][0]), 	DEVICE_APP_TYPE_GT831_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT812][0]), 	DEVICE_APP_TYPE_GT812_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT813][0]), 	DEVICE_APP_TYPE_GT813_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT881][0]), 	DEVICE_APP_TYPE_GT881_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT861][0]), 	DEVICE_APP_TYPE_GT861_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT891][0]), 	DEVICE_APP_TYPE_GT891_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT810][0]), 	DEVICE_APP_TYPE_GT816_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT863][0]), 	DEVICE_APP_TYPE_GT863_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_CTC][0]), 	DEVICE_APP_TYPE_GTCTC_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT865][0]), 	DEVICE_APP_TYPE_GT865_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT816][0]), 	DEVICE_APP_TYPE_GT816_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT811_A][0]), 	DEVICE_APP_TYPE_GT811A_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT812_A][0]), 	DEVICE_APP_TYPE_GT812A_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831_A][0]), 	DEVICE_APP_TYPE_GT831A_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831_A_CATV][0]), 		DEVICE_APP_TYPE_GT831A_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT815][0]), 		DEVICE_APP_TYPE_GT815_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT812_B][0]), 	DEVICE_APP_TYPE_GT812B_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831_B][0]), 	DEVICE_APP_TYPE_GT831B_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT866][0]), 		DEVICE_APP_TYPE_GT866_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT811_B][0]), 	DEVICE_APP_TYPE_GT811B_EPON_default);
	/*VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT851][0]), 		DEVICE_APP_TYPE_GT851_EPON_default);*/
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT813_B][0]), 	DEVICE_APP_TYPE_GT813B_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT862][0]), 		DEVICE_APP_TYPE_GT862_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT892][0]), 		DEVICE_APP_TYPE_GT892_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT835][0]), 		DEVICE_APP_TYPE_GT835_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT831_B_CATV][0]), 	DEVICE_APP_TYPE_GT831B_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT815_B][0]), 		DEVICE_APP_TYPE_GT815B_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT871][0]), 		DEVICE_APP_TYPE_GT871_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT873][0]), 		DEVICE_APP_TYPE_GT871_EPON_default);/*add by shixh20100122*/
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT871_R][0]), 		DEVICE_APP_TYPE_GT871_EPON_default);
	VOS_StrCpy(	(char *)&(OnuAppType_1[V2R1_ONU_GT872][0]), 		DEVICE_APP_TYPE_GT871_EPON_default);

	return;
}

static void ClearOnuVoiceTypeArray(void)
{
	int i;
	
	for(i=0; i< V2R1_ONU_MAX; i++)
		{
		VOS_StrCpy((char *)&(OnuVoiceType_1[i][0]),	DEVICE_APP_TYPE_UNKNOWN);
		}

	return;
}

static void InitOnuVoiceTypeByDefault(void)
{
	ClearOnuVoiceTypeArray();

	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831_CATV][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831_A][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831_A_CATV][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);
	/*
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831_B][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT831_B_CATV][0]), 	DEVICE_APP_TYPE_GT831_VOICE_default);*/
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT861][0]), 	DEVICE_APP_TYPE_GT861_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT866][0]), 		DEVICE_APP_TYPE_GT866_VOICE_default);
	VOS_StrCpy(	(char *)&(OnuVoiceType_1[V2R1_ONU_GT863][0]), 		DEVICE_APP_TYPE_GT863_VOICE_default);/*add by shixh20090806*/

	return;
}

static void ClearOnuFpgaTypeArray(void)
{
	int i;

	for(i=0; i< V2R1_ONU_MAX; i++)
		{
		VOS_StrCpy((char *)&(OnuFpgaType_1[i][0]),	DEVICE_APP_TYPE_UNKNOWN);
		}
	
	return;
}

static void InitOnuFpgaTypeByDefault(void)
{
	ClearOnuFpgaTypeArray();

	VOS_StrCpy(	(char *)&(OnuFpgaType_1[V2R1_ONU_GT861][0]), 	DEVICE_APP_TYPE_GT861_FPGA_default);
	VOS_StrCpy(	(char *)&(OnuFpgaType_1[V2R1_ONU_GT865][0]), 	DEVICE_APP_TYPE_GT865_FPGA_default);

	return;
}

/* 使用默认值初始化私有ONU 在CTC 协议中的注册码*/
static void InitOnuCtcRegisterIdByDefault()
{
	int OnuType;
	for(OnuType=0; OnuType< V2R1_ONU_MAX; OnuType++)
		OnuCtcRegisterId[OnuType] = 0;

	OnuCtcRegisterId[V2R1_ONU_GT813] = CTC_GT813_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT816] = CTC_GT816_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT810] = CTC_GT810_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT811_A] = CTC_GT811_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT812_A] = CTC_GT812_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT812_B] = CTC_GT812B_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT831_A] = CTC_GT831_A_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT831_A_CATV] = CTC_GT831_A_CATV_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT861] = CTC_GT861_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT815] = CTC_GT815_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT866] = CTC_GT866_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT831_B] = CTC_GT831B_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT811_B] = CTC_GT811B_MODEL;
	/*OnuCtcRegisterId[V2R1_ONU_GT851] = CTC_GT851_MODEL;*/
	OnuCtcRegisterId[V2R1_ONU_GT813_B] = CTC_GT813B_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT835] = CTC_GT835_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT862] = CTC_GT862_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT863] = CTC_GT863_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT892] = CTC_GT892_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT815_B] = CTC_GT815_B_MODEL;
	OnuCtcRegisterId[V2R1_ONU_GT831_B_CATV] = CTC_GT831_B_CATV_MODEL;
	return;
}

/* 使用默认值初始化ONU 管理信息*/
void  InitOnuMgmtInfoByDefault()
{
	InitOnuBoardStringInfoByDefault();
	InitOnuUniPortByDefault();
	InitOnuDeviceDescByDefault();
	InitOnuDeviceTypeByDefault();
	InitOnuAppTypeByDefault();
	InitOnuVoiceTypeByDefault();
	InitOnuFpgaTypeByDefault();
	InitOnuCtcRegisterIdByDefault();
	
	VOS_StrCpy(&OnuVendorInfoDefault[0], ONU_VENDOR_INFO_DEFAULT_default);
	VOS_StrCpy(&OnuAppKeyName[0], PRODUCT_ONU_EPONAPP_KEY_NAME_GW);
	return;
}

/*  清空ONU 管理表中的所有信息*/
void  OnuMgmtInfoClear()
{
	int i;
	int OnuType;
	
	for(i=0; i<SUB_BOARD_GT_LAST; i++)
		VOS_StrCpy(&(OnuBoardTypeString[i][0]), "unknown");

	VOS_MemZero(&OnuUniPort[0], sizeof(OnuUniPort));
	ClearOnuDeviceDescArray();
	ClearOnuDeviceTypeArray();
	ClearOnuAppTypeArray();
	ClearOnuVoiceTypeArray();
	ClearOnuFpgaTypeArray();
		
	for(OnuType=0; OnuType< V2R1_ONU_MAX; OnuType++)
		OnuCtcRegisterId[OnuType] = 0;

	VOS_MemZero(&OnuVendorInfoDefault[0], sizeof(OnuVendorInfoDefault));
	VOS_MemZero(&OnuAppKeyName[0], sizeof(OnuAppKeyName));
	
}

/****  以下API 用于使用系统文件初始化ONU 管理信息*/

#ifdef  ONU_MGMT_INFO_BY_SYSFILE_
#endif

/* ONU vendor 信息，对应关键字[ONU_VENDOR_INFO]*/
void  InitOnuVendorInfoBySysfile(char *VendorInfo)
{
	int length;

	if(VendorInfo == NULL)
		return;

	length = VOS_StrLen(VendorInfo);
	if(length > MAXVENDORINFOLEN) 
		length = MAXVENDORINFOLEN;
	
	VOS_StrnCpy(&OnuVendorInfoDefault[0],VendorInfo,length);
	OnuVendorInfoDefault[length] = '\0';

	return;
}

/* ONU 侧板卡名称，对应关键字[ONU_BOARD] */
void InitOnuBoardTypeStringBySysfile(int  BoardType, char *TypeString)
{
	int length;

	if((BoardType < SUB_BOARD_GT_EPON_B ) || (BoardType >= SUB_BOARD_GT_LAST )) 
		return;
	if(TypeString == NULL )
		return;

	length = VOS_StrLen(TypeString);
	if(length > BOARDTYPESTRINGLEN ) length = BOARDTYPESTRINGLEN;

	VOS_StrnCpy( &(OnuBoardTypeString[BoardType][0]), TypeString, length);
	OnuBoardTypeString[BoardType][length] = '\0';
	
	return;
}

/* 使用系统文件初始化ONU设备信息，对应关键字: [TYPE_INTEGER],[TYPE_STRING],[DESC_STRING]*/
void InitOnuTypeInfoBySysfile(int OnuType, char *TypeString, char *DescString)
{
	int length;
	
	if(OnuType >=V2R1_ONU_MAX)
		return;

	if(TypeString != NULL )
		{
		length = VOS_StrLen(TypeString);
		if(length > ONU_TYPE_LEN)
			length = ONU_TYPE_LEN;
		VOS_StrnCpy( &(DeviceType_1[OnuType][0]), TypeString, length);
		DeviceType_1[OnuType][length]='\0';
		}
	if(DescString != NULL)
		{
		length = VOS_StrLen(DescString);
		if(length > MAXDEVICEDESCLEN)
			length = MAXDEVICEDESCLEN;
		VOS_StrnCpy( &(DeviceDesc_1[OnuType][0]), DescString, length);
		DeviceDesc_1[OnuType][length]='\0';
		}
	
	return;
}
/* 使用系统文件初始化ONU设备UNI 端口数;对应关键字:[TYPE_INTEGER],[MAX_SLOT],[FE_PORT_NUM],[GE_PORT_NUM],[POTS_PORT_NUM],[VOIP_PORT_NUM],[E1_PORT_NUM]*/
void InitOnuUniPortBySysfile(int OnuType, int SlotNum, int FePortNum, int GePortNum, int PotsPortNum, int VoipPortNum, int E1PortNum)
{
	if(OnuType >=V2R1_ONU_MAX)
		return;

	OnuUniPort[OnuType].SlotNum =  SlotNum;
	OnuUniPort[OnuType].FePortNum =  FePortNum;
	OnuUniPort[OnuType].GePortNum = GePortNum;
	OnuUniPort[OnuType].PotsPortNum = PotsPortNum;
	OnuUniPort[OnuType].VoipPortNum = VoipPortNum;
	OnuUniPort[OnuType].E1PortNum = E1PortNum;

	return;
}

/* 使用系统文件初始化ONU设备信息，对应关键字：TYPE_INTEGER，APP_EPON_ID，APP_VOICE_ID，APP_FPGA_ID */
void InitOnuAppStringBySysfile(int OnuType, char *eponAppString, char *voiceAppString, char *fpgaAppString)
{
	int length;
	
	if(OnuType >=V2R1_ONU_MAX)
		return;

	if(eponAppString != NULL)
	{
		length = VOS_StrLen(eponAppString);
		if(length > ONU_TYPE_LEN) length = ONU_TYPE_LEN;
		if(VOS_StrCmp(eponAppString, DEVICE_APP_TYPE_UNKNOWN_ID) != 0)
		{
			VOS_StrnCpy( &(OnuAppType_1[OnuType][0]), eponAppString, length );
			OnuAppType_1[OnuType][length] = '\0';
		}
		else
		{
			VOS_StrCpy( &(OnuAppType_1[OnuType][0]), DEVICE_APP_TYPE_UNKNOWN );
		}
	}
	if(voiceAppString != NULL)
	{
		length = VOS_StrLen(voiceAppString);
		if(length > ONU_TYPE_LEN) length = ONU_TYPE_LEN;
		if(VOS_StrCmp(voiceAppString, DEVICE_APP_TYPE_UNKNOWN_ID) != 0)
		{
			VOS_StrnCpy( &(OnuVoiceType_1[OnuType][0]), voiceAppString, length );
			OnuVoiceType_1[OnuType][length] = '\0';
		}
		else
		{
			VOS_StrCpy( &(OnuVoiceType_1[OnuType][0]), DEVICE_APP_TYPE_UNKNOWN );
		}
	}
	if(fpgaAppString != NULL)
	{
		length = VOS_StrLen(fpgaAppString);
		if(length > ONU_TYPE_LEN) length = ONU_TYPE_LEN;
		if(VOS_StrCmp(fpgaAppString, DEVICE_APP_TYPE_UNKNOWN_ID) != 0)
		{
			VOS_StrnCpy( &(OnuFpgaType_1[OnuType][0]), fpgaAppString, length );
			OnuFpgaType_1[OnuType][length] = '\0';
		}
		else
		{
			VOS_StrCpy( &(OnuFpgaType_1[OnuType][0]), DEVICE_APP_TYPE_UNKNOWN);
		}
	}

	return;
}

/* 使用系统文件初始化私有ONU 在CTC 协议中的注册码*/
void InitOnuCtcRegisterIdBySysfile(int OnuType, int RegisterId)
{
	if(OnuType >=V2R1_ONU_MAX)
		return;
	
	OnuCtcRegisterId[OnuType] = RegisterId;
	return;
}

/* 使用系统文件初始化私有ONU 程序名称前缀*/
void InitOnuAppPrefixBySysfile(char *AppFrefix, int len)
{
	if(len > ONU_TYPE_LEN)
		len = ONU_TYPE_LEN;
	
	VOS_MemCpy(&OnuAppKeyName[0], AppFrefix, len);
	OnuAppKeyName[len] = '\0';
	return;
}


#ifdef ONU_MGMT_INFO_API__
#endif

/* ONU 侧板卡类型关键字与板卡类型转换*/
int  OnuBoardTypeKeywordToInteger(char *TypeKeyword)
{
	if(TypeKeyword == NULL)
		return( SUB_BOARD_NULL);

	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_EPON", VOS_StrLen("ONU_BOARD_EPON")) == 0)
		return SUB_BOARD_GT_EPON_B;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8POTS_A", VOS_StrLen("ONU_BOARD_8POTS_A")) == 0)
		return SUB_BOARD_GT_8POTS_A;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_6FE", VOS_StrLen("ONU_BOARD_6FE")) == 0)
		return SUB_BOARD_GT_6FE;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8FE", VOS_StrLen("ONU_BOARD_8FE")) == 0)
		return SUB_BOARD_GT_8FE;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_16FE", VOS_StrLen("ONU_BOARD_16FE")) == 0)
		return SUB_BOARD_GT_16FE;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8FXS_A", VOS_StrLen("ONU_BOARD_8FXS_A")) == 0)
		return SUB_BOARD_GT_8FXS_A;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8POTS_B", VOS_StrLen("ONU_BOARD_8POTS_B")) == 0)
		return SUB_BOARD_GT_8POTS_B;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8FXS_B", VOS_StrLen("ONU_BOARD_8FXS_B")) == 0)
		return SUB_BOARD_GT_8FXS_B;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8POTSO_A", VOS_StrLen("ONU_BOARD_8POTSO_A")) == 0)
		return SUB_BOARD_GT_8POTSO_A;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_8POTSO_B", VOS_StrLen("ONU_BOARD_8POTSO_B")) == 0)
		return SUB_BOARD_GT_8POTSO_B;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_4E1_120OHM", VOS_StrLen("ONU_BOARD_4E1_120OHM")) == 0)
		return SUB_BOARD_4E1_120OHM;
	if(VOS_StrnCmp(TypeKeyword, "ONU_BOARD_4E1_75OHM", VOS_StrLen("ONU_BOARD_4E1_75OHM")) == 0)
		return SUB_BOARD_4E1_75OHM;

	return SUB_BOARD_NULL;
}

/* 取ONU fe 端口数*/
int  GetOnuFePortNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].FePortNum;
	return 0;
}

/* 取ONU ge 端口数*/
int  GetOnuGePortNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].GePortNum;
	return 0;
}

/* 取onu  子槽位数*/
int  GetOnuSubSlotNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].SlotNum;
	return 1;
}

/* 取ONU pots 端口数*/
int  GetOnuPotsPortNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].PotsPortNum;
	return 0;
}

/* 取ONU voip 端口数*/
int  GetOnuVoipPortNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].PotsPortNum;
	return 0;
}

/* 取ONU E1 端口数*/
int  GetOnuE1PortNum(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuUniPort[OnuType].E1PortNum;
	return 0;
}

/*  取ONU ctc 协议注册ID */
int GetOnuCtcRegisterId(short int OnuType)
{
	if((OnuType >= V2R1_ONU_GT811) && (OnuType < V2R1_ONU_MAX))
		return OnuCtcRegisterId[OnuType];
	return 0;
}

/* 比对私有ONU 是否以自定义的CTC 注册码来注册
    返回值: 
	   RERROR -- 否
	   其它值 -- 以自定义的CTC 注册码来注册
*/
int CompareCtcRegisterId(short int PonPortIdx, short int OnuIdx, int RegisterId)
{
	int OnuType;

    CHECK_ONU_RANGE


    if((( RegisterId & 0xffff0000) >> 16 ) != CTC_REGISTER_ID_PRE)
        return(RERROR);

    for (OnuType = V2R1_ONU_GT811; OnuType < V2R1_ONU_MAX; OnuType++)
    {
        if (OnuCtcRegisterId[OnuType] == RegisterId)
            break;
    }

    /*commented by wangxiaoyu 2011-09-09
     * 当register id以GT开头时，如果类型表中没有找到匹配的，我们认为是ONU送错了，但它仍然是私有ONU的一种，需要在后续
     * 处理中通过私有OAM的方式获取设备类型
    if (OnuType >= V2R1_ONU_MAX)
        return (RERROR);
        */

    return (OnuType);
}

/* 用于搜索与字符串匹配的第一种ONU 类型*/
int  SearchOnuType( char *TypeString)
{
	int  OnuType;

	for(OnuType = V2R1_ONU_GT811; OnuType < V2R1_ONU_MAX; OnuType++)
		{
		if(VOS_StrnCmp(GetDeviceTypeString(OnuType),DEVICE_TYPE_NAME_UNKNOWN_STR, VOS_StrLen(DEVICE_TYPE_NAME_UNKNOWN_STR)) ==0)
			continue;

		if(VOS_StrniCmp(TypeString, GetDeviceTypeString(OnuType), VOS_StrLen(TypeString)) == 0)
			return OnuType;
		}
	
	if(OnuType == V2R1_ONU_MAX && searchCtcOnuTypeString(TypeString) == VOS_OK)
	    return V2R1_ONU_CTC;
	if(OnuType == V2R1_ONU_MAX && searchGponOnuTypeString(TypeString) == VOS_OK)
	    return V2R1_ONU_GPON;
	return RERROR;	
}

int  SearchOnuTypeFullMatch( char *TypeString)
{
	int  OnuType;

	for(OnuType = V2R1_ONU_GT811; OnuType < V2R1_ONU_MAX; OnuType++)
		{
		if(VOS_StrCmp(GetDeviceTypeString(OnuType),DEVICE_TYPE_NAME_UNKNOWN_STR) ==0)
			continue;

		if(VOS_StrCmp(TypeString, GetDeviceTypeString(OnuType)) == 0)
			return OnuType;
		}
	
	return RERROR;	
}

/* 用于ONU 类型字符串信息提示*/
void NotificationOnuTypeString(struct vty *vty)
{
	int OnuType, count=0;
	
	vty_out(vty,"type error; onu type should be");
	for(OnuType=V2R1_ONU_GT811; OnuType< V2R1_ONU_MAX;OnuType++)
		{
		if(VOS_StrnCmp(GetDeviceTypeString(OnuType),DEVICE_TYPE_NAME_UNKNOWN_STR, VOS_StrLen(DEVICE_TYPE_NAME_UNKNOWN_STR)) ==0)
			continue;
		else{
			if(count == 0)
				vty_out(vty," %s", GetDeviceTypeString(OnuType));
			else
				vty_out(vty,", %s", GetDeviceTypeString(OnuType));
			count++;
			}
		if(count >= 3 )
			break;
		}
	
	vty_out(vty," etc.\r\n");	
	return;
}


#ifdef INIT_MGMT_INFO_BY_SYSFILE_
#endif

#ifdef  _OEM_TYPE_CLI_
extern void init_information(void);
extern void init_onu_cli_information(void);
extern void init_sync_information(void);
extern void  init_onu_daya_information(void);
extern void  pon_init_information(void);
#endif

extern int mn_set_hostname(char *new_hostname, char **errmsg,unsigned short lan_type);
extern long device_sysfile_read_flash_to_mem_deflate( CHAR* memfile, LONG * memfile_length );
char SysfileBuffer[DEVICE_FLASH_SYNINFO_MAXLEN+10];
int GetOltSysfileVer(char *ver)
{
    app_desc_t *sysfile_desc = (app_desc_t *)SysfileBuffer;
    if(ver)
        VOS_StrCpy(ver, sysfile_desc->file_ver);
    return VOS_OK;
}
void   InitMgmtInfoBySysfile( struct vty *vty)
{
	
	int length = 0;
	int Product_type = GetOltType();
	char * errmsg = NULL;
	int new_host = 0;
	int new_name = 0, new_desc = 0;

	OnuMgmtInfoClear();
	OltMgmtInfoClear();
    
    /*added by luh 2013-1-28, sysfile升级问题*/
	VOS_MemZero(SysfileBuffer, DEVICE_FLASH_SYNINFO_MAXLEN+10);		
	device_sysfile_read_flash_to_mem_deflate(SysfileBuffer, (long *)&length);
	if( length > 0 )
	{
		ParseSysfile(SysfileBuffer + sizeof(app_desc_t), (length - sizeof(app_desc_t)));

		/* modified by xieshl 20111026, 执行active sysfile命令不能影响hostname配置，问题单13727 */
		/*if(Product_type == V2R1_OLT_GFA6700)
		{
			VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(V2R1_OLT_GFA6700));
			VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(V2R1_OLT_GFA6700));
		}
		else if(Product_type == V2R1_OLT_GFA6100)
		{
			VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(V2R1_OLT_GFA6100));
			VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(V2R1_OLT_GFA6100));
		}
		else if(Product_type == V2R1_OLT_GFA6900)
		{
			VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(V2R1_OLT_GFA6900));
			VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(V2R1_OLT_GFA6900));
		}*/

		new_host = VOS_StrCmp( mn_get_hostname(), &OltDeviceNameDefault[0] );
		/* modified by xieshl 20111107, 执行active sysfile命令不能影响device name配置，问题单13741 */
		new_name = VOS_StrCmp( GetOltDeviceNameString(), &OltDeviceNameDefault[0] );
		new_desc = VOS_StrCmp( GetOltDeviceDescString(), &OltDeviceDescriptionDefault[0] );
		
		VOS_StrCpy(&OltDeviceDescriptionDefault[0], GetDeviceDescString(Product_type));
		VOS_StrCpy(&OltDeviceNameDefault[0], GetDeviceTypeString(Product_type));
		if( new_name == 0 )
			SetOltDeviceName(&OltDeviceNameDefault[0], VOS_StrLen(&OltDeviceNameDefault[0]));
		if( new_desc == 0 )
			SetOltDeviceDesc(&OltDeviceDescriptionDefault[0], VOS_StrLen(&OltDeviceDescriptionDefault[0]));

		if( new_host == 0 )
		{
			/*  使用设备名字做为默认CLI 提示符；cli提示符长度有要求，必须大于3*/
			if(VOS_StrLen(typesdb_product_mn_default_hostname()) > 3 )
			{
				if(!mn_set_hostname( typesdb_product_mn_default_hostname(), &errmsg, 1))
				{
					v2r1_printf( vty, "  %% %s%s", errmsg, VTY_NEWLINE );
				}
			}
			else
			{
				char  prompt[32];
				char *temp = &prompt[0];
				int hostlen = VOS_StrLen(typesdb_product_mn_default_hostname());
				
				v2r1_printf( vty, "\r\n %%  hostname length %d is not enough, cli prompt is modified\r\n", hostlen );
				
				VOS_StrCpy(temp,  typesdb_product_mn_default_hostname());
				
				while(hostlen <=3)
				{
					if(hostlen == 0 )
					{
						prompt[hostlen++] = 'C';
						prompt[hostlen++] = 'l';
						prompt[hostlen] = 'i';
					}
					else 
						prompt[hostlen] = '-';
					hostlen++;
				}
				prompt[hostlen] = '\0';
				
				if(!mn_set_hostname( prompt, &errmsg, 1))
				{
					v2r1_printf( vty, "  %% %s%s", errmsg, VTY_NEWLINE );
				}
			}
		}
#ifdef  _OEM_TYPE_CLI_
		init_information();
		init_onu_cli_information();
		init_onu_daya_information();
		init_sync_information();
		pon_init_information();
#endif
	}

	return;
}

LONG Devsm_Sysfile_Local_Active( void )
{
    if(xflash_sysfile_exist() != VOS_OK)
    {
        sys_console_printf("\r\nthere is no sysfile in flash\r\n"); 
        return VOS_ERROR;
    }

    InitMgmtInfoBySysfile(NULL);
    InitProductTypeArray();

    return VOS_OK;
}

#ifdef __cplusplus
}
#endif
