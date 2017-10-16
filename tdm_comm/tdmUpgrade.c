
#ifdef __cplusplus
extern"C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"
#include  "Device_flash.h"
#include "vos/vospubh/vos_byteorder.h"/*add by shixh20090519*/

#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "tdm_comm.h"
#include "tdm_apis.h"

#include "man/cli/cli.h"
#include "man/cli/cl_vty.h"
#include  "eventMain.h"      /*add by shixh20090609*/

enum{
	TDM_UPGRADE_START=1,
	TDM_UPGRADE_RECV = TDM_MSG_RECV_CODE,
	TDM_UPGRADE_TIMEOUT = TDM_MSG_RECV_TIMEOUT
};

enum{
	ERR_SUCCESS = 0x0000,
	ERR_SYS_FAIL = 0x4001,
	ERR_FLOW_FAIL,
	ERR_SERIALNO_FAIL,
	ERR_FILE_TOOBIG,
	ERR_OFFSET_FAIL,
	ERR_LEN_FAIL,
	ERR_OTHER_FAIL= 0xffff
};

enum{
	RET_OK = 0,
	RET_COMM_ERR,
	RET_PDU_ERR
};

extern STATUS registRecvHandler( int msgType, int msgSubType, int tableIdx, FUNC_PRC handler );

typedef struct{
	USHORT sendsn;
	ULONG filelen;
	ULONG offset;
	USHORT buflen;
}PACKED file_trans_ctrl;

#define	MAX_TRY_NUM	3
/*#define SYSFILE_INI   "sysfile.ini"   add by shixh20090518*/
file_trans_ctrl g_fileTransCtrl;

extern ULONG g_tdmDebugFlag;
/*
#define PDU_CHECK( x ) \
	if( x == NULL )\
	{\
		if( g_tdmDebugFlag & TDM_DEBUG_INFO )\
		{\
			sys_console_printf( "\r\nTDM sending queue is full, abort TDM request\r\n" );\
			sys_console_printf( "file:%s, line:%d\r\n",__FILE__, __LINE__  );\
		}\
		return VOS_ERROR;\
	}
*/
static STATUS process_rtn_tdmupgraderes( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( len >= 12 && pdata[4] == MSG_TYPE_TRANSFILE && pdata[5] == MSG_SUBTYPE_UPDATE_RES &&
		*(USHORT*)(pdata+6) == 0 && ret_ptr != NULL )
	{
		*(ULONG*)ret_ptr = *(ULONG*)(pdata+8);
		rc = VOS_OK;
	}

	return rc;
}

static STATUS process_rtn_transsegmentres( const char* pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( len >= 20 && pdata[4] == MSG_TYPE_TRANSFILE && pdata[5] == MSG_SUBTYPE_TRANS_RES &&
		*(USHORT*)(pdata+6) == 0 && ret_ptr != NULL )
	{
		file_trans_ctrl * pEntry = (file_trans_ctrl*)ret_ptr ;
		VOS_MemCpy( pEntry, pdata+8, sizeof(file_trans_ctrl) );
		rc = VOS_OK;
	}		
	
	return rc;
}

static STATUS process_rtn_transendres( const char* pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	if( len >= 20 && pdata[4] == MSG_TYPE_TRANSFILE && pdata[5] == MSG_SUBTYPE_TRANS_END_RES &&
		*(USHORT*)(pdata+6) == 0 && ret_ptr != NULL )
		rc = VOS_OK;
	
	return rc;	
}

static STATUS endUpgradeProcess( USHORT errno )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	ULONG maxfilelen = 0;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_END_REQ );
	pdu->msgCode = errno;

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		if( VOS_OK != (*(FUNC_PRC)getRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_END_RES, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG,  &maxfilelen ) )
				rc = VOS_ERROR;
		else
			rc = VOS_OK;
			
		tdmCommMsgFree( pRecv );
				
	}

	VOS_MemZero( &g_fileTransCtrl, sizeof(file_trans_ctrl) );

	return rc;
	
}

static void upgradeIdlerHandleProc(void *para)
{
	endUpgradeProcess( ERR_OTHER_FAIL );
}

const ULONG TDM_UPGRADE_IDLE_TIMEOUT = 15000;

/**/

STATUS tdmUpgradeRequest ( UCHAR filetype, ULONG *maxfilelen )
{
	
	STATUS rc = RET_COMM_ERR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_TRANSFILE, MSG_SUBTYPE_UPDATE_REQ);
	pdu->msgData[0] = filetype;

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0,  (char*)pdu, pdulen+1, &pRecv, &recvlen ) )
	{

		rc = RET_OK;
		
		if( RET_OK != (*(FUNC_PRC)getRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_UPDATE_RES, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG,  maxfilelen) )
				rc = RET_PDU_ERR;
			
			tdmCommMsgFree( pRecv );
				
	}

	return rc;
	
}


STATUS tdmTransSegment ( USHORT sn, ULONG totallen, ULONG offset, USHORT buflen, char* buf )
{
	
	STATUS rc = RET_COMM_ERR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;

	file_trans_ctrl *pTransCtrl = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	pTransCtrl = (file_trans_ctrl*)pdu->msgData;
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_REQ );

	pTransCtrl->sendsn = sn;
	pTransCtrl->filelen = totallen;
	pTransCtrl->offset = offset;
	pTransCtrl->buflen = buflen;

	memcpy( pdu->msgData+sizeof(file_trans_ctrl), buf, buflen );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen+sizeof(file_trans_ctrl)+buflen, &pRecv, &recvlen ) )
	{

		file_trans_ctrl ftc;

		
		if( (RET_OK == (*(FUNC_PRC)getRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_RES, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, &ftc )) 
			&& ftc.sendsn == (sn+1) 
			&& ftc.filelen == totallen 
			&& ftc.offset == offset )
		{			
			rc = RET_OK;
		}
		else
			rc = RET_PDU_ERR;
		
		tdmCommMsgFree( pRecv );
				
	}

	return rc;
	
}


void initTdmUpgradeRtnProcessFunc( void )
{
	registRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_UPDATE_RES, 0, process_rtn_tdmupgraderes );
	registRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_RES, 0, process_rtn_transsegmentres );
	registRecvHandler( MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_END_RES, 0, process_rtn_transendres );
}

static ULONG readTdmFileFromFlash( char ** pFile, const int fileType )
{
	if( pFile == NULL || fileType < TDM_FPGA_FILE || fileType > TDM_APP_FILE )
		return 0;
	else
	{
	}

	return 0;
}

static struct vty *g_pvty = NULL;
static void outDebugInfo(const char* pCont, const int filelen)
{
	if(cl_vty_valid(g_pvty) == VOS_OK)
	{
		vty_out(g_pvty, "\r\n%s", pCont);
		vty_event(VTY_WRITE, g_pvty->fd, g_pvty);
	}
	else
		sys_console_printf( "\r\nftp info:	%s", pCont );
}

static ULONG readTdmFileFromFtp( int ftpmode, const char *host, const char *user,
							const char *pwd, const char *filename, char **pFile)
{
	int filelen = 0;
	if(ftpcDownLoad(ftpmode, host, user, pwd, filename, outDebugInfo, pFile, &filelen) == VOS_OK)
	{
		return filelen;
	}
	return 0;
}

void tdmUpgradeSetVtyPoint(ULONG vty)
{
	g_pvty = (struct vty *)vty;
}

#define GFA_SIG_FILE_HEAD_LEN	0x80
int sendUpgradeFile(const int fileType, char *pFile, const int filelen)
{
	int ret = TDM_TRANS_OK;
	char *pFileData = NULL;
	unsigned char *FileName;


	/* modified by xieshl 20080523, 增加文件头，用于识别文件类型是否正确，问题单#6477 */
	if (0 != get_gfa_e1_slotno())
	{
              pFileData = pFile;
	} 
	else if (0 != get_gfa_sg_slotno())
	{
		/* begin: modified by jianght 20090702 */
		pFileData = pFile/* + GFA_SIG_FILE_HEAD_LEN*/;
		/* end: modified by jianght 20090702 */
	}
	else
	{
		sys_console_printf("\r\n no sg or e1 board in olt!\r\n");
		return TDM_TRANS_ERR;
	}

	if( filelen <= GFA_SIG_FILE_HEAD_LEN )
		ret = TDM_TRANS_ERR;

	if( fileType == 0 )
	{
		if (0 != get_gfa_e1_slotno())
		{
			FileName = typesdb_product_file_key_name(PRODUCT_OLT_E1APP_KEY_ID);
			if( VOS_StrCmp(pFile, FileName) != 0 )	/* modified by jianght 20080526 */
			{
				ret = TDM_TRANS_ERR;
			}
		} 
		else if (0 != get_gfa_sg_slotno())
		{
			/*unsigned char FileName[128];

			VOS_StrCpy(&FileName[0], typesdb_product_file_key_name(PRODUCT_OLT_SIGAPP_KEY_ID));
			VOS_StrCat(FileName,"SIGAPP");*/
			FileName = typesdb_product_file_key_name(PRODUCT_OLT_SIGAPP_KEY_ID);

			if( VOS_StrCmp(pFile, FileName) != 0 )	/* modified by xieshl 20080526 */
			{
				ret = TDM_TRANS_ERR;
			}
		}
		else
		{
			sys_console_printf("\r\n no sg or e1 board in olt!\r\n");
			return TDM_TRANS_ERR;
		}
	}
	else
	{
		if (0 != get_gfa_e1_slotno())
		{
			FileName = typesdb_product_file_key_name(PRODUCT_OLT_E1FPGA_KEY_ID);
			
			if( VOS_StrCmp(pFile, FileName/*"E1FPGA"*/) != 0 )	/* modified by jianght 20080526 */
			{
				ret = TDM_TRANS_ERR;
			}
		} 
		else if (0 != get_gfa_sg_slotno())
		{
			/*unsigned char FileName[128];
			
			VOS_StrCpy(&FileName[0], typesdb_product_file_key_name(PRODUCT_OLT_SIGAPP_KEY_ID));
			VOS_StrCat(FileName,"SIGFPGA");*/
			FileName = typesdb_product_file_key_name(PRODUCT_OLT_SIGFPGA_KEY_ID);
			
			if( VOS_StrCmp(pFile, FileName/*&FileName[0]*/) != 0 )
			{
				ret = TDM_TRANS_ERR;
			}
		}
		else
		{
			sys_console_printf("\r\n no sg or e1 board in olt!\r\n");
			return TDM_TRANS_ERR;
		}
	}

	if( ret == TDM_TRANS_ERR )
	{
		sys_console_printf("\r\n GFA-SIG upgrade file type error\r\n");
		/*free( pFile );*/	/* removed by xieshl 20091028, 重复释放了 */
		return ret;
	}

	if (0 != get_gfa_e1_slotno())
	{
	       g_fileTransCtrl.filelen = filelen;
	}
	else if (0 != get_gfa_sg_slotno())
	{
		/* begin: modified by jianght 20090702 */
		g_fileTransCtrl.filelen = filelen/* - GFA_SIG_FILE_HEAD_LEN*/;
		/* end: modified by jianght 20090702 */
	}
	else
	{
		sys_console_printf("\r\n no %s or %s board in olt!\r\n", GetSigBoardNameString(), GetE1BoardNameString());
		return TDM_TRANS_ERR;
	}

	if( g_fileTransCtrl.filelen != 0 )
	{
		ULONG pageCount = g_fileTransCtrl.filelen>>10;
		ULONG tailCount = g_fileTransCtrl.filelen&0x3ff;

		ULONG maxfilelen = 0;

		/*文件长度满足最大长度限制才能开始更新*/
		if( (tdmUpgradeRequest( fileType, &maxfilelen ) == RET_OK) && (maxfilelen >= g_fileTransCtrl.filelen) )
		{		
			STATUS oneret = RET_OK;
			int i=0;

			LONG monitorTimer = VOS_TimerCreate( MODULE_TDM_COMM, 0, TDM_UPGRADE_IDLE_TIMEOUT, 
				upgradeIdlerHandleProc, NULL, VOS_TIMER_NO_LOOP );

			if( monitorTimer == 0 )
				ret = TDM_TRANS_ERR;
			else
			{
				for( ; i<pageCount; i++ )
				{
					int trynum = 0;
					g_fileTransCtrl.sendsn = i;
					g_fileTransCtrl.offset = i*1024;
					g_fileTransCtrl.buflen = 1024;

					for( ; trynum<MAX_TRY_NUM; trynum++ )
					{
						oneret = tdmTransSegment( i, g_fileTransCtrl.filelen, g_fileTransCtrl.offset, g_fileTransCtrl.buflen, (char *)(pFileData+g_fileTransCtrl.offset) ) ;
						if( oneret == RET_COMM_ERR )
						{
							endUpgradeProcess( 0xffff );
							sys_console_printf("\r\nforce exit transmit process!");
							/*free( pFile );*/
							break;
						}
						else if( oneret == RET_OK )
							break;
					}

					if( oneret != RET_OK )
					{
						VOS_TimerDelete( MODULE_TDM_COMM, monitorTimer );
						break;
					}
					else
						VOS_TimerChange( MODULE_TDM_COMM, monitorTimer, TDM_UPGRADE_IDLE_TIMEOUT );
				}

				if( i != pageCount )
					ret = TDM_TRANS_ERR;
				else if( tailCount != 0 )
				{
					int j=0;
					g_fileTransCtrl.offset = pageCount*1024;					

					for( ; j<MAX_TRY_NUM; j++ )
					{
						oneret = tdmTransSegment( pageCount, g_fileTransCtrl.filelen, g_fileTransCtrl.offset, tailCount, (char *)(pFileData+g_fileTransCtrl.offset) );
						
						if( oneret == RET_COMM_ERR )
						{
							endUpgradeProcess( 0xffff );
							sys_console_printf("\r\nforce exit transmit process!");
							/*free( pFile );*/
							break;
						}
						else if( oneret == RET_OK )
							break;
					}

					if( oneret != RET_OK )	/*发送失败，传输中止，删除定时器*/
					{
						VOS_TimerDelete( MODULE_TDM_COMM, monitorTimer );
						ret = TDM_TRANS_ERR;
						sys_console_printf("\r\ntransmit error, stop!");
					}
					else
						endUpgradeProcess(0);
					
				}
			}

			if( ret == TDM_TRANS_OK )
			{
				VOS_TimerDelete( MODULE_TDM_COMM, monitorTimer );
				VOS_MemZero( &g_fileTransCtrl, sizeof(file_trans_ctrl) );
			}
		}
		else
		{
			sys_console_printf("\r\nfile is too long:    maximum file length %d, current length %d", maxfilelen, filelen);
			ret = TDM_TRANS_ERR;
		}
	}
	else
	{
		ret = TDM_TRANS_ERR;
	}

	/*free( pFile );*/	/* removed by xieshl 20091028, 重复释放了 */
	return ret;
}

int startUpdateTdm(const int fileType, const char *name, int ftpmode, 
							const char *host, const char *user, const char *pwd)
{
	char *pFile = NULL;
	int filelen = 0;
	int ret = 0;
extern void	g_free (void *__ptr);
	
	filelen = readTdmFileFromFtp(ftpmode, host, user, pwd, name, &pFile);

	if( filelen > GFA_SIG_FILE_HEAD_LEN )
	{
		if( (ret = sendUpgradeFile(fileType, pFile, filelen)) == VOS_OK)
			outDebugInfo("\r\nupgrade tdm file OK!", 0);
	}

	/* modified by chenfj 2009-6-17
	    应释放此内存*/
	if( pFile != NULL )
		g_free( pFile);

	return ret;
}
/*add bu shixh20090518*/

#endif	/* EPON_MODULE_TDM_SERVICE */

extern long device_sysfile_read_flash_to_mem( CHAR* memfile, LONG * memfile_length );
extern long device_sysfile_write_mem_to_flash( CHAR* memfile, LONG * memfile_length );
extern long device_sysfile_erase_from_flash( VOID );
extern int cl_vty_all_out( const char * format, ... );

/*  added by chenfj 2009-6-2 
    增加两个函数, 用于确认OEM sysfile 文件的是否有效*/
    
/* 1 通过头结构确认OEM系统配置文件*/
int CheckSysfileValid(char *pBuffer, int buf_len, struct vty *vty)
{
	app_desc_t  *file_head_ptr;
	int  file_len;
	char *p;

	/* 判断文件格式及内容有效*/
	if(buf_len <= sizeof(app_desc_t))
		{
		/*cl_vty_all_out("\r\n sysfile is too short(<%d)\r\n", sizeof(app_desc_t));*/
		return VOS_ERROR;
		}
	if(buf_len > DEVICE_FLASH_SYNINFO_MAXLEN)
		{
		cl_vty_all_out("\r\n sysfile is too long(>%d)\r\n", DEVICE_FLASH_SYNINFO_MAXLEN);
		return VOS_ERROR;
		}

	file_head_ptr = (app_desc_t *)pBuffer;
	if(VOS_StrnCmp( file_head_ptr->dev_type, SYSFILE_INI, VOS_StrLen(SYSFILE_INI) ) != 0)
		{
		cl_vty_all_out("\r\n sysfile type is wrong(should be %s)\r\n", SYSFILE_INI);
		return VOS_ERROR;
		}
		
	p = (char*)&file_head_ptr->file_len;
	file_len = MAKELONG(  MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) );

	if(file_len > (DEVICE_FLASH_SYNINFO_MAXLEN - sizeof(app_desc_t)))
		{
		cl_vty_all_out("\r\nsysfile len is too long \r\n");
		return VOS_ERROR;
		}

	return VOS_OK;

}

/* 2 用于判断FLASH 中是否存放有效的sysfile 文件*/
int  xflash_sysfile_exist(void)
{
	char *fileBuffer = 0;
	app_desc_t  *file_head_ptr;
	char *p;
	int len=0,ret;
#ifdef INCLUDE_FS
    len = 128;
    fileBuffer = VOS_Malloc(len, MODULE_RPU_PON);
    if(fileBuffer == NULL )
        return VOS_ERROR;
	if( VOS_ERROR == xflash_file_read2(get_file_name(FLASH_SYSFILE_INI),fileBuffer,&len))
	{
		VOS_Free(fileBuffer);
		return VOS_ERROR;
	}
#else
	fileBuffer =  (unsigned char*)(DEVICE_FLASH_SYNINFO_OFFSET + FLASH_BASE_ADDR);
#endif
	file_head_ptr = (app_desc_t *)fileBuffer;
	p = (char *)&file_head_ptr->file_len;
	len = MAKELONG(  MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) ) + sizeof(app_desc_t); 

	ret = CheckSysfileValid(fileBuffer,len, NULL);
#ifdef INCLUDE_FS
	VOS_Free(fileBuffer);
#endif
	return ret;
}

#if 0
/* 从ftp server 上下载sysfile, 并将文件写入flash 保存*/
int downloadSysfile(const int fileType, const char *name, int ftpmode, 
							const char *host, const char *user, const char *pwd, struct vty *vty)
{

	char *pFile = NULL;
	int filelen = 0;
	int ret=VOS_OK;
	
	cl_vty_all_out("\r\nGet ftp operational popedom!\r\n");
	cl_vty_all_out("Connecting to server:%s\r\n",host);
	if(ftpcDownLoad(ftpmode, host, user, pwd, name, NULL, &pFile, &filelen) != VOS_OK)
		{
		cl_vty_all_out("\r\nDownload sysfile from server ...error\r\n");
		onuSysFileDownloadFailure_EventReport(1);
		ret = VOS_ERROR;
		}
	else{
		cl_vty_all_out("\r\nDownload sysfile from server ...ok\r\n");
		ret=VOS_OK;
		}
	
	if((ret == VOS_OK) && (CheckSysfileValid(pFile, filelen, vty) == ROK))
		{
		/* 写sysfile 文件内容到flash */
		if(device_sysfile_write_mem_to_flash(pFile,(LONG *)&filelen)	==VOS_OK)
			{
			cl_vty_all_out("\r\nWrite to flash...ok\r\n");
			onuSysFileDownloadSuccess_EventReport(1);
			ret=VOS_OK;

			/* 同步sysfile，问题单12840，add by zhaozhg*/
			if( ret == VOS_OK )
			{
			      ULONG file_mask = 0;
	             
	              file_mask |= (1<<FLASH_SYSFILE_INI); /* OLT */

				if( VOS_OK != Load_upgrade_session(vty, file_mask/* 1<<FLASH_RES_5*/) )
				{
					vty_out(vty,"\r\n  %% Download onu file to pon board falied please check!!!");
				}
			}
			}
		else {
			cl_vty_all_out("\r\nWrite to flash...error\r\n");
			onuSysFileDownloadFailure_EventReport(1);
			ret = VOS_ERROR;
			}
		}

	/* modified by chenfj 2009-6-17
	    应释放此内存*/
	if(pFile != NULL)
		free(pFile);
	/* wangysh add 更新sysfile 文件版本号问题单11894*/
	update_file_ver(FLASH_SYSFILE_INI);
	sys_console_printf("Release ftp operational popedom\r\n");
	return ret;
#if 0	
	if((filelen > sizeof(app_desc_t))&& ( filelen <DEVICE_FLASH_SYNINFO_MAXLEN))
	{
		file_head_ptr = (app_desc_t *)pFile;
		p = (char*)&file_head_ptr->file_len;
		
		file_head_ptr->file_len = MAKELONG( MAKEWORD(*p, *(p+1)), MAKEWORD( *(p+2), *(p+3) ) );
		if( (file_head_ptr->file_len>0)&& (file_head_ptr->file_len<0x0FF80))
		{
			if(VOS_StrnCmp( file_head_ptr->dev_type, SYSFILE_INI, VOS_StrLen(SYSFILE_INI) )== 0)
			{
				/* 写sysfile 文件内容到flash */
				if(device_sysfile_write_mem_to_flash(pFile,&filelen)	==VOS_OK)
				{
					vty_out(vty, "Write to flsah...ok\r\n");
					return VOS_OK;
				}
				else 
					vty_out(vty, "Write to flash...error\r\n");
			}
			else
				vty_out(vty, "file type error!\r\n");
		}
	}
	else 
		{
		if(vty == NULL)
			sys_console_printf("file error\r\n");
		else vty_out(vty, "file error\r\n");
		
		return VOS_ERROR;
		}

	return VOS_OK;
#endif
}

/* 从flash 中读出sysfile, 并上传 ftp server */
int uploadSysfile( char *host,  char *user,  char *pwd, struct vty *vty)
{
	char pFile[DEVICE_FLASH_SYNINFO_MAXLEN+10];
	int  filelen = 0;
	sys_console_printf("test a 20090519\r\n");

	if(device_sysfile_read_flash_to_mem( pFile,(LONG *)&filelen)==VOS_ERROR)
		sys_console_printf("read flash error!!\r\n");
	else
		sys_console_printf("file len is %d\r\n",filelen);
	if(filelen>sizeof(app_desc_t))/* && ( filelen < DEVICE_FLASH_SYNINFO_MAXLEN)*/
	{
		sys_console_printf("test b 20090519\r\n");
		if(ftpcUpLoad( host,user, pwd, pFile, 1,0, NULL )!=VOS_OK)
			return  VOS_ERROR;
	}
	else 
		{
		if(vty == NULL)
			sys_console_printf("file len not in the range\r\n");
		else
			vty_out(vty,"file len not in the range\r\n");
		
		return VOS_ERROR;
		}
	return VOS_OK;
}
#endif

#ifdef __cplusplus
}
#endif

