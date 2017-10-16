#include "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#include "gwEponSys.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include "lib_ethPortMib.h"
#include	"lib_gwEponMib.h"

#include "CT_RMan_Main.h"
#include  "../cli/Olt_cli.h"

#include "CT_Rman_Multicast.h"


#if 0
extern CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];
#else
extern CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPONNOLIMIT][MAX_ONU_ETHPORT];
#endif
static eth_ctc_multicast_vlan_switch_list_t *pEthCtcMVlanSwitchHead = NULL;


eth_ctc_multicast_vlan_switch_list_t *CTC_addCtcMVlanSwitchNode( const ULONG grpId )
{
	eth_ctc_multicast_vlan_switch_list_t *pCur = pEthCtcMVlanSwitchHead;
	eth_ctc_multicast_vlan_switch_list_t *pPre = NULL;
	eth_ctc_multicast_vlan_switch_list_t *pNewNode;

	if( grpId > CTC_MULTICAST_VLAN_SWITCH_ID_MAX )
		return NULL;
	
	while(  pCur!= NULL )
	{
		if( grpId < pCur->ctcMVlanSwitchGroupId )
		{
			break;
		}
		else if(pCur->ctcMVlanSwitchGroupId == grpId )
		{
			return pCur;
		}
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	pNewNode = VOS_Malloc( sizeof(eth_ctc_multicast_vlan_switch_list_t), MODULE_RPU_ONU );
		
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return pNewNode;
	}

	VOS_MemSet( pNewNode, 0, sizeof(eth_ctc_multicast_vlan_switch_list_t) );
	pNewNode->ctcMVlanSwitchGroupId = grpId;

	if( pPre != NULL )
	{
		pNewNode->pNextNode = pPre->pNextNode;
		pPre->pNextNode = pNewNode;
	}
	else
	{
		if( pCur == NULL )
		{
			if(  pEthCtcMVlanSwitchHead == NULL )
			{
				pEthCtcMVlanSwitchHead = pNewNode;
			}
			else
			{
				sys_console_printf(" ## %s %d\r\n", __FILE__, __LINE__);
				return NULL;
			}
		}
		else
		{
			pNewNode->pNextNode = pEthCtcMVlanSwitchHead;
			pEthCtcMVlanSwitchHead = pNewNode;
		}
	}
	
	return pNewNode;
}

STATUS CTC_delCtcMVlanSwitchNode( const ULONG grpId )
{
	STATUS rc = VOS_OK;
	eth_ctc_multicast_vlan_switch_list_t *pCur = pEthCtcMVlanSwitchHead;
	eth_ctc_multicast_vlan_switch_list_t *pPre = NULL;

	while( pCur )
	{
		if( grpId == pCur->ctcMVlanSwitchGroupId )
			break;
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	if( pCur != NULL )
	{
		if( pPre == NULL )
		{
			pEthCtcMVlanSwitchHead = pCur->pNextNode;
		}
		else
		{
			pPre->pNextNode = pCur->pNextNode;
		}
		VOS_Free( pCur );
		pCur = NULL;
		rc = VOS_OK;
	}
	else
		rc = VOS_ERROR;

	/* 扫描ONU 删除 */
	/*if( rc == VOS_OK )
	{
	}*/
	
	return rc;
}

eth_ctc_multicast_vlan_switch_list_t *CTC_findCtcMVlanSwitchNode( const ULONG grpId )
{
	eth_ctc_multicast_vlan_switch_list_t *pCur = pEthCtcMVlanSwitchHead;

	if( grpId == 0 )
		return NULL;
	while(  pCur!= NULL )
	{
		if(pCur->ctcMVlanSwitchGroupId == grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}

eth_ctc_multicast_vlan_switch_list_t *CTC_findNextCtcMVlanSwitchNode( const ULONG grpId )
{
	eth_ctc_multicast_vlan_switch_list_t *pCur = pEthCtcMVlanSwitchHead;

	while(  pCur!= NULL )
	{
		if(pCur->ctcMVlanSwitchGroupId > grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}

extern int vlan_list_in_ascending_sort( USHORT *pArray, ULONG maxNum );
int mvlan_switch_list_in_ascending_sort( eth_ctc_multicast_vlan_switch_list_t *pNode )
{
	int i, j, n;
	USHORT m_vlan;
	USHORT u_vlan;

	n=0;
	if( pNode->multicast_vlan[n] == 0 )
	{
		m_vlan = 0;
		u_vlan = 0;
	}
	else
	{
		m_vlan = pNode->multicast_vlan[n];
		u_vlan = pNode->iptv_user_vlan[n];
	}
	
	for( i=1; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
	{
		if( (pNode->multicast_vlan[i] > 4094) || (pNode->iptv_user_vlan[i] > 4094) )
		{
			pNode->multicast_vlan[i] = 0;
			pNode->iptv_user_vlan[i] = 0;
		}

		if( pNode->multicast_vlan[i] )
		{
			pNode->multicast_vlan[n] = pNode->multicast_vlan[i];
			pNode->iptv_user_vlan[n] = pNode->iptv_user_vlan[i];
			
			pNode->multicast_vlan[i] = 0;
			pNode->iptv_user_vlan[i] = 0;
			n++;
		}
	}
	
	pNode->multicast_vlan[n] = m_vlan;
	pNode->iptv_user_vlan[n] = u_vlan;
	if( (m_vlan == 0) && (n > 0) )
		n--;

	for( i=0; i<n; i++ )
	{
		for( j=0; j<n-i; j++ )
		{
			if( (pNode->multicast_vlan[j] == 0) ||
				((pNode->multicast_vlan[j] > pNode->multicast_vlan[j+1]) && (pNode->multicast_vlan[j+1] != 0)) )
			{
				m_vlan = pNode->multicast_vlan[j];
				u_vlan = pNode->iptv_user_vlan[j];
				
				pNode->multicast_vlan[j] = pNode->multicast_vlan[j+1];
				pNode->iptv_user_vlan[j] = pNode->iptv_user_vlan[j+1];
				
				pNode->multicast_vlan[j+1] = m_vlan;
				pNode->iptv_user_vlan[j+1] = u_vlan;
			}
		}
	}

	return VOS_OK;
}

int CTC_addMVlanSwitch( ULONG grpId, ULONG multiVid, ULONG userVid )
{
	STATUS rc = VOS_OK;
	int i;
	int i_bak = -1;

	eth_ctc_multicast_vlan_switch_list_t *pNode;
		
	if( (grpId == 0) || (grpId > CTC_MULTICAST_VLAN_SWITCH_ID_MAX) )
		return VOS_ERROR;

	pNode = (eth_ctc_multicast_vlan_switch_list_t *)CTC_addCtcMVlanSwitchNode( grpId );
	if( pNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
	{
		if( pNode->multicast_vlan[i] == 0 )
		{
			if( i_bak == -1 ) i_bak = i;
		}
		else if( pNode->multicast_vlan[i] == multiVid )
		{
			i_bak = i;
			break;
		}
	}
	if( i_bak == -1 )
		rc = VOS_ERROR;

	if( rc == VOS_OK )
	{
		pNode->multicast_vlan[i_bak] = multiVid;
		pNode->iptv_user_vlan[i_bak] = userVid;
		mvlan_switch_list_in_ascending_sort( pNode );
	}
	
	return rc;
}

STATUS CTC_delMVlanSwitch( ULONG grpId, ULONG multiVid, ULONG userVid )
{
	STATUS rc = VOS_ERROR;
	int i;

	eth_ctc_multicast_vlan_switch_list_t *pNode;
		
	pNode = (eth_ctc_multicast_vlan_switch_list_t *)CTC_findCtcMVlanSwitchNode( grpId );
	if( pNode == NULL )
	{
		return rc;
	}

	for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
	{
		if( pNode->multicast_vlan[i] == multiVid )
		{
			pNode->multicast_vlan[i] = 0;
			pNode->iptv_user_vlan[i] = 0;
			rc = VOS_OK;
		}
	}
	if( rc == VOS_OK )
	{
		mvlan_switch_list_in_ascending_sort( pNode );
	}

	return rc;
}

int CTC_getEthPortMVlanSwitchGroup( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethCtcMVlanSwitchSelect;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_setEthPortMVlanSwitchGroup( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t  llid;
    CTC_management_object_t mid;
	CTC_management_object_index_t *midx;
	CTC_STACK_multicast_vlan_switching_t mvlan_switch;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > CTC_MULTICAST_VLAN_SWITCH_ID_MAX )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;

	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == INVALID_LLID )
		return RERROR;

    mid.leaf = CTC_MANAGEMENT_OBJECT_LEAF_PORT;
    midx = &mid.index;
	midx->frame_number = 0;
	midx->slot_number = 0;
	midx->port_number = portIdx;
	midx->port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	VOS_MemZero( &mvlan_switch, sizeof(mvlan_switch) );
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if( OnuMgt_SetObjMulticastTagOper(olt_id, onu_id, &mid, CTC_TRANSLATE_VLAN_TAG, &mvlan_switch) != CTC_STACK_EXIT_OK )
	{
		sys_console_printf(" MulticastTagOper err\r\n" );
	}
		
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethCtcMVlanSwitchSelect = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int mn_getFirstCtcMVlanSwitchEntryIndex( ULONG *pGrpId, ULONG *pMultiVid, ULONG *pUserVid )
{
	eth_ctc_multicast_vlan_switch_list_t *pNode = pEthCtcMVlanSwitchHead;
	int i;

	if( (pGrpId == NULL) || (pMultiVid == NULL) || (pUserVid == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcMVlanSwitchGroupId )
		{
			for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
			{
				if( pNode->multicast_vlan[i] )
				{
					*pGrpId = pNode->ctcMVlanSwitchGroupId;
					*pMultiVid = pNode->multicast_vlan[i];
					*pUserVid = pNode->iptv_user_vlan[i];
					return VOS_OK;
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_getNextCtcMVlanSwitchEntryIndex( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG *pNextGrpId, ULONG *pNextMultiVid, ULONG *pNextUserVid )
{
	eth_ctc_multicast_vlan_switch_list_t *pNode = pEthCtcMVlanSwitchHead;
	int i;

	if( (pNextGrpId == NULL) || (pNextMultiVid == NULL) || (pNextUserVid == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcMVlanSwitchGroupId )
		{
			if( pNode->ctcMVlanSwitchGroupId == grpId )
			{
				for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
				{
					if( pNode->multicast_vlan[i] > multiVid )
					{
						*pNextGrpId = pNode->ctcMVlanSwitchGroupId;
						*pNextMultiVid = pNode->multicast_vlan[i];
						*pNextUserVid = pNode->iptv_user_vlan[i];
						return VOS_OK;
					}
				}
			}

			if( pNode->ctcMVlanSwitchGroupId > grpId )
			{
				for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
				{
					if( pNode->multicast_vlan[i] )
					{
						*pNextGrpId = pNode->ctcMVlanSwitchGroupId;
						*pNextMultiVid = pNode->multicast_vlan[i];
						*pNextUserVid = pNode->iptv_user_vlan[i];
						return VOS_OK;
					}
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_checkCtcMVlanSwitchEntryIndex( ULONG grpId, ULONG multiVid, ULONG userVid )
{
	int i;
	eth_ctc_multicast_vlan_switch_list_t *pNode = pEthCtcMVlanSwitchHead;

	if( (CTC_MULTICAST_VLAN_SWITCH_ID_MAX < grpId) || (0 == multiVid) || (4094 < multiVid) || (4094 < userVid) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcMVlanSwitchGroupId == grpId )
		{
			for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
			{
				if( (pNode->multicast_vlan[i] == multiVid) && (pNode->iptv_user_vlan[i] == userVid) )
				{
					return VOS_OK;
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}


int mn_getCtcMVlanSwitchEntryStatus( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG *pStatus )
{
	if( pStatus == NULL )
		return VOS_ERROR;
	
	if( mn_checkCtcMVlanSwitchEntryIndex(grpId, multiVid, userVid) == VOS_OK )
		*pStatus = RS_ACTIVE;

	return VOS_ERROR;
}

int mn_setCtcMVlanSwitchEntryStatus( ULONG grpId, ULONG multiVid, ULONG userVid, ULONG status )
{
	int rc = VOS_ERROR;

	if( (CTC_MULTICAST_VLAN_SWITCH_ID_MAX < grpId) || (0 == multiVid) || (4094 < multiVid) || (4094 < userVid) )
		return VOS_ERROR;

	if( (status == RS_CREATEANDGO) || (status == RS_CREATEANDWAIT) )
	{
		rc = CTC_addMVlanSwitch( grpId, multiVid, userVid );
	}
	else if( status == RS_DESTROY )
	{
		rc = CTC_delMVlanSwitch( grpId, multiVid, userVid );
	}
	else if( status == RS_ACTIVE )
	{
	}
	else
	{
	}
	return rc;
}

int mn_getEthPortMVlanSwitchGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pGrpId)
{
	ULONG id = 0;
	if( NULL == pGrpId )
		return VOS_ERROR;
	
	if( CTC_getEthPortMVlanSwitchGroup(devIdx, portIdx, &id) == VOS_OK )
	{
		if( CTC_findCtcMVlanSwitchNode(id) == NULL )
			*pGrpId = 0;
		else
			*pGrpId = id;
			
		return VOS_OK;
	}
	return VOS_ERROR;
}
int mn_setEthPortMVlanSwitchGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId)
{
	if( CTC_findCtcMVlanSwitchNode( grpId ) == NULL )
	{
		return VOS_ERROR;
	}
	return CTC_setEthPortMVlanSwitchGroup( devIdx, portIdx, grpId );
}


DEFUN(mvlan_switch_group_add,
	mvlan_switch_group_add_cmd,
       "ctc mvlan-switch add <1-10000> <1-4094> <1-4094>",
	CTC_STR
	"multicase vlan switch\n"
	"add multicast vlan-switch group\n"
	"input mvlan switch-group id\n"
	"input multicast vlan id\n"
	"input iptv user vlan id\n"
	)
{
	ULONG grpId;
       ULONG multiVid;
	ULONG userVid;
	   
	grpId=VOS_AtoI( argv[0] );
	multiVid=VOS_AtoI( argv[1] );
	userVid=VOS_AtoI( argv[2] );

	if(mn_setCtcMVlanSwitchEntryStatus( grpId, multiVid, userVid, RS_CREATEANDGO ) == VOS_ERROR)
	{
		vty_out( vty, "mvlan-switch err\r\n" );
		return  CMD_WARNING;
	}
	
	return  CMD_SUCCESS;
}



DEFUN(mvlan_switch_group_del,
	mvlan_switch_group_del_cmd,
       "ctc mvlan-switch delete <1-10000> {<1-4094> <1-4094>}*1",
	CTC_STR
	"multicast vlan switch\n"
	"delete multicast vlan-switch\n"
	"input mvlan switch-group id\n"
	"input multicast vlan id\n"
	"input iptv user vlan id\n"
	)
{
	ULONG grpId;
       ULONG multiVid;
       ULONG userVid;
	   
	grpId = VOS_AtoI( argv[0] );

	if( argc == 1 )
	{
		if( CTC_delCtcMVlanSwitchNode(grpId) == VOS_ERROR )
		{
			vty_out(vty, "mvlan-switch group delete error!\r\n");        
			return CMD_WARNING;
		}
	}
	else
	{
		multiVid = VOS_AtoI( argv[1] );
		userVid = VOS_AtoI( argv[2] );
		if( mn_setCtcMVlanSwitchEntryStatus( grpId, multiVid, userVid, RS_DESTROY) == VOS_ERROR )
		{
			vty_out(vty, "mvlan-switch delete error!\r\n");        
			return CMD_WARNING;
		}
	}

	return CMD_SUCCESS;
}

DEFUN(mvlan_switch_map,
	mvlan_switch_map_cmd,
	"ctc mvlan-switch map <1-10000> [all|<port_list>]",
	CTC_STR
	"multicast vlan switch\n"
	"map port\n"
	"input mvlan switch-group id\n"
	"All port\n"
	"input onu port number\n"
	)
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
	ULONG pon;
	ULONG onu;

	ULONG portNum;
	ULONG grpId;

	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	grpId = VOS_AtoI( argv[0] );
	if( CTC_findCtcMVlanSwitchNode(grpId) == NULL )
	{
		vty_out(vty, "mvlan switch-group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortMVlanSwitchGrpId( devIdx, ethIdx, grpId) == VOS_ERROR)
				vty_out(vty, "mvlan switch-group-map port=%d error!\r\n", ethIdx);
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ethIdx )
		if(ethIdx>portNum)
		{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();*/	/* removed by xieshl 20120906, 解决内存丢失问题，同时非法端口不处理，但不报错，下同 */

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ethIdx )
		if( ethIdx <= portNum )
		{
			if(mn_setEthPortMVlanSwitchGrpId( devIdx, ethIdx, grpId)==VOS_ERROR)
				vty_out(vty, "mvlan switch-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}



DEFUN(undo_mvlan_switch_map,
	undo_mvlan_switch_map_cmd,
	"undo ctc mvlan-switch map <1-10000> [all|<port_list>]",
	"undo operation\n"
	CTC_STR
	"undo multicast vlan switch\n"
	"map port\n"
	"input vlan mvlan switch-group id\n"
	"All port\n"
	"input onu port number\n"
	)
	 
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
 	ULONG pon;
	ULONG onu;
   
	ULONG portNum;
	ULONG grpId;
	   
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	grpId = VOS_AtoI( argv[0] );
	if( CTC_findCtcMVlanSwitchNode(grpId) == NULL )
	{
		vty_out(vty, "mvlan switch-group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortMVlanSwitchGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "mvlan switch-group-map port=%d error!\r\n", ethIdx);
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ethIdx )
		if(ethIdx > portNum)
		{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();*/

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ethIdx )
		if( ethIdx <= portNum )
		{
			if(mn_setEthPortMVlanSwitchGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "mvlan switch-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}

static VOID showMVlanSwitchGroupNode( struct vty *vty, eth_ctc_multicast_vlan_switch_list_t *pNode )
{
	int i, n = 0;

	if( pNode )
	{
		vty_out( vty, "  %-10d", pNode->ctcMVlanSwitchGroupId );

		for( i=0; i<CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES; i++ )
		{
			if( pNode->multicast_vlan[i] )
			{
				if( n == 0 )
					vty_out( vty, " %-15d %d\r\n", pNode->multicast_vlan[i], pNode->iptv_user_vlan[i] );
				else
					vty_out( vty, "%-13s%-15d %d\r\n", " ", pNode->multicast_vlan[i], pNode->iptv_user_vlan[i] );

				n++;
			}
		}
		if( n == 0 ) vty_out( vty, "\r\n" );
	}
}
VOID showMVlanSwitchGroup( struct vty *vty, ULONG grpId )
{
	eth_ctc_multicast_vlan_switch_list_t *pNode = pEthCtcMVlanSwitchHead;

	vty_out( vty, "\r\n group-id  multicast-vlan  user-vlan\r\n" );
	while( pNode )
	{
		if( grpId )
		{
			if( pNode->ctcMVlanSwitchGroupId == grpId )
			{
				showMVlanSwitchGroupNode( vty, pNode );
			}
		}
		else
		{
			if( pNode->ctcMVlanSwitchGroupId )
			{
				showMVlanSwitchGroupNode( vty, pNode );
			}
		}
		pNode = pNode->pNextNode;
	}
	vty_out( vty, "\r\n" );
}

DEFUN(mvlan_switch_group_show,
	mvlan_switch_group_show_cmd,
       "show ctc mvlan-switch {<1-10000>}*1",
	SHOW_STR
	CTC_STR
	"multicast vlan switch\n"
	"input mvlan switch-group id\n"
	)
{
	if( argc == 0 )
	{
		showMVlanSwitchGroup( vty, 0 );
	}
	else
	{
		showMVlanSwitchGroup( vty, VOS_AtoI( argv[0] ) );
	}
	
	return  CMD_SUCCESS;
}
DEFUN(mvlan_switch_map_show,
	mvlan_switch_map_show_cmd,
       "show ctc mvlan-switch map",
	SHOW_STR
	CTC_STR
	"multicast vlan switch\n"
	"mvlan switch port map\n"
	)
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
 	ULONG pon;
	ULONG onu;
	ULONG val;
	PON_olt_id_t olt_id = 0;
	PON_onu_id_t onu_id = 0;
   
	ULONG portNum;

	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;

	vty_out(vty, "\r\n portNo   switchGroup\r\n" );
	for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
	{
		val = 0;
		mn_getEthPortMVlanSwitchGrpId( devIdx, ethIdx, &val ); 
		if( val != 0 )
			vty_out(vty, "  %-9d%-8d\r\n", ethIdx, val );
		else
			vty_out(vty, "  %-9d%-8s\r\n", ethIdx, "-" );
	}

	return  CMD_SUCCESS;
}

LONG  CT_MVlanSwitchCli_Init()
{
	install_element ( CONFIG_NODE, &mvlan_switch_group_add_cmd);
	install_element ( CONFIG_NODE, &mvlan_switch_group_del_cmd);
	install_element ( CONFIG_NODE, &mvlan_switch_group_show_cmd);

	install_element ( ONU_CTC_NODE, &mvlan_switch_group_add_cmd);
	install_element ( ONU_CTC_NODE, &mvlan_switch_group_del_cmd);
	install_element ( ONU_CTC_NODE, &mvlan_switch_group_show_cmd);
	
	install_element ( ONU_CTC_NODE, &mvlan_switch_map_cmd);
	install_element ( ONU_CTC_NODE, &undo_mvlan_switch_map_cmd);
	install_element ( ONU_CTC_NODE, &mvlan_switch_map_show_cmd);
	
	return VOS_OK;
}
#endif

/*----------------------------------------------------------------------------*/

/* CLI */

/*----------------------------------------------------------------------------*/


