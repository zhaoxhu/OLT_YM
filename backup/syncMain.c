#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "sys/main/sys_main.h"

#if (RPU_MODULE_HOT ==  RPU_YES )
#include "vos/vospubh/cdp_syn.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "TdmDataSave.h"
#endif
#include "syncMain.h"

#include "cpi/cdsms/cdsms_file.h"
#include "onu/onuConfMgt.h"



const ULONG g_ulDataSyncMsgLen = 1400;	/* 同步消息长度，不包括帧头 */

ULONG g_ulDataSyncSemId = 0;			/* 同步互斥信号量 */
ULONG g_ulDataSyncFileType = 0;			/* 同步文件类型 */
static ULONG  SYNC_COUNTER=0;	   /*同步计数器add by shixh20090512*/
LONG  SynCounter_TimeID=VOS_ERROR;

ULONG g_ulDataSyncDebugSwitch = 0; 

#define SYNC_TRACE if(g_ulDataSyncDebugSwitch) sys_console_printf
#define   SynCounter_Time          10000

extern VOID device_slot_module_pull( ULONG slotno, ULONG hot_cmd );
extern VOID switchoverTimerCallback();
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern LONG print_tdm_Init();
#endif
extern LONG device_standby_master_slotno_get();
extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
extern int xflash_file_write(int fileID, unsigned char *writebuf, int *size);
extern LONG epon_device_local_board_version_get(char *boot, char *sw, char *fw, char *cpld);
/* extern void	g_free (void *__ptr); */
/* extern void *	g_malloc (size_t __size);*/
extern VOID devsm_set_hot_check_period( LONG lPeriod );
extern LONG (*cdsms_file_osimage_write_to_rom)(CHAR *,LONG *);
extern LONG ( *cdsms_file_osimage_read_from_rom ) ( CHAR *, LONG * );
extern int cl_vty_all_out( const char * format, ... );
extern LONG epon_device_slot_board_version_get(ULONG slotno, char *boot, char *sw, char *fw, char *cpld);

VOID dataSyncRpcCallback( ULONG srcSlotNo, ULONG srcModuleID,
                           VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen );
VOID dataSyncConfigProc(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSyncTestProc(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen);
VOID switchover_NotifyAck(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_ResetAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen );
VOID dataSync_NegotiationAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_RequestAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_NotifyAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_SendDataAck( eponSyncMsgHead_t *pRecvPkt, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_LoadedAck( eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_FinishedAck( eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen);
VOID dataSync_TestAck(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen);
STATUS dataSync_Reset( ULONG dst_slotno );
STATUS dataSync_SendFile( ULONG dst_slotno, syncFileType_t fileType );
STATUS dataSync_SendMsg( eponSyncFilePkt_t  *pPkt, ULONG pktLen );

char *get_sync_filename( syncFileType_t fileType );
STATUS dataSync_FlashEraseFile( syncFileType_t fileType );
LONG syncCommandInstall();
char *get_sync_filename( syncFileType_t fileType );

VOID GET_VER_Ack(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen);


static BOOL data_sync_init_flag = FALSE;
static void dataSync_SaveFile();
static saveFilePkt_t   savefile;
static char *pRecvBuffer = NULL;
VOS_HANDLE dataSync_TaskId = NULL;

/* 数据同步模块初始化函数 */
STATUS dataSyncInit()
{
#if (RPU_MODULE_HOT ==  RPU_YES )
        if ( !SYS_IS_DISTRIBUTED )
		return VOS_OK;
	if( data_sync_init_flag )
		return VOS_OK;
	data_sync_init_flag = TRUE;
	
	/* 同步CDP_SYNC通道注册*/
	if ( CDP_SYNC_Register( MODULE_RPU_EPON, dataSyncRpcCallback ) == VOS_ERROR )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}
	g_ulDataSyncFileType = 0;
	
	g_ulDataSyncSemId = VOS_SemBCreate( VOS_SEM_Q_PRIORITY, VOS_SEM_FULL );
	if( g_ulDataSyncSemId == 0 )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}

	syncCommandInstall();
	
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	print_tdm_Init();/*add by shixh@20071128*/
#endif

#endif
	return VOS_OK;
}

/* 功能: 发起数据同步，能够同步的文件包括flash中所有文件类型，只能在主GFA-SW执行 
    输入参数: vty=null表示直接通过串口打印，非0表示CLI虚拟终端
    			fileType表示同步文件类型，参见syncFileType_t定义
    返回值: VOS_OK 成功， VOS_ERROR 失败
*/
STATUS dataSyncProcess( struct vty *vty, syncFileType_t fileType )
{
	STATUS rc = VOS_ERROR;
	ULONG dst_slotno = 0;

	/* 如果本板不是主控则不能执行 */
	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_ERROR;
	/* 检查备用GFA-SW是否存在，并取出其槽位号，只有双主控时本函数才能执行 */
	if( (dst_slotno = device_standby_master_slotno_get()) == 0 )
	{
		cl_vty_all_out("\r\n standby-sw not exist \r\n");
		return rc;
	}

	/* sync_file_type_list表示正在同步的文件类型，如果不为0说明有文件同步没有结束 */
	if( g_ulDataSyncFileType != 0 )
	{
		cl_vty_all_out( " SYNC is busy, please wait or sync reset first\r\n" );
		return rc;
	}

	/* 检查该文件类型是否支持同步 */
	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:			/* TDM 配置数据 */
			/* 如果没有GFA-TDM板，则不同步TDM配置数据 */
			if( get_gfa_tdm_slotno() > 0 )
				rc = VOS_OK;
			break;
		case SYNC_FILETYPE_CFG_CTC:			/* CTC ONU 配置数据 */
			/* 如果没有启动CTC协议栈，则不同步CTC ONU配置数据 */
			if( V2R1_CTC_STACK )	
				rc = VOS_OK;
			break;
		case SYNC_FILETYPE_CFG_DATA:		/*所有SHOW RUN 中保存的数据和命令*/
			rc=VOS_OK;
			break;
		case SYNC_FILETYPE_APP_SW:			/* OLT 主控GFA-SW板APP 软件 */
			/*if(get_gfa_sw_slotno() > 0 )*/
				rc=VOS_OK;
			break;
		case SYNC_FILETYPE_BOOT_SW:		/* GFA-SW BSP */
			/*if(get_gfa_sw_slotno() > 0 )*/
				rc=VOS_OK;
			break;
		case SYNC_FILETYPE_PON_FIRM:		/* OLT  PON firmware*/
			/*if(get_gfa_epon_slotno() > 0 )*/
				rc=VOS_OK;
			break;
		case SYNC_FILETYPE_PON_DBA:               /* OLT  PON DBA */
			/*if(get_gfa_epon_slotno() > 0 )*/
				rc=VOS_OK;
			break;	
		case SYNC_FILETYPE_ONU:				/* 所有类型的ONU 软件和固件 */
			rc=VOS_OK;
			break;	
		case  SYNC_FILETYPE_SYSFILE:               /*add by shixh20090605*/
			rc=VOS_OK;
			break;
#if 0		
		case SYNC_FILETYPE_CFG_BATFILE:	       /* batfile数据 */
		case SYNC_FILETYPE_DRV_TDM,			/* OLT GFA-TDM板软件和FPGA */
		case SYNC_FILETYPE_BOOT_TDM:		/* GFA-TDM BSP */
			rc = VOS_ERROR;
			break;
#endif
		default:
			break;
	}
	if( rc == VOS_ERROR )
	{
		cl_vty_all_out( " SYNC is not supported\r\n" );
		return rc;
	}

	/* 开始同步文件 */

	SYNC_TRACE( "\r\n SYNC (%s) begin, Please wait......", get_sync_filename(fileType) );
	
	/*if( g_ulDataSyncFileType != 0 )
	{
		g_ulDataSyncFileType = 0;
	}
	VOS_TaskDelay( 30 );*/
	
	VOS_SemTake(g_ulDataSyncSemId, WAIT_FOREVER );
	dataSync_Reset( dst_slotno );		/* 确保备用主控同步状态一致 */

	g_ulDataSyncFileType = fileType;
	if( (rc = dataSync_SendFile(dst_slotno, fileType)) == VOS_ERROR )
	{
		dataSync_Reset( dst_slotno );
	}
	/*rc = dataSyncConfigBatchProcess( dst_slotno );
	if( rc == VOS_ERROR )
	{
		dataSync_Reset( dst_slotno );
	}*/
	/*g_ulDataSyncFileType = 0;*//*shixh20090323*/
	VOS_SemGive( g_ulDataSyncSemId );

	if( rc != VOS_OK )
		SYNC_TRACE( "Error!!!\r\n" );


	return rc;
}

/* 功能: 同步协议复位，主要用于确保主备同步状态一致 */
/* 输入参数: dst_slotno 备用槽位号*/
STATUS dataSync_Reset( ULONG dst_slotno )
{
	ULONG rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
	ULONG sendMsgLen, recvMsgLen;

	sendMsgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendMsg, sendMsgLen );

	pSendMsg->srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendMsg->dstSlotNo = dst_slotno;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_RESET;
	pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
	pSendMsg->msgData = 0;

	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

	g_ulDataSyncFileType = 0;

	/*if( (rc == VOS_OK) && (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvMsg );
		SYNC_TRACE( "\r\nsync reset ok" );
		return VOS_OK;
	}*/
	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync reset ok" );
		}
		else
		{
			SYNC_TRACE( "\r\nsync reset failure" );
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvMsg );
	}

	return rc;
}

/* 功能: 同步协商 ，协商同步文件类型，避免同步冲突 */
/* 输入参数: dst_slotno 备用槽位号
				 fileType 同步文件类型*/
STATUS dataSync_Negotiation( ULONG dst_slotno, syncFileType_t fileType )
{
	ULONG rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
	ULONG sendMsgLen, recvMsgLen;

	sendMsgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendMsg, sendMsgLen );

	pSendMsg->srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendMsg->dstSlotNo = dst_slotno;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_NEGOTIATION;
	pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
	pSendMsg->msgData = fileType;

	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

	/*if( (rc == VOS_OK) && (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvMsg );
		SYNC_TRACE( "\r\nsync negotiation %s ok", get_sync_filename(fileType) );
		return VOS_OK;
	}
	 if(pRecvMsg->msgResult == SYNC_MSGRESULT_NAK)
		cl_vty_all_out( "\r\n now!may be writing flash ,please wait a little!");*/

	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync negotiation %s ok", get_sync_filename(fileType) );
		}
		else
		{
			cl_vty_all_out( "\r\n now!may be writing flash ,please wait a little!");
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvMsg );
	}

	return rc;
}

/* 功能: 同步通知，通知备用准备接收数据 */
/* 输入参数: dst_slotno 备用槽位号
				 fileType 同步文件类型*/
STATUS dataSync_Notify( ULONG dst_slotno, syncFileType_t fileType )
{
	ULONG rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
	ULONG sendMsgLen, recvMsgLen;

	sendMsgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendMsg, sendMsgLen );

	pSendMsg->srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendMsg->dstSlotNo = dst_slotno;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_NOTIFY;
	pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
	pSendMsg->msgData = fileType;

	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

	/*if( (rc == VOS_OK) && (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvMsg );
		SYNC_TRACE( "\r\nsync notify %s ok", get_sync_filename(fileType) );
		return VOS_OK;
	}
	cl_vty_all_out( "\r\n sync notify %s error", get_sync_filename(fileType) );*/

	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync notify %s ok", get_sync_filename(fileType) );
		}
		else
		{
			cl_vty_all_out( "\r\n sync notify %s error", get_sync_filename(fileType) );
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvMsg );
	}

	return rc;
}

/* 功能: 同步请求，备用GFA-SW主动申请同步，暂时不支持 */
/* 输入参数: dst_slotno 主GFA-SW槽位号
				 fileType 同步文件类型*/
STATUS dataSync_Request( ULONG dst_slotno, syncFileType_t fileType )
{
	ULONG rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
	ULONG sendMsgLen, recvMsgLen;

	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_ERROR;

	sendMsgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendMsg, sendMsgLen );
	
	pSendMsg->srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendMsg->dstSlotNo = dst_slotno;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
	pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
	pSendMsg->msgData = fileType;

	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

	/*if( (rc == VOS_OK) && (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvMsg );
		SYNC_TRACE( "\r\nsync request %s ok", get_sync_filename(fileType) );
		return VOS_OK;
	}
	cl_vty_all_out( "\r\n sync request %s error", get_sync_filename(fileType) );*/

	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync request %s ok", get_sync_filename(fileType) );
		}
		else
		{
			cl_vty_all_out( "\r\n sync request %s error", get_sync_filename(fileType) );
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvMsg );
	}

	return rc;
}

/* 功能: 同步文件传输完成，通知备用GFA-SW将接收数据保存到flash */
/* 输入参数: dst_slotno 备用槽位号
				 fileType 同步文件类型
				 pktNum 发送总报文数*/
STATUS dataSync_Loaded( ULONG dst_slotno, syncFileType_t fileType, ULONG pktNum )
{
	ULONG rc;
	eponSyncFilePkt_t *pRecvPkt;
	eponSyncFilePkt_t *pSendPkt;
	ULONG sendPktLen, recvPktLen;

	sendPktLen = sizeof(eponSyncFilePkt_t);
	pSendPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( sendPktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendPkt )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendPkt, sendPktLen );

	pSendPkt->msgHead.srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendPkt->msgHead.dstSlotNo = dst_slotno;
	pSendPkt->msgHead.srcModuleID = MODULE_RPU_EPON;
	pSendPkt->msgHead.dstModuleID = MODULE_RPU_EPON;
	pSendPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
	pSendPkt->msgHead.msgCode = SYNC_MSGCODE_LOADED;
	pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_REQACK;
	pSendPkt->msgHead.msgData = fileType;

	pSendPkt->fileHead.fileType = fileType;
	pSendPkt->fileHead.totalPktCount = pktNum;
	pSendPkt->fileHead.curPktId = pktNum;
	pSendPkt->fileHead.pktDataLen = 0;
	
	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendPkt, sendPktLen, ( VOID** ) &pRecvPkt, &recvPktLen, 60 ); 
#if 0
	if( (rc == VOS_OK) && (pRecvPkt->msgHead.msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvPkt );
		g_ulDataSyncFileType=0;/*shixh20090323*/
		/*delay(100);*/
		/*sys_console_printf( "\r\ninform beiban save file to FLASH!!");*/
		return VOS_OK;
	}
	cl_vty_all_out( "\r\n sync loaded %s error\r\n", get_sync_filename(fileType) );
#endif
	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvPkt->msgHead.msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync loaded %s ok", get_sync_filename(fileType) );
			savefile.filetype = g_ulDataSyncFileType;	/* modified by xieshl 20091120, 问题单9194 */
			g_ulDataSyncFileType=0;
		}
		else
		{
			cl_vty_all_out( "\r\n sync loaded %s error\r\n", get_sync_filename(fileType) );
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvPkt );
	}

	return rc;
}

/* 功能: 同步文件全部完成，备用通知主用GFA-SW，数据保存到flash完成 */
/* 输入参数: dst_slotno 主用槽位号
				 fileType 同步文件类型 */
STATUS dataSync_Finished( ULONG dst_slotno, syncFileType_t fileType )
{
	ULONG rc;
	eponSyncFilePkt_t *pRecvPkt;
	eponSyncFilePkt_t *pSendPkt;
	ULONG sendPktLen, recvPktLen;

	sendPktLen = sizeof(eponSyncFilePkt_t);
	pSendPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( sendPktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendPkt )
    	{
        ASSERT( 0 );
        return VOS_ERROR;
   	 }
	VOS_MemZero( pSendPkt, sendPktLen );

	pSendPkt->msgHead.srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
	pSendPkt->msgHead.dstSlotNo = dst_slotno;
	pSendPkt->msgHead.srcModuleID = MODULE_RPU_EPON;
	pSendPkt->msgHead.dstModuleID = MODULE_RPU_EPON;
	pSendPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
	pSendPkt->msgHead.msgCode = SYNC_MSGCODE_FINISH;
	pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_REQACK;
	pSendPkt->msgHead.msgData = fileType;

	rc = CDP_SYNC_Call( MODULE_RPU_EPON, dst_slotno, MODULE_RPU_EPON, 0, 
							pSendPkt, sendPktLen, ( VOID** ) &pRecvPkt, &recvPktLen, 60 ); 

	/*if( (rc == VOS_OK) && (pRecvPkt->msgHead.msgResult == SYNC_MSGRESULT_ACK) )
	{
		CDP_SYNC_FreeMsg( pRecvPkt );
		g_ulDataSyncFileType=0;
		return VOS_OK;
	}
	cl_vty_all_out( "\r\n sync finished %s error\r\n", get_sync_filename(fileType) );*/

	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvPkt->msgHead.msgResult == SYNC_MSGRESULT_ACK )
		{
			SYNC_TRACE( "\r\nsync finished %s ok", get_sync_filename(fileType) );
			g_ulDataSyncFileType=0;
		}
		else
		{
			cl_vty_all_out( "\r\n sync finished %s error\r\n", get_sync_filename(fileType) );
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvPkt );
	}

	return rc;
}

/* 将文件分片发送 */
STATUS dataSync_SendData( ULONG dst_slotno, syncFileType_t fileType, char *pFileData, ULONG fileLen )
{
	int i = 0;
	ULONG pktNum;
	ULONG pktLen, pktDataLen=0;
	eponSyncFilePkt_t  *pPkt;
	char *pPktData;

	/* 分割读出的数据，逐片发送 */
	pktNum = fileLen / g_ulDataSyncMsgLen;
	if( (fileLen % g_ulDataSyncMsgLen) != 0 )
		pktNum++;

	SYNC_TRACE( "\r\nsync data send start... %s(%d)", get_sync_filename(fileType), fileLen );

	if(pktNum)
	{
        for( i=0; i<pktNum; i++ )
        {
            if( i == (pktNum - 1) )
                pktDataLen = fileLen - i * g_ulDataSyncMsgLen;
            else
                pktDataLen = g_ulDataSyncMsgLen;

            pktLen = sizeof(eponSyncMsgHead_t) + sizeof( eponSyncFileHead_t ) + pktDataLen;
            pPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
             if ( NULL == pPkt )
                {
                ASSERT( 0 );
                return VOS_ERROR;
             }
            pPkt->msgHead.srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
            pPkt->msgHead.dstSlotNo = dst_slotno;
            pPkt->msgHead.srcModuleID = MODULE_RPU_EPON;
            pPkt->msgHead.dstModuleID = MODULE_RPU_EPON;
            pPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
            pPkt->msgHead.msgCode = SYNC_MSGCODE_LOADING;
            pPkt->msgHead.msgResult = SYNC_MSGRESULT_REQACK;
            pPkt->msgHead.msgData = fileType;

            pPkt->fileHead.fileType = fileType;
            pPkt->fileHead.totalPktCount = pktNum;
            pPkt->fileHead.curPktId = i;
            pPkt->fileHead.pktDataLen = pktDataLen;

            pPktData = (char *)&pPkt->fileData;
            VOS_MemCpy( pPktData, (pFileData + i * g_ulDataSyncMsgLen), pktDataLen );

            if( dataSync_SendMsg(pPkt, pktLen) == VOS_ERROR )
            {
                SYNC_TRACE( "\r\nsend sync data %d error, pktLen=%d", i,pktDataLen );
                break;
            }
            /*SYNC_TRACE( "\r\nsend sync data %d ok, pktLen=%d", i, pktDataLen );*/
        }

        if( i != pktNum )
            return VOS_ERROR;

        /* 发送完成后，通知对端保存 */
        if( dataSync_Loaded(dst_slotno, fileType, pktNum) == VOS_ERROR )
        {
            return VOS_ERROR;
        }
        SYNC_TRACE( "\r\nsync finished %s, len=%d,pktnum=%d\r\n", get_sync_filename(fileType), fileLen, pktNum );

	}
	else /*文件长度为0，通知备用主控清除该文件*/
	{
        pktLen = sizeof(eponSyncMsgHead_t) + sizeof( eponSyncFileHead_t );
        pPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
         if ( NULL == pPkt )
            {
            ASSERT( 0 );
            return VOS_ERROR;
         }

        pPkt->msgHead.srcSlotNo = SYS_LOCAL_MODULE_SLOTNO;
        pPkt->msgHead.dstSlotNo = dst_slotno;
        pPkt->msgHead.srcModuleID = MODULE_RPU_EPON;
        pPkt->msgHead.dstModuleID = MODULE_RPU_EPON;
        pPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
        pPkt->msgHead.msgCode = SYNC_MSGCODE_LOADING;
        pPkt->msgHead.msgResult = SYNC_MSGRESULT_REQACK;
        pPkt->msgHead.msgData = fileType;

        pPkt->fileHead.fileType = fileType;
        pPkt->fileHead.totalPktCount = 0;
        pPkt->fileHead.curPktId = 0;
        pPkt->fileHead.pktDataLen = 0;

        if( dataSync_SendMsg(pPkt, pktLen) == VOS_ERROR )
        {
            SYNC_TRACE( "\r\nsend sync data %d error, pktLen=%d", i,pktDataLen );
            return VOS_ERROR;
        }
		else /*added by wangxiaoyu to reset sync file type for the next data sync process 2011-10-19*/
		{
			g_ulDataSyncFileType = 0;
		}
        /*SYNC_TRACE( "\r\nsend sync data %d ok, pktLen=%d", i, pktDataLen );*/

	}

	return VOS_OK;
}

/* 取flash文件id */
ULONG dataSync_FileLengthGet( syncFileType_t fileType )
{
	ULONG fileLen = 0;
	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:
			fileLen = TDM_FLASH_FILE_LEN;
			break;
		case SYNC_FILETYPE_CFG_CTC:
			fileLen = CFG_DATA_FLASFH_FILE_LEN;/*CTC_FLASH_FILE_LEN;*/ /*modified by wangxiaoyu 2011-10-18*/
			break;
		case SYNC_FILETYPE_CFG_DATA:		
			fileLen=CFG_DATA_FLASFH_FILE_LEN;
			break;
		case SYNC_FILETYPE_APP_SW:			
			 fileLen=APP_SW_FLASH_FILE_LEN;
			break;
		case SYNC_FILETYPE_BOOT_SW:		
			fileLen=BOOT_SW_FLASH_FILE_LEN;
			break;
		case SYNC_FILETYPE_PON_FIRM:		
			fileLen=PON_FIRM_FLASH_FILE_LEN;
			break;
		case SYNC_FILETYPE_PON_DBA:             
			fileLen=PON_DBA_FLASH_FILE_LEN;
			break;	
		case SYNC_FILETYPE_ONU:	
			fileLen=ONU_FLASH_FILE_LEN;
			break;
		case SYNC_FILETYPE_SYSFILE:          /*add by shixh20090605*/
			fileLen=SYSTERM_FILE_LEN;
			break;
		default:
			break;
	}
	return fileLen;
}

/*#define __SYNC_DEBUG*/
extern ULONG  retrieveCfgFileHead( const ULONG addr,  ULONG *length, ULONG *fileno, ULONG *onuNum );

extern LONG  (*cdsms_file_sysfile_write)(CHAR *,LONG *);
extern LONG (*cdsms_file_sysfile_read)(CHAR *,LONG *);
extern LONG (*cdsms_file_sysfile_erase)(VOID);
/* 从flash中读出待同步的文件 */
STATUS dataSync_FileDataRead( syncFileType_t fileType, UCHAR *pFileData, ULONG *pFileLen )
{
	ULONG fileLen = *pFileLen;
	ULONG len/*, ver, num*/;
	STATUS rc = VOS_ERROR;
	/*char  a[40]={0};*/
	if( (pFileData == NULL) || (pFileLen == NULL) )
	{
		VOS_ASSERT(0);
		return rc;
	}
#ifndef __SYNC_DEBUG

	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
			if( (rc = xflash_file_read(TDM_FLASH_FILE_ID, pFileData, (int *)&fileLen)) == VOS_OK )
			{
				if( VOS_StrCmp(((nvm_tdmNvmDataHead_t *)pFileData)->nvmFlag, "TDM") != 0 )
				{
					return VOS_OK;
				}
				fileLen = ((nvm_tdmNvmDataHead_t *)pFileData)->nvmDataSize + sizeof(nvm_tdmNvmDataHead_t);
				if( fileLen > TDM_FLASH_FILE_LEN )
				{
					ASSERT( 0 );
					rc = VOS_ERROR;
				}
			}
#endif
			break;
			
		case SYNC_FILETYPE_CFG_CTC:
#if 0
			if( (rc = xflash_file_read(CTC_FLASH_FILE_ID, pFileData, (int *)&fileLen)) == VOS_OK )
			{
				retrieveCfgFileHead( (ULONG)pFileData, &len, &ver, &num );
				/*if( (len == 0) || (len > CTC_FLASH_FILE_LEN) || (num == 0) || (num > 1280) )*/
				if(  (len > CTC_FLASH_FILE_LEN) ||  (num > 1280) )
				{
					rc = VOS_ERROR;
				}
				fileLen = len;
			}
#else
			if(cdsms_file_onu_conf_read)
			{
			    rc = (*cdsms_file_onu_conf_read)(ONU_CONF_SAVE_FILE_HEAD, pFileData, &fileLen);
				/*added by wangxiaoyu for sync delete ctc onu config file on the standby master*/
				if(rc != ROK)
				{
					rc = VOS_OK;
					fileLen = 0;
				}
			}
			else
			    rc = VOS_ERROR;
#endif
			
			break;
	       case SYNC_FILETYPE_CFG_DATA:
		   	rc = xflash_file_read(CFG_DATA_FLASFH_FILE_ID, pFileData,(int *)&fileLen);
			break;
		case SYNC_FILETYPE_APP_SW:			
			rc = xflash_file_read(APP_SW_FLASH_FILE_ID, pFileData, (int *)&fileLen);
			break;
		case SYNC_FILETYPE_BOOT_SW:		
			/*rc = xflash_file_read(BOOT_SW_FLASH_FILE_ID, pFileData, (int *)&fileLen);*/
			if( cdsms_file_osimage_read_from_rom )
				rc = ( *cdsms_file_osimage_read_from_rom )( pFileData, (LONG*)&len);

			break;
		case SYNC_FILETYPE_PON_FIRM:		
			rc = xflash_file_read(PON_FIRM_FLASH_FILE_ID, pFileData, (int *)&fileLen);
			break;
		case SYNC_FILETYPE_PON_DBA:             
			rc = xflash_file_read(PON_DBA_FLASH_FILE_ID, pFileData, (int *)&fileLen);
			break;	
		case SYNC_FILETYPE_ONU:	
			rc = xflash_file_read(ONU_FLASH_FILE_ID, pFileData,(int *)&fileLen);
			break;
		case  SYNC_FILETYPE_SYSFILE:
			if(cdsms_file_sysfile_read)
			rc=(*cdsms_file_sysfile_read)(pFileData, (LONG*)&fileLen);
			break;
		default:
			break;
	}
#else
	fileLen = *pFileLen - 100;
	VOS_MemSet( pFileData, 0x11, fileLen );
#endif

	*pFileLen = fileLen;

	return rc;
}

/* 功能：同步文件，将文件发送到备用GFA-SW */
/* 输入参数: dst_slotno 备用槽位号
				 fileType 同步文件类型 */
STATUS dataSync_SendFile( ULONG dst_slotno, syncFileType_t fileType )
{
	ULONG fileLen, maxFileLen;
	char *pFileData = NULL;

	maxFileLen = dataSync_FileLengthGet( fileType );
	if( maxFileLen == 0 )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}

	/*pFileData = (UCHAR*)g_malloc( maxFileLen );
	if( pFileData == NULL )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}*/

	if( dataSync_Negotiation(dst_slotno, fileType) == VOS_OK )
	{
		
		if( dataSync_Notify( dst_slotno, fileType) == VOS_OK )
		{
			pFileData = (UCHAR*)g_malloc( maxFileLen );
			if( pFileData == NULL )
			{
				ASSERT( 0 );
				return VOS_ERROR;
			}
			fileLen = maxFileLen;
			if( dataSync_FileDataRead(fileType, pFileData, &fileLen) == VOS_OK )
			{
				if( fileLen > maxFileLen )
				{
					VOS_ASSERT( 0 );
				}
				/* 分割读出的数据，逐片发送 */
				else
				{
					if( dataSync_SendData(dst_slotno, fileType, pFileData, fileLen) == VOS_OK )
					{
						g_free( pFileData );
						return VOS_OK;
					}
				}
			}
		}
	}
	if(pFileData!=NULL)
	g_free( pFileData );

	return VOS_ERROR;
}

/* 功能：发数据同步消息*/
/* 输入参数: pPkt 发送消息
				 pktLen 发送消息长度 */
STATUS dataSync_SendMsg( eponSyncFilePkt_t  *pPkt, ULONG pktLen )
{
	STATUS rc;
	eponSyncMsgHead_t *pRecvMsg;
	ULONG recvMsgLen;

	rc = CDP_SYNC_Call( pPkt->msgHead.dstModuleID, pPkt->msgHead.dstSlotNo, MODULE_RPU_EPON, 0, 
							(VOID *)pPkt, pktLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

	if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
	{
		if( pRecvMsg->msgResult != SYNC_MSGRESULT_ACK )
		{
			rc = VOS_ERROR;
		}
		CDP_SYNC_FreeMsg( pRecvMsg );
	}
	return rc;
}

/* 功能：接收并组装同步数据，并写入flash */
/* 输入参数:  pPkt 接收消息*/
STATUS dataSync_RecvMsg( eponSyncFilePkt_t  *pPkt )
{
	static ULONG bufferOffset = 0;
	/*char a[50]={0};
	char b[50]={0};*/
	ULONG bufferLength[SYNC_FILETYPE_MAX] = {0, TDM_FLASH_FILE_LEN, CTC_FLASH_FILE_LEN, CFG_DATA_FLASFH_FILE_LEN,APP_SW_FLASH_FILE_LEN,BOOT_SW_FLASH_FILE_LEN,PON_FIRM_FLASH_FILE_LEN,PON_DBA_FLASH_FILE_LEN,ONU_FLASH_FILE_LEN,0,0,0,SYSTERM_FILE_LEN};

	if( pPkt->msgHead.msgCode == SYNC_MSGCODE_RESET )	/* 复位 时释放内存资源 */
	{
		if( dataSync_TaskId )		/* modified by xieshl 20120428, 禁止数据在写flash过程中被复位 */
		{
			VOS_ASSERT(0);
		}
		else
		{
			bufferOffset = 0;
			if( pRecvBuffer )
			{
				g_free( pRecvBuffer );
				pRecvBuffer = NULL;
			}
		}
	}
	else
	{
		if( (g_ulDataSyncFileType != pPkt->fileHead.fileType) ||
			(pPkt->fileHead.fileType >= SYNC_FILETYPE_MAX) ||
			(pPkt->fileHead.fileType == 0) )
		{
			ASSERT( 0 );
			return VOS_ERROR;
		}
	
		if( pPkt->msgHead.msgCode == SYNC_MSGCODE_LOADING )	/* 接收同步数据 */
		{
			if( (pPkt->fileHead.curPktId != 0) && (pRecvBuffer == NULL) )
			{
				ASSERT( 0 );
				return VOS_ERROR;
			}

			/* 如果总帧数为0，说明主GFA-SW无数据，备GFA-SW也应清除数据 */
			if( pPkt->fileHead.totalPktCount == 0 )
			{
				/*add by wangxiaoyu 2011-10-19 for data sync for erase*/
				g_ulDataSyncFileType = 0;
				return dataSync_FlashEraseFile( pPkt->fileHead.fileType );
			}

			/* 分配接收缓冲内存 */
			if( pRecvBuffer == NULL )
			{
				pRecvBuffer = (UCHAR*)g_malloc( bufferLength[g_ulDataSyncFileType] );
				if( pRecvBuffer == NULL )
				{
					ASSERT( 0 );
					return VOS_ERROR;
				}
				bufferOffset = 0;
			}

			/* 发送和接收都是严格按顺序进行的，不存在接收颠倒问题，
			    curPktId＝0表示第一帧，数据从头开始缓存 */
			if( pPkt->fileHead.curPktId == 0 ) 
			{
				VOS_MemZero( pRecvBuffer, bufferLength[g_ulDataSyncFileType] );
				bufferOffset = 0;
				SYNC_TRACE( "\r\nrecv sync data start... %s(%d)", get_sync_filename(pPkt->fileHead.fileType), pPkt->fileHead.pktDataLen );
			}
			if( (bufferOffset + pPkt->fileHead.pktDataLen) > bufferLength[g_ulDataSyncFileType] )
			{
				VOS_ASSERT( 0 );
				goto sync_recv_err;
			}
			/* 缓存接收数据 */
			VOS_MemCpy( pRecvBuffer + bufferOffset, &pPkt->fileData, pPkt->fileHead.pktDataLen );
			bufferOffset += pPkt->fileHead.pktDataLen;
			
		}
		else if( pPkt->msgHead.msgCode == SYNC_MSGCODE_LOADED )	/* 保存同步数据 */
		{
			if( pRecvBuffer == NULL )
			{
				VOS_ASSERT( 0 );
				goto sync_recv_err;
			}
			if( bufferOffset == 0 )
			{
				VOS_ASSERT( 0 );
				goto sync_recv_err;
			}
			/* 保存接收数据 */
			savefile.buffer=pRecvBuffer;
			savefile.len=bufferOffset;
			savefile.filetype=pPkt->fileHead.fileType;
			/*sys_console_printf("len is=%d,pPkt->fileHead.fileType=%d\r\n",bufferOffset,pPkt->fileHead.fileType);
			VOS_StrnCat(a, savefile.buffer,40);
			sys_console_printf("3:the first 40 byte is%s\r\n",a);*/
#if 1
			dataSync_TaskId = (VOS_HANDLE)VOS_TaskCreate("tSaveFile", 165, dataSync_SaveFile, NULL);
			pRecvBuffer = NULL;
			bufferOffset = 0;
#else
			if( dataSync_FlashSaveFile(pRecvBuffer, bufferOffset, pPkt->fileHead.fileType) == VOS_ERROR )
				goto sync_recv_err;
			else
			{
				free( pRecvBuffer );
				pRecvBuffer = NULL;
				bufferOffset = 0;
			}
#endif

		}
		else
		{
			VOS_ASSERT( 0 );
			goto sync_recv_err;
		}
	}
	return VOS_OK;
	
sync_recv_err:

	if( pRecvBuffer )
	{
		g_free( pRecvBuffer );
		pRecvBuffer = NULL;
	}
	bufferOffset = 0;
	return VOS_ERROR;
}

/* 功能：将接收数据保存flash，备GFA-SW执行。接收数据同步文件结束后，应创建flash保存任务
		      目前暂时放在SYNC-CDP任务中执行
    输入参数: pBuffer 文件存储缓冲区
				length 文件长度
				fileType 文件类型*/
STATUS dataSync_FlashSaveFile( char *pBuffer, ULONG length, syncFileType_t fileType )
{
	STATUS rc;
	int fileId = 0;
	ULONG len = length;
	/*int i;
	char a[50]={0};*/
	SYNC_TRACE("\r\nsync-data save %s(%d) ok\r\n", get_sync_filename(fileType), length );

	rc = VOS_OK;
	
	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
			fileId = TDM_FLASH_FILE_ID;
#else
			return VOS_ERROR;
#endif
			break;
		case SYNC_FILETYPE_CFG_CTC:
			fileId = CTC_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_CFG_DATA:		
			fileId=CFG_DATA_FLASFH_FILE_ID;
			break;
		case SYNC_FILETYPE_APP_SW:			
			 fileId=APP_SW_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_BOOT_SW:		
			fileId=BOOT_SW_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_PON_FIRM:		
			fileId=PON_FIRM_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_PON_DBA:             
			fileId=PON_DBA_FLASH_FILE_ID;
			break;	
		case SYNC_FILETYPE_ONU:	
			fileId=ONU_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_SYSFILE:	
			/*fileId=SYSFILE_FILE_ID;*/
			break;	
		default:
			rc = VOS_ERROR;
			break;
	}
	
	if( rc == VOS_OK )
	{
		V2R1_disable_watchdog();
		if( fileType == SYNC_FILETYPE_SYSFILE)
			{
			int lock_id = VOS_IntLock();
			VOS_TaskLock();
			if( cdsms_file_sysfile_write )
			{
				rc = ( *cdsms_file_sysfile_write )( pBuffer, (LONG*)&len);
			}
			VOS_TaskUnlock();
			VOS_IntUnlock(lock_id);
			}
		else if( fileId == BOOT_SW_FLASH_FILE_ID )
		{
			int lock_id = VOS_IntLock();
			VOS_TaskLock();
			if( cdsms_file_osimage_write_to_rom )
			{
				rc = ( *cdsms_file_osimage_write_to_rom )( pBuffer, (LONG*)&len);
			}
			VOS_TaskUnlock();
			VOS_IntUnlock(lock_id);
		}
		else if(fileId == CTC_FLASH_FILE_ID)
		{
            int lock_id = VOS_IntLock();
            VOS_TaskLock();
            if( cdsms_file_onu_conf_write )
            {
                rc = ( *cdsms_file_onu_conf_write )("/flash/sys/onuconf.data", pBuffer, (LONG*)&len);
            }
            VOS_TaskUnlock();
            VOS_IntUnlock(lock_id);
		}
		else
		{
			rc = xflash_file_write( fileId, pBuffer, (int*)&len );
		}
		V2R1_enable_watchdog();
	}

	if(g_ulDataSyncDebugSwitch)
		sys_console_printf( "\r\n SYNC (%s) begin......end\r\n", get_sync_filename(fileType) );
	else		
		cl_vty_all_out( "end\r\n" );

	return rc;
}
static void dataSync_SaveFile()
{
	ULONG fileType = g_ulDataSyncFileType;
	if(dataSync_FlashSaveFile(savefile.buffer, savefile.len, savefile.filetype)==VOS_OK)
	{
		if(savefile.buffer )
		{
			g_free(savefile.buffer);
			/*Malloc_count--;
			sys_console_printf( "file type:%d,show free rom is:%d\r\n",g_ulDataSyncFileType,Malloc_count);*/
		}
		savefile.buffer = NULL;
		savefile.len = 0;

		dataSync_Finished( SYS_MASTER_ACTIVE_SLOTNO, fileType );

		g_ulDataSyncFileType=0;/*test shixh20090323*/
	}
	dataSync_TaskId = NULL;
	return ;
}

/* 功能: 从flash擦除指定文件*/
/* 输入参数: fileType  文件类型 */
STATUS dataSync_FlashEraseFile( syncFileType_t fileType )
{
	STATUS rc;
	int fileId = 0;
	char *pBuf;
	int len = 0x1000;

	SYNC_TRACE("\r\n sync-data erase %s(%d) ok", get_sync_filename(fileType) );

	pBuf = (char *)g_malloc( len );
	if( pBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero( pBuf, len );

	rc = VOS_OK;
	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
			fileId = TDM_FLASH_FILE_ID;
#else
             /* aaaaaa 20141120  应该free( pBuf ); */
			/*VOS_ASSERT(0);
			return VOS_ERROR;*/
			rc = VOS_ERROR;
#endif
			break;
		case SYNC_FILETYPE_CFG_CTC:
			fileId = CTC_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_CFG_DATA:		
			fileId=CFG_DATA_FLASFH_FILE_ID;
			break;
		case SYNC_FILETYPE_APP_SW:			
			 fileId=APP_SW_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_BOOT_SW:		
			fileId=BOOT_SW_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_PON_FIRM:		
			fileId=PON_FIRM_FLASH_FILE_ID;
			break;
		case SYNC_FILETYPE_PON_DBA:             
			fileId=PON_DBA_FLASH_FILE_ID;
			break;	
		case SYNC_FILETYPE_ONU:	
			fileId=ONU_FLASH_FILE_ID;
			break;
		/*case SYNC_FILETYPE_SYSFILE:	
			fileId=SYSFILE_FILE_ID;
			break;*/
		default:
			rc = VOS_ERROR;
			break;
	}
	if( rc == VOS_OK )
	{
	    if(fileId == CTC_FLASH_FILE_ID)
	    {
            int lock_id = VOS_IntLock();
            VOS_TaskLock();
            if( cdsms_file_onu_conf_erase )
            {
                rc = ( *cdsms_file_onu_conf_erase )("/flash/sys/onuconf.data");
            }
            VOS_TaskUnlock();
            VOS_IntUnlock(lock_id);
	    }
	    else
	    {
            V2R1_disable_watchdog();
            rc = xflash_file_write( fileId, pBuf, &len );
            V2R1_enable_watchdog();
	    }
	}

	g_free( pBuf );
	
	return rc;
}


static void  counter_count_10s()
{
    /*modified by luh 2012-10-26, 问题单15992*/
	if(SYNC_COUNTER>15/*9*/)
	{
		SYNC_TRACE("\r\nsync file %s(%d) timeout\r\n", get_sync_filename(g_ulDataSyncFileType), g_ulDataSyncFileType );
		if( dataSync_TaskId )		/* modified by xieshl 20120428, flash保存没有完成之前是不能释放内存的*/
		{
			/*VOS_TaskDelete( dataSync_TaskId );
			dataSync_TaskId = NULL;*/
			VOS_ASSERT(0);
		}
		if( (dataSync_TaskId == NULL) || (SYNC_COUNTER > 18) )
		{
			g_ulDataSyncFileType=0;
			if(pRecvBuffer)
			{
				g_free(pRecvBuffer);
				pRecvBuffer = NULL;
			}
			SYNC_COUNTER=0;
			VOS_TimerDelete(MODULE_RPU_EPON,SynCounter_TimeID);
		}
	}
	else
		SYNC_COUNTER++;
	return;
}
/* 功能: CDP_SYNC通道RPC回调函数*/
/* 输入参数: srcSlotNo 本板槽位号
				srcModuleID 模块号
				pRecvData  接收数据
				recvDataLen 接收数据长度 */
/* 输出参数: ppSendData 发送数据
				pSendDataLen 发送数据长度*/
VOID dataSyncRpcCallback( ULONG srcSlotNo, ULONG srcModuleID,
                           VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen )
{
	eponSyncMsgHead_t *pRecvMsg/*, *pSendMsg*/;

	if( pRecvData == NULL )
	{
		ASSERT( 0 );
		return;
	}
	if( recvDataLen < sizeof(eponSyncMsgHead_t) )
	{
		ASSERT( 0 );
		return;
	}

	pRecvMsg = (eponSyncMsgHead_t *)pRecvData;
	
	if( pRecvMsg->msgType == SYNC_MSGTYPE_CONFIG )
	{
		switch( pRecvMsg->msgCode )
		{
			case SYNC_MSGCODE_NEGOTIATION:		/* 同步协商*/
				dataSync_NegotiationAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			case SYNC_MSGCODE_REQUEST:			/* 同步请求*/
				dataSync_RequestAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			case SYNC_MSGCODE_NOTIFY:				/* 同步通知*/
				dataSync_NotifyAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			case SYNC_MSGCODE_LOADING:			/* 数据传输*/
				dataSync_SendDataAck( pRecvMsg, ppSendData, pSendDataLen );
				break;				
			case SYNC_MSGCODE_LOADED:				/* 数据传输结束*/
				dataSync_LoadedAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			case SYNC_MSGCODE_FINISH:				/* 同步完成*/
				dataSync_FinishedAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			case SYNC_MSGCODE_RESET:				/* 同步复位*/
				dataSync_ResetAck( pRecvMsg, ppSendData, pSendDataLen );
				break;
			default:
				break;
		}
	}
	else if( pRecvMsg->msgType == SYNC_MSGTYPE_TEST )
	{
		dataSync_TestAck( pRecvData,  recvDataLen, ppSendData, pSendDataLen );
	}
	else if( pRecvMsg->msgType == SYNC_MSGTYPE_SWITCHOVER )
	{
		switchover_NotifyAck( pRecvData,  recvDataLen, ppSendData, pSendDataLen );
	}
	else if( pRecvMsg->msgType == SYNC_MSGTYPE_GET_VER )
	{
		GET_VER_Ack( pRecvData,  recvDataLen, ppSendData, pSendDataLen );/*add by shixh20080418*/
	}
	else
	{
		SYNC_TRACE(" SYNC MSG error %d\r\n", pRecvMsg->msgType );
	}
}

/* 功能: 同步复位处理确认*/
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_ResetAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen )
{
	eponSyncMsgHead_t *pSendMsg;
	ULONG msgLen;

	msgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( msgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, msgLen );
				
	pSendMsg->srcSlotNo = pRecvMsg->dstSlotNo;
	pSendMsg->dstSlotNo = pRecvMsg->srcSlotNo;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_RESET;
	
	/*g_ulDataSyncFileType = 0;*//*shixh20090323*/

	/* 复位时需要释放备GFA-SW已申请的内存空间 */
	if( dataSync_RecvMsg( (eponSyncFilePkt_t *)pRecvMsg ) == VOS_OK )
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_ACK;
		SYNC_TRACE( "sync reset ok\r\n" );
	}
	else
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "sync reset error\r\n" );
	}
	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = msgLen;
}

/* 功能: 同步协商确认 */
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_NegotiationAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncMsgHead_t *pSendMsg;
	ULONG msgLen;

	msgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( msgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, msgLen );
				
	pSendMsg->srcSlotNo = pRecvMsg->dstSlotNo;
	pSendMsg->dstSlotNo = pRecvMsg->srcSlotNo;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_NEGOTIATION;
	if( (pRecvMsg->msgData != 0) && (g_ulDataSyncFileType == 0) )
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_ACK;		/* 这里还应检查flash状态 */
		g_ulDataSyncFileType = pRecvMsg->msgData;
		/*创建一个定时器*//*add by shixh20090512*/
		 if( SynCounter_TimeID == VOS_ERROR )
		{
			SynCounter_TimeID = VOS_TimerCreate(MODULE_DEVICE_MANAGE, 0, SynCounter_Time, (VOID *)counter_count_10s, NULL, VOS_TIMER_LOOP);
			if( SynCounter_TimeID == VOS_ERROR )
			{
				VOS_ASSERT( 0 );
				return;
			}
		}
		/*end 20090512*/
		SYNC_COUNTER=0;
		SYNC_TRACE( "sync negotiation %s ack\r\n", get_sync_filename(g_ulDataSyncFileType) );

		sys_console_printf( "SYNC (%s) begin......", get_sync_filename(g_ulDataSyncFileType) );
	}
	else
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "sync negotiation %s nak\r\n", get_sync_filename(g_ulDataSyncFileType) );
	}
	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = msgLen;
}

/* 功能: 同步请求确认 */
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_RequestAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncMsgHead_t *pSendMsg;
	ULONG msgLen;

	msgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( msgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, msgLen );
	
	pSendMsg->srcSlotNo = pRecvMsg->dstSlotNo;
	pSendMsg->dstSlotNo = pRecvMsg->srcSlotNo;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
	if( (pRecvMsg->msgData & g_ulDataSyncFileType) != 0 )
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_ACK;		/* 这里还应检查flash状态 */
		SYNC_TRACE( "\r\nsync request %s ack", get_sync_filename(g_ulDataSyncFileType) );
	}
	else
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "\r\nsync request %s nak", get_sync_filename(g_ulDataSyncFileType) );
	}
	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = msgLen;
}

/* 功能: 同步通知确认*/
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_NotifyAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncMsgHead_t *pSendMsg;
	ULONG msgLen;

	msgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( msgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, msgLen );
	
	pSendMsg->srcSlotNo = pRecvMsg->dstSlotNo;
	pSendMsg->dstSlotNo = pRecvMsg->srcSlotNo;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_NOTIFY;
	if( (pRecvMsg->msgData & g_ulDataSyncFileType) != 0 )
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_ACK;		/* 这里还应检查flash状态 */
		SYNC_TRACE( "\r\nsync notify %s ack", get_sync_filename(g_ulDataSyncFileType) );
	}
	else
	{
		pSendMsg->msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "\r\nsync notify %s nak", get_sync_filename(g_ulDataSyncFileType) );
	}
	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = msgLen;
}

/* 功能: 文件同步确认*/
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_SendDataAck( eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncFilePkt_t *pRecvPkt;
	eponSyncFilePkt_t *pSendPkt;
	ULONG pktLen;

	pktLen = sizeof(eponSyncFilePkt_t);
	pSendPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendPkt )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendPkt, pktLen );
	
	pSendPkt->msgHead.srcSlotNo = pRecvMsg->dstSlotNo;
	pSendPkt->msgHead.dstSlotNo = pRecvMsg->srcSlotNo;
	pSendPkt->msgHead.srcModuleID = pRecvMsg->dstSlotNo;
	pSendPkt->msgHead.dstModuleID = pRecvMsg->srcSlotNo;
	pSendPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
	pSendPkt->msgHead.msgCode = SYNC_MSGCODE_LOADING;
	pSendPkt->msgHead.msgData = g_ulDataSyncFileType;

	pRecvPkt = (eponSyncFilePkt_t *)pRecvMsg;

	if( dataSync_RecvMsg(pRecvPkt) == VOS_OK )
	{
		SYNC_COUNTER=0;
		pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_ACK;
		/*SYNC_TRACE( "\r\nrecv sync data %d ok, pktLen=%d", pRecvPkt->fileHead.curPktId, pRecvPkt->fileHead.pktDataLen );*/
	}
	else
	{
		pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "\r\nrecv sync data %d error, pktLen=%d", pRecvPkt->fileHead.curPktId, pRecvPkt->fileHead.pktDataLen );
	}
	
	*ppSendData = (VOID *)pSendPkt;
	*pSendDataLen = pktLen;
}

/* 功能: 文件同步完成应答*/
/* 输入参数:  pRecvMsg 接收消息*/
/* 输出参数:  ppSendData 发送消息
			  	 pSendDataLen 发送消息长度*/
VOID dataSync_LoadedAck( eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncFilePkt_t *pRecvPkt;
	eponSyncFilePkt_t *pSendPkt;
	ULONG pktLen;

	pktLen = sizeof(eponSyncFilePkt_t);
	pSendPkt = (eponSyncFilePkt_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendPkt )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendPkt, pktLen );
	
	pSendPkt->msgHead.srcSlotNo = pRecvMsg->dstSlotNo;
	pSendPkt->msgHead.dstSlotNo = pRecvMsg->srcSlotNo;
	pSendPkt->msgHead.srcModuleID = pRecvMsg->dstSlotNo;
	pSendPkt->msgHead.dstModuleID = pRecvMsg->srcSlotNo;
	pSendPkt->msgHead.msgType = SYNC_MSGTYPE_CONFIG;
	pSendPkt->msgHead.msgCode = SYNC_MSGCODE_LOADED;
	pSendPkt->msgHead.msgData = g_ulDataSyncFileType;

	pRecvPkt = (eponSyncFilePkt_t *)pRecvMsg;

	if( dataSync_RecvMsg(pRecvPkt) == VOS_OK )
	{
		/*g_ulDataSyncFileType = 0;*/
		SYNC_COUNTER=0;/*add by shixh20090512*/
		pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_ACK;
		SYNC_TRACE( "\r\nrecv sync data finished %d ok", pRecvPkt->fileHead.curPktId );
	}
	else
	{
		pSendPkt->msgHead.msgResult = SYNC_MSGRESULT_NAK;
		SYNC_TRACE( "\r\nrecv sync data finished %d error", pRecvPkt->fileHead.curPktId );
	}
	
	*ppSendData = (VOID *)pSendPkt;
	*pSendDataLen = pktLen;
}

VOID dataSync_FinishedAck(eponSyncMsgHead_t *pRecvMsg, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncMsgHead_t *pSendMsg;
	ULONG msgLen;

	/*SYNC_COUNTER = 7;	*/

	msgLen = sizeof(eponSyncMsgHead_t);
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( msgLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, msgLen );
	
	pSendMsg->srcSlotNo = pRecvMsg->dstSlotNo;
	pSendMsg->dstSlotNo = pRecvMsg->srcSlotNo;
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_CONFIG;
	pSendMsg->msgCode = SYNC_MSGCODE_FINISH;

	pSendMsg->msgResult = SYNC_MSGRESULT_ACK;		/* 这里还应检查flash状态 */
	SYNC_TRACE( "\r\nsync(%d) finished ack", savefile.filetype/*g_ulDataSyncFileType*/ );

	SYNC_TRACE( " %s sync finished!\r\n", get_sync_filename(savefile.filetype) );	/* modified by xieshl 20091120, 问题单9194 */

	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = msgLen;
}


/*add by shixh@20080418*/
VOID GET_VER_Ack(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen)
{
	char boot[MAXBOOTVERSIONLEN], sw[MAXSWVERSIONLEN], fw[MAXFWVERSIONLEN], 
		cpld[MAXFWVERSIONLEN],fpga[MAXFWVERSIONLEN];
	int len;
	eponSyncMsgHead_t *pSendMsg;
	ULONG pktLen;
	char *pTxData;
	char *pRxData = (char*)pRecvData;

	pktLen = sizeof(eponSyncMsgHead_t) + 100;
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, pktLen );
	VOS_MemZero(fpga, MAXFWVERSIONLEN);
	
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_GET_VER;
	pSendMsg->msgCode = SYNC_MSGCODE_NOTIFY;
	pSendMsg->msgResult = SYNC_MSGRESULT_ACK;
	pSendMsg->msgData = 0;

	epon_device_local_board_version_get(boot, sw, fw, cpld);
	
	if(fpga)
	getLocalFPGAVersion( fpga );
	
	pTxData = (char *)(pSendMsg + 1);
	len = VOS_StrLen(boot);
	pTxData[0] = len;
	VOS_MemCpy(&pTxData[1], boot,len);
	pTxData += (len+1);
	
	len = VOS_StrLen(sw);
	pTxData[0] = len;
	VOS_MemCpy(&pTxData[1], sw,len);
	pTxData += (len+1);
	
	len = VOS_StrLen(fw);
	pTxData[0] = len;
	VOS_MemCpy(&pTxData[1], fw,len);
	pTxData += (len+1);

	len = VOS_StrLen(cpld);
	pTxData[0] = len;
	VOS_MemCpy(&pTxData[1], cpld,len);
	pTxData += (len+1);

	len = VOS_StrLen(fpga);
	pTxData[0] = len;
	VOS_MemCpy(&pTxData[1], fpga,len);

	
	/*pTxData = (char *)(pSendMsg + 1);
	len = VOS_StrLen(boot);
	pTxData[0] = len;
	VOS_StrCpy(&pTxData[1], boot);
	pTxData += (len+1);
	
	len = VOS_StrLen(sw);
	pTxData[0] = len;
	VOS_StrCpy(&pTxData[1], sw);
	pTxData += (len+1);
	
	len = VOS_StrLen(fw);
	pTxData[0] = len;
	VOS_StrCpy(&pTxData[1], fw);*/

	/*pTxData = (char *)(pSendMsg + 1);
	
	
	VOS_StrCpy(pTxData, "shixiaohui");
	pTxData += strlen("shixiaohui");
	

	
	VOS_StrCpy(pTxData, "GFA6700");
	pTxData +=strlen("GFA6700");
	
	VOS_StrCpy(pTxData, "12.34.56.78");*/

	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = pktLen;
	/*sys_console_printf("\r\n RECV:%s, SEND:%s\r\n", pRxData, pTxData);*/
	SYNC_TRACE("\r\n RECV:%s, SEND:%s", pRxData, pTxData);
}/* 功能:测试消息处理 */
/* 输入参数: pRecvData  接收数据
				recvDataLen 接收数据长度 */
/* 输出参数: ppSendData 发送数据
				pSendDataLen 发送数据长度*/
VOID dataSync_TestAck(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen)
{
	/*eponSyncMsgHead_t *pRecvMsg;*/
	eponSyncMsgHead_t *pSendMsg;
	eponSyncFileHead_t *pSyncFile;
	ULONG pktLen;
	char *pTxData;
	char *pRxData;

	pktLen = sizeof(eponSyncMsgHead_t) + sizeof(eponSyncFileHead_t) + 100;
	pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( pktLen, MODULE_RPU_EPON );
	 if ( NULL == pSendMsg )
    	{
        ASSERT( 0 );
        return ;
   	 }
	VOS_MemZero( pSendMsg, pktLen );
	
	pSendMsg->srcModuleID = MODULE_RPU_EPON;
	pSendMsg->dstModuleID = MODULE_RPU_EPON;
	pSendMsg->msgType = SYNC_MSGTYPE_TEST;
	pSendMsg->msgCode = SYNC_MSGCODE_NOTIFY;
	pSendMsg->msgResult = SYNC_MSGRESULT_ACK;
	pSendMsg->msgData = SYNC_FILETYPE_CFG_TDM;

	pSyncFile = (eponSyncFileHead_t *)(pSendMsg + 1);
	pSyncFile->fileType = SYNC_FILETYPE_CFG_TDM;
	pSyncFile->totalPktCount = 10;
	pSyncFile->curPktId = 1;

	pTxData = (char *)(pSyncFile + 1);

	pRxData = (char *)&((eponSyncFilePkt_t *)pRecvData)->fileData;
	/*pRxData += (sizeof(eponSyncMsgHead_t) + sizeof(eponSyncFileHead_t));*/
	if( VOS_StrCmp(pRxData, "hello") == 0 )
	{
		VOS_StrCpy(pTxData, "Hello too" );
		pSyncFile->pktDataLen = VOS_StrLen(pTxData);
	}
	else if( VOS_StrCmp(pRxData, "thx") == 0 )
	{
		VOS_StrCpy(pTxData, "thank you very much" );
		pSyncFile->pktDataLen = VOS_StrLen(pTxData);
	}
	else if( VOS_StrCmp(pRxData, "w") == 0 )
	{
		VOS_StrCpy(pTxData, "what ?" );
		pSyncFile->pktDataLen = VOS_StrLen(pTxData);
	}
	else
	{
		VOS_StrCpy(pTxData, pRxData );
		pSyncFile->pktDataLen = VOS_StrLen(pTxData);
	}
	*ppSendData = (VOID *)pSendMsg;
	*pSendDataLen = pktLen;

	SYNC_TRACE("\r\n RECV:%s, SEND:%s", pRxData, pTxData);
}

/* 功能: 主备强制倒换消息处理 */
/* 输入参数: pRecvData  接收数据
				recvDataLen 接收数据长度 */
/* 输出参数: ppSendData 发送数据
				pSendDataLen 发送数据长度*/
VOID switchover_NotifyAck(VOID *pRecvData, ULONG recvDataLen, VOID **ppSendData, ULONG *pSendDataLen)
{
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;

	pRecvMsg = (eponSyncMsgHead_t *)pRecvData;
	
	if(pRecvMsg->msgCode == SYNC_MSGCODE_REQUEST )
	{
		pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sizeof(eponSyncMsgHead_t), MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return ;
	   	 }
		VOS_MemZero( pSendMsg, sizeof(eponSyncMsgHead_t) );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_SWITCHOVER;
		pSendMsg->msgCode = SYNC_MSGCODE_NOTIFY;
		pSendMsg->msgResult = SYNC_MSGRESULT_ACK;
			
		*ppSendData = (VOID *)pSendMsg;
		*pSendDataLen = sizeof(eponSyncMsgHead_t);

		VOS_TimerCreate( MODULE_RPU_CLI, 0, 1000, switchoverTimerCallback, NULL, VOS_TIMER_NO_LOOP );
	}
}

#if 0	/* removed by xieshl 20110124, 问题单11819 */
DEFUN (
    sync_private_config_data,
    sync_private_config_data_cmd,
   "sync-file [sw-app|sw-boot|pon-firm|pon-dba|onu-app|cfg-data|tdm-config|ctc-config|sysfile]",
    /* "sync-file [tdm-config|ctc-config]",*/
    /*BOARD_TYPE_GFA6700_SW_STR" active and standby sync\n"*/
    "sync file type\n"
    "sync SW APP\n"
    "sync SW BOOT\n"
    "sync PON FIRM\n"
    "sync PON DBA\n"
    "sync ONU APP\n"
    "sync config  data\n"
    "sync TDM config data\n"
    "sync CTC ONU config data\n"
    "sync systerm file data\n"
    )
{
	char master_boot[MAXBOOTVERSIONLEN], master_sw[MAXSWVERSIONLEN], master_fw[MAXFWVERSIONLEN];
	char standby_boot[MAXBOOTVERSIONLEN], standby_sw[MAXSWVERSIONLEN], standby_fw[MAXFWVERSIONLEN];
	int   rc=VOS_OK;
	syncFileType_t fileType = SYNC_FILETYPE_NULL;

	if( SYS_LOCAL_MODULE_ISMASTERSTANDBY )	/* modified by xieshl 20100510, 问题单10223 */
		return CMD_SUCCESS;

	VOS_MemZero( master_boot, sizeof(master_boot) );
	VOS_MemZero( master_sw, sizeof(master_sw) );
	VOS_MemZero( master_fw, sizeof(master_fw) );
	VOS_MemZero( standby_boot, sizeof(standby_boot) );
	VOS_MemZero( standby_sw, sizeof(standby_sw) );
	VOS_MemZero( standby_fw, sizeof(standby_fw) );
	
	if( g_ulDataSyncFileType != 0 )
		{
		vty_out( vty, "\r\nsync is busy,please waitting!\r\n" );
		return  VOS_OK;
		}
	/* added by chenfj 2009-6-18 备用板在位检查  */
	if( device_standby_master_slotno_get()  == 0)
		{
		vty_out(vty, "\r\n standby-sw not exist\r\n");
		return CMD_WARNING;
		}
	
	if( VOS_StriCmp(argv[0], "ctc-config") == 0 )
		fileType = SYNC_FILETYPE_CFG_CTC;
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	else if( VOS_StriCmp(argv[0], "tdm-config") == 0 )
		fileType = SYNC_FILETYPE_CFG_TDM;
#endif
	else if(VOS_StriCmp(argv[0], "cfg-data") == 0)
		fileType=SYNC_FILETYPE_CFG_DATA;
	else if(VOS_StriCmp(argv[0], "sw-app") == 0)
		{
			if( SYS_LOCAL_MODULE_SLOTNO )
			{
				rc=epon_device_local_board_version_get(master_boot, master_sw, master_fw);
				rc = epon_device_slot_board_version_get( device_standby_master_slotno_get()/*slotno*/, standby_boot, standby_sw, standby_fw);
			}
			
		 if(VOS_StrCmp(master_sw, "V1R07B137")>=0)/*主板和备板的软件版本比较*//*add  by shixh20090605*/
		 	{
			if(VOS_StrCmp(standby_sw,"V1R07B137")>=0)
				{
				fileType=SYNC_FILETYPE_APP_SW;
				}
			else
				{
				fileType = SYNC_FILETYPE_NULL;
				vty_out(vty,"standby app version is not supporting\r\n");
				}
		 	}
		 else
		 	{
				fileType = SYNC_FILETYPE_NULL;
				vty_out(vty,"standby app version is not supporting\r\n");
		 	}	
		}
	else if(VOS_StriCmp(argv[0], "sw-boot") == 0)
		fileType=SYNC_FILETYPE_BOOT_SW;
	else if(VOS_StriCmp(argv[0], "pon-firm") == 0)
		fileType=SYNC_FILETYPE_PON_FIRM;
	else if(VOS_StriCmp(argv[0], "pon-dba") == 0)
		fileType=SYNC_FILETYPE_PON_DBA;
	else if(VOS_StriCmp(argv[0], "onu-app") == 0)
		fileType=SYNC_FILETYPE_ONU;	
	else if(VOS_StriCmp(argv[0],"sysfile")==0)/*add by shixh20090605*/
		fileType=SYNC_FILETYPE_SYSFILE;

	/*else if( VOS_StriCmp(argv[0], "batfile") == 0 )
		fileType = SYNC_FILETYPE_CFG_BATFILE;
	else if( VOS_StriCmp(argv[0], "tdm-app") == 0 )
		fileType = SYNC_FILETYPE_DRV_TDM;
	else if( VOS_StriCmp(argv[0], "tdm-boot") == 0 )
		fileType = SYNC_FILETYPE_BOOT_TDM;*/

	if( fileType != SYNC_FILETYPE_NULL )
	{
		dataSyncProcess( vty, fileType );
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (
    sync_reset,
    sync_reset_cmd,
    "sync reset",
    /* BOARD_TYPE_GFA6700_SW_STR" active and standby sync\n"*/
    "sw active and standby sync\n"
    "sync-module reset\n"
    )
{
	ULONG dst_slotno = 0;

	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return CMD_WARNING;
	if( (dst_slotno = device_standby_master_slotno_get()) == 0 )
		return CMD_WARNING;
	
	dataSync_Reset( dst_slotno );
	
	return CMD_SUCCESS;
}

DEFUN (
    sync_debug_open,
    sync_debug_open_cmd,
    "debug sync",
    "debug\n"
    "sync debug\n"
    )
{
	g_ulDataSyncDebugSwitch = 1;
	return CMD_SUCCESS;
}
DEFUN (
    sync_debug_close,
    sync_debug_close_cmd,
    "undo debug sync",
    "undo\n"
    "debug\n"
    "sync debug\n"
    )
{
	g_ulDataSyncDebugSwitch = 0;
	return CMD_SUCCESS;
}
#endif

/* 功能：找出可以同步的配置文件，然后逐个发送并保存到备用GFA-SW */
/* 输入参数: dst_slotno  备用主控槽位号(对于GFA6700只能是3或4) */
STATUS dataSyncConfigBatchProcess( ULONG dst_slotno )
{
	if( g_ulDataSyncFileType != 0 )
		return VOS_ERROR;

	/* 如果有GFA-TDM板，则同步TDM配置数据 */
	if( get_gfa_tdm_slotno() != 0 )
	{
		g_ulDataSyncFileType = SYNC_FILETYPE_CFG_TDM;
		if( dataSync_SendFile(dst_slotno, g_ulDataSyncFileType) == VOS_ERROR )
		{
			return VOS_ERROR;
		}
	}

	/* 如果启动了CTC协议栈，则同步CTC ONU配置数据 */
	if( V2R1_CTC_STACK )
	{
		g_ulDataSyncFileType = SYNC_FILETYPE_CFG_CTC;
		if( dataSync_SendFile(dst_slotno, g_ulDataSyncFileType) == VOS_ERROR )
		{
			return VOS_ERROR;
		}
	}
	
	return VOS_OK;
}

/*获取备用交换板的版本信息，add by shixh@20080418*/
LONG epon_device_slot_board_version_get(ULONG slotno, char *boot, char *sw, char *fw, char *cpld)
{
	int rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
       int len;
	   
	char *pRxData;
	
	ULONG recvMsgLen = 0;
	ULONG sendMsgLen = sizeof(eponSyncMsgHead_t) +256;

	if( /*slotno == device_standby_master_slotno_get()*/SYS_MODULE_SLOT_ISHAVECPU(slotno) )
	{
		pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return VOS_ERROR;
	   	 }
		VOS_MemZero( pSendMsg, sendMsgLen );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_GET_VER;
		pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
		pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
		pSendMsg->msgData = 0;
		
		rc = CDP_SYNC_Call( MODULE_RPU_EPON, slotno, MODULE_RPU_EPON, 0x10000, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 
		
		if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
		{
			if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
			{
				pRxData = (char *)(pRecvMsg+1);
				/*sys_console_printf( "\r\nRECV: %s\r\n", pRxData );*/
				
				len = pRxData[0];
				VOS_MemCpy(boot, &pRxData[1], len);
				/*sys_console_printf( "boot : %s\r\n", boot);*/
		              pRxData += (len+1);

				len = pRxData[0];
				VOS_MemCpy(sw, &pRxData[1], len);
				/*sys_console_printf( "sw : %s\r\n", sw);*/
		              pRxData +=(len+1);

				len = pRxData[0];
				VOS_MemCpy(fw, &pRxData[1], len);
		              pRxData +=(len+1);

				len = pRxData[0];
				VOS_MemCpy(cpld, &pRxData[1], len);
		          	/*sys_console_printf( "fw: %s\r\n", fw);*/
			}
			else
				rc = VOS_ERROR;
			
			CDP_SYNC_FreeMsg( pRecvMsg );
		}

		if(rc == VOS_ERROR)	/* add by shixh@20080519*/
		{
			if( devsm_module[slotno].software_version.v_version==1)
			{
				if( (devsm_module[slotno].software_version.r_version==2)&&(devsm_module[slotno].software_version.b_version==3) )
					VOS_StrCpy(sw,"V1R05B100" );
				else if( devsm_module[slotno].software_version.r_version==3 )
				{
					if( devsm_module[slotno].software_version.b_version < 88)
						VOS_StrCpy(sw,"V1R07B137" );
					else if( devsm_module[slotno].software_version.b_version==88)
						VOS_StrCpy(sw,"V1R07B164" );
					else
						VOS_StrCpy(sw,"V1R09" );
				}
				else
					VOS_StrCpy(sw,"V1R09" );
			}	
			/*VOS_StrCpy(boot,devsm_module[slotno].pub_info.bootloader_version);
			VOS_StrCpy(fw,devsm_module[slotno].pub_info.hardware_version);*/
			VOS_Sprintf(boot,"V%d.%d.%d",devsm_module[slotno].pub_info.bootloader_version.b_version,
										devsm_module[slotno].pub_info.bootloader_version.d_version,
										devsm_module[slotno].pub_info.bootloader_version.sp_version);
				

			/*sys_console_printf("boot=%s",boot);*/
			
			VOS_StrCpy(fw,"V1.4" );
			/*VOS_Sprintf(fw,"V%d.%d.%d.%d.%d",devsm_module[slotno].pub_info.hardware_version.v_version,
											devsm_module[slotno].pub_info.hardware_version.r_version,
											devsm_module[slotno].pub_info.hardware_version.b_version,
											devsm_module[slotno].pub_info.hardware_version.d_version,
											devsm_module[slotno].pub_info.hardware_version.sp_version);
				
			sys_console_printf("fw=%s",fw);*/
				
		}
	}

	return VOS_OK;
}
/*gfa8000 device获取备用交换板的版本信息，add by mengxsh@20150506*/
LONG epon_device_8000_slot_board_version_get(ULONG slotno, char *boot, char *sw, char *fw, char *cpld, char *fpga)
{
	int rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
       int len;
	   
	char *pRxData;
	
	ULONG recvMsgLen = 0;
	ULONG sendMsgLen = sizeof(eponSyncMsgHead_t) +256;

	if( /*slotno == device_standby_master_slotno_get()*/SYS_MODULE_SLOT_ISHAVECPU(slotno) )
	{
		pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return VOS_ERROR;
	   	 }
		VOS_MemZero( pSendMsg, sendMsgLen );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_GET_VER;
		pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
		pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
		pSendMsg->msgData = 0;
		
		rc = CDP_SYNC_Call( MODULE_RPU_EPON, slotno, MODULE_RPU_EPON, 0x10000, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 
		
		if( rc == VOS_OK )	/* modified by xieshl 20091021, 问题单9031 */
		{
			if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
			{
				pRxData = (char *)(pRecvMsg+1);
				/*sys_console_printf( "\r\nRECV: %s\r\n", pRxData );*/
				
				len = pRxData[0];
				VOS_MemCpy(boot, &pRxData[1], len);
				/*sys_console_printf( "boot : %s\r\n", boot);*/
		              pRxData += (len+1);

				len = pRxData[0];
				VOS_MemCpy(sw, &pRxData[1], len);
				/*sys_console_printf( "sw : %s\r\n", sw);*/
		              pRxData +=(len+1);

				len = pRxData[0];
				VOS_MemCpy(fw, &pRxData[1], len);
		              pRxData +=(len+1);

				len = pRxData[0];
				VOS_MemCpy(cpld, &pRxData[1], len);
					  pRxData +=(len+1);

				len = pRxData[0];
				VOS_MemCpy(fpga, &pRxData[1], len);
				
		          	/*sys_console_printf( "fw: %s\r\n", fw);*/
			}
			else
				rc = VOS_ERROR;
			
			CDP_SYNC_FreeMsg( pRecvMsg );
		}

		if(rc == VOS_ERROR)	/* add by shixh@20080519*/
		{
			if( devsm_module[slotno].software_version.v_version==1)
			{
				if( (devsm_module[slotno].software_version.r_version==2)&&(devsm_module[slotno].software_version.b_version==3) )
					VOS_StrCpy(sw,"V1R05B100" );
				else if( devsm_module[slotno].software_version.r_version==3 )
				{
					if( devsm_module[slotno].software_version.b_version < 88)
						VOS_StrCpy(sw,"V1R07B137" );
					else if( devsm_module[slotno].software_version.b_version==88)
						VOS_StrCpy(sw,"V1R07B164" );
					else
						VOS_StrCpy(sw,"V1R09" );
				}
				else
					VOS_StrCpy(sw,"V1R09" );
			}	
			/*VOS_StrCpy(boot,devsm_module[slotno].pub_info.bootloader_version);
			VOS_StrCpy(fw,devsm_module[slotno].pub_info.hardware_version);*/
			VOS_Sprintf(boot,"V%d.%d.%d",devsm_module[slotno].pub_info.bootloader_version.b_version,
										devsm_module[slotno].pub_info.bootloader_version.d_version,
										devsm_module[slotno].pub_info.bootloader_version.sp_version);
				

			/*sys_console_printf("boot=%s",boot);*/
			
			VOS_StrCpy(fw,"V1.4" );
			/*VOS_Sprintf(fw,"V%d.%d.%d.%d.%d",devsm_module[slotno].pub_info.hardware_version.v_version,
											devsm_module[slotno].pub_info.hardware_version.r_version,
											devsm_module[slotno].pub_info.hardware_version.b_version,
											devsm_module[slotno].pub_info.hardware_version.d_version,
											devsm_module[slotno].pub_info.hardware_version.sp_version);
				
			sys_console_printf("fw=%s",fw);*/
				
		}
	}

	return VOS_OK;
}



#ifdef __SYNC_DEBUG
DEFUN (
    file_sync_test,
    file_sync_test_cmd,
    "sync test <teststr>",
    /*BOARD_TYPE_GFA6700_SW_STR" active and standby sync\n"*/
    "sw board active and standby sync\n"
    "sync test\n"
    "test str\n"
    )
{
	int rc = VOS_ERROR;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;

	eponSyncFileHead_t *pSyncFile;
	char *pTxData, *pRxData;
	
	ULONG recvMsgLen = 0;
	ULONG sendMsgLen = sizeof(eponSyncMsgHead_t) + sizeof(eponSyncFileHead_t) + 100;
	ULONG slotno = device_standby_master_slotno_get();

	if( slotno )
	{
		pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return rc;
	   	 }
		VOS_MemZero( pSendMsg, sendMsgLen );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_TEST;
		pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
		pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
		pSendMsg->msgData = SYNC_FILETYPE_CFG_TDM;

		pSyncFile = (eponSyncFileHead_t *)(pSendMsg + 1);
		pSyncFile->fileType = SYNC_FILETYPE_CFG_TDM;
		pSyncFile->totalPktCount = 10;
		pSyncFile->curPktId = 1;
		pSyncFile->pktDataLen = VOS_StrLen(argv[0]);

		pTxData = (char*)(pSyncFile + 1);
		VOS_StrCpy( pTxData, argv[0] );
		/*sendMsgLen = sizeof(eponSyncMsgHead_t) + sizeof(eponSyncFileHead_t) + pSyncFile->pktDataLen;*/
		
		rc = CDP_SYNC_Call( MODULE_RPU_EPON, slotno, MODULE_RPU_EPON, 0x10000, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

		/*if( (rc == VOS_OK) && (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK) )
		{
			pRxData = (char *)&((eponSyncFilePkt_t *)pRecvMsg)->fileData;
			vty_out( vty, "RECV: %s\r\n", pRxData );
			
			CDP_SYNC_FreeMsg( pRecvMsg );
			return CMD_SUCCESS;
		}*/

		/* modified by xieshl 20091021, 问题单9031 */
		if( rc == VOS_OK )
		{
			if( pRecvMsg->msgResult == SYNC_MSGRESULT_ACK )
			{
				pRxData = (char *)&((eponSyncFilePkt_t *)pRecvMsg)->fileData;
				vty_out( vty, "RECV: %s\r\n", pRxData );
			}
			else 
				rc = VOS_ERROR;
			
			CDP_SYNC_FreeMsg( pRecvMsg );
		}
	}
	if( rc == VOS_OK )
		return CMD_SUCCESS;
	return CMD_WARNING;
}

/*add by shixh@20080417*/

STATUS  Get_slaveSWver(char  *ver )
{
	int rc;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;

	char *pTxData, *pRxData;
	
	ULONG recvMsgLen = 0;
	ULONG sendMsgLen = sizeof(eponSyncMsgHead_t) + 100;
	ULONG slotno = device_standby_master_slotno_get();
	sys_console_printf("test\r\n");
       sys_console_printf("slot: %d\r\n", slotno);
	if( slotno )
	{
		pSendMsg = (eponSyncMsgHead_t *)CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return VOS_ERROR;
	   	 }
		VOS_MemZero( pSendMsg, sendMsgLen );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_GET_VER;
		pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
		pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;
		pSendMsg->msgData = 0;
		sys_console_printf("test2\r\n");
		pTxData =0;
		VOS_StrCpy( pTxData, 0);
		/*sendMsgLen = sizeof(eponSyncMsgHead_t) + sizeof(eponSyncFileHead_t) + pSyncFile->pktDataLen;*/
		
		rc = CDP_SYNC_Call( MODULE_RPU_EPON, slotno, MODULE_RPU_EPON, 0x10000, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 
		
              /*sys_console_printf("test3\r\n");
		sys_console_printf("rc=%d\r\n",rc);
		sys_console_printf("msgresult=%d\r\n",pRecvMsg->msgResult );*/
		
		if( (rc == VOS_OK) /*&& (pRecvMsg->msgResult == SYNC_MSGRESULT_ACK)*/ )
		{
			
			VOS_MemCpy(ver, pRecvMsg->msgData,100);
			sys_console_printf("RECV ver: %s\r\n", ver);
			/*vty_out( vty, "RECV ver: %s\r\n", ver );*/
			
			CDP_SYNC_FreeMsg( pRecvMsg );
			return VOS_OK;
		}
	}

	return VOS_ERROR;
}
#endif
/* 功能: 定时器回调函数，在定时器中延时1s后开始倒换*/
VOID switchoverTimerCallback()
{
	ULONG slotno = SYS_MASTER_ACTIVE_SLOTNO;

	if( SYS_LOCAL_MODULE_SLOTNO == slotno )
		return;

	/*VOS_TaskDelay( 100 );*/
	
	device_slot_module_pull( slotno, DEVSM_HOT_PULL_BY_REBOOT );
}

/* 功能: 发起主备SW板倒换*/
int switchoverNotifyProc()
{
	int rc = VOS_ERROR;
	eponSyncMsgHead_t *pRecvMsg;
	eponSyncMsgHead_t *pSendMsg;
	ULONG recvMsgLen = 0;
	ULONG sendMsgLen = sizeof(eponSyncMsgHead_t);
	ULONG slotno = device_standby_master_slotno_get();

	if( slotno )
	{
		if ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)
			bms_port_set(1,0);
		if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
			bms_port_set_8000(1,0);
		
		pSendMsg = ( VOID* ) CDP_SYNC_AllocMsg( sendMsgLen, MODULE_RPU_EPON );
		 if ( NULL == pSendMsg )
	    	{
	        ASSERT( 0 );
	        return rc;
	   	 }
		VOS_MemZero( pSendMsg, sendMsgLen );
		pSendMsg->srcModuleID = MODULE_RPU_EPON;
		pSendMsg->dstModuleID = MODULE_RPU_EPON;
		pSendMsg->msgType = SYNC_MSGTYPE_SWITCHOVER;
		pSendMsg->msgCode = SYNC_MSGCODE_REQUEST;
		pSendMsg->msgResult = SYNC_MSGRESULT_REQACK;

		rc = CDP_SYNC_Call( MODULE_RPU_EPON, slotno, MODULE_RPU_EPON, 0, 
							pSendMsg, sendMsgLen, ( VOID** ) &pRecvMsg, &recvMsgLen, 60 ); 

		/* modified by xieshl 20091021, 问题单9031 */
		if( rc == VOS_OK )
		{
			if( pRecvMsg->msgResult != SYNC_MSGRESULT_ACK )
				rc = VOS_ERROR;
			CDP_SYNC_FreeMsg( pRecvMsg );
		}
	}

	return rc;
}

/* modified by xieshl 20081027, 主备倒换时增加提示确认 */
int cl_gfa_sw_switchover( struct vty * vty )
{
	if( switchoverNotifyProc() == VOS_OK )
	{
		vty_out( vty, "\r\n%% SWITCHOVER......start\r\n" );
		/*return CMD_SUCCESS;*/
	}
	else
		vty_out( vty, "\r\n SWITCHOVER FAILED\r\n" );
	while( 1 )
	{;}
	return 0;
}

DEFUN (
    gfa_sw_switchover,
    gfa_sw_switchover_cmd,
    "switchover",
    "sw board switchover\n"
    /*BOARD_TYPE_GFA6700_SW_STR" board switchover\n" */
    )
{
	ULONG slotno = device_standby_master_slotno_get();
	if( slotno == 0 )
	{
		vty_out( vty, "%%Only one master board, not support switchover.\r\n", slotno );
		return CMD_WARNING;
	}
	
	if ( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		vty_out( vty, "%%You have no right to switchover to slot%d\r\n", slotno );
		return CMD_WARNING;
	}

	/*if ( SYS_MODULE_RUNNINGSTATE( slotno ) < MODULE_INSERTED )
	{
		vty_out( vty, "%%Slot %d is empty.\r\n", dst_slotno );
		return CMD_WARNING;
	}
	else*/ if ( SYS_MODULE_RUNNINGSTATE( slotno ) < MODULE_RUNNING )
	{
		vty_out( vty, "%%Slot %d is not ready.\r\n", slotno );
		return CMD_WARNING;
	}

	vty_out( vty, "Are you sure want to switchover now? [Y/N]" );
	vty->prev_node = vty->node;
	vty->node = CONFIRM_ACTION_NODE;
	vty->action_func = cl_gfa_sw_switchover;
	return CMD_SUCCESS;
}

#ifdef _OEM_TYPE_CLI_
void init_sync_information(void)
{
	/*sync reset*/
	VOS_StrCpy(&sync_ret[0],GetSwBoardNameString());
	VOS_StrCat(sync_ret," active and standby sync\nsync-module reset\n");

	/*switchover*/
	VOS_StrCpy(&sw_switchover[0],GetSwBoardNameString());
	VOS_StrCat(sw_switchover," board switchover\n");
	return;
}
#endif

LONG syncCommandInstall()
{
	install_element ( CONFIG_NODE, &gfa_sw_switchover_cmd );   /* 主备倒换命令 */
#if 0	/* removed by xieshl 20110124, 问题单11819 */
	install_element( CONFIG_NODE, &sync_private_config_data_cmd );
#if 0
	install_element( CONFIG_NODE, &sync_reset_cmd );
	install_element( CONFIG_NODE, &sync_debug_open_cmd );
	install_element( CONFIG_NODE, &sync_debug_close_cmd );
#else
	/*install_element( DEBUG_HIDDEN_NODE, &file_sync_test_cmd );*/
	install_element( DEBUG_HIDDEN_NODE, &sync_reset_cmd );
	install_element( DEBUG_HIDDEN_NODE, &sync_private_config_data_cmd );
	install_element( DEBUG_HIDDEN_NODE, &sync_debug_open_cmd );
	install_element( DEBUG_HIDDEN_NODE, &sync_debug_close_cmd );
#endif
#endif
	return VOS_OK;
}

#endif

/* 主备GFA-SW配置文件同步 */
STATUS config_sync_notify_event()
{
	syncFileType_t fileType = SYNC_FILETYPE_NULL;
#if (RPU_MODULE_HOT ==  RPU_YES )
	/* 如果有GFA-TDM板，则同步TDM配置数据 */
	if( get_gfa_tdm_slotno() != 0 )
	{
		fileType = SYNC_FILETYPE_CFG_TDM;
	}
	/* 如果启动了CTC协议栈，则同步CTC ONU配置数据 */
	else/* if( V2R1_CTC_STACK )*/ /*问题单14761：如果禁止CTC协议栈，则备用主控无法同步ONU的配置数据 2012-08-29*/
	{
		fileType = SYNC_FILETYPE_CFG_CTC;
	}
	if( SYNC_FILETYPE_NULL != fileType )
		return dataSyncProcess( 0, fileType );
#else
	sys_console_printf("Don't support sync\r\n");
#endif
	return VOS_OK;
}

ULONG get_gfa_tdm_slotno()
{
	int i;
	/*for( i=1; i<=SYS_CHASSIS_SLOTNUM; i++ )*/
	for( i=PONCARD_FIRST; i<=PONCARD_LAST; i++ )
	{
		if(SlotCardIsTdmBoard(i) == ROK ) 
		/*if( __SYS_MODULE_TYPE__(i) == MODULE_E_GFA_SIG )*/
			return (ULONG)i;
	}
	return 0;
}

/* begin: added by jianght 20090304  */

ULONG get_gfa_e1_slotno()
{
	int i;
	for( i=1; i<=SYS_CHASSIS_SLOTNUM; i++ )
	{
		if(SlotCardIsTdmE1Board(i) == ROK ) 
			return (ULONG)i;
	}
	return 0;
}

ULONG get_gfa_sg_slotno()
{
	int i;
	for( i=1; i<=SYS_CHASSIS_SLOTNUM; i++ )
	{
		if(SlotCardIsTdmSgBoard(i) == ROK ) 
			return (ULONG)i;
	}
	return 0;
}
/* end: added by jianght 20090304 */

char *get_sync_filename( syncFileType_t fileType )
{
	char *filename;
	switch( fileType )
	{
		case SYNC_FILETYPE_CFG_TDM:
			filename = "TDM-CFG";				/* TDM 配置数据 */
			break;
		case SYNC_FILETYPE_CFG_CTC:
			filename = "CTC-ONU-CFG";				/* CTC ONU 配置数据 */
			break;	
		case SYNC_FILETYPE_CFG_DATA:		
			filename="CFG-DATA";
			break;
		case SYNC_FILETYPE_APP_SW:			
			 filename="SW-APP";
			break;
		case SYNC_FILETYPE_BOOT_SW:		
			filename="SW-BOOT";
			break;
		case SYNC_FILETYPE_PON_FIRM:		
			filename="PON-FIRM";
			break;
		case SYNC_FILETYPE_PON_DBA:             
			filename="PON-DBA";
			break;	
		case SYNC_FILETYPE_ONU:	
			filename="ONU-APP";
			break;
		case SYNC_FILETYPE_SYSFILE:  /*add by shixh20090605*/
			filename="SYS-FILE";
			break;

#if 0
		case SYNC_FILETYPE_CFG_BATFILE:
			filename = "BATFILE";			/* batfile数据 */
			break;
		case SYNC_FILETYPE_APP_SW:
			filename = "SW-APP";			/* OLT 主控GFA-SW板APP 软件 */
			break;
		case SYNC_FILETYPE_DRV_PON:
			filename = "PON-FIRM";		/* OLT  PON firmware和DBA */
			break;
		case SYNC_FILETYPE_DRV_TDM:
			filename = "TDM-APP";			/* OLT GFA-TDM板软件和FPGA */
			break;
		case SYNC_FILETYPE_ONU_SOFT_FIRM:
			filename = "ONU";				/* 所有类型的ONU 软件和固件 */
			break;
		case SYNC_FILETYPE_BOOT_SW:
			filename = "SW-BSP";			/* GFA-SW BSP */
			break;
		case SYNC_FILETYPE_BOOT_TDM:
			filename = "TDM-BSP";			/* GFA-TDM BSP */
			break;
#endif
		default:
			 filename = "unknown";
			break;
	}
	return filename;
}


