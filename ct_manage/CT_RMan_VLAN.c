#include "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#if 0
#include "gwEponSys.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include "lib_ethPortMib.h"
#include	"lib_gwEponMib.h"

#include "CT_RMan_Main.h"
#include  "../cli/Olt_cli.h"

#include "CT_Rman_VLAN.h"


extern CT_Onu_EthPortItem_t  (*onuEthPort)[MAXONUPERPON][MAX_ONU_ETHPORT];


/* TPID 默认0x8100 */
int CTC_ethPortVlanTagTpid_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
#if 0	/* xieshl 20070328, 读该对象时刷新所有VLAN数据 */
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;

	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid;
	return VOS_OK;
#else
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	/*CTC_STACK_port_vlan_configuration_t  port_vlan;*/
	
	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;
#endif
#if 0
	if( CTC_STACK_get_vlan_port_configuration(olt_id, llid, portIdx+1, &port_vlan) != CTC_STACK_EXIT_OK )
		return VOS_ERROR;

	if( port_vlan.mode == CTC_VLAN_MODE_TRANSPARENT )
	{
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode = 1;/* transparent */
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid = 0x8100;
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid = 1;
	}
	else if( port_vlan.mode == CTC_VLAN_MODE_TAG )
	{
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode = 2; /* tag */
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid = ((port_vlan.vlan_list[0] >> 16) &0xffff);
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid = (port_vlan.vlan_list[0] & 0xffff);
	}
	else if( port_vlan.mode == CTC_VLAN_MODE_TRANSLATION )
	{
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode = 3; /* translation */
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid = ((port_vlan.vlan_list[0] >> 16) & 0xffff);
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid = (port_vlan.vlan_list[0] & 0xffff);
	}
	else
		return VOS_ERROR;
	
	return VOS_OK;
#endif

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

int CTC_ethPortVlanTagTpid_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid = val;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/* CFI */
int CTC_ethPortVlanTagCfi_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
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
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid & 0x1000;
	VOS_SemGive( onuEthPortSemId );

	if( *pVal )
		*pVal = 1;
	else
		*pVal = 2;

	return VOS_OK;
}

int CTC_ethPortVlanTagCfi_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	if( val == 1 )
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid |= 0x1000;
	else
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid &= 0xefff;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/* PRI */
int CTC_ethPortVlanTagPri_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid >> 13) & 0x7);
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int CTC_ethPortVlanTagPri_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid &= 0x1fff;
	onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid |= ((val << 13) & 0xe000);
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/* VID */
int CTC_ethPortVlanTagVid_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = (onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid & 0xfff);
	VOS_SemGive( onuEthPortSemId );

	/*
	added by wangxy 2007-06-29
	如果从PMC读来的值为0,默认改为1
	*/
	if( *pVal == 0 )
		*pVal = 1;
	
	return VOS_OK;
}
int CTC_ethPortVlanTagVid_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	if( val > 10000000 )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid &= 0xf000;
	onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid |= (val & 0xfff);
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/* VLAN mode : 0-transparent、1-tag、2-translation*/
int CTC_ethPortVlanMode_get( ULONG devIdx, ULONG portIdx,  ULONG *pVal)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( (pVal == NULL) || (portIdx > MAX_ONU_ETHPORT) )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	*pVal = onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode;
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}
int CTC_ethPortVlanMode_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;

	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;

	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	if( onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode != val )
	{
		onuEthPort[olt_id][onu_id][portIdx].ethPortVlanMode = val;
	}
	VOS_SemGive( onuEthPortSemId );

	return VOS_OK;
}

/*
BEGIN: modified by wangxy 2007-07-30
vlantranslation模式时，设置defaultvlan位置发生变化，同时对numberofentries的个数定义也发生变化
*/

int CTC_ethPortVlanAction_set( ULONG devIdx, ULONG portIdx,  ULONG val)
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;
	PON_STATUS lret;

	CTC_STACK_port_vlan_configuration_t  port_vlan;
	CTC_management_object_index_t midx;
	LogicEntity * pentry = NULL;

	int i=0, j=0;
	
	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	VOS_MemZero( &port_vlan, sizeof(port_vlan) );
	if( val == CTC_VLAN_MODE_TRANSPARENT )		/* transparent */
	{
		port_vlan.mode = val;
		port_vlan.default_vlan = 1;
		if( CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx+1, port_vlan) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}
	else if( val == CTC_VLAN_MODE_TAG ) /* tag */
	{
		port_vlan.mode = val;
		port_vlan.number_of_entries = 1;
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		port_vlan.vlan_list[0] = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid & 0xffff) << 16) |
							  onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid;
		VOS_SemGive( onuEthPortSemId );

		port_vlan.default_vlan = port_vlan.vlan_list[0];

		if( CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx+1, port_vlan) != CTC_STACK_EXIT_OK )
			return VOS_ERROR;
	}
	else if( val == CTC_VLAN_MODE_TRANSLATION )	/* translation */
	{
		port_vlan.mode = val;
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		port_vlan.default_vlan = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid & 0xffff) << 16) |
								  onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid;
		VOS_SemGive( onuEthPortSemId );
		
		if( getLogicEntity( devIdx, &pentry ) == VOS_OK )
		{
			for( i=0; i<MAX_VLAN_TRANS_PER_PORT; i++ )
			{
				if( pentry->ethvlantrans[portIdx][i].entryStatus == RS_ACTIVE )
				{
					port_vlan.vlan_list[j] = pentry->ethvlantrans[portIdx][i].inVlanId;
					j++;
					port_vlan.vlan_list[j] = pentry->ethvlantrans[portIdx][i].outVlanId;					
					j++;
					if( j >= pentry->ethvlantransperport[portIdx]*2 )
						break;
				}
			}

			port_vlan.number_of_entries = pentry->ethvlantransperport[portIdx];

			if( (lret = CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx+1, port_vlan)) != CTC_STACK_EXIT_OK )
			{
				sys_console_printf( "\r\n%-40s %d", "CTC_ethPortVlanAction_set fail num:", lret );
				return VOS_ERROR;
			}
		}
	}
	else if( val == CTC_VLAN_MODE_AGGREGATION )
	{
		int targetN = 0;
		int aggrN = 0;
		ULONG aggrId = 0;
		eth_ctc_vlan_aggr_list_t *pNode;
		
		port_vlan.mode = val;
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		port_vlan.default_vlan = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid & 0xffff) << 16) |
								  onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid;
		VOS_SemGive( onuEthPortSemId );

		port_vlan.number_of_entries = 1;
		port_vlan.vlan_list[0] = port_vlan.default_vlan;

		if( CTC_getEthPortVlanAggrGroup(devIdx, portIdx+1, &aggrId) == VOS_ERROR )
			return VOS_ERROR;
		pNode = CTC_findCtcVlanAggrNode( aggrId );
		
		if( pNode )
		{
			for( i=0; i<CTC_MAX_VLAN_AGGREGATION_TABLES; i++ )
			{
				if( pNode->ctcVlanAggrEntries[i].target_vlan == 0 )
					continue;

				port_vlan.number_of_entries = 0;
				
				aggrN = 0;
				for( j=0; j<CTC_MAX_VLAN_AGGREGATION_ENTRIES; j++ )
				{
					if( pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] != 0 )
					{
						port_vlan.vlan_aggregation_tables[targetN].vlan_aggregation_list[aggrN] = pNode->ctcVlanAggrEntries[i].vlan_aggregation_list[j] | (0x8100 << 16 );
						aggrN++;
					}
				}
				if( aggrN )
				{
					port_vlan.vlan_aggregation_tables[targetN].target_vlan = pNode->ctcVlanAggrEntries[i].target_vlan;
					port_vlan.vlan_aggregation_tables[targetN].number_of_aggregation_entries = aggrN;
					targetN++;
				}
			}

			if( targetN )
			{
				port_vlan.number_of_aggregation_tables = targetN;

				midx.frame_number = 0;
				midx.slot_number = 0;
				midx.port_number = portIdx+1;
				midx.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
				if( (lret = CTC_STACK_set_vlan_management_object_configuration(olt_id, llid, midx, port_vlan)) != CTC_STACK_EXIT_OK )
				/*if( (lret = CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx+1, port_vlan)) != CTC_STACK_EXIT_OK )*/
				{
					sys_console_printf( "\r\n%-40s %d", "CTC_ethPortVlanAction_set fail num:", lret );
					return VOS_ERROR;
				}
			}
		}
	}
	else if( val == CTC_VLAN_MODE_TRUNK )
	{
#if 0
		port_vlan.mode = val;
		port_vlan.default_vlan = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid & 0xffff) << 16) |
								  onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid;

		if( getLogicEntity( devIdx, &pentry ) == VOS_OK )
		{
			j = 0;
			for( i=0; i<MAX_VLAN_TRANS_PER_PORT; i++ )
			{
				if( pentry->eth_vlan_trunk[portIdx][i] )
				{
					port_vlan.vlan_list[j] = pentry->eth_vlan_trunk[portIdx][i] | (0x8100 << 16);
					j++;
				}
			}
			
			if( j )
			{
				port_vlan.number_of_entries = j;

				midx.frame_number = 0;
				midx.slot_number = 0;
				midx.port_number = portIdx+1;
				midx.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
				if( (lret = CTC_STACK_set_vlan_management_object_configuration(olt_id, llid, midx, port_vlan)) != CTC_STACK_EXIT_OK )
				/*if( (lret = CTC_STACK_set_vlan_port_configuration(olt_id, llid, portIdx+1, port_vlan)) != CTC_STACK_EXIT_OK )*/
				{
					sys_console_printf( "\r\n%-40s %d", "CTC_ethPortVlanAction_set fail num:", lret );
					return VOS_ERROR;
				}
			}
		}
#endif
		ULONG trunkId = 0;
		eth_ctc_vlan_trunk_list_t *pNode;
		
		port_vlan.mode = val;
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		port_vlan.default_vlan = ((onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagTpid & 0xffff) << 16) |
								  onuEthPort[olt_id][onu_id][portIdx].ethPortVlanTagPri_Cfi_Vid;
		VOS_SemGive( onuEthPortSemId );

		port_vlan.number_of_entries = 1;
		port_vlan.vlan_list[0] = port_vlan.default_vlan;

		if( CTC_getEthPortVlanTrunkGroup(devIdx, portIdx, &trunkId) == VOS_ERROR )
			return VOS_ERROR;
		pNode = CTC_findCtcVlanTrunkNode( trunkId );
		
		if( pNode )
		{
			j = 0;
			for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
			{
				if( pNode->ctcVlanTrunkEntries[i] )
				{
					port_vlan.vlan_list[j] = pNode->ctcVlanTrunkEntries[i] | (0x8100 << 16);
					j++;
				}
			}

			if( j )
			{
				port_vlan.number_of_entries = j;
				
				midx.frame_number = 0;
				midx.slot_number = 0;
				midx.port_number = portIdx+1;
				midx.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
				if( (lret = CTC_STACK_set_vlan_management_object_configuration(olt_id, llid, midx, port_vlan)) != CTC_STACK_EXIT_OK )
				{
					sys_console_printf( "\r\n%-40s %d", "CTC_ethPortVlanTrunk_set fail num:", lret );
					return VOS_ERROR;
				}
			}
		}
	}
	else
		return VOS_ERROR;

	return VOS_OK;
}

STATUS CTC_ethPortVlanAction_get( ULONG devIdx, ULONG portIdx )
{
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t	llid;

	CT_Onu_EthPortItem_t *pItem;
	CTC_STACK_port_vlan_configuration_t  port_vlan;

	LogicEntity * pentry = NULL;
	int i, j;
	
	portIdx--;
	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;
	
	if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

	if( CTC_STACK_get_vlan_port_configuration( olt_id, llid, portIdx+1,  &port_vlan ) != CTC_STACK_EXIT_OK )
		 return VOS_ERROR;

	pItem = (CT_Onu_EthPortItem_t*)&onuEthPort[olt_id][onu_id][portIdx];

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	pItem->ethPortVlanMode = port_vlan.mode;
	pItem->ethPortVlanTagPri_Cfi_Vid = port_vlan.vlan_list[0]&0xffff;
	pItem->ethPortVlanTagTpid = port_vlan.vlan_list[0]>>16;
	VOS_SemGive( onuEthPortSemId );
		
	if( port_vlan.mode == CTC_VLAN_MODE_TAG )
	{
	}
	else if( port_vlan.mode == CTC_VLAN_MODE_TRANSLATION )
	{
		if( getLogicEntity( devIdx, &pentry ) == VOS_OK )
		{
			j = 0;

			if(port_vlan.number_of_entries > MAX_VLAN_TRANS_PER_PORT)
			{
				sys_console_printf("\r\nentry number is too big, exceed the maximum");
				return VOS_ERROR;
			}
				
			pentry->ethvlantransperport[portIdx] = port_vlan.number_of_entries;
			VOS_MemZero( pentry->ethvlantrans[portIdx], sizeof(pentry->ethvlantrans[portIdx]) );

			for( i=0; i<port_vlan.number_of_entries; i++ )
			{
				pentry->ethvlantrans[portIdx][i].entryStatus = 1;
				pentry->ethvlantrans[portIdx][i].inVlanId = port_vlan.vlan_list[j];
				j++;
				pentry->ethvlantrans[portIdx][i].outVlanId = port_vlan.vlan_list[j];
				j++;
			}
		}
	}
	else if( port_vlan.mode == CTC_VLAN_MODE_AGGREGATION )
	{

		/* nothing to do */
	}
	else if( port_vlan.mode == CTC_VLAN_MODE_TRUNK )
	{
		/* nothing to do */
#if 0
		if( getLogicEntity( devIdx, &pentry ) == VOS_OK )
		{
			if(port_vlan.number_of_entries > CTC_MAX_VLAN_FILTER_ENTRIES)
			{
				sys_console_printf("\r\nentry number is too big, exceed the maximum");
				return VOS_ERROR;
			}
				
			VOS_MemZero( pentry->eth_vlan_trunk[portIdx], sizeof(pentry->eth_vlan_trunk[portIdx]) );

			j = 0;
			for( i=0; i< port_vlan.number_of_entries; i++ )
			{
				if( port_vlan.vlan_list[i] != 0 )
				{
					pentry->eth_vlan_trunk[portIdx][j] = port_vlan.vlan_list[i];
					j++;
				}
			}
		}
#endif
	}
	return VOS_OK;
}

static int show_ctc_onu_port_default_vlan( struct vty *vty, ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx )
{
	ULONG value;
	if(getEthPortVlanTagVid(devIdx, brdIdx,ethIdx, &value)==VOS_OK)	
		vty_out( vty, ",default vid=%d", value);
	if(getEthPortVlanTagPri(devIdx, brdIdx,ethIdx, &value)==VOS_OK)	
		vty_out( vty, ",pri=%d", value);
	if(getEthPortVlanTagTpid(devIdx, brdIdx,ethIdx, &value)==VOS_OK)	
		vty_out( vty, ",Tpid=%x", value);
	if(getEthPortVlanTagCfi(devIdx, brdIdx,ethIdx, &value)==VOS_OK)	
	{
		if( value != 1 ) value = 0;
		vty_out( vty, ",Cfi=%d", value);
	}
	vty_out( vty, "\r\n" );
	return CMD_SUCCESS;
}

extern VOID showVlanAggrGroup( struct vty *vty, ULONG aggrId );
extern VOID showVlanTrunkGroup( struct vty *vty, ULONG grpId );
static int show_ctc_onu_port_vlan( struct vty *vty, ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx )
{
	ULONG nextDev;
	ULONG nextBrd;
	ULONG nextEth;
	ULONG nextInVid; 

	ULONG inVid;
	ULONG newVid;
	ULONG vlanMode;
	ULONG count;
	char *vlan_mode_str[] = {"transparent","tag","translation","aggregation","trunk"};

	int i;
	LogicEntity * pentry = NULL;
	ULONG portIdx = ethIdx-1;
	ULONG val = 0;

	if( portIdx > MAX_ONU_ETHPORT )
		return VOS_ERROR;


	if(getEthPortVlanMode( devIdx, brdIdx,ethIdx, &vlanMode )==VOS_ERROR)
	{
		vty_out( vty, " error\r\n" );
		return CMD_WARNING;
	}

	if( vlanMode <= CTC_VLAN_MODE_TRUNK )
		vty_out( vty, " port%d vlan mode=%s", ethIdx, vlan_mode_str[vlanMode] );
	
	if( vlanMode == CTC_VLAN_MODE_TRANSPARENT )
	{
		vty_out( vty, "\r\n" );
	}
	else if( vlanMode == CTC_VLAN_MODE_TAG )
	{
		show_ctc_onu_port_default_vlan( vty, devIdx, brdIdx, ethIdx );
	}
	else if( vlanMode == CTC_VLAN_MODE_TRANSLATION )
	{
		show_ctc_onu_port_default_vlan( vty, devIdx, brdIdx, ethIdx );

		inVid = 0;
		brdIdx = 1;
		count=0;
		while( getNextEthPortVlanTransEntry(devIdx, brdIdx,ethIdx, inVid,&nextDev,&nextBrd,&nextEth,&nextInVid) == VOS_OK )
		{
			if( (devIdx != nextDev)  || (ethIdx != nextEth ) || (count > 24) )
				break;
			newVid = 0;
			inVid = nextInVid;
			getEthPortVlanTranNewVid( devIdx, brdIdx,ethIdx, inVid, &newVid );
			vty_out( vty, "  in_vid=%d, new_vid=%d\r\n", inVid, newVid );
			count++;
		}
	}
	else if( vlanMode == CTC_VLAN_MODE_AGGREGATION )
	{
		show_ctc_onu_port_default_vlan( vty, devIdx, brdIdx, ethIdx );
		val = 0;
		if( (CTC_getEthPortVlanAggrGroup(devIdx, ethIdx, &val) == VOS_OK) &&
			(val != 0) )
		{
			showVlanAggrGroup( vty, val );
		}
	}
	else if( vlanMode == CTC_VLAN_MODE_TRUNK )
	{
		show_ctc_onu_port_default_vlan( vty, devIdx, brdIdx, ethIdx );
#if 0
		if( getLogicEntity( devIdx, &pentry ) == VOS_OK )
		{
			vty_out( vty, "  trunk_vlan=" );
			for( i=0; i<CTC_MAX_VLAN_FILTER_ENTRIES; i++ )
			{
				if( pentry->eth_vlan_trunk[portIdx][i] )
				{
					vty_out( vty, "%d ", pentry->eth_vlan_trunk[portIdx][i] );
				}
			}
			vty_out( vty, "\r\n" );
		}
#endif
		val = 0;
		if( (CTC_getEthPortVlanTrunkGroup(devIdx, ethIdx, &val) == VOS_OK) &&
			(val != 0) )
		{
			showVlanTrunkGroup( vty, val );
		}
	}
	else
	{
		vty_out( vty, " error\r\n" );
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/*add by shixh@2007/08/02*/
/*show  vlan*/
DEFUN(show_ctc_vlan,
	show_ctc_vlan_cmd,
	"show ctc vlan {<port_list>}*1",
	SHOW_STR
	CTC_STR
	"show vlan information\n"
	"Port number\n")
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
	ULONG pon;
	ULONG onu;
	ULONG portNum;

	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if( argc == 0 )
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			show_ctc_onu_port_vlan( vty, devIdx, brdIdx, ethIdx );
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ethIdx )
		{
			if(ethIdx>portNum)
			{
				vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ethIdx )
		{
			show_ctc_onu_port_vlan( vty, devIdx, brdIdx, ethIdx );
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	
	return CMD_SUCCESS;
}
			
/*add by shixh@2007/08/02*/
/*set ctc  port_vlan_mode*/
DEFUN(set_port_vlan_mode,
	set_port_vlan_mode_cmd,
	"ctc vlan-mode [all|<port_list>] [transparent|tag|translation|aggregation|trunk]",
	CTC_STR
	"set vlan mode\n"
	"All port\n"
	"port number\n"
	"transparent vlan mode\n"
	"tag vlan mode\n"
	"translation vlan mode\n"
	"aggregation vlan mode"
	"trunk vlan mode" )
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
	ULONG pon;
	ULONG onu;

	ULONG vlanMode = CTC_VLAN_MODE_TRANSPARENT;
	ULONG portNum = 0;

	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		(portNum > MAX_ETH_PORT_NUM) )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	if(VOS_StriCmp(argv[1], "tag")==0)
		vlanMode = CTC_VLAN_MODE_TAG;
	else if(VOS_StriCmp(argv[1], "translation")==0)
		vlanMode=CTC_VLAN_MODE_TRANSLATION;
	else if(VOS_StriCmp(argv[1], "aggregation")==0)
		vlanMode=CTC_VLAN_MODE_AGGREGATION;
	else if(VOS_StriCmp(argv[1], "trunk")==0)
		vlanMode=CTC_VLAN_MODE_TRUNK;
	/*else
		vlanMode = CTC_VLAN_MODE_TRANSPARENT;*/

	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(setEthPortVlanMode( devIdx, brdIdx, ethIdx, vlanMode)==VOS_ERROR)
			{
				vty_out(vty, "set port=%d vlan-mode error!\r\n", ethIdx);
			}
			if( vlanMode == CTC_VLAN_MODE_TRANSPARENT )
			{
				CTC_ethPortVlanAction_set(devIdx, ethIdx, vlanMode);
			}
		}
	}	
	else
  	{
  	     	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(ethIdx>portNum)
			{
				vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
  	     	}
		END_PARSE_PORT_LIST_TO_PORT();
			
  	     	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(setEthPortVlanMode( devIdx, brdIdx, ethIdx, vlanMode)==VOS_ERROR)
			{
		    		vty_out(vty, "set port=%d vlan-mode error!\r\n", ethIdx);
			}
			if( vlanMode == CTC_VLAN_MODE_TRANSPARENT )
			{
				CTC_ethPortVlanAction_set(devIdx, ethIdx, vlanMode);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return CMD_SUCCESS;
}

/*add by shixh@2007/08/07*/
/*set ctc  vlan_port_vlan-mode_tag*/
DEFUN(set_vlan_default,
	set_vlan_default_cmd,
       "ctc vlan-default [all|<port_list>] vid <1-4094> {[pri] <0-7>}*1 {[cfi] <0-1>}*1 {[tpid] <hexvalue>}*1",
	CTC_STR
	"set default vlan\n"
	"All port\n"
	"input port number\n"
	"input vlan id\n"
	"priority\n"
	"input priority\n"
	"cfi\n"
	"input cfi\n"
	"tpid\n"
	"input tpid in hex-value\n")
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
	ULONG portNum;
	ULONG pon;
	ULONG onu;
   
	ULONG Pri = 0;
	ULONG Cfi = 0;
	ULONG Tpid = 0x8100;
	ULONG vid;
   
	int i;
	char *pToken;
   
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	vid = VOS_AtoI( argv[1] );
	if( (vid>4094) || (vid<1) )
		return CMD_WARNING;

	for(i=2;i<argc;i++)
	{
		if( VOS_StriCmp( argv[i], "pri") == 0 )
		{
			Pri=VOS_AtoI(argv[i+1]);
		}
		else if( VOS_StriCmp(argv[i], "cfi") == 0 )
		{
			Cfi=VOS_AtoI(argv[i+1]);
		}
	       else if( VOS_StriCmp(argv[i], "tpid") == 0 )
	      	{
			Tpid = VOS_StrToUL( argv[i+1], &pToken, 16 );
	      	}
	        else
	      	{
	        	/*vty_out(vty, " para error!!\r\n");	*/
	      	}
		i++;           
	}


	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(setEthPortVlanTagVid( devIdx, brdIdx, ethIdx, vid  )==VOS_ERROR)
				vty_out(vty, "set port=%d vlan tag vid error!\r\n", ethIdx);		
			if(setEthPortVlanTagPri( devIdx, brdIdx, ethIdx, 0  )==VOS_ERROR)
		       	vty_out(vty, "set  port=%d vlan PRI error!\r\n", ethIdx);	
			if(setEthPortVlanTagCfi( devIdx, brdIdx, ethIdx, 0 )==VOS_ERROR)
		       	vty_out(vty, "set port=%d vlan CFI error!\r\n", ethIdx);	
			if(setEthPortVlanTagTpid( devIdx, brdIdx, ethIdx, Tpid )==VOS_ERROR)
		       	vty_out(vty, "set port=%d vlan Tpid error!\r\n", ethIdx);	
		}
   	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(ethIdx>portNum)
			{
				vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(setEthPortVlanTagVid( devIdx, brdIdx, ethIdx, vid  )==VOS_ERROR)
				vty_out(vty, "set port=%d vlan tag vid error!\r\n", ethIdx);	
			if(setEthPortVlanTagPri( devIdx, brdIdx, ethIdx, 0  )==VOS_ERROR)
				vty_out(vty, "set port=%d vlan PRI error!\r\n", ethIdx);	
			if(setEthPortVlanTagCfi( devIdx, brdIdx, ethIdx, 0 )==VOS_ERROR)
				vty_out(vty, "set port=%d vlan CFI error!\r\n", ethIdx);	
			if(setEthPortVlanTagTpid( devIdx, brdIdx, ethIdx, 0x8100  )==VOS_ERROR)
				vty_out(vty, "set port=%d vlan Tpid error!\r\n", ethIdx);	
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	
       return   CMD_SUCCESS;
}

/*add by shixh@2007/08/03*/
/*set  vlan_translation*/
DEFUN(set_vlan_translation,
	  set_vlan_translation_cmd,
         "ctc vlan-translation [all|<port_list>] old-vid <1-4094> new-vid <1-4094>",
	CTC_STR
	  "set vlan translation\n"
	  "All port\n"
	  "port number\n"
	  "old-vid\n"
	  "input old vid\n"
	  "new-vid\n"
	  "input new vid\n"
	  )
{
      ulong_t devIdx;
      ulong_t brdIdx;
      ulong_t ethIdx;

	ulong_t portNum;
	ulong_t pon;
	ulong_t onu;
       ULONG inVid;
       ULONG newVid;
	   
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	inVid=VOS_AtoI( argv[1] );
	newVid=VOS_AtoI( argv[2] );
	if( (inVid > 4094) || (inVid == 0) || (newVid > 4094) || (newVid == 0) )
	{
		return CMD_WARNING;
	}	
	
	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(setEthPortVlanTraStatus( devIdx, brdIdx, ethIdx,inVid, 5) == VOS_ERROR )
				vty_out(vty, " setEthPortVlanTraStatus set  error!\r\n");
			if(setEthPortVlanTranNewVid( devIdx, brdIdx, ethIdx,inVid,newVid)==VOS_ERROR)
				vty_out(vty, " set  error!\r\n");        
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if(ethIdx>portNum)
		{
			vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();

		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if( setEthPortVlanTraStatus( devIdx, brdIdx, ethIdx,inVid, 5) == VOS_OK )
			{
	              	/*vty_out(vty, " port=%d  invid%d vlan  translation is newVid%d\r\n", ethIdx,inVid,newVid);*/
			}
			else
				vty_out(vty, "setEthPortVlanTraStatus set error!\r\n");
					
			if(setEthPortVlanTranNewVid( devIdx, brdIdx, ethIdx,inVid,newVid)==VOS_OK)
			{
			}
			else
				vty_out(vty, " set error!\r\n");
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return  CMD_SUCCESS;
}


/*add by shixh@2007/08/03*/
/*undo vlan-translation */
DEFUN(undo_vlan_translation,
	  undo_vlan_translation_cmd,
         "undo ctc vlan-translation [all|<port_list>] old-vid <1-4094>",
	  "undo operation\n"
	CTC_STR
	  "undo vlan translation\n"
	  "All port\n"
	  "port number\n"
	  "old-vid\n"
	  "input old vid\n"
	  )
	 
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
    
	ULONG portNum;
	ULONG pon;
	ULONG onu;
       ULONG inVid;
	   
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	inVid=VOS_AtoI( argv[1] );
	if( (inVid > 4094) || (inVid == 0) )
		return CMD_WARNING;
	
	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if(setEthPortVlanTraStatus( devIdx, brdIdx, ethIdx,inVid,6)==VOS_OK)
			{
			}
			else
			{
				vty_out(vty, " vlan translation table is empty!\r\n");
			}
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
				vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		{
			if(setEthPortVlanTraStatus( devIdx, brdIdx, ethIdx,inVid,6)==VOS_OK)
			{
			}
			else
			{
				vty_out(vty, " port=%d vlan translation table is empty!\r\n", ethIdx);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return     CMD_SUCCESS;
}

DEFUN(apply_vlan_set,
	  apply_vlan_set_cmd,
         "ctc vlan-action [all|<port_list>] [apply|clear|update]",
	CTC_STR
	  "vlan action\n"
	  "port number all\n"
	  "input port number\n"
	  "set vlan data to onu\n"
	  "clear onu vlan\n"
	  "get vlan data from onu\n")
{
	ULONG devIdx;
	ULONG brdIdx;
	ULONG ethIdx;
    
	ULONG portNum;
	ULONG pon;
	ULONG onu;
       ULONG action;
	   
	if(parse_onu_devidx_command_parameter(vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "no ethernet port\r\n");
		return CMD_WARNING;
	}

	if( VOS_StriCmp(argv[1], "apply") == 0 )
		action = 3;
	else if( VOS_StriCmp(argv[1], "clear") == 0 )
		action = 4;
	else if( VOS_StriCmp(argv[1], "update") == 0 )
		action = 2;
	else
		return CMD_WARNING;
	
	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			if( setEthPortVlanAction(devIdx, brdIdx, ethIdx, action) == VOS_ERROR )
			{
				vty_out(vty, " port=%d,setEthPortVlanAction ERR!\r\n", ethIdx);
			}
		}
	}
	else
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ethIdx )
			if(ethIdx>portNum)
			{
				vty_out(vty, "out of range,the toal ethernet port Num is %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ethIdx )
		{
			if( setEthPortVlanAction(devIdx, brdIdx, ethIdx, action) == VOS_ERROR )
			{
				vty_out(vty, " port=%d,setEthPortVlanAction ERR!\r\n", ethIdx);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return     CMD_SUCCESS;
}

LONG  CT_VlanCli_Init()
{
	install_element ( ONU_CTC_NODE, &show_ctc_vlan_cmd );
	install_element ( ONU_CTC_NODE, &set_port_vlan_mode_cmd );
	install_element ( ONU_CTC_NODE, &set_vlan_default_cmd);
	install_element ( ONU_CTC_NODE, &set_vlan_translation_cmd);
	install_element ( ONU_CTC_NODE, &undo_vlan_translation_cmd);
	install_element ( ONU_CTC_NODE, &apply_vlan_set_cmd);

	CT_VlanAggrCli_Init();
	CT_VlanTrunkCli_Init();
	CT_Alarm_Init();
	
	return VOS_OK;
}


/*----------------------------------------------------------------------------*/

/* CLI */

/*----------------------------------------------------------------------------*/
#endif
#endif

