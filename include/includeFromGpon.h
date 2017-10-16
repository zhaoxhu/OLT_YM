/**************************************************************
*
*   IncludeFromGpon.h -- 
*
*  
*    Copyright (c)  2015.11 , GWD Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ----------|--- --------|---------------------|------------
*	1.00	        | 11/06/2015  | Creation				| jinhl
*
***************************************************************/
#ifndef  __INCLUDEFROMGPON_H
#define __INCLUDEFROMGPON_H


#ifndef PACK
#define PACK __attribute__((packed))
#endif

#if defined(_GPON_BCM_SUPPORT_)


#define ONU_SEQUENCENUM_SIZE (16 + 1)

typedef struct GPON_olt_initialization_parameters_t
{
    unsigned char   	    olt_mac_address[6];
    unsigned char           firmware_version[100];
	void				    *firmware_location;
	long int				firmware_size;
} GPON_olt_initialization_parameters_t;

/* Handler functions enum */
typedef enum
{
    GPON_HANDLER_LINK_ACTIVE,				           
    GPON_HANDLER_LINK_LOSS,				              
    GPON_HANDLER_GPIO_CHANGED,
    GPON_HANDLER_OLT_ADD,
    GPON_HANDLER_OLT_REMOVE,
    GPON_HANDLER_OLT_RESET,
    GPON_HANDLER_PON_LOSS,
    GPON_HANDLER_CNI_LINK,
    GPON_HANDLER_ALARM,
    GPON_HANDLER_ONU_DYING_GASP,
    GPON_HANDLER_ONU_REGISTRATION,
    GPON_HANDLER_ONU_DEREGISTRATION,
    
    GPON_HANDLER_MAX

}GPON_handler_functions_index_t;

typedef enum
{
	GPON_PM_PONLINK_ACTIVE_COUNTER = 1024,/*为了区别EPON中统计时collector_id作为llid使用，这里collector_id作为类型使用*/
	GPON_PM_PONNNI_ACTIVE_COUNTER,
	GPON_PM_PONGEM_ACTIVE_COUNTER,
	GPON_PM_MAX
}GPON_PM_t;

typedef struct GPON_add_olt_result_t
{
	unsigned char  olt_mac_address[6];
} GPON_add_olt_result_t;


typedef struct {
  unsigned short SLA_CLASS;         /* 0 (lowest priority) ?7 (highest priority) */
  unsigned short fixed_bw;			/* Fixed guaranteed bandwidth (1Mbps units)  */
  unsigned short fixed_bw_fine;     /* Fixed guaranteed bandwidth (64Kbps units)  */
  unsigned short max_gr_bw;         /* Max guaranteed bandwidth (1Mbps units)  */
  unsigned short max_gr_bw_fine;    /* Max guaranteed bandwidth (64Kbps units)  */
  unsigned short max_be_bw;         /* Max best effort bandwidth (1Mbps units) */
  unsigned short max_be_bw_fine;    /* Max best effort bandwidth (64Kbps units) */
} GPON_SLA_t;

typedef struct
{
	uint64 fec_codewords;             /**< Received FEC codewords. */
    uint64 fec_codewords_uncorrected; /**< Received uncorrected FEC codewords. */
    uint64 bip8_bytes;                /**< Received bytes protected by bip8. */
    uint64 bip8_errors;               /**< Received bip8 errors. */
    uint64 rx_gem_packets;            /**< Received GEM frames. */
    uint64 rx_gem_dropped;            /**< Received dropped GEM ID packets. */
    uint64 rx_gem_idle;               /**< Received idle GEM frames. */
    uint64 rx_gem_corrected;          /**< Received corrected GEM frames. */
    uint64 rx_gem_illegal;            /**< Received illegal GEM frames. */
    uint64 rx_allocations_valid;      /**< Received valid allocations. */
    uint64 rx_allocations_invalid;    /**< Received invalid allocations. */
    uint64 rx_allocations_disabled;   /**< Received disabled allocations. */
    uint64 rx_ploams;                 /**< Received Ploams. */
    uint64 rx_ploams_non_idle;        /**< Received non idle Ploams. */
    uint64 rx_ploams_error;           /**< Received error Ploams. */
    uint64 rx_ploams_dropped;         /**< Received dropped Ploams. */
    uint64 rx_cpu;                    /**< Received CPU packets. */
    uint64 rx_omci;                   /**< Received OMCI packets. */
    uint64 rx_dropped_too_short;      /**< Received packets dropped due to length too short. */
    uint64 rx_dropped_too_long;       /**< Received packet dropped due to length too long. */
    uint64 rx_crc_errors;             /**< Received packet dropped due to crc error. */
    uint64 rx_key_errors;             /**< Received packet dropped due to key error. */
    uint64 rx_fragments_errors;       /**< Received packet dropped due to fragmentation error. */
    uint64 rx_packets_dropped;        /**< Global dropped packets. */
    uint64 tx_gem;                    /**< Transmitted GEM frames. */
    uint64 tx_ploams;                 /**< Transmitted Ploams. */
    uint64 tx_cpu;                    /**< Transmitted CPU packets. */
    uint64 tx_omci;                   /**< Transmitted OMCI packets. */
    uint64 tx_dropped_illegal_length; /**< Transmitted packet dropped due to illegal length. */
    uint64 tx_dropped_tpid_miss;      /**< Dropped because of TPID miss. */
    uint64 tx_dropped_vid_miss;       /**< Dropped because of VID miss. */

}tOgCmPmPonLinkNewCounter;

typedef struct
{

    uint64 rx_frames_64;              /**< The count of RX 64 byte frames on this NNI. */
    uint64 rx_frames_65_127;          /**< The count of RX 65 to 127 byte frames on this NNI. */
    uint64 rx_frames_128_255;         /**< The count of RX 128 to 255 byte frames on this NNI. */
    uint64 rx_frames_256_511;         /**< The count of RX 256 to 511 byte frames on this NNI. */
    uint64 rx_frames_512_1023;        /**< The count of RX 512 to 1023 byte frames on this NNI. */
    uint64 rx_frames_1024_1518;       /**< The count of RX 1024 to 1518 byte frames on this NNI. */
    uint64 rx_frames_1519_2047;       /**< The count of RX 1519 to 2047 byte frames on this NNI. */
    uint64 rx_frames_2048_4095;       /**< The count of RX 2048 to 4095 byte frames on this NNI. */
    uint64 rx_frames_4096_9216;       /**< The count of RX 4096 to 9216 byte frames on this NNI. */
    uint64 rx_frames_9217_16383;      /**< The count of RX 9217 to 16383 byte frames on this NNI. */
    uint64 rx_frames;                 /**< The number of received frames on this NNI. This includes all errored frames as well. */
    uint64 rx_bytes;                  /**< The number of received bytes on this NNI. This includes all errored frames as well. */
    uint64 rx_good_frames;            /**< The number of received good frames on this NNI. */
    uint64 rx_unicast_frames;         /**< The number of received unicast frames on this NNI. */
    uint64 rx_multicast_frames;       /**< The number of received multicast frames on this NNI. */
    uint64 rx_broadcast_frames;       /**< The number of received broadcast frames on this NNI. */
    uint64 rx_fcs_errors;             /**< The number of received FCS errors on this NNI. */
    uint64 rx_control_frames;         /**< The number of received control frames on this NNI. */
    uint64 rx_pause_frames;           /**< The number of received pause frames on this NNI. */
    uint64 rx_pfc_frames;             /**< The number of received PFC frames on this NNI. */
    uint64 rx_unsupported_opcode;     /**< The number of received Unsupported Opcode frames on this NNI. */
    uint64 rx_unsupported_da;         /**< The number of received unsupported DA frames on this NNI. */
    uint64 rx_alignment_errors;       /**< The number of received alignment errors on this NNI. */
    uint64 rx_length_out_of_range;    /**< The number of received length out of range errors on this NNI. */
    uint64 rx_code_errors;            /**< The number of received code errors on this NNI. */
    uint64 rx_oversized_frames;       /**< The number of received oversized frames on this NNI. */
    uint64 rx_jabber_frames;          /**< The number of received jabber frames on this NNI. these are oversized frames that also contain an invalid CRC, code error, or IEEE length check error. */
    uint64 rx_mtu_check_errors;       /**< The number of received MTU Check Errors on this NNI. */
    uint64 rx_promiscuous_frames;     /**< The number of received frames on this NNI that are not control packets and have a DA that is not matching with the RX SA. */
    uint64 rx_vlan_frames;            /**< The number of received VLAN tagged frames on this NNI (with TPID 8100). This counts both single and double tagged frames. */
    uint64 rx_double_vlan_frames;     /**< The number of received double VLAN tagged frames on this NNI (with TPID 8100).  */
    uint64 rx_truncated_frames;       /**< The number of received truncated frames on this NNI. This is likely due to RX FIFO Full.  */
    uint64 rx_undersize_frames;       /**< The number of received undersized frames on this NNI. */
    uint64 rx_fragmented_frames;      /**< The number of received fragmented frames on this NNI. */
    uint64 rx_runt_frames;            /**< The number of received runt frames on this NNI. */
    uint64 tx_frames_64;              /**< The count of TX 64 byte frames on this NNI. */
    uint64 tx_frames_65_127;          /**< The count of TX 65 to 127 byte frames on this NNI. */
    uint64 tx_frames_128_255;         /**< The count of TX 128 to 255 byte frames on this NNI. */
    uint64 tx_frames_256_511;         /**< The count of TX 256 to 511 byte frames on this NNI. */
    uint64 tx_frames_512_1023;        /**< The count of TX 512 to 1023 byte frames on this NNI. */
    uint64 tx_frames_1024_1518;       /**< The count of TX 1024 to 1518 byte frames on this NNI. */
    uint64 tx_frames_1519_2047;       /**< The count of TX 1519 to 2047 byte frames on this NNI. */
    uint64 tx_frames_2048_4095;       /**< The count of TX 2048 to 4095 byte frames on this NNI. */
    uint64 tx_frames_4096_9216;       /**< The count of TX 4096 to 9216 byte frames on this NNI. */
    uint64 tx_frames_9217_16383;      /**< The count of TX 9217 to 16383 byte frames on this NNI. */
    uint64 tx_frames;                 /**< The number of transmitted frames on this NNI. This includes all errored frames as well. */
    uint64 tx_bytes;                  /**< The number of transmitted bytes on this NNI. This includes all errored frames as well. */
    uint64 tx_good_frames;            /**< The number of transmitted good frames on this NNI. */
    uint64 tx_unicast_frames;         /**< The number of transmitted unicast frames on this NNI. */
    uint64 tx_multicast_frames;       /**< The number of transmitted multicast frames on this NNI. */
    uint64 tx_broadcast_frames;       /**< The number of transmitted broadcast frames on this NNI. */
    uint64 tx_pause_frames;           /**< The number of transmitted pause frames on this NNI. */
    uint64 tx_pfc_frames;             /**< The number of transmitted PFC frames on this NNI. */
    uint64 tx_jabber_frames;          /**< The number of transmitted jabber frames on this NNI. These are oversized frames that also contain an invalid FCS. */
    uint64 tx_fcs_errors;             /**< The number of transmitted FCS errors on this NNI.  */
    uint64 tx_control_frames;         /**< The number of transmitted control frames on this NNI.  */
    uint64 tx_oversize_frames;        /**< The number of transmitted oversized frames on this NNI.  */
    uint64 tx_fragmented_frames;      /**< The number of transmitted fragmented frames on this NNI.  */
    uint64 tx_error_frames;           /**< The number of transmitted errored frames on this NNI.  */
    uint64 tx_vlan_frames;            /**< The number of transmitted VLAN tagged frames on this NNI (with TPID 8100). This counts both single and double tagged frames. */
    uint64 tx_double_vlan_frames;     /**< The number of transmitted double VLAN tagged frames on this NNI (with TPID 8100).  */
    uint64 tx_runt_frames;            /**< The number of transmitted runt frames on this NNI.  */
    uint64 tx_underrun_frames;        /**< The number of transmitted underrun frames on this NNI. Thus happens when a frame encounters a MAC underrun (Tx Sync FIFO runs out of data before the end of packet). */
	
}tOgCmPmPonNNICounter;

typedef struct
{
	uint64 rx_packets;    /**< Received GEM frames. */
    uint64 rx_bytes;      /**< Received bytes. */
    uint64 tx_packets;    /**< Transmitted GEM frames. */
    uint64 tx_bytes;      /**< Transmitted bytes. */   
}tOgCmPmPonGemCounter;

#if 0
typedef enum
{
	GPONADP_ONU_TCONT_TYPE_MIN,
    GPONADP_ONU_TCONT_TYPE_1,
    GPONADP_ONU_TCONT_TYPE_2,
    GPONADP_ONU_TCONT_TYPE_3,
    GPONADP_ONU_TCONT_TYPE_4,
    GPONADP_ONU_TCONT_TYPE_5,
    GPONADP_ONU_TCONT_TYPE_MAX
}GPONADP_ONU_TCONT_TYPE_E;

typedef struct
{
	unsigned int  bwAssured;  /* bandwidth, unit: Kbit/s*/
    unsigned int  bwMax;  /* unit: Kbit/s */
    unsigned int  bwFixed;/* unit: Kbit/s */
	GPONADP_ONU_TCONT_TYPE_E tcontType;
}GPONADP_ONU_BW;

typedef enum
{
    GPONADP_ONU_STATE_MIN,
    GPONADP_ONU_STATE_ENABLE,
    GPONADP_ONU_STATE_DISABLE,
    GPONADP_ONU_STATE_MAX
} GPONADP_ONU_STATE_E;
#endif
extern int gpon_init();
extern int gponOltAdp_ActivePon(int oltId);
extern int gponOltAdp_StartSnAcq(short int olt_id);
extern int gponOltAdp_StopSnAcq(short int olt_id);


extern bool gponOltAdp_IsExist(short int olt_id);
extern int gponOltAdp_Reset(short int olt_id);
extern int gponOltAdp_RemoveOlt(short int olt_id, bool shutDown, bool reset);
extern int gponOltAdp_GetVersion(short int olt_id, PON_device_versions_t *device_versions);
extern int gponOltAdp_GetDbaVersion(short int olt_id, OLT_DBA_version_t *dba_version);
extern int gponOltAdp_SetAdmin(short int olt_id, bool admin);
extern int gponOltAdp_GetAdmin(short int olt_id, bool *admin);
extern int gponOltAdp_GetCniLinkStatus(short int olt_id, bool *status);
extern int gponOltAdp_SetMaxRange(short int olt_id, int max_range);
extern int gponOltAdp_SetRssi(short int olt_id, int onu_id);
extern int gponOltAdp_GetPonLinkCounter(short int olt_id, tOgCmPmPonLinkNewCounter *counter);
extern int gponOltAdp_GetPonNNICounter(short int olt_id, tOgCmPmPonNNICounter *counter);
extern int gponOltAdp_GetPonGemCounter(short int olt_id, short int gemId, tOgCmPmPonGemCounter *counter);
extern int gponOltAdp_ClearPonLinkCounter(short int olt_id);
extern int gponOltAdp_ClearPonNNICounter(short int olt_id);
extern int gponOltAdp_ClearPonGemCounter(short int olt_id, short int gemId);
extern void gponOltAdp_DevReInit();
extern int gponOltAdp_DevDisconnect();
extern int gponOltAdp_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl);
extern int gponOltAdp_GetMacAddrVlanTbl(short int olt_id, short int *addr_num, PON_address_vlan_table_t addr_tbl);
#if 0
extern int gponOnuAdp_GetSn(short int olt_id, short int onu_id, uint8 serialNumber[ONU_SEQUENCENUM_SIZE]);
extern int gponOnuAdp_DeregOnu(int oltId, int onuId);
extern int gponOnuAdp_GetOnuMacAddress(short int olt_id, short int onu_id, uint8 *onuMacAddress);
extern int gponOnuAdp_SetUpBW(int oltId, int onuId, GPONADP_ONU_BW bw);
extern int gponOnuAdp_GetUpBW(int oltId, int onuId, GPONADP_ONU_BW *bw);
extern bool gponOnuAdp_isOnline(int oltId, int onuId);
extern int gponOltAdp_GetOpticalMode(short int olt_id, int *tx_mode);
extern int gponOltAdp_SetOpticalMode(short int olt_id, int tx_mode);

extern int gponOnuAdp_SetFec(int oltId, int onuId, GPONADP_ONU_STATE_E mode);
extern int gponOnuAdp_GetFec(int oltId, int onuId, GPONADP_ONU_STATE_E *mode);
#endif
extern int gponAdapter_PAS5201_Init();
extern int gpon_assign_pashandler_function( const PAS_handler_functions_index_t     handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );

extern int gpon_assign_handler_function
                                     ( const GPON_handler_functions_index_t     handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id );


extern int gpon_add_olt 
                    ( const PON_olt_id_t  				          olt_id, 
				            GPON_olt_initialization_parameters_t  *olt_initialization_parameters,
                            GPON_add_olt_result_t                 *add_olt_result );

extern int gpon_isStartExp();
extern void gpon_setStartExp(int flag);

#endif



#endif 
