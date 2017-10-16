#include "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
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

Alarm_Onu_info_t * pEthCtcAlarmOnuHead = NULL;
Alarm_Pon_info_t * pEthCtcAlarmPonHead = NULL;
Alarm_Port_info_t *pEthCtcAlarmPortHead = NULL;

extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);

STATUS getFirstAlarmOnuEntry(unsigned short *GroupId )
{
	if(NULL==pEthCtcAlarmOnuHead)
		return VOS_ERROR;

	*GroupId = pEthCtcAlarmOnuHead->AlarmPolicyId;

	return VOS_OK;
}

STATUS getNextEthAlarmOnuEntry(unsigned short curGroupId,unsigned short * GroupId)
{
	Alarm_Onu_info_t *pCur=pEthCtcAlarmOnuHead;

	while(pCur != NULL)
	{
		if(pCur->AlarmPolicyId == curGroupId )
		{
			if(pCur->nextNode != NULL)
			{
				* GroupId = pCur->nextNode->AlarmPolicyId;
				return VOS_OK;
			}
			else
			{
				return VOS_ERROR;
			}
		}
		else if(pCur->AlarmPolicyId >= curGroupId)
		{
			* GroupId = pCur->AlarmPolicyId;
			return VOS_OK;
		}
		else
		{
			return VOS_ERROR;
		}
		pCur = pCur->nextNode;
	}

	return VOS_ERROR;
}


STATUS getFirstAlarmPonEntry(unsigned short *GroupId )
{
	if(NULL==pEthCtcAlarmPonHead)
		return VOS_ERROR;

	*GroupId = pEthCtcAlarmPonHead->alarmpolicyid;

	return VOS_OK;
}

STATUS getNextEthAlarmPonEntry(unsigned short curGroupId,unsigned short * GroupId)
{
	Alarm_Pon_info_t *pCur=pEthCtcAlarmPonHead;

	while(pCur != NULL)
	{
		if(pCur->alarmpolicyid == curGroupId )
		{
			if(pCur->nextNode != NULL)
			{
				* GroupId = pCur->nextNode->alarmpolicyid;
				return VOS_OK;
			}
			else
			{
				return VOS_ERROR;
			}
		}
		else if(pCur->alarmpolicyid >= curGroupId)
		{
			* GroupId = pCur->alarmpolicyid;
			return VOS_OK;
		}
		else
		{
			return VOS_ERROR;
		}
		pCur = pCur->nextNode;
	}

	return VOS_ERROR;
}

STATUS getFirstAlarmPortEntry(unsigned short *GroupId )
{
	if(NULL==pEthCtcAlarmPortHead)
		return VOS_ERROR;

	*GroupId = pEthCtcAlarmPortHead->EthAlarmPolicyId;

	return VOS_OK;
}

STATUS getNextEthAlarmPortEntry(unsigned short curGroupId,unsigned short * GroupId)
{
	Alarm_Port_info_t *pCur=pEthCtcAlarmPortHead;

	while(pCur != NULL)
	{
		if(pCur->EthAlarmPolicyId == curGroupId )
		{
			if(pCur->nextNode != NULL)
			{
				* GroupId = pCur->nextNode->EthAlarmPolicyId;
				return VOS_OK;
			}
			else
			{
				return VOS_ERROR;
			}
		}
		else if(pCur->EthAlarmPolicyId >= curGroupId)
		{
			* GroupId = pCur->EthAlarmPolicyId;
			return VOS_OK;
		}
		else
		{
			return VOS_ERROR;
		}
		pCur = pCur->nextNode;
	}

	return VOS_ERROR;
}

Alarm_Onu_info_t * CTC_OnuAlarm_Find_Node( const ULONG GroupId )
{
	Alarm_Onu_info_t *pCur=pEthCtcAlarmOnuHead;

	while(pCur != NULL)
	{
		if(pCur->AlarmPolicyId == GroupId )
			return pCur;
		pCur = pCur->nextNode;
	}

	return NULL;
}

STATUS CTC_AlarmOnu_Add_Node( const ULONG GroupId )
{
	Alarm_Onu_info_t *pCur=pEthCtcAlarmOnuHead,*pNewNode = NULL, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->AlarmPolicyId == GroupId )
		{
			sys_console_printf("The Group Id has existed !\r\n");
			return VOS_ERROR;
		}
		else if(GroupId <= pCur->AlarmPolicyId)
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	pNewNode = VOS_Malloc( sizeof(Alarm_Onu_info_t), MODULE_RPU_ONU );	
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemSet( pNewNode, 0, sizeof(Alarm_Onu_info_t) );
	pNewNode->nextNode = NULL;
	pNewNode->AlarmPolicyId = GroupId;
	
	if(pre ==NULL)
	{
		pNewNode->nextNode = pEthCtcAlarmOnuHead;
		pEthCtcAlarmOnuHead = pNewNode;
	}
	else
	{
		pNewNode->nextNode = pre->nextNode;
		pre->nextNode = pNewNode;
	}
	return VOS_OK;
}

STATUS CTC_AlarmOnu_Del_Node( const ULONG GroupId)
{
	Alarm_Onu_info_t *pCur=pEthCtcAlarmOnuHead, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->AlarmPolicyId == GroupId )
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	if(pCur == NULL)
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}
	else
	{
		if(pre == NULL)
		{
			pEthCtcAlarmOnuHead = pCur->nextNode;
		}
		else
		{
			pre->nextNode = pCur->nextNode;
		}
		VOS_Free(pCur);
		return VOS_OK;
	}
	
	return VOS_OK;
}


DEFUN(ctc_alarm_onu_policy_add,
	ctc_alarm_onu_policy_add_cmd,
      "ctc alarm-onu policy [add|delete] <1-100>",
	CTC_STR
       "onu alarm policy\n"
       "onu alarm policy\n"
       "add\n"
       "delete\n"
	"input policy number\n")
{
	ULONG GroupId;
	GroupId=VOS_AtoL( argv[1] );
	if(VOS_StriCmp(argv[0], "add")==0)
	{
		if(CTC_AlarmOnu_Add_Node( GroupId)==VOS_ERROR)
			return CMD_WARNING;
	}
	else if(VOS_StriCmp(argv[0], "delete")==0)
	{
		if(CTC_AlarmOnu_Del_Node( GroupId )==VOS_ERROR)
			return CMD_WARNING;
	}
	return  CMD_SUCCESS;
}

static int onu_alarm_enable_cmd_process( struct vty *vty, int argc, char **argv, ULONG flagenable )
{
	ULONG GroupId;
	Alarm_Onu_info_t  *pNode;
	int tempnum=1;

	GroupId=VOS_AtoL( argv[0] );
	pNode =(Alarm_Onu_info_t  *)CTC_OnuAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		return CMD_WARNING;
	}

	if( VOS_StriCmp(argv[tempnum], "equipment") == 0 )
	{
		pNode->EquipmentAlarm = flagenable;
	}
	else if( VOS_StriCmp(argv[tempnum], "power") == 0 )
	{
		pNode->PowerAlarm= flagenable;
	}
	else if( VOS_StriCmp(argv[tempnum], "intrusion") == 0 )
		pNode->PhysicalIntrusionAlarm= flagenable;
	else if( VOS_StriCmp(argv[tempnum], "selftest") == 0 )
		pNode->OnuSelfTestFailure= flagenable;
	else if( VOS_StriCmp(argv[tempnum], "iadconnect") == 0 )
		pNode->IadConnectFailure= flagenable;
	else if( VOS_StriCmp(argv[tempnum], "ponswitch") == 0 )
		pNode->PonSwitch= flagenable;
	else if( VOS_StriCmp(argv[1], "battery") == 0 )
	{
		pNode->BatteryMissing= flagenable;
		pNode->BatteryFailure= flagenable;
		pNode->BatteryVoltLow= flagenable;

		if( argc > 3 )
		{
			pNode->BatteryVoltLowAlarmThr = VOS_AtoL( argv[2] );
			pNode->BatteryVoltLowClearThr = VOS_AtoL( argv[3] );
		}
	}
	else if( VOS_StriCmp(argv[1], "temphigh") == 0 )
	{
		pNode->OnuTempHighAlarm= flagenable;

		if( argc > 3 )
		{
			pNode->OnuTempHighAlarmThr = VOS_AtoL( argv[2] );
			pNode->OnuTempHighClearThr = VOS_AtoL( argv[3] );
		}
	}
	else if( VOS_StriCmp(argv[1], "templow") == 0 )
	{
		pNode->OnuTempLowAlarm= flagenable;

		if( argc > 3 )
		{
			pNode->OnuTempLowAlarmThr = VOS_AtoL( argv[2] );
			pNode->OnuTempLowClearThr = VOS_AtoL( argv[3] );
		}
	}
	
	return  CMD_SUCCESS;
}

DEFUN(alarm_onu_policy_able_add,
	alarm_onu_policy_able_add_cmd,
	"ctc alarm-onu enable <1-100> [equipment|power|intrusion|selftest|iadconnect|ponswitch]",
	CTC_STR
	"onu alarm enable\n"
	"onu alarm enable\n"
	"input policy number\n"
	"equipment alarm\n"
	"power alarm\n"
	"physical intrusion alarm\n"
	"onu self test failure alarm\n"
	"onu temperature high or low alarm\n"
	"pon switch\n"
	)
{
	return  onu_alarm_enable_cmd_process( vty, argc, argv, 1 );
}

DEFUN(undo_alarm_onu_policy_able_add,
	undo_alarm_onu_policy_able_add_cmd,
	"undo ctc alarm-onu enable <1-100> [equipment|power|intrusion|selftest|iadconnect|ponswitch]",
	"undo\n"
	CTC_STR
	"onu alarm enable\n"
	"onu alarm enable\n"
	"input policy number\n"
	"equipment alarm\n"
	"power alarm\n"
	"physical intrusion alarm\n"
	"onu self test failure alarm\n"
	"onu temperature high or low alarm\n"
	"pon switch\n"
	)
{
	return  onu_alarm_enable_cmd_process( vty, argc, argv, 0 );
}

DEFUN(alarm_onu_policy_thr_add,
	alarm_onu_policy_thr_add_cmd,
       "ctc alarm-onu enable <1-100> [battery|temphigh|templow] <alarm> <clear>",
 	CTC_STR
       "onu alarm enable\n"
       "onu alarm enable\n"
	"input policy number\n"
	"battery missing,failure or voltage exceed threshold alarm\n"
	"temperature high alarm\n"
	"temperature low alarm\n"
	"input alarm threshold\n"
	"input alarm clear threshold\n"
	)
{
	return  onu_alarm_enable_cmd_process( vty, argc, argv, 1 );
}

DEFUN(undo_alarm_onu_policy_thr_add,
	undo_alarm_onu_policy_thr_add_cmd,
       "undo ctc alarm-onu enable <1-100> [battery|temphigh|templow]",
	"undo\n"
	CTC_STR
       "onu alarm enable\n"
       "onu alarm enable\n"
	"input policy number\n"
	"battery missing,failure or voltage exceed threshold alarm\n"
	"temperature high alarm\n"
	"temperature low alarm\n"
	)
{
	return  onu_alarm_enable_cmd_process( vty, argc, argv, 0 );
}

static char *enable_to_str( int ebl )
{
	char *eblStr;
	if( ebl == 1 )
		eblStr = "enable";
	else
		eblStr = "disable";
	return eblStr;
}
VOID showAlarmOnuEnbleGroup( struct vty *vty)
{
	Alarm_Onu_info_t *pNode = pEthCtcAlarmOnuHead;

	while(pNode != NULL)
	{
		vty_out(vty,"\r\n AlarmPolicyId  is %d \r\n",pNode->AlarmPolicyId);
		vty_out(vty," %-20s %-8s %-8s %s\r\n", "Type","Enable", "Thr", "Clearthr\r\n");
		
		vty_out(vty," %-20s %-8s %-8s %s\r\n","Equipment", enable_to_str(pNode->EquipmentAlarm), "-", "-" );
		vty_out(vty," %-20s %-8s %-8s %s\r\n","Power", enable_to_str(pNode->PowerAlarm), "-", "-" );
		vty_out(vty," %-20s %-8s %-8s %s\r\n","BatteryMissing", enable_to_str(pNode->BatteryMissing), "-", "-" );
		vty_out(vty," %-20s %-8s %-8s %s\r\n","BatteryFailure", enable_to_str(pNode->BatteryFailure), "-", "-" );
		vty_out(vty," %-20s %-8s %-8d %d\r\n","BatteryVoltLow", enable_to_str(pNode->BatteryVoltLow), pNode->BatteryVoltLowAlarmThr, pNode->BatteryVoltLowClearThr);
		vty_out(vty," %-20s %-8s %-8s %s\r\n","PhyIntrusion", enable_to_str(pNode->PhysicalIntrusionAlarm), "-", "-" );
		vty_out(vty," %-20s %-8s %-8s %s\r\n","SelfTest", enable_to_str(pNode->OnuSelfTestFailure), "-", "-" );
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TemperatureHigh", enable_to_str(pNode->OnuTempHighAlarm), pNode->OnuTempHighAlarmThr, pNode->OnuTempHighClearThr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TemperatureLow", enable_to_str(pNode->OnuTempLowAlarm), pNode->OnuTempLowAlarmThr, pNode->OnuTempLowClearThr);
		vty_out(vty," %-20s %-8s %-8s %s\r\n","IadConnect", enable_to_str(pNode->IadConnectFailure), "-", "-" );
		vty_out(vty," %-20s %-8s %-8s %s\r\n","PonSwitch", enable_to_str(pNode->PonSwitch), "-", "-" );

		pNode = pNode->nextNode;
	}
	return ;
}



	
DEFUN(alarm_onu_show,
	alarm_onu_show_cmd,
       "show ctc alarm-onu",
	SHOW_STR
	CTC_STR
	"onu alarm\n")
{
	showAlarmOnuEnbleGroup(vty) ;
	return  CMD_SUCCESS;
}


DEFUN(alarm_onu_map,
	alarm_onu_map_cmd,
       "ctc alarm-map onu <slot/port/onuid> <1-100>",
	CTC_STR
	"alarm-onu policy map\n"
	"alarm-onu policy map\n"
	"input onu id\n"
	"input alarm-onu policy number\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	ULONG GroupId;
	Alarm_Onu_info_t  *pNode;
	LogicEntity *pEntry = NULL;

	GroupId=VOS_AtoL( argv[1] );
	pNode =(Alarm_Onu_info_t  *)CTC_OnuAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		return CMD_WARNING;
	}
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_ONU] = GroupId;
	}

	return  CMD_SUCCESS;
}

DEFUN(undo_alarm_onu_map,
	undo_alarm_onu_map_cmd,
       "undo ctc alarm-map onu <slot/port/onuid>",
       "undo\n"
	CTC_STR
	"alarm-onu policy map\n"
	"alarm-onu policy map\n"
	"input onu id\n\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;

 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_ONU] = 0;
	}

	return  CMD_SUCCESS;
}

DEFUN(alarm_map_show,
	alarm_map_show_cmd,
       "show ctc alarm-map <slot/port/onuid>",
       SHOW_STR
	CTC_STR
	"onu alarm policy map\n"
	"input onu id\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{	
		if(0 != pEntry->ctcAlarmSelect[CTC_ALARM_ONU])
		{
			vty_out(vty,"onu policy:%d\r\n",pEntry->ctcAlarmSelect[CTC_ALARM_ONU]);
		}
		if(0 != pEntry->ctcAlarmSelect[CTC_ALARM_PON])
		{
			vty_out(vty,"pon policy:%d\r\n",pEntry->ctcAlarmSelect[CTC_ALARM_PON]);
		}
		if(0 != pEntry->ctcAlarmSelect[CTC_ALARM_PORT])
		{
			vty_out(vty,"port policy:%d\r\n",pEntry->ctcAlarmSelect[CTC_ALARM_PORT]);
		}
	}
	return  CMD_SUCCESS;
}


Alarm_Port_info_t * CTC_PortAlarm_Find_Node( const ULONG GroupId );
STATUS Ctc_Alarm_Action_Port(ULONG dexIdx,INT GroupId)
{
  	short int testflag = 0;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	ULONG ethIdx;   
	ULONG portNum;
	Alarm_Port_info_t  *pNode;
	CTC_management_object_index_t management_object;
	VOS_MemZero(&management_object, sizeof(management_object));

	if( (getDeviceCapEthPortNum( dexIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		sys_console_printf("no ethernet port\r\n");
		return VOS_ERROR;
	}
	
	pNode =(Alarm_Port_info_t  *)CTC_PortAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}

	management_object.frame_number = 0;
	management_object.slot_number = 0;
	management_object.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if(parse_onuidx_command_parameter(dexIdx, &olt_id, &onu_id)== VOS_OK && 
		GetOnuOperStatus(olt_id, onu_id) == 1  )
	{
		llid = GetLlidByOnuIdx( olt_id, onu_id );

		if(VOS_StriCmp(pNode->pcPort_List, "all")==0)
		{
			for(ethIdx=1;ethIdx<=portNum;ethIdx++)
			{
				management_object.port_number = ethIdx;
				
				if(pNode->EthAutoNegFailure == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_AUTO_NEG_FAILURE,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_AUTO_NEG_FAILURE,0 );

				if(pNode->EthLos == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOS,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOS,0 );

				if(pNode->EthFailure == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_FAILURE,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_FAILURE,0 );

				if(pNode->EthLoopback == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOOPBACK,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOOPBACK,0 );

				if(pNode->EthCongestion == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_CONGESTION,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_CONGESTION,0 );
			}
		}
		else
		{
			BEGIN_PARSE_PORT_LIST_TO_PORT( pNode->pcPort_List, ethIdx )
			if(ethIdx>portNum)
			{
				sys_console_printf("ethernet port is out of range %d\r\n", portNum);
				VOS_Free(_pulIfArray);
				return VOS_ERROR;
			}
			END_PARSE_PORT_LIST_TO_PORT();

			BEGIN_PARSE_PORT_LIST_TO_PORT( pNode->pcPort_List, ethIdx )
			{
				management_object.port_number = ethIdx;
				
				if(pNode->EthAutoNegFailure == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_AUTO_NEG_FAILURE,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_AUTO_NEG_FAILURE,0 );

				if(pNode->EthLos == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOS,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOS,0 );

				if(pNode->EthFailure == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_FAILURE,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_FAILURE,0 );

				if(pNode->EthLoopback == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOOPBACK,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_LOOPBACK,0 );

				if(pNode->EthCongestion == 1)
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_CONGESTION,1 );
				else
					testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ETH_PORT_CONGESTION,0 );
			}
			END_PARSE_PORT_LIST_TO_PORT();	
		}
	}
	return VOS_OK;
}
STATUS Ctc_Alarm_Action_Onu(ULONG dexIdx,INT GroupId)
{
  	short int testflag = 0;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	unsigned long	 alarm_threshold,clear_threshold;
	PON_llid_t llid;
	Alarm_Onu_info_t  *pNode;
	CTC_management_object_index_t management_object;
	
	VOS_MemZero(&management_object, sizeof(management_object));

	pNode =(Alarm_Onu_info_t  *)CTC_OnuAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}

	management_object.frame_number = 0;
	management_object.port_number = 0xFFFFFFFF;
	management_object.slot_number = 0;
	management_object.port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if(parse_onuidx_command_parameter(dexIdx, &olt_id, &onu_id)== VOS_OK && 
		GetOnuOperStatus(olt_id, onu_id) == 1  )
	{
		llid = GetLlidByOnuIdx( olt_id, onu_id );
		sys_console_printf("In Ctc_Alarm_Action_Onu : olt_id is %d,llid is %d\r\n",olt_id, llid);

		/*sys_console_printf("EquipmentAlarm is %d\r\n",pNode->EquipmentAlarm);*/
		if(pNode->EquipmentAlarm == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,EQUIPMENT_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,EQUIPMENT_ALARM,0 );
		 /*sys_console_printf("PowerAlarm is %d\r\n",pNode->PowerAlarm);*/
		if(pNode->PowerAlarm  == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,POWERING_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,POWERING_ALARM,0 );

		/*alarm_threshold = pNode->
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,POWERING_ALARM, alarm_threshold,clear_threshold);*/
		/*sys_console_printf("BatteryMissing is %d\r\n",pNode->BatteryMissing);*/
		if(pNode->BatteryMissing == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_MISSING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_MISSING,0 );
		/*sys_console_printf("BatteryFailure is %d\r\n",pNode->BatteryFailure);*/
		if(pNode->BatteryFailure == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_FAILURE,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_FAILURE,0 );
		/*sys_console_printf("BatteryVoltLow is %d\r\n",pNode->BatteryVoltLow);*/
		if(pNode->BatteryVoltLow == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_VOLT_LOW,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_VOLT_LOW,0 );

		alarm_threshold = pNode->BatteryVoltLowAlarmThr;
		clear_threshold = pNode->BatteryVoltLowClearThr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,BATTERY_VOLT_LOW, alarm_threshold,clear_threshold);

		if(pNode->PhysicalIntrusionAlarm == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,PHYSICAL_INTRUSION_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,PHYSICAL_INTRUSION_ALARM,0 );

		if(pNode->OnuSelfTestFailure == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_SELF_TEST_FAILURE,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_SELF_TEST_FAILURE,0 );

		if(pNode->OnuTempHighAlarm == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_HIGH_ALARM,0 );

		alarm_threshold = pNode->OnuTempHighAlarmThr;
		clear_threshold = pNode->OnuTempHighClearThr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,ONU_TEMP_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->OnuTempLowAlarm == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_LOW_ALARM,0 );

		alarm_threshold = pNode->OnuTempLowAlarmThr;
		clear_threshold = pNode->OnuTempLowClearThr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,ONU_TEMP_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->IadConnectFailure == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,IAD_CONNECTION_FAILURE,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,IAD_CONNECTION_FAILURE,0 );

		if(pNode->PonSwitch == 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,PON_IF_SWITCH,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,PON_IF_SWITCH,0 );
	}


	return VOS_OK;
}
/*
STATUS test_onu_status(ULONG dexIdx,SHORT flag)
{
  	short int testflag = 0;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_management_object_index_t management_object;
	
	VOS_MemZero(&management_object, sizeof(management_object));

	management_object.frame_number = 0;
	management_object.port_number = 0xFFFFFFFF;
	management_object.slot_number = 0;
	management_object.port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if(parse_onuidx_command_parameter(dexIdx, &olt_id, &onu_id)== VOS_OK && 
		GetOnuOperStatus(olt_id, onu_id) == 1  )
	{
		llid = GetLlidByOnuIdx( olt_id, onu_id );
		sys_console_printf("In Ctc_Alarm_Action_Onu : olt_id is %d,llid is %d\r\n",olt_id, llid);
		if(flag == 1)
		{
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,EQUIPMENT_ALARM,1 );
			sys_console_printf("testflag is %d\r\n",testflag);
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_MISSING,1 );
			sys_console_printf("testflag is %d\r\n",testflag);
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_VOLT_LOW,1 );
			sys_console_printf("testflag is %d\r\n",testflag);
		}
		else
		{
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,EQUIPMENT_ALARM,0 );
			sys_console_printf("testflag is %d\r\n",testflag);
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_MISSING,0 );
			sys_console_printf("testflag is %d\r\n",testflag);
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,BATTERY_VOLT_LOW,0 );
			sys_console_printf("testflag is %d\r\n",testflag);
		}
	
	}
	return VOS_OK;
}

DEFUN(onu_status_test,
	onu_status_test_cmd,
       "test onustatus <slotId/port/onuId> <1-2>",
	SHOW_STR
	"onustatus\n"
	"slotId/port/onuId\n")
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	ULONG flag=0;
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );
	dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;
	flag=VOS_AtoL( argv[1] );
	test_onu_status(dexIdx,flag);
	return  CMD_SUCCESS;
}
*/
/*
STATUS Get_Onu_Status(ULONG dexIdx)
{
  	short int testflag = 0;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	PON_llid_t llid;
	CTC_management_object_index_t management_object;
	
	unsigned short number = 10;
	CTC_STACK_alarm_admin_state_t alarms_state[10];
	VOS_MemZero(&management_object, sizeof(management_object));

	management_object.frame_number = 0;
	management_object.port_number = 0xFFFFFFFF;
	management_object.slot_number = 0;
	management_object.port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if(parse_onuidx_command_parameter(dexIdx, &olt_id, &onu_id)== VOS_OK && 
		GetOnuOperStatus(olt_id, onu_id) == 1  )
	{
		llid = GetLlidByOnuIdx( olt_id, onu_id );
		sys_console_printf("In Ctc_Alarm_Action_Onu : olt_id is %d,llid is %d\r\n",olt_id, llid);

		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,EQUIPMENT_ALARM, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("EQUIPMENT_ALARM :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("EQUIPMENT_ALARM : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("EQUIPMENT_ALARM : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,BATTERY_MISSING, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("BATTERY_MISSING :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("BATTERY_MISSING : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("BATTERY_MISSING : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,BATTERY_FAILURE, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("BATTERY_FAILURE :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("BATTERY_FAILURE : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("BATTERY_FAILURE : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,BATTERY_VOLT_LOW, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("BATTERY_VOLT_LOW :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("BATTERY_VOLT_LOW : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("BATTERY_VOLT_LOW : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,POWERING_ALARM, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("POWERING_ALARM :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("POWERING_ALARM : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("POWERING_ALARM : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,PHYSICAL_INTRUSION_ALARM, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("PHYSICAL_INTRUSION_ALARM :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("PHYSICAL_INTRUSION_ALARM : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("PHYSICAL_INTRUSION_ALARM : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,ONU_SELF_TEST_FAILURE, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("ONU_SELF_TEST_FAILURE :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("ONU_SELF_TEST_FAILURE : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("ONU_SELF_TEST_FAILURE : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_HIGH_ALARM, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("ONU_TEMP_HIGH_ALARM :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("ONU_TEMP_HIGH_ALARM : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("ONU_TEMP_HIGH_ALARM : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,ONU_TEMP_LOW_ALARM, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("ONU_TEMP_LOW_ALARM :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("ONU_TEMP_LOW_ALARM : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("ONU_TEMP_LOW_ALARM : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,IAD_CONNECTION_FAILURE, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("IAD_CONNECTION_FAILURE :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("IAD_CONNECTION_FAILURE : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("IAD_CONNECTION_FAILURE : Alarm onu get fail!\r\n" );
		}
		number = 10;
		testflag = CTC_STACK_get_alarm_admin_state(olt_id, llid,management_object,PON_IF_SWITCH, &number,alarms_state);
		if(testflag == CTC_STACK_EXIT_OK ) 
		{	
			if(1 == alarms_state[0].enable)
			{
				sys_console_printf("PON_IF_SWITCH :Alarm onu is enabled !\r\n");
			}
			else
			{
				sys_console_printf("PON_IF_SWITCH : Alarm onu is disabled !\r\n");
			}
		}
		else
		{
			sys_console_printf("PON_IF_SWITCH : Alarm onu get fail!\r\n" );
		}

	}
	return VOS_OK;
}
DEFUN(onu_status_show,
	onu_status_show_cmd,
       "show onustatus <slotId/port/onuId>",
	SHOW_STR
	"onustatus\n"
	"slotId/port/onuId\n")
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );
	dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;
	Get_Onu_Status(dexIdx);
	return  CMD_SUCCESS;
}
*/
STATUS Ctc_Alarm_Action_Pon(ULONG dexIdx,INT GroupId)
{
  	short int testflag = 0;
	PON_olt_id_t olt_id;
	PON_onu_id_t onu_id;
	unsigned long	 alarm_threshold,clear_threshold;
	PON_llid_t llid;
	Alarm_Pon_info_t  *pNode;
	CTC_management_object_index_t management_object;
	VOS_MemZero(&management_object, sizeof(management_object));


	pNode =(Alarm_Pon_info_t  *)CTC_PonAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}

	management_object.frame_number = 0;
	management_object.port_number = 0;
	management_object.slot_number = 0;
	management_object.port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	if(parse_onuidx_command_parameter(dexIdx, &olt_id, &onu_id)== VOS_OK && 
		GetOnuOperStatus(olt_id, onu_id) == 1  )
	{
		llid = GetLlidByOnuIdx( olt_id, onu_id );
		if(pNode->rxpowerhighalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_HIGH_ALARM,0 );
		
		alarm_threshold = pNode->rxpowerhighalarmthr;
		clear_threshold = pNode->rxpowerhighalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,RX_POWER_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->rxpowerlowalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_LOW_ALARM,0 );
		
		alarm_threshold = pNode->rxpowerlowalarmthr;
		clear_threshold = pNode->rxpowerlowalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,RX_POWER_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->txpowerhighalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_HIGH_ALARM,0 );
		
		alarm_threshold = pNode->txpowerhighalarmthr;
		clear_threshold = pNode->txpowerhighalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_POWER_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->txpowerlowalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_LOW_ALARM,0 );
		
		alarm_threshold = pNode->txpowerlowalarmthr;
		clear_threshold = pNode->txpowerlowalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_POWER_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->txbiashighalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_HIGH_ALARM,0 );
		
		alarm_threshold = pNode->txbiashighalarmthr;
		clear_threshold = pNode->txbiashighalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_BIAS_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->txbiaslowalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_LOW_ALARM,0 );
		
		alarm_threshold = pNode->txbiaslowalarmthr;
		clear_threshold = pNode->txbiaslowalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_BIAS_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->vcchighalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_HIGH_ALARM,0 );
		
		alarm_threshold = pNode->vcchighalarmthr;
		clear_threshold = pNode->vcchighalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,VCC_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->vcclowalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_LOW_ALARM,0 );
		
		alarm_threshold = pNode->vcclowalarmthr;
		clear_threshold = pNode->vcclowalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,VCC_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->temphighalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_HIGH_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_HIGH_ALARM,0 );
		
		alarm_threshold = pNode->temphighalarmthr;
		clear_threshold = pNode->temphighalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TEMP_HIGH_ALARM, alarm_threshold,clear_threshold);

		if(pNode->templowalarm== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_LOW_ALARM,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_LOW_ALARM,0 );
		
		alarm_threshold = pNode->templowalarmthr;
		clear_threshold = pNode->templowalarmclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TEMP_LOW_ALARM, alarm_threshold,clear_threshold);

		if(pNode->rxpowerhighwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_HIGH_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_HIGH_WARNING,0 );
		
		alarm_threshold = pNode->rxpowerhighwarningthr;
		clear_threshold = pNode->rxpowerhighwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,RX_POWER_HIGH_WARNING, alarm_threshold,clear_threshold);

		if(pNode->rxpowerlowwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_LOW_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,RX_POWER_LOW_WARNING,0 );
		
		alarm_threshold = pNode->rxpowerlowwarningthr;
		clear_threshold = pNode->rxpowerlowwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,RX_POWER_LOW_WARNING, alarm_threshold,clear_threshold);

		if(pNode->txpowerhighwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_HIGH_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_HIGH_WARNING,0 );
		
		alarm_threshold = pNode->txpowerhighwarningthr;
		clear_threshold = pNode->txpowerhighwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_POWER_HIGH_WARNING, alarm_threshold,clear_threshold);

		if(pNode->txpowerlowwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_LOW_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_POWER_LOW_WARNING,0 );
		
		alarm_threshold = pNode->txpowerlowwarningthr;
		clear_threshold = pNode->txpowerlowwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_POWER_LOW_WARNING, alarm_threshold,clear_threshold);

		if(pNode->txbiashighwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_HIGH_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_HIGH_WARNING,0 );
		
		alarm_threshold = pNode->txbiashighwarningthr;
		clear_threshold = pNode->txbiashighwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_BIAS_HIGH_WARNING, alarm_threshold,clear_threshold);

		if(pNode->txbiaslowwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_LOW_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TX_BIAS_LOW_WARNING,0 );
		
		alarm_threshold = pNode->txbiaslowwarning;
		clear_threshold = pNode->txbiaslowwarning;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TX_BIAS_LOW_WARNING, alarm_threshold,clear_threshold);

		if(pNode->vcchighwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_HIGH_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_HIGH_WARNING,0 );
		
		alarm_threshold = pNode->vcchighwarningthr;
		clear_threshold = pNode->vcchighwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,VCC_HIGH_WARNING, alarm_threshold,clear_threshold);

		if(pNode->vcclowwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_LOW_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,VCC_LOW_WARNING,0 );
		
		alarm_threshold = pNode->vcclowwarningthr;
		clear_threshold = pNode->vcclowwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,VCC_LOW_WARNING, alarm_threshold,clear_threshold);

		if(pNode->temphighwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_HIGH_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_HIGH_WARNING,0 );
		
		alarm_threshold = pNode->temphighwarningthr;
		clear_threshold = pNode->temphighwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TEMP_HIGH_WARNING, alarm_threshold,clear_threshold);

		if(pNode->templowwarning== 1)
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_LOW_WARNING,1 );
		else
			testflag = CTC_STACK_set_alarm_admin_state(olt_id, llid,management_object,TEMP_LOW_WARNING,0 );
		
		alarm_threshold = pNode->templowwarningthr;
		clear_threshold = pNode->templowwarningclearthr;
		testflag = CTC_STACK_set_alarm_threshold(olt_id, llid,management_object,TEMP_LOW_WARNING, alarm_threshold,clear_threshold);
				
	}

	return VOS_OK;
}
DEFUN(ctc_alarm_action,
	ctc_alarm_action_cmd,
       "ctc alarm-action <slot/port/onuid>",
	CTC_STR
	"Send the configulation to ONUs\n"
	"input onu id\n")
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		if(pEntry->ctcAlarmSelect[CTC_ALARM_ONU] != 0)
		{
			if(Ctc_Alarm_Action_Onu(dexIdx,pEntry->ctcAlarmSelect[CTC_ALARM_ONU])==VOS_ERROR)
				return CMD_WARNING;
		}
		if(pEntry->ctcAlarmSelect[CTC_ALARM_PON] != 0)
		{
			if(Ctc_Alarm_Action_Pon(dexIdx,pEntry->ctcAlarmSelect[CTC_ALARM_PON])==VOS_ERROR)
				return CMD_WARNING;
		}

		if(pEntry->ctcAlarmSelect[CTC_ALARM_PORT] != 0)
		{
			if(Ctc_Alarm_Action_Port(dexIdx,pEntry->ctcAlarmSelect[CTC_ALARM_PORT])==VOS_ERROR)
				return CMD_WARNING;
		}
		/*
		if(pEntry->ctcAlarmSelect[CTC_ALARM_CARD] != 0)
		{

		}*/
	}
	return  CMD_SUCCESS;
}

DEFUN(alarmPon_policy_add,
	alarmPon_policy_add_cmd,
      "ctc alarm-pon policy [add|delete] <1-100> ",
	CTC_STR
	"onu alarm policy\n"
	"onu pon alarm policy\n"
       "add policy\n"
       "delete policy\n"
	"input policy number\n")
{
	ULONG GroupId;
	GroupId=VOS_AtoL( argv[1] );
	if(VOS_StriCmp(argv[0], "add")==0)
	{
		if(CTC_AlarmPon_Add_Node( GroupId ,vty)==VOS_ERROR)
			return CMD_WARNING;
	}
	else if(VOS_StriCmp(argv[0], "delete") == 0)
	{
		if(CTC_AlarmPon_Del_Node( GroupId, vty )==VOS_ERROR)
			return CMD_WARNING;
	}
	return  CMD_SUCCESS;
}

STATUS CTC_AlarmPon_Add_Node( const ULONG GroupId )
{
	Alarm_Pon_info_t *pCur=pEthCtcAlarmPonHead,*pNewNode = NULL, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->alarmpolicyid == GroupId )
		{
			sys_console_printf("The Group Id has existed !\r\n");
			return VOS_ERROR;
		}
		else if(GroupId <= pCur->alarmpolicyid)
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	pNewNode = VOS_Malloc( sizeof(Alarm_Pon_info_t), MODULE_RPU_ONU );	
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemSet( pNewNode, 0, sizeof(Alarm_Pon_info_t) );
	pNewNode->nextNode = NULL;
	pNewNode->alarmpolicyid = GroupId;
	
	if(pre ==NULL)
	{
		pNewNode->nextNode = pEthCtcAlarmPonHead;
		pEthCtcAlarmPonHead = pNewNode;
	}
	else
	{
		pNewNode->nextNode = pre->nextNode;
		pre->nextNode = pNewNode;
	}
	return VOS_OK;
}

STATUS CTC_AlarmPon_Del_Node( const ULONG GroupId )
{
	Alarm_Pon_info_t *pCur=pEthCtcAlarmPonHead, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->alarmpolicyid == GroupId )
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	if(pCur == NULL)
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}
	else
	{
		if( NULL == pre)
		{
			pEthCtcAlarmPonHead = pCur->nextNode;
		}
		else
		{
			pre->nextNode = pCur->nextNode;
		}
		VOS_Free(pCur);
		return VOS_OK;
	}
	
	return VOS_OK;
}

Alarm_Pon_info_t * CTC_PonAlarm_Find_Node( const ULONG GroupId )
{
	Alarm_Pon_info_t *pCur=pEthCtcAlarmPonHead;

	while(pCur != NULL)
	{
		if(pCur->alarmpolicyid == GroupId )
			return pCur;
		pCur = pCur->nextNode;
	}

	return NULL;
}

pon_alarm_enable_cmd_process( struct vty *vty, int argc, char **argv, ULONG flagenable )
{
	ULONG GroupId;
	Alarm_Pon_info_t  *pNode;
	LONG alarmthr = 0, clearthr = 0;
	ULONG alarmlevel = 0;
	ULONG thresholdAssign = 0;

	GroupId=VOS_AtoL( argv[0] );
	pNode =(Alarm_Pon_info_t  *)CTC_PonAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		VOS_ASSERT(0);
		return CMD_WARNING;
	}

	if(VOS_StriCmp(argv[1], "alarm")==0)
		alarmlevel = 1;
	
	if( (argc > 4) && flagenable )
	{
		thresholdAssign = 1;
		alarmthr = VOS_AtoL(argv[3]);
		clearthr = VOS_AtoL(argv[4]);
	}

	if( VOS_StriCmp(argv[2], "rxpowerhigh") == 0 )
	{
		if( alarmlevel )
		{
			pNode->rxpowerhighalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->rxpowerhighalarmthr = alarmthr;
				pNode->rxpowerhighalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->rxpowerhighwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->rxpowerhighwarningthr = alarmthr;
				pNode->rxpowerhighwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "rxpowerlow") == 0 )
	{
		if( alarmlevel )
		{
			pNode->rxpowerlowalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->rxpowerlowalarmthr = alarmthr;
				pNode->rxpowerlowalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->rxpowerlowwarning= flagenable;
			if( thresholdAssign )
			{
				pNode->rxpowerlowwarningthr= alarmthr;
				pNode->rxpowerlowwarningclearthr= clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "txpowerhigh") == 0 )
	{
		if( alarmlevel )
		{
			pNode->txpowerhighalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->txpowerhighalarmthr = alarmthr;
				pNode->txpowerhighalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->txpowerhighwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->txpowerhighwarningthr = alarmthr;
				pNode->txpowerhighwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "txpowerlow") == 0 )
	{
		if( alarmlevel )
		{
			pNode->txpowerlowalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->txpowerlowalarmthr = alarmthr;
				pNode->txpowerlowalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->txpowerlowwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->txpowerlowwarningthr = alarmthr;
				pNode->txpowerlowwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "txbiashigh") == 0 )
	{
		if( alarmlevel )
		{
			pNode->txbiashighalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->txbiashighalarmthr = alarmthr;
				pNode->txbiashighalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->txbiashighwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->txbiashighwarningthr = alarmthr;
				pNode->txbiashighwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "txbiaslow") == 0 )
	{
		if( alarmlevel )
		{
			pNode->txbiaslowalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->txbiaslowalarmthr = alarmthr;
				pNode->txbiaslowalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->txbiaslowwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->txbiaslowwarningthr = alarmthr;
				pNode->txbiaslowwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "vcchigh") == 0 )
	{
		if( alarmlevel )
		{
			pNode->vcchighalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->vcchighalarmthr = alarmthr;
				pNode->vcchighalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->vcchighwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->vcchighwarningthr = alarmthr;
				pNode->vcchighwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "vcclow") == 0 )
	{
		if( alarmlevel )
		{
			pNode->vcclowalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->vcclowalarmthr = alarmthr;
				pNode->vcclowalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->vcclowwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->vcclowwarningthr = alarmthr;
				pNode->vcclowwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "temphigh") == 0 )
	{
		if( alarmlevel )
		{
			pNode->temphighalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->temphighalarmthr = alarmthr;
				pNode->temphighalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->temphighwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->temphighwarningthr = alarmthr;
				pNode->temphighwarningclearthr = clearthr;
			}
		}
	}
	else if( VOS_StriCmp(argv[2], "templow") == 0 )
	{
		if( alarmlevel )
		{
			pNode->templowalarm = flagenable;
			if( thresholdAssign )
			{
				pNode->templowalarmthr = alarmthr;
				pNode->templowalarmclearthr = clearthr;
			}
		}
		else
		{
			pNode->templowwarning = flagenable;
			if( thresholdAssign )
			{
				pNode->templowwarningthr = alarmthr;
				pNode->templowwarningclearthr = clearthr;
			}
		}
	}

	return  CMD_SUCCESS;
}

DEFUN(alarm_pon_policy_able_add,
	alarm_pon_policy_able_add_cmd,
	"ctc alarm-pon enable <1-100> [alarm|warning] [rxpowerhigh|rxpowerlow|txpowerhigh|txpowerlow|txbiashigh|txbiaslow|vcchigh|vcclow|temphigh|templow] <alarm> <clear>",
	CTC_STR
	"pon alarm enable\n"
	"pon alarm enable\n"
	"input policy number\n"
	"alarm level\n"
	"warning level\n"
	"rx optical power high\n"
	"rx optical power low\n"
	"tx optical power high\n"
	"tx optical power low\n"
	"tx bias high\n"
	"tx bias low\n"
	"vcc high\n"
	"vcc low\n"
	"temperature high\n"
	"temperature low\n"
	"input alarm or warning threshold\n"
	"input alarm or warning clear threshold\n"
	)
{
	return pon_alarm_enable_cmd_process( vty, argc, argv, 1 );
}

DEFUN(undo_alarm_pon_policy_able_add,
	undo_alarm_pon_policy_able_add_cmd,
	"undo ctc alarm-pon enable <1-100> [alarm|warning] [rxpowerhigh|rxpowerlow|txpowerhigh|txpowerlow|txbiashigh|txbiaslow|vcchigh|vcclow|temphigh|templow]",
	"undo\n"
	CTC_STR
	"pon alarm enable\n"
	"pon alarm enable\n"
	"input policy number\n"
	"alarm level\n"
	"warning level\n"
	"rx optical power high\n"
	"rx optical power low\n"
	"tx optical power high\n"
	"tx optical power low\n"
	"tx bias high\n"
	"tx bias low\n"
	"vcc high\n"
	"vcc low\n"
	"temperature high\n"
	"temperature low\n"
	"input alarm or warning threshold\n"
	"input alarm or warning clear threshold\n"
	)
{
	return pon_alarm_enable_cmd_process( vty, argc, argv, 0 );
}


VOID showAlarmPonEnbleGroup( struct vty *vty)
{
	Alarm_Pon_info_t *pNode = pEthCtcAlarmPonHead;

	while(pNode != NULL)
	{
		vty_out(vty,"\r\n AlarmPolicyId  is %d \r\n",pNode->alarmpolicyid);
		vty_out(vty," %-20s %-8s %-8s %s\r\n", "Type","Enable", "Thr", "Clearthr\r\n");
	
		vty_out(vty," %-20s %-8s %-8d %d\r\n","RxPowerHighAlarm", enable_to_str(pNode->rxpowerhighalarm), pNode->rxpowerhighalarmthr, pNode->rxpowerhighalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","RxPowerLowAlarm", enable_to_str(pNode->rxpowerlowalarm), pNode->rxpowerlowalarmthr, pNode->rxpowerlowalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxPowerHighAlarm", enable_to_str(pNode->txpowerhighalarm), pNode->txpowerhighalarmthr, pNode->txpowerhighalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxPowerLowAlarm", enable_to_str(pNode->txpowerlowalarm), pNode->txpowerlowalarmthr, pNode->txpowerlowalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxBiasHighAlarm", enable_to_str(pNode->txbiashighalarm), pNode->txbiashighalarmthr, pNode->txbiashighalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxBiasLowAlarm", enable_to_str(pNode->txbiaslowalarm), pNode->txbiaslowalarmthr, pNode->txbiaslowalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","VccHighAlarm", enable_to_str(pNode->vcchighalarm), pNode->vcchighalarmthr, pNode->vcchighalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","VccLowalArm", enable_to_str(pNode->vcclowalarm), pNode->vcclowalarmthr, pNode->vcclowalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TempHighAlarm", enable_to_str(pNode->temphighalarm), pNode->temphighalarmthr, pNode->temphighalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TempLowAlarm", enable_to_str(pNode->templowalarm), pNode->templowalarmthr, pNode->templowalarmclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","RxPowerHighWarning", enable_to_str(pNode->rxpowerhighwarning), pNode->rxpowerhighwarningthr, pNode->rxpowerhighwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","RxPowerLowWarning", enable_to_str(pNode->rxpowerlowwarning), pNode->rxpowerlowwarningthr, pNode->rxpowerlowwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxPowerHighWarning", enable_to_str(pNode->txpowerhighwarning), pNode->txpowerhighwarningthr, pNode->txpowerhighwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxPowerLowWarning", enable_to_str(pNode->txpowerlowwarning), pNode->txpowerlowwarningthr, pNode->txpowerlowwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxBiasHighWarning", enable_to_str(pNode->txbiashighwarning), pNode->txbiashighwarningthr, pNode->txbiashighwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TxBiasLowWarning", enable_to_str(pNode->txbiaslowwarning), pNode->txbiaslowwarningthr, pNode->txbiaslowwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","VccHighWarning", enable_to_str(pNode->vcchighwarning), pNode->vcchighwarningthr, pNode->vcchighwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","VccLowWarning", enable_to_str(pNode->vcclowwarning), pNode->vcclowwarningthr, pNode->vcclowwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TempHighWarning", enable_to_str(pNode->temphighwarning), pNode->temphighwarningthr, pNode->temphighwarningclearthr);
		vty_out(vty," %-20s %-8s %-8d %d\r\n","TempLowWarning", enable_to_str(pNode->templowwarning), pNode->templowwarningthr, pNode->templowwarningclearthr);

		pNode = pNode->nextNode;
	}
	return ;
}


DEFUN(alarm_pon_show,
	alarm_pon_show_cmd,
       "show ctc alarm-pon",
	SHOW_STR
	CTC_STR
	"alarm pon\n"
	)
{
	showAlarmPonEnbleGroup(vty) ;
	return  CMD_SUCCESS;
}

DEFUN(alarm_pon_map,
	alarm_pon_map_cmd,
       "ctc alarm-map pon <slot/port/onuid> <1-100>",
	CTC_STR
	"pon alarm policy map\n"
	"pon alarm policy map\n"
	"input onu id\n"
	"input alarm-pon policy number\n")
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	ULONG GroupId;
	Alarm_Pon_info_t  *pNode;
	LogicEntity *pEntry = NULL;

	GroupId=VOS_AtoL( argv[1] );
	pNode =(Alarm_Pon_info_t  *)CTC_PonAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		return CMD_WARNING;
	}
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
/*	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       	vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}*/
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_PON] = GroupId;
	}

	return  CMD_SUCCESS;
}

DEFUN(undo_alarm_pon_map,
	undo_alarm_pon_map_cmd,
       "undo ctc alarm-map pon <slotId/port/onuId>",
       "undo\n"
       CTC_STR
	"pon alarm policy map\n"
	"pon alarm policy map\n"
	"input onu id\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;

 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
/*	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       	vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}*/
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_PON] = 0;
	}

	return  CMD_SUCCESS;
}
/*
DEFUN(alarm_pon_map_show,
	alarm_pon_map_show_cmd,
       "show ctcalarmpon-policy map  <slotId/port/onuId>",
       SHOW_STR
	"ctcAlarm Pon Policy\n"
	"map\n"
	"Please input slotId/port/onuId\n\n")
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;
	
	sscanf( argv[0], "%d/%d/%d", &ulslotId, &ulport, &ulonuId );

	dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;
	sys_console_printf("dexIdx is %d\r\n",dexIdx);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		vty_out(vty,"ONU %d/%d/%d map %d \r\n",ulslotId,ulport,ulonuId,pEntry->ctcAlarmSelect[CTC_ALARM_PON]);
	}
	return  CMD_SUCCESS;
}
*/
Alarm_Port_info_t * CTC_PortAlarm_Find_Node( const ULONG GroupId )
{
	Alarm_Port_info_t *pCur=pEthCtcAlarmPortHead;

	while(pCur != NULL)
	{
		if(pCur->EthAlarmPolicyId == GroupId )
			return pCur;
		pCur = pCur->nextNode;
	}

	return NULL;
}

STATUS CTC_AlarmPort_Add_Node( const ULONG GroupId )
{
	Alarm_Port_info_t *pCur=pEthCtcAlarmPortHead,*pNewNode = NULL, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->EthAlarmPolicyId== GroupId )
		{
			sys_console_printf("The Group Id has existed !\r\n");
			return VOS_ERROR;
		}
		else if(GroupId <= pCur->EthAlarmPolicyId)
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	pNewNode = VOS_Malloc( sizeof(Alarm_Port_info_t), MODULE_RPU_ONU );	
	if( pNewNode == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemSet( pNewNode, 0, sizeof(Alarm_Port_info_t) );
	pNewNode->nextNode = NULL;
	pNewNode->EthAlarmPolicyId = GroupId;
	
	if(pre ==NULL)
	{
		pNewNode->nextNode = pEthCtcAlarmPortHead;
		pEthCtcAlarmPortHead = pNewNode;
	}
	else
	{
		pNewNode->nextNode = pre->nextNode;
		pre->nextNode = pNewNode;
	}
	return VOS_OK;
}

STATUS CTC_AlarmPort_Del_Node( const ULONG GroupId )
{
	Alarm_Port_info_t *pCur=pEthCtcAlarmPortHead, *pre = NULL;

	while(pCur != NULL)
	{
		if(pCur->EthAlarmPolicyId == GroupId )
			break;
		pre = pCur;
		pCur = pCur->nextNode;
	}

	if(pCur == NULL)
	{
		sys_console_printf("The Group does not exist !\r\n");
		return VOS_ERROR;
	}
	else
	{
		if(pre == NULL)
		{
			pEthCtcAlarmPortHead = pCur->nextNode;
		}
		else
		{
			pre->nextNode = pCur->nextNode;
		}
		VOS_Free(pCur);
		return VOS_OK;
	}
	
	return VOS_OK;
}


DEFUN(alarmPort_policy_add,
	alarmPort_policy_add_cmd,
	"ctc alarm-port policy [add|delete] <1-100>",
	CTC_STR
       "onu port alarm policy\n"
       "onu alarm policy\n"
       "add\n"
       "delete\n"
	"input policy number\n"
	)
{
	ULONG GroupId;
	GroupId=VOS_AtoL( argv[1] );
	if(VOS_StriCmp(argv[0], "add")==0)
	{
		if(CTC_AlarmPort_Add_Node( GroupId )==VOS_ERROR)
			return CMD_WARNING;
	}
	else if(VOS_StriCmp(argv[0], "delete")==0)
	{
		if(CTC_AlarmPort_Del_Node( GroupId)==VOS_ERROR)
			return CMD_WARNING;
	}
	return  CMD_SUCCESS;
}

static int eth_alarm_enable_cmd_process( struct vty *vty, int argc, char **argv, ULONG flagenable )
{
	ULONG GroupId;
	Alarm_Port_info_t  *pNode;

	GroupId=VOS_AtoL( argv[0] );
	pNode =(Alarm_Port_info_t  *)CTC_PortAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		return CMD_WARNING;
	}

	if(VOS_StriCmp(argv[1], "autoneg") == 0 )
		pNode->EthAutoNegFailure = flagenable;
	else if(VOS_StriCmp(argv[1], "los")==0)
		pNode->EthLos = flagenable;
	else if(VOS_StriCmp(argv[1], "failure")==0)
		pNode->EthFailure = flagenable;
	else if(VOS_StriCmp(argv[1], "loopback")==0)
		pNode->EthLoopback = flagenable;
	else if(VOS_StriCmp(argv[1], "congestion")==0)
		pNode->EthCongestion = flagenable;

	return  CMD_SUCCESS;
}

DEFUN(alarmPort_policy_able_add,
	alarmPort_policy_able_add_cmd,
	"ctc alarm-eth enable <1-100> [autoneg|los|failure|loopback|congestion]",
	CTC_STR
	"onu eth-port alarm enable\n"
	"onu eth-port alarm enable\n"
	"input policy number\n"
	"ethernet port autoneg failure\n"
	"ethernet port LOS\n"
	"ethernet port failure\n"
	"ethernet port loopback\n"
	"ethernet port congestion\n"
	)
{
	return eth_alarm_enable_cmd_process( vty, argc, argv, 1 );
}

DEFUN(undo_alarmPort_policy_able_add,
	undo_alarmPort_policy_able_add_cmd,
	"undo ctc alarm-eth enable <1-100> [autoneg|los|failure|loopback|congestion]",
	"undo\n"
	CTC_STR
	"onu eth-port alarm enable\n"
	"onu eth-port alarm enable\n"
	"input policy number\n"
	"ethernet port autoneg failure\n"
	"ethernet port LOS\n"
	"ethernet port failure\n"
	"ethernet port loopback\n"
	"ethernet port congestion\n"
	)
{
	return eth_alarm_enable_cmd_process( vty, argc, argv, 0 );
}

DEFUN(alarmPort_policy_able_add2,
	alarmPort_policy_able_add2_cmd,
	"ctc alarm-eth <1-100> portlist {[all|<port_list>]}*1",
	CTC_STR
	"onu eth-port alarm enable\n"
	"input policy number\n"
	"all ports\n"
	"input port number\n")
{
	ULONG GroupId;
	Alarm_Port_info_t  *pNode;
	GroupId=VOS_AtoL( argv[0] );
	pNode =(Alarm_Port_info_t  *)CTC_PortAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	
	if(argc == 2)
	{
		if( VOS_StrLen(argv[1]) >= 32 )
		{
			vty_out( vty, "portlist is too long\r\n" );
			return CMD_WARNING;
		}
		VOS_StrCpy(pNode->pcPort_List, argv[1]);
	}
	else
		VOS_StrCpy(pNode->pcPort_List, "all" );
		
	return  CMD_SUCCESS;
}

VOID showAlarmPortEnbleGroup( struct vty *vty)
{
	Alarm_Port_info_t *pNode = pEthCtcAlarmPortHead;

	while(pNode != NULL)
	{
		vty_out(vty,"\r\n AlarmPolicyId  is %d \r\n",pNode->EthAlarmPolicyId);
		vty_out(vty," %-20s %-8s\r\n", "Type","Enable\r\n");
	
		vty_out(vty," %-20s %-8s\r\n","EthAutoNegFailure", enable_to_str(pNode->EthAutoNegFailure));
		vty_out(vty," %-20s %-8s\r\n","EthLos", enable_to_str(pNode->EthLos));
		vty_out(vty," %-20s %-8s\r\n","EthFailure", enable_to_str(pNode->EthFailure));
		vty_out(vty," %-20s %-8s\r\n","EthLoopback", enable_to_str(pNode->EthLoopback));
		vty_out(vty," %-20s %-8s\r\n","EthCongestion", enable_to_str(pNode->EthCongestion));
		vty_out(vty," %-20s %-8s\r\n","PortList", pNode->pcPort_List);

		pNode = pNode->nextNode;
	}
	return ;
}
DEFUN(alarm_port_map,
	alarm_port_map_cmd,
       "ctc alarm-map eth <slot/port/onuid> <1-100>",
       CTC_STR
	"eth-port alarm policy map\n"
	"eth-port alarm policy map\n"
	"input onu id\n"
	"input policy number\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	ULONG GroupId;
	Alarm_Port_info_t  *pNode;
	LogicEntity *pEntry = NULL;

	GroupId=VOS_AtoL( argv[1] );
	pNode =(Alarm_Port_info_t  *)CTC_PortAlarm_Find_Node( GroupId );
	if( pNode == NULL )
	{
		vty_out(vty,"The Group does not exist !\r\n");
		return CMD_WARNING;
	}
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
/*	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       	vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}*/
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_PORT] = GroupId;
	}

	return  CMD_SUCCESS;
}

DEFUN(undo_alarm_port_map,
	undo_alarm_port_map_cmd,
       "undo ctc alarm-map eth <slot/port/onuid>",
       "undo\n"
       CTC_STR
	"eth-port alarm policy map\n"
	"eth-port alarm policy map\n"
	"input onu id\n"
	)
{
	ULONG ulslotId = 0;
	ULONG ulonuId = 0; 
	ULONG ulport = 0;
	ULONG dexIdx = 0;
	LogicEntity *pEntry = NULL;
	
 	if( PON_ParseSlotPortOnu( argv[0], &ulslotId, &ulport, &ulonuId )  != VOS_OK )
		return CMD_WARNING;
/*	if ((ulonuId<(CLI_EPON_ONUMIN+1)) && (ulonuId>(CLI_EPON_ONUMAX+1)))
	{
       	vty_out( vty, "  %% Onuid error. \r\n");
		return CMD_WARNING;	
	}*/
	/*dexIdx = 10000*ulslotId + 1000*ulport + ulonuId;*/
        dexIdx=MAKEDEVID(ulslotId,ulport,ulonuId);
	if( getLogicEntity( dexIdx, &pEntry ) == VOS_OK )
	{
		pEntry->ctcAlarmSelect[CTC_ALARM_PORT] = 0;
	}

	return  CMD_SUCCESS;
}

DEFUN(alarm_port_show,
	alarm_port_show_cmd,
       "show ctc alarm-eth",
	SHOW_STR
	CTC_STR
	"onu eth-port alarm"
	)
{
	showAlarmPortEnbleGroup(vty) ;
	return  CMD_SUCCESS;
}

LONG  CT_Alarm_Init()
{
	install_element ( CONFIG_NODE, &ctc_alarm_onu_policy_add_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_policy_able_add_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_onu_policy_able_add_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_policy_thr_add_cmd );
	install_element ( CONFIG_NODE, &undo_alarm_onu_policy_thr_add_cmd );
	
	install_element ( CONFIG_NODE, &alarm_onu_show_cmd);
	install_element ( CONFIG_NODE, &alarm_onu_map_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_onu_map_cmd);
	
	install_element ( CONFIG_NODE, &alarm_map_show_cmd);
	install_element ( CONFIG_NODE, &ctc_alarm_action_cmd);


	install_element ( CONFIG_NODE, &alarmPon_policy_add_cmd);
	install_element ( CONFIG_NODE, &alarm_pon_policy_able_add_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_pon_policy_able_add_cmd);
	
	install_element ( CONFIG_NODE, &alarm_pon_show_cmd);
	install_element ( CONFIG_NODE, &alarm_pon_map_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_pon_map_cmd);
	
	install_element ( CONFIG_NODE, &alarmPort_policy_add_cmd);
	install_element ( CONFIG_NODE, &alarmPort_policy_able_add_cmd);
	install_element ( CONFIG_NODE, &undo_alarmPort_policy_able_add_cmd);
	install_element ( CONFIG_NODE, &alarmPort_policy_able_add2_cmd);
	install_element ( CONFIG_NODE, &alarm_port_map_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_port_map_cmd);
	install_element ( CONFIG_NODE, &alarm_port_show_cmd);

	/*install_element ( CONFIG_NODE, &onu_status_show_cmd);
	install_element ( CONFIG_NODE, &onu_status_test_cmd);*/
	
	return VOS_OK;
}
#endif
#endif

