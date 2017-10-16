
#ifndef _ONUAUTOCFG_H
#define  _ONUAUTOCFG_H

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )


#define V2R1_ONU_OFFLINE  1

#define MODULE_ONU_AUTO_LOAD	MODULE_RPU_ONU		/*0X8780*/

#define AUTO_LOAD_TYPE_NULL		0x00
#define AUTO_LOAD_TYPE_CFG		0x01
#define AUTO_LOAD_TYPE_UPG		0x02
#define AUTO_LOAD_TYPE_ALL		(AUTO_LOAD_TYPE_CFG | AUTO_LOAD_TYPE_UPG)


#define AUTO_LOAD_OAM_REQ			1
#define AUTO_LOAD_OAM_RSP			1
#define AUTO_LOAD_OAM_ACTIVE_REQ	12
#define AUTO_LOAD_OAM_ACTIVE_RSP	12
#define AUTO_LOAD_OAM_FINISH_REQ	11
#define AUTO_LOAD_OAM_FINISH_RSP	11

#define  AUTO_LOAD_STATE_USING		1
#define  AUTO_LOAD_STATE_NOT_USING	0

#define  AUTO_LOAD_FTP_CLIENT_IPNUM	5

#define AUTO_LOAD_FTP_CTRL_PORT		21
#define AUTO_LOAD_FTP_DATA_PORT		20
#define AUTO_LOAD_FTP_IP_STR_LEN		16
#define AUTO_LOAD_FTP_IPMASK_STR_LEN	20
#define AUTO_LOAD_FTP_USER_LEN		32
#define AUTO_LOAD_FTP_PASSWORD_LEN	32
#define AUTO_LOAD_FTP_FILENAME_LEN	32

typedef struct{
	UCHAR  serverIp[AUTO_LOAD_FTP_IP_STR_LEN+2];
	UCHAR  userName[AUTO_LOAD_FTP_USER_LEN];
	UCHAR  password[AUTO_LOAD_FTP_PASSWORD_LEN];	
	USHORT port;
	USHORT dataPort;
}__attribute__((packed))  auto_load_ftpserver_t;

typedef struct{
	UCHAR ipMask[AUTO_LOAD_FTP_IPMASK_STR_LEN];
	UCHAR ipGateway[AUTO_LOAD_FTP_IP_STR_LEN+2];
	USHORT vid;
}__attribute__((packed)) auto_load_ftpclient_t; 

typedef struct{
	auto_load_ftpclient_t ftpClient;
	USHORT timecount; 
	UCHAR  status;/*表示当前IP是否正在被使用*/
	UCHAR  rowStatus;/*表示是否有可用的IP*/
	ULONG  onuDevIdx;
}__attribute__((packed)) auto_load_onuctrl_t; 


typedef struct  {
	UCHAR  onu_cfg_status;
	UCHAR  onu_cfg_ftp_direct;
	UCHAR  onu_upg_status;
	UCHAR  onu_upg_ftp_direct;		/* 保留 */
	UCHAR  onu_cfg_filename[AUTO_LOAD_FTP_FILENAME_LEN];
} auto_load_onu_list_t;


#define   AUTO_LOAD_FILETYPE_NULL		0x00
#define   AUTO_LOAD_FILETYPE_CFG		0x01  
#define   AUTO_LOAD_FILETYPE_BOOT		0x02
#define   AUTO_LOAD_FILETYPE_APP		0x04
#define   AUTO_LOAD_FILETYPE_FW		0x08
#define   AUTO_LOAD_FILETYPE_VOIP		0x10
#define   AUTO_LOAD_FILETYPE_FPGA		0x20
#define   AUTO_LOAD_FILETYPE_UPG		(AUTO_LOAD_FILETYPE_BOOT|AUTO_LOAD_FILETYPE_APP|AUTO_LOAD_FILETYPE_FW|AUTO_LOAD_FILETYPE_VOIP|AUTO_LOAD_FILETYPE_FPGA)

#define   AUTO_LOAD_FILETYPE_COUNT	6

#define  AUTO_LOAD_STATE_IDLE			0
#define  AUTO_LOAD_STATE_WAIT			1
#define  AUTO_LOAD_STATE_FINISHED		2
#define  AUTO_LOAD_STATE_PROCESSING	3
#define  AUTO_LOAD_STATE_FAILURE		4
#define  AUTO_LOAD_STATE_ABORT		5


#define AUTO_LOAD_DIR_SERVER2ONU		2
#define AUTO_LOAD_DIR_ONU2SERVER		1
#define AUTO_LOAD_DIR_DEFAULT			AUTO_LOAD_DIR_SERVER2ONU


typedef struct  {
	int  filetype;
	UCHAR  ver[32];
}onu_file_type_t;

typedef struct  {
	onu_file_type_t file_type[AUTO_LOAD_FILETYPE_COUNT];
	int match;
} auto_load_filetype_list_t;

extern auto_load_ftpserver_t  auto_load_ftpserver;
extern auto_load_onuctrl_t  auto_load_onuctrl[AUTO_LOAD_FTP_CLIENT_IPNUM];
extern auto_load_filetype_list_t  auto_load_filetype_list[V2R1_ONU_MAX];

typedef struct {
	ULONG fileType;
	UCHAR loadDir;
	UCHAR fileNameLen;
	UCHAR fileName[AUTO_LOAD_FTP_FILENAME_LEN];
} __attribute__((packed)) auto_load_file_list_t;
#define AUTO_LOAD_FTP_FILELIST_LEN	(AUTO_LOAD_FILETYPE_COUNT * sizeof(auto_load_file_list_t) + 1)		/* 保留1 个字节 */

typedef struct {
	UCHAR msgType;
	UCHAR result;
	UCHAR ftpServerIp[AUTO_LOAD_FTP_IP_STR_LEN];
	USHORT ftpPort;
	USHORT ftpDataPort;
	UCHAR  userName[AUTO_LOAD_FTP_USER_LEN];
	UCHAR  password[AUTO_LOAD_FTP_PASSWORD_LEN];	
	USHORT vid;
	UCHAR clientIp[AUTO_LOAD_FTP_IPMASK_STR_LEN];
	UCHAR clientGateway[AUTO_LOAD_FTP_IP_STR_LEN];
	UCHAR fileTotalCount;
	UCHAR fileListBuf[AUTO_LOAD_FTP_FILELIST_LEN];
} __attribute__((packed)) auto_load_resource_oam_pdu_t;


typedef struct {
	ULONG onuDevIdx;
	UCHAR result;
	UCHAR slot;
	UCHAR pon;
	UCHAR onu;
	UCHAR data[4];
} __attribute__((packed)) auto_load_msg_t;

#define AUTOLOAD_OAM_PDU_LEN		sizeof(auto_load_resource_oam_pdu_t)
typedef struct {
	ULONG onuDevIdx;
	UCHAR result;
	UCHAR slot;
	UCHAR pon;
	UCHAR onu;
	UCHAR data[4];
	UCHAR file_name[AUTO_LOAD_FILETYPE_COUNT][64];
	UCHAR pdu[AUTOLOAD_OAM_PDU_LEN];
	SHORT pduLen;
} __attribute__((packed)) auto_load_resource_msg_t;

#define	AUTO_LOAD_MSG_CODE_STARTUP_TIMER		100
#define	AUTO_LOAD_MSG_CODE_AGING_TIMER		101
#define	AUTO_LOAD_MSG_CODE_ONU_REGISTER		102
#define	AUTO_LOAD_MSG_CODE_ONU_REQUEST		103
#define	AUTO_LOAD_MSG_CODE_ONU_UPGRADING		104
#define	AUTO_LOAD_MSG_CODE_ONU_DEREGISTER		105
#define	AUTO_LOAD_MSG_CODE_ONU_COMPLETED		106

#define	AUTO_LOAD_MSG_CODE_CDP_IP_RESOURCE	200
#define	AUTO_LOAD_MSG_CODE_CDP_IP_RELEASE		201
#define	AUTO_LOAD_MSG_CODE_CDP_ONU_REQUEST	202
#define	AUTO_LOAD_MSG_CODE_CDP_ONU_COMPLETED	203


extern LONG   onuAutoLoadTimerId;
extern ULONG  onuAutoLoadSemId;

extern LONG  autoload_ftp_server_ip_set( UCHAR *ftpServerIp, UCHAR *userName, UCHAR *password, USHORT ftpPort, USHORT ftpDataPort );
extern LONG  autoload_ftp_server_ip_get( UCHAR *ftpServerIp, UCHAR *userName, UCHAR *password, USHORT *ftpPort, USHORT *ftpDataPort );
extern LONG  autoload_ftp_client_ip_set( struct vty *vty, UCHAR *ipMask, UCHAR *ipGateway, USHORT vid );
extern LONG  autoload_ftp_client_ip_get( ULONG onuDevIdx, UCHAR *ipMask, UCHAR *ipGateway, USHORT *vid );
extern LONG  autoload_ftp_client_ip_del(UCHAR *ipMask );
extern LONG  autoload_cfg_filename_and_direct_get( ULONG onuDevIdx, UCHAR *filename, ULONG *direct );

extern char *auto_load_mac_to_str(UCHAR *onu_mac);


extern void onuAutoLoadStartupTimerCallback();
extern void onuAutoLoadAgingTimerCallback();
extern LONG onuAutoLoadOnuNewRegisterCallback( ULONG onuDevIdx );
extern LONG onuAutoLoadOnuRequestCallback( ULONG onuDevIdx );
extern LONG onuAutoLoadOnuDeregisterCallback( ULONG onuDevIdx );
extern LONG onuAutoLoadOnuCompletedCallback( ULONG onuDevIdx, ULONG result );
extern LONG onuAutoLoadOnuUpgradingCallback (ULONG onuDevIdx);

extern BOOL autoload_config_onulist_check(ULONG devIdx);
extern STATUS getAutoLoadUpgradeDeviceIndex(ULONG devIdx,ULONG *pvar);
extern STATUS getAutoLoadUpgradeBoardIndex(ULONG brdIdx,ULONG *pvar);
extern STATUS getAutoLoadUpgradePonPortIndex(ULONG ponIdx,ULONG *pvar);
extern STATUS getAutoLoadUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS getAutoLoadDisplayWaitUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS  getAutoLoadDisplayUpgradingOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS  getAutoLoadDisplayWaitConfigOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS getAutoLoadDisplayConfigingOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS getAutoLoadDisplayConfigedOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR ret_value[64]);
extern STATUS setAutoLoadConfigDirection(ULONG onuIdx,ULONG set_value);
extern STATUS setAutoLoadUpgradeOnuList(ULONG devIdx,ULONG brdIdx,ULONG ponIdx,UCHAR set_value[64]);
extern STATUS getAutoLoadUserIpGateway(ULONG idxs,UCHAR *onu_cfg_ipgateway,UCHAR *ul_length);
extern STATUS getAutoLoadUserVlanid(ULONG idxs,ULONG  *cfg_vid,UCHAR *cfg_vidlen);
extern STATUS getAutoLoaduserIpStatus(ULONG  idxs,ULONG *cfg_rowstatus,UCHAR*cfg_rowstatuslen);
extern STATUS getAutoLoadRowStatus(ULONG  idxs,ULONG *cfg_rowstatus,UCHAR*cfg_rowstatuslen);
extern STATUS getAutoLoadOnuDevIdx(ULONG idxs,ULONG *cfg_onuidx,UCHAR *cfg_onuidxlen);
extern STATUS getAutoLoadUserIpAddr(ULONG idxs,UCHAR *onu_cfg_ipadd, UCHAR *ul_length);
extern STATUS setAutoLoadUserIpAddr(ULONG idxs,UCHAR onucfg_ipadd[32]);
extern STATUS setAutoLoadUserIpGateWay(ULONG idxs,UCHAR onucfg_ipgateway[32]);
extern STATUS setAutoLoadUserVlanid(ULONG idxs,ULONG onucfg_vid);
extern STATUS setAutoLoadRowStatus(UCHAR idxs,ULONG onucfg_rowstatus);
extern STATUS setAutoLoadConfigRowStatus(ULONG onuIdx,ULONG set_value);
extern STATUS getAutoLoadConfigRowStatus(ULONG idxs,ULONG *cfg_rowStatus);
extern STATUS setAutoLoadAgingTime(ULONG setVal);
extern STATUS getAutoLoadAgingTime(ULONG* ul_result);
extern STATUS getAutoLoadDisplayDeviceIndex(ULONG index,ULONG *pvar);
extern STATUS getAutoLoadDisplayBoardIndex(ULONG index,ULONG *pvar);
extern STATUS getAutoLoadDisplayPonPortIndex(ULONG index,ULONG *pvar);

extern LONG  autoload_filetype_to_filename( UCHAR  *onu_type, UCHAR  file_type, UCHAR *ver, UCHAR *fileName );

extern ULONG  autoload_onu_debug_flag;
#define AUTOLOAD_ONU_DEBUG(x) if(autoload_onu_debug_flag) sys_console_printf x

#endif		/* EPON_MODULE_ONU_AUTO_LOAD */
#endif


