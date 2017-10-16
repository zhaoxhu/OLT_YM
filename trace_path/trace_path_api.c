#ifdef __cplusplus
extern "C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#pragma pack(1)
#include  "trace_path_lib.h"
#include  "trace_path_main.h"
#include  "trace_path_api.h"


extern LONG trace_path_fdb_resolving( UCHAR *pMacAddr, user_loc_t* pLoc );


static user_loc_t mib_user_loc_bak;

LONG trace_path_fdb_init()
{
	VOS_MemZero( &mib_user_loc_bak, sizeof(user_loc_t) );
	
	return VOS_OK;
}

static LONG trace_path_fdb_read_ok( UCHAR *pMacAddr )
{
	LONG rc = VOS_ERROR;
	if( MAC_ADDR_IS_UNEQUAL(mib_user_loc_bak.userMac, pMacAddr) )
		return rc;
	if( !SYS_SLOT_PORT_IS_LEGAL(mib_user_loc_bak.oltBrdIdx, mib_user_loc_bak.oltPortIdx) )
		return rc;
	if( mib_user_loc_bak.resolvingStatus != USER_TRACE_RESOLVED_OK )
		return rc;
	if( mib_user_loc_bak.resolvingTimer >= 33 )
		return rc;

	mib_user_loc_bak.resolvingTimer++;
	
	return VOS_OK;
}

LONG swMac2Cascaded( UCHAR *pSwMacAddr )
{
	LONG cas = 1;
	if( pSwMacAddr != NULL )
	{
		if( !MAC_ADDR_IS_INVALID(pSwMacAddr) )
			cas = 2;
	}
	return cas;
}

typedef enum {
	macTraceMacAddr = 1,
	macTraceOltSysId,
	macTraceOltBrdIdx,
	macTraceOltPortIdx,
	macTraceOnuDevIdx,
	macTraceOnuBrdIdx,
	macTraceOnuPortIdx,
	macTraceSwCascaded,
	macTraceSwMacAddr,
	macTraceSwPortIdx,
	macTraceMacStatus
} trace_mac_obj_id_t;

static LONG trace_mac_obj_val_get( UCHAR *pMacAddr, trace_mac_obj_id_t obj_id, VOID *pValue )
{
	user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pMacAddr == NULL) || (pValue == NULL) )
		return rc;

	if( (rc = trace_mac_location_get(pMacAddr, &loc)) == VOS_OK )
	{
		trace_path_debug_out( "TRACE-PATH:get mac %s OK\r\n", trace_path_mac_addr_2_str(loc.userMac) );
	}
	else
	{
		if( (rc = trace_path_fdb_read_ok(pMacAddr)) == VOS_OK )
		{
			trace_path_debug_out( "TRACE-PATH:get mac %s from fdb record OK\r\n", trace_path_mac_addr_2_str(loc.userMac) );
			VOS_MemCpy( &loc, &mib_user_loc_bak, sizeof(user_loc_t) );
		}
		else
		{
			if( (rc = trace_path_fdb_resolving(pMacAddr, &loc)) == VOS_OK )
			{
				if( SYS_SLOT_PORT_IS_LEGAL(loc.oltBrdIdx, loc.oltPortIdx) )
				{
					VOS_MemCpy( &mib_user_loc_bak, &loc, sizeof(user_loc_t) );
					MAC_ADDR_CPY( (mib_user_loc_bak.userMac), pMacAddr );
					mib_user_loc_bak.resolvingTimer = 0;
				}
				else
				{
					rc = VOS_ERROR;
				}
			}
			trace_path_debug_out( "TRACE-PATH:get mac %s from fdb %s\r\n", trace_path_mac_addr_2_str(pMacAddr), ((rc == VOS_OK) ? "OK" : "ERR") );
		}
	}
	
	if( rc == VOS_OK )
	{
		switch( obj_id )
		{
			case macTraceMacAddr:
				break;
			case macTraceOltSysId:
				MAC_ADDR_CPY( (char *)pValue, SYS_PRODUCT_BASEMAC );
				break;
			case macTraceOltBrdIdx:
				*(ULONG*)pValue = loc.oltBrdIdx;
				break;
			case macTraceOltPortIdx:
				*(ULONG*)pValue = loc.oltPortIdx;
				break;
			case macTraceOnuDevIdx:
				*(ULONG*)pValue = MAKEDEVID( loc.oltBrdIdx, loc.oltPortIdx, loc.onuId );
				break;
			case macTraceOnuBrdIdx:
				*(ULONG*)pValue = loc.onuBrdIdx;
				break;
			case macTraceOnuPortIdx:
				*(ULONG*)pValue = loc.onuPortIdx;
				break;
			case macTraceSwCascaded:
				*(ULONG*)pValue = swMac2Cascaded( loc.swMacAddr );
				break;
			case macTraceSwMacAddr:
				MAC_ADDR_CPY( (char *)pValue, loc.swMacAddr );
				break;
			case macTraceSwPortIdx:
				*(ULONG*)pValue = loc.swPortIdx;
				break;
			case macTraceMacStatus:
				break;
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	return rc;
}

LONG getMacTraceMacAddr( UCHAR *pMacAddr )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceMacAddr, pMacAddr );
}
LONG getMacTraceOltSysId( UCHAR *pMacAddr, UCHAR *pOltSysMac )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOltSysId, pOltSysMac );
}
LONG getMacTraceOltBrdIdx( UCHAR *pMacAddr, ULONG *pOltBrdIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOltBrdIdx, pOltBrdIdx );
}
LONG getMacTraceOltPortIdx( UCHAR *pMacAddr, ULONG *pOltPortIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOltPortIdx, pOltPortIdx );
}
LONG getMacTraceOnuDevIdx( UCHAR *pMacAddr, ULONG *pOnuDevIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOnuDevIdx, pOnuDevIdx );
}
LONG getMacTraceOnuBrdIdx( UCHAR *pMacAddr, ULONG *pOnuBrdIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOnuBrdIdx, pOnuBrdIdx );
}
LONG getMacTraceOnuPortIdx( UCHAR *pMacAddr, ULONG *pOnuPortIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceOnuPortIdx, pOnuPortIdx );
}
LONG getMacTraceSwCascaded( UCHAR *pMacAddr, ULONG *pSwFlag )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceSwCascaded, pSwFlag );
}
LONG getMacTraceSwMacAddr( UCHAR *pMacAddr, UCHAR *pSwMacAddr )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceSwMacAddr, pSwMacAddr );
}
LONG getMacTraceSwPortIdx( UCHAR *pMacAddr, ULONG *pSwPortIdx )
{
	return trace_mac_obj_val_get( pMacAddr, macTraceSwPortIdx, pSwPortIdx );
}
LONG getMacTraceMacStatus( UCHAR *pMacAddr, ULONG *pMacStatus )
{
	LONG rc = VOS_ERROR;
	if( (rc = trace_mac_obj_val_get(pMacAddr, macTraceSwMacAddr, pMacStatus)) == VOS_OK )
	{
		*pMacStatus = 1;
	}
	return rc;
}

LONG getMacTraceFirstIndex( UCHAR *pFirstMacAddr )
{
	trace_mac_table_t tbl;
	LONG rc = VOS_ERROR;
	
	if( pFirstMacAddr == NULL )
		return rc;

	rc = trace_mac_tbl_first_get( &tbl );
	if( rc == VOS_OK )
	{
		MAC_ADDR_CPY( pFirstMacAddr, tbl.userMac );
	}
	return rc;
}

LONG getMacTraceNextIndex( UCHAR *pMacAddr, UCHAR *pNextMacAddr )
{
	trace_mac_table_t tbl;
	LONG rc = VOS_ERROR;
	
	if( (pMacAddr == NULL) || (pNextMacAddr == NULL) )
		return rc;
	if( MAC_ADDR_IS_ZERO(pMacAddr) )
		return getMacTraceFirstIndex( pNextMacAddr );

	rc = trace_mac_tbl_next_get( pMacAddr, &tbl );
	if( rc == VOS_OK )
	{
		MAC_ADDR_CPY( pNextMacAddr, tbl.userMac );
	}
	
	return rc;
}

LONG checkMacTraceIndex( UCHAR *pMacAddr )
{
	LONG rc = VOS_ERROR;
	if( pMacAddr == NULL )
		return rc;
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return rc;
	if( trace_mac_status_get(pMacAddr) == USER_TRACE_RESOLVED_OK )
	{
		rc = VOS_OK;
	}
	return rc;
}



typedef enum {
	userLocUserId = 1,
	userLocOltSysId,
	userLocOltBrdIdx,
	userLocOltPortIdx,
	userLocOnuDevIdx,
	userLocOnuBrdIdx,
	userLocOnuPortIdx,
	userLocSwCascaded,
	userLocSwMacAddr,
	userLocSwPortIdx,
	userLocRowStatus,
	userLocUserMacAddr
} trace_userid_obj_id_t;

static LONG trace_userid_obj_val_get( UCHAR *pUserId, trace_userid_obj_id_t obj_id, VOID *pValue )
{
	user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pValue == NULL) )
		return rc;

	if( (rc = trace_userid_location_get(pUserId, &loc)) == VOS_OK )
	{
		switch( obj_id )
		{
			case userLocUserId:
				break;
			case userLocOltSysId:
				MAC_ADDR_CPY( (char *)pValue, SYS_PRODUCT_BASEMAC );
				break;
			case userLocOltBrdIdx:
				*(ULONG*)pValue = loc.oltBrdIdx;
				break;
			case userLocOltPortIdx:
				*(ULONG*)pValue = loc.oltPortIdx;
				break;
			case userLocOnuDevIdx:
				*(ULONG*)pValue = MAKEDEVID( loc.oltBrdIdx, loc.oltPortIdx, loc.onuId );
				break;
			case userLocOnuBrdIdx:
				*(ULONG*)pValue = loc.onuBrdIdx;
				break;
			case userLocOnuPortIdx:
				*(ULONG*)pValue = loc.onuPortIdx;
				break;
			case userLocSwCascaded:
				*(ULONG*)pValue = (MAC_ADDR_IS_INVALID(loc.swMacAddr) ? 1 : 2);
				break;
			case userLocSwMacAddr:
				MAC_ADDR_CPY( (char *)pValue, loc.swMacAddr );
				break;
			case userLocSwPortIdx:
				*(ULONG*)pValue = loc.swPortIdx;
				break;
			case userLocUserMacAddr:
				VOS_Sprintf( pValue, "%02x:%02x:%02x:%02x:%02x:%02x", loc.userMac[0], loc.userMac[1], loc.userMac[2], loc.userMac[3], loc.userMac[4], loc.userMac[5] );
				break;
			case userLocRowStatus:
				break;
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	return rc;
}

LONG getUserLocUserId( UCHAR *pUserId )
{
	return trace_userid_obj_val_get( pUserId, userLocUserId, pUserId );
}

LONG getUserLocOltSysId( UCHAR *pUserId, UCHAR *pOltSysMac )
{
	/*LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOltSysMac == NULL) )
		return rc;
	if( (rc = checkUserLocIndex(pUserId)) == VOS_OK )
	{
		VOS_MemCpy( pOltSysMac, SYS_PRODUCT_BASEMAC, 6 );
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOltSysId, pOltSysMac );
}

LONG getUserLocOltBrdIdx( UCHAR *pUserId, ULONG *pOltBrdIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOltBrdIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pOltBrdIdx = loc.oltBrdIdx;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOltBrdIdx, pOltBrdIdx );
}

LONG getUserLocOltPortIdx( UCHAR *pUserId, ULONG *pOltPortIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOltPortIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pOltPortIdx = loc.oltPortIdx;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOltPortIdx, pOltPortIdx );
}

LONG getUserLocOnuDevIdx( UCHAR *pUserId, ULONG *pOnuDevIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOnuDevIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pOnuDevIdx = MAKEDEVID( loc.oltBrdIdx, loc.oltPortIdx, loc.onuId );
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOnuDevIdx, pOnuDevIdx );
}

LONG getUserLocOnuBrdIdx( UCHAR *pUserId, ULONG *pOnuBrdIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOnuBrdIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pOnuBrdIdx = loc.onuBrdIdx;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOnuBrdIdx, pOnuBrdIdx );
}

LONG getUserLocOnuPortIdx( UCHAR *pUserId, ULONG *pOnuPortIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pOnuPortIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pOnuPortIdx = loc.onuPortIdx;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocOnuPortIdx, pOnuPortIdx );
}

LONG getUserLocSwCascaded( UCHAR *pUserId, ULONG *pSwFlag )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pSwFlag == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		if( MAC_ADDR_IS_INVALID(loc.swMacAddr) )
			*pSwFlag = 1;
		else
			*pSwFlag = 2;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocSwCascaded, pSwFlag );
}

LONG getUserLocSwMacAddr( UCHAR *pUserId, UCHAR *pSwMac )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pSwMac == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		VOS_MemCpy( pSwMac, loc.swMacAddr, USER_MACADDR_LEN );
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocSwMacAddr, pSwMac );
}

LONG getUserLocSwPortIdx( UCHAR *pUserId, ULONG *pSwPortIdx )
{
	/*user_loc_t loc;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pSwPortIdx == NULL) )
		return rc;
	if( (rc = user_loc_get(pUserId, &loc)) == VOS_OK )
	{
		*pSwPortIdx = loc.swPortIdx;
	}
	return rc;*/
	return trace_userid_obj_val_get( pUserId, userLocSwPortIdx, pSwPortIdx );
}

LONG getUserLocUserMacAddr( UCHAR *pUserId, UCHAR *pSwMac )
{
	return trace_userid_obj_val_get( pUserId, userLocUserMacAddr, pSwMac );
}

LONG getUserLocRowStatus( UCHAR *pUserId, ULONG *pRowStatus )
{
	LONG rc = VOS_ERROR;
	LONG status;
	
	if( (pUserId == NULL) || (pRowStatus == NULL) )
		return rc;
	
	status = trace_userid_status_get( pUserId );
	if( status == USER_TRACE_RESOLVED_OK )
	{
		*pRowStatus = USER_LOC_ROWSTATUS_ACTIVE;
		rc = VOS_OK;
	}
	else if( status == USER_TRACE_RESOLVING_WAIT )
	{
		*pRowStatus = USER_LOC_ROWSTATUS_NOTREADY;
		rc = VOS_OK;
	}
	/*else  if( status == USER_TRACE_RESOLVING_NON )
	{
		*pRowStatus = USER_LOC_ROWSTATUS_DESTROY;
	}*/
	return rc;
}

LONG setUserLocRowStatus( UCHAR *pUserId, ULONG rowStatus )
{
	LONG rc = VOS_ERROR;
	LONG status;
	
	if( pUserId == NULL )
		return rc;

	status = trace_userid_status_get( pUserId );
	if( (status == USER_TRACE_RESOLVED_OK) || (status == USER_TRACE_RESOLVING_WAIT) )
	{
		if( rowStatus == USER_LOC_ROWSTATUS_DESTROY )
		{
			rc = trace_userid_tbl_delete( pUserId );
		}
	}

	return rc;
}

LONG getUserLocFirstIndex( UCHAR *pFirstUserId )
{
	trace_userid_table_t tbl;
	LONG rc = VOS_ERROR;
	
	if( pFirstUserId == NULL )
		return rc;

	rc = trace_userid_tbl_first_get( &tbl );
	if( rc == VOS_OK )
	{
		VOS_MemZero( pFirstUserId, USER_ID_MAXLEN );
		VOS_StrnCpy( pFirstUserId, tbl.userId, USER_ID_MAXLEN );
	}
	return rc;
}

LONG getUserLocNextIndex( UCHAR *pUserId, UCHAR *pNextUserId )
{
	trace_userid_table_t tbl;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pNextUserId == NULL) )
		return rc;
	if( pUserId[0] == 0 )
		return getUserLocFirstIndex( pNextUserId );

	rc = trace_userid_tbl_next_get( pUserId, &tbl );
	if( rc == VOS_OK )
	{
		VOS_MemZero( pNextUserId, USER_ID_MAXLEN );
		VOS_StrnCpy( pNextUserId, tbl.userId, USER_ID_MAXLEN );
	}
	
	return rc;
}

LONG checkUserLocIndex( UCHAR *pUserId )
{
	if( pUserId == NULL )
		return VOS_ERROR;
	if( trace_userid_status_get(pUserId) == USER_TRACE_RESOLVED_OK )
		return VOS_OK;
	return VOS_ERROR;
}

typedef enum {
	vlanTraceSvlan = 1,
	vlanTraceCvlan,
	vlanTraceOltSysId,
	vlanTraceOltbrdIdx,
	vlanTraceOltPortIdx,
	vlanTraceOnuDevIdx,
	vlanTraceOnuBrdIdx,
	vlanTraceOnuPortIdx,
} trace_vlan_obj_id_t;

trace_vlan_data_t g_trace_vlan_buff;
int g_trace_vlan_flag = 0;
#define TRACE_VLAN_DEBUG  if(g_trace_vlan_flag) sys_console_printf
static ULONG s_mib_trace_tick = 0;
static int s_mib_trace_ret = VOS_ERROR;

extern int GetVFPUserLocationByVlan(USHORT svlan, USHORT cvlan, short int *slot, short int *port, short int *onuId, ULONG *onuBrd, ULONG *onuPort);

LONG getvlanTraceOltBrdIdx( USHORT svlan, USHORT cvlan, ULONG *pOltBrdIdx )
{
    int ret = VOS_ERROR;
    short int ulSlot = 0;
    short int ulPort = 0;
    ULONG ulOnuBrd = 0;
    ULONG ulOnuPort = 0;
    if(g_trace_vlan_buff.flag && g_trace_vlan_buff.svlan == svlan && g_trace_vlan_buff.cvlan == cvlan)
    {
        *pOltBrdIdx = g_trace_vlan_buff.OltBrd;
        ret = s_mib_trace_ret;
    }
    else
    {        
        ULONG begin_time = VOS_GetTick();
        ULONG end_time = 0;
        short int ulOnuId = 0;
        if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
        {
            g_trace_vlan_buff.flag = 1;
            g_trace_vlan_buff.OltBrd = ulSlot;
            g_trace_vlan_buff.OltPort = ulPort;
            g_trace_vlan_buff.OnuId = ulOnuId;
            g_trace_vlan_buff.OnuBrd = ulOnuBrd;
            g_trace_vlan_buff.OnuPort = ulOnuPort;
            *pOltBrdIdx = ulSlot; 
            end_time = VOS_GetTick();
			s_mib_trace_ret = VOS_OK;			
            s_mib_trace_tick = VOS_GetTick();
            TRACE_VLAN_DEBUG("find ok by QinQ-Map with %d Ticks!\r\n", end_time-begin_time);
            return VOS_OK;                                
        }
        
        for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
        {
#if 1            
            if(!SYS_MODULE_IS_PON(ulSlot))
                continue;
#endif            
            for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
                short int OnuIdx = 0;     
                static Olt_llid_vlan_manipulation vlan_qinq_policy;
                
                if(PonPortIdx == RERROR)
                    continue;

                /*提高检索效率，检测下行标签剥离。现网大多一个或多个pon口一个外层vlan，无需遍历全部的onu*/
                GetPonDownlinkVlanQinQ(PonPortIdx, svlan, &vlan_qinq_policy);
                if(vlan_qinq_policy.vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
                    continue;
                    
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK)
                        continue;
                    
                    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
                    if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED &&
                        vlan_qinq_policy.new_vlan_id == svlan)
                    {
                        if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE
                            || vlan_qinq_policy.original_vlan_id == cvlan)
                        {
                            if(SearchOnuVlanPort(PonPortIdx, OnuIdx, cvlan, &ulOnuBrd, &ulOnuPort) == VOS_OK)
                            {
                                g_trace_vlan_buff.flag = 1;
                                g_trace_vlan_buff.OltBrd = ulSlot;
                                g_trace_vlan_buff.OltPort = ulPort;
                                g_trace_vlan_buff.OnuId = OnuIdx+1;
                                g_trace_vlan_buff.OnuBrd = ulOnuBrd;
                                g_trace_vlan_buff.OnuPort = ulOnuPort;

                                *pOltBrdIdx = ulSlot; 
                                end_time = VOS_GetTick();
								s_mib_trace_ret = VOS_OK;								
                                s_mib_trace_tick = VOS_GetTick();
                                TRACE_VLAN_DEBUG("find ok with %d Ticks!\r\n", end_time-begin_time);
                                return VOS_OK;                                
                            }
                        }
                    }         
                    else
                    {
                        /*do nothing*/
                    }
                }
            }
        }
        end_time = VOS_GetTick();
        TRACE_VLAN_DEBUG("find error with %d Ticks!\r\n", end_time-begin_time);

		/*added by luh@2015-7-3. 该vlan对找不到，需要标记一下，防止后续重复查找*/
        g_trace_vlan_buff.flag = 1;
		s_mib_trace_ret = VOS_ERROR;

    }
    
	return ret;
}

LONG getvlanTraceOltPortIdx( USHORT svlan, USHORT cvlan, ULONG *pOltPortIdx )
{
    int ret = VOS_ERROR;
    short int ulSlot = 0;
    short int ulPort = 0;
    ULONG ulOnuBrd = 0;
    ULONG ulOnuPort = 0;
    if(g_trace_vlan_buff.flag && g_trace_vlan_buff.svlan == svlan && g_trace_vlan_buff.cvlan == cvlan)
    {
        *pOltPortIdx = g_trace_vlan_buff.OltPort;
        ret = s_mib_trace_ret;
    }
    else
    {    
        short int ulOnuId = 0;
        if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
        {
            g_trace_vlan_buff.flag = 1;
            g_trace_vlan_buff.OltBrd = ulSlot;
            g_trace_vlan_buff.OltPort = ulPort;
            g_trace_vlan_buff.OnuId = ulOnuId;
            g_trace_vlan_buff.OnuBrd = ulOnuBrd;
            g_trace_vlan_buff.OnuPort = ulOnuPort;
			s_mib_trace_ret = VOS_OK;

            *pOltPortIdx = ulPort;                                    
            s_mib_trace_tick = VOS_GetTick();
            TRACE_VLAN_DEBUG("find ok\r\n");
            return VOS_OK;                                
        }
        
        for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
        {
            
#if 1            
            if(!SYS_MODULE_IS_PON(ulSlot))
                continue;
#endif            
            for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
                short int OnuIdx = 0;  
                static Olt_llid_vlan_manipulation vlan_qinq_policy;
				
                if(PonPortIdx == RERROR)
                    continue;

                /*提高检索效率，检测下行标签剥离。现网大多一个或多个pon口一个外层vlan，无需遍历全部的onu*/
                GetPonDownlinkVlanQinQ(PonPortIdx, svlan, &vlan_qinq_policy);
                if(vlan_qinq_policy.vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
                    continue;
				
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    static Olt_llid_vlan_manipulation vlan_qinq_policy;
                    if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK)
                        continue;
                    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
                    if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED &&
                        vlan_qinq_policy.new_vlan_id == svlan)
                    {
                        if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE
                            || vlan_qinq_policy.original_vlan_id == cvlan)
                        {
                            if(SearchOnuVlanPort(PonPortIdx, OnuIdx, cvlan, &ulOnuBrd, &ulOnuPort) == VOS_OK)
                            {
                                g_trace_vlan_buff.flag = 1;
                                g_trace_vlan_buff.OltBrd = ulSlot;
                                g_trace_vlan_buff.OltPort = ulPort;
                                g_trace_vlan_buff.OnuId = OnuIdx+1;
                                g_trace_vlan_buff.OnuBrd = ulOnuBrd;
                                g_trace_vlan_buff.OnuPort = ulOnuPort;
                                TRACE_VLAN_DEBUG("find ok\r\n");
								s_mib_trace_ret = VOS_OK;								
                                s_mib_trace_tick = VOS_GetTick();
                                *pOltPortIdx = ulPort;                                    
                                return VOS_OK;                                
                            }
                        }
                        else
                        {
                        }
                    }
                }
            }
        }
		/*added by luh@2015-7-3. 该vlan对找不到，需要标记一下，防止后续重复查找*/
        g_trace_vlan_buff.flag = 1;
		s_mib_trace_ret = VOS_ERROR;
        TRACE_VLAN_DEBUG("find error!\r\n");
		
    }
	return ret;
}

LONG getvlanTraceOnuDevIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuDevIdx )
{
    int ret = VOS_ERROR;
    short int ulSlot = 0;
    short int ulPort = 0;
    ULONG ulOnuBrd = 0;
    ULONG ulOnuPort = 0;
    if(g_trace_vlan_buff.flag && g_trace_vlan_buff.svlan == svlan && g_trace_vlan_buff.cvlan == cvlan)
    {
        *pOnuDevIdx = g_trace_vlan_buff.OnuId;
        ret = s_mib_trace_ret;
    }
    else
    {  
        short int ulOnuId = 0;
        if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
        {
            g_trace_vlan_buff.flag = 1;
            g_trace_vlan_buff.OltBrd = ulSlot;
            g_trace_vlan_buff.OltPort = ulPort;
            g_trace_vlan_buff.OnuId = ulOnuId;
            g_trace_vlan_buff.OnuBrd = ulOnuBrd;
            g_trace_vlan_buff.OnuPort = ulOnuPort;

            *pOnuDevIdx = ulOnuId;  
			s_mib_trace_ret = VOS_OK;			
            s_mib_trace_tick = VOS_GetTick();
            TRACE_VLAN_DEBUG("find ok\r\n");
            return VOS_OK;                                
        }
        
        for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
        {
            
#if 1            
            if(!SYS_MODULE_IS_PON(ulSlot))
                continue;
#endif            
            for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
                short int OnuIdx = 0;   
                static Olt_llid_vlan_manipulation vlan_qinq_policy;
				
                if(PonPortIdx == RERROR)
                    continue;

                /*提高检索效率，检测下行标签剥离。现网大多一个或多个pon口一个外层vlan，无需遍历全部的onu*/
                GetPonDownlinkVlanQinQ(PonPortIdx, svlan, &vlan_qinq_policy);
                if(vlan_qinq_policy.vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
                    continue;
				
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    static Olt_llid_vlan_manipulation vlan_qinq_policy;
                    if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK)
                        continue;
                    
                    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
                    if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED &&
                        vlan_qinq_policy.new_vlan_id == svlan)
                    {
                        if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE
                            || vlan_qinq_policy.original_vlan_id == cvlan)
                        {
                            if(SearchOnuVlanPort(PonPortIdx, OnuIdx, cvlan, &ulOnuBrd, &ulOnuPort) == VOS_OK)
                            {
                                g_trace_vlan_buff.flag = 1;
                                g_trace_vlan_buff.OltBrd = ulSlot;
                                g_trace_vlan_buff.OltPort = ulPort;
                                g_trace_vlan_buff.OnuId = OnuIdx+1;
                                g_trace_vlan_buff.OnuBrd = ulOnuBrd;
                                g_trace_vlan_buff.OnuPort = ulOnuPort;
                                TRACE_VLAN_DEBUG("find ok\r\n");
								s_mib_trace_ret = VOS_OK;								
                                s_mib_trace_tick = VOS_GetTick();
                                *pOnuDevIdx = OnuIdx+1;                                    
                                return VOS_OK;                                
                            }
                        }
                        else
                        {
                        }
                    }
                }
            }
        }
		/*added by luh@2015-7-3. 该vlan对找不到，需要标记一下，防止后续重复查找*/
        g_trace_vlan_buff.flag = 1;
		s_mib_trace_ret = VOS_ERROR;
        TRACE_VLAN_DEBUG("find error!\r\n");		
    }
	return ret;
}

LONG getvlanTraceOnuBrdIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuBrdIdx )
{
    int ret = VOS_ERROR;
    short int ulSlot = 0;
    short int ulPort = 0;
    ULONG ulOnuBrd = 0;
    ULONG ulOnuPort = 0;
    if(g_trace_vlan_buff.flag && g_trace_vlan_buff.svlan == svlan && g_trace_vlan_buff.cvlan == cvlan)
    {
        *pOnuBrdIdx = g_trace_vlan_buff.OnuBrd;
        ret = s_mib_trace_ret;
    }
    else
    {      
        short int ulOnuId = 0;
        if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
        {
            g_trace_vlan_buff.flag = 1;
            g_trace_vlan_buff.OltBrd = ulSlot;
            g_trace_vlan_buff.OltPort = ulPort;
            g_trace_vlan_buff.OnuId = ulOnuId;
            g_trace_vlan_buff.OnuBrd = ulOnuBrd;
            g_trace_vlan_buff.OnuPort = ulOnuPort;

            *pOnuBrdIdx = ulOnuBrd;   
			s_mib_trace_ret = VOS_OK;			
            s_mib_trace_tick = VOS_GetTick();
            TRACE_VLAN_DEBUG("find ok\r\n");
            return VOS_OK;                                
        }
        
        for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
        {
            
#if 1            
            if(!SYS_MODULE_IS_PON(ulSlot))
                continue;
#endif            
            for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
                short int OnuIdx = 0;   
                static Olt_llid_vlan_manipulation vlan_qinq_policy;
				
                if(PonPortIdx == RERROR)
                    continue;

                /*提高检索效率，检测下行标签剥离。现网大多一个或多个pon口一个外层vlan，无需遍历全部的onu*/
                GetPonDownlinkVlanQinQ(PonPortIdx, svlan, &vlan_qinq_policy);
                if(vlan_qinq_policy.vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
                    continue;
				
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    static Olt_llid_vlan_manipulation vlan_qinq_policy;
                    if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK)
                        continue;
                    
                    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
                    if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED &&
                        vlan_qinq_policy.new_vlan_id == svlan)
                    {
                        if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE
                            || vlan_qinq_policy.original_vlan_id == cvlan)
                        {
                            if(SearchOnuVlanPort(PonPortIdx, OnuIdx, cvlan, &ulOnuBrd, &ulOnuPort) == VOS_OK)
                            {
                                g_trace_vlan_buff.flag = 1;
                                g_trace_vlan_buff.OltBrd = ulSlot;
                                g_trace_vlan_buff.OltPort = ulPort;
                                g_trace_vlan_buff.OnuId = OnuIdx+1;
                                g_trace_vlan_buff.OnuBrd = ulOnuBrd;
                                g_trace_vlan_buff.OnuPort = ulOnuPort;
                                TRACE_VLAN_DEBUG("find ok\r\n");
								s_mib_trace_ret = VOS_OK;								
                                s_mib_trace_tick = VOS_GetTick();
                                *pOnuBrdIdx = ulOnuBrd;                                    
                                return VOS_OK;                                
                            }
                        }
                        else
                        {
                        }
                    }
                }
            }
        }
		/*added by luh@2015-7-3. 该vlan对找不到，需要标记一下，防止后续重复查找*/
        g_trace_vlan_buff.flag = 1;
		s_mib_trace_ret = VOS_ERROR;
        TRACE_VLAN_DEBUG("find error!\r\n");		
    }
	return ret;
}

LONG getvlanTraceOnuPortIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuPortIdx )
{
    int ret = VOS_ERROR;
    short int ulSlot = 0;
    short int ulPort = 0;
    ULONG ulOnuBrd = 0;
    ULONG ulOnuPort = 0;
    if(g_trace_vlan_buff.flag && g_trace_vlan_buff.svlan == svlan && g_trace_vlan_buff.cvlan == cvlan)
    {
        *pOnuPortIdx = g_trace_vlan_buff.OnuPort;
        ret = s_mib_trace_ret;
    }
    else
    {   
        short int ulOnuId = 0;
        if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
        {
            g_trace_vlan_buff.flag = 1;
            g_trace_vlan_buff.OltBrd = ulSlot;
            g_trace_vlan_buff.OltPort = ulPort;
            g_trace_vlan_buff.OnuId = ulOnuId;
            g_trace_vlan_buff.OnuBrd = ulOnuBrd;
            g_trace_vlan_buff.OnuPort = ulOnuPort;

            *pOnuPortIdx = ulOnuPort; 
			s_mib_trace_ret = VOS_OK;			
            s_mib_trace_tick = VOS_GetTick();
            TRACE_VLAN_DEBUG("find ok\r\n");
            return VOS_OK;                                
        }
        
        for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
        {
            
#if 1           
            if(!SYS_MODULE_IS_PON(ulSlot))
                continue;
#endif            
            for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
            {
                short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
                short int OnuIdx = 0;    
                static Olt_llid_vlan_manipulation vlan_qinq_policy;
				
                if(PonPortIdx == RERROR)
                    continue;

                /*提高检索效率，检测下行标签剥离。现网大多一个或多个pon口一个外层vlan，无需遍历全部的onu*/
                GetPonDownlinkVlanQinQ(PonPortIdx, svlan, &vlan_qinq_policy);
                if(vlan_qinq_policy.vlan_manipulation != PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG)
                    continue;
				
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    static Olt_llid_vlan_manipulation vlan_qinq_policy;
                    if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK)
                        continue;
                    
                    GetPonUplinkVlanQinQ(PonPortIdx, OnuIdx, &vlan_qinq_policy);	
                    if(vlan_qinq_policy.vlan_manipulation == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED &&
                        vlan_qinq_policy.new_vlan_id == svlan)
                    {
                        if(vlan_qinq_policy.original_vlan_id == PON_ALL_FRAMES_AUTHENTICATE
                            || vlan_qinq_policy.original_vlan_id == cvlan)
                        {
                            if(SearchOnuVlanPort(PonPortIdx, OnuIdx, cvlan, &ulOnuBrd, &ulOnuPort) == VOS_OK)
                            {
                                g_trace_vlan_buff.flag = 1;
                                g_trace_vlan_buff.OltBrd = ulSlot;
                                g_trace_vlan_buff.OltPort = ulPort;
                                g_trace_vlan_buff.OnuId = OnuIdx+1;
                                g_trace_vlan_buff.OnuBrd = ulOnuBrd;
                                g_trace_vlan_buff.OnuPort = ulOnuPort;
                                TRACE_VLAN_DEBUG("find ok\r\n");
								s_mib_trace_ret = VOS_OK;								
                                s_mib_trace_tick = VOS_GetTick();
                                *pOnuPortIdx = ulOnuPort;                                    
                                return VOS_OK;                                
                            }
                        }
                        else
                        {
                        }
                    }
                }
            }
        }
		/*added by luh@2015-7-3. 该vlan对找不到，需要标记一下，防止后续重复查找*/
        g_trace_vlan_buff.flag = 1;
		s_mib_trace_ret = VOS_ERROR;
        TRACE_VLAN_DEBUG("find error!\r\n");		
    }
	return ret;
}
int vlan_trace_is_still_valid()
{
    return (VOS_GetTick()-s_mib_trace_tick < VOS_TICK_SECOND/10?VOS_OK:VOS_ERROR);
}
LONG getUserLocTrapVarBindData( ULONG userid_hash_idx, UCHAR *pUserMac, UCHAR *pUserId, user_loc_t* pLoc )
{
	if( (pUserMac == NULL) || (pUserId == NULL) || (pLoc == NULL) )
		return VOS_ERROR;

	return trace_mac_userid_location_get( userid_hash_idx, pUserMac, pUserId, pLoc );
}

#pragma pack()


#ifdef __cplusplus
}
#endif

