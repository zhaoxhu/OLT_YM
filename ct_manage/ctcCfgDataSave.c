
/*文件名:	ctcCfgDatasave.c
*Copyright (c) 2003-2008 GW Technnologies Co., LTD
*All rights reserved
*modification history
*
*modified by wangxy 2007-8-1 17:01:28
*ponSwitchCtcOnuCfgData中改为删除目的ONU的配置数据；
*添加初始化ONU各类数据的函数，完善ONU配置数据的删除函数；
*
*文件摘要:	CTC类ONU配置数据配置、恢复实现
*/

#ifdef	__cplusplus
extern "C"{
#endif

#include "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#if 0
#include "gwEponSys.h"
#include "Pongeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include	"lib_ethPortMib.h"
#include	"lib_gwEponMib.h"

#include "includefromPas.h"
#include "CT_Rman_Main.h"
#include "CT_Rman_EthPort.h"
#include "CT_Rman_VLAN.h"
#include "CT_Rman_Multicast.h"
#include	"ctcCfgDataSave.h"

#include "backup/syncMain.h"

#ifndef	PACKED
#define	PACKED	__attribute__((packed))
#endif

#define	MAX_SAVE_RESTORE_OBJ	16

enum{
CTC_OBJ_ONUHEAD=0,
CTC_OBJ_LLIDCFGDATA,
CTC_OBJ_ETHPORTCFGDATA,
CTC_OBJ_VLANCFGDATA,
CTC_OBJ_QOSCFGDATA,
CTC_OBJ_MULCASTCFGDATA,
CTC_OBJ_CFGDATAEND
};

typedef	struct{
	ULONG fileLen;
	ULONG version;
	USHORT onunum;	
}PACKED cfg_file_head_t;

typedef	struct{
	USHORT	oamDiscTime;
	CHAR	oamOui[16];
	UCHAR	oamCtcVer;
	UCHAR	encUpdKeyTime;
	USHORT	enNoReplayTimeout;
	USHORT	enTimingThrHold;
}PACKED ct_global_set_t;

typedef struct{
	USHORT	length;	
	ULONG	devIdx;
	UCHAR	mulcsw;
}PACKED ct_onu_head_t;

typedef	struct{
	UCHAR ctcFecMode;
	UCHAR ctcEncCtrl;
	UCHAR ctcDbaQueSetNum;
}PACKED	ct_llid_cfg_data_t;


typedef	struct{
	UCHAR	ctcDbaQueIdx;
	UCHAR	ctcDbaQueMap;
	USHORT	ctcDbaQueThrHold[8];
}PACKED ct_llid_dba_que_set_t;

typedef	struct{
	UCHAR	portIdx;
	UCHAR	portAdmin;
	UCHAR	anAdmin;
	UCHAR	pauseAdmin;
}PACKED ct_eth_port_t;

typedef	struct{
	UCHAR	portIdx;
	USHORT	tagTpid;
	UCHAR	tagCfi;
	UCHAR	tagPri;
	USHORT	tagVid;
	UCHAR	vlanMode;
	UCHAR	vlTransNum;
}PACKED ct_eth_port_vlan_t;

typedef	struct{
	UCHAR portidx;
	ULONG polCIR;
	ULONG polCBS;
	ULONG polEBS;
	UCHAR polEnable;
	UCHAR qossetsel;
}PACKED ct_eth_port_qos_t;

typedef	struct{
	UCHAR	qos_set_index;
	UCHAR	rulenum;
}PACKED ct_eth_port_qos_set__t;

typedef	struct{
	UCHAR	qos_rule_set_index;
	UCHAR	qos_rule_index;
	UCHAR	qos_rule_quemap;
	UCHAR	qos_rule_que_primap;
}PACKED ct_eth_port_qos_rule_t;

typedef	struct{
	UCHAR	qos_rule_set_index;
	UCHAR	qos_rule_index;	
	UCHAR	qos_rule_field_index;
	UCHAR	qos_rule_field_sel;
	UCHAR	qos_rule_field_match_val[6];
	UCHAR	qos_rule_field_operator;	
}PACKED ct_eth_port_qos_rule_field_t;

typedef struct{
/*	UCHAR ethDevIndex;
	UCHAR ethBoardIndex;*/
	UCHAR ethPorIndex;
	UCHAR ethPortMultiTagStriped;
	UCHAR ethPortMulticastGroupNum;
}PACKED eth_port_multicast_ctrl_entry;


typedef struct{
/*	UCHAR ethportMulticastDevIndex;
	UCHAR ethportMulticastBoardIndex;*/
	UCHAR ethPortMulticastIndex;
	USHORT ethportMulticastVlanVid;
}PACKED eth_port_multicast_vlan_entry;

typedef	struct{
	UCHAR	vlan_mode;
	UCHAR    reserve;
	USHORT	aggr_group_num;
}PACKED ct_eth_vlan_aggr_entry_head;
typedef	struct{
	USHORT	aggr_group_id;
	eth_ctc_vlan_aggregation_t ctcVlanAggrEntries[CTC_MAX_VLAN_AGGREGATION_TABLES];
}PACKED ct_eth_vlan_aggr_entry;

typedef	struct{
	UCHAR	vlan_mode;
	UCHAR    reserve;
	USHORT	trunk_group_num;
}PACKED ct_eth_vlan_trunk_entry_head;
typedef	struct{
	USHORT	trunk_group_id;
	USHORT ctcVlanTrunkEntries[CTC_MAX_VLAN_FILTER_ENTRIES];
}PACKED ct_eth_vlan_trunk_entry;

typedef struct{
	USHORT  reserve;
	USHORT	switch_group_num;
}__attribute__((packed))ct_eth_mvlan_switch_entry_head;
typedef struct{
	USHORT  mvlan_switch_group_id;
	USHORT multicast_vlan[CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES];
	USHORT iptv_user_vlan[CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES];
}__attribute__((packed))ct_eth_mvlan_switch_entry;


/*wysh def  ended  */
ULONG	g_ctcSaveDebFlag = 0;

/*FLASH存储的最大长度*/
const ULONG	g_flashFileLen = CTC_FLASH_FILE_LEN;

#define CTC_CFG_DATA_VER1	0x00000001
#define CTC_CFG_DATA_VER2	0x00000021
const ULONG	g_ctcSaveVer = CTC_CFG_DATA_VER2;
ULONG g_ctcRetrieveVer = 0;

FUNCPTR_CFGDATA	funcsaveptrs[MAX_SAVE_RESTORE_OBJ] = { NULL };
FUNCPTR_CFGDATA	funcrestoreptrs[MAX_SAVE_RESTORE_OBJ] = { NULL };

/*extern CT_Onu_EthPortItem_t  onuEthPort[][][];*/
extern int  MAX_ONU_NUM;
extern int qosRuleFieldValue_ValToStr_parase( unsigned char select, CTC_STACK_value_t *pMatchVal, unsigned char *pMatchStr );
extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );

#define ctc_debug_print(x) 	/*sys_console_printf x*/

/*******************************************************************************************
函数名：parseOnuIndexFromDevIdx
功能：	将设备索引转换为pon端口索引、pon上ONU索引，返回onu从0开始的编号
输入：	devIdx,设备索引；
输出：	ponIdx,ONU所在PON端口编号(0-19);   onuIdx,onu在PON上的编号（0－63）,两个参数可以同时为NULL
返回值：ONU的编号(0-19*64); -1,错误
*******************************************************************************************/
short int parseOnuIndexFromDevIdx( const ULONG devIdx, ULONG * pPonIdx, ULONG *pOnuIdx )
{
	short int ret = VOS_ERROR;
	short int ponIdx, onuIdx;
	LOCATIONDES	lct = {0};
	
	if( (OLT_DEV_ID != devIdx) && (getLocation( devIdx, &lct, CONV_YES ) == VOS_OK) )
	{
		onuIdx = lct.onuId - 1;
		ponIdx= GetPonPortIdxBySlot( lct.slot, lct.port );
		if( ponIdx == VOS_ERROR )		/* modified by xieshl 20080812 */
			return ret;
		ret = ponIdx * MAXONUPERPON + onuIdx;

		if( NULL != pPonIdx )
		{
			*pPonIdx = ponIdx;
		}
		if( NULL != pOnuIdx )
		{
			*pOnuIdx = onuIdx;
		}
	}
	return ret;
}

ULONG parseDevidxFromPonOnu( const short int ponIdx, const short int onuIdx )
{
	ULONG slot=0, port=0, ret = 0;
	if( VOS_OK == ponIndex2slot( ponIdx, &slot, &port ) )
	{
		/*ret = slot*10000+port*1000+onuIdx+1;*/
               ret=MAKEDEVID(slot,port,(onuIdx+1));
	}

	return ret;
}

ULONG parseDevidxFromOnuEntry( const int onuEntry )
{
	ULONG ret = 0;

	if( 0 <= onuEntry && onuEntry < MAXONU )
	{
		ULONG onuid = onuEntry%MAXONUPERPON;
		ULONG ponid = onuEntry/MAXONUPERPON;

		ret = parseDevidxFromPonOnu( ponid, onuid );
	}

	return ret;
}

#if 0	/* removed by xieshl 20111118 */
STATUS retrieveOnuEthPortNum(  ULONG devIdx, ULONG *num )
{
	STATUS rc = VOS_ERROR;
	ULONG val = 0;
/*	short int onuEntry = parseOnuIndexFromDevIdx( devIdx, NULL, NULL );
	if( -1 != onuEntry )
	{
		*num = OnuMgmtTable[onuEntry].GE_Ethernet_ports_number+ OnuMgmtTable[onuEntry].FE_Ethernet_ports_number ;
		rc = VOS_OK;
	}*/	/* modified by xieshl 20080827,  不能正确读出端口数 */
	if( getDeviceType(devIdx, &val) == VOS_OK )
	{
		if( val == V2R1_ONU_CTC )
		{
			rc = getDeviceCapEthPortNum( devIdx, &val );
		}
	}
	*num = val;
	return rc;
}

ULONG saveCfgFileHead( const ULONG addr, const ULONG length, const ULONG fileno, const ULONG onuc )
{
	cfg_file_head_t * pFile = ( cfg_file_head_t* )addr;

	pFile->fileLen = length;
	pFile->version = fileno;
	pFile->onunum = onuc;
	
	return sizeof(cfg_file_head_t);
}

ULONG  retrieveCfgFileHead( const ULONG addr,  ULONG *length, ULONG *fileno, ULONG *onuNum )
{
	cfg_file_head_t * pFile = ( cfg_file_head_t* )addr;

	*length = pFile->fileLen;
	*fileno = pFile->version;
	*onuNum = pFile->onunum;

	ctc_debug_print( ("retrieveCfgFileHead:onuNum=%d,fileSize=%d,ver=%d\r\n",pFile->onunum,pFile->fileLen,pFile->version) );
	
	return sizeof( cfg_file_head_t );
}

ULONG saveGlobalSet( ULONG addr )
{
	ULONG val=0;
	CHAR	buf[16]="";

	ct_global_set_t *pGlobal = ( ct_global_set_t* )addr;

	getGwEponPonCtcExtOamDiscoveryTiming( &val );
	pGlobal->oamDiscTime = val;

	getGwEponPonCtcEncrypTimingThreshold( &val );
	pGlobal->enTimingThrHold = val;

	getGwEponPonCtcEncrypUpdKeyTime( &val );
	pGlobal->encUpdKeyTime = val;

	getGwEponPonCtcExtOamCtcOui( buf, &val, 16 );
	VOS_MemCpy( pGlobal->oamOui, buf, val );

	getGwEponPonCtcExtOamCtcVer( &val  );
	pGlobal->oamCtcVer = val;

	getGwEponPonCtcEncrypNoReplyTimeout( &val );
	pGlobal->enNoReplayTimeout = val;

	
	return sizeof( ct_global_set_t );
}

ULONG retrieveGlobalSet( ULONG addr )
{
	ct_global_set_t * pGlobal = ( ct_global_set_t * )addr;

	/* modified by chenfj 2007-8-7 */
	if( pGlobal->oamDiscTime != 0 )	
		setGwEponPonCtcExtOamDiscoveryTiming( pGlobal->oamDiscTime );
	setGwEponPonCtcEncrypTimingThreshold( pGlobal->enTimingThrHold );
	if( pGlobal->encUpdKeyTime != 0)
		setGwEponPonCtcEncrypUpdKeyTime( pGlobal->encUpdKeyTime );
	/*
	setGwEponPonCtcExtOamCtcOui( pGlobal->oamOui, 16 );
	setGwEponPonCtcExtOamCtcVer( pGlobal->oamCtcVer );
	*/

	if( pGlobal->enNoReplayTimeout != 0 )
	setGwEponPonCtcEncrypNoReplyTimeout( pGlobal->enNoReplayTimeout );

	return sizeof( ct_global_set_t );
}

ULONG saveOnuHead( ULONG addr, ULONG devIdx )
{
	ct_onu_head_t *pHead = (ct_onu_head_t *)addr;
	ULONG  val = 0;

	pHead->length = 0;
	pHead->devIdx = devIdx;

	if( getEthPortMulticastSwitch( devIdx, &val ) == VOS_OK )
		pHead->mulcsw = val;
	else
		pHead->mulcsw = 1;
	
	return sizeof(ct_onu_head_t);
}

ULONG retrieveOnuHead( ULONG addr, ULONG *devIdx, ULONG *pLength )
{

	short int onuIdx = -1;
	ULONG length = 0;
	ULONG ponidx=0;
	
	ct_onu_head_t *pHead = ( ct_onu_head_t* )addr;

	*devIdx = pHead->devIdx;
	*pLength = pHead->length;

	onuIdx = parseOnuIndexFromDevIdx( pHead->devIdx, &ponidx, NULL );

	if( onuIdx != -1 )
	{
		OnuMgmtTable[onuIdx].multicastSwitch = pHead->mulcsw;
		length = sizeof( ct_onu_head_t );
	}
	else
		length = 0;
	/*setEthPortMulticastSwitch( *devIdx, pHead->mulcsw );*/

	return length;
}

ULONG saveCtcLlidCfgData( const ULONG addr, const ULONG devIdx )
{
	ct_llid_cfg_data_t *pData = (ct_llid_cfg_data_t *)addr;

	ULONG dev=0, llid=0;
	ULONG nextDev=0, nextLlid=0;

	ULONG	length = 0;

	if( getFirstLLIDEntryIndex( &dev, &llid ) == VOS_OK )
	{
		if( devIdx != dev )
		{
			while( getNextLLIDEntryIndex( dev, llid,  &nextDev, &nextLlid ) == VOS_OK )
			{
				dev = nextDev;
				llid = nextLlid;

				if( devIdx == dev )
					break;
			}			
		}

		if( devIdx == dev )
		{
			ct_llid_dba_que_set_t *pdbaque = (ct_llid_dba_que_set_t*) (addr+sizeof( ct_llid_cfg_data_t ));
			
			ULONG  quesetnum=0, val=0, onuEntry=0;
			UINT i=0;

			onuEntry = parseOnuIndexFromDevIdx( devIdx, NULL, NULL );
			
/*
			getPonPortLlidCtcFecMode( dev, llid, &fecmod );
			getPonPortLlidCtcEncrypCtrl( dev, llid, &encctrl );
			getPonPortLlidCtcDbaQueSetNum( dev, llid, &quesetnum );

			pData->ctcFecMode = fecmod;
			pData->ctcEncCtrl = encctrl;
			pData->ctcDbaQueSetNum = quesetnum;
*/
			/* modified by chenfj 2007-8-21 */
			/*pData->ctcFecMode = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcFecMode;*/
			pData->ctcFecMode = OnuMgmtTable[onuEntry].FEC_Mode;
			pData->ctcEncCtrl = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl;
			pData->ctcDbaQueSetNum = OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetNum;

			quesetnum = pData->ctcDbaQueSetNum;
			if( quesetnum < 1 )
			{
				length = sizeof( ct_llid_cfg_data_t );
				
			}
			else
			{
			
				for( i=1; i<quesetnum; i++ )
				{
					int j=0;
					getPonLlidCtcDbaReportBitmap( dev, llid, i, &val );

					pdbaque->ctcDbaQueIdx = i;
					pdbaque->ctcDbaQueMap = val;

					for( j=0; j<8; j++ )
					{
						getPonLlidCtcDbaQueueThreathold( dev, llid, i, j, &val );
						pdbaque->ctcDbaQueThrHold[j] = val;
					}
					pdbaque++;
				}

				length = sizeof( ct_llid_cfg_data_t )+(quesetnum-1)*sizeof(ct_llid_dba_que_set_t);
			}
		}
			
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcLlidCfgData:devIdx=%d,queueNum=%d,len=%d\r\n", devIdx, (ULONG)pData->ctcDbaQueSetNum, length );
	}

	return length;
	
}

ULONG retrieveCtcLlidCfgData( const ULONG addr, const ULONG devIdx )
{
	ct_llid_dba_que_set_t * pset = NULL;
	ct_llid_cfg_data_t * p = ( ct_llid_cfg_data_t * )addr;

	int i=0, j=0;
	ULONG length = 0;
	short int onuIdx = parseOnuIndexFromDevIdx( devIdx, NULL, NULL );
/*
	setPonPortLlidCtcFecMode( devIdx, 1, p->ctcFecMode );
	setPonPortLlidCtcEncrypCtrl( devIdx, 1, p->ctcEncCtrl );
	setPonPortLlidCtcDbaQueSetNum( devIdx, 1, p->ctcDbaQueSetNum );		
*/
	ctc_debug_print( ("retrieveCtcLlidCfgData:devIdx=%d\r\n", devIdx) );
	if( onuIdx >= 0 && onuIdx < MAXONU)
	{
		OnuMgmtTable[onuIdx].FEC_Mode = p->ctcFecMode;
		/*OnuMgmtTable[onuIdx].CTC_EncryptCtrl = p->ctcEncCtrl;*/
		OnuMgmtTable[onuIdx].LlidTable[0].llidCtcEncrypCtrl = p->ctcEncCtrl;
		OnuMgmtTable[onuIdx].LlidTable[0].llidCtcDbaQuesetNum = p->ctcDbaQueSetNum;

		ctc_debug_print( ("fecMode=%d,encCtrl=%d,dbaQueNum=%d\r\n", p->ctcFecMode, p->ctcEncCtrl, p->ctcDbaQueSetNum) );
	}
	else
	{
		length += sizeof( ct_llid_cfg_data_t );
		length += (p->ctcDbaQueSetNum-1)*sizeof(ct_llid_dba_que_set_t);
		sys_console_printf("retrieveCtcLlidCfgData failure:devIdx=%d\r\n", devIdx);
		return length;
	}

	length += sizeof( ct_llid_cfg_data_t );

	if( p->ctcDbaQueSetNum > 1 )
	{
		pset = ( ct_llid_dba_que_set_t * )(addr+length);
		for( i=1; i<p->ctcDbaQueSetNum; i++ )
		{
			setPonLlidCtcDbaReportBitmap( devIdx, 1, pset->ctcDbaQueIdx, pset->ctcDbaQueMap );
			for( j=0; j<8; j++ )
				setPonLlidCtcDbaQueueThreathold( devIdx, 1, pset->ctcDbaQueIdx, j, pset->ctcDbaQueThrHold[j] );

			pset++;
		}
		length += (p->ctcDbaQueSetNum-1)*sizeof(ct_llid_dba_que_set_t);
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "retrieveCtcLlidCfgData:devIdx=%d,dbaQueNum=%d,len=%d\r\n", devIdx, (ULONG)(p->ctcDbaQueSetNum-1), length );
	}

	return length;
	
}

void	initCtcOnuLlidCfgData( const ULONG devIdx )
{
	short int onuEntry = parseOnuIndexFromDevIdx( devIdx, NULL, NULL );

	if( onuEntry != -1 )
	{
		OnuMgmtTable[onuEntry].FEC_Mode = 1;
		/*OnuMgmtTable[onuEntry].CTC_EncryptCtrl = 2;*/
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcEncrypCtrl = 2;
		OnuMgmtTable[onuEntry].LlidTable[0].llidCtcDbaQuesetNum = 1;
	}
}

ULONG	saveCtcEthPortCfgData( const ULONG addr, const ULONG devIdx )
{
	ULONG length = 0, counter=0;

	UCHAR *portnum = (UCHAR*)addr;
	ct_eth_port_t * pEthPort = (ct_eth_port_t*)(addr+1);

	if( retrieveOnuEthPortNum( devIdx, &counter ) == VOS_OK )
	{
		UINT i=0;
		ULONG ponIdx=0, onuIdx=0;
		short int ret = 0;

		ret = parseOnuIndexFromDevIdx( devIdx, &ponIdx, &onuIdx );
		if( -1 == ret )
			return length;
		
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		for(  i=0; i<counter; i++ )
		{
			pEthPort->portIdx = i+1;

/*			getEthPortAdminStatus( dev, brd, eth, &val );*/
			if( (onuEthPort[ponIdx][onuIdx][i].ethPortAdminStatus != 1) && (onuEthPort[ponIdx][onuIdx][i].ethPortAdminStatus != 2 ))
				onuEthPort[ponIdx][onuIdx][i].ethPortAdminStatus = 1;
			pEthPort->portAdmin = onuEthPort[ponIdx][onuIdx][i].ethPortAdminStatus;
			/*getEthPortAnAdminStatus( dev, brd, eth, &val );*/
			pEthPort->anAdmin = onuEthPort[ponIdx][onuIdx][i].ethPortAutoNegAdminStatus;

			/*getEthPortPauseAdminStatus( dev, brd, eth, &val );*/
			pEthPort->pauseAdmin = onuEthPort[ponIdx][onuIdx][i].ethPortPauseAdminMode;

ctc_debug_print( ("%d/%d:portNo=%d,admin=%d,autoNeg=%d,pause=%d\r\n", ponIdx, onuIdx, pEthPort->portIdx, pEthPort->portAdmin, pEthPort->anAdmin, pEthPort->pauseAdmin) );
	
			pEthPort++;
		}
		VOS_SemGive( onuEthPortSemId );

	}

	length = counter * sizeof( ct_eth_port_t )+1;
	*portnum = (UCHAR)counter;

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcEthPortCfgData:devIdx=%d,portNum=%d,len=%d\r\n", devIdx, counter, length );
	}
	
	return length;
}

ULONG	retrieveCtcEthPortCfgData( const ULONG addr, const ULONG devIdx )
{
	UCHAR portNum = *(UCHAR*)addr;
	ct_eth_port_t *peth = (ct_eth_port_t*)(addr+1);
	UCHAR i=0;
	ULONG length=0;

	PON_olt_id_t oltid=0;
	PON_onu_id_t onuid = 0;
		
	if( (parse_onuidx_command_parameter( devIdx, &oltid, &onuid ) != VOS_OK) ||
					(portNum == 0) || (portNum > MAX_ONU_ETHPORT) )
	{
		sys_console_printf("retrieveCtcEthPortCfgData failure:devIdx=%d\r\n", devIdx);
	}

	ctc_debug_print( ("retrieveCtcLlidCfgData:devIdx=%d,portNum=%d\r\n", devIdx, portNum) );

	VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
	for( i=0; i<portNum; i++ )
	{
		/*setEthPortAdminStatus( devIdx, 1, peth->portIdx, peth->portAdmin );
		setEthPortAnAdminStatus( devIdx, 1, peth->portIdx, peth->anAdmin );
		setEthPortPauseAdminStatus( devIdx, 1, peth->portIdx, peth->pauseAdmin );*/

		if( (peth->portIdx != 0) && (peth->portIdx <= MAX_ONU_ETHPORT) )
		{
			onuEthPort[oltid][onuid][peth->portIdx-1].ethPortAdminStatus = peth->portAdmin;
			onuEthPort[oltid][onuid][peth->portIdx-1].ethPortAutoNegAdminStatus = peth->anAdmin;
			onuEthPort[oltid][onuid][peth->portIdx-1].ethPortPauseAdminMode = peth->pauseAdmin;

			ctc_debug_print( ("%d/%d:portNo=%d,adminState=%d,autoNeg=%d,pause=%d\r\n", oltid, onuid, peth->portIdx, peth->portAdmin, peth->anAdmin, peth->pauseAdmin) );
		}
		peth++;
	}
	VOS_SemGive( onuEthPortSemId );

	length = sizeof( ct_eth_port_t )*portNum+1;

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "retrieveCtcEthPortCfgData:devIdx=%d,portNum=%d,len=%d\r\n", devIdx, portNum, length );
	}
	
	return (length);
}



void	initCtcOnuEthPortCfgData( const ULONG devIdx )
{
	ULONG counter = 0;
   
	if( retrieveOnuEthPortNum( devIdx, &counter ) == VOS_OK )
	{
		UINT i=0;
		ULONG ponIdx=0, onuIdx=0;
		short int ret = 0;

		ret = parseOnuIndexFromDevIdx( devIdx, &ponIdx, &onuIdx );
		if( -1 == ret )
			return ;
		
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		for(  i=0; i<counter; i++ )
		{
			onuEthPort[ponIdx][onuIdx][i].ethPortAdminStatus = 1;
			onuEthPort[ponIdx][onuIdx][i].ethPortAutoNegAdminStatus = 1;
			onuEthPort[ponIdx][onuIdx][i].ethPortPauseAdminMode = 1;
		}
		VOS_SemGive( onuEthPortSemId );
	}	
}

ULONG saveCtcVlanCfgData( const ULONG addr, const ULONG devIdx )
{
	ULONG dev=0, brd=0, eth=0,   val=0, length=0, ethCounter=0;

	UCHAR *portNum = (UCHAR*)addr;
	ct_eth_port_vlan_t *pVlEntry = NULL;

	*portNum = 0;
	length++;
	
	dev=devIdx;
	brd=1;
	eth=0;

	if( retrieveOnuEthPortNum( devIdx, &ethCounter ) == VOS_OK )
	{
		int i=0;
		for( i=0; i<ethCounter; i++ )
		{
			pVlEntry = (ct_eth_port_vlan_t*)(addr+length);
			eth = i+1;
			pVlEntry->portIdx = eth;
			
			getEthPortVlanTagTpid( dev, brd, eth, &val );
			pVlEntry->tagTpid = val;

			getEthPortVlanTagCfi( dev, brd, eth, &val );
			pVlEntry->tagCfi = val;

			getEthPortVlanTagPri( dev, brd, eth, &val );
			pVlEntry->tagPri = val;

			getEthPortVlanTagVid( dev, brd, eth, &val );
			pVlEntry->tagVid = val;

			getEthPortVlanMode( dev, brd, eth, &val );
			pVlEntry->vlanMode = val;

			pVlEntry->vlTransNum = 0;

			length += sizeof( ct_eth_port_vlan_t );

			if( pVlEntry->vlanMode == CTC_VLAN_MODE_TRANSLATION )
			{
				int i = 0;			
				LogicEntity *pEntry = NULL;
				
				if( getLogicEntity( dev, &pEntry ) == VOS_OK )
				{
					short int * pMulVids = (short int*)(&(pVlEntry->vlTransNum)+1);
					pVlEntry->vlTransNum = pEntry->ethvlantransperport[eth-1];
					for( i=0; i<MAX_VLAN_TRANS_PER_PORT; i++ )
					{
						if( pEntry->ethvlantrans[eth-1][i].entryStatus == RS_ACTIVE )
						{
							*pMulVids = pEntry->ethvlantrans[eth-1][i].inVlanId;
							pMulVids++;
							*pMulVids = pEntry->ethvlantrans[eth-1][i].outVlanId;
							pMulVids++;
						}
					}

					length += sizeof(USHORT)*pVlEntry->vlTransNum*2;
				}
			}
			else if( pVlEntry->vlanMode == CTC_VLAN_MODE_AGGREGATION )
			{
				ULONG grpId = 0;
				ULONG *pSaveGrpId = (ULONG *)( pVlEntry + 1 );
				CTC_getEthPortVlanAggrGroup( devIdx, eth, &grpId );
				*pSaveGrpId = grpId;
				length += sizeof(ULONG);
			}
			else if( pVlEntry->vlanMode == CTC_VLAN_MODE_TRUNK )
			{
				ULONG grpId = 0;
				ULONG *pSaveGrpId = (ULONG *)( pVlEntry + 1 );
				CTC_getEthPortVlanTrunkGroup( devIdx, eth, &grpId );
				*pSaveGrpId = grpId;
				length += sizeof(ULONG);
			}
		}
		*portNum = ethCounter;
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcVlanCfgData:devIdx=%d,portNum=%d,len=%d\r\n", devIdx, (int)(*portNum), length );
	}

	return length;
}

ULONG retrieveCtcVlanCfgData( const ULONG addr, const ULONG devIdx )
{
	ct_eth_port_vlan_t *peth = (ct_eth_port_vlan_t *)(addr+1);
	int i=0, j=0;
	ULONG length = 0;
	UCHAR portNum = *(UCHAR*)addr;
	UCHAR vlTransNum;
	short int invid, outvid;
	short int *pMulVids;

	ctc_debug_print( ("retrieveCtcVlanCfgData:devIdx=%d\r\n",devIdx) );
	for( i=0; i<portNum; i++ )
	{
		setEthPortVlanTagTpid( devIdx, 1, peth->portIdx, peth->tagTpid );
		setEthPortVlanTagCfi( devIdx, 1, peth->portIdx, peth->tagCfi );
		setEthPortVlanTagPri( devIdx, 1, peth->portIdx, peth->tagPri );
		setEthPortVlanTagVid( devIdx, 1, peth->portIdx, peth->tagVid );
		setEthPortVlanMode( devIdx,1, peth->portIdx, peth->vlanMode );

		ctc_debug_print( (" portNo=%d,vlan-mode=%d,vid=%d\r\n", peth->portIdx, peth->vlanMode, peth->tagVid) );

		if( peth->vlanMode == CTC_VLAN_MODE_TRANSLATION )
		{
			vlTransNum = peth->vlTransNum;
			if( vlTransNum > MAX_VLAN_TRANS_PER_PORT )
				vlTransNum = MAX_VLAN_TRANS_PER_PORT;

			ctc_debug_print( (" translation:num=%d\r\n", vlTransNum) );

			pMulVids = (short int*)(peth+1);
			for( j=0; j<vlTransNum; j++ )
			{
				invid = *pMulVids;
				pMulVids++;
				outvid = *pMulVids;
				pMulVids++;
				setEthPortVlanTraStatus( devIdx, 1, peth->portIdx, invid, RS_CREATEANDWAIT );
				setEthPortVlanTranNewVid( devIdx, 1, peth->portIdx, invid, outvid );			
				ctc_debug_print( ("  invid=%d,outvid=%d\r\n", invid, outvid) );
			}

			peth = (ct_eth_port_vlan_t *)pMulVids;
			length += (sizeof( ct_eth_port_vlan_t ) + peth->vlTransNum*2*sizeof(USHORT));
		}
		if( peth->vlanMode == CTC_VLAN_MODE_AGGREGATION )
		{
			ULONG *pGrpId = (ULONG *)( peth + 1 );
			CTC_setEthPortVlanAggrGroup( devIdx, peth->portIdx, *pGrpId );

			pGrpId++;
			peth = (ct_eth_port_vlan_t *)(pGrpId);
			length += ( sizeof( ct_eth_port_vlan_t ) + sizeof(ULONG) );
		}
		else if( peth->vlanMode == CTC_VLAN_MODE_TRUNK )
		{
			ULONG *pGrpId = (ULONG *)( peth + 1 );
			CTC_setEthPortVlanTrunkGroup( devIdx, peth->portIdx, *pGrpId );

			pGrpId++;
			peth = (ct_eth_port_vlan_t *)(pGrpId);
			length += ( sizeof( ct_eth_port_vlan_t ) + sizeof(ULONG) );
		}
		else
		{
			peth++;
			length +=  sizeof(ct_eth_port_vlan_t) ;
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "retrieveCtcVlanCfgData:devIdx=%d,portNum=%d,len=%d\r\n", devIdx, portNum, length );
	}
	return length;
}

void initCtcOnuVlanCfgData( const ULONG devIdx )
{
	ULONG ethCounter = 0;
	if( retrieveOnuEthPortNum( devIdx, &ethCounter ) == VOS_OK )
	{
		UINT i = 0;
		ULONG brdIdx, portIdx;
		LogicEntity *pEntry = NULL;
		
		if( VOS_OK == getLogicEntity( devIdx, &pEntry ) )
		{
			brdIdx = 1;
			for( i = 0; i<ethCounter; i++ )
			{
				portIdx = i+1;
				setEthPortVlanTagTpid( devIdx, brdIdx, portIdx, 0 );
				setEthPortVlanTagCfi( devIdx, brdIdx, portIdx, 0 );
				setEthPortVlanTagPri( devIdx, brdIdx, portIdx, 0 );
				setEthPortVlanTagVid( devIdx, brdIdx, portIdx, 1 );
				setEthPortVlanMode( devIdx,brdIdx, portIdx, 1 );

				if( pEntry->ethvlantransperport[i] > 0 )
				{
					pEntry->ethvlantransperport[i] = 0;
					VOS_MemSet( pEntry->ethvlantrans, 0, sizeof( pEntry->ethvlantrans ) );
				}
			}
		}
	}	
}

ULONG	saveCtcQosCfgData( const ULONG addr, const ULONG devIdx )
{
	ULONG length=0, val=0, startAddr = addr;
	ULONG dev=0,brd=0,eth=0, counter = 0;
	ULONG	idxs[4]={0}, idxCounter=0;
	UCHAR	*portnum = (UCHAR*)addr;
	UCHAR	 *qosrulenum = NULL, *qosrulefieldnum = NULL;

	QOS_RULE_ENTRY * pRuleEntry = NULL;
	QOS_FIELD_ENTRY *pFieldEntry = NULL;
	ct_eth_port_qos_t * pqos = (ct_eth_port_qos_t*)(addr+1);
	ct_eth_port_qos_rule_t *pRule = NULL;
	ct_eth_port_qos_rule_field_t *pField = NULL;
	
	dev = devIdx;
	brd = 1;
	eth = 0;
	
	if( retrieveOnuEthPortNum( devIdx, &counter ) == VOS_OK )
	{

		UINT i=0;
		for( i=0; i<counter; i++ )
		{
			eth = i+1;
			pqos->portidx = eth;
			getEthPortQosSetSel( dev, brd, eth, &val );
			pqos->qossetsel = val;
			getEthPortPolcingCIR( dev, brd, eth, &val );
			pqos->polCIR = val;
			getEthPortPolcingCBS( dev, brd, eth, &val );
			pqos->polCBS = val;
			getEthPortPolcingEBS( dev, brd, eth, &val );
			pqos->polEBS = val;
			getEthPortPolcingEnable( dev, brd, eth, &val );
			pqos->polEnable = val;

			pqos++;
		}
		*portnum = counter;	
	}

	startAddr = addr+counter*sizeof(ct_eth_port_qos_t)+1;

	qosrulenum = (UCHAR*)startAddr;
	startAddr++;
	
	*qosrulenum = 0;

	idxs[0] = devIdx;

	pRule = (ct_eth_port_qos_rule_t*)startAddr;
	idxCounter =1;
	while( getNextQosRuleEntry( idxCounter, idxs, &pRuleEntry ) == VOS_OK )
	{
		idxCounter = 3;
		if( idxs[0] != devIdx )
			break;
		pRule->qos_rule_set_index = idxs[1];
		pRule->qos_rule_index = idxs[2];
		pRule->qos_rule_quemap = pRuleEntry->qos_rule_queue_mapped;
		pRule->qos_rule_que_primap = pRuleEntry->qos_rule_pri_mask;

		pRule++;
		(*qosrulenum)++;
	}

	startAddr += (*qosrulenum)*sizeof( ct_eth_port_qos_rule_t );

	qosrulefieldnum = (UCHAR*)startAddr;
	*qosrulefieldnum = 0;
	startAddr++;

	pField = (ct_eth_port_qos_rule_field_t*)startAddr;
	idxs[0] = devIdx;
	idxCounter = 1;
	while( getNextQosRuleFieldEntry( idxCounter, idxs, &pFieldEntry ) == VOS_OK )
	{
		idxCounter = 4;
		if( idxs[0] != devIdx )
			break;
		pField->qos_rule_set_index = idxs[1];
		pField->qos_rule_index = idxs[2];
		pField->qos_rule_field_index = idxs[3];

		pField->qos_rule_field_sel = pFieldEntry->qos_field_sel;
		pField->qos_rule_field_operator = pFieldEntry->qos_filed_operator;
		VOS_MemCpy( pField->qos_rule_field_match_val, pFieldEntry->qos_filed_match_val, 6 );

		pField++;
		(*qosrulefieldnum)++;
	}

	startAddr += (*qosrulefieldnum)*sizeof( ct_eth_port_qos_rule_field_t );

	length = startAddr-addr;
	
	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcQosCfgData:devIdx=%d,portNum=%d,ruleNum=%d,fieldNum=%d,len=%d\r\n", 
			devIdx, (*portnum), (*qosrulenum), (*qosrulefieldnum), length );
	}
	return length;
	
}

ULONG	retrieveCtcQosCfgData( const ULONG addr, const ULONG devIdx )
{
	UCHAR portNum = *(UCHAR*)addr;
	UCHAR ruleNum=0, fieldNum = 0;
	ct_eth_port_qos_t *pQos = (ct_eth_port_qos_t*)(addr+1);
	ct_eth_port_qos_rule_t * pRule = NULL;
	ct_eth_port_qos_rule_field_t *pField = NULL;

	QOS_RULE_ENTRY * pRuleEntry = NULL;

	ULONG oltid=0;
	ULONG onuid=0;
	
	ULONG length = 1;

	int i=0;

	
	if( -1 == parseOnuIndexFromDevIdx( devIdx, &oltid, &onuid ) )
	{
		if( g_ctcSaveDebFlag )
		{
			sys_console_printf( "retrieveCtcQosCfgData:devIdx=%d\r\n", devIdx );
			return length;
		}
	}

	for( i=0; i<portNum; i++ )
	{
		LogicEntity *pEntry = NULL;

		getLogicEntity( devIdx, &pEntry );
		setEthPortPolcingCIR( devIdx, 1, pQos->portidx, pQos->polCIR );
		setEthPortPolcingCBS( devIdx, 1, pQos->portidx, pQos->polCBS );
		setEthPortPolcingEBS( devIdx, 1, pQos->portidx, pQos->polEBS  );
		/*
		setEthPortPolcingEnable( devIdx, 1, pQos->portidx, pQos->polEnable );
		*/
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		onuEthPort[oltid][onuid][i].ethPortPolicingEnable = pQos->polEnable;
		VOS_SemGive( onuEthPortSemId );

		pEntry->qos_rule_set_select[i]= pQos->qossetsel;
		length += sizeof( ct_eth_port_qos_t );
	}

	ruleNum = *(UCHAR*)(addr+length );
	pRule = ( ct_eth_port_qos_rule_t* )( addr+length+1 );
	length++;

	for( i=0; i<ruleNum; i++ )
	{
		AddQosRuleEntry( devIdx, pRule->qos_rule_set_index, pRule->qos_rule_index );
		if( locateQosRuleEntry( devIdx, pRule->qos_rule_set_index, pRule->qos_rule_index,  &pRuleEntry ) == VOS_OK )
		{
			pRuleEntry->qos_rule_queue_mapped = pRule->qos_rule_quemap;
			pRuleEntry->qos_rule_pri_mask = pRule->qos_rule_que_primap;
		}
		length += sizeof( ct_eth_port_qos_rule_t );
		pRule++;
	}

	fieldNum = *(UCHAR*)(addr+length);
	pField = ( ct_eth_port_qos_rule_field_t* )(addr+length+1);
	length++;

	for( i=0; i<fieldNum; i++ )
	{
		if( locateQosRuleEntry( devIdx, pField->qos_rule_set_index, pField->qos_rule_index, &pRuleEntry ) == VOS_OK )
		{
			pRuleEntry->qos_rule_fields[pField->qos_rule_field_index-1].qos_field_entry_status = RS_ACTIVE;
			pRuleEntry->qos_rule_fields[pField->qos_rule_field_index-1].qos_field_sel = pField->qos_rule_field_sel;
			pRuleEntry->qos_rule_fields[pField->qos_rule_field_index-1].qos_filed_operator = pField->qos_rule_field_operator;
			VOS_MemCpy( pRuleEntry->qos_rule_fields[pField->qos_rule_field_index-1].qos_filed_match_val, pField->qos_rule_field_match_val, 6 );
			length += sizeof( ct_eth_port_qos_rule_field_t );
			pField++;
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "retrieveCtcQosCfgData:devIdx=%d,portNum=%d,ruleNum=%d,fieldNum=%d,len=%d\r\n", 
			devIdx, portNum, ruleNum, fieldNum, length );
	}
	return length;
}

/* chenfj 2009-4-8
    变量ULONG oltid, onuid 未被赋值就使用了
    parseOnuIndexFromDevIdx() 为什么被注释掉了?
    */
void	initCtcOnuQosCfgData( const ULONG devIdx )
{
	ULONG	counter=0, eth=0;

	ULONG oltid, onuid;
	
	if( (-1 != parseOnuIndexFromDevIdx( devIdx, &oltid, &onuid ) ) && (retrieveOnuEthPortNum( devIdx, &counter ) == VOS_OK ) )
	{

		UINT i=0;
		QOS_RULE_ENTRY *pRule = NULL;
		ULONG idx[4] = { 0 };
		ULONG idxCounter = 0;
		
		for( i=0; i<counter; i++ )
		{
			LogicEntity *pEntry = NULL;
			eth = i+1;
			
			getLogicEntity( devIdx, &pEntry );
			setEthPortPolcingCIR( devIdx, 1, eth,  64 );
			setEthPortPolcingCBS( devIdx, 1, eth, 0 );
			setEthPortPolcingEBS( devIdx, 1, eth, 0  );

			VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
			onuEthPort[oltid][onuid][i].ethPortPolicingEnable = 2;
			VOS_SemGive( onuEthPortSemId );
			pEntry->qos_rule_set_select[i] = 0;			
		}

		idx[0]=devIdx;
		idxCounter = 1;
		while( VOS_OK == getNextQosRuleEntry( idxCounter, idx, &pRule ) && idx[0] == devIdx )
		{
			ULONG i = 0;
			idxCounter = 3;			
			for( i =0; i<MAX_FIELDS_PER_RULE; i++ )
			{
				if( pRule->qos_rule_fields[i].qos_field_entry_status == RS_ACTIVE )
				{
					VOS_MemSet( &pRule->qos_rule_fields[i], 0, sizeof( QOS_FIELD_ENTRY) );
					pRule->qos_rule_fields[i].qos_field_entry_status = RS_NOTREADY;
				}
			}
			delQosRuleEntry( idx[0], idx[1], idx[2] );
		}		

	}	
}

/*only this funtion is def by wysh*/

ULONG ctccfgMulticastSave(const ULONG addr, const ULONG devIdx)
{
	ULONG counter = 0, length=0;
	LogicEntity *pEntry = NULL;
	UCHAR *mulvlnum = NULL;
	UCHAR * mulctrlnum =(UCHAR*)addr;
	   
	eth_port_multicast_ctrl_entry *pMulCtrl = (eth_port_multicast_ctrl_entry * )(addr+1);
	eth_port_multicast_vlan_entry *pMulcVl = NULL;

	*mulctrlnum = 0;
	length++;

	if( getLogicEntity( devIdx, &pEntry ) == VOS_ERROR )
		return length;
	   

	if( retrieveOnuEthPortNum( devIdx, &counter ) == VOS_OK )
	{	
		UINT i=0;
		for( i=0; i<counter; i++ )
		{
			pMulCtrl->ethPorIndex = i+1;
			pMulCtrl->ethPortMultiTagStriped = pEntry->ethmulcastentrys[i].mulcasttagstrip;
			pMulCtrl->ethPortMulticastGroupNum = pEntry->ethmulcastentrys[i].mulcastgroupnum;	
			length += sizeof( eth_port_multicast_ctrl_entry );

			/*lines increase*/
			(*mulctrlnum)++;
			pMulCtrl++;
		}
	}
		
	/* table 2*/
	mulvlnum = (UCHAR*)(addr+length);
	*mulvlnum = 0;
	length++;
	pMulcVl = (eth_port_multicast_vlan_entry*)( addr+length );
	if( counter >0 )
	{	 
		/*store only index of table2*/
		UINT i=0, j=0; 
		for( i=0; i<counter; i++ )
		{
			for( j=0; j<MAX_MULCAST_VLAN_PER_ETH; j++ )
			{
				if( pEntry->ethmulcastentrys[i].mulcastvlanvids[j] != 0 )
				{
					pMulcVl->ethPortMulticastIndex = i+1;
					pMulcVl->ethportMulticastVlanVid = pEntry->ethmulcastentrys[i].mulcastvlanvids[j];

					(*mulvlnum)++;
					pMulcVl++;
					length += sizeof( eth_port_multicast_vlan_entry );
				}
			}
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "ctccfgMulticastSave:devIdx=%d,ctrlNum=%d,vidNum=%d,len=%d\r\n", devIdx, (int)(*mulctrlnum), (int)(*mulvlnum), length );
	}

	return length;
}


ULONG	retrieveCtcMulticastCfgData( const ULONG addr, const ULONG devIdx )
{
	ULONG length=0;
	UCHAR mulctrlnum=0, mulvidnum=0, i=0;
	LogicEntity *pEntry = NULL;

	if( getLogicEntity( devIdx, &pEntry ) == VOS_ERROR )
		return length;

	mulctrlnum = *(UCHAR*)addr;
	length++;

	for( i=0; i<mulctrlnum; i++ )
	{
		eth_port_multicast_ctrl_entry * pMctrl = (eth_port_multicast_ctrl_entry*)(addr+length);
		pEntry->ethmulcastentrys[pMctrl->ethPorIndex-1].mulcasttagstrip = pMctrl->ethPortMultiTagStriped;
		pEntry->ethmulcastentrys[pMctrl->ethPorIndex-1].mulcastgroupnum = pMctrl->ethPortMulticastGroupNum;
		length += sizeof( eth_port_multicast_ctrl_entry );
	}

	mulvidnum = *(UCHAR*)(addr+length);
	length++;

	for( i=0; i<mulvidnum; i++ )
	{
		eth_port_multicast_vlan_entry *pMvid = ( eth_port_multicast_vlan_entry* )(addr+length);
		/*
		setEthMultiCastVlanStatus( devIdx, 1,
			pMvid->ethPortMulticastIndex, pMvid->ethportMulticastVlanVid, 4 );
			*/
		pEntry->ethmulcastentrys[pMvid->ethPortMulticastIndex-1].mulcastvlanvids[i] = pMvid->ethportMulticastVlanVid;
		length += sizeof( eth_port_multicast_vlan_entry );
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcMulticastCfgData:devIdx=%d,ctrlNum=%d,vidNum=%d,len=%d\r\n", devIdx, (int)mulctrlnum, (int)mulvidnum, length );
	}
	
	return length;
}

void	initCtcOnuMulticastCfgData( const ULONG devIdx )
{
	ULONG counter=0;
	LogicEntity	*pEntry = NULL;

	if( VOS_OK == getLogicEntity( devIdx, &pEntry ) )
	{
		if( VOS_OK == retrieveOnuEthPortNum( devIdx, &counter ) )
		{
			int i=0;
			for( i=0; i<counter; i++ )
			{
				pEntry->ethmulcastentrys[i].mulcastgroupnum = 0;
				pEntry->ethmulcastentrys[i].mulcasttagstrip = 2;
				VOS_MemSet( pEntry->ethmulcastentrys[i].mulcastvlanvids, 0, sizeof( short )*MAX_MULCAST_VLAN_PER_ETH );
			}
		}
	}
}

ULONG saveCtcVlanAggrCfgData( const ULONG addr )
{
	eth_ctc_vlan_aggr_list_t *pNode;
	ULONG length = 0;
	ULONG grpId = 0;

	ct_eth_vlan_aggr_entry_head *pEntryHead = (ct_eth_vlan_aggr_entry_head *)addr;
	ct_eth_vlan_aggr_entry *pEntry = (ct_eth_vlan_aggr_entry *)(pEntryHead + 1);

	pEntryHead->reserve = 0;
	pEntryHead->vlan_mode = CTC_VLAN_MODE_AGGREGATION;
	pEntryHead->aggr_group_num = 0;
	length += sizeof(ct_eth_vlan_aggr_entry_head);
	
	while( (pNode = CTC_findNextCtcVlanAggrNode(grpId)) != NULL )
	{
		grpId = pNode->ctcVlanAggrGroupId;

		pEntry->aggr_group_id = grpId;
		VOS_MemCpy( pEntry->ctcVlanAggrEntries, pNode->ctcVlanAggrEntries, sizeof(pEntry->ctcVlanAggrEntries) );

ctc_debug_print(("saveCtcVlanAggrCfgData:aggrid=%d\r\n", grpId) );
		pEntryHead->aggr_group_num++;
		pEntry++;
		length += sizeof(ct_eth_vlan_aggr_entry);/*sizeof(pEntry->ctcVlanAggrEntries);*/
	}
	
	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcVlanAggrCfgData:aggr_num=%d,len=%d\r\n", pEntryHead->aggr_group_num, length );
	}

	return length;
}

ULONG	retrieveCtcVlanAggrCfgData( const ULONG addr )
{
	eth_ctc_vlan_aggr_list_t *pNode;
	ULONG length = 0;
	int i;

	ct_eth_vlan_aggr_entry_head *pEntryHead = (ct_eth_vlan_aggr_entry_head *)addr;
	ct_eth_vlan_aggr_entry *pEntry = (ct_eth_vlan_aggr_entry *)(pEntryHead + 1);

	if( g_ctcRetrieveVer < CTC_CFG_DATA_VER2 )
		return length;
	
	length += sizeof(ct_eth_vlan_aggr_entry_head);

	if( (pEntryHead->vlan_mode != CTC_VLAN_MODE_AGGREGATION) || (pEntryHead->aggr_group_num == 0) || (pEntryHead->aggr_group_num > CTC_VLAN_AGGR_ID_MAX) )
	{
	}
	else
	{
		for( i=0; i<pEntryHead->aggr_group_num; i++ )
		{
			if( pEntry == NULL )
			{
				VOS_ASSERT(0);
				break;
			}
ctc_debug_print( ("retrieveCtcVlanAggrCfgData:aggrid=") );

			pNode = CTC_addCtcVlanAggrNode( pEntry->aggr_group_id );
			if( pNode == NULL )
				break;
			VOS_MemCpy( pNode->ctcVlanAggrEntries, pEntry->ctcVlanAggrEntries, sizeof(pNode->ctcVlanAggrEntries) );
ctc_debug_print( ("%d\r\n", pEntry->aggr_group_id) );

			pEntry++;

			length += sizeof(ct_eth_vlan_aggr_entry);/*sizeof(pEntry->ctcVlanAggrEntries);*/
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcVlanAggrCfgData:aggr_num=%d,len=%d\r\n", pEntryHead->aggr_group_num, length );
	}
	
	return length;
}

ULONG saveCtcVlanTrunkCfgData( const ULONG addr )
{
	eth_ctc_vlan_trunk_list_t *pNode;
	ULONG length = 0;
	ULONG grpId = 0;

	ct_eth_vlan_trunk_entry_head *pEntryHead = (ct_eth_vlan_trunk_entry_head *)addr;
	ct_eth_vlan_trunk_entry *pEntry = (ct_eth_vlan_trunk_entry *)(pEntryHead + 1);

	pEntryHead->reserve = 0;
	pEntryHead->vlan_mode = CTC_VLAN_MODE_TRUNK;
	pEntryHead->trunk_group_num = 0;
	length += sizeof(ct_eth_vlan_trunk_entry_head);
	
	while( (pNode = CTC_findNextCtcVlanTrunkNode(grpId)) != NULL )
	{
		grpId = pNode->ctcVlanTrunkGroupId;

		pEntry->trunk_group_id = grpId;
		VOS_MemCpy( pEntry->ctcVlanTrunkEntries, pNode->ctcVlanTrunkEntries, sizeof(pEntry->ctcVlanTrunkEntries) );

ctc_debug_print(("saveCtcVlanTrunkCfgData:trunkid=%d\r\n", grpId) );
		pEntryHead->trunk_group_num++;
		pEntry++;
		length += sizeof(ct_eth_vlan_trunk_entry);/*sizeof(pEntry->ctcVlanTrunkEntries);*/
	}
	
	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcVlanTrunkCfgData:trunk_group_num=%d,len=%d\r\n", pEntryHead->trunk_group_num, length );
	}

	return length;
}

ULONG retrieveCtcVlanTrunkCfgData( const ULONG addr )
{
	eth_ctc_vlan_trunk_list_t *pNode;
	ULONG length = 0;
	int i;

	ct_eth_vlan_trunk_entry_head *pEntryHead = (ct_eth_vlan_trunk_entry_head *)addr;
	ct_eth_vlan_trunk_entry *pEntry = (ct_eth_vlan_trunk_entry *)(pEntryHead + 1);

	if( g_ctcRetrieveVer < CTC_CFG_DATA_VER2 )
		return length;
	
	length += sizeof(ct_eth_vlan_trunk_entry_head);

	if( (pEntryHead->vlan_mode != CTC_VLAN_MODE_TRUNK) || (pEntryHead->trunk_group_num == 0) || (pEntryHead->trunk_group_num > CTC_VLAN_TRUNK_ID_MAX) )
	{
	}
	else
	{
		for( i=0; i<pEntryHead->trunk_group_num; i++ )
		{
			if( pEntry == NULL )
			{
				VOS_ASSERT(0);
				break;
			}
ctc_debug_print( ("retrieveCtcVlanTrunkCfgData:trunkid=") );

			pNode = CTC_addCtcVlanTrunkNode( pEntry->trunk_group_id );
			if( pNode == NULL )
				break;
			VOS_MemCpy( pNode->ctcVlanTrunkEntries, pEntry->ctcVlanTrunkEntries, sizeof(pNode->ctcVlanTrunkEntries) );
ctc_debug_print( ("%d\r\n", pEntry->trunk_group_id) );

			pEntry++;

			length += sizeof(ct_eth_vlan_trunk_entry);/*sizeof(pEntry->ctcVlanTrunkEntries);*/
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcVlanTrunkCfgData:trunk_num=%d,len=%d\r\n", pEntryHead->trunk_group_num, length );
	}
	
	return length;
}

ULONG saveCtcMVlanSwitchCfgData( const ULONG addr )
{
	eth_ctc_multicast_vlan_switch_list_t *pNode;
	ULONG length = 0;
	ULONG grpId = 0;

	ct_eth_mvlan_switch_entry_head *pEntryHead = (ct_eth_mvlan_switch_entry_head *)addr;
	ct_eth_mvlan_switch_entry *pEntry = (ct_eth_mvlan_switch_entry *)(pEntryHead + 1);

	pEntryHead->reserve = 0;
	pEntryHead->switch_group_num = 0;
	length += sizeof(ct_eth_mvlan_switch_entry_head);
	
	while( (pNode = CTC_findNextCtcMVlanSwitchNode(grpId)) != NULL )
	{
		grpId = pNode->ctcMVlanSwitchGroupId;

		pEntry->mvlan_switch_group_id = grpId;
		VOS_MemCpy( pEntry->multicast_vlan, pNode->multicast_vlan, sizeof(pEntry->multicast_vlan) );
		VOS_MemCpy( pEntry->iptv_user_vlan, pNode->iptv_user_vlan, sizeof(pEntry->iptv_user_vlan) );

ctc_debug_print(("saveCtcMVlanSwitchCfgData:tvmid=%d\r\n", grpId) );
		pEntryHead->switch_group_num++;
		pEntry++;
		length += sizeof(ct_eth_mvlan_switch_entry);
	}
	
	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcMVlanSwitchCfgData:mvlan_switch_group_num=%d,len=%d\r\n", pEntryHead->switch_group_num, length );
	}

	return length;
}

ULONG	retrieveCtcMVlanSwitchCfgData( const ULONG addr )
{
	eth_ctc_multicast_vlan_switch_list_t *pNode;
	ULONG length = 0;
	int i;

	ct_eth_mvlan_switch_entry_head *pEntryHead = (ct_eth_mvlan_switch_entry_head *)addr;
	ct_eth_mvlan_switch_entry *pEntry = (ct_eth_mvlan_switch_entry *)(pEntryHead + 1);

	if( g_ctcRetrieveVer < CTC_CFG_DATA_VER2 )
		return length;
	
	length += sizeof(ct_eth_mvlan_switch_entry_head);

	if( (pEntryHead->switch_group_num == 0) || (pEntryHead->switch_group_num > CTC_MULTICAST_VLAN_SWITCH_ID_MAX) )
	{
	}
	else
	{
		for( i=0; i<pEntryHead->switch_group_num; i++ )
		{
			if( pEntry == NULL )
			{
				VOS_ASSERT(0);
				break;
			}
ctc_debug_print( ("retrieveCtcMVlanSwitchCfgData:grpid=") );

			pNode = CTC_addCtcMVlanSwitchNode( pEntry->mvlan_switch_group_id );
			if( pNode == NULL )
				break;
			VOS_MemCpy( pNode->multicast_vlan, pEntry->multicast_vlan, sizeof(pNode->multicast_vlan) );
			VOS_MemCpy( pNode->iptv_user_vlan, pEntry->iptv_user_vlan, sizeof(pNode->iptv_user_vlan) );
ctc_debug_print( ("%d\r\n", pEntry->mvlan_switch_group_id) );

			pEntry++;

			length += sizeof(ct_eth_mvlan_switch_entry);
		}
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf("retrieveCtcMVlanSwitchCfgData:mvlan_switch_group_num=%d,len=%d\r\n", pEntryHead->switch_group_num, length );
	}
	
	return length;
}

extern ULONG saveCtcAuthModeCfgData( const ULONG addr );
extern ULONG retrieveCtcAuthModeCfgData( const ULONG addr );
extern ULONG saveCtcAuthLoidCfgData( const ULONG addr );
extern ULONG retrieveCtcAuthLoidCfgData( const ULONG addr );

void	initOperateFunctions( void )
{
	funcsaveptrs[0] = saveOnuHead;

	funcsaveptrs[1] = saveCtcLlidCfgData;
	funcrestoreptrs[1] = retrieveCtcLlidCfgData;

	funcsaveptrs[2] = saveCtcEthPortCfgData;
	funcrestoreptrs[2] = retrieveCtcEthPortCfgData;

	funcsaveptrs[3] = saveCtcVlanCfgData;
	funcrestoreptrs[3] = retrieveCtcVlanCfgData;

	funcsaveptrs[4] = saveCtcQosCfgData;
	funcrestoreptrs[4] = retrieveCtcQosCfgData;

	funcsaveptrs[5] = ctccfgMulticastSave;
	funcrestoreptrs[5] = retrieveCtcMulticastCfgData;

}

ULONG saveCtcOneOnuCfgData( ULONG addr, ULONG devIdx )
{
	int i=0; 

	ULONG	onuIdx=devIdx, length=0,offset=0;

	ct_onu_head_t * const pHead = (ct_onu_head_t*)addr;
			
			
	for( i=0; i<(int)CTC_OBJ_CFGDATAEND; i++ )
	{
		if( funcsaveptrs[i] == NULL )
			break;
		/*if( g_ctcSaveDebFlag )
			reportd( "save data offset:", offset );*/
		length = funcsaveptrs[i]( addr+offset, onuIdx );
		offset+=length;
	}

	if( i != CTC_OBJ_CFGDATAEND )
		offset = 0;
	else
		pHead->length = offset;
		pHead->devIdx=devIdx;/*add by shixh20100107*/
	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "saveCtcOneOnuCfgData:devIdx=%d, len=%d\r\n", devIdx, pHead->length );
	}

	return offset;
	
}

ULONG retrieveCtcOneOnuCfgData( ULONG addr, ULONG newDev )
{
	ULONG devIdx=0, length=0, offset=0, len=0;

	LOCATIONDES lct = { 0 };

	int i=0;

	len = retrieveOnuHead( addr, &devIdx, &length );
	offset += len;

	if( VOS_OK == getLocation( newDev, &lct, CONV_NO ) )
		devIdx = newDev;
	
	else if( VOS_OK == getLocation( devIdx, &lct, CONV_NO ) )
	{
	
		for( i=1; i<CTC_OBJ_CFGDATAEND; i++ )
		{
			/*if( g_ctcSaveDebFlag )
				reportd( "retrieve data offset:", offset );*/
			len = funcrestoreptrs[i]( addr+offset, devIdx );		
			offset += len;
		}
	}
	else
	{
		sys_console_printf( "retrieveCtcOneOnuCfgData:failure, devIdx=%d\r\n", devIdx );
	}

	if( g_ctcSaveDebFlag )
	{
		sys_console_printf( "retrieveCtcOneOnuCfgData:devIdx=%d,len=%d\r\n", devIdx, offset );
	}
	return length;
}

int	retrieveCtcOneOnuAction( const ULONG devIdx )
{
	int portnum = 0, i=0;

	PON_olt_id_t oltid = 0;
	PON_onu_id_t onuid = 0;

	UCHAR adminStatus, autoNeg, pause, policEbl;

	LogicEntity *pEntry = NULL;
	ULONG pont=0,onut=0;/*add by shixh@20080908*/
	short int onuIdx = parseOnuIndexFromDevIdx( devIdx, &pont, &onut );
return VOS_OK;	/* modified by xieshl 20101217, 暂不支持CTC ONU配置数据恢复 */

	getLogicEntity( devIdx, &pEntry );

	if( onuIdx == -1 )
		return (-1);
	
	if( parse_onuidx_command_parameter( devIdx, &oltid, &onuid ) == VOS_ERROR )
		return (-1);
	
	portnum = OnuMgmtTable[onuIdx].GE_Ethernet_ports_number+OnuMgmtTable[onuIdx].FE_Ethernet_ports_number;
	
	/*onuHead*/
	setEthPortMulticastSwitch( devIdx, OnuMgmtTable[onuIdx].multicastSwitch );

	/*llid cfg data*/
	setPonPortLlidCtcFecMode( devIdx, 1, OnuMgmtTable[onuIdx].FEC_Mode);
	/*setPonPortLlidCtcEncrypCtrl( devIdx, 1, OnuMgmtTable[onuIdx].CTC_EncryptCtrl); */
	setPonPortLlidCtcEncrypCtrl( devIdx, 1, OnuMgmtTable[onuIdx].LlidTable[0].llidCtcEncrypCtrl);
	setPonPortLlidCtcDbaQueSetNum( devIdx, 1, OnuMgmtTable[onuIdx].LlidTable[0].llidCtcDbaQuesetNum );	
	setPonPortLlidCtcDbaQueSetCfgStatus( devIdx, 1, 3 );

	/*onu ethport group*/
	for( i=0; i<portnum; i++ )
	{
		VOS_SemTake( onuEthPortSemId, WAIT_FOREVER );
		adminStatus = onuEthPort[oltid][onuid][i].ethPortAdminStatus;
		autoNeg = onuEthPort[oltid][onuid][i].ethPortAutoNegAdminStatus;
		pause = onuEthPort[oltid][onuid][i].ethPortPauseAdminMode;
		policEbl = onuEthPort[oltid][onuid][i].ethPortPolicingEnable;
		VOS_SemGive( onuEthPortSemId );
		
		setEthPortAdminStatus( devIdx, 1, i+1,  adminStatus );
		setEthPortAnAdminStatus( devIdx, 1, i+1, autoNeg );
		setEthPortPauseAdminStatus( devIdx, 1, i+1, pause );
		
		/*onu vlan cfg data*/
		setEthPortVlanAction( devIdx, 1, i+1, 3 );

		/*onu qos cfg data*/
		setEthPortPolcingEnable( devIdx, 1, i+1, policEbl );	

		/*onu qos set sel*/
		setEthPortQosSetSel(  devIdx, 1, i+1, pEntry->qos_rule_set_select[i] );

		/*onu multicast ctrl*/
		setEthMultiTagStriped( devIdx, 1, i+1, pEntry->ethmulcastentrys[i].mulcasttagstrip);
		setEthMultiGroupNum( devIdx, 1, i+1, pEntry->ethmulcastentrys[i].mulcastgroupnum);

		/*onu multicast vids*/
		setEthMultiCastAction( devIdx, 1, i+1, 3 );
	}

	
	return (0);
}

void	saveCtcCfgData( void )
{
	UCHAR * pSave =  (UCHAR*)g_malloc( g_flashFileLen + 0x10000 );	/* modified by xieshl 20080827, 因CTC配置数据预留空间不足，需要防止溢出 */

	/*if( g_ctcSaveDebFlag )
		reportlx( "saveCtc alloc memory address:", pSave );*/

	if( pSave != NULL )
	{
		ULONG	onuIdx=0, length=0,offset=0, onuCounter=0;
		LOCATIONDES	lct = { 0 };

		offset = sizeof( cfg_file_head_t ) + sizeof( ct_global_set_t );

		
		onuIdx = findFirstOnu( CTC_ONU );		
		while( onuIdx != 0 )
		{			
			length = saveCtcOneOnuCfgData( (ULONG)pSave+offset, onuIdx );
			offset += length;

			
			if( length != 0 )
				onuCounter++;
			
			if( getLocation( onuIdx, &lct, CONV_NO ) == VOS_OK )
				onuIdx =  findNextOnu( lct.slot, lct.port, lct.onuId, CTC_ONU );
			else
				break;

			if( offset >= g_flashFileLen )
			{
				sys_console_printf("ONU config file is too long (cur=%d,max=%d)\r\n", offset, g_flashFileLen);
				break;
			}
		}

		length = saveCtcVlanAggrCfgData( (ULONG)pSave+offset );
		offset += length;
		length = saveCtcVlanTrunkCfgData( (ULONG)pSave+offset );
		offset += length;

		length = saveCtcMVlanSwitchCfgData( (ULONG)pSave+offset );
		offset += length;

		length = saveCtcAuthModeCfgData( (ULONG)pSave+offset );
		offset += length;
		length = saveCtcAuthLoidCfgData( (ULONG)pSave+offset );
		offset += length;
		
		length = saveGlobalSet( (ULONG)pSave+sizeof( cfg_file_head_t ) );
		length = saveCfgFileHead( (ULONG)pSave, offset, g_ctcSaveVer, onuCounter );

		if( g_ctcSaveDebFlag )
		{
			sys_console_printf( "saveCtcCfgData: fileLen=%d,onuNum=%d\r\n", offset, onuCounter );
		}

		V2R1_disable_watchdog();
		xflash_file_write( CTC_FLASH_FILE_ID, pSave, &offset );
		V2R1_enable_watchdog();

		free( pSave );
		pSave = NULL;
	}
	
}

void retrieveCtcCfgData( void )
{

	ULONG offset=0, len=0, length=0, ver=0, onuNum=0;
	UCHAR	*pData = (UCHAR*)g_malloc( g_flashFileLen );

	if( pData != NULL )
	{
		int readlen = g_flashFileLen;

		V2R1_disable_watchdog();
		xflash_file_read( CTC_FLASH_FILE_ID, pData, &readlen );
		V2R1_enable_watchdog();

		len = retrieveCfgFileHead( (ULONG)pData, &length, &ver, &onuNum );
		g_ctcRetrieveVer = ver;

		if( g_ctcSaveDebFlag )
		{
			sys_console_printf( "retrieveCtcCfgData:fileLen=%d,onuNum=%d\r\n", length, onuNum );
		}

		if( (length > 0) && (( onuNum >= 0) && (onuNum <= MAX_ONU_NUM )) /*&& (ver == g_ctcSaveVer)*/ )
		{
			UINT i=0;
			offset += len;

			len = retrieveGlobalSet( (ULONG)(pData+offset) );
			offset += len;

			for( i=0; i<onuNum; i++ )
			{
				len = retrieveCtcOneOnuCfgData( (ULONG)(pData+offset), 0 );
				offset += len;
			}

			if( g_ctcRetrieveVer >= CTC_CFG_DATA_VER2 )
			{
				len = retrieveCtcVlanAggrCfgData( (ULONG)(pData+offset) );
				offset += len;

				len = retrieveCtcVlanTrunkCfgData( (ULONG)(pData+offset) );
				offset += len;

				len = retrieveCtcMVlanSwitchCfgData( (ULONG)(pData+offset) );
				offset += len;

				len = retrieveCtcAuthModeCfgData( (ULONG)(pData+offset) );
				offset += len;
				len = retrieveCtcAuthLoidCfgData( (ULONG)(pData+offset) );
				offset += len;

			}
		}
		
		free( pData );
	}
	
}

void	deleteCtcRunningCfgData( const ULONG devIdx )
{
	initCtcOnuLlidCfgData( devIdx );
	initCtcOnuEthPortCfgData( devIdx );
	initCtcOnuVlanCfgData( devIdx );
	initCtcOnuQosCfgData( devIdx );
	initCtcOnuMulticastCfgData( devIdx );
}

void ponSwitchCtcOnuCfgData( const short int srcPon, const short int desPon )
{
	if( srcPon >=0 && srcPon < MAXPON && desPon >= 0 && desPon < MAXPON )
	{
		ULONG slot=0, pon=0/*, desSlot = 0*/, desPon = 0;
		if( VOS_OK == ponIndex2slot( srcPon, &slot, &pon ) )
		{
			ULONG onuidx = 0;
			void *pBuf = VOS_Malloc( 0x20000, MODULE_RPU_PON );

			if( NULL != pBuf )
			{
				onuidx = findNextOnu( slot, pon, 0, CTC_ONU );
				while( 0 != onuidx )
				{
					LOCATIONDES lct = {0};
					if( VOS_OK == getLocation( onuidx , &lct, CONV_NO ) )
					{
						ULONG desOnuIdx = 0, length = 0;
						if( !((slot == lct.slot) && ( pon == lct.port ) ) )
							break;

						length = saveCtcOneOnuCfgData( (ULONG)pBuf, onuidx );

						desOnuIdx = parseDevidxFromPonOnu( desPon, lct.onuId-1 );

						deleteCtcRunningCfgData( desOnuIdx );
						
						length = retrieveCtcOneOnuCfgData( (ULONG)pBuf, desOnuIdx );
						
						onuidx = findNextOnu( lct.slot, lct.port, lct.onuId, CTC_ONU );
						
					}
					else
						break;
				}
				VOS_Free( pBuf );
				pBuf = NULL;
			}
		}
	}
}

#ifdef	PRINTCFGDATA
#endif
ULONG printfilehead(ULONG addr , struct vty * vty)
{	
	cfg_file_head_t * pfilehead = (cfg_file_head_t *)addr ;

	vty_out(vty, "\r\n ctc onu config-file:ver=%02x,size=%d,onu-num=%d\r\n", pfilehead->version, pfilehead->fileLen, pfilehead->onunum );

	return sizeof(cfg_file_head_t) ;
}

ULONG printglobalset(ULONG addr ,struct vty * vty)
{
	ct_global_set_t * pglobalset = (ct_global_set_t *)addr ;

	vty_out(vty, " oamDiscTime=%u, oamOui=%02x%02x%02x, oamCtcVer=%02x\r\n", pglobalset->oamDiscTime,
			pglobalset->oamOui[0], pglobalset->oamOui[1], pglobalset->oamOui[2], pglobalset->oamCtcVer );
	vty_out(vty, " encUpdKeyTime=%u, enNoReplayTimeout=%u, enTimingThrHold=%u\r\n", pglobalset->encUpdKeyTime , pglobalset->enNoReplayTimeout , pglobalset->enTimingThrHold);

	return sizeof( ct_global_set_t ) ;
}


ULONG printonuhead(ULONG addr , struct vty * vty)
{
	ct_onu_head_t * ponuhead = (ct_onu_head_t *)addr ;

	vty_out(vty, "\r\n onu-dev=%d,size=%d\r\n", ponuhead->devIdx, ponuhead->length );
	vty_out( vty, "----------------\r\n" );
	vty_out(vty, " mulcsw=%d\r\n", ponuhead->mulcsw );
	
	return sizeof(ct_onu_head_t) ;
}


ULONG printctcLlidcfgdata(ULONG addr , struct vty * vty)
{
	ULONG length = 0 ;  
	int i, j;

	ct_llid_cfg_data_t * plid = (ct_llid_cfg_data_t *)addr ;
	ct_llid_dba_que_set_t * pset =NULL ; 

	vty_out(vty, " ctcFecMode=%d, ctcEncCtrl=%d\r\n", plid->ctcFecMode, plid->ctcEncCtrl ) ;
	vty_out(vty, " ctcDbaQueSetNum=%d\r\n", plid->ctcDbaQueSetNum ) ;

	length = sizeof( ct_llid_cfg_data_t ) ;
	pset = ( ct_llid_dba_que_set_t * )(addr+length) ;
	
	/*判断ctcDbaQueSetNum的值*/
	if( plid->ctcDbaQueSetNum > 1)
	{	
		for( i=1; i < plid->ctcDbaQueSetNum; i++ )
		{
			vty_out(vty, " dbaQueIdx  dbaQueMap  dbaQueThrHold\r\n");
			vty_out(vty, "   %-12d%-12d" , pset->ctcDbaQueIdx , pset->ctcDbaQueMap) ;
		
			for( j=0; j<8; j++ )
			{
				if( j == 0 )
					vty_out( vty, "%-5d\r\n",  pset->ctcDbaQueThrHold[j] );
				else
					vty_out( vty, "%-27s%-5d\r\n", "", pset->ctcDbaQueThrHold[j] );
			}
		}	

		length = sizeof( ct_llid_cfg_data_t )+(plid->ctcDbaQueSetNum -1)*sizeof(ct_llid_dba_que_set_t);
	}
	return length ;
}

ULONG printctcethportcfgdata(ULONG addr , struct vty * vty)
{
	ULONG length = 1 ;
	UCHAR * portnum = (UCHAR *)addr ; 
	int  i ;
	char *adminStr[] = { "-", "up", "down", "testing" };
	char *anStr[] = { "-", "enable", "disable" };
	char *pauseStr[] = { "-", "disable", "enabledXmit", "enabledRcv", "enabledXmitAndRcv" };

	ct_eth_port_t * pethport = (ct_eth_port_t *)(addr + length) ;

	vty_out(vty, "\r\n eth-port num=%d\r\n", *portnum) ;
		
	vty_out(vty, " portNo  AdminState  antoNeg   pause\r\n") ;
		
	for(i = 0 ; i < *portnum ; i++)   /*设备索引和端口号的区别*/
	{
		if(  (int)pethport->portAdmin > 3 )
			pethport->portAdmin = 0;
		if( pethport->anAdmin > 2 )
			pethport->anAdmin = 0;
		if( pethport->pauseAdmin > 4 )
			pethport->pauseAdmin = 0;
		
		vty_out(vty, "   %-7d%-12s%-10s%-20s\r\n" , pethport->portIdx, 
				adminStr[pethport->portAdmin],
				anStr[pethport->anAdmin],
				pauseStr[pethport->pauseAdmin] );
		
		pethport++;
	}

	length = (*portnum)*sizeof(ct_eth_port_t) +1 ;

	return length ;
}


ULONG printctcvlancfgdata(ULONG addr , struct vty * vty)
{	
	ULONG length = 1;
	
	UCHAR *portNum = (UCHAR*)addr;
	int i, k;
	char *vlanModeStr[] = { "transparent", "tag", "translation", "aggregation", "trunk" };

	ct_eth_port_vlan_t *pVlEntry = NULL ;

	vty_out( vty,"\r\n vlan  portNum=%d\r\n", *portNum ) ;

	pVlEntry = (ct_eth_port_vlan_t *)(addr + length) ;

	vty_out(vty, " portNo  Tpid  Pri  Vid   transNum  Cfi   vlanMode   invid/aggr  outvid\r\n"); 
	
	for(i = 0 ; i < * portNum ; i++)
	{
		if( pVlEntry ->vlanMode > 4 )
			pVlEntry ->vlanMode = 0;
		vty_out(vty, "   %-7d%-6x%-5d%-6d%-10d%-5d%-12s" , 
			pVlEntry->portIdx , pVlEntry->tagTpid , pVlEntry->tagPri , pVlEntry->tagVid , pVlEntry->vlTransNum, 
			pVlEntry->tagCfi, vlanModeStr[pVlEntry ->vlanMode] );
		
		if( pVlEntry->vlanMode == CTC_VLAN_MODE_TRANSLATION )   /* 条件不满足没有测试过输出*/
		{
			USHORT * pMulVids = NULL ;
			UCHAR	vlTransNum = pVlEntry->vlTransNum;    /*pointer address has been confussed*/

			pVlEntry++;
			pMulVids= (USHORT *)pVlEntry ;

			if( vlTransNum > MAX_VLAN_TRANS_PER_PORT )
				vlTransNum = MAX_VLAN_TRANS_PER_PORT;
			if( vlTransNum )
			{
				for(k = 0 ; k < vlTransNum ; k++ )
				{	
					if( k )
						vty_out( vty, "%-59s", "" );
					vty_out( vty, "%-12d%-7d\r\n", *pMulVids, *(pMulVids+1) );

					pMulVids += 2 ;
				}
			}
			else
				vty_out( vty, "\r\n" );

			pVlEntry = (ct_eth_port_vlan_t *)pMulVids ;
			length += (sizeof( ct_eth_port_vlan_t ) + sizeof(USHORT) * vlTransNum * 2);
		}
		else if( pVlEntry->vlanMode == CTC_VLAN_MODE_AGGREGATION )
		{
			ULONG *pAggrId = (ULONG *)(pVlEntry + 1);

			vty_out( vty, "  %-12d\r\n", *pAggrId );
			length += (sizeof( ct_eth_port_vlan_t ) + sizeof(ULONG) );

			pVlEntry = (ct_eth_port_vlan_t *)(pAggrId+1);
		}
		else
		{
			vty_out( vty, "\r\n" );
			
			pVlEntry++;
			length +=  sizeof(ct_eth_port_vlan_t) ;
		}
	}
	
	return length ;
}

ULONG printctcqoscfgdata(ULONG addr , struct vty * vty)
{
	ULONG length = 1 ;

	UCHAR * portnum = (UCHAR *)addr ;
	UCHAR * rulenum = NULL;
	UCHAR * fieldnum = NULL;
	int  i;
	char *enStr[] = {"-", "enable", "disable" };
		
	ct_eth_port_qos_t * pqos = (ct_eth_port_qos_t *)(addr + length) ;
	ct_eth_port_qos_rule_t * prule = NULL ;
	ct_eth_port_qos_rule_field_t * pfield = NULL ;

	vty_out(vty,"\r\n qos port policing\r\n" ) ;

	vty_out(vty, " portNo   CIR    CBS     EBS    select   state\r\n" );
	for(i = 0; i< *portnum; i++)
	{
		if( pqos->polEnable > 2 )
			pqos->polEnable = 0;
		vty_out(vty, "   %-6d%-8d%-8d%-8d%-8d%-8s\r\n", pqos->portidx, pqos->polCIR, pqos->polCBS, pqos->polEBS, pqos->qossetsel,
				enStr[pqos->polEnable] );
		
		pqos++;
	}
	
	length += (*portnum)*sizeof(ct_eth_port_qos_t) ;
	
	rulenum = (UCHAR *)(addr + length ) ;
	prule = (ct_eth_port_qos_rule_t *)(addr + length + 1) ; 
	length ++;    
	
	if( *rulenum > 0 )
	{
		vty_out(vty, "\r\n qos-rule  rule-num=%d\r\n" ,*rulenum) ;
		vty_out(vty, " rule_set   rule   que-map   pri-map\r\n") ;
		for( i=0; i< *rulenum; i++ )
		{
			if( prule == NULL )
				break;
			vty_out(vty, "  %-10d%-7d%-10d%-8d\r\n" , prule->qos_rule_set_index , prule->qos_rule_index , prule->qos_rule_quemap , prule->qos_rule_que_primap ) ;
			prule++;
		}
	}

	length += (*rulenum)*sizeof(ct_eth_port_qos_rule_t) ;
	
	fieldnum = (UCHAR *)(addr + length) ;
	pfield = (ct_eth_port_qos_rule_field_t *)(addr +length +1) ;
	length ++;

	if( *fieldnum > 0 )
	{
		vty_out(vty , "\r\n qos-rule-field  field-num=%d\r\n", *fieldnum) ;
		vty_out(vty, "  rule-set   rule   field  field_sel   field_match  operator\r\n") ;
		for( i=0; i< *fieldnum; i++)
		{	
			CTC_STACK_value_t val;
			UCHAR szText[80];
			
			VOS_MemZero( &val, sizeof( CTC_STACK_value_t) );
			VOS_MemZero( &szText, sizeof( szText) );

			if( (pfield->qos_rule_field_sel == 1) || (pfield->qos_rule_field_sel == 2) )
				VOS_MemCpy(val.mac_address, pfield->qos_rule_field_match_val, sizeof( mac_address_t) );
			else
				VOS_MemCpy( &val.match_value, pfield->qos_rule_field_match_val, sizeof( LONG ) );

			qosRuleFieldValue_ValToStr_parase( pfield->qos_rule_field_sel-1, &val, szText );
			
			vty_out(vty, "   %-10d%-7d%-7d%-12s%-13s%-16s", 
					pfield->qos_rule_set_index, pfield->qos_rule_index, pfield->qos_rule_field_index,
					qos_rule_field_sel_str(pfield->qos_rule_field_sel), 
					szText,
					qos_rule_field_operator_str(pfield->qos_rule_field_operator) );

			pfield++;
			
		}
	}

	length += (*fieldnum)*sizeof(ct_eth_port_qos_rule_field_t) ;

	return length ;

}

ULONG printctcmulticastcfgdata(ULONG addr , struct vty * vty)
{
	ULONG length = 1 ;
	UCHAR *mulctrlnum = (UCHAR *)addr;
	UCHAR *mulvidnum = NULL;
	int i , j ;
	char *tagStripedStr[] = { "striped", "unstriped" };

	eth_port_multicast_ctrl_entry * pMctrl = (eth_port_multicast_ctrl_entry *)(addr +length);
	eth_port_multicast_vlan_entry*pmvid = NULL ;

 	if( *mulctrlnum > 0 )
	{
		vty_out(vty, "\r\n multicast-ctrl  portnum=%d\r\n" , *mulctrlnum) ;
		vty_out(vty, " portNo  groupNum  multiTag\r\n") ;

		for( i = 0 ; i< *mulctrlnum ; i++)
		{
			if( pMctrl->ethPortMultiTagStriped > 1 )
				pMctrl->ethPortMultiTagStriped = 1;
			vty_out(vty, "   %-7d%-10d%-10s\r\n" , pMctrl->ethPorIndex , pMctrl->ethPortMulticastGroupNum, tagStripedStr[pMctrl->ethPortMultiTagStriped] ) ;

			pMctrl++;
	}
	}
	length += (*mulctrlnum)*sizeof(eth_port_multicast_ctrl_entry) ;

	
	mulvidnum = (UCHAR *)(addr +length) ;
	pmvid = (eth_port_multicast_vlan_entry *)(addr +length+1) ;
	length++ ;


	if( *mulvidnum > 0 )
	{
		vty_out(vty, "\r\n multicast_vlan  multi-num=%d\r\n", * mulvidnum) ;
		vty_out(vty, "\r\n portNo multiVlanId\r\n") ;

		for( j =0 ; j < *mulvidnum ; j ++ )
		{
			vty_out(vty, "   %-8d%-d\r\n" , pmvid->ethPortMulticastIndex , pmvid->ethportMulticastVlanVid );
			pmvid++;
		}
	}
	
	length += (*mulvidnum)*sizeof(eth_port_multicast_vlan_entry) ;
	
	return length ;
}


ULONG printonectconucfgdata(ULONG addr ,  struct vty * vty)
{
	ULONG length  = 0 ;

	ctc_debug_print( ("onuhead offset: %d\r\n", length) );
	length += printonuhead( (addr +length), vty) ;

	ctc_debug_print( ("llid offset: %d\r\n", length) );
	length += printctcLlidcfgdata( (addr +length) ,  vty) ;

	ctc_debug_print( ("ethport offset: %d\r\n", length) );
	length += printctcethportcfgdata( (addr +length) ,  vty) ;

	ctc_debug_print( ("vlancfg offset: %d\r\n", length) );
	length += printctcvlancfgdata( (addr +length) ,  vty);

	ctc_debug_print( ("qoscfg offset: %d\r\n", length) );
	length += printctcqoscfgdata( (addr +length) ,  vty);

	ctc_debug_print( ("mulcastcfg offset: %d\r\n", length) );
	length += printctcmulticastcfgdata( (addr +length) ,  vty);

	ctc_debug_print( (" offset: %d\r\n", length) );
	
	return length ;
}

void printCtcCfgData( struct vty * vty , UCHAR *addr )
{
	UCHAR *pprint = addr;
	cfg_file_head_t * phead = NULL ;
	ULONG  ptransfer = 0 ;

	ULONG onunum = 0 ;	/*指针定义有问题*/
	int i ;
	ULONG length  = 0 ;  
	
	phead = (cfg_file_head_t *)pprint ;
	onunum = phead->onunum ;

	ptransfer = (ULONG)pprint ;       /*pay attention to this kind of usage(将指针类型强制转化成变量类型变量里就可以存地址了但是在使用时还要强制转化成指针)*/
	
	if( (phead->fileLen > 0) && (phead->onunum<=MAX_ONU_NUM) &&  (phead->version == g_ctcSaveVer) )
	{
		length += printfilehead( (ptransfer +length) , vty);
		length += printglobalset( (ptransfer +length) ,  vty);
		/*ptransfer += length ;*/                  /*指针加乱了*/
		
		for(i = 0 ; i < onunum ; i++)
		{
			/*vty_out(vty,"\r\n\r\n onu%d " , i+1) ;*/
			length += printonectconucfgdata(  (ptransfer +length) , vty) ;
			/*ptransfer += length ;*/
		}
	}

	/*free( pprint ) ;
	pprint = NULL ;*/	/* 统一由上层函数释放 */
}
	

ULONG * applyforaddress( struct vty * vty )       /*申请地址*/
{
	ULONG * paddr = (ULONG *)g_malloc(g_flashFileLen) ;

	if( paddr != NULL)
	{
		int readlen = g_flashFileLen; 

		V2R1_disable_watchdog();
		if( VOS_ERROR == xflash_file_read( CTC_FLASH_FILE_ID, paddr, &readlen ) )
		{
			free( paddr );
			paddr = NULL;
		}
		V2R1_enable_watchdog();
	
		return paddr ;
	}
	else
	{
		VOS_ASSERT(0);
		return NULL;/*return 要返回一个值*/
	}
}

void judgePonId( ULONG addr , ULONG idx , struct vty * vty )/*判断设备索引*/
{
	UCHAR i = 0 ;
	USHORT onunumber = 0;

	cfg_file_head_t * pfilehead = (cfg_file_head_t *)addr ;
	ct_onu_head_t * pjudge = NULL;

	onunumber = pfilehead->onunum;

	if( g_ctcSaveDebFlag )
	{
		vty_out(vty,"addr=%x\r\n", addr);
		vty_out(vty,"cfg_file_head_t=%d\r\n", sizeof( cfg_file_head_t ) );
		vty_out(vty,"ct_global_set_t=%d\r\n", sizeof( ct_global_set_t ));
	}

	addr += (sizeof( cfg_file_head_t ) + sizeof( ct_global_set_t )) ;

	pjudge = (ct_onu_head_t *)addr ;

	if( g_ctcSaveDebFlag )
	{
		vty_out(vty, "pjudge=%x\r\n" , pjudge) ;
		vty_out(vty,"onunumber=%d\r\n" , onunumber);
	}
	
	for( i = 0 ; i < onunumber ; i++)
	{
		if( idx == pjudge->devIdx)
		{
			ULONG * pconvert = (ULONG *)pjudge ;

			if( g_ctcSaveDebFlag )
			{
				vty_out(vty,"devIdx=%d\r\n", pjudge->devIdx);
				vty_out(vty, "pconvert=%x" , pconvert) ;
				vty_out(vty, "val match\r\n");
				vty_out(vty,"idx2=%d\r\n", idx);
			}
			printonectconucfgdata((ULONG)pconvert , vty) ;
			break ;
		}
		else                /*找下一个设备*/
		{
			addr += ( ULONG )(pjudge->length) ;
			pjudge = (ct_onu_head_t *)addr ;			
		}		
	}	

	if( i == onunumber && g_ctcSaveDebFlag )
	{
		vty_out(vty, "can not find the device\r\n");
	}
}
#endif
#endif

#ifdef	__cplusplus
}
#endif
#endif

