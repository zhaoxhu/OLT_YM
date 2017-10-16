#include "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#include "gwEponSys.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include "lib_ethPortMib.h"
#include	"lib_gwEponMib.h"

#include "CT_RMan_Main.h"
#include  "../cli/Olt_cli.h"

#include "CT_Rman_VLAN.h"


#if 0
extern CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];
#else
extern CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPONNOLIMIT][MAX_ONU_ETHPORT];
#endif
static eth_ctc_vlan_trunk_list_t *pEthCtcVlanTrunkHead = NULL;


eth_ctc_vlan_trunk_list_t *CTC_addCtcVlanTrunkNode( const ULONG grpId )
{
	eth_ctc_vlan_trunk_list_t *pCur = pEthCtcVlanTrunkHead;
	eth_ctc_vlan_trunk_list_t *pPre = NULL;
	eth_ctc_vlan_trunk_list_t *pNewNode;

	if( grpId > CTC_VLAN_TRUNK_ID_MAX )
		return NULL;
	
	while(  pCur!= NULL )
	{
		if( grpId < pCur->ctcVlanTrunkGroupId )
		{
			break;
		}
		else if(pCur->ctcVlanTrunkGroupId == grpId )
		{
			return pCur;
		}
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	pNewNode = VOS_Malloc( sizeof(eth_ctc_vlan_trunk_list_t), MODULE_RPU_ONU );
		
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return pNewNode;
	}

	VOS_MemZero( pNewNode, sizeof(eth_ctc_vlan_trunk_list_t) );
	pNewNode->ctcVlanTrunkGroupId = grpId;

	if( pPre != NULL )
	{
		pNewNode->pNextNode = pPre->pNextNode;
		pPre->pNextNode = pNewNode;
	}
	else
	{
		if( pCur == NULL )
		{
			if(  pEthCtcVlanTrunkHead == NULL )
			{
				pEthCtcVlanTrunkHead = pNewNode;
			}
			else
			{
				sys_console_printf(" ## %s %d\r\n", __FILE__, __LINE__);
				return NULL;
			}
		}
		else
		{
			pNewNode->pNextNode = pEthCtcVlanTrunkHead;
			pEthCtcVlanTrunkHead = pNewNode;
		}
	}
	
	return pNewNode;
}

STATUS	CTC_delCtcVlanTrunkNode( const ULONG grpId )
{
	STATUS rc = VOS_OK;
	eth_ctc_vlan_trunk_list_t *pCur = pEthCtcVlanTrunkHead;
	eth_ctc_vlan_trunk_list_t *pPre = NULL;

	while( pCur )
	{
		if( grpId == pCur->ctcVlanTrunkGroupId )
			break;
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	if( pCur != NULL )
	{
		if( pPre == NULL )
		{
			pEthCtcVlanTrunkHead = pCur->pNextNode;
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

eth_ctc_vlan_trunk_list_t *CTC_findCtcVlanTrunkNode( const ULONG grpId )
{
	eth_ctc_vlan_trunk_list_t *pCur = pEthCtcVlanTrunkHead;

	if( grpId == 0 )
		return NULL;
	while(  pCur!= NULL )
	{
		if(pCur->ctcVlanTrunkGroupId == grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}
eth_ctc_vlan_trunk_list_t *CTC_findNextCtcVlanTrunkNode( const ULONG grpId )
{
	eth_ctc_vlan_trunk_list_t *pCur = pEthCtcVlanTrunkHead;

	while(  pCur!= NULL )
	{
		if(pCur->ctcVlanTrunkGroupId > grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}

int vlan_list_in_ascending_sort( USHORT *pArray, ULONG maxNum )
{
	int i, j, n;
	USHORT temp;

	n=0;
	temp = pArray[0];
	for( i=1; i<maxNum; i++ )
	{
		if( pArray[i] > 4094 )
			pArray[i] = 0;
		
		if( pArray[i] )
		{
			pArray[n] = pArray[i];
			pArray[i] = 0;
			n++;
		}
	}

	pArray[n] = temp;
	if( (pArray[n] == 0) && (n > 0) )
		n--;
	
	for( i=0; i<n; i++ )
	{
		for( j=0; j<n-i; j++ )
		{
			if( (pArray[j] == 0) || ((pArray[j] > pArray[j+1]) && (pArray[j+1] != 0)) )
			{
				temp = pArray[j];
				pArray[j] = pArray[j+1];
				pArray[j+1] = temp;
			}
			/*else if( (pArray[j] > pArray[j+1]) && (pArray[j+1] != 0) )
			{
				temp = pArray[j];
				pArray[j] = pArray[j+1];
				pArray[j+1] = temp;
			}*/
		}
	}

	return VOS_OK;
}

int CTC_addVlanTrunk( ULONG grpId, ULONG trunkVid)
{
	STATUS rc = VOS_OK;
	int i;
	int i_bak = -1;

	eth_ctc_vlan_trunk_list_t *pNode;
		
	if( (grpId == 0) || (grpId > CTC_VLAN_TRUNK_ID_MAX) )
		return VOS_ERROR;

	pNode = (eth_ctc_vlan_trunk_list_t *)CTC_addCtcVlanTrunkNode( grpId );
	if( pNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
	{
		if( pNode->ctcVlanTrunkEntries[i] == 0 )
		{
			if( i_bak == -1 ) i_bak = i;
		}
		else if( pNode->ctcVlanTrunkEntries[i] == trunkVid )
		{
			i_bak = i;
			break;
		}
	}
	if( i_bak == -1 )
		rc = VOS_ERROR;

	if( rc == VOS_OK )
	{
		pNode->ctcVlanTrunkEntries[i_bak] = trunkVid;
		vlan_list_in_ascending_sort( pNode->ctcVlanTrunkEntries, CTC_MAX_VLAN_FILTER_ENTRIES );
	}
	
	return rc;
}

STATUS	CTC_delVlanTrunk( ULONG grpId, ULONG trunkVid )
{
	STATUS rc = VOS_ERROR;
	int i;

	eth_ctc_vlan_trunk_list_t *pNode;
		
	pNode = (eth_ctc_vlan_trunk_list_t *)CTC_findCtcVlanTrunkNode( grpId );
	if( pNode == NULL )
	{
		return rc;
	}

	for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
	{
		if( pNode->ctcVlanTrunkEntries[i] == trunkVid )
		{
			pNode->ctcVlanTrunkEntries[i] = 0;
			rc = VOS_OK;
		}
	}
	if( rc == VOS_OK )
	{
		vlan_list_in_ascending_sort( pNode->ctcVlanTrunkEntries, CTC_MAX_VLAN_FILTER_ENTRIES );
	}

	return rc;
}

int CTC_getEthPortVlanTrunkGroup( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
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
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethCtcVlanTrunkSelect;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_setEthPortVlanTrunkGroup( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > CTC_VLAN_TRUNK_ID_MAX )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethCtcVlanTrunkSelect = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int mn_getFirstCtcVlanTrunkEntryIndex( ULONG *pGrpId, ULONG *pVlanId )
{
	eth_ctc_vlan_trunk_list_t *pNode = pEthCtcVlanTrunkHead;
	int i;

	if( (pGrpId == NULL) || (pVlanId == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanTrunkGroupId )
		{
			for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
			{
				if( pNode->ctcVlanTrunkEntries[i] )
				{
					*pGrpId = pNode->ctcVlanTrunkGroupId;
					*pVlanId = pNode->ctcVlanTrunkEntries[i];
					return VOS_OK;
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_getNextCtcVlanTrunkEntryIndex( ULONG grpId, ULONG trunkVid, ULONG *pNextGrpId, ULONG *pNextTrunkVid )
{
	eth_ctc_vlan_trunk_list_t *pNode = pEthCtcVlanTrunkHead;
	int i;

	if( (pNextGrpId == NULL) || (pNextTrunkVid == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanTrunkGroupId )
		{
			if( pNode->ctcVlanTrunkGroupId == grpId )
			{
				for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
				{
					if( pNode->ctcVlanTrunkEntries[i] > trunkVid )
					{
						*pNextGrpId = pNode->ctcVlanTrunkGroupId;
						*pNextTrunkVid = pNode->ctcVlanTrunkEntries[i];
						return VOS_OK;
					}
				}
			}

			if( pNode->ctcVlanTrunkGroupId > grpId )
			{
				for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
				{
					if( pNode->ctcVlanTrunkEntries[i] )
					{
						*pNextGrpId = pNode->ctcVlanTrunkGroupId;
						*pNextTrunkVid = pNode->ctcVlanTrunkEntries[i];
						return VOS_OK;
					}
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_checkCtcVlanTrunkEntryIndex( ULONG grpId, ULONG trunkVid )
{
	int i;
	eth_ctc_vlan_trunk_list_t *pNode = pEthCtcVlanTrunkHead;

	if( (CTC_VLAN_TRUNK_ID_MAX < grpId) || (0 == trunkVid) || (4094 < trunkVid) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanTrunkGroupId == grpId )
		{
			for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
			{
				if( pNode->ctcVlanTrunkEntries[i] == trunkVid )
				{
					return VOS_OK;
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}


int mn_getCtcVlanTrunkEntryStatus( ULONG grpId, ULONG trunkVid, ULONG *pStatus )
{
	if( pStatus == NULL )
		return VOS_ERROR;
	
	if( mn_checkCtcVlanTrunkEntryIndex(grpId, trunkVid) == VOS_OK )
		*pStatus = RS_ACTIVE;

	return VOS_ERROR;
}

int mn_setCtcVlanTrunkEntryStatus( ULONG grpId, ULONG trunkVid, ULONG status )
{
	int rc = VOS_ERROR;

	if( (CTC_VLAN_TRUNK_ID_MAX < grpId) || (0 == trunkVid) || (4094 < trunkVid) )
		return VOS_ERROR;

	if( (status == RS_CREATEANDGO) || (status == RS_CREATEANDWAIT) )
	{
		rc = CTC_addVlanTrunk( grpId, trunkVid );
	}
	else if( status == RS_DESTROY )
	{
		rc = CTC_delVlanTrunk( grpId, trunkVid );
	}
	else if( status == RS_ACTIVE )
	{
	}
	else
	{
	}
	return rc;
}

int mn_getEthPortVlanTrunkGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pTrunkId)
{
	ULONG id = 0;
	if( CTC_getEthPortVlanTrunkGroup(devIdx, portIdx, &id) == VOS_OK )
	{
		if( CTC_findCtcVlanTrunkNode(id) == NULL )
			*pTrunkId = 0;
		else
			*pTrunkId = id;
			
		return VOS_OK;
	}
	return VOS_ERROR;
}
int mn_setEthPortVlanTrunkGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId)
{
	if( CTC_findCtcVlanTrunkNode( grpId ) == NULL )
	{
		return VOS_ERROR;
	}
	return CTC_setEthPortVlanTrunkGroup( devIdx, portIdx, grpId );
}


DEFUN(vlan_trunk_group_add,
	vlan_trunk_group_add_cmd,
       "ctc vlan-trunk add <1-10000> <1-4094>",
	CTC_STR
	"vlan trunk\n"
	"add vlan-trunk group\n"
	"input vlan trunk-group id\n"
	"input trunk vlan id\n"
	)
{
	ULONG grpId;
       ULONG trunkVlanId;
	   
	grpId=VOS_AtoI( argv[0] );
	trunkVlanId=VOS_AtoI( argv[1] );

	if(mn_setCtcVlanTrunkEntryStatus( grpId, trunkVlanId, RS_CREATEANDGO ) == VOS_ERROR)
	{
		vty_out( vty, "vlan-trunk err\r\n" );
		return  CMD_WARNING;
	}
	
	return  CMD_SUCCESS;
}

DEFUN(vlan_trunk_group_del,
	vlan_trunk_group_del_cmd,
       "ctc vlan-trunk delete <1-10000> {<1-4094>}*1",
	CTC_STR
	"vlan trunk\n"
	"delete vlan-trunk items\n"
	"input vlan trunk-group id\n"
	"input trunk vlan id\n"
	)
{
	ULONG grpId;
       ULONG trunkVlanId;
	   
	grpId = VOS_AtoI( argv[0] );

	if( argc == 1 )
	{
		if( CTC_delCtcVlanTrunkNode(grpId) == VOS_ERROR )
		{
			vty_out(vty, "vlan-trunk group delete error!\r\n");        
			return CMD_WARNING;
		}
	}
	else
	{
		trunkVlanId = VOS_AtoI( argv[1] );
		if( mn_setCtcVlanTrunkEntryStatus( grpId, trunkVlanId, RS_DESTROY) == VOS_ERROR )
		{
			vty_out(vty, "vlan-trunk delete error!\r\n");        
			return CMD_WARNING;
		}
	}

	return CMD_SUCCESS;
}

DEFUN(vlan_trunk_map,
	vlan_trunk_map_cmd,
       "ctc vlan-trunk map <1-10000> [all|<port_list>]",
	CTC_STR
	"vlan trunk\n"
	"map port\n"
	"input vlan trunk-group id\n"
	"All port\n"
	"input onu port number\n")
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
	if( CTC_findCtcVlanTrunkNode(grpId) == NULL )
	{
		vty_out(vty, "vlan trunk-group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortVlanTrunkGrpId( devIdx, ethIdx, grpId) == VOS_ERROR)
				vty_out(vty, "trunk-group-map port=%d error!\r\n", ethIdx);
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
			if(mn_setEthPortVlanTrunkGrpId( devIdx, ethIdx, grpId)==VOS_ERROR)
				vty_out(vty, "trunk-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}


DEFUN(undo_vlan_trunk_map,
	undo_vlan_trunk_map_cmd,
	"undo ctc vlan-trunk map <1-10000> [all|<port_list>]",
	"undo operation\n"
	CTC_STR
	"undo vlan trunk\n"
	"map port\n"
	"input vlan trunk-group id\n"
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
	if( CTC_findCtcVlanTrunkNode(grpId) == NULL )
	{
		vty_out(vty, "vlan trunk-group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortVlanTrunkGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "vlan trunk-group-map port=%d error!\r\n", ethIdx);
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
			if(mn_setEthPortVlanTrunkGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "vlan trunk-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}

static VOID showVlanTrunkGroupNode( struct vty *vty, eth_ctc_vlan_trunk_list_t *pNode )
{
	int i;

	if( pNode )
	{
		vty_out( vty, "  %-10d", pNode->ctcVlanTrunkGroupId );

		for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
		{
			if( pNode->ctcVlanTrunkEntries[i] )
			{
				vty_out( vty, " %d", pNode->ctcVlanTrunkEntries[i] );
			}
		}
		vty_out( vty, "\r\n" );
	}
}
VOID showVlanTrunkGroup( struct vty *vty, ULONG grpId )
{
	eth_ctc_vlan_trunk_list_t *pNode = pEthCtcVlanTrunkHead;

	vty_out( vty, "\r\n trunk-id  trunk-vlan\r\n" );
	while( pNode )
	{
		if( grpId )
		{
			if( pNode->ctcVlanTrunkGroupId == grpId )
			{
				showVlanTrunkGroupNode( vty, pNode );
			}
		}
		else
		{
			if( pNode->ctcVlanTrunkGroupId )
			{
				showVlanTrunkGroupNode( vty, pNode );
			}
		}
		pNode = pNode->pNextNode;
	}
	vty_out( vty, "\r\n" );
}

DEFUN(vlan_trunk_group_show,
	vlan_trunk_group_show_cmd,
       "show ctc vlan-trunk {<1-10000>}*1",
	SHOW_STR
	CTC_STR
	"vlan trunk\n"
	"input vlan trunk-group id\n"
	)
{
	if( argc == 0 )
	{
		showVlanTrunkGroup( vty, 0 );
	}
	else
	{
		showVlanTrunkGroup( vty, VOS_AtoI( argv[0] ) );
	}
	
	return  CMD_SUCCESS;
}
DEFUN(vlan_trunk_map_show,
	vlan_trunk_map_show_cmd,
       "show ctc vlan-trunk map",
	SHOW_STR
	CTC_STR
	"vlan trunk\n"
	"vlan trunk-port map\n"
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

	vty_out(vty, "\r\n portNo   trunkGroup\r\n" );
	for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
	{
		val = 0;
		mn_getEthPortVlanTrunkGrpId( devIdx, ethIdx, &val ); 
		if( val != 0 )
			vty_out(vty, "  %-9d%-8d\r\n", ethIdx, val );
		else
			vty_out(vty, "  %-9d%-8s\r\n", ethIdx, "-" );
	}

	return  CMD_SUCCESS;
}

LONG  CT_VlanTrunkCli_Init()
{
	install_element ( CONFIG_NODE, &vlan_trunk_group_add_cmd);
	install_element ( CONFIG_NODE, &vlan_trunk_group_del_cmd);
	install_element ( CONFIG_NODE, &vlan_trunk_group_show_cmd);

	install_element ( ONU_CTC_NODE, &vlan_trunk_group_add_cmd);
	install_element ( ONU_CTC_NODE, &vlan_trunk_group_del_cmd);
	install_element ( ONU_CTC_NODE, &vlan_trunk_group_show_cmd);

	install_element ( ONU_CTC_NODE, &vlan_trunk_map_cmd);
	install_element ( ONU_CTC_NODE, &undo_vlan_trunk_map_cmd);
	install_element ( ONU_CTC_NODE, &vlan_trunk_map_show_cmd);
	
	return VOS_OK;
}


/*----------------------------------------------------------------------------*/

/* CLI */

/*----------------------------------------------------------------------------*/
#endif

