
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_ONUh
#define __INCCT_RMan_ONUh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*---------------------------------------------------------------------------------------*/

/*#ifdef __CT_EXTOAM_SUPPORT*/
#if 0
typedef struct {
	UCHAR OUI[3];
	UCHAR extOpcode;
	UCHAR branch;
	USHORT leaf;
} __attribute__((packed))  CT_ExtOam_PduData_ONU_GetReq_t;

typedef struct {
	UCHAR vendor_Id[4];
	ULONG ONU_Model;
	UCHAR ONU_ID[6];
	UCHAR hardwareVersion[8];
	UCHAR softwareVersion[16];
} __attribute__((packed))  CT_RMan_OnuSN_t;

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	CT_RMan_OnuSN_t SN;
} __attribute__((packed))  CT_ExtOam_PduData_OnuSN_GetResp_t;

/*---------------------------------------------------------------------------------------*/

typedef struct {
	UCHAR Version[32];
} __attribute__((packed))  CT_RMan_FirmwareVer_t;

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	CT_RMan_FirmwareVer_t FirmwareVer;
} __attribute__((packed))  CT_ExtOam_PduData_FirmwareVer_GetResp_t;

/*---------------------------------------------------------------------------------------*/

typedef struct {
	USHORT Vendor_ID;
	USHORT Chip_Model;
	UCHAR  Revision;
	UCHAR  IC_Version[3];
} __attribute__((packed))  CT_RMan_ChipsetId_t;

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	CT_RMan_ChipsetId_t ChipsetId;
} __attribute__((packed))  CT_ExtOam_PduData_ChipsetId_GetResp_t;

/*---------------------------------------------------------------------------------------*/

typedef struct {
	UCHAR  ServiceSupported;
	UCHAR  Number_of_GE_Ports;
	UCHAR  Bitmap_of_GE_Ports[8];
	UCHAR  Number_of_FE_Ports;
	UCHAR  Bitmap_of_FE_Ports[8];
	UCHAR  Number_of_POTS_ports;
	UCHAR  Number_of_E1_port;
	UCHAR  Number_of_US_Queues;
	UCHAR  QueueMax_per_US_Port;
	UCHAR  Number_of_DS_Queues;
	UCHAR  QueueMax_per_DS_Port;
	UCHAR  Battery_Backup;
} __attribute__((packed))  CT_RMan_Capabilities_t;

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	CT_RMan_Capabilities_t Capabilities;
} __attribute__((packed))  CT_ExtOam_PduData_Capabilities_GetResp_t;


/*---------------------------------------------------------------------------------------*/

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	UCHAR FECAbility;
} __attribute__((packed))  CT_ExtOam_PduData_FECAbility_GetResp_t;

/*---------------------------------------------------------------------------------------*/

typedef struct {
	CT_ExtOam_PduData_ONU_GetReq_t descriptor;
	UCHAR variable_Width;
	UCHAR FECMode;
} __attribute__((packed))  CT_ExtOam_PduData_FECMode_GetResp_t;

typedef CT_ExtOam_PduData_FECMode_GetResp_t CT_ExtOam_PduData_FECMode_SetReq_t;
typedef CT_ExtOam_PduData_FECMode_GetResp_t CT_ExtOam_PduData_FECMode_SetResp_t;

/*---------------------------------------------------------------------------------------*/

extern int CT_RMan_OnuSN_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag, 
			ULONG call_notify, CT_RMan_OnuSN_t* pSN );
extern int CT_RMan_FirmwareVer_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_FirmwareVer_t* pFirmwareVer );
extern int CT_RMan_ChipsetId_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_ChipsetId_t* pChipsetId );
extern int CT_RMan_Capabilities_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_Capabilities_t* pCapabilities );
extern int CT_RMan_FECAbility_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag, 
			ULONG call_notify, UCHAR *pFECAbility );
extern int CT_RMan_FECMode_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR *pFECMode );
extern int CT_RMan_FECMode_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR FECMode );

#endif	/* __CT_EXTOAM_SUPPORT */

/*---------------------------------------------------------------------------------------*/




/**************************************************************************************************
 * 函    数：CTC_GetOnuCapability
 * 说    明：获取ONU能力参数，并将结果保存在ONU结构数组中
 * 入口参数：PonPortIdx - PON 索引号，0 - 19
 *           OnuIdx     - ONU 索引号, 0 - 63
 * 出口参数: 无
 * 返 回 值：ROK/RERROR
 **************************************************************************************************/
int CTC_GetOnuCapability(short int PonPortIdx, short int OnuIdx );

int Downstream_queues_number_Get( short int PonPortIdx, short int OnuIdx, int *value );

int MaxDownStreamQueuesPerPort_Get( short int PonPortIdx, short int OnuIdx, int *value );
int BatteryBackup_Get( short int PonPortIdx, short int OnuIdx, int *value );

int CTC_StartOnuEncryption( short int PonPortIdx, short int OnuIdx );

int CTC_StopOnuEncrypt( short int PonPortIdx, short int OnuIdx );

int CTC_EncryptionTiming_Get( unsigned char *update_key_time, unsigned short int *no_reply_time_out );
/* 设置和读取ONU认证模式 */
int SetOnuAuthenticationMode( short int PonPortIdx, unsigned char mode );
int GetOnuAuthenticationMode( short int PonPortIdx, unsigned char *mode );
int AddMacAuthentication( short int PonPortIdx,  mac_address_t mac );
int DeleteMacAuthentication( short int PonPortIdx, mac_address_t mac );
int GetMacAuthentication( short int PonPortIdx, unsigned char *numberOfMac, mac_addresses_list_t mac_address_list );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_ONUh */
