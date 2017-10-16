/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  OnuPmcComm.h -  Header file for PMC RemoteManagement Comm Define 
**
**  This file was written by liwei056, 16/10/2013
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 14/09/2012 |	creation	      | liwei056
*/

#if !defined(__ONU_PMC_COMM_H__)
#define __ONU_PMC_COMM_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "OnuPmcRemoteManagement.h"
#include "OnuPmcApi.h"
#include "PonMicroParser.h"


typedef enum
{
   ONU_HPROT_GROUP_EPONM_OPCODES,
   ONU_HPROT_GROUP_FRAME_PROCESSING_OPCODES,
   ONU_HPROT_GROUP_ADDR_TABLE_OPCODES,
   ONU_HPROT_GROUP_UNI_OPCODES,
   ONU_HPROT_GROUP_OAM_OPCODES,
   ONU_HPROT_GROUP_DEVICE_OPCODES,
   ONU_HPROT_GROUP_PERIPH_OPCODES,
   ONU_HPROT_GROUP_PROTOCOL_OPCODES,
   ONU_HPROT_GROUP_EVENTS_OPCODES,
   ONU_HPROT_LAST_GROUP_OPCODE = ONU_HPROT_GROUP_EVENTS_OPCODES
   
   
}  ONU_HPROT_group_opcode_enum_t;

typedef enum
{
   ONU_HPROT_SET_ENCRYPTION_CONFIGURATION_OPCODE          =  0x02,
   ONU_HPROT_ENCRYPTION_SET_OPCODE                        =  0x03,
   ONU_HPROT_ENCRYPTION_GET_STATE_OPCODE                  =  0x04,
   ONU_HPROT_ENCRYPTION_KEY_SET_OPCODE                    =  0x05,
   ONU_HPROT_ENRYPTION_KEY_GET_OPCODE                     =  0x06,
   ONU_HPROT_SET_ACTIVE_KEY_OPCODE                        =  0x07,
   ONU_HPROT_GET_ACTIVE_KEY_OPCODE                        =  0x08,
   ONU_HPROT_FEC_SET_CONFIG_OPCODE                        =  0x09,
   ONU_HPROT_FEC_GET_CONFIG_OPCODE                        =  0x0A,
   ONU_HPROT_SET_ENCRYPTION_MODE_OPCODE                   =  0x0B,
   ONU_HPROT_GET_ENCRYPTION_MODE_OPCODE                   =  0x0C,
   ONU_HPROT_GET_ENCRYPTION_CONFIGURATION_OPCODE          =  0x0D,
   ONU_HPROT_FEC_SET_OUT_CLCK          					  =  0x0E,
   ONU_HPROT_FEC_GET_OUT_CLCK 					          =  0x0F,
   ONU_HPROT_LAST_EPONM_OPCODE                            =  ONU_HPROT_FEC_GET_OUT_CLCK
   
   
}  ONU_HPROT_EPONM_opcode_enum_t;

typedef enum
{
   ONU_HPROT_SET_CLASSIFIER_DEFAULT_OPCODE                =  0x02,
   ONU_HPROT_GET_CLASSIFIER_DEFAULT_OPCODE                =  0x03,
   ONU_HPROT_SET_DEFAULT_PRIORITY_TRANSLATION_OPCODE      =  0x04,
   ONU_HPROT_GET_DEFAULT_PRIORITY_TRANSLATION_OPCODE      =  0x05,
   ONU_HPROT_SET_CLASSIFIER_DEFAULT_QUEUE_MAPPING_OPCODE  =  0x06,
   ONU_HPROT_GET_CLASSIFIER_DEFAULT_QUEUE_MAPPING_OPCODE  =  0x07,
   ONU_HPROT_ADD_CLASSIFIER_FILTER_OPCODE                 =  0x08,
   ONU_HPROT_GET_CLASSIFIER_FILTER_OPCODE                 =  0x09,
   ONU_HPROT_REMOVE_CLASSIFIER_FILTER_OPCODE              =  0x0A,
   ONU_HPROT_CLASSIFIER_ADD_DA_FILTER_OPCODE              =  0x0B,
   ONU_HPROT_CLASSIFIER_REMOVE_DA_FILTER_OPCODE           =  0x0C,
   ONU_HPROT_SET_PRIORITY_TRANSLATION_OPCODE              =  0x0D,
   ONU_HPROT_GET_PRIORITY_TRANSLATION_OPCODE              =  0x0E,
   ONU_HPROT_SET_CLASSIFIER_QUEUE_MAPPING_OPCODE          =  0x0F,
   ONU_HPROT_GET_CLASSIFIER_QUEUE_MAPPING_OPCODE          =  0x10,
   ONU_HPROT_ADD_PRIORITY_MANIPULATION_OPCODE             =  0x11,
   ONU_HPROT_GET_PRIORITY_MANIPULATION_OPCODE             =  0x12,
   ONU_HPROT_REMOVE_PRIORITY_MANIPULATION_OPCODE          =  0x13,
   ONU_HPROT_SET_IP_MULTICAST_OPCODE                      =  0x14,
   ONU_HPROT_GET_IP_MULTICAST_OPCODE                      =  0x15,
   ONU_HPROT_SET_POLICER_PARAMETERS_OPCODE                =  0x16,
   ONU_HPROT_GET_POLICER_PARAMETERS_OPCODE                =  0x17,
   ONU_HPROT_SET_VLAN_CONFIGURATION_OPCODE                =  0x18,
   ONU_HPROT_GET_VLAN_CONFIGURATION_OPCODE                =  0x19,
   ONU_HPROT_SET_VLAN_MODE_OPCODE                         =  0x1A,
   ONU_HPROT_GET_VLAN_MODE_OPCODE                         =  0x1B,
   ONU_HPROT_SET_VLAN_DEFAULTS_CONFIGURATION_OPCODE       =  0x1C,
   ONU_HPROT_GET_VLAN_DEFAULTS_CONFIGURATION_OPCODE       =  0x1D,
   ONU_HPROT_ADD_VLAN_ENGINE_RULE_OPCODE                  =  0x1E,
   ONU_HPROT_REMOVE_VLAN_ENGINE_RULE_OPCODE               =  0x1F,
   ONU_HPROT_GET_VLAN_ENGINE_RULE_OPCODE                  =  0x20,
   ONU_HPROT_SET_THRESHOLD_REPORTER_CONFIG_OPCODE         =  0x21,
   ONU_HPROT_GET_THRESHOLD_REPORTER_CONFIG_OPCODE         =  0x22,
   ONU_HPROT_SET_PQUEUE_INGRESS_LIMIT_OPCODE              =  0x23,
   ONU_HPROT_GET_PQUEUE_INGRESS_LIMIT_OPCODE              =  0x24,
   ONU_HPROT_SET_PQUEUE_FLOW_CONTROL_OPCODE               =  0x25,
   ONU_HPROT_GET_PQUEUE_FLOW_CONTROL_OPCODE               =  0x26,
   ONU_HPROT_SET_GENERAL_PQUEUE_FLOW_CONTROL_OPCODE       =  0x27,
   ONU_HPROT_GET_GENERAL_PQUEUE_FLOW_CONTROL_OPCODE       =  0x28,
   ONU_HPROT_GET_PQUEUE_OCCUPANCY_OPCODE                  =  0x29,
   ONU_HPROT_PQUEUE_EXT_MEM_TEST_OPCODE                   =  0x2F,
   ONU_HPROT_SET_RESERVED_ADDRESSES_TO_CPU_OPCODE         =  0x30,
   ONU_HPROT_GET_RESERVED_ADDRESSES_TO_CPU_OPCODE         =  0x31,
   ONU_HPROT_SET_CLASSIFIER_DSCP_CONFIGURATION            =  0x32,
   ONU_HPROT_GET_CLASSIFIER_DSCP_CONFIGURATION            =  0x33,
   ONU_HPROT_SET_CLASSIFIER_L3L4_CONFIGURATION            =  0x34,
   ONU_HPROT_GET_CLASSIFIER_L3L4_CONFIGURATION            =  0x35,
   ONU_HPROT_SET_CLASSIFIER_L3L4_QUEUE_MAPPING            =  0x36,
   ONU_HPROT_REMOVE_CLASSIFIER_L3L4_QUEUE_MAPPING         =  0x37,
   ONU_HPROT_GET_CLASSIFIER_L3L4_QUEUE_MAPPING            =  0x38,
   ONU_HPROT_ADD_CLASSIFIER_L3L4_FILTER_OPCODE            =  0x39,
   ONU_HPROT_REMOVE_CLASSIFIER_L3L4_FILTER_OPCODE         =  0x3A,
   ONU_HPROT_GET_CLASSIFIER_L3L4_FILTER_OPCODE            =  0x3B,
   ONU_HPROT_CLASSIFIER_ADD_UP_SA_FILTER_OPCODE           =  0x3C,
   ONU_HPROT_CLASSIFIER_MULTICAST_CONFIG_SET_OPCODE       =  0x3D,
   ONU_HPROT_CLASSIFIER_MULTICAST_CONFIG_GET_OPCODE       =  0x3E,
   ONU_HPROT_SET_SINGLE_RATE_OPCODE                       =  0x3F,
   ONU_HPROT_GET_SINGLE_RATE_OPCODE                       =  0x40,
   ONU_HPROT_SET_VLAN_PRIORITY_METHOD_OPCODE              =  0x41,
   ONU_HPROT_GET_VLAN_PRIORITY_METHOD_OPCODE              =  0x42,
   ONU_HPROT_SET_CLASSIFIER_VLAN_PRECEDENCE               =  0x43,
   ONU_HPROT_GET_CLASSIFIER_VLAN_PRECEDENCE               =  0x44,
   ONU_HPROT_SET_POLICER_CAST                             =  0x45,
   ONU_HPROT_GET_POLICER_CAST                             =  0x46,
   ONU_HPROT_CLASSIFIER_DELETE_USER_RULES_OPCODE          =  0x47,
   ONU_HPROT_SET_POLICER_VID_OPCODE                       =  0x48,
   ONU_HPROT_GET_POLICER_VID_OPCODE                       =  0x49,
   ONU_HPROT_GET_PQUEUE_NET_OCCUPANCY_OPCODE              =  0x4A,
   ONU_HPROT_GET_CLASSIFIER_USER_RULES_OPCODE             =  0x4B,
   ONU_HPROT_GET_PQUEUE_OVERLAP_OPCODE                    =  0x4C,
   ONU_HPROT_SET_PQUEUE_OVERLAP_OPCODE                    =  0x4D,
   ONU_HPROT_GET_SHAPE_LIMIT_OPCODE                       =  0x4E,
   ONU_HPROT_SET_SHAPE_LIMIT_OPCODE                       =  0x4F,
   ONU_HPROT_SET_CLASSIFIER_IPV4_SUBNET_FILTER_OPCODE     =  0x50,
   ONU_HPROT_GET_CLASSIFIER_IPV4_SUBNET_FILTER_OPCODE     =  0x51,
   ONU_HPROT_DEL_CLASSIFIER_IPV4_SUBNET_FILTER_OPCODE     =  0x52,
   ONU_HPROT_SET_CLASSIFIER_IPV4_SUBNET_QUEUE_MAPPING_OPCODE  =  0x53,
   ONU_HPROT_GET_CLASSIFIER_IPV4_SUBNET_QUEUE_MAPPING_OPCODE  =  0X54,
   ONU_HPROT_DEL_CLASSIFIER_IPV4_SUBNET_QUEUE_MAPPING_OPCODE  =  0x55,
   ONU_HPROT_SET_SHAPE_LIMITS_PER_QUEUE_OPCODE            =  0x56,
   ONU_HPROT_GET_SHAPE_LIMITS_PER_QUEUE_OPCODE            =  0x57,
   ONU_HPROT_LAST_FRAME_PROCESSING_OPCODE                 =  ONU_HPROT_GET_SHAPE_LIMITS_PER_QUEUE_OPCODE

}  ONU_HPROT_FRAME_PROCESSING_opcode_enum_t;


typedef enum
{
   ONU_HPROT_ADDRTBL_CLEAR_ALL_OPCODE                     = 0x01,
   ONU_HPROT_SET_ADDRTBL_UNI_LEARN_OPCODE                 = 0x02,
   ONU_HPROT_ADDRTBL_AGING_CONFIG_OPCODE                  = 0x03,
   ONU_HPROT_GET_ADDRTBL_ENTRY_NUM_OPCODE                 = 0x04,
   ONU_HPROT_GET_ADDRESS_CONFIGURATION_OPCODE             = 0x05,
   ONU_HPROT_GET_LRU_ADDRESS_OPCODE                       = 0x06,
   ONU_HPROT_GET_ADTBL_FIRST_ELEMENT_OPCODE               = 0x07,
   ONU_HPROT_GET_ADTBL_NEXT_ELEMENT_OPCODE                = 0x08,
   ONU_HPROT_GET_ADTBL_CURRENT_ELEMENT_OPCODE             = 0x09,
   ONU_HPROT_SET_ADDRTBL_UNI_LEARN_LIMIT_OPCODE           = 0x0A,
   ONU_HPROT_GET_ADDRTBL_UNI_LEARN_LIMIT_OPCODE           = 0x0B,
   ONU_HPROT_ADDRTBL_GET_AGING_CONFIG_OPCODE              = 0x0C,
   ONU_HPROT_LAST_ADDR_TABLE_OPCODE                       = ONU_HPROT_ADDRTBL_GET_AGING_CONFIG_OPCODE

}  ONU_HPROT_ADDR_TABLE_opcode_enum_t;

typedef enum
{
   ONU_HPROT_SET_UNI_PORT_OPCODE                          = 0x01,
   ONU_HPROT_GET_UNI_PORT_OPCODE                          = 0x02,
   ONU_HPROT_SET_UNI_PORT_CONFIGURATION_OPCODE            = 0x03,
   ONU_HPROT_GET_UNI_PORT_CONFIGURATION_OPCODE            = 0x04,
   ONU_HPROT_GET_UNI_STATUS_OPCODE                        = 0x05,
   ONU_HPROT_SET_UNI_EXT_CONFIG_OPCODE                    = 0x06,
   ONU_HPROT_GET_UNI_EXT_CONFIG_OPCODE                    = 0x07,
   ONU_HPROT_LAST_UNI_OPCODE                              = ONU_HPROT_GET_UNI_EXT_CONFIG_OPCODE
   
}  ONU_HPROT_UNI_opcode_enum_t;

typedef enum
{
   ONU_HPROT_GET_OAM_LENGTH_NEGOTATION_OPCODE             = 0x01,
   ONU_HPROT_SET_OAM_MAX_FRAME_LENGTH_OPCODE              = 0x02,
   ONU_HPROT_SET_ALARM_CONFIGURATION_OPCODE               = 0x03,
   ONU_HPROT_GET_ALARM_CONFIGURATION_OPCODE               = 0x04,
   ONU_HPROT_GENERATE_OAM_EVENT_OPCODE                    = 0x05,
   ONU_HPROT_GET_STATISTIC_OPCODE                         = 0x06,
   ONU_HPROT_CLEAR_STATISTIC_OPCODE                       = 0x07,
   ONU_HPROT_SET_OAM_VENDOR_DATA_OPCODE                   = 0x0A,
   ONU_HPROT_GET_OAM_VENDOR_DATA_OPCODE                   = 0x0B,
   ONU_HPROT_SET_SLOW_PROTOCOL_LIMIT_OPCODE               = 0x0C,
   ONU_HPROT_GET_SLOW_PROTOCOL_LIMIT_OPCODE               = 0x0D,
   ONU_HPROT_SET_OAM_VERSION_OPCODE                       = 0x0E,
   ONU_HPROT_GET_OAM_VERSION_OPCODE                       = 0x0F,
   ONU_HPROT_SET_OAM_DYING_GASP_TIMES_OPCODE              = 0x10,
   ONU_HPROT_GET_OAM_DYING_GASP_TIMES_OPCODE              = 0x11,
   ONU_HPROT_GET_STATISTIC_LEAF_OPCODE                    = 0x12,
   ONU_HPROT_SET_MPCP_REPORT_FORMAT_OPCODE                = 0x13,
   ONU_HPROT_GET_MPCP_REPORT_FORMAT_OPCODE                = 0x14,
   ONU_HPROT_SET_MPCP_STANDARD_MODE_OPCODE                = 0x15,
   ONU_HPROT_GET_MPCP_STANDARD_MODE_OPCODE                = 0x16,
   ONU_HPROT_OAM_TRANSMIT_OPCODE                          = 0x17,
   ONU_HPROT_SET_SERVICE_DBA_OPCODE                       = 0x18,
   ONU_HPROT_GET_SERVICE_DBA_OPCODE                       = 0x19,
   ONU_HPROT_GET_SWAL_STATISTICS_OPCODE                   = 0x1A,
   ONU_HPROT_LAST_OAM_OPCODE                              = ONU_HPROT_GET_SWAL_STATISTICS_OPCODE
   																													
}  ONU_HPROT_OAM_opcode_enum_t;															
																														
typedef enum
{
   ONU_HPROT_SET_CONNECTION_OPCODE                        = 0x01,
   ONU_HPROT_GET_CONNECTION_OPCODE                        = 0x02,
   ONU_HPROT_GET_STATUS_OPCODE                            = 0x03,
   ONU_HPROT_GET_VERSION_OPCODE                           = 0x04,
   ONU_HPROT_GET_PON_CLK_OPCODE                           = 0x05,
   ONU_HPROT_GET_MAC_ADDRESS_OPCODE                       = 0x06,
   ONU_HPROT_SET_LED_CONFIGURATION_OPCODE                 = 0x07,
   ONU_HPROT_LASER_ON_ENABLE_OPCODE                       = 0x08,
   ONU_HPROT_GET_LASER_ON_ENABLE_OPCODE                   = 0x09,
   ONU_HPROT_GET_PON_LOS_SIGNAL_OPCODE                    = 0x0A,
   ONU_HPROT_ASSIGN_ALARM_HOOK_OPCODE                     = 0x0C,
   ONU_HPROT_SET_EVENT_CONFIGURATION_OPCODE               = 0x0D,
   ONU_HPROT_RESET_DEVICE_OPCODE                          = 0x0E,
   ONU_HPROT_DIAGNOSTIC_TEST_OPCODE                       = 0x0F,
   ONU_HPROT_DEBUG_PRINTF_OPCODE                          = 0x10,
   ONU_HPROT_EXTENDED_MTU_SIZE_OPCODE                     = 0x12,
   ONU_HPROT_GET_MTU_SIZE_OPCODE                          = 0x13,
   ONU_HPROT_SET_REBOOT_COUNTER_OPCODE                    = 0x14,
   ONU_HPROT_GET_REBOOT_COUNTER_OPCODE                    = 0x15,
   ONU_HPROT_UP_TIME_OPCODE                               = 0x16,
   ONU_HPROT_SERDES_SHUTDOWN_OPCODE                       = 0x17,
   ONU_HPROT_SET_SYSTEM_PARAMS_OPCODE                     = 0x18,
   ONU_HPROT_GET_SYSTEM_PARAMS_OPCODE                     = 0x19,
   ONU_HPROT_SET_PON_PAUSE_OPCODE                         = 0x1A,
   ONU_HPROT_GET_PON_PAUSE_OPCODE                         = 0x1B,
   ONU_HPROT_GET_OS_OPCODE                                = 0x1C,
   ONU_HPROT_LAST_DEVICE_OPCODE                           = ONU_HPROT_GET_OS_OPCODE

}  ONU_HPROT_device_opcode_enum_t;

typedef enum
{
   ONU_HPROT_EEPROM_WRITE_OPCODE                          = 0x01,
   ONU_HPROT_EEPROM_READ_OPCODE                           = 0x02,
   ONU_HPROT_EMAPPER_SET_OPCODE                           = 0x03,
   ONU_HPROT_EMAPPER_GET_OPCODE                           = 0x04,
   ONU_HPROT_MDIO_READ_OPCODE                             = 0x05,
   ONU_HPROT_WRITE_MDIO_DATA_OPCODE                       = 0x06,
   ONU_HPROT_I2C_READ_OPCODE                              = 0x07,
   ONU_HPROT_I2C_WRITE_OPCODE                             = 0x08,
   ONU_HPROT_SET_GPIO_LEVEL_OPCODE                        = 0x09,
   ONU_HPROT_SET_GPIO_DIRECTION_OPCODE                    = 0x0A,
   ONU_HPROT_GET_GPIO_OPCODE                              = 0x0B,
   ONU_HPROT_FLASH_MEMORY_BURN_OPCODE                     = 0x0C,
   ONU_HPROT_FLASH_MEMORY_ERASE_OPCODE                    = 0x0D,
   ONU_HPROT_FLASH_IMAGE_BURN_OPCODE                      = 0x0E,
   ONU_HPROT_IS_IMAGE_BURN_COMPLETE_OPCODE                = 0x0F,
   ONU_HPROT_SET_ACTIVE_IMAGE_OPCODE                      = 0x10,
   ONU_HPROT_GET_ACTIVE_IMAGE_OPCODE                      = 0x11,
   ONU_HPROT_SET_EXTMEM_WAITSTATE_OPCODE                  = 0x12,
   ONU_HPROT_GET_EXTMEM_WAITSTATE_OPCODE                  = 0x13,
   ONU_HPROT_EXTERNAL_MEMORY_WRITE_OPCODE                 = 0x14,
   ONU_HPROT_EXTERNAL_MEMORY_READ_OPCODE                  = 0x15,
   ONU_HPROT_SET_EXTMEM_BUS_WIDTH_OPCODE                  = 0x16,
   ONU_HPROT_GET_EXTMEM_BUS_WIDTH_OPCODE                  = 0x17,
   ONU_HPROT_FLASH_MEMORY_READ_OPCODE                     = 0x18,
   ONU_HPROT_SPI_READ_OPCODE                              = 0x19,
   ONU_HPROT_SPI_WRITE_OPCODE                             = 0x1A,
   ONU_HPROT_SPI_CFG_OPCODE                               = 0x1B,
   ONU_HPROT_I2C_READ_BUF_OPCODE                          = 0x1C,
   ONU_HPROT_I2C_WRITE_BUF_OPCODE                         = 0x1D,
   ONU_HPROT_GET_NVDB_INFO_OPCODE                         = 0x1E,
   ONU_HPROT_GET_IMAGE_INFO_OPCODE                        = 0x1F,
   ONU_HPROT_SET_ADC_CONFIG                               = 0x20,
   ONU_HPROT_GET_ADC_CONFIG                               = 0x21,
   ONU_HPROT_GET_ADC_SAMPLE                               = 0x22,
   ONU_HPROT_LAST_PERIPH_OPCODE                           = ONU_HPROT_GET_IMAGE_INFO_OPCODE

}  ONU_HPROT_periph_opcode_enum_t;

typedef enum
{
   ONU_HPROT_SET_802_1X_CONFIG_OPCODE                     = 0x01,
   ONU_HPROT_GET_802_1X_CONFIG_OPCODE                     = 0x02,
   ONU_HPROT_SET_802_1X_ENABLE_OPCODE                     = 0x03,
   ONU_HPROT_GET_802_1X_ENABLE_OPCODE                     = 0x04,
   ONU_HPROT_SET_802_1X_PARAMS_OPCODE                     = 0x05,
   ONU_HPROT_GET_802_1X_PARAMS_OPCODE                     = 0x06,
   ONU_HPROT_IGMP_CONFIGURATION_OPCODE                    = 0x07,
   ONU_HPROT_IGMP_CONFIGURATION_GET_OPCODE                = 0x08,
   ONU_HPROT_IGMP_ADD_FILTER_OPCODE                       = 0x09,
   ONU_HPROT_IGMP_REMOVE_FILTER_OPCODE                    = 0x0A,
   ONU_HPROT_GET_IGMP_FILTER_TABLE_OPCODE                 = 0x0B,
   ONU_HPROT_SET_IGMP_LAST_MEMBER_OPCODE                  = 0x0C,
   ONU_HPROT_GET_IGMP_LAST_MEMBER_OPCODE                  = 0x0D,
   /* ONU_HPROT_SET_GVRP_CONFIGURATION_OPCODE                = 0x0E, */ /* not used opcode */
   ONU_HPROT_SET_IGMP_PARAMS_OPCODE                       = 0x0F,
   ONU_HPROT_GET_IGMP_PARAMS_OPCODE                       = 0x10,
   ONU_HPROT_IGMP_ADD_VLAN_FILTER_OPCODE                  = 0x11,
   ONU_HPROT_IGMP_REMOVE_VLAN_FILTER_OPCODE               = 0x12,
   ONU_HPROT_GET_IGMP_VLAN_FILTER_TABLE_OPCODE            = 0x13,
   ONU_HPROT_MLD_SET_CONFIG_OPCODE                        = 0x14,
   ONU_HPROT_MLD_GET_CONFIG_OPCODE                        = 0x15,
   ONU_HPROT_MLD_ADD_FILTER_OPCODE                        = 0x16,
   ONU_HPROT_MLD_REMOVE_FILTER_OPCODE                     = 0x17,
   ONU_HPROT_MLD_REMOVE_HOST_OPCODE                       = 0x18,
   ONU_HPROT_MLD_GET_TABLE_OPCODE                         = 0x19,
   ONU_HPROT_MLD_SET_PARAM                                = 0x1A,
   ONU_HPROT_MLD_GET_PARAM                                = 0x1B,
   ONU_HPROT_IGMP_REMOVE_HOST_OPCODE                      = 0x1C,
   ONU_HPROT_IGMP_JOIN_FILTER_SET_CONFIG_OPCODE           = 0x1D,
   ONU_HPROT_IGMP_JOIN_FILTER_GET_CONFIG_OPCODE           = 0x1E,
   ONU_HPROT_IGMP_JOIN_FILTER_ADD_OPCODE                  = 0x1F,
   ONU_HPROT_IGMP_JOIN_FILTER_REMOVE_OPCODE               = 0x20,
   ONU_HPROT_IGMP_JOIN_FILTER_GET_TABLE_OPCODE            = 0x21,
   ONU_HPROT_IGMP_GET_GDA_HOST_TABLE_OPCODE               = 0x22,
   ONU_HPROT_IGMP_CLEAR_COUNTERS_OPCODE                   = 0x23,
   ONU_HPROT_IGMP_GET_COUNTER_OPCODE                      = 0x24,
   ONU_HPROT_IGMP_GET_SOURCE_ADDRESSES_OPCODE             = 0x25,
   ONU_HPROT_IGMP_GET_GROUP_TIME_INFO_OPCODE              = 0x26,
   ONU_HPROT_IGMP_JOIN_VID_FILTER_ADD_OPCODE              = 0x27,
   ONU_HPROT_IGMP_JOIN_VID_FILTER_REMOVE_OPCODE           = 0x28,
   ONU_HPROT_IGMP_JOIN_VID_FILTER_GET_TABLE_OPCODE        = 0x29,
   ONU_HPROT_LAST_PROTOCOL_OPCODE                         = ONU_HPROT_IGMP_JOIN_VID_FILTER_GET_TABLE_OPCODE

}  ONU_HPROT_PROTOCOL_opcode_enum_t;															

typedef enum
{
    REMOTE_SWAL_GET_VERSION				        = 0x0000,
    REMOTE_SWAL_SET_PORT_LOOPBACK_MODE           = 0x0001,
    REMOTE_SWAL_GET_PORT_LOOPBACK_MODE  		    = 0x0002,
    REMOTE_SWAL_GET_STATISTICS_PER_PORT			= 0x0003,
    REMOTE_SWAL_SET_PORT_MAC_LIMIT				= 0x0004,
    REMOTE_SWAL_GET_PORT_MAC_LIMIT				= 0x0005,
    REMOTE_SWAL_SET_PORT_CONFIGURATION			= 0x0006,
    REMOTE_SWAL_GET_PORT_CONFIGURATION			= 0x0007,
    REMOTE_SWAL_GET_PORT_STATUS              	= 0x0008,
    REMOTE_SWAL_CLEAR_ALL_STATS              	= 0x0009,
    REMOTE_SWAL_SET_PORT_EXT_CONFIG                          = 0x000A,
    REMOTE_SWAL_GET_PORT_EXT_CONFIG                          = 0x000B,
    REMOTE_SWAL_GET_STP_PORT_STATE                           = 0x000F,
    REMOTE_SWAL_SET_QUEUE_SCHEDULING_MODE                    = 0x0014,
    REMOTE_SWAL_GET_QUEUE_SCHEDULING_MODE                    = 0x0015,
    REMOTE_SWAL_SET_ADV_QUEUE_SCHEDULING                     = 0x0016,
    REMOTE_SWAL_GET_ADV_QUEUE_SCHEDULING                     = 0x0017,
    REMOTE_SWAL_SET_UNKNOWN_DA_FWD_MODE          = 0x00018,
    REMOTE_SWAL_GET_UNKNOWN_DA_FWD_MODE          = 0x00019,
    REMOTE_SWAL_SET_MAX_FRAME_SIZE                           = 0x001A,
    REMOTE_SWAL_GET_MAX_FRAME_SIZE                           = 0x001B
}  REMOTE_SWAL_opcode_enum_t;

typedef enum
{
   ONU_HPROT_EVENT_ALARM_OPCODE                  = 0x01

} ONU_HPROT_Event_opcode_t;


#define APPLICATION_ID_MASK			0x0000ffff

#define MAKE_OPCODE_WORD( group, specific_opcode ) (APPLICATION_ID_MASK & ((group * 0x100) + specific_opcode) );


typedef enum
{
   REMOTE_MANAGEMENT_RSTP_LOOP_DETECT_EVENT_OPCODE	= 0x0001
}  REMOTE_MANAGEMENT_events_opcode_enum_t;

#define RSTP_EVENT_APPLICATION_ID_MASK	0x00030000

#define MAKE_RSTP_EVENT_OPCODE_WORD( app_id, specific_opcode ) (app_id | (specific_opcode) )



/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/

#define REMOTE_COMMUNICATION_EXIT_OK				        EXIT_OK
#define REMOTE_COMMUNICATION_EXIT_ERROR			            EXIT_ERROR
#define REMOTE_COMMUNICATION_TIME_OUT				        -2 
#define REMOTE_COMMUNICATION_NOT_IMPLEMENTED		        -3
#define REMOTE_COMMUNICATION_PARAMETER_ERROR		        -4
#define REMOTE_COMMUNICATION_HARDWARE_ERROR		            -5
#define REMOTE_COMMUNICATION_MEMORY_ERROR			        -6
#define REMOTE_COMMUNICATION_ILLEGAL_SIZE                   (-14001)
#define REMOTE_COMMUNICATION_WRONG_RESPONSE_MESSAGE_ID      (-14002)
#define REMOTE_COMMUNICATION_WRONG_RESPONSE_PDU_TYPE        (-14003)
#define REMOTE_COMMUNICATION_WRONG_RESPONSE_OPCODE_ID       (-14004)
#define REMOTE_COMMUNICATION_NULL_POINTER                   (-14005)
#define REMOTE_COMMUNICATION_NO_RESOURCES                   (-14006)          
#define REMOTE_COMMUNICATION_VLAN_MODE_NOT_CONFIGURED       (-14007)
#define REMOTE_COMMUNICATION_BAD_CONFIGURATION              (-14008)
#define REMOTE_COMMUNICATION_DISCARD_OAM                    (-14009)
#define REMOTE_COMMUNICATION_ALREADY_EXISTS                 (-14010)
#define REMOTE_COMMUNICATION_ILLEGAL_ADDRESS                (-14011)
#define REMOTE_COMMUNICATION_MISMATCH                       (-14012)
#define REMOTE_COMMUNICATION_OLT_NOT_EXIST			        (-14013)
#define REMOTE_COMMUNICATION_QUERY_FAILED			        (-14014)
#define REMOTE_COMMUNICATION_ONU_NOT_AVAILABLE		        (-14015)


/* timings settings */
#define PON_CLOCKS_PER_SEC              TQ_IN_A_SECOND  
#define REMOTE_MANAGEMENT_TIMEOUT       3         /* seconds */

#define ETHERNET_HEADER_PLACE           0
#if 0
#define OAM_HEADER_PLACE                14
#define LLC_HEADER_PLACE                22
#define APPLICATION_HEADER_PLACE        28
#else
#define OAM_HEADER_PLACE                0
#define LLC_HEADER_PLACE                4
#define APPLICATION_HEADER_PLACE        10
#endif

#define LLC_HEADER_SIZE                 6
#define MSG_ID_PLACE                    0
#define PDU_TYPE_PLACE                  4

#define OAM_HEADER_SIZE             8
#define OAM_SUB_TYPE_PLACE          0
#define OAM_FLAGS_PLACE             1
#define OAM_CODE_PLACE              3
#define OAM_OUI_PLACE               4
#define OAM_MAGIC_NUMBER_PLACE      7
#define OAM_LEGACY_NUMBER           0xFF
 

#define MAX_OAM_PDU_SIZE                        1514 /* The length (in bytes) of an Ethernet MTU */


/* Remote managment PASLOG functionality */
#define  PONLOG_FUNC_REMOTE_MGMT_COMM 			"remote-mgmt-comm" /* Remote Management package communication layer messages	*/


#if 1
/* Application Layer */

short int PON_PAS_APPLICATIONS_init();

short int PON_PAS_APPLICATIONS_terminate();


#ifndef PAS_ONU_VER_T
#define PAS_ONU_VER_T

typedef struct 
{
    PON_onu_hardware_version_t   hardware_version;              /* ONU hardware version */
    short int                    fw_remote_mgnt_major_version;  /* FW remote management major version */
    short int                    fw_remote_mgnt_minor_version;  /* FW remote management minor version */
    OAM_standard_version_t       oam_version;                   /* ONU OAM version */
}PON_onu_versions;
#endif

short int PON_PAS_APPLICATIONS_get_onu_deviceid 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_hardware_version_t  *device_id );

short int PON_PAS_APPLICATIONS_get_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_versions  *device_versions );
short int PON_PAS_APPLICATIONS_set_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PON_onu_versions  *device_versions );

void PON_PAS_APPLICATIONS_clr_onu_infos 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id );
void PON_PAS_APPLICATIONS_clr_olt_infos( const PON_olt_id_t  	olt_id );

unsigned short PON_PAS_APPLICATIONS_get_oam_frame_size(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id );
void PON_PAS_APPLICATIONS_set_oam_frame_size(
                      const PON_olt_id_t     olt_id, 
                      const PON_onu_id_t     onu_id,
					  const unsigned short   maximum_data_frame_size );

bool PON_PAS_APPLICATIONS_get_oam_frame_size_valid(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id );
void PON_PAS_APPLICATIONS_set_oam_frame_size_valid(
                      const PON_olt_id_t   olt_id, 
                      const PON_onu_id_t   onu_id,
					  const bool		   oam_negotiation_field_is_valid );

short int PON_PAS_APPLICATIONS_send_receive_message  ( 
                      const PON_olt_id_t      olt_id, 
                      const PON_onu_id_t      onu_id,
                      unsigned char          *sent_data,  
                      const unsigned short    sent_data_size,
                      unsigned char          *received_data,
                      unsigned short         *received_data_size,
                      unsigned short          allocated_received_data_size);

short int PON_PAS_Adaptation_layer_assign_registration( const void	(*registration_function)());
short int PON_PAS_Adaptation_layer_terminate_registration( const void	(*registration_function)());

short int PON_PAS_Adaptation_layer_assign_deregistration( const void	(*deregistration_function)());
short int PON_PAS_Adaptation_layer_terminate_deregistration( const void	(*deregistration_function)());

#endif



#if 1
/* Express Layer */

typedef enum
{
  COMPOSER_STATUS_OK                        = S_OK,
  COMPOSER_STATUS_ERROR                     = S_ERROR,
  COMPOSER_STATUS_INVALID                   = S_INVALID,
  COMPOSER_STATUS_FAIL                      = S_FAIL,
  COMPOSER_STATUS_NOT_SUPPORTED             = S_NOT_SUPPORTED,
  COMPOSER_STATUS_NOT_INITIALIZED           = S_NOT_INITIALIZED,
  COMPOSER_STATUS_OUT_OF_RANGE              = S_OUT_OF_RANGE,
  COMPOSER_STATUS_BAD_PARAM                 = S_BAD_PARAM,
  COMPOSER_STATUS_BAD_CONFIGURATION         = S_BAD_CONFIGURATION,
  COMPOSER_STATUS_NO_RESOURCES              = S_NO_RESOURCES,
  COMPOSER_STATUS_NOT_FOUND                 = S_NOT_FOUND,
  COMPOSER_STATUS_ALREADY_EXISTS            = S_ALREADY_EXISTS,
  COMPOSER_STATUS_ILLEGAL_ADDRESS           = S_ILLEGAL_ADDRESS,
  COMPOSER_STATUS_NO_MEMORY                 = S_NO_MEMORY,
  COMPOSER_STATUS_DISCARD                   = S_DISCARD,
  COMPOSER_STATUS_MISMATCH                  = S_MISMATCH,
  COMPOSER_STATUS_WRONG_NUM_PARAMS          = S_WRONG_NUM_PARAMS,
  COMPOSER_STATUS_OS_ERROR                  = S_OS_ERROR,
  COMPOSER_STATUS_OVERFLOW                  = S_OVERFLOW,
  COMPOSER_STATUS_TIMEOUT                   = S_TIMEOUT,
  COMPOSER_STATUS_FUTURE_USE                = S_FUTURE_USE,
  COMPOSER_STATUS_WRONG_OPCODE              = S_LAST_STATUS,
} COMPOSER_status_t;


typedef enum
{
    PON_PORT,
    UNI_PORT,
    PON_UNI
} PASONU_port_t;

typedef enum
{
    PASONU_DOWNSTREAM = 0,
    PASONU_UPSTREAM = 1
} PASONU_direction_t;

typedef enum
{
    DISCONNECT,
    CONNECT
} PASONU_connection_t;

typedef enum
{
    UNI_HOST,
    UART_HOST,
    MII_HOST
} PASONU_HOST_t;

typedef enum
{
   PASONU_DONT_PASS,
   PASONU_PASS_DATAPATH,
   PASONU_PASS_CPU,
   PASONU_PASS_BOTH
} PASONU_forwarding_action_t;


#define MSG_STRING_MAX_SIZE      100


#define CHECK_COMM_RESULT_AND_RETURN \
    if ( result != REMOTE_COMMUNICATION_EXIT_OK ) \
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result; \
}

#define CHECK_COMPOSER_RESULT_AND_RETURN \
{   result = Convert_composer_error_code_to_remote_management(composer_result);\
    if ( result != REMOTE_PASONU_EXIT_OK )\
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result;\
}\
}

#define CHECK_OPCODE_AND_RETURN \
{   if( received_opcode != expected_opcode )\
    {\
      PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Unexpected opcode: received-0x%lx,expected-0x%lx \n", received_opcode,expected_opcode );\
      return COMPOSER_STATUS_WRONG_OPCODE;\
    }\
}

#define CHECK_ONU_RESULT_AND_RETURN \
{   if( result != 0 )\
    {\
      PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "received error code %d\n", result );\
      return result;\
    }\
}

#define CHECK_ONU_NOT_SUPPORTED_RESULT \
{   if( result != 0 )\
    {\
      if( result != S_NOT_SUPPORTED )\
      {\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "received error code %d\n", result );\
      }\
     return result;\
    }\
}

#define CHECK_BUFFER_SIZE_AND_RETURN( offset, total_length, buffer, size_to_write )\
{\
    if((offset + size_to_write - buffer) > total_length)\
    {\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "failed to compose command. Allocated buffer is too small %d",  total_length)\
        return COMPOSER_STATUS_NO_MEMORY;\
    }\
}


#define FILL_ULONG_LONG_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, 2*BYTES_IN_LONG)\
    ULONG_LONG_2_ENDIANITY_UBUFFER( x, offset )\
    offset+= 2*BYTES_IN_LONG;\
}

#define FILL_ULONG_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    ULONG_2_ENDIANITY_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_LONG;\
}

#define FILL_USHORT_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    ULONG_2_ENDIANITY_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_LONG;\
}

#define FILL_UCHAR_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    ULONG_2_ENDIANITY_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_LONG;\
}


#define FILL_BOOL_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    ULONG_2_ENDIANITY_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_LONG;\
}

#define FILL_MAC_ADDRESS_IN_BUFFER( x, offset, length, buffer )\
{ \
    memset(offset,0,2*BYTES_IN_LONG);\
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, 2*BYTES_IN_LONG)\
    PON_MAC_ADDRESS_COPY( offset, x )\
    offset+= 2*BYTES_IN_LONG;\
}

#define EXTRACT_OPCODE_AND_RESULT_RETURN(receive_pdu_data,received_opcode,result, length, buffer) \
{\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, received_opcode, length, buffer)\
    CHECK_OPCODE_AND_RETURN\
    EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER(receive_pdu_data, result, length, buffer)\
    CHECK_ONU_RESULT_AND_RETURN\
}

#define EXTRACT_OPCODE_AND_RESULT_CHECK(receive_pdu_data,received_opcode,result, length, buffer) \
{\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER(receive_pdu_data, received_opcode, length, buffer)\
    CHECK_OPCODE_AND_RETURN\
    EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER(receive_pdu_data, result, length, buffer)\
    CHECK_ONU_NOT_SUPPORTED_RESULT\
}

#define EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, 2*BYTES_IN_LONG)\
    UBUFFER_2_ENDIANITY_ULONG_LONG( offset, x )\
    offset+= 2*BYTES_IN_LONG;\
}

#define EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    UBUFFER_2_ENDIANITY_ULONG( offset, x )\
    offset+= BYTES_IN_LONG;\
}



#define EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    UBUFFER_2_ENDIANITY_USHORT( offset, x )\
    offset+= BYTES_IN_LONG;\
}


#define EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    WORD_UBUFFER_2_ENDIANITY_UCHAR( offset, x )\
    offset+= BYTES_IN_LONG;\
}


#define EXTRACT_BOOL_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_LONG)\
    ENDIANITY_WORD_UBUFFER_2_BOOL( offset, x )\
    offset+= BYTES_IN_LONG;\
}

#define EXTRACT_MAC_ADDRESS_IN_BUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, 2*BYTES_IN_LONG)\
    PON_MAC_ADDRESS_COPY( x, offset )\
    offset+= 2*BYTES_IN_LONG;\
}

short int Convert_comm_error_code_to_remote_management( short int result);
short int Convert_composer_error_code_to_remote_management( short int result);

void RmDecompose_print(char *string_buffer);

COMPOSER_status_t Decompose_general_ack ( 
                           const unsigned long   expected_opcode,  
                           const unsigned char  *receive_buffer,
                           const unsigned short  length);


typedef enum
{
    PASONU_OAM_AUTO,
    PASONU_OAM_DRAFT_2_0,
    PASONU_OAM_STANDARD
} PASONU_oam_version_t;

typedef PACK_START struct PASONU_version_s
{
    INT16U hw_major_ver;
    INT16U hw_minor_ver;
    INT16U fw_major_ver;
    INT16U fw_minor_ver;
    INT16U fw_build_num;
    INT16U fw_maintenance_ver;
    PASONU_oam_version_t OAM_version;
} PACK_END PASONU_version_t;

COMPOSER_status_t Compose_get_version (
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_get_version ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/PASONU_version_t     *versions);
short int Onu_to_olt_converstion_device_version
                ( const PASONU_version_t  *onu_versions,
                  PON_device_version_t    *olt_versions );


typedef PACK_START struct
{
    PASONU_connection_t connection;
    BOOLEAN oam_link_established;
    BOOLEAN authorization_state;
    BOOLEAN PON_Loopback;
    GENERAL_mac_addr_t mac_addr;
    INT16U  ONU_llid;
} PACK_END PASONU_status_t;

COMPOSER_status_t Compose_get_status (
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_get_status ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/PASONU_status_t      *status);
short int Onu_to_olt_converstion_device_status
                ( const PASONU_status_t    *onu_status,
                  PON_onu_device_status_t  *olt_status );


COMPOSER_status_t Compose_set_slow_protocol_limit (
                  /*IN*/    BOOLEAN          enable,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);


COMPOSER_status_t Compose_get_slow_protocol_limit (
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_get_slow_protocol_limit ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/BOOLEAN              *enable );


COMPOSER_status_t Compose_get_oam_version (
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_get_oam_version ( 
                 /*IN*/ const unsigned char   *receive_buffer,
                 /*IN*/ const unsigned short   length,
                 /*OUT*/PASONU_oam_version_t  *oam_version );
short int Onu_to_olt_converstion_oam_version
                ( const PASONU_oam_version_t   onu_version,
                  OAM_standard_version_t      *olt_version );


COMPOSER_status_t Compose_oam_get_length_negotiation (
                  /*OUT*/   unsigned char               *sent_pdu_data,
                  /*IN_OUT*/unsigned short              *length);
COMPOSER_status_t Decompose_oam_get_length_negotiation ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/INT16U               *oam_frame_length );


typedef enum
{
    PASONU_RESET_NONE,
    PASONU_RESET_SW,
    PASONU_RESET_SW_ERROR,
    PASONU_RESET_PERIPH_LOW,
    PASONU_RESET_PERIPH_HIGH
} PASONU_reset_reason_t;

COMPOSER_status_t Compose_reset_device ( 
                 /*IN*/    const PASONU_reset_reason_t   reason,
                 /*OUT*/   unsigned char                *sent_pdu_data,
                 /*IN_OUT*/unsigned short                *length);


#define PASONU_ALARM_OFFSET             0x0
#define PASONU_EVENT_OFFSET             0x8000

typedef enum
{
    ERRORED_SYMBOLS_PERIOD_ALARM = PASONU_ALARM_OFFSET,
    ERRORED_FRAME_ALARM          = PASONU_ALARM_OFFSET + 1,
    ERRORED_FRAME_PERIOD_ALARM   = PASONU_ALARM_OFFSET + 2,
    ERRORED_SECOND_SUMMURY_ALARM = PASONU_ALARM_OFFSET + 3,
    DYING_GASP_ALARM             = PASONU_ALARM_OFFSET + 4,
    LINK_FAULT_ALARM             = PASONU_ALARM_OFFSET + 5,
    CRITICAL_EVENT_ALARM         = PASONU_ALARM_OFFSET + 6,
    /* add alrams before MAX_ALARM_TYPE */
    MAX_ALARM_TYPE,

    IGMP_JOIN_EVENT              = PASONU_EVENT_OFFSET,
    IGMP_LEAVE_EVENT             = PASONU_EVENT_OFFSET + 1,
    ENCRYPTION_KEY_EVENT         = PASONU_EVENT_OFFSET + 2,
    ENCRYPTION_STATE_EVENT       = PASONU_EVENT_OFFSET + 3,
    FEC_RX_MODE_EVENT            = PASONU_EVENT_OFFSET + 4,
    ADDRESS_TBL_FULL_EVENT       = PASONU_EVENT_OFFSET + 5,
    CONNECTION_STATE_EVENT       = PASONU_EVENT_OFFSET + 6,
    UNI_LINK_EVENT               = PASONU_EVENT_OFFSET + 7,
    PON_LOOPBACK_EVENT           = PASONU_EVENT_OFFSET + 8,
    AUTHORIZATION_EVENT          = PASONU_EVENT_OFFSET + 9,
    PRE_RESET_EVENT              = PASONU_EVENT_OFFSET + 10,
    IMAGE_BURN_COMPLETE_EVENT    = PASONU_EVENT_OFFSET + 11,
    EAPOL_RESPONSE_SENT_EVENT    = PASONU_EVENT_OFFSET + 12,
    PASONU_RESERVED_EVENT_1      = PASONU_EVENT_OFFSET + 13,
    PASONU_RESERVED_EVENT_2      = PASONU_EVENT_OFFSET + 14,
    MPCP_NACK_EVENT              = PASONU_EVENT_OFFSET + 15,
    MLD_JOIN_EVENT               = PASONU_EVENT_OFFSET + 16,
    MLD_DONE_EVENT               = PASONU_EVENT_OFFSET + 17,
    HOST_TIMEOUT_EVENT           = PASONU_EVENT_OFFSET + 18,
    OAM_RECEIVE_EVENT            = PASONU_EVENT_OFFSET + 19,
    GPIO_IRQ_EVENT               = PASONU_EVENT_OFFSET + 20,
    /* add events before MAX_EVENT_TYPE */
    MAX_EVENT_TYPE
} PASONU_alarm_event_type;

extern COMPOSER_status_t Compose_set_alarm_configuration (
                      const PASONU_alarm_event_type   type,
                      const BOOLEAN                   enable_local_monitoring,
                      const INT64U                    window,
                      const INT64U                    threshold,
                      unsigned char                  *sent_pdu_data,
                      unsigned short                 *length);
extern COMPOSER_status_t Compose_get_alarm_configuration (
                      const PASONU_alarm_event_type   type,
                      unsigned char                  *sent_pdu_data,
                      unsigned short                 *length);
extern COMPOSER_status_t Decompose_get_alarm_configuration ( 
                           const unsigned char  *receive_buffer,
                           const unsigned short  length,
                           BOOLEAN              *enable_local_monitoring,
                           INT64U               *window,
                           INT64U               *threshold );
short int Olt_to_onu_converstion_event
                ( const PON_alarm_event_type   type,
                  PASONU_alarm_event_type     *onu_type );


COMPOSER_status_t Compose_clear_all_statistics (
				  /*IN*/	const BOOLEAN    multi_port,
                  /*IN*/    const INT8U      lport,	/*for multi_port only*/
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);


#define PON_MIN_ONU_EEPROM_REGISTER_ADDRESS  0   /* Min EEPROM register number				   */
#define PON_MAX_ONU_EEPROM_REGISTER_ADDRESS  63  /* PAS6001 - A/B/N Max EEPROM register number */

COMPOSER_status_t Compose_eeprom_write (
                  /*IN*/    const INT16U     address,
                  /*IN*/    const INT16U     data,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);

COMPOSER_status_t Compose_eeprom_read (
                  /*IN*/    const INT16U     address,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_eeprom_read ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/INT16U               *data);

typedef enum
{
    EMAPPER_COOKIE_ID = 0,
    EMAPPER_EXT_RAM_READ_WS = 1,
    EMAPPER_EXT_RAM_WRITE_WS = 2,
    EMAPPER_EXT_RAM_SIZE = 3,
    EMAPPER_FIRST_FW_START_ADDR = 4,
    EMAPPER_SECOND_FW_START_ADDR = 5,
    EMAPPER_BOOT_MONITOR_START_ADDR = 6,
    EMAPPER_MANAGE_SECS_START_ADDR = 7,
    EMAPPER_EXT_FLASH_READ_WS = 8,
    EMAPPER_EXT_FLASH_WRITE_WS = 9,
    EMAPPER_FLASH_VENDOR = 10,
    EMAPPER_SDRAM_INFO_INDEX = 11,
    EMAPPER_SDRAM_REFRESH_CYCLE = 12,
    EMAPPER_SDRAM_TCAS = 13,
    EMAPPER_SDRAM_TRAS = 14,
    EMAPPER_SDRAM_WIDTH_MODE = 15,
    EMAPPER_SDRAM_RBC_OR_BRC = 16,
    EMAPPER_EXT_RAM_TYPE_IN_USE = 20,
    EMAPPER_BOOT_LOADER_MODE = 21,
    EMAPPER_UART_BAUD_INDEX = 22,
    EMAPPER_PON_LOSS_SIGNAL_POLARITY = 23,
    EMAPPER_PON_TBC_SIGNAL_POLARITY = 24,
    EMAPPER_LASER_TX_ENABLE_POLARITY = 25,
    EMAPPER_ZBT_USE = 28,
    EMAPPER_ZBT_CONFIG = 29,
    EMAPPER_PON_CONNECT_ON_POWER_UP = 30,
    EMAPPER_OUI_ADDR = 31,
    EMAPPER_EEPROM_MAC_ADDR = 32,
    EMAPPER_EEPROM_IP_ADDR = 33,
    EMAPPER_EEPROM_NET_MASK = 34,
    EMAPPER_LASER_TON = 35,
    EMAPPER_LASER_TOFF = 36,
    EMAPPER_UNI_PHY_ADVERTISING_ENABLE = 37,
    EMAPPER_UNI_BRIDGE_ENABLE = 38,
    EMAPPER_UNI_AUTONEG_ENABLE = 39,
    EMAPPER_UNI_MDIO_EXTERN_PHY_ADDR = 40,
    EMAPPER_UNI_MAC_TYPE = 41,
    EMAPPER_UNI_MASTER_MODE = 42,
    EMAPPER_UNI_ADVERTISE_1000T_MULTI_PORT = 43,
    EMAPPER_UNI_ADVERTISE_1000T_HALF_DUPLEX = 44,
    EMAPPER_UNI_ADVERTISE_1000T_FULL_DUPLEX = 45,
    EMAPPER_UNI_ADVERTISE_PAUSE_ASYMMETRIC = 46,
    EMAPPER_UNI_ADVERTISE_PAUSE_ENABLED = 47,
    EMAPPER_UNI_ADVERTISE_100T4 = 48,
    EMAPPER_UNI_ADVERTISE_100TX_FD = 49,
    EMAPPER_UNI_ADVERTISE_100TX_HD = 50,
    EMAPPER_UNI_ADVERTISE_10TX_FD = 51,
    EMAPPER_UNI_ADVERTISE_10TX_HD = 52,
    EMAPPER_THRESHOLD_MODE = 53,
    EMAPPER_UNIFY_THRESHOLD_MODE = 54,
    EMAPPER_POINT_TO_POINT_ENABLE = 55,
    EMAPPER_GRANTED_ALWAYS = 56,
    EMAPPER_LASER_ON_PERMANENTLY = 57,
    EMAPPER_PON_TX_DISABLE_DATA_FORMAT = 58,
    EMAPPER_IDLE_BYTE0 = 59,
    EMAPPER_IDLE_BYTE1 = 60,
    EMAPPER_IDLE_BYTE2 = 61,
    EMAPPER_IDLE_BYTE3 = 62,
    EMAPPER_IDLE_PREAMBLE_DATA = 63,
    EMAPPER_USER_NAME_802_1X = 64,
    EMAPPER_PASSWORD_802_1X = 65,
    EMAPPER_GENERAL_PURPOSE_FIELD1 = 66,
    EMAPPER_GENERAL_PURPOSE_FIELD2 = 67,
    EMAPPER_PON_CLOCK_FINE_TUNE = 68,
    EMAPPER_TIMESTAMP_DELAY_FEC = 69,
    EMAPPER_ARB_PON_TIMESTAMP_DELTA = 70,
    EMAPPER_PON_CLK_CALIB_TX = 71,
    EMAPPER_PON_CLK_CALIB_RX = 72,
    EMAPPER_DYING_GASP_POLARITY = 73,
    EMAPPER_LINK_FAULT_POLARITY = 74,
    EMAPPER_CRITICAL_EVENT_POLARITY = 75,
    EMAPPER_BOOT_LOADER_REBOOT_SWITCH_COUNT = 76,
    EMAPPER_HMII_MODE = 77,
    EMAPPER_LAST_SYSTEM_PARAM,
    EMAPPER_USER_PARAM_0 = 0x200,
    EMAPPER_USER_PARAM_1,
    EMAPPER_USER_PARAM_2,
    EMAPPER_USER_PARAM_3,
    EMAPPER_USER_PARAM_4,
    EMAPPER_USER_PARAM_5,
    EMAPPER_USER_PARAM_6,
    EMAPPER_USER_PARAM_7,
    EMAPPER_USER_PARAM_8,
    EMAPPER_USER_PARAM_9,
    EMAPPER_USER_PARAM_10,
    EMAPPER_USER_PARAM_11,
    EMAPPER_USER_PARAM_12,
    EMAPPER_USER_PARAM_13,
    EMAPPER_USER_PARAM_14,
    EMAPPER_USER_PARAM_15,
    EMAPPER_USER_PARAM_16,
    EMAPPER_USER_PARAM_17,
    EMAPPER_USER_PARAM_18,
    EMAPPER_USER_PARAM_19,
    EMAPPER_USER_PARAM_20,
    EMAPPER_USER_PARAM_21,
    EMAPPER_USER_PARAM_22,
    EMAPPER_USER_PARAM_23,
    EMAPPER_USER_PARAM_24,
    EMAPPER_USER_PARAM_25,
    EMAPPER_USER_PARAM_26,
    EMAPPER_USER_PARAM_27,
    EMAPPER_USER_PARAM_28,
    EMAPPER_USER_PARAM_29,
    EMAPPER_USER_PARAM_30,
    EMAPPER_USER_PARAM_31,
    EMAPPER_USER_PARAM_32,
    EMAPPER_USER_PARAM_33,
    EMAPPER_USER_PARAM_34,
    EMAPPER_USER_PARAM_35,
    EMAPPER_USER_PARAM_36,
    EMAPPER_USER_PARAM_37,
    EMAPPER_USER_PARAM_38,
    EMAPPER_USER_PARAM_39,
    EMAPPER_USER_PARAM_40,
    EMAPPER_USER_PARAM_41,
    EMAPPER_USER_PARAM_42,
    EMAPPER_USER_PARAM_43,
    EMAPPER_USER_PARAM_44,
    EMAPPER_USER_PARAM_45,
    EMAPPER_USER_PARAM_46,
    EMAPPER_USER_PARAM_47,
    EMAPPER_USER_PARAM_48,
    EMAPPER_USER_PARAM_49,
    EMAPPER_USER_PARAM_50,
    EMAPPER_USER_PARAM_51,
    EMAPPER_USER_PARAM_52,
    EMAPPER_USER_PARAM_53,
    EMAPPER_USER_PARAM_54,
    EMAPPER_USER_PARAM_55,
    EMAPPER_USER_PARAM_56,
    EMAPPER_USER_PARAM_57,
    EMAPPER_USER_PARAM_58,
    EMAPPER_USER_PARAM_59,
    EMAPPER_USER_PARAM_60,
    EMAPPER_USER_PARAM_61,
    EMAPPER_USER_PARAM_62,
    EMAPPER_USER_PARAM_63,
    EMAPPER_USER_PARAM_64,
    EMAPPER_USER_PARAM_65,
    EMAPPER_USER_PARAM_66,
    EMAPPER_USER_PARAM_67,
    EMAPPER_USER_PARAM_68,
    EMAPPER_USER_PARAM_69,
    EMAPPER_USER_PARAM_70,
    EMAPPER_USER_PARAM_71,
    EMAPPER_USER_PARAM_72,
    EMAPPER_USER_PARAM_73,
    EMAPPER_USER_PARAM_74,
    EMAPPER_USER_PARAM_75,
    EMAPPER_USER_PARAM_76,
    EMAPPER_USER_PARAM_77,
    EMAPPER_USER_PARAM_78,
    EMAPPER_USER_PARAM_79,
    EMAPPER_USER_PARAM_80,
    EMAPPER_USER_PARAM_81,
    EMAPPER_USER_PARAM_82,
    EMAPPER_USER_PARAM_83,
    EMAPPER_USER_PARAM_84,
    EMAPPER_USER_PARAM_85,
    EMAPPER_USER_PARAM_86,
    EMAPPER_USER_PARAM_87,
    EMAPPER_USER_PARAM_88,
    EMAPPER_USER_PARAM_89,
    EMAPPER_USER_PARAM_90,
    EMAPPER_USER_PARAM_91,
    EMAPPER_USER_PARAM_92,
    EMAPPER_USER_PARAM_93,
    EMAPPER_USER_PARAM_94,
    EMAPPER_USER_PARAM_95,
    EMAPPER_USER_PARAM_96,
    EMAPPER_USER_PARAM_97,
    EMAPPER_USER_PARAM_98,
    EMAPPER_USER_PARAM_99,
    EMAPPER_USER_PARAM_100,
    EMAPPER_USER_PARAM_101,
    EMAPPER_USER_PARAM_102,
    EMAPPER_USER_PARAM_103,
    EMAPPER_USER_PARAM_104,
    EMAPPER_USER_PARAM_105,
    EMAPPER_USER_PARAM_106,
    EMAPPER_USER_PARAM_107,
    EMAPPER_USER_PARAM_108,
    EMAPPER_USER_PARAM_109,
    EMAPPER_USER_PARAM_110,
    EMAPPER_USER_PARAM_111,
    EMAPPER_USER_PARAM_112,
    EMAPPER_USER_PARAM_113,
    EMAPPER_USER_PARAM_114,
    EMAPPER_USER_PARAM_115,
    EMAPPER_USER_PARAM_116,
    EMAPPER_USER_PARAM_117,
    EMAPPER_USER_PARAM_118,
    EMAPPER_USER_PARAM_119,
    EMAPPER_USER_PARAM_120,
    EMAPPER_USER_PARAM_121,
    EMAPPER_USER_PARAM_122,
    EMAPPER_USER_PARAM_123,
    EMAPPER_USER_PARAM_124,
    EMAPPER_USER_PARAM_125,
    EMAPPER_USER_PARAM_126,
    EMAPPER_USER_PARAM_127,
    EMAPPER_USER_PARAM_128,
    EMAPPER_USER_PARAM_129,
    EMAPPER_USER_PARAM_130,
    EMAPPER_USER_PARAM_131,
    EMAPPER_USER_PARAM_132,
    EMAPPER_USER_PARAM_133,
    EMAPPER_USER_PARAM_134,
    EMAPPER_USER_PARAM_135,
    EMAPPER_USER_PARAM_136,
    EMAPPER_USER_PARAM_137,
    EMAPPER_USER_PARAM_138,
    EMAPPER_USER_PARAM_139,
    EMAPPER_USER_PARAM_140,
    EMAPPER_USER_PARAM_141,
    EMAPPER_USER_PARAM_142,
    EMAPPER_USER_PARAM_143,
    EMAPPER_USER_PARAM_144,
    EMAPPER_USER_PARAM_145,
    EMAPPER_USER_PARAM_146,
    EMAPPER_USER_PARAM_147,
    EMAPPER_USER_PARAM_148,
    EMAPPER_USER_PARAM_149,
    EMAPPER_USER_PARAM_150,
    EMAPPER_USER_PARAM_151,
    EMAPPER_USER_PARAM_152,
    EMAPPER_USER_PARAM_153,
    EMAPPER_USER_PARAM_154,
    EMAPPER_USER_PARAM_155,
    EMAPPER_USER_PARAM_156,
    EMAPPER_USER_PARAM_157,
    EMAPPER_USER_PARAM_158,
    EMAPPER_USER_PARAM_159,
    EMAPPER_USER_PARAM_160,
    EMAPPER_USER_PARAM_161,
    EMAPPER_USER_PARAM_162,
    EMAPPER_USER_PARAM_163,
    EMAPPER_USER_PARAM_164,
    EMAPPER_USER_PARAM_165,
    EMAPPER_USER_PARAM_166,
    EMAPPER_USER_PARAM_167,
    EMAPPER_USER_PARAM_168,
    EMAPPER_USER_PARAM_169,
    EMAPPER_USER_PARAM_170,
    EMAPPER_USER_PARAM_171,
    EMAPPER_USER_PARAM_172,
    EMAPPER_USER_PARAM_173,
    EMAPPER_USER_PARAM_174,
    EMAPPER_USER_PARAM_175,
    EMAPPER_USER_PARAM_176,
    EMAPPER_USER_PARAM_177,
    EMAPPER_USER_PARAM_178,
    EMAPPER_USER_PARAM_179,
    EMAPPER_USER_PARAM_180,
    EMAPPER_USER_PARAM_181,
    EMAPPER_USER_PARAM_182,
    EMAPPER_USER_PARAM_183,
    EMAPPER_USER_PARAM_184,
    EMAPPER_USER_PARAM_185,
    EMAPPER_USER_PARAM_186,
    EMAPPER_USER_PARAM_187,
    EMAPPER_USER_PARAM_188,
    EMAPPER_USER_PARAM_189,
    EMAPPER_USER_PARAM_190,
    EMAPPER_USER_PARAM_191,
    EMAPPER_USER_PARAM_192,
    EMAPPER_USER_PARAM_193,
    EMAPPER_USER_PARAM_194,
    EMAPPER_USER_PARAM_195,
    EMAPPER_USER_PARAM_196,
    EMAPPER_USER_PARAM_197,
    EMAPPER_USER_PARAM_198,
    EMAPPER_USER_PARAM_199,
    EMAPPER_USER_PARAM_200,
    EMAPPER_USER_PARAM_201,
    EMAPPER_USER_PARAM_202,
    EMAPPER_USER_PARAM_203,
    EMAPPER_USER_PARAM_204,
    EMAPPER_USER_PARAM_205,
    EMAPPER_USER_PARAM_206,
    EMAPPER_USER_PARAM_207,
    EMAPPER_USER_PARAM_208,
    EMAPPER_USER_PARAM_209,
    EMAPPER_USER_PARAM_210,
    EMAPPER_USER_PARAM_211,
    EMAPPER_USER_PARAM_212,
    EMAPPER_USER_PARAM_213,
    EMAPPER_USER_PARAM_214,
    EMAPPER_USER_PARAM_215,
    EMAPPER_USER_PARAM_216,
    EMAPPER_USER_PARAM_217,
    EMAPPER_USER_PARAM_218,
    EMAPPER_USER_PARAM_219,
    EMAPPER_USER_PARAM_220,
    EMAPPER_USER_PARAM_221,
    EMAPPER_USER_PARAM_222,
    EMAPPER_USER_PARAM_223,
    EMAPPER_USER_PARAM_224,
    EMAPPER_USER_PARAM_225,
    EMAPPER_USER_PARAM_226,
    EMAPPER_USER_PARAM_227,
    EMAPPER_USER_PARAM_228,
    EMAPPER_USER_PARAM_229,
    EMAPPER_USER_PARAM_230,
    EMAPPER_USER_PARAM_231,
    EMAPPER_USER_PARAM_232,
    EMAPPER_USER_PARAM_233,
    EMAPPER_USER_PARAM_234,
    EMAPPER_USER_PARAM_235,
    EMAPPER_USER_PARAM_236,
    EMAPPER_USER_PARAM_237,
    EMAPPER_USER_PARAM_238,
    EMAPPER_USER_PARAM_239,
    EMAPPER_USER_PARAM_240,
    EMAPPER_USER_PARAM_241,
    EMAPPER_USER_PARAM_242,
    EMAPPER_USER_PARAM_243,
    EMAPPER_USER_PARAM_244,
    EMAPPER_USER_PARAM_245,
    EMAPPER_USER_PARAM_246,
    EMAPPER_USER_PARAM_247,
    EMAPPER_USER_PARAM_248,
    EMAPPER_USER_PARAM_249,
    EMAPPER_USER_PARAM_250,
    EMAPPER_USER_PARAM_251,
    EMAPPER_USER_PARAM_252,
    EMAPPER_USER_PARAM_253,
    EMAPPER_USER_PARAM_254,
    EMAPPER_USER_PARAM_255,
    EMAPPER_LAST_PARAM
} PASONU_emapper_param_t;

COMPOSER_status_t Compose_eeprom_mapper_set_param (
                 /*IN*/    const PASONU_emapper_param_t   code,
                 /*IN*/    const void                    *data,
                 /*IN*/    const INT32U                   size,
                 /*OUT*/   unsigned char                 *sent_pdu_data,
                 /*IN_OUT*/unsigned short                *length);

COMPOSER_status_t Compose_eeprom_mapper_get_param (
                 /*IN*/    const PASONU_emapper_param_t   code,
                 /*IN*/    const INT32U                   size,
                 /*OUT*/   unsigned char                 *sent_pdu_data,
                 /*IN_OUT*/unsigned short                *length);
COMPOSER_status_t Decompose_eeprom_mapper_get_param ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/INT32U               *size,
                  /*OUT*/void                 *data);


COMPOSER_status_t Decompose_rstp_loop_detect_event ( 
                  /*IN*/      const unsigned char		*receive_buffer,
                  /*IN*/      const unsigned short		length,
                  /*OUT*/     INT32U					*port_number,
                  /*OUT*/     INT32U					*loop_status,
                  /*OUT*/     INT32U					*link_status);


COMPOSER_status_t Compose_address_table_clear_all ( 
                  /*OUT*/   unsigned char    *sent_pdu_data,
                  /*IN_OUT*/unsigned short   *length);


COMPOSER_status_t Compose_address_table_set_uni_learn ( 
                  /*IN*/    const BOOLEAN    enable,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);


COMPOSER_status_t Compose_address_table_ageing_configuration ( 
                  /*IN*/    const BOOLEAN    enable,
                  /*IN*/    const INT32U     max_age,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);

COMPOSER_status_t Compose_get_address_table_ageing_configuration ( 
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_get_address_table_ageing_configuration ( 
                /*IN*/ const unsigned char  *receive_buffer,
                /*IN*/ const unsigned short  length,
                /*OUT*/BOOLEAN			    *enable,
				/*OUT*/INT32U				*max_age );


COMPOSER_status_t Compose_address_table_get_entry_num ( 
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_address_table_get_entry_num ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/long                 *number_of_entries );



typedef PACK_START struct
{
    INT16U index;
    INT16U hash;
} PACK_END ADDRTBL_iterator_t;

COMPOSER_status_t Compose_address_table_get_first_element ( 
                  /*OUT*/   unsigned char              *sent_pdu_data,
                  /*IN_OUT*/unsigned short             *length);
COMPOSER_status_t Decompose_address_table_get_first_element ( 
                  /*IN*/ const unsigned char          *receive_buffer,
                  /*IN*/ const unsigned short          length,
                  /*OUT*/ADDRTBL_iterator_t           *iter,
                  /*OUT*/GENERAL_mac_addr_t           *mac_address,
                  /*OUT*/PASONU_forwarding_action_t   *action,
                  /*OUT*/INT8U                        *age,
                  /*OUT*/BOOLEAN                      *is_static );
short int Onu_to_olt_converstion_mac_address
                ( const GENERAL_mac_addr_t  *onu_mac_address,
                  mac_address_t              olt_mac_address);

COMPOSER_status_t Compose_address_table_get_next_element (
                  /*IN*/    const ADDRTBL_iterator_t   *itr,
                  /*OUT*/   unsigned char              *sent_pdu_data,
                  /*IN_OUT*/unsigned short             *length);
COMPOSER_status_t Decompose_address_table_get_next_element ( 
                  /*IN*/ const unsigned char          *receive_buffer,
                  /*IN*/ const unsigned short          length,
                  /*OUT*/ADDRTBL_iterator_t           *iter,
                  /*OUT*/GENERAL_mac_addr_t           *mac_address,
                  /*OUT*/PASONU_forwarding_action_t   *action,
                  /*OUT*/INT8U                        *age,
                  /*OUT*/BOOLEAN                      *is_static );


COMPOSER_status_t Compose_address_table_set_uni_learn_limit ( 
				  /*IN*/	const BOOLEAN    multi_port,
                  /*IN*/    const INT8U      lport,	/*for multi_port only*/			  
                  /*IN*/    const INT32U     limit,
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_address_table_get_uni_learn_limit ( 
				  /*IN*/ const BOOLEAN         multi_port,
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/INT32U               *limit );


COMPOSER_status_t Compose_uni_set_port (
                  /*IN*/    const BOOLEAN     enable_cpu, 
                  /*IN*/    const BOOLEAN     enable_datapath,
                  /*OUT*/   unsigned char    *sent_pdu_data,
                  /*IN_OUT*/unsigned short   *length);

COMPOSER_status_t Compose_uni_get_port (
                  /*OUT*/   unsigned char    *sent_pdu_data,
                  /*IN_OUT*/unsigned short   *length);
COMPOSER_status_t Decompose_uni_get_port ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/BOOLEAN              *enable_cpu, 
                  /*OUT*/BOOLEAN              *enable_datapath );


typedef PACK_START struct
{
    INT8U MAC_type;
    INT8U MII_rate;
    INT8U duplex;
    INT8U gmii_master_mode;
    INT8U auto_negotiate;
    INT8U mode;
    INT8U advertise;
    INT8U adv_10_half;
    INT8U adv_10_full;
    INT8U adv_100_half;
    INT8U adv_100_full;
    INT8U adv_100_t4;
    INT8U adv_pause;
    INT8U adv_asym_pause;
    INT8U adv_1000_half;
    INT8U adv_1000_full;
    INT8U adv_port_type;
    INT8U phy_address;
} PACK_END PASONU_uni_config_t;

short int Olt_to_onu_converstion_uni_port_configuration
                ( const PON_uni_configuration_t  *olt_uni_configuration,
                  PASONU_uni_config_t            *onu_uni_configuration );
COMPOSER_status_t Compose_uni_set_port_configuration (
				  /*IN*/	const BOOLEAN                multi_port,
                  /*IN*/    const INT8U                  lport,	/*for multi_port only*/
                  /*IN*/    const PASONU_uni_config_t   *configuration,
                  /*OUT*/   unsigned char               *sent_pdu_data,
                  /*IN_OUT*/unsigned short              *length);
           
COMPOSER_status_t Compose_uni_get_port_configuration (
				  /*IN*/	const BOOLEAN                multi_port,
                  /*IN*/    const INT8U                  lport,	/*for multi_port only*/
                  /*OUT*/   unsigned char               *sent_pdu_data,
                  /*IN_OUT*/unsigned short              *length);
COMPOSER_status_t Decompose_uni_get_port_configuration ( 
				  /*IN*/ const BOOLEAN         multi_port,
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/PASONU_uni_config_t  *config );
short int Onu_to_olt_converstion_uni_port_configuration
                ( const PASONU_uni_config_t  *onu_uni_configuration,
                  PON_uni_configuration_t    *olt_uni_status  );


typedef PACK_START struct
{
    INT8U link;
    INT8U MAC_type;
    INT8U MII_rate;
    INT8U duplex;
    INT8U auto_negotiate;
    INT8U mode;
    INT8U pause_tx;
    INT8U pause_rx;
    INT8U port_type;
} PACK_END PASONU_uni_status_t;

COMPOSER_status_t Compose_uni_get_port_status (
				  /*IN*/	const BOOLEAN                multi_port,
                  /*IN*/    const INT8U                  lport,	/*for multi_port only*/
                  /*OUT*/   unsigned char               *sent_pdu_data,
                  /*IN_OUT*/unsigned short              *length);
COMPOSER_status_t Decompose_uni_get_port_status ( 
				  /*IN*/ const BOOLEAN         multi_port,
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/PASONU_uni_status_t  *status );
short int Onu_to_olt_converstion_uni_status
                ( const PASONU_uni_status_t  *onu_uni_status,
                  PON_uni_status_t           *uni_status  );


COMPOSER_status_t Compose_flash_get_burn_image_complete (
                  /*OUT*/   unsigned char   *sent_pdu_data,
                  /*IN_OUT*/unsigned short  *length);
COMPOSER_status_t Decompose_flash_get_burn_image_complete ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/BOOLEAN              *complete);


short int Olt_to_onu_converstion_encryption_direction
                ( const PON_encryption_direction_t   olt_direction, 
                  ENCRYPTION_type_t                 *onu_direction );
COMPOSER_status_t Compose_encryption_set_state ( 	  
                      /*IN*/    const ENCRYPTION_type_t   type,
                      /*OUT*/   unsigned char            *sent_pdu_data,
                      /*IN_OUT*/unsigned short           *length);

COMPOSER_status_t Compose_encryption_get_state ( 	  
                      /*OUT*/   unsigned char            *sent_pdu_data,
                      /*IN_OUT*/unsigned short           *length);
COMPOSER_status_t Decompose_encryption_get_state ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/ENCRYPTION_type_t    *type );
short int Onu_to_olt_converstion_encryption_direction
                ( const ENCRYPTION_type_t      onu_direction,
                  PON_encryption_direction_t  *olt_direction);

                      
typedef PACK_START struct
{
    INT32U key[4];
} PACK_END ENCRYPTION_key_t;

short int Olt_to_onu_converstion_encryption_key
                ( const PON_encryption_key_t   encryption_key,
				  ENCRYPTION_key_t            *onu_encryption_key);
COMPOSER_status_t Compose_encryption_set_key ( 
                      /*IN*/    const ENCRYPTION_key_t  *enc_key,
                      /*IN*/    const INT8U              key_seq,
                      /*OUT*/   unsigned char           *sent_pdu_data,
                      /*IN_OUT*/unsigned short          *length);

COMPOSER_status_t Compose_encryption_get_key ( 
                      /*IN*/    const INT8U              key_seq,
                      /*OUT*/   unsigned char           *sent_pdu_data,
                      /*IN_OUT*/unsigned short          *length);
COMPOSER_status_t Decompose_encryption_get_key ( 
                  /*IN*/ const unsigned char  *receive_buffer,
                  /*IN*/ const unsigned short  length,
                  /*OUT*/ENCRYPTION_key_t     *enc_key );
short int Onu_to_olt_converstion_encryption_key
                ( const ENCRYPTION_key_t  onu_encryption_key,
                  PON_encryption_key_t    encryption_key);



typedef enum
{
   FRAME_ETHERTYPE,
   FRAME_VLAN,
   FRAME_IPV4,
   FRAME_IPV4_TOS,
   FRAME_IPV4_SA,
   FRAME_IPV4_DA
} PASONU_frame_qualifier_t;

short int Olt_to_onu_converstion_direction
                ( const PON_pon_network_traffic_direction_t olt_direction, 
                  PASONU_direction_t *onu_direction );

COMPOSER_status_t Compose_classifier_add_filter( 
                      /*IN*/    const PASONU_direction_t           direction,
                      /*IN*/    const PASONU_frame_qualifier_t     qualifier,
                      /*IN*/    const INT16U                       value,
                      /*IN*/    const PASONU_forwarding_action_t   action,
                      /*OUT*/   unsigned char                     *sent_pdu_data,
                      /*IN_OUT*/unsigned short                    *length);


COMPOSER_status_t Compose_classifier_get_filter( 
                     /*IN*/    const PASONU_direction_t           direction,
                     /*IN*/    const PASONU_frame_qualifier_t     qualifier,
                     /*IN*/    const INT16U                       value,
                     /*OUT*/   unsigned char                     *sent_pdu_data,
                     /*IN_OUT*/unsigned short                    *length);

COMPOSER_status_t Compose_classifier_remove_filter( 
                     /*IN*/    const PASONU_direction_t           direction,
                     /*IN*/    const PASONU_frame_qualifier_t     qualifier,
                     /*IN*/    const INT16U                       value,
                     /*OUT*/   unsigned char                     *sent_pdu_data,
                     /*IN_OUT*/unsigned short                    *length);


short int Olt_to_onu_converstion_mac_address
                ( const mac_address_t   olt_mac_address,
                  GENERAL_mac_addr_t   *onu_mac_address );

COMPOSER_status_t Compose_classifier_add_da_filter ( 
                     /*IN*/    const GENERAL_mac_addr_t          *address,
                     /*IN*/    const PASONU_forwarding_action_t   action,
                     /*OUT*/   unsigned char                     *sent_pdu_data,
                     /*IN_OUT*/unsigned short                    *length);

COMPOSER_status_t Compose_classifier_add_up_sa_filter(
				/* IN */		const GENERAL_mac_addr_t *address,
                /* IN */		const PASONU_forwarding_action_t action,
				/* OUT */		unsigned char *sent_pdu_data,
				/* IN_OUT */	unsigned short *length);

COMPOSER_status_t Compose_classifier_remove_da_filter( 
                     /*IN*/    const GENERAL_mac_addr_t  *mac_address,
                     /*OUT*/   unsigned char             *sent_pdu_data,
                     /*IN_OUT*/unsigned short            *length);


typedef enum
{
  PASONU_TRAFFIC_IP_ADDRESS,
  PASONU_TRAFFIC_IP_ADDRESS_NO_UDP,
  PASONU_TRAFFIC_IP_ADDRESS_NO_TCP,
  PASONU_TRAFFIC_IP_ADDRESS_NO_UDPTCP,
  PASONU_TRAFFIC_UDP_PORT,
  PASONU_TRAFFIC_TCP_PORT,
  PASONU_TRAFFIC_UDPTCP_PORT,
  PASONU_TRAFFIC_IP_AND_UDP_PORT,
  PASONU_TRAFFIC_IP_AND_TCP_PORT,
  PASONU_TRAFFIC_IP_AND_UDPTCP_PORT
} PASONU_traffic_qualifier_t;


typedef enum
{
  PASONU_TRAFFIC_DESTINATION,
  PASONU_TRAFFIC_SOURCE,
  PASONU_TRAFFIC_BOTH
} PASONU_traffic_address_t;

COMPOSER_status_t Compose_classifier_add_l3l4_filter(
				/* IN */		const PASONU_direction_t           direction,
				/* IN */    	const PASONU_traffic_qualifier_t   traffic,
				/* IN */    	const PASONU_traffic_address_t     address,
			    /* IN */		const INT32U                       ip_address,
                /* IN */        const INT16U                       l4_port_num,
                /* IN */        const PASONU_forwarding_action_t   action,
				/* OUT */   	unsigned char                     *sent_pdu_data,
				/* IN_OUT */	unsigned short                    *length);


COMPOSER_status_t Compose_classifier_remove_l3l4_filter(
				/* IN */		const PASONU_direction_t           direction,
				/* IN */		const PASONU_traffic_qualifier_t   traffic,
				/* IN */		const PASONU_traffic_address_t     address,
				/* IN */    	const INT32U                       ip_address,
                /* IN */        const INT16U                       l4_port_num,
				/* OUT */   	unsigned char                     *sent_pdu_data,
				/* IN_OUT */	unsigned short                    *length);


COMPOSER_status_t Compose_classifier_get_l3l4_filter(
				/* IN */		const PASONU_direction_t           direction,
				/* IN */		const PASONU_traffic_qualifier_t   traffic,
				/* IN */		const PASONU_traffic_address_t     address,
				/* IN */    	const INT32U                       ip_address,
                /* IN */        const INT16U                       l4_port_num,
				/* OUT */		unsigned char                     *sent_pdu_data,
			    /* IN_OUT */	unsigned short                    *length);
COMPOSER_status_t Decompose_classifier_get_l3l4_filter(
				/* IN */	const unsigned char   *receive_buffer,
				/* IN */	const unsigned short   length,
                /* OUT */	PASONU_forwarding_action_t *action);

#endif



#if defined(__cplusplus)
}
#endif

#endif /* __ONU_PMC_COMM_H__ */


