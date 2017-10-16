#ifdef	__cplusplus
extern "C"
{
#endif

#include "OltGeneral.h"

#if 1
#define MAL_MAX_DESC            256            /* max descriptors per channel */
#define EMAC_RXD_MAX        MAL_MAX_DESC   /* max # of RX descriptors/MAL chn */
#define EMAC_TXD_MAX        MAL_MAX_DESC   /* max # of TX descriptors/MAL chn */
#define MAL_NUM_TYPES  2
#define MAL_MAX_CHANNELS 32    /* 32 for receive, 32 for transmit */


/* malLib.h */

/* MAL TX Descriptor Control/Status bits */
#define MAL_TX_CTRL_READY     0x8000           /* Packet ready to transmit    */
#define MAL_TX_CTRL_WRAP      0x4000           /* Last descriptor in the ring */
#define MAL_TX_CTRL_CM        0x2000
#define MAL_TX_CTRL_LAST      0x1000           /* Last buffer for this packet */
#define MAL_TX_CTRL_INTR      0x0400           /* Intr when packet TX complete*/

/* MAL RX Descriptor Control/Status bits */
#define MAL_RX_CTRL_EMPTY     0x8000           /* Buffer empty, ready to RX   */
#define MAL_RX_CTRL_WRAP      0x4000           /* Last descriptor in the ring */
#define MAL_RX_CTRL_CM        0x2000
#define MAL_RX_CTRL_LAST      0x1000           /* Last buffer for this packet */
#define MAL_RX_CTRL_FIRST     0x0800           /* 1st buffer for this packet  */
#define MAL_RX_CTRL_INTR      0x0400           /* Intr when packet RX complete*/

/* Masks */
#define MAL_MAL_CTRL_BITS     0xFC00           /* All descriptor bits for MAL */
#define MAL_COMMAC_CTRL_BITS  0x03FF           /* All descriptor bits for the */
                                               /* communications core         */

/*
 * MAL Buffer Descriptor Structure.  The same structure is used for both
 * Transmit and Receive channels.
 */
typedef struct
{
    USHORT	statusControl;	/* Shared between MAL and comm core */
    USHORT	dataLen;		/* Buffer byte count                */
    char	*bufferAdrs;	/* Buffer address                   */

} MAL_BD;

typedef struct
    {
    UINT     typeFree;                   /* Cluster or Mblk                  */
    UCHAR  * pFree;                      /* Cluster address, Mblk, etc.      */
    } EMAC_FREE;

typedef struct
    {
    UINT dcrBaseReg;
    UINT validChannels[MAL_NUM_TYPES];
    UINT intLvlTxeob;
    UINT intVecTxeob;
    UINT intLvlRxeob;
    UINT intVecRxeob;
    UINT intLvlTxde;
    UINT intVecTxde;
    UINT intLvlRxde;
    UINT intVecRxde;
    UINT intLvlSerr;
    UINT intVecSerr;
    } MAL_INIT;

typedef struct
    {
    UINT         channelType;       /* TX_TYPE or RX_TYPE                     */
    UINT         channelNum;        /* channel number (0 through 31)          */
    UINT         bufferSizeRX;      /* receive buffer size if an RX channel   */
    UINT         channelIntMask;    /* interrupt mask for this channel        */
    VOIDFUNCPTR  functionEOB;       /* function for EOB int on this channel   */
    UINT         parmEOB;           /* parameter for the above function       */
    VOIDFUNCPTR  functionDescErr;   /* function for DE int on this channel    */
    UINT         parmDescErr;       /* parameter for the above function       */
    VOIDFUNCPTR  functionSERR;      /* function for MAL SERR int for this chn */
    UINT         parmSERR;          /* parameter for the above function       */
    } MAL_CHANNEL;

/* ibmEmacEnd.h */

/* Structure to keep track of TX descriptor queue information */
typedef struct
{
    MAL_BD		*pTxDesc;				/* First descriptor in the TX ring    */
    int			numTxD;					/* Number of TX descriptors           */
    int			indexTxD;				/* Current TX descriptor index        */
    int			indexTxDClean;			/* Current TX cleanup descriptor index*/
    UINT		getNewPacketTX;			/* Write to EMAC_TMR0 to kick off TX  */
    EMAC_FREE	txFree[EMAC_TXD_MAX];	/* array of info for TX free cleanup  */

} TX_INFO;


/*
 *  MAL driver control stucture.
 */
typedef struct
{
	MAL_INIT  malInit;
    char *descTablesAlloc;
    char *descTables;
    UINT malInitializedChannels[MAL_NUM_TYPES];
    MAL_CHANNEL malChannelArray[MAL_NUM_TYPES][MAL_MAX_CHANNELS];

} MAL_DATA;


/* #include <end.h> */

/*
 * The definition of the overall driver control structure.  Each instance of
 * the driver creates its own copy of this structure.
 */
typedef struct ibmEmacDevice
{
#if 0
    END_OBJ     end;               /* END_OBJ for this device            */
#else
    UCHAR reserve[0x2b0];
#endif
    int         unit;              /* Unit number of this EMAC device    */
    UINT        baseAdrs;          /* Base address of EMAC registers     */
    UINT8       enetAddr[6];       /* Ethernet MAC address               */
    int         txChn0MalChannel;  /* MAL channel # for EMAC TX channel 0*/
    int         txChn1MalChannel;  /* MAL channel # for EMAC TX channel 1*/
    int         rxChn0MalChannel;  /* MAL channel # for EMAC RX channel 0*/
    MAL_DATA	*pMalData;         /* ptr to MAL data for this EMAC      */
    int         numTxChannels;     /* using 1 or 2 TX Channels           */
    int         txChannel;         /* TX channel next for next transmit  */
    TX_INFO     txInfo[2];         /* Tx desc info, one for each channel */
    MAL_BD		*pRxDesc;          /* First descriptor in the RX ring    */
    int         numRxD;            /* Number of RX descriptors           */
    int         indexRxD;          /* Current RX descriptor index        */
    int         ivec;              /* Ethernet Interrupt vector          */
    int         ilevel;            /* Ethernet Interrupt level           */
    int         phyAdrs;           /* Address of PHY to be used          */
    USHORT      phyAnlpar;         /* Results of auto-negotiation        */
    char		*memInputAdrs;     /* Memory address from input string   */
    char		*memAdrsMalloc;    /* Memory that was malloced           */
    int         memSizeMalloc;     /* Size of memory that was malloced   */
    char		*memAdrs;          /* Actual address to be used          */
    int         memInputSize;      /* Memory size from input string      */
    int         memSize;           /* Actual memory size to be used      */
    UINT        inputFlags;        /* Flags passed in load string        */
    UINT        localFlags;        /* Local status flags                 */
#if 0
    CACHE_FUNCS cacheFuncs;        /* Cache function pointers            */
    CL_POOL_ID  pClPoolId;         /* Cluster pool Id                    */
    M_CL_CONFIG mClCfg;            /* mBlk & cluster config structure    */
    CL_DESC     clDesc;            /* Cluster descriptor table           */
    END_ERR     lastError;         /* Last error sent to muxError        */
    UINT        errorEmac;         /* saves last EMAC error              */
    int         intErrorTX;        /* number of TX error interrupts      */
    int         intErrorRX;        /* number of RX error interrupts      */
    int         cacheLineSize;     /* size of the processor cache line   */
    int         opbSpeedMhz;       /* OPB bus speed in MHz               */
    UINT        stacrOpbSpeed;     /* used in STACR reg OPB speed field  */
    WDOG_ID		wdRestart;			/* WD to unblock MUX in case ot fails */
#endif
} EMAC_DRV_CTRL;
#endif

extern EMAC_DRV_CTRL *pEmac1Ctrl;
extern void ReInitCtrlChannel(void);
extern int devsm_reboot_times_default_set( ULONG slotno);	/* modified by xieshl 20080517 */

LONG __ctrlchan_tx_check_enable = V2R1_ENABLE;
LONG __ctrlchan_tx_reset_enable = V2R1_ENABLE;
ULONG __ctrlchan_tx_status = 0;
ULONG __ctrlchan_tx_exception_count = 0;
ULONG __ctrlchan_tx_exception_max = 20;
ULONG __ctrlchan_tx_exception_resume_times = 5;
extern VOID (*check_ppc405_emac1_hook_rtn) (VOID);

VOID emac_check_mal_bd_tx_status()
{
	int i;
	TX_INFO *pTxInfo;
	MAL_BD *pMal;
	int bc_status_exception_num = 0;
	int bd_status_exception_threshold;

	if( (pEmac1Ctrl == NULL) || (__ctrlchan_tx_check_enable != V2R1_ENABLE) )
		return;
	
	pTxInfo = &pEmac1Ctrl->txInfo[0];	
	pMal = pTxInfo->pTxDesc;

	if( (bd_status_exception_threshold = (pTxInfo->numTxD >> 1)) == 0 )
		return;
	
	for( i=0; i<pTxInfo->numTxD; i++ )
	{
		if( pMal[i].statusControl & MAL_TX_CTRL_READY )
		{
			bc_status_exception_num++;
			if( bc_status_exception_num > bd_status_exception_threshold )
				break;
		}
	}
	
	if( bc_status_exception_num > bd_status_exception_threshold /*pMal->statusControl & MAL_TX_CTRL_READY*/ )
	{
		if( __ctrlchan_tx_exception_resume_times > 0 )
		{
			if( __ctrlchan_tx_exception_count > __ctrlchan_tx_exception_max )
			{
				if( __ctrlchan_tx_status == 0 )
				{
					__ctrlchan_tx_status = 1;

					/* 控制通道重新初始化 */
					if( __ctrlchan_tx_reset_enable == V2R1_ENABLE )
						ReInitCtrlChannel();
                    if( SYS_LOCAL_MODULE_ISMASTERACTIVE )	/* modified by xieshl 20100114, 备用主控不上报 */
					{
						/* 上报控制通道异常告警 */
						cpuCtrlChan_EventReport( other_ctrlchan_fail );

						/* 如果因控制通道造成业务板重启超时，恢复重启次数限制，设备管理模块会自动
						恢复没有启动的单板*/
						for( i=1; i<=SYS_CHASSIS_SLOTNUM; i++ )
						{
							devsm_reboot_times_default_set(i);
						}
						__ctrlchan_tx_exception_resume_times --;
					}
				}
			}
			else
				__ctrlchan_tx_exception_count++;

			sys_console_printf( "contral channel is busy (statusControl=0x%02x, indexTxD=%d)\r\n", pMal->statusControl, pTxInfo->indexTxD );
		}
		else
		{
			/* 控制通道自动恢复最多执行5次，如果仍产生异常，必须考虑更换交换板 */
			sys_console_printf(" %% %s contral channel is error, please replace a new one\r\n", GetSwBoardNameString()/*BOARD_TYPE_GFA6700_SW_STR*/);
		}
	}
	else
	{
		__ctrlchan_tx_exception_count = 0;
		
		if( __ctrlchan_tx_status != 0 )
		{
			__ctrlchan_tx_status = 0;
			/* 上报控制通道告警恢复 */
			cpuCtrlChan_EventReport( other_ctrlchan_success );
		}
	}
}

DEFUN( show_emac_mal_bd_tx_status,
       show_emac_mal_bd_tx_status_cmd,
       "show emac_mal_bd status",
	"show information\n"
	"contral channel buffer descriptor\n"
	"buffer descriptor send status\n"
       )
{
	int i, j, count;
	TX_INFO *pTxInfo = &pEmac1Ctrl->txInfo[0];
	MAL_BD *pMal = pTxInfo->pTxDesc;
	
	vty_out( vty, "  index  status      index  status\r\n" );
	count = pTxInfo->numTxD/2;
	for( i=0; i<count; i++ )
	{
		j = i+count;
		vty_out( vty, "  %3d%3s %5s     %5d%3s %5s\r\n", 
			i+1, ((pTxInfo->indexTxD == i) ? "(T)" : " " ), ((pMal[i].statusControl & MAL_TX_CTRL_READY) ? "ready" : "idle"),
			j+1, ((pTxInfo->indexTxD == j) ? "(T)" : " " ), ((pMal[j].statusControl & MAL_TX_CTRL_READY) ? "ready" : "idle"));
	}
	vty_out( vty, "\r\n" );
	return CMD_SUCCESS;
}

DEFUN( emac_mal_bd_check_enable_fun,
       emac_mal_bd_check_enable_cmd,
       "config ctrlchan-check [enable|disable] {not-reset}*1",
       DescStringCommonConfig
       "Config contral-channel-check setting\n"
	"Set contral-channel-check enable\n"
	"Set contral-channel-check disable\n"
       )
{
	if( VOS_StrCmp( argv[0], "enable") == 0 )
	{
		if( __ctrlchan_tx_check_enable != V2R1_ENABLE )
		{
			__ctrlchan_tx_check_enable = V2R1_ENABLE;
		}
		/*else
		{
			vty_out( vty, " Contral-channel-check is enabled already\r\n" );
		}*/
	}
	else
	{
		if( __ctrlchan_tx_check_enable != V2R1_DISABLE )
		{
			__ctrlchan_tx_check_enable = V2R1_DISABLE;
		}
		/*else
		{
			vty_out( vty, " Contral-channel-check is disabled already\r\n" );
		}*/
	}
	if( argc > 1 )
		__ctrlchan_tx_reset_enable = V2R1_DISABLE;
	else
		__ctrlchan_tx_reset_enable = V2R1_ENABLE;
	
	return CMD_SUCCESS;
}

DEFUN( emac_mal_bd_check_cfg_show,
       emac_mal_bd_check_cfg_show_cmd,
       "show ctrlchan-check configuration",
	"show information\n"
       "Show contral-channel-check setting\n"
       "Show contral-channel-check setting\n"
       )
{
	if( __ctrlchan_tx_check_enable == V2R1_ENABLE )
	{
		vty_out( vty, " Contral-channel-check is enabled\r\n" );
		if( __ctrlchan_tx_reset_enable == V2R1_ENABLE )
			vty_out( vty, " Contral-channel-auto-reset is enabled\r\n" );
	}
	else
	{
		vty_out( vty, " Contral-channel-check is disabled\r\n" );
	}
	
	return CMD_SUCCESS;
}

DEFUN( emac0_net_addr_show,
       emac0_net_addr_show_cmd,
       "show nm-e netconfig",
	"show information\n"
       "Show NM-E information\n"
       "Show NM-E ip and mac address\n"
       )
{
	typedef struct {
		int iIpAdd;
		int iIpMask;
		int iGateway;
		unsigned short iUsedFlag;
	} Enet_IpAdd_Infor;
	  
	typedef struct {
		unsigned char bMacAdd[6];
	} Enet_MACAdd_Infor;
	
	Enet_IpAdd_Infor  lM_Stru_Ipinfo;
	Enet_MACAdd_Infor  lM_Stru_Macinfo;
	extern ULONG outband_port_link_status;
	extern ULONG outband_port_link_sacn_enable;
  
	extern Enet_IpAdd_Infor *funReadIPAddFromNvram( Enet_IpAdd_Infor *);
	extern Enet_MACAdd_Infor *funReadMacAddFromNvramD( Enet_MACAdd_Infor *);

	funReadIPAddFromNvram(&lM_Stru_Ipinfo);
	vty_out( vty, "  IP Address  : %d.%d.%d.%d\r\n  Subnet Mask : %d.%d.%d.%d\r\n  Gateway     : %d.%d.%d.%d\r\n",
  			(lM_Stru_Ipinfo.iIpAdd>>24)&0x0FF,(lM_Stru_Ipinfo.iIpAdd>>16)&0x0FF,(lM_Stru_Ipinfo.iIpAdd>>8)&0x0FF,lM_Stru_Ipinfo.iIpAdd&0x0FF,
			(lM_Stru_Ipinfo.iIpMask>>24)&0x0FF,(lM_Stru_Ipinfo.iIpMask>>16)&0x0FF,(lM_Stru_Ipinfo.iIpMask>>8)&0x0FF,lM_Stru_Ipinfo.iIpMask&0x0FF,
			(lM_Stru_Ipinfo.iGateway>>24)&0x0FF,(lM_Stru_Ipinfo.iGateway>>16)&0x0FF,(lM_Stru_Ipinfo.iGateway>>8)&0x0FF,lM_Stru_Ipinfo.iGateway&0x0FF );

	funReadMacAddFromNvramD(&lM_Stru_Macinfo);
	vty_out( vty, "  MAC Address : %02X%02X.%02X%02X.%02X%02X\r\n",lM_Stru_Macinfo.bMacAdd[0],lM_Stru_Macinfo.bMacAdd[1],
			                  lM_Stru_Macinfo.bMacAdd[2],lM_Stru_Macinfo.bMacAdd[3],lM_Stru_Macinfo.bMacAdd[4],lM_Stru_Macinfo.bMacAdd[5]);

	vty_out(vty, "  Outband Management Status: %s\r\n", ((outband_port_link_status == 1) && (outband_port_link_sacn_enable == 0)) ? "enabled" : "disable" );
	
	return CMD_SUCCESS;
}


int emac_check_init()
{
	if( (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100) )
	{
		__ctrlchan_tx_status = 0;
		__ctrlchan_tx_exception_count = 0;

		check_ppc405_emac1_hook_rtn = emac_check_mal_bd_tx_status;

		install_element ( DEBUG_HIDDEN_NODE, &show_emac_mal_bd_tx_status_cmd);
		install_element ( DEBUG_HIDDEN_NODE, &emac_mal_bd_check_enable_cmd);
		install_element ( DEBUG_HIDDEN_NODE, &emac_mal_bd_check_cfg_show_cmd);
	}
	install_element ( DEBUG_HIDDEN_NODE, &emac0_net_addr_show_cmd);
	
	return VOS_OK;
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */

