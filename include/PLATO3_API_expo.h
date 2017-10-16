/*
 * PLATO3_API.h
 *
 *  This software is licensed for use according to the terms set by the
 *  Passave API license agreement.
 *  Copyright Passave Ltd. 
 *  Ackerstein Towers - A, 9 Hamenofim St.
 *  POB 2089, Herziliya Pituach 46120 Israel
 *
 */


#ifndef _PLATO3_API_H_
#define _PLATO3_API_H_

/*#include "../PAS/PAS_expo.h" */

#define PLATO3_ERR_OK                PAS_EXIT_OK
#define PLATO3_ERR_GENERAL_ERROR     PAS_EXIT_ERROR
#define PLATO3_ERR_PARAMETER_ERROR   PAS_PARAMETER_ERROR
#define PLATO3_ERR_DBA_NOT_LOADED    PAS_DBA_DLL_NOT_LOADED
#define PLATO3_ERR_NO_COMMUNICATION  PAS_TIME_OUT
#define PLATO3_ERR_OLT_NOT_EXIST     PAS_OLT_NOT_EXIST
#define PLATO3_ERR_DBA_NOT_RUNNING   PAS_DBA_NOT_RUNNING
#define PLATO3_ERR_ONU_NOT_AVAILABLE PAS_ONU_NOT_AVAILABLE

#define PLATO3_ECODE_NO_ERROR                    0
#define PLATO3_ECODE_UNKNOWN_LLID                1
#define PLATO3_ECODE_TOO_MANY_LLIDS              2
#define PLATO3_ECODE_ILLEGAL_LLID                3
#define PLATO3_ECODE_MIN_GREATER_THAN_MAX        4
#define PLATO3_ECODE_MIN_TOTAL_TOO_LARGE         5
#define PLATO3_ECODE_ILLEGAL_CLASS               6
#define PLATO3_ECODE_ILLEGAL_GR_BW               7
#define PLATO3_ECODE_ILLEGAL_BE_BW               8
#define PLATO3_ECODE_ILLEGAL_BURST               9
#define PLATO3_ECODE_ILLEGAL_FIXED_BW            10
#define PLATO3_ECODE_ILLEGAL_FIXED_PACKET_SIZE   11
#define PLATO3_ECODE_WRONG_DBA_TYPE              12
#define PLATO3_ECODE_ILLEGAL_DBA_TYPE            13
#define PLATO3_ECODE_UNSUPPORTED_DBA_TYPE        14
#define PLATO3_ECODE_ILLEGAL_WEIGHT              15
#define PLATO3_ECODE_WRONG_WEIGHTS_SUM           16
#define PLATO3_ECODE_ILLEGAL_BE_METHOD           17
#define PLATO3_ECODE_UNSUPPORTED_BE_METHOD       18
#define PLATO3_ECODE_ILLEGAL_MIN_SP              19

#define PLATO3_NUMBER_OF_SERVICES 4 /* The number of queues in the ONU */

typedef enum {
  PLATO3_DBA_TYPE_GLOBAL,
  PLATO3_DBA_TYPE_SERVICE
} PLATO3_DBA_type_t;

typedef enum {
  PLATO3_BE_METHOD_STRICT,
  PLATO3_BE_METHOD_WFQ,
  PLATO3_BE_METHOD_SP_WFQ,
} PLATO3_SERVICE_BE_method_t;

typedef struct {
  unsigned short class;             /* 0 (lowest priority) – 7 (highest priority) */
  unsigned short fixed_packet_size;	/* Packet size of fixed allocation (bytes) */
  unsigned short fixed_bw;			/* Fixed guaranteed bandwidth (1Mbps units)  */
  unsigned short fixed_bw_fine;     /* Fixed guaranteed bandwidth (64Kbps units)  */
  unsigned short max_gr_bw;         /* Max guaranteed bandwidth (1Mbps units)  */
  unsigned short max_gr_bw_fine;    /* Max guaranteed bandwidth (64Kbps units)  */
  unsigned short max_be_bw;         /* Max best effort bandwidth (1Mbps units) */
  unsigned short max_be_bw_fine;    /* Max best effort bandwidth (64Kbps units) */
} PLATO3_SLA_t;

typedef struct {
  PLATO3_SERVICE_BE_method_t best_effort_method;  /* The method in which best-effort is allocated among the services */
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
  } service_SLA[PLATO3_NUMBER_OF_SERVICES];
} PLATO3_SERVICE_SLA_t;

typedef struct {
  unsigned short cycle_length;        /* Cycle length in 50us units */
  unsigned short discovery_frequency; /* Disocvery frequency in 1ms units */
  unsigned short max_llids;           /* Maximum number of LLIDs expected */
} PLATO3_conf_t;

PON_STATUS PLATO3_init( void );

PON_STATUS PLATO3_cleanup( void );

/* this function should be called by PAS_HANDLER_DBA_EVENT handler function
   when its olt_id parameter is an OLT which runs PLATO3 DBA */
PON_STATUS PLATO3_algorithm_event( short olt_id, unsigned short id, short size,
                                   const char *data );

PON_STATUS PLATO3_get_info( short olt_id, char *name,
                            unsigned char max_name_size, char *version, 
                            unsigned char max_version_size ); 
          
PON_STATUS PLATO3_set_DBA_type( short olt_id, unsigned char LLID,
                                PLATO3_DBA_type_t DBA_type, short *DBA_error_code );
PON_STATUS PLATO3_get_DBA_type( short olt_id, unsigned char LLID,
                                PLATO3_DBA_type_t *DBA_type, short *DBA_error_code );

PON_STATUS PLATO3_set_SLA( short olt_id, unsigned char LLID,
                           const PLATO3_SLA_t *SLA, short *DBA_error_code );
PON_STATUS PLATO3_get_SLA( short olt_id, unsigned char LLID,
                                 PLATO3_SLA_t *SLA, short *DBA_error_code );

PON_STATUS PLATO3_set_service_SLA( short olt_id, unsigned char LLID,
                                   const PLATO3_SERVICE_SLA_t *service_SLA, 
                                   short *DBA_error_code );
PON_STATUS PLATO3_get_service_SLA( short olt_id, unsigned char LLID,
                                   PLATO3_SERVICE_SLA_t *service_SLA, 
                                   short *DBA_error_code );

/*** supporting REDUNDANCY_MNG package ***/

/* PLATO3 handler functions enum */
typedef enum
{
	PLATO3_HANDLER_UPDATE_LLID_FLAGS,    /*Update llid_flags event*/
    PLATO3_HANDLER_LAST_HANDLER,

} PLATO3_handler_index_t;

/* Assign handler function									
**
** Assign (register) a handler function which handles a single event type.
**
** Input Parameters:
**			    handler_function_index  : Event type to be handled by the handler function
**			    handler_function        : Pointer to the handler function
** Output Parameters:
**			    handler_id			    : Handler function identification. Should be used when activating Delete handler function
**										  
**
** Return codes:
**		All the defined PLATO3_* return codes
*/
PON_STATUS PLATO3_assign_handler_function
                ( const PLATO3_handler_index_t    handler_function_index, 
				  const void					  (*handler_function)(),
				  unsigned short                  *handler_id );

/* Delete handler function									
**
** Remove a handler function from the DB.
**
** Input Parameters:
**	    handler_id     : Handler function identification.
**										  
**
** Return codes:
**		All the defined PLATO3_* return codes
*/
PON_STATUS PLATO3_delete_handler_function ( const unsigned short  handler_id );


/*event to notify llid_flags was updated*/
typedef void (*PLATO3_llid_flags_handler_t) 
				  ( const PON_olt_id_t		olt_id, 
					const PON_onu_id_t		onu_id );


PON_STATUS PLATO3_set_plato3_llid_flags( const PON_olt_id_t olt_id, const PON_onu_id_t onu_id,
                                         const unsigned short llid_flags);

PON_STATUS PLATO3_get_plato3_llid_flags( const PON_olt_id_t olt_id, const PON_onu_id_t onu_id,
                                         unsigned short *llid_flags);

/*** end of supporting REDUNDANCY_MNG package ***/

#endif

