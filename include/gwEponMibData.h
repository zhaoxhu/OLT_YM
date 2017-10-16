
/***************************************************************
*
*						Module Name: gwEponMibData.h
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
#ifndef __GWEPONMIBDATA__H
#define __GWEPONMIBDATA__H

/*
#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "PonEventHandler.h"
*/

/* from wangxiaoyu */
typedef struct DeviceIdx{
	LONG slot;
	LONG port;
	LONG onuId;
}DeviceIndex_S;

typedef enum{
	ONUOFFLINE=0,
	ONUONLINE  
}OnuStatus_t;

/*extern STATUS setOnuStatus( const ulong_t slot, const ulong_t port, const ulong_t onuid, OnuStatus_t  status );*/

/*前三个参数定位ONU，最后一个参数是状态：1 – ONU在线 0：ONU离线
extern unsigned long MAKEDEVID(unsigned long slot, unsigned long port, unsigned onuId );*/


#ifdef DEVICE_MGMT
#endif
extern int DeviceTypeGet( DeviceIndex_S  DevIdx,  int *type );
extern int DeviceNameGet(DeviceIndex_S  DevIdx,  char *Name, int *NameLen );
extern int DeviceNameSet(DeviceIndex_S  DevIdx,  char *Name, int NameLen );
extern int DeviceDescGet(DeviceIndex_S  DevIdx,  char *Desc, int *Len );
extern int DeviceDescSet(DeviceIndex_S  DevIdx,  char *Desc, int len );
extern int DeviceLocationGet( DeviceIndex_S  DevIdx,  char *location, int *Len );
extern int DeviceLocationSet(DeviceIndex_S  DevIdx,  char *location, int  len );
extern int DeviceVendorGet(DeviceIndex_S  DevIdx,  char *Name, int *NameLen );
extern int DeviceFrimwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len );
extern int DeviceSoftwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len );
extern int DeviceHardwareVersionGet( DeviceIndex_S  DevIdx,  char *version, int *len );
extern int DeviceOperStatusGet( DeviceIndex_S  DevIdx,  int *OperStaus );
/*extern int DeviceAlarmStatusGet( DeviceIndex_S  DevIdx,  int *AlarmStaus );*/
extern int DeviceMacAddrGet( DeviceIndex_S  DevIdx,  char *MacAddr, int *len );
extern int DeviceMacAddrSet( DeviceIndex_S  DevIdx,  char *MacAddr, int len );
extern int DeviceLastChangeGet( DeviceIndex_S  DevIdx,  int *LastChange );
extern int DeviceUpTimeGet( DeviceIndex_S  DevIdx,  int *Uptime );
extern int DeviceUpDateTimeGet(DeviceIndex_S  DevIdx, sysDateAndTime_t *pTime );
extern int DeviceAlarmMaskGet(DeviceIndex_S  DevIdx, unsigned long *pMask );
extern int DeviceAlarmMaskSet(DeviceIndex_S  DevIdx,  unsigned long mask );
extern int DeviceModelGet( DeviceIndex_S  DevIdx,  char *pModelStr, int *Len );
extern int DeviceResetSet( DeviceIndex_S  DevIdx,  int Reset );
extern int DeviceResetGet( DeviceIndex_S  DevIdx,  int *Reset );
extern int DeviceOnuTdGet( DeviceIndex_S  DevIdx,  int *Td );

#ifdef PON_PORT_FUNC
#endif
extern int  PonPortPartnerDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev );
extern int  PonPortPartnerBrdGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Brd );
extern int  PonPortPartnerPortGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *PonIdx );
extern int  PonPortProtectDevGet( DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Dev );
extern int  PonPortProtectBrdGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Brd );
extern int  PonPortProtectBrdGet(DeviceIndex_S  DevIdx,  int brdIdx, int portIdx, int *Brd );
extern int PonPortTypeGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *type );
extern  int PonPortMaxOnuGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OnuNum );
extern int PonPortCurrOnuGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OnuNum );
extern int PonPortOperStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *OperStatus );

extern int PonPortAlarmStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long *AlarmStatus );
extern int PonPortAlarmMaskGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long *AlarmMask);
extern int PonPortAlarmMaskSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned long AlarmMask);
extern int PonPortMaxBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* MaxBw );
extern int PonPortActBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* ActBw );
extern int PonPortRemainBWGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int* RemainBw );
extern int PonPortApsCtrlSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  ctrl );
extern int PonPortApsStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *ApsStatus );
extern int PonPortEncryptSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  cryptionDirection );
extern int PonPortEncryptGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *cryptionDirection );

extern int PonPortAdminStatusSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int AdminStatus );
extern int PonPortAdminStatusGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, int *AdminStatus );
extern int PonPortReset( DeviceIndex_S DevIdx, int brdIdx, int  portIdx );
extern int OnuTrafficServiceSet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int EnableFlag );
extern int OnuTrafficServiceGet( DeviceIndex_S DevIdx, int brdIdx, int  portIdx, unsigned int  *EnableFlag );

#ifdef LLIDTABLE
#endif
extern int  findFirstLlid( unsigned long *slot_ret, unsigned long *port_ret, unsigned long *onuId_ret, unsigned long *llid_ret );
extern int   findNextLlid( unsigned long slot, unsigned long port, unsigned long onuId, const unsigned long llid,
				unsigned long *slot_ret, unsigned long *port_ret, unsigned long *onuId_ret, unsigned long *llid_ret);
extern int  LLIDEntryIndexCheck( DeviceIndex_S DevIdx, unsigned long  LlidIdx );
extern int PonPortLlidTypeGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf );
extern int PonPortLlidTypeSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val );
extern int PonPortLlidOltBrdGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf );
extern int PonPortLlidOltBrdSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long Val );
extern int PonPortLlidOltPortGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf );
extern int PonPortLlidOltPortSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long Val );
extern int PonPortLlidOnuBrdGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf );
extern int PonPortLlidOnuBrdSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long Val );
extern int PonPortLlidOnuPortGet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long *pValBuf );
extern int PonPortLlidOnuPortSet( DeviceIndex_S DevIdx, unsigned long  llidIdx, unsigned long Val );
extern int PonPortLlidllidGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long  *pValBuf );
extern int  PonPortLlidUpLinkBWGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf );
extern int  PonPortLlidUpLinkBWGet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *UplinkClass, unsigned long *UplinkDelaly, unsigned long *assured_bw, unsigned long *best_effort_bw );
extern int  PonPortLlidUpLinkBWSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val );
extern int  PonPortLlidUpLinkBWSet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long UplinkClass, unsigned long UplinkDelaly, unsigned long assured_bw, unsigned long best_effort_bw );
extern int  PonPortLlidDownLinkBWGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf );
extern int  PonPortLlidDownLinkBWGet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *DownlinkClass, unsigned long *DownlinkDelaly, unsigned long *assured_bw, unsigned long *best_effort_bw );
extern int  PonPortLlidDownLinkBWSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val );
extern int  PonPortLlidDownLinkBWSet_1( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long DownlinkClass, unsigned long DownlinkDelaly, unsigned long assured_bw, unsigned long best_effort_bw );
extern int PonPortLlidDescGet( DeviceIndex_S DevIdx, unsigned long llidIdx,  char *pValBuf, unsigned long  *pValLen );
extern int PonPortLlidDescSet( DeviceIndex_S DevIdx, unsigned long llidIdx,  char *pValBuf, unsigned long  valLen );
extern int  PonPortLlidSupportedMacNumGet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long *pValBuf );
extern int  PonPortLlidSupportedMacNumSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val );
extern int  PonPortLlidSupportedMacNumSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long val );
extern int  PonPortLlidRowStatusSet( DeviceIndex_S DevIdx, unsigned long llidIdx, unsigned long  val );

#ifdef ONU_SW_UPDATE 
#endif
extern  int  OnuSwUpdateEnableSet(DeviceIndex_S DevIdx,  unsigned long flag);
extern int  OnuSwUpdateEnableGet( DeviceIndex_S DevIdx, unsigned long *pVal);

extern int  OnuSwUpdateStatusGet(DeviceIndex_S DevIdx, unsigned long *pVal);

#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
extern int PonPortTransPowerGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *tpowr );
extern int PonPortRecvPowerGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *powr );
extern int PonPortTempGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *temperature );
extern int PonPortVoltageGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *vol );
extern int PonPortCurrentGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *cur );
extern int PonPortSigMonEnGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *cur );
extern int PonPortSigMonEnSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val );
extern int PonPortSigMonIntervalGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *interval );
extern int PonPortSigMonIntervalSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val );
extern int PonPortAlwaysOnEnGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *en );
extern int PonPortAlwaysOnEnSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val );
/*extern void ModifiedPonPortSampleInterval( LONG val);*/

extern int UplinkPortBiasCurrentGet(ULONG brdIdx,ULONG  ethIdx,LONG *val);
extern int UplinkSfpWorkVoltageGet(ULONG brdIdx,ULONG  ethIdx,LONG *val);
extern int UplinkSfpModuleTemperatureGet(ULONG brdIdx,ULONG  ethIdx,LONG *val);
extern int UplinkSfpReceiverPowerGet(ULONG brdIdx,ULONG  ethIdx,LONG *val);
extern int UplinkSfpTransmissionPowerGet(ULONG brdIdx,ULONG  ethIdx,LONG *val);

#endif

#endif
