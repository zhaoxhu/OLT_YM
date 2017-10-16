
#include  "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
/*#include  "sn_cvtovos.h"*/

#include "sys/main/sys_main.h"

#include "backup/syncMain.h"
#include "eventMain.h"

#include "tdm_apis.h"
#include "Tdm_comm.h"

#include "E1DataSave.h"
#include "e1_apis.h"


#define	  NVMFLAG           PRODUCT_OLT_E1CFG_KEY_NAME /*"E1"*/


extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
extern int xflash_file_write(int fileID, unsigned char *writebuf, int *size);
extern STATUS dataSync_FlashEraseFile( syncFileType_t fileType );

extern int GetOltSWVersion( char *Version, int *len );
extern void* g_malloc(unsigned long);
extern void g_free(void *);

/*extern  BrdTableExt_t ExtBrdMgmtTable[MAX_PON_CHIP][MAXONUPERPON+1];*/

#ifndef NVM_CFG_DATA_VER_INVALID
#define NVM_CFG_DATA_VER_INVALID	0
#define NVM_CFG_DATA_VER_V1R07		1		/* 采用V1R07数据格式定义存储 */
#endif
/*  deleted by  chenfj 2009-3-24
	这个函数与在TDMDataSave.C中的完全一致,引用过来就行了
*/
extern  int checkCfgDataVersion( UCHAR *pNvmVersion);

/*E1 配置数据保存到flash中*/
STATUS epon_e1_cfgdata_save()
{
	ULONG idxs[3], i, j, k, l, swOnuE1PortTableOffset = 0;
	UCHAR ver_str[16];
	int  len;

	nvm_e1NvmDataHead_t	*pDataHead;

	e1LinkTable_row_entry e1Link;
	nvm_e1LinkEntry_t *ptrE1LinkEntry;
	nvm_e1LinkTable_t *ptrE1LinkTable;

	e1PortTable_row_entry e1Port;
	nvm_e1PortEntry_t *ptrE1PortEntry;
	nvm_e1PortTable_t *ptrE1PortTable;

	e1VlanTable_row_entry e1Vlan;
	nvm_e1VlanEntry_t *ptrE1VlanEntry;
	nvm_e1VlanTable_t *ptrE1VlanTable;

	nvm_sw_onu_e1PortTable_t *ptrSwOnuE1PortTable;

	UCHAR *pNvmE1DataBuf;
	ULONG nvmDataLen;

	pNvmE1DataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN); /* 512 KByte */
	if( pNvmE1DataBuf == NULL )
	{
		E1_ERROR_INFO_PRINT(("pNvmE1DataBuf=NULL\r\n"));
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	pDataHead =(nvm_e1NvmDataHead_t *)pNvmE1DataBuf;
	nvmDataLen = 0;

	E1_ERROR_INFO_PRINT(("\r\n TDM data save begin...\r\n"));

	nvmDataLen += sizeof(nvm_e1NvmDataHead_t);
	pDataHead++;

	ptrE1LinkTable = (nvm_e1LinkTable_t *)pDataHead;
	ptrE1LinkEntry = ptrE1LinkTable->e1LinkEntry;
	nvmDataLen    += sizeof(ptrE1LinkTable->e1LinkTableEntry_Num);

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	ptrE1LinkTable->e1LinkTableEntry_Num = 0;

	for (idxs[2] = 0; idxs[2] < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); idxs[2]++)
	{
		if ( VOS_OK != tdm_e1LinkTable_get(idxs, &e1Link) )
		{
			E1_ERROR_INFO_PRINT(("epon_e1_cfgdata_save::tdm_e1LinkTable_get()   E1 Idx=%d   error!\r\n", idxs[2]));
			sys_console_printf("%% Get E1 link table Fail!\r\n  %% save E1 data failed!\r\n");
			g_free( pNvmE1DataBuf );
			return VOS_ERROR;
		}

		ptrE1LinkEntry->onuDevId    = e1Link.onuDevId;
		ptrE1LinkEntry->onuE1SlotId = e1Link.onuE1SlotId;
		ptrE1LinkEntry->onuE1Id     = e1Link.onuE1Id;

		if ( sizeof(e1Link.eponE1LocalEnable) < (E1_DESCRIPTION_MAX + 1) )
		{
			ptrE1LinkEntry->eponE1LocalEnable = e1Link.eponE1LocalEnable;
			memcpy(ptrE1LinkEntry->eponE1Description, e1Link.eponE1Description, E1_DESCRIPTION_MAX + 1);
		}

		ptrE1LinkEntry++;
		nvmDataLen += sizeof(nvm_e1LinkEntry_t);
		ptrE1LinkTable->e1LinkTableEntry_Num++;
	}
	E1_ERROR_INFO_PRINT((" E1-LINK Table: count=%d, length=%d\r\n", ptrE1LinkTable->e1LinkTableEntry_Num, nvmDataLen ));

	ptrE1PortTable = (nvm_e1PortTable_t *)ptrE1LinkEntry;
	ptrE1PortEntry = ptrE1PortTable->e1PortEntry;
	nvmDataLen    += sizeof(ptrE1PortTable->e1PortTableEntry_Num);
	ptrE1PortTable->e1PortTableEntry_Num = 0;

	for (idxs[2] = 0; idxs[2] < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); idxs[2]++)
	{
		if ( VOS_OK != tdm_e1PortTable_get(idxs, &e1Port) )
		{
			E1_ERROR_INFO_PRINT(("epon_e1_cfgdata_save::tdm_e1PortTable_get()   E1 Idx=%d   error!\r\n", idxs[2]));
			sys_console_printf("%% Get E1 port table Fail!\r\n  %% save E1 data failed!\r\n");
			g_free( pNvmE1DataBuf );
			return VOS_ERROR;
		}

		ptrE1PortEntry->eponE1PortAlarmStatus = e1Port.eponE1PortAlarmStatus;/* 状态不需要保存 chenfj */
		ptrE1PortEntry->eponE1PortAlarmMask   = e1Port.eponE1PortAlarmMask;
		ptrE1PortEntry->eponE1Loop            = e1Port.eponE1Loop; /* 环回设置也需要保存?  chenfj  */
		ptrE1PortEntry->eponE1TxClock         = e1Port.eponE1TxClock;

		ptrE1PortEntry++;
		nvmDataLen += sizeof(nvm_e1PortEntry_t);
		ptrE1PortTable->e1PortTableEntry_Num++;
	}
	E1_ERROR_INFO_PRINT((" E1-PORT Table: count=%d, length=%d\r\n", ptrE1PortTable->e1PortTableEntry_Num, nvmDataLen ));


	ptrE1VlanTable = (nvm_e1VlanTable_t *)ptrE1PortEntry;
	ptrE1VlanEntry = ptrE1VlanTable->e1VlanEntry;
	nvmDataLen    += sizeof(ptrE1VlanTable->e1VlanTableEntry_Num);
	ptrE1VlanTable->e1VlanTableEntry_Num = 0;

	for (idxs[2] = 0; idxs[2] < TDM_FPGA_MAX; idxs[2]++)
	{
		if ( VOS_OK != tdm_e1VlanTable_get(idxs, &e1Vlan) )
		{
			E1_ERROR_INFO_PRINT(("epon_e1_cfgdata_save::tdm_e1VlanTable_get()   E1 fpga Idx=%d   error!\r\n", idxs[2]));
			sys_console_printf("%% Get E1 vlan table Fail!\r\n  %% save E1 data failed!\r\n");
			g_free( pNvmE1DataBuf );
			return VOS_ERROR;
		}

		ptrE1VlanEntry->eponVlanEnable = e1Vlan.eponVlanEnable;
		ptrE1VlanEntry->eponVlanPri    = e1Vlan.eponVlanPri;
		ptrE1VlanEntry->eponVlanId     = e1Vlan.eponVlanId;

		ptrE1VlanEntry++;
		nvmDataLen += sizeof(nvm_e1VlanEntry_t);
		ptrE1VlanTable->e1VlanTableEntry_Num++;
	}
	E1_ERROR_INFO_PRINT((" E1-VLAN Table: count=%d, length=%d\r\n", ptrE1VlanTable->e1VlanTableEntry_Num, nvmDataLen ));

	/* 保存SW上onu的E1端口数据 */
	ptrSwOnuE1PortTable = (nvm_sw_onu_e1PortTable_t *)ptrE1VlanEntry;
	for (i = 0; i < MAXPON; i++)
	{
		for (j = 0; j < MAXONUPERPON; j++)
		{
			for (k = 0; k < MAX_ONU_E1_SLOT_ID; k++)
			{
				for (l = 0; l < MAX_ONU_BOARD_E1; l++)
				{
					ptrSwOnuE1PortTable->alarmmask[swOnuE1PortTableOffset++] = ExtBrdMgmtTable[i][j + 1].BrdMgmtTable[k].onuE1PortTable[l].eponE1PortAlarmMask;
				}
			}
		}
	}
	nvmDataLen += sizeof(nvm_sw_onu_e1PortTable_t);


	pDataHead =(nvm_e1NvmDataHead_t *)pNvmE1DataBuf;
	VOS_MemZero( pDataHead->nvmFlag, sizeof(pDataHead->nvmFlag) );
	VOS_MemCpy( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag) );
	E1_ERROR_INFO_PRINT(("nvm =%s\r\n", pDataHead->nvmFlag ));
	VOS_MemZero( ver_str, sizeof(ver_str) );
	GetOltSWVersion( ver_str, &len );
	E1_ERROR_INFO_PRINT(("ver is =%s, len=%d\r\n", ver_str, len ));
	VOS_MemCpy(pDataHead->nvmVersion, ver_str,/*"V2R1Z001", */sizeof(pDataHead->nvmVersion) );
	E1_ERROR_INFO_PRINT(("ver=%s\r\n", pDataHead->nvmVersion ));

	pDataHead->nvmDataSize = nvmDataLen;
	E1_ERROR_INFO_PRINT(("data len =%d\r\n", pDataHead->nvmDataSize));

	if( nvmDataLen > E1_CFG_FILE_LEN )
	{
		VOS_ASSERT( 0 );
		g_free( pNvmE1DataBuf );
		return VOS_ERROR;
	}

	V2R1_disable_watchdog();
	xflash_file_write( TDM_FLASH_FILE_ID, pNvmE1DataBuf, (int *)&nvmDataLen );
	V2R1_enable_watchdog();
	g_free( pNvmE1DataBuf );

	return VOS_OK;
}


/*从flash中恢复TDM 配置数据*/
STATUS epon_e1_cfgdata_retrieve()
{
	ULONG idx[3], i, j, k, l, swOnuE1PortTableOffset = 0;

	nvm_e1NvmDataHead_t   *pDataHead;

	nvm_e1LinkEntry_t *ptrE1LinkEntry;
	nvm_e1LinkTable_t *ptrE1LinkTable;

	nvm_e1PortEntry_t *ptrE1PortEntry;
	nvm_e1PortTable_t *ptrE1PortTable;

	nvm_e1VlanEntry_t *ptrE1VlanEntry;
	nvm_e1VlanTable_t *ptrE1VlanTable;

	nvm_sw_onu_e1PortTable_t *ptrSwOnuE1PortTable;

	UCHAR *pNvmE1DataBuf;
	ULONG  nvmDataLen = TDM_FLASH_FILE_LEN;

	pNvmE1DataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN);
	if( pNvmE1DataBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	sys_console_printf(" \r\nE1 data retrieve begin...\r\n");

	/*VOS_TaskDelay(30000);*/

	V2R1_disable_watchdog();
	xflash_file_read( TDM_FLASH_FILE_ID, pNvmE1DataBuf, (int *)&nvmDataLen );
	V2R1_enable_watchdog();

	pDataHead = (nvm_e1NvmDataHead_t *)pNvmE1DataBuf;

	E1_ERROR_INFO_PRINT(("\r\nnvmFlag:%c%c", pDataHead->nvmFlag[0], pDataHead->nvmFlag[1]));
	E1_ERROR_INFO_PRINT(( "\r\n nvmVersion=%s, nvmDataLen=%d\r\n", pDataHead->nvmVersion, pDataHead->nvmDataSize));

	if(VOS_MemCmp( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag)) != 0 )
	{
		sys_console_printf("NvmFlag:%s is not %s\r\n", pDataHead->nvmFlag, NVMFLAG);
		g_free(pNvmE1DataBuf);
		return  VOS_ERROR;
	}

	if( (nvmDataLen == 0) || (nvmDataLen > TDM_FLASH_FILE_LEN) )
	{
		sys_console_printf("E1 NVM file is empty\r\n");
		g_free(pNvmE1DataBuf);
		return  VOS_ERROR;
	}

	/*initSwE1Data(); OLT侧E1端口属性表初始化 */

	if( checkCfgDataVersion(pDataHead->nvmVersion) == NVM_CFG_DATA_VER_V1R07 )
	{
		idx[0] = 1;
		idx[1] = get_gfa_e1_slotno();
		idx[2] = 0;

		pDataHead++;

		ptrE1LinkTable = (nvm_e1LinkTable_t *)pDataHead;
		ptrE1LinkEntry = ptrE1LinkTable->e1LinkEntry;

		E1_ERROR_INFO_PRINT((" E1 Link num=%d\r\n", ptrE1LinkTable->e1LinkTableEntry_Num));

		if( ptrE1LinkTable->e1LinkTableEntry_Num <= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
		{
			for(i = 0; i < ptrE1LinkTable->e1LinkTableEntry_Num; i++)
			{
				if( VOS_OK != tdm_e1LinkTable_rowset(idx,ptrE1LinkEntry->onuDevId, ptrE1LinkEntry->onuE1SlotId,
					ptrE1LinkEntry->onuE1Id, ptrE1LinkEntry->eponE1LocalEnable, ptrE1LinkEntry->eponE1Description) )
				{
					E1_ERROR_INFO_PRINT(("set E1 Link Table: %d error\r\n", i)); 
					sys_console_printf("%% Set E1 link table Fail!\r\n  %% retrieve E1 data failed!\r\n");
					g_free(pNvmE1DataBuf);
					return VOS_ERROR;
				}

				idx[2]++;
				ptrE1LinkEntry++;
			}
		}

		idx[2] = 0;

		ptrE1PortTable = (nvm_e1PortTable_t *)ptrE1LinkEntry;
		ptrE1PortEntry = ptrE1PortTable->e1PortEntry;

		E1_ERROR_INFO_PRINT((" E1 Port num=%d\r\n", ptrE1PortTable->e1PortTableEntry_Num));

		if( ptrE1PortTable->e1PortTableEntry_Num <= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
		{
			for(i = 0; i < ptrE1PortTable->e1PortTableEntry_Num; i++)
			{
				/* 恢复TDM板上保存的e1 port data，环回和告警状态不恢复 */
				if( VOS_OK != tdm_e1PortTable_rowset(idx, ptrE1PortEntry->eponE1PortAlarmMask, 0/*ptrE1PortEntry->eponE1Loop*/, ptrE1PortEntry->eponE1TxClock) )
				{
					E1_ERROR_INFO_PRINT(("set tdm E1 Port Table: %d error\r\n", i));
					sys_console_printf("%% Set TDM E1 port table Fail!\r\n  %% retrieve E1 data failed!\r\n");
					g_free(pNvmE1DataBuf);
					return VOS_ERROR;
				}
				/* 恢复SW板上保存的e1 port data */
				if( VOS_OK != sw_e1PortTable_rowset(idx, ptrE1PortEntry->eponE1PortAlarmMask, 0/*ptrE1PortEntry->eponE1Loop*/, ptrE1PortEntry->eponE1TxClock) )
				{
					E1_ERROR_INFO_PRINT(("set sw E1 Port Table: %d error\r\n", i));
					sys_console_printf("%% Set SW E1 port table Fail!\r\n  %% retrieve E1 data failed!\r\n");
					g_free(pNvmE1DataBuf);
					return VOS_ERROR;
				}

				idx[2]++;
				ptrE1PortEntry++;
			}
		}

		idx[2] = 0;

		ptrE1VlanTable = (nvm_e1VlanTable_t *)ptrE1PortEntry;
		ptrE1VlanEntry = ptrE1VlanTable->e1VlanEntry;

		E1_ERROR_INFO_PRINT((" E1 Vlan num=%d\r\n", ptrE1VlanTable->e1VlanTableEntry_Num));

		if( ptrE1VlanTable->e1VlanTableEntry_Num <= TDM_FPGA_MAX )
		{
			for(i = 0; i < ptrE1VlanTable->e1VlanTableEntry_Num; i++)
			{
				if( VOS_OK != tdm_e1VlanTable_rowset(idx, ptrE1VlanEntry->eponVlanEnable, ptrE1VlanEntry->eponVlanPri, ptrE1VlanEntry->eponVlanId) )
				{
					E1_ERROR_INFO_PRINT(("set E1 Vlan Table: %d error\r\n", i));
					sys_console_printf("%% Set E1 vlan table Fail!\r\n  %% retrieve E1 data failed!\r\n");
					g_free(pNvmE1DataBuf);
					return VOS_ERROR;
				}

				idx[2]++;
				ptrE1VlanEntry++;
			}
		}

		/* 恢复SW上onu的E1端口数据 */
		ptrSwOnuE1PortTable = (nvm_sw_onu_e1PortTable_t *)ptrE1VlanEntry;
		for (i = 0; i < MAXPON; i++)
		{
			for (j = 0; j < MAXONUPERPON; j++)
			{
				for (k = 0; k < MAX_ONU_E1_SLOT_ID; k++)
				{
					for (l = 0; l < MAX_ONU_BOARD_E1; l++)
					{
						ExtBrdMgmtTable[i][j + 1].BrdMgmtTable[k].onuE1PortTable[l].eponE1PortAlarmMask = ptrSwOnuE1PortTable->alarmmask[swOnuE1PortTableOffset++];
						ExtBrdMgmtTable[i][j + 1].BrdMgmtTable[k].onuE1PortTable[l].eponE1Loop = 0;
					}
				}
			}
		}

	}

	g_free(pNvmE1DataBuf);

	sys_console_printf(" \r\nE1 data retrieve OK!\r\n");
	
	return VOS_OK;
}

/*显示从flash中读出的E1配置数据*/
STATUS printE1cfgData(struct vty *vty)
{
	int i;
	USHORT eponE1PortAlarmStatus = 0x0000;

	nvm_e1NvmDataHead_t   *pDataHead;

	nvm_e1LinkEntry_t *ptrE1LinkEntry;
	nvm_e1LinkTable_t *ptrE1LinkTable;

	nvm_e1PortEntry_t *ptrE1PortEntry;
	nvm_e1PortTable_t *ptrE1PortTable;

	nvm_e1VlanEntry_t *ptrE1VlanEntry;
	nvm_e1VlanTable_t *ptrE1VlanTable;

	UCHAR *ptrSwOnuE1PortTable;

	UCHAR *pNvmE1DataBuf;
	ULONG  nvmDataLen = TDM_FLASH_FILE_LEN;

	pNvmE1DataBuf = (UCHAR *)g_malloc(TDM_FLASH_FILE_LEN);
	if( pNvmE1DataBuf == NULL )
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}

	/*vty_out( vty, " \r\nTDM data retrieve begin...\r\n" );*/

	V2R1_disable_watchdog();
	xflash_file_read( TDM_FLASH_FILE_ID, pNvmE1DataBuf, (int *)&nvmDataLen );
	V2R1_enable_watchdog();

	pDataHead = (nvm_e1NvmDataHead_t *)pNvmE1DataBuf;

	if(VOS_MemCmp( pDataHead->nvmFlag, NVMFLAG, sizeof(pDataHead->nvmFlag)) != 0 )
	{
		vty_out(vty,"NvmFlag:%s is not %s\r\n", pDataHead->nvmFlag, NVMFLAG);
		g_free(pNvmE1DataBuf);
		return  VOS_ERROR;
	}

	if( (nvmDataLen == 0) || (nvmDataLen > TDM_FLASH_FILE_LEN) )
	{
		vty_out(vty,"E1 config file in flash is empty.\r\n");
		g_free(pNvmE1DataBuf);
		return  VOS_ERROR;
	}

	vty_out( vty, "\r\nnvmFlag:%c%c", pDataHead->nvmFlag[0], pDataHead->nvmFlag[1]);
	vty_out( vty, "\r\n nvmVersion=%s, nvmDataLen=%d\r\n", pDataHead->nvmVersion, pDataHead->nvmDataSize);


	if( checkCfgDataVersion(pDataHead->nvmVersion) == NVM_CFG_DATA_VER_V1R07 )
	{
		pDataHead++;

		ptrE1LinkTable = (nvm_e1LinkTable_t *)pDataHead;
		ptrE1LinkEntry = ptrE1LinkTable->e1LinkEntry;

		vty_out( vty, "\r\n E1 Link (num=%d)\r\n", ptrE1LinkTable->e1LinkTableEntry_Num);

		/*vty_out( vty, "*************************eponE1LinkTable************************\r\n");*/
		vty_out( vty, "Idx  OnuDevId  SlotId  OnuE1Id  Enable  Description\r\n");
		/*vty_out( vty, "----------------------------------------------------------------\r\n");*/

		if( ptrE1LinkTable->e1LinkTableEntry_Num <= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
		{
			for(i = 0; i < ptrE1LinkTable->e1LinkTableEntry_Num; i++)
			{
				vty_out( vty, "%02d   %07ld   %03d     %03d      %d       %s\r\n", i, ptrE1LinkEntry->onuDevId, ptrE1LinkEntry->onuE1SlotId, ptrE1LinkEntry->onuE1Id + 1, ptrE1LinkEntry->eponE1LocalEnable, ptrE1LinkEntry->eponE1Description);

				ptrE1LinkEntry++;
			}
		}

		ptrE1PortTable = (nvm_e1PortTable_t *)ptrE1LinkEntry;
		ptrE1PortEntry = ptrE1PortTable->e1PortEntry;

		vty_out( vty, "\r\n E1 Port (num=%d)\r\n", ptrE1PortTable->e1PortTableEntry_Num);

		/*vty_out( vty, "*************************eponE1PortTable************************\r\n");*/
		vty_out( vty, "Idx     AlarmStat       AlarmMask         Loop           TxClock\r\n");
		/*vty_out( vty, "----------------------------------------------------------------\r\n");*/

		if( ptrE1PortTable->e1PortTableEntry_Num <= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
		{
			for(i = 0; i < ptrE1PortTable->e1PortTableEntry_Num; i++)
			{
				vty_out( vty, "%02d      0x%04x          0x%04x            0x%02x           %d\r\n", i, eponE1PortAlarmStatus, ptrE1PortEntry->eponE1PortAlarmMask, ptrE1PortEntry->eponE1Loop, ptrE1PortEntry->eponE1TxClock);

				ptrE1PortEntry++;
			}
		}

		ptrE1VlanTable = (nvm_e1VlanTable_t *)ptrE1PortEntry;
		ptrE1VlanEntry = ptrE1VlanTable->e1VlanEntry;

		vty_out( vty, "\r\n E1 Vlan (num=%d)\r\n", ptrE1VlanTable->e1VlanTableEntry_Num);

		/*vty_out( vty, "***************eponE1VlanTable**************\r\n");*/
		vty_out( vty, "FpgaIndex    VlanEnable    VlanPri   VlanId\r\n");
		/*vty_out( vty, "--------------------------------------------\r\n");*/

		if( ptrE1VlanTable->e1VlanTableEntry_Num <= TDM_FPGA_MAX )
		{
			for(i = 0; i < ptrE1VlanTable->e1VlanTableEntry_Num; i++)
			{
				vty_out( vty, "%d            %d             %d         %d\r\n", i, ptrE1VlanEntry->eponVlanEnable, ptrE1VlanEntry->eponVlanPri, ptrE1VlanEntry->eponVlanId);

				ptrE1VlanEntry++;
			}
		}

		ptrSwOnuE1PortTable = (UCHAR *)ptrE1VlanEntry;
		/*vty_out( vty, "*************Onu E1 Port Alarm Mask**********\r\n");*/
		/*for (i = 0; i < sizeof(nvm_sw_onu_e1PortTable_t); i++)
		{
			if (i % 16 == 0)
			{
				vty_out( vty, "\r\n");
			}
			vty_out( vty, "%02x ", ptrSwOnuE1PortTable[i]);
		}*/
		vty_out( vty, "\r\n");
	}

	g_free(pNvmE1DataBuf);
	return VOS_OK;
}

/*显示TDM板上运行的数据*/
STATUS printE1runningData(struct vty *vty)
{
	ULONG idxs[3];
	int i;

	e1LinkTable_row_entry e1Link;
	e1PortTable_row_entry e1Port;
	e1VlanTable_row_entry e1Vlan;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = 0;

	vty_out( vty, "\r\n E1 Link\r\n");
	/*vty_out( vty, "*************************eponE1LinkTable************************\r\n");*/
	vty_out( vty, "Idx  OnuDevId  SlotId  OnuE1Id  Enable  Description\r\n");
	/*vty_out( vty, "----------------------------------------------------------------\r\n");*/

	for (i = 0; i < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); i++)
	{
		if (VOS_OK != tdm_e1LinkTable_get( idxs, &e1Link ))
		{
			E1_ERROR_INFO_PRINT(("printE1runningData()::tdm_e1LinkTable_get() error! index=%d\r\n", i));
			/*sys_console_printf("%% Get E1 link table Fail!\r\n  %% print E1 data failed!\r\n");*/
			return VOS_ERROR;
		}

		vty_out( vty, "%02d   %07ld   %03d     %03d      %d       %s\r\n", i, e1Link.onuDevId, e1Link.onuE1SlotId, e1Link.onuE1Id + 1, e1Link.eponE1LocalEnable, e1Link.eponE1Description);

		idxs[2]++;
	}

	idxs[2] = 0;

	vty_out( vty, "\r\n E1 Port\r\n");
	/*vty_out( vty, "*************************eponE1PortTable************************\r\n");*/
	vty_out( vty, "Idx     AlarmStat       AlarmMask         Loop           TxClock\r\n");
	/*vty_out( vty, "----------------------------------------------------------------\r\n");*/

	for (i = 0; i < (MAX_E1_PER_FPGA * TDM_FPGA_MAX); i++)
	{
		if (VOS_OK != tdm_e1PortTable_get( idxs, &e1Port ))
		{
			E1_ERROR_INFO_PRINT(("printE1runningData()::tdm_e1PortTable_get() error! index=%d\r\n", i));
			/*sys_console_printf("%% Get E1 port table Fail!\r\n  %% print E1 data failed!\r\n");*/
			return VOS_ERROR;
		}

		vty_out( vty, "%02d      0x%04x          0x%04x            0x%02x           %d\r\n", i, e1Port.eponE1PortAlarmStatus, e1Port.eponE1PortAlarmMask, e1Port.eponE1Loop, e1Port.eponE1TxClock);

		idxs[2]++;
	}

	idxs[2] = 0;

	vty_out( vty, "\r\n E1 Vlan\r\n");
	/*vty_out( vty, "***************eponE1VlanTable**************\r\n");*/
	vty_out( vty, "FpgaIndex    VlanEnable    VlanPri   VlanId\r\n");
	/*vty_out( vty, "--------------------------------------------\r\n");*/

	for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		if (VOS_OK != tdm_e1VlanTable_get( idxs, &e1Vlan ))
		{
			E1_ERROR_INFO_PRINT(("printE1runningData()::tdm_e1VlanTable_get() error! index=%d\r\n", i));
			/*sys_console_printf("%% Get E1 vlan table Fail!\r\n  %% print E1 data failed!\r\n");*/
			return VOS_ERROR;
		}

		vty_out( vty, "%d            %d             %d         %d\r\n", i, e1Vlan.eponVlanEnable, e1Vlan.eponVlanPri, e1Vlan.eponVlanId);

		idxs[2]++;
	}

	return VOS_OK;
}



DEFUN(
	  save_E1_cfg_data,
	  save_E1_cfg_data_cmd,
	  "save e1-config",
	  "save e1-config data\n"
	  "save e1 config data\n"
	  )
{
	if(VOS_OK == epon_e1_cfgdata_save())
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(
	  retrieve_E1_cfg_data,
	  retrieve_E1_cfg_data_cmd,
	  "retrieve e1-config",
	  "retrieve e1-config data\n"
	  "retrieve e1 config data\n"
	  )
{
	if(VOS_OK == epon_e1_cfgdata_retrieve())
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

/* deleted by chenfj 2009- 3-24 
     删除以下命令, 分别与show tdm-running-config 等命令合并*/
     
DEFUN(
	  show_E1_cfg_data,
	  show_E1_cfg_data_cmd,
	  "show e1-config",
	  "show e1 config data\n"
	  "show e1 config data\n"
	  )
{
	if(VOS_OK == printE1cfgData(vty))
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(
	  show_E1_running_data,
	  show_E1_running_data_cmd,
	  "show e1-running-config",
	  "show e1 running data\n"
	  "show e1 running data\n"
	  )
{
	if(VOS_OK == printE1runningData(vty))
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN(
	  erase_E1_cfg_data,
	  erase_E1_cfg_data_cmd,
	  "erase e1-config",
	  "erase e1 config data\n"
	  "erase e1 config data\n"
	  )
{
	if(VOS_OK == dataSync_FlashEraseFile(SYNC_FILETYPE_CFG_TDM))
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

LONG e1_DataSave_CommandInstall(void)
{	
	install_element ( E1_NODE, &save_E1_cfg_data_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &retrieve_E1_cfg_data_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_E1_cfg_data_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &show_E1_running_data_cmd );
	install_element ( E1_NODE, &erase_E1_cfg_data_cmd );

	return VOS_OK;
}

#undef NVMFLAG

#endif
