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

#include "CT_Rman_VLAN.h"


#if 0
extern CT_Onu_EthPortItem_t  onuEthPort[20][MAXONUPERPON][MAX_ONU_ETHPORT];
#else
extern CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPONNOLIMIT][MAX_ONU_ETHPORT];
#endif
eth_ctc_vlan_aggr_list_t *pEthCtcVlanAggrHead = NULL;


eth_ctc_vlan_aggr_list_t *CTC_addCtcVlanAggrNode( const ULONG grpId )
{
	eth_ctc_vlan_aggr_list_t *pCur = pEthCtcVlanAggrHead;
	eth_ctc_vlan_aggr_list_t *pPre = NULL;
	eth_ctc_vlan_aggr_list_t *pNewNode;

	if( grpId > CTC_VLAN_AGGR_ID_MAX )
		return NULL;
	
	while(  pCur!= NULL )
	{
		if( grpId < pCur->ctcVlanAggrGroupId )
		{
			break;
		}
		else if(pCur->ctcVlanAggrGroupId == grpId )
		{
			return pCur;
		}
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	pNewNode = VOS_Malloc( sizeof(eth_ctc_vlan_aggr_list_t), MODULE_RPU_ONU );
		
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return pNewNode;
	}

	VOS_MemZero( pNewNode, sizeof(eth_ctc_vlan_aggr_list_t) );
	pNewNode->ctcVlanAggrGroupId = grpId;

	if( pPre != NULL )
	{
		pNewNode->pNextNode = pPre->pNextNode;
		pPre->pNextNode = pNewNode;
	}
	else
	{
		if( pCur == NULL )
		{
			if(  pEthCtcVlanAggrHead == NULL )
			{
				pEthCtcVlanAggrHead = pNewNode;
			}
			else
			{
				sys_console_printf(" ## %s %d\r\n", __FILE__, __LINE__);
				return NULL;
			}
		}
		else
		{
			pNewNode->pNextNode = pEthCtcVlanAggrHead;
			pEthCtcVlanAggrHead = pNewNode;
		}
	}
	
	return pNewNode;
}
STATUS	CTC_delCtcVlanAggrNode( const ULONG grpId )
{
	STATUS rc = VOS_OK;
	eth_ctc_vlan_aggr_list_t *pCur = pEthCtcVlanAggrHead;
	eth_ctc_vlan_aggr_list_t *pPre = NULL;

	while( pCur )
	{
		if( grpId == pCur->ctcVlanAggrGroupId )
			break;
		pPre = pCur;
		pCur = pCur->pNextNode;
	}

	if( pCur != NULL )
	{
		if( pPre == NULL )
		{
			pEthCtcVlanAggrHead = pCur->pNextNode;
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

eth_ctc_vlan_aggr_list_t *CTC_findCtcVlanAggrNode( const ULONG grpId )
{
	eth_ctc_vlan_aggr_list_t *pCur = pEthCtcVlanAggrHead;

	if( grpId == 0 )
		return NULL;
	while(  pCur!= NULL )
	{
		if(pCur->ctcVlanAggrGroupId == grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}
eth_ctc_vlan_aggr_list_t *CTC_findNextCtcVlanAggrNode( const ULONG grpId )
{
	eth_ctc_vlan_aggr_list_t *pCur = pEthCtcVlanAggrHead;

	/*if( grpId == 0 )
	{
		return pCur;
	}*/
	while(  pCur!= NULL )
	{
		if(pCur->ctcVlanAggrGroupId > grpId )
		{
			break;
		}
		pCur = pCur->pNextNode;
	}
	return pCur;
}

extern int vlan_list_in_ascending_sort( USHORT *pArray, ULONG maxNum );
int aggr_vlan_list_in_ascending_sort( eth_ctc_vlan_aggr_list_t *pNode )
{
	int i, j, n;
	eth_ctc_vlan_aggregation_t temp;

	n=0;
	if( pNode->ctcVlanAggrEntries[n].target_vlan == 0 )
		VOS_MemZero( &temp, sizeof(eth_ctc_vlan_aggregation_t) );
	else
		VOS_MemCpy( &temp, &pNode->ctcVlanAggrEntries[n], sizeof(eth_ctc_vlan_aggregation_t) );
	
	for( i=1; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
	{
		if( pNode->ctcVlanAggrEntries[i].target_vlan )
		{
			VOS_MemCpy( &pNode->ctcVlanAggrEntries[n], &pNode->ctcVlanAggrEntries[i], sizeof(eth_ctc_vlan_aggregation_t) );
			pNode->ctcVlanAggrEntries[i].target_vlan = 0;
			n++;
		}
		else
			VOS_MemZero( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list, sizeof(pNode->ctcVlanAggrEntries[i].vlan_aggregation_list) );
	}
	VOS_MemCpy( &pNode->ctcVlanAggrEntries[n], &temp, sizeof(eth_ctc_vlan_aggregation_t) );
	if( (temp.target_vlan == 0) && (n > 0) )
		n--;

	for( i=0; i<n; i++ )
	{
		for( j=0; j<n-i; j++ )
		{
			/*if( pNode->ctcVlanAggrEntries[j].target_vlan == 0 )
			{
				VOS_MemCpy( &temp, &pNode->ctcVlanAggrEntries[j], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j], &pNode->ctcVlanAggrEntries[j+1], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j+1], &temp, sizeof(eth_ctc_vlan_aggregation_t) );
			}
			else if( (pNode->ctcVlanAggrEntries[j].target_vlan > pNode->ctcVlanAggrEntries[j+1].target_vlan) && (pNode->ctcVlanAggrEntries[j+1].target_vlan != 0) )
			{
				VOS_MemCpy( &temp, &pNode->ctcVlanAggrEntries[j], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j], &pNode->ctcVlanAggrEntries[j+1], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j+1], &temp, sizeof(eth_ctc_vlan_aggregation_t) );
			}*/
			if( (pNode->ctcVlanAggrEntries[j].target_vlan == 0) ||
				((pNode->ctcVlanAggrEntries[j].target_vlan > pNode->ctcVlanAggrEntries[j+1].target_vlan) && (pNode->ctcVlanAggrEntries[j+1].target_vlan != 0)) )
			{
				VOS_MemCpy( &temp, &pNode->ctcVlanAggrEntries[j], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j], &pNode->ctcVlanAggrEntries[j+1], sizeof(eth_ctc_vlan_aggregation_t) );
				VOS_MemCpy( &pNode->ctcVlanAggrEntries[j+1], &temp, sizeof(eth_ctc_vlan_aggregation_t) );
			}
		}
	}

	for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
	{
		if( pNode->ctcVlanAggrEntries[i].target_vlan )
		{
			vlan_list_in_ascending_sort( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list, CTC_MAX_VLAN_AGGREGATION_ENTRIES );
		}
	}
	
	return VOS_OK;
}

int CTC_addVlanAggregation( ULONG grpId, ULONG targetVid, ULONG userVid)
{
	int i;
	int targetP = -1, aggrP = -1;

	eth_ctc_vlan_aggr_list_t *pNode;
		
	if( (grpId == 0) || (grpId > CTC_VLAN_AGGR_ID_MAX) )
		return VOS_ERROR;

	pNode = (eth_ctc_vlan_aggr_list_t *)CTC_addCtcVlanAggrNode( grpId );
	if( pNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
	{
		if( pNode->ctcVlanAggrEntries[i].target_vlan == 0 )
		{
			if( targetP == -1 ) targetP = i;
		}
		else if( pNode->ctcVlanAggrEntries[i].target_vlan == targetVid )
		{
			targetP = i;
			break;
		}
	}
	if( targetP == -1 )
		return VOS_ERROR;
	
	for( i=0; i<CTC_MAX_VLAN_AGGREGATION_ENTRIES; i++ )
	{
		if( pNode->ctcVlanAggrEntries[targetP].vlan_aggregation_list[i] == 0 )
		{	
			if( aggrP == -1 ) aggrP = i;
		}
		else if( pNode->ctcVlanAggrEntries[targetP].vlan_aggregation_list[i] == userVid )
		{
			aggrP = i;
			break;
		}
	}
	if( aggrP == -1 )
		return VOS_ERROR;

	pNode->ctcVlanAggrEntries[targetP].target_vlan = targetVid;
	pNode->ctcVlanAggrEntries[targetP].vlan_aggregation_list[aggrP] = userVid;

	aggr_vlan_list_in_ascending_sort( pNode );

	return VOS_OK;
}

STATUS	CTC_delVlanAggregation( ULONG grpId, ULONG targetVid, ULONG userVid)
{
	int count = 0;
	int i, j;

	eth_ctc_vlan_aggr_list_t *pNode;
		
	pNode = (eth_ctc_vlan_aggr_list_t *)CTC_findCtcVlanAggrNode( grpId );
	if( pNode == NULL )
	{
		return VOS_ERROR;
	}

	for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
	{
		if( pNode->ctcVlanAggrEntries[i].target_vlan != targetVid )
			continue;
			
		count = 0;
		for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
		{
			if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] == userVid )
			{
				pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] = 0;
			}
			if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] != 0 )
				count++;
		}
		if( count == 0 )
			pNode->ctcVlanAggrEntries[i].target_vlan = 0;
	}

	aggr_vlan_list_in_ascending_sort( pNode );

	return VOS_OK;
}

int CTC_getEthPortVlanAggrGroup( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
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
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethCtcVlanAggrSelect;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_setEthPortVlanAggrGroup( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > CTC_VLAN_AGGR_ID_MAX )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethCtcVlanAggrSelect = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int mn_getFirstCtcVlanAggrEntryIndex( ULONG *pGrpId, ULONG *pTargetVid, ULONG *pUserVid )
{
	eth_ctc_vlan_aggr_list_t *pNode = pEthCtcVlanAggrHead;
	int i, j;

	if( (pGrpId == NULL) || (pTargetVid == NULL) || (pUserVid == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanAggrGroupId )
		{
			for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
			{
				if( pNode->ctcVlanAggrEntries[i].target_vlan != 0 )
				{
					for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
					{
						if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] != 0 )
						{
							*pGrpId = pNode->ctcVlanAggrGroupId;
							*pTargetVid = pNode->ctcVlanAggrEntries[i].target_vlan;
							*pUserVid = pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j];
							return VOS_OK;
						}
					}
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_getNextCtcVlanAggrEntryIndex( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG *pNextGrpId, ULONG *pNextTargetVid, ULONG *pNextUserVid )
{
	eth_ctc_vlan_aggr_list_t *pNode = pEthCtcVlanAggrHead;
	int i, j;

	if( (pNextGrpId == NULL) || (pNextTargetVid == NULL) || (pNextUserVid == NULL) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanAggrGroupId == 0 )
		{
			pNode = pNode->pNextNode;
			continue;
		}
		
		if( pNode->ctcVlanAggrGroupId == grpId )
		{
			for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
			{
				if( (pNode->ctcVlanAggrEntries[i].target_vlan != 0) && (pNode->ctcVlanAggrEntries[i].target_vlan == targetVid) )
				{
					for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
					{
						if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] > userVid )
						{
							*pNextGrpId = pNode->ctcVlanAggrGroupId;
							*pNextTargetVid = pNode->ctcVlanAggrEntries[i].target_vlan;
							*pNextUserVid = pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j];
							return VOS_OK;
						}
					}
				}
				
				if( pNode->ctcVlanAggrEntries[i].target_vlan > targetVid )
				{
					for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
					{
						if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] )
						{
							*pNextGrpId = pNode->ctcVlanAggrGroupId;
							*pNextTargetVid = pNode->ctcVlanAggrEntries[i].target_vlan;
							*pNextUserVid = pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j];
							return VOS_OK;
						}
					}
				}
			}
		}
		if( pNode->ctcVlanAggrGroupId > grpId )
		{
			for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
			{
				if( pNode->ctcVlanAggrEntries[i].target_vlan )
				{
					for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
					{
						if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] )
						{
							*pNextGrpId = pNode->ctcVlanAggrGroupId;
							*pNextTargetVid = pNode->ctcVlanAggrEntries[i].target_vlan;
							*pNextUserVid = pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j];
							return VOS_OK;
						}
					}
				}
			}
		}

		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}

int mn_checkCtcVlanAggrEntryIndex( ULONG grpId, ULONG targetVid, ULONG userVid )
{
	int i, j;
	eth_ctc_vlan_aggr_list_t *pNode = pEthCtcVlanAggrHead;

	if( (CTC_VLAN_AGGR_ID_MAX < grpId) || (0 == targetVid) || (4094 < targetVid) || (0 == userVid) || (4094 < userVid) )
		return VOS_ERROR;

	while( pNode )
	{
		if( pNode->ctcVlanAggrGroupId == grpId )
		{
			for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
			{
				if( pNode->ctcVlanAggrEntries[i].target_vlan == targetVid )
				{
					for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
					{
						if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] == userVid )
						{
							return VOS_OK;
						}
					}
				}
			}
		}
		pNode = pNode->pNextNode;	
	}
	return VOS_ERROR;
}


int mn_getCtcVlanAggrEntryStatus( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG *pStatus )
{
	if( pStatus == NULL )
		return VOS_ERROR;
	
	if( mn_checkCtcVlanAggrEntryIndex(grpId, targetVid, userVid) == VOS_OK )
		*pStatus = RS_ACTIVE;

	return VOS_ERROR;
}

int mn_setCtcVlanAggrEntryStatus( ULONG grpId, ULONG targetVid, ULONG userVid, ULONG status )
{
	int rc = VOS_ERROR;

	if( (CTC_VLAN_AGGR_ID_MAX < grpId) || (0 == targetVid) || (4094 < targetVid) || (0 == userVid) || (4094 < userVid) )
		return VOS_ERROR;

	if( (status == RS_CREATEANDGO) || (status == RS_CREATEANDWAIT) )
	{
		rc = CTC_addVlanAggregation( grpId, targetVid, userVid );
	}
	else if( status == RS_DESTROY )
	{
		rc = CTC_delVlanAggregation( grpId, targetVid, userVid );
	}
	else if( status == RS_ACTIVE )
	{
	}
	else
	{
	}
	return rc;
}
int mn_getEthPortVlanAggrGrpId( ULONG devIdx, ULONG portIdx,  ULONG *pAggrId)
{
	ULONG id = 0;
	if( CTC_getEthPortVlanAggrGroup(devIdx, portIdx, &id) == VOS_OK )
	{
		if( CTC_findCtcVlanAggrNode( id ) == NULL )
			*pAggrId = 0;
		else
			*pAggrId = id;
		
		return VOS_OK;
	}
	return VOS_ERROR;
}

/*int CTC_ethPortVlanAggr_map_set( ULONG devIdx, ULONG portIdx,  ULONG grpId)*/
int mn_setEthPortVlanAggrGrpId( ULONG devIdx, ULONG portIdx,  ULONG grpId)
{
	if( CTC_findCtcVlanAggrNode( grpId ) == NULL )
	{
		return VOS_ERROR;
	}
	return CTC_setEthPortVlanAggrGroup( devIdx, portIdx, grpId );
}


DEFUN(vlan_aggregation_group_add,
	vlan_aggregation_group_add_cmd,
       "ctc vlan-aggregation add <1-10000> <1-4094> <1-4094>",
	CTC_STR
	"vlan aggregation\n"
	"add vlan-aggr group\n"
	"input vlan aggr-group id\n"
	"input target vlan id\n"
	"input aggregated user vlan id\n"
	)
{
	ULONG grpId;
       ULONG aggrTargetVlanId;
       ULONG aggrUserVlanId;
	   
	grpId=VOS_AtoI( argv[0] );
	aggrTargetVlanId=VOS_AtoI( argv[1] );
	aggrUserVlanId=VOS_AtoI( argv[2] );
	
	if( mn_setCtcVlanAggrEntryStatus( grpId, aggrTargetVlanId, aggrUserVlanId, RS_CREATEANDGO) == VOS_OK )
	{
		return  CMD_SUCCESS;
	}
	
	vty_out( vty, "vlan-aggregation err\r\n" );
	return  CMD_WARNING;
}



DEFUN(vlan_aggregation_group_del,
	vlan_aggregation_group_del_cmd,
       "ctc vlan-aggregation delete <1-10000> {<1-4094> <1-4094>}*1",
	CTC_STR
	"vlan aggregation\n"
	"delete aggr-vlan items\n"
	"aggr-group id\n"
	"input target vlan id\n"
	"input aggregated user vlan id\n"
	)
{
	ULONG grpId;
       ULONG aggrTargetVlanId;
       ULONG aggrUserVlanId;
	   
	grpId = VOS_AtoI( argv[0] );

	if( argc == 1 )
	{
		if( CTC_delCtcVlanAggrNode(grpId) == VOS_ERROR )
		{
			vty_out(vty, "vlan-aggr group delete error!\r\n");        
			return CMD_WARNING;
		}
	}
	else
	{
		aggrTargetVlanId = VOS_AtoI( argv[1] );
		aggrUserVlanId = VOS_AtoI( argv[2] );
		if( mn_setCtcVlanAggrEntryStatus( grpId, aggrTargetVlanId, aggrUserVlanId, RS_DESTROY) == VOS_ERROR )
		{
			vty_out(vty, "vlan-aggr delete error!\r\n");        
			return CMD_WARNING;
		}
	}
	return CMD_SUCCESS;
}

DEFUN(vlan_aggregation_map,
	vlan_aggregation_map_cmd,
       "ctc vlan-aggregation map <1-10000> [all|<port_list>]",
	CTC_STR
	"vlan aggregation\n"
	"map port\n"
	"aggr-group id\n"
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

	grpId=VOS_AtoI( argv[0] );
	if( CTC_findCtcVlanAggrNode(grpId) == NULL )
	{
		vty_out(vty, "aggr group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortVlanAggrGrpId( devIdx, ethIdx, grpId)==VOS_ERROR)
				vty_out(vty, "aggr-group-map port=%d error!\r\n", ethIdx);
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
			if(mn_setEthPortVlanAggrGrpId( devIdx, ethIdx, grpId)==VOS_ERROR)
				vty_out(vty, "aggr-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}

DEFUN(undo_vlan_aggregation_map,
	undo_vlan_aggregation_map_cmd,
	"undo ctc vlan-aggregation map <1-10000> [all|<port_list>]",
	"undo operation\n"
	CTC_STR
	"undo vlan aggregation\n"
	"map port\n"
	"aggr-group id\n"
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

	grpId=VOS_AtoI( argv[0] );
	if( CTC_findCtcVlanAggrNode(grpId) == NULL )
	{
		vty_out(vty, "aggr group id is not exist\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[1], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(mn_setEthPortVlanAggrGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "aggr-group-map port=%d error!\r\n", ethIdx);
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
		END_PARSE_PORT_LIST_TO_PORT();*/

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[1], ethIdx )
		if( ethIdx <= portNum )
		{
			if(mn_setEthPortVlanAggrGrpId( devIdx, ethIdx, 0)==VOS_ERROR)
				vty_out(vty, "aggr-group-map port=%d error!\r\n", ethIdx);
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}

static VOID showVlanAggrGroupNode( struct vty *vty, eth_ctc_vlan_aggr_list_t *pNode )
{
	int i,j;
	int target_count, user_count;

	if( pNode )
	{
		vty_out( vty, "  %-10d", pNode->ctcVlanAggrGroupId );

		target_count = 0;
		for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
		{
			if( pNode->ctcVlanAggrEntries[i].target_vlan )
			{
				if( target_count == 0 )
					vty_out( vty, "%-12d", pNode->ctcVlanAggrEntries[i].target_vlan );
				else
					vty_out( vty, "%-12s%-12d", " ", pNode->ctcVlanAggrEntries[i].target_vlan );
				target_count++;
						
				user_count = 0;
				for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
				{
					if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] != 0 )
					{
						if( user_count == 0 )
							vty_out( vty, " %d", pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] );
						else
							vty_out( vty, "%-24s%d", " ", pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] );
					}
				}
				vty_out( vty, "\r\n" );
			}
		}
		if( target_count == 0 )
			vty_out( vty, "\r\n" );
	}
}
VOID showVlanAggrGroup( struct vty *vty, ULONG grpId )
{
	eth_ctc_vlan_aggr_list_t *pNode = pEthCtcVlanAggrHead;

	vty_out( vty, "\r\n aggr-id  target-vlan  user-vlan\r\n" );
	while( pNode )
	{
		if( grpId )
		{
			if( pNode->ctcVlanAggrGroupId == grpId )
			{
				showVlanAggrGroupNode( vty, pNode );
			}
		}
		else
		{
			if( pNode->ctcVlanAggrGroupId )
			{
				showVlanAggrGroupNode( vty, pNode );
			}
		}
		pNode = pNode->pNextNode;
	}
	vty_out( vty, "\r\n" );
}

DEFUN(vlan_aggregation_group_show,
	vlan_aggregation_group_show_cmd,
       "show ctc vlan-aggregation {<1-10000>}*1",
	SHOW_STR
	CTC_STR
	"vlan aggregation\n"
	"aggr-group id\n"
	)
{
	if( argc == 0 )
	{
		showVlanAggrGroup( vty, 0 );
	}
	else
	{
		showVlanAggrGroup( vty, VOS_AtoI( argv[0] ) );
	}
	
	return  CMD_SUCCESS;
}
DEFUN(vlan_aggregation_map_show,
	vlan_aggregation_map_show_cmd,
       "show ctc vlan-aggregation map",
	SHOW_STR
	CTC_STR
	"vlan aggregation\n"
	"aggr-port map\n"
	)
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
 	ULONG pon;
	ULONG onu;
	PON_olt_id_t olt_id = 0;
	PON_onu_id_t onu_id = 0;
   
	ULONG portNum;
	USHORT sel;

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

	vty_out(vty, "\r\n portNo   aggrGroup\r\n" );
	for( ethIdx=0; ethIdx<portNum; ethIdx++ )
	{
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		sel = onuEthPort[olt_id][onu_id][ethIdx].ethCtcVlanAggrSelect;
		VOS_SemGive( onuEthPortSemId );
		if( sel != 0 )
			vty_out(vty, "  %-9d%-8d\r\n", ethIdx+1, sel );
		else
			vty_out(vty, "  %-9d%-8s\r\n", ethIdx+1, "-" );
	}

	return  CMD_SUCCESS;
}

LONG  CT_VlanAggrCli_Init()
{
	install_element ( CONFIG_NODE, &vlan_aggregation_group_add_cmd);
	install_element ( CONFIG_NODE, &vlan_aggregation_group_del_cmd);
	install_element ( CONFIG_NODE, &vlan_aggregation_group_show_cmd);

	install_element ( ONU_CTC_NODE, &vlan_aggregation_group_add_cmd);
	install_element ( ONU_CTC_NODE, &vlan_aggregation_group_del_cmd);
	install_element ( ONU_CTC_NODE, &vlan_aggregation_group_show_cmd);

	install_element ( ONU_CTC_NODE, &vlan_aggregation_map_cmd);
	install_element ( ONU_CTC_NODE, &undo_vlan_aggregation_map_cmd);
	install_element ( ONU_CTC_NODE, &vlan_aggregation_map_show_cmd);
	
	return VOS_OK;
}


/*----------------------------------------------------------------------------*/

/* CLI */

/*----------------------------------------------------------------------------*/
#endif

