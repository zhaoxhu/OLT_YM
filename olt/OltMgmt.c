/***************************************************************
*
*						Module Name:  OltMgmt.c
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
*   Date: 			2006/04/18
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/04/18  |   chenfj          |     create 
**----------|-----------|------------------
**
** 1  add by chenfj 2006/09/19 
**  #2604 问题telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了
** 2   modified by chenfj 2008-7-9
**         增加GFA6100 产品支持
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"
#include  "ponEventHandler.h"

#include "..\..\superset\platform\vos\vospubh\Vos_syslog.h"
#include "..\event\eventMain.h"

long V2r1TimerId =0;
LONG onuAgingTimerId = 0;
LONG onuupdateTimerId = 0;

ULONG g_onu_updatetime = 0;	/* 0:never be aged, range 10 *60 - 365 * 24 * 3600 seconds, default 2 days */
const ULONG g_onu_updatetime_default = (15 * 24 * 3600);

unsigned char  *Fec_Ability[] = 
	{
	(unsigned char *)"unknown",
	(unsigned char *)"unknown",
	(unsigned char *)"supported",
	(unsigned char *)"Unsupported"
	};

/*typedef struct {
	ushort_t year;
	uchar_t  month;
	uchar_t  day;
	uchar_t  hour;
	uchar_t  minute;
	uchar_t  second;
	uchar_t  reserver[4];
}__attribute__((packed)) sysDateAndTime_t;*/

unsigned int V2R1_SYS_TIME_PERIOD = (30*60);

OLTMgmtInfo_S  OLTMgmt/*= {0} */;	/* 20100201 */
unsigned long  OltMgmtDataSemId= 0;

int V2R1_watchdog_init_flag = V2R1_DISABLE;
int V2R1_watchdog_startup_flag = V2R1_DISABLE;	/* added by xieshl 20090212 */
int SWAP_PASSIVE_ID_DEBUG = V2R1_DISABLE;

AlarmInfo_S AlarmInfoTable[ALLALARMNUM];
#if 0
PONChipInfo_S  PonChipMgmtTable[SYS_MAX_PON_PORTNUM];
#else
PONChipInfo_S  *PonChipMgmtTable;
#endif
unsigned long  PonChipMgmtDataSemId = 0;

unsigned char  OltMacAddrDefault[6]={ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };

unsigned char  OltOperStatusDefault = V2R1_UP;
unsigned char  OltAdminStatusDefault = V2R1_ENABLE;
unsigned char  OltPWUtemperatureThreshold_gen_Default = DEFAULT_GEN_PWU;  
unsigned char  OltPWUtemperatureThreshold_dis_Default = DEFAULT_DIS_PWU;   
unsigned char  OltTemperatureThreshold_gen_Default = DEFAULT_GEN;  
unsigned char  OltTemperatureThreshold_dis_Default = DEFAULT_DIS;  
unsigned long  OltAlarmMaskDefault = 0;
unsigned char  OltMaxPonPortDefault = SYS_MAX_PON_PORTNUM;

extern int pon_swap_switch_enable;
extern int PAS_send_olt_msg[];
extern int PAS_rev_olt_msg[];
/*extern  SYS_MODULE_TYPE(slotno);*/
extern unsigned char /*mac_address_t */ HOST_MAC_ADDRESS_VXWORKS[];
extern  STATUS getOnuAuthEnable(ULONG slot, ULONG port, unsigned long *enable);

 /* 私有MIB定义trap事件，非法ONU事件上报 */
extern int onuRegAuthFailure_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, 
		uchar_t *pOnuMacAddr );
 
 /*extern int PonPortVersion_EventReport( ulong_t slot, ulong_t port,  ulong_t AlarmId);*/
extern int OnuEvent_Active_UplinkRunningBandWidth(short int PonPortIdx, short int OnuIdx);
extern int OnuEvent_Active_DownlinkRunningBandWidth(short int PonPortIdx, short int OnuIdx);
extern int ponSFPTypeMismatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponSFPTypeMatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponFWVersionMismatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponFWVersionMatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponDBAVersionMismatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponDBAVersionMatch_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx );
extern int ponPortLosAlarm_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx);
extern int ponPortLosAlarmClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx);

extern int GetPonPortSfpState(short int PonPortIdx);

/*extern LONG onuExtMgmt_OnuRegCallback( ULONG devIdx );
extern LONG onuExtMgmt_OnuDeregCallback( ULONG devIdx );*/

/**************************************************************
 *
 *    Function:  GetSysCurrentTime( Date_S *CurrentTime )
 *
 *    Param: Date_S *CurrentTime
 *
 *    Desc:  get current date from real timer 
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  GetSysCurrentTime( Date_S *CurrentTime)
{
	sysDateAndTime_t CurTime;
	
	if( CurrentTime == NULL ) return( RERROR );

	/*
	if( VOS_OK  == VOS_GetCurrentTime( ( unsigned long *)&(CurrentTime->year), (unsigned long *) &(CurrentTime->hour), (unsigned long *)&(CurrentTime->MilliSecond[0] )) )
		return ( ROK );
	*/
	if( eventGetCurTime( &CurTime )  == VOS_OK)
		{
		CurrentTime->year = CurTime.year;
		if( CurrentTime->year <= 99 )
			CurrentTime->year += 2000;
		CurrentTime->month = CurTime.month;
		CurrentTime->day = CurTime.day;
		CurrentTime->hour = CurTime.hour;
		CurrentTime->minute = CurTime.minute;
		CurrentTime->second = CurTime.second;
		CurrentTime->MilliSecond = *(int*)&(CurTime.reserver[0] );
		return( ROK );
		}
	return ( RERROR );
}

/**************************************************************
 *
 *    Function:  ClearOltMgmgInfo()
 *
 *    Param: none
 *
 *    Desc:  clear all the Olt management data 
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  ClearOltMgmtInfo()
{
	unsigned long alarmMask = OLTMgmt.AlarmMask;

	VOS_MemSet( &OLTMgmt, 0, sizeof( OLTMgmtInfo_S ));
	OLTMgmt.AlarmMask = alarmMask;
#if 0
	OLTMgmt.adminStatus = V2R1_DISABLE;
	OLTMgmt.operStatus = V2R1_DOWN;
	OLTMgmt.PWUtemperatureThreshold_dis = 0;
	OLTMgmt.PWUtemperatureThreshold_gen = 0;
	OLTMgmt.temperatureThreshold_dis = 0;
	OLTMgmt.temperatureThreshold_gen = 0;
	OLTMgmt.InsertedPonCard = 0;
	OLTMgmt.AlarmStatus = 0;
	VOS_MemSet( &OLTMgmt.DeviceInfo, 0, sizeof( DeviceInfo_S ) );
	VOS_MemSet( & OLTMgmt.environment, 0, sizeof( OltEnvironment_S ));
#endif	
	return (ROK ); 
}

#if 0
/**************************************************************
 *
 *    Function:  OltMgmgInfoFromFlash()
 *
 *    Param: none
 *
 *    Desc:  Initialize the Olt management data from flash
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  OltMgmgInfoFromFlash()
{
	ClearOltMgmtInfo();

	/* read Data from Flash */
	
	OLTMgmt.adminStatus = V2R1_ENABLE;
	OLTMgmt.operStatus = V2R1_ENABLE;
	/* read Info from flash */
	OLTMgmt.PWUtemperatureThreshold_dis = 0;
	OLTMgmt.PWUtemperatureThreshold_gen = 0;
	OLTMgmt.temperatureThreshold_dis = 0;
	OLTMgmt.temperatureThreshold_gen = 0;
	/*OLTMgmt.InsertedPonCard = 0;
	OLTMgmt.AlarmStatus = 0;*/
	VOS_MemSet( &OLTMgmt.DeviceInfo, 0, sizeof( DeviceInfo_S ) );
	/*VOS_MemSet( & OLTMgmt.environment, 0, sizeof( OltEnvironment_S ));*/
	
	return ( ROK );
}
#endif

/**************************************************************
 *
 *    Function:  OltMgmgInfoDefault()
 *
 *    Param: none
 *
 *    Desc:  Initialize the Olt management data using Default value
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
extern ULONG device_producttype_get( VOID );
extern void InitPonCardMgmtStatus(void);
int  InitProductTypeVar()
{
	/* 默认数据 初始化*/
	InitPonCardMgmtStatus();
    
	VOS_MemZero( AlarmInfoTable, sizeof(AlarmInfoTable) );
    
	ClearOltMgmtInfo();
	if( ( SYS_PRODUCT_TYPE ==  PRODUCT_E_EPON3 ) )
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA6700;
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA6100;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA6900;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900M)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA6900M;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900S)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA6900S;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA8000)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA8000;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA8000M)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA8000M;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA8000S)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA8000S;
	else if(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA8100)
		OLTMgmt.DeviceInfo.type = V2R1_OLT_GFA8100;
	else
		OLTMgmt.DeviceInfo.type = V2R1_DEVICE_UNKNOWN;

	InitGlobalVariable();
	return(ROK);
}

int  OltMgmtInfoDefault()
{
	/* 默认数据 初始化*/
#if 0
	OLTMgmt.OltConfigInfoDefault.adminStatusDefault = OltAdminStatusDefault;
	OLTMgmt.OltConfigInfoDefault.operStatusDefault = OltOperStatusDefault; 
#endif
	OLTMgmt.OltConfigInfoDefault.PWUtemperatureThreshold_dis_Default= OltPWUtemperatureThreshold_dis_Default;
	OLTMgmt.OltConfigInfoDefault.PWUtemperatureThreshold_gen_Default = OltPWUtemperatureThreshold_gen_Default;
	OLTMgmt.OltConfigInfoDefault.temperatureThreshold_dis_Default = OltTemperatureThreshold_dis_Default;
	OLTMgmt.OltConfigInfoDefault.temperatureThreshold_gen_Default = OltTemperatureThreshold_gen_Default;

	OLTMgmt.OltConfigInfoDefault.AlarmMaskDefault = OltAlarmMaskDefault;
	OLTMgmt.OltConfigInfoDefault.MaxPonPortDefault = MAXPON;
#if 0
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.VendorInfoDefault, OltVendorInfoDefault, VOS_StrLen( OltVendorInfoDefault ));
	OLTMgmt.OltConfigInfoDefault.VendorInfoLen = VOS_StrLen( OltVendorInfoDefault );
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.VendorLocationDefault, OltVendorLocationDefault, VOS_StrLen( OltVendorLocationDefault ) );
	OLTMgmt.OltConfigInfoDefault.VendorLocationLen = VOS_StrLen( OltVendorLocationDefault );
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.VendorContactDefault, OltVendorContactDefault, VOS_StrLen( OltVendorContactDefault ) );
	OLTMgmt.OltConfigInfoDefault.VendorContactLen = VOS_StrLen( OltVendorContactDefault );
#endif
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.DeviceNameDefault, OltDeviceNameDefault, VOS_StrLen( OltDeviceNameDefault ));
	OLTMgmt.OltConfigInfoDefault.DeviceNameLen = VOS_StrLen( OltDeviceNameDefault );
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.DeviceDesc, OltDeviceDescriptionDefault, VOS_StrLen( OltDeviceDescriptionDefault ) );
	OLTMgmt.OltConfigInfoDefault.DeviceDescLen = VOS_StrLen( OltDeviceDescriptionDefault );
#if 0
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.DeviceTypeDefault, OltDeviceTypeDefault, VOS_StrLen( OltDeviceTypeDefault ));
	OLTMgmt.OltConfigInfoDefault.DeviceTypeLen = VOS_StrLen( OltDeviceTypeDefault );
#endif
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.LocationDefault, OltLocationDefault, VOS_StrLen( OltLocationDefault ));
	OLTMgmt.OltConfigInfoDefault.LocationLen = VOS_StrLen( OltLocationDefault );
	VOS_MemCpy( OLTMgmt.OltConfigInfoDefault.MacAddrDefault, OltMacAddrDefault, BYTES_IN_MAC_ADDRESS );


	/* 运行数据初始化 */
	OLTMgmt.adminStatus = OltAdminStatusDefault; /*OLTMgmt.OltConfigInfoDefault.adminStatusDefault ;*/
	OLTMgmt.operStatus = OltOperStatusDefault; /*OLTMgmt.OltConfigInfoDefault.operStatusDefault;*/
	OLTMgmt.PWUtemperatureThreshold_dis =  OLTMgmt.OltConfigInfoDefault.PWUtemperatureThreshold_dis_Default;
	OLTMgmt.PWUtemperatureThreshold_gen = OLTMgmt.OltConfigInfoDefault.PWUtemperatureThreshold_gen_Default;
	OLTMgmt.temperatureThreshold_dis = OLTMgmt.OltConfigInfoDefault.temperatureThreshold_dis_Default;
	OLTMgmt.temperatureThreshold_gen = OLTMgmt.OltConfigInfoDefault.temperatureThreshold_gen_Default;
	OLTMgmt.AlarmStatus = 0;
	OLTMgmt.AlarmMask = OLTMgmt.OltConfigInfoDefault.AlarmMaskDefault;
	OLTMgmt.MaxPonPort = OLTMgmt.OltConfigInfoDefault.MaxPonPortDefault;
	/*
	VOS_MemCpy( OLTMgmt.DeviceInfo.VendorInfo, OltVendorInfoDefault, VOS_StrLen( OltVendorInfoDefault ));
	OLTMgmt.DeviceInfo.VendorInfoLen = VOS_StrLen( OltVendorInfoDefault );*/
#if 0
	VOS_MemCpy( OLTMgmt.DeviceInfo.VendorLocation, OLTMgmt.OltConfigInfoDefault.VendorLocationDefault, VOS_StrLen( OLTMgmt.OltConfigInfoDefault.VendorLocationDefault ) );
	VOS_MemCpy( OLTMgmt.DeviceInfo.VendorContact, OLTMgmt.OltConfigInfoDefault.VendorContactDefault, VOS_StrLen( OLTMgmt.OltConfigInfoDefault.VendorContactDefault ));
	OLTMgmt.DeviceInfo.VendorLocationLen = VOS_StrLen(  OltVendorLocationDefault );
	OLTMgmt.DeviceInfo.VendorContactLen = VOS_StrLen ( OltVendorContactDefault );
#endif
	/* modified by xieshl 20111123，防止丢掉结束符，问题单13917 */
	OLTMgmt.DeviceInfo.DeviceNameLen = VOS_StrLen( OLTMgmt.OltConfigInfoDefault.DeviceNameDefault );
	OLTMgmt.DeviceInfo.DeviceDescLen = VOS_StrLen ( OLTMgmt.OltConfigInfoDefault.DeviceDesc );
	OLTMgmt.DeviceInfo.LocationLen = VOS_StrLen( OLTMgmt.OltConfigInfoDefault.LocationDefault );
	OLTMgmt.DeviceInfo.SwVersionLen = VOS_StrLen( V2R1_VERSION );

	if( OLTMgmt.DeviceInfo.DeviceNameLen >= MAXDEVICENAMELEN )
	{
		OLTMgmt.DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN - 1;
		OLTMgmt.OltConfigInfoDefault.DeviceNameDefault[OLTMgmt.DeviceInfo.DeviceNameLen] = 0;
	}
	if( OLTMgmt.DeviceInfo.DeviceDescLen >= MAXDEVICEDESCLEN )
	{
		OLTMgmt.DeviceInfo.DeviceDescLen = MAXDEVICEDESCLEN - 1;
		OLTMgmt.OltConfigInfoDefault.DeviceDesc[OLTMgmt.DeviceInfo.DeviceDescLen] = 0;
	}
	if( OLTMgmt.DeviceInfo.LocationLen >= MAXLOCATIONLEN )
	{
		OLTMgmt.DeviceInfo.LocationLen = MAXLOCATIONLEN -1;
		OLTMgmt.DeviceInfo.Location[OLTMgmt.DeviceInfo.LocationLen] = 0;
	}
	
	VOS_StrnCpy( OLTMgmt.DeviceInfo.DeviceName, OLTMgmt.OltConfigInfoDefault.DeviceNameDefault, MAXDEVICENAMELEN);
	VOS_StrnCpy( OLTMgmt.DeviceInfo.DeviceDesc, OLTMgmt.OltConfigInfoDefault.DeviceDesc, MAXDEVICEDESCLEN );
#if 0
	VOS_MemCpy( OLTMgmt.DeviceInfo.DeviceType, OLTMgmt.OltConfigInfoDefault.DeviceTypeDefault, VOS_StrLen( OLTMgmt.OltConfigInfoDefault.DeviceTypeDefault ));
#endif
	VOS_StrnCpy( OLTMgmt.DeviceInfo.Location, OLTMgmt.OltConfigInfoDefault.LocationDefault, MAXLOCATIONLEN);
	VOS_MemCpy( OLTMgmt.DeviceInfo.MacAddr, OLTMgmt.OltConfigInfoDefault.MacAddrDefault, BYTES_IN_MAC_ADDRESS );
	VOS_StrCpy( OLTMgmt.DeviceInfo.SwVersion, V2R1_VERSION);
	
	/*OLTMgmt.MaxPonPort = OLTMgmt.OltConfigInfoDefault.MaxPonPortDefault;*/

	HOST_MAC_ADDRESS_VXWORKS[BYTES_IN_MAC_ADDRESS-1] = SYS_LOCAL_MODULE_SLOTNO;

	return ( ROK );
}


/**************************************************************
 *
 *    Function:  SetOltVendorInfo( char *Info, int len )
 *
 *    Param:  char *Name -- the vendor name to be set
 *                int  len -- the length of the vendor name 
 *
 *    Desc:  set the Olt Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltVendorInfo( char *Info, int len )
{
#if 0
	int  GetSemID;
	
	/*GetSemID = semTake(OltMgmtDataSemId, WAIT_FOREVER ); */		
	GetSemID = VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER);

	if( GetSemID == VOS_ERROR ) {
		sys_console_printf( " get SemId error (SetOltVendorInfo())\r\n" );
		return ( RERROR );
		}
#endif	

	if( len > MAXVENDORINFOLEN ) 
		{
		/*sys_console_printf("Olt Vendor Name is to long %d (SetOltVendorInfo())\r\n", len );*/
		len = MAXVENDORINFOLEN - 1;
		}
	VOS_MemSet( &OltVendorInfo[0], 0, MAXVENDORINFOLEN );
	VOS_MemCpy( &OltVendorInfo[0], Info, len );
	OltVendorInfo[len] = '\0';
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltVendorInfo( char *Info, int *len)
{
	if( (Info == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("  error:the pointer is null(GetOltVendorInfo())\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );		
		}
	*len = VOS_StrLen(&OltVendorInfo[0]);
	/*VOS_MemCpy( Info, &OltVendorInfo[0], *len);*/
	VOS_StrCpy( Info, &OltVendorInfo[0] );	/* modified by xieshl 20091216, 防止漏掉结束符 */
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOltVendorLocation( unsigned char *Location, unsigned char len )
 *
 *    Param:  unsigned char *Location -- the vendor Location to be set
 *                 unsigned char  len -- the length of the vendor location 
 *
 *    Desc:  set the olt Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOltVendorLocation( unsigned char *Location, int len )
{


	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltVendorLocation() )\r\n" );
		return ( RERROR );
		}

	if( len > MAXVENDORLOCATIONLEN ) {
		sys_console_printf(" Olt Vendor Location is to long %d (SetOltVendorLocation() )\r\n", len );
		len = MAXVENDORLOCATIONLEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.VendorLocation, 0, MAXVENDORLOCATIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.VendorLocation, Location, len );
	OLTMgmt.DeviceInfo.VendorLocationLen = len;
	
	VOS_SemGive(OltMgmtDataSemId);	
	return ( ROK );

}


int  GetOltVendorLocation( unsigned char *Location , int *len )
{
	if(( Location== NULL ) ||( len == NULL)){
		sys_console_printf("error : the pointer is null ( GetOltVendorLocation() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Location, OLTMgmt.DeviceInfo.VendorLocation, OLTMgmt.DeviceInfo.VendorLocationLen);
	*len = OLTMgmt.DeviceInfo.VendorLocationLen;
	
	return( ROK );

}
#endif
/**************************************************************
 *
 *    Function:  SetOltVendorContact( unsigned char *Telenum, int len )
 *
 *    Param:  unsigned char *Telenum -- the vendor Contact to be set
 *               int len -- the length of the vendor Contact 
 *
 *    Desc:  set the olt Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOltVendorContact( unsigned char *Telenum, int  len )
{

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltVendorLocation() )\r\n" );
		return ( RERROR );
		}

	if( len > MAXVENDORCONTACTLEN ) {
		sys_console_printf(" Olt Vendor Contact is to long %d (GetOltVendorContact() )\r\n", len);
		len = MAXVENDORCONTACTLEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.VendorContact, 0, MAXVENDORCONTACTLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.VendorContact, Telenum, len );
	OLTMgmt.DeviceInfo.VendorContactLen = len;
	
	VOS_SemGive(OltMgmtDataSemId);
	return ( ROK );
}


int  GetOltVendorContact( unsigned char *Contact , int *len)
{
	if(( Contact == NULL )||( len == NULL)) {
		sys_console_printf("error : the pointer is null ( GetOltVendorContact() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Contact, OLTMgmt.DeviceInfo.VendorContact, OLTMgmt.DeviceInfo.VendorContactLen);
	*len = OLTMgmt.DeviceInfo.VendorContactLen;
	return( ROK );
}
#endif

/**************************************************************
 *
 *    Function:  SetOltCustomerName( unsigned char *Name, int len )
 *
 *    Param:  unsigned char *Name -- the Customer name to be set
 *               int  len -- the length of the Customer name 
 *
 *    Desc:  set the Olt Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOltCustomerName( unsigned char *Name, int len )
{

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOltCustomerName() )\r\n" );
		return ( RERROR );
		}

	if( len > MAXCUSTOMERNAMELEN ) {
		sys_console_printf(" Olt Customer Name is to long %d (SetOltCustomerName() )\r\n", len );
		len = MAXCUSTOMERNAMELEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.CustomerName, 0, MAXCUSTOMERNAMELEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.CustomerName, Name, len );
	OLTMgmt.DeviceInfo.CustomerNameLen = len;
	
	VOS_SemGive(OltMgmtDataSemId);
	return ( ROK );
}


int  GetOltCustomerName( unsigned char *Name, int *len )
{
	if(( Name == NULL )||( len == NULL)) {
		sys_console_printf("error : the pointer is null (GetOltCustomerName() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Name, OLTMgmt.DeviceInfo.CustomerName, OLTMgmt.DeviceInfo.CustomerNameLen );
	*len = OLTMgmt.DeviceInfo.CustomerNameLen;
	return( ROK );
}
#endif
/**************************************************************
 *
 *    Function:  SetOltCustomerLocation( unsigned char *Location, int len )
 *
 *    Param:  unsigned char *Location -- the Customer Location to be set
 *                int  len -- the length of the Customer location 
 *
 *    Desc:  set the olt Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOltCustomerLocation( unsigned char *Location, int len )
{

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltCustomerLocation() )\r\n");
		return ( RERROR );
		}

	if( len > MAXCUSTOMERLOCATIONLEN ) {
		sys_console_printf(" Olt Customer Location is to long %d \r\n", len );
		len = MAXCUSTOMERLOCATIONLEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.CustomerLocation, 0, MAXCUSTOMERLOCATIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.CustomerLocation, Location, len );
	OLTMgmt.DeviceInfo.CustomerLocationLen = len;
	
	VOS_SemGive(OltMgmtDataSemId);
	return ( ROK );
}

int  GetOltCustomerLocation( unsigned char *Location, int *len )
{
	if(( Location == NULL )||( len == NULL )) {
		sys_console_printf("error : the pointer is null (GetOltCustomerLocation() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Location, OLTMgmt.DeviceInfo.CustomerLocation, OLTMgmt.DeviceInfo.CustomerLocationLen);
	*len = OLTMgmt.DeviceInfo.CustomerLocationLen;
	return( ROK );
}
#endif
/**************************************************************
 *
 *    Function:  SetOltCustomerContact( unsigned char *Telenum, int len )
 *
 *    Param:  unsigned char *Telenum -- the Customer Contact to be set
 *               int len -- the length of the Customer Contact 
 *
 *    Desc:  set the olt Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOltCustomerContact( unsigned char *Telenum, int len )
{

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOltCustomerContact() )\r\n" );
		return ( RERROR );
		}
	
	if( len > MAXCUSTOMERCONTACTLEN) {
		sys_console_printf(" Olt Customer Contact is to long %d (GetOltCustomerContact() ) \r\n", len );
		len = MAXCUSTOMERCONTACTLEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.CustomerContact, 0, MAXCUSTOMERCONTACTLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.CustomerContact, Telenum, len );
	OLTMgmt.DeviceInfo.CustomerContactLen = len;

	VOS_SemGive(OltMgmtDataSemId);
	return ( ROK );
}


int  GetOltCustomerContact( unsigned char *Contact, int *len )
{
	if((Contact == NULL )||( len == NULL )) {
		sys_console_printf("error : the pointer is null (GetOltCustomerContact() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Contact, OLTMgmt.DeviceInfo.CustomerContact, OLTMgmt.DeviceInfo.CustomerContactLen );
	*len = OLTMgmt.DeviceInfo.CustomerContactLen;
	return( ROK );
}
#endif
/**************************************************************
 *
 *    Function:  SetOltDeviceName( unsigned char *Name, unsigned char len );
 *
 *    Param:  unsigned char *Name -- the Device name to be set
 *               unsigned char  len -- the length of the Device name 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltDeviceName( char *Name, int  len )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltDeviceName() )\r\n" );
		return ( RERROR );
		}
#endif
	if( len > MAXDEVICENAMELEN ) 
		{
		/*sys_console_printf(" Olt Device Name is to long %d (SetOltDeviceName() )\r\n", len );*/
		len = MAXDEVICENAMELEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.DeviceName, 0, MAXDEVICENAMELEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.DeviceName, Name, len );
	OLTMgmt.DeviceInfo.DeviceNameLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );
}

int  GetOltDeviceName(  char *Name ,  int *NameLen)
{
	if(( Name == NULL ) || ( NameLen == NULL ) ) 
	{
		/*sys_console_printf("  error:the pointer is null(GetOltDeviceName)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
	}
	/* modified by xieshl 20111122, 防止漏掉结束符 */
	if( OLTMgmt.DeviceInfo.DeviceNameLen >= MAXDEVICENAMELEN )
	{
		OLTMgmt.DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN - 1;
	}
	VOS_StrnCpy( Name, OLTMgmt.DeviceInfo.DeviceName, OLTMgmt.DeviceInfo.DeviceNameLen);
	Name[OLTMgmt.DeviceInfo.DeviceNameLen] = 0;
	*NameLen = OLTMgmt.DeviceInfo.DeviceNameLen;
	return( ROK );
}

int  GetOltDeviceNameDefault(  char *Name ,  int *NameLen)
{
	if(( Name == NULL ) || ( NameLen == NULL ) ) 
	{
		/*sys_console_printf("  error:the pointer is null(GetOltDeviceNameDefault)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
	}
	/* modified by xieshl 20111122, 防止漏掉结束符 */
	if( OLTMgmt.OltConfigInfoDefault.DeviceNameLen >= MAXDEVICENAMELEN )
	{
		OLTMgmt.DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN - 1;
	}
	VOS_StrnCpy( Name, OLTMgmt.OltConfigInfoDefault.DeviceNameDefault, OLTMgmt.OltConfigInfoDefault.DeviceNameLen);	
	Name[OLTMgmt.OltConfigInfoDefault.DeviceNameLen] = 0;
	*NameLen = OLTMgmt.OltConfigInfoDefault.DeviceNameLen;
	return( ROK );

}

/**************************************************************
 *
 *    Function:  SetOltDeviceDesc( char *Desc, int len )
 *
 *    Param:  unsigned char *Desc -- the Device Desc to be set
 *               unsigned char  len -- the length of the Device desc 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int SetOltDeviceDesc( char *Desc, int len )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltDeviceDesc() )\r\n" );
		return ( RERROR );
		}
#endif
	if( len > MAXDEVICENAMELEN ) 
		{
		/*sys_console_printf(" Olt Device Name is to long %d (SetOltDeviceDesc() )\r\n", len );*/
		len = MAXDEVICENAMELEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.DeviceDesc, 0, MAXDEVICEDESCLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.DeviceDesc, Desc, len );
	OLTMgmt.DeviceInfo.DeviceDescLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltDeviceDesc( char *Desc, int *len )
{
	if(( Desc == NULL ) || ( len == NULL ) ) 
		{
		/*sys_console_printf("  error:the pointer is null (GetOltDeviceName)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Desc, OLTMgmt.DeviceInfo.DeviceDesc);
	*len = OLTMgmt.DeviceInfo.DeviceDescLen;
	return( ROK );
}

int  GetOltDeviceDescDefault( char *Desc, int *len)
{
	if(( Desc == NULL ) || ( len == NULL ) ) 
		{
		/*sys_console_printf("  error:the pointer is null(GetOltDeviceDescDefault)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Desc, OLTMgmt.OltConfigInfoDefault.DeviceDesc);
	*len = OLTMgmt.OltConfigInfoDefault.DeviceDescLen;
	return( ROK );

}

/**************************************************************
 *
 *    Function:  SetOltDeviceType( unsigned char *Type, int len );
 *
 *    Param:  unsigned char *Type -- the Device Type to be set
 *               int  len -- the length of the Device Type 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 /*
int  SetOltDeviceType( unsigned char *Type, int len )
{


	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOltDeviceType() )\r\n" );
		return ( RERROR );
		}
	
	if( len > MAXDEVICETYPELEN ) {
		sys_console_printf(" Olt Device Type is to long %d (SetOltDeviceType() ) \r\n", len );
		len = MAXDEVICETYPELEN;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.DeviceType, 0, MAXDEVICETYPELEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.DeviceType, Type, len );
	OLTMgmt.DeviceInfo.DeviceTypeLen = len;

	VOS_SemGive(OltMgmtDataSemId);
	return ( ROK );
}

int  GetOltDeviceType( unsigned char  *Type, int *len  )
{
	if((Type == NULL ) ||( len == NULL )){
		sys_console_printf("error : the pointer is null (GetOltDeviceType() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Type, OLTMgmt.DeviceInfo.DeviceType, OLTMgmt.DeviceInfo.DeviceTypeLen );
	*len = OLTMgmt.DeviceInfo.DeviceTypeLen ;
	return( ROK );
}*/

int GetOltType()
{
	if( !V2R1_PRODUCT_IS_EPON(OLTMgmt.DeviceInfo.type) )
		OLTMgmt.DeviceInfo.type =  V2R1_DEVICE_UNKNOWN;

	return (OLTMgmt.DeviceInfo.type);
}
/**************************************************************
 *
 *    Function:  SetOltSWVersion( char *SwVersion, int len );
 *
 *    Param:  char *SwVersion -- the Olt SwVersion to be set
 *               int len -- the length of the Olt SwVersion 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltSWVersion( char *SwVersion, int len )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltSWVersion() )\r\n" );
		return ( RERROR );
		}
#endif
	if( len > MAXSWVERSIONLEN ) 
		{
		/*sys_console_printf(" Olt SwVersion is to long %d  (SetOltSWVersion() )\r\n", len );*/
		len = MAXSWVERSIONLEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.SwVersion, 0, MAXSWVERSIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.SwVersion, SwVersion, len );
	OLTMgmt.DeviceInfo.SwVersionLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltSWVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("error : the pointer is null ( GetOltSWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*VOS_MemCpy( Version, OLTMgmt.DeviceInfo.SwVersion, OLTMgmt.DeviceInfo.SwVersionLen);*/
	*len = OLTMgmt.DeviceInfo.SwVersionLen;
	*len = VOS_StrLen(V2R1_VERSION);
	VOS_StrCpy( Version, V2R1_VERSION );
	return( ROK );
}

extern char PAS_VERSION[];
int  GetOltPASVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("error : the pointer is null ( GetOltSWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*VOS_MemCpy( Version, OLTMgmt.DeviceInfo.SwVersion, OLTMgmt.DeviceInfo.SwVersionLen);*/
	*len = VOS_StrLen( PAS_VERSION );
	VOS_StrCpy( Version, PAS_VERSION );
	return( ROK );
}

extern char TK_VERSION[];
int  GetOltTKVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("error : the pointer is null ( GetOltSWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*VOS_MemCpy( Version, OLTMgmt.DeviceInfo.SwVersion, OLTMgmt.DeviceInfo.SwVersionLen);*/
	*len = VOS_StrLen( TK_VERSION );
	VOS_StrCpy( Version, TK_VERSION );
	return( ROK );
}

extern char BCM_VERSION[];
int  GetOltBcmVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("error : the pointer is null ( GetOltSWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*VOS_MemCpy( Version, OLTMgmt.DeviceInfo.SwVersion, OLTMgmt.DeviceInfo.SwVersionLen);*/
	*len = VOS_StrLen( BCM_VERSION );
	VOS_StrCpy( Version, BCM_VERSION );
	return( ROK );
}

extern char GPON_VERSION[];
int  GetOltGponVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL)) 
		{
		/*sys_console_printf("error : the pointer is null ( GetOltSWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*VOS_MemCpy( Version, OLTMgmt.DeviceInfo.SwVersion, OLTMgmt.DeviceInfo.SwVersionLen);*/
	*len = VOS_StrLen( GPON_VERSION );
	VOS_StrCpy( Version, GPON_VERSION );
	return( ROK );
}
/**************************************************************
 *
 *    Function:  SetOltSWVersion( char *HwVersion, int len );
 *
 *    Param:  char *HwVersion -- the Olt HwVersion to be set
 *               int len -- the length of the Olt HwVersion 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltHWVersion( char *HwVersion, int len )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltHWVersion() )\r\n" );
		return ( RERROR );
		}
#endif
	if( len > MAXHWVERSIONLEN ) 
		{
		/*sys_console_printf(" Olt HwVersion is to long %d  (SetOltHWVersion() )\r\n", len );*/
		len = MAXHWVERSIONLEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.HwVersion, 0, MAXHWVERSIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.HwVersion, HwVersion, len );
	OLTMgmt.DeviceInfo.HwVersionLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltHWVersion( char *Version, int *len )
{
	if( (Version == NULL )||( len == NULL ))
		{
		/*sys_console_printf("error : the pointer is null (GetOltHWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Version, OLTMgmt.DeviceInfo.HwVersion);
	*len = OLTMgmt.DeviceInfo.HwVersionLen;
	return( ROK );
}

int GetDeviceInfoFromHardware()
{
	/* read Info from flash or hardware */

	/* record this Info to the data: OLTMgmt.DeviceInfo.HwVersion, SwVersion, FwVersion, etc */

	
	return( ROK );
}


/**************************************************************
 *
 *    Function:  SetOltFWVersion( char *FwVersion, int len );
 *
 *    Param:  char *SwVersion -- the Olt FwVersion to be set
 *               int len -- the length of the Olt FwVersion 
 *
 *    Desc:  set the Olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltFWVersion( char *FwVersion, int len )
{
#if 0
	if(  VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltFWVersion() )\r\n" );
		return ( RERROR );
		}
#endif
	if( len > MAXFWVERSIONLEN ) 
		{
		/*sys_console_printf(" Olt FwVersion is to long %d ( SetOltFWVersion() )\r\n", len );*/
		len = MAXFWVERSIONLEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.FwVersion, 0, MAXFWVERSIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.FwVersion, FwVersion, len );
	OLTMgmt.DeviceInfo.FwVersionLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltFWVersion( char *Version, int *len)
{
	if( (Version == NULL )||( len == NULL )) 
		{
		/*sys_console_printf("error : the pointer is null (GetOltFWVersion() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Version, OLTMgmt.DeviceInfo.FwVersion );
	*len = OLTMgmt.DeviceInfo.FwVersionLen ;
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOltLocation( char *Location, int len )
 *
 *    Param:   char *Location -- the Olt Device Location to be set
 *                int len -- the length of the Olt Device location 
 *
 *    Desc:  set the olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltLocation( char *Location, int  len )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOltLocation() )\r\n" );
		return ( RERROR );
		}
#endif
	/*MaxLen = sizeof ( OLTMgmt.DeviceInfo.Location );*/
	if( len > MAXLOCATIONLEN ) 
		{
		/*sys_console_printf(" Olt Device Location is to long %d ( SetOltLocation() ) \r\n", len );*/
		len = MAXLOCATIONLEN - 1;
		}
	VOS_MemSet( OLTMgmt.DeviceInfo.Location, 0, MAXLOCATIONLEN );
	VOS_MemCpy( OLTMgmt.DeviceInfo.Location, Location, len );
	OLTMgmt.DeviceInfo.LocationLen = len;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}


int  GetOltLocation( char *Location, int *len )
{
	if(( Location == NULL ) || ( len == NULL)) 
		{
		/*sys_console_printf("  error : the pointer is null(GetOltLocation())\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Location, OLTMgmt.DeviceInfo.Location);
	*len = OLTMgmt.DeviceInfo.LocationLen;
	return( ROK );
}

int  GetOltLocationDefault( char *Location, int *len )
{
	if(( Location == NULL ) || ( len == NULL)) 
		{
		/*sys_console_printf("  error:the pointer is null(GetOltLocationDefault)\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_StrCpy( Location, OLTMgmt.OltConfigInfoDefault.LocationDefault);
	*len = OLTMgmt.OltConfigInfoDefault.LocationLen;
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetPonMgmtMacAddr( unsigned char *MacAddr )
 *
 *    Param:  unsigned char *MacAddr -- the Pon management channel Host Mac Address to be set , the 
 *                                               Mac address is used to communication with PON chip
 *
 *    Desc:  set the olt Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOltMgmtMacAddr( char *MacAddr, int len)
{


	if( len > BYTES_IN_MAC_ADDRESS ) 
		{
		/*sys_console_printf(" PON chip management channel Host MAC is to long %d ( file %s SetPonMgmtMacAddr() )\r\n", len,__FILE__ );*/
		len = BYTES_IN_MAC_ADDRESS - 1;
		}

	
	len = BYTES_IN_MAC_ADDRESS;
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( file %s SetPonMgmtMacAddr() )\r\n", __FILE__ );
		return( RERROR );
		}
#endif	
	VOS_MemSet( OLTMgmt.DeviceInfo.MacAddr, 0, len );
	VOS_MemCpy( OLTMgmt.DeviceInfo.MacAddr, MacAddr, len );
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

}

int  GetOltMgmtMAC( char *MacAddr, int *len )
{
	/*Enet_MACAdd_Infor1 OltMac;*/
		
	if(( MacAddr == NULL )||( len == NULL)){
		/*sys_console_printf("error: the address pointer is null (GetOltMgmtMAC())\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	/*
	 获得系统的MAC地址 
	ULONG device_product_basemac_get( UCHAR * mac_mem )
	*/
	/* modified by xieshl 20091216, 防止读eeprom */
	/*funReadMacAddFromNvram(&OltMac );
	OltMac.bMacAdd, BYTES_IN_MAC_ADDRESS );*/
	VOS_MemCpy( MacAddr, SYS_PRODUCT_BASEMAC, BYTES_IN_MAC_ADDRESS );
	*len = BYTES_IN_MAC_ADDRESS;
	
	return( ROK );
}


/**************************************************************
 *
 *    Function:  RecordSysLaunchTime ( Date_S LaunchDate )
 *
 *    Param:  Date_S LaunchDate -- the Olt Device First LaunchTime ,
 *                                          indicated as year/month/day/hour/minute/second/MilliSecond
 *
 *    Desc:  set the olt Device first Launch Time
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  RecordSysLaunchTime ()
{
	Date_S LaunchDate;
	int ret = ROK;
		
	/* get the current Date from realTime */
	if( GetSysCurrentTime( &LaunchDate ) == RERROR ) return ( RERROR );
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( RecordSysLaunchTime() )\r\n" );
		ret = RERROR;
		}
#endif
/*
	if(( LaunchDate.year >2050 ) ||(LaunchDate.year < 2005 )){
		sys_console_printf(" the YEAR %d of the DATE is not right( RecordSysLaunchTime() )  \r\n", LaunchDate.year );
		ret = RERROR ;
		}
	if(( LaunchDate.month > 12 ) || (LaunchDate.month == 0)){
		sys_console_printf(" the MONTH %d of the DATE is not right( RecordSysLaunchTime() ) \r\n", LaunchDate.month  );
		ret = RERROR ;
		}
	if(( LaunchDate.day > 31 ) ||(LaunchDate.day ==0 )){
		sys_console_printf(" the DAY %d of the DATE is not right ( RecordSysLaunchTime() )\r\n", LaunchDate.day  );
		ret = RERROR ;
		}
	if(( LaunchDate.hour > 23 ) ||(LaunchDate.hour ==0 )){
		sys_console_printf(" the HOUR %d of the DATE is not right ( RecordSysLaunchTime() )\r\n", LaunchDate.hour  );
		ret = RERROR ;
		}
	if( LaunchDate.minute > 59 ){
		sys_console_printf(" the MINUTE %d of the DATE is not right( RecordSysLaunchTime() ) \r\n", LaunchDate.minute  );
		ret = RERROR ;
		}
	if(LaunchDate.second > 59 ){
		sys_console_printf(" the SECOND  %d of the DATE is not right( RecordSysLaunchTime() ) \r\n", LaunchDate.second  );
		ret = RERROR ;
		}	
	if( ret != RERROR )
	*/
		VOS_MemCpy ((char *) &(OLTMgmt.DeviceInfo.SysLaunchTime.year), (char *)&(LaunchDate.year), sizeof( Date_S) );
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif	
	return ( ret );
}

/**************************************************************
 *
 *    Function: GetSysLaunchTime ( Date_S *LaunchDate )
 *
 *    Param
 *                 out : Date_S *LaunchDate -- the pointer of Olt Device First LaunchTime ,
 *                                          indicated as year/month/day/hour/minute/second
 *
 *    Desc:  get the olt Device first Launch Time
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
extern int get_gfasw_sys_up_time( sysDateAndTime_t *pTime );
int  GetSysLaunchTime ( Date_S *LaunchDate )
{
	return get_gfasw_sys_up_time( (sysDateAndTime_t *)LaunchDate );
#if 0
	if( LaunchDate == NULL ){
		sys_console_printf("  error: the pointer is null(GetSysLaunchTime())\r\n" );
		return( RERROR );
		}
	VOS_MemCpy( (unsigned char *)LaunchDate, (unsigned char *)&( OLTMgmt.DeviceInfo.SysLaunchTime ), sizeof(Date_S ) );
	return ( ROK );
#endif
}

/**************************************************************
 *
 *    Function: RecordOLtSysUpTime ()
 *
 *    Param : none
 *
 *    Desc:  get the current system ticks and record it
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  RecordOLtSysUpTime( void )
{
	/* modified by xieshl 20110512, 防止时间溢出, 和ONU统一 */
	/*unsigned  int tick;

	tick = VOS_GetTick();
	OLTMgmt.DeviceInfo.SysUptime = tick;
	OLTMgmt.DeviceInfo.RelativeUptime = 0;*/
	OLTMgmt.DeviceInfo.SysUptime = time(0);
	OLTMgmt.DeviceInfo.RelativeUptime = OLTMgmt.DeviceInfo.SysUptime;

	return( ROK );
}

int  GetOltSysUpTime( void )
{
	/*if(  GetOltOperStatus() ){
		return(OLTMgmt.DeviceInfo.SysUptime );
		}
	else{ return ( RERROR ); }*/
	ULONG ti = time(0);
	if(  GetOltOperStatus() )
	{
		return(ti - OLTMgmt.DeviceInfo.SysUptime );
	}
	else{ return ( RERROR ); }
}

#if 0	/* removed by xieshl 20110902, 这个函数返回值是错误的，禁用，问题单13410 */
int  GetOltSysUpLastTime( void )
{
	/*int tick;
	int CurTick;
	
	CurTick = VOS_GetTick();
	tick = GetOltSysUpTime();

	if( tick != RERROR ){ return ( CurTick - tick ); }
	else { return ( RERROR ); }*/
	int ti;
	int CurTime;
	
	CurTime = time(0);
	ti = GetOltSysUpTime();

	if( ti != RERROR ){ return (CurTime - ti); }
	else { return ( RERROR ); }
}
#endif

/**************************************************************
 *
 *    Function: GetOltOperStatus( )
 *
 *    Param:
 *
 *    Desc:  get the olt Device operation status
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  GetOltOperStatus()
{
	return( OLTMgmt.operStatus );
}

int GetOltAdminStatus()
{
	return( OLTMgmt.adminStatus );
}

int ResetOlt()
{
	devsm_reset_system();
	/*sys_console_printf("\r\n---=== Reset the Olt  ===---\r\n");*/
	return( ROK );
}

int GetOltAlarmMask( unsigned long *pMask )
{
	if( pMask == NULL )
		return RERROR;
	*pMask = OLTMgmt.AlarmMask & EVENT_MASK_DEV_ALL;	/*modified by zhengyt@10-3-22,设备告警屏蔽对olt也要新增告警类型*/
	return ( ROK );
}
int SetOltAlarmMask( unsigned long mask )
{
	OLTMgmt.AlarmMask = (mask & EVENT_MASK_DEV_ALL);		/*modified by zhengyt@10-3-22,设备告警屏蔽对olt也要新增告警类型*/
	return( ROK );
}


/**************************************************************
 *
 *    Function: SetOltTemperatureThreshold(unsigned char gen_threshold, unsigned char dis_threshold)
 *
 *    Param:    unsigned char gen_threshold--temperature Alarm generate threshold 
 *                   unsigned char dis_threshold -- temperature Alarm disappear threshold
 *
 *    Desc:  set the olt Device temperature alarm threshold
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int SetOltTemperatureThreshold_gen( unsigned char AlarmThreshold )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltTemperatureThreshold_gen() )\r\n" );
		return( RERROR );
		}
#endif
	OLTMgmt.temperatureThreshold_gen = AlarmThreshold;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
}

int SetOltTemperatureThreshold_dis( unsigned char AlarmThreshold )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltTemperatureThreshold_dis() )\r\n" );
		return( RERROR );
		}
#endif	
	OLTMgmt.temperatureThreshold_dis = AlarmThreshold;
#if 0	
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
}


int SetOltTemperatureThreshold( unsigned char AlarmThreshold_gen, unsigned char AlarmThreshold_dis )
{
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltTemperatureThreshold() )\r\n" );
		return( RERROR );
		}
#endif
	OLTMgmt.temperatureThreshold_gen = AlarmThreshold_gen;
	OLTMgmt.temperatureThreshold_dis = AlarmThreshold_dis;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
}

/**************************************************************
 *
 *    Function: GetOltTemperatureThreshold_gen()
 *
 *    Param:    
 *
 *    Desc:  get the olt Device temperature alarm threshold
 *
 *    Return:   temperature alarm threshold
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int GetOltTemperatureThreshold_gen( unsigned char  *AlarmThreshold )
{
	if( AlarmThreshold == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	*AlarmThreshold=  OLTMgmt.temperatureThreshold_gen ;
	return( ROK );
}

int GetOltTemperatureThreshold_dis( unsigned char  *AlarmThreshold )
{
	if( AlarmThreshold == NULL )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	*AlarmThreshold =OLTMgmt.temperatureThreshold_dis;
	return( ROK);
}

/**************************************************************
 *
 *    Function: SetAlarmMask( unsigned int alarmIdx, unsigned int alarmMask )
 *
 *    Param:    unsigned int alarmIdx -- Alarm Index, used to identify the specific Alarm
 *                  unsigned int alarmMask -- Used to Mask the alarm or open 
 *
 *    Desc:  set the alarm Mask flag; Also, if there is a alarm and the alarmMask is OPEN , then 
 *               a alarm generation msg shoud be send ; if there is a alarm and the alarmMask is CLOSE, 
 *               a alarm disappear msg should be send .
 *
 *    Return:   ROK
 *
 *    Notes: this function need to be modified when debugging
 *
 *    modified:
 *
 ***************************************************************/
int  SetAlarmMask( unsigned int alarmIdx, unsigned long alarmMask )
{
	if( alarmIdx >  ALLALARMNUM ){
		/*sys_console_printf(" the alarm Id %d is not exist  (SetAlarmMask() )\r\n", alarmIdx );*/
		return( RERROR );
		}
#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetAlarmMask() )\r\n" );
		return( RERROR );
		}
#endif		
	AlarmInfoTable[alarmIdx].AlarmMask = alarmMask;
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return ( ROK );

#if 0
	if( alarmMask == ALARMOPEN ){	
		
		if( alarmIdx in the OLT alarm ){
			
			if( alarmIdx == PWUALARMID ){
				unsigned char whichPWU;
				/* check whether the first PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << FIRSTPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt first PWU ), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				/* check whether the second PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << SECONDPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt second PWU ), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				/* check whether the third PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << THIRDPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt third PWU ), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			
			else if( alarmIdx == FANALARMID ){
				if( ( OLTMgmt.AlarmStatus & ( 1 << FANALARM_BIT ) ) == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt FAN), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			
			else if( alarmIdx == TEMPERATUREALARMID ){
				if( ( OLTMgmt.AlarmStatus & ( 1 << TEMPERATUREALARM_BIT ) ) == ALARMEXIST ){
					if( OLTMgmt.environment.UpLinkTemperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt Uplink), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWU1Temperature>=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU1), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWU2Temperature >=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU2), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWU3Temperature >=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU3), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.SelfTemperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt SW1), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON1Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON1), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON2Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON2), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON3Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON3), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON4Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON4), Len, TrapId(产生), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			}

		
		else if( alarmIdx in the PON alarm ){
			}
		else if( alarmIdx in the ONU alarm ){
			}
		semGive(OltMgmtDataSemId);
		return( ROK );
		}
	
	else if( alarmMask == ALARMMASK ){
		
		if( alarmIdx in the OLT alarm ){
			
			if( alarmIdx == PWUALARMID ){
				
				unsigned char whichPWU;
				/* check whether the first PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << FIRSTPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt first PWU ), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				/* check whether the second PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << SECONDPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt second PWU ), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				/* check whether the third PWU alarm is exist */
				whichPWU = OLTMgmt.AlarmStatus & ( 1 << THIRDPWUALARM_BIT );
				if( whichPWU == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt third PWU ), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			
			else if( alarmIdx == FANALARMID ){
				if( ( OLTMgmt.AlarmStatus & ( 1 << FANALARM_BIT ) ) == ALARMEXIST ){
					AlarmDispatch( Alarmsource(olt FAN), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			
			else if( alarmIdx == TEMPERATUREALARMID ){
				if( ( OLTMgmt.AlarmStatus & ( 1 << TEMPERATUREALARM_BIT ) ) == ALARMEXIST ){
					if( OLTMgmt.environment.UpLinkTemperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt Uplink), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWR1Temperature>=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU1), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWU2Temperature >=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU2), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PWU3Temperature >=  OLTMgmt.PWUtemperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PWU3), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.SelfTemperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt SW1), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON1Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON1), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON2Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON2), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON3Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON3), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					if( OLTMgmt.environment.PON4Temperature >=  OLTMgmt.temperatureThreshold_gen )
						AlarmDispatch( Alarmsource(olt PON4), Len, TrapId(消失), AlarmInfoTable[alarmIdx].AlarmClass, (char *)  AlarmInfoTable[alarmIdx].AlarmDesc);
					}
				semGive(OltMgmtDataSemId);
				return ( ROK );
				}
			
			}
		else if( alarmIdx in the PON alarm ){
			}
		
		else if( alarmIdx in the ONU alarm ){
			}
		
		semGive(OltMgmtDataSemId);
		return ( ROK );
		}
	
	else{
		sys_console_printf(" the alarm Mask %d is not correct ( file %s SetAlarmMask() )\r\n", alarmMask, __FILE__ );

		VOS_SemGive(OltMgmtDataSemId);
		return( RERROR );
		}
#endif

}

/**************************************************************
 *
 *    Function: GetAlarmMask( unsigned int alarmIdx)
 *
 *    Param:    unsigned int alarmIdx -- Alarm Index, used to identify the specific Alarm
 *
 *    Desc:  get the alarm Mask flag 
 *
 *    Return:   the alarm Mask flag, or RERROR
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
unsigned long  GetAlarmMask ( unsigned int alarmIdx )
{
	if( alarmIdx >  ALLALARMNUM ){
		/*sys_console_printf(" the alarm Id %d is not exist  ( GetAlarmMask() )\r\n", alarmIdx );*/
		return( RERROR );
		}

	return (int)( AlarmInfoTable[alarmIdx].AlarmMask );
}


/**************************************************************
 *
 *    Function: GetOltEnvironment( Environment_S *CurEnvirnoment )
 *
 *    Param:    
 *           out    Environment_S *CurEnvirnoment -- point to the data struct of the OLT working environment
 *
 *    Desc:  get the alarm Mask flag 
 *
 *    Return: ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 int  GetOltEnvironment( OltEnvironment_S *CurEnvirnoment ) 
{
	if( CurEnvirnoment == NULL ){
		/*sys_console_printf("error: the paramter pointer is null( GetOltEnvironment()) \r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
 	VOS_MemCpy( CurEnvirnoment, (OltEnvironment_S *)&(OLTMgmt.environment), sizeof(OltEnvironment_S) );
	return ( ROK );
}

/**************************************************************
 *
 *    Function: RecordOltCurrentTemperature( int whichCard, unsigned char value )
 *
 *    Param:  int whichCard -- the specific Cardslot 
 *             	   unsigned char value -- the current  temperature value measured 
 *
 *    Desc:  record the current temperature value measured
 *
 *    Return: ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  RecordOltCurrentTemperature( int whichCard, unsigned char  value )
{

	/*unsigned char *CardslotPtr;*/
#if 0
	if(( whichCard < UPLINK0) ||( whichCard > PWU3 )){
		sys_console_printf(" the Card Slot %d is not correct ( RecordOltCurrentTemperature() )\r\n", whichCard );	
		return( RERROR );
		}

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( RecordOltCurrentTemperature() )\r\n" );
		return( RERROR );
		}
#endif

#if 0
	switch( whichCard ){
		case UPLINK0:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.UpLink0Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x100 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt Uplink0 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x100;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.UpLink0Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x100 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt Uplink0 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfeff;
					}
				}
			
			break;

		case UPLINK1:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.UpLink1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x200 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt Uplink1 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x200;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.UpLink1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x200 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt Uplink1 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfdff;
					}
				}
			break;

		case SW1:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.SW1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x400 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt SW1 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x400;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.SW1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x400 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt SW1 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfbff;
					}
				}
			break;

		case PON5:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.SW2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x800 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt SW2 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x800;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.SW2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x800 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt SW2 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xf7ff;
					}
				}
			break;

		case PON4:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PON4Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x1000 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON4 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x1000;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PON4Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x1000 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON4 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xefff;
					}
				}
			break;

		case PON3:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PON3Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x2000 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON3 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x2000;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PON3Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x2000 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON3 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xdfff;
					}
				}
			break;

		case PON2:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PON2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x4000 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON2 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x4000;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PON2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x4000 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON2 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xbfff;
					}
				}
			break;

		case PON1:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PON1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x8000 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON1 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x8000;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PON1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x8000 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PON1 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0x7fff;
					}
				}
			break;

		case PWU1:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PWU1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x0010 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU1 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0010;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PWU1Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x0010 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU1 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xFfef;
					}
				}
			break;

		case PWU2:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PWU2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x0020 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU2 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0020;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PWU2Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x2000 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU2 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xffdf;
					}
				}
			break;

		case PWU3:
			if( value >= OLTMgmt.temperatureThreshold_gen ){
				OLTMgmt.environment.PWU3Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x0040 ) == 0 ){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU3 card ), 
							unsigned int Len, 
							unsigned int TrapId(产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0040;
					}
				}
			
			else
			if( value < OLTMgmt.temperatureThreshold_dis ) {
				OLTMgmt.environment.PWU3Temperature= value;
				if(( OLTMgmt.AlarmStatus & 0x0040 ) == 1){
					if( card temperature alarm mask is open ){
						AlarmDispatch ( AlarmSource( olt PWU3 card ), 
							unsigned int Len, 
							unsigned int TrapId(消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xffbf;
					}
				}
			break;			
		}
#endif
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
}


/**************************************************************
 *
 *    Function: RecordOltPWUValue( int whichPWUCard, unsigned char  vlaue )
 *
 *    Param:  int whichPWUCard -- the specific PWU Cardslot 
 *             	   unsigned char value -- the current  power value measured 
 *
 *    Desc:  record the current power value measured
 *
 *    Return: ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 int  RecordOltPWUValue( int whichPWUCard, unsigned char  value )
{

/*	unsigned char LastValue;*/
#if 0
	if(( whichPWUCard < PWU1 ) ||( whichPWUCard > PWU3 )){
		sys_console_printf(" the PWU Card Slot %d is not correct (RecordOltPWUValue() )\r\n", whichPWUCard );	
		return( RERROR );
		}

	if(( value != PWR_NORMAL)||(value != PWR_LOW)||(value != PWR_HIGH )||( value != PWR_NOTPRESENT)){
		sys_console_printf(" the power value %d is not corrent ( RecordOltPWUValue() )\r\n", value );
		return ( RERROR );
		}

	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( RecordOltPWUValue() )\r\n" );
		return( RERROR );
		}
#endif
#if 0
	/*whichPWUCard = whichPWUCard - PWU1; */
	switch(whichPWUCard){
		case PWU1:
			if( OLTMgmt.environment.CurrentPwu1 != value ) {
				if( value == PWR_NORAML ){
					if(PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT first pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu1 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfffe;
					}
				else{
					if( PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT first pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu1 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
					
						AlarmDispatch ( AlarmSource( OLT first pwu ), 
							unsigned int Len, 
							unsigned int TrapId(value 产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0001;
					}
				}
						
			break;
			
		case PWU2:
			if( OLTMgmt.environment.CurrentPwu2 != value ) {
				if( value == PWR_NORAML ){
					if(PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT second pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu2 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfffd;
					}
				else{
					if( PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT second pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu2 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
					
						AlarmDispatch ( AlarmSource( OLT second pwu ), 
							unsigned int Len, 
							unsigned int TrapId(value 产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0002;
					}
				}
			
			break;
			
		case PWU3:
			if( OLTMgmt.environment.CurrentPwu3 != value ) {
				if( value == PWR_NORAML ){
					if(PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT third pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu3 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & 0xfffb;
					}
				else{
					if( PWU alarm is open ){
						AlarmDispatch ( AlarmSource( OLT third pwu ), 
							unsigned int Len, 
							unsigned int TrapId( OLTMgmt.environment.CurrentPwu3 消失), 
							unsigned int AlarmClass, 
							char *AlarmDec);
					
						AlarmDispatch ( AlarmSource( OLT third pwu ), 
							unsigned int Len, 
							unsigned int TrapId(value 产生), 
							unsigned int AlarmClass, 
							char *AlarmDec);
						}
					OLTMgmt.environment.CurrentPwu1 = value;
					OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus | 0x0004;
					}
				}
			break;

		default: 
			return( ROK );
		}
#endif
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
	
}

/**************************************************************
 *
 *    Function: RecordOltFanStatus( unsigned char  Curstatus  )
 *
 *    Param:  unsigned char Curstatus -- the FAN current working status  
 *
 *    Desc:  record the current FAN working status
 *
 *    Return: ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  RecordOltFanStatus( unsigned char  CurStatus  )
{
	if( (CurStatus != FAN_ABNORAML ) ||( CurStatus != FAN_NORMAL )){
		/*sys_console_printf(" the FAN Card Slot is not correct ( RecordOltFanStatus() )\r\n" );	*/
		VOS_ASSERT(0);
		return( RERROR );
		}

#if 0
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( RecordOltFanStatus() )\r\n" );
		return( RERROR );
		}
#endif
#if 0
	if(fan alarm mask is open ){ /* fan alarm is open */
		
		if( CurStatus  == FAN_NORMAL ){ /* fan normal currently*/
			/* check whether fan is abnormal last time */
			if((OLTMgmt.environment.CurrentPan  == FAN_ABNORAML ) ||((OLTMgmt.AlarmStatus |(1<<FANALARM_BIT)) !=0 ))
				AlarmDispatch ( AlarmSource( OLT fan ), 
				unsigned int Len, 
				unsigned int TrapId(消失), 
				unsigned int AlarmClass, 
				char *AlarmDec);
			OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus & (WORD_FF  - ( 1 << FANALARM_BIT ) );
			}
	
		else 
		if( CurStatus == FAN_ABNORAML ){ /* fan abnormal currently */
			/* check whether fan is normal last time */
			if(( OLTMgmt.environment.CurrentPan == FAN_NORMAL ) ||((OLTMgmt.AlarmStatus |(1<<FANALARM_BIT)) ==0 ))
				AlarmDispatch ( AlarmSource(olt fan ), 
				unsigned int Len, 
				unsigned int TrapId(产生), 
				unsigned int AlarmClass, 
				char *AlarmDec);
			OLTMgmt.AlarmStatus = OLTMgmt.AlarmStatus |( 1 << FANALARM_BIT ) ;
			}
		}
	
	OLTMgmt.environment.CurrentPan = CurStatus;
#endif
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif
	return( ROK );
}


/**************************************************************
 *
 *    Function: GetOltCurrentAlarmRank()
 *
 *    Param:  none
 *
 *    Desc:  get the olt current highly class alarm
 *
 *    Return: highly class alarm
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
AlarmClass  GetOltCurrentAlarmRank()
{
	unsigned long  CurrentAlarm;
       AlarmClass AlarmRank = CLEAR;
	   
	CurrentAlarm = OLTMgmt.AlarmStatus;
	
	if( CurrentAlarm == 0 ) return (AlarmRank);
	
	if(( CurrentAlarm &  0x0007 ) != 0 ){  /* pwu output has alarm existing */
		AlarmRank =  MAJOR;
		}

	else if(( CurrentAlarm &  0x0008 ) != 0 ) { /* the fan has alarm existing */
		AlarmRank =  MAJOR;
		}
		
	else if(( CurrentAlarm & 0x0070 ) != 0 ) { /*  the pwu has temperature alarm existing */
		AlarmRank =  MINOR;
		}

	else if(( CurrentAlarm & 0xff00 ) != 0 ){ /* the other card has temperature alarm existing */
		AlarmRank =  WARNING;
		}

	return (AlarmRank);

}
#endif

int GetOltCurrenAlarmStatus()
{
	return( OLTMgmt.AlarmStatus );
}


/**************************************************************
 *
 *    Function: GetOltCardslotInserted()
 *
 *    Param:  none
 *
 *    Desc:  
 *
 *    Return: current Inserted  card flag 
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 unsigned short int  GetPonSlotInsertedAll( )
{
	return OLTMgmt.InsertedPonCard;
}

int GetOltCardslotInserted( int CardIndex)
{
	if(PonCardSlotRangeCheckByVty(CardIndex, NULL) != ROK )
		return(RERROR);
	
	if( ( OLTMgmt.InsertedPonCard & (1 << CardIndex) ) > 0 ){ return CARDINSERT;}
	
	return( CARDNOTINSERT );
}

int SetOltCardslotInserted( int CardIndex)
{	
	if(PonCardSlotRangeCheckByVty(CardIndex, NULL) != ROK )
		return(RERROR);

#if 0	
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltCardslotInserted() )\r\n" );
		return ( RERROR );
		}
#endif

	OLTMgmt.InsertedPonCard |= (1 << CardIndex);
	OLTMgmt.InsertedPonCardNum++;
    
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif

	return ( ROK );	
}

int SetOltCardslotPulled( int CardIndex)
{
	if(PonCardSlotRangeCheckByVty(CardIndex, NULL) != ROK )
		return(RERROR);
    
#if 0	
	if( VOS_SemTake(OltMgmtDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOltCardslotPulled() )\r\n" );
		return ( RERROR );
		}
#endif

	OLTMgmt.InsertedPonCard &= ~( 1 << CardIndex );
	if(OLTMgmt.InsertedPonCardNum > 0 )  OLTMgmt.InsertedPonCardNum--;
    
#if 0
	VOS_SemGive(OltMgmtDataSemId);
#endif

	return ( ROK );	
}

/**************************************************************
 *
 *    Function: V2r1TimerStart()
 *
 *    Param:  none
 *
 *    Desc:  
 *
 *    Return: current Inserted  card flag 
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
/*
int dogflag = 0;
int WatchdogPon = 4;
*/
int Emac1ReInitCtrlChannelFlag = 0;

extern void ReInitCtrlChannel(void);
extern void CheckPonSFPTypeValid();
void (*pon_enable_check_callback)() = NULL;

int V2r1TimerHandler()
{
	static ULONG  V2R1_TimeCounter = 0;
	PON_device_versions_t ver;/*by jinhl@2016.08.12*/

	/*
	int ret=0 ;
	unsigned long cardslot;
	short int PonPortIdx;
	static short int value=0;
	
	ScanPonPortMgmtTableTimer();*/
	V2R1_TimeCounter ++;

	if( Emac1ReInitCtrlChannelFlag == 0xaa55 )
	{
		cpuCtrlChan_EventReport( other_ctrlchan_fail );
		ReInitCtrlChannel();
		Emac1ReInitCtrlChannelFlag = 0;
		cpuCtrlChan_EventReport( other_ctrlchan_success );
	}
	
	ScanPonPortMgmtTableBerFer();
	ScanOnuMgmtTableTimer( V2R1_TimeCounter );
	ScanOnuMgmtTableBerFer();
	
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
    if ( V2R1_ENABLE == pon_swap_switch_enable )
    {
    	ScanPonPortHotSwap();
    }
#endif

	ScanOnuMgmtTableBurningFlash();

	ScanOnuPowerOffCounter();

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
    if ( PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE )
    {
    	OnuSecondMsgToTdmMgmt();
    }
#endif
	/*if((V2R1_TimeCounter %2) == 0)*/
	if( (V2R1_TimeCounter & 0x03) == 0 )	/* modified by xieshl 20091104, 改为4个延时周期 */
	{
		ScanAuthenticationPendingOnuAll();

		if( (V2R1_TimeCounter & 0x1f) == 0 )	/* modified by xieshl 20100104, 改为32个延时周期, 同时光功率检测和光模块类型检测功能分开 */
		{
		    /*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#if defined(_EPON_10G_PMC_SUPPORT_)            
		    if (!SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
#endif
	    	{
	    		if (!SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON /*&& !SYS_LOCAL_MODULE_TYPE_IS_8000_GPON*/)/*GPON板卡不能检查光模块类型changed by yanjy*/
				CheckPonSFPTypeValid();
	    	}

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
		/*	CheckPonSFPSupportRSSI();*/
#endif
		}
		/* added by xieshl 20151022, 现场问题，定时检查BCM PON芯片使能，不正常时复位芯片 */
		if( pon_enable_check_callback )
		{
			(*pon_enable_check_callback)();
		}
	}

#if 0
( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	OltPonPowerMetering();
#endif

#if 0	
	if( SYS_INIT_COMP == TRUE )
		{
	

	
		if( dogflag == 1 )
			{
			for( PonPortIdx = 0; PonPortIdx < MAXPONCHIP; PonPortIdx ++ )
				{
				if( value == 0 ) { value = 1; }
				else if(value == 1 ){ value = 0 ; }
				if( PonPortIsWorking( PonPortIdx) == TRUE)
					V2r1PonWatchdog( PonPortIdx, value );
				}
			}
		}
#endif	

	
	return( ROK );
}

extern int gpon_isDevReady();

extern void gponOltAdp_DevDisConnectNotify();
extern int gponOltAdp_GetStatusForKeepAlive();

ULONG V2R1_FailMaxTimes = 6;
int dbgKP = 1;
ULONG KPTimeCounter = 0;
int KPTimeCounterGpon = 0;

void V2r1TimerHandlerForKP()
{
	KPTimeCounter++;
	if(dbgKP)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		{
			if(gpon_isDevReady())/*pon芯片已经加载完成*/
			{
				if(0 == KPTimeCounter%180)/*1分钟检查一次*/
				{
					extern void PalDrvClearCtrlArrayAll();
					PalDrvClearCtrlArrayAll();
					
				}
				
				#if 1
				if(0 == KPTimeCounter%60)/*1分钟检查一次*/
				{
					if(VOS_OK != gponOltAdp_GetStatusForKeepAlive())/*获取不到pon芯片状态，则次数++*/
					{
						KPTimeCounterGpon++;
					}
					else
					{
						KPTimeCounterGpon = 0;
					}
					if(KPTimeCounterGpon >= V2R1_FailMaxTimes)
					{
						KPTimeCounterGpon = 0;
						gponOltAdp_DevDisConnectNotify();
						return;
					
					}
				}
				
				#endif
				#if 0/*34761,当pon芯片失联时，此段代码会耗时超长约30分钟以上by jinhl@2017.04.28*/
				if(0 == KPTimeCounter%120)/*2分钟判断一下*/
				{
					if(0 != KPTimeCounterGpon)
						return;
					extern void PonOnuNetworkMibActivePendingProcess();
					if(debugSnExp)
					PonOnuNetworkMibActivePendingProcess();	
				}
				#endif
			}
			else
			{
				KPTimeCounterGpon = 0;
			}
		}
	}
}

unsigned int timer_counter = VOS_TICK_SECOND * 60 *3;

int V2r1TimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_V2R1_TIMEOUT, 0, 0};
	
	/* sleep 6 minutes 
	if( timer_counter > 0 )
		{
		timer_counter -= VOS_TICK_SECOND;
		return( RERROR );
		}
	*/


	if(VOS_QueNum(g_Olt_KPALIVE_Queue_Id) <= 10)
	{
		if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
		{
			aulMsg[0] = MODULE_OLT;
			if( VOS_QueSend( g_Olt_KPALIVE_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
			{
				VOS_ASSERT(0);
				/*sys_console_printf("  error: VOS send 1second timer-out to olt-Queue message err\r\n"  );*/
			}
		}
	}
	
	
	
#if( EPON_MODULE_ONU_REG_FILTER	 == EPON_MODULE_YES )
	if(VOS_QueNum(g_Onu_Queue_Id) > 10) return(ROK);
	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
	{
		aulMsg[0] = MODULE_ONU;
		if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("  error: VOS send 1second timer-out to olt-Queue message err\r\n"  );*/
		}
	}
#endif

	if(VOS_QueNum(g_Olt_Queue_Id) > 10) return(ROK);
	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
	{
		aulMsg[0] = MODULE_OLT;
		if( VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("  error: VOS send 1second timer-out to olt-Queue message err\r\n"  );*/
		}
	}
	
    /* B--added by liwei056@2010-7-20 for Flash-vfs support */
    {
        extern int PON_LoadRescClearFlag;

        if (0 != PON_LoadRescClearFlag)
        {
         	if(VOS_QueNum(g_Pon_Queue_Id) > 10) return(ROK);
        	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
        	{
        		if( VOS_QueSend( g_Pon_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
        		{
        			VOS_ASSERT(0);
        			/*sys_console_printf("  error: VOS send 1second timer-out to olt-Queue message err\r\n"  );*/
        		}
        	}
        }
    }
    /* E--added by liwei056@2010-7-20 for Flash-vfs support */
   
	return( ROK);
}

int V2r1TimerStart()
{
	timer_counter = VOS_TICK_SECOND * 60 *3;
	
	
	V2r1TimerId = VOS_TimerCreate( MODULE_PON, (unsigned int )0/*g_Olt_Queue_Id*/, V2R1_TIMERTICK, (void *)V2r1TimerCallback, (void *)NULL/*&PonPortIdx*/, VOS_TIMER_LOOP );
	if( V2r1TimerId == VOS_ERROR ){
		sys_console_printf("\r\nstart product timer err \r\n ");
		return( RERROR );
		}
	else{
		/*sys_console_printf("\r\nstart product timer OK \r\n ");*/
		return( V2r1TimerId );
		}
}

/* added by xieshl 20111019, 主控和PON板间：ONU状态定时同步 */
LONG onuStatusSyncTimerId = 0;
void OnuStatusSyncTimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ONU_STATUS_SYNC_TIMEOUT, 0, 0};

	if( /*SYS_LOCAL_MODULE_ISMASTERACTIVE*/SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (VOS_QueNum(g_Onu_Queue_Id) <= 3) )
	{
		if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
			VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  );
		}
	}
}
int OnuStatusSyncTimerStart()
{
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 /*|| SYS_PRODUCT_TYPE == PRODUCT_E_EPON3*/ )	/* modified by xieshl 20111129, 6100上不需要启动主控和PON板间ONU状态同步 */
		return VOS_OK;
	if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;
	onuStatusSyncTimerId = VOS_TimerCreate( MODULE_ONU, 0, 20000, (void *)OnuStatusSyncTimerCallback, NULL, VOS_TIMER_LOOP );
	return onuStatusSyncTimerId;
}


/* added by xieshl 20110520, 自动删除不在线ONU PR11109 */
extern ULONG g_onu_agingtime;
extern const ULONG g_onu_agingtime_default;
VOID onuAgingTimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_ONU_AGINGTIME, 0, 0};

	if( g_onu_agingtime == 0 )
		return;
	
	if( SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) && ( VOS_QueNum(g_Olt_Queue_Id) < 3 ) )
	{
		if( VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
		}
	}
}

int onuAgingTimerStart()
{
	g_onu_agingtime = g_onu_agingtime_default;
	onuAgingTimerId = VOS_TimerCreate( MODULE_OLT, 0, MINUTE_5, onuAgingTimerCallback, NULL, VOS_TIMER_LOOP );
	return onuAgingTimerId;
}

VOID onuupdateTimerCallback()
{
	ULONG deviceid=0;
	if( (deviceid = findFirstWaitingOnu()) == 0 )
	{
		OLT_GPON_UPDATE_DEBUG("no waiting onu \r\n");
    	return VOS_OK;
	}
	OLT_GPON_UPDATE_DEBUG("SendActiveGponUpdateWiatingMsg deviceid = %d\r\n",deviceid);
	SendActiveGponUpdateWiatingMsg(deviceid);
}
int onuupdateTimerStart()
{
	g_onu_updatetime = MINUTE_1;
	onuupdateTimerId = VOS_TimerCreate( MODULE_OLT, 0, g_onu_updatetime, onuupdateTimerCallback, NULL, VOS_TIMER_LOOP );
	return onuupdateTimerId;
}

#if 0
int V2r1TimerStop()
{
	long ret;

	ret = VOS_TimerDelete( MODULE_PON, V2r1TimerId);
	if( ret == VOS_ERROR ){
		sys_console_printf("\r\nstop product timer err\r\n ");
		return( RERROR );
		}
	else{
		sys_console_printf("\r\nstop product timer OK\r\n ");
		return( ROK );
		}
	
}

int V2r1TimerHavePassed()
{
	unsigned long PassedTime;
	long ret;

	ret = VOS_TimerHavePassedTime( MODULE_PON, V2r1TimerId, &PassedTime);

	if( ret == VOS_ERROR ){
		sys_console_printf("\r\nGet product timer Have passed length err \r\n ");
		return( RERROR );
		}
	else{
		sys_console_printf("\r\nproduct timer have passed %d  \r\n ", PassedTime);
		return( ROK);
		}

}
#endif
void  UpdatePonPortNum()
{
	int i;
	int counter = 0;;
	
	for(i=0; i< MAXPONCHIP;i++){

		if(( PonPortTable[i].PortWorkingStatus == PONPORT_UP ) ||( PonPortTable[i].PortWorkingStatus == PONPORT_LOOP ) 
			|| ( PonPortTable[i].PortWorkingStatus == PONPORT_UPDATE ))
		/*
		if(( PonChipMgmtTable[i].operStatus == PONCHIP_UP)||( PonChipMgmtTable[i].operStatus == PONCHIP_ONLINE )
			||( PonChipMgmtTable[i].operStatus == PONCHIP_TESTING ) ||( PonChipMgmtTable[i].operStatus == PONCHIP_INIT)
			|| ( PonChipMgmtTable[i].operStatus == PONCHIP_DORMANT )) */
			counter ++;			
		}
	OLTMgmt.CurrentPonPort = counter;

}
#if 0
int ShowOltInfo()
{
	unsigned char  Info[MAXVENDORINFOLEN+1];
	int len, i,ret;

	sys_console_printf("\r\n  The OLT device information : \r\n");
	
	{
		sys_console_printf("   Vendor Info: ");
		ret = GetOltVendorInfo( Info, &len);
			
		if( ret == ROK )
			{
			unsigned char  temp[51];
			i = 0;
			Info[len] = '\0';	
			while( len > 0 )
				{
				if ( len > 30 )
					{
					VOS_MemCpy( temp, &Info[i], 50) ;
					temp[50] ='\0';
					i+=50;
					len-=50;
					}
				else 
					{
					VOS_MemCpy( temp, &Info[i], len) ;
					temp[len] ='\0';
					len = 0;
					}
				sys_console_printf("\r\n         %s", temp );
				}
			}
		else{
			sys_console_printf(" Get Info err \r\n");
			}
		sys_console_printf("\r\n");
	}
/*
	{
		ret = GetOltVendorLocation(Info, &len);
		if( ret == ROK )
			{
			Info[len] = '\0';
			sys_console_printf("   Vendor address:    %s \r\n", Info );
			}
	}

	{
		ret = GetOltVendorContact(Info, &len);
		if( ret == ROK )
			{
			Info[len] = '\0';
			sys_console_printf("   Vendor contact:    %s \r\n", Info );
			}
	}
*/
	{
		ret = GetOltLocation( Info, &len );
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   location: %s \r\n", Info );
			}
	}

	{
		ret = GetOltDeviceName( Info, &len );
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   device Name: %s \r\n", Info );
			}
	}
		  
	{
		int type;
		GetOltType( &type );
		if( type == V2R1_OLT_GFA6700 ) sys_console_printf("   device type: %s\r\n", GetDeviceTypeString(type));
		else { 
			sys_console_printf("   the device type err \r\n");

			}
	}

	{
		ret = GetOltDeviceDesc(Info, &len);
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   device desc: %s \r\n", Info );
			}
	}

	{
		ret = GetOltFWVersion( Info, &len);
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   firmware version: %s \r\n", Info );
			}
	}

	{
		ret = GetOltSWVersion( Info, &len);
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   application version: %s \r\n", Info );
			}
	}

	{
		ret = GetOltHWVersion( Info, &len);
		if( ret == ROK ){
			Info[len] = '\0';
			sys_console_printf("   hardware version: %s \r\n", Info );
			}
	}

	{ 
		ret = GetOltAdminStatus();
		(ret == V2R1_ENABLE ) ? sys_console_printf("   admin status:  up \r\n") : sys_console_printf("   admin status:  down\r\n");
	}

	{
		ret = GetOltOperStatus();
		(ret == V2R1_UP) ? sys_console_printf("   operation status: up \r\n" ) : sys_console_printf("   operation status:  down\r\n");
	}

	{
		ret = GetOltCurrenAlarmStatus();
		sys_console_printf("   current alarm: %d \r\n", ret );
	}

/*
	{
		unsigned char MacAddr[50];
		ret = GetOltMgmtMAC(Info, &len );
		for(i=0;i<len;i++)
			sprintf (&MacAddr[5*i], "0x%02x ",Info[i]);
		sys_console_printf("   Mac addr : %s \r\n", MacAddr );		
	}
*/

	{
		sys_console_printf("   Mac addr : ");
		ret = GetOltMgmtMAC(Info, &len );
		if ( ret != RERROR )
		for(i=0;i<( BYTES_IN_MAC_ADDRESS); i++ )
			{
			sys_console_printf("%02x%02x", Info[i], Info[i+1] );
			i++;
			if( i != 5 )sys_console_printf(".");
			}
		sys_console_printf("\r\n");
	}
	

	{
		
		sys_console_printf("   Pon Card Inserted  : %d \r\n", OLTMgmt.InsertedPonCardNum);
		sys_console_printf("          %s",CardSlot_s[PON1] );
		if( GetOltCardslotInserted (PON1 ) == CARDINSERT ) sys_console_printf(" inserted \r\n");
		else sys_console_printf(" not inserted \r\n" );
		sys_console_printf("          %s",CardSlot_s[PON2] );
		if( GetOltCardslotInserted (PON2 ) == CARDINSERT ) sys_console_printf(" inserted \r\n");
		else sys_console_printf(" not inserted \r\n" );
		sys_console_printf("          %s",CardSlot_s[PON3] );
		if( GetOltCardslotInserted (PON3 ) == CARDINSERT ) sys_console_printf(" inserted \r\n");
		else sys_console_printf(" not inserted \r\n" );
		sys_console_printf("          %s",CardSlot_s[PON4] );
		if( GetOltCardslotInserted (PON4 ) == CARDINSERT ) sys_console_printf(" inserted \r\n");
		else sys_console_printf(" not inserted \r\n" );
		sys_console_printf("          %s",CardSlot_s[PON5] );
		if( GetOltCardslotInserted (PON5 ) == CARDINSERT ) sys_console_printf(" inserted \r\n");
		else sys_console_printf(" not inserted \r\n" );
		sys_console_printf("   Pon Port MAX : %d \r\n",  OLTMgmt.MaxPonPort );
		sys_console_printf("   Pon Port current: %d \r\n", OLTMgmt.CurrentPonPort );
		
	}

	{
		Date_S  SysUpDate;
		GetSysLaunchTime( &SysUpDate);
		sys_console_printf("   System Up Time: \r\n");
		sys_console_printf("       [MM-DD-YY] : %02d-%02d-%04d\r\n", SysUpDate.month, SysUpDate.day, SysUpDate.year );
		sys_console_printf("       [HH-MM-SS:MS] : %d-%d-%d:%d \r\n", SysUpDate.hour, SysUpDate.minute, SysUpDate.second);
	}

	sys_console_printf("------  olt info end ------- \r\n\r\n");

	return( ROK );
		
}
#endif

/* add by chenfj 2006/09/19 */
/* #2604 问题telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了*/
int  ShowPonCardInfo( struct vty *vty)
{
	int CardIndex;
	short int FirstPonPortIdx, PonPortIdx;
	short int PonChipType = 0;
	int count;
	short int i;

	for(CardIndex=PONCARD_FIRST; CardIndex<=PONCARD_LAST; CardIndex++)
	{
		if( SlotCardIsPonBoard(CardIndex) != ROK ) continue;
		
		count = 0;
		for(i=1; i<=PONPORTPERCARD; i++)
		{
			PonPortIdx = GetPonPortIdxBySlot( (short int)CardIndex, (short int)i );
			/*if(PonPortIsWorking(PonPortIdx) == TRUE)*/
			if(OLTAdv_IsExist(PonPortIdx) == TRUE)
			/*if(getPonChipInserted( (unsigned char)CardIndex, (unsigned char)i) == PONCHIP_EXIST )*/
			{
				count ++;
				if(count == 1 )
				{
					PonChipType = V2R1_GetPonchipType(PonPortIdx);
				}
			}
		}
        
		if(count != 0 )
		{
			vty_out( vty, "     %s inserted,type:",CardSlot_s[CardIndex] );
			vty_out(vty, "%s,%d*%s\r\n", typesdb_module_type2name(__SYS_MODULE_TYPE__(CardIndex)), (count/4)==0? 1:count/4, pon_chip_type2name(PonChipType));
		}
    	else
        {
            continue;
        }         
			
		vty_out( vty, "         inserted PonPort BitMap(1-Up, 0-Down):");
        
		FirstPonPortIdx = GetPonPortIdxBySlot(CardIndex, FIRSTPONPORTPERCARD);
		for(PonPortIdx=FirstPonPortIdx; PonPortIdx<(FirstPonPortIdx+PONPORTPERCARD);PonPortIdx++)
		{
			/*if( PonPortIsWorking(PonPortIdx) ) {vty_out(vty, "1");}*/
			if(OLTAdv_IsExist(PonPortIdx) == TRUE) 
			{
			    vty_out(vty, "1");
            }
			else
			{
			    vty_out(vty, "0");
            }
		}

        vty_out(vty, "\r\n");
	}

    return( ROK );
}

int  ShowOltInfoByVty(struct vty *vty)
{
	extern char * app_creationDate;	/* added by xieshl 20091023, 区分产品软件build日期 */
	unsigned char  Info[MAXVENDORINFOLEN+1];
	/* modified by xieshl 20101227, 问题单11775，函数优化 */
	int len = 0, ret;
	
	char *pDispStr;
	int type;

	vty_out( vty, "   Vendor Info:\r\n");
	ret = GetOltVendorInfo( Info, &len);
	if(len > MAXVENDORINFOLEN) return(CMD_WARNING);
	Info[len] = '\0';
	vty_out( vty, "%s\r\n", Info );
	vty_out(vty,"   Vendor location:%s\r\n", typesdb_product_olt_vendor_location());
		  
	type = GetOltType();
	vty_out(vty,"   product type:%s\r\n",GetDeviceTypeString(type));
	vty_out(vty,"   product desc:%s\r\n",GetDeviceDescString(type));
	if(type == V2R1_OLT_GFA6100)
		pDispStr = OLT_DEVICE_6100_CHASSIS;
	else if(type == V2R1_OLT_GFA6700)
		pDispStr = OLT_DEVICE_6700_CHASSIS;
	else if(type == V2R1_OLT_GFA6900)
		pDispStr = OLT_DEVICE_6900_CHASSIS;
	else if(type == V2R1_OLT_GFA8000)
		pDispStr = OLT_DEVICE_8000_CHASSIS;
	else
		pDispStr = OLT_LOCATION_DEFAULT_default;
	vty_out( vty,"   product chassis:%s\r\n", pDispStr );

	ret = GetOltSWVersion( Info, &len);
	if( ret == ROK ){
		if(len > MAXSWVERSIONLEN ) return(CMD_WARNING);
		Info[len] = '\0';
		vty_out( vty, "   application version:%s(Build on %s)\r\n", Info, app_creationDate );
	}

	vty_out( vty, "   System MAC address:");
	ret = GetOltMgmtMAC(Info, &len );
	if ( ret != RERROR )
		vty_out( vty, "%02x%02x.%02x%02x.%02x%02x\r\n", Info[0], Info[1], Info[2], Info[3], Info[4], Info[5] );

	ret = GetOltDeviceName( Info, &len );
	if( ret == ROK ){
		if(len > MAXDEVICENAMELEN) return(CMD_WARNING);
		Info[len] = '\0';
		vty_out( vty, "   device name:%s \r\n", Info );
	}

	ret = GetOltDeviceDesc(Info, &len);
	if( ret == ROK ){
		if(len > MAXDEVICEDESCLEN) return(CMD_WARNING);
		Info[len] = '\0';
		vty_out( vty, "   device desc:%s\r\n", Info );
	}

	ret = GetOltLocation( Info, &len );
	if( ret == ROK ){
		if(len > MAXLOCATIONLEN) return(CMD_WARNING);
		Info[len] = '\0';
		vty_out(vty, "   device location:%s \r\n", Info );
	}

	vty_out( vty, "   Pon Card Inserted Counter:%d\r\n", OLTMgmt.InsertedPonCardNum);
	ShowPonCardInfo(vty);
	UpdatePonPortNum();
	vty_out( vty, "     MAX PonPort Num:%d \r\n",  MAXPON/*OLTMgmt.MaxPonPort*/);
	vty_out( vty, "     inserted PonPort counter:%d \r\n", OLTMgmt.CurrentPonPort );
	
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	{
	unsigned long  slotIdx;
	unsigned long tdmType = 0;
	unsigned char j;
	slotIdx = get_gfa_tdm_slotno();
	/* TDMCardIdx */
	if(slotIdx != 0 )
		{
		for(j=0;j< TDM_FPGA_MAX; j++)
			{
			if(getTdmChipInserted ((slotIdx-1),j) == TDM_FPGA_EXIST )
				tdmType = getTdmChipType((slotIdx-1),j);
			break;
			}	
			
		vty_out(vty,"   %s(slot%d) Inserted, FPGA %s bitMap(1-Up,0-Down):",typesdb_module_type2name(__SYS_MODULE_TYPE__(slotIdx)),slotIdx, tdm_chip_type2name(tdmType));
		for(j=0;j< TDM_FPGA_MAX; j++)
			{
			if(TdmChipDownloadComp[j] == V2R1_STARTED) vty_out(vty,"1");
			else vty_out(vty,"0");
			/*
			if(getTdmChipInserted ((slotIdx-1),j) == 1 ) vty_out(vty,"1");
			else vty_out(vty,"0");
			*/
			}
		vty_out(vty,"\r\n");
		}
	}
#endif

/* 许永刚问题: 去测试设备的时候，有时候可能需要重启系统或者更换版本，但是不能让别人看出来，所以需要增加一条这样的命令。*/
#if 1   

	{
		Date_S  SysUpDate;
		GetSysLaunchTime( &SysUpDate);
		vty_out( vty, "   System Up Time:%04d-%02d-%02d,%02d:%02d:%02d\r\n", SysUpDate.year, SysUpDate.month, SysUpDate.day,
				SysUpDate.hour, SysUpDate.minute, SysUpDate.second );
	}

	{
		ULONG  SysUpDelayed;
		ULONG days, hours,minutes, seconds;
		
		SysUpDelayed = GetOltSysUpTime();	/*GetOltSysUpLastTime();*/	/* modified by xieshl 20110902, 问题单13410 */

		days = SysUpDelayed /V2R1_ONE_DAY;
		hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
		minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
		seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

		vty_out(vty,"   Running-time:%04u:%02u:%02u:%02u\r\n",days, hours, minutes, seconds );	/* modified by xieshl 20100319, GWD网上问题9966 */
	}
#endif
	vty_out( vty, "   downlink policer:%s\r\n", ((GetOnuPolicerFlag() == V2R1_DISABLE) ? "disable" : "enable") );

	{
		/* modified by xieshl 20101227, 问题单11775, 只要CTC协议栈标志CTC_StackSupported=1，即认为启动，
		变量CTC_StackSupported只在系统启动时初始化，在运行过程中不管主控eeprom如何设置，
		都不会修改到这个变量，因此CTC_StackSupported基本能反映所有PON板CTC协议栈是否启动，
		不需再到各PON板上查询 */
		/*bool  Ctc_Stack_Start = FALSE;
		if(( CTC_STACK_is_init(&Ctc_Stack_Start) == CTC_STACK_EXIT_OK ) && (Ctc_Stack_Start == TRUE))
			vty_out(vty, "   onu CTC stack is started\r\n");
		else vty_out( vty, "   onu CTC stack is not started\r\n");*/
		vty_out(vty, "   onu CTC stack is%s started\r\n", (V2R1_CTC_STACK) ? "" : " not" );
	}

	{
		unsigned long enable ;
		getOnuAuthEnable(0, 0, &enable);
		if( enable == V2R1_ONU_AUTHENTICATION_ENABLE)
			pDispStr = "enabled";			
		else /*if( enable == V2R1_ONU_AUTHENTICATION_DISABLE)*/
			pDispStr = "disabled";
		vty_out( vty, "   onu authentication is %s\r\n", pDispStr );
	}
	
	return( ROK );
		
}

int  RestoreToDefaultData()
{

	return( ROK );
}


#ifdef V2R1_PRODUCT_TRAP


/* TRAP  接口函数   */
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
extern int ClearOpticalPowerAlamWhenOnuOffline(short int PonPortIdx,short int OnuIdx);
extern int ReInitWhenNewOnuAddedOrOldOnuReregister(short int PonPortIdx,short int OnuIdx);
#endif
/* ONU注册事件上报*/

/* modified by xieshl 20110615, FirstFlag是从ONU表里取得，没有必要作为输入参数 */
 int Trap_OnuRegister( short int PonPortIdx, short int OnuIdx )
{
	short int FirstFlag;
	short int OnuEntry;
	int slot ;
	int port;
	unsigned long DevIdx;
	unsigned char PowerOn;
	
	CHECK_ONU_RANGE;

	OnuEntry = MAXONUPERPON * PonPortIdx + OnuIdx;

	ONU_MGMT_SEM_TAKE;

	if( OnuMgmtTable[OnuEntry].RegisterFlag == ONU_FIRST_REGISTER_FLAG )
	{
		/* send onu first registration trap to NMS */
		FirstFlag = ONU_FIRST_REGISTER_FLAG;
		OnuMgmtTable[OnuEntry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
		OnuMgmtTable[OnuEntry].UsedFlag = ONU_USED_FLAG;
	}
	else
	{
		FirstFlag = NOT_ONU_FIRST_REGISTER_FLAG;
	}
	PowerOn = OnuMgmtTable[OnuEntry].PowerOn;
	OnuMgmtTable[OnuEntry].RegisterTrapFlag = ONU_REGISTER_TRAP;
	
	ONU_MGMT_SEM_GIVE;

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	ReInitWhenNewOnuAddedOrOldOnuReregister( PonPortIdx, OnuIdx );
#endif
		
	if( PowerOn != V2R1_POWER_ON )
	{
		Trap_OnuPowerAlarm(PonPortIdx, OnuIdx, V2R1_POWER_ON );
		return( ROK );
	}

	/*if( (FirstFlag != NOT_ONU_FIRST_REGISTER_FLAG) &&( FirstFlag != ONU_FIRST_REGISTER_FLAG)) return( RERROR );*/

	slot  = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip( PonPortIdx );
	DevIdx = MAKEDEVID(slot, port, (OnuIdx+1) );

	/*onuExtMgmt_OnuRegCallback( DevIdx );*/	/* added by xieshl 20080821 */

	if(FirstFlag == ONU_FIRST_REGISTER_FLAG)
	{
		if( checkOnuEmapperParameter(PonPortIdx, OnuIdx, 1) == 0 )
			sys_console_printf( "\r\n %% onu emapper incorrectly, auto-adjust success\r\n" );

		if( EVENT_TRAP == V2R1_ENABLE )
			onuNewRegSuccess_EventReport(DevIdx );
	}
	else
	{
		if( EVENT_TRAP == V2R1_ENABLE )
			onuReregSuccess_EventReport(DevIdx );
	}

	return( ROK );

}

/* ONU离线事件上报*/
int  Trap_OnuDeregister( short int PonPortIdx, short int OnuIdx, unsigned long reason, int forcibly )
{
	short int OnuEntry;
	int slot ;
	int port;
	unsigned long DevIdx;
	unsigned char PowerOn, RegisterTrapFlag, PowerOffCounter = 0;
	
	CHECK_ONU_RANGE;

	if( !forcibly )
	{
		OnuEntry = MAXONUPERPON * PonPortIdx + OnuIdx;
		VOS_ASSERT( MAXONU > OnuEntry );

		ONU_MGMT_SEM_TAKE;
		PowerOn = OnuMgmtTable[OnuEntry].PowerOn;
		RegisterTrapFlag = OnuMgmtTable[OnuEntry].RegisterTrapFlag;
		PowerOffCounter = OnuMgmtTable[OnuEntry].PowerOffCounter;
		OnuMgmtTable[OnuEntry].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
		ONU_MGMT_SEM_GIVE;

		if( PowerOn == V2R1_POWER_OFF )
		{
			return( ROK );
		}
		if( RegisterTrapFlag != ONU_REGISTER_TRAP )
			return( RERROR );
	}
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	DevIdx = MAKEDEVID(slot, port, (OnuIdx+1));

	if( reason >= PON_ONU_DEREGISTRATION_LAST_CODE )
		reason = PON_ONU_DEREGISTRATION_UNKNOWN;
	if(EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf("\r\n onu %d/%d/%d deregister,reason %s\r\n", slot, port, (OnuIdx+1),PON_onu_deregistration_code_s[reason]);
	}
	if( EVENT_TRAP == V2R1_ENABLE )
	{
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
		ClearOpticalPowerAlamWhenOnuOffline( PonPortIdx, OnuIdx );
#endif	
		if( PowerOffCounter > 0 )
			Trap_OnuPowerAlarm(PonPortIdx, OnuIdx, V2R1_POWER_OFF );
		else
		{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
			/* begin: added by jianght 20090327  */
			e1OnuTakeOff(DevIdx);
			/* end: added by jianght 20090327 */
#endif
			check_onu_notpresent_callback(DevIdx);
			onuNotPresent_EventReport(DevIdx , reason);
		}
	}
	
	return( ROK );
} 

/* ONU 注册冲突事件*/
int Trap_OnuRegisterConflict(short int PonPortIdx, short int OnuIdx /*, short int PonPort, unsigned char *MacAddr*/)
{
	int slot ;
	int port;
	unsigned long DevIdx;
	
	CHECK_ONU_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	DevIdx = MAKEDEVID(slot,port,(OnuIdx+1))/*slot * 10000 + port * 1000 + (OnuIdx+1)*/;

	if( EVENT_TRAP  == V2R1_ENABLE )
		onuRegisterConflict_EventReport(DevIdx );

	return( ROK );
}

/* 功能:    PON固件加载事件上报*/
int Trap_FirmwareLoad(short int PonPortIdx, int Result )
{
	unsigned long DevIdx, BrdIdx, ponIdx;

	CHECK_PON_RANGE

	if( (Result != V2R1_LOAD_FILE_SUCCESS ) &&(Result != V2R1_LOAD_FILE_FAILURE)) return( RERROR );

	DevIdx = 1;
	BrdIdx = GetCardIdxByPonChip( PonPortIdx );
    ponIdx = GetPonPortByPonChip( PonPortIdx );

	if( EVENT_TRAP == V2R1_ENABLE )
	{
		if( Result == V2R1_LOAD_FILE_SUCCESS) 	
			return( firmwareLoadSuccess_EventReport( DevIdx, BrdIdx, ponIdx) );
		else
            return( firmwareLoadFailure_EventReport( DevIdx, BrdIdx, ponIdx) );
	}
    return( ROK );
}

/* 功能:    PON DBA 加载事件上报*/
int Trap_DBALoad(short int PonPortIdx, int Result )
{
	unsigned long DevIdx, BrdIdx, ponIdx;

	CHECK_PON_RANGE

	if( (Result != V2R1_LOAD_FILE_SUCCESS ) &&(Result != V2R1_LOAD_FILE_FAILURE)) return( RERROR );

	DevIdx = 1;
	BrdIdx = GetCardIdxByPonChip( PonPortIdx );
    ponIdx = GetPonPortByPonChip( PonPortIdx );

	if( EVENT_TRAP == V2R1_ENABLE )
	{
		if( Result == V2R1_LOAD_FILE_SUCCESS) 	
			return( dbaLoadSuccess_EventReport( DevIdx, BrdIdx, ponIdx) );
		else
            return( dbaLoadFailure_EventReport( DevIdx, BrdIdx, ponIdx) );
	}
	return( ROK );
}

#if 0
/* 功能:    通过OAM文件加载通道传输软件到ONU成功事件上报
   输入参数:devIdx－ONU设备索引，设备索引定义规则遵循MIB定义
   返回值:  成功－VOS_OK，错误－VOS_ERROR */

int Trap_OnuAppImageUpdate(short int PonPortIdx, short int OnuIdx, int Result )
{
	int slot ;
	int port;
	unsigned long DevIdx;
	
	CHECK_ONU_RANGE

	if( (Result != V2R1_LOAD_FILE_SUCCESS ) &&(Result != V2R1_LOAD_FILE_FAILURE)) return( RERROR );

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	DevIdx =MAKEDEVID(slot,port,(OnuIdx+1)) /*slot * 10000 + port * 1000 + (OnuIdx+1)*/;

	if( EVENT_TRAP == V2R1_ENABLE )
	{
		if( Result == V2R1_LOAD_FILE_SUCCESS) 	
			return( onuSoftwareLoadSuccess_EventReport( DevIdx));
		else
            return( onuSoftwareLoadFailure_EventReport( DevIdx ));
	}
	return( ROK );	
}
#endif

/* PON	端口异常事件上报*/
int Trap_PonPortAbnormal( short int PonPortIdx , int ResetCode )
{
	CHECK_PON_RANGE
        
	if(( ResetCode < PON_OLT_RESET_HOST_TIMEOUT ) || ( ResetCode > PON_OLT_RESET_OLT_NOT_INITED_RESPONSE )) return( RERROR );

	if( EVENT_TRAP == V2R1_ENABLE )
    {
    	if(PonPortTable[PonPortIdx].AbnormalFlag != V2R1_ENABLE)
    	{
        	unsigned long DevIdx, BrdIdx, ponIdx;
            
        	DevIdx = 1;
        	BrdIdx = GetCardIdxByPonChip( PonPortIdx );
            ponIdx = GetPonPortByPonChip( PonPortIdx );
        	
        	ponPortAbnormal_EventReport(DevIdx, BrdIdx, ponIdx, (long)ResetCode );

            PonPortTable[PonPortIdx].AbnormalFlag = V2R1_ENABLE;
    	}
    }   
    
	return( ROK );
}

/* B--added by liwei056@2011-11-25 for D13166 */
int Trap_PonPortNormal( short int PonPortIdx )
{
	CHECK_PON_RANGE

	if(PonPortTable[PonPortIdx].AbnormalFlag == V2R1_ENABLE)
    {
    	unsigned long DevIdx, BrdIdx, ponIdx;
        
    	DevIdx = 1;
    	BrdIdx = GetCardIdxByPonChip( PonPortIdx );
        ponIdx = GetPonPortByPonChip( PonPortIdx );

        ponPortAbnormalClear_EventReport(DevIdx, BrdIdx, ponIdx);
        PonPortTable[PonPortIdx].AbnormalFlag = V2R1_DISABLE;
    }   
    
	return( ROK );
}
/* E--added by liwei056@2011-11-25 for D13166 */

/* ONU 电源告警*/
int Trap_onuPowerOff(short int PonPortIdx, short int OnuIdx )
{
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].PowerOffCounter = V2R1_POWER_OFF_WAIT;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

extern int SetOnuPowerStatus( short int PonPortIdx, short int OnuIdx, LONG status );
int Trap_OnuPowerAlarm(short int PonPortIdx, short int OnuIdx, unsigned long  Power_OnOff)
{
	unsigned long DevIdx;
	int slot;
	int port;

	CHECK_ONU_RANGE;

	if(( Power_OnOff != V2R1_POWER_ON ) &&( Power_OnOff != V2R1_POWER_OFF )) return( RERROR );

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	DevIdx = MAKEDEVID( slot, port, (OnuIdx+1) );
	
	if( Power_OnOff == V2R1_POWER_ON )
	{
		/*onuExtMgmt_OnuRegCallback( DevIdx );*/	/* added by xieshl 20080821 */

		SetOnuPowerStatus( PonPortIdx, OnuIdx, V2R1_POWER_ON );

		if( EVENT_TRAP == V2R1_ENABLE )
			devPowerOn_EventReport( DevIdx );
	}
	else if( Power_OnOff == V2R1_POWER_OFF )
	{
		if( SetOnuPowerStatus( PonPortIdx, OnuIdx, V2R1_POWER_OFF ) == RERROR )
			return ROK;

		/* begin: added by jianght 20090327  */
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		e1OnuTakeOff(DevIdx);
#endif
		/* end: added by jianght 20090327 */
		check_onu_notpresent_callback(DevIdx);

		if( EVENT_TRAP == V2R1_ENABLE )
			devPowerOff_EventReport( DevIdx );
	}
	
	return( ROK );
}

#if 0
/* ONU 软件更新上报*/
int Trap_OnuSoftwareUpdate(short int PonPortIdx, short int OnuIdx, unsigned long Result )
{
	unsigned long DevIdx;
	int slot;
	short int port;
	
	CHECK_ONU_RANGE

	if( (Result != V2R1_LOAD_FILE_SUCCESS ) &&(Result != V2R1_LOAD_FILE_FAILURE)) return( RERROR );

	slot  = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip( PonPortIdx );
	DevIdx = MAKEDEVID(slot,port,(OnuIdx+1))/*slot * 10000 + port * 1000 + (OnuIdx+1)*/;

	if( EVENT_TRAP == V2R1_ENABLE )
		{
		if( Result == V2R1_LOAD_FILE_SUCCESS )
			onuSoftwareLoadSuccess_EventReport(DevIdx );
		else if(Result == V2R1_LOAD_FILE_FAILURE)
			onuSoftwareLoadFailure_EventReport(DevIdx );
		}

	return( ROK );
}
#endif

/* LLID带宽超限或不足事件上报*/
int Trap_LlidBandwidthExceeding(short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned long Result )
{
	unsigned long DevIdx;
	int slot;
	short int port;
	short int Llid;

	CHECK_ONU_RANGE

	if((Result != V2R1_EXCEED_RANGE ) &&(Result != V2R1_NOT_EXCEED_RANGE)) return( RERROR );
	
	Llid= GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	DevIdx = 1;
	/*DevIdx = (slot+1) * 10000 + (port+1) * 1000 + (OnuIdx+1);*/

	if( EVENT_TRAP == V2R1_ENABLE )
		{
		if( Result == V2R1_EXCEED_RANGE )
			{
			llidActBWExceeding_EventReport(DevIdx, slot, port, Llid );
			}
		else{
			llidActBWExceedingClear_EventReport(DevIdx, slot, port, Llid );
			}
		}
	
	return( ROK );
}

/*  PON保护倒换事件上报
  	 输入参数:devIdx－OLT设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
 	  返回值:  成功－VOS_OK，错误－VOS_ERROR
*/
int  Trap_PonPortProtectSwitch(short int PonPortIdx )	
{
	unsigned long devIdx, brdIdx, ponIdx;

	CHECK_PON_RANGE

	devIdx = 1;
	brdIdx = GetCardIdxByPonChip(PonPortIdx);
	ponIdx = GetPonPortByPonChip(PonPortIdx);

	if( EVENT_TRAP == V2R1_ENABLE )
		autoProtectSwitch_EventReport( devIdx, brdIdx,  ponIdx );

	return( ROK );
}

/* 功能:    PON误码率超门限事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			ber－误码率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */
int  Trap_PonPortBER(short int PonPortIdx, unsigned int Ber, int result )
{
	unsigned long devIdx, brdIdx, ponIdx;

	
	CHECK_PON_RANGE

	if((result != V2R1_BER_ALARM ) &&( result != V2R1_BER_CLEAR )) return( RERROR );
	if( PonPortIsWorking(PonPortIdx) != TRUE )  return( RERROR );

	devIdx = 1;
	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	ponIdx = GetPonPortByPonChip( PonPortIdx );
		
	if(result == V2R1_BER_ALARM)
		{
		if ( PonPortTable[PonPortIdx].BerFlag == V2R1_TRAP_SEND) return ( ROK );
		
		ponPortBERAlarm_EventReport( devIdx, brdIdx, ponIdx, Ber );
		PonPortTable[PonPortIdx].BerFlag = V2R1_TRAP_SEND;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
        PonOpticalDataAbnormal(PonPortIdx);
#endif
		}
	else {
		if ( PonPortTable[PonPortIdx].BerFlag == V2R1_TRAP_CLEAR ) return ( ROK );
				
       	ponPortBERAlarmClear_EventReport( devIdx,  brdIdx,  ponIdx, Ber );
		PonPortTable[PonPortIdx].BerFlag = V2R1_TRAP_CLEAR;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
        PonOpticalDataNormal(PonPortIdx);
#endif
		}
	return( ROK );
}


int  Trap_OnuPonPortBER(short int PonPortIdx, short int OnuIdx, unsigned int Ber, int result )
{
	unsigned long devIdx, brdIdx, ponIdx;

	int slot;
	short int port;
	short int OnuEntry;
	int berFlag;
	
	CHECK_ONU_RANGE;

	if((result != V2R1_BER_ALARM ) &&( result != V2R1_BER_CLEAR )) return( RERROR );
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) return( RERROR );

	slot = GetCardIdxByPonChip( PonPortIdx );
	port = GetPonPortByPonChip( PonPortIdx );
	/*devIdx = (unsigned long ) slot *10000 +(unsigned long) port *1000 +(OnuIdx + 1);*/
       devIdx=MAKEDEVID((unsigned long )slot,(unsigned long) port,(OnuIdx + 1) );

	brdIdx = 1;
	ponIdx = 1;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	berFlag = OnuMgmtTable[OnuEntry].BerFlag;
	ONU_MGMT_SEM_GIVE;
	if(result == V2R1_BER_ALARM)
	{
		if ( berFlag != V2R1_TRAP_SEND)
		{
			ponPortBERAlarm_EventReport( devIdx, brdIdx, ponIdx, Ber );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].BerFlag = V2R1_TRAP_SEND;
			ONU_MGMT_SEM_GIVE;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
            PonOpticalDataAbnormal(PonPortIdx);
#endif
		}
	}
	else 
	{
		if ( berFlag != V2R1_TRAP_CLEAR )
		{
       		ponPortBERAlarmClear_EventReport( devIdx,  brdIdx,  ponIdx, Ber );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].BerFlag = V2R1_TRAP_CLEAR;
			ONU_MGMT_SEM_GIVE;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
            PonOpticalDataNormal(PonPortIdx);
#endif
		}
	}
	return( ROK );
}


/* 功能:    PON误帧率超门限事件上报
   输入参数:devIdx－OLT/ONU设备索引
   			brdIdx－单板(槽位号)索引，范围1~11
			ponIdx－PON端口索引，范围1~4
			fer－误帧率，单位10E-6
   返回值:  成功－VOS_OK，错误－VOS_ERROR */

int  Trap_PonPortFER(short int PonPortIdx, unsigned int Fer, int result )
{
	unsigned long devIdx, brdIdx, ponIdx;


	CHECK_PON_RANGE

	if((result != V2R1_BER_ALARM ) &&( result != V2R1_BER_CLEAR )) return( RERROR );
	if( PonPortIsWorking(PonPortIdx) != TRUE )  return( RERROR );

	devIdx = 1;
	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	ponIdx = GetPonPortByPonChip( PonPortIdx );
		
	if(result == V2R1_BER_ALARM)
		{
		if ( PonPortTable[PonPortIdx].FerFlag == V2R1_TRAP_SEND) return ( ROK );
		
		ponPortFERAlarm_EventReport( devIdx, brdIdx, ponIdx, Fer );
		PonPortTable[PonPortIdx].FerFlag = V2R1_TRAP_SEND;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
        PonOpticalDataAbnormal(PonPortIdx);
#endif
		}
	else {
		if ( PonPortTable[PonPortIdx].FerFlag == V2R1_TRAP_CLEAR ) return ( ROK );
				
       	ponPortFERAlarmClear_EventReport( devIdx,  brdIdx,  ponIdx, Fer );
		PonPortTable[PonPortIdx].FerFlag = V2R1_TRAP_CLEAR;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
        PonOpticalDataNormal(PonPortIdx);
#endif
		}
	return( ROK );
}


int  Trap_OnuPonPortFER(short int PonPortIdx, short int OnuIdx, unsigned int Fer, int result )
{
	unsigned long devIdx, brdIdx, ponIdx;

	int slot;
	short int port;
	short int OnuEntry;
	int ferFlag;
	
	CHECK_ONU_RANGE

	if((result != V2R1_BER_ALARM ) &&( result != V2R1_BER_CLEAR )) return( RERROR );
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) return( RERROR );

	slot = GetCardIdxByPonChip( PonPortIdx );
	port = GetPonPortByPonChip( PonPortIdx );
	/*devIdx = (unsigned long ) slot *10000 +(unsigned long) port *1000 +(OnuIdx + 1);*/
        devIdx=MAKEDEVID((unsigned long ) slot,(unsigned long) port,(OnuIdx + 1));

	brdIdx = 1;
	ponIdx = 1;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
		
	ONU_MGMT_SEM_TAKE;
	ferFlag = OnuMgmtTable[OnuEntry].FerFlag;
	ONU_MGMT_SEM_GIVE;
	if(result == V2R1_BER_ALARM)
	{
		if( ferFlag != V2R1_TRAP_SEND )
		{
			ponPortFERAlarm_EventReport( devIdx, brdIdx, ponIdx, Fer );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].FerFlag = V2R1_TRAP_SEND;
			ONU_MGMT_SEM_GIVE;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
            PonOpticalDataAbnormal(PonPortIdx);
#endif
		}
	}
	else
	{
		if( ferFlag != V2R1_TRAP_CLEAR )
		{
       		ponPortFERAlarmClear_EventReport( devIdx,  brdIdx,  ponIdx, Fer );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[OnuEntry].FerFlag = V2R1_TRAP_CLEAR;
			ONU_MGMT_SEM_GIVE;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICERROR == EPON_MODULE_YES )
            PonOpticalDataNormal(PonPortIdx);
#endif
		}
	}
	return( ROK );
}

int  Trap_OnuAuthFailure( short int  PonPortIdx,  unsigned char  *MacAddr )
{
	unsigned long devIdx, brdIdx, ponIdx;
	
	CHECK_PON_RANGE

	if( MacAddr == NULL ) return( RERROR );

	devIdx = 1;
	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	ponIdx = GetPonPortByPonChip( PonPortIdx );
	
	onuRegAuthFailure_EventReport( devIdx, brdIdx, ponIdx, MacAddr );
 	return( ROK );
}

int Trap_PonAutoProtectSwitch( short int PonPortIdx)
{
	unsigned long devIdx = 1, brdIdx = 0, ponIdx = 0, partnerBrdIdx = 0, partnerPonIdx = 0;
	CHECK_PON_RANGE

	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	ponIdx = GetPonPortByPonChip( PonPortIdx );
	/*added by wangjiah@2017-04-14 trap different pon protect event by protect type -- remote or local*/
	if(VOS_OK == PonPortAutoProtectPortQuery(brdIdx, ponIdx, &partnerBrdIdx, &partnerPonIdx) && SYS_MODULE_IS_REMOTE(partnerBrdIdx))
	{
		ponProtectSwitch_EventReport(brdIdx, ponIdx, partnerBrdIdx, partnerPonIdx, 0, 0);
	}
	else
	{
		autoProtectSwitch_EventReport( devIdx, brdIdx, ponIdx );
	}

	return( ROK );
}

/* added by chenfj 2009-2-12
	用于记录 PON 端口运行异常和运行版本不匹配
*/
void  SysLog_PonPortAbnormal(short int PonPortIdx, int type)
{
	int slot, port;
	/*CHECK_PON_RANGE*/

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	switch (type )
		{
		case 1:
 			VOS_SysLog( LOG_TYPE_DEVSM, LOG_CRIT, "pon%d/%d is abnormal(sw-cpu timeout)", slot, port);
			break;
		case 2:
			VOS_SysLog( LOG_TYPE_DEVSM, LOG_CRIT, "pon%d/%d is abnormal(pon-self reset)", slot, port);
			break;
		case 3:
			VOS_SysLog( LOG_TYPE_DEVSM, LOG_CRIT, "pon%d/%d is abnormal(pon not-inited)", slot, port);
			break;
		case ROK:
			VOS_SysLog( LOG_TYPE_DEVSM, LOG_CRIT, "pon%d/%d is comm with sw-cpu ok", slot, port);
			break;
		default:
			VOS_SysLog( LOG_TYPE_DEVSM, LOG_CRIT, "pon%d/%d is comm with sw-cpu err-code:%d", slot, port,type);
			break;
		}
}

void  FlashAlarm_PonPortVersionMismatch(short int PonPortIdx, int VersionType, int  MismatchFlag)
{
	int slot, port;
	/*CHECK_PON_RANGE*/

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if( MismatchFlag  == V2R1_ENABLE)  /* 版本不匹配*/
	{
		switch (VersionType )
		{
			case  PON_OLT_BINARY_FIRMWARE: /* firmware */
				ponFWVersionMismatch_EventReport( 1, slot, port );
				break;
			case PON_OLT_BINARY_DBA_DLL:  /* DBA */
				ponDBAVersionMismatch_EventReport( 1, slot, port );
				break;
		}
	}
	else if( MismatchFlag  == V2R1_DISABLE)  /* 版本匹配*/
	{
		switch (VersionType )
		{
			case  PON_OLT_BINARY_FIRMWARE: /* firmware */
				ponFWVersionMatch_EventReport( 1, slot, port );
				break;
			case PON_OLT_BINARY_DBA_DLL:  /* DBA */
				ponDBAVersionMatch_EventReport( 1, slot, port );
				break;
		}
	}
}

void  Trap_PonPortSFPTypeMismatch(short int PonPortIdx, int  MismatchFlag)
{
	ulong_t  slot, port;

	slot = (ulong_t)GetCardIdxByPonChip(PonPortIdx);
	port = (ulong_t)GetPonPortByPonChip(PonPortIdx);

	if( MismatchFlag  == V2R1_ENABLE)  /* 类型不匹配*/
	{
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE)
		{
			ponSFPTypeMismatch_EventReport(1, slot, port);
		}
	}
	else if( MismatchFlag  == V2R1_DISABLE)  /* 类型匹配*/
	{
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm == V2R1_ENABLE)
		{
			ponSFPTypeMatch_EventReport(1, slot, port);
		}
	}
	return;
}

extern int check_sfp_online_state(unsigned int PonPortIdx);
void Trap_PonPortSignalLoss(short int PonPortIdx, int SignalLoss)
{
	int slot, port;
	int iXfpType = XFP_TYPE_UNKNOWN;
	
	
	CHECK_PON_RANGE
	if((SignalLoss != V2R1_ENABLE) && ((SignalLoss != V2R1_DISABLE)))
		return;
	if( SYS_LOCAL_MODULE_TYPE_IS_GPON)/*32404, pon灯常亮不灭 by jinhl@2016.09.07*/
	{
		extern int GW_PonLinkFlag(short int olt_id, bool flag);
		if(SignalLoss == V2R1_ENABLE)
			GW_PonLinkFlag(PonPortIdx,0);
		else
			GW_PonLinkFlag(PonPortIdx,1);
	}
    /* B--added by liwei056@2015-1-5 for D23585 */
#if 1
    if( PonPortIsWorking(PonPortIdx) != TRUE )  return;
#endif
    /* E--added by liwei056@2015-1-5 for D23585 */
	
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	/*added by wangjiah@2016-09-22:begin*/
	/*set link LED especially for 8xep to solve issue:32970*/
	if(SignalLoss == V2R1_DISABLE)
	{
        SetPonNotLoosingFlag( PonPortIdx );	
	}
	else
	{	
        SetPonLoosingFlag( PonPortIdx );
	}
	/*added by wangjiah@2016-09-22:begin*/

	/*add by yanjy2017-1*/
	/*光模块不匹配时重复上报Los和Los clear*/
	if (VOS_OK == check_sfp_online_state( PonPortIdx )) 
	{
		 if(2 ==  checkSfpTypeEponOrGpon(PonPortIdx))/*不匹配*/
			return;
	}
	/*add by yanjy*/

	/* modified by xieshl 20111102, 解决PON LOS告警漏报问题，PON芯片和ONU全部离线、ONU第一个注册时都会上报，
	    这里需要防止重复上报，问题单13210 */
	if( PonPortTable[PonPortIdx].SignalLossFlag == SignalLoss )
	{
		/*sys_console_printf("\r\n %% Rx pon %d/%d LOS alm%s,ignored!\r\n", slot, port, (SignalLoss == V2R1_ENABLE ? "" : " clear"));*/
		return;
	}

	/*modified by wangjiah@2016-09-22:begin*/
	PonPortTable[PonPortIdx].SignalLossFlag = SignalLoss;
	/*added by wangjiah@2017-02-21 to report 10gepon port status change*/
	if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
	{
		if(VOS_OK != ponSfp_getXfpType(PonPortIdx, &iXfpType))
		{
			iXfpType = XFP_TYPE_UNKNOWN;
		}
		UpdateXGEPonPortInfoByType(PonPortIdx, iXfpType);
	}
	if(SignalLoss == V2R1_DISABLE)
	{
        //SetPonNotLoosingFlag( PonPortIdx );
		if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
		{
			ponPortLosAlarmClearWithXFPType_EventReport(1, slot, port, iXfpType);
		}
		else
		{
			ponPortLosAlarmClear_EventReport(1,slot, port);
		}
		
	}
	else
	{	
        //SetPonLoosingFlag( PonPortIdx );
		if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
		{
			ponPortLosAlarmWithXFPType_EventReport(1, slot, port, iXfpType);
		}
		else
		{
			ponPortLosAlarm_EventReport(1, slot, port);
		}
		ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss(PonPortIdx); 
	}
	/* modified by wangjiah@2016-09-22:end */
	return;
}	

int  Trap_PonPortFull(short int PonPortIdx)
{
	unsigned long devIdx, slot, port;
	
	CHECK_PON_RANGE
	
	/*if(PonPortTable[PonPortIdx].PortFullAlarm == V2R1_ENABLE)
		return ROK;*/		/* removed by xieshl 20110810 */

	devIdx=1;
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	ponPortFull_EventReport(devIdx, slot, port);
	PonPortTable[PonPortIdx].PortFullAlarm = V2R1_ENABLE;
	
	return ROK;
}

int Trap_PonPortFullClear(short int PonPortIdx)
{
/*	unsigned long devIdx, slot, port;*/
	
	CHECK_PON_RANGE
	
	if(PonPortTable[PonPortIdx].PortFullAlarm != V2R1_ENABLE)
		return ROK;

/*	devIdx=1;
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	ponPortFullClear_EventReport(devIdx, slot, port);*/
	PonPortTable[PonPortIdx].PortFullAlarm = V2R1_DISABLE;
	
	return ROK;
}

/* B--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */
int AbnormalOnuReport(const PON_olt_id_t				 olt_id,
					        const PON_onu_id_t				 onu_id,
					        PASCOMM_event_onu_registration_t  *reg_event)
{
    int iRlt;
    int brdIdx, ponIdx;
    short int OnuIdx = 0;
	brdIdx = GetCardIdxByPonChip( olt_id );
	ponIdx = GetPonPortByPonChip( olt_id );
    OnuIdx = GetOnuIdxByLlid(olt_id, onu_id);
    sys_console_printf("\r\nPon%d/%d: llid(%d)-Mac(%02X.%02X.%02X.%02X.%02X.%02X) is a abnormal onu[passave_onu(%d), oam_device_iter(0x%04X), oam_version_iter(0x%04X)]\r\n"
        , brdIdx, ponIdx
        , onu_id
        , reg_event->onu_mac[0]
        , reg_event->onu_mac[1]
        , reg_event->onu_mac[2]
        , reg_event->onu_mac[3]
        , reg_event->onu_mac[4]
        , reg_event->onu_mac[5]
        , reg_event->passave_originated
        , reg_event->device_id
        , reg_event->hw_version
        );
    AddPendingOnu(olt_id, OnuIdx, onu_id, reg_event->onu_mac, PENDING_REASON_CODE_GET_ONUVER_FAIL);
	/*iRlt = onuRegAuthFailure_EventReport( OLT_DEV_ID, brdIdx, ponIdx, reg_event->onu_mac );*/

    return iRlt;
}

int TestAbnormalOnuReport(int iTestTimes)
{
    int i;
    int iOkTimes;
    PASCOMM_event_onu_registration_t reg_event;

    VOS_MemZero(&reg_event, sizeof(reg_event));
    reg_event.onu_id = 1;
    reg_event.onu_mac[0] = 0x00;
    reg_event.onu_mac[1] = 0x01;
    reg_event.onu_mac[2] = 0x02;
    reg_event.onu_mac[3] = 0x03;
    reg_event.onu_mac[4] = 0x04;
    reg_event.onu_mac[5] = 0x05;
    reg_event.passave_originated = FALSE;
    reg_event.device_id = 0x6301;
    reg_event.hw_version = 0x6300;
    for (i=0, iOkTimes=0; i<iTestTimes; ++i)
    {
        if ( 0 == AbnormalOnuReport(0, 1, &reg_event ) )
        {
            ++iOkTimes;
        }
    }

    return iOkTimes;
}
/* E--added by liwei056@2011-5-27 for [山东聊城下接其它厂商的ONU，在此失败] */

/* B--added by liwei056@2011-12-20 for [PON板测试] */
int AbnormalPonReport(const PON_olt_id_t				 olt_id,
					       int abnormal_code, const char *abnormal_desc)
{
    int iRlt;
    int brdIdx, ponIdx;

	brdIdx = GetCardIdxByPonChip( olt_id );
	ponIdx = GetPonPortByPonChip( olt_id );

    sys_console_printf("\r\nPon%d/%d access abnormal(%d) on %s.\r\n"
        , brdIdx, ponIdx
        , abnormal_code
        , (NULL == abnormal_desc) ? "unknown" : abnormal_desc
        );
    
	/* iRlt = Trap_PonPortAbnormal( olt_id, abnormal_code ); */

    return iRlt;
}
/* E--added by liwei056@2011-12-20 for [PON板测试] */

#endif


#ifdef V2R1_DEBUG_CLI
#endif

int ShowOnuLlidMappingByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	short int Llid;
	short int Llid_num;

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx , OnuIdx ) != ROK )
		/*
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr , Invalid_Mac_Addr ) == ROK ) 
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr , Invalid_Mac_Addr1 ) == ROK )) 
		*/
		{
		vty_out(vty, "\r\n  %s/port%d onu%d not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
		return( RERROR );
		} 

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN )
		{
		vty_out(vty, "\r\n  %s/port%d onu%d off-line\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
		return( RERROR );
		}

    if ( 1 == (Llid_num = GetOnuLLIDNum(PonPortIdx, OnuIdx)) )
    {
    	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
    	if( Llid  == INVALID_LLID ) 
    		vty_out(vty, "\r\n  %s/port%d onu%d -- llid mapping err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
    	else
    		vty_out(vty, "\r\n  %s/port%d onu%d -- llid %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), Llid);
    }
    else
    {
       	short int LLIDIdx;
       	short int LLIDMax;
        int OnuID;
        int CardIdx, PortIdx;

        CardIdx = GetCardIdxByPonChip(PonPortIdx);
        PortIdx = GetPonPortByPonChip(PonPortIdx);
        OnuID   = (OnuIdx+1);

        LLIDMax = GetOnuLLIDNumMax(PonPortIdx, OnuIdx);
        LLIDMax = min(MAXLLIDPERONU, LLIDMax);
        for ( LLIDIdx = 0; LLIDIdx < LLIDMax; LLIDIdx++ )
        {
        	Llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);
        	if( Llid != INVALID_LLID ) 
        		vty_out(vty, "\r\n  %s/port%d onu%d -- llid %d\r\n", CardSlot_s[CardIdx], PortIdx, OnuID, Llid);
        }
    }
    
	return( ROK );
}

int ShowOnuLlidMappingByVty_1( short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	short int Llid;
	short int Llid_num;

	CHECK_ONU_RANGE

	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK )
		/*
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr , Invalid_Mac_Addr ) == ROK ) 
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr , Invalid_Mac_Addr1 ) == ROK )) 
		*/
		{
		/*vty_out(vty,"\r\n  %s/port%d onu%d not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );*/
		return( RERROR );
		} 

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN )
		{
		/*vty_out(vty, "\r\n  %s/port%d onu%d off-line\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );*/
		return( RERROR );
		}

    if ( 1 == (Llid_num = GetOnuLLIDNum(PonPortIdx, OnuIdx)) )
    {
    	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
    	if( Llid  == INVALID_LLID ) 
    		vty_out(vty, "  %s/port%d onu%d -- llid mapping err\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
    	else
    		vty_out(vty, "  %s/port%d onu%d -- llid %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), Llid);
    }
    else
    {
       	short int LLIDIdx;
       	short int LLIDMax;
        int OnuID;
        int CardIdx, PortIdx;

        CardIdx = GetCardIdxByPonChip(PonPortIdx);
        PortIdx = GetPonPortByPonChip(PonPortIdx);
        OnuID   = (OnuIdx+1);
        
        LLIDMax = GetOnuLLIDNumMax(PonPortIdx, OnuIdx);
        LLIDMax = min(MAXLLIDPERONU, LLIDMax);
        for ( LLIDIdx = 0; LLIDIdx < LLIDMax; LLIDIdx++ )
        {
        	Llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);
        	if( Llid != INVALID_LLID ) 
        		vty_out(vty, "  %s/port%d onu%d -- llid %d\r\n", CardSlot_s[CardIdx], PortIdx, OnuID, Llid);
        }
    }
    
	return( ROK );
}

#ifdef V2R1_DEBUG_INFO
#endif

int SetPonPhyDebug(short int PonPortIdx, int Debug_flag)
{
	CHECK_PON_RANGE

	if(( Debug_flag != V2R1_ENABLE ) && (Debug_flag != V2R1_DISABLE ))return( RERROR );

	PonPhyDebug[PonPortIdx] = Debug_flag;
	return( ROK );
}

int SetPonMsgDebug(short int PonPortIdx, int Debug_flag)
{
	CHECK_PON_RANGE

	if(( Debug_flag != V2R1_ENABLE ) && (Debug_flag != V2R1_DISABLE ))return( RERROR );

	PonMsgDebug[PonPortIdx] = Debug_flag;
	return( ROK );
}
 
int SetPonOamMsgDebug(short int PonPortIdx, int Debug_flag)
{
	CHECK_PON_RANGE

	if(( Debug_flag != V2R1_ENABLE ) && (Debug_flag != V2R1_DISABLE ))return( RERROR );

	PonOamMsgDebug[PonPortIdx] = Debug_flag;
	return( ROK );
}
 
int ClearMsgCount( short int PonPortIdx )
{
	CHECK_PON_RANGE
	
	PAS_send_olt_msg[PonPortIdx] = 0;
	PAS_rev_olt_msg[PonPortIdx] = 0;
	return( ROK );
}

int ShowMsgCountByVty( short int PonPortIdx, struct vty *vty )
{
	CHECK_PON_RANGE
	vty_out(vty, "\r\n  %s/port%d Msg Counter:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	vty_out(vty, "    Send Msg:%d\r\n", PAS_send_olt_msg[PonPortIdx]);
	vty_out(vty, "    Recv Msg:%d\r\n", PAS_rev_olt_msg[PonPortIdx]);
	return( ROK );
}

/*int ShowMsgCount( short int PonPortIdx )
{
	CHECK_PON_RANGE
	sys_console_printf("\r\n  %s/port%d Msg Counter:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
	sys_console_printf("    Send Msg:%d\r\n", PAS_send_olt_msg[PonPortIdx]);
	sys_console_printf("    Recv Msg:%d\r\n", PAS_rev_olt_msg[PonPortIdx]);
	return( ROK );
}*/

int SetGeneralEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_DEBUG = V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_DEBUG = V2R1_DISABLE;
		return( ROK );
		}	
	return(RERROR );
}

int SetEncryptEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_ENCRYPT= V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_ENCRYPT = V2R1_DISABLE;
		return( ROK );
		}	
	return(RERROR );
}


int SetRegisterEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_REGISTER= V2R1_ENABLE;
		EVENT_EUQ = V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_REGISTER = V2R1_DISABLE;
		EVENT_EUQ = V2R1_DISABLE;
		return( ROK );
		}	
	return( RERROR );
}

int SetAlarmEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_ALARM =V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_ALARM = V2R1_DISABLE;
		return( ROK );
		}	
	return( RERROR );
}

int SetResetEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_RESET= V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_RESET = V2R1_DISABLE;
		return( ROK );
		}	
	return( RERROR );
}

int SetAddPonEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_PONADD = V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_PONADD = V2R1_DISABLE;
		return( ROK );
		}	
	return( RERROR );
}

int SetRpcEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
	{
        EVENT_RPC  = V2R1_ENABLE;
        EVENT_NULL = V2R1_ENABLE;
        EVENT_GW   = V2R1_ENABLE;
        EVENT_PAS  = V2R1_ENABLE;
        EVENT_TK   = V2R1_ENABLE;
        EVENT_BCM  = V2R1_ENABLE;
        return( ROK );
	}
	else if( debugFlag == V2R1_DISABLE)
	{
        EVENT_RPC  = V2R1_DISABLE;
        EVENT_NULL = V2R1_DISABLE;
        EVENT_GW   = V2R1_DISABLE;
        EVENT_PAS  = V2R1_DISABLE;
        EVENT_TK   = V2R1_DISABLE;
        EVENT_BCM  = V2R1_DISABLE;
        return( ROK );
	}	
	return( RERROR );
}

int SetSwitchPonEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
		{
		EVENT_PONSWITCH = V2R1_ENABLE;
		EVENT_SYN = V2R1_ENABLE;
		return( ROK );
		}
	else if( debugFlag == V2R1_DISABLE)
		{
		EVENT_PONSWITCH = V2R1_DISABLE;
		EVENT_SYN = V2R1_DISABLE;
		return( ROK );
		}	
	return( RERROR );
}

int SetRemoteEvent( int debugFlag )
{
	if(debugFlag == V2R1_ENABLE)
	{
        EVENT_REMOTE  = V2R1_ENABLE;
        return( ROK );
	}
	else if( debugFlag == V2R1_DISABLE)
	{
        EVENT_REMOTE  = V2R1_DISABLE;
        return( ROK );
	}	
	return( RERROR );
}

int  SetMakeingTestEvent(int debugFlag )
{
	short int PonPortIdx;

#if 0
	if(debugFlag == V2R1_ENABLE)
	{
		/*add by chenfj 2006/10/11
	    #2606 问题 2606.在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
		if(MAKEING_TEST_FLAG == V2R1_DISABLE)
		{
			for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
			{
				ActivatePendingOnuMsg( PonPortIdx );
				ActivatePendingOnuMsg_conf_all( PonPortIdx );
			}
		}
	
		MAKEING_TEST_FLAG = V2R1_ENABLE;
		return( ROK );
	}
	else if( debugFlag == V2R1_DISABLE)
	{
		MAKEING_TEST_FLAG = V2R1_DISABLE;
		return( ROK );
	}	
#else
	if(debugFlag != MAKEING_TEST_FLAG)
    {
        if ( OLT_CALL_ISOK( OLT_SetAllOnuBindMode(OLT_ID_ALL, debugFlag) )
            && (V2R1_ENABLE == debugFlag))
        {
            if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
            {
    			for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
    			{
    				ActivatePendingOnuMsg( PonPortIdx );
    				ActivatePendingOnuMsg_conf_all( PonPortIdx );
    			}
            }
        }
    }   
#endif

	return( ROK );
}

int GetMakeingTestFlagDefault()
{
	return( MAKEING_TEST_FLAG_DEFAULT);
}

int GetMakeingTestFlag()
{
	return(MAKEING_TEST_FLAG);
}

int SetOamCommFlag( int flag )
{
	if(flag == V2R1_ENABLE )
	{
		EVENT_OAM_DEBUG = V2R1_ENABLE;
		return( ROK );
	}
	else if( flag == V2R1_DISABLE)
	{
		EVENT_OAM_DEBUG =V2R1_DISABLE;
		return( ROK );
	}	

    return( RERROR );
}

int SetOnuPolicerFlag( int flag)
{
#if 1		
    OLT_SetAllOnuDownlinkPoliceMode(OLT_ID_ALL, flag);
#else
    downlinkBWlimit = V2R1_ENABLE;
#endif
    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
        short int PonPortIdx, OnuIdx;

        for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
        {
            /* 判断该pon是否可用*/
            if (!OLTAdv_IsExist ( PonPortIdx )) continue;

        	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
            {
                if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) ) continue;
                if ( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) continue;
#if 0
        	    ActiveOnuDownlinkBW(PonPortIdx, OnuIdx );
#else
                OnuEvent_Active_DownlinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
            }            
        }         
    }
		
    return( ROK );
}

int GetOnuPolicerFlagDefault()
{
    return( downlinkBWlimitDefault);
}

int GetOnuPolicerFlag()
{
    return( downlinkBWlimit );
}

/* B--added by liwei056@2011-4-7 for GuestNeed */
int GetOltPolicerFlag(short int PonPortIdx)
{
    OLT_LOCAL_ASSERT(PonPortIdx);
    return( PonPortTable[PonPortIdx].DownlinkPoliceMode );
}

int SetOltPolicerFlag(short int PonPortIdx, int flag)
{
    OLT_LOCAL_ASSERT(PonPortIdx);

	SWAP_TO_ACTIVE_PON_PORT_INDEX(PonPortIdx)

    if ( PonPortTable[PonPortIdx].DownlinkPoliceMode != flag )
    {
	    OLT_SetOnuDownlinkPoliceMode(PonPortIdx, flag);

        if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
        {
        	short int OnuIdx;
            
            /* 判断该pon是否可用*/
            if ( OLTAdv_IsExist ( PonPortIdx ) )
            {
    			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
                {
                    if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) ) continue;
                    if ( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) continue;
#if 0
    				ActiveOnuDownlinkBW(PonPortIdx, OnuIdx );
#else
                    OnuEvent_Active_DownlinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
                }            
            }
        }
    }

    return( ROK );
}
/* E--added by liwei056@2011-4-7 for GuestNeed */

int ShowAllDebugFlag(struct vty *vty)
{
	if( EVENT_DEBUG == V2R1_ENABLE )
		vty_out(vty, "  general event: enable\r\n");
	else vty_out ( vty, "  general event: disable\r\n");

	if( EVENT_ENCRYPT== V2R1_ENABLE )
		vty_out(vty, "  encrypt event: enable\r\n");
	else vty_out ( vty, "  encrypt event: disable\r\n");

	if( EVENT_REGISTER == V2R1_ENABLE )
		vty_out(vty, "  onu register event: enable\r\n");
	else vty_out ( vty, "  onu register event: disable\r\n");

	if( EVENT_ALARM == V2R1_ENABLE )
		vty_out(vty, "  alarm event: enable\r\n");
	else vty_out ( vty, "  alarm event: disable\r\n");

	if( EVENT_PONADD == V2R1_ENABLE )
		vty_out(vty, "  activate pon port: enable\r\n");
	else vty_out ( vty, "  activate pon port: disable\r\n");
	
	if( EVENT_RESET == V2R1_ENABLE )
		vty_out(vty, "  pon reset flag:enable\r\n");
	else vty_out ( vty, "  pon reset flag:disable\r\n");

	if( EVENT_PONSWITCH == V2R1_ENABLE )
		vty_out(vty, "  switch pon port: enable\r\n");
	else vty_out ( vty, "  switch pon port: disable\r\n");

	if( EVENT_RPC == V2R1_ENABLE )
		vty_out(vty, "  rpc pon port: enable\r\n");
	else vty_out ( vty, "  rpc pon port: disable\r\n");
	/*
	if( MAKEING_TEST_FLAG == V2R1_ENABLE )
		vty_out(vty, "  making test: enable\r\n");
	else vty_out ( vty, "  making test: disable\r\n");

	if( downlinkBWlimit == V2R1_ENABLE)
		vty_out(vty, "  downlink policer: enable\r\n");
	else vty_out ( vty, "  downlink polilcer: disable\r\n");

	if(  EVENT_OAM_DEBUG == V2R1_ENABLE )
		vty_out(vty, "  Oam comm: enable\r\n");
	else vty_out ( vty, "  Oam comm: disable\r\n");
	*/
	if(EVENT_UPDATE_ONU_FILE == V2R1_ENABLE)
		vty_out(vty,"  debug update onu file: enable\r\n");
	else vty_out(vty, "  debug update onu file: disable\r\n");

	vty_out(vty, "  (host <-->pon chip)physical level msg trace(1-enable,2-disable) \r\n");
	{
	short int slot;
	short int FirstPonPort;
	for( slot=PONCARD_FIRST; slot<=PONCARD_LAST; slot++)
		{
		if(SlotCardIsPonBoard(slot) != ROK ) continue;
	
		vty_out(vty, "    %s:", CardSlot_s[slot]);
		for(FirstPonPort=1; FirstPonPort<=PONPORTPERCARD; FirstPonPort++)
			vty_out(vty,"%3d",PonMsgDebug[GetPonPortIdxBySlot(slot, FirstPonPort)]);
		vty_out(vty,"\r\n");
		}

	}

	vty_out(vty, "  (host <-->pon chip) oam level msg trace(1-enable,2-disable) \r\n");
	{
	short int slot;
	short int FirstPonPort;
	for( slot=PONCARD_FIRST; slot<=PONCARD_LAST; slot++)
		{
		if(SlotCardIsPonBoard(slot) != ROK ) continue;
	
		vty_out(vty, "    %s:", CardSlot_s[slot]);
		for(FirstPonPort=1; FirstPonPort<=PONPORTPERCARD; FirstPonPort++)
			vty_out(vty,"%3d",PonOamMsgDebug[GetPonPortIdxBySlot(slot, FirstPonPort)]);
		}

	}
	
	return( ROK );
}

/* B--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */
#if 1
int  SetOnuPolicerParam(int BwBurstSize, int BwPreference, int BwWeightSize )
{
	short int PonPortIdx, OnuIdx;

    if ( OLT_CALL_ISERROR(OLT_SetAllOnuDownlinkPoliceParam(OLT_ID_ALL, BwBurstSize, BwPreference, BwWeightSize)) )
    {
        return RERROR;
    }

    /* B--added by liwei056@2010-11-30 for D11311 */
	if ( V2R1_ENABLE == downlinkBWlimit )
    /* E--added by liwei056@2010-11-30 for D11311 */
    {
        if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
        {
			for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
            {
                /* 判断该pon是否可用*/
                if (!OLTAdv_IsExist ( PonPortIdx )) continue;

				for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
                {
                    if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) ) continue;
                    if ( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) continue;
#if 0        
					ActiveOnuDownlinkBW(PonPortIdx, OnuIdx);
#else
                    OnuEvent_Active_DownlinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
                }            
            }         
        }
    }   
    
	return( ROK );
}

int  GetOnuPolicerParam(int *BwBurstSize, int *BwPreference, int *BwWeightSize )
{
    *BwBurstSize  = downlinkBWlimitBurstSize;
    *BwPreference = downlinkBWlimitPreference;
    *BwWeightSize = downlinkBWWeight;
    
    return( ROK );
}
#endif
/* E--added by liwei056@2010-5-26 for LLID-DownLineRate BUG */

/* B--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */
#if 1
int  SetOnuDbaParam(int BwFixedPktSize, int BwBurstSize, int BwWeightSize)
{
	short int PonPortIdx, OnuIdx;

    if ( OLT_CALL_ISERROR(OLT_SetAllOnuUplinkDBAParam(OLT_ID_ALL, BwFixedPktSize, BwBurstSize, BwWeightSize)) )
    {
        return RERROR;
    }

    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
		for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
        {
            /* 判断该pon是否可用*/
            if (!OLTAdv_IsExist ( PonPortIdx )) continue;

			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
            {
                if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) ) continue;
                if ( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) continue;
#if 0
				ActiveOnuUplinkBW(PonPortIdx, OnuIdx);
#else
                OnuEvent_Active_UplinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
            }            
        }         
    }
    
	return( ROK );
}

int  GetOnuDbaParam(int *BwFixedPktSize, int *BwBurstSize, int *BwWeightSize)
{
    *BwFixedPktSize = uplinkBWPacketUnitSize;
    *BwBurstSize = uplinkBWlimitBurstSize;
    *BwWeightSize = uplinkBWWeight;
    
    return( ROK );
}
#endif
/* E--added by liwei056@2010-12-20 for P2P-64KLineRate-1G BUG */

/* B--added by liwei056@2012-3-6 for 国电测试 */
#if 1
int  SetOnuBwParams(int uplink_bwradio, int dwlink_bwradio)
{
	short int PonPortIdx, OnuIdx;

    if ( OLT_CALL_ISERROR(OLT_SetAllOnuBWParams(OLT_ID_ALL, uplink_bwradio, dwlink_bwradio)) )
    {
        return RERROR;
    }

    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
		for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx ++)
        {
            /* 判断该pon是否可用*/
            if (!OLTAdv_IsExist ( PonPortIdx )) continue;

			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
            {
                if ( ROK != ThisIsValidOnu(PonPortIdx, OnuIdx) ) continue;
                if ( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) continue;
#if 0
				ActiveOnuUplinkBW(PonPortIdx, OnuIdx);
#else
                OnuEvent_Active_UplinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
            }            
        }         
    }
    
	return( ROK );
}

int  GetOnuBwParams(int *uplink_bwradio, int *dwlink_bwradio)
{
    if ( NULL != uplink_bwradio )
    {
        *uplink_bwradio = Bata_ratio;
    }
    
    return( ROK );
}
#endif
/* E--added by liwei056@2012-3-6 for 国电测试 */


/* B--added by liwei056@2011-1-26 for D11957 */
#if 1
int  SetOnuDefaultBW(ONU_bw_t *default_bw, struct vty *vty)
{
    unsigned int ActDir, SetDir;
	short int ponRate = default_bw->bw_rate;

    SetDir = default_bw->bw_direction;
    if ( OLT_CFG_DIR_UPLINK & SetDir )
    {
        if(UPLINK_BW_EQUALS_TO_DEFAULT(ponRate, default_bw->bw_gr, default_bw->bw_be))
        {
            /* 上行默认带宽无变化 */
            SetDir &= ~OLT_CFG_DIR_UPLINK;
        }

        if ( OLT_CFG_DIR_DOWNLINK & SetDir )
        {
			if(DOWNLINK_BW_EQUALS_TO_DEFAULT(ponRate, default_bw->bw_fixed, default_bw->bw_actived))
            {
                /* 下行默认带宽无变化 */
                SetDir &= ~OLT_CFG_DIR_DOWNLINK;
            }
            else
            {
                if ( !(OLT_CFG_DIR_UPLINK & SetDir) )
                {
                    default_bw->bw_gr = default_bw->bw_fixed;
                    default_bw->bw_be = default_bw->bw_actived;
                }
            }
        }
    }
    else
    {
		if(DOWNLINK_BW_EQUALS_TO_DEFAULT(ponRate, default_bw->bw_gr, default_bw->bw_be))
        {
            /* 下行默认带宽无变化 */
            SetDir &= ~OLT_CFG_DIR_DOWNLINK;
        }
    }

    if ( 0 == SetDir )
    {
        /* 全局默认带宽无变化 */
        return 0;
    }
    else
    {
        default_bw->bw_direction = SetDir;
    }
    
    if ( OLT_CALL_ISERROR(OLT_SetAllOnuDefaultBW(OLT_ID_ALL, default_bw)) )
    {
        return RERROR;
    }

    if ( SYS_LOCAL_MODULE_RUNNINGSTATE == MODULE_RUNNING )
    {
        short int PonPortIdx, OnuIdx;
        int iRlt;
        int onu_entry_base;
        CHAR *pMac;
        ONUTable_S *pOnuCfg;
        unsigned int PonMaxBW, PonMaxDnBW, PonActUpBW, PonActDnBW;
		unsigned int def_up_bw_gr = 0;
		int onuType = V2R1_DEVICE_UNKNOWN;
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
        /* B--added by liwei056@2012-8-10 for D15287 */
        unsigned int def_up_bw_new, def_dn_bw_new;
        unsigned int act_up_bw_diff, act_dn_bw_diff;
        
		if(PON_RATE_10_10G == default_bw->bw_rate)
		{
			def_up_bw_new = OnuConfigDefault.UplinkBandwidth_XGEPON_SYM;
			def_dn_bw_new = OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM;
		}
		else if(PON_RATE_1_10G == default_bw->bw_rate)
		{
			def_up_bw_new = OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM;
			def_dn_bw_new = OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM;
		}
		else if(PON_RATE_NORMAL_1G == default_bw->bw_rate)
		{
			def_up_bw_new = OnuConfigDefault.UplinkBandwidth;
			def_dn_bw_new = OnuConfigDefault.DownlinkBandwidth;
		}
		else if(PON_RATE_1_2G == default_bw->bw_rate)
		{
			def_up_bw_new = OnuConfigDefault.UplinkBandwidth_GPON;
			def_dn_bw_new = OnuConfigDefault.DownlinkBandwidth_GPON;
		}
        /* E--added by liwei056@2012-8-10 for D15287 */
#endif      


    	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++ )
        {
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
            /* B--added by liwei056@2012-8-10 for D15287 */
        	UpdateProvisionedBWInfo(PonPortIdx);
            PonActUpBW = PonPortTable[PonPortIdx].ActiveBW;
            PonActDnBW = PonPortTable[PonPortIdx].DownlinkActiveBw;
            /* E--added by liwei056@2012-8-10 for D15287 */
#else
            PonActUpBW = 0;
            PonActDnBW = 0;
#endif      
			PonMaxBW = PonPortTable[PonPortIdx].MaxBW;
			PonMaxDnBW = PonPortTable[PonPortIdx].DownlinkMaxBw;

            /* SetDir = default_bw->bw_direction; */
            onu_entry_base = PonPortIdx * MAXONUPERPON;
    		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++ )
            {
                pOnuCfg = &(OnuMgmtTable[onu_entry_base + OnuIdx]);
                pMac = pOnuCfg->DeviceInfo.MacAddr;
                if ( ThisIsValidOnu(PonPortIdx,OnuIdx))  continue;

				GetOnuType(PonPortIdx,OnuIdx,&onuType);

				if(onuType == V2R1_ONU_GPON && ponRate != PON_RATE_1_2G) continue;  
				else if (onuType != V2R1_ONU_GPON && ponRate != OnuMgmtTable[onu_entry_base + OnuIdx].PonRate) continue;
#if 1
                /* 默认带宽的ONU , 才需激活其默认带宽*/
                if ( 0 != (ActDir = (pOnuCfg->BandWidthIsDefault & SetDir)) )
#else
                /* 不在线的ONU , 才需激活其默认带宽 */
                if ( ONU_OPER_STATUS_DOWN == OnuMgmtTable[onu_entry_base + OnuIdx].OperStatus )
#endif
                {
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
                    /* B--added by liwei056@2012-8-10 for D15287 */

                    /* 要设置的默认带宽方向有超限，则无需遍历后面的ONU了[因为前面的肯定比后面的默认带宽大] */
                    if ( OLT_CFG_DIR_UPLINK & ActDir )
                    {
                        act_up_bw_diff = def_up_bw_new - pOnuCfg->ActiveUplinkBandwidth;
                    	if( (PonActUpBW + act_up_bw_diff) > PonMaxBW )
                        {
                            /* 有一个方向的默认带宽超限，就不设置 */
                            if ( vty != NULL )
                                vty_out(vty, "  warning:PON%d/%d's uplink-bandwidth is out of Max-Bandwidth-Limit.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));

                            break;
                        }   
                    }

                    if ( OLT_CFG_DIR_DOWNLINK & ActDir )
                    {
                        act_dn_bw_diff = def_dn_bw_new - pOnuCfg->ActiveDownlinkBandwidth;
                    	if( (PonActDnBW + act_dn_bw_diff) > PonMaxDnBW )
                        {
                            /* 有一个方向的默认带宽超限，就不设置 */
                            if ( vty != NULL )
                                vty_out(vty, "  warning:PON%d/%d's downlink-bandwidth is out of Max-Bandwidth-Limit.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));

                            break;
                        }   
                    }
                    /* E--added by liwei056@2012-8-10 for D15287 */
#endif

                    iRlt = ActiveOnuDefaultBW(PonPortIdx, OnuIdx, ActDir);
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
                    /* B--added by liwei056@2012-8-10 for D15287 */
        			if ( 0 == iRlt )
                    {
                        if ( OLT_CFG_DIR_UPLINK & ActDir )
                        {
                            PonActUpBW += act_up_bw_diff;
                        }

                        if ( OLT_CFG_DIR_DOWNLINK & ActDir )
                        {
                            PonActDnBW += act_dn_bw_diff;
                        }
                    }         
                    /* E--added by liwei056@2012-8-10 for D15287 */
#endif
                }

#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
                if ( PonActDnBW < PonMaxDnBW )
                {
                    if ( (PonActDnBW += pOnuCfg->ActiveDownlinkBandwidth) > PonMaxDnBW )
                    {
                        /* 下行带宽超限，仅仅提示，后续的ONU仍需再设置新的默认下行带宽 */
                        if ( vty != NULL )
                            vty_out(vty, "  warning:PON%d/%d's downlink-bandwidth is out of Max-Bandwidth-Limit.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
                    }
                }

                if ( PonActUpBW < PonMaxBW )
				{
					if ( pOnuCfg->BandWidthIsDefault & OLT_CFG_DIR_UPLINK )
					{
						GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &def_up_bw_gr, NULL);
						PonActUpBW += def_up_bw_gr;
					}
					else
					{
						PonActUpBW += pOnuCfg->ActiveUplinkBandwidth + pOnuCfg->FinalUplinkBandwidth_fixed;
					}

					if ( PonActUpBW > PonMaxBW )
					{                   
						if ( vty != NULL )
							vty_out(vty, "  warning:PON%d/%d's uplink-bandwidth is out of Max-Bandwidth-Limit.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));

#if 1
						/* 上行带宽超限，则后续的ONU无需再设置新的默认上行带宽 */
						if ( 0 == (SetDir &= ~OLT_CFG_DIR_UPLINK) )
						{
							break;
						}
#else
						/* 上行带宽超限，仅仅提示，则后续的ONU仍需再设置新的默认上行带宽(修改分配带宽) */
#endif
					}
				}
#endif
            }            
        }         
    }

    return( ROK );
}
#endif
/* E--added by liwei056@2011-1-26 for D11957 */


#ifdef V2R1_WATCH_DOG

#define WDOGON 0
#define WDOGOFF 0x1
#define DOG_EN 0x7
int WDI= 0x6;
typedef int bit; 

extern void write_gpio(int GPIOx,bit a);
extern bit read_gpio(int GPIOx);

extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern void WriteCPLDReg( unsigned char * RegAddr, unsigned char ucIntData );
#if 0
int feedWatchdog()
{
bit a;
while(1)
{
	a = !read_gpio(WDI);
	write_gpio(WDI,a);
	VOS_TaskDelay(50);
}
return 1;
}

int test_wdog(int onoff)
{
int sel_dog;
sel_dog=onoff;
write_gpio(DOG_EN,WDOGOFF);
write_gpio(WDI,1);
if (sel_dog==0)
	taskSpawn ("watchdog", 20, 0, 2000, (FUNCPTR)feedWatchdog, 0,0,0,0,0,0,0,0,0,0);
write_gpio(DOG_EN,sel_dog);
return 1;
}
#endif

static unsigned char watchdog_turnon = 0;

/* modified by xieshl 20090212, 看门狗使能默认不启动，只能通过命令启动，但不影响喂狗和disable，
    看门狗操作涉及全局变量修改，为了防止冲突，加了信号量互斥 */
void V2R1_enable_watchdog(void)
{
	int Product_type = GetOltType();
	VOS_SemTake( OltMgmtDataSemId, WAIT_FOREVER );
	if( watchdog_turnon > 1 )
		watchdog_turnon --;
	else
	{
		watchdog_turnon = 0;
		if( (V2R1_watchdog_init_flag == V2R1_ENABLE) && (V2R1_watchdog_startup_flag == V2R1_ENABLE) )	
		{
			V2R1_feed_watchdog();
			if(Product_type == V2R1_OLT_GFA6700)
				write_gpio(DOG_EN,WDOGON);
			else if(Product_type == V2R1_OLT_GFA6100)
			{
				unsigned char data;
				ReadCPLDReg((unsigned char *)4, &data);
				data &= 0xfb;
				WriteCPLDReg((unsigned char *)4,data);
			}
		}
	}
	/*sys_console_printf(" watchdog enable\r\n" );*/
	VOS_SemGive( OltMgmtDataSemId );
	return;
}

void V2R1_disable_watchdog(void)
{
	int Product_type = GetOltType();
	VOS_SemTake( OltMgmtDataSemId, WAIT_FOREVER );

	watchdog_turnon ++;
	if( V2R1_watchdog_init_flag == V2R1_ENABLE )
	{
		if(Product_type == V2R1_OLT_GFA6700)
			write_gpio(DOG_EN,WDOGOFF);
		else if(Product_type == V2R1_OLT_GFA6100)
		{
			unsigned char data;
			ReadCPLDReg((unsigned char *)4, &data);
			data |= 0x4;
			WriteCPLDReg((unsigned char *)4,data);
		}
	}
	/*sys_console_printf(" watchdog disable\r\n" );*/

	VOS_SemGive( OltMgmtDataSemId );
	return;
}

void V2R1_disable_watchdog_forced(void)
{
	int Product_type = GetOltType();
	VOS_SemTake( OltMgmtDataSemId, WAIT_FOREVER );
	watchdog_turnon = 0;
	if( V2R1_watchdog_init_flag == V2R1_ENABLE )
	{
		if(Product_type == V2R1_OLT_GFA6700)
			write_gpio(DOG_EN,WDOGOFF);
		else if(Product_type == V2R1_OLT_GFA6100)
		{
			unsigned char data;
			ReadCPLDReg((unsigned char *)4, &data);
			data |= 0x4;
			WriteCPLDReg((unsigned char *)4,data);
		}
	}
	VOS_SemGive( OltMgmtDataSemId );
}

void V2R1_feed_watchdog(void)
{
	bit a;
	if(V2R1_watchdog_init_flag == V2R1_ENABLE)
		{
		a = !read_gpio(WDI);
		write_gpio(WDI,a);
		}
	return;
}

void V2R1_init_watchdog(void)
{	
	int Product_type = GetOltType();
	if(Product_type == V2R1_OLT_GFA6700)
		write_gpio(DOG_EN,WDOGOFF);
	else  if(Product_type == V2R1_OLT_GFA6100)
		{
		unsigned char data;
		ReadCPLDReg((unsigned char *)4, &data);
		data |= 0x4;
		WriteCPLDReg((unsigned char *)4,data);
		}
	write_gpio(WDI,1);
	V2R1_watchdog_init_flag = V2R1_ENABLE;
	watchdog_turnon = 0;
	return;
}
#endif

int GetDeviceTypeLength()
{
	int length = 0;
	int len1 = 0;
	int i;
	int defaultLength = VOS_StrLen("GT831_CATV_A"); /* modified by xieshl 20100207 */

	for( i = V2R1_DEVICE_UNKNOWN; i< V2R1_DEVICE_LAST; i++)
	{
		len1 = VOS_StrLen(GetDeviceTypeString(i));
		if(len1 > length )
			length = len1;
	 }
	if(length < defaultLength)  length = defaultLength;
	return( length );
}
/*
int  ConvertCharCase( unsigned char *CharString)
{
	int length;
	int i;
	
	if(CharString == NULL ) return( ROK );

	length = VOS_StrLen(CharString);
	if(length != 0 )
		{
		if(( CharString[0] == 'g') || ( CharString[0] == 'G'))
			{
			for( i=0; i< length; i++)
				CharString[i] = VOS_ToUpper(CharString[i]);
			}
		else if(( CharString[0] == 'o') || ( CharString[0] == 'O'))
			{
			for( i=0; i< length; i++)
				CharString[i] = VOS_ToLower(CharString[i]);
			}
		}
	return(ROK);
}
*/

void  DoubleDataPrintf(double *Data, unsigned int precision, struct vty *vty)
{

	int  integer_fraction;
	int  decimal_fraction;
	double  decimal;
	int multiple;
	int i;
		
	integer_fraction = *Data;
	if( *Data >= 0 )
		decimal = *Data - integer_fraction;
	else  decimal = integer_fraction - (*Data);
	
	multiple = 1;
	for(i=0; i< precision; i++)
		multiple = multiple * 10;
	decimal_fraction = (int)((float)multiple*decimal);
	
	if( precision ==0 )
	{
		/*if(vty == NULL)
			sys_console_printf("%d.0", integer_fraction);
		else 
			vty_out(vty,"%d.0", integer_fraction);*/
		v2r1_printf(vty,"%d.0", integer_fraction);
	}
	else
	{
		/*if(vty == NULL)
			sys_console_printf("%d.%d", integer_fraction,decimal_fraction);
		else 
			vty_out(vty,"%d.%d", integer_fraction,decimal_fraction);*/
		v2r1_printf(vty,"%d.%d", integer_fraction,decimal_fraction);
	}
	return ;
}

#if 0
int TestFlag = V2R1_DISABLE;
void ReadPonPortVer(short int PonPortIdx,unsigned long arg2,unsigned long arg3,unsigned longarg4,unsigned long arg5,unsigned long arg6,unsigned long arg7,unsigned long arg8,unsigned long arg9,unsigned long arg10)
{
	PAS_device_versions_t  device_version;
	short int OnuIdx;
	while (TestFlag == V2R1_ENABLE )
		{		
		PAS_get_olt_versions(PonPortIdx, &device_version);
		GetDBAInfo(PonPortIdx);
		StatsRealTimePonRawGet(PonPortIdx);
		for(OnuIdx = 0; OnuIdx< MAXONUPERPON; OnuIdx++)
			{
			if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP)
				StatsRealTimeOnuRawGet(PonPortIdx, OnuIdx);
			}
		}	
}

extern STATUS StatsRealTimePonRawGet(short int PonPortIdx);
extern VOID check_oam_frame_send_by_port( ULONG slotno, ULONG portno, USHORT vid );
void ReadPonPortVer1(void)
{
	short int PonPortIdx;
	short int OnuIdx;
	PAS_device_versions_t  device_version;
	for(;;)
		{
		for(PonPortIdx=0;PonPortIdx<MAXPON;PonPortIdx++)
			{
			if(TestFlag == V2R1_DISABLE) return;
			if(Olt_exists(PonPortIdx) )
				{		
				PAS_get_olt_versions(PonPortIdx, &device_version);
				GetDBAInfo(PonPortIdx);
				StatsRealTimePonRawGet(PonPortIdx);
				for(OnuIdx = 0; OnuIdx< MAXONUPERPON; OnuIdx++)
					{
					if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP)
						StatsRealTimeOnuRawGet(PonPortIdx, OnuIdx);
					}
				/*
				for(OnuIdx=50;OnuIdx<100;OnuIdx++)
					check_oam_frame_send_by_port( GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx );
				VOS_TaskDelay(VOS_TICK_SECOND/10);*/
				}
			}		
		}
}
#endif



#ifdef __cplusplus

}

#endif
