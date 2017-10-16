#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include  "gwEponSys.h"
#include "CT_RMan_Main.h"


/*----------------------------------------------------------------------------*/

ULONG	CT_ExtOamDebugSwitch = 1;


int CT_RMan_Init()
{
	/*initCtExtOamComm();*/

	/*CT_RMan_CLI_Init();*/

	return VOS_OK;
}

extern char *macAddress_To_Strings(UCHAR *pMacAddr);
/*char *macAddress_To_Strings(UCHAR *pMacAddr)
{
	static char mac_str[20];

	if( pMacAddr != NULL )
	{
		VOS_Sprintf( mac_str, "%02x%02x.%02x%02x.%02x%02x",
				pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5] );
	}
	else
		VOS_StrCpy( mac_str,  "0000.0000.0000");
	return mac_str;
}*/

char *binData_To_Strings(UCHAR *pBin, ULONG len, UCHAR separator )
{
	static char bin_str[1024];
	char tmp_str[8];
	int i;

	if( (pBin != NULL) || ((len < 1024) && (len >= 1)) )
	{
		bin_str[0] = 0;
		for( i=len-1; i>=0; i-- )
		{
			if( separator == 0 )
				VOS_Sprintf( tmp_str, "%02x", pBin[i] );
			else	
				VOS_Sprintf( tmp_str, "%02x%c", pBin[i], separator );
			VOS_StrCat( bin_str, tmp_str );
		}
	}
	else
		VOS_StrCpy( bin_str,  "00");
	return bin_str;
}

char *bitmap_To_Strings(UCHAR *pBitmap, ULONG len )
{
	static char bin_str[1024];
	char tmp_str[8];
	int i;

	if( (pBitmap != NULL) || ((len < 1023/3) && (len >= 1)) )
	{
		bin_str[0] = 0;
		for( i=len-1; i>=0; i-- )
		{
			if( ((i & 4) == 0) && (i != 0) )
				VOS_StrCat( bin_str, " " );
			VOS_Sprintf( tmp_str, "%02x", pBitmap[i] );
			VOS_StrCat( bin_str, tmp_str );
		}
	}
	else
		VOS_StrCpy( bin_str,  "00");
	return bin_str;
}
/*----------------------------------------------------------------------------*/


int parse_onuidx_command_parameter( ULONG devIdx, PON_olt_id_t *pPonIdx, PON_onu_id_t *pOnuIdx)
{
	ULONG ulSlot, ulPort, ulOnu;
	PON_olt_id_t	olt_id;

	int err_rc = VOS_ERROR;
	
	if( (pPonIdx == NULL) || (pOnuIdx == NULL) )
	{
		VOS_ASSERT(0);
    		return err_rc;
	}
    	if( devIdx == 1 )
		return err_rc;

	/*ulSlot = devIdx / 10000;
	ulPort = (devIdx%10000)/1000;
	ulOnu = devIdx % 1000;*/
        ulSlot=GET_PONSLOT(devIdx);
        ulPort=GET_PONPORT(devIdx);
        ulOnu=GET_ONUID(devIdx);
	
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if( (olt_id == err_rc) || (ulOnu == 0) || (ulOnu > 64) )
	{
		/*sys_console_printf( "  GetPonPortIdxBySlot error:olt_id,onu_id\r\n");*/
    	return err_rc;
	}

	*pPonIdx = olt_id;
	*pOnuIdx = ulOnu - 1;
	
	return VOS_OK;
}

/* modified by xieshl 20080910, 调用该函数的地方，把onu_id和llid搞混了，导致CTC ONU数据保持和恢复
    错误，在数据恢复时甚至导致配置数据缓存越界 */
int parse_llid_command_parameter( ULONG devIdx, PON_olt_id_t *pOltId,	PON_onu_id_t *pOnuId, PON_llid_t *pLlidId)
{
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	PON_llid_t	llid;
	int err_rc = VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == err_rc )
		return err_rc;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
		/*sys_console_printf( "  GetOnuOperStatus error\r\n");*/
    		return err_rc;
	}

	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	*pOltId = olt_id;
	*pOnuId = onu_id;
	*pLlidId = llid;
	
	return VOS_OK;
}

/*从Llid中得到Devid用以告警by jinhl*/
ULONG parseDevidxFromLlid( const short int ponIdx, const short int llid )
{
	ULONG slot=0, port=0, ret = 0;
	short int onuIdx = 0;
	
	if( VOS_OK == ponIndex2slot( ponIdx, &slot, &port ) )
	{
		/*ret = slot*10000+port*1000+onuIdx+1;*/
		onuIdx = GetOnuIdxByLlid(ponIdx, llid);
               ret=MAKEDEVID(slot,port,(onuIdx+1));
	}

	return ret;
}

#ifndef CTC_OBSOLETE		/* removed by xieshl 20120607 */
/*******************************************************************************************
函数名：parseOnuIndexFromDevIdx
功能：	将设备索引转换为pon端口索引、pon上ONU索引，返回onu从0开始的编号
输入：	devIdx,设备索引；
输出：	ponIdx,ONU所在PON端口编号(0-19);   onuIdx,onu在PON上的编号（0－63）,两个参数可以同时为NULL
返回值：ONU的编号(0-19*64); -1,错误
*******************************************************************************************/
short int parseOnuIndexFromDevIdx( const ULONG devIdx, ULONG * pPonIdx, ULONG *pOnuIdx )
{
	short int ret = VOS_ERROR;
	short int ponIdx, onuIdx;
	LOCATIONDES	lct = {0};
	
	if( (OLT_DEV_ID != devIdx) && (getLocation( devIdx, &lct, CONV_YES ) == VOS_OK) )
	{
		onuIdx = lct.onuId - 1;
		ponIdx= GetPonPortIdxBySlot( lct.slot, lct.port );
		if( ponIdx == VOS_ERROR )		/* modified by xieshl 20080812 */
			return ret;
		ret = ponIdx * MAXONUPERPON + onuIdx;

		if( NULL != pPonIdx )
		{
			*pPonIdx = ponIdx;
		}
		if( NULL != pOnuIdx )
		{
			*pOnuIdx = onuIdx;
		}
	}
	return ret;
}

ULONG parseDevidxFromPonOnu( const short int ponIdx, const short int onuIdx )
{
	ULONG slot=0, port=0, ret = 0;
	if( VOS_OK == ponIndex2slot( ponIdx, &slot, &port ) )
	{
		/*ret = slot*10000+port*1000+onuIdx+1;*/
               ret=MAKEDEVID(slot,port,(onuIdx+1));
	}

	return ret;
}

ULONG parseDevidxFromOnuEntry( const int onuEntry )
{
	ULONG ret = 0;

	if( 0 <= onuEntry && onuEntry < MAXONU )
	{
		ULONG onuid = onuEntry%MAXONUPERPON;
		ULONG ponid = onuEntry/MAXONUPERPON;

		ret = parseDevidxFromPonOnu( ponid, onuid );
	}

	return ret;
}

int parse_onu_devidx_command_parameter( struct vty *vty, ULONG *devIdx, ULONG *slotno, ULONG *pon, ULONG *onu)
{
    ULONG ulIfIndex = 0;	

 	if( devIdx == NULL || slotno==NULL || pon == NULL || onu == NULL )
		return VOS_ERROR;
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	if( PON_GetSlotPortOnu( ulIfIndex, slotno, pon, onu) == VOS_OK )
	{
		*devIdx = MAKEDEVID(*slotno, *pon, *onu);
		return VOS_OK;
	}
	return VOS_ERROR;
}

int parse_onu_command_parameter( struct vty *vty, PON_olt_id_t *pPonPortIdx, PON_onu_id_t *pOnuIdx, PON_onu_id_t *pOnuId)
{
	ULONG ulSlot, ulPort, ulOnu;
	PON_olt_id_t	olt_id;
	PON_onu_id_t	onu_id;
	int err_rc = VOS_ERROR;
	
	if( (pPonPortIdx == NULL) || (pOnuId == NULL) )
	{
		VOS_ASSERT(0);
    		return err_rc;
	}

	if( PON_GetSlotPortOnu( (ULONG)vty->index, &ulSlot, &ulPort, &ulOnu ) != VOS_OK )
	{
		vty_out( vty, "  PON_GetSlotPortOnu error\r\n");
    		return err_rc;
	}
	olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if (olt_id == err_rc )
	{
		vty_out( vty, "  GetPonPortIdxBySlot error\r\n");
    		return err_rc;
	}

	/*if( !PonPortIsWorking(olt_id) )
	{
		vty_out( vty, "  PonPortIsWorking error\r\n");
    		return err_rc;
	}*/

	onu_id = ulOnu - 1;
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
		vty_out( vty, "  onu is off-line\r\n");
    		return err_rc;
	}

	if( pOnuIdx )  *pOnuIdx = onu_id;
	
	onu_id = GetLlidByOnuIdx( olt_id, onu_id );
	if( onu_id == -1 )
	{
		vty_out( vty, "  GetLlidByOnuIdx error\r\n");
		return VOS_ERROR;
	}

	*pPonPortIdx = olt_id;
	*pOnuId = onu_id;
	
	return VOS_OK;
}
#endif

#if 0 /* __test_ctc*/

static UCHAR __test_ctc_version = 1;
PON_STATUS CTC_STACK_get_version( unsigned char  *version )
{
	*version = __test_ctc_version;
	return 0;
}
PON_STATUS CTC_STACK_set_version( const unsigned char  version )
{
	__test_ctc_version = version;
	return 0;
}

static  ULONG __test_ctc_oui = 0x00;
PON_STATUS CTC_STACK_set_oui( const unsigned long  oui )
{
	__test_ctc_oui = oui;
	return 0;
}
PON_STATUS CTC_STACK_get_oui( unsigned long  *oui )
{
	*oui = __test_ctc_oui;
	return 0;
}
PON_STATUS CTC_STACK_reset_onu ( const PON_olt_id_t olt_id, 	const PON_onu_id_t onu_id )
{
	return 0;
}
static USHORT __test_ctc_discovery_timeout = 100;
PON_STATUS CTC_STACK_set_extended_oam_discovery_timing( unsigned short int discovery_timeout )
{
	__test_ctc_discovery_timeout = discovery_timeout;
	return 0;
}
PON_STATUS CTC_STACK_get_extended_oam_discovery_timing( unsigned short int  *discovery_timeout )
{
	*discovery_timeout = __test_ctc_discovery_timeout;
	return 0;
}

static CTC_oui_version_record_t __params_list[32] = {{{0,0x0f,0xe9},1},{{0,1,2},2}};
static UCHAR __params_number_of_records = 0;
static UCHAR __params_auto_mode = ENABLE;
static UCHAR __params_auto_onucfg_mode = DISABLE;
PON_STATUS CTC_STACK_get_parameters( unsigned char	  *automatic_mode,
			unsigned char		  *number_of_records,
			CTC_oui_version_record_t  *oui_version_records_list,
			unsigned char		  *automatic_onu_configuration)
{
	*automatic_mode = __params_auto_mode;
	*number_of_records = __params_number_of_records;
	*automatic_onu_configuration = __params_auto_onucfg_mode;
	VOS_MemCpy( oui_version_records_list, __params_list, __params_number_of_records * sizeof(CTC_oui_version_record_t) );
	return 0;
}
PON_STATUS CTC_STACK_set_parameters(
			const unsigned char automatic_mode,  
			const unsigned char  number_of_records,
			const CTC_oui_version_record_t  *oui_version_records_list,
			const unsigned char	 automatic_onu_configuration)
{
	__params_auto_mode = automatic_mode;
	__params_number_of_records = number_of_records;
	__params_auto_onucfg_mode = automatic_onu_configuration;
	VOS_MemCpy( __params_list, oui_version_records_list, number_of_records * sizeof(CTC_oui_version_record_t) );
	return 0;
}

static unsigned char __ctc_update_key_time = 100;
static unsigned short __ctc_no_reply_timeout = 1000;
PON_STATUS CTC_STACK_get_encryption_timing( unsigned char  *update_key_time, unsigned short int  *no_reply_timeout )
{
	*update_key_time = __ctc_update_key_time;
	*no_reply_timeout = __ctc_no_reply_timeout;
	return 0;
}

PON_STATUS CTC_STACK_set_encryption_timing( const unsigned char  update_key_time, const unsigned short int  no_reply_timeout )
{
	__ctc_update_key_time = update_key_time;
	__ctc_no_reply_timeout = no_reply_timeout;
	return 0;
}

static UCHAR __ctc_start_encryption_threshold = 0;
PON_STATUS CTC_STACK_set_encryption_timing_threshold( const unsigned char   start_encryption_threshold )
{
	__ctc_start_encryption_threshold = start_encryption_threshold;
	return 0;
}
PON_STATUS CTC_STACK_get_encryption_timing_threshold( unsigned char   *start_encryption_threshold )
{
	*start_encryption_threshold = __ctc_start_encryption_threshold;
}

PON_STATUS CTC_STACK_start_encryption( const PON_olt_id_t   olt_id, const PON_llid_t  llid )
{
	return 0;
}
PON_STATUS CTC_STACK_stop_encryption( const PON_olt_id_t   olt_id, const PON_llid_t  llid )
{
	return 0;
}

static UCHAR __ctc_number_of_queue_sets = 1;
static CTC_onu_queue_set_thresholds_t  __ctc_queues_thresholds[4];
PON_STATUS CTC_STACK_set_dba_report_thresholds( const PON_olt_id_t olt_id, 
			const PON_onu_id_t onu_id,  unsigned char *number_of_queue_sets,
			CTC_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{	
	__ctc_number_of_queue_sets = *number_of_queue_sets;
	VOS_MemCpy((VOID*)__ctc_queues_thresholds, (VOID*)queues_sets_thresholds, sizeof(__ctc_queues_thresholds) );
	return 0;
}
PON_STATUS CTC_STACK_get_dba_report_thresholds( const PON_olt_id_t olt_id, 
			const PON_onu_id_t  onu_id,  unsigned char *number_of_queue_sets,
			CTC_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
	*number_of_queue_sets = __ctc_number_of_queue_sets;
	VOS_MemCpy( (VOID*)queues_sets_thresholds, (VOID*)__ctc_queues_thresholds, sizeof(__ctc_queues_thresholds) );
	return 0;
}


#endif
