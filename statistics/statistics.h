/**************************************************************************************
* Statistics.h - .定义了GW历史统计的数据结构，一些宏定义初始化函数,以及外部函数
*			调试函数声明等
* 
* Copyright (c) 2006 GW Technologies Co., LTD.
* All rights reserved.
* 
* create: 
* 2006-06-222  wutw
* 
* 文件描述：
*
* 1 modified by chenfj 2008-1-14
*    程序中使用的PAS 结构定义，或PAS-API，统一从文件includefromPas.h中引用
* 
**************************************************************************************/

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PAS_STATISTIC_5201
#define	PAS_STATISTIC_5201
#endif

#include "includeFromPas.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"


#define  PON_total_frames_raw_data_5001_t PON_total_frames_raw_data_t
#define  PON_total_octets_raw_data_5001_t PON_total_octets_raw_data_t

#define  PON_total_frames_raw_data_5201_t PON_total_frames_raw_data_t
#define  PON_total_octets_raw_data_5201_t PON_total_octets_raw_data_t

/*#define    STATS_DEBUG*/

#define STATS_DEVICE_OFF		0
#define STATS_DEVICE_ON		1
#define STATS_OLT_ALL			(-1)
#define STATS_ONU_ALL			(-1)
#define STATS_PON_ALL_ACTIVE_LLIDS		(-1)
#define DEFAULT_STATSOBJ_COUNT				20
#define DEFAULT_STATSOBJ_MAX				40
#define STATS_OLTPON_24HOUR				1
#define STATS_OLTETH_24HOUR				2
#define STATS_ONU_STATUS					3
#define STATS_OLT_STATUS					4
#define STATS_REALTIME_TIMEOUT			5
#define STATS_RTDATA_CLEAR				6
#define STATS_LOOP_ONU_PRO				7
#define STATS_RECORD_MODIFIED				8
#define STATS_TIMER_TIMEOUT				9
#define STATS_PONOBJ_MODIFIED				10
#define STATS_ONUOBJ_MODIFIED				11
#define STATS_PON_RECORD_MODIFIED		12
#define STATS_ONU_RECORD_MODIFIED		13
#define STATS_HISDATA_CLEAR				14

/*added by wangxy 2007-04-12 add eth port history statistic*/
#define STATS_ETHPORT_MODIFIED			15


#define STATS_HIS_MAXPON					MAXPON
#define STATS_HIS_MAXONU					MAXONUPERPON
/*#define STATS_RT_MASTER					2	only monitor the geo board*/
#define STATS_RT_MAXEHTER					4 /*24->4, statistic data is open for user only include 4 ports*/

#define STATS_MAX24HOUR_BUCKETNUM		60 /*buckNum = 30(day) * 24 / 24 */
#define STATS_MAX15MIN_BUCKETNUM			200/*bucketNum = 2(day) * 24 * 60/15  tow days*/
#define STATS_DEFAULT24H_BUCKETNUM		10 /*buckNum = 10   10(day)*/
#define STATS_DEFAULT15MIN_BUCKETNUM	20/*bucketNum = 20 , five hours*/

/*0代表主用主控卡
1 代表备用主控卡*/
#define STATS_RT_MASTERBRD	                0
#define STATS_RT_SLAVEBRD                   1

#if 0
#define MAX_LLID_NUM 						MAXONUPERPON
#define MAX_PON_NUM						    MAXPON
#endif
#define STATS_SAMPLING_COUNT				95/*(24Hour * 60Min / 15Min)-1 = 95*/

#define STATS_TASK_PRIO						240

#if 0
#define MIN_PRIORITY_QUEUE						 0 /* 802.1P standard */
#define MAX_PRIORITY_QUEUE						 7 /* 802.1P standard */
#endif


/*========define for alarm===============*/
#if 0
#endif
#define STATS_DROPEVENTS_TYPE		1
#define STATS_OCTETS_TYPE			2
#define STATS_PKTS_TYPE				3
#define STATS_BROADCASTPKTS_TYPE	4
#define STATS_MULTICASTPKTS_TYPE	5
#define STATS_CRCALIGNERR_TYPE		6
#define STATS_UNDERSIZEPKTS_TYPE	7
#define STATS_OVERSIZEPKTS_TYPE		8
#define STATS_FRAGMENTS_TYPE		9
#define STATS_JABBERS_TYPE			10
#define STATS_COLLISIONS_TYPE		11
#define STATS_PKT64OCTETS			12
#define STATS_PKT65TO127OCTETS		13
#define STATS_PKT128TO255OCTETS	14
#define STATS_PKT256TO511OCTETS	15
#define STATS_PKT512TO1023OCTETS	16
#define STATS_PKT1024TO1518OCTETS	17
#define STATS_ONULOOPBACK_RXOK	18
#define STATS_ONULOOPBACK_TXOK	19
#define STATS_ONULOOPBACK_RXERR	20
#define STATS_ONULOOPBACK_TOTALRXBAD	21

#define STATS_TXDROPPKTS  22
#define STATS_TXOCTETS 23
#define STATS_TXBROADCASTPKTS 24
#define STATS_TXMULTICASTPKTS 25
#define STATS_TXPKTS  26
#define STATS_TXGRANT  27
#define STATS_UPBANDWIDTH 28
#define STATS_DOWNBANDWIDTH  29
#define STATS_TXPAUSEPKTS  30
#define STATS_PAUSEPKTS  31

#define STATS_CLEAR_INPROCESS		2
#define STATS_CLEAR_START			1
#define STATS_CLEAR_NOOP			0

#define MON_OLT_BER				0
#define MON_OLT_FER				0


/*#define bool 		BOOL*/
/**/
#if 0
#endif

/*return error	code*/
#define	STATS_OK						(0)
#define	STATS_ERR						(-1)
#define	STATS_ERRCODE_TIMEOUT_ERR	(-9000)
#define	STATS_MSGSEND_FAILUE_ERR	(-9001)
#define	STATS_MAXPON_OUTRANG_ERR	(-9002)
#define	STATS_CHAIN_NULL_ERR		(-9003)
#define	STATS_INVALUE_OBJ_ERR		(-9004)
#define	STATS_POINT_NULL_ERR		(-9005)
#define	STATS_PONOBJ_NULL_ERR		(-9006)
#define	STATS_WDOG_CREATE_ERRO		(-9007)
#define	STATS_MALLOC_FAILUE_ERR		(-9008)
#define	STATS_TASKSPAWN_FAILUE_ERR	(-9009)
#define	STATS_TIMER_CREAR_ERR		(-9010)
#define	STATS_STATSTPYE_ERR			(-9011)
#define	STATS_NOFINE_CHAIN_ERR		(-9012)
#define	STATS_END_CHAIN_ERR			(-9013)
#define	STATS_END_OBJ_ERR			(-9014)

#define STATS_15MIN_SET    1
#define STATS_24HOUR_SET   2

/*added by wangxy, indicates port type about ethernet prot , pon ports on olt and ponport on onus*/
#define	PORT_TYPE_ETH	1
#define	PORT_TYPE_PON	2
#define	PORT_TYPE_ONU	3

#if 0
#define PON_BROADCAST_LLID		  127  /* Special LLID, indicating one of the broadcast LLIDs,		    */

typedef enum  
{
	PON_RAW_STAT_ONU_BER 						      = 0  , /* Average ONU Bit error rate counters (upstream)  */
												             /* as measured by the OLT						   */
	PON_RAW_STAT_OLT_BER						      = 1  , /* Average OLT Bit error rate counters (downstream)*/
													         /* as measured by the ONU						   */
	PON_RAW_STAT_ONU_FER							  = 2  , /* Average ONU frames error rate counters(upstream)*/
													         /* as measured by the OLT						   */
	PON_RAW_STAT_LLID_BROADCAST_FRAMES				  = 3  , /* LLID frames counters							   */
	PON_RAW_STAT_TOTAL_FRAMES						  = 4  , /* Total frames transmitted and received per       */
													         /* physical port								   */
	PON_RAW_STAT_TOTAL_OCTETS						  = 5  , /* Total octets (bytes) transmitted and received   */
													         /* per physical port							   */
	PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID		      = 6  , /* Total octets (bytes) transmitted per LLID	   */
													         /* through the PON port							   */
	PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY      = 7  , /* Number of frames, received from the UNI port and*/
													         /* transmitted to the PON port, per priority	   */
	PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID	      = 8  , /* Frames transmitted per LLID through the PON port*/
	PON_RAW_STAT_DROPPED_FRAMES 					  = 9  , /* Queue-dropped frames to transmit through the PON*/
													         /* port											   */
	PON_RAW_STAT_DROPPED_FRAMES_PER_LLID			  = 10 , /* Dropped frames by OLT due to LLID restriction   */
													         /* /error										   */
	PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES			  = 11 , /* Dropped frames received (by drop reason type)   */
	PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES		      = 12 , /* Total dropped frames to transmit through both   */
													         /* ports										   */
	PON_RAW_STAT_SINGLE_COLLISION_FRAMES			  = 13 , /* Not active (not used in full duplex mode)	   */
	PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES			  = 14 , /* Not active (not used in full duplex mode)	   */
	PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS		  = 15 , /* FCS check errors of frames received by the PON  */
													         /* port											   */
	PON_RAW_STAT_ALIGNMENT_ERRORS					  = 16 , /* Alignment check errors of frames received by the*/
													         /* PON port										   */
	PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID      = 17 , /* In range length check errors of frames received */
													         /* by LLIDs										   */
	PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS               = 18 , /* In range length check errors of frames received */
													         /* by System port								   */
	PON_RAW_STAT_FRAME_TOO_LONG_ERRORS                = 19 , /* Frame too long check errors of frames received  */
													         /* by both ports								   */
	PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID       = 20 ,/* Frame too long check errors of frames received  */
													         /* from specified LLID							   */
	PON_RAW_STAT_HEC_FRAMES_ERRORS 					  = 21 , /* Header error correction check errors of frames  */
													         /* received by the PON port						   */
	PON_RAW_STAT_UNSUPPORTED_MPCP_FRAMES			  = 22 , /* Unsupported MPCP type check errors of frames    */
													         /* received by the PON port						   */
	PON_RAW_STAT_HOST_FRAMES						  = 23 , /* Frames received and transmitted to the Host	   */
	PON_RAW_STAT_HOST_OCTETS						  = 24 , /* Octets received and transmitted to the Host	   */
	PON_RAW_STAT_HOST_MESSAGES						  = 25 , /* Traffic load of the Host - Firmware parallel    */
													         /* interface									   */
	PON_RAW_STAT_PAUSE_FRAMES						  = 26 , /* Pause frames received and transmitted by both   */
													         /* ports										   */
	PON_RAW_STAT_PAUSE_TIME 						  = 27 , /* Link in Pause mode times						   */ 
	PON_RAW_STAT_REGISTRATION_FRAMES				  = 28 , /* Registration frames counters by both OLT and ONU*/
	PON_RAW_STAT_OAM_FRAMES 						  = 29 , /* OAM frames received and transmitted in the PON  */
	PON_RAW_STAT_GRANT_FRAMES						  = 30 , /* Grant frames counters by both OLT and ONU	   */
	PON_RAW_STAT_REPORT_FRAMES						  = 31 , /* MPCP 'Report' frames counters collected by both */
													         /* OLT and ONU									   */
	PON_RAW_STAT_MULTICAST_FRAMES					  = 32 , /* Multicast frames received and transmitted by the*/
													         /* PON port per ONU								   */
	PON_RAW_STAT_BROADCAST_FRAMES					  = 33 , /* Broadcast frames received and transmitted by the*/
													         /* PON port per ONU								   */
	PON_RAW_STAT_P2P_FRAMES 						  = 34 , /* P2P frames received, dropped and transmitted by */
													         /* the PON port per ONU							   */
	PON_RAW_STAT_BRIDGE_FRAMES						  = 35 , /* Frames bridged by the ONU (at System port)	   */
	PON_RAW_STAT_PROMISCUOUS_STATUS 				  = 36 , /* PON Promiscuous Status						   */
	PON_RAW_STAT_DUPLEX_STATUS						  = 37 , /* PON Duplex status (Half / Full)				   */
	PON_RAW_STAT_RTT								  = 38 , /* Round trip time from OLT to ONU and back        */
	PON_RAW_STAT_RATE_CONTROL_ABILITY				  = 39 , /* Rate Control through lowering the average data  */
													         /* rate of the MAC sublayer ability				   */
	PON_RAW_STAT_MPCP_STATUS    					  = 40 , /* MPCP configuration and traffic counters		   */
	PON_RAW_STAT_OAM_STATUS     					  = 41 , /* OAM configuration and traffic counters		   */
	PON_RAW_STAT_OAM_INFORMATION_DATA				  = 42 , /* Fields inside remote and local OAM information  */
													         /* frames										   */
	PON_RAW_STAT_OAM_FRAMES_COUNTERS				  = 43 , /* Traffic counters of OAM frames by OAM frame code*/
	PON_RAW_STAT_ERRORED_SYMBOL_PERIOD_ALARM		  = 44 , /* Errored Symbol Period alarm configuration and   */
													         /* event counters								   */
	PON_RAW_STAT_ERRORED_FRAME_ALARM				  = 45 , /* Errored Frame alarm configuration and event     */
													         /* counters										   */
	PON_RAW_STAT_ERRORED_FRAME_PERIOD_ALARM	    	  = 46 , /* Errored Frame Period alarm configuration and    */
													         /* event counters								   */
	PON_RAW_STAT_ERRORED_FRAME_SECONDS_SUMMARY_ALARM  = 47 , /*Errored Frame Seconds Summary alarm			   */
													         /* configuration and event counters				   */
	                                                        
    PON_RAW_STAT_STANDARD_OAM_STATUS                  = 48 , /* Standard OAM status                            */
    PON_RAW_STAT_STANDARD_OAM_PEER_STATUS             = 49 , /* Standard OAM peer status                       */
    PON_RAW_STAT_STANDARD_OAM_LOOPBACK_STATUS         = 50 , /* Standard OAM loopback status                   */
    PON_RAW_STAT_STANDARD_OAM_STATISTIC               = 51 , /* Standard OAM statistic                         */
    PON_RAW_STAT_STANDARD_OAM_EVENT_CONFIG            = 52 , /* Standard OAM Event config                      */
    PON_RAW_STAT_STANDARD_OAM_LOCAL_EVENT_LOG         = 53 , /* Standard OAM local event log                   */
    PON_RAW_STAT_STANDARD_OAM_REMOTE_EVENT_LOG        = 54 , /* Standard OAM remote event log                  */
    PON_RAW_STAT_STANDARD_OAM_MPCP_STATUS             = 55 , /* Standard OAM MPCP status                       */
    PON_RAW_STAT_STANDARD_OAM_MPCP_STATISTIC          = 56 , /* Standard OAM MPCP statistic                    */
    PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATUS    = 57 , /* Standard OAM OMP emulation status              */
    PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS= 58 , /* Standard OAM OMP emulation statistics          */
    PON_RAW_STAT_STANDARD_OAM_MAU_STATUS              = 59 , /* Standard OAM MAU status                        */
    PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY        = 60 , /* Received frames to CPU per priority            */
    PON_RAW_STAT_TRANSMITTED_FRAMES_FROM_CPU_PER_PRIORITY   = 61 , /* Transmitted frames from CPU per priority       */
    PON_RAW_STAT_CPU_PORTS_OCTET                            = 62 , /* CPU ports octets                               */
    PON_RAW_STAT_CPU_PORTS_FRAMES                           = 63 , /* CPU ports frames                               */
    PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES                = 64 , /* Total dropped CPU RX frames                    */
    PON_RAW_STAT_ENCRYPT_FRAMES		                        = 65 , /* Encrypt frames                                 */
    PON_RAW_STAT_START_END_SYMBOL_FRAMES		            = 66 , /* Start, End symbols frames                      */ 
    PON_RAW_STAT_LAST_STAT                             
} PON_raw_statistics_t;


typedef enum
{
	STAT_QUERY_HARDWARE = 2	/* Direct query from hardware counters	*/
} statistics_query_method_t;

typedef struct
{
	unsigned long  error_bytes;			   /* Error bytes received by the OLT (PON port)				*/
	long double	   used_byte_count;		   /* Total bytes containing data received by the OLT which     */
										   /* originated from the measured ONU (not including IDLEs)	*/
	long double	   good_byte_count;        /* Total bytes received by the OLT which	originated from the	*/
										   /* measured ONU (including IDLEs)							*/
} PON_onu_ber_raw_data_t;

/* OLT BER raw statistics */
typedef struct
{
	unsigned long  error_bytes;			/* Error bytes received by the ONU (PON port)				   */
	long double	   byte_count;			/* Total number of data and padding bytes (octets) that were   */
										/* Successfully received by the ONU's PON port (entire		   */
										/* downstream traffic)										   */
	unsigned long  error_onu_bytes;		/* Error bytes destined for the ONU (PON port)				   */
	long double	   onu_byte_count;		/* Total number of bytes that were successfully received by the*/
										/* ONU's PON port and were destined for the ONU				   */

    long double    onu_total_byte_count;/* Total number of bytes (good + bad)  that were received by   */
                                        /* the ONU's PON port and were destined for the ONU            */
} PON_olt_ber_raw_data_t;

typedef struct
{
	unsigned long  received_error;		  /* Error frames received by the OLT and originated from the  */
										  /* ONU measured											   */
	unsigned long  received_ok;			  /* Good frames received by the OLT and originated from the   */
										  /* ONU measured											   */
	unsigned long  firmware_received_ok;  /* Good frames received by the OLT, originated from the ONU  */
										  /* measured, and destined to the OLT firmware (including host*/
										  /* frames)												   */
} PON_onu_fer_raw_data_t;


/* Statistics query methods */
typedef enum
{
	PON_STATISTICS_QUERY_HARDWARE = 2	/* Direct query from hardware counters	*/
} PON_statistics_query_method_t;


typedef unsigned long  PON_timestamp_t;		/* Stated in TQ */


/*=====================*/
/* Total dropped RX frames raw statistics */
typedef struct
{
	unsigned long  total_control_dropped;	/* (PON received) Frames dropped due to full control queue */
	unsigned long  source_alert_dropped;    /* Frames dropped due to source alert (frame source MAC    */
											/* address == OLT MAC address)					           */
    unsigned long  pon_match           ;    /* Frames dropped due to match address (frame's source MAC */
                                            /* address learned before from the system port)            */
} PON_total_dropped_rx_frames_raw_data_t;

/* Total dropped TX frames raw statistics */
typedef struct
{
	unsigned long  total_pon_dropped;		/* Total dropped frames (to transmit to the PON) due  */
											/* to queue overflow								  */
	unsigned long  total_system_dropped;	/* Total dropped frames (to transmit to the System)   */
											/* due to queue overflow							  */
} PON_total_tx_dropped_frames_raw_data_t;



/* Total octets raw statistics 
**
** Statistics parameter: OLT_PHYSICAL_PORT_PON / OLT_PHYSICAL_PORT_SYSTEM  - OLT collector
**						 ONU_PHYSICAL_PORT_PON / ONU_PHYSICAL_PORT_SYSTEM  - ONU collector
*/
typedef struct
{
	long double    received_ok;		/* Bytes (octets), data and padding, received by the physical port */
	long double    received_error;	/* Error bytes(octets), data and padding, received by the physical */
									/* port															   */
	long double    transmitted_ok;	/* Bytes (octets), data and padding, transmitted successfully	   */
									/* through the physical port									   */
    long double    received_total;	/* Total received octets through the physical port	               */
} PON_total_octets_raw_data_5201_t;

typedef struct
{
	unsigned long received_ok;
	unsigned long received_error;
	unsigned long transmitted_ok;
	unsigned long received_total;
} PON_total_octets_raw_data_5001_t;

/* Broadcast frames raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**							   PON_BROADCAST_LLID (for multicast LLID)
**							   
*/
typedef struct
{
	unsigned long  received_ok;		/* Received frames (by the PON port) which were directed to the */ 
									/* broadcast group address										*/
	unsigned long  transmitted_ok;	/* Successfully transmitted frames (through the PON port) which */
									/* were directed to the broadcast group address					*/
} PON_broadcast_frames_raw_data_t;


/* Multicast frames raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**							   PON_BROADCAST_LLID (for multicast LLID)
**							   
*/
typedef struct
{
	unsigned long  received_ok;	    /* Received frames (by the PON port) which were directed to an  */
									/* active non-broadcast group MAC address					    */
	unsigned long  transmitted_ok;  /* Successfully transmitted frames (through the PON port) which */
									/* were directed to an active non-broadcast group MAC address	*/
} PON_multicast_frames_raw_data_t;



/* Alignment errors raw statistics */
typedef struct
{
	unsigned long  received;	/* Received frames (PON port) which didn't have an integral         */
								/* number of bytes (octets) in length and didn't pass the FCS check.*/
								/* Not implemented - thus always 0									*/

} PON_alignmnet_errors_raw_data_t;

typedef struct
{
	unsigned long  received;		/* Received frames (PON port) which had an integral number   */
									/* of bytes (octets) in length and didn't pass the FCS check */
} PON_frame_check_sequence_errors_raw_data_t;


/* In range length errors per LLID raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   / multicast LLID including discovery-time data)
*/
typedef struct
{
	unsigned long  received;	 /* length/type field value was less than the minimum		   */
								 /* allowed unpadded data size and the number of data octets   */
								 /* received was greater than the minimum unpadded data size   */
} PON_in_range_length_errors_per_llid_raw_data_t;


/* In range length errors raw statistics
**
** Statistics parameter: 0
*/
typedef struct
{
	unsigned long  system_received;	 /* length/type field value was less than the minimum		  */
									 /* allowed unpadded data size and the number of data octets  */
									 /* received was greater than the minimum unpadded data size  */
} PON_in_range_length_errors_raw_data_t;


/* Frame too long errors raw statistics
**
** Statistics parameter: 0
*/
typedef struct
{
	unsigned long  pon_received;	/* Received frames (PON port) which exceeded the maximum	  */
									/* permitted frame size										  */
	unsigned long  system_received;	/* Received frames (System port) which exceeded the maximum   */
									/* permitted frame size										  */
} PON_frame_too_long_errors_raw_data_t;


/* Frame too long errors per LLID raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   / multicast LLID including discovery-time data)
*/
typedef struct
{
	unsigned long  received;	/* Received frames (from the specified LLID) which exceeded the */
								/* maximum permitted frame size									*/
} PON_frame_too_long_errors_per_llid_raw_data_t;





/* Single collision frames raw statistics */
typedef struct
{
	unsigned long  single_collision_frames;		/* Single collision frames counter - always 0 in EPON */
} PON_single_collision_frames_raw_data_t;


/* Multiple collision frames raw statistics */
typedef struct
{
	unsigned long  multiple_collision_frames;/* Multiple collision frames counter - always 0 in EPON */
} PON_multiple_collision_frames_raw_data_t;

/* HEC frames errors raw statistics */
typedef struct
{
	unsigned long  received; /* Received frames (PON port) with HEC (header error correction) errors */
} PON_hec_frames_errors_raw_data_t;


/* Note: update PON_OLT_PHYSICAL_PORT_IN_BOUNDS on changes */
typedef enum
{
	OLT_PHYSICAL_PORT_AUTOMATIC,	  /* Default selection of physical port */
	OLT_PHYSICAL_PORT_PON,			  /* PON port						    */
	OLT_PHYSICAL_PORT_SYSTEM,		  /* a.k.a. CNI, Network, upstream port */
	OLT_PHYSICAL_PORT_PON_AND_SYSTEM  /* Both PON and System ports			*/
} PON_olt_physical_port_t;

typedef struct
{
	unsigned long  received_ok;			   /* Good frames received by the physical port				   */
	unsigned long  received_error;		   /* Error frames received by the physical port			   */
	unsigned long  transmitted_ok;		   /* Frames transmitted successfully through the physical port*/
    unsigned long  total_bad;		       /* Total bad received frames through the physical port      */
} PON_total_frames_raw_data_5001_t;

typedef struct
{
	long double  received_ok;			   /* Good frames received by the physical port				   */
	long double  received_error;		   /* Error frames received by the physical port			   */
	long double  transmitted_ok;		   /* Frames transmitted successfully through the physical port*/
    	long double  total_bad;		       /* Total bad received frames through the physical port      */
} PON_total_frames_raw_data_5201_t;


/* Registration frames raw statistics */
typedef struct
{
	unsigned long  register_request_transmitted_ok;	/* Successfully transmitted registration request */
													/* frames, a count of number of attempts to		 */
													/* perform registration							 */
	unsigned long  register_received_ok;			/* Received OAM frames (PON port), a count of	 */
													/* OAMPDUs passed by the OAM subordinate sublayer*/
													/* to the OAM sublayer							 */
	unsigned long  register_ack_transmitted_ok;		/* Successfully transmitted OAM frames (PON port)*/
													/* ,a count of OAMPDUs passed to the OAM		 */
													/* subordinate sublayer for transmission 		 */
} PON_registration_frames_raw_data_t;


/* OAM frames raw statistics */
typedef struct
{
	unsigned long  received_ok;					 /* Received OAM frames (PON port)					 */
	unsigned long  transmitted_ok;				 /* Successfully transmitted OAM frames (PON port)	 */
} PON_oam_frames_raw_data_t;

typedef enum
{
	PON_MPCP_STATUS_UNREGISTERED = 0x01,
	PON_MPCP_STATUS_REGISTERING	 = 0X02,
	PON_MPCP_STATUS_REGISTERED   = 0X03
} PON_mpcp_registration_status_t;
typedef enum
{
	PON_MPCP_MODE_OLT = 0x01,
	PON_MPCP_MODE_ONU = 0x02
} PON_mpcp_mode_t;
/* MPCP status raw statistics */
typedef struct
{
	bool							admin_state;		 		  /*  PON Interface can/can't provide Multi-point MAC control.									*/
																  /*  values: ENABLE/DISABLE																	*/
	PON_mpcp_mode_t					mode;					      /*  PON Interface can provide Multi-point MAC control functions or							*/ 
																  /*  acts as OLT  values: OLT/ONU.																*/
	PON_mpcp_registration_status_t	registration_state;			  /*  The operational state of the Multi-Point MAC Control sublayer..						    */
																  /*  Values: Enum value (Unregistered/ Registering/ Registered),								*/
																  /*  Always PON_MPCP_STATUS_REGISTERED when read from the OLT host 							*/
	unsigned long					unsupported;				  /*  Received frames (PON port) with unsupported MPCP type.								    */
	unsigned long					mac_ctrl_frames_transmitted;  /*  A count of MPCP frames passed to the MAC sublayer for transmission					    */
	unsigned long					mac_ctrl_frames_received;     /*  A count of MPCP frames passed by the MAC sublayer to the MAC Control sublayer			    */
	unsigned short					maximum_pending_grants;		  /*  The maximum number of grants an ONU can store. Constant value of 8 for PAS6001-N device   */
} PON_mpcp_status_raw_data_t;			

/* 
** Host frames raw statistics
**
** Statistics parameter: OLT collector		  - 0  
**						 ONU collector		  - 0
**						 OLT & ONU collectors - PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**												PON_BROADCAST_LLID (broadcast / multicast LLID including
**											    discovery frames)
*/
typedef struct
{
	unsigned long  pon_received_ok;						/* Received frames, originated from PON port,  */
														/* which were destined for the host			   */
	unsigned long  system_received_ok;					/* Received frames, originated from System port*/
														/* ,which were destined for the host		   */
	unsigned long  pon_transmitted_ok;					/* Firmware/ Host originated, successfully     */
														/* transmitted to the PON, frames			   */
	unsigned long  system_transmitted_ok;				/* Firmware/ Host originated, successfully	   */
														/* transmitted to the System, frames		   */
	unsigned long  pon_dropped;							/* Received frames from PON port, destined for */
														/* the host, which were dropped due to full    */
														/* buffer									   */
	unsigned long  system_dropped;						/* Received frames from System port, destined  */
														/* for the host, which were dropped due to full*/
														/* buffer									   */
} PON_host_frames_raw_data_t;

/* Host messages raw statistics 
**
** Statistics parameter: Host - firmware communication type (PON_comm_type_t values)
** Parallel - Parallel interface queues counters
** Ethernet - Counters of frames traffic between secondary host port and the OLT firmware
** UART		- UART driver framer counters
*/
typedef struct
{
	unsigned long  received_from_firmware_ok;	/* Messages sent from the OLT firmware to the Host    */
	unsigned long  sent_to_firmware_ok;			/* Messages sent by the Host to the OLT firmware	  */
} PON_host_messages_raw_data_t;

/* Pause frames raw statistics */
typedef struct
{
	unsigned long  pon_received_ok;				/* Received Pause frames (PON port)					  */
	unsigned long  system_received_ok;			/* Received Pause frames (System port)				  */
	unsigned long  pon_transmitted_ok;			/* Successfully transmitted Pause frames (PON port)	  */
												/* Valid when in P2P mode only						  */
	unsigned long  system_transmitted_ok;		/* Successfully transmitted Pause frames (System port)*/
} PON_pause_frames_raw_data_t;


/* Pause time raw statistics */
typedef struct
{
	unsigned long  system_port;				/* 'System port in pause mode' time, 512 nSsec resolution */
											/* (PON_STATISTICS_SAMPLING_TIMESTAMP_CYCLE_LONG cycle)   */
} PON_pause_time_raw_data_t;

/* Grant frames raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_DISCOVERY_LLID for  
**							   discovery grants statistics (received_ok will be zeroed)
*/
typedef struct
{
	unsigned long  received_ok;	        /* Received grant (upstream bandwidth allocation) frames	  */
	unsigned long  transmitted_dba_ok;  /* Successfully transmitted grant frames, originated from DBA */
										/* module													  */
	unsigned long  transmitted_ctrl_ok; /* Successfully transmitted 'REGISTER' frames (broadcast LLID)*/
} PON_grant_frames_raw_data_t;


/* MPCP 'REPORT' frames raw statistics */
typedef struct
{
	unsigned long  received_ok;					/* Received MPCP 'report' frames					*/
	unsigned long  transmitted_ok;				/* Successfully transmitted MPCP 'report' frames	*/
} PON_report_frames_raw_data_t;



/* Promiscuous Status raw statistics */
typedef struct
{
	short int  status;		 /* Promiscuous mode enabled / disabled in the PON port,				*/
							 /* values: DISABLE/ENABLE, always ENABLE								*/
} PON_promiscuous_status_raw_data_t;

/* Duplex mode */
typedef enum 
{
	PON_HALF_DUPLEX,
	PON_FULL_DUPLEX
} PON_duplex_status_t;
/* Duplex Status raw statistics */
typedef struct
{
	PON_duplex_status_t  status;		   /* Duplex status (PON_HALF_DUPLEX / PON_FULL_DUPLEX)    */
										   /* in the PON port, always PON_FULL_DUPLEX			   */
	PON_duplex_status_t  mac_capabilities; /* MAC capabilities in the PON port,					   */
										   /* values: PON_HALF_DUPLEX / PON_FULL_DUPLEX,		   */
										   /* always PON_FULL_DUPLEX							   */
} PON_duplex_status_raw_data_t;

typedef long  PON_rtt_t;  /* C-type representation of RTT measurement (round trip time from    */
						  /* OLT to ONU and back, measured in TQ)*/	
/* Round trip time raw statistics */
typedef struct
{
	PON_rtt_t  rtt;				 /* ONU RTT measurement (from OLT to ONU and back). measured in TQ */
} PON_rtt_raw_data_t;
/* P2P frames raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**							   PON_BROADCAST_LLID (broadcast / multicast LLID - significant results 
**							   only for 'transmitted_ok' field)
**							   
*/
typedef struct
{
	unsigned long  received_ok;						/* Received P2P frames							   */
	unsigned long  dropped_by_policer;				/* Received P2P frames dropped by Policer		   */
	unsigned long  dropped_by_access_control;		/* Received P2P frames dropped by access control   */
	unsigned long  dropped_due_to_tx_buffer_full;   /* Received P2P frames dropped due to full PON	   */
													/* transmit buffer								   */
	unsigned long  transmitted_ok;					/* Successfully transmitted P2P frames			   */
} PON_p2p_frames_raw_data_t;

/* Received frames to CPU per priority raw statistics */
typedef struct
{
    /* Number of frames, received from the physical port and send to CPU, per priority */
	unsigned long  received_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 
	
} PON_received_frames_to_cpu_per_priority_raw_data_t;

/* Transmitted frames from CPU per priority */
typedef struct
{
    /* Number of frames, transmitted to the physical port from the CPU,per priority */
	unsigned long  transmitted_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 
	
} PON_transmitted_frames_from_cpu_per_priority_raw_data_t;

/* Encrypt frames raw statistics */
typedef struct
{
	unsigned long  encrypt;	    /* The number of encrypted frames transmitted at PON port               */
											
	unsigned long  decrypt;	/* The number of frames received at PON port, with encryption           */
	
} PON_encrypt_frames_raw_data_t;

/* Total dropped CPU RX frames raw statistics */
typedef struct
{
	unsigned long  total_pon_dropped;	/* Total dropped frames received from  PON to CPU due to queue overflow.  */
											
	unsigned long  total_system_dropped;/* Total dropped frames received from  SYSTEM to CPU due to queue overflow*/
	
} PON_total_dropped_cpu_rx_frames_raw_data_t;


/* OAM frames counters raw statistics */
typedef struct
{
	unsigned long unsupported_codes_rx;				/* Rx OAMPDU with "unsupported OAM" code														*/
	unsigned long information_tx;					/* Tx OAMPDU with "information" code															*/
	unsigned long information_rx;					/* Rx OAMPDU with "information" code															*/
	unsigned long event_notification_tx;			/* Tx OAMPDU with "event notification" code														*/
	unsigned long unike_event_notification_rx;		/* Rx OAMPDU with "event notification" code with sequence number different from the last one	*/
	unsigned long duplicate_event_notification_rx;	/* Rx OAMPDU with "event notification code" with sequence number equal to the last one			*/
	unsigned long oam_loopback_control_tx;			/* Tx OAMPDU with "loopback control" code														*/
	unsigned long oam_loopback_control_rx;			/* Rx OAMPDU with "loopback control" code														*/
	unsigned long oam_variable_request_tx;			/* Tx OAMPDU with "variable request" code														*/
	unsigned long oam_variable_request_rx;			/* Rx OAMPDU with "variable response" code														*/
	unsigned long oam_variable_response_tx;			/* Tx OAMPDU with "variable response" code														*/
	unsigned long oam_variable_response_rx;			/* Rx OAMPDU with "variable response" code														*/
	unsigned long oam_organization_specific_tx;		/*  Tx OAMPDU with "Organization specific" code													*/
	unsigned long oam_organization_specific_rx;		/*  Rx OAMPDU with "Organization specific" code													*/
	unsigned long frames_lost_due_to_oam_error;		/* number of frames that not sent due to OAM error												*/
} PON_oam_frames_counters_raw_data_t;

/* Host octets raw statistics */
typedef struct
{
	unsigned long  received_ok;		/* Received bytes (octets) from the PON port which were destined   */
									/* for the host													   */
	unsigned long  transmitted_ok;	/* Host originated, successfully transmitted (through the PON port)*/
									/* bytes (octets)												   */
} PON_host_octets_raw_data_t;

/* Bridge frames raw statistics */
typedef struct
{
	unsigned long  received;						/* Received and bridged frames (from System port)  */
} PON_bridge_frames_raw_data_t;
#endif

/* 
** Complex statistics types	(hardware counters + software manipulations)
**
*/
#if 0
typedef enum  
{
	PON_STAT_ONU_BER,			/* Average ONU Bit error rate (upstream) as measured by the OLT        */
	PON_STAT_OLT_BER,			/* Average OLT Bit error rate (downstream) as measured by the ONU      */
	PON_STAT_ONU_FER,			/* Average ONU frames error rate (upstream) as measured by the OLT     */
	PON_STAT_OLT_FER,			/* Average OLT frames error rate (downstream) as measured by the ONU   */
	PON_STAT_LAST_STAT
} PON_statistics_t;

#endif

#if 0
typedef struct
	{
		unsigned int eponHis15MinMaxRecord;
		unsigned int eponHis24HourMaxRecord;
	}statsLimitEntry;                 
#endif

/*实时采样数据结构*/
typedef struct
	{
		unsigned long ifIndex;
		unsigned long rtStatsDropEvents;
		long double 	rtStatsOctets;
		unsigned long rtStatsPkts;
		unsigned long rtStatsBroadcastPkts;
		unsigned long rtStatsMulticastPkts;
		unsigned long rtStatsCRCAlignErrors;
		unsigned long rtStatsUndersizePkts;
		unsigned long rtStatsOversizePkts;
		unsigned long rtStatsFragments;
		unsigned long rtStatsJabbers;
		unsigned long rtStatsCollisions;
		unsigned long rtStatsPkts64Octets;
		unsigned long rtStatsPkts65to127Octets;
		unsigned long rtStatsPkts128to255Octets;
		unsigned long rtStatsPkts256to511Octets;
		unsigned long rtStatsPkts512to1023Octets;
		unsigned long rtStatsPkts1024to1518Octets;
		
		/*以下用于环回测试使用*/
		unsigned long rtStatsRxOk;
		unsigned long rtStatsTxOk;
		unsigned long rtStatsRxTotalBad;
		unsigned long rtStatsRxErr;		

		/*added by wangxy 2007-04-18 PON port ber/fer record and realtime bandwidth customed*/
		long double	rtStatsTransmmitOctets;
		long double	rtStatsTransmmitPkts;
		long	double	rtStatsErrOctets;		/*BER统计错误字节*/
		unsigned	long	rtStatsErrPkts;		/*FER统计错误帧*/
		long	double	rtBerRecord;			/*误码率*/
		long	double	rtFerRecord;			/*错误帧率*/
		unsigned	long	rtUpBandWidth;		/*上行实时带宽unit: bps*/
		unsigned long	rtDownBandWidth;		/*下行实时带宽unit: bps*/

		/*added by zhengyt 2008-5-14 增加发送方向的数据统计*/
		unsigned long long  rtTxDropFrame;
		unsigned long long  rtTxBroadcastFrame;
		unsigned long long  rtTxMulticastFrame;
		unsigned long long  rtTxOctets;
		unsigned long long  rtTxPkts;
		unsigned long long  rtTxGrantPkts;
		unsigned long long  rtTxPausePkts;
		unsigned long long rtPausePkts;
		
	}rtStatsEntry;

typedef struct
	{
		unsigned long ifIndex;
		unsigned long long rtStatsDropEvents;
		long double 	rtStatsOctets;
		unsigned long long  rtStatsPkts;
		unsigned long long  rtStatsBroadcastPkts;
		unsigned long long  rtStatsMulticastPkts;
		unsigned long long  rtStatsCRCAlignErrors;
		unsigned long long   rtStatsUndersizePkts;
		unsigned long long   rtStatsOversizePkts;
		unsigned long long   rtStatsFragments;
		unsigned long long   rtStatsJabbers;
		unsigned long long   rtStatsCollisions;
		unsigned long long   rtStatsPkts64Octets;
		unsigned long long   rtStatsPkts65to127Octets;
		unsigned long long   rtStatsPkts128to255Octets;
		unsigned long long   rtStatsPkts256to511Octets;
		unsigned long long   rtStatsPkts512to1023Octets;
		unsigned long long   rtStatsPkts1024to1518Octets;
		
		/*以下用于环回测试使用*/
		unsigned long rtStatsRxOk;
		unsigned long rtStatsTxOk;
		unsigned long rtStatsRxTotalBad;
		unsigned long rtStatsRxErr;		

		/*added by wangxy 2007-04-18 PON port ber/fer record and realtime bandwidth customed*/
		long double	rtStatsTransmmitOctets;
		long double	rtStatsTransmmitPkts;
		long	double	rtStatsErrOctets;		/*BER统计错误字节*/
		unsigned	long	rtStatsErrPkts;		/*FER统计错误帧*/
		long	double	rtBerRecord;			/*误码率*/
		long	double	rtFerRecord;			/*错误帧率*/
		unsigned	long	rtUpBandWidth;		/*上行实时带宽unit: bps*/
		unsigned long	rtDownBandWidth;	/*下行实时带宽unit: bps*/

		/*added by zhengyt 2008-5-14 增加发送方向的数据统计*/
		unsigned long  long  rtTxDropFrame;
		unsigned long  long  rtTxBroadcastFrame;
		unsigned long  long  rtTxMulticastFrame;
		unsigned long  long  rtTxOctets;
		unsigned long  long  rtTxPkts;
		unsigned long  long  rtTxGrantPkts;
		unsigned long  long  rtTxPausePkts;/* Out */
		unsigned long  long  rtPausePkts;/* In */

		/* begin: added by jianght 20090904 */
		/*unsigned long  long  rtFragments;*/
		/* end: added by jianght 20090904 */
	
	}rtStatsEntry64;


/*PON端口数据采集：*/
  typedef struct {
  		unsigned long ifIndex;
		unsigned long hisMonSampleIndex;
		unsigned long hisMonIntervalStart;
		
		unsigned long hisMonDropEvents;
		/*unsigned long hisMonOctets;*/
		double long hisMonOctets;
		unsigned long hisMonPkts;
		unsigned long hisMonBroadcastPkts;
		unsigned long hisMonMulticastPkts;
		unsigned long hisMonCRCAlignErrors;
		unsigned long hisMonUndersizePkts;
		unsigned long hisMonOversizePkts;
		unsigned long hisMonFragments;
		unsigned long hisMonJabbers;
		unsigned long hisMonCollisions;
		unsigned long hisMonPkts64Octets;
		unsigned long hisMonPkts65to127Octets;
		unsigned long hisMonPkts128to255Octets;
		unsigned long hisMonPkts256to511Octets;
		unsigned long hisMonPkts512to1023Octets;
		unsigned long hisMonPkts1024to1518Octets;
	} PON_StatsData_Table;

/*PON端口数据采集：*/
  typedef struct {
  		unsigned long ifIndex;
		unsigned long hisMonSampleIndex;
		unsigned long hisMonIntervalStart;
		
		unsigned long long  hisMonDropEvents;
		/*unsigned long hisMonOctets;*/
		unsigned long long  hisMonOctets;
		unsigned long long  hisMonPkts;
		unsigned long  long hisMonBroadcastPkts;
		unsigned long long  hisMonMulticastPkts;
		unsigned long long  hisMonCRCAlignErrors;
		unsigned long long  hisMonUndersizePkts;
		unsigned long long  hisMonOversizePkts;
		unsigned long long  hisMonFragments;
		unsigned long long  hisMonJabbers;
		unsigned long long  hisMonCollisions;
		unsigned long long  hisMonPkts64Octets;
		unsigned long long  hisMonPkts65to127Octets;
		unsigned long long  hisMonPkts128to255Octets;
		unsigned long long  hisMonPkts256to511Octets;
		unsigned long long  hisMonPkts512to1023Octets;
		unsigned long long  hisMonPkts1024to1518Octets;
	} PON_StatsData_Table64;

 typedef struct {
 		/*onu ber*/
		unsigned long onu_Error_bytes[66];
		long double onu_Used_byte_count[66];
		long double onu_Good_byte_count[66];

		/*olt ber*/
		unsigned long olt_Error_bytes;
		long double olt_Byte_count;
		unsigned long olt_Error_ONU_count;
		long double olt_ONU_byte_count;
		long double olt_ONU_total_bytes_count;

		/*onu frame  65 为总的计数，66*/
		unsigned long onu_Received_Error[66];
		unsigned long onu_Received_OK[66];
		unsigned long onu_Firmware_rec_OK[66];

		/*3 - LLID-Broadcast frames*/
		unsigned long llid_LLID_frames;
		unsigned long llid_Brdcst_frames_rec;
		unsigned long llid_Invalid_err_frames_rec;
		unsigned long llid_brdcst_err_frames_rec;

		/*4 - Total frames (per physical port)*/
		unsigned long frame_Rec_OK;
		unsigned long frame_Rec_err;
		unsigned long frame_Tran_OK;
		unsigned long frame_Total_bad_rec;

		/*5 - Total octets (per physical port)*/
		unsigned long Octets_rec_ok;
		unsigned long Oct_Rec_err;
		unsigned long Oct_tran_ok;
		unsigned long Oct_Rec_total;

		/*6 - Transmitted bytes (octets) per LLID*/
		long double llid_tran_ok;

		/*7 - Transmitted frames per priority*/
		unsigned long pri_Tran_ok[8];

		/*8 - Transmitted frames per LLID*/
		unsigned long frame_Pon_ok;
		unsigned long frame_sys_ok;

		/*9 - Dropped frames (per priority)*/
		unsigned long frame_drop[8];

		/*10 - Dropped frames per LLID*/
		unsigned long Policer_dropped;
		unsigned long Mismatch_dropped;

		/*11 - Total dropped RX frames*/
		unsigned long Total_Control_dropped;
		unsigned long Source_alert_dropped;
		unsigned long PON_match_addr_dropped;

		/*12 - Total dropped TX frames*/
		unsigned long Total_PON_dropped;
		unsigned long Total_System_dropped;
 }Statistics_debug_Table;


typedef struct _statistc_HISTORY_CHAIN_{

	unsigned short year;
	unsigned short hour;	
	unsigned char month;
	unsigned char day;
	unsigned char minute;
	unsigned char second;	
	unsigned long MillSecond;
	unsigned long currentTick;
	BOOL		  bstatus;
	PON_StatsData_Table64 hisStats;

	struct _statistc_HISTORY_CHAIN_ *pPre;
	/*struct _statistc_HISTORY_CHAIN_ *pCurrent;*/
	struct _statistc_HISTORY_CHAIN_ *pNext;
}statistc_HISTORY_CHAIN;


/*modified by wutw 测量对象表数据结构：*/
typedef struct _statistic_PON_OID_
{
	short int			slotIdx;
	short int			ponIdx;
	short int			onuIdx ;

	/*added by wangxy 2007-04-12*/
	short int			ethIdx;
	
	unsigned short			maxBckets;
	/*(24小时* 60 ) / 15Min = 96,每采集96次15分钟的数据,
	就必须换下一个存贮空间*/
	unsigned int		samplingCount24H;
	BOOL			iUsed15Min;
	BOOL			iUsed24Hour;
	BOOL 			firstFlag15M;
	BOOL 			firstFlag24H;
	/*PON_StatsData_Table statsOld;*/
	/*unsigned long		ulStatsOld15Min;
	unsigned long		ulStatsOld24Hour;*/

	statistc_HISTORY_CHAIN *pstats15MinHead;
	statistc_HISTORY_CHAIN *pCurrent15M;/*当前要纪录*/	
	statistc_HISTORY_CHAIN *pstats15MinTail;
	statistc_HISTORY_CHAIN *pstats24HourHead;
	statistc_HISTORY_CHAIN *pCurrent24H;
	statistc_HISTORY_CHAIN *pstats24HourTail;

	/*纪录上一个数据统计的值*/
	PON_StatsData_Table64	 *pstatsDataPre;
	/*unsigned short StatsInterval;*/
	/*WDOG_ID	statsWdocId;*/
	/*statistic_PON_OID *pSelfadd;*/
	struct	_statistic_PON_OID_ *pPre;
	struct	_statistic_PON_OID_ *pNext;

} statistic_PON_OID;


typedef  statistic_PON_OID statistic_ETH_PORT;

typedef  struct _statistic_ONU_OID_{

	unsigned char		ponIndex;
	unsigned char		onuIndex;
	unsigned short		maxBckets;
	BOOL				iUsed15Min;
	BOOL				iUsed24Hour;
	BOOL 				firstFlag ;

	/*struct _statistic_ONU_OID_  statsOld;*/
	struct _statistic_ONU_OID_  *pstats15Min;
	struct _statistic_ONU_OID_  *pstats24Hour;
	struct _statistic_ONU_OID_  *pSelfadd;
	struct _statistic_ONU_OID_  *pPre;
	struct _statistic_ONU_OID_  *pNext;
} statistic_ONU_OID;
	

/* historyStatsMsg receive */
typedef struct  
{
	short int			slotId;
	short int			ponId;
	short int			onuId;
	unsigned int 		bucketNum;
	unsigned int 		bucketNum15m;
	unsigned int 		bucketNum24h;
	unsigned int 		bcktNum15mOld;
	unsigned int 		bcktNum24hOld;	
	BOOL 			flag15M;
	BOOL 			flag24H;
	statistic_PON_OID	*pSelf;
	unsigned char 		statsType;    
	unsigned char 	userd;
	unsigned char		*pBuf;
	signed	int		errCode;
	uchar_t			actcode;
}historyStatsMsg;

#if 0
/* Uplink / downlink network directions */
typedef enum
{  
	PON_DIRECTION_UPLINK,
	PON_DIRECTION_DOWNLINK,
	PON_DIRECTION_UPLINK_AND_DOWNLINK
} PON_pon_network_traffic_direction_t;
#endif
/*------------------------------------------*/

#if 0
/*added by wutw at 19 October for debug cli*/
/* LLID-Broadcast frames raw statistics */
typedef struct
{
	unsigned long  llid_frames_received;					/* Received frames with valid LLID (and not*/ 
															/* preamble broadcast)					   */
	unsigned long  broadcast_frames_received;				/* Received frames with valid preamble     */
															/* broadcast (and not LLID)				   */
    unsigned long  invalid_llid_error_frames_received;		/*Dropped frames due to different LLID (and*/
															/* not broadcast). Any LLID which isn't    */
															/* allocated (to the collector ONU).	   */
	unsigned long  llid_broadcast_error_frames_received;	/*Dropped frames due to broadcast LLID plus*/
															/* one of ONU LLIDs (frame reflection)	   */
} PON_llid_Broadcast_frames_raw_data_t;

/* Transmitted bytes per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID including discovery frames)
*/
typedef struct
{
	long double  transmitted_ok;				/* Transmitted bytes (through the PON port) per LLID */
} PON_transmitted_bytes_per_llid_raw_data_t;

/* (Received from the UNI port and) Transmitted frames per priority raw statistics */
typedef struct
{
	/* Number of frames, received from the UNI port and transmitted to the PON port, per priority */
	unsigned long  transmitted_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 							  
} PON_transmitted_frames_per_priority_raw_data_t;

/* Transmitted frames per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID including discovery frames)
*/
typedef struct
{
	unsigned long  pon_ok;		 /* Transmitted frames (through the PON port) to the LLID			    */
	unsigned long  system_ok;	 /* Transmitted frames (through the System port) originated by the LLID */
} PON_transmitted_frames_per_llid_raw_data_t;

/* Dropped frames raw statistics */
typedef struct
{
	unsigned long  dropped[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1];	/* Dropped frames (to transmit */
													                    /* to the PON) due to queue    */
																		/* overflow per priority	   */
} PON_dropped_frames_raw_data_t;

/* Dropped frames per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID)
*/
typedef struct
{
	unsigned long  policer_dropped;		/* Frames, destined to the LLID, dropped by downlink policer   */
	unsigned long  mismatch_dropped;	/* Frames dropped due to LLID mismatch (frames received when   */
										/* LLID not granted)										   */
} PON_dropped_frames_per_llid_raw_data_t;


typedef unsigned char mac_address_t [6];
/* OAM Configuration field definitions */
typedef enum
{
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_INFORMATION_SUPPORTED    = 0x1, /* DTE supports interpreting Organization Specific Information TLVs		   */
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_NFORMATION_NOT_SUPPORTED = 0x0  /* DTE does not support interpreting Organization Specific Information TLVs */
} PON_oam_organization_specific_information_tlv_t;

typedef enum
{
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_SUPPORTED		= 0x1, /* DTE supports interpreting Organization Specific Events		 */
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_NOT_SUPPORTED = 0x0  /* DTE does not support interpreting Organization Specific Events */
} PON_oam_organization_specific_events_t;

typedef enum
{
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_SUPPORTED		= 0x1, /* DTE supports interpreting Organization Specific OAMPDU		  */
	OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_NOT_SUPPORTED	= 0x0  /* DTE does not support interpreting Organization Specific OAMPDUs */
} PON_oam_organization_specific_oampdu_t;

typedef enum
{
	OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED		= 0x1, /* DTE supports sending Variable Response OAMPDUs		 */
	OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED	= 0x0  /* DTE does not support sending Variable Response OAMPDUs */
} PON_oam_variable_retrieval_t;

typedef enum
{
	OAM_INTERPRETING_LINK_EVENTS_SUPPORTED		= 0x1, /* DTE supports interpreting Link Events */
	OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED	= 0x0  /* DTE does not support interpreting Link Events */
} PON_oam_link_events_t;

typedef enum
{
	OAM_LOOPBACK_SUPPORTED		= 0x1, /* DTE is capable of OAM loopback mode	  */
	OAM_LOOPBACK_NOT_SUPPORTED  = 0x0  /* DTE is not capable of OAM loopback mode */
} PON_oam_2_0_loopback_support_t;

typedef enum
{
	OAM_UNIDIRECTIONAL_SUPPORTED     = 0x1,	/* DTE is capable of sending OAMPDUs when the receive path is non-operational     */
	OAM_UNIDIRECTIONAL_NOT_SUPPORTED = 0x0	/* DTE is not capable of sending OAMPDUs when the receive path is non-operational */
} PON_oam_2_0_unidirectional_support_t;


/*  The differnce betwennn PON_oam_2_0_mode_t and PON_oam_2_0_remote_mode_t is because the difference in OAM draft 2.0  */
typedef enum
{
	OAM_STATUS_MODE_ACTIVE	= 0x2,	/* DTE configured in Active mode  */
	OAM_STATUS_MODE_PASSIVE	= 0x1	/* DTE configured in Passive mode */
} PON_oam_status_2_0_mode_t;

typedef enum
{
	OAM_MODE_ACTIVE	 = 0x1,	/* DTE configured in Active mode  */
	OAM_MODE_PASSIVE = 0x0	/* DTE configured in Passive mode */
} PON_oam_2_0_mode_t;

typedef struct
{
	PON_oam_organization_specific_information_tlv_t  organization_specific_information;
	PON_oam_organization_specific_events_t			 organization_specific_events;
	PON_oam_organization_specific_oampdu_t			 organization_specific_oampdu;
	PON_oam_variable_retrieval_t					 variable_retrieval;
	PON_oam_link_events_t							 link_events;
	PON_oam_2_0_loopback_support_t					 loopback_support;
	PON_oam_2_0_unidirectional_support_t			 unidirectional_support;
	PON_oam_2_0_mode_t								 oam_mode;
} PON_oam_2_0_configuration_t;

typedef struct
{
	bool	oam_2_0_local_stable;
	bool	oam_2_0_remote_stable;
	bool	oam_2_0_critical_event;
	bool	oam_2_0_dying_gasp;
	bool	oam_2_0_link_fault;
}PON_oam_2_0_flags_t;
/* State field definitions */
typedef enum
{
	OAM_MULTIPLEXER_ACTION_FORWARDING = 0x0, /* Device is forwarding non-OAMPDUs to the lower sublayer */
	OAM_MULTIPLEXER_ACTION_DISCARDING = 0x1  /* Device is discarding non-OAMPDUs					   */
} PON_oam_multiplexer_action_t;

typedef enum
{
	OAM_PARSER_ACTION_FORWARDING	= 0x0, /* Device is forwarding non-OAMPDUs to higher sublayer */
	OAM_PARSER_ACTION_LOOPING_BACK	= 0x1, /* Device is looping back non-OAMPDUs to the lower sublayer */
	OAM_PARSER_ACTION_DISCARDING	= 0x2, /* Device is discarding non-OAMPDUs */
	OAM_PARSER_ACTION_RESERVED		= 0x3  /* Reserved. This value shall not be sent. If the value is received, it shall be ignored and not change the last received value. */
} PON_oam_parser_action_t;
typedef struct
{
	PON_oam_multiplexer_action_t  multiplexer_action;
	PON_oam_parser_action_t		  parser_action;
} PON_oam_2_0_state_t;

/* OAM information data raw statistics */
typedef struct
{
	mac_address_t				 oam_remote_mac_adress;				 /* The mac address of the remote OAM according to last OAMPDU																		*/
	PON_oam_2_0_configuration_t  oam_remote_configuration;			 /* A group of eight bits corresponding to the OAM Configuration field																*/
	short int					 oam_remote_pdu_configuration;		 /* The maximum PDU, in octets, supported by the DTE																					*/
	PON_oam_2_0_flags_t			 oam_local_flags_field;				 /* A group of five bits corresponding to the Flags field in the most recently transmitted OAMPDU.									*/
	PON_oam_2_0_flags_t			 oam_remote_flags_field;			 /* A group of five bits corresponding to the Flags field in the most recently received OAMPDU.										*/
	PON_oam_2_0_state_t			 oam_remote_state;					 /* A group of three bits corresponding to the State field of the most recently received Information OAMPDU							*/
	unsigned long				 oam_remote_vendor_oui;				 /* The value of the OUI variable in the Vendor Identifier field of the most recently received Information OAMPDU.					*/
	short int					 oam_remote_vendor_id_device_number; /* The value of the Device (product) identifier variable in the Vendor Identifier of the most recently received Information OAMPDU  */
	short int					 oam_remote_vendor_id_version;		 /* The value of the Version Identifier variable in the Vendor Identifier field of the most recently received Information OAMPDU		*/
} PON_oam_information_data_raw_data_t;
#endif

extern STATUS HisStatsPon15MModified (short int ponId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsPon24HModified (short int ponId, unsigned int bucketNum,  BOOL flag24H);
extern STATUS HisStatsOnu15MModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag15M);
extern STATUS HisStatsOnu24HModified(short int ponId, short int onuId, unsigned int bucketNum, BOOL flag24H);
extern STATUS HisStatsEthFirstEntry( statistic_ETH_PORT **ppStatsEth, statistc_HISTORY_CHAIN **ppStatsChain, UCHAR type );
extern STATUS HisStatsEthNextEntry(statistic_ETH_PORT *pstatEth, statistc_HISTORY_CHAIN *pStatschain, 
								statistic_ETH_PORT **ppstatEth, statistc_HISTORY_CHAIN **ppStatschain , UCHAR type);

/*以下为获取olt pon端口(15分钟和24 小时)的历史统计数据*/
extern STATUS HisStatsPon15MTableFirstIndexGet(statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsPon15MTableNextIndexGet(statistic_PON_OID *pstatPon, statistc_HISTORY_CHAIN *pStatschain, 
								statistic_PON_OID **ppstatPonIdx, statistc_HISTORY_CHAIN **ppStatschainIdx);

extern STATUS HisStatsPon15MTableGet(short int StatsObjType, statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData);

extern STATUS HisStatsPon24HTableFirstIndexGet(statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsPon24HTableNextIndexGet( statistic_PON_OID *pstatPon,statistc_HISTORY_CHAIN *pStatsChain, 
									statistic_PON_OID **ppstatsPonIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsPon24HTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData);


/*以下为获取onu pon端口的(15分钟和24 小时)的历史统计数据*/
extern STATUS HisStatsOnu15MTableFirstIndexGet(statistic_PON_OID **ppStatsOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsOnu15MTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsOnu15MTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData);

extern STATUS HisStatsOnu24HTableFirstIndexGet(statistic_PON_OID **ppstatPonIdx ,statistc_HISTORY_CHAIN **ppStatsChainIdx);


extern STATUS HisStatsOnu24HTableNextIndexGet(statistic_PON_OID *pstatOnu, statistc_HISTORY_CHAIN *pStatsChain, 
								statistic_PON_OID **ppstatOnuIdx, statistc_HISTORY_CHAIN **ppStatsChainIdx);

extern STATUS HisStatsOnu24HTableGet(short int StatsObjType,statistc_HISTORY_CHAIN *pstatsIdx, unsigned long *pStatsData);


/**/
extern STATUS HisStatsOnuCtrlTableFirstIndexGet(VOID **ppStatsIdx);

extern STATUS HisStatsOnuCtrlTableNextIndexGet(statistic_PON_OID *pstatsPon, statistic_PON_OID **ppStatsIdx);

extern STATUS HisStatsOnuCtrlTableGet(statistic_PON_OID	*pstatsPon, statistic_PON_OID **ppStatsData);

extern STATUS HisStatsOnuCtrlEntryGet(short int SlotId, short int PonId,short int OnuId, statistc_HISTORY_CHAIN **ppHisStats24HHead, statistc_HISTORY_CHAIN **ppHisStats15MHead);

extern STATUS HisStatsOnuCtrlTablePonIdGet(statistic_PON_OID *pHisStatsPon, short int *pSlotId, short int *pPonId, short int *pOnuId);


/**/
extern STATUS SHisStatsPonCtrlEntryGet(short int SlotId, short int PonId, statistc_HISTORY_CHAIN **ppHisStats24HHead, statistc_HISTORY_CHAIN **ppHisStats15MHead);

extern STATUS HisStatsPonCtrlTablePonIdGet(statistic_PON_OID *pHisStatsPon, short int *pSlotId, short int *pPonId);

extern STATUS HisStatsPonCtrlTableFirstIndexGet(/*short int SlotId, short int PonId, */statistic_PON_OID **ppStatsIdx);

extern STATUS HisStatsPonCtrlTableNextIndexGet(/*short int SlotId, short int PonId, */statistic_PON_OID *pstatsPon, statistic_PON_OID **ppStatsIdx);

extern STATUS HisStatsPonCtrlTableGet(/*short int SlotId, short int PonId, *//* short int StatsObjType,*/statistic_PON_OID *pstatsIdx, statistic_PON_OID **ppStatsData);


/*以下为设置和获取15m .24hours的最大记录*/
extern STATUS HisStats15MinMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordSet(unsigned short value);
extern STATUS HisStats24HoursMaxRecordGet(unsigned int *pValue);
extern STATUS HisStats15MinMaxRecordGet(unsigned int *pValue);



/****************************************************
* HisStats24HoursStatusGet
* description : 查看pon 设备24 hour 历史统计使能状态
*
*
*
*****************************************************/
extern BOOL HisStatsPon24HoursStatusGet(unsigned short  ponId);


/****************************************************
* HisStatsPon15MinStatusGet
* description : 查看pon 设备15 Min 历史统计使能状态
*
*
*
*****************************************************/
extern BOOL HisStatsPon15MinStatusGet(unsigned short  ponId);

/****************************************************
* HisStatsOnu24HoursStatusGet
* description : 查看pon 设备24 hour 历史统计使能状态
*
*
*
*****************************************************/
extern BOOL HisStatsOnu24HoursStatusGet(unsigned short  ponId, unsigned short  onuId);
/****************************************************
* HisStatsOnu15MinStatusGet
* description : 查看pon 设备15 Min 历史统计使能状态
*
*
*
*****************************************************/
extern BOOL HisStatsOnu15MinStatusGet(unsigned short  ponId, unsigned short  onuId);


/****************************************************
*
* ClearOltPonPortStatisticCounter()
* ClearOnuPonPortStatisticCounter()
* description : 用于清零OLT 及ONU 上软件及硬件统计计数器
*
*****************************************************/
extern  int  ClearOltPonPortStatisticCounter(short int PonPortIdx );
extern  int  ClearOnuPonPortStatisticCounter(short int PonPortIdx, short int OnuIdx );




#ifdef __cplusplus
}
#endif
#endif
