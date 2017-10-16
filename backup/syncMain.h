#ifndef __INCsyncMainh
#define __INCsyncMainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	SYNC_MSGTYPE_CONFIG			1		/* ��������ͬ��*/
#define	SYNC_MSGTYPE_OLT_FILE		2		/* OLT �����ļ�ͬ��*/
#define	SYNC_MSGTYPE_ONU_FILE		3		/* ONU�����ļ�ͬ��*/
#define	SYNC_MSGTYPE_SWITCHOVER		100		/* ��������*/
#define	SYNC_MSGTYPE_SWITCHOVER1	101		/* ��������1 */
#define	SYNC_MSGTYPE_TEST			200		/* ���� */
#define	SYNC_MSGTYPE_GET_VER			201		/* ���� */

#define	SYNC_MSGCODE_NEGOTIATION	1		/* ͬ��Э��*/
#define	SYNC_MSGCODE_REQUEST		2		/* ͬ������*/
#define	SYNC_MSGCODE_NOTIFY			3		/* ͬ��֪ͨ*/
#define	SYNC_MSGCODE_LOADING		4		/* ���ݴ���*/
#define	SYNC_MSGCODE_LOADED			5		/* ���ݴ������*/
#define	SYNC_MSGCODE_FINISH			6		/* ͬ�����*/
#define	SYNC_MSGCODE_RESET			100		/* ͬ����λ*/

#define	SYNC_MSGRESULT_REQACK		1		/**/
#define	SYNC_MSGRESULT_ACK			2		/**/
#define	SYNC_MSGRESULT_NAK			3		/**/

typedef struct{
	USHORT	srcSlotNo;		/* Դ��λ�� */
	USHORT	dstSlotNo;		/* Ŀ�Ĳ�λ�� */
	ULONG	srcModuleID; 	/* Դģ��� */
	ULONG	dstModuleID; 	/* Ŀ��ģ��� */
	USHORT	msgType; 		/* ��Ϣ���� */
	USHORT	msgCode; 		/* ��Ϣ�� */
	ULONG	msgResult; 		/* ��Ϣ��Ӧ��� */
	ULONG	msgData; 		/* ��Ϣ�������� */
} eponSyncMsgHead_t;


typedef enum {
	SYNC_FILETYPE_NULL = 0,
	SYNC_FILETYPE_CFG_TDM,			/* TDM �������� */
	SYNC_FILETYPE_CFG_CTC,			/* CTC ONU �������� */
	SYNC_FILETYPE_CFG_DATA,                  /*����SHOW RUN �б�������ݺ�����*/
	SYNC_FILETYPE_APP_SW,				/* OLT ����GFA-SW��APP ��� */
	SYNC_FILETYPE_BOOT_SW,			/* GFA-SW BSP */
	SYNC_FILETYPE_PON_FIRM,			/* OLT  PON firmware */
	SYNC_FILETYPE_PON_DBA,			/* OLT  PON DBA */
	SYNC_FILETYPE_ONU,				/* �������͵�ONU ����͹̼� */
	

	
	SYNC_FILETYPE_CFG_BATFILE,		/* batfile���� */
	SYNC_FILETYPE_DRV_TDM,			/* OLT GFA-TDM�������FPGA */
	SYNC_FILETYPE_BOOT_TDM,			/* GFA-TDM BSP */
	SYNC_FILETYPE_SYSFILE,                    /*ϵͳ�ļ�*//*add by shixh20090605*/
	SYNC_FILETYPE_MAX
}syncFileType_t;

typedef struct {
	ULONG	fileType; 		/* �ļ����� */
	ULONG	totalPktCount; 	/* ��Ҫ�����ܰ��� */
	ULONG	curPktId; 		/* ��ǰ���ı�ţ�0��ulTotalPktCount��1 */
	ULONG	pktDataLen;		/* ���ĳ�����Ч���ɳ��� */
} eponSyncFileHead_t;

typedef struct {
	int reserved;
	char	 *buffer; 		
	ULONG	len; 		
	syncFileType_t filetype; 	
} saveFilePkt_t;
typedef struct {
	eponSyncMsgHead_t	msgHead; 		/* ͬ����Ϣͷ */
	eponSyncFileHead_t	fileHead; 		/* ͬ���ļ���Ϣͷ */
	ULONG		fileData; 	/* ͬ���ļ���Ƭ���� */
} eponSyncFilePkt_t;

#if 0
typedef struct {
UCHAR	nvmFlag[3]; 				/* ������Ч�Ա�־���̶�ֵ"TDM"*/
UCHAR	nvmVersion[9];			/* ����汾�ţ���ʽ"VxRyyBzzz"*/
ULONG	nvmDataSize; 			/* �洢���ݳ��ȣ�������ͷ������ */
}__attribute__((packed)) nvm_tdmNvmDataHead_t;
#endif


#define TDM_FLASH_FILE_ID		8
#define CTC_FLASH_FILE_ID		8
#define CFG_DATA_FLASFH_FILE_ID   2
#define  APP_SW_FLASH_FILE_ID        11
#define  BOOT_SW_FLASH_FILE_ID       6
#define  PON_FIRM_FLASH_FILE_ID     7
#define  PON_DBA_FLASH_FILE_ID     10
#define  ONU_FLASH_FILE_ID               5

#define TDM_FLASH_FILE_LEN		0xc0000
#define CTC_FLASH_FILE_LEN		0xc0000	/*0x100000*/	/* modified by xieshl 20080827 */
#define CFG_DATA_FLASFH_FILE_LEN    0x140000
#define  APP_SW_FLASH_FILE_LEN         (0x4c0000 + 0xc0000)
#define  BOOT_SW_FLASH_FILE_LEN       0x80000
#define  PON_FIRM_FLASH_FILE_LEN      0x80000
#define  PON_DBA_FLASH_FILE_LEN       0x40000
#define  ONU_FLASH_FILE_LEN               0x600000
#define  SYSTERM_FILE_LEN                    0x10000 /*add by shixh20090605*/


#ifndef ROM_BASE_ADRS
#define ROM_BASE_ADRS         0xfff00000              /* base address of ROM  */
#endif
#if (RPU_MODULE_HOT ==  RPU_YES )
extern STATUS dataSyncInit();
#endif
extern STATUS config_sync_notify_event();
extern ULONG get_gfa_tdm_slotno();
extern STATUS dataSync_FlashEraseFile( syncFileType_t fileType );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCsyncMainh */
