/**************************************************************
*
*   IncludeFromBcm.h -- 
*
*  
*    Copyright (c)  2014.05 , GWD Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ----------|--- --------|---------------------|------------
*	1.00	        | 05/12/2014  | Creation				| liwei056
*
***************************************************************/
#ifndef  __INCLUDEFROMBCM_H
#define __INCLUDEFROMBCM_H


#ifndef PACK
#define PACK __attribute__((packed))
#endif


/* lint -strong(AJX,bcmEmmiBscBus); */
typedef U8 bcmEmmiBscBus;   /**< bcmEmmiBscBus: typed alias for a 8-bit unsigned integer. */

/* lint -strong(AJX,bcmEmmiBscDeviceId); */
typedef U8 bcmEmmiBscDeviceId;  /**< bcmEmmiBscDeviceId: typed alias for a 8-bit unsigned integer. */

/* lint -strong(AJX,bcmEmmiBscRegister); */
typedef U32 bcmEmmiBscRegister; /**< bcmEmmiBscRegister: typed alias for a 32-bit unsigned integer. */

/* lint -strong(AJX,bcmEmmiBscRegisterSize); */
typedef U8 bcmEmmiBscRegisterSize;  /**< bcmEmmiBscRegisterSize: typed alias for a 8-bit unsigned integer. */

typedef U8 bcmMacAddress[6];
typedef U32 BcmIPv4Address;

typedef struct BcmOui
    {
    U8 _u8[3];   /* < The 3 byte mac address bytes */
    } PACK BcmOui;

#define BCM_EXIT_OK             0
#define BCM_EXIT_ERROR          -1
#define BCM_HAVE_EXIST	       -2
#define BCM_NOT_EXIST	       -3
#define BCM_PARAMETER_ERROR     -5
#define BCM_TIME_OUT            -6
#define BCM_NOT_IMPLEMENTED	   -10
#define BCM_MODE_ERROR	       -12
#define BCM_NO_RESOURCE         -16
#define BCM_HARDWARE_ERROR	   -17
#define BCM_MEMORY_ERROR	       -18
#define BCM_ERROR_OR_OK	       -21

#define BCM_ERROR_BAD_FILE      (-999)
#define BCM_ERROR_NEED_FILE     (-1000)
#define BCM_OLT_NOT_EXIST       (-1003)
#define BCM_QUERY_FAILED        (-1004) /* No record was found to match the */


#define BCM_ADDRESS_TABLE_SIZE  16384


#if 1
/* --------------BCM-SOFT's API(55538)----------------- */

int BcmSoftIsCompatibledWithFirmware ( const PON_olt_id_t olt_id, bool *is_compatibled );

int BCM_init ( void );

int BcmAdapter_PAS5201_Init();


int BCM_assign_pashandler_function( const PAS_handler_functions_index_t     handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );
int BCM_delete_pashandler_function( const unsigned short  handler_id );

 
/* Handler functions enum */
typedef enum
{
    BCM_HANDLER_OAM_FRAME_RECEIVED,					    /* OAM frame originated from OLT's PON  port was received  */

    BCM_HANDLER_OLT_READY,						        /* OLT ready 									        */
    BCM_HANDLER_OLT_RESET,						        /* OLT reset 									        */
    BCM_HANDLER_OLT_ADD,						        /* OLT added 									        */
    BCM_HANDLER_OLT_REMOVE,						        /* OLT removed     							        */

    BCM_HANDLER_LINK_DISCOVERY,				            /* LLID registration to the PON					        */
    BCM_HANDLER_LINK_LOSS,				                /* LLID deregistration (disconnection) from the PON          */

    BCM_HANDLER_ALARM,							        /* Hardware / Software alarm					        */
    BCM_HANDLER_PON_LOSS,                               /* PON loss */
    BCM_HANDLER_CNI_LOSS,                               /* CNI loss */
    BCM_HANDLER_CNI_LINK,                               /* CNI link                                             */

    BCM_HANDLER_GPIO_CHANGED,							/* GPIO Change Notify					        */
    BCM_HANDLER_OTDR_ISDONE,							/* OTDR Done				        */
    BCM_HANDLER_MPCP_TIMESTAMP_REPORT,					/* MPCP Timestamp Report		        */

    BCM_HANDLER_ONU_RANGE_CHANGED,                      /* ONU's Range is changed              */
    BCM_HANDLER_ONU_DYING_GASP,                         /* ONU's DyingGasp alarm */
    BCM_HANDLER_MPCP_REGISTER_REPEAT,                   /* MPCP's register frams is duplicated              */

    BCM_HANDLER_OLT_CLI_DEBUG_OUTPUT,                   /* OLT's Cli Debug Output */

    BCM_HANDLER_REDUNDANCY_ONU_SWITCH_NOTIFY,           /* redundancy ONU is switched */
    BCM_HANDLER_REDUNDANCY_OLT_LINK_ACTIVE,             /* redundancy OLT Link is actived */
    BCM_HANDLER_REDUNDANCY_OLT_LINK_STANDBY,            /* redundancy OLT Link is standby */
    BCM_HANDLER_INVALID_GRANT_DETECTED,                 /* ONU's grant is invalid  */    
    BCM_HANDLER_INVALID_LLID_DETECTED,                  /* ONU's llid is invalid  */    

    BCM_HANDLER_LAST_EXPO_HANDLER = BCM_HANDLER_INVALID_LLID_DETECTED

} BCM_handler_functions_index_t;

/* Assign handler function									
**
** This command registers a function for an event handling. The function is 
** registered as a callback function. For each defined event, a function 
** registration must occur before first-time activation of the event. If a 
** handler function is not registered, either the event will be discarded or 
** it will be processed as part of an initiating command. Handler-function 
** parameters are defined as the corresponding events definitions.
** Command activation with the same handler function index parameter will 
** result in multiple callbacks to the same event.
**
**
** Input Parameters:
**			    handler_function_index  : Type of the handler function assigned 
**			    handler_function        : Pointer to the handler function 
**			    handler_function_id     : Handler function identification. Should 
**										  be used when activating Delete handler 
**										  function
**
** Return codes:
**				PAS_EXIT_OK			   : Success
**				PAS_EXIT_ERROR		   : General Error
**				PAS_PARAMETER_ERROR    : Function parameter error (e.g. handler_function_index out of 
**										 range)
**
*/
int BCM_assign_handler_function
                                 ( const BCM_handler_functions_index_t    handler_function_index, 
								   const void							(*handler_function)(), 
								   unsigned short                        *handler_function_id);

/* Delete handler function									
**
** This command deregisters a function from the event-handling mechanism. 
** The specified function should have been assigned to the event mechanism 
** using the Assign handler function API command.
**
** Input Parameters:
**			    handler_id  : Handler function identification .
**
** Return codes:
**				All the defined TK_* return codes
*/
int BCM_delete_handler_function( const unsigned short  handler_id );


bool BcmOltIsExist ( const short int olt_id );
bool BcmOnuIsExist ( const short int olt_id, const short int llid );
bool BcmOnuIsOnline ( const short int olt_id, const short int llid );

int Bcm_set_olt_base_address(unsigned short int olt_base_address);


/* OLT Initialization parameters struct 
**
**		olt_mac_address				    : OLT MAC address
**										  If MAC address is to be set by caller - valid unicast MAC 
**											address value
**										  If OLT default MAC address is used - OLT_DEFAULT_MAC_ADDRESS
**											(0xFF-0xFF-0xFF-0xFF-0xFF-0xFF - all 1s) the MAC address retrieve from the EEPROM
*/
typedef struct BCM_olt_initialization_parameters_t
{
    unsigned char   	    olt_mac_address[6];
    unsigned char           support_tk_onus;
    unsigned char           ram_test;
    unsigned char           firmware_version[100];
	void				    *firmware_location;
	long int				firmware_size;
} BCM_olt_initialization_parameters_t;

/* Add OLT result parameters struct 
**
**      olt_mac_address : The input parameter changes its value to the OLT's actual MAC     
**                        address, whether set explicitly by the input parameter or         
**                        taken from the OLT default (EEPROM) MAC address                   
*/
typedef struct BCM_add_olt_result_t
{
	unsigned char  olt_mac_address[6];
} BCM_add_olt_result_t;

/* Add OLT							
**
** Indication for the PON software that an OLT hardware is present, physically turned on and ready to be 
** activated. This function has to be called for every OLT present / added before any other reference.
** If an OLT firmware image file is used - all file-oriented preparations for reading as post-reading 
** manipulations are done explicitly within the function implementation (e.g. open file, close file). 
** Note: Call this function after hardware driver can physically communicate with the OLT (this function
** calls the driver's per-OLT init function)
**  
**
** Input Parameters:
**		olt_id						    : OLT id (determined by the caller), range: 
**										  PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		olt_initialization_parameters   : see PON_olt_initialization_parameters_t
**
** Output Parameter:
**		add_olt_result				    : see PON_add_olt_result_t 
**										  
**										  
** Return codes:
**				All the defined TK_* return codes
**						
*/
int BCM_add_olt 
                    ( const short int				         olt_id, 
				      const BCM_olt_initialization_parameters_t  *olt_initialization_parameters,
                            BCM_add_olt_result_t                 *add_olt_result );

int BCM_remove_olt ( const short int  olt_id,
					   const int		send_shutdown_msg_to_olt, 
					   const int        reset_olt);

int BCM_reset_olt ( const short int  olt_id );


int BcmAdapter_PAS5201_GetOltChipVersion(short int olt_id, PON_device_versions_t *device_versions);
int BcmAdapter_PAS5201_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version);


/* Send OAM frame
**
** Send OAM organization specific 
**
** Input Parameters:
**      olt_id        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid          : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      oam_code      : See OAM_code_t for details
**      length        : Frame length in bytes (excluding Preamble, SFD, and FCS), 
**						range: 10 - 1464 in case of information, 42 - 1496 in case of vendor extension
**      content       : Pointer to the first byte of the frame
**
** Return codes:
**          All the defined         TK_* return codes
*/
int BCM_send_oam 
                         ( const short int  					     olt_id, 
						   const short int							 llid,
						   const PON_oam_code_t 	                 oam_code,
						   const void								*content,
						   const unsigned short						 length );


/* Send frame									
**
** Send an Ethernet frame to remote destination (PON / System). 
** The frame should be a valid Ethernet frame, excluding Preamble, SFD, and FCS.
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid			: Destination LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT ,
**						  PON_LLID_NOT_AVAILABLE (a case where the LLID is not relevant - CNI port frame)
**		content		    : Pointer to the first byte of the frame
**		length			: Frame length in bytes (excluding Preamble, SFD, and FCS), 
**						  range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE
**
** Return codes:
**				All the defined TK_* return codes
**
*/
int BCM_send_frame 
                         ( const short int  					     olt_id, 
						   const short int							 llid,
						   const void								*content,
						   const unsigned short						 length );

int BcmOLT_GetOamHeartbeatInfo(short int olt_id, short int llid, unsigned short *send_period, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_timeout, unsigned short *recv_size, unsigned char *recv_data, bool *recv_IgnoreTrailingBytes);
int BcmOLT_SetOamHeartbeatInfo(short int olt_id, short int llid, unsigned short send_period, unsigned short send_size, unsigned char *send_data, unsigned short recv_timeout, unsigned short recv_size, unsigned char *recv_data, bool recv_IgnoreTrailingBytes);


/* Return TK-SOFT state (running / init etc.)
**
** Input Parameters:
**				none
**
** Return values:
**				TK-SOFT state represented by PON_module_state_t enum values
*/
PON_module_state_t Get_bcm_soft_state ( void );


/** EPON Rate. 
 */
typedef enum bcmEmmiEponRate
{
    bcmEmmiEponRateTenTen       = 0,    /**< 10 gbps up, 10 gbps down. */
    bcmEmmiEponRateTenOne       = 1,    /**< 10 gbps up, 1 gbps down. */
    bcmEmmiEponRateOneOne       = 2 /**< 1 gbps up, 1 gbps down. */
} bcmEmmiEponRate;

/** The current registration status of a link. 
 */
typedef enum bcmEmmiLinkStatus
{
    bcmEmmiLinkStatusNone                   = 0,
    bcmEmmiLinkStatusDiscovered             = 0x0001,   /**< Link has completed MPCP registration. */
    bcmEmmiLinkStatusProtectedWorking       = 0x0002,   /**< Link is in the "working" state for protection switching. */
    bcmEmmiLinkStatusProtectedStandby       = 0x0004,   /**< Link is in the "standby" state for protection switching. */
    bcmEmmiLinkStatusRegistrationPrevented  = 0x0008    /**< Link is being prevented from registering. */
} bcmEmmiLinkStatus;

/** MPCP Discovery Info. 
 */
typedef enum bcmEmmiMpcpDiscoveryInfo
{
    bcmEmmiMpcpDiscoveryInfoNone            = 0,
    bcmEmmiMpcpDiscoveryInfoOneGCapable     = 0x0001,   /**< 1G Capable. */
    bcmEmmiMpcpDiscoveryInfoTenGCapable     = 0x0002,   /**< 10G Capable. */
    bcmEmmiMpcpDiscoveryInfoOneGWindow      = 0x0010,   /**< 1G Window. */
    bcmEmmiMpcpDiscoveryInfoTenGWindow      = 0x0020    /**< 10G Window. */
} bcmEmmiMpcpDiscoveryInfo;

/** The tunnel ID for a link, which maps VLAN tags to EPON LLIDs. 
 */
typedef struct bcmEmmiTunnelId
{
    U32 upstreamId; /**< The VLAN tag (TPID included) to use when sending traffic on this link upstream. */
    U32 downstreamId;   /**< The VLAN tag (TPID included) to use when sending traffic on this link downstream. */
} bcmEmmiTunnelId;

/** Information about an EPON logical link. 
 */
typedef struct bcmEmmiLinkInfo
{
    bcmEmmiLinkStatus status;   /**< Link status flags. */
    bcmEmmiEponRate rate;   /**< The rate at which the link operates. */
    bcmMacAddress sourceMac;    /**< SA to use when sending frames to this link. */
    U16 llid;   /**< The LLID of the link. */
    U16 reserved;    /**< Reserved. */
    bcmEmmiMpcpDiscoveryInfo mpcpDiscoveryInfo; /**< Flags from MPCP discovery. */
    U8 onuLaserOnTime;  /**< Laser on time reported by ONU.  This is set to 0 for links on the 1/1 RP (1G MPCP doesn't include this field). */
    U8 onuLaserOffTime; /**< Laser off time reported by ONU.  This is set to 0 for links on the 1/1 RP (1G MPCP doesn't include this field). */
    U8 pendingGrants;   /**< The number of pending grants. */
    U32 rangeValue; /**< The link's range value in TQ. */
    bcmEmmiTunnelId tunnelId;   /**< The tunnel ID to use when sending traffic to the link. */
    U32 distance;   /**< Approximate length of fiber to ONU (in meters). */
} bcmEmmiLinkInfo;


int BCM_authorize_llid 
                            ( const short int		  olt_id, 
							  const short int		  llid,
							  const bool              authorize_mode );

int BcmOLT_DeregisterLink(short int olt_id, short int llid);
int BcmOLT_GetLinkInfo(short int olt_id, short int llid, bcmEmmiLinkInfo *link_info);
int BcmGetOnuMacAddress( const short int    olt_id, 
								const short int     llid,
								mac_address_t       link_mac );

int BcmAdapter_PAS5201_GetAllOnuParams(short int olt_id, short int *number, PAS_onu_parameters_t onu_parameters);


/** Chip Revision. 
 */
typedef enum bcmEmmiChipRevision
{
    bcmEmmiChipRevisionA                        = 0,    /**< A. */
    bcmEmmiChipRevisionB                        = 1,    /**< B. */
    bcmEmmiChipRevisionC                        = 2,    /**< C. */
    bcmEmmiChipRevisionD                        = 3,    /**< D. */
    bcmEmmiChipRevisionE                        = 4,    /**< E. */
    bcmEmmiChipRevisionF                        = 6,    /**< F. */
    bcmEmmiChipRevisionG                        = 7,    /**< G. */
    bcmEmmiChipRevisionH                        = 8,    /**< H. */
    bcmEmmiChipRevisionI                        = 9,    /**< I. */
    bcmEmmiChipRevisionJ                        = 10,   /**< J. */
    bcmEmmiChipRevisionK                        = 11,   /**< K. */
    bcmEmmiChipRevisionL                        = 12,   /**< L. */
    bcmEmmiChipRevisionM                        = 13,   /**< M. */
    bcmEmmiChipRevisionN                        = 14,   /**< N. */
    bcmEmmiChipRevisionO                        = 15,   /**< O. */
    bcmEmmiChipRevisionP                        = 16,   /**< P. */
    bcmEmmiChipRevisionQ                        = 17,   /**< Q. */
    bcmEmmiChipRevisionR                        = 18,   /**< R. */
    bcmEmmiChipRevisionS                        = 19,   /**< S. */
    bcmEmmiChipRevisionT                        = 20,   /**< T. */
    bcmEmmiChipRevisionU                        = 21,   /**< U. */
    bcmEmmiChipRevisionV                        = 22,   /**< V. */
    bcmEmmiChipRevisionW                        = 23,   /**< W. */
    bcmEmmiChipRevisionX                        = 24,   /**< X. */
    bcmEmmiChipRevisionY                        = 25,   /**< Y. */
    bcmEmmiChipRevisionZ                        = 26    /**< Z. */
} bcmEmmiChipRevision;

/** Type of the firmware load. 
 */
typedef enum bcmEmmiLoadType
{
    bcmEmmiLoadTypeRunning                      = 0,    /**< The currently running app. */
    bcmEmmiLoadTypeBoot                         = 1,    /**< The boot code. */
    bcmEmmiLoadTypeApp0                         = 2,    /**< The app code in APP-0. */
    bcmEmmiLoadTypeApp1                         = 3,    /**< The app code in APP-1. */
    bcmEmmiLoadTypeApp2                         = 4,    /**< The app code in APP-2. */
    bcmEmmiLoadTypePers                         = 5,    /**< The primary personality. */
    bcmEmmiLoadTypePers0                        = 6,    /**< The personality in the first region. */
    bcmEmmiLoadTypePers1                        = 7 /**< The personality in the second region. */
} bcmEmmiLoadType;

/** Maturity of the firmware load. 
 */
typedef enum bcmEmmiLoadMaturity
{
    bcmEmmiLoadMaturityRelease                  = 82,   /**< 'R': release build. */
    bcmEmmiLoadMaturityCustom                   = 67,   /**< 'C': custom build. */
    bcmEmmiLoadMaturityBeta                     = 66,   /**< 'B': beta build. */
    bcmEmmiLoadMaturityAlpha                    = 65,   /**< 'A': alpha build. */
    bcmEmmiLoadMaturityEngineering              = 69,   /**< 'E': engineering build. */
    bcmEmmiLoadMaturityDevelopment              = 68    /**< 'D': development build. */
} bcmEmmiLoadMaturity;

/** Load Timestamp. 
 */
typedef struct bcmEmmiLoadTimestamp
{
    U16 year;   /**< The year of the build. */
    U8 month;   /**< The month (1-12). */
    U8 day; /**< The day (1-31). */
    U8 hour;    /**< The hour (0-23). */
    U8 min; /**< The minute (0-59). */
    U8 sec; /**< The second (0-59). */
} bcmEmmiLoadTimestamp;

/** Detailed load version number. 
 */
typedef struct bcmEmmiLoadExtendedVersionNumber
{
    bcmEmmiLoadMaturity maturity;   /**< Build maturity as a single ASCII character ('R' for R-release, etc). */
    U8 major;   /**< Major version number. */
    U8 minor;   /**< Minor version number. */
    U16 patch;  /**< Patch revision number. */
} bcmEmmiLoadExtendedVersionNumber;

/** Firmware load information. 
 */
typedef struct bcmEmmiLoadInfo
{
    U32 stream; /**< Stream number. */
    U32 revision;   /**< Revision number. */
    bcmEmmiLoadTimestamp timestamp; /**< Timestamp information. */
    bcmEmmiLoadExtendedVersionNumber extendedVersion;   /**< Full load version number. */
    U32 loadSize;   /**< File size. */
    U32 crc32;  /**< File CRC-32. */
} bcmEmmiLoadInfo;

/** OLT firmware load information. 
 */
typedef struct bcmEmmiOltLoadInfo
{
    bcmEmmiLoadType loadType;   /**< Type of the firmware load. */
    bcmEmmiLoadInfo loadInfo;   /**< Firmware load information. */
} bcmEmmiOltLoadInfo;

typedef struct bcmOltInfo
{
    bcmMacAddress eponMac;  /**< The primary/first EPON MAC address of this OLT. */
    bcmEmmiChipRevision chipRev;    /**< The revision of the chip. */
    U32 reserved0;  /**< Reserved. */
    U16 productCode;    /**< The product code of the OLT (as retrieved from OLT personality). */
    U16 reserved1;  /**< Reserved. */
    U16 firmwareVersion;    /**< The firmware version that the OLT is running. */
    U8 vendorInfo[64];  /**< Vendor defined personality region. */
    U16 jedecId;    /**< OLT chip JEDEC ID. */
    U16 chipId; /**< Teknovus chip product ID. */
    U32 chipVersion;    /**< Teknovus chip product revision. */
    U8 numEponPorts;    /**< Number of EPON ports on the OLT. */
    U8 numNniPorts; /**< Number of NNI ports on the OLT. */
    U8 numOfLoadInfo;   /**< The number of load info sets. */
    bcmEmmiOltLoadInfo loadInfoSets[8];   /**< Set of the firmware load information. */
} bcmOltInfo;

int BcmOLT_GetOltInfo(short int olt_id, bcmOltInfo *olt_info);
int BcmOLT_ResetChip(short int olt_id);

int BcmOLT_PonPortIsEnabled(short int olt_id, bool *enabled);
int BcmOLT_NniPortIsEnabled(short int olt_id, bool *enabled);

int BcmOLT_EnablePonPort(short int olt_id);
int BcmOLT_DisablePonPort(short int olt_id);

int BcmOLT_EnableNniPort(short int olt_id);
int BcmOLT_DisableNniPort(short int olt_id);

int BcmOltOpenDataPath(short int olt_id);
int BcmOltCloseDataPath(short int olt_id);

int BcmChip_GetCommonHostID(short int olt_id);
int BcmChip_GetOltPonPortID(short int olt_id);
int BcmChip_GetOltEthPortID(short int olt_id);

int BcmOltSetOpticalTxMode(short int olt_id, int tx_mode);
int BcmOltGetOpticalTxMode(short int olt_id, int *tx_mode);


/** Possible encryption modes. 
 */
typedef enum bcmEmmiEncryptionMode
{
    bcmEmmiEncryptionModeNone                       = 0,    /**< No encryption. */
    bcmEmmiEncryptionModeAes                        = 1,    /**< * 1G: AES-CFB. */
    bcmEmmiEncryptionModeTripleChurning             = 2,    /**< Triple churning (CTC mode). */
    bcmEmmiEncryptionModeZeroOverheadAes            = 3 /**< Zero overhead AES (10G only). */
} bcmEmmiEncryptionMode;

/** Encryption Key Choice. 
 */
typedef enum bcmEmmiEncryptionKeyChoice
{
    bcmEmmiEncryptionKeyChoiceEncryptOff            = 0,    /**< Disable encryption. */
    bcmEmmiEncryptionKeyChoiceKey0                  = 1,    /**< Use encryption key 0. */
    bcmEmmiEncryptionKeyChoiceKey1                  = 2 /**< Use encryption key 1. */
} bcmEmmiEncryptionKeyChoice;

typedef struct bcmLinkEncryptCfg
{
    U8 downstreamKey[24];   /**< Downstream encryption key. */
    bcmEmmiEncryptionMode downstreamEncryptionMode; /**< Downstream encryption mode. */
    bcmEmmiEncryptionKeyChoice downstreamKeyChoice; /**< Which key to use for upstream encryption. */
    U8 upstreamKey[24]; /**< Upstream encryption key. */
    bcmEmmiEncryptionMode upstreamEncryptionMode;   /**< Upstream encryption mode. */
    bcmEmmiEncryptionKeyChoice upstreamKeyChoice;   /**< Which key to use for downstream encryption. */
} bcmLinkEncryptCfg;

int BcmOLT_GetLinkEncryptionData(short int olt_id, short int llid, bcmLinkEncryptCfg *encrypt_config);
int BcmOLT_SetLinkEncryptionData(short int olt_id, short int llid, bcmLinkEncryptCfg *encrypt_config);

int BcmOLT_GetLinkCtcTripleChurning(short int olt_id, short int llid, unsigned short *keyExchangeTime);
int BcmOLT_SetLinkCtcTripleChurning(short int olt_id, short int llid, unsigned short keyExchangeTime);


/** OLT GPIO purpose codes. 
 */
typedef enum bcmEmmiOltGpioPurposePio
{
    bcmEmmiOltGpioPurposePioUnused                      = 0,    /**< Pin is not in use. */
    bcmEmmiOltGpioPurposePioUserInput0                  = 1,    /**< User Input0. */
    bcmEmmiOltGpioPurposePioUserInput1                  = 2,    /**< User Input1. */
    bcmEmmiOltGpioPurposePioUserInput2                  = 3,    /**< User Input2. */
    bcmEmmiOltGpioPurposePioUserInput3                  = 4,    /**< User Input3. */
    bcmEmmiOltGpioPurposePioUserInput4                  = 5,    /**< User Input4. */
    bcmEmmiOltGpioPurposePioUserInput5                  = 6,    /**< User Input5. */
    bcmEmmiOltGpioPurposePioUserInput6                  = 7,    /**< User Input6. */
    bcmEmmiOltGpioPurposePioUserInput7                  = 8,    /**< User Input7. */
    bcmEmmiOltGpioPurposePioUserInput8                  = 9,    /**< User Input8. */
    bcmEmmiOltGpioPurposePioUserInput9                  = 10,   /**< User Input9. */
    bcmEmmiOltGpioPurposePioUserInput10                 = 11,   /**< User Input10. */
    bcmEmmiOltGpioPurposePioUserInput11                 = 12,   /**< User Input11. */
    bcmEmmiOltGpioPurposePioUserInput12                 = 13,   /**< User Input12. */
    bcmEmmiOltGpioPurposePioUserInput13                 = 14,   /**< User Input13. */
    bcmEmmiOltGpioPurposePioUserInput14                 = 15,   /**< User Input14. */
    bcmEmmiOltGpioPurposePioUserInput15                 = 16,   /**< User Input15. */
    bcmEmmiOltGpioPurposePioUserInput16                 = 17,   /**< User Input16. */
    bcmEmmiOltGpioPurposePioUserInput17                 = 18,   /**< User Input17. */
    bcmEmmiOltGpioPurposePioUserInput18                 = 19,   /**< User Input18. */
    bcmEmmiOltGpioPurposePioUserInput19                 = 20,   /**< User Input19. */
    bcmEmmiOltGpioPurposePioUserInput20                 = 21,   /**< User Input20. */
    bcmEmmiOltGpioPurposePioUserInput21                 = 22,   /**< User Input21. */
    bcmEmmiOltGpioPurposePioUserInput22                 = 23,   /**< User Input22. */
    bcmEmmiOltGpioPurposePioUserInput23                 = 24,   /**< User Input23. */
    bcmEmmiOltGpioPurposePioUserInput24                 = 25,   /**< User Input24. */
    bcmEmmiOltGpioPurposePioUserInput25                 = 26,   /**< User Input25. */
    bcmEmmiOltGpioPurposePioUserInput26                 = 27,   /**< User Input26. */
    bcmEmmiOltGpioPurposePioUserInput27                 = 28,   /**< User Input27. */
    bcmEmmiOltGpioPurposePioUserInput28                 = 29,   /**< User Input28. */
    bcmEmmiOltGpioPurposePioUserInput29                 = 30,   /**< User Input29. */
    bcmEmmiOltGpioPurposePioUserInput30                 = 31,   /**< User Input30. */
    bcmEmmiOltGpioPurposePioUserInput31                 = 32,   /**< User Input31. */
    bcmEmmiOltGpioPurposePioStatsOff                    = 33,   /**< Stats Off. */
    bcmEmmiOltGpioPurposePioBaud57600                   = 34,   /**< Baud57600. */
    bcmEmmiOltGpioPurposePioCliOnly                     = 35,   /**< CLI Only Mode. */
    bcmEmmiOltGpioPurposePioResetOlt                    = 36,   /**< Reset OLT. */
    bcmEmmiOltGpioPurposePioAlarmTemperature            = 37,   /**< Alarm Temperature. */
    bcmEmmiOltGpioPurposePioAlarmPower                  = 38,   /**< Alarm Power. */
    bcmEmmiOltGpioPurposePioPulseHandler                = 39,   /**< Pulse Handler. */
    bcmEmmiOltGpioPurposePioSlotId0                     = 40,   /**< Slot Id 0. */
    bcmEmmiOltGpioPurposePioSlotId1                     = 41,   /**< Slot Id 1. */
    bcmEmmiOltGpioPurposePioSlotId2                     = 42,   /**< Slot Id 2. */
    bcmEmmiOltGpioPurposePioSlotId3                     = 43,   /**< Slot Id 3. */
    bcmEmmiOltGpioPurposePioSlotId4                     = 44,   /**< Slot Id 4. */
    bcmEmmiOltGpioPurposePioUserAlarm0                  = 45,   /**< User Alarm0. */
    bcmEmmiOltGpioPurposePioUserAlarm1                  = 46,   /**< User Alarm1. */
    bcmEmmiOltGpioPurposePioFpgaPrgConfDone             = 47,   /**< FPGA Prg Conf Done. */
    bcmEmmiOltGpioPurposePioFpgaPrgConfData0            = 48,   /**< FPGA Prg Conf Data 0. */
    bcmEmmiOltGpioPurposePioZlInt                       = 49,   /**< Zilink Int. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon0        = 50,   /**< Optical TX Degrade â€?PON 0. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon1        = 51,   /**< Optical TX Degrade â€?PON 1. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon2        = 52,   /**< Optical TX Degrade â€?PON 2. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon3        = 53,   /**< Optical TX Degrade â€?PON 3. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon4        = 54,   /**< Optical TX Degrade â€?PON 4. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon5        = 55,   /**< Optical TX Degrade â€?PON 5. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon6        = 56,   /**< Optical TX Degrade â€?PON 6. */
    bcmEmmiOltGpioPurposePioOpticalTxDegradePon7        = 57,   /**< Optical TX Degrade â€?PON 7. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon0        = 58,   /**< Optical TX Failure â€?PON 0. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon1        = 59,   /**< Optical TX Failure â€?PON 1. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon2        = 60,   /**< Optical TX Failure â€?PON 2. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon3        = 61,   /**< Optical TX Failure â€?PON 3. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon4        = 62,   /**< Optical TX Failure â€?PON 4. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon5        = 63,   /**< Optical TX Failure â€?PON 5. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon6        = 64,   /**< Optical TX Failure â€?PON 6. */
    bcmEmmiOltGpioPurposePioOpticalTxFailurePon7        = 65,   /**< Optical TX Failure â€?PON 7. */
    bcmEmmiOltGpioPurposePioTriggerPSPon0               = 66,   /**< PON 0 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon1               = 67,   /**< PON 1 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon2               = 68,   /**< PON 2 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon3               = 69,   /**< PON 3 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon4               = 70,   /**< PON 4 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon5               = 71,   /**< PON 5 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon6               = 72,   /**< PON 6 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioTriggerPSPon7               = 73,   /**< PON 7 â€?Input Protection Switching Trigger. */
    bcmEmmiOltGpioPurposePioI2cSda1                     = 92,   /**< BSC SDA1. */
    bcmEmmiOltGpioPurposePioI2cScl1                     = 93,   /**< BSC SCL1. */
    bcmEmmiOltGpioPurposePioI2cSda0                     = 94,   /**< BSC SDA0. */
    bcmEmmiOltGpioPurposePioI2cScl0                     = 95,   /**< BSC SCL0. */
    bcmEmmiOltGpioPurposePioMpioDataOut                 = 128,  /**< MPIO Data Out. */
    bcmEmmiOltGpioPurposePioMpioEpmAgc                  = 129,  /**< MPIO Epm Agc. */
    bcmEmmiOltGpioPurposePioMpioEpmCdr                  = 130,  /**< MPIO Epm Cdr. */
    bcmEmmiOltGpioPurposePioMpioDataEpmStrobe0          = 131,  /**< MPIO Data Epm Strobe 0. */
    bcmEmmiOltGpioPurposePioMpioDataEpmStrobe1          = 132,  /**< MPIO Data Epm Strobe 1. */
    bcmEmmiOltGpioPurposePioMpioDataRangeStrobe         = 133,  /**< MPIO Data Range Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataSerdesStrobe        = 134,  /**< MPIO Data SerDes Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataUnassignStrobe      = 135,  /**< MPIO Data Unassign Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataEpmRssi             = 136,  /**< MPIO Data Epm Rssi. */
    bcmEmmiOltGpioPurposePioMpioDataEpmOtdr             = 137,  /**< MPIO Data Epm Otdr. */
    bcmEmmiOltGpioPurposePioMpioDataXemAgc              = 138,  /**< MPIO Data Xem Agc. */
    bcmEmmiOltGpioPurposePioMpioDataXemCdr              = 139,  /**< MPIO Data Xem Cdr. */
    bcmEmmiOltGpioPurposePioMpioDataXemStrobe0          = 140,  /**< MPIO Data Xem Strobe 0. */
    bcmEmmiOltGpioPurposePioMpioDataXemStrobe1          = 141,  /**< MPIO Data Xem Strobe 1. */
    bcmEmmiOltGpioPurposePioMpioDataXemRangeStrobe      = 142,  /**< MPIO Data Xem Range Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataXemSerdesStrobe     = 143,  /**< MPIO Data Xem SerDes Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataXemUnassignStrobe   = 144,  /**< MPIO Data Xem Unassign Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataXemRssi             = 145,  /**< MPIO Data Xem Rssi. */
    bcmEmmiOltGpioPurposePioMpioDataXemOtdr             = 146,  /**< MPIO Data Xem Otdr. */
    bcmEmmiOltGpioPurposePioMpioDataAndAgc              = 147,  /**< MPIO Data And Agc. */
    bcmEmmiOltGpioPurposePioMpioDataAndCdr              = 148,  /**< MPIO Data And Cdr. */
    bcmEmmiOltGpioPurposePioMpioDataAndStrobe0          = 149,  /**< MPIO Data And Strobe 0. */
    bcmEmmiOltGpioPurposePioMpioDataAndStrobe1          = 150,  /**< MPIO Data And Strobe 1. */
    bcmEmmiOltGpioPurposePioMpioDataAndRangeStrobe      = 151,  /**< MPIO Data And Range Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataAndSerdesStrobe     = 152,  /**< MPIO Data And SerDes Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataAndUnassignStrobe   = 153,  /**< MPIO Data And Unassign Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataAndRssi             = 154,  /**< MPIO Data And Rssi. */
    bcmEmmiOltGpioPurposePioMpioDataAndOtdr             = 155,  /**< MPIO Data And Otdr. */
    bcmEmmiOltGpioPurposePioMpioDataOrAgc               = 156,  /**< MPIO Data Or Agc. */
    bcmEmmiOltGpioPurposePioMpioDataOrCdr               = 157,  /**< MPIO Data Or Cdr. */
    bcmEmmiOltGpioPurposePioMpioDataOrStrobe0           = 158,  /**< MPIO Data Or Strobe 0. */
    bcmEmmiOltGpioPurposePioMpioDataOrStrobe1           = 159,  /**< MPIO Data Or Strobe 1. */
    bcmEmmiOltGpioPurposePioMpioDataOrRangeStrobe       = 160,  /**< MPIO Data Or Range Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataOrSerdesStrobe      = 161,  /**< Mpio Data And SerDes Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataOrUnassignStrobe    = 162,  /**< Mpio Data Or SerDes Strobe. */
    bcmEmmiOltGpioPurposePioMpioDataOrRssi              = 163,  /**< MPIO Data Or Rssi. */
    bcmEmmiOltGpioPurposePioMpioDataOrOtdr              = 164,  /**< MPIO Data Or Otdr. */
    bcmEmmiOltGpioPurposePioMpioDataEpmGrantId          = 165,  /**< MPIO Data Epm Grant Id. */
    bcmEmmiOltGpioPurposePioMpioDataXemGrantId          = 166,  /**< MPIO Data Xem Grant Id. */
    bcmEmmiOltGpioPurposePioMpioDataIn                  = 167,  /**< MPIO Data In. */
    bcmEmmiOltGpioPurposePioSystemStatus                = 192,  /**< System Status. */
    bcmEmmiOltGpioPurposePioLed10gDownPon0              = 193,  /**< LED 10G Down â€?PON 0. */
    bcmEmmiOltGpioPurposePioLed10gUpPon0                = 194,  /**< LED 10G Up â€?PON 0. */
    bcmEmmiOltGpioPurposePioLed1GDownPon0               = 195,  /**< LED 1G Down â€?PON 0. */
    bcmEmmiOltGpioPurposePioLed1GUpPon0                 = 196,  /**< LED 1G Up â€?PON 0. */
    bcmEmmiOltGpioPurposePioLed10gDownPon1              = 197,  /**< LED 10G Down â€?PON 1. */
    bcmEmmiOltGpioPurposePioLed10gUpPon1                = 198,  /**< LED 10G Up â€?PON 1. */
    bcmEmmiOltGpioPurposePioLed1GDownPon1               = 199,  /**< LED 1G Down â€?PON 1. */
    bcmEmmiOltGpioPurposePioLed1GUpPon1                 = 200,  /**< LED 1G Up â€?PON 1. */
    bcmEmmiOltGpioPurposePioLed10gDownPon2              = 201,  /**< LED 10G Down â€?PON 2. */
    bcmEmmiOltGpioPurposePioLed10gUpPon2                = 202,  /**< LED 10G Up â€?PON 2. */
    bcmEmmiOltGpioPurposePioLed1GDownPon2               = 203,  /**< LED 1G Down â€?PON 2. */
    bcmEmmiOltGpioPurposePioLed1GUpPon2                 = 204,  /**< LED 1G Up â€?PON 2. */
    bcmEmmiOltGpioPurposePioLed10gDownPon3              = 205,  /**< LED 10G Down â€?PON 3. */
    bcmEmmiOltGpioPurposePioLed10gUpPon3                = 206,  /**< LED 10G Up â€?PON 3. */
    bcmEmmiOltGpioPurposePioLed1GDownPon3               = 207,  /**< LED 1G Down â€?PON 3. */
    bcmEmmiOltGpioPurposePioLed1GUpPon3                 = 208,  /**< LED 1G Up â€?PON 3. */
    bcmEmmiOltGpioPurposePioLed10gDownPon4              = 209,  /**< LED 10G Down â€?PON 4. */
    bcmEmmiOltGpioPurposePioLed10gUpPon4                = 210,  /**< LED 10G Up â€?PON 4. */
    bcmEmmiOltGpioPurposePioLed1GDownPon4               = 211,  /**< LED 1G Down â€?PON 4. */
    bcmEmmiOltGpioPurposePioLed1GUpPon4                 = 212,  /**< LED 1G Up â€?PON 4. */
    bcmEmmiOltGpioPurposePioLed10gDownPon5              = 213,  /**< LED 10G Down â€?PON 5. */
    bcmEmmiOltGpioPurposePioLed10gUpPon5                = 214,  /**< LED 10G Up â€?PON 5. */
    bcmEmmiOltGpioPurposePioLed1GDownPon5               = 215,  /**< LED 1G Down â€?PON 5. */
    bcmEmmiOltGpioPurposePioLed1GUpPon5                 = 216,  /**< LED 1G Up â€?PON 5. */
    bcmEmmiOltGpioPurposePioLed10gDownPon6              = 217,  /**< LED 10G Down â€?PON 6. */
    bcmEmmiOltGpioPurposePioLed10gUpPon6                = 218,  /**< LED 10G Up â€?PON 6. */
    bcmEmmiOltGpioPurposePioLed1GDownPon6               = 219,  /**< LED 1G Down â€?PON 6. */
    bcmEmmiOltGpioPurposePioLed1GUpPon6                 = 220,  /**< LED 1G Up â€?PON 6. */
    bcmEmmiOltGpioPurposePioLed10gDownPon7              = 221,  /**< LED 10G Down â€?PON 7. */
    bcmEmmiOltGpioPurposePioLed10gUpPon7                = 222,  /**< LED 10G Up â€?PON 7. */
    bcmEmmiOltGpioPurposePioLed1GDownPon7               = 223,  /**< LED 1G Down â€?PON 7. */
    bcmEmmiOltGpioPurposePioLed1GUpPon7                 = 224,  /**< LED 1G Up â€?PON 7. */
    bcmEmmiOltGpioPurposePioLedTxNni0                   = 225,  /**< LED TX NNI 0. */
    bcmEmmiOltGpioPurposePioLedRxNni0                   = 226,  /**< LED RX NNI 0. */
    bcmEmmiOltGpioPurposePioLedTxNni1                   = 227,  /**< LED TX NNI 1. */
    bcmEmmiOltGpioPurposePioLedRxNni1                   = 228,  /**< LED RX NNI 1. */
    bcmEmmiOltGpioPurposePioLedTxNni2                   = 229,  /**< LED TX NNI 2. */
    bcmEmmiOltGpioPurposePioLedRxNni2                   = 230,  /**< LED RX NNI 2. */
    bcmEmmiOltGpioPurposePioLedTxNni3                   = 231,  /**< LED TX NNI 3. */
    bcmEmmiOltGpioPurposePioLedRxNni3                   = 232,  /**< LED RX NNI 3. */
    bcmEmmiOltGpioPurposePioLedTxNni4                   = 233,  /**< LED TX NNI 4. */
    bcmEmmiOltGpioPurposePioLedRxNni4                   = 234,  /**< LED RX NNI 4. */
    bcmEmmiOltGpioPurposePioLedTxNni5                   = 235,  /**< LED TX NNI 5. */
    bcmEmmiOltGpioPurposePioLedRxNni5                   = 236,  /**< LED RX NNI 5. */
    bcmEmmiOltGpioPurposePioLedTxNni6                   = 237,  /**< LED TX NNI 6. */
    bcmEmmiOltGpioPurposePioLedRxNni6                   = 238,  /**< LED RX NNI 6. */
    bcmEmmiOltGpioPurposePioLedTxNni7                   = 239,  /**< LED TX NNI 7. */
    bcmEmmiOltGpioPurposePioLedRxNni7                   = 240,  /**< LED RX NNI 7. */
    bcmEmmiOltGpioPurposePioSystemAlarm                 = 241,  /**< System Alarm. */
    bcmEmmiOltGpioPurposePioUserOutput0                 = 256,  /**< User Output0. */
    bcmEmmiOltGpioPurposePioUserOutput1                 = 257,  /**< User Output1. */
    bcmEmmiOltGpioPurposePioUserOutput2                 = 258,  /**< User Output2. */
    bcmEmmiOltGpioPurposePioUserOutput3                 = 259,  /**< User Output3. */
    bcmEmmiOltGpioPurposePioUserOutput4                 = 260,  /**< User Output4. */
    bcmEmmiOltGpioPurposePioUserOutput5                 = 261,  /**< User Output5. */
    bcmEmmiOltGpioPurposePioUserOutput6                 = 262,  /**< User Output6. */
    bcmEmmiOltGpioPurposePioUserOutput7                 = 263,  /**< User Output7. */
    bcmEmmiOltGpioPurposePioUserOutput8                 = 264,  /**< User Output8. */
    bcmEmmiOltGpioPurposePioUserOutput9                 = 265,  /**< User Output9. */
    bcmEmmiOltGpioPurposePioUserOutput10                = 266,  /**< User Output10. */
    bcmEmmiOltGpioPurposePioUserOutput11                = 267,  /**< User Output11. */
    bcmEmmiOltGpioPurposePioUserOutput12                = 268,  /**< User Output12. */
    bcmEmmiOltGpioPurposePioUserOutput13                = 269,  /**< User Output13. */
    bcmEmmiOltGpioPurposePioUserOutput14                = 270,  /**< User Output14. */
    bcmEmmiOltGpioPurposePioUserOutput15                = 271,  /**< User Output15. */
    bcmEmmiOltGpioPurposePioUserOutput16                = 272,  /**< User Output16. */
    bcmEmmiOltGpioPurposePioUserOutput17                = 273,  /**< User Output17. */
    bcmEmmiOltGpioPurposePioUserOutput18                = 274,  /**< User Output18. */
    bcmEmmiOltGpioPurposePioUserOutput19                = 275,  /**< User Output19. */
    bcmEmmiOltGpioPurposePioUserOutput20                = 276,  /**< User Output20. */
    bcmEmmiOltGpioPurposePioUserOutput21                = 277,  /**< User Output21. */
    bcmEmmiOltGpioPurposePioUserOutput22                = 278,  /**< User Output22. */
    bcmEmmiOltGpioPurposePioUserOutput23                = 279,  /**< User Output23. */
    bcmEmmiOltGpioPurposePioUserOutput24                = 280,  /**< User Output24. */
    bcmEmmiOltGpioPurposePioUserOutput25                = 281,  /**< User Output25. */
    bcmEmmiOltGpioPurposePioUserOutput26                = 282,  /**< User Output26. */
    bcmEmmiOltGpioPurposePioUserOutput27                = 283,  /**< User Output27. */
    bcmEmmiOltGpioPurposePioUserOutput28                = 284,  /**< User Output28. */
    bcmEmmiOltGpioPurposePioUserOutput29                = 285,  /**< User Output29. */
    bcmEmmiOltGpioPurposePioUserOutput30                = 286,  /**< User Output30. */
    bcmEmmiOltGpioPurposePioUserOutput31                = 287,  /**< User Output31. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon0         = 288,  /**< Optical TX Enable â€?PON 0. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon1         = 289,  /**< Optical TX Enable â€?PON 1. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon2         = 290,  /**< Optical TX Enable â€?PON 2. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon3         = 291,  /**< Optical TX Enable â€?PON 3. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon4         = 292,  /**< Optical TX Enable â€?PON 4. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon5         = 293,  /**< Optical TX Enable â€?PON 5. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon6         = 294,  /**< Optical TX Enable â€?PON 6. */
    bcmEmmiOltGpioPurposePioOpticalTxEnablePon7         = 295,  /**< Optical TX Enable â€?PON 7. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon0        = 296,  /**< PON 0 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon1        = 297,  /**< PON 1 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon2        = 298,  /**< PON 2 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon3        = 299,  /**< PON 3 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon4        = 300,  /**< PON 4 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon5        = 301,  /**< PON 5 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon6        = 302,  /**< PON 6 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioProtectionSwitchPon7        = 303,  /**< PON 7 â€?Protection Switching Switch Control. */
    bcmEmmiOltGpioPurposePioFailPon0                    = 304,  /**< Fail â€?PON 0. */
    bcmEmmiOltGpioPurposePioFailPon1                    = 305,  /**< Fail â€?PON 1. */
    bcmEmmiOltGpioPurposePioFailPon2                    = 306,  /**< Fail â€?PON 2. */
    bcmEmmiOltGpioPurposePioFailPon3                    = 307,  /**< Fail â€?PON 3. */
    bcmEmmiOltGpioPurposePioFailPon4                    = 308,  /**< Fail â€?PON 4. */
    bcmEmmiOltGpioPurposePioFailPon5                    = 309,  /**< Fail â€?PON 5. */
    bcmEmmiOltGpioPurposePioFailPon6                    = 310,  /**< Fail â€?PON 6. */
    bcmEmmiOltGpioPurposePioFailPon7                    = 311,  /**< Fail â€?PON 7. */
    bcmEmmiOltGpioPurposePioFlashProtect                = 312,  /**< Flash Protect. */
    bcmEmmiOltGpioPurposePioFlashAccessComplete         = 313,  /**< Flash Access Complete. */
    bcmEmmiOltGpioPurposePioFpgaResetN                  = 314,  /**< FPGA Reset N. */
    bcmEmmiOltGpioPurposePioTkResetM                    = 315   /**< Tk Reset M. */
} bcmEmmiOltGpioPurposePio;

int BcmOLT_GetGpioState(short int olt_id, bcmEmmiOltGpioPurposePio gpio_func, bool *gpio_state);
int BcmOLT_SetGpioState(short int olt_id, bcmEmmiOltGpioPurposePio gpio_func, bool gpio_state);

int BcmOLT_GetMdio(short int olt_id, unsigned long phy_id, unsigned long reg_address, unsigned long *reg_value);
int BcmOLT_SetMdio(short int olt_id, unsigned long phy_id, unsigned long reg_address, unsigned long reg_value);

int BcmOLT_GetReg(short int olt_id, unsigned long reg_address, unsigned long *reg_value);
int BcmOLT_SetReg(short int olt_id, unsigned long reg_address, unsigned long reg_value);


/** BSC Speed. 
 */
typedef enum bcmEmmiBscSpeed
{
    bcmEmmiBscSpeedOneHundredKbps                   = 100,  /**< 100 Kbps. */
    bcmEmmiBscSpeedFourHundredKbps                  = 400   /**< 400 Kbps. */
} bcmEmmiBscSpeed;

int BcmOLT_GetI2cData(short int olt_id, unsigned char bus_id, unsigned short bus_speed, unsigned short i2cDevAddr, unsigned char internalDevAddrCount, unsigned char internalDevAddr[], unsigned short *numBytesToRead, unsigned char returnedData[]);
int BcmOLT_SetI2cData(short int olt_id, unsigned char bus_id, unsigned short bus_speed, unsigned short i2cDevAddr, unsigned char internalDevAddrCount, unsigned char internalDevAddr[], unsigned short numBytesToWrite, unsigned char writedData[]);


int BcmOLT_IsCliOverEmmi(short int olt_id, bool *cli_enabled);
int BcmOLT_SetCliOverEmmi(short int olt_id, bool cli_enabled);
int BcmOLT_SendEmmiCli(short int olt_id, unsigned short str_len, unsigned char *cli_str);


/** Scheduler Flags. 
 */
typedef enum bcmEmmiSchedulerFlags
{
    bcmEmmiSchedulerFlagsNone                                   = 0,
    bcmEmmiSchedulerFlagsForceReport                            = 0x0001,   /**< Force the link to send a report with every burst. */
    bcmEmmiSchedulerFlagsForceReportExtraGrant                  = 0x0002    /**< TDM Only: Force the link to send a report on extra grant. */
} bcmEmmiSchedulerFlags;

/** Type of scheduler, solicted or TDM. 
 */
typedef enum bcmEmmiSchedulerType
{
    bcmEmmiSchedulerTypeSolicited                               = 0,    /**< Best effort scheduler. */
    bcmEmmiSchedulerTypeTdm                                     = 1 /**< Fixed interval scheduler. */
} bcmEmmiSchedulerType;

/** Scheduler Solicited. 
 */
typedef struct bcmEmmiSchedulerSolicited
{
    U32 bandwidth;  /**< Unit: kbps. */
    U16 burstSize;  /**< Unit: kb. */
    U16 weight; /**< Unit: kb. */
    U8 level;   /**< Scheduler level. */
} bcmEmmiSchedulerSolicited;

/** Scheduler TDM. 
 */
typedef struct bcmEmmiSchedulerTdm
{
    U32 interval;   /**< Unit: 96us */
    U16 length; /**< Unit: TQ */
    U16 extraGrantLength;   /**< Unit: TQ */
} bcmEmmiSchedulerTdm;

typedef struct BcmSlaQueueParams
{
    bcmEmmiSchedulerSolicited min; /* < Min shaper settings */
    bcmEmmiSchedulerSolicited max; /* < Max shaper settings */
} BcmSlaQueueParams;

typedef struct BcmSlaDbaParams
{
    U32 token_size;
    U8  polling_level;    /* < DBA lcheduling level (set to 0 on TK3721) */
    bcmEmmiSchedulerFlags dba_flags;
    bcmEmmiSchedulerTdm   tdm;
} BcmSlaDbaParams;

typedef struct BcmLinkSlaDbaInfo
{
    BcmSlaQueueParams sla;    /* < SLA parameters */
    BcmSlaDbaParams   dba;    /* < DBA parameters */
} BcmLinkSlaDbaInfo;

int BcmOnuGetSLA(short int olt_id, short int llid, BcmLinkSlaDbaInfo *SLA);
int BcmOnuSetSLA(short int olt_id, short int llid, BcmLinkSlaDbaInfo *SLA);


int BcmOLT_GetFecMode(short int olt_id, bcmEmmiEponRate pon_rate, bool *up_fec, bool *down_fec);
int BcmOLT_SetFecMode(short int olt_id, bcmEmmiEponRate pon_rate, bool up_fec, bool down_fec);


/** Identifies an EPON port strobe. 
 */
typedef enum bcmEmmiStrobe
{
    bcmEmmiStrobeAgc10g                             = 0,    /**< AGC strobe for 10G upstream. */
    bcmEmmiStrobeAgc1g                              = 1,    /**< AGC strobe for 1G upstream. */
    bcmEmmiStrobeCdr10g                             = 2,    /**< CDR strobe for 10G upstream. */
    bcmEmmiStrobeCdr1g                              = 3,    /**< CDR strobe for 1G upstream. */
    bcmEmmiStrobeStrobe0_10g                        = 4,    /**< Receive strobe 0 for 10G upstream. */
    bcmEmmiStrobeStrobe0_1g                         = 5,    /**< Receive strobe 0 for 1G upstream. */
    bcmEmmiStrobeStrobe1_10g                        = 6,    /**< Receive strobe 1 for 10G upstream. */
    bcmEmmiStrobeStrobe1_1g                         = 7 /**< Receive strobe 1 for 1G upstream. */
} bcmEmmiStrobe;

/** A signal polarity. 
 */
typedef enum bcmEmmiPolarity
{
    bcmEmmiPolarityActiveHigh                       = 0,    /**< This signal is active when high. */
    bcmEmmiPolarityActiveLow                        = 1 /**< This signal is active when low. */
} bcmEmmiPolarity;

/** A type of strobe offset. 
 */
typedef enum bcmEmmiStrobeOffsetType
{
    bcmEmmiStrobeOffsetTypeStartOfGrant             = 0,    /**< The strobe offset will be from the start of grant. */
    bcmEmmiStrobeOffsetTypeEndOfGrant               = 1 /**< The strobe offset will be from the end of grant. */
} bcmEmmiStrobeOffsetType;

/** Modes for a strobe. 
 */
typedef enum bcmEmmiStrobeMode
{
    bcmEmmiStrobeModeNormal                         = 0,    /**< The strobe will occur normally (once per grant). */
    bcmEmmiStrobeModePeriodic                       = 1 /**< The strobe will occur periodically during ranging. During non-ranging grants it will operate normally. */
} bcmEmmiStrobeMode;

/** A 10G upstream receive strobe configuration. 
 */
typedef struct bcmEmmiStrobe10g
{
    bcmEmmiPolarity polarity;   /**< The polarity of this strobe. */
    U32 offset; /**< The offset of the beginning of this strobe from the start of laser on in TQ (16ns). */
    S8 endOffset;   /**< The offset of the end of this strobe from the end of laser off in TQ (16ns). Positive values increase the width of the strobe; Negative values decrease the width. */
} bcmEmmiStrobe10g;

/** A 1G upstream receive strobe configuration. 
 */
typedef struct bcmEmmiStrobe1g
{
    bcmEmmiPolarity polarity;   /**< The polarity of this strobe. */
    bcmEmmiStrobeOffsetType offsetType; /**< Controls whether the offset is from the start fo grant or end of grant. */
    U32 offset; /**< The offset of the beginning of this strobe from the location specified in Offset Type in TQ (16ns). */
    U16 width;  /**< The width of this strobe in TQ (16ns). */
    bcmEmmiStrobeMode mode; /**< The mode for this strobe. */
} bcmEmmiStrobe1g;

/** A strobe configuration. 
 */
typedef struct bcmEmmiStrobeConfig
{
    bcmEmmiStrobe strobe;   /**< The strobe this configuration applies to. */
    union
    {
        struct
        {
            bcmEmmiStrobe10g strobeConfig;  /**< Configuration for the 10G AGC strobe. */
        } agc10g;

        struct
        {
            bcmEmmiStrobe1g strobeConfig;   /**< Configuration for the 1G AGC strobe. */
        } agc1g;

        struct
        {
            bcmEmmiStrobe10g strobeConfig;  /**< Configuration for the 10G CDR strobe. */
        } cdr10g;

        struct
        {
            bcmEmmiStrobe1g strobeConfig;   /**< Configuration for the 1G CDR strobe. */
        } cdr1g;

        struct
        {
            bcmEmmiStrobe10g strobeConfig;  /**< Configuration for the 10G receive strobe 0. */
        } strobe0_10g;

        struct
        {
            bcmEmmiStrobe1g strobeConfig;   /**< Configuration for the 1G receive strobe 0. */
        } strobe0_1g;

        struct
        {
            bcmEmmiStrobe10g strobeConfig;  /**< Configuration for the 10G receive strobe 1. */
        } strobe1_10g;

        struct
        {
            bcmEmmiStrobe1g strobeConfig;   /**< Configuration for the 1G receive strobe 1. */
        } strobe1_1g;
    }x;
} bcmEmmiStrobeConfig;

int BcmOLT_GetStrobeConfig(short int olt_id, bcmEmmiStrobe strobe_id, bcmEmmiStrobeConfig *strobe_config);
int BcmOLT_SetStrobeConfig(short int olt_id, bcmEmmiStrobe strobe_id, bcmEmmiStrobeConfig *strobe_config);

int BcmOLT_GetLaserOnOffTime(short int olt_id, bcmEmmiEponRate pon_rate, unsigned long *on_time, unsigned long *off_time);
int BcmOLT_SetLaserOnOffTime(short int olt_id, bcmEmmiEponRate pon_rate, unsigned long on_time, unsigned long off_time);


/** Optical hardware configuration and capabilities. 
 */
typedef enum bcmEmmiOpticalMonitoringBscMode
{
    bcmEmmiOpticalMonitoringBscModeDoNotReadBsc     = 0,    /**< Do Not Read BSC. */
    bcmEmmiOpticalMonitoringBscModeReadBsc          = 1 /**< Read BSC. */
} bcmEmmiOpticalMonitoringBscMode;

typedef struct bcmOpticalMonitoringConfiguration
{
    U32 flags;  /**< Reserved. */
    bcmEmmiOpticalMonitoringBscMode mode;   /**< Whether or not to read the BSC. */
    bcmEmmiBscBus bscBus;   /**< BSC bus. */
    bcmEmmiBscSpeed speed;  /**< BSC operating speed in kHz. */
    bcmEmmiBscDeviceId device;  /**< BSC device address (0xA2 for SFF-8472 compliant devices). */
    bcmEmmiBscRegister reg; /**< Register address on the BSC device. */
    U8 length;  /**< Length of the BSC device register (number of bytes to read). */
    float powerCorrections[5];  /**< Correction values for RX power calculation (IEEE 754 single precision floating point numbers). */
} bcmOpticalMonitoringConfiguration;

/** Modes of operation for optical power monitoring feature. 
 */
typedef enum bcmEmmiOltOpticalRunMode
{
    bcmEmmiOltOpticalRunModeOff                     = 0,    /**< Suspends collection of RX power and idle power statistics. */
    bcmEmmiOltOpticalRunModeOn                      = 1,    /**< Starts collection of RX power and idle stats over the entire range of links. */
    bcmEmmiOltOpticalRunModeNoIdle                  = 2,    /**< Stops collection of RX idle power â€?useful if you want to use the RX idle power for something else. */
    bcmEmmiOltOpticalRunModeIdle                    = 3,    /**< Only RX idle power is monitored. */
    bcmEmmiOltOpticalRunModeSingle                  = 4,    /**< Collects RX power for a single link continuously. */
    bcmEmmiOltOpticalRunModeOneShot                 = 5 /**< Sample only once. */
} bcmEmmiOltOpticalRunMode;

typedef struct bcmOpticalMonitoringControl
{
    U32 flags;  /**< Reserved. */
    bcmEmmiOltOpticalRunMode runMode;   /**< Run mode for collection of RX power stats. */
    bcmMacAddress linkMac;  /**< Link MAC Address (Ignored if run mode is not "single"). */
    U32 startStrobeOffset;  /**< Offset from the start of the grant to the start of the strobe in TQ. */
    U32 grantSize;  /**< Size of the grant in TQ. */
    U16 endStrobeOffset;    /**< Offset from the end of the strobe to the end of the grant in TQ. */
} bcmOpticalMonitoringControl;

int BcmOLT_GetOpticalMonitoringConfiguration(short int olt_id, bcmOpticalMonitoringConfiguration *config);
int BcmOLT_SetOpticalMonitoringConfiguration(short int olt_id, bcmOpticalMonitoringConfiguration *config);

int BcmOLT_GetOpticalMonitoringControl(short int olt_id, bcmOpticalMonitoringControl *config);
int BcmOLT_SetOpticalMonitoringControl(short int olt_id, bcmOpticalMonitoringControl *config);


int BcmAdapter_PAS5201_set_virtual_scope_adc_config(short int olt_id, PON_adc_config_t *adc_config);
int BcmAdapter_PAS5201_get_virtual_scope_adc_config(short int olt_id, PON_adc_config_t *adc_config);

int BcmAdapter_PAS5201_get_virtual_scope_measurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, void *configuration, void *result);  
int BcmAdapter_PAS5201_get_virtual_scope_rssi_measurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result);
int BcmAdapter_PAS5201_get_virtual_scope_onu_voltage(short int olt_id, short int onu_id, float *voltage, unsigned short int *sample, float *dbm);

/*-------wangjiah@2016-07-29----begin-------*/
int BcmAdapter_PAS5201_get_raw_statistics
                                 ( const PON_olt_id_t					 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   void								    *statistics_data,
								   PON_timestamp_t						*timestamp );
/*-------wangjiah@2016-07-29----end-------*/


/** Types of memory. 
 */
typedef enum bcmEmmiMemoryType
{
    bcmEmmiMemoryTypeDdr                                        = 0,    /**< The external DDR SDRAM. */
    bcmEmmiMemoryTypeFlash                                      = 1,    /**< External Flash. */
    bcmEmmiMemoryTypeInternalMemory                             = 2 /**< Internal Memory. */
} bcmEmmiMemoryType;

int BcmOLT_RunMemoryDiagnostic(short int olt_id, bcmEmmiMemoryType mem_type);


/** Loopback Location. 
 */
typedef enum bcmEmmiLoopbackLocation
{
    bcmEmmiLoopbackLocationPcs                                  = 0,    /**< Loopback at PCS. */
    bcmEmmiLoopbackLocationPcsBypass                            = 1 /**< Loopback at PCS Bypass. */
} bcmEmmiLoopbackLocation;

/** Loopback Direction. 
 */
typedef enum bcmEmmiLoopbackDirection
{
    bcmEmmiLoopbackDirectionDownstreamToUpstream                = 0,    /**< Loopback Downstream to Upstream traffic. */
    bcmEmmiLoopbackDirectionUpstreamToDownstream                = 1 /**< Loopback Upstream to Downstream traffic. */
} bcmEmmiLoopbackDirection;

/** Loopback State. 
 */
typedef enum bcmEmmiLoopbackState
{
    bcmEmmiLoopbackStateDisabled                                = 0,    /**< Loopback Disabled. */
    bcmEmmiLoopbackStateEnabled                                 = 1 /**< Loopback Enabled. */
} bcmEmmiLoopbackState;

int BcmOLT_GetNniPortLoopbackState(short int olt_id, bcmEmmiLoopbackLocation loc, bcmEmmiLoopbackDirection dir, bool *state);
int BcmOLT_SetNniPortLoopbackState(short int olt_id, bcmEmmiLoopbackLocation loc, bcmEmmiLoopbackDirection dir, bool state);

int BcmOLT_GetPonPortLoopbackState(short int olt_id, bcmEmmiLoopbackLocation loc, bcmEmmiLoopbackDirection dir, bool *state);
int BcmOLT_SetPonPortLoopbackState(short int olt_id, bcmEmmiLoopbackLocation loc, bcmEmmiLoopbackDirection dir, bool state);
#endif 


#if 1
/* --------------BCM-SDK's API(88650)----------------- */
int BhfBridgeGetMacAgingTime(short int olt_id, unsigned long *ageTimeInMs);
int BhfBridgeSetMacAgingTime(short int olt_id, unsigned long ageTimeInMs);

int BhfAdapter_PAS5201_GetAddrTable(short int olt_id, short int	*active_records, PON_address_table_t address_table);
int BhfAdapter_PAS5201_AddAddrTable(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int BhfAdapter_PAS5201_DeleteAddrTable(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int BhfAdapter_PAS5201_DeleteMacAddr(short int olt_id, TkMacAddress mac);

int BhfAdapter_PAS5201_GetLLIDAddrTable(short int olt_id, short int llid, short int *active_records, PON_address_table_t address_table);
int BhfAdapter_PAS5201_ResetLLIDAddrTable(short int olt_id, short int llid, PON_address_aging_t address_type);

int BhfAdapter_PAS5201_GetOltVlanTpid(short int olt_id, unsigned short int *tpid_outer, unsigned short int *tpid_inner);
int BhfAdapter_PAS5201_SetOltVlanTpid(short int olt_id, unsigned short int tpid_outer, unsigned short int tpid_inner);
int BhfAdapter_PAS5201_SetOnuVlanUplinkMode(short int olt_id, short int llid, PON_olt_vlan_uplink_config_t *vlan_uplink_config);
int BhfAdapter_PAS5201_SetVlanDownlinkMode(short int olt_id, PON_vlan_tag_t vlan_id, PON_olt_vid_downlink_config_t *vid_downlink_config);

int BhfAdapter_PAS5201_set_classification_rule(short int olt_id, const PON_pon_network_traffic_direction_t direction, const PON_olt_classification_t classification_entity, const void *classification_parameter, const PON_olt_classifier_destination_t destination);
int BhfAdapter_PAS5201_get_cni_status(short int olt_id, int * status);

int BhfBridgeGetP2PDataPathNum(short int olt_id, int *p2p_num);
int BhfBridgeOpenP2PDataPath(short int olt_id);
int BhfBridgeCloseP2PDataPath(short int olt_id);
int BhfBridgeUpdateP2PDataPath(short int olt_id, int p2p_num);

int BhfOnuSetP2PAccessList ( const short int olt_id, const short int onu_id, const unsigned short access_num, short int onu_ids[64], bool access, bool cover );
int BhfOnuGetP2PAccessList ( const short int  olt_id, const short int onu_id, unsigned short *access_num, short int onu_ids[64] );


int BhfBridgeGetOnuDataPathNum(short int olt_id, short int onu_id);
int BhfBridgeSetOnuDataPathNum(short int olt_id, short int onu_id, int path_num);

int BhfBridgeGetOltDataPathNum(short int olt_id);
int BhfBridgeOpenMultiDataPath(short int olt_id);
int BhfBridgeCloseMultiDataPath(short int olt_id);

/*added by liyang @2015-04-01 */
bool BcmOlt_exists ( const short int olt_id );

#endif 

#endif 
