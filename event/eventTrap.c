#include  "OltGeneral.h"
/*#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"*/
#include "gwEponSys.h"
#include "lib_gwEponMib.h"
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
#include "trace_path/trace_path_lib.h"
#include  "trace_path/trace_path_api.h"
#endif

/*#include "sys/main/sys_main.h"*/
#include "snmp/sn_a_trp.h"
#include "snmp/sn_snmp.h"

#include "eventMain.h"
#include "eventTrap.h"


int bindMib2TrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum )
{
	oid mib2_trap_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 5 };
	ulong_t oidLen = sizeof( mib2_trap_oid )/sizeof(oid);

	VOS_MemZero( trapvar, sizeof( SNMPTRAP ) );
	trapvar->GenericType = SNMP_TRAP_ENTERPRISESPECIFIC;
	trapvar->SpecificType = 10;
	VOS_MemCpy( trapvar->TrapOid, mib2_trap_oid, sizeof( mib2_trap_oid ) );
	trapvar->TrapOid[oidLen] = trapId;
	trapvar->TrapOidLength = oidLen + 1;
	trapvar->VarNum = 0;

	return VOS_OK;
}

int sendMib2Trap( ulong_t trapid )
{
	SNMPTRAP mib2Trap;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		bindMib2TrapVar( &mib2Trap, trapid, 0, 0 );
		return mn_send_msg_trap( &mib2Trap, MSG_PRI_NORMAL );
	}
	return VOS_OK;
}

int bindBridgeTrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum )
{
	oid stp_trap_oid[] = {1, 3, 6, 1, 2, 1, 17, 0};
	ulong_t oidLen = sizeof( stp_trap_oid )/sizeof(oid);

	VOS_MemZero( trapvar, sizeof( SNMPTRAP ) );
	trapvar->GenericType = SNMP_TRAP_ENTERPRISESPECIFIC;
	trapvar->SpecificType = 10;
	VOS_MemCpy( trapvar->TrapOid, stp_trap_oid, sizeof( stp_trap_oid ) );
	trapvar->TrapOid[oidLen] = trapId;
	trapvar->TrapOidLength = oidLen + 1;
	trapvar->VarNum = 0;

	return VOS_OK;
}

int sendBridgeTrap( ulong_t trapid )
{
	SNMPTRAP stpTrap;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		bindBridgeTrapVar( &stpTrap, trapid, 0, 0 );
		return mn_send_msg_trap( &stpTrap, MSG_PRI_NORMAL );
	}
	return VOS_OK;
}

STATUS bindTrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum )
{
	STATUS rc = VOS_ERROR;

	static oid baseoid[] = {1,3,6,1,4,1,10072,2,20,1,1,6};
	static oid deviceIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,1};
	static oid boardIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,2,1,1,1};
	static oid ponportIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,1,1,1};
	static oid ponLlidIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,5,1,1,1};
	static oid deviceMacAddressOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,12};
	static oid ponPortAlarmStatusOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,1,1,14};

	/*static oid  ponportlosOid[]={1,3,6,1,4,1,10072,2,20,1,1,3,1,1,14};*/ /*add by shixh20090625*/
#if 0
	static oid e1devIdxOid[] = {1,3,6,1,4,1,10072,2,23,1,2,1,1};
	static oid e1brdIdxOid[] = {1,3,6,1,4,1,10072,2,23,1,2,1,2};
	static oid e1clusterIdxOid[] = {1,3,6,1,4,1,10072,2,23,1,2,1,3};
#endif


	static oid ethDevIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,1};
	static oid ethBrdIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,2};
	static oid ethPortIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,3};
	static oid switchPortIdxOid[] = {1,3,6,1,4,1,10072,2,23,2,2,2,1,1,2};/*add by shixh20090521*/
	static oid switchMacIdxOid[] = {1,3,6,1,4,1,10072,2,23,2,2,2,1,1,1};
	static oid BRASmacOid[]={1,3,6,1,4,1,10072,2,20,1,1,1,2,1,4};    /*add by shixh20090720*/

	static oid ponUpBerOid[] =	{1,3,6,1,4,1,10072,2,20,1,1,3,3,1,1};
	static oid ponDownBerOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,3,1,2};

	static oid ponUpFerOid[] =	{1,3,6,1,4,1,10072,2,20,1,1,3,3,1,3};
	static oid ponDownFerOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,3,1,4};

	static oid deviceTypeOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,2};
	/*static oid deviceLogicCommOid[]	= {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,18};*/
	static oid deviceSoftWareVerOid[]	= {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,8};/* added by xieshl 20070703 */
	static oid deviceFirmWareVerOid[]	= {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,7};
	static oid deviceHardWareVerOid[]	= {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,9};
	static oid deviceRestartupTimeOid[]= {1,3,6,1,4,1,10072,2,20,1,1,1,1,1,37};

	static oid curDeviceTypeOid[] = {1,3,6,1,4,1,10072,2,20,1,1,2,1,1,2};

	static oid onuAuthMacOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,8,2,1,4 };

	/*add by shixh@20071017*/
	static oid e1PortDevIdexOid[]={1,3,6,1,4,1,10072,2,23,1,1,1,1};
	static oid e1PortBrdIndexOid[]={1,3,6,1,4,1,10072,2,23,1,1,1,2};
	static oid e1Porte1ClusterIndexOid[]={1,3,6,1,4,1,10072,2,23,1,1,1,3};
	static oid e1PortIndexOid[]={1,3,6,1,4,1,10072,2,23,1,1,1,4};
	/*end add by shixh@20071017*/

	/*static oid portTransmissionPowerOid[]	= {1,3,6,1,4,1,10072,2,19,4,3,1,1,4};
	  static oid portReceiverPowerOid[]		= {1,3,6,1,4,1,10072,2,19,4,3,1,1,5};
	  static oid portModuleTemperatureOid[]	= {1,3,6,1,4,1,10072,2,19,4,3,1,1,6};
	  static oid portWorkVoltageOid[]		= {1,3,6,1,4,1,10072,2,19,4,3,1,1,7};
	  static oid portBiasCurrentOid[]			= {1,3,6,1,4,1,10072,2,19,4,3,1,1,8};
	  static oid portTemperatureOid[]		= {1,3,6,1,4,1,10072,2,19,4,3,1,1,9};*//*add byshixh@20080820*/

	static oid uplinkReceiverPowerLowOid[]	={1,3,6,1,4,1,10072,2,19,4,6,1};
	static oid uplinkReceiverPowerHighOid[]    ={1,3,6,1,4,1,10072,2,19,4,6,2};
	static oid uplinkTransmissionPowerLowOid[]={1,3,6,1,4,1,10072,2,19,4,6,3};
	static oid uplinkTransmissionPowerHighOid[]={1,3,6,1,4,1,10072,2,19,4,6,4};
	static oid uplinkModuleTemperatureLowOid[]={1,3,6,1,4,1,10072,2,19,4,6,5};
	static oid uplinkModuleTemperatureHighOid[]={1,3,6,1,4,1,10072,2,19,4,6,6};
	static oid uplinkWorkVoltageLowOid[]            ={1,3,6,1,4,1,10072,2,19,4,6,7};
	static oid uplinkWorkVoltageHighOid[]           ={1,3,6,1,4,1,10072,2,19,4,6,8};
	static oid uplinkBiasCurrentLowOid[]                    ={1,3,6,1,4,1,10072,2,19,4,6,9};
	static oid uplinkBiasCurrentHighOid[]                    ={1,3,6,1,4,1,10072,2,19,4,6,10};
	/*ctc onu temp threshold*/
	static oid ctcOnuBatteryVoltLowOid[]={1,3,6,1,4,1,10072,2,19,4,8,1};
	static oid ctcOnuTempLowOid[]	={1,3,6,1,4,1,10072,2,19,4,8,2};
	static oid ctcOnuTempHighOid[]	={1,3,6,1,4,1,10072,2,19,4,8,3};
	/*Dos attack*/
	/*static oid ethportDoSattack[]={1,3,6,1,4,1,10072,2,19,4,6,11};
	  static oid ethportDoSattackClear[]={1,3,6,1,4,1,10072,2,19,4,6,12};*/


	/*modfied by shixh@20080830*/
	static oid ponReceiverPowerLowOid[]	={1,3,6,1,4,1,10072,2,19,4,1,1};
	static oid ponReceiverPowerHighOid[]    ={1,3,6,1,4,1,10072,2,19,4,1,2};
	static oid ponTransmissionPowerLowOid[]={1,3,6,1,4,1,10072,2,19,4,1,3};
	static oid ponTransmissionPowerHighOid[]={1,3,6,1,4,1,10072,2,19,4,1,4};
	static oid ponModuleTemperatureLowOid[]={1,3,6,1,4,1,10072,2,19,4,1,5};
	static oid ponModuleTemperatureHighOid[]={1,3,6,1,4,1,10072,2,19,4,1,6};
	static oid ponWorkVoltageLowOid[]            ={1,3,6,1,4,1,10072,2,19,4,1,7};
	static oid ponWorkVoltageHighOid[]           ={1,3,6,1,4,1,10072,2,19,4,1,8};
	static oid ponBiasCurrentLowOid[]                    ={1,3,6,1,4,1,10072,2,19,4,1,9};
	static oid ponBiasCurrentHighOid[]                    ={1,3,6,1,4,1,10072,2,19,4,1,10};

	/*add by shixh@20080831*/
	static oid oltRxdeviceIdxOid[] = {1,3,6,1,4,1,10072,2,19,4,4,2,1,1};
	static oid oltRxboardIdxOid[] = {1,3,6,1,4,1,10072,2,19,4,4,2,1,2};
	static oid oltRxponportIdxOid[] = {1,3,6,1,4,1,10072,2,19,4,4,2,1,3};
	static oid oltRxonuIndexOid[]                    ={1,3,6,1,4,1,10072,2,19,4,4,2,1,4};
	static oid oltReceiverPowerOid[]                ={1,3,6,1,4,1,10072,2,19,4,4,2,1,5};	
	static oid  onuAutoLoadUpgradeOid[]={1,3,6,1,4,1,10072,2,12,3,3,1,2};
	const ulong_t baselen = sizeof( baseoid )/sizeof(oid);

	static oid onuPredefPonSlotIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,2,1,1};
	static oid onuPredefPonPortIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,2,1,2};
	static oid onuPredefOnuIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,2,1,3};
	static oid onuPredefOnuMacAddrOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,2,1,4};
	static oid onuPredefOnuDevIdxOid[] = {1,3,6,1,4,1,10072,2,20,1,1,1,2,1,6};

#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
	oid userLocUserIdOid[]	  = {1,3,6,1,4,1,10072,2,26,2,1,1,1};
	oid macTraceMacAddrOid[] = {1,3,6,1,4,1,10072,2,26,1,1,1,1};
	int macTraceOidLen = sizeof(macTraceMacAddrOid)/sizeof(oid);
	int i = 0;
#endif
	static oid logicalSlotRemoteIpOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,8,1,10};
	static oid logicalSlotRemotePortOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,8,1,11};
	static oid remotePhysicalSlotOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,8,1,12};
	static oid remotePhysicalPortOid[] = {1,3,6,1,4,1,10072,2,20,1,1,3,8,1,13};

	
	static oid pwureportalarmidOid[]={1,3,6,1,4,1,10072,2,15,1,5,1,1,13};
	static oid pwureportclearalarmidOid[]={1,3,6,1,4,1,10072,2,15,1,5,1,1,14};


	/*VOS_MemSet( &trapvar, 0, sizeof(trapvar) );*/
	VOS_MemSet( trapvar, 0, sizeof(SNMPTRAP) );
	trapvar->GenericType = SNMP_TRAP_ENTERPRISESPECIFIC;
	/*trapvar->SpecificType = ?;*/
	trapvar->SpecificType = 10;

	trapvar->VarNum = varNum;

	switch( trapId )
	{
		case trap_onuNotPresent:
		case trap_devPowerOff:
		case trap_cfgDataSaveSuccess:
		case trap_cfgDataSaveFail:
		case trap_flashClearSuccess:
		case trap_flashClearFail:
		case trap_softwareUpdateSuccess:
		case trap_softwareUpdateFail:
		case trap_firmwareUpdateSuccess:
		case trap_firmwareUpdateFail:
		case trap_cfgDataBackupSuccess:
		case trap_cfgDataBackupFail:
		case trap_cfgDataRestoreSuccess:
		case trap_cfgDataRestoreFail:
		case trap_cpuUsageFactorHigh:
		case trap_onuRegisterConflict:
		case trap_dbaUpdateSuccess:
		case trap_dbaUpdateFailure:
		case trap_onuSoftwareLoadSuccess:
		case trap_onuSoftwareLoadFailure:
		case trap_onuMacTableOverFlow:   /*add bysxh20111227*/
		case trap_onuMacTableOverFlowClear:
		case trap_bootUpdateSuccess:
		case trap_bootUpdateFailure:
		case trap_batfileBackupSuccess:
		case trap_batfileBackupFailure:
		case trap_batfileRestoreSuccess:
		case trap_batfileRestoreFailure:
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
		case trap_deviceTemperatureHigh:
		case trap_deviceTemperatureHighClear:
		case trap_deviceTemperatureLow:
		case trap_deviceTemperatureLowClear:
#endif
			if( varNum != 1 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;


			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;
			rc = VOS_OK;
			break;

		case trap_devFanAlarm:
		case trap_devFanAlarmClear:
		case trap_powerOffAlarm:
		case trap_powerOnAlarm:

		case trap_boardTemperatureHigh:
		case trap_boardTemperatureHighClear:
		case trap_ponBoardReset:
			/*add by shixh@20080831*/
		case trap_boardCpuUsageAlarm:   
		case trap_boardCpuUsageAlarmClear:
		case trap_boardMemoryUsageAlarm:
		case trap_boardMemoryUsageAlarmClear:

		case trap_swBoardProtectedSwitch:				
			if( varNum != 2 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;
			rc = VOS_OK;				
			break;
			/*			case trap_devPowerOn:
						if( varNum != 2 )
						break;
						VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
						trapvar->TrapOid[baselen] = trapId;
						trapvar->TrapOidLength = baselen+1;

						VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
						trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			 *(ulong_t*)trapvar->Var[0] = pVarlist[0];
			 trapvar->VarLength[0] = sizeof(ulong_t);
			 trapvar->VarType[0] = ASN_INTEGER;

			 VOS_MemCpy( trapvar->VarOid[1], deviceTypeOid, sizeof( deviceTypeOid ) );
			 trapvar->VarOidLength[1] = sizeof(deviceTypeOid)/sizeof(oid);
			 *(ulong_t*)trapvar->Var[1] = pVarlist[1];
			 trapvar->VarLength[1] = sizeof(ulong_t);
			 trapvar->VarType[1] = ASN_INTEGER;
			 rc = VOS_OK;				
			 break;
			 */	/* erased by xieshl 20070703 */

		/*BEGIN: add binding info by @muqw 2017-4-26*/
		case trap_pwuStatusAbnoarmal:
			if( varNum != 3 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], pwureportalarmidOid, sizeof( pwureportalarmidOid ) );
			trapvar->VarOidLength[2] = sizeof(pwureportalarmidOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;
			rc = VOS_OK;
			break;

		case trap_pwuStatusAbnoarmalClear:
			if( varNum != 3 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], pwureportclearalarmidOid, sizeof( pwureportclearalarmidOid ) );
			trapvar->VarOidLength[2] = sizeof(pwureportclearalarmidOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;
			rc = VOS_OK;
			break;
		/*END: add binding info by @muqw 2017-4-26*/


		case trap_devBoardInterted:
		case trap_devBoardPull:
		case trap_boardLosAlarm:
		case trap_boardLosAlarmClear:
			/* begin: added by jianght 20090603 */
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
			/*case trap_PWUPowerOff:
			  case trap_PWUPowerOn:*/
#endif
			/* end: added by jianght 20090603 */

			if( varNum != 3 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], curDeviceTypeOid, sizeof( curDeviceTypeOid ) );
			trapvar->VarOidLength[2] = sizeof(curDeviceTypeOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;
			rc = VOS_OK;
			break;
		case  trap_ethLinkdown:
		case  trap_ethLinkup:
		case  trap_ethFerAlarm:
		case  trap_ethFerAlarmClear:
		case  trap_ethFlrAlarm:
		case  trap_ethFlrAlarmClear:
		case  trap_ethTranmittalIntermitAlarm:
		case  trap_ethTranmittalIntermitAlarmClear:
			if(varNum!=3)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			rc = VOS_OK;
			break;
		case trap_firmwareLoadSuccess:
		case trap_firmwareLoadFailure:
		case trap_dbaLoadSuccess:
		case trap_dbaLoadFailure:
		case trap_autoProtectSwitch:				

		case trap_ponToEthLinkdown:	/* added 20070703 */
		case trap_ponToEthLinkup:
		case trap_PonPortFullAlarm:  /*add by shixh20090507*/
		case trap_ponLaserAlwaysOnAlarm:		/* added by xieshl 20080812 */
		case trap_ponLaserAlwaysOnAlarmClear:
		case trap_ponPortAbnormal:
		case trap_ponPortAbnormalClear:
		case trap_ponPortlosAlarm:       /*add by shixh20090626*/
		case trap_ponPortlosAlarmClear:
		case trap_ponFWVersionMismatch:/*add by shixh20090710*/
		case trap_ponFWVersionMatch:
		case trap_ponDBAVersionMismatch:
		case trap_ponDBAVersionMatch:
		case trap_ponSFPTypeMismatch:      
		case trap_ponSFPTypeMitch:
			if( varNum != 3 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;		
			rc = VOS_OK;
			break;
#if 0
		case  trap_ponPortlosAlarm:          /*add by shixh20090629*/
		case trap_ponPortlosAlarmClear:
			if( varNum != 4 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			VOS_MemCpy( trapvar->VarOid[3], ponportlosOid, sizeof( ponportlosOid ) );
			trapvar->VarOidLength[3] = sizeof(ponportlosOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;	
			rc = VOS_OK;
			break;
#endif
		case trap_llidActBWExceeding:
		case trap_llidActBWExceedingClear:
			if( varNum != 4 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], ponLlidIdxOid, sizeof( ponLlidIdxOid ) );
			trapvar->VarOidLength[3] = sizeof(ponLlidIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;	
			rc = VOS_OK;				
			break;
		case trap_ponPortBERAlarm:		/* modified by xieshl 20080812 */
		case trap_ponPortBERAlarmClear:
		case trap_ponPortFERAlarm:
		case trap_ponPortFERAlarmClear:
			/* added by xieshl 20080812 */	
		case trap_ponReceiverPowerTooLow:
		case trap_ponReceiverPowerTooLowClear:
		case trap_ponReceiverPowerTooHigh:
		case trap_ponReceiverPowerTooHighClear:
		case trap_ponTransmissionPowerTooLow:
		case trap_ponTransmissionPowerTooLowClear:
		case trap_ponTransmissionPowerTooHigh:
		case trap_ponTransmissionPowerTooHighClear:
		case trap_ponAppliedVoltageTooHigh:
		case trap_ponAppliedVoltageTooHighClear:
		case trap_ponAppliedVoltageTooLow:
		case trap_ponAppliedVoltageTooLowClear:
		case trap_ponBiasCurrentTooHigh:
		case trap_ponBiasCurrentTooHighClear:
		case trap_ponBiasCurrentTooLow:
		case trap_ponBiasCurrentTooLowClear:
			/* end 20080812 */
		case trap_ponTemperatureTooHigh:
		case trap_ponTemperatureTooHighClear:
		case trap_ponTemperatureTooLow:
		case trap_ponTemperatureTooLowClear:
			if( varNum != 4 )
				break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			if( (trapId == trap_ponPortBERAlarm) || (trapId == trap_ponPortBERAlarmClear) )
			{
				if( pVarlist[0] == 1 )
				{
					VOS_MemCpy( trapvar->VarOid[3], ponUpBerOid, sizeof(ponUpBerOid));
					trapvar->VarOidLength[3] = sizeof(ponUpBerOid)/sizeof(oid);
				}
				else
				{
					VOS_MemCpy( trapvar->VarOid[3], ponDownBerOid, sizeof(ponDownBerOid));
					trapvar->VarOidLength[3] = sizeof(ponDownBerOid)/sizeof(oid);					
				}
			}
			else if( (trapId == trap_ponPortFERAlarm) || (trapId == trap_ponPortFERAlarmClear))
			{
				if( pVarlist[0] == 1 )
				{
					VOS_MemCpy( trapvar->VarOid[3], ponUpFerOid, sizeof(ponUpFerOid));
					trapvar->VarOidLength[3] = sizeof(ponUpFerOid)/sizeof(oid);
				}
				else
				{
					VOS_MemCpy( trapvar->VarOid[3], ponDownFerOid, sizeof(ponDownFerOid));
					trapvar->VarOidLength[3] = sizeof(ponDownFerOid)/sizeof(oid);					
				}
			}
			else if( (trapId == trap_ponReceiverPowerTooLow) || (trapId == trap_ponReceiverPowerTooLowClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], ponReceiverPowerLowOid, sizeof(ponReceiverPowerLowOid));
				trapvar->VarOidLength[3] = sizeof(ponReceiverPowerLowOid)/sizeof(oid);	
			}
			else if((trapId == trap_ponReceiverPowerTooHigh) || (trapId == trap_ponReceiverPowerTooHighClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], ponReceiverPowerHighOid, sizeof(ponReceiverPowerHighOid));
				trapvar->VarOidLength[3] = sizeof(ponReceiverPowerHighOid)/sizeof(oid);
			}
			else if( (trapId == trap_ponTransmissionPowerTooLow) || (trapId == trap_ponTransmissionPowerTooLowClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], ponTransmissionPowerLowOid, sizeof(ponTransmissionPowerLowOid));
				trapvar->VarOidLength[3] = sizeof(ponTransmissionPowerLowOid)/sizeof(oid);

			}
			else if((trapId == trap_ponTransmissionPowerTooHigh) || (trapId == trap_ponTransmissionPowerTooHighClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], ponTransmissionPowerHighOid, sizeof(ponTransmissionPowerHighOid));
				trapvar->VarOidLength[3] = sizeof(ponTransmissionPowerHighOid)/sizeof(oid);
			}
			else if( (trapId == trap_ponAppliedVoltageTooHigh) || (trapId == trap_ponAppliedVoltageTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], ponWorkVoltageHighOid, sizeof(ponWorkVoltageHighOid));
				trapvar->VarOidLength[3] = sizeof(ponWorkVoltageHighOid)/sizeof(oid);
			}
			else if((trapId == trap_ponAppliedVoltageTooLow) || (trapId == trap_ponAppliedVoltageTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], ponWorkVoltageLowOid, sizeof(ponWorkVoltageLowOid));
				trapvar->VarOidLength[3] = sizeof(ponWorkVoltageLowOid)/sizeof(oid);
			}
			else if( (trapId == trap_ponBiasCurrentTooHigh) || (trapId == trap_ponBiasCurrentTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], ponBiasCurrentHighOid, sizeof(ponBiasCurrentHighOid));
				trapvar->VarOidLength[3] = sizeof(ponBiasCurrentHighOid)/sizeof(oid);
			}
			else if((trapId == trap_ponBiasCurrentTooLow) || (trapId == trap_ponBiasCurrentTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], ponBiasCurrentLowOid, sizeof(ponBiasCurrentLowOid));
				trapvar->VarOidLength[3] = sizeof(ponBiasCurrentLowOid)/sizeof(oid);
			}
			else if( (trapId == trap_ponTemperatureTooHigh) || (trapId == trap_ponTemperatureTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], ponModuleTemperatureHighOid, sizeof(ponModuleTemperatureHighOid) );
				trapvar->VarOidLength[3] = sizeof(ponModuleTemperatureHighOid)/sizeof(oid);
			}
			else if((trapId == trap_ponTemperatureTooLow) || (trapId == trap_ponTemperatureTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], ponModuleTemperatureLowOid, sizeof(ponModuleTemperatureLowOid) );
				trapvar->VarOidLength[3] = sizeof(ponModuleTemperatureLowOid)/sizeof(oid);
			}

			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;	
			rc = VOS_OK;		
			break;
		case trap_uplinkReceiverPowerTooLow:
		case trap_uplinkReceiverPowerTooLowClear:
		case trap_uplinkReceiverPowerTooHigh:
		case trap_uplinkReceiverPowerTooHighClear:
		case trap_uplinkTransmissionPowerTooLow:
		case trap_uplinkTransmissionPowerTooLowClear:
		case trap_uplinkTransmissionPowerTooHigh:
		case trap_uplinkTransmissionPowerTooHighClear:
		case trap_uplinkAppliedVoltageTooHigh:
		case trap_uplinkAppliedVoltageTooHighClear:
		case trap_uplinkAppliedVoltageTooLow:
		case trap_uplinkAppliedVoltageTooLowClear:
		case trap_uplinkBiasCurrentTooHigh:
		case trap_uplinkBiasCurrentTooHighClear:
		case trap_uplinkBiasCurrentTooLow:
		case trap_uplinkBiasCurrentTooLowClear:
		case trap_uplinkTemperatureTooHigh:
		case trap_uplinkTemperatureTooHighClear:
		case trap_uplinkTemperatureTooLow:
		case trap_uplinkTemperatureTooLowClear:
			if( varNum != 4 )
				break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			if( (trapId == trap_uplinkReceiverPowerTooLow) || (trapId == trap_uplinkReceiverPowerTooLowClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkReceiverPowerLowOid, sizeof(uplinkReceiverPowerLowOid));
				trapvar->VarOidLength[3] = sizeof(uplinkReceiverPowerLowOid)/sizeof(oid);	
			}
			else if((trapId == trap_uplinkReceiverPowerTooHigh) || (trapId == trap_uplinkReceiverPowerTooHighClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkReceiverPowerHighOid, sizeof(uplinkReceiverPowerHighOid));
				trapvar->VarOidLength[3] = sizeof(uplinkReceiverPowerHighOid)/sizeof(oid);
			}
			else if( (trapId == trap_uplinkTransmissionPowerTooLow) || (trapId == trap_uplinkTransmissionPowerTooLowClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkTransmissionPowerLowOid, sizeof(uplinkTransmissionPowerLowOid));
				trapvar->VarOidLength[3] = sizeof(uplinkTransmissionPowerLowOid)/sizeof(oid);

			}
			else if((trapId == trap_uplinkTransmissionPowerTooHigh) || (trapId == trap_uplinkTransmissionPowerTooHighClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkTransmissionPowerHighOid, sizeof(uplinkTransmissionPowerHighOid));
				trapvar->VarOidLength[3] = sizeof(uplinkTransmissionPowerHighOid)/sizeof(oid);
			}
			else if( (trapId == trap_uplinkAppliedVoltageTooHigh) || (trapId == trap_uplinkAppliedVoltageTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkWorkVoltageHighOid, sizeof(uplinkWorkVoltageHighOid));
				trapvar->VarOidLength[3] = sizeof(uplinkWorkVoltageHighOid)/sizeof(oid);
			}
			else if((trapId == trap_uplinkAppliedVoltageTooLow) || (trapId == trap_uplinkAppliedVoltageTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkWorkVoltageLowOid, sizeof(uplinkWorkVoltageLowOid));
				trapvar->VarOidLength[3] = sizeof(uplinkWorkVoltageLowOid)/sizeof(oid);
			}
			else if( (trapId == trap_uplinkBiasCurrentTooHigh) || (trapId == trap_uplinkBiasCurrentTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkBiasCurrentHighOid, sizeof(uplinkBiasCurrentHighOid));
				trapvar->VarOidLength[3] = sizeof(uplinkBiasCurrentHighOid)/sizeof(oid);
			}
			else if((trapId == trap_uplinkBiasCurrentTooLow) || (trapId == trap_uplinkBiasCurrentTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkBiasCurrentLowOid, sizeof(uplinkBiasCurrentLowOid));
				trapvar->VarOidLength[3] = sizeof(uplinkBiasCurrentLowOid)/sizeof(oid);
			}
			else if( (trapId == trap_uplinkTemperatureTooHigh) || (trapId == trap_uplinkTemperatureTooHighClear))

			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkModuleTemperatureHighOid, sizeof(uplinkModuleTemperatureHighOid) );
				trapvar->VarOidLength[3] = sizeof(uplinkModuleTemperatureHighOid)/sizeof(oid);
			}
			else if((trapId == trap_uplinkTemperatureTooLow) || (trapId == trap_uplinkTemperatureTooLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[3], uplinkModuleTemperatureLowOid, sizeof(uplinkModuleTemperatureLowOid) );
				trapvar->VarOidLength[3] = sizeof(uplinkModuleTemperatureLowOid)/sizeof(oid);
			}

			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;	
			rc = VOS_OK;		
			break;
		case trap_ctcOnuEquipmentAlarm:
		case trap_ctcOnuEquipmentAlarmClear:
		case trap_ctcOnuBatteryMissing:
		case trap_ctcOnuBatteryMissingClear:
		case trap_ctcOnuBatteryFailure:
		case trap_ctcOnuBatteryFailureClear:
		case trap_ctcOnuPhysicalIntrusionAlarm:
		case trap_ctcOnuPhysicalIntrusionAlarmClear:
		case trap_ctcOnuSelfTestFailure:
		case trap_ctcOnuSelfTestFailureClear:
		case trap_ctcOnuIADConnectionFailure:
		case trap_ctcOnuIADConnectionFailureClear:     
		case trap_ctcOnuPonIfSwitch:
		case trap_ctcOnuPonIfSwitchClear:
			if( varNum != 1 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;


			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;
			rc = VOS_OK;
			break;
		case trap_ctcOnuBatteryVoltLow:
		case trap_ctcOnuBatteryVoltLowClear:
		case trap_ctcOnuTemperatureHigh:
		case trap_ctcOnuTemperatureHighClear:
		case trap_ctcOnuTemperatureLow:
		case trap_ctcOnuTemperatureLowClear:
			if( varNum !=2)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;


			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			if( (trapId == trap_ctcOnuBatteryVoltLow) || (trapId == trap_ctcOnuBatteryVoltLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[1], ctcOnuBatteryVoltLowOid, sizeof(ctcOnuBatteryVoltLowOid));
				trapvar->VarOidLength[1] = sizeof(ctcOnuBatteryVoltLowOid)/sizeof(oid);	
			}
			else if( (trapId == trap_ctcOnuTemperatureHigh) || (trapId == trap_ctcOnuTemperatureHighClear))
			{
				VOS_MemCpy( trapvar->VarOid[1], ctcOnuTempLowOid, sizeof(ctcOnuTempLowOid));
				trapvar->VarOidLength[1] = sizeof(ctcOnuTempLowOid)/sizeof(oid);	
			}
			else if( (trapId == trap_ctcOnuTemperatureLow) || (trapId == trap_ctcOnuTemperatureLowClear))
			{
				VOS_MemCpy( trapvar->VarOid[1], ctcOnuTempHighOid, sizeof(ctcOnuTempHighOid));
				trapvar->VarOidLength[1] = sizeof(ctcOnuTempHighOid)/sizeof(oid);	
			}

			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;	

			rc = VOS_OK;
			break;     

		case trap_ethAutoNegFailure:
		case trap_ethAutoNegFailureClear:
		case trap_ethLos:
		case trap_ethLosCLear:
		case trap_ethFailure:
		case trap_ethFailureClear:
		case trap_ethCongestion:
		case trap_ethCongestionClear:
		case trap_eth_DosAttack:            /*eth port DoS attack*/
		case trap_eth_DosAttackClear:
			if( varNum != 3 )
				break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			/*VOS_MemCpy( trapvar->VarOid[3], ethportDoSattack, sizeof(ethportDoSattack) );
			  trapvar->VarOidLength[3] = sizeof(ethportDoSattack)/sizeof(oid);
			 *(ulong_t*)trapvar->Var[3] = pVarlist[3];
			 trapvar->VarLength[3] = sizeof(ulong_t);
			 trapvar->VarType[3] = ASN_INTEGER;*/
			rc = VOS_OK;		
			break;


			/*add by shixh@20080831*/
		case trap_oltPonReceiverPowerTooLow:
		case trap_oltPonReceiverPowerTooLowClear:
		case trap_oltPonReceiverPowerTooHigh:
		case trap_oltPonReceiverPowerTooHighClear:
			if( varNum != 5 )
				break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], oltRxdeviceIdxOid, sizeof( oltRxdeviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(oltRxdeviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], oltRxboardIdxOid, sizeof( oltRxboardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(oltRxboardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], oltRxponportIdxOid, sizeof( oltRxponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(oltRxponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], oltRxonuIndexOid, sizeof( oltReceiverPowerOid ) );
			trapvar->VarOidLength[3] = sizeof(oltReceiverPowerOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[4], oltReceiverPowerOid, sizeof( oltReceiverPowerOid ) );
			trapvar->VarOidLength[4] = sizeof(oltReceiverPowerOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = pVarlist[4];
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;
			rc = VOS_OK;	
			break;

		case trap_onuNewRegSuccess:
		case trap_onuReregSuccess:
		case trap_devPowerOn:			/* modified by xieshl 20070703,这几个trap的绑定对象变化 */
		case trap_deviceColdStart:
		case trap_deviceWarmStart:
		case trap_deviceExceptionRestart:
			/*case trap_onuAutoConfigSuccess:
			  case trap_onuAutoConfigFailure:*/
			if( (varNum == 5) || (varNum == 6) )
			{
				int list_item = 0;
				ulong_t devIdx = pVarlist[list_item];
				VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
				trapvar->TrapOid[baselen] = trapId;
				trapvar->TrapOidLength = baselen+1;

				VOS_MemCpy( trapvar->VarOid[list_item], deviceIdxOid, sizeof( deviceIdxOid ) );
				trapvar->VarOidLength[list_item] = sizeof(deviceIdxOid)/sizeof(oid);
				*(ulong_t*)trapvar->Var[list_item] = pVarlist[list_item];
				trapvar->VarLength[list_item] = sizeof(ulong_t);
				trapvar->VarType[list_item] = ASN_INTEGER;

				list_item++;
				VOS_MemCpy( trapvar->VarOid[list_item], deviceTypeOid, sizeof( deviceTypeOid ) );
				trapvar->VarOidLength[list_item] = sizeof(deviceTypeOid)/sizeof(oid);
				*(ulong_t*)trapvar->Var[list_item] = pVarlist[list_item];
				trapvar->VarLength[list_item] = sizeof(ulong_t);
				trapvar->VarType[list_item] = ASN_INTEGER;

				/*VOS_MemCpy( trapvar->VarOid[2], deviceLogicCommOid, sizeof(deviceLogicCommOid) );
				  trapvar->VarOidLength[2] = sizeof( deviceLogicCommOid)/sizeof(oid);
				  getDeviceLogicalCommunity( pVarlist[0], trapvar->Var[2], &trapvar->VarLength[2] );
				  trapvar->VarType[2] = ASN_OCTET_STR;*/

				list_item++;
				VOS_MemCpy( trapvar->VarOid[list_item], deviceSoftWareVerOid, sizeof(deviceSoftWareVerOid) );
				trapvar->VarOidLength[list_item] = sizeof( deviceSoftWareVerOid)/sizeof(oid);
				getDeviceSoftwareVersion( devIdx, (char *)trapvar->Var[list_item], (ulong_t *)&trapvar->VarLength[list_item] );
				trapvar->VarType[list_item] = ASN_OCTET_STR;

				list_item++;
				VOS_MemCpy( trapvar->VarOid[list_item], deviceFirmWareVerOid, sizeof(deviceFirmWareVerOid) );
				trapvar->VarOidLength[list_item] = sizeof( deviceFirmWareVerOid)/sizeof(oid);
				getDeviceFirmwareVersion( devIdx, (char *)trapvar->Var[list_item], (ulong_t *)&trapvar->VarLength[list_item] );
				trapvar->VarType[list_item] = ASN_OCTET_STR;

				list_item++;
				VOS_MemCpy( trapvar->VarOid[list_item], deviceHardWareVerOid, sizeof(deviceHardWareVerOid) );
				trapvar->VarOidLength[list_item] = sizeof( deviceHardWareVerOid)/sizeof(oid);
				getDeviceHardwareVersion( devIdx, (char *)trapvar->Var[list_item], (ulong_t *)&trapvar->VarLength[list_item] );
				trapvar->VarType[list_item] = ASN_OCTET_STR;

				if( varNum == 6 )
				{
					list_item++;
					VOS_MemCpy( trapvar->VarOid[list_item], deviceRestartupTimeOid, sizeof(deviceRestartupTimeOid) );
					trapvar->VarOidLength[list_item] = sizeof( deviceRestartupTimeOid)/sizeof(oid);
					/*getDeviceRestartupTime( devIdx, trapvar->Var[list_item], &trapvar->VarLength[list_item] );*/
					trapvar->VarType[list_item] = ASN_OCTET_STR;
				}

				rc = VOS_OK;
			}
			break;
		case trap_sysfileUploadsuccess:/*add by shixh20090604*/
		case trap_sysfileUploadfailure:
		case trap_sysfileDownloadsuccess:
		case trap_sysfileDownloadfailure:
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
		case trap_onuAutoLoadConfigSuccess:/*add by shixh@20090218*/
		case trap_onuAutoLoadConfigFailure:
			if( varNum != 1 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;
			rc = VOS_OK;
			break;
		case trap_onuAutoLoadUpgradeSuccess:
		case trap_onuAutoLoadUpgradeFailure:
			if( varNum != 2 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], onuAutoLoadUpgradeOid, sizeof( onuAutoLoadUpgradeOid ) );
			trapvar->VarOidLength[1] = sizeof(onuAutoLoadUpgradeOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;
			rc = VOS_OK;
			break;
#endif
		case trap_onuRegAuthFailure:

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], onuAuthMacOid, sizeof(onuAuthMacOid) );
			trapvar->VarOidLength[3] = sizeof(onuAuthMacOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	
			rc = VOS_OK;	

			break;

		case trap_onuDeletingNotify:
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], onuPredefPonSlotIdxOid, sizeof( onuPredefPonSlotIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(onuPredefPonSlotIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], onuPredefPonPortIdxOid, sizeof( onuPredefPonPortIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(onuPredefPonPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], onuPredefOnuIdxOid, sizeof( onuPredefOnuIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(onuPredefOnuIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], onuPredefOnuMacAddrOid, sizeof(onuPredefOnuMacAddrOid) );
			trapvar->VarOidLength[3] = sizeof(onuPredefOnuMacAddrOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	

			VOS_MemCpy( trapvar->VarOid[4], onuPredefOnuDevIdxOid, sizeof( onuPredefOnuDevIdxOid ) );
			trapvar->VarOidLength[4] = sizeof(onuPredefOnuDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = pVarlist[4];
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;
			rc = VOS_OK;	

			break;
		case trap_ponportBRASAlarm:
		case trap_ponportBRASAlarmClear:
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], BRASmacOid, sizeof(BRASmacOid) );
			trapvar->VarOidLength[3] = sizeof(BRASmacOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	
			rc = VOS_OK;	

			break;		
			/*add by shixh20071017*/
		case trap_e1LosAlarm:			       
		case trap_e1LosAlarmClear:		
		case trap_e1LofAlarm:			
		case trap_e1LofAlarmClear:		
		case trap_e1AisAlarm:			
		case  trap_e1AisAlarmClear:		
		case trap_e1RaiAlarm:			
		case  trap_e1RaiAlarmClear:		
		case trap_e1SmfAlarm:		
		case trap_e1SmfAlarmClear:		
		case trap_e1LomfAlarm:			
		case  trap_e1LomfAlarmClear:		
		case trap_e1Crc3Alarm:		
		case  trap_e1Crc3AlarmClear:		
		case trap_e1Crc6Alarm:			
		case  trap_e1Crc6AlarmClear:	
		case  trap_E1OutOfService:
		case  trap_E1OutOfServiceClear:
			if( varNum != 3)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], e1PortDevIdexOid, sizeof( e1PortDevIdexOid ) );
			trapvar->VarOidLength[0] = sizeof(e1PortDevIdexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], e1PortBrdIndexOid, sizeof( e1PortBrdIndexOid ) );
			trapvar->VarOidLength[1] = sizeof(e1PortBrdIndexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			/*VOS_MemCpy( trapvar->VarOid[2], e1Porte1ClusterIndexOid, sizeof( e1Porte1ClusterIndexOid ) );
			  trapvar->VarOidLength[2] = sizeof(e1Porte1ClusterIndexOid)/sizeof(oid);
			 *(ulong_t*)trapvar->Var[2] = pVarlist[2];
			 trapvar->VarLength[2] = sizeof(ulong_t);
			 trapvar->VarType[2] = ASN_INTEGER;*/

			VOS_MemCpy( trapvar->VarOid[2], e1PortIndexOid, sizeof( e1PortIndexOid ) );
			trapvar->VarOidLength[2] = sizeof(e1PortIndexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	
			rc = VOS_OK;				
			break;
		case  trap_tdmServiceAbortAlarm: 
		case trap_tdmServiceAbortAlarmClear:
			if( varNum != 1 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;


			VOS_MemCpy( trapvar->VarOid[0], e1PortDevIdexOid, sizeof( e1PortDevIdexOid ) );
			trapvar->VarOidLength[0] = sizeof(e1PortDevIdexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;
			rc = VOS_OK;
			break;   	
			/*add by shixh20071017*/

			/*added by xieshl 20080116*/
		case  trap_ethLoopAlarm:
		case  trap_ethLoopAlarmClear:
			if(varNum!=3)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			rc = VOS_OK;
			break;
		case trap_onuLoopAlarm:
		case trap_onuLoopAlarmClear:
			if( varNum != 1 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;
			rc = VOS_OK;
			break;
			/* end 20080116 */
		case trap_SwitchEthPortLoop:/*add by shixh20090521*/
		case trap_SwitchEthPortLoopClear:
			if(varNum!=5)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			VOS_MemCpy( trapvar->VarOid[3], switchMacIdxOid, sizeof( switchMacIdxOid ) );	/* 问题单12797 */
			trapvar->VarOidLength[3] = sizeof(switchMacIdxOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	/* modified by xieshl 20110321, 问题单12385 */

			VOS_MemCpy( trapvar->VarOid[4], switchPortIdxOid, sizeof( switchPortIdxOid ) );
			trapvar->VarOidLength[4] = sizeof(switchPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = pVarlist[4];
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;	

			rc = VOS_OK;
			break;
		case trap_switchNewRegSuccess:
		case trap_switchReregSuccess:
		case trap_switchNotPresent:
			if(varNum != 4)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			VOS_MemCpy( trapvar->VarOid[3], switchMacIdxOid, sizeof( switchMacIdxOid ) );
			trapvar->VarOidLength[3] = sizeof(switchMacIdxOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	/* modified by xieshl 20110321, 问题单12385 */

			rc = VOS_OK;
			break;
		case trap_switchEthEgressLimitExceed:
		case trap_switchEthEgressLimitExceedClear:
		case trap_switchEthIngressLimitExceed:
		case trap_switchEthIngressLimitExceedClear:
			if(varNum != 5)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[3], switchMacIdxOid, sizeof( switchMacIdxOid ) );
			trapvar->VarOidLength[3] = sizeof(switchMacIdxOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;

			VOS_MemCpy(trapvar->VarOid[4], switchPortIdxOid, sizeof(switchPortIdxOid));
			trapvar->VarOidLength[4] = sizeof(switchPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = pVarlist[4];
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;
			rc = VOS_OK;
			break;

		case  trap_backboneEthLinkdown:  /*add by shixh@20080215*/
		case  trap_backboneEthLinkup:
		case trap_ethPortBroadCastFloodControl:/*add by shixh20090612*/
		case  trap_ethPortBroadCastFloodControlClear:
			if(varNum!=3)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			rc = VOS_OK;
			break;

		case trap_tdmToEthLinkdown:	/* added by shixh20080202 */
		case trap_tdmToEthLinkup:
			if( varNum != 3 )
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], e1PortDevIdexOid, sizeof( e1PortDevIdexOid ) );
			trapvar->VarOidLength[0] = sizeof(e1PortDevIdexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], e1PortBrdIndexOid, sizeof( e1PortBrdIndexOid ) );
			trapvar->VarOidLength[1] = sizeof(e1PortBrdIndexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], e1Porte1ClusterIndexOid, sizeof( e1Porte1ClusterIndexOid ) );
			trapvar->VarOidLength[2] = sizeof(e1Porte1ClusterIndexOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;		
			rc = VOS_OK;
			break;

#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
		case trap_userLocUpdateNotify:
			if( varNum == 9 )
			{
				user_loc_t loc;

				VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid) );
				trapvar->TrapOid[baselen] = trapId;
				trapvar->TrapOidLength = baselen+1;

				i = 0;
				if( (rc = getUserLocTrapVarBindData( pVarlist[0], (CHAR *)pVarlist[1], (CHAR*)trapvar->Var[i], &loc)) == VOS_OK )
				{
					VOS_MemCpy( trapvar->VarOid[i], userLocUserIdOid, sizeof( userLocUserIdOid ) );
					trapvar->VarOidLength[i] = macTraceOidLen;	/*sizeof(userLocUserIdOid)/sizeof(oid);*/
					trapvar->VarLength[i] = VOS_StrLen( (CHAR *)trapvar->Var[i] );
					trapvar->VarType[i] = ASN_OCTET_STR;

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof(macTraceMacAddrOid) );
					trapvar->VarOidLength[i] = macTraceOidLen;
					VOS_MemCpy( (VOID*)trapvar->Var[i], (VOID*)pVarlist[1], 6 );
					trapvar->VarLength[i] = 6;
					trapvar->VarType[i] = ASN_OCTET_STR;	

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof( macTraceMacAddrOid ) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 3;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = loc.oltBrdIdx;
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof( macTraceMacAddrOid ) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 4;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = loc.oltPortIdx;
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof( macTraceMacAddrOid ) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 5;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = MAKEDEVID(loc.oltBrdIdx, loc.oltPortIdx, loc.onuId);
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof( macTraceMacAddrOid ) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 7;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = loc.onuPortIdx;
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof( macTraceMacAddrOid ) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 8;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = swMac2Cascaded( loc.swMacAddr );
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof(macTraceMacAddrOid) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 9;
					trapvar->VarOidLength[i] = macTraceOidLen;
					VOS_MemCpy( (VOID*)trapvar->Var[i], loc.swMacAddr, 6 );
					trapvar->VarLength[i] = 6;
					trapvar->VarType[i] = ASN_OCTET_STR;	

					i++;
					VOS_MemCpy( trapvar->VarOid[i], macTraceMacAddrOid, sizeof(macTraceMacAddrOid) );
					trapvar->VarOid[i][macTraceOidLen - 1] = 10;
					trapvar->VarOidLength[i] = macTraceOidLen;
					*(ulong_t*)trapvar->Var[i] = loc.swPortIdx;
					trapvar->VarLength[i] = sizeof(ULONG);
					trapvar->VarType[i] = ASN_INTEGER;		
				}
			}
			break;
#endif

		case trap_backupPonAlarm:/*add by shixh20090521*/
		case trap_backupPonAlarmClear:
			if(varNum!=5)
				break;
			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy( trapvar->VarOid[0], deviceIdxOid, sizeof( deviceIdxOid ) );
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[1], boardIdxOid, sizeof( boardIdxOid ) );
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy( trapvar->VarOid[2], ponportIdxOid, sizeof( ponportIdxOid ) );
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;	

			VOS_MemCpy( trapvar->VarOid[3], deviceMacAddressOid, sizeof( deviceMacAddressOid ) );	/* 问题单12797 */
			trapvar->VarOidLength[3] = sizeof(deviceMacAddressOid)/sizeof(oid);
			VOS_MemCpy( (VOID*)trapvar->Var[3], (VOID*)pVarlist[3], 6 );
			trapvar->VarLength[3] = 6;
			trapvar->VarType[3] = ASN_OCTET_STR;	/* modified by xieshl 20110321, 问题单12385 */

			VOS_MemCpy( trapvar->VarOid[4], ponPortAlarmStatusOid, sizeof( ponPortAlarmStatusOid ) );
			trapvar->VarOidLength[4] = sizeof(ponPortAlarmStatusOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = 8;/*当前仅支持pon los*/
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;	

			rc = VOS_OK;
			break;
		case trap_logicalSlotInsert:
		case trap_logicalSlotPull:
			if( varNum != 4) break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy(trapvar->VarOid[0], deviceIdxOid, sizeof(deviceIdxOid));
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[1], boardIdxOid, sizeof(boardIdxOid));
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[2], logicalSlotRemoteIpOid, sizeof(logicalSlotRemoteIpOid));
			trapvar->VarOidLength[2] = sizeof(logicalSlotRemoteIpOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_IPADDRESS;

			VOS_MemCpy(trapvar->VarOid[3], logicalSlotRemotePortOid, sizeof(logicalSlotRemotePortOid));
			trapvar->VarOidLength[3] = sizeof(logicalSlotRemotePortOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;
			rc = VOS_OK;
			break;

		case trap_ponProtectSwitch:
			if( varNum != 8) break;

			VOS_MemCpy( trapvar->TrapOid, baseoid, sizeof(baseoid));
			trapvar->TrapOid[baselen] = trapId;
			trapvar->TrapOidLength = baselen+1;

			VOS_MemCpy(trapvar->VarOid[0], deviceIdxOid, sizeof(deviceIdxOid));
			trapvar->VarOidLength[0] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[0] = pVarlist[0];
			trapvar->VarLength[0] = sizeof(ulong_t);
			trapvar->VarType[0] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[1], boardIdxOid, sizeof(boardIdxOid));
			trapvar->VarOidLength[1] = sizeof(boardIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[1] = pVarlist[1];
			trapvar->VarLength[1] = sizeof(ulong_t);
			trapvar->VarType[1] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[2], ponportIdxOid, sizeof(ponportIdxOid));
			trapvar->VarOidLength[2] = sizeof(ponportIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[2] = pVarlist[2];
			trapvar->VarLength[2] = sizeof(ulong_t);
			trapvar->VarType[2] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[3], deviceIdxOid, sizeof(deviceIdxOid));
			trapvar->VarOidLength[3] = sizeof(deviceIdxOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[3] = pVarlist[3];
			trapvar->VarLength[3] = sizeof(ulong_t);
			trapvar->VarType[3] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[4], remotePhysicalSlotOid, sizeof(remotePhysicalSlotOid));
			trapvar->VarOidLength[4] = sizeof(remotePhysicalSlotOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[4] = pVarlist[4];
			trapvar->VarLength[4] = sizeof(ulong_t);
			trapvar->VarType[4] = ASN_INTEGER;

			VOS_MemCpy(trapvar->VarOid[5], remotePhysicalPortOid, sizeof(remotePhysicalPortOid));
			trapvar->VarOidLength[5] = sizeof(remotePhysicalPortOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[5] = pVarlist[5];
			trapvar->VarLength[5] = sizeof(ulong_t);
			trapvar->VarType[5] = ASN_INTEGER;


			VOS_MemCpy(trapvar->VarOid[6], logicalSlotRemoteIpOid, sizeof(logicalSlotRemoteIpOid));
			trapvar->VarOidLength[6] = sizeof(logicalSlotRemoteIpOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[6] = pVarlist[6];
			trapvar->VarLength[6] = sizeof(ulong_t);
			trapvar->VarType[6] = ASN_IPADDRESS;

			VOS_MemCpy(trapvar->VarOid[7], logicalSlotRemotePortOid, sizeof(logicalSlotRemotePortOid));
			trapvar->VarOidLength[7] = sizeof(logicalSlotRemotePortOid)/sizeof(oid);
			*(ulong_t*)trapvar->Var[7] = pVarlist[7];
			trapvar->VarLength[7] = sizeof(ulong_t);
			trapvar->VarType[7] = ASN_INTEGER;
			rc = VOS_OK;
			break;

		default:
			break;

	}
	return rc;
}

extern STATUS notifyAlmEventForAlmLevel( ulong_t trapid, ulong_t *varlist, const ulong_t varnum );
STATUS    sendPrivateTrap( ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum )
{
	SNMPTRAP trapvar;

	STATUS rc = VOS_ERROR;

	/*added by wangxy 2006-11-27告警收集*/
	/*notifyAlmEventForAlmLevel( trapId, pVarlist, varNum );*/	/* removed by xieshl 20110331 */
	
	rc = bindTrapVar( &trapvar, trapId, pVarlist, varNum );
	
	if( rc == VOS_OK )
	{
	    rc = mn_send_msg_trap( &trapvar, MSG_PRI_NORMAL );
	}
	
	return rc;
}

/*----------------------------------------------------------------------------
* 功能: 发送OLT掉电trap
* 输入参数: devIdx－OLT设备索引
* 输出参数: 
* 返回值: 正确返回OK，错误返回ERROR */
extern int mn_send_snmp_trap( SNMPTRAP * Trap );
void sendOltPowerOffTrap( ulong_t devIdx )
{
	SNMPTRAP trapvar;
	int i=0;
	/*uchar_t oltPowerOffTrapPduBuf[] = {
			0x30, 0x82, 0x00, 0x67, 0x02, 0x01, 0x01, 0x04, 0x06, 0x70,
			0x75, 0x62, 0x6c, 0x69, 0x63, 0xa7, 0x82, 0x00, 0x58, 0x02,
			0x02, 0x47, 0xe6, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00, 0x30,
			0x82, 0x00, 0x4a, 0x30, 0x82, 0x00, 0x0f, 0x06, 0x08, 0x2b,
			0x06, 0x01, 0x02, 0x01, 0x01, 0x03, 0x00, 0x43, 0x03, 0x00,
			0xcd, 0xf8, 0x30, 0x82, 0x00, 0x1b, 0x06, 0x0a, 0x2b, 0x06,
			0x01, 0x06, 0x03, 0x01, 0x01, 0x04, 0x01, 0x00, 0x06, 0x0d,
			0x2b, 0x06, 0x01, 0x04, 0x01, 0xce, 0x58, 0x02, 0x14, 0x01,
			0x01, 0x06, 0x04, 0x30, 0x82, 0x00, 0x14, 0x06, 0x0f, 0x2b,
			0x06, 0x01, 0x04, 0x01, 0xce, 0x58, 0x02, 0x14, 0x01, 0x01,
			0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01 };*/
	/*int intlock = VOS_IntLock();
	VOS_TaskLock();*/
	ulong_t varList[6] = { 0, 0, 0, 0, 0, 0 };
	varList[0] = devIdx;

	if( bindTrapVar(&trapvar, trap_devPowerOff, varList, 1) == VOS_OK )
	{
		do
		{
			mn_send_snmp_trap( &trapvar );
			/*VOS_TaskDelay(1);*/
			i++;
		} while( i < 5 );
	}
	
	/*VOS_TaskUnlock();
	VOS_IntUnlock( intlock );*/
}


int bindCmcCtrlTrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, ulong_t varNum )
{
	/* static oid mib1_trap_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 }; */
    static oid cmcControllerTrapEvent_oid[] = { 1,3,6,1,4,1,4413,2,2,2,1,19,1,99,2 };
    static oid cmcControllerEvent_oid[] = { 1,3,6,1,4,1,4413,2,2,2,1,19,1,99,1,1, 0 };
    static oid cmcMacAddress_oid[] = { 1,3,6,1,4,1,4413,2,2,2,1,19,1,99,1,2, 0 };
    static oid cmMacAddress_oid[] = { 1,3,6,1,4,1,4413,2,2,2,1,19,1,99,1,3, 0 };
	ulong_t oidLen = sizeof( cmcControllerTrapEvent_oid )/sizeof(oid);
    uchar *pucPtr;
    uchar aucCmcMac[6];
    int macLen;
    short int slot, port;
    short int pon_idx, onu_idx;

	VOS_MemZero( trapvar, sizeof( SNMPTRAP ) );
	trapvar->GenericType = SNMP_TRAP_ENTERPRISESPECIFIC;
	trapvar->SpecificType = 10;
	VOS_MemCpy( trapvar->TrapOid, cmcControllerTrapEvent_oid, sizeof( cmcControllerTrapEvent_oid ) );
	trapvar->TrapOidLength = oidLen;

    slot = (short int)pVarlist[0];
    port = (short int)pVarlist[1];
    pon_idx = GetPonPortIdxBySlot(slot, port);
    onu_idx = (short int)(pVarlist[2] - 1);
    GetOnuMacAddr(pon_idx, onu_idx, aucCmcMac, &macLen);

	VOS_MemCpy( trapvar->VarOid[0], cmcControllerEvent_oid, sizeof( cmcControllerEvent_oid ) );
    trapvar->VarOidLength[0] = sizeof( cmcControllerEvent_oid )/sizeof(oid);
    *(ulong_t*)trapvar->Var[0] = trapId;
    trapvar->VarLength[0] = sizeof(ulong_t);
    trapvar->VarType[0] = ASN_INTEGER;

	VOS_MemCpy( trapvar->VarOid[1], cmcMacAddress_oid, sizeof( cmcMacAddress_oid ) );
    trapvar->VarOidLength[1] = sizeof( cmcMacAddress_oid )/sizeof(oid);
    VOS_MemCpy( trapvar->Var[1], aucCmcMac, 6 );
    trapvar->VarLength[1] = 6;
    trapvar->VarType[1] = ASN_OCTET_STR;

    pucPtr = (uchar*)pVarlist[3];
    if ( pucPtr[0] & 1 )
    {
        varNum = 2;
    }
    else
    {
        varNum = 3;

    	VOS_MemCpy( trapvar->VarOid[2], cmMacAddress_oid, sizeof( cmMacAddress_oid ) );
        trapvar->VarOidLength[2] = sizeof( cmMacAddress_oid )/sizeof(oid);
        VOS_MemCpy( trapvar->Var[2], pucPtr, 6 );
        trapvar->VarLength[2] = 6;
        trapvar->VarType[2] = ASN_OCTET_STR;
    }
    
	trapvar->VarNum = varNum;

	return VOS_OK;
}

int sendCmcCtrlTrap( ulong_t trapid, ulong_t *pVarlist, const ulong_t varNum )
{
	SNMPTRAP mib1Trap;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		bindCmcCtrlTrapVar( &mib1Trap, trapid, pVarlist, varNum );
		return mn_send_msg_trap( &mib1Trap, MSG_PRI_NORMAL );
	}
	return VOS_OK;
}

