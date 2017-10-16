/****************************************************************************
*
*     Copyright (c) 2006 GWTT Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           GWTT Corporation
*
*     All information contained in this document is GWTT Corporation
*     company private, proprietary, and trade secret.
*
*****************************************************************************/


#ifdef	__cplusplus
extern "C"
{
#endif

#include "syscfg.h"
#include "OltGeneral.h"

#if( EPON_MODULE_SYS_DIAGNOSE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"
#include "V2R1_product.h"
#include "PonEventHandler.h"
#include "eventOam.h"
#include "Cdp_pub.h"
#include "vos/vospubh/vos_int.h"
#include "bmsp/product_info/bms_product_info.h"
#include "bcm/pkt.h"
#include "../cli/Olt_cli.h"
#include "user_sw_port.h"
#include "bsp_cpld.h"
#include "src/customer/patch_bcm.h"

#define __SYS_DIAG_CPU_EMAC
#ifdef __SYS_DIAG_CPU_EMAC
extern LONG swport_2_slot_port(int sw_port, ULONG *ulSlot, ULONG *ulPort);
extern LONG slot_port_2_swport(int slot, int user_port, ULONG *ulSwSlot, ULONG *ulSwport);
extern void ethLoopChkRecvCallback(char *packet, ULONG *swport);
extern int emac_check_init();


/* add by xieshl 20090609，统计带内管理报文 */
typedef struct {
	ULONG all_pkts;		
	ULONG bad_size_pkts;
	ULONG data_null_pkts;
}cpu_rx_counter_t;

static cpu_rx_counter_t cpu_bms_rx = {0,0,0};
static cpu_rx_counter_t cpu_emac0_rx = {0,0,0};
static cpu_rx_counter_t cpu_emac1_rx = {0,0,0};

extern int (*emac0_rx_pkt_hookrtn)(ULONG, ULONG, UCHAR *, ULONG);
int pktFromEmac0Parse(ULONG portno, ULONG recv_flag, UCHAR * recv_buf, ULONG recv_len)
{
	cpu_emac0_rx.all_pkts++;
	if( recv_len < 48 )
	{
		cpu_emac0_rx.bad_size_pkts++;
		sys_console_printf("emac0_bad_size_pkts=%d,len=%d\r\n", cpu_emac0_rx.bad_size_pkts, recv_len );
		return VOS_ERROR;
	}
	if( recv_buf == NULL )
	{
		cpu_emac0_rx.data_null_pkts++;
		sys_console_printf("emac0_rx_data_null_pkts=%d,len=%d\r\n", cpu_emac0_rx.data_null_pkts, recv_len );
		return VOS_ERROR;
	}
	return VOS_OK;
}

extern int (*emac1_rx_pkt_hookrtn) (void *,unsigned int );
int pktFromEmac1Parse(void *lpPacketRecBuf,unsigned int buf_data_size)
{
	cpu_emac1_rx.all_pkts++;
	if( buf_data_size <= 14 /*48*/ )
	{
		cpu_emac1_rx.bad_size_pkts++;
		sys_console_printf("emac1_rx_bad_size_pkts=%d,len=%d\r\n", cpu_emac1_rx.bad_size_pkts, buf_data_size );
		return VOS_ERROR;
	}
	if( lpPacketRecBuf == NULL )
	{
		cpu_emac1_rx.data_null_pkts++;
		sys_console_printf("emac1_rx_data_null_pkts=%d,len=%d\r\n", cpu_emac1_rx.data_null_pkts, buf_data_size );
		return VOS_ERROR;
	}
	return VOS_OK;
}

DEFUN ( show_mgt_pkts,
         show_mgt_pkts_cmd,
         "show mgt_statistics",
 	  "Show information\n"
         "Show bms/emac0/emac1 receive pkts\n"
          )
{
	vty_out( vty, "            rx_all  rx_bad_size    rx_null\r\n" );	
	vty_out( vty, "------------------------------------------\r\n" );
	vty_out( vty, " BMS     %8d    %8d    %8d\r\n", cpu_bms_rx.all_pkts, cpu_bms_rx.bad_size_pkts, cpu_bms_rx.data_null_pkts );
	vty_out( vty, " EMC0    %8d    %8d    %8d\r\n", cpu_emac0_rx.all_pkts, cpu_emac0_rx.bad_size_pkts, cpu_emac0_rx.data_null_pkts );
	vty_out( vty, " EMC1    %8d    %8d    %8d\r\n", cpu_emac1_rx.all_pkts, cpu_emac1_rx.bad_size_pkts, cpu_emac1_rx.data_null_pkts );
	
	return CMD_SUCCESS;
}
DEFUN ( clear_mgt_pkts,
         clear_mgt_pkts_cmd,
         "clear mgt_statistics",
 	  "Clear information\n"
         "Clear bms/emac0/emac1 receive pkts\n"
          )
{
	cpu_bms_rx.all_pkts = 0;
	cpu_bms_rx.bad_size_pkts = 0;
	cpu_bms_rx.data_null_pkts = 0;
	cpu_emac0_rx.all_pkts = 0;
	cpu_emac0_rx.bad_size_pkts = 0;
	cpu_emac0_rx.data_null_pkts = 0;
	cpu_emac1_rx.all_pkts = 0;
	cpu_emac1_rx.bad_size_pkts = 0;
	cpu_emac1_rx.data_null_pkts = 0;
	
	return CMD_SUCCESS;
}

#if 0
typedef struct{
	UCHAR	desMac[6];
	UCHAR	srcMac[6];
	/*ULONG	ethTag;*/
	USHORT	ethType;
	USHORT	IpHead;
	USHORT	pduLen;
	UCHAR	pduBuf[50];
} __attribute__ ((packed)) ttt_pon_check_Frm_t;
VOID ponTxAndRxDiag( short int ponId, unsigned short vid )
{
	static UCHAR ttt_pon_frame_smac[6] = { 0x00, 0x0f, 0xe9, 0x00, 0x00, 0x00 };
	ttt_pon_check_Frm_t *pFrm;

	pFrm = VOS_Malloc( sizeof(ttt_pon_check_Frm_t), MODULE_PON );
	if( pFrm == NULL ) 
	{
		VOS_ASSERT(0);
		return;
	}
	VOS_MemCpy( pFrm->desMac, SYS_PRODUCT_BASEMAC, 6 );
	VOS_MemCpy( pFrm->srcMac, ttt_pon_frame_smac, 6 );
	pFrm->ethType = 64;
	pFrm->IpHead = 0x4500;
	pFrm->pduLen = sizeof( pFrm->pduBuf );;
	VOS_MemZero( pFrm->pduBuf, pFrm->pduLen );
	pFrm->pduBuf[0] = ponId;
	pFrm->pduBuf[1] = vid;
	pFrm->pduBuf[2] = 'O';		/*  */
	pFrm->pduBuf[4] = 'K';		/*  */

	/*PAS_send_frame( ponId, sizeof(ttt_pon_check_Frm_t), PON_PORT_SYSTEM, PON_LLID_NOT_AVAILABLE, PON_EVERY_LLID, pFrm );*/
    OLT_SendFrame2CNI( ponId, PON_LLID_NOT_AVAILABLE, (void*)pFrm, sizeof(ttt_pon_check_Frm_t) );

	VOS_Free( pFrm );
}
#endif
#endif	/* __SYS_DIAG_CPU_EMAC */

#define __SYS_DIAG_PON_TRAFFIC
#ifdef __SYS_DIAG_PON_TRAFFIC
/* modified by xieshl 20111018, 增加PON MAC地址表检查功能，支持命令配置并同步到各业务板 */
#define DIAG_MSG_CODE_TIMER		1
#define DIAG_MSG_CODE_CFG_SYNC	2

#define DIAG_MSG_SUBCODE_ENABLE	1
#define DIAG_MSG_SUBCODE_DISABLE	2
#define DIAG_MSG_SUBCODE_PERIOD	3


#define DIAG_MAXPONCHIP SYS_MAX_PON_PORTNUM

#define DIAG_ENABLE_NULL				0x00000000
#define DIAG_ENABLE_PON_UPTRAFFIC	0x00000001
#define DIAG_ENABLE_PON_LEARNING		0x00000002
#define DIAG_ENABLE_ALL				(DIAG_ENABLE_PON_UPTRAFFIC | DIAG_ENABLE_PON_LEARNING)

#define DIAG_INTERVAL_DEFAULT		90
#define DIAG_INTERVAL_MIN			60
#define DIAG_INTERVAL_MAX			3600

typedef struct {
/*	ULONG diag_slotno;
	ULONG diag_portno;
	ULONG diag_vid;*/
	ULONG diag_enable;
	ULONG diag_interval;
	ULONG diag_debug;
	
	ULONG cpu_rx_pkts[64];
	ULONG cpu_rx_oam[DIAG_MAXPONCHIP+1];
	ULONG pon_no_traffic[DIAG_MAXPONCHIP+1];
	ULONGLONG port_rx_pkts[DIAG_MAXPONCHIP+1];
}sys_diag_data_t;

typedef  struct{
	ULONG subCode;
	ULONG extData;
} __attribute__((packed)) sysDiagSyncCfgMsg_t;

sys_diag_data_t sys_diag_data;
ULONG sys_diag_queid = 0;
ULONG sys_diag_semid = 0;
LONG  sys_diag_timerid = 0;
VOS_HANDLE sys_diag_taskid = NULL;

extern int (*bms_rx_diag_hookrtn ) ( int unit, bcm_pkt_t *pkt, void *cookie );
extern int onuIdToOnuIndex( ushort_t ponId, ushort_t llId );

extern LONG eth_loop_detect_frame_is( UCHAR *pFrm, unsigned short length );
LONG sysDiagStart( ULONG enable );
LONG sysDiagStop( ULONG enable );
LONG sysDiagPeriod( ULONG period );
extern int bms_debug_rx;
int pktFromBmsParse( int unit, bcm_pkt_t *pkt, void *cookie )
{
	/*int i;*/
	ULONG slotno = 0,  portno = 0;
	ULONG swPort = 0;
    uint8  *packet = NULL;
	
	cpu_bms_rx.all_pkts++;
	if( (pkt == NULL) || (pkt->_pkt_data.data == NULL) )
	{
		cpu_bms_rx.data_null_pkts++;
		sys_console_printf("bms_rx_data_null_pkts=%d\r\n",cpu_bms_rx.data_null_pkts);
		return VOS_ERROR;
	}

    if( bms_debug_rx )
    {
       int i = pkt->pkt_len;
       if( i > 2000 )
          i = 1000;
       sys_console_printf("bms receive pkt,port=%d,length=%d\r\n",pkt->rx_port,pkt->pkt_len);
       pktDataPrintf( pkt->_pkt_data.data, i );
    }

	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )
		return VOS_ERROR;

	/*len = (ULONG)pkt->pkt_len;
	pFrm = (UCHAR *)pkt->_pkt_data.data;*/


#ifdef _EPON_12EPON_SUPPORT_
	swPort =pkt->rx_port ;   /* 在12EPON 上这里把交换芯片端口直接传送过来 */
#else
	swPort = PHY_2_L2( unit, pkt->rx_port );
#endif
	if( (pkt->pkt_len < 48) || (pkt->pkt_len >= 2048) )
	{
		cpu_bms_rx.bad_size_pkts++;
		sys_console_printf("bms_rx_bad_size_pkts(p%d)=%d\r\n", swPort, cpu_bms_rx.bad_size_pkts);
		return VOS_ERROR;
	}
#if 0
	if( swPort > 26 ) return VOS_ERROR;
#else
    SW_PORT_CHECK(swPort,VOS_ERROR);
#endif
	if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )
	{
		if( sys_diag_semid )
		{
			VOS_SemTake(sys_diag_semid, WAIT_FOREVER );
			sys_diag_data.cpu_rx_pkts[swPort]++;
			VOS_SemGive(sys_diag_semid);
		}
	}   /* 这里还在使用二层的端口在传递尚未修改 kkkkkkkk  */

	if( eth_loop_detect_frame_is(pkt->_pkt_data.data, pkt->pkt_len) && !SYS_LOCAL_MODULE_ISMASTERSTANDBY )
	{
#if _EPON_12EPON_SUPPORT_    /* modified by duzhk 2011-12-20  Carry Interface Index */
		swPort  = IFM_ETH_CREATE_INDEX(SWITCH_PORT_2_USER_SLOT( unit, pkt->rx_port ),
			SWITCH_PORT_2_USER_PORT(unit, pkt->rx_port));
#else
		if( VOS_OK != swport_2_slot_port(swPort, &slotno, &portno) )
			return VOS_ERROR;
#endif
		{
		       packet=(char *)VOS_Malloc(256, MODULE_LOOPBACK);/*lxl 20060726*/
			if( packet == NULL )	/* modified by xieshl 20090609 */
			{
				sys_console_printf("4K g_malloc NULL from BMS %s Line:%d\r\n", __FILE__, __LINE__ );
				return VOS_ERROR;
			}
		       VOS_MemCpy((uint8 *)packet, (uint8 *)pkt->_pkt_data.data, (ULONG)pkt->pkt_len); /*lxl 20060630*/
	
			ethLoopChkRecvCallback(packet, &swPort);
		}
		return VOS_ERROR;	/* modified by xieshl 20141212, 直接返回error，不需要再往上层送了 */
	}

	return VOS_OK;
}

/* 上行无流量监测OAM */
int eventOamMsg_uplinkTraffic( ushort_t ponId, ulong_t llId, commEventOamMsg_t *pOamMsg )
{
	int rc = VOS_OK;

	if( ponId >= DIAG_MAXPONCHIP ) 
		return VOS_ERROR;
	if( sys_diag_data.diag_debug )
	{
		int onuDevIdx = onuIdToOnuIndex( ponId, llId );
		if( onuDevIdx == VOS_ERROR )
		{
			sys_console_printf( "\r\nRECV OAM ERR: PON traffic uplink of pon_id=%d onu_id=%d\r\n", ponId, llId );
			return onuDevIdx;
		}
		/*else
			sys_console_printf("Uplink-Traffic-OAM:ponid=%d, onuid=%d, traf=%d\r\n", ponId, llId, sys_diag_data.cpu_rx_oam[ponId]);*/
	}

	VOS_SemTake( sys_diag_semid, WAIT_FOREVER );
	sys_diag_data.cpu_rx_oam[ponId]++;
	VOS_SemGive( sys_diag_semid );

	return rc;
}

extern int getPonToEthRxPkts( int slotno, int portno, int ponId, ULONGLONG *pRxPkts64 );
VOID ponTrafficDiag( )
{
	ULONG swUnit, swPort; 
	int ponId;
	int slotno, portno;
	int PonChipType;
	ULONGLONG pkts64 = 0;
	if (SYS_PRODUCT_TYPE != PRODUCT_E_EPON3)
		return;
	if( sys_diag_semid == 0 )
		return;
	
	if( sys_diag_data.diag_debug )
	{
		sys_console_printf("\r\n");
	}
	for( slotno = PONCARD_FIRST; slotno <= PONCARD_LAST; slotno++ )
	{
		if( SlotCardIsPonBoard(slotno) == VOS_ERROR )
			continue;
		if( !SYS_MODULE_IS_RUNNING(slotno) )
			continue;

		if( sys_diag_data.diag_debug )
			sys_console_printf( " Diag:" );
		
		for( portno = 1; portno <= PONPORTPERCARD; portno++ )
		{
			if( sys_diag_data.diag_debug )
				sys_console_printf( " PON%d/%d", slotno, portno );

			if( (slot_port_2_swport(slotno, portno, &swUnit, &swPort) == VOS_ERROR) || (swPort == 0xff) )
				continue;
			if( (ponId = GetPonPortIdxBySlot(slotno, portno)) == VOS_ERROR )
				continue;

			/* 如果PON 没有处于工作状态则不检测 */
			if( PonPortIsWorking( ponId ) != TRUE)
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--invalid" );
				continue;
			}
			/* 如果不是5001 PON 则不检测*/
			PonChipType = V2R1_GetPonchipType( ponId);
			if( PonChipType != PONCHIP_PAS5001 )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--skip   " );
				continue;
			}
			/* 如果PON下没有注册ONU则不检测 */
			if( OnLineOnuCounter(ponId)  <= 0 )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--no onu " );
				continue;
			}
			
			/* 如果PON 对应的ETH 端口上行统计有流量则不检测 */
			/*if( getRtStatsPkts(1, slotno, portno, &pkts64) != VOS_OK )
			{
				if( pkts64 != sys_diag_data.port_rx_pkts[ponId] )
				{
					sys_diag_data.port_rx_pkts[ponId] = pkts64;
					continue;
				}
			}*/
			if( getPonToEthRxPkts( slotno, portno, ponId, &pkts64) == VOS_ERROR )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--if err " );
				continue;
			}
			if( pkts64 != sys_diag_data.port_rx_pkts[ponId] )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--ok(%d)", (LONG)(pkts64-sys_diag_data.port_rx_pkts[ponId]) );
				sys_diag_data.port_rx_pkts[ponId] = pkts64;
				continue;
			}

			/* 如果CPU 从PON 上没有收到检测OAM，或者从其对应的ETH 端口上收到了报文，则不检测 */
			if( (sys_diag_data.cpu_rx_pkts[swPort] != 0) || ((sys_diag_data.cpu_rx_oam[ponId] == 0)) )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( "--ok(%d/%d)", sys_diag_data.cpu_rx_oam[ponId], sys_diag_data.cpu_rx_pkts[swPort] );

				VOS_SemTake(sys_diag_semid, WAIT_FOREVER );
				sys_diag_data.cpu_rx_pkts[swPort] = 0;
				sys_diag_data.cpu_rx_oam[ponId] = 0;
				VOS_SemGive(sys_diag_semid);

				continue;
			}

			/* 上行无流量，重新加载PON 固件 */
			sys_diag_data.pon_no_traffic[ponId]++;
			if( sys_diag_data.diag_debug )
				sys_console_printf( "--no traffic" );

			PonPortOther_EventReport( slotno, portno,  other_pon_update_file);
			UpdatePonFirmware( ponId );		/* reset pon */
			VOS_TaskDelay(1500);

			VOS_SemTake(sys_diag_semid, WAIT_FOREVER );
			sys_diag_data.cpu_rx_pkts[swPort] = 0;
			sys_diag_data.cpu_rx_oam[ponId] = 0;
			VOS_SemGive(sys_diag_semid);
		}
		
		if( sys_diag_data.diag_debug )
		{
			sys_console_printf("\r\n");
		}
	}
}

/* added by xieshl 20111017, 定时检查PON MAC地址表学习状态 */
VOID ponFdbLearningDiag()
{
  	short int  pas_ret;
	short int PonPortIdx;
	ULONG slotno, portno;
	#if 1/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_addr_table_cfg_t addrtbl_cfg;
	#else
  	PON_address_table_config_t  address_table_config;
	#endif

	/*for(slotno = PONCARD_FIRST; slotno <= PONCARD_LAST; slotno++)*/
	slotno = SYS_LOCAL_MODULE_SLOTNO;
	{
		if(SlotCardIsPonBoard(slotno) != ROK) return;
		
		for( portno=1; portno<=PONPORTPERCARD; portno++ )
		{
			if( getPonChipInserted((unsigned char )(slotno), (unsigned char )(portno) ) !=  PONCHIP_EXIST )
			{
				continue;
			}
			PonPortIdx = GetPonPortIdxBySlot( slotno, portno );
			if( PonPortIsWorking(PonPortIdx) == FALSE ) 
			{
				continue;
			}
			/*if(( GetPonChipTypeByPonPort( PonPortIdx) != PONCHIP_PAS5201 ) ) 
			{
				continue;
			}*/
            #if 1
		
			VOS_MemZero( &addrtbl_cfg, sizeof(addrtbl_cfg) );
			if( OLT_GetAddressTableConfig(PonPortIdx, &addrtbl_cfg) != 0 )
					continue;

			if( PON_DIRECTION_UPLINK == addrtbl_cfg.allow_learning )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( " Diag:pon%d/%d fdb-entry learning status is OK\r\n", slotno, portno );
				continue;
			}
			else
			{
				sys_console_printf( " Diag:pon%d/%d fdb-entry learning status is changed\r\n", slotno, portno );
			}
            /*modi by luh@2016-5-5; 参数不生效，需要修改为enable*/
			addrtbl_cfg.allow_learning = V2R1_ENABLE;/*PON_DIRECTION_UPLINK;*/
			pas_ret = OLT_SetAddressTableConfig(PonPortIdx, &addrtbl_cfg);

            #else
			VOS_MemZero( &address_table_config, sizeof(address_table_config) );
			if( PAS_get_address_table_configuration(PonPortIdx, &address_table_config) != PAS_EXIT_OK )
				continue;

			if( PON_DIRECTION_UPLINK == address_table_config.allow_learning )
			{
				if( sys_diag_data.diag_debug )
					sys_console_printf( " Diag:pon%d/%d fdb-entry learning status is OK\r\n", slotno, portno );
				continue;
			}
			else
			{
				sys_console_printf( " Diag:pon%d/%d fdb-entry learning status is changed\r\n", slotno, portno );
			}
			
			address_table_config.allow_learning = PON_DIRECTION_UPLINK;

			pas_ret = PAS_set_address_table_configuration(PonPortIdx, address_table_config);
			#endif
			/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			/*if(pas_ret == PAS_EXIT_OK)
			{
				sys_console_printf( " Diag:pon%d/%d fdb-entry allow-learning changed\r\n", slotno, portno );
			}*/
		}
	}
}

void sysDiagTimerCallback()
{
	ULONG aulMsg[4] = {MODULE_RPU_DIAG, DIAG_MSG_CODE_TIMER, 0, 0};
	SYS_MSG_S * pstMsg = NULL;

	if( (sys_diag_data.diag_enable & DIAG_ENABLE_ALL) == DIAG_ENABLE_NULL )
		return;
	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return;
	
	if ( VOS_QueNum( sys_diag_queid ) >= 3 )
	{
		if( sys_diag_data.diag_debug)
			sys_console_printf( "\r\nTask sys-diag message queue is busy\r\n" );
		return;
	}

	pstMsg = ( SYS_MSG_S* ) VOS_Malloc( sizeof(SYS_MSG_S), MODULE_RPU_DIAG );
	if ( NULL == pstMsg )
	{
		return ;
	}
	VOS_MemZero( pstMsg, sizeof(SYS_MSG_S) );
	pstMsg->ulSrcModuleID = MODULE_RPU_DIAG;
	pstMsg->ulDstModuleID = MODULE_RPU_DIAG;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;/*目的slot*/
	pstMsg->ucMsgType = MSG_TIMER;
	pstMsg->usMsgCode = DIAG_MSG_CODE_TIMER;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
	pstMsg->ptrMsgBody = NULL;
	pstMsg->usFrameLen = 0;

	aulMsg[3] = (ULONG)pstMsg;
	
	if( VOS_QueSend( sys_diag_queid, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		VOS_ASSERT(0);
		VOS_Free( pstMsg );
	}
}

LONG sysDiagSync_configData_2Slave( ULONG slotno, VOID *pCfgData, ULONG dataLen )
{
	LONG rc;
	SYS_MSG_S   *pstMsg;
	sysDiagSyncCfgMsg_t *pSndCfgMsg;
	ULONG msgLen;

	if( (pCfgData == NULL) || (dataLen == 0) || (dataLen >sizeof(sysDiagSyncCfgMsg_t)) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if( !SYS_MODULE_SLOT_ISHAVECPU(slotno) )
		return VOS_OK;

	msgLen = sizeof(SYS_MSG_S) + dataLen;
	
	if(  SYS_MODULE_IS_READY(slotno) && (slotno != SYS_LOCAL_MODULE_SLOTNO) )
	{
		pstMsg = CDP_AllocMsg( msgLen, MODULE_RPU_DIAG );
		if ( NULL == pstMsg )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		pSndCfgMsg = (sysDiagSyncCfgMsg_t *)(pstMsg + 1);

		VOS_MemZero( pstMsg, msgLen );
		pstMsg->ulSrcModuleID = MODULE_RPU_DIAG;
		pstMsg->ulDstModuleID = MODULE_RPU_DIAG;
		pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
		pstMsg->ulDstSlotID = slotno;/*目的slot*/
		pstMsg->ucMsgType = MSG_NOTIFY;
		pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  /* 消息头和数据一体 */
		pstMsg->ptrMsgBody = (VOID *)pSndCfgMsg;
		pstMsg->usFrameLen = dataLen;
		pstMsg->usMsgCode = DIAG_MSG_CODE_CFG_SYNC;

		VOS_MemCpy( pSndCfgMsg, pCfgData, dataLen );
				
		rc = CDP_Send( RPU_TID_CDP_DIAG, slotno,  RPU_TID_CDP_DIAG,  CDP_MSG_TM_ASYNC, pstMsg, msgLen, MODULE_RPU_DIAG );
		if( rc !=  VOS_OK )
		{
			VOS_ASSERT(0);
			CDP_FreeMsg( (void *) pstMsg );
			return VOS_ERROR;
		}	
	}

	return VOS_OK;
}
LONG sysDiagSync_configData_2AllSlave( VOID *pCfgData, ULONG dataLen )
{
	ULONG slotno;

	if( pCfgData == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		return VOS_OK;
	}

	for( slotno = 1; slotno <= SYS_CHASSIS_SLOTNUM; slotno++ )
	{
		if( slotno == SYS_LOCAL_MODULE_SLOTNO )
			continue;
		sysDiagSync_configData_2Slave( slotno, pCfgData, dataLen );
	}
	return VOS_OK;
}

LONG sysDiagSync_cdpRecvProc( sysDiagSyncCfgMsg_t *pRecvMsg )
{
	LONG rc = VOS_ERROR;
	
	if( NULL == pRecvMsg )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	switch( pRecvMsg->subCode )
	{
		case DIAG_MSG_SUBCODE_ENABLE:
			rc = sysDiagStart( pRecvMsg->extData );
			break;
		case DIAG_MSG_SUBCODE_DISABLE:
			rc = sysDiagStop( pRecvMsg->extData );
			break;
		case DIAG_MSG_SUBCODE_PERIOD:
			rc = sysDiagPeriod( pRecvMsg->extData );
			break;
		default:
			break;
	}
	return rc;
}


static VOID sysDiagTask()
{
	ULONG ulRcvMsg[4];
	LONG result;
	SYS_MSG_S *pMsg;

	while(!SYS_MODULE_IS_READY(SYS_LOCAL_MODULE_SLOTNO) )
	{
		VOS_TaskDelay( 50 );
	}

	while( 1 )
	{
		result = VOS_QueReceive( sys_diag_queid, ulRcvMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_TaskDelay(100);
			ASSERT(0);
			continue;
		}

		pMsg = (SYS_MSG_S *)ulRcvMsg[3];
		if( NULL == pMsg )
		{
			VOS_ASSERT(pMsg);
			VOS_TaskDelay(20);
			continue;
		}

		if ( CDP_NOTI_FLG_SEND_FINISH == ulRcvMsg[1] )
		{
			CDP_FreeMsg( pMsg ); 	   /* cdp 异步发送完成*/
			continue;
		}

		switch( pMsg->usMsgCode )
		{
			case DIAG_MSG_CODE_TIMER:
				if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )
				{
					ponTrafficDiag();		/* 统计交换芯片到CPU 数据流量 */
				}
				if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_LEARNING )
				{
					ponFdbLearningDiag();
				}
				break;
			case DIAG_MSG_CODE_CFG_SYNC:
				sysDiagSync_cdpRecvProc( (sysDiagSyncCfgMsg_t *)(pMsg+1) );
				break;
			case AWMC_CLI_BASE:
				decode_command_msg_packet( pMsg, VOS_MSG_TYPE_QUE );
				break;
			default:
				break;
		}
		
		if(SYS_MSG_SRC_SLOT(pMsg) != (SYS_LOCAL_MODULE_SLOTNO))
		{
			CDP_FreeMsg( pMsg );    /*CDP消息释放*/
		}
		else
		{
			VOS_Free(pMsg);         /*本板消息释放(包括PDU消息)*/
		}
		pMsg = NULL;
	}
	
	sys_diag_taskid = NULL;
}

LONG sysDiagStart( ULONG enable )
{
	LONG rc = VOS_OK;
	sysDiagSyncCfgMsg_t cfgMsg;

	if( sys_diag_data.diag_debug)
		sys_console_printf( "\r\n Diag:set enable=%x\r\n", enable );

	sys_diag_data.diag_enable |= (enable & DIAG_ENABLE_ALL);

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		cfgMsg.subCode = DIAG_MSG_SUBCODE_ENABLE;
		cfgMsg.extData = enable;
		rc = sysDiagSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(sysDiagSyncCfgMsg_t) );
	}
	return rc;
}

LONG sysDiagStop( ULONG enable )
{
	LONG rc = VOS_OK;
	sysDiagSyncCfgMsg_t cfgMsg;

	if( sys_diag_data.diag_debug)
		sys_console_printf( "\r\n Diag:set disable=%x\r\n", enable );

	enable &= DIAG_ENABLE_ALL;
	sys_diag_data.diag_enable &= ( ~enable );

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		cfgMsg.subCode = DIAG_MSG_SUBCODE_DISABLE;
		cfgMsg.extData = enable;
		rc = sysDiagSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(sysDiagSyncCfgMsg_t) );
	}
	return rc;
}

LONG sysDiagPeriod( ULONG period )
{
	LONG rc = VOS_OK;
	sysDiagSyncCfgMsg_t cfgMsg;

	if( sys_diag_data.diag_debug)
		sys_console_printf( "\r\n Diag:set period=%d\r\n", period );

	if( (period < DIAG_INTERVAL_MIN) || (period > DIAG_INTERVAL_MAX) )
		period = DIAG_INTERVAL_DEFAULT;
	
	sys_diag_data.diag_interval = period;
	VOS_TimerChange( MODULE_RPU_DIAG, sys_diag_timerid, period * 1000 );

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		cfgMsg.subCode = DIAG_MSG_SUBCODE_PERIOD;
		cfgMsg.extData = period;
		rc = sysDiagSync_configData_2AllSlave( (VOID *)&cfgMsg, sizeof(sysDiagSyncCfgMsg_t) );
	}
	return rc;
}

QDEFUN( sys_diag_enable_pon_uplink,
       sys_diag_enable_pon_uplink_cmd,
       "diagnose pon-link [enable|disable]",
	"System diagnose\n"
	"Diagnose pon uplink traffic\n"
	"Diagnosis start\n"
	"Diagnosis stop\n",
	&sys_diag_queid
       )
{
	if( argv[0][0] == 'e' )
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )
			vty_out( vty, " Diagnose is enable already\r\n" );
		else
			sysDiagStart( DIAG_ENABLE_PON_UPTRAFFIC );
	}
	else
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )
			sysDiagStop( DIAG_ENABLE_PON_UPTRAFFIC );
		else
			vty_out( vty, " Diagnose is disable already\r\n" );
	}
	return CMD_SUCCESS;
}

QDEFUN( sys_diag_enable_pon_learning,
       sys_diag_enable_pon_learning_cmd,
       "diagnose pon-learning [enable|disable]",
	"System diagnose\n"
	"Diagnose pon fdb entry learning status\n"
	"Diagnosis start\n"
	"Diagnosis stop\n",
	&sys_diag_queid
       )
{
	if( argv[0][0] == 'e' )
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_LEARNING )
			vty_out( vty, " Diagnose is enable already\r\n" );
		else
			sysDiagStart( DIAG_ENABLE_PON_LEARNING );
	}
	else
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_LEARNING )
			sysDiagStop( DIAG_ENABLE_PON_LEARNING );
		else
			vty_out( vty, " Diagnose is disable already\r\n" );
	}
	return CMD_SUCCESS;
}


QDEFUN( sys_diag_period_config,
       sys_diag_period_config_cmd,
       "diagnose period <60-3600>",
	"Diagnose pon uplink traffic\n"
	"Diagnose pon period\n"
	"Input diagnosis period, unit:second\n",
	&sys_diag_queid
       )
{
	LONG period = VOS_AtoL(argv[0]);
	if( (period >= 60) && (period <= 3600) )
	{
		sysDiagPeriod( period );
	}
	return CMD_SUCCESS;
}

QDEFUN( sys_diag_config_show,
       sys_diag_config_show_cmd,
       "show diagnose",
	"Show running system information\n"
	"Show diagnose config info.\n",
	&sys_diag_queid
       )
{
	int slotno, portno;
	int ponId;
	
	if( sys_diag_data.diag_enable & DIAG_ENABLE_ALL )
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )
		{
			vty_out( vty, " Diagnose pon uplink traffic is enable\r\n" );
			
			for( slotno = PONCARD_FIRST; slotno <= PONCARD_LAST; slotno++ )
			{
				for( portno = 1; portno <= PONPORTPERCARD; portno++ )
				{
					ponId = GetPonPortIdxBySlot( slotno, portno );
					if( ponId == VOS_ERROR )
						continue;

					if( sys_diag_data.pon_no_traffic[ponId] )
						vty_out( vty, "   PON%d/%d RST=%d\r\n", slotno, portno, sys_diag_data.pon_no_traffic[ponId] );
				}
			}
		}
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_LEARNING )
		{
			vty_out( vty, " Diagnose pon fdb learning status is enable\r\n" );
		}
		vty_out( vty, " Diagnose period:%d seconds\r\n", sys_diag_data.diag_interval );

	}
	else
		vty_out( vty, " Diagnose is disable\r\n" );
	return CMD_SUCCESS;
}

DEFUN( sys_diag_debug,
       sys_diag_debug_cmd,
       "debug diagnose pon",
	"Debug\n"
	"system diagnose\n"
	"diagnose pon uplink traffic and learning status\n"
       )
{
	sys_diag_data.diag_debug = 1;
	return CMD_SUCCESS;
}

DEFUN( sys_diag_undo_debug,
       sys_diag_undo_debug_cmd,
       "undo debug diagnose pon",
       "Negate a command or set its defaults\n"
	"Debug\n"
	"system diagnose\n"
	"diagnose pon uplink traffic and learning status\n"
       )
{
	sys_diag_data.diag_debug = 0;
	return CMD_SUCCESS;
}

LONG test_pon_fdb_cfg(short int PonPortIdx, short int learning)
{
	LONG iRlt;
	#if 1/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	OLT_addr_table_cfg_t addrtbl_cfg;
	#else
	PON_address_table_config_t address_table_config;	
    #endif
	if( !OLT_LOCAL_ISVALID(PonPortIdx) )
		return VOS_ERROR;
	if( PonPortIsWorking(PonPortIdx) == FALSE )
		return VOS_ERROR;
    #if 1
	VOS_MemZero( &addrtbl_cfg, sizeof(addrtbl_cfg) );
	if ( PAS_EXIT_OK == (iRlt = OLT_GetAddressTableConfig(PonPortIdx, &addrtbl_cfg)) )
	{
		if ( V2R1_ENABLE == learning )
			addrtbl_cfg.allow_learning = PON_DIRECTION_UPLINK;
		else
			addrtbl_cfg.allow_learning = PON_DIRECTION_NO_DIRECTION;
		iRlt = OLT_SetAddressTableConfig(PonPortIdx, &addrtbl_cfg);
	}
			
    #else
	if ( PAS_EXIT_OK == (iRlt = PAS_get_address_table_configuration(PonPortIdx, &address_table_config)) )
	{
		if ( V2R1_ENABLE == learning )
			address_table_config.allow_learning = PON_DIRECTION_UPLINK;
		else
			address_table_config.allow_learning = PON_DIRECTION_NO_DIRECTION;
		iRlt = PAS_set_address_table_configuration(PonPortIdx, address_table_config);
	}
	#endif
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	return iRlt;
}
#endif	/* __SYS_DIAG_PON_TRAFFIC */

#define __SYS_DIAG_PHY_LOS
#ifdef __SYS_DIAG_PHY_LOS
/* added by xieshl 20110620, 解决GEM光口误报linkdown问题 */
extern unsigned long phy_5464_reset_counter[64];
extern char phy_5464_reset_flag[64];
extern unsigned long phy_5464_reset_enable;
extern int phy_5464_fiber_signal_detect_init(int unit, int port);
extern int phy_5464_mode_get(int unit, int port);
extern ULONG ETH_slot_logical_portnum( ULONG slotno );

int phy5464_fiber_los_init( ULONG slotno, ULONG portno )
{
	int phyUnit, phyPort;

	if (VOS_ERROR == slot_port_2_swport( slotno, portno, &phyUnit, &phyPort)) ;
		return VOS_ERROR;
	/*sys_console_printf( "eth%d/%d phy-port %d-%d\r\n", slotno, portno, phyUnit, phyPort);*/
	return phy_5464_fiber_signal_detect_init( phyUnit, phyPort );
}
int phy5464_fiber_signal_get( ULONG slotno, ULONG portno )
{
	int phyUnit, phyPort;
	uint16 mode = 0;

	if( (__SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA_GEM) || (portno == 0) || (portno > ETH_slot_logical_portnum(slotno)) )
		return VOS_ERROR;
	if (VOS_ERROR == slot_port_2_swport( slotno, portno, &phyUnit, &phyPort)) ;
		return VOS_ERROR;
	mode = phy_5464_mode_get(phyUnit, phyPort);
	
	/*sys_console_printf( "eth%d/%d (phy%d-%d) mode=%d\r\n", slotno, portno, phyUnit, phyPort, mode );*/
	return mode;
}

#define SYS_MODULE_IS_UPLINK_FIBER(slotno) ((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)||\
                                           (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_GE)||\
                                           (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEO)||\
                                           (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEM)||\
                                           (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_GEM))
DEFUN(show_phy_fiber_reset_counter,
	show_phy_fiber_reset_counter_cmd,
	"show phy5464-reset-counter",
	SHOW_STR
	"show phy5464 fiber reset counter\n"
	)
{
	ULONG slotno, portno;
	int phyUnit, phyPort;
	int count = 0;

	vty_out( vty, " %-12s%-12s %-12s%-12s\r\n", "eth-phy", "rst-count", "eth-phy", "rst-count" );
	for( slotno=1; slotno<SYS_CHASSIS_SLOTNUM; slotno++ )
	{
		if( !SYS_MODULE_IS_UPLINK_FIBER(slotno) )
			continue;
		for( portno=1; portno<=ETH_slot_logical_portnum(slotno); portno++ )
		{
			if (VOS_ERROR == slot_port_2_swport( slotno, portno, &phyUnit, &phyPort)) ;
				continue;
			if( phyPort < 64 )
			{
				vty_out( vty, " %2d/%-9d%-12d", slotno, portno, phy_5464_reset_counter[phyPort] );
				count++;
			}
			if( (count & 1) == 0 )
				vty_out( vty, "\r\n" );
		}
	}
	if( count & 1 )
		vty_out( vty, "\r\n" );
	
	return CMD_SUCCESS;
}

DEFUN(phy_fiber_reset_enable,
	phy_fiber_reset_enable_cmd,
	"phy5464-fiber-reset {[enable|disable]}*1",
	"phy5464 fiber reset contral\n"
	"enable\n"
	"disable\n"
	)
{
	int ebl = 0;
	char *pStr[] = {"enable", "disable" };
	ULONG slotno, portno;
	int swPort, phyUnit, phyPort;
	int count = 0;
	
	if( argc == 0 )
	{
		vty_out( vty, "phy5464 fiber reset %s\r\n", (phy_5464_reset_enable ? pStr[0] : pStr[1]) );
		vty_out( vty, " %-12s%-12s %-12s%-12s\r\n", "eth-phy", "rst-state", "eth-phy", "rst-state" );

		for( slotno=1; slotno<SYS_CHASSIS_SLOTNUM; slotno++ )
		{
			if( !SYS_MODULE_IS_UPLINK_FIBER(slotno) )
				continue;
			for( portno=1; portno<=ETH_slot_logical_portnum(slotno); portno++ )
			{
				if (VOS_ERROR == slot_port_2_swport( slotno, portno, &phyUnit, &phyPort)) ;
					continue;
				if( phyPort < 64 )
				{
					vty_out( vty, " %2d/%-9d%-12d", slotno, portno, phy_5464_reset_flag[phyPort] );
					count++;
				}
				if( (count & 1) == 0 )
					vty_out( vty, "\r\n" );
			}
		}
		if( count & 1 )
			vty_out( vty, "\r\n" );
	}
	else
	{
		if( argv[0][0] == 'e' )
		{
			ebl = 1;
			for(phyPort=0; phyPort<64; phyPort++)
				phy_5464_reset_flag[phyPort]=ebl;

		}
		if( ebl != phy_5464_reset_enable )
		{
			for( phyPort=0; phyPort<64; phyPort++)
				phy_5464_reset_flag[phyPort]=ebl;
		}
		else
			vty_out( vty, "phy5464 fiber reset already %s\r\n", (ebl ? pStr[0] : pStr[1]) );
	}
	return CMD_SUCCESS;
}
#endif	/* __SYS_DIAG_PHY_LOS */

#define __SYS_DIAG_TK_PON_EN
#ifdef __SYS_DIAG_TK_PON_EN
/* added by xieshl 20151022，现场问题，统计发送消息，检查PON使能状态 */
#define TK_OLT_CMDMSG_ID_MIN	0
#define TK_OLT_CMDMSG_ID_MAX	872
#define TK_OLT_EVNMSG_ID_MIN	0x8000U
#define TK_OLT_EVNMSG_ID_MAX	0x8036U
#define TK_OLT_MSG_ID_NUM		(TK_OLT_CMDMSG_ID_MAX - TK_OLT_CMDMSG_ID_MIN + 1 + TK_OLT_EVNMSG_ID_MAX - TK_OLT_EVNMSG_ID_MIN + 1)
#define TK_MSG_ID_2_IDX(msg_id) ((msg_id <= TK_OLT_CMDMSG_ID_MAX) ? msg_id : (msg_id - TK_OLT_EVNMSG_ID_MIN + TK_OLT_CMDMSG_ID_MAX + 1) )
#define TK_MSG_IDX_2_ID(idx) 	((idx <= TK_OLT_CMDMSG_ID_MAX) ? idx : (idx - TK_OLT_CMDMSG_ID_MAX - 1 + TK_OLT_EVNMSG_ID_MIN) )
#define TK_MSG_ID_VALID(msg_id)      (((TK_OLT_CMDMSG_ID_MIN <= (msg_id)) && ((msg_id) <= TK_OLT_CMDMSG_ID_MAX)) ||\
				((TK_OLT_EVNMSG_ID_MIN <= (msg_id)) && ((msg_id) <= TK_OLT_EVNMSG_ID_MAX)))

typedef struct {
	ULONG sn;
	ULONG count;
} tk_olt_msg_count_t;
typedef struct {
	ULONG lastSn;
	ULONG lastMsgId;
	tk_olt_msg_count_t *pMsgCount;
} tk_olt_msg_stat_t;
static tk_olt_msg_stat_t tk_olt_msg_stat[CARD_MAX_PON_PORTNUM+1];
static UCHAR tk_olt_chk_count[CARD_MAX_PON_PORTNUM+2];
ULONG tk_olt_chk_thr = 2;
LONG tk_olt_chk_en = VOS_NO;
static VOID TkOLT_ScanPonPortState()
{
	int i, n;
	unsigned char en;
    short int olt_ids[MAXOLTPERPONCHIP];
	int resetPon = -1;
	
	if( tk_olt_chk_en != VOS_YES )
		return;

    n = GetPonChipPonPorts(0, olt_ids);
	if( n == 0 )
		return;
	
	for( i=0; i<MAXPONPORT_PER_BOARD; i++ )
	{
		if( !PonPortIsWorking(i) )
		{
			continue;
		}
		
		en = 0;
		if( (TkOLT_IsEnabled(i, &en) != TK_EXIT_OK) || (en == 0) )
		{
			tk_olt_chk_count[i]++;
			if( tk_olt_chk_count[i] >= tk_olt_chk_thr )
			{
				tk_olt_chk_count[i] = 0;
				resetPon = (i / n) * n;
				break;
			}
		}
		else
		{
			tk_olt_chk_count[i] = 0;
		}
	}
	if( resetPon != -1 )
	{
		for( i=resetPon+(n-1); i>=resetPon; i-- )
		{
			/* PON复位 */
#if 0
			Pon_RemoveOlt(i, FALSE, FALSE);	
			PonAbnormalHandler(i, PON_OLT_RESET_OLT_EVENT);

		    if( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
		    {
		        OLT_SYNC_Data(i, FC_PON_RESET, NULL, 0);
		    }
			ClearPonRunningData( i );
			Add_PonPort( i );
			/*if( i % n == 0 )
				OLT_ResetPon(i);*/
#else
			sendPonResetEventMsg( i, 0xff );
#endif
		}
	}
}
static VOID TkOLT_StatPonPortMsg( short int olt_id, unsigned short msg_id, void *msg_ptr, unsigned long msg_len )
{
	tk_olt_msg_count_t *pCount = NULL;
	tk_olt_msg_stat_t *pStat = NULL;

	if( tk_olt_chk_en != VOS_YES )
		return;
	
	if( (olt_id < 0) || (olt_id >= CARD_MAX_PON_PORTNUM) || !TK_MSG_ID_VALID(msg_id) )
	{
		sys_console_printf("pon_idx=%d,msg_id=%d ERR!\r\n", olt_id, msg_id);
		return;
	}
	pStat = &tk_olt_msg_stat[olt_id];
	if( pStat->pMsgCount == NULL )
	{
		VOS_ASSERT(0);
		return;
	}
	pCount = &pStat->pMsgCount[TK_MSG_ID_2_IDX(msg_id)];
	pCount->count++;
	pCount->sn = pStat->lastSn++;
	pStat->lastMsgId = msg_id;
}
static VOID TkOLT_ShowPonMsgCounter( struct vty *vty, short int olt_id )
{
	int i, msg_id, pnt = 0;
	tk_olt_msg_stat_t *pStat = NULL;
	tk_olt_msg_count_t *pCount = NULL;
	char *pTitle = "  MSG_ID    COUNT       SN\r\n";
	
	if( (olt_id < 0) || (olt_id >= CARD_MAX_PON_PORTNUM) )
		return;
	
	pStat = &tk_olt_msg_stat[olt_id];
	if( pStat->pMsgCount == NULL )
		return;
	
	for( i=0; i<TK_OLT_MSG_ID_NUM; i++ )
	{
		pCount = &pStat->pMsgCount[i];
		if( (pCount->count == 0) || (pCount->sn == 0) )
			continue;

		if( pnt == 0 )
		{
			pnt++;
			if( vty )
				vty_out( vty, pTitle );
			else
				sys_console_printf( pTitle );
		}
		
		msg_id = TK_MSG_IDX_2_ID(i);
		if( vty )
		{
			if( msg_id == pStat->lastMsgId )
				vty_out(vty, "->%6x %8d %8d\r\n", msg_id, pCount->count, pCount->sn );
			else
				vty_out(vty, "%8x %8d %8d\r\n", msg_id, pCount->count, pCount->sn );
		}
		else
		{
			if( msg_id == pStat->lastMsgId )
				sys_console_printf( "->%6x %8d %8d\r\n", msg_id, pCount->count, pCount->sn );
			else
				sys_console_printf( "%8x %8d %8d\r\n", msg_id, pCount->count, pCount->sn );
		}
	}
}

VOID TkOLT_ScanPonPortStart( ULONG en )
{
	extern void (*pon_enable_check_callback)();
	extern void (*TkHostMsgStatisticsCallback)(short int olt_id, unsigned short msg_type, void *msg_ptr, unsigned long msg_len);

	if( tk_olt_chk_en == en )
		return;
	if( en == VOS_YES )
	{
		VOS_MemZero( tk_olt_chk_count, sizeof(tk_olt_chk_count) );
		pon_enable_check_callback = TkOLT_ScanPonPortState;
		TkHostMsgStatisticsCallback = TkOLT_StatPonPortMsg;
	}
	else
	{
		/*pon_enable_check_callback = NULL;
		TkHostMsgStatisticsCallback = NULL;*/
	}
	tk_olt_chk_en = en;
}

DEFUN(config_pon_state_check,
	config_pon_state_check_cmd,
	"config pon-state-check {[enable|disable]}*1",
	"config pon state check\n"
	"config pon state check\n"
	"enable to check\n"
	"disable to check\n"
	)
{
	ULONG en = VOS_NO;
	if( argc == 0 )
	{
		vty_out( vty, "pon enable check: %s\r\n", (tk_olt_chk_en ? "enable" : "disable") );
	}
	else
	{
		if( argv[0][0] == 'e' )
		{
			en = VOS_YES;
		}
		if( en != tk_olt_chk_en )
		{
			TkOLT_ScanPonPortStart( en );
		}
		else
			vty_out( vty, "pon state check config already %s\r\n" );
	}
	return CMD_SUCCESS;
}


DEFUN( show_pon_port_msg_stat,
    show_pon_port_msg_stat_cmd,
    "show pon-msg-stat <slot/port>",
	"show pon msg statistics\n"
	"show pon msg statistics\n"
	"input pon port index\n" )
{
    ULONG  	ulSlot = 0, ulPort = 0 ;
	short int olt_id = 0;

    IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	
	if( PonCardSlotPortCheckByVty(ulSlot, ulPort, vty) != ROK )
	{
		vty_out( vty, "  %% pon %s is not exist\r\n", argv[0] );
		return(CMD_WARNING );
	}
	if(SlotCardMayBePonBoardByVty(ulSlot, vty)  != ROK )
	{
		vty_out( vty, "  %% slot %d is not PON board\r\n", ulSlot );
		return(CMD_WARNING);
	}
    olt_id = GetPonPortIdxBySlot( (short int)ulSlot, (short int)ulPort );
    if (olt_id == VOS_ERROR)
    {
        vty_out( vty, "  %% Parameter is error.\r\n" );
        return CMD_WARNING;
    }
	
	TkOLT_ShowPonMsgCounter( vty, olt_id );
	return CMD_SUCCESS;
}

VOID ScanPonPortInit()
{
	int i, j;
	
	VOS_MemZero( tk_olt_msg_stat, sizeof(tk_olt_msg_stat) );
	for( i=0; i<=MAXPON; i++ )
	{
		tk_olt_msg_stat[i].pMsgCount = g_malloc( TK_OLT_MSG_ID_NUM * sizeof(tk_olt_msg_count_t) );
		if( tk_olt_msg_stat[i].pMsgCount == NULL )
		{
			for( j=0; j<i; j++ )
			{
				if(tk_olt_msg_stat[j].pMsgCount) g_free(tk_olt_msg_stat[j].pMsgCount);
				tk_olt_msg_stat[j].pMsgCount = NULL;
			}
			VOS_ASSERT(0);
			return;
		}
		VOS_MemZero( tk_olt_msg_stat[i].pMsgCount, TK_OLT_MSG_ID_NUM * sizeof(tk_olt_msg_count_t) );
	}

	TkOLT_ScanPonPortStart( VOS_YES );

	install_element ( DEBUG_HIDDEN_NODE, &show_pon_port_msg_stat_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &config_pon_state_check_cmd);
}
#endif	/* __SYS_DIAG_TK_PON_EN */


/* added by xieshl 20160711, 解决bcmDPC忙问题，BCM交换芯片相关补丁接口 */
#define __SYS_DIAG_BCM_PATCH
#ifdef __SYS_DIAG_BCM_PATCH
#include "src/customer/patch_bcm.h"
bcm_pat_proc_func_t *pBcmPatchDrvFuns = NULL;

int bcm_patch_ioctl( int patchid, void *para )
{
	int ret = -1;
	
	if( pBcmPatchDrvFuns == NULL )
	{
		return ret;
	}
	
	switch( patchid )
	{
		case BCM_PAT_DPC_INIT:
			if( pBcmPatchDrvFuns->dpc_init )
				ret = pBcmPatchDrvFuns->dpc_init();
			break;

		case BCM_PAT_DPC_INT_IS_ENABLED:
			if( pBcmPatchDrvFuns->dpc_intr_is_enabled )
				ret = pBcmPatchDrvFuns->dpc_intr_is_enabled();
			break;
		case BCM_PAT_DPC_INT_ENABLE:
			if( pBcmPatchDrvFuns->dpc_intr_enable )
				ret = pBcmPatchDrvFuns->dpc_intr_enable();
			break;
		case BCM_PAT_DPC_INT_DISABLE:
			if( pBcmPatchDrvFuns->dpc_intr_disable )
				ret = pBcmPatchDrvFuns->dpc_intr_disable();
			break;

		case BCM_PAT_DPC_ISR_IS_ENABLED:
			if( pBcmPatchDrvFuns->dpc_isr_is_registered )
				ret = pBcmPatchDrvFuns->dpc_isr_is_registered();
			break;
		case BCM_PAT_DPC_ISR_ENABLE:
			/* TODO */
			ret = 0;
			break;
		case BCM_PAT_DPC_ISR_DISABLE:
			if( pBcmPatchDrvFuns->dpc_isr_deregister )
				ret = pBcmPatchDrvFuns->dpc_isr_deregister();
			break;

		case BCM_PAT_DPC_CORR_IS_ENABLED:
			if( pBcmPatchDrvFuns->dpc_corr_is_enabled )
				ret = pBcmPatchDrvFuns->dpc_corr_is_enabled();
			break;
		case BCM_PAT_DPC_CORR_ENABLE:
			if( pBcmPatchDrvFuns->dpc_corr_enable )
				ret = pBcmPatchDrvFuns->dpc_corr_enable();
			break;
		case BCM_PAT_DPC_CORR_DISABLE:
			if( pBcmPatchDrvFuns->dpc_corr_disable )
				ret = pBcmPatchDrvFuns->dpc_corr_disable();
			break;

		case BCM_PAT_DPC_INT_STATE_GET:
			if( pBcmPatchDrvFuns->soc_intr_state_get )
				ret = pBcmPatchDrvFuns->soc_intr_state_get();
			break;
		case BCM_PAT_DPC_INT_MASK_GET:
			if( pBcmPatchDrvFuns->soc_intr_mask_get )
				ret = pBcmPatchDrvFuns->soc_intr_mask_get();
			break;
		case BCM_PAT_DPC_INT_STATS_SHOW:
			if( pBcmPatchDrvFuns->soc_intr_stats_show )
				ret = pBcmPatchDrvFuns->soc_intr_stats_show( para );
			break;


#ifdef BCM_PATCH_12EPON_MMU
		case BCM_PAT_12EPON_FB_MMU_INIT:
			if( pBcmPatchDrvFuns->fb_mmu_init )
				ret = pBcmPatchDrvFuns->fb_mmu_init();
			break;
		case BCM_PAT_12EPON_FB_MMU_CHECK:
			if( pBcmPatchDrvFuns->fb_mmu_check )
				ret = pBcmPatchDrvFuns->fb_mmu_check();
			break;
#endif
		default:
			break;
	}

	return ret;
}


DEFUN(show_bcm_dpc_ctrl_stat,
	show_bcm_dpc_ctrl_stat_cmd,
	"show bcm-dpc-intr {[configuration|statistics]}*1",
	"show bcm dpc patch\n"
	"show bcm dpc patch configuration\n"
	"show bcm dpc patch statistics\n"
	)
{
	uint32 mask = 0;
	int en = 0, cfg_en = 0;
	char *pEnableStr = "enable";
	char *pDisableStr = "disable";

	if( argc == 1 )
	{
		if( argv[0][0] == 'c' )
		{
			cfg_en = bcm_patch_ioctl( BCM_PAT_DPC_INT_IS_ENABLED, NULL );
			mask = (uint32)bcm_patch_ioctl( BCM_PAT_DPC_INT_MASK_GET, NULL );
			en = bcm_patch_ioctl(BCM_PAT_DPC_INT_STATE_GET, NULL );
			
			vty_out( vty, "patch bcmDPC control:\r\n" );
			vty_out( vty, " %-10s%s(%s)\r\n", " intr:", ((cfg_en == TRUE) ? pEnableStr : pDisableStr),
				((en == cfg_en) ? "valid" : "failure") );
			vty_out( vty, " %-10s%s\r\n", " isr-reg:", ((bcm_patch_ioctl(BCM_PAT_DPC_ISR_IS_ENABLED, NULL) == TRUE) ? pEnableStr : pDisableStr) );
			vty_out( vty, " %-10s%s\r\n", " err-corr:", ((bcm_patch_ioctl(BCM_PAT_DPC_CORR_IS_ENABLED, NULL) == TRUE) ? pEnableStr : pDisableStr) );
			vty_out( vty, " %-10s0x%08x\r\n", " intr-mask:", mask );
			
			return CMD_SUCCESS;
		}
	}
	
	bcm_patch_ioctl( BCM_PAT_DPC_INT_STATS_SHOW, (void*)vty );
	
	return CMD_SUCCESS;
}

DEFUN(config_bcm_dpc_ctrl,
	config_bcm_dpc_ctrl_cmd,
	"config bcm-dpc [intr|isr-reg|err-corr] [enable|disable]",
	"config bcm switch control\n"
	"config bcm dpc control\n"
	"bcm parity intr-mask\n"
	"bcm parity intr-service-routine register\n"
	"bcm parity error correction\n"
	"enable\n"
	"disable\n"
	)
{
	int en = FALSE;
	void *para = NULL;
	int ret = -1;
	
	if( argc < 2 )
	{
		return CMD_WARNING;
	}

	if( VOS_StrCmp(argv[1], "enable") == 0 )
	{
		en = TRUE;
	}

	if( VOS_StrCmp("intr", argv[0]) == 0 )
	{
		if( en )
		{
			if( bcm_patch_ioctl(BCM_PAT_DPC_ISR_IS_ENABLED, para) == 0 )
			{
				vty_out( vty, "Sorry, after isr-reg disable, it's forbidden now.\r\n" );
				return CMD_WARNING;
			}
			ret = bcm_patch_ioctl( BCM_PAT_DPC_INT_ENABLE, para );
		}
		else
		{
			ret = bcm_patch_ioctl( BCM_PAT_DPC_INT_DISABLE, para );
		}
	}
	else if( VOS_StrCmp("isr-reg", argv[0]) == 0 )
	{
		if( en )
		{
			/*if( bcm_patch_ioctl(BCM_PAT_DPC_ISR_IS_ENABLED, para) == 0 )
			{
				vty_out( vty, "Sorry, after isr-reg disable, it's not allowed enable again.\r\n" );
				return CMD_WARNING;
			}
			bcm_patch_ioctl( BCM_PAT_DPC_ISR_ENABLE, para );*/
			vty_out( vty, "Sorry, not support %s %s now!\r\n", argv[0], argv[1] );
			return CMD_WARNING;
		}
		else
		{
			ret = bcm_patch_ioctl( BCM_PAT_DPC_ISR_DISABLE, para );
		}
	}
	else if( VOS_StrCmp("err-corr", argv[0]) == 0 )
	{
		if( en )
		{
			ret = bcm_patch_ioctl( BCM_PAT_DPC_CORR_ENABLE, para );
		}
		else
		{
			ret = bcm_patch_ioctl( BCM_PAT_DPC_CORR_DISABLE, para );
		}
	}

	if( ret == -1 )
	{
		vty_out( vty, "Sorry, not support %s %s on %s\r\n", argv[0], argv[1], CardSlot_s[SYS_LOCAL_MODULE_SLOTNO] );
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

LONG bcm_patch_dpc_intr_init( void )
{
	if( bcm_patch_ioctl(BCM_PAT_DPC_INIT, NULL) != -1 )
	{
		install_element ( DEBUG_HIDDEN_NODE, &config_bcm_dpc_ctrl_cmd);
		install_element ( DEBUG_HIDDEN_NODE, &show_bcm_dpc_ctrl_stat_cmd);
	}

	return VOS_OK;
}

#if 1
/* added by xieshl 20160511, 解决旧12EPON 测速不达标问题 */
extern int bcm_fd_mmu_para_check( int unit );
extern int bcm_fb_mmu_reinit_12epon( int unit );

LONG bcm_12epon_burst_init( void )
{
	int result = 0;
	int unit = 0;
	if( SYS_LOCAL_MODULE_TYPE_IS_12EPON )
	{
		if( GetCPLDType() != CPLD_TYPE_6900_12PON_A0_EP )/*只针对BCM56514的PON板*/
		{
			return VOS_OK;
		}
		
		if( bcm_fd_mmu_para_check(unit) )/*检查是否需要重新初始化MMU参数*/
		{
			return VOS_OK;
		}

		sys_console_printf( "\r\n re-init bcm fb mmu ...\r\n" );
		result = bcm_fb_mmu_reinit_12epon( unit );/*重新初始化MMU参数，解决10G到1G突发丢包问题*/
		sys_console_printf( " reset bcm fb mmu %s\r\n\r\n", result==0 ? "OK" : "error" );
	}

	return VOS_OK;
}
#else
/* modified by xieshl 20160725, 暂不使用最新代码 */
LONG bcm_12epon_burst_init( void )
{
	int result = 0;
	if( SYS_LOCAL_MODULE_TYPE_IS_12EPON )
	{
		if( GetCPLDType() != CPLD_TYPE_6900_12PON_A0_EP )/*只针对BCM56514的PON板*/
		{
			return VOS_OK;
		}

		if( bcm_patch_ioctl(BCM_PAT_12EPON_FB_MMU_CHECK, NULL) == 1 )/*检查是否需要重新初始化MMU参数*/
		{
			return VOS_OK;
		}

		sys_console_printf( "\r\n re-init bcm fb mmu ...\r\n" );
		result = bcm_patch_ioctl( BCM_PAT_12EPON_FB_MMU_INIT, NULL );/*重新初始化MMU参数，解决10G到1G突发丢包问题*/
		sys_console_printf( " reset bcm fb mmu %s\r\n\r\n", result==0 ? "OK" : "error" );
	}

	return VOS_OK;
}
#endif
#endif	/* __SYS_DIAG_BCM_PATCH */


LONG sys_diag_showrun( struct vty * vty )
{
	/*if( SYS_PRODUCT_TYPE !=  PRODUCT_E_EPON3 )*/	/* modified by xieshl 20091019, 仅GFA6700上有效 */
		/*return VOS_OK;*/
	
	vty_out( vty, "!System diagnose config\r\n" );
	/*if( sys_diag_data.diag_enable & DIAG_ENABLE_ALL )*/
	{
		if( sys_diag_data.diag_enable & DIAG_ENABLE_PON_UPTRAFFIC )		/* 默认关闭 */
			vty_out( vty, " diagnose pon-link enable\r\n") ;
		if( (sys_diag_data.diag_enable & DIAG_ENABLE_PON_LEARNING) == 0 )	/* 默认打开 */
			vty_out( vty, " diagnose pon-learning disable\r\n") ;

		if( (sys_diag_data.diag_interval < DIAG_INTERVAL_MIN) || (sys_diag_data.diag_interval > DIAG_INTERVAL_MAX) )
			sys_diag_data.diag_interval = DIAG_INTERVAL_DEFAULT;
		if( sys_diag_data.diag_interval != DIAG_INTERVAL_DEFAULT )
		{
			vty_out( vty, " diagnose period %d\r\n", sys_diag_data.diag_interval );
		}
	}
	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}

LONG sys_diag_init()
{
	VOS_MemZero(phy_5464_reset_flag, sizeof(phy_5464_reset_flag));
	VOS_MemZero(phy_5464_reset_counter, sizeof(phy_5464_reset_counter));
	
	emac_check_init();

	if( SYS_LOCAL_MODULE_ISHAVEPP() )
	{
#ifdef __SYS_DIAG_BCM_PATCH
		bcm_patch_init( &pBcmPatchDrvFuns );
		bcm_patch_dpc_intr_init();
#endif
		bcm_12epon_burst_init();
	}

	emac0_rx_pkt_hookrtn = pktFromEmac0Parse;
	emac1_rx_pkt_hookrtn = pktFromEmac1Parse;
	bms_rx_diag_hookrtn = pktFromBmsParse;

	/*sys_diag_data.diag_slotno = 6;
	sys_diag_data.diag_portno = 1;
	sys_diag_data.diag_vid = 1;*/

	sys_diag_data.diag_enable = DIAG_ENABLE_NULL;
	sys_diag_data.diag_interval = DIAG_INTERVAL_DEFAULT;

	VOS_MemZero( sys_diag_data.cpu_rx_pkts, sizeof(sys_diag_data.cpu_rx_pkts) );
	VOS_MemZero( sys_diag_data.cpu_rx_oam, sizeof(sys_diag_data.cpu_rx_oam) );
	VOS_MemZero( sys_diag_data.pon_no_traffic, sizeof(sys_diag_data.pon_no_traffic) );
	sys_diag_data.diag_debug = 0;

	if( sys_diag_semid == 0 )
	{
		if( (sys_diag_semid = VOS_SemMCreate(VOS_SEM_Q_FIFO)) == 0 )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
	}

	if( sys_diag_queid == 0 ) 
		sys_diag_queid = VOS_QueCreate( 20 , VOS_MSG_Q_FIFO);
	if( sys_diag_queid  == 0 )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	if( VOS_OK != CDP_Create(RPU_TID_CDP_DIAG, CDP_NOTI_VIA_QUEUE, sys_diag_queid, NULL) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if( sys_diag_taskid == NULL )
	{
		sys_diag_taskid = (VOS_HANDLE)VOS_TaskCreate("tSysDiag", 251, sysDiagTask, NULL);
		if( sys_diag_taskid == NULL )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		VOS_QueBindTask( sys_diag_taskid, sys_diag_queid );
	}

	sysDiagStart( DIAG_ENABLE_PON_LEARNING );

	sys_diag_timerid = VOS_TimerCreate( MODULE_RPU_DIAG, 0, sys_diag_data.diag_interval * 1000, (void *)sysDiagTimerCallback, NULL, VOS_TIMER_LOOP );
	if( sys_diag_timerid == VOS_ERROR )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	if( SYS_PRODUCT_TYPE ==  PRODUCT_E_EPON3 )	/* modified by xieshl 20091019, 仅GFA6700上有效 */
	{
		install_element ( CONFIG_NODE, &sys_diag_enable_pon_uplink_cmd);
	}
	install_element ( CONFIG_NODE, &sys_diag_enable_pon_learning_cmd);
	install_element ( CONFIG_NODE, &sys_diag_config_show_cmd);
	install_element ( CONFIG_NODE, &sys_diag_period_config_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &sys_diag_debug_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &sys_diag_undo_debug_cmd );

#ifdef TTT_VOS_MEM_TEST
	install_element ( DEBUG_HIDDEN_NODE, &ttt_vos_memory_alloc_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &ttt_vos_memory_free_cmd);
	/*install_element ( DEBUG_HIDDEN_NODE, &ttt_report_alarm_cmd);*/
#endif
	install_element ( DEBUG_HIDDEN_NODE, &show_mgt_pkts_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &clear_mgt_pkts_cmd );

	install_element ( DEBUG_HIDDEN_NODE, &show_phy_fiber_reset_counter_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &phy_fiber_reset_enable_cmd);

    return VOS_OK;
}

void pktDataPrintf( uchar_t *pOctBuf, ulong_t bufLen )
{
	int i, j;
	char str[64];
	uchar_t cc;

	if( bufLen >= 2048 )
	{
		sys_console_printf("\r\n buffer is to long %d\r\n", bufLen );
		return;
	}
	
	j=0;
	for( i=0; i<bufLen; i++ )
	{
		cc = ((pOctBuf[i] >> 4) & 0xf);
		if( cc<=9 )
			str[j] = cc+'0';
		else if( (cc>=0x0a) && (cc<=0x0f) )
			str[j] = cc + 0x37;		/*'A' -- 'F'*/
		else
			str[j] = '-';
		j++;
		cc = (pOctBuf[i] & 0xf );
		if( cc<=9 )
			str[j] = cc+'0';
		else if( (cc>=0x0a) && (cc<=0x0f) )
			str[j] = cc + 0x37;
		else
			str[j] = '-';
		j++;
		str[j++] = ' ';

		if( j >= 60 )
		{
			str[j] = 0;
			sys_console_printf( "%s\r\n", str );
			j = 0;
		}
	}
	if( (j != 0) && (j <= 60) )
	{
		str[j] = 0;
		sys_console_printf( "%s\r\n", str );
	}
}



#undef __SYS_DIAG_TEST
#ifdef __SYS_DIAG_TEST

#define TEST_TYPE_8XEP_MSG_I2C		1
#define TEST_TYPE_8XEP_MSG_OPT		2
#define TEST_TYPE_8XEP_MSG_GPIO		3
#define TEST_TYPE_8XEP_MSG_DBA		4

int test8xepEnable = 1;	/* 0-disable, 1-enable */
int test8xepTaskNum = 1;
ULONG test8xepMsgI2cCount = 0;
ULONG test8xepMsgOptCount = 0;
ULONG test8xepMsgGpioCount = 0;
ULONG test8xepMsgDbaCount = 0;

ULONG test8xepRateSet( ULONG rate, LONG *pTxNum, LONG *pTxDelay )
{
	LONG num = 1;
	LONG delay = 100;
	if( (pTxNum == NULL) || (pTxDelay == NULL) )
		return VOS_ERROR;
	
	if( rate == 0 )
	{
		delay = -1;
		num = 100;
	}
	else if( rate >= 5000 )
	{
		delay = 1;
		num = (rate + 50) / 100;
	}
	else if( rate >= 1000 )
	{
		delay = 2;
		num = (rate + 20) / 50;
	}
	else if( rate >= 100 )
	{
		delay = 5;
		num = (rate + 10) / 20;
	}
	else if( rate >= 10 )
	{
		delay = 10;
		num = (rate + 5) / 10;
	}
	else 
	{
		delay = 100;
		num = rate;
	}

	*pTxNum = num;
	*pTxDelay = delay;
		
	return VOS_OK;
}

static VOID test8xepTask( ULONG arg1,ULONG arg2,ULONG arg3,ULONG arg4,ULONG arg5,ULONG arg6,ULONG arg7,ULONG arg8,ULONG arg9,ULONG arg10 )
{
	int i = 1;
	int mode = 0;
	unsigned short data1,data2;
	OLT_DBA_version_t ver;
	LONG olt_id = arg2;
	LONG test_type = arg3;
	LONG test_rate = 1, test_delay = 100;
	PON_rssi_result_t         rssi_result;

	short int Llid = 1;
    unsigned short numBytesToRead;
    unsigned char ucBusId;
    unsigned char ucRegAddr;
    unsigned char ucRegVal;
	ucBusId = (unsigned char)BcmChip_GetOltPonPortID(olt_id);

	test8xepRateSet(arg1, &test_rate, &test_delay);

	sys_console_printf(" Start test type=%d, ponid=%d, test_rate=%d, test_delay=%d\r\n", test_type, olt_id, test_rate, test_delay );

	while( test8xepEnable != 0 )
	{
		switch( test_type )
		{
			case TEST_TYPE_8XEP_MSG_I2C:
				for( i=0; i<test_rate; i++ )
				{
                    ucRegAddr = 127;
                    numBytesToRead = 1;
                	if ( 0 == BcmOLT_GetI2cData(olt_id, ucBusId, 100, 0xA0, 1, &ucRegAddr, &numBytesToRead, &ucRegVal) )
                	{
						ucRegVal = 0;
						numBytesToRead = 1;
						ucRegAddr = (unsigned char)102;
						BcmOLT_GetI2cData(olt_id, ucBusId, 100, 0xA0, 1, &ucRegAddr, &numBytesToRead, &ucRegVal);
                	}
					else break;
					
                    ucRegAddr = 127;
                    numBytesToRead = 1;
                	if ( 0 == BcmOLT_GetI2cData(olt_id, ucBusId, 100, 0xA0, 1, &ucRegAddr, &numBytesToRead, &ucRegVal) )
                	{
						ucRegVal = 0;
						numBytesToRead = 1;
						ucRegAddr = (unsigned char)103;
						BcmOLT_GetI2cData(olt_id, ucBusId, 100, 0xA0, 1, &ucRegAddr, &numBytesToRead, &ucRegVal);

						test8xepMsgI2cCount++;
                	}
				}
				break;
			case TEST_TYPE_8XEP_MSG_OPT:
				for( i=0; i<test_rate; i++ )
				{
					/*OLT_ReadI2CRegister(olt_id, 1, 102, &data2);
					OLT_ReadI2CRegister(olt_id, 1, 103, &data2);*/
					if( Llid == 0 )
						Llid = 1;
					else
						Llid = 0;

					OLT_GetVirtualScopeRssiMeasurement(olt_id, Llid, &rssi_result);
					VOS_TaskDelay(5);
					OLT_ReadI2CRegister(olt_id, 1, 103, &data1);
					OLT_ReadI2CRegister(olt_id, 1, 104, &data2);

					test8xepMsgOptCount++;
				}
				break;
			case TEST_TYPE_8XEP_MSG_GPIO:
				for( i=0; i<test_rate; i++ )
				{
					BcmOltGetOpticalTxMode(olt_id, &mode);
					BcmOltGetOpticalTxMode(olt_id, &mode);
					test8xepMsgGpioCount++;
				}
				break;
			case TEST_TYPE_8XEP_MSG_DBA:
				for( i=0; i<test_rate; i++ )
				{
					BcmAdapter_PAS5201_GetDBAVersion(olt_id, &ver);
					BcmAdapter_PAS5201_GetDBAVersion(olt_id, &ver);
					test8xepMsgDbaCount++;
				}
				break;
			default:
				break;
		}
		if( test_delay != -1)
			VOS_TaskDelay(test_delay);
	}

__end:
	sys_console_printf(" End test type=%d\r\n", test_type );
	test8xepTaskNum = 1;
}

VOID test8xep( ULONG pri, ULONG rate, ULONG type, ULONG olt_id )
{
	ULONG argv[10];
	CHAR taskName[12];

	VOS_MemZero( taskName, sizeof(taskName) );
	VOS_Sprintf( taskName, "t8XEP_%d", test8xepTaskNum++ );
	argv[0] = rate;
	argv[1] = olt_id;
	argv[2] = type;
	VOS_TaskCreate( taskName, pri, test8xepTask, argv );
}
#endif	/* __SYS_DIAG_TEST */

#endif	/* EPON_MODULE_SYS_DIAGNOSE */

#ifdef	__cplusplus
}
#endif/* __cplusplus */
