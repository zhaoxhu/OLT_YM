#ifdef __cplusplus
extern "C"{
#endif

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"
#include "Cdp_pub.h"
#pragma pack(1)
#include "trace_path_lib.h"
#include "trace_path_main.h"


ULONG tracePathMSemId = 0;


trace_mac_table_t *gpTraceMacHashTable[TRACE_MAC_HASH_BUCKET];
trace_userid_table_t *gpTraceUserIdHashTable[TRACE_USERID_HASH_BUCKET];
static ULONG tracePathUserIdCurNum = 0;
ULONG tracePathUserIdDefaultNum = 0;
ULONG tracePathUserIdSupportNum = 0;

extern LONG trace_path_snoop_init();
static LONG trace_userid_tbl_del( UCHAR *pUserId );


ULONG trace_mac_hash_idx( const UCHAR *pUserMac )
{
	int i;
	ULONG seed = 131;
	ULONG hash = 0;

	for( i=0; i<USER_MACADDR_LEN; i++ )
	{
		hash = hash * seed + pUserMac[i];
	}
	return (hash % TRACE_MAC_HASH_BUCKET);
}

static trace_mac_table_t * trace_mac_tbl_new( UCHAR *pUserMac, ULONG userid_hash_idx )
{
	trace_mac_table_t *pTemp = NULL;
	trace_mac_table_t *pMacTbl = NULL;
	ULONG idx;

	if( pUserMac == NULL )
	{
		return pTemp;
	}

	idx = trace_mac_hash_idx( pUserMac );
	pMacTbl = gpTraceMacHashTable[idx];

	while( pMacTbl )
	{
		if( MAC_ADDR_IS_EQUAL(pMacTbl->userMac, pUserMac) )
		{
			pMacTbl->userIdHashIdx = userid_hash_idx;
			return pMacTbl;
		}
		pTemp = pMacTbl;
		pMacTbl = pMacTbl->next;
	}

	pMacTbl = (trace_mac_table_t *)VOS_Malloc(sizeof(trace_mac_table_t), MODULE_RPU_TRACEPATH);
	if( pMacTbl  )
	{
		VOS_MemZero( pMacTbl, sizeof(trace_mac_table_t) );

		MAC_ADDR_CPY( pMacTbl->userMac, pUserMac );
		pMacTbl->userIdHashIdx = userid_hash_idx;
		pMacTbl->next = NULL;
		
		if( pTemp )
		{
			pTemp->next = pMacTbl;
		}
		else
		{
			gpTraceMacHashTable[idx] = pMacTbl;
		}
		trace_path_debug_out( "TRACE-PATH:save mac %s to hash(%d)\r\n", trace_path_mac_addr_2_str(pUserMac), idx );
	}
	
	return pMacTbl;
}

static LONG trace_mac_tbl_del( UCHAR *pUserMac )
{
	trace_mac_table_t *pTemp = NULL;
	trace_mac_table_t *pMacTbl = NULL;
	ULONG idx;

	if( pUserMac == NULL )
	{
		return VOS_ERROR;
	}

	idx = trace_mac_hash_idx( pUserMac );
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pMacTbl = gpTraceMacHashTable[idx];
	while( pMacTbl )
	{
		if( MAC_ADDR_IS_EQUAL(pMacTbl->userMac, pUserMac) )
		{
			break;
		}
		pTemp = pMacTbl;
		pMacTbl = pMacTbl->next;
	}

	if( pMacTbl )
	{
		if( pTemp )
		{
			pTemp->next = pMacTbl->next;
			VOS_Free( pMacTbl );
		}
		else		/* 第一个 */
		{
			gpTraceMacHashTable[idx] = pMacTbl->next;
			VOS_Free( pMacTbl );
		}
		trace_path_debug_out( "TRACE-PATH:delete mac %s from hash(%d)\r\n", trace_path_mac_addr_2_str(pUserMac), idx );
	}
	VOS_SemGive( tracePathMSemId );
	
	return VOS_OK;
}

trace_mac_table_t * trace_mac_tbl_search( UCHAR *pUserMac )
{
	trace_mac_table_t *pMacTbl = NULL;
	ULONG idx;

	if( pUserMac != NULL )
	{
		idx = trace_mac_hash_idx( pUserMac );

		pMacTbl = gpTraceMacHashTable[idx];
		while( pMacTbl )
		{
			if( MAC_ADDR_IS_EQUAL(pMacTbl->userMac, pUserMac) )
				break;
			
			pMacTbl = pMacTbl->next;
		}
	}
	return pMacTbl;
}

trace_userid_table_t * trace_mac_userid_tbl_search( ULONG userid_hash_idx, UCHAR *pUserMac )
{
	trace_userid_table_t *pUserTbl = NULL;
	int i;

	if( pUserMac == NULL )
		return pUserTbl;
	if( userid_hash_idx >= TRACE_USERID_HASH_BUCKET )
		return pUserTbl;

	pUserTbl = gpTraceUserIdHashTable[userid_hash_idx];
	
	while( pUserTbl )
	{
		for( i=0; i<USER_MACADDR_NUM; i++ )
		{
			if( MAC_ADDR_IS_EQUAL(pUserTbl->userLocList[i].userMac, pUserMac) )
			{	
				return pUserTbl;
			}
		}
		pUserTbl = pUserTbl->next;
	}

	return NULL;	
}

user_loc_t * trace_mac_user_loc_search( ULONG userid_hash_idx, UCHAR *pUserMac )
{
	trace_userid_table_t *pUserTbl;
	user_loc_t *pLoc = NULL;
	int i;

	if( pUserMac == NULL )
		return pLoc;
	if( userid_hash_idx >= TRACE_USERID_HASH_BUCKET )
		return pLoc;

	pUserTbl = gpTraceUserIdHashTable[userid_hash_idx];
	
	while( pUserTbl )
	{
		for( i=0; i<USER_MACADDR_NUM; i++ )
		{
			if( MAC_ADDR_IS_EQUAL(pUserTbl->userLocList[i].userMac, pUserMac) )
			{
				pLoc = &(pUserTbl->userLocList[i]);
				return pLoc;
			}
		}
		pUserTbl = pUserTbl->next;
	}

	return pLoc;	
}


#if 1
LONG trace_mac_userid_tbl_delete( UCHAR *pUserId, UCHAR *pUserMac )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl = NULL;
	ULONG userid_hash_idx;
	int i;

	if( (pUserId == NULL) || (pUserMac == NULL) )
		return rc;
	
	if( trace_mac_tbl_del(pUserMac) == VOS_ERROR )
		return rc;

	userid_hash_idx = trace_userid_hash_idx( pUserId );

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	
	pUserTbl = trace_mac_userid_tbl_search( userid_hash_idx, pUserMac );
	if( pUserTbl )
	{
		for( i=0; i<USER_MACADDR_NUM; i++ )
		{
			if( MAC_ADDR_IS_EQUAL(pUserTbl->userLocList[i].userMac, pUserMac) )
			{
				MAC_ADDR_ZERO( pUserTbl->userLocList[i].userMac );
				pUserTbl->userLocList[pUserTbl->userCurIdx].onuId = 0;
				
				if( i != 0 )
					pUserTbl->userCurIdx = i-1;
				else
					pUserTbl->userCurIdx = USER_MACADDR_NUM-1;

				if( MAC_ADDR_IS_INVALID(pUserTbl->userLocList[pUserTbl->userCurIdx].userMac) ||
					(pUserTbl->userLocList[pUserTbl->userCurIdx].onuId == 0) )
				{
					/*pUserTbl->userCurIdx = 0;*/
					trace_userid_tbl_del( pUserTbl->userId );
				}
				/*else
				{
					pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus = USER_TRACE_RESOLVED_OK;
					pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingTimer = 0;
				}*/

				rc = VOS_OK;
				break;
			}
		}
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}
#endif
extern int GetVFPUserLocationByVlan(USHORT svlan, USHORT cvlan, short int *slot, short int *port, short int *onuId, ULONG *onuBrd, ULONG *onuPort);

LONG trace_vlan_location_get(short int svlan, short int cvlan, user_loc_t* pLoc)
{
    short int ulSlot = 0, ulPort = 0;
    ULONG ulOnuBrd = 0, ulOnuPort = 0;
    short int ulOnuId = 0;

    if(VOS_OK == GetVFPUserLocationByVlan(svlan, cvlan, &ulSlot, &ulPort, &ulOnuId, &ulOnuBrd, &ulOnuPort))
    {
        pLoc->oltBrdIdx = ulSlot;
        pLoc->oltPortIdx = ulPort;
        pLoc->onuId = ulOnuId;
        pLoc->onuBrdIdx = ulOnuBrd;
        pLoc->onuPortIdx = ulOnuPort;
        return VOS_OK;
    }
    
    for(ulSlot=PONCARD_FIRST;ulSlot<=PONCARD_LAST;ulSlot++)
    {
        if(!SYS_MODULE_IS_PON(ulSlot))
            continue;
        
        for(ulPort=1;ulPort<=/*CARD_MAX_PON_PORTNUM*/PONPORTPERCARD;ulPort++)
        {
            short int PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);
            short int OnuIdx = 0;     
            if(PonPortIdx == RERROR)
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
                            pLoc->oltBrdIdx = ulSlot;
                            pLoc->oltPortIdx = ulPort;
                            pLoc->onuId = OnuIdx+1;
                            pLoc->onuBrdIdx = ulOnuBrd;
                            pLoc->onuPortIdx = ulOnuPort;
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
    return VOS_ERROR;
}
LONG trace_mac_status_get( UCHAR *pUserMac )
{
	LONG rc = USER_TRACE_RESOLVING_NON;
	trace_mac_table_t *pMacTbl;
	trace_userid_table_t *pUserTbl;
	int i;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	
	pMacTbl = trace_mac_tbl_search( pUserMac );
	if( pMacTbl )
	{
		/*pUserTbl = trace_mac_userid_tbl_search( pMacTbl->userIdHashIdx, pUserMac );*/
		if( pMacTbl->userIdHashIdx < TRACE_USERID_HASH_BUCKET )
		{
			pUserTbl = gpTraceUserIdHashTable[pMacTbl->userIdHashIdx];
			
			while( pUserTbl )
			{
				for( i=0; i<USER_MACADDR_NUM; i++ )
				{
					if( MAC_ADDR_IS_EQUAL(pUserTbl->userLocList[i].userMac, pUserMac) )
					{	
						rc = pUserTbl->userLocList[i].resolvingStatus;
						if( (rc != USER_TRACE_RESOLVED_OK) && (rc != USER_TRACE_RESOLVING_WAIT) )
							rc = USER_TRACE_RESOLVING_NON;
						VOS_SemGive( tracePathMSemId );
						return rc;
					}
				}
				pUserTbl = pUserTbl->next;
			}
		}
	}
	VOS_SemGive( tracePathMSemId );

	return rc;
}

LONG trace_mac_location_get( UCHAR *pUserMac, user_loc_t* pLoc )
{
	LONG rc = VOS_ERROR;
	trace_mac_table_t *pMacTbl;
	user_loc_t *p;

	if( (pLoc == NULL) || (pUserMac == NULL) )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pMacTbl = trace_mac_tbl_search( pUserMac );
	if( pMacTbl )
	{
		p = trace_mac_user_loc_search( pMacTbl->userIdHashIdx, pUserMac );
		if( p )
		{
			if( p->resolvingStatus == USER_TRACE_RESOLVED_OK )
			{	
				VOS_MemCpy( pLoc, p, sizeof(user_loc_t) );
				rc = VOS_OK;
			}
		}
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}

LONG trace_mac_userid_get( ULONG userid_hash_idx, UCHAR *pUserMac, UCHAR* pUserId )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl;

	if( (pUserMac == NULL) || (pUserId == NULL) )
		return rc;
	if( userid_hash_idx >= TRACE_USERID_HASH_BUCKET )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pUserTbl = trace_mac_userid_tbl_search( userid_hash_idx, pUserMac );
	if( pUserTbl )
	{
		VOS_MemZero( pUserId, USER_ID_MAXLEN );
		VOS_StrnCpy( pUserId, pUserTbl->userId, USER_ID_MAXLEN );
		rc = VOS_OK;
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}

LONG trace_mac_userid_location_get( ULONG userid_hash_idx, UCHAR *pUserMac, UCHAR* pUserId, user_loc_t* pLoc )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl;
	user_loc_t *p;

	if( pUserMac == NULL )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	p = trace_mac_user_loc_search( userid_hash_idx, pUserMac );
	if( p )
	{
		pUserTbl = gpTraceUserIdHashTable[userid_hash_idx];
		
		if( p->resolvingStatus == USER_TRACE_RESOLVED_OK )
		{
			if( pUserId != NULL )
			{
				VOS_MemZero( pUserId, USER_ID_MAXLEN );
				VOS_StrnCpy( pUserId, pUserTbl->userId, USER_ID_MAXLEN );
			}
			if( pLoc != NULL )
			{
				VOS_MemCpy( pLoc, p, sizeof(user_loc_t) );
			}
			rc = VOS_OK;
		}
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}


LONG trace_mac_tbl_first_get( trace_mac_table_t *pFirstTbl )
{
	int idx;
	trace_mac_table_t *pMacTbl;
	user_loc_t *pLoc;
	LONG rc = VOS_ERROR;
	
	if( pFirstTbl == NULL )
		return rc;
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	
	for( idx=0; idx<TRACE_MAC_HASH_BUCKET; idx++ )
	{
		pMacTbl = gpTraceMacHashTable[idx];

		while( pMacTbl )
		{
			pLoc = trace_mac_user_loc_search( pMacTbl->userIdHashIdx, pMacTbl->userMac );
			if( pLoc )
			{
				if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
				{
					VOS_MemCpy( pFirstTbl, pMacTbl, sizeof(trace_mac_table_t) );
					VOS_SemGive( tracePathMSemId );
					return VOS_OK;
				}
			}
			pMacTbl = pMacTbl->next;
		}
	}
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}

LONG trace_mac_tbl_next_get( UCHAR *pUserMac, trace_mac_table_t *pNextTbl )
{
	ULONG idx;
	trace_mac_table_t *pMacTbl = NULL;
	user_loc_t *pLoc = NULL;
	LONG rc = VOS_ERROR;
	
	if( pNextTbl == NULL )
		return rc;
	if( pUserMac == NULL )
		return trace_mac_tbl_first_get( pNextTbl );

	idx = trace_mac_hash_idx( pUserMac );
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pMacTbl = gpTraceMacHashTable[idx];
	if( pMacTbl == NULL )
		idx++;
	else
	{
		/*pMacTbl = pMacTbl->next;
		if( pMacTbl == NULL )
			idx++;*/
		while( pMacTbl )
		{
			if( MAC_ADDR_IS_EQUAL(pMacTbl->userMac, pUserMac) )
			{
				pMacTbl = pMacTbl->next;
				break;
			}
			pMacTbl = pMacTbl->next;
		}
		if( pMacTbl == NULL )
			idx++;
	}
	
	for( ; idx<TRACE_MAC_HASH_BUCKET; idx++ )
	{
		if( pMacTbl == NULL )
			pMacTbl = gpTraceMacHashTable[idx];
		
		while( pMacTbl )
		{
			pLoc = trace_mac_user_loc_search( pMacTbl->userIdHashIdx, pMacTbl->userMac );
			if( pLoc )
			{
				if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
				{
					VOS_MemCpy( pNextTbl, pMacTbl, sizeof(trace_mac_table_t) );
					VOS_SemGive( tracePathMSemId );
					return VOS_OK;
				}
			}
			pMacTbl = pMacTbl->next;
		}
	}
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}


ULONG trace_userid_num_get()
{
	return tracePathUserIdCurNum;
}

ULONG trace_userid_hash_idx( const UCHAR *pUserId )
{

    ULONG seed = 131;
    ULONG hash = 0;

    while (*pUserId)
    {
        hash = hash * seed + (*pUserId++);
    }

    return (hash % TRACE_USERID_HASH_BUCKET);
}

static trace_userid_table_t * trace_userid_tbl_new( UCHAR *pUserId, UCHAR *pUserMac )
{
	trace_userid_table_t *pTemp = NULL;
	trace_userid_table_t *pUserTbl = NULL;
	ULONG idx;
	int i;

	if( (pUserId == NULL) || (pUserMac == NULL) )
	{
		return pTemp;
	}

	idx = trace_userid_hash_idx( pUserId );
	pUserTbl = gpTraceUserIdHashTable[idx];

	while( pUserTbl )
	{
		if( VOS_StrnCmp(pUserTbl->userId, pUserId, USER_ID_MAXLEN) == 0 )
		{
			for( i=0; i<USER_MACADDR_NUM; i++ )
			{
				if( MAC_ADDR_IS_EQUAL( pUserTbl->userLocList[i].userMac, pUserMac ) )
				{
					pUserTbl->userCurIdx = i;
					break;
				}
			}
			if( i >= USER_MACADDR_NUM )
			{
				if( pUserTbl->userCurIdx >= USER_MACADDR_NUM )
				{
					VOS_ASSERT(0);
					pUserTbl->userCurIdx = 0;
				}
				else if( pUserTbl->userCurIdx == (USER_MACADDR_NUM-1) )
				{
					pUserTbl->userCurIdx = 0;
				}
				else
					pUserTbl->userCurIdx++;
				
				if( !MAC_ADDR_IS_INVALID(pUserTbl->userLocList[pUserTbl->userCurIdx].userMac) )
					trace_mac_tbl_del( pUserTbl->userLocList[pUserTbl->userCurIdx].userMac );

				MAC_ADDR_CPY( pUserTbl->userLocList[pUserTbl->userCurIdx].userMac, pUserMac );
				pUserTbl->userLocList[pUserTbl->userCurIdx].oltBrdIdx = 0;
				pUserTbl->userLocList[pUserTbl->userCurIdx].oltPortIdx = 0;
				pUserTbl->userLocList[pUserTbl->userCurIdx].onuId = 0;
				pUserTbl->userLocList[pUserTbl->userCurIdx].onuPortIdx = 0;
				trace_mac_tbl_new( pUserMac, idx );
			}

			pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingTimer = 0;
			/*pUserTbl->userLocList[pUserTbl->userCurIdx].locFlag = USER_TRACE_FLAG_DYNAMIC;*/

			/*trace_path_debug_out( "TRACE-PATH:insert %s %s in hash (%d)\r\n", pUserId, trace_path_mac_addr_2_str(pUserMac), idx );*/

			/*return pUserTbl;*/
			goto new_exit;
		}
		pTemp = pUserTbl;
		pUserTbl = pUserTbl->next;
	}

	if( tracePathUserIdCurNum >= USER_ID_SUPPORT_MAXNUM )
	{
		trace_path_debug_out( "TRACE-PATH:insert %s ERR,as hash tbl full\r\n", pUserId );
		goto new_exit;
	}
	
	pUserTbl = (trace_userid_table_t *)VOS_Malloc(sizeof(trace_userid_table_t), MODULE_RPU_TRACEPATH);
	if( pUserTbl  )
	{
		VOS_MemZero( pUserTbl, sizeof(trace_userid_table_t) );

		VOS_StrnCpy( pUserTbl->userId, pUserId, USER_ID_MAXLEN );
		/*pUserTbl->userCurIdx = 0;*/
		MAC_ADDR_CPY( pUserTbl->userLocList[pUserTbl->userCurIdx].userMac, pUserMac );
		pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus = USER_TRACE_RESOLVING_WAIT;
		/*pUserTbl->userLocList[0].locFlag = USER_TRACE_FLAG_DYNAMIC;*/
		pUserTbl->next = NULL;

		if( pTemp )
		{
			pTemp->next = pUserTbl;
		}
		else
		{
			gpTraceUserIdHashTable[idx] = pUserTbl;
		}
		tracePathUserIdCurNum++;

		trace_mac_tbl_new( pUserMac, idx );

		trace_path_debug_out( "TRACE-PATH:resolving %s %s to hash (%d)\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pUserTbl->userLocList[pUserTbl->userCurIdx].userMac), idx );
	}

new_exit:
	/*user_trace_debug_out( "TRACE-PATH:new %s %s to hash (%d)\r\n", pUserId, trace_path_mac_addr_2_str(pUserMac), idx );*/
	
	return pUserTbl;
}

static LONG trace_userid_tbl_del( UCHAR *pUserId )
{
	trace_userid_table_t *pTemp = NULL;
	trace_userid_table_t *pUserTbl = NULL;
	ULONG idx;
	int i;

	if( pUserId == NULL )
	{
		return VOS_ERROR;
	}

	idx = trace_userid_hash_idx( pUserId );
	
	pUserTbl = gpTraceUserIdHashTable[idx];
	while( pUserTbl )
	{
		if( VOS_StrnCmp(pUserTbl->userId, pUserId, USER_ID_MAXLEN) == 0 )
		{
			break;
		}
		pTemp = pUserTbl;
		pUserTbl = pUserTbl->next;
	}

	if( pUserTbl )
	{
		for( i=0; i<USER_MACADDR_NUM; i++ )
		{
			if( MAC_ADDR_IS_INVALID(pUserTbl->userLocList[i].userMac) )
				continue;
			trace_mac_tbl_del( pUserTbl->userLocList[i].userMac );
		}

		if( tracePathUserIdCurNum > 0 )
			tracePathUserIdCurNum--;
		
		if( pTemp )
		{
			pTemp->next = pUserTbl->next;
			VOS_Free( pUserTbl );
		}
		else		/* 第一个 */
		{
			gpTraceUserIdHashTable[idx] = pUserTbl->next;
			VOS_Free( pUserTbl );
		}
	}
	
	return VOS_OK;
}

trace_userid_table_t * trace_userid_tbl_search( UCHAR *pUserId )
{
	trace_userid_table_t *pUserTbl = NULL;
	ULONG idx;

	if( pUserId != NULL )
	{
		idx = trace_userid_hash_idx( pUserId );
		pUserTbl = gpTraceUserIdHashTable[idx];
		while( pUserTbl )
		{
			if( VOS_StrnCmp(pUserTbl->userId, pUserId, USER_ID_MAXLEN) == 0 )
				break;
			
			pUserTbl = pUserTbl->next;
		}
	}
	return pUserTbl;
}

LONG trace_userid_tbl_insert( UCHAR *pUserId, UCHAR *pUserMac, UCHAR oltBrdIdx, UCHAR oltPortIdx )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl;

	if( (oltBrdIdx == 0) || (oltBrdIdx > SYS_CHASSIS_SWITCH_SLOTNUM) ||
		(oltPortIdx == 0) || (oltPortIdx > 24) )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_new( pUserId, pUserMac );
	if( pUserTbl )
	{
        if(pUserTbl->userLocList[pUserTbl->userCurIdx].oltBrdIdx == oltBrdIdx
            && pUserTbl->userLocList[pUserTbl->userCurIdx].oltPortIdx == oltPortIdx)
        {
            /*统一pon口下的onu拨号，不再清空信息，当重新定位后需要上报trap时与原有的定位做比较*/
        }
        else
        {
    		pUserTbl->userLocList[pUserTbl->userCurIdx].oltBrdIdx = oltBrdIdx;
    		pUserTbl->userLocList[pUserTbl->userCurIdx].oltPortIdx = oltPortIdx;
    		pUserTbl->userLocList[pUserTbl->userCurIdx].onuId = 0;        
    		pUserTbl->userLocList[pUserTbl->userCurIdx].onuPortIdx = 0;
            VOS_MemZero(pUserTbl->userLocList[pUserTbl->userCurIdx].swMacAddr, USER_MACADDR_LEN);
            pUserTbl->userLocList[pUserTbl->userCurIdx].swPortIdx = 0;
        }
        pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus = USER_TRACE_RESOLVING_WAIT;
		rc = VOS_OK;
	}
	VOS_SemGive( tracePathMSemId );

	trace_path_debug_out( "TRACE-PATH:insert %s %s in hash\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pUserTbl->userLocList[pUserTbl->userCurIdx].userMac) );

	return rc;
}

LONG trace_userid_tbl_delete( UCHAR *pUserId )
{
	LONG rc = VOS_ERROR;
	LONG status;

	status = trace_userid_status_get( pUserId );
	if( status != USER_TRACE_RESOLVING_NON )
	{
		VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
		rc = trace_userid_tbl_del( pUserId );
		VOS_SemGive( tracePathMSemId );
	}
	return rc;
}

LONG trace_userid_status_get( UCHAR *pUserId )
{
	LONG rc = USER_TRACE_RESOLVING_NON;
	trace_userid_table_t *pUserTbl = NULL;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_search( pUserId );
	if( pUserTbl )
	{
		rc = pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus;
	}
	VOS_SemGive( tracePathMSemId );

	if( (rc != USER_TRACE_RESOLVED_OK) && (rc != USER_TRACE_RESOLVING_WAIT) )
		rc = USER_TRACE_RESOLVING_NON;
	
	return rc;
}



LONG trace_userid_update_over( ULONG userid_hash_idx, UCHAR *pUserMac )
{
	LONG rc = VOS_ERROR;
	user_loc_t *pLoc;

	if( pUserMac == NULL )
		return rc;
	if( userid_hash_idx >= TRACE_USERID_HASH_BUCKET )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pLoc = trace_mac_user_loc_search( userid_hash_idx, pUserMac );
	if( pLoc )
	{
		pLoc->updateFlag = 0;
		rc = VOS_OK;
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}

LONG trace_userid_location_get( UCHAR *pUserId, user_loc_t* pLoc )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl;
	int i, j;

	if( pLoc == NULL )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_search( pUserId );
	if( pUserTbl )
	{
		if( pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus == USER_TRACE_RESOLVED_OK )
		{	
			VOS_MemCpy( pLoc, &(pUserTbl->userLocList[pUserTbl->userCurIdx]), sizeof(user_loc_t) );
			rc = VOS_OK;
		}
		else
		{
			j = pUserTbl->userCurIdx;
			for( i=0; i<USER_MACADDR_NUM; i++ )
			{
				if( j <= 0 )
					j = USER_MACADDR_NUM - 1;
				else
					j--;
				
				if( j == pUserTbl->userCurIdx )
					continue;

				if( (pUserTbl->userLocList[j].resolvingStatus == USER_TRACE_RESOLVED_OK) &&
					!MAC_ADDR_IS_INVALID(pUserTbl->userLocList[j].userMac) &&
					(pUserTbl->userLocList[j].onuId != 0) )
				{
					VOS_MemCpy( pLoc, &(pUserTbl->userLocList[j]), sizeof(user_loc_t) );
					rc = VOS_OK;
				}
			}
		}
	}
	VOS_SemGive( tracePathMSemId );

	return rc;	
}

LONG trace_userid_location_cmp( user_loc_t* pLoc1, user_loc_t *pLoc2 )
{
	if( (pLoc1 == NULL ) || (pLoc2 == NULL) )
		return 1;
	if( (pLoc1->onuPortIdx != pLoc2->onuPortIdx) ||
		(pLoc1->onuBrdIdx != pLoc2->onuBrdIdx) ||
		(pLoc1->onuId != pLoc2->onuId) ||
		(pLoc1->oltPortIdx != pLoc2->oltPortIdx) ||
		(pLoc1->oltBrdIdx != pLoc2->oltBrdIdx) ||
		MAC_ADDR_IS_UNEQUAL(pLoc1->swMacAddr, pLoc2->swMacAddr) ||
		(pLoc1->swPortIdx != pLoc2->swPortIdx) )
		return 1;
	return 0;	
}

LONG trace_userid_location_set( UCHAR *pUserId, UCHAR *pUserMac, user_loc_t* pNewLoc )
{
	trace_userid_table_t *pUserTbl = NULL;
	user_loc_t *pLoc;
	int i;

	if( pNewLoc == NULL )
		return VOS_ERROR;
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_new( pUserId, pUserMac );
	if( pUserTbl )
	{
		for( i=0; i<USER_MACADDR_NUM; i++ )
		{
			pLoc = &(pUserTbl->userLocList[i]);
			
			if( MAC_ADDR_IS_EQUAL( pLoc->userMac, pUserMac ) )
			{
				if( pNewLoc->resolvingStatus != USER_TRACE_RESOLVED_OK )
				{
                    /*modified by luh 2013-06-26*/
#if 1                    
					pLoc->oltPortIdx = pNewLoc->oltPortIdx;
					pLoc->oltBrdIdx = pNewLoc->oltBrdIdx;
#else                    
					pLoc->onuPortIdx = pNewLoc->onuPortIdx;
					pLoc->onuBrdIdx = pNewLoc->onuBrdIdx;
#endif                    
					/*pLoc->onuId = pNewLoc->onuId;*/
					pLoc->resolvingStatus = USER_TRACE_RESOLVING_WAIT;
					pLoc->resolvingTimer = 0;
				}
				else
				{
					if( trace_userid_location_cmp(&(pUserTbl->userLocList[i]), pNewLoc) != 0 )
					{
						/*VOS_MemCpy( &(pUserTbl->userLocList[i]), pNewLoc, sizeof(user_loc_t) );*/
						pLoc->onuPortIdx = pNewLoc->onuPortIdx;
						pLoc->onuBrdIdx = pNewLoc->onuBrdIdx;
						pLoc->onuId = pNewLoc->onuId;
						pLoc->oltPortIdx = pNewLoc->oltPortIdx;
						pLoc->oltBrdIdx = pNewLoc->oltBrdIdx;
						MAC_ADDR_CPY(pLoc->swMacAddr, pNewLoc->swMacAddr );
						pLoc->swPortIdx = pNewLoc->swPortIdx;

						pLoc->updateFlag = 1;
					}
					pLoc->resolvingStatus = USER_TRACE_RESOLVED_OK;
					pLoc->resolvingTimer = 0;
					trace_path_debug_out( "TRACE-PATH:finished %s %s in hash,upd_flag=%d\r\n", pUserTbl->userId, trace_path_mac_addr_2_str(pLoc->userMac), pLoc->updateFlag );
				}
				break;
			}
		}
	}
	VOS_SemGive( tracePathMSemId );

	return VOS_OK;
}

/*LONG trace_userid_resolving_declare( UCHAR *pUserId, UCHAR *pUserMac )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl = NULL;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_search( pUserId );
	if( pUserTbl )
	{
		MAC_ADDR_CPY( pUserTbl->userMac, pUserMac );
		pUserTbl->resolvingStatus = USER_TRACE_RESOLVING_WAIT;
		rc = VOS_OK;
	}
	VOS_SemGive( tracePathMSemId );
	return rc;
}
LONG trace_userid_resolved_over( UCHAR *pUserId )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl = NULL;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	pUserTbl = trace_userid_tbl_search( pUserId );
	if( pUserTbl )
	{
		pUserTbl->resolvingStatus = USER_TRACE_RESOLVED_OK;
		rc = VOS_OK;
	}
	VOS_SemGive( tracePathMSemId );
	return rc;
}*/

LONG trace_userid_counter_get( ULONG *pUserIdCount, ULONG *pResolvedCount, ULONG *pResolvingCount )
{
	trace_userid_table_t *pUserTbl;
	ULONG idx, i;
	ULONG userid_count = 0;
	ULONG resolved_count = 0;
	ULONG resolving_count = 0;

	if( (pUserIdCount == NULL) || (pResolvedCount == NULL) || (pResolvingCount == NULL) )
		return VOS_ERROR;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
		
	for( idx=0; idx<TRACE_USERID_HASH_BUCKET; idx++ )
	{
		pUserTbl = gpTraceUserIdHashTable[idx];
		while( pUserTbl )
		{
			userid_count++;
			
			for( i=0; i<USER_MACADDR_NUM; i++ )
			{
				if( MAC_ADDR_IS_INVALID(pUserTbl->userLocList[i].userMac) )
					continue;

				if( pUserTbl->userLocList[i].resolvingStatus == USER_TRACE_RESOLVED_OK )
					resolved_count++;
				else
					resolving_count++;
			}

			pUserTbl = pUserTbl->next;
		}
	}
	VOS_SemGive( tracePathMSemId );

	*pUserIdCount = userid_count;
	*pResolvedCount = resolved_count;
	*pResolvingCount = resolving_count;
	
	return VOS_OK;
}

LONG trace_userid_tbl_get( UCHAR *pUserId, trace_userid_table_t *pCurrTbl )
{
	trace_userid_table_t *pTbl = NULL;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pCurrTbl == NULL) )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pTbl = trace_userid_tbl_search(pUserId);
	if( pTbl != NULL )
	{
		VOS_MemCpy( pCurrTbl, pTbl, sizeof(trace_userid_table_t) );
		rc = VOS_OK;
	}
		
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}

LONG trace_userid_tbl_first_get( trace_userid_table_t *pFirstTbl )
{
	int idx;
	trace_userid_table_t *pTbl;
	LONG rc = VOS_ERROR;
	
	if( pFirstTbl == NULL )
		return rc;
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
	for( idx=0; idx<TRACE_USERID_HASH_BUCKET; idx++ )
	{
		pTbl = gpTraceUserIdHashTable[idx];
		if( pTbl != NULL )
		{
			VOS_MemCpy( pFirstTbl, pTbl, sizeof(trace_userid_table_t) );
			rc = VOS_OK;
			break;
		}
	}
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}

LONG trace_userid_tbl_next_get( UCHAR *pUserId, trace_userid_table_t *pNextTbl )
{
	ULONG idx;
	trace_userid_table_t *pTbl = NULL;
	LONG rc = VOS_ERROR;
	
	if( (pUserId == NULL) || (pNextTbl == NULL) )
		return rc;
	if( pUserId[0] == 0 )
		return trace_userid_tbl_first_get( pNextTbl );

	idx = trace_userid_hash_idx( pUserId );
	
	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pTbl = gpTraceUserIdHashTable[idx];
	if( pTbl == NULL )
		idx++;
	else
	{
		/*pTbl = pTbl->next;
		if( pTbl == NULL )
			idx++;*/
		while( pTbl )
		{
			if( VOS_StrnCmp(pTbl->userId, pUserId, USER_ID_MAXLEN) == 0 )
			{
				pTbl = pTbl->next;
				break;
			}
			pTbl = pTbl->next;
		}
		if( pTbl == NULL )
			idx++;
	}
	
	for( ; idx<TRACE_USERID_HASH_BUCKET; idx++ )
	{
		if( pTbl == NULL )
			pTbl = gpTraceUserIdHashTable[idx];
		if( pTbl != NULL )
		{
			VOS_MemCpy( pNextTbl, pTbl, sizeof(trace_userid_table_t) );
			rc = VOS_OK;
			break;
		}
	}
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}

/*LONG trace_userid_tbl_aging( UCHAR *pUserId )
{
	LONG rc = VOS_ERROR;
	trace_userid_table_t *pUserTbl = NULL;
	
	if( pUserId == NULL )
		return rc;

	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );

	pUserTbl = trace_userid_tbl_search(pUserId);
	if( pUserTbl != NULL )
	{
		if( pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingTimer > USER_ID_RESOLVING_TIMES )
		{
			trace_userid_del_notify( pUserTbl->userId, pUserTbl->userLocList[pUserTbl->userCurIdx].userMac );
			rc = VOS_OK;
		}
		else
		{
			if( (pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingStatus == USER_TRACE_RESOLVING_WAIT) || 
				(pUserTbl->userLocList[pUserTbl->userCurIdx].onuId == 0) )
			{
				pUserTbl->userLocList[pUserTbl->userCurIdx].resolvingTimer++;
			}
		}
	}
		
	VOS_SemGive( tracePathMSemId );
	
	return rc;
}*/

LONG trace_path_tbl_init( )
{
	int idx;

	if( tracePathMSemId != 0 )
		return VOS_OK;

	if( (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100) )
	{
		tracePathUserIdDefaultNum = USER_ID_CAPACITY_LOW_DEF;
	}
	else
	{
		tracePathUserIdDefaultNum = USER_ID_CAPACITY_HIGH_DEF;
	}
	tracePathUserIdSupportNum = tracePathUserIdDefaultNum;
	
	tracePathMSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );

	for( idx=0; idx<TRACE_MAC_HASH_BUCKET; idx++ )
	{
		gpTraceMacHashTable[idx] = NULL;
	}
	for( idx=0; idx<TRACE_USERID_HASH_BUCKET; idx++ )
	{
		gpTraceUserIdHashTable[idx] = NULL;
	}
	
	trace_path_snoop_init();

	return VOS_OK;						
}

#pragma pack()


#ifdef __cplusplus
}
#endif

