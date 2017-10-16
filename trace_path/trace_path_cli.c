#ifdef __cplusplus
extern "C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"
#pragma pack(1)
#include  "trace_path_lib.h"
#include  "trace_path_main.h"


extern ULONG tracePathMSemId;
extern LONG GetMacAddr( CHAR * szStr, CHAR * pucMacAddr );
extern LONG trace_path_fdb_resolving( UCHAR *pMacAddr, user_loc_t* pLoc );
extern STATUS getDeviceName( const ulong_t devIdx, char* pValBuf, ulong_t *pValLen );
extern LONG userport_is_pon (int slot, int port);
extern void trace_path_auto_sync_msg_send();
extern ULONG  tracePathAutoSyncMaxCircle;
extern ULONG  tracePathAutoSyncEnable;
extern trace_userid_table_t *gpTraceUserIdHashTable[TRACE_USERID_HASH_BUCKET];

char *trace_path_resolvingflag_2_str( UCHAR flag )
{
	char *str;
	if( flag == USER_TRACE_RESOLVING_WAIT )
		str = "resolving";
	else if( flag == USER_TRACE_RESOLVED_OK )
		str = "resolved";
	else
		str = "unknown";
	return str;
}

#define TRACE_PATH_RESULT_TIMEOUT	-1
#define TRACE_PATH_RESULT_COMPLETE	0
#define TRACE_PATH_RESULT_NOTFIND	1

char *trace_path_result_hint_2_str( int flag )
{
	char *pHintStr;
	if( flag == TRACE_PATH_RESULT_COMPLETE )
		pHintStr = "Trace complete.";
	else if( flag == TRACE_PATH_RESULT_NOTFIND )
		pHintStr = "Not find!";
	else
		pHintStr = "Request timed out.";
	return pHintStr;
}

CHAR *trace_path_mac_addr_2_str( UCHAR *pUserMac )
{
	static CHAR macStr[20];
	if( pUserMac != NULL )
		VOS_Sprintf( macStr, "%02x%02x.%02x%02x.%02x%02x", pUserMac[0], pUserMac[1], pUserMac[2], pUserMac[3], pUserMac[4], pUserMac[5] );
	else
		return "-";
	return macStr;
}


QDEFUN( trace_path_list_show,
	trace_path_list_show_cmd,
	"show trace-path history {[count]}*1",
	"Show information\n"
	"Show user path records based user id\n"
	"Show user path records history based user id\n"
	"Show all users patch records counter\n",
	&tracePathQId)
{
	int i, j;
	trace_userid_table_t userTbl;
	trace_userid_table_t *pUserTbl = &userTbl;
	UCHAR user_id[USER_ID_MAXLEN];
    UCHAR print_userid[20] = {0};
	user_loc_t *pLoc;
	ULONG resolving_count = 0;
	ULONG resolved_count = 0;
	ULONG userid_count = 0;
	char tmp[32];
#if 0
	if( argc == 0 )
	{
		vty_out( vty, "%-22s %-11s %-10s %-5s %-10s %-18s\r\n", "user-id", "flag", "mac-addr", "olt/port", "onu/port", "switch/port" ); 

		user_id[0] = 0;
		while( trace_userid_tbl_next_get(user_id, pUserTbl) == VOS_OK )
		{
			userid_count++;
			
			pLoc = &(pUserTbl->userLocList[pUserTbl->userCurIdx]);
			
			for( i=0; i<USER_MACADDR_NUM; i++ )
			{
				if( i <= pUserTbl->userCurIdx )
					j = pUserTbl->userCurIdx - i;
				else
					j = USER_MACADDR_NUM + pUserTbl->userCurIdx - i;
					
				pLoc = &(pUserTbl->userLocList[j]);
				if( !MAC_ADDR_IS_INVALID(pLoc->userMac) )
				{
					if( j == pUserTbl->userCurIdx )
					{
                        VOS_MemCpy(print_userid, pUserTbl->userId, 20);
						vty_out( vty, "%-20s %-10s*", print_userid/*pUserTbl->userId*/, trace_path_resolvingflag_2_str(pLoc->resolvingStatus) );
					}
					else
						vty_out( vty, "%-20s %-10s ", "", trace_path_resolvingflag_2_str(pLoc->resolvingStatus) );

					if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
						resolved_count++;
					else
						resolving_count++;

					vty_out( vty, "%-14s ", trace_path_mac_addr_2_str(pLoc->userMac) );

					VOS_Sprintf( tmp, "%d/%d", pLoc->oltBrdIdx, pLoc->oltPortIdx );
					vty_out( vty, "%-6s", tmp );

					vty_out( vty, "%6d/%-3d ", pLoc->onuId, pLoc->onuPortIdx );

					if( MAC_ADDR_IS_INVALID(pLoc->swMacAddr) )
						vty_out( vty, "\r\n" );
					else
					{
						/*VOS_Sprintf( tmp, "%s/%d", trace_path_mac_addr_2_str(pLoc->swMacAddr), pLoc->swPortIdx );
						vty_out( vty, "%-18s\r\n", tmp );*/
						vty_out( vty, "%s/%d\r\n", trace_path_mac_addr_2_str(pLoc->swMacAddr), pLoc->swPortIdx );
					}
				}
			}

			if( (resolving_count + resolved_count) > USER_MAC_SUPPORT_MAXNUM )
			{
				VOS_ASSERT(0);
				break;
			}
			
			VOS_StrCpy( user_id, pUserTbl->userId );
		}

		vty_out( vty, " Total userid count=%d\r\n usermac resolved count=%d,resolving count=%d\r\n", userid_count, resolved_count, resolving_count );
	}
	else if( argc == 1 )
	{
		trace_userid_counter_get( &userid_count, &resolved_count, &resolving_count );

		vty_out( vty, " Userid total count=%d(%d)\r\n usermac resolved count=%d,resolving count=%d\r\n", userid_count, trace_userid_num_get(), resolved_count, resolving_count );
	}
	else
		return CMD_WARNING;
#else
	if( argc == 0 )
	{
		vty_out( vty, "%-22s %-11s %-5s %-10s %-18s\r\n", "user-id", "flag", "olt/port", "onu/port", "switch/port" ); 

		user_id[0] = 0;
		while( trace_userid_tbl_next_get(user_id, pUserTbl) == VOS_OK )
		{
			userid_count++;
			
			pLoc = &(pUserTbl->userLocList[pUserTbl->userCurIdx]);
			
			for( i=0; i<USER_MACADDR_NUM; i++ )
			{
				if( i <= pUserTbl->userCurIdx )
					j = pUserTbl->userCurIdx - i;
				else
					j = USER_MACADDR_NUM + pUserTbl->userCurIdx - i;
					
				pLoc = &(pUserTbl->userLocList[j]);
				if( !MAC_ADDR_IS_INVALID(pLoc->userMac) )
				{
					if( j == pUserTbl->userCurIdx )
					{
                        VOS_MemCpy(print_userid, pUserTbl->userId, 20);
						vty_out( vty, "%-21s %-10s ", print_userid/*pUserTbl->userId*/, trace_path_resolvingflag_2_str(pLoc->resolvingStatus) );
					}
					else
						vty_out( vty, "%-21s %-10s ", "", trace_path_resolvingflag_2_str(pLoc->resolvingStatus) );

					if( pLoc->resolvingStatus == USER_TRACE_RESOLVED_OK )
						resolved_count++;
					else
						resolving_count++;					

					VOS_Sprintf( tmp, "%d/%d", pLoc->oltBrdIdx, pLoc->oltPortIdx );
					vty_out( vty, "%6s ", tmp );

					vty_out( vty, "%6d/%-3d ", pLoc->onuId, pLoc->onuPortIdx );

					if( MAC_ADDR_IS_INVALID(pLoc->swMacAddr) )
						vty_out( vty, "\r\n" );
					else
					{
						/*VOS_Sprintf( tmp, "%s/%d", trace_path_mac_addr_2_str(pLoc->swMacAddr), pLoc->swPortIdx );
						vty_out( vty, "%-18s\r\n", tmp );*/
						vty_out( vty, "%s/%d\r\n", trace_path_mac_addr_2_str(pLoc->swMacAddr), pLoc->swPortIdx );
					}
				}
			}

			if( (resolving_count + resolved_count) > USER_MAC_SUPPORT_MAXNUM )
			{
				VOS_ASSERT(0);
				break;
			}
			
			VOS_StrCpy( user_id, pUserTbl->userId );
		}

		vty_out( vty, " Total userid count=%d\r\n usermac resolved count=%d,resolving count=%d\r\n", userid_count, resolved_count, resolving_count );
	}
	else if( argc == 1 )
	{
		trace_userid_counter_get( &userid_count, &resolved_count, &resolving_count );

		vty_out( vty, " Userid total count=%d(%d)\r\n usermac resolved count=%d,resolving count=%d\r\n", userid_count, trace_userid_num_get(), resolved_count, resolving_count );
	}
	else
		return CMD_WARNING;
#endif
	return CMD_SUCCESS;
}

static LONG trace_path_user_loc_show( struct vty *vty, user_loc_t *pLoc )
{
	ULONG devIdx, ifIdx;
	char name[MAXDEVICENAMELEN];
	ULONG nameLen;
	int i = 1;

	if( pLoc == NULL )
	{
		return CMD_WARNING;
	}
	
	/* tracing OLT */
	if( (pLoc->oltBrdIdx == 0) && (pLoc->oltPortIdx == 0) )
	{
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_NOTFIND) );
		return CMD_WARNING;
	}
	vty_out( vty, "%d.%-10s -- ", i++, "OLT" );
	devIdx = OLT_DEV_ID;
	if( getDeviceName(devIdx, name, &nameLen) == VOS_ERROR )
	{
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_TIMEOUT) );
		return CMD_WARNING;
	}
	if( userport_is_pon(pLoc->oltBrdIdx, pLoc->oltPortIdx) )
	{
		vty_out( vty, "pon%d/%d of %s\r\n", pLoc->oltBrdIdx, pLoc->oltPortIdx, name );
	}
	else
	{
		if( pLoc->oltBrdIdx == 0 )	/* 上联trunk */
		{
			ifIdx= IFM_TRUNK_CREATE_INDEX( pLoc->oltPortIdx - 1 );
			if( IFM_GetIfNameApi(ifIdx, name, 32) != 0 )
			{
				VOS_StrCpy( name, "unkown" );
			}

			vty_out( vty, "trunk(id=%d) %s\r\n", pLoc->oltPortIdx, name );
		}
		else
		{
			vty_out( vty, "eth%d/%d of %s\r\n", pLoc->oltBrdIdx, pLoc->oltPortIdx, name );
		}	
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_COMPLETE) );
		return CMD_SUCCESS;
	}

	/* tracing PON */
	VOS_Sprintf( name, "PON%d/%d", pLoc->oltBrdIdx, pLoc->oltPortIdx );
	vty_out( vty, "%d.%-10s -- ", i++, name );

	if( pLoc->onuId == 0 )
	{
		vty_out( vty, "Not find onu\r\n" );
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_COMPLETE) );
		return CMD_SUCCESS;
	}
	devIdx = MAKEDEVID( pLoc->oltBrdIdx, pLoc->oltPortIdx, pLoc->onuId );
	if( getDeviceName(devIdx, name, &nameLen) == VOS_ERROR )
	{
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_TIMEOUT) );
		return CMD_WARNING;
	}
	vty_out( vty, "onu%d named %s\r\n", pLoc->onuId, name );

	/* tracing ONU */
	VOS_Sprintf( name, "ONU%d/%d/%d", pLoc->oltBrdIdx, pLoc->oltPortIdx, pLoc->onuId );
	vty_out( vty, "%d.%-10s -- ", i++, name );
	if( pLoc->onuPortIdx == 0 )
	{
		vty_out( vty, "Not find onu eth port\r\n" );
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_COMPLETE) );
		return CMD_SUCCESS;
	}
	vty_out( vty, "onu port eth%d/%d\r\n", pLoc->onuBrdIdx, pLoc->onuPortIdx );

	/* tracing onu port */
	if( MAC_ADDR_IS_INVALID(pLoc->swMacAddr) )
	{
		vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_COMPLETE) );
		return CMD_SUCCESS;
	}
	VOS_Sprintf( name, "ETH%d/%d", pLoc->onuBrdIdx, pLoc->onuPortIdx );
	vty_out( vty, "%d.%-10s -- ", i++, name );
	vty_out( vty, "  switch %s\r\n", trace_path_mac_addr_2_str(pLoc->swMacAddr) );

	/* tracing switch*/
	if( pLoc->swPortIdx != 0 )
	{
		vty_out( vty, "%d.%-10s -- ", i++, "SWITCH" );
		vty_out( vty, "switch port %d\r\n", pLoc->swPortIdx );
	}

	vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_COMPLETE) );

	return CMD_SUCCESS;
}

QDEFUN( trace_path_based_userid_show,
	trace_path_based_userid_show_cmd,
	"trace-path userid <userid>",
	"Trace information\n"
	"List user path based user id\n"
	"List user path based user id\n",
	&tracePathQId)
{
	user_loc_t loc;
	
	if( argc == 1 )
	{
		VOS_MemZero( &loc, sizeof(loc) );
		if( trace_userid_location_get(argv[0], &loc) == VOS_OK )
		{
			trace_path_user_loc_show( vty, &loc );
		}
		else
		{
			vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_NOTFIND) );
		}
	}
	return CMD_SUCCESS;
}

QDEFUN( trace_path_based_userid_del,
	trace_path_based_userid_del_cmd,
	"undo trace-path userid {<userid>}*1",
	"Undo\n"
	"Trace information\n"
	"List user path based user id\n"
	"List user path based user id\n",
	&tracePathQId)
{
	LONG status;
	UCHAR user_id[USER_ID_MAXLEN];
	
	if( argc == 1 )
	{
		status = trace_userid_status_get( argv[0] );
		if( (status == USER_TRACE_RESOLVED_OK) || (status == USER_TRACE_RESOLVING_WAIT) )
		{
			trace_userid_tbl_delete( argv[0] );
		}
		else
		{
			vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_NOTFIND) );
		}
	}
    else
    {
        int idx=0;
        trace_userid_table_t *pTbl = NULL;
        trace_userid_table_t *pTbl_cur = NULL;
		user_id[0] = 0;
    	VOS_SemTake( tracePathMSemId, WAIT_FOREVER );
        for(idx=0;idx<TRACE_USERID_HASH_BUCKET;idx++)
        {
        	pTbl = gpTraceUserIdHashTable[idx];
        	if( pTbl == NULL )
        		continue;
        	else
        	{
        		while(pTbl)
        		{
                    pTbl_cur = pTbl;
    				pTbl = pTbl->next;
        			trace_userid_tbl_delete(pTbl_cur->userId);
        		}
        	}
        }
    	VOS_SemGive( tracePathMSemId );
	}
	return CMD_SUCCESS;
}

QDEFUN( trace_path_based_mac_show,
	trace_path_based_mac_show_cmd,
	"trace-path mac-address <H.H.H>",
	"Trace information\n"
	"List user path based user mac addr.\n"
	"List user path based user mac addr.\n",
	&tracePathQId)
{
	user_loc_t loc;
	UCHAR macAddr[USER_MACADDR_LEN];
	
	if( argc == 1 )
	{
		if( VOS_ERROR == GetMacAddr(argv[0], macAddr) )
		{
			vty_out( vty, "  %% Invalid MAC address.\r\n" );
			return CMD_WARNING;
		}

		if( MAC_ADDR_IS_EQUAL(macAddr, SYS_PRODUCT_BASEMAC) )
		{
			vty_out( vty, " OLT sysmac address\r\n" );
			return CMD_SUCCESS;
		}

		VOS_MemZero( &loc, sizeof(user_loc_t) );
		if( trace_mac_location_get(macAddr, &loc) == VOS_OK )	/* 已经侦听到该用户MAC地址 */
		{
			trace_path_user_loc_show( vty, &loc );
			return CMD_SUCCESS;
		}
		else
		{
			if( trace_path_fdb_resolving(macAddr, &loc) == VOS_OK )
			{
				trace_path_user_loc_show( vty, &loc );
				return CMD_SUCCESS;
			}
		}
	}
	vty_out( vty, "%s\r\n", trace_path_result_hint_2_str(TRACE_PATH_RESULT_NOTFIND) );

	return CMD_SUCCESS;
}

QDEFUN( trace_path_userid_capacity,
	trace_path_userid_capacity_cmd,
	"trace-path capacity {<0-65536>}*1",
	"Trace information\n"
	"trace path based user id capacity\n"
	"support user id number, 0 is default\n",
	&tracePathQId)
{
	ULONG num = 0;
	if( argc == 1 )
	{
		num = VOS_AtoL( argv[0] );

		if( num == 0 )
			num = USER_ID_DEFAULT_MAXNUM;
		else if( num > USER_ID_CAPACITY_MAX )
			num = USER_ID_CAPACITY_MAX;
		
		USER_ID_SUPPORT_MAXNUM = num;
	}
	else
		vty_out( vty, " trace-path capacity %d\r\n", USER_ID_SUPPORT_MAXNUM );

	return CMD_SUCCESS;
}

extern ULONG  tracePathResolveRate;
extern ULONG  tracePathTrapRate;

QDEFUN( trace_path_resolve_rate,
	trace_path_resolve_rate_cmd,
	"trace-path resolve-rate {<0-100>}*1",
	"Trace information\n"
	"trace path resolving rate\n"
	"resolving number per second, 0 is default\n",
	&tracePathQId)
{
	ULONG rate = 0;
	if( argc == 1 )
	{
		rate = VOS_AtoL( argv[0] );
		if( rate == 0 )
			rate = TRACE_PATH_RESOLVE_RATE_DEFAULT;
		else if( rate > 100 )
			rate = 100;
		
		tracePathResolveRate = rate;
	}
	else
		vty_out( vty, " trace-path resolving rate %d\r\n", tracePathResolveRate );

	return CMD_SUCCESS;
}

QDEFUN( trace_path_trap_rate,
	trace_path_trap_rate_cmd,
	"trace-path trap-rate {<0-50>}*1",
	"Trace information\n"
	"trace path change trap report rate\n"
	"report number per second\n",
	&tracePathQId)
{
	ULONG rate = 0;
	if( argc == 1 )
	{
		rate = VOS_AtoL( argv[0] );
		if( rate == 0 )
			rate = TRACE_PATH_TRAP_RATE_DEFAULT;
		else if( rate > 50 )
			rate = 50;
		
		tracePathTrapRate = rate;
	}
	else
		vty_out( vty, " trace-path trap rate %d\r\n", tracePathTrapRate );

	return CMD_SUCCESS;
}

QDEFUN( trace_path_manual_sync,
	trace_path_manual_sync_cmd,
	"trace-path sync pon-fdb-entry",
	"Trace information\n"
	"Trace path manual-sync information\n"
	"All pon fdb entry\n",
	&tracePathQId)
{
    if(tracePathAutoSyncEnable == 0)
        vty_out(vty, "Please enable the trace-path auto-sync function!\r\n");
    else
        trace_path_auto_sync_msg_send();
	return CMD_SUCCESS;
}

DEFUN( trace_path_auto_sync_timer,
	trace_path_auto_sync_timer_cmd,
	"trace-path auto-sync-circle {<30-1440>}*1",
	"Trace information\n"
	"Trace path auto-sync circle\n"
	"Please input the circle(min)\n" )
{
	ULONG circle = 0;
	if( argc == 1 )
	{
		circle = VOS_AtoL(argv[0]);
	    tracePathAutoSyncMaxCircle = circle*60;
    }
    else
    {
        vty_out(vty, " trace-path auto-sync circle is %d min!\r\n", tracePathAutoSyncMaxCircle/60);
    }
	return CMD_SUCCESS;
}

DEFUN( trace_path_auto_sync_enable,
	trace_path_auto_sync_enable_cmd,
	"trace-path auto-sync-enable {[0|1]}*1",
	"Trace information\n"
	"Trace path auto-sync enable\n"
	"Disable\n"
	"Enable\n")
{
	ULONG circle = 0;
	if( argc == 1 )
	{
		circle = VOS_AtoL(argv[0]);
	    tracePathAutoSyncEnable = circle;
    }
    else
    {
        vty_out(vty, " trace-path auto-sync is %s!\r\n", tracePathAutoSyncEnable?"enable":"disable");
    }
	return CMD_SUCCESS;
}

DEFUN( trace_path_debug_fun,
	trace_path_debug_cmd,
	"debug trace-path",
	"Debug information\n"
	"Debug user path\n" )
{
	trace_path_debug = 1;
	return CMD_SUCCESS;
}
DEFUN( trace_path_undo_debug_fun,
	trace_path_undo_debug_cmd,
	"undo debug trace-path",
	"Undo\n"
	"Undo debug information\n"
	"Debug user path\n" )
{
	trace_path_debug = 0;
	return CMD_SUCCESS;
}

DEFUN( trace_path_based_vlan_show,
	trace_path_based_vlan_cmd,
	"trace-path vlan <1-4095> <1-4095>",
	"Trace information\n"
	"User path based user vlan.\n"
	"Please input svlan.\n"
	"Please input cvlan.\n")
{
	user_loc_t loc;
	ULONG svlan = 0, cvlan = 0;
    svlan = VOS_AtoL(argv[0]);
    cvlan = VOS_AtoL(argv[1]);

	VOS_MemZero( &loc, sizeof(user_loc_t) );
	if( trace_vlan_location_get(svlan, cvlan, &loc) == VOS_OK )	/* 已经侦听到该用户MAC地址 */
	{
		trace_path_user_loc_show( vty, &loc );
		return CMD_SUCCESS;
	}

	return CMD_SUCCESS;
}

LONG trace_path_show_run( struct vty * vty )
{
	if( USER_ID_SUPPORT_MAXNUM != USER_ID_DEFAULT_MAXNUM )
		vty_out( vty, " trace-path capacity %d\r\n", USER_ID_SUPPORT_MAXNUM );

	if( tracePathResolveRate != TRACE_PATH_RESOLVE_RATE_DEFAULT )
		vty_out( vty, " trace-path resolve-rate %d\r\n", tracePathResolveRate );
	if( tracePathTrapRate != TRACE_PATH_TRAP_RATE_DEFAULT )
		vty_out( vty, " trace-path trap-rate %d\r\n", tracePathTrapRate );
	if( tracePathAutoSyncMaxCircle != TRACE_PATH_AUTO_SYNC_TIME)
        vty_out( vty, " trace-path auto-sync-circle %d\r\n", tracePathAutoSyncMaxCircle/60);
    if(tracePathAutoSyncEnable)
        vty_out( vty, " trace-path auto-sync-enable %d\r\n", tracePathAutoSyncEnable);
	return VOS_OK;
}

LONG trace_path_cli_cmd_install()
{
	install_element ( CONFIG_NODE, &trace_path_list_show_cmd);
	install_element ( CONFIG_NODE, &trace_path_based_userid_show_cmd);
	install_element ( CONFIG_NODE, &trace_path_based_userid_del_cmd);
	install_element ( CONFIG_NODE, &trace_path_based_mac_show_cmd);
	install_element ( VIEW_NODE, &trace_path_based_userid_show_cmd);
	install_element ( VIEW_NODE, &trace_path_based_mac_show_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &trace_path_debug_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &trace_path_undo_debug_cmd);

	install_element ( CONFIG_NODE, &trace_path_manual_sync_cmd);
	install_element ( CONFIG_NODE, &trace_path_auto_sync_timer_cmd);
	install_element ( CONFIG_NODE, &trace_path_auto_sync_enable_cmd);
	install_element ( CONFIG_NODE, &trace_path_based_vlan_cmd);
    
	/*install_element ( CONFIG_NODE, &trace_path_resolve_rate_cmd);*/
	install_element ( CONFIG_NODE, &trace_path_trap_rate_cmd);

	return VOS_OK;
}

#pragma pack()


#ifdef __cplusplus
}
#endif

