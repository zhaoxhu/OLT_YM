/*  
**  CTC_STACK_variable_descriptor_defines_expo.h
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 12/03/2006
**
**	NOTE FOR ONU ENVIRONEMENT: This file must be preceeded with the following include files:
**  "CTC_types.h"
**  "GENERAL_types.h"
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 20/03/2006 |	Creation		     | Meital Levy
*/

#ifndef __CTC_VAR_DESC_DEF_EXPO_H__
#define __CTC_VAR_DESC_DEF_EXPO_H__


/* CTC Extended Variable Descriptor instances structures */

/* C7-CTC_EX_VAR_ONU_IDENTIFIER */

#define HW_VERSION_NUM_OF_BYTES 8
#define SW_VERSION_NUM_OF_BYTES 16
#define ONU_EXTEND_MODEL_NUM_OF_BYTES 16

typedef struct
{
	unsigned char		vendor_id[4];
	unsigned long int	onu_model;
	mac_address_t		onu_id;
	unsigned char		hardware_version[HW_VERSION_NUM_OF_BYTES];
	unsigned char		software_version[SW_VERSION_NUM_OF_BYTES];
	unsigned char       extendedModel[ONU_EXTEND_MODEL_NUM_OF_BYTES];/*add for CTC V3.0 by yangzl@2016-4-8*/
} CTC_STACK_onu_serial_number_t;


/* C7-CTC_EX_VAR_CHIPSET_ID */
#define CTC_VENDOR_ID_LENGTH 2
typedef struct
{
	unsigned char		vendor_id[CTC_VENDOR_ID_LENGTH]; /* PMC-SIERRA value: ASCII 'E6' */
	unsigned short		chip_model;
	unsigned char		revision;
	unsigned char		date[3];
} CTC_STACK_chipset_id_t;

#define PMC_SIERRA_VENDOR_ID "E6"

/* C7-CTC_EX_VAR_ONU_CAPABILITIES */
typedef enum
{
    CTC_SERVICE_GBIT_ETHERNET	= 0x1,
	CTC_SERVICE_FE_ETHERNET		= 0x2,
	CTC_SERVICE_VOIP			= 0x4,
    CTC_SERVICE_TDM_CES			= 0x8,
}ctc_stack_service_supported_t;

typedef struct
{
	unsigned char						service_supported;
	unsigned char						ge_ethernet_ports_number;
	INT64U								ge_ports_bitmap;
	unsigned char						fe_ethernet_ports_number;
	INT64U								fe_ports_bitmap;
	unsigned char						pots_ports_number;
	unsigned char						e1_ports_number;
	unsigned char						upstream_queues_number;
	unsigned char						max_upstream_queues_per_port;
	unsigned char						downstream_queues_number;
	unsigned char						max_downstream_queues_per_port;
	bool								battery_backup;
} CTC_STACK_onu_capabilities_t;


/* C7-0x0011 */
#define MAX_PORT_ENTRIES_NUMBER 32/*24*/

#define MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER 32/*24*/

/* C7-CTC_EX_VAR_ETH_PORT_POLICING */
typedef enum
{
    CTC_STACK_STATE_DEACTIVATE	= 0x00,
	CTC_STACK_STATE_ACTIVATE	= 0x01,
} CTC_STACK_on_off_state_t;



typedef struct
{
	CTC_STACK_on_off_state_t	operation;
	unsigned long int			cir;
	unsigned long int			bucket_depth;
	unsigned long int			extra_burst_size;
} CTC_STACK_ethernet_port_policing_entry_t;



/* C7-0x0021 */
#define CTC_MAX_VLAN_FILTER_ENTRIES 31
#define CTC_MAX_VLAN_AGGREGATION_TABLES 4
#define CTC_MAX_VLAN_AGGREGATION_ENTRIES 8

typedef enum
{
    CTC_VLAN_MODE_TRANSPARENT       = 0x00,
    CTC_VLAN_MODE_TAG               = 0x01,
    CTC_VLAN_MODE_TRANSLATION       = 0x02,
    CTC_VLAN_MODE_AGGREGATION       = 0x03,
    CTC_VLAN_MODE_TRUNK             = 0x04,
} CTC_STACK_vlan_modes_t;


typedef struct
{
    unsigned long               target_vlan;
    unsigned long               vlan_aggregation_list[CTC_MAX_VLAN_AGGREGATION_ENTRIES];
    unsigned short              number_of_aggregation_entries;  /* This parameter describes how many entries in <vlan_aggregation_list> are used */
} CTC_STACK_port_vlan_aggregation_table_t;


typedef struct
{
    CTC_STACK_vlan_modes_t      			mode;
    unsigned long               			default_vlan;
    unsigned long               			vlan_list[CTC_MAX_VLAN_FILTER_ENTRIES];
    unsigned char               			number_of_entries;  			/* This parameter describes how many entries in <vlan_list> are used, when <mode> == <CTC_VLAN_MODE_TRANSLATION> or <CTC_VLAN_MODE_TRUNK> */
    unsigned short              			number_of_aggregation_tables; 	/* This parameter describes how many aggregation tables */ 
    CTC_STACK_port_vlan_aggregation_table_t vlan_aggregation_tables[CTC_MAX_VLAN_AGGREGATION_TABLES];
} CTC_STACK_port_vlan_configuration_t; 


/* C7-0x0031 */
#ifdef __PASSAVE_ONU__
#	define CTC_MAX_CLASS_RULES_COUNT   9
#	define CTC_MAX_CLASS_RULE_PAIRS    1 
#else
#	define CTC_MAX_CLASS_RULES_COUNT   32
#	define CTC_MAX_CLASS_RULE_PAIRS    4  
#endif	/* __PASSAVE_ONU__ */

typedef enum
{
	CTC_CLASS_ACTION_DELETE_RULE	= 0x00,
	CTC_CLASS_ACTION_ADD_RULE		= 0x01,
	CTC_CLASS_ACTION_DELETE_LIST	= 0x02,
	CTC_CLASS_ACTION_LIST			= 0x03,
} CTC_STACK_classification_rule_action_t;

typedef enum
{
	CTC_FIELD_SEL_DA_MAC				= 0x00,
	CTC_FIELD_SEL_SA_MAC				= 0x01,
	CTC_FIELD_SEL_ETHERNET_PRIORITY		= 0x02,
	CTC_FIELD_SEL_VLAN_ID				= 0x03,
	CTC_FIELD_SEL_ETHER_TYPE			= 0x04,
	CTC_FIELD_SEL_DEST_IP				= 0x05,
	CTC_FIELD_SEL_SRC_IP				= 0x06,
	CTC_FIELD_SEL_IP_PROTOCOL_TYPE		= 0x07,
	CTC_FIELD_SEL_IPV4_TOS_DSCP			= 0x08,
	CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS		= 0x09,
	CTC_FIELD_SEL_L4_SRC_PORT			= 0x0A,
	CTC_FIELD_SEL_L4_DEST_PORT			= 0x0B,
	CTC_FIELD_SEL_IP_VERSION			= 0x0C,
	CTC_FIELD_SEL_IP_FLOW_LABLE		= 0x0D,
	CTC_FIELD_SEL_DEST_IPV6			= 0x0E,
	CTC_FIELD_SEL_SRC_IPV6			= 0x0F,
	CTC_FIELD_SEL_DEST_IPV6_PREFIX		= 0x10,
	CTC_FIELD_SEL_SRC_IPV6_PREFIX		= 0x11
} CTC_STACK_field_selection_t;

typedef enum
{
	CTC_VALIDATION_OPERS_NEVER_MATCH			= 0x00,
	CTC_VALIDATION_OPERS_EQUAL					= 0x01,
	CTC_VALIDATION_OPERS_NOT_EQUAL				= 0x02,
	CTC_VALIDATION_OPERS_LESS_THAN_OR_EQUAL		= 0x03,
	CTC_VALIDATION_OPERS_GREATER_THAN_OR_EQUAL	= 0x04,
	CTC_VALIDATION_OPERS_FIELD_EXIST			= 0x05,
	CTC_VALIDATION_OPERS_FIELD_NOT_EXIST		= 0x06,
	CTC_VALIDATION_OPERS_ALWAYS_MATCH			= 0x07,
} CTC_STACK_validation_operators_t;



typedef struct
{
	unsigned long		match_value;
	mac_address_t	mac_address;
	PON_ipv6_addr_t	ipv6_match_value;	/*match value for IPv6 address*/
}CTC_STACK_value_t;

typedef struct
{
	CTC_STACK_field_selection_t			field_select;
	CTC_STACK_value_t					value;
	CTC_STACK_validation_operators_t	validation_operator;
} CTC_STACK_class_rule_entry;

typedef struct
{
	bool									valid;
	unsigned char							queue_mapped;
	unsigned char							priority_mark;
	unsigned char							num_of_entries;
	CTC_STACK_class_rule_entry				entries[CTC_MAX_CLASS_RULE_PAIRS];
} CTC_STACK_classification_rule_t;


typedef  CTC_STACK_classification_rule_t CTC_STACK_classification_rules_t[CTC_MAX_CLASS_RULES_COUNT];


typedef struct
{
	CTC_STACK_classification_rules_t rules;

	CTC_STACK_classification_rule_action_t	action;

} CTC_STACK_classification_and_marking_t;	/* NOTE: This may exceed the standard size of 128bytes, but the frame composer blocks this with error. */


/* C7-0x0041 */
#define CTC_MAX_MULTICAST_VLAN_ENTRIES 50

typedef enum
{
	CTC_MULTICAST_VLAN_OPER_DELETE	= 0x00,
	CTC_MULTICAST_VLAN_OPER_ADD		= 0x01,
	CTC_MULTICAST_VLAN_OPER_CLEAR	= 0x02,
	CTC_MULTICAST_VLAN_OPER_LIST	= 0x03,
} CTC_STACK_multicast_vlan_opers_t;

typedef struct
{
	CTC_STACK_multicast_vlan_opers_t	vlan_operation;
	unsigned short						vlan_id[CTC_MAX_MULTICAST_VLAN_ENTRIES];
	unsigned char						num_of_vlan_id;	/* Number of entries used in <vlan_id> */
} CTC_STACK_multicast_vlan_t;



/* C7-0x0042 */
#define CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES 31

typedef enum
{
	CTC_NO_STRIP_VLAN_TAG	=	0x00,
	CTC_STRIP_VLAN_TAG		=	0x01,
	CTC_TRANSLATE_VLAN_TAG	=	0x02
} CTC_STACK_tag_oper_t;

typedef struct
{
	unsigned short						       multicast_vlan;
	unsigned short   					       iptv_user_vlan;
} CTC_STACK_multicast_vlan_switching_entry_t;

typedef struct
{
    CTC_STACK_multicast_vlan_switching_entry_t entries[CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES];
    unsigned char                              number_of_entries;
} CTC_STACK_multicast_vlan_switching_t;


#define CTC_MAX_MULTICAST_MAC_ENTRIES 256
#define CTC_MAX_MULTICAST_MAC_ENTRIES_PER_FRAME 132
#define CTC_MAX_MULTICAST_IPV6_ENTRIES_PER_FRAME	66
#define CTC_MAX_MULTICAST_SAIP_ENTRIES_PER_FRAME	55
#define CTC_MULTICAST_CONTROL_IPV4_ENTRY_LENGTH 10
#define CTC_MULTICAST_CONTROL_IPV6_ENTRY_LENGTH 20
#define CTC_MULTICAST_CONTROL_SAIP_ENTRY_LENGTH 24
#define CTC_MULTICAST_CONTROL_TLV_HEADER_LENGTH 3
#define CTC_MAX_IPV6_STRING_LENGTH  16

typedef enum
{
    CTC_STACK_MULTICAST_CONTROL_TYPE_DA_ONLY	= 0x00,
    CTC_STACK_MULTICAST_CONTROL_TYPE_DA_VID		= 0x01,
    CTC_STACK_MULTICAST_CONTROL_TYPE_DA_SAIP      = 0x02,	/*IPv4&IPv6*/
    CTC_STACK_MULTICAST_CONTROL_TYPE_DIPV4_VID    = 0x03,	/*IPv4 only*/
    CTC_STACK_MULTICAST_CONTROL_TYPE_DIPV6_VID    = 0x04	/*IPv6 only*/
} CTC_STACK_multicast_control_type_t;

#define CTC_STACK_MULTICAST_CONTROL_TYPE_DIP_VID CTC_STACK_MULTICAST_CONTROL_TYPE_DIPV4_VID

typedef enum
{
	CTC_MULTICAST_CONTROL_ACTION_DELETE			= 0x00,
	CTC_MULTICAST_CONTROL_ACTION_ADD			= 0x01,
	CTC_MULTICAST_CONTROL_ACTION_CLEAR			= 0x02,
	CTC_MULTICAST_CONTROL_ACTION_LIST			= 0x03
} CTC_STACK_multicast_control_action_t;

typedef enum
{
	CTC_STACK_IPV4		= 0x00,
	CTC_STACK_IPV6		= 0x01		
}CTC_STACK_ip_version_t;

typedef struct
{
	mac_address_t		da;
	unsigned short		vid;	/* Multicast VLAN ID */
	unsigned short		user_id;
	unsigned long	 		sa;			/*Source IP, IPv4*/	
} CTC_STACK_multicast_entry_t;

typedef struct
{
    CTC_STACK_multicast_entry_t	ipv4_entry;	/*backward compatible with IPv4 entry*/
    PON_ipv6_addr_t	ipv6_sa;	/*Source IP, IPv6*/
    PON_ipv6_addr_t	ipv6_gda;	/*IPv6 GDA*/
    CTC_STACK_ip_version_t			ip_flag;  /*0x00-use Ipv4 address, 0x01-use Ipv6 address*/
} CTC_STACK_multicast_entry_ipv6_ext_t;

typedef struct
{
	CTC_STACK_multicast_control_action_t	action;
	CTC_STACK_multicast_control_type_t		control_type;
	CTC_STACK_multicast_entry_t				entries[CTC_MAX_MULTICAST_MAC_ENTRIES];
	unsigned short							num_of_entries;	/* The count of rules within <entries> */
	
} CTC_STACK_multicast_control_t;

typedef struct
{
  CTC_STACK_multicast_control_action_t	action;
  CTC_STACK_multicast_control_type_t		control_type;
  CTC_STACK_multicast_entry_ipv6_ext_t	 entries[CTC_MAX_MULTICAST_MAC_ENTRIES];
  unsigned short		num_of_entries;	/* The count of rules within <entries> */
} CTC_STACK_multicast_control_ipv6_ext_t;

typedef enum
{
    CTC_STACK_FAST_LEAVE_ADMIN_STATE_DISABLED	= 0x00000001,
	CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED	= 0x00000002
} CTC_STACK_fast_leave_admin_state_t;


/* aFastLeaveAbility */
#define CTC_MAX_FAST_LEAVE_ABILITY_MODES	6
typedef enum
{
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_IGMP_SNOOPING_MODE	= 0x00000000,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_IGMP_SNOOPING_MODE		= 0x00000001,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_MC_CONTROL_MODE		= 0x00000010,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MC_CONTROL_MODE			= 0x00000011,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_MLD_SNOOPING_MODE	= 0x00000002,
	CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MLD_SNOOPING_MODE		= 0x00000003
} CTC_STACK_fast_leave_ability_mode_t;

typedef struct
{
	CTC_STACK_fast_leave_ability_mode_t		modes[CTC_MAX_FAST_LEAVE_ABILITY_MODES];
	unsigned short							num_of_modes; /* The count of modes within <modes> */
} CTC_STACK_fast_leave_ability_t;

typedef struct
{
	unsigned char		port_number;
	unsigned long		CIR;	/* Committed Information Rate of the port (in Kbps) */
	unsigned long		PIR;	/* Peak Information Rate of the port (in Kbps) */
	bool				state;	/* Enable or Disable */
} CTC_STACK_ethernet_port_ds_rate_limiting_entry_t;

typedef  CTC_STACK_ethernet_port_ds_rate_limiting_entry_t CTC_STACK_ethernet_ports_ds_rate_limiting_t[MAX_PORT_ENTRIES_NUMBER];


/* 9-313 aFECAbility */
typedef enum
{
	STD_FEC_ABILITY_UNKNOWN = 0x1,
	STD_FEC_ABILITY_SUPPORTED = 0x2,
	STD_FEC_ABILITY_UNSUPPORTED = 0x3
} CTC_STACK_standard_FEC_ability_t;


/* 9-314 aFECmode */
typedef enum
{
	STD_FEC_MODE_UNKNOWN = 0x1,
	STD_FEC_MODE_ENABLED = 0x2,
	STD_FEC_MODE_DISABLED = 0x3
} CTC_STACK_standard_FEC_mode_t;



typedef enum
{
    CTC_STACK_LINK_STATE_DOWN	= 0x00,
	CTC_STACK_LINK_STATE_UP	= 0x01,
} CTC_STACK_link_state_t;

typedef enum
{
    CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING	= 0x00,
	CTC_STACK_PROTOCOL_CTC				= 0x01,
} CTC_STACK_multicast_protocol_t;

typedef enum
{
	CTC_STACK_STATISTIC_STATE_DISABLE = 0x01,
	CTC_STACK_STATISTIC_STATE_ENABLE = 0x02,		
} CTC_STACK_statistic_state_enum_t;

typedef struct{
	CTC_STACK_statistic_state_enum_t status;
	unsigned long int circle;
}CTC_STACK_statistic_state_t;

#if 0 /*declare for ctc 3.0 draft version*/
typedef struct
{
	unsigned long int circle;
	unsigned long int time;
	INT64U framestransmitted;
	INT64U framesreceived;
	INT64U octetstransmitted;
	INT64U octetsreceived;
	INT64U toolongframesreceived;
	INT64U tooshortframesreceived;
	INT64U crcerrorframesreceived;
	INT64U receivedframesdropped;
	INT64U transmittedframesdropped;
	INT64U errorframesnottransmitted;
	INT64U errorframesreceived;
	INT64U unicastframesreceived;
	INT64U multicastframesreceived;
	INT64U broadcastframesreceived;
	INT64U unicastframestransmitted;
	INT64U multicastframestransmitted;
	INT64U broadcastframestransmitted;
	INT64U portstatuschangetimes;
}CTC_STACK_statistic_data_t;
#else
typedef struct
{
	INT64U downStreamDropEvents;
	INT64U upStreamDropEvents;
	INT64U downStreamOctets;
	INT64U upStreamOctets;
	INT64U downStreamFrames;
	INT64U upStreamFrames;
	INT64U downStreamBroadcastFrames;
	INT64U upStreamBroadcastFrames;
	INT64U downStreamMulticastFrames;
	INT64U upStreamMulticastFrames;
	INT64U downStreamCrcErroredFrames;
	INT64U upStreamCrcErroredFrames;
	INT64U downStreamUndersizeFrames;
	INT64U upStreamUndersizeFrames;
	INT64U downStreamOversizeFrames;
	INT64U upStreamOversizeFrames;
	INT64U downStreamFragments;
	INT64U upStreamFragments;
	INT64U downStreamJabbers;
	INT64U upStreamJabbers;
	INT64U downStreamFrames64Octets;
	INT64U upStreamFrames64Octets;
	INT64U downStreamFrames65to127Octets;
	INT64U upStreamFrames65to127Octets;
	INT64U downStreamFrames128to255Octets;
	INT64U upStreamFrames128to255Octets;
	INT64U downStreamFrames256to511Octets;
	INT64U upStreamFrames256to511Octets;
	INT64U downStreamFrames512to1023Octets;
	INT64U upStreamFrames512to1023Octets;
	INT64U downStreamFrames1024to1518Octets;	
	INT64U upStreamFrames1024to1518Octets;	
	INT64U downStreamDiscards;
	INT64U upStreamDiscards;	
	INT64U downStreamErrors;
	INT64U upStreamErrors;	
	INT64U portstatuschangetimes;
}CTC_STACK_statistic_data_t;
#endif

#define CTC_STACK_PROTOCOL_IGMP_SNOOPING CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING

typedef enum
{
    CTC_STACK_ETHERNET_PORT_STATE_DISABLE	= 0x01,
	CTC_STACK_ETHERNET_PORT_STATE_ENABLE	= 0x02,
} CTC_STACK_ethernet_port_state_t;

typedef enum
{
    CTC_STACK_ETHERNET_PORT_ACTION_DEACTIVATE	= 0x01,
	CTC_STACK_ETHERNET_PORT_ACTION_ACTIVATE		= 0x02,
} CTC_STACK_ethernet_port_action_t;


typedef enum
{
	CTC_STACK_TECHNOLOGY_ABILITY_UNDEFINED		= 1,
	CTC_STACK_TECHNOLOGY_ABILITY_UNKNOWN		= 2,
    CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_T		= 14,
	CTC_STACK_TECHNOLOGY_ABILITY_10_BASE_TFD	= 142,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T4	= 23,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TX	= 25,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_TXFD	= 252,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_PAUSE		= 312,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_A_PAUSE	= 313,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_S_PAUSE	= 314,
	CTC_STACK_TECHNOLOGY_ABILITY_FDX_B_PAUSE	= 315,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2	= 32,
	CTC_STACK_TECHNOLOGY_ABILITY_100_BASE_T2FD	= 322,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_X	= 36,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_XFD	= 362,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_T	= 40,
	CTC_STACK_TECHNOLOGY_ABILITY_1000_BASE_TFD	= 402,
	CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT1		= 37,
	CTC_STACK_TECHNOLOGY_ABILITY_REM_FAULT2		= 372,
	CTC_STACK_TECHNOLOGY_ABILITY_ISO_ETHERNET	= 8029,
	
} CTC_STACK_technology_ability_t;

#define MAX_TECHNOLOGY_ABILITY 20
typedef struct
{
	unsigned char					number_of_abilities;
	CTC_STACK_technology_ability_t	technology[MAX_TECHNOLOGY_ABILITY];

} CTC_STACK_auto_negotiation_technology_ability_t;

/* C7-0x1021 */
#define CTC_MAX_VLAN_QINQ_ENTRIES 17

typedef enum
{
    CTC_QINQ_MODE_NONE           = 0x00,
    CTC_QINQ_MODE_PER_PORT       = 0x01,
    CTC_QINQ_MODE_PER_VLAN       = 0x02, 
} CTC_STACK_qinq_modes_t;

typedef struct
{
	CTC_STACK_qinq_modes_t		mode;          
	unsigned long				default_vlan;                         /* For Per Port QinQ , Q_TV*/
	unsigned long				vlan_list[CTC_MAX_VLAN_QINQ_ENTRIES]; /* For Per VLAN QinQ, 1st Entry is Q_OV, 2nd is Q_SV */  
	unsigned char				number_of_entries;                    /* This parameter describes how many entries in <vlan_list> are used, when <mode> == < CTC_QinQ_MODE_PER_VLAN > */     
} CTC_STACK_port_qinq_configuration_t; 

typedef struct
{
	unsigned short transceiver_temperature;			/* Working temperature of ONU optical module */
	unsigned short supply_voltage;					/* Supply Voltage(Vcc) of ONU optical module */
	unsigned short tx_bias_current;					/* Bias Current of ONU optical TX */
	unsigned short tx_power; 						/* Power of ONU optical TX (Output) */
	unsigned short rx_power;						/* Power of ONU optical RX (Input) */
} CTC_STACK_optical_transceiver_diagnosis_t; 


#define CTC_MAX_SERVICE_SLA_ENTRIES		8

typedef enum
{
    CTC_STACK_BEST_EFFORT_SCHEDULING_SCHEME_SP			= 0,
	CTC_STACK_BEST_EFFORT_SCHEDULING_SCHEME_WRR			= 1,
	CTC_STACK_BEST_EFFORT_SCHEDULING_SCHEME_SP_AND_WRR	= 2,
} CTC_STACK_best_effort_scheduling_scheme_t;

typedef struct
{
	unsigned char	queue_number;				/* Queue to which service traffic is classified */
	unsigned short	fixed_packet_size;			/* Given in byte units */
	unsigned short	fixed_bandwidth;			/* Fixed bandwidth in 256 Kbps units */
	unsigned short	guaranteed_bandwidth;		/* Assured bandwidth in 256 Kbps units */
	unsigned short	best_effort_bandwidth;		/* Best effort bandwidth in 256 Kbps units */
	unsigned char	wrr_weight;					/* Possible values: 0 (SP), 1 - 100 (WRR) */
} CTC_STACK_service_sla_entry_t;

typedef struct
{
	CTC_STACK_best_effort_scheduling_scheme_t	best_effort_scheduling_scheme;
	unsigned char								high_priority_boundary;
	unsigned long int							cycle_length;					/* Given in TQ units */
	unsigned char								number_of_service_sla_entries;	/* 1..8 */
	CTC_STACK_service_sla_entry_t				service_sla_entries[CTC_MAX_SERVICE_SLA_ENTRIES];
} CTC_STACK_service_sla_t;


typedef enum
{
	OBJECT_LEAF_TYPE_ONU_LEAF		= 0,
	OBJECT_LEAF_TYPE_PORT_LEAF		= 1,
	OBJECT_LEAF_TYPE_VLAN_LEAF		= 3,
	OBJECT_LEAF_TYPE_QOS_LEAF		= 4,
	OBJECT_LEAF_TYPE_MULTICAST_LEAF = 5,

	OBJECT_LEAF_TYPE_NONE			= 0xff
}object_leaf_type_t;

typedef enum
{
	CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE			= 0x00,

	CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT	= 0x01,
	CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT		= 0x02,
	CTC_MANAGEMENT_OBJECT_PORT_TYPE_ADSL2_PLUS_PORT	= 0x03,
	CTC_MANAGEMENT_OBJECT_PORT_TYPE_VDSL2_PORT		= 0x04,
	CTC_MANAGEMENT_OBJECT_PORT_TYPE_E1_PORT			= 0x05,
}CTC_management_object_port_type_t;

typedef enum
{
	CTC_MANAGEMENT_OBJECT_LEAF_NONE		= 0xFFFF, /*for ONU there is no leaf*/

	CTC_MANAGEMENT_OBJECT_LEAF_PORT		= 0x0001,
	CTC_MANAGEMENT_OBJECT_LEAF_CARD		= 0x0002,
	CTC_MANAGEMENT_OBJECT_LEAF_LLID		= 0x0003,
	CTC_MANAGEMENT_OBJECT_LEAF_PON_IF	= 0x0004,

	CTC_MANAGEMENT_OBJECT_LEAF_LAST
}CTC_management_object_leaf_t;

typedef struct
{
	unsigned char							frame_number; /* 0..3 */
	unsigned char							slot_number;  /* 0..63 (63 for all slots) */
	unsigned short							port_number;  /* (FF/FFFF for all ports) */
	CTC_management_object_port_type_t		port_type;
}CTC_management_object_index_t;

typedef struct
{
	CTC_management_object_leaf_t			leaf;
	CTC_management_object_index_t			index;
}CTC_management_object_t;


/* event notification */
#define SPECIFIC_EVENT_TYPE     0xfe /* event type of Organization Specific Event Notification */
#define END_OF_FRAME            0x00
#define MAX_ALARM_STR_SIZE      40

typedef enum
{
    EQUIPMENT_ALARM		             = 0x0001,
    POWERING_ALARM			         = 0x0002,
    BATTERY_MISSING		             = 0x0003, 
    BATTERY_FAILURE			         = 0x0004,
    BATTERY_VOLT_LOW			     = 0x0005,
    PHYSICAL_INTRUSION_ALARM         = 0x0006, 
    ONU_SELF_TEST_FAILURE            = 0x0007,
    ONU_TEMP_HIGH_ALARM		         = 0x0009, 
    ONU_TEMP_LOW_ALARM		         = 0x000A,
    IAD_CONNECTION_FAILURE			 = 0x000B,
	PON_IF_SWITCH                    = 0x000C,
    RX_POWER_HIGH_ALARM		         = 0x0101,
    RX_POWER_LOW_ALARM		         = 0x0102, 
    TX_POWER_HIGH_ALARM		         = 0x0103,
    TX_POWER_LOW_ALARM		         = 0x0104,
    TX_BIAS_HIGH_ALARM		         = 0x0105, 
    TX_BIAS_LOW_ALARM		         = 0x0106,
    VCC_HIGH_ALARM 			         = 0x0107,
    VCC_LOW_ALARM  			         = 0x0108, 
    TEMP_HIGH_ALARM			         = 0x0109,
    TEMP_LOW_ALARM		             = 0x010a,
    RX_POWER_HIGH_WARNING	         = 0x010b, 
    RX_POWER_LOW_WARNING		     = 0x010c,
    TX_POWER_HIGH_WARNING	         = 0x010d,
    TX_POWER_LOW_WARNING		     = 0x010e, 
    TX_BIAS_HIGH_WARNING		     = 0x010f,
    TX_BIAS_LOW_WARNING		         = 0x0110,
    VCC_HIGH_WARNING			     = 0x0111, 
    VCC_LOW_WARNING			         = 0x0112,
    TEMP_HIGH_WARNING		         = 0x0113,
    TEMP_LOW_WARNING			     = 0x0114, 
    CARD_ALARM    			         = 0x0201,
    SELF_TEST_FAILURE		         = 0x0202,
    ETH_PORT_AUTO_NEG_FAILURE        = 0x0301,
    ETH_PORT_LOS   			         = 0x0302,
    ETH_PORT_FAILURE                 = 0x0303,
    ETH_PORT_LOOPBACK		         = 0x0304,
    ETH_PORT_CONGESTION		         = 0x0305,
    POTS_PORT_FAILURE		         = 0x0401,
    E1_PORT_FAILURE			         = 0x0501,
    E1_TIMING_UNLOCK			     = 0x0502,
    E1_LOS           		         = 0x0503,
    
    
} CTC_STACK_alarm_id_t;


typedef enum
{
    CLEAR_REPORT_ALARM               = 0x00,
    REPORT_ALARM                     = 0x01,

} CTC_STACK_alarm_state_t;

typedef enum
{
    FAILURE_CODE_ALARM_INFO_TYPE,           
    TEST_VALUE_ALARM_INFO_TYPE,      /*this type can get/set alarm threshold*/     
    NONE_ALARM_INFO_TYPE           
} CTC_STACK_alarm_info_type_t;                   
                                                 
typedef struct                                   
{                                                
	unsigned char				event_type;
	unsigned char				event_length;
    OAM_oui_t                   oui;
} CTC_STACK_event_header_t;

typedef union
{
	long  				        alarm_info_long;
} CTC_STACK_alarm_info_t;

typedef struct
{
    CTC_management_object_t     management_object;
	CTC_STACK_alarm_id_t		alarm_id;
	unsigned short				time_stamp;
	CTC_STACK_alarm_state_t		alarm_state;
	CTC_STACK_alarm_info_t		alarm_info;
} CTC_STACK_event_value_t;

typedef struct
{
	unsigned char				 alarm_str[MAX_ALARM_STR_SIZE];
	CTC_STACK_alarm_id_t		 alarm_id;
    unsigned char                alarm_info_size;
    CTC_STACK_alarm_info_type_t  alarm_info_type;
} CTC_STACK_alarm_str_t;
/* end of event notification */

#define CTC_MAX_LLID_QUEUE_CONFIG_ENTRIES		8

typedef struct
{
	unsigned short	queue_id;				    
	unsigned short	wrr_weight;					/* Possible values: 0 (SP), 1 - 100 (WRR) */
} CTC_STACK_llid_queue_config_entry_t;

typedef struct
{
	PON_llid_t                                  llid;		/* if Multi LLID mode is not used, this parameter is not relevant */
	unsigned char								number_of_llid_queue_config_entries; /* 1..8 */
	CTC_STACK_llid_queue_config_entry_t			llid_queue_config_entries[CTC_MAX_LLID_QUEUE_CONFIG_ENTRIES];
} CTC_STACK_llid_queue_config_t;


typedef struct
{
	CTC_management_object_index_t   management_object_index;
	unsigned long					CIR;						/* Committed Information Rate of the port (in Kbps) */
	unsigned long					PIR;						/* Peak Information Rate of the port (in Kbps) */
	bool							state;						/* Enable or Disable */
} CTC_STACK_ethernet_management_object_ds_rate_limiting_entry_t;

typedef  CTC_STACK_ethernet_management_object_ds_rate_limiting_entry_t CTC_STACK_ethernet_management_object_ds_rate_limiting_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];

typedef enum
{
    CTC_STACK_VOIP_PROTOCOL_H248              = 0x00,
    CTC_STACK_VOIP_PROTOCOL_SIP               = 0x01
} CTC_STACK_voip_protocol_t;


#define VOIP_SW_VERSION_SIZE			        32
#define VOIP_SW_TIME_SIZE			            32

typedef struct
{
    mac_address_t                         mac_address;                      /* The IAD MAC address */
    CTC_STACK_voip_protocol_t             voip_protocol;                    /* The supported VOIP protocol */
    unsigned char                         sw_version[VOIP_SW_VERSION_SIZE]; /* The IAD software version */
    unsigned char                         sw_time[VOIP_SW_TIME_SIZE];       /* The LAD software time */
    unsigned char                         user_count;                       /* The VOIP user count */
}CTC_STACK_voip_iad_info_t;

typedef enum
{
    CTC_STACK_VOIP_VOICE_IP_STATIC            = 0,
    CTC_STACK_VOIP_VOICE_IP_DHCP              = 1,
    CTC_STACK_VOIP_VOICE_IP_PPPOE             = 2
}CTC_STACK_voice_ip_mode_t;

typedef enum
{
    CTC_STACK_PPPOE_MODE_AUTO                 = 0,           
    CTC_STACK_PPPOE_MODE_CHAP                 = 1,            /* Change Handshake Authentication Protocol */
    CTC_STACK_PPPOE_MODE_PAP                  = 2             /* Password Authentication Protocol */
}CTC_STACK_pppoe_mode_t;

typedef enum
{
    CTC_STACK_VOIP_TAGGED_FLAG_TRANSPARENT    = 0,           
    CTC_STACK_VOIP_TAGGED_FLAG_TAG            = 1,       
    CTC_STACK_VOIP_TAGGED_FLAG_VLAN_STACK     = 2  
}CTC_STACK_voip_tag_flag_t;

#define VOIP_PPPOE_USER_SIZE			        32
#define VOIP_PPPOE_PASSWD_SIZE			        32

typedef struct
{
    CTC_STACK_voice_ip_mode_t             voice_ip_mode;                        /* The IP address configured mode */
    unsigned long                         iad_ip_addr;                          /* The IAD IP address */
    unsigned long                         iad_net_mask;                         /* The IAD NET mask */
    unsigned long                         iad_def_gw;                           /* The IAD default gateway */
    CTC_STACK_pppoe_mode_t                pppoe_mode;                           /* The PPPOE mode */
    unsigned char                         pppoe_user[VOIP_PPPOE_USER_SIZE];     /* The PPPOE user name */
    unsigned char                         pppoe_passwd[VOIP_PPPOE_PASSWD_SIZE]; /* The PPPOE password */
    CTC_STACK_voip_tag_flag_t             tag_flag;                             /* Voice data transmit with flag */
    unsigned short                        cvlan_id;                             /* The CVLAN of voice data */
    unsigned short                        svlan_id;                             /* The SVLAN of voice data */
    unsigned char                         priority;                             /* The voice priority */
}CTC_STACK_voip_global_param_conf_t;

typedef enum
{
    CTC_STACK_H248_HEART_BEAT_MODE_OFF             = 0,
    CTC_STACK_H248_HEART_BEAT_MODE_CTC_STO         = 1,

    CTC_STACK_H248_HEART_BEAT_MODE_OTHER           = 0xFF
}CTC_STACK_H248_heart_beat_mode_t;

typedef enum
{
    CTC_STACK_H248_REG_MODE_IP                    = 0,
    CTC_STACK_H248_REG_MODE_DOMAIN                = 1,
    CTC_STACK_H248_REG_MODE_DEVICE_NAME           = 2
}CTC_STACK_H248_reg_mode_t;

typedef enum
{
    CTC_STACK_H248_ACTIVE_MGC_BACKUP 		= 0,
    CTC_STACK_H248_ACTIVE_MGC_PRIMARY 		= 1
}CTC_STACK_H248_active_mgc_t;

#define H248_MID_SIZE			            64

typedef struct
{
    unsigned short                        	mg_port;                                  	/* MG port number */
    unsigned long                        	mgcip;                                    	/* Primary softswitch platform IP address */
    unsigned short                        	mgccom_port_num;                          	/* Primary softswitch platform port number */
    unsigned long                         	back_mgcip;                               	/* Backup softswitch platform IP address */
    unsigned short                        	back_mgccom_port_num;                     	/* Backup softswitch platform port number */
    CTC_STACK_H248_active_mgc_t             active_mgc;                               	/* Active MGC that ONU registered, 0x00-backup, 0x01:primary*/
    CTC_STACK_H248_reg_mode_t             	reg_mode;                                 	/* Register mode:default MGCP domain name using mode */
    unsigned char                         	mid[H248_MID_SIZE];                       	/* MG domain name */
    CTC_STACK_H248_heart_beat_mode_t       	heart_beat_mode;            				/* Heart beat mode, default is 0x01 */
    unsigned short                        	heart_beat_cycle;                         	/* Heartbeat Cycle,default is 60s */
    unsigned char                         	heart_beat_count;                         	/* Heartbeat detect quantity,default is 3 times */
}CTC_STACK_h248_param_config_t;

#define H248_USER_TID_NAME_SIZE	        	32

typedef struct
{
    unsigned char                         	user_tid_name[H248_USER_TID_NAME_SIZE];      /* User TID name */
}CTC_STACK_h248_user_tid_config_t;

typedef struct
{
  CTC_management_object_index_t	          	management_object_index;
  CTC_STACK_h248_user_tid_config_t          h248_user_tid_config;

}CTC_STACK_h248_user_tid_config_object_t;

typedef CTC_STACK_h248_user_tid_config_object_t CTC_STACK_h248_user_tid_config_array_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];


#define H248_RTP_TID_PREFIX_SIZE	        16
#define H248_RTP_TID_DIGIT_SIZE              8

typedef enum
{
    CTC_STACK_H248_RTP_TID_MODE_ALIGNED   	= 0x00,
    CTC_STACK_H248_RTP_TID_MODE_UNALIGNED   = 0x01
}CTC_STACK_h248_rtp_tid_mode_t;

typedef struct
{
    unsigned char                       num_of_rtp_tid;                                     /* number of RTP TID*/
    unsigned char                       rtp_tid_prefix[H248_RTP_TID_PREFIX_SIZE];        	/* RTP prefix(only for H.248) */
    unsigned char                       rtp_tid_digit_begin[H248_RTP_TID_DIGIT_SIZE]; 		/* RTP TID digit begin value */
    CTC_STACK_h248_rtp_tid_mode_t		rtp_tid_mode;                                       /* RTP TID digit align mode */
    unsigned char                       rtp_tid_digit_length;                               /* RTP TID digit length, valid when mode=0*/
}CTC_STACK_h248_rtp_tid_config_t;

#define H248_RTP_TID_NAME_SIZE	        32

typedef struct
{
    unsigned char                         num_of_rtp_tid;                              /* number of RTP TID*/
    unsigned char                         rtp_tid_name[H248_RTP_TID_NAME_SIZE];        /* RTP TID name(only for H.248) */
 }CTC_STACK_h248_rtp_tid_info_t;

typedef enum
{
    CTC_STACK_SIP_HEART_BEAT_ON                   = 0,
    CTC_STACK_SIP_HEART_BEAT_OFF                  = 1
}CTC_STACK_SIP_heart_beat_t;

typedef struct
{
    unsigned short                        mg_port;             			/* MG port number */
    unsigned long                         server_ip;           			/* Active SIP agent server IP address  */
    unsigned short                        serv_com_port;       			/* Active SIP agent server port number */
    unsigned long                         back_server_ip;      			/* Standby SIP agent server IP address */
    unsigned short                        back_serv_com_port;  			/* Standby SIP agent server port number */
    unsigned long                         active_proxy_server;  		/* SIP Proxy that ONU registered*/
    unsigned long                         reg_server_ip;           		/* Active SIP register server IP address  */
    unsigned short                        reg_serv_com_port;      		/* Active SIP register server port number */
    unsigned long                         back_reg_server_ip;      		/* Standby SIP register server IP address */
    unsigned short                        back_reg_serv_com_port;  		/* Standby SIP register server port number */
    unsigned long                         outbound_server_ip;           /* OutBound server IP address */
    unsigned short                        outbound_serv_com_port;       /* OutBound server port number */
    unsigned long                         reg_interval;        			/* Register refresh period, unit is second */
    CTC_STACK_SIP_heart_beat_t            heart_beat_switch;   			/* SIP Heartbeat enable switch,0-open,1-close */
    unsigned short                        heart_beat_cycle;    			/* Heartbeat Cycle,default is 60s */
    unsigned short                        heart_beat_count;    			/* Heartbeat detect quantity,default is 3 times */
}CTC_STACK_sip_param_config_t;

#define SIP_PORT_NUM_SIZE			      16
#define SIP_USER_NAME_SIZE			      32
#define SIP_PASSWD_SIZE			          16

typedef struct
{
    unsigned char                         sip_port_num[SIP_PORT_NUM_SIZE];        /* SIP port phone number */
    unsigned char                         user_name[SIP_USER_NAME_SIZE];          /* SIP port user name */
    unsigned char                         passwd[SIP_PASSWD_SIZE];                /* SIP port keyword */
}CTC_STACK_sip_user_param_config_t;

typedef struct
{
  CTC_management_object_index_t	          		management_object_index;
  CTC_STACK_sip_user_param_config_t          	sip_user_param_config;

}CTC_STACK_sip_user_param_config_object_t;

typedef CTC_STACK_sip_user_param_config_object_t CTC_STACK_sip_user_param_config_array_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];

typedef enum
{
    CTC_STACK_VOIP_FAX_TRANSPARENT                = 0,           
    CTC_STACK_VOIP_FAX_T38                        = 1
}CTC_STACK_voip_fax_t38_t;

typedef enum
{
    CTC_STACK_VOIP_FAX_CONTROL_NEGOTIATED         = 0,
    CTC_STACK_VOIP_FAX_CONTROL_AUTO_VBD           = 1
}CTC_STACK_voip_fax_control_t;

typedef struct
{
    CTC_STACK_voip_fax_t38_t              t38_enable;          /* voice T38 enable */
    CTC_STACK_voip_fax_control_t          fax_control;         /* voice Fax control */
}CTC_STACK_voip_fax_config_t;

typedef enum
{
    CTC_STACK_VOIP_IAD_OPER_STATUS_REGISTERING    = 0,
    CTC_STACK_VOIP_IAD_OPER_STATUS_REGISTERED     = 1,
    CTC_STACK_VOIP_IAD_OPER_STATUS_FAULT          = 2,
    CTC_STACK_VOIP_IAD_OPER_STATUS_DEREGISTER     = 3,
    CTC_STACK_VOIP_IAD_OPER_STATUS_REBOOT         = 4,

    CTC_STACK_VOIP_IAD_OPER_STATUS_OTHER          = 255
}CTC_STACK_voip_iad_oper_status_t;

typedef enum
{
    CTC_STACK_VOIP_IAD_PORT_STATUS_REGISTERING    = 0,
    CTC_STACK_VOIP_IAD_PORT_STATUS_IDLE           = 1,
    CTC_STACK_VOIP_IAD_PORT_STATUS_OFFHOOK        = 2,
    CTC_STACK_VOIP_IAD_PORT_STATUS_DAILING        = 3,
    CTC_STACK_VOIP_IAD_PORT_STATUS_RINGING        = 4,
    CTC_STACK_VOIP_IAD_PORT_STATUS_RINGBACK       = 5,
    CTC_STACK_VOIP_IAD_PORT_STATUS_CONNECTING     = 6,
    CTC_STACK_VOIP_IAD_PORT_STATUS_CONNECTED      = 7,
    CTC_STACK_VOIP_IAD_PORT_STATUS_DISCONNECTING  = 8,
    CTC_STACK_VOIP_IAD_PORT_STATUS_REGISTER_FAILED= 9,
    CTC_STACK_VOIP_IAD_PORT_STATUS_INACTIVE       = 10
}CTC_STACK_voip_iad_port_status_t;

typedef enum
{
    CTC_STACK_VOIP_IAD_PORT_SERVICE_STATUS_END_LOCAL    = 0,
    CTC_STACK_VOIP_IAD_PORT_SERVICE_STATUS_END_REMOTE   = 1,
    CTC_STACK_VOIP_IAD_PORT_SERVICE_STATUS_END_AUTO     = 2,
    CTC_STACK_VOIP_IAD_PORT_SERVICE_STATUS_NORMAL       = 3
}CTC_STACK_voip_iad_port_service_status_t;

typedef enum
{
    CTC_STACK_VOIP_IAD_PORT_CODEC_G711A           = 0,
    CTC_STACK_VOIP_IAD_PORT_CODEC_G729            = 1,
    CTC_STACK_VOIP_IAD_PORT_CODEC_G711U           = 2,
    CTC_STACK_VOIP_IAD_PORT_CODEC_G723            = 3,
    CTC_STACK_VOIP_IAD_PORT_CODEC_G726            = 4,
    CTC_STACK_VOIP_IAD_PORT_CODEC_T38             = 5
}CTC_STACK_voip_iad_port_codec_mode_t;


typedef struct
{
    CTC_STACK_voip_iad_port_status_t          port_status;    /* User port status */
    CTC_STACK_voip_iad_port_service_status_t  service_status; /* User port service type */
    CTC_STACK_voip_iad_port_codec_mode_t      codec_mode;     /* User port codec mode */
}CTC_STACK_voip_pots_status_t;

typedef struct
{
  CTC_management_object_index_t	          	management_object_index;
  CTC_STACK_voip_pots_status_t          	pots_status;

}CTC_STACK_voip_pots_status_object_t;

typedef CTC_STACK_voip_pots_status_object_t CTC_STACK_voip_pots_status_array_t[MAX_MANAGEMENT_OBJECT_ENTRIES_NUMBER];

typedef enum
{
    CTC_STACK_OPERATION_TYPE_REGISTER                 		= 0,
    CTC_STACK_OPERATION_TYPE_DEREGISTER               		= 1,
    CTC_STACK_OPERATION_TYPE_RESET                    		= 2
}CTC_STACK_operation_type_t;

#define CTC_STACK_SIP_DIGIT_MAP_BLOCK_SIZE    125
#define CTC_STACK_SIP_DIGIT_MAP_TLV_SIZE      131 
#define CTC_STACK_SIP_DIGIT_MAP_MAX_BLOCK     255 
typedef struct
{
    unsigned char* digital_map;
	unsigned long  digital_map_length;
}CTC_STACK_SIP_digit_map_t;

typedef enum
{
    CTC_STACK_ONU_TYPE_SFU                        			= 0,
    CTC_STACK_ONU_TYPE_HGU                        			= 1,
    CTC_STACK_ONU_TYPE_SBU                        			= 2,
    CTC_STACK_ONU_TYPE_CASSETE_MDU_ETH                    	= 3,
    CTC_STACK_ONU_TYPE_SMALL_PLUG_IN_CARD_MDU_ETH          	= 4,
    CTC_STACK_ONU_TYPE_SMALL_PLUG_IN_CARD_MDU_DSL          	= 5,
    CTC_STACK_ONU_TYPE_MIDDLE_PLUG_IN_CARD_MDU_DSL  		= 6,
    CTC_STACK_ONU_TYPE_MIXED_PLUG_IN_CARD_MDU_ETH_DSL       = 7,
    CTC_STACK_ONU_TYPE_MTU                        			= 8    
}CTC_STACK_onu_type_t;


#define    CTC_STACK_ONU_INVALID_MULTI_LLID_VALUE                0
#define    CTC_STACK_ONU_SUPPORT_ONLY_SINGLE_LLID                1

typedef enum
{
    CTC_STACK_ONU_MODE_GE_TYPE                        		= 0,
    CTC_STACK_ONU_MODE_FE_TYPE                        		= 1,
    CTC_STACK_ONU_MODE_VOIP_TYPE                        	= 2,
    CTC_STACK_ONU_MODE_TDM_TYPE                        		= 3,
    CTC_STACK_ONU_MODE_ADSL2_TYPE                        	= 4,
    CTC_STACK_ONU_MODE_VDSL2_TYPE                 			= 5,
    CTC_STACK_ONU_MODE_WLAN_TYPE                  			= 6,
    CTC_STACK_ONU_MODE_USB_TYPE                   			= 7,
    CTC_STACK_ONU_MODE_CATV_RF_TYPE               			=8
}CTC_STACK_onu_interface_type_t;

typedef enum
{
    CTC_STACK_ONU_PROTECTION_NOT_SUPPORTED                	= 0,
    CTC_STACK_ONU_PROTECTION_TYPE_C                        	= 1,
    CTC_STACK_ONU_PROTECTION_TYPE_D                        	= 2
} CTC_STACK_protection_type_t;


#define CTC_STACK_CAPABILITY_MAX_INTERFACE_TYPE_NUM    9

typedef struct
{
    CTC_STACK_onu_interface_type_t       interface_type;
    unsigned short                       num_of_port;
 } CTC_STACK_onu_capability_interface_type_t;

typedef struct
{
    CTC_STACK_onu_type_t                        onu_type;
    unsigned char                               multi_llid;
    CTC_STACK_protection_type_t                 protection_type;
    unsigned char                               num_of_pon_if;
    unsigned char                               num_of_slot;
    unsigned char                               num_of_infc_type;
    CTC_STACK_onu_capability_interface_type_t   interface_type[CTC_STACK_CAPABILITY_MAX_INTERFACE_TYPE_NUM];
    bool                                        battery_backup;
} CTC_STACK_onu_capabilities_2_t;

typedef enum
{
    CTC_STACK_ONU_IPv6_AWARE_NOT_SUPPORTED	= 0x00,
    CTC_STACK_ONU_IPv6_AWARE_SUPPORTED		= 0x01
} CTC_STACK_onu_ipv6_aware_support_t;


typedef enum
{
    CTC_STACK_ONU_POWER_SUPPLY_CONTROL_NOT_SUPPORTED	= 0x00,
    CTC_STACK_ONU_POWER_SUPPLY_CONTROL_TX_ONLY		= 0x01,
    CTC_STACK_ONU_POWER_SUPPLY_CONTROL_TX_RX		= 0x02
} CTC_STACK_onu_power_supply_control_t;

typedef struct
{
    CTC_STACK_onu_ipv6_aware_support_t onu_ipv6_aware;
    CTC_STACK_onu_power_supply_control_t onu_power_supply_control;    
} CTC_STACK_onu_capabilities_3_t;

typedef struct
{
    unsigned long                         mng_ip;  			/* The ip for manage */
    unsigned long                         mng_mask;  		/* The mask for manage*/
    unsigned long                         mng_gw;  			/* The gateway */
    unsigned short                        data_cvlan;		/* The data cvlan */
    unsigned short                        data_svlan; 		/* The data svlan */
    unsigned char                         data_priority; 	/* The data priority */
} CTC_STACK_mxu_mng_global_parameter_config_t;

typedef struct
{
    CTC_STACK_mxu_mng_global_parameter_config_t ipv4_param;
    PON_ipv6_addr_t			mng_ipv6;
	unsigned char				mng_ipv6_prefix_len;
    PON_ipv6_addr_t			mng_gw_ipv6;
    CTC_STACK_ip_version_t		ip_flag;
} CTC_STACK_mxu_mng_global_parameter_config_ipv6_ext_t;


#define SECURITY_NAME_SIZE  32
#define COMMUNITY_FOR_READ_SIZE 32
#define COMMUNITY_FOR_WRITE_SIZE 32
#define CTC_STACK_MXU_MNG_SNMP_MAX_STRING_LEN SECURITY_NAME_SIZE	/*modify this macro if necessary*/

typedef struct
{
    unsigned char                         snmp_ver;  										/* The ip for manage */
    unsigned long                         trap_host_ip; 									/* The mask for manage*/
    unsigned short                        trap_port; 										/* Trap port */
    unsigned short                        snmp_port; 										/* snmp server port */
    unsigned char                         security_name[SECURITY_NAME_SIZE]; 				/* The data svlan */
    unsigned char                         community_for_read[COMMUNITY_FOR_READ_SIZE]; 		/* name of read community */
    unsigned char                         community_for_write[COMMUNITY_FOR_WRITE_SIZE];	/* name of write community */
} CTC_STACK_mxu_mng_snmp_parameter_config_t;

typedef struct
{
    CTC_STACK_mxu_mng_snmp_parameter_config_t	ipv4_param;
    PON_ipv6_addr_t		trap_host_ipv6;
    CTC_STACK_ip_version_t					ip_flag;
} CTC_STACK_mxu_mng_snmp_parameter_config_ipv6_ext_t;


/* C7-0x0017  CTC_EX_VAR_PORT_LOOP_DETECT */
typedef enum
{
    CTC_STACK_PORT_LOOP_DETECT_STATE_DEACTIVATED	= 0x00000001,
    CTC_STACK_PORT_LOOP_DETECT_STATE_ACTIVATED		= 0x00000002,
} CTC_STACK_port_loop_detect_on_off_state_t;

/* C7-0x0008 CTC_EX_VAR_HOLDOVER_CONFIG */
typedef enum
{
    CTC_STACK_HOLDOVER_STATE_DEACTIVATED	= 0x00000001,
    CTC_STACK_HOLDOVER_STATE_ACTIVATED		= 0x00000002,
} CTC_STACK_holdover_on_off_state_t;

typedef struct
{
    CTC_STACK_holdover_on_off_state_t       holdover_state;  		/* Holdover state*/
    unsigned long                         	holdover_time;  		/* Holdover time*/
} CTC_STACK_holdover_state_t;

/* C7-0x000B CTC_EX_VAR_PON_IF_ADMIN_STATE */
typedef struct
{
    unsigned char                        	active_pon_if;  			/* Active PON_IF */
} CTC_STACK_active_pon_if_state_t;


typedef enum
{
    CTC_DISCOVERY_STATE_IDLE,
    CTC_DISCOVERY_STATE_IN_PROGRESS,
    CTC_DISCOVERY_STATE_UNSUPPORTED,
    CTC_DISCOVERY_STATE_COMPLETE,
}CTC_STACK_discovery_state_t;

#define CTC_AUTH_ONU_ID_SIZE       24
#define CTC_AUTH_PASSWORD_SIZE     20/*12   */  /*modi by luh for gpon support*/

#define CTC_AUTH_REQUEST_DATA_SIZE      0x01 
#define CTC_AUTH_RESPONSE_DATA_SIZE     0x25 
#define CTC_AUTH_NAK_RESPONSE_DATA_SIZE 0x02
#define CTC_AUTH_SUCCESS_DATA_SIZE      0x00 
#define CTC_AUTH_FAILURE_DATA_SIZE      0x01 
#define CTC_AUTH_LOID_DATA_NOT_USED     "NOT_USED" 

typedef enum
{
    CTC_AUTH_TYPE_LOID                  = 0x01,   /* ONU_ID+Password mode */
    CTC_AUTH_TYPE_NOT_SUPPORTED         = 0x02    /* ONU not supported or can not accept the required authentication type */
}CTC_STACK_auth_type_t;

typedef enum
{
    CTC_AUTH_SUCCESS = 0,	/* added by xieshl 20100610, 类型扩展，解决注册成功情况下仍报失败问题 */
    CTC_AUTH_FAILURE_ONU_ID_NOT_EXIST 	= 0x01,   /* ONU_ID is not exist */
    CTC_AUTH_FAILURE_WRONG_PASSWORD   	= 0x02,   /* ONU_ID is exist but the Password is wrong */
    CTC_AUTH_FAILURE_ONU_ID_USED   		= 0x03    /* ONU_ID is already used */
}CTC_STACK_auth_failure_type_t;

typedef struct  
{
    unsigned char                   onu_id[CTC_AUTH_ONU_ID_SIZE+1];
    unsigned char                   password[CTC_AUTH_PASSWORD_SIZE+1];
}CTC_STACK_auth_loid_data_t;

typedef struct
{
	CTC_STACK_auth_type_t		    auth_type;
    CTC_STACK_auth_loid_data_t      loid_data;
	CTC_STACK_auth_type_t		    desired_auth_type;
} CTC_STACK_auth_response_t;

typedef enum
{
    CTC_AUTH_MODE_MAC,               /* MAC mode */
    CTC_AUTH_MODE_LOID,              /* ONU_ID+Password mode */
    CTC_AUTH_MODE_HYBRID,            /* hybrid mode */
    CTC_AUTH_MODE_LOID_NOT_CHK_PWD,  /* ONU_ID+Password but not check password mode */
    CTC_AUTH_MODE_HYBRID_NOT_CHK_PWD /* hybrid mode but not check password mode */
}CTC_STACK_auth_mode_t;

typedef struct  
{
    CTC_STACK_auth_loid_data_t      loid[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    PON_onu_id_t                    onu_llid[PON_ONU_ID_PER_OLT_ARRAY_SIZE];
    CTC_STACK_auth_mode_t           auth_mode;
}authentication_olt_database_t;

typedef enum
{
    CTC_STACK_STATE_VALID,
    CTC_STACK_STATE_RECEIVED_FRAME,
    CTC_STACK_STATE_WAITING_FOR_RESPONSE,
}ctc_stack_response_encryption_state_t;

#define CTC_ENCRYPTION_KEY_SIZE           3	/* CTC encryption key size measured in Bytes */
#ifndef __PASSAVE_ONU__
typedef unsigned char CTC_encryption_key_t[CTC_ENCRYPTION_KEY_SIZE];
#endif	/* __PASSAVE_ONU__ */

typedef enum
{
    CTC_ENCRYPTION_STATE_NOT_ENCRYPTED,
    CTC_ENCRYPTION_STATE_ENCRYPTION_COMPLETE,
    CTC_ENCRYPTION_STATE_ENCRYPTION_FAILED,
    CTC_ENCRYPTION_STATE_WAITING_FOR_ENCRYPTION,
}CTC_STACK_encryption_state_t;

typedef struct  
{
    ctc_stack_response_encryption_state_t response_encryption_state;
    PON_encryption_key_index_t            key_index;
	PON_encryption_key_index_t            temp_key_index;
    CTC_encryption_key_t                  key_value;
    bool                                  start_encryption_by_the_user;
    CTC_STACK_encryption_state_t          encryption_state;
    short int                             number_of_sending_request;
	unsigned char						  key_request_received_oam_pdu_data[100];
	unsigned short						  key_request_received_length;
}CTC_STACK_encryption_onu_database_t;

/*for updating redundancy_database*/
typedef struct
{
    CTC_STACK_discovery_state_t           state; 
    unsigned char					      common_version;
    unsigned short					      maximum_data_frame_size;
    authentication_olt_database_t         auth_db;
    CTC_STACK_encryption_onu_database_t   enc_db;
    /* B--modified by liwei056@2011-4-21 for CodeCheck */
#if 1
    PON_encryption_type_t                 encryption_type;
#else
    PON_encryption_key_index_t            encryption_type;
#endif
    /* E--modified by liwei056@2011-4-21 for CodeCheck */
    OSSRV_semaphore_t                     enc_semaphore;
} CTC_STACK_redundancy_database_t;

typedef enum
{
    CTC_OPTICAL_TRANSMITER_ACTIVE = 0x00000000,
    CTC_OPTICAL_TRANSMITER_STANDBY = 0x00000001,
    CTC_OPTICAL_TRANSMITER_BOTH = 0x00000002
}CTC_STACK_optical_transmiter_id_t;

#define CTC_OPTICAL_TX_ACTION_REENABLE   0
#define CTC_OPTICAL_TX_ACTION_PERMANENTLY_SHUTDOWN  65535

typedef struct
{
    mac_address_t		onu_sn;
    CTC_STACK_optical_transmiter_id_t       optical_id;
    unsigned long                           action;
} CTC_STACK_onu_tx_power_supply_control_t;
#endif	/* __CTC_VAR_DESC_DEF_EXPO_H__ */

