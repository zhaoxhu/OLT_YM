/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcApi.h -  Header file for PMC RemoteManagement API Define 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 14/09/2012 |	creation	      | liwei056
*/

#if !defined(__ONU_PMC_API_H__)
#define __ONU_PMC_API_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "OnuPmcRemoteManagement.h"


typedef struct
{
    unsigned short              hw_major_ver;
    unsigned short              hw_minor_ver;
    unsigned short              fw_major_ver;
    unsigned short              fw_minor_ver;
    unsigned short              fw_build_num;
    unsigned short              fw_maintenance_ver;
    OAM_standard_version_t      oam_version;
} PON_device_version_t;

/* Get device version
**
** This function returns the hardware device version and the firmware version number.
**
** Input Parameters:
**	    olt_id : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Input Parameters:
**      versions : contains ONU version information. See PON_device_version_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_get_device_version( 
                                   const PON_olt_id_t     olt_id, 
                                   const PON_onu_id_t     onu_id,
                                   PON_device_version_t  *versions);


typedef struct
{
    bool                connection;
    bool                oam_link_established;
    bool                authorization_state;
    bool                pon_loopback;
    mac_address_t       mac_addr;
    PON_onu_id_t        onu_llid;
} PON_onu_device_status_t;

/* Get status
**
** This function gets various ONU status parameters.
**
** Input Parameters:
**	    olt_id : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Input Parameters:
**      status : contains current state of the ONU. See PON_onu_device_status_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_device_status( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_onu_device_status_t  *status);


/* Set slow protocol limit
**
** This function sets slow protocol limit state.
**
** Input Parameters:
**	    olt_id  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      enable  : Specifies whether slow protocol is enabled or disabled. 
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_set_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable);


/* Get slow protocol limit
**
** This function retrieve slow protocol limit state.
**
** Input Parameters:
**	    olt_id  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Input Parameters:
**      enable  : Specifies whether slow protocol is enabled or disabled. 
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable);



/* Get oam version
**
** This function retrieve the current version of OAM link.
**
** Input Parameters:
**	    olt_id  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      version : Version of the OAM link to be esteblished. See OAM_standard_version_t for details
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_get_oam_version( 
                                   const PON_olt_id_t       olt_id, 
                                   const PON_onu_id_t       onu_id,
                                   OAM_standard_version_t  *version);



/* Get oam frame length negotiation
**
** This function returns the OAM frame length that was negotiated during the OAM discovery sequence. 
**
** Input Parameters:
**	    olt_id		      : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		      : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      oam_frame_length  : Specifies the OAM frame length that was negotiated. 64- MAX_ETHERNET_FRAME_SIZE_STANDARDIZED
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_oam_frame_length_negotiation( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   unsigned short      *oam_frame_length );

typedef enum
{
    PON_RESET_NONE,
    PON_RESET_SW,
    PON_RESET_SW_ERROR
#ifdef  PAS_SOFT_VERSION_V5_3_5
    ,
    PON_RESET_PERIPH_LOW,
    PON_RESET_PERIPH_HIGH
#endif
} PON_reset_reason_t;

/* Reset device
**
** This function resets the ONU device.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_reset_device ( 
                         const PON_olt_id_t        olt_id, 
                         const PON_onu_id_t        onu_id,
                         const PON_reset_reason_t  reason);


/* Set alarm configuration
**
** This function configures the alarm generation thresholds and windows.
**
** Input Parameters:
**	    olt_id		            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      type                    : Type of the alarm for which parameters are set. See PON_alarm_event_type for details
**      enable_hook_function    : Specifies whether the hook function is to be called for this alarm.
**      window                  : Window size of alarm for which errors are collected.
**      threshold               : Threshold that is crossed for alarm indication. Given in number of errors.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_set_alarm_configuration( 
                                   const PON_olt_id_t          olt_id, 
                                   const PON_onu_id_t          onu_id,
                                   const PON_alarm_event_type  type,
                                   const bool                  enable_hook_function,
                                   const unsigned__int64       window,
                                   const unsigned__int64       threshold);


/* Get alarm configuration
**
** This function retrieves the alarm generation thresholds and windows
**
** Input Parameters:
**	    olt_id		            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      type                    : Type of the alarm for which parameters are set. See PON_alarm_event_type for details
**      enable_hook_function    : Specifies whether the hook function is to be called for this alarm.
** Output Parameters:
**      window                  : Window size of alarm for which errors are collected.
**      threshold               : Threshold that is crossed for alarm indication. Given in number of errors.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_alarm_configuration( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   const PON_alarm_event_type   type,
                                   bool                        *enable_hook_function,
                                   unsigned__int64             *window,
                                   unsigned__int64             *threshold);


/* Write ONU EEPROM register
**
** Write a value to an ONU EEPROM device register (16-bit). ONU EEPROM memory map is detailed in the
** ONU device data sheet document.
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		register_address	: EEPROM register number,
**							  range: PON_MIN_ONU_EEPROM_REGISTER_ADDRESS - PON_MAX_ONU_EEPROM_REGISTER_ADDRESS
**		data				: EEPROM register content, range: unsigned short int values
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_write_onu_eeprom_register ( 
                              const PON_olt_id_t		olt_id, 
							  const PON_onu_id_t	    onu_id,
							  const short int			register_address, 
							  const unsigned short int  data );


/* Read ONU EEPROM register
**
** Read an ONU EEPROM device register (16-bit) content. ONU EEPROM memory map is detailed in the
** ONU device data sheet document.
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		register_address	: EEPROM register number, 
**							  range: PON_MIN_ONU_EEPROM_REGISTER_ADDRESS - PON_MAX_ONU_EEPROM_REGISTER_ADDRESS
**
** Output Parameters:
**		data				: EEPROM register content.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_read_onu_eeprom_register ( 
                              const PON_olt_id_t   olt_id, 
							  const PON_onu_id_t   onu_id,
							  const short int	   register_address, 
							  unsigned short int  *data );


typedef enum
{
    EEPROM_MAPPER_COOKIE_ID = 0,
    EEPROM_MAPPER_EXT_RAM_READ_WS = 1,
    EEPROM_MAPPER_EXT_RAM_WRITE_WS = 2,
    EEPROM_MAPPER_EXT_RAM_SIZE = 3,
    EEPROM_MAPPER_FIRST_FW_START_ADDR = 4,
    EEPROM_MAPPER_SECOND_FW_START_ADDR = 5,
    EEPROM_MAPPER_BOOT_MONITOR_START_ADDR = 6,
    EEPROM_MAPPER_MANAGE_SECS_START_ADDR = 7,
    EEPROM_MAPPER_EXT_FLASH_READ_WS = 8,
    EEPROM_MAPPER_EXT_FLASH_WRITE_WS = 9,
    EEPROM_MAPPER_FLASH_VENDOR = 10,
    EEPROM_MAPPER_SDRAM_INFO_INDEX = 11,
    EEPROM_MAPPER_SDRAM_REFRESH_CYCLE = 12,
    EEPROM_MAPPER_SDRAM_TCAS = 13,
    EEPROM_MAPPER_SDRAM_TRAS = 14,
    EEPROM_MAPPER_SDRAM_WIDTH_MODE = 15,
    EEPROM_MAPPER_SDRAM_RBC_OR_BRC = 16,
    EEPROM_MAPPER_EXT_RAM_TYPE_IN_USE = 20,
    EEPROM_MAPPER_BOOT_LOADER_MODE = 21,
    EEPROM_MAPPER_UART_BAUD_INDEX = 22,
    EEPROM_MAPPER_PON_LOSS_SIGNAL_POLARITY = 23,
    EEPROM_MAPPER_PON_TBC_SIGNAL_POLARITY = 24,
    EEPROM_MAPPER_LASER_TX_ENABLE_POLARITY = 25,
    EEPROM_MAPPER_ZBT_USE = 28,
    EEPROM_MAPPER_ZBT_CONFIG = 29,
    EEPROM_MAPPER_PON_CONNECT_ON_POWER_UP = 30,
    EEPROM_MAPPER_OUI_ADDR = 31,
    EEPROM_MAPPER_EEPROM_MAC_ADDR = 32,
    EEPROM_MAPPER_EEPROM_IP_ADDR = 33,
    EEPROM_MAPPER_EEPROM_NET_MASK = 34,
    EEPROM_MAPPER_LASER_TON = 35,
    EEPROM_MAPPER_LASER_TOFF = 36,
    EEPROM_MAPPER_UNI_PHY_ADVERTISING_ENABLE = 37,
    EEPROM_MAPPER_UNI_BRIDGE_ENABLE = 38,
    EEPROM_MAPPER_UNI_AUTONEG_ENABLE = 39,
    EEPROM_MAPPER_UNI_MDIO_EXTERN_PHY_ADDR = 40,
    EEPROM_MAPPER_UNI_MAC_TYPE = 41,
    EEPROM_MAPPER_UNI_MASTER_MODE = 42,
    EEPROM_MAPPER_UNI_ADVERTISE_1000T_MULTI_PORT = 43,
    EEPROM_MAPPER_UNI_ADVERTISE_1000T_HALF_DUPLEX = 44,
    EEPROM_MAPPER_UNI_ADVERTISE_1000T_FULL_DUPLEX = 45,
    EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ASYMMETRIC = 46,
    EEPROM_MAPPER_UNI_ADVERTISE_PAUSE_ENABLED = 47,
    EEPROM_MAPPER_UNI_ADVERTISE_100T4 = 48,
    EEPROM_MAPPER_UNI_ADVERTISE_100TX_FD = 49,
    EEPROM_MAPPER_UNI_ADVERTISE_100TX_HD = 50,
    EEPROM_MAPPER_UNI_ADVERTISE_10TX_FD = 51,
    EEPROM_MAPPER_UNI_ADVERTISE_10TX_HD = 52,
    EEPROM_MAPPER_THRESHOLD_MODE = 53,
    EEPROM_MAPPER_UNIFY_THRESHOLD_MODE = 54,
    EEPROM_MAPPER_POINT_TO_POINT_ENABLE = 55,
    EEPROM_MAPPER_GRANTED_ALWAYS = 56,
    EEPROM_MAPPER_LASER_ON_PERMANENTLY = 57,
    EEPROM_MAPPER_PON_TX_DISABLE_DATA_FORMAT = 58,
    EEPROM_MAPPER_IDLE_BYTE0 = 59,
    EEPROM_MAPPER_IDLE_BYTE1 = 60,
    EEPROM_MAPPER_IDLE_BYTE2 = 61,
    EEPROM_MAPPER_IDLE_BYTE3 = 62,
    EEPROM_MAPPER_IDLE_PREAMBLE_DATA = 63,
    EEPROM_MAPPER_USER_NAME_802_1X = 64,
    EEPROM_MAPPER_PASSWORD_802_1X = 65,
    EEPROM_MAPPER_GENERAL_PURPOSE_FIELD1 = 66,
    EEPROM_MAPPER_GENERAL_PURPOSE_FIELD2 = 67,
    EEPROM_MAPPER_PON_CLOCK_FINE_TUNE = 68,
    EEPROM_MAPPER_TIMESTAMP_DELAY_FEC = 69,
    EEPROM_MAPPER_ARB_PON_TIMESTAMP_DELTA = 70,
    EEPROM_MAPPER_PON_CLK_CALIB_TX = 71,
    EEPROM_MAPPER_PON_CLK_CALIB_RX = 72,
    EEPROM_MAPPER_DYING_GASP_POLARITY = 73,
    EEPROM_MAPPER_LINK_FAULT_POLARITY = 74,
    EEPROM_MAPPER_CRITICAL_EVENT_POLARITY = 75,
    EEPROM_MAPPER_LAST_SYSTEM_PARAM,
    EEPROM_MAPPER_USER_PARAM_0 = 0x200,
    EEPROM_MAPPER_USER_PARAM_1,
    EEPROM_MAPPER_USER_PARAM_2,
    EEPROM_MAPPER_USER_PARAM_3,
    EEPROM_MAPPER_USER_PARAM_4,
    EEPROM_MAPPER_USER_PARAM_5,
    EEPROM_MAPPER_USER_PARAM_6,
    EEPROM_MAPPER_USER_PARAM_7,
    EEPROM_MAPPER_USER_PARAM_8,
    EEPROM_MAPPER_USER_PARAM_9,
    EEPROM_MAPPER_USER_PARAM_10,
    EEPROM_MAPPER_USER_PARAM_11,
    EEPROM_MAPPER_USER_PARAM_12,
    EEPROM_MAPPER_USER_PARAM_13,
    EEPROM_MAPPER_USER_PARAM_14,
    EEPROM_MAPPER_USER_PARAM_15,
    EEPROM_MAPPER_USER_PARAM_16,
    EEPROM_MAPPER_USER_PARAM_17,
    EEPROM_MAPPER_USER_PARAM_18,
    EEPROM_MAPPER_USER_PARAM_19,
    EEPROM_MAPPER_USER_PARAM_20,
    EEPROM_MAPPER_USER_PARAM_21,
    EEPROM_MAPPER_USER_PARAM_22,
    EEPROM_MAPPER_USER_PARAM_23,
    EEPROM_MAPPER_USER_PARAM_24,
    EEPROM_MAPPER_USER_PARAM_25,
    EEPROM_MAPPER_USER_PARAM_26,
    EEPROM_MAPPER_USER_PARAM_27,
    EEPROM_MAPPER_USER_PARAM_28,
    EEPROM_MAPPER_USER_PARAM_29,
    EEPROM_MAPPER_USER_PARAM_30,
    EEPROM_MAPPER_USER_PARAM_31,
    EEPROM_MAPPER_USER_PARAM_32,
    EEPROM_MAPPER_USER_PARAM_33,
    EEPROM_MAPPER_USER_PARAM_34,
    EEPROM_MAPPER_USER_PARAM_35,
    EEPROM_MAPPER_USER_PARAM_36,
    EEPROM_MAPPER_USER_PARAM_37,
    EEPROM_MAPPER_USER_PARAM_38,
    EEPROM_MAPPER_USER_PARAM_39,
    EEPROM_MAPPER_USER_PARAM_40,
    EEPROM_MAPPER_USER_PARAM_41,
    EEPROM_MAPPER_USER_PARAM_42,
    EEPROM_MAPPER_USER_PARAM_43,
    EEPROM_MAPPER_USER_PARAM_44,
    EEPROM_MAPPER_USER_PARAM_45,
    EEPROM_MAPPER_USER_PARAM_46,
    EEPROM_MAPPER_USER_PARAM_47,
    EEPROM_MAPPER_USER_PARAM_48,
    EEPROM_MAPPER_USER_PARAM_49,
    EEPROM_MAPPER_USER_PARAM_50,
    EEPROM_MAPPER_USER_PARAM_51,
    EEPROM_MAPPER_USER_PARAM_52,
    EEPROM_MAPPER_USER_PARAM_53,
    EEPROM_MAPPER_USER_PARAM_54,
    EEPROM_MAPPER_USER_PARAM_55,
    EEPROM_MAPPER_USER_PARAM_56,
    EEPROM_MAPPER_USER_PARAM_57,
    EEPROM_MAPPER_USER_PARAM_58,
    EEPROM_MAPPER_USER_PARAM_59,
    EEPROM_MAPPER_USER_PARAM_60,
    EEPROM_MAPPER_USER_PARAM_61,
    EEPROM_MAPPER_USER_PARAM_62,
    EEPROM_MAPPER_USER_PARAM_63,
    EEPROM_MAPPER_USER_PARAM_64,
    EEPROM_MAPPER_USER_PARAM_65,
    EEPROM_MAPPER_USER_PARAM_66,
    EEPROM_MAPPER_USER_PARAM_67,
    EEPROM_MAPPER_USER_PARAM_68,
    EEPROM_MAPPER_USER_PARAM_69,
    EEPROM_MAPPER_USER_PARAM_70,
    EEPROM_MAPPER_USER_PARAM_71,
    EEPROM_MAPPER_USER_PARAM_72,
    EEPROM_MAPPER_USER_PARAM_73,
    EEPROM_MAPPER_USER_PARAM_74,
    EEPROM_MAPPER_USER_PARAM_75,
    EEPROM_MAPPER_USER_PARAM_76,
    EEPROM_MAPPER_USER_PARAM_77,
    EEPROM_MAPPER_USER_PARAM_78,
    EEPROM_MAPPER_USER_PARAM_79,
    EEPROM_MAPPER_USER_PARAM_80,
    EEPROM_MAPPER_USER_PARAM_81,
    EEPROM_MAPPER_USER_PARAM_82,
    EEPROM_MAPPER_USER_PARAM_83,
    EEPROM_MAPPER_USER_PARAM_84,
    EEPROM_MAPPER_USER_PARAM_85,
    EEPROM_MAPPER_USER_PARAM_86,
    EEPROM_MAPPER_USER_PARAM_87,
    EEPROM_MAPPER_USER_PARAM_88,
    EEPROM_MAPPER_USER_PARAM_89,
    EEPROM_MAPPER_USER_PARAM_90,
    EEPROM_MAPPER_USER_PARAM_91,
    EEPROM_MAPPER_USER_PARAM_92,
    EEPROM_MAPPER_USER_PARAM_93,
    EEPROM_MAPPER_USER_PARAM_94,
    EEPROM_MAPPER_USER_PARAM_95,
    EEPROM_MAPPER_USER_PARAM_96,
    EEPROM_MAPPER_USER_PARAM_97,
    EEPROM_MAPPER_USER_PARAM_98,
    EEPROM_MAPPER_USER_PARAM_99,
    EEPROM_MAPPER_USER_PARAM_100,
    EEPROM_MAPPER_USER_PARAM_101,
    EEPROM_MAPPER_USER_PARAM_102,
    EEPROM_MAPPER_USER_PARAM_103,
    EEPROM_MAPPER_USER_PARAM_104,
    EEPROM_MAPPER_USER_PARAM_105,
    EEPROM_MAPPER_USER_PARAM_106,
    EEPROM_MAPPER_USER_PARAM_107,
    EEPROM_MAPPER_USER_PARAM_108,
    EEPROM_MAPPER_USER_PARAM_109,
    EEPROM_MAPPER_USER_PARAM_110,
    EEPROM_MAPPER_USER_PARAM_111,
    EEPROM_MAPPER_USER_PARAM_112,
    EEPROM_MAPPER_USER_PARAM_113,
    EEPROM_MAPPER_USER_PARAM_114,
    EEPROM_MAPPER_USER_PARAM_115,
    EEPROM_MAPPER_USER_PARAM_116,
    EEPROM_MAPPER_USER_PARAM_117,
    EEPROM_MAPPER_USER_PARAM_118,
    EEPROM_MAPPER_USER_PARAM_119,
    EEPROM_MAPPER_USER_PARAM_120,
    EEPROM_MAPPER_USER_PARAM_121,
    EEPROM_MAPPER_USER_PARAM_122,
    EEPROM_MAPPER_USER_PARAM_123,
    EEPROM_MAPPER_USER_PARAM_124,
    EEPROM_MAPPER_USER_PARAM_125,
    EEPROM_MAPPER_USER_PARAM_126,
    EEPROM_MAPPER_USER_PARAM_127,
    EEPROM_MAPPER_USER_PARAM_128,
    EEPROM_MAPPER_USER_PARAM_129,
    EEPROM_MAPPER_USER_PARAM_130,
    EEPROM_MAPPER_USER_PARAM_131,
    EEPROM_MAPPER_USER_PARAM_132,
    EEPROM_MAPPER_USER_PARAM_133,
    EEPROM_MAPPER_USER_PARAM_134,
    EEPROM_MAPPER_USER_PARAM_135,
    EEPROM_MAPPER_USER_PARAM_136,
    EEPROM_MAPPER_USER_PARAM_137,
    EEPROM_MAPPER_USER_PARAM_138,
    EEPROM_MAPPER_USER_PARAM_139,
    EEPROM_MAPPER_USER_PARAM_140,
    EEPROM_MAPPER_USER_PARAM_141,
    EEPROM_MAPPER_USER_PARAM_142,
    EEPROM_MAPPER_USER_PARAM_143,
    EEPROM_MAPPER_USER_PARAM_144,
    EEPROM_MAPPER_USER_PARAM_145,
    EEPROM_MAPPER_USER_PARAM_146,
    EEPROM_MAPPER_USER_PARAM_147,
    EEPROM_MAPPER_USER_PARAM_148,
    EEPROM_MAPPER_USER_PARAM_149,
    EEPROM_MAPPER_USER_PARAM_150,
    EEPROM_MAPPER_USER_PARAM_151,
    EEPROM_MAPPER_USER_PARAM_152,
    EEPROM_MAPPER_USER_PARAM_153,
    EEPROM_MAPPER_USER_PARAM_154,
    EEPROM_MAPPER_USER_PARAM_155,
    EEPROM_MAPPER_USER_PARAM_156,
    EEPROM_MAPPER_USER_PARAM_157,
    EEPROM_MAPPER_USER_PARAM_158,
    EEPROM_MAPPER_USER_PARAM_159,
    EEPROM_MAPPER_USER_PARAM_160,
    EEPROM_MAPPER_USER_PARAM_161,
    EEPROM_MAPPER_USER_PARAM_162,
    EEPROM_MAPPER_USER_PARAM_163,
    EEPROM_MAPPER_USER_PARAM_164,
    EEPROM_MAPPER_USER_PARAM_165,
    EEPROM_MAPPER_USER_PARAM_166,
    EEPROM_MAPPER_USER_PARAM_167,
    EEPROM_MAPPER_USER_PARAM_168,
    EEPROM_MAPPER_USER_PARAM_169,
    EEPROM_MAPPER_USER_PARAM_170,
    EEPROM_MAPPER_USER_PARAM_171,
    EEPROM_MAPPER_USER_PARAM_172,
    EEPROM_MAPPER_USER_PARAM_173,
    EEPROM_MAPPER_USER_PARAM_174,
    EEPROM_MAPPER_USER_PARAM_175,
    EEPROM_MAPPER_USER_PARAM_176,
    EEPROM_MAPPER_USER_PARAM_177,
    EEPROM_MAPPER_USER_PARAM_178,
    EEPROM_MAPPER_USER_PARAM_179,
    EEPROM_MAPPER_USER_PARAM_180,
    EEPROM_MAPPER_USER_PARAM_181,
    EEPROM_MAPPER_USER_PARAM_182,
    EEPROM_MAPPER_USER_PARAM_183,
    EEPROM_MAPPER_USER_PARAM_184,
    EEPROM_MAPPER_USER_PARAM_185,
    EEPROM_MAPPER_USER_PARAM_186,
    EEPROM_MAPPER_USER_PARAM_187,
    EEPROM_MAPPER_USER_PARAM_188,
    EEPROM_MAPPER_USER_PARAM_189,
    EEPROM_MAPPER_USER_PARAM_190,
    EEPROM_MAPPER_USER_PARAM_191,
    EEPROM_MAPPER_USER_PARAM_192,
    EEPROM_MAPPER_USER_PARAM_193,
    EEPROM_MAPPER_USER_PARAM_194,
    EEPROM_MAPPER_USER_PARAM_195,
    EEPROM_MAPPER_USER_PARAM_196,
    EEPROM_MAPPER_USER_PARAM_197,
    EEPROM_MAPPER_USER_PARAM_198,
    EEPROM_MAPPER_USER_PARAM_199,
    EEPROM_MAPPER_USER_PARAM_200,
    EEPROM_MAPPER_USER_PARAM_201,
    EEPROM_MAPPER_USER_PARAM_202,
    EEPROM_MAPPER_USER_PARAM_203,
    EEPROM_MAPPER_USER_PARAM_204,
    EEPROM_MAPPER_USER_PARAM_205,
    EEPROM_MAPPER_USER_PARAM_206,
    EEPROM_MAPPER_USER_PARAM_207,
    EEPROM_MAPPER_USER_PARAM_208,
    EEPROM_MAPPER_USER_PARAM_209,
    EEPROM_MAPPER_USER_PARAM_210,
    EEPROM_MAPPER_USER_PARAM_211,
    EEPROM_MAPPER_USER_PARAM_212,
    EEPROM_MAPPER_USER_PARAM_213,
    EEPROM_MAPPER_USER_PARAM_214,
    EEPROM_MAPPER_USER_PARAM_215,
    EEPROM_MAPPER_USER_PARAM_216,
    EEPROM_MAPPER_USER_PARAM_217,
    EEPROM_MAPPER_USER_PARAM_218,
    EEPROM_MAPPER_USER_PARAM_219,
    EEPROM_MAPPER_USER_PARAM_220,
    EEPROM_MAPPER_USER_PARAM_221,
    EEPROM_MAPPER_USER_PARAM_222,
    EEPROM_MAPPER_USER_PARAM_223,
    EEPROM_MAPPER_USER_PARAM_224,
    EEPROM_MAPPER_USER_PARAM_225,
    EEPROM_MAPPER_USER_PARAM_226,
    EEPROM_MAPPER_USER_PARAM_227,
    EEPROM_MAPPER_USER_PARAM_228,
    EEPROM_MAPPER_USER_PARAM_229,
    EEPROM_MAPPER_USER_PARAM_230,
    EEPROM_MAPPER_USER_PARAM_231,
    EEPROM_MAPPER_USER_PARAM_232,
    EEPROM_MAPPER_USER_PARAM_233,
    EEPROM_MAPPER_USER_PARAM_234,
    EEPROM_MAPPER_USER_PARAM_235,
    EEPROM_MAPPER_USER_PARAM_236,
    EEPROM_MAPPER_USER_PARAM_237,
    EEPROM_MAPPER_USER_PARAM_238,
    EEPROM_MAPPER_USER_PARAM_239,
    EEPROM_MAPPER_USER_PARAM_240,
    EEPROM_MAPPER_USER_PARAM_241,
    EEPROM_MAPPER_USER_PARAM_242,
    EEPROM_MAPPER_USER_PARAM_243,
    EEPROM_MAPPER_USER_PARAM_244,
    EEPROM_MAPPER_USER_PARAM_245,
    EEPROM_MAPPER_USER_PARAM_246,
    EEPROM_MAPPER_USER_PARAM_247,
    EEPROM_MAPPER_USER_PARAM_248,
    EEPROM_MAPPER_USER_PARAM_249,
    EEPROM_MAPPER_USER_PARAM_250,
    EEPROM_MAPPER_USER_PARAM_251,
    EEPROM_MAPPER_USER_PARAM_252,
    EEPROM_MAPPER_USER_PARAM_253,
    EEPROM_MAPPER_USER_PARAM_254,
    EEPROM_MAPPER_USER_PARAM_255,
    EEPROM_MAPPER_LAST_PARAM
} EEPROM_mapper_param_t;

/* Set EEPROM mapper parameter
**
** This function writes EEPROM parameter to the ONU EEPROM.
**
** Input Parameters:
**		olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		parameter	: Parameter of EEPROM. See EEPROM_mapper_param_t for details.
**      data        : Pointer to data container of parameter's value.
**		size		: Size of parameter in bytes.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_eeprom_mapper_set_parameter ( 
                              const PON_olt_id_t		    olt_id, 
						      const PON_onu_id_t	        onu_id,
							  const EEPROM_mapper_param_t   parameter, 
							  const void                   *data,
                              const unsigned long           size);



/* Get EEPROM mapper parameter
**
** This function reads from the EEPROM device of the ONU. It reads values stored in the address
** that was specified by the user in the EEPROM device.  
**
** Input Parameters:
**		olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		parameter	: Parameter of EEPROM. See EEPROM_mapper_param_t for details.
**		size		: allocated size of parameter in bytes.
**
** Output Parameters:
**      data        : Pointer to data container of parameter's value.
**		size		: received size of parameter in bytes.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_eeprom_mapper_get_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
                                void                         *data,
								unsigned long                *size);



/* clear address table
**
** This function clears all entries in the address table
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/


PON_STATUS RM_PASONU_address_table_clear( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id);


/* Set address table UNI learning
**
** This function enables/disables the address table learning from the user network interface port.
**
** Input Parameters:
**	    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      enable  : Specifies whether the UNI learning is enabled or disabled.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_address_table_set_uni_learning( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id,
                                   const bool          enable);


/* Set address table ageing configuration
**
** This function enables/disables the address table ageing mechanism, and also sets the maximum 
** allowed age of an entry before it is to be flushed out (in case ageing is enabled).
**
** Input Parameters:
**	    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      enable  : Specifies whether the ageing is enabled/disabled.
**      max_age : Maximum age of an entry in seconds.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable,
                                   const unsigned long  max_age);



/* Get address table ageing configuration
**
** This function retrieves address table ageing mechanism, and maximum 
** allowed age of an entry before it is to be flushed out (in case ageing is enabled).
**
** Input Parameters:
**	    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      enable  : Specifies whether the ageing is enabled/disabled.
**      max_age : Maximum age of an entry in seconds.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable,
                                   unsigned long	   *max_age);

/* Get address table number of entries
**
** This function returns the current total number of MAC addresses in the address table.
**
** Input Parameters:
**	    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      number_of_entries  : Current number of entries in the address table. 0- 128
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_address_table_get_number_of_entries( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   long                *number_of_entries);

typedef struct
{
    mac_address_t            addr;
    PON_forwarding_action_t  action;
    unsigned char            age;
    PON_address_aging_t      type;
} PON_onu_address_table_record_t;

/* Get all address table records
**
** This API command queries all the records from the ONU address table. It combines several PAS6201
** address table API commands.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      num_of_entries : Pointer to the number of the records in the address table 
**      address_table  : Pointer to an array of elements (the address table records).
**
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_address_table_get (
                            const PON_olt_id_t               olt_id, 
                            const PON_onu_id_t               onu_id,
                            long int                        *num_of_entries,
                            PON_onu_address_table_record_t  *address_table      );



/* Address Table setting limit for UNI Learning
**
** This function set limit to number of dynamic address that will learn from UNI. If this limit is 
** set the SA address that isn't learn will not pass to the upstream.
**
** Input Parameters:
**	    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      limit     : Specifies the UNI learning limit.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_address_table_set_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const unsigned long  limit);


/* Address Table setting limit for UNI Learning
**
** This function get limit to number of dynamic address that will learn from UNI. 
**
** Input Parameters:
**	    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      limit     : Specifies the UNI learning limit.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_address_table_get_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   unsigned long       *limit);


/* Set UNI port
**
** This function enables/disables the UNI port traffic on two paths: towards the CPU and to the data path.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      enable_cpu     : Enables/disables UNI port traffic towards the CPU.
**      enable_datapath: Enables/disables UNI port traffic to the data path.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_uni_set_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable_cpu, 
                                   const bool           enable_datapath );


/* Get UNI port
**
** function gets the UNI port state of both paths: towards the CPU and to the data path.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      enable_cpu     : Enables/disables UNI port traffic towards the CPU.
**      enable_datapath: Enables/disables UNI port traffic to the data path.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_uni_get_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable_cpu, 
                                   bool                *enable_datapath );

typedef enum
{
    PON_MII_RATE_10_MBPS,
    PON_MII_RATE_100_MBPS
} PON_mii_rate_t;

typedef enum
{
    PON_LINK_DOWN,
    PON_LINK_UP
} PON_link_state_t;

typedef struct
{
    PON_mac_t                       mac_type;
    PON_mii_rate_t                  mii_rate;
    PON_duplex_status_t             duplex;
    PON_master_slave_t              gmii_master_mode;
    PON_auto_negotiation_t          auto_negotiate;
    PON_master_slave_t              mode;
    bool                            advertise;
    PON_advertise_t                 adv_10_half;
    PON_advertise_t                 adv_10_full;
    PON_advertise_t                 adv_100_half;
    PON_advertise_t                 adv_100_full;
    PON_advertise_t                 adv_100_t4;
    PON_advertise_t                 adv_pause;
    PON_advertise_t                 adv_asym_pause;
    PON_advertise_t                 adv_1000_half;
    PON_advertise_t                 adv_1000_full;
    PON_advertise_port_type_t       adv_port_type;
    unsigned char                   phy_address;
} PON_uni_configuration_t;

/* Set UNI port configuration
**
** This function sets parameters of the internal UNI port.
**
** Input Parameters:
**	    olt_id		      : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		      : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      uni_configuration : See PON_uni_configuration_t for details
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_uni_set_port_configuration( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id,
                                   const PON_uni_configuration_t  *uni_configuration );


/* Get UNI port configuration
**
** This function retrieves the configuration of the internal UNI port.
**
** Input Parameters:
**	    olt_id		      : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		      : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      uni_configuration : See PON_uni_configuration_t for details
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_uni_get_port_configuration( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_uni_configuration_t  *uni_configuration );

    
typedef struct
{
    PON_link_state_t            link;
    PON_mac_t                   mac_type;
    PON_mii_rate_t              mii_rate;
    PON_duplex_status_t         duplex;
    PON_auto_negotiation_t      auto_negotiate;
    PON_master_slave_t          mode;
    bool                        pause_tx;
    bool                        pause_rx;
    PON_advertise_port_type_t   port_type;
} PON_uni_status_t;

/* Get UNI port status
**
** This function gets the current status of the UNI port.
**
** Input Parameters:
**	    olt_id		      : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		      : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      uni_status        : See PON_uni_status_t for details
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_uni_get_port_status( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   PON_uni_status_t    *uni_status );



/* complete burn image
**
** This function indicates whether an image burn is in progress.
**
** Input Parameters:
**		olt_id	        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	        : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		complete        : indicates wheter an image burn completed or not.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_get_burn_image_complete (
                                const PON_olt_id_t   olt_id, 
								const PON_onu_id_t   onu_id,
                                const bool          *complete);


/* Set encryption state								
**
** This function controls the PON data-path encryption
**
** Input Parameters:
**	    olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction   : see PON_encryption_direction_t for details
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_encryption_set_state ( 
                                   const PON_olt_id_t                olt_id, 
                                   const PON_onu_id_t                onu_id,
                                   const PON_encryption_direction_t  direction);



/* Get encryption state								
**
** This function controls the PON data-path encryption
**
** Input Parameters:
**	    olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      direction   : see PON_encryption_direction_t for details
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_encryption_get_state ( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   PON_encryption_direction_t  *direction);


/* Set encryption key								
**
** This function sets the encryption key, which the ONU will use to encrypt and decrypt data frames.
**
** Input Parameters:
**	    olt_id	                : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      encryption_key          : Encryption key that the ONU will use to encrypt and decrypt frames.
**                                see PON_encryption_key_t for details
**      encryption_key_index    : Key index to be set.see PON_encryption_key_index_t for details
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_encryption_set_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_t         encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index);


/* Get encryption key								
**
** This function retrieves the encryption key, which the ONU is using to encrypt and decrypt data frames.
**
** Input Parameters:
**	    olt_id	                : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      encryption_key_index    : Key index to retrieve.see PON_encryption_key_index_t for details
**
** Output Parameters:
**      encryption_key          : Encryption key that the ONU is using to encrypt and decrypt frames.
**                                see PON_encryption_key_t for details
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_encryption_get_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   PON_encryption_key_t               encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index);



/* Add classification filter
**
** This function sets the desired filter action.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction      : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      qualifier      : See PON_frame_qualifier_t for details.
**      value          : The value according to which filtering is performed.
**      action         : See PON_forwarding_action_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_classifier_add_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                const PON_forwarding_action_t               action );



/* Get classification filter
**
** This function gets the desired filter's action.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction      : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      qualifier      : See PON_frame_qualifier_t for details.
**      value          : The value according to which filtering is performed.
**
** Output Parameters:
**      action         : current action of the specified filter. See PON_forwarding_action_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/
PON_STATUS RM_PASONU_classifier_get_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                PON_forwarding_action_t                    *action );


/* Remove classification filter
**
** This function removes the desired filter and its action from the filter hardware module.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction      : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      qualifier      : See PON_frame_qualifier_t for details.
**      value          : The value according to which filtering is performed.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_classifier_remove_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value );


/* Add destination address filter to the classifier
**
** This function adds a MAC address and a forwarding action as a classification rule.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      address        : DA used for classification
**      action         : Specifies whether a frame having the specified DA is directed to the CPU 
**                       and/or to the data path. See PON_forwarding_action_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_classifier_add_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );


/* Add source address filter to the classifier
**
** This function adds a MAC address and a forwarding action as a classification rule. The rule 
** forwards any frame in the upstream with the configure source MAC address according to the defined action
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      address        : DA used for classification
**      action         : Specifies whether a frame having the specified DA is directed to the CPU 
**                       and/or to the data path. See PON_forwarding_action_t for details.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

PON_STATUS RM_PASONU_classifier_add_source_address_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );


/* Remove destination address filter from the classifier
**
** This function removes a MAC address classification rule. 
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      address        : DA used for classification.
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/


PON_STATUS RM_PASONU_classifier_remove_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address);


/* Add classifier L3L4 filter
**
** This function is used to set the filtering rule in the L4PC hardware module. The action argument
** determines what will be done with the packet. The packet can continue in the datapath, copied 
** to the CPU, continue in the datapath and copied to the CPU, or discarded. If the rule is not 
** added an error is returned
**
** Input Parameters:
**	    olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction           : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      traffic             : The type of traffic of the new queue mapping rule
**      address             : The direction of traffic 
**      ip_address          : The IP address of the traffic to be prioritized. This input argument is used only if
**                            the traffic indicate IP traffic; in this case the address can be either source or destination
**      l4_port_num         : The UDP source port number or TCP source port number of the traffic to be prioritized.
**                            This input argument is used only if the traffic indicate UDP or TCP port traffic;
**                            in this case the address can be only source
**      action              : Determines whether a frame matching the filter is directed to the CPU and/or to the data path
** Output Parameters:
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
*/

PON_STATUS RM_PASONU_classifier_l3l4_add_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num,
                                   const PON_forwarding_action_t              action );



/* Remove classifier L3L4 filter
**
** The function removes a desired traffic filtering rule from the L4PC hardware module, of the 
** appropriate port
**
** Input Parameters:
**	    olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction           : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      traffic             : The type of traffic of the new queue mapping rule
**      address             : The direction of traffic 
**      ip_address          : The IP address of the traffic to be prioritized. This input argument is used only if
**                            the traffic indicate IP traffic; in this case the address can be either source or destination
**      l4_port_num         : The UDP source port number or TCP source port number of the traffic to be prioritized.
**                            This input argument is used only if the traffic indicate UDP or TCP port traffic;
**                            in this case the address can be only source
** Output Parameters:
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
*/
PON_STATUS RM_PASONU_classifier_l3l4_remove_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num);


/* Get classifier L3L4 filter
**
** The function looks for the traffic filtering rule in the L4PC hardware module
**
** Input Parameters:
**	    olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**      direction           : Signifies the direction of data flow.PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK only
**      traffic             : The type of traffic of the new queue mapping rule
**      address             : The direction of traffic 
**      ip_address          : The IP address of the traffic to be prioritized. This input argument is used only if
**                            the traffic indicate IP traffic; in this case the address can be either source or destination
**      l4_port_num         : The UDP source port number or TCP source port number of the traffic to be prioritized.
**                            This input argument is used only if the traffic indicate UDP or TCP port traffic;
**                            in this case the address can be only source
** Output Parameters:
**      action              : Determines whether a frame matching the filter is directed to the CPU and/or to the data path
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
*/
PON_STATUS RM_PASONU_classifier_l3l4_get_filter ( 
                                   const PON_olt_id_t                          olt_id,
                                   const PON_onu_id_t                          onu_id,
                                   const PON_pon_network_traffic_direction_t   direction,
                                   const PON_traffic_qualifier_t               traffic,    
                                   const PON_traffic_address_t                 address,    
                                   const unsigned long                         ip_address, 
                                   const unsigned short int                    l4_port_num,
                                   PON_forwarding_action_t                    *action );



#if defined(__cplusplus)
}
#endif

#endif /* __ONU_PMC_API_H__ */


