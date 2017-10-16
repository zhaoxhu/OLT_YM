

#include "syscfg.h"

#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"


#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "vos/vospubh/vos_ctype.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "addr_aux.h"
#include "linux/inetdevice.h"

#include "ifm/ifm_type.h"
#include "ifm/ifm_pub.h"
#include "ifm/ifm_gtable.h"
#include "ifm/ifm_aux.h"
#include "ifm/ifm_act.h"
#include "ifm/ifm_cli.h"
#include "ifm/ifm_task.h"
#include "ifm/ifm_lock.h"
#include "mn_oam.h"

#include "ifm/ifm_debug.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"
#include "manage/snmp/sn_tc.h"

#include  "GwttOam/OAM_gw.h"

#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"
#include "tdm/mn_tdm.h"
#include "gwEponSys.h"
#include "onu/ExtBoardType.h"

#include "e1_apis.h"
#include "e1_oam.h"
#include "E1_MIB.h"
#include "E1DataSave.h"

#define E1_NODE ATM_SUBIF_NODE




/* begin: tdmµ÷ÊÔÃüÁî */
DEFUN  (
		tdm_E1LinkTable_get,
		tdm_E1LinkTable_get_cmd,
		"tdm-E1LinkTable-get <tableIndex>",
		"get E1LinkTable\n"
		"the index of e1LinkTable 0~23\n"
		)
{
	ULONG idxs[3];
	e1LinkTable_row_entry e1LinkTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1LinkTable_get(idxs, &e1LinkTableEntry) )
	{
		vty_out( vty, "command executing failed\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "onuDevId: %ld\r\n", e1LinkTableEntry.onuDevId);
	vty_out( vty, "onuE1SlotId: %d\r\n", e1LinkTableEntry.onuE1SlotId);
	vty_out( vty, "onuE1Id: %d\r\n", e1LinkTableEntry.onuE1Id);
	vty_out( vty, "eponE1LocalEnable: %d\r\n", e1LinkTableEntry.eponE1LocalEnable);
	vty_out( vty, "eponE1Description: %s\r\n", e1LinkTableEntry.eponE1Description);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1LinkTable_getNext,
		tdm_E1LinkTable_getNext_cmd,
		"tdm-E1LinkTable-getnext <tableIndex>",
		"getNext e1LinkTable\n"
		"the index of e1LinkTable 0~23\n"
		)
{
	ULONG idxs[3];
	e1LinkTable_row_entry e1LinkTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1LinkTable_getNext(idxs, &e1LinkTableEntry) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "onuDevId: %ld\r\n", e1LinkTableEntry.onuDevId);
	vty_out( vty, "onuE1SlotId: %d\r\n", e1LinkTableEntry.onuE1SlotId);
	vty_out( vty, "onuE1Id: %d\r\n", e1LinkTableEntry.onuE1Id);
	vty_out( vty, "eponE1LocalEnable: %d\r\n", e1LinkTableEntry.eponE1LocalEnable);
	vty_out( vty, "eponE1Description: %s\r\n", e1LinkTableEntry.eponE1Description);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1LinkTable_set,
		tdm_E1LinkTable_set_cmd,
		"tdm-E1LinkTable-set <leafIndex> <tableIndex> <setValue>",
		"set e1LinkTable leaf\n"
		"the leaf index of e1LinkTable 1,2,3,4,5\n"
		"the index of e1LinkTable 0~23\n"
		"the leaf value of e1LinkTable leaf\n"
		)
{
	ULONG idxs[3];

	/*if (3 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[1]);

	if (VOS_AtoL(argv[0]) == 5)
	{
		/*E1_ERROR_INFO_PRINT(("description: %s, strlen=%d\r\n", argv[2], strlen(argv[2])));*/

		if ( VOS_OK != tdm_e1LinkTable_set(VOS_AtoL(argv[0]), idxs, 0, argv[2]) )
		{
			vty_out( vty, "command executing failed!\r\n");
			return CMD_WARNING;
		}
	} 
	else
	{
		if ( VOS_OK != tdm_e1LinkTable_set(VOS_AtoL(argv[0]), idxs, VOS_AtoL(argv[2]), NULL) )
		{
			vty_out( vty, "command executing failed!\r\n");
			return CMD_WARNING;
		}
	}

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1LinkTable_rowset,
		tdm_E1LinkTable_rowset_cmd,
		"tdm-E1LinkTable-rowset <tableIndex> <onuDevId> <onuE1SlotId> <onuE1Id> <eponE1LocalEnable> <eponE1Description>",
		"rowset E1LinkTable\n"
		"the index of e1LinkTable 0~23\n"		
		"onuDevId is a large number\n"
		"onuE1SlotId 0~6\n"
		"onuE1Id 0~3\n"
		"eponE1LocalEnable 1 or 2\n"
		"eponE1Description < 32 Bytes\n"
		)
{
	ULONG idxs[3];

	/*if (6 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1LinkTable_rowset( idxs, VOS_AtoL(argv[1]), VOS_AtoL(argv[2]), VOS_AtoL(argv[3]), VOS_AtoL(argv[4]), argv[5] ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}


DEFUN  (
		tdm_E1PortTable_get,
		tdm_E1PortTable_get_cmd,
		"tdm-E1PortTable-get <tableIndex>",
		"get e1PortTable\n"
		"the index of e1PortTable 0~23\n"
		)
{
	ULONG idxs[3];
	e1PortTable_row_entry e1PortTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1PortTable_get(idxs, &e1PortTableEntry) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponE1PortAlarmStatus: 0x%04x\r\n", e1PortTableEntry.eponE1PortAlarmStatus);
	vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", e1PortTableEntry.eponE1PortAlarmMask);
	vty_out( vty, "eponE1Loop: %d\r\n", e1PortTableEntry.eponE1Loop);
	vty_out( vty, "eponE1TxClock: %d\r\n", e1PortTableEntry.eponE1TxClock);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1PortTable_getNext,
		tdm_E1PortTable_getNext_cmd,
		"tdm-E1PortTable-getnext <tableIndex>",
		"getNext e1PortTable\n"
		"the index of e1PortTable 0~23\n"
		)
{
	ULONG idxs[3];
	e1PortTable_row_entry e1PortTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1PortTable_getNext(idxs, &e1PortTableEntry) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponE1PortAlarmStatus: 0x%04x\r\n", e1PortTableEntry.eponE1PortAlarmStatus);
	vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", e1PortTableEntry.eponE1PortAlarmMask);
	vty_out( vty, "eponE1Loop: %d\r\n", e1PortTableEntry.eponE1Loop);
	vty_out( vty, "eponE1TxClock: %d\r\n", e1PortTableEntry.eponE1TxClock);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1PortTable_set,
		tdm_E1PortTable_set_cmd,
		"tdm-E1PortTable-set <leafIndex> <tableIndex> <setValue>",
		"set E1PortTable leaf\n"
		"the leaf index of e1PortTable 2,3,4\n"
		"the index of e1PortTable 0~23\n"
		"the leaf value of e1PortTable leaf\n"
		)
{
	ULONG idxs[3];

	/*if (3 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[1]);

	if ( VOS_OK != tdm_e1PortTable_set(VOS_AtoL(argv[0]), idxs, VOS_AtoL(argv[2])) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1PortTable_rowset,
		tdm_E1PortTable_rowset_cmd,
		"tdm-E1PortTable-rowset <tableIndex>  <eponE1PortAlarmMask> <eponE1Loop> <eponE1TxClock>",
		"rowset E1PortTable\n"
		"the index of e1PortTable 0~23\n"
		"eponE1PortAlarmMask is hex\n"
		"eponE1Loop\n"
		"eponE1TxClock 0~3\n"
		)
{
	ULONG idxs[3];

	/*if (4 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1PortTable_rowset( idxs, VOS_AtoL(argv[1]), VOS_AtoL(argv[2]), VOS_AtoL(argv[3]) ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}


DEFUN  (
		tdm_E1VlanTable_get,
		tdm_E1VlanTable_get_cmd,
		"tdm-E1VlanTable-get <tableIndex>",
		"get E1VlanTable\n"
		"the index of e1VlanTable 0~2\n"
		)
{
	ULONG idxs[3];
	e1VlanTable_row_entry e1VlanTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1VlanTable_get(idxs, &e1VlanTableEntry) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponVlanEnable: %d\r\n", e1VlanTableEntry.eponVlanEnable);
	vty_out( vty, "eponVlanPri: %d\r\n", e1VlanTableEntry.eponVlanPri);
	vty_out( vty, "eponVlanId: %d\r\n", e1VlanTableEntry.eponVlanId);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1VlanTable_getNext,
		tdm_E1VlanTable_getNext_cmd,
		"tdm-E1VlanTable-getnext <tableIndex>",
		"getNext E1VlanTable\n"
		"the index of e1VlanTable 0~2\n"
		)
{
	ULONG idxs[3];
	e1VlanTable_row_entry e1VlanTableEntry;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1VlanTable_getNext(idxs, &e1VlanTableEntry) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponVlanEnable: %d\r\n", e1VlanTableEntry.eponVlanEnable);
	vty_out( vty, "eponVlanPri: %d\r\n", e1VlanTableEntry.eponVlanPri);
	vty_out( vty, "eponVlanId: %d\r\n", e1VlanTableEntry.eponVlanId);

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1VlanTable_set,
		tdm_E1VlanTable_set_cmd,
		"tdm-E1VlanTable-set <leafIndex> <tableIndex> <setValue>",
		"set E1VlanTable leaf\n"
		"the leaf index of e1VlanTable 1,2,3\n"
		"the index of e1VlanTable 0~2\n"
		"the leaf value of e1VlanTable leaf\n"
		)
{
	ULONG idxs[3];

	/*if (3 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[1]);

	if ( VOS_OK != tdm_e1VlanTable_set(VOS_AtoL(argv[0]), idxs, VOS_AtoL(argv[2])) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		tdm_E1VlanTable_rowset,
		tdm_E1VlanTable_rowset_cmd,
		"tdm-E1VlanTable-rowset <tableIndex>  <eponVlanEnable> <eponVlanPri> <eponVlanId>",
		"rowset E1VlanTable\n"
		"the index of e1VlanTable 0~2\n"
		"eponVlanEnable is 1 or 2\n"
		"eponVlanPri 0-7\n"
		"eponVlanId 1~4094\n"
		)
{
	ULONG idxs[3];

	/*if (4 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = VOS_AtoL(argv[0]);

	if ( VOS_OK != tdm_e1VlanTable_rowset( idxs, VOS_AtoL(argv[1]), VOS_AtoL(argv[2]), VOS_AtoL(argv[3]) ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		onuE1Info_get,
		onuE1Info_get_cmd,
		"onuE1Info-get <onuDevIdx>",
		"get onu e1 info\n"
		"the index of Onu \n"
		)
{
	ULONG idxs[2], i;
	onuE1Info *pOnuE1Info;

	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	pOnuE1Info = VOS_Malloc(sizeof(onuE1Info), MODULE_TDM_COMM );
	if( NULL == pOnuE1Info )
		return CMD_WARNING;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getOnuE1Info(idxs, VOS_AtoL(argv[0]), pOnuE1Info) )
	{
		vty_out( vty, "command executing failed!\r\n");
		VOS_Free(pOnuE1Info);
		return CMD_WARNING;
	}

	vty_out( vty, "onuValidE1Count: %d\r\n\r\n", pOnuE1Info->onuValidE1Count);

	for (i = 0; i < pOnuE1Info->onuValidE1Count; i++)
	{
		vty_out( vty, "onuE1Slot: %d\r\n", pOnuE1Info->onuEachE1Info[i].onuE1Slot);
		vty_out( vty, "onuE1Index: %d\r\n", pOnuE1Info->onuEachE1Info[i].onuE1Index);
		vty_out( vty, "tdmE1Enable: %d\r\n", pOnuE1Info->onuEachE1Info[i].tdmE1Enable);
		vty_out( vty, "tdmSgIfId: %d\r\n", pOnuE1Info->onuEachE1Info[i].tdmSgIfId);
		vty_out( vty, "tdmE1Id: %d\r\n", pOnuE1Info->onuEachE1Info[i].tdmE1Id);
		vty_out( vty, "tdmVlanEnable: %d\r\n", pOnuE1Info->onuEachE1Info[i].tdmVlanEnable);
		vty_out( vty, "tdmVlanTag: 0x%04x\r\n", pOnuE1Info->onuEachE1Info[i].tdmVlanTag);
		vty_out( vty, "tdmAlarmStat: 0x%04x\r\n", pOnuE1Info->onuEachE1Info[i].tdmAlarmStat);
		vty_out( vty, "tdmAlarmMask: 0x%04x\r\n", pOnuE1Info->onuEachE1Info[i].tdmAlarmMask);
		vty_out( vty, "tdmE1Loop: %d\r\n", pOnuE1Info->onuEachE1Info[i].tdmE1Loop);
		vty_out( vty, "tdmE1TxClk: %d\r\n\r\n", pOnuE1Info->onuEachE1Info[i].tdmE1TxClk);
	}
	VOS_Free(pOnuE1Info);

	return CMD_SUCCESS;
}

DEFUN  (
		allE1Info_get,
		allE1Info_get_cmd,
		"allE1Info-get {<fpgaIndex>}*1",
		"get all tdm e1 info\n"
		"the index of fpga 0-2\n"
		)
{
	ULONG idxs[2], i, j, count;
	AllE1Info *pAllE1Info = NULL;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	pAllE1Info = (AllE1Info *)VOS_Malloc(sizeof(AllE1Info), MODULE_TDM_COMM );
	if( NULL == pAllE1Info )
	{
		return CMD_WARNING;
	}
#if 0 /* modified by xieshl 20100202 */
	switch(argc)
	{
	case 0:
		if ( VOS_OK != tdm_getAllE1Info( idxs, 0xFF, pAllE1Info ) )
		{
			vty_out( vty, "command executing failed! tdm_getAllE1Info() error!\r\n");
			VOS_Free(pAllE1Info);
			return CMD_WARNING;
		}
		break;
	case 1:
		if ( VOS_OK != tdm_getAllE1Info( idxs, VOS_AtoL(argv[0]), pAllE1Info ) )
		{
			vty_out( vty, "command executing failed! tdm_getAllE1Info() error!\r\n");
			VOS_Free(pAllE1Info);
			return CMD_WARNING;
		}
		break;
	default:
		vty_out( vty, "command executing failed!\r\n");
		VOS_Free(pAllE1Info);
		return CMD_WARNING;
		break;
	}

	vty_out( vty, "print all e1 info now!\r\n\r\n");

	vty_out( vty, "type: %d\r\n\r\n", pAllE1Info->type);

	if (0xFF == pAllE1Info->type)
	{
		for (i = 0; i < TDM_FPGA_MAX; i++)
		{
			vty_out( vty, "fpgaIndex: %d\r\n", pAllE1Info->fpgaE1Info[i].fpgaIndex);
			vty_out( vty, "eponVlanEnable: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanEnable);
			vty_out( vty, "eponVlanPri: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanPri);
			vty_out( vty, "eponVlanId: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanId);
			vty_out( vty, "*fpgaValidE1Count: %d\r\n", pAllE1Info->fpgaE1Info[i].fpgaValidE1Count);

			for (j = 0; j < pAllE1Info->fpgaE1Info[i].fpgaValidE1Count; j++)
			{
				vty_out( vty, "e1Index: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].e1Index);

				vty_out( vty, "onuDevId: %ld\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId);
				vty_out( vty, "onuE1SlotId: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuE1SlotId);
				vty_out( vty, "onuE1Id: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuE1Id);
				vty_out( vty, "eponE1LocalEnable: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1LocalEnable);
				/*vty_out( vty, "eponE1Description: %s\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1Description);*/
				vty_out( vty, "eponE1PortAlarmStatus: 0x%04xc\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1PortAlarmStatus);
				vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1PortAlarmMask);
				vty_out( vty, "eponE1Loop: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1Loop);
				vty_out( vty, "eponE1TxClock: %d\r\n\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1TxClock);
			}

			vty_out( vty, "\r\n");
		}
	} 
	else
	{
		vty_out( vty, "fpgaIndex: %d\r\n", pAllE1Info->fpgaE1Info[0].fpgaIndex);
		vty_out( vty, "eponVlanEnable: %d\r\n", pAllE1Info->fpgaE1Info[0].eponVlanEnable);
		vty_out( vty, "eponVlanPri: %d\r\n", pAllE1Info->fpgaE1Info[0].eponVlanPri);
		vty_out( vty, "eponVlanId: %d\r\n", pAllE1Info->fpgaE1Info[0].eponVlanId);
		vty_out( vty, "*fpgaValidE1Count: %d\r\n\r\n", pAllE1Info->fpgaE1Info[0].fpgaValidE1Count);

		for (j = 0; j < pAllE1Info->fpgaE1Info[0].fpgaValidE1Count; j++)
		{
			vty_out( vty, "e1Index: %d\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].e1Index);

			vty_out( vty, "onuDevId: %ld\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].onuDevId);
			vty_out( vty, "onuE1SlotId: %d\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].onuE1SlotId);
			vty_out( vty, "onuE1Id: %d\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].onuE1Id);
			vty_out( vty, "eponE1LocalEnable: %d\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1LocalEnable);
			/*vty_out( vty, "eponE1Description: %s\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1Description);*/
			vty_out( vty, "eponE1PortAlarmStatus: 0x%04xc\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1PortAlarmStatus);
			vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1PortAlarmMask);
			vty_out( vty, "eponE1Loop: %d\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1Loop);
			vty_out( vty, "eponE1TxClock: %d\r\n\r\n", pAllE1Info->fpgaE1Info[0].eachE1Info[j].eponE1TxClock);
		}
	}
#endif
	if( argc == 1 )
	{
		i = VOS_AtoL(argv[0]);
		count = 1;
	}
	else
	{
		i = 0xFF;
		count = TDM_FPGA_MAX;
	}
	if ( VOS_OK != tdm_getAllE1Info( idxs, i, pAllE1Info ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}	

	for (i = 0; i < count; i++)
	{
		vty_out( vty, "fpgaIndex: %d\r\n", pAllE1Info->fpgaE1Info[i].fpgaIndex);
		vty_out( vty, "eponVlanEnable: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanEnable);
		vty_out( vty, "eponVlanPri: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanPri);
		vty_out( vty, "eponVlanId: %d\r\n", pAllE1Info->fpgaE1Info[i].eponVlanId);
		vty_out( vty, "*fpgaValidE1Count: %d\r\n", pAllE1Info->fpgaE1Info[i].fpgaValidE1Count);

		for (j = 0; j < pAllE1Info->fpgaE1Info[i].fpgaValidE1Count; j++)
		{
			vty_out( vty, "e1Index: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].e1Index);

			vty_out( vty, "onuDevId: %ld\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuDevId);
			vty_out( vty, "onuE1SlotId: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuE1SlotId);
			vty_out( vty, "onuE1Id: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].onuE1Id);
			vty_out( vty, "eponE1LocalEnable: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1LocalEnable);
			/*vty_out( vty, "eponE1Description: %s\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1Description);*/
			vty_out( vty, "eponE1PortAlarmStatus: 0x%04xc\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1PortAlarmStatus);
			vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1PortAlarmMask);
			vty_out( vty, "eponE1Loop: %d\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1Loop);
			vty_out( vty, "eponE1TxClock: %d\r\n\r\n", pAllE1Info->fpgaE1Info[i].eachE1Info[j].eponE1TxClock);
		}

		vty_out( vty, "\r\n");
	}

	VOS_Free(pAllE1Info);
	return CMD_SUCCESS;
}

/* end: tdmµ÷ÊÔÃüÁî */
#if OAM
#endif
/* begin: oamµ÷ÊÔÃüÁî */
DEFUN  (
		setOnuE1Link,
		setOnuE1Link_cmd,
		"setOnuE1Link  <onuDevIdx> <param>",
		"set onu e1 link\n"
		"onu device index"
		"1: set 1 e1 port; 2: set max e1 ports\n"
		)
{
	ULONG i;
	OAM_OnuE1Link oam_OnuE1Link;
	UCHAR mac[6] = {0x00, 0x0F, 0xE9, 0x04, 0x03, 0x00};

	/*if (2 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

       memset((UCHAR *)&oam_OnuE1Link, 0, sizeof(OAM_OnuE1Link));
	oam_OnuE1Link.MsgType = SET_ONU_E1_LINK_REQ;

	if ( 1 == VOS_AtoL(argv[1]) )
	{
		oam_OnuE1Link.E1PortTotalCount = 1;

		oam_OnuE1Link.oam_OnuE1Link[0].E1PortIdx = 2;
		oam_OnuE1Link.oam_OnuE1Link[0].E1SlotIdx = 5;
		oam_OnuE1Link.oam_OnuE1Link[0].E1Enable = OAM_E1_ENABLE;
		memcpy(oam_OnuE1Link.oam_OnuE1Link[0].DesMac, mac, 6);
		mac[5] += 0x80;
		memcpy(oam_OnuE1Link.oam_OnuE1Link[0].SrcMac, mac, 6);
		oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = OAM_E1_VLAN_DISABLE;
		oam_OnuE1Link.oam_OnuE1Link[0].VlanTag = 0xCFFD;/* pri=6  id=4093 */
	} 
	else
	{
		oam_OnuE1Link.E1PortTotalCount = MAX_ONU_E1;

		for (i = 0; i < MAX_ONU_E1; i++)
		{
			oam_OnuE1Link.oam_OnuE1Link[i].E1PortIdx = i % MAX_ONU_BOARD_E1;
			oam_OnuE1Link.oam_OnuE1Link[i].E1SlotIdx = i % MAX_ONU_E1_SLOT_ID;

			if (MIN_ONU_E1_SLOT_ID > oam_OnuE1Link.oam_OnuE1Link[i].E1SlotIdx)
			{
				oam_OnuE1Link.oam_OnuE1Link[i].E1SlotIdx = MIN_ONU_E1_SLOT_ID;
			}

			oam_OnuE1Link.oam_OnuE1Link[i].E1Enable = i % 2;
			memcpy(oam_OnuE1Link.oam_OnuE1Link[i].DesMac, mac, 6);
			mac[5] += 0x80;
			memcpy(oam_OnuE1Link.oam_OnuE1Link[i].SrcMac, mac, 6);
			mac[5] -= 0x80;
			oam_OnuE1Link.oam_OnuE1Link[i].VlanEable = i % 2;
			oam_OnuE1Link.oam_OnuE1Link[i].VlanTag   = 51213/* 0xC80D */ + i;/* pri=6  id=2061+i */
		}
	}

	if ( V2R1_E1_OK != SetOnuE1Link( VOS_AtoL(argv[0]), &oam_OnuE1Link ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		setOnuE1Vlan,
		setOnuE1Vlan_cmd,
		"setOnuE1Vlan  <onuDevIdx> <param>",
		"set onu e1 vlan\n"
		"onu device index\n"
		"1: set 1 e1 port; 2: set max e1 ports\n"
		)
{
	ULONG i;
	OAM_OnuE1Vlan oam_OnuE1Vlan;

	/*if (2 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	oam_OnuE1Vlan.MsgType = SET_ONU_E1_VLAN_REQ;

	if ( 1 == VOS_AtoL(argv[1]) )
	{
		oam_OnuE1Vlan.E1PortTotalCount = 1;

		oam_OnuE1Vlan.oam_OnuE1Vlan[0].E1PortIdx = 2;
		oam_OnuE1Vlan.oam_OnuE1Vlan[0].E1SlotIdx = 5;
		oam_OnuE1Vlan.oam_OnuE1Vlan[0].VlanEable = OAM_E1_VLAN_ENABLE;
		oam_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag = 0xCFFD;/* pri=6  id=4093 */
	} 
	else
	{
		oam_OnuE1Vlan.E1PortTotalCount = MAX_ONU_E1;

		for (i = 0; i < MAX_ONU_E1; i++)
		{
			oam_OnuE1Vlan.oam_OnuE1Vlan[i].E1PortIdx = i % MAX_ONU_BOARD_E1;
			oam_OnuE1Vlan.oam_OnuE1Vlan[i].E1SlotIdx = i % MAX_ONU_E1_SLOT_ID;

			if (MIN_ONU_E1_SLOT_ID > oam_OnuE1Vlan.oam_OnuE1Vlan[i].E1SlotIdx)
			{
				oam_OnuE1Vlan.oam_OnuE1Vlan[i].E1SlotIdx = MIN_ONU_E1_SLOT_ID;
			}

			oam_OnuE1Vlan.oam_OnuE1Vlan[i].VlanEable = i % 2;
			oam_OnuE1Vlan.oam_OnuE1Vlan[i].VlanTag   = 51213/* 0xC80D */ + i;/* pri=6  id=2061+i */
		}
	}

	if ( V2R1_E1_OK != SetOnuE1Vlan( VOS_AtoL(argv[0]), &oam_OnuE1Vlan ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		setOnuE1AlarmMask,
		setOnuE1AlarmMask_cmd,
		"setOnuE1AlarmMask  <onuDevIdx> <param>",
		"set onu e1 alarm mask\n"
		"onu device index"
		"1: set 1 e1 port; 2: set max e1 ports\n"
		)
{
	ULONG i;
	OAM_OnuE1AlarmMask oam_OnuE1AlarmMask;

	/*if (2 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	oam_OnuE1AlarmMask.MsgType = SET_ONU_E1_ALARMMASK_REQ;

	if ( 1 == VOS_AtoL(argv[1]) )
	{
		oam_OnuE1AlarmMask.E1PortTotalCount = 1;

		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1PortIdx = 2;
		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].E1SlotIdx = 5;
		oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[0].AlarmMask = 0xABCD;
	} 
	else
	{
		oam_OnuE1AlarmMask.E1PortTotalCount = MAX_ONU_E1;

		for (i = 0; i < MAX_ONU_E1; i++)
		{
			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[i].E1PortIdx = i % MAX_ONU_BOARD_E1;
			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[i].E1SlotIdx = i % MAX_ONU_E1_SLOT_ID;

			if (MIN_ONU_E1_SLOT_ID > oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[i].E1SlotIdx)
			{
				oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[i].E1SlotIdx = MIN_ONU_E1_SLOT_ID;
			}

			oam_OnuE1AlarmMask.oam_OnuE1AlarmMask[i].AlarmMask = i + 1;
		}
	}

	if ( V2R1_E1_OK != SetOnuE1AlarmMask( VOS_AtoL(argv[0]), &oam_OnuE1AlarmMask ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		setOnuE1ALL,
		setOnuE1ALL_cmd,
		"setOnuE1ALL  <onuDevIdx> <param>",
		"set onu e1 all e1 info\n"
		"onu device index"
		"1: set 1 e1 port; 2: set max e1 ports\n"
		)
{
	ULONG i;
	OAM_OnuE1Info oam_OnuE1Info;
	UCHAR lmac[6] = {0x00, 0x0F, 0xE9, 0x04, 0x03, 0x00};
	UCHAR rmac[6] = {0x00, 0x0F, 0xE9, 0x04, 0x03, 0x80};

	/*if (2 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/
	memset((UCHAR *)&oam_OnuE1Info, 0, sizeof(OAM_OnuE1Info));

	oam_OnuE1Info.MsgType = SET_ONU_E1_ALL_REQ;

	if ( 1 == VOS_AtoL(argv[1]) )
	{
		oam_OnuE1Info.E1PortTotalCount = 1;

		oam_OnuE1Info.oam_OnuE1Info[0].E1PortIdx = 2;
		oam_OnuE1Info.oam_OnuE1Info[0].E1SlotIdx = 5;
		oam_OnuE1Info.oam_OnuE1Info[0].E1Enable = OAM_E1_ENABLE;
		oam_OnuE1Info.oam_OnuE1Info[0].VlanEable = OAM_E1_VLAN_ENABLE;
		oam_OnuE1Info.oam_OnuE1Info[0].VlanTag = 0xCFFD;/* pri=6  id=4093 */
		oam_OnuE1Info.oam_OnuE1Info[0].ClockControl = 0;
		oam_OnuE1Info.oam_OnuE1Info[0].LoopControl = 0;
		oam_OnuE1Info.oam_OnuE1Info[0].AlarmStat = 0;
		oam_OnuE1Info.oam_OnuE1Info[0].AlarmMask = 0xABCD;

		memcpy(oam_OnuE1Info.oam_OnuE1Info[0].SrcMac, lmac, 6);
		memcpy(oam_OnuE1Info.oam_OnuE1Info[0].DesMac, rmac, 6);
	} 
	else
	{
		oam_OnuE1Info.E1PortTotalCount = MAX_ONU_E1;

		for (i = 0; i < MAX_ONU_E1; i++)
		{
			oam_OnuE1Info.oam_OnuE1Info[i].E1PortIdx = i % MAX_ONU_BOARD_E1;
			oam_OnuE1Info.oam_OnuE1Info[i].E1SlotIdx = i % MAX_ONU_E1_SLOT_ID;

			if (MIN_ONU_E1_SLOT_ID > oam_OnuE1Info.oam_OnuE1Info[i].E1SlotIdx)
			{
				oam_OnuE1Info.oam_OnuE1Info[i].E1SlotIdx = MIN_ONU_E1_SLOT_ID;
			}

			oam_OnuE1Info.oam_OnuE1Info[i].E1Enable = i % 2;
			oam_OnuE1Info.oam_OnuE1Info[i].VlanEable = i % 2;
			oam_OnuE1Info.oam_OnuE1Info[i].VlanTag = 51213/* 0xC80D */ + i;/* pri=6  id=2061+i */
			oam_OnuE1Info.oam_OnuE1Info[i].ClockControl = 0;
			oam_OnuE1Info.oam_OnuE1Info[i].LoopControl = 0;
			oam_OnuE1Info.oam_OnuE1Info[i].AlarmStat = 0;
			oam_OnuE1Info.oam_OnuE1Info[i].AlarmMask = i + 1;

			memcpy(oam_OnuE1Info.oam_OnuE1Info[i].SrcMac, lmac, 6);
			memcpy(oam_OnuE1Info.oam_OnuE1Info[i].DesMac, rmac, 6);
		}
	}

	if ( V2R1_E1_OK != SetOnuE1All( VOS_AtoL(argv[0]), &oam_OnuE1Info ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		getOnuE1ALL,
		getOnuE1ALL_cmd,
		"getOnuE1ALL <onuDevIdx>",
		"get onu e1 all info\n"
		"onu device index"
		)
{
	OAM_OnuE1Info oam_OnuE1Info;

	/*if (2 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	oam_OnuE1Info.MsgType = SET_ONU_E1_ALL_RSP;

	if ( V2R1_E1_OK != GetOnuE1All( VOS_AtoL(argv[0]), &oam_OnuE1Info ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	/*TODO  ´òÓ¡Êý¾Ý*/

	return CMD_SUCCESS;
}
/* end: oamµ÷ÊÔÃüÁî */

#if SW_E1_PORT
#endif
/* begin: SWÉÏE1¶Ë¿Úµ÷ÊÔÃüÁî */
DEFUN  (
		sw_E1PortTable_get,
		sw_E1PortTable_get_cmd,
		"sw-E1PortTable-get <devIndex> <slotIndex> <e1portIndex>",
		"get E1PortTable\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		)
{
	ULONG idxs[3];
	e1PortTable_t e1PortTable;

	/*if (3 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponE1PortAlarmStatus: 0x%04x\r\n", e1PortTable.eponE1PortAlarmStatus);
	vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", e1PortTable.eponE1PortAlarmMask);
	vty_out( vty, "eponE1Loop: %d\r\n", e1PortTable.eponE1Loop);
	vty_out( vty, "eponE1TxClock: %d\r\n", e1PortTable.eponE1TxClock);

	return CMD_SUCCESS;
}

DEFUN  (
		sw_E1PortTable_getNext,
		sw_E1PortTable_getNext_cmd,
		"sw-E1PortTable-getnext <devIndex> <slotIndex> <e1portIndex>",
		"getNext e1PortTable\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		)
{
	ULONG idxs[3];
	e1PortTable_t e1PortTable;

	/*if (3 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != sw_e1PortTable_getNext(idxs, &e1PortTable) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	vty_out( vty, "eponE1PortAlarmStatus: 0x%04x\r\n", e1PortTable.eponE1PortAlarmStatus);
	vty_out( vty, "eponE1PortAlarmMask: 0x%04x\r\n", e1PortTable.eponE1PortAlarmMask);
	vty_out( vty, "eponE1Loop: %d\r\n", e1PortTable.eponE1Loop);
	vty_out( vty, "eponE1TxClock: %d\r\n", e1PortTable.eponE1TxClock);
	vty_out( vty, "\r\nOnuDevIdx=%ld    OnuSlotIdx=%ld    OnuE1PortIdx=%ld\r\n" , idxs[0], idxs[1], idxs[2]);

	return CMD_SUCCESS;
}

DEFUN  (
		sw_E1PortTable_set,
		sw_E1PortTable_set_cmd,
		"sw-E1PortTable-set <devIndex> <slotIndex> <e1portIndex> <leafIndex> <setValue>",
		"set E1PortTable leaf\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		"the leaf index of e1PortTable 2,3,4\n"
		"the leaf value of e1PortTable leaf\n"
		)
{
	ULONG idxs[3];

	/*if (5 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != sw_e1PortTable_set(VOS_AtoL(argv[3]), idxs, VOS_AtoL(argv[4])) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		sw_E1PortTable_rowset,
		sw_E1PortTable_rowset_cmd,
		"sw-E1PortTable-rowset <devIndex> <slotIndex> <e1portIndex> <eponE1PortAlarmMask> <eponE1Loop> <eponE1TxClock>",
		"rowset E1PortTable\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		"eponE1PortAlarmMask is hex\n"
		"eponE1Loop\n"
		"eponE1TxClock 0~3\n"
		)
{
	ULONG idxs[3];

	/*if (6 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != sw_e1PortTable_rowset( idxs, VOS_AtoL(argv[3]), VOS_AtoL(argv[4]), VOS_AtoL(argv[5]) ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

extern STATUS printSwE1PortTable(struct vty *vty, UCHAR showOnu);
DEFUN  (
		showSwE1PortTable,
		showSwE1PortTable_cmd,
		"showSwE1PortTable <param>",
		"show sw E1PortTable\n"
		"1: only show tdm e1 port    2: show online onu e1 port data\n"
		)
{
	/*if (1 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	if ( VOS_OK != printSwE1PortTable( vty, (UCHAR) VOS_AtoL(argv[0]) ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}
/* end: SWÉÏE1¶Ë¿Úµ÷ÊÔÃüÁî */

#if SET_SW_TDM_ONU_E1_TABLE
#endif
DEFUN  (
		E1PortTable_set,
		E1PortTable_set_cmd,
		"E1PortTable-set <devIndex> <slotIndex> <e1portIndex> <leafIndex> <setValue>",
		"set E1PortTable leaf\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		"the leaf index of e1PortTable 2,3,4\n"
		"the leaf value of e1PortTable leaf\n"
		)
{
	ULONG idxs[3];

	/*if (5 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != e1PortTable_set(VOS_AtoL(argv[3]), idxs, VOS_AtoL(argv[4])) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		E1PortTable_rowset,
		E1PortTable_rowset_cmd,
		"E1PortTable-rowset <devIndex> <slotIndex> <e1portIndex> <eponE1PortAlarmMask> <eponE1Loop> <eponE1TxClock>",
		"rowset E1PortTable\n"
		"the index of device 1 or a large number of ONU\n"
		"the index of slot \n"
		"the index of e1 port tdm:0~23 or onu:0-3\n"
		"eponE1PortAlarmMask is hex\n"
		"eponE1Loop\n"
		"eponE1TxClock 0~3\n"
		)
{
	ULONG idxs[3];

	/*if (6 != argc)
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = VOS_AtoL(argv[0]);

	if (1 == idxs[0])
	{
		idxs[1] = get_gfa_e1_slotno();
	}
	else
	{
		idxs[1] = VOS_AtoL(argv[1]);
	}

	idxs[2] = VOS_AtoL(argv[2]);

	if ( VOS_OK != e1PortTable_rowset( idxs, VOS_AtoL(argv[3]), VOS_AtoL(argv[4]), VOS_AtoL(argv[5]) ) )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

void e1_debug_CommandInstall(void)
{
	/* command for debug */
	install_element(E1_NODE,&tdm_E1LinkTable_get_cmd);
	install_element(E1_NODE,&tdm_E1LinkTable_getNext_cmd);
	install_element(E1_NODE,&tdm_E1LinkTable_set_cmd);
	install_element(E1_NODE,&tdm_E1LinkTable_rowset_cmd);

	install_element(E1_NODE,&tdm_E1PortTable_get_cmd);
	install_element(E1_NODE,&tdm_E1PortTable_getNext_cmd);
	install_element(E1_NODE,&tdm_E1PortTable_set_cmd);
	install_element(E1_NODE,&tdm_E1PortTable_rowset_cmd);

	install_element(E1_NODE,&tdm_E1VlanTable_get_cmd);
	install_element(E1_NODE,&tdm_E1VlanTable_getNext_cmd);
	install_element(E1_NODE,&tdm_E1VlanTable_set_cmd);
	install_element(E1_NODE,&tdm_E1VlanTable_rowset_cmd);

	install_element(E1_NODE,&onuE1Info_get_cmd);
	install_element(E1_NODE,&allE1Info_get_cmd);

	install_element(CONFIG_NODE,&allE1Info_get_cmd);

	install_element(E1_NODE,&setOnuE1Link_cmd);
	install_element(E1_NODE,&setOnuE1Vlan_cmd);
	install_element(E1_NODE,&setOnuE1AlarmMask_cmd);
	install_element(E1_NODE,&setOnuE1ALL_cmd);
	install_element(E1_NODE,&getOnuE1ALL_cmd);

	install_element(E1_NODE,&sw_E1PortTable_get_cmd);
	install_element(E1_NODE,&sw_E1PortTable_getNext_cmd);
	install_element(E1_NODE,&sw_E1PortTable_set_cmd);
	install_element(E1_NODE,&sw_E1PortTable_rowset_cmd);

	install_element(E1_NODE,&showSwE1PortTable_cmd);

	install_element(E1_NODE,&E1PortTable_set_cmd);
	install_element(E1_NODE,&E1PortTable_rowset_cmd);
}

#endif	/* EPON_MODULE_TDM_SERVICE */

