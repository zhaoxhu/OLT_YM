#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includeFromPas.h"
#include "CT_RMan_Main.h"

#include "onu/onuconfmgt.h"

int CTC_GetChipSetID( short int PonPortIdx, short int OnuIdx );
int CTC_GetOnuCapability(short int PonPortIdx, short int OnuIdx );
int CTC_GetOnuSerialNumber(short int PonPortIdx, short int OnuIdx );
int CTC_GetFirmwareVersion(short int PonPortIdx, short int OnuIdx );
int CTC_getDeviceCapEthPortNum( short int PonPortIdx, short int OnuIdx,  int *value );
int CTC_ChipsetVendorId_Translate( unsigned char *pVendorId, char *pVendorStr, unsigned long *pLen );
int CTC_ChipsetMode_Translate( unsigned long mode, unsigned long *pMapMode );
int CTC_ChipsetRevision_Translate( unsigned char revision, char *pRevisionStr, unsigned long *pLen );
int CTC_onu_cli_cmd_install();
extern UCHAR OnuEvent_Get_OnuAbility(short int PonPortIdx, short int OnuIdx);
extern int OnuEvent_Set_OnuAbility(short int PonPortIdx, short int OnuIdx, UCHAR OnuAbility);

extern STATUS	getOnuAuthEnable(ULONG slot, ULONG port, ULONG *enable );
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);

int CTC_GetOnuDeviceInfoByGwdOam(short int olt_id, short int onu_id)
{
    /*added by wangxiaoyu 2011-10-20
     * get device name for cortina onu*/
    int ret = ROK;
    ULONG onu_model = 0;
    char onu_vendor[80] = "";
    int OnuEntry = olt_id*MAXONUPERPON+onu_id;

    ONU_MGMT_SEM_TAKE
    onu_model = OnuMgmtTable[OnuEntry].onu_model;
    VOS_MemCpy(onu_vendor, OnuMgmtTable[OnuEntry].device_vendor_id, sizeof(OnuMgmtTable[OnuEntry].device_vendor_id));
    ONU_MGMT_SEM_GIVE

    if( onu_model == CTC_GT811_C_MODEL && (!VOS_StriCmp(onu_vendor, CTC_ONU_VENDORID)))
    {
        EQU_SendMsgToOnu_ASYNC(olt_id, onu_id, GET_ONU_SYS_INFO_REQ,0,0);
        /*ret = GetOnuEUQInfo( olt_id, onu_id );*/
    }
    return ret;
}
int CTC_ONU_IS_811C(short int olt_id, short int onu_id)
{
    int ret = ROK;
    ULONG onu_model = 0;
    char onu_vendor[80] = "";
    int OnuEntry = olt_id*MAXONUPERPON+onu_id;

    ONU_MGMT_SEM_TAKE
    onu_model = OnuMgmtTable[OnuEntry].onu_model;
    VOS_MemCpy(onu_vendor, OnuMgmtTable[OnuEntry].device_vendor_id, sizeof(OnuMgmtTable[OnuEntry].device_vendor_id));
    ONU_MGMT_SEM_GIVE

    if( onu_model == CTC_GT811_C_MODEL && (!VOS_StriCmp(onu_vendor, CTC_ONU_VENDORID)))
        ret = 1;

    return ret;
}
/**************************************************************************************************
 * 函    数：CTC_GetOnuDeviceInfo
 * 说    明：获取ONU信息，并将结果保存在ONU结构数组中。本函数在ONU注册成功后被调用。
 * 入口参数：PonPortIdx - PON 索引号，0 - 19
 *           OnuIdx     - ONU 索引号, 0 - 63
 * 出口参数: 无
 * 返 回 值：ROK/RERROR
 **************************************************************************************************/
int CTC_GetOnuDeviceInfo( short int PonPortIdx, short int OnuIdx )
{
	int ret, val = 0;
	unsigned char onu_ver = 0;

	ret = OnuMgt_GetCtcVersion(PonPortIdx, OnuIdx, &onu_ver );
	if( ret == 0 )
	{
		int onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
	        OnuMgmtTable[onuEntry].onu_ctc_version = onu_ver ;		
		if(onu_ver<0x20)      
        {      
			return VOS_ERROR;
        }
	}
	else
    {
		return VOS_ERROR;
    }
    
	CTC_GetChipSetID(PonPortIdx, OnuIdx);
	CTC_GetFirmwareVersion(PonPortIdx, OnuIdx);

	ret = CTC_GetOnuCapability( PonPortIdx, OnuIdx);
	CTC_getDeviceCapEthPortNum( PonPortIdx, OnuIdx,  &val );
	if( (ret == RERROR) || (val == 0) )
	{
		ret = CTC_GetOnuCapability( PonPortIdx, OnuIdx );
	}
	
/*	llid = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( llid != -1 ) 
	{
		CTC_STACK_set_phy_admin_control( PonPortIdx, llid, 0xff, ENABLE );
	}*/
	/* modified by xieshl 20080804, 在ONU注册时（有时）SN读错误，放在最后读则正常，不知何故 */
	CTC_GetOnuSerialNumber(PonPortIdx, OnuIdx);

    /*moved by luh 2012-11-21, 私有设备信息获取统一处理，此处不再处理*/
	/*CTC_GetOnuDeviceInfoByGwdOam(PonPortIdx, OnuIdx);*/

	if( ret != VOS_OK )
		sys_console_printf("CTC: get onu %d/%d/%d device info. error\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1);

	return ret;
}

int CTC_GetChipSetID( short int PonPortIdx, short int OnuIdx )
{
	short int olt_id;
	short int onu_id;
	int ret;
	CTC_STACK_chipset_id_t	chipset_id;
	int OnuEntryIdx;
#if 1
    ret = OnuMgt_GetChipsetID(PonPortIdx, OnuIdx, &chipset_id);
#else
	int cur_onustatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	/*CHECK_ONU_RANGE*/
	if(cur_onustatus  != 1 && cur_onustatus != 4)
		return RERROR;


	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
	olt_id = PonPortIdx;

	ret = CTC_STACK_get_chipset_id( olt_id, onu_id, &chipset_id );
#endif	
	if( ret == 0 )
	{
	    OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;
		VOS_MemCpy( ( void *)OnuMgmtTable[OnuEntryIdx].chip_vendor_id, ( void *)chipset_id.vendor_id, CTC_VENDOR_ID_LENGTH );
		OnuMgmtTable[OnuEntryIdx].revision = chipset_id.revision;
		OnuMgmtTable[OnuEntryIdx].chip_model = chipset_id.chip_model;
		VOS_MemCpy( ( void *)OnuMgmtTable[OnuEntryIdx].date, ( void *)chipset_id.date, 3 );
	}
	else
	{
		if( EVENT_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("Error: CTC_GetChipSetID\r\n");
		}
		return RERROR;
	}
	if( EVENT_REGISTER == V2R1_ENABLE )
	{
		sys_console_printf(" chipset vendor id=0x%02x%02x\r\n", chipset_id.vendor_id[0], chipset_id.vendor_id[1] );	/* added by xieshl 20120307, sysfile中要用到该字段，提供显示手段*/
		sys_console_printf(" chipset revision=0x%04x\r\n", chipset_id.chip_model );
		sys_console_printf(" chipset model=0x%02x\r\n", chipset_id.revision );
	}
	return ROK;
}

int CTC_GetFirmwareVersion(short int PonPortIdx, short int OnuIdx )
{
	short int olt_id;
	short int onu_id;
	int ret;
	int OnuEntryIdx;
	unsigned short ver;
    char value[128] = {0};
    ULONG len = 0;
#if 1
    ret = OnuMgt_GetFirmwareVersion(PonPortIdx, OnuIdx, &ver);
#else
	int cur_onustatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	/*CHECK_ONU_RANGE*/
	if(cur_onustatus  != 1 && cur_onustatus != 4)
		return RERROR;
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
	olt_id = PonPortIdx;

	ret = CTC_STACK_get_firmware_version( olt_id, onu_id, &ver );
#endif	
	if( ret != 0 )
	{
		if( EVENT_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("Error: CTC_GetFirmwareVersion\r\n");
		}
		return RERROR;
	}
    /*added by luh 2012-11-1*/
#if 1   
    OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;
	VOS_Sprintf( value, "V%X.%02X", ((ver >> 8) & 0xff), (ver & 0xff));
    len = VOS_StrLen( value );
	OnuMgmtTable[OnuEntryIdx].DeviceInfo.FwVersionLen = len;
	if( len != 0) 
	{
		VOS_MemCpy(OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].DeviceInfo.FwVersion, value, len);
	}
#endif    
	OnuMgmtTable[OnuEntryIdx].firmware_version = ver; 

	return ROK;
}

static int CTC_convertSNString(char *dstString, const int dstlen, char *srcString, const int srclen)
{
	int len = 0;

	if(dstString && srcString && dstlen > 0 && srclen > 0)
	{
		int i = 0;
		int reallen = 0;
		for(i=0; i<srclen; i++)
		{
			if(srcString[i])
				break;
		}

		len = (srclen-i)>dstlen?dstlen:(srclen-i);

		VOS_MemSet(dstString, 0, dstlen);
		VOS_MemCpy(dstString, srcString+i, len);

	}

	return len;
}

int CTC_GetOnuSerialNumber(short int PonPortIdx, short int OnuIdx )
{
	short int olt_id;
	short int onu_id;
	int ret;
	CTC_STACK_onu_serial_number_t	onu_serial_number = {0};
	int OnuEntryIdx;

  	OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;

#if 1
    ret = OnuMgt_GetSerialNumber(PonPortIdx, OnuIdx, &onu_serial_number);
#else
	int onu_status = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	/*CHECK_ONU_RANGE*/
	if( onu_status != 1 && onu_status != 4)
		return RERROR;
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
	olt_id = PonPortIdx;

	ret = CTC_STACK_get_onu_serial_number( olt_id, onu_id, &onu_serial_number );
#endif	
	if( ret != 0 )
	{
		ret = OnuMgt_GetSerialNumber(PonPortIdx, OnuIdx, &onu_serial_number);
		if( ret != 0 )
		{
			OnuMgmtTable[OnuEntryIdx].onu_model = 0;
	    
			if( EVENT_DEBUG == V2R1_ENABLE)
			{
				sys_console_printf("Error: CTC_GetOnuSerialNumber\r\n");
			}
	        
			return RERROR;
		}
	}

	OnuMgmtTable[OnuEntryIdx].onu_model = onu_serial_number.onu_model;
	VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].device_vendor_id, onu_serial_number.vendor_id, 4 );
	if(OnuMgmtTable[OnuEntryIdx].onu_ctc_version == 0x30)
		{
		onu_auth_loid_cpy(OnuMgmtTable[OnuEntryIdx].extendedModel, onu_serial_number.extendedModel, 16);
		}
	if( EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" device extendModel=%s\r\n %s\r\n", OnuMgmtTable[OnuEntryIdx].extendedModel);
	if( EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" device vendor id=%c%c%c%c\r\n", OnuMgmtTable[OnuEntryIdx].device_vendor_id[0],
		OnuMgmtTable[OnuEntryIdx].device_vendor_id[1],OnuMgmtTable[OnuEntryIdx].device_vendor_id[2],OnuMgmtTable[OnuEntryIdx].device_vendor_id[3]);	/* added by xieshl 20120307, 便于支持互通ONU的sysfile扩展，提供显示手段*/
    
#if 0
	VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].hardware_version, onu_serial_number.hardware_version, 8 );
	VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].software_version, onu_serial_number.software_version, 16 );
#else
	CTC_convertSNString(OnuMgmtTable[OnuEntryIdx].hardware_version, HW_VERSION_NUM_OF_BYTES+1, onu_serial_number.hardware_version, HW_VERSION_NUM_OF_BYTES);
	CTC_convertSNString(OnuMgmtTable[OnuEntryIdx].software_version, SW_VERSION_NUM_OF_BYTES+1, onu_serial_number.software_version, SW_VERSION_NUM_OF_BYTES);
#endif

	/*for sync onu version to show onu-version cmd*/
#if 0
	VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].DeviceInfo.HwVersion, onu_serial_number.hardware_version, 8);
	VOS_MemCpy( OnuMgmtTable[OnuEntryIdx].DeviceInfo.SwVersion, onu_serial_number.software_version, 16);
#else
	OnuMgmtTable[OnuEntryIdx].DeviceInfo.HwVersionLen = CTC_convertSNString(OnuMgmtTable[OnuEntryIdx].DeviceInfo.HwVersion, MAXHWVERSIONLEN, onu_serial_number.hardware_version, HW_VERSION_NUM_OF_BYTES);
	OnuMgmtTable[OnuEntryIdx].DeviceInfo.SwVersionLen = CTC_convertSNString(OnuMgmtTable[OnuEntryIdx].DeviceInfo.SwVersion, MAXSWVERSIONLEN, onu_serial_number.software_version, SW_VERSION_NUM_OF_BYTES);
#endif
	
	if( EVENT_REGISTER == V2R1_ENABLE )
		sys_console_printf(" onu-model=0x%08x\r\n", onu_serial_number.onu_model );	/* added by xieshl 20120307, 便于支持互通ONU的sysfile扩展，提供显示手段*/

	return ROK;
}
/**************************************************************************************************
 * 函    数：CTC_GetOnuCapability
 * 说    明：获取ONU能力参数，并将结果保存在ONU结构数组中
 * 入口参数：PonPortIdx - PON 索引号，0 - 19
 *           OnuIdx     - ONU 索引号, 0 - 63
 * 出口参数: 无
 * 返 回 值：ROK/RERROR
 **************************************************************************************************/
static int CTC_GetOnuCapability1(short int PonPortIdx, short int OnuIdx );
static int CTC_GetOnuCapability2(short int PonPortIdx, short int OnuIdx );
int CTC_GetOnuCapability(short int PonPortIdx, short int OnuIdx )
{
	int ret = RERROR;
	unsigned char onu_ver = 0;
#if 0
	short int onu_id;
    int onu_status = 0;
	/*CHECK_ONU_RANGE*/
    onu_status = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	if( onu_status != 1 && onu_status != 4)
		return ret;

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );
	
	ret = CTC_STACK_get_onu_version(PonPortIdx, onu_id, &onu_ver );
#else
    ret = OnuMgt_GetCtcVersion(PonPortIdx, OnuIdx, &onu_ver );
#endif
	if( ret == 0 )
	{
	    int onuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;

	    OnuMgmtTable[onuEntry].onu_ctc_version = onu_ver ;
        /*CTC 版本大于2.1的支持端口统计*/
        if(onu_ver == 0x30)
        {
            UCHAR ability = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);
            ability |= 0x80;
            OnuEvent_Set_OnuAbility(PonPortIdx, OnuIdx, ability);
        }
        
		if( onu_ver < CTC_2_1_ONU_VERSION )
			ret = CTC_GetOnuCapability1( PonPortIdx, OnuIdx );
		else
			ret = CTC_GetOnuCapability2( PonPortIdx, OnuIdx );
	}
	return ret;
}

static int CTC_GetOnuCapability1(short int PonPortIdx, short int OnuIdx )
{
	int ret;
	int i;
	CTC_STACK_onu_capabilities_t device_capabilities;
	short int onu_id;
	unsigned long b;
	ONUTable_S *pOnuEntry;

#if 0
	/*CHECK_ONU_RANGE*/
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );

	ret = CTC_STACK_get_onu_capabilities( PonPortIdx, onu_id, &device_capabilities);
#else
    ret = OnuMgt_GetOnuCap1( PonPortIdx, OnuIdx, &device_capabilities );
#endif

	if( ret == 0 )
	{
		/* save these info to OnuMgmtTable[] */
		pOnuEntry = &OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx];
		
		pOnuEntry->GE_supporting   =  (  device_capabilities.service_supported & CTC_SERVICE_GBIT_ETHERNET ) ? SUPPORTING : NOT_SUPPORTING;
		pOnuEntry->FE_supporting   =  (  device_capabilities.service_supported & CTC_SERVICE_FE_ETHERNET ) ? SUPPORTING : NOT_SUPPORTING;
		pOnuEntry->VoIP_supporting =  (  device_capabilities.service_supported & CTC_SERVICE_VOIP ) ? SUPPORTING : NOT_SUPPORTING;
		pOnuEntry->TDM_CES_supporting  =  (  device_capabilities.service_supported & CTC_SERVICE_TDM_CES ) ? SUPPORTING : NOT_SUPPORTING;

    	VOS_MemZero( pOnuEntry->Ports_distribution, sizeof(pOnuEntry->Ports_distribution) );

		b = 1;
		for( i = 0; i < 32; i++ )   /* 保存接口类型，最多共64个接口 */
		{
			if( device_capabilities.ge_ports_bitmap.lsb  & b )                                       
				pOnuEntry->Ports_distribution[i>>2] |= ( GE_INTERFACE << ( (i&3)<<1 ) );
			else if( device_capabilities.fe_ports_bitmap.lsb & b )
				pOnuEntry->Ports_distribution[i>>2] |= ( FE_INTERFACE << ( (i&3)<<1 ) );
			b <<= 1;
		}
		
		pOnuEntry->GE_Ethernet_ports_number = device_capabilities.ge_ethernet_ports_number;
		pOnuEntry->FE_Ethernet_ports_number = device_capabilities.fe_ethernet_ports_number;
		pOnuEntry->POTS_ports_number = device_capabilities.pots_ports_number;
		pOnuEntry->E1_ports_number = device_capabilities.e1_ports_number;

		pOnuEntry->Upstream_queues_number = device_capabilities.upstream_queues_number;
		pOnuEntry->Max_upstream_queues_per_port = device_capabilities.max_upstream_queues_per_port;
/*		pOnuEntry->Upstream_queues_allocation_increment = device_capabilities.Upstream_queues_allocation_increment; */
		pOnuEntry->Downstream_queues_number = device_capabilities.downstream_queues_number;
		pOnuEntry->Max_downstream_queues_per_port = device_capabilities.max_downstream_queues_per_port;
		pOnuEntry->HaveBattery = device_capabilities.battery_backup;
		
		if( EVENT_DEBUG == V2R1_ENABLE)
		{
#if 0
			int ports_sum;
			sys_console_printf("\r\n%s/port%d Onu %d capabilities \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
			sys_console_printf("   GBIT Ethernet service -  %s \r\n", (  device_capabilities.service_supported & CTC_SERVICE_GBIT_ETHERNET ) ? "supporting":"not supporting");
			sys_console_printf("   FE Ethernet service   -  %s \r\n", (  device_capabilities.service_supported & CTC_SERVICE_FE_ETHERNET ) ? "supporting":"not supporting" );
			sys_console_printf("   VoIP service          -  %s \r\n", (  device_capabilities.service_supported & CTC_SERVICE_VOIP ) ? "supporting":"not supporting" );
			sys_console_printf("   TDM CES service       -  %s \r\n", (  device_capabilities.service_supported & CTC_SERVICE_TDM_CES ) ? "supporting":"not supporting" );
		
			sys_console_printf("   GE Ethernet ports number %d \r\n", device_capabilities.ge_ethernet_ports_number );
			sys_console_printf("   FE Ethernet ports number %d \r\n", device_capabilities.fe_ethernet_ports_number );
			sys_console_printf("   POTS ports number %d \r\n", device_capabilities.pots_ports_number );
			sys_console_printf("   E1 ports number %d \r\n", device_capabilities.e1_ports_number );

			ports_sum = device_capabilities.ge_ethernet_ports_number + device_capabilities.fe_ethernet_ports_number;
			b = 1;
			for( i = 0; i < ports_sum ; i++ )
			{
				if( device_capabilities.ge_ports_bitmap.lsb  & b  )
					sys_console_printf("   Port %d - GE interface \r\n", i + 1 );
				else if( device_capabilities.fe_ports_bitmap.lsb  & b )
					sys_console_printf("   Port %d - FE interface \r\n", i + 1 );
				b <<= 1;
			}
			sys_console_printf("   Upstream queues number               %d \r\n",device_capabilities.upstream_queues_number);
			sys_console_printf("   Max upstream queues per port         %d \r\n",device_capabilities.max_upstream_queues_per_port);
/*			sys_console_printf("   Upstream queues allocation increment %d \r\n", device_capabilities.Upstream_queues_allocation_increment); */
			sys_console_printf("   Downstream queues number             %d \r\n", device_capabilities.downstream_queues_number);
			sys_console_printf("   Max Downstream queues per port       %d \r\n", device_capabilities.max_downstream_queues_per_port);
			sys_console_printf("   Battery backup - %s \r\n", device_capabilities.battery_backup ? "supporting":"not supporting" );
#else
			sys_console_printf("onu %d/%d/%d: CTC_STACK_get_onu_capabilities1 OK\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
#endif
		}
	
		return( ROK );
	}
	else
	{
		if( EVENT_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("onu %d/%d/%d: CTC_STACK_get_onu_capabilities1 ERROR\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		}
		
	}

	return( RERROR );
}

static int CTC_GetOnuCapability2(short int PonPortIdx, short int OnuIdx )
{
	int ret;
	int i,j,k;
	unsigned short nPortNum, nPortNum2;
	ONUTable_S *pOnuEntry;
	CTC_STACK_onu_capabilities_2_t  onu_capabilities2;
	/*for onu swap by jinhl@2013-04-27*/
	VOS_MemSet(&onu_capabilities2, 0, sizeof(CTC_STACK_onu_capabilities_2_t));
#if 0
	short int onu_id;
	/*CHECK_ONU_RANGE*/
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );

    /* 注意: 有的ONU得到的接口信息有错误 */
	ret = CTC_STACK_get_onu_capabilities_2( PonPortIdx, onu_id, &onu_capabilities2 );
#else
    /* 注意: 有的ONU得到的接口信息有错误 */
    ret = OnuMgt_GetOnuCap2( PonPortIdx, OnuIdx, &onu_capabilities2 );
#endif

	if( ret != 0 )
	{
		if( EVENT_DEBUG == V2R1_ENABLE)
		{
			sys_console_printf("onu %d/%d/%d: CTC_STACK_get_onu_capabilities2 ERROR\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
		}
		return RERROR;
	}

	pOnuEntry = &OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx];    
	pOnuEntry->GE_supporting = NOT_SUPPORTING;
	pOnuEntry->FE_supporting = NOT_SUPPORTING;
	pOnuEntry->VoIP_supporting = NOT_SUPPORTING;
	pOnuEntry->TDM_CES_supporting = NOT_SUPPORTING;
    
	pOnuEntry->GE_Ethernet_ports_number = 0;
	pOnuEntry->FE_Ethernet_ports_number = 0;
	pOnuEntry->POTS_ports_number = 0;
	pOnuEntry->E1_ports_number = 0;
	pOnuEntry->ADSL_ports_number = 0;
	pOnuEntry->VDSL_ports_number = 0;
	pOnuEntry->WLAN_ports_number = 0;
	pOnuEntry->USB_ports_number = 0;
	pOnuEntry->CATV_ports_number = 0;

	VOS_MemZero( pOnuEntry->Ports_distribution, sizeof(pOnuEntry->Ports_distribution) );
		
	for( i=0; i<CTC_STACK_CAPABILITY_MAX_INTERFACE_TYPE_NUM; i++ )
	{
		switch( onu_capabilities2.interface_type[i].interface_type )
		{
			case CTC_STACK_ONU_MODE_GE_TYPE:
				if( 0 < (nPortNum = onu_capabilities2.interface_type[i].num_of_port) )
				{
				    nPortNum2 = pOnuEntry->FE_Ethernet_ports_number;
				    if ( CTC_ONU_MAX_ETHPORT >= (nPortNum + nPortNum2) )
                    {
    					pOnuEntry->GE_supporting = SUPPORTING;
    					pOnuEntry->GE_Ethernet_ports_number = nPortNum;
    					
    					for( j=0; j<nPortNum; j++ )
    					{
    						k = j + nPortNum2;
    						pOnuEntry->Ports_distribution[k>>2] |= ( GE_INTERFACE << ( (k & 0x03) << 1 ) );
    					}
                    }            
				}
					
				break;
			case CTC_STACK_ONU_MODE_FE_TYPE:
				if( 0 < (nPortNum = onu_capabilities2.interface_type[i].num_of_port) )
				{
				    nPortNum2 = pOnuEntry->GE_Ethernet_ports_number;
				    if ( CTC_ONU_MAX_ETHPORT >= (nPortNum + nPortNum2) )
                    {
    					pOnuEntry->FE_supporting = SUPPORTING;
    					pOnuEntry->FE_Ethernet_ports_number = nPortNum;
    					
    					for( j=0; j<nPortNum; j++ )
    					{
    						k = j + nPortNum2;
    						pOnuEntry->Ports_distribution[k>>2] |= ( FE_INTERFACE << ( (k & 0x03) << 1 ) );
    					}
                    }            
				}

				break;
			case CTC_STACK_ONU_MODE_VOIP_TYPE:
				if( 0 < (nPortNum = onu_capabilities2.interface_type[i].num_of_port) )
				{
    				pOnuEntry->VoIP_supporting = SUPPORTING;
    				pOnuEntry->POTS_ports_number = nPortNum;
                }
                break;
			case CTC_STACK_ONU_MODE_TDM_TYPE:
				if( 0 < (nPortNum = onu_capabilities2.interface_type[i].num_of_port) )
				{
    				pOnuEntry->TDM_CES_supporting = SUPPORTING;
    				pOnuEntry->E1_ports_number = nPortNum;
                }
				break;
			case CTC_STACK_ONU_MODE_ADSL2_TYPE:
				pOnuEntry->ADSL_ports_number = onu_capabilities2.interface_type[i].num_of_port;
				break;
			case CTC_STACK_ONU_MODE_VDSL2_TYPE:
				pOnuEntry->VDSL_ports_number = onu_capabilities2.interface_type[i].num_of_port;
				break;
			case CTC_STACK_ONU_MODE_WLAN_TYPE:
				pOnuEntry->WLAN_ports_number = onu_capabilities2.interface_type[i].num_of_port;
				break;
			case CTC_STACK_ONU_MODE_USB_TYPE:
				pOnuEntry->USB_ports_number = onu_capabilities2.interface_type[i].num_of_port;
				break;
			case CTC_STACK_ONU_MODE_CATV_RF_TYPE:
				pOnuEntry->CATV_ports_number = onu_capabilities2.interface_type[i].num_of_port;
				break;
			default:
				break;
		}
	}
	pOnuEntry->HaveBattery = onu_capabilities2.battery_backup;
	pOnuEntry->ProtectType = onu_capabilities2.protection_type;

	/*pOnuEntry->Upstream_queues_number = 4;
	pOnuEntry->Max_upstream_queues_per_port = 4;
	pOnuEntry->Downstream_queues_number = 4;
	pOnuEntry->Max_downstream_queues_per_port = 4;*/
		
	if( EVENT_DEBUG == V2R1_ENABLE)
	{
		sys_console_printf("onu %d/%d/%d: CTC_STACK_get_onu_capabilities2 OK\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
	}
	
	return( ROK );
}

int Downstream_queues_number_Get( short int PonPortIdx, short int OnuIdx, int *value )
{
	short int onu_id;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return RERROR ;	

	*value = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Downstream_queues_number;

	return ROK;
}

int MaxDownStreamQueuesPerPort_Get( short int PonPortIdx, short int OnuIdx, int *value )
{
	short int onu_id;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return RERROR ;	

	*value = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Max_downstream_queues_per_port;

	return ROK;
}

int CTC_StartOnuEncryption( short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return RERROR ;	
    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	OnuMgt_StartEncrypt( PonPortIdx, OnuIdx );
	#else
	CTC_STACK_start_encryption( PonPortIdx, onu_id );
	#endif

	return ROK;
}

int CTC_StopOnuEncrypt( short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return RERROR ;	

	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	OnuMgt_StopEncrypt( PonPortIdx, OnuIdx );
	#else
	CTC_STACK_stop_encryption( PonPortIdx, onu_id );
	#endif

	return ROK;
}

int CTC_EncryptionTiming_Get( unsigned char *update_key_time, unsigned short int *no_reply_time_out )
{
	static unsigned char upKeyTime = 10;
	static unsigned short int noReplyTime = 30;
	
	*update_key_time = upKeyTime;
	*no_reply_time_out = noReplyTime;

	return ROK;
} 

/* 设置和读取ONU认证模式 */
int SetOnuAuthenticationMode( short int PonPortIdx, unsigned char mode )
{
	/* short int PonPortType; */

	CHECK_PON_RANGE

#if 1
    if ( OLT_CALL_ISERROR( OLT_SetOnuAuthMode(PonPortIdx, (int)mode) ) )
    {
    	return( RERROR );
    }
#else
	PonPortType = V2R1_GetPonchipType( PonPortIdx );

	if(( PonPortType == PONCHIP_PAS5201 ) && ( V2R1_CTC_STACK == TRUE))
		{
		if( PAS_set_authorize_mac_address_according_list_mode( PonPortIdx, mode ) == PAS_EXIT_OK ) return ROK;
		else return RERROR;
		}
	
	else { /* other pon chip type handler */
		

		}
#endif

	return( ROK );
}

int GetOnuAuthenticationMode( short int PonPortIdx, unsigned char *mode )
{
	short int PonChipType;
	unsigned long enable;
    ULONG slot, port;
	CHECK_PON_RANGE

	if( mode == NULL ) return( RERROR );

	*mode = 1;

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if((PonChipType != PONCHIP_PAS5001) && ( V2R1_CTC_STACK == TRUE))
	{
#if 1	
	    *mode = PonPortTable[PonPortIdx].OnuAuthMode;
#else
		if( PAS_get_authorize_mac_address_according_list_mode( PonPortIdx, mode ) == PAS_EXIT_OK ) return ROK;
		else return RERROR;
#endif
	}
	else if( (PonChipType == PONCHIP_PAS5001) || (V2R1_CTC_STACK != TRUE))
	{
        slot = GetCardIdxByPonChip( PonPortIdx );
        port = GetPonPortByPonChip( PonPortIdx );
		getOnuAuthEnable(slot, port, &enable );
        enable &= 0xff;
		if(enable == V2R1_ONU_AUTHENTICATION_ENABLE) 
            *mode = TRUE;
		else  *mode = FALSE;
	}
	else
    { /* other pon chip type handler */
	}

    return( ROK );
}

int AddMacAuthentication( short int PonPortIdx,  mac_address_t mac )
{
	/*short int PonChipType;
	short int ret;*/

	CHECK_PON_RANGE;

#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX_R(PonPortIdx) //PonPortIdx may excced 255

    OLT_SetMacAuth( PonPortIdx, ENABLE, mac );
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if((PonChipType == PONCHIP_PAS5201 ) && ( V2R1_CTC_STACK == TRUE) )
	{
		ret = PAS_set_mac_address_authentication( PonPortIdx, ENABLE, 1, (mac_address_t *)mac );
		if((  ret == PAS_EXIT_OK ) || ( ret == ENTRY_ALREADY_EXISTS ))return ROK;
		else return RERROR;
	}
	else
    { /* other pon chip type handler */
	}
#endif
    
	return( ROK );
}


int DeleteMacAuthentication( short int PonPortIdx, mac_address_t mac )
{
	/*short int PonChipType;
	short int ret;*/

	CHECK_PON_RANGE;

#if 1
	SWAP_TO_ACTIVE_PON_PORT_INDEX_R(PonPortIdx) //PonPortIdx may excced 255

    OLT_SetMacAuth( PonPortIdx, DISABLE, mac );
#else
	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if((PonChipType == PONCHIP_PAS5201 ) && ( V2R1_CTC_STACK == TRUE) )
		{
		ret = PAS_set_mac_address_authentication( PonPortIdx, DISABLE, 1, (mac_address_t *)mac );
		if((ret == PAS_EXIT_OK ) || ( ret == PAS_QUERY_FAILED )) return ROK;
		else return RERROR; 
		}
	else{

		}
#endif

	return( ROK );
}

int GetMacAuthentication( short int PonPortIdx, unsigned char *numberOfMac, mac_addresses_list_t mac_address_list )
{
	short int PonChipType;

	CHECK_PON_RANGE

	PonChipType = V2R1_GetPonchipType( PonPortIdx );
	if((PonChipType != PONCHIP_PAS5001 ) && ( V2R1_CTC_STACK == TRUE))
		{	
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if( OLT_GetMACAddressAuthentication( PonPortIdx, numberOfMac, mac_address_list ) == PAS_EXIT_OK ) return ROK;
		else return RERROR;
		}
	else{

		}
	return( ROK );
}

#if 0 /* for test */
void TestAuthentication( void )
{
	unsigned char mac[] = {00,0x0c,0xd5,0x62,0x15,0x10 };
	AddMacAuthentication( 8, mac );
}
void TestDelete( void )
{
	mac_address_t mac[] = {00,0x0c,0xd5,0x62,0x15,0x10 };
	DeleteMacAuthentication( 8, mac );
}

void ListAuthenticationMac( void )
{
	mac_addresses_list_t mac_list;
	unsigned char numberOfMac, i;

	GetMacAuthentication( 8, &numberOfMac, mac_list );

	sys_console_printf("number of mac %d \r\n", numberOfMac );

	for( i = 0; i < numberOfMac; i++ )
	{
		sys_console_printf("%02X-%02X-%02X-%02X-%02X-%02X\r\n",mac_list[i][0],mac_list[i][1],mac_list[i][2],
		mac_list[i][3],mac_list[i][4],mac_list[i][5] );
	}
}


int CTC_DBA_ThresholdSet( short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;
	short int ret;
	unsigned char number_of_queue_sets;
	CTC_STACK_onu_queue_set_thresholds_t queues_sets_thresholds;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	return ROK;

}

int CTC_DBA_ThresholdGet( short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;
	short int ret;
	unsigned char number_of_queue_sets;
	CTC_STACK_onu_queue_set_thresholds_t queues_sets_thresholds;

	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	ret = CTC_STACK_get_dba_report_thresholds( PonPortIdx, onu_id, &number_of_queue_sets, &queues_sets_thresholds );

	if( ret == CTC_STACK_EXIT_OK )
	{
		
	}
	return ROK;
}

int CTC_API_TEST( short int PonPortIdx, short int OnuIdx, unsigned char port_number )
{
	short int olt_id,onu_id, ret;
	CTC_STACK_port_vlan_configuration_t port_configuration;
	CTC_STACK_vlan_configuration_ports_t vlan_info;
	CTC_STACK_multicast_ports_tag_strip_t ports_strip;

	bool bValue;
	unsigned char number_of_entries;

	CTC_STACK_multicast_control_t  multicast_control;
	CTC_STACK_multicast_protocol_t multicast_protocol;
	CTC_STACK_multicast_vlan_t	   multicast_vlan_ports;

	bool automatic_mode;
	unsigned char number_of_records;
	CTC_STACK_oui_version_record_t records_list[64];
	bool automatic_onu_configuration;

	CTC_STACK_link_state_t link_state;
	CTC_STACK_ethernet_port_policing_entry_t port_policing;

	CTC_STACK_ethernet_ports_state_t ethernet_ports_state;
	CTC_STACK_auto_negotiation_technology_ability_t	abilities;
	CTC_STACK_standard_FEC_ability_t			fec_ability;
	CTC_STACK_auto_negotiation_ports_technology_ability_t  ports_abilities;
	CTC_STACK_standard_FEC_mode_t			fec_mode;
	CTC_STACK_onu_queue_set_thresholds_t    queues_thresholds[4];

	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
	olt_id = PonPortIdx;

	/* Get/Set parameters */

	ret = CTC_STACK_get_parameters(&automatic_mode, &number_of_records, records_list, &automatic_onu_configuration ); 

	/* Ethernet link state */

	ret = CTC_STACK_get_ethernet_link_state( PonPortIdx, onu_id, port_number, &link_state );

	/* Ethernet port policing */
	
	ret = CTC_STACK_get_ethernet_port_policing( PonPortIdx, onu_id, port_number, &port_policing );

	if( port_policing.operation == CTC_STACK_STATE_ACTIVATE ) port_policing.operation = CTC_STACK_STATE_DEACTIVATE;
	else port_policing.operation = CTC_STACK_STATE_ACTIVATE;;
	ret = CTC_STACK_set_ethernet_port_policing( PonPortIdx, onu_id, port_number, port_policing );
	port_policing.operation = 0xFF;
	ret = CTC_STACK_get_ethernet_port_policing( PonPortIdx, onu_id, port_number, &port_policing );

	/* vlan port configuration */

	ret = CTC_STACK_get_vlan_port_configuration( olt_id, onu_id, port_number, &port_configuration );
	ret = CTC_STACK_get_vlan_all_port_configuration( olt_id, onu_id, &port_number, vlan_info );

	/* multicast vlan */

	ret = CTC_STACK_get_multicast_vlan( olt_id, onu_id, port_number, &multicast_vlan_ports );
	multicast_vlan_ports.num_of_vlan_id = 1;
	multicast_vlan_ports.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
	multicast_vlan_ports.vlan_id[0] = 2;
	ret = CTC_STACK_set_multicast_vlan( olt_id, onu_id, port_number, multicast_vlan_ports );
	multicast_vlan_ports.num_of_vlan_id = 0;
	multicast_vlan_ports.vlan_operation = 0;
	multicast_vlan_ports.vlan_id[0] = 0;
	ret = CTC_STACK_get_multicast_vlan( olt_id, onu_id, port_number, &multicast_vlan_ports );

	/* multicast */
	ret = CTC_STACK_get_multicast_tag_strip( olt_id, onu_id, port_number, &bValue );
	ret = CTC_STACK_get_multicast_all_port_tag_strip(olt_id, onu_id, &number_of_entries, ports_strip );
	
	/* Ethernet port pause */
	
	CTC_STACK_get_ethernet_port_pause( olt_id, onu_id, port_number, &bValue );
	CTC_STACK_set_ethernet_port_pause( olt_id, onu_id, port_number, !bValue );
	CTC_STACK_get_ethernet_port_pause( olt_id, onu_id, port_number, &bValue );

	/* multicast switch protocol */

	multicast_protocol = 0xff;
	ret = CTC_STACK_get_multicast_switch( olt_id, onu_id, &multicast_protocol );
	if( multicast_protocol == CTC_STACK_PROTOCOL_IGMP_SNOOPING ) multicast_protocol = CTC_STACK_PROTOCOL_CTC;
	else multicast_protocol = CTC_STACK_PROTOCOL_IGMP_SNOOPING;
	ret = CTC_STACK_set_multicast_switch( olt_id, onu_id, multicast_protocol );
	multicast_protocol = 0xff;
	ret = CTC_STACK_get_multicast_switch( olt_id, onu_id, &multicast_protocol );


	/* multicast control ，必须先将所用协议设为 CTC_STACK_PROTOCOL_CTC，否则后面的操作会失败 */

	multicast_protocol = CTC_STACK_PROTOCOL_CTC;	
	ret = CTC_STACK_set_multicast_switch( olt_id, onu_id, multicast_protocol ); 

	ret = CTC_STACK_get_multicast_control( olt_id, onu_id, &multicast_control );
	multicast_control.control_type = CTC_STACK_MULTICAST_CONTROL_TYPE_DA_ONLY;
	multicast_control.action = CTC_MULTICAST_CONTROL_ACTION_ADD;
	multicast_control.num_of_entries = 1;
	ret = CTC_STACK_set_multicast_control( olt_id, onu_id, multicast_control );
	ret = CTC_STACK_get_multicast_control( olt_id, onu_id, &multicast_control );

	/* PHY admin state */
	ret = CTC_STACK_get_phy_admin_state( olt_id, onu_id, port_number, &bValue );
	ret = CTC_STACK_set_phy_admin_control( olt_id, onu_id, port_number, !bValue );
	ret = CTC_STACK_get_phy_admin_state( olt_id, onu_id, port_number, &bValue );
	ret = CTC_STACK_get_all_port_phy_admin_state( olt_id, onu_id, &number_of_entries, ethernet_ports_state );
	ret = CTC_STACK_set_phy_admin_control( olt_id, onu_id, port_number, ENABLE );
	
	/* auto negotiation */
	ret = CTC_STACK_get_auto_negotiation_admin_state( olt_id, onu_id, port_number, &bValue );	
	ret = CTC_STACK_set_auto_negotiation_admin_control( olt_id, onu_id, port_number, !bValue );
	ret = CTC_STACK_get_auto_negotiation_admin_state( olt_id, onu_id, port_number, &bValue );
	ret = CTC_STACK_get_auto_negotiation_all_ports_admin_state( olt_id, onu_id, &number_of_entries, ethernet_ports_state );
	ret = CTC_STACK_get_auto_negotiation_local_technology_ability( olt_id, onu_id, port_number, &abilities );
	ret = CTC_STACK_get_auto_negotiation_all_ports_local_technology_ability( olt_id, onu_id, &number_of_entries, ports_abilities);
	ret = CTC_STACK_get_auto_negotiation_advertised_technology_ability( olt_id, onu_id, port_number, &abilities);
	ret = CTC_STACK_get_auto_negotiation_all_ports_advertised_technology_ability( olt_id, onu_id, &number_of_entries, ports_abilities );

	ret = CTC_STACK_set_auto_negotiation_restart_auto_config( olt_id, onu_id, port_number );

	/* fec ability */
	ret = CTC_STACK_get_fec_ability( olt_id, onu_id, &fec_ability); 
	fec_mode = STD_FEC_MODE_ENABLED;
	ret = CTC_STACK_set_fec_mode( olt_id, onu_id, fec_mode );
	ret = CTC_STACK_get_fec_mode( olt_id, onu_id, &fec_mode );

	/* DBA */

	ret = CTC_STACK_get_dba_report_thresholds( PonPortIdx, onu_id, &number_of_entries, queues_thresholds );
	bValue = queues_thresholds[0].queue[0].state;
	queues_thresholds[0].queue[0].state = !bValue;
	queues_thresholds[0].queue[0].threshold = 456;
	ret = CTC_STACK_set_dba_report_thresholds (olt_id, onu_id, &number_of_entries, queues_thresholds);
/*	queues_thresholds[0].queue[0].state = bValue; */
	queues_thresholds[0].queue[0].threshold = 23;
	ret = CTC_STACK_get_dba_report_thresholds( PonPortIdx, onu_id, &number_of_entries, queues_thresholds );
	
	return ROK;

}
#endif

int CTC_getDeviceChipsetVendor( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
	
	/* modified by xieshl 20080804, 中间做一次转换 */
	/*VOS_MemCpy( ( void *)valbuf, ( void *)OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].chip_vendor_id, CTC_VENDOR_ID_LENGTH );
	*len = CTC_VENDOR_ID_LENGTH;
	return ROK;*/
	return CTC_ChipsetVendorId_Translate( OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].chip_vendor_id, valbuf, len );
}

int CTC_getDeviceChipsetMode( short int PonPortIdx, short int OnuIdx,  unsigned long *value )
{
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	/* modified by xieshl 20080804, 中间做一次转换 */
	return CTC_ChipsetMode_Translate(OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].chip_model, value);
}

/*int CTC_getDeviceChipsetRevision( short int PonPortIdx, short int OnuIdx,  int *value )*/  /* modified by xieshl 20080813 */
int CTC_getDeviceChipsetRevision( short int PonPortIdx, short int OnuIdx,  UCHAR *valbuf, ULONG *len )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	/**value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].revision;
	return ROK;*/
	return CTC_ChipsetRevision_Translate(OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].revision, valbuf, len);
}

int CTC_getDeviceChipsetDate( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	VOS_MemCpy( ( void *)valbuf, ( void *)OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].date, 3 );
	*len = 3;

	return ROK;
}

int CTC_getDeviceCapPortDesc( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len )
{
	short int onu_id;
	short int i,ports_sum;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	VOS_Sprintf( ( void *)valbuf,"GE - %s \r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_supporting ? "supporting":"not supporting");
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"FE - %s \r\n", (  OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_supporting ) ? "supporting":"not supporting" );
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"VoIP - %s \r\n", (  OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].VoIP_supporting ) ? "supporting":"not supporting" );
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"TDM CES - %s \r\n", (  OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].TDM_CES_supporting ) ? "supporting":"not supporting" );
		
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"GE num=%d \r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_Ethernet_ports_number );
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"FE num=%d \r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_Ethernet_ports_number );
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"POTS num=%d \r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].POTS_ports_number );
	VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"E1 num=%d \r\n", OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].E1_ports_number );

	ports_sum = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_Ethernet_ports_number + OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_Ethernet_ports_number;
	for( i = 0; i < ports_sum ; i++ )
	{
		if( ( ( OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Ports_distribution[i/4] >> ( ( i%4 )*2 ) ) & 3 ) == GE_INTERFACE )
			VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"Port %d - GE \r\n", i + 1 );
		else if( ( ( OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Ports_distribution[i/4] >> ( ( i%4 )*2 ) ) & 3 ) == FE_INTERFACE )
			VOS_Sprintf( ( void *)&valbuf[ VOS_StrLen( valbuf ) ],"Port %d - FE \r\n", i + 1 );
	}

	*len = VOS_StrLen( valbuf );

	return ROK;
}

int CTC_getDeviceCapiadCtcOnuType( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].ctc_onu_type;
	if( (*value <= 1) || (*value > 9) ) 
		*value = 2;

	return ROK;
}

int CTC_getDeviceCapiadPotsPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].POTS_ports_number;
	/*if( *value == 0 ) *value = 256;*/
	if( *value > 32 ) *value = 0;

	return ROK;
}

int CTC_getDeviceCapE1PortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].E1_ports_number;
	/*if( *value == 0 ) *value = 256;*/
	if( *value > 32 ) *value = 0;

	return ROK;
}

int CTC_getDeviceCapFePortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_Ethernet_ports_number;
	if( *value > MAX_ONU_ETHPORT )
		*value = 8;	

	return ROK;
}	

int CTC_getDeviceCapGePortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_Ethernet_ports_number;
	if( *value > MAX_ONU_ETHPORT )
		*value = 8;	

	return ROK;
}	

int CTC_getDeviceCapVoidPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].POTS_ports_number;

	return ROK;
}	

int CTC_getDeviceCapTdmPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].E1_ports_number;

	return ROK;
}	

int CTC_getDeviceCapAdslPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].ADSL_ports_number;

	return ROK;
}	

int CTC_getDeviceCapVdslPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].VDSL_ports_number;

	return ROK;
}	

int CTC_getDeviceCapWlanPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].WLAN_ports_number;

	return ROK;
}	

int CTC_getDeviceCapUsbPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].USB_ports_number;

	return ROK;
}	

int CTC_getDeviceCapCatvPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{	short int onu_id;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].CATV_ports_number;

	return ROK;
}	

int CTC_getDeviceCapEthPortNum( short int PonPortIdx, short int OnuIdx,  int *value )
{
    short int onu_id;
	int OnuEntryIdx;
	
	CHECK_ONU_RANGE
    /*moved by luh 2013-1-17*/
#if 0        
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
#endif

    OnuEntryIdx = MAXONUPERPON * PonPortIdx + OnuIdx;
	*value = ( int )OnuMgmtTable[OnuEntryIdx].GE_Ethernet_ports_number + ( int )OnuMgmtTable[OnuEntryIdx].FE_Ethernet_ports_number;
	/*if( *value == 0 ) *value = 256;*/
	/* for 网通测试 ,支持多于24个UNI端口的ONU  2008-7-15*/
	/*if( *value > 24 ) *value = 1;*/
	if( *value > MAX_ONU_ETHPORT )
		*value = 8;	

	return ROK;
}	
int CTC_getDeviceCapUQueueTotal( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Upstream_queues_number;
	/*if( *value == 0 ) *value = 256;*/
	if( *value > 64 ) *value = 0;

	return ROK;
}


int CTC_getDeviceCapUQueuePort( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Max_upstream_queues_per_port;
	/*if( *value == 0 ) *value = 256;*/
	if( *value > 64 ) *value = 0;

	return ROK;
}

#if 0
int CTC_getEthPortMulticastSwitch( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	short int ret;
	CTC_STACK_multicast_protocol_t multicast_protocol;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	ret = CTC_STACK_get_multicast_switch( PonPortIdx, onu_id, &multicast_protocol );
	if( ret != CTC_STACK_EXIT_OK ) return RERROR;

	*value = ( int )multicast_protocol;
	
	return ROK;
}

int CTC_setEthPortMulticastSwitch( short int PonPortIdx, short int OnuIdx,  int value )
{
	short int onu_id;
	short int ret;
	CTC_STACK_multicast_protocol_t multicast_protocol;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	multicast_protocol = value;

	ret = CTC_STACK_set_multicast_switch( PonPortIdx, onu_id, multicast_protocol );
	if( ret != CTC_STACK_EXIT_OK ) return RERROR;
	
	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].multicastSwitch = value;

	return ROK;
}

int CTC_getEthPortMulticastTagStriped( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	short int ret;
	unsigned char number_of_entries;
	CTC_STACK_multicast_ports_tag_strip_t	port_tag_strip;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	ret = CTC_STACK_get_multicast_all_port_tag_strip( PonPortIdx, onu_id, &number_of_entries, port_tag_strip );

	if( ret != CTC_STACK_EXIT_OK ) return RERROR;

	*value = ( int )port_tag_strip[0].tag_strip;

	return ROK;
}

/* 需要设置到所有端口上 */
int CTC_setEthPortMulticastTagStriped( short int PonPortIdx, short int OnuIdx,  int value )
{
	short int onu_id;
	short int ret;
	unsigned char number_of_entries,i;
	CTC_STACK_multicast_ports_tag_strip_t	port_tag_strip;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	ret = CTC_STACK_get_multicast_all_port_tag_strip( PonPortIdx, onu_id, &number_of_entries, port_tag_strip );

	if( ret != CTC_STACK_EXIT_OK ) return RERROR;

	for( i = 0; i < number_of_entries; i++ )
	{
		ret = CTC_STACK_set_multicast_tag_strip( PonPortIdx, onu_id, port_tag_strip[i].port_number, ( bool )value );
		if( ret != CTC_STACK_EXIT_OK ) return RERROR;
	}
	return ROK;
}

#else

int CTC_getEthPortMulticastSwitch( ULONG devIdx,  ULONG *value )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	CTC_STACK_multicast_protocol_t multicast_protocol;

	if( value == NULL )
		return RERROR;

	if( parse_llid_command_parameter(devIdx, &pon_id, &onu_id, &llid) == VOS_ERROR )
		return RERROR;
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_GetMulticastSwitch(pon_id, onu_id, &multicast_protocol) != CTC_STACK_EXIT_OK )
	#else
	if(  CTC_STACK_get_multicast_switch(pon_id, llid, &multicast_protocol) != CTC_STACK_EXIT_OK )
	#endif
		return RERROR;

	if( multicast_protocol == CTC_STACK_PROTOCOL_CTC )
		*value = 2;
	else /*if( multicast_protocol == CTC_STACK_PROTOCOL_IGMP_SNOOPING )*/
		*value = 1;
	
	return ROK;
}

int CTC_setEthPortMulticastSwitch( ULONG devIdx,  ULONG value )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	CTC_STACK_multicast_protocol_t multicast_protocol;

	if( value == 1 )
		multicast_protocol = CTC_STACK_PROTOCOL_IGMP_SNOOPING;
	else if( value == 2 ) 
		multicast_protocol = CTC_STACK_PROTOCOL_CTC;
	else
		return RERROR;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;
	
	if( GetOnuOperStatus(pon_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( pon_id, onu_id );
	if( llid == INVALID_LLID )
		return RERROR;

    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_SetMulticastSwitch(pon_id, onu_id, multicast_protocol) != CTC_STACK_EXIT_OK ) 
    #else
	if( CTC_STACK_set_multicast_switch(pon_id, llid, multicast_protocol) != CTC_STACK_EXIT_OK ) 
    #endif
	    return RERROR;
	
	OnuMgmtTable[MAXONUPERPON * pon_id + onu_id].multicastSwitch = value;

	return ROK;
}


int CTC_getEthPortMulticastTagStriped( ULONG devIdx,  ULONG *value )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	unsigned char number_of_entries;
	CTC_STACK_multicast_ports_tag_strip_t	port_tag_strip;

	if( value == NULL )
		return RERROR;

	if( parse_llid_command_parameter(devIdx, &pon_id, &onu_id, &llid) == VOS_ERROR )
		return RERROR;
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if(  OnuMgt_GetAllPortMulticastTagStrip( pon_id, onu_id, &number_of_entries, port_tag_strip ) != CTC_STACK_EXIT_OK ) 
		return RERROR;

	if( port_tag_strip[0].tag_strip )
		*value = 1;
	else
		*value = 2;

	return ROK;
}

/* 需要设置到所有端口上 */
int CTC_setEthPortMulticastTagStriped( ULONG devIdx,  ULONG value )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	unsigned char	tag_strip;
	unsigned char portno = 0xff;

	if( value == 1 )
		tag_strip = TRUE;
	else if( value == 2 )
		tag_strip = FALSE;
	else
		return RERROR;

	if( parse_llid_command_parameter(devIdx, &pon_id, &onu_id, &llid) == VOS_ERROR )
		return RERROR;
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_SetEthPortMulticastTagStrip(pon_id, onu_id, portno, tag_strip) != CTC_STACK_EXIT_OK )
	#else
	if( CTC_STACK_set_multicast_tag_strip(pon_id, llid, portno, tag_strip) != CTC_STACK_EXIT_OK )
	#endif
		return RERROR;
	
	return ROK;
}

#endif


int CTC_getDeviceCapDQueuePort( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Max_downstream_queues_per_port;
	if( *value == 0 ) *value = 256;

	return ROK;
}
int CTC_getDeviceCapBattery( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;

	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].HaveBattery;
	if( *value != 1 ) 
		*value = 2;

	return ROK;
}
int CTC_getDeviceCapDQueueTotal( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;

	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	*value = ( int )OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].Downstream_queues_number;
	if( *value == 0 ) *value = 256;

	return ROK;
}

int CTC_GetLlidFecCapability( short int PonPortIdx, short int OnuIdx,  int *value )
{
	short int onu_id;
	short int ret;

	
	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	#if 1
	ret = OnuMgt_GetFecAbility( PonPortIdx, OnuIdx, (CTC_STACK_standard_FEC_ability_t *)value );
	#else
	ret = CTC_STACK_get_fec_ability( PonPortIdx, onu_id, (CTC_STACK_standard_FEC_ability_t *)value );
	#endif
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( ret == PAS_EXIT_OK )
		{
		OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FEC_ability = *value;
		return( ROK );
		}
	return (RERROR );
}
int CTC_deviceFirmWareVersion( short int PonPortIdx, short int OnuIdx,  char *value, int *len )
{
	short int onu_id;
	unsigned short ver;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	/* modified by xieshl 20080804, 修改显示格式 */
	ver = OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].firmware_version;
	VOS_Sprintf( value, "V%X.%02X", ((ver >> 8) & 0xff), (ver & 0xff));
	*len = VOS_StrLen( value );

	return ROK;
} 
int CTC_deviceHardWareVersion( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len )
{
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	VOS_StrCpy( ( void *)valbuf, ( void *)OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].hardware_version );
	*len = VOS_StrLen( OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].hardware_version );
	if( *len > 8 ) *len = 8;

	return ROK;
} 
int CTC_deviceSoftWareVersion  ( short int PonPortIdx, short int OnuIdx, UCHAR *valbuf,  ULONG *len )
{
	short int onu_id;

	CHECK_ONU_RANGE
	onu_id = GetLlidByOnuIdx( PonPortIdx, OnuIdx );
	if( onu_id == INVALID_LLID ) return( RERROR );	

	VOS_StrCpy( ( void *)valbuf, ( void *)OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].software_version );
	*len = VOS_StrLen( OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].software_version );

	return ROK;
} 

int CTC_ResetONU(short int PonPortIdx, short int OnuIdx )
{
	short int onu_id;
	short int ret;
	CHECK_ONU_RANGE

	onu_id = GetLlidByOnuIdx( PonPortIdx,  OnuIdx);
	if(onu_id == INVALID_LLID ) return( RERROR );

	if( GetOnuExtOAMStatus(PonPortIdx, OnuIdx ) != TRUE )  return( RERROR );
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_ResetOnu( PonPortIdx, OnuIdx );
	#else
	ret = CTC_STACK_reset_onu( PonPortIdx, onu_id );
	#endif
	if( ret == PAS_EXIT_OK ) return (ROK );
	else return( RERROR );
}

/* added by xieshl 20080807 */

#define CTC_ONU_MODEL_LIST_NUM	128
#define CTC_ONU_MODEL_STR_LEN		24

typedef struct {
	char flag;
	unsigned long type; /*added by wangxiaoyu 2012-06-20 : ctc onu subtype id define*/
	unsigned long model;
	char modelStr[CTC_ONU_MODEL_STR_LEN];
	char modelAppStr[ONU_TYPE_LEN];
} ctcOnuModelList_t;
ctcOnuModelList_t ctcOnuModelList[CTC_ONU_MODEL_LIST_NUM];

#define EQUIPMENT_TYPE_LEN                    40+1
typedef struct {
	char flag;
	unsigned long type; /*added by wangxiaoyu 2012-06-20 : ctc onu subtype id define*/
	char equipment[EQUIPMENT_TYPE_LEN];
	char equipmentStr[CTC_ONU_MODEL_STR_LEN];
	char equipmentAppStr[ONU_TYPE_LEN];
} gponOnuEquipmentList_t;
gponOnuEquipmentList_t gponOnuEquipmentList[CTC_ONU_MODEL_LIST_NUM];


#define CTC_CHIPSET_LIST_NUM	8
	
#define CTC_CHIPSET_VENDORID_LEN			2
#define CTC_CHIPSET_VENDORID_STR_LEN		24
typedef struct {
	char flag;
	char vendorId[CTC_CHIPSET_VENDORID_LEN+1];
	char vendorStr[CTC_CHIPSET_VENDORID_STR_LEN];
} ctcChipsetIdList_t;
ctcChipsetIdList_t ctcChipsetIdList[CTC_CHIPSET_LIST_NUM];

typedef struct {
	char flag;
	unsigned short mode;
	unsigned short mapMode;
} ctcChipsetModeList_t;
ctcChipsetModeList_t ctcChipsetModeList[CTC_CHIPSET_LIST_NUM];

#define CTC_CHIPSET_REVISION_STR_LEN		16
typedef struct {
	char flag;
	unsigned char revision;
	char revisionStr[CTC_CHIPSET_REVISION_STR_LEN];
} ctcChipsetRevisionList_t;
ctcChipsetRevisionList_t ctcChipsetRevisionList[CTC_CHIPSET_LIST_NUM];

int CTC_onu_init()
{
/*	ctcOnuModelList_t *pModel = ctcOnuModelList;
	ctcChipsetIdList_t *pChip = ctcChipsetIdList;
	ctcChipsetModeList_t *pMode = ctcChipsetModeList;*/
	
    /* comment by wangxiaoyu 2011-08-12, because we have got this data from sysfile.
	VOS_MemZero( ctcOnuModelList, sizeof(ctcOnuModelList) );
	VOS_MemZero( ctcChipsetIdList, sizeof(ctcChipsetIdList) );
	VOS_MemZero( ctcChipsetModeList, sizeof(ctcChipsetModeList) );
	VOS_MemZero( ctcChipsetRevisionList, sizeof(ctcChipsetRevisionList) );
	*/

/*	pModel->flag = 1;
	pModel->model = 0x313138;
	VOS_StrCpy(pModel->modelStr, DeviceType_1[V2R1_ONU_GT811]);
	pModel++;
	pModel->flag = 1;
	pModel->model = 0x323138;
	VOS_StrCpy(pModel->modelStr, DeviceType_1[V2R1_ONU_GT812]);

	pChip->flag = 1;
	VOS_StrCpy(pChip->vendorId, "E6" );
	VOS_StrCpy(pChip->vendorStr, "PMC");

	pMode->flag = 1;
	pMode->mode = 0x6301;
	pMode->mapMode = 6301;

	ctcChipsetRevisionList[0].flag = 1;
	ctcChipsetRevisionList[0].revision = 0;
	VOS_StrCpy( ctcChipsetRevisionList[0].revisionStr, "A0" );
*/
	CTC_onu_cli_cmd_install();
	
	return VOS_OK;
}

int searchCtcOnuTypeString(const char *string)
{

    int i, ret = VOS_ERROR;

    for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
    {
        if( 1 == ctcOnuModelList[i].flag )
        {
            if(!VOS_StriCmp(ctcOnuModelList[i].modelStr, string))
            {
                ret = VOS_OK;
                break;
            }
        }
    }

    return ret;
}

int CTC_OnuModel_Translate(unsigned long model, unsigned long *type, char *pModelStr, int *pLen)
{
	int i/*, j*/;
	
	ctcOnuModelList_t *pItem = NULL;
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			if( model == ctcOnuModelList[i].model )
			{
				pItem = &ctcOnuModelList[i];
				break;
			}
		}
	}
	if( pItem )
	{
		*type = pItem->type;
		if( pModelStr )
		{
			VOS_StrCpy( pModelStr, pItem->modelStr );
			*pLen = VOS_StrLen(pModelStr);
		}
	}
	else
	{
			*type = V2R1_ONU_CTC;
			if( pModelStr )
			{
				VOS_StrCpy( pModelStr, GetDeviceTypeString(V2R1_ONU_CTC) );
				*pLen = VOS_StrLen(pModelStr);
			}
	}
	return VOS_OK;
}

/* added by xieshl 20130205, 问题单16958 */
int CTC_OnuModelStr_2_Type( char *pModelStr, unsigned long *pOnuType )
{
	int i, rc = VOS_ERROR;
	ctcOnuModelList_t *pItem = NULL;

	if( (pOnuType == NULL) || (pModelStr == NULL) )
		return rc;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			if( VOS_StriCmp(pModelStr, ctcOnuModelList[i].modelStr) == 0 )
			{
				pItem = &(ctcOnuModelList[i]);
				break;
			}
		}
	}
	if( pItem )
	{
		*pOnuType = pItem->type;
		rc = VOS_OK;
	}
	/*else
	{
		*pOnuType = V2R1_ONU_CTC;
	}*/
	return rc;
}


char * CTC_OnuModelType_2_Str( unsigned long onuType )
{
	int i;
	ctcOnuModelList_t *pItem = NULL;
	char *pOnuTypeStr = NULL;

	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			if( onuType == ctcOnuModelList[i].type )
			{
				pItem = &(ctcOnuModelList[i]);
				break;
			}
		}
	}
	if( pItem )
	{
		pOnuTypeStr = pItem->modelStr;
	}
	return pOnuTypeStr;
}

int CTC_getAppNameWithModel( unsigned long model, char *name, int *pLen )
{
    int i/*, j*/;
    /*char *p;*/

    ctcOnuModelList_t *pItem = NULL;

    if(model == 0)
    {
        VOS_StrCpy(name, "GT_CTC_4FE");
        *pLen = VOS_StrLen(name);
        return VOS_OK;
    }

    for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
    {
        if( 1 == ctcOnuModelList[i].flag )
        {
            if( model == ctcOnuModelList[i].model )
            {
                pItem = &ctcOnuModelList[i];
                break;
            }
        }
    }
    if( pItem )
    {
        VOS_StrCpy( name, pItem->modelAppStr );
        *pLen = VOS_StrLen(name);
    }
    else
    {
        VOS_StrCpy(name, GetOnuEponAppString(V2R1_ONU_CTC));
        *pLen = VOS_StrLen(name);
    }
    return VOS_OK;
}
void CTC_ClearOnuModel()
{
    VOS_MemZero(ctcOnuModelList, sizeof(ctcOnuModelList_t)*CTC_ONU_MODEL_LIST_NUM);    
}
int CTC_addOnuModel( unsigned long model, unsigned long type, char *pModelStr, char * appName )
{
	short int i;
	short int j = CTC_ONU_MODEL_LIST_NUM;
	ctcOnuModelList_t *pItem = NULL;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			if( model == ctcOnuModelList[i].model )
			{
				pItem = &ctcOnuModelList[i];
				break;
			}
		}
		else
		{
			if( j == CTC_ONU_MODEL_LIST_NUM ) 
				j = i;
		}
	}
	if( pItem == NULL )
	{
		if( j >= CTC_ONU_MODEL_LIST_NUM )
			return VOS_ERROR;
		
		pItem = &ctcOnuModelList[j];
	}
	
	pItem->flag = 1;
	pItem->type = type;
	pItem->model = model;
	VOS_MemZero(pItem->modelStr, CTC_ONU_MODEL_STR_LEN);
	VOS_StrnCpy(pItem->modelStr, pModelStr, CTC_ONU_MODEL_STR_LEN-1 );
	if(appName)
	{
		VOS_MemZero(pItem->modelAppStr, ONU_TYPE_LEN);
	   	VOS_StrnCpy(pItem->modelAppStr, appName, ONU_TYPE_LEN-1);
	}
	return VOS_OK;
}
int CTC_delOnuModel( unsigned long model )
{
	short int i;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			if( model == ctcOnuModelList[i].model )
			{
				VOS_MemZero( &ctcOnuModelList[i], sizeof(ctcOnuModelList_t) );
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}

int CTC_ChipsetVendorId_Translate( unsigned char *pVendorId, char *pVendorStr, unsigned long *pLen )
{
	short int i;
	
	ctcChipsetIdList_t *pItem = NULL;
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			if( VOS_MemCmp(pVendorId, ctcChipsetIdList[i].vendorId, CTC_CHIPSET_VENDORID_LEN) == 0 )
			{
				pItem = &ctcChipsetIdList[i];
				break;
			}
		}
	}
	if( pItem )
	{
		VOS_StrCpy( pVendorStr, pItem->vendorStr );
		*pLen = VOS_StrLen(pVendorStr);
	}
	else
	{
		VOS_StrCpy( pVendorStr, pVendorId );
		*pLen = VOS_StrLen(pVendorStr);
	}
	return VOS_OK;
}

int CTC_addChipsetVendorId( unsigned char *pVendorId, char *pVendorStr )
{
	short int i;
	short int j = CTC_CHIPSET_LIST_NUM;
	ctcChipsetIdList_t *pItem = NULL;
	
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			if( VOS_MemCmp(pVendorId, ctcChipsetIdList[i].vendorId, CTC_CHIPSET_VENDORID_LEN) == 0 )
			{
				pItem = &ctcChipsetIdList[i];
				break;
			}
		}
		else
		{
			if( j == CTC_CHIPSET_LIST_NUM ) 
				j = i;
		}
	}
	if( pItem == NULL )
	{
		if( j >= CTC_CHIPSET_LIST_NUM ) 
			return VOS_ERROR;
		
		pItem = &ctcChipsetIdList[j];
	}

	pItem->flag = 1;
	VOS_MemZero( pItem->vendorId, CTC_CHIPSET_VENDORID_LEN );
	VOS_MemZero( pItem->vendorStr, CTC_CHIPSET_VENDORID_STR_LEN );
	VOS_StrnCpy( pItem->vendorId, pVendorId, CTC_CHIPSET_VENDORID_LEN );
	VOS_StrnCpy( pItem->vendorStr, pVendorStr, CTC_CHIPSET_VENDORID_STR_LEN-1 );
	
	return VOS_OK;
}
int CTC_delChipsetVendorId( unsigned char *pVendorId )
{
	short int i;
	
	/*ctcChipsetIdList_t *pItem = NULL;*/
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			if( VOS_MemCmp(pVendorId, ctcChipsetIdList[i].vendorId, CTC_CHIPSET_VENDORID_LEN) == 0 )
			{
				VOS_MemZero( &ctcChipsetIdList[i], sizeof(ctcChipsetIdList_t) );
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}

int CTC_ChipsetMode_Translate( unsigned long mode, unsigned long *pMapMode )
{
	short int i;
	
	ctcChipsetModeList_t *pItem = NULL;
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetModeList[i].flag )
		{
			if( mode == ctcChipsetModeList[i].mode )
			{
				pItem = &ctcChipsetModeList[i];
				break;
			}
		}
	}
	if( pItem )
	{
		*pMapMode = pItem->mapMode;
	}
	else
	{
		*pMapMode = mode;
	}
	return VOS_OK;
}

int CTC_addChipsetMode( unsigned long mode, unsigned long mapMode )
{
	short int i;
	short int j = CTC_CHIPSET_LIST_NUM;
	ctcChipsetModeList_t *pItem = NULL;
	
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetModeList[i].flag )
		{
			if( mode == ctcChipsetModeList[i].mode )
			{
				pItem = &ctcChipsetModeList[i];
				break;
			}
		}
		else
		{
			if( j == CTC_CHIPSET_LIST_NUM ) 
				j = i;
		}
	}
	if( pItem == NULL )
	{
		if( j >= CTC_CHIPSET_LIST_NUM )
			return VOS_ERROR;
		
		pItem = &ctcChipsetModeList[j];
	}
	
	pItem->flag = 1;
	pItem->mode = mode;
	pItem->mapMode = mapMode;
	
	return VOS_OK;
}
int CTC_delChipsetMode( unsigned long mode )
{
	short int i;
	
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetModeList[i].flag )
		{
			if( mode == ctcChipsetModeList[i].mode )
			{
				VOS_MemZero( &ctcChipsetModeList[i], sizeof(ctcChipsetModeList_t) );
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}

int CTC_ChipsetRevision_Translate( unsigned char revision, char *pRevisionStr, unsigned long *pLen )
{
	short int i;
	
	ctcChipsetRevisionList_t *pItem = NULL;
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetRevisionList[i].flag )
		{
			if( revision == ctcChipsetRevisionList[i].revision )
			{
				pItem = &ctcChipsetRevisionList[i];
				break;
			}
		}
	}
	if( pItem )
	{
		VOS_StrCpy( pRevisionStr, pItem->revisionStr );
		*pLen = VOS_StrLen(pRevisionStr);
	}
	else
	{
		VOS_Sprintf( pRevisionStr, "%d", revision );
		*pLen = VOS_StrLen(pRevisionStr);
	}
	return VOS_OK;
}

int CTC_addChipsetRevision( unsigned char revision, char *pRevisionStr )
{
	short int i;
	short int j = CTC_CHIPSET_LIST_NUM;
	ctcChipsetRevisionList_t *pItem = NULL;
	
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetRevisionList[i].flag )
		{
			if( revision == ctcChipsetRevisionList[i].revision )
			{
				pItem = &ctcChipsetRevisionList[i];
				break;
			}
		}
		else
		{
			if( j == CTC_CHIPSET_LIST_NUM ) 
				j = i;
		}
	}
	if( pItem == NULL )
	{
		if( j >= CTC_CHIPSET_LIST_NUM ) 
			return VOS_ERROR;
		
		pItem = &ctcChipsetRevisionList[j];
	}

	pItem->flag = 1;
	pItem->revision = revision;
	VOS_MemZero( pItem->revisionStr, CTC_CHIPSET_REVISION_STR_LEN );
	VOS_StrnCpy( pItem->revisionStr, pRevisionStr, CTC_CHIPSET_REVISION_STR_LEN-1 );
	
	return VOS_OK;
}
int CTC_delChipsetRevision( unsigned char revision )
{
	short int i;
	
	/*ctcChipsetRevisionList_t *pItem = NULL;*/
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			if( revision == ctcChipsetRevisionList[i].revision )
			{
				VOS_MemZero( &ctcChipsetRevisionList[i], sizeof(ctcChipsetRevisionList_t) );
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}


#if 1
int searchGponOnuTypeString(const char *string)
{

    int i, ret = VOS_ERROR;

    for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
    {
        if( 1 == gponOnuEquipmentList[i].flag )
        {
            if(!VOS_StriCmp(gponOnuEquipmentList[i].equipmentStr, string))
            {
                ret = VOS_OK;
                break;
            }
        }
    }

    return ret;
}

int GPON_OnuModel_Translate(char *equipment, unsigned long *type, char *pEquipmentStr, int *pLen)
{
	int i/*, j*/;
	
	gponOnuEquipmentList_t *pItem = NULL;
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == gponOnuEquipmentList[i].flag )
		{
			if( VOS_StrCmp(gponOnuEquipmentList[i].equipment, equipment) == 0)
			{
				pItem = &gponOnuEquipmentList[i];
				break;
			}
		}
	}
	if( pItem )
	{
		*type = pItem->type;
		if( pEquipmentStr )
		{
			VOS_StrCpy( pEquipmentStr, pItem->equipmentStr);
			*pLen = VOS_StrLen(pEquipmentStr);
		}
	}
	else
	{
			*type = V2R1_ONU_GPON;
			if( pEquipmentStr )
			{
				VOS_StrCpy( pEquipmentStr, GetDeviceTypeString(V2R1_ONU_GPON) );
				*pLen = VOS_StrLen(pEquipmentStr);
			}
	}
	return VOS_OK;
}

/* added by xieshl 20130205, 问题单16958 */
int GPON_OnuEquipmentStr_2_Type( char *pEquipmentStr, unsigned long *pOnuType )
{
	int i, rc = VOS_ERROR;
	ctcOnuModelList_t *pItem = NULL;

	if( (pOnuType == NULL) || (pEquipmentStr == NULL) )
		return rc;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == gponOnuEquipmentList[i].flag )
		{
			if( VOS_StriCmp(pEquipmentStr, gponOnuEquipmentList[i].equipmentStr) == 0 )
			{
				pItem = &(gponOnuEquipmentList[i]);
				break;
			}
		}
	}
	if( pItem )
	{
		*pOnuType = pItem->type;
		rc = VOS_OK;
	}
	/*else
	{
		*pOnuType = V2R1_ONU_CTC;
	}*/
	return rc;
}


char * GPON_OnuType_2_Str( unsigned long onuType )
{
	int i;
	gponOnuEquipmentList_t *pItem = NULL;
	char *pOnuTypeStr = NULL;

	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == gponOnuEquipmentList[i].flag )
		{
			if( onuType == gponOnuEquipmentList[i].type )
			{
				pItem = &(gponOnuEquipmentList[i]);
				break;
			}
		}
	}
	if( pItem )
	{
		pOnuTypeStr = pItem->equipmentStr;
	}
	return pOnuTypeStr;
}

int GPON_getAppNameWithEquipment( char* equipment, char *name, int *pLen )
{
    int i/*, j*/;
    /*char *p;*/

    gponOnuEquipmentList_t *pItem = NULL;

    if(equipment == NULL)
    {
        return VOS_OK;
    }

    for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
    {
        if( 1 == gponOnuEquipmentList[i].flag )
        {
            if( VOS_StrCmp(gponOnuEquipmentList[i].equipment, equipment) == 0)
            {
                pItem = &gponOnuEquipmentList[i];
                break;
            }
        }
    }
    if( pItem )
    {
        VOS_StrCpy( name, pItem->equipmentAppStr);
        *pLen = VOS_StrLen(name);
    }
    else
    {
        VOS_StrCpy(name, GetOnuEponAppString(V2R1_ONU_GPON));
        *pLen = VOS_StrLen(name);
    }
    return VOS_OK;
}
void GPON_ClearOnuEquipment()
{
    VOS_MemZero(gponOnuEquipmentList, sizeof(gponOnuEquipmentList_t)*CTC_ONU_MODEL_LIST_NUM);    
}
int GPON_addOnuEquipment( char* equipment, unsigned long type, char *pEquipmentStr, char * appName )
{
	short int i;
	short int j = CTC_ONU_MODEL_LIST_NUM;
	gponOnuEquipmentList_t *pItem = NULL;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == gponOnuEquipmentList[i].flag )
		{
			if( VOS_StrCmp(gponOnuEquipmentList[i].equipment, equipment) == 0 )
			{
				pItem = &gponOnuEquipmentList[i];
				break;
			}
		}
		else
		{
			if( j == CTC_ONU_MODEL_LIST_NUM ) 
				j = i;
		}
	}
	if( pItem == NULL )
	{
		if( j >= CTC_ONU_MODEL_LIST_NUM )
			return VOS_ERROR;
		
		pItem = &gponOnuEquipmentList[j];
	}
	
	pItem->flag = 1;
	pItem->type = type;
	VOS_StrCpy(pItem->equipment, equipment);
	VOS_MemZero(pItem->equipmentStr, CTC_ONU_MODEL_STR_LEN);
	VOS_StrnCpy(pItem->equipmentStr, pEquipmentStr, CTC_ONU_MODEL_STR_LEN-1 );
	if(appName)
	{
		VOS_MemZero(pItem->equipmentAppStr, ONU_TYPE_LEN);
	   	VOS_StrnCpy(pItem->equipmentAppStr, appName, ONU_TYPE_LEN-1);
	}
	return VOS_OK;
}
int GPON_delOnuEquipment( char* equipment )
{
	short int i;
	
	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == gponOnuEquipmentList[i].flag )
		{
			if( VOS_StrCmp(gponOnuEquipmentList[i].equipment,equipment) == 0 )
			{
				VOS_MemZero( &gponOnuEquipmentList[i], sizeof(gponOnuEquipmentList_t) );
				return VOS_OK;
			}
		}
	}
	return VOS_ERROR;
}
#endif
/*** add igmp fast leave config  2008/9/8 ***/
int CTC_getOnuFastLeaveAbility( ULONG devIdx, ULONG *value )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	
#ifdef PAS_SOFT_VERSION_V5_3_5

	PON_llid_t	llid;
	int i;
	CTC_STACK_fast_leave_ability_t	fast_leave_ability ;
	unsigned char mode;

	if( value == NULL )
		return RERROR;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;

	if( GetOnuOperStatus(pon_id, onu_id) != 1 )
    		return VOS_ERROR;

	llid = GetLlidByOnuIdx( pon_id, onu_id );
	if( llid == INVALID_LLID )
		return RERROR;
#if 0
	if(CTC_STACK_get_fast_leave_ability(pon_id,llid, &fast_leave_ability) != CTC_STACK_EXIT_OK)
		return RERROR;
#else
    if(OnuMgt_GetMulticastFastleaveAbility(pon_id, onu_id, &fast_leave_ability) != CTC_STACK_EXIT_OK)
        return RERROR;
#endif

	mode = 0;

	for(i=0;i<fast_leave_ability.num_of_modes;i++)
		{
		if(i >= CTC_MAX_FAST_LEAVE_ABILITY_MODES)break;
		if(fast_leave_ability.modes[i] & CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_IGMP_SNOOPING_MODE)
			mode |= 0x40;
		else if(fast_leave_ability.modes[i] & CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MC_CONTROL_MODE)
			mode |= 0x80;
		}
	*value = (mode<<24);
	OnuMgmtTable[pon_id*MAXONUPERPON+onu_id].fastleaveAbility = mode;
	
	return ROK;
	
#else 

	if( value == NULL )
		return RERROR;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;
	*value = OnuMgmtTable[pon_id*MAXONUPERPON+onu_id].fastleaveAbility;
	
	return ROK;
#endif
}

int CTC_getOnuFastLeaveAdminState( ULONG devIdx, ULONG *fase_leave_control )
{
/*
#ifdef PAS_SOFT_VERSION_V5_3_5
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	CTC_STACK_fast_leave_admin_state_t	fast_leave_admin_state ;

	if( fase_leave_control == NULL )
		return RERROR;

	if( parse_llid_command_parameter(devIdx, &pon_id, &onu_id, &llid) == VOS_ERROR )
		return RERROR;
	if(CTC_STACK_get_fast_leave_admin_state(pon_id,llid, &fast_leave_admin_state) != CTC_STACK_EXIT_OK)
		return RERROR;
	if(fast_leave_admin_state == CTC_STACK_FAST_LEAVE_ADMIN_STATE_DISABLED)
		*fase_leave_control = 2;
	else if(fast_leave_admin_state == CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED)
		*fase_leave_control = 1;
	else return RERROR;

	return ROK;

#else*/
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;

	if( fase_leave_control == NULL )
		return RERROR;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;
	*fase_leave_control = OnuMgmtTable[pon_id*MAXONUPERPON+onu_id].fastleaveControl;
	
	return ROK;
/*#endif*/
}

int CTC_getOnuFastLeaveControl( ULONG devIdx, ULONG *fase_leave_control )
{
	PON_olt_id_t pon_id;
	PON_onu_id_t onu_id;

	if( fase_leave_control == NULL )
		return RERROR;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;
	*fase_leave_control = OnuMgmtTable[pon_id*MAXONUPERPON+onu_id].fastleaveControl;
	
	return ROK;
}
int CTC_setOnuFastLeaveControl( ULONG devIdx, ULONG fase_leave_control )
{
#ifdef  PAS_SOFT_VERSION_V5_3_5
	PON_olt_id_t pon_id;
	PON_llid_t	llid;
	PON_onu_id_t onu_id;
	
	CTC_STACK_fast_leave_admin_state_t  fast_leave_admin_state;

	if( parse_onuidx_command_parameter(devIdx, &pon_id, &onu_id) == VOS_ERROR )
		return RERROR;

	if(fase_leave_control == 1)
		fast_leave_admin_state = CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED;
	else if(fase_leave_control == 0)
		fast_leave_admin_state = CTC_STACK_FAST_LEAVE_ADMIN_STATE_DISABLED;
	else
		return RERROR;
	
	if( GetOnuOperStatus(pon_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( pon_id, onu_id );
	if( llid == INVALID_LLID )
		return RERROR;
#if 0
	if(CTC_STACK_set_fast_leave_admin_control(pon_id,llid, fast_leave_admin_state) != CTC_STACK_EXIT_OK)
	    return RERROR;
#else
	    if(OnuMgt_SetMulticastFastleave(pon_id, onu_id, fast_leave_admin_state) != CTC_STACK_EXIT_OK)
	        return RERROR;
#endif


	OnuMgmtTable[pon_id*MAXONUPERPON+onu_id].fastleaveControl = fase_leave_control;	
#endif
	
	return ROK;
}

DEFUN(
	onu_show_ctc_fastleave_ability_and_state,
	onu_show_ctc_fastleave_ability_and_state_cmd,
	"show ctc igmp-fastleave",
	CTC_STR
	"show ctc onu info\n"
	"show ctc onu fast-leave abiltiy and state\n")
{
	unsigned long ulIfIndex;
	unsigned long lRet;
	unsigned long ulSlot, ulPort, ulOnuid;
	unsigned long devIdx; 
	unsigned long ability, state;
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	/*devIdx = ulSlot*10000 + ulPort*1000 + ulOnuid;*/
        devIdx=MAKEDEVID(ulSlot,ulPort,ulOnuid);

	CTC_getOnuFastLeaveAbility(devIdx, &ability);
	CTC_getOnuFastLeaveControl(devIdx, &state);

	vty_out(vty,"onu%d/%d/%d fast leave\r\n",ulSlot,ulPort,ulOnuid);
	vty_out(vty,"ability:");
	if((ability & (1<<(32-1))) && (ability & (1<<(32-2))))
		vty_out(vty,"igmp snooping & ctc multicast control\r\n");
	else if(ability & (1<<(32-2)))
		vty_out(vty,"igmp snooping\r\n");
	else if(ability & (1<<(32-1)))
		vty_out(vty,"ctc multicast control\r\n");
	else vty_out(vty,"none\r\n");
	
	vty_out(vty,"admin state:");
	(state == 1) ? vty_out(vty,"enable\r\n"):vty_out(vty,"disable\r\n");		
	
	return CMD_SUCCESS;

}

DEFUN(
	onu_set_ctc_fastleave_state,
	onu_set_ctc_fastleave_state_cmd,
	"ctc igmp-fastleave {[enable|disable]}*1",
	CTC_STR
	"config ctc onu fast-leave state\n"
	"enable\n"
	"disable\n")
{
	unsigned long ulIfIndex;
	unsigned long lRet;
	unsigned long ulSlot, ulPort, ulOnuid;
	unsigned long devIdx, suffix;
	unsigned long state;

	ulIfIndex = ( ULONG ) ( vty->index ) ;

	if(argc == 1)
	{
        if( VOS_StrCmp( argv[ 0 ] , "enable" )  == 0 )
            state = 1;
        else
            state = 0;		
	}

	if(!IsProfileNodeVty(ulIfIndex, &suffix))
	{
	    short int olt_id, onu_id;
        lRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
        if( lRet != VOS_OK )
            return CMD_WARNING;

        devIdx = MAKEDEVID(ulSlot, ulPort, ulOnuid);

        if(argc == 1)
        {
            if(CTC_setOnuFastLeaveControl(devIdx, state) == ROK)
                return CMD_SUCCESS;
            else
            {
#if 0                
                vty_out(vty, "ctc igmp-fastleave %s fail!\r\n", argv[0]);
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
                return CMD_WARNING;
            }
        }
        else
        {
            ULONG ability;
            CTC_getOnuFastLeaveAbility(devIdx, &ability);

            if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
            {
                vty_out(vty, "onu index error!\r\n");
                return CMD_WARNING;
            }
            else if(OnuMgt_GetMulticastFastleave(olt_id, onu_id, &state) != VOS_OK)
            {
#if 0
                vty_out(vty, "get igmp snooping fastleave fail!\r\n");
#else
                vty_out(vty, ONU_CMD_ERROR_STR);
#endif
            }
            vty_out(vty,"onu%d/%d/%d fast leave\r\n",ulSlot,ulPort,ulOnuid);
            vty_out(vty,"ability:");
            if((ability & (1<<(32-1))) && (ability & (1<<(32-2))))
                vty_out(vty,"igmp snooping & ctc multicast control\r\n");
            else if(ability & (1<<(32-2)))
                vty_out(vty,"igmp snooping\r\n");
            else if(ability & (1<<(32-1)))
                vty_out(vty,"ctc multicast control\r\n");
            else vty_out(vty,"none\r\n");

            vty_out(vty,"admin state:");
            (state == CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED) ? vty_out(vty,"enable\r\n"):vty_out(vty,"disable\r\n");
        }

	}
	else
	{
	    if(argc == 1)
	    {
            if(setOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_igmp_fastleave_enable, (state == 1)?1:0) != VOS_OK)
            {
                vty_out(vty, "ctc igmp-fastleave %s fail!\r\n", argv[0]);
                return CMD_WARNING;
            }
	    }
	    else
	    {
	        if(getOnuConfSimpleVarByPtr(suffix, vty->onuconfptr, sv_enum_igmp_fastleave_enable, &state) != VOS_OK)
	        {
	            vty_out(vty, "ctc igmp-fastleave get fail!\r\n");
	            return CMD_WARNING;
	        }
            else
            {
	            vty_out(vty, " ctc igmp-fastleave %s\r\n",state==1?"enable":"disable");
            }
	    }
	}
    return CMD_SUCCESS;
}

DEFUN(
	set_ctc_chipset_revision,
	set_ctc_chipset_revision_cmd,
	"ctc chipset-revision <revision> <map_revision_name>",
	CTC_STR
	"rename CTC ONU PON chipset revision\n"
	"mapping the revision which onu reported, in decimal value\n"
	"map the revision to revision-name\n")
{
	unsigned long revision;
	char *pToken;
	
	revision = VOS_StrToUL( argv[0], &pToken, 10 );

	if( CTC_addChipsetRevision(revision, argv[1]) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	del_ctc_chipset_revision,
	del_ctc_chipset_revision_cmd,
	"undo ctc chipset-revision <revision>",
	NO_STR
	CTC_STR
	"undo mapping CTC ONU PON chipset revision, in decimal value\n"
	"chipset revision\n")
{
	unsigned long revision = 0;
	char *pToken;
	
	revision = VOS_StrToUL( argv[0], &pToken, 10 );

	if( CTC_delChipsetRevision(revision) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	show_ctc_chipset_revision,
	show_ctc_chipset_revision_cmd,
	"show ctc chipset-revision",
	SHOW_STR
	CTC_STR
	"show CTC PON chipset revision mapping\n")
{
	short int i;

	vty_out( vty, "   chipset-revision    map-name\r\n" );
	/*vty_out( vty, "  ------------------------------\r\n" );*/

	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetRevisionList[i].flag )
		{
			vty_out( vty, "   %8x            %s\r\n", ctcChipsetRevisionList[i].revision, ctcChipsetRevisionList[i].revisionStr );
		}
	}
	return CMD_SUCCESS;
}

DEFUN(
	set_ctc_chipset_mode,
	set_ctc_chipset_mode_cmd,
	"ctc chipset-mode <mode> <map_mode>",
	CTC_STR
	"map the PON chipset mode to a new one\n"
	"chipset mode which onu reported in decimal value\n"
	"mapping mode in decimal\n")
{
	unsigned long mode, mapMode;
	char *pToken;
	
	mode = VOS_StrToUL( argv[0], &pToken, 10 );
	mapMode = VOS_StrToUL( argv[1], &pToken, 10 );

	if( mode > 0xffff )
	{
		vty_out( vty, " hex_mode is too big\r\n" );
		return CMD_WARNING;
	}	
	if( mapMode > 0xffff )
	{
		vty_out( vty, " dec_mode is too big\r\n" );
		return CMD_WARNING;
	}	
	if( CTC_addChipsetMode(mode, mapMode) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	del_ctc_chipset_mode,
	del_ctc_chipset_mode_cmd,
	"undo ctc chipset-mode <mode>",
	NO_STR
	CTC_STR
	"undo map the PON chipset mode\n"
	"chipset-mode in decimal\n")
{
	char *pToken;
	unsigned long mode10;
	
	mode10 = VOS_StrToUL( argv[0], &pToken, 10 );
	if( mode10 > 0xffff )
	{
		vty_out( vty, " hex_mode is too big\r\n" );
		return CMD_WARNING;
	}	

	if( CTC_delChipsetMode(mode10) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	show_ctc_chipset_mode,
	show_ctc_chipset_mode_cmd,
	"show ctc chipset-mode",
	SHOW_STR
	CTC_STR
	"show chipset mode mapping\n")
{
	short int i;

	vty_out( vty, "     mode    mao-mode\r\n" );
	/*vty_out( vty, "  ------------------------\r\n" );*/

	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetModeList[i].flag )
		{
			vty_out( vty, "   %6d    %d\r\n", ctcChipsetModeList[i].mode, ctcChipsetModeList[i].mapMode );
		}
	}
	return CMD_SUCCESS;
}

DEFUN(
	set_ctc_chipset_vendor,
	set_ctc_chipset_vendor_cmd,
	"ctc chipset-vendor <vendor_id> <map_vendor_name>",
	CTC_STR
	"map the PON chipset vendor ID\n"
	"chipset vendor ID which onu reported\n"
	"map the vendor ID to vendor-name\n")
{
	if( VOS_StrLen(argv[0]) > CTC_CHIPSET_VENDORID_LEN )
	{
		vty_out( vty, " vendor ID is too long\r\n" );
		return CMD_WARNING;
	}	
	if( VOS_StrLen(argv[1]) >= CTC_CHIPSET_VENDORID_STR_LEN )
	{
		vty_out( vty, " vendor name is too long\r\n" );
		return CMD_WARNING;
	}	
	if( CTC_addChipsetVendorId(argv[0], argv[1]) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	del_ctc_chipset_vendor,
	del_ctc_chipset_vendor_cmd,
	"undo ctc chipset-vendor <vendor_id>",
	NO_STR
	CTC_STR
	"undo map the PON chipset id to vendor name\n"
	"vendor ID\n")
{
	if( VOS_StrLen(argv[0]) > CTC_CHIPSET_VENDORID_LEN )
	{
		vty_out( vty, " vendor ID is too long\r\n" );
		return CMD_WARNING;
	}	

	if( CTC_delChipsetVendorId(argv[0]) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	show_ctc_chipset_vendor,
	show_ctc_chipset_vendor_cmd,
	"show ctc chipset-vendor",
	SHOW_STR
	CTC_STR
	"show chipset vendor mapping\n")
{
	short int i;

	vty_out( vty, "   vendor-ID    vendor\r\n" );
	/*vty_out( vty, "  -----------------------\r\n" );*/

	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			vty_out( vty, "   %8s     %s\r\n", ctcChipsetIdList[i].vendorId, ctcChipsetIdList[i].vendorStr );
		}
	}
	return CMD_SUCCESS;
}

DEFUN(
	set_ctc_onu_model,
	set_ctc_onu_model_cmd,
	"ctc onu-model <model> <map_model_name>",
	CTC_STR
	"rename CTC ONU model\n"
	"mapping the onu model which onu reported, in hex value\n"
	"map the onu model to a new name\n")
{
	unsigned long model;
	char *pToken;
	
	model = VOS_StrToUL( argv[0], &pToken, 16 );

	if( CTC_addOnuModel(model, V2R1_ONU_CTC, argv[1], NULL) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	del_ctc_onu_model,
	del_ctc_onu_model_cmd,
	"undo ctc onu-model <model>",
	NO_STR
	CTC_STR
	"undo mapping CTC ONU model, in hex value\n"
	"onu model\n")
{
	unsigned long model;
	char *pToken;
	
	model = VOS_StrToUL( argv[0], &pToken, 16 );

	if( CTC_delOnuModel(model) != VOS_OK )
	{
		CTC_STACK_CLI_ERROR_PRINT;
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
	show_ctc_onu_model,
	show_ctc_onu_model_cmd,
	"show ctc onu-model",
	SHOW_STR
	CTC_STR
	"show CTC ONU model mapping\n")
{
	short int i;

	vty_out( vty, "   ONU-model    ONU-type\r\n" );
	/*vty_out( vty, "  -----------------------\r\n" );*/

	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			vty_out( vty, "   %8x     %s\r\n", ctcOnuModelList[i].model, ctcOnuModelList[i].modelStr );
		}
	}
	return CMD_SUCCESS;
}

#if	1	/*#ifdef __CTC_TEST_*/
DEFUN(
	chg_ctc_onu_cap,
	chg_ctc_onu_cap_cmd,
	"ctc onu-capability <devIdx> <eth_num> <voip_num>",
	CTC_STR
	"config onu capability\n"
	"onu devidx\n"
	"ethernet port number\n"
	"voip port number\n")
{
	char *pToken;
	unsigned long devIdx;
	PON_olt_id_t PonPortIdx;
	PON_onu_id_t OnuIdx;
	short eth_num = 0, voip_num = 0;
	
	devIdx = VOS_StrToUL( argv[0], &pToken, 10 );
	eth_num = VOS_StrToUL( argv[1], &pToken, 10 );
	voip_num = VOS_StrToUL( argv[2], &pToken, 10 );
	if( parse_onuidx_command_parameter(devIdx, &PonPortIdx, &OnuIdx) != VOS_OK )
	{
		vty_out( vty, "devIdx is err\r\n" );
		return CMD_WARNING;
	}

	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_supporting   =  NOT_SUPPORTING;
	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_supporting   =  NOT_SUPPORTING;
	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].VoIP_supporting =  NOT_SUPPORTING;

	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].GE_Ethernet_ports_number = 0;
	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].FE_Ethernet_ports_number = eth_num;
	OnuMgmtTable[MAXONUPERPON * PonPortIdx + OnuIdx].POTS_ports_number = voip_num;
	return CMD_SUCCESS;
}
#endif

extern LONG onu_auth_cli_cmd_install();
int CTC_onu_cli_cmd_install()
{
    /*modi by luh 2013-1-31*/
	install_element ( /*CONFIG_NODE*/DEBUG_HIDDEN_NODE, &set_ctc_onu_model_cmd);
	install_element ( /*CONFIG_NODE*/DEBUG_HIDDEN_NODE, &del_ctc_onu_model_cmd);
	install_element ( CONFIG_NODE, &show_ctc_onu_model_cmd);
	install_element ( CONFIG_NODE, &set_ctc_chipset_vendor_cmd);
	install_element ( CONFIG_NODE, &del_ctc_chipset_vendor_cmd);
	install_element ( CONFIG_NODE, &show_ctc_chipset_vendor_cmd);
	install_element ( CONFIG_NODE, &set_ctc_chipset_mode_cmd);
	install_element ( CONFIG_NODE, &del_ctc_chipset_mode_cmd);
	install_element ( CONFIG_NODE, &show_ctc_chipset_mode_cmd);
	install_element ( CONFIG_NODE, &set_ctc_chipset_revision_cmd);
	install_element ( CONFIG_NODE, &del_ctc_chipset_revision_cmd);
	install_element ( CONFIG_NODE, &show_ctc_chipset_revision_cmd);

#ifdef PAS_SOFT_VERSION_V5_3_5
	/*install_element ( ONU_CTC_NODE, &onu_show_ctc_fastleave_ability_and_state_cmd );*/
	install_element ( ONU_CTC_NODE, &onu_set_ctc_fastleave_state_cmd );
	install_element ( ONU_PROFILE_NODE, &onu_set_ctc_fastleave_state_cmd);
#endif	

/*#ifdef __CTC_TEST*/
#if 1
	install_element ( DEBUG_HIDDEN_NODE, &chg_ctc_onu_cap_cmd);
#endif

	onu_auth_cli_cmd_install();

	return VOS_OK;
}

/*extern LONG CTC_onu_auth_show_run( struct vty * vty );*/
LONG CTC_onu_show_run( struct vty * vty )
{
	int i;

	vty_out( vty, "!CTC ONU parameter mapping config\r\n" );

	for( i=0; i<CTC_ONU_MODEL_LIST_NUM; i++ )
	{
		if( 1 == ctcOnuModelList[i].flag )
		{
			vty_out( vty, " ctc onu-model %x %s\r\n", ctcOnuModelList[i].model, ctcOnuModelList[i].modelStr );
		}
	}

	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetIdList[i].flag )
		{
			vty_out( vty, " ctc chipset-vendor %s %s\r\n", ctcChipsetIdList[i].vendorId, ctcChipsetIdList[i].vendorStr );
		}
	}

	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetModeList[i].flag )
		{
			vty_out( vty, " ctc chipset-mode %d %d\r\n", ctcChipsetModeList[i].mode, ctcChipsetModeList[i].mapMode );
		}
	}
	
	for( i=0; i<CTC_CHIPSET_LIST_NUM; i++ )
	{
		if( 1 == ctcChipsetRevisionList[i].flag )
		{
			vty_out( vty, " ctc chipset-revision %d %s\r\n", ctcChipsetRevisionList[i].revision, ctcChipsetRevisionList[i].revisionStr );
		}
	}

	/*CTC_onu_auth_show_run( vty );*/
	
	vty_out( vty, "!\r\n\r\n" );
	
	return VOS_OK;
}
/* end 20080807 */ 
int CTC_SetOnuUniPort(short int olt_id, short int llid, bool enable_cpu, bool enable_datapath)
{
    int iRlt = OLT_ERR_NOTSUPPORT;
    
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON) 
        iRlt = GW10G_REMOTE_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath);
    else
#endif
    if (SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER)
        if (SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER)
            iRlt = RM_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath);   
        else
            iRlt = TK_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath);   
    else
        iRlt = REMOTE_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath);   
    
	return iRlt;
}

int CTC_GetOui ( unsigned long  *oui )
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_get_oui(oui);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_get_oui(oui);
	}

	return iRlt;
    
}

int CTC_SetOui ( const unsigned long  oui )
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_set_oui(oui);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_set_oui(oui);
	}

	return iRlt;
}

int CTC_GetVersion( unsigned char  *version )
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_get_version(version);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_get_version(version);
	}

	return iRlt;
	
	
}

int CTC_SetVersion( const unsigned char  version )
{
    int iRlt;
    
	
#if defined(_EPON_10G_PMC_SUPPORT_)            
	if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
	{
	    iRlt = GW10G_CTC_STACK_set_version(version);
	}
	else 
#endif
	{
	    iRlt = CTC_STACK_set_version(version);
	}

	return iRlt;
    
}

