#ifndef __INCsyncMainh
#define __INCsyncMainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	SYNC_MSGTYPE_CONFIG			1		/* 配置数据同步*/
#define	SYNC_MSGTYPE_OLT_FILE		2		/* OLT 升级文件同步*/
#define	SYNC_MSGTYPE_ONU_FILE		3		/* ONU升级文件同步*/
#define	SYNC_MSGTYPE_SWITCHOVER		100		/* 主备倒换*/
#define	SYNC_MSGTYPE_SWITCHOVER1	101		/* 主备倒换1 */
#define	SYNC_MSGTYPE_TEST			200		/* 测试 */
#define	SYNC_MSGTYPE_GET_VER			201		/* 测试 */

#define	SYNC_MSGCODE_NEGOTIATION	1		/* 同步协商*/
#define	SYNC_MSGCODE_REQUEST		2		/* 同步请求*/
#define	SYNC_MSGCODE_NOTIFY			3		/* 同步通知*/
#define	SYNC_MSGCODE_LOADING		4		/* 数据传输*/
#define	SYNC_MSGCODE_LOADED			5		/* 数据传输结束*/
#define	SYNC_MSGCODE_FINISH			6		/* 同步完成*/
#define	SYNC_MSGCODE_RESET			100		/* 同步复位*/

#define	SYNC_MSGRESULT_REQACK		1		/**/
#define	SYNC_MSGRESULT_ACK			2		/**/
#define	SYNC_MSGRESULT_NAK			3		/**/

typedef struct{
	USHORT	srcSlotNo;		/* 源槽位号 */
	USHORT	dstSlotNo;		/* 目的槽位号 */
	ULONG	srcModuleID; 	/* 源模块号 */
	ULONG	dstModuleID; 	/* 目的模块号 */
	USHORT	msgType; 		/* 消息类型 */
	USHORT	msgCode; 		/* 消息码 */
	ULONG	msgResult; 		/* 消息响应结果 */
	ULONG	msgData; 		/* 消息附加内容 */
} eponSyncMsgHead_t;


typedef enum {
	SYNC_FILETYPE_NULL = 0,
	SYNC_FILETYPE_CFG_TDM,			/* TDM 配置数据 */
	SYNC_FILETYPE_CFG_CTC,			/* CTC ONU 配置数据 */
	SYNC_FILETYPE_CFG_DATA,                  /*所有SHOW RUN 中保存的数据和命令*/
	SYNC_FILETYPE_APP_SW,				/* OLT 主控GFA-SW板APP 软件 */
	SYNC_FILETYPE_BOOT_SW,			/* GFA-SW BSP */
	SYNC_FILETYPE_PON_FIRM,			/* OLT  PON firmware */
	SYNC_FILETYPE_PON_DBA,			/* OLT  PON DBA */
	SYNC_FILETYPE_ONU,				/* 所有类型的ONU 软件和固件 */
	

	
	SYNC_FILETYPE_CFG_BATFILE,		/* batfile数据 */
	SYNC_FILETYPE_DRV_TDM,			/* OLT GFA-TDM板软件和FPGA */
	SYNC_FILETYPE_BOOT_TDM,			/* GFA-TDM BSP */
	SYNC_FILETYPE_SYSFILE,                    /*系统文件*//*add by shixh20090605*/
	SYNC_FILETYPE_MAX
}syncFileType_t;

typedef struct {
	ULONG	fileType; 		/* 文件类型 */
	ULONG	totalPktCount; 	/* 需要发送总包数 */
	ULONG	curPktId; 		/* 当前报文编号，0～ulTotalPktCount－1 */
	ULONG	pktDataLen;		/* 报文承载有效负荷长度 */
} eponSyncFileHead_t;

typedef struct {
	int reserved;
	char	 *buffer; 		
	ULONG	len; 		
	syncFileType_t filetype; 	
} saveFilePkt_t;
typedef struct {
	eponSyncMsgHead_t	msgHead; 		/* 同步消息头 */
	eponSyncFileHead_t	fileHead; 		/* 同步文件信息头 */
	ULONG		fileData; 	/* 同步文件分片数据 */
} eponSyncFilePkt_t;

#if 0
typedef struct {
UCHAR	nvmFlag[3]; 				/* 数据有效性标志，固定值"TDM"*/
UCHAR	nvmVersion[9];			/* 软件版本号，格式"VxRyyBzzz"*/
ULONG	nvmDataSize; 			/* 存储数据长度，不包括头自身长度 */
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
