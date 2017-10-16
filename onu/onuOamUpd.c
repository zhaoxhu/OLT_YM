

/* modified by chenfj  2009-5-18
     数组onuque[][][] 在使用中槽位号下标减3 操作在6100 环境中是错误的；
     存在造成通过GW-OAM 方式升级ONU 软件时失败，并破坏内存的风险
     
     修改:重新定义数据的下标，使机框中的每个槽位与数组
     下标一一对应。 省去映射
     数组操作中槽位号下标减3 操作
*/

#ifdef __cpluplus
extern "C" {
#endif

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_Product.h"

#include "gwEponSys.h"
#include "gwEponMibData.h"
#include "V2R1General.h"
#include "TransFile.h"
#include "Cdp_pub.h"
#include "onuOamUpd.h"

#include "Device_flash.h"


#define    ONU_UPDATE_TASK    "tOamUpd"
#define    MAX_UPDATE_MSG_NUM    512

const  USHORT WAIT_UPDATE = 1;
const  USHORT CLR_UPDATE = 0;

#define GPON_WAIT_UPDATE  1
#define GPON_CLR_UPDATE   0

ULONG  onuOamUpdQueId = 0;
VOS_HANDLE onuOamUpdTaskId = NULL;
/*LONG onuOamUpdTimerId = 0;*/

USHORT onu_oam_upd_waiting_list[SYS_SLOTNUM+1][MAXPONPORT_PER_BOARD+1][MAXONUPERPONNOLIMIT+1];

static unsigned char s_ctcOnuNeedCommit[SYS_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT];

typedef struct {
	LONG wsTimers;
	ULONG wsOnus;
} onu_oam_upd_aging_t;
onu_oam_upd_aging_t onu_oam_upd_aging_list[MAX_UPDATE_ONU_NUM];


int  OnuSwUpdateStart( DeviceIndex_S  DevIdx, ULONG vendorType, ULONG spec_type, ULONG fileType );
int  StartOnuSWUpdate( short int PonPortIdx, short int OnuIdx , int Blockflag, unsigned int fileType);
VOID  onuOamUpd_CDP_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID *pData, ULONG ulDataLen);
LONG onuOamUpdCommandInstall();
extern int GetOnuSwUpdateType( short int PonPortIdx, short int OnuIdx);

USHORT *getOnuOamUpdWaitingOnuList( ULONG slot, ULONG pon)
{
	if( (slot > SYS_SLOTNUM) || (pon > MAXPONPORT_PER_BOARD) )
	{
		VOS_ASSERT(0);
		return NULL;
	}
	return (onu_oam_upd_waiting_list[slot][pon]);
}

static VOID setOnuOamUpdFlag( ULONG idx, const USHORT flag )
{
	LOCATIONDES lct;
	if( getLocation( idx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return;
		}
		onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId] = flag;
	}
}
static VOID getOnuOamUpdFlag( ULONG idx,USHORT *flag )
{
	LOCATIONDES lct;
	if( getLocation( idx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return;
		}
		*flag= onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
	}
}
unsigned int setOnuUpWaitingFlag(UCHAR slot,UCHAR port,UCHAR onuid, const UCHAR flag)
{
	if(slot > SYS_SLOTNUM
|| port > MAXPONPORT_PER_BOARD || onuid > MAXONUPERPONNOLIMIT|| flag > GPON_WAIT_UPDATE )
	{
		VOS_ASSERT(0);
		return ERROR;
	}
	if(slot < GPON_CLR_UPDATE
|| port < GPON_CLR_UPDATE || onuid < GPON_CLR_UPDATE|| flag < GPON_CLR_UPDATE )
	{
		VOS_ASSERT(0);
		return ERROR;
	}
	onu_oam_upd_waiting_list[slot][port][onuid] = flag;
	return ROK;
}
	
	/*******************************************************************************************
	函数名：setOnuUpdateFileTypeList
	功能：设定ONU升级文件类型列表
	输入：onuIdx:onu的设备索引，
	setType:要指定的文件类型，
	0-7;0不指定类型，1-7指定类型
	输出：无
	返回值：无
	*******************************************************************************************/
	
static int setOnuOamUpdFileTypeList( ULONG onuDevIdx, int setType  )
{
	ULONG PonPortIdx, OnuIdx;
	LOCATIONDES lct;
	USHORT *pVal;

	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		pVal = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		
		/*setOnuOamUpdFlag( onuIdx_, WAIT_UPDATE );*/
		*pVal = WAIT_UPDATE;
		
		if( setType >0 && setType < 14 )
		{
			*pVal |= 0x0001<<setType;
		}
		else if( parseOnuIndexFromDevIdx( onuDevIdx, &PonPortIdx, &OnuIdx ) != -1 )
		{
			CHECK_ONU_RANGE
				
			*pVal |= 0x0001<<IMAGE_TYPE_APP;
			if(OnuEponAppHasVoiceApp(PonPortIdx, OnuIdx) == ROK)
				*pVal |= 0x0001<<IMAGE_TYPE_VOIP;
			if(OnuFirmwareHasFpgaApp(PonPortIdx, OnuIdx) == ROK)
				*pVal |= 0x0001<<IMAGE_TYPE_FPGA;
		}
	}
	return VOS_OK;
}
	
	/*added by wangxy 2007-10-24
	清除onu升级文件类型，当全部类型被清除后清掉升级标识
	*/
int delOnuOamUpdFileTypeList( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	USHORT *pval;
	int i;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}

		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		for( i=1; i<14; i++ )
		{
			if( (*pval)&(0x0001<<i) )
			{
				(*pval) &= ~(0x0001<<i);
				break;
			}
		}
		
		if( ( (*pval) & 0x3ffe) == 0 )
			*pval = CLR_UPDATE;
		
		return i;
	}
	return 0;
}

static BOOL isEmptyUpdateFileTypeList( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	USHORT *pval;
	
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return TRUE;
		}

		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		
		if( (*pval)&0x3ffe )
		{
			return FALSE;
		}
		else
			return TRUE;
	}
	return TRUE;	
}

static int setOnuWsStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = -1;
	int i;
	USHORT *pval;
	
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return ret;
		}
		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		*pval |= 0x8000;
		
		for( i=0; i<MAX_UPDATE_ONU_NUM; i++ )
		{
			if( onu_oam_upd_aging_list[i].wsTimers == 0 )
			{
				ret = i;
				onu_oam_upd_aging_list[i].wsOnus = onuDevIdx;
				break;
			}
		}
	}
	return ret;
}
    
static int clsOnuWsStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = -1;
	int i;
	USHORT *pval;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return ret;
		}
		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		*pval &= 0x7fff;
		
		for( i=0; i<MAX_UPDATE_ONU_NUM; i++ )
		{
			if( onu_oam_upd_aging_list[i].wsOnus == onuDevIdx )
			{
				ret = i;
				onu_oam_upd_aging_list[i].wsOnus = 0;
				break;
			}
		}
	}
	return ret; 
}
static int isSetWsStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = 0;
	USHORT val;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return -1;
		}
		val = onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		if( (val&0x8000) == 0 )
			ret = -1;
	}
	else
		ret = -1;
	
	return ret;	
}
	
static int setOnuUpdatingStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = 0;
	USHORT *pval;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return -1;
		}
		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		*pval |= 0x4000;
	}
	else
		ret = -1;
	
	return ret;
}
	
static int clsOnuUpdatingStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = 0;
	USHORT *pval;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return -1;
		}
		pval = &onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		*pval &= 0xbfff;
	}
	else
		ret = -1;
	
	return ret;	
}

static int isSetUpdatingStatus( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = 0;
	USHORT val;
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return -1;
		}
		val = onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
		if( (val&0x4000) == 0 )
			ret = -1;
	}
	else
		ret = -1;
	
	return ret;	
}
	
USHORT getOnuRunFlag( ULONG onuDevIdx )
{
	LOCATIONDES lct;
	int ret = -1;
	
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return -1;
		}
		ret = onu_oam_upd_waiting_list[lct.slot][lct.port][lct.onuId];
	}	
	
	return ret;
}

CHAR *onuOamUpdFileType2Str( int nTypeBit )
{
	CHAR *pTypeStr = NULL;
	switch( nTypeBit )
	{
		case 1:
			pTypeStr = "voice image";
			break;
		case 2:
			pTypeStr = "fpga image";
			break;
		case 3:
			pTypeStr = "app image";
			break;
		default:
			pTypeStr = "UNKNOWN";
			break;
	}
	return pTypeStr;
}


/* modified by xieshl 20110212, 升级失败时给出提示，问题单11797 */
char * onuOamUpdResult2Str( int ret )
{
	char *pStr = NULL;
	switch( ret )
	{
		/*case ONU_UPDATE_OK:*/
		case ONU_UPDATE_ERR:
			pStr = "resource";
			break;
		case ONU_UPDATE_INPROCESS:
			pStr = "inprocessing";
			break;
		case ONU_VERSION_IDENTICAL:
			pStr = "same version";
			break;
		case ONU_UPDATE_FORBIDDED:
			pStr = "forbidden";
			break;
		default:
			pStr = "unknown";
			break;
	}
	return pStr;
}

int updateOneOnu( ULONG onuDevIdx, ULONG vendor_type, ULONG spec_type )
{
	int onuStatus, ret = ONU_UPDATE_ERR;
	/*short int ponIdx;*/
	LOCATIONDES lct;
	
	/*升级onu前，将onu的升级告警标志位清空，使之可以上报告警，add by luh @2016-6-1 for Q.24223*/
    SetOnuUpdatedAlarmStatus(onuDevIdx, 0);
	/*setOnuOamUpdFlag( onuDevIdx, CLR_UPDATE );*/
	getLocation( onuDevIdx, &lct, CONV_YES );
	/*检查当前ONU状态，如果不在线，放弃升级,发送Trap通知网管*/
	DeviceOperStatusGet( LOC2DevIdx(&lct), &onuStatus );
	/*ponIdx = GetPonPortIdxBySlot( lct.slot, lct.port );*/
	if( onuStatus != 1 )
	{
		/* reports( "ONU UPDATE:", "FAIL, ONU not online" );*/
		/*Trap_OnuAppImageUpdate( ponIdx, lct.onuId, V2R1_LOAD_FILE_FAILURE );*/
		ret = ONU_UPDATE_ERR;
	}
	else
	{
  		int filetype ;
		while ( isEmptyUpdateFileTypeList( onuDevIdx )!= TRUE )
		{
			filetype = delOnuOamUpdFileTypeList( onuDevIdx );

			/*waiting for OnuSwUpdateStart added the last parameter for 'filetype'*/
			ret = OnuSwUpdateStart( LOC2DevIdx( &lct ), vendor_type, spec_type, filetype );
			/* ret = ONU_UPDATE_OK;
			setOnuUpdatingStatus( onuDevIdx );*/
			if( ret == ONU_UPDATE_OK )
			{
			    	/*setOnuOamUpdFlag( onuDevIdx, CLR_UPDATE );*/
				setOnuUpdatingStatus( onuDevIdx );
                    
				/*IncCurUpdatingOnuNum();*/
                /*此处不应返回*/
				return ret;
			}
			else	/* modified by xieshl 20110212, 升级失败时给出提示，问题单11797 */
			{
				int onuEntry = parseOnuIndexFromDevIdx( onuDevIdx, 0, 0 );
				if( (onuEntry >= 0) && (onuEntry < MAXONU) )
				{
					/*ONU_MGMT_SEM_TAKE;*/
                    /*moved by luh 2013-3-15,问题单17286*/
					/*v2r1_printf(OnuMgmtTable[onuEntry].vty, "\r\n onu %d/%d/%d update err:%s\r\n", GET_PONSLOT(onuDevIdx), GET_PONPORT(onuDevIdx), GET_ONUID(onuDevIdx), onuOamUpdResult2Str(ret) );*/
					/*ONU_MGMT_SEM_GIVE;*/
				}
			}
		}
	}
	return ret;
}

ULONG findFirstWaitingOnu(void)
{
	int i,j,k;
	ULONG idx = 0;

	for( i=1; i<=SYS_SLOTNUM; i++)
	{
		if( SlotCardIsPonBoard(i) == RERROR )
			continue;
		for( j=0; j<=MAXPONPORT_PER_BOARD; j++)
		{
			for( k=0; k<=MAXONUPERPON; k++)
			{
				if( onu_oam_upd_waiting_list[i][j][k] & WAIT_UPDATE )
				{
					idx = MAKEDEVID( i, j, k );
					break;
				}
			}
		}
	}
	return idx;
}

ULONG findNextWaitingOnu( ULONG onuDevIdx )
{
	int i,j,k;
	LOCATIONDES lct;
	ULONG idx=0;
	
	if( getLocation( onuDevIdx, &lct, CONV_YES ) == VOS_OK )
	{
		if( (lct.slot > SYS_SLOTNUM) || (lct.port > MAXPONPORT_PER_BOARD) || (lct.onuId > MAXONUPERPON) )
		{
			VOS_ASSERT(0);
			return 0;
		}
        /*modi by luh 2012-3-8*/
		for( i=lct.slot, j=lct.port, k=lct.onuId+1; i<=SYS_SLOTNUM; i++ )
		{
			if( SlotCardIsPonBoard(i) == RERROR )
			{
				j = 0;
				k = 0;
				continue;
			}
			for( ; j<MAXPONPORT_PER_BOARD; j++ )
			{
				for( ; k<MAXONUPERPON; k++ )
				{
					if( onu_oam_upd_waiting_list[i][j][k] & 0x0001 )
					{
						idx = MAKEDEVID( i, j, k );
						return idx;
					}
				}
				k = 0;
			}
			j = 0;
		}
	}
	return idx;
}

LONG GetNumofOnuUpdatingStatusBySlot( SHORT olt_id )
{
	onu_updating_counter_t onulist;
	onulist.count = 0;
	if( OLT_GetOnuUpdatingStatusBySlot( olt_id, &onulist ) == OLT_ERR_OK )
	{
		return onulist.count;
	}
	return MAX_UPDATE_ONU_NUM;
}
LONG GetOnuUpdatingStatusBySlot( SHORT olt_id, SHORT onu_id )
{
	onu_updating_counter_t onulist;
	int i;
	if( OLT_GetOnuUpdatingStatusBySlot( olt_id, &onulist ) == OLT_ERR_OK )
	{
		for( i=0; i<MAX_UPDATE_ONU_NUM; i++ )
		{
			if( (onulist.ponid_list[i] == olt_id) && (onulist.onuid_list[i] == onu_id) )
				return ONU_SW_UPDATE_STATUS_INPROGRESS;
		}
	}
	return ONU_SW_UPDATE_STATUS_NOOP;
}

static void wsStatusTimeOutCallback( void* pData )
{
	ULONG onuDevIdx = *(ULONG*)pData;
	int idx = clsOnuWsStatus( onuDevIdx );

	if( idx >= 0 )
	{
		if( onu_oam_upd_aging_list[idx].wsTimers )
			VOS_TimerDelete( MODULE_PONUPDATE, onu_oam_upd_aging_list[idx].wsTimers );
		onu_oam_upd_aging_list[idx].wsTimers = 0;
	}
}

/*static void onuOamUpdTimerCallback()
{
	if( VOS_QueNum( onuOamUpdQueId ) > 3 )
		return;
	sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_TIMER, 0, 0);
}

int ONU_OAM_UPD_AGING_TIME = 6;
void onuOamUpdTimerProc()
{
	int i;
	for( i=0; i<MAX_UPDATE_ONU_NUM; i++ )
	{
		if( onu_oam_upd_aging_list[i].wsOnus == 0 )
			continue;

		if( onu_oam_upd_aging_list[i].wsTimers > ONU_OAM_UPD_AGING_TIME )
		{
			if( sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, MSGTYPE_UPDATE_OVER, onu_oam_upd_aging_list[i].wsOnus, IMAGE_TYPE_NONE) == VOS_OK )
			{
				onu_oam_upd_aging_list[i].wsOnus = 0;
				onu_oam_upd_aging_list[i].wsTimers = 0;
			}
		}
		else
			onu_oam_upd_aging_list[i].wsTimers++;
	}
}*/

LONG onuOamUpdAllOnuProc( ULONG spec_type, ULONG filetype )
{
	LOCATIONDES lct;
	ULONG onuIdx;
	ULONG ponId, onuId;
	int vendorType;

	while( 1 )
	{
		if( (onuIdx = findNextOnu(PONCARD_FIRST, 0, 0, ALL_ONU)) == 0 ) 
			break;
		if( parseOnuIndexFromDevIdx( onuIdx, &ponId, &onuId ) != -1 )
			break;
		
		if( (vendorType = GetOnuVendorType( ponId, onuId )) == VOS_ERROR )
			continue;
			
		if( getLocation( onuIdx, &lct, CONV_NO ) == VOS_ERROR )
			continue;
		
		/*setOnuOamUpdFlag( onuIdx, WAIT_UPDATE );*/
		setOnuOamUpdFileTypeList( onuIdx, filetype );
		if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
		{
			if( GetNumOfOnuUpdate() == 0)
			{
				if( updateOneOnu( onuIdx, vendorType, spec_type ) != VOS_OK )
				{
					setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
				}
			}
		}
		else
		{
			if( MAX_UPDATE_ONU_NUM > GetNumOfOnuUpdate() )
			{
				if( updateOneOnu( onuIdx, vendorType, spec_type ) != VOS_OK )
				{
					setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
				}
			}
		}
	}
	return VOS_OK;
}

LONG onuOamUpdOneOnuProc( ULONG onuIdx, ULONG spec_type, ULONG filetype )
{
	LOCATIONDES lct;
	ULONG ponId, onuId;
	int vendorType = 0;

	if( parseOnuIndexFromDevIdx( onuIdx, &ponId, &onuId ) != VOS_ERROR )
	{
		vendorType = GetOnuVendorType( ponId, onuId );
		if( getLocation( onuIdx, &lct, CONV_YES ) == VOS_OK )
		{
			setOnuOamUpdFileTypeList( onuIdx, filetype );

			if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
			{
				if( GetNumOfOnuUpdate() == 0)
				{
					if( updateOneOnu( onuIdx, vendorType, spec_type ) != VOS_OK )
					{
						setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
					}
				}
			}
			else
			{
				if( GetNumOfOnuUpdate() < MAX_UPDATE_ONU_NUM )
				{
					if( updateOneOnu( onuIdx, vendorType, spec_type ) != VOS_OK )
					{
						setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
					}
				}
			}
		}
	}
	return VOS_OK;
}
LONG UpdateOneGponOnu(ULONG DeviceId)
{
	unsigned int slot,port,waitingflag,updatingstats;
	ULONG ponid,NumOfUpdating,onuid;
	OLT_GPON_UPDATE_DEBUG("UpdateOneGponOnu,DeviceId= %d\r\n",DeviceId);
	if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
		return VOS_OK;

	NumOfUpdating = GetNumOfOnuUpdate();
	if(NumOfUpdating > MAX_UPDATE_GPON_ONU_NUM)
	{
		getOnuOamUpdFlag(DeviceId,&waitingflag);
		GetGponOnuSwUpdateStatus(ponid,onuid,&updatingstats);
		
		if((waitingflag != GPON_WAIT_UPDATE )&& (updatingstats != ONU_SW_UPDATE_STATUS_INPROGRESS))
		setOnuOamUpdFlag( DeviceId, GPON_WAIT_UPDATE );
		return VOS_OK;
	}
	/*升级onu前，将onu的升级告警标志位清空，使之可以上报告警，add by luh @2016-6-1 for Q.24223*/
    SetOnuUpdatedAlarmStatus(DeviceId, 0);
	
	OnuMgt_OnuSwUpdate(ponid,onuid,0,0);

	setOnuOamUpdFlag( DeviceId, GPON_CLR_UPDATE );
	SetGponOnuSwUpdateStatus(ponid,onuid,ONU_SW_UPDATE_STATUS_INPROGRESS);
	
	return VOS_OK;
}
int SetGponOnuUpdateflag(ULONG DeviceId)
{
	unsigned int slot,port;
	ULONG ponid,NumOfUpdating,onuid;
	OLT_GPON_UPDATE_DEBUG("SetGponOnuUpdateflag DeviceId= %d\r\n",DeviceId);
	if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
	{
		OLT_GPON_UPDATE_DEBUG("parseOnuIndexFromDevIdx error \r\n",ponid,onuid);
		return VOS_OK;
	}	
	SetGponOnuSwUpdateStatus(ponid,onuid,ONU_SW_UPDATE_STATUS_NOOP);
	return VOS_OK;
}
int SendGponUpdateMsg(ULONG DeviceId)
{
	ULONG ponid,onuid;
	onu_update_msg_t updMsg;
	OLT_GPON_UPDATE_DEBUG("SendGponUpdateMsg,Devidx = %d,ponid = %d , onuid = %d\r\n",DeviceId,ponid,onuid);
	if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
		return VOS_ERROR;
	OLT_GPON_UPDATE_DEBUG("SendGponUpdateMsg,Devidx = %d,ponid = %d , onuid = %d\r\n",DeviceId,ponid,onuid);
	VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
	updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
	updMsg.msg_type = MSGTYPE_UPDATEGPON_ONE;
	updMsg.onuDevIdx = DeviceId;
	updMsg.file_type = IMAGE_TYPE_ALL;
	if( OLT_SetOnuUpdateMsg( ponid, &updMsg ) != OLT_ERR_OK )
		sys_console_printf("perhaps update task is busy now!\r\n" );
}
int SendGponUpdateOverMsg(ULONG DeviceId)
{
	ULONG ponid,onuid;
	onu_update_msg_t updMsg;
	OLT_GPON_UPDATE_DEBUG("SendGponUpdateOverMsg,Devidx = %d\r\n",DeviceId);
	if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
		return VOS_ERROR;
	OLT_GPON_UPDATE_DEBUG("SendGponUpdateOverMsg,Devidx = %d,ponid = %d , onuid = %d\r\n",DeviceId,ponid,onuid);
	VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
	updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
	updMsg.msg_type = MSGTYPE_UPDATEGPON_OVER;
	updMsg.onuDevIdx = DeviceId;
	updMsg.file_type = IMAGE_TYPE_ALL;
	if( OLT_SetOnuUpdateMsg( ponid, &updMsg ) != OLT_ERR_OK )
		sys_console_printf("perhaps update task is busy now!\r\n" );
}
int SendActiveGponUpdateWiatingMsg(ULONG DeviceId)
{
	ULONG ponid,onuid;
	onu_update_msg_t updMsg;
	OLT_GPON_UPDATE_DEBUG("SendActiveGponUpdateWiatingMsg,Devidx = %d\r\n",DeviceId);
	if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
		return VOS_ERROR;
	OLT_GPON_UPDATE_DEBUG("SendActiveGponUpdateWiatingMsg,Devidx = %d,ponid = %d , onuid = %d\r\n",DeviceId,ponid,onuid);
	VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
	updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
	updMsg.msg_type = MSGTYPE_UPDATEGPONWAITING_ACTIVE;
	updMsg.onuDevIdx = DeviceId;
	updMsg.file_type = IMAGE_TYPE_ALL;
	if( OLT_SetOnuUpdateMsg( ponid, &updMsg ) != OLT_ERR_OK )
		sys_console_printf("perhaps update task is busy now!\r\n" );
	return VOS_OK;
}
int ActiveGponUpdatewaiting()
{
	ULONG DeviceId=0;
	ULONG NumOfUpdating=0,NeedActNum=0;
	unsigned int waitingflag;
	ULONG ponid,onuid,updatingstats;
	if( (DeviceId = findFirstWaitingOnu()) == 0 )
	{
		OLT_GPON_UPDATE_DEBUG("no waiting onu \r\n");
    	return VOS_OK;
	}
	NumOfUpdating = GetNumOfOnuUpdate();
	NeedActNum= (MAX_UPDATE_GPON_ONU_NUM+1)*2-NumOfUpdating;
	if(NumOfUpdating >MAX_UPDATE_GPON_ONU_NUM)
	{
		OLT_GPON_UPDATE_DEBUG("updating is full \r\n");
		return VOS_OK;
	}
	while( NeedActNum)
	{	
		OLT_GPON_UPDATE_DEBUG("while DeviceId = %d,NeedActNum = %d \r\n",DeviceId,NeedActNum);
		if( parseOnuIndexFromDevIdx( DeviceId, &ponid, &onuid ) == -1 )
			return VOS_OK;
		GetGponOnuSwUpdateStatus(ponid,onuid,&updatingstats);
		if(updatingstats == ONU_SW_UPDATE_STATUS_INPROGRESS)
		{
			continue;
		}
		SendGponUpdateMsg(DeviceId);
		NeedActNum--;
		DeviceId = findNextWaitingOnu(DeviceId);
		if(!DeviceId)
			break;
	}
	return VOS_OK;
}
LONG onuOamUpdOverProc( ULONG onuIdx, ULONG spec_type, ULONG filetype )
{
	ULONG ponId, onuId;
	int vendorType = 0;
	int idx;

	if( !isEmptyUpdateFileTypeList( onuIdx ) )
	{
		idx = setOnuWsStatus( onuIdx );
		if( idx >= 0 )	
			onu_oam_upd_aging_list[idx].wsTimers = VOS_TimerCreate( MODULE_PONUPDATE, 0, 60*1000, wsStatusTimeOutCallback, &onuIdx, VOS_TIMER_NO_LOOP );
	}
    
    
	while( 1 )
	{
        if( (onuIdx = findFirstWaitingOnu()) == 0 )
    		return VOS_OK;
		if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
		{
			if( GetNumOfOnuUpdate() != 0 )
				break;
		}
		else
		{
			if( GetNumOfOnuUpdate() >= MAX_UPDATE_ONU_NUM )
			break;
		}
		
		if(  isSetWsStatus( onuIdx ) == -1 &&  isSetUpdatingStatus(onuIdx) == -1 )
		{
			if( parseOnuIndexFromDevIdx( onuIdx, &ponId, &onuId ) != -1 )
			{
				vendorType = GetOnuVendorType( ponId, onuId );
				if( updateOneOnu( onuIdx, vendorType, spec_type ) == VOS_OK )
				{
					break;
				}
			}
			setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
		}
		else
		{
			/*sys_console_printf("\r\n %% onu %d update status(0x%x) ERR\r\n", onuIdx, getOnuRunFlag(onuIdx));*/
    		/*if( (onuIdx = findNextWaitingOnu(onuIdx)) == 0 )*/
                break;
			/*setOnuOamUpdFlag( onuIdx, CLR_UPDATE );*/
		}
        
	}
	return VOS_OK;
}

LONG onuOamUpdSaveProc( ULONG onuIdx, ULONG spec_type, ULONG filetype )
{
	ULONG ponId, onuId;
	int vendorType = 0;
	int idx;

	idx = clsOnuWsStatus( onuIdx );
	if( idx >= 0 )
	{
		if( onu_oam_upd_aging_list[idx].wsTimers )
			VOS_TimerDelete( MODULE_PONUPDATE, onu_oam_upd_aging_list[idx].wsTimers );
		onu_oam_upd_aging_list[idx].wsTimers = 0;
	}

	if( !isEmptyUpdateFileTypeList( onuIdx ) )
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_10G_EPON)
		{
			if( GetNumOfOnuUpdate() == 0  )
			{
				if( parseOnuIndexFromDevIdx( onuIdx, &ponId, &onuId ) != -1 &&
					(vendorType = GetOnuVendorType( ponId, onuId ) )!= -1   )
				{
					updateOneOnu( onuIdx, vendorType, spec_type );
				}
			}
		}
		else
		{
			if( GetNumOfOnuUpdate() < MAX_UPDATE_ONU_NUM  )
			{
				if( parseOnuIndexFromDevIdx( onuIdx, &ponId, &onuId ) != -1 &&
					(vendorType = GetOnuVendorType( ponId, onuId ) )!= -1   )
				{
					updateOneOnu( onuIdx, vendorType, spec_type );
				}
			}
		}
	}
	return VOS_OK;
}

/*消息数据，
				   msgItem[0]: sender module ID
				   msgItem[1]: message ID
				   msgItem[2]: update range flag, 0-7bits:    0: specified onu,1: all online onus,2:some one updated compeletly.
				   8-15bits:    specified file type; 0:default, value: identified type
				   16-31bits:    specified switch image type: 0:not switch, 1: ctc-gw 2:gw-ctc
				   msgItem[3]: onuDevIdx when msgItem[2]==0, otherwise, it is always being 0
*/	 
void onuOamUpdTask()
{
	LONG ret;
	ULONG msgItem[4];
	ULONG spec_type = 0, act = 0, filetype=0;
	ULONG onuIdx;
 
	while( 1 )
	{		
		ret = VOS_QueReceive( onuOamUpdQueId, msgItem, 200 );
		if( ret == VOS_ERROR )
			continue;

		spec_type = msgItem[2]>>16;		
		act = msgItem[2]&0xff;
		filetype = (msgItem[2]>>8)&0xff;
		onuIdx = msgItem[3];

		OLT_GPON_DEBUG("onuOamUpdTask,deviceid = %d \r\n",onuIdx);
		switch( act )
		{
			case  MSGTYPE_UPDATE_TIMER:
				/*onuOamUpdTimerProc();*/
				break;
			case MSGTYPE_UPDATE_ALL:	/*升级全部ONU*/
				onuOamUpdAllOnuProc( spec_type, filetype );
				break;
			case MSGTYPE_UPDATE_ONE:		/*升级一个ONU*/
				onuOamUpdOneOnuProc( onuIdx, spec_type, filetype );
				break;
			case MSGTYPE_UPDATEGPON_ONE:		/*升级一个GPON ONU*/
				UpdateOneGponOnu(onuIdx);
				break;
			case MSGTYPE_UPDATEGPON_OVER:
				SetGponOnuUpdateflag(onuIdx);
				break;
			case MSGTYPE_UPDATEGPONWAITING_ACTIVE:
				ActiveGponUpdatewaiting();
				break;
			case MSGTYPE_UPDATE_FAIL:		/*提示一个ONU升级完毕*/
			case MSGTYPE_UPDATE_OVER:
				if( clsOnuUpdatingStatus( onuIdx ) == -1 )
					break;
				if( act == MSGTYPE_UPDATE_FAIL )
					setOnuOamUpdFlag( onuIdx, CLR_UPDATE );
				
				onuOamUpdOverProc( onuIdx, spec_type, filetype );
				break;
			case MSGTYPE_UPDATE_SAVE_OK:
				onuOamUpdSaveProc( onuIdx, spec_type, filetype );
				break;
			default:
				break;
		}
	}
}

STATUS onuOamUpdateInit( void )
{
	STATUS rc = VOS_ERROR;
	
	VOS_MemZero( onu_oam_upd_waiting_list, sizeof(onu_oam_upd_waiting_list) );
	VOS_MemZero( onu_oam_upd_aging_list, sizeof(onu_oam_upd_aging_list) );

	VOS_MemZero( s_ctcOnuNeedCommit, sizeof(s_ctcOnuNeedCommit));

	onuOamUpdQueId = VOS_QueCreate( MAX_UPDATE_MSG_NUM, VOS_MSG_Q_FIFO );
	if( onuOamUpdQueId == 0 )
	{
		sys_console_printf( "\r\nCreate queue for onuOamUpdTask fail!" );
		return rc;
	}
	
	onuOamUpdTaskId = VOS_TaskCreate( ONU_UPDATE_TASK, TASK_PRIORITY_NORMAL, onuOamUpdTask, NULL );
	if( onuOamUpdTaskId != NULL )
		rc = VOS_OK;

	VOS_QueBindTask( onuOamUpdTaskId, onuOamUpdQueId );

	if( VOS_OK != CDP_Create( RPU_TID_CDP_OAM_UPD, CDP_NOTI_VIA_FUNC, 0, onuOamUpd_CDP_Callback) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	/*if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		onuOamUpdTimerId = VOS_TimerCreate( MODULE_PONUPDATE, 0, 10000, onuOamUpdTimerCallback, NULL, VOS_TIMER_LOOP );*/

	onuOamUpdCommandInstall();
	
	return rc;
}

LONG sendOnuOamUpdMsg( int update_mode, ULONG act, ULONG para, ULONG fileType )
{
	ULONG msgItem[4];
	msgItem[0]=MODULE_PONUPDATE;
	msgItem[1]=0;
	msgItem[2]= (update_mode<<16) |act;
	/*added by wangxy 2007-10-25
	增加升级文件类型
	*/
	msgItem[2] |= ((fileType << 8) & 0xff00);
	msgItem[3]=para;
	
	return VOS_QueSend( onuOamUpdQueId, msgItem, 200, MSG_PRI_NORMAL);
}

VOID  onuOamUpd_CDP_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID *pData, ULONG ulDataLen)
{
	SYS_MSG_S *pstMsg = NULL;
	onu_oam_upd_msg_t *pOamUpdMsg;
	ULONG PonPortIdx = 0, OnuIdx = 0;
	
	if(ulDstChId != RPU_TID_CDP_OAM_UPD || ulChID != RPU_TID_CDP_OAM_UPD)
	{
		CDP_FreeMsg(pData);
		return ;
	}

	pstMsg = (SYS_MSG_S *)pData ;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA:
			if( pstMsg == NULL )
			{
				VOS_ASSERT( 0 );
				return ;
			}
			if( SYS_MSG_MSG_CODE(pstMsg) == OAM_UPD_MSG_CODE_CDP_RESULT_DISP )
			{
				pOamUpdMsg = (onu_oam_upd_msg_t *)(pstMsg + 1);
				onuOamUpdPrintf( GET_PONSLOT(pOamUpdMsg->onuDevIdx), GET_PONPORT(pOamUpdMsg->onuDevIdx), GET_ONUID(pOamUpdMsg->onuDevIdx), pOamUpdMsg->data);
			}
			else if( SYS_MSG_MSG_CODE(pstMsg) == OAM_UPD_MSG_CODE_CDP_STATUS_REP )
			{
				pOamUpdMsg = (onu_oam_upd_msg_t *)(pstMsg + 1);
				if( parseOnuIndexFromDevIdx(pOamUpdMsg->onuDevIdx, &PonPortIdx, &OnuIdx) != -1 )
				{
					SetOnuSwUpdateStatus(PonPortIdx, OnuIdx, pOamUpdMsg->status, pOamUpdMsg->file_type );
				}
			}
			CDP_FreeMsg(pstMsg);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pstMsg);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pstMsg);
			break;
	}

	return ;
}

LONG onuOamUpdMsg2Master( onu_oam_upd_msg_t *pOamUpdMsg, ULONG msgCode )
{
	int rc;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(onu_oam_upd_msg_t);
	SYS_MSG_S   *pstMsg;

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		return VOS_OK;

	if( (NULL == pOamUpdMsg) || (0 == pOamUpdMsg->onuDevIdx) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	pstMsg = (SYS_MSG_S *)CDP_AllocMsg( msgLen, MODULE_ONU );
	if(pstMsg == NULL)
	{
		VOS_ASSERT(pstMsg);
		return VOS_ERROR;
	}

	pstMsg->ulSrcModuleID = MODULE_ONU;
	pstMsg->ulDstModuleID = MODULE_ONU;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_MASTER_ACTIVE_SLOTNO;
	pstMsg->ucMsgType = MSG_NOTIFY;
	pstMsg->usMsgCode = msgCode;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = (pstMsg + 1);
	pstMsg->usFrameLen = sizeof(onu_oam_upd_msg_t);

	VOS_MemCpy( pstMsg->ptrMsgBody, pOamUpdMsg, pstMsg->usFrameLen );
	
	rc = CDP_Send( RPU_TID_CDP_OAM_UPD, SYS_MASTER_ACTIVE_SLOTNO,  RPU_TID_CDP_OAM_UPD, CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_ONU );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (void *) pstMsg );
	}	
	return rc;
}

LONG onuOamUpdStatusReport( short int PonPortIdx, short int OnuIdx, int status, unsigned int fileType)
{
	LONG rc = VOS_ERROR;
	if( SYS_LOCAL_MODULE_WORKMODE_ISSLAVE )
	{
		onu_oam_upd_msg_t oamUpdMsg;
		ULONG onuDevIdx = parseDevidxFromPonOnu( PonPortIdx, OnuIdx );
		if( onuDevIdx )
		{
			oamUpdMsg.onuDevIdx = onuDevIdx;
			oamUpdMsg.mode = 0;
			oamUpdMsg.act = 0;
			oamUpdMsg.file_type = fileType;
			oamUpdMsg.status = status;
			oamUpdMsg.result = 0;
			rc = onuOamUpdMsg2Master( &oamUpdMsg, OAM_UPD_MSG_CODE_CDP_STATUS_REP );
		}
	}
	return rc;
}


CHAR *onu_oam_upg_file_type_2_str( ULONG fileType)
{
	CHAR *str = OnuImageType[0];
	switch(fileType)
	{
		case IMAGE_TYPE_ALL:
			str = "all";
			break;
		case IMAGE_TYPE_NONE:
			str = "null";
			break;
		case IMAGE_TYPE_APP:
		case IMAGE_TYPE_VOIP:
		case IMAGE_TYPE_FPGA:
			str = OnuImageType[fileType];
			break;
		default:
			break;
	}
	return str;
}

LONG onuOamUpdPrintf( ULONG slot, ULONG port, ULONG onu, CHAR *str )
{
	if( str == NULL )
		str = "null";
	
	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		struct vty *vty = NULL;
		PON_olt_id_t PonPortIdx = GetPonPortIdxBySlot(slot, port);
		PON_onu_id_t OnuIdx = onu - 1;

		CHECK_ONU_RANGE;
			
		ONU_MGMT_SEM_TAKE;
		vty = OnuMgmtTable[PonPortIdx*MAXONUPERPON + OnuIdx].vty ;
		if( cl_vty_valid( vty ) == VOS_ERROR )
		{
			vty = NULL;	
		}
		ONU_MGMT_SEM_GIVE;
		v2r1_printf(vty, "\r\n onu %d/%d/%d %s\r\n", slot, port, onu, str );
	}
	else
	{
		onu_oam_upd_msg_t oamUpdMsg;
		VOS_MemZero( &oamUpdMsg, sizeof(oamUpdMsg) );
		oamUpdMsg.onuDevIdx = MAKEDEVID(slot, port, onu);
		oamUpdMsg.mode = 0;
		oamUpdMsg.act = 0;
		oamUpdMsg.file_type = 0;
		oamUpdMsg.status = 0;
		oamUpdMsg.result = 0;
		VOS_StrnCpy( oamUpdMsg.data, str, 120 );
		onuOamUpdMsg2Master( &oamUpdMsg, OAM_UPD_MSG_CODE_CDP_RESULT_DISP );

		sys_console_printf( "\r\n onu %d/%d/%d %s\r\n", slot, port, onu, str );
	}
	return VOS_OK;
}
	
static BOOL validDateAndTime( onuupdateclk_t *dt )
{
    BOOL ret = FALSE;
    short int year = 0;
	signed char month=0, day=0, hour=0, minute=0, second=0;
	
	year = ( dt->st_clk.year < 0 )?2000+32+dt->st_clk.year:dt->st_clk.year+2000;
	month = (dt->st_clk.month<0)?16+dt->st_clk.month:dt->st_clk.month;
	day = (dt->st_clk.day<0)?32+dt->st_clk.day:dt->st_clk.day;
	
	hour = (dt->st_clk.hour<0 )?32+dt->st_clk.hour:dt->st_clk.hour;
	minute=(dt->st_clk.minute<0)?64+dt->st_clk.minute:dt->st_clk.minute;
	second=(dt->st_clk.second<0)?64+dt->st_clk.second:dt->st_clk.second;
	
    if( year>=2000 && year<=2031 &&
		month>=1 && month <= 12 &&
		day >= 1 && day <= 31 &&
		hour >= 0 && hour <= 23 &&
		minute>=0 && minute<= 59 &&
		second>=0 && second <= 59 )
		ret = TRUE;
	
	return ret;
	
}

/* modified by xieshl 20110111, 问题单11796 */
int getOnuOamUpdatedStatus( ULONG ponidx, onuUpdateStatusList_t *pbuf )
{
	int onuswupdatest, i, lenver, lenApptype;
	
	int onutype = 0;
	char onuswver[MAXSWVERSIONLEN] = "";
	char flashver[256] = "";
	char onuApptype[COMMNAME_LEN] = "";
	ULONG slot = 0, port=0;

	/*PonPortItem *pPonPortItem;
	const LogicEntity *pEntry;*/
	onuUpdateStatusList_t *pUpdList = pbuf;
	onuUpdateStatus_t *pUpdStatBuf;
	onuupdateclk_t updaterec;

    
	/*ponIdx 为非法值，退出*/
	if( ponIndex2slot( ponidx, &slot, &port ) == VOS_ERROR )
		return 0;
	
	if( pUpdList == NULL )
		return 0;

	/* 增加槽位/ 端口检查 chenfj 2009-5-18 */
	if(PonCardSlotPortCheckByVty(slot, port, NULL)  != VOS_OK) return VOS_ERROR;

	/*if ( NULL == (pPonPortItem = getPonPortItemBySlot(slot, port)) ) return VOS_ERROR;*/

	pUpdList->num = 0;
	for( i=0; i<MAX_ONU_PER_PONPORT; i++ )
	{
		/*pEntry = &pPonPortItem->logicEntity[i];*/

		/*不存在的ONU不处理*/
		/*if( pEntry->exist == ONU_NOT_EXIST )*/
		if( GetOnuOperStatus(ponidx, i) != ONU_OPER_STATUS_UP )
			continue;

		/*if( GetOnuType( ponidx, i, &onutype ) == RERROR )
			continue;*/
		if( GetOnuAppTypeString( ponidx, i, onuApptype, &lenApptype) == RERROR )
			continue;
		if( GetOnuSWVersion( ponidx, i, onuswver, &lenver ) == RERROR )
			continue;
		
		/* added by chenfj 2007-10-30 */
		if( OnuEponAppHasVoiceApp(ponidx,i) == ROK )
		{
			unsigned char *Ptr = NULL;
			unsigned char *Ptr1 = &onuswver[0];
			Ptr = VOS_StrStr( onuswver, "/");
			if( Ptr != NULL )
			{
				onuswver[Ptr - Ptr1] ='\0';
				/*lenver = VOS_StrLen(onuswver);*/
			}
		}
		
		onuswupdatest = GetOnuSWUpdateStatus( ponidx, i );
		
		/* modified by chenfj 2007-12-21
		命令show onu updating information存在bug: 若某个ONU的APP没有出现在
		FLASH中，那么会显示上一个出现在FLASH中的ONU APP版本信息
		*/
		flashver[0] = '\0';
		if( get_ONU_Info( onuApptype, NULL, NULL, NULL, NULL, (int*)flashver ) == VOS_OK )
		{
			/*当前正在升级，不统计*/
			if( onuswupdatest == ONU_SW_UPDATE_STATUS_INPROGRESS )
				continue;

			ONU_MGMT_SEM_TAKE;
			updaterec.clk = OnuMgmtTable[ponidx*MAX_ONU_PER_PONPORT+i].updaterec.clk;
			ONU_MGMT_SEM_GIVE;
	
			pUpdStatBuf = &(pUpdList->updStat[(int)pUpdList->num]);
			pUpdStatBuf->onuid = i;
			pUpdStatBuf->type = onutype;
			VOS_StrCpy( pUpdStatBuf->version, flashver/*, COMMNAME_LEN*/ );
			VOS_StrCpy( pUpdStatBuf->swversion, onuswver/*, COMMNAME_LEN*/ );
			
			if( onu_oam_upd_waiting_list[slot][port][i+1] & WAIT_UPDATE )
			{
				/*VOS_StrCpy( p->result, "waiting" );*/
				pUpdStatBuf->result_type = ONU_UPD_RESULT_TYPE_WAITING;
				pUpdStatBuf->result_data = 0;
			}
			else if( validDateAndTime( &updaterec ) == 0 )
			{
				/*VOS_StrCpy( p->result, "unknown" );*/
				pUpdStatBuf->result_type = ONU_UPD_RESULT_TYPE_UNKNOWN;
				pUpdStatBuf->result_data = 0;
			}
			else
			{
				pUpdStatBuf->result_type = ONU_UPD_RESULT_TYPE_TIME;
				pUpdStatBuf->result_data = updaterec.clk;
			}
			pUpdList->num++;
		}
	}
	return pUpdList->num;
}

int getOnuOamUpdatingStatus( ULONG ponidx, onuUpdateStatusList_t *pbuf )
{
	int onuswupdatest = 0, i, onuEntry;
	int onutype = 0;
	char filetypestr[COMMNAME_LEN];
	char onuswver[32];
	char onuver[256];
	
	int typelen = 0;
	int fileType;
	
	float filelen;
	float transfilelen;
	int rate, onuswverlen;

	onuUpdateStatusList_t *pUpdList = pbuf;
	onuUpdateStatus_t *pUpdStatBuf;
		
	if( pbuf == NULL )
		return 0;

	pUpdList->num = 0;
	for( i=0; i<MAX_ONU_PER_PONPORT; i++ )
	{
		if( GetOnuType( ponidx, i, &onutype ) == RERROR )
			continue;
		
		onuswupdatest = GetOnuSWUpdateStatus( ponidx, i );
		if( onuswupdatest != ONU_SW_UPDATE_STATUS_INPROGRESS )
			continue;
		
		/********************** start *******************************
		2007-11-21 modified by chenfj
		显示ONU 版本信息时，判断当前ONU 正在升级的文件类型
		*/
		fileType = GetOnuSwUpdateType(ponidx, i );
		if(( fileType != IMAGE_TYPE_APP) && ( fileType != IMAGE_TYPE_VOIP) && ( fileType != IMAGE_TYPE_FPGA) )
			continue;

		if( fileType == IMAGE_TYPE_APP )
		{
			if( GetOnuAppTypeString( ponidx, i, filetypestr, &typelen ) == RERROR )
				continue;
		}
		else if( fileType == IMAGE_TYPE_VOIP )
		{
			if( GetOnuVoiceTypeString( ponidx, i, filetypestr, &typelen ) == RERROR )
				continue;
		}
		else if( fileType == IMAGE_TYPE_FPGA)
		{
			if( GetOnuFPGATypeString( ponidx, i, filetypestr, &typelen ) == RERROR )
				continue;
		}
		/*
	        if( GetOnuTypeString( ponidx, i, onutype, &lentype ) == RERROR )
				continue;
		*/
		onuswverlen = 0;
		if( (fileType == IMAGE_TYPE_APP ) || ( fileType == IMAGE_TYPE_VOIP ))
		{	
			if( GetOnuSWVersion( ponidx, i, onuswver, &onuswverlen ) == RERROR )
				continue;
			/* added by chenfj 2007-10-30 */
			if( OnuEponAppHasVoiceApp(ponidx,i) == ROK )
			{
				unsigned char *Ptr = NULL;
				unsigned char *Ptr1 = &onuswver[0];
				int len = 0;
				Ptr = VOS_StrStr( onuswver, "/");
				if( Ptr != NULL )
				{
					if(fileType == IMAGE_TYPE_APP )
					{
						onuswver[Ptr - Ptr1] ='\0';
						onuswverlen = VOS_StrLen(onuswver);
					}
					else {
						Ptr++;
						len = VOS_StrLen( Ptr);					
						VOS_MemCpy(Ptr1, Ptr, len);
						Ptr1[len] = '\0';
						onuswverlen = len;
					}
				}
			}
		}
		else if( fileType == IMAGE_TYPE_FPGA )
		{
			if(GetOnuFWVersion( ponidx, i, onuswver, &onuswverlen ) == RERROR )
				continue;
			if(OnuFirmwareHasFpgaApp(ponidx,i) == ROK )
			{
				unsigned char *Ptr = NULL;
				unsigned char *Ptr1 = &onuswver[0];
				int len = 0;
				Ptr = VOS_StrStr( onuswver, "/");
				if( Ptr != NULL )
				{		
					Ptr++;
					len = VOS_StrLen( Ptr);					
					VOS_MemCpy(Ptr1, Ptr, len);
					Ptr1[len] = '\0';
					onuswverlen = len;					
				}
				else {
					onuswver[0] = '\0';
					onuswverlen = 0;
				}
			}
		}
		/************** end *******************/	
	 	onuver[0] = '\0';
        	if( get_ONU_Info( filetypestr, NULL, NULL, NULL, NULL, (int*)onuver ) == VOS_ERROR )
			continue;

		onuEntry = ponidx*MAX_ONU_PER_PONPORT+i;
		ONU_MGMT_SEM_TAKE;
        	filelen = OnuMgmtTable[onuEntry].swFileLen;
		transfilelen = OnuMgmtTable[onuEntry].transFileLen;
		ONU_MGMT_SEM_GIVE;
		
		if ( (transfilelen<filelen) || ((transfilelen == filelen) && (filelen != 0)))
			rate = transfilelen/filelen*100;
		else
			rate = 0;

		pUpdStatBuf = &(pUpdList->updStat[(int)pUpdList->num]);
		pUpdStatBuf->onuid = i;
		/*VOS_Sprintf( pbuf[sum].result, "%d%%", rate );*/
		pUpdStatBuf->result_type = ONU_UPD_RESULT_TYPE_RATE;
		pUpdStatBuf->result_data = rate;
		pUpdStatBuf->type = onutype;
		VOS_StrCpy( pUpdStatBuf->version, onuver/*, COMMNAME_LEN*/ );
		VOS_StrCpy( pUpdStatBuf->swversion, onuswver/*, COMMNAME_LEN*/ );
		pUpdList->num++;
	}
	
	return pUpdList->num;
}

int  OnuSwUpdateStart( DeviceIndex_S  DevIdx, ULONG vendorType, ULONG spec_type, ULONG fileType )
{
	short int  OnuIdx;
	short int PonPortIdx;
	int  ret;

	if(( (DevIdx.slot == 0 ) && ( DevIdx.port == 0 ) && ( DevIdx.onuId == 0) )) return (RERROR);

	if( fileType > IMAGE_TYPE_APP )		/* modified by xieshl 20091216, 增加文件类型范围限制 */
	{
		/*sys_console_printf("\r\nonu %d/%d/%d update file type error\r\n", DevIdx.slot, DevIdx.port, DevIdx.onuId );*/
		onuOamUpdPrintf( DevIdx.slot, DevIdx.port, DevIdx.onuId, "update file type error" );
		return RERROR;
	}
	
	OnuIdx = (unsigned short int )DevIdx.onuId - 1;
	
	PonPortIdx = GetPonPortIdxBySlot( DevIdx.slot,  DevIdx.port);
	CHECK_ONU_RANGE
	/*
	if( GetOnuSWUpdateStatus(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_STATUS_FORBIDDEN )
		return( ONU_UPDATE_FORBIDDED );
	*/
	if( spec_type == IMAGE_UPDATE_MODE_INVALID )
	{
		if( vendorType == GW_ONU )
		{
			if( EVENT_UPDATE_ONU_FILE == V2R1_ENABLE )
			{
				sys_console_printf("\r\nonu %d/%d/%d %s file is to be updated\r\n", DevIdx.slot, DevIdx.port, DevIdx.onuId, onu_oam_upg_file_type_2_str(fileType) );
			}
			ret = StartOnuSWUpdate(PonPortIdx, OnuIdx, V2R1_NO_WAIT_RETURN, fileType);
		}
		else if( vendorType == CTC_ONU )
		{
			if( EVENT_UPDATE_ONU_FILE == V2R1_ENABLE )
			{
				sys_console_printf("\r\nctc onu %d/%d/%d %s file is to be updated\r\n", DevIdx.slot, DevIdx.port, DevIdx.onuId, onu_oam_upg_file_type_2_str(fileType) );
			}
			ret = StartOnuSWUpdate_1( PonPortIdx, OnuIdx );
		}
		else
		{
			if( EVENT_UPDATE_ONU_FILE == V2R1_ENABLE )
			{
				sys_console_printf("\r\nonu %d/%d/%d updated file(%s) error\r\n", DevIdx.slot, DevIdx.port, DevIdx.onuId, onu_oam_upg_file_type_2_str(fileType) );
			}
			ret = ONU_UPDATE_ERR ;
		}
	}
	else
	{
		if( EVENT_UPDATE_ONU_FILE == V2R1_ENABLE )
		{
			sys_console_printf("\r\nonu %d/%d/%d %s file is to be updated2\r\n", DevIdx.slot, DevIdx.port, DevIdx.onuId, onu_oam_upg_file_type_2_str(fileType) );
		}
		ret= StartOnuSWUpdate_2( PonPortIdx, OnuIdx, spec_type );	/* 问题单11878，暂时只在PON板上实现 */
		/*ret=OnuMgt_OnuGwCtcSwConvert(PonPortIdx, OnuIdx, spec_type);*//*6900 CTC 和GW 软件的互升*/
	}
	/*  ONU自动升级任务中已有对错误的判断处理，故此处的消息被注释
	if(ret != ONU_UPDATE_OK)
		sendOnuUpdateMsg( MSG_NO_SPEC_ONU_TYPE, MSGTYPE_UPDATE_FAIL, parseDevidxFromPonOnu( PonPortIdx, OnuIdx ), 0);
	*/
	return( ret );
	
}

int  StartOnuSWUpdate(short int PonPortIdx, short int OnuIdx , int Blockflag , unsigned int fileType )
{
	int slotId, port;
	int ret = ONU_UPDATE_OK;
#if 0   
	char onuTypeString[ONU_TYPE_LEN+2]= {'\0'};
	char OnuCurVersion[ONU_VERSION_LEN +1]= {'\0'};
	char OnuFlashVersion[ONU_VERSION_LEN+1] ={'\0'};
	int typeLen, VersionLen;
	int offset=0, file_len=0, compress_flag=0, file_fact_len=0;
	/*int offset1=0, file_len1=0, compress_flag1=0, file_fact_len1=0; */
	/*int OnuType;*/
#endif
	struct vty *vty;
	
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

	if((fileType != IMAGE_TYPE_APP ) &&(fileType != IMAGE_TYPE_VOIP ) && (fileType != IMAGE_TYPE_FPGA))
	{
		/*v2r1_printf(vty, "\r\n  input Onu file image type err\r\n");*/
		onuOamUpdPrintf( slotId, port, (OnuIdx+1), "input file type err" );
		ret = ONU_UPDATE_ERR;
	}
	else if( (Blockflag != V2R1_NO_WAIT_RETURN ) &&( Blockflag != V2R1_WAIT_RETURN ))
	{
		/*v2r1_printf(vty, "\r\n  Update onu %d/%d/%d %s Image parameter err \r\n", slotId, port, (OnuIdx+1), OnuImageType[fileType] );*/
		onuOamUpdPrintf( slotId, port, (OnuIdx+1), "block flag err" );
		ret = ONU_UPDATE_ERR;
	}
	/* onu 升级条件检查，包括ONU是否在线，当前是否开始升级，及ONU升级使能*/
	else if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_DISABLE )
	{
		/*v2r1_printf(vty, "\r\n  onu %d/%d/%d software update is disabled, so can't update this onu\r\n",  slotId, port, (OnuIdx+1) );*/
		onuOamUpdPrintf( slotId, port, (OnuIdx+1), "software update is disabled, so can't update this onu" );
		ret = ONU_UPDATE_ERR;
	}
	else if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		/*v2r1_printf( vty, "\r\n  onu %d/%d/%d is off-line \r\n", slotId, port, (OnuIdx+1) );*/
		onuOamUpdPrintf( slotId, port, (OnuIdx+1), "is off-line" );
		ret = ONU_UPDATE_ERR;
	}
	if( ret == ONU_UPDATE_ERR )
	{
		return ret;
	}

	ret = OnuMgt_OnuSwUpdate(PonPortIdx,OnuIdx, Blockflag, fileType);
	if ( OLT_CALL_ISOK(ret) )
	{
		/* modified by xieshl 20110316, 问题单12075 */
		CHAR str[64];
		VOS_Sprintf( str, "%s image upgrading......", onu_oam_upg_file_type_2_str(fileType) );
		onuOamUpdPrintf( slotId, port, (OnuIdx+1), str );
	}
	else
	{
		switch ( ret = OLT_ERR_UPGRADE_BEGIN - ret )
		{
			case ONU_UPDATE_INPROCESS:
				/*v2r1_printf( vty, "\r\n  onu %d/%d/%d image is updating\r\n",  slotId, port, (OnuIdx+1) );*/
				onuOamUpdPrintf( slotId, port, (OnuIdx+1), "image is updating" );
				break;
			case ONU_FILE_NOEXIST:
				/*v2r1_printf(vty, "\r\n  there is no %s image for onu%d/%d/%d in flash\r\n", OnuImageType[fileType], slotId, port, (OnuIdx+1) );*/
				onuOamUpdPrintf( slotId, port, (OnuIdx+1), "no image file in flash" );
				break;
			case ONU_VERSION_IDENTICAL:
				/*v2r1_printf(vty, "\r\n  onu %d/%d/%d %s Image is the newest version now\r\n",  slotId, port, (OnuIdx+1), OnuImageType[fileType] );*/
				onuOamUpdPrintf( slotId, port, (OnuIdx+1), "is the newest version now" );
				break;
			default:
        			/*v2r1_printf(vty, "\r\n  onu %d/%d/%d is failed to update\r\n",  slotId, port, (OnuIdx+1) );*/
				onuOamUpdPrintf( slotId, port, (OnuIdx+1), "is failed to update" );
				break;
		}
	}

	return ret;
}


int setCtcOnuCommitFlag(short int ponid, short int onuid)
{
    int ret = VOS_ERROR;

    if(ponid >= 0 && ponid < MAXPON &&
            onuid >= 0 && onuid < MAXONUPERPON)
    {
        s_ctcOnuNeedCommit[ponid][onuid] = 1;
        ret = VOS_OK;
    }

    return ret;
}

int clearCtcOnuCommitFlag(short int ponid, short int onuid)
{
    int ret = VOS_ERROR;

    if(ponid >= 0 && ponid < MAXPON &&
            onuid >= 0 && onuid < MAXONUPERPON)
    {
        s_ctcOnuNeedCommit[ponid][onuid] = 0;
        ret = VOS_OK;
    }

    return ret;
}

int getCtcOnuCommitFlag(short int ponid, short int onuid)
{

    int ret = 0;

    if(ponid >= 0 && ponid < MAXPON &&
            onuid >= 0 && onuid < MAXONUPERPON)
    {
        ret = s_ctcOnuNeedCommit[ponid][onuid];
    }

    return ret;
}

/*added by liub 2017-6-8, 检查升级文件类型和onu类型是否匹配*/
int gpon_update_file_check(short int ponid, short int onuid)
{
	int nameLen=0;
	unsigned char  onuTypeString[MAXDEVICENAMELEN+1];
	int ret = VOS_ERROR;

	int size = 2048, i=0;
	UCHAR file[2048]="";
	app_desc_t *pdesc = NULL;
	extern CHAR GPON_ONUAPP_FILE_HAED[];

	if(GetOnuTypeString(ponid, onuid, onuTypeString, &nameLen)!=VOS_OK)
		return VOS_ERROR;

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
	{
		pdesc = (app_desc_t*)GPON_ONUAPP_FILE_HAED; 
	}
	else
	{	
		VOS_MemSet(file, 0, size );
		xflash_file_read( ONU_FLASH_FILE_ID, file, &size );

		pdesc = (app_desc_t*)file;
	}

	if( VOS_StrCmp( onuTypeString, pdesc->dev_type ) == 0 )
		ret = VOS_OK;
	if(!VOS_StrStr(pdesc->dev_type,"GT") && VOS_StrCmp( onuTypeString, "UNKNOW" ) == 0)/*同时读到其他厂商ONU 和 升级文件时，升级该ONU*/
		ret = VOS_OK;

	return ret;
}
int update_onu_slot_pon_onuid(char *szName,char *onuList,struct vty *vty)
{
	    LONG lRet = VOS_OK;	
		ULONG ulSlot = 0;
		ULONG ulPort = 0;
		ULONG upOnuId = 0;
		INT16 phyPonId = 0;	
		INT16 userOnuId = 0;	
		int count = 0;
		int	error_count=0;
		ULONG fileType;
		onu_update_msg_t updMsg;

		lRet = IFM_ParseSlotPort( szName, &ulSlot, &ulPort );
		if( lRet != VOS_OK )
			return CMD_WARNING;

		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) !=  VOS_OK)
			return(CMD_WARNING);

		if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
			return CMD_WARNING;

		if(SYS_MODULE_IS_GPON(ulSlot))
		{
			sys_console_printf("the onu is Gpon! you con't update epon app!");
			return CMD_WARNING;
		}

		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
		    vty_out( vty, "  %% Parameter is error.\r\n" );
		    return CMD_WARNING;
		}

		fileType = IMAGE_TYPE_ALL;

		/*added by wutw 2006/11/10*/
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( onuList, upOnuId )
		{
		    count++;
		    
			userOnuId = (INT16)(upOnuId - 1);

		    if ( ROK != ThisIsValidOnu(phyPonId, userOnuId) )
		    {
		        error_count++;
		    	vty_out( vty, "  onu %d/%d/%d not exist\r\n", ulSlot, ulPort, upOnuId );
		        continue;
		    }

		    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
		    lRet = GetOnuOperStatus( phyPonId, userOnuId);
		    if ( 2 == lRet)
		    {
		        error_count++;
		    	vty_out( vty, "  onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, upOnuId );
		        continue;
		    }	
		    
			/* added by chenfj 2008-2-21
			启动ONU软件升级时,判断ONU是否正处于升级状态;若ONU正升级,
			则不将此ONU 添加到等待软件升级队列中
			*/
			if( GetOnuSWUpdateStatus(phyPonId, userOnuId ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
			{
				error_count++;
				vty_out( vty, "  onu %d/%d/%d image is updating now\r\n", ulSlot, ulPort, upOnuId );
				continue;
			}

			/*act:操作码， 0 -- 升级单个ONU；1 -- 升级全部ONU
			para: 操作数， 当act为0时，该值为要升级的ONU索引，当act为 1 时，该值为0
			*/
			/*act = MSGTYPE_UPDATE_ONE;
			para = 10000*ulSlot + 1000*ulPort + upOnuId;*/
			ONU_MGMT_SEM_TAKE;
			OnuMgmtTable[phyPonId*MAXONUPERPON+userOnuId].vty = vty;
			ONU_MGMT_SEM_GIVE;
			/*sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, fileType);*/

			VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
			updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
			updMsg.msg_type = MSGTYPE_UPDATE_ONE;
			updMsg.onuDevIdx = MAKEDEVID(ulSlot, ulPort, upOnuId);
			updMsg.file_type = fileType;
			if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
				vty_out( vty, "perhaps update task is busy now!\r\n" );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();

		if ( error_count == count )
		{
		   	return CMD_WARNING;
		}
		return CMD_SUCCESS;
			
}
static char *zeName;
static char *onuList;
static struct vty *vty1;
void update_callback()
{
	update_onu_slot_pon_onuid(zeName,onuList,vty1);
}
/*int calc_define_time_to_update_onu(char *time)
{
   SLOCAL_TIME stTime;
   char hours[2] = {0};
   char minute[2] = {0};
   VOS_StrnCpy(hours,time,2);
   minute[0] = time[3];
   minute[1] = time[4];
   VOS_AtoI(hours);
   
   devsm_get_slocal_time( &stTime );
   
}*/
DEFUN  (
    clear_waiting_onu,
    clear_waiting_onu_cmd,
    "clear update waiting-onu UpdateStatus {<slot/port>}*1",
    "clear\n"
    "software update command"
    "waiting-onu  onu waiting to update \n"
    "input pon slot/port\n"
    /*"onu app image\n"
    "onu voice image\n"
    "onu fgpa image\n" */
    )
{
	ULONG slot,port,onuid;
	int updatingstaus;
	short int llid;
	unsigned int PonPortIdx;

	if(argc == 1)
	{
		IFM_ParseSlotPort( argv[0], &slot, &port );

		PonPortIdx = GetPonPortIdxBySlot(slot, port);
	    if(PonPortIdx == VOS_ERROR)
	    {   
			vty_out(vty,"  %%  %d/%d is invalid\r\n", slot, port);
			return(CMD_WARNING );
	    }
		for(onuid=0; onuid < MAXONUPERPON; onuid++)
		{
			GetGponOnuSwUpdateStatus(PonPortIdx,onuid,&updatingstaus);
			if(updatingstaus)
			{
				SetGponOnuSwUpdateStatus(PonPortIdx,onuid,ONU_SW_UPDATE_STATUS_NOOP);
				llid = GetLlidByOnuIdx(PonPortIdx,onuid);
				gponOnuAdp_StopUpgrade(PonPortIdx,llid);
			}
		}
	}
	else
	{
		for ( slot = PONCARD_FIRST; slot <= PONCARD_LAST; slot ++ )
    	{
            if(SlotCardMayBePonBoard(slot) != ROK)
                continue;
            
            for( port=1; port<=PONPORTPERCARD; port++ )
            {
                PonPortIdx = GetPonPortIdxBySlot(slot, port);
                if( PonPortIdx == RERROR)
                    continue;
                for(onuid=0; onuid < MAXONUPERPON; onuid++)
					SetGponOnuSwUpdateStatus(PonPortIdx,onuid,ONU_SW_UPDATE_STATUS_NOOP);
            }
		}
	}
	return ROK;
}

DEFUN  (
    clear_update_Status,
    clear_update_Status_cmd,
    "clear update waiting {<slot/port>}*1",
    "software update command\n"
    "software update command"
    "status  Get update status \n"
    "input pon slot/port\n"
    /*"onu app image\n"
    "onu voice image\n"
    "onu fgpa image\n" */
    )
{
	ULONG slot,port,onuid;
	unsigned int PonPortIdx;

	if(argc == 1)
	{
		IFM_ParseSlotPort( argv[0], &slot, &port );

		PonPortIdx = GetPonPortIdxBySlot(slot, port);
	    if(PonPortIdx == VOS_ERROR)
	    {   
			vty_out(vty,"  %%  %d/%d is invalid\r\n", slot, port);
			return(CMD_WARNING );
	    }
		for(onuid=0; onuid < MAXONUPERPON; onuid++)
			setOnuUpWaitingFlag( slot, port,onuid,GPON_CLR_UPDATE);
	}
	else
	{
		for ( slot = PONCARD_FIRST; slot <= PONCARD_LAST; slot ++ )
    	{
            if(SlotCardMayBePonBoard(slot) != ROK)
                continue;
            
            for( port=1; port<=PONPORTPERCARD; port++ )
            {
                PonPortIdx = GetPonPortIdxBySlot(slot, port);
                if( PonPortIdx == RERROR)
                    continue;
                for(onuid=0; onuid < MAXONUPERPON; onuid++)
					setOnuUpWaitingFlag( slot, port,onuid,GPON_CLR_UPDATE);
            }
		}
	}
	return ROK;
}
DEFUN  (
    config_gonu_file_update,
   config_gonu_file_update_cmd,
    "update gpon onu file <slot/port> <onuid_list>",
    "software update command\n"
    "onu gpon"
    "onu software update\n"
    "onu software update\n"
    "input pon slot/port/onuid\n"
    /*"onu app image\n"
    "onu voice image\n"
    "onu fgpa image\n" */
    )
{
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuid = 0;
	ULONG upOnuId = 0;
	INT16 phyPonId = 0;
	INT16 userOnuId = 0;
	ULONG fileType;
	onu_update_msg_t updMsg;
	
	lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if(!SYS_MODULE_IS_GPON(ulSlot))
	{
		sys_console_printf("onu is epon!you con't update Gpon app!");
		return CMD_WARNING;
	}

	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
	if (phyPonId == VOS_ERROR)
	{
        sys_console_printf("  %% Parameter is error.\r\n" );
        return CMD_WARNING;
	}
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], upOnuId )
	{
		userOnuId = (INT16)(upOnuId - 1);
		if ( ROK != ThisIsValidOnu(phyPonId, userOnuId) )
	    {
			sys_console_printf("  onu %d/%d/%d not exist\r\n", ulSlot, ulPort, upOnuId );
	    	continue;
	    }
		
		lRet = GetOnuOperStatus( phyPonId, userOnuId);
		if ( ONU_OPER_STATUS_DOWN == lRet)
		{
		   	sys_console_printf("  onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, upOnuId );
		    continue;
		}	
		/*begin: added by liub 2017-6-8, 检查升级文件类型和onu类型是否匹配*/
		if(gpon_update_file_check(phyPonId, userOnuId) != VOS_OK)
		{
			sys_console_printf("app file does not match to onu %d/%d/%d \r\n", ulSlot, ulPort, upOnuId);
			continue;
		}
		/*end: added by liub 2017-6-8*/

		if( GetOnuSWUpdateStatus(phyPonId, userOnuId ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
		{
			sys_console_printf("  onu %d/%d/%d image is updating now\r\n", ulSlot, ulPort, upOnuId );
			continue;
		}
		fileType = IMAGE_TYPE_ALL;
		VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
		updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
		updMsg.msg_type = MSGTYPE_UPDATEGPON_ONE;
		updMsg.onuDevIdx = MAKEDEVID(ulSlot, ulPort, upOnuId);
		updMsg.file_type = fileType;
		OLT_GPON_DEBUG("  onu %d OLT_SetOnuUpdateMsg\r\n",updMsg.onuDevIdx);
		if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
			vty_out( vty, "perhaps update task is busy now!\r\n" );
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	if(lRet == VOS_OK)
		return CMD_SUCCESS;
	else
		return CMD_WARNING;
}

DEFUN  (
    config_onu_file_update,
   config_onu_file_update_cmd,
    "update epon onu file <slot/port> <onuid_list> { <time> }*1",
    "software update command\n"
    "onu epon\n"
    "onu software update\n"
    "one or more onu software update\n"
    "input pon slot/port\n"
    OnuIDStringDesc
    "input definite time,e,g  hour.minute\n"
    /*"onu app image\n"
    "onu voice image\n"
    "onu fgpa image\n" */
    )
{


    
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	/*ULONG ulOnuId = 0;
	ULONG ulIfIndex = 0;*/
	ULONG upOnuId = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	
	/*ulong_t act = 0;
	ulong_t para = 0;*/
	int count = 0;
	int	error_count=0;
	ULONG fileType;
	onu_update_msg_t updMsg;
 /*  if( argc < 3 )
   {
	   update_onu_slot_pon_onuid(argv[0],argv[1],vty);
   }
   else
   {*/
        int ret = 0;
        zeName = argv[0];
        onuList = argv[1];
        vty1 = vty;
        ret =  VOS_TimerCreate( MODULE_RPU_CMC, 0, 300000, update_callback, (void *)NULL, VOS_TIMER_NO_LOOP );
		sys_console_printf("zhaoxh ret = %d",ret);
   			
 //   }
	
}

/* modified by chenfj 2007-10-25
	参数中增加ONU侧epon程序，ONU侧的语音程序(voice) 可选项	
	当不带任何参数时，对GT831类型ONU，会同时升级其epon和voice 	
	对其他类型ONU，只升级epon；
	当带有可选项时，则升级指定的程序
	以后还会有ONU的FPGA程序 ，待扩展
	*/
/* modified by chenfj 2008-2-18
     GT831 内部发布技术评审时, 讨论决定,简化ONU 程序升级命令, 
     不再带有epon  和voice 可选参数;
     这样就避免了只升级GT831 语音程序后, ONU不重启,从而OLT 上保存的ONU
     语音程序版本与ONU实际运行版本不一致的可能
     */
DEFUN  (
    pon_onu_file_update,
    pon_onu_file_update_cmd,
    "update onu file <onuid_list> {[epon]}*1",
    "software update command\n"
    "onu software update\n"
    "one or more onu software update\n"
    OnuIDStringDesc
    "onu app image\n"
    /*"onu voice image\n"
    "onu fgpa image\n" */
    )
{
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	
	int count = 0;
	int	error_count=0;
	ULONG fileType;
	onu_update_msg_t updMsg;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	if( argc == 1)
		fileType = IMAGE_TYPE_ALL;
	/*else if( VOS_StrCmp(argv[1], "epon") == 0)
		fileType = IMAGE_TYPE_APP;
	else if( VOS_StrCmp(argv[1], "voice") == 0)
		fileType =  IMAGE_TYPE_VOIP;
	else if( VOS_StrCmp(argv[1], "fpga") == 0)
		fileType =  IMAGE_TYPE_FPGA;
	else{
		vty_out(vty, "  %% parameter err\r\n");
		return( CMD_WARNING );
		}*/
	else
		fileType = IMAGE_TYPE_APP;

	/*added by wutw 2006/11/10*/
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
	    count++;
        
		userOnuId = (INT16)(ulOnuId - 1);

        if ( ROK != ThisIsValidOnu(phyPonId, userOnuId) )
        {
            error_count++;
	    	vty_out( vty, "  onu %d/%d/%d not exist\r\n", ulSlot, ulPort, ulOnuId );
            continue;
        }
            
	    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	    lRet = GetOnuOperStatus( phyPonId, userOnuId);
	    if ( 2 == lRet)
	    {
            error_count++;
	    	vty_out( vty, "  onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId );
	        continue;
	    }	
        
		/* added by chenfj 2008-2-21
		启动ONU软件升级时,判断ONU是否正处于升级状态;若ONU正升级,
		则不将此ONU 添加到等待软件升级队列中
		*/
		if( GetOnuSWUpdateStatus(phyPonId, userOnuId ) == ONU_SW_UPDATE_STATUS_INPROGRESS )
		/*if( GetOnuUpdatingStatusBySlot(phyPonId, userOnuId ) == ONU_SW_UPDATE_STATUS_INPROGRESS )*/	/* modified by xieshl 20111107, 这个函数有缺陷，对于onu1/1/1无法查找其状态 */
		{
			error_count++;
			vty_out( vty, "  onu %d/%d/%d image is updating\r\n", ulSlot, ulPort, ulOnuId );
			continue;
		}
	
		/*act:操作码， 0 -- 升级单个ONU；1 -- 升级全部ONU
		para: 操作数， 当act为0时，该值为要升级的ONU索引，当act为 1 时，该值为0
		*/
		/*act = MSGTYPE_UPDATE_ONE;
		para = 10000*ulSlot + 1000*ulPort + upOnuId;*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[phyPonId*MAXONUPERPON+userOnuId].vty = vty;
		ONU_MGMT_SEM_GIVE;
		/*if( sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_INVALID, act, para, fileType) == VOS_ERROR )
			vty_out( vty, "update task is not working\r\n" );*/
		VOS_MemZero( updMsg.reserved, ONU_TYPE_LEN );
		updMsg.upd_mode = IMAGE_UPDATE_MODE_INVALID;
		updMsg.msg_type = MSGTYPE_UPDATE_ONE;
		updMsg.onuDevIdx = MAKEDEVID(ulSlot, ulPort, ulOnuId);
		updMsg.file_type = fileType;
		if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
			vty_out( vty, "perhaps update task is busy now!\r\n" );
	}
	END_PARSE_ONUID_LIST_TO_ONUID();

    if ( error_count == count )
    {
	   	return CMD_WARNING;
    }
    
	return CMD_SUCCESS;	
}

/* modified by chenfj 2007-8-17 
	在命令行中增加将要升级的APP类型: gw文件及CTC文件
	
	modified by chenfj 2007-10-24
	单独做一个命令，用于ONU APP文件在GW模式和CTC模式之间切换
	当前支持的ONU类型: 
	   在GW模式下，向CTC模式切换时，ONU 类型有GT810/GT816；升级的文件为GT_CTC_4FE.   其余类型均不升级(待扩展);
	   在CTC模式下，向GW模式切换时，如果ONU的FE端口等于1，VOIP与E1端口均为0，则升级GT816/810文件（GT816与GT810目前是同一个文件），否则不升级
	*/

LONG  UpdateEnabledOnuAll = 0;
LONG UpdateDisabledOnuAll = 0;
char convert_onu_file_type[ONU_TYPE_LEN +4];
DEFUN  (
    pon_onu_file_convert,
    pon_onu_file_convert_cmd,
    "convert onu-file <onuid_list> <onu_file_id>",
    "convert onu file command\n"
    "convert onu file\n"
    OnuIDStringDesc
    "onu file identify\n"	/*"if running non-ctc mode, then update to CTC file; and if running CTC mode, then update to non-ctc file\n"*/
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	INT16 userOnuId = 0;	
	/*int count = 0;*/
	/*ULONG file_spec;*/
	/*int OnuType;*/
	/*int FE_num, POTS_num, E1_num;*/
	onu_update_msg_t updMsg;

	if( VOS_StrLen(argv[1]) > ONU_TYPE_LEN )
	{
		vty_out( vty, " file_type is too long\r\n" );
		return CMD_WARNING;
	}
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	if( checkVtyPonIsValid(vty, ulSlot, ulPort) != VOS_OK )
		return CMD_WARNING;

	VOS_MemZero( convert_onu_file_type, sizeof(convert_onu_file_type) );
	VOS_StrCpy( convert_onu_file_type, argv[1] );

	/*added by wutw 2006/11/10*/
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 0 ], ulOnuId )
	{
		userOnuId = (INT16)(ulOnuId - 1);
#if 0	/* modified by xieshl 20100304, 放开GW<->CTC ONU 文件转换限制*/
	    /*onu`s status of running. when status is onu_down and not register. Cann`t set!*/	
	    lRet = GetOnuOperStatus( phyPonId, userOnuId);
	    if ( 2 == lRet)
	    {
	       continue;
	    }	

		lRet = GetOnuType(phyPonId, userOnuId, &OnuType );
		if( lRet != ROK )
			continue;
		if(( OnuType == V2R1_ONU_GT810) || ( OnuType == V2R1_ONU_GT816) || ( OnuType == V2R1_ONU_GT811_A ))
			{
			file_spec=  MSG_SPEC_CTC_TYPE;
			}
		else if( OnuType == V2R1_ONU_CTC )
			{
			CTC_getDeviceCapEthPortNum(phyPonId, userOnuId, &FE_num);
			CTC_getDeviceCapiadPotsPortNum(phyPonId, userOnuId, &POTS_num);
			CTC_getDeviceCapE1PortNum( phyPonId, userOnuId, &E1_num);

			if((( FE_num == 1 ) && ( E1_num == 0 ) && ( POTS_num == 0)) /* GT816/810 */
				|| (( FE_num == 4 ) && ( E1_num == 0 ) && ( POTS_num == 0)) ) /* GT811_A */
				{
				file_spec = MSG_SPEC_GW_TYPE;
				}
			
			else{
				continue;	
				}
			}
		else {
			continue;
			}
#else
		/*update_mode = get_onu_image_update_mode( 0, phyPonId, userOnuId );
		if( update_mode == IMAGE_UPDATE_MODE_UNKNOWN )
			continue;*/
#endif
		/*act:操作码， 0 -- 升级单个ONU；1 -- 升级全部ONU
		para: 操作数， 当act为0时，该值为要升级的ONU索引，当act为 1 时，该值为0
		*/
		/*act = MSGTYPE_UPDATE_ONE;
		para = 10000*ulSlot + 1000*ulPort + upOnuId;*/
		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[phyPonId*MAXONUPERPON+userOnuId].vty = vty;
		ONU_MGMT_SEM_GIVE;
		/*sendOnuOamUpdMsg( IMAGE_UPDATE_MODE_UNKNOWN, act, para, IMAGE_TYPE_APP );*/
		VOS_MemCpy( updMsg.reserved, convert_onu_file_type, ONU_TYPE_LEN );
		updMsg.upd_mode = IMAGE_UPDATE_MODE_UNKNOWN;
		updMsg.msg_type = MSGTYPE_UPDATE_ONE;
		updMsg.onuDevIdx = MAKEDEVID(ulSlot, ulPort, ulOnuId);
		updMsg.file_type = IMAGE_TYPE_APP;
		if( OLT_SetOnuUpdateMsg( phyPonId, &updMsg ) != OLT_ERR_OK )
			vty_out( vty, "perhaps update task is busy now!\r\n" );
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	return CMD_SUCCESS;	
	
}

/* added by xieshl 20110110, 简化命令实现，问题单11796 */
void show_onu_updating_bar( struct vty *vty, int result_type )
{
	if( result_type == ONU_UPD_RESULT_TYPE_UNKNOWN )
		vty_out( vty, " No updating onu\r\n" );
	else
	{
		vty_out( vty, "\r\nonuIdx  onu-type        running-ver  updating-ver  %s\r\n\r\n", 
			(result_type == ONU_UPD_RESULT_TYPE_TIME) ? "last-updating" : "rate" );
		/*vty_out( vty, "-------------------------------------------------------------------------\r\n\r\n");	*/
	}
}
void show_onu_updating_list( struct vty *vty, short int PonPortIdx, onuUpdateStatusList_t *pUpdateInfo)
{
	int count = 0;
	onuUpdateStatus_t *pUpd;
	for( count = 0; count < pUpdateInfo->num; count++ )
	{
        UCHAR name[MAXDEVICENAMELEN+1] = {0}; 
        int namelen = 0;
        UCHAR print_name[16] = {0};
		pUpd = &pUpdateInfo->updStat[count];
		if( GetOnuModel( PonPortIdx, pUpd->onuid, name, &namelen) != ROK )
			VOS_StrCpy( print_name, "-" );
        else
        {
            if(namelen>16)
                VOS_MemCpy(print_name, name, 15);
            else
			    VOS_StrCpy(print_name, name);
        }
		vty_out( vty," %-5d  %-16s%-14s%-13s", (pUpd->onuid+1), print_name/*OnuTypeToString(pUpd->type)*/, pUpd->swversion, pUpd->version );
		
		if( pUpd->result_type == ONU_UPD_RESULT_TYPE_WAITING )
			vty_out( vty, "waiting" );
		else if( pUpd->result_type == ONU_UPD_RESULT_TYPE_TIME )
		{
			onuupdateclk_t *pdata = (onuupdateclk_t *)&pUpd->result_data;
			int year = ( pdata->st_clk.year < 0 ) ? 2000+32+pdata->st_clk.year : pdata->st_clk.year+2000;
			int month = (pdata->st_clk.month<0)?16+pdata->st_clk.month:pdata->st_clk.month;
			int day = (pdata->st_clk.day<0)?32+pdata->st_clk.day:pdata->st_clk.day;
			int hour = (pdata->st_clk.hour<0 )?32+pdata->st_clk.hour:pdata->st_clk.hour;
			int minute = (pdata->st_clk.minute<0)?64+pdata->st_clk.minute:pdata->st_clk.minute;
			int second = (pdata->st_clk.second<0)?64+pdata->st_clk.second:pdata->st_clk.second;

			vty_out( vty, "%d-%02d-%02d,%02d:%02d:%02d", year, month, day, hour, minute, second );
		}
		else if( pUpd->result_type == ONU_UPD_RESULT_TYPE_RATE )
			vty_out( vty, "%d%%", pUpd->result_data );
		else /*if( type == ONU_UPD_RESULT_TYPE_UNKNOWN )*/
			vty_out( vty, "unknown" );

		vty_out( vty,"\r\n");
	}
	vty_out( vty,"\r\n");
}

DEFUN  (
    show_olt_updating_onu,
    show_olt_updating_onu_cmd,
    "show updating onu {<slot/port>}*1",
    DescStringCommonShow
    "Show updating onu\n"
    "Show updating onu\n"
    "input slot/port\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	/*ULONG ulOnuId = 0;*/
	INT16 phyPonId = 0;	
	onuUpdateStatusList_t updateInfo;

	if( argc == 1 )
	{
		IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
		if ((ulSlot<PONCARD_FIRST) || (ulSlot>PONCARD_LAST))
		{
		   	vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
			return CMD_WARNING;
		}
		
		if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
			return(CMD_WARNING);
		
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}
		
		VOS_MemZero( &updateInfo, sizeof(updateInfo) );
		if( (OLT_GetUpdatingOnuList( phyPonId, &updateInfo ) != 0) || (updateInfo.num == 0) )
		{
			show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_UNKNOWN );
		}
		else
		{
			show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_RATE );
			show_onu_updating_list( vty, phyPonId, &updateInfo );
		}
	}
	else
	{
		show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_RATE );

		for(phyPonId = 0; phyPonId < MAXPON; phyPonId ++)
		{
			VOS_MemZero( &updateInfo, sizeof(updateInfo) );
			if( (OLT_GetUpdatingOnuList( phyPonId, &updateInfo ) != 0) || (updateInfo.num == 0) )
				continue;
			
			vty_out(vty," pon%d/%d [updating onu counter=%d]\r\n",GetCardIdxByPonChip(phyPonId),GetPonPortByPonChip(phyPonId), updateInfo.num);
				
			show_onu_updating_list( vty, phyPonId, &updateInfo );
		}
	}
	return CMD_SUCCESS;
}

DEFUN  (
    show_localpon_updating_onu,
    show_localpon_updating_onu_cmd,
    "show local pon updating onu",
    DescStringCommonShow
    "Show updating onu\n"
    "Show updating onu\n"
    "input slot/port\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	unsigned char update_status=ONU_SW_UPDATE_STATUS_NOOP;


	vty_out( vty, " ponid/onuIdx\r\n");

	for(phyPonId = 0; phyPonId < MAXPON; phyPonId ++)
	{
		for(ulOnuId=0;ulOnuId<MAXONUPERPON;ulOnuId ++)
		{
			update_status = ONU_SW_UPDATE_STATUS_NOOP;
			if(GetGponOnuSwUpdateStatus(phyPonId,ulOnuId,&update_status) == ROK)
			{
				if(update_status == ONU_SW_UPDATE_STATUS_INPROGRESS)
				{
					vty_out( vty, "%d/%d\r\n",phyPonId,ulOnuId);
				}
			}
		}
	}
	return CMD_SUCCESS;
}


DEFUN  (
    show_pon_updating_onu,
    show_pon_updating_onu_cmd,
    "show updating onu",
    DescStringCommonShow
    "Show updating onu\n"
    "Show updating onu\n" )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	onuUpdateStatusList_t updateInfo;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	VOS_MemZero( &updateInfo, sizeof(updateInfo) );
	if( (OLT_GetUpdatingOnuList(phyPonId, &updateInfo) != 0) || (updateInfo.num == 0) )
	{
		show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_UNKNOWN );
	}
	else
	{
		show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_RATE );
		show_onu_updating_list( vty, phyPonId, &updateInfo );
	}
	return CMD_SUCCESS;
	
}


DEFUN (
	pon_wait_update_onu_show,
	pon_wait_update_onu_show_cmd,
	"show waiting-update onu",
	"show instruction\n"
	"updating status is waiting\n"
	"updating target is onu\n"
	/*"pon port location description\n"*/
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	onu_update_waiting_t waitList;
	int i, j;
	int count=0;

	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	VOS_MemZero( &waitList, sizeof(waitList) );
	if( OLT_GetOnuUpdateWaiting(phyPonId, &waitList) != 0 )
	{
		vty_out(vty, "\r\npon %d/%d waiting onu NULL\r\n", ulSlot, ulPort);
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty, "\r\npon %d/%d waiting onu list:\r\n", ulSlot, ulPort);
		for( i=1; i<=MAXONUPERPON; i++)
		{
			USHORT val = waitList.wait_onu[i];
			if( val & WAIT_UPDATE )
			{
				count++;
				vty_out(vty, "\r\n%-40s%d", "onu index:", MAKEDEVID(ulSlot, ulPort,(i)));
				for(j=1;j<14;j++)
				{
					if(val & (1<<j))
					{
						vty_out(vty, "\r\n%-40s%s", "file type is:", onuOamUpdFileType2Str(j));
					}
				}
				vty_out(vty, "\r\n");
			}
		}
		vty_out(vty, "\r\n%-40s%d\r\n", "total onu num is:", count);
	}
	return CMD_SUCCESS;
}


DEFUN (
	wait_update_onu_show,
	wait_update_onu_show_cmd,
	"show waiting-update onu {<slot/pon>}*1",
	"show instruction\n"
	"updating status is waiting\n"
	"updating target is onu\n"
	"pon port location description\n"
	)
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	INT16 phyPonId = 0;
	unsigned char display_flag;
	onu_update_waiting_t waitList;
	int count=0;
	int i,j;

	if(argc > 1)
	{
		return (CMD_WARNING);
	}
	else if(argc == 1)
	{
		IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
		if ((ulSlot<PONCARD_FIRST) || (ulSlot>PONCARD_LAST))
		{
		   	vty_out( vty, "  %% Error slot %d.\r\n", ulSlot );
			return CMD_WARNING;
		}
		
		if(SlotCardMayBePonBoardByVty(ulSlot, vty) != ROK )
			return(CMD_WARNING);
		
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
		if (phyPonId == VOS_ERROR)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}
		
		VOS_MemZero( &waitList, sizeof(waitList) );
		if( OLT_GetOnuUpdateWaiting(phyPonId, &waitList) != 0 )
		{
			vty_out(vty, "\r\npon %d/%d waiting onu NULL\r\n", ulSlot, ulPort);
			return CMD_SUCCESS;
		}
		else
		{
			vty_out(vty, "\r\npon %d/%d waiting onu list:\r\n", ulSlot, ulPort);
			for( i=1; i<=MAXONUPERPON; i++)
			{
				if( waitList.wait_onu[i] & WAIT_UPDATE )
				{
					count++;
					vty_out(vty, "\r\n%-40s%d", "onu index:", MAKEDEVID(ulSlot, ulPort,(i)));
					for(j=1;j<14;j++)
					{
						if(waitList.wait_onu[i] & (1<<j))
						{
							vty_out(vty, "\r\n%-40s%s", "file type is:", onuOamUpdFileType2Str(j));
						}
					}
					vty_out(vty, "\r\n");
				}
			}
			vty_out(vty, "\r\n%-40s%d\r\n", "total onu num is:", count);
		}
	}
	else
	{
		for( ulSlot = PONCARD_FIRST; ulSlot <= PONCARD_LAST; ulSlot++)
		{
			/* modified by chenfj 2008-3-18
			 现象: config 节点下show waiting onu命令只能显示最靠前的PON板,后面的PON就
			               显示不到了;
			 修改: 在每个循环前, 给ponidx 赋初值=1 
			 */
			if((SYS_MODULE_IS_RUNNING(ulSlot)) && (SlotCardIsPonBoard(ulSlot)== ROK))
			{
				for( ulPort=1; ulPort<=PONPORTPERCARD; ulPort++)
				{
					/* modified by chenfj 2008-3-18
					     原因: 当没有输入参数, 若有多个PON板,打印的信息就太多了
					     修改: 增加信息打印开关display_flag, 只显示有等待升级的ONU
					     */
					display_flag = 1;
					count = 0;

					phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
					if (phyPonId == VOS_ERROR)
					{
						continue;
					}

					VOS_MemZero( &waitList, sizeof(waitList) );
					if( OLT_GetOnuUpdateWaiting(phyPonId, &waitList) != 0 )
					{
						vty_out(vty, "\r\npon %d/%d waiting onu NULL\r\n", ulSlot, ulPort);
						continue;
					}
					
					for( i=1; i<=MAXONUPERPON; i++ )
					{
						if( waitList.wait_onu[i] & WAIT_UPDATE )
						{
							count++;
							if( display_flag == 1 )
							{
								vty_out(vty, "\r\npon %d/%d waiting onu list:\r\n", ulSlot, ulPort);
								display_flag = 2 ;
							}
							
							vty_out(vty, "\r\n%-40s%d", "onu index:", MAKEDEVID(ulSlot, ulPort, i));								
							for( j=1; j<8; j++ )
							{
								if( waitList.wait_onu[i] & (1<<j) )
								{
									vty_out(vty, "\r\n%-40s%s", "file type is:", onuOamUpdFileType2Str(j));
								}
							}
							vty_out(vty, "\r\n");
						}
					}
					if( display_flag == 2 )
					vty_out(vty, "\r\n%-40s%d\r\n","total onu num is:", count);
				}
			}
		}
	}
	return CMD_SUCCESS;
}

/* modified by xieshl 20110117, 重新修改显示格式，问题单11919 */
void show_onu_soft_upd_header( struct vty *vty )
{
	vty_out(vty, " OnuIdx  en/disable  OnuIdx  en/disable  OnuIdx  en/disable\r\n");
	vty_out(vty, "-----------------------------------------------------------\r\n");
}
int show_onu_soft_upd_list( struct vty *vty, short int PonPortIdx )
{
	short int OnuIdx, slot, port;
	char str[16];
	int enabledOnu=0, disabledOnu=0;
	int i = 0;

	CHECK_PON_RANGE;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	for(OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		if(ThisIsValidOnu( PonPortIdx, OnuIdx) != ROK )  continue;

		VOS_Sprintf( str, "%d/%d/%d", slot, port, (OnuIdx+1) );
		vty_out(vty, " %-9s", str );
		if( GetOnuSWUpdateCtrl(PonPortIdx, OnuIdx ) == ONU_SW_UPDATE_ENABLE )
		{
			vty_out(vty, "enable    ");
			enabledOnu ++;
		}
		else {
			vty_out(vty, "disable   ");
			disabledOnu ++;
		}
		i++;
		if( i == 3 )
		{
			vty_out(vty, "\r\n");
			i = 0;
		}
	}
	UpdateEnabledOnuAll += enabledOnu;
	UpdateDisabledOnuAll +=disabledOnu;

	if( i ) vty_out(vty, "\r\n");
	vty_out(vty,"\r\n");
	return ROK;
}

int ShowOnuSoftwareUpdateByVty( short int PonPortIdx, struct vty *vty)
{
	/*short int OnuIdx;
	int enabledOnu=0, disabledOnu=0;*/
	int counter= 0;
	
	CHECK_PON_RANGE;

	counter = AllOnuCounter(PonPortIdx );
	if( counter <= 0)
	{
		vty_out(vty,"  no onu update enable\r\n\r\n");
		return( ROK);
	}

	show_onu_soft_upd_header( vty );
	show_onu_soft_upd_list( vty, PonPortIdx );

	return( ROK );
}


int  ShowOnuSoftwareUpdateAllByVty(struct vty *vty)
{
	short int PonPortIdx;

	UpdateEnabledOnuAll = 0;
	UpdateDisabledOnuAll = 0;

	show_onu_soft_upd_header( vty );

	for(PonPortIdx=0;PonPortIdx < MAXPON; PonPortIdx ++)
	{
		if( AllOnuCounter(PonPortIdx) > 0 )
			show_onu_soft_upd_list( vty, PonPortIdx );
	}

	vty_out(vty, " Total software update:enabled-onu=%d,disabled-onu=%d\r\n\r\n", UpdateEnabledOnuAll, UpdateDisabledOnuAll);
	return( ROK );
}
/* end 20110117 */

DEFUN  (
    pon_updated_onu,
    pon_updated_onu_cmd,
    "show onu updating information",
    DescStringCommonShow
    "Show onu information\n"
    "Show onu updated information\n"
    "Show onu updated information\n" )
{ 
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG ulOnuId = 0;
	INT16 phyPonId = 0;	
	onuUpdateStatusList_t updateInfo;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &phyPonId) != VOS_OK )
		return CMD_WARNING;

	VOS_MemZero( &updateInfo, sizeof(updateInfo) );
	if( OLT_GetUpdatedOnuList( phyPonId, &updateInfo ) == 0 )
	{
		show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_UNKNOWN );
	}
	else
	{
		show_onu_updating_bar( vty, ONU_UPD_RESULT_TYPE_TIME );
		show_onu_updating_list( vty, phyPonId, &updateInfo );
	}
	return CMD_SUCCESS;
}

DEFUN  (
    olt_all_auto_update_enable,
    olt_all_auto_update_enable_cmd,
    "onu software update enable",
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update enable\n"
    "Config the onu software update enable\n"
    )
{
	INT16 userOnuId = 0;	
	INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	EnableFlag = V2R1_ENABLE;

	for( phyPonId = 0; phyPonId < MAXPON; phyPonId++)
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId++ )
		{
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			#if 0
			lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			if (lRet != VOS_OK)
			{
				vty_out( vty, "  %% Executing error.\r\n");
				return CMD_WARNING;
			}
			#endif
		}
	}
	return CMD_SUCCESS;
}


DEFUN  (
    olt_all_auto_update_disable,
    olt_all_auto_update_disable_cmd,
    "undo onu software update enable",
    NO_STR
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update disable\n"
    "Config the onu software update disable\n"
    )
{	
	INT16 userOnuId = 0;	
	INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	EnableFlag = V2R1_DISABLE;

	for( phyPonId = 0; phyPonId < MAXPON; phyPonId++)
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId++ )
		{
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			#if 0
			lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			if (lRet != VOS_OK)
			{
				vty_out( vty, "  %% Executing error.\r\n");
				return CMD_WARNING;
			}
			#endif
		}
	}
	
	return CMD_SUCCESS;
}


DEFUN  (
    olt_pon_auto_update_enable,
    olt_pon_auto_update_enable_cmd,
    "onu software update enable <slotId/port> {<onuid_list>}*1",
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update enable\n"
    "Config the onu software update enable\n"
    "Please input slot/port\n"
    OnuIDStringDesc
    )
{	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG onuId = 0;
	INT16 userOnuId = 0;	
	INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	EnableFlag = V2R1_ENABLE;

	VOS_Sscanf( argv[0], "%d/%d", &ulSlot, &ulPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return(CMD_WARNING);
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if(phyPonId == (VOS_ERROR))
	{
		vty_out( vty, "  %% slotId/port err\r\n");
		return CMD_WARNING;
	}
	
	if (argc == 1)
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId++ )
		{
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			#if 0
			lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			if (lRet != VOS_OK)
			{
				vty_out( vty, "  %% Executing error.\r\n");
				return CMD_WARNING;
			}
			#endif
		}
	}
	else
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], onuId )
		{
			userOnuId = (short int)(onuId -1);
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}
	return CMD_SUCCESS;
}


DEFUN  (
    olt_pon_auto_update_disable,
    olt_pon_auto_update_disable_cmd,
    "undo onu software update enable <slotId/port> {<onuid_list>}*1",
    NO_STR
    "Config the onu software\n"
    "Config the onu software\n"
    "Config the onu software update disable\n"
    "Config the onu software update disable\n"
    "Please input slot/port\n"
    OnuIDStringDesc
    )
{
	LONG lRet = VOS_OK;	
	ULONG ulSlot = 0;
	ULONG ulPort = 0;
	ULONG onuId = 0;
	INT16 userOnuId = 0;	
	INT16 phyPonId = 0;
	unsigned char EnableFlag = 0;

	EnableFlag = V2R1_DISABLE;

	VOS_Sscanf( argv[0], "%d/%d", &ulSlot, &ulPort );
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
		return(CMD_WARNING);
	phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
	if(phyPonId == (VOS_ERROR))
	{
		vty_out( vty, "  %% slotId/port err\r\n");
		return CMD_WARNING;
	}

	if (argc == 1)
	{
		for( userOnuId = 0; userOnuId < MAXONUPERPON; userOnuId++ )
		{
			lRet = SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
			/*
			if (lRet != VOS_OK)
			{
				vty_out( vty, "  %% Executing error.\r\n");
				return CMD_WARNING;
			}*/
		}
	}
	else
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[ 1 ], onuId )
		{
			userOnuId = (short int)(onuId -1);
			SetOnuSWUpdateCtrl( phyPonId, userOnuId, EnableFlag);
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}
    return CMD_SUCCESS;
}

DEFUN  (
    olt_pon_auto_updated_show,
    olt_pon_auto_updated_show_cmd,
    "show onu software update {<slot/port>}*1",
    DescStringCommonShow
    "Show onu information\n"
    "Show onu information\n"
    "Show onu update information\n"
    "Please input slotId/port\n"
    )
{
	ULONG ulSlot = 0;
	ULONG ulPort = 0;	
	INT16 phyPonId = VOS_ERROR;

	vty_out( vty, "\r\n" );
	if ( argc == 0 )
		ShowOnuSoftwareUpdateAllByVty( vty );
	else
	{
		if( IFM_ParseSlotPort(argv[0], &ulSlot, &ulPort) != VOS_OK )
			return CMD_WARNING;
		if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort,vty) != ROK)
			return(CMD_WARNING);
		phyPonId = GetPonPortIdxBySlot( (short int)ulSlot, (short  int)ulPort );
		if(phyPonId == (VOS_ERROR))
		{
			vty_out( vty, "  %% slotId/port err\r\n");
			return CMD_WARNING;
		}

		ShowOnuSoftwareUpdateByVty( phyPonId, vty );
	}

	return CMD_SUCCESS;
}



LONG onuOamUpdCommandInstall()
{
	install_element ( CONFIG_NODE, &config_onu_file_update_cmd );
	install_element ( CONFIG_NODE, &config_gonu_file_update_cmd );
	install_element ( PON_PORT_NODE, &pon_onu_file_update_cmd);
	install_element ( PON_PORT_NODE, &pon_onu_file_convert_cmd );

	install_element (PON_PORT_NODE,&pon_wait_update_onu_show_cmd );
	install_element ( CONFIG_NODE, &show_olt_updating_onu_cmd );
	install_element ( CONFIG_NODE, &wait_update_onu_show_cmd);
	install_element ( VIEW_NODE, &wait_update_onu_show_cmd);

	install_element ( PON_PORT_NODE, &pon_updated_onu_cmd);
	install_element ( PON_PORT_NODE, &show_pon_updating_onu_cmd);

	install_element ( CONFIG_NODE, &olt_all_auto_update_enable_cmd);
	install_element ( CONFIG_NODE, &olt_all_auto_update_disable_cmd);
	install_element ( CONFIG_NODE, &olt_pon_auto_update_enable_cmd);
	install_element ( CONFIG_NODE, &olt_pon_auto_update_disable_cmd);
	install_element ( CONFIG_NODE, &olt_pon_auto_updated_show_cmd);
	install_element ( VIEW_NODE, &olt_pon_auto_updated_show_cmd);

	install_element ( DEBUG_HIDDEN_NODE, &clear_update_Status_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &clear_waiting_onu_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &show_localpon_updating_onu_cmd);
	return VOS_OK;
}

#ifdef __cplusplus
}
#endif
