/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
/*此文件为PMC8411适配接口新增*/
#ifndef  __INCLUDEFROMPAS8411_H
#define __INCLUDEFROMPAS8411_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*============================= Include Files ===============================*/

#include "includeFromPas.h"


/*Begin:PAS 8411相关*/
#if 1
typedef signed long     INT32S; /* Signed 32 bit quantity */

#define PON_DBA_VERSION_NAME_LENGTH     80
#define PON_DBA_VERSION_COMMENT_LENGTH  120

#define PON_CPU_DDR_LANE_TOF_PARAMS_NUMBER				3
#define PON_PACKET_BUFFER_DDR_LANE_TOF_PARAMS_NUMBER	8

#define SCHED_SIMPLE_DATA_ARRAY_SIZE            8
typedef struct PON_dba_version_t
{
	INT8U	name[PON_DBA_VERSION_NAME_LENGTH]; /*!< The DBA full name */
	INT16U	major;
	INT16U	minor;
	INT16U	build;
	INT16U	maintenance;
    INT8U	comment[PON_DBA_VERSION_COMMENT_LENGTH]; /*!< Additional text description (CTC, FEC, something special about this version) */

} PON_dba_version_t;

/* Optics type (1G / 10G) */
typedef enum PON_pon_type
{
    PON_1G     = 0,    /* 1G EPON optic */
    PON_10G	   = 1,    /* 10G EPON optic */
    PON_1G_10G = 2     /* Both 1G and 10G EPON optics */
} PON_pon_type;



/* NE_addr_config_e */
typedef enum NE_addr_config_e
{
    NE_ADDR_CONFIG_AGE_TIMER  = 0,   /* This field set the address table aging timer in millisecond. */
    NE_ADDR_CONFIG_AGE_REMOVE = 1,   /* Remove or not address with maximum age. */
    NE_ADDR_CONFIG_ENABLE     = 3,   /* Enable or disable the use of address table. */
    NE_ADDR_CONFIG_FULL_MODE  = 4,   /* Set the behavior of addres table when hash line is full. */
	NE_ADDR_CONFIG_LIMIT_STATIC = 5, /* Set the limit to include static address */
} NE_addr_config_e;


/* The behavior of addres table when hash line is full */
typedef enum NE_addr_full_mode_e
{
    NE_ADDR_FULL_MODE_DELETE_LRU,    /* When address table hash line is full discard the LRU value. */
    NE_ADDR_FULL_MODE_DO_NOTHING,    /* When address table hash line is full don't do any operation. */
    NE_ADDR_FULL_MODE_SEND_EVENT     /* When address table hash line is full send event to the host. */
} NE_addr_full_mode_e;


/* DDR interface type */
typedef enum PON_ddr_interface_type_e
{
	PON_DDR_TYPE_CPU,
    PON_DDR_TYPE_PACKET_BUFFER,
} PON_ddr_interface_type_e;

/* DDR bus width */
typedef enum PON_ddr_bus_width_e
{
	PON_DDR_BUS_WIDTH_32 = 0,
    PON_DDR_BUS_WIDTH_64,
	PON_DDR_BUS_WIDTH_16
} PON_ddr_bus_width_e;

/* DDR size */
typedef enum PON_ddr_size_e
{
	PON_DDR_SIZE_1024_MBITS = 1024,
	PON_DDR_SIZE_2048_MBITS = 2048
} PON_ddr_size_e;

/* DDR rate */
typedef enum PON_ddr_rate_e
{
	PON_DDR_RATE_1066 = 1066, /* default for CPU DDR */
	PON_DDR_RATE_1333 = 1333, /* default for PACKET_BUFFER DDR */
	PON_DDR_RATE_1600 = 1600
} PON_ddr_rate_e;

/* DDR RAM test */
typedef enum PON_ddr_ram_test_e
{
	PON_DDR_RAM_NO_TEST,
	PON_DDR_RAM_CONTROLLER_TEST,
	PON_DDR_RAM_SW_TEST
} PON_ddr_ram_test_e;

/* DDR time of flight param */
typedef struct PON_ddr_lane_tof_param_s
{
    INT32S  tof_ck;
    INT32S  tof_dqs;
} PON_ddr_lane_tof_param_s;

typedef union PON_ddr_lane_tof_params_u
{
	PON_ddr_lane_tof_param_s cpu_ddr_lane_tof_params[PON_CPU_DDR_LANE_TOF_PARAMS_NUMBER];
	PON_ddr_lane_tof_param_s packet_buffer_ddr_lane_tof_params[PON_PACKET_BUFFER_DDR_LANE_TOF_PARAMS_NUMBER];
} PON_ddr_lane_tof_params_u;

typedef struct PON_ddr_parameters_s
{
	PON_ddr_size_e				ddr_size;			/* DDR size in Mbits. */
	PON_ddr_rate_e				mt;					/* The data rate per pin in MT. */
	PON_ddr_bus_width_e			bus_configuration;	/* DDR bus configuration. */
	INT32U						cas_latency;		/* The CAS latency in DDR clock. */
	INT32U						trp_delay;			/* TRP delay in ns. */
	INT32U						trcd_delay;			/* TRCD delay in ns. */
	INT32U						trc_delay;			/* TRC delay in ns. */
	bool						ecc;				/* Support DDR Error Correction Code. */
	PON_ddr_lane_tof_params_u	lane_tof_params;	/* Time of flight parameters */
	PON_ddr_ram_test_e			ram_test;			/* Choose if to do RAM test. */
} PON_ddr_parameters_s;

typedef enum {
	PON_OLT_BAUD_RATE_1200,
	PON_OLT_BAUD_RATE_2400,
	PON_OLT_BAUD_RATE_4800,
	PON_OLT_BAUD_RATE_9600,
	PON_OLT_BAUD_RATE_14400,
	PON_OLT_BAUD_RATE_19200,
	PON_OLT_BAUD_RATE_28800,
	PON_OLT_BAUD_RATE_38400,
	PON_OLT_BAUD_RATE_57600,
	PON_OLT_BAUD_RATE_115200,
	PON_OLT_BAUD_RATE_230400,
	PON_OLT_BAUD_RATE_460800 
}PON_OLT_BAUD_RATE;
typedef struct PON_new_olt_response_parameters_t  
{
  PON_update_olt_parameters_t		  update_olt_parameters;
  PON_external_downlink_buffer_size_t memory_size;
  PON_ddr_bus_width_e                 bus_width;
  mac_address_t                       olt_mac_address;

} PON_new_olt_response_parameters_t;


typedef enum 
{
	TO_SPECIFIED_LLID = 0,
	TO_ALL_BUT_SPECIFIED_LLID,
	PON_EVERY_1G_LLID,
	PON_EVERY_10G_LLID
} PON_broadcast_type_e;

typedef enum PON_link_status_e
{
	PON_LINK_STATUS_DOWN = 0,
    PON_LINK_STATUS_UP
} PON_link_status_e; 

/* NE_direction_e */
/* ============== */
typedef enum NE_direction_e
{
    NE_DIRECTION_DOWN   = 0,    /* Downstream direction. */
    NE_DIRECTION_UP     = 1,    /* Upstream direction. */
} NE_direction_e;

/* NE_VLAN_type_e */
typedef enum NE_VLAN_type_e
{
    NE_VLAN_TYPE_V8100 = 0,      /* VLAN Ethernet type is 0x8100 */
    NE_VLAN_TYPE_V9100 = 1,      /* VLAN Ethernet type is 0x9100 */
    NE_VLAN_TYPE_V88A8 = 2,      /* VLAN Ethernet type is 0x88a8. */
    NE_VLAN_TYPE_CUSTOM1 = 4,    /* VLAN Ethernet type is custom Ethernet type number 1. */
    NE_VLAN_TYPE_CUSTOM2 = 5,    /* VLAN Ethernet type is custom Ethernet type number 2. */
    NE_VLAN_TYPE_CUSTOM3 = 6,    /* VLAN Ethernet type is custom Ethernet type number 3. */
    NE_VLAN_TYPE_CUSTOM4 = 7,    /* VLAN Ethernet type is custom Ethernet type number 4. */
    NE_VLAN_TYPE_CUSTOM5 = 8     /* VLAN Ethernet type is custom Ethernet type number 5. */
} NE_VLAN_type_e;




typedef enum PON_reset_scheme_e
{
    PON_RST_SCHEME_SD_BASED,
	PON_RST_SCHEME_NON_SD_BASED,
	PON_RST_SINGLE
} PON_reset_scheme_e;

typedef enum PON_reset_per_rate_e
{
    PON_RST_UNIFIED,
    PON_RST_SEPARATE
} PON_reset_per_rate_e;

typedef enum PON_reset_stuck_value_e
{
    PON_RST_STUCK_AT_0,
    PON_RST_STUCK_AT_1,
    PON_RST_STUCK_AT_2
}PON_reset_stuck_value_e;

typedef enum PON_reset_rx_xsbi_mode_e
{
    PON_RST_RX_XSBI_APPLY,
    PON_RST_RX_XSBI_DONT_APPLY
}PON_reset_rx_xsbi_mode_e;

typedef struct 
{
    PON_reset_per_rate_e	sd_per_rate;        
    PON_reset_stuck_value_e ecdr_1g_lock_disc_mode; 
    INT8U					ecdr_1G_reset_gate_offset;      
    INT8U					ecdr_1G_reset_duration;             
    bool					ecdr_1G_use_reset;                  
    PON_reset_scheme_e      ecdr_1G_reset_scheme; 
    bool					ecdr_1G_use_lock_window;       
    INT8U                   ecdr_1G_discovery_left_shoulder; 
    INT8U                   ecdr_1G_discovery_right_shoulder; 
    bool					ecdr_1G_use_disc_envelop;       
    PON_polarity_t			ecdr_1G_reset_pol;         
    PON_polarity_t			ecdr_1G_disc_win_pol;      
}PAS_1G_10G_optics_configuration_t;


typedef struct 
{
	INT16U			discovery_mode_sync_time;		/* (TQ) SyncTime in Discovery mode 	The length of the preamble in Discovery window. The SyncTime must be long enough to fit the optics module settling time and the CDR lock time. 	*/
	INT16U			normal_mode_sync_time;			/* (TQ) SyncTime in Normal mode  The length of the preamble in regular traffic. The SyncTime must be long enough to fit the optics module settling time and the CDR lock time.		*/
	INT32U			allocation_offset;				/* (TQ) The offset for the reference point of the XCVR Reset and CDR Reset. (range 0-63) */
	INT16U			laser_on_time;					/* (TQ)	Laser on time adjustment to the ONUs. */
	INT16U			laser_off_time;					/* (TQ)	Laser off time adjustment to the ONUs.  */
	INT8U			xcvr_reset_gate_offset;			/* Optics module reset activation offset relatively to the reference point. */
	INT8U			xcvr_reset_duration;			/* Optics module reset assertion. */
	PON_polarity_t 	xcvr_reset_polarity;			/* Optics module reset pulse polarity (PON_POLARITY_ACTIVE_LOW: Active low, PON_POLARITY_ACTIVE_HIGH: Active high) */
	bool	        xcvr_use_reset;
	INT8U			cdr_reset_gate_offset;			/* CDR reset activation offset relatively to the reference point. */
	PON_polarity_t 	xcvr_sd_rx_polarity;			/* Laser Rx Signal Detect signal polarity (PON_POLARITY_ACTIVE_LOW: Active low, PON_POLARITY_ACTIVE_HIGH: Active high) */
	PON_reset_scheme_e xcvr_reset_scheme;			/* Which XCVR resetting scheme using during Discovery window */
	PON_reset_scheme_e cdr_reset_scheme;			/* Which CDR resetting scheme using during Discovery window */
	PON_polarity_t 	pon_tx_disable_line_polarity;	/* PON optics transmission signal polarity, This signal will always be enabled upon OLT initialization (PON_POLARITY_ACTIVE_LOW: Active low, PON_POLARITY_ACTIVE_HIGH: Active high) */
	INT16U			optics_dead_zone;				/* Minimal length between the end of a grant to the start of the other */
	INT8U			mac_active_data_window_left_shoulder;	/* MAC Active Data Window left shoulder length.  */
	INT8U			mac_active_data_window_right_shoulder;  /* MAC Active Data Window right shoulder length. */
	INT16U			cdr_lock_window_left_shoulder;			/* CDR Lock Window left shoulder length.  */
	INT16U			cdr_lock_window_right_shoulder;			/* CDR Lock Window right shoulder length. */
}PAS_1G_optics_configuration_t;


typedef struct 
{
    INT8U           cdr_10g_reset_gate_offset;		/* CDR reset activation offset relatively to the reference point. */
    INT8U           ecdr_10g_reset_gate_offset;		/* ECDR reset activation offset relatively to the reference point. */
    INT8U           ecdr_10g_reset_duration;	
    INT8U           xcvr_10g_reset_gate_offset;		/* Optics module reset activation offset relatively to the reference point. */
    INT8U           xcvr_10g_reset_duration;		/* Optics module reset assertion. */
    bool			xcvr_use_reset;     
    bool			ecdr_10g_use_reset;         
    PON_reset_scheme_e  cdr_reset_scheme;			/* Which CDR resetting scheme using during Discovery window */ 
    PON_reset_scheme_e  ecdr_10g_reset_scheme;		/* Which ECDR resetting scheme using during Discovery window */
    PON_reset_scheme_e  xcvr_reset_scheme;			/* Which XCDR resetting scheme using during Discovery window */
    INT16U          cdr_10g_lock_window_left_shoulder;  /* CDR_10G_LOCK_WINDOW_LEFT_SHOULDER  */
    INT16U          cdr_10g_lock_window_right_shoulder; /* CDR_10G_LOCK_WINDOW_RIGHT_SHOULDER */
    PON_reset_per_rate_e	 rx_lane_per_rate;			/* RX_LANE_PER_RATE */
    PON_reset_per_rate_e	 rx_rst_per_rate;			/* RX_RST_PER_RATE */ 
    PON_reset_rx_xsbi_mode_e rx_xsbi_mode;				/* RX_XSBI_MODE */ 
    bool					ecdr_10g_use_lock_window;   /* ECDR_10G_USE_LOCK_WINDOW */
    PON_reset_stuck_value_e ecdr_10g_lock_disc_mode;    /* ECDR_10G_LOCK_DISC_MODE */
    INT8U                   ecdr_10g_discovery_left_shoulder;  /* ECDR_10G_DISCOVERY_LEFT_SHOULDER */
    INT8U                   ecdr_10g_discovery_right_shoulder; /* ECDR_10G_DISCOVERY_RIGHT_SHOULDER */
    bool					ecdr_10g_use_disc_envelop;  /* ECDR_10G_USE_DISC_ENVELOP */
    PON_polarity_t			ecdr_10g_rst_pol;			/* ECDR_10G_RST_POLARITY */
    PON_polarity_t			ecdr_10g_disc_win_pol;      /* ECDR_10G_DISC_WIN_POLARITY */
    INT16U                  normal_10G_mode_sync_time;  /* 10G_NORMAL_MODE_SYNC_TIME */
    INT16U                  disc_10G_mode_sync_time;    /* 10G_DISCOVERY_MODE_SYNC_TIME */
    INT8U                   mac_10g_active_window_left_shoulder;	/* MAC Active Data Window left shoulder length.  */
    INT8U                   mac_10g_active_window_right_shoulder;   /* MAC Active Data Window right shoulder length. */
    INT8U                   xcvr_user_rate_select;
    INT32U                  pon_opt_rate_sel_left_shoulder;
    INT32U                  pon_opt_rate_sel_right_shoulder;
    INT16U                  optics_dead_zone_post_1G_to_10G;
    INT16U                  optics_dead_zone_post_1G_to_1G;
    INT16U                  optics_dead_zone_post_10G_to_1G;
    INT16U                  optics_dead_zone_post_10G_to_10G;
    INT16U                  optics_dead_zone_pre_1G;
    INT16U                  optics_dead_zone_pre_10G;
    INT32U                  cdr_rate_sel_left_shoulder;
    INT32U                  cdr_rate_sel_right_shoulder;
	PAS_1G_optics_configuration_t		pon_1g_optic;
	PAS_1G_10G_optics_configuration_t	pon_1g_10g_optic;
}PAS_10G_optics_configuration_t;

typedef union
{
	PAS_1G_optics_configuration_t  pon_1g_optics;
	PAS_10G_optics_configuration_t pon_10g_optics;
}PON_optics_param_u;


typedef enum PON_ref_clock_e
{
	PON_REF_CLOCK_NOT_INITIALIZED = -1,
	PON_REF_CLOCK_125_MHZ_SINGLE_ENDED,
	PON_REF_CLOCK_161_13_MHZ_DIFFERENTIAL,
	PON_REF_CLOCK_125_MHZ_DIFFERENTIAL,
	PON_REF_CLOCK_161_13_MHZ_SINGLE_ENDED,
	PON_REF_CLOCK_125_MHZ = PON_REF_CLOCK_125_MHZ_SINGLE_ENDED,
	PON_REF_CLOCK_161_13_MHZ = PON_REF_CLOCK_161_13_MHZ_DIFFERENTIAL

} PON_ref_clock_e;

typedef enum NE_ne_mode_e
{
	NE_MODE_MODE1   = 1,
	NE_MODE_DEFAULT = 2,
	NE_MODE_LAST
} NE_ne_mode_e;


typedef unsigned int PON_gpio_t ;

typedef enum
{
	PON_GPIO_POLARITY_HIGH_E,
	PON_GPIO_POLARITY_LOW_E
} PON_gpio_polarity_e;



typedef enum
{
	PON_GPIO_GRP_TYPE_1G_PON_E,
	PON_GPIO_GRP_TYPE_10G_PON_E,
	PON_GPIO_GRP_OTDR_PON_E,
	PON_GPIO_GRP_AVS_E,
	PON_GPIO_GRP_SLED_CONTROL_E,
	PON_GPIO_GRP_XAUI_REDN_E,
	PON_GPIO_GRP_EDK_1_E,
	PON_GPIO_GRP_EDK_2_E,
	PON_GPIO_GRP_EDK_3_E,
	PON_GPIO_GRP_LAST
} PON_gpio_grp_type_e;



typedef enum
{
	PON_GPIO_GRP_LEVEL_PRIMARY_E, 
	PON_GPIO_GRP_LEVEL_SECONDARY_E,
	PON_GPIO_GRP_LEVEL_NOT_RELEVANT_E,
	PON_GPIO_GRP_LEVEL_LAST
} PON_gpio_grp_level_e;


typedef struct  
{
	PON_gpio_t			module_exist;			/* Input							*/
    PON_gpio_polarity_e module_exist_polarity;	/* Module exist polarity - PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			i2c_data;				/* I2C data (Bi-directional)		*/
	PON_gpio_t			i2c_clock;				/* I2C clock (Output)				*/
	PON_gpio_t			tx_fault;				/* Input fault from optics (Input)	*/
    PON_gpio_polarity_e tx_fault_polarity;		/* Polarity of TX fault signal PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			tx_disable;				/* Output TX optics disable (Input) */
	PON_gpio_polarity_e tx_disable_polarity;	/* Polarity of  TX disable PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
}PON_gpio_1G_pon_map_t;


typedef struct  
{
	PON_gpio_t			module_exist;			/* Input							*/
    PON_gpio_polarity_e module_exist_polarity;	/* Module exist polarity - PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			i2c_data;				/* I2C data (Bi-directional)		*/
	PON_gpio_t			i2c_clock;				/* I2C clock (Output)				*/
	PON_gpio_t			mod_desel;				/* select/deselect module for I2C (Output) */
	PON_gpio_t			tx_1G_disable;			/* Output 1G TX optics disable (Input) */
	PON_gpio_polarity_e tx_1G_disable_polarity;	/* Polarity of 1G TX disable PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			tx_10G_disable;			/* Output 1G TX optics disable (Input) */
	PON_gpio_polarity_e tx_10G_disable_polarity;	/* Polarity of 1G TX disable PON_POLARITY_ACTIVE_LOW, PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			sqelch;					/* Output */
	PON_gpio_polarity_e sqelch_polarity;		/* Polarity of  sqelch PON_POLARITY_ACTIVE_LOW,  PON_POLARITY_ACTIVE_HIGH */
	PON_gpio_t			rx_loss;				/* Loss select , FOR 1G + 10G XFP selects the SD strobe meaning  - 10G loss or 1G loss (Input) */
}PON_gpio_10G_pon_map_t;

typedef struct  
{
	PON_gpio_t module_exist;			/* Input							*/
	PON_gpio_t i2c_data;				/* I2C data (Bi-directional)		*/
	PON_gpio_t i2c_clock;				/* I2C clock (Output)				*/
}PON_gpio_otdr_map_t;


typedef struct  
{
	PON_gpio_t d_1;						/* Used for  Adaptive Voltage Scaling coding */
	PON_gpio_t d_0;						/* Used for  Adaptive Voltage Scaling coding */
}PON_gpio_avs_map_t;

typedef struct  
{
	PON_gpio_t oen;						/* serival LED */
	PON_gpio_t clr;						/*  */
}PON_gpio_sled_map_t;
 

typedef struct
{
	PON_gpio_t switchover_indication; /* xaui switchover indication */
}PON_gpio_xaui_redn_map_t;

typedef union
{
	PON_gpio_1G_pon_map_t	  gpio_1G_pon_map;
	PON_gpio_10G_pon_map_t	  gpio_10G_pon_map;
	PON_gpio_otdr_map_t		  gpio_otdr_map;
	PON_gpio_avs_map_t		  gpio_avs_map;
	PON_gpio_sled_map_t		  gpio_sled_map;
	PON_gpio_xaui_redn_map_t  gpio_xaui_redn_map;
}PON_gpio_grp_map_value_u;

typedef enum TM_sched_model_e
{
    TM_SCHED_SIMPLE,              /* Simple schedule with 8 priorities per each port. This model supports 2 configuration internal memory or external memory for queues. */
    TM_SCHED_SIMPLE_POWERSAVE,    /* Simple schedule with 8 priorities with support to power save per LLID. This model supports 2 configuration internal memory or external memory for queues. */
    TM_SCHED_ADVANCE,             /* Schedule tree with up to 2040 queues split between 2 port and 2 priorities. */
	TM_SCHED_SIMPLE_10G_1G_CNI,   /* Same as TM_SCHED_SIMPLE with support for Sniff port as CNI. */
	TM_SCHED_SIMPLE_POWERSAVE_10G_1G_CNI,   /* Same as TM_SCHED_SIMPLE_POWERSAVE with support for Sniff port as CNI. */
} TM_sched_model_e;

/* TM_queue_type_e */
/* =============== */
typedef enum TM_queue_type_e
{
    TM_QUEUE_TYPE_INVALID,			/* Invalid type used to indicate when needed if queue is value is valid. like in TM_sched_mode */
    TM_QUEUE_TYPE_DATA,				/* Queue that used for data. */
    TM_QUEUE_TYPE_DATA_INTERNAL,	/* Queue that used for data and use internal memory. */
    TM_QUEUE_TYPE_DATA_FAST,		/* Queue that used for data with low latency. Frames are written to queue immediately after they receive. */
    TM_QUEUE_TYPE_MC,				/* Queue that use for multicast or broadcast. */
    TM_QUEUE_TYPE_MC_INTERNAL,		/* Queue that use for multicast or broadcast and use internal memory. */
    TM_QUEUE_TYPE_IMMEDIATELY,		/* Queue that send data immediately. This queue is using internal memory. It should be used for application like VoIP. */
    TM_QUEUE_TYPE_BESTEFFORT,		/* Queue that used for low priority data */
	TM_QUEUE_TYPE_DATA_MID,			/* Queue that used for mid priority data. This queue type is using external memory */
	TM_QUEUE_TYPE_DATA_HIGH,		/* Queue that used for high priority data. This queue type is using external memory */
	TM_QUEUE_TYPE_DATA_MEDIA,		/* This queue intend for using media like video conference */
	TM_QUEUE_TYPE_LAST
} TM_queue_type_e;


typedef struct TM_sched_simple_data_s
{
    TM_queue_type_e down_pon0_type[SCHED_SIMPLE_DATA_ARRAY_SIZE]; 
                                       /* The queue type per priority or TM_queue_type_invalid in case it not used.
                                          The queue_ids for place X is TM_queue_id_pon0_priorityX.
                                          Multicast types are not valid for these values. */

    TM_queue_type_e down_pon1_type[SCHED_SIMPLE_DATA_ARRAY_SIZE]; 
                                       /* The queue type per priority or TM_queue_type_invalid in case it not used.
                                          The queue_ids for place X is TM_queue_id_pon1_priorityX.
                                          Multicast types are not valid for these values   */                                       

    TM_queue_type_e down_cni0_type[SCHED_SIMPLE_DATA_ARRAY_SIZE]; 
                                       /* The queue type per priority or TM_queue_type_invalid in case it not used.
                                          The queue_ids for place X is TM_queue_id_cni0_priorityX.
                                          Multicast types are not valid for these values.*/
                                          
    TM_queue_type_e down_cni1_type[SCHED_SIMPLE_DATA_ARRAY_SIZE];  
                                       /* The queue type per priority or TM_queue_type_invalid in case it not used. 
                                          The queue_ids for place X is TM_queue_id_cni0_priorityX. 
                                          Multicast types are not valid for these values. */                                          
                                   
    TM_queue_type_e p2p_type[SCHED_SIMPLE_DATA_ARRAY_SIZE];  
                                       /* The queue type per priority or TM_queue_type_invalid in case it not used. 
                                          The queue_ids for place X is TM_queue_id_p2p_priorityX. 
                                          Multicast types are not valid for these values */
    
    INT32U           down_mc_prio;        /* The priority of the multicast queue. */
    TM_queue_type_e  down_mc_type;        /* The queue type for multicast. Unicast types are not valid for this queue.*/
    INT32U           down_bc_prio;        /* The priority of the broadcast queue. in case of TM_sched_simple_powersave. */
    TM_queue_type_e  down_bc_type;        /* The queue type for broadcast. Unicast types are not valid for this queue */
    TM_queue_type_e  down_powersave_type; /* The queue type for power save queues. only use When TM_sched_simple_powersave model is selected.*/
                                          
} TM_sched_simple_data_s;


typedef struct TM_sched_advance_data_s
{
    INT32U    pon0_num_low_data_queues;  /* The number of low priority queues on PON0/PON_10G. These queues are using group_id 1. */
    INT32U    pon0_num_high_data_queues; /* The number of high priority queues on PON0/PON_10G. These queues are using group_id 2. */
    INT32U    pon1_num_low_data_queues;  /* The number of low priority queues on PON1/PON_1G. These queues are using group_id 3. */
    INT32U    pon1_num_high_data_queues; /* The number of high priority queues on PON1/PON_1G. These queues are using group_id 4. */
} TM_sched_advance_data_s;
typedef union TM_sched_data_u
{
    TM_sched_simple_data_s     simple;    /* This field contains TM data for TM_sched_simple and TM_sched_simple_powersave. */
    TM_sched_advance_data_s    advance;    /* This field contains TM data for TM_sched_advance. */
} TM_sched_data_u;

typedef enum TM_operation_e
{
    TM_OPERATION_TRAFFIC_ENABLE,    /* Enable or disable traffic. */
    TM_OPERATION_COMMIT_SCHED       /* Commit schedule tree changes. */
} TM_operation_e;

typedef enum
{ 
    GW10G_PON_OLT_LLID_ASSIGNMENT_ALL_LLID             = 0,
    GW10G_PON_OLT_LLID_ASSIGNMENT_ODD_LLID             = 1,
    GW10G_PON_OLT_LLID_ASSIGNMENT_EVEN_LLID            = 2
} GW10G_PON_olt_llid_assignment_t;

typedef enum
{ 
    GW10G_PON_OLT_I2C_FAST_SPEED_TIMER = 0x15,
    GW10G_PON_OLT_I2C_STANDARD_SPEED_TIMER = 0x5E,
} GW10G_PON_olt_i2c_speed_mode_t;

typedef struct GW10G_PON_update_olt_parameters_t
{
	PON_rtt_t		min_rtt;
	PON_rtt_t		max_rtt;
    short int		grant_filtering;
	short int		vendor_specific;
	unsigned long	oui;
	PON_OLT_BAUD_RATE baud_rate;
	GW10G_PON_olt_llid_assignment_t llid_assignment;
	short int       sled_strobe;
	short int       sled_mute;
	GW10G_PON_olt_i2c_speed_mode_t i2c_speed;
} GW10G_PON_update_olt_parameters_t;


typedef struct GW10G_PON_olt_initialization_parameters_t
{
	mac_address_t					    olt_mac_address;
	PON_binary_t					    firmware_image;
} GW10G_PON_olt_initialization_parameters_t;

typedef enum GW10G_PON_enable_disable_t
{
  GW10G_PON_DISABLE = 0,
  GW10G_PON_ENABLE
} GW10G_PON_enable_disable_t;

typedef struct GW10G_PON_olt_initialization_params_t
{
	mac_address_t                        olt_mac_address;
	INT16U                               dba_mode;
    INT16U                               memory_size;
    INT16U                               bus_width;
    PON_binary_source_t                  fw_image_source;
    INT8U  *                             fw_image_location;
    INT16U                               fw_image_size;
    GW10G_PON_enable_disable_t                 ram_test;
} GW10G_PON_olt_initialization_params_t;

/* DDR interface type */
typedef enum GW10G_PON_ddr_interface_type_e
{
	GW10G_PON_DDR_TYPE_CPU,
    GW10G_PON_DDR_TYPE_PACKET_BUFFER,
} GW10G_PON_ddr_interface_type_e;

/* DDR bus width */
typedef enum GW10G_PON_ddr_bus_width_e
{
	GW10G_PON_DDR_BUS_WIDTH_32 = 0,
    GW10G_PON_DDR_BUS_WIDTH_64,
	GW10G_PON_DDR_BUS_WIDTH_16
} GW10G_PON_ddr_bus_width_e;

/* DDR size */
typedef enum GW10G_PON_ddr_size_e
{
	GW10G_PON_DDR_SIZE_1024_MBITS = 1024,
	GW10G_PON_DDR_SIZE_2048_MBITS = 2048
} GW10G_PON_ddr_size_e;

/* DDR rate */
typedef enum GW10G_PON_ddr_rate_e
{
	GW10G_PON_DDR_RATE_1066 = 1066, /* default for CPU DDR */
	GW10G_PON_DDR_RATE_1333 = 1333, /* default for PACKET_BUFFER DDR */
	GW10G_PON_DDR_RATE_1600 = 1600
} GW10G_PON_ddr_rate_e;

/* DDR RAM test */
typedef enum GW10G_PON_ddr_ram_test_e
{
	GW10G_PON_DDR_RAM_NO_TEST,
	GW10G_PON_DDR_RAM_CONTROLLER_TEST,
	GW10G_PON_DDR_RAM_SW_TEST
} GW10G_PON_ddr_ram_test_e;

/* DDR time of flight param */
typedef struct GW10G_PON_ddr_lane_tof_param_s
{
    INT32S  tof_ck;
    INT32S  tof_dqs;
} GW10G_PON_ddr_lane_tof_param_s;

typedef union GW10G_PON_ddr_lane_tof_params_u
{
	GW10G_PON_ddr_lane_tof_param_s cpu_ddr_lane_tof_params[PON_CPU_DDR_LANE_TOF_PARAMS_NUMBER];
	GW10G_PON_ddr_lane_tof_param_s packet_buffer_ddr_lane_tof_params[PON_PACKET_BUFFER_DDR_LANE_TOF_PARAMS_NUMBER];
} GW10G_PON_ddr_lane_tof_params_u;

typedef struct GW10G_PON_ddr_parameters_s
{
	GW10G_PON_ddr_size_e				ddr_size;			/* DDR size in Mbits. */
	GW10G_PON_ddr_rate_e				mt;					/* The data rate per pin in MT. */
	GW10G_PON_ddr_bus_width_e			bus_configuration;	/* DDR bus configuration. */
	INT32U						cas_latency;		/* The CAS latency in DDR clock. */
	INT32U						trp_delay;			/* TRP delay in ns. */
	INT32U						trcd_delay;			/* TRCD delay in ns. */
	INT32U						trc_delay;			/* TRC delay in ns. */
	bool						ecc;				/* Support DDR Error Correction Code. */
	GW10G_PON_ddr_lane_tof_params_u	lane_tof_params;	/* Time of flight parameters */
	GW10G_PON_ddr_ram_test_e			ram_test;			/* Choose if to do RAM test. */
} GW10G_PON_ddr_parameters_s;

typedef enum GW10G_PON_ref_clock_e
{
	GW10G_PON_REF_CLOCK_NOT_INITIALIZED = -1,
	GW10G_PON_REF_CLOCK_125_MHZ_SINGLE_ENDED,
	GW10G_PON_REF_CLOCK_161_13_MHZ_DIFFERENTIAL,
	GW10G_PON_REF_CLOCK_125_MHZ_DIFFERENTIAL,
	GW10G_PON_REF_CLOCK_161_13_MHZ_SINGLE_ENDED,
	GW10G_PON_REF_CLOCK_125_MHZ = GW10G_PON_REF_CLOCK_125_MHZ_SINGLE_ENDED,
	GW10G_PON_REF_CLOCK_161_13_MHZ = GW10G_PON_REF_CLOCK_161_13_MHZ_DIFFERENTIAL

} GW10G_PON_ref_clock_e;

typedef enum GW10G_PON_edk_type_e
{
	GW10G_PON_EDK_TYPE_1,
	GW10G_PON_EDK_TYPE_2,
	GW10G_PON_EDK_TYPE_3,
	DEFAULT_USER_EDK,
	GW10G_PON_EDK_LAST = DEFAULT_USER_EDK
} GW10G_PON_edk_type_e;


typedef enum GW10G_PON_optics_module_type_e
{
	AUTO = 0, 					/* Auto mode ?should receive the module type from the device */
	LIGENT_OPTICS_LTX5302,
	LIGENT_OPTICS_LTX5302A,
	LIGENT_OPTICS_LTX5302B,
	FIBERXON_FTM_9712S_SL20G,
	SUPERXON
} GW10G_PON_optics_module_type_e;

/* GW10G_PAS_simple_operation_type_e */
typedef enum GW10G_PAS_simple_operation_type_e
{
	GW10G_PAS_SIMPLE_OPERATION_FIRST = 0,
	GW10G_PAS_SIMPLE_OPERATION_SET_ENABLE_RULE_FOR_DEREG_LLIDS = GW10G_PAS_SIMPLE_OPERATION_FIRST,
	GW10G_PAS_SIMPLE_OPERATION_GET_ENABLE_RULE_FOR_DEREG_LLIDS,
	GW10G_PAS_SIMPLE_OPERATION_SET_CNI_INTERFACE,
	GW10G_PAS_SIMPLE_OPERATION_GET_CNI_INTERFACE,
	GW10G_PAS_SIMPLE_OPERATION_SET_NE_MODE,
	GW10G_PAS_SIMPLE_OPERATION_GET_NE_MODE,
	GW10G_PAS_SIMPLE_OPERATION_GET_1PPS_PON_CLOCK,
	GW10G_PAS_SIMPLE_OPERATION_SET_1PPS_PON_CLOCK,
	GW10G_PAS_SIMPLE_OPERATION_ADD_1PPS_PON_CLOCK,
	GW10G_PAS_SIMPLE_OPERATION_SET_OLT_INTERFACE_MTU,
	GW10G_PAS_SIMPLE_OPERATION_GET_OLT_INTERFACE_MTU,
	GW10G_PAS_SIMPLE_OPERATION_LAST = GW10G_PAS_SIMPLE_OPERATION_GET_OLT_INTERFACE_MTU
} GW10G_PAS_simple_operation_type_e;


/* Device components versions structure */
typedef struct GW10G_PAS_device_versions_t
{
    unsigned short int  device_id;	/* Device ID number                 		*/
	short int  host_major;			/* Device host software major version		*/
	short int  host_minor;			/* Device host software minor version		*/
	short int  host_compilation;	/* Device host software compilation number  */
	short int  host_maintenance;	/* Device host software compilation number  */
	short int  firmware_major;		/* Device firmware major version			*/
	short int  firmware_minor;		/* Device firmware minor version			*/
    short int  build_firmware;      /* Device firmware build version			*/
    short int  maintenance_firmware;/* Device firmware maintenance version  	*/
	unsigned int  hardware_major;		/* Device hardware major version			*/
	unsigned int  hardware_minor;		/* Device hardware minor version			*/
	PON_mac_t  system_mac;			/* Device System port MAC type				*/
	short int  ports_supported;		/* Device Ports (LLIDs)					    */
} GW10G_PAS_device_versions_t;	

/* 
** GPIO definitions
*/
#define GW10G_PON_gpio_lines_t unsigned short int 

#define GW10G_PON_GPIO_MIN_LINE  0
#define GW10G_PON_GPIO_MAX_LINE  67

#define TM_NE_FIRST_PROFILE_INDEX     0
#define TM_NE_LAST_PROFILE_INDEX      -1



#define TM_NE_POLICY_CIR_MAX_VAL 10000000
#define TM_NE_POLICY_CBS_MAX_VAL 16383
#define TM_NE_POLICY_EIR_MAX_VAL 10000000
#define TM_NE_POLICY_EBS_MAX_VAL 16383

#define  TM_TYPE_Q_ID_NUM             4095

#define MAX_QUEUE_INFO_PROFILES_IN_ONE_FRAME    15
#define MAX_POLICY_PROFILES_IN_ONE_FRAME        70
#define MAX_QUEUE_PROFILES_IN_ONE_FRAME			35
#define MAX_QUEUE_CONFIG_IN_ONE_FRAME           25
#define MAX_MC_RULES_IN_ONE_FRAME               10
#define MAX_USER_RULES_IN_ONE_FRAME             5
#define MAX_SERVICE_MAPS_IN_ONE_FRAME           1          

#define TM_MAX_QUEUE_GROUP						255
#define TM_MAX_PRIORITY							7
#define TM_MAX_SCHED_WEIGHT						1023

#define SCHED_SIMPLE_DATA_ARRAY_SIZE            8

#define NE_MAX_SERVICES                         128
#define NE_MAX_SERVICES_PER_MSG                 120
#define SERVICE_MAP_VALUES_ARRAY_SIZE           64
#define NE_SERVICE_MAP_INVALID_MAP_DATA         0xFFFFFFFF
#define NE_MAX_DSCP_ENTRIES						64
#define NE_MAX_DSCP_VALUE						63

#define NE_SERVICE_IVT_NUM                      1023
#define MAX_IVTS_PER_MSG                        60
#define MAX_ADDRESSES_PER_MSG                   25

#define VLAN_PRI_CONV_TABLE_SIZE                64

#define NE_MAX_IVT_ID                           4095
#define NE_MAX_USER_ID                          4095
#define USER_ID_UNUSED                          0xFFFF
#define USER_UNTAG_VLAN                         0xFFFF
#define USER_UNMATCH_VLAN                       0x1FFF
#define TM_NE_MIN_VID                           0
#define TM_NE_MAX_VID                           4095
#define TM_NE_NO_VLAN                           0xFFFF

#define TM_NE_MIN_PRIORITY                      0
#define TM_NE_MAX_PRIORITY                      7

#define TM_NE_MIN_ETHERTYPE                     0x0
#define TM_NE_MAX_ETHERTYPE                     0xFFFF

#define TM_NE_MIN_PORT_EDIT_ID                  0
#define TM_NE_MAX_PORT_EDIT_ID                  63

#define TM_MIN_RATE_LIMIT_ID                    1
#define TM_MAX_RATE_LIMIT_ID                    4096

#define NE_SERVICE_NUM_PER_DIRECTION            64
#define NE_USER_RULE_ID_NUM                     8192
#define NE_ATTRIB_GROUP_NUM_PER_DIR             16
#define NE_SERVICE_MAP_NUM_PER_DIRECTION        16

#define NE_MAX_ADDRESS_TABLE_ENTRIES			32767
#define NE_PROTOCOL_FILTER_ALL_LLIDS			0xFFFF

/*============================= Public Types ============================*/
/* NE ID Ranges */
typedef enum
{
/* NE Service */   
    NE_SERVICE_UPSTREAM_START_ID          = 0x100000,
    NE_SERVICE_UPSTREAM_END_ID            = NE_SERVICE_UPSTREAM_START_ID+NE_SERVICE_NUM_PER_DIRECTION,
    NE_SERVICE_DOWNSTREAM_START_ID        = NE_SERVICE_UPSTREAM_END_ID,
    NE_SERVICE_DOWNSTREAM_END_ID          = NE_SERVICE_DOWNSTREAM_START_ID+NE_SERVICE_NUM_PER_DIRECTION,

/* NE User */
    NE_USER_START_ID                      = 0,
    NE_USER_END_ID                        = NE_USER_START_ID + NE_USER_RULE_ID_NUM,

/* Ne Group */
    NE_ATTRIB_GROUP_START_UPSTREAM_ID     = 0x120000,
    NE_ATTRIB_GROUP_END_UPSTREAM_ID       = NE_ATTRIB_GROUP_START_UPSTREAM_ID+NE_ATTRIB_GROUP_NUM_PER_DIR,
    NE_ATTRIB_GROUP_START_DOWNSTREAM_ID   = NE_ATTRIB_GROUP_END_UPSTREAM_ID,
    NE_ATTRIB_GROUP_END_DOWNSTREAM_ID     = NE_ATTRIB_GROUP_START_DOWNSTREAM_ID+NE_ATTRIB_GROUP_NUM_PER_DIR,    

/* NE SERVICE MAP */
    NE_SERVICE_MAP_UPSTREAM_START_ID      = 0x110000,
    NE_SERVICE_MAP_UPSTREAM_END_ID        = NE_SERVICE_MAP_UPSTREAM_START_ID+NE_SERVICE_MAP_NUM_PER_DIRECTION,
    NE_SERVICE_MAP_DOWNSTREAM_START_ID    = NE_SERVICE_MAP_UPSTREAM_END_ID,
    NE_SERVICE_MAP_DOWNSTREAM_END_ID      = NE_SERVICE_MAP_DOWNSTREAM_START_ID+NE_SERVICE_MAP_NUM_PER_DIRECTION,
    
} NE_TYPE_ID_t;


#define SERVICE_ID_DEFAULT_UPSTREAM     NE_SERVICE_UPSTREAM_START_ID
#define SERVICE_ID_DEFAULT_DOWNSTREAM   NE_SERVICE_DOWNSTREAM_START_ID



/* TM_port_e */
/* ========= */
typedef enum TM_port_e
{
    TM_PORT_PON_10G     = 0x1,              /* 10G PON port */
    TM_PORT_PON_1G      = 0x2,              /* 1G PON port */
    TM_PORT_CNI0        = 0x4,              /* CNI #0  */
    TM_PORT_CNI1        = 0x8,              /* CNI #1  */
    TM_PORT_SNIFF       = 0x10,             /* Sniff port  */
    TM_PORT_CNI_ALL     = 0x10000000,       /* Use for rule that ignore which CNI port is used. This value should use only with function that state that it is valid. */
    TM_PORT_PON_ALL     = 0x20000000,       /* Use for rule that ignore which PON port is used. This value should use only with function that state that it is valid. */
    TM_PORT_ALL         = 0x40000000,       /* Use for rule that applied to all port simultaneously. This value should use only with function that state that it is valid. */
	TM_PORT_AUTO_CNI	= 0x80000000,		/* Use for automatic CNI according to OLT channel */
	TM_PORT_PON_10G_ASYM = TM_PORT_PON_1G	/* Use for 10G asymetric channel */
} TM_port_e;





/* TM_queue_profile_s */
/* ================== */
typedef struct 
{
    INT32U				queue_profile_id;	/* The queue profile ID that receives when queue profile is created. This field should not be change from the default value because this id is used to identify the profile. */
    INT32U				group;				/* This parameter is use for queue allocation. For predefine queue this value is 0 and can't be change. */
    INT32U				queue_info_id;		/* The queue info profile id. */
    INT32U				shape_profile;		/* The policy profile id. 0 indicate that no shaping is used. */
    INT32U				PFC_priority;		/* The priority that use when the TM used with PFC (priority flow control). */
    BOOLEAN				fc_wred_enable;		/* Enable or disable use of flow control or WRED (based on global configuration fc_wred value). */
    TM_port_e			fc_source_ports;	/* The source ports bitmap of the incoming frame that need to receive pause frame when flow control is used. */
    INT32U				sched_prio;			/* The priority that is used for strict priority schedule. Higher priority will always schedule before lower priority and ignore the weight of lower priority. */
    INT32U				sched_weight;		/* The weight that used for WRR scheduling is used. The weight use only between queues with the same priority. */
    TM_queue_type_e		queue_type;			/* The type of the queue. This parameter can't change by the function TM_queue_set. */
} TM_queue_profile_s;





/* TM_policy_s */
/* =========== */
typedef struct TM_policy_s
{
    INT32U    policy_id;    /* The policy id that receive when queue is create. */
    INT32U    CIR;			/* The CIR (Committed Information Rate) in kbps for green frames. */
    INT32U    CBS;			/* The CBS (Committed Burst Size) in kilobytes for maximum green burst. */
    INT32U    EIR;			/* The EIR (Excess Information Rate) in kbps for yellow frames. */
    INT32U    EBS;			/* The EBS (Excess Burst Size) in kilobytes for yellow frames. */
} TM_policy_s;

typedef enum TM_config_e
{
    TM_CONFIG_MTU 					= 0,    /* This field set the maximum frame size. (The size is including the CRC.) The valid values are 1522 or 2000. */
    TM_CONFIG_FC_WRED 				= 1,    /* This field set if TM support WRED or flow control. This value must be set before traffic start using TM_operation. */
    TM_CONFIG_RL_CPU_TYPE 			= 2,    /* Set the type of rate limiting that used by the CPU. */
    TM_CONFIG_RL_TYPE_DOWN 			= 3,    /* Set type of algorithm for rate limiter. */
    TM_CONFIG_SHAPE_TYPE_DOWN 		= 5,    /* Set type of algorithm for rate limiter. */
	TM_CONFIG_SCHED_PREFER_DOWN 	= 6,    /* Choose method of scheduling base on priority before color or vice versa. */
    TM_CONFIG_RL_TYPE_UP 			= 7,    /* Set type of algorithm for rate limiter. */
    TM_CONFIG_SHAPE_TYPE_UP 		= 9,    /* Set type of algorithm for rate limiter. */
	TM_CONFIG_SCHED_PREFER_UP 		= 10,   /* Choose method of scheduling base on priority before color or vice versa. */
    TM_CONFIG_LAST_PARAMETER
} TM_config_e;

typedef enum TM_queue_info_type_e
{
    TM_QUEUE_INFO_TYPE_EXTMEM,                     /* The profile is used for external memory. Up to 31 profiles available. */
    TM_QUEUE_INFO_TYPE_INTMEM_DOWNSTREAM,          /* The profile is used for internal memory in the downstream direction. The profiles number is 7 for internal memory configuration or 6 for external memory configuration. */
    TM_QUEUE_INFO_TYPE_INTMEM_UPSTREAM,            /* The profile is used for internal memory in the downstream direction. The profiles number is 7 for internal memory configuration or 6 for external memory configuration. */
} TM_queue_info_type_e;


typedef struct TM_fc_data_s
{
    INT32U    min;    /* Minimum threshold in percent. */
    INT32U    max;    /* Maximum threshold in percent. */
} TM_fc_data_s;


typedef enum TM_WRED_mode_e
{
    TM_WRED_MODE_COLOR,       /* The WRED is work in color based mode. Only the first 3 drop_data are used. drop_data[TM_thresh_green] used for green, drop_data[TM_thresh_yellow] for yellow and drop_data[TM_thresh_red] for red. */
    TM_WRED_MODE_PRIORITY     /* The WRED is use priority based mode. drop_data[TM_thresh_prio_6_7] is used for priority 6,7. drop_data[TM_thresh_prio_4_5] is used for priority 4,5. drop_data[TM_thresh_prio_2_3] is used for priority 2,3. drop_data[TM_thresh_prio_0_1] is used for priority 0,1. */
} TM_WRED_mode_e;


typedef struct TM_tresh_tuple_s
{
    INT32U    min;          /* Minimum threshold in percent. */
    INT32U    max;          /* Maximum threshold in percent. */
    INT32U    drop_prob;    /* Drop probability in percent. */
} TM_tresh_tuple_s;


typedef struct TM_wred_data_s
{
    TM_tresh_tuple_s    thresh_data[4];  /* Minimum threshold and maximum threshold and drop probability for WRED configuration. When using color mode only the 3 threshold are used. */
    INT32U              exponent;        /* The exponent time factor. */
    TM_WRED_mode_e      mode;            /* The mode that use for WRED calculation priority base or color based. */
} TM_wred_data_s;


typedef union TM_fc_wred_params_u
{
    TM_fc_data_s      fc;      /* Flow control configuration data. */
    TM_wred_data_s    wred;    /* WRED configuration data. */
} TM_fc_wred_params_u;


typedef enum TM_config_fc_wred_e
{
    TM_CONFIG_FC_WRED_MODE_FC,      /* The TM is using flow control instead of WRED */
    TM_CONFIG_FC_WRED_MODE_WRED     /* The TM is using WRED instead of flow control. */
} TM_config_fc_wred_e;


typedef struct TM_fc_wred_data_s
{
    BOOLEAN              enable;        /* Enable or disable using congestion. */
    TM_config_fc_wred_e  fc_wred_mode;  /* The configuration value type in the data. */
    TM_fc_wred_params_u  data;          /* The configuration data for flow control or WRED. */
   
} TM_fc_wred_data_s;


typedef struct TM_queue_info_s
{
    INT32U               queue_info_id; /* The queue profile ID that receives when queue profile is created. */
    TM_queue_info_type_e profile_type;  /* The type of profile for external memory or for internal memory. */
    INT32U               guaranteed;    /* Guaranteed memory size that is available to the queue in KB. Value is round to nearest hardware possible size. (The granularity is 4KB for 256MB DRAM, 8KB for 512MB DRAM and 16KB for 1GB or 1KB for internal memory). */
    INT32U               maximum;       /* Maximum value for the queue size in KB. Value is round to nearest hardware possible size. (The granularity is 4KB for 256MB DRAM, 8KB for 512MB DRAM and 16KB for 1GB or 1KB for internal memory). */
    TM_fc_wred_data_s    congestion;    /* The flow control or WRED configuration. */
} TM_queue_info_s;


typedef struct TM_queue_config_s 
{
    INT32U              queue_id;           /* The queue id. */
    INT32U	            queue_profile_id;   /* The queue profile id. */
    TM_queue_profile_s  config;             /* The queue profile. */
    BOOLEAN             enable;             /* Queue is enabled or disabled. */
    
} TM_queue_config_s;


typedef enum TM_config_rl_cpu_e
{
    TM_CONFIG_CPU_RL_FRAME, /* The CPU rate limiting is done by frame count. */
    TM_CONFIG_CPU_RL_KB	    /* The CPU rate limiting is done by kilobits. */
} TM_config_rl_cpu_e;


typedef enum TM_config_rl_type_e
{
    TM_CONFIG_POLICY_RFC2697,	/* Rate limiting is compute according to RFC2697. EIR value is ignored in this mode. */
    TM_CONFIG_POLICY_RFC2698,	/* Rate limiting is compute according to RFC2698.  EIR is used as PIR of the RFC2698. */
    TM_CONFIG_POLICY_MEF_CF0,	/* Rate limiting is compute according to MEF 10.2 with CF=0. */
    TM_CONFIG_POLICY_MEF_CF1	/* Rate limiting is compute according to MEF 10.2 with CF=1. */
} TM_config_rl_type_e;


typedef enum TM_config_sched_prefer_e
{
    TM_CONFIG_SCHED_PREFER_COLOR,       /* Prefer color on priority in the scheduler. */
    TM_CONFIG_SCHED_PREFER_PRIORITY     /* Prefer priority on color in the scheduler. */
} TM_config_sched_prefer_e;




/* NE_addr_ivt_type_e */
typedef enum NE_addr_ivt_type_e
{
    NE_ADDR_IVT_NONE,             /* Don抰 use bind. */
    NE_ADDR_IVT_BIND_IPV4_SIP,    /* Bind IPv4 source address. Up to 2047 addresses can be support. */
    NE_ADDR_IVT_BIND_SA_DA        /* Bind SA to DA. Up to 128 addresses are supported. */
} NE_addr_ivt_type_e;



/* NE_VLAN_pri_mode_e */
typedef enum NE_VLAN_pri_mode_e
{
    NE_VLAN_PRI_MODE_COS = 0,          /* Update the primary VLAN priority using primary VLAN COS. Only the first 8 values are used. */
    NE_VLAN_PRI_MODE_IP4 = 1,          /* Update the primary VLAN priority using IPv4 DHCP values. If the frame is not IPv4 the VLAN priority is set to default. */
    NE_VLAN_PRI_MODE_IP6 = 2,          /* Update the primary VLAN priority using IPv6 DHCP values. If the frame is not IPv6 the VLAN priority is set to default. */
    NE_VLAN_PRI_MODE_IPV4_IPV6 = 3     /* Update the primary VLAN priority using IPv4 DHCP or IPv6 DSCP values. If the frame is not IP the VLAN priority is set to default. */
} NE_VLAN_pri_mode_e;


/* NE_VLAN_op_e */
typedef enum NE_VLAN_op_e
{
    NE_VLAN_OP_NOP = 0, /* No VLAN editing will used. */
    NE_VLAN_OP_ADD,     /* VLAN will be added. */
    NE_VLAN_OP_EDIT,    /* VLAN will be update. */
    NE_VLAN_OP_DEL      /* VLAN will be deleted. */
} NE_VLAN_op_e;

/* NE_VLAN_field_e */
typedef enum NE_VLAN_field_e
{
    NE_VLAN_FIELD_NONE,            /* No field is used. */
    NE_VLAN_FIELD_TYPE,            /* VLAN Ethernet type field is used. */
    NE_VLAN_FIELD_VID,            /* The VLAN id field is used. */
    NE_VLAN_FIELD_TYPE_VID,        /* The VLAN id and the VLAN Ethernet type fields are used. */
    NE_VLAN_FIELD_PRI,             /* The VLAN priority field is used. */
    NE_VLAN_FIELD_TYPE_PRI,        /* The VLAN priority and the VLAN Ethernet type fields are used. */
    NE_VLAN_FIELD_VID_PRI,         /* The VLAN type and VLAN priority fields are used. */
    NE_VLAN_FIELD_TYPE_VID_PRI     /* The VLAN type, VLAN priority and VLAN Ethernet type fields are used. */
} NE_VLAN_field_e;

/* NE_vlan_fields_s */
typedef struct NE_vlan_fields_s
{
    NE_VLAN_field_e    fields;    /* The field that will update. The value of NE_VLAN_field_none is meaning that the tag not exists. */
    NE_VLAN_type_e     vtype;     /* The VLAN ether type. */
    INT16U             vid;       /* The VLAN id that used with add operation and edit operation. */
    INT16U             priority;  /* The defaults modify priority. */
} NE_vlan_fields_s;

/* NE_edit_vlan_s */
typedef struct NE_edit_vlan_s
{
    NE_VLAN_op_e        pr_op;          /* VLAN editing operation on primary VLAN. */
    NE_vlan_fields_s    pr_vlan;        /* VLAN editing rule for primary VLAN. */
    NE_VLAN_op_e        sc_op;          /* VLAN editing operation on secondary VLAN. */
    NE_vlan_fields_s    sc_vlan;        /* VLAN editing rule for secondary VLAN. */
    INT32U              port_edit_id;   /* Rule id for editing per port. */
    BOOLEAN             vlan_swap;      /* Swap VLAN after editing. It can be used if more then 2 VLAN exits. */
} NE_edit_vlan_s;


/* NE_edit_type_e */
typedef enum NE_edit_type_e
{
    NE_EDIT_TYPE_NONE = 0,  /* The rule will not do any editing. */
    NE_EDIT_TYPE_VLAN      /* The rule will update VLAN. */
  /*NE_EDIT_TYPE_DSA*/        /* The rule will update DSA tag. */
} NE_edit_type_e;

/* NE_edit_rule_u */
typedef struct NE_edit_rule_u
{
    NE_edit_vlan_s      vlan;   /* VLAN editing rule */
    /*TBD NE_dsa        DSA;*/  /* DSA tag. */
} NE_edit_rule_u;

/* NE_edit_op_s */
typedef struct NE_edit_op_s
{
    NE_edit_type_e    rule_type;    /* The type of the edit operation. */
    NE_edit_rule_u    rule;         /* The data of the edit operation. */
} NE_edit_op_s;

/* NE_field_value_u */
typedef union NE_field_value_u
{
    INT32U    pr_vlan;    /* The frame has primary VLAN. */
    INT32U    sc_vlan;    /* The frame has secondary VLAN. */
    INT32U    ipv4;    /* The frame is IPv4. */
    INT32U    ipv6;    /* The frame is IPv6. */
    INT32U    tcp;    /* The frame is UDP. */
    INT32U    udp;    /* The frame is TCP. */
    INT32U    igmp;    /* The frame is IGMP. */
    INT32U    icmpv6;    /* The frame is ICMPv6. */
    INT32U    direction;    /* 0 for downstream direction */
    INT32U    ip_mc;    /* The frame is IP multicast. */
    INT32U    mc;    /* The frame is multicast */
    INT32U    bc;    /* The frame is broadcast */
    INT32U    cfi;    /* The value of CFI bit in primary VLAN. */
    INT8U     sa[6];    /* MAC source address */
    INT8U     da[6];    /* MAC destination address */
    INT32U    ether_type;    /* Ethernet type */
    INT32U    pr_vid;    /* Primary VLAN ID */
    INT32U    pr_cos;    /* Primary VLAN priority COS */
    INT32U    sc_vid;    /* Secondary VLAN ID */
    INT32U    sc_cos;    /* Secondary VLAN priority COS */
    INT32U    ipv4_sip;    /* IPv4 source address */
    INT32U    ipv4_dip;    /* IPv4 destination address */
    INT32U    ipv4_tos;    /* IPv4 type of service */
    INT32U    ipv4_dscp;    /* IPv4 differential service */
    INT32U    ipv4_protocol;    /* IPv4 protocol */
    INT8U     ipv6_sip[16];    /* IPv6 source address */
    INT8U     ipv6_dip[16];    /* IPv6 destination address */
    INT32U    ipv6_tos;    /* IPv6 type of service */
    INT32U    ipv6_dscp;    /* IPv6 differential service */
    INT32U    ipv6_protocol;    /* IPv6 protocol */
    INT32U    tcp_src;    /* TCP source port */
    INT32U    tcp_dst;    /* TCP destination address */
    INT32U    udp_src;    /* UDP source port */
    INT32U    udp_dst;    /* UDP destination address */
	INT32U	  flow_label;	/* IPv6 flow label */
	INT32U	  llid;			/* The llid of the frame */
} NE_field_value_u;


/* NE_addr_ivt_config_s */
typedef struct NE_addr_ivt_config_s
{
    INT32U                service_id;  /* The service_id of the ivt. */
    NE_addr_ivt_type_e    ivt_type;    /* The type of ivt key of the rule. */
    NE_field_value_u      value;       /* The ivt value. (Like MAC address or IPv4 SIP.) */
    INT32U                ivt;         /* The id of the key. */
} NE_addr_ivt_config_s;
/* NE_protocol_da_ether_data_s */
typedef struct NE_protocol_da_ether_data_s
{
    INT16U    ethertype;     /* Ethernet type. */
    INT16U    ethertype_mask;/* Ethernet type. */
    INT8U     da[6];         /* MAC DA. */
    INT8U     da_mask[6];    /* MAC DA. */
} NE_protocol_da_ether_data_s;

typedef enum NE_ieee1588_msg_type_e
{
	NE_IEEE1588_MSG_TYPE_SYNC              = 0,
	NE_IEEE1588_MSG_TYPE_DELAY_REQ            ,
	NE_IEEE1588_MSG_TYPE_PDELAY_REQ           ,
	NE_IEEE1588_MSG_TYPE_PDELAY_RESP          ,
	NE_IEEE1588_MSG_TYPE_RESERVED_4           ,
	NE_IEEE1588_MSG_TYPE_RESERVED_5           ,
	NE_IEEE1588_MSG_TYPE_RESERVED_6           ,
	NE_IEEE1588_MSG_TYPE_RESERVED_7           ,
	NE_IEEE1588_MSG_TYPE_FOLLOW_UP            ,
	NE_IEEE1588_MSG_TYPE_DELAY_RESP           ,
	NE_IEEE1588_MSG_TYPE_PDELAY_RESP_FOLLOW_UP,
	NE_IEEE1588_MSG_TYPE_ANNOUNCE             ,
	NE_IEEE1588_MSG_TYPE_SIGNALING            ,
	NE_IEEE1588_MSG_TYPE_MANAGEMENT           ,
	NE_IEEE1588_MSG_TYPE_RESERVED_14          ,
	NE_IEEE1588_MSG_TYPE_RESERVED_15
} NE_ieee1588_msg_type_e;


typedef struct NE_protocol_ieee1588_data_s
{
	NE_ieee1588_msg_type_e  message_type;
	INT8U                   ptp_version	;
	BOOLEAN                 enable		;
	BOOLEAN                 time_stamp	;
	BOOLEAN                 two_step_flag;
} NE_protocol_ieee1588_data_s;

typedef struct NE_rate_burst_data_s
{
	INT8U                   rate;
	INT8U                   burst;
} NE_rate_burst_data_s;


/*	NE_protocol_op_value_u */
typedef union NE_protocol_op_value_u
{
    BOOLEAN   dp_discard;                         /* Discard the frame or not. */
    INT32U    cpu_queue_id;                       /* The id of the CPU queue that receive the frame. */
    INT32U    sniff_queue_id;                     /* The sniff port queue id. */
    INT32U    cpu_sniff_id;                       /* User defined value that enable to user to know the rule that used for receiving the frame. The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
    INT32U    attribute;
	NE_protocol_da_ether_data_s *da_ether_ptr;    /* Pointer to a structure that define the classified MAC DA and Ethernet type. */
    INT32U    opcode;                             /* Opcode for */
    INT16U    opcode_range[2];                    /* Opcode for */
	NE_protocol_ieee1588_data_s *ieee1588_ptr;
	NE_rate_burst_data_s rate_limiter;
} NE_protocol_op_value_u;

/* NE_action_type_e */
typedef enum NE_action_type_e
{
    NE_ACTION_TYPE_DP_QUEUE_ID		= 0,    /* The id of the queue in data path that receive the frame. When the frame should discard in data path the user need to set this value to TM_queue_id_invalid. */
    NE_ACTION_TYPE_SNIFF_QUEUE_ID	= 2,    /* The sniff port queue id. */
    NE_ACTION_TYPE_EDIT				= 3,    /* The editing on the result. */
    NE_ACTION_TYPE_DP_RATE_LIMIT0	= 4,    /* The rate limiter id of first data path rate limiter. */
    NE_ACTION_TYPE_DP_RATE_LIMIT1	= 5,    /* The rate limiter id of second data path rate limiter. */
    NE_ACTION_TYPE_DP_RATE_LIMIT2	= 6,    /* The rate limiter id of third data path rate limiter. */
    NE_ACTION_TYPE_CPU_FLOW_ID		= 7,	/* User defined value that enable to user to know the rule that used for receiving the frame. */
    NE_ACTION_TYPE_ELLID			= 8     /* The egress LLID. (This field only used in downstream direction) */
} NE_action_type_e;

/* NE_attribute_e */
typedef enum NE_attribute_e
{
    NE_ATTRIBUTE_PR_VLAN = 3,    /* The frame has primary VLAN. */
    NE_ATTRIBUTE_SC_VLAN = 4,    /* The frame has secondary VLAN. */
    NE_ATTRIBUTE_IPV4 = 5,       /* The frame is IPv4. */
    NE_ATTRIBUTE_IPV6 = 6,       /* The frame is IPv6. */
    NE_ATTRIBUTE_TCP = 7,        /* The frame is UDP. */
    NE_ATTRIBUTE_UDP = 8,        /* The frame is TCP. */
    NE_ATTRIBUTE_IGMP = 9,       /* The frame is IGMP. */
    NE_ATTRIBUTE_ICMPV6 = 10,    /* The frame is ICMPv6. */
	NE_ATTRIBUTE_CHANNEL = 26,   /* On upstream 0 mean PON0 and 1 mean PON1. On downstream 0 mean CNI0 and 1 mean CNI1. */	
    NE_ATTRIBUTE_IP_MC = 27,     /* The frame is IP multicast. */
    NE_ATTRIBUTE_MC = 28,        /* The frame is multicast */
    NE_ATTRIBUTE_BC = 29,        /* The frame is broadcast */
    NE_ATTRIBUTE_CFI = 30        /* The value of CFI bit in primary VLAN. */
} NE_attribute_e;



/* NE_attributes_s */
typedef struct NE_attributes_s
{
    INT32U          num;              /* Number of valid attributes. */
    NE_attribute_e  attributes[5];    /* The attribute to test. */
    INT32U          values[5];        /* The attribute exists or not */
} NE_attributes_s;


/* NE_action_value_u */
typedef union NE_action_value_u
{
    INT32U    dp_queue_id;       /* The id of the queue in data path that receive the frame. */
    INT32U    cpu_queue_id;      /* The id of the CPU queue that receive the frame. */
    INT32U    sniff_queue_id;    /* The sniff port queue id. */
    NE_edit_op_s *edit;          /* Pointer to editing rule on the result. */
    INT32U    dp_rate_limit0;    /* The rate limiter id of first data path rate limiter. */
    INT32U    dp_rate_limit1;    /* The rate limiter id of second data path rate limiter. */
    INT32U    dp_rate_limit2;    /* The rate limiter id of third data path rate limiter. */
/*  INT32U    cpu_rate_limit - yellow not supported*/    /* The rate limiter id of CPU rate limiter. */
    INT32U    cpu_flow_id;       /* User defined value that enable to user to know the rule that used for receiving the frame. */
    INT32U    ellid;             /* The egress LLID. (This field only used in downstream direction) */
} NE_action_value_u;


/* NE_addr_rule_type_e */
typedef enum NE_addr_rule_type_e
{
    NE_ADDR_RULE_TYPE_MAC,					/* Rule that use MAC address. */
	NE_ADDR_RULE_TYPE_VID,					/* Rule that use VID. */
    NE_ADDR_RULE_TYPE_MAC_VID,				/* Rule that use mac address and vid. */
    NE_ADDR_RULE_TYPE_IPV4_DIP,				/* Rule that use DIP IP address. */
	NE_ADDR_RULE_TYPE_IPV4_DIP_VID,			/* Rule that use DIP IP address & VID. */
    NE_ADDR_RULE_TYPE_IPV6_DIP,				/* Rule that use DIP */
    NE_ADDR_RULE_TYPE_IPV6_DIP_LSB_VID,		/* Rule that use 64bit LSB of IP DIP & VID */
	/*below enum's are only for del and get and not for add*/
    NE_ADDR_RULE_SEARCH_LLID = 13,				/* Rule that use only for searching or deleting address base on llid. */
    NE_ADDR_RULE_SEARCH_LLID_STATIC,		/* Rule that use only for searching or deleting address base on llid or if the address is static or not. */
    NE_ADDR_RULE_SEARCH_ALL,				/* Rule that use only for searching or deleting all address. */
    NE_ADDR_RULE_SEARCH_STATIC,				/* Rule that use only for searching or deleting all the type of address if it static or dynamic. */
	NE_ADDR_RULE_SEARCH_IVT,				/* Rule that use only for searching or deleting address base on ivt_id */
	NE_ADDR_RULE_SEARCH_IVT_STATIC			/* Rule that use only for searching or deleting address base on ivt_id or if the address is static or not */
} NE_addr_rule_type_e;


/* NE_addr_vid_mac_s */
typedef struct 
{
    INT16U   vid;       /* VLAN id.    */
    INT8U    mac[6];    /* MAC address */
    
} NE_addr_vid_mac_s;

typedef struct 
{
	INT16U	vid;	/* VLAN id */
	INT32U	ipv4;	/* IPv4 address */
} NE_addr_vid_ipv4_s;

typedef struct 
{
	INT16U	vid;			/* VLAN id */
	INT8U	ipv6_lsb[8];	/* The lower 64 bit of the IPV6 address */
} NE_addr_vid_ipv6_lsb_s;

/* NE_addr_value_u */
typedef union 
{
    INT8U					mac[6];				/* MAC address */
    NE_addr_vid_mac_s		vlan_mac;			/* VID and MAC address */
    INT32U					ipv4_dip;			/* IPv4 dip address */
    INT8U					ipv6_dip[10];		/* The last 80 bit of IPv6 address. */
	NE_addr_vid_ipv4_s		vlan_ipv4_dip;		/* VID and IPv4 address */
	NE_addr_vid_ipv6_lsb_s	vlan_ipv6_dip_lsb;	/* VID and lower 64 bits of IPv6 address */
	INT16U                  prvid;				/* Primary Vlan address */
} NE_addr_value_u;

/* NE_addr_entry_s */
typedef struct 
{
    NE_addr_rule_type_e    addr_type;   /* The address type */
    NE_addr_value_u        addr_value;  /* The address value. */
    TM_port_e              port;        /* Logical source port. */
    INT32U                 llid;        /* The LLID of that was learned. */
    INT32U                 age;         /* The age of the address in ms.  This value is round to the nearest period that set with NE_addr_config_age_timer. */
    INT32U                 ivt_id;      /* This is internal id that used when special service like binding is used with this service. The id is generating using NE_service_ivt_add. */
    INT8U                  addr_group;  /* Reserved */
    BOOLEAN                da_discard;  /* Discard frame with this destination address. */
    BOOLEAN                da_to_sniff; /* Send frame with this destination address to sniff port. */
    BOOLEAN                da_to_cpu;   /* Send frame with this destination address to the CPU. */
    BOOLEAN                sa_discard;  /* Discard frame with this destination address. */
    BOOLEAN                sa_to_sniff; /* Send frame with this source address to sniff port. */
    BOOLEAN                sa_to_cpu;   /* Send frame with this source address to the CPU. */
    BOOLEAN                is_static;   /* Address is static or not. */
} NE_addr_entry_s;


/* NE_port_config_e */
typedef enum NE_port_config_e
{
    NE_PORT_CONFIG_DSCP_UPDATE,        /* Update DSCP in frame of IPv4 or IPv6 according to NE_dscp_conv_set. */
    NE_PORT_CONFIG_CFI_UPDATE,         /* Update CFI bit. */
    NE_PORT_CONFIG_VLAN_PRI_UPDATE,    /* Update the VLAN priority according to NE_VLAN_pri_conv_set. */
    NE_PORT_CONFIG_DISCARD_RED,        /* Discard RED frame in rate limiter. */
    NE_PORT_CONFIG_COLOR_AWARE         /* Use color aware mode based on CFI value or not. */
} NE_port_config_e;

/* NE_protocol_type_e */
typedef enum NE_protocol_type_e
{
    NE_PROTOCOL_TYPE_PPPOE_DISCOVERY      = 0,  /* PPPoE discovery frames */
    NE_PROTOCOL_TYPE_PPPOE_SESSION        = 1,  /* PPPoE sessions frames */
	NE_PROTOCOL_TYPE_OAM				  = 2,  /* OAM frames */
	NE_PROTOCOL_TYPE_MPCP				  = 3,  /* MPCP frames with configurable opcode */
    NE_PROTOCOL_TYPE_LACP                 = 7,  /* LACP frames */
    NE_PROTOCOL_TYPE_LLTD                 = 8,  /* LLTD frames */
    NE_PROTOCOL_TYPE_STP                  = 9,  /* STP/RSTP/MSTP frames */
    NE_PROTOCOL_TYPE_802_1X               = 10, /* 802.1X frames */
    NE_PROTOCOL_TYPE_CONFIG_DA_ETHER0     = 11, /* Configurable DA MAC and Ether type and search mask */
    NE_PROTOCOL_TYPE_CONFIG_DA_ETHER1     = 12, /* Configurable DA MAC and Ether type and search mask */
    NE_PROTOCOL_TYPE_DA_EQUAL_SA          = 13, /* DA=SA */
    NE_PROTOCOL_TYPE_LINK_CONSTRAINTS_0X  = 14, /* DA=01-80-C2-00-00-0X and not MPCP, LACP, OAM or 802.1x */
    NE_PROTOCOL_TYPE_LINK_CONSTRAINTS_2X  = 15, /* DA=01-80-C2-00-00-2X and not MPCP, LACP, OAM or 802.1x */
    NE_PROTOCOL_TYPE_LINK_CONSTRAINTS_XX  = 16, /* DA=01-80-C2-00-00-XX and not MPCP, LACP, OAM or 802.1x */
    NE_PROTOCOL_TYPE_IPV4_LOCAL_MULTICAST = 17, /* DA = 01-00-5E-00-00-XX */
    NE_PROTOCOL_TYPE_IPV4_MANAGEMENT      = 18, /* DA = 01-00-5E-00-01-XX */
    NE_PROTOCOL_TYPE_IPV4_MULTICAST       = 19, /* DA = 01-00-5E-XX-XX-XX */
    NE_PROTOCOL_TYPE_IPV6_LOCAL_MULTICAST = 20, /* DA = 33-33-00-00-00-XX */
    NE_PROTOCOL_TYPE_IPV6_MANAGEMENT      = 21, /* DA = 33-33-00-00-XX-XX */
    NE_PROTOCOL_TYPE_IPV6_MULTICAST       = 22, /* DA = 33-33-XX-XX-XX-XX */
    NE_PROTOCOL_TYPE_ARP                  = 23, /* ARP frames */
    NE_PROTOCOL_TYPE_IGMP                 = 24, /* IGMP frames */
    NE_PROTOCOL_TYPE_ICMPV4               = 25, /* ICMPv4 frames */
    NE_PROTOCOL_TYPE_ICMPV6               = 26, /* ICMPv6 frames */
    NE_PROTOCOL_TYPE_PIM                  = 27, /* PIM frames */
    NE_PROTOCOL_TYPE_DHCPV4               = 28, /* DHCPv4 frames */
    NE_PROTOCOL_TYPE_DHCPV6               = 29  /* DHCPv6 frames */
} NE_protocol_type_e;



/* NE_protocol_op_type_e */
typedef enum NE_protocol_op_type_e
{
    NE_PROTOCOL_OP_TYPE_DP_DISCARD,        /* Discard the frame on data path. */
    NE_PROTOCOL_OP_TYPE_CPU_QUEUE_ID,      /* The id of the CPU queue that receive the frame. */
    NE_PROTOCOL_OP_TYPE_SNIFF_QUEUE_ID,    /* The sniff port queue id. */
    NE_PROTOCOL_OP_TYPE_CPU_SNIFF_ID,      /* User defined value that enable to user to know the rule that used for receiving the frame. The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
    NE_PROTOCOL_OP_TYPE_RULE_UPDATE ,      /* Update the rule search values can be use only with NE_protocol_type_config_DA_ether0 or NE_protocol_type_config_DA_ether1. */
	NE_PROTOCOL_OP_TYPE_RULE_SET_ATTRIBUTE, /* Update one of the attribute bits. */
	NE_PROTOCOL_OP_RATE_LIMITER,/*update the rate limiter of a protocol*/
} NE_protocol_op_type_e;


/* NE_dscp_mode_e */
typedef enum NE_dscp_mode_e
{
    NE_DSCP_MODE_VLAN_PRIO,     /* DSCP update is based on VLAN priority. */
    NE_DSCP_MODE_DSCP_VALUE     /* DSCP update is based on DSCP value. */
} NE_dscp_mode_e;


/* NE_service_search_type_e */
typedef enum NE_service_search_type_e
{
	NE_SERVICE_SEARCH_TYPE_UP_LLID							= 0,	/* Upstream rules are base on ingress LLID. */
	NE_SERVICE_SEARCH_TYPE_UP_LLID_PRVLAN					= 1,	/* Upstream rules are base on ingress LLID and primary VLAN. */
	NE_SERVICE_SEARCH_TYPE_UP_LLID_PRVLAN_PRI				= 2,	/* Upstream rules are base on ingress LLID, primary VLAN and primary VLAN priority. */
	NE_SERVICE_SEARCH_TYPE_UP_LLID_SCVLAN					= 3,	/* Upstream rules are base on ingress LLID and secondary VLAN. */
	NE_SERVICE_SEARCH_TYPE_UP_LLID_PRVLAN_SCVLAN			= 4,	/* Upstream rules are base on ingress LLID, primary VLAN and secondary VLAN. */
	NE_SERVICE_SEARCH_TYPE_UP_LLID_PRVLAN_ETHER				= 5,	/* Upstream rules are base on ingress LLID, primary VLAN and Ethernet type. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN							= 6,	/* Rules are base on primary VLAN. This search doesn't support TM_NE_NO_VLAN setting. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN_LLID						= 7,	/* Rules are base on primary VLAN and user is search by LLID. This search doesn't support TM_NE_NO_VLAN setting. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN_PRI						= 8,	/* Rules are base on primary VLAN and primary VLAN priority. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN_SCVLAN					= 9,	/* Rules are base on primary VLAN and secondary VLAN. This search doesn't support TM_NE_NO_VLAN setting. */
	NE_SERVICE_SEARCH_TYPE_ETHERTYPE						= 10,	/* Rules are base on Ethernet type. */
	NE_SERVICE_SEARCH_TYPE_SVC_ETHERTYPE_USR_LLID			= 11,	/* Rules are base on Ethernet type in service and llid as user. For example PPPoE classification. The values for serice are set using NE_service_op_set for ether type and the llid for user using NE_user_rule_add. */
	NE_SERVICE_SEARCH_TYPE_N1_PRVLAN						= 12,	/* Define rules for N:1 VLAN translation. The rule use primary VLAN. This search doesn't support TM_NE_NO_VLAN setting. */
	NE_SERVICE_SEARCH_TYPE_N1_LLID_PRVLAN					= 13,	/* Define rules for N:1 VLAN translation in the upstream. The rule use ingress LLID and primary VLAN. */
	NE_SERVICE_SEARCH_TYPE_N1_PRVLAN_SCVLAN					= 14,	/* Define rules for N:1 VLAN translation. The rule use ingress LLID, primary VLAN and secondary VLAN. This search doesn't support TM_NE_NO_VLAN setting. */
	NE_SERVICE_SEARCH_TYPE_N1_LLID_PRVLAN_SCVLAN            = 15,	/* Define rules for N:1 VLAN translation. The upstream part of the rule use ingress LLID, primary VLAN and secondary VLAN. This search support TM_NE_NO_VLAN setting but it must be set on primary VLAN and secondary VLAN. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN_IPV4_SIP					= 16,	/* Rules are for service only and set using NE_service_op_set base on primary VLAN and Source IPv4 address. */
	NE_SERVICE_SEARCH_TYPE_PRVLAN_IPV4_DIP					= 17,	/* Rules are for service only and set using NE_service_op_set base on primary VLAN and destination IPv4 address. */
	NE_SERVICE_SEARCH_TYPE_MODE1_DOWN_N1_PRVLAN				= 22,	/* N:1 rule that base on MAC and primary VLAN. The primary VLAN in upstream is learning after modification */
	NE_SERVICE_SEARCH_TYPE_MODE1_DOWN_N1_PRVLAN_DUP			= 23,	/* N:1 rule that base on MAC and primary VLAN. The primary VLAN in upstream is learning after modification. Unknown frame are duplicate per LLID by the CPU in the downstream */
	NE_SERVICE_SEARCH_TYPE_MODE1_PRVLAN						= 24,	/* Mode1 For 1:1 single tagged	 */
	NE_SERVICE_SEARCH_TYPE_MODE1_PRVLAN_SCVLAN              = 25,	/* For 1:1 double tagged		 */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_TAGGED                  = 26,	/* Upstream rules are based on frames that are tagged and are non IPv6 */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_TAGGED_IPV6_LLID        = 27,	/* Upstream rules are based on frames that are tagged and are IPv6.It search LLID for map priority per LLID */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_UNTAGGED_ANTISPOOF_IPV4 = 28,	/* Upstream rules are based on frames that are tagged and are non IPv6. It also implement anti spoofing rule for IPv4 */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_UNTAGGED_ANTISPOOF_IPV6 = 29,	/* Upstream rules are based on frames that are tagged and are IPv6. It also implement anti spoofing rule for IPv6 */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_TAGGED_LEARN			= 33,	/* Upstream rules are based on frame that are tagged and are non IPv6 and have LLID with default of un match to learn. (It not necessarily the same service in each stage.). Only one service with this type is create automatically this service can't be deleted. */
	NE_SERVICE_SEARCH_TYPE_MODE1_UP_TAGGED_IPV6_LLID_LEARN	= 34,	/* Upstream rules are based on frame that are tagged and are IPv6 and have LLID with default of un match to learn. The service search port and LLID for set default VLAN. (It not necessarily the same service in each stage.). Only one service with this type is create automatically this service can't be deleted. */
	NE_SERVICE_SEARCH_TYPE_DEFAULT							= 36					 
} NE_service_search_type_e;


typedef struct NE_service_config_s
{
    INT32U                      service_id;	 /* The service id. */                           
    NE_service_search_type_e    search;    	 /* The search fields that used with rule. */    
    TM_port_e                   port;      	 /* The port that associated with the service. */
} NE_service_config_s;


/* NE_action_s */
typedef struct NE_action_s
{
    INT32U			dp_queue_id;		/* The id of the queue in data path that receive the frame. When the frame should discard in data path the user need to set this value to TM_queue_id_discard. */
    INT32U			sniff_queue_id;		/* The sniff port queue id. When the frame should discard in sniff port the user need to set this value to TM_queue_id_discard. */
	INT32U			cpu_queue_id;		/* The cpu queue id. When the frame should not be uploaded to cpu the user need to set this value to TM_queue_id_discard. */
    NE_edit_op_s    edit;				/* The editing rule. */
    INT32U			dp_rate_limit0;		/* The rate limiter id of first data path rate limiter. 0 mean no rate limiter. */
    INT32U			dp_rate_limit1;		/* The rate limiter id of second data path rate limiter. 0 mean no rate limiter. */
    INT32U			dp_rate_limit2;		/* The rate limiter id of third data path rate limiter. 0 mean no rate limiter. */
    INT32U			cpu_flow_id;		/* User defined value that enable to user to know the rule that used for receiving the frame. */
    INT32U			ellid;				/* The egress LLID. (This field only used in downstream direction) */
} NE_action_s;

/* NE_mc_rule_type_e */
typedef enum NE_mc_rule_type_e
{
    NE_MC_RULE_TYPE_DA,                  /* Multicast according to DA MAC address. */
    NE_MC_RULE_TYPE_DA_VID,              /* Multicast according to DA MAC address and VID. */
    NE_MC_RULE_TYPE_DA_IPV4_SIP,         /* Multicast according to DA MAC address and IPv4 SIP. */
    NE_MC_RULE_TYPE_DA_IPV6_SIP,         /* Multicast according to DA MAC address and IPv6 SIP. */
	NE_MC_RULE_TYPE_IPV4_DIP,			 /* Multicast according to IPv4 DIP */
	NE_MC_RULE_TYPE_IPV6_DIP_LSB,		 /* Multicast according to IPv6 DIP 80 less significant bits */
	NE_MC_RULE_TYPE_IPV4_SIP_DIP,		 /* Multicast according to IPv4 SIP and DIP */
	NE_MC_RULE_TYPE_IPV6_SIP_DIP,		 /* Multicast according to IPv6 SIP and DIP */
    NE_MC_RULE_TYPE_IPV4_DIP_VID,        /* Multicast according to IPv4 DIP and vid. */
    NE_MC_RULE_TYPE_IPV6_DIP_VID,        /* Multicast according to IPv6 DIP and vid. */
    NE_MC_RULE_TYPE_IPV6_DIP_LSB_VID,	 /* Multicast according to 64bit LSB of IP DIP & VID  */
    NE_MC_RULE_TYPE_IPV4_SIP_DIP_VID,    /* Multicast according to ipv6 SIP, DIP and vid. */
    NE_MC_RULE_TYPE_IPV6_SIP_DIP_VID,    /* Multicast according to ipv6 SIP, DIP and vid. Only 16 VID are supported. */
    NE_MC_RULE_TYPE_IPV4_VID,            /* Multicast according to VID and that the packet is IPv4 multicast. (Packets that are used this VID and not IPv4 multicast will discard.) */
    NE_MC_RULE_TYPE_IPV6_VID,            /* Multicast according to VID and that the packet is IPv6 multicast. (Packets that are used this VID and not IPv6 multicast will discard.) */
    NE_MC_RULE_TYPE_VID,				 /* Multicast according to vid only */
    NE_MC_RULE_TYPE_ALL	                 /* This type only can be used for getting all the rules type using the function NE_mc_rule_get */

} NE_mc_rule_type_e;

/* NE_mc_data_s */
typedef struct
{
	NE_mc_rule_type_e 		mc_type;
	NE_field_value_u		dst;
	NE_field_value_u		src;
	INT32U					vid;
	TM_port_e				port;
	NE_action_s				act;
} NE_mc_data_s;

typedef enum NE_service_op_e
{
	NE_SERVICE_OP_SA_LEARN_MODE				    = 0,     /* Set the addres table source address learn configuration. */
	NE_SERVICE_OP_DA_DISCARD				    = 1,     /*	Set the default value that used in learning for da_discard. */
	NE_SERVICE_OP_SA_DISCARD				    = 4,     /*	Set the default value that used in learning for sa_discard. */
	NE_SERVICE_OP_MIGRATION_ENABLE			    = 11,    /*	Enable learn frame with the same LLID and the same port. */
	NE_SERVICE_OP_MIGRATION_DISCARD			    = 12,    /*	Discard frames with learn address that migrate. */
	NE_SERVICE_OP_MIGRATION_TO_SNIFF		    = 13,    /*	Send addresses that migrate to sniff port queue. */ 
	NE_SERVICE_OP_MIGRATION_TO_CPU			    = 14,    /*	Send address that migrates to CPU queue. */ 
	NE_SERVICE_OP_MIGRATION_CPU_SNIFF_ID	    = 15,    /*	The sniff id that identify the frame when it receive by CPU. The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_UNBIND_DISCARD			    = 16,    /*	Discard frames with address that not according to the bind. */
	NE_SERVICE_OP_UNBIND_TO_SNIFF			    = 17,    /*	Send addresses that unbind to sniff port queue. */ 
	NE_SERVICE_OP_UNBIND_TO_CPU				    = 18,    /*	Send address that unbind to CPU queue. */ 
	NE_SERVICE_OP_UNBIND_CPU_SNIFF_ID		    = 19,    /*	The sniff id that identify the frame when it receive by CPU. Should be number in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_SA_MATCH_QUEUE_ID			    = 20,    /*	The default data path queue id to send frame that match. */
	NE_SERVICE_OP_SA_MATCH_TO_SNIFF			    = 21,    /*	Send source address to sniff port queue. */ 
	NE_SERVICE_OP_SA_MATCH_TO_CPU			    = 22,    /*	Send source address to CPU queue. */ 
	NE_SERVICE_OP_SA_MATCH_CPU_SNIFF_ID		    = 23,    /*	The sniff id that identify the frame when it receive by CPU.  The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_SA_NO_MATCH_QUEUE_ID		    = 24,    /*	The default data path queue id to send frame that match. */
	NE_SERVICE_OP_SA_NO_MATCH_TO_SNIFF		    = 25,    /*	Send source address to sniff port queue. */ 
	NE_SERVICE_OP_SA_NO_MATCH_TO_CPU		    = 26,    /*	Send source address to CPU queue. */ 
	NE_SERVICE_OP_SA_NO_MATCH_CPU_SNIFF_ID	    = 27,    /*	The sniff id that identify the frame when it receive by CPU.  The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_IVT_MODE					    = 28,    /*	Set the use of ivt id. For example enable binding. */
	NE_SERVICE_OP_DA_SEARCH_MODE			    = 29,    /*	Set the addres table destination address search configuration. (The value NE_addr_op_mode_learn is not valid). */
	NE_SERVICE_OP_DA_MATCH_CPU_SNIFF_ID		    = 30,    /*	The flow id that identify the frame when it receive by CPU. Should be number in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_DA_MATCH_CPU_QUEUE_ID	    	= 31,    /*	The CPU queue id to send frame that match and have attribute to send to CPU. */
	NE_SERVICE_OP_DA_MATCH_SNIFF_QUEUE_ID	    = 32,    /*	The sniff port queue id to send frame that match and have attribute to send to sniff port. */
	NE_SERVICE_OP_DA_NO_MATCH_TO_SNIFF		    = 33,    /*	Send destination address that isn't in address table to sniff port queue. */ 
	NE_SERVICE_OP_DA_NO_MATCH_TO_CPU		    = 34,    /*	Send destination address that isn't in address table to CPU queue. */ 
	NE_SERVICE_OP_DA_NO_MATCH_CPU_SNIFF_ID	    = 35,    /*	The sniff id that identify the frame when it receive by CPU. The valid values are in the range 0 to 63. When this used it always overwrite flow_id receive from other rules. */
	NE_SERVICE_OP_DA_NO_MATCH_ELLID			    = 36,    /*	The default LLID that use when destination address not match. */ 
	NE_SERVICE_OP_DA_NO_MATCH_USE_ANTI_LLID	    = 37,    /*	Use anti-LLID when destination address not matches. */ 

/* @@@ need to add support...CLS_NE_L2 */
	NE_SERVICE_OP_ADD_VALUES				    = 38,    /*	Pointer to structure that contains the values that will used to identify the service. This option can be used on with service that document to support this function. */ 
	NE_SERVICE_OP_DEL_VALUES				    = 39,    /*	Pointer to structure that contains the values that will remove and will not used to identify the service. This option can be used on with service that document to support this function. */ 
	NE_SERVICE_OP_GET_VALUES				    = 40,    /*	Pointer to structure that contains the values that are used to identify the service. This option can be used on with service that document to support this function. */ 
    NE_SERVICE_OP_ADDR_TYPE						= 41,	 /* The type of address that used by the service. If this is change the group NE_service_op_addr_group set by default to the value that is used only by this service. The addresses that already learned will not affected by this change. Valid values are only NE_addr_rule_type_mac, NE_addr_rule_type_mac_vid. */    
	NE_SERVICE_OP_ADDR_GROUP					= 42,	 /* Define the group of rules that used this addresses. By default it is 0 and share between all the services (except multicast). The group of services must have the same address type. The value is in the range 0 to 63. Each different id represents partition of the address table to different group of addresses and effectively split the address table to number tables. The addresses that already learned will not affected by this change. */
} NE_service_op_e;

/* NE_service_search_value_s */
typedef struct NE_service_search_value_s
{
	NE_service_search_type_e	search_type;/* The search fields that used with rule. */
	INT32U						llid;		/* LLID */
	INT32U						ethertype;	/* Ethernet type. */
	INT32U						pr_vid;		/* Primary VLAN id. This value can be TM_NE_NO_VLAN in case primary VLAN not exists. */
	INT32U						pr_prio;	/* Primary VLAN priority. */
	NE_VLAN_type_e				pr_vtype;	/* Primary VLAN type. */
	INT32U						sc_vid;		/* Secondary VLAN id. This value can be TM_NE_NO_VLAN in case secondary VLAN not exists. */
	INT32U						sc_prio;	/* Secondary VLAN priority. */
	NE_VLAN_type_e				sc_vtype;	/* Secondary VLAN type. */
} NE_service_search_value_s;

/* NE_user_data_s */
typedef struct NE_user_data_s
{
	INT32U						id;			/* To which service_id or service_map_id to add the rule. This id is received from NE_service_create or NE_service_map_create functions. */
	NE_service_search_value_s	values;		/* The value is structure with the fields that used for classification. */
	NE_action_s					action;		/* The actions that will done on the frame. Like target queue, VLAN editing rate limiting and etc. */
	INT32U						user_id;	/* Optional number with value in the range 0-4095 that identify group of user rule. This value should set to USER_ID_UNUSED if not used.  Only one rule can use the same user_id per direction. It responsible of the user application to mange the user_id allocation. */
} NE_user_data_s;

/* NE_user_config_s */
typedef struct NE_user_config_s
{
    INT32U    			rule_id;    /* To which service_id or service_map_id to add the rule. This id is received from NE_service_create or NE_service_map_create functions. */
    BOOLEAN    			bdirction;	/* Rule is b-direction or not. */
    NE_user_data_s    	data;    	/* The rule data or rule upstream data in case of b-direction rule. */
    NE_user_data_s    	down_data;  /* The downstream data for bi-directional rule. */
} NE_user_config_s;

/* NE_discard_op_e */
typedef enum NE_discard_op_e
{
    NE_DISCARD_OP_EQUAL,    /* Test that specify field is equal to the API value. */
    NE_DISCARD_OP_RANGE     /* Test that specify field is range between values. */
} NE_discard_op_e;

/* NE_field_type_e */
typedef enum NE_field_type_e
{
    NE_FIELD_TYPE_INVALID = 0,          /* Field type is invalid */
    NE_FIELD_TYPE_PR_VLAN = 3,          /* The frame has primary VLAN. */
    NE_FIELD_TYPE_SC_VLAN = 4,          /* The frame has secondary VLAN. */
    NE_FIELD_TYPE_IPV4 = 5,             /* The frame is IPv4. */
    NE_FIELD_TYPE_IPV6 = 6,             /* The frame is IPv6. */
    NE_FIELD_TYPE_TCP = 7,              /* The frame is UDP. */
    NE_FIELD_TYPE_UDP = 8,              /* The frame is TCP. */
    NE_FIELD_TYPE_IGMP = 9,             /* The frame is IGMP. */
    NE_FIELD_TYPE_ICMPV6 = 10,           /* The frame is ICMPv6. */
    NE_FIELD_TYPE_DIRECTION = 15,        /* 0 for downstream direction */
    NE_FIELD_TYPE_IP_MC = 27,            /* The frame is IP multicast. */
    NE_FIELD_TYPE_MC = 28,               /* The frame is multicast */
    NE_FIELD_TYPE_BC = 29,               /* The frame is broadcast */
    NE_FIELD_TYPE_CFI = 30,              /* The value of CFI bit in primary VLAN. */
    NE_FIELD_TYPE_SA = 64,               /* MAC source address */
    NE_FIELD_TYPE_DA,               /* MAC destination address */
    NE_FIELD_TYPE_ETHER_TYPE,       /* Ethernet type */
    NE_FIELD_TYPE_PR_VID,           /* Primary VLAN ID */
    NE_FIELD_TYPE_PR_COS,           /* Primary VLAN priority COS */
    NE_FIELD_TYPE_SC_VID,           /* Secondary VLAN ID */
    NE_FIELD_TYPE_SC_COS,           /* Secondary VLAN priority COS */
    NE_FIELD_TYPE_IPV4_SIP,         /* IPv4 source address */
    NE_FIELD_TYPE_IPV4_DIP,         /* IPv4 destination address */
    NE_FIELD_TYPE_IPV4_TOS,         /* IPv4 type of service */
    NE_FIELD_TYPE_IPV4_DSCP,        /* IPv4 differential service */
    NE_FIELD_TYPE_IPV4_PROTOCOL,    /* IPv4 protocol */
    NE_FIELD_TYPE_IPV6_SIP,         /* IPv6 source address */
    NE_FIELD_TYPE_IPV6_DIP,         /* IPv6 destination address */
    NE_FIELD_TYPE_IPV6_TOS,         /* IPv6 type of service */
    NE_FIELD_TYPE_IPV6_DSCP,        /* IPv6 differential service */
    NE_FIELD_TYPE_IPV6_PROTOCOL,    /* IPv6 protocol */
    NE_FIELD_TYPE_TCP_SRC,          /* TCP source port */
    NE_FIELD_TYPE_TCP_DST,          /* TCP destination address */
    NE_FIELD_TYPE_UDP_SRC,          /* UDP source port */
    NE_FIELD_TYPE_UDP_DST,          /* UDP destination address */
    NE_FIELD_TYPE_IPV4TCP_SRC,      /* IPv4 TCP source port */
    NE_FIELD_TYPE_IPV4TCP_DST,      /* IPv4 TCP destination address */
    NE_FIELD_TYPE_IPV4UDP_SRC,      /* IPv4 UDP source port */
    NE_FIELD_TYPE_IPV4UDP_DST,      /* IPv4 UDP destination address */
    NE_FIELD_TYPE_IPV6TCP_SRC,      /* IPv6 TCP source port */
    NE_FIELD_TYPE_IPV6TCP_DST,      /* IPv6 TCP destination address */
    NE_FIELD_TYPE_IPV6UDP_SRC,      /* IPv6 UDP source port */
    NE_FIELD_TYPE_IPV6UDP_DST,      /* IPv6 UDP destination address */
	NE_FIELD_TYPE_IPV6_FLOW_LABEL,	/* IPv6 flow label */
	NE_FIELD_TYPE_LLID				/* The frame LLID */
} NE_field_type_e;

/* NE_service_map_type_e */
typedef enum NE_service_map_type_e
{
	NE_SERVICE_MAP_DSCP_IPV4_SEARCH_ILLID = 256,			/* Mapped primary VLAN priority and queue according to IPv4 DSCP value. Search field in the user rules is according to ingress LLID. */
	NE_SERVICE_MAP_DSCP_IPV6_SEARCH_ILLID,					/* Mapped primary VLAN priority and queue according to IPv6 DSCP value. Search field in the user rules is according to ingress LLID. */
	NE_SERVICE_MAP_DSCP_IPV4_IPV6_SEARCH_ILLID,				/* Mapped primary VLAN priority and queue according to IPv4 and IPv6 DSCP value. Search field in the user rules is according to ingress LLID. */
	NE_SERVICE_MAP_VLAN_PRIO_SEARCH_LLID,					/* Mapped primary VLAN priority and queue according to VLAN COS. The search fields in the user rules are according to LLID. */
	NE_SERVICE_MAP_VLAN_PRIO_SEARCH_LLID_VLAN,				/* Mapped primary VLAN priority and queue according to VLAN COS. The search fields in the user rules are according to LLID and VLAN. */
	NE_SERVICE_MAP_VLAN_PRIO_SEARCH_VLAN,					/* Mapped primary VLAN priority and queue according to VLAN COS. The search fields in the user rules are according to VLAN. */
	NE_SERVICE_MAP_VLAN_PRIO_GLOBALLY_SEARCH_VLAN_DOWN,		/* Mapped primary VLAN priority and queue according to VLAN COS. The search fields in the user rules are according to VLAN. */
	NE_SERVICE_MAP_SCVLAN_PRIO_GLOBALLY_SEARCH_PRVLAN_DOWN,	/* Mapped primary VLAN priority and queue according to secondary VLAN COS if the primary VLAN is found or not. The search fields in the user rules are according to the primary VLAN. */
	NE_SERVICE_MAP_TAGGED_SEARCH_LLID,						/* Mapped primary VLAN priority and queue according to frame have primary VLAN tag or not. The search fields in the user rules are according to LLID. */
	NE_SERVICE_MAP_TRANSPARENT_UP,							/* Default map in the upstream direction that is based on primary VLAN priority and queue according to VLAN COS. No user rules can be defined. The frames are redirect according to address table. Only one service map of this type can be defined. */
	NE_SERVICE_MAP_TRANSPARENT_DOWN_PON0,					/* Default map in the downstream direction to PON0 that is based on primary VLAN priority and queue according to VLAN COS. No user rules can be defined. The frames are redirect according to address table. If the source port is TM_port_cni and the service map NE_service_map_transparent_down_pon1 is not define all the entries for PON1 will discard. Only one service map of this type can be defined. */
	NE_SERVICE_MAP_TRANSPARENT_DOWN_PON1					/* Default map in the downstream direction to PON1 that is based on primary VLAN priority and queue according to VLAN COS. No user rules can be defined. The frames are redirect according to address table. If the source port is TM_port_cni and the service map NE_service_map_transparent_down_pon0 is not define all the entries for PON0 will discard. Only one service map of this type can be defined. */

} NE_service_map_type_e;

/* NE_service_map_value_s */
typedef struct NE_service_map_value_s
{
	INT32U	priority_src;	/* The value of the search for priority. It can be VLAN priority or DSCP */
	INT32U	priority;		/* The VLAN priority that used when VLAN edit is done on frame that have the map value. Value of 8 mean that VLAN priority will not change */
	INT32U	queue_id;		/* The target queue_id for user rule with the map value. All queue_ids in map must have the same destination port */

} NE_service_map_value_s;

/* NE_service_map_data_s */
typedef struct NE_service_map_data_s
{
    NE_service_map_value_s		values[SERVICE_MAP_VALUES_ARRAY_SIZE];    				/* The mapping values table. */
    INT32U    					default_tagged_queue_id;    /* The default queue_id setting for VLAN tagged frame. If queue_id is equal to TM_queue_id_invalid the fame is discarded. */
    INT32U    					default_tagged_priority;    /* The VLAN priority that used when VLAN edit is done on frame that have tagged. Value of 8 mean that VLAN priority will not change. */
    INT32U    					default_untagged_queue_id;  /* The default queue_id setting for frame without VLAN tag. If queue_id is equal to TM_queue_id_invalid the fame is discarded. */
    INT32U    					default_untagged_priority;	/* The VLAN priority that used when VLAN edit is done on frame without tagged. Value of 8 mean that VLAN priority will not change. */
} NE_service_map_data_s;

/* NE_service_map_config_s */
typedef struct NE_service_map_config_s
{
    INT32U    				service_map_id; /* The service_map_id */
    NE_service_map_type_e   map_type;    	/* The service map type that used to set priority base on field and user rule search */
    NE_service_map_data_s   data;    		/* The serice map configuration value */
    TM_port_e    			port;    		/* The port of the rule */
} NE_service_map_config_s;

/* NE_addr_op_mode_e */
typedef enum NE_addr_op_mode_e
{
	NE_ADDR_OP_MODE_NONE,	/* Don't learn */ 
	NE_ADDR_OP_MODE_SEARCH,	/* Search address */ 
	NE_ADDR_OP_MODE_LEARN	/* Search and learn address */
} NE_addr_op_mode_e;

/* NE_addr_group_e */
typedef enum NE_addr_group_e
{
    NE_ADDR_GROUP_MAC = 0,
    NE_ADDR_GROUP_VID = 58,
	NE_ADDR_GROUP_MAC_VID = 59,
	NE_ADDR_GROUP_IPV4_DIP = 60,
	NE_ADDR_GROUP_IPV4_DIP_VID = 61,
	NE_ADDR_GROUP_IPV6_DIP = 62
} NE_addr_group_e;

typedef enum TM_cpu_rate_limit_type_e
{
    TM_CPU_RATE_LIMIT_TYPE_UP_PER_LLID = 0,             /* Rate limiter is per LLID in the upstream direction */
    TM_CPU_RATE_LIMIT_TYPE_CNI0_DOWN_PER_SNIFF_ID = 1,  /* Rate limiter is per sniff ID in the CNI0 downstream direction */
    TM_CPU_RATE_LIMIT_TYPE_CNI1_DOWN_PER_SNIFF_ID = 2   /* Rate limiter is per sniff ID in the CNI1 downstream direction */
    
} TM_cpu_rate_limit_type_e;



#if 1 /*统计相关*/
#define STAT_UNKNOWN_NAME				   "UNKNOWN STATISTIC"                 


/*****************************************************************************/
/*****************************************************************************/
/*
 * FW <-> HOST common definitions (MUST be the same definitions to FW and HOST)
 */
/*****************************************************************************/
/*****************************************************************************/
 

typedef enum
{
	LAG_CHNL_LAG_STAT_FIRST					= 0,

	/* RLAG_COUNTERS_CLEAR */
	LAG_CHNL_RLAG_FIRST                       = LAG_CHNL_LAG_STAT_FIRST,
	LAG_CHNL_RLAG_FRM_IN_COUNTER              = LAG_CHNL_RLAG_FIRST,
	LAG_CHNL_RLAG_FRM_OUT_COUNTER,
	LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW,
	LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH,
	LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW,
	LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH,
	LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER,
	LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER,
	LAG_CHNL_RLAG_LAST,

	/* TLAG_MISC_COUNTERS_CLEAR         */
	/* TLAG_CBFC_COUNTERS_CLEAR         */
	LAG_CHNL_TLAG_FIRST                       = LAG_CHNL_RLAG_LAST,
	LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER        = LAG_CHNL_TLAG_FIRST,
	LAG_CHNL_TLAG_FRMN_COUNTER,
	LAG_CHNL_TLAG_BYTEN_COUNTER_LOW,
	LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH,
	LAG_CHNL_TLAG_FRM_OUT_COUNTER,
	LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW,
	LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH,
	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_01,
	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_23,
	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_45,
	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_67,
	LAG_CHNL_TLAG_LAST,

	LAG_CHNL_LNC_FIRST                        = LAG_CHNL_TLAG_LAST,
	LAG_CHNL_LNC_LEN_ERR_CNT                  = LAG_CHNL_LNC_FIRST,	
	LAG_CHNL_LNC_LAST,

	/* CBP_COUNTERS_CLEAR               */
	LAG_CHNL_CBP_FIRST                        = LAG_CHNL_LNC_LAST,
	LAG_CHNL_CBP_PRIO_7_PAUSE_CNT             = LAG_CHNL_CBP_FIRST, 
	LAG_CHNL_CBP_PRIO_6_PAUSE_CNT, 
	LAG_CHNL_CBP_PRIO_5_PAUSE_CNT, 
	LAG_CHNL_CBP_PRIO_4_PAUSE_CNT,
	LAG_CHNL_CBP_PRIO_3_PAUSE_CNT,  
	LAG_CHNL_CBP_PRIO_2_PAUSE_CNT, 
	LAG_CHNL_CBP_PRIO_1_PAUSE_CNT, 
	LAG_CHNL_CBP_PRIO_0_PAUSE_CNT, 
	LAG_CHNL_CBP_TOTAL_PAUSE_CNT, 
	LAG_CHNL_CBP_TOTAL_DRP_CNT,  
	LAG_CHNL_CBP_LAST,

	LAG_CHNL_TRP_FIRST                 = LAG_CHNL_CBP_LAST,
	LAG_CHNL_TRP_FRM_MARK_REQ_DROP_CNT = LAG_CHNL_TRP_FIRST,
	LAG_CHNL_TRP_FRM_MARK_REQ_CNT,                          
	LAG_CHNL_TRP_LAST,

	LAG_CHNL_PRP_FIRST            = LAG_CHNL_TRP_LAST,
	LAG_CHNL_PRP_GOOD_PKT_CNT = LAG_CHNL_PRP_FIRST,  
	LAG_CHNL_PRP_BAD_PKT_CNT,                       
	LAG_CHNL_PRP_LAST,

	LAG_CHNL_LAG_STAT_LAST        = LAG_CHNL_PRP_LAST,

} LAG_CHNL_stat_counter_e;



#define LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW_NAME			"LAG_CHNL_RLAG_BYTE_IN_COUNTER_LOW"
#define LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH_NAME			"LAG_CHNL_RLAG_BYTE_IN_COUNTER_HIGH"
#define LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW_NAME			"LAG_CHNL_RLAG_BYTE_OUT_COUNTER_LOW"
#define LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH_NAME		"LAG_CHNL_RLAG_BYTE_OUT_COUNTER_HIGH"
#define	LAG_CHNL_RLAG_FRM_IN_COUNTER_NAME				"LAG_CHNL_RLAG_FRM_IN_COUNTER"
#define	LAG_CHNL_RLAG_FRM_OUT_COUNTER_NAME				"LAG_CHNL_RLAG_FRM_OUT_COUNTER"
#define	LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER_NAME		"LAG_CHNL_RLAG_FRM_TOTAL_DROP_COUNTER"
#define	LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER_NAME			"LAG_CHNL_RLAG_FRM_IN_DROP_COUNTER"
#define	LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER_NAME			"LAG_CHNL_TLAG_FRM_MARK_RES_COUNTER"
#define	LAG_CHNL_TLAG_FRMN_COUNTER_NAME					"LAG_CHNL_TLAG_FRMN_COUNTER"
#define	LAG_CHNL_TLAG_BYTEN_COUNTER_LOW_NAME			"LAG_CHNL_TLAG_BYTEN_COUNTER_LOW"
#define	LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH_NAME			"LAG_CHNL_TLAG_BYTEN_COUNTER_HIGH"
#define	LAG_CHNL_TLAG_FRM_OUT_COUNTER_NAME				"LAG_CHNL_TLAG_FRM_OUT_COUNTER"
#define	LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW_NAME			"LAG_CHNL_TLAG_BYTE_OUT_COUNTER_LOW"
#define	LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH_NAME		"LAG_CHNL_TLAG_BYTE_OUT_COUNTER_HIGH"
#define	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_01_NAME			"LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_01"
#define	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_23_NAME			"LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_23"
#define	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_45_NAME			"LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_45"
#define	LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_67_NAME			"LAG_CHNL_TLAG_FRM_CBFC_COUNTERS_67"
#define	LAG_CHNL_LNC_LEN_ERR_CNT_NAME					"LAG_CHNL_LNC_LEN_ERR_CNT"
#define	LAG_CHNL_CBP_PRIO_7_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_7_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_6_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_6_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_5_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_5_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_4_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_4_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_3_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_3_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_2_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_2_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_1_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_1_PAUSE_CNT"
#define	LAG_CHNL_CBP_PRIO_0_PAUSE_CNT_NAME				"LAG_CHNL_CBP_PRIO_0_PAUSE_CNT"
#define	LAG_CHNL_CBP_TOTAL_PAUSE_CNT_NAME				"LAG_CHNL_CBP_TOTAL_PAUSE_CNT"
#define	LAG_CHNL_CBP_TOTAL_DRP_CNT_NAME					"LAG_CHNL_CBP_TOTAL_DRP_CNT"
#define	LAG_CHNL_TRP_FRM_MARK_REQ_DROP_CNT_NAME			"LAG_CHNL_TRP_FRM_MARK_REQ_DROP_CNT"
#define	LAG_CHNL_TRP_FRM_MARK_REQ_CNT_NAME				"LAG_CHNL_TRP_FRM_MARK_REQ_CNT"
#define	LAG_CHNL_PRP_GOOD_PKT_CNT_NAME					"LAG_CHNL_PRP_GOOD_PKT_CNT"
#define	LAG_CHNL_PRP_BAD_PKT_CNT_NAME					"LAG_CHNL_PRP_BAD_PKT_CNT"



typedef enum
{
	LAG_CHNL_1G_FIRST 						= LAG_CHNL_LAG_STAT_LAST,

	LAG_CHNL_1G_aFramesTransmittedOK		= LAG_CHNL_1G_FIRST,
	LAG_CHNL_1G_aFramesReceivedOK,
	LAG_CHNL_1G_aFrameCheckSequenceErrors,
	LAG_CHNL_1G_aAlignmentErrors,
	LAG_CHNL_1G_aOctetsTransmittedOK,
	LAG_CHNL_1G_aOctetsReceivedOK,
	LAG_CHNL_1G_aTxPAUSEMACCtrlFrames,
	LAG_CHNL_1G_aRxPAUSEMACCtrlFrames,
	LAG_CHNL_1G_ifInErrors,
	LAG_CHNL_1G_ifOutErrors,
	LAG_CHNL_1G_ifInUcastPkts,
	LAG_CHNL_1G_ifInMulticastPkts,
	LAG_CHNL_1G_ifInBroadcastPkts,
	LAG_CHNL_1G_ifOutDiscards,
	LAG_CHNL_1G_ifOutUcastPkts,
	LAG_CHNL_1G_ifOutMulticastPkts,
	LAG_CHNL_1G_ifOutBroadcastPkts,
	LAG_CHNL_1G_etherStatsDropEvents,
	LAG_CHNL_1G_etherStatsOctets,
	LAG_CHNL_1G_etherStatsPkts,
	LAG_CHNL_1G_etherStatsUndersizePkts,
	LAG_CHNL_1G_etherStatsOversizePkts,
	LAG_CHNL_1G_etherStatsPkts64Octets,
	LAG_CHNL_1G_etherStatsPkts65to127Octets,
	LAG_CHNL_1G_etherStatsPkts128to255Octets,
	LAG_CHNL_1G_etherStatsPkts256to511Octets,
	LAG_CHNL_1G_etherStatsPkts512to1023Octets,
	LAG_CHNL_1G_etherStatsPkts1024to1518Octets,
	LAG_CHNL_1G_etherStatsPkts1519toXOctets,
	LAG_CHNL_1G_etherStatsJabbers,
	LAG_CHNL_1G_etherStatsFragments,

	LAG_CHNL_1G_LAST 
} LAG_CHNL_1G_stat_counter_e;

#define	LAG_CHNL_1G_aFramesTransmittedOK_NAME			   		"LAG_CHNL_1G_aFramesTransmittedOK" 
#define LAG_CHNL_1G_aFramesReceivedOK_NAME						"LAG_CHNL_1G_aFramesReceivedOK"
#define LAG_CHNL_1G_aFrameCheckSequenceErrors_NAME				"LAG_CHNL_1G_aFrameCheckSequenceErrors"
#define LAG_CHNL_1G_aAlignmentErrors_NAME						"LAG_CHNL_1G_aAlignmentErrors"
#define LAG_CHNL_1G_aOctetsTransmittedOK_NAME					"LAG_CHNL_1G_aOctetsTransmittedOK"
#define LAG_CHNL_1G_aOctetsReceivedOK_NAME						"LAG_CHNL_1G_aOctetsReceivedOK"
#define LAG_CHNL_1G_aTxPAUSEMACCtrlFrames_NAME					"LAG_CHNL_1G_aTxPAUSEMACCtrlFrames"
#define LAG_CHNL_1G_aRxPAUSEMACCtrlFrames_NAME					"LAG_CHNL_1G_aRxPAUSEMACCtrlFrames"
#define LAG_CHNL_1G_ifInErrors_NAME 							"LAG_CHNL_1G_ifInErrors"
#define LAG_CHNL_1G_ifOutErrors_NAME							"LAG_CHNL_1G_ifOutErrors"
#define LAG_CHNL_1G_ifInUcastPkts_NAME							"LAG_CHNL_1G_ifInUcastPkts"
#define LAG_CHNL_1G_ifInMulticastPkts_NAME						"LAG_CHNL_1G_ifInMulticastPkts"
#define LAG_CHNL_1G_ifInBroadcastPkts_NAME						"LAG_CHNL_1G_ifInBroadcastPkts"
#define LAG_CHNL_1G_ifOutDiscards_NAME							"LAG_CHNL_1G_ifOutDiscards"
#define LAG_CHNL_1G_ifOutUcastPkts_NAME 						"LAG_CHNL_1G_ifOutUcastPkts"
#define LAG_CHNL_1G_ifOutMulticastPkts_NAME						"LAG_CHNL_1G_ifOutMulticastPkts"
#define LAG_CHNL_1G_ifOutBroadcastPkts_NAME						"LAG_CHNL_1G_ifOutBroadcastPkts"
#define LAG_CHNL_1G_etherStatsDropEvents_NAME					"LAG_CHNL_1G_etherStatsDropEvents"
#define LAG_CHNL_1G_etherStatsOctets_NAME						"LAG_CHNL_1G_etherStatsOctets"
#define LAG_CHNL_1G_etherStatsPkts_NAME							"LAG_CHNL_1G_etherStatsPkts"
#define LAG_CHNL_1G_etherStatsUndersizePkts_NAME				"LAG_CHNL_1G_etherStatsUndersizePkts"
#define LAG_CHNL_1G_etherStatsOversizePkts_NAME					"LAG_CHNL_1G_etherStatsOversizePkts"
#define LAG_CHNL_1G_etherStatsPkts64Octets_NAME					"LAG_CHNL_1G_etherStatsPkts64Octets"
#define LAG_CHNL_1G_etherStatsPkts65to127Octets_NAME			"LAG_CHNL_1G_etherStatsPkts65to127Octets"
#define LAG_CHNL_1G_etherStatsPkts128to255Octets_NAME			"LAG_CHNL_1G_etherStatsPkts128to255Octets"
#define LAG_CHNL_1G_etherStatsPkts256to511Octets_NAME			"LAG_CHNL_1G_etherStatsPkts256to511Octets"
#define LAG_CHNL_1G_etherStatsPkts512to1023Octets_NAME			"LAG_CHNL_1G_etherStatsPkts512to1023Octets"
#define LAG_CHNL_1G_etherStatsPkts1024to1518Octets_NAME			"LAG_CHNL_1G_etherStatsPkts1024to1518Octets"
#define LAG_CHNL_1G_etherStatsPkts1519toXOctets_NAME			"LAG_CHNL_1G_etherStatsPkts1519toXOctets"
#define LAG_CHNL_1G_etherStatsJabbers_NAME						"LAG_CHNL_1G_etherStatsJabbers"
#define LAG_CHNL_1G_etherStatsFragments_NAME					"LAG_CHNL_1G_etherStatsFragments" 

/* NOTE: 'L' (LSB) MUST preceed 'U' (MSB) and match order of lag_10g_mac_counter_address in STATISTICS_lag.c */
typedef enum
{
	LAG_CHNL_10G_FIRST 								= LAG_CHNL_1G_LAST,
	LAG_CHNL_10G_aFramesTransmittedOK_L 			= LAG_CHNL_10G_FIRST,
	LAG_CHNL_10G_aFramesTransmittedOK_U,
	LAG_CHNL_10G_aFramesReceivedOK_L, 
	LAG_CHNL_10G_aFramesReceivedOK_U, 
	LAG_CHNL_10G_aFrameCheckSequenceErrors_L,
	LAG_CHNL_10G_aFrameCheckSequenceErrors_U,
	LAG_CHNL_10G_aAlignmentErrors_L,
	LAG_CHNL_10G_aAlignmentErrors_U,
	LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_L,
	LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_U,
	LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_L,
	LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_U,
	LAG_CHNL_10G_aFrameTooLongErrors_L,
	LAG_CHNL_10G_aFrameTooLongErrors_U,
	LAG_CHNL_10G_aInRangeLengthErrors_L,
	LAG_CHNL_10G_aInRangeLengthErrors_U,
	LAG_CHNL_10G_VLANTransmittedOK_L,
	LAG_CHNL_10G_VLANTransmittedOK_U,
	LAG_CHNL_10G_VLANReceivedOK_L,
	LAG_CHNL_10G_VLANReceivedOK_U,
	LAG_CHNL_10G_ifOutOctets_L,
	LAG_CHNL_10G_ifOutOctets_U ,
	LAG_CHNL_10G_ifInOctets_L,
	LAG_CHNL_10G_ifInOctets_U,
	LAG_CHNL_10G_ifInUcastPkts_L,
	LAG_CHNL_10G_ifInUcastPkts_U,
	LAG_CHNL_10G_ifInMulticastPkts_L,
	LAG_CHNL_10G_ifInMulticastPkts_U,
	LAG_CHNL_10G_ifInBroadcastPkts_L,
	LAG_CHNL_10G_ifInBroadcastPkts_U,
	LAG_CHNL_10G_ifOutErrors_L,
	LAG_CHNL_10G_ifOutErrors_U,
	LAG_CHNL_10G_ifOutUcastPkts_L,
	LAG_CHNL_10G_ifOutUcastPkts_U,
	LAG_CHNL_10G_ifOutMulticastPkts_L,
	LAG_CHNL_10G_ifOutMulticastPkts_U,
	LAG_CHNL_10G_ifOutBroadcastPkts_L,
	LAG_CHNL_10G_ifOutBroadcastPkts_U,
	LAG_CHNL_10G_etherStatsDropEvents_L,
	LAG_CHNL_10G_etherStatsDropEvents_U,
	LAG_CHNL_10G_etherStatsOctets_L,
	LAG_CHNL_10G_etherStatsOctets_U,
	LAG_CHNL_10G_etherStatsPkts_L,
	LAG_CHNL_10G_etherStatsPkts_U,
	LAG_CHNL_10G_etherStatsUndersizePkts_L,
	LAG_CHNL_10G_etherStatsUndersizePkts_U,
	LAG_CHNL_10G_etherStatsPkts64Octets_L,
	LAG_CHNL_10G_etherStatsPkts64Octets_U,
	LAG_CHNL_10G_etherStatsPkts65to127Octets_L,
	LAG_CHNL_10G_etherStatsPkts65to127Octets_U,
	LAG_CHNL_10G_etherStatsPkts128to255Octets_L,
	LAG_CHNL_10G_etherStatsPkts128to255Octets_U,
	LAG_CHNL_10G_etherStatsPkts256to511Octets_L,
	LAG_CHNL_10G_etherStatsPkts256to511Octets_U,
	LAG_CHNL_10G_etherStatsPkts512to1023Octets_L,
	LAG_CHNL_10G_etherStatsPkts512to1023Octets_U,
	LAG_CHNL_10G_etherStatsPkts1024to1518Octets_L,
	LAG_CHNL_10G_etherStatsPkts1024to1518Octets_U,
	LAG_CHNL_10G_etherStatsPkts1519toX_L,
	LAG_CHNL_10G_etherStatsPkts1519toX_U,
	LAG_CHNL_10G_etherStatsOversizePkts_L,
	LAG_CHNL_10G_etherStatsOversizePkts_U,
	LAG_CHNL_10G_etherStatsJabbers_L,
	LAG_CHNL_10G_etherStatsJabbers_U,
	LAG_CHNL_10G_etherStatsFragments_L,
	LAG_CHNL_10G_etherStatsFragments_U,
	LAG_CHNL_10G_ifInErrors_L,
	LAG_CHNL_10G_ifInErrors_U,
	LAG_CHNL_10G_LAST 
} LAG_CHNL_10G_stat_counter_e;

#define	LAG_CHNL_10G_aFramesTransmittedOK_L_NAME				"LAG_CHNL_10G_aFramesTransmittedOK_L"			
#define	LAG_CHNL_10G_aFramesTransmittedOK_U_NAME				"LAG_CHNL_10G_aFramesTransmittedOK_U"
#define	LAG_CHNL_10G_aFramesReceivedOK_L_NAME                   "LAG_CHNL_10G_aFramesReceivedOK_L"
#define	LAG_CHNL_10G_aFramesReceivedOK_U_NAME					"LAG_CHNL_10G_aFramesReceivedOK_U"
#define	LAG_CHNL_10G_aFrameCheckSequenceErrors_L_NAME			"LAG_CHNL_10G_aFrameCheckSequenceErrors_L"
#define	LAG_CHNL_10G_aFrameCheckSequenceErrors_U_NAME			"LAG_CHNL_10G_aFrameCheckSequenceErrors_U"
#define	LAG_CHNL_10G_aAlignmentErrors_L_NAME					"LAG_CHNL_10G_aAlignmentErrors_L"
#define	LAG_CHNL_10G_aAlignmentErrors_U_NAME					"LAG_CHNL_10G_aAlignmentErrors_U"
#define	LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_L_NAME		"LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_L"
#define	LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_U_NAME		"LAG_CHNL_10G_aPAUSEMACCtrlFramesTransmitted_U"
#define	LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_L_NAME			"LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_L"
#define	LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_U_NAME			"LAG_CHNL_10G_aPAUSEMACCtrlFramesGW10G_Received_U"
#define	LAG_CHNL_10G_aFrameTooLongErrors_L_NAME					"LAG_CHNL_10G_aFrameTooLongErrors_L"
#define	LAG_CHNL_10G_aFrameTooLongErrors_U_NAME					"LAG_CHNL_10G_aFrameTooLongErrors_U"
#define	LAG_CHNL_10G_aInRangeLengthErrors_L_NAME				"LAG_CHNL_10G_aInRangeLengthErrors_L"
#define	LAG_CHNL_10G_aInRangeLengthErrors_U_NAME				"LAG_CHNL_10G_aInRangeLengthErrors_U"
#define	LAG_CHNL_10G_VLANTransmittedOK_L_NAME					"LAG_CHNL_10G_VLANTransmittedOK_L"
#define	LAG_CHNL_10G_VLANTransmittedOK_U_NAME					"LAG_CHNL_10G_VLANTransmittedOK_U"
#define	LAG_CHNL_10G_VLANReceivedOK_L_NAME						"LAG_CHNL_10G_VLANReceivedOK_L"
#define	LAG_CHNL_10G_VLANReceivedOK_U_NAME						"LAG_CHNL_10G_VLANReceivedOK_U"
#define	LAG_CHNL_10G_ifOutOctets_L_NAME							"LAG_CHNL_10G_ifOutOctets_L"
#define	LAG_CHNL_10G_ifOutOctets_U_NAME							"LAG_CHNL_10G_ifOutOctets_U"
#define	LAG_CHNL_10G_ifInOctets_L_NAME							"LAG_CHNL_10G_ifInOctets_L"
#define	LAG_CHNL_10G_ifInOctets_U_NAME							"LAG_CHNL_10G_ifInOctets_U"
#define	LAG_CHNL_10G_ifInUcastPkts_L_NAME						"LAG_CHNL_10G_ifInUcastPkts_L"
#define	LAG_CHNL_10G_ifInUcastPkts_U_NAME						"LAG_CHNL_10G_ifInUcastPkts_U"
#define	LAG_CHNL_10G_ifInMulticastPkts_L_NAME					"LAG_CHNL_10G_ifInMulticastPkts_L"
#define	LAG_CHNL_10G_ifInMulticastPkts_U_NAME					"LAG_CHNL_10G_ifInMulticastPkts_U"
#define	LAG_CHNL_10G_ifInBroadcastPkts_L_NAME					"LAG_CHNL_10G_ifInBroadcastPkts_L"
#define	LAG_CHNL_10G_ifInBroadcastPkts_U_NAME					"LAG_CHNL_10G_ifInBroadcastPkts_U"
#define	LAG_CHNL_10G_ifOutErrors_L_NAME							"LAG_CHNL_10G_ifOutErrors_L"
#define	LAG_CHNL_10G_ifOutErrors_U_NAME							"LAG_CHNL_10G_ifOutErrors_U"
#define	LAG_CHNL_10G_ifOutUcastPkts_L_NAME						"LAG_CHNL_10G_ifOutUcastPkts_L"
#define	LAG_CHNL_10G_ifOutUcastPkts_U_NAME						"LAG_CHNL_10G_ifOutUcastPkts_U"
#define	LAG_CHNL_10G_ifOutMulticastPkts_L_NAME					"LAG_CHNL_10G_ifOutMulticastPkts_L"
#define	LAG_CHNL_10G_ifOutMulticastPkts_U_NAME					"LAG_CHNL_10G_ifOutMulticastPkts_U"
#define	LAG_CHNL_10G_ifOutBroadcastPkts_L_NAME					"LAG_CHNL_10G_ifOutBroadcastPkts_L"
#define	LAG_CHNL_10G_ifOutBroadcastPkts_U_NAME					"LAG_CHNL_10G_ifOutBroadcastPkts_U"
#define	LAG_CHNL_10G_etherStatsDropEvents_L_NAME				"LAG_CHNL_10G_etherStatsDropEvents_L"
#define	LAG_CHNL_10G_etherStatsDropEvents_U_NAME				"LAG_CHNL_10G_etherStatsDropEvents_U"
#define	LAG_CHNL_10G_etherStatsOctets_L_NAME					"LAG_CHNL_10G_etherStatsOctets_L"
#define	LAG_CHNL_10G_etherStatsOctets_U_NAME					"LAG_CHNL_10G_etherStatsOctets_U"
#define	LAG_CHNL_10G_etherStatsPkts_L_NAME						"LAG_CHNL_10G_etherStatsPkts_L"
#define	LAG_CHNL_10G_etherStatsPkts_U_NAME						"LAG_CHNL_10G_etherStatsPkts_U"
#define	LAG_CHNL_10G_etherStatsUndersizePkts_L_NAME				"LAG_CHNL_10G_etherStatsUndersizePkts_L"
#define	LAG_CHNL_10G_etherStatsUndersizePkts_U_NAME				"LAG_CHNL_10G_etherStatsUndersizePkts_U"
#define	LAG_CHNL_10G_etherStatsPkts64Octets_L_NAME				"LAG_CHNL_10G_etherStatsPkts64Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts64Octets_U_NAME				"LAG_CHNL_10G_etherStatsPkts64Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts65to127Octets_L_NAME			"LAG_CHNL_10G_etherStatsPkts65to127Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts65to127Octets_U_NAME			"LAG_CHNL_10G_etherStatsPkts65to127Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts128to255Octets_L_NAME		"LAG_CHNL_10G_etherStatsPkts128to255Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts128to255Octets_U_NAME		"LAG_CHNL_10G_etherStatsPkts128to255Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts256to511Octets_L_NAME		"LAG_CHNL_10G_etherStatsPkts256to511Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts256to511Octets_U_NAME		"LAG_CHNL_10G_etherStatsPkts256to511Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts512to1023Octets_L_NAME		"LAG_CHNL_10G_etherStatsPkts512to1023Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts512to1023Octets_U_NAME		"LAG_CHNL_10G_etherStatsPkts512to1023Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts1024to1518Octets_L_NAME		"LAG_CHNL_10G_etherStatsPkts1024to1518Octets_L"
#define	LAG_CHNL_10G_etherStatsPkts1024to1518Octets_U_NAME		"LAG_CHNL_10G_etherStatsPkts1024to1518Octets_U"
#define	LAG_CHNL_10G_etherStatsPkts1519toX_L_NAME				"LAG_CHNL_10G_etherStatsPkts1519toX_L"
#define	LAG_CHNL_10G_etherStatsPkts1519toX_U_NAME				"LAG_CHNL_10G_etherStatsPkts1519toX_U"
#define	LAG_CHNL_10G_etherStatsOversizePkts_L_NAME				"LAG_CHNL_10G_etherStatsOversizePkts_L"
#define	LAG_CHNL_10G_etherStatsOversizePkts_U_NAME				"LAG_CHNL_10G_etherStatsOversizePkts_U"
#define	LAG_CHNL_10G_etherStatsJabbers_L_NAME					"LAG_CHNL_10G_etherStatsJabbers_L"
#define	LAG_CHNL_10G_etherStatsJabbers_U_NAME					"LAG_CHNL_10G_etherStatsJabbers_U"
#define	LAG_CHNL_10G_etherStatsFragments_L_NAME					"LAG_CHNL_10G_etherStatsFragments_L"
#define	LAG_CHNL_10G_etherStatsFragments_U_NAME					"LAG_CHNL_10G_etherStatsFragments_U"
#define	LAG_CHNL_10G_ifInErrors_L_NAME							"LAG_CHNL_10G_ifInErrors_L"
#define	LAG_CHNL_10G_ifInErrors_U_NAME							"LAG_CHNL_10G_ifInErrors_U"

typedef enum
{
  OLT_1G_MAC_LLID_FIRST_COUNTER,
  OLT_1G_MAC_LLID_TX_UCAST_IND_LSB = OLT_1G_MAC_LLID_FIRST_COUNTER,
  OLT_1G_MAC_LLID_TX_UCAST_IND_MSB,
  OLT_1G_MAC_LLID_TX_BROADCAST_IND_LSB,
  OLT_1G_MAC_LLID_TX_BROADCAST_IND_MSB,
  OLT_1G_MAC_LLID_TX_MULTICAST_IND_LSB,
  OLT_1G_MAC_LLID_TX_MULTICAST_IND_MSB,
  OLT_1G_MAC_LLID_VR1_PON_TX_PKT_LSB,
  OLT_1G_MAC_LLID_VR1_PON_TX_PKT_MSB,
  OLT_1G_MAC_LLID_OUT_OCTETS_LSB,
  OLT_1G_MAC_LLID_OUT_OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_LSB,
  OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_MSB,
  OLT_1G_MAC_LLID_TX_PKTS64OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS64OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_MSB,
  OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_LSB,
  OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_UCAST_IND_LSB,
  OLT_1G_MAC_LLID_RX_UCAST_IND_MSB,
  OLT_1G_MAC_LLID_RX_BROADCAST_IND_LSB,
  OLT_1G_MAC_LLID_RX_BROADCAST_IND_MSB,
  OLT_1G_MAC_LLID_RX_MULTICAST_IND_LSB,
  OLT_1G_MAC_LLID_RX_MULTICAST_IND_MSB,
  OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_LSB,
  OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_MSB,
  OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_LSB,
  OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_MSB,
  OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_LSB,
  OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_MSB,
  OLT_1G_MAC_LLID_IN_SYMBOL_ERR_LSB,
  OLT_1G_MAC_LLID_IN_SYMBOL_ERR_MSB,
  OLT_1G_MAC_LLID_RX_PKTS64OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS64OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_MSB,
  OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_LSB,
  OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_MSB,
  OLT_1G_MAC_LLID_SLD_ERR_LSB,
  OLT_1G_MAC_LLID_SLD_ERR_MSB,
  OLT_1G_MAC_LLID_CRC8_ERR_LSB,
  OLT_1G_MAC_LLID_CRC8_ERR_MSB,
  OLT_1G_MAC_LLID_BAD_LLID_LSB,
  OLT_1G_MAC_LLID_BAD_LLID_MSB,
  OLT_1G_MAC_LLID_GOOD_LLID_LSB,
  OLT_1G_MAC_LLID_GOOD_LLID_MSB,
  OLT_1G_MAC_LLID_PON_CAST_LLID_LSB,
  OLT_1G_MAC_LLID_PON_CAST_LLID_MSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_LSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_MSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_LSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_MSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_LSB,
  OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_MSB,
  OLT_1G_MAC_LLID_IN_ERRORS_LSB,
  OLT_1G_MAC_LLID_IN_ERRORS_MSB,
  OLT_1G_MAC_LLID_VR1_DISP_ERR_LSB,
  OLT_1G_MAC_LLID_VR1_DISP_ERR_MSB,
  OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_LSB,
  OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_MSB,
  OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_LSB,
  OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_MSB,
  OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_LSB,
  OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_MSB,
  OLT_1G_MAC_LLID_LAST_COUNTER = OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_MSB,
} OLT_1G_MAC_LLID_stat_counter_e;



#define OLT_1G_MAC_LLID_TX_UCAST_IND_LSB_NAME				   "OLT_1G_MAC_LLID_TX_UCAST_IND_LSB"                 
#define OLT_1G_MAC_LLID_TX_UCAST_IND_MSB_NAME				   "OLT_1G_MAC_LLID_TX_UCAST_IND_MSB"                 
#define OLT_1G_MAC_LLID_TX_BROADCAST_IND_LSB_NAME			   "OLT_1G_MAC_LLID_TX_BROADCAST_IND_LSB"             
#define OLT_1G_MAC_LLID_TX_BROADCAST_IND_MSB_NAME			   "OLT_1G_MAC_LLID_TX_BROADCAST_IND_MSB"             
#define OLT_1G_MAC_LLID_TX_MULTICAST_IND_LSB_NAME			   "OLT_1G_MAC_LLID_TX_MULTICAST_IND_LSB"             
#define OLT_1G_MAC_LLID_TX_MULTICAST_IND_MSB_NAME			   "OLT_1G_MAC_LLID_TX_MULTICAST_IND_MSB"             
#define OLT_1G_MAC_LLID_VR1_PON_TX_PKT_LSB_NAME				   "OLT_1G_MAC_LLID_VR1_PON_TX_PKT_LSB"               
#define OLT_1G_MAC_LLID_VR1_PON_TX_PKT_MSB_NAME				   "OLT_1G_MAC_LLID_VR1_PON_TX_PKT_MSB"               
#define OLT_1G_MAC_LLID_OUT_OCTETS_LSB_NAME					   "OLT_1G_MAC_LLID_OUT_OCTETS_LSB"                   
#define OLT_1G_MAC_LLID_OUT_OCTETS_MSB_NAME					   "OLT_1G_MAC_LLID_OUT_OCTETS_MSB"                   
#define OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_LSB_NAME			   "OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_LSB"              
#define OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_MSB_NAME			   "OLT_1G_MAC_LLID_TX_FEC_OH_BYTES_MSB"              
#define OLT_1G_MAC_LLID_TX_PKTS64OCTETS_LSB_NAME			   "OLT_1G_MAC_LLID_TX_PKTS64OCTETS_LSB"              
#define OLT_1G_MAC_LLID_TX_PKTS64OCTETS_MSB_NAME			   "OLT_1G_MAC_LLID_TX_PKTS64OCTETS_MSB"              
#define OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_LSB"         
#define OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS65TO127OCTETS_MSB"         
#define OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_LSB"        
#define OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS128TO255OCTETS_MSB"        
#define OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_LSB"        
#define OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS256TO511OCTETS_MSB"        
#define OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_LSB"       
#define OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTS512TO1023OCTETS_MSB"       
#define OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_LSB_NAME	   "OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_LSB"      
#define OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_MSB_NAME	   "OLT_1G_MAC_LLID_TX_PKTS1024TO1518OCTETS_MSB"      
#define OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_LSB"        
#define OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_TX_PKTSOVER1518OCTETS_MSB"        
#define OLT_1G_MAC_LLID_RX_UCAST_IND_LSB_NAME				   "OLT_1G_MAC_LLID_RX_UCAST_IND_LSB"                 
#define OLT_1G_MAC_LLID_RX_UCAST_IND_MSB_NAME				   "OLT_1G_MAC_LLID_RX_UCAST_IND_MSB"                 
#define OLT_1G_MAC_LLID_RX_BROADCAST_IND_LSB_NAME			   "OLT_1G_MAC_LLID_RX_BROADCAST_IND_LSB"             
#define OLT_1G_MAC_LLID_RX_BROADCAST_IND_MSB_NAME			   "OLT_1G_MAC_LLID_RX_BROADCAST_IND_MSB"             
#define OLT_1G_MAC_LLID_RX_MULTICAST_IND_LSB_NAME			   "OLT_1G_MAC_LLID_RX_MULTICAST_IND_LSB"             
#define OLT_1G_MAC_LLID_RX_MULTICAST_IND_MSB_NAME			   "OLT_1G_MAC_LLID_RX_MULTICAST_IND_MSB"             
#define OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_LSB_NAME		   "OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_LSB"          
#define OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_MSB_NAME		   "OLT_1G_MAC_LLID_RX_OVERSIZED_PACKET_MSB"          
#define OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_LSB_NAME		   "OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_LSB"         
#define OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_MSB_NAME		   "OLT_1G_MAC_LLID_RX_UNDERSIZED_PACKET_MSB"         
#define OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_LSB_NAME		   "OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_LSB"       
#define OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_MSB_NAME		   "OLT_1G_MAC_LLID_VR1_GMII_FCS_DISCARDED_MSB"       
#define OLT_1G_MAC_LLID_IN_SYMBOL_ERR_LSB_NAME				   "OLT_1G_MAC_LLID_IN_SYMBOL_ERR_LSB"                
#define OLT_1G_MAC_LLID_IN_SYMBOL_ERR_MSB_NAME				   "OLT_1G_MAC_LLID_IN_SYMBOL_ERR_MSB"                
#define OLT_1G_MAC_LLID_RX_PKTS64OCTETS_LSB_NAME			   "OLT_1G_MAC_LLID_RX_PKTS64OCTETS_LSB"              
#define OLT_1G_MAC_LLID_RX_PKTS64OCTETS_MSB_NAME			   "OLT_1G_MAC_LLID_RX_PKTS64OCTETS_MSB"              
#define OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_LSB"         
#define OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS65TO127OCTETS_MSB"         
#define OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_LSB"        
#define OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS128TO255OCTETS_MSB"        
#define OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_LSB"        
#define OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS256TO511OCTETS_MSB"        
#define OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_LSB"       
#define OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTS512TO1023OCTETS_MSB"       
#define OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_LSB_NAME	   "OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_LSB"      
#define OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_MSB_NAME	   "OLT_1G_MAC_LLID_RX_PKTS1024TO1518OCTETS_MSB"      
#define OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_LSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_LSB"        
#define OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_MSB_NAME		   "OLT_1G_MAC_LLID_RX_PKTSOVER1518OCTETS_MSB"        
#define OLT_1G_MAC_LLID_SLD_ERR_LSB_NAME					   "OLT_1G_MAC_LLID_SLD_ERR_LSB"                      
#define OLT_1G_MAC_LLID_SLD_ERR_MSB_NAME					   "OLT_1G_MAC_LLID_SLD_ERR_MSB"                      
#define OLT_1G_MAC_LLID_CRC8_ERR_LSB_NAME					   "OLT_1G_MAC_LLID_CRC8_ERR_LSB"                     
#define OLT_1G_MAC_LLID_CRC8_ERR_MSB_NAME					   "OLT_1G_MAC_LLID_CRC8_ERR_MSB"                     
#define OLT_1G_MAC_LLID_BAD_LLID_LSB_NAME					   "OLT_1G_MAC_LLID_BAD_LLID_LSB"                     
#define OLT_1G_MAC_LLID_BAD_LLID_MSB_NAME					   "OLT_1G_MAC_LLID_BAD_LLID_MSB"                     
#define OLT_1G_MAC_LLID_GOOD_LLID_LSB_NAME					   "OLT_1G_MAC_LLID_GOOD_LLID_LSB"                    
#define OLT_1G_MAC_LLID_GOOD_LLID_MSB_NAME					   "OLT_1G_MAC_LLID_GOOD_LLID_MSB"                    
#define OLT_1G_MAC_LLID_PON_CAST_LLID_LSB_NAME				   "OLT_1G_MAC_LLID_PON_CAST_LLID_LSB"                
#define OLT_1G_MAC_LLID_PON_CAST_LLID_MSB_NAME				   "OLT_1G_MAC_LLID_PON_CAST_LLID_MSB"                
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_LSB_NAME		   "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_LSB"       
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_MSB_NAME		   "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_NO_ERR_MSB"       
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_LSB_NAME   "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_LSB"  
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_MSB_NAME   "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_RECOVERABLE_MSB"  
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_LSB_NAME "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_LSB"
#define OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_MSB_NAME "OLT_1G_MAC_LLID_GMRX_VR1_FEC_CW_UNRECOVERABLE_MSB"
#define OLT_1G_MAC_LLID_IN_ERRORS_LSB_NAME					   "OLT_1G_MAC_LLID_IN_ERRORS_LSB"                    
#define OLT_1G_MAC_LLID_IN_ERRORS_MSB_NAME					   "OLT_1G_MAC_LLID_IN_ERRORS_MSB"                    
#define OLT_1G_MAC_LLID_VR1_DISP_ERR_LSB_NAME				   "OLT_1G_MAC_LLID_VR1_DISP_ERR_LSB"                 
#define OLT_1G_MAC_LLID_VR1_DISP_ERR_MSB_NAME				   "OLT_1G_MAC_LLID_VR1_DISP_ERR_MSB"                 
#define OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_LSB_NAME	   "OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_LSB"     
#define OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_MSB_NAME	   "OLT_1G_MAC_LLID_RX_BAD_FEC_PARITY_LENGTH_MSB"     
#define OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_LSB_NAME			   "OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_LSB"              
#define OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_MSB_NAME			   "OLT_1G_MAC_LLID_RX_FEC_OH_BYTES_MSB"              
#define OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_LSB_NAME	   "OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_LSB"   
#define OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_MSB_NAME	   "OLT_1G_MAC_LLID_VR1_FEC_UNCORRECTED_PACKET_MSB"   

  
  
typedef enum
{

  OLT_1G_MAC_GEN_FIRST_COUNTER,
  OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB = OLT_1G_MAC_GEN_FIRST_COUNTER,
  OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_LSB,
  OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB,
  OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB,
  OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB,
  OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB,
  OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB,
  OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB,
  OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_LSB,
  OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_MSB,
  OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_LSB,
  OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_MSB,
  OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_LSB,
  OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_MSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_LSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_MSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_LSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_MSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_LSB,
  OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_MSB,
  OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_SLD_ERR_TOT_LSB,
  OLT_1G_MAC_GEN_SLD_ERR_TOT_MSB,
  OLT_1G_MAC_GEN_CRC8_ERR_TOT_LSB,
  OLT_1G_MAC_GEN_CRC8_ERR_TOT_MSB,
  OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_LSB,
  OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_MSB,
  OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_LSB,
  OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_MSB,
  OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_LSB,
  OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_MSB,
  OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_LSB,
  OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_MSB,
  OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB,
  OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB,
  OLT_1G_MAC_GEN_IN_ERRORS_TOT_LSB,
  OLT_1G_MAC_GEN_IN_ERRORS_TOT_MSB,
  OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_LSB,
  OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_MSB,
  OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_LSB,
  OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_MSB,
  OLT_1G_MAC_GEN_OUT_OCTETS_TOT_LSB,
  OLT_1G_MAC_GEN_OUT_OCTETS_TOT_MSB,
  OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_LSB,
  OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_MSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_LSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_MSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_LSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_MSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_LSB,
  OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_MSB,
  OLT_1G_MAC_GEN_LAST_COUNTER = OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_MSB,
} OLT_1G_MAC_gen_counter_e;

#define OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB_NAME 		   	  	  "OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB_NAME		   	  	  "OLT_1G_MAC_GEN_VR1_PON_TX_PKT_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_LSB_NAME		   	  	  "OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_MSB_NAME		   	  	  "OLT_1G_MAC_GEN_TX_PKTS64OCTETS_TOT_MSB"	 
#define OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_LSB_NAME   	  	  "OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_MSB_NAME   	  	  "OLT_1G_MAC_GEN_TX_PKTS65TO127OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTS128TO255OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTS256TO511OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_LSB_NAME 	  	  "OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_MSB_NAME 	  	  "OLT_1G_MAC_GEN_TX_PKTS512TO1023OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_LSB_NAME	  	  "OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_MSB_NAME	  	  "OLT_1G_MAC_GEN_TX_PKTS1024TO1518OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_TX_PKTSOVER1518OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_LSB_NAME 	  	  "OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_MSB_NAME 	  	  "OLT_1G_MAC_GEN_VR1_GMII_FCS_DISCARDED_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_LSB_NAME		   	  	  "OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_MSB_NAME		   	  	  "OLT_1G_MAC_GEN_RX_PKTS64OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_LSB_NAME   	  	  "OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_MSB_NAME   	  	  "OLT_1G_MAC_GEN_RX_PKTS65TO127OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTS128TO255OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTS256TO511OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_LSB_NAME 	  	  "OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_MSB_NAME 	  	  "OLT_1G_MAC_GEN_RX_PKTS512TO1023OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_LSB_NAME	  	  "OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_MSB_NAME	  	  "OLT_1G_MAC_GEN_RX_PKTS1024TO1518OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_LSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_MSB_NAME  	  	  "OLT_1G_MAC_GEN_RX_PKTSOVER1518OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB_NAME			   	  	  "OLT_1G_MAC_GEN_BAD_LLID_TOT_LSB"
#define OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB_NAME			   	  	  "OLT_1G_MAC_GEN_BAD_LLID_TOT_MSB"
#define OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB_NAME			   	  	  "OLT_1G_MAC_GEN_GOOD_LLID_TOT_LSB"
#define OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB_NAME					  "OLT_1G_MAC_GEN_GOOD_LLID_TOT_MSB"
#define OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB_NAME				  "OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_LSB"
#define OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB_NAME				  "OLT_1G_MAC_GEN_PON_CAST_LLID_TOT_MSB"
#define OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_LSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_MSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_NO_ERRORS_IN_PACKET_TOT_MSB"
#define OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_LSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_MSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_CORRECTED_PACKET_TOT_MSB"
#define OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_LSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_MSB_NAME	  "OLT_1G_MAC_GEN_VR1_FEC_UNCORRECTED_PACKET_TOT_MSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_LSB_NAME		  "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_LSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_MSB_NAME		  "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_NO_ERR_TOT_MSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_LSB_NAME	  "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_LSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_MSB_NAME	  "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_RECOVERABLE_TOT_MSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_LSB_NAME "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_LSB"
#define OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_MSB_NAME "OLT_1G_MAC_GEN_GMRX_VR1_FEC_CW_UNRECOVERABLE_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB_NAME				  "OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB_NAME				  "OLT_1G_MAC_GEN_TX_UCAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB_NAME			  "OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB_NAME			  "OLT_1G_MAC_GEN_TX_BROADCAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB_NAME			  "OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB_NAME			  "OLT_1G_MAC_GEN_TX_MULTICAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_SLD_ERR_TOT_LSB_NAME						  "OLT_1G_MAC_GEN_SLD_ERR_TOT_LSB"
#define OLT_1G_MAC_GEN_SLD_ERR_TOT_MSB_NAME						  "OLT_1G_MAC_GEN_SLD_ERR_TOT_MSB"
#define OLT_1G_MAC_GEN_CRC8_ERR_TOT_LSB_NAME					  "OLT_1G_MAC_GEN_CRC8_ERR_TOT_LSB"
#define OLT_1G_MAC_GEN_CRC8_ERR_TOT_MSB_NAME					  "OLT_1G_MAC_GEN_CRC8_ERR_TOT_MSB"
#define OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_LSB_NAME				  "OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_LSB"
#define OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_MSB_NAME			   	  "OLT_1G_MAC_GEN_TX_FEC_OH_BYTES_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_LSB_NAME			  "OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_MSB_NAME			  "OLT_1G_MAC_GEN_RX_OVERSIZED_PACKET_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_LSB_NAME		  "OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_MSB_NAME		  "OLT_1G_MAC_GEN_RX_UNDERSIZED_PACKET_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_LSB_NAME	  "OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_MSB_NAME	  "OLT_1G_MAC_GEN_RX_BAD_FEC_PARITY_LENGTH_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB_NAME				  "OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB_NAME				  "OLT_1G_MAC_GEN_RX_UCAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB_NAME			  "OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB_NAME			  "OLT_1G_MAC_GEN_RX_BROADCAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB_NAME			  "OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB_NAME			  "OLT_1G_MAC_GEN_RX_MULTICAST_IND_TOT_MSB"
#define OLT_1G_MAC_GEN_IN_ERRORS_TOT_LSB_NAME				      "OLT_1G_MAC_GEN_IN_ERRORS_TOT_LSB"
#define OLT_1G_MAC_GEN_IN_ERRORS_TOT_MSB_NAME				      "OLT_1G_MAC_GEN_IN_ERRORS_TOT_MSB"
#define OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_LSB_NAME				  "OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_LSB"
#define OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_MSB_NAME				  "OLT_1G_MAC_GEN_VR1_DISP_ERR_TOT_MSB"
#define OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_LSB_NAME			      "OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_LSB"
#define OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_MSB_NAME			      "OLT_1G_MAC_GEN_IN_SYMBOL_ERR_TOT_MSB"
#define OLT_1G_MAC_GEN_OUT_OCTETS_TOT_LSB_NAME				      "OLT_1G_MAC_GEN_OUT_OCTETS_TOT_LSB"
#define OLT_1G_MAC_GEN_OUT_OCTETS_TOT_MSB_NAME				      "OLT_1G_MAC_GEN_OUT_OCTETS_TOT_MSB"
#define OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_LSB_NAME			      "OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_LSB"
#define OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_MSB_NAME			      "OLT_1G_MAC_GEN_RX_FEC_OH_BYTES_TOT_MSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_LSB_NAME     "OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_LSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_MSB_NAME     "OLT_1G_MAC_GEN_FORCE_PKT_END_LENGTH_OVRS_TOT_MSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_LSB_NAME	      "OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_LSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_MSB_NAME	      "OLT_1G_MAC_GEN_FORCE_PKT_END_OUT_WIN_TOT_MSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_LSB_NAME     "OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_LSB"
#define OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_MSB_NAME     "OLT_1G_MAC_GEN_FORCE_PKT_END_PARITY_OVRS_TOT_MSB"
																  
 
typedef enum
{
	OLT_10G_XMT_LLID_FIRST_COUNTER,
	OLT_10G_XMT_LLID_PKT_TX_OK_LSB = OLT_10G_XMT_LLID_FIRST_COUNTER,
	OLT_10G_XMT_LLID_PKT_TX_OK_MSB,
	OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_LSB,
	OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_MSB,
	OLT_10G_XMT_LLID_DA_UCAST_LSB,
	OLT_10G_XMT_LLID_DA_UCAST_MSB,
	OLT_10G_XMT_LLID_DA_MCAST_LSB,
	OLT_10G_XMT_LLID_DA_MCAST_MSB,
	OLT_10G_XMT_LLID_DA_BCAST_LSB,
	OLT_10G_XMT_LLID_DA_BCAST_MSB,
	OLT_10G_XMT_LLID_PKTS64OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS64OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTS65TO127OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS65TO127OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTS128TO255OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS128TO255OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTS256TO511OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS256TO511OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_MSB,
	OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_LSB,
	OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_MSB,
	OLT_10G_XMT_LLID_OCTETS_TX_OK_LSB,
	OLT_10G_XMT_LLID_OCTETS_TX_OK_MSB,
	OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_LSB,
	OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_MSB,
	OLT_10G_XMT_LLID_LAST_COUNTER = OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_MSB
} OLT_10G_XMT_llid_stat_counter_e;


#define OLT_10G_XMT_LLID_PKT_TX_OK_LSB_NAME			   "OLT_10G_XMT_LLID_PKT_TX_OK_LSB"           	  
#define OLT_10G_XMT_LLID_PKT_TX_OK_MSB_NAME			   "OLT_10G_XMT_LLID_PKT_TX_OK_MSB"           	  
#define OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_LSB_NAME	   "OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_LSB"      		  
#define OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_MSB_NAME	   "OLT_10G_XMT_LLID_PKT_TX_BAD_FCS_MSB"      		  
#define OLT_10G_XMT_LLID_DA_UCAST_LSB_NAME			   "OLT_10G_XMT_LLID_DA_UCAST_LSB"            
#define OLT_10G_XMT_LLID_DA_UCAST_MSB_NAME			   "OLT_10G_XMT_LLID_DA_UCAST_MSB"            	  
#define OLT_10G_XMT_LLID_DA_MCAST_LSB_NAME			   "OLT_10G_XMT_LLID_DA_MCAST_LSB"            	  
#define OLT_10G_XMT_LLID_DA_MCAST_MSB_NAME			   "OLT_10G_XMT_LLID_DA_MCAST_MSB"            	  
#define OLT_10G_XMT_LLID_DA_BCAST_LSB_NAME			   "OLT_10G_XMT_LLID_DA_BCAST_LSB"            	  
#define OLT_10G_XMT_LLID_DA_BCAST_MSB_NAME			   "OLT_10G_XMT_LLID_DA_BCAST_MSB"            	  
#define OLT_10G_XMT_LLID_PKTS64OCTETS_LSB_NAME		   "OLT_10G_XMT_LLID_PKTS64OCTETS_LSB"        	  
#define OLT_10G_XMT_LLID_PKTS64OCTETS_MSB_NAME		   "OLT_10G_XMT_LLID_PKTS64OCTETS_MSB"        	  
#define OLT_10G_XMT_LLID_PKTS65TO127OCTETS_LSB_NAME	   "OLT_10G_XMT_LLID_PKTS65TO127OCTETS_LSB"   	  
#define OLT_10G_XMT_LLID_PKTS65TO127OCTETS_MSB_NAME	   "OLT_10G_XMT_LLID_PKTS65TO127OCTETS_MSB"   	  
#define OLT_10G_XMT_LLID_PKTS128TO255OCTETS_LSB_NAME   "OLT_10G_XMT_LLID_PKTS128TO255OCTETS_LSB"  		  
#define OLT_10G_XMT_LLID_PKTS128TO255OCTETS_MSB_NAME   "OLT_10G_XMT_LLID_PKTS128TO255OCTETS_MSB"  		  
#define OLT_10G_XMT_LLID_PKTS256TO511OCTETS_LSB_NAME   "OLT_10G_XMT_LLID_PKTS256TO511OCTETS_LSB"  		  
#define OLT_10G_XMT_LLID_PKTS256TO511OCTETS_MSB_NAME   "OLT_10G_XMT_LLID_PKTS256TO511OCTETS_MSB"  		  
#define OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_LSB_NAME  "OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_LSB" 	  
#define OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_MSB_NAME  "OLT_10G_XMT_LLID_PKTS512TO1023OCTETS_MSB" 	  
#define OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_LSB_NAME "OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_LSB"	  
#define OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_MSB_NAME "OLT_10G_XMT_LLID_PKTS1024TO1518OCTETS_MSB"	  
#define OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_LSB_NAME   "OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_LSB"  		  
#define OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_MSB_NAME   "OLT_10G_XMT_LLID_PKTSOVER1518OCTETS_MSB"  		  
#define OLT_10G_XMT_LLID_OCTETS_TX_OK_LSB_NAME		   "OLT_10G_XMT_LLID_OCTETS_TX_OK_LSB"        	  
#define OLT_10G_XMT_LLID_OCTETS_TX_OK_MSB_NAME		   "OLT_10G_XMT_LLID_OCTETS_TX_OK_MSB"        	  
#define OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_LSB_NAME	   "OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_LSB"   	  
#define OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_MSB_NAME	   "OLT_10G_XMT_LLID_OCTETS_TX_BAD_FCS_MSB"   	  

	
typedef enum
{	
	OLT_10G_XMT_GEN_FIRST_COUNTER,
	OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB = OLT_10G_XMT_GEN_FIRST_COUNTER,
	OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB,
	OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB,
	OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB,
	OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB,
	OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB,
	OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB,
	OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB,
	OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB,
	OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB,
	OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB,
	OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB,
	OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_LSB,
	OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_MSB,
	OLT_10G_XMT_GEN_LAST_COUNTER = OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_MSB,

} OLT_10G_XMT_gen_stat_counter_e;


#define OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB_NAME			"OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_LSB"           
#define OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB_NAME			"OLT_10G_XMT_GEN_TOTAL_PKT_TX_OK_MSB"           
#define OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB_NAME		"OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_LSB"      
#define OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB_NAME		"OLT_10G_XMT_GEN_TOTAL_PKT_TX_BAD_FCS_MSB"      
#define OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB_NAME			"OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_LSB"        
#define OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB_NAME			"OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_OK_MSB"        
#define OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_LSB"   
#define OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_OCTETS_TX_BAD_FCS_MSB"   
#define OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_UCAST_LSB"            
#define OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_UCAST_MSB"            
#define OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_MCAST_LSB"            
#define OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_MCAST_MSB"            
#define OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_BCAST_LSB"            
#define OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB_NAME				"OLT_10G_XMT_GEN_TOTAL_DA_BCAST_MSB"            
#define OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_LSB_NAME			"OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_LSB"        
#define OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_MSB_NAME			"OLT_10G_XMT_GEN_TOTAL_PKTS64OCTETS_MSB"        
#define OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_LSB"   
#define OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS65TO127OCTETS_MSB"   
#define OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_LSB"  
#define OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS128TO255OCTETS_MSB"  
#define OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_LSB"  
#define OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS256TO511OCTETS_MSB"  
#define OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_LSB" 
#define OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS512TO1023OCTETS_MSB" 
#define OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB"
#define OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB"
#define OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_LSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_LSB"  
#define OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_MSB_NAME	"OLT_10G_XMT_GEN_TOTAL_PKTSOVER1518OCTETS_MSB"  

typedef enum
{

	OLT_10G_XMR_LLID_FIRST_COUNTER,
	OLT_10G_XMR_LLID_SYMBOL_ERR_LSB = OLT_10G_XMR_LLID_FIRST_COUNTER,
	OLT_10G_XMR_LLID_SYMBOL_ERR_MSB,
	OLT_10G_XMR_LLID_GOOD_PKT_LSB,
	OLT_10G_XMR_LLID_GOOD_PKT_MSB,
	OLT_10G_XMR_LLID_OVERSIZE_LSB,
	OLT_10G_XMR_LLID_OVERSIZE_MSB,
	OLT_10G_XMR_LLID_BAD_FCS_LSB,
	OLT_10G_XMR_LLID_BAD_FCS_MSB,
	OLT_10G_XMR_LLID_FIFO_ERR_LSB,
	OLT_10G_XMR_LLID_FIFO_ERR_MSB,
	OLT_10G_XMR_LLID_DA_UCAST_LSB,
	OLT_10G_XMR_LLID_DA_UCAST_MSB,
	OLT_10G_XMR_LLID_DA_MCAST_LSB,
	OLT_10G_XMR_LLID_DA_MCAST_MSB,
	OLT_10G_XMR_LLID_DA_BCAST_LSB,
	OLT_10G_XMR_LLID_DA_BCAST_MSB,
	OLT_10G_XMR_LLID_PKTS64OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS64OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTS65TO127OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS65TO127OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTS128TO255OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS128TO255OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTS256TO511OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS256TO511OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_MSB,
	OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_LSB,
	OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_MSB,
	OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_LSB,
	OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_MSB,
	OLT_10G_XMR_LLID_PCS_RXERR_LSB,
	OLT_10G_XMR_LLID_PCS_RXERR_MSB,
	OLT_10G_XMR_LLID_BAD_SLD_LSB,
	OLT_10G_XMR_LLID_BAD_SLD_MSB,
	OLT_10G_XMR_LLID_BAD_CRC8_LSB,
	OLT_10G_XMR_LLID_BAD_CRC8_MSB,
	OLT_10G_XMR_LLID_LLID_UNWANTED_LSB,
	OLT_10G_XMR_LLID_LLID_UNWANTED_MSB,
	OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_LSB,
	OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_MSB,
	OLT_10G_XMR_LLID_LLID_ACCEPTED_LSB,
	OLT_10G_XMR_LLID_LLID_ACCEPTED_MSB,
	OLT_10G_XMR_LLID_UNDERSIZE_LSB,
	OLT_10G_XMR_LLID_UNDERSIZE_MSB,
	OLT_10G_XMR_LLID_BAD_PKT_LSB,
	OLT_10G_XMR_LLID_BAD_PKT_MSB,
	OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_LSB,
	OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_MSB,
	OLT_10G_XMR_LLID_LAST_COUNTER = OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_MSB,

} OLT_10G_XMR_llid_stat_counter_e;

#define OLT_10G_XMR_LLID_SYMBOL_ERR_LSB_NAME			   "OLT_10G_XMR_LLID_SYMBOL_ERR_LSB"             
#define OLT_10G_XMR_LLID_SYMBOL_ERR_MSB_NAME			   "OLT_10G_XMR_LLID_SYMBOL_ERR_MSB"             
#define OLT_10G_XMR_LLID_GOOD_PKT_LSB_NAME				   "OLT_10G_XMR_LLID_GOOD_PKT_LSB"               
#define OLT_10G_XMR_LLID_GOOD_PKT_MSB_NAME				   "OLT_10G_XMR_LLID_GOOD_PKT_MSB"               
#define OLT_10G_XMR_LLID_OVERSIZE_LSB_NAME				   "OLT_10G_XMR_LLID_OVERSIZE_LSB"               
#define OLT_10G_XMR_LLID_OVERSIZE_MSB_NAME				   "OLT_10G_XMR_LLID_OVERSIZE_MSB"               
#define OLT_10G_XMR_LLID_BAD_FCS_LSB_NAME				   "OLT_10G_XMR_LLID_BAD_FCS_LSB"                
#define OLT_10G_XMR_LLID_BAD_FCS_MSB_NAME				   "OLT_10G_XMR_LLID_BAD_FCS_MSB"                
#define OLT_10G_XMR_LLID_FIFO_ERR_LSB_NAME				   "OLT_10G_XMR_LLID_FIFO_ERR_LSB"               
#define OLT_10G_XMR_LLID_FIFO_ERR_MSB_NAME				   "OLT_10G_XMR_LLID_FIFO_ERR_MSB"               
#define OLT_10G_XMR_LLID_DA_UCAST_LSB_NAME				   "OLT_10G_XMR_LLID_DA_UCAST_LSB"               
#define OLT_10G_XMR_LLID_DA_UCAST_MSB_NAME				   "OLT_10G_XMR_LLID_DA_UCAST_MSB"               
#define OLT_10G_XMR_LLID_DA_MCAST_LSB_NAME				   "OLT_10G_XMR_LLID_DA_MCAST_LSB"               
#define OLT_10G_XMR_LLID_DA_MCAST_MSB_NAME				   "OLT_10G_XMR_LLID_DA_MCAST_MSB"               
#define OLT_10G_XMR_LLID_DA_BCAST_LSB_NAME				   "OLT_10G_XMR_LLID_DA_BCAST_LSB"               
#define OLT_10G_XMR_LLID_DA_BCAST_MSB_NAME				   "OLT_10G_XMR_LLID_DA_BCAST_MSB"               
#define OLT_10G_XMR_LLID_PKTS64OCTETS_LSB_NAME			   "OLT_10G_XMR_LLID_PKTS64OCTETS_LSB"           
#define OLT_10G_XMR_LLID_PKTS64OCTETS_MSB_NAME			   "OLT_10G_XMR_LLID_PKTS64OCTETS_MSB"           
#define OLT_10G_XMR_LLID_PKTS65TO127OCTETS_LSB_NAME		   "OLT_10G_XMR_LLID_PKTS65TO127OCTETS_LSB"      
#define OLT_10G_XMR_LLID_PKTS65TO127OCTETS_MSB_NAME		   "OLT_10G_XMR_LLID_PKTS65TO127OCTETS_MSB"      
#define OLT_10G_XMR_LLID_PKTS128TO255OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_PKTS128TO255OCTETS_LSB"     
#define OLT_10G_XMR_LLID_PKTS128TO255OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_PKTS128TO255OCTETS_MSB"     
#define OLT_10G_XMR_LLID_PKTS256TO511OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_PKTS256TO511OCTETS_LSB"     
#define OLT_10G_XMR_LLID_PKTS256TO511OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_PKTS256TO511OCTETS_MSB"     
#define OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_LSB"    
#define OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_PKTS512TO1023OCTETS_MSB"    
#define OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_LSB"   
#define OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_PKTS1024TO1518OCTETS_MSB"   
#define OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_LSB"     
#define OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_PKTSOVER1518OCTETS_MSB"     
#define OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_LSB_NAME	   "OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_LSB"     
#define OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_MSB_NAME	   "OLT_10G_XMR_LLID_GOOD_PKT_RX_OCTETS_MSB"     
#define OLT_10G_XMR_LLID_PCS_RXERR_LSB_NAME				   "OLT_10G_XMR_LLID_PCS_RXERR_LSB"              
#define OLT_10G_XMR_LLID_PCS_RXERR_MSB_NAME				   "OLT_10G_XMR_LLID_PCS_RXERR_MSB"              
#define OLT_10G_XMR_LLID_BAD_SLD_LSB_NAME				   "OLT_10G_XMR_LLID_BAD_SLD_LSB"                
#define OLT_10G_XMR_LLID_BAD_SLD_MSB_NAME				   "OLT_10G_XMR_LLID_BAD_SLD_MSB"                
#define OLT_10G_XMR_LLID_BAD_CRC8_LSB_NAME				   "OLT_10G_XMR_LLID_BAD_CRC8_LSB"               
#define OLT_10G_XMR_LLID_BAD_CRC8_MSB_NAME				   "OLT_10G_XMR_LLID_BAD_CRC8_MSB"               
#define OLT_10G_XMR_LLID_LLID_UNWANTED_LSB_NAME			   "OLT_10G_XMR_LLID_LLID_UNWANTED_LSB"          
#define OLT_10G_XMR_LLID_LLID_UNWANTED_MSB_NAME			   "OLT_10G_XMR_LLID_LLID_UNWANTED_MSB"          
#define OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_LSB_NAME  "OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_LSB"
#define OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_MSB_NAME  "OLT_10G_XMR_LLID_LLID_OUTSIDE_ACTIVE_WIN_MSB"
#define OLT_10G_XMR_LLID_LLID_ACCEPTED_LSB_NAME			   "OLT_10G_XMR_LLID_LLID_ACCEPTED_LSB"          
#define OLT_10G_XMR_LLID_LLID_ACCEPTED_MSB_NAME			   "OLT_10G_XMR_LLID_LLID_ACCEPTED_MSB"          
#define OLT_10G_XMR_LLID_UNDERSIZE_LSB_NAME				   "OLT_10G_XMR_LLID_UNDERSIZE_LSB"              
#define OLT_10G_XMR_LLID_UNDERSIZE_MSB_NAME				   "OLT_10G_XMR_LLID_UNDERSIZE_MSB"              
#define OLT_10G_XMR_LLID_BAD_PKT_LSB_NAME				   "OLT_10G_XMR_LLID_BAD_PKT_LSB"                
#define OLT_10G_XMR_LLID_BAD_PKT_MSB_NAME				   "OLT_10G_XMR_LLID_BAD_PKT_MSB"                
#define OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_LSB_NAME		   "OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_LSB"      
#define OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_MSB_NAME		   "OLT_10G_XMR_LLID_BAD_PKT_RX_OCTETS_MSB"      
 
	
typedef enum
{	
	OLT_10G_XMR_GEN_FIRST_COUNTER,
	OLT_10G_XMR_GEN_PCS_RXERR_C_LSB = OLT_10G_XMR_GEN_FIRST_COUNTER,
	OLT_10G_XMR_GEN_PCS_RXERR_C_MSB,
	OLT_10G_XMR_GEN_PCS_RXERR_S_LSB,
	OLT_10G_XMR_GEN_PCS_RXERR_S_MSB,
	OLT_10G_XMR_GEN_PCS_RXERR_T_LSB,
	OLT_10G_XMR_GEN_PCS_RXERR_T_MSB,
	OLT_10G_XMR_GEN_PCS_RXERR_D_LSB,
	OLT_10G_XMR_GEN_PCS_RXERR_D_MSB,
	OLT_10G_XMR_GEN_PCS_RXERR_E_LSB,
	OLT_10G_XMR_GEN_PCS_RXERR_E_MSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB,
	OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB,
	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB,
	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB,
	OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB,
	OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB,
	OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB,
	OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB,
	OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB,
	OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB,
	OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB,
	OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB,
	OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB,
	OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_LSB,
	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_MSB,
	OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_LSB,
	OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_MSB,
	OLT_10G_XMR_GEN_DROP_SOP_MINIPG_LSB,
	OLT_10G_XMR_GEN_DROP_SOP_MINIPG_MSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_SLD_LSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_SLD_MSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_LSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_MSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_LSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_MSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_LSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_MSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_LSB,
	OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_MSB,
	OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_LSB,
	OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_MSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_LSB,
	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_MSB,
	OLT_10G_XMR_GEN_LAST_COUNTER = OLT_10G_XMR_GEN_TOTAL_BAD_PKT_MSB,

} OLT_10G_XMR_gen_stat_counter_e;
 

#define	OLT_10G_XMR_GEN_PCS_RXERR_C_LSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_C_LSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_C_MSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_C_MSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_S_LSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_S_LSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_S_MSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_S_MSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_T_LSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_T_LSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_T_MSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_T_MSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_D_LSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_D_LSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_D_MSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_D_MSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_E_LSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_E_LSB"                  
#define	OLT_10G_XMR_GEN_PCS_RXERR_E_MSB_NAME			   		"OLT_10G_XMR_GEN_PCS_RXERR_E_MSB"                  
#define	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB_NAME   		"OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_LSB"      
#define	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB_NAME   		"OLT_10G_XMR_GEN_TOTAL_BAD_PKT_RX_OCTETS_MSB"      
#define	OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_LSB"             
#define	OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_SYMBOL_ERR_MSB"             
#define	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_OVERSIZE_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_OVERSIZE_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_FCS_LSB"                
#define	OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_FCS_MSB"                
#define	OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_FIFO_ERR_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_UCAST_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_UCAST_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_MCAST_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_MCAST_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_BCAST_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_DA_BCAST_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_LSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_LSB"           
#define	OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_MSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_PKTS64OCTETS_MSB"           
#define	OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_LSB_NAME   		"OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_LSB"      
#define	OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_MSB_NAME   		"OLT_10G_XMR_GEN_TOTAL_PKTS65TO127OCTETS_MSB"      
#define	OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_LSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_LSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_MSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTS128TO255OCTETS_MSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_LSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_LSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_MSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTS256TO511OCTETS_MSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_LSB_NAME 		"OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_LSB"    
#define	OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_MSB_NAME 		"OLT_10G_XMR_GEN_TOTAL_PKTS512TO1023OCTETS_MSB"    
#define	OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB_NAME		"OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_LSB"   
#define	OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB_NAME		"OLT_10G_XMR_GEN_TOTAL_PKTS1024TO1518OCTETS_MSB"   
#define	OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_LSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_LSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_MSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_PKTSOVER1518OCTETS_MSB"     
#define	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_LSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_LSB"     
#define	OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_MSB_NAME  		"OLT_10G_XMR_GEN_TOTAL_GOOD_PKT_RX_OCTETS_MSB"     
#define	OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_LSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_LSB"              
#define	OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_MSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_PCS_RXERR_MSB"              
#define	OLT_10G_XMR_GEN_DROP_SOP_MINIPG_LSB_NAME		   		"OLT_10G_XMR_GEN_DROP_SOP_MINIPG_LSB"              
#define	OLT_10G_XMR_GEN_DROP_SOP_MINIPG_MSB_NAME		   		"OLT_10G_XMR_GEN_DROP_SOP_MINIPG_MSB"              
#define	OLT_10G_XMR_GEN_TOTAL_BAD_SLD_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_SLD_LSB"                
#define	OLT_10G_XMR_GEN_TOTAL_BAD_SLD_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_SLD_MSB"                
#define	OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_LSB"               
#define	OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_CRC8_MSB"               
#define	OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_LSB_NAME	   		"OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_LSB"          
#define	OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_MSB_NAME	   		"OLT_10G_XMR_GEN_TOTAL_LLID_UNWANTED_MSB"          
#define	OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_LSB_NAME	"OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_LSB"
#define	OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_MSB_NAME	"OLT_10G_XMR_GEN_TOTAL_LLID_OUTSIDE_ACTIVE_WIN_MSB"
#define	OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_LSB_NAME	   		"OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_LSB"          
#define	OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_MSB_NAME	   		"OLT_10G_XMR_GEN_TOTAL_LLID_ACCEPTED_MSB"          
#define	OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_LSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_LSB"              
#define	OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_MSB_NAME		   		"OLT_10G_XMR_GEN_TOTAL_UNDERSIZE_MSB"              
#define	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_LSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_PKT_LSB"                
#define	OLT_10G_XMR_GEN_TOTAL_BAD_PKT_MSB_NAME			   		"OLT_10G_XMR_GEN_TOTAL_BAD_PKT_MSB"                

typedef enum 
{
	GW10G_QUEUE_STAT_FIRST_COUNTER,
	GW10G_QUEUE_STAT_IBB_COUNTER = GW10G_QUEUE_STAT_FIRST_COUNTER,
	GW10G_QUEUE_STAT_MBM_COUNTER,
	GW10G_QUEUE_STAT_BAB_OCCUPANCY_COUNTER,
	GW10G_QUEUE_STAT_LAST_COUNTER = GW10G_QUEUE_STAT_BAB_OCCUPANCY_COUNTER
} GW10G_QUEUE_stat_counter_e;

#define	IBB_COUNTER_NAME			   					"OLT_QUEUE_IBB_COUNTER"                  
#define	MBM_COUNTER_NAME			   					"OLT_QUEUE_MBM_COUNTER"                  
#define	BAB_OCCUPANCY_COUNTER_NAME				 		"OLT_QUEUE_BAB_OCCUPANCY_COUNTER"          
typedef enum{
    OLT_STAT_OAM_FIRST_COUNTER,
	OLT_STAT_STANDARD_OAM_STATUS_ID = OLT_STAT_OAM_FIRST_COUNTER,
    OLT_STAT_STANDARD_OAM_STATUS_ADMIN_STATE,
    OLT_STAT_STANDARD_OAM_STATUS_DISCOVERY_STATE,
    OLT_STAT_STANDARD_OAM_STATUS_LOCAL_FLAGS,
    OLT_STAT_STANDARD_OAM_STATUS_REMOTE_FLAGS,
    OLT_STAT_STANDARD_OAM_STATUS_MODE,
    OLT_STAT_STANDARD_OAM_STATUS_LOCAL_PDU_CONFIG,
    OLT_STAT_STANDARD_OAM_STATUS_LOCAL_REVISION,
    OLT_STAT_STANDARD_OAM_STATUS_LOCAL_CONFIG,
    
	OLT_STAT_STANDARD_OAM_PEER_STATUS_DISCOVERY_STATE,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART1,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART2,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_OUI,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_VENDOR_SPECIFIC,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_CONFIG,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_PDU_CONFIG,
	OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_REVISION,
	
	OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_LOCAL_STATE,
	OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_REMOTE_STATE,
	
	OLT_STAT_STANDARD_OAM_STATISTICS_INFO_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_INFO_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_TX,
	OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPRTED_CODES_TX,
    OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPORT_CODES_RX,
	OLT_STAT_STANDARD_OAM_STATISTICS_FRAMES_LOST_DUE_TO_OAM_ERR,
	
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_ID,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_MODE,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_LINK_ID,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART1,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART2,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_REG_STATE,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_TX_ELAPSED,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_RX_ELAPSED,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_RTT,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAXIMUM_PEND_GRANT,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_ADMIN_STATE,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_ON_TIME,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_OFF_TIME,
    OLT_STAT_STANDARD_OAM_MPCP_STATUS_SYNCH_TIME,
    
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_TX,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_RX,
    OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_WIN_SENT,
    OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_TIMEOUT,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_REQ,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_REQ,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_ACK,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_ACK,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REPORT,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REPORT,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_GATE,
	OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_GATE,
    OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG,
    OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG,
    
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_ID,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_TYPE,
	
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_SLD_ERRORS,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_CRC_ERRORS,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_BAD_LLID,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_GOOD_LLID,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_ONU_PON_CAST_LLID,
	OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_OLT_PON_CAST_LLID,
	
	OLT_STAT_STANDARD_OAM_MAU_STATUS_PCS_CODING_VIOLATION,
	OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_ABILITY,
	OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_MODE,
	OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_CORRECTED_BLOCKS,
	OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_UNCORRECTABLE_BLOCKS,
	
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_0,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_1,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_2,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_3,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_4,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_5,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_6,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_7,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_0,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_1,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_2,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_3,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_4,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_5,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_6,
	OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_7,	
	OLT_STAT_OAM_LAST_COUNTER = OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_UNCORRECTABLE_BLOCKS,
} OLT_OAM_stat_counter_e;

#define OLT_STAT_STANDARD_OAM_STATUS_ID_NAME  					  "OLT_STAT_STANDARD_OAM_STATUS_ID"
#define OLT_STAT_STANDARD_OAM_STATUS_ADMIN_STATE_NAME			  "OLT_STAT_STANDARD_OAM_STATUS_ADMIN_STATE"
#define OLT_STAT_STANDARD_OAM_STATUS_DISCOVERY_STATE_NAME		  "OLT_STAT_STANDARD_OAM_STATUS_DISCOVERY_STATE"
#define OLT_STAT_STANDARD_OAM_STATUS_LOCAL_FLAGS_NAME			  "OLT_STAT_STANDARD_OAM_STATUS_LOCAL_FLAGS"
#define OLT_STAT_STANDARD_OAM_STATUS_REMOTE_FLAGS_NAME		      "OLT_STAT_STANDARD_OAM_STATUS_REMOTE_FLAGS"
#define OLT_STAT_STANDARD_OAM_STATUS_MODE_NAME 					  "OLT_STAT_STANDARD_OAM_STATUS_MODE"
#define OLT_STAT_STANDARD_OAM_STATUS_LOCAL_PDU_CONFIG_NAME		  "OLT_STAT_STANDARD_OAM_STATUS_LOCAL_PDU_CONFIG"
#define OLT_STAT_STANDARD_OAM_STATUS_LOCAL_REVISION_NAME		  "OLT_STAT_STANDARD_OAM_STATUS_LOCAL_REVISION"
#define OLT_STAT_STANDARD_OAM_STATUS_LOCAL_CONFIG_NAME			  "OLT_STAT_STANDARD_OAM_STATUS_LOCAL_CONFIG"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_DISCOVERY_STATE_NAME	  "OLT_STAT_STANDARD_OAM_PEER_STATUS_DISCOVERY_STATE"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART1_NAME	  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART1"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART2_NAME   "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_MAC_ADDR_PART2"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_OUI_NAME			  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_OUI"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_VENDOR_SPECIFIC_NAME  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_VENDOR_SPECIFIC"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_CONFIG_NAME			  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_CONFIG"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_PDU_CONFIG_NAME		  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_PDU_CONFIG"
#define OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_REVISION_NAME		  "OLT_STAT_STANDARD_OAM_PEER_STATUS_REMOTE_REVISION"
#define OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_LOCAL_STATE_NAME			  "OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_LOCAL_STATE"
#define OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_REMOTE_STATE_NAME			  "OLT_STAT_STANDARD_OAM_LOOPBACK_STATUS_REMOTE_STATE"
#define OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPRTED_CODES_TX_NAME     "OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPRTED_CODES_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_INFO_TX_NAME                 "OLT_STAT_STANDARD_OAM_STATISTICS_INFO_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_INFO_RX_NAME                 "OLT_STAT_STANDARD_OAM_STATISTICS_INFO_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_RX_NAME    "OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_RX_NAME "OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_TX_NAME        "OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_RX_NAME        "OLT_STAT_STANDARD_OAM_STATISTICS_LOOPBACK_CTRL_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_TX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_RX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_REQ_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_TX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_RX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_VARIABLE_RES_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_TX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_RX_NAME         "OLT_STAT_STANDARD_OAM_STATISTICS_ORG_SPECIFIC_RX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_FRAMES_LOST_DUE_TO_OAM_ERR_NAME  "OLT_STAT_STANDARD_OAM_STATISTICS_FRAMES_LOST_DUE_TO_OAM_ERR"
#define OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_TX_NAME	  "OLT_STAT_STANDARD_OAM_STATISTICS_UNIQUE_EVENT_NOTE_TX"
#define OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_TX_NAME "OLT_STAT_STANDARD_OAM_STATISTICS_DUPLICATE_EVENT_NOTE_TX"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_WIN_SENT_NAME     "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_WIN_SENT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_TIMEOUT_NAME      "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_DISCOVERY_TIMEOUT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_TX_NAME            "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_TX"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_RX_NAME            "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_MAC_CTRL_RX"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_NAME                 "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_ACK_NAME             "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_ACK"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_REQ_NAME             "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_REQ"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG_NAME     			  "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REG"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_ACK_NAME      		  "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_ACK"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_REQ_NAME             "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REG_REQ"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REPORT_NAME              "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_REPORT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REPORT_NAME              "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_REPORT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_GATE_NAME                "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_TX_GATE"
#define OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_GATE_NAME                "OLT_STAT_STANDARD_OAM_MPCP_STATISTICS_RX_GATE"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_ID_NAME     			      "OLT_STAT_STANDARD_OAM_MPCP_STATUS_ID"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_MODE_NAME      		  	  "OLT_STAT_STANDARD_OAM_MPCP_STATUS_MODE"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_LINK_ID_NAME             	  "OLT_STAT_STANDARD_OAM_MPCP_STATUS_LINK_ID"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_REG_STATE_NAME              "OLT_STAT_STANDARD_OAM_MPCP_STATUS_REG_STATE"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_TX_ELAPSED_NAME             "OLT_STAT_STANDARD_OAM_MPCP_STATUS_TX_ELAPSED"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_RX_ELAPSED_NAME             "OLT_STAT_STANDARD_OAM_MPCP_STATUS_RX_ELAPSED"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_RTT_NAME     			      "OLT_STAT_STANDARD_OAM_MPCP_STATUS_RTT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAXIMUM_PEND_GRANT_NAME     "OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAXIMUM_PEND_GRANT"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_ADMIN_STATE_NAME            "OLT_STAT_STANDARD_OAM_MPCP_STATUS_ADMIN_STATE"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART1_NAME  "OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART1"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART2_NAME  "OLT_STAT_STANDARD_OAM_MPCP_STATUS_MAC_ADDR_PART2"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_SYNCH_TIME_NAME     "OLT_STAT_STANDARD_OAM_MPCP_STATUS_SYNCH_TIME"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_ID_NAME            "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_ID"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_TYPE_NAME          "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATUS_TYPE"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_SLD_ERRORS_NAME             	  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_SLD_ERRORS"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_CRC_ERRORS_NAME     			  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_CRC_ERRORS"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_BAD_LLID_NAME     				  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_BAD_LLID"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_GOOD_LLID_NAME            		  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_GOOD_LLID"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_ONU_PON_CAST_LLID_NAME  		  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_ONU_PON_CAST_LLID"
#define OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_OLT_PON_CAST_LLID_NAME  		  "OLT_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS_OLT_PON_CAST_LLID"
#define OLT_STAT_STANDARD_OAM_MAU_STATUS_PCS_CODING_VIOLATION_NAME     	  "OLT_STAT_STANDARD_OAM_MAU_STATUS_PCS_CODING_VIOLATION"
#define OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_MODE_NAME            		  "OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_MODE"
#define OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_CORRECTED_BLOCKS_NAME  	  "OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_CORRECTED_BLOCKS"
#define OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_UNCORRECTABLE_BLOCKS_NAME    "OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_UNCORRECTABLE_BLOCKS"
#define OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_ABILITY_NAME            	  "OLT_STAT_STANDARD_OAM_MAU_STATUS_FEC_ABILITY"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_0_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_0"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_1_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_1"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_2_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_2"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_3_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_3"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_4_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_4"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_5_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_5"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_6_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_6"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_7_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_PON_OK_7"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_0_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_0"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_1_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_1"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_2_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_2"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_3_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_3"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_4_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_4"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_5_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_5"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_6_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_6"
#define OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_7_NAME			  "OLT_STAT_RECEIVED_FRAME_BY_CPU_PER_PRIORITY_CNI_OK_7"
#define OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPORT_CODES_RX_NAME      "OLT_STAT_STANDARD_OAM_STATISTICS_UNSUPPORT_CODES_RX"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_ON_TIME_NAME "OLT_STAT_STANDARD_OAM_MPCP_STATUS_ON_TIME"
#define OLT_STAT_STANDARD_OAM_MPCP_STATUS_OFF_TIME_NAME "OLT_STAT_STANDARD_OAM_MPCP_STATUS_OFF_TIME"


typedef enum{
    OLT_STAT_MPCP_FIRST_COUNTER,
    OLT_STAT_MPCP_LAST_COUNTER = OLT_STAT_MPCP_FIRST_COUNTER,
} OLT_MPCP_stat_counter_e;


typedef enum{
    OLT_STAT_LLID_GENERAL_FIRST_COUNTER,
    OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP = OLT_STAT_LLID_GENERAL_FIRST_COUNTER,
    OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX,
    OLT_STAT_LLID_GENERAL_LAST_COUNTER = OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX,
} OLT_LLID_GENERAL_stat_counter_e;

#define OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP_NAME  "OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_DROP"
#define OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX_NAME  "OLT_STAT_LLID_GENERAL_HOST_RATE_LIMITER_LLID_RX"

typedef enum{
    OLT_STAT_ARP_RELAY_FIRST_COUNTER,
    OLT_STAT_ARP_DISCARD_ARP_RELAY_DISABLED_LSB = OLT_STAT_ARP_RELAY_FIRST_COUNTER,
    OLT_STAT_ARP_DISCARD_ARP_RELAY_DISABLED_MSB,
    OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_LSB,
    OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_MSB,
    OLT_STAT_ARP_REQUEST_DISCARD_SERVICE_IS_N_2_1_REGULAR_LSB,
    OLT_STAT_ARP_REQUEST_DISCARD_SERVICE_IS_N_2_1_REGULAR_MSB,
    OLT_STAT_ARP_REQUEST_DISCARD_ANTISPOOF_FOUND_BUT_NO_UPRULE_LSB,
    OLT_STAT_ARP_REQUEST_DISCARD_ANTISPOOF_FOUND_BUT_NO_UPRULE_MSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NO_UNKNOW_PRVLAN_DEFINED_LSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NO_UNKNOW_PRVLAN_DEFINED_MSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_ANTISPOOF_ARRAY_LSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_ANTISPOOF_ARRAY_MSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_N_2_1_ARRAY_LSB,
    OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_N_2_1_ARRAY_MSB,
    OLT_STAT_ARP_DISCARD_UNKNOW_MAC_VID_OR_ARP_REPLY_LSB,
    OLT_STAT_ARP_DISCARD_UNKNOW_MAC_VID_OR_ARP_REPLY_MSB,
    OLT_STAT_ARP_DISCARD_NO_UPDOWN_RULES_LSB,
    OLT_STAT_ARP_DISCARD_NO_UPDOWN_RULES_MSB,
    OLT_STAT_ARP_RELAY_TASK_ERROR_LSB,
    OLT_STAT_ARP_RELAY_TASK_ERROR_MSB,
    OLT_STAT_ARP_RELAY_TOTAL_DISCARD_LSB,
    OLT_STAT_ARP_RELAY_TOTAL_DISCARD_MSB,
    OLT_STAT_ARP_RELAY_SEND_LSB,
    OLT_STAT_ARP_RELAY_SEND_MSB,
    OLT_STAT_ARP_DUPLICATE_SEND_LSB,
    OLT_STAT_ARP_DUPLICATE_SEND_MSB,
    OLT_STAT_ARP_RELAY_TOTAL_LSB,
    OLT_STAT_ARP_RELAY_TOTAL_MSB,
    OLT_STAT_ARP_RELAY_SINGLE_SEND_LSB,
    OLT_STAT_ARP_RELAY_SINGLE_SEND_MSB,
    OLT_STAT_ARP_RELAY_LAST_COUNTER = OLT_STAT_ARP_RELAY_SINGLE_SEND_MSB,
} OLT_ARP_RELAY_stat_counter_e;

#define OLT_STAT_ARP_DISCARD_ARP_RELAY_DISABLED_LSB_NAME "OLT_STAT_ARP_DISC_ARP_RELAY_DISABLED_LSB"
#define OLT_STAT_ARP_DISCARD_ARP_RELAY_DISABLED_MSB_NAME "OLT_STAT_ARP_DISC_ARP_RELAY_DISABLED_MSB"
#define OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_LSB_NAME "OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_LSB"
#define OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_MSB_NAME "OLT_STAT_ARP_RELAY_DISABLED_NOT_FOUND_IN_AT_MSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_SERVICE_IS_N_2_1_REGULAR_LSB_NAME "OLT_STAT_ARP_REQ_DISC_SERVICE_IS_N_2_1_REGULAR_LSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_SERVICE_IS_N_2_1_REGULAR_MSB_NAME "OLT_STAT_ARP_REQ_DISC_SERVICE_IS_N_2_1_REGULAR_MSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_ANTISPOOF_FOUND_BUT_NO_UPRULE_LSB_NAME "OLT_STAT_ARP_REQ_DISC_ANTI_FIND_BUT_NO_UPRULE_LSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_ANTISPOOF_FOUND_BUT_NO_UPRULE_MSB_NAME "OLT_STAT_ARP_REQ_DISC_ANTI_FIND_BUT_NO_UPRULE_MSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NO_UNKNOW_PRVLAN_DEFINED_LSB_NAME "OLT_STAT_ARP_REQ_DISC_NO_UNKNOW_PRVLAN_DEFINED_LSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NO_UNKNOW_PRVLAN_DEFINED_MSB_NAME "OLT_STAT_ARP_REQ_DISC_NO_UNKNOW_PRVLAN_DEFINED_MSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_ANTISPOOF_ARRAY_LSB_NAME "OLT_STAT_ARP_REQ_DISC_NOT_FOUND_IN_ANTI_ARRAY_LSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_ANTISPOOF_ARRAY_MSB_NAME "OLT_STAT_ARP_REQ_DIS_NOT_FOUND_IN_ANTI_ARRAY_MSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_N_2_1_ARRAY_LSB_NAME "OLT_STAT_ARP_REQ_DISC_NOT_FOUND_IN_N_2_1_ARRAY_LSB"
#define OLT_STAT_ARP_REQUEST_DISCARD_NOT_FOUND_IN_N_2_1_ARRAY_MSB_NAME "OLT_STAT_ARP_REQ_DISC_NOT_FOUND_IN_N_2_1_ARRAY_MSB"
#define OLT_STAT_ARP_DISCARD_UNKNOW_MAC_VID_OR_ARP_REPLY_LSB_NAME "OLT_STAT_ARP_DISC_UNKNOW_MAC_VID_OR_ARP_REPLY_LSB"
#define OLT_STAT_ARP_DISCARD_UNKNOW_MAC_VID_OR_ARP_REPLY_MSB_NAME "OLT_STAT_ARP_DISC_UNKNOW_MAC_VID_OR_ARP_REPLY_MSB"
#define OLT_STAT_ARP_DISCARD_NO_UPDOWN_RULES_LSB_NAME "OLT_STAT_ARP_DISC_NO_UPDOWN_RULES_LSB"
#define OLT_STAT_ARP_DISCARD_NO_UPDOWN_RULES_MSB_NAME "OLT_STAT_ARP_DISC_NO_UPDOWN_RULES_MSB"
#define OLT_STAT_ARP_RELAY_TASK_ERROR_LSB_NAME "OLT_STAT_ARP_RELAY_TASK_ERROR_LSB"
#define OLT_STAT_ARP_RELAY_TASK_ERROR_MSB_NAME "OLT_STAT_ARP_RELAY_TASK_ERROR_MSB"
#define OLT_STAT_ARP_RELAY_TOTAL_DISCARD_LSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_DISCARD_LSB"
#define OLT_STAT_ARP_RELAY_TOTAL_DISCARD_MSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_DISCARD_MSB"
#define OLT_STAT_ARP_RELAY_SEND_LSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_SEND_LSB"
#define OLT_STAT_ARP_RELAY_SEND_MSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_SEND_MSB"
#define OLT_STAT_ARP_DUPLICATE_SEND_LSB_NAME "OLT_STAT_ARP_DUPLICATE_SEND_LSB"
#define OLT_STAT_ARP_DUPLICATE_SEND_MSB_NAME "OLT_STAT_ARP_DUPLICATE_SEND_MSB"
#define OLT_STAT_ARP_RELAY_TOTAL_LSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_PACKETS_LSB"
#define OLT_STAT_ARP_RELAY_TOTAL_MSB_NAME "OLT_STAT_ARP_RELAY_TOTAL_PACKETS_MSB"
#define OLT_STAT_ARP_RELAY_SINGLE_SEND_LSB_NAME "OLT_STAT_ARP_RELAY_SINGLE_SEND_LSB"
#define OLT_STAT_ARP_RELAY_SINGLE_SEND_MSB_NAME "OLT_STAT_ARP_RELAY_SINGLE_SEND_MSB"

typedef enum{
    OLT_STAT_MLD_FIRST_COUNTER,
    OLT_STAT_MLD_ALLOW_NEW_SRC_CNT_RX_E = OLT_STAT_MLD_FIRST_COUNTER,
    OLT_STAT_MLD_ALLOW_NEW_SRC_CNT_TX_E, 
    OLT_STAT_MLD_BLOCK_OLD_SRC_CNT_RX_E,
    OLT_STAT_MLD_BLOCK_OLD_SRC_CNT_TX_E,
	OLT_STAT_MLD_IS_INCLUDE_CNT_RX_E,
	OLT_STAT_MLD_IS_INCLUDE_CNT_TX_E,
	OLT_STAT_MLD_IS_EXCLUDE_CNT_RX_E,
	OLT_STAT_MLD_IS_EXCLUDE_CNT_TX_E,
	OLT_STAT_MLD_TO_INCLUDE_CNT_RX_E,
	OLT_STAT_MLD_TO_INCLUDE_CNT_TX_E,
	OLT_STAT_MLD_TO_EXCLUDE_CNT_RX_E,
	OLT_STAT_MLD_TO_EXCLUDE_CNT_TX_E,
	OLT_STAT_MLD_GQ_CNT_RX_E,
	OLT_STAT_MLD_GQ_CNT_TX_E,
	OLT_STAT_MLD_MASQ_CNT_RX_E,
	OLT_STAT_MLD_MASQ_CNT_TX_E,
	OLT_STAT_MLD_MASSQ_CNT_RX_E,
	OLT_STAT_MLD_MASSQ_CNT_TX_E,
    OLT_STAT_MLD_LAST_COUNTER = OLT_STAT_MLD_MASSQ_CNT_TX_E,
} OLT_MLD_stat_counter_e;


#define OLT_STAT_MLD_ALLOW_NEW_SRC_RX_CNT_NAME	"OLT_STAT_MLD_ALLOW_NEW_SRC_RX_CNT"
#define OLT_STAT_MLD_ALLOW_NEW_SRC_TX_CNT_NAME	"OLT_STAT_MLD_ALLOW_NEW_SRC_TX_CNT"
#define OLT_STAT_MLD_BLOCK_OLD_SRC_RX_CNT_NAME	"OLT_STAT_MLD_BLOCK_OLD_SRC_RX_CNT"
#define OLT_STAT_MLD_BLOCK_OLD_SRC_TX_CNT_NAME	"OLT_STAT_MLD_BLOCK_OLD_SRC_TX_CNT"
#define OLT_STAT_MLD_IS_INCLUDE_RX_CNT_NAME		"OLT_STAT_MLD_IS_INCLUDE_RX_CNT"
#define OLT_STAT_MLD_IS_INCLUDE_TX_CNT_NAME		"OLT_STAT_MLD_IS_INCLUDE_TX_CNT"
#define OLT_STAT_MLD_IS_EXCLUDE_RX_CNT_NAME		"OLT_STAT_MLD_IS_EXCLUDE_RX_CNT"
#define OLT_STAT_MLD_IS_EXCLUDE_TX_CNT_NAME		"OLT_STAT_MLD_IS_EXCLUDE_TX_CNT"
#define OLT_STAT_MLD_TO_INCLUDE_RX_CNT_NAME		"OLT_STAT_MLD_TO_INCLUDE_RX_CNT"
#define OLT_STAT_MLD_TO_INCLUDE_TX_CNT_NAME		"OLT_STAT_MLD_TO_INCLUDE_TX_CNT"
#define OLT_STAT_MLD_TO_EXCLUDE_RX_CNT_NAME		"OLT_STAT_MLD_TO_EXCLUDE_RX_CNT"
#define OLT_STAT_MLD_TO_EXCLUDE_TX_CNT_NAME		"OLT_STAT_MLD_TO_EXCLUDE_TX_CNT"
#define OLT_STAT_MLD_GQ_RX_CNT_NAME				"OLT_STAT_MLD_GQ_RX_CNT"
#define OLT_STAT_MLD_GQ_TX_CNT_NAME				"OLT_STAT_MLD_GQ_TX_CNT"
#define OLT_STAT_MLD_MASQ_RX_CNT_NAME			"OLT_STAT_MLD_MASQ_RX_CNT"
#define OLT_STAT_MLD_MASQ_TX_CNT_NAME			"OLT_STAT_MLD_MASQ_TX_CNT"
#define OLT_STAT_MLD_MASSQ_RX_CNT_NAME			"OLT_STAT_MLD_MASSQ_RX_CNT"
#define OLT_STAT_MLD_MASSQ_TX_CNT_NAME			"OLT_STAT_MLD_MASSQ_TX_CNT"


typedef enum{
    OLT_STAT_PON_10G_ENC_FIRST_COUNTER,
    OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_LSB_E = OLT_STAT_PON_10G_ENC_FIRST_COUNTER,
    OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_LSB_E, 
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_MSB_E, 
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_MSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_LSB_E,
	OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_MSB_E,
	OLT_STAT_PON_10G_ENC_LAST_COUNTER = OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_MSB_E,
} OLT_PON_10G_ENC_stat_counter_e;


#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_LSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_MSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_PROT_OR_CHURN0_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_LSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_MSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_ENC_OR_CHURN1_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_LSB_NAME			"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_MSB_NAME			"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_DISCARD_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_LSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_MSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_UC_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_LSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_MSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_MC_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_LSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_MSB_NAME					"OLT_STAT_PON_10G_ENC_TOTAL_IP_FRAME_BC_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_LSB_NAME	"OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_MSB_NAME	"OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_PROT_OR_CHURN0_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_LSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_MSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_OCTETS_ENC_OR_CHURN1_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_LSB_NAME			"OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_MSB_NAME			"OLT_STAT_PON_10G_ENC_TOTAL_IP_NET_DATA_LENGTH_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_LSB_NAME				"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_MSB_NAME				"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_NOEP_MSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_LSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_LSB"
#define OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_MSB_NAME		"OLT_STAT_PON_10G_ENC_TOTAL_OP_FRAME_EP_TOO_LONG_MSB"


typedef enum{
    OLT_STAT_PON_10G_DEC_FIRST_COUNTER,
    OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_LSB_E = OLT_STAT_PON_10G_DEC_FIRST_COUNTER,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_MSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_LSB_E,
	OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_MSB_E,
	OLT_STAT_PON_10G_DEC_LAST_COUNTER = OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_MSB_E,
} OLT_PON_10G_DEC_stat_counter_e;


#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNTAGGED_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_TAG_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_BAD_TAG_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_BYPASS_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_DISCARD_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_TCI_PKTS_NO_REMOVE_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_LSB_NAME				"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_MSB_NAME				"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_VALIDATED_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_OCTETS_DECRYPTED_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_LATE_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_DELAYED_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNCHECKED_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_VALID_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_INVALID_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_LSB_NAME				"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_MSB_NAME				"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_OK_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNUSED_SA_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_LSB_NAME	"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_MSB_NAME	"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NOT_USING_SA_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_LSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_MSB_NAME		"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_UNKNOWN_SCI_MSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_LSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_LSB"
#define OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_MSB_NAME			"OLT_STAT_PON_10G_DEC_TOTAL_IN_PKTS_NO_SCI_MSB"


typedef enum{
    OLT_STAT_LLID_10G_ENC_FIRST_COUNTER,
    OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_LSB_E = OLT_STAT_LLID_10G_ENC_FIRST_COUNTER,
	OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_MSB_E,
	OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_LSB_E,
	OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_MSB_E,
	OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_LSB_E,
	OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_MSB_E,
	OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_LSB_E,
	OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_MSB_E,
	OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_LSB_E,
	OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_MSB_E,
	OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_LSB_E,
	OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_MSB_E,
	OLT_STAT_LLID_10G_ENC_LAST_COUNTER = OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_MSB_E,
} OLT_LLID_10G_ENC_stat_counter_e;


#define OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_LSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_LSB"
#define OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_MSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_UC_MSB"
#define OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_LSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_LSB"
#define OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_MSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_MC_MSB"
#define OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_LSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_LSB"
#define OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_MSB_NAME					"OLT_STAT_LLID_10G_ENC_IP_FRAME_BC_MSB"
#define OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_LSB_NAME				"OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_LSB"
#define OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_MSB_NAME				"OLT_STAT_LLID_10G_ENC_OP_FRAME_NOEP_MSB"
#define OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_LSB_NAME			"OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_LSB"
#define OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_MSB_NAME			"OLT_STAT_LLID_10G_ENC_OP_FRAME_EP_TOO_LONG_MSB"
#define OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_LSB_NAME			"OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_LSB"
#define OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_MSB_NAME			"OLT_STAT_LLID_10G_ENC_IP_NET_DATA_LENGTH_MSB"


typedef enum{
    OLT_STAT_LLID_10G_DEC_FIRST_COUNTER,
    OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_LSB_E = OLT_STAT_LLID_10G_DEC_FIRST_COUNTER,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_MSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_LSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_MSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_LSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_MSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_LSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_MSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_LSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_MSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_LSB_E,
	OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_MSB_E,
	OLT_STAT_LLID_10G_DEC_IN_OCTETS_LSB_E,
	OLT_STAT_LLID_10G_DEC_IN_OCTETS_MSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_LSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_MSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_LSB_E,
	OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_MSB_E,
	OLT_STAT_LLID_10G_DEC_LAST_COUNTER = OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_MSB_E,
} OLT_LLID_10G_DEC_stat_counter_e;


#define OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_LSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_MSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_UNTAGGED_MSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_LSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_MSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_TAG_MSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_LSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_MSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_BAD_TAG_MSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_LSB_NAME			"OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_LSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_MSB_NAME			"OLT_STAT_LLID_10G_DEC_TCI_PKTS_BYPASS_MSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_LSB_NAME			"OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_LSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_MSB_NAME			"OLT_STAT_LLID_10G_DEC_TCI_PKTS_DISCARD_MSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_LSB_NAME		"OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_LSB"
#define OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_MSB_NAME		"OLT_STAT_LLID_10G_DEC_TCI_PKTS_NO_REMOVE_MSB"
#define OLT_STAT_LLID_10G_DEC_IN_OCTETS_LSB_NAME				"OLT_STAT_LLID_10G_DEC_IN_OCTETS_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_OCTETS_MSB_NAME				"OLT_STAT_LLID_10G_DEC_IN_OCTETS_MSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_LSB_NAME		"OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_MSB_NAME		"OLT_STAT_LLID_10G_DEC_IN_PKTS_UNKNOWN_SCI_MSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_LSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_LSB"
#define OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_MSB_NAME			"OLT_STAT_LLID_10G_DEC_IN_PKTS_NO_SCI_MSB"


typedef enum{
    OLT_STAT_KEY_10G_ENC_FIRST_COUNTER,
    OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_LSB_E = OLT_STAT_KEY_10G_ENC_FIRST_COUNTER,
	OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_MSB_E,
	OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_LSB_E,
	OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_MSB_E,
	OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_LSB_E,
	OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_MSB_E,
	OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_LSB_E,
	OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_MSB_E,
	OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_LSB_E,
	OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_MSB_E,
	OLT_STAT_KEY_10G_ENC_LAST_COUNTER = OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_MSB_E,
} OLT_KEY_10G_ENC_stat_counter_e;


#define OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_LSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_LSB"
#define OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_MSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_FRAME_PROT_OR_CHURN0_MSB"
#define OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_LSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_LSB"
#define OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_MSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_FRAME_ENC_OR_CHURN1_MSB"
#define OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_LSB_NAME				"OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_LSB"
#define OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_MSB_NAME				"OLT_STAT_KEY_10G_ENC_OP_FRAME_DISCARD_MSB"
#define OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_LSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_LSB"
#define OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_MSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_OCTETS_PROT_OR_CHURN0_MSB"
#define OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_LSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_LSB"
#define OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_MSB_NAME		"OLT_STAT_KEY_10G_ENC_OP_OCTETS_ENC_OR_CHURN1_MSB"


typedef enum{
    OLT_STAT_KEY_10G_DEC_FIRST_COUNTER,
    OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_LSB_E = OLT_STAT_KEY_10G_DEC_FIRST_COUNTER,
	OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_UNCHECKED_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_UNCHECKED_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_MSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_LSB_E,
	OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_MSB_E,
	OLT_STAT_KEY_10G_DEC_LAST_COUNTER = OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_MSB_E,
} OLT_KEY_10G_DEC_stat_counter_e;


#define OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_LSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_MSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_OCTETS_VALIDATED_MSB"		
#define OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_LSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_MSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_OCTETS_DECRYPTED_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_LSB_NAME				"OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_MSB_NAME				"OLT_STAT_KEY_10G_DEC_IN_PKTS_LATE_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_LSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_MSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_DELAYED_MSB"
#define OLT_STAT_KEY_10G__DEC_IN_PKTS_UNCHECKED_LSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_PKTS_UNCHECKED_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_UNCHECKED_MSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_UNCHECKED_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_LSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_MSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_VALID_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_LSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_MSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_INVALID_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_LSB_NAME				"OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_MSB_NAME				"OLT_STAT_KEY_10G_DEC_IN_PKTS_OK_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_LSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_MSB_NAME			"OLT_STAT_KEY_10G_DEC_IN_PKTS_UNUSED_SA_MSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_LSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_LSB"
#define OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_MSB_NAME		"OLT_STAT_KEY_10G_DEC_IN_PKTS_NOT_USING_SA_MSB"

typedef enum{
    OLT_STAT_PSTAT_UPSTREAM_OCTET_LSB,
    OLT_STAT_PSTAT_UPSTREAM_OCTET_MSB,
    OLT_STAT_PSTAT_UPSTREAM_PACKET_LSB,
    OLT_STAT_PSTAT_UPSTREAM_PACKET_MSB,
    OLT_STAT_PSTAT_DOWNSTREAM_OCTET_LSB,
    OLT_STAT_PSTAT_DOWNSTREAM_OCTET_MSB,
    OLT_STAT_PSTAT_DOWNSTREAM_PACKET_LSB,
    OLT_STAT_PSTAT_DOWNSTREAM_PACKET_MSB,
} OLT_PSTAT_stat_counter_e; 

#define OLT_STAT_PSTAT_UPSTREAM_OCTET_LSB_NAME    "OLT_STAT_PSTAT_UPSTREAM_OCTET_LSB"
#define OLT_STAT_PSTAT_UPSTREAM_OCTET_MSB_NAME    "OLT_STAT_PSTAT_UPSTREAM_OCTET_MSB"
#define OLT_STAT_PSTAT_UPSTREAM_PACKET_LSB_NAME   "OLT_STAT_PSTAT_UPSTREAM_PACKET_LSB"
#define OLT_STAT_PSTAT_UPSTREAM_PACKET_MSB_NAME   "OLT_STAT_PSTAT_UPSTREAM_PACKET_MSB"
#define OLT_STAT_PSTAT_DOWNSTREAM_OCTET_LSB_NAME  "OLT_STAT_PSTAT_DOWNSTREAM_OCTET_LSB"
#define OLT_STAT_PSTAT_DOWNSTREAM_OCTET_MSB_NAME  "OLT_STAT_PSTAT_DOWNSTREAM_OCTET_MSB"
#define OLT_STAT_PSTAT_DOWNSTREAM_PACKET_LSB_NAME "OLT_STAT_PSTAT_DOWNSTREAM_PACKET_LSB"
#define OLT_STAT_PSTAT_DOWNSTREAM_PACKET_MSB_NAME "OLT_STAT_PSTAT_DOWNSTREAM_PACKET_MSB"

/* entities definition */										  
/* =================== */										  
typedef enum                                                      
{                                                                 
  STAT_VIEW_CNI_BER,
  STAT_VIEW_PON1G_BER,
  STAT_VIEW_PON10G_BER,
  STAT_VIEW_CNI_FER,
  STAT_VIEW_PON1G_FER,
  STAT_VIEW_PON10G_FER
} STAT_view_type_e;
/* entities definition */										  
/* =================== */										  
typedef enum                                                      
{                                                                 
  STAT_ENTITY_FIRST_ENUM			= 0,
  STAT_ENTITY_PON_E					= STAT_ENTITY_FIRST_ENUM,
  STAT_ENTITY_CNI_E					= 1,
  STAT_ENTITY_LLID_E				= 2, 
  STAT_ENTITY_PON_10G_E				= 3,
  STAT_ENTITY_LLID_10G_E			= 4,
  /* The following types are only sent to the FW to distinguish between XMT/XMR statistics */
  STAT_ENTITY_PON_10G_XMT_E			= 5,
  STAT_ENTITY_PON_10G_XMR_E			= 6,
  STAT_ENTITY_LLID_10G_XMT_E		= 7,
  STAT_ENTITY_LLID_10G_XMR_E		= 8,
  STAT_ENTITY_QUEUE_OCCUPANCY_E     = 9,
  STAT_ENTITY_OAM_E                 = 10,
  STAT_ENTITY_MPCP_E				= 11,
  STAT_ENTITY_LLID_GENERAL_E		= 12,
  STAT_ENTITY_ARP_RELAY_E			= 13,
  STAT_ENTITY_MLD_CNI_E				= 14,
  STAT_ENTITY_MLD_PON_E				= 15,
  STAT_ENTITY_MLD_LLID_E			= 16,
  STAT_ENTITY_PON_10G_ENC_E			= 17,
  STAT_ENTITY_PON_10G_DEC_E			= 18,
  STAT_ENTITY_LLID_10G_ENC_E		= 19,
  STAT_ENTITY_LLID_10G_DEC_E		= 20,
  STAT_ENTITY_KEY_10G_ENC_E			= 21,
  STAT_ENTITY_KEY_10G_DEC_E			= 22,
  STAT_ENTITY_PSTAT_E        = 23,
  STAT_ENTITY_LAST_ENUM				= STAT_ENTITY_PSTAT_E,
  STAT_ENTITY_ERROR_E		 
} STAT_olt_entity_e;



typedef struct
{
	STAT_olt_entity_e	entity;
	INT32U				entity_index;
	INT32U				statistic_index;    
	INT32U				reset_status;    
} STAT_reset_field_t;

typedef struct
{
	STAT_olt_entity_e	entity;
	INT32U				entity_index;
	INT32U				statistic_index;    
	INT32U				statistic_value;    
} STAT_field_t;
#endif

/* Optics type (1G / 10G) */
typedef enum GW10G_PAS_ethernet_mode_e
{
    ETHERNET_MAC_MODE_XAUI  = 0,    
    ETHERNET_MAC_MODE_RXAUI	= 1,    
    ETHERNET_MAC_MODE_SGMII = 2    /* applicable in PAS5402 only */ 
} GW10G_PAS_ethernet_mode_e;


typedef enum GW10G_PON_cni_mac_state
{
	GW10G_PON_CNI_MAC_DISABLE = 0,
	GW10G_PON_CNI_MAC_ENABLE,
	GW10G_PON_CNI_MAC_STANDBY
} GW10G_PON_cni_mac_state_e;
typedef enum GW10G_PON_cni_config_type_e
{
	GW10G_PON_CNI_CONFIG_MAC_MODE = 0,
	GW10G_PON_CNI_CONFIG_MAC_STATE
} GW10G_PON_cni_config_type_e;


typedef union GW10G_PON_cni_config_value_u
{
	GW10G_PAS_ethernet_mode_e	       cni_mac_mode;
	GW10G_PON_cni_mac_state_e	       cni_mac_state;
} GW10G_PON_cni_config_value_u;

/* TM_queue_id_e */
typedef enum TM_queue_id_e
{
    TM_QUEUE_ID_INVALID     = 0,                /* Invalid queue id. */
    TM_QUEUE_ID_CPU_UP_RX_0 = 0x300000,         /* CPU receive queue in upstream for priority 0. */
    
    TM_QUEUE_ID_CPU_UP_RX_1,         /* CPU receive queue in upstream for priority 1. */
    TM_QUEUE_ID_CPU_UP_RX_2,         /* CPU receive queue in upstream for priority 2. */
    TM_QUEUE_ID_CPU_UP_RX_3,         /* CPU receive queue in upstream for priority 3. */
    TM_QUEUE_ID_CPU_UP_RX_4,         /* CPU receive queue in upstream for priority 4. */
    TM_QUEUE_ID_CPU_UP_RX_5,         /* CPU receive queue in upstream for priority 5. */
    TM_QUEUE_ID_CPU_UP_RX_6,         /* CPU receive queue in upstream for priority 6. */
    TM_QUEUE_ID_CPU_UP_RX_7,         /* CPU receive queue in upstream for priority 7. */
    
    TM_QUEUE_ID_CPU_UP_RX_USER,      /* CPU receive queue in upstream for user.
                                        Should be equal to  M_queue_id_cpu_up_rx_3 for 4 queues
                                     */
                                     
    TM_QUEUE_ID_CPU_DOWN_RX_0,       /* CPU receive queue in downstream for priority 0. */
    TM_QUEUE_ID_CPU_DOWN_RX_1,       /* CPU receive queue in downstream for priority 1. */
    TM_QUEUE_ID_CPU_DOWN_RX_2,       /* CPU receive queue in downstream for priority 2. */
    TM_QUEUE_ID_CPU_DOWN_RX_3,       /* CPU receive queue in downstream for priority 3. */
    TM_QUEUE_ID_CPU_DOWN_RX_4,       /* CPU receive queue in downstream for priority 4. */
    TM_QUEUE_ID_CPU_DOWN_RX_5,       /* CPU receive queue in downstream for priority 5. */
    TM_QUEUE_ID_CPU_DOWN_RX_6,       /* CPU receive queue in downstream for priority 6. */
    TM_QUEUE_ID_CPU_DOWN_RX_7,       /* CPU receive queue in downstream for priority 7. */
    
    TM_QUEUE_ID_CPU_DOWN_RX_USER,    /* CPU receive queue in downstream for user.
                                        Should be equal to  M_queue_id_cpu_down_rx_3 for 4 queues
                                     */
    
    TM_QUEUE_ID_SNIFF_RX_0,          /* Sniff port receive queue in upstream for priority 0. */
    TM_QUEUE_ID_SNIFF_RX_1,          /* Sniff port receive queue in upstream for priority 1. */
    TM_QUEUE_ID_SNIFF_RX_2,          /* Sniff port receive queue in upstream for priority 2. */
    TM_QUEUE_ID_SNIFF_RX_3,          /* Sniff port receive queue in upstream for priority 3. */
    TM_QUEUE_ID_SNIFF_RX_4,          /* Sniff port receive queue in upstream for priority 4. */
    TM_QUEUE_ID_SNIFF_RX_5,          /* Sniff port receive queue in upstream for priority 5. */
    TM_QUEUE_ID_SNIFF_RX_6,          /* Sniff port receive queue in upstream for priority 6. */
    TM_QUEUE_ID_SNIFF_RX_7,          /* Sniff port receive queue in upstream for priority 7. */
    TM_QUEUE_ID_SNIFF_TX_0,          /* Sniff port transmit queue in upstream for priority 0. */
    TM_QUEUE_ID_SNIFF_TX_1,          /* Sniff port transmit queue in upstream for priority 1. */
    TM_QUEUE_ID_SNIFF_TX_2,          /* Sniff port transmit queue in upstream for priority 2. */
    TM_QUEUE_ID_SNIFF_TX_3,          /* Sniff port transmit queue in upstream for priority 3. */
    TM_QUEUE_ID_SNIFF_TX_4,          /* Sniff port transmit queue in upstream for priority 4. */
    TM_QUEUE_ID_SNIFF_TX_5,          /* Sniff port transmit queue in upstream for priority 5. */
    TM_QUEUE_ID_SNIFF_TX_6,          /* Sniff port transmit queue in upstream for priority 6. */
    TM_QUEUE_ID_SNIFF_TX_7,          /* Sniff port transmit queue in upstream for priority 7. */
    
    TM_QUEUE_ID_CNI0_PRIORITY0,      /* This queue id is used for priority 0 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY1,      /* This queue id is used for priority 1 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY2,      /* This queue id is used for priority 2 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY3,      /* This queue id is used for priority 3 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY4,      /* This queue id is used for priority 4 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY5,      /* This queue id is used for priority 5 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY6,      /* This queue id is used for priority 6 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI0_PRIORITY7,      /* This queue id is used for priority 7 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY0,      /* This queue id is used for priority 0 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY1,      /* This queue id is used for priority 1 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY2,      /* This queue id is used for priority 2 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY3,      /* This queue id is used for priority 3 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY4,      /* This queue id is used for priority 4 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY5,      /* This queue id is used for priority 5 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY6,      /* This queue id is used for priority 6 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_CNI1_PRIORITY7,      /* This queue id is used for priority 7 frames that transmit to CNI1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    
    TM_QUEUE_ID_PON0_PRIORITY0,      /* This queue id is used for priority 0 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY1,      /* This queue id is used for priority 1 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY2,      /* This queue id is used for priority 2 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY3,      /* This queue id is used for priority 3 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY4,      /* This queue id is used for priority 4 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY5,      /* This queue id is used for priority 5 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY6,      /* This queue id is used for priority 6 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON0_PRIORITY7,      /* This queue id is used for priority 7 frames that transmit to PON0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY0,      /* This queue id is used for priority 0 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY1,      /* This queue id is used for priority 1 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY2,      /* This queue id is used for priority 2 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY3,      /* This queue id is used for priority 3 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY4,      /* This queue id is used for priority 4 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY5,      /* This queue id is used for priority 5 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY6,      /* This queue id is used for priority 6 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_PON1_PRIORITY7,      /* This queue id is used for priority 7 frames that transmit to PON1. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    
	TM_QUEUE_ID_PS_PON0_LLID0,       /* This queue id is used for llid 0 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID1,       /* This queue id is used for llid 1 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID2,       /* This queue id is used for llid 2 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID3,       /* This queue id is used for llid 3 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID4,       /* This queue id is used for llid 4 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID5,       /* This queue id is used for llid 5 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID6,       /* This queue id is used for llid 6 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID7,       /* This queue id is used for llid 7 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID8,       /* This queue id is used for llid 8 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID9,       /* This queue id is used for llid 9 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID10,      /* This queue id is used for llid 10 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID11,      /* This queue id is used for llid 11 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID12,      /* This queue id is used for llid 12 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID13,      /* This queue id is used for llid 13 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID14,      /* This queue id is used for llid 14 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID15,      /* This queue id is used for llid 15 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID16,      /* This queue id is used for llid 16 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID17,      /* This queue id is used for llid 17 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID18,      /* This queue id is used for llid 18 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID19,      /* This queue id is used for llid 19 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID20,      /* This queue id is used for llid 20 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID21,      /* This queue id is used for llid 21 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID22,      /* This queue id is used for llid 22 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID23,      /* This queue id is used for llid 23 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID24,      /* This queue id is used for llid 24 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID25,      /* This queue id is used for llid 25 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID26,      /* This queue id is used for llid 26 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID27,      /* This queue id is used for llid 27 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID28,      /* This queue id is used for llid 28 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID29,      /* This queue id is used for llid 29 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID30,      /* This queue id is used for llid 30 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID31,      /* This queue id is used for llid 31 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID32,      /* This queue id is used for llid 32 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID33,      /* This queue id is used for llid 33 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID34,      /* This queue id is used for llid 34 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID35,      /* This queue id is used for llid 35 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID36,      /* This queue id is used for llid 36 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID37,      /* This queue id is used for llid 37 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID38,      /* This queue id is used for llid 38 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID39,      /* This queue id is used for llid 39 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID40,      /* This queue id is used for llid 40 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID41,      /* This queue id is used for llid 41 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID42,      /* This queue id is used for llid 42 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID43,      /* This queue id is used for llid 43 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID44,      /* This queue id is used for llid 44 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID45,      /* This queue id is used for llid 45 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID46,      /* This queue id is used for llid 46 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID47,      /* This queue id is used for llid 47 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID48,      /* This queue id is used for llid 48 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID49,      /* This queue id is used for llid 49 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID50,      /* This queue id is used for llid 50 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID51,      /* This queue id is used for llid 51 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID52,      /* This queue id is used for llid 52 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID53,      /* This queue id is used for llid 53 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID54,      /* This queue id is used for llid 54 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID55,      /* This queue id is used for llid 55 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID56,      /* This queue id is used for llid 56 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID57,      /* This queue id is used for llid 57 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID58,      /* This queue id is used for llid 58 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID59,      /* This queue id is used for llid 59 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID60,      /* This queue id is used for llid 60 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID61,      /* This queue id is used for llid 61 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID62,      /* This queue id is used for llid 62 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID63,      /* This queue id is used for llid 63 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID64,      /* This queue id is used for llid 64 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID65,      /* This queue id is used for llid 65 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID66,      /* This queue id is used for llid 66 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID67,      /* This queue id is used for llid 67 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID68,      /* This queue id is used for llid 68 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID69,      /* This queue id is used for llid 69 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID70,      /* This queue id is used for llid 70 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID71,      /* This queue id is used for llid 71 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID72,      /* This queue id is used for llid 72 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID73,      /* This queue id is used for llid 73 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID74,      /* This queue id is used for llid 74 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID75,      /* This queue id is used for llid 75 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID76,      /* This queue id is used for llid 76 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID77,      /* This queue id is used for llid 77 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID78,      /* This queue id is used for llid 78 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID79,      /* This queue id is used for llid 79 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID80,      /* This queue id is used for llid 80 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID81,      /* This queue id is used for llid 81 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID82,      /* This queue id is used for llid 82 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID83,      /* This queue id is used for llid 83 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID84,      /* This queue id is used for llid 84 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID85,      /* This queue id is used for llid 85 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID86,      /* This queue id is used for llid 86 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID87,      /* This queue id is used for llid 87 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID88,      /* This queue id is used for llid 88 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID89,      /* This queue id is used for llid 89 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID90,      /* This queue id is used for llid 90 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID91,      /* This queue id is used for llid 91 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID92,      /* This queue id is used for llid 92 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID93,      /* This queue id is used for llid 93 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID94,      /* This queue id is used for llid 94 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID95,      /* This queue id is used for llid 95 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID96,      /* This queue id is used for llid 96 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID97,      /* This queue id is used for llid 97 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID98,      /* This queue id is used for llid 98 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID99,      /* This queue id is used for llid 99 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID100,     /* This queue id is used for llid 100 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID101,     /* This queue id is used for llid 101 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID102,     /* This queue id is used for llid 102 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID103,     /* This queue id is used for llid 103 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID104,     /* This queue id is used for llid 104 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID105,     /* This queue id is used for llid 105 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID106,     /* This queue id is used for llid 106 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID107,     /* This queue id is used for llid 107 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID108,     /* This queue id is used for llid 108 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID109,     /* This queue id is used for llid 109 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID110,     /* This queue id is used for llid 110 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID111,     /* This queue id is used for llid 111 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID112,     /* This queue id is used for llid 112 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID113,     /* This queue id is used for llid 113 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID114,     /* This queue id is used for llid 114 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID115,     /* This queue id is used for llid 115 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID116,     /* This queue id is used for llid 116 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID117,     /* This queue id is used for llid 117 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID118,     /* This queue id is used for llid 118 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID119,     /* This queue id is used for llid 119 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID120,     /* This queue id is used for llid 120 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID121,     /* This queue id is used for llid 121 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID122,     /* This queue id is used for llid 122 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID123,     /* This queue id is used for llid 123 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID124,     /* This queue id is used for llid 124 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID125,     /* This queue id is used for llid 125 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID126,     /* This queue id is used for llid 126 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON0_LLID127,     /* This queue id is used for llid 127 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/

    TM_QUEUE_ID_PS_PON1_LLID0,       /* This queue id is used for llid 1 for pon0 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID1,       /* This queue id is used for llid 1 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID2,       /* This queue id is used for llid 2 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID3,       /* This queue id is used for llid 3 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID4,       /* This queue id is used for llid 4 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID5,       /* This queue id is used for llid 5 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID6,       /* This queue id is used for llid 6 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID7,       /* This queue id is used for llid 7 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID8,       /* This queue id is used for llid 8 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID9,       /* This queue id is used for llid 9 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID10,      /* This queue id is used for llid 10 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID11,      /* This queue id is used for llid 11 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID12,      /* This queue id is used for llid 12 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID13,      /* This queue id is used for llid 13 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID14,      /* This queue id is used for llid 14 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID15,      /* This queue id is used for llid 15 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID16,      /* This queue id is used for llid 16 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID17,      /* This queue id is used for llid 17 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID18,      /* This queue id is used for llid 18 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID19,      /* This queue id is used for llid 19 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID20,      /* This queue id is used for llid 20 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID21,      /* This queue id is used for llid 21 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID22,      /* This queue id is used for llid 22 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID23,      /* This queue id is used for llid 23 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID24,      /* This queue id is used for llid 24 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID25,      /* This queue id is used for llid 25 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID26,      /* This queue id is used for llid 26 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID27,      /* This queue id is used for llid 27 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID28,      /* This queue id is used for llid 28 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID29,      /* This queue id is used for llid 29 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID30,      /* This queue id is used for llid 30 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID31,      /* This queue id is used for llid 31 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID32,      /* This queue id is used for llid 32 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID33,      /* This queue id is used for llid 33 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID34,      /* This queue id is used for llid 34 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID35,      /* This queue id is used for llid 35 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID36,      /* This queue id is used for llid 36 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID37,      /* This queue id is used for llid 37 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID38,      /* This queue id is used for llid 38 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID39,      /* This queue id is used for llid 39 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID40,      /* This queue id is used for llid 40 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID41,      /* This queue id is used for llid 41 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID42,      /* This queue id is used for llid 42 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID43,      /* This queue id is used for llid 43 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID44,      /* This queue id is used for llid 44 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID45,      /* This queue id is used for llid 45 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID46,      /* This queue id is used for llid 46 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID47,      /* This queue id is used for llid 47 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID48,      /* This queue id is used for llid 48 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID49,      /* This queue id is used for llid 49 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID50,      /* This queue id is used for llid 50 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID51,      /* This queue id is used for llid 51 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID52,      /* This queue id is used for llid 52 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID53,      /* This queue id is used for llid 53 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID54,      /* This queue id is used for llid 54 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID55,      /* This queue id is used for llid 55 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID56,      /* This queue id is used for llid 56 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID57,      /* This queue id is used for llid 57 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID58,      /* This queue id is used for llid 58 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID59,      /* This queue id is used for llid 59 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID60,      /* This queue id is used for llid 60 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID61,      /* This queue id is used for llid 61 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID62,      /* This queue id is used for llid 62 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID63,      /* This queue id is used for llid 63 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID64,      /* This queue id is used for llid 64 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID65,      /* This queue id is used for llid 65 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID66,      /* This queue id is used for llid 66 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID67,      /* This queue id is used for llid 67 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID68,      /* This queue id is used for llid 68 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID69,      /* This queue id is used for llid 69 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID70,      /* This queue id is used for llid 70 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID71,      /* This queue id is used for llid 71 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID72,      /* This queue id is used for llid 72 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID73,      /* This queue id is used for llid 73 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID74,      /* This queue id is used for llid 74 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID75,      /* This queue id is used for llid 75 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID76,      /* This queue id is used for llid 76 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID77,      /* This queue id is used for llid 77 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID78,      /* This queue id is used for llid 78 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID79,      /* This queue id is used for llid 79 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID80,      /* This queue id is used for llid 80 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID81,      /* This queue id is used for llid 81 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID82,      /* This queue id is used for llid 82 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID83,      /* This queue id is used for llid 83 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID84,      /* This queue id is used for llid 84 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID85,      /* This queue id is used for llid 85 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID86,      /* This queue id is used for llid 86 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID87,      /* This queue id is used for llid 87 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID88,      /* This queue id is used for llid 88 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID89,      /* This queue id is used for llid 89 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID90,      /* This queue id is used for llid 90 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID91,      /* This queue id is used for llid 91 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID92,      /* This queue id is used for llid 92 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID93,      /* This queue id is used for llid 93 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID94,      /* This queue id is used for llid 94 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID95,      /* This queue id is used for llid 95 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID96,      /* This queue id is used for llid 96 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID97,      /* This queue id is used for llid 97 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID98,      /* This queue id is used for llid 98 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID99,      /* This queue id is used for llid 99 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID100,     /* This queue id is used for llid 100 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID101,     /* This queue id is used for llid 101 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID102,     /* This queue id is used for llid 102 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID103,     /* This queue id is used for llid 103 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID104,     /* This queue id is used for llid 104 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID105,     /* This queue id is used for llid 105 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID106,     /* This queue id is used for llid 106 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID107,     /* This queue id is used for llid 107 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID108,     /* This queue id is used for llid 108 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID109,     /* This queue id is used for llid 109 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID110,     /* This queue id is used for llid 110 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID111,     /* This queue id is used for llid 111 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID112,     /* This queue id is used for llid 112 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID113,     /* This queue id is used for llid 113 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID114,     /* This queue id is used for llid 114 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID115,     /* This queue id is used for llid 115 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID116,     /* This queue id is used for llid 116 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID117,     /* This queue id is used for llid 117 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID118,     /* This queue id is used for llid 118 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID119,     /* This queue id is used for llid 119 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID120,     /* This queue id is used for llid 120 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID121,     /* This queue id is used for llid 121 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID122,     /* This queue id is used for llid 122 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID123,     /* This queue id is used for llid 123 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID124,     /* This queue id is used for llid 124 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID125,     /* This queue id is used for llid 125 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID126,     /* This queue id is used for llid 126 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/
    TM_QUEUE_ID_PS_PON1_LLID127,     /* This queue id is used for llid 127 for pon1 frames. This queue id is only valid when TM_sched_simple_powersave is used.*/

    TM_QUEUE_ID_P2P_PRIORITY0,       /* This queue id is used for priority 0 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY1,       /* This queue id is used for priority 1 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY2,       /* This queue id is used for priority 2 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY3,       /* This queue id is used for priority 3 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY4,       /* This queue id is used for priority 4 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY5,       /* This queue id is used for priority 5 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY6,       /* This queue id is used for priority 6 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    TM_QUEUE_ID_P2P_PRIORITY7,       /* This queue id is used for priority 7 frames that transmit to peer to peer port. This queue id is only valid when TM_sched_mode is used.*/
    
	TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY0,      /* This queue id is used for priority 0 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY1,      /* This queue id is used for priority 1 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY2,      /* This queue id is used for priority 2 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY3,      /* This queue id is used for priority 3 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY4,      /* This queue id is used for priority 4 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY5,      /* This queue id is used for priority 5 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY6,      /* This queue id is used for priority 6 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    TM_QUEUE_ID_SNIF_DATA_Q_PRIORITY7,      /* This queue id is used for priority 7 frames that transmit to CNI0. This queue id is only valid for TM_sched_simple and TM_sched_simple_powersave mode. */
    
    TM_QUEUE_ID_PON0_VOIP,           /* Queue for voip on PON0. This queue id is valid for TM_sched_advance.*/
    TM_QUEUE_ID_PON1_VOIP,           /* Queue for voip on PON1. This queue id is valid for TM_sched_advance. */

    TM_QUEUE_ID_CNI0_VOIP,           /* Queue for voip on PON0. This queue id is valid for TM_sched_advance.*/
    TM_QUEUE_ID_CNI1_VOIP,           /* Queue for voip on PON1. This queue id is valid for TM_sched_advance. */

    TM_QUEUE_ID_MULTICAST   = 0x400100,   /* Multicast queue. */
    TM_QUEUE_ID_BROADCAST   = 0x400000,   /* Broadcast queue. */
    
}TM_queue_id_e;

typedef enum
{
  GW10G_PON_1G_ONU				= 0,
  GW10G_PON_ASYMMETRIC_10G_ONU	= 1,
  GW10G_PON_1G_DS_XG_US_ONU		= 2, /* Not in use (no such onu), defined only for compatibility with ONU FW enum */
  GW10G_PON_SYMMETRIC_10G_ONU		= 3,
  GW10G_PON_UNKNOWN_SPEED_ONU		= 4, /*The ONU speed is unknown, used only when set static LLID assignment*/
} GW10G_PON_onu_type_t;

typedef struct GW10G_PON_registration_data_t
{
    mac_address_t				   mac_address;           
    PON_authentication_sequence_t  authentication_sequence;
    OAM_standard_version_t		   supported_oam_standard; 
    GW10G_PON_onu_type_t                 onu_type;

} GW10G_PON_registration_data_t;


typedef enum GW10G_PON_cni_id_e
{
	GW10G_PON_PORT_ID_CNI0 = 0,
	GW10G_PON_PORT_ID_CNI1
} GW10G_PON_cni_id_e; 

typedef enum GW10G_PON_link_status_e
{
	GW10G_PON_LINK_STATUS_DOWN = 0,
    GW10G_PON_LINK_STATUS_UP
} GW10G_PON_link_status_e; 

/* Handler functions enum */
typedef enum
{
	GW10G_PAS_HANDLER_FRAME_RECEIVED,					        /* Ethernet frame originated from OLT's PON / System    */
												        /* port was received								    */
	GW10G_PAS_HANDLER_EXTENDED_FRAME_RECEIVED,				/* Ethernet frame with shim header originated from		*/
												        /* OLT's PON / System port was received					*/
	GW10G_PAS_HANDLER_ONU_REGISTRATION,				        /* ONU registration to the PON					        */
	GW10G_PAS_HANDLER_ALARM,							        /* Hardware / Software alarm					        */
	GW10G_PAS_HANDLER_OLT_RESET,						        /* OLT reset 									        */
	GW10G_PAS_HANDLER_ONU_DEREGISTRATION,				        /* ONU deregistration (disconnection) from the PON      */
	GW10G_PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,           /* Acknowledge for GW10G_PAS_start_encryption function        */
	GW10G_PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,      /*Acknowledge for GW10G_PAS_update_encryption_key function    */
	GW10G_PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,	        /* Acknowledge for GW10G_PAS_stop_encryption function	        */
	GW10G_PAS_HANDLER_LOAD_OLT_BINARY_COMPLETED,		        /* Load OLT binary process completed				    */
	GW10G_PAS_HANDLER_DBA_EVENT,						        /* Event originating from DBA algorithm			        */
    GW10G_PAS_HANDLER_RADIUS_MESSAGE_RECEIVED,		        /* RADIUS message originated from OLT received	        */
	GW10G_PAS_HANDLER_ONU_AUTHORIZATION,				        /* Authorization mode update of a single ONU		    */
    GW10G_PAS_HANDLER_EVENT_LOG,                              /* Received event log message                           */
    GW10G_PAS_HANDLER_PON_LOSS,                               /* PON loss                                             */
    GW10G_PAS_HANDLER_ENCRYPTION_RESULT,                      /* There is a result in the encryption                  */
	GW10G_PAS_HANDLER_AUTHENTICATION_KEY_RECEIVED,		    /* Authentication key received	  			            */
    GW10G_PAS_HANDLER_OLT_ADD,								/* OLT add										        */
	GW10G_PAS_HANDLER_OLT_REMOVE,								/* OLT remove									        */
	GW10G_PAS_HANDLER_ONU_DENIED,								/* Denied ONU 											*/
	GW10G_PAS_HANDLER_CONVERT_RSSI_TO_DBM,					/* Convert RSSI value to dBm units						*/
	GW10G_PAS_HANDLER_GET_OLT_TEMPERATURE,					/* Retrieve OLT temperature 							*/
    GW10G_PAS_HANDLER_CNI_LINK,                               /* CNI link                                             */
    GW10G_PAS_HANDLER_REDUNDANCY_OLT_FAILURE,                 /* redundancy OLT failure                               */
    GW10G_PAS_HANDLER_REDUNDANCY_SWITCH_OVER,                 /* redundancy switch over                               */
    GW10G_PAS_HANDLER_OTDR_END_MEASUREMENT,                   /* end of OTDR measurement                              */
	GW10G_PAS_HANDLER_TIME_OF_DAY,							/* Time Of Day event								    */
	GW10G_PAS_HANDLER_FW_HOST_INIT_DONE,						/* FW host init done event								*/
	GW10G_PAS_HANDLER_FW_INIT_DONE,							/* FW init done event								    */
	GW10G_PAS_HANDLER_DDR_ECC_ERROR,							/* DDR ECC Error event								    */
	GW10G_PAS_HANDLER_MIGRATION,							    /* Migration event								        */
	GW10G_PAS_HANDLER_MIGRATION_EXTEND,					    /* Extended Migration event								        */
    GW10G_PAS_HANDLER_OLT_DOWNLINK_FEC_CHANGED,           	/* If some OLT DOWNLINK FEC changed, trigger this event */
    GW10G_PAS_HANDLER_REDUNDANCY_OLT_FAILURE_WITH_EXTEND_INFO,/* redundancy OLT failure with extended infor           */
	GW10G_PAS_HANDLER_SCHED_TREE_COMMIT_RESULT,
	GW10G_PAS_HANDLER_GPIO_MODULE_EXIST_CHANGE,
	GW10G_PAS_HANDLER_PS_DISCOVERY_DONE,
	GW10G_PAS_HANDLER_TM_QUEUES_EMPTY,
	GW10G_PAS_HANDLER_MLD_ONU_CONFIG_STATUS,
	GW10G_PAS_HANDLER_MLD_ONU_FILTER_UPDATE,
	GW10G_PAS_HANDLER_MLD_OAM_MESSAGE_SEND,
	GW10G_PAS_HANDLER_OLT_HOST_RECOVERY,
	GW10G_PAS_HANDLER_PS_DL_EVENT,							/* Event originating from PS DL							*/		
	GW10G_PAS_HANDLER_OTDR_PON_TOPOLOGY_CHANGED,              /* OTDR detect that PON topology has changed            */
	GW10G_PAS_HANDLER_PS_FORCE_LLID,
	GW10G_PAS_HANDLER_MKA_KEY_CHANGE,
	GW10G_PAS_HANDLER_MKA_KEEP_ALIVE_RESULT,
	GW10G_PAS_HANDLER_XAUI_REDUNDANCY_SWITCH_OVER,

	/* for INTERNAL use */
	GW10G_PAS_HANDLER_REMOTE_ACCESS_ACKNOWLEDGE,				/* general ack originated from onu						*/
	GW10G_PAS_HANDLER_REMOTE_MDIO_ACCESS,						/* read MDIO register message originated from onu		*/
    GW10G_PAS_HANDLER_REMOTE_STATISTIC,                       /* response of the remote statistics from onu           */

    GW10G_PAS_HANDLER_LAST_HANDLER,

} GW10G_PAS_handler_functions_index_t;

typedef long  GW10G_PON_rtt_t;  /* C-type representation of RTT measurement (round trip time from    */
						  /* OLT to ONU and back, measured in TQ) */		
/* Structure containing the ONU originated data updated during ONU registration to the network. */
/* This data doesn't include ONU identifications: Id and MAC address							*/
typedef struct
{
	unsigned short int		laser_on_time;	/* Measured in TQ, range: 0 - (2^16-1) */
	unsigned short int		laser_off_time; /* Measured in TQ, range: 0 - (2^16-1) */
	OAM_standard_version_t  oam_version;
	bool					passave_onu;    /* Device contains one of Passave's ONU chips */
	GW10G_PON_rtt_t				rtt;
    unsigned /*char*/ long  session_id;
	GW10G_PON_onu_type_t          onu_type;
} GW10G_onu_registration_data_record_t;

/* Frame destination OLT port options */
typedef enum
{
	GW10G_PON_PORT_PON,					   /* PON port													   */
	GW10G_PON_PORT_SYSTEM,                   /* a.k.a. Network											   */
	GW10G_PON_PORT_SNIFF,					   /* Sniff port												   */
	GW10G_PON_PORT_PON_AND_SYSTEM,		   /* Both PON and System ports									   */
	GW10G_PON_PORT_SYSTEM_AND_SNIFF,		   /* Both system and sniff ports								   */
	GW10G_PON_PORT_PON_AND_SYSTEM_AND_SNIFF, /* Both PON, System and Sniff ports							   */
	GW10G_PON_PORT_AUTOMATIC,                /* Default selection of physical port by address table / Both   */
									   /* PON and System ports (if address not found in address table) */
	GW10G_PON_PORT_AUTOMATIC_INHIBIT_ALL,	   /* Default selection / discard if address not found in address  */
									   /* table)													   */
	GW10G_PON_PORT_AUTOMATIC_INHIBIT_PON,	   /* Default selection / System port (if address not found in     */
									   /* address table)											   */
	GW10G_PON_PORT_AUTOMATIC_INHIBIT_SYSTEM  /* Default selection / PON port (if address not found in address*/
									   /* table)													   */
} GW10G_PON_sent_frame_destination_port_t;

#endif
/*End:PAS 8411相关*/



/*Begin:DBA PLATO4相关*/
#if 1
#define PLATO4_ERR_OK                PAS_EXIT_OK
#define PLATO4_ERR_GENERAL_ERROR     PAS_EXIT_ERROR
#define PLATO4_ERR_PARAMETER_ERROR   PAS_PARAMETER_ERROR
#define PLATO4_ERR_DBA_NOT_LOADED    PAS_DBA_DLL_NOT_LOADED
#define PLATO4_ERR_NO_COMMUNICATION  PAS_TIME_OUT
#define PLATO4_ERR_OLT_NOT_EXIST     PAS_OLT_NOT_EXIST
#define PLATO4_ERR_DBA_NOT_RUNNING   PAS_DBA_NOT_RUNNING
#define PLATO4_ERR_ONU_NOT_AVAILABLE PAS_ONU_NOT_AVAILABLE

#define PLATO4_ECODE_NO_ERROR                    0
#define PLATO4_ECODE_UNKNOWN_LLID                1
#define PLATO4_ECODE_TOO_MANY_LLIDS              2
#define PLATO4_ECODE_ILLEGAL_LLID                3
#define PLATO4_ECODE_MIN_GREATER_THAN_MAX        4
#define PLATO4_ECODE_MIN_TOTAL_TOO_LARGE         5
#define PLATO4_ECODE_ILLEGAL_CLASS               6
#define PLATO4_ECODE_ILLEGAL_GR_BW               7
#define PLATO4_ECODE_ILLEGAL_BE_BW               8
#define PLATO4_ECODE_ILLEGAL_BURST               9
#define PLATO4_ECODE_ILLEGAL_FIXED_BW            10
#define PLATO4_ECODE_ILLEGAL_FIXED_PACKET_SIZE   11
#define PLATO4_ECODE_WRONG_DBA_TYPE              12
#define PLATO4_ECODE_ILLEGAL_DBA_TYPE            13
#define PLATO4_ECODE_UNSUPPORTED_DBA_TYPE        14
#define PLATO4_ECODE_ILLEGAL_WEIGHT              15
#define PLATO4_ECODE_WRONG_WEIGHTS_SUM           16
#define PLATO4_ECODE_ILLEGAL_BE_METHOD           17
#define PLATO4_ECODE_UNSUPPORTED_BE_METHOD       18
#define PLATO4_ECODE_ILLEGAL_MIN_SP              19

#define PLATO4_NUMBER_OF_SERVICES 4 /* The number of queues in the ONU */

#define PL_STATUS	short int

typedef enum {
  PLATO4_DBA_TYPE_GLOBAL,
  PLATO4_DBA_TYPE_SERVICE
} PLATO4_DBA_type_t;

typedef enum {
  PLATO4_BE_METHOD_STRICT,
  PLATO4_BE_METHOD_WFQ,
  PLATO4_BE_METHOD_SP_WFQ,
} PLATO4_SERVICE_BE_method_t;

 
#ifdef CPP_COMPILATION
	#define SLA_CLASS sla_class
#else
	#define SLA_CLASS class
#endif

typedef struct {
  unsigned short SLA_CLASS;         /* 0 (lowest priority) ?7 (highest priority) */
  unsigned short fixed_packet_size;	/* Packet size of fixed allocation (bytes) */
  unsigned short fixed_bw;			/* Fixed guaranteed bandwidth (1Mbps units)  */
  unsigned short fixed_bw_fine;     /* Fixed guaranteed bandwidth (64Kbps units)  */
  unsigned short max_gr_bw;         /* Max guaranteed bandwidth (1Mbps units)  */
  unsigned short max_gr_bw_fine;    /* Max guaranteed bandwidth (64Kbps units)  */
  unsigned short max_be_bw;         /* Max best effort bandwidth (1Mbps units) */
  unsigned short max_be_bw_fine;    /* Max best effort bandwidth (64Kbps units) */
} PLATO4_SLA_t;

typedef struct {
  PLATO4_SERVICE_BE_method_t best_effort_method;  /* The method in which best-effort is allocated among the services */
  unsigned char min_sp_service;  /* The lowest priorty which is still under SP policy (for SP+WFQ) */
  unsigned short tot_be_bw;      /* Total Best-effort bandwidth (1Mbps units) */
  unsigned short tot_be_bw_fine; /* Total Best-effort bandwidth (64Kbps units) */

  struct {
    unsigned short fixed_packet_size; /* Packet size of fixed allocation (bytes) */
    unsigned short fixed_bw;		  /* Fixed guaranteed bandwidth (1Mbps units)  */
    unsigned short fixed_bw_fine;     /* Fixed guaranteed bandwidth (64Kbps units)  */
    unsigned short gr_bw;             /* Guaranteed bandwidth (1Mbps units) */
    unsigned short gr_bw_fine;        /* Guaranteed bandwidth (64Kbps units) */
    unsigned short be_bw;             /* Best-effort bandwidth (1Mbps units) */
    unsigned short be_bw_fine;        /* Best-effort bandwidth (64Kbps units) */
    unsigned short wfq_weight;        /* WFQ weight (0-100, relevent only for the WFQ best-effort method) */
  } service_SLA[PLATO4_NUMBER_OF_SERVICES];
} PLATO4_SERVICE_SLA_t;

typedef struct {
  unsigned short cycle_length;        /* Cycle length in 50us units */
  unsigned short discovery_frequency; /* Disocvery frequency in 1ms units */
  unsigned short max_llids;           /* Maximum number of LLIDs expected */
} PLATO4_conf_t;

typedef struct {
  unsigned long rate;           /* 0 for 1G and 1 for 10G  */
  unsigned long fec;			/* 0 for FEC off and 1 for FEC on  */
  unsigned long length;			/* at least 42  */
} PLATO4_report_length_t;

typedef enum PLATO4_onu_type_e
{
	PLATO4_ONU_TYPE_DEFUALT,		
	PLATO4_ONU_TYPE_1G_NO_LOOP_TIMING,
	PLATO4_ONU_TYPE_10G_ASYM_NO_LOOP_TIMING
} PLATO4_onu_type_e;

typedef enum PLATO4_fairness_type_e
{
	FR_AVERAGE_BETWEEN_ONUS = 1,		/* same allocation to every LLID */
	FR_ASSURE_BW_RATIO    = 2,			/* based on GR only */
	FR_ASSURE_FIX_BW_RATIO = 3,			/* based on GR + FIX */
	FR_BEST_EFFORT_NO_FIX_RATIO = 4,	/* based on BE portion only */
} PLATO4_fairness_type_e;


extern PL_STATUS PLATO4_init( void );

extern PL_STATUS PLATO4_cleanup( void );

/* this function should be called by PAS_HANDLER_DBA_EVENT handler function
   when its olt_id parameter is an OLT which runs PLATO4 DBA */
extern PL_STATUS PLATO4_algorithm_event( short olt_id, unsigned short id, short size,
                                   const char *data );
          
extern PL_STATUS PLATO4_set_DBA_type( short olt_id, short int LLID,
                                PLATO4_DBA_type_t DBA_type, short *DBA_error_code );
extern PL_STATUS PLATO4_get_DBA_type( short olt_id, short int LLID,
                                PLATO4_DBA_type_t *DBA_type, short *DBA_error_code );

extern PL_STATUS PLATO4_set_SLA( short olt_id, short int LLID,
                           const PLATO4_SLA_t *SLA, short *DBA_error_code );
extern PL_STATUS PLATO4_get_SLA( short olt_id, short int LLID,
                                 PLATO4_SLA_t *SLA, short *DBA_error_code );
/* PLATO4 handler functions enum */
typedef enum
{
	PLATO4_HANDLER_UPDATE_LLID_FLAGS,    /*Update llid_flags event*/
    PLATO4_HANDLER_LAST_HANDLER,

} PLATO4_handler_index_t;

extern PON_STATUS PLATO4_assign_handler_function
                ( const PLATO4_handler_index_t    handler_function_index, 
				  const void					  (*handler_function)(),
				  unsigned short                  *handler_id );

extern PON_STATUS PLATO4_delete_handler_function ( const unsigned short  handler_id );


extern PON_STATUS PLATO4_set_plato4_llid_flags( const PON_olt_id_t olt_id, const PON_onu_id_t onu_id,
                                         const unsigned short llid_flags);

extern PON_STATUS PLATO4_get_plato4_llid_flags( const PON_olt_id_t olt_id, const PON_onu_id_t onu_id,
                                         unsigned short *llid_flags);
#endif
/*End:DBA PLATO4相关*/

/*Begin:PAS 8411 API相关*/
#if 1

/*==================== external macros ==================*/
/* The macro NE_ACTION_DEFAULT set the NE_action_s structure to default values. */
/* The VLAN editing in this case is NE_VLAN_op_nop for both VLAN with swap_vlan FALSE */
#define NE_ACTION_DEFAULT(_act_) /* _act_ type is NE_action_s*/\
{\
    _act_.dp_queue_id					= TM_QUEUE_ID_INVALID;\
    _act_.sniff_queue_id				= TM_QUEUE_ID_INVALID;\
    _act_.edit.rule_type				= NE_EDIT_TYPE_NONE;\
	_act_.edit.rule.vlan.port_edit_id	= 0;\
	_act_.edit.rule.vlan.vlan_swap		= FALSE;\
	_act_.edit.rule.vlan.pr_op			= NE_VLAN_OP_NOP;\
	_act_.edit.rule.vlan.sc_op			= NE_VLAN_OP_NOP;\
    _act_.dp_rate_limit0				= 0;\
    _act_.dp_rate_limit1				= 0;\
    _act_.dp_rate_limit2				= 0;\
    _act_.cpu_flow_id					= 0;\
    _act_.ellid							= 0;\
}


/* The macro NE_USER_DATA_DEFAULT set the NE_action_s structure to default values. */
/* The VLAN editing in this case is NE_VLAN_op_nop for both VLAN with swap_vlan FALSE. */
/* The user_id set to USER_ID_UNUSED. */
#define NE_USER_DATA_DEFAULT(_user_data_) /* _user_data_ type is NE_user_data_s */\
{\
	/* _user_data_.id		= should be defined by the user */\
	/* _user_data_.values	= should be defined by the user */\
	_user_data_.user_id		= USER_ID_UNUSED;\
	NE_ACTION_DEFAULT(_user_data_.action);\
}


/* The macro NE_USER_DATA_VLAN set the NE_action_s structure to default values except the edit type that set to NE_edit_type_vlan. */
/* The VLAN editing in this case is NE_VLAN_op_nop for both VLAN with swap_vlan FALSE. */
/* The user_id set to USER_ID_UNUSED. */
#define NE_USER_DATA_VLAN(_user_data_) /* _user_data_ type is NE_user_data_s */\
{\
	/* _user_data_.id		= should be defined by the user */\
	/* _user_data_.values	= should be defined by the user */\
	_user_data_.user_id		= USER_ID_UNUSED;\
	NE_ACTION_DEFAULT(_user_data_.action);\
	_user_data_.action.edit.rule_type	= NE_EDIT_TYPE_VLAN;\
}


extern PON_module_state_t GW10G_Get_pas_soft_state ( void );
extern PON_STATUS GW10G_PAS_get_ddr_configuration
     		( 	const PON_olt_id_t 				olt_id,
       			const GW10G_PON_ddr_interface_type_e 	interface_type,
       			GW10G_PON_ddr_parameters_s			*ddr_params);

extern PON_STATUS GW10G_PAS_get_ref_clock ( const PON_olt_id_t	  olt_id,
								     GW10G_PON_ref_clock_e  *ref_clock );

extern PON_STATUS GW10G_PAS_get_olt_ne_mode ( const PON_olt_id_t		olt_id,
									   NE_ne_mode_e		*ne_mode );



extern PON_STATUS GW10G_PAS_get_address_table
                             (const PON_olt_id_t         olt_id,
                              const INT32U               start_index,
                              const INT32S               end_index,
                              const INT32U               max_count,
                                    INT32U              *count,
                                    NE_addr_entry_s     *search,
                                    NE_addr_entry_s     *entries );

extern PON_STATUS GW10G_PAS_add_address_table_record 
                              (const PON_olt_id_t         olt_id,
                               const NE_addr_entry_s     *entry );

extern PON_STATUS GW10G_PAS_get_olt_pon_module_exist(const PON_olt_id_t			olt_id,
										const PON_gpio_grp_level_e	group_level,
											  bool					*exist);



extern PON_STATUS GW10G_PAS_reset_olt_statistic(const PON_olt_id_t olt_id,
										 unsigned int *count,		
										 STAT_reset_field_t *stat_entry);

extern PON_STATUS GW10G_PAS_get_olt_statistic(const	PON_olt_id_t olt_id,
										unsigned int *count,		
										STAT_field_t *stat_entry);



extern PON_STATUS GW10G_PAS_set_ethernet_cni_mac_mode ( const PON_olt_id_t           olt_id,
										   const TM_port_e				cni_port,
										   const GW10G_PON_cni_config_type_e  cni_conf_type,
										   const GW10G_PON_cni_config_value_u cni_conf_value );

extern PON_STATUS GW10G_PAS_get_ethernet_cni_mac_mode ( const PON_olt_id_t			olt_id,
										   const TM_port_e				cni_port,
										   const GW10G_PON_cni_config_type_e  cni_conf_type,
										   GW10G_PON_cni_config_value_u		*cni_conf_value );



extern PON_STATUS GW10G_PAS_NE_service_map_create
                                (const PON_olt_id_t             olt_id,
                                 const TM_port_e                port,
                                 const NE_service_map_type_e    map_type,
                                 const NE_service_map_data_s    values,
                                 INT32U                         *service_map_id );
extern PON_STATUS GW10G_PAS_TM_policy_profile_create(const PON_olt_id_t    olt_id,
                                              TM_policy_s     *policy_profile );

extern PON_STATUS GW10G_PAS_TM_policy_profile_set
                                    (const PON_olt_id_t		olt_id,
                                     const TM_policy_s		*policy_profile );

extern PON_STATUS GW10G_PAS_TM_rate_limit_set(const PON_olt_id_t        olt_id,
                                 const NE_direction_e      direction,
                                 const INT32U              policy_id,
                                 const INT32U              rate_limit_id );
extern PON_STATUS GW10G_PAS_NE_user_rule_add
                           (const PON_olt_id_t          olt_id,
					        const NE_user_data_s 		*data,
					        INT32U 					    *rule_id );

extern PON_STATUS GW10G_PAS_ne_attrib_group_create
                                         (const PON_olt_id_t         olt_id,
                                          const NE_direction_e       direction,
                                          const NE_attributes_s     *attribs,
                                                INT32U              *id );
extern PON_STATUS GW10G_PAS_ne_action_set
                                (const PON_olt_id_t           olt_id,
                                 const INT32U                 id,
                                 const NE_action_type_e       act_type,
                                 const INT32U                 priority,
                                 const NE_action_value_u      value );


#ifdef SYS_DUMP
extern PON_STATUS GW10G_PAS_sys_dump(const PON_sys_dump_type_t	dump_type, 
					   PON_olt_id_t					olt_id, 
					   PON_onu_id_t					onu_id);
#endif

extern PON_STATUS GW10G_PAS_get_olt_versions
								(const PON_olt_id_t      olt_id,
								 GW10G_PAS_device_versions_t *olt_versions,
								 INT32U     *critical_events_counter,
								 INT32U     *non_critical_events_counter );

extern PON_STATUS GW10G_PAS_get_dba_version
                                          (const PON_olt_id_t         olt_id,
                                           PON_dba_version_t          *dba_version );

extern PON_STATUS GW10G_PAS_get_cni_link_status ( const PON_olt_id_t		olt_id,
       								 const TM_port_e 		cni_port,
       								 PON_link_status_e 	    *status);

extern PON_STATUS GW10G_PAS_set_olt_interface_mtu ( const PON_olt_id_t	olt_id,
									   const unsigned long	mtu );

extern PON_STATUS GW10G_PAS_get_olt_interface_mtu ( const PON_olt_id_t	olt_id,
											 unsigned long	*mtu );

extern PON_STATUS GW10G_PAS_NE_vlan_custom_set
                                 (const PON_olt_id_t        olt_id,
                                   const NE_direction_e      direction,
                                   const NE_VLAN_type_e      ether,
                                  const INT16U              value );

extern PON_STATUS GW10G_PAS_get_oam_configuration
                                    ( const PON_olt_id_t   olt_id,
                                            bool          *limitation );

extern PON_STATUS GW10GAdp_get_oam_information 
                                  ( const PON_olt_id_t      olt_id, 
								    const PON_onu_id_t	    onu_id,
								    PON_oam_information_t  *oam_information );

extern PON_STATUS GW10GAdp_set_llid_vlan_mode 
                                 (const PON_olt_id_t                      olt_id,
                                  const PON_llid_t                        llid,
                                  const PON_olt_vlan_uplink_config_t      vlan_uplink_config );

extern PON_STATUS GW10GAdp_get_llid_vlan_mode 
                                 (const PON_olt_id_t                      olt_id,
                                  const PON_llid_t                        llid,
                                        PON_olt_vlan_uplink_config_t     *vlan_uplink_config );

extern PON_STATUS GW10GAdp_set_classification_rule 
                                      (const PON_olt_id_t                             olt_id,
                                       const PON_pon_network_traffic_direction_t      direction,
                                       const PON_olt_classification_t                 classification_entity,
                                       const void                                    *classification_parameter,
                                       const PON_olt_classifier_destination_t         destination );
extern PON_STATUS GW10GAdp_get_classification_rule 
                                      (const PON_olt_id_t                             olt_id,
                                       const PON_pon_network_traffic_direction_t      direction,
                                       const PON_olt_classification_t                 classification_entity,
                                       const void                                    *classification_parameter,
                                             PON_olt_classifier_destination_t        *destination );

extern PON_STATUS GW10G_PAS_get_onu_parameters ( const PON_olt_id_t	  olt_id, 
								    short int			  *number, 
								    PAS_onu_parameters_t  onu_parameters );

extern PON_STATUS GW10GAdp_set_policing_thresholds 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_policer_t                  policer,
                                           const PON_high_priority_frames_t     high_priority_frames,
                                           const long                           high_priority_reserved );

extern PON_STATUS GW10G_PAS_set_address_table_configuration
                                              (const PON_olt_id_t          olt_id,
                                               const NE_addr_config_e      config_type,
                                               const INT32U                address_table_config );

extern PON_STATUS GW10G_PAS_get_address_table_configuration
                                             (const PON_olt_id_t          olt_id,
                                              const NE_addr_config_e      config_type,
                                                    INT32U               *address_table_config );

extern PON_STATUS GW10GAdp_get_address_table 
                                ( const PON_olt_id_t    olt_id, 
								  short int			   *active_records,	
								  PON_address_table_t   address_table );

extern PON_STATUS GW10GAdp_add_address_table_record 
                                       ( const PON_olt_id_t			  olt_id, 
										 const PON_address_record_t  *address_record );

extern PON_STATUS GW10GAdp_add_address_table_multi_records 
                                       ( const PON_olt_id_t			  olt_id, 
                                         const unsigned short		  num_of_records,
										 PON_address_table_t          address_table );

extern PON_STATUS GW10GAdp_remove_address_table_record 
                                          ( const PON_olt_id_t   olt_id, 
									        const mac_address_t  mac_address );

extern PON_STATUS GW10GAdp_reset_address_table
                                 ( const PON_olt_id_t	      olt_id ,
                                   const PON_llid_t           llid   ,
                                   const PON_address_aging_t  address_type
                                 );

extern PON_STATUS GW10GAdp_get_olt_optics_parameters 
                                    (const PON_olt_id_t                        olt_id,
                                           PON_olt_optics_configuration_t     *optics_configuration,
                                           bool                               *pon_tx_signal );
extern PON_STATUS GW10G_PAS_get_olt_device_optics( const PON_olt_id_t	 olt_id,
        							  const PON_pon_type     optics_type,
              						  PON_optics_param_u     *optics_param );

extern PON_STATUS GW10G_PAS_get_olt_pon_transmission(const PON_olt_id_t      olt_id,
										const PON_pon_type      pon_type,
											  bool              *mode);

extern PON_STATUS GW10G_PAS_update_olt_parameters
                                        (const PON_olt_id_t                olt_id,
                                         const GW10G_PON_update_olt_parameters_t *updated_parameters );

extern PON_STATUS GW10GAdp_get_raw_statistics 
                                 ( const PON_olt_id_t					 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   void								    *statistics_data,
								   PON_timestamp_t						*timestamp );

extern short int GW10G_PAS_reset_olt_counters (const PON_olt_id_t olt_id);

extern PON_STATUS GW10G_PAS_set_alarm_configuration 
                                    ( const PON_olt_id_t    olt_id, 
									  const short int	    source,
									  const PON_alarm_t     type,
									  const bool			activate,
									  const void		   *configuration );
extern PON_STATUS GW10G_PAS_get_alarm_configuration 
                                     ( const PON_olt_id_t olt_id, 
									   const short int	  source,
									   const PON_alarm_t  type,
									   bool			      *activated,
									   void		          *configuration );

extern PON_STATUS GW10GAdp_start_olt_encryption 
                                ( const PON_olt_id_t	 olt_id, 
								  const PON_llid_t	     llid );

extern PON_STATUS GW10GAdp_finalize_start_olt_encryption 
                                        ( const PON_olt_id_t   olt_id, 
										  const PON_llid_t	   llid,
										  const short int      status );

extern PON_STATUS GW10GAdp_stop_olt_encryption 
                                ( const PON_olt_id_t	 olt_id, 
								  const PON_llid_t	     llid );

extern PON_STATUS GW10GAdp_set_olt_encryption_key 
                                   ( const PON_olt_id_t		     olt_id,           
                                     const PON_llid_t		     llid,             
                                     const PON_encryption_key_t  encryption_key );

extern PON_STATUS GW10GAdp_finalize_set_olt_encryption_key
                                            ( const PON_olt_id_t  olt_id, 
											  const PON_llid_t	  llid,
											  const short int     status );

extern short int GW10G_Get_encryption_mode ( const short int	   olt_id, 
								const PON_llid_t   llid,
								bool			  *encryption_active );

extern PON_STATUS GW10G_PAS_set_llid_fec_mode
                                    (const PON_olt_id_t    olt_id,
                                     const PON_llid_t      llid,
                                     const bool            downlink_fec );

extern PON_STATUS GW10G_PAS_get_llid_fec_mode
                                   (const PON_olt_id_t    olt_id,
                                    const PON_llid_t      llid,
                                          bool           *downlink_fec,
                                          bool           *uplink_fec,
                                          bool           *last_uplink_frame_fec);

extern short int GW10G_Get_onu_mac_address ( const short int	    olt_id, 
								const PON_onu_id_t  onu_id,
								mac_address_t       onu_mac );

extern PON_STATUS GW10G_PAS_get_onu_register_info_from_fw (const PON_olt_id_t  olt_id, 
                                               const PON_onu_id_t  onu_id,
                                               PON_redundancy_onu_register_t*   onu_register);

extern PON_STATUS GW10G_PAS_get_onu_mode 
                           ( const PON_olt_id_t  olt_id, 
							 const PON_onu_id_t  onu_id );

extern PON_STATUS GW10G_PAS_deregister_onu 
                             ( const PON_olt_id_t  olt_id, 
							   const PON_onu_id_t  onu_id,
							   const bool		   wait_for_deregistration_completion );

extern PON_STATUS GW10GAdp_set_p2p_access_control 
                                     ( const PON_olt_id_t   olt_id, 
									   const PON_llid_t     configured_llid,
									   const short int      number_of_llids,
									   const PON_llid_t     llids[],
									   const bool		    access );

extern PON_STATUS GW10GAdp_set_llid_p2p_configuration 
                                         ( const PON_olt_id_t  olt_id, 
										   const PON_llid_t    llid,
										   const bool		   address_not_found,
										   const bool		   broadcast );

extern PON_STATUS GW10GAdp_set_policing_parameters 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_llid_t                     llid,
                                           const PON_policer_t                  policer,
                                           const bool                           enable,
                                           const PON_policing_parameters_t      policing_params     );

extern PON_STATUS GW10GAdp_get_policing_parameters 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_llid_t                     llid,
                                           const PON_policer_t                  policer,
                                                 bool                          *enable,
                                                 PON_policing_parameters_t     *policing_params     );

extern PON_STATUS GW10G_PAS_get_llid_parameters
                                     (const PON_olt_id_t                olt_id,
                                      const PON_llid_t                  llid,
									        PON_llid_parameters_t      *llid_parameters 
											);

extern PON_STATUS GW10G_PAS_start_encryption 
                               ( const PON_olt_id_t			       olt_id, 
								 const PON_llid_t				   llid,
								 const PON_encryption_direction_t  direction );

extern PON_STATUS GW10G_PAS_stop_encryption 
                              ( const PON_olt_id_t   olt_id, 
						        const PON_llid_t     llid );

extern PON_STATUS GW10G_PAS_update_encryption_key 
                                    ( const PON_olt_id_t				 olt_id, 
									  const PON_llid_t					 llid,
									  const PON_encryption_key_t		 encryption_key,
									  const	PON_encryption_key_update_t  key_update_method );

extern PON_STATUS GW10GAdp_get_address_table_llid 
                ( const PON_olt_id_t    olt_id,
                  const PON_onu_id_t    onu_id,
				  short int			   *active_records,	
				  PON_address_table_t   address_table );

extern PON_STATUS GW10G_PAS_set_port_mac_limiting
                                          (const PON_olt_id_t		olt_id,
                                           const TM_port_e			port,
                                           const PON_llid_t			llid,
                                           const INT32U				maximum_entries );

extern PON_STATUS GW10GAdp_get_onu_uni_port_mac_configuration ( 
									const PON_olt_id_t					   olt_id, 
									const PON_onu_id_t					   onu_id,
									PON_oam_uni_port_mac_configuration_t  *mac_configuration );

extern PON_STATUS GW10G_PAS_get_onu_registration_data 
                                        ( const PON_olt_id_t   		       olt_id, 
										  const PON_onu_id_t			   onu_id,
										  GW10G_onu_registration_data_record_t  *onu_registration_data  );

extern PON_STATUS GW10G_PAS_get_onu_version ( const PON_olt_id_t           olt_id,
                                const PON_onu_id_t		     onu_id,
                                PON_onu_versions            *onu_versions);

extern short int GW10G_Remove_olt ( const short int  olt_id,
					   const bool		send_shutdown_msg_to_olt, 
					   const bool       reset_olt);

extern PON_STATUS GW10G_PAS_pre_init_simple_operation_add(const PON_olt_id_t					olt_id, 
											 const INT32U						simple_operation_type,
											 const INT32U						in_val);

extern PON_STATUS GW10G_PAS_set_ddr_configuration
     		( 	const PON_olt_id_t 				olt_id,
       			const PON_ddr_interface_type_e 	interface_type,
       			const PON_ddr_parameters_s		*ddr_params);

extern PON_STATUS GW10G_PAS_set_ref_clock ( const PON_olt_id_t	  olt_id,
							   const PON_ref_clock_e  ref_clock );

extern PON_STATUS GW10G_PAS_set_olt_ne_mode ( const PON_olt_id_t		olt_id,
								 const NE_ne_mode_e		ne_mode );

extern PON_STATUS GW10G_PAS_set_olt_flavor_edk_type(const  PON_olt_id_t					olt_id, 
									   const  GW10G_PON_edk_type_e				flavor);

extern PON_STATUS GW10G_PAS_add_olt 
                    ( const PON_olt_id_t  				          olt_id, 
				      const PON_olt_initialization_parameters_t  *olt_initialization_parameters,
                            PON_add_olt_result_t                 *add_olt_result );

extern PON_STATUS GW10G_PAS_set_gpio_group_mapping (	const PON_olt_id_t				olt_id,
										const PON_gpio_grp_type_e		gpio_grp_type,
										const PON_gpio_grp_level_e		gpio_grp_pon_level,
										const PON_gpio_grp_map_value_u	*gpio_grp_map_val);

extern PON_STATUS GW10G_PAS_gpio_access 
                              ( const PON_olt_id_t			olt_id, 
							    const PON_gpio_lines_t		line_number,
							    const PON_gpio_line_io_t    set_direction,
							    const short int		  	    set_value,
							    PON_gpio_line_io_t		    *direction,
							    bool					    *value );

extern PON_STATUS GW10G_PAS_TM_sched_mode_set(	 
                    const PON_olt_id_t		    olt_id,
                    const TM_sched_model_e	    model,
                    const TM_sched_data_u	    *data );

extern PON_STATUS GW10G_PAS_TM_operation(	 
                    const PON_olt_id_t		    olt_id,
                    const TM_operation_e		operation, 
                    const INT32U				value );

extern PON_STATUS GW10G_PAS_set_olt_device_optics( const PON_olt_id_t   		olt_id,
        							  const PON_pon_type        optics_type,
              						  const PON_optics_param_u	*optics_param );


/*以下为设计文档中未列出的接口*/

extern PON_STATUS GW10G_PAS_write_mdio_register 
                                  ( const PON_olt_id_t		  olt_id, 
									const short int			  phy_address, 
									const short int			  reg_address, 
									const unsigned short int  value );

extern PON_LONG_STATUS GW10G_PAS_read_mdio_register 
                                       ( const PON_olt_id_t    olt_id, 
							             const short int       phy_address, 
							             const short int       reg_address,
							             unsigned short int   *value);

extern PON_STATUS GW10G_PAS_get_mac_address_authentication
                ( const PON_olt_id_t	 olt_id,
				  unsigned char			*number_of_mac_address,
				  mac_addresses_list_t   mac_addresses_list);

extern PON_STATUS GW10GAdp_send_frame
                             (const PON_olt_id_t                           olt_id,
                              const short int                              length,
                              const PON_sent_frame_destination_port_t      destination_port,
                              const PON_llid_t                             llid,
                              const PON_broadcast_type_e                   broadcast_type,
                              const void *                                 content );

extern PON_STATUS GW10GAdp_get_statistics
                             ( const PON_olt_id_t		olt_id, 
							   const short int			collector_id, 
							   const PON_statistics_t   statistics_type, 
							   const short int		    statistics_parameter, 
							   long double			   *statistics_data );

extern PON_STATUS GW10G_PAS_set_current_interface(const PON_olt_id_t         olt_id,
									       PON_comm_type_t comm_interface);

extern PON_STATUS GW10G_PAS_get_system_parameters ( PAS_system_parameters_t  *system_parameters );

extern PON_STATUS GW10G_PAS_set_system_parameters ( const PAS_system_parameters_t  *system_parameters );

extern PON_STATUS GW10G_PAS_set_olt_pon_transmission
                                       (const PON_olt_id_t      olt_id,
                                        const PON_pon_type      pon_type,
                                        const bool              mode );

extern PON_STATUS GW10G_PAS_set_authorize_mac_address_according_list_mode
                ( const PON_olt_id_t  olt_id,
				  const bool		  authentication_according_to_list);

extern PON_STATUS GW10G_PAS_set_encryption_configuration(const PON_olt_id_t			olt_id,
                                            const PON_encryption_type_t	encryption_type);

extern PON_STATUS GW10G_PAS_get_encryption_configuration(      
                  const PON_olt_id_t      olt_id,
                  PON_encryption_type_t  *encryption_type );

extern PON_STATUS GW10G_PAS_authorize_onu 
                            ( const PON_olt_id_t		  olt_id, 
							  const PON_onu_id_t		  onu_id,
							  const PON_authorize_mode_t  authorize_mode );

extern PON_STATUS GW10G_PAS_link_test 
                        ( const PON_olt_id_t						 olt_id, 
						  const PON_llid_t							 llid,
						  const short int					         number_of_frames,
						  const short int							 frame_size,
						  const bool								 link_delay_measurement,
						  const PON_link_test_vlan_configuration_t  *vlan_configuration,	
						  PON_link_test_results_t					*test_results );

extern PON_STATUS GW10G_PAS_set_standard_onu_loopback (
                                   	       const PON_olt_id_t      olt_id,
                                           const PON_onu_id_t      onu_id,
                                           const bool              loopback_mode );

extern PON_STATUS GW10G_PAS_start_dba_algorithm 
                                  ( const PON_olt_id_t   olt_id,
								    const bool		     use_default_dba,
									const short int	     initialization_data_size,
									const void		    *initialization_data );

extern PON_STATUS GW10G_PAS_get_authorize_mac_address_according_list_mode
                ( const PON_olt_id_t  olt_id,
				  bool				  *authentication_according_to_list);

extern PON_STATUS GW10G_PAS_init ( void );

extern PON_STATUS GW10G_PAS_terminate ( void );

extern PON_STATUS GW10G_PAS_get_olt_parameters (const PON_olt_id_t                 olt_id,
							       PON_new_olt_response_parameters_t  *olt_parameters);
extern PON_STATUS GW10GAdp_set_olt_register 
                            ( const PON_olt_id_t   olt_id,
                              const unsigned long  register_address, 
		                      const unsigned long  register_value );

extern PON_STATUS GW10GAdp_get_olt_register 
                            ( const PON_olt_id_t    olt_id,
                              const unsigned long   register_address, 
				              unsigned long        *register_value );

extern PON_STATUS GW10G_PAS_set_oam_configuration (const PON_olt_id_t olt_id,
                                      const bool         ignore_oam_limit );

extern PON_STATUS GW10G_PAS_reset_olt ( const PON_olt_id_t  olt_id );

extern PON_STATUS GW10G_PAS_send_cli_command 
                ( const PON_olt_id_t     olt_id,
                  const unsigned short   size,
                  const unsigned char   *command );

extern PON_STATUS GW10G_PAS_assign_handler_function
                                     ( const PAS_handler_functions_index_t    handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );

extern PON_STATUS GW10GAdp_set_olt_cni_port_mac_configuration 
                                    ( const PON_olt_id_t 	 				       olt_id,
									  const PON_olt_cni_port_mac_configuration_t  *olt_cni_port_mac_configuration );

extern PON_STATUS GW10GAdp_get_olt_cni_port_mac_configuration 
                                               ( const PON_olt_id_t					   olt_id, 
											     PON_olt_cni_port_mac_configuration_t  *olt_cni_port_mac_configuration );

extern PON_STATUS GW10GAdp_set_olt_igmp_snooping_mode 
                                         (const PON_olt_id_t                      olt_id,
                                          const PON_olt_igmp_configuration_t      igmp_configuration );

extern PON_STATUS GW10GAdp_get_olt_igmp_snooping_mode 
                                         (const PON_olt_id_t                      olt_id,
                                                PON_olt_igmp_configuration_t     *igmp_configuration );

extern PON_STATUS GW10GAdp_set_address_table_full_handling_mode
                ( const PON_olt_id_t   olt_id,
				  const bool		   remove_entry_when_table_full);

extern PON_STATUS GW10G_PAS_set_dba_report_format(      
                  const PON_olt_id_t             olt_id,
                  const PON_DBA_report_format_t  report_format );

extern PON_STATUS GW10G_PAS_get_dba_report_format(      
                  const PON_olt_id_t        olt_id,
                  PON_DBA_report_format_t  *report_format );

extern PON_STATUS GW10G_PAS_set_encryption_preamble_mode 
                                           (const PON_olt_id_t      olt_id,
                                            const bool              encryption_mode );

extern PON_STATUS GW10G_PAS_get_encryption_preamble_mode 
                                           (const PON_olt_id_t      olt_id,
                                                  bool             *encryption_mode );

extern PON_STATUS GW10GAdp_set_olt_mld_forwarding_mode 
                                        (const PON_olt_id_t                     olt_id,
                                         const disable_enable_t                 mode );

extern PON_STATUS GW10GAdp_get_olt_mld_forwarding_mode 
                                        (const PON_olt_id_t                     olt_id,
                                               disable_enable_t                *mode );

extern PON_STATUS GW10G_PAS_load_olt_binary
                             ( const PON_olt_id_t		   olt_id, 
		  					   const PON_indexed_binary_t  *olt_binary );

extern PON_TEST_RESULT GW10G_PAS_olt_self_test ( const PON_olt_id_t  olt_id );

extern PON_STATUS GW10G_PAS_read_i2c_register (const PON_olt_id_t	olt_id, 
								  const INT8U		    device_address, 
								  const INT8U		    register_address, 
								  INT8U			        *data);

extern PON_STATUS GW10G_PAS_set_virtual_scope_adc_config
                                            (const PON_olt_id_t        olt_id,
                                             const PON_adc_config_t    adc_config);

extern PON_STATUS GW10G_PAS_get_virtual_scope_measurement 
                                              (const PON_olt_id_t                olt_id,
                                               const PON_llid_t                  llid,
                                               const PON_measurement_type_t      measurement_type,
                                               const void *                      configuration, 
                                                     void *                      result         );

extern PON_STATUS GW10G_PAS_get_virtual_scope_rssi_measurement
                                            (const PON_olt_id_t                                olt_id,
                                             const PON_llid_t                                  llid,
                                             PON_rssi_result_t                                 *rssi_result);

extern PON_STATUS GW10G_PAS_set_virtual_llid
                              (const PON_olt_id_t                 olt_id,
                               const PON_llid_t                   llid,
                               const PON_virtual_llid_operation_t operation);

extern short int GW10GAdp_get_llid_parameters_seq ( const PON_olt_id_t	            olt_id, 
		 					    const PON_llid_t	            llid, 
		    					PON_llid_parameters_t          *llid_parameters, 
                                onu_registration_data_record_t *registration_data,
                                PON_authentication_sequence_t  *authentication_sequence);

extern bool GW10G_Olt_exists ( const short int olt_id );

extern bool GW10G_Onu_exist ( const short int     olt_id, 
				 const PON_onu_id_t  onu_id );

extern PON_STATUS GW10G_PAS_olt_host_recovery 
                                   ( const PON_olt_id_t		              olt_id,
									 PON_olt_mode_t			              *olt_mode,
									 PON_olt_initialization_parameters_t  *initialization_configuration, 
									 unsigned short int		              *dba_mode, 
									 PON_llid_t				              *link_test,
									 PON_active_llids_t		              *active_llids );

extern PON_STATUS GW10GAdp_get_olt_mode_query
                                 ( const PON_olt_id_t		    olt_id,
								   PON_olt_mode_t			   *olt_mode,
								   mac_address_t			    mac_address,
								   PON_olt_init_parameters_t   *initialization_configuration, 
								   unsigned short int		   *dba_mode, 
								   PON_llid_t				   *link_test,  
								   PON_active_llids_t		   *active_llids );

extern short int GW10G_Get_device_versions ( const short int			    olt_id, 
								const PON_device_id_t	    device_id,
								PAS_device_versions_t      *device_versions,
                                PON_remote_mgnt_version_t  *remote_mgnt_version,
                                unsigned long int          *critical_events_counter);

extern short int GW10G_PASCOMM_init_olt ( const short int  olt_id);

extern void GW10G_Monitor_enable(bool status);

extern int GW10G_Receive_ponframe_thread_func1(short int olt_id, void *lpPacketRecBuf,unsigned int buf_data_size);

extern char *GW10G_PON_boolean_string ( const bool  parameter );

extern short int GW10G_virtual_scope_get_onu_voltage(PON_olt_id_t olt_id, PON_onu_id_t onu_id, float *voltage,unsigned short int *sample, float *dbm);

extern int GW10G_Receive_polling_thread_func1(void *lpPacketRecBuf,unsigned int buf_data_size);



/*以下为PAS redundancy相关*/
extern PON_STATUS GW10G_PAS_get_redundancy_config
                                    (const PON_olt_id_t            olt_id,
                                     PON_redundancy_olt_state_t   *olt_state,
                                     PON_gpio_lines_t              *gpio_number,
                                     PON_redundancy_type_t        *type,
                                     bool                         *pon_rx_enabled);

extern PON_STATUS GW10G_PAS_redundancy_onu_register
                                    (const PON_olt_id_t                   olt_id,
                                     const PON_redundancy_onu_register_t  onu_register);

extern PON_STATUS GW10G_PAS_set_redundancy_config
                                    (const PON_olt_id_t                 olt_id,
                                     const PON_redundancy_olt_state_t   olt_state,
                                     const PON_gpio_lines_t             gpio_number,
                                     const PON_redundancy_type_t        type,
                                     const bool                         pon_rx_enabled);

extern PON_STATUS GW10G_PAS_redundancy_switch_over
                                    (const PON_olt_id_t                   olt_id);

extern PON_STATUS GW10G_PAS_get_redundancy_address_table 
                                           ( const PON_olt_id_t            olt_id, 
											 short int			          *active_records,	
											 PON_address_table_t           address_table );


#endif
/*End:PAS 8411 API相关*/

/***Begin:remote by jinhl**************/
#if 1
PON_STATUS GW10G_REMOTE_PASONU_address_table_clear( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id);

PON_STATUS GW10G_REMOTE_PASONU_classifier_add_source_address_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );

PON_STATUS GW10G_REMOTE_PASONU_classifier_remove_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address);

PON_STATUS GW10G_REMOTE_PASONU_classifier_l3l4_add_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num,
                                   const PON_forwarding_action_t              action );
PON_STATUS GW10G_REMOTE_PASONU_classifier_l3l4_remove_filter ( 
                                   const PON_olt_id_t                         olt_id,
                                   const PON_onu_id_t                         onu_id,
                                   const PON_pon_network_traffic_direction_t  direction,
                                   const PON_traffic_qualifier_t              traffic,    
                                   const PON_traffic_address_t                address,    
                                   const unsigned long                        ip_address, 
                                   const unsigned short int                   l4_port_num);

PON_STATUS GW10G_REMOTE_PASONU_classifier_add_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value,
                const PON_forwarding_action_t               action );

PON_STATUS GW10G_REMOTE_PASONU_clear_all_statistics( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id);

PON_STATUS GW10G_REMOTE_PASONU_address_table_get( 
                                   const PON_olt_id_t               olt_id,
                                   const PON_onu_id_t               onu_id,
                                   long int                        *num_of_entries,
                                   PON_onu_address_table_record_t  *address_table   );

PON_STATUS GW10G_REMOTE_PASONU_uni_get_port_configuration( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_uni_configuration_t  *uni_configuration );

PON_STATUS GW10G_REMOTE_PASONU_reset_device ( 
                          const PON_olt_id_t        olt_id, 
                          const PON_onu_id_t        onu_id,
                          const PON_reset_reason_t  reason);

PON_STATUS GW10G_REMOTE_PASONU_eeprom_mapper_set_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
								const void                   *data,
                                const unsigned long           size);

PON_STATUS GW10G_REMOTE_PASONU_eeprom_mapper_get_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
                                void                         *data,
								unsigned long                *size);

/*以下为之前未统计到设计文档中的接口*/
PON_STATUS GW10G_REMOTE_PASONU_update_onu_firmware (
                                const PON_olt_id_t    olt_id, 
								const PON_onu_id_t    onu_id,
								const PON_binary_t   *onu_firmware);

PON_STATUS GW10G_REMOTE_PASONU_get_burn_image_complete (
                                const PON_olt_id_t   olt_id, 
								const PON_onu_id_t   onu_id,
                                const bool          *complete);

PON_STATUS GW10G_REMOTE_PASONU_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable,
                                   const unsigned long  max_age);

PON_STATUS GW10G_REMOTE_PASONU_address_table_get_number_of_entries( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   long                *number_of_entries);

PON_STATUS GW10G_REMOTE_PASONU_encryption_get_state ( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   PON_encryption_direction_t  *direction);

PON_STATUS GW10G_REMOTE_PASONU_encryption_set_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_t         encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index);

PON_STATUS GW10G_REMOTE_PASONU_get_device_version( 
                                   const PON_olt_id_t     olt_id, 
                                   const PON_onu_id_t     onu_id,
                                   PON_device_version_t  *versions);

PON_STATUS GW10G_REMOTE_PASONU_get_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PAS_device_versions_t  *device_versions );

PON_STATUS GW10G_REMOTE_PASONU_uni_set_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable_cpu, 
                                   const bool           enable_datapath );

PON_STATUS GW10G_REMOTE_PASONU_set_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable);

PON_STATUS GW10G_REMOTE_PASONU_init ( void );

PON_STATUS GW10G_REMOTE_PASONU_classifier_add_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address,
                                   const PON_forwarding_action_t   action );

PON_STATUS GW10G_REMOTE_PASONU_classifier_remove_filter ( 
                const PON_olt_id_t                          olt_id, 
                const PON_onu_id_t                          onu_id,
                const PON_pon_network_traffic_direction_t   direction,
                const PON_frame_qualifier_t                 qualifier,
                const unsigned short                        value );
#endif
/***End:remote by jinhl**************/

/***Begin:CTC by jinhl**************/
#if 1
PON_STATUS GW10G_CTC_STACK_get_fec_ability ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   CTC_STACK_standard_FEC_ability_t  *fec_ability );

PON_STATUS GW10G_CTC_STACK_set_fec_mode ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   const CTC_STACK_standard_FEC_mode_t  fec_mode);

PON_STATUS GW10G_CTC_STACK_reset_onu ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id );

PON_STATUS GW10G_CTC_STACK_start_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid );

PON_STATUS GW10G_CTC_STACK_stop_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid );

PON_STATUS GW10G_CTC_STACK_get_phy_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS GW10G_CTC_STACK_set_phy_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     state);

PON_STATUS GW10G_CTC_STACK_get_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *flow_control_enable);

PON_STATUS GW10G_CTC_STACK_set_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool			 flow_control_enable);

PON_STATUS GW10G_CTC_STACK_get_auto_negotiation_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS GW10G_CTC_STACK_set_auto_negotiation_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const bool			 state);

PON_STATUS GW10G_CTC_STACK_set_auto_negotiation_restart_auto_config ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number);

PON_STATUS GW10G_CTC_STACK_get_ethernet_link_state ( 
           const PON_olt_id_t	    olt_id, 
           const PON_onu_id_t	    onu_id,
		   const unsigned char	    port_number,
		   CTC_STACK_link_state_t  *link_state);

PON_STATUS GW10G_CTC_STACK_get_ethernet_port_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const unsigned char						  port_number,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing);

PON_STATUS GW10G_CTC_STACK_set_ethernet_port_policing ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   const CTC_STACK_ethernet_port_policing_entry_t   port_policing);

PON_STATUS GW10G_CTC_STACK_get_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting );

PON_STATUS GW10G_CTC_STACK_set_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const unsigned char										port_number,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting);

PON_STATUS GW10G_CTC_STACK_get_vlan_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration);

PON_STATUS GW10G_CTC_STACK_set_vlan_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration);

PON_STATUS GW10G_CTC_STACK_get_classification_and_marking ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   CTC_STACK_classification_rules_t   classification_and_marking);

PON_STATUS GW10G_CTC_STACK_set_classification_and_marking ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const unsigned char							port_number,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking);

PON_STATUS GW10G_CTC_STACK_delete_classification_and_marking_list ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t    onu_id,
		   const unsigned char   port_number);

PON_STATUS GW10G_CTC_STACK_get_multicast_vlan ( 
           const PON_olt_id_t			olt_id, 
           const PON_onu_id_t			onu_id,
		   const unsigned char			port_number,
		   CTC_STACK_multicast_vlan_t  *multicast_vlan);

PON_STATUS GW10G_CTC_STACK_set_multicast_vlan ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   const CTC_STACK_multicast_vlan_t   multicast_vlan);

PON_STATUS GW10G_CTC_STACK_clear_multicast_vlan ( 
           const PON_olt_id_t    olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number );

PON_STATUS GW10G_CTC_STACK_set_multicast_control ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_t   multicast_control );

PON_STATUS GW10G_CTC_STACK_clear_multicast_control ( 
           const PON_olt_id_t   olt_id, 
           const PON_onu_id_t	onu_id );

PON_STATUS GW10G_CTC_STACK_get_multicast_control ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_control_t	*multicast_control );

PON_STATUS GW10G_CTC_STACK_get_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   unsigned char	    *group_num);

PON_STATUS GW10G_CTC_STACK_set_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const unsigned char	 group_num);

PON_STATUS GW10G_CTC_STACK_get_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *tag_strip);

PON_STATUS GW10G_CTC_STACK_set_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     tag_strip);

PON_STATUS GW10G_CTC_STACK_get_multicast_switch ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_protocol_t  *multicast_protocol );

PON_STATUS GW10G_CTC_STACK_set_multicast_switch ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const CTC_STACK_multicast_protocol_t   multicast_protocol );

PON_STATUS GW10G_CTC_STACK_get_fast_leave_ability ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_ability_t		*fast_leave_ability );

PON_STATUS GW10G_CTC_STACK_get_fast_leave_admin_state ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state );

PON_STATUS GW10G_CTC_STACK_set_fast_leave_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   CTC_STACK_fast_leave_admin_state_t     fast_leave_admin_state );

PON_STATUS GW10G_CTC_STACK_get_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_vlan_configuration_ports_t   ports_info );


PON_STATUS GW10G_CTC_STACK_set_qinq_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration);

PON_STATUS GW10G_CTC_STACK_set_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_holdover_state_t  parameter);

PON_STATUS GW10G_CTC_STACK_get_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_holdover_state_t*  parameter);

PON_STATUS GW10G_CTC_STACK_set_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_t  parameter);

PON_STATUS GW10G_CTC_STACK_get_onu_version(
                                    const PON_olt_id_t		   olt_id, 
                                    const PON_onu_id_t		   onu_id,
                                    unsigned char             *onu_version );

PON_STATUS GW10G_CTC_STACK_onu_tx_power_supply_control ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
           const CTC_STACK_onu_tx_power_supply_control_t parameter,
           const bool               broadcast);

/*以下是设计文档中未列出的接口*/
PON_STATUS GW10G_CTC_STACK_get_onu_serial_number ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_onu_serial_number_t  *onu_serial_number );

PON_STATUS GW10G_CTC_STACK_init 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list );

PON_STATUS GW10GAdp_CTC_STACK_get_ethport_statistic_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   void *data);

PON_STATUS GW10GAdp_CTC_STACK_get_ethport_statistic_state (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   void *state);

PON_STATUS GW10G_CTC_STACK_get_auto_negotiation_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS GW10G_CTC_STACK_get_auto_negotiation_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS GW10G_CTC_STACK_get_multicast_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const unsigned char	                  port_number,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching);

PON_STATUS GW10G_CTC_STACK_set_multicast_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const unsigned char	                       port_number,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);

PON_STATUS GW10G_CTC_STACK_get_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_global_parameter_config_t*  parameter);

PON_STATUS GW10G_CTC_STACK_get_extended_oam_discovery_timing 
                ( unsigned short int  *discovery_timeout );

PON_STATUS GW10G_CTC_STACK_set_extended_oam_discovery_timing 
                ( unsigned short int  discovery_timeout );

PON_STATUS GW10G_CTC_STACK_get_encryption_timing 
                ( unsigned char       *update_key_time,
                  unsigned short int  *no_reply_timeout );

PON_STATUS GW10G_CTC_STACK_set_encryption_timing 
                ( const unsigned char       update_key_time,
                  const unsigned short int  no_reply_timeout );

PON_STATUS GW10G_CTC_STACK_set_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_mode_t          ctc_auth_mode);

PON_STATUS GW10G_CTC_STACK_set_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const bool								enable);

PON_STATUS GW10GAdp_CTC_STACK_set_ethport_statistic_state( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const int state);

PON_STATUS GW10GAdp_CTC_STACK_set_ethport_statistic_data( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number
);

PON_STATUS GW10GAdp_CTC_STACK_get_ethport_statistic_history_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   void *data);

PON_STATUS GW10G_CTC_STACK_is_init ( bool *init );

PON_STATUS GW10G_CTC_STACK_assign_handler_function
                ( const CTC_STACK_handler_index_t    handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id );

PON_STATUS GW10G_CTC_STACK_get_encryption_timing_threshold 
                ( unsigned char   *start_encryption_threshold);

PON_STATUS GW10G_CTC_STACK_set_encryption_timing_threshold 
                ( const unsigned char   start_encryption_threshold);

PON_STATUS GW10G_CTC_STACK_get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id );

PON_STATUS GW10G_CTC_STACK_get_firmware_version ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           unsigned short      *version );

PON_STATUS GW10G_CTC_STACK_get_onu_capabilities ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_t  *onu_capabilities );

PON_STATUS GW10G_CTC_STACK_get_onu_capabilities_2 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_2_t  *onu_capabilities );

PON_STATUS GW10G_CTC_STACK_get_multicast_all_port_tag_strip ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_strip_t   ports_info );

short int GW10G_CTC_STACK_auth_request ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   CTC_STACK_auth_response_t         *auth_response);

short int GW10G_CTC_STACK_auth_success ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id);

short int GW10G_CTC_STACK_auth_failure ( 
                                         const PON_olt_id_t                  olt_id, 
                                         const PON_onu_id_t                  onu_id,
                                         const CTC_STACK_auth_failure_type_t failure_type );

authentication_olt_database_t * GW10G_CTC_STACK_get_auth_loid_database( const PON_olt_id_t olt_id );

PON_STATUS GW10G_CTC_STACK_set_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const unsigned long						alarm_threshold,
			const unsigned long						clear_threshold );

PON_STATUS GW10G_CTC_STACK_set_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);

PON_STATUS GW10G_CTC_STACK_get_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_snmp_parameter_config_t*  parameter);

PON_STATUS GW10G_CTC_STACK_set_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_t  parameter);

PON_STATUS GW10G_CTC_STACK_get_oui 
                ( unsigned long  *oui );

PON_STATUS GW10G_CTC_STACK_set_oui 
                ( const unsigned long  oui );

PON_STATUS GW10G_CTC_STACK_get_version 
                ( unsigned char  *version );

PON_STATUS GW10G_CTC_STACK_set_version 
                ( const unsigned char  version );

PON_STATUS GW10G_CTC_STACK_update_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const char		            *file_name);

short int GW10G_CTC_STACK_activate_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);

short int GW10G_CTC_STACK_commit_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);

PON_STATUS GW10G_CTC_STACK_set_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );

PON_STATUS GW10G_CTC_STACK_get_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );
#if 0
short int GW10G_CTC_STACK_start_loid_authentication ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   bool                              *auth_success);
#endif
PON_STATUS GW10G_CTC_STACK_get_optical_transceiver_diagnosis ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );

PON_STATUS GW10G_CTC_STACK_set_voip_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state );

PON_STATUS GW10G_CTC_STACK_get_voip_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state);

PON_STATUS GW10G_CTC_STACK_set_voip_management_object ( 
           const PON_olt_id_t				    olt_id, 
           const PON_onu_id_t				    onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const CTC_STACK_on_off_state_t       port_state );

PON_STATUS GW10G_CTC_STACK_get_voip_management_object ( 
           const PON_olt_id_t		            olt_id, 
           const PON_onu_id_t		            onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_on_off_state_t            *port_state);

PON_STATUS GW10G_CTC_STACK_get_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_fax_config_t    	           *voip_fax);

PON_STATUS GW10G_CTC_STACK_set_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_fax_config_t            voip_fax);

PON_STATUS GW10G_CTC_STACK_voip_iad_operation( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_operation_type_t                operation_type);

PON_STATUS GW10G_CTC_STACK_get_voip_iad_oper_status( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_oper_status_t	           *iad_oper_status);

PON_STATUS GW10G_CTC_STACK_get_voip_global_param_conf ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			CTC_STACK_voip_global_param_conf_t         *global_param);

PON_STATUS GW10G_CTC_STACK_set_voip_global_param_conf( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_global_param_conf_t     global_param);

PON_STATUS GW10G_CTC_STACK_get_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_param_config_t    	       *h248_param);

PON_STATUS GW10G_CTC_STACK_set_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_param_config_t          h248_param);

PON_STATUS GW10G_CTC_STACK_set_h248_user_tid_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_h248_user_tid_config_t   user_tid_config);

PON_STATUS GW10G_CTC_STACK_get_h248_user_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
           const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
		   CTC_STACK_h248_user_tid_config_array_t    	       *h248_user_tid_array);

PON_STATUS GW10G_CTC_STACK_set_h248_rtp_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_rtp_tid_config_t       h248_rtp_tid_config);

PON_STATUS GW10G_CTC_STACK_get_h248_rtp_tid_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_rtp_tid_info_t    	       *h248_rtp_tid_info);

PON_STATUS GW10G_CTC_STACK_get_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_sip_param_config_t    	           *sip_param);

PON_STATUS GW10G_CTC_STACK_set_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_sip_param_config_t           sip_param);

PON_STATUS GW10G_CTC_STACK_get_sip_user_param_config(const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
           CTC_STACK_sip_user_param_config_array_t       *sip_user_param_array);


PON_STATUS GW10G_CTC_STACK_set_sip_user_param_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_sip_user_param_config_t   sip_user_param);

PON_STATUS GW10G_CTC_STACK_set_sip_digit_map( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_SIP_digit_map_t           sip_digit_map);

PON_STATUS GW10G_CTC_STACK_get_voip_iad_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_info_t	     	       *iad_info);

PON_STATUS GW10G_CTC_STACK_get_voip_pots_status ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
			CTC_STACK_voip_pots_status_array_t       *pots_status_array);

PON_STATUS GW10G_CTC_STACK_get_onu_capabilities_3 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_3_t  *onu_capabilities );

/*以下为CTC redundancy相关*/
PON_STATUS GW10G_CTC_STACK_get_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           CTC_STACK_redundancy_database_t             *redundancy_db);

PON_STATUS GW10G_CTC_STACK_set_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           const CTC_STACK_redundancy_database_t        redundancy_db);

PON_STATUS ctc_GW10G_redundancy_init (void);
#endif
/***End:CTC by jinhl**************/

/***Begin:redundancy by jinhl**************/
#if 1
PON_STATUS GW10G_redundancy_enable(void);

PON_STATUS GW10G_redundancy_disable(void);

short int GW10G_get_redundancy_state( const short int              olt_id, 
                                PON_redundancy_olt_state_t  *state );

bool GW10G_redundancy_olt_exists ( const short int olt_id );

short int GW10G_reset_redundancy_olt_record ( const short int    olt_id,
                                        const PON_redundancy_olt_state_t state );

short int GW10G_set_redundancy_state(const short int   olt_id, 
                               const PON_redundancy_olt_state_t  state);


short int GW10G_remove_olt_from_mapping_list(const short int     olt_id);

short int GW10G_swap_local_mapping_list(const PON_olt_id_t   master_olt_id, 
                            const PON_olt_id_t slave_olt_id);

short int  GW10G_insert_mapping_pair_to_local_list(const PON_olt_id_t   master_olt_id, 
                                      const PON_olt_id_t slave_olt_id);

short int GW10G_remote_olt_removed_handler(const PON_olt_id_t  olt_id, const short int  remote_olt_id);

short int GW10G_remote_olt_loosed_handler(const PON_olt_id_t  olt_id, const short int  remote_olt_id);

short int GW10G_get_pair_slave_olt_id(const PON_olt_id_t          olt_id,
                                PON_olt_id_t                *slave_olt_id,
                                PON_redundancy_olt_state_t  *state);

PON_STATUS GW10G_redundancy_init (void);


PON_STATUS GW10G_redundancy_assign_handler_function
                ( const redundancy_handler_index_t    handler_function_index, 
                  const void                        (*handler_function)(),
                  unsigned short                     *handler_id );

PON_STATUS GW10G_redundancy_master_slave_config(const olt_id_list_t master_olt_list,
                                          const unsigned short master_olt_num,
                                          const olt_id_list_t slave_olt_list,
                                          const unsigned short slave_olt_num,
                                          const gpio_list_t master_gpio_list,
                                          const gpio_list_t slave_gpio_list);

short int  GW10G_get_pair_master_olt_id(const PON_olt_id_t          olt_id,
                                 PON_olt_id_t                *master_olt_id,
                                 PON_redundancy_olt_state_t  *state);

/*PON_STATUS GW10G_redundancy_disable_olt(const PON_olt_id_t          olt_id);*/

PON_STATUS GW10G_redundancy_switch_over(const PON_olt_id_t master_olt_id, 
                                  const PON_olt_id_t slave_olt_id);
#endif

/***End:redundancy by jinhl**************/


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/

