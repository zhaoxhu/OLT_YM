/***************************************************************
*
*						Module Name: gwEponMibData.c
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
*   Date: 			2006/06/20
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/06/20 |   chenfj          |     create 
**----------|-----------|------------------
**
** 注:  所有函数类型均为int, 返回值为ROK(0) , 或者RERROR(-1); 
**         当返回值为ROK时，返回的参数才有效
***************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "PonEventHandler.h"
#include  "gwEponMibData.h"

#include "gwEponSys.h"
#include "lib_gwEponOnuMib.h"
#include "../../superset/platform/sys/devsm/devsm_remote.h"

extern short int Remove_olt ( const short int  olt_id,
					   const bool		send_shutdown_msg_to_olt,
					   const bool       reset_olt);

extern  int ResetOnu( short int PonPortIdx, short int OnuIdx );
extern  int ResetOlt();

extern int CTC_deviceFirmWareVersion( short int PonPortIdx, short int OnuIdx,  char *value, int *len );
extern int CTC_deviceSoftWareVersion  ( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len );
extern int CTC_deviceHardWareVersion( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len );

extern ULONG parseDevidxFromPonOnu( const short int ponIdx, const short int onuIdx );
extern LONG getModuleHardwareVersion( ULONG slotno, char * version );
extern int GetOnuMacCheckEnable(ULONG  *enable);
extern int OnuEvent_Active_UplinkRunningBandWidth(short int PonPortIdx, short int OnuIdx);
extern int OnuEvent_Active_DownlinkRunningBandWidth(short int PonPortIdx, short int OnuIdx);
#ifdef DEVICE_MGMT
#endif

/**************************************************************
 *
 *    Function:  DeviceTypeGet( DeviceIndex_S  DevIdx,  int *type )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *type -- output, is a pointer
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int MIB_DEBUG = V2R1_DISABLE;
#define OLT_MIB_DEBUG if(MIB_DEBUG == V2R1_ENABLE) sys_console_printf;

int  DeviceTypeGet( DeviceIndex_S  DevIdx,  int *type )
{
	short int  slot, port, onuId;
	
	if( type == NULL ) { return( RERROR ); }
	
	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding type */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		if(GetOnuType( PonPortIdx, OnuIdx, type )== ROK )
		{
			return ROK;

			/* modified by chenfj 2008-5-30
			     mib中已扩充GT811_A, GT812_A, GT831_A, GT831_A_CATV 等ONU类型
			     在此取消类型变换
			switch (*type )
				{
				case V2R1_ONU_GT811_A:
					*type = V2R1_ONU_GT811;
					break;
				case V2R1_ONU_GT812_A:
					*type = V2R1_ONU_GT812;
					break;
				case V2R1_ONU_GT831_A:
					*type = V2R1_ONU_GT831;
					break;
				case V2R1_ONU_GT831_A_CATV:
					*type = V2R1_ONU_GT831_CATV;
					break;
				default :
					break;
				}
			*/
		}
		else{ return( RERROR ); }
	}
	else{ /* get olt device type */
		/*if( GetOltType( type) == ROK ) { return( ROK ); }
		else { return( RERROR ); }*/
		*type = GetOltType();
		if(V2R1_PRODUCT_IS_8100Series(*type))
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			{*type = V2R1_OLT_GFA8100;}
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{*type = V2R1_OLT_GFA8100GP;}
		}
	}
	return( ROK ); 
}

/**************************************************************
 *
 *    Function:  DeviceNameGet( DeviceIndex_S  DevIdx,  char *Name, int *NameLen )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *Name -- output, pointer
 *               int *NameLen -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceNameGet(DeviceIndex_S  DevIdx,  char *Name, int *NameLen )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if( (Name == NULL ) || ( NameLen == NULL )){ return( RERROR ); } 
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		/*CHECK_ONU_RANGE*/
		if( OLT_LOCAL_ISVALID(PonPortIdx) && ONU_IDX_ISVALID(OnuIdx) )
			rc = GetOnuDeviceName( PonPortIdx, OnuIdx, Name, NameLen);
		else
		{
			/*sys_console_printf("onu %d/%d/%d\r\n", slot, port, onuId);
			VOS_ASSERT(0);*/
			VOS_StrCpy( Name, "Unkown" );
			*NameLen = VOS_StrLen(Name);
		}
	}
	else
	{ /* get olt device info */
		rc = GetOltDeviceName( Name, NameLen );
	}
	return rc;
}

int DeviceNameSet(DeviceIndex_S  DevIdx,  char *Name, int NameLen )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if( Name == NULL){ return( RERROR ); }
	
	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( ( short int)slot, (short int )port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = SetOnuDeviceName( PonPortIdx, OnuIdx, Name, NameLen);
	}
	
	else
	{ /* get olt device info */
		rc = SetOltDeviceName( Name, NameLen );
	}
	return rc;
}


 /**************************************************************
 *
 *    Function:  DeviceDescGet( DeviceIndex_S  DevIdx,  char *Desc, int *Len )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *Desc -- output, pointer
 *               int *Len -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceDescGet(DeviceIndex_S  DevIdx,  char *Desc, int *Len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if((Desc == NULL ) || ( Len == NULL )){ return( RERROR ); }
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if ( Len == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = GetOnuDeviceDesc( PonPortIdx, OnuIdx, Desc, Len);
	}
	else
	{ /* get olt device info */
		rc = GetOltDeviceDesc( Desc, Len );
	}
	return rc;
}

int DeviceDescSet(DeviceIndex_S  DevIdx,  char *Desc, int len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if(Desc == NULL ){ return( RERROR ); }
	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( ( short int)slot, (short int )port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = SetOnuDeviceDesc(PonPortIdx, OnuIdx, Desc, len);
	}
	
	else
	{ /* get olt device info */
		rc = SetOltDeviceDesc( Desc, len );
	}
	return rc;
}

 /**************************************************************
 *
 *    Function:  DeviceLocationGet( DeviceIndex_S  DevIdx,  char *location, int *Len )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *location -- output, pointer
 *               int *Len -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceLocationGet( DeviceIndex_S  DevIdx,  char *location, int *Len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if(( location == NULL )||( Len == NULL )) { return( RERROR ); }
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = GetOnuLocation( PonPortIdx, OnuIdx, location, Len);
	}
	else
	{ /* get olt device info */
		rc = GetOltLocation( location, Len );
	}
	return rc;
}

int DeviceLocationSet(DeviceIndex_S  DevIdx,  char *location, int len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if( location == NULL ){ return( RERROR ); }
	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( ( short int)slot, (short int )port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = SetOnuLocation( PonPortIdx, OnuIdx, location, len);
	}
	else
	{ /* get olt device info */
		rc = SetOltLocation( location, len );
	}
	return rc;
}


/**************************************************************
 *
 *    Function:  DeviceVendorGet( DeviceIndex_S  DevIdx,  char *Info, int *InfoLen )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *Info -- output, pointer
 *               int *InfoLen -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceVendorGet(DeviceIndex_S  DevIdx,  char *Info, int *InfoLen )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if((Info == NULL ) ||( InfoLen == NULL)) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;
		
		CHECK_ONU_RANGE
			
		rc = GetOnuVendorInfo( PonPortIdx, OnuIdx, Info, InfoLen);
	}
	else
	{ /* get olt device info */
		rc = GetOltVendorInfo( Info, InfoLen );
	}
	return rc;
}


 /**************************************************************
 *
 *    Function:  DeviceFrimwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *version -- output, pointer
 *               int *len -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceFrimwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if(( version == NULL ) ||( len == NULL )) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;
		
		CHECK_ONU_RANGE
        /*del by luh 2012-11-1*/            
#if 0		
		/****Added by suipl 2007/03/23******/
		if( GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
		{
			/* modified by zhuyf 2007/03/27, CTC 的firmware version 是short int 类型，改为字符串型 */
			rc = CTC_deviceFirmWareVersion( PonPortIdx, OnuIdx, version, len );
		}
		else
#endif            
			rc = GetOnuFWVersion( PonPortIdx, OnuIdx, version, len);
	}
	else
	{ /* get olt device info */
		/*if( GetOltFWVersion( version,  len ) == ROK ) { return( ROK ); }*/
		rc = GetPonChipFWVersion(version, len);
	}
	return rc;
}


int DeviceSoftwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if((version==NULL)||(len ==NULL)) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
        /*moved by luh 2013-4-27*/
#if 0
		/****Added by suipl 2007/03/23******/
		if( GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
		{
			rc = CTC_deviceSoftWareVersion( PonPortIdx, OnuIdx, version, (ULONG *)len );
		}
		else	
#endif            
			rc = GetOnuSWVersion( PonPortIdx, OnuIdx, version, len);
	}
	else
	{ /* get olt device info */
		rc = GetOltSWVersion( version,  len );
	}
	return rc;
}


int DeviceHardwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if((version==NULL)||(len ==NULL)) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
        /*del by luh 2012-11-1*/                        
#if 0			
		/****Added by suipl 2007/03/23******/
		if( GetOnuVendorType(PonPortIdx, OnuIdx) == ONU_VENDOR_CT )
		{
			rc = CTC_deviceHardWareVersion( PonPortIdx, OnuIdx, version, (ULONG *)len );
		}
		else
#endif            
			rc = GetOnuHWVersion( PonPortIdx, OnuIdx, version, len);
	}
	else
	{ /* get olt device info */
		/*if( GetOltHWVersion( version,  len ) == ROK ) { return( ROK ); }
		else { return( RERROR ); }*/
		*len = getModuleHardwareVersion( SYS_LOCAL_MODULE_SLOTNO, version );	/* modified by xieshl 20080402 */
		rc = ROK;
	}
	return rc;
}

/**************************************************************
 *
 *    Function:  DeviceOperStatusGet( DeviceIndex_S  DevIdx,  int *OperStatus )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *OperStaus -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceOperStatusGet( DeviceIndex_S  DevIdx,  int *OperStatus )
{
	short int  slot, port, onuId;
	/*int rc = RERROR;*/

	if( OperStatus == NULL ) return( RERROR );
	*OperStatus = ONU_OPER_STATUS_DOWN;
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE

		/* modified by chenfj 2008-2-20
		     使用扩展的ONU操作状态
		    */
		if(( *OperStatus =  GetOnuOperStatus_Ext( PonPortIdx, OnuIdx ))  == RERROR ) { return ( RERROR) ; }
	}
	
	else{ /* get olt device info */		
		*OperStatus =  GetOltOperStatus() ;
		}
	 return( ROK );
}

/* added 20070705 */
int DeviceAlarmMaskGet(DeviceIndex_S  DevIdx, unsigned long *pMask )
{
	short int  slot, port, onuId;

	if( pMask == NULL )
		return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE;
		
		return GetOnuAlarmMask( PonPortIdx, OnuIdx, pMask);
	}
	else
	{ /* get olt device info */
		return GetOltAlarmMask( pMask );
	}
	return( RERROR );
}

int DeviceAlarmMaskSet(DeviceIndex_S  DevIdx,  unsigned long mask )
{
	short int  slot, port, onuId;

	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( ( short int)slot, (short int )port );
		OnuIdx = onuId - 1;

		/*  modified by xieshl 20110615, 直接返回错误即可，不需要断言，问题单12882  */
		/*CHECK_ONU_RANGE;*/
		if( OLT_LOCAL_ISVALID(PonPortIdx) && ONU_IDX_ISVALID(OnuIdx) )
			return SetOnuAlarmMask( PonPortIdx, OnuIdx, mask );
	}
	else
	{ /* get olt device info */
		return SetOltAlarmMask( mask );
	}
	return ERROR;
}

extern int get_gfasw_sys_up_time( sysDateAndTime_t *pTime );
int DeviceUpDateTimeGet(DeviceIndex_S  DevIdx, sysDateAndTime_t *pTime )
{
	short int  slot, port, onuId;

	if( pTime == NULL )
		return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		/*CHECK_ONU_RANGE*/
		return GetOnuLaunchTime(PonPortIdx, OnuIdx, (Date_S*)pTime);
	}
	else /* get olt device info---up date and time */
	{
		return get_gfasw_sys_up_time( pTime );
	}
	return( RERROR );
}

#if 0
int DeviceUpDateTimeSet(DeviceIndex_S  DevIdx,  sysDateAndTime_t *pTime )
{
	short int  slot, port, onuId;

	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( ( short int)slot, (short int )port );
		OnuIdx = onuId;

		/*CHECK_ONU_RANGE*/
		return GetOnuLaunchTime( PonPortIdx, OnuIdx, pTime );
	}
	else
	{ /* get olt device info */
		return ROK;
	}
	return RERROR;
}
#endif
/**************************************************************
 *
 *    Function:  DeviceAlarmStatusGet( DeviceIndex_S  DevIdx,  int *AlarmStaus )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *AlarmStaus -- output, ponter 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0	/* removed 20070705 */
int DeviceAlarmStatusGet( DeviceIndex_S  DevIdx,  int *AlarmStaus )
{
	short int  slot, port, onuId;

	if(AlarmStaus == NULL ) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId;

		CHECK_ONU_RANGE
		
		if( GetOnuCurrentAlarmRank( PonPortIdx, OnuIdx,(int *) AlarmStaus) == ROK ) { return ( ROK ) ; }
		else { return( RERROR ); }
		}
	
	else{ /* get olt device info */		
		*AlarmStaus =  GetOltCurrentAlarmRank() ;
		 return( ROK );
		}



}
#endif

/**************************************************************
 *
 *    Function:  DeviceMacAddrGet( DeviceIndex_S  DevIdx,  char *MacAddr, int *len )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               char *MacAddr -- output, ponter 
 *               int *len -- output , pointer
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceMacAddrGet( DeviceIndex_S  DevIdx,  char *MacAddr, int *len )
{
	short int  slot, port, onuId;
	int rc = RERROR;

	if((MacAddr == NULL ) ||( len == NULL)) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		rc = GetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr, len );
	}
	else
	{ /* get olt device info */
		rc = GetOltMgmtMAC( MacAddr, len );
	}
	return rc;
}


int DeviceMacAddrSet( DeviceIndex_S  DevIdx,  char *MacAddr, int len )
{
	short int  slot, port, onuId;

	if(MacAddr == NULL )return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu corresponding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE

		if( GetOnuOperStatus( PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
			/*if( AddOnuToPonPort(PonPortIdx,  OnuIdx,  MacAddr) == ROK ) { return( ROK ); }*/
			if( SetOnuMacAddr( PonPortIdx, OnuIdx, MacAddr, len ) == ROK )
				return ( ROK ) ;
		}
	}
	
	else
	{ /* get olt device info */
		if( SetOltMgmtMacAddr( MacAddr, len ) == ROK ){  return( ROK ); }
	}

	return( RERROR );
}

/**************************************************************
 *
 *    Function:  DeviceLastChangeGet( DeviceIndex_S  DevIdx,  int *LastChange )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *LastChange -- output , pointer
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceLastChangeGet( DeviceIndex_S  DevIdx,  int *LastChange )
{
	short int  slot, port, onuId;

	if( LastChange == NULL )  return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		if(( *LastChange = GetOnuUpLastTime ( PonPortIdx, OnuIdx )) == RERROR ) { return ( RERROR ) ; }
		else { return( ROK ); }
		}
	
	else{ /* get olt device info */
		if(( *LastChange = GetOltSysUpTime( )) == RERROR ){  return( RERROR ); }
		else { return( ROK); }
		}

	return( RERROR );

}

int DeviceUpTimeGet( DeviceIndex_S  DevIdx,  int *Uptime )
{
	short int  slot, port, onuId;

	if( Uptime == NULL )  return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE

		if( GetOnuOperStatus(PonPortIdx, OnuIdx )  != ONU_OPER_STATUS_UP )  
		{
    		*Uptime = 0;
    		return( ROK );
		}
#if 0
		if(( *Uptime = GetOnuUpTime ( PonPortIdx, OnuIdx )) == RERROR ) 
           return RERROR;
		else
            return ROK ;
#else
		if(( *Uptime = GetOnuUpLastTime ( PonPortIdx, OnuIdx )) == RERROR )
            return RERROR; 
        else 
            return ROK;
#endif
		}
	else return( RERROR );
}

/**************************************************************
 *
 *    Function:  DeviceResetSet( DeviceIndex_S  DevIdx,  int Reset )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *Reset -- intput 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceResetSet( DeviceIndex_S  DevIdx,  int Reset )
{
	short int  slot, port, onuId;

	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
			
		if( Reset ==  V2R1_OPERATION_NO )
			{ /* nothing is need to do */
			return( ROK );
			}
		else {
			/*return(ResetOnu(PonPortIdx, OnuIdx ));*/
			return(OnuMgt_ResetOnu(PonPortIdx, OnuIdx )); /*modfied by shixh20101110 for mib*/
			}		
		}
	
	else{ /* get olt device info */
		if( Reset == V2R1_OPERATION_NO ){ /* nothing is need to do */
			return( ROK );
			}
		else{
			return( ResetOlt());
			}
		}



}

int DeviceResetGet( DeviceIndex_S  DevIdx,  int *Reset )
{
	short int  slot, port, onuId;

	if( Reset == NULL ) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		/*short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId;
		CHECK_ONU_RANGE
		*/
		*Reset = V2R1_OPERATION_NO; /*OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].*/
		}
	
	else{ /* get olt device info */
		*Reset = V2R1_OPERATION_NO;
		}
	return( ROK );
}

/**************************************************************
 *
 *    Function:  DeviceOnuTdGet( DeviceIndex_S  DevIdx,  int *Td )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *               int *Td -- output 
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int DeviceOnuTdGet( DeviceIndex_S  DevIdx,  int *Td )
{
	short int  slot, port, onuId;
	int  t = 0;
	
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if( Td == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;
		CHECK_ONU_RANGE
			
		/*if (( *Td = GetOnuDistance( PonPortIdx, OnuIdx )) == RERROR )*/
		
		
		if ((  OnuMgt_GetOnuDistance(PonPortIdx,OnuIdx, &t)) == ROK )	
			*Td=t;
		if(*Td==RERROR)
			{ return ( RERROR ); }
		else 
			{ return ( ROK ); }
		
		}
	
	else{ /* get olt device info */
		return( RERROR );
		}



}

/* added by xieshl 20080807, 增加ONU型号，以满足互通测试时CTC ONU型号识别 */
int DeviceModelGet( DeviceIndex_S  DevIdx,  char *pModelStr, int *Len )
{
	short int  slot, port, onuId;

	if(( pModelStr == NULL )||( Len == NULL )) { return( RERROR ); }
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
	{ /* get onu correspongding info */
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId - 1;

		CHECK_ONU_RANGE
		
		if(GetOnuModel( PonPortIdx, OnuIdx, pModelStr, Len) == ROK ) { return( ROK ); }
		else { return ( RERROR ); }
	}
	
	else{ /* get olt device info */
		/* added by chenfj 2008-11-26
		     增加对6100 的支持*/ /* modified by xieshl 20081226 */
		int DeviceType = GetOltType();
		/*if( DeviceType == V2R1_OLT_GFA6700)
			{
			VOS_StrCpy( pModelStr, DEVICE_TYPE_NAME_GFA6700_STR );
			*Len = VOS_StrLen(DEVICE_TYPE_NAME_GFA6700_STR);
			}
		else if(DeviceType == V2R1_OLT_GFA6100 )
			{
			VOS_StrCpy( pModelStr, DEVICE_TYPE_NAME_GFA6100_STR );
			*Len = VOS_StrLen(DEVICE_TYPE_NAME_GFA6100_STR);
			}*/

		if( V2R1_PRODUCT_IS_EPON(DeviceType) )
		{
			VOS_StrCpy( pModelStr,GetDeviceTypeString(DeviceType));
			*Len = VOS_StrLen(pModelStr/*GetDeviceTypeString(DeviceType)*/); /* modified by xieshl 20100207 */
		}
		else
			*Len = 0;
		return( ROK );
	}
}



#ifdef PON_PORT_FUNC
#endif


/**************************************************************
 *
 *    Function:  PonPortPartnerDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  PonPortPartnerDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev )
{
	short int  slot, port, onuId;

	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if( Dev == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu pon port partner device */
		/*
		short int PonPortIdx; 
		short int OnuIdx;

		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		OnuIdx = onuId-1;
		CHECK_ONU_RANGE
		*/
		*Dev = 1;
		}
	
	else{ /* get olt pon port partner device */
		*Dev = 0;
		}
	 return ( ROK );  
}

int  PonPortPartnerBrdGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Brd )
{
	short int  slot, port, onuId;

	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if( Brd == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu pon port partner board */

		/*if(( *Brd = GetCardIdxByPonChip( slot*MAXPONPORTPERCARD +port )) == RERROR ) { return( RERROR ); }
		else { return ( ROK );  }*/
		*Brd = slot;
		}
	
	else{ /* get olt pon port partner board */
		*Brd = 0;
		}
	return( ROK );
}

int  PonPortPartnerPortGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *PonIdx )
{
	short int  slot, port, onuId;

	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;

	if( PonIdx == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) ))
		{ /* get onu pon port partner board */

		/*if(( *PonIdx = GetPonPortIdxBySlot( slot, port )) == RERROR ) { return( RERROR ); }
		else { return ( ROK );  }*/
		*PonIdx = port;
		}
	
	else{ /* get olt pon port partner board */
		*PonIdx = 0;
		}
	return ( ROK );
}


/**************************************************************
 *
 *    Function:  PonPortTypeGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *type )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get the pon port type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 int PonPortTypeGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *type )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;
	int result;
	
	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( type == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
		{ /* get onu pon port type */

		PonPortIdx = GetPonPortIdxBySlot( slot, port);
		CHECK_ONU_RANGE

		result = GetOnuPonType( PonPortIdx, OnuIdx, type) ;
		
		if( result == RERROR ) return( RERROR );
		/*if( OnuMgmtTable[PonPortIdx*MAXONUPERPON+onuId].OperStatus !=  ONU_OPER_STATUS_UP ) return( RERROR );*/
		/**type = result;  wxy modify*/
		return( ROK );
		}
	else{ /* get olt pon port type */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE

		result = GetPonPortType( PonPortIdx );
		
		if( result != RERROR ) {
			*type = result;
			return( ROK );
			}
		}
	return( RERROR ); 
}

/**************************************************************
 *
 *    Function:  PonPortMaxOnuGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *type )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  The max number of supported ONUs in this PON port;
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int PonPortMaxOnuGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OnuNum )
{
	short int  slot, port, onuId;
	short int PonPortIdx;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	onuId = DevIdx.onuId;

	if( OnuNum == NULL ) return( RERROR );
	 *OnuNum = 0;
			   
	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )){ /*  */
		*OnuNum = 1;
		return( ROK );
		}
	else{ /* get olt pon port Max onu number supported */		
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx);
		CHECK_PON_RANGE

		result = GetPonPortMaxOnuNum( PonPortIdx );
		
		if( result  == RERROR ) return( RERROR);
		*OnuNum = result;
		return( ROK );
		}

}


int PonPortCurrOnuGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OnuNum )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( OnuNum == NULL ) return( RERROR );
	*OnuNum = 0;
	
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /*  */
	       *OnuNum = 0;
		return( ROK );
		}
	else{ /* get olt pon port Max onu number supported */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx);
		CHECK_PON_RANGE

		result = GetPonPortCurrentOnuNum(PonPortIdx );
		if( result == RERROR )return( RERROR );
		*OnuNum = result;
		return( ROK );
		/*
		if( PonPortIdx != RERROR ) {
			
			if( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP ){
				*OnuNum = PonPortTable[PonPortIdx].CurrOnu;
				return( ROK );
				}
			else{
				return( RERROR ); 
				}
			}
		else{ return( RERROR ); } */
		}

	
}

/**************************************************************
 *
 *    Function:  PonPortOperStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OperStatus )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get PON port operation status ;
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 int PonPortOperStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OperStatus )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(OperStatus == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		CHECK_ONU_RANGE
		result = GetOnuOperStatus( PonPortIdx, OnuIdx );
		
		if( result == RERROR ) return( RERROR );
		/*if( OnuMgmtTable[PonPortIdx*MAXONUPERPON + onuId].OperStatus == ONU_OPER_STATUS_UP ) 
		if( result == ONU_OPER_STATUS_PENDING ) result = ONU_OPER_STATUS_DOWN;*/
	       * OperStatus= result;
		return( ROK );
		}
	else{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = GetPonPortOperStatus(PonPortIdx );
		if( result == RERROR ) return( RERROR );
		*OperStatus = result ;
		return( ROK );
		/*	
		if( PonPortIdx != RERROR ) {
			*OperStatus = PonPortTable[PonPortIdx].PortWorkingStatus;
			return( ROK );			
			}
		else{ return( RERROR ); } */
		}


}

int PonPortAdminStatusSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int AdminStatus )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		CHECK_ONU_RANGE		
		/*if( result == RERROR ) return( RERROR );*/
		return( ROK );
		}
	else{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = SetPonPortAdminStatus(PonPortIdx, AdminStatus );
		if( result == RERROR ) return( RERROR );
		return( ROK );
		/*	
		if( PonPortIdx != RERROR ) {
			*OperStatus = PonPortTable[PonPortIdx].PortWorkingStatus;
			return( ROK );			
			}
		else{ return( RERROR ); } */
		}


}

int PonPortAdminStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *AdminStatus )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(AdminStatus == NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		CHECK_ONU_RANGE
		result = GetOnuOperStatus( PonPortIdx, OnuIdx );
		
		if( result == RERROR ) return( RERROR );
		/*if( OnuMgmtTable[PonPortIdx*MAXONUPERPON + onuId].OperStatus == ONU_OPER_STATUS_UP )
		if( result == ONU_OPER_STATUS_PENDING ) result = ONU_OPER_STATUS_DOWN; */
	       * AdminStatus= result;
		return( ROK );
		}
	else{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = GetPonPortAdminStatus(PonPortIdx );
		if( result == RERROR ) return( RERROR );
		*AdminStatus = result ;
		return( ROK );
		/*	
		if( PonPortIdx != RERROR ) {
			*OperStatus = PonPortTable[PonPortIdx].PortWorkingStatus;
			return( ROK );			
			}
		else{ return( RERROR ); } */
		}


}

int PonPortReset( DeviceIndex_S DevIdx, int brdIdx, int  portIdx )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		return( ROK );
	}
	else
	{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE;

		ResetPonPort( PonPortIdx );

		return( ROK );
		/*	
			if( PonPortIdx != RERROR ) {
		 *OperStatus = PonPortTable[PonPortIdx].PortWorkingStatus;
		 return( ROK );			
		 }
		 else{ return( RERROR ); } */
	}

}



/**************************************************************
 *
 *    Function:  PonPortAlarmStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *AlarmStatus )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get PON port alarm status ;
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int PonPortAlarmStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long *AlarmStatus )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( AlarmStatus == NULL ) return( RERROR );
	*AlarmStatus = 0;
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) )){ /* Onu pon port */
		PonPortIdx = GetPonPortIdxBySlot( slot, port);
		CHECK_ONU_RANGE
		return( GetOnuCurrentAlarm( PonPortIdx, OnuIdx, AlarmStatus ) );
		}
	else{ /* get olt pon port */
		unsigned long AlarmNum;
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx);
		CHECK_PON_RANGE
		return( GetPonPortCurrentAlarmStatus( PonPortIdx ,  &AlarmNum,  AlarmStatus) );
		}


}

int PonPortAlarmMaskGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long *AlarmMask)
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( AlarmMask == NULL ) return( RERROR );
	*AlarmMask = 0;
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port);
		CHECK_ONU_RANGE
		if( PonPortIdx == RERROR ) return( RERROR );
		
		ONU_MGMT_SEM_TAKE;
		*AlarmMask = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].AlarmMask;
		ONU_MGMT_SEM_GIVE;
	}
	else
	{ /* get olt pon port */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		if( PonPortIdx == RERROR )  return( RERROR );
		/*return GetOltPonPortAlarmMask(PonPortIdx, AlarmMask );*/
		*AlarmMask = PonPortTable[PonPortIdx].AlarmMask;
	}
	return( ROK );
}


/* this function need to be modified later */
int PonPortAlarmMaskSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long AlarmMask)
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{ /* Onu pon port */
		PonPortIdx = GetPonPortIdxBySlot( slot, port);
		CHECK_ONU_RANGE;

		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].AlarmMask = AlarmMask;
		ONU_MGMT_SEM_GIVE;
	}
	else
	{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		/*  modified by xieshl 20110615, 直接返回错误即可，不需要断言，问题单12882  */
		/*CHECK_PON_RANGE;*/
		if( OLT_LOCAL_ISVALID(PonPortIdx) )
			PonPortTable[PonPortIdx].AlarmMask = AlarmMask;
		else
			return( RERROR );
	}
	return( ROK );
}

/**************************************************************
 *
 *    Function:  PonPortMaxBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* MaxBw )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get PON port Max bandwidth supported
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int PonPortMaxBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* MaxBw )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( MaxBw == NULL ) return( RERROR );
	*MaxBw = 0;
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port  */
		
		return( ROK );
		}
	else{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = GetPonPortMaxBw(PonPortIdx );
		if( result  == RERROR )  return( RERROR );
		*MaxBw  = result ;
		return( ROK );
		}


	
}

int PonPortActBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* ActBw )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( ActBw == NULL ) return( RERROR );
	*ActBw = 0;
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port */
		
		return( ROK );
		}
	else{ /* get olt pon port */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = GetPonPortProvisionedBw( PonPortIdx );
		if( result  == RERROR )  return( RERROR );
		*ActBw  = result;
		return( ROK );
		}


	
}

int PonPortRemainBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* RemainBw )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( RemainBw == NULL ) return( RERROR );
	*RemainBw =0;
	
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0 ) )){ /* Onu pon port  */
		return( ROK );
		}
	else{ /* get olt pon port */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		result = GetPonPortRemainBW(PonPortIdx);
		if( result == RERROR )  return( RERROR );
		*RemainBw  = result;
		return( ROK );
		}
	
}


/**************************************************************
 *
 *    Function:  PonPortProtectDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  PonPortProtectDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev )
{
	if( Dev == NULL )
		return( RERROR );
	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*Dev = 0;
	}
	else
	{
		*Dev = OLT_DEV_ID;
	}
	return( ROK );
}

static int  pon_aps_brd_idx1=-1, pon_aps_brd_idx2=-1;
static int  pon_aps_port_idx1=-1, pon_aps_port_idx2=-1;

int  PonPortProtectBrdGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Brd )
{
	short int PonPortIdx;

	if( Brd == NULL )
		return( RERROR );

	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*Brd = 0;
	}
	else
	{
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE;
		
		*Brd = PonPortTable[PonPortIdx].swap_slot;
		if( *Brd == 0 )
		{
			if( (brdIdx == pon_aps_brd_idx1) && (portIdx == pon_aps_port_idx1) )
			{
				if( pon_aps_brd_idx2 != -1 )
					*Brd = pon_aps_brd_idx2;
			}
			else if( (brdIdx == pon_aps_brd_idx2) && (portIdx == pon_aps_port_idx2) )
			{
				if( pon_aps_brd_idx1 != -1 )
					*Brd = pon_aps_brd_idx1;
			}
		}
#else
		*Brd = 0;
#endif
	}
	return( ROK );
}

int  PonPortProtectPortGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *PonIdx )
{
	short int PonPortIdx;

	if( PonIdx == NULL )
		return( RERROR );

	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*PonIdx = 0;
	}
	else
	{
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE;
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		*PonIdx = PonPortTable[PonPortIdx].swap_port;
		if( *PonIdx == 0 )
		{
			if( (brdIdx == pon_aps_brd_idx1) && (portIdx == pon_aps_port_idx1) )
			{
				if( pon_aps_port_idx2 != -1 )
					*PonIdx = pon_aps_port_idx2;
			}
			else if( (brdIdx == pon_aps_brd_idx2) && (portIdx == pon_aps_port_idx2) )
			{
				if( pon_aps_port_idx1 != -1 )
					*PonIdx = pon_aps_port_idx1;
			}
		}
#else
		*PonIdx = 0;
#endif
	}
	return( ROK );
}


/**************************************************************
 *
 *    Function:  PonPortProtectDevSet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get the device type; if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  PonPortProtectDevSet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int Dev )
{
	/*if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
		{
		}
	else{
		}*/
	return( ROK );
}

/* modified by chenfj 
     采用临时变量来保存待设置的保护PON 口
    */

int  PonPortProtectBrdSet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int swap_slot )
{
	short int  PonPortIdx;

	if( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) )
	{
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		PonPortIdx = GetPonPortIdxBySlot((short int )(brdIdx), (short int)(portIdx));
		CHECK_PON_RANGE;

		if( PonPortSwapEnableQuery(PonPortIdx) == V2R1_PON_PORT_SWAP_ENABLE )
			return( RERROR );
		/* modified by wangjiah@2017-04-27 to support virtual slot configuration */
		if(!(SlotCardIsPonBoard(swap_slot) == ROK || SYS_SLOT_IS_VALID_REMOTE(swap_slot)))
			return(RERROR);
		if( SlotCardIsPonBoard(brdIdx) != ROK )
			return(RERROR);

		pon_aps_brd_idx1 = brdIdx;
		pon_aps_port_idx1 = portIdx;
		pon_aps_brd_idx2 = swap_slot;
	
		/*PonPortTable[PonPortIdx].swap_slot  = swap_slot;*/
#endif
	}
	return( ROK );
}

int  PonPortProtectPortSet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int PonIdx )
{
	short int PonPortIdx;
	
	if( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) )
	{
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		PonPortIdx = GetPonPortIdxBySlot((short int )(brdIdx), (short int)(portIdx));
		CHECK_PON_RANGE;

		if( PonPortSwapEnableQuery(PonPortIdx) == V2R1_PON_PORT_SWAP_ENABLE )
			return( RERROR );
		if(( PonIdx < 1)  || ( PonIdx >  PONPORTPERCARD ))
			return( RERROR );

		pon_aps_brd_idx1 = brdIdx;
		pon_aps_port_idx1 = portIdx;
		pon_aps_port_idx2 = PonIdx;
		/*PonPortTable[PonPortIdx].swap_port  = PonIdx;*/
#endif
	}
	return( ROK );
}

/**************************************************************
 *
 *    Function:  PonPortApsCtrlSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* MaxBw )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  get PON port auto protect switch control
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int PonPortApsCtrlSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  ctrl )
{
	short int PonPortIdx;
	unsigned int PartnerSlot, PartnerPort;
	int ret = RERROR;
	unsigned int swap_slot,swap_port;
	int swap_mode = V2R1_PON_PORT_SWAP_AUTO;

	if( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) )
	{
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		swap_slot = brdIdx;
		swap_port = portIdx;

		if( ctrl == V2R1_PON_PORT_SWAP_ENABLE) 
		{
			if((swap_slot != pon_aps_brd_idx1) || (swap_port != pon_aps_port_idx1))
				return(RERROR);

			PonPortIdx = GetPonPortIdxBySlot((short int )(brdIdx), (short int)(portIdx));
			CHECK_PON_RANGE;


			PartnerSlot = pon_aps_brd_idx2;
			PartnerPort = pon_aps_port_idx2;

			/*added by wangjiah@2017-04-27 for remote protect only support swap_slowly mode*/
			if(SYS_SLOT_IS_VALID_REMOTE(PartnerSlot)) swap_mode = V2R1_PON_PORT_SWAP_SLOWLY; 

			ret = EnablePonPortAutoProtect( swap_slot, swap_port, PartnerSlot, PartnerPort, swap_mode);
			if(/*( ret == ROK ) ||*/ ( ret == V2R1_IS_HOT_SWAP_PORT ))
				ret = ROK;
		}
		else 
		{
			if( ctrl == V2R1_PON_PORT_SWAP_FORCED)
			{
				ret = ForcePonPortAutoProtect( swap_slot, swap_port );
			}
			else if ( ctrl == V2R1_PON_PORT_SWAP_DISABLE) 
			{
				ret = DisablePonPortAutoProtect( swap_slot, swap_port );
				if(/*(ret == ROK ) ||*/(ret == V2R1_PORT_HAS_NO_HOT_SWAP_PORT )) 
					ret = ROK;
			}
		}
#endif
	}
	else
		ret = ROK;

	return ret;
}

int PonPortApsCtrlGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *ctrl )
{
	short int PonPortIdx;
	int swap_brd, swap_port;
	int ret;

	if( ctrl == NULL ) return(RERROR );

	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*ctrl = 0;
	}
	else
	{
		PonPortIdx = GetPonPortIdxBySlot((short int )(brdIdx), (short int)(portIdx));
		CHECK_PON_RANGE;
		
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		ret = PonPortSwapEnableQuery( PonPortIdx );
		if( ret != RERROR ) 
		{
			*ctrl = ret;
			if(PonPortAutoProtectPortQuery( brdIdx, portIdx, &swap_brd, &swap_port) != ROK)
				return(RERROR);

			/*pon_aps_brd_idx1 = brdIdx;
			pon_aps_port_idx1 = portIdx;
			pon_aps_brd_idx2 = swap_brd;
			pon_aps_port_idx2 = swap_port;*/
		}
		else
		{
			*ctrl = V2R1_PON_PORT_SWAP_DISABLE;
			return( RERROR  );
		}	
#endif
	}
	return( ROK );
}

int PonPortApsStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *ApsStatus )
{
	short int PonPortIdx;
	int ret;

	if( ApsStatus == NULL ) return( RERROR );
	
	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*ApsStatus = 0;
	}
	else
	{
		PonPortIdx = GetPonPortIdxBySlot((short int )(brdIdx), (short int)(portIdx));
		CHECK_PON_RANGE;

#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		ret = PonPortHotStatusQuery( PonPortIdx );
		if( ret != RERROR ) 
		{
			*ApsStatus = ret;
		}
		else
		{
			*ApsStatus = V2R1_PON_PORT_SWAP_UNKNOWN;
			return( RERROR  );
		}
#endif
	}
	return( ROK );
}

/**************************************************************
 *
 *    Function:  PonPortEncryptSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* MaxBw )
 *
 *    Param:  DeviceIndex_S  DevIdx-- intput, the Device index
 *                
 *
 *    Desc:  Onu encrypt operationl
 *              if ( slot && port == 0 )  && ( OnuId == 1 ) then the device is an olt, else the device is onu
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int PonPortEncryptSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  cryptionDirection )
{
	short int  OnuIdx;
	short int PonPortIdx;
	unsigned int EncryptStatus;

	OnuIdx = DevIdx.onuId - 1;

	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		if((cryptionDirection != PON_ENCRYPTION_PURE) && (cryptionDirection != PON_ENCRYPTION_DIRECTION_DOWN) && (cryptionDirection != PON_ENCRYPTION_DIRECTION_ALL))
			return(RERROR);
		PonPortIdx = GetPonPortIdxBySlot( DevIdx.slot,  DevIdx.port);
		CHECK_ONU_RANGE;
		/*if(GetOnuOperStatus(PonPortIdx,OnuIdx)  != ONU_OPER_STATUS_UP ) return(RERROR);*/
		if(EncryptIsSupported(PonPortIdx, OnuIdx) != ROK) return(RERROR);
		
		if((GetOnuEncryptStatus(PonPortIdx, OnuIdx , &EncryptStatus) == ROK )
			&& (EncryptStatus == V2R1_STARTED))
		{
			/*if((cryptionDirection != PON_ENCRYPTION_PURE)  && 
				( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection != cryptionDirection ))
				{
				return(RERROR);
				}*/ /* 问题单9464 */
			UCHAR encrypt_dir;
			ONU_MGMT_SEM_TAKE;
			encrypt_dir = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptDirection;
			ONU_MGMT_SEM_GIVE;
			if( encrypt_dir == cryptionDirection )
				return ROK;
		}	
		return(OnuEncryptionOperation( PonPortIdx, OnuIdx, cryptionDirection));
	}
	else{
		
	}
	return( RERROR );
}

int PonPortEncryptGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *cryptionDirection )
{
	short int  OnuIdx;
	short int PonPortIdx;

	OnuIdx = DevIdx.onuId - 1;

	if(cryptionDirection == NULL ) return( RERROR );
	*cryptionDirection = PON_ENCRYPTION_PURE;
	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) )){ /* Onu pon port  */
		
		PonPortIdx = GetPonPortIdxBySlot( DevIdx.slot,  DevIdx.port);

		CHECK_ONU_RANGE;
			
		ONU_MGMT_SEM_TAKE;
		*cryptionDirection = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].EncryptDirection;
		ONU_MGMT_SEM_GIVE;
		return ROK;
		
		}
	else{ /*olt pon port  */
		*cryptionDirection = PON_ENCRYPTION_PURE;
		return( ROK );
		}

}

/*end*/

#ifdef LLIDTABLE
#endif

#if 1
int  findFirstLlid( unsigned long *slot_ret, unsigned long *port_ret, unsigned long *onuId_ret, unsigned long *llid_ret )
{
	int llidIdx;
	short int PonPortIdx, OnuEntry,exit_flag = FALSE ;
		
	if( ( slot_ret == NULL )||( port_ret == NULL ) ||( onuId_ret == NULL ) ||( llid_ret == NULL )) return RERROR;

	ONU_MGMT_SEM_TAKE;
	for(OnuEntry =0; OnuEntry < MAXONU; OnuEntry++ )
    {
        for(llidIdx=0; llidIdx<MAXLLIDPERONU; llidIdx++)
        {
    		if(/*( OnuMgmtTable[OnuEntry].UsedFlag == ONU_USED_FLAG )||*/( OnuMgmtTable[OnuEntry].LlidTable[llidIdx].EntryStatus != LLID_ENTRY_UNKNOWN ))
			{
				exit_flag = TRUE;
				break;
    		}
        }
		
		if(exit_flag == TRUE)
			break;
	}
	ONU_MGMT_SEM_GIVE;
	
	if( OnuEntry == MAXONU ) return RERROR;

	PonPortIdx = ( OnuEntry / MAXONUPERPON);
	if( PonPortIdx >= MAXPONCHIP ) return RERROR;
			
	*slot_ret = GetCardIdxByPonChip( PonPortIdx );
	*port_ret =  GetPonPortByPonChip( PonPortIdx );
	*onuId_ret = OnuEntry % MAXONUPERPON + 1;
	*llid_ret = llidIdx+1;
	
	return ROK;
}


int   findNextLlid( unsigned long slot, unsigned long port, unsigned long onuId, const unsigned long llid,
				unsigned long *slot_ret, unsigned long *port_ret, unsigned long *onuId_ret, unsigned long *llid_ret)
{
	int llidIdx;
	short int PonPortIdx, OnuIdx, OnuEntry,i,exit_flag = FALSE;

	if( ( slot_ret == NULL )||( port_ret == NULL ) ||( onuId_ret == NULL ) ||( llid_ret == NULL )) return RERROR;
	
	PonPortIdx = GetPonPortIdxBySlot( slot, port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if( OnuEntry >= MAXONU ) return RERROR;

    if( MAXLLIDPERONU <= (llidIdx = llid + 1) )
    {
        llidIdx = 0;
        if ( ++OnuEntry >= MAXONU ) return RERROR;
    }
	
	ONU_MGMT_SEM_TAKE;
	for( i = OnuEntry; i < MAXONU; i++ ){
        for(; llidIdx<MAXLLIDPERONU; llidIdx++)
        {
    		if(/*( OnuMgmtTable[i].UsedFlag == ONU_USED_FLAG )||*/( OnuMgmtTable[i].LlidTable[llidIdx].EntryStatus != LLID_ENTRY_UNKNOWN )) 
    		{
    			exit_flag = TRUE;
    			break;
    		}
		}
		
		if(exit_flag == TRUE)
			break;
		
        llidIdx = 0;
    }
	ONU_MGMT_SEM_GIVE;

	if( i == MAXONU ) return RERROR;
	
	PonPortIdx = ( i / MAXONUPERPON);
	if( PonPortIdx >= MAXPONCHIP ) return RERROR;
			
	*slot_ret = GetCardIdxByPonChip( PonPortIdx );
	*port_ret =  GetPonPortByPonChip( PonPortIdx );
	*onuId_ret = i % MAXONUPERPON + 1 ;
	*llid_ret = llidIdx+1;
	
	return ROK;
}

int  LLIDEntryIndexCheck( DeviceIndex_S DevIdx, unsigned long  LlidIdx )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	UCHAR status;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( LlidIdx > MAXLLIDPERONU || LlidIdx < 1) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	status = OnuMgmtTable[OnuEntry].LlidTable[LlidIdx-1].EntryStatus;
	ONU_MGMT_SEM_GIVE;

	if(/*(OnuMgmtTable[OnuEntry].UsedFlag == ONU_USED_FLAG ) &&*/ ( status != LLID_ENTRY_UNKNOWN ))
		return( ROK );

	return( RERROR );
	
}
	
int PonPortLlidTypeSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1) return RERROR;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].LlidType = val;
		rc = ROK;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;
}


int PonPortLlidTypeGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		*pValBuf  = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].LlidType ;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;
}

int PonPortLlidOltBrdGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		*pValBuf  = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOltBoard;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidOltBrdSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOltBoard = val;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}


int PonPortLlidOltPortGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		*pValBuf  = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOltPort;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidOltPortSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOltPort = val;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidOnuBrdGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		*pValBuf  = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOnuBoard;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidOnuBrdSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOnuBoard = val;
	}
	ONU_MGMT_SEM_GIVE;
	
	return rc;
}

int PonPortLlidOnuPortGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		*pValBuf  = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOnuPort;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidOnuPortSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	ONU_MGMT_SEM_TAKE;
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = ROK;
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].llidOnuPort = val;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

int PonPortLlidllidGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long  *pValBuf )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if( pValBuf == NULL ) return( RERROR );
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_ACTIVE )
		*pValBuf = 0; /*不在线可返回0，但不能返回错误*/
	else
	    *pValBuf = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].Llid;
	ONU_MGMT_SEM_GIVE;

    if ( INVALID_LLID == *pValBuf )
    {
        *pValBuf = 0;
    }

	return( ROK );
}

int  PonPortLlidUpLinkBWGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if( pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN );
		rc = GetOnuUplinkBW( PonPortIdx, OnuIdx ,(unsigned int *) pValBuf );
	ONU_MGMT_SEM_GIVE;

	return rc;
}

int  PonPortLlidUpLinkBWGet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *UplinkClass, unsigned long *UplinkDelaly, unsigned long *assured_bw, unsigned long *best_effort_bw )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(( UplinkClass == NULL ) || ( UplinkDelaly == NULL ) || ( assured_bw == NULL ) || ( best_effort_bw == NULL ) )  return( RERROR );
	*UplinkClass  = 0;
	*UplinkDelaly = 0;
	*assured_bw = 0;
	*best_effort_bw = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
		rc = GetOnuUplinkBW_1( PonPortIdx, OnuIdx , (unsigned int *)UplinkClass, (unsigned int *)UplinkDelaly, (unsigned int *)assured_bw, (unsigned int *)best_effort_bw );

	return rc;
}

/*begin:
added by wangxiaoyu 2008-02-14
*/
#ifdef PLATO_DBA_V3
int  PonPortLlidUpLinkBWGet_2( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *UplinkClass, unsigned long *pfixedbw, unsigned long *assured_bw, unsigned long *best_effort_bw )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(( UplinkClass == NULL ) || ( pfixedbw == NULL ) || ( assured_bw == NULL ) || ( best_effort_bw == NULL ) )  return rc;
	*UplinkClass  = 0;
	*pfixedbw = 0;
	*assured_bw = 0;
	*best_effort_bw = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
		rc = GetOnuUplinkBW_2( PonPortIdx, OnuIdx , (unsigned int *)UplinkClass, (unsigned int *)pfixedbw, (unsigned int *)assured_bw, (unsigned int *)best_effort_bw );

	return rc;
}
#endif

/*end*/	

int  PonPortLlidUpLinkBWSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx/*, OnuEntry*/;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
#if 0	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );
#endif

	/*SetOnuUsedFlag( PonPortIdx, onuId);*/
	return (SetOnuUplinkBW( PonPortIdx, OnuIdx , val ));

}

int  PonPortLlidUpLinkBWSet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long UplinkClass, unsigned long UplinkDelaly, unsigned long assured_bw, unsigned long best_effort_bw )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx/*, OnuEntry*/;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;

#if 0	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );
#endif

	/*SetOnuUsedFlag( PonPortIdx, onuId);*/
#ifdef PLATO_DBA_V3
	return (SetOnuUplinkBW_2( PonPortIdx, OnuIdx, UplinkClass, UplinkDelaly, 0, assured_bw, best_effort_bw ));
#else
	return (SetOnuUplinkBW_1( PonPortIdx, OnuIdx, UplinkClass, UplinkDelaly, assured_bw, best_effort_bw ));
#endif
}

/*begin:
added by wangxiaoyu 2008-02-14
*/
int  PonPortLlidUpLinkBWSet_2( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long UplinkClass, unsigned long UplinkDelaly, unsigned long fixedbw, unsigned long assured_bw, unsigned long best_effort_bw )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx/*, OnuEntry*/;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
#if 0
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );
#endif

	/*SetOnuUsedFlag( PonPortIdx, onuId);*/
#ifdef PLATO_DBA_V3
	return (SetOnuUplinkBW_2( PonPortIdx, OnuIdx, UplinkClass, UplinkDelaly, fixedbw, assured_bw, best_effort_bw ));
#else
	return (SetOnuUplinkBW_1( PonPortIdx, OnuIdx, UplinkClass, UplinkDelaly, assured_bw, best_effort_bw ));
#endif
}


/*end*/

int  PonPortLlidDownLinkBWGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if( pValBuf == NULL ) return rc;
	*pValBuf  = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
		rc = GetOnuDownlinkBW( PonPortIdx, OnuIdx , (unsigned int *) pValBuf );

	return rc;
}

int  PonPortLlidDownLinkBWGet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *DownlinkClass, unsigned long *DownlinkDelaly, unsigned long *assured_bw, unsigned long *best_effort_bw )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if(( DownlinkClass == NULL ) || ( DownlinkDelaly == NULL ) || ( assured_bw == NULL ) || ( best_effort_bw == NULL ) )  return rc;
	*DownlinkClass  = 0;
	*DownlinkDelaly = 0;
	*assured_bw = 0;
	*best_effort_bw = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;
	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1  ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
		rc = GetOnuDownlinkBW_1( PonPortIdx, OnuIdx , (unsigned int *) DownlinkClass, (unsigned int *)DownlinkDelaly, (unsigned int *)assured_bw, (unsigned int *)best_effort_bw);

	return rc;
}


int  PonPortLlidDownLinkBWSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*if(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/

	/*SetOnuUsedFlag( PonPortIdx, onuId);*/
	return (SetOnuDownlinkBW( PonPortIdx, OnuIdx , val ));

}

int  PonPortLlidDownLinkBWSet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long DownlinkClass, unsigned long DownlinkDelaly, unsigned long assured_bw, unsigned long best_effort_bw )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx/*, OnuEntry*/;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return RERROR;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
#if 0
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );
#endif

	/*SetOnuUsedFlag( PonPortIdx, onuId);*/
	return (SetOnuDownlinkBW_1( PonPortIdx, OnuIdx , DownlinkClass, DownlinkDelaly, assured_bw, best_effort_bw));

}

int PonPortLlidDescGet( DeviceIndex_S DevIdx, unsigned long llidIdx,  char *pValBuf, unsigned long  *pValLen )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if((pValBuf == NULL )||(pValLen == NULL )) return rc;
	*pValBuf  = 0;
	*pValLen = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
#if 0        
		*pValLen = OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDescLen;
		VOS_MemCpy( pValBuf, &(OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDesc[0] ), OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDescLen );
#else
        *pValLen = 0;
        pValBuf = NULL;
#endif
		rc = ROK;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;
}

int PonPortLlidDescSet( DeviceIndex_S DevIdx, unsigned long llidIdx,  char *pValBuf, unsigned long  pValLen )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

/*	if((pValBuf == NULL ) ) return( RERROR );*/

	if( pValLen > MAXLLIDDESCLEN ) pValLen = MAXLLIDDESCLEN;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
#if 0        
		VOS_MemZero( &(OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDesc[0]), sizeof( OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDesc ) );
		OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDescLen = pValLen;
		if( pValLen != 0 )
			VOS_MemCpy( &(OnuMgmtTable[OnuEntry].LlidTable[llidIdx].LlidDesc[0] ), pValBuf, pValLen );
#endif        
		rc = ROK;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;
}


int  PonPortLlidSupportedMacNumGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	if( pValBuf == NULL ) return rc;
	*pValBuf = 0;
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;
	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = GetOnuMaxMacNum( PonPortIdx, OnuIdx , (unsigned int *) pValBuf );
	}

	return rc;
}
	

int  PonPortLlidSupportedMacNumSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val )
{
	int rc = RERROR;
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;
	
	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    val |= ONU_NOT_DEFAULT_MAX_MAC_FLAG;    
	if( OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		rc = SetOnuMaxMacNum( PonPortIdx, OnuIdx , llidIdx-1, val );
	}

	return rc;
}


int  PonPortLlidRowStatusSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long  val )
{
	short int  slot, port, onuId;
	short int PonPortIdx, OnuIdx, OnuEntry;
	ULONG status, maxmac;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	onuId = (unsigned short int )DevIdx.onuId;

	if(( (slot == 0 ) && ( port == 0 ) && ( onuId == 1) )) return(RERROR);

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	OnuIdx = onuId - 1;

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	status = OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus;
	ONU_MGMT_SEM_GIVE;

	if( val == status  ) return( ROK );

	if( val == LLID_ENTRY_CREATE_AND_WAIT )
		{
		return( ROK );
		/*
		CreateOnuLlidEntry( PonPortIdx,  onuId, (short int) llidIdx);
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_READY;
		return( ROK );
		*/
		}
	
	if( val == LLID_ENTRY_CREATE_AND_GO )
		{
		return( ROK );
		/*
		CreateOnuLlidEntry( PonPortIdx,  onuId, (short int) llidIdx);
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		ActiveOnuUplinkBW( PonPortIdx, onuId);
		ActiveOnuDownlinkBW( PonPortIdx, onuId );
		llidIdx = 0;
		SetOnuMaxMacNum( PonPortIdx, onuId, llidIdx, OnuMgmtTable[PonPortIdx*MAXONUPERPON+onuId].LlidTable[0].MaxMAC );
		return( ROK );
		*/
		}

	if(val == LLID_ENTRY_DESTORY )
		{
		return( ROK );
#if 0
		if( OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN ) return( ROK );
		SetOnuUplinkBW( PonPortIdx, onuId,0 );
		SetOnuDownlinkBW( PonPortIdx, onuId, 0 );
		llidIdx = 0;
		SetOnuMaxMacNum( PonPortIdx, onuId, llidIdx,0);
		/*
		ActiveOnuUplinkBW( PonPortIdx, onuId);
		ActiveOnuDownlinkBW( PonPortIdx, onuId );
		SetOnuMaxMacNum( PonPortIdx, onuId, OnuMgmtTable[PonPortIdx*MAXONUPERPON+onuId].LlidTable[0].MaxMAC );
		*/		
		ClearOnuLlidEntry( PonPortIdx, onuId , ( short int) llidIdx);
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_UNKNOWN;
		return( ROK );
#endif
		}

	if( val == LLID_ENTRY_ACTIVE )
		{
		if( status == LLID_ENTRY_UNKNOWN ) return( ROK );
	
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		maxmac = OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC;
		ONU_MGMT_SEM_GIVE;
#if 0        
		ActiveOnuUplinkBW( PonPortIdx, OnuIdx);
		ActiveOnuDownlinkBW( PonPortIdx, OnuIdx );
#else
        OnuEvent_Active_UplinkRunningBandWidth(PonPortIdx, OnuIdx);
        OnuEvent_Active_DownlinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
		llidIdx = 0;
        maxmac |= ONU_NOT_DEFAULT_MAX_MAC_FLAG;
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, llidIdx, maxmac );
		return( ROK );
		}

	if( val == LLID_ENTRY_NOT_IN_ACTIVE )
		{
		OnuLLIDTable_S  llidTable;	
		
		if( status == LLID_ENTRY_UNKNOWN ) return( ROK );
		if( GetOnuOperStatus( PonPortIdx,  OnuIdx) != ONU_OPER_STATUS_UP ) return( ROK );
		
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy((char *) &( llidTable.EntryStatus ), (char *)&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus), sizeof( OnuLLIDTable_S ) );
		maxmac = OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC;
		ONU_MGMT_SEM_GIVE;

		ClearOnuLlidEntry( PonPortIdx, OnuIdx , ( short int) llidIdx);
#if 0        
		ActiveOnuUplinkBW( PonPortIdx, OnuIdx);
		ActiveOnuDownlinkBW( PonPortIdx, OnuIdx );
#else
        OnuEvent_Active_UplinkRunningBandWidth(PonPortIdx, OnuIdx);
        OnuEvent_Active_DownlinkRunningBandWidth(PonPortIdx, OnuIdx);
#endif
        maxmac |= ONU_NOT_DEFAULT_MAX_MAC_FLAG;
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, 0, maxmac);
		
		ONU_MGMT_SEM_TAKE;
		llidTable.ActiveUplinkBandwidth = OnuMgmtTable[OnuEntry].LlidTable[0].ActiveUplinkBandwidth;
		llidTable.ActiveDownlinkBandwidth = OnuMgmtTable[OnuEntry].LlidTable[0].ActiveDownlinkBandwidth ;
		VOS_MemCpy((char *)&(OnuMgmtTable[OnuEntry].LlidTable[0]), (char *) &llidTable, sizeof( OnuLLIDTable_S ) );
		OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
		ONU_MGMT_SEM_GIVE;
		
		return( ROK );
		
		}
	return( ROK );

}

int  PonPortLlidRowStatusGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long  *pValbuf )
{
	int rc = RERROR;
	short int  slot, port, OnuIdx;
	short int PonPortIdx, OnuEntry;

	if( pValbuf == NULL ) return rc;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	OnuIdx = (unsigned short int )DevIdx.onuId - 1;

	if(( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )) return rc;

	PonPortIdx = GetPonPortIdxBySlot( slot,  port);

	CHECK_ONU_RANGE;

    if ( llidIdx > MAXLLIDPERONU || llidIdx < 1 ) return RERROR;
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus != LLID_ENTRY_UNKNOWN )
	{
		*pValbuf = OnuMgmtTable[OnuEntry].LlidTable[llidIdx-1].EntryStatus;
		rc = ROK;
	}
	ONU_MGMT_SEM_GIVE;

	return rc;

}

	
#endif

#ifdef ONU_SW_UPDATE 
#endif

int  OnuSwUpdateEnableSet(DeviceIndex_S DevIdx,  unsigned long flag)
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	OnuIdx = (unsigned short int )DevIdx.onuId - 1;

	if(( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )) return(RERROR);
	PonPortIdx = GetPonPortIdxBySlot( slot,  port);

	CHECK_ONU_RANGE
	
	if((flag != ONU_SW_UPDATE_ENABLE ) && (flag != ONU_SW_UPDATE_DISABLE) ) return ( RERROR );
	SetOnuSWUpdateCtrl( PonPortIdx, OnuIdx, flag );
	
	return( ROK );
}

int  OnuSwUpdateEnableGet( DeviceIndex_S DevIdx, unsigned long *pVal)
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	if( pVal == NULL ) return( RERROR );

	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	OnuIdx = (unsigned short int )DevIdx.onuId - 1;

	if(( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot( slot,  port);

	CHECK_ONU_RANGE

	*pVal = GetOnuSWUpdateCtrl( PonPortIdx, OnuIdx);

	return( ROK );

}


int  OnuSwUpdateStatusGet(DeviceIndex_S DevIdx, unsigned long *pVal)
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	if( pVal == NULL ) return( RERROR );
	
	slot =(unsigned short int ) DevIdx.slot;
	port = (unsigned short int )DevIdx.port;
	OnuIdx = (unsigned short int )DevIdx.onuId - 1;

	if(( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )) return (RERROR);
	PonPortIdx = GetPonPortIdxBySlot( slot,  port);
	
	CHECK_ONU_RANGE

	*pVal = GetOnuSWUpdateStatus(PonPortIdx, OnuIdx);

	return( ROK );
}


int OnuTrafficServiceSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  EnableFlag )
{

	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		CHECK_ONU_RANGE

#if 0 
		if( V2R1_CTC_STACK )
		{
		unsigned long auth_mode;
		getOnuAuthEnable(0, 0, &auth_mode);
		if( auth_mode == V2R1_ONU_AUTHENTICATION_DISABLE )
			{
			return( RERROR );
			}		
		}
#endif

		result = SetOnuTrafficServiceEnable( PonPortIdx, OnuIdx, EnableFlag );
		return( result );
		}
	else{ /* get olt pon port  */

		return( ROK );
		}
}

int OnuTrafficServiceGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *EnableFlag )
{

	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( EnableFlag == NULL ) return( RERROR );

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) )){ /* Onu pon port  */
		PonPortIdx = GetPonPortIdxBySlot( slot, port );
		CHECK_ONU_RANGE

		result = GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx );
		if( result != RERROR ) 
			*EnableFlag = result;
		return( ROK );
		}
	else{ /* get olt pon port  */
		*EnableFlag = 1;
		return( ROK );
		}
}
/*
BEGIN: modified by wangxy 2007-07-20
 修改PonPortWindowRangeGet 及PonPortWindowRangeSet参数;
 改为只有对OLT的pon端口进行相关操作;
*/
int PonPortWindowRangeGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, unsigned long *pRange )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( pRange == NULL ) return( RERROR );

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{ /* Onu pon port  */
		return ROK;
	}
	else
	{ /* get olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_ONU_RANGE

		if( GetPonRange( PonPortIdx, &result ) == ROK )
			*pRange = result+1;		
	}
	return( ROK );
}

int PonPortWindowRangeSet(DeviceIndex_S DevIdx,  int brdIdx, int portIdx, unsigned long range )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx ;
	int result = PON_RANGE_20KM;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId - 1;

	if( range == 1 )
		result = PON_RANGE_CLOSE;
	else if( range == 2 )
		result = PON_RANGE_20KM;
	else if( range == 3 )
		result = PON_RANGE_40KM;
	else if( range == 4 )
		result = PON_RANGE_60KM;
	else if( range == 5 )
		result = PON_RANGE_80KM;
	else
		return RERROR;
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 0) ))
	{
		return ROK;
	}
	else
	{ /* set olt pon port  */
		PonPortIdx = GetPonPortIdxBySlot(brdIdx, portIdx);
		CHECK_ONU_RANGE

		return SetPonRange( PonPortIdx, result );		
	}
	return( ROK );
}
/*END*/

/*获取实际的ONU MAC数目*/
int OnuMacAddrNumberGet( DeviceIndex_S  DevIdx,  short int *MacAddrNum )
{
	short int  slot, port, onuId;
	int rc = RERROR;
    ULONG num,enable;

	if(MacAddrNum == NULL ) return( RERROR );
	slot =(short int ) DevIdx.slot;
	port = (short int )DevIdx.port;
	onuId = (short int )DevIdx.onuId;


	if(!( (slot == 0 ) && ( port == 0 ) && ( onuId == 1)))
        { 
        		short int PonPortIdx; 
        		short int OnuIdx;

        		PonPortIdx = GetPonPortIdxBySlot( slot, port );
        		OnuIdx = onuId - 1;
                
                    if( GetOnuOperStatus(PonPortIdx, OnuIdx )  == ONU_OPER_STATUS_UP )  
        		{
        		    rc=GetOnuMacCheckEnable(&enable); 
                    
                                if(enable==1)
                                    {
                                        rc=OnuMgt_GetOnuMacCheckFlag(PonPortIdx,OnuIdx,&num);
                                        if(rc==VOS_OK)
                                         {
                                            *MacAddrNum=num;
                                         }
                                    }
                                else
                                    {
                                        rc=OnuMgt_GetOltMacAddrTbl( PonPortIdx,OnuIdx, MacAddrNum,NULL);
                                       /* if(rc==VOS_OK)
                                             {
                                                *MacAddrNum=num;
                                             }*/
                                   }                        
        		}    
                    else
                        {
                            *MacAddrNum = 1;         
        			return  rc;/*( ROK );*/
                        }
        }
	else
	{ 
	      /*short int PonPortIdx; 
		PonPortIdx = GetPonPortIdxBySlot( slot, port );		
		rc=OLT_GetMacAddrTbl(PonPortIdx,MacAddrNum,NULL);*/
		*MacAddrNum=0;	
	}
	return rc;

}

/* added by wangjiah@2017-04-27 -- Begin*/
int LogicalSlotServiceStatusGet(int * status)
{	
	if(NULL == status) return RERROR;

	DEVSM_REMOTE_LOCK();
	*status = (DEVRM_ENABLE == devsm_remote_srvflags) ? 2 : 1;
	DEVSM_REMOTE_UNLOCK();

	return ROK;
}

int LogicalSlotServiceStatusSet(int status)
{
	int rc = VOS_ERROR;

	if(1 == status)
	{
		rc = devsm_remote_slot_set_service_mode(DEVRM_DISABLE);
	}
	else if(2 == status)
	{
		rc = devsm_remote_slot_set_service_mode(DEVRM_ENABLE);
	}

	return rc;
}

int LogicalSlotServicePortGet(unsigned int * port)
{
	if(NULL == port) return RERROR;

	DEVSM_REMOTE_LOCK();
	*port = devsm_remote_srvport;
	DEVSM_REMOTE_UNLOCK();
	return ROK;
}

int LogicalSlotServicePortSet(unsigned int port)
{
	if(port < 0 || port > 65535) return ERROR;
	return devsm_remote_slot_setsrvport(port);
}

int LogicalSlotServiceAuthStatusGet(int * status) 
{
	if(NULL == status) return RERROR;
	DEVSM_REMOTE_LOCK();
	*status = (DEVRM_ENABLE == devsm_remote_srvauth) ? 2 : 1;
	DEVSM_REMOTE_UNLOCK();
	return ROK;
}

int LogicalSlotServiceAuthStatusSet(int status)
{
	int rc = RERROR;

	if(1 == status)
	{
		rc = devsm_remote_slot_set_service_auth(DEVRM_DISABLE);
	}
	else if(2 == status)
	{
		rc = devsm_remote_slot_set_service_auth(DEVRM_ENABLE);
	}
	return rc;
}

int LogicalSlotServiceHearbeatGet(unsigned int * heartout)
{
	if(NULL == heartout) return RERROR;
	DEVSM_REMOTE_LOCK();
	* heartout = devsm_remote_heartout;
	DEVSM_REMOTE_UNLOCK();
	return ROK;
}	

int LogicalSlotServiceHearbeatSet(unsigned int heartout)
{
	if(heartout < 0 || heartout > 65535) return ERROR;
	if(heartout > LOGICAL_SLOT_SERVICE_HEART_MAX) heartout = LOGICAL_SLOT_SERVICE_HEART_MAX;
	if(heartout < LOGICAL_SLOT_SERVICE_HEART_MIN) heartout = LOGICAL_SLOT_SERVICE_HEART_MIN;
	return devsm_remote_slot_set_service_heart(heartout);
}

//to store IP and port setting of latest modification
static int gs_iIpAddresIsSet = 0;
static int gs_iUdpPortIsSet = 0;
static ulong_t gs_ulIpAddr = 0;
static ulong_t gs_ulUdpPort = 0;

//the board index of addr being modified at latest.
static int gi_lastAddrModiBrd = 0; 

static void clearStaticTag()
{
	gs_iIpAddresIsSet = 0;
	gs_iUdpPortIsSet = 0;
	gs_ulIpAddr = 0;
	gs_ulUdpPort = 0;
}

int LogicalSlotRemoteIpGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, unsigned long * pUlIpAddr)
{
	int rc = RERROR;
	unsigned long udpPort = 0;

	if(NULL == pUlIpAddr) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		*pUlIpAddr = 0;
		return ROK;
	}

	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*pUlIpAddr = 0;
		rc = ROK;
	}
	else
	{
		if(gs_iIpAddresIsSet && !gs_iUdpPortIsSet && brdIdx == gi_lastAddrModiBrd)
		{
			*pUlIpAddr = gs_ulIpAddr;
			rc = ROK;
		}
		else
		{
			rc = devsm_remote_slot_getremoteaddr(brdIdx, pUlIpAddr, &udpPort);
			if(REMOTE_ERR_NOTEXIST == rc)
			{
				*pUlIpAddr = 0;
				rc = ROK;
			}
		}
	}
	OLT_MIB_DEBUG("\r\n LogicalSlotRemoteIpGet(%d,%d) ret(%d) ip:%x port:%d",brdIdx, portIdx, rc, *pUlIpAddr, udpPort);
	return rc;
}

int LogicalSlotRemoteIpSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, unsigned long ulIpAddr)
{
	int rc = RERROR;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;

	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		if(gs_iUdpPortIsSet && brdIdx == gi_lastAddrModiBrd)
		{
			rc = devsm_remote_slot_setipaddress(brdIdx, ulIpAddr, gs_ulUdpPort, DEVRM_FLAGS_USRCFG);
			if(ROK == rc)
			{
				clearStaticTag();
			}
		}
		else
		{
			gs_ulIpAddr = ulIpAddr;
			gs_iIpAddresIsSet = 1;
			gi_lastAddrModiBrd = brdIdx;
			rc = ROK;
		}
	}
	OLT_MIB_DEBUG("\r\n LogicalSlotRemoteIpSet(%d,%d) ret(%d) ip:%x port:%d",brdIdx, portIdx, rc, ulIpAddr, gs_ulUdpPort);
	return rc;
}

int LogicalSlotRemotePortGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, unsigned long * pUlPort)
{
	int rc = RERROR;
	unsigned long ulIpAddr = 0;

	if(NULL == pUlPort) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		*pUlPort = 0;
	   	return ROK;
	}
	if(!( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1) ))
	{
		*pUlPort= 0;
		rc = ROK;
	}
	else
	{
		if(gs_iUdpPortIsSet && !gs_iIpAddresIsSet && brdIdx == gi_lastAddrModiBrd)
		{
			*pUlPort = gs_ulUdpPort;
			rc = ROK;
		}
		else
		{
			rc = devsm_remote_slot_getremoteaddr(brdIdx, &ulIpAddr, pUlPort);
			if(REMOTE_ERR_NOTEXIST == rc)
			{
				*pUlPort = 0;
				rc = ROK;
			}
		}
	}
	OLT_MIB_DEBUG("\r\n LogicalSlotRemotePortGet(%d,%d) ret(%d) ip:%x port:%d",brdIdx, portIdx, rc, ulIpAddr, *pUlPort);
	return rc;
}

int LogicalSlotRemotePortSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, unsigned long ulPort)
{
	int rc = RERROR;
	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;
	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		if(gs_iIpAddresIsSet && brdIdx == gi_lastAddrModiBrd)
		{
			rc = devsm_remote_slot_setipaddress(brdIdx, gs_ulIpAddr, ulPort, DEVRM_FLAGS_USRCFG);
			if(ROK == rc)
			{
				clearStaticTag();
			}
		}
		else
		{
			gs_ulUdpPort = ulPort;
			gs_iUdpPortIsSet = 1;
			gi_lastAddrModiBrd = brdIdx;
			rc = ROK;
		}
	}
	OLT_MIB_DEBUG("\r\n LogicalSlotRemotePortSet(%d,%d) ret(%d) ip:%x port:%d",brdIdx, portIdx, rc, gs_ulIpAddr, ulPort);
	return rc;
}

//to store slot and pon port setting of latest modification
static int gs_iSlot = -1;
static int gs_iPort = -1;

//the board index of pon-port being modified at latest.
static int gi_lastPonModiBrd = 0;

int RemotePhysicalSlotGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, int * piSlot)
{
	int rc = RERROR;
	int iPort = 0;
	if(NULL == piSlot) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		*piSlot = 0;
	   	return ROK;
	}
	
	if(!((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1)))
	{
		*piSlot = 0;
		rc = ROK;
	}
	else
	{
		if(-1 == gs_iPort && -1 != gs_iSlot && brdIdx == gi_lastPonModiBrd)
		{
			*piSlot = gs_iSlot;	
			rc = ROK;
		}
		else
		{
			rc = devsm_remote_port_getremoteport(brdIdx, portIdx, piSlot, &iPort);
			if(REMOTE_ERR_NOTEXIST == rc)
			{
				*piSlot = 0;
				rc = ROK;
			}
		}
	}
	OLT_MIB_DEBUG("\r\n RemotePhysicalSlotGet(%d,%d) ret(%d) slot:%d, port:%d",brdIdx, portIdx, rc, *piSlot, iPort);
	return rc;
}

int RemotePhysicalSlotSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, int iSlot)
{
	int rc = RERROR;	
	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;
		
	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		if(-1 != gs_iPort && gi_lastPonModiBrd == brdIdx)
		{
			if(ROK == devsm_remote_port_checkremoteport(&brdIdx, &portIdx, iSlot, gs_iPort))
			{
				rc = devsm_remote_port_setremoteport(brdIdx, portIdx, iSlot, gs_iPort, DEVRM_FLAGS_USRCFG);
				if(ROK == rc)
				{
					gs_iPort = -1;
					gs_iSlot = -1;
				}
			}
			else
			{
				rc = RERROR;
			}
		}
		else
		{
			gs_iSlot = iSlot;
			gi_lastPonModiBrd = brdIdx;
			rc = ROK;
		}
	}
	OLT_MIB_DEBUG("\r\n RemotePhysicalSlotSet(%d,%d) ret(%d) slot:%d port:%d",brdIdx, portIdx, rc, iSlot, gs_iPort);
	return rc;
}

int RemotePhysicalPortGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, int * piPort)
{
	int rc = RERROR;
	int iSlot= 0;
	if(NULL == piPort) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		*piPort = 0;
		return ROK;
	}
	if(!((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1)))
	{
		*piPort = 0;
		rc = ROK;
	}
	else
	{
		if(-1 == gs_iSlot && -1 != gs_iPort && brdIdx == gi_lastPonModiBrd)
		{
			*piPort= gs_iPort;	
			rc = ROK;
		}
		else
		{
			rc = devsm_remote_port_getremoteport(brdIdx, portIdx, &iSlot, piPort);
			if(REMOTE_ERR_NOTEXIST == rc)
			{
				*piPort = 0;
				rc = ROK;
			}
		}
	}
	OLT_MIB_DEBUG("\r\n RemotePhysicalPortGet(%d,%d) ret(%d) slot:%d port:%d",brdIdx, portIdx, rc, iSlot, *piPort);
	return rc;
}

int RemotePhysicalPortSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, int iPort)
{
	int rc = RERROR;	
	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;
	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		if(-1 != gs_iSlot && gi_lastPonModiBrd == brdIdx)
		{
			if(ROK == devsm_remote_port_checkremoteport(&brdIdx, &portIdx, gs_iSlot, iPort))
			{
				rc = devsm_remote_port_setremoteport(brdIdx, portIdx, gs_iSlot, iPort, DEVRM_FLAGS_USRCFG);
				if(rc = ROK)
				{
					gs_iPort = -1;
					gs_iSlot = -1;
				}
			}
			else
			{
				rc = RERROR;
			}
		}
		else
		{
			gs_iPort = iPort;
			gi_lastPonModiBrd = brdIdx;
			rc = ROK;
		}
	}
	OLT_MIB_DEBUG("\r\n RemotePhysicalPortSet(%d,%d) ret(%d) slot:%d port:%d",brdIdx, portIdx, rc, gs_iSlot, iPort);
	return rc;
}

int RemoteDeviceAccessUserGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * user)
{
	int rc = RERROR;
	if(NULL == user) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		user = " ";
		return ROK;
	}

	if(!((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1)))
	{
		user = " ";
		rc = ROK;
	}
	else
	{
		rc = devsm_remote_slot_getdevuser(brdIdx, user);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceAccessUserGet(%d,%d) ret(%d) user:%s size:%d",brdIdx, portIdx, rc, user, VOS_StrLen(user));
	return rc;
}

int RemoteDeviceAccessUserSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * user)
{
	int rc = RERROR;
	if(NULL == user) return rc;
	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;
	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		rc = devsm_remote_slot_setdevuser(brdIdx, user);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceAccessUserSet(%d,%d) ret(%d) user:%s size:%d",brdIdx, portIdx, rc, user, VOS_StrLen(user));
	return rc;
}


int RemoteDeviceAccessPasswordGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * password)
{
	int rc = RERROR;
	if(NULL == password) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		password = " ";
		return ROK;
	}

	if(!((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1)))
	{
		password = " ";
		rc = ROK;
	}
	else
	{
		rc = devsm_remote_slot_getdevpw(brdIdx, password);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceAccessPasswordGet(%d,%d) ret(%d) pw:%s size:%d",brdIdx, portIdx, rc, password, VOS_StrLen(password));
	return rc;
}

int RemoteDeviceAccessPasswordSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * password)
{
	int rc = RERROR;
	if(NULL == password) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;

	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		rc = devsm_remote_slot_setdevpw(brdIdx, password);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceAccessPasswordSet(%d,%d) ret(%d) pw:%s size:%d",brdIdx, portIdx, rc, password, VOS_StrLen(password));
	return rc;
}

int RemoteDeviceDesGet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * des)
{
	int rc = RERROR;
	if(NULL == des) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx))
	{
		des = " ";
		return ROK;
	}

	if(!((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1)))
	{
		des = " ";
		rc = ROK;
	}
	else
	{
		rc = devsm_remote_slot_getdevname(brdIdx, des);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceDesGet(%d,%d) ret(%d) des:%s size:%d",brdIdx, portIdx, rc, des, VOS_StrLen(des));
	return rc;
}

int RemoteDeviceDesSet(DeviceIndex_S DevIdx, int brdIdx, int portIdx, char * des)
{
	int rc = RERROR;

	if(NULL == des) return rc;

	if(!SYS_SLOT_IS_VALID_REMOTE(brdIdx)) return rc;

	if((DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 1))
	{
		rc = devsm_remote_slot_setdevname(brdIdx, des);
	}
	OLT_MIB_DEBUG("\r\n RemoteDeviceDesSet(%d,%d) ret(%d) des:%s size:%d",brdIdx, portIdx, rc, des, VOS_StrLen(des));
	return rc;
}
/* added by wangjiah@2017-04-27 -- End*/

#ifdef __cplusplus
}
#endif

