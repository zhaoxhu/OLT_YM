#include  "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "sys/main/sys_main.h"

/*文件名:	TdmDataSave.c
*Copyright (c) 2003-2008 GW Technnologies Co., LTD
*All rights reserved
*
*created by shixh@20071018
*
*文件摘要:	TDM配置数据备份
*/



#include "TdmDataSave.h"
#include "tdm_apis.h"
#include "Tdm_comm.h"
#include "syncMain.h"
#include "eventMain.h"
#include "e1/E1_MIB.h"

#define	  NVMFLAG           PRODUCT_OLT_SIGCFG_KEY_NAME	/*"TDM"*/

ULONG g_tdmBackupDebugConfig = 0; 
#define TDM_BACKUP_TRACE if(g_tdmBackupDebugConfig) sys_console_printf

extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
extern int xflash_file_write(int fileID, unsigned char *writebuf, int *size);
extern STATUS dataSync_FlashEraseFile( syncFileType_t fileType );


#ifdef __debug_tdm_data
ULONG  savetdmNvmDataHead(const ULONG addr, const UCHAR *nvmFlag, const UCHAR *nvmVersion, const ULONG size)
{
	nvm_tdmNvmDataHead_t * pFile = ( nvm_tdmNvmDataHead_t* )addr;
					   
	VOS_MemCpy(pFile->nvmFlag,nvmFlag,3);
	VOS_MemCpy(pFile->nvmVersion,nvmVersion,9);
	pFile->nvmDataSize=size;
					   
	return sizeof(nvm_tdmNvmDataHead_t);	   
}

ULONG  retrievetdmNvmDataHead(const ULONG addr, UCHAR *nvmFlag,  UCHAR *nvmVersion, ULONG *size)
{
	nvm_tdmNvmDataHead_t * pFile = ( nvm_tdmNvmDataHead_t* )addr;
					   
	VOS_MemCpy(nvmFlag,pFile->nvmFlag,3);
	VOS_MemCpy(nvmVersion,pFile->nvmVersion,9);
	*size=pFile->nvmDataSize;
					   
	return sizeof(nvm_tdmNvmDataHead_t);	 
}

ULONG savetdmNvmeponSgTable(const ULONG addr,USHORT  eponSgNum, nvm_eponSgEntry_t  *eponSgEntry)
{
	nvm_eponSgTable_t * pGgTal = ( nvm_eponSgTable_t* )addr;
 					   
	pGgTal->eponSgEntry_Num=eponSgNum;
	VOS_MemCpy(pGgTal->eponSgEntry,eponSgEntry,eponSgNum*sizeof(nvm_eponSgEntry_t));
	return  eponSgNum*sizeof(nvm_eponSgEntry_t)+sizeof(USHORT);	   
}

ULONG retrievetdmNvmeponSgTable(const ULONG addr,USHORT  *eponSgNum, nvm_eponSgEntry_t  *eponSgEntry)
{
	nvm_eponSgTable_t * pGgTal = ( nvm_eponSgTable_t* )addr;
 
	*eponSgNum=pGgTal->eponSgEntry_Num;
	VOS_MemCpy(eponSgEntry,pGgTal->eponSgEntry,(*eponSgNum)*sizeof(nvm_eponSgEntry_t));
		
	return  (*eponSgNum)*sizeof(nvm_eponSgEntry_t)+sizeof(USHORT);	   
}

ULONG savetdmNvmeponOnuTable(const ULONG addr,USHORT  eponTdmOnuNum, nvm_eponTdmOnuEntry_t  *eponTdmOnuEntry )
{
	nvm_eponTdmOnuTable_t * pOnuTal = ( nvm_eponTdmOnuTable_t* )addr;
 					   
	pOnuTal->eponTdmOnuEntry_Num=eponTdmOnuNum;
	VOS_MemCpy(pOnuTal->eponTdmOnuEntry,eponTdmOnuEntry,eponTdmOnuNum*sizeof(nvm_eponTdmOnuEntry_t));
	return  eponTdmOnuNum*sizeof(nvm_eponTdmOnuEntry_t)+sizeof(USHORT);	   
}

ULONG retrievetdmNvmeponOnuTable(const ULONG addr,USHORT  *eponTdmOnuNum, nvm_eponTdmOnuEntry_t  *eponTdmOnuEntry)
{
	nvm_eponTdmOnuTable_t * pOnuTal = ( nvm_eponTdmOnuTable_t* )addr;

	*eponTdmOnuNum=pOnuTal->eponTdmOnuEntry_Num;
	VOS_MemCpy(eponTdmOnuEntry,pOnuTal->eponTdmOnuEntry,(*eponTdmOnuNum)*sizeof(nvm_eponTdmOnuEntry_t));
	return  (*eponTdmOnuNum)*sizeof(nvm_eponTdmOnuEntry_t)+sizeof(USHORT);	
}

ULONG  savetdmNvmeponPotsLInkTable( const ULONG addr,USHORT  eponpotsNum, nvm_eponPotsLinkEntry_t  *eponpotsEntry)
{
	nvm_eponPotsLinkTable_t * pLKTal = ( nvm_eponPotsLinkTable_t* )addr;
 					   
	pLKTal->eponPotsLinkEntry_Num=eponpotsNum;
	VOS_MemCpy(pLKTal->eponPotsLinkEntry,eponpotsEntry,eponpotsNum*sizeof(nvm_eponPotsLinkEntry_t));
	return  eponpotsNum*sizeof(nvm_eponPotsLinkEntry_t)+sizeof(USHORT);	   
}

ULONG  retrievetdmNvmeponPotsLInkTable( const ULONG addr,USHORT  *eponpotsNum, nvm_eponPotsLinkEntry_t  *eponpotsEntry)
{
	nvm_eponPotsLinkTable_t * pLKTal = ( nvm_eponPotsLinkTable_t* )addr;
 					   
	*eponpotsNum=pLKTal->eponPotsLinkEntry_Num;
	VOS_MemCpy(eponpotsEntry,pLKTal->eponPotsLinkEntry,(*eponpotsNum)*sizeof(nvm_eponPotsLinkEntry_t));
	return  (*eponpotsNum)*sizeof(nvm_eponPotsLinkEntry_t)+sizeof(USHORT);	   
}

STATUS tdm_sgTable_getNext1( UCHAR calltype, ULONG callnotifier, ULONG idxs, sgtable_row_entry *pEntry )
{
	if( idxs > 2)
		return VOS_ERROR;
	idxs++;

	pEntry->sgifIdx = idxs;
	pEntry->clusterIdx=2;
	pEntry->mastere1=1;
	pEntry->slavee1=2;
	pEntry->e1clk=3;
	pEntry->vlanen=1;
	pEntry->vlanid=2;
	pEntry->vlanpri=1;
	return VOS_OK;
}

STATUS tdm_tdmOnuTable_getNext1( UCHAR calltype, ULONG callnotifier, ULONG *idxs, tdmonutable_row_entry *pEntry )
{
	if( idxs[1] == 0 )
		idxs[1] = 61001;
	else if( idxs[1] < 61005 )
		idxs[1]++;
	else
	{
		idxs[0]++;
		idxs[1] += 1000;
	}
	if( idxs[0] == 0 )
		idxs[0] = 1;
	else if( idxs[0] > 3 )
		return VOS_ERROR;

	pEntry->sgIdx =(UCHAR) idxs[0];
	pEntry->devIdx=idxs[1];
	pEntry->logiconuIdx=1;
	pEntry->serviceIdx=1;
	pEntry->rowstatus=1;
	pEntry->potsloopenable=1;
	return VOS_OK;
}

STATUS tdm_potsLinkTable_getNext1( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], potslinktable_row_entry *pEntry )
{
	if( idxs[1] > 4 )
	{
		idxs[0]++;
		idxs[1] = 1;
	}
	else 
		idxs[1]++;
	
	if(idxs[0] == 0 )
		idxs[0] = 1;
	else if( idxs[0] > 2 )
		return VOS_ERROR;

	pEntry->linksgIdx = idxs[0];
	pEntry->linksgportIdx=idxs[1];
	pEntry->devIdx=1;
	pEntry->brdIdx=2;
	pEntry->potsIdx=3;
	pEntry->phonecode=1010101000;
	VOS_StrCpy(pEntry->linkdesc,"how are you?");/*{'h','o','w', 'a','r','e',' y','o','u'};*/
	pEntry->linkrowstatus=1;
	return VOS_OK;
}
#endif

extern int  GetOltSWVersion( char *Version, int *len );
/* added by xieshl 20080324 */
#define NVM_CFG_DATA_VER_INVALID		0
#define NVM_CFG_DATA_VER_V1R07		1		/* 采用V1R07数据格式定义存储 */
int checkCfgDataVersion( UCHAR *pNvmVersion)
{
	int  len;
	UCHAR cur_ver[16];
	UCHAR v1r07[] = "V1R07";
	
	GetOltSWVersion( cur_ver, &len );
	if( VOS_MemCmp(cur_ver, v1r07, sizeof(v1r07)) >= 0 )
	{
		if( VOS_MemCmp(pNvmVersion, v1r07, sizeof(v1r07)) >= 0 )
			return NVM_CFG_DATA_VER_V1R07;
	}
	return NVM_CFG_DATA_VER_INVALID;
}

/*TDM 配置数据保存到flash中*/
STATUS  epon_SG_cfgdata_save()
{
	ULONG idxs[2];
	UCHAR ver_str[16];
	int  len;
			
	sgtable_row_entry			psg;
	tdmonutable_row_entry		ptonu;
	potslinktable_row_entry		plt;
	e1porttable_row_entry          e1port;/*add by shixh@20080402*/

	nvm_tdmNvmDataHead_t	*pDataHead;
	nvm_eponSgEntry_t		*pSgEntry;
	nvm_eponSgTable_t		*pSgTable;
	nvm_eponTdmOnuEntry_t	*pTdmOnuEntry;
	nvm_eponTdmOnuTable_t	*pTdmOnuTable;
	nvm_eponPotsLinkEntry_t	*pPotsLinkEntry;
	nvm_eponPotsLinkTable_t	*pPotsLinkTable;
	nvm_e1portentry_t               *pE1PortEntry;/*add by shixh@20080402*/
	nvm_eponE1PortTable_t        *pE1PortTable;

	UCHAR *pNvmTdmDataBuf;
	ULONG nvmDataLen;

	pNvmTdmDataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN);
	if( pNvmTdmDataBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	pDataHead =(nvm_tdmNvmDataHead_t *)pNvmTdmDataBuf;
	nvmDataLen = 0;

	TDM_BACKUP_TRACE( "\r\n TDM data save begin...\r\n" );

	nvmDataLen += sizeof(nvm_tdmNvmDataHead_t);
	pDataHead++;

	pSgTable = (nvm_eponSgTable_t *)pDataHead;
	pSgEntry = pSgTable->eponSgEntry;
	nvmDataLen += sizeof(pSgTable->eponSgEntry_Num);
	
	idxs[0] = 0;
	pSgTable->eponSgEntry_Num = 0;
	while( VOS_OK == tdm_sgTable_getNext(idxs[0], &psg) )
	{
		if( SG_NUM_MAX <= pSgTable->eponSgEntry_Num )
		{
			VOS_ASSERT( 0 );
			break;
		}
		pSgEntry->eponSgIfIndex = psg.sgifIdx;
		pSgEntry->eponSgHdlcMasterE1 = (UCHAR)psg.mastere1;
		pSgEntry->eponSgHdlcSlaveE1 = (UCHAR)psg.slavee1;
		pSgEntry->eponSgHdlcE1Clk = (UCHAR)psg.e1clk;
		pSgEntry->eponSgVlanEnable = (UCHAR)psg.vlanen;
		pSgEntry->eponSgVlanId = psg.vlanid;
		pSgEntry->eponSgVlanPri = (UCHAR)psg.vlanpri;

		TDM_BACKUP_TRACE( " sgindex=%d\r\n", pSgEntry->eponSgIfIndex );		   
				  
		pSgEntry++;
		nvmDataLen += sizeof(nvm_eponSgEntry_t);

		idxs[0] = psg.sgifIdx;
		pSgTable->eponSgEntry_Num++;	
	}
	TDM_BACKUP_TRACE( " SG Table: count=%d, length=%d\r\n", pSgTable->eponSgEntry_Num, nvmDataLen );

	pTdmOnuTable = (nvm_eponTdmOnuTable_t *)pSgEntry;
	pTdmOnuEntry = pTdmOnuTable->eponTdmOnuEntry;
	nvmDataLen += sizeof(pTdmOnuTable->eponTdmOnuEntry_Num);
	
	idxs[0] = 0;
	idxs[1] = 0;
	pTdmOnuTable->eponTdmOnuEntry_Num = 0;
	while( VOS_OK == tdm_tdmOnuTable_getNext( idxs, &ptonu) )
	{
		if( TDMONU_NUM_MAX <= pTdmOnuTable->eponTdmOnuEntry_Num )
		{
			VOS_ASSERT( 0 );
			break;
		}
		pTdmOnuEntry->eponTdmOnuSgIfIndex = ptonu.sgIdx;
		pTdmOnuEntry->eponTdmOnuDevIndex = ptonu.devIdx;
		pTdmOnuEntry->eponTdmOnuLogicalOnuIndex = ptonu.logiconuIdx;
		pTdmOnuEntry->eponTdmOnuRowStatus = ptonu.rowstatus;

		pTdmOnuEntry++;
		nvmDataLen += sizeof(nvm_eponTdmOnuEntry_t);
		
		idxs[0] = ptonu.sgIdx;	  
		idxs[1] = ptonu.devIdx;	  
		pTdmOnuTable->eponTdmOnuEntry_Num++;	
	}
	TDM_BACKUP_TRACE( " TDM-ONU Table: count=%d, length=%d\r\n", pTdmOnuTable->eponTdmOnuEntry_Num, nvmDataLen );

	pPotsLinkTable = (nvm_eponPotsLinkTable_t *)pTdmOnuEntry;
	pPotsLinkEntry = pPotsLinkTable->eponPotsLinkEntry;
	nvmDataLen += sizeof(pPotsLinkTable->eponPotsLinkEntry_Num);
		
	idxs[0] = 0;
	idxs[1] = 0;
	pPotsLinkTable->eponPotsLinkEntry_Num = 0;
	while( VOS_OK == tdm_potsLinkTable_getNext(idxs,&plt) )
	{
		if( POTSLINK_NUM_MAX <= pPotsLinkTable->eponPotsLinkEntry_Num )
		{
			VOS_ASSERT( 0 );
			break;
		}
		pPotsLinkEntry->eponPotsLinkSgIfIndex = plt.linksgIdx;
		pPotsLinkEntry->eponPotsLinkSgPortIndex = plt.linksgportIdx;
		pPotsLinkEntry->eponPotsLinkOnuDev = plt.devIdx;
		pPotsLinkEntry->eponPotsLinkOnuBoard = plt.brdIdx;
		pPotsLinkEntry->eponPotsLinkOnuPots = plt.potsIdx;
		pPotsLinkEntry->eponPotsLinkPhoneCode = plt.phonecode;
		VOS_StrCpy( pPotsLinkEntry->eponPotsLinkDesc, plt.linkdesc );
		pPotsLinkEntry->eponPotsLinkRowStatus = plt.linkrowstatus;	

		pPotsLinkEntry++;
		nvmDataLen += sizeof(nvm_eponPotsLinkEntry_t);
			
		idxs[0] = plt.linksgIdx;
		idxs[1] = plt.linksgportIdx;
		pPotsLinkTable->eponPotsLinkEntry_Num++;
	}
	TDM_BACKUP_TRACE( " POTS-LINK Table: count=%d, length=%d\r\n", pPotsLinkTable->eponPotsLinkEntry_Num, nvmDataLen );

	/*add by shixh@20080402*/
	pE1PortTable=(nvm_eponE1PortTable_t*)pPotsLinkEntry;
	pE1PortEntry=pE1PortTable->epone1portentry;
	nvmDataLen += sizeof(pE1PortTable->epone1portentry_Num);

	idxs[0] = 1;
	idxs[1] = 0;
	idxs[2] = 0;
	pE1PortTable->epone1portentry_Num = 0;
	while( VOS_OK == tdm_e1portTable_getNext(idxs,&e1port) )
	{
		if( E1PORT_NUM_MAX <= pE1PortTable->epone1portentry_Num )
		{
			VOS_ASSERT( 0 );
			break;
		}
		pE1PortEntry->devIdx = e1port.devIdx;
		pE1PortEntry->brdIdx =get_gfa_sg_slotno();
		pE1PortEntry->e1portIdx = e1port.e1portIdx;
		pE1PortEntry->almmask = e1port.almmask;
		pE1PortEntry->crcenable = e1port.crcenable;
		pE1PortEntry->reserved= 0;
		
		pE1PortEntry++;
		nvmDataLen += sizeof(nvm_e1portentry_t);
			
		idxs[0] = e1port.devIdx;
		idxs[1] = e1port.brdIdx;
		idxs[2] = e1port.e1portIdx;
		pE1PortTable->epone1portentry_Num++;
	}
	TDM_BACKUP_TRACE( " E1-PORT Table: count=%d, length=%d\r\n", pE1PortTable->epone1portentry_Num, nvmDataLen );
	
	pDataHead =(nvm_tdmNvmDataHead_t *)pNvmTdmDataBuf;
	VOS_MemCpy( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag) );
	TDM_BACKUP_TRACE( "nvm =%s\r\n", pDataHead->nvmFlag );
	VOS_MemZero( ver_str, sizeof(ver_str) );
	GetOltSWVersion( ver_str, &len );
	TDM_BACKUP_TRACE( "ver is =%s, len=%d\r\n", ver_str, len );
	VOS_MemCpy(pDataHead->nvmVersion, ver_str,/*"V2R1Z001", */sizeof(pDataHead->nvmVersion) );
	TDM_BACKUP_TRACE( "ver=%s\r\n", pDataHead->nvmVersion );
	pDataHead->nvmDataSize = nvmDataLen;
        TDM_BACKUP_TRACE( "data len =%d\r\n", pDataHead->nvmDataSize );
	if( nvmDataLen > TDM_CFG_FILE_LEN )
	{
		VOS_ASSERT( 0 );
		g_free( pNvmTdmDataBuf );
		return VOS_ERROR;
	}

	V2R1_disable_watchdog();
	xflash_file_write( TDM_FLASH_FILE_ID, pNvmTdmDataBuf, (int *)&nvmDataLen );	   

	V2R1_enable_watchdog();
	g_free( pNvmTdmDataBuf );

	return VOS_OK;
}

/*从flash中恢复TDM 配置数据*/
extern STATUS  retrieve_E1AlarmMask();
/*extern uchar_t  e1_Alarm_Mask[24+1];*/
extern ushort_t   tdm_MaskBase;
/*extern  uchar_t e1_Alarm_status[24+1];*/

STATUS epon_SG_cfgdata_retrieve( )
	{
	int  i;
	/*int  len;
	UCHAR ver_str[16];*/
	int chk_ver;
	USHORT  sgrow[6];
	UCHAR logicalOnuIdx;
	UCHAR onuRowStatus;
	ULONG  idx[2];
	/*ULONG  phonecode;
	UCHAR linkdesc[32];*/
			
	
	nvm_tdmNvmDataHead_t	*pDataHead;
	nvm_eponSgEntry_t		*pSgEntry;
	nvm_eponSgTable_t		*pSgTable;
	nvm_eponTdmOnuEntry_t	*pTdmOnuEntry;
	nvm_eponTdmOnuTable_t	*pTdmOnuTable;
	nvm_eponPotsLinkEntry_t	*pPotsLinkEntry;
	nvm_eponPotsLinkTable_t	*pPotsLinkTable;
	nvm_e1portentry_t               *pE1PortEntry;/*add by shixh@20080402*/
	nvm_eponE1PortTable_t        *pE1PortTable;

	UCHAR *pNvmTdmDataBuf;
	ULONG    nvmDataLen=TDM_FLASH_FILE_LEN;
       	
	pNvmTdmDataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN);
	if( pNvmTdmDataBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	TDM_BACKUP_TRACE( " \r\nTDM data retrieve begin...\r\n" );

	V2R1_disable_watchdog();
	xflash_file_read( TDM_FLASH_FILE_ID, pNvmTdmDataBuf, (int *)&nvmDataLen );
	V2R1_enable_watchdog();
	
	pDataHead=( nvm_tdmNvmDataHead_t*)pNvmTdmDataBuf;

	TDM_BACKUP_TRACE("\r\nnvmFlag:%c%c%c", pDataHead->nvmFlag[0], pDataHead->nvmFlag[1], pDataHead->nvmFlag[2] );
	TDM_BACKUP_TRACE( "\r\n nvmVersion=%s,nvmDataLen=%d\r\n", pDataHead->nvmVersion,pDataHead->nvmDataSize);
	
	if(VOS_MemCmp( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag)) != 0 )
	{
		/* begin: added by jianght 20090601 */
		TDM_BACKUP_TRACE("Sig NvmFlag:%s is not %s\r\n", pDataHead->nvmFlag, NVMFLAG);
		/* end: added by jianght 20090601 */
		TDM_BACKUP_TRACE("TDM NVM Flag is invalid\r\n");
		g_free(pNvmTdmDataBuf);
              return  VOS_ERROR;
	}
	
	/*GetOltSWVersion( ver_str, &len );
	if((VOS_MemCmp(pDataHead->nvmVersion, ver_str, sizeof(pDataHead->nvmVersion)) >= 0)&&
		((nvmDataLen>0)&&(nvmDataLen<=TDM_FLASH_FILE_LEN)))*/
	if( (nvmDataLen == 0) || (nvmDataLen > TDM_FLASH_FILE_LEN) )
	{
		TDM_BACKUP_TRACE("TDM NVM file is empty\r\n");
		g_free(pNvmTdmDataBuf);
              return  VOS_ERROR;
	}

	chk_ver = checkCfgDataVersion(pDataHead->nvmVersion);
	if( chk_ver == NVM_CFG_DATA_VER_V1R07 )
	{
		/*恢复第一个表*/
		pDataHead++;
		pSgTable=(nvm_eponSgTable_t*)pDataHead;
		pSgEntry=pSgTable->eponSgEntry;
		TDM_BACKUP_TRACE( " epon sg num=%d\r\n", pSgTable->eponSgEntry_Num);
		if( pSgTable->eponSgEntry_Num <= SG_NUM_MAX )
		{
			for(i=0;i<pSgTable->eponSgEntry_Num;i++)
			{
				idx[0] =pSgEntry->eponSgIfIndex;
				TDM_BACKUP_TRACE( " eponSgIfIndex=%d\r\n", idx[0]);
				sgrow[0]=pSgEntry->eponSgHdlcMasterE1;
				sgrow[1]=pSgEntry->eponSgHdlcSlaveE1;
				sgrow[2]=pSgEntry->eponSgHdlcE1Clk;
				sgrow[3]=pSgEntry->eponSgVlanEnable;
				sgrow[4]=pSgEntry->eponSgVlanId;
				sgrow[5]=pSgEntry->eponSgVlanPri;
				if (VOS_OK!=tdm_sgTable_rowset( idx[0], sgrow))
				{
					TDM_BACKUP_TRACE( "set sg table error\r\n");
				}
				pSgEntry++;			 
			}
		}
		/*else
			sys_console_printf("epon sg num is 0!\r\n");*/
			
		 /*恢复第二个表*/
		pTdmOnuTable=(nvm_eponTdmOnuTable_t*)pSgEntry;
		pTdmOnuEntry=pTdmOnuTable->eponTdmOnuEntry;
		TDM_BACKUP_TRACE( " tdm onu num=%d\r\n", pTdmOnuTable->eponTdmOnuEntry_Num );
		if( pTdmOnuTable->eponTdmOnuEntry_Num <= TDMONU_NUM_MAX )
		{
			for(i=0;i<pTdmOnuTable->eponTdmOnuEntry_Num;i++)	
			{
				idx[0] = GET_PONSLOT(pTdmOnuEntry->eponTdmOnuDevIndex)/*pTdmOnuEntry->eponTdmOnuDevIndex / 10000*/;
				if(SYS_SLOTNO_IS_ILLEGAL(idx[0] ))
					{
					/* begin: added by jianght 20090629 */
					g_free(pNvmTdmDataBuf);
					/* end: added by jianght 20090629 */
					VOS_ASSERT(0);
					return VOS_ERROR;
					}
				if( (SlotCardIsPonBoard(idx[0]) ==ROK) || 
					(__SYS_MODULE_TYPE__(idx[0]) <= MODULE_TYPE_UNKNOW) )
				{
					idx[0]=pTdmOnuEntry->eponTdmOnuSgIfIndex;
					idx[1]=pTdmOnuEntry->eponTdmOnuDevIndex;
					TDM_BACKUP_TRACE( " eponTdmOnuDevIndex=%d\r\n", idx[1]);
			 		logicalOnuIdx=pTdmOnuEntry->eponTdmOnuLogicalOnuIndex;
					onuRowStatus=pTdmOnuEntry->eponTdmOnuRowStatus;
				 	if(VOS_OK!=tdm_tdmOnuTable_rowset(idx,logicalOnuIdx,onuRowStatus))
				 	{
				 		 TDM_BACKUP_TRACE( "set onu table error\r\n");
				 	}
				}
				pTdmOnuEntry++;
			}
		}
		/*else
			sys_console_printf("epon onu num is 0!\r\n");*/
			
		 /*恢复第三个表格*/
		 pPotsLinkTable=(nvm_eponPotsLinkTable_t*)pTdmOnuEntry;
		pPotsLinkEntry=pPotsLinkTable->eponPotsLinkEntry;
		TDM_BACKUP_TRACE( " pots link num=%d\r\n", pPotsLinkTable->eponPotsLinkEntry_Num);
		if( pPotsLinkTable->eponPotsLinkEntry_Num <= POTSLINK_NUM_MAX )
		{
			for(i=0;i<pPotsLinkTable->eponPotsLinkEntry_Num;i++)	
			{
				idx[0] = GET_PONSLOT(pPotsLinkEntry->eponPotsLinkOnuDev)/*pPotsLinkEntry->eponPotsLinkOnuDev / 10000*/;
				if(SYS_SLOTNO_IS_ILLEGAL(idx[0] ))
					{
					/* begin: added by jianght 20090629 */
					g_free(pNvmTdmDataBuf);
					/* end: added by jianght 20090629 */
					VOS_ASSERT(0);
					return  VOS_ERROR;
					}
				if( (SlotCardIsPonBoard(idx[0]) == ROK) || 
					(__SYS_MODULE_TYPE__(idx[0]) <= MODULE_TYPE_UNKNOW) )
				{
					idx[0]=pPotsLinkEntry->eponPotsLinkSgIfIndex;
				  	idx[1]=pPotsLinkEntry->eponPotsLinkSgPortIndex;
					/*		
				 	phonecode=pPotsLinkEntry->eponPotsLinkPhoneCode;
				  	VOS_StrCpy(linkdesc, pPotsLinkEntry->eponPotsLinkDesc );*/
					 if(VOS_OK!= tdm_potsLinkTable_rowset(idx, pPotsLinkEntry->eponPotsLinkOnuDev, pPotsLinkEntry->eponPotsLinkOnuBoard, 
		  									pPotsLinkEntry->eponPotsLinkOnuPots, pPotsLinkEntry->eponPotsLinkPhoneCode,
		  									pPotsLinkEntry->eponPotsLinkDesc, pPotsLinkEntry->eponPotsLinkRowStatus))
					 {
				 		TDM_BACKUP_TRACE( "set link table error\r\n"); 
					 }
				}
				pPotsLinkEntry++;
			} 
		}

		/*恢复第四个表格*//*add by shixh@20070402*/
		pE1PortTable=(nvm_eponE1PortTable_t*)pPotsLinkEntry;
		pE1PortEntry=pE1PortTable->epone1portentry;
		TDM_BACKUP_TRACE( " e1 port num=%d\r\n", pE1PortTable->epone1portentry_Num);
		/*VOS_MemZero( e1_Alarm_status, sizeof(e1_Alarm_status) );*//*modyfied 20080430*/
		/*VOS_MemSet( e1_Alarm_Mask, E1_ALM_DEF, sizeof(e1_Alarm_Mask) );*/
		if( pE1PortTable->epone1portentry_Num <= E1PORT_NUM_MAX )
		{
			for(i=0;i<pE1PortTable->epone1portentry_Num;i++)	
			{
				idx[0] = pE1PortEntry->devIdx;
				idx[1] = pE1PortEntry->brdIdx;
				idx[2] = pE1PortEntry->e1portIdx;
				if( (idx[0] != 1) || 
					SYS_SLOTNO_IS_ILLEGAL(idx[1] ) ||
					(idx[2] == 0) || (idx[2] > E1PORT_NUM_MAX) )
				{
					/*VOS_ASSERT(0);
					return  VOS_ERROR;*/
				}
				else if( (SlotCardIsTdmSgBoard(idx[1]) == ROK) || 
					(__SYS_MODULE_TYPE__(idx[1]) <= MODULE_TYPE_UNKNOW) )
				{
					if(VOS_OK!= tdm_e1portTable_rowset(idx, pE1PortEntry->almmask,pE1PortEntry->crcenable, 3))
					{
						TDM_BACKUP_TRACE( "set E1 port table error\r\n"); 
					}
					else
					 	/*SetE1AlarmMask(i+1,pE1PortEntry->almmask);*/
						e1_Alarm_Mask(idx[2])=pE1PortEntry->almmask;
				}
				pE1PortEntry++;
			} 
		}
		for( i=1; i<=24; i++)   /*add by shixh@20080430*/
		{
			idx[0] = 1;
			idx[1] = 0;
			idx[2] = i;
			if( (idx[1] != 0 )&&(e1_Alarm_Mask(i)!=0) )
			{
					/*VOS_ASSERT(0);
					return  VOS_ERROR;*/
			}
			else 
			{
				SetE1AlarmMask(idx,((tdm_MaskBase >>8)&&E1_ALM_ALL));
			}
		}
		/*else
			sys_console_printf("epon link num is 0!\r\n");*/
	}
		/*retrieve_E1AlarmMask();*/ /*当TDM热拔插时，恢复原来配置的数据。add by shixh@20080319*/
	 g_free(pNvmTdmDataBuf);
        return VOS_OK;
}
	
#ifdef __debug_tdm_data
ulong printTdmFileHead(ULONG addr , struct vty * vty)
{	

	nvm_tdmNvmDataHead_t * pfilehead = (nvm_tdmNvmDataHead_t *)addr ;
       int i,j;
	
	vty_out(vty,"\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "filehead_table\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "nvmFlag         nvmVersion         nvmDataSize\r\n" ) ;
	vty_out(vty, "-----------------------------------------\r\n") ;
       for (i=0;i<3;i++)
		vty_out(vty, "%c\r\n", pfilehead->nvmFlag[i] ) ;	
	for(j=0;j<9, j++)		
		vty_out(vty, "%c\r\n", pfilehead->nvmVersion[j] ) ;
       vty_out(vty, "%d\r\n",  pfilehead->nvmDataSize ) ;
	return sizeof(nvm_tdmNvmDataHead_t) ;
}

ulong printEponSgTable(ULONG addr ,struct vty * vty)
{
             int i;
	nvm_eponSgTable_t * pSg = (nvm_eponSgTable_t *)addr ;

	vty_out(vty,"\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "EponSgTable\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty,"epon sg num is:%d\r\n",pSg->eponSgEntry_Num) ;
	vty_out(vty, "eponSgIfIndex    eponSgHdlcMasterE1    eponSgHdlcSlaveE1   eponSgHdlcE1Clk   eponSgVlanEnable   eponSgVlanId   eponSgVlanPri\r\n" ) ;
	vty_out(vty,"-------------------------------------------------------------------------------------------------\r\n") ;
	for(i=0;i<pSg->eponSgEntry_Num;i++)
	vty_out(vty, "%d-%d-%d-%d-%d-%d-%d\r\n", i,pSg->eponSgEntry[i].eponSgIfIndex, pSg->eponSgEntry[i].eponSgHdlcMasterE1 ,  pSg->eponSgEntry[i].eponSgHdlcSlaveE1,pSg->eponSgEntry[i].eponSgHdlcE1Clk,pSg->eponSgEntry[i].eponSgVlanEnable ,  pSg->eponSgEntry[i].eponSgVlanId , pSg->eponSgEntry[i].eponSgVlanPri);

	return sizeof( nvm_eponSgTable_t ) ;
}

ulong printEponTdmOnuTable(ULONG addr ,struct vty * vty)
{
       int i;
	nvm_eponTdmOnuTable_t * ptdmonu = (nvm_eponTdmOnuTable_t *)addr ;

	vty_out(vty,"\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "EponTdmOnuTable\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty,"epon tdm onu num is:%d\r\n",ptdmonu->eponTdmOnuEntry_Num) ;
	vty_out(vty, "eponTdmOnuDevIndex    eponTdmOnuSgIfIndex       eponTdmOnuLogicalOnuIndex   eponTdmOnuRowStatus\r\n" ) ;
	vty_out(vty,"-------------------------------------------------------------------------------------------------\r\n") ;
	for(i=0;i<ptdmonu->eponTdmOnuEntry_Num;i++)/*eponTdmOnuNum=3*256*/
	vty_out(vty, "%d-%c-%d-%c\r\n", i,ptdmonu->eponTdmOnuEntry[i].eponTdmOnuDevIndex, ptdmonu->eponTdmOnuEntry[i].eponTdmOnuSgIfIndex ,  ptdmonu->eponTdmOnuEntry[i].eponTdmOnuLogicalOnuIndex,ptdmonu->eponTdmOnuEntry[i].eponTdmOnuRowStatus);
	return sizeof( nvm_eponTdmOnuTable_t ) ;
}

ulong printEponPotsLinkTable(ULONG addr ,struct vty * vty)
{
        int i,j,k;
	 nvm_eponPotsLinkTable_t * plk = (nvm_eponPotsLinkTable_t *)addr ;

	vty_out(vty,"\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "EponPotsLinkTable\r\n") ;
	vty_out(vty,"\r\n") ;
	vty_out(vty, "eponPotsLinkNum     eponPotsLinkSgIfIndex    eponPotsLinkSgPortIndex       eponPotsLinkOnuDev   eponPotsLinkOnuBoard  eponPotsLinkOnuPots   eponPotsLinkRowStatus\r\n" ) ;
	vty_out(vty,"-------------------------------------------------------------------------------------------------\r\n") ;
	for(i=0;i<plk->eponPotsLinkEntry_Num;i++)/*eponPotsLinkNum=3*2048*/
	vty_out(vty, "%c-%d-%d-%c-%c-%c\r\n", i,plk->eponPotsLinkEntry[i].eponPotsLinkSgIfIndex, plk->eponPotsLinkEntry[i].eponPotsLinkSgPortIndex ,  plk->eponPotsLinkEntry[i].eponPotsLinkOnuDev,plk->eponPotsLinkEntry[i].eponPotsLinkOnuBoard,plk->eponPotsLinkEntry[i].eponPotsLinkOnuPots,plk->eponPotsLinkEntry[i].eponPotsLinkRowStatus);
       vty_out(vty, " eponPotsLinkPhoneCode:\r\n") ;
	vty_out(vty, "-%d",plk->eponPotsLinkEntry[i].eponPotsLinkPhoneCode) ;
	vty_out(vty, " eponPotsLinkDesc:\r\n") ;
	   for(k=0;k<32;k++)
		vty_out(vty, " -%c",plk->eponPotsLinkEntry[i].eponPotsLinkDesc[k]) ;
	   
	return sizeof( nvm_eponPotsLinkTable_t ) ;
}
#endif

char *row_status_status_to_str(ULONG status )
{
	char *str = NULL;
	switch( status )
	{
		case 1:
				str = "Y";/*"active";*/
			break;
		/*case 2:
			str = "notInService";
			break;
		case 3:
			str = "notReady";
			break;
		case 4:
		case 5:
			str = "create";
			break;*/
		default:
			str = "-";
			break;
	}
	return str;
}

/*creat by zhengyt 2008-5-21,解决问题单6688*/
char *row_status_status_to_str_bak(ULONG onudevIdx, ULONG status )
{
	char *str = NULL;
	switch( status )
	{
		case 1:
			if(onuIsOnLine(onudevIdx))
				str = "Y";/*"active";*/
			else
				str="-";
			break;
		case 2:
			str = "notInService";
			
			break;
		/*case 3:
			str = "notReady";
			break;
		case 4:
		case 5:
			str = "create";
			break;*/
		default:
			str = "-";
			break;
	}
	return str;
}

/*显示从flash中读出的TDM配置数据*/
STATUS  printSGcfgData(struct vty * vty )
{
	/*UCHAR idex;
	ULONG idxs;*/
	int  i;
	int chk_ver;
	UCHAR temp_str[16];
	ULONG  tdm_slot;
	/*int  len;
	UCHAR ver_str[16];*/
	/*USHORT  sgrow[6];
	UCHAR sgifIdx;
	UCHAR logicalOnuIdx;
	UCHAR onuRowStatus;
	ULONG  idx[2];
	ULONG  phonecode;
	UCHAR linkdesc[32];*/
			
	nvm_tdmNvmDataHead_t	*pDataHead;
	nvm_eponSgEntry_t		*pSgEntry;
	nvm_eponSgTable_t		*pSgTable;
	nvm_eponTdmOnuEntry_t	*pTdmOnuEntry;
	nvm_eponTdmOnuTable_t	*pTdmOnuTable;
	nvm_eponPotsLinkEntry_t	*pPotsLinkEntry;
	nvm_eponPotsLinkTable_t	*pPotsLinkTable;
	nvm_e1portentry_t               *pE1PortEntry;/*add by shixh@20080402*/
	nvm_eponE1PortTable_t        *pE1PortTable;

	UCHAR *pNvmTdmDataBuf;
	ULONG    nvmDataLen=TDM_FLASH_FILE_LEN;
       	
	pNvmTdmDataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN);
	if( pNvmTdmDataBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	
	V2R1_disable_watchdog();
	xflash_file_read( TDM_FLASH_FILE_ID, pNvmTdmDataBuf, (int *)&nvmDataLen );
	V2R1_enable_watchdog();
	
	pDataHead=( nvm_tdmNvmDataHead_t*)pNvmTdmDataBuf;

	if(VOS_MemCmp( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag)) != 0)	/* modified by xieshl 20080324 */
	{
		/* begin: modified by jianght 20090601 */
		vty_out(vty,"NvmFlag:%s is not %s\r\n", pDataHead->nvmFlag, NVMFLAG);
		/*vty_out(vty,"tdm config file in flash is empty.\r\n");*/
		/* end: modified by jianght 20090601 */
		g_free(pNvmTdmDataBuf);
              return  VOS_ERROR;
	}
	if( (nvmDataLen == 0) || (nvmDataLen > TDM_FLASH_FILE_LEN) )
	{
		vty_out(vty,"tdm config file in flash is empty.\r\n");
		g_free(pNvmTdmDataBuf);
              return  VOS_ERROR;
	}

	VOS_MemZero( temp_str, sizeof(temp_str) );
	VOS_StrnCpy( temp_str, pDataHead->nvmFlag, sizeof(pDataHead->nvmFlag) );
	vty_out(vty, "\r\n nvmFlag=%s,",  temp_str);
	VOS_MemZero( temp_str, sizeof(temp_str) );
	VOS_StrnCpy( temp_str, pDataHead->nvmVersion, sizeof(pDataHead->nvmVersion) );
	vty_out(vty, "nvmVersion=%s,",  temp_str);
	vty_out(vty, "nvmDataLen=%d\r\n",  pDataHead->nvmDataSize);

	chk_ver = checkCfgDataVersion(pDataHead->nvmVersion);
	
	if( chk_ver == NVM_CFG_DATA_VER_V1R07 )	
	{
		/*恢复第一个表*/
		pDataHead++;
		pSgTable=(nvm_eponSgTable_t*)pDataHead;
		pSgEntry=pSgTable->eponSgEntry;
		vty_out( vty,"\r\n epon sg num=%d\r\n", pSgTable->eponSgEntry_Num);
		if(pSgTable->eponSgEntry_Num <= SG_NUM_MAX)
		{
			vty_out(vty,"%-7s%-12s%-10s%-11s%-8s%s\r\n", " SgIdx", "HdlcE1(M/S)",  "HdlcE1Clk","VlanEnable","Vlan-Id","Priority") ;
			vty_out(vty,"---------------------------------------------------------\r\n") ;
			for(i=0;i<pSgTable->eponSgEntry_Num;i++)
			{
				vty_out(vty,"   %-7d%d/%-10d%-11d%-10d%-8d%d\r\n",pSgEntry->eponSgIfIndex, pSgEntry->eponSgHdlcMasterE1,  pSgEntry->eponSgHdlcSlaveE1,pSgEntry->eponSgHdlcE1Clk,pSgEntry->eponSgVlanEnable , pSgEntry->eponSgVlanId, pSgEntry->eponSgVlanPri);
				pSgEntry++;	
			}
		}
		else
			vty_out(vty,"epon sg num is 0!\r\n");
			
		 /*恢复第二个表*/
		pTdmOnuTable=(nvm_eponTdmOnuTable_t*)pSgEntry;
		pTdmOnuEntry=pTdmOnuTable->eponTdmOnuEntry;
		vty_out(vty, "\r\n tdm onu num=%d\r\n", pTdmOnuTable->eponTdmOnuEntry_Num );
		if(pTdmOnuTable->eponTdmOnuEntry_Num <= TDMONU_NUM_MAX )
		{
			vty_out(vty," %-8s%-7s%-10s%s\r\n", "OnuDev", "SgIdx", "voiceOnu", "Status") ;
			vty_out(vty,"---------------------------------\r\n") ;
			for(i=0;i<pTdmOnuTable->eponTdmOnuEntry_Num;i++)/*eponTdmOnuNum=3*256*/
			{
				vty_out(vty,"  %-9d%-8d%-9d%s\r\n", 
							pTdmOnuEntry->eponTdmOnuDevIndex, 
							pTdmOnuEntry->eponTdmOnuSgIfIndex , 
							pTdmOnuEntry->eponTdmOnuLogicalOnuIndex,
							row_status_status_to_str(pTdmOnuEntry->eponTdmOnuRowStatus) );
				pTdmOnuEntry++;
			}
		}
		else
			vty_out(vty,"\r\n epon onu num is 0!\r\n");
			
		/*恢复第三个表格*/
		pPotsLinkTable=(nvm_eponPotsLinkTable_t*)pTdmOnuEntry;
		pPotsLinkEntry=pPotsLinkTable->eponPotsLinkEntry;
		vty_out(vty, "\r\n pots link num=%d\r\n", pPotsLinkTable->eponPotsLinkEntry_Num);
		if(pPotsLinkTable->eponPotsLinkEntry_Num <= POTSLINK_NUM_MAX)
		{
			vty_out(vty," %-6s%-7s%-7s%-7s%-8s%-10s%-15s%s\r\n","SgIdx","SgPort", "OnuDev","OnuBrd","OnuPots","Tel.","Desc","Status") ;
			vty_out(vty,"--------------------------------------------------------------------\r\n") ;
			for(i=0;i<pPotsLinkTable->eponPotsLinkEntry_Num;i++)   /*eponPotsLinkNum=3*2048*/	
			{
				/*VOS_MemZero( phone_number, sizeof(phone_number) );
				bcdtoa( pPotsLinkEntry->eponPotsLinkPhoneCode, phone_number );*/

				if( VOS_StrLen(pPotsLinkEntry->eponPotsLinkDesc) > 16 )
					pPotsLinkEntry->eponPotsLinkDesc[16] = 0;
				vty_out( vty, "   %-6d%-5d%-9d%-7d%-5d%-8x  %-16s  %s\r\n",  
					pPotsLinkEntry->eponPotsLinkSgIfIndex,pPotsLinkEntry->eponPotsLinkSgPortIndex , 
					pPotsLinkEntry->eponPotsLinkOnuDev,pPotsLinkEntry->eponPotsLinkOnuBoard, pPotsLinkEntry->eponPotsLinkOnuPots,
					pPotsLinkEntry->eponPotsLinkPhoneCode, pPotsLinkEntry->eponPotsLinkDesc,
					row_status_status_to_str(pPotsLinkEntry->eponPotsLinkRowStatus) );

				pPotsLinkEntry++;
			}
		}
		else
			vty_out(vty,"\r\n epon link num is 0!\r\n");
		
              /*恢复第四个表格*/
		/*add by shixh@20080402*/
		pE1PortTable=(nvm_eponE1PortTable_t*)pPotsLinkEntry;
		pE1PortEntry=pE1PortTable->epone1portentry;
		vty_out(vty, "\r\n e1 port num=%d\r\n", pE1PortTable->epone1portentry_Num);
		if(pE1PortTable->epone1portentry_Num <= E1PORT_NUM_MAX)
		{
		              vty_out(vty," %-8s%-7s%-7s%-10s%-10s%s\r\n", "Dev", "brd", "e1port", "almmask","crcenable","reserved") ;
				vty_out(vty,"----------------------------------------------------------\r\n") ;
				for(i=0;i<pE1PortTable->epone1portentry_Num ;i++)   
					{
					pE1PortEntry->reserved= 0;
					tdm_slot=get_gfa_sg_slotno();
					pE1PortEntry->devIdx=1;
					vty_out(vty,"%-8d%-7d%-7d%-7x%-7d%d\r\n", pE1PortEntry->devIdx,tdm_slot,pE1PortEntry->e1portIdx,pE1PortEntry->almmask,pE1PortEntry->crcenable,pE1PortEntry->reserved);
	                          	   pE1PortEntry++;
					}

		}
		else
			vty_out(vty,"\r\n e1 port num is 0!\r\n");
		
		}
		 
	g_free(pNvmTdmDataBuf);
       return VOS_OK;
}

/*显示TDM板上运行的数据*/
STATUS  printSGrunningData(struct vty * vty)
{
	ULONG idxs[2]={0,0} ;
 	int i,ulOnuType=0;
	short int ulslot,ulport,ulonuid,ulPonportIdx;
	/*char phone_number[16];*/

	 sgtable_row_entry       psg;
 	tdmonutable_row_entry  ptonu;
 	potslinktable_row_entry   plt;
	e1porttable_row_entry   e1port;
 
/*取第一个表里的内容并显示*/
	idxs[0]=0;
	i=1;
	vty_out(vty,"\r\n%-7s%-12s%-10s%-11s%-8s%s\r\n", " SgIdx", "HdlcE1(M/S)",  "HdlcE1Clk","VlanEnable","Vlan-Id","Priority") ;
	vty_out(vty,"---------------------------------------------------------\r\n") ;
	while(VOS_OK==tdm_sgTable_getNext(idxs[0],&psg))
	{
		if( i > SG_NUM_MAX )
		{
			VOS_ASSERT(0);
			break;
		}
		idxs[0]=psg.sgifIdx;
		vty_out(vty,"   %-7d%d/%-10d%-11d%-10d%-8d%d\r\n",psg.sgifIdx, psg.mastere1,  psg.slavee1,psg.e1clk,psg.vlanen , psg.vlanid, psg.vlanpri);
		i++;
	}
	 
	 /*取第二个表里的内容并显示*/
	idxs[0]=0;
	idxs[1]=0;
	i=1;
	vty_out(vty,"\r\n %-8s%-7s%-10s%s\r\n", "OnuDev", "SgIdx", "voiceOnu", "Status") ;
	vty_out(vty,"---------------------------------\r\n") ;
	while(VOS_OK==tdm_tdmOnuTable_getNext(idxs,&ptonu))
	{
		if( i > TDMONU_NUM_MAX )
		{
			VOS_ASSERT(0);
			break;
		}
		idxs[0]=ptonu.sgIdx;	
		idxs[1]=ptonu.devIdx;	
		vty_out(vty,"  %-9d%-8d%-9d%s\r\n", ptonu.devIdx, ptonu.sgIdx, ptonu.logiconuIdx, row_status_status_to_str_bak(ptonu.devIdx,ptonu.rowstatus));
		i++;
	}

	
	     	
	 /*取第三个表里的内容并显示*/
	idxs[0]=0;
	idxs[1]=0;
	i=1;
	vty_out(vty,"\r\n %-6s%-7s%-7s%-7s%-8s%-10s%-15s%s\r\n","SgIdx","SgPort", "OnuDev","OnuBrd","OnuPots","Tel.","Desc","Status") ;
	vty_out(vty,"--------------------------------------------------------------------\r\n") ;
	while( VOS_OK == tdm_potsLinkTable_getNext(idxs,&plt) )
	{
		if( i > POTSLINK_NUM_MAX )
		{
			VOS_ASSERT(0);
			break;
		}
		idxs[0] = plt.linksgIdx;
		idxs[1] = plt.linksgportIdx;
		/*VOS_MemZero( phone_number, sizeof(phone_number) );
		bcdtoa(plt.phonecode, phone_number );*/
		if( VOS_StrLen(plt.linkdesc) > 16 )
			plt.linkdesc[16] = 0;

		ulslot=GET_PONSLOT(plt.devIdx)/*plt.devIdx/10000*/;
		ulport=GET_PONPORT(plt.devIdx)/*plt.devIdx%10000/1000*/;
		ulonuid=GET_ONUID(plt.devIdx)/*plt.devIdx%1000*/;

		ulPonportIdx=GetPonPortIdxBySlot(ulslot, ulport);
		if( ulPonportIdx == VOS_ERROR )	/* modified by xieshl 20080811, 发现snmp异常，原因不详，这里增加判断 */
			continue;
		if( GetOnuType(ulPonportIdx, ulonuid-1, &ulOnuType) == VOS_ERROR )
			continue;
	
		if(ulOnuType==V2R1_ONU_GT861)
			{
				if((plt.potsIdx>=1)&&(plt.potsIdx<=8))
					{
						plt.brdIdx=2;
					}
				else if((plt.potsIdx)>8&&(plt.potsIdx<=16))
					{
						plt.brdIdx=3;
						plt.potsIdx-=8;
					}
				else if((plt.potsIdx)>16&&(plt.potsIdx<=24))
					{
						plt.brdIdx=4;
						plt.potsIdx-=16;
					}
					
				else if((plt.potsIdx)>24&&(plt.potsIdx<=32))
					{
						plt.brdIdx=5;
						plt.potsIdx-=24;
					}
					
			}

		vty_out( vty, "   %-6d%-5d%-9d%-7d%-5d%-8x  %-16s  %s\r\n",  
				plt.linksgIdx, plt.linksgportIdx, plt.devIdx,plt.brdIdx, plt.potsIdx, plt.phonecode,
				plt.linkdesc, row_status_status_to_str(plt.linkrowstatus) );
		i++;
	}
	
	/*取第四个表里的内容显示*/
	idxs[0] = 1;
	idxs[1] = 0;
	idxs[2] = 0;
	i=1;
	vty_out(vty," %-8s%-7s%-7s%-10s%s\r\n", "Dev", "brd", "e1port", "almmask","crcenable") ;
	vty_out(vty,"------------------------------------------------\r\n") ;
	while( VOS_OK == tdm_e1portTable_getNext(idxs,&e1port) )
	{
		if( i>E1PORT_NUM_MAX)
		{
			VOS_ASSERT( 0 );
			break;
		}
		
		idxs[0] = e1port.devIdx;
		
		idxs[1] = get_gfa_sg_slotno();
		
		idxs[2] = e1port.e1portIdx;
		vty_out(vty,"%-8d%-7d%-7d%-7x%d\r\n",
			e1port.devIdx,idxs[1],e1port.e1portIdx,e1port.almmask,e1port.crcenable);
		i++;
	}
	return VOS_OK;
}

#ifdef  __TDM_CONFIG_DATA_
#endif
/* added by chenfj 2009-3-24
    增加以下函数, 用于统一TDM-E1 和TDM-SG 配置数据操作
 */
extern STATUS epon_e1_cfgdata_save();
extern STATUS printE1cfgData(struct vty *vty);
extern STATUS printE1runningData(struct vty *vty);
extern STATUS epon_e1_cfgdata_retrieve();

STATUS  epon_tdm_cfgdata_save()
{
	int TdmSlot = 0;
	if((TdmSlot = get_gfa_tdm_slotno()) != 0 )
		{
		if(SlotCardIsTdmSgBoard(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==epon_SG_cfgdata_save())
				return CMD_SUCCESS;
			}
		else if(SlotCardIsTdmE1Board(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==epon_e1_cfgdata_save())
				return CMD_SUCCESS;
			}
		}
	return(CMD_WARNING);
}	

STATUS epon_tdm_cfgdata_retrieve( )
{
	int TdmSlot = 0;
	if((TdmSlot = get_gfa_tdm_slotno()) != 0 )
		{
		if(SlotCardIsTdmSgBoard(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==epon_SG_cfgdata_retrieve())
				return CMD_SUCCESS;
			}
		else if(SlotCardIsTdmE1Board(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==epon_e1_cfgdata_retrieve())
				return CMD_SUCCESS;
			}
		}
	return(CMD_WARNING);
}

STATUS  printTDMcfgData(struct vty * vty )
{
	int TdmSlot = 0;
	if((TdmSlot = get_gfa_tdm_slotno()) != 0 )
		{
		if(SlotCardIsTdmSgBoard(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==printSGcfgData(vty))
				return CMD_SUCCESS;
			}
		else if(SlotCardIsTdmE1Board(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==printE1cfgData(vty))
				return CMD_SUCCESS;
			}
		}
	return(CMD_WARNING);
}

STATUS  printTDMrunningData(struct vty * vty)
{
	int TdmSlot = 0;
	if((TdmSlot = get_gfa_tdm_slotno()) != 0 )
		{
		if(SlotCardIsTdmSgBoard(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==printSGrunningData(vty))
				return CMD_SUCCESS;
			}
		else if(SlotCardIsTdmE1Board(TdmSlot) == VOS_OK)
			{
			if(VOS_OK==printE1runningData(vty))
				return CMD_SUCCESS;
			}
		}
	return(CMD_WARNING);
}

DEFUN  (
	show_TDM_cfg_data,
	show_TDM_cfg_data_cmd,
	"show tdm-config",
	DescStringCommonShow
	"show tdm config data\n"
	)
{
	if(VOS_OK==printTDMcfgData(vty))
		return CMD_SUCCESS;
	return CMD_WARNING;
}

DEFUN  (
	show_TDM_running_data,
	show_TDM_running_data_cmd,
	"show tdm-running-config",
	DescStringCommonShow
	"show tdm running data\n"
	)
{
	if(VOS_OK==printTDMrunningData(vty))
		return CMD_SUCCESS;
	return CMD_WARNING;
}

DEFUN  (
	erase_TDM_cfg_data,
	erase_TDM_cfg_data_cmd,
	"erase tdm-config",
	DescStringCommonDelete
	"erase tdm config data\n"
	)
{
	/* wangysh add */
	if( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
	    return CMD_SUCCESS;
	if(VOS_OK==dataSync_FlashEraseFile(SYNC_FILETYPE_CFG_TDM))
		return CMD_SUCCESS;
	return CMD_WARNING;
}

LONG print_tdm_Init()
{
	install_element ( CONFIG_NODE, &show_TDM_cfg_data_cmd);

	install_element ( CONFIG_NODE, &show_TDM_running_data_cmd);

	install_element ( CONFIG_NODE, &erase_TDM_cfg_data_cmd);

	return VOS_OK;
}

/*void _testtdmdata()
{
	sys_console_printf("\r\n  nvm max len=%d\r\n",TDM_CFG_FILE_LEN);
}*/

#endif
