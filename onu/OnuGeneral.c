/***************************************************************
*
*						Module Name:  OnuGeneral.c
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
*   Date: 			2006/05/15
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ------- |-------------|------------------ 
**  2006/05/15 |   chenfj          |     create 
**------------|-------------|------------------
**
** 1 add by chenfj 2006/09/19
**    #2604 问题telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了
** 2  modify by chenfj 2006/09/20 
**    #2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置
** 3 deleted by chenfj 2006/10/11 
**    删除ONU设备信息显示中LLID项
** 4 added by chenfj 2006/10/19
**    #2906问题建议不存在的ONU不要再显示信息了,另外ONU端口号没有做限制
** 5 modified by chenfj 2006/10/20
**	  #2939问题单设置不准确的时间后，启动时信息表述不恰当
**	  #2911问题单设置系统时间为2000-2-2后,重启系统,提示错误
** 6 modified by chenfj 2006/10/30 
**    onu带宽分配：
**      ONU注册时，若已分配带宽，使用已分配的带宽
**       反之，使用默认带宽
**    onu被删除时，收回已分配的带宽。对未注册过或未配置或被删除的ONU，不分配带宽
** 7 modified  by chenfj 2006/11/25
**    #3277  对所有ONU进行远程升级时，版本已经是最新的ONU也进行了升级，建议优化
** 8  modified by chenfj 2006/11/28
**	 问题单#3267: 在ONU节点下学不到OLT上的千兆口发下来的源MAC,onu收不到OLT发下来的包 
** 9 added by chenfj 2006/12/06
**   set onu llid peer-to-peer,设置同一个PON 端口下ONU之间互通( 不经过交换板)
** 10 added by chenfj 2006-12-21 
**    设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
** 11  modified by chenfj 2007/02/07 
**   # 3568问题单 GW-EPON-DEV-MIB中自动升级状态的属性修改影响了升级状态属性
** 12  added by chenfj 2007-02-07
**     增加ONU自动配置
** 13 modified by chenfj 2007-03-25
**	#3907 问题单: 关于ONU带宽分配的两个问题
** 14 add by chenfj 2007/6/1
**	 增加设置/清除ONU 数据包原MAC 过滤
**  15  added by chenfj 2007/6/8
**   增加ONU 数据包IP/port 过滤
** 16   modified by chenfj  2007-7-9
**  	问题单:#4816 命令show onu infor 参数控制错误 
** 17 	 modified by chenfj 2007-7-16
**		增加一个带宽比例因子,用于测试带宽精度
**	18  added by chenfj 2007-8-14 
**   	通过PMC提供的API, ONU文件升级
** 19  added by chenfj 2008-8-30
**      增加ONU 光功率检测
** 20   modified by chenfj 2008-7-9
*         增加GFA6100 产品支持; 
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include "gwEponSys.h"
/*#include  "cli/cl_vty.h"*/
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "tdm_apis.h"
#endif
#include "onu/onuOamUpd.h"
#include "onu/onuConfMgt.h"
#include "onu/Onu_manage.h"

OLT_onu_table_t  olt_onu_global_table;
PON_address_record_t Mac_addr_table[PON_ADDRESS_TABLE_SIZE];
PON_address_vlan_record_t Mac_addr_vlan_table[PON_ADDRESS_TABLE_SIZE];/*for GPON by jinhl*/
unsigned char GW_private_MAC[] = { 0x00,0x0f,0xe9, 0xff };

#if 0
ONUTable_S  OnuMgmtTable[SYS_MAX_PON_ONUNUM];
#else
ONUTable_S  *OnuMgmtTable;
#endif

int onu_mgmt_debug = 0;
unsigned long OnuMgmtTableDataSemId = 0;
unsigned long OnuEncryptSemId =0;
unsigned int Bata_ratio = 1000;


/*extern long UpdateOnuAppReq(short int usPonID, short int usOnuID, char *type, char *name, long ltype, struct vty *vty );*/
extern int  CTC_getDeviceCapEthPortNum(short int PonPortIdx, short int OnuIdx, int *FE_num);
extern int  CTC_getDeviceCapiadPotsPortNum(short int PonPortIdx, short int OnuIdx, int *POTS_num);
extern int  CTC_getDeviceCapE1PortNum(short int PonPortIdx, short int OnuIdx, int *E1_num);
extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
extern int ShowExtOnuDeviceInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty );
extern int GetMaxOnuByPonPort(short int PonPortIdx);

PON_olt_monitoring_configuration_t OnuDefaultAlarmConfiguration = {0};

/*unsigned char Invalid_Mac_Addr[BYTES_IN_MAC_ADDRESS] = { 0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char Invalid_Mac_Addr1[BYTES_IN_MAC_ADDRESS] = { 0x00,0x00,0x00,0x00,0x00,0x00};*/
#if 0
unsigned char *OnuActivatedMode[] = 
	{
	(unsigned char *)"Mode_Onu_ON",
	(unsigned char *)"Mode_Onu_OFF",
	(unsigned char *)"Mode_Onu_Pending"
	};
#endif
unsigned char *OnuCurrentStatus[] = 
	{
	(unsigned char *)"unknown  ",
	(unsigned char *)" up      ",
	(unsigned char *)"down     ",
	(unsigned char *)"pend     ",
	(unsigned char *)"dormant  ",
	(unsigned char *)"powerDown"
	};

unsigned char *v2r1Enable[] =
	{
	(unsigned char *)"unknown",
	(unsigned char *)"enable",
	(unsigned char *)"disable"
	};

unsigned char *v2r1Start[] = 
	{
	(unsigned char *)"unknown",
	(unsigned char *)"started",
	(unsigned char *)"notstarted"
	};

unsigned char *v2r1EncryptDirection[] = 
	{
	(unsigned char *)"unknown",
	(unsigned char *)"none",
	(unsigned char *)"down",
	(unsigned char *)"up_and_down"
	};

unsigned char *v2r1AddrTableAgeingMode[] = 
	{
	(unsigned char *)"dynamic",
	(unsigned char *)"static ",
	(unsigned char *)"dynamic_and_static"
	};

 unsigned char *OnuSwUpdateStatus[] = 
 	{
	(unsigned char *)"unknown",
	(unsigned char *)"noop",
	(unsigned char *)"--",
	(unsigned char *)"inProcess",
	(unsigned char *)"forbidded"
 	};

 unsigned char *OnuImageType[] =
{
	(unsigned char *)"unknown",	
	(unsigned char *)"voice",
	(unsigned char *)"fpga",
	(unsigned char *)"epon",
	(unsigned char *)""
};

OnuConfigInfo_S OnuConfigDefault/*={{0}}*/;

PON_onu_address_table_record_t  MAC_Address_Table[8192]/*={{0}}*/;

#if 0
char OnuVendorLocationDefault[MAXVENDORLOCATIONLEN] = "北京市海淀区上地西路38 号";
char OnuVendorContactDefault[MAXVENDORCONTACTLEN] = "Tel: 010-62961177, Fax: 010-82899881";
char OnuDeviceTypeDefault[MAXDEVICETYPELEN] = "V2R1 ONU";
#endif

/*unsigned char  OnuMacAddrDefault[6] ={ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };*/
ULONG conf_associate_share=0;
ULONG Timeout_delete_authentry=0;

unsigned short int OnuIdxDefault = BYTE_FF;
unsigned int  MaxMACDefault = ONU_DEFAULT_MAX_MAC/*128*/;  /* 512 is modified as 128 by liwei056@2009-12-30 for ONU's Default Mac-Limit  */

/*added by luh 2014-10-23*/
unsigned int  MaxOnuDefault = DAFAULT_MAX_ONU; /*始终默认为64*/
unsigned int  glMaxOnuTable[14][16] = {0};

unsigned int  UplinkBandwidthDefault = ONU_DEFAULT_BW;
unsigned int  DownlinkBandwidthDefault = ONU_DEFAULT_BW;
	
unsigned int  EncryptTypeDefault = 0;	
unsigned int  EncryptEnableDefault = V2R1_DISABLE;	
unsigned int  EncryptStatusDefault =  V2R1_NOTSTARTED;
unsigned int  EncryptDirectionDefault = PON_ENCRYPTION_PURE; 
unsigned int  EncryptKeyTimeDefault = (SECOND_5*24);   /* 2 分钟*/

unsigned char  CTC_EncryptKeyTimeDefault = (SECOND_10/SECOND_1); /* unit : S */
unsigned char  CTC_EncryptTimeingThreshold = 0; /* unit : S */
unsigned short int  CTC_EncryptNoReplyTime = 30;  /* unit : 100ms */
int  CTCEncryptCtrlDefault = V2R1_CTC_ENCRYPT_NOOP;
int  CTCEncryptStatusDefault = V2R1_CTC_ENCRYPT_NOOP;

unsigned long  AlarmMaskDefault = 0;
unsigned long  OptOnuAlarmMask = EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH | 
		EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_VOLTAGE;

MacTable_S  *StaticMacDefault = 0;	

unsigned char  OnuOperStatusDefault = ONU_OPER_STATUS_DOWN;
unsigned char  OnuAdminStatusDefault = ONU_ADMIN_STATUS_UP;
unsigned char  OnuSoftwareUpdateCtrl =  ONU_SW_UPDATE_ENABLE;
unsigned char  OnuSoftwareUpdateFlag = ONU_SW_UPDATE_STATUS_NOOP;

int VOS_Ticks_Per_Seconds = VOS_TICK_SECOND;
int ONU_FLASH_BURNING_WAIT = 5;

extern int onuIdToOnuIndex( ushort_t ponId, ushort_t llId );
extern int llidActBWExceeding_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx, ulong_t llidIdx );

#ifdef ONUINFO_INIT
#endif

/*****************************************************
 *
 *    Function:  ThisIsValidOnu( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx 
 *                 short int OnuIdx
 *                
 *    Desc:   判断onu(PonPortIdx, OnuIdx) 是否已存在于ONU管理表中
 *
 *    Return:  ROK or  V2R1_ONU_NOT_EXIST
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
#define HEX_GEN_BASE_NUM                   16
#define HEX_GEN_ASCII_PREFIX_LEN           2
#define HEX_GEN_ASCII_STR_LEN_PER_OCTET    2
#define UINT32_BYTE_GEN_COUNT              4
#define INT32_HEX_GEN_ASCII_NUM            (UINT32_BYTE_GEN_COUNT * HEX_GEN_ASCII_STR_LEN_PER_OCTET)
      
int VOS_ConvertHexAsciiStrToBinOctets(char *strP, char *bufP, short int *bufLenP)
{

    uint32 intVal;
    uint32 strLength;
    uint16 dataLength;
    uint16 indexI;
    char savedChar;


    strLength = VOS_StrLen(strP);
    if ((strLength != (*bufLenP * HEX_GEN_ASCII_STR_LEN_PER_OCTET))
      || (strLength % HEX_GEN_ASCII_STR_LEN_PER_OCTET))
    {
        return (-2);
    }

    for (indexI = 0; indexI < strLength; indexI++)
    {
        if (isxdigit(strP[indexI]) == 0)
        {
            break;
        }
    }
    if (indexI != strLength)
    {
        return (-1);
    }

    dataLength = 0;
    while (strLength >= INT32_HEX_GEN_ASCII_NUM)
    {
        savedChar = strP[INT32_HEX_GEN_ASCII_NUM];
        strP[INT32_HEX_GEN_ASCII_NUM] = '\0';
        intVal = strtoul(strP, NULL, HEX_GEN_BASE_NUM);
        *bufP++ = (uint8)((intVal & 0xFF000000) >> 24);
        *bufP++ = (uint8)((intVal & 0x00FF0000) >> 16);
        *bufP++ = (uint8)((intVal & 0x0000FF00) >> 8);
        *bufP++ = (uint8)(intVal & 0x000000FF);
        strP[INT32_HEX_GEN_ASCII_NUM] = savedChar;
        strP += INT32_HEX_GEN_ASCII_NUM;
        strLength -= INT32_HEX_GEN_ASCII_NUM;
        dataLength += UINT32_BYTE_GEN_COUNT;
    }

    if (strLength > 0)
    {
        intVal = strtoul(strP, NULL, HEX_GEN_BASE_NUM);
        if (strLength > 6)
        {
            *bufP++ = (uint8)((intVal & 0xFF000000) >> 24);
            dataLength++;
        }
        if (strLength > 4)
        {
            *bufP++ = (uint8)((intVal & 0x00FF0000) >> 16);
            dataLength++;
        }
        if (strLength > 2)
        {
            *bufP++ = (uint8)((intVal & 0x0000FF00) >> 8);
            dataLength++;
        }
        *bufP++ = (uint8)(intVal & 0x000000FF);
        dataLength++;
    }

    *bufLenP = dataLength;

    return (0);

}

int ThisIsValidOnu( short int PonPortIdx, short int OnuIdx )
{
    int rc = ROK;
    CHAR *pMac;

    if ( OLT_ISLOCAL(PonPortIdx) )
    {
        CHECK_ONU_RANGE;
        ONU_MGMT_SEM_TAKE;
        if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].IsGponOnu)
        {
            if(VOS_StrLen(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.DeviceSerial_No) == 0)
                rc = V2R1_ONU_NOT_EXIST;
        }
        else
        {
            pMac = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr;
            if( MAC_ADDR_IS_ZERO(pMac) || MAC_ADDR_IS_BROADCAST(pMac) )
                rc = V2R1_ONU_NOT_EXIST;
        }
        ONU_MGMT_SEM_GIVE;
    }
    else
    {
        rc = OnuMgtAdv_IsValid(PonPortIdx, OnuIdx);
    }
    
    return rc;
}
int ThisIsGponOnu(short int PonPortIdx, short int OnuIdx)
{
    int rc = FALSE;
    
    CHECK_ONU_RANGE; 
    if ( OLT_ISLOCAL(PonPortIdx) )
    {        
        ONU_MGMT_SEM_TAKE;
        if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].IsGponOnu)
        {
            rc = TRUE;
        }
        ONU_MGMT_SEM_GIVE;
    }
    return rc;
}
#if 0
/*****************************************************
 *
 *    Function: ClearOnuMgmtTable()
 *
 *    Param:   none
 *                
 *    Desc:   clear all onu mgmt table data
 *
 *    Return:  ROK 
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int ClearOnuMgmtTable()
{
	int i;

	for( i = 0; i<MAXONU; i++ ){
		VOS_MemSet( &(OnuMgmtTable[i]), 0, sizeof( ONUTable_S ) );
		OnuMgmtTable[i].AdminStatus = ONU_ADMIN_STATUS_UP;
		OnuMgmtTable[i].OperStatus = ONU_OPER_STATUS_DOWN;
		OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
		OnuMgmtTable[i].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
		OnuMgmtTable[i].UsedFlag = NOT_ONU_USED_FLAG;
		OnuMgmtTable[i].EncryptEnable =V2R1_DISABLE;
		OnuMgmtTable[i].EncryptDirection = PON_ENCRYPTION_PURE;
		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.MacAddr, Invalid_Mac_Addr, BYTES_IN_MAC_ADDRESS );
		}
	/*
	UsedFlagChange = V2R1_DATA_NOT_CHANGED;
	OnuMapChange = V2R1_DATA_NOT_CHANGED;
	*/
	return ( ROK );
}

int ClearOneOnuByMAC( unsigned char *MacAddress )
{
	int i;

	for( i = 0; i< MAXONU; i ++ ){
		if( VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.MacAddr, MacAddress, BYTES_IN_MAC_ADDRESS ) == 0 ){
			VOS_MemSet( &(OnuMgmtTable[i]), 0, sizeof( ONUTable_S ) );
			OnuMgmtTable[i].AdminStatus = ONU_ADMIN_STATUS_UP;
			OnuMgmtTable[i].OperStatus = ONU_OPER_STATUS_DOWN;
			OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
			OnuMgmtTable[i].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
			OnuMgmtTable[i].UsedFlag = NOT_ONU_USED_FLAG;
			OnuMgmtTable[i].EncryptEnable =V2R1_DISABLE;
			OnuMgmtTable[i].EncryptDirection = PON_ENCRYPTION_PURE;
			VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.MacAddr, Invalid_Mac_Addr, BYTES_IN_MAC_ADDRESS );
			return( ROK );
			}
		}
	return( RERROR );
}
int  ClearOneOnuByName( unsigned char *Name )
{
	int i;

	for( i=0; i< MAXONU; i++ )
		{
		if( VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.CustomerName, Name, sizeof( Name ) ) == 0 )
			{
			VOS_MemSet( &(OnuMgmtTable[i].Index), 0, sizeof( ONUTable_S));
			OnuMgmtTable[i].AdminStatus = ONU_ADMIN_STATUS_UP;
			OnuMgmtTable[i].OperStatus = ONU_OPER_STATUS_DOWN;
			OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
			OnuMgmtTable[i].UsedFlag = NOT_ONU_USED_FLAG;
			OnuMgmtTable[i].EncryptEnable =V2R1_DISABLE;
			OnuMgmtTable[i].EncryptDirection = PON_ENCRYPTION_PURE;
			VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.MacAddr, Invalid_Mac_Addr, BYTES_IN_MAC_ADDRESS );
			return( ROK );
			}
		}
	return( RERROR );	
}
int  ClearOneOnuByIdx( short int OnuIdx )
{
	short int i;
	
	i = OnuIdx;
	VOS_MemSet( &(OnuMgmtTable[i].Index), 0, sizeof( ONUTable_S ) );
	OnuMgmtTable[i].LLID = INVALID_LLID;
	OnuMgmtTable[i].AdminStatus = ONU_ADMIN_STATUS_UP;
	OnuMgmtTable[i].OperStatus = ONU_OPER_STATUS_DOWN;
	OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
	OnuMgmtTable[i].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
	OnuMgmtTable[i].UsedFlag = NOT_ONU_USED_FLAG;
	OnuMgmtTable[i].EncryptEnable =V2R1_DISABLE;
	OnuMgmtTable[i].EncryptDirection = PON_ENCRYPTION_PURE;
	OnuMgmtTable[i].EncryptFirstTime = V2R1_ENABLE;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.MacAddr, Invalid_Mac_Addr, BYTES_IN_MAC_ADDRESS );
	return( ROK );
}
#endif

int  ClearOnuLlidRunningData( short int PonPortIdx, short int OnuIdx, short int LLIDIdx )
{
    OnuLLIDTable_S *pstLlidCfg;
	int onuEntry;
	int llidNum;
    short int llid;

	onuEntry = MAXONUPERPON * PonPortIdx + OnuIdx;
    pstLlidCfg = &OnuMgmtTable[onuEntry].LlidTable[LLIDIdx];

	ONU_MGMT_SEM_TAKE;

    llid = (short int)pstLlidCfg->Llid;
    pstLlidCfg->Llid = INVALID_LLID;
    pstLlidCfg->ActiveUplinkBandwidth = 0;
    pstLlidCfg->ActiveDownlinkBandwidth = 0;

    llidNum = --OnuMgmtTable[onuEntry].llidNum;

    if ( OnuMgmtTable[onuEntry].LLID == llid )
    {
        if ( llidNum > 0 )
        {
            OnuLLIDTable_S *pstLlidTbl = OnuMgmtTable[onuEntry].LlidTable;
            
            for ( llidNum = 0; llidNum < MAXLLIDPERONU; llidNum++ )
            {
                if ( INVALID_LLID != (llid = (short int)pstLlidTbl[llidNum].Llid) )
                {
                    OnuMgmtTable[onuEntry].LLID = llid;
                    break;
                }
            }
        }
        else
        {
            OnuMgmtTable[onuEntry].LLID = INVALID_LLID;
        }
    }
    
	ONU_MGMT_SEM_GIVE;

    return llidNum;
}

int  ClearOnuRunningData( short int PonPortIdx, short int OnuIdx, int ClearFlags )
{
	short int OnuEntry;
	short int llidEntry;
	
	CHECK_ONU_RANGE

    if ( 0 == ClearFlags )
    {
        if ( ONU_ADAPTER_RPC < ONU_GetIFType(PonPortIdx, OnuIdx) )
        {
        	pon_lose_onu(PonPortIdx, OnuIdx);
        }
    }
    
	OnuEntry = MAXONUPERPON * PonPortIdx + OnuIdx;

	/* modified by xieshl 20110106, 这里不能返回，不管什么情况都要清配置，
	防止ONU注册过程失败(如带宽配置错误被踢下去后，OperStatus=DOWN，
	但GetEUQflag不清除，会导致ONU将不能正常获取设备信息)，重新注册
	时将被加入pending队列，无法马上获得注册机会*/
	/*if( OnuMgmtTable[OnuEntry].OperStatus == ONU_OPER_STATUS_DOWN) return( ROK );*/

	ONU_MGMT_SEM_TAKE;

#if 0	
	VOS_MemSet( OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion, 0, MAXSWVERSIONLEN);
	VOS_MemSet( OnuMgmtTable[OnuEntry].DeviceInfo.HwVersion, 0, MAXHWVERSIONLEN);
	VOS_MemSet( OnuMgmtTable[OnuEntry].DeviceInfo.BootVersion, 0, MAXHWVERSIONLEN);
	VOS_MemSet( OnuMgmtTable[OnuEntry].DeviceInfo.FwVersion, 0, MAXFWVERSIONLEN);
	OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen = 0;
	OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen = 0;
	OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen= 0;
	OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen = 0;

	OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_OTHER; /*DEVICE_OTHER */
#endif	
    OnuMgmtTable[OnuEntry].OnuAbility = 0;/*added by luh 2012-11-1*/
	OnuMgmtTable[OnuEntry].PonIdx = MAXPON;
	OnuMgmtTable[OnuEntry].LLID = INVALID_LLID;
	OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DOWN;
	OnuMgmtTable[OnuEntry].ExtOAM = FALSE;
	/*OnuMgmtTable[OnuEntry].SoftwareUpdateStatus = OnuConfigDefault.SoftwareUpdateFlag;*/
	OnuMgmtTable[OnuEntry].RTT = NOT_KNOWN;
	/*VOS_MemSet( OnuMgmtTable[OnuEntry].AlarmStatus, 0, sizeof( OnuMgmtTable[OnuEntry].AlarmStatus ));*/	/* removed 20070705 */
	OnuMgmtTable[OnuEntry].AlarmStatus = 0;
	/*OnuMgmtTable[OnuEntry].AlarmMask = 0;*/
	
	OnuMgmtTable[OnuEntry].ProtectType = NOT_KNOWN;
	OnuMgmtTable[OnuEntry].PowerStatus = NOT_KNOWN;
	OnuMgmtTable[OnuEntry].HaveBattery = NOT_KNOWN;
	OnuMgmtTable[OnuEntry].BatteryCapability = NOT_KNOWN;
	VOS_MemSet( OnuMgmtTable[OnuEntry].SequenceNo, 0, PON_AUTHENTICATION_SEQUENCE_SIZE );
	OnuMgmtTable[OnuEntry].OAM_Ver = OAM_STANDARD_VERSION_1_2;
	/*
	OnuMgmtTable[OnuEntry].EncryptEnable = EncryptEnableDefault;
	OnuMgmtTable[OnuEntry].EncryptDirection = EncryptDirectionDefault;
	OnuMgmtTable[OnuEntry].EncryptKeyTime = SECOND_5;
      */
	OnuMgmtTable[OnuEntry].EncryptStatus = EncryptStatusDefault;
	OnuMgmtTable[OnuEntry].EncryptCounter = (OnuMgmtTable[OnuEntry].EncryptKeyTime/SECOND_1 );
	OnuMgmtTable[OnuEntry].EncryptFirstTime = V2R1_ENABLE;
	
	OnuMgmtTable[OnuEntry].LlidTable[0].Llid = INVALID_LLID;
	OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
	OnuMgmtTable[OnuEntry].LlidTable[0].ActiveUplinkBandwidth = 0;
	OnuMgmtTable[OnuEntry].LlidTable[0].ActiveDownlinkBandwidth = 0;
	
	OnuMgmtTable[OnuEntry].UsedFlag = NOT_ONU_USED_FLAG;
	/*OnuMgmtTable[OnuEntry].PowerOn = V2R1_POWER_ON;*/

	if( OnuMgmtTable[OnuEntry].NeedDeleteFlag == TRUE )
	{
		/*if( EVENT_REGISTER == V2R1_ENABLE )
			sys_console_printf("\r\n this entry should be deleted \r\n");*/
		/*
		OnuMgmtTable[PonPort_id * MAXONUPERPON + OnuIdx ].NeedDeleteFlag == FALSE;
		VOS_MemSet( OnuMgmtTable[PonPort_id * MAXONUPERPON + OnuIdx ].DeviceInfo.MacAddr, 0xff, BYTES_IN_MAC_ADDRESS );
		*/
		/* modified by chenfj 2006-10-30 */
		PonPortTable[PonPortIdx].DownlinkProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr -0; /*OnuConfigDefault.DownlinkBandwidth;*/
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr - 0;/* OnuConfigDefault.UplinkBandwidth;*/
#ifdef PLATO_DBA_V3
		PonPortTable[PonPortIdx].ProvisionedBW += OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed - 0;
#endif
		InitOneOnuByDefault( PonPortIdx, OnuIdx);		
	}
    /* B--added by liwei056@2011-1-27 for DefaultBW */
    else
    {
        OnuLLIDTable_S *pstLlidCfg;
        OnuMgmtTable[OnuEntry].ActiveDownlinkBandwidth = 0;
        OnuMgmtTable[OnuEntry].ActiveUplinkBandwidth = 0;

        OnuMgmtTable[OnuEntry].llidNum = 0;
		for( llidEntry=0;llidEntry<MAXLLIDPERONU;llidEntry++)
		{
		    pstLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[llidEntry]);

            pstLlidCfg->OnuhandlerStatus = ONU_REGISTER_STATUS_IDLE;
            pstLlidCfg->OnuEventRegisterTimeout = 0;

            pstLlidCfg->Llid = INVALID_LLID;
            pstLlidCfg->ActiveUplinkBandwidth = 0;
            pstLlidCfg->ActiveDownlinkBandwidth = 0;

            if ( 0 != pstLlidCfg->BandWidthIsDefault )
            {
                /* 恢复默认带宽 */
                if ( OLT_CFG_DIR_UPLINK & pstLlidCfg->BandWidthIsDefault )
                {
					GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &pstLlidCfg->UplinkBandwidth_gr, &pstLlidCfg->UplinkBandwidth_be);
                }

                if ( OLT_CFG_DIR_DOWNLINK & pstLlidCfg->BandWidthIsDefault )
                {
					GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &pstLlidCfg->DownlinkBandwidth_gr, &pstLlidCfg->DownlinkBandwidth_be);
                }
            }
		}
    }
    /* E--added by liwei056@2011-1-27 for DefaultBW */

	OnuMgmtTable[OnuEntry].GetEUQflag = FALSE;
	OnuMgmtTable[OnuEntry].GetEUQCounter = 0;

	OnuMgmtTable[OnuEntry].CTC_EncryptStatus = OnuConfigDefault.CTC_EncryptStatus;
	/*
	OnuMgmtTable[OnuEntry].CTC_EncryptCtrl = OnuConfigDefault.CTC_EncryptCtrl ;

	OnuMgmtTable[OnuEntry].FEC_ability = OnuConfigDefault.FEC_ability;
	OnuMgmtTable[OnuEntry].FEC_Mode = OnuConfigDefault.FEC_mode;
	*/

	OnuMgmtTable[OnuEntry].BurningFlash = V2R1_DISABLE;
	OnuMgmtTable[OnuEntry].BurningWait = 0;
	
	/*DelOneNodeFromGetEUQ( PonPortIdx, OnuIdx );*/
	
	ONU_MGMT_SEM_GIVE;
	
#if 0 /*def V2R1_MIB*/
	{
		int slot;
		int port ;
		slot  = GetCardIdxByPonChip(PonPortIdx);
		port = GetPonPortByPonChip( PonPortIdx );		
		setOnuStatus( slot,  port, OnuIdx, ONU_OFFLINE );
	}
#endif

#ifdef STATISTIC_TASK_MODULE
	StatsMsgOnuOnSend( PonPortIdx, OnuIdx, ONU_OFFLINE );
#endif

	return( ROK );
}

int InitOnuConfigDefault()
{
	VOS_MemSet( &OnuConfigDefault, 0, sizeof( OnuConfigInfo_S ) );
	
	VOS_StrnCpy( OnuConfigDefault.VendorInfo, OnuVendorInfoDefault, MAXVENDORINFOLEN-1 );
	OnuConfigDefault.VendorInfoLen= VOS_StrLen( OnuConfigDefault.VendorInfo ) ;
	#if 0
	VOS_MemCpy( OnuConfigDefault.VendorLocation, OnuVendorLocationDefault, VOS_StrLen( OnuVendorLocationDefault ) );
	OnuConfigDefault.VendorLocationLen = VOS_StrLen( OnuVendorLocationDefault );
	VOS_MemCpy( OnuConfigDefault.VendorContact, OnuVendorContactDefault, VOS_StrLen( OnuVendorContactDefault ) );
	OnuConfigDefault.VendorContactLen = VOS_StrLen( OnuVendorContactDefault );
	#endif
	VOS_StrnCpy( OnuConfigDefault.DeviceName, OnuDeviceNameDefault, MAXDEVICENAMELEN-1 );
	OnuConfigDefault.DeviceNameLen = VOS_StrLen( OnuConfigDefault.DeviceName );
	#if 0
	VOS_MemCpy( OnuConfigDefault.DeviceType, OnuDeviceTypeDefault, VOS_StrLen( OnuDeviceTypeDefault ) );
	OnuConfigDefault.DeviceTypeLen = VOS_StrLen( OnuDeviceTypeDefault );
	#endif
	VOS_StrnCpy( OnuConfigDefault.Location, OnuLocationDefault, MAXLOCATIONLEN-1 );
	OnuConfigDefault.LocationLen = VOS_StrLen( OnuConfigDefault.Location );

	VOS_StrnCpy( OnuConfigDefault.DeviceDesc, OnuDescDefault, MAXDEVICEDESCLEN-1 );
	OnuConfigDefault.DeviceDescLen = VOS_StrLen( OnuConfigDefault.DeviceDesc );
	
	VOS_MemSet( OnuConfigDefault.MacAddr, 0xff/*OnuMacAddrDefault*/, BYTES_IN_MAC_ADDRESS );
	
	OnuConfigDefault.OnuIdx = OnuIdxDefault;
	/*OnuConfigDefault.MaxMAC = MaxMACDefault;*/
	OnuConfigDefault.AdminStatus = OnuAdminStatusDefault;
	OnuConfigDefault.OperStatus = OnuOperStatusDefault;
	OnuConfigDefault.SoftwareUpdateCtrl = OnuSoftwareUpdateCtrl;
	OnuConfigDefault.SoftwareUpdateFlag = OnuSoftwareUpdateFlag;
	OnuConfigDefault.UplinkBandwidth = PonPortConfigDefault.DefaultOnuBW;/* UplinkBandwidthDefault;*/
	OnuConfigDefault.DownlinkBandwidth = PonPortConfigDefault.DefaultOnuBW; /*DownlinkBandwidthDefault;*/
	OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM = PonPortConfigDefault.DefaultOnuBW;/* UplinkBandwidthDefault;*/
	OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM = PonPortConfigDefault.DefaultXGOnuBW; /*DownlinkBandwidthDefault;*/
	OnuConfigDefault.UplinkBandwidth_XGEPON_SYM = PonPortConfigDefault.DefaultXGOnuBW; /*75Mbps*/
	OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM = PonPortConfigDefault.DefaultXGOnuBW; /*75Mbps*/
	OnuConfigDefault.UplinkBandwidth_GPON = PonPortConfigDefault.DefaultOnuBW; /*7.5Mbps*/
	OnuConfigDefault.DownlinkBandwidth_GPON = PonPortConfigDefault.DefaultGPONOnuBW; /*15Mbps*/
	/*added by chenfj 2006-12-21 
	    设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
	*/

	/* modified for 共进测试chenfj 2007-8-27 */
#ifndef GONGJIN_VERSION
	OnuConfigDefault.UplinkBandwidthBe = ONU_DEFAULT_BE_BW;  /* OnuConfigDefault.UplinkBandwidth + BITS_PER_SECOND_1M */
	OnuConfigDefault.DownlinkBandwidthBe = ONU_DEFAULT_BE_BW; /* OnuConfigDefault.DownlinkBandwidth + BITS_PER_SECOND_1M */
	OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM = ONU_DEFAULT_BE_BW;  /* 1000Mbps*/ 
	OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM = GW10G_ONU_DEFAULT_BE_BW; /* 1000Mbps*/ 
	OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM = GW10G_ONU_DEFAULT_BE_BW; /*1000Mbps*/
	OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM = GW10G_ONU_DEFAULT_BE_BW; /*1000Mbps*/
	OnuConfigDefault.UplinkBandwidthBe_GPON = GPON_ONU_DEFAULT_BE_BW; /*1000Mbps*/
	OnuConfigDefault.DownlinkBandwidthBe_GPON = GPON_ONU_DEFAULT_BE_BW; /*1000Mbps*/
#else 
	OnuConfigDefault.UplinkBandwidthBe = BITS_PER_SECOND_1M * 960;
	OnuConfigDefault.DownlinkBandwidthBe = BITS_PER_SECOND_1M * 960;
#endif
	
	OnuConfigDefault.UplinkClass = PRECEDENCE_OF_FLOW_0/*PRECEDENCE_OF_FLOW_2*/;/*解决16epon延时问题*/
	OnuConfigDefault.UplinkDelay = V2R1_DELAY_LOW;
	OnuConfigDefault.DownlinkClass = PRECEDENCE_OF_FLOW_2;
	OnuConfigDefault.DownlinkDelay = V2R1_DELAY_LOW;
	
	OnuConfigDefault.EncryptEnable = EncryptEnableDefault;
	OnuConfigDefault.EncryptStatus = EncryptStatusDefault;
	OnuConfigDefault.EncryptDirection = EncryptDirectionDefault;
	OnuConfigDefault.EncryptKeyTime = EncryptKeyTimeDefault;
	/*VOS_MemCpy( OnuConfigDefault.AlarmMask, AlarmMaskDefault, (MAX_ONU_ALARM_NUM/8));*/
	OnuConfigDefault.AlarmMask = AlarmMaskDefault;
	
	OnuConfigDefault.StaticMac = ( MacTable_S *)StaticMacDefault;
	
	/* 2007/6/1 增加ONU 数据包原MAC 过滤*/
	OnuConfigDefault.FilterSaMac = ( MacTable_S *)StaticMacDefault;

	/* 2007/6/8 增加ONU 数据包IP/port 过滤*/
	OnuConfigDefault.Filter_SIp = ( IpTable_S *)StaticMacDefault;
	OnuConfigDefault.Filter_SIp_tcp = (Ip_Port_Table_S *)StaticMacDefault;
	OnuConfigDefault.Filter_SIp_udp = (Ip_Port_Table_S *)StaticMacDefault;
	OnuConfigDefault.Filter_DIp = ( IpTable_S *)StaticMacDefault;
	OnuConfigDefault.Filter_DIp_tcp = (Ip_Port_Table_S *)StaticMacDefault;
	OnuConfigDefault.Filter_DIp_udp = (Ip_Port_Table_S *)StaticMacDefault;

	/* 2007/6/12 增加ONU数据包VID过滤*/
	OnuConfigDefault.Filter_Vid = ( VlanId_Table_S *)StaticMacDefault;

	/* added by chenfj 2007-6-14   增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤 */
	OnuConfigDefault.Filter_Ether_Type = (Ether_Type_Table_S *)StaticMacDefault;
	OnuConfigDefault.Filter_Ip_Type = ( Ip_Type_Table_S *) StaticMacDefault;
#ifdef  V2R1_ONU_AUTO_CONFIG
	OnuConfigDefault.AutoConfig = V2R1_ONU_USING_LOCAL_DATA;
#endif
	OnuConfigDefault.CTC_EncryptCtrl = CTCEncryptCtrlDefault;
	OnuConfigDefault.CTC_EncryptStatus = CTCEncryptStatusDefault;

	OnuConfigDefault.FEC_ability = STD_FEC_ABILITY_UNKNOWN;
	OnuConfigDefault.FEC_mode = STD_FEC_MODE_DISABLED; /*STD_FEC_MODE_UNKNOWN;*/

	return( ROK );
}

#if 0
 /* added by xieshl 20110713, 主备倒换时清除备用主控上ONU数据 */
LONG onuRunningDataSwitchhoverCallback( ULONG slotno  )
{
	short int PonPortIdx, OnuIdx;
	short int onuBaseEntry, onuEntry;
	int portno, ponPortNum;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;

	ponPortNum = GetSlotCardPonPortRange(0, slotno);

	ONU_MGMT_SEM_TAKE;
	for( portno=1; portno<=ponPortNum; portno++ )
	{
		PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
		if( PonPortIdx == RERROR )
			continue;

		/*pon_lose_olt(PonPortIdx);*/
		/*OLT_SetupIFs(PonPortIdx, OLT_ADAPTER_NULL);*/
		
		onuBaseEntry = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
		{
			/*InitOneOnuByDefault( PonPortIdx, OnuIdx );*/
			/*ONU_SetupIFs(PonPortIdx, OnuIdx, ONU_ADAPTER_NULL);*/	/* modified by xieshl 20110815, 问题单13341 */

			onuEntry = onuBaseEntry + OnuIdx;
			resetOnuOperStatusAndUpTime( onuEntry, ONU_OPER_STATUS_DOWN );
			OnuMgmtTable[onuEntry].AlarmStatus = 0;
			/*VOS_MemSet(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 0xff, BYTES_IN_MAC_ADDRESS );
	   		OnuMgmtTable[onuEntry].PonIdx = PonPortIdx;
	    		OnuMgmtTable[onuEntry].LLID = INVALID_LLID;*/
		}
	}
	ONU_MGMT_SEM_GIVE;
	return VOS_OK;
}


LONG onuRunningDataHotCallback( ULONG slotno  )
{
	short int PonPortIdx, OnuIdx;
	short int onuBaseEntry, onuEntry;
	int state, valid;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( !SYS_MODULE_ISMASTERSTANDBY(slotno) )
		return VOS_OK;

	for( PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++ )
	{
		onuBaseEntry = PonPortIdx * MAXONUPERPON;
		
		for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuBaseEntry + OnuIdx;
			ONU_MGMT_SEM_TAKE;
			state = OnuMgmtTable[onuEntry].OperStatus;
			valid = !MAC_ADDR_IS_INVALID( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr);
			ONU_MGMT_SEM_GIVE;
			if( valid && (state == ONU_OPER_STATUS_UP || state == ONU_OPER_STATUS_PENDING || state == ONU_OPER_STATUS_DORMANT) )
				OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
		}
	}
	return VOS_OK;
}
#endif

int testOnuMgmtTable()
{
    int i;
    for (i=0; i<MAXONU; i++)
    {
        sys_console_printf("OnuMgmtTable[%d].OnuIFs = 0x%08x\r\n", i, (ULONG)(OnuMgmtTable[i].OnuIFs));
    }

    return 0;
}

int clearErrOnu(short int onu_rd_idx)
{
    OnuMgmtTable[onu_rd_idx].OnuIFs = (OnuMgmtIFs *)0x12345678;
	return VOS_OK;
}

extern short int parseOnuIndexFromDevIdx( ULONG devIdx, ULONG * pPonIdx, ULONG *pOnuIdx );
void testOnuMgmtTableByOnuDevIdx( ULONG onuDevIdx )
{
	int onuEntry = parseOnuIndexFromDevIdx(onuDevIdx, 0, 0);
 	if( onuEntry < 0 || onuEntry >= MAXONU )
 	{
 		VOS_ASSERT(0);
		return;
	}
	sys_console_printf("\r\nOnuMgmtTable[%3d].OnuIFs = 0x%08x\r\n", onuEntry, (ULONG)OnuMgmtTable[onuEntry].OnuIFs);
 
	return;
}
void InitMaxOnuTableByDefault()
{
    short int ulSlot = 0;
    short int ulPort = 0;
	for( ulSlot=PONCARD_FIRST; ulSlot<=PONCARD_LAST; ulSlot++)
	{
        for( ulPort=1; ulPort<=MAXPONPORT_PER_BOARD;ulPort++)
        {
            glMaxOnuTable[ulSlot-1][ulPort-1] = DAFAULT_MAX_ONU|CONFIG_DEFAULT_MAX_ONU_FLAG;
        }
	}
}

int InitOnuMgmtTableByDefault()
{
    int i, j;
    short int PonPortIdx, OnuIdx;
    short int OnuBaseIdx;
    MacTable_S *MacPointer, *NextPointer, *CurrentPointer;
    ULONG ulSize;

	/*setSeed();*/
#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/

    VOS_ASSERT(NULL == OnuMgmtTable);
    ulSize = sizeof(ONUTable_S) * MAXONU;
    if(ulSize > (0x5000000/2))
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "onu MgmtTable memory create fail!\r\n");
		sys_console_printf("onu MgmtTable memory is (%d k)too large!(only reserved 40M)\r\n", (ulSize/1024));            
    }
	if((sysPhysMemTop() <= 0x10000000))
	{
		if ( NULL == (OnuMgmtTable = (ONUTable_S*)g_malloc(ulSize)))
		{
#ifdef __T_PON_MEM__        
			sys_console_printf("OnuTotalNum:%d, sizeof(OnuMgtUnit)=%d, sizeof(OnuMgtTbl)=%d, g_malloc Fail.\r\n", MAXONU, sizeof(ONUTable_S), ulSize);
			VOS_TaskDelay(VOS_TICK_SECOND);
#endif
			VOS_ASSERT(0);
			return RERROR;
		}
#ifdef __T_PON_MEM__        
		else
		{
			sys_console_printf("OnuTotalNum:%d, sizeof(OnuMgtUnit)=%d, sizeof(OnuMgtTbl)=%d, g_malloc OK.\r\n", MAXONU, sizeof(ONUTable_S), ulSize);
		}
#endif
	}
	else
	{
    if ( NULL == (OnuMgmtTable = GetOnuManageMemStart())/*(ONUTable_S*)g_malloc(ulSize))*/ )
    {
#ifdef __T_PON_MEM__        
        sys_console_printf("OnuTotalNum:%d, sizeof(OnuMgtUnit)=%d, sizeof(OnuMgtTbl)=%d, g_malloc Fail.\r\n", MAXONU, sizeof(ONUTable_S), ulSize);
        VOS_TaskDelay(VOS_TICK_SECOND);
#endif
        VOS_ASSERT(0);
        return RERROR;
    }
#ifdef __T_PON_MEM__        
    else
    {
        sys_console_printf("OnuTotalNum:%d, sizeof(OnuMgtUnit)=%d, sizeof(OnuMgtTbl)=%d, g_malloc OK.\r\n", MAXONU, sizeof(ONUTable_S), ulSize);
    }
#endif
	}
#endif

	VOS_Ticks_Per_Seconds = VOS_TICK_SECOND;

	InitOnuConfigDefault();

	for( PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
    {
        OnuBaseIdx = PonPortIdx * MAXONUPERPON;
    	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
        {
            i = OnuBaseIdx + OnuIdx;

    		VOS_MemSet((void*)(&OnuMgmtTable[i]), 0, sizeof(ONUTable_S));
            ONU_SetupIFs(PonPortIdx, OnuIdx, ONU_ADAPTER_NULL);
/*comment by wangxiaoyu 2011-07-26 for onuid-map fixed*/
#if 0            
            VOS_StrCpy(OnuMgmtTable[i].configFileName, DEFAULT_ONU_CONF);
#endif
            /*add by wangxy 2007-03-01 默认ONU的PON端口类型为eponMauType1000BasePXONU*/
    		OnuMgmtTable[i].PonType = EPONMAUTYPE1000BASEPXONU; 
    		OnuMgmtTable[i].PonRate = PON_RATE_NORMAL_1G; 
    		                               
    		OnuMgmtTable[i].Index.type = PONPORT_TYPE;
    		OnuMgmtTable[i].Index.subcard = 0;
    		OnuMgmtTable[i].Index.slot = GetCardIdxByPonChip( PonPortIdx );
    		OnuMgmtTable[i].Index.port = GetPonPortByPonChip( PonPortIdx );
    		OnuMgmtTable[i].Index.subNo = 0;
    		OnuMgmtTable[i].Index.Onu_FE = 0;

    		OnuMgmtTable[i].PonIdx = PonPortIdx;
    		OnuMgmtTable[i].LLID = INVALID_LLID;
    		
    		OnuMgmtTable[i].DeviceInfo.VendorInfoLen =  OnuConfigDefault.VendorInfoLen;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorInfo,OnuConfigDefault.VendorInfo, OnuMgmtTable[i].DeviceInfo.VendorInfoLen);
    		
#if 0
    		OnuMgmtTable[i].DeviceInfo.VendorLocationLen = OnuConfigDefault.VendorLocationLen ;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorLocation, OnuConfigDefault.VendorLocation, OnuMgmtTable[i].DeviceInfo.VendorLocationLen );
    		OnuMgmtTable[i].DeviceInfo.VendorContactLen = OnuConfigDefault.VendorContactLen;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorContact, OnuConfigDefault.VendorContact, OnuMgmtTable[i].DeviceInfo.VendorContactLen );
#endif
    		{
    		/*
        		int slot, devIdx;
        		short int port;
        		*/
    		
    		OnuMgmtTable[i].DeviceInfo.DeviceNameLen = OnuConfigDefault.DeviceNameLen ;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceName, OnuConfigDefault.DeviceName, OnuMgmtTable[i].DeviceInfo.DeviceNameLen );
    		/* deleted by chenfj 2008-5-28 */
    		#if 0
    		slot = GetCardIdxByPonChip(i/MAXONUPERPON);
    		port = GetPonPortByPonChip(i/MAXONUPERPON);
    		devIdx = slot * 10000 + port * 1000 +( i % MAXONUPERPON ) +1;
    		/*sys_console_printf(" slot%d/ port%d onu%d  \r\n",slot,port,devIdx );*/
    		sprintf(&(OnuMgmtTable[i].DeviceInfo.DeviceName[OnuConfigDefault.DeviceNameLen]), "%05d", devIdx );
    		OnuMgmtTable[i].DeviceInfo.DeviceNameLen += 5;
    		#endif
    		}
#if 0
    		OnuMgmtTable[i].DeviceInfo.DeviceTypeLen = OnuConfigDefault.DeviceTypeLen ;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceType, OnuConfigDefault.DeviceType, OnuMgmtTable[i].DeviceInfo.DeviceTypeLen );
#endif
    		OnuMgmtTable[i].DeviceInfo.DeviceDescLen = OnuConfigDefault.DeviceDescLen;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceDesc, OnuConfigDefault.DeviceDesc, OnuMgmtTable[i].DeviceInfo.DeviceDescLen);
    		OnuMgmtTable[i].DeviceInfo.LocationLen = OnuConfigDefault.LocationLen;
    		VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.Location, OnuConfigDefault.Location, OnuMgmtTable[i].DeviceInfo.LocationLen );

    		VOS_MemCpy(OnuMgmtTable[i].DeviceInfo.MacAddr, OnuConfigDefault.MacAddr, BYTES_IN_MAC_ADDRESS );

    		OnuMgmtTable[i].DeviceInfo.type = V2R1_OTHER ; /*V2R1_UNKNOWN */
            OnuMgmtTable[i].DeviceInfo.PonChipVendor = 0;
            OnuMgmtTable[i].DeviceInfo.PonChipType = 0;
    		
    		OnuMgmtTable[i].NeedDeleteFlag = FALSE;
    		OnuMgmtTable[i].AdminStatus = OnuConfigDefault.AdminStatus;
    		OnuMgmtTable[i].OperStatus = OnuConfigDefault.OperStatus;
    		/*OnuMgmtTable[i].DeviceInfo.SysUptime = VOS_GetTick();
    		OnuMgmtTable[i].DeviceInfo.RelativeUptime = 0;*/
    		OnuMgmtTable[i].ExtOAM = FALSE;
    		OnuMgmtTable[i].SoftwareUpdateCtrl = OnuConfigDefault.SoftwareUpdateCtrl;
    		OnuMgmtTable[i].SoftwareUpdateStatus = OnuConfigDefault.SoftwareUpdateFlag;
    		
    		OnuMgmtTable[i].EncryptEnable = OnuConfigDefault.EncryptEnable;
    		OnuMgmtTable[i].EncryptStatus = OnuConfigDefault.EncryptStatus;
    		OnuMgmtTable[i].EncryptDirection = PonPortTable[PonPortIdx].EncryptDirection; /*OnuConfigDefault.EncryptDirection;*/
    		OnuMgmtTable[i].EncryptKeyTime = PonPortTable[PonPortIdx].EncryptKeyTime ; /*OnuConfigDefault.EncryptKeyTime;*/
    		OnuMgmtTable[i].EncryptCounter = (OnuMgmtTable[i].EncryptKeyTime/V2R1_TIMERTICK);
    		OnuMgmtTable[i].EncryptFirstTime = V2R1_ENABLE;

            OnuMgmtTable[i].llidNum = 0;
    		for( j=0;j<MAXLLIDPERONU;j++)
    		{
    		    InitOnuLlidEntry(PonPortIdx, OnuIdx, j);
    		}

    		/*VOS_MemCpy( OnuMgmtTable[i].AlarmMask, OnuConfigDefault.AlarmMask, sizeof( OnuConfigDefault.AlarmMask ));*/
    		OnuMgmtTable[i].AlarmMask = OnuConfigDefault.AlarmMask;
    		OnuMgmtTable[i].devAlarmMask = OnuConfigDefault.AlarmMask;	/* added by xieshl 20070927 */

    		if( OnuConfigDefault.StaticMac != NULL )
    		{			
    			NextPointer = OnuConfigDefault.StaticMac;
    			CurrentPointer = &(OnuMgmtTable[i].StaticMac);
    			
    			while ( NextPointer != NULL )
    			{
    				MacPointer = (MacTable_S *) VOS_Malloc( sizeof( MacTable_S ), MODULE_ONU );
    				if( MacPointer == NULL )
    				{
    					sys_console_printf("\r\nerror: Malloc memory failed (InitOnuMgmtTableByDefault())\r\n" );
    					break;
    				}
    				VOS_MemCpy( MacPointer->MAC, NextPointer->MAC, BYTES_IN_MAC_ADDRESS );
    				MacPointer->nextMac = 0;
    				CurrentPointer->nextMac = (unsigned int )MacPointer;
    				CurrentPointer = (MacTable_S *)CurrentPointer->nextMac;
    				NextPointer = ( MacTable_S *)NextPointer->nextMac;
    				OnuMgmtTable[i].StaticMacNum ++;
    			}
    		}	

    		/* add by chenfj 2007/6/1
          		 增加设置/清除ONU 数据包原MAC 过滤
        		*/
    		OnuMgmtTable[i].FilterSaMac= OnuConfigDefault.FilterSaMac ;
    		OnuMgmtTable[i].FilterSaMacNum = 0;

    		/* 2007/6/8 增加ONU 数据包IP/port 过滤*/
    		OnuMgmtTable[i].Filter_SIp = OnuConfigDefault.Filter_SIp;
    		OnuMgmtTable[i].Filter_SIp_tcp = OnuConfigDefault.Filter_SIp_tcp;
    		OnuMgmtTable[i].Filter_SIp_udp = OnuConfigDefault.Filter_SIp_udp;
    		OnuMgmtTable[i].Filter_DIp = OnuConfigDefault.Filter_DIp;
    		OnuMgmtTable[i].Filter_DIp_tcp = OnuConfigDefault.Filter_DIp_tcp;
    		OnuMgmtTable[i].Filter_DIp_udp = OnuConfigDefault.Filter_DIp_udp;

    		/* 2007/6/12 增加ONU数据包VID过滤*/
    		OnuMgmtTable[i].Filter_Vid = OnuConfigDefault.Filter_Vid;

    		/* added by chenfj 2007-6-14   增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤 */
    		OnuMgmtTable[i].Filter_Ether_Type =  OnuConfigDefault.Filter_Ether_Type;
    		OnuMgmtTable[i].Filter_Ip_Type = OnuConfigDefault.Filter_Ip_Type;

    		OnuMgmtTable[i].BerFlag = V2R1_TRAP_NOT_SEND;
    		OnuMgmtTable[i].FerFlag = V2R1_TRAP_NOT_SEND;
    		OnuMgmtTable[i].LastBerAlarmTime = 0;
    		OnuMgmtTable[i].LastFerAlarmTime = 0;

    		OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
    		OnuMgmtTable[i].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
    		OnuMgmtTable[i].PowerOn = V2R1_POWER_ON;
    		OnuMgmtTable[i].PowerOffCounter = 0;

    		OnuMgmtTable[i].vty = NULL;

    		OnuMgmtTable[i].swFileLen = 0;
    		OnuMgmtTable[i].transFileLen = 0;

    		/* onu p2p 前转规则*/
    		OnuMgmtTable[i].address_not_found_flag = V2R1_DISABLE;
    		OnuMgmtTable[i].broadcast_flag = V2R1_DISABLE;

    		/* 2007-02-07 增加ONU自动配置*/
#ifdef   V2R1_ONU_AUTO_CONFIG
    		OnuMgmtTable[i].AutoConfig = OnuConfigDefault.AutoConfig;
    		OnuMgmtTable[i].AutoConfigFlag = V2R1_ONU_INITIAL_STATUS;
#endif
    		OnuMgmtTable[i].GetEUQflag = FALSE;
    		OnuMgmtTable[i].GetEUQCounter = 0;

    		OnuMgmtTable[i].CTC_EncryptCtrl = OnuConfigDefault.CTC_EncryptCtrl;
    		OnuMgmtTable[i].CTC_EncryptStatus = OnuConfigDefault.CTC_EncryptStatus;

    		OnuMgmtTable[i].FEC_ability = OnuConfigDefault.FEC_ability;
    		OnuMgmtTable[i].FEC_Mode = OnuConfigDefault.FEC_mode;
    		OnuMgmtTable[i].LoopbackEnable = V2R1_DISABLE;

            /*add by wangxy 2007-03-30  ONU stp enable status init*/
    		OnuMgmtTable[i].stpEnable = 2;	/*initialize stpEnable disable*/	

    		OnuMgmtTable[i].TrafficServiceEnable = V2R1_ENABLE;

    		OnuMgmtTable[i].BurningFlash = V2R1_DISABLE;
    		OnuMgmtTable[i].BurningWait = 0;
    		
    		/*get follow data from nvram 
        		OnuMgmtTable[i].RegisterFlag = 
        		OnuMgmtTable[i].UsedFlag = 
        		VOS_MemCpy( &OnuMgmtTable[i].DeviceInfo.MacAddr[0], source, BYTES_IN_MAC_ADDRESS );
        		*/
    		OnuMgmtTable[i].vlan_priority.row_status = V2R1_DOWN;

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
    		PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;
    		/*PonPortTable[i/MAXONUPERPON].Downlink_vlan_manipulation[i % MAXONUPERPON].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;*/
#endif
    		OnuMgmtTable[i].fastleaveAbility = 0;
    		OnuMgmtTable[i].fastleaveControl = V2R1_DISABLE;
			OnuMgmtTable[i].needToRestoreConfFile = V2R1_ENABLE;
        }   
    }   

	return( ROK );	
}

int InitOneOnuByDefault( short int PonPortIdx, short int OnuIdx)
{
	int i,j;
	MacTable_S *MacPointer, *NextPointer, *CurrentPointer;

	CHECK_ONU_RANGE;
	
	i = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	if ( ONU_ADAPTER_RPC < ONU_GetIFType(PonPortIdx, OnuIdx) )
	{
	    pon_lose_onu(PonPortIdx, OnuIdx);
	}
	VOS_MemSet((&OnuMgmtTable[i].Index), 0, sizeof(ONUTable_S) - sizeof(OnuMgmtIFs*));

	OnuMgmtTable[i].PonType = EPONMAUTYPE1000BASEPXONU; /*add by wangxy 2007-03-01
	                               默认ONU的PON端口类型为eponMauType1000BasePXONU*/
	OnuMgmtTable[i].PonRate = PON_RATE_NORMAL_1G; 
	OnuMgmtTable[i].Index.type = PONPORT_TYPE;
	OnuMgmtTable[i].Index.subcard = 0;
	OnuMgmtTable[i].Index.slot = GetCardIdxByPonChip( PonPortIdx );
	OnuMgmtTable[i].Index.port = GetPonPortByPonChip( PonPortIdx );
	OnuMgmtTable[i].Index.subNo = 0;
	OnuMgmtTable[i].Index.Onu_FE = 0;

	OnuMgmtTable[i].PonIdx = PonPortIdx;
	OnuMgmtTable[i].LLID = INVALID_LLID;
	
	OnuMgmtTable[i].DeviceInfo.VendorInfoLen = OnuConfigDefault.VendorInfoLen;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorInfo,OnuConfigDefault.VendorInfo, OnuMgmtTable[i].DeviceInfo.VendorInfoLen);
#if 0
	OnuMgmtTable[i].DeviceInfo.VendorLocationLen = OnuConfigDefault.VendorLocationLen ;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorLocation, OnuConfigDefault.VendorLocation, OnuMgmtTable[i].DeviceInfo.VendorLocationLen );
	OnuMgmtTable[i].DeviceInfo.VendorContactLen = OnuConfigDefault.VendorContactLen;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.VendorContact, OnuConfigDefault.VendorContact, OnuMgmtTable[i].DeviceInfo.VendorContactLen );
#endif
	{
	/*
	int slot, devIdx;
	short int port;		
	*/
	OnuMgmtTable[i].DeviceInfo.DeviceNameLen = OnuConfigDefault.DeviceNameLen ;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceName, OnuConfigDefault.DeviceName, OnuMgmtTable[i].DeviceInfo.DeviceNameLen );
	/* deleted by chenfj 2008-5-28
	发件人: KD解世立 
	发送时间: 2008年5月27日 18:31
	收件人: KD刘冬; KD王小宇; KD张新辉; KD陈福军
	抄送: KD盖鹏飞; KD王楠
	主题: 关于ONU device-name默认值，后续版本中统一改成"ONU"，不再用ONU的类型作为其默认名称，
	OLT侧对于Unknown类型的ONU名称也改成"ONU"
	主要目的是为了方便网管界面上对ONU名称做进一步处理，同时OEM ONU也不再需要对名称做特殊处理。
	*/
	#if 0
	slot = GetCardIdxByPonChip(i/MAXONUPERPON);
	port = GetPonPortByPonChip(i/MAXONUPERPON);
	devIdx = slot * 10000 + port * 1000 +( i % MAXONUPERPON ) +1;
	/*sys_console_printf(" slot%d/ port%d onu%d  \r\n",slot,port,devIdx );*/
	sprintf(&(OnuMgmtTable[i].DeviceInfo.DeviceName[OnuConfigDefault.DeviceNameLen]), "%05d", devIdx );
	OnuMgmtTable[i].DeviceInfo.DeviceNameLen += 5;
	#endif
	}

#if 0
	OnuMgmtTable[i].DeviceInfo.DeviceTypeLen = OnuConfigDefault.DeviceTypeLen ;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceType, OnuConfigDefault.DeviceType, OnuMgmtTable[i].DeviceInfo.DeviceTypeLen );
#endif
	OnuMgmtTable[i].DeviceInfo.DeviceDescLen = OnuConfigDefault.DeviceDescLen;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.DeviceDesc, OnuConfigDefault.DeviceDesc, OnuMgmtTable[i].DeviceInfo.DeviceDescLen);
	OnuMgmtTable[i].DeviceInfo.LocationLen = OnuConfigDefault.LocationLen;
	VOS_MemCpy( OnuMgmtTable[i].DeviceInfo.Location, OnuConfigDefault.Location, OnuMgmtTable[i].DeviceInfo.LocationLen );

	VOS_MemCpy(OnuMgmtTable[i].DeviceInfo.MacAddr, OnuConfigDefault.MacAddr, BYTES_IN_MAC_ADDRESS );

	OnuMgmtTable[i].DeviceInfo.type = V2R1_OTHER ; /*V2R1_UNKNOWN */
	OnuMgmtTable[i].DeviceInfo.PonChipVendor = 0;
	OnuMgmtTable[i].DeviceInfo.PonChipType = 0;

	OnuMgmtTable[i].NeedDeleteFlag = FALSE;
	OnuMgmtTable[i].AdminStatus = OnuConfigDefault.AdminStatus;
    /*recover by luh 2012-6-20 问题单15268，初始化数据需要更新onu状态，对于时间的统计则在外面进行处理*/
	OnuMgmtTable[i].OperStatus = OnuConfigDefault.OperStatus;	/* modified by xieshl 20101222, 问题单11743 */
	/*if( OnuMgmtTable[i].OperStatus != OnuConfigDefault.OperStatus )
	{
		ULONG tick = VOS_GetTick();
		OnuMgmtTable[i].DeviceInfo.SysUptime = tick;
		OnuMgmtTable[i].DeviceInfo.RelativeUptime = tick - OLTMgmt.DeviceInfo.SysUptime;
	}*/
	OnuMgmtTable[i].ExtOAM = FALSE;
	OnuMgmtTable[i].SoftwareUpdateCtrl = OnuConfigDefault.SoftwareUpdateCtrl;
	OnuMgmtTable[i].SoftwareUpdateStatus = OnuConfigDefault.SoftwareUpdateFlag;
	
	OnuMgmtTable[i].EncryptEnable = OnuConfigDefault.EncryptEnable;
	OnuMgmtTable[i].EncryptStatus = OnuConfigDefault.EncryptStatus;
	OnuMgmtTable[i].EncryptDirection = PonPortTable[PonPortIdx].EncryptDirection; /*OnuConfigDefault.EncryptDirection;*/
	OnuMgmtTable[i].EncryptKeyTime = PonPortTable[PonPortIdx].EncryptKeyTime ; /*OnuConfigDefault.EncryptKeyTime;*/
	OnuMgmtTable[i].EncryptCounter = (OnuMgmtTable[i].EncryptKeyTime/V2R1_TIMERTICK);
	OnuMgmtTable[i].EncryptFirstTime = V2R1_ENABLE;

    OnuMgmtTable[i].llidNum = 0;
	for( j=0;j<MAXLLIDPERONU;j++)
	{
	    InitOnuLlidEntry(PonPortIdx, OnuIdx, j);
	}

	/*VOS_MemCpy( OnuMgmtTable[i].AlarmMask, OnuConfigDefault.AlarmMask, sizeof( OnuConfigDefault.AlarmMask ));*/
	OnuMgmtTable[i].AlarmMask = OnuConfigDefault.AlarmMask;
	OnuMgmtTable[i].devAlarmMask = OnuConfigDefault.AlarmMask;	/* added by xieshl 20070927 */
	
	if( OnuConfigDefault.StaticMac != NULL )
	{			
		NextPointer = OnuConfigDefault.StaticMac;
		CurrentPointer = &(OnuMgmtTable[i].StaticMac);
		
		while ( NextPointer != NULL )
		{
			MacPointer = (MacTable_S *) VOS_Malloc( sizeof( MacTable_S ), MODULE_ONU );
			if( MacPointer == NULL )
			{
				sys_console_printf("\r\nerror: Malloc memory failed (InitOnuMgmtTableByDefault())\r\n" );
				break;
			}
			VOS_MemCpy( MacPointer->MAC, NextPointer->MAC, BYTES_IN_MAC_ADDRESS );
			MacPointer->nextMac = 0;
			CurrentPointer->nextMac = (unsigned int )MacPointer;
			CurrentPointer = (MacTable_S *)CurrentPointer->nextMac;
			NextPointer = ( MacTable_S *)NextPointer->nextMac;
			OnuMgmtTable[i].StaticMacNum ++;
		}
	}	

	/* add by chenfj 2007/6/1
  		 增加设置/清除ONU 数据包原MAC 过滤
		*/
	OnuMgmtTable[i].FilterSaMac= OnuConfigDefault.FilterSaMac ;
	OnuMgmtTable[i].FilterSaMacNum = 0;

	/* 2007/6/8 增加ONU 数据包IP/port 过滤*/
	OnuMgmtTable[i].Filter_SIp = OnuConfigDefault.Filter_SIp;
	OnuMgmtTable[i].Filter_SIp_tcp = OnuConfigDefault.Filter_SIp_tcp;
	OnuMgmtTable[i].Filter_SIp_udp = OnuConfigDefault.Filter_SIp_udp;
	OnuMgmtTable[i].Filter_DIp = OnuConfigDefault.Filter_DIp;
	OnuMgmtTable[i].Filter_DIp_tcp = OnuConfigDefault.Filter_DIp_tcp;
	OnuMgmtTable[i].Filter_DIp_udp = OnuConfigDefault.Filter_DIp_udp;

	/* 2007/6/12 增加ONU数据包VID过滤*/
	OnuMgmtTable[i].Filter_Vid = OnuConfigDefault.Filter_Vid;

	/* added by chenfj 2007-6-14   增加ONU 数据流ETHER TYPE / IP PROTOCOL 过滤 */
	OnuMgmtTable[i].Filter_Ether_Type =  OnuConfigDefault.Filter_Ether_Type;
	OnuMgmtTable[i].Filter_Ip_Type = OnuConfigDefault.Filter_Ip_Type;
	

	OnuMgmtTable[i].BerFlag = V2R1_TRAP_NOT_SEND;
	OnuMgmtTable[i].FerFlag = V2R1_TRAP_NOT_SEND;
	OnuMgmtTable[i].LastBerAlarmTime = 0;
	OnuMgmtTable[i].LastFerAlarmTime = 0;
	
	/*get follow data from nvram  */
	OnuMgmtTable[i].RegisterFlag = ONU_FIRST_REGISTER_FLAG;
	OnuMgmtTable[i].RegisterTrapFlag = NO_ONU_REGISTER_TRAP;
	OnuMgmtTable[i].PowerOn = V2R1_POWER_ON;
	OnuMgmtTable[i].PowerOffCounter = 0;

	OnuMgmtTable[i].vty = NULL;

	OnuMgmtTable[i].swFileLen = 0;
	OnuMgmtTable[i].transFileLen = 0;

	/* onu p2p 前转规则*/
	OnuMgmtTable[i].address_not_found_flag = V2R1_DISABLE;
	OnuMgmtTable[i].broadcast_flag = V2R1_DISABLE;
	
	/* 2007-02-07 增加ONU自动配置*/
#ifdef   V2R1_ONU_AUTO_CONFIG
	OnuMgmtTable[i].AutoConfig = OnuConfigDefault.AutoConfig;
	OnuMgmtTable[i].AutoConfigFlag = V2R1_ONU_INITIAL_STATUS;
#endif
	OnuMgmtTable[i].GetEUQflag = FALSE;
	OnuMgmtTable[i].GetEUQCounter = 0;

	OnuMgmtTable[i].CTC_EncryptCtrl = OnuConfigDefault.CTC_EncryptCtrl;
	OnuMgmtTable[i].CTC_EncryptStatus = OnuConfigDefault.CTC_EncryptStatus;

	OnuMgmtTable[i].FEC_ability = OnuConfigDefault.FEC_ability;
	OnuMgmtTable[i].FEC_Mode = OnuConfigDefault.FEC_mode;
	OnuMgmtTable[i].LoopbackEnable = V2R1_DISABLE;

/*add by wangxy 2007-03-30  ONU stp enable status init*/
	OnuMgmtTable[i].stpEnable = 2;

	OnuMgmtTable[i].TrafficServiceEnable = V2R1_ENABLE;

	OnuMgmtTable[i].BurningFlash = V2R1_DISABLE;
	OnuMgmtTable[i].BurningWait = 0;
	
	/*
	OnuMgmtTable[i].UsedFlag = 
	VOS_MemCpy( &OnuMgmtTable[i].DeviceInfo.MacAddr[0], source, BYTES_IN_MAC_ADDRESS );
	*/
	OnuMgmtTable[i].vlan_priority.row_status = V2R1_DOWN;

#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
	/*PonPortTable[PonPortIdx].Uplink_vlan_manipulation[OnuIdx].vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION;*//*moved by luh 2012-12-7*/
	/*PonPortTable[i/MAXONUPERPON].Downlink_vlan_manipulation[i % MAXONUPERPON].vlan_manipulation = PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION;*/
#endif
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	/*	PonPortTable[i/MAXONUPERPON].recvOpticalPower[i % MAXONUPERPON] = 0;
	PonPortTable[i/MAXONUPERPON].OpticalPowerAlarm_onu[i % MAXONUPERPON] = 0;
	PonPortTable[i/MAXONUPERPON].OpticalPowerAlarm_gen_onu[i % MAXONUPERPON] = 0;
	PonPortTable[i/MAXONUPERPON].OpticalPowerAlarm_clr_onu[i % MAXONUPERPON] = 0;

	OnuMgmtTable[i].recvOpticalPower = 0;
	OnuMgmtTable[i].transOpticalPower = 0;
	OnuMgmtTable[i].ponTemperature = 0;
	OnuMgmtTable[i].ponVoltageApplied = 0;
	OnuMgmtTable[i].ponBiasCurrent = 0;
	OnuMgmtTable[i].uplink_delta = 0;
	OnuMgmtTable[i].downlink_delta = 0;
	
	OnuMgmtTable[i].OpticalPowerAlarm = 0;*/

	OnuMgmtTable[i].fastleaveAbility = 0;
	OnuMgmtTable[i].fastleaveControl = V2R1_DISABLE;
#endif
	OnuMgmtTable[i].needToRestoreConfFile = V2R1_ENABLE;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}


/*****************************************************
 *
 *    Function: SearchFreeOnuEntryByPonPort( short int PonPort_id)
 *
 *    Param:   short int PonPort_id -- the specific pon port
 *          
 *    Desc:  when onu is not provisioned and is registering now, find a free entry 
 *
 *    Return:  free entry, or RERROR 
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
/* removed by xieshl 20110511, 为了提高检索效率，参见宏定义MAC_ADDR_IS_EQUAL */
/*int CompTwoMacAddress( unsigned char *MacAddress1, unsigned char *MacAddress2 )
{
	if(( MacAddress1[0] ==MacAddress2[0] ) &&(MacAddress1[1] ==MacAddress2[1]) && (MacAddress1[2] ==MacAddress2[2])
		&& (MacAddress1[3] ==MacAddress2[3]) && (MacAddress1[4] ==MacAddress2[4]) && (MacAddress1[5] ==MacAddress2[5]) )
		return( ROK );

	return( RERROR );
}*/

/* modified by xieshl 20110512, 查找不到空闲ONU时，则返回一个离线时间最长的ONU */
short int SearchFreeOnuIdxByPonPort(short int PonPortIdx, short int *rep_flag)
{
	int i;
	int onuEntryBase;
	ULONG max_offline_time = -1;
	LONG max_offline_time_onu_idx = RERROR;
	ONUTable_S  *pOnuDevInfo;
	
	CHECK_PON_RANGE;
		
	onuEntryBase = PonPortIdx *MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
	for (i=0; i<MAXONUPERPON; i++)
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( MAC_ADDR_IS_INVALID(pOnuDevInfo->DeviceInfo.MacAddr) )
		/*if( ThisIsValidOnu(PonPortIdx , i ) != ROK )*/
		{
			/* added by chenfj 2007-02-07
                     增加ONU自动配置
			*/
#ifdef   V2R1_ONU_AUTO_CONFIG
			pOnuDevInfo->AutoConfig = V2R1_ONU_USING_AUTO_CONFIG;
			VOS_MemCpy( &(pOnuDevInfo->AutoConfigFile[0]), OnuDefaultConfigName,15);
			pOnuDevInfo->AutoConfigFlag = V2R1_ONU_INITIAL_STATUS;
#endif
			*rep_flag = 0;
			ONU_MGMT_SEM_GIVE;
			return ( i );
		}

		if(PONid_ONUMAC_BINDING == V2R1_DISABLE)
		{
			if( (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_UP)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_PENDING)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_DORMANT)
				)
			{
				if( max_offline_time > pOnuDevInfo->DeviceInfo.SysUptime )
				{
					max_offline_time = pOnuDevInfo->DeviceInfo.SysUptime;
					max_offline_time_onu_idx = i;
				}
			}
		}
	}
	ONU_MGMT_SEM_GIVE;

	*rep_flag = 1;
	return max_offline_time_onu_idx;
}

/* added by xieshl 20110610, 一次完成ONU MAC地址检索，并返回OnuIdx */
/* 输出参数: reg_flag   0-re-register, 1-new register, 2-replaced */
/* 返回值: RERROR-ONU注册满或输入参数错误, 0~63-ONU在PonPortIdx下的索引 */
short int SearchFreeOnuIdxForRegister( short int PonPortIdx, unsigned char *MacAddress, short int *reg_flag )
{
	int onuEntryBase;
	ULONG max_offline_time = -1;
	LONG offlineOnuIdx = RERROR;
	ONUTable_S  *pOnuDevInfo;
	short int i;
	short int freeOnuIdx = RERROR;
    short int max_onu = GetMaxOnuByPonPort(PonPortIdx)&0xff;
	if( (MacAddress == NULL) || (reg_flag == NULL) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	CHECK_PON_RANGE;
		
	onuEntryBase = PonPortIdx *MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
	for( i=0; i<MAXONUPERPON; i++ )
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( MAC_ADDR_IS_EQUAL(pOnuDevInfo->DeviceInfo.MacAddr, MacAddress) )
		{
		    /*当前MAXLLIDPERONU == 1。暂不管多llid注册情况，因为涉及PON 保护中llid的恢复。此处先注释掉，带需要支持多llid注册时，重新考虑*/
#if 0		
            if ( MAXLLIDPERONU == pOnuDevInfo->llidNum )
            {
                /* 此ONU的LLID已满，则认为无空余Entry */
                i = RERROR;
            }
#endif            
			ONU_MGMT_SEM_GIVE;
			*reg_flag = 0;
			return i;
		}
	}
	ONU_MGMT_SEM_GIVE;


	ONU_MGMT_SEM_TAKE;
	for( i=0; i<max_onu; i++ )
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( MAC_ADDR_IS_INVALID(pOnuDevInfo->DeviceInfo.MacAddr) )
		{
			if( freeOnuIdx == RERROR )
				freeOnuIdx = i;
		}
        
		if( (PONid_ONUMAC_BINDING == V2R1_DISABLE) && ( freeOnuIdx == RERROR ) )
		{
			if( (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_UP)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_PENDING)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_DORMANT)
				)
			{
				if( max_offline_time > pOnuDevInfo->DeviceInfo.SysUptime )
				{
					max_offline_time = pOnuDevInfo->DeviceInfo.SysUptime;
					offlineOnuIdx = i;
				}
			}
		}
	}
	ONU_MGMT_SEM_GIVE;

	if( freeOnuIdx != RERROR )
	{
		*reg_flag = 1;
		return freeOnuIdx;
	}

	*reg_flag = 2;
	return offlineOnuIdx;
}

/*begin: 新增获取ONU有效ID的接口，供ocs底层调用, add by liuyh, 2017-1-13*/
short int SearchGponOnuIdxReReg(short int PonPortIdx, unsigned char *sn)
{
    int onuEntryBase;
	ONUTable_S  *pOnuDevInfo;
	short int i;
    
	if(sn == NULL)
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	CHECK_PON_RANGE;
		
	onuEntryBase = PonPortIdx *MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
	for( i=0; i<MAXONUPERPON; i++ )
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( VOS_StrCmp(pOnuDevInfo->DeviceInfo.DeviceSerial_No, sn) == 0)
		{   
			ONU_MGMT_SEM_GIVE;
			return i;
		}
	}
	ONU_MGMT_SEM_GIVE;

    return ERROR;
}

short int SearchFreeGponOnuIdxNewReg(short int PonPortIdx, unsigned char *sn, short int startOnuId)
{
    int onuEntryBase;
	ONUTable_S  *pOnuDevInfo;
	short int i;
	short int freeOnuIdx = RERROR;
	/*begin: 删除max_onu对OCS中LLID分配的限制，mod by liuyh, 2017-4-26*/
    /*short int max_onu = GetMaxOnuByPonPort(PonPortIdx)&0xff;*/
    /*end: mod by liuyh, 2017-4-26*/

    if(sn == NULL)
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	CHECK_PON_RANGE;

    onuEntryBase = PonPortIdx *MAXONUPERPON;

    ONU_MGMT_SEM_TAKE;
	for( i=startOnuId; i<MAXONUPERPON; i++ )    /* 按端口最大onu数进行LLID分配 */
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( VOS_StrLen(pOnuDevInfo->DeviceInfo.DeviceSerial_No) == 0)
		{
			if( freeOnuIdx == RERROR )
				freeOnuIdx = i;
		}
	}
	ONU_MGMT_SEM_GIVE;

    return freeOnuIdx;
}

unsigned long GetGponOnuIdxOfflineTime(short int PonPortIdx, short int OnuIdx)
{
    int onuEntryBase;
	ULONG offline_time = -1;
	ONUTable_S  *pOnuDevInfo;
    /*begin: 删除max_onu对OCS中LLID分配的限制，mod by liuyh, 2017-4-26*/
    /*short int max_onu = GetMaxOnuByPonPort(PonPortIdx)&0xff;*/
    /*end: mod by liuyh, 2017-4-26*/

    CHECK_PON_RANGE;
    onuEntryBase = PonPortIdx *MAXONUPERPON;

    /*begin: 删除max_onu对OCS中LLID分配的限制，mod by liuyh, 2017-4-26*/
    /*if (OnuIdx >= max_onu)
    {
        return RERROR;
    }*/
    /*end: mod by liuyh, 2017-4-26*/

    ONU_MGMT_SEM_TAKE;
    pOnuDevInfo = &OnuMgmtTable[onuEntryBase + OnuIdx];   

    /* 去除绑定mac配置对OCS中LLID分配的限制, mod by liuyh, 2017-4-26 */
	if( (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_UP)
		&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_PENDING)
		&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_DORMANT)
		)
	{
        offline_time = pOnuDevInfo->DeviceInfo.SysUptime;            
	}
    /*end: mod by liuyh, 2017-4-26*/
    ONU_MGMT_SEM_GIVE;

    return offline_time;
}

#if 0
short int SearchFreeGponOnuIdxForRegister( short int PonPortIdx, unsigned char *sn, short int *reg_flag )
{
	int onuEntryBase;
	ULONG max_offline_time = -1;
	LONG offlineOnuIdx = RERROR;
	ONUTable_S  *pOnuDevInfo;
	short int i;
	short int freeOnuIdx = RERROR;
    short int max_onu = GetMaxOnuByPonPort(PonPortIdx)&0xff;
	if( (sn == NULL) || (reg_flag == NULL) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	CHECK_PON_RANGE;
		
	onuEntryBase = PonPortIdx *MAXONUPERPON;

	ONU_MGMT_SEM_TAKE;
	for( i=0; i<MAXONUPERPON; i++ )
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( VOS_StrCmp(pOnuDevInfo->DeviceInfo.DeviceSerial_No, sn) == 0)
		{
		    /*当前MAXLLIDPERONU == 1。暂不管多llid注册情况，因为涉及PON 保护中llid的恢复。此处先注释掉，带需要支持多llid注册时，重新考虑*/
#if 0		
            if ( MAXLLIDPERONU == pOnuDevInfo->llidNum )
            {
                /* 此ONU的LLID已满，则认为无空余Entry */
                i = RERROR;
            }
#endif            
			ONU_MGMT_SEM_GIVE;
			*reg_flag = 0;
			return i;
		}
	}
	ONU_MGMT_SEM_GIVE;


	ONU_MGMT_SEM_TAKE;
	for( i=0; i<max_onu; i++ )
	{
		pOnuDevInfo = &OnuMgmtTable[onuEntryBase + i];
		if( VOS_StrLen(pOnuDevInfo->DeviceInfo.DeviceSerial_No) == 0)
		{
			if( freeOnuIdx == RERROR )
				freeOnuIdx = i;
		}
        
		if( (PONid_ONUMAC_BINDING == V2R1_DISABLE) && ( freeOnuIdx == RERROR ) )
		{
			if( (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_UP)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_PENDING)
				&& (pOnuDevInfo->OperStatus != ONU_OPER_STATUS_DORMANT)
				)
			{
				if( max_offline_time > pOnuDevInfo->DeviceInfo.SysUptime )
				{
					max_offline_time = pOnuDevInfo->DeviceInfo.SysUptime;
					offlineOnuIdx = i;
				}
			}
		}
	}
	ONU_MGMT_SEM_GIVE;

	if( freeOnuIdx != RERROR )
	{
		*reg_flag = 1;
		return freeOnuIdx;
	}

	*reg_flag = 2;
	return offlineOnuIdx;
}
#else
short int SearchFreeGponOnuIdxForRegister( short int PonPortIdx, short int llid, unsigned char *sn, short int *reg_flag )
{
	int onuEntryBase;
	ULONG max_offline_time = -1;
	LONG offlineOnuIdx = RERROR;
	ONUTable_S  *pOnuDevInfo;
	short int i;
	short int freeOnuIdx = RERROR;
    short int max_onu = GetMaxOnuByPonPort(PonPortIdx)&0xff;
    short int onuIdx = llid-1;
    
	if( (sn == NULL) || (reg_flag == NULL) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	CHECK_PON_RANGE;

    if (MAXONUPERPON <= onuIdx)
    {
        /* 注册满 */
        return RERROR;
    }
		
	onuEntryBase = PonPortIdx *MAXONUPERPON;
    
    ONU_MGMT_SEM_TAKE;    
    pOnuDevInfo = &OnuMgmtTable[onuEntryBase + onuIdx];   

    /* 1、判断是否是已创建ONU的注册 */
	if( VOS_StrCmp(pOnuDevInfo->DeviceInfo.DeviceSerial_No, sn) == 0)
	{
        ONU_MGMT_SEM_GIVE;
		*reg_flag = 0;
		return onuIdx;
	}

    if (onuIdx >= max_onu)
    {
        ONU_MGMT_SEM_GIVE;
		return RERROR;
    }
    
    /* 2、判断是否是新创建ONU的注册 */
    if (VOS_StrLen(pOnuDevInfo->DeviceInfo.DeviceSerial_No) == 0)
    {
        ONU_MGMT_SEM_GIVE;
		*reg_flag = 1;
		return onuIdx;
    }
    ONU_MGMT_SEM_GIVE;
    
    /* 3、替换离线时间最长的ONU */
    *reg_flag = 2;

    /* 上层增加绑定mac配置对ONU注册的限制, 保持和原有逻辑一致，mod by liuyh, 2017-4-26 */
    if (PONid_ONUMAC_BINDING == V2R1_DISABLE)
    {        
    	return onuIdx;    
    }
    /*end: mod by liuyh, 2017-4-26*/

    return ERROR;
}
#endif 
/*end: add by liuyh, 2017-1-13*/

int SearchValidOnuEntry( short int PonPortIdx )
{
	int i;
	
	CHECK_PON_RANGE

	for( i=0; i< MAXONUPERPON; i++)
		{
		if( ThisIsValidOnu(PonPortIdx , i ) == ROK )
			{
			return ( ROK);
			}
		}
	return( RERROR );
}

#ifdef ONU_INFO
#endif

/*****************************************************
 *
 *    Function: GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                short int  OnuIdx -- the specific Onu
 *    Desc:   the onu should be registered and activated;
 *
 *    Return:  the llid used by this onu  
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx )
{
	short int llid = INVALID_LLID;
	short int onuEntry;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if( /*GetOnuOperStatus_1(PonPortIdx, OnuIdx )*/OnuMgmtTable[onuEntry].OperStatus != ONU_OPER_STATUS_DOWN )	
	{
		llid = OnuMgmtTable[onuEntry].LLID;
#if 0
		ret = PAS_get_onu_mode( PonPortIdx, llid );
		ONU_MGMT_SEM_GIVE;
		if(( ret != PON_ONU_MODE_ON) && ( ret != PON_ONU_MODE_PENDING))
			return(INVALID_LLID);
		else return( llid);
#endif
	}
	ONU_MGMT_SEM_GIVE;
	return llid;
}

short int GetLlidActivatedByOnuIdx( short int PonPortIdx, short int OnuIdx )
{
    short int llid;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    ONU_MGMT_SEM_TAKE;
    llid = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LLID;
    ONU_MGMT_SEM_GIVE;
    if (llid <= 0)
    {
        return(INVALID_LLID);
    }

#if 0
    short int ret;
    ret = PAS_get_onu_mode( PonPortIdx, llid );
    if(( ret != PON_ONU_MODE_ON) && ( ret != PON_ONU_MODE_PENDING))
        return(INVALID_LLID);
#endif

    return(llid);
}

/* B--added by liwei056@2012-11-18 for Tk's Multi-llid ONU's Supported */
short int GetLlidByLlidIdx( short int PonPortIdx, short int OnuIdx, short int LLIDIdx )
{
    short int llid;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);
    LLID_ID_ASSERT(LLIDIdx);

    ONU_MGMT_SEM_TAKE;
    llid = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[LLIDIdx].Llid;
    ONU_MGMT_SEM_GIVE;
    if (llid <= 0)
    {
        return(INVALID_LLID);
    }

    return(llid);
}

short int GetOnuLLIDIdx( short int PonPortIdx, short int OnuIdx, short int llid )
{
    int i;
    ONUTable_S *pstOnu;
    OnuLLIDTable_S *pstLLIDs;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    pstOnu = &OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ];
    pstLLIDs = pstOnu->LlidTable;
    
    ONU_MGMT_SEM_TAKE;
    for ( i=0; i<MAXLLIDPERONU; i++ )
    {
        if ( llid == (short int)pstLLIDs[i].Llid )
        {
            break;
        }
    }
    ONU_MGMT_SEM_GIVE;

    if (i < MAXLLIDPERONU)
    {
        return i;
    }

    return(RERROR);
}

short int GetOnuLLIDNum( short int PonPortIdx, short int OnuIdx )
{
    short int llid_num;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    ONU_MGMT_SEM_TAKE;
    llid_num = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].llidNum;
    ONU_MGMT_SEM_GIVE;

    return(llid_num);
}

short int GetOnuLLIDNumMax( short int PonPortIdx, short int OnuIdx )
{
    short int llid_max;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    ONU_MGMT_SEM_TAKE;
    llid_max = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].llidMax;
    ONU_MGMT_SEM_GIVE;

    return(llid_max);
}
/* E--added by liwei056@2012-11-18 for Tk's Multi-llid ONU's Supported */

/* B--added by liwei056@2013-3-5 for BCM's CMC ONU's Supported */
short int GetOnuCmNumMax( short int PonPortIdx, short int OnuIdx )
{
    short int cm_max;
    
    OLT_LOCAL_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    ONU_MGMT_SEM_TAKE;
    cm_max = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].cmMax;
    ONU_MGMT_SEM_GIVE;

    return(cm_max);
}
/* E--added by liwei056@2013-3-5 for BCM's CMC ONU's Supported */

short int GetOnuIdxByLlid( short int PonPortIdx, short int llid )
{
    int i, iOltBase;
    int j, iLLIDNum;
    ONUTable_S *pOnuDatas;
    OnuLLIDTable_S *pLlidDatas;
    short int OnuIdx = RERROR;

    CHECK_PON_RANGE;

    if ( llid != INVALID_LLID )
    {
        iOltBase = PonPortIdx * MAXONUPERPON;

        ONU_MGMT_SEM_TAKE;
        for (i=0; i<MAXONUPERPON; i++)
        {
            pOnuDatas = &OnuMgmtTable[ iOltBase + i ];
            if ( pOnuDatas->LLID == llid )
            {
                OnuIdx = (short int)i;
                break;
            }
            /* B--added by liwei056@2012-11-18 for Tk's Multi-llid ONU's Supported */
            else if ( 1 < (iLLIDNum = pOnuDatas->llidNum) )
            {
                pLlidDatas = pOnuDatas->LlidTable;
                for (j=0; j<MAXLLIDPERONU; j++)
                {
                    if ( llid == (short int)pLlidDatas[j].Llid )
                    {
                        OnuIdx = (short int)i;
                        
                        i = MAXONUPERPON;
                        break;
                    }
                }
            }
            /* E--added by liwei056@2012-11-18 for Tk's Multi-llid ONU's Supported */
        }
        ONU_MGMT_SEM_GIVE;
    }   

    return OnuIdx;
}

short int  GetOnuEntryByMac( unsigned char *MacAddress )
{
	short int i;
	short int ret = RERROR;

	if( MacAddress == NULL ) return ret;

	ONU_MGMT_SEM_TAKE;
	for( i=0; i<MAXONU; i++ )
	{
		if( MAC_ADDR_IS_EQUAL( OnuMgmtTable[i].DeviceInfo.MacAddr, MacAddress ) )
		{
			/* modified by xieshl 20110622, 当ONU在多个PON下注册时，优选取在线的ONU，问题单13066 */
			if( (ONU_OPER_STATUS_UP == OnuMgmtTable[i].OperStatus) ||
				(ONU_OPER_STATUS_PENDING == OnuMgmtTable[i].OperStatus) ||
				(ONU_OPER_STATUS_DORMANT == OnuMgmtTable[i].OperStatus) )
			{
				ret = i;
				break;
			}
			if( ret == RERROR )
				ret = i;
		}
	}
	ONU_MGMT_SEM_GIVE;
	
	return ret;
}

short int  GetOnuIdxByMacPerPon( short int PonPortIdx, unsigned char *MacAddress )
{
    int i, iOltBase;
    short int OnuIdx = RERROR;

	CHECK_PON_RANGE;

    if ( MacAddress != NULL )
    {
        iOltBase = PonPortIdx * MAXONUPERPON;
        ONU_MGMT_SEM_TAKE;
        for (i=0; i<MAXONUPERPON; i++)
        {
            if ( MAC_ADDR_IS_EQUAL(OnuMgmtTable[iOltBase+i].DeviceInfo.MacAddr, MacAddress) )
            {
                OnuIdx = (short int)i;
                break;
            }
        }
        ONU_MGMT_SEM_GIVE;
    }
    
    return OnuIdx;
}
short int  GetOnuIdxBySnPerPon( short int PonPortIdx, unsigned char *sn )
{
    int i, iOltBase;
    short int OnuIdx = RERROR;

	CHECK_PON_RANGE;

    if ( sn != NULL )
    {
        iOltBase = PonPortIdx * MAXONUPERPON;
        ONU_MGMT_SEM_TAKE;
        for (i=0; i<MAXONUPERPON; i++)
        {
            if ( VOS_StrCmp(OnuMgmtTable[iOltBase+i].DeviceInfo.DeviceSerial_No, sn) == 0)
            {
                OnuIdx = (short int)i;
                break;
            }
        }
        ONU_MGMT_SEM_GIVE;
    }
    
    return OnuIdx;
}

/* 2007-7-9 modified by chenfj 
	问题单:#4816 命令show onu infor 参数控制错误 */
short int  GetOnuIdxByName(unsigned char *Name )
{
	short int i, ret = RERROR;
	/*short int PonPortIdx, OnuIdx;
	unsigned long len;
	unsigned char DeviceName[MAXDEVICENAMELEN];*/
	unsigned long len;

	if( (Name == NULL) || (Name[0] == 0) )
		ret = RERROR;
	
	ONU_MGMT_SEM_TAKE;
	for( i=0; i<MAXONU; i++ )
	{
		/*PonPortIdx = i/MAXONUPERPON;
		OnuIdx = i%MAXONUPERPON;
		if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
			continue;
		if( GetOnuDeviceName( PonPortIdx, OnuIdx, DeviceName, (int *)&len) != ROK )
			continue;
		if( len > MAXDEVICENAMELEN )
			len = MAXDEVICENAMELEN;	*/

		if( MAC_ADDR_IS_INVALID(OnuMgmtTable[i].DeviceInfo.MacAddr) )
			continue;
		len = OnuMgmtTable[i].DeviceInfo.DeviceNameLen;
		if( len > MAXDEVICENAMELEN )
		{
			OnuMgmtTable[i].DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN;
			OnuMgmtTable[i].DeviceInfo.DeviceName[len] = 0;
		}
		if(  VOS_StrCmp( Name, OnuMgmtTable[i].DeviceInfo.DeviceName ) == 0 )
		{
			ret = i;
			break;
		}
	}
	ONU_MGMT_SEM_GIVE;

	return ret;
}

short int  GetOnuIdxByName_OnePon(unsigned char *Name, short int PonPortIdx )
{
	short int i, ret = RERROR;
	unsigned long len;
	unsigned char DeviceName[MAXDEVICENAMELEN];

	if( Name == NULL )
		return ret;
	CHECK_PON_RANGE;

	ONU_MGMT_SEM_TAKE;
	for( i=(PonPortIdx* MAXONUPERPON); i<((PonPortIdx+1)*MAXONUPERPON); i++ )
	{
		if(ThisIsValidOnu(PonPortIdx, i%MAXONUPERPON) != ROK )
			continue;
		if( GetOnuDeviceName( i/MAXONUPERPON, i%MAXONUPERPON, DeviceName, (int *)&len) != ROK )
			continue;
		if( len > MAXDEVICENAMELEN )
			len = MAXDEVICENAMELEN;
		
		if(  VOS_StrCmp( Name, OnuMgmtTable[i].DeviceInfo.DeviceName ) == 0 ) 
		{
			ret = i;
			break;
		}
	}
	ONU_MGMT_SEM_GIVE;

	return ret;
}

int  GetOnuTableEntryFromMacAddr( unsigned char MacAddr[6], unsigned int entryIdx )
{


	return ( ROK );
}

int  GetOnuTableEntryFromIfidx( short int PonPortIdx, short int OnuIdx, unsigned int entryIdx )
{
	CHECK_ONU_RANGE
		
	return ( ROK );
}

int  GetOnuTableEntryFromCustomerName( unsigned char *OnuCustomerName, unsigned int entryIdx )
{
			
	return ( ROK );
}

int  GetOnuTableEntryFromPASOnuId( unsigned char PonPortIdx, short int OnuIdx, unsigned int entryIdx )
{
	
	return ( ROK );
}


/**************************************************************
 *
 *    Function:  SetOnuVendorInfo(short int PonPortIdx, short int OnuIdx, char *Info, int len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *Info -- the vendor name to be set
 *                int len     -- the length of the vendor name 
 *
 *    Desc:  set the onu Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuVendorInfo( short int PonPortIdx, short int OnuIdx, char *Info, int len )
{
	/*int  GetSemID;*/
	int onuEntry;

	CHECK_ONU_RANGE;	

	if(Info == NULL ) return ( RERROR);
	if( len >= MAXVENDORINFOLEN ) /* modified by xieshl 20110527, 这种情况允许设置，防止ONU无法注册，下同 */
	{
		sys_console_printf("  Onu Vendor Name is too long (%d)\r\n", len );
		/*return RERROR;*/
		len = MAXVENDORINFOLEN - 1;
		Info[len] = 0;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.VendorInfo, '\0', MAXVENDORINFOLEN );
	if( len != 0)
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.VendorInfo, Info, len );
	OnuMgmtTable[onuEntry].DeviceInfo.VendorInfoLen = len;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuVendorInfo( short int PonPortIdx, short int OnuIdx, char *Info, int *len)
{
	int Type;
	CHECK_ONU_RANGE
	
	if( (Info == NULL )||( len == NULL)) {
		/*sys_console_printf("  error : the pointer is null ( GetOnuVendorName() )\r\n");*/
		VOS_ASSERT(0);
		return( RERROR );		
		}
	/* 应加ONU 类型判断*/

	Type = GetOnuVendorType( PonPortIdx, OnuIdx );
	if(  Type == ONU_VENDOR_GW )
	{
		*len = VOS_StrLen(typesdb_pruduct_onu_vendor_info());
		VOS_StrCpy(Info, typesdb_pruduct_onu_vendor_info());
		/*
		VOS_MemCpy( Info, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorInfo, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorInfoLen );
		*len = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorInfoLen ;
		Info[OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorInfoLen] = '\0';*/
	}
	else if ( Type == ONU_VENDOR_CT )
	{
		/*modify by wangxy 2007-03-29*/
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy( Info, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].device_vendor_id, 4 );
		ONU_MGMT_SEM_GIVE;
        if(*(ULONG *)Info == 0)
            *len = 0;
        else
        {
    		Info[4] = 0;
    		*len = 4;
        }
	}
	else return( RERROR );
	
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuSerialNo(short int PonPortIdx, short int OnuIdx, char *Info, int len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *Info -- the serial-No to be set
 *                int len     -- the length of  serial-No 
 *
 *    Desc:  set the onu serial No
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuSerialNo( short int PonPortIdx, short int OnuIdx, char *Info, int len )
{
	/*int  GetSemID;*/
	int onuEntry;

	CHECK_ONU_RANGE	

	if(Info == NULL ) return ( RERROR);
	if( len >= MAXSNLEN ) {
		sys_console_printf("  Onu device serial No is too long (%d)\r\n", len );
		len = MAXSNLEN - 1;
		Info[len] = 0;
		/*return RERROR;*/
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, '\0', MAXSNLEN );
	if( len != 0)
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, Info, len );
	OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen = len;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuSerialNo( short int PonPortIdx, short int OnuIdx, char *Info, int *len)
{
	int /*Type,*/ onuEntry;
	CHECK_ONU_RANGE
	
	if( (Info == NULL )||( len == NULL)) {
		VOS_ASSERT(0);
		return( RERROR );		
		}
	/* 应加ONU 类型判断*/
	/*Type = GetOnuVendorType( PonPortIdx, OnuIdx );*/
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
#if 1   /*added by wangying 问题单31182 */
		if( OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen > MAXSNLEN )
			OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen = MAXSNLEN;
		VOS_StrnCpy( Info, OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen);
		*len = VOS_StrLen(Info); 
		Info[*len] = '\0';
#endif
#if 0
	if( OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen > MAXSNLEN )
		OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen = MAXSNLEN;
	VOS_StrnCpy( Info, OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_No, OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen);
	*len = OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen;
	Info[OnuMgmtTable[onuEntry].DeviceInfo.DeviceSerial_NoLen] = '\0';
#endif
	ONU_MGMT_SEM_GIVE;
		
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuProduceDate(short int PonPortIdx, short int OnuIdx, char *Info, int len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *Info -- the madeDate to be set
 *                int len     -- the length of madeDate 
 *
 *    Desc:  set the onu madeDate
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuProduceDate( short int PonPortIdx, short int OnuIdx, char *Info, int len )
{
	/*int  GetSemID;*/
	int onuEntry;
	
	CHECK_ONU_RANGE;

	if(Info == NULL ) return ( RERROR);
	if( len >= MAXDATELEN ) {
		sys_console_printf("  Onu device madeDate is to long (%d)\r\n", len );
		/*pktDataPrintf( Info, len );
		return RERROR;*/
		len = MAXDATELEN - 1;
		Info[len] = 0;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.MadeInDate, '\0', MAXDATELEN );
	if( len != 0)
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.MadeInDate, Info, len );
	OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen = len;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuProduceDate( short int PonPortIdx, short int OnuIdx, char *Info, int *len)
{
	int /*Type,*/onuEntry;
	CHECK_ONU_RANGE
	
	if( (Info == NULL )||( len == NULL)) {
		VOS_ASSERT(0);
		return( RERROR );		
		}
	/* 应加ONU 类型判断*/

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*Type = GetOnuVendorType( PonPortIdx, OnuIdx );*/
	ONU_MGMT_SEM_TAKE;
	if( OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen > MAXDATELEN )
		OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen = MAXDATELEN;
	VOS_MemCpy( Info, OnuMgmtTable[onuEntry].DeviceInfo.MadeInDate, OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen);
	*len = OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen;
	Info[OnuMgmtTable[onuEntry].DeviceInfo.MadeInDateLen] = '\0';
	ONU_MGMT_SEM_GIVE;
		
	return( ROK );
}

/** below this */
/**************************************************************
 *
 *    Function:  SetOnuVendorLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Name, unsigned char len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Location -- the vendor Location to be set
 *                 unsigned char  len -- the length of the vendor location 
 *
 *    Desc:  set the onu Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0 
int  SetOnuVendorLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOnuVendorLocation() )\r\n");
		return ( RERROR );
		}

	if( len > MAXVENDORLOCATIONLEN ) {
		sys_console_printf(" Olt Vendor Location is to long %d (SetOnuVendorLocation() )\r\n", len);
		len = MAXVENDORLOCATIONLEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorLocation, 0, MAXVENDORLOCATIONLEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorLocation, Location, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorLocationLen = len;
	
	VOS_SemGive(OnuMgmtTableDataSemId);	
	return ( ROK );

}


int  GetOnuVendorLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location )
{
	CHECK_ONU_RANGE
	
	if( Location== NULL ) {
		sys_console_printf("error : the pointer is null ( GetOnuVendorLocation() )\r\n");
		return( RERROR );		
		}
	VOS_MemCpy( Location, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorLocation,OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorLocationLen);
	return( ROK );

}

/**************************************************************
 *
 *    Function:  SetOnuVendorContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Telenum -- the vendor Contact to be set
 *                unsigned char  len -- the length of the vendor Contact 
 *
 *    Desc:  set the onu Vendor Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuVendorContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOnuVendorContact() )\r\n");
		return ( RERROR );
		}

	if( len > MAXVENDORCONTACTLEN ) {
		sys_console_printf(" Olt Vendor Contact is to long %d (SetOnuVendorContact() )\r\n", len);
		len = MAXVENDORCONTACTLEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorContact, 0, MAXVENDORCONTACTLEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorContact, Telenum, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorContactLen = len;
	
	VOS_SemGive(OnuMgmtTableDataSemId);
	return ( ROK );
}


int  GetOnuVendorContact( short int PonPortIdx, short int OnuIdx, unsigned char *Contact )
{
	CHECK_ONU_RANGE
	
	if( Contact == NULL ) {
		sys_console_printf("error : the pointer is null ( GetOnuVendorContact() )\r\n");
		return( RERROR );		
		}
	VOS_MemCpy( Contact, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorContact, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.VendorContactLen);
	return( ROK );
}
/**************************************************************
 *
 *    Function:  SetOnuCustomerName( short int PonPortIdx, short int OnuIdx, unsigned char *Name, unsigned char len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Name -- the Customer name to be set
 *                unsigned char len -- the length of the Customer name 
 *
 *    Desc:  set the Onu Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuCustomerName( short int PonPortIdx, short int OnuIdx, unsigned char *Name, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOnuCustomerName() )\r\n" );
		return ( RERROR );
		}

	if( len > MAXCUSTOMERNAMELEN ) {
		sys_console_printf(" Olt Customer Name is to long %d ( SetOnuCustomerName() )\r\n", len );
		len = MAXCUSTOMERNAMELEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerName, 0, MAXCUSTOMERNAMELEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerName, Name, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerNameLen = len;
	
	VOS_SemGive(OnuMgmtTableDataSemId);
	return ( ROK );
}


int  GetOnuCustomerName( short int PonPortIdx, short int OnuIdx, unsigned char *Name )
{
	CHECK_ONU_RANGE
	
	if( Name == NULL ) {
		sys_console_printf("error : the pointer is null (GetOnuCustomerName() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Name, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerName, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerNameLen );
	return( ROK );
}
/**************************************************************
 *
 *    Function:  SetOnuCustomerLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location, unsigned char len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Location -- the Customer Location to be set
 *                unsigned char  len -- the length of the Customer location 
 *
 *    Desc:  set the onu Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuCustomerLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOnuCustomerLocation() )\r\n");
		return ( RERROR );
		}

	if( len > MAXCUSTOMERLOCATIONLEN ) {
		sys_console_printf(" Olt Customer Location is to long %d (SetOnuCustomerLocation() )\r\n", len );
		len = MAXCUSTOMERLOCATIONLEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerLocation, 0, MAXCUSTOMERLOCATIONLEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerLocation, Location, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerLocationLen = len;
	
	VOS_SemGive(OnuMgmtTableDataSemId);
	return ( ROK );
}

int  GetOnuCustomerLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location )
{
	CHECK_ONU_RANGE
	
	if( Location == NULL ) {
		sys_console_printf("error : the pointer is null ( GetOnuCustomerLocation() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Location, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerLocation, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerLocationLen);
	return( ROK );
}
/**************************************************************
 *
 *    Function:  SetOnuCustomerContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Telenum -- the Customer Contact to be set
 *                unsigned char  len -- the length of the Customer Contact 
 *
 *    Desc:  set the onu Customer Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuCustomerContact( short int PonPortIdx, short int OnuIdx, unsigned char *Telenum, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error (SetOnuCustomerContact() )\r\n");
		return ( RERROR );
		}
	
	if( len > MAXCUSTOMERCONTACTLEN) {
		sys_console_printf(" Olt Customer Contact is to long %d ( SetOnuCustomerContact() ) \r\n", len);
		len = MAXCUSTOMERCONTACTLEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerContact, 0, MAXCUSTOMERCONTACTLEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerContact, Telenum, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerContactLen = len;

	ONU_MGMT_SEM_GIVE;
	return ( ROK );
}


int  GetOnuCustomerContact( short int PonPortIdx, short int OnuIdx, unsigned char *Contact )
{
	CHECK_ONU_RANGE
	
	if( Contact == NULL ) {
		sys_console_printf("error : the pointer is null ( GetOnuCustomerContact() )\r\n" );
		return( RERROR );		
		}
	VOS_MemCpy( Contact, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerContact, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.CustomerContactLen );
	return( ROK );
}
#endif
/**************************************************************
 *
 *    Function:  SetOnuDeviceName( short int PonPortIdx, short int OnuIdx, char *Name, int len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *Name -- the Device name to be set
 *                int  len -- the length of the Device Name 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuDeviceName_1( short int PonPortIdx, short int OnuIdx,  char *Name,  int  NameLen )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( Name == NULL )
	{
		VOS_ASSERT(0);
		return( RERROR );
	}
	if( NameLen >= MAXDEVICENAMELEN ) {/* 留出结束符 */
		sys_console_printf("  Onu Device Name is to long (%d)\r\n", NameLen);
		/*return RERROR;*/
		NameLen = MAXDEVICENAMELEN -1;
		Name[NameLen] = 0;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(( NameLen > 1) || ((NameLen == 1) && ( *Name != 0 )))
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.DeviceName, Name, NameLen );
	VOS_MemZero( &OnuMgmtTable[onuEntry].DeviceInfo.DeviceName[NameLen], MAXDEVICENAMELEN - NameLen );
	OnuMgmtTable[onuEntry].DeviceInfo.DeviceNameLen = NameLen;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuDeviceName( short int PonPortIdx, short int OnuIdx,  char *Name , int *NameLen)
{
    int OnuMgtIdx;

	CHECK_ONU_RANGE;
	
	if( Name == NULL )
    {
		/*sys_console_printf("  error : the pointer is null (GetOnuDeviceName() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );		
	}

   	 OnuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	/**NameLen  = OnuMgmtTable[OnuMgtIdx].DeviceInfo.DeviceNameLen;*/
    /*modified by luh 2012-11-22,解决支持设置onu device-name，但是不支持私有设备信息获取的onu上报trap异常*/
    *NameLen = strlen(OnuMgmtTable[OnuMgtIdx].DeviceInfo.DeviceName);
	if( *NameLen > MAXDEVICENAMELEN )
		*NameLen = MAXDEVICENAMELEN;
	VOS_MemCpy( Name, OnuMgmtTable[OnuMgtIdx].DeviceInfo.DeviceName, *NameLen);
	ONU_MGMT_SEM_GIVE;
	Name[*NameLen] = '\0';
	return( ROK );
}

int  GetOnuDeviceNameByIdx( short int slot, short int port, short int  OnuIdx, char *OnuName, int *len  )
{
	short int PonPortIdx; 
	/*if((slot > PON1 ) ||(slot < PON5)) return( RERROR );
	
	if(__SYS_MODULE_TYPE__(slot+1) != MODULE_E_GFA_EPON ) return( RERROR );*/
	if(SlotCardIsPonBoard(slot) != ROK ) return(RERROR);
	
	PonPortIdx = GetPonPortIdxBySlot( slot, port);
	CHECK_ONU_RANGE

	if((OnuName == NULL ) ||( len == NULL )) return( RERROR );

	return( GetOnuDeviceName( PonPortIdx, OnuIdx, OnuName, len ));
}

int GetOnuDeviceIdxByName_OnePon( char *OnuName, int len, short  int slot, short int port, short int *OnuIdx1)
{
	short int ret;
	short int PonPortIdx;
	
	if((OnuName == NULL ) ||(OnuIdx1 == NULL) || (len > MAXDEVICENAMELEN) )
		return( RERROR );

	PonPortIdx = GetPonPortIdxBySlot( slot, port);
	CHECK_PON_RANGE
		
	ret = GetOnuIdxByName_OnePon( OnuName, PonPortIdx ); 
	if( ret != RERROR )
		{
		*OnuIdx1 = ret % MAXONUPERPON;
		return( ROK );
		}
	else return (V2R1_ONU_NOT_EXIST);
}


int GetOnuDeviceIdxByName( char *OnuName, int len, short  int *slot, short int *port, short int *OnuIdx1)
{
	short int ret, PonPortIdx;
	
	if((OnuName == NULL ) ||(slot == NULL)||(port == NULL)||(OnuIdx1 == NULL )) return( RERROR );

	ret = GetOnuIdxByName( OnuName ); 
	if( ret != RERROR )
	{
	    PonPortIdx = ret / MAXONUPERPON;
		*OnuIdx1 = ret % MAXONUPERPON;
		*slot = GetCardIdxByPonChip( ret / MAXONUPERPON );
		*port = GetPonPortByPonChip( ret / MAXONUPERPON );
		return( ROK );
	}
	else return (V2R1_ONU_NOT_EXIST);
}
/**************************************************************
 *
 *    Function:  SetOnuDeviceDesc( short int PonPortIdx, short int OnuIdx, char *Desc, int len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *Name -- the Device name to be set
 *                int len -- the length of the Device Desc 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuDeviceDesc_1( short int PonPortIdx, short int OnuIdx,  char *Desc,  int  len )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( Desc == NULL )
	{
		VOS_ASSERT(0);
		return( RERROR );
	}
	if( len >= MAXDEVICEDESCLEN ) {	/* 留出结束符 */
		sys_console_printf("  Olt Device Name is to long (%d)\r\n", len);
		len = MAXDEVICEDESCLEN - 1;
		Desc[len] = 0;
		/*return RERROR;*/
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	if(( len > 1) ||( ( len ==1) &&( *Desc != 0 )))
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc, Desc, len );
	VOS_MemZero( &OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc[len], MAXDEVICEDESCLEN - len );
	OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen= len;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuDeviceDesc( short int PonPortIdx, short int OnuIdx,  char *Desc , int *len)
{	
	int onuEntry;
	CHECK_ONU_RANGE
	
	if( Desc == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );
		}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	/*VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc, '\0', MAXDEVICENAMELEN );*/	/* modified by xieshl 20110328, 问题单12448 */
	if( OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen > MAXDEVICEDESCLEN )
		OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen = MAXDEVICEDESCLEN;
	VOS_MemCpy( Desc, OnuMgmtTable[onuEntry].DeviceInfo.DeviceDesc, OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen );
	*len  = OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen;
	Desc[OnuMgmtTable[onuEntry].DeviceInfo.DeviceDescLen] = '\0';
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuDeviceType( short int PonPortIdx, short int OnuIdx, unsigned char *Type, unsigned char len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Type -- the Device Type to be set
 *                unsigned char  len -- the length of the Device Type 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
#if 0
int  SetOnuDeviceType( short int PonPortIdx, short int OnuIdx, unsigned char *Type, unsigned char len )
{

	CHECK_ONU_RANGE
	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) ==  VOS_ERROR ) {
		sys_console_printf( "get SemId error (  SetOnuDeviceType() )\r\n" );
		return ( RERROR );
		}

	if( len > MAXDEVICETYPELEN ) {
		sys_console_printf(" Olt Device Type is to long %d ( SetOnuDeviceType() ) \r\n", len );
		len = MAXDEVICETYPELEN;
		}
	VOS_MemSet( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceType, 0, MAXDEVICETYPELEN );
	VOS_MemCpy( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceType, Type, len );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceTypeLen = len;

	VOS_SemGive(OnuMgmtTableDataSemId);
	return ( ROK );
}

int  GetOnuDeviceType( short int PonPortIdx, short int OnuIdx, unsigned char *Type )
{
	CHECK_ONU_RANGE
	
	if( Type == NULL ) {
		sys_console_printf("error : the pointer is null ( GetOnuDeviceType() )\r\n");
		return( RERROR );		
		}
	VOS_MemCpy( Type, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceType, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.DeviceTypeLen );
	return( ROK );
}
#endif
int SetOnuType( short int PonPortIdx, short int OnuIdx, int type )
{
	CHECK_ONU_RANGE;

	/* type 判断*/ 

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].DeviceInfo.type = type;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

int GetOnuType( short int PonPortIdx, short int OnuIdx, int *type )
{
	CHECK_ONU_RANGE

	if( type == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );		
	}
	ONU_MGMT_SEM_TAKE;
	*type = (int )OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].DeviceInfo.type;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}
/*added by wangjiah@2017-01-18 begin*/
int SetNeedToRestoreConfFile(short int PonPortIdx, short int OnuIdx, unsigned char isNeeded)
{
	CHECK_ONU_RANGE

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].needToRestoreConfFile = isNeeded;
	ONU_MGMT_SEM_GIVE;
	return (ROK);
}

int GetNeedToRestoreConfFile(short int PonPortIdx, short int OnuIdx, unsigned char * isNeeded)
{
	CHECK_ONU_RANGE

	if( isNeeded == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );		
	}
	ONU_MGMT_SEM_TAKE;
	*isNeeded= (int )OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].needToRestoreConfFile;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}
/*added by wangjiah@2017-01-18 end*/

int GetOnuChipVenderID(OnuVendorTypes vendorType)
{
    int iPonChipVendor = PONCHIP_VENDOR_UNKNOWN;

    if ( vendorType & OnuVendorTypesPmc )
    {
        iPonChipVendor = PONCHIP_VENDOR_PAS;
    }
    else if ( vendorType & OnuVendorTypesTeknovus )
    {
        iPonChipVendor = PONCHIP_VENDOR_TK;
    }
    else if ( vendorType & OnuVendorTypesInterop )
    {
        iPonChipVendor = PONCHIP_VENDOR_CT;
    }
    else
    {
        NULL;
    }
    
    return iPonChipVendor;
}

int GetOnuChipVenderTypeByOUI(unsigned char vendor_oui[3])
{
    OnuVendorTypes vendorType = OnuVendorTypesUnknown;


    return vendorType;
}

int GetOnuChipVendor( short int PonPortIdx, short int OnuIdx )
{
	int type;
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	type = (int)OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].DeviceInfo.PonChipVendor;
	ONU_MGMT_SEM_GIVE;

	return type;
}

int GetOnuChipType( short int PonPortIdx, short int OnuIdx )
{
	int type;
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	type = (int)OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].DeviceInfo.PonChipType;
	ONU_MGMT_SEM_GIVE;

	return type;
}

char *OnuTypeToString( int type )
{
	if(( type < V2R1_ONU_GT811 ) ||(type >= V2R1_ONU_MAX )) return "-";
	if( VOS_StrLen(&(DeviceType_1[type][0])) >= ONU_TYPE_LEN )
		DeviceType_1[type][ONU_TYPE_LEN] = 0;
	return &(DeviceType_1[type][0]);
}

int GetOnuTypeString(short int PonPortIdx, short int OnuIdx, char *TypeString, int *len )
{
	int type;
	int ret;
	int typeLen;
	
	if((TypeString == NULL )  ||( len == NULL )) return( RERROR );

	CHECK_ONU_RANGE

	ret = GetOnuType( PonPortIdx, OnuIdx, &type );

	if ( ret != ROK ) return( RERROR );
	if(( type < V2R1_ONU_GT811 ) ||(type >= V2R1_ONU_MAX )) return (RERROR );

	if(type == V2R1_ONU_GPON)
	{
		ULONG typeid = 0;
		GPON_OnuModel_Translate(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.equipmentID,&typeid,TypeString,len);
	}
	else if(type == V2R1_ONU_CTC)
	{
		ULONG typeid = 0;
		CTC_OnuModel_Translate(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].onu_model, &typeid, TypeString, len);
	}
	else
	{
        typeLen = VOS_StrLen(&(DeviceType_1[type][0]));
        if( typeLen >  ONU_TYPE_LEN ) typeLen = ONU_TYPE_LEN;
        *len = typeLen;
        if(typeLen  < ONU_TYPE_LEN )
            TypeString[typeLen]='\0';

        VOS_MemCpy( TypeString, &(DeviceType_1[type][0]), typeLen );
	}

	return( ROK );
}
int  GetOnuModelId( short int PonPortIdx, short int OnuIdx, int *model )
{
	int onuEntry = 0;
	CHECK_ONU_RANGE;
	
	onuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	*model = OnuMgmtTable[onuEntry].onu_model;
	ONU_MGMT_SEM_GIVE;
	
	return ROK;
}
int SeachOnuIsCtcByOnuid(short int PonPortIdx,short int OnuIdx)
{
	int iRlt= VOS_OK;
	int onuEntry = 0;
	int typeid = 0;
	ULONG model = 0;
	/*char pModel;
	int pLen;
	onuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	model = OnuMgmtTable[onuEntry].onu_model;
	ONU_MGMT_SEM_GIVE;
	iRlt = CTC_OnuModel_Translate(model, &typeid, &pModel, &pLen);*/
	GetOnuType(PonPortIdx,OnuIdx, &typeid);
	if(typeid == V2R1_ONU_CTC)
	{
		return VOS_OK;
	}
	else
	{
		return VOS_ERROR;
	}
}
/* b-判断字符串是否是可显示(0-9,a-z,A-Z,-,_),added by zhaoxh*/
int CheckIfAscllValid(char *array, int strlen, int array_len_max)
{
    int index = 0; 

	if((NULL == array) || (0 == strlen))
	{
		return VOS_ERROR;
	}

	if (strlen > array_len_max)
	{
		strlen = array_len_max;
	}
	
	for (index=0; index < strlen; index++)
	{
		if (((array[index] >= '0') && (array[index] <= '9'))
		 || ((array[index] >= 'a') && (array[index] <= 'z'))
		 || ((array[index] >= 'A') && (array[index] <= 'Z'))
		 || (array[index] == '-') || (array[index] == '_'))
		{
			continue;
		}
		else
		{
			return VOS_ERROR;
		}
	}
	
	return VOS_OK;
}
/* e-判断字符串是否是可显示(0-9,a-z,A-Z,-,_),added by zhaoxh*/

int  GetOnuModel( short int PonPortIdx, short int OnuIdx, char *pModel, int *pLen )
{
	int onuEntry = 0, onuType = 0, iRlt = VOS_OK;;
	ULONG model = 0, typeid = 0;
	unsigned char verdor[5] = {0,0,0,0,0};
	char equipmentID[MAXEQUIPMENTIDLEN];
	unsigned char GE_PORT_NUM = 0, FE_PORT_NUM = 0;
	unsigned char onu_model[5]={0, 0, 0, 0, 0};
	int oam_ver = 0;
	char extendModel[17]={ 0 };
    
	CHECK_ONU_RANGE;
	
	if( (pModel == NULL) || (pLen == NULL) )
	{
		VOS_ASSERT(0);
		return( RERROR );
	}
	/*if( GetOnuType( PonPortIdx, OnuIdx, &OnuType )  != ROK ) 
		return( RERROR );*/
	onuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	onuType = OnuMgmtTable[onuEntry].DeviceInfo.type;
	model = OnuMgmtTable[onuEntry].onu_model;
	VOS_MemCpy(onu_model, &OnuMgmtTable[onuEntry].onu_model, 4);
	VOS_MemCpy(verdor, OnuMgmtTable[onuEntry].device_vendor_id, 4);
	VOS_MemCpy(equipmentID, OnuMgmtTable[onuEntry].DeviceInfo.equipmentID, MAXEQUIPMENTIDLEN);
	GE_PORT_NUM = OnuMgmtTable[onuEntry].GE_Ethernet_ports_number;
	FE_PORT_NUM = OnuMgmtTable[onuEntry].FE_Ethernet_ports_number;
	oam_ver = OnuMgmtTable[onuEntry].onu_ctc_version;
	VOS_MemCpy(extendModel, OnuMgmtTable[onuEntry].extendedModel, 17);
	ONU_MGMT_SEM_GIVE;
	
	if( V2R1_CTC_STACK && (onuType == V2R1_ONU_CTC) )
	{
	    iRlt = CTC_OnuModel_Translate(model, &typeid, pModel, pLen);
		if(typeid == V2R1_ONU_CTC)/*modified by yangzl@2016-8-9*/
		{
			uint_t vendor_len = VOS_StrLen(verdor);
			uint_t model_len = VOS_StrLen(onu_model);
			uint_t extModel_len = VOS_StrLen(extendModel);
			
			if(vendor_len>0 && model_len>0)
			{
				if(VOS_OK == (CheckIfAscllValid(verdor,vendor_len,4)) 
					&& (VOS_OK == CheckIfAscllValid(onu_model, model_len, 4))) 
				{
					*pLen = VOS_Snprintf(pModel, 12, "%s-%s", verdor, onu_model);
				}
				else
				{
					*pLen = VOS_Snprintf(pModel, 7, "ONU-%d", FE_PORT_NUM+GE_PORT_NUM );		
				}
			}                
			else if(oam_ver == 0x30)
			{
			    if(VOS_OK == CheckIfAscllValid(extendModel, extModel_len, 16))
			    {			    	
					VOS_MemCpy(pModel, extendModel, ONU_TYPE_LEN);
					*pLen = VOS_StrLen(pModel);
				}
				else
				{
					*pLen = VOS_Snprintf(pModel, 7, "ONU-%d", FE_PORT_NUM+GE_PORT_NUM );	
				}
				
			} 
			else            
            	*pLen = VOS_Snprintf(pModel, 7, "ONU-%d", FE_PORT_NUM+GE_PORT_NUM );	
		}
#if 0
		 if(oam_ver == 0x30 && onuType == V2R1_ONU_CTC)
		{
			VOS_MemCpy(pModel, extendModel, 17);
			*pLen = VOS_StrLen(pModel);
		}
		else if(typeid == V2R1_ONU_CTC)
	     {

            *pLen = VOS_Snprintf(pModel, 15, "%s-%s", verdor, onu_model);
          
	     }
#endif  

		return iRlt;
	}
    else if (onuType == V2R1_ONU_CMC)
    {
    	VOS_StrnCpy( pModel, "CMC", ONU_TYPE_LEN );
    	*pLen = VOS_StrLen(pModel);
    	return ROK;
    }
    else if(onuType == V2R1_ONU_GPON)
    {
        iRlt = GPON_OnuModel_Translate(equipmentID, &typeid, pModel, pLen);
        if(typeid == V2R1_ONU_GPON)
        {
            /*修改其他厂商的gpon onu显示为equipmentID add by zhaoxh*/
    	    VOS_StrnCpy( pModel, equipmentID, ONU_TYPE_LEN );
    	    *pLen = VOS_StrLen(pModel);
    	}
    	return iRlt;
    }
	/*return GetOnuTypeString( PonPortIdx, OnuIdx, pModel, pLen);*/

	VOS_MemZero( pModel, ONU_TYPE_LEN );
	VOS_StrnCpy( pModel, OnuTypeToString(onuType), ONU_TYPE_LEN );
	*pLen = VOS_StrLen(pModel);
	
	return ROK;
}

int GetOnuAppTypeString( short int PonPortIdx, short int OnuIdx, unsigned char *AppTypeString, int *len )
{
	int ret, OnuType;
	int typeLen;
	
	if(( AppTypeString == NULL ) || ( len == NULL )) return( RERROR );
	CHECK_ONU_RANGE

	ret = GetOnuType( PonPortIdx, OnuIdx, &OnuType );

	if ( ret != ROK ) return( RERROR );
	if(( OnuType < V2R1_ONU_GT811 ) ||(OnuType >= V2R1_ONU_MAX )) return (RERROR );

	typeLen = VOS_StrLen(&(OnuAppType_1[OnuType][0]));
	if( typeLen >  ONU_TYPE_LEN ) typeLen = ONU_TYPE_LEN;
	*len = typeLen;
	if(typeLen  < ONU_TYPE_LEN )
		AppTypeString[typeLen]='\0';
	
	VOS_MemCpy( AppTypeString, &(OnuAppType_1[OnuType][0]), typeLen );

	return( ROK );

}

int GetOnuVoiceTypeString( short int PonPortIdx, short int OnuIdx, unsigned char *VoiceTypeString, int *len )
{
	int ret, OnuType;
	int typeLen;
	
	if(( VoiceTypeString == NULL ) || ( len == NULL )) return( RERROR );
	CHECK_ONU_RANGE

	ret = GetOnuType( PonPortIdx, OnuIdx, &OnuType );

	if ( ret != ROK ) return( RERROR );
	if(( OnuType < V2R1_ONU_GT811 ) ||(OnuType >= V2R1_ONU_MAX )) return (RERROR );

	typeLen = VOS_StrLen(&(OnuVoiceType_1[OnuType][0]));
	if( typeLen >  ONU_TYPE_LEN ) typeLen = ONU_TYPE_LEN;
	*len = typeLen;
	if(typeLen  < ONU_TYPE_LEN )
		VoiceTypeString[typeLen]='\0';
	
	VOS_MemCpy( VoiceTypeString, &(OnuVoiceType_1[OnuType][0]), typeLen );

	return( ROK );

}

int GetOnuFPGATypeString( short int PonPortIdx, short int OnuIdx, unsigned char *FPGATypeString, int *len )
{
	int ret, OnuType;
	int typeLen;
	
	if(( FPGATypeString == NULL ) || ( len == NULL )) return( RERROR );
	CHECK_ONU_RANGE

	ret = GetOnuType( PonPortIdx, OnuIdx, &OnuType );

	if ( ret != ROK ) return( RERROR );
	if(( OnuType < V2R1_ONU_GT811 ) ||(OnuType >= V2R1_ONU_MAX )) return (RERROR );

	typeLen = VOS_StrLen(&(OnuFpgaType_1[OnuType][0]));
	if( typeLen >  ONU_TYPE_LEN ) typeLen = ONU_TYPE_LEN;
	*len = typeLen;
	if(typeLen  < ONU_TYPE_LEN )
		FPGATypeString[typeLen]='\0';
	
	VOS_MemCpy( FPGATypeString, &(OnuFpgaType_1[OnuType][0]), typeLen );

	return( ROK );

}

int SetOnuOUI( short int PonPortIdx, short int OnuIdx, unsigned char *OUI )
{
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy( OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].DeviceInfo.OUI , OUI, sizeof(OnuMgmtTable[0].DeviceInfo.OUI));
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

int GetOnuOUI( short int PonPortIdx, short int OnuIdx, unsigned char *OUI )
{
	CHECK_ONU_RANGE;

	if( OUI == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy(OUI, OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].DeviceInfo.OUI,  sizeof(OnuMgmtTable[0].DeviceInfo.OUI));
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

int OnuIsCMC( short int PonPortIdx, short int OnuIdx )
{
    int isCmc = FALSE;
    int iOnuType;

    if ( ROK == GetOnuType(PonPortIdx, OnuIdx, &iOnuType) )
    {
        if ( V2R1_ONU_CMC == iOnuType )
        {
            isCmc = TRUE;
        }
    }

    return isCmc;
}

int  SetOnuPonType( short int PonPortIdx, short int OnuIdx, int type )
{
	CHECK_ONU_RANGE;
	/* type 判断*/ 
	if( (type > EPONMAUTYPE1000BASEPX20UONU   ) ||( type < EPONMAUTYPE1000BASEPXOLT )) return ( RERROR );

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].PonType = type;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

int  GetOnuPonType( short int PonPortIdx, short int OnuIdx, int  *type )
{
	CHECK_ONU_RANGE;

	if( type == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	ONU_MGMT_SEM_TAKE;
	*type = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].PonType;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}


int  GetOnuPonRate( short int PonPortIdx, short int OnuIdx, int  *rate )
{
	CHECK_ONU_RANGE;

	if( rate == NULL ) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	ONU_MGMT_SEM_TAKE;
	*rate = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].PonRate;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}


/**************************************************************
 *
 *    Function:  SetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *SwVersion, int len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *SwVersion -- the Onu SwVersion to be set
 *                int len -- the length of the Onu SwVersion 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *SwVersion, int len )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( SwVersion== NULL  ) return( RERROR );
	if( len > MAXSWVERSIONLEN ) {
		/*sys_console_printf(" Olt SwVersion is to long %d  ( SetOnuSWVersion() )\r\n", len );*/
		return RERROR; 
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.SwVersion, '\0', MAXSWVERSIONLEN );
	OnuMgmtTable[onuEntry].DeviceInfo.SwVersionLen = len;
	if( len > 0) 
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.SwVersion, SwVersion, len );
	ONU_MGMT_SEM_GIVE;

	return ( ROK );

}

int  GetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *Version, int *len )
{
	int onuEntry;
	CHECK_ONU_RANGE;
	
	if((Version == NULL )||( len == NULL)) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    
	ONU_MGMT_SEM_TAKE;
	*len = OnuMgmtTable[onuEntry].DeviceInfo.SwVersionLen;
	if( *len != 0) 
	{
    	VOS_MemCpy( Version, OnuMgmtTable[onuEntry].DeviceInfo.SwVersion, *len);
    	*len = VOS_StrLen(Version);
	}
	ONU_MGMT_SEM_GIVE;
	Version[*len] = '\0';
    
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuSWVersion( short int PonPortIdx, short int OnuIdx, char *HwVersion, int len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *HwVersion -- the Onu HwVersion to be set
 *                int len -- the length of the Onu HwVersion 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuHWVersion( short int PonPortIdx, short int OnuIdx, char *HwVersion, int len )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( HwVersion == NULL ) return ( RERROR );
	if( len > MAXHWVERSIONLEN ) {
		/*sys_console_printf(" Olt HwVersion is to long %d\r\n", len );*/
		return RERROR;;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.HwVersion, '\0', MAXHWVERSIONLEN );
	OnuMgmtTable[onuEntry].DeviceInfo.HwVersionLen = len;
	if( len > 0)
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.HwVersion, HwVersion, len );
	ONU_MGMT_SEM_GIVE;

	return ( ROK );

}

int  GetOnuHWVersion( short int PonPortIdx, short int OnuIdx, char *Version, int *len)
{
	int onuEntry;
	CHECK_ONU_RANGE;
	
	if( (Version == NULL ) || ( len == NULL)){
		VOS_ASSERT(0);
		return( RERROR );
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*len = OnuMgmtTable[onuEntry].DeviceInfo.HwVersionLen;
	if( *len != 0) 
	{
		VOS_MemCpy( Version, OnuMgmtTable[onuEntry].DeviceInfo.HwVersion, *len);
	}
	ONU_MGMT_SEM_GIVE;
	Version[*len] = '\0';
	
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuBootVersion( short int PonPortIdx, short int OnuIdx, char *BootVersion, int len )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *BootVersion -- the Onu BootVersion to be set
 *                int len -- the length of the Onu BootVersion 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuBootVersion( short int PonPortIdx, short int OnuIdx, char *BootVersion, int len )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( BootVersion == NULL ) return( RERROR );
	if( len > MAXBOOTVERSIONLEN ) {
		/*sys_console_printf(" Olt HwVersion is to long %d  (SetOnuBootVersion() )\r\n", len );*/
		return RERROR;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.BootVersion, '\0', MAXBOOTVERSIONLEN );
	OnuMgmtTable[onuEntry].DeviceInfo.BootVersionLen = len;
	if(len > 0 )
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.BootVersion, BootVersion, len );
	ONU_MGMT_SEM_GIVE;

	return ( ROK );

}

int  GetOnuBootVersion( short int PonPortIdx, short int OnuIdx, char *Version, int *len)
{
	int onuEntry;
	CHECK_ONU_RANGE;
	
	if( (Version == NULL ) || ( len == NULL)){
		VOS_ASSERT(0);
		return( RERROR );
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*len = OnuMgmtTable[onuEntry].DeviceInfo.BootVersionLen;
	if( *len != 0) 
	{
		VOS_MemCpy( Version, OnuMgmtTable[onuEntry].DeviceInfo.BootVersion, *len);
	}
	ONU_MGMT_SEM_GIVE;
	Version[*len] = '\0';
    
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuFWVersion( short int PonPortIdx, short int OnuIdx, char *FwVersion, int len );
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *HwVersion -- the Onu FwVersion to be set
 *                int len -- the length of the Onu FwVersion 
 *
 *    Desc:  set the Onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuFWVersion( short int PonPortIdx, short int OnuIdx, char *FwVersion, int len )
{
	int onuEntry;
	CHECK_ONU_RANGE;

	if( FwVersion == NULL ) return( RERROR );
	if( len > MAXFWVERSIONLEN ) {
		/*sys_console_printf(" Olt FwVersion is to long %d ( SetOnuFWVersion() )\r\n", len );*/
		return RERROR; 
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.FwVersion, '\0', MAXFWVERSIONLEN );
	OnuMgmtTable[onuEntry].DeviceInfo.FwVersionLen = len;
	if( len > 0 )
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.FwVersion, FwVersion, len );
	ONU_MGMT_SEM_GIVE;

	return ( ROK );

}

int  GetOnuFWVersion( short int PonPortIdx, short int OnuIdx, char *Version , int *len )
{
	int onuEntry;
	CHECK_ONU_RANGE
	
	if( (Version == NULL )||( len == NULL )) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*len = OnuMgmtTable[onuEntry].DeviceInfo.FwVersionLen;
	if( *len != 0) 
	{
		VOS_MemCpy( Version, OnuMgmtTable[onuEntry].DeviceInfo.FwVersion, *len);
	}
	ONU_MGMT_SEM_GIVE;
	Version[*len] = '\0';

	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuLocation( short int PonPortIdx, short int OnuIdx, unsigned char *Location, unsigned char len )
 *
 *    Param:  short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *Location -- the Onu Device Location to be set
 *                 unsigned char  len -- the length of the Onu Device location 
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
int  SetOnuLocation_1( short int PonPortIdx, short int OnuIdx, char *Location, int len )
{
	int onuEntry;
	CHECK_ONU_RANGE

	if( Location == NULL ) return( RERROR );
	if( len > MAXLOCATIONLEN ) {
		/*sys_console_printf(" Olt Device Location is to long %d ( SetOnuLocation() ) \r\n", len );*/
		return RERROR; 
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.Location, '\0', MAXLOCATIONLEN );
	if(( len > 1 ) ||(( len == 1) && ( *Location != 0 )))
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.Location, Location, len );
	OnuMgmtTable[onuEntry].DeviceInfo.LocationLen = len;
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  GetOnuLocation( short int PonPortIdx, short int OnuIdx, char *Location, int *len )
{
	int onuEntry;
	CHECK_ONU_RANGE
	
	if(( Location == NULL ) || ( len == NULL )) {
		VOS_ASSERT(0);
		return( RERROR );
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*len = OnuMgmtTable[onuEntry].DeviceInfo.LocationLen;
	if( *len != 0) 
		VOS_MemCpy( Location, OnuMgmtTable[onuEntry].DeviceInfo.Location, *len);
	ONU_MGMT_SEM_GIVE;
	Location[*len] = '\0';
    
	return( ROK );
}

/**************************************************************
 *
 *    Function:  SetOnuMacAddr( short int PonPortIdx, short int OnuIdx, char *MacAddr )
 *
 *    Param:  short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                char *MacAddr -- the Pon channel  Mac Address to be set , the 
 *                                         address is used to communication with PON chip
 *                int len -- the len of the mac add
 *
 *    Desc:  set the onu Device Info
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/

int ThisIsInvalidMacAddr( unsigned char *MacAddr )
{
	/*if( CompTwoMacAddress( MacAddr, Invalid_Mac_Addr1) == ROK) return( TRUE );
	if( CompTwoMacAddress( MacAddr, Invalid_Mac_Addr) == ROK) return( TRUE );
	return( FALSE );*/
	return (MAC_ADDR_IS_ZERO(MacAddr) || MAC_ADDR_IS_BROADCAST(MacAddr));
}

int  SetOnuMacAddr( short int PonPortIdx, short int OnuIdx, char *MacAddr, int len )
{
	int ret, onuEntry;

	CHECK_ONU_RANGE
	if( len > BYTES_IN_MAC_ADDRESS ) {
		sys_console_printf(" %% onu MAC is to long %d(SetOnuMacAddr)\r\n", len);
		len = BYTES_IN_MAC_ADDRESS;
	}
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	if( ThisIsInvalidMacAddr( MacAddr ) == TRUE )
	{
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, MacAddr, len );
		ONU_MGMT_SEM_GIVE;
		ret = ROK;
	}
	else
	{
		ret = GetOnuIdxByMacPerPon( PonPortIdx, MacAddr );
		if( ret == RERROR )
		{
			ONU_MGMT_SEM_TAKE;
			VOS_MemSet( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 0, len );
			VOS_MemCpy( OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, MacAddr, len );
			ONU_MGMT_SEM_GIVE;
			ret = ROK;
		}
		else
		{
			if( ret == onuEntry ) ret = ROK;
			else ret = RERROR;
		}
	}	
	return ret;
}

int  GetOnuMacAddr( short int PonPortIdx, short int OnuIdx, char *MacAddr, int *len )
{
	CHECK_ONU_RANGE
	
	if( (MacAddr == NULL )||( len == NULL)){
		VOS_ASSERT(0);
		return( RERROR );
	}
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy( MacAddr, OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS );
	ONU_MGMT_SEM_GIVE;
	*len = BYTES_IN_MAC_ADDRESS;
	return( ROK );
}

int  GetOnuMacAddrDefault( char *MacAddr, int *len )
{
	if( (MacAddr == NULL )||( len == NULL)){
		/*sys_console_printf("error: the address pointer is null(GetOnuMacAddrDefault)\r\n");*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_MemCpy( MacAddr, OnuConfigDefault.MacAddr, BYTES_IN_MAC_ADDRESS );
	*len = BYTES_IN_MAC_ADDRESS;
	return( ROK );
}

/**************************************************************
 *
 *    Function:  RecordOnuLaunchTime ( short int PonPortIdx, short int OnuIdx, Date_S LaunchDate )
 *
 *    Param:  short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                Date_S LaunchDate -- the Olt Device First LaunchTime ,
 *                                          indicated as year/month/day/hour/minute/second
 *
 *    Desc:  set the onu Device first Launch Time
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  RecordOnuLaunchTime ( short int PonPortIdx, short int OnuIdx )
{
	 Date_S LaunchDate;

	CHECK_ONU_RANGE;
	 /* get the current Date from real Timer  */
	 GetSysCurrentTime( &LaunchDate );
	 
	/* modified by chenfj 2006/10/20
	#2939问题单设置不准确的时间后，启动时信息表述不恰当
	#2911问题单设置系统时间为2000-2-2后,重启系统,提示错误
	*/
#if 0
	if(( LaunchDate.year >2100 ) ||(LaunchDate.year < 2005 )){
		sys_console_printf(" the YEAR %d of the DATE is not right( RecordOnuLaunchTime() )  \r\n", LaunchDate.year );
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}
	if(( LaunchDate.month > 12 ) || (LaunchDate.month == 0)){
		sys_console_printf(" the MONTH %d of the DATE is not right( RecordOnuLaunchTime() ) \r\n", LaunchDate.month );
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}
	if(( LaunchDate.day > 31 ) ||(LaunchDate.day ==0 )){
		sys_console_printf(" the DAY %d of the DATE is not right ( RecordOnuLaunchTime() )\r\n", LaunchDate.day );
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}
	if(( LaunchDate.hour > 24 ) /*||(LaunchDate.hour ==0 )*/){
		sys_console_printf(" the HOUR %d of the DATE is not right ( RecordOnuLaunchTime() )\r\n", LaunchDate.hour);
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}
	if( LaunchDate.minute > 59 ){
		sys_console_printf(" the MINUTE %d of the DATE is not right( RecordOnuLaunchTime() ) \r\n", LaunchDate.minute);
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}
	if(LaunchDate.second > 59 ){
		sys_console_printf(" the SECOND  %d of the DATE is not right( RecordOnuLaunchTime() ) \r\n", LaunchDate.second );
		VOS_SemGive(OnuMgmtTableDataSemId);
		return ( RERROR );
		}	
#endif 
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy ((char *) &(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.SysLaunchTime.year), (char *)&(LaunchDate.year), sizeof( Date_S) );
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

/**************************************************************
 *
 *    Function: GetOnuSysLaunchTime ( short int PonPortIdx, short int OnuIdx, Date_S *LaunchDate )
 *
 *    Param    short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                out : Date_S *LaunchDate -- the pointer of Olt Device First LaunchTime ,
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
int  GetOnuLaunchTime ( short int PonPortIdx, short int OnuIdx, Date_S *LaunchDate )
{
	CHECK_ONU_RANGE;
	
	if( LaunchDate == NULL ){
		VOS_ASSERT(0);
		return( RERROR );
	}
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy( LaunchDate, &( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.SysLaunchTime ), sizeof(Date_S ) );
	ONU_MGMT_SEM_GIVE;
	LaunchDate->MilliSecond = 0;
	return ( ROK );
}

/**************************************************************
 *
 *    Function:  RecordOnuUpTime( short int PonPortIdx, short int OnuIdx )
 *
 *    Param : short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
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

int  GetOnuUpTime( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	CHECK_ONU_RANGE;
	
	ONU_MGMT_SEM_TAKE;
	ret = (int)(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.SysUptime );
	ONU_MGMT_SEM_GIVE;
	return ret;
}

int  GetOnuUpLastTime( short int PonPortIdx, short int OnuIdx )
{
	/* modified by xieshl 20110512, 记录运行时间的单位改为秒，防止溢出，便于检查ONU实际离线时间 */
	/*int tick;
	unsigned long  CurTick;
	CHECK_ONU_RANGE
	CurTick = VOS_GetTick();
	tick = GetOnuUpTime(PonPortIdx, OnuIdx );
	if( tick != RERROR ) return (int )( CurTick - tick );
	return( RERROR );*/

	unsigned long  ti = GetOnuUpTime(PonPortIdx, OnuIdx );
	if( ti != RERROR ) return (int )( time(0) - ti );
	return( RERROR );
}

/**************************************************************
 *
 *    Function: SetOnuAdminStatus( short int PonPortIdx, short int OnuIdx, unsigned char adminStatus )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char adminStatus -- the administration status to be set
 *    Desc:  set the ONU administration status
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuAdminStatus( short int PonPortIdx, short int OnuIdx, unsigned char adminStatus )
{
	int ret = RERROR;
	int onuEntry;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON +OnuIdx;
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
	{
		if( adminStatus == ONU_ADMIN_STATUS_UP )
		{			
			ret = ActivateOnu( PonPortIdx, OnuIdx );
		}
		else if( adminStatus == ONU_ADMIN_STATUS_DOWN )
		{			
			ret = DeactivateOnu(PonPortIdx, OnuIdx);
		}
	}
	else/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].OperStatus == ONU_OPER_STATUS_DOWN )*/
	{
		if( (adminStatus == ONU_ADMIN_STATUS_UP) || (adminStatus == ONU_ADMIN_STATUS_DOWN) ) 
		{
			ret = ROK;
		}
	}
	if( ret == ROK )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuEntry].AdminStatus = adminStatus;
		ONU_MGMT_SEM_GIVE;
	}
	return ret;
}


int  GetOnuAdminStatus( short int PonPortIdx, short int OnuIdx )
{
	int status;
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	status = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].AdminStatus;
	ONU_MGMT_SEM_GIVE;
	return status;
}

/* added by xieshl 20101220, 问题单11740 */
int  resetOnuOperStatusAndUpTime( ULONG OnuEntry, int OperStatus )
{
	/*ULONG tick;*/
	Date_S LaunchDate;

	if( OnuEntry >= MAXONU )
		return RERROR;
	if( (OperStatus <= ONU_OPER_STATUS_MIN) || (OperStatus >= ONU_OPER_STATUS_MAX) )
		/*return VOS_ERROR;*/
		OperStatus = ONU_OPER_STATUS_DOWN;	/* modified by xieshl 20110830, 主控和PON板间同步ONU状态时，如果ONU状态初始化滞后会导致时间错误 */

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].OperStatus = OperStatus;
	/*tick = VOS_GetTick();
	OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime = tick;
	OnuMgmtTable[OnuEntry].DeviceInfo.RelativeUptime = tick - OLTMgmt.DeviceInfo.SysUptime;*/
	OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime = time(0);
	OnuMgmtTable[OnuEntry].DeviceInfo.RelativeUptime = OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime - OLTMgmt.DeviceInfo.SysUptime;
	if( GetSysCurrentTime(&LaunchDate) != ROK )
	{
        ONU_MGMT_SEM_GIVE;
        return VOS_ERROR;
    }
    if(ONU_OPER_STATUS_DOWN == OperStatus)
    {
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.year = LaunchDate.year;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.month = LaunchDate.month;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.day = LaunchDate.day;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.hour = LaunchDate.hour;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.minute = LaunchDate.minute;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.second = LaunchDate.second;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysOfflineTime.MilliSecond = LaunchDate.MilliSecond;
    }
    else
	{
		/*VOS_MemCpy ((char *) &(OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year), (char *)&(LaunchDate.year), sizeof( Date_S) );*/
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year = LaunchDate.year;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.month = LaunchDate.month;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.day = LaunchDate.day;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.hour = LaunchDate.hour;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.minute = LaunchDate.minute;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.second = LaunchDate.second;
		OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.MilliSecond = LaunchDate.MilliSecond;
	}
	ONU_MGMT_SEM_GIVE;

	return ( ROK );
}

int  updateOnuOperStatusAndUpTime( ULONG OnuEntry, int OperStatus )
{
	LONG curStatus;

	if( OnuEntry >= MAXONU )
		return RERROR;

	ONU_MGMT_SEM_TAKE;
	curStatus = OnuMgmtTable[OnuEntry].OperStatus;
	ONU_MGMT_SEM_GIVE;

	/* modified by xieshl 20110629, 问题单13089 */
	if( (curStatus != OperStatus) && (/*(OperStatus == ONU_OPER_STATUS_UP) ||*/
		(OperStatus == ONU_OPER_STATUS_UP) || (OperStatus == ONU_OPER_STATUS_DOWN)/*(curStatus == ONU_OPER_STATUS_PENDING) || (curStatus == ONU_OPER_STATUS_DORMANT)*/ ))
	{
		resetOnuOperStatusAndUpTime( OnuEntry, OperStatus );
	}

	return ( ROK );
}

/* modified by xieshl 20110512, 防止时间溢出, 便于ONU离线老化 */
int  RecordOnuUpTime( ULONG onuEntry )
{
	/*unsigned long tick;
	Date_S LaunchDate;
		
	if( OnuEntry >= MAXONU )
		return RERROR;

	tick = VOS_GetTick();
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime = tick;
	OnuMgmtTable[OnuEntry].DeviceInfo.RelativeUptime = tick - OLTMgmt.DeviceInfo.SysUptime;

	if( GetSysCurrentTime(&LaunchDate) == ROK )
		VOS_MemCpy ((char *) &(OnuMgmtTable[OnuEntry].DeviceInfo.SysLaunchTime.year), (char *)&(LaunchDate.year), sizeof( Date_S) );
	ONU_MGMT_SEM_GIVE;

	return( ROK );*/
	ULONG ti;
	Date_S LaunchDate;

	if( onuEntry >= MAXONU )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	ti = time(0);
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].DeviceInfo.SysUptime = ti;
	OnuMgmtTable[onuEntry].DeviceInfo.RelativeUptime = ti - OLTMgmt.DeviceInfo.SysUptime;

	 /* get the current Date from real Timer  */
	if( GetSysCurrentTime( &LaunchDate ) == ROK )
		VOS_MemCpy ((char *) &(OnuMgmtTable[onuEntry].DeviceInfo.SysLaunchTime.year), (char *)&(LaunchDate.year), sizeof( Date_S) );
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}


/**************************************************************
 *
 *    Function: SetOnuOperStatus( short int PonPortIdx, short int OnuIdx, int OperStatus )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                int OperStatus -- the operating status to be record 
 *    Desc:  set the ONU operation status
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuOperStatus( short int PonPortIdx, short int OnuIdx, int OperStatus )
{
	CHECK_ONU_RANGE;
/*#if 0	
	if( VOS_SemTake(OnuMgmtTableDataSemId, WAIT_FOREVER) == VOS_ERROR ) {
		sys_console_printf( "get SemId error ( SetOnuOperStatus() )\r\n" );
		return( RERROR );
		}
#endif
	if((OperStatus != ONU_OPER_STATUS_UP )
        && (OperStatus != ONU_OPER_STATUS_DOWN ) 
		&&(OperStatus != ONU_OPER_STATUS_PENDING )
		&&(OperStatus != ONU_OPER_STATUS_UNKNOWN )
		&&(OperStatus != ONU_OPER_STATUS_POWERDOWN )
		&&(OperStatus != ONU_OPER_STATUS_DORMANT ))
		return( RERROR );
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].OperStatus = OperStatus;
#if 0	
	ONU_MGMT_SEM_GIVE;
#endif
	return ( ROK );*/
	return updateOnuOperStatusAndUpTime(PonPortIdx * MAXONUPERPON + OnuIdx, OperStatus );
}

int GetOnuOperStatusByMacAddr( short int PonPortIdx, short int *OnuIdx_ret, unsigned char *MacAddr )
{
	short int OnuIdx, entry;
	int rc = RERROR;

	CHECK_PON_RANGE;

	if( OnuIdx_ret == NULL ) return( RERROR );
	*OnuIdx_ret = 0;

	entry = PonPortIdx * MAXONUPERPON;
	ONU_MGMT_SEM_TAKE;
	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
	{
		if( ThisIsValidOnu(PonPortIdx,OnuIdx) == ROK )
		{
			if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[entry + OnuIdx].DeviceInfo.MacAddr, MacAddr ) )
			{
				*OnuIdx_ret = OnuIdx;
				rc = GetOnuOperStatus(PonPortIdx, OnuIdx);
				break;
			}
		}
	}
	ONU_MGMT_SEM_GIVE;

	return rc;
}
#define ONU_ALARM_STATUS            0x00000001
#define ONU_UPDATE_ALARM_STATUS     0x00000002
ULONG GetOnuAlarmStatus(short int PonPortIdx, short int OnuIdx )
{
    ULONG status = 0;
 	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
    status = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus&ONU_ALARM_STATUS;
	ONU_MGMT_SEM_GIVE;
	return status;
}
int SetOnuAlarmStatus(short int PonPortIdx, short int OnuIdx ,ULONG status)
{
 	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	if(status)
        OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus |= ONU_ALARM_STATUS;
    else
        OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus &= ~ONU_ALARM_STATUS;
	ONU_MGMT_SEM_GIVE;
	return VOS_OK;
}

ULONG GetOnuUpdatedAlarmStatus(ULONG deviceIdx, ULONG *status)
{
    short int PonPortIdx = GetPonPortIdxBySlot(GET_PONSLOT(deviceIdx), GET_PONPORT(deviceIdx));
    short int OnuIdx = GET_ONUID(deviceIdx)-1;  
    
 	CHECK_ONU_RANGE;
 	
	ONU_MGMT_SEM_TAKE;
    *status = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus&ONU_UPDATE_ALARM_STATUS;
	ONU_MGMT_SEM_GIVE;
	return VOS_OK;
}
int SetOnuUpdatedAlarmStatus(ULONG deviceIdx,ULONG status)
{
    short int PonPortIdx = GetPonPortIdxBySlot(GET_PONSLOT(deviceIdx), GET_PONPORT(deviceIdx));
    short int OnuIdx = GET_ONUID(deviceIdx)-1;  

 	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	if(status)
        OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus |= ONU_UPDATE_ALARM_STATUS;
    else
        OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ctcAlarmstatus &= ~ONU_UPDATE_ALARM_STATUS;
	ONU_MGMT_SEM_GIVE;
	return VOS_OK;
}
int  GetOnuOperStatus( short int PonPortIdx, short int OnuIdx )
{	
	/*short int llid;
	short int PonChipType;*/
	int status = ONU_OPER_STATUS_DOWN;
	
	CHECK_ONU_RANGE;

	/*return ( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].OperStatus );*/
	/*modified by chenfj 2008-6-3 
		问题单6707: 问题1 
		对ONU 的当前状态, 要先判断PAS-SOFT 中的状态;只有在PAS-SOFT中处于在线的ONU,才会再在ONU管理表中查询其状态
	*/
	/* 模拟6900注册时屏蔽掉的*//*add by shixh20100528*/
#if 0  
    /* B--modified by liwei056@2011-1-26 for CodeCheck */
#if 0
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(llid == INVALID_LLID) return( ONU_OPER_STATUS_DOWN);
#else
    if ( 0 != OnuMgtAdv_IsOnline(PonPortIdx, OnuIdx) ) return( ONU_OPER_STATUS_DOWN );
#endif
    /* E--modified by liwei056@2011-1-26 for CodeCheck */
#endif

	ONU_MGMT_SEM_TAKE;
	if( ONU_OPER_STATUS_UP == OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].OperStatus )
	{
		status = ONU_OPER_STATUS_UP;
	}
	ONU_MGMT_SEM_GIVE;
	return status;
}

int   GetOnuOperStatus_Ext(short int PonPortIdx, short int OnuIdx )
{
	int status;

	status = GetOnuOperStatus_1(PonPortIdx, OnuIdx );
	/*问题单: 6548
		813在线，显示的状态是powerDown
		经测试发现, 快速开关电源, ONU只上报了调电, 但并未重新启动, 也就没有重新注册
		增加一个容错功能: ONU 瞬间调电, 但并未重新注册
		 */
	if( status != ONU_OPER_STATUS_UP )
	{
		ONU_MGMT_SEM_TAKE;
		if( OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].PowerOn == V2R1_POWER_OFF )
			status = ONU_OPER_STATUS_POWERDOWN;
		ONU_MGMT_SEM_GIVE;
	}
	
	/* 当前休眠状态还没加上*/
	
	return(status );
}

int  GetOnuOperStatus_1( short int PonPortIdx, short int OnuIdx )
{
	int status;
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	status = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].OperStatus;
	ONU_MGMT_SEM_GIVE;

	return status;
}

/* added by xieshl 20110523 */
#if 0
int  SetOnuPowerOnStatus( short int PonPortIdx, short int OnuIdx )
{
	int OnuMgtIdx;

	CHECK_ONU_RANGE;

	OnuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuMgtIdx].PowerOn = V2R1_POWER_ON;
	OnuMgmtTable[OnuMgtIdx].PowerOffCounter = 0;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

int  SetOnuPowerOffStatus( short int PonPortIdx, short int OnuIdx )
{
	int rc = RERROR;
	int OnuMgtIdx;

	CHECK_ONU_RANGE;

	OnuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	if( OnuMgmtTable[OnuMgtIdx].PowerOn !=  V2R1_POWER_OFF ) 
	{
		OnuMgmtTable[OnuMgtIdx].PowerOn = V2R1_POWER_OFF;
		OnuMgmtTable[OnuMgtIdx].PowerOffCounter = 0;
		rc = ROK;
	}
	ONU_MGMT_SEM_GIVE;
	
	return rc;
}
#else
/* modified by xieshl 20110615, 问题单13029 */
int  SetOnuPowerStatus( short int PonPortIdx, short int OnuIdx, LONG status )
{
	int OnuMgtIdx;

	CHECK_ONU_RANGE;

	OnuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuMgtIdx].PowerOn = status;
	OnuMgmtTable[OnuMgtIdx].PowerOffCounter = 0;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}
int  SetOnuDevicePowerStatus( ULONG onuDevIdx, LONG status )
{
	/*int OnuMgtIdx;*/
	short int PonPortIdx, OnuIdx;
	short int slot, port;

	slot = GET_PONSLOT(onuDevIdx);
	port = GET_PONPORT(onuDevIdx);
	if( SYS_SLOTNO_IS_ILLEGAL(slot) )
		return RERROR;
	
	PonPortIdx = GetPonPortIdxBySlot(slot, port );
	OnuIdx = GET_ONUID(onuDevIdx) -1;

	return SetOnuPowerStatus( PonPortIdx, OnuIdx, status );
}
#endif

int GetOnuCurStatus( int slot , int port , int OnuId )
{
	short int PonPortIdx;
	short int OnuIdx;
	
	if(SlotCardIsPonBoard(slot) != ROK ) return(RERROR);
	
	if(( port <= 0) ||( port > PONPORTPERCARD )) return( RERROR );
	if((OnuId < 0 )||(OnuId > MAXONUPERPON)) return( RERROR );

	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	if( PonPortIdx == RERROR ) return( RERROR );
	
	OnuIdx = (short int)(OnuId - 1);

	return( GetOnuOperStatus(PonPortIdx, OnuIdx) );
}

int GetOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx)
{
	int rc;
	CHECK_ONU_RANGE;
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	rc = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ExtOAM;
	ONU_MGMT_SEM_GIVE;
	return rc;
}

int SetOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx)
{
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ExtOAM = V2R1_ENABLE;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

int ClearOnuExtOAMStatus( short int PonPortIdx, short int OnuIdx)
{
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].ExtOAM = (V2R1_DISABLE-V2R1_DISABLE);
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

int  GetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned long *pMask )
{
	CHECK_ONU_RANGE
	if( pMask == NULL )
		return RERROR;
	ONU_MGMT_SEM_TAKE;
	*pMask = (OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].devAlarmMask & EVENT_MASK_DEV_ALL);
	ONU_MGMT_SEM_GIVE;
	return ( ROK );
}
int  SetOnuAlarmMask( short int PonPortIdx, short int OnuIdx, unsigned long mask )
{
	int onuEntry;
	/*ULONG onuDevIdx;*/
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].devAlarmMask = (mask & EVENT_MASK_DEV_ALL);
	ONU_MGMT_SEM_GIVE;

	/*onuDevIdx = parseDevidxFromOnuEntry(onuEntry);
	if( onuDevIdx )
		almStatusOnuDevAlmMaskCallback( onuDevIdx, mask );	*/	/* added by xieshl 20110407 */
	
	return ( ROK );
}

/**************************************************************
 *
 *    Function: SetOnuSWUpdateEnable( short int PonPortIdx, short int OnuIdx, unsigned char EnableFlag )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char EnableFlag -- enable/disable onu software update 
 *    Desc:  set the ONU software update  control flag
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuSWUpdateCtrl(short int PonPortIdx, short int OnuIdx, unsigned char EnableFlag)
{
	CHECK_ONU_RANGE
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	if(( EnableFlag == ONU_SW_UPDATE_ENABLE ) || ( EnableFlag == ONU_SW_UPDATE_DISABLE))
	{
        /* B--modified by liwei056@2011-2-12 for D11920 */
#if 1
        return (OLT_CALL_ISOK(OnuMgt_SetOnuSWUpdateMode(PonPortIdx, OnuIdx, EnableFlag)) ? ROK : RERROR);
#else
	    OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].SoftwareUpdateCtrl = EnableFlag;
#endif
        /* E--modified by liwei056@2011-2-12 for D11920 */
	}
	else return(RERROR);
	/* modified by chenfj 2007/02/07 
	问题单# 3568  GW-EPON-DEV-MIB中自动升级状态的属性修改影响了升级状态属性
	
	if((EnableFlag == ONU_SW_UPDATE_ENABLE ) &&( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx) == ONU_SW_UPDATE_STATUS_FORBIDDEN ))
		SetOnuSwUpdateStatus( PonPortIdx, OnuIdx, ONU_SW_UPDATE_STATUS_NOOP );
	if(( EnableFlag == ONU_SW_UPDATE_DISABLE ) && ( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx) == ONU_SW_UPDATE_STATUS_NOOP))
		SetOnuSwUpdateStatus( PonPortIdx, OnuIdx, ONU_SW_UPDATE_STATUS_FORBIDDEN );
	*/
	return ( ROK );
}

int  GetOnuSWUpdateCtrl( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	CHECK_ONU_RANGE;
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

	ONU_MGMT_SEM_TAKE;
	ret = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].SoftwareUpdateCtrl;
	ONU_MGMT_SEM_GIVE;
	return ret;
}

int  GetOnuSWUpdateCtrlDefault()
{
	if(( OnuConfigDefault.SoftwareUpdateCtrl == V2R1_ENABLE ) ||( OnuConfigDefault.SoftwareUpdateCtrl == V2R1_DISABLE ))
		return( OnuConfigDefault.SoftwareUpdateCtrl );
	return OnuSoftwareUpdateCtrl;
}

/* B--added by liwei056@2011-2-12 for D11920 */
int CopyOnuSWUpdateCfg(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int runSrcSoftUpdata;

    runSrcSoftUpdata = GetOnuSWUpdateCtrl(SrcPonPortIdx, SrcOnuIdx);
    if( VOS_ERROR != runSrcSoftUpdata )
    {
        if ( OLT_COPYFLAGS_COVER & CopyFlags )
        {
            iRlt = OnuMgt_SetOnuSWUpdateMode(DstPonPortIdx, DstOnuIdx, runSrcSoftUpdata);
        }
        else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
        {
            int defSoftUpdata = GetOnuSWUpdateCtrlDefault();

            if ( (runSrcSoftUpdata != defSoftUpdata) && (VOS_ERROR != defSoftUpdata) )
            {
                iRlt = OnuMgt_SetOnuSWUpdateMode(DstPonPortIdx, DstOnuIdx, runSrcSoftUpdata);
            }
        }
        else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
        {
            if ( OLT_ISLOCAL(DstPonPortIdx) ) 
            {
                int runDstSoftUpdata = GetOnuSWUpdateCtrl(DstPonPortIdx, DstOnuIdx);

                if ( (runSrcSoftUpdata != runDstSoftUpdata) && (VOS_ERROR != runDstSoftUpdata) )
                {
                    iRlt = OnuMgt_SetOnuSWUpdateMode(DstPonPortIdx, DstOnuIdx, runSrcSoftUpdata);
                }
            }
            else
            {
                iRlt = OnuMgt_SetOnuSWUpdateMode(DstPonPortIdx, DstOnuIdx, runSrcSoftUpdata);
            }
        }
    }

    return iRlt;
}
/* E--added by liwei056@2011-2-12 for D11920 */

/**************************************************************
 *
 *    Function:  GetOnuSWUpdateStatus( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx -- the specific Onu
 *              
 *    Desc:  
 *
 *    Return:   onu software update status
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
 int  GetOnuSWUpdateStatus( short int PonPortIdx, short int OnuIdx )
{
	int UpdateStatus;
	int UpdateCtrl;
	int onuEntry;
	
	CHECK_ONU_RANGE;

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	UpdateCtrl = OnuMgmtTable[onuEntry].SoftwareUpdateCtrl;	/*GetOnuSWUpdateCtrl( PonPortIdx, OnuIdx )*/
	UpdateStatus = OnuMgmtTable[onuEntry].SoftwareUpdateStatus;
	ONU_MGMT_SEM_GIVE;

	/* modified by chenfj 2007/02/07 
	问题单# 3568  GW-EPON-DEV-MIB中自动升级状态的属性修改影响了升级状态属性
	*/
	if( ONU_SW_UPDATE_DISABLE == UpdateCtrl )
	{
		if( UpdateStatus ==  ONU_SW_UPDATE_STATUS_NOOP )
			UpdateStatus = ONU_SW_UPDATE_STATUS_FORBIDDEN;
	}
	return UpdateStatus;
}

int SetOnuSwUpdateStatus( short int PonPortIdx, short int OnuIdx, int status, unsigned int fileType)
{
	int onuEntry;
	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	if((status != ONU_SW_UPDATE_STATUS_NOOP ) &&(status != ONU_SW_UPDATE_STATUS_INPROGRESS) &&(status != ONU_SW_UPDATE_STATUS_FORBIDDEN )) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].SoftwareUpdateStatus = status;
	if( status == ONU_SW_UPDATE_STATUS_NOOP )
		OnuMgmtTable[onuEntry].SoftwareUpdateType = IMAGE_TYPE_NONE;
	else if( status == ONU_SW_UPDATE_STATUS_INPROGRESS )
		OnuMgmtTable[onuEntry].SoftwareUpdateType = fileType;
	ONU_MGMT_SEM_GIVE;

	onuOamUpdStatusReport( PonPortIdx, OnuIdx, status, fileType);

	return( ROK );
}
int GetGponOnuSwUpdateStatus( short int PonPortIdx, short int OnuIdx, int *status)
{
	int onuEntry;
	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	*status = OnuMgmtTable[onuEntry].SoftwareUpdateStatus;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}
int SetGponOnuSwUpdateStatus( short int PonPortIdx, short int OnuIdx, int status)
{
	int onuEntry;
	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	if((status != ONU_SW_UPDATE_STATUS_NOOP ) &&(status != ONU_SW_UPDATE_STATUS_INPROGRESS)) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].SoftwareUpdateStatus = status;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}
int GetOnuSwUpdateType( short int PonPortIdx, short int OnuIdx)
{
	int ret;
	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	ret = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].SoftwareUpdateType;
	ONU_MGMT_SEM_GIVE;
	return ret;
}

/*
1、在GW模式下升级GT810/GT816/GT811等型号的GW模式文件，使用老命令update

2、在CTC模式下升级GT810/GT816/GT811/821四个型号的CTC模式文件，使用新命令
        update，在flash里增加CTC_4FE与CTC_4FE_2VOIP型号文件，此文件是CTC的文件，
        对于CTConu,使用此命令升级时，需要察看端口数，如果Fe数量小于等于4，
        VOIP数量等于0，E1接口数量等于0，则升级CTC_4FE文件，如果Fe数量小于等
        于4，VOIP数量等于2，E1接口数量等于0，则升级CTC_4FE_2VOIP文件；否则不
        升级文件（后续开发出新CTC产品时可以再扩充此部分）。
3、在GW模式下升级GT810/GT816两个型号的CTC模式文件，或者在CTC模式下升
        级GT810/GT816两个型号的GW模式文件，使用命令update必须增加一个参数 
        ，参数为枚举，CTC、GW，解释如下：

         GW――>CTC：参数只能为CTC，如果GW模式下ONU型号为GT810/GT816，则升级
         CTC_4FE文件，其余型号均不升级（后续开发出新产品可以再扩充此部分）。

         CTC――>GW：参数只能为GW，如果CTC模式下ONU的FE端口等于1，VOIP与E1端
         口均为0，则升级GT816/810文件（GT816与GT810目前是同一个文件），否则
         不升级（后续开发出新产品可以再扩充此部分）。
4、下一步考虑如何实现对于CTConu的型号划分（暂时根据端口型号数量进
       行划分）。

*/

/* added by chenfj 2007-8-14 
	通过PMC提供的API, ONU文件升级*/


int GetOnuBurnFlashFlag(short int PonPortIdx, short int OnuIdx, int *flag )
{
	CHECK_ONU_RANGE

	if( flag == NULL ) return( RERROR );
	ONU_MGMT_SEM_TAKE;
	*flag = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].BurningFlash ;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

int get_onu_image_update_mode( struct vty *vty, short int phyPonId, short int userOnuId )
{
	int OnuType = 0;
	int update_mode = IMAGE_UPDATE_MODE_UNKNOWN;

	ULONG slotno = GetCardIdxByPonChip(phyPonId);
	ULONG portno = GetPonPortByPonChip(phyPonId);
	ULONG onuno = userOnuId+1;
	
	if( GetOnuOperStatus( phyPonId, userOnuId) == ONU_OPER_STATUS_DOWN )
	{
		if( vty ) vty_out( vty, "  %% onu%d/%d/%d is off-line.\r\n", slotno, portno, onuno ) ;
		return update_mode;
	}	
	if( GetOnuType(phyPonId, userOnuId, &OnuType ) != ROK )
	{
		if( vty ) vty_out(vty, "  %% Get onu%d/%d/%d type err\r\n", slotno, portno, onuno );
		return update_mode;
	}

	if( OnuType == V2R1_ONU_CTC )
	{
		update_mode = IMAGE_UPDATE_MODE_CTC;
		if( vty ) vty_out(vty, "  %% onu %d/%d/%d CTC->%s\r\n", slotno, portno, onuno, PRODUCT_CORPORATION_AB_NAME) ;
	}
	else
	{
		update_mode=  IMAGE_UPDATE_MODE_GW;
		if( vty ) vty_out(vty, "  %% onu %d/%d/%d %s->CTC\r\n", slotno, portno, onuno, PRODUCT_CORPORATION_AB_NAME) ;
	}
	return update_mode;
}

/* 用于CTC ONU程序升级*/
int  StartOnuSWUpdate_1( short int PonPortIdx, short int OnuIdx)
{
	/*short int llid;*/
	/*int OnuType;*/
	int slotId, port;
	
	/*unsigned char  onuTypeString[ONU_TYPE_LEN+2] ={'\0'};
	int length ;
	int file_len;*/
		
	int ret;
	/*long retval;*/
	struct vty *vty;

	/*int FE_num=0, POTS_num=0, E1_num=0;*/

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK )
		return( RERROR );
	slotId = GetCardIdxByPonChip(PonPortIdx);
	port   = GetPonPortByPonChip(PonPortIdx);
	
	/*!!应该对ONU的VTY进行判断，当其无效时应置为NULL*/
	ONU_MGMT_SEM_TAKE;
	vty = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].vty ;
	if( cl_vty_valid( vty ) == VOS_ERROR )
	{
		vty = NULL;
	}
	ONU_MGMT_SEM_GIVE;
	
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		v2r1_printf( vty, "  onu %d/%d/%d is off-line \r\n", slotId, port, (OnuIdx+1) );
		return( ONU_UPDATE_ERR );
	}

	if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_DISABLE )
	{
		v2r1_printf(vty, "  onu %d/%d/%d software update is disabled,so can't update this onu\r\n",  slotId, port, (OnuIdx+1) );
		return (ONU_UPDATE_ERR) ;
	}

	ret = OnuMgt_OnuSwUpdate(PonPortIdx,OnuIdx, 0, 0);
    if ( OLT_CALL_ISOK(ret) )
    {
       	v2r1_printf(vty, "onu-app upgrading....... \r\n");
    }
    else
    {
        switch ( ret = OLT_ERR_UPGRADE_BEGIN - ret )
        {
            case ONU_UPDATE_INPROCESS:
        		v2r1_printf( vty, "\r\n  onu %d/%d/%d image is updating\r\n",  slotId, port, (OnuIdx+1) );
                break;
            case ONU_FILE_NOEXIST:
    			v2r1_printf(vty, "\r\n  there is no %s image for onu%d/%d/%d in flash\r\n", V2R1_CTC_ONU, slotId, port, (OnuIdx+1) );
                break;
            case ONU_VERSION_IDENTICAL:
                v2r1_printf(vty, "\r\n  onu %d/%d/%d %s Image is the newest version now\r\n", slotId, port, (OnuIdx+1), V2R1_CTC_ONU );
                break;
            default:
        		v2r1_printf(vty, "\r\n  onu %d/%d/%d is failed to update\r\n",  slotId, port, (OnuIdx+1) );
        }
    }

    return ret;
}

/* 这个函数用于ONU程序在GW模式和CTC模式之间切换*/

extern char convert_onu_file_type[ONU_TYPE_LEN +4];
int  StartOnuSWUpdate_2( short int PonPortIdx, short int OnuIdx, int UpdateFileType)
{
	/*short int llid;
	int OnuType;*/
	int slotId, port;
	
	/*unsigned char  onuTypeString[ONU_TYPE_LEN+2] ={'\0'};
	int length ;*/
	/*int file_len = 0, file_len1=0;*/
		
	int ret;
	struct vty *vty;

	/*int update_mode;*/
	/*int FE_num=0, POTS_num=0, E1_num=0;*/
	/*unsigned char  *onuTypeString = convert_onu_file_type;
	int length = VOS_StrLen( onuTypeString );*/

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK )
		return( RERROR );
	
	/*!!应该对ONU的VTY进行判断，当其无效时应置为NULL*/
	ONU_MGMT_SEM_TAKE;
	vty = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].vty ;
	if( cl_vty_valid( vty ) == VOS_ERROR )
	{
		vty = NULL;
	}
	ONU_MGMT_SEM_GIVE;
	
	slotId = GetCardIdxByPonChip(PonPortIdx);
	port   = GetPonPortByPonChip(PonPortIdx);

	if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_DISABLE )
	{
		v2r1_printf(vty, "\r\n  onu %d/%d/%d software update is disabled, so can't update this onu\r\n",  slotId, port, (OnuIdx+1) );
		return (ONU_UPDATE_FORBIDDED) ;
	}

	/*if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
		{
		v2r1_printf( vty, "  onu %d/%d/%d is off-line \r\n", slotId, port, (OnuIdx+1) );
		return( ONU_UPDATE_ERR );
		}*/


	ret = OnuMgt_OnuGwCtcSwConvert(PonPortIdx, OnuIdx, convert_onu_file_type);
    if ( OLT_CALL_ISOK(ret) )
    {
       	v2r1_printf(vty, "onu-app converting....... \r\n");
    }
    else
    {
        switch ( ret = OLT_ERR_UPGRADE_BEGIN - ret )
        {
            case ONU_UPDATE_INPROCESS:
        		v2r1_printf( vty, "\r\n  onu %d/%d/%d image is updating\r\n",  slotId, port, (OnuIdx+1) );
                break;
            case ONU_FILE_NOEXIST:
    			v2r1_printf(vty, "\r\n  there is no %s image for onu%d/%d/%d in flash\r\n", convert_onu_file_type, slotId, port, (OnuIdx+1) );
                break;
            default:
        		v2r1_printf(vty, "\r\n  onu %d/%d/%d is failed to update\r\n",  slotId, port, (OnuIdx+1) );
        }
    }

    return ret;
}

extern ULONG parseDevidxFromPonOnu( const short int ponIdx, const short int onuIdx );
int SendUpdateOnuImageMsg(short int PonPortIdx, short int OnuIdx, unsigned char *onuTypeString, int length )
{
	LONG ret;
	unsigned long aulMsg[4] = { MODULE_ONU, FC_UPDATE_ONU_APP_IMAGE, 0, 0 };

	fileName_t  *DataBuf;
	
	CHECK_ONU_RANGE;
	if( (onuTypeString == NULL) || (length == 0) || (length > ONU_TYPE_LEN) )
	{
		VOS_ASSERT(0);
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE);
		return( RERROR );
	}

	DataBuf = (fileName_t *)VOS_Malloc(sizeof( fileName_t),  MODULE_ONU);
	if(DataBuf == NULL ) 
	{
		ASSERT(0);
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE);
		return( RERROR );
	}

	VOS_MemSet( DataBuf, 0, sizeof( fileName_t ));
	DataBuf->file_len = length ;
	VOS_MemCpy( DataBuf->file_name, onuTypeString, length );
	
	DataBuf->PonPortIdx = PonPortIdx ;
	DataBuf->OnuIdx = OnuIdx;

	aulMsg[3] = (int )DataBuf;

	ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
	
	if( ret !=  VOS_OK )
	{
		ASSERT( 0 );
		VOS_Free( (void *)DataBuf );
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
		return( RERROR );
	}	
		
	return ( ROK );	
}

int update_delay_flag = 5;
int update_func_switch = 0;
/*char *update_ctc_file_name = "PMC.PAS6301.V104.1307";*/
char *update_ctc_file_name = "onuapp.bin";
static int __UpdateOnuFileImage(short int PonPortIdx, short int OnuIdx, unsigned char *onuTypeString, int length)
{
	short int llid;
	short int ret;

	int slotId, port;

	int size = 2048, i=0;	
	app_desc_t *pdesc = NULL;
	UCHAR file[2048];
	unsigned char *FileImage;
	PON_binary_t  OnuImage;
	
	unsigned int  flag = V2R1_DISABLE;

	int file_len = 0;
	struct vty *vty;

	CHECK_ONU_RANGE;

	ONU_MGMT_SEM_TAKE;
	vty = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].vty ;
	if( cl_vty_valid( vty ) == VOS_ERROR )
		vty = NULL;
	ONU_MGMT_SEM_GIVE;

	slotId = GetCardIdxByPonChip(PonPortIdx);
	port   = GetPonPortByPonChip(PonPortIdx);

	if( onuTypeString == NULL ) 
		{
		v2r1_printf(vty, "  get onu %d/%d/%d type err \r\n",  slotId, port, (OnuIdx+1) );
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
		return( RERROR );
		}

	llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID )
		{
		v2r1_printf( vty, "  onu %d/%d/%d is off-line \r\n", slotId, port, (OnuIdx+1) );
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ) , IMAGE_TYPE_NONE );
		return( RERROR );
		}

	VOS_MemSet( &OnuImage, 0, sizeof( PON_binary_t ));
	VOS_MemSet( file, 0, 2048);

	
	{
		xflash_file_read( ONU_FLASH_FILE_ID, file, &size );

		pdesc = (app_desc_t*)file;

		for( i=0; i<(2048/128); i++ )
		{
			if( pdesc->dev_type[0] == 0 )
				break;
			if( VOS_StrCmp( onuTypeString, pdesc->dev_type ) == 0 )
				{
				char *p  = (char*)&(pdesc->file_len);
				char *p1 = (char *)&file_len;		

				p1[0] = p[3];
				p1[1] = p[2];
				p1[2] = p[1];
				p1[3] = p[0];
				flag = V2R1_ENABLE;
				/*sys_console_printf(" the file len is %d--p1=%02x %02x %02x %02x\r\n",file_len, p1[3], p1[2],p1[1],p1[0] );*/
				break;
				}			
			pdesc++;			
		}
	}

	if( flag == V2R1_DISABLE ) 
		{
		v2r1_printf(vty, "  no %s image in flash,update onu %d/%d/%d app image err\r\n", onuTypeString,slotId, port, (OnuIdx+1));
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
		return( RERROR );
		}

	/*  modified by chenfj 2008-3-5
	     问题单6342.[GFA6700]: 在CTC模式下，将ONU从CTC版本升级为GW版本时报错
	     使用VOS_Malloc 申请的空间有限制,最大为128K字节;
	     为适应不同文件大小, 改用malloc 函数
	    */	
	/*FileImage = (char*)VOS_Malloc(MODULE_ONU,(file_len + 4));*/
	FileImage = (char*)g_malloc(file_len + 4);
	if( FileImage == NULL )
	{
		v2r1_printf(vty, "  no memory allocation,update onu %d/%d/%d app image err\r\n", slotId, port, (OnuIdx+1));
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
		return( RERROR );
	}

	if( get_ONU_file( onuTypeString, FileImage,&(file_len), NULL ) != ROK )
	{
		g_free(FileImage);
		sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
		return( RERROR );
	}
	
	SetOnuSwUpdateStatus(PonPortIdx, OnuIdx , ONU_SW_UPDATE_STATUS_INPROGRESS,IMAGE_TYPE_APP );
	v2r1_printf( vty, "  send app image to onu %d/%d/%d Start.....\r\n",slotId, port, (OnuIdx+1)) ;

	{

	    char sz[128] = "";
	    VOS_Sprintf(sz, "send app image to onu %d/%d/%d Start.....\r\n",slotId, port, (OnuIdx+1));
	    onuOamUpdPrintf(slotId, port, OnuIdx+1, sz);
	}

    if( update_func_switch )
    {
    	OnuImage.type = PON_ONU_BINARY_FIRMWARE;
    	OnuImage.source = PON_BINARY_SOURCE_MEMORY;
    	OnuImage.location = FileImage;
    	OnuImage.size = file_len;
    	/*sys_console_printf(" onu file_len=%d, addr=0x%x\r\n", file_len, FileImage);*/

    	V2R1_disable_watchdog();
	
	#if 0/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_UpdateOnuFirmwareR( PonPortIdx, llid, &OnuImage );
	#else
	ret = REMOTE_PASONU_update_onu_firmware( PonPortIdx, llid, &OnuImage ); 	
	#endif

    	/*ret = REMOTE_PASONU_update_onu_firmware_image_index( PonPortIdx, llid, &OnuImage, Image );*/
    	V2R1_enable_watchdog();

    	for( i=0; i<update_delay_flag; i++ )
    	{
    		bool complete = 0;
    		VOS_TaskDelay(200);
		#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		ret = OnuMgt_GetBurnImageComplete(PonPortIdx, OnuIdx, &complete);
		#else
		    ret = REMOTE_PASONU_get_burn_image_complete(PonPortIdx, llid, &complete);
		#endif
		
		if( ret == PAS_EXIT_OK )
    		{
    			/*sys_console_printf("onu burn image complete=%d\r\n", complete );*/
    			if( complete )
    				break;
    		}
    	}
    }
    else
    {
#if 0
    	CTC_STACK_binary_t  f_image;
    	f_image.source = PON_BINARY_SOURCE_MEMORY;
    	f_image.location = FileImage;
    	f_image.size = file_len;
#endif       
    	sys_console_printf("CTC:onu firmware loading begin...\r\n" );
    	V2R1_disable_watchdog();
#if 1
        ret = OnuMgt_UpdateOnuFirmware(PonPortIdx, OnuIdx, FileImage, file_len, update_ctc_file_name);    
#else    
    	ret = CTC_STACK_update_onu_firmware( PonPortIdx, llid, &f_image, update_ctc_file_name ); 	
#endif
    	V2R1_enable_watchdog();
    	sys_console_printf("CTC:onu firmware loading end, ret=%d\r\n", ret );

        {
            char sz[128] = "";
            VOS_Sprintf(sz, "send app image to onu %d/%d/%d done!\r\n",slotId, port, (OnuIdx+1));
            onuOamUpdPrintf(slotId, port, OnuIdx+1, sz);
        }

    	if( ret == 0 )
    	{
    		/* 使用备用存储区的软件( 新下载的)镜像重新启动 */
#if 1
            ret = OnuMgt_ActiveOnuFirmware( PonPortIdx, OnuIdx );
#else    
    		ret = CTC_STACK_activate_image( PonPortIdx, llid );
#endif
    		sys_console_printf("CTC:activate new image: ret=%d\r\n", ret );

    	    /*记录升级成功的ONU，在ONU重启再注册时执行commit操作*/
    	    setCtcOnuCommitFlag(PonPortIdx, OnuIdx);
    	}
    	else
        {

            char sz[128] = "";
            VOS_Sprintf(sz, "send app image to onu %d/%d/%d error!\r\n",slotId, port, (OnuIdx+1));
            onuOamUpdPrintf(slotId, port, OnuIdx+1, sz);
        }

    	if( 0/*ret == PAS_EXIT_OK*/ )	/* 这个过程应放在ONU重新注册之后 */
    	{
    		/* 将当前备用存储区的软件变为主用存储区的软件，作为ONU启动时默认加载执行的软件，
    		    而主用区的软件镜像变为备用区的软件 */
#if 1
            ret = OnuMgt_CommitOnuFirmware( PonPortIdx, OnuIdx );
#else    
    		ret = CTC_STACK_commit_image( PonPortIdx, llid );
#endif
    		sys_console_printf("CTC:commit new image: ret=%d\r\n", ret );
    	}

    }

	g_free(FileImage);
	SetOnuSwUpdateStatus(PonPortIdx, OnuIdx , ONU_SW_UPDATE_STATUS_NOOP,IMAGE_TYPE_APP );

	sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), IMAGE_TYPE_NONE );
	
	if( ret == 0 ) 
	{
		/* 文件正确传送,等待ONU写FLASH结束*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].BurningFlash = V2R1_ENABLE;
		ONU_MGMT_SEM_GIVE;
		/* modified by chenfj 2008-1-31
		    问题单6255.将调试信息Transfer onu。。。。Sucessfully屏蔽掉
		 */
	}
	else
    {
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].BurningFlash = V2R1_DISABLE;
		ONU_MGMT_SEM_GIVE;
		v2r1_printf( vty, "  Transfer onu %d/%d/%d app Image(name=%s):Send trying fail\r\n", slotId, port, (OnuIdx+1), ONU_APP_NAME);
		return (RERROR );
	}
		
	return( ROK );

}

/* modified by xieshl 20120305，非GWD方式(目前有CTC协议或PAS协议)升级ONU，升级完成后产生告警 ，问题单14690 */
int UpdateOnuFileImage(short int PonPortIdx, short int OnuIdx, unsigned char *onuTypeString, int length)
{
	int ret;
	LONG slotno, portno;
	ULONG onuDevIdx;

	slotno = GetCardIdxByPonChip( PonPortIdx );
	portno = GetPonPortByPonChip( PonPortIdx );
	if( (slotno == RERROR) || (portno == RERROR) )
		return VOS_ERROR;

	onuDevIdx = MAKEDEVID( slotno, portno, OnuIdx + 1 );
	
	ret = __UpdateOnuFileImage(PonPortIdx, OnuIdx, onuTypeString, length);
	if( ret == ROK )
	{
		onuSoftwareLoadSuccess_EventReport( onuDevIdx );
	}
	else
	{
		onuSoftwareLoadFailure_EventReport( onuDevIdx );
	}
	return ret;
}

int ScanOnuMgmtTableBurningFlash()
{
	short int PonPortIdx, OnuIdx;
	LONG entry, burning;

	for( PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		if( PonPortIsWorking(PonPortIdx) != TRUE ) continue;

		entry = PonPortIdx * MAXONUPERPON;
		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
		{
			if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
			if( GetOnuOperStatus( PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP )
				continue;

			ONU_MGMT_SEM_TAKE;
			burning = OnuMgmtTable[entry+OnuIdx].BurningFlash;
			ONU_MGMT_SEM_GIVE;
			if((burning == V2R1_ENABLE ) ||(burning == V2R1_COMPLETE ))
				CheckOnuWhetherBurnFlashComplete(PonPortIdx, OnuIdx);
		}
	}
	
	return( ROK );
}

/* modified by chenfj 2007-11-28
  	检查到ONU 写flash 完成后，再等待5 秒钟后，复位ONU 
  	*/
int  CheckOnuWhetherBurnFlashComplete( short int PonPortIdx, short int OnuIdx)
{
	short int ret;
	short int llid;
	int slotId, port;
	bool complete;
	LONG onuEntry;
	
	struct vty *vty;
	
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	vty = OnuMgmtTable[onuEntry].vty ;
	if( cl_vty_valid( vty ) == VOS_ERROR )
		vty = NULL;

	slotId = GetCardIdxByPonChip(PonPortIdx);
	port   = GetPonPortByPonChip(PonPortIdx);

	/* 升级过程中，ONU 离线*/
	if( GetOnuOperStatus( PonPortIdx , OnuIdx ) != ONU_OPER_STATUS_UP )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuEntry].BurningFlash = V2R1_DISABLE;
		ONU_MGMT_SEM_GIVE;
		return( RERROR );
	}
	
	llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

	ONU_MGMT_SEM_TAKE;
	if( OnuMgmtTable[onuEntry].BurningFlash == V2R1_COMPLETE )
	{
		OnuMgmtTable[onuEntry].BurningWait ++;
		if( OnuMgmtTable[onuEntry].BurningWait > ONU_FLASH_BURNING_WAIT )
		{		
			OnuMgmtTable[onuEntry].BurningFlash = V2R1_DISABLE;
			OnuMgmtTable[onuEntry].BurningWait = 0;
			ResetOnu( PonPortIdx, OnuIdx );
		}
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	ONU_MGMT_SEM_GIVE;
		
	
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		ret = OnuMgt_GetBurnImageComplete(PonPortIdx, OnuIdx, &complete );
	#else
	ret = REMOTE_PASONU_get_burn_image_complete(PonPortIdx, llid, &complete );
	#endif
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(ret == PAS_EXIT_OK ) 
		{
		if (complete == TRUE )
			{
			/*
			if( vty != NULL )
				{
				vty_out(vty, "  Onu %d/%d/%d burn app image to flash complete\r\n", slotId, port, (OnuIdx +1));
				vty_event( VTY_WRITE, vty->fd, vty );
				}
			else sys_console_printf("  Onu %d/%d/%d burn app image to flash complete\r\n", slotId, port, (OnuIdx +1));
			*/
			unsigned long devIdx;
			/*devIdx = slotId * 10000 + port * 1000 + (OnuIdx +1);*/
                    devIdx=MAKEDEVID(slotId,port,(OnuIdx +1));
			onuSoftwareLoadSuccess_EventReport(devIdx );
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[onuEntry].BurningFlash = V2R1_COMPLETE;
			OnuMgmtTable[onuEntry].BurningWait = 0;
			ONU_MGMT_SEM_GIVE;
			/*ResetOnu( PonPortIdx, OnuIdx );*/
			}
		return( ROK );
		}
	else {
		
		return( RERROR );
		}

	/*sys_console_printf(" onu %d/%d/%d write app image to flash completed\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));*/
}

int GetNumOfOnuUpdate()
{
	short int PonPortIdx, OnuIdx;
	int count = 0;

	for( PonPortIdx= 0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
		{
			if( GetOnuSWUpdateStatus( PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
				count++;
		}
	}
	return( count );
}

/******   end ***************/

/**************************************************************
 *
 *    Function:  GetOnuDistance( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *              
 *    Desc:  get the ONU ranging value
 *
 *    Return:   ranging value
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  GetOnuDistance( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	LONG onuEntry;
	short int PonChipType;
#if 0	
	short int onu_id;
	onu_registration_data_record_t onu_registration_data;
		
	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	
	if( ( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 ) )
	{
		if( onu_id != INVALID_LLID  )
		{
			onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
			ret = PAS_get_onu_registration_data( PonPortIdx,  onu_id, &onu_registration_data);
			if ( ret == PAS_EXIT_OK )
			{
				ONU_MGMT_SEM_TAKE;
				OnuMgmtTable[onuEntry].RTT = onu_registration_data.rtt * 16 /10;
				ONU_MGMT_SEM_GIVE;
			}
			return ( OnuMgmtTable[onuEntry].RTT );
		}
	}
	else{ /* other pon chip handler */

	}
#else
    onu_registration_info_t onu_registration_data;

	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

    if ( 0 == (ret = OnuMgt_GetOnuRegisterInfo( PonPortIdx, OnuIdx, &onu_registration_data )) )
    {
    	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
        
		ONU_MGMT_SEM_TAKE;
        if ( OLT_PONCHIP_ISPAS(PonChipType) )
        {
    		OnuMgmtTable[onuEntry].RTT = (onu_registration_data.rtt *16/10);
        }
        else
        {
    		OnuMgmtTable[onuEntry].RTT = onu_registration_data.rtt;
        }
		ONU_MGMT_SEM_GIVE;
    }
#endif

	return( RERROR );
}


int  GetOnuMaxMacNum( short int PonPortIdx, short int OnuIdx, unsigned int *pValBuf )
{
	int onuStatus;
	int max_mac = 0;
	CHECK_ONU_RANGE

	if( pValBuf == NULL ) return( RERROR );

	onuStatus = ThisIsValidOnu( PonPortIdx, OnuIdx);
	if( onuStatus != ROK ) return ( onuStatus );
	ONU_MGMT_SEM_TAKE;
	max_mac = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].MaxMAC;
	ONU_MGMT_SEM_GIVE;
    * pValBuf = max_mac & (~ONU_NOT_DEFAULT_MAX_MAC_FLAG);
	return( ROK );
}

int  SetOnuMaxMacNum( short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned int val )
{
	/*short int PonChipType;
	short int PonChipVer = RERROR;
	short int onu_id ;*/
	int onuStatus;
	
	CHECK_ONU_RANGE

	onuStatus = ThisIsValidOnu( PonPortIdx, OnuIdx);
	if( onuStatus != ROK ) return ( onuStatus );

#if 1
	return OnuMgt_SetOnuMaxMac(PonPortIdx, OnuIdx, LlidIdx, &val);
#else
	/*sys_console_printf("pon%d/onu%d MAX mac %d\r\n", PonPortIdx, (OnuIdx+1), val );*/
	OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].MaxMAC = val;
	if(GetOnuOperStatus_1( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_DOWN ) 
		{
		/*OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].MaxMAC = val;*/
		return( ROK );
		}
	
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
	if( onu_id == INVALID_LLID ) return( RERROR );

	LlidIdx = 0;
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	/*1  if onu is onLine, set this value to onu */
	/*if( OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE ) return( RERROR );*/

	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
			/* modified for PAS-SOFT V5.1
			PonChipVer = V2R1_GetPonChipVersion( PonPortIdx); */
			if( (PonChipVer != PONCHIP_PAS5001) && (PonChipVer != PONCHIP_PAS5201) ) return( RERROR );
			/*	
			onuStatus = GetOnuOperStatus(PonPortIdx, OnuIdx );
			if( onuStatus != ONU_OPER_STATUS_UP ) return( RERROR );
			*/
			if( PonChipVer == PONCHIP_PAS5001 )
				{
				PON_address_table_entries_llid_limitation_t  entryNum;
				
				/*if( val > MAXMACNUM ) entryNum =(PON_address_table_entries_llid_limitation_t) MAXMACNUM;*/
				if(( val >= (PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022+2)) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022;
				if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_512) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_512;
				if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_256 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_512 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_256;
				if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_128 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_256 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_128;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_64 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_128 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_64;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_32 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_64 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_32;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_16 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_32 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_16;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_8 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_16))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_8;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_4 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_8 ))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_4;
				else if(( val >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_2 ) &&( val < PON_ADDRESS_TABLE_ENTRY_LIMITATION_4))
					entryNum = PON_ADDRESS_TABLE_ENTRY_LIMITATION_2;
				else entryNum =(PON_address_table_entries_llid_limitation_t) val;
				
#if 0
				/* pas5001 not supported, so disable it 
				ret = PAS_set_llid_address_table_configuration_v4( PonPortIdx, onu_id, entryNum );*/
				REMOTE_PASONU_address_table_set_uni_learn_limit( PonPortIdx, onu_id, val );
				REMOTE_PASONU_address_table_clear( PonPortIdx, onu_id );
				EnableOnuSAMacFilter( PonPortIdx, OnuIdx );
				PAS_reset_address_table_extended_v4( PonPortIdx, onu_id, DYNAMIC );
#endif
				/*
				PAS_set_port_mac_limiting( PonPortIdx, onu_id, val);
				PAS_reset_address_table( PonPortIdx, onu_id, ADDR_DYNAMIC );
				*/
				/*2 save value in the onuMgmtTable */
				OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[LlidIdx].MaxMAC = val;
				
				/*if( ret == PAS_EXIT_OK )*/
				return( ROK );
				}
#if 0
			else if(( PonChipVer == PONCHIP_PAS5201 ) &&( V2R1_NOT_CTC_STACK ))
				{
				/*PAS_set_port_mac_limiting( PonPortIdx, onu_id, val );*/
				REMOTE_PASONU_address_table_set_uni_learn_limit( PonPortIdx, onu_id, val );
				PAS_reset_address_table( PonPortIdx, onu_id, DYNAMIC );
				REMOTE_PASONU_address_table_clear( PonPortIdx, onu_id );
				EnableOnuSAMacFilter( PonPortIdx, OnuIdx );
				
				/*2 save value in the onuMgmtTable */
				OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[LlidIdx].MaxMAC =val;
				
				/*if( ret == PAS_EXIT_OK )*/
				return( ROK );
				}	
#endif
			else if(( PonChipVer == PONCHIP_PAS5201)/* && ( V2R1_CTC_STACK )*/)
				{
				PON_address_table_config_t      address_table_config;
				short int  pas_ret;

				pas_ret = PAS_get_address_table_configuration( PonPortIdx, &address_table_config );
				if( pas_ret != PAS_EXIT_OK ) return( RERROR );

				if( val == 0 ) 
					{
					/*address_table_config.discard_llid_unlearned_sa =  V2R1_discard_llid_unlearned_sa;
					address_table_config.discard_unknown_da = V2R1_discard_unknown_da;
					PAS_set_address_table_configuration( PonPortIdx, address_table_config );*/
					PAS_set_port_mac_limiting( PonPortIdx, onu_id, 8191);
					
					}
				else {
					/*address_table_config.discard_llid_unlearned_sa = V2R1_discard_llid_unlearned_sa;
					address_table_config.discard_unknown_da = V2R1_discard_unknown_da;
					PAS_set_address_table_configuration( PonPortIdx, address_table_config );*/
					if( val >= 8192 )
						PAS_set_port_mac_limiting( PonPortIdx, onu_id, (val-1));
					else 
						PAS_set_port_mac_limiting( PonPortIdx, onu_id, (val+1));
					}
				
				/*PAS_reset_address_table( PonPortIdx, onu_id, ADDR_DYNAMIC );*/
				OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[LlidIdx].MaxMAC = val;
				
				return( ROK );
				}
		}
	
	else {
		return( RERROR );
		}
	
	return( RERROR );
#endif
}

int CopyOnuMaxMacCfg(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    unsigned int nSrcCfg;

    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    ONU_MGMT_SEM_TAKE;
    nSrcCfg   = OnuMgmtTable[iSrcEntry].LlidTable[0].MaxMAC;
    ONU_MGMT_SEM_GIVE;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuMaxMac(DstPonPortIdx, DstOnuIdx, 0, &nSrcCfg);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( MaxMACDefault != nSrcCfg )
        {
            iRlt = OnuMgt_SetOnuMaxMac(DstPonPortIdx, DstOnuIdx, 0, &nSrcCfg);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            unsigned int nDstCfg;
            
            iDstEntry = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            ONU_MGMT_SEM_TAKE;
            nDstCfg   = OnuMgmtTable[iDstEntry].LlidTable[0].MaxMAC;
            ONU_MGMT_SEM_GIVE;
            if ( nDstCfg != nSrcCfg )
            {
                iRlt = OnuMgt_SetOnuMaxMac(DstPonPortIdx, DstOnuIdx, 0, &nSrcCfg);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuMaxMac(DstPonPortIdx, DstOnuIdx, 0, &nSrcCfg);
        }
    }

    return iRlt;
}

/*begin: 增加支持ONU管理IP的配置加载，mod by liuyh, 2017-5-12*/
int CopyOnuMngIpCfg(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    RPC_CTC_mxu_mng_global_parameter_config_t stSrcOnuMng;

    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    
    ONU_MGMT_SEM_TAKE;
    stSrcOnuMng.mxu_mng.mng_ip = OnuMgmtTable[iSrcEntry].mngIp.ip;
    stSrcOnuMng.mxu_mng.mng_gw = OnuMgmtTable[iSrcEntry].mngIp.gw;
    stSrcOnuMng.mxu_mng.mng_mask = OnuMgmtTable[iSrcEntry].mngIp.mask;
    stSrcOnuMng.mxu_mng.data_cvlan = OnuMgmtTable[iSrcEntry].mngIp.cVlan;
    stSrcOnuMng.mxu_mng.data_svlan = OnuMgmtTable[iSrcEntry].mngIp.sVlan;
    stSrcOnuMng.mxu_mng.data_priority = OnuMgmtTable[iSrcEntry].mngIp.pri;    
    ONU_MGMT_SEM_GIVE;

    stSrcOnuMng.needSaveOlt = TRUE;
    
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetMxuMngGlobalConfig(DstPonPortIdx, DstOnuIdx, &stSrcOnuMng);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( 0 !=  stSrcOnuMng.mxu_mng.data_cvlan)
        {
            iRlt = OnuMgt_SetMxuMngGlobalConfig(DstPonPortIdx, DstOnuIdx, &stSrcOnuMng);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            RPC_CTC_mxu_mng_global_parameter_config_t stDstOnuMng;
            
            iDstEntry = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            
            ONU_MGMT_SEM_TAKE;
            stDstOnuMng.mxu_mng.mng_ip = OnuMgmtTable[iDstEntry].mngIp.ip;
            stDstOnuMng.mxu_mng.mng_gw = OnuMgmtTable[iDstEntry].mngIp.gw;
            stDstOnuMng.mxu_mng.mng_mask = OnuMgmtTable[iDstEntry].mngIp.mask;
            stDstOnuMng.mxu_mng.data_cvlan = OnuMgmtTable[iDstEntry].mngIp.cVlan;
            stDstOnuMng.mxu_mng.data_svlan = OnuMgmtTable[iDstEntry].mngIp.sVlan;
            stDstOnuMng.mxu_mng.data_priority = OnuMgmtTable[iDstEntry].mngIp.pri;
            ONU_MGMT_SEM_GIVE;
            
            if (stDstOnuMng.mxu_mng.mng_ip != stSrcOnuMng.mxu_mng.mng_ip
             || stDstOnuMng.mxu_mng.mng_mask != stSrcOnuMng.mxu_mng.mng_mask
             || stDstOnuMng.mxu_mng.mng_gw != stSrcOnuMng.mxu_mng.mng_gw
             || stDstOnuMng.mxu_mng.data_cvlan != stSrcOnuMng.mxu_mng.data_cvlan
             || stDstOnuMng.mxu_mng.data_svlan != stSrcOnuMng.mxu_mng.data_svlan
             || stDstOnuMng.mxu_mng.data_priority != stSrcOnuMng.mxu_mng.data_priority)
            {
                iRlt = OnuMgt_SetMxuMngGlobalConfig(DstPonPortIdx, DstOnuIdx, &stSrcOnuMng);
            }
        }
        else
        {
            iRlt = OnuMgt_SetMxuMngGlobalConfig(DstPonPortIdx, DstOnuIdx, &stSrcOnuMng);
        }
    }

    return iRlt;
}
        
/*end: mod by liuyh, 2017-5-12*/ 

int SetOnuMaxMacNumAll( short int PonPortIdx, unsigned int Number)
{
	short int OnuIdx;
	short int LlidIdx;

	CHECK_PON_RANGE

	LlidIdx = 0;

	for( OnuIdx =0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
	{
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, LlidIdx, Number );
	}
	return( ROK );
}

int CancelOnuMaxMacNum(short int PonPortIdx, short int OnuIdx)
{
	short int LlidIdx;

	CHECK_ONU_RANGE	

	LlidIdx = 0;
	return( SetOnuMaxMacNum( PonPortIdx, OnuIdx, LlidIdx, PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192 ));
}

int CancelOnuMaxMacNumAll(short int PonPortIdx)
{

	short int OnuIdx;
	short int LlidIdx;

	CHECK_PON_RANGE	

	LlidIdx = 0;
	for( OnuIdx = 0; OnuIdx< MAXONUPERPON; OnuIdx++)
	{
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, LlidIdx, PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192 );
	}
	return( ROK );
}


/**************************************************************
 *
 *    Function:  GetOnuCurrentAlarm( short int PonPortIdx, short int OnuIdx, unsigned char *CurrentAlarmStatus )
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *              
 *    Desc:  get the ONU currently alarm status
 *
 *    Return:  ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  GetOnuCurrentAlarm( short int PonPortIdx, short int OnuIdx,  unsigned long *CurrentAlarmStatus )
{
	/*char AlarmStatus[MAX_ONU_ALARM_NUM/8] = {0};*/	/* removed 20070705 */
	
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	if( CurrentAlarmStatus == NULL ) {
		/*sys_console_printf(" error: the pointer is null ( GetOnuCurrentAlarm() )\r\n" );*/
		VOS_ASSERT(0);
		return( RERROR );
		}
	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		/*VOS_MemCpy( AlarmStatus, OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx].AlarmStatus, sizeof( OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx].AlarmStatus ) );*/
		ONU_MGMT_SEM_TAKE;
		*CurrentAlarmStatus = OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx].AlarmStatus;
		ONU_MGMT_SEM_GIVE;
		return ( ROK );
		}
	else{
		/**CurrentAlarmStatus = CLEAR;*/
		*CurrentAlarmStatus = 0;
		}

	return( RERROR );
}

#if 0	/* removed 20070705 */
int  GetOnuCurrentAlarmRank(short int PonPortIdx, short int OnuIdx , int *CurAlarm )
{

	char AlarmStatus[MAX_ONU_ALARM_NUM/8] = {0};
	AlarmClass AlarmRank = CLEAR;

	if( CurAlarm == NULL ) return( RERROR );
	
	CHECK_ONU_RANGE
		
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	VOS_MemCpy( AlarmStatus, OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx].AlarmStatus, sizeof( OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx].AlarmStatus ) );

	/* 当前存在告警解析*/
	
	*CurAlarm = AlarmRank;
	return( ROK );

}
#endif

/**************************************************************
 *
 *    Function:  SetOnuStaticMacAddr( short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr)
 *
 *    Param:   short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx        -- the specific Onu
 *                unsigned char *MacAddr - the mac address to be added as static
 *              
 *    Desc:  
 *
 *    Return:  ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ***************************************************************/
int  SetOnuStaticMacAddr( short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr)
{
    
	short int PonChipType;
	
	short int llid;
	PON_address_record_t address_record;

	MacTable_S *CurrentPointer, *MacPointer;
	
	
	CHECK_ONU_RANGE

	if( MacAddr == NULL ) return( RERROR );
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	MacPointer = (MacTable_S *) VOS_Malloc( sizeof( MacTable_S ), MODULE_ONU );
	if( MacPointer == NULL )
		{
		VOS_ASSERT(0);
		return( RERROR );
		}
	VOS_MemCpy( MacPointer->MAC, MacAddr, BYTES_IN_MAC_ADDRESS );
	
	ONU_MGMT_SEM_TAKE;
	/* save this address to the OnuMgmtTable */
	CurrentPointer = &(OnuMgmtTable[PonPortIdx * MAXONUPERPON +OnuIdx].StaticMac);			
	MacPointer->nextMac = CurrentPointer->nextMac;;
	CurrentPointer->nextMac = (unsigned int )MacPointer;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON +OnuIdx].StaticMacNum ++;
	ONU_MGMT_SEM_GIVE;
	
	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		
		/*if( PonChipType == PONCHIP_PAS )*/
		if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		
			llid = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);		
			address_record.type = ADDR_STATIC;
			VOS_MemCpy( address_record.mac_address, MacAddr, BYTES_IN_MAC_ADDRESS );		
			address_record.logical_port = llid;	
		    #if 1
			OLT_AddMacAddrTbl( PonPortIdx,1, &address_record);
			#else
			PAS_add_address_table_record( PonPortIdx, &address_record);
			#endif
		}/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	
		else { /* other pon chip handler */
			}

		}

	return( ROK );
}

int  DelOnuStaticMacAddr( short int PonPortIdx, short int OnuIdx, unsigned char *MacAddr )
{
	int onuEntry;
	MacTable_S *CurrentPointer, *PrePointer;

	CHECK_ONU_RANGE
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	if( OnuMgmtTable[onuEntry].StaticMacNum == 0 ) return( ROK );
	if( MacAddr == NULL ) return( RERROR );

	/*PonChipType = GetPonChipTypeByPonPort(  PonPortIdx);*/

	ONU_MGMT_SEM_TAKE;

	/* check if the mac address is in the address list; if yes, delete it */
	PrePointer = &(OnuMgmtTable[onuEntry].StaticMac);

	while( PrePointer->nextMac != 0 )
	{
		CurrentPointer = (MacTable_S *)PrePointer->nextMac;
		if( MAC_ADDR_IS_EQUAL( MacAddr, CurrentPointer->MAC) )
		{
			PrePointer = (MacTable_S *) CurrentPointer->nextMac;
			VOS_MemSet( (char *)CurrentPointer, 0 , sizeof( MacTable_S ) );
			VOS_Free( (void *)CurrentPointer );			
		}
		else{
			PrePointer = CurrentPointer;
		}
	}
	ONU_MGMT_SEM_GIVE;

	/* check if the mac address is in pon chip address tabel; if yes delete it */
#if 0
	if( PonPortIsWorking( PonPortIdx ) == TRUE )
		{
		if( PonChipType == PONCHIP_PAS )
			{
			short int active_records;
			short int i;

			if(PAS_get_address_table(PonPortIdx, & active_records, Mac_addr_table) != PAS_EXIT_OK) return( ROK );
			if( active_records == 0 ) return( ROK );

			for( i=0; i<active_records; i++ )
				{
				if( Mac_addr_table[i].type == ADDRESS_STATIC )
					if( CompTwoMacAddress( Mac_addr_table[i].mac_address, MacAddr ) == ROK )
						PAS_remove_address_table_record( PonPortIdx, MacAddr );
				}
			}
	
		else {

			}
		}
	
	else{

		}
#endif

	return( ROK );
}

int  GetOnuStaticMacAddrList( short int PonPortIdx, short int OnuIdx, unsigned char *NumOfMacAddr, MacTable_S *MacAddr )
{
	LONG onuEntry;
	CHECK_ONU_RANGE

	if( MacAddr != NULL ) return( RERROR );
	if( NumOfMacAddr == NULL ) return( RERROR );
	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*NumOfMacAddr = OnuMgmtTable[onuEntry].StaticMacNum ;
	MacAddr = ( MacTable_S *)OnuMgmtTable[onuEntry].StaticMac.nextMac;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

/*****************************************************
 *
 *    Function:   set_PASONU_alarm_configuration( PonPort_id, onu_id ) 
 *
 *    Param:   none
 *                 
 *    Desc:  enable/disable onu alarm and set onu alarm threshold
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int InitOnuAlarmConfigDefault()
{
	OnuDefaultAlarmConfiguration.ber_alarm_active = TRUE;
	OnuDefaultAlarmConfiguration.ber_alarm_configuration.ber_threshold = PON_MONITORING_BER_THRESHOLD_DEFAULT_VALUE;
	OnuDefaultAlarmConfiguration.ber_alarm_configuration.direction = PON_DIRECTION_DOWNLINK;
	OnuDefaultAlarmConfiguration.ber_alarm_configuration.minimum_error_bytes_threshold = PON_MONITORING_MINIMUM_ERROR_BYTES_THRESHOLD_DEFAULT_VALUE;
	/*modified for PAS-SOFT V5.1*/
	/*OnuDefaultAlarmConfiguration.ber_alarm_configuration.TBD = 0;*/

	OnuDefaultAlarmConfiguration.fer_alarm_active = TRUE;
	OnuDefaultAlarmConfiguration.fer_alarm_configuration.fer_threshold = PON_MONITORING_FER_THRESHOLD_DEFAULT_VALUE;
	OnuDefaultAlarmConfiguration.fer_alarm_configuration.direction = PON_DIRECTION_DOWNLINK;
	OnuDefaultAlarmConfiguration.fer_alarm_configuration.minimum_error_frames_threshold = PON_MONITORING_MINIMUM_ERROR_FRAMES_THRESHOLD_DEFAULT_VALUE;
	/*modified for PAS-SOFT V5.1*/
	/*OnuDefaultAlarmConfiguration.fer_alarm_configuration.TBD = 0;*/
	
	return( ROK );
}


int set_ONU_alarm_configuration( short int PonPortIdx, short int OnuIdx ) 
{
	short int onu_id ;
	short int PonChipType, PonChipVer;
	
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx) !=  ROK ) return( RERROR );
	onu_id = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);
	if(onu_id == INVALID_LLID ) return ( RERROR );	

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if(OLT_PONCHIP_ISPAS(PonChipType)) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{

		}
	else{ /* other pon chip handler */

		}
	
	return( ROK );
}

/*****************************************************
 *
 *    Function:   SetOnuUplinkBW( short int PonPort_id, short int onu_id, int UplinkBW ) 
 *
 *    Param:   none
 *                 
 *    Desc:  
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int  SetOnuUplinkBW( short int PonPortIdx, short int OnuIdx, unsigned  int UplinkBw )
{
    int iRlt = ROK;
	/*int onuStatus;*/
	/*int Pre_Bw;*/
    int onuMgtIdx;
    /*unsigned int old_assured_bw;*/
    short int PonPortIdx_Swap;
    short int BWDir;
    ONU_bw_t onuBW;
	
	CHECK_ONU_RANGE

	/* added by chenfj 2006-10-30 */
	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );
	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/

    /* B--added by liwei056@2011-1-27 for DefaultBW */
    if ( 0 == UplinkBw )
    {
        /* 恢复默认带宽 */
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &UplinkBw, NULL);
        BWDir = OLT_CFG_DIR_UNDO;
    }
    else
    {
        /* 设置带宽 */
        BWDir = 0;
    }
    /* E--added by liwei056@2011-1-27 for DefaultBW */
	
	/*if( UplinkBw != OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr )*/
	{
		/*Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].UplinkBandwidth_gr;
		Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].ActiveUplinkBandwidth;*/
		UpdateProvisionedBWInfo( PonPortIdx );
        onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
		
		/* modify by chenfj 2006/09/20 */ 
		/*#2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置*/
		if( UplinkBw  > PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr)
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}

#if 1
        VOS_MemZero(&onuBW, sizeof(onuBW));
        onuBW.bw_class     = -1;
        onuBW.bw_gr        = UplinkBw;
        onuBW.bw_direction = BWDir | OLT_CFG_DIR_UPLINK;
        iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            if ( (iRlt > OLT_ERR_DBA_END) && (iRlt < OLT_ERR_DBA_BEGIN) )
            {
                iRlt = OLT_ERR_DBA_BEGIN - iRlt;
            }
        }
#else               
		if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )
			OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;

        /* B--added by liwei056@2009-1-13 for D9425 */
        old_assured_bw = OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr;
        /* E--added by liwei056@2009-1-13 for D9425 */
    
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr = UplinkBw;
		/*onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
		UpdateProvisionedBWInfo( PonPortIdx);*/
		
		onuStatus = GetOnuOperStatus(PonPortIdx, OnuIdx );
		/*onuStatus = PAS_get_onu_mode( PonPortIdx,  onu_id );
		if( (onuStatus == PON_ONU_MODE_ON ) &&( OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_ACTIVE )) */
		if( (onuStatus == ONU_OPER_STATUS_UP )/* &&( OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_ACTIVE )*/)
		{
		    int iRlt;
            
			/*PonPortTable[PonPortIdx].ActiveBW -= Pre_Bw;
			PonPortTable[PonPortIdx].RemainBW += Pre_Bw;*/
    	    iRlt = ActiveOnuUplinkBW( PonPortIdx, OnuIdx );
            /* B--added by liwei056@2009-1-13 for D9425 */
            if (iRlt != ROK)
            {
            	OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr = old_assured_bw;

                return iRlt;
            }
            /* E--added by liwei056@2009-1-13 for D9425 */
		}
#endif
	}
    
    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    if(UplinkBw > ( ponBWInfo.RemainBW + onuBWInfo.FinalUplinkBandwidth_gr ) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
	
	return( iRlt );
}

/**
 * get onu default bw(uplink or downlink) according to pon rate
 * params:
 *		PonPortIdx: index of pon port, check range before calling
 *		OnuIdx: index of onu in pon port, check range before calling
 *		direction: OLT_CFG_DIR_UPLINK or OLT_CFG_DIR_DOWNLINK, can't be OLT_CFG_DIR_BOTH
 *		bw_gr: pointer to default assured bandwidth to get, can be NULL 
 *		bw_be: pointer to default best-effort bandwidth to get, can be NULL 
 *	return:
 *		ROK or RERROR, return RERROR when neither bw_gr nor bw_be point to NULL or when PonRate is undefined. 
 */
int GetOnuDefaultBWByPonRate(short int PonPortIdx, short int OnuIdx, char direction, unsigned int * bw_gr, unsigned int * bw_be)
{
	short int OnuEntry;
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx; 
	if(!bw_gr && !bw_be) return RERROR;
	if(V2R1_ONU_GPON == OnuMgmtTable[OnuEntry].DeviceInfo.type)
	{
		if(bw_gr)
		{
			if(direction & OLT_CFG_DIR_UPLINK)
			{
				*bw_gr = OnuConfigDefault.UplinkBandwidth_GPON;
			}
			if(direction & OLT_CFG_DIR_DOWNLINK)
			{
				*bw_gr = OnuConfigDefault.DownlinkBandwidth_GPON;
			}
		}
		if(bw_be)
		{
			if(direction & OLT_CFG_DIR_UPLINK)
			{
				*bw_be = OnuConfigDefault.UplinkBandwidthBe_GPON;
			}
			if(direction & OLT_CFG_DIR_DOWNLINK)
			{
				*bw_be = OnuConfigDefault.DownlinkBandwidthBe_GPON;
			}
		}
	}
	else
	{
		switch (OnuMgmtTable[OnuEntry].PonRate) //expand cases when increase pon rate type
		{
			case PON_RATE_1_10G:
				if(bw_gr)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_gr = OnuConfigDefault.UplinkBandwidth_XGEPON_ASYM;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_gr = OnuConfigDefault.DownlinkBandwidth_XGEPON_ASYM;
					}
				}
				
				if(bw_be)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_be = OnuConfigDefault.UplinkBandwidthBe_XGEPON_ASYM;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_be = OnuConfigDefault.DownlinkBandwidthBe_XGEPON_ASYM;
					}
				}
				break;
			case PON_RATE_10_10G:
				if(bw_gr)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_gr = OnuConfigDefault.UplinkBandwidth_XGEPON_SYM;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_gr = OnuConfigDefault.DownlinkBandwidth_XGEPON_SYM;
					}
				}

				if(bw_be)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_be = OnuConfigDefault.UplinkBandwidthBe_XGEPON_SYM;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_be = OnuConfigDefault.DownlinkBandwidthBe_XGEPON_SYM;
					}
				}
				break;
			case PON_RATE_NORMAL_1G://no break here.
			default:
				if(bw_gr)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_gr = OnuConfigDefault.UplinkBandwidth;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_gr = OnuConfigDefault.DownlinkBandwidth;
					}
				}

				if(bw_be)
				{
					if(direction & OLT_CFG_DIR_UPLINK)
					{
						*bw_be = OnuConfigDefault.UplinkBandwidthBe;
					}
					if(direction & OLT_CFG_DIR_DOWNLINK)
					{
						*bw_be = OnuConfigDefault.DownlinkBandwidthBe;
					}
				}
				break;
		}
	}
	return ROK;
}


int  SetOnuUplinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int assured_bw, unsigned int best_effort_bw )
{
    int iRlt = ROK;
    int onuMgtIdx;
    short int PonPortIdx_Swap;
    short int BWDir;
	short int ponRate = PON_RATE_NORMAL_1G;
	short int onuType = 0;
    ONU_bw_t onuBW;
	
	CHECK_ONU_RANGE

	/* added by chenfj 2006-10-30 */
	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );

    onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	ponRate = OnuMgmtTable[onuMgtIdx].PonRate;

	GetOnuType(PonPortIdx, OnuIdx, &onuType);
	if(onuType == V2R1_ONU_GPON) ponRate = PON_RATE_1_2G; /* set pon rate 2g/1g to identify GPON onu*/

    /* B--added by liwei056@2011-1-27 for DefaultBW modified by liyang @2015-04-15 for Q25309*/
    if ( 0 == assured_bw || UPLINK_BW_EQUALS_TO_DEFAULT(ponRate, assured_bw, best_effort_bw) )
    {
        /* 恢复默认带宽 */
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &assured_bw, &best_effort_bw);
        BWDir = OLT_CFG_DIR_UNDO;
    }
    else
    {
        /* 设置带宽 */
        BWDir = 0;
    }
    /* E--added by liwei056@2011-1-27 for DefaultBW */

	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/
	
	/*if( UplinkBw != OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr )*/
	{
		/*Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].UplinkBandwidth_gr;
		Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].ActiveUplinkBandwidth;*/
		UpdateProvisionedBWInfo( PonPortIdx );
		
		/* modify by chenfj 2006/09/20 */ 
		/*#2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置*/
		if( assured_bw  > PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr)
			{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
			}

		if( UplinkClass > PRECEDENCE_OF_FLOW_7) return( RERROR );
		if(( Uplinkdelay != V2R1_DELAY_HIGH ) && ( Uplinkdelay != V2R1_DELAY_LOW )) return( RERROR );
		if( assured_bw > best_effort_bw ) return( RERROR );

        VOS_MemZero(&onuBW, sizeof(onuBW));
        onuBW.bw_gr        = assured_bw;
        onuBW.bw_be        = best_effort_bw;
        onuBW.bw_class     = UplinkClass;
        onuBW.bw_delay     = Uplinkdelay;
        onuBW.bw_direction = BWDir | OLT_CFG_DIR_UPLINK;
        iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            if ( (iRlt > OLT_ERR_DBA_END) && (iRlt < OLT_ERR_DBA_BEGIN) )
            {
                iRlt = OLT_ERR_DBA_BEGIN - iRlt;
            }
        }
	}
    
    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    if(assured_bw > ( ponBWInfo.RemainBW + onuBWInfo.FinalUplinkBandwidth_gr ) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
	
	return( ROK );
}

/* for PLATO DBA v3 */
#ifdef PLATO_DBA_V3
int  SetOnuUplinkBW_ByMac(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int fixed_bw, unsigned int assured_bw, unsigned int best_effort_bw )
{
    int iRlt = ROK;
    int onuMgtIdx;
    short int PonPortIdx_Swap;
    short int BWDir = OLT_CFG_DIR_DO_ACTIVE;
	short int ponRate = PON_RATE_NORMAL_1G;
	short int onuType = 0;

    ONU_bw_t onuBW;
    
	CHECK_ONU_RANGE

	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );

	onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	ponRate = OnuMgmtTable[onuMgtIdx].PonRate;

	GetOnuType(PonPortIdx, OnuIdx, &onuType);
	if(onuType == V2R1_ONU_GPON) ponRate = PON_RATE_1_2G; /* set pon rate 2g/1g to identify GPON onu*/

      /* B--added by liwei056@2011-1-27 for DefaultBW modified by liyang @2015-04-15 for Q25309*/
    if ( 0 == assured_bw || (fixed_bw == 0 && UPLINK_BW_EQUALS_TO_DEFAULT(ponRate, assured_bw, best_effort_bw)))
 	{
        /* 恢复默认带宽 */
        fixed_bw       = 0;
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &assured_bw, &best_effort_bw);
        BWDir |= OLT_CFG_DIR_UNDO;
    }
    else
    {
        /* 设置带宽 */
        BWDir |= 0;
    }
    /* E--added by liwei056@2011-1-27 for DefaultBW */

	UpdateProvisionedBWInfo( PonPortIdx );
	if((fixed_bw+assured_bw) > ( PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_fixed + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr) )
	{
		return ( V2R1_EXCEED_RANGE );
	}

	if(UplinkClass > PRECEDENCE_OF_FLOW_7) return( RERROR );
	if((Uplinkdelay != V2R1_DELAY_HIGH ) && ( Uplinkdelay != V2R1_DELAY_LOW )) return( RERROR );
	if((fixed_bw+assured_bw) > best_effort_bw ) return( RERROR );

    VOS_MemZero(&onuBW, sizeof(onuBW));
    onuBW.bw_gr        = assured_bw;
    onuBW.bw_be        = best_effort_bw;
    onuBW.bw_fixed     = fixed_bw;
    onuBW.bw_class     = UplinkClass;
    onuBW.bw_delay     = Uplinkdelay;
    onuBW.bw_direction = BWDir | OLT_CFG_DIR_UPLINK;
    iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        if ( (iRlt > OLT_ERR_DBA_END) && (iRlt < OLT_ERR_DBA_BEGIN) )
        {
            iRlt = OLT_ERR_DBA_BEGIN - iRlt;
        }
    }
    
    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    if((fixed_bw+assured_bw) > ( ponBWInfo.RemainBW + onuBWInfo.FinalUplinkBandwidth_fixed + onuBWInfo.FinalUplinkBandwidth_gr) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
				
			
        }
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
	
	return( iRlt );
}

int  SetOnuUplinkBW_2(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int fixed_bw, unsigned int assured_bw, unsigned int best_effort_bw )
{
    int iRlt = ROK;
	/*int onuStatus;*/
    int onuMgtIdx;
    /*unsigned int old_fixed_bw, old_assured_bw, old_best_effort_bw, old_class, old_delay;*/
    short int PonPortIdx_Swap;
    short int BWDir = OLT_CFG_DIR_BY_ONUID;
	short int ponRate = PON_RATE_NORMAL_1G;
	short int onuType = 0;
    ONU_bw_t onuBW;
    
	CHECK_ONU_RANGE

	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) return( V2R1_ONU_NOT_EXIST );


    if(CheckOnuActionIsValid(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK) == VOS_OK)
	{
        BWDir |= OLT_CFG_DIR_DO_ACTIVE;
	}

    onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	ponRate = OnuMgmtTable[onuMgtIdx].PonRate;

	GetOnuType(PonPortIdx, OnuIdx, &onuType);
	if(onuType == V2R1_ONU_GPON) ponRate = PON_RATE_1_2G; /* set pon rate 2g/1g to identify GPON onu*/
    
    /* B--added by liwei056@2011-1-27 for DefaultBW modified by liyang @2015-04-15 for Q25309*/
    if ( 0 == assured_bw || (fixed_bw == 0 && UPLINK_BW_EQUALS_TO_DEFAULT(ponRate, assured_bw, best_effort_bw)))
 	{
        /* 恢复默认带宽 */
        fixed_bw       = 0;
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &assured_bw, &best_effort_bw);
        BWDir |= OLT_CFG_DIR_UNDO;
    }
    else
    {
        /* 设置带宽 */
        BWDir |= 0;
    }
    /* E--added by liwei056@2011-1-27 for DefaultBW */

	UpdateProvisionedBWInfo( PonPortIdx );
    onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
	if((fixed_bw+assured_bw) > ( PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_fixed + OnuMgmtTable[onuMgtIdx].FinalUplinkBandwidth_gr) )
	{
		return ( V2R1_EXCEED_RANGE );
	}

	if(UplinkClass > PRECEDENCE_OF_FLOW_7) return( RERROR );
	if((Uplinkdelay != V2R1_DELAY_HIGH ) && ( Uplinkdelay != V2R1_DELAY_LOW )) return( RERROR );
	if((fixed_bw+assured_bw) > best_effort_bw ) return( RERROR );

    VOS_MemZero(&onuBW, sizeof(onuBW));
    onuBW.bw_gr        = assured_bw;
    onuBW.bw_be        = best_effort_bw;
    onuBW.bw_fixed     = fixed_bw;
    onuBW.bw_class     = UplinkClass;
    onuBW.bw_delay     = Uplinkdelay;
    onuBW.bw_direction = BWDir | OLT_CFG_DIR_UPLINK;
    iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        if ( (iRlt >= OLT_ERR_DBA_END) && (iRlt < OLT_ERR_DBA_BEGIN) )
        {
            iRlt = OLT_ERR_DBA_BEGIN - iRlt;
        }
    }
    
    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
             
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    if((fixed_bw+assured_bw) > ( ponBWInfo.RemainBW + onuBWInfo.FinalUplinkBandwidth_fixed + onuBWInfo.FinalUplinkBandwidth_gr ) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
	
	return( iRlt );
}
#endif

int SetOnuUpLinkBWAll( short int PonPortIdx, unsigned int UplinkBw)
{
	short int OnuIdx;
	CHECK_PON_RANGE

	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
	{
		SetOnuUplinkBW( PonPortIdx, OnuIdx, UplinkBw );
	}

    return( ROK );
}

int  GetOnuUplinkBW( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkBw )
{
	CHECK_ONU_RANGE

	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/

	if( UplinkBw != NULL )
	{
		ONU_MGMT_SEM_TAKE;
		*UplinkBw = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr ;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	else
        return( RERROR );
}

int  GetOnuRunningUplinkBW( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	int onuEntry;
	CHECK_ONU_RANGE

	if( ( UplinkClass == NULL ) ||(delay == NULL ) ||(assured_bw == NULL ) ||(best_effort_bw == NULL ) ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*UplinkClass = OnuMgmtTable[onuEntry].FinalUplinkClass;
	*delay = OnuMgmtTable[onuEntry].FinalUplinkDelay;
	*assured_bw = OnuMgmtTable[onuEntry].FinalUplinkBandwidth_gr;
	*best_effort_bw = OnuMgmtTable[onuEntry].FinalUplinkBandwidth_be;
	ONU_MGMT_SEM_GIVE;
	return( ROK );	
}
int  GetOnuRunningDownlinkBW(short int PonPortIdx, short int OnuIdx, unsigned int *DownlinkClass, unsigned int *DownlinkDelay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	ULONG devIdx = 0, enable = 0;
	int onuEntry;	
	CHECK_ONU_RANGE

	if((DownlinkClass == NULL ) ||(DownlinkDelay== NULL ) || (assured_bw == NULL ) || (best_effort_bw == NULL ) ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*DownlinkClass = OnuMgmtTable[onuEntry].FinalDownlinkClass ;
	*DownlinkDelay = OnuMgmtTable[onuEntry].FinalDownlinkDelay ;

	devIdx = parseDevidxFromPonOnu( PonPortIdx, OnuIdx );
	if (VOS_OK != getPonPortDownLinkPolicingEbl( devIdx, 1, 1, &enable ))
	{
		enable = 2;
	}

	if (2 == enable)
	{
		*assured_bw = 0;
	}
	else
	{
		*assured_bw = OnuMgmtTable[onuEntry].FinalDownlinkBandwidth_gr;	
	}
	
	*best_effort_bw = OnuMgmtTable[onuEntry].FinalDownlinkBandwidth_be;	
	ONU_MGMT_SEM_GIVE;

	/*sys_console_printf(" downlink bw: class %d delay %d gr %d best %d \r\n", *DownlinkClass, *DownlinkDelay, *assured_bw, *best_effort_bw );*/
	return( ROK );
}

int  GetOnuUplinkBW_1( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	int onuEntry;
	CHECK_ONU_RANGE

	if( ( UplinkClass == NULL ) ||(delay == NULL ) ||(assured_bw == NULL ) ||(best_effort_bw == NULL ) ) return( RERROR );

#if 0
	OnuCurStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx );

	if( OnuCurStatus == ONU_OPER_STATUS_UP )
		{
		PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		Llid = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
		if( Llid == INVALID_LLID ) return( RERROR );
		if( PonChipType == PONCHIP_PAS )
			{
			PLATO2_SLA_t SLA;
			short int error_code;
			ret = PLATO2_get_SLA( PonPortIdx, Llid, &SLA, &error_code );
			if(( ret == PLATO2_ERR_OK ) &&( error_code == ECODE_NO_ERROR ))
				{
				*UplinkClass = SLA.class;
				*delay = SLA.delay;
				*assured_bw = SLA.max_gr_bw * BITS_PER_SECOND_1M + SLA.max_gr_bw_fine * BITS_PER_SECOND_64K;
				*best_effort_bw = SLA.max_be_bw * BITS_PER_SECOND_1M + SLA.max_be_bw_fine * BITS_PER_SECOND_64K;
				/*sys_console_printf(" uplink bw1: class %d delay %d gr %d best %d \r\n", *UplinkClass, *delay, *assured_bw, *best_effort_bw );*/
				return( ROK );
				}
			}
		else{  /* other pon chip handler */
			}
		}
#endif
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*UplinkClass = OnuMgmtTable[onuEntry].LlidTable[0].UplinkClass;
	*delay = OnuMgmtTable[onuEntry].LlidTable[0].UplinkDelay;
	*assured_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_gr;
	*best_effort_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_be;
	ONU_MGMT_SEM_GIVE;
	/*sys_console_printf(" uplink bw2: class %d delay %d gr %d best %d \r\n", *UplinkClass, *delay, *assured_bw, *best_effort_bw );*/
	return( ROK );	
}

#ifdef PLATO_DBA_V3 
int  GetOnuUplinkBW_2( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *fixed, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	int onuEntry;
	CHECK_ONU_RANGE

	if( ( UplinkClass == NULL ) ||(fixed == NULL ) ||(assured_bw == NULL ) ||(best_effort_bw == NULL ) ) return( RERROR );

#if 0
	OnuCurStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx );

	if( OnuCurStatus == ONU_OPER_STATUS_UP )
		{
		PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		Llid = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
		if( Llid == INVALID_LLID ) return( RERROR );
		if( PonChipType == PONCHIP_PAS )
			{
			PLATO2_SLA_t SLA;
			short int error_code;
			ret = PLATO2_get_SLA( PonPortIdx, Llid, &SLA, &error_code );
			if(( ret == PLATO2_ERR_OK ) &&( error_code == ECODE_NO_ERROR ))
				{
				*UplinkClass = SLA.class;
				*delay = SLA.delay;
				*assured_bw = SLA.max_gr_bw * BITS_PER_SECOND_1M + SLA.max_gr_bw_fine * BITS_PER_SECOND_64K;
				*best_effort_bw = SLA.max_be_bw * BITS_PER_SECOND_1M + SLA.max_be_bw_fine * BITS_PER_SECOND_64K;
				/*sys_console_printf(" uplink bw1: class %d delay %d gr %d best %d \r\n", *UplinkClass, *delay, *assured_bw, *best_effort_bw );*/
				return( ROK );
				}
			}
		else{  /* other pon chip handler */
			}
		}
#endif
	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*UplinkClass = OnuMgmtTable[onuEntry].LlidTable[0].UplinkClass;
	*fixed = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_fixed;
	*assured_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_gr;
	*best_effort_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_be;
	ONU_MGMT_SEM_GIVE;
	/*sys_console_printf(" uplink bw2: class %d delay %d gr %d best %d \r\n", *UplinkClass, *delay, *assured_bw, *best_effort_bw );*/
	return( ROK );	
}

/* B--added by liwei056@2011-12-15 for D14182 */
int  GetOnuUplinkBW_3( short int PonPortIdx, short int OnuIdx , unsigned int *UplinkClass, unsigned int *delay, unsigned int *fixed, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	int onuEntry;
    
	CHECK_ONU_RANGE

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
    if ( UplinkClass )
    {
    	*UplinkClass = OnuMgmtTable[onuEntry].LlidTable[0].UplinkClass;
    }

    if ( delay )
    {
    	*delay = OnuMgmtTable[onuEntry].LlidTable[0].UplinkDelay;
    }

    if ( fixed )
    {
    	*fixed = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_fixed;
    }

    if ( assured_bw )
    {
    	*assured_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_gr;
    }

    if ( best_effort_bw )
    {
    	*best_effort_bw = OnuMgmtTable[onuEntry].LlidTable[0].UplinkBandwidth_be;
    }
	ONU_MGMT_SEM_GIVE;

	return( ROK );	
}
/* E--added by liwei056@2011-12-15 for D14182 */

#endif

int  GetOnuUplinkBWDefault(  unsigned int *UplinkBw )
{
	if( UplinkBw != NULL )
	{
		*UplinkBw = OnuConfigDefault.UplinkBandwidth ;
		return( ROK );
	}
	else
        return( RERROR );
}

int  GetOnuUplinkBWDefault_1(  unsigned int *UplinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{

	if( ( UplinkClass == NULL ) ||( delay == NULL ) ||( assured_bw == NULL ) ||( best_effort_bw == NULL ) ) return( RERROR );
		
	*UplinkClass = OnuConfigDefault.UplinkClass ;
	*delay = OnuConfigDefault.UplinkDelay;
	*assured_bw = OnuConfigDefault.UplinkBandwidth;
	*best_effort_bw = OnuConfigDefault.UplinkBandwidthBe;
	
	return( ROK );
}

int  SetOnuDownlinkBW( short int PonPortIdx, short int OnuIdx,  unsigned int DownlinkBw )
{
	/*int onuStatus;*/
	/*int Pre_Bw;*/
    int iRlt = ROK;
    int onuMgtIdx;
    short int PonPortIdx_Swap;
    short int BWDir;
    ONU_bw_t onuBW;
	
	CHECK_ONU_RANGE

	/* added by chenfj 2006-10-30 */
	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );
	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR ); */

	/*if( DownlinkBw != OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].DownlinkBandwidth_gr )*/
	{
		/*Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].DownlinkBandwidth_gr;*/
		if( DownlinkBw > MAX_DOWNLINK_SPEED ) 
		{
			/*DownlinkBw = MAX_DOWNLINK_SPEED;*/
			sys_console_printf("\r\n   Max downlink bandwidth is 500Mbps\r\n");
			return( RERROR );			
		}

        /* B--added by liwei056@2011-1-27 for DefaultBW */
        if ( 0 == DownlinkBw )
        {
            /* 恢复默认带宽 */
			GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &DownlinkBw, NULL);
            BWDir = OLT_CFG_DIR_UNDO;
        }
        else
        {
            /* 设置带宽 */
            BWDir = 0;
        }
        /* E--added by liwei056@2011-1-27 for DefaultBW */
		
		UpdateProvisionedBWInfo( PonPortIdx );
        onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;

		/* modify by chenfj 2006/09/20 */ 
		/*#2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置*/
		/* modified by chenfj 2006/11/15 
	       #3202问题单: 下行方向不受限方式，带宽只能一次比一次小*/
		if( DownlinkBw  > PonPortTable[PonPortIdx].DownlinkRemainBw + OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr)
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
		
#if 1
        VOS_MemZero(&onuBW, sizeof(onuBW));
        onuBW.bw_class     = -1;
        onuBW.bw_gr        = DownlinkBw;
        onuBW.bw_direction = BWDir | OLT_CFG_DIR_DOWNLINK;
        iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
#else        
		if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )
			OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;

		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr = DownlinkBw;
		/*onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
		UpdateProvisionedBWInfo(PonPortIdx);*/
		
		/*onuStatus = PAS_get_onu_mode( PonPortIdx,  onu_id);*/
		onuStatus = GetOnuOperStatus( PonPortIdx,  OnuIdx);
		if(( onuStatus == ONU_OPER_STATUS_UP )/*&&(OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_ACTIVE)*/)
		{
			/*PonPortTable[PonPortIdx].DownlinkRemainBw += Pre_Bw;
			PonPortTable[PonPortIdx].DownlinkActiveBw -= Pre_Bw;*/
			return( ActiveOnuDownlinkBW( PonPortIdx, OnuIdx ));
		}
#endif
	}

    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
             
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    if( DownlinkBw > ( ponBWInfo.DownlinkRemainBw + onuBWInfo.FinalDownlinkBandwidth_gr) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */

    return( ROK );
}

int  SetOnuDownlinkBW_ByMac(short int PonPortIdx, short int OnuIdx, unsigned int DownlinkClass, unsigned int DownlinkDelay, unsigned int assured_bw, unsigned int best_effort_bw )
{
    int iRlt = ROK;
	/*int onuStatus;*/
	/*int Pre_Bw;*/
    int onuMgtIdx;
    /*unsigned int old_assured_bw, old_best_effort_bw, old_class, old_delay;*/
    short int PonPortIdx_Swap;
    short int BWDir = OLT_CFG_DIR_DO_ACTIVE;
    ONU_bw_t onuBW;
	
	CHECK_ONU_RANGE

	/* added by chenfj 2006-10-30 */
	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );
	{
		if(DownlinkClass > PRECEDENCE_OF_FLOW_7 ) return( RERROR );
		if(( DownlinkDelay != V2R1_DELAY_HIGH ) && ( DownlinkDelay != V2R1_DELAY_LOW)) return( RERROR );

        /* B--added by liwei056@2011-1-27 for DefaultBW modified by liyang @2015-04-15 for Q25309*/
        if ( 0 == assured_bw || (assured_bw == OnuConfigDefault.DownlinkBandwidth && best_effort_bw == OnuConfigDefault.DownlinkBandwidthBe))
        {
            /* 恢复默认带宽 */
            assured_bw     = OnuConfigDefault.DownlinkBandwidth;
            best_effort_bw = OnuConfigDefault.DownlinkBandwidthBe;
            BWDir |= OLT_CFG_DIR_UNDO;
        }
        else
        {
            /* 设置带宽 */
    		if( assured_bw > best_effort_bw ) 
                return( RERROR );
            BWDir |= 0;
        }
        /* E--added by liwei056@2011-1-27 for DefaultBW */
		onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
		UpdateProvisionedBWInfo( PonPortIdx );
        

		/* modify by chenfj 2006/09/20 */ 
		/*#2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置*/
		/* modified by chenfj 2006/11/15 
	       #3202问题单: 下行方向不受限方式，带宽只能一次比一次小*/
		if( assured_bw  > PonPortTable[PonPortIdx].DownlinkRemainBw + OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr)
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
#endif

        VOS_MemZero(&onuBW, sizeof(onuBW));
        onuBW.bw_gr        = assured_bw;
        onuBW.bw_be        = best_effort_bw;
        onuBW.bw_class     = DownlinkClass;
        onuBW.bw_delay     = DownlinkDelay;
        onuBW.bw_direction = BWDir | OLT_CFG_DIR_DOWNLINK;
        iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
	}

    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
             
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    #if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
					    if( assured_bw > ( ponBWInfo.DownlinkRemainBw + onuBWInfo.FinalDownlinkBandwidth_gr) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						#endif
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }

#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
		UpdateProvisionedBWInfo( PonPortIdx );
		if( PonPortTable[PonPortIdx].DownlinkProvisionedBW > PonPortTable[PonPortIdx].DownlinkMaxBw )
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
#endif
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
    
	return( iRlt );
}

int  SetOnuDownlinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int DownlinkClass, unsigned int DownlinkDelay, unsigned int assured_bw, unsigned int best_effort_bw )
{
    int iRlt = ROK;
	/*int onuStatus;*/
	/*int Pre_Bw;*/
    int onuMgtIdx;
    /*unsigned int old_assured_bw, old_best_effort_bw, old_class, old_delay;*/
    short int PonPortIdx_Swap;
    short int BWDir = OLT_CFG_DIR_BY_ONUID;
	short int ponRate = PON_RATE_NORMAL_1G;
    ONU_bw_t onuBW;
    short int onuType = 0;
	
	CHECK_ONU_RANGE

	/* added by chenfj 2006-10-30 */
	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK ) return( V2R1_ONU_NOT_EXIST );
	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR ); */

    if(CheckOnuActionIsValid(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK) == VOS_OK)
	{
        BWDir |= OLT_CFG_DIR_DO_ACTIVE;
	}

	/*if( DownlinkBw != OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].DownlinkBandwidth_gr )*/
	{
		/*Pre_Bw = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].DownlinkBandwidth_gr;*/
        
		if(DownlinkClass > PRECEDENCE_OF_FLOW_7 ) return( RERROR );
		if(( DownlinkDelay != V2R1_DELAY_HIGH ) && ( DownlinkDelay != V2R1_DELAY_LOW)) return( RERROR );
		onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
		ponRate = OnuMgmtTable[onuMgtIdx].PonRate;

		GetOnuType(PonPortIdx, OnuIdx, &onuType);
		if(onuType == V2R1_ONU_GPON) ponRate = PON_RATE_1_2G; /* set pon rate 2g/1g to identify GPON onu*/

        /* B--added by liwei056@2011-1-27 for DefaultBW modified by liyang @2015-04-15 for Q25309*/
        if ( 0 == assured_bw || DOWNLINK_BW_EQUALS_TO_DEFAULT(ponRate, assured_bw, best_effort_bw))
        {
            /* 恢复默认带宽 */
			GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &assured_bw, &best_effort_bw);
            BWDir |= OLT_CFG_DIR_UNDO;
        }
        else
        {
            /* 设置带宽 */
    		if( assured_bw > best_effort_bw ) 
                return( RERROR );
            BWDir |= 0;
        }
        /* E--added by liwei056@2011-1-27 for DefaultBW */
		onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;

#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
		UpdateProvisionedBWInfo( PonPortIdx );
        
		/* modify by chenfj 2006/09/20 */ 
		/*#2590问题PON模式下，分配带宽；在设置带宽值超出正常范围后，仍然可以配置*/
		/* modified by chenfj 2006/11/15 
	       #3202问题单: 下行方向不受限方式，带宽只能一次比一次小*/
		if( assured_bw  > PonPortTable[PonPortIdx].DownlinkRemainBw + OnuMgmtTable[onuMgtIdx].FinalDownlinkBandwidth_gr)
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
#endif

        VOS_MemZero(&onuBW, sizeof(onuBW));
        onuBW.bw_gr        = assured_bw;
        onuBW.bw_be        = best_effort_bw;
        onuBW.bw_class     = DownlinkClass;
        onuBW.bw_delay     = DownlinkDelay;
        onuBW.bw_direction = BWDir | OLT_CFG_DIR_DOWNLINK;
        iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
	}

    /* B--added by liwei056@2010-3-9 for Pon-FastSwicthHover*/
    if ( ROK == iRlt )
    {
        if ( ROK == PonPortSwapPortQuery( PonPortIdx, &PonPortIdx_Swap) )
        {
            /*Begin:for onu swap by jinhl@2013-04-27*/
            PonPortBWInfo_S ponBWInfo;
			ONUBWInfo_S  onuBWInfo;
			short int onuIdx_Swap = 0;
            UpdateProvisionedBWInfo(PonPortIdx_Swap);
             
            if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(PonPortIdx) )
            {
                /* 需要同步修改备用OLT上的备用ONU */
                (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, OnuIdx, &onuBW);
            }
			else if(V2R1_PON_PORT_SWAP_ONU== GetPonPortHotSwapMode(PonPortIdx))
			{
			    onuIdx_Swap = OLTAdv_GetActOnuIdxByMac(PonPortIdx_Swap, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr);
				if(onuIdx_Swap >= 0)
				{
				    iRlt = OnuMgtAdv_GetBWInfo(PonPortIdx_Swap, onuIdx_Swap, &ponBWInfo, &onuBWInfo);
					if(VOS_OK == iRlt)
					{
					    #if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
					    if( assured_bw > ( ponBWInfo.DownlinkRemainBw + onuBWInfo.FinalDownlinkBandwidth_gr) )
						{
							return ( V2R1_Parter_EXCEED_RANGE );
						}
						#endif
						
                        (void)OnuMgt_SetOnuBW(PonPortIdx_Swap, onuIdx_Swap, &onuBW);
					}
				}
			}
			/*End:for onu swap by jinhl@2013-04-27*/
        }

#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_NO )
		UpdateProvisionedBWInfo( PonPortIdx );
		if( PonPortTable[PonPortIdx].DownlinkProvisionedBW > PonPortTable[PonPortIdx].DownlinkMaxBw )
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
#endif
    }
    /* E--added by liwei056@2010-3-9 for Pon-FastSwicthHover */
    
	return( iRlt );
}

int SetOnuDownLinkBWAll( short int PonPortIdx, unsigned int UplinkBw)
{
	short int OnuIdx;
	CHECK_PON_RANGE

	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
		{
		SetOnuDownlinkBW( PonPortIdx, OnuIdx, UplinkBw );
		}
	return( ROK );
}

int  Onu_IsDefaultBWSetting(short int PonPortIdx, short int OnuIdx)
{
    return OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].BandWidthIsDefault;
}

int  GetOnuDownlinkBW( short int PonPortIdx, short int OnuIdx , unsigned int *DownlinkBw )
{

	CHECK_ONU_RANGE

	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/

	if( DownlinkBw != NULL ){
		ONU_MGMT_SEM_TAKE;
		*DownlinkBw = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].DownlinkBandwidth_gr ;
		ONU_MGMT_SEM_GIVE;
	}
		
	return( ROK );
}

/* begin: modified by jianght 20090730 */
extern STATUS getPonPortDownLinkPolicingEbl( ulong_t devIdx, ulong_t brdIdx, ulong_t portIdx, ulong_t *enable );
int  GetOnuDownlinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int *DownlinkClass, unsigned int *DownlinkDelay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{
	ULONG devIdx = 0, enable = 0;
	int onuEntry;	
	CHECK_ONU_RANGE

	/*if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].LlidTable[0].EntryStatus == LLID_ENTRY_UNKNOWN )return( RERROR );*/
	if((DownlinkClass == NULL ) ||(DownlinkDelay== NULL ) || (assured_bw == NULL ) || (best_effort_bw == NULL ) ) return( RERROR );

	onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*DownlinkClass = OnuMgmtTable[onuEntry].LlidTable[0].DownlinkClass ;
	*DownlinkDelay = OnuMgmtTable[onuEntry].LlidTable[0].DownlinkDelay ;

	devIdx = parseDevidxFromPonOnu( PonPortIdx, OnuIdx );
	if (VOS_OK != getPonPortDownLinkPolicingEbl( devIdx, 1, 1, &enable ))
	{
		enable = 2;
	}

	if (2 == enable)
	{
		*assured_bw = 0;
	}
	else
	{
		*assured_bw = OnuMgmtTable[onuEntry].LlidTable[0].DownlinkBandwidth_gr;	
	}
	
	*best_effort_bw = OnuMgmtTable[onuEntry].LlidTable[0].DownlinkBandwidth_be;	
	ONU_MGMT_SEM_GIVE;

	/*sys_console_printf(" downlink bw: class %d delay %d gr %d best %d \r\n", *DownlinkClass, *DownlinkDelay, *assured_bw, *best_effort_bw );*/
	return( ROK );
}
/* end: modified by jianght 20090730 */

int  GetOnuDownlinkBWDefault(  unsigned int *DownlinkBw )
{
	if( DownlinkBw != NULL )
		{
		*DownlinkBw = OnuConfigDefault.DownlinkBandwidth ;
		return( ROK );
		}
	else return( RERROR );
}

int  GetOnuDownlinkBWDefault_1(  unsigned int *DownlinkClass, unsigned int *delay, unsigned int *assured_bw, unsigned int *best_effort_bw )
{

	if( ( DownlinkClass == NULL ) ||( delay == NULL ) ||( assured_bw == NULL ) ||( best_effort_bw == NULL ) ) return( RERROR );
		
	*DownlinkClass = OnuConfigDefault.DownlinkClass ;
	*delay = OnuConfigDefault.DownlinkDelay;
	*assured_bw = OnuConfigDefault.DownlinkBandwidth;
	*best_effort_bw = OnuConfigDefault.DownlinkBandwidthBe;
	
	return( ROK );
}

int CopyOnuUplinkBW(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iSrcEntry;
    OnuLLIDTable_S *pLlidCfg;
    ONU_bw_t onuSrcBW;
	int iRlt = 0;

    /* 目前，只支持Cover拷贝模式 */
    VOS_MemZero(&onuSrcBW, sizeof(onuSrcBW));
    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    
	ONU_MGMT_SEM_TAKE;
    pLlidCfg = &(OnuMgmtTable[iSrcEntry].LlidTable[0]);
    onuSrcBW.bw_gr    = pLlidCfg->UplinkBandwidth_gr;
    onuSrcBW.bw_be    = pLlidCfg->UplinkBandwidth_be;
    onuSrcBW.bw_fixed = pLlidCfg->UplinkBandwidth_fixed;
    onuSrcBW.bw_class = pLlidCfg->UplinkClass;
    onuSrcBW.bw_delay = pLlidCfg->UplinkDelay;
    if ( OLT_CFG_DIR_UPLINK & pLlidCfg->BandWidthIsDefault )
    {
        /* 拷贝默认带宽 */
        onuSrcBW.bw_direction = OLT_CFG_DIR_UNDO | OLT_CFG_DIR_UPLINK;
    }
    else
    {
        /* 拷贝指定带宽 */
        onuSrcBW.bw_direction = OLT_CFG_DIR_UPLINK;
    }
	ONU_MGMT_SEM_GIVE;
    onuSrcBW.bw_direction |= OLT_CFG_DIR_BY_ONUID;

	/*Begin:for onu swap by jinhl@2013-04-27*/
	{
	    PonPortBWInfo_S ponBWInfo;
		ONUBWInfo_S  onuBWInfo;
		
	    UpdateProvisionedBWInfo(DstPonPortIdx);
	    iRlt = OnuMgtAdv_GetBWInfo(DstPonPortIdx, DstOnuIdx, &ponBWInfo, &onuBWInfo);
		if(VOS_OK == iRlt)
		{
		    if( onuSrcBW.bw_gr + onuSrcBW.bw_fixed > ( ponBWInfo.RemainBW+ onuBWInfo.FinalUplinkBandwidth_fixed + onuBWInfo.FinalUplinkBandwidth_gr) )
			{
				return ( V2R1_Parter_EXCEED_RANGE );
			}
		}
	}
	
	/*End:for onu swap by jinhl@2013-04-27*/
    return OnuMgt_SetOnuBW(DstPonPortIdx, DstOnuIdx, &onuSrcBW);
}

int ResumeOnuUplinkBW( short int PonPortIdx, short int OnuIdx )
{
    return CopyOnuUplinkBW(PonPortIdx, OnuIdx, PonPortIdx, OnuIdx, OLT_COPYFLAGS_COVERSYNC);
}

int CopyOnuDownlinkBW(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iSrcEntry;
    OnuLLIDTable_S *pLlidCfg;
    ONU_bw_t onuSrcBW;
	int iRlt = 0;

    /* 目前，只支持Cover拷贝模式 */
    VOS_MemZero(&onuSrcBW, sizeof(onuSrcBW));
    iSrcEntry = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    
	ONU_MGMT_SEM_TAKE;
    pLlidCfg = &(OnuMgmtTable[iSrcEntry].LlidTable[0]);
    onuSrcBW.bw_gr        = pLlidCfg->DownlinkBandwidth_gr;
    onuSrcBW.bw_be        = pLlidCfg->DownlinkBandwidth_be;
    onuSrcBW.bw_class     = pLlidCfg->DownlinkClass;
    onuSrcBW.bw_delay     = pLlidCfg->DownlinkDelay;
    if ( OLT_CFG_DIR_DOWNLINK & pLlidCfg->BandWidthIsDefault )
    {
        /* 拷贝默认带宽 */
        onuSrcBW.bw_direction = OLT_CFG_DIR_UNDO | OLT_CFG_DIR_DOWNLINK;
    }
    else
    {
        /* 拷贝指定带宽 */
        onuSrcBW.bw_direction = OLT_CFG_DIR_DOWNLINK;
    }
	ONU_MGMT_SEM_GIVE;
    onuSrcBW.bw_direction |= OLT_CFG_DIR_BY_ONUID;

	/*Begin:for onu swap by jinhl@2013-04-27*/
	#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
    PonPortBWInfo_S ponBWInfo;
	ONUBWInfo_S  onuBWInfo;
	
    UpdateProvisionedBWInfo(DstPonPortIdx);
    iRlt = OnuMgtAdv_GetBWInfo(DstPonPortIdx, DstOnuIdx, &ponBWInfo, &onuBWInfo);
	if(VOS_OK == iRlt)
	{
	    if( onuSrcBW.bw_gr > ( ponBWInfo.DownlinkRemainBw + onuBWInfo.FinalDownlinkBandwidth_gr) )
		{
			return ( V2R1_Parter_EXCEED_RANGE );
		}
		
		  
	}
	#endif
	/*End:for onu swap by jinhl@2013-04-27*/
    return OnuMgt_SetOnuBW(DstPonPortIdx, DstOnuIdx, &onuSrcBW);
}

int ResumeOnuDownlinkBW( short int PonPortIdx, short int OnuIdx )
{
    return CopyOnuDownlinkBW(PonPortIdx, OnuIdx, PonPortIdx, OnuIdx, OLT_COPYFLAGS_COVERSYNC);
}

int ReleaseOnuUplinkBW( short int PonPortIdx, short int OnuIdx )
{
    ONU_bw_t onuBW;

    VOS_MemZero(&onuBW, sizeof(onuBW));
    onuBW.bw_gr        = ONU_MIN_BW;
    onuBW.bw_be        = ONU_MIN_BW;
    onuBW.bw_fixed     = 0;
    onuBW.bw_class     = 0;
    onuBW.bw_delay     = V2R1_DELAY_HIGH;
    onuBW.bw_direction = OLT_CFG_DIR_INACTIVE | OLT_CFG_DIR_UPLINK;
    return OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
}

int ReleaseOnuDownlinkBW( short int PonPortIdx, short int OnuIdx )
{
    ONU_bw_t onuBW;

    VOS_MemZero(&onuBW, sizeof(onuBW));
    onuBW.bw_direction = OLT_CFG_DIR_INACTIVE | OLT_CFG_DIR_DOWNLINK;
    return OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &onuBW);
}

int  InActiveOnuUplinkBW( short int PonPortIdx, short int OnuIdx )
{
    int iRlt;

    OLT_LOCALID_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    iRlt = ReleaseOnuUplinkBW(PonPortIdx, OnuIdx);

    return iRlt;
}

int  InActiveOnuDownlinkBW( short int PonPortIdx, short int OnuIdx )
{
    int iRlt;

    OLT_LOCALID_ASSERT(PonPortIdx);
    ONU_ASSERT(OnuIdx);

    iRlt = ReleaseOnuDownlinkBW(PonPortIdx, OnuIdx);
    
    return iRlt;
}

int  ActiveOnuUplinkBW( short int PonPortIdx, short int OnuIdx )
{
	/*short int PonChipType, PonChipVer;
	short int onu_id;
	short int ret;*/
	OnuLLIDTable_S *pLLIDCfg;
    int onu_entry;
    int iRlt;

	CHECK_ONU_RANGE 

#if 0
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
	
	onu_id = GetLlidActivatedByOnuIdx( PonPortIdx, OnuIdx );
	if(onu_id == INVALID_LLID ) return( RERROR );
#endif
	
	if( EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf("   set %s/port%d onu %d uplink bandwidth\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
	
	/*if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE ) return( RERROR );*/
	/*if(GetOnuOperStatus_1(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )return( RERROR );*/

    onu_entry = PonPortIdx * MAXONUPERPON + OnuIdx;	
	pLLIDCfg = &(OnuMgmtTable[onu_entry].LlidTable[0]);
    
	UpdateProvisionedBWInfo(PonPortIdx);
	PonPortTable[PonPortIdx].ActiveBW -= pLLIDCfg->ActiveUplinkBandwidth;
	/* modified by chenfj 2007-03-25
	#3907 问题单: 关于ONU带宽分配的两个问题
	原来判断时，激活的带宽只能 小于最大带宽；改为激活的带宽不能大于最大带宽
	*/
	if(( (PonPortTable[PonPortIdx].ActiveBW + pLLIDCfg->UplinkBandwidth_gr) > (PonPortTable[PonPortIdx].MaxBW) ) 
			/*|| ( PonPortTable[PonPortIdx].RemainBW < OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr)*/) 
	{
/* B--remed by liwei056@2009-11-12 for D9243 */
#if 0                    
		/* send trip to NMS */
		if( PonPortTable[PonPortIdx].BWExceedFlag == FALSE )
		{
			PonPortTable[PonPortIdx].BWExceedFlag = TRUE;
			if( PonPortTable[PonPortIdx].AlarmMask == 0 )
            { /* this should be modified later*/
				llidActBWExceeding_EventReport( onuIdToOnuIndex(PonPortIdx, OnuIdx+1), 1, 1, 1 ); 	/* added by xieshl 20091111, 带宽不足时告警 */
			}
		}	
#endif
/* E--remed by liwei056@2009-11-12 for D9243 */
		return( V2R1_EXCEED_RANGE );
	}
	else
    {   
#if 1                    
        iRlt = ResumeOnuUplinkBW(PonPortIdx, OnuIdx);
        if ( OLT_CALL_ISERROR(iRlt) )
        {
            if ( (iRlt > OLT_ERR_DBA_END) && (iRlt < OLT_ERR_DBA_BEGIN) )
            {
                iRlt = OLT_ERR_DBA_BEGIN - iRlt;
            }
			/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            iRlt = Pon_PLATO_ErrRet(iRlt, PonPortIdx);
			#if 0
            switch (iRlt)
            {
                case PLATO3_ECODE_MIN_GREATER_THAN_MAX:
                case PLATO3_ECODE_MIN_TOTAL_TOO_LARGE:
                case PLATO3_ECODE_ILLEGAL_GR_BW:
                case PLATO3_ECODE_ILLEGAL_BE_BW:
                case PLATO3_ECODE_ILLEGAL_FIXED_BW:
                   iRlt = V2R1_EXCEED_RANGE;
                   break;
                case PLATO3_ECODE_UNKNOWN_LLID:
                case PLATO3_ECODE_TOO_MANY_LLIDS:
                case PLATO3_ECODE_ILLEGAL_LLID:
                case PAS_ONU_NOT_AVAILABLE:    
                   iRlt = V2R1_ONU_OFF_LINE;
                   break;
                default:
                   iRlt = -1;
            }
			#endif
            /*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            return iRlt;
        }
#else
		if( PonChipType == PONCHIP_PAS )
		{
			if( PonPortTable[PonPortIdx].DBA_mode == OLT_EXTERNAL_DBA )
#ifdef PLATO_DBA_V3
			{	/* external DBA algorithm */
				PLATO3_SLA_t  V3_SLA;
				short DBA_error_code = 0;
				int Bw;
	
				if( OnuMgmtTable[onu_entry].LlidTable[0].UplinkClass > PRECEDENCE_OF_FLOW_7 )
					V3_SLA.class = OnuConfigDefault.UplinkClass;
				else
					V3_SLA.class = OnuMgmtTable[onu_entry].LlidTable[0].UplinkClass;
					

				V3_SLA.fixed_packet_size = 128;
				
				Bw = OnuMgmtTable[ onu_entry ].LlidTable[0].UplinkBandwidth_fixed ;			
				V3_SLA.fixed_bw = Bw / BITS_PER_SECOND_1M; 
				Bw = Bw % BITS_PER_SECOND_1M;
				V3_SLA.fixed_bw_fine = Bw / BITS_PER_SECOND_64K;  
				if( ( Bw % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.fixed_bw_fine ++;

				Bw = ((OnuMgmtTable[ onu_entry ].LlidTable[0].UplinkBandwidth_gr) * Bata_ratio)/1000 ;
				V3_SLA.max_gr_bw =  Bw / BITS_PER_SECOND_1M; 
				Bw = Bw % BITS_PER_SECOND_1M;
				V3_SLA.max_gr_bw_fine = Bw / BITS_PER_SECOND_64K;  
				if( ( Bw % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.max_gr_bw_fine ++;
				
				Bw = ((OnuMgmtTable[ onu_entry ].LlidTable[0].UpLinkBandwidth_be) * Bata_ratio)/1000 ;
				V3_SLA.max_be_bw = Bw  / BITS_PER_SECOND_1M; 
				Bw = Bw % BITS_PER_SECOND_1M;
				V3_SLA.max_be_bw_fine = Bw / BITS_PER_SECOND_64K;  
				if( ( Bw % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.max_be_bw_fine++;

				if(V3_SLA.max_be_bw == ( V3_SLA.fixed_bw + V3_SLA.max_gr_bw ))
				{
					if( V3_SLA.max_be_bw_fine < (V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine ) )
					{
						V3_SLA.max_be_bw_fine = V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine;
					}
				}
				else if(V3_SLA.max_be_bw < ( V3_SLA.fixed_bw + V3_SLA.max_gr_bw ))
				{
					V3_SLA.max_be_bw = V3_SLA.fixed_bw + V3_SLA.max_gr_bw;
					V3_SLA.max_be_bw_fine = V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine;
				}
				
				ret = PLATO3_set_SLA( PonPortIdx,  onu_id, (PLATO3_SLA_t *)&V3_SLA, (short *)&DBA_error_code) ;
				if( ret != PAS_EXIT_OK )
				{
					ret = PLATO3_set_SLA( PonPortIdx,  onu_id, (PLATO3_SLA_t *)&V3_SLA, (short *)&DBA_error_code) ;
					if( ret != PAS_EXIT_OK )
					{
						if( EVENT_REGISTER == V2R1_ENABLE)
						sys_console_printf("\r\n  set %s/port%d onu %d uplink bandwidth %dkbit/s err, errId %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr, ret);
						return( RERROR );
					}
				}
				else
                {
                    /* B--added by liwei056@2010-1-13 for D9425 */
                    if ( DBA_error_code != PLATO3_ECODE_NO_ERROR )
                    {
    				/*if( EVENT_REGISTER == V2R1_ENABLE)
    					sys_console_printf("\r\n  onu %d/%d/%d up-bw DBA_error_code=%d\r\n", GetCardIdxByPonChip(PonPortIdx)+1, GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), DBA_error_code);*/
                        return DBA_error_code;
                    }
                    else
                    /* E--added by liwei056@2010-1-13 for D9425 */
                    {
    					if( EVENT_REGISTER == V2R1_ENABLE)
    						sys_console_printf("\r\n  set %s/port%d onu %d uplink bandwidth %dkbit/s OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr);
    					OnuMgmtTable[onu_entry].LlidTable[0].ActiveUplinkBandwidth = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr;
    					return( ROK );
    				}
                }
                    
			}
#else
			{	/* external DBA algorithm */
				PLATO2_SLA_t  SLA;
				short DBA_error_code;
				int Bw;
				/*Bw = bw; only for test */
				
				/*added by chenfj 2006-12-21 
	    			   设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参数
				*/
				/*
				SLA.class = 2;
				SLA.delay = 1;
				*/
				if( OnuMgmtTable[onu_entry].LlidTable[0].UplinkClass > PRECEDENCE_OF_FLOW_7 )
					SLA.class = OnuConfigDefault.UplinkClass;
				else
					SLA.class = OnuMgmtTable[onu_entry].LlidTable[0].UplinkClass;

				if(( OnuMgmtTable[onu_entry].LlidTable[0].UplinkDelay == V2R1_DELAY_HIGH )
					|| ( OnuMgmtTable[onu_entry].LlidTable[0].UplinkDelay== V2R1_DELAY_LOW ))
					SLA.delay = OnuMgmtTable[onu_entry].LlidTable[0].UplinkDelay;
				else
					SLA.delay = OnuConfigDefault.UplinkDelay;

				/* modified by chenfj 2007-7-16
				增加一个带宽比例因子,用于测试带宽精度
				*/
				Bw = ((OnuMgmtTable[onu_entry].LlidTable[0].UplinkBandwidth_gr) * Bata_ratio)/1000 ;
				SLA.max_gr_bw =  Bw / BITS_PER_SECOND_1M; 
				Bw = Bw % BITS_PER_SECOND_1M;
				SLA.max_gr_bw_fine = Bw / BITS_PER_SECOND_64K;  
				if( ( Bw % BITS_PER_SECOND_64K ) != 0 ) SLA.max_gr_bw_fine ++;
				/*added by chenfj 2006-12-21 
	    			   设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参数
				*/
				/*
				SLA.max_be_bw = SLA.max_gr_bw + 1;   
				SLA.max_be_bw_fine = SLA.max_gr_bw_fine  ; 
				*/

				/* modified by chenfj 2007-7-16
				增加一个带宽比例因子,用于测试带宽精度
				*/
				Bw = ((OnuMgmtTable[onu_entry].LlidTable[0].UpLinkBandwidth_be) * Bata_ratio)/1000 ;
				SLA.max_be_bw = Bw  / BITS_PER_SECOND_1M; 
				Bw = Bw % BITS_PER_SECOND_1M;
				SLA.max_be_bw_fine = Bw / BITS_PER_SECOND_64K;  
				if( ( Bw % BITS_PER_SECOND_64K ) != 0 ) SLA.max_be_bw_fine++;

				if( SLA.max_be_bw < SLA.max_gr_bw )
				{
					SLA.max_be_bw = SLA.max_gr_bw;
					SLA.max_be_bw_fine = SLA.max_gr_bw_fine;
				}
				/*sys_console_printf("onu %d/%d bandwidth setting,gr=%dM%dKB, be=%dM%dKB\r\n", PonPortIdx, (OnuIdx+1), SLA.max_gr_bw,(SLA.max_gr_bw_fine*BITS_PER_SECOND_64K), SLA.max_be_bw, (SLA.max_be_bw_fine * BITS_PER_SECOND_64K ) );*/
				/*
				SLA.max_gr_bw =  PonPortTable[PonPortIdx].DefaultOnuBW / BITS_PER_SECOND_1M;				
				SLA.max_gr_bw_fine = 0;  
				SLA.max_be_bw = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr / BITS_PER_SECOND_1M;
				Bw = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr % BITS_PER_SECOND_1M;
				SLA.max_be_bw_fine =  Bw / BITS_PER_SECOND_64K;; 
				*/
				ret = PLATO2_set_SLA( PonPortIdx,  onu_id, (PLATO2_SLA_t *)&SLA, (short *)&DBA_error_code) ;
				if( ret != PAS_EXIT_OK )
				{
					ret = PLATO2_set_SLA( PonPortIdx,  onu_id, (PLATO2_SLA_t *)&SLA, (short *)&DBA_error_code) ;
					if( ret != PAS_EXIT_OK )
					{
						if( EVENT_REGISTER == V2R1_ENABLE)
						sys_console_printf("\r\n  set %s/port%d onu %d uplink bandwidth %dkbit/s err, errId %d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), OnuMgmtTable[onu_entry].LlidTable[0].UplinkBandwidth_gr, ret);
						return( RERROR );
					}
				}
				else
                {
					if( EVENT_REGISTER == V2R1_ENABLE)
						sys_console_printf("\r\n  set %s/port%d onu %d uplink bandwidth %dkbit/s OK\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), OnuMgmtTable[onu_entry].LlidTable[0].UplinkBandwidth_gr);
					OnuMgmtTable[onu_entry].LlidTable[0].ActiveUplinkBandwidth = OnuMgmtTable[onu_entry].LlidTable[0].UplinkBandwidth_gr;
					/*PonPortTable[PonPortIdx].ActiveBW += OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr;*/
					/*PonPortTable[PonPortIdx].RemainBW -= OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr;*/
					return( ROK );
				}
			}
#endif
			else
            { /* internal DBA algorithm */
			}
		}
		else
        { /* other pon chip handler */
		}
#endif
	}
	
	return( ROK );
}


int  ActiveOnuDownlinkBW( short int PonPortIdx, short int OnuIdx )
{
#if ( EPON_MODULE_ONU_DOWNBW_STRICT_LIMIT == EPON_MODULE_YES )
	/*short int PonChipType;
	short int PonChipVer;
	short int onu_id;
	short int ret;*/
    int onu_entry;
	OnuLLIDTable_S *pLLIDCfg;

	CHECK_ONU_RANGE

#if 0
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
	
	onu_id = GetLlidActivatedByOnuIdx(  PonPortIdx,  OnuIdx);
	if( onu_id == INVALID_LLID ) return ( RERROR );
#endif

	/* no policer */
	/*
	if( downlinkBWlimit == V2R1_DISABLE ) 
		return( ROK );
	*/
	if( EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" set %s/port%d onu %d Downlink bandwidth\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );
	
	/*if( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE ) return( ERROR );*/
	/*if(GetOnuOperStatus_1(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_DOWN )return( RERROR );*/
	
    onu_entry = PonPortIdx * MAXONUPERPON + OnuIdx;	
	pLLIDCfg = &(OnuMgmtTable[onu_entry].LlidTable[0]);
    
	UpdateProvisionedBWInfo(PonPortIdx);
	PonPortTable[PonPortIdx].DownlinkActiveBw -= pLLIDCfg->ActiveDownlinkBandwidth;
	
	if(( (PonPortTable[PonPortIdx].DownlinkActiveBw + pLLIDCfg->DownlinkBandwidth_gr ) >  PonPortTable[PonPortIdx].DownlinkMaxBw ) 
			/*|| ( PonPortTable[PonPortIdx].DownlinkRemainBw < OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].DownlinkBandwidth_gr)*/) 
	{			
/* B--remed by liwei056@2009-11-12 for D9243 */
#if 0                    
			/* send trip to NMS */
			if( PonPortTable[PonPortIdx].DownlinkBWExceedFlag == FALSE )
			{
				PonPortTable[PonPortIdx].DownlinkBWExceedFlag = TRUE;
				if( PonPortTable[PonPortIdx].AlarmMask == 0 )
                { /* this is should be modified later*/
					llidActBWExceeding_EventReport( onuIdToOnuIndex(PonPortIdx, OnuIdx+1), 1, 1, 1 ); 	/* added by xieshl 20091111, 带宽不足时告警 */
				}
			}	
#endif
/* E--remed by liwei056@2009-11-12 for D9243 */
			return( V2R1_EXCEED_RANGE );
	}
	else
#endif
    {
#if 1                    
        return ResumeOnuDownlinkBW(PonPortIdx, OnuIdx);
#else
		if( PonChipType == PONCHIP_PAS )
		{
			/* modified for PAS-SOFT V5.1
			PonChipVer = V2R1_GetPonChipVersion(PonPortIdx );*/
			if((PonChipVer != PONCHIP_PAS5001 ) && (PonChipVer != PONCHIP_PAS5201 )) return (RERROR );

			if(PonChipVer== PONCHIP_PAS5001 )
			{
				PON_policer_t policer_id;
				PON_policing_struct_t  policing_struct;

				policer_id = PON_POLICER_DOWNSTREAM_TRAFFIC;

				policing_struct.maximum_bandwidth = OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr;
				if(policing_struct.maximum_bandwidth > MAX_DOWNLINK_SPEED)
					policing_struct.maximum_bandwidth = MAX_DOWNLINK_SPEED;
				/*Real_bw = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].DownlinkBandwidth_gr;
				Config_bw = Real_bw /(1000000 - Real_bw)  ;
				sys_console_printf("\r\n config bw %d \r\n", Config_bw);
				Config_bw = Config_bw * 1000000;			
				sys_console_printf("\r\n config bw %d %d \r\n", Config_bw, Real_bw );
				*/
				/* modified by chenfj 2006/11/28
				 问题单#3267: 在ONU节点下学不到OLT上的千兆口发下来的源MAC,onu收不到OLT发下来的包 */
				policing_struct.maximum_burst_size = 5580000;/* 3*1024;   max 8388480 */
				policing_struct.high_priority_frames_preference = DISABLE;
				policing_struct.short_frames_preference = DISABLE;
				
				if( downlinkBWlimit == V2R1_ENABLE )
					ret = PAS_set_policing_parameters_v4( PonPortIdx, onu_id,  policer_id, ENABLE, &policing_struct );
				else ret = PAS_set_policing_parameters_v4( PonPortIdx, onu_id,  policer_id, DISABLE, &policing_struct );

				if( ret!= PAS_EXIT_OK )
				{
					if( EVENT_REGISTER == V2R1_ENABLE )
					sys_console_printf("\r\n  error %d: %s/port%d onu %d downlink BW %dkbit/s can't set\r\n", ret,CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr);
					return( RERROR );
				}
				else
                {
					if( EVENT_REGISTER == V2R1_ENABLE )
						sys_console_printf(" set %s/port%d onu %d downlink BW %dkbit/s ok\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr);
					OnuMgmtTable[onu_entry].LlidTable[0].ActiveDownlinkBandwidth = OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr;
					/*PonPortTable[PonPortIdx].DownlinkActiveBw += OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].DownlinkBandwidth_gr;
					PonPortTable[PonPortIdx].DownlinkRemainBw -= OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].DownlinkBandwidth_gr;*/
					return( ROK );
				}
			}

            /* modified for PAS-SOFT V5.1*/
			else if( PonChipVer == PONCHIP_PAS5201 )
			{
				PON_policer_t  policer_id;
				PON_policing_parameters_t  policing_param;

				policer_id = PON_POLICER_DOWNSTREAM_TRAFFIC;

				policing_param.maximum_bandwidth = OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr;
				policing_param.maximum_burst_size = downlinkBWlimitBurstSize;/* 3*1024;   max 16777215 */
				policing_param.high_priority_frames_preference = downlinkBWlimitPreference;
				/*policing_param.short_frames_preference = DISABLE;*/

				if( policing_param.maximum_bandwidth  >  PAS5201_MAX_DOWNLINK_SPEED )
					policing_param.maximum_bandwidth  = PAS5201_MAX_DOWNLINK_SPEED;
				
				if( downlinkBWlimit == V2R1_ENABLE )
					ret = PAS_set_policing_parameters(PonPortIdx, onu_id, policer_id, ENABLE, policing_param);
				else ret = PAS_set_policing_parameters(PonPortIdx, onu_id, policer_id, DISABLE, policing_param);

				if( ret!= PAS_EXIT_OK )
				{
					if( EVENT_REGISTER == V2R1_ENABLE )
					sys_console_printf("\r\n  error %d: %s/port%d onu %d downlink BW %dkbit/s can't set\r\n", ret,CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr);
					return( RERROR );
				}
				else
                {
					if( EVENT_REGISTER == V2R1_ENABLE )
						sys_console_printf(" set %s/port%d onu %d downlink BW %dkbit/s ok\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1),OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr);
					OnuMgmtTable[onu_entry].LlidTable[0].ActiveDownlinkBandwidth = OnuMgmtTable[onu_entry].LlidTable[0].DownlinkBandwidth_gr;
					/*PonPortTable[PonPortIdx].DownlinkActiveBw += OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].DownlinkBandwidth_gr;
					PonPortTable[PonPortIdx].DownlinkRemainBw -= OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].LlidTable[0].DownlinkBandwidth_gr;*/
					return( ROK );
				}
			}
		}
		else
        { /* other pon chip type handler */
		}
#endif
	}

	return ( ROK );
}
	
/* B--added by liwei056@2011-1-26 for D11957 */
int  ActiveOnuDefaultBW( short int PonPortIdx, short int OnuIdx, unsigned int ActDir )
{
    int iRlt;
    ONU_bw_t default_bw;

    VOS_MemZero(&default_bw, sizeof(ONU_bw_t));
    default_bw.bw_class = (unsigned int)-1;
    default_bw.bw_delay = (unsigned int)-1;
    default_bw.bw_direction = ActDir | OLT_CFG_DIR_UNDO | OLT_CFG_DIR_FORCE | OLT_CFG_DIR_BY_ONUID;

    if ( OLT_CFG_DIR_BOTH == ActDir )
    {
        if(CheckOnuActionIsValid(PonPortIdx, OnuIdx, ActDir) == VOS_OK)
            default_bw.bw_direction |= OLT_CFG_DIR_DO_ACTIVE;

		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &default_bw.bw_gr, &default_bw.bw_be);
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &default_bw.bw_fixed, &default_bw.bw_actived);
    }
    else
    {
        if ( OLT_CFG_DIR_UPLINK & ActDir )
        {
            if(CheckOnuActionIsValid(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK) == VOS_OK)
                default_bw.bw_direction |= OLT_CFG_DIR_DO_ACTIVE;
            
			GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &default_bw.bw_gr, &default_bw.bw_be);
        }
        else
        {
            if(CheckOnuActionIsValid(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK) == VOS_OK)
                default_bw.bw_direction |= OLT_CFG_DIR_DO_ACTIVE;
            
			GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &default_bw.bw_gr, &default_bw.bw_be);
        }
    }

    iRlt = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &default_bw);

    return iRlt;
}
/* E--added by liwei056@2011-1-26 for D11957 */

int SetDefaultBwForAllOnu(short int PonPortIdx )
{
	short int PonChipType, PonChipVer;
	PLATO2_SLA_t  SLA;
	short DBA_error_code;
	short int ret;
	SLA.class = 2;
	SLA.delay = 1;
	SLA.max_gr_bw =5; /* only for test */
	SLA.max_gr_bw_fine = 6;  
	SLA.max_be_bw = 100;   
	SLA.max_be_bw_fine = 2; 

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);

	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if( PonChipType == PONCHIP_PAS )
	{
	    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		ret = PLATO2_set_SLA( PonPortIdx,  GW1G_PAS_BROADCAST_LLID, (PLATO2_SLA_t *)&SLA, (short *)&DBA_error_code) ;
		if( ret == PLATO2_ERR_OK )
        {
			sys_console_printf("\r\n set Dynamic Bandwidth ok %d  error_code %d \r\n", ret, DBA_error_code );
			return( ROK );
		}
		else
        {
			sys_console_printf("\r\n set Dynamic Bandwidth err %d \r\n", ret);
			return( RERROR );
		}
	}
	else
    { /* other pon chip handler */
		return( RERROR );
	}

    return( RERROR );
}



#ifdef ONU_SHOW
#endif

unsigned char DBA_name[128];
unsigned char NameSize;
unsigned char DBA_Version[128];
unsigned char VersionSize;

int  GetDBAInfo( short int PonPortIdx )
{
	int ret;
	OLT_DBA_version_t stDBAinfo;
	
	CHECK_PON_RANGE

	NameSize = 10;
	VersionSize = 10;

	VOS_MemSet(DBA_name, 0, 128);
	VOS_MemSet(DBA_Version, 0,128 );

#if 1
    ret = OLT_GetDBAVersion(PonPortIdx, &stDBAinfo);
    if ( OLT_CALL_ISOK(ret) )
    {
        ret = 0;

        VOS_ASSERT(NameSize < sizeof(stDBAinfo.szDBAname));
        VOS_ASSERT(VersionSize < sizeof(stDBAinfo.szDBAversion));
        
        VOS_StrnCpy(DBA_name, stDBAinfo.szDBAname, NameSize);
        VOS_StrnCpy(DBA_Version, stDBAinfo.szDBAversion, VersionSize);
    }
#else
#ifdef PLATO_DBA_V3
		ret = PLATO3_get_info( PonPortIdx, DBA_name, NameSize, DBA_Version, VersionSize );
#else
		ret = PLATO2_get_info( PonPortIdx, DBA_name, NameSize, DBA_Version, VersionSize );
#endif
#endif
	if(( ret == 0 ) && (EVENT_DEBUG == V2R1_ENABLE))
    {
		sys_console_printf("\r\nGet DBA Info ok \r\n");
		/*sys_console_printf(" DBA name: %s ,  name len : %d \r\n", DBA_name, NameSize);
		sys_console_printf(" DBA version: %s, version len: %d \r\n", DBA_Version, VersionSize );*/
		ret = ROK;
	}
	else if( ret != 0 )
    {
		sys_console_printf("\r\nGet pon%d/%d DBA Info err, RetCode : %d (GetDBAInfo()) \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ret);
		ret = RERROR;
	}

	return( ret );
}
/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int  GetSLAInfo( short int PonPortIdx, short int OnuIdx )
{
	short int ret;
	/*short int DBA_error_code;*/
#if 1
    ONU_SLA_INFO_t SLA_Info;
#else    
	short int llid;
#endif		
	
	CHECK_ONU_RANGE

#if 1
    ret = OnuMgt_GetOnuSLA(PonPortIdx, OnuIdx, &SLA_Info);
#else    
	llid = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
	if( llid == INVALID_LLID ) return( RERROR );

    {
#ifdef PLATO_DBA_V3
	PLATO3_SLA_t  SLA;
	short int  DBA_error_code;
	ret = PLATO3_get_SLA( PonPortIdx, llid, &SLA, &DBA_error_code);
#else
	PLATO2_SLA_t  SLA;
	short int  DBA_error_code;
	ret = PLATO2_get_SLA( PonPortIdx, llid, &SLA, &DBA_error_code);
#endif
    }
#endif		

	if( ret == PAS_EXIT_OK )
    {
#if 0
#ifdef PLATO_DBA_V3
		PLATO3_SLA_t SLA;

        SLA = SLA_Info.SLA.SLA3;
#else
		PLATO2_SLA_t  SLA;

        SLA = SLA_Info.SLA.SLA2;
#endif
        DBA_error_code = SLA_Info.DBA_ErrCode;

        sys_console_printf("\r\nGet SLA Info ok \r\n");
		sys_console_printf(" SLA.class = %d \r\n", SLA.class );
#ifdef PLATO_DBA_V3
		sys_console_printf("SLA.fixed_packet_size = %d\r\n",SLA.fixed_packet_size );
		sys_console_printf("SLA.fixed_bw = %d\r\n",SLA.fixed_bw );
		sys_console_printf("SLA.fixed_bw_fine = %d\r\n",SLA.fixed_bw_fine );
#else
		sys_console_printf(" SLA.delay = %d \r\n", SLA.delay );
#endif			
		sys_console_printf(" SLA.max_gr_bw = %d \r\n", SLA.max_gr_bw );
		sys_console_printf(" SLA.max_gr_bw_fine = %d \r\n", SLA.max_gr_bw_fine );
		sys_console_printf(" SLA.max_be_bw = %d \r\n", SLA.max_be_bw );
		sys_console_printf(" SLA.max_be_bw_fine = %d \r\n", SLA.max_be_bw_fine );
		sys_console_printf(" error_code %d \r\n", DBA_error_code );	
#endif
        Pon_Show_SLAInfo(PonPortIdx, SLA_Info);
		ret = ROK;
	}
	else
    {
		sys_console_printf("\r\nGet SLA Info err, RetCode : %d (GetSLAInfo())\r\n", ret);
		ret = RERROR;
	}
    
	return( ret );
}
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
int SetOnuUsedFlag( short int PonPortIdx, short int OnuIdx )
{
	CHECK_ONU_RANGE

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[PonPortIdx * MAXONUPERPON  + OnuIdx ].UsedFlag = ONU_USED_FLAG;
	ONU_MGMT_SEM_GIVE;

	return( ROK );

}


int ClearOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx )
{
	short int OnuEntry;
    OnuLLIDTable_S *pLLID;
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx; 
    pLLID = &OnuMgmtTable[OnuEntry].LlidTable[LlidIdx - 1];
	
	ONU_MGMT_SEM_TAKE;
	pLLID->EntryStatus = LLID_ENTRY_NOT_READY;
	pLLID->LlidType = LLIDTYPE_ETHLINK;
	pLLID->MaxMAC = 0;
	pLLID->UplinkBandwidth_gr = 0;
	pLLID->DownlinkBandwidth_gr = 0;
#ifdef PLATO_DBA_V3
	pLLID->UplinkBandwidth_fixed = 0;
#endif
	pLLID->llidOltBoard = 0;
	pLLID->llidOltPort = 0;
	pLLID->llidOnuBoard = 0;
	pLLID->llidOnuPort = 0;
	ONU_MGMT_SEM_GIVE;
    
	return( ROK );
}

int CreateOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx )
{
	short int OnuEntry;
    OnuLLIDTable_S *pLLID;
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx; 
    pLLID = &OnuMgmtTable[OnuEntry].LlidTable[LlidIdx - 1];
	
	ONU_MGMT_SEM_TAKE;
	pLLID->EntryStatus = LLID_ENTRY_NOT_READY;
	pLLID->LlidType = LLIDTYPE_ETHLINK;
	pLLID->MaxMAC = MaxMACDefault;
	GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &pLLID->UplinkBandwidth_gr, NULL);
	GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &pLLID->DownlinkBandwidth_gr, NULL);
#ifdef PLATO_DBA_V3
	pLLID->UplinkBandwidth_fixed = 0;
#endif
	pLLID->llidOltBoard = OnuMgmtTable[OnuEntry].Index.slot;
	pLLID->llidOltPort = OnuMgmtTable[OnuEntry].Index.port;
	pLLID->llidOnuBoard = 0;
	pLLID->llidOnuPort = 0;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

int InitOnuLlidEntry( short int PonPortIdx, short int OnuIdx, short int LlidIdx )
{
	short int OnuEntry;
    OnuLLIDTable_S *pLLID;
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx; 
    pLLID = &OnuMgmtTable[OnuEntry].LlidTable[LlidIdx];
	
	ONU_MGMT_SEM_TAKE;
	pLLID->EntryStatus = LLID_ENTRY_UNKNOWN;
	pLLID->LlidType = LLIDTYPE_ETHLINK;
	pLLID->Llid = INVALID_LLID;
	pLLID->MaxMAC = MaxMACDefault/*OnuConfigDefault.MaxMAC*/;

	pLLID->BandWidthIsDefault = OLT_CFG_DIR_BOTH;
	/*added by chenfj 2006-12-21 
   	设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
	*/
	GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &pLLID->UplinkBandwidth_gr, &pLLID->UplinkBandwidth_be);
	GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &pLLID->DownlinkBandwidth_gr, &pLLID->DownlinkBandwidth_be);
#ifdef PLATO_DBA_V3
	pLLID->UplinkBandwidth_fixed = 0;
#endif

	pLLID->UplinkClass = OnuConfigDefault.UplinkClass;
	pLLID->UplinkDelay = OnuConfigDefault.UplinkDelay;
	pLLID->DownlinkClass = OnuConfigDefault.DownlinkClass;
	pLLID->DownlinkDelay = OnuConfigDefault.DownlinkDelay;
	/* modified by chenfj 2006-10-30 
	OnuMgmtTable[i].LlidTable[j].UplinkBandwidth_gr = OnuConfigDefault.UplinkBandwidth;
	OnuMgmtTable[i].LlidTable[j].DownlinkBandwidth_gr = OnuConfigDefault.DownlinkBandwidth;
	*/
	pLLID->llidOltBoard = OnuMgmtTable[OnuEntry].Index.slot;
	pLLID->llidOltPort = OnuMgmtTable[OnuEntry].Index.port;
	pLLID->llidOnuBoard = 0;
	pLLID->llidOnuPort = 0;
	
	pLLID->OnuhandlerStatus = ONU_IDLE;
	pLLID->OnuEventRegisterTimeout = 0;
	
#ifdef	CTC_EXT_OID
	/*modified by wangxy 2007-03-19
	add CTC extention definition
	*/
	pLLID->llidCtcFecAbility = 1; /*unknown */
	pLLID->llidCtcFecMode = 1; /*unknown */
	pLLID->llidCtcEncrypCtrl = 2;/*init default value to 'disable'*/
#if 0
	pLLID->llidCtcDbaQuesetNum = 1;/*init queset array is null*/
	pLLID->llidCtcDbaQuesetCfgStatus =1;/*noop*/
	VOS_MemSet( pLLID->llidCtcDbaQueset, 0, sizeof(pLLID->llidCtcDbaQueset ) );
#endif
#endif
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

int DeleteOnuMacAddr( short int PonPortIdx, short int OnuIdx, short int LlidIdx, unsigned char *MacAddr )
{
	short int OnuEntry;
	short int active_records;
	short int llid;
	short int i;
	
	CHECK_ONU_RANGE

	if( MacAddr == NULL ) return( RERROR );

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	LlidIdx = 0;

	ONU_MGMT_SEM_TAKE;
	llid = OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].Llid;
	ONU_MGMT_SEM_GIVE;

#if 1
	if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table) ) ) return( RERROR );
#else
	if(( PAS_get_address_table(PonPortIdx, &active_records, Mac_addr_table)) != PAS_EXIT_OK ) return( RERROR );
#endif
	if( active_records == 0 ) return( ROK );

	for( i=0; i<active_records; i++)
	{
    	if( (Mac_addr_table[i].logical_port == llid)
            && MAC_ADDR_IS_EQUAL( MacAddr, Mac_addr_table[i].mac_address) )
		{
#if 1
			if( OLT_CALL_ISOK( OLT_RemoveMac( PonPortIdx, Mac_addr_table[i].mac_address ) ) ) 
#else
			if( PAS_remove_address_table_record( PonPortIdx, Mac_addr_table[i].mac_address ) == PAS_EXIT_OK ) 
#endif
				return( ROK );
			else
                return( RERROR );
		}
	}
    
	return( ROK );
}

int DeleteOnuMacAddrAll( short int PonPortIdx, short int OnuIdx, short int LlidIdx)
{
	short int OnuEntry;
	short int active_records;
	short int llid;
	short int i;	
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	LlidIdx = 0;

	ONU_MGMT_SEM_TAKE;
	llid = OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].Llid;
	ONU_MGMT_SEM_GIVE;

#if 1
	if( OLT_CALL_ISERROR( OLT_GetMacAddrTbl(PonPortIdx, &active_records, Mac_addr_table) ) ) return( RERROR );
#else
	if(( PAS_get_address_table(PonPortIdx, &active_records, Mac_addr_table)) != PAS_EXIT_OK) return( RERROR );
#endif
	if( active_records == 0 ) return( ROK );

	for( i=0; i<active_records; i++)
	{
		if( Mac_addr_table[i].logical_port == llid )
#if 1
			OLT_RemoveMac( PonPortIdx, Mac_addr_table[i].mac_address );
#else
			PAS_remove_address_table_record( PonPortIdx, Mac_addr_table[i].mac_address );
#endif
	}

    return( ROK );
}


/* add by chenfj 2006/09/19 */
/* #2604 问题telnet用户在串口pon节点下使用命令show current-onu，输出显示在串口上了*/
int ShowOnuEncryptInfoByVty( short int PonPortIdx, short int OnuIdx ,struct vty *vty)
{
	short int OnuEntry;
	/*short int Onu_llid;*/
#if 0
	PON_llid_parameters_t llid_parameters;
#else
    int iEncDir = 0, iKeyTime = 0, status = 0;
#endif
	
	CHECK_ONU_RANGE;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	vty_out( vty, "\r\n  onu %d/%d/%d encrypt Info\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	/*sys_console_printf(" -- encrypt enable:");
	(OnuMgmtTable[OnuEntry].EncryptEnable == V2R1_DISABLE ) ? sys_console_printf(" disable\r\n"):sys_console_printf(" enable\r\n");*/
	/*vty_out( vty, "    encrypt enable:%s\r\n", v2r1Enable[OnuMgmtTable[OnuEntry].EncryptEnable] );*/
#if 0
	if((Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx)) != INVALID_LLID)
		{
		/* modified by xieshl 20091215, 增加返回值和最大取值范围限制，防止数组越界 */
		if( PAS_get_llid_parameters ( PonPortIdx, Onu_llid, &llid_parameters ) != PAS_EXIT_OK )
		{
			VOS_ASSERT(0);
			return CMD_WARNING;
		}
        
    {
		if(llid_parameters.encryption_mode != PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)
		{
			if( (ULONG)(llid_parameters.encryption_mode+PON_ENCRYPTION_PURE) > 3 )
			{
				VOS_ASSERT(0);
				return CMD_WARNING;
			}
			vty_out(vty, "    encrypt started: started \r\n");
			vty_out( vty, "    encrypt direction:%s\r\n", v2r1EncryptDirection[llid_parameters.encryption_mode+PON_ENCRYPTION_PURE]);
			vty_out( vty, "    encrypt key change time:%d(s)\r\n", (OnuMgmtTable[OnuEntry].EncryptKeyTime/SECOND_1) );
			return(ROK);
		}
    }
#else
    /*问题单11646*/
    if ( 0 == OnuMgt_GetOnuEncryptParams(PonPortIdx, OnuIdx, &iEncDir, &iKeyTime, &status) )
    {
		if(iEncDir > PON_ENCRYPTION_PURE)
		{
			if( iEncDir > 3 )
			{
				VOS_ASSERT(0);
				return CMD_WARNING;
			}
            
			vty_out(vty, "    encrypt started: %s \r\n", v2r1Start[status]);
			vty_out( vty, "    encrypt direction:%s\r\n", v2r1EncryptDirection[iEncDir]);
			vty_out( vty, "    encrypt key change time:%d(s)\r\n", (iKeyTime / SECOND_1) );
			return(ROK);
		}
    }
#endif
	
	vty_out(vty, "    encrypt started:%s started\r\n", ((OnuMgmtTable[OnuEntry].EncryptStatus == V2R1_STARTED) ? "" : "not") );
	vty_out( vty, "    encrypt direction:%s\r\n", v2r1EncryptDirection[OnuMgmtTable[OnuEntry].EncryptDirection]);
	vty_out( vty, "    encrypt key change time:%d(s)\r\n", (OnuMgmtTable[OnuEntry].EncryptKeyTime/SECOND_1) );
	
	return( ROK );
}

int  ShowOnuSWUpdateInfoByVty( short int PonPortIdx, short int OnuIdx , struct vty *vty)
{
	unsigned int status;
    int OnuEntry;    
    
	CHECK_ONU_RANGE

	if(ThisIsValidOnu(PonPortIdx, OnuIdx ) != ROK )
	{
		vty_out(vty,  "\r\n  onu%d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
	}

	vty_out(vty, "\r\n  onu%d/%d/%d software update info\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));

    OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
        
    /* modified by xieshl 20091215, 增加返回值和最大取值范围限制，防止数组越界 */
	status = OnuMgmtTable[OnuEntry].SoftwareUpdateCtrl;
	if( (status != ONU_SW_UPDATE_ENABLE) && (status != ONU_SW_UPDATE_DISABLE) )
		status = 0;
	vty_out(vty, "    enable flag:%s\r\n" ,v2r1Enable[status]);
	/*(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].SoftwareUpdateCtrl == ONU_SW_UPDATE_ENABLE ) ? \
		sys_console_printf("enable\r\n"):sys_console_printf("disable\r\n");
		*/
	status = OnuMgmtTable[OnuEntry].SoftwareUpdateStatus;
	if( status > 4 /*ONU_SW_UPDATE_STATUS_FORBIDDEN*/ )
		status = 0;
	vty_out(vty, "    update status:%s\r\n", OnuSwUpdateStatus[status] );

	return( ROK );
}

int ShowOnuDeviceInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	short int OnuEntry;
	/*short int PonChipType;
	short int llid;*/
	/*short int count;*/
	/*unsigned char InfoString[256] = {0};*/
	unsigned char TempString[256];
	int ret;
	int status/*,ulOnuType=0*/;
	unsigned char AppPrefix[36], AppSuffix[36];
	unsigned char  AppPrefixLen=0,  AppSuffixLen = 0;
	unsigned char FwPrefix[36], FwSuffix[36];
	unsigned char  FwPrefixLen=0,  FwSuffixLen = 0;
	int  len = 0;
	PON_device_versions_t  device_versions_struct;	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	
	/* added by chenfj 2006/10/19
	     #2906问题建议不存在的ONU不要再显示信息了,另外ONU端口号没有做限制
	     */
	/*
	ret = CompTwoMacAddress((char *) OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, (char *)Invalid_Mac_Addr);
	if( ret == ROK ) 
		{
		vty_out(vty , "\r\nONU %d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
		return( RERROR );
		}
	ret = CompTwoMacAddress((char *)OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, (char *)Invalid_Mac_Addr1 );
	if( ret == ROK ) 
	*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	{
		vty_out(vty , "\r\n onu %d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );
		return( RERROR );
	}
	
	vty_out(vty, "\r\n onu %d/%d/%d Device Information list:\r\n\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), ( OnuIdx+1) );

	/*Basic */

	/*vty_out(vty, "device Type:%s \r\n", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));*/
	TempString[0] = 0;
	if( GetOnuModel( PonPortIdx, OnuIdx, TempString, &len) != ROK )
		VOS_StrCpy( TempString, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
	vty_out( vty, "device Type:%s ", TempString );
		
	vty_out(vty, "device desc:%s \r\n", GetDeviceDescString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
	vty_out(vty, "Mac addr:");
	{
		vty_out(vty, "%02x%02x.%02x%02x.%02x%02x\r\n\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
							OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
							OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
	}
	
	vty_out(vty, "Vendor Info:");
	ret = GetOnuVendorInfo(PonPortIdx, OnuIdx,  TempString, &len);
	if( ret != ROK )
		VOS_StrCpy( TempString, "-" );
	vty_out(vty,"%s\r\n", TempString );
			
	if ( OnuMgt_GetOnuPonVer ( PonPortIdx, OnuIdx, &device_versions_struct ) == 0)
	{
		vty_out(vty,"ponchip version: ");

        if ( device_versions_struct.pon_vendors & PON_VENDOR_PMC )
        {
			if (device_versions_struct.hardware_major == 0x1)
			{
				switch(device_versions_struct.hardware_minor)
				{
					case(0x0):
						vty_out(vty,"PAS6001-A,B");
						break;
					case(0x1):
						vty_out(vty,"PAS6001-N");
						break;
					case(0x2):
						vty_out(vty,"PAS6001-N M3");
						break;
					default:
						vty_out(vty,"PAS6001 Hardware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
						break;
				}
			} 
			else if (device_versions_struct.hardware_major == 0x6201)
			{
				switch(device_versions_struct.hardware_minor)
				{
					case(0x0):
						vty_out(vty,"PAS6201-A0");
						break;
					case(0x1):
						vty_out(vty,"PAS6201-A1");
						break;
					case(0x2):
						vty_out(vty,"PAS6201-A2");
						break;
					default:
						vty_out(vty,"PAS6201 Hradware minor version not supported (0x%hx)",device_versions_struct.hardware_minor);
						break;
				}
			}
			else
            {
                vty_out(vty,"PAS%X-A%u", device_versions_struct.hardware_major, device_versions_struct.hardware_minor);
            }         
        }
        else if ( device_versions_struct.pon_vendors & PON_VENDOR_TEKNOVUS )
        {
			if (device_versions_struct.hardware_major == 0x3715)
			{
				vty_out(vty,"TK3715-%X", device_versions_struct.hardware_minor);
			}
			else
            {
    			vty_out(vty,"TK%X-%X", device_versions_struct.hardware_major, device_versions_struct.hardware_minor);
            }         
        }
        else if ( device_versions_struct.pon_vendors & PON_VENDOR_INTEROP )
        {
            char *pcVer = &device_versions_struct.hardware_major;
        
  			vty_out(vty,"CS%c%cXX-%X", *pcVer, *(pcVer+1), device_versions_struct.hardware_minor);
        }
        else
        {
			vty_out(vty,"PON Hardware ChipVendor(0x%x) not supported (0x%hx, 0x%hx)", device_versions_struct.pon_vendors, device_versions_struct.hardware_major, device_versions_struct.hardware_minor);
        }
		vty_out(vty,"\r\n");
    }
	

	if( OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_NoLen > MAXSNLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_NoLen = MAXSNLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No[OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_NoLen] = '\0';
	vty_out(vty, "device SN:%s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No );

	if( OnuMgmtTable[OnuEntry].DeviceInfo.MadeInDateLen > MAXDATELEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.MadeInDateLen = MAXDATELEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.MadeInDate[OnuMgmtTable[OnuEntry].DeviceInfo.MadeInDateLen] = '\0';
	vty_out(vty, "product Date:%s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.MadeInDate );

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		unsigned int ver = OnuMgmtTable[OnuEntry].OAM_Ver;
		if( ver > 3 ) ver = 0;
		vty_out(vty,"Oam version:%d %s\r\n", OnuMgmtTable[OnuEntry].OAM_Ver, OAMVersion_s[ver]);
	}
	vty_out(vty, "\r\n");
	
	if( OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen > MAXDEVICENAMELEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen = MAXDEVICENAMELEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.DeviceName[OnuMgmtTable[OnuEntry].DeviceInfo.DeviceNameLen] = '\0';
	vty_out(vty, "onu name:%s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceName );

	if( OnuMgmtTable[OnuEntry].DeviceInfo.DeviceDescLen > MAXDEVICEDESCLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.DeviceDescLen = MAXDEVICEDESCLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.DeviceDesc[OnuMgmtTable[OnuEntry].DeviceInfo.DeviceDescLen] = '\0';
	vty_out(vty, "onu description:%s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceDesc );
	
	if( OnuMgmtTable[OnuEntry].DeviceInfo.LocationLen > MAXLOCATIONLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.LocationLen = MAXLOCATIONLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.Location[OnuMgmtTable[OnuEntry].DeviceInfo.LocationLen] = '\0';
	vty_out(vty, "onu location:%s\r\n\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.Location );

	/* Version */
	if( OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen > MAXBOOTVERSIONLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen = MAXBOOTVERSIONLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.BootVersion[OnuMgmtTable[OnuEntry].DeviceInfo.BootVersionLen] = '\0';
	VOS_Sprintf(&TempString[0], "Boot Ver:%s", OnuMgmtTable[OnuEntry].DeviceInfo.BootVersion );
	vty_out(vty, "%-34s", TempString );

	/* modified by chenfj 2008-10-28
	     若ONU 中有voip voice, 则将其从APP 版本号中分离出来,单独一行显示
	*/
	if( OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen > MAXSWVERSIONLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen = MAXSWVERSIONLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion[OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen] = '\0';
	VOS_MemCpy( TempString, OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion,OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen);
	TempString[OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen] = '\0';
	if(OnuEponAppHasVoiceApp(PonPortIdx,OnuIdx) != ROK)
		vty_out(vty, "Software Ver:%s\r\n", TempString );
	else
	{			
		ParsePrefixAndSuffixFromString(TempString,  AppPrefix,  &AppPrefixLen,  AppSuffix,  &AppSuffixLen);
		if( AppPrefixLen == 0 ) vty_out(vty, "Software Ver:%s\r\n", TempString );
		else vty_out(vty, "Software Ver:%s\r\n",  AppPrefix );
	}
	
	if( OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen > MAXHWVERSIONLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen = MAXHWVERSIONLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.HwVersion[OnuMgmtTable[OnuEntry].DeviceInfo.HwVersionLen] = '\0';
	VOS_Sprintf( &TempString[0], "Hardware Ver:%s", OnuMgmtTable[OnuEntry].DeviceInfo.HwVersion );
	vty_out(vty, "%-34s", TempString );

	/* modified by chenfj 2008-10-28
	     若ONU 中有TDM FPGA, 则将其从固件版本号中分离出来,单独一行显示
	*/
	if( OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen > MAXFWVERSIONLEN )
		OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen = MAXFWVERSIONLEN;
	OnuMgmtTable[OnuEntry].DeviceInfo.FwVersion[OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen] = '\0';
	VOS_MemCpy( TempString, OnuMgmtTable[OnuEntry].DeviceInfo.FwVersion,OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen);
	TempString[OnuMgmtTable[OnuEntry].DeviceInfo.FwVersionLen] = '\0';
	if(OnuFirmwareHasFpgaApp(PonPortIdx,OnuIdx) != ROK )
		vty_out(vty, "Firmware Ver:%s\r\n", TempString );
	else
	{
		ParsePrefixAndSuffixFromString(TempString,  FwPrefix,  &FwPrefixLen,  FwSuffix,  &FwSuffixLen);
		if(FwPrefixLen ==0) vty_out(vty, "Firmware Ver:%s\r\n", TempString );
		else vty_out(vty, "Firmware Ver:%s\r\n", FwPrefix );
	}

	if(AppSuffixLen != 0 )
	{
		VOS_Sprintf(&TempString[0], "Voice Ver:%s", AppSuffix);
		vty_out(vty,"%-34s\r\n",TempString);
	}
	if(FwSuffixLen != 0)
	{
		vty_out(vty,"Tdm Fpga:%r\n",FwSuffix);
	}
	vty_out(vty,"\r\n");

	
	/*Information */
	VOS_MemZero(&TempString[0], sizeof(TempString) );
	/* modified by chenfj 2008-2-20
		     使用扩展的ONU操作状态
		    */
	status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
	/* modified by xieshl 20091215, 增加返回值和最大取值范围限制，防止数组越界 */
	if( (status < 0) || (status > ONU_OPER_STATUS_POWERDOWN) )
		status = 0;
	VOS_Sprintf(&TempString[0], "opStatus:%s", OnuCurrentStatus[status] );
	vty_out(vty, "%-34s", TempString );
	
	VOS_StrCpy( TempString, "not started");
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		if(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].EncryptStatus == V2R1_STARTED)
			VOS_StrCpy( TempString, "started");
	}
	vty_out( vty, "encrypt started:%s\r\n", TempString );

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
		VOS_Sprintf( TempString, "Range:%dm", OnuMgmtTable[OnuEntry].RTT );
	else
		VOS_StrCpy( TempString, "Range:unknown" );
	vty_out(vty,"%-34s", TempString );

	status = OnuMgmtTable[OnuEntry].EncryptDirection;
	if( (status < 0) || (status > 3) )
		status = 0;
	vty_out(vty,"Encrypt direction:%s\r\n", v2r1EncryptDirection[status] );

	{
	ULONG /*CurTicks,*/ SysUpDelayed;
	ULONG days, hours,minutes, seconds;
	Date_S UpDate;
	
	/*CurTicks = VOS_GetTick();
	SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

	seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;
	days = 0;
	hours = 0;
	minutes = 0;*/
	SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
	days = SysUpDelayed /V2R1_ONE_DAY;
	hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
	minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
	seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

	GetOnuLaunchTime( PonPortIdx, OnuIdx, &UpDate);		

	if( GetOnuOperStatus (PonPortIdx,  OnuIdx) == ONU_OPER_STATUS_UP )
		VOS_Sprintf( TempString, "on-line time:%04u:%02u:%02u:%02u", days, hours, minutes, seconds );/* modified by xieshl 20100319, GWD网上问题9966 */
	else /*if( GetOnuOperStatus (PonPortIdx,  OnuIdx) != ONU_OPER_STATUS_UP )*/
		VOS_Sprintf( TempString, "off-line time:%04u:%02u:%02u:%02u", days, hours, minutes, seconds ); 
	vty_out( vty,"%-34s", TempString ); 

	vty_out(vty,"Encrypt Keytime:%d(s)\r\n",  (OnuMgmtTable[OnuEntry].EncryptKeyTime/SECOND_1) );

	VOS_Sprintf( TempString,"Start time: %04u-%02u-%02u, %02u:%02u:%02u", UpDate.year, UpDate.month, UpDate.day, UpDate.hour, UpDate.minute, UpDate.second );	
	vty_out(vty,"%-34s", TempString );	
	}
	status = GetOnuTrafficServiceEnable( PonPortIdx, OnuIdx );
	if( (status != V2R1_ENABLE) && (status != V2R1_DISABLE) )
		status = 0;
	vty_out(vty, "onu traffic service:%s\r\n", v2r1Enable[status] );

	VOS_Sprintf( TempString, "Up-Bandwidth:%d(kbit/s)", OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr );
	vty_out( vty,"%-34s", TempString );
	/* modified by chenfj 2008-10-27
	修改ONU设备信息显示格式,去掉laser on/off光参数,将下行带宽显示放到上行带宽显示的右侧;
	*/
	/*
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		VOS_MemZero(&TempString[0], sizeof(TempString) );
		sprintf(TempString,"Onu Laser On/Off time:%d/%d", OnuMgmtTable[OnuEntry].Laser_ON, OnuMgmtTable[OnuEntry].Laser_OFF );
		vty_out(vty,"%s\r\n",TempString );
		}
	else vty_out(vty,"\r\n");
	*/
	
	vty_out( vty, "Dn-Bandwidth:");
	if( PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
		vty_out( vty, "No policer\r\n");
	else
		vty_out( vty, "%d(kbit/s)\r\n", OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr );		
	
#ifdef PLATO_DBA_V3
	VOS_Sprintf( TempString, "Up-fixed:%d(kbit/s)", OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed );
	vty_out(vty,"%-34s\r\n", TempString );
#endif
	/*
	VOS_MemZero(&TempString[0], sizeof(TempString) );
	if( downlinkBWlimit == V2R1_DISABLE )
		sprintf( TempString, "Dn-Bandwidth:No policer");
	else sprintf(TempString,"Dn-Bandwidth:%d(kbit/s)", OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr );		
	vty_out(vty,"%-34s\r\n", TempString );
	*/
	vty_out(vty,"\r\n");

	if( onu_ext_dev_support(PonPortIdx,  OnuIdx) )
	/*if(GetOnuType( PonPortIdx,  OnuIdx, &ulOnuType) == ROK)*/
	{
		/*if(ulOnuType==V2R1_ONU_GT861)*/
			ShowExtOnuDeviceInfoByVty(PonPortIdx,  OnuIdx, vty);
	}
	

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	{
		unsigned char slot, port, i;
		unsigned short int LogicOnuId;
		unsigned long OnuDevIdx;
		int ret;
		unsigned long PotsEnableList = 0;

		/*OnuDevIdx = ( GetCardIdxByPonChip( PonPortIdx ) ) *10000 + ( GetPonPortByPonChip(PonPortIdx) ) * 1000 + (OnuIdx + 1);*/
		OnuDevIdx=MAKEDEVID(GetCardIdxByPonChip( PonPortIdx ),GetPonPortByPonChip(PonPortIdx),(OnuIdx + 1));
		ret = GetOnuBelongToSG(OnuDevIdx, &slot, &port, &LogicOnuId );
		if( ret == TDM_VM_OK )
		{
			VOS_Sprintf( TempString, "voice-service belong to TDM:%d/%d", slot, port );
			vty_out( vty, "%-34s logic onu-id: %d\r\n", TempString, LogicOnuId );
			ret = GetOnuPotsLinkAll(OnuDevIdx, &PotsEnableList);
			vty_out(vty,"%-34s","enabled pots-link list:");
			for(i = 0; i < ONU_POTS_NUM_MAX; i++)
			{
				if((PotsEnableList & ( 1<< i ) ) != 0 )
					TempString[i] = '1';
				else
					TempString[i] = '0';
			}
			TempString[i] = '\0';
			vty_out(vty,"%s\r\n", TempString );
		}
	}
#endif

    if(ThisIsGponOnu(PonPortIdx, OnuIdx))
    {
        /*vty_out();*/
    }
	return( ROK );
}

int  ShowOnuBandwidthByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	short int OnuEntry;
	short int LlidIdx = 0;
	
	CHECK_ONU_RANGE
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr) == ROK ) ||
		( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1) == ROK ))
	*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	{
		vty_out (vty, "\r\n onu %d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		return( ROK );
	}	
	
	vty_out (vty, "\r\n  display onu%d/%d/%d bandwidth Information \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	
	vty_out (vty, "        provisioned Bandwidth   \t Activated Bandwidth \r\n");
	vty_out (vty, "      Uplink  \tDownlink\tUplink\t Downlink(Unit: kbit/s)\r\n");

	vty_out (vty, "      %6d  \t %6d ", OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].UplinkBandwidth_gr,OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].DownlinkBandwidth_gr);
	
	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
		{
		vty_out (vty, "    \t %d", OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveUplinkBandwidth );

		if(PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
			{
			vty_out (vty, "      No policer\r\n");
			}
		else{
			vty_out (vty, "      %d\r\n",OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveDownlinkBandwidth );
			}
		/*
		vty_out (vty, "  \t%d",( downlinkBWlimit == V2R1_DISABLE ) ? PonPortTable[PonPortIdx].DownlinkMaxBw : OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveDownlinkBandwidth);
		if( downlinkBWlimit == V2R1_DISABLE ) vty_out (vty, "(no policeing)\r\n");
		else vty_out (vty, "\r\n");
		*/
		
		}
	else{
		vty_out (vty, "              0         0\r\n" );
		}

	vty_out(vty, "\r\n");
	return( ROK );
}



int  ShowOnuBandwidthAllByVty( short int PonPortIdx,  struct vty *vty)
{
	short int OnuEntry;
	short int OnuIdx;
	short int LlidIdx = 0;
    short int sDownIsPolice;
	
	CHECK_PON_RANGE

    if ( V2R1_DISABLE == PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ )
    {
        sDownIsPolice = 0;
    }
    else
    {
        sDownIsPolice = 1;
    }

/*
	if( PonPortIsWorking(PonPortIdx ) != TRUE ) 
		{
		vty_out(vty, "\r\n  %s/port%d not working\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( ROK );
		}
*/
	vty_out (vty, "\r\n  pon %d/%d All Onu bandwidth Information list \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));

	vty_out (vty, "  OnuIdx   provisioned Bandwidth          Activated Bandwidth\r\n");
	vty_out (vty, "          Uplink         Downlink        Uplink        Downlink(Unit: kbit/s)\r\n");
	
	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
		
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
		/* add by chenfj 2006-10-30*/
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
		
		vty_out (vty, "  %2d      %6d         %6d", (OnuIdx + 1 ),
				OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].UplinkBandwidth_gr,OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].DownlinkBandwidth_gr);
	
		if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
		{
			vty_out (vty, "          %6d        ", OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveUplinkBandwidth );

			if( sDownIsPolice )
			{
				vty_out (vty, "%d\r\n",OnuMgmtTable[OnuEntry].LlidTable[LlidIdx].ActiveDownlinkBandwidth );
			}
			else
            {
				vty_out (vty, "No policer\r\n");
			}
			}
		else{
			vty_out (vty, "             0            0\r\n" );
			}
		}
	return ( ROK );
}

int  ShowOnuBandwidthByVty_1(short int PonPortIdx, short int OnuIdx, struct vty *vty )
{
	short int OnuEntry; 
	short int PonChipType;
	unsigned int UplinkDelay, UplinkClass, Uplink_gr, Uplink_be;
	unsigned int DownlinkDelay, DownlinkClass, Downlink_gr, Downlink_be;
	
	CHECK_ONU_RANGE
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		/*vty_out (vty, "\r\n  %s/port%d Onu%d not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));*/
		return( V2R1_ONU_NOT_EXIST );
		}	
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
		
	vty_out (vty, "\r\n  onu %d/%d/%d bandwidth Information(unit:kbit/s) \r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
#if 0
	GetOnuUplinkBW_1(PonPortIdx, OnuIdx, &UplinkClass, &UplinkDelay, &Uplink_gr, &Uplink_be );
	GetOnuDownlinkBW_1(PonPortIdx, OnuIdx, &DownlinkClass, &DownlinkDelay, &Downlink_gr, &Downlink_be );
#else
	GetOnuRunningUplinkBW(PonPortIdx, OnuIdx, &UplinkClass, &UplinkDelay, &Uplink_gr, &Uplink_be );
	GetOnuRunningDownlinkBW(PonPortIdx, OnuIdx, &DownlinkClass, &DownlinkDelay, &Downlink_gr, &Downlink_be );
#endif

	vty_out (vty, "Uplink:   class:");
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "%2d ", UplinkClass);
	else
        vty_out(vty, " --");
	vty_out(vty, "                     ");

#ifdef PLATO_DBA_V3
	vty_out(vty,"fixed bw:");
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */)
		vty_out(vty,"%6d\r\n", /*OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_fixed*/OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed);

#else	
	vty_out(vty, "delay:");
	if( UplinkDelay == V2R1_DELAY_LOW)
	{
		vty_out(vty, "%s\r\n", ((PonChipType == PONCHIP_PAS) ? "low" : " --") );
	}
	else
	{
		vty_out(vty, "%s\r\n", ((PonChipType == PONCHIP_PAS) ? "high" : " --") );
	}
#endif

	vty_out(vty, "          assured bw:" );
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "%6d             ", Uplink_gr);
	else vty_out(vty, " --                ");

	vty_out(vty, "best-effort bw:");
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
		vty_out(vty, "%6d\r\n", Uplink_be);
	else vty_out(vty, " --\r\n");

	vty_out(vty, "Downlink: class:");
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		vty_out(vty, " --");
	else
		vty_out(vty, "%2d ", DownlinkClass);
	vty_out(vty, "                     ");
	
	vty_out(vty, "delay:");
	if( DownlinkDelay == V2R1_DELAY_LOW)
		{
		if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
			vty_out(vty, " --\r\n");
		else vty_out ( vty, "low\r\n");
		}	
	else {
		if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
			vty_out(vty, " --\r\n");
		else vty_out ( vty, "high\r\n");
		}
	
	vty_out(vty, "          assured bw:");
	if( 1 /* OLT_PONCHIP_ISPAS(PonChipType) */ )
	{
    	if(PonPortTable[PonPortIdx].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_DISABLE )
		{
    		vty_out(vty, "No policer         ");
		}
    	else
            vty_out(vty, "%6d             ", Downlink_gr);
	}
	else vty_out(vty, "%6d             ", Downlink_gr);

	vty_out(vty, "best-effort bw:"  );
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
		vty_out(vty, " --\r\n");
	}
	else vty_out(vty, "%6d\r\n", Downlink_be );
	
	return( ROK );
}






int ShowPAS6201MacLearningByVty(short int PonPortIdx, short int OnuIdx , struct vty *vty)
{
	short int Llid;
	short int PonChipType;
	short int ret = PAS_EXIT_ERROR;
	int i;
	long EntryNum=0;
	PON_onu_address_table_record_t  *address_table;

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr1 ) == ROK ))
		*/
		{
		vty_out(vty,  "\r\n  onu%d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}
	
	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) 
		{
		vty_out(vty,  "\r\n  onu %d/%d/%d off-line\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}

	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	address_table = &MAC_Address_Table[0];
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
		}
		else
#endif
		{
			ret = REMOTE_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		ret = TK_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
    }
	else {
		return( RERROR );
		}
	if( ret == PAS_EXIT_OK )
		{
		vty_out(vty, "\r\n  onu %d/%d/%d mac entry list\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		if( EntryNum == 0) 
			{
			vty_out(vty, "  total Learned mac counter=0\r\n");
			return( ROK );
			}
		vty_out(vty, "  total Learned mac counter=%d\r\n", EntryNum );
		vty_out(vty,  "    macAddr     type   dataPath    agingTime\r\n");
		vty_out(vty,  " --------------------------------------------\r\n");
		
		for(i=0;i< EntryNum;i++)
			{
			vty_out(vty, " %02x%02x.%02x%02x.%02x%02x", address_table->addr[0],address_table->addr[1],address_table->addr[2],address_table->addr[3],address_table->addr[4],address_table->addr[5]);

			if( address_table->type == ADDR_DYNAMIC )
				vty_out(vty, "  D     ");
			else if( address_table->type == ADDR_STATIC )
				vty_out(vty, "  S     " );
			else if( address_table->type == ADDR_DYNAMIC_AND_STATIC )
				vty_out(vty, "  D|S   ");
			else vty_out(vty, "  N/A   ");

			if(address_table->action == PON_DONT_PASS )
				vty_out(vty, "no path     ");
			else if( address_table->action == PON_PASS_DATAPATH ) vty_out(vty, "  data      " );
			else if( address_table->action == PON_PASS_CPU ) vty_out(vty, "  cpu       ");
			else if( address_table->action == PON_PASS_BOTH ) vty_out(vty, "cpu&data    ");
			
			vty_out(vty, "  %d\r\n", address_table->age );
			
			address_table++;
			}
		vty_out( vty,"\r\n");
		/* vty_out(vty, "    total Learned mac counter=%d\r\n", EntryNum ); */
		}
	else 
		{
		vty_out(vty, "    total Learned mac counter=0\r\n");
		return( ROK );
		}
	return( ROK );

}

int ShowPAS6201MacLearningCounterByVty(short int PonPortIdx, short int OnuIdx , struct vty *vty)
{
	short int Llid;
	short int PonChipType;
	short int ret = PAS_EXIT_ERROR;
	long EntryNum=0;
	PON_onu_address_table_record_t  *address_table;

	CHECK_ONU_RANGE
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
		*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		vty_out(vty,  "\r\n  onu %d/%d/%d not exist\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}
	
	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) 
		{
		vty_out(vty,  "\r\n  onu %d/%d/%d off-line\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}

	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	
	address_table = &MAC_Address_Table[0];
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
		}
		else
#endif
		{
			ret = REMOTE_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		ret = TK_PASONU_address_table_get( PonPortIdx, Llid, &EntryNum, address_table );
    }
	else {
		return( RERROR );
		}
	if( ret == PAS_EXIT_OK )
		{
		vty_out(vty, "\r\n  onu %d/%d/%d learned mac counter=%d\r\n\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1), EntryNum);
		
		return( ROK );
		}
	return( RERROR );	

}

int ShowOnuVersionInfoByVty(short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	/*int j;*/
	short int OnuEntry;
	unsigned char  DataString[MAXDEVICENAMELEN+1], *pVer;
	int len;
	/*unsigned int DeviceTypeLen = 0;*/
	/*unsigned char name[MAXDEVICENAMELEN+1];*/
	int nameLen=0;
	
	CHECK_PON_RANGE
		
#if 0	
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
		{
		vty_out(vty,  "  %s/port%d is not Working \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
		}
	
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
		{
		vty_out(vty,  "\r\n  %s/port%d onu%d not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		return( RERROR );
		}
#endif

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*
	if((CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ) ||
		(CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1 ) == ROK ))
		*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		vty_out(vty, " onu%d/%d/%d not exitst\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		return( V2R1_ONU_NOT_EXIST);
		}

	/*DeviceTypeLen = GetDeviceTypeLength();
	vty_out(vty, " onu %d/%d/%d version info:\r\n", GetCardIdxByPonChip(PonPortIdx),(GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	vty_out(vty, "Idx  type    ");
	for(j=0;j<(DeviceTypeLen - 5); j++)
		vty_out(vty," ");
	vty_out(vty,"range    HW-version  SW-version    userName    \r\n");
	vty_out(vty, "----------------------------------------------------------------\r\n");*/
	show_onu_version_banner(vty);
	
	/*vty_out(vty, "%2d   ", (OnuIdx+1));*/
	
	/*if( OnuMgmtTable[OnuEntry].DeviceInfo.type != V2R1_OTHER )
		vty_out(vty, "%s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
	else
	vty_out(vty, "%s  ",GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
	for(j=0;j<(DeviceTypeLen-VOS_StrLen(GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)));j++)
		vty_out(vty," ");*/

	vty_out(vty, "%d/%d/%-3d ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	VOS_MemZero( DataString, sizeof(DataString) );
	if( GetOnuModel( PonPortIdx, OnuIdx, DataString, &nameLen) != ROK )
				VOS_StrCpy( DataString, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
			vty_out( vty, "  %-12s  ", DataString );
			
	(GetOnuOperStatus(PonPortIdx , OnuIdx) != ONU_OPER_STATUS_DOWN ) ?
		vty_out(vty, "%5d     ", OnuMgmtTable[OnuEntry].RTT ): vty_out(vty,"  N/A     ");
	
	/*
	for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
		{
		vty_out(vty, "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
		i++;
		if( i != 5 )vty_out(vty, ".");
		}
	vty_out(vty, "  ");
	*/
	
	VOS_MemZero( DataString, sizeof(DataString) );
	pVer = &(DataString[0]);	
	len = 0;
	GetOnuHWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
	if( len == 0 )
		{
		vty_out(vty, "  N/A       ");
		}
	else {
		if( len > HWVERSIONLEN ) len = HWVERSIONLEN;
		pVer[len] = '\0';
		vty_out(vty, "%-12s", pVer );
		/*for( j=0; j<( HWVERSIONLEN-len+2); j++)
			vty_out(vty, " ");*/
		}
	
	pVer = &(DataString[0]);	
	len = 0;
	GetOnuSWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
	if( len == 0 )
		{
		vty_out(vty, "  N/A         ");
		}
	else {
		if( OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK )
			{
			unsigned char *Ptr= NULL;
			Ptr = VOS_StrStr( pVer, "/");
			if( Ptr != NULL )
				{
				pVer[Ptr -&pVer[0]] ='\0';
				len = VOS_StrLen(pVer);
				}
			}
		
		if( len > SWVERSIONLEN ) len = SWVERSIONLEN;
		pVer[len] = '\0';
		vty_out(vty, "%-14s", pVer );
		/*for( j=0; j<( SWVERSIONLEN-len); j++)
			vty_out(vty, " ");*/
		}	

	pVer = &(DataString[0]);	
	len = 0;
	GetOnuDeviceName( PonPortIdx, OnuIdx, pVer, &len );
			
	pVer[len ] = '\0';
	vty_out(vty, "%s\r\n", pVer); 

	/* modified by chenfj 2008-1-4
     问题单#6001:
          对GT831，增加一行用于显示语音程序版本
          */
	if(OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK )
		{
		unsigned char *Ptr = NULL;
		
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuSWVersion(PonPortIdx, OnuIdx, pVer, &len) ;

		Ptr = VOS_StrStr( pVer, "/");
		if(Ptr != NULL )
			{
			Ptr++;
			/*len = VOS_StrLen( Ptr );*/
			/*for(j=0; j<36; j++)
				vty_out(vty," ");*/
			vty_out(vty,  "%36s%s(VOICE)\r\n", " ", Ptr); 
			}		
		}
	else if(OnuFirmwareHasFpgaApp(PonPortIdx, OnuIdx) == ROK )
		{
		unsigned char *Ptr = NULL;
		
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuFWVersion(PonPortIdx, OnuIdx, pVer, &len) ;

		Ptr = VOS_StrStr( pVer, "/");
		if(Ptr != NULL )
			{
			Ptr++;
			/*len = VOS_StrLen( Ptr );*/
			/*for(j=0; j<36; j++)
				vty_out(vty," ");*/
			vty_out(vty,  "%36s%s(FPGA)\r\n", " ", Ptr); 
			}		
		}

	return( ROK );

}

/* modified by chenfj 2008-2-27 #6349
     在显示ONU 版本信息时, 增加ONU 类型可选参数, 这样可以只显示指定类型的ONU
     */
int ShowOnuVersionInfoByVty_1(short int PonPortIdx, short int OnuIdx, unsigned char *OnuTypeString, struct vty *vty)
{
	/*int j;*/
	short int OnuEntry;
	unsigned char  DataString[MAXDEVICENAMELEN+1], *pVer;
	int len;
	/*int DeviceTypeLen = 0;*/
	int  OnuType;
	/*unsigned char name[MAXDEVICENAMELEN+1];*/
	int nameLen=0;
	
	CHECK_PON_RANGE

#if 0
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
		{
		/*vty_out(vty,  "  %s/port%d is not Working \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));*/
		return( RERROR );
		}
	
	if(( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
		{
		/*vty_out(vty,  "\r\n  %s/port%d onu%d not exist\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));*/
		return( RERROR );
		}
#endif

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*
	if((CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ) ||
		(CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1 ) == ROK ))
		*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
		/*vty_out(vty, "%s/port%d onu%d not provisioned\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));*/
		return( ROK );
		}

	if( OnuTypeString != NULL )
		{
		/*
		if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) == ROK)
			{
			if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) != 0 ) return( RERROR );
			}
		else return( RERROR );
		*/
        if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) == ROK )
        {
			if(OnuType == V2R1_ONU_GPON)
			{
				VOS_MemZero( DataString, sizeof(DataString) );

				if(GetOnuTypeString(PonPortIdx, OnuIdx, DataString, &nameLen) == ROK)
				{
					if( VOS_StriCmp(OnuTypeString, DataString) != 0)
						return ERROR;
				}
                /*if(GetOnuModel(PonPortIdx, OnuIdx, DataString, &nameLen) != ROK)
                    return (RERROR);*/
			}
            else if(OnuType != V2R1_ONU_CTC && VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)))
                return (ERROR);
            else
            {

                VOS_MemZero( DataString, sizeof(DataString) );

                if(GetOnuModel(PonPortIdx, OnuIdx, DataString, &nameLen) != ROK)
                    return (RERROR);
                else if(VOS_StriCmp(OnuTypeString, DataString))
                    return (RERROR);
            }
		}
        else
            return (RERROR);
		}

	{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
		
		vty_out(vty, "%d/%d/%-3d ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
		/*DeviceTypeLen = GetDeviceTypeLength();*/
		/*if( OnuMgmtTable[OnuEntry].DeviceInfo.type != V2R1_OTHER )
			vty_out(vty,  "%s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
		else vty_out(vty,  "%s  ",GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
		for(j=0;j<(DeviceTypeLen -VOS_StrLen(GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type)));j++)
			vty_out(vty," ");*/

		VOS_MemZero( DataString, sizeof(DataString) );
		if( GetOnuModel( PonPortIdx, OnuIdx, DataString, &nameLen) != ROK )
			VOS_StrCpy( DataString, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
		vty_out( vty, "%-14s ", DataString );

		(GetOnuOperStatus(PonPortIdx , OnuIdx) != ONU_OPER_STATUS_DOWN ) ?
			vty_out(vty, "%5d     ", OnuMgmtTable[OnuEntry].RTT ): vty_out(vty, "  N/A     ");
	
		/*
		for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
			{
			vty_out(vty,  "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
			i++;
			if( i != 5 )vty_out(vty, ".");
			}
		svty_out(vty, "  ");
		*/
	
		VOS_MemZero( DataString, sizeof(DataString) );
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuHWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
		if( len == 0 )
			{
			vty_out(vty, "N/A         ");
			}
		else {
			if( len > HWVERSIONLEN ) len = HWVERSIONLEN;
			pVer[len] = '\0';
			vty_out(vty, "%-12s", pVer );
			/*for( j=0; j<( HWVERSIONLEN-len+2); j++)
				vty_out(vty, " ");		*/
			}
	
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuSWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
		if( len == 0 )
			{
			vty_out(vty, "N/A           ");
			}
		else {
			if( OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK )
			{
			unsigned char *Ptr = NULL;
			Ptr = VOS_StrStr( pVer, "/");
			if( Ptr != NULL )
				{
				pVer[Ptr -&pVer[0]] ='\0';
				len = VOS_StrLen(pVer);
				}
			}
					
			if( len > SWVERSIONLEN ) len = SWVERSIONLEN;
			pVer[len] = '\0';
			vty_out(vty, "%-14s", pVer );
			/*for( j=0; j<( SWVERSIONLEN-len); j++)
				vty_out(vty, " ");		*/
			}	

		pVer = &(DataString[0]);	
		len = 0;
		GetOnuDeviceName( PonPortIdx, OnuIdx, pVer, &len );
			
		pVer[len ] = '\0';
		vty_out(vty,  " %s\r\n", pVer); 
		}
/* modified by chenfj 2008-1-4
     问题单#6001:
          对GT831，增加一行用于显示语音程序版本
          */
	if(OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK )
		{
		unsigned char *Ptr = NULL;
		
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuSWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
			
		Ptr = VOS_StrStr( pVer, "/");
		if( Ptr != NULL )
			{
			Ptr++;
			/*len = VOS_StrLen( Ptr );*/
			/*for(j=0; j<41; j++)
				vty_out(vty," ");*/
			vty_out(vty,  "%41s%s(VOICE)\r\n", " ", Ptr); 
			}
		}
	/* modified by chenfj 2008-5-13
          对GT865，增加一行用于显示FPGA 版本
          */
	if(OnuFirmwareHasFpgaApp(PonPortIdx, OnuIdx) == ROK )
		{
		unsigned char *Ptr = NULL;
		
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuFWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
			
		Ptr = VOS_StrStr( pVer, "/");
		if( Ptr != NULL )
			{
			Ptr++;
			/*len = VOS_StrLen( Ptr );*/
			/*for(j=0; j<41; j++)
				vty_out(vty," ");		*/
			vty_out(vty,  "%41s%s(FPGA)\r\n", " ", Ptr); 
			}
		}

	return( ROK );

}

int ShowOnuVersionInfoByVtyAll(short int PonPortIdx, struct vty *vty)
{
	/*int j;*/
	short int OnuIdx;
	short int OnuEntry;
	unsigned char  DataString[MAXDEVICENAMELEN+1], *pVer;
	int len;
	/*int DeviceTypeLen = 0;*/
	/*unsigned char name[MAXDEVICENAMELEN+1];*/
	int nameLen=0;
	
	CHECK_PON_RANGE
/*	
	if(( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DOWN) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UNKNOWN ) ||( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_INIT ) )
		{
		vty_out(vty, "  %s/port%d is not Working \r\n",CardSlot_s[GetCardIdxByPonChip(PonPortIdx)],(GetPonPortByPonChip(PonPortIdx));
		return( RERROR );
		}
*/
	/*DeviceTypeLen = GetDeviceTypeLength();
	
	vty_out(vty, "pon %d/%d onu version info:\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
	vty_out(vty, "Idx  type   ");
	for(j=0;j<(DeviceTypeLen-5);j++)
		vty_out(vty," ");
	vty_out(vty,"range(m)  HW-version  SW-version    userName    \r\n");
	vty_out(vty, "----------------------------------------------------------------------\r\n");*/
	show_onu_version_banner(vty);

	for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
		{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
		/*
		if((CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ) ||
			(CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr1 ) == ROK ))
			continue;
		*/
		vty_out(vty, "%d/%d/%-4d   ", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	
	/*	if( OnuMgmtTable[OnuEntry].DeviceInfo.type != V2R1_OTHER )
			vty_out(vty,  "%s  ", GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
		else vty_out(vty,  "%s  ",GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type));
		for(j=0; j<(DeviceTypeLen - VOS_StrLen(GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type))); j++)
			vty_out(vty," ");*/
		VOS_MemZero( DataString, sizeof(DataString) );
		if( GetOnuModel( PonPortIdx, OnuIdx, DataString, &nameLen) != ROK )
			VOS_StrCpy( DataString, GetDeviceTypeString(OnuMgmtTable[OnuEntry].DeviceInfo.type) );
		vty_out( vty, "%s ", DataString );
			


		(GetOnuOperStatus(PonPortIdx , OnuIdx) != ONU_OPER_STATUS_DOWN ) ?
			vty_out(vty, "%4d      ", OnuMgmtTable[OnuEntry].RTT ): sys_console_printf("  N/A     ");
	
		/*
		for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
			{
			vty_out(vty,  "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
			i++;
			if( i != 5 )vty_out(vty, ".");
			}
		svty_out(vty, "  ");
		*/
	
		VOS_MemZero( DataString, sizeof(DataString) );
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuHWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
		if( len == 0 )
			{
			vty_out(vty, "N/A         ");
			}
		else {
			if( len > HWVERSIONLEN ) len = HWVERSIONLEN;
			pVer[len] = '\0';
			vty_out(vty, "%-12s", pVer );
			/*for( j=0; j<( HWVERSIONLEN-len+2); j++)
				vty_out(vty, " ");		*/
			}
	
		pVer = &(DataString[0]);	
		len = 0;
		GetOnuSWVersion(PonPortIdx, OnuIdx, pVer, &len) ;
		if( len == 0 )
			{
			vty_out(vty, "N/A           ");
			}
		else {
			if( OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK )
			{
			unsigned char *Ptr = NULL;
			Ptr = VOS_StrStr( pVer, "/");
			if( Ptr != NULL )
				{
				pVer[Ptr -&pVer[0]] ='\0';
				len = VOS_StrLen(pVer);
				}
			}
					
			if( len > SWVERSIONLEN ) len = SWVERSIONLEN;
			pVer[len] = '\0';
			vty_out(vty, "%-14s", pVer );
			/*for( j=0; j<( SWVERSIONLEN-len); j++)
				vty_out(vty, " ");		*/
			}	

		pVer = &(DataString[0]);	
		len = 0;
		GetOnuDeviceName( PonPortIdx, OnuIdx, pVer, &len );
			
		pVer[len ] = '\0';
		vty_out(vty,  "%s\r\n", pVer); 
		}
	vty_out(vty, "\r\n");	

	return ( ROK );

}

#ifdef  PAS6201_REMOTE_MANAGEMENT

                                   
int SetPas6201AgingTime( short int PonPortIdx, short int OnuIdx,  unsigned long AgeTime/*unit: in seconds*/)
{
	short int Llid;
	short int PonChipType;
	
	CHECK_ONU_RANGE

	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) return( RERROR );

	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(OLT_PONCHIP_ISPAS(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			return(GW10G_REMOTE_PASONU_address_table_ageing_configuration( PonPortIdx, Llid, ENABLE, AgeTime ));
		}
		else
#endif
		{
			return(REMOTE_PASONU_address_table_ageing_configuration( PonPortIdx, Llid, ENABLE, AgeTime ));
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		return(TK_PASONU_address_table_ageing_configuration( PonPortIdx, Llid, ENABLE, AgeTime ));
    }
	else{
		return( ROK );
		}

}

int GetPas6201AddressEntry(short int PonPortIdx, short int OnuIdx, long *EntryNum)
{
	short int Llid;
	short int PonChipType;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) return( RERROR );

	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(OLT_PONCHIP_ISPAS(PonChipType))
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			return(GW10G_REMOTE_PASONU_address_table_get_number_of_entries( PonPortIdx, Llid, EntryNum ));
		}
		else
#endif
		{
			return(REMOTE_PASONU_address_table_get_number_of_entries( PonPortIdx, Llid, EntryNum ));
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		return(TK_PASONU_address_table_get_number_of_entries( PonPortIdx, Llid, EntryNum ));
    }
	else{
		return( ROK );
		}

}

int GetPas6201AddressTable(short int PonPortIdx, short int OnuIdx, long *EntryNum, PON_onu_address_table_record_t *address_table)
{
	short int Llid;
	short int PonChipType;
	short int ret = PAS_EXIT_ERROR;
	int i;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP ) return( RERROR );

	Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if(Llid == INVALID_LLID ) return( RERROR );
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if( OLT_PONCHIP_ISPAS(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			ret = GW10G_REMOTE_PASONU_address_table_get( PonPortIdx, Llid, EntryNum, address_table );
		}
		else
#endif
		{
			ret = REMOTE_PASONU_address_table_get( PonPortIdx, Llid, EntryNum, address_table );
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		ret = TK_PASONU_address_table_get( PonPortIdx, Llid, EntryNum, address_table );
    }
	else {
		return( RERROR );
		}
	if( ret == PAS_EXIT_OK )
		{
		sys_console_printf("%s/port%d onu%d Address Table, entry=%d\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx +1), *EntryNum);
		for(i=0;i< *EntryNum;i++)
			{
			sys_console_printf(" %02x%02x.%02x%02x.%02x%02x\r\n", address_table->addr[0],address_table->addr[1],address_table->addr[2],address_table->addr[3],address_table->addr[4],address_table->addr[5]);
			sys_console_printf(" aging time:%d", address_table->age );
			sys_console_printf(" type:%d\r\n", address_table->type );
			address_table++;
			}
		}
	return( ROK );

}


int GetPas6201EncryptState(short int PonPortIdx, short int OnuIdx, unsigned int *Direction  )
{
	short int PonChipType;
	short int Llid;

	CHECK_ONU_RANGE

	if(Direction == NULL ) return( RERROR );

	if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	Llid = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if(  OLT_PONCHIP_ISPAS(PonChipType) )
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			GW10G_REMOTE_PASONU_encryption_get_state(PonPortIdx, Llid, (PON_encryption_direction_t*)Direction);
		}
		else
#endif
		{
			REMOTE_PASONU_encryption_get_state(PonPortIdx, Llid, (PON_encryption_direction_t*)Direction);
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		TK_PASONU_encryption_get_state(PonPortIdx, Llid, (PON_encryption_direction_t*)Direction);
    }
	else{
		}
	return( ROK );		

}


int SetPas6201EncryptKey( short int PonPortIdx, short int OnuIdx, PON_encryption_key_t  key, PON_encryption_key_index_t Idx)
{
	short int PonChipType;
	short int Llid;

	CHECK_ONU_RANGE

	if(( Idx != PON_ENCRYPTION_KEY_INDEX_0 ) &&( Idx != PON_ENCRYPTION_KEY_INDEX_1 )) return( RERROR );

	if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return( RERROR );
	Llid = GetLlidByOnuIdx(PonPortIdx,  OnuIdx);

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if( OLT_PONCHIP_ISPAS(PonChipType)) 
	{
#if defined(_EPON_10G_PMC_SUPPORT_)            
		if(OLT_PONCHIP_ISPAS10G(PonChipType))
		{
			GW10G_REMOTE_PASONU_encryption_set_key(PonPortIdx, Llid, key, Idx );
		}
		else
#endif
		{
			REMOTE_PASONU_encryption_set_key(PonPortIdx, Llid, key, Idx );
		}
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
		TK_PASONU_encryption_set_key(PonPortIdx, Llid, key, Idx );
    }
	else{
		/*REMOTE_PASONU_encryption_set_key(PonPortIdx, Llid, key, Idx );*/
		}
	
	return( ROK);	

}
#endif

/*  added by chenfj 2006/12/06
	set onu peer-to-peer 
*/

#ifdef ONU_PEER_TO_PEER

int  RecordOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 )
{
	short int OnuEntry;
	short int OnuIdx;
	unsigned char  idx1, idx2;
	unsigned char CurValue;
	
	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE

	if( OnuIdx1 == OnuIdx2 ) return( RERROR );
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
	idx1 = OnuIdx2 /8;
	idx2 = OnuIdx2 %8;

	ONU_MGMT_SEM_TAKE;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	CurValue  = CurValue |(1 << idx2);	
	OnuMgmtTable[OnuEntry].PeerToPeer[idx1] = CurValue;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx2;
	idx1 = OnuIdx1 /8;
	idx2 = OnuIdx1 %8;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	CurValue  = CurValue |(1 << idx2);	
	OnuMgmtTable[OnuEntry].PeerToPeer[idx1] = CurValue;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

int  ClearOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1, short int OnuIdx2 )
{
	short int OnuEntry;
	short int OnuIdx;
	unsigned char  idx1, idx2;
	unsigned char CurValue;
	
	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE
		
	if( OnuIdx1 == OnuIdx2 ) return( RERROR );
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
	idx1 = OnuIdx2 /8;
	idx2 = OnuIdx2 %8;
	ONU_MGMT_SEM_TAKE;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	CurValue  = CurValue &( BYTE_FF -(1 << idx2));	
	OnuMgmtTable[OnuEntry].PeerToPeer[idx1] = CurValue;

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx2;
	idx1 = OnuIdx1 /8;
	idx2 = OnuIdx1 %8;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	CurValue  = CurValue & (BYTE_FF -(1 << idx2));	
	OnuMgmtTable[OnuEntry].PeerToPeer[idx1] = CurValue;
	ONU_MGMT_SEM_GIVE;

	return( ROK );
}

/* added by xieshl 20110614 */
int GetOnuPeerToPeerInOnuEntry( short int OnuEntry,short int OnuIdx )
{
	unsigned char CurValue;
	unsigned char  idx1, idx2;

	if( (OnuEntry < 0) || (OnuEntry > MAXONU) )
	{
		VOS_ASSERT(0);
		return RERROR;
	}
	idx1 = OnuIdx >> 3;
	idx2 = OnuIdx & 7;

	ONU_MGMT_SEM_TAKE;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	ONU_MGMT_SEM_GIVE;

	if(( CurValue & (1 << idx2) ) != 0)
		return( V2R1_ENABLE );
	return( V2R1_DISABLE );
}
int GetOnuPeerToPeer( short int PonPortIdx, short int OnuIdx1,short int OnuIdx2/*, int *address_not_foundFlag, int *broadcastFlag*/ )
{
	short int OnuEntry, OnuIdx;
	/*unsigned char CurValue;
	unsigned char  idx1, idx2;*/

	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE

	if( OnuIdx1 == OnuIdx2 ) return( RERROR );
	/*
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx2;
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx2) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx1) != ROK )
		return( V2R1_ONU_NOT_EXIST );
#if 0	
	idx1 = OnuIdx2 >> 3;
	idx2 = OnuIdx2 & 7;

	ONU_MGMT_SEM_TAKE;
	CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[idx1];
	ONU_MGMT_SEM_GIVE;

	/*if( EVENT_DEBUG == V2R1_ENABLE )
		{
	sys_console_printf(" Onu%d 0x%x 0x%x Onu%d 0x%x 0x%x \r\n", (OnuIdx1 +1),*(int*)&OnuMgmtTable[OnuEntry].PeerToPeer[0],
		*(int*)&OnuMgmtTable[OnuEntry].PeerToPeer[4], (OnuIdx2+1), *(int*)&OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx2].PeerToPeer[0],
		*(int*)&OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx2].PeerToPeer[4] );
		sys_console_printf(" cur value %d \r\n", CurValue );
	}*/

	if(( CurValue & (1 << idx2) ) != 0) return( V2R1_ENABLE );
	else return( V2R1_DISABLE );
#else
	return GetOnuPeerToPeerInOnuEntry( OnuEntry, OnuIdx2 );
#endif
}

int GetOnuIsSetPeerToPeer( short int PonPortIdx, short int OnuIdx )
{
	int Peer1, Peer2;
	short int OnuEntry;
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	
	ONU_MGMT_SEM_TAKE;
	Peer1 = *(int *)&(OnuMgmtTable[OnuEntry].PeerToPeer[0]);
	Peer2 = *(int *)&(OnuMgmtTable[OnuEntry].PeerToPeer[4]);
	ONU_MGMT_SEM_GIVE;

	if(( Peer1 != 0 ) ||( Peer2 != 0 )) return ( V2R1_ONU_SETING_P2P );
	
	return( V2R1_ONU_NOSETING_P2P );

}

int  GetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx, int *address_not_foundFlag, int *broadcastFlag )
{
	short int OnuEntry;
	
	CHECK_ONU_RANGE

	if(( address_not_foundFlag == NULL ) ||(broadcastFlag == NULL )) return( RERROR );

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	
	ONU_MGMT_SEM_TAKE;
	*address_not_foundFlag = OnuMgmtTable[OnuEntry].address_not_found_flag;
	*broadcastFlag = OnuMgmtTable[OnuEntry].broadcast_flag;
	ONU_MGMT_SEM_GIVE;
	return( ROK );
}

/*int  DisplayOnuPeerToPeer( short int PonPortIdx, short int OnuIdx)
{
	short int OnuEntry;
	unsigned char  i,j, k;
	unsigned char CurValue;
	
	CHECK_ONU_RANGE
		
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	sys_console_printf("%s/port%d onu%d peer-to-peer list:\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1));
	sys_console_printf("--------------------------------------------------\r\n");
	k = 0;
	for( i= 0; i< 8; i++)
		{
		CurValue = OnuMgmtTable[OnuEntry].PeerToPeer[i];
		for(j=0; j<8;j++)
			{
			if((CurValue & (1 << j)) !=0) 
				{
				sys_console_printf(" %2d", (i*8 + j+1));
				k++;
				}
			if((  k!=0 ) &&(( k % 16) == 0))
				sys_console_printf("\r\n");
			}
		}
	if((  k!=0 ) &&( k % 16 != 0))
		sys_console_printf(" \r\n");
	sys_console_printf(" total peer-to-peer onu counter:%d\r\n", k);

	return( ROK );
}*/

int DisableOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx )
{
	/*short int Llid;*/
	short int ret;
	/*short int PonChipType,PonChipVer;*/
	
	CHECK_ONU_RANGE

#if 1
    ret = OnuMgt_SetOnuPeerToPeerForward(PonPortIdx, OnuIdx, DISABLE, DISABLE);
#else
	Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if(Llid == INVALID_LLID ) return( V2R1_ONU_OFF_LINE );
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		ret = PAS_set_llid_p2p_configuration( PonPortIdx, Llid, DISABLE , DISABLE );
		}
	else{
		ret = PAS_EXIT_ERROR;
		}
#endif

	if( ret != PAS_EXIT_OK ) return( RERROR );
	return( ROK );
}

int CopyOnuPeerToPeerForward(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    int iSrcEntry;
    int iSrcCfg[2];

    iSrcEntry  = SrcPonPortIdx * MAXONUPERPON + SrcOnuIdx;
    ONU_MGMT_SEM_TAKE;
    iSrcCfg[0] = OnuMgmtTable[iSrcEntry].address_not_found_flag;
    iSrcCfg[1] = OnuMgmtTable[iSrcEntry].broadcast_flag;   
    ONU_MGMT_SEM_GIVE;
    if ( OLT_COPYFLAGS_COVER & CopyFlags )
    {
        iRlt = OnuMgt_SetOnuPeerToPeerForward(DstPonPortIdx, DstOnuIdx, iSrcCfg[0], iSrcCfg[1]);
    }
    else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
    {
        if ( (V2R1_ENABLE == iSrcCfg[0])
            || (V2R1_ENABLE == iSrcCfg[1]) )
        {
            iRlt = OnuMgt_SetOnuPeerToPeerForward(DstPonPortIdx, DstOnuIdx, iSrcCfg[0], iSrcCfg[1]);
        }
    }
    else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
    {
        if ( OLT_ISLOCAL(DstPonPortIdx) )
        {
            int iDstEntry;
            int iDstCfg[2];
            
            iDstEntry  = DstPonPortIdx * MAXONUPERPON + DstOnuIdx;
            ONU_MGMT_SEM_TAKE;
            iDstCfg[0] = OnuMgmtTable[iDstEntry].address_not_found_flag;
            iDstCfg[1] = OnuMgmtTable[iDstEntry].broadcast_flag;   
            ONU_MGMT_SEM_GIVE;
            if ( (iDstCfg[0] != iSrcCfg[0])
                || (iDstCfg[1] != iSrcCfg[1]) )
            {
                iRlt = OnuMgt_SetOnuPeerToPeerForward(DstPonPortIdx, DstOnuIdx, iSrcCfg[0], iSrcCfg[1]);
            }
        }
        else
        {
            iRlt = OnuMgt_SetOnuPeerToPeerForward(DstPonPortIdx, DstOnuIdx, iSrcCfg[0], iSrcCfg[1]);
        }
    }

    return iRlt;
}

int EnableOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx /*, int address_not_found, int broadcast*/ )
{
#if 1
    return CopyOnuPeerToPeerForward(PonPortIdx, OnuIdx, PonPortIdx, OnuIdx, OLT_COPYFLAGS_RESUME);
#else
	/*short int Llid;*/
	short int ret;
	/*short int PonChipType, PonChipVer;*/
	int address_not_found, broadcast;
	
	CHECK_ONU_RANGE
	
	address_not_found = OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].address_not_found_flag;
	broadcast = OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].broadcast_flag;
	if( address_not_found != V2R1_ENABLE ) address_not_found = DISABLE;
	if( broadcast != V2R1_ENABLE ) broadcast = DISABLE;

	Llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if(Llid == INVALID_LLID ) return( V2R1_ONU_OFF_LINE );
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
	
	if( PonChipType == PONCHIP_PAS )
		{
		ret = PAS_set_llid_p2p_configuration( PonPortIdx, Llid, address_not_found, broadcast );
		}
	else{
		ret = PAS_EXIT_ERROR;
		}

	return( ret );
#endif
}

#if 0
int EnableOnuPeerToPeer_1(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2)
{
	short int Llid1, Llid2;
	short int llidArray[10];
	short int OnuIdx;
	short int ret;

	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE

	Llid1 = GetLlidByOnuIdx( PonPortIdx, OnuIdx1 );
	Llid2 = GetLlidByOnuIdx( PonPortIdx, OnuIdx2 );
	llidArray[0]= Llid2;
	if(( Llid1 == INVALID_LLID ) ||( Llid2 == INVALID_LLID ) )
		return( V2R1_ONU_OFF_LINE );
	/*
	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx1);
	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx2);
	*/
	ret = PAS_set_llid_p2p_configuration(PonPortIdx, Llid1, ENABLE, ENABLE );
	if( ret != PAS_EXIT_OK ) return( RERROR );
	ret = PAS_set_llid_p2p_configuration(PonPortIdx, Llid2, ENABLE, ENABLE );
	if( ret != PAS_EXIT_OK ) return( RERROR );
	ret = PAS_set_p2p_access_control ( PonPortIdx, Llid1, 1, llidArray, ENABLE );
	if( ret != PAS_EXIT_OK ) return( RERROR );
	return( ROK );
}
#endif

int EnableOnuPeerToPeer(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2)
{
	/*short int Llid1, Llid2;
	short int llidArray[10];*/
	short int OnuIdx;
	/*short int ret;
	short int PonChipType, PonChipVer;*/
	/*int  unicast_Flag, broadcastFlag;*/
    int iRlt;

	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE;
        
	if( OnuIdx1 ==  OnuIdx2 ) return( RERROR );

#if 1
    iRlt = OnuMgt_SetOnuPeerToPeer(PonPortIdx, OnuIdx1, OnuIdx2, V2R1_ENABLE);
    if ( OLT_CALL_ISERROR(iRlt) )
    {
        if ( OLT_ERR_NOTEXIST == iRlt )
        {
            iRlt = V2R1_ONU_OFF_LINE;
        }
        else
        {
            iRlt = RERROR;
        }
    }
#else
	Llid1 = GetLlidByOnuIdx( PonPortIdx, OnuIdx1 );
	Llid2 = GetLlidByOnuIdx( PonPortIdx, OnuIdx2 );
	llidArray[0]= Llid2;
	if(( Llid1 == INVALID_LLID ) ||( Llid2 == INVALID_LLID ) )
		return( V2R1_ONU_OFF_LINE );
	/*
	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx1);
	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx2);
	*/
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if(PonChipType == PONCHIP_PAS )
		{
		/*
		unicast_Flag = OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx1].address_not_found_flag;
		if(unicast_Flag == V2R1_ENABLE ) unicast_Flag = ENABLE;
		else unicast_Flag = DISABLE;
		broadcastFlag = OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx1].broadcast_flag;
		if(broadcastFlag == V2R1_ENABLE ) broadcastFlag=ENABLE;
		else broadcastFlag = DISABLE;
		ret = PAS_set_llid_p2p_configuration(PonPortIdx, Llid1, unicast_Flag,  broadcastFlag);
		if( ret != PAS_EXIT_OK ) return( RERROR );

		unicast_Flag = OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx1].address_not_found_flag;
		if(unicast_Flag == V2R1_ENABLE ) unicast_Flag = ENABLE;
		else unicast_Flag = DISABLE;
		broadcastFlag = OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx1].broadcast_flag;
		if(broadcastFlag == V2R1_ENABLE ) broadcastFlag=ENABLE;
		else broadcastFlag = DISABLE;
		ret = PAS_set_llid_p2p_configuration(PonPortIdx, Llid2, OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx2].address_not_found_flag, OnuMgmtTable[PonPortIdx *MAXONUPERPON + OnuIdx2].broadcast_flag );
		if( ret != PAS_EXIT_OK ) return( RERROR );
		*/
		ret = PAS_set_p2p_access_control ( PonPortIdx, Llid1, 1, llidArray, ENABLE );
		if( ret != PAS_EXIT_OK ) return( RERROR );
		}
	else{
		ret = PAS_EXIT_ERROR ;
		return( ret );
		}
	return( ROK );
#endif
	return( iRlt );
}



int DisableOnuPeerToPeer(short int PonPortIdx,short int OnuIdx1, short int OnuIdx2)
{
	/*short int Llid1, Llid2;
	short int llidArray[10];*/
	short int OnuIdx;
	short int ret;
	/*short int PonChipType, PonChipVer;*/

	OnuIdx = OnuIdx1;
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;
	CHECK_ONU_RANGE

	if( OnuIdx1 ==  OnuIdx2 ) return( RERROR );

#if 1
    ret = OnuMgt_SetOnuPeerToPeer(PonPortIdx, OnuIdx1, OnuIdx2, V2R1_DISABLE);
#else
	Llid1 = GetLlidByOnuIdx( PonPortIdx, OnuIdx1 );
	Llid2 = GetLlidByOnuIdx( PonPortIdx, OnuIdx2 );
	
	llidArray[0]= Llid2;
	if(( Llid1 == INVALID_LLID ) ||( Llid2 == INVALID_LLID )  )
		return( V2R1_ONU_OFF_LINE );

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_PAS )
		{
		ret =  PAS_set_p2p_access_control ( PonPortIdx, Llid1, 1, llidArray, DISABLE );
		}
	else{
		ret = PAS_EXIT_ERROR;
		}
#endif

	if( ret != PAS_EXIT_OK ) return( RERROR );
	return( ROK );
}

int SetOnuPeerToPeerForward( short int PonPortIdx, short int OnuIdx , int address_not_found, int broadcast )
{
	/*short int OnuEntry;
	short int llid;*/
	short int ret = PAS_EXIT_OK;
	/*short int PonChipType;*/

	CHECK_ONU_RANGE
	
	if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	if(( address_not_found != V2R1_ENABLE ) &&(address_not_found != V2R1_DISABLE )) return( RERROR );
	if(( broadcast != V2R1_ENABLE ) &&( broadcast != V2R1_DISABLE )) return( RERROR );
	/*
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	
	/* modified by chenfj 2007-10-22
	问题单#5605: 配置p2p forward address-not-found disable时，同时配置了广播，且就没有命令来关闭了
	修改: 在配置前转规则时，判断ONU是否配置了P2P；若是，则同时配置广播包前转使能.
	              只要删除ONU的P2P配置，广播包前转变为禁止
	*/
	if( GetOnuIsSetPeerToPeer(PonPortIdx, OnuIdx) == V2R1_ONU_SETING_P2P)
		broadcast = V2R1_ENABLE;
	else
        broadcast = V2R1_DISABLE;

#if 1
    ret = OnuMgt_SetOnuPeerToPeerForward(PonPortIdx, OnuIdx, address_not_found, broadcast);
#else
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;

	OnuMgmtTable[OnuEntry].address_not_found_flag = address_not_found;
	OnuMgmtTable[OnuEntry].broadcast_flag = broadcast;
	/*
	if( address_not_found == V2R1_ENABLE ) address_not_found = ENABLE;
	else address_not_found = DISABLE;
	if( broadcast == V2R1_ENABLE ) broadcast = ENABLE;
	else broadcast = DISABLE;	
	*/
	ret = EnableOnuPeerToPeerForward( PonPortIdx, OnuIdx/*, address_not_found, broadcast*/);
	return( ROK );
#if 0	
	if((GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP )
		&&( (*(int *)&OnuMgmtTable[OnuEntry].PeerToPeer[0] != 0) ||(*(int *)&OnuMgmtTable[OnuEntry].PeerToPeer[4] != 0)))
		{
		PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx);
		if( llid == INVALID_LLID ) return( V2R1_ONU_OFF_LINE );
		if(  PonChipType == PONCHIP_PAS )
			{
			ret = PAS_set_llid_p2p_configuration(PonPortIdx, llid, address_not_found, broadcast );
			}
		else {
			ret = PAS_EXIT_ERROR ;
			}
		}
	
	if( ret != PAS_EXIT_OK ) return( ret );
	else return( ROK );
#endif	
#endif
	return( ret );
}

int  SetOnuPeerToPeer(short int PonPortIdx, short int OnuIdx1, short int OnuIdx2)
{
	short int OnuIdx;
	short int OnuEntry;
	int Broadcast_Flag = V2R1_ENABLE;
	int address_not_found_flag1, address_not_found_flag2;
	int ret;

	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE

	if( OnuIdx1 == OnuIdx2 ) return( RERROR );
	/*
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx1) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	/*
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx2;
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx2) != ROK )
		return( V2R1_ONU_NOT_EXIST );

#if 1
    ret = OnuMgt_SetOnuPeerToPeer(PonPortIdx, OnuIdx1, OnuIdx2, V2R1_ENABLE);
#else
	RecordOnuPeerToPeer( PonPortIdx, OnuIdx1, OnuIdx2);

	ret =  EnableOnuPeerToPeer( PonPortIdx, OnuIdx1, OnuIdx2 );
#endif
    if ( 0 == ret )
    {
    	/* modified by chenfj 2007-10-22
        	问题单#5605: 配置p2p forward address-not-found disable时，同时配置了广播，且就没有命令来关闭了
        	修改: 在配置前转规则时，判断ONU是否配置了P2P；若是，则同时配置广播包前转使能.
        	              只要删除ONU的P2P配置，广播包前转变为禁止
        	*/
       OnuEntry = PonPortIdx*MAXONUPERPON;
	ONU_MGMT_SEM_TAKE;
	address_not_found_flag1 = OnuMgmtTable[OnuEntry + OnuIdx1].address_not_found_flag;
	address_not_found_flag2 = OnuMgmtTable[OnuEntry + OnuIdx2].address_not_found_flag;
	ONU_MGMT_SEM_GIVE;
    	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx1, address_not_found_flag1, Broadcast_Flag );
    	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx2, address_not_found_flag2, Broadcast_Flag );
    }


	return( ret );	
}

int  DiscOnuPeerToPeer(short int PonPortIdx, short int OnuIdx1, short int OnuIdx2)
{
	short int OnuIdx;
	short int OnuEntry;
	int ret;
	int Broadcast_Flag = V2R1_DISABLE;
	int address_not_found_flag1, address_not_found_flag2;
	
	OnuIdx = OnuIdx1;	
	CHECK_ONU_RANGE
	OnuIdx = OnuIdx2;	
	CHECK_ONU_RANGE

	if( OnuIdx1 ==  OnuIdx2 ) return( RERROR );
	/*
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if( ThisIsValidOnu(PonPortIdx, OnuIdx1) != ROK )
		return( V2R1_ONU_NOT_EXIST );
	/*
	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx2;
	if(( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK )
		||( CompTwoMacAddress( OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, Invalid_Mac_Addr ) == ROK ))
	*/
	if(ThisIsValidOnu(PonPortIdx, OnuIdx2) != ROK )
		return( V2R1_ONU_NOT_EXIST );

#if 1
	ret = OnuMgt_SetOnuPeerToPeer(PonPortIdx, OnuIdx1, OnuIdx2, V2R1_DISABLE);
#else
	ClearOnuPeerToPeer( PonPortIdx, OnuIdx1, OnuIdx2);

	ret =  DisableOnuPeerToPeer( PonPortIdx, OnuIdx1, OnuIdx2 );
	/*ret =  DisableOnuPeerToPeerForward( PonPortIdx, OnuIdx1);*/
#endif
    if ( 0 == ret )
    {
    	/* modified by chenfj 2007-10-22
    	问题单#5605: 配置p2p forward address-not-found disable时，同时配置了广播，且就没有命令来关闭了
    	修改: 在配置前转规则时，判断ONU是否配置了P2P连接；若是，则同时配置广播包前转使能.
    	              只要删除ONU的P2P配置(onu上没有P2P连接)，广播包前转变规则修改为禁止
    	*/
    	OnuEntry = PonPortIdx*MAXONUPERPON;
	ONU_MGMT_SEM_TAKE;
	address_not_found_flag1 = OnuMgmtTable[OnuEntry + OnuIdx1].address_not_found_flag;
	address_not_found_flag2 = OnuMgmtTable[OnuEntry + OnuIdx2].address_not_found_flag;
	ONU_MGMT_SEM_GIVE;
    	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx1, address_not_found_flag1, Broadcast_Flag );
    	SetOnuPeerToPeerForward(PonPortIdx, OnuIdx2, address_not_found_flag2, Broadcast_Flag );
    }

	return( ROK );	
}

int CopyOnuPeerToPeer(short int DstPonPortIdx, short int DstOnuIdx, short int SrcPonPortIdx, short int SrcOnuIdx, int CopyFlags)
{
    int iRlt = 0;
    short int OnuIdx1;
    short int sSrcCfg;

    for ( OnuIdx1 = 0; OnuIdx1 < MAXONUPERPON; OnuIdx1++ )
    {
        sSrcCfg = GetOnuPeerToPeer( SrcPonPortIdx, SrcOnuIdx, OnuIdx1 );
    	if ( (V2R1_ENABLE == sSrcCfg)
            || (V2R1_DISABLE == sSrcCfg) )
        {   
            if ( OLT_COPYFLAGS_COVER & CopyFlags )
            {
                OnuMgt_SetOnuPeerToPeer(DstPonPortIdx, DstOnuIdx, OnuIdx1, sSrcCfg);
            }
            else if ( OLT_COPYFLAGS_ONLYNEW & CopyFlags )
            {
                if ( V2R1_ENABLE == sSrcCfg )
                {
                    OnuMgt_SetOnuPeerToPeer(DstPonPortIdx, DstOnuIdx, OnuIdx1, V2R1_ENABLE);
                }
            }
            else if ( OLT_COPYFLAGS_CHECK & CopyFlags )
            {
                if ( OLT_ISLOCAL(DstPonPortIdx) )
                {
                    if ( sSrcCfg != GetOnuPeerToPeer( DstPonPortIdx, DstOnuIdx, OnuIdx1 ) )
                    {
                        OnuMgt_SetOnuPeerToPeer(DstPonPortIdx, DstOnuIdx, OnuIdx1, sSrcCfg);
                    }
                }
                else
                {
                    OnuMgt_SetOnuPeerToPeer(DstPonPortIdx, DstOnuIdx, OnuIdx1, sSrcCfg);
                }
            }
        }
    }
    
    return iRlt;
}

int ShowOnuPeerToPeerByVty( short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	short int OnuEntry, OnuIdx1;
	int slot, port;
	/*short int i;*/
	int address_not_found, broadcast;
	int ret;

	
	CHECK_ONU_RANGE

	slot = GetCardIdxByPonChip( PonPortIdx );
	port = GetPonPortByPonChip( PonPortIdx ) ;
/*
	if( getPonChipInserted((unsigned char )slot, (unsigned char )port) !=  PONCHIP_EXIST )
		{
		sys_console_printf( "\r\n  %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( ROK );
		} 
	if( PonPortIsWorking(PonPortIdx) == FALSE )
		{
		sys_console_printf( "\r\n%s/port%d not working\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
		return( ROK );
		}	
*/	
	if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	/*if(CompTwoMacAddress(OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.MacAddr,Invalid_Mac_Addr) == ROK)*/
	{
		vty_out(vty, "\r\n%s/port%d onu%d not exist\r\n",CardSlot_s[slot], port, (OnuIdx+1));
		return( ROK );
	}
	if( GetOnuIsSetPeerToPeer( PonPortIdx, OnuIdx )  ==  V2R1_ONU_NOSETING_P2P )
	{
		vty_out(vty, "\r\n%s/port%d onu%d no peer-to-peer setting\r\n", CardSlot_s[slot], port , (OnuIdx +1));
		return( ROK );
	}
	vty_out(vty, "\r\n%s/port%d onu%d peer-to-peer Info list\r\n", CardSlot_s[slot], port , (OnuIdx +1));
	vty_out(vty, "%s/port%d onu%d peer-to-peer forward rule:\r\n", CardSlot_s[slot], port , (OnuIdx +1));
	ret = GetOnuPeerToPeerForward( PonPortIdx, OnuIdx, &address_not_found, &broadcast );
	if( ret == ROK )
		{
		if( address_not_found == V2R1_ENABLE )
			vty_out(vty, "  address_not_found:forward  ");
		else vty_out(vty, "  address_not_found:discard  ");
		if( broadcast == V2R1_ENABLE )
			vty_out(vty, "broadcast:forward\r\n");
		else vty_out(vty, "broadcast:discard\r\n");	
		}
	else{
		vty_out(vty, "  address_not_found:discard,broadcast:discard");
		}
		
	vty_out(vty, "OnuIdx     Mac addr     Status   address-not-found  broadcast \r\n");
	for( OnuIdx1 = 0; OnuIdx1 < MAXONUPERPON; OnuIdx1++)
		{
		OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx1;
		if((ThisIsValidOnu(PonPortIdx, OnuIdx1) == ROK)
		/*if((CompTwoMacAddress(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr,Invalid_Mac_Addr) == RERROR)*/
			&&(GetOnuPeerToPeer( PonPortIdx,OnuIdx, OnuIdx1 ) == V2R1_ENABLE))
		{
			vty_out(vty, "  %2d    ", (OnuIdx1+1));
			/*for(i=0;i<( BYTES_IN_MAC_ADDRESS-1); i++ )
				{
				vty_out(vty, "%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[i+1] );
				i++;
				if( i != 5 )vty_out(vty, ".");
				}*/
			vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],
								OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],
								OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5] );
			
			if(( GetOnuOperStatus(PonPortIdx , OnuIdx1 ) == ONU_OPER_STATUS_UP ) /*&&(OnuMgmtTable[OnuEntry].LlidTable[0].EntryStatus == LLID_ENTRY_ACTIVE)*/)
				vty_out(vty,  "    UP  ");
			else vty_out(vty, "   Down ");

			ret = GetOnuPeerToPeerForward( PonPortIdx, OnuIdx1, &address_not_found, &broadcast );
			if( ret == ROK )
				{
				if( address_not_found == V2R1_ENABLE )
					vty_out(vty, "        forward  ");
				else vty_out(vty, "        discard  ");
				if( broadcast == V2R1_ENABLE )
					vty_out(vty, "      forward");
				else vty_out(vty, "      discard");	
				}
			else{
				vty_out(vty, "        discard        discard");
				}
			vty_out(vty, "\r\n");
		}		
	}
	return( ROK );
}

int ShowOnuPeerToPeerByVty_1( short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	return(ShowOnuPeerToPeerByVty( PonPortIdx, OnuIdx, vty ));
}

#endif


#ifdef CTC_EXT_OID

int getLlidCtcFecAbility( short int PonPortIdx, short int OnuIdx, ULONG *ability )
{
	int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		ONU_MGMT_SEM_TAKE;
		*ability = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcFecAbility;
		ONU_MGMT_SEM_GIVE;
		return ROK;
	}

	return RERROR;
}

int getLlidCtcFecMode( short int PonPortIdx, short int OnuIdx, ULONG *mode )
{
	short int onuEntry;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		ONU_MGMT_SEM_TAKE;
		*mode = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcFecMode;
		ONU_MGMT_SEM_GIVE;
		return ROK;
	}

	return RERROR;
}

int setLlidCtcFecMode( short int PonPortIdx, short int OnuIdx, const ULONG mode )
{
	short int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		if( mode != 1 )
		{
			/*TODO:add API call to complete operation!*/
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[onuEntry].LlidTable[0].llidCtcFecMode = mode;
			ONU_MGMT_SEM_GIVE;
		}
		return ROK;
	}
	else if( 1 != mode )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcFecMode = mode;
		ONU_MGMT_SEM_GIVE;
	}
	return RERROR;
}

int getLlidCtcEncrypCtrl( short int PonPortIdx, short int OnuIdx, ULONG *ctrl )
{
	short int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		/*TODO:add API call to complete operation!*/
		ONU_MGMT_SEM_TAKE;
		*ctrl = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl;
		ONU_MGMT_SEM_GIVE;
		return ROK;
	}

	return RERROR;
}

int setLlidCtcEncrypCtrl( short int PonPortIdx, short int OnuIdx, const ULONG ctrl )
{
	short int onuEntry = 0;
	int rc = RERROR;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		if( ctrl == 1 )
		{
		    int ret = CTC_StartLlidEncrypt( PonPortIdx, OnuIdx );
		    if( ret == VOS_OK )
		    {
				/*OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = ctrl; encryping enable*/
				rc = ROK;
		    }
		}
		else if( ctrl == 2 )
		{
		    int ret = CTC_StopLlidEncrypt( PonPortIdx, OnuIdx );
			if( ret == VOS_OK )
			{
				/*OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl  = ctrl; noop, encryping disable*/
				rc = ROK;
			}
		}
		else
			rc = RERROR;
		/*OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = (ctrl==2)?4:1; ctrl:2 --start encryp, ret should be encryping*/

	}

	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = ctrl;
	ONU_MGMT_SEM_GIVE;
	
	return rc;
}

int getLlidCtcDbaQuesetNum( short int PonPortIdx, short int OnuIdx, ULONG *num )
{
	short int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		/*TODO:add API call to complete operation!*/
#if 0        
		ONU_MGMT_SEM_TAKE;
		*num = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetNum;
		ONU_MGMT_SEM_GIVE;
#else
        *num = 0;
#endif
		return ROK;
	}

	return RERROR;
}

int setLlidCtcDbaQuesetNum( short int PonPortIdx, short int OnuIdx, const ULONG num )
{
	short int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		/*TODO:add API call to complete operation!*/
#if 0        
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetNum= num;
		ONU_MGMT_SEM_GIVE;
#endif        
		return ROK;
	}
#if 0
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetNum= num;
	ONU_MGMT_SEM_GIVE;
#endif
	return RERROR;
}

int getLlidCtcDbaQueSetCfgStatus( short int PonPortIdx, short int OnuIdx, ULONG *st )
{
	short int onuEntry = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		/*TODO:add API call to complete operation!*/
#if 0        
		ONU_MGMT_SEM_TAKE;
		*st = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetCfgStatus;
		ONU_MGMT_SEM_GIVE;
#else 
        *st = 0;
#endif
		return ROK;
	}

	return RERROR;
}

int setLlidCtcDbaQueSetCfgStatus( short int PonPortIdx, short int OnuIdx, const ULONG st )
{
	short int onuEntry = 0;
	short int onuId;
	OnuLLIDTable_S *pllidEnt;
	int i, j;
	UCHAR	num = 0;
	
	CHECK_ONU_RANGE;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	if( GetOnuOperStatus( PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
	{
		/*TODO:add API call to complete operation!*/
#if 0        
		pllidEnt = &OnuMgmtTable[onuEntry].LlidTable[0];
		if( st == 2 ) /*get operation*/
		{
			CTC_STACK_onu_queue_set_thresholds_t    theshold[MAX_QUEUE_NUMBER];

			onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
		       if( CTC_STACK_EXIT_OK  == CTC_STACK_get_dba_report_thresholds( PonPortIdx, onuId, &num, theshold ) )
		       {
				ONU_MGMT_SEM_TAKE;
				pllidEnt->llidCtcDbaQuesetNum = num;
				for( i=0; i< num; i++ )
				{
					for( j=0; j<MAX_QUEUE_NUMBER; j++ )
					{
						if( theshold[i].queue[j].state == 1 )
						{
							pllidEnt->llidCtcDbaQueset[i].DbaReportBitMap |= (0x80000000>>j);
							pllidEnt->llidCtcDbaQueset[i].DbaQueueThreathold[j]=theshold[i].queue[j].threshold;
						}
						else
							pllidEnt->llidCtcDbaQueset[i].DbaReportBitMap &= (~(0x80000000>>j));
					}
				}
				ONU_MGMT_SEM_GIVE;
				return ROK;
	        	}
		}
		else if( st == 3 )/*set operation*/
		{
			CTC_STACK_onu_queue_set_thresholds_t setVal[MAX_QUEUE_NUMBER];
			
			onuId = GetLlidByOnuIdx( PonPortIdx, OnuIdx );

			ONU_MGMT_SEM_TAKE;
			pllidEnt = &OnuMgmtTable[onuEntry].LlidTable[0];
			num = pllidEnt->llidCtcDbaQuesetNum;
			for( i=0; i<num; i++ )
			{
				for( j=0; j<MAX_QUEUE_NUMBER; j++ )
				{
					setVal[i].queue[j].state = (pllidEnt->llidCtcDbaQueset[i].DbaReportBitMap&(0x80000000>>j))?1:0;
					if( setVal[i].queue[j].state == 1 )
						setVal[i].queue[j].threshold = pllidEnt->llidCtcDbaQueset[i].DbaQueueThreathold[j];
				}
			}
			ONU_MGMT_SEM_GIVE;
			
			if( CTC_STACK_set_dba_report_thresholds( PonPortIdx, onuId, &num, setVal ) == CTC_STACK_EXIT_OK )
				return ROK;
		}
#endif        
	}
	return RERROR;
}

/*
void    initLlidDbaQueset( short int pon, short int onu, CTC_STACK_onu_queue_set_thresholds_t *queset, UCHAR num )
{
	bitsdef QuesetSt;
	short int i = 0, j=0;
	
	short int onuEntry = pon*MAXONUPERPON+onu;

	OnuLLIDTable_S *pLlidtable = &OnuMgmtTable[onuEntry].LlidTable[0];
	
	for( i=0; i<num; i++ )
	{
		ULONG  st = 0;
		for( j=0; j<MAX_QUEUE_NUMBER; j++ )
		{
			
			pLlidtable->llidCtcDbaQueset[i].DbaQueueThreathold[j] = queset[i].queue.threshold[j];
			if( queset[j].queue.state == 1 )
				st |= (0x80000000>>j);
		}
		pLlidtable->llidCtcDbaQueset[i].DbaReportBitMap = st;
	}
} 
*/

STATUS	getLlidDbaQueset( short int PonPortIdx, short int OnuIdx, short int queidx, _queset_t *data )
{
	STATUS	rc = VOS_ERROR;
	short int onuEntry;
	
	CHECK_ONU_RANGE;
	if( data == NULL )
		return rc;
	if( MAX_QUEUE_NUMBER <= queidx )
		return rc;
	
	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
#if 0
	ONU_MGMT_SEM_TAKE;
	data->DbaReportBitMap = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQueset[queidx].DbaReportBitMap;
	VOS_MemCpy( data->DbaQueueThreathold, OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQueset[queidx].DbaQueueThreathold,
		MAX_QUEUE_NUMBER*sizeof(ULONG ) );
	ONU_MGMT_SEM_GIVE;
#else
    data->DbaReportBitMap = 0;
    VOS_MemZero( data->DbaQueueThreathold, MAX_QUEUE_NUMBER*sizeof(ULONG ) );
#endif
	rc = VOS_OK;
	
	return rc;
}

STATUS  setLlidDbaQuesetBitmap( short int PonPortIdx , short int OnuIdx, short int queidx, ULONG map )
{
	short int onuEntry;
	
	CHECK_ONU_RANGE;
	if( MAX_QUEUE_NUMBER <= queidx )
		return VOS_ERROR;
	
	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
#if 0
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQueset[queidx].DbaReportBitMap = map;
	ONU_MGMT_SEM_GIVE;
#endif
	return VOS_OK;
}

STATUS  setLlidDbaQuesetThreathold( short int PonPortIdx , short int OnuIdx, short int quesetidx, short int queidx,  ULONG val )
{
	short int onuEntry;
	
	CHECK_ONU_RANGE;
	if( (MAX_QUEUE_NUMBER <= quesetidx) || (8 <= queidx) )
		return VOS_ERROR;

	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
#if 0
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQueset[quesetidx].DbaQueueThreathold[queidx] = val;
	ONU_MGMT_SEM_GIVE;
#endif
	return VOS_OK;
}

ULONG  getOnuEthPortNum( short int PonPortIdx, short int OnuIdx )
{
	ULONG onuEntry;
	ULONG num;
	CHECK_ONU_RANGE;
	onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
	ONU_MGMT_SEM_TAKE;
	num = (OnuMgmtTable[onuEntry].FE_Ethernet_ports_number+OnuMgmtTable[onuEntry].GE_Ethernet_ports_number);
	ONU_MGMT_SEM_GIVE;
	return num;
}

UCHAR*	getOnuPortDistribution( ULONG onuEntry )
{
	static UCHAR portlist[16];
	if( onuEntry < MAXONU )
	{
		ONU_MGMT_SEM_TAKE;
		VOS_MemCpy( portlist, OnuMgmtTable[onuEntry].Ports_distribution, sizeof(portlist) );
		ONU_MGMT_SEM_GIVE;
	}
	else
	{
		VOS_MemZero( portlist, sizeof(portlist) );
	}
	return portlist;
}

void	setOnuStpEnableStatus( ULONG onuEntry, ULONG st )
{
	if( onuEntry < MAXONU )
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[onuEntry].stpEnable = st;
		ONU_MGMT_SEM_GIVE;
	}
}

ULONG  getOnuStpEnableStatus( ULONG onuEntry )
{
	ULONG enable = 0;
	if( onuEntry < MAXONU )
	{
		ONU_MGMT_SEM_TAKE;
		enable = (ULONG)OnuMgmtTable[onuEntry].stpEnable;
		ONU_MGMT_SEM_GIVE;
	}
	return enable;
}

#endif

/**************************************************************
 *
 *    Function:  int IsPas6201Onu(short int PonPortIdx, short int OnuIdx)
 *
 *    Param:  short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx       -- the specific Onu
 *
 *    Desc:  判断ONU使用的芯片类型(PAS6201, or PAS6301),
 *
 *    Return:   
 *
 *    Notes: 
 *
 *    modified:
 *
 ***************************************************************/
int IsPas6201Onu(short int PonPortIdx, short int OnuIdx)
{
	int OnuType = V2R1_DEVICE_UNKNOWN;
#if 0    
	short int ret = PAS_EXIT_ERROR;
	PON_onu_versions   onu_versions;
#endif
	
	CHECK_ONU_RANGE;

	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return (RERROR );

#if 1    
    if ( PON_PASSAVE_ONU_6201_VERSION == GetOnuChipType( PonPortIdx, OnuIdx ) )
    {
    	return( ROK );
    }
#else
#if  1       
    ret= OnuMgt_GetOnuVer(PonPortIdx, OnuIdx,  &onu_versions);
    if( ret == OLT_ERR_OK )
    {
        if(onu_versions.hardware_version == PON_PASSAVE_ONU_6201_VERSION )
        	return( ROK );
    }
#else
	if(GetOnuOperStatus( PonPortIdx, OnuIdx )  == ONU_OPER_STATUS_UP )
		{
		short int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if( llid != INVALID_LLID )
			{
			ret = PAS_get_onu_version(PonPortIdx, llid,  &onu_versions);	
			if( ret == PAS_EXIT_OK )
				{
				if(onu_versions.hardware_version == PON_PASSAVE_ONU_6201_VERSION )
					return( ROK );
				}
			}
		}
#endif
#endif
	GetOnuType(PonPortIdx, OnuIdx, &OnuType );
	if((OnuType == V2R1_ONU_GT811 ) || (OnuType == V2R1_ONU_GT812 ) || (OnuType == V2R1_ONU_GT831 ) || (OnuType == V2R1_ONU_GT831_CATV ))
		return(ROK );
	else return( RERROR );

}

int IsPas6301Onu(short int PonPortIdx, short int OnuIdx)
{
	int OnuType = V2R1_DEVICE_UNKNOWN;
	/*int OnuChipType = 0;*/
#if 0    
	short int ret = PAS_EXIT_ERROR;
	PON_onu_versions   onu_versions;
#endif
	
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return (RERROR );

#if 1
    if ( PON_PASSAVE_ONU_6301_VERSION == GetOnuChipType( PonPortIdx, OnuIdx ) )
    {
    	return( ROK );
    }
#else
#if 1    
    ret= OnuMgt_GetOnuVer(PonPortIdx, OnuIdx,  &onu_versions);
    if( ret == OLT_ERR_OK )
    {
        if(onu_versions.hardware_version == PON_PASSAVE_ONU_6301_VERSION )
        	return( ROK );
    }
#else
	if(GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP )
		{
		short int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if( llid != INVALID_LLID )
			{
			ret = PAS_get_onu_version(PonPortIdx, llid,  &onu_versions);
			/* modified by chenfj 2009-4-2
			     判断ONU芯片是否为6301时,若ONU在线, 则取ONU的硬件版本,
			     并根据返回值作出判断; 此处if 判断弄反了,但不影响使用
			    */
			if( ret == PAS_EXIT_OK )
				{
				if(onu_versions.hardware_version == PON_PASSAVE_ONU_6301_VERSION )
					return( ROK );
				}
			}
		}
#endif
#endif

	GetOnuType(PonPortIdx, OnuIdx, &OnuType );

	if((OnuType != V2R1_OTHER ) && (OnuType != V2R1_ONU_GT811 ) && (OnuType != V2R1_ONU_GT812 ) &&  (OnuType != V2R1_ONU_GT831 ) && (OnuType != V2R1_ONU_GT831_CATV ))
		return( ROK );
	else return (RERROR );

}

/**************************************************************
 *
 *    Function:  int IsSupportPotsService(unsigned long OnuDeviceIdx, bool *SupportVoice)
 *
 *    Param:  unsigned long OnuDeviceIdx  --ONU设备索引
 *                bool *SupportVoice  -- ONU是否支持语音指示
 *
 *    Desc:  根据ONU类型，判断ONU是否支持POTS(语音)业务
 *
 *    Return:   ROK －函数返回正确，*SupportVoice ＝ 1 支持语音，*SupportVoice ＝ 2，不支持语音； 
 *                  RERROR -- 函数执行错误
 *
 *    Notes: 当前支持GT865支持POTS业务；以后有新类型ONU支持POTS业务时，本函数需扩展
 *
 *    modified:
 *
 ***************************************************************/
/*int IsSupportPotsService(short int PonPortIdx, short int OnuIdx)*/
int OnuIsSupportVoice(unsigned long OnuDeviceIdx, bool *SupportVoice)
{
	int OnuType = V2R1_DEVICE_UNKNOWN;
	short int PonPortIdx, OnuIdx;
	int Slot, Port;

	if( SupportVoice == NULL ) return( RERROR );
	*SupportVoice = FALSE;

	Slot = GET_PONSLOT(OnuDeviceIdx)/*( OnuDeviceIdx / 10000)*/;
	Port = GET_PONPORT(OnuDeviceIdx)/*(OnuDeviceIdx % 10000 ) / 1000*/;
	OnuIdx =GET_ONUID(OnuDeviceIdx)/* (OnuDeviceIdx % 1000) */- 1;
    
	PonPortIdx = GetPonPortIdxBySlot(Slot, Port);
	
	CHECK_ONU_RANGE

	if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return (RERROR );
	
	if(GetOnuType(PonPortIdx, OnuIdx, &OnuType )!= ROK)
		{
		*SupportVoice = FALSE;
		return( RERROR);
		}
	
	if((OnuType == V2R1_ONU_GT865 )||(OnuType == V2R1_ONU_GT861) ||((OnuType==V2R1_OTHER)&& (GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)))
		{
		*SupportVoice = TRUE;
		return( ROK );
		}
	else {
		*SupportVoice = FALSE;
		return( ROK );
		}

	return( RERROR );
}

/**************************************************************
 *
 *    Function:  int  EncryptIsSupported( short int PonPortIdx, short int OnuIdx)
 *
 *    Param:  short int PonPortIdx  -- the specific pon port
 *                short int OnuIdx       -- the specific Onu
 *
 *    Desc:  判断加密时，ONU使用的芯片类型(PAS6201, or PAS6301)是否与OLT侧PON芯片(PAS5001, or PAS5201)匹配
 *
 *    Return:   ROK －类型匹配；RERROR －类型不匹配
 *
 *    Notes: 当前支持的匹配类型有：
 *               	 OLT pas5001 -- ONU pas6201, ONU类型有GT811/GT812/GT821/GT831
 *                   OLT pas5201 -- ONU pas6301, ONU类型有GT813/GT810/GT816 etc.
 *
 *    modified:
 *
 ***************************************************************/
/* added by chenfj 2007-9-13
	在对ONU加密时，判断ONU与OLT类型是否匹配
	当前支持的匹配类型有：
	OLT pas5001 -- ONU pas6201, ONU类型有GT811/GT812/GT821/GT831
	OLT pas5201 -- ONU pas6301, ONU类型有GT813/GT810/GT816 etc.

	modified by chenfj 2008-5-12
	使用已有的函数, 来判断OLT PON 芯片与ONU PON 芯片加密时是否匹配
*/
int  EncryptIsSupported( short int PonPortIdx, short int OnuIdx)
{
	short int PonChipType;
	/*int OnuType = V2R1_OTHER;
	int ret;	*/
	
	CHECK_ONU_RANGE

	/*if( ThisIsValidOnu( PonPortIdx, OnuIdx ) != ROK ) return (RERROR );
	return( ROK);*/
/*
	modified by chenfj 2008-12-30
	在系统启动恢复数据时及在ONU 注册前, 无法判断pon 类型匹配;
	默认为支持
*/
	if(!(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))) return(ROK);
	if(GetOnuOperStatus(PonPortIdx,OnuIdx) != ONU_OPER_STATUS_UP ) return( ROK);
	
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	/*
	ret = GetOnuType(PonPortIdx, OnuIdx, &OnuType );
	if(( OnuType < V2R1_ONU_GT811 ) ||(OnuType >= V2R1_DEVICE_LAST )) return (RERROR );
	*/
	/*if(PonChipType == PONCHIP_PAS5001 )
		{
		if(IsPas6201Onu(PonPortIdx, OnuIdx) == ROK )
			return(ROK );
		}
	
	else*/ if(OLT_PONCHIP_ISPAS(PonChipType))
		{
		if(IsPas6301Onu(PonPortIdx, OnuIdx) == ROK )
		/*if((OnuType != V2R1_ONU_GT811 ) && (OnuType != V2R1_ONU_GT812 ) &&  (OnuType != V2R1_ONU_GT831 ) && (OnuType != V2R1_ONU_GT831_CATV ))*/
			return( ROK );
		}
	
	else  /*if( OLT_PONCHIP_ISTK(PonChipType) )*/	/* modified by xieshl 20160107, 先标注一下，为防止PON异常，TK暂不支持 */
		{ /* other pon chip type handler */
		return( RERROR );
		}

	return( RERROR );

}

/* chenfj 2008-10-28
     修改函数含义, 用于判断这个ONU 的EPON 程序中是否包含有voice 程序
     GT831 类型ONU支持voip 语音和数据,但只有一个APP 程序
	GT866 类型ONU支持voip 语音和数据有一个APP 程序和voip 程序
*/
int OnuEponAppHasVoiceApp(short int PonPortIdx, short int OnuIdx)
{
	int OnuType;
	int ret;
	
	CHECK_ONU_RANGE

	ret = GetOnuType(PonPortIdx, OnuIdx, &OnuType );

	if( ret != ROK ) return (RERROR );

	if(( OnuType == V2R1_ONU_GT831) || ( OnuType == V2R1_ONU_GT831_A) || 
		( OnuType == V2R1_ONU_GT831_CATV) || ( OnuType == V2R1_ONU_GT831_A_CATV) /*||( OnuType == V2R1_ONU_GT831_B)*/||
		(OnuType == V2R1_ONU_GT866) || ( OnuType == V2R1_ONU_GT861)||(OnuType == V2R1_ONU_GT863))
		return( ROK );
	
	else if(VOS_StrCmp(GetOnuVoiceAppString(OnuType),DEVICE_APP_TYPE_UNKNOWN) != 0)
		return ROK;

	else return( RERROR );

}

int OnuIsGT831(short int PonPortIdx, short int OnuIdx)
{
	int OnuType;
	int ret;
	
	CHECK_ONU_RANGE

	ret = GetOnuType(PonPortIdx, OnuIdx, &OnuType );

	if( ret != ROK ) return (RERROR );

	if(( OnuType == V2R1_ONU_GT831) || ( OnuType == V2R1_ONU_GT831_A) || ( OnuType == V2R1_ONU_GT831_CATV) || ( OnuType == V2R1_ONU_GT831_A_CATV) || ( OnuType == V2R1_ONU_GT831_B) || ( OnuType == V2R1_ONU_GT831_B_CATV ))
		return( ROK );
	else return( RERROR );

}
/* chenfj 2008-10-28
     修改函数含义, 用于判断这个ONU 的firmware 程序中是否包含有fpga 程序
*/
int OnuFirmwareHasFpgaApp(short int PonPortIdx, short int OnuIdx)
{
	int OnuType;
	int ret;
	
	CHECK_ONU_RANGE

	ret = GetOnuType(PonPortIdx, OnuIdx, &OnuType );

	if( ret != ROK ) return (RERROR );

	if((OnuType == V2R1_ONU_GT865) || (OnuType == V2R1_ONU_GT861))
		return( ROK );

	else if(VOS_StrCmp(GetOnuFpgaAppString(OnuType),DEVICE_APP_TYPE_UNKNOWN) != 0)
		return ROK;
	else return( RERROR );

}
/* added by chenfj 2008-10-28
     这个函数用于将以'/' 相连的字符串分为两部分
     如将V1R01B012/V1R00B001 分成两个字符串: V1R01B012, V1R00B001
     若在输入的字符串参数中没有'/', 则函数返回OK, 但len1 = 0; len2=0;
     所以在调用这个函数时, 需判断函数的返回值, 同时应判断返回参数len1, len2
*/
int  ParsePrefixAndSuffixFromString(unsigned char *String, unsigned char *Prefix, unsigned char *PrefixLen, unsigned char *Suffix, unsigned char *SuffixLen)
{
	unsigned char *Ptr = NULL;
	unsigned char *Ptr1 = String;
	
	if(String == NULL ) return( ROK);
	if((Prefix == NULL ) || ( PrefixLen == NULL ) ||(Suffix == NULL)||(SuffixLen == NULL)) return( RERROR );

	*PrefixLen = 0;
	*SuffixLen = 0;
	Ptr = VOS_StrStr( Ptr1, "/");
	if( Ptr != NULL )
		{
		*PrefixLen = Ptr - Ptr1;
		VOS_MemCpy(Prefix, Ptr1, *PrefixLen);
		Prefix[*PrefixLen] = '\0';
		Ptr++;
		*SuffixLen = VOS_StrLen( Ptr);
		VOS_MemCpy(Suffix, Ptr, *SuffixLen);
		Suffix[*SuffixLen] = '\0';
		}
	return(ROK);
}

#if 0	/* removed by xieshl 20091216 */
unsigned int v3_fixed_bw = 2;
unsigned int v3_fixed_bw_fine = 1;
unsigned int v3_fixed_packet_size = 64;
unsigned int v3_max_be_bw = 16;
unsigned int v3_max_be_bw_fine = 0;
unsigned int v3_max_gr_bw = 15;
unsigned int v3_max_gr_bw_fine = 0;

/*#ifdef PLATO_DBA_V3 */
int  TestPlatoDbaV3(short int PonPortIdx, short int OnuIdx )
{
	PLATO3_SLA_t v3_sla;
	short int ret;
	short int llid;
	short  v3_DBA_error_code;

	v3_sla.class = 2;
	v3_sla.fixed_bw = v3_fixed_bw;
	v3_sla.fixed_bw_fine = v3_fixed_bw_fine;
	v3_sla.fixed_packet_size = v3_fixed_packet_size;
	v3_sla.max_be_bw = v3_max_be_bw;
	v3_sla.max_be_bw_fine = v3_max_be_bw_fine;
	v3_sla.max_gr_bw = v3_max_gr_bw;
	v3_sla.max_gr_bw_fine = v3_max_gr_bw_fine;

	llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	
	ret = PLATO3_set_SLA(PonPortIdx, llid, &v3_sla, &v3_DBA_error_code );

	if( ret == PAS_EXIT_OK )
		{
		sys_console_printf("set sla ok, dba-error-code=%d\r\n", v3_DBA_error_code );
		}
	else sys_console_printf("set sla err, error code=%d\r\n", ret );

	return( ROK );

}
#endif
/*  modified by chenfj 2008-2-1
    BUG 描述:
    若只升级GT831的语音程序;在升级结束后,GT831当前运行的语音程序
    版本与OLT上ONU设备信息中的版本号可能不一致
    
	改正:
	由于在ONU OAM消息定义中没有可用来同步语音程序版本的消息,
	故先采取一个补救措施:在GT831升级语音程序时,待得到其语音
	程序写入到FLASH的报告后,将OLT FLASH中的GT831语音程序版本作为
	当前运行版本
  */
 #if 0
int ModifyOnuVoiceVersion(short int PonPortIdx, short int OnuIdx )
{
	char Ver1[ONU_VERSION_LEN+1] = "\0";
	char Ver2[ONU_VERSION_LEN+1] = "\0";
	unsigned char *Ptr;
	int OnuType, length1,length2, offset,flag,fact_len;
	long ret;
	
	CHECK_ONU_RANGE

	if( OnuIsGT831(PonPortIdx, OnuIdx) == ROK )
		{
		if(GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK )
			return( RERROR );
		ret = get_ONU_Info(OnuVoiceType_1[OnuType], &offset, &length1, &flag, &fact_len, (int *)Ver1);
		if((ret == VOS_OK ) && ((length1 = VOS_StrLen(Ver1)) != 0))
			{
			length2 = 0;
			GetOnuSWVersion(PonPortIdx, OnuIdx, Ver2, &length2) ;
			if( length2 == 0 ) return( RERROR );
			Ptr = VOS_StrStr(Ver2, "/");
			Ptr++;
			Ver2[(int)Ptr - (int)&Ver2[0]]='\0';
			Ptr = &Ver2[0];
			
			if(( VOS_StrLen(Ptr) + length1 ) > MAXSWVERSIONLEN )
				{
				length1 = MAXSWVERSIONLEN - VOS_StrLen(Ptr);
				Ver1[length1] = '\0';
				}
				
			VOS_StrCat(Ptr, &Ver1[0]);
			Ptr = &Ver2[0];
			length2 = VOS_StrLen(Ptr);
			SetOnuSWVersion(PonPortIdx,OnuIdx, Ptr, length2);
			}
		else return( RERROR );
		}	
	return( ROK );
}
#endif

int  SearchOnuByType(short int PonPortIdx, unsigned char *OnuTypeString )
{
	short int OnuIdx;
	int OnuType;

	CHECK_PON_RANGE

	if(OnuTypeString == NULL ) return( RERROR );

	for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
	{
		if(ThisIsValidOnu(PonPortIdx, OnuIdx ) == ROK )
		{

		    if( GetOnuType(PonPortIdx, OnuIdx, &OnuType) != ROK ) continue;

			if(OnuType == V2R1_ONU_GPON)
			{
			 	char name[ONU_TYPE_LEN+1]="";
		        int namelen = 0;
		        if(GetOnuModel(PonPortIdx, OnuIdx, name, &namelen) == VOS_OK &&
		        	VOS_StrniCmp(OnuTypeString, name, VOS_StrLen(OnuTypeString)) == 0)
		       	return (ROK);
			}
			else if(OnuType == V2R1_ONU_CTC)
		    {
		        char name[ONU_TYPE_LEN+1]="";
		        int namelen = 0;
		        if(GetOnuModel(PonPortIdx, OnuIdx, name, &namelen) == VOS_OK &&
		                VOS_StrniCmp(OnuTypeString, name, VOS_StrLen(OnuTypeString)) == 0)
		            return (ROK);
		    }
			else
			{
                if( VOS_StrniCmp(OnuTypeString, GetDeviceTypeString(OnuType), VOS_StrLen(OnuTypeString)) == 0 )
                    return( ROK );
			}
		    
		}
	}
	return( RERROR );
}

int GetOnuEuqInfoRetryCounter(short int PonPortIdx, short int OnuIdx )
{
	int counter;
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	counter = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].GetEUQCounter;
	ONU_MGMT_SEM_GIVE;
	return counter;
}
/* end */



int SetOnuAutoConfigFlag( short int PonPortIdx, short int OnuIdx, unsigned char AutoConfig )
{
	CHECK_ONU_RANGE

	if( (AutoConfig== V2R1_ONU_AUTO_CONFIG_FLAG ) ||( AutoConfig == NO_V2R1_ONU_AUTO_CONFIG_FLAG ))
	{
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.AutoConfigFlag = AutoConfig;
		ONU_MGMT_SEM_GIVE;
		return( ROK );
	}
	else return( RERROR );
}


int GetOnuAutoConfigFlag( short int PonPortIdx, short int OnuIdx )
{
	int flag;
	CHECK_ONU_RANGE;
	ONU_MGMT_SEM_TAKE;
	flag = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].DeviceInfo.AutoConfigFlag;
	ONU_MGMT_SEM_GIVE;
	return flag;
}

/* added by chenfj 2008-3-25 
     用于保存ONU 业务优先级设置
     */

int SetOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx, unsigned char flag, PON_olt_vlan_uplink_config_t    vlan_uplink_config )
{
	short int OnuEntry;
	
	CHECK_ONU_RANGE

	if((flag != V2R1_DOWN ) && ( flag != V2R1_UP )) return ( RERROR );

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].vlan_priority.EthType = vlan_uplink_config.vlan_type;
	OnuMgmtTable[OnuEntry].vlan_priority.vid = vlan_uplink_config.new_vlan_tag_id;
	OnuMgmtTable[OnuEntry].vlan_priority.Priority = vlan_uplink_config.vlan_priority;
	OnuMgmtTable[OnuEntry].vlan_priority.row_status = flag;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

int GetOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx, unsigned char *flag, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
	short int OnuEntry;

	CHECK_ONU_RANGE;
	if(( vlan_uplink_config == NULL ) || ( flag == NULL )) return( RERROR );

	OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
	ONU_MGMT_SEM_TAKE;
	*flag = OnuMgmtTable[OnuEntry].vlan_priority.row_status;
	vlan_uplink_config->vlan_priority = OnuMgmtTable[OnuEntry].vlan_priority.Priority;
	vlan_uplink_config->new_vlan_tag_id = OnuMgmtTable[OnuEntry].vlan_priority.vid;
	vlan_uplink_config->vlan_type = OnuMgmtTable[OnuEntry].vlan_priority.EthType;
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}

int RestoreOnuUplinkVlanPriority(short int PonPortIdx, short int OnuIdx )
{
	PON_olt_vlan_uplink_config_t vlan_uplink_config;
	unsigned char flag = V2R1_DOWN;
	short int Onu_llid;
	short int PonChipType;
	
	OLT_vlan_qinq_t vlan_qinq_config;

	
	VOS_MemSet(&vlan_qinq_config, 0, sizeof(vlan_qinq_config));
	CHECK_ONU_RANGE

	if( GetOnuUplinkVlanPriority(PonPortIdx, OnuIdx, &flag, &vlan_uplink_config ) != ROK ) return ( RERROR );
	if( flag == V2R1_DOWN ) return (ROK );
	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) return ( RERROR );
	Onu_llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( Onu_llid == INVALID_LLID ) return (RERROR );

   
	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	if(OLT_PONCHIP_ISPAS(PonChipType)&&!OLT_PONCHIP_ISPAS5001(PonChipType))
	{
		vlan_uplink_config.untagged_frames_authentication_vid = PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0;
		vlan_uplink_config.authenticated_vid = PON_ALL_FRAMES_AUTHENTICATE;
		vlan_uplink_config.discard_untagged = FALSE;
		vlan_uplink_config.discard_tagged = FALSE;
		vlan_uplink_config.discard_null_tagged = FALSE;
		vlan_uplink_config.discard_nested = FALSE;
		vlan_uplink_config.vlan_manipulation = PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED;
		vlan_uplink_config.vlan_type = PON_OLT_VLAN_ETHERNET_TYPE_8100;

		#if 1
		vlan_qinq_config.qinq_direction = OLT_CFG_DIR_UPLINK;
		vlan_qinq_config.qinq_objectid = OnuIdx;
		vlan_qinq_config.qinq_cfg.up_cfg = vlan_uplink_config;
		if( OLT_SetVlanQinQ(PonPortIdx, &vlan_qinq_config) == PAS_EXIT_OK )
		#else
		if( PAS_set_llid_vlan_mode(PonPortIdx, Onu_llid, vlan_uplink_config) == PAS_EXIT_OK )
		#endif/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			return( ROK );
		else return( RERROR );
		}
	
	else{ /* 其他PON芯片类型处理*/

		}

	return ( RERROR );
	
}

	/*问题单: 6548
		813在线，显示的状态是powerDown
		经测试发现, 快速开关ONU电源, ONU只上报了掉电, 但并未重新启动(从ONU串口上观察到), 也就没有重新注册; 故出现问题单中所述
  		增加一个容错处理: ONU 瞬间掉电, 但并未重新注册情况
  		在ONU 管理表中增加PowerOffCounter 标签; 若收到ONU掉电OAM 帧, 不直接上报ONU掉电告警;而是在PowerOffCounter计数内(当前是5秒)等待ONU离线消息;若能等到ONU离线消息,则上报ONU掉电告警;若等不到, 则认为是一次瞬间掉电(误告警), 不做处理
		 */

int ScanOnuPowerOffCounter()
{
	short int PonPortIdx, OnuIdx;
	int onuEntry;

	ONU_MGMT_SEM_TAKE;
	for( PonPortIdx =0; PonPortIdx < MAXPON; PonPortIdx ++ )
	{
		if( PonPortIsWorking(PonPortIdx) != TRUE ) continue;

		onuEntry = PonPortIdx * MAXONUPERPON;
		for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++ )
		{
			/*if( ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK ) continue;
			if( GetOnuOperStatus( PonPortIdx, OnuIdx ) !=  ONU_OPER_STATUS_UP )
				continue;*/
			if( ONU_OPER_STATUS_UP != OnuMgmtTable[onuEntry + OnuIdx ].OperStatus )
				continue;

			if(OnuMgmtTable[onuEntry + OnuIdx].PowerOffCounter > 0 ) 
				OnuMgmtTable[onuEntry + OnuIdx].PowerOffCounter --;
		}
	}
	ONU_MGMT_SEM_GIVE;
	
	return( ROK );
}


/* added by chenfj 2008-12-9
	增加以下函数, 供ONU 自动升级和自动配置调用
*/

/*******************************************************************
	参数介绍: 
		输入参数:
			int slot -- ONU所在的物理槽位号
			int port -- ONU所在的物理端口号
			int onuid -- ONU 编号, 范围1-64
		输出参数:
			unsigned char *Version -- 当前运行版本; 在调用函数时, 应先申请此参数, 空间长度不小于32字节
			int  *VersionLength  -- 版本长度 
		返回值:  
			ROK / RERROR
		注:
			只当函数返回ROK 时, 输出参数有效;
*********************************************************************/

/*   读取ONU APP 程序版本 */
int  GetOnuDeviceAppVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength)
{
	short int PonPortIdx, OnuIdx;
	unsigned char *Ptr = NULL;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	OnuIdx = onuid - 1;

	if(GetOnuSWVersion(PonPortIdx, OnuIdx, Version, VersionLength) != ROK ) return( RERROR);
	
	Ptr = VOS_StrStr(Version, "/");
	if(Ptr != NULL )
		Version[Ptr -&Version[0]] ='\0';
	*VersionLength = VOS_StrLen(Version);
	
	return(ROK);
}

/*   读取ONU voice 程序版本 */
int  GetOnuDeviceVoiceVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength)
{
	short int PonPortIdx, OnuIdx;
	unsigned char *Ptr = NULL;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	OnuIdx = onuid - 1;

	if(GetOnuSWVersion(PonPortIdx, OnuIdx, Version, VersionLength) != ROK ) return( RERROR);

	Ptr = VOS_StrStr( Version, "/");
	if(Ptr != NULL )
		{
		Ptr++;
		*VersionLength = VOS_StrLen( Ptr );
		VOS_StrCpy(Version, Ptr);
		Version[*VersionLength ]='\0';
		}
	else{ 
		Version[0] = '\0';
		*VersionLength = 0;
		}
	
	return(ROK);
}

/*   读取ONU firmware 版本 */
int  GetOnuDeviceFrimwareVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength)
{
	short int PonPortIdx, OnuIdx;
	unsigned char *Ptr = NULL;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	OnuIdx = onuid - 1;

	if(GetOnuFWVersion(PonPortIdx, OnuIdx, Version, VersionLength) != ROK ) return( RERROR);
	
	Ptr = VOS_StrStr(Version, "/");
	if(Ptr != NULL )
		Version[Ptr -&Version[0]] ='\0';
	*VersionLength = VOS_StrLen(Version);
	
	return(ROK);
}

/*   读取ONU fpga 版本 */
int  GetOnuDeviceFpgaVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength)
{
	short int PonPortIdx, OnuIdx;
	unsigned char *Ptr = NULL;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	OnuIdx = onuid - 1;

	if(GetOnuFWVersion(PonPortIdx, OnuIdx, Version, VersionLength) != ROK ) return( RERROR);
	
	Ptr = VOS_StrStr( Version, "/");
	if(Ptr != NULL )
		{
		Ptr++;
		*VersionLength = VOS_StrLen( Ptr );
		VOS_StrCpy(Version, Ptr );
		Version[*VersionLength ]='\0';
		}
	else{ 
		Version[0] = '\0';
		*VersionLength = 0;
		}
	
	return(ROK);
}

/*   读取ONU boot 版本 */
int  GetOnuDeviceBootVersion(int slot, int port, int onuid, unsigned char *Version, int  *VersionLength)
{
	short int PonPortIdx, OnuIdx;

	if(PonCardSlotPortCheckByVty(slot, port, 0) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((short int)slot, (short int)port);
	OnuIdx = onuid - 1;

	return (GetOnuBootVersion(PonPortIdx, OnuIdx, Version, VersionLength));
}
int GetMaxOnuByPonPort(short int PonPortIdx)
{
    short int slot = GetCardIdxByPonChip(PonPortIdx);
    short int port = GetPonPortByPonChip(PonPortIdx);
    
    if(OLT_SLOT_ISVALID(slot) && OLT_PORT_ISVALID(port))   
        return glMaxOnuTable[slot-1][port-1];  
    else
        return VOS_ERROR;
}
int SetMaxOnuByPonPort(short int PonPortIdx, int max_onu)
{
    short int slot = GetCardIdxByPonChip(PonPortIdx);
    short int port = GetPonPortByPonChip(PonPortIdx);
    /*已经改为非默认值的，不再根据全局配置进行修改*/
    if(!(glMaxOnuTable[slot-1][port-1]&CONFIG_DEFAULT_MAX_ONU_FLAG)
        && (max_onu & CONFIG_CODE_ALL_PORT))
        return VOS_ERROR;
    
    if(OLT_SLOT_ISVALID(slot) && OLT_PORT_ISVALID(port))   
        glMaxOnuTable[slot-1][port-1] = max_onu & (~CONFIG_CODE_ALL_PORT);  

    return VOS_OK;
}
LONG CopyMaxOnuConfig(short int DstPonPortIdx, short int SrcPonPortIdx, int CopyFlags)
{
    int liv_slot = GetCardIdxByPonChip(SrcPonPortIdx);
    int liv_pon = GetPonPortByPonChip(SrcPonPortIdx);
    ULONG S_MaxOnu = 0;
    ULONG D_MaxOnu = 0;
    if(liv_slot == RERROR || liv_pon == RERROR)
        return VOS_ERROR;
	/*if( (SrcPonPortIdx < 0) || (SrcPonPortIdx >= MAXPON)||(DstPonPortIdx < 0) || (DstPonPortIdx >= MAXPON)  )
	{
		return VOS_ERROR;    	
	}*/

    S_MaxOnu = GetMaxOnuByPonPort(SrcPonPortIdx);
    
    if ( SrcPonPortIdx == DstPonPortIdx )
    {
        /* 自拷贝，应该是配置恢复 */
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE && MaxOnuDefault != (S_MaxOnu&0xff))     
        {
            OLT_SetMaxOnu(SrcPonPortIdx, S_MaxOnu);
        }      
    }
    else
    {
        /*do nothing*/	          
    }
  
    return VOS_OK;
}

/* added by xieshl 20120605, 基于ONU管理表重新改写检索函数，原函数定义在gweponsys.c中 */
#ifndef CTC_OBSOLETE
/*STATUS setOnuStatus( const ulong_t slot, const ulong_t port, const ulong_t onuindex, OnuStatus status )
{
	return VOS_OK;
}*/

STATUS checkOnuStatus( ulong_t slot, ulong_t port, ulong_t onuId )
{
	SHORT PonPortIdx, OnuIdx;
	
	if( (slot < PONCARD_FIRST) || (slot > PONCARD_LAST/* SYS_CHASSIS_SLOTNUM*/) || /* modified by chenfj 2009-5-18*/
		(port == 0) || (port > MAX_PONPORT_PER_BOARD) ||
		(onuId == 0) || (onuId > MAX_ONU_PER_PONPORT) )
		return VOS_ERROR;
	
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx = onuId - 1;
	if( PonPortIdx == RERROR )
		return VOS_ERROR;

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) == ROK )
		return VOS_OK;

	return VOS_ERROR;
}

#if 1
ULONG findFirstOnu( ULONG onuType )
{
    int slot, port;
    int type;
    short int PonPortIdx, OnuIdx;
    int entrybase;
    UCHAR *pMac;
 
    ONU_MGMT_SEM_TAKE;
    for( PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++ )
    {
        /*if( !PonPortIsWorking(PonPortIdx) )
                continue;*/
        entrybase = PonPortIdx*MAXONUPERPON;
        for( OnuIdx=0; OnuIdx<MAX_ONU_PER_PONPORT; OnuIdx++ )
        {
            /*if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
                         continue;*/
            type = 0;
			slot = GetCardIdxByPonChip(PonPortIdx);
            if(SYS_MODULE_IS_GPON(slot))
        	{
            	if(VOS_StrLen(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.DeviceSerial_No) == 0)
                	continue;
        	}
        	else
        	{
            	pMac = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr;
            	if( MAC_ADDR_IS_ZERO(pMac) || MAC_ADDR_IS_BROADCAST(pMac) )
                	continue;
        	}
#if 0                            
            if( MAC_ADDR_IS_ZERO(pMac) || MAC_ADDR_IS_BROADCAST(pMac) )
            continue;
#endif
            if(onuType != ALL_ONU)
            {
                 type = GetOnuVendorType(PonPortIdx, OnuIdx);
                 if( (type == VOS_ERROR) || (type != onuType) )
                    continue;
            }
            slot = GetCardIdxByPonChip(PonPortIdx);
            if( slot == RERROR )
                 continue;
            port = GetPonPortByPonChip(PonPortIdx);
            if( port == RERROR )
                 continue;

            ONU_MGMT_SEM_GIVE;
            return MAKEDEVID(slot, port, OnuIdx+1);
        }
    }
    ONU_MGMT_SEM_GIVE;
    return 0;
}
#else
ULONG findFirstOnu( ULONG onuType )
{
	int slot, port, onuid;
	int type;
	short int PonPortIdx, OnuIdx;

	for( slot=PONCARD_FIRST; slot<=PONCARD_LAST; slot++ )
	{
		/*if( SlotCardIsPonBoard(slot) != ROK )
			continue;*/
		
		for( port=1; port<=MAX_PONPORT_PER_BOARD; port++ )
		{
			PonPortIdx = GetPonPortIdxBySlot( slot, port );
			if( PonPortIdx == RERROR )
				continue;
			/*if( !PonPortIsWorking(PonPortIdx) )
				continue;*/

			for( onuid=1; onuid<=MAX_ONU_PER_PONPORT; onuid++ )
			{
				OnuIdx = onuid - 1;
				type = 0;
				/*if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
					continue;*/
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
					continue;

				if(onuType != ALL_ONU)
				{
					type = GetOnuVendorType(PonPortIdx, OnuIdx);
					if( (type == VOS_ERROR) || (type != onuType) )
						continue;
				}
				
		    		return MAKEDEVID(slot, port, onuid);
			}
		}
	}
	return 0;
}
#endif

#if 1
ULONG findNextOnu( ULONG brdIdx, ULONG portIdx, ULONG onuId, ULONG onuType )
{
	int type;
	ULONG slot, port, onu;
	short int PonPortIdx, OnuIdx;
    int entrybase;
    UCHAR *pMac;
    
	if( brdIdx < PONCARD_FIRST)
		return findFirstOnu( onuType );
    
	slot = brdIdx;	
	port = portIdx;
	OnuIdx = onuId;
    PonPortIdx = GetPonPortIdxBySlot( slot, port );
    
    ONU_MGMT_SEM_TAKE;    
    for(; PonPortIdx<MAXPON; PonPortIdx++ )
    {
    	slot = GetCardIdxByPonChip(PonPortIdx);
        entrybase = PonPortIdx*MAXONUPERPON;
        for(; OnuIdx<MAX_ONU_PER_PONPORT; OnuIdx++ )
        {
            type = 0;
            if(SYS_MODULE_IS_GPON(slot))
        	{
            	if(VOS_StrLen(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.DeviceSerial_No) == 0)
                	continue;
        	}
        	else
        	{
            	pMac = OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].DeviceInfo.MacAddr;
            	if( MAC_ADDR_IS_ZERO(pMac) || MAC_ADDR_IS_BROADCAST(pMac) )
                	continue;
        	}
            

            if(onuType != ALL_ONU)
            {
                type = GetOnuVendorType(PonPortIdx, OnuIdx);
                if( (type == VOS_ERROR) || (type != onuType) )
                    continue;
            }
            slot = GetCardIdxByPonChip(PonPortIdx);
            if( slot == RERROR )
                continue;
            port = GetPonPortByPonChip(PonPortIdx);
            if( port == RERROR )
                continue;

            ONU_MGMT_SEM_GIVE;
            return MAKEDEVID(slot, port, OnuIdx+1);
        }
        OnuIdx = 0;
    }
    ONU_MGMT_SEM_GIVE;
    return 0;
}
#else
ULONG findNextOnu( ULONG brdIdx, ULONG portIdx, ULONG onuId, ULONG onuType )
{
	int type;
	ULONG slot, port, onu;
	short int PonPortIdx, OnuIdx;

	if( brdIdx < PONCARD_FIRST)
		return findFirstOnu( onuType );

	slot = brdIdx;	
	port = portIdx;
	onu = onuId + 1;
	for( ; slot<=PONCARD_LAST; slot++ )
	{
		/*if( SlotCardIsPonBoard(slot) != ROK )
			continue;*/

		for( ; port<=MAX_PONPORT_PER_BOARD; port++ )
		{
			PonPortIdx = GetPonPortIdxBySlot( slot, port );
			if( PonPortIdx == RERROR )
				continue;
			/*if( !PonPortIsWorking(PonPortIdx) )
				continue;*/
		
			for( ; onu<=MAX_ONU_PER_PONPORT; onu++ )
			{
				OnuIdx = onu - 1;
				type = 0;
				/*if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
					continue;*/
				if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
					continue;
			
				if(onuType != ALL_ONU)
				{
					type = GetOnuVendorType(PonPortIdx, OnuIdx);
					if( (type == VOS_ERROR) || (type != onuType) )
						continue;
				}
				return MAKEDEVID(slot, port, onu);
			}
			onu = 1;
		}
		onu = 1;
		port = 1;
	}
	return 0;
}
#endif
LONG getFirstOnu( ULONG *pBrdIdx, ULONG *pPortIdx, ULONG *pOnuId )
{
	ULONG devIdx;

	if( (pBrdIdx == NULL) || (pPortIdx == NULL) || (pOnuId == NULL) )
		return VOS_ERROR;

	devIdx = findFirstOnu( ALL_ONU );
	if( devIdx == 0 )
		return VOS_ERROR;

	*pBrdIdx = GET_PONSLOT(devIdx);
	*pPortIdx = GET_PONPORT(devIdx);
	*pOnuId = GET_ONUID(devIdx);

	return VOS_OK;
}
LONG getNextOnu( ULONG brdIdx, ULONG portIdx, ULONG onuId, ULONG *pNextBrdIdx, ULONG *pNextPortIdx, ULONG *pNextOnuId )
{
	ULONG devIdx;
	
	if( (pNextBrdIdx == NULL) || (pNextPortIdx == NULL) || (pNextOnuId == NULL) )
		return VOS_ERROR;

	if( brdIdx < PONCARD_FIRST)
	{
		devIdx = findFirstOnu( ALL_ONU );
	}
	else
	{
		devIdx = findNextOnu( brdIdx, portIdx, onuId, ALL_ONU );
	}
	if( devIdx == 0 )
		return VOS_ERROR;
	if( MAKEDEVID(brdIdx, portIdx, onuId) == devIdx )
		return VOS_ERROR;
	
	*pNextBrdIdx = GET_PONSLOT(devIdx);
	*pNextPortIdx = GET_PONPORT(devIdx);
	*pNextOnuId = GET_ONUID(devIdx);

	return VOS_OK;
}

#endif

#ifdef __cplusplus

}
#endif

