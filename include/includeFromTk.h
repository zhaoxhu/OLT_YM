/**************************************************************
*
*   IncludeFromTk.h -- 
*
*  
*    Copyright (c)  2012.10 , GWD Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ----------|--- --------|---------------------|------------
*	1.00	        | 08/10/2012  | Creation				| liwei056
*
***************************************************************/
#ifndef  __INCLUDEFROMTK_H
#define __INCLUDEFROMTK_H


#ifndef PACK
#define PACK __attribute__((packed))
#endif

/* lint -strong(AJX,U8); */
typedef unsigned char U8;

/* lint -strong(AJX,U16); */
typedef unsigned short U16;

/* it's really just an array of 3 bytes, stored in big endian */
typedef struct
{
    U8 array[3];
} U24;

/* lint -strong(AJX,U32); */
typedef unsigned long U32;

/* lint -strong(AJX,U64); */
#ifndef INT64_INCLUDED
#define INT64_INCLUDED
typedef unsigned long long  U64;
#endif

/* lint -strong(AJX,S8); */
typedef signed char S8;

/* lint -strong(AJX,S16); */
typedef signed short S16;

/* lint -strong(AJX,S32); */
typedef signed long S32;

typedef U8 TkMacAddress[6];
typedef U32 TkIPv4Address;

typedef struct TkOui
    {
    U8 _u8[3];   /* < The 3 byte mac address bytes */
    } PACK TkOui;

#define TK_CTC_ENABLE          2
#define TK_CTC_DISABLE         1

#define TK_EXIT_OK             0
#define TK_EXIT_ERROR          -1
#define TK_HAVE_EXIST	       -2
#define TK_NOT_EXIST	       -3
#define TK_PARAMETER_ERROR     -5
#define TK_TIME_OUT            -6
#define TK_NOT_IMPLEMENTED	   -10
#define TK_MODE_ERROR	       -12
#define TK_NO_RESOURCE         -16
#define TK_HARDWARE_ERROR	   -17
#define TK_MEMORY_ERROR	       -18
#define TK_ERROR_OR_OK	       -21

#define TK_ERROR_BAD_FILE      (-999)
#define TK_ERROR_NEED_FILE     (-1000)
#define TK_OLT_NOT_EXIST       (-1003)
#define TK_QUERY_FAILED        (-1004) /* No record was found to match the */


#define TK_ADDRESS_TABLE_SIZE  16384


int TkSoftIsCompatibledWithFirmware ( const PON_olt_id_t olt_id, bool *is_compatibled );

int TK_init ( void );


int TkAdapter_PAS5201_Init();

int TK_assign_pashandler_function( const PAS_handler_functions_index_t     handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );
int TK_delete_pashandler_function( const unsigned short  handler_id );


int TkCTC_Stack_Init( const unsigned char            number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list );
int TkCTC_Stack_isInit( bool *init );


PON_STATUS TK_PASONU_init ( void );
PON_STATUS TK_PASONU_is_init ( bool *init );

 
/* Handler functions enum */
typedef enum
{
    TK_HANDLER_ETH_FRAME_RECEIVED,					    /* Ethernet frame originated from OLT's PON port was received  */
    TK_HANDLER_OAM_FRAME_RECEIVED,					    /* OAM frame originated from OLT's PON  port was received  */
    TK_HANDLER_EAP_FRAME_RECEIVED,					    /* EAP frame originated from OLT's PON  port was received  */
    TK_HANDLER_CAP_FRAME_RECEIVED,					    /* Captured Ethernet frame originated from OLT's PON / System  port was received  */
    TK_HANDLER_CAP_WITHDEST_FRAME_RECEIVED,				/* Captured Ethernet frame originated from OLT's PON / System  port was received  */

    TK_HANDLER_OLT_READY,						        /* OLT ready 									        */
    TK_HANDLER_OLT_RESET,						        /* OLT reset 									        */
    TK_HANDLER_OLT_ADD,						            /* OLT added 									        */
    TK_HANDLER_OLT_REMOVE,						        /* OLT removed     							        */

    TK_HANDLER_LINK_DISCOVERY,				            /* LLID registration to the PON					        */
    TK_HANDLER_LINK_LOSS,				                /* LLID deregistration (disconnection) from the PON          */
    TK_HANDLER_DENIED_LINK_DISCOVERY,                   /* Link's MAC not in AllowList  */    
    TK_HANDLER_LINK_AUTH_COMPLETED,				        /* Link's Authentication Result					        */
    TK_HANDLER_ONU_REGISTRATION,				        /* ONU registration to the PON					        */
    TK_HANDLER_ONU_DEREGISTRATION,				        /* ONU deregistration (disconnection) from the PON      */
    TK_HANDLER_DENIED_ONU_REGISTER,                     /* ONU's MAC not in AllowList  */    
    TK_HANDLER_ONU_AUTH_COMPLETED,				        /* ONU's Authentication Result     */

    TK_HANDLER_ALARM,							        /* Hardware / Software alarm					        */
    TK_HANDLER_PON_LOSS,                                /* PON loss */
    TK_HANDLER_CNI_LINK,                                /* CNI link                                             */
    TK_HANDLER_GPIO_CHANGED,							/* GPIO Change Notify					        */
    TK_HANDLER_MIGRATION_ATTEMPT,                       /* Migration attempt               */
    TK_HANDLER_LINK_MOVE_NOTIFY,                        /* ONU's Link is moved between OLT's Pon Ports */
    TK_HANDLER_ONU_RANGE_CHANGED,                       /* ONU's Range is changed              */
    TK_HANDLER_OAM_SA_MISMATCH,                         /* OAM's SA is Unexpected              */
    TK_HANDLER_MPCP_REGISTER_REPEAT,                    /* MPCP's register frams is duplicated              */

    TK_HANDLER_CTC_EXT_EVENT_NOTIFY,                    /* ONU's CTC Extended Event Notify */
    TK_HANDLER_OLT_CLI_DEBUG_OUTPUT,                    /* OLT's Cli Debug Output */

    TK_HANDLER_CNU_ARRIVAL,                             /* CNU is arrival */
    TK_HANDLER_CNU_DEPARTURE,                           /* CNU is depature */

    TK_HANDLER_IP_L3BIND_DETECTED,                      /* IPv4's L3Binding Detected  */    
    TK_HANDLER_IP_L3UNBIND_DETECTED,                    /* IPv4's L3Unbinding Detected  */    
    TK_HANDLER_IPv4_BIND_DETECTED,                      /* IPv4's Binding Detected  */    
    TK_HANDLER_IPv4_UNBIND_DETECTED,                    /* IPv4's Unbinding Detected  */    
    TK_HANDLER_IPv6_BIND_DETECTED,                      /* IPv6's Binding Detected  */    
    TK_HANDLER_IPv6_UNBIND_DETECTED,                    /* IPv6's Unbinding Detected  */    
    TK_HANDLER_PPPOE_BIND_DETECTED,                     /* PPPoE's Session Binding Detected  */    
    TK_HANDLER_PPPOE_UNBIND_DETECTED,                   /* PPPoE's Session Unbinding Detected  */    
    TK_HANDLER_DHCPv4_BIND_CONFLICT_DETECTED,           /* PPPoE's Session Binding Detected  */    
    TK_HANDLER_DHCPv6_BIND_CONFLICT_DETECTED,           /* PPPoE's Session Unbinding Detected  */    

    TK_HANDLER_REDUNDANCY_ONU_SWITCH_NOTIFY,            /* redundancy ONU is switched */
    TK_HANDLER_REDUNDANCY_OLT_LINK_ACTIVE,              /* redundancy OLT Link is actived */
    TK_HANDLER_REDUNDANCY_OLT_LINK_STANDBY,             /* redundancy OLT Link is standby */
    TK_HANDLER_INVALID_GRANT_DETECTED,                  /* ONU's grant is invalid  */    
    TK_HANDLER_INVALID_LLID_DETECTED,                   /* ONU's llid is invalid  */    

    TK_HANDLER_LAST_EXPO_HANDLER = TK_HANDLER_INVALID_LLID_DETECTED

} TK_handler_functions_index_t;

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
int TK_assign_handler_function
                                 ( const TK_handler_functions_index_t    handler_function_index, 
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
int TK_delete_handler_function( const unsigned short  handler_id );


bool OltIsExist ( const short int olt_id );
bool OnuIsExist ( const short int olt_id, const short int onu_id );
bool LinkIsOnline ( const short int olt_id, const short int llid );

int Tk_set_olt_base_address(unsigned short int olt_base_address);


/* OLT Initialization parameters struct 
**
**		olt_mac_address				    : OLT MAC address
**										  If MAC address is to be set by caller - valid unicast MAC 
**											address value
**										  If OLT default MAC address is used - OLT_DEFAULT_MAC_ADDRESS
**											(0xFF-0xFF-0xFF-0xFF-0xFF-0xFF - all 1s) the MAC address retrieve from the EEPROM
*/
typedef struct TK_olt_initialization_parameters_t
{
    unsigned char   	    olt_mac_address[6];
    unsigned char           support_tk_onus;
    unsigned char           ram_test;
    unsigned char           firmware_version[100];
	void				    *firmware_location;
	long int				firmware_size;
} TK_olt_initialization_parameters_t;

/* Add OLT result parameters struct 
**
**      olt_mac_address : The input parameter changes its value to the OLT's actual MAC     
**                        address, whether set explicitly by the input parameter or         
**                        taken from the OLT default (EEPROM) MAC address                   
*/
typedef struct TK_add_olt_result_t
{
	unsigned char  olt_mac_address[6];
} TK_add_olt_result_t;

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
int TK_add_olt 
                    ( const short int				         olt_id, 
				      const TK_olt_initialization_parameters_t  *olt_initialization_parameters,
                            TK_add_olt_result_t                 *add_olt_result );

int TK_remove_olt ( const short int  olt_id,
					   const int		send_shutdown_msg_to_olt, 
					   const int        reset_olt);

int TK_reset_olt ( const short int  olt_id );


typedef struct PON_device_versions_t
{
    short int  device_id;			/* Device ID number                 		*/
	short int  host_major;			/* Device host software major version		*/
	short int  host_minor;			/* Device host software minor version		*/
	short int  host_compilation;	/* Device host software compilation number  */
	short int  firmware_major;		/* Device firmware major version			*/
	short int  firmware_minor;		/* Device firmware minor version			*/
    short int  build_firmware;      /* Device firmware build version			*/
    short int  maintenance_firmware;/* Device firmware maintenance version  	*/
	short int  hardware_major;		/* Device hardware major version			*/
	short int  hardware_minor;		/* Device hardware minor version			*/
	PON_mac_t  system_mac;			/* Device System port MAC type				*/
	short int  ports_supported;		/* Device Ports (LLIDs)					    */
	short int  pon_vendors;		    /* added by liwei056@2013-7-30 for TkAdapter */
} PON_device_versions_t;	

int TkAdapter_PAS5201_GetOltChipVersion(short int olt_id, PON_device_versions_t *device_versions);
int TkAdapter_PAS5201_GetOnuChipVersion(short int olt_id, short int onu_id, PON_device_versions_t *device_versions);

int TkAdapter_PAS5201_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version);


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
int TK_send_oam 
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
int TK_send_frame 
                         ( const short int  					     olt_id, 
						   const short int							 llid,
						   const void								*content,
						   const unsigned short						 length );


/* Return TK-SOFT state (running / init etc.)
**
** Input Parameters:
**				none
**
** Return values:
**				TK-SOFT state represented by PON_module_state_t enum values
*/
PON_module_state_t Get_tk_soft_state ( void );



int TkLinkGetOnuID ( const short int     olt_id, 
						    const short int  llid,
						    short int       *onu_id );

int TK_authorize_onu 
                            ( const short int		  olt_id, 
							  const short int		  onu_id,
							  const bool              authorize_mode );

int TK_authorize_llid 
                            ( const short int		  olt_id, 
							  const short int		  llid,
							  const bool              authorize_mode );

int TkONU_Deregister(short int olt_id, short int onu_id);

int TkOLT_DeregisterLLID(short int olt_id, short int llid);
int TkOLT_SilentLLID(short int olt_id, short int llid);

int TkOltOpenDataPath(short int olt_id);
int TkOltCloseDataPath(short int olt_id);

int TkChip_GetOltPonPortID(short int olt_id);
int TkChip_GetOltEthPortID(short int olt_id);

int TkChip_GetCommonHostID(short int olt_id);


/* /////////////////////
 TkPortSpeed: Teknovus API structure

/// Port speeds
///////////////////// */
typedef enum TkPortSpeed
    {
    TkPortSpeedTenMbps                  = 0,    /* < 10 Mbps */
    TkPortSpeedOneHundredMbps           = 1,    /* < 100 Mbps */
    TkPortSpeedOneGbps                  = 2,    /* < 1000 Mbps */
    TkPortSpeedTwoGDownOneGUp           = 3,    /* < 2Gbps Downstream 1Gbps Upstream */
    TkPortSpeedTenGDownOneGUp           = 4,    /* < 10Gbps Downstream 1Gbps Upstream */
    TkPortSpeedTenGDownTenGUp           = 5     /* < 10Gbps Downstream 10Gbps Upstream */
    } TkPortSpeed;

/* /////////////////////
 TkMdiMode: Teknovus API structure

/// Mdi Mode
///////////////////// */
typedef enum TkMdiMode
    {
    TkMdiModeNone                       = 0,    /* < None */
    TkMdiModeAuto                       = 1,    /* < Auto */
    TkMdiModeManual                     = 2,    /* < Manual */
    TkMdiModeCrossover                  = 3     /* <Crossover */
    } TkMdiMode;

/* /////////////////////
 TkFecMode: Teknovus API structure

/// Fec Mode for OLT
///////////////////// */
typedef enum TkFecMode
    {
    TkFecModeOff                        = 0,    /* < Off */
    TkFecModeOn                         = 1,    /* < On globally */
    TkFecModePerLink                    = 2,    /* < On per Link */
    TkFecModePerOnu                     = 3,    /* < On per ONU */
    TkFecModeHostManagedPerLink         = 4     /* < Host managed per link */
    } TkFecMode;

/* /////////////////////
 TkRamanMitigationMode: Teknovus API structure

/// Possible Raman Mitigation settings
///////////////////// */
typedef enum TkRamanMitigationMode
    {
    TkRamanMitigationModeOff                = 0,    /* < Off */
    TkRamanMitigationModeIdlePacketInsert   = 1,    /* < Idle packet insert (random) */
    TkRamanMitigationModeScramble           = 2     /* < Scramble */
    } TkRamanMitigationMode;

typedef struct TkPortInfo
    {
    U8 speed; /* TkPortSpeed */
    U8 duplex;
    U8 flowctrl;
    U8 autonego;
    U8 reserved;
    U8 enabled;
    U8 linked;   /* TkFecMode */
    U8 mdimode; /* TkMdiMode */
    U8 upfec;   /* TkFecMode */
    U8 downfec; /* TkFecMode */
    U8 raman;   /* TkRamanMitigationMode */
    U8 filter;
    U8 isolate;
    TkMacAddress mac;
    U16 mtu;
    } PACK TkPortInfo;

typedef struct TkPortCfg
    {
    U8 speed; /* TkPortSpeed */
    U8 duplex;
    U8 flowctrl;
    U8 autonego;
    U8 reserved;
    U8 mdimode; /* TkMdiMode */
    U8 upfec;   /* TkFecMode */
    U8 downfec; /* TkFecMode */
    U8 raman;   /* TkRamanMitigationMode */
    U8 filter;
    U8 isolate;
    } PACK TkPortCfg;

int TkOLT_GetPort(short int olt_id, short int port_id, TkPortInfo *port_info);
int TkOLT_SetPort(short int olt_id, short int port_id, TkPortCfg *port_cfg);

int TkOLT_EnablePort(short int olt_id, short int port_id);
int TkOLT_DisablePort(short int olt_id, short int port_id);

int TkONU_GetPort(short int olt_id, short int onu_id, short int port_id, TkPortInfo *port_info);
int TkONU_SetPort(short int olt_id, short int onu_id, short int port_id, TkPortCfg *port_cfg);

int TkONU_EnablePort(short int olt_id, short int onu_id, short int port_id);
int TkONU_DisablePort(short int olt_id, short int onu_id, short int port_id);

int TkOltSetOpticalTxMode(short int olt_id, int tx_mode);
int TkOltGetOpticalTxMode(short int olt_id, int *tx_mode);


typedef struct TkOltAdditionalVlanEthertypes
    {
    U16 cvlanEtherType; /* < Alternate CVLAN type (in addition to 0x8100) that OLT will recognize as CVLAN */
    U8 useCvlanUp;    /* < Use this CVLAN type to tag upstream */
    U8 useCvlanDown;  /* < Use this CVLAN type to tag downstream */
    U16 svlanEtherType; /* < Alternate SVLAN type (in addition to 0x88A8) that OLT will recognize as SVLAN */
    U16 sReserve;   /* < E264's Unknown Field(88 e7) */
    } PACK TkOltAdditionalVlanEthertypes;

int TkOLT_GetVlanExtType(short int olt_id, TkOltAdditionalVlanEthertypes *types);
int TkOLT_SetVlanExtType(short int olt_id, TkOltAdditionalVlanEthertypes *types);

int TkAdapter_PAS5201_SetOnuVlanUplinkMode(short int olt_id, short int onu_id, short int llid, PON_olt_vlan_uplink_config_t *vlan_uplink_config);
int TkAdapter_PAS5201_SetVlanDownlinkMode(short int olt_id, PON_vlan_tag_t vlan_id, PON_olt_vid_downlink_config_t *vid_downlink_config);


typedef struct TkDefaultOamRate
    {
    U8 minRate; /* < Number of seconds between OAM heartbeats */
    U8 maxRate; /* < Maximum number of OAM frames per second */
    U8 oamTimeout;  /* < When sending OAM to the ONU, how long to wait for the ONU to respond in seconds */
    } PACK TkDefaultOamRate;

#define TK_OAM_RATE_MAX           0

int TkOLT_GetOamRate(short int olt_id, TkDefaultOamRate *rate);
int TkOLT_SetOamRate(short int olt_id, TkDefaultOamRate *rate);


int TkAdapter_PAS5201_GetAddrTable(short int olt_id, short int	*active_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_AddAddrTable(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_DeleteAddrTable(short int olt_id, short int num_of_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_DeleteMacAddr(short int olt_id, TkMacAddress mac);

int TkAdapter_PAS5201_GetLLIDAddrTable(short int olt_id, short int llid, short int *active_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_ResetLLIDAddrTable(short int olt_id, short int llid, PON_address_aging_t address_type);
int TkAdapter_PAS5201_GetLlidByMac(short int olt_id, mac_address_t mac, short int *llid);

int TkAdapter_PAS5201_GetAllOnuParams(short int olt_id, short int *number, PAS_onu_parameters_t onu_parameters);


/* PON MAC控制项的标识 */
typedef enum PON_mac_ctrl_id_t
{  
    PON_MAC_CTL_FLAG_NONE = 0,

    PON_MAC_CTL_FLAG_MIN = 1,
    PON_MAC_CTL_FLAG_MAX = 2,
    PON_MAC_CTL_FLAG_OPT = 4,

    PON_MAC_CTL_FLAG_ALL = 0xFF
} PON_mac_ctrl_id_t;

typedef struct TkDestinationLearnControl
    {
    U16 minLearningEntries; /* < Minimum (guaranteed) learning entries, 0 to turn learning off */
    U16 maxLearningEntries; /* < Maximum (if available) learning entries, should be 0 if min is 0, > min otherwise */
    U16 options;            /* TkDestinationLearningOptions< Learning options */
    U16 entryCount;         /* < Current number of entries learned */
    } PACK TkDestinationLearnControl;

int TkBridgeSetOnuDestMacCtrl(short int olt_id, short int onu_id, unsigned int ctrl_flags, TkDestinationLearnControl *mac_ctrl);


/* /////////////////////
 TkEncryptMode: Teknovus API structure

/// Possible encryption modes
///////////////////// */
typedef enum TkEncryptMode
    {
    TkEncryptModeNone                                   = 0,    /* < No encryption */
    TkEncryptModeAes                                    = 1,    /* < 1G: AES-CFB */
    TkEncryptModeTripleChurning                         = 2,    /* < Triple churning (CTC mode) */
    TkEncryptModeZeroOverheadAes                        = 3     /* < Zero overhead AES (10G only) */
    } TkEncryptMode;

typedef struct TkEncryptionInfo
    {
    U8 mode; /* TkEncryptMode< Mode to put the link into */
    U16 keyExchangeTimeout; /* < Key exchange timeout for mode (seconds) */
    } PACK TkEncryptionInfo;

int TkOLT_GetPortEncryptMode(short int olt_id, short int port_id, TkEncryptionInfo *encrypt_info);
int TkOLT_SetPortEncryptMode(short int olt_id, short int port_id, TkEncryptionInfo *encrypt_info);

/* 
 TkEncryptDirection: Teknovus API structure

/// Directions for encryption modes
 */
typedef enum TkEncryptDirection
    {
    TkEncryptDirectionDownOnly                                          = 1,    /* < Encrypt downstream only */
    TkEncryptDirectionBiDirectional                                     = 2     /* < Encrypt downstream and upstream */
    } TkEncryptDirection;

typedef struct TkLinkEncryptionInfo
    {
    U8 encryptionMode;   /* TkEncryptMode< Encryption mode to set */
    U8 encryptionDirection; /* TkEncryptDirection< Direction to encrypt */
    U16 keyExchangeTime;    /* < Maximum time between key exchanges (in seconds) */
    } PACK TkLinkEncryptionInfo;

int TkOLT_GetLinkEncryptMode2(short int olt_id, short int llid, TkLinkEncryptionInfo *encrypt_info);
int TkOLT_SetLinkEncryptMode2(short int olt_id, short int llid, TkLinkEncryptionInfo *encrypt_info);

int TkOLT_GetLinkEncryptMode(short int olt_id, short int llid, TkEncryptionInfo *encrypt_info);
int TkOLT_SetLinkEncryptMode(short int olt_id, short int llid, TkEncryptionInfo *encrypt_info);
int TkOLT_DeleteLinkEncryptMode(short int olt_id, short int llid);

int TkOnuEncrypt(short int olt_id, short int onu_id, short int link_id, short int llid, unsigned char encrypt_mode, unsigned short keyExchangeTimeout);


int TkOLT_GetMacAgingTime(short int olt_id, unsigned long *ageTimeInMs);
int TkOLT_SetMacAgingTime(short int olt_id, unsigned long ageTimeInMs);

typedef enum TkOltGpioPurpose
    {
    TkOltGpioPurposeUnused                                  = 0,    /* < Pin is not in use */
    TkOltGpioPurposeUserInput0                              = 1,    /* < User Input 0 */
    TkOltGpioPurposeUserInput1                              = 2,    /* < User Input 1 */
    TkOltGpioPurposeUserInput2                              = 3,    /* < User Input 2 */
    TkOltGpioPurposeUserInput3                              = 4,    /* < User Input 3 */
    TkOltGpioPurposeUserInput4                              = 5,    /* < User Input 4 */
    TkOltGpioPurposeUserInput5                              = 6,    /* < User Input 5 */
    TkOltGpioPurposeUserInput6                              = 7,    /* < User Input 6 */
    TkOltGpioPurposeUserInput7                              = 8,    /* < User Input 7 */
    TkOltGpioPurposeUserInput8                              = 9,    /* < User Input 8 */
    TkOltGpioPurposeUserInput9                              = 10,   /* < User Input 9 */
    TkOltGpioPurposeUserInput10                             = 11,   /* < User Input 10 */
    TkOltGpioPurposeUserInput11                             = 12,   /* < User Input 11 */
    TkOltGpioPurposeUserInput12                             = 13,   /* < User Input 12 */
    TkOltGpioPurposeUserInput13                             = 14,   /* < User Input 13 */
    TkOltGpioPurposeUserInput14                             = 15,   /* < User Input 14 */
    TkOltGpioPurposeUserInput15                             = 16,   /* < User Input 15 */
    TkOltGpioPurposeUserInput16                             = 17,   /* < User Input 16 */
    TkOltGpioPurposeUserInput17                             = 18,   /* < User Input 17 */
    TkOltGpioPurposeUserInput18                             = 19,   /* < User Input 18 */
    TkOltGpioPurposeUserInput19                             = 20,   /* < User Input 19 */
    TkOltGpioPurposeUserInput20                             = 21,   /* < User Input 20 */
    TkOltGpioPurposeUserInput21                             = 22,   /* < User Input 21 */
    TkOltGpioPurposeUserInput22                             = 23,   /* < User Input 22 */
    TkOltGpioPurposeUserInput23                             = 24,   /* < User Input 23 */
    TkOltGpioPurposeUserInput24                             = 25,   /* < User Input 24 */
    TkOltGpioPurposeUserInput25                             = 26,   /* < User Input 25 */
    TkOltGpioPurposeUserInput26                             = 27,   /* < User Input 26 */
    TkOltGpioPurposeUserInput27                             = 28,   /* < User Input 27 */
    TkOltGpioPurposeUserInput28                             = 29,   /* < User Input 28 */
    TkOltGpioPurposeUserInput29                             = 30,   /* < User Input 29 */
    TkOltGpioPurposeUserInput30                             = 31,   /* < User Input 30 */
    TkOltGpioPurposeUserInput31                             = 32,   /* < User Input 31 */
    TkOltGpioPurposeStatsOff                                = 33,   /* < Stats Off */
    TkOltGpioPurposeBaud57600                               = 34,   /* < Baud 57600 */
    TkOltGpioPurposeCliOnly                                 = 35,   /* < CLI Only Mode */
    TkOltGpioPurposeResetOlt                                = 36,   /* < Reset OLT */
    TkOltGpioPurposeAlarmTemperature                        = 37,   /* < Alarm -- Temperature */
    TkOltGpioPurposeAlarmPower                              = 38,   /* < Alarm -- Power */
    TkOltGpioPurposePulseHandler                            = 39,   /* < Pulse Handler */
    TkOltGpioPurposeOpticalTxDegradePon0                    = 40,   /* < Optical Tx Degrade -- PON 0 */
    TkOltGpioPurposeOpticalTxDegradePon1                    = 41,   /* < Optical Tx Degrade -- PON 1 */
    TkOltGpioPurposeOpticalTxFailurePon0                    = 42,   /* < Optical Tx Failure -- PON 0 */
    TkOltGpioPurposeOpticalTxFailurePon1                    = 43,   /* < Optical Tx Failure -- PON 1 */
    TkOltGpioPurposeUserAlarm0                              = 49,   /* < User Alarm 0 */
    TkOltGpioPurposeUserAlarm1                              = 50,   /* < User Alarm 1 */
    TkOltGpioPurposeTdo                                     = 51,   /* < TDO */
    TkOltGpioPurposeOptMonAckPort0                          = 53,   /* < Optical Monitoring Ack Port 0 */
    TkOltGpioPurposeOptMonAckPort1                          = 54,   /* < Optical Monitoring Ack Port 1 */
    TkOltGpioPurposeTriggerPSPon0                           = 55,   /* < PON-0 -- Input Protection Switching Trigger */
    TkOltGpioPurposeTriggerPSPon1                           = 56,   /* < PON-1 -- Input Protection Switching Trigger */
    TkOltGpioPurposeI2c_Sda1                                = 92,   /* < I2C -- SDA1 */
    TkOltGpioPurposeI2c_Scl1                                = 93,   /* < I2C -- SCL1 */
    TkOltGpioPurposeI2c_Sda0                                = 94,   /* < I2C -- SDA0 */
    TkOltGpioPurposeI2c_Scl0                                = 95,   /* < I2C -- SCL0 */
    TkOltGpioPurposeLedLinkRegisteredPon0                   = 96,   /* < LED -- Link Registered -- PON 0 */
    TkOltGpioPurposeLedLinkRegisteredPon1                   = 97,   /* < LED -- Link Registered -- PON 1 */
    TkOltGpioPurposeLedSpeedPon0                            = 98,   /* < LED -- Speed -- PON 0 */
    TkOltGpioPurposeLedSpeedPon1                            = 99,   /* < LED -- Speed -- PON 1 */
    TkOltGpioPurposeLedTxActivityNni0                       = 100,  /* < LED -- Tx Activity -- NNI-0 */
    TkOltGpioPurposeLedTxActivityNni1                       = 101,  /* < LED -- Tx Activity -- NNI-1 */
    TkOltGpioPurposeLedRxActivityNni0                       = 102,  /* < LED -- Rx Activity -- NNI-0 */
    TkOltGpioPurposeLedRxActivityNni1                       = 103,  /* < LED -- Rx Activity -- NNI-1 */
    TkOltGpioPurposeLedSystemStatus                         = 104,  /* < LED -- System Status */
    TkOltGpioPurposeLedAlarmPon0                            = 105,  /* < LED -- Alarm PON 0 */
    TkOltGpioPurposeLedAlarmPon1                            = 106,  /* < LED -- Alarm PON 1 */
    TkOltGpioPurposeLedOltReady                             = 107,  /* < LED -- Fiber A */
    TkOltGpioPurposeUserOutput0                             = 160,  /* < User Output 0 */
    TkOltGpioPurposeUserOutput1                             = 161,  /* < User Output 1 */
    TkOltGpioPurposeUserOutput2                             = 162,  /* < User Output 2 */
    TkOltGpioPurposeUserOutput3                             = 163,  /* < User Output 3 */
    TkOltGpioPurposeUserOutput4                             = 164,  /* < User Output 4 */
    TkOltGpioPurposeUserOutput5                             = 165,  /* < User Output 5 */
    TkOltGpioPurposeUserOutput6                             = 166,  /* < User Output 6 */
    TkOltGpioPurposeUserOutput7                             = 167,  /* < User Output 7 */
    TkOltGpioPurposeUserOutput8                             = 168,  /* < User Output 8 */
    TkOltGpioPurposeUserOutput9                             = 169,  /* < User Output 9 */
    TkOltGpioPurposeUserOutput10                            = 170,  /* < User Output 10 */
    TkOltGpioPurposeUserOutput11                            = 171,  /* < User Output 11 */
    TkOltGpioPurposeUserOutput12                            = 172,  /* < User Output 12 */
    TkOltGpioPurposeUserOutput13                            = 173,  /* < User Output 13 */
    TkOltGpioPurposeUserOutput14                            = 174,  /* < User Output 14 */
    TkOltGpioPurposeUserOutput15                            = 175,  /* < User Output 15 */
    TkOltGpioPurposeUserOutput16                            = 176,  /* < User Output 16 */
    TkOltGpioPurposeUserOutput17                            = 177,  /* < User Output 17 */
    TkOltGpioPurposeUserOutput18                            = 178,  /* < User Output 18 */
    TkOltGpioPurposeUserOutput19                            = 179,  /* < User Output 19 */
    TkOltGpioPurposeUserOutput20                            = 180,  /* < User Output 20 */
    TkOltGpioPurposeUserOutput21                            = 181,  /* < User Output 21 */
    TkOltGpioPurposeUserOutput22                            = 182,  /* < User Output 22 */
    TkOltGpioPurposeUserOutput23                            = 183,  /* < User Output 23 */
    TkOltGpioPurposeUserOutput24                            = 184,  /* < User Output 24 */
    TkOltGpioPurposeUserOutput25                            = 185,  /* < User Output 25 */
    TkOltGpioPurposeUserOutput26                            = 186,  /* < User Output 26 */
    TkOltGpioPurposeUserOutput27                            = 187,  /* < User Output 27 */
    TkOltGpioPurposeUserOutput28                            = 188,  /* < User Output 28 */
    TkOltGpioPurposeUserOutput29                            = 189,  /* < User Output 29 */
    TkOltGpioPurposeUserOutput30                            = 190,  /* < User Output 30 */
    TkOltGpioPurposeUserOutput31                            = 191,  /* < User Output 31 */
    TkOltGpioPurposeOpticalTxEnablePon0                     = 192,  /* < Optical Tx Enable -- PON 0 */
    TkOltGpioPurposeOpticalTxEnablePon1                     = 193,  /* < Optical Tx Enable -- PON 1 */
    TkOltGpioPurposePon0SpeedSelect                         = 194,  /* < PON 0 -- Speed Select */
    TkOltGpioPurposePon1SpeedSelect                         = 195,  /* < PON 1 -- Speed Select */
    TkOltGpioPurposeTms                                     = 196,  /* < TMS */
    TkOltGpioPurposeTck                                     = 197,  /* < TCK */
    TkOltGpioPurposeTdi                                     = 198,  /* < TDI */
    TkOltGpioPurposePon0Enable1G                            = 200,  /* < PON 0 -- Enable 1G */
    TkOltGpioPurposePon0Enable2G                            = 201,  /* < PON 0 -- Enable 2G */
    TkOltGpioPurposePon1Enable1G                            = 202,  /* < PON 1 -- Enable 1G */
    TkOltGpioPurposePon1Enable2G                            = 203,  /* < PON 1 -- Enable 2G */
    TkOltGpioPurposePon0SyncEn1G                            = 204,  /* < PON 0 -- Enable Sync for 1G */
    TkOltGpioPurposePon1SyncEn1G                            = 205,  /* < PON 1 -- Enable Sync for 1G */
    TkOltGpioPurposePon0LockToRxData                        = 206,  /* < PON 0 -- Lock to RX Data */
    TkOltGpioPurposePon1LockToRxData                        = 207,  /* < PON 1 -- Lock to RX Data */
    TkOltGpioPurposeFlashProtect                            = 208,  /* < Flash Protect */
    TkOltGpioPurposeProtectionSwitchPon0                    = 209,  /* < PON-0 -- Protection Switching Switch Control */
    TkOltGpioPurposeProtectionSwitchPon1                    = 210,  /* < PON-1 -- Protection Switching Switch Control */
    TkOltGpioPurposeFailPon0                                = 211,  /* < PON-0 -- Fail Indication */
    TkOltGpioPurposeFailPon1                                = 212   /* < PON-1 -- Fail Indication */
    } TkOltGpioPurpose;

int TkOLT_GetGpioState(short int olt_id, TkOltGpioPurpose gpio_func, unsigned char *gpio_state);
int TkOLT_SetGpioState(short int olt_id, TkOltGpioPurpose gpio_func, unsigned char gpio_state);

int TkOLT_GetPortMdio(short int olt_id, short int port_id, unsigned short reg_address, unsigned short *reg_value);
int TkOLT_SetPortMdio(short int olt_id, short int port_id, unsigned short reg_address, unsigned short reg_value);

int TkOLT_GetI2cDataV1(short int olt_id, unsigned char bus_id, unsigned char bus_speed, unsigned char i2cDevAddr, unsigned char internalDevAddr, unsigned short *numBytesToRead, unsigned char returnedData[]);
int TkOLT_SetI2cDataV1(short int olt_id, unsigned char bus_id, unsigned char bus_speed, unsigned char i2cDevAddr, unsigned char internalDevAddr, unsigned short numBytesToWrite, unsigned char writedData[]);

int TkOLT_GetI2cDataV2(short int olt_id, unsigned char bus_id, unsigned short bus_speed, unsigned short i2cDevAddr, unsigned char internalDevAddrCount, unsigned char internalDevAddr[], unsigned short *numBytesToRead, unsigned char returnedData[]);
int TkOLT_SetI2cDataV2(short int olt_id, unsigned char bus_id, unsigned short bus_speed, unsigned short i2cDevAddr, unsigned char internalDevAddrCount, unsigned char internalDevAddr[], unsigned short numBytesToWrite, unsigned char writedData[]);


int TkOLT_IsCliHost(short int olt_id, unsigned char *cli_enabled);
int TkOLT_SetCliHost(short int olt_id, unsigned char cli_enabled);
int TkOLT_SendHostCli(short int olt_id, unsigned short str_len, unsigned char *cli_str);


typedef struct TkOltFirmwareInfo
    {
    U16 bootVersion; /* < Version of bootstrap loader (if any) */
    U32 bootCrc32;   /* < CRC-32 of boot loader serves as additional unique identifier and verification */
    U16 person1Version; /* < Version of presonality (if any) */
    U32 person1Crc32;   /* < CRC-32 of presonality serves as additional unique identifier and verification */
    U16 app0Version; /* < Version of main application software running on the ONU */
    U32 app0Crc32;   /* < CRC-32 of application software serves as unique ID and verification */
    U16 app1Version; /* < Version of main application software running on the ONU */
    U32 app1Crc32;   /* < CRC-32 of application software serves as unique ID and verification */
    U16 diagVersion; /* < Version of diagnostic software (if any) */
    U32 diagCrc32;   /* < CRC-32 of diagnostic software serves as additional unique identifier and verification */
    U16 person2Version; /* < Version of presonality (if any) */
    U32 person2Crc32;   /* < CRC-32 of presonality serves as additional unique identifier and verification */
    } PACK TkOltFirmwareInfo;

int TkOLT_GetOltVersion(short int olt_id, TkOltFirmwareInfo *versions);

/* 
 TkOnuVendorTypes: Teknovus API structure

/// ONU vendor types
 */
typedef enum TkOnuVendorTypes
    {
    TkOnuVendorTypesNone                                    = 0,
    TkOnuVendorTypesTeknovus                                = 1,    /* < Teknovus */
    TkOnuVendorTypesInterop                                 = 2,    /* < Interop */
    TkOnuVendorTypesKTExtInterop                            = 4,    /* < KT extended interop */
    TkOnuVendorTypesDasan                                   = 8,    /* < Dasan */
    TkOnuVendorTypesPmc                                     = 16    /* < PMC */
    } TkOnuVendorTypes;


typedef struct TkOnuFirmwareInfo
    {
    U16 bootVersion; /* < Version of bootstrap loader (if any) */
    U32 bootCrc32;   /* < CRC-32 of boot loader serves as additional unique identifier and verification */
    U16 personVersion; /* < Version of presonality (if any) */
    U32 personCrc32;   /* < CRC-32 of presonality serves as additional unique identifier and verification */
    U16 app0Version; /* < Version of main application software running on the ONU */
    U32 app0Crc32;   /* < CRC-32 of application software serves as unique ID and verification */
    U16 app1Version; /* < Version of main application software running on the ONU */
    U32 app1Crc32;   /* < CRC-32 of application software serves as unique ID and verification */
    U16 diagVersion; /* < Version of diagnostic software (if any) */
    U32 diagCrc32;   /* < CRC-32 of diagnostic software serves as additional unique identifier and verification */
    } PACK TkOnuFirmwareInfo;

int TkONU_GetOnuVersion(short int olt_id, short int onu_id, TkOnuFirmwareInfo *versions);

typedef struct TkOltLinkInfo
    {
    U16 length; /* < Length of OLT specific ONU info */
    TkOui vendorOui;   /* < The IEEE OUI of the manufacturer of the ONU (as retrieved from ONU personality) */
    U16 productCode;    /* < The product code of the ONU (as retrieved from ONU personality) */
    U16 productVersion; /* < The product version of the ONU (as retrieved from ONU personality) */
    U16 llid;   /* < The currently assigned LLID */
    U16 rtt;    /* < The current round trip time (range value) */
    U8 downstreamEponPort;  /* < Which EPON port the ONU is registered on (downstream) */
    U16 vendorType;    /* TkOnuVendorTypes< ONU vendor type */
    U8 reportingFormats; /* TkOnuReportingFormats< Reporting format flags */
    U8 pendingGrantsLimit;  /* < Pending grants limit */
    U8 upstreamSpeeds;    /* TkEponSpeeds< Upstream speeds the ONU is capable of */
    U8 upstreamEponPort;    /* < Which EPON port the ONU is registered on (upstream) */
    U8 numUpstreamChannels; /* < Number of upstream channels (primarily for CMC ONUs, non CMC ONUs are generally 1) */
    U8 numDownstreamChannels;   /* < Number of downstream channels (primarily for CMC ONUs, non CMC ONUs are generally 1) */
    U8 numCnusSupported;    /* < Number of CNUs supported (nonzero implies this is a CMC ONU) */
    } PACK TkOltLinkInfo;

int TkLinkGetOnuRegisterInfo ( short int olt_id, 
						    short int        llid,
						    TkMacAddress     link_mac,
						    short int       *onu_id,
						    TkMacAddress     onu_mac,
						    TkOltLinkInfo   *link_info,
						    unsigned char   *max_links_num,
						    unsigned char   *curr_links_num,
						    unsigned char   *max_cm_num,
						    unsigned char   *curr_cm_num );

int TkGetLinkMacAddress( const short int	    olt_id, 
								const short int     llid,
								mac_address_t       link_mac );

int TkONU_Reset(short int olt_id, short int onu_id);
int TkCTC_ResetOnu(short int olt_id, short int onu_id);


/* /////////////////////
 TkSlaShaperParams: Teknovus API structure

/// Setting for upstream/downstream SLA paramters
///////////////////// */
typedef struct TkSlaShaperParams
    {
    U32 bandwidth;  /* < Bandwidth (Kbps) */
    U16 burst;  /* < Burst size KB */
    U8 schedulerLevel;  /* < Scheduler level */
    U16 weight; /* < Scheduler tokens/weight */
    } PACK TkSlaShaperParams;

/* /////////////////////
 TkSlaQueueParams: Teknovus API structure

/// Per-queue SLA parameters
///////////////////// */
typedef struct TkSlaQueueParams
    {
    TkSlaShaperParams min; /* < Min shaper settings */
    TkSlaShaperParams max; /* < Max shaper settings */
    } PACK TkSlaQueueParams;

/* PON芯片的桥通路 */
typedef enum PON_bridge_mode_t
{  
    PON_BRIDGE_PATH_NONE = 0,

    /* 1:1's BothDir */
    PON_BRIDGE_PATH_S_DOWNLINK,
    PON_BRIDGE_PATH_S_UPLINK,

    /* P2P's BothDir */
    PON_BRIDGE_PATH_P_DOWNLINK,
    PON_BRIDGE_PATH_P_UPLINK,

    /* M:1's BothDir */
    PON_BRIDGE_PATH_M_DOWNLINK,
    PON_BRIDGE_PATH_M_UPLINK,

    PON_BRIDGE_PATH_MAX
} PON_bridge_path_t;

int TkBridgeGetLinkSLA(short int olt_id, short int llid, int bridge_path, TkSlaQueueParams *sla);
int TkBridgeSetLinkSLA(short int olt_id, short int llid, int bridge_path, TkSlaQueueParams *sla);
int TkBridgeDftLinkSLA(short int olt_id, short int llid, int bridge_path);


/* /////////////////////
 TkSlaDbaOptions: Teknovus API structure

/// Global SLA DBA options
///////////////////// */
typedef enum TkSlaDbaOptions
    {
    TkSlaDbaOptionsNone                                     = 0,
    TkSlaDbaOptionsForceReport                              = 1,    /* < Force a report on each grant */
    TkSlaDbaOptionsDisableOamUplift                         = 2,    /* < Prevents extra bandwidth for OAM from being added to the SLA */
    TkSlaDbaOptionsAll                                      = 255   /* < All options set (also used to indicate 'not applicable') */
    } TkSlaDbaOptions;

/* /////////////////////
 TkSlaDbaParams: Teknovus API structure

/// DBA SLA parameters
///////////////////// */
typedef struct TkSlaDbaParams
    {
    U8 dbaOptions; /* TkSlaDbaOptions< SLA DBA option bits */
    U8 pollingLevel;    /* < DBA lcheduling level (set to 0 on TK3721) */
    U32 tdmRate;    /* < TDM rate (x 250 us) */
    U32 tdmGrantLength; /* < TDM grant length (bytes) */
    } PACK TkSlaDbaParams;

typedef struct TkLinkSlaDbaInfo
    {
    TkSlaQueueParams sla;    /* < SLA parameters */
    TkSlaDbaParams   dba;    /* < DBA parameters */
    } PACK TkLinkSlaDbaInfo;

int TkOLT_GetLinkSLA(short int olt_id, short int llid, TkLinkSlaDbaInfo *sla);
int TkOLT_SetLinkSLA(short int olt_id, short int llid, TkLinkSlaDbaInfo *sla);


typedef struct TkLinkOamRate
    {
    U8 maxRate; /* < Maximum number of OAM frames in units of PDUs/100ms */
    U8 minRate; /* < Number of 100ms periods between heartbeats */
    } PACK TkLinkOamRate;

int TkOLT_GetLinkOamRate(short int olt_id, short int llid, TkLinkOamRate *rate);
int TkOLT_SetLinkOamRate(short int olt_id, short int llid, TkLinkOamRate *rate);


typedef struct TkFecAbilities
    {
    U8 upstreamFecSupported;  /* < Is FEC supported upstream? */
    U8 downstreamFecSupported;    /* < Is FEC supported downstream? */
    } PACK TkFecAbilities;

typedef struct TkFecInfo
    {
    U8 upstreamFecEnabled;    /* < Is FEC enabled upstream? */
    U8 downstreamFecEnabled;  /* < Is FEC enabled downstream? */
    } PACK TkFecInfo;

int TkOLT_GetFECAbility(short int olt_id, TkFecAbilities *fec_abilities);
int TkOLT_GetPortFECMode(short int olt_id, short int port_id, TkFecInfo *fec_info);
int TkOLT_SetPortFECMode(short int olt_id, short int port_id, TkFecInfo *fec_info);

int TkOLT_GetLinkFECAbility(short int olt_id, short int llid, TkFecAbilities *fec_abilities);
int TkOLT_GetLinkFECMode(short int olt_id, short int llid, TkFecInfo *fec_info);
int TkOLT_SetLinkFECMode(short int olt_id, short int llid, TkFecInfo *fec_info);

int TkONU_GetFECAbility(short int olt_id, short int onu_id, TkFecAbilities *fec_abilities);
int TkONU_GetFECMode(short int olt_id, short int onu_id, TkFecInfo *fec_info);
int TkONU_SetFECMode(short int olt_id, short int onu_id, TkFecInfo *fec_info);


int TkOLT_ResetAllCounter(short int olt_id);
int TkONU_ResetAllCounter(short int olt_id, short int onu_id);



/* 
 TkOltStrobeMode: Teknovus API structure

/// Possible strobe operation modes
 */
typedef enum TkOltStrobeMode
    {
    TkOltStrobeModeNormalOperation                          = 0,    /* < Normal operation. Strobing occurs only once during a grant slot. */
    TkOltStrobeModePeriodicStrobing                         = 1     /* < Periodic strobing during ranging.  During non-ranging slot, it operates in normal mode (default) */
    } TkOltStrobeMode;

/* 
 TkOltStrobePolarity: Teknovus API structure

/// Possible strobe polarities
 */
typedef enum TkOltStrobePolarity
    {
    TkOltStrobePolarityActiveHigh                           = 0,    /* < Active high (default) */
    TkOltStrobePolarityActiveLow                            = 1     /* < Active low */
    } TkOltStrobePolarity;

typedef struct TkOltStrobeSingle
    {
    U24 offset; /* < Strobe offset */
    U8 len; /* < Strobe length */
    U8 mode;   /* TkOltStrobeMode< Strobe mode */
    U8 polarity;   /* TkOltStrobePolarity< Strobe polarity */
    } PACK TkOltStrobeSingle;

typedef struct TkOltEponStrobesParameters
    {
    TkOltStrobeSingle agc; /* < AGC strobe parameters */
    TkOltStrobeSingle cdr; /* < CDR strobe parameters */
    TkOltStrobeSingle firstReceive;    /* < First receive strobe parameters */
    TkOltStrobeSingle secondReceive;   /* < Second receive strobe parameters */
    U24 rangingPeriod;  /* < Ranging strobe period */
    U24 rangingOffset;  /* < Ranging strobe offset */
    U8 rangingPolarity;    /* TkOltStrobePolarity< Ranging strobe polarity */
    U24 unassignedGrantOffset;  /* < Unassigned grant strobe offset */
    U8 unassignedGrantPolarity;    /* TkOltStrobePolarity< Unassigned grant strobe polarity */
    } PACK TkOltEponStrobesParameters;

int TkOLT_GetPortOpticalParams(short int olt_id, short int port_id, TkOltEponStrobesParameters *optical_params);
int TkOLT_SetPortOpticalParams(short int olt_id, short int port_id, TkOltEponStrobesParameters *optical_params);


/* 
 TkOltOpticalHardwareType: Teknovus API structure

/// Optical hardware configuration and capabilities
 */
typedef enum TkOltOpticalHardwareType
    {
    TkOltOpticalHardwareTypeNone                            = 0,    /* < Optical monitoring not supported on this hardware */
    TkOltOpticalHardwareTypeSff8472                         = 1,    /* < SFF-8472 compiant using architecture */
    TkOltOpticalHardwareTypeAdc7992                         = 2,    /* < Rx power only from an ADC external to optics - architecture (2) */
    TkOltOpticalHardwareTypeAdc7992Hybrid                   = 3,    /* < ADC 7992 hybrid */
    TkOltOpticalHardwareTypeStrobeOnly                      = 4,    /* < Strobe only */
    TkOltOpticalHardwareTypeSffInf8077                      = 5     /* < SFF INF-8077 compilant */
    } TkOltOpticalHardwareType;

/* 
 TkOltI2cBus: Teknovus API structure

/// Possible locations for OLT I2C bus
 */
typedef enum TkOltI2cBus
    {
    TkOltI2cBusGpio0                                        = 0,
    TkOltI2cBusGpio1                                        = 1,
    TkOltI2cBusXfp                                          = 2,
    TkOltI2cBusSfp                                          = 3
    } TkOltI2cBus;

/* 
 TkOltOpticalMonitoringParams: Teknovus API structure

/// Configuration for OLT optical power monitoring feature
 */
typedef struct TkOltOpticalMonitoringParams
    {
    U8 hardwareType;  /* TkOltOpticalHardwareType< Optical hardware type */
    U8 bus;    /* TkOltI2cBus< Which I2C bus to use */
    U8 speed;   /* < I2C operating speed in kHz */
    U8 device;  /* < I2C device address (0xA2 for SFF-8472 compliant devices) */
    U8 reg; /* < Register address on the I2C device */
    U8 length;  /* < Length of the I2C device register (number of bytes to read) */
    float powerCorrections[5];  /* < Correction values for rx power calculation -- IEEE 754 single precision floating point numbers */
    } PACK TkOltOpticalMonitoringParams;

int TkOLT_GetPortOpticalMonitorConfig(short int olt_id, short int port_id, TkOltOpticalMonitoringParams *config);
int TkOLT_SetPortOpticalMonitorConfig(short int olt_id, short int port_id, TkOltOpticalMonitoringParams *config);

int TkAdapter_PAS5201_get_virtual_scope_adc_config(short int olt_id, PON_adc_config_t *adc_config);
int TkAdapter_PAS5201_set_virtual_scope_adc_config(short int olt_id, PON_adc_config_t *adc_config);


/* 
 TkOltOpticalRunMode: Teknovus API structure

/// Modes of operation for optical power monitoring feature
 */
typedef enum TkOltOpticalRunMode
    {
    TkOltOpticalRunModeOff                                  = 0,    /* < Suspends collection of rx power and idle power statistics */
    TkOltOpticalRunModeOn                                   = 1,    /* < Starts collection of rx power and idle stats over the entire range of links */
    TkOltOpticalRunModeNoIdle                               = 2,    /* < Stops collection of rx idle power - useful if you want to use the rx idle power for something else */
    TkOltOpticalRunModeIdle                                 = 3,    /* < Only rx idle power is monitored */
    TkOltOpticalRunModeSingle                               = 4,    /* < Collects rx power for a single link continuously */
    TkOltOpticalRunModeOneShot                              = 5     /* < Sample only once */
    } TkOltOpticalRunMode;

/* 
 TkOltOpticalDebug5Control: Teknovus API structure

/// Controls DEBUG 5 signal output during optical monitoring.
 */
typedef enum TkOltOpticalDebug5Control
    {
    TkOltOpticalDebug5ControlWaitForReportFrame             = 0,    /* < Wait until a REPORT frame is detected in the upstream then disable DEBUG 5 output. */
    TkOltOpticalDebug5ControlWaitForPollingGate             = 1,    /* < Wait long enough for a polling GATE to be issued then disable DEBUG 5 output. */
    TkOltOpticalDebug5ControlGenerateProcessorGate          = 2     /* < Generate processor GATE and output exactly one DEBUG 5 signal. */
    } TkOltOpticalDebug5Control;

typedef struct TkOltOpticalMonitoringControl
    {
    U8 newRunMode; /* TkOltOpticalRunMode< New run mode for collection of rx power stats */
    TkMacAddress linkMac;   /* < Link MAC Address -- Ignored if run mode is not 'single' */
    U32 startStrobe;    /* < Start strobe (1st strobe) */
    U32 endStrobe;  /* < End strobe (2nd strobe) */
    U8 debug5GenerationControl;  /* TkOltOpticalDebug5Control< Options for controlling DEBUG 5 signal output */
    U16 additionGateTQ; /* < Additional TQ to gate beyond 2nd stobe */
    } PACK TkOltOpticalMonitoringControl;

int TkOLT_GetPortOpticalMonitorControl(short int olt_id, short int port_id, TkOltOpticalMonitoringControl *config);
int TkOLT_SetPortOpticalMonitorControl(short int olt_id, short int port_id, TkOltOpticalMonitoringControl *config);

int TkAdapter_PAS5201_get_virtual_scope_measurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, void *configuration, void *result);  
int TkAdapter_PAS5201_get_virtual_scope_rssi_measurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result);
int TkAdapter_PAS5201_get_virtual_scope_onu_voltage(short int olt_id, short int onu_id, float *voltage, unsigned short int *sample, float *dbm);


int TkONU_GetI2cData(short int olt_id, short int onu_id, unsigned char i2cDevAddr, unsigned char internalDevAddrCount, unsigned char internalDevAddr[], unsigned short *numBytesToRead, unsigned char returnedData[]);
int TkONU_SetI2cData(short int olt_id, short int onu_id, unsigned char i2cDevAddr, unsigned short numBytesToWrite, unsigned char writedData[]);


int TkAdapter_PAS5201_set_classification_rule(short int olt_id, const PON_pon_network_traffic_direction_t direction, const PON_olt_classification_t classification_entity, const void *classification_parameter, const PON_olt_classifier_destination_t destination);


int TkBridgeGetP2PDataPathNum(short int olt_id, int *p2p_num);
int TkBridgeOpenP2PDataPath(short int olt_id);
int TkBridgeCloseP2PDataPath(short int olt_id);
int TkBridgeUpdateP2PDataPath(short int olt_id, int p2p_num);

int TkOnuSetP2PAccessList ( const short int olt_id, const short int onu_id, const unsigned short access_num, short int onu_ids[64], bool access, bool cover );
int TkOnuGetP2PAccessList ( const short int  olt_id, const short int onu_id, unsigned short *access_num, short int onu_ids[64] );


int TkBridgeGetOnuDataPathNum(short int olt_id, short int onu_id);
int TkBridgeSetOnuDataPathNum(short int olt_id, short int onu_id, int path_num);

int TkBridgeGetOltDataPathNum(short int olt_id);
int TkBridgeOpenMultiDataPath(short int olt_id);
int TkBridgeCloseMultiDataPath(short int olt_id);

/* llid frames's loose raw statistics */
typedef struct PON_llid_frames_loose_data_t
{
	unsigned long  llid_nomatch;				/* < Number of frames received whose LLID cannot be matched */
	unsigned long  llid_nogrant;				/* < Number of frames dropped because the LLID did not match the expected LLID */
} PON_llid_frames_loose_data_t;

int TkAdapter_PAS5201_get_raw_statistics 
                                 ( const PON_olt_id_t					 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   void								    *statistics_data,
								   PON_timestamp_t						*timestamp );

int TkAdapter_PAS5201_get_statistics
                             ( const PON_olt_id_t		olt_id, 
							   const short int			collector_id, 
							   const PON_statistics_t   statistics_type, 
							   const short int		    statistics_parameter, 
							   long double			   *statistics_data );


int TkAdapter_PAS5201_set_alarm_configuration 
                                    ( const PON_olt_id_t    olt_id, 
									  const short int	    source,
									  const PON_alarm_t     type,
									  const bool			activate,
									  const void		   *configuration );

int TkAdapter_PAS5201_get_alarm_configuration 
                                    ( const PON_olt_id_t  olt_id, 
									   const short int	  source,
									   const PON_alarm_t  type,
									   bool			      *activated,
									   void		          *configuration );


/* /////////////////////
 TkOltDiagnosticTestType: Teknovus API structure

/// Possible types of OLT diagnostic tests
///////////////////// */
typedef enum TkOltDiagnosticTestType
    {
    TkOltDiagnosticTestTypeProcessorSdramDataBus            = 16,   /* < Processor SDRAM data bus test */
    TkOltDiagnosticTestTypeProcessorSdramAddressBus         = 17,   /* < Processor SDRAM address bus test */
    TkOltDiagnosticTestTypeProcessorSdramFullWrite          = 18,   /* < Processor SDRAM full write test */
    TkOltDiagnosticTestTypeProcessorFlashDataBus            = 32,   /* < Processor flash data bus test */
    TkOltDiagnosticTestTypeProcessorFlashFullWrite          = 34,   /* < Processor flash full write test */
    TkOltDiagnosticTestTypeProcessorFlashCrcValidation      = 35,   /* < Processor flash CRC validation test */
    TkOltDiagnosticTestTypePacketBufferDdr2InternalLoopback = 48    /* < Packet buffer DDR2 internal loopback test */
    } TkOltDiagnosticTestType;

int TkOLT_SelfTest(short int olt_id, TkOltDiagnosticTestType test_type, unsigned char *test_error);


/* 
 TkLoopbackLocation: Teknovus API structure

/// Loopback locations
 */
typedef enum TkLoopbackLocation
    {
    TkLoopbackLocationMac                   = 1,    /* < MAC level */
    TkLoopbackLocationPhy                   = 2     /* < PHY level */
    } TkLoopbackLocation;

/* 
 TkLoopbackDirection: Teknovus API structure

/// Possible loopback traffic directions
 */
typedef enum TkLoopbackDirection
    {
    TkLoopbackDirectionUpstreamToDownstream = 1,    /* < From downstream to upstream */
    TkLoopbackDirectionDownstreamToUpstream = 2,    /* < From upstream to downstream */
    TkLoopbackDirectionCrossNni             = 3     /* < From the cross-chip NNI to this one */
    } TkLoopbackDirection;

int TkOLT_EnablePortLoopback(short int olt_id, short int port_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);
int TkOLT_DisablePortLoopback(short int olt_id, short int port_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);

int TkOLT_EnableLinkLoopback(short int olt_id, short int llid, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);
int TkOLT_DisableLinkLoopback(short int olt_id, short int llid, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);

int TkONU_EnableLinkLoopback(short int olt_id, short int onu_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);
int TkONU_DisableLinkLoopback(short int olt_id, short int onu_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);

int TkONU_EnablePortLoopback(short int olt_id, short int onu_id, short int port_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);
int TkONU_DisablePortLoopback(short int olt_id, short int onu_id, short int port_id, TkLoopbackLocation loopback_type, TkLoopbackDirection loopback_dir);

/* 
 TkLoopbackPayloadType: Teknovus API structure

/// Types of loopback payload
 */
typedef enum TkLoopbackPayloadType
    {
    TkLoopbackPayloadTypeIncrementing                   = 0,    /* < Incrementing */
    TkLoopbackPayloadTypeRandom                         = 1,    /* < Random */
    TkLoopbackPayloadTypeAllOnes                        = 2,    /* < All 1s (all Fs) */
    TkLoopbackPayloadTypeAllZeroes                      = 3     /* < All 0s */
    } TkLoopbackPayloadType;

/* 
 TkLoopbackOption: Teknovus API structure

/// Loopback option bits
 */
typedef enum TkLoopbackOption
    {
    TkLoopbackOptionNone                                = 0,
    TkLoopbackOptionRestoreDynamicMacs                  = 1     /* < Restore dynamically learned MAC addresses when finished */
    } TkLoopbackOption;

/* 
 TkLoopbackTestRequestParams: Teknovus API structure

/// Loopback test request parameters.
 */
typedef struct TkLoopbackTestRequestParams
    {
    U8  location;  /* TkLoopbackLocation< added by liwei056@2013-10-10 */
    U16 numFrames;  /* < Number of frames to send */
    U16 payloadLen; /* < Length of the frame payload (in bytes) */
    U8  payloadType;  /* TkLoopbackPayloadType< Type of the frame payload */
    U16 vid;    /* < VLAN ID of frame VLAN tag (0 for untagged frames) */
    U16 options;   /* TkLoopbackOption< Options bitmap */
    } PACK TkLoopbackTestRequestParams;

/* 
 TkLoopbackTestResult: Teknovus API structure

/// Returned loopback test result
 */
typedef struct TkLoopbackTestResult
    {
    U8  location;  /* TkLoopbackLocation< added by liwei056@2013-10-10 */
    U16 framesSent; /* < Number of frames sent */
    U16 framesReceived; /* < Number of frames received OK */
    U16 framesCorrupted;    /* < Number of frames received corrupted */
    U16 minDelay;   /* < Minimum frame delay (in us) */
    U16 maxDelay;   /* < Maximum frame delay (in us) */
    U16 averageDelay;   /* < Average frame delay (in us) */
    } PACK TkLoopbackTestResult;

int TkOLT_LinkLoopbackTest(short int olt_id, short int llid, TkLoopbackTestRequestParams *test_params, TkLoopbackTestResult *test_results);
int TkONU_LinkLoopbackTest(short int olt_id, short int onu_id, TkLoopbackTestRequestParams *test_params, TkLoopbackTestResult *test_results);
int TkONU_PortLoopbackTest(short int olt_id, short int onu_id, short int port_id, TkLoopbackTestRequestParams *test_params, TkLoopbackTestResult *test_results);


#if 1
/* ------------------------------------- CTC -----------------------------------*/

/* CTC Stack specific handler functions enum */
typedef enum
{
    TK_CTC_HANDLER_UPDATE_ENCRYPTION_KEY,		/* Update encryption event					 */
    TK_CTC_HANDLER_END_OAM_DISCOVERY,			/* End of extended OAM discovery process	 */
    TK_CTC_HANDLER_UPDATE_ONU_FIRMWARE_ERROR,	/* Update ONU Firmware error event           */
    TK_CTC_HANDLER_ONU_FIRMWARE_REQUEST,			/* ONU requests to update its firmware event */ 
    TK_CTC_HANDLER_AUTH_RESPONSE_LOID,          	/* Authentication response ONU_ID+Password	 */
    TK_CTC_HANDLER_AUTHORIZED_LOID,              /* Authorized ONU_ID+Password           	 */
    TK_CTC_HANDLER_EVENT_NOTIFICATION,           /* Organization Specific Event Notification  */
    TK_CTC_HANDLER_ONU_AUTHORIZE_TO_THE_NETWORK, /* ONU is authorized to the network          */
    TK_CTC_HANDLER_ENCRYPTION_KEY_VALUE,	     	/* Update encryption event + the key value   */
    TK_CTC_HANDLER_UPDATE_FEC_MODE,              /* Update fec mode event                     */

    TK_CTC_HANDLER_LAST_HANDLER,

} TK_CTC_handler_index_t;

int TkCTC_assign_handler_function
                                     ( const TK_CTC_handler_index_t           handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );
int TkCTC_delete_handler_function( const unsigned short  handler_id );


int TkGetOnuCtcVersion(short int olt_id, short int onu_id, unsigned char *version);

/* /////////////////////
 TkCtcOnuFirmwareVersionParams: Teknovus API structure

/// Parameters for CTC "get ONU firmware version" TLV
///////////////////// */
typedef struct TkCtcOnuFirmwareVersionParams
    {
    U8 versionCount;    /* < Number of elements in Version */
    U8 version[256];    /* < Firmware version (ASCII) */
    } PACK TkCtcOnuFirmwareVersionParams;

int TkCTC_GetOnuFirmwareVersion(short int olt_id, short int onu_id, TkCtcOnuFirmwareVersionParams *version);

/* /////////////////////
 TkCtcChipsetIdTlv: Teknovus API structure

/// Ctc Chipset Id Tlv
///////////////////// */
typedef struct TkCtcChipsetIdTlv
    {
    U16 vendorId;   /* < Vendor Id */
    U16 chipModel;  /* < Chip Model */
    U8 revision;    /* < Revsion */
    U8 designDate[3];   /* < Design Date */
    } PACK TkCtcChipsetIdTlv;

int TkCTC_GetOnuChipInfo(short int olt_id, short int onu_id, TkCtcChipsetIdTlv *chip_info);

/* /////////////////////
 TkCtcOnuSerialNumberParams: Teknovus API structure

/// Parameters for CTC "get ONU serial number" TLV
///////////////////// */
typedef struct TkCtcOnuSerialNumberParams
    {
    U8 vendorId[4]; /* < Vendor ID */
    U32 onuModel;   /* < ONU Model */
    TkMacAddress onuMac;    /* < ONU MAC address */
    U8 hwVersion[8];    /* < Hardware version (ASCII) */
    U8 swVersion[16];   /* < Software version (ASCII) */
    U8  extendedModel[16];/*add for CTC V3.0 by yangzl@2016-4-18*/
    } PACK TkCtcOnuSerialNumberParams;

int TkCTC_GetOnuSerialNumber(short int olt_id, short int onu_id, TkCtcOnuSerialNumberParams *serial_number);


/* /////////////////////
 TkCtc21OnuInterface: Teknovus API structure

/// CTC 2.1 ONU Interface
///////////////////// */
typedef struct TkCtc21OnuInterface
    {
    U32 type;   /* TkCtc21OnuInterfaceType< CTC ONU Interface Type */
    U16 numPorts;   /* < Number of Ports */
    } PACK TkCtc21OnuInterface;

/* /////////////////////
 TkCtcOnuCapabilities21: Teknovus API structure

/// Ctc Onu Capabilities2 Params
///////////////////// */
#define TK_CTC_STACK_CAPABILITY_MAX_INTERFACE_TYPE_NUM    9
typedef struct TkCtcOnuCapabilities21
    {
    U32 onuType;   /* TkCtcOnuType< CTC ONU type */
    U8 numLinks;    /* < Number of links the ONU supports */
    U8 protectionType;  /* < CTC ONU protection type */
    U8 numPonIFs;   /* < Number of PON interfaces */
    U8 numSlots;    /* < Number of slots */
    U8 interfacesCount; /* < Number of elements in Interfaces */
    TkCtc21OnuInterface interfaces[TK_CTC_STACK_CAPABILITY_MAX_INTERFACE_TYPE_NUM];   /* < ONU interfaces */
    U8 batteryBackup; /* < Whether or not the ONU features battery backup */
    } PACK TkCtcOnuCapabilities21;

/* /////////////////////
 TkCtcOnuCapsTlv: Teknovus API structure

/// Parameters for CTC "get ONU capabilities" TLV
///////////////////// */
typedef struct TkCtcOnuCapsTlv
    {
    U8 services; /* < Services supported by this ONU */
    U8 numGePorts;  /* < Number of gigabit ethernet ports */
    U64 gePortMap;  /* < Bitmap of ports that are gigabit ethernet ports */
    U8 numFePorts;  /* < Number of fast ethernet ports */
    U64 fePortMap;  /* < Bitmap of ports that are fast ethernet ports */
    U8 numPotsPorts;    /* < Number of plain-old-telephone-service ports */
    U8 numE1Ports;  /* < Number of E1 (TDM) ports */
    U8 numUpstreamQueues;   /* < Number of upstream queues */
    U8 numQueuesPerPortUpMax;   /* < Max number of queues per port upstream */
    U8 numDownstreamQueues; /* < Number of downstream queues */
    U8 numQueuesPerPortDnMax;   /* < Max number of queues per port downstream */
    U8 batteryBackup;   /* < Supports battery backup */
    } PACK TkCtcOnuCapsTlv;

typedef struct TkCtcOnuCapabilities30
    {
    U8 ipv6; /* < ipv6 supported by this ONU */
    U8 power_ctrl; /* < onu's Tx/Rx power supply control supported by this ONU */
    U8 service_sla; /* < service SLA supported by this ONU */
    } PACK TkCtcOnuCapabilities30;

int TkCTC_GetOnuCap1(short int olt_id, short int onu_id, TkCtcOnuCapsTlv *capabilities);
int TkCTC_GetOnuCap2(short int olt_id, short int onu_id, TkCtcOnuCapabilities21  *capabilities);
int TkCTC_GetOnuCap3(short int olt_id, short int onu_id, TkCtcOnuCapabilities30 *capabilities);

int TkUpgradeCtcOnuFirmware(short int olt_id, short int onu_id, void *firmware_location, int firmware_size, char *file_name);
int TkCTC_ActiveOnuImage(short int olt_id, short int onu_id);
int TkCTC_CommitOnuImage(short int olt_id, short int onu_id);

/* /////////////////////
 TkCtcOnuAuthenticationType: Teknovus API structure

/// Possible ONU CTC authentication types
///////////////////// */
typedef enum TkCtcOnuAuthenticationType
    {
    TkCtcOnuAuthenticationTypeLoidPassword                          = 1,
    TkCtcOnuAuthenticationTypeNak                                   = 2
    } TkCtcOnuAuthenticationType;

/* /////////////////////
 TkCtcOnuAuthenticationResultCode: Teknovus API structure

/// Possible ONU CTC authentication results
///////////////////// */
typedef enum TkCtcOnuAuthenticationResultCode
    {
    TkCtcOnuAuthenticationResultCodeSuccess                         = 0,
    TkCtcOnuAuthenticationResultCodeLoidDoesNotExist                = 1,
    TkCtcOnuAuthenticationResultCodeWrongPassword                   = 2,
    TkCtcOnuAuthenticationResultCodeLoidConflict                    = 3
    } TkCtcOnuAuthenticationResultCode;

typedef struct TkCtcOnuAuthRequestInfo
    {
    U8 type;    /* TkCtcOnuAuthenticationType< The type of authentication to use */
    union
        {
        struct
            {
            U8 loid[24];    /* < LOID */
            U8 password[12];    /* < Password */
            } LoidPassword;
        struct
            {
            U8 desiredAuthenticationType;   /* TkCtcOnuAuthenticationType< Desired Authentication Type */
            } Nak;
        } x;
    } PACK TkCtcOnuAuthRequestInfo;

int TkCTC_GetOnuAuthInfo(short int olt_id, short int onu_id, unsigned char auth_type, TkCtcOnuAuthRequestInfo *auth_info);
int TkCTC_SetOnuAuthResult(short int olt_id, short int onu_id, unsigned char auth_result);

typedef struct TkCtcDbaQueueSet
    {
    U8 reported;    /* < Bitmap of which of the following thresholds are to be used */
    U16 thresholds[8];  /* < Reporting thresholds 0-7 */
    } PACK TkCtcDbaQueueSet;

#define CTC_STACK_DBA_QUEUESET_MAX_NUM    4
typedef struct TkCtcDbaInfo
    {
    U8 qSetsCount;  /* < Number of elements in Q Sets */
    TkCtcDbaQueueSet qSets[CTC_STACK_DBA_QUEUESET_MAX_NUM];   /* < DBA queue sets */
    } PACK TkCtcDbaInfo;


int TkCTC_GetDBA_ReportThresholds(short int olt_id, short int onu_id, TkCtcDbaInfo *dba_info);
int TkCTC_SetDBA_ReportThresholds(short int olt_id, short int onu_id, TkCtcDbaInfo *dba_info);


/* 
 TkCtcOnuPortLabelBase: Teknovus API structure

/// CTC ONU Port Label Base
 */
typedef struct TkCtcOnuPortLabelBase
    {
    TkMacAddress targetMac; /* < Target ONU Mac Address */
    U8 version;   /* TkCtcVersionInfo< Ctc Version */
    union
        {
        struct
            {
            U8 label;  /* TkCtc21OnuInstanceLabel< CTC ONU Instance Label */
            U8 type;    /* TkCtc21OnuPortType< CTC ONU Port Type */
            U8 options; /* < CTC ONU Port Label Options */
            U16 portNumber; /* < CTC ONU Port Number */
            } PACK Ctc21;
        struct
            {
            U8 port;    /* < Port */
            } PACK Ctc20;
        struct
            {
            U8 port;    /* < Port */
            U32 reserved;
            } PACK Ctc20Extended;
        } x;
    } PACK TkCtcOnuPortLabelBase;

int TkCTC_GetTkObjFromCtcObj(short int olt_id, short int onu_id, CTC_management_object_t *ctc_src_obj, TkCtcOnuPortLabelBase *tk_dst_obj);
int TkCTC_GetCtcObjFromTkObj(TkCtcOnuPortLabelBase *tk_src_obj, CTC_management_object_t *ctc_dst_obj);

int TkCTC_GetObjAlarmState(short int olt_id, short int onu_id, TkCtcOnuPortLabelBase *ctc_obj, unsigned short alarm_id, int *state);
int TkCTC_SetObjAlarmState(short int olt_id, short int onu_id, TkCtcOnuPortLabelBase *ctc_obj, unsigned short alarm_id, int state);

int TkCTC_GetObjAlarmThreshold(short int olt_id, short int onu_id, TkCtcOnuPortLabelBase *ctc_obj, unsigned short alarm_id, unsigned long alarm_threshold[2]);
int TkCTC_SetObjAlarmThreshold(short int olt_id, short int onu_id, TkCtcOnuPortLabelBase *ctc_obj, unsigned short alarm_id, unsigned long alarm_threshold[2]);


typedef enum TkCtcOnuObjectType
    {
    TkCtcOnuObjectTypePort                                              = 1,        /* < Ethernet, VoIP, ADSL/ADSL2+, VDSL2+, E1 */
    TkCtcOnuObjectTypeCard                                              = 2,        /* < Card */
    TkCtcOnuObjectTypeLlid                                              = 3,        /* < Llid */
    TkCtcOnuObjectTypePonIf                                             = 4,        /* < PON IF */
    TkCtcOnuObjectTypeOnu                                               = 65535U    /* < ONU */
    } TkCtcOnuObjectType;

typedef struct TkCtc30EventEntry
    {
    U16 objectType;  /* TkCtcOnuObjectType< Object Type */
    U32 objectInstance; /* < Object Instance */
    U16 alarmID;    /* TkCtcOnuAlarmID< Alarm ID */
    } PACK TkCtc30EventEntry;

typedef struct TkCtc30EventStatusEntry
    {
    TkCtc30EventEntry eventEntry;  /* < Event Entry */
    U32 eventStatus; /* TkCtc30EventStatus< Event Status */
    } PACK TkCtc30EventStatusEntry;

int TkCTC_GetOnuAlarmState(short int olt_id, short int onu_id, unsigned short *alarm_num, TkCtc30EventStatusEntry alarm_states[]);
int TkCTC_SetOnuAlarmState(short int olt_id, short int onu_id, unsigned short alarm_num, TkCtc30EventStatusEntry alarm_states[]);

typedef struct TkCtc30EventThresholdEntry
    {
    TkCtc30EventEntry eventEntry;  /* < Event Entry */
    U64 setThreshold;   /* < Set Threshold */
    U64 clearThreshold; /* < Clear Threshold */
    } PACK TkCtc30EventThresholdEntry;

int TkCTC_GetOnuAlarmThreshold(short int olt_id, short int onu_id, unsigned short *alarm_num, TkCtc30EventThresholdEntry alarm_thresholds[]);
int TkCTC_SetOnuAlarmThreshold(short int olt_id, short int onu_id, unsigned short alarm_num, TkCtc30EventThresholdEntry alarm_thresholds[]);


typedef struct TkOnuHoldoverParams
    {
    U16 holdoverInterval;   /* < Holdover interval in 1ms units */
    U8 holdoverFlags;  /* TkHoldoverFlags< Holdover Mode */
    } PACK TkOnuHoldoverParams;

int TkCTC_GetPonPortHoldover(short int olt_id, short int onu_id, short int port_id, TkOnuHoldoverParams *hold_info);
int TkCTC_SetPonPortHoldover(short int olt_id, short int onu_id, short int port_id, TkOnuHoldoverParams *hold_info);


int TkCTC_EnablePort(short int olt_id, short int onu_id, short int port_id);
int TkCTC_DisablePort(short int olt_id, short int onu_id, short int port_id);


#define TK_MAX_TECHNOLOGY_ABILITY 20
typedef struct TkCtcEthernetPortInfo
    {
    U8 linked;    /* < Link UP or DOWN (true implies UP) */
    U8 flowControlEnabled;    /* < Flow control enabled */
    U8 phyEnabled;    /* < PHY enabled */
    U8 autoNegoEnabled;   /* < Auto-negotiation enabled */
    U8 abilitiesCount;  /* < Number of elements in Abilities */
    U16 abilities[TK_MAX_TECHNOLOGY_ABILITY];    /* TkCtcOnuAutoCaps< Local auto-negotiation abilities */
    U8 advertisedCount; /* < Number of elements in Advertised */
    U16 advertised[TK_MAX_TECHNOLOGY_ABILITY];   /* TkCtcOnuAutoCaps< Advertised auto-negotiation abilities */
    } PACK TkCtcEthernetPortInfo;

int TkCTC_GetEthPortInfo(short int olt_id, short int onu_id, short int port_id, TkCtcEthernetPortInfo *port_info);

typedef struct TkCtcEthernetPortCtrl
    {
    U8 flowControlEnable; /* < Flow control enable */
    U8 autoNegoEnable;    /* < Auto-negotiation enable */
    U8 restartAutoNego;   /* < Restart auto-negotiation */
    U8 phyEnable; /* < PHY enable */
    } PACK TkCtcEthernetPortCtrl;

int TkCTC_SetEthPort(short int olt_id, short int onu_id, short int port_id, TkCtcEthernetPortCtrl *port_ctrl);


typedef struct TkCtcPolicingParam
    {
    U8 policingEnabled;   /* < Port policing enabled */
    U24 cir;    /* < Committed information rate in Kbps */
    U24 policeBucket;   /* < Depth of this token bucket */
    U24 extraBucket;    /* < Extra burst size */
    } PACK TkCtcPolicingParam;

int TkCTC_GetEthPortPolicing(short int olt_id, short int onu_id, short int port_id, TkCtcPolicingParam *police_info);
int TkCTC_SetEthPortPolicing(short int olt_id, short int onu_id, short int port_id, TkCtcPolicingParam *police_info);

typedef struct TkCtcDownstreamRateLimitingParam
    {
    U8 enabled;   /* < Whether or not downstream rate limiting is enabled on this port */
    U24 cir;    /* < The committed information rate for this port in Kbps */
    U24 pir;    /* < The peak information rate for this port in Kbps */
    } PACK TkCtcDownstreamRateLimitingParam;

int TkCTC_GetEthPortDownRateLimit(short int olt_id, short int onu_id, short int port_id, TkCtcDownstreamRateLimitingParam *rate_info);
int TkCTC_SetEthPortDownRateLimit(short int olt_id, short int onu_id, short int port_id, TkCtcDownstreamRateLimitingParam *rate_info);


int TkAdapter_PASCTC_get_ethport_vlan_config(short int olt_id, short int onu_id, short int port_id, CTC_STACK_port_vlan_configuration_t *vlan_conf);
int TkAdapter_PASCTC_set_ethport_vlan_config(short int olt_id, short int onu_id, short int port_id, CTC_STACK_port_vlan_configuration_t *vlan_conf);
int TkAdapter_PASCTC_get_all_ethport_vlan_config(short int olt_id, short int onu_id, unsigned char *port_num, CTC_STACK_vlan_configuration_ports_t ports_vlan);


int TkAdapter_PASCTC_get_port_class_config(short int olt_id, short int onu_id, short int port_id, CTC_STACK_classification_rules_t class_cfg);
int TkAdapter_PASCTC_set_port_class_config(short int olt_id, short int onu_id, short int port_id, CTC_STACK_classification_rule_mode_t op_mode, CTC_STACK_classification_rules_t class_cfg);
int TkCTC_ClearPortClass(short int olt_id, short int onu_id, short int port_id);


int TkCTC_GetPortMulticastVlan(short int olt_id, short int onu_id, short int port_id, unsigned short *vlan_nums, unsigned short vlan_ids[]);
int TkCTC_AddPortMulticastVlan(short int olt_id, short int onu_id, short int port_id, unsigned short vlan_nums, unsigned short vlan_ids[]);
int TkCTC_DeletePortMulticastVlan(short int olt_id, short int onu_id, short int port_id, unsigned short vlan_nums, unsigned short vlan_ids[]);
int TkCTC_ClearPortMulticastVlan(short int olt_id, short int onu_id, short int port_id);

int TkAdapter_PASCTC_get_multicast_control(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc);
int TkAdapter_PASCTC_set_multicast_control(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc);
int TkCTC_ClearOnuMulticastGroups(short int olt_id, short int onu_id);

int TkCTC_GetPortMulticastGroupNumberMax(short int olt_id, short int onu_id, short int port_id, unsigned char *group_num);
int TkCTC_SetPortMulticastGroupNumberMax(short int olt_id, short int onu_id, short int port_id, unsigned char group_num);

int TkCTC_GetPortMulticastTagStrip(short int olt_id, short int onu_id, short int port_id, unsigned char *strip_mode, unsigned char *switch_nums, CTC_STACK_multicast_vlan_switching_entry_t switch_entries[]);
int TkCTC_SetPortMulticastTagStrip(short int olt_id, short int onu_id, short int port_id, unsigned char strip_mode, unsigned char switch_nums, CTC_STACK_multicast_vlan_switching_entry_t switch_entries[]);

typedef enum TkCtcOnuMulticastSwitchingMode
    {
    TkCtcOnuMulticastSwitchingModeIgmpMldSnooping           = 0,    /* < IGMP/MLD Snooping dual stack mode */
    TkCtcOnuMulticastSwitchingModeCtcControllableIgmpMld    = 1,    /* < CTC controllable IGMP/MLD mode, snooping disabled */
    TkCtcOnuMulticastSwitchingModeIgmpSnooping              = 2,    /* < IGMP snooping mode */
    TkCtcOnuMulticastSwitchingModeCtcControllableIgmp       = 3,    /* < CTC controllable IGMP mode, snooping disabled */
    TkCtcOnuMulticastSwitchingModePassThrough               = 127   /* < Pass through mode (all IGMP/IPMC/MLD/IPv6 Multicast traffic is passed through) */
    } TkCtcOnuMulticastSwitchingMode;

int TkCTC_GetOnuMulticastSwitchMode(short int olt_id, short int onu_id, unsigned char *switch_mode);
int TkCTC_SetOnuMulticastSwitchMode(short int olt_id, short int onu_id, unsigned char switch_mode);

typedef enum TkCtcFastLeaveAbilityBitmap
    {
    TkCtcFastLeaveAbilityBitmapNone                         = 0,
    TkCtcFastLeaveAbilityBitmapNonFastLeaveInSnooping       = 1,    /* < Support Non-Fast-Leave when in IGMP snooping mode */
    TkCtcFastLeaveAbilityBitmapFastLeaveInSnooping          = 2,    /* < Support Fast-Leave when in IGMP snooping mode */
    TkCtcFastLeaveAbilityBitmapNonFastLeaveInCtcControlled  = 4,    /* < Support Non-Fast-Leave when in CTC Controlled IGMP mode */
    TkCtcFastLeaveAbilityBitmapFastLeaveInCtcControlled     = 8,    /* < Support Fast-Leave when in CTC Controlled IGMP Mode */
    TkCtcFastLeaveAbilityBitmapNonFastLeaveInMldSnooping    = 16,   /* < Support Non-Fast-Leave when In MLD snooping mode */
    TkCtcFastLeaveAbilityBitmapFastLeaveInMldSnooping       = 32    /* < Support Fast-Leave When in MLD snooping mode */
    } TkCtcFastLeaveAbilityBitmap;

int TkCTC_GetOnuFastLeaveAbility(short int olt_id, short int onu_id, unsigned char *abilitiy_bitmap);
int TkCTC_GetOnuFastLeaveState(short int olt_id, short int onu_id, unsigned char *state);
int TkCTC_SetOnuFastLeaveState(short int olt_id, short int onu_id, unsigned char state);


int TkCTC_GetPortMonitorState(short int olt_id, short int onu_id, short int port_id, unsigned short *state, unsigned long *period);
int TkCTC_SetPortMonitorState(short int olt_id, short int onu_id, short int port_id, unsigned short state, unsigned long period);
int TkAdapter_PASCTC_get_ethport_statistic_data(short int olt_id, short int onu_id, short int port_id, CTC_STACK_statistic_data_t *data);


typedef struct TkCtc21IadInformation
    {
    TkMacAddress iadMac;    /* < IAD MAC Address */
    U8 protocolSupport;    /* TkCtc21VoipProtocol< Protocol Support */
    U8 iadSoftwareVer[32];  /* < IAD Software version */
    U8 iadSoftwareTime[32]; /* < IAD software time, version time format uses:YYYYMMDDHHMMSS */
    U8 voipUserCount;   /* < Indicate POTS number of IAD module */
    } PACK TkCtc21IadInformation;

int TkCTC_GetVoipIadInfo(short int olt_id, short int onu_id, TkCtc21IadInformation *iad_info);

typedef enum TkCtc21IadOperation
    {
    TkCtc21IadOperationReRegister                                   = 0,    /* < Re-Register for softswitch platform */
    TkCtc21IadOperationLogout                                       = 1,    /* < Logout from softswitch platform */
    TkCtc21IadOperationReset                                        = 2     /* < Reset, only for voice module */
    } TkCtc21IadOperation;

int TkCTC_SetVoipIadOperation(short int olt_id, short int onu_id, TkCtc21IadOperation operation);

typedef struct TkCtc21VoipGlobalParameters
    {
    U8 voiceIPMode; /* TkCtc21VoiceIPMode< CTC Voice IP Mode */
    U32 iadIPAddr;    /* < IAD IP Address */
    U32 iadNetMask;   /* < IAD Net Mask Address */
    U32 iadDefaultGW; /* < IAD Default Gateway */
    U8 pppoeMode;   /* TkCtcPppoeMode< PPPoE Mode */
    U8 pppoeUsername[32];   /* < PPPoE Username */
    U8 pppoePassword[32];   /* < PPPoE Password */
    U8 taggingMode;    /* TkCtc21VoiceTaggingMode< Tagging Mode */
    U16 voiceCvlan; /* < Voice CVLAN */
    U16 voiceSvlan; /* < Voice SVLAN */
    U8 voicePri;    /* < Voice Priority */
    } PACK TkCtc21VoipGlobalParameters;

int TkCTC_GetVoipGlobalParams(short int olt_id, short int onu_id, TkCtc21VoipGlobalParameters *params);
int TkCTC_SetVoipGlobalParams(short int olt_id, short int onu_id, TkCtc21VoipGlobalParameters *params);

typedef struct TkCtc21FaxModemConfiguration
    {
    U8 voiceT38Enable; /* TkCtc21VoiceT38Mode< Ctc Voice T38 Enable Mode */
    U8 voiceFaxModemControl;   /* TkCtc21VoiceFaxModemControlMode< CTC Voice Fax Modem Control Mode */
    } PACK TkCtc21FaxModemConfiguration;

int TkCTC_GetFoipGlobalParams(short int olt_id, short int onu_id, TkCtc21FaxModemConfiguration *params);
int TkCTC_SetFoipGlobalParams(short int olt_id, short int onu_id, TkCtc21FaxModemConfiguration *params);

typedef enum TkCtc21IadPortStatus
    {
    TkCtc21IadPortStatusRegistring                                  = 0,    /* < Port is registring */
    TkCtc21IadPortStatusIdle                                        = 1,    /* < Port is idle */
    TkCtc21IadPortStatusPickup                                      = 2,    /* < Pick up */
    TkCtc21IadPortStatusDialing                                     = 3,    /* < Dialing */
    TkCtc21IadPortStatusRinging                                     = 4,    /* < Ringing */
    TkCtc21IadPortStatusRingback                                    = 5,    /* < Ringing back */
    TkCtc21IadPortStatusConnecting                                  = 6,    /* < Connecting */
    TkCtc21IadPortStatusConnected                                   = 7,    /* < Connected */
    TkCtc21IadPortStatusReleasingConnection                         = 8,    /* < Releasing Connection */
    TkCtc21IadPortStatusRegistrationFailure                         = 9,    /* < Port Registration Failure */
    TkCtc21IadPortStatusNotActivated                                = 10    /* < Port is not activated */
    } TkCtc21IadPortStatus;

typedef enum TkCtc21IadPortServiceState
    {
    TkCtc21IadPortServiceStateEndLocal                              = 0,    /* < Local end terminates service, caused by 'user disable port' */
    TkCtc21IadPortServiceStateEndRemote                             = 1,    /* < Remote and terminates service, caused by 'MGC sends down command' */
    TkCtc21IadPortServiceStateEndAuto                               = 2,    /* < Automatically terminate service, caused by MGC fault */
    TkCtc21IadPortServiceStateNormal                                = 3     /* < Normal service normal */
    } TkCtc21IadPortServiceState;

typedef enum TkCtc21IadPortCodecMode
    {
    TkCtc21IadPortCodecModeG711a                                    = 0,    /* < G.711 A */
    TkCtc21IadPortCodecModeG729                                     = 1,    /* < G.729 */
    TkCtc21IadPortCodecModeG711u                                    = 2,    /* < G.711 U */
    TkCtc21IadPortCodecModeG723                                     = 3,    /* < G.723 */
    TkCtc21IadPortCodecModeG726                                     = 4,    /* < G.726 */
    TkCtc21IadPortCodecModeT38                                      = 5     /* < T.38 */
    } TkCtc21IadPortCodecMode;

typedef struct TkCtc21PotsStatus
    {
    TkCtc21IadPortStatus iadPortStatus; /* < CTC IAD Port Status */
    TkCtc21IadPortServiceState iadPortServiceState; /* < CTC IAD Port Service State */
    TkCtc21IadPortCodecMode iadPortCodecMode;   /* < CTC IAD Port Codec Mode */
    } TkCtc21PotsStatus;

int TkCTC_GetVoipPortStatus(short int olt_id, short int onu_id, short int port_id, TkCtc21PotsStatus *status);
int TkCTC_GetVoipPortState(short int olt_id, short int onu_id, short int port_id, bool *state);


typedef struct TkCtc21H248Parameters
    {
    U16 mGPortNumber;   /* < MG Port Number */
    TkIPv4Address mgcIP;    /* < Activate softswitch platform IP address */
    U16 mgcComPort; /* < Activate softswitch platform COM port number */
    TkIPv4Address backupMgcIP;  /* < Backup softswitch platform IP address. If it is 0x00000000, dual homing is not enable */
    U16 backupMgcComPort;   /* < Backup softswitch platform COM port number, if it is 0, dual homing is not enabled */
    U8 useActiveMgc;  /* < Should we use the active softswitch platform instead of the backup? */
    U8 regMode; /* TkCtc21H248RegMode< H248 Reg Mode */
    U8 mid[64]; /* < MID */
    U8 heartbeatMode; /* TkCtc21H248HeartbeatMode< Heartbeat Mode */
    U16 heartbeatCycle; /* < Heartbeat Cycle */
    U8 heartbeatCount;  /* < Heartbeat Count */
    } PACK TkCtc21H248Parameters;

int TkCTC_GetVoipH248Params(short int olt_id, short int onu_id, TkCtc21H248Parameters *params);
int TkCTC_SetVoipH248Params(short int olt_id, short int onu_id, TkCtc21H248Parameters *params);

typedef enum TkCtc21IadOperationStatus
    {
    TkCtc21IadOperationStatusRegistering                            = 0,    /* < Registering */
    TkCtc21IadOperationStatusRegSuccessful                          = 1,    /* < Registration Successful */
    TkCtc21IadOperationStatusIadFault                               = 2,    /* < IAD Fault */
    TkCtc21IadOperationStatusLogout                                 = 3,    /* < Logout */
    TkCtc21IadOperationStatusIadRestarting                          = 4     /* < IAD Restarting */
    } TkCtc21IadOperationStatus;

int TkCTC_GetVoipH248IadStatus(short int olt_id, short int onu_id, TkCtc21IadOperationStatus *status);

typedef struct TkCtc21H248RtpTidInformation
    {
    U8 numRtpTids;  /* < Number of RTP TIDs */
    U8 firstRtpTidName[32]; /* < Frist RTP TID Name */
    } PACK TkCtc21H248RtpTidInformation;

typedef struct TkCtc21H248RtpTidConfiguration
    {
    U8 numRtpTids;  /* < Number of RTP TIDs */
    U8 prefix[16];  /* < Prefix in ASCII */
    U64 digitBegin; /* < RTP TID digital portion start value */
    U8 rtpTidMode;   /* TkCtc21RtpTidMode< RTP TID Mode */
    U8 rtpTidLength;    /* < RTP TID Digit Length */
    } PACK TkCtc21H248RtpTidConfiguration;

int TkCTC_GetVoipH248RtpTid(short int olt_id, short int onu_id, TkCtc21H248RtpTidInformation *tid_info);
int TkCTC_SetVoipH248RtpTid(short int olt_id, short int onu_id, TkCtc21H248RtpTidConfiguration *tid_cfg);

int TkCTC_GetVoipPortH248Tid(short int olt_id, short int onu_id, short int port_id, unsigned char tid_name[32]);
int TkCTC_SetVoipPortH248Tid(short int olt_id, short int onu_id, short int port_id, unsigned char tid_name[32]);


typedef struct TkCtc21SipParameters
    {
    U16 mGPortNumber;   /* < MG Port Number */
    TkIPv4Address activeSipIP;  /* < Active SIP agent server IP address */
    U16 activeSipComPort;   /* < Active SIP agent server port number */
    TkIPv4Address backupSipIP;  /* < Backup SIP agent server IP address */
    U16 backupSipComPort;   /* < Backup SIP Proxy Server Com Port Number */
    TkIPv4Address activeSipProxyServer; /* < Active SIP Proxy Server */
    TkIPv4Address activeSipRegistrationServerIP;    /* < SIP Register Server IP */
    U16 activeSipRegistrationServerComPort; /* < Active SIP registration server port number */
    TkIPv4Address backupSipRegIP;   /* < Backup SIP Registration Server IP */
    U16 backupSipRegComPort;    /* < Backup SIP Registration Server Com Port Number */
    TkIPv4Address outboundServerIP; /* < Out Bound Server IP */
    U16 outboundServerPort; /* < Out Bound Server Port Number */
    U32 sipRegistrationInterval;    /* < Registration refresh cycle, unit is second, and the default value is 3600s */
    U8 disableHeartbeatSwitch;    /* < Disable Heartbeat Switch Feature */
    U16 heartbeatCycle; /* < Heartbeat Cycle */
    U16 heartbeatCount; /* < Heartbeat Count */
    } PACK TkCtc21SipParameters;

int TkCTC_GetVoipSipParams(short int olt_id, short int onu_id, TkCtc21SipParameters *params);
int TkCTC_SetVoipSipParams(short int olt_id, short int onu_id, TkCtc21SipParameters *params);
int TkCTC_SetVoipSipDigitMap(short int olt_id, short int onu_id, unsigned short map_len, unsigned char *map_str);

typedef struct TkCtc21SipUserParameters
    {
    U8 account[16]; /* < User account, User phone number, and should use ASCII code */
    U8 name[32];    /* < User name, SIP port username, and should use ASCII code */
    U8 password[16];    /* < User password, SIP port password, and should use ASCII code. */
    } TkCtc21SipUserParameters;

int TkCTC_GetVoipPortSipUserParams(short int olt_id, short int onu_id, short int port_id, TkCtc21SipUserParameters *params);
int TkCTC_SetVoipPortSipUserParams(short int olt_id, short int onu_id, short int port_id, TkCtc21SipUserParameters *params);


int TkCTC_GetOnuFECAbility(short int olt_id, short int onu_id, TkFecAbilities *fec_abilities);
int TkCTC_GetOnuFECMode(short int olt_id, short int onu_id, TkFecInfo *fec_info);
int TkCTC_SetOnuFECMode(short int olt_id, short int onu_id, TkFecInfo *fec_info);


typedef struct TkCtc21OnuMxuGlobalParameters
    {
    TkIPv4Address onuIPAddress; /* < Management ONU IP Address */
    TkIPv4Address onuNetworkMask;   /* < Management ONU Mask Address */
    TkIPv4Address onuDefaultGW; /* < Management ONU default Gateway */
    U16 dataCVlan;  /* < Management Data Customer VLAN */
    U16 dataSVlan;  /* < Management Data Service VLAN */
    U8 dataPriority;    /* < Management Data Priority */
    } PACK TkCtc21OnuMxuGlobalParameters;

int TkCTC_GetMxuGlobalParams(short int olt_id, short int onu_id, TkCtc21OnuMxuGlobalParameters *params);
int TkCTC_SetMxuGlobalParams(short int olt_id, short int onu_id, TkCtc21OnuMxuGlobalParameters *params);

typedef struct TkCtc21OnuMxuSnmpParameters
    {
    U8 snmpVersion; /* < SNMP Version */
    TkIPv4Address trapHostIP;   /* < Trap Host IP Address */
    U16 trapPort;   /* < Trap Port */
    U16 snmpPort;   /* < SNMP Port */
    U8 securityName[32];    /* < Security Name */
    U8 communityForRead[32];    /* < Community For Read */
    U8 communityForWrite[32];   /* < Community For Write */
    } PACK TkCtc21OnuMxuSnmpParameters;

int TkCTC_GetMxuSnmpParams(short int olt_id, short int onu_id, TkCtc21OnuMxuSnmpParameters *params);
int TkCTC_SetMxuSnmpParams(short int olt_id, short int onu_id, TkCtc21OnuMxuSnmpParameters *params);


typedef struct TkCtc21OpticalTransceiverDiagnosis
    {
    U16 transceiverTemperature; /* < Transceiver Temperature */
    U16 supplyVcc;  /* < Supply Voltage (Vcc) */
    U16 txBiasCur;  /* < Tx Bias Current */
    U16 txPowerOut; /* < Tx Power Output */
    U16 rxPowerIn;  /* < Rx Power Input */
    } PACK TkCtc21OpticalTransceiverDiagnosis;

int TkCTC_GetOpticalDiagInfo(short int olt_id, short int onu_id, TkCtc21OpticalTransceiverDiagnosis *diag_info);

typedef struct TkCtcOnuTxPowerSupplyControl
    {
    U32 action; /* < 0: re-enable the Tx Power supply; 1-65534:duration during which optical Tx Power supply is shut down,sec; 65535: permanently shutdown. */
    TkMacAddress onuId; /* < ONU Mac or ONU Base Mac for multi-LLID ONU */
    U32 opticalTxId;   /* TkCtcOnuOpticalTransmitterID< Ctc Onu Optical Transmitter ID */
    } PACK TkCtcOnuTxPowerSupplyControl;

typedef struct TkDiscoveryProcess
 {
    U16 period; /* < Discovery period in ms */
    U32 grantLength;    /* < Discovery grant length in bytes */
   	U8  downSpeed; /* < Downstream speed (where gate is sent) */
    U8  upSpeed;   /* < Upstream speed (which speed is invited) */
 }PACK TkDiscoveryProcess;

typedef struct TkGetDiscoveryParametersV2Response
    {
    U16 discoveryProcessesCount;    /*< Number of elements in Discovery Processes*/
    TkDiscoveryProcess discoveryProcesses;    /*< Discovery processes*/
    BOOL oneRegPerGrant;    /*< If set, then only on registration allowed per grant, otherwise there is not a limit.*/
    }PACK TkDiscoveryParametersV2;

typedef struct TkEponLoopTiming
{
    U16 minPropDelay;   /*< Minimum propagation delay*/
    U16 maxPropDelay;   /*< Maximum propagation delay*/
    U16 onuDelay;   /*< ONU Delay*/
    S16 oltUpDnDelayOffset; /*< OLT up/dn delay offset*/
    U16 nullGrantSize;  /*< Null Grant Size*/
 }PACK TkEponLoopTiming;


int TkCTC_SetPonPortTxPowerControl(short int olt_id, short int port_id, TkCtcOnuTxPowerSupplyControl *ctrl_info);
int TkOLT_DisableOlt(short int olt_id);
int TkOLT_EnaOlt(short int olt_id);
int TkOLT_GetDiscoveryParametersV2(short int olt_id, short int port_id, TkDiscoveryParametersV2 * discovery_parameters);
int TkOLT_SetDiscoveryParametersV2(short int olt_id, short int port_id, TkDiscoveryParametersV2 * discovery_parameters);
int TkOLT_GetLoopTiming(short int olt_id, short int port_id, TkEponLoopTiming * loop_timing);
int TkOLT_SetLoopTiming(short int olt_id, short int port_id, TkEponLoopTiming * loop_timing);
#endif


#if 1
/* ------------------------------------- CMC -----------------------------------*/

int TkCmGetCnuID ( const short int          olt_id, 
						    const short int      onu_id,
						    const mac_address_t  cnu_mac,
						    short int           *cnu_id );
int TkGetCnuMacAddress( const short int	    olt_id, 
								const short int     onu_id,
								const short int     cnu_id,
								mac_address_t       cnu_mac );

int TkAdapter_PAS5201_GetCmcAddrTable(short int olt_id, short int llid, mac_address_t cmc_mac, unsigned short *active_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_GetCnuAddrTable(short int olt_id, short int llid, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *active_records, PON_address_table_t address_table);
int TkAdapter_PAS5201_ResetCnuAddrTable(short int olt_id, short int llid, mac_address_t cmc_mac, mac_address_t cm_mac, PON_address_aging_t address_type);

int TkBridgeSetOltCmcSVlanID(short int olt_id, unsigned short svlan);

#endif


#if 1
/* ------------------------------------- PMC -----------------------------------*/

PON_STATUS TK_PASONU_set_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable);
PON_STATUS TK_PASONU_get_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable);


PON_STATUS TK_PASONU_address_table_clear( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id);

PON_STATUS TK_PASONU_address_table_get (
                            const PON_olt_id_t               olt_id, 
                            const PON_onu_id_t               onu_id,
                            long int                        *num_of_entries,
                            PON_onu_address_table_record_t  *address_table      );

PON_STATUS TK_PASONU_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable,
                                   const unsigned long  max_age);

PON_STATUS TK_PASONU_address_table_get_number_of_entries( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   long                *number_of_entries);


PON_STATUS TK_PASONU_uni_set_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable_cpu, 
                                   const bool           enable_datapath );


PON_STATUS TK_PASONU_encryption_get_state ( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   PON_encryption_direction_t  *direction);
PON_STATUS TK_PASONU_encryption_set_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_t         encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index);


PON_STATUS TK_PASONU_eeprom_mapper_set_parameter ( 
                              const PON_olt_id_t		    olt_id, 
						      const PON_onu_id_t	        onu_id,
							  const EEPROM_mapper_param_t   parameter, 
							  const void                   *data,
                              const unsigned long           size);
PON_STATUS TK_PASONU_eeprom_mapper_get_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
                                void                         *data,
								unsigned long                *size);


PON_STATUS TK_PASONU_get_burn_image_complete (
                                const PON_olt_id_t   olt_id, 
								const PON_onu_id_t   onu_id,
                                const bool          *complete);


PON_STATUS TK_PASONU_classifier_add_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                const PON_forwarding_action_t               action );
PON_STATUS TK_PASONU_classifier_remove_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value );

PON_STATUS TK_PASONU_classifier_add_source_address_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );

PON_STATUS TK_PASONU_classifier_add_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );
PON_STATUS TK_PASONU_classifier_remove_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address);

PON_STATUS TK_PASONU_classifier_l3l4_add_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num,
                                   const PON_forwarding_action_t              action );
PON_STATUS TK_PASONU_classifier_l3l4_remove_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num);

#endif



extern int ShowTkOnuTable(const short int olt_id, short int p_onu_id, struct vty * vty);

#ifdef BCM_WARM_BOOT_SUPPORT
/*B-added by liyang @2015-05-18 for warm restart */
typedef enum
{
	TK_OLT_MODE_NOT_CONNECTED,					/* OLT is not connected to the Host	/ Host recovery  	*/
												/* not supported										*/
	TK_OLT_MODE_CONNECTED_AND_NOT_CONFIGURED,	/* OLT is connected to the Host but has not been		*/
												/* initialized by the Host								*/
	TK_OLT_MODE_CONFIGURED_AND_ACTIVATED,		/* OLT is connected to the Host and has been initialized*/
												/* by the Host, this is the only mode which enables 	*/
												/* recovery from the OLT								*/
	TK_OLT_MODE_LAST_MODE						/* Indication for an invalid OLT recovery mode		 	*/
} TK_olt_mode_t;

extern int TK_olt_host_recovery 
                                   ( const PON_olt_id_t		              olt_id,
									 TK_olt_mode_t			              *olt_mode
									 );

/*E-added by liyang @2015-05-18 for warm restart */
#endif
#endif 
