

/*#include "../PAS/Common_components/PAS/PAS_expo.h"
#include "../PAS/Common_components/PON/PON_expo.h"
#include "vxworks.h"*/
#include "VOS_Base.h"
#include "VOS_types.h"
#include "Vos_typevx.h"
#include "Vos_sem.h"
#include "Vos_que.h"
#include "Vos_task.h" 

#include "Vos_timer.h"
#include "statistics.h"
#include "../superset/platform/include/man/cli/cli.h"
#include "monitor/monitor.h"
/*
extern short int PAS_get_raw_statistics ( const short int						 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   const PON_statistics_query_method_t	 collecting_method,
								   void									*statistics_data,
								   PON_timestamp_t						*timestamp 
								   );
*/
#define TEST_STAT_DEBUG
#define TEST_PON_BROADCAST_LLID 127
int stat_debug_on = 0;
extern short int GetLlidByOnuIdx( short int PonPortIdx, short int OnuIdx );

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int statDebugOltRealTimeStatisticsVty(short int slotId, short int port, short int PonId,short int OnuId,struct vty *vty)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t  timestamp = 0 ;

	unsigned long long counter = 0;
	char szText[64] = "";

	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData/*,allinRangeLenErrRawData*/;
	/*PON_frame_check_sequence_errors_raw_data_t	SequenceErrorRawData;*/
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
	/*PON_registration_frames_raw_data_t		registertFrameRawData;*/
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
	/*PON_mpcp_status_raw_data_t MPCPstatisticsCounter;*/
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_dropped_frames_per_llid_raw_data_t llidDropFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_bytes_per_llid_raw_data_t llidBrdbytesSendRawData;
	PON_llid_Broadcast_frames_raw_data_t llidBrdFrameRawData;
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;
	OLT_raw_stat_item_t    stat_item;

	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ( (-1) == llid ) 
	{
		return (-1);
	}

	{
		collector_id = -1;	
		stat_parameter = llid;
		
		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_BER;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &onuBerRawData;
	    stat_item.statistics_data_size = sizeof(PON_onu_ber_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuBerRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
			vty_out( vty,"\r\n  %% %d/%d/%d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",slotId,port,OnuId+1,PonId);
		}
	

		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &totalOctetsRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		#endif
		if(iRes != 0)
		{
			vty_out( vty,"\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}	
	}


	{
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_FER;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &onuFerRawData;
	    stat_item.statistics_data_size = sizeof(PON_onu_fer_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",PonId,stat_parameter);
		}

	}


	{
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_LLID_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_llid_Broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
	}

	
	{
		memset(&llidBrdbytesSendRawData, 0, sizeof(PON_transmitted_bytes_per_llid_raw_data_t));
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdbytesSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_transmitted_bytes_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdbytesSendRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}


	
	{
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdFrameSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_transmitted_frames_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}	
	}



	{
		memset(&llidDropFrameSendRawData, 0, sizeof(PON_dropped_frames_per_llid_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidDropFrameSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_dropped_frames_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_DROPPED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidDropFrameSendRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}

	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &TotalDropFrameRevRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_dropped_rx_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		#endif
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}


	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &TotalDropFrameTxRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_tx_dropped_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);
		#endif
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}

	
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_SINGLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &singleCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_single_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &multCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multiple_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}

	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		stat_parameter = llid;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &checkSeqErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_check_sequence_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

	}	

	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &inRangeLenErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_in_range_length_errors_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

		
	}

	
		
	{

		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_HEC_FRAMES_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &hecFrameErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_hec_frames_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	}




	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &mulFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multicast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	
	}

	

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &broadcastFramRawData;
	    stat_item.statistics_data_size = sizeof(PON_broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		
	}
	
		vty_out( vty,"\r\n  %d/%d/%d (ponId %d) \r\n",slotId,port,OnuId+1,PonId);
		memset(buf, 0, 256);


		vty_out( vty,"  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);
		counter = (unsigned long long)onuBerRawData.used_byte_count;
		sprintf64Bits( szText, counter );
		sprintf( buf, "  GoodbytesRev                          :  %s\r\n",szText );
		vty_out( vty,"%s",buf);
		vty_out( vty,"  llidBrdFrmTx                          :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		vty_out( vty,"  llidFrmPolicerDrop                    :  %lu\r\n",llidDropFrameSendRawData.policer_dropped);
		vty_out( vty,"  llidFrmMismatchDrop                   :  %lu\r\n",llidDropFrameSendRawData.mismatch_dropped);
		memset(buf, 0, 256);
		memset( szText, 0, 64 );
		counter = llidBrdbytesSendRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf( buf,"  llidOctetsTx                          :  %s\r\n", szText );
		vty_out( vty,"%s",buf);

		vty_out( vty,"  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		vty_out( vty,"  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		vty_out( vty,"  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		
		vty_out( vty,"  BrdFrmRev                             :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		vty_out( vty,"  llidBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		vty_out( vty,"  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		vty_out( vty,"  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		vty_out( vty,"  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
		vty_out( vty,"  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
		vty_out( vty,"  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);
		vty_out( vty,"  inRangeLenErrRx                       :  %lu\r\n",inRangeLenErrRawData.received);
		vty_out( vty,"  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		vty_out( vty,"  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		vty_out( vty,"  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
		vty_out( vty,"  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		vty_out( vty,"  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);

		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		vty_out( vty,"  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		vty_out( vty,"  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		vty_out( vty,"%s",buf);

		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n", szText );		
		vty_out( vty,"%s",buf);
		return 0;
}





int statDebugOnuRealTimeStatisticsVty(short int slotId, short int port, short int PonId,short int OnuId,struct vty *vty)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t timestamp = 0 ;
	unsigned long long counter = 0;
	char szText[64] = "";
	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData/*,allinRangeLenErrRawData*/;
/*	PON_frame_check_sequence_errors_raw_data_t	SequenceErrorRawData;*/
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
/*	PON_registration_frames_raw_data_t		registertFrameRawData;*/
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
/*	PON_mpcp_status_raw_data_t MPCPstatisticsCounter;*/
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_dropped_frames_per_llid_raw_data_t llidDropFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_bytes_per_llid_raw_data_t llidBrdbytesSendRawData;
	PON_llid_Broadcast_frames_raw_data_t llidBrdFrameRawData;
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_olt_ber_raw_data_t oltBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;
	OLT_raw_stat_item_t    stat_item;

	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ( (-1) == llid ) 
	{
		return (-1);
	}

	{
		collector_id = llid;	
		stat_parameter = 0;
		
		memset(&oltBerRawData, 0, sizeof(PON_olt_ber_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_OLT_BER;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &oltBerRawData;
	    stat_item.statistics_data_size = sizeof(PON_olt_ber_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_OLT_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &oltBerRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
			vty_out( vty,"\r\n  %% %d/%d/%d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",slotId,port,OnuId+1,PonId);
		}
	

		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_OCTETS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &totalOctetsRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_octets_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		#endif
		if(iRes != 0)
		{
			vty_out( vty,"\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}	
	}


	{
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_ONU_FER;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &onuFerRawData;
	    stat_item.statistics_data_size = sizeof(PON_onu_fer_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",PonId,stat_parameter);
		}

	}


	{
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_LLID_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_llid_Broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
	}

	
	{
		memset(&llidBrdbytesSendRawData, 0, sizeof(PON_transmitted_bytes_per_llid_raw_data_t));
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdbytesSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_transmitted_bytes_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdbytesSendRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}


	
	{
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidBrdFrameSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_transmitted_frames_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}	
	}



	{
		memset(&llidDropFrameSendRawData, 0, sizeof(PON_dropped_frames_per_llid_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_DROPPED_FRAMES_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &llidDropFrameSendRawData;
	    stat_item.statistics_data_size = sizeof(PON_dropped_frames_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_DROPPED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidDropFrameSendRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}

	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &TotalDropFrameRevRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_dropped_rx_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		#endif
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}


	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &TotalDropFrameTxRawData;
	    stat_item.statistics_data_size = sizeof(PON_total_tx_dropped_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);
		#endif
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}

	
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_SINGLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &singleCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_single_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &multCollisionFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multiple_collision_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}

	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		stat_parameter = llid;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &checkSeqErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_frame_check_sequence_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

	}	

	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &inRangeLenErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_in_range_length_errors_per_llid_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

		
	}

	
		
	{

		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_HEC_FRAMES_ERRORS;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &hecFrameErrRawData;
	    stat_item.statistics_data_size = sizeof(PON_hec_frames_errors_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	}




	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_MULTICAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &mulFrameRawData;
	    stat_item.statistics_data_size = sizeof(PON_multicast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	
	}

	

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		#if 1
		VOS_MemZero(&stat_item, sizeof(OLT_raw_stat_item_t));
		stat_item.collector_id = collector_id;
	    stat_item.raw_statistics_type = PON_RAW_STAT_BROADCAST_FRAMES;
	    stat_item.statistics_parameter = stat_parameter;
		
	    stat_item.statistics_data = &broadcastFramRawData;
	    stat_item.statistics_data_size = sizeof(PON_broadcast_frames_raw_data_t);
	    iRes = OLT_GetRawStatistics(PonId, &stat_item);
		timestamp = stat_item.timestam;
		#else
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		#endif
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		
	}
	
		vty_out( vty,"\r\n  %d/%d/%d (ponId %d) \r\n",slotId,port,OnuId+1,PonId);
		memset(buf, 0, 256);
/*		vty_out( vty,"  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);*/
/*		sprintf(buf, "  GoodbytesRev                          :  %lu\r\n",onuBerRawData.used_byte_count);
		vty_out( vty,"%s",buf);*/
		vty_out( vty,"  llidBrdFrmTx                          :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		vty_out( vty,"  llidFrmPolicerDrop                    :  0 %lu\r\n",llidDropFrameSendRawData.policer_dropped);
		vty_out( vty,"  llidFrmMismatchDrop                   :  %lu\r\n",llidDropFrameSendRawData.mismatch_dropped);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = llidBrdbytesSendRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf( buf,"  llidOctetsTx                          :  %s\r\n", szText );
		
		vty_out( vty,"%s",buf);
		vty_out( vty,"  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		vty_out( vty,"  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		vty_out( vty,"  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		
		vty_out( vty,"  BrdFrmRev                             :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		vty_out( vty,"  llidBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		vty_out( vty,"  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		vty_out( vty,"  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		vty_out( vty,"  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
		vty_out( vty,"  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
		vty_out( vty,"  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);
		vty_out( vty,"  inRangeLenErrRx                       :  %lu\r\n",inRangeLenErrRawData.received);
		vty_out( vty,"  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		vty_out( vty,"  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		vty_out( vty,"  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
		vty_out( vty,"  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		vty_out( vty,"  GoodbrdEthFrmTx                       :  0%lu\r\n",broadcastFramRawData.transmitted_ok);

		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		vty_out( vty,"  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		vty_out( vty,"  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);
		
		memset(buf, 0, 256);	
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );		
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		vty_out( vty,"%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );				
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n", szText );		
		vty_out( vty,"%s",buf);
		return 0;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

#if 0
int statDebugOltRealTimeStatisticsShow( short int PonId,short int OnuId)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t  timestamp = 0 ;
	unsigned long long counter = 0;
	char szText[64] = "";
	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData/*,allinRangeLenErrRawData*/;
	/*PON_frame_check_sequence_errors_raw_data_t	SequenceErrorRawData;*/
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
	/*PON_registration_frames_raw_data_t		registertFrameRawData;*/
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
	/*PON_mpcp_status_raw_data_t MPCPstatisticsCounter;*/
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_dropped_frames_per_llid_raw_data_t llidDropFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_bytes_per_llid_raw_data_t llidBrdbytesSendRawData;
	PON_llid_Broadcast_frames_raw_data_t llidBrdFrameRawData;
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;

	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ((-1) == llid ) 
	{
		return (-1);
	}
	
		printf("\r\n  ponId %d  onuId %d   llid %d\r\n", PonId, OnuId+1, llid);
		{
		collector_id = -1;	
		stat_parameter = llid;
		
		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuBerRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
			printf("\r\n  %% onuId %d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",OnuId+1,PonId);
		}
		memset(buf, 0, 256);	
		printf("  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);

		counter = onuBerRawData.used_byte_count;
		sprintf64Bits( szText, counter );
		sprintf( buf, "  GoodbytesRev                          :  %s\r\n", szText);
		printf("%s",buf);	

		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}
		
		memset(buf, 0, 256);	
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		printf("%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n", szText );		
		printf("%s",buf);
		
	}


	{
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",PonId,stat_parameter);
		}
		printf("  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		printf("  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		printf("  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		

	}


	{
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);		
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
		printf("  BrdFrmRev                             :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		printf("  llidBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		printf("  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		printf("  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		
	}

	
	{
		memset(&llidBrdbytesSendRawData, 0, sizeof(PON_transmitted_bytes_per_llid_raw_data_t));
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdbytesSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = llidBrdbytesSendRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf( buf,"  llidOctetsTx                          :  %s\r\n", szText );
		printf("%s",buf);

	}


	
	{
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}	
		printf("  llidBrdFrmTx                          :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		
	}



	{
		memset(&llidDropFrameSendRawData, 0, sizeof(PON_dropped_frames_per_llid_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_DROPPED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidDropFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

		printf("  llidFrmPolicerDrop                    :  %lu\r\n",llidDropFrameSendRawData.policer_dropped);
		printf("  llidFrmMismatchDrop                   :  %lu\r\n",llidDropFrameSendRawData.mismatch_dropped);

	}

	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		if (iRes != STATS_OK)
		{
		printf( "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		printf("  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		
	}


	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		printf("  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);		
	}

	
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
		printf("  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
		printf("  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
	
	}

	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		stat_parameter = llid;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);

	}	

	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  inRangeLenErrRx                       :  %lu\r\n",inRangeLenErrRawData.received);

		
	}

	
		
	{

		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		
	}




	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		printf("  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
	
	}

	

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		printf("  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);
		
	}
	

		

		return 0;
}

int statDebugOltStatisticsShow( short int PonId,short int OnuId)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t  timestamp ;

	char szText[64] = "";
	unsigned long long counter = 0;
	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData/*,allinRangeLenErrRawData*/;
	/*PON_frame_check_sequence_errors_raw_data_t	SequenceErrorRawData;*/
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
	/*PON_registration_frames_raw_data_t		registertFrameRawData;*/
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
	/*PON_mpcp_status_raw_data_t MPCPstatisticsCounter;*/
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_dropped_frames_per_llid_raw_data_t llidDropFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_bytes_per_llid_raw_data_t llidBrdbytesSendRawData;

	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;
	PON_total_frames_raw_data_t totalFrameRawData;
	PON_in_range_length_errors_raw_data_t	inRangeLengthErrRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_frame_too_long_errors_per_llid_raw_data_t frameTooLongErrPerLlidRawData;
	PON_host_frames_raw_data_t	hostframeRawData;
	PON_host_messages_raw_data_t hostMsgRawData;
	PON_pause_frames_raw_data_t	pauseFrameRawData;
	PON_pause_time_raw_data_t	pauseTimeRawData;
	PON_grant_frames_raw_data_t	grantFrameRawData;
	PON_p2p_frames_raw_data_t p2pFrameRawData;
	PON_report_frames_raw_data_t reportFrameRawData;
	PON_promiscuous_status_raw_data_t promiscuousStatusRawData;
	PON_duplex_status_raw_data_t duplexStatusRawData;
	PON_rtt_raw_data_t			rttRawData;
	/*OnuId 与pas-soft llid 的转换，不用关心*/
	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ((-1) == llid ) 
	{
		return (-1);
	}
	
		printf("\r\n  ponId %d  onuId %d   llid %d\r\n", PonId, OnuId+1, llid);
	{/*onu_ber*/
		collector_id = -1;	
		stat_parameter = llid;
		
		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuBerRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
			printf("\r\n  %% onuId %d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",OnuId+1,PonId);
		}
		memset(buf, 0, 256);	
		printf("  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);
		VOS_MemZero( szText, 64 );
		counter = onuBerRawData.used_byte_count;
		sprintf64Bits( szText, counter );
		sprintf(buf,"  GoodbytesRev                          :  %s\r\n", szText );

		printf("%s",buf);	

	}

	{
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",PonId,stat_parameter);
		}
		printf("  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		printf("  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		printf("  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		

	}

	{/*LLID-broadcast frame*/

	}

	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		collector_id = -1;
		stat_parameter = 1;/*port id : 1- PON, 2 - system*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
		iRes = PAS_get_raw_statistics(  PonId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&totalFrameRawData,
										&timestamp
										);
		if ( iRes != 0 )
			printf("\r\n  %% PonId %d PON_RAW_STAT_TOTAL_FRAMES EPPOR\r\n",PonId);
		printf("  for pon :\r\n");
		/*printf("  totalOctetsRawData.received_error			: %lu \r\n",totalOctetsRawData.received_error);*/
		printf("  totalFrameRawData.received_ok			: %lu \r\n",totalFrameRawData.received_ok );
		/*printf("  totalOctetsRawData.received_total			: %lu \r\n",totalOctetsRawData.received_total );*/
		/*printf("  totalOctetsRawData.transmitted_ok			: %lu \r\n",totalOctetsRawData.transmitted_ok );*/
		
	}

	{/*PON_RAW_STAT_TOTAL_FRAMES for system*/
		collector_id = -1;
		stat_parameter = 2;/*port id : 1- PON, 2 - system*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
		iRes = PAS_get_raw_statistics(  PonId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&totalFrameRawData,
										&timestamp
										);
		if ( iRes != 0 )
			printf("\r\n  %% PonId %d PON_RAW_STAT_TOTAL_FRAMES EPPOR\r\n",PonId);
		printf("  for system :\r\n");
		printf("  totalOctetsRawData.received_error			: %lu \r\n",totalFrameRawData.received_error);
		printf("  totalOctetsRawData.received_ok			: %lu \r\n",totalFrameRawData.received_ok );
		/*printf("  totalOctetsRawData.total_bad			: %lu \r\n",totalFrameRawData.total_bad );*/
		printf("  totalOctetsRawData.transmitted_ok			: %lu \r\n",totalFrameRawData.transmitted_ok );
		
	}

	{/*total Octect for pon*/
		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		printf("%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );		
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n", szText );		
		printf("%s\r\n",buf);
		
	}
	


	{/*total Octect for system*/
		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 2;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}
		memset(buf, 0, 256);	
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );				
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		printf("%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );			
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n", szText);		
		printf("%s\r\n",buf);
		
	}

	{/*Tranmitts octects per LLID*/
		memset(&llidBrdbytesSendRawData, 0, sizeof(PON_transmitted_bytes_per_llid_raw_data_t));
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdbytesSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = llidBrdbytesSendRawData.transmitted_ok;
		sprintf64Bits( szText, counter );		
		sprintf( buf,"  llidOctetsTx                          :  %s\r\n", szText);
		printf("%s",buf);

	}

	
	{
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}	
		printf("  llidBrdFrameSendRawData.pon_ok        :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		printf("  llidBrdFrameSendRawData.system_ok     :  %lu\r\n",llidBrdFrameSendRawData.system_ok);
		
	}


	{
		memset(&llidDropFrameSendRawData, 0, sizeof(PON_dropped_frames_per_llid_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_DROPPED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidDropFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

		printf("  llidFrmPolicerDrop                    :  %lu\r\n",llidDropFrameSendRawData.policer_dropped);
		printf("  llidFrmMismatchDrop                   :  %lu\r\n",llidDropFrameSendRawData.mismatch_dropped);

	}

	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		if (iRes != STATS_OK)
		{
		printf( "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		printf("  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		
	}

	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		printf("  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);		
	}


#if 0
	{
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);		
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
		printf("  BrdFrmRev                             :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		printf("  llidBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		printf("  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		printf("  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		
	}

#endif

	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
		printf("  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
		printf("  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
	
	}

	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		stat_parameter = llid;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);

	}	

	{/*alignment error -- ONU*/
		
	}

	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  inRangeLenErrRx                       :  %lu\r\n",inRangeLenErrRawData.received);

		
	}

	{/*In range length error*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&inRangeLengthErrRawData, 0, sizeof(PON_in_range_length_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS,
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&inRangeLengthErrRawData, 
										&timestamp
										);
		if (iRes != 0)
			printf("  %% ponId %d PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS ERR\r\n",PonId);

		printf("  inRangeLengthErrRawData.system_received = %lu\r\n",inRangeLengthErrRawData.system_received);
		
	}

	{/*frame too long err*/
		collector_id = -1;
		stat_parameter = 0;
		memset( &frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&frameTooLongErrRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_FRAME_TOO_LONG_ERRORS ERRPR\r\n",PonId);

		printf("  frameTooLongErrRawData.pon_received = %lu\r\n",frameTooLongErrRawData.pon_received);
		printf("  frameTooLongErrRawData.system_received = %lu\r\n",frameTooLongErrRawData.system_received);
	}

	{/*frame too long err per llid*/
		collector_id = -1;
		stat_parameter = llid;
		memset( &frameTooLongErrPerLlidRawData, 0, sizeof(PON_frame_too_long_errors_per_llid_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&frameTooLongErrPerLlidRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID ERROR\r\n",PonId);

		
		printf("  frameTooLongErrPerLlidRawData.received = %lu\r\n",frameTooLongErrPerLlidRawData.received);
		
	}
		
	{
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		
	}

	{/*host frame for pon*/
		collector_id = -1;
		stat_parameter = 0;
		
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&hostframeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_FRAMES ERROR\r\n",PonId );
		printf("  for Pon total:\r\n");
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_transmitted_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_transmitted_ok);
	}

	{/*host frame for pon&per llid*/
		collector_id = -1;
		stat_parameter = llid;
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&hostframeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_FRAMES ERROR\r\n",PonId );		
		printf("  for Pon&per llid:\r\n");
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_transmitted_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_transmitted_ok);


	}

	{/*host frame for brd_llid*/
		collector_id = -1;
		stat_parameter = 127/*brd_llid*/;
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&hostframeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_FRAMES ERROR\r\n",PonId );		
		printf("  for Pon&per llid:\r\n");
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.pon_transmitted_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_transmitted_ok);
		
	}

	{/*host message*/
		collector_id = -1;
		stat_parameter = 1;/*Host physical interface type (0 – Parallel, 1 – Ethernet, 2 - UART)*/
		memset(&hostMsgRawData,0 , sizeof(PON_host_messages_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_HOST_MESSAGES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&hostMsgRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_MESSAGES ERRPR\r\n",PonId);

		printf("  hostMsgRawData.received_from_firmware_ok = %lu\r\n",hostMsgRawData.received_from_firmware_ok);
		printf("  hostMsgRawData.sent_to_firmware_ok = %lu\r\n",hostMsgRawData.sent_to_firmware_ok);
	}

	{/*pause frame*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&pauseFrameRawData,
										&timestamp);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PAUSE_FRAMES ERROR\r\n",PonId);
		
		printf("  pauseFrameRawData.pon_received_ok		= %lu\r\n",pauseFrameRawData.pon_received_ok);
		printf("  pauseFrameRawData.pon_transmitted_ok	= %lu\r\n",pauseFrameRawData.pon_transmitted_ok);
		printf("  pauseFrameRawData.system_received_ok	= %lu\r\n",pauseFrameRawData.system_received_ok);
		printf("  pauseFrameRawData.system_transmitted_ok	= %lu\r\n",pauseFrameRawData.system_transmitted_ok);
	}

	{/*pause tiem*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&pauseTimeRawData, 0, sizeof(PON_pause_time_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_TIME, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&pauseTimeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PAUSE_TIME ERROR!\r\n",PonId);
		
		printf("  pauseTimeRawData.system_port		= %lu\r\n",pauseTimeRawData.system_port);
	}

	{/*grant frame*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_GRANT_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&grantFrameRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_GRANT_FRAMES ERROR\r\n",PonId);
		/*grantFrameRawData.received_ok*/
		printf("  grantFrameRawData.transmitted_dba_ok	= %lu\r\n",grantFrameRawData.transmitted_dba_ok);
		printf("  grantFrameRawData.transmitted_ctrl_ok	= %lu\r\n",grantFrameRawData.transmitted_ctrl_ok);
		
	}

	{/*report frame*/
		collector_id = -1;
		stat_parameter = llid;
		
		memset(&reportFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_REPORT_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&reportFrameRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_REPORT_FRAMES ERROR\r\n",PonId);
		/*reportFrameRawData.transmitted_ok*/
		printf("  reportFrameRawData.received_ok	= %lu\r\n",reportFrameRawData.received_ok);
	}

	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		printf("  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
	
	}
	{/*multicast brd_llid*/
		collector_id = -1;
		stat_parameter = 127;
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  for multicast brdLlid:\r\n");
		printf("  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		printf("  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
			
	}

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		printf("  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);
		
	}

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = 127;/*brdLlid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		printf("  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		printf("  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);
		
	}

	{/*p2p frame*/
		collector_id = -1;
		stat_parameter = llid;
		
		memset(&p2pFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_P2P_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&p2pFrameRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_P2P_FRAMES ERROR \r\n",PonId);

		printf("  p2pFrameRawData.dropped_by_access_control	= %lu\r\n",p2pFrameRawData.dropped_by_access_control);
		printf("  p2pFrameRawData.dropped_by_policer = %lu\r\n",p2pFrameRawData.dropped_by_policer);
		printf("  p2pFrameRawData.dropped_due_to_tx_buffer_full = %lu\r\n",p2pFrameRawData.dropped_due_to_tx_buffer_full);
		printf("  p2pFrameRawData.received_ok		= %lu\r\n",p2pFrameRawData.received_ok);
		printf("  p2pFrameRawData.transmitted_ok	= %lu\r\n",p2pFrameRawData.transmitted_ok);
	}

	{/*p2p frame*/
		collector_id = -1;
		stat_parameter = 127;
		memset(&p2pFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_P2P_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&p2pFrameRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_P2P_FRAMES ERROR \r\n",PonId);
		printf("  p2p for brdLlid:\r\n");
		printf("  p2pFrameRawData.dropped_by_access_control	= %lu\r\n",p2pFrameRawData.dropped_by_access_control);
		printf("  p2pFrameRawData.dropped_by_policer = %lu\r\n",p2pFrameRawData.dropped_by_policer);
		printf("  p2pFrameRawData.dropped_due_to_tx_buffer_full = %lu\r\n",p2pFrameRawData.dropped_due_to_tx_buffer_full);
		printf("  p2pFrameRawData.received_ok		= %lu\r\n",p2pFrameRawData.received_ok);
		printf("  p2pFrameRawData.transmitted_ok	= %lu\r\n",p2pFrameRawData.transmitted_ok);
	}

	{/*Promiscuous Status*/
		collector_id = -1;
		stat_parameter = 0;
		
		memset(&promiscuousStatusRawData, 0, sizeof(PON_promiscuous_status_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_PROMISCUOUS_STATUS, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&promiscuousStatusRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PROMISCUOUS_STATUS ERROR\r\n",PonId);
		
		printf("  promiscuousStatusRawData.status = %d\r\n",promiscuousStatusRawData.status);
	}
	
	{/*duplex status*/
		collector_id = -1;	
		stat_parameter = 0;
		memset(&duplexStatusRawData, 0, sizeof(PON_duplex_status_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_DUPLEX_STATUS, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&duplexStatusRawData,
										&timestamp);
		if( iRes != 0 )
			printf("  %% PonId %d PON_RAW_STAT_DUPLEX_STATUS ERROR\r\n",PonId);
		
		if(0 == duplexStatusRawData.mac_capabilities)
			printf("  MAC duplex : HALF\r\n");
		else
			printf("  MAC duplex : FULL\r\n");
		if (0 == duplexStatusRawData.status)
			printf("  PON port duplex : HALF\r\n");
		else
			printf("  PON port duplex : FULL\r\n");
	}

	{/*RTT*/
		collector_id = -1;
		stat_parameter = llid;
		memset(&rttRawData, 0, sizeof(PON_rtt_raw_data_t));
		iRes = PAS_get_raw_statistics( PonId, 
										collector_id, 
										PON_RAW_STAT_RTT, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE, */
										&rttRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_RTT ERROR\r\n",PonId);
		
		printf("  rttRawData.rtt		= %lu\r\n",rttRawData.rtt);
	}
	

	
	return 0;
}





int statDebugOnuStatisticsShow(short int PonId,short int OnuId)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t timestamp  ;
	char szText[64] = "";
	unsigned long long counter = 0;
	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_llid_Broadcast_frames_raw_data_t llidBrdFrameRawData;
	PON_olt_ber_raw_data_t oltBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;

	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ((-1) == llid ) 
	{
		return (-1);
	}

	{
		collector_id = llid;	
		stat_parameter = 0;
		
		memset(&oltBerRawData, 0, sizeof(PON_olt_ber_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_OLT_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &oltBerRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
			printf("\r\n  %% onu %d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n", OnuId+1,PonId);
		}
	

		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = llid;	
		stat_parameter = 1;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type onu %d\r\n",PonId,collector_id);
		}	
	}



	{/*llid broadcast frame*/
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);		
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
	}
	
	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}


	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}

	
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}

	{/*check sequence error, one err of FCS err*/
		collector_id = llid;
		stat_parameter = 0;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,collector_id);
		}

	}	

	{/*PON_RAW_STAT_ALIGNMENT_ERRORS*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = llid;/*olt*/
		stat_parameter = 0;/*llid*/
		/*PON_RAW_STAT_ALIGNMENT_ERRORS;*/
	}

	
		
	{

		collector_id = llid;/*olt*/
		stat_parameter = 0;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,collector_id);
		}
	}




	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = llid;		
		stat_parameter = 0;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,collector_id);
		}
	
	}

	

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = llid;	
		stat_parameter = 0;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,collector_id);
		}
		
	}
	
		printf("\r\n  onu %d (ponId %d) \r\n",OnuId+1,PonId);
		memset(buf, 0, 256);	
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsRx                         :  %s\r\n", szText );	
		printf("%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_error;
		sprintf64Bits( szText, counter );		
		sprintf(buf, "  BadOctbetsRx                          :  %s\r\n", szText);	
		printf("%s",buf);
		memset(buf, 0, 256);	
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_total;
		sprintf64Bits( szText, counter );			
		sprintf(buf, "  OctbetsTotalRx                        :  %s\r\n", szText);	
		printf("%s",buf);
							
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );			
		sprintf(buf, "  GoodOctbetsTotalTx                    :  %s\r\n", szText);		
		printf("%s",buf);		
		printf("  ValidPreBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		printf("  ValidllidBrdFrmRev                    :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		printf("  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		printf("  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		printf("  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
		printf("  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
		printf("  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);
		printf("  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		printf("  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		printf("  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
		printf("  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		printf("  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		printf("  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		printf("  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		printf("  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);
		return 0;
}








int stat_pon_mib( short int ponId, short int onuId)
{
	int delay_time = (60*2); /*1s = 60 tick*/
	int temp = 4;
	int i = 0;
	for(i = 0; i < temp; i++)
	{
		statDebugOltStatisticsShow( ponId, onuId);
		taskDelay(delay_time);
	}
	return 0;
}

int statOltRealTimeStatisticsVty(short int slotId, short int port, short int PonId,short int OnuId,struct vty *vty)
{
	short int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	short int llid = 0;
	unsigned char buf[256] = {0};
	PON_timestamp_t timestamp ;

	char szText[64]="";
	unsigned long long counter = 0;
	
	PON_single_collision_frames_raw_data_t	singleCollisionFrameRawData;
	PON_multiple_collision_frames_raw_data_t	multCollisionFrameRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData/*,allcheckSeqErrRawData*/;
	PON_in_range_length_errors_per_llid_raw_data_t inRangeLenErrRawData/*,allinRangeLenErrRawData*/;
	/*PON_frame_check_sequence_errors_raw_data_t	SequenceErrorRawData;*/
	PON_hec_frames_errors_raw_data_t		hecFrameErrRawData;
	/*PON_registration_frames_raw_data_t		registertFrameRawData;*/
	PON_broadcast_frames_raw_data_t		broadcastFramRawData/*, allbrdcstFramRawData*/;
	PON_multicast_frames_raw_data_t		mulFrameRawData/*, allmulFrameRawData*/;
	/*PON_mpcp_status_raw_data_t MPCPstatisticsCounter;*/
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_dropped_frames_per_llid_raw_data_t llidDropFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_bytes_per_llid_raw_data_t llidBrdbytesSendRawData;
	PON_llid_Broadcast_frames_raw_data_t llidBrdFrameRawData;
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;

	llid = GetLlidByOnuIdx( PonId, OnuId);
	if ((-1) == llid ) 
	{
		return (-1);
	}

	{
		collector_id = -1;	
		stat_parameter = llid;
		
		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuBerRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
			vty_out( vty,"\r\n  %% %d/%d/%d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",slotId,port,OnuId+1,PonId);
		}
	

		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			vty_out( vty,"\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",PonId,stat_parameter);
		}	
	}


	{
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",PonId,stat_parameter);
		}

	}


	{
		memset(&llidBrdFrameRawData, 0, sizeof(PON_llid_Broadcast_frames_raw_data_t));
		collector_id = llid;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_LLID_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameRawData,
									   &timestamp 
										);		
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_LLID_BROADCAST_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}
	}

	
	{
		memset(&llidBrdbytesSendRawData, 0, sizeof(PON_transmitted_bytes_per_llid_raw_data_t));
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdbytesSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}


	
	{
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}	
	}



	{
		memset(&llidDropFrameSendRawData, 0, sizeof(PON_dropped_frames_per_llid_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_DROPPED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidDropFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty,"\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. PonId %d Llid %d\r\n",PonId,collector_id);
		}

	}

	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}


	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		vty_out( vty, "\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",PonId,collector_id);	
		}
	}

	
	{/*single collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&singleCollisionFrameRawData, 0, sizeof(PON_single_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_SINGLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &singleCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_SINGLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}
	
	{/*multCollisionFrameRawData collision frame*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt*/
		memset(&multCollisionFrameRawData, 0, sizeof(PON_multiple_collision_frames_raw_data_t));
		collector_id = -1;	
		stat_parameter = 0;
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &multCollisionFrameRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES ERROR. olt PonId %d\r\n",PonId);
		}
	
	}

	{/*check sequence error, one err of FCS err*/
		/*olt = -1*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		collector_id = -1;
		stat_parameter = llid;/*llid*/
		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

	}	

	{/*less than 60byte Err and ere otherwise well formed*/
		/*获取接收到的Collisionsd 的数据包*/
		/*olt = -1*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&inRangeLenErrRawData, 0, sizeof(PON_in_range_length_errors_per_llid_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &inRangeLenErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}

		
	}

	
		
	{

		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	}




	{/*获取接收到的组播数据*/
		/*olt = -1*/
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
	
	}

	

	{/*获取接收到的广播数据*/
		/*olt = -1*/

		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( PonId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		vty_out( vty, "  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",PonId,stat_parameter);
		}
		
	}
	
		vty_out( vty,"\r\n  %d/%d/%d (ponId %d) \r\n",slotId,port,OnuId+1,PonId);
		memset(buf, 0, 256);	
		vty_out( vty,"  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);

		VOS_MemZero( szText, 64 );
		counter = onuBerRawData.used_byte_count;
		sprintf64Bits( szText, counter );
		sprintf( buf, "  GoodbytesRev                          :  %s\r\n", szText );
		vty_out( vty,"%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = llidBrdbytesSendRawData.transmitted_ok;
		sprintf64Bits( szText, counter );
		sprintf( buf,"  llidOctetsTx                          :  %s\r\n", szText );
		vty_out( vty,"%s",buf);
		
		vty_out( vty,"  llidBrdFrmTx                          :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		vty_out( vty,"  llidFrmPolicerDrop                    :  %lu\r\n",llidDropFrameSendRawData.policer_dropped);
		vty_out( vty,"  llidFrmMismatchDrop                   :  %lu\r\n",llidDropFrameSendRawData.mismatch_dropped);
		vty_out( vty,"  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		vty_out( vty,"  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		vty_out( vty,"  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		
		vty_out( vty,"  BrdFrmRev                             :  %lu\r\n",llidBrdFrameRawData.broadcast_frames_received);
		vty_out( vty,"  llidBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_frames_received);
		vty_out( vty,"  DropBrdFrmRev                         :  %lu\r\n",llidBrdFrameRawData.llid_broadcast_error_frames_received);
		vty_out( vty,"  DropllidBrdFrmRev                     :  %lu\r\n",llidBrdFrameRawData.invalid_llid_error_frames_received);
		vty_out( vty,"  singlecollisionFrmRx                  :  %lu\r\n",singleCollisionFrameRawData.single_collision_frames);
		vty_out( vty,"  multiplecollisionFrmRx                :  %lu\r\n",multCollisionFrameRawData.multiple_collision_frames);
		vty_out( vty,"  checkSeqErrRx                         :  %lu\r\n",checkSeqErrRawData.received);
		vty_out( vty,"  inRangeLenErrRx                       :  %lu\r\n",inRangeLenErrRawData.received);
		vty_out( vty,"  HecErrFrm                             :  %lu\r\n",hecFrameErrRawData.received);
		vty_out( vty,"  GoodmulEthFrmRx                       :  %lu\r\n",mulFrameRawData.received_ok);
		vty_out( vty,"  GoodmulEthFrmTx                       :  %lu\r\n",mulFrameRawData.transmitted_ok);
		vty_out( vty,"  GoodbrdEthFrmRx                       :  %lu\r\n",broadcastFramRawData.received_ok);
		vty_out( vty,"  GoodbrdEthFrmTx                       :  %lu\r\n",broadcastFramRawData.transmitted_ok);

		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameRevRawData.total_control_dropped);
		vty_out( vty,"  ToltalDropoverflowTTosystem           :  %lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);
		vty_out( vty,"  TotalDropQueueoverflowTx              :  %lu\r\n",TotalDropFrameTxRawData.total_pon_dropped);
		vty_out( vty,"  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);

		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.received_ok;
		sprintf64Bits( szText, counter );
		sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %s\r\n", szText );	
		vty_out( vty,"%s",buf);
		
		memset(buf, 0, 256);
		VOS_MemZero( szText, 64 );
		counter = totalOctetsRawData.transmitted_ok;
		sprintf64Bits( szText, counter );		
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %s\r\n",szText);		
		vty_out( vty,"%s",buf);
		return 0;
}


#define PON_OLT_AND_ONU_COMBINED_ID (-2)
STATUS CliRealTimeStatsPon_test( short int ponId )
{
	int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	PON_timestamp_t timestamp;
	unsigned char 	buf[256] = {0};
	short int 	llid = 0;
	short int 	onuId = 0;
	unsigned long long counter = 0;
	char szText[64]="";
	
	PON_total_frames_raw_data_t totalFrameRawData;
	PON_total_octets_raw_data_t totalOctetsRawData;
	PON_transmitted_frames_per_llid_raw_data_t llidBrdFrameSendRawData;
	PON_transmitted_frames_per_llid_raw_data_t totalLlidBrdFrameSendRawData;
	PON_onu_fer_raw_data_t onuFerRawData;
	PON_onu_fer_raw_data_t onuTotalFerRawData;
	PON_onu_ber_raw_data_t onuBerRawData;
	PON_onu_ber_raw_data_t onuTotalBerRawData;
	PON_total_tx_dropped_frames_raw_data_t TotalDropFrameTxRawData;
	PON_total_dropped_rx_frames_raw_data_t TotalDropFrameRevRawData;
	PON_frame_check_sequence_errors_raw_data_t checkSeqErrRawData;
	PON_frame_check_sequence_errors_raw_data_t totalCheckSeqErrRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_hec_frames_errors_raw_data_t 		hecFrameErrRawData;
	PON_host_frames_raw_data_t 				hostframeRawData;
	PON_pause_frames_raw_data_t 			pauseFrameRawData;
	PON_grant_frames_raw_data_t 			grantFrameRawData;
	PON_report_frames_raw_data_t			reportFrameRawData;
	PON_multicast_frames_raw_data_t			mulFrameRawData;
	PON_multicast_frames_raw_data_t			totalMulFrameRawData;
	PON_broadcast_frames_raw_data_t			broadcastFramRawData;
	PON_broadcast_frames_raw_data_t			totalBroadcastFramRawData;
	PON_p2p_frames_raw_data_t				p2pFrameRawData;
	PON_p2p_frames_raw_data_t				p2pTotalFrameRawData;
	PON_grant_frames_raw_data_t 			grantTotalFrameRawData;
	PON_report_frames_raw_data_t 			reportTotalFrameRawData;

	printf("\r\n");
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		collector_id = -1;
		stat_parameter = 1;/*port id : 1- PON, 2 - system*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
		iRes = PAS_get_raw_statistics(  ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&totalFrameRawData,
										&timestamp
										);
		if ( iRes != 0 )
			printf("\r\n  %% PonId %d P*/ON_RAW_STAT_TOTAL_FRAMES EPPOR\r\n",ponId);
/*		printf("  for pon :\r\n");*/
		/*printf("  totalOctetsRawData.received_error			: %lu \r\n",totalOctetsRawData.received_error);*/
		/*printf("  totalFrameRawData.received_ok			: %lu \r\n",totalFrameRawData.received_ok );
		*//*printf("  totalOctetsRawData.received_total			: %lu \r\n",totalOctetsRawData.received_total );*/
		/*printf("  totalOctetsRawData.transmitted_ok			: %lu \r\n",totalOctetsRawData.transmitted_ok );*/
		
	}

	{/*total Octect for pon*/
		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 1;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",ponId,stat_parameter);
		}
		memset(buf, 0, 256);	
		/*sprintf(buf, "  GoodOctbetsTotalRx(from all onus)     :  %lu\r\n",totalOctetsRawData.received_ok);	
		printf("%s",buf);
		memset(buf, 0, 256);
		sprintf(buf, "  GoodOctbetsTotalTx(to all onus)       :  %lu\r\n",(totalOctetsRawData.transmitted_ok));		
		printf("%s\r\n",buf);
		*/
	}

	memset(&totalLlidBrdFrameSendRawData, 0, sizeof(PON_transmitted_frames_per_llid_raw_data_t));
	for(onuId = 0;onuId < 64; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		collector_id = -1;	
		stat_parameter = llid;
		memset(&llidBrdFrameSendRawData, 0, sizeof(PON_transmitted_frames_per_llid_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &llidBrdFrameSendRawData,
									   &timestamp 
										);	
		if (iRes != 0)
		{
		printf("\r\n  PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID ERROR. ponId %d Llid %d\r\n",ponId,collector_id);
		}
		totalLlidBrdFrameSendRawData.pon_ok += llidBrdFrameSendRawData.pon_ok;
/*		printf("  llidBrdFrameSendRawData.pon_ok        :  %lu\r\n",llidBrdFrameSendRawData.pon_ok);
		printf("  llidBrdFrameSendRawData.system_ok     :  %lu\r\n",llidBrdFrameSendRawData.system_ok);
		*/
	}

	memset(&onuTotalFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		collector_id = -1;	
		stat_parameter = llid;
		memset(&onuFerRawData, 0, sizeof(PON_onu_fer_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_FER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuFerRawData,
									   &timestamp 
										);	
		if (iRes != 0)
			printf("\r\n  PON_RAW_STAT_ONU_FER ERROR. PonId %d Llid %d\r\n",ponId,stat_parameter);
		onuTotalFerRawData.received_error += onuFerRawData.received_error;
		/*printf("  FirmwareFrmRev                        :  %lu\r\n",onuFerRawData.firmware_received_ok);
		printf("  ErrFrmRev                             :  %lu\r\n",onuFerRawData.received_error);
		printf("  GoodFrmRev                            :  %lu\r\n",onuFerRawData.received_ok);		
*/
	}

	memset(&onuTotalBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{/*onu_ber*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;	
		collector_id = -1;	
		stat_parameter = llid;

		memset(&onuBerRawData, 0, sizeof(PON_onu_ber_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_ONU_BER, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &onuBerRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
			printf("\r\n  %% onuId %d ponId %d PON_RAW_STAT_ONU_BER ERR! \r\n",onuId+1,ponId);
		}

		onuTotalBerRawData.error_bytes += onuBerRawData.error_bytes;
		/*memset(buf, 0, 256);	
		printf("  BadbytesRev                           :  %lu\r\n",onuBerRawData.error_bytes);
		sprintf( buf, "  GoodbytesRev                          :  %lu\r\n",onuBerRawData.used_byte_count);
		printf("%s",buf);	
*/
	}
	
	{
		memset(&TotalDropFrameRevRawData, 0, sizeof(PON_total_dropped_rx_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameRevRawData,
									   &timestamp 
										);
		if (iRes != STATS_OK)
			printf( "\r\n  PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES ERROR. PonId %d Llid %d\r\n",ponId,collector_id);	
	
	}
	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",ponId,collector_id);	
		}
		/*printf("  TotalDropCtrlQueueoverflowTx          :  %lu\r\n",TotalDropFrameTxRawData.total_system_dropped);		
	*/
	}

	memset(&totalCheckSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));	
	for(onuId = 0; onuId < 64; onuId++)
	{/*check sequence error, one err of FCS err*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;			
		collector_id = -1;
		stat_parameter = llid;/*llid*/

		memset(&checkSeqErrRawData, 0, sizeof(PON_frame_check_sequence_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &checkSeqErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
			printf("  PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
		totalCheckSeqErrRawData.received += checkSeqErrRawData.received;

	}	
	
	{/*frame too long err*/
		collector_id = -1;
		stat_parameter = 0;
		memset( &frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&frameTooLongErrRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_FRAME_TOO_LONG_ERRORS ERRPR\r\n",ponId);

		/*printf("  frameTooLongErrRawData.system_received = %lu\r\n",frameTooLongErrRawData.system_received);
		*/
	}
	
	{/*HEC error*/
		collector_id = -1;/*olt*/
		stat_parameter = llid;/*llid*/
		memset(&hecFrameErrRawData, 0, sizeof(PON_hec_frames_errors_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_HEC_FRAMES_ERRORS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &hecFrameErrRawData,
									   &timestamp 
										);
		if (iRes != 0)
			printf("  PON_RAW_STAT_HEC_FRAMES_ERRORS ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
		
	}

	{/*host frame for pon&per llid*/
		collector_id = -1;
		stat_parameter = llid;
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&hostframeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_FRAMES ERROR\r\n",ponId );		
		/*printf("  for Pon&per llid:\r\n");*/
/*
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_dropped);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_received_ok);
		printf("  hostframeRawData.pon_dropped		= %lu\r\n",hostframeRawData.system_transmitted_ok);
*/
	}

	{/*pause frame*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&pauseFrameRawData,
										&timestamp);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PAUSE_FRAMES ERROR\r\n",ponId);
		
		/*printf("  pauseFrameRawData.system_received_ok	= %lu\r\n",pauseFrameRawData.system_received_ok);
		printf("  pauseFrameRawData.system_transmitted_ok	= %lu\r\n",pauseFrameRawData.system_transmitted_ok);
		*/
	}

	memset(&grantTotalFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{/*grant frame*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;		 
		collector_id = PON_OLT_AND_ONU_COMBINED_ID;
		stat_parameter = llid;
		memset(&grantFrameRawData, 0, sizeof(PON_grant_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_GRANT_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&grantFrameRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d onuId %d PON_RAW_STAT_GRANT_FRAMES ERROR\r\n",ponId, onuId);
		/*grantFrameRawData.received_ok*/
		grantTotalFrameRawData.received_ok += grantFrameRawData.received_ok;
		grantTotalFrameRawData.transmitted_ctrl_ok += grantFrameRawData.transmitted_ctrl_ok;
		grantTotalFrameRawData.transmitted_dba_ok += grantFrameRawData.transmitted_dba_ok;
	}

	memset(&reportTotalFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{/*report frame*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;			
		collector_id = PON_OLT_AND_ONU_COMBINED_ID;
		stat_parameter = llid;
		
		memset(&reportFrameRawData, 0, sizeof(PON_report_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_REPORT_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&reportFrameRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_REPORT_FRAMES ERROR\r\n",ponId);

		reportTotalFrameRawData.received_ok += reportFrameRawData.received_ok;
		reportTotalFrameRawData.transmitted_ok += reportFrameRawData.transmitted_ok;
		/*reportFrameRawData.transmitted_ok*/
	}

	memset(&totalMulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{/*获取接收到的组播数据*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		collector_id = -1;		
		stat_parameter = llid;/*llid*/
		memset(&mulFrameRawData, 0, sizeof(PON_multicast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_MULTICAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &mulFrameRawData,
									   &timestamp 
										);
		if (iRes != 0)
		{
		printf("  PON_RAW_STAT_MULTICAST_FRAMES ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
		}
		totalMulFrameRawData.received_ok += mulFrameRawData.received_ok;
		totalMulFrameRawData.transmitted_ok += mulFrameRawData.transmitted_ok;
	}

	memset(&totalBroadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
	for(onuId = 0; onuId < 64; onuId++)
	{/*获取接收到的广播数据*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;
		collector_id = -1;	
		stat_parameter = llid;/*llid*/
		memset(&broadcastFramRawData, 0, sizeof(PON_broadcast_frames_raw_data_t));
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_BROADCAST_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &broadcastFramRawData,
									   &timestamp 
										);	
		if (iRes != 0)
			printf("  PON_RAW_STAT_BROADCAST_FRAMES ERROR. PonId %d llid %d\r\n",ponId,stat_parameter);
		totalBroadcastFramRawData.received_ok += broadcastFramRawData.received_ok;
		totalBroadcastFramRawData.transmitted_ok += broadcastFramRawData.transmitted_ok;
		
	}
 
	memset(&p2pTotalFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
	for( onuId = 0; onuId < 64; onuId++)
	{/*p2p frame*/
		llid = GetLlidByOnuIdx( ponId, onuId);
		if ((-1) == llid ) 
			continue;	
		collector_id = -1;
		stat_parameter = llid;
		
		memset(&p2pFrameRawData, 0, sizeof(PON_p2p_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_P2P_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&p2pFrameRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_P2P_FRAMES ERROR \r\n",ponId);
		p2pTotalFrameRawData.received_ok += p2pFrameRawData.received_ok;
		p2pTotalFrameRawData.transmitted_ok += p2pFrameRawData.transmitted_ok;
		p2pTotalFrameRawData.dropped_by_policer += p2pFrameRawData.dropped_by_policer;
		p2pTotalFrameRawData.dropped_by_access_control += p2pFrameRawData.dropped_by_access_control;
	}

	/*for Pon Rev*/
	printf("  RxFrameOk			     :  %-12lu",totalFrameRawData.received_ok );
	
	memset(buf, 0, 256);	
	VOS_MemZero( szText, 64 );
	counter = totalOctetsRawData.received_ok;
	sprintf64Bits( szText, counter );
	sprintf(buf, "  RxOctetsOk             :  %s\r\n", szText );	
	printf("%-12s",buf);

	printf("  RxFrameErr		 	 :  %-12lu",onuTotalFerRawData.received_error);
	printf("  RxOctetsErr            :  %-12lu\r\n",onuTotalBerRawData.error_bytes);

	printf("  RxFrameCtrDrop         :  %-12lu",TotalDropFrameRevRawData.total_control_dropped);
	printf("  RxFrameScrAlertDrop    :  %-12lu\r\n",TotalDropFrameRevRawData.source_alert_dropped);

	printf("  RxFramecheckSeqErr     :  %-12lu",totalCheckSeqErrRawData.received);
	printf("  RxFrameTooLongErr      :  %-12lu\r\n",frameTooLongErrRawData.pon_received);

	printf("  RxFrameHecErr          :  %-12lu",hecFrameErrRawData.received);
	printf("  RxHostFrameDrop        :  %-12lu\r\n",hostframeRawData.pon_dropped);

	printf("  RxHostFrameOk          :  %-12lu",hostframeRawData.pon_received_ok);	
	printf("  RxPauseFrameOk	     :  %-12lu\r\n",pauseFrameRawData.pon_received_ok);

	printf("  RxReportFrameOk        :  %-12lu",reportTotalFrameRawData.received_ok);
	printf("  RxMulticastFrameOk     :  %-12lu\r\n",totalMulFrameRawData.received_ok);

	printf("  RxbroadcastFrameOk     :  %-12lu",totalBroadcastFramRawData.received_ok);
	printf("  Rxp2pFrameOk           :  %-12lu\r\n",p2pTotalFrameRawData.received_ok);

	printf("  Rxp2pFramepolicerDrop  :  %-12lu",p2pTotalFrameRawData.dropped_by_policer);
	printf("  Rxp2pFrameAccessCtrDrop:  %-12lu\r\n",p2pTotalFrameRawData.dropped_by_access_control);

	/*for Pon Transmit*/
	printf("\r\n");
	printf("  TxFrameOk              :  %-12lu",totalLlidBrdFrameSendRawData.pon_ok);
	memset(buf, 0, 256);
	VOS_MemZero( szText, 64 );
	counter = totalOctetsRawData.transmitted_ok;
	sprintf64Bits( szText, counter );
	sprintf(buf, "  TxOctetsOk            :  %s\r\n", szText );		
	printf("%-12s",buf);

	printf("  TxFrameDrop            :  %-12lu",TotalDropFrameTxRawData.total_pon_dropped);
	printf("  TxHostframeOk          :  %-12lu\r\n",hostframeRawData.pon_transmitted_ok);

	printf("  TxPauseFrameOk         :  %-12lu",pauseFrameRawData.pon_transmitted_ok);
	printf("  TxGrantFrameOk         :  %-12lu\r\n",grantTotalFrameRawData.transmitted_dba_ok);

	printf("  TxGrantCtrFrameOk      :  %-12lu",grantTotalFrameRawData.transmitted_ctrl_ok);
	printf("  TxMulticastFrameOk     :  %-12lu\r\n",totalMulFrameRawData.transmitted_ok);

	printf("  TxbroadcastFrameOk     :  %-12lu",totalBroadcastFramRawData.transmitted_ok);
	printf("  Txp2pFrameOk           :  %-12lu\r\n",p2pTotalFrameRawData.transmitted_ok);

	printf("  Txp2pFrameBuffFullDrop :  %-12lu\r\n",p2pTotalFrameRawData.dropped_due_to_tx_buffer_full);
	printf("\r\n");
	return 0;

}


STATUS CliReslTimeStatsCNI_test( short int ponId )
{
	int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	PON_timestamp_t timestamp ;
	unsigned char 	buf[256] = {0};
	short int 	llid = 0;

	char szText[64] = "";
	unsigned long long counter = 0;

	PON_in_range_length_errors_raw_data_t	inRangeLengthErrRawData;
	PON_frame_too_long_errors_raw_data_t	frameTooLongErrRawData;
	PON_host_frames_raw_data_t				hostframeRawData;
	PON_total_frames_raw_data_t				totalFrameRawData;
	PON_total_tx_dropped_frames_raw_data_t	TotalDropFrameTxRawData;
	PON_pause_frames_raw_data_t		pauseFrameRawData;
	PON_pause_time_raw_data_t		pauseTimeRawData;
	PON_total_octets_raw_data_t		totalOctetsRawData;

	
	{/*PON_RAW_STAT_TOTAL_FRAMES for pon*/
		collector_id = -1;
		stat_parameter = 2;/*port id : 1- PON, 2 - system*/
		memset( &totalFrameRawData, 0, sizeof( PON_total_frames_raw_data_t ) );
		iRes = PAS_get_raw_statistics(  ponId, 
										collector_id, 
										PON_RAW_STAT_TOTAL_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&totalFrameRawData,
										&timestamp
										);
		if ( iRes != 0 )
			printf("\r\n  %% PonId %d P*/ON_RAW_STAT_TOTAL_FRAMES EPPOR\r\n",ponId);


	}
	
	{/*total Octect for pon*/
		/*olt = -1*/
		memset(&totalOctetsRawData, 0, sizeof(PON_total_octets_raw_data_t));
		collector_id = -1;	
		stat_parameter = 2;/*1-pon 2-system*/
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_OCTETS, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &totalOctetsRawData,
									   &timestamp 
										);	
		if(iRes != 0)
		{
			printf("\r\n  PON_RAW_STAT_TOTAL_OCTETS ERROR. PonId %d type Pon %d\r\n",ponId,stat_parameter);
		}
		memset(buf, 0, 256);	

	}

	
	{
		memset(&TotalDropFrameTxRawData, 0, sizeof(PON_total_tx_dropped_frames_raw_data_t) );
		collector_id = -1;	
		stat_parameter = llid;
		iRes = PAS_get_raw_statistics ( ponId, 
									   collector_id, 
									   PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES, 
									   stat_parameter,
									   /*PON_STATISTICS_QUERY_HARDWARE,*/
									   &TotalDropFrameTxRawData,
									   &timestamp 
										);	
		if (iRes != STATS_OK)
		{
		printf("\r\n  PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES ERROR. PonId %d Llid %d\r\n",ponId,collector_id);	
		}
		/*);		
	*/
	}	
	{/*In range length error*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&inRangeLengthErrRawData, 0, sizeof(PON_in_range_length_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS,
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&inRangeLengthErrRawData, 
										&timestamp
										);
		if (iRes != 0)
			printf("  %% ponId %d PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS ERR\r\n",ponId);

		
	}

	{/*frame too long err*/
		collector_id = -1;
		stat_parameter = 0;
		memset( &frameTooLongErrRawData, 0, sizeof(PON_frame_too_long_errors_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_FRAME_TOO_LONG_ERRORS, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&frameTooLongErrRawData,
										&timestamp
										);
		if (iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_FRAME_TOO_LONG_ERRORS ERRPR\r\n",ponId);
	}	

	{/*host frame for pon&per llid*/
		collector_id = -1;
		stat_parameter = llid;
		memset(&hostframeRawData, 0, sizeof(PON_host_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&hostframeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_FRAMES ERROR\r\n",ponId );		
	

	}
	
	{/*pause frame*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&pauseFrameRawData, 0, sizeof(PON_pause_frames_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_FRAMES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&pauseFrameRawData,
										&timestamp);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PAUSE_FRAMES ERROR\r\n",ponId);
		
		/*
		
		*/
	}
	
	{/*pause tiem*/
		collector_id = -1;
		stat_parameter = 0;
		memset(&pauseTimeRawData, 0, sizeof(PON_pause_time_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_PAUSE_TIME, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&pauseTimeRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_PAUSE_TIME ERROR!\r\n",ponId);
		
		
	}

	printf("\r\n");	
	printf("  RxFrameOk              :  %-12lu",totalFrameRawData.received_ok );
	memset(buf, 0, 256);
	VOS_MemZero( szText, 64);
	counter = totalOctetsRawData.received_ok;
	sprintf64Bits( szText, counter );
	sprintf(buf, "  RxOctetsOk             :  %s\r\n", szText );	
	printf("%-12s",buf);
	
	printf("  RxFrameErr             :  %-12lu",totalFrameRawData.received_error);
	printf("  RxInRangeLengthErr     :  %-12lu\r\n",inRangeLengthErrRawData.system_received);

	printf("  RxframeTooLongErr      :  %-12lu",frameTooLongErrRawData.system_received);
	printf("  RxHostFrameOk          :  %-12lu\r\n",hostframeRawData.system_received_ok);

	printf("  RxHostFrameDrop        :  %-12lu",hostframeRawData.system_dropped);
	printf("  RxPauseFrameOk         :  %-12lu\r\n",pauseFrameRawData.system_received_ok);

	/*for system transmitt*/
	printf("\r\n");       
	printf("  TxFrameOk              :  %-12lu",totalFrameRawData.transmitted_ok );
	
	memset(buf, 0, 256);
	VOS_MemZero( szText, 64 );
	counter = totalOctetsRawData.transmitted_ok;
	sprintf64Bits( szText, counter );
	sprintf(buf, "  TxOctetsOk             :  %s\r\n", szText );		
	printf("%-12s",buf);
	
	printf("  TxFrameDrop            :  %-12lu",TotalDropFrameTxRawData.total_system_dropped);
	printf("  TxHostFrameDrop        :  %-12lu\r\n",hostframeRawData.system_transmitted_ok);

	printf("  TxPauseFrameOk         :  %-12lu",pauseFrameRawData.system_transmitted_ok);
	printf("  TxPauseTime            :  %lu\r\n",pauseTimeRawData.system_port);
	printf("\r\n");

	return 0;
}

STATUS CliRealTimeStatsHostMsg_test( short int ponId )
{
	int iRes = 0;
	short int collector_id = 0;
	short int stat_parameter = 0;
	PON_timestamp_t timestamp;
	PON_host_messages_raw_data_t	hostMsgRawData;
	{/*host message*/
		collector_id = -1;
		stat_parameter = 1;/*Host physical interface type (0 – Parallel, 1 – Ethernet, 2 - UART)*/
		memset(&hostMsgRawData,0 , sizeof(PON_host_messages_raw_data_t));
		iRes = PAS_get_raw_statistics( ponId, 
										collector_id, 
										PON_RAW_STAT_HOST_MESSAGES, 
										stat_parameter, 
										/*PON_STATISTICS_QUERY_HARDWARE,*/
										&hostMsgRawData,
										&timestamp
										);
		if(iRes != 0)
			printf("  %% PonId %d PON_RAW_STAT_HOST_MESSAGES ERRPR\r\n",ponId);
		printf("\r\n");
		printf("  RxHostMsgOk            :  %-12lu",hostMsgRawData.received_from_firmware_ok);
		printf("  TxHostMsgOk            :  %-12lu\r\n",hostMsgRawData.sent_to_firmware_ok);
		printf("\r\n");
	}	
	return 0;
}

#endif

