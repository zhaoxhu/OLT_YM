/**************************************************************
*
*   IncludeFromPas.h -- 
*
*  
*    Copyright (c)  2006.4 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ---------|--- -------|---------------------|------------
*	1.00	  | 22/05/2006 | Creation				| chen fj
*    1.01       | 02/03/2007| Modified                   | chenfj  for PAS5201
*
***************************************************************/
#ifndef  __INCLUDEFROMPAS_H
#define __INCLUDEFROMPAS_H

#ifndef PAS_SOFT_5001
#define  PAS_SOFT_5001
#endif

#ifndef PAS_SOFT_5201
#define  PAS_SOFT_5201
#endif

#ifndef bool
#define bool unsigned char /* TRUE / FALSE */
#endif

#ifndef EOF
#define	EOF	(-1)
#endif

#ifndef MIN
#define MIN(_x,_y)  ((_x)<(_y))?(_x):(_y);
#endif

#ifndef MAX
#define MAX(_x,_y)  ((_x)>(_y))?(_x):(_y);
#endif

/* Fast macro for swapping between types which support the ^= operator */
#ifndef SWAP
#define	SWAP(_x,_y)	(_x) ^= (_y); (_y) ^= (_x); (_x) ^= (_y)
#endif


/* Activation commands */ 
#ifndef OFF
#define OFF   0
#define ON    1
#endif

#ifndef ENABLE
#define ENABLE  1
#define DISABLE 0
#endif

#ifndef RESET
#define RESET 2
#endif


/* Basic types ranges */

/* Unsigned char range: 0 - 255 */
#define MINUCHAR 0
#define MAXUCHAR 0xFF



/* Unsigned word range: 0 - 0xFFFF */
#define MINUWORD 0
#define MAXUWORD (0xFFFF)



#ifndef MAXINT
#define MAXINT 32767
#define MININT (-32767)
#endif


/* Unsigned long range: 0 - 0xFFFFFFFF */
#define MINULONG 0
#define MAXULONG (0xFFFFFFFF)


#ifndef disable_enable_t
typedef char disable_enable_t;
#endif 

/***  return  code difined ***/
#ifndef PAS_EXIT_OK

#ifdef  PLATO_DBA_V3
#define PLATO_DBA_V3_3
#endif

#ifdef  PLATO_DBA_V35
#define PLATO_DBA_V3_5
#endif


/* B--added by liwei056@2011-06-23 for NewVersion5.3.13 */
#ifdef PAS_SOFT_VERSION_V5_3_13
#define  PAS_SOFT_VERSION_V5_3_12
#endif
/* E--added by liwei056@2011-06-23 for NewVersion5.3.13 */


/* B--added by liwei056@2010-06-22 for NewVersion5.3.12 */
#ifdef PAS_SOFT_VERSION_V5_3_12
#define  PAS_SOFT_VERSION_V5_3_11
#endif
/* E--added by liwei056@2009-06-22 for NewVersion5.3.12 */


/* B--added by liwei056@2009-11-3 for NewVersion5.3.11 */
#ifdef PAS_SOFT_VERSION_V5_3_11
#define  PAS_SOFT_VERSION_V5_3_9
#endif
/* E--added by liwei056@2009-11-3 for NewVersion5.3.11 */

#ifdef PAS_SOFT_VERSION_V5_3_9
#define  PAS_SOFT_VERSION_V5_3_5
#endif


/* B--added by liwei056@2011-11-30 for OLT's ManageResume */
#undef OLT_RECOVERY_BY_EVENT

#ifdef OLT_RECOVERY_BY_EVENT
#undef  PAS_RECOVERY_BY_EVENT
#undef  OLT_RECOVERY_BY_DATA
#else
#define OLT_RECOVERY_BY_DATA
#endif
/* E--added by liwei056@2011-11-30 for OLT's ManageResume */

/*
**					API functions return codes						
**
** Note:  Some of these codes are for internal use 
*/

#define PON_CLI_MAX_COMMAND_LEN  120

/* Generic exit codes */
#define EXIT_OK             0
#define EXIT_ERROR        (-1)
#define TIME_OUT          (-2)
#define NOT_IMPLEMENTED	  (-3)
#define PARAMETER_ERROR   (-4)
#define HARDWARE_ERROR	  (-5)
#define MEMORY_ERROR	  (-6)
#define QUEUE_IS_FULL     (-7)
#define QUEUE_IS_EMPTY    (-8)


#define OLT_ERR_NOTREADY     (-9999)
#define OLT_IS_NOTREADY(rlt) ( OLT_ERR_NOTREADY == (rlt) )

#define FIRMWARE_LOAD_ERROR (-10000)
#define FIRMWARE_IS_NEED    (-10001)
#define FIRMWARE_IS_ERROR   (-10002)
#define FIRMWARE_LOAD_ISFAILED(rlt)  ( (-10000 >= (rlt)) && (-200000 < (rlt)) )

#define DBA_LOAD_ERROR      (-20000)
#define DBA_IS_NEED         (-20001)
#define DBA_IS_ERROR        (-20002)
#define DBA_LOAD_ISFAILED(rlt)       ( (-20000 >= (rlt)) && (-300000 < (rlt)) )

#define FILE_IS_NEED(rlt)  ( (FIRMWARE_IS_NEED == (rlt)) || (DBA_IS_NEED == (rlt)) )
#define FILE_IS_BAD(rlt)   ( (FIRMWARE_IS_ERROR == (rlt)) || (DBA_IS_ERROR == (rlt)) )


/* Remote exit code */
#define IGMP_ENTRY_EXISTS (-3012)
#define IGMP_TABLE_FULL	  (-3013)


#define ENTRY_ALREADY_EXISTS		(-3014)
#define TABLE_FULL					(-3015)
#define TABLE_EMPTY					(-3017)
#define OLT_WAS_RESETED  			(-2011)


#define PAS_EXIT_OK										EXIT_OK
#define PAS_EXIT_ERROR									EXIT_ERROR
#define PAS_TIME_OUT									TIME_OUT
#define PAS_NOT_IMPLEMENTED								NOT_IMPLEMENTED
#define PAS_PARAMETER_ERROR								PARAMETER_ERROR
#define PAS_HARDWARE_ERROR								HARDWARE_ERROR
#define PAS_MEMORY_ERROR								MEMORY_ERROR
#define PAS_PC_UART_ERROR								(-1000)
#define PAS_PC_CRC_ERROR								(-1001)
#define PAS_PARSE_ANSWER_ERROR							(-1002)
#define PAS_OLT_NOT_EXIST								(-1003)
#define PAS_QUERY_FAILED								(-1004) /* No record was found to match the */
																/* key of a query				    */
#define PAS_NOT_SUPPORTED_IN_CURRENT_HARDWARE_VERSION	(-1005)
#define PAS_OAM_LINK_NOT_ESTABLISHED					(-1006)
#define PAS_ONU_NOT_AVAILABLE							(-1007) /* ONU doesn't exist / during mode  */
																/* change process					*/
#define PAS_DBA_DLL_NOT_LOADED							(-1008) /* DBA algorithm not loaded to OLT, */
																/* DBA commands can't be executed	*/
#define PAS_DBA_ALREADY_RUNNING							(-1009) /* External DBA algorithm already   */
																/* running,can't initiate new       */
																/* provisioning method				*/
#define PAS_DBA_NOT_RUNNING								(-1010) /* Command can't be executed because*/
																/* it requires external DBA running */
																/* on the specified OLT				*/
#define PAS_DEVICE_VERSION_MISMATCH_ERROR               (-1011) /* The referred device version (OLT */
                                                                /* / ONU) doesn't support the       */
                                                                /* requested capability             */
#define PAS_SW_VERSION_MISMATCH_ERROR                   (-1012) /* The referred device software     */
                                                                /* version (OLT / ONU) doesn't      */
                                                                /* support the requested capability */
#define PAS_ADDRESS_TABLE_FULL							(-1013) /* Address table is full, can not   */
																/* add the specific address         */
#define PAS_ADDRESS_TABLE_ENTRY_LIMITER_FULL			(-1014) /* Address table entry limiter is   */
																/* full, can not add the specific   */															
																/* address                          */	

#define PAS_EXIT_SW_DOESNT_CONTROL_LED					(-1015) /* SW does not control the LEDS     */

#endif

#define BITS_IN_BYTE    8
#define BYTES_IN_LONG   4
#define BYTES_IN_DOUBLE 8
#define BYTES_IN_SYMBOL 4
#define BYTES_IN_WORD   2
#define BYTES_IN_CHAR	1

#ifndef BYTES_IN_MAC_ADDRESS
#define BYTES_IN_MAC_ADDRESS 6
#endif

typedef unsigned char mac_address_t [BYTES_IN_MAC_ADDRESS]; 


/* Print input MAC address to a string										*/
/* sample '__mac_string__' definition: char mac_string[BYTES_IN_MAC_ADDRESS*6]; (PON_DECLARE_MAC_STRING) */
#define PON_JOINED_MAC_ADDRESS_SPRINTF(__mac_address__, __mac_string__)	     sprintf (__mac_string__,"%02X%02X.%02X%02X.%02X%02X",__mac_address__[0],__mac_address__[1],__mac_address__[2],__mac_address__[3],__mac_address__[4],__mac_address__[5])
#define PON_MAC_ADDRESS_SPRINTF(__mac_address__, __mac_string__)  sprintf (__mac_string__,"%02X-%02X-%02X-%02X-%02X-%02X",__mac_address__[0],__mac_address__[1],__mac_address__[2],__mac_address__[3],__mac_address__[4],__mac_address__[5])

/* Convention for mac string declaration */
#define PON_MAC_STRING_NAME												mac_string	
#define PON_DECLARE_MAC_STRING											char PON_MAC_STRING_NAME[BYTES_IN_MAC_ADDRESS*6+2]
#define PON_MAC_ADDRESS_SPRINTF_INTO_MAC_STRING(__mac_address__)	    PON_MAC_ADDRESS_SPRINTF(__mac_address__, PON_MAC_STRING_NAME)
#define PON_JOINED_MAC_ADDRESS_SPRINTF_INTO_MAC_STRING(__mac_address__) PON_JOINED_MAC_ADDRESS_SPRINTF(__mac_address__, &PON_MAC_STRING_NAME)


#ifndef BOOLEAN
typedef unsigned char BOOLEAN;
#endif
#ifndef INT8U
typedef unsigned char  INT8U;   /* Unsigned 8 bit quantity */
#endif
#ifndef INT16U
typedef unsigned short  INT16U;  /* Unsigned 16 bit quantity */
#endif
#ifndef INT32U
typedef unsigned long  INT32U;  /* Unsigned 32 bit quantity */
#endif

/* 
** IPv6 address definitions
*/
#define PON_IPV6_ADDRESS_SIZE  4
typedef union {
    INT32U addr_32[PON_IPV6_ADDRESS_SIZE];
    INT16U addr_16[PON_IPV6_ADDRESS_SIZE<<1];
    INT8U  addr_8 [PON_IPV6_ADDRESS_SIZE<<2];
} PON_ipv6_addr_t;

#ifndef PON_VALUE_NOT_CHANGED
#define PON_VALUE_NOT_CHANGED  (-1)
#define PON_UNKNOWN_VALUE_STRING	"unknown"
#define PON_NOT_CHANGED_IP_STRING	 "255.255.255.255"
#endif


/*
** 802.3ah (EPON) standard definitions
*/
#define PON_NETWORK_RATE	 (PON_1000M * 1000000) /* PON rate: 1 Gigabits / Sec */
								
#define TQ_IN_A_SECOND				    (62500000) /* 16nSec - standard */

#define PON_GRANT_MAX_LENGTH				0xFFFF /* stated in TQ, 802.3ah (EPON) standard, includes  */
												   /* ONU transmission overhead (laser on time + CDR   */
												   /* time + AGC time).								   */

#ifndef PON_OLT_ID
#define PON_OLT_ID							(-1)   /* Device ID which indicates an OLT				   */
#define PON_MIN_LLID_PER_OLT_STANDARDIZED  0  /* 802.3ah (EPON) standard						   */
#define PON_MAX_LLID_PER_OLT_STANDARDIZED   32767  /*802.3ah (EPON) standard,15 bits out of 16bit field*/
#endif

#ifdef PAS_SOFT_VERSION_V5_3_13
#define PON_LLID_SESSION_ID_INVALID       0xFFFF           /*define an invalid session ID*/
#endif

/* LLID and ONU special values definitions */
#define PON_LLID_NOT_AVAILABLE		(-1) /* Value indicating LLID value is not available and should be */
									     /* determined automatically by OLT hardware (according to MAC */
									     /* address)
												   */
#define PON_EVERY_LLID				((-1)*PON_MAX_LLID_PER_OLT_STANDARDIZED)

#define PON_ONU_ID_NOT_AVAILABLE	PON_LLID_NOT_AVAILABLE

#define PON_OLT_AND_ONU_COMBINED_ID	(-2) /* When numbering devices includes OLT & ONU combination this */
										 /* is the index for the combination						   */

#define PON_PAS_SOFT_ID				(-3) /* When numbering devices includes PAS-SOFT,this is the index */
										 /* for PAS-SOFT indication	*/

#define PAS_ADDRESS_TABLE_SIZE  8192
#ifndef PON_ADDRESS_TABLE_SIZE
#define PON_ADDRESS_TABLE_SIZE  8192 /* 16384 */
#endif

#ifndef DEFAULT_LINK_ALARM_PARAMETER_VALUE
#define DEFAULT_LINK_ALARM_PARAMETER_VALUE	(-1) /* Value used to mark PON software internally */
					              /* configured link alarm parameters	*/
#endif

/* -1 is illegal OLT/ ONU / LLID value so in some cases it can be used for non-single OLT/ ONU / LLID */
/* marking																							  */
#define PON_OLT_ID_NOT_AVAILABLE (-1) /* OLT id not available */
#define PON_ALL_ACTIVE_OLTS      (-1) /* All active OLTs  */
#define PON_ALL_ACTIVE_ONUS      (-1) /* All active ONUs  */
#define PON_ALL_ACTIVE_LLIDS     (-1) /* All active LLIDs */
#define PON_DEFAULT_LLID         (-1) /* Default llid */
#define PON_ALL_LLIDS            (-1) /* All the LLIDs without checking registering state, it will be translated to be 0xFFFF when composing*/


#define MAKE_UNSIGNED_64_PARAM( msb, lsb ) ((msb * 0x100000000) + lsb)

#define MAKE_UNSIGNED_LONG_PARAM( msb, lsb ) ((msb * 0x10000) + lsb)

#define MAKE_UNSIGNED_SHORT_PARAM( msb, lsb ) ((msb * 0x100) + lsb)

#define PON_MAC_ADDRESS_COPY(dst_address, src_address)  memcpy(dst_address, src_address, BYTES_IN_MAC_ADDRESS);

#define GET_MSB_LSB_FROM_UNSIGNED_64_PARAM(number,msb, lsb) \
{ \
    lsb = (unsigned long) (number & 0x00000000ffffffff); \
    msb = (unsigned long) ( (number >> BITS_IN_BYTE * BYTES_IN_LONG )& 0x00000000ffffffff); \
} \

typedef short int  PON_onu_id_t;  /* C-type representation of ONU index */
#define PON_llid_t PON_onu_id_t	  /* In current version there is 1:1 ratio between ONU and LLID */

typedef short int  PON_olt_id_t;  /* C-type representation of OLT index */

#ifndef MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS
#define	MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS	128
typedef  mac_address_t mac_addresses_list_t[MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS];
#endif


#if 1
/*
#define PON_MALLOC_FUNCTION(size)  VOS_Malloc(size, MODULE_PON)
#define PON_FREE_FUNCTION(p)       VOS_Free(p)
*/
#ifndef USE_VOS_MALLOC
#define USE_VOS_MALLOC  1
#endif

#ifdef USE_VOS_MALLOC
extern void* VOS_MallocWithDebug (unsigned long size, unsigned long moduleId, unsigned char *file, unsigned long line);
#define VOS_Malloc(ulSize,ulModuleId) VOS_MallocWithDebug(ulSize,ulModuleId,__FILE__,__LINE__)
#endif

#ifndef VOS_BIG_MEM_SUPPORT
#define VOS_BIG_MEM_SUPPORT     1
#endif

#if VOS_BIG_MEM_SUPPORT 
#ifdef g_malloc
#undef g_malloc
#endif

#ifdef g_free
#undef g_free
#endif

#define g_malloc(s)	    VOS_MallocWithDebug(s, 0x8d00 /*MODULE_MEM_GMALLOC*/, __FILE__, __LINE__)
#define g_free(s)		    VOS_Free(s)
#define g_calloc(a,b)         VOS_MallocWithDebug((a*b),0x8d00, __FILE__, __LINE__)
#define g_realloc(a,b)        VOS_Realloc(a,b,0x8d00)
#endif

/* Definition for alternative g_malloc and free function name used */ 
#ifdef USE_SYS_MALLOC

#define PON_MALLOC_FUNCTION  sys_malloc

#else

#ifdef USE_VOS_MALLOC

#define PON_MALLOC_FUNCTION  g_malloc

#else

#define PON_MALLOC_FUNCTION  malloc

#endif

#endif


#ifdef USE_VOS_MALLOC

#define PON_FREE_FUNCTION  g_free

#else

#define PON_FREE_FUNCTION  free

#endif


#ifdef USE_SYS_MALLOC

#define PON_REALLOC_FUNCTION  sys_realloc

#else

#define PON_REALLOC_FUNCTION  g_realloc

#endif


#ifdef USE_SYS_MALLOC

#define PON_CALLOC_FUNCTION  sys_calloc

#else

#define PON_CALLOC_FUNCTION  g_calloc

#endif
#endif

#if 1
unsigned long OSSRV_create_thread ( void  *thread_function );
void OSSRV_terminate_thread ( void );

int OSSRV_wait ( unsigned short int  msec );
#endif


#if 1
/* Definition for alternative clock function name used */ 
#define USE_TICK_GET

#define PON_CLOCKS_PER_SEC                    TQ_IN_A_SECOND  

/* Clock ticks macro - ANSI version */
#define CLOCKS_PER_SEC  100

#ifdef USE_TICK_GET
    #define PON_CLOCK_FUNCTION tickGet
    #define PON_HOST_CLOCKS_PER_SEC CLOCKS_PER_SEC
    #define PON_clock_t unsigned long
#else
    #define PON_CLOCK_FUNCTION clock
    #define PON_HOST_CLOCKS_PER_SEC CLOCKS_PER_SEC
    #define PON_clock_t clock_t
#endif


#if defined (WIN32) && defined (DLL_EXPORTS)
    #define PON_STATUS          __declspec(dllexport) short int
    #define PON_LONG_STATUS     __declspec(dllexport) long
    #define PON_TEST_RESULT     __declspec(dllexport) PAS_olt_test_results_t
    #define PON_BOOL_STATUS     __declspec(dllexport) bool

#else
    #define PON_STATUS          short int
    #define PON_LONG_STATUS     long
    #define PON_TEST_RESULT     PAS_olt_test_results_t
    #define PON_BOOL_STATUS     bool
#endif


#define PON_DECLARE_TIMEOUT_VARIABLES \
	PON_clock_t		 __wait_start_time__, __wait_current_time__;\
	double			 __duration__ = 0, __timeout_time__, __timeout_loop__;\
	unsigned int	 __loop_counter__;

#define PON_TIMEOUT_INIT(__timeout__ /* Seconds, can be fraction */) \
	__wait_start_time__   = PON_CLOCK_FUNCTION();\
	__wait_current_time__ = PON_CLOCK_FUNCTION();\
	__duration__		  = 0;\
	__timeout_time__	  = __timeout__;\
	__timeout_loop__	  = (__timeout__ * 1000 / PON_DEFAULT_TIMEOUT_POLLING_CHECK);\
	__loop_counter__	  = 0;

#define PON_TIMEOUT_ITERATION \
	{OSSRV_wait (PON_DEFAULT_TIMEOUT_POLLING_CHECK);\
	__wait_current_time__ = PON_CLOCK_FUNCTION();\
	__loop_counter__++;\
	__duration__ = (double)(__wait_current_time__ - __wait_start_time__) / PON_HOST_CLOCKS_PER_SEC;}

#define PON_TIMEOUT_EXPIRED ((__duration__     < __timeout_time__) &&  \
						     (__loop_counter__ < __timeout_loop__))?(bool)FALSE:(bool)TRUE

#define PON_DEFAULT_TIMEOUT_POLLING_CHECK	10   /* Default polling time for timeout code, stated in Milliseconds */
#endif


#if 1
#define SW_ALARM_raise_alarm(error_id)    sys_console_printf("[SOFT-ERROR-ALARM:]%s", #error_id)

#define SEMAPHORE_INIT_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _result)\
	*(_semaphore) = VOS_SemCCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);\
	if (*(_semaphore) == 0)\
	{\
	    _result = VOS_ERROR; \
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d initializing event "#_semaphore_name"\n", _result);\
	}

#define EVENT_INIT_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _result)\
	*(_semaphore) = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_EMPTY);\
	if (*(_semaphore) == 0)\
	{\
	    _result = VOS_ERROR; \
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d initializing semaphore "#_semaphore_name"\n", _result);\
	}  


#define SEMAPHORE_CLOSE_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _result)\
	_result = VOS_SemDelete(*(_semaphore));\
	if (_result != EXIT_OK)\
	{\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d closing semaphore "#_semaphore_name"\n", _result);\
	}


#define SEMAPHORE_LISTEN_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _result)\
	_result = VOS_SemTake(*(_semaphore), NO_WAIT);\
	if (_result == EXIT_ERROR)\
	{\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d listening to "#_semaphore_name" semaphore\n", _result);\
	}

#define SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _signal_result)\
	_signal_result = VOS_SemTake(*(_semaphore), WAIT_FOREVER);\
	if (_signal_result != EXIT_OK)\
	{\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d signaling "#_semaphore_name" semaphore\n", _signal_result);\
	}

#define SIGNAL_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC(__functionality__, _semaphore, _semaphore_name, _signal_result)\
		_signal_result = (short int)VOS_SemGive(*(_semaphore));\
		if (_signal_result != EXIT_OK)\
		{\
			PONLOG_ERROR_1( __functionality__, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				   "Error %d signaling "#_semaphore_name" semaphore\n", _signal_result ); \
			SW_ALARM_raise_alarm(PON_SOFTWARE_ALARM_SEMAPHORE_ERROR);\
		}


#define RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR(_func, _semaphore, _semaphore_name, _release_result)\
	_release_result = VOS_SemGive(*(_semaphore));\
	if (_release_result != EXIT_OK)\
	{\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %d releasing "#_semaphore_name" semaphore\n", _release_result);\
	}

#define RELEASE_SEMAPHORE_AND_LOG_FOR_ERROR_FUNC(__functionality__, _semaphore, _semaphore_name, _release_result)\
		_release_result = (short int)VOS_SemGive(*(_semaphore));\
		if (_release_result != EXIT_OK)\
		{\
			PONLOG_ERROR_1( __functionality__, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT ,\
   	                        "Error %d releasing "#_semaphore_name" semaphore\n", _release_result);\
			SW_ALARM_raise_alarm(PON_SOFTWARE_ALARM_SEMAPHORE_ERROR);\
		}
    
#endif


#if 1
#define PON_OLT_ID_VARIABLE						   olt_id
#define PON_DECLARE_OLT_ID_VARIABLE				   PON_olt_id_t  PON_OLT_ID_VARIABLE;
#define PON_OLT_PARAMETER_IN_FUNCTION_DECLARATION  PON_olt_id_t  PON_OLT_ID_VARIABLE

#define PON_OLT_IN_BOUNDS ((PON_OLT_ID_VARIABLE >= PON_MIN_OLT_ID) && (PON_OLT_ID_VARIABLE <= PON_MAX_OLT_ID))
																	   
#define PON_OLT_BOUNDS_LOOP for(PON_OLT_ID_VARIABLE = PON_MIN_OLT_ID; PON_OLT_ID_VARIABLE <= PON_MAX_OLT_ID; PON_OLT_ID_VARIABLE ++)


/* Changed to avoid warning "comparison of unsigned expression >= 0 is always true" */
#define PON_CHECK_IN_BOUNDS(_variable_,_lower_limit_,_upper_limit_) (((_variable_ > _lower_limit_) || (_variable_ == _lower_limit_)) && ((_variable_ < _upper_limit_) || (_variable_ == _upper_limit_))) 

#define PON_CHECK_IN_BOUNDS_OR_VALUE(_variable_,_lower_limit_,_upper_limit_,_value_) ( ((_variable_ >= _lower_limit_) && (_variable_ <= _upper_limit_)) || (_variable_ == _value_) )

#define PON_CHECK_LESS_THAN(_variable_,_upper_limit_) ((_variable_ <= _upper_limit_)) 

#define PON_CHECK_MORE_THAN(_variable_,_lower_limit_) ((_variable_ >= _lower_limit_)) 

/* Macro for verifying a parameter value is equal to TRUE or FALSE */
#define PON_IS_BOOL_VARIABLE_IN_BOUNDS(__param__) ((__param__ == TRUE) || (__param__ == FALSE))?(bool)TRUE:(bool)FALSE
#define PON_IS_ENABLE_BOOL_VARIABLE_IN_BOUNDS(__param__) ((__param__ == ENABLE) || (__param__ == DISABLE))?(bool)TRUE:(bool)FALSE


/* Handle fail in CHECK_IN_BOUNDS*/
#define PON_GENERAL_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_,_return_code_)\
    if (PON_CHECK_IN_BOUNDS(_variable_,_lower_limit_,_upper_limit_) != TRUE) \
	{\
        PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error %s = %d out of bounds (%d..%d)\n",#_variable_name_,_variable_,_lower_limit_,_upper_limit_);\
		return (_return_code_);\
	}

#define PON_GENERAL_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(_func_,_variable_,_variable_name_,_lower_limit_,_upper_limit_,_return_code_)\
    if (PON_CHECK_IN_BOUNDS(_variable_,_lower_limit_,_upper_limit_) != TRUE) \
	{\
		PONLOG_ERROR_4( _func_, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
							"Error %s = %d out of bounds (%d..%d)\n", \
							#_variable_name_,_variable_,_lower_limit_,_upper_limit_); \
		return (_return_code_);\
	}


#define PON_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_,PARAMETER_ERROR)

#define PON_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(_func_,_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_HANDLE_ERROR_CHECK_IN_BOUNDS_FUNC(_func_,_variable_,_variable_name_,_lower_limit_,_upper_limit_,PARAMETER_ERROR)

#define PON_GENERAL_NO_OLT_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_,_return_code_)\
    if (PON_CHECK_IN_BOUNDS(_variable_,_lower_limit_,_upper_limit_) != TRUE) \
	{\
        PONLOG_ERROR_4(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %s = %d out of bounds (%d..%d)\n",#_variable_name_,_variable_,_lower_limit_,_upper_limit_);\
		return (_return_code_);\
	}

#define PON_NO_OLT_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_NO_OLT_HANDLE_ERROR_CHECK_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_,PARAMETER_ERROR)


#define PON_GENERAL_HANDLE_ERROR_CHECK_LESS_THAN(_variable_,_variable_name_,_upper_limit_,_return_code_)\
        if (PON_CHECK_LESS_THAN(_variable_,_upper_limit_) != TRUE) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error %s = %d out of bounds (0..%d)\n",#_variable_name_,_variable_,_upper_limit_);\
            return _return_code_; \
        }

#define PON_GENERAL_HANDLE_ERROR_CHECK_LESS_THAN_FUNC(_func,_variable_,_variable_name_,_upper_limit_,_return_code_)\
        if (PON_CHECK_LESS_THAN(_variable_,_upper_limit_) != TRUE) \
        { \
            PONLOG_ERROR_3(_func, olt_id, PONLOG_ONU_IRRELEVANT, "Error %s = %d out of bounds (0..%d)\n",#_variable_name_,_variable_,_upper_limit_);\
            return _return_code_; \
        }
        
#define PON_GENERAL_NO_OLT_HANDLE_ERROR_CHECK_LESS_THAN(_variable_,_variable_name_,_upper_limit_,_return_code_)\
        if (PON_CHECK_LESS_THAN(_variable_,_upper_limit_) != TRUE) \
        { \
            PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Error %s = %d out of bounds (0..%d)\n",#_variable_name_,_variable_,_upper_limit_);\
            return _return_code_; \
        }

#define PON_GENERAL_HANDLE_ERROR_BOOL(__param__, __param_name__,_return_code_) \
    if ( !PON_IS_ENABLE_BOOL_VARIABLE_IN_BOUNDS(__param__) ) \
{ \
    PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, "Error, %s access illegal value (%d)\n",#__param_name__,__param__)\
    return _return_code_; \
}


/* Handle fail in CHECK_UNSIGNED_IN_BOUNDS */
#define PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_HANDLE_ERROR_CHECK_LESS_THAN(_variable_,_variable_name_,_upper_limit_,PARAMETER_ERROR)


#define PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func,_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_HANDLE_ERROR_CHECK_LESS_THAN_FUNC(_func,_variable_,_variable_name_,_upper_limit_,PARAMETER_ERROR)

#define PON_NO_OLT_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS(_variable_,_variable_name_,_lower_limit_,_upper_limit_)\
        PON_GENERAL_NO_OLT_HANDLE_ERROR_CHECK_LESS_THAN(_variable_,_variable_name_,_upper_limit_,PARAMETER_ERROR)

/* Handle fail in CHECK_BOOL */
#define PON_CHECK_BOOL_VARIABLE_IN_BOUNDS(__param__, __param_name__) \
        PON_GENERAL_HANDLE_ERROR_BOOL(__param__, __param_name__,PARAMETER_ERROR)
#endif


#define	MAX_IGMP_PROXY_ENTRIES	256

/*  v5.3.0 之前版本 
typedef enum
{
	PON_IGMP_GROUP_STATE_ACTIVE = 0x1,
	PON_IGMP_GROUP_STATE_GROUP_SPECIFIC_PENDING = 0x2,
	PON_IGMP_GROUP_STATE_GROUP_GENERAL_PENDING  = 0x3,
	
}PON_igmp_group_status_t;
*/
typedef enum
{
	PON_IGMP_GROUP_STATUS_NO_GROUP_QUERY_PENDING = 0x1,
	PON_IGMP_GROUP_STATUS_GROUP_QUERY_PENDING  = 0x2,
	
}PON_igmp_group_status_t;


typedef struct PON_igmp_proxy_entry_t
{
	unsigned long				igmp_address;
	PON_igmp_group_status_t		group_status;
}PON_igmp_proxy_entry_t;

/*typedef  PON_igmp_proxy_entry_t igmp_proxy_table_t[MAX_IGMP_PROXY_ENTRIES];*/

/****  2007/04/29 新增加的********/

typedef enum
{
	PON_RECORD_MODE_EXCLUDE					= 0x0,
	PON_RECORD_MODE_INCLUDE					= 0x1,
	
}PON_record_mode_t;


#define	MAX_IGMP_PROXY_SOURCE_ADDRESSES_PER_GROUP	16

#define	MAX_IGMP_PROXY_GROUP_ADDRESS_DOES_NOT_EXIST	0xFF

typedef struct PON_igmp_proxy_group_address_t
{
	unsigned short				number_of_sources;
	PON_record_mode_t			record_mode;
	unsigned long				sources_address[MAX_IGMP_PROXY_SOURCE_ADDRESSES_PER_GROUP];
}PON_igmp_proxy_group_info_t;


typedef  PON_igmp_proxy_entry_t igmp_proxy_table_t[MAX_IGMP_PROXY_ENTRIES];


#ifdef  PAS_SOFT_VERSION_V5_3_11
typedef enum
{
	PON_IGMP_PROXY_ADMINISTRATIVE_VERSION_IGMP_V1		    = 0x1,
	PON_IGMP_PROXY_ADMINISTRATIVE_VERSION_IGMP_V2		    = 0x2,
	PON_IGMP_PROXY_ADMINISTRATIVE_VERSION_IGMP_V3		    = 0x3,
    PON_IGMP_PROXY_ADMINISTRATIVE_VERSION_IGMP_DONT_CHANGE  = 0xFF
}PON_igmp_proxy_administrative_version_t;

#else

typedef enum
{
	PON_IGMP_PROXY_SERVER_VERSION_LEARN_MODE	= 0x0,
	PON_IGMP_PROXY_SERVER_VERSION_IGMP_V2		= 0x2,
	PON_IGMP_PROXY_SERVER_VERSION_IGMP_V3		= 0x3,
	
}PON_igmp_proxy_server_version_t;


typedef enum
{
	PON_IGMP_PROXY_CLIENT_VERSION_IGMP_V1		= 0x1,
	PON_IGMP_PROXY_CLIENT_VERSION_IGMP_V2		= 0x2,
	PON_IGMP_PROXY_CLIENT_VERSION_IGMP_V3		= 0x3,
	
}PON_igmp_proxy_client_version_t;
#endif

/******  结束************/

/* OLT activation modes */ 
typedef enum 
{  
		PON_OLT_MODE_ON  = 101,         
		PON_OLT_MODE_OFF = 102       
} PON_olt_activation_modes_t;

/* ONU activation modes */ 
typedef enum 
{  
	PON_ONU_MODE_ON      = 201,         
	PON_ONU_MODE_OFF     = 202,       
	PON_ONU_MODE_PENDING = 203  
} PON_onu_activation_modes_t;

typedef enum
{
    PON_OLT_CLASSIFICATION_LINK_CONSTRAINT          = 1,   /* classification_parameter = PON_classification_link_constraint_frames_data_t */
    PON_OLT_CLASSIFICATION_802_1X                      ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_ARP                         ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_IGMP                        ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_MLD                         ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_OLT_MAC_ADDRESS             ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_STP                         ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_DHCP                        ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_PIM                         ,   /* classification_parameter = NULL */
    PON_OLT_CLASSIFICATION_NON_PRIORITIZED_FRAMES          /* classification_parameter = PON_classification_non_prioritized_frames_data_t */
} PON_olt_classification_t; 

typedef enum
{
    PON_OLT_CLASSIFIER_DESTINATION_DATA_PATH          ,
    PON_OLT_CLASSIFIER_DESTINATION_HOST               ,
    PON_OLT_CLASSIFIER_DESTINATION_HOST_AND_DATA_PATH ,
    PON_OLT_CLASSIFIER_DESTINATION_NONE               

} PON_olt_classifier_destination_t; 

/*
** 802.3 Ethernet frame definitions
*/
#define MIN_ETHERNET_FRAME_SIZE					60 /* Minimal Ethernet frame length excluding Preamble,*/
												   /* SFD and FCS fields							   */
#define MAX_ETHERNET_FRAME_SIZE_STANDARDIZED  1514 /* Maximal Ethernet frame length excluding Preamble */
												   /* ,SFD and FCS fields standard value, not including*/
												   /* any extensions	*/
/* 
** IGMP definitions
*/

/*
** ONU IGMP definitions
*/
#define PON_MAX_IGMP_ADDRESSES_PER_ONU   8 /* Maximal number of IGMP addresses an ONU can 'listen' to */

/*
** OLT IGMP definitions
*/
#define PON_NO_TIMEOUT	     0  /* Zero value on timeout counters sometimes represents no timeout */
#define PON_NO_IGMP_TIMEOUT  PON_NO_TIMEOUT  /* No IGMP timeout (no removal from ONU IGMP filter table */
											 /* according to membership message timeout)			   */

#define PON_IGMP_GRANULARITY 150			 /* in miliseconds										   */

/* 
** Port Id definitions
*/
#define PAS_PORT_ID_MAX_LLID_SUPPORTED  126 /* Maximum LLID supported in when sending frames to the PON */
#define PAS_PORT_ID_CNI					(PAS_PORT_ID_MAX_LLID_SUPPORTED+1)

#define PON_PORT_ID_MAX_LLID_SUPPORTED  259 /* (259~1023) Maximum LLID supported in when sending frames to the PON */
#define PON_PORT_ID_CNI					0

/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#define GW10G_PAS_PORT_ID_MAX_LLID_SUPPORTED  254
#define GW10G_PAS_PORT_ID_CNI            (GW10G_PAS_PORT_ID_MAX_LLID_SUPPORTED+1)    
#define GW1G_PAS_BROADCAST_LLID		  127  /* Special LLID, indicating one of the broadcast LLIDs,		    */
									   /* including multicast										    */
#define PAS_BROADCAST_LLID            0xFFF
#define GW10G_PAS_BROADCAST_LLID      0xFFF
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/									   /* including multicast										    */
#define PAS_DISCOVERY_LLID		  PAS_BROADCAST_LLID  /* Broadcast LLID also indicates discovery grants */


#define PON_BROADCAST_LLID		  0   /* Special LLID, indicating one of the broadcast LLIDs,		    */
									   /* including multicast										    */
#define PON_DISCOVERY_LLID		  0 /* Broadcast LLID also indicates discovery grants */
#define PON_VIRTUAL_LLID		  0  /* Virtual LLID for ALARM_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON */

#define PON_NOT_USED_ONU_ID		  PON_DISCOVERY_LLID
#define PON_NOT_USED_LLID		  PON_NOT_USED_ONU_ID


#define PAS_MIN_LLID_PER_OLT	  PON_MIN_LLID_PER_OLT_STANDARDIZED
#define PAS_MAX_LLID_PER_OLT	  PAS_PORT_ID_MAX_LLID_SUPPORTED /* Actually MIN (PON_MAX_LLID_PER_OLT_STANDARDIZED, PON_PORT_ID_MAX_LLID_SUPPORTED) */

#define PON_MIN_LLID_PER_OLT	  PON_MIN_LLID_PER_OLT_STANDARDIZED
#define PON_MAX_LLID_PER_OLT	  PON_PORT_ID_MAX_LLID_SUPPORTED /* Actually MIN (PON_MAX_LLID_PER_OLT_STANDARDIZED, PON_PORT_ID_MAX_LLID_SUPPORTED) */

#define PAS_MIN_ONU_ID_PER_OLT	  PON_MIN_LLID_PER_OLT /* In current version there is an one-to-one		*/
													   /* connection between ONU index and LLID			*/
#define PAS_MAX_ONU_ID_PER_OLT	  PAS_MAX_LLID_PER_OLT /* In current version there is an one-to-one		*/
													   /* connection between ONU index and LLID			*/

#define PON_MIN_ONU_ID_PER_OLT	  PON_MIN_LLID_PER_OLT /* In current version there is an one-to-one		*/
													   /* connection between ONU index and LLID			*/
#define PON_MAX_ONU_ID_PER_OLT	  PON_MAX_LLID_PER_OLT /* In current version there is an one-to-one		*/
													   /* connection between ONU index and LLID			*/

#define PON_STATISTICS_SYSTEM_LLID  128 

#define PON_STATISTICS_ALL_LLID    129 /* Special LLID, indicating an accumulation of all LLIDs		    */


#define PON_llid_t PON_onu_id_t	  /* In current version there is 1:1 ratio between ONU and LLID */

#define PON_MAX_MSG_LOG_EVENT_SIZE   500	   /* Maximal PAS-SOFT <-> OLT FW log event size, measured in bytes */

#define PON_MAX_PING_LENGTH	   100	  /* maximal ping frame send by ping API command   */


#define PON_MAX_RTT	       MAXINT		/* Stated in TQ, represents the maximum PON diameter reasonable	  */
										/* for an EPON access system, 52.4KM ~ 534 MicroSeconds. 		  */
										/* PAS-SOFT uses this definition for discovery grant safe guard	  */
										/* calculations.												  */

#define PON_MAX_SPANNING_TREE_TIMING 3600 /* Single constant used for the upper bound of all spanning */
										  /* tree algorithm time-dependant configurations. Stated in  */
										  /* seconds (corresponds to 1 hour)						  */

/* 
** DBA algorithms definitions 
** For DBA algorithms detailed descriptions refer to 'Passave DBA Overview' document
*/

#define PON_MAX_DBA_DATA_SIZE 1500 /* DBA msg / event data size, measured in bytes */




#define PON_EMPTY_ARP_FILTERING_STRUCT(__arp_filtering_configuration_struct__) \
	__arp_filtering_configuration_struct__.from_pon_to_firmware = PON_VALUE_NOT_CHANGED;\
	__arp_filtering_configuration_struct__.from_cni_to_firmware = PON_VALUE_NOT_CHANGED;\
	__arp_filtering_configuration_struct__.from_pon_to_cni = PON_VALUE_NOT_CHANGED;\
	__arp_filtering_configuration_struct__.from_cni_to_pon = PON_VALUE_NOT_CHANGED;

#define PON_EMPTY_OLT_VLAN_CONFIGURATION_STRUCT(__olt_vlan_configuration_struct__) \
	__olt_vlan_configuration_struct__.vlan_exchange_downlink_tag_prefix = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_configuration_struct__.nested_mode_vlan_type			    = PON_VALUE_NOT_CHANGED;\


#define PON_EMPTY_OLT_HEC_CONFIGURATION_STRUCT(__olt_hec_configuration_struct__) \
	__olt_hec_configuration_struct__.tx_hec_configuration = PON_VALUE_NOT_CHANGED;\
	__olt_hec_configuration_struct__.rx_hec_configuration = PON_VALUE_NOT_CHANGED;

#define PON_EMPTY_OLT_VLAN_TAG_FILTERING_CONFIGURATION_STRUCT(__olt_vlan_tag_filtering_configuration_struct__)\
	__olt_vlan_tag_filtering_configuration_struct__.untagged_frames_filtering = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_tag_filtering_configuration_struct__.multiple_copy_broadcast_enable_mode = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_tag_filtering_configuration_struct__.filtering_unexpected_tagged_downstream_frames = PON_VALUE_NOT_CHANGED;


/* Fill a PON_VALUE_NOT_CHANGED value in every field of a PON_olt_update_parameters_t struct		*/
/* After using this macro, the caller of PAS_update_olt_parameters may fill only the updated params */
/* in the struct																					*/
#define PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT(__olt_update_parameters_struct__) \
	__olt_update_parameters_struct__.max_rtt = PON_VALUE_NOT_CHANGED;\
	__olt_update_parameters_struct__.address_table_aging_timer = PON_VALUE_NOT_CHANGED;\
	__olt_update_parameters_struct__.cni_port_maximum_entries = PON_VALUE_NOT_CHANGED;\
	PON_EMPTY_ARP_FILTERING_STRUCT (__olt_update_parameters_struct__.arp_filtering_configuration)\
	__olt_update_parameters_struct__.igmp_configuration.enable_igmp_snooping = PON_VALUE_NOT_CHANGED;\
	__olt_update_parameters_struct__.igmp_configuration.igmp_timeout = PON_VALUE_NOT_CHANGED;\
	PON_EMPTY_OLT_VLAN_CONFIGURATION_STRUCT (__olt_update_parameters_struct__.vlan_configuration)\
	PON_EMPTY_OLT_HEC_CONFIGURATION_STRUCT (__olt_update_parameters_struct__.hec_configuration)\
	PON_EMPTY_OLT_VLAN_TAG_FILTERING_CONFIGURATION_STRUCT (__olt_update_parameters_struct__.vlan_tag_filtering_config)\
	__olt_update_parameters_struct__.upstream_default_priority = PON_VALUE_NOT_CHANGED;\
    __olt_update_parameters_struct__.pon_tbc_polarity = PON_VALUE_NOT_CHANGED;


/* Fill a PON_VALUE_NOT_CHANGED value in every field of a PON_update_olt_parameters_t struct		*/
/* After using this macro, the caller of PON_update_olt_parameters_t may fill only the updated params */
/* in the struct																					*/
#define PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT(___update_olt_parameters___) \
	___update_olt_parameters___.max_rtt = PON_VALUE_NOT_CHANGED;\
	PON_EMPTY_OLT_HEC_CONFIGURATION_STRUCT (___update_olt_parameters___.hec_configuration)\
    ___update_olt_parameters___.grant_filtering = PON_VALUE_NOT_CHANGED;\
    ___update_olt_parameters___.support_passave_onus = PON_VALUE_NOT_CHANGED;

/* Statistics types definitions */
typedef long double PON_ber_t;  /* BER (Bit Error Rate - 8/10 violations) value  */
typedef long double PON_fer_t;  /* FER (Frame Error Rate - FCS violations) value */

typedef short int  PON_device_id_t;  /* C-type representation of device index */

/* 
** 802.1Q standard definitions - VLAN tagged frames
*/
#define VLAN_TAG_SIZE								4  /* Bytes */

													/* Maximal VLAN-including Ethernet frame length */
													/* ,excluding Preamble, SFD and FCS fields	    */
#define MAX_VLAN_ETHERNET_FRAME_SIZE_STANDARDIZED   (MAX_ETHERNET_FRAME_SIZE_STANDARDIZED + VLAN_TAG_SIZE) 

#define VLAN_MIN_TAG_ID					  0	    /* 802.1Q standard, VLAN tag id a.k.a. VLAN identifier.  */
												/* this value is semi-legal, it indicates priority frames*/
#define VLAN_MAX_TAG_ID				      4094  /* 802.1Q standard, VLAN tag id a.k.a. VLAN identifier   */
#define DEFAULT_VLAN_TAG_ID				  (-1)	/* Defualt VLAN tag id	 					             */
#define VLAN_UNTAGGED_ID				  4095  /* VLAN untagged ID                                      */
#define NULL_VLAN                         VLAN_MIN_TAG_ID
typedef short int  PON_vlan_tag_t;				/* C-type representation of VLAN tag index */

#define VLAN_MIN_TAG_PRIORITY					 0 /* 802.1Q standard, minimum VLAN tag priority   */
#define VLAN_MAX_TAG_PRIORITY				     7 /* 802.1Q standard, maximum VLAN tag priority   */				  
typedef signed char  PON_vlan_priority_t;      /* C-type representation of VLAN tag priority   */

typedef unsigned char  PON_priority_t;             /* priority queue */

typedef unsigned long  PON_timestamp_t;		/* Stated in TQ */

/* 
** 802.1P standard
*/
#define MIN_PRIORITY_QUEUE						 0 /* 802.1P standard */
#define MAX_PRIORITY_QUEUE						 7 /* 802.1P standard */
										
/* 
** FEC mode structure
**
** Parameters:
**
**      downlink_fec                 : Downlink FEC mode is disabled/enabled for the LLID
**      uplink_fec                   : Uplink FEC mode is disabled/enabled for the LLID
**      last_uplink_frame_fec_status : Last uplink frame for the given LLID didn't/do have FEC
*/ 
typedef struct PON_olt_fec_mode_t
{
    bool                downlink_fec;
    bool                uplink_fec;
    bool                last_uplink_frame_fec_status;
} PON_olt_fec_mode_t;

typedef struct PON_downlink_buffer_priority_limits_t 
{
    /* Maximum occupancy of the priority , measured in bytes units 
     * 0 - Max downlink buffer size  (The value is truncated to the lower 512 multiple value)
     */
    unsigned long int priority_limit[MAX_PRIORITY_QUEUE+1];

} PON_downlink_buffer_priority_limits_t;

typedef struct PON_priority_queue_mapping_t  
{
    /* Priority queue to which frame with priority X is inserted  */
    PON_priority_t priority[MAX_PRIORITY_QUEUE+1];

} PON_priority_queue_mapping_t;

/* Alarm threshold configuration parameters 
**
** ber_threshold - Bit Error Rate (8/10 code violations) threshold, 
** Values: DEFAULT_LINK_ALARM_PARAMETER_VALUE  - BER threshold value set to PON_MONITORING_BER_THRESHOLD_DEFAULT_VALUE 
**		   OFF								   - BER monitoring is off
**         0+ - 1 (ten digit precision)		   - BER threshold value
** 
** minimum_error_bytes_threshold - The minimal number of error bytes required for BER alarm
** Values: DEFAULT_LINK_ALARM_PARAMETER_VALUE  - PON_MONITORING_MINIMUM_ERROR_BYTES_THRESHOLD_DEFAULT_VALUE
**         0 - min(PON_RAW_STAT_ONU_BER_ERROR_BYTES_MAX_HARDWARE_VALUE, PON_RAW_STAT_OLT_BER_ERROR_BYTES_MAX_HARDWARE_VALUE)
** 
** fer_threshold - Frame Error Rate (FCS violations) threshold, 
** Values: DEFAULT_LINK_ALARM_PARAMETER_VALUE  - FER threshold value set to PON_MONITORING_FER_THRESHOLD_DEFAULT_VALUE 
**		   OFF								   - FER monitoring is off
**         0+ - 1 (ten digit precision)		   - FER threshold value
** 
** minimum_error_frames_threshold - The minimal number of error frames required for FER alarm,
** range: DEFAULT_LINK_ALARM_PARAMETER_VALUE (results PON_MONITORING_MINIMUM_ERROR_FRAMES_THRESHOLD_DEFAULT_VALUE) or, 
**        0 - min(PON_RAW_STAT_ONU_FER_RECEIVED_ERROR_MAX_HARDWARE_VALUE, PON_RAW_STAT_TOTAL_FRAMES_RECEIVED_ERROR_MAX_HARDWARE_VALUE) 
** 
** llid_mismatch_threshold - The minimal number of mismatched frames required for LLID mismatch alarm (PON_ALARM_LLID_MISMATCH)
** Values: DEFAULT_LINK_ALARM_PARAMETER_VALUE  - PON_MONITORING_LLID_MISMATCH_THRESHOLD_DEFAULT_VALUE
**		   OFF								   - LLID mismatch monitoring is off
**		   >0								   - Threshold value, corresponds to aMismatchDroppedFrames 
*/

typedef struct PAS_link_alarm_params_t
{
	PON_ber_t  ber_threshold;					
	long	   minimum_error_bytes_threshold;	
	PON_fer_t  fer_threshold;					
	long	   minimum_error_frames_threshold;	
	long	   llid_mismatch_threshold;
} PAS_link_alarm_params_t;

#define PAS_OLT_AUTOMATIC_DETECTION  PON_ALL_ACTIVE_OLTS

#ifndef PON_AUTHENTICATION_SEQUENCE_SIZE
#define PON_AUTHENTICATION_SEQUENCE_SIZE 16
/* Authentication key */
typedef unsigned char PON_authentication_sequence_t[PON_AUTHENTICATION_SEQUENCE_SIZE];
#endif


/* 
** DBA report format enum 
**
*/
typedef enum
{
	PON_DBA_STANDARD_REPORT,	        /* 802.3ah (non threshold) REPORT	*/
	PON_DBA_THRESHOLD_REPORT			/* threshold REPORT	*/	         
							
} PON_DBA_report_format_t;


/* == 1 == the start of data struct " PAS_pon_initialization_parameters_t " ====== */

/* PON initialization struct 
**
** automatic_authorization_policy - ENABLE /DISABLE PAS-SOFT auto-authorization for each registrating ONU
**
*/

/* PAS5001
typedef struct
{
	bool		   TBD; 
	bool		   automatic_authorization_policy;
} PAS_pon_initialization_parameters_t;
*/
typedef struct PAS_pon_initialization_parameters_t
{
	bool		   automatic_authorization_policy;
} PAS_pon_initialization_parameters_t;

/* == 1 == the end of data struct " PAS_pon_initialization_parameters_t " ====== */


/* == 2 == the start of date struct " PON_olt_init_parameters_t " ====== */
typedef enum
{
	PON_OLT_OPTICS_CONFIGURATION_SOURCE_EEPROM = 0, /* OLT EEPROM device */
	PON_OLT_OPTICS_CONFIGURATION_SOURCE_HOST		/* OLT Host software */
} PON_olt_optics_configuration_source_t;
/*  V5.3.0 以前版本
typedef enum
{
	PON_POLARITY_ACTIVE_LOW,
	PON_POLARITY_ACTIVE_HIGH
} PON_polarity_t;
*/
typedef enum PON_polarity_t
{
    PON_POLARITY_NO_CHANGE = -1,
	PON_POLARITY_ACTIVE_LOW,
	PON_POLARITY_ACTIVE_HIGH
} PON_polarity_t;

/* 
** AGC reset configurations
*/
typedef struct PON_agc_reset_configuration_t
{
	short int		gate_offset;		/* Activation offset before normal grant CDR reset activation, */
										/* measured in 8 nanoseconds units. Values:					   */
										/* PON_OPTICS_OFFSET_MIN_VALUE - PON_OPTICS_OFFSET_MAX_VALUE   */
	short int		discovery_offset;	/* Activation offset before discovery CDR reset activation,    */
										/* measured in 8 nanoseconds units. Values:					   */
										/* PON_OPTICS_OFFSET_MIN_VALUE - PON_OPTICS_OFFSET_MAX_VALUE   */
	short int		duration;			/* Pulse duration, measured in 8 nanoseconds units.			   */
										/* Values: 0       - AGC Lock to reference mode (level mode),  */
										/*		   (1 - 7) - Pulse duration							   */
	PON_polarity_t  polarity;			/* Pulse polarity. Values: enum values						   */
} PON_agc_reset_configuration_t;


/* 
** CDR reset configurations
*/
typedef struct PON_cdr_reset_configuration_t
{
	short int		gate_offset;	  /* Activation offset before start of normal grant, measured in 8 */
									  /* nanoseconds units. Values:									   */
									  /* PON_OPTICS_OFFSET_MIN_VALUE - PON_OPTICS_OFFSET_MAX_VALUE     */
	short int		discovery_offset; /* Activation offset before start of discovery window, measured  */
									  /* in 8 nanoseconds units. Values: 							   */
									  /* PON_OPTICS_OFFSET_MIN_VALUE - PON_OPTICS_OFFSET_MAX_VALUE     */
	short int		duration;		  /* Pulse duration, measured in 8 nanoseconds units.			   */
									  /* Values: 0       - CDR Lock to reference mode (level mode),	   */
									  /*		 (1 - 7) - Pulse duration							   */
	PON_polarity_t  polarity;		  /* Pulse polarity. Values: enum values						   */
} PON_cdr_reset_configuration_t;


/* 
** End of grant reset configurations
*/
typedef struct PON_end_of_grant_reset_configuration_t
{
	short int		offset;			 /* Activation offset, measured in 8 nanoseconds units.	Values:	   */
									 /* PON_OPTICS_OFFSET_MIN_VALUE - PON_OPTICS_OFFSET_MAX_VALUE      */
	short int		duration;	     /* Pulse duration, measured in 8 nanoseconds units.               */
                                     /* PAS5001 Values: 0 - 7 , PAS5201 0-63                           */
	PON_polarity_t  polarity;		 /* Pulse polarity. Values: enum values							   */
} PON_end_of_grant_reset_configuration_t;

/*
** OLT optics clocks and lines polarity configuration
*/

#if 0 
/*  PAS5001 */
typedef struct
{
	PON_polarity_t	pon_port_link_indication_polarity;  /* Polarity of the PON port link indication clk*/
	PON_polarity_t	cni_port_link_indication_polarity;  /* Polarity of the CNI (System) port link	   */
														/* indication clock							   */
} PON_olt_optics_polarity_configuration_t;

#endif 

#define PON_OPTICS_DEAD_ZONE_MIN_VALUE 0x0  /* Optics dead zone range */
#define PON_OPTICS_DEAD_ZONE_MAX_VALUE 0xF

typedef struct PON_olt_optics_polarity_configuration_t
{
	PON_polarity_t	pon_port_link_indication_polarity;  /* Polarity of the PON port link indication clk */
	PON_polarity_t	cni_port_link_indication_polarity;  /* Polarity of the CNI (System) port link	    */
														/* indication clock							    */
    PON_polarity_t  pon_tbc_polarity;                   /* Polarity of output TBC clock for the TBI bus */ 
} PON_olt_optics_polarity_configuration_t;

/* OLT recovery mode */
typedef enum
{
	PON_OLT_MODE_NOT_CONNECTED,					/* OLT is not connected to the Host	/ Host recovery  	*/
												/* not supported										*/
	PON_OLT_MODE_CONNECTED_AND_NOT_CONFIGURED,	/* OLT is connected to the Host but has not been		*/
												/* initialized by the Host								*/
	PON_OLT_MODE_CONFIGURED_AND_ACTIVATED,		/* OLT is connected to the Host and has been initialized*/
												/* by the Host, this is the only mode which enables 	*/
												/* recovery from the OLT								*/
	PON_OLT_MODE_LAST_MODE						/* Indication for an invalid OLT recovery mode		 	*/
} PON_olt_mode_t;


#if 0
/* PAS5001 */
typedef struct
{
	PON_olt_optics_configuration_source_t    configuration_source;
	short int 	agc_lock_time;
	PON_agc_reset_configuration_t		 agc_reset_configuration;
	short int 	cdr_lock_time;
	PON_cdr_reset_configuration_t		 cdr_reset_configuration;
	PON_end_of_grant_reset_configuration_t   end_of_grant_reset_configuration;
	bool	discovery_re_locking_enable;
	PON_polarity_t	 discovery_laser_rx_loss_polarity;
	PON_polarity_t	 pon_tx_disable_line_polarity;
	unsigned short int	optics_dead_zone;
	bool	use_optics_signal_loss;
	PON_olt_optics_polarity_configuration_t  polarity_configuration;
} PON_olt_optics_configuration_t;
#endif

#define end_of_grant_reset_configuration cdr_end_of_grant_reset_configuration

typedef struct PON_olt_optics_configuration_t
{
	PON_olt_optics_configuration_source_t    configuration_source;
	short int								 agc_lock_time;
	PON_agc_reset_configuration_t			 agc_reset_configuration;
	short int								 cdr_lock_time;
	PON_cdr_reset_configuration_t			 cdr_reset_configuration;
	PON_end_of_grant_reset_configuration_t   cdr_end_of_grant_reset_configuration;
    PON_end_of_grant_reset_configuration_t   optics_end_of_grant_reset_configuration;
	bool									 discovery_re_locking_enable;
	PON_polarity_t							 discovery_laser_rx_loss_polarity;
	PON_polarity_t							 pon_tx_disable_line_polarity;
	unsigned short int					     optics_dead_zone;
	bool									 use_optics_signal_loss;
	PON_olt_optics_polarity_configuration_t  polarity_configuration;
	unsigned short int			             discovery_laser_on_time;
	unsigned short int			             discovery_laser_off_time;

} PON_olt_optics_configuration_t;


typedef struct PON_olt_igmp_configuration_t
{
	short int  enable_igmp_snooping; /* Enable / disable OLT IGMP snooping algorithm.				   */
									 /* Values: ENABLE, DISABLE, PON_VALUE_NOT_CHANGED (when updated   */
									 /* during runtime only)										   */
	short int  igmp_timeout;		 /* Remove a group address from the ONU IGMP filter table		   */
									 /* according to membership message timeout, stated in seconds.	   */
									 /* Valid only if enable_igmp_snooping field has ENABLE value.	   */
									 /* Range: PON_NO_IGMP_TIMEOUT, 1 - PON_MAX_IGMP_TIMEOUT,		   */
									 /* PON_VALUE_NOT_CHANGED										   */
									 /* (when updated during runtime only)							   */
} PON_olt_igmp_configuration_t;

/* OLT global VLAN definitions */
typedef struct PON_olt_vlan_configuration_t
{
	short int	vlan_exchange_downlink_tag_prefix; /* VLAN tag prefix for VLAN exchange handling of */
												   /* downlink traffic. Values: MINUCHAR - MAXUCHAR,*/
												   /* PON_VALUE_NOT_CHANGED (for configuration      */
												   /* update only)									*/
	long int	nested_mode_vlan_type;			   /* Nested VLAN type to be added to stacked-		*/
												   /* configured LLIDs with PON_NESTED_MODE_VLAN_   */
												   /* TYPE_OTHER_VALUE nested_mode_vlan_type field.	*/
												   /* Values: MINUWORD - MAXUWORD,				    */
												   /* PON_VALUE_NOT_CHANGED (for configuration		*/
												   /* update only)									*/
} PON_olt_vlan_configuration_t;

#define PON_EMPTY_OLT_VLAN_CONFIGURATION_STRUCT(__olt_vlan_configuration_struct__) \
	__olt_vlan_configuration_struct__.vlan_exchange_downlink_tag_prefix = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_configuration_struct__.nested_mode_vlan_type			    = PON_VALUE_NOT_CHANGED;\


typedef enum
{
	PON_VLAN_HANDLING_MODE_NOT_CHANGED,	/* LLID original VLAN tags are passed both in downlink and */
										/* in uplink											   */
	PON_VLAN_HANDLING_MODE_EXCHANGE,	/* LLID original VLAN tags are exchanged				   */
	PON_VLAN_HANDLING_MODE_STACK		/* LLID original VLAN tags are stacked					   */
} PON_vlan_handling_mode_t;

/* VLAN exchange handling mode definitions */
#define PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE1_MIN_VALUE 0
#define PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE1_MAX_VALUE 3
#define PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE2_MIN_VALUE 8
#define PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE2_MAX_VALUE 15

#define PON_VLAN_EXCHANGE_TAG_SUFFIX_V3_MIN_VALUE 0
#define PON_VLAN_EXCHANGE_TAG_SUFFIX_V3_MAX_VALUE 15

 
typedef struct PON_vlan_exchange_configuration_t
{
	PON_vlan_tag_t	uplink_tag;		/* VLAN tag id for uplink traffic.								   */
									/* Range: VLAN_MIN_TAG_ID - VLAN_MAX_TAG_ID						   */
									/* Downlink tag is a concatenation of							   */
									/* vlan_exchange_downlink_tag_prefix and downlink_tag_suffix	   */
									/* vlan_exchange_downlink_tag_prefix is part of					   */
									/* PON_olt_vlan_configuration_t, set at OLT initialization		   */
	short int	downlink_tag_suffix;/* VLAN tag suffix id used as downlink exchanged tag suffix. Range:*/
/*  PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE1_MIN_VALUE - PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE1_MAX_VALUE,	   */
/*	PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE2_MIN_VALUE - PON_VLAN_EXCHANGE_TAG_SUFFIX_RANGE2_MAX_VALUE	   */
} PON_vlan_exchange_configuration_t;


typedef enum
{
	PON_NESTED_MODE_VLAN_TYPE_8100_VALUE  = 0x0, /* Nested VLAN type is 0x8100						  */
	PON_NESTED_MODE_VLAN_TYPE_9100_VALUE  = 0x1, /* Nested VLAN type is 0x9100						  */
	PON_NESTED_MODE_VLAN_TYPE_OTHER_VALUE		 /* Nested VLAN type determined by OLT initialization */
												 /* parameter value									  */
} PON_llid_nested_mode_vlan_type_t;


typedef struct PON_vlan_stack_configuration_t
{
  PON_vlan_tag_t  stacked_tag;		  /* Stacked VLAN tag id for both uplink and downlink traffic.	   */
									  /* Range: VLAN_MIN_TAG_ID - VLAN_MAX_TAG_ID					   */
  bool			  nested_mode_support;/* Nested VLAN mode support on uplink tag adding.				   */
									  /* Nested mode - Value: TRUE, Tag adding to every frame, tagged  */
									  /* or untagged. VLAN tag is removed from every downlink tagged   */
									  /* frames.													   */
									  /* Service mode - Value: FALSE, Tag adding to untagged frames    */
									  /* only, VLAN tag is removed from downlink tagged frames with tag*/
									  /* equal to stacked_tag										   */
  PON_llid_nested_mode_vlan_type_t    /* Nested VLAN type, valid only if nested_mode_support is TRUE.  */
				nested_mode_vlan_type;/*Values: enum values										       */
} PON_vlan_stack_configuration_t;	

/*
** Definitions of binary files which can be loaded to the OLT 
*/
typedef enum
{
	PON_OLT_BINARY_FIRMWARE = 1, /* OLT firmware				   */
	PON_OLT_BINARY_DBA_DLL	= 2, /* DBA algorithm formatted as DLL */
	PON_ONU_BINARY_FIRMWARE = 3  /* ONU firmware				   */
} PON_binary_type_t;

typedef enum
{
	PON_BINARY_SOURCE_FLASH,   /* Binary found on the flash memory of the device	*/
	PON_BINARY_SOURCE_FILE,    /* Binary found on file in the host file system		*/
	PON_BINARY_SOURCE_MEMORY   /* Binary found in a certain location in the memory	*/
} PON_binary_source_t;

/* Binary (external executable unit) identification definitions */ 

/* Binary User ID size */
#define PON_BINARY_USER_ID_SIZE			16  /* Bytes */

/* Binary User ID */
typedef unsigned char PON_binary_user_id_t[PON_BINARY_USER_ID_SIZE]; /* Values: 128-bit value */

/* Binary encryption key size */
#define PON_BINARY_ENCRYPTION_KEY_SIZE	32  /* Bytes */

/* Binary encryption key */
typedef unsigned char PON_binary_encryption_key_t[PON_BINARY_ENCRYPTION_KEY_SIZE]; /* Values: 256-bit value */

/* Macro for zeroing a not-reuiqered identification of binary structure
**
** usage:  PON_binary_t   my_binary;
**
**		   PON_IDENTIFICATION_NOT_REQUIRED(my_binary.identification)
*/
#define PON_IDENTIFICATION_NOT_REQUIRED(__binary_identification_parameter__) {int __tmp__;\
			for(__tmp__=0; __tmp__<PON_BINARY_USER_ID_SIZE; __binary_identification_parameter__.user_id[__tmp__]=0x0,__tmp__++);\
			for(__tmp__=0; __tmp__<PON_BINARY_ENCRYPTION_KEY_SIZE; __binary_identification_parameter__.key[__tmp__]=0x0,__tmp__++);}

/* Binary identification structure 
**
** Optional identification structure which is relevant only for some of the binaries (consult specific 
** binary documentation for details).
** 
** NOTE: if the loaded binary doesn't require identification, use the PON_IDENTIFICATION_NOT_REQUIRED macro 
*/
typedef struct PON_binary_identification_t
{
	PON_binary_user_id_t		  user_id;	/* User identification. Values: 128-bit value. Supplied    */
											/* with the binary file									   */
	PON_binary_encryption_key_t   key;      /* Activation key (password). Values: 256-bit activation   */
											/* key. Supplied with the binary file					   */
} PON_binary_identification_t;

/* Definition of binary - PAS-SOFT external executable unit */
typedef struct PON_binary_t
{
	PON_binary_type_t			  type;		/* Binary type, values: enumerated values				   */
	PON_binary_source_t			  source;	/* Type of device where the binary currently resides,	   */
											/* values: enumerated values							   */
	void					     *location;	/* Location of the binary, different type for different    */
											/* sources:												   */
											/* MEMORY source - Pointer to the beginning of the binary  */
											/* FILE source   - The name of the file containing the	   */
											/* binary												   */
											/* FLASH		 - Not required							   */
	long int					  size;		/* Size of the binary, stated in bytes. Required only for  */
											/* MEMORY source										   */
	PON_binary_identification_t   identification;	/* Identification of the binary user, see		   */
											/* PON_binary_identification_t definition for details	   */
} PON_binary_t;

/* Indexed binary
**
** Indexed binary in an PAS-SOFT external executable unit which has an additional
** system index field. The index field marks each separate binary model according
** to each System / System model. The index field may change for each System run.
** This field contains the PON_PROVISIONING_MODE_EXTERNAL_DBA values for DBA DLLs
**
*/
typedef struct
{
	PON_binary_t		 binary;
	unsigned short int	 index;
} PON_indexed_binary_t;


/* Upstream default priority insertion			*/
#define PON_MIN_UPSTREAM_DEFAULT_PRIORITY	0
#define PON_MAX_UPSTREAM_DEFAULT_PRIORITY	3
/* OLT Initialization parameters struct ,used with add_olt_v4
**
**		optics_configuration			: OLT Optics configuration. See PON_olt_optics_configuration_t
**										  for details.
**		discovery_laser_on_time			: Laser on time of the ONUs during the discovery process
**										  ,measured in TQ. 
**										  range: OFF - no constraint, 1 - (2^16-1) - constraint value
**		discovery_laser_off_time		: Laser off time of the ONUs during the discovery process
**										  , measured in TQ. 
**										  range: OFF - no constraint, 1 - (2^16-1) - constraint value
**		igmp_configuration				: OLT IGMP configuration, see PON_olt_igmp_configuration_t for
**										  details
**      vlan_configuration				: OLT global VLAN configuration, see PON_olt_vlan_configuration_t
**										  for details
**		multiple_copy_broadcast_enable	: Enable / Disable multiple LLID VLANs. Values: ENABLE / DISABLE
**		discard_unlearned_addresses		: Discard / forward any frame received from unlearned source 
**										  address of an ONU when ONU entries limit inside address table 
**										  is reached. Values: TRUE / FALSE.
**		
**		firmware_image					: OLT firmware image file characteristics. See PON_binary_t for 
**										  details. Structure Values:
**										  type - PON_OLT_BINARY_FIRMWARE
**										  source - All supported options
**										  location - According to PON_binary_t guidelines 
**										  size - According to PON_binary_t guidelines 
**										  identification - Not required in current versions
**										  
*/
typedef struct PON_olt_init_parameters_t
{
	PON_olt_optics_configuration_t   optics_configuration;
	unsigned short int			     discovery_laser_on_time;
	unsigned short int			     discovery_laser_off_time;
	PON_olt_igmp_configuration_t	 igmp_configuration;
	PON_olt_vlan_configuration_t	 vlan_configuration;
	bool							 multiple_copy_broadcast_enable;
	bool							 discard_unlearned_addresses;
	PON_binary_t					 firmware_image;
} PON_olt_init_parameters_t;  


/* OLT Initialization parameters struct 
**
**		olt_mac_address				    : OLT MAC address
**										  If MAC address is to be set by caller - valid unicast MAC 
**											address value
**										  If OLT default MAC address is used - OLT_DEFAULT_MAC_ADDRESS
**											(0xFF-0xFF-0xFF-0xFF-0xFF-0xFF - all 1s) the MAC address retrieve from the EEPROM
**		optics_configuration			: OLT Optics configuration. See PON_olt_optics_configuration_t
**										  for details.
**		external_downlink_buffer_size   : The size of the external downlink buffer size, if there is no external 
**                                        downlink buffer use PON_EXTERNAL_DOWNLINK_BUFFER_0MB
**      support_passave_onus            : Supporting or not Passave ONUs' extended capabilities 
**		firmware_image					: OLT firmware image file characteristics. See PON_binary_t for 
**										  details. Structure Values:
**										  type - PON_OLT_BINARY_FIRMWARE
**										  source - All supported options
**										  location - According to PON_binary_t guidelines 
**										  size - According to PON_binary_t guidelines 
**										  identification - Not required in current versions
**      ram_test                        : Perform or not RAM test before loading the firmware to the RAM 
**                                        (available only when 'Firmware image Source' is not from flash) 
**										  
*/
typedef enum  
{
    PON_EXTERNAL_DOWNLINK_BUFFER_0MB  = 0,
    PON_EXTERNAL_DOWNLINK_BUFFER_2MB  = 2,
    PON_EXTERNAL_DOWNLINK_BUFFER_4MB  = 4,
    PON_EXTERNAL_DOWNLINK_BUFFER_8MB  = 8,
    PON_EXTERNAL_DOWNLINK_BUFFER_16MB = 16,
    PON_EXTERNAL_DOWNLINK_BUFFER_32MB = 32
} PON_external_downlink_buffer_size_t;

typedef struct PON_olt_initialization_parameters_t
{
	mac_address_t					    olt_mac_address;
	PON_olt_optics_configuration_t      optics_configuration;
	PON_external_downlink_buffer_size_t external_downlink_buffer_size;
	bool                                support_passave_onus;
	PON_binary_t					    firmware_image;
	bool                                ram_test;
} PON_olt_initialization_parameters_t;


/* Empty and set to defaults add olt parameters */
#define PON_EMPTY_ADD_OLT_PARAMETERS_STRUCT(__add_olt_parameters_struct__) \
    memset(&__add_olt_parameters_struct__,0,sizeof(PON_olt_initialization_parameters_t)); 

/* Add OLT result parameters struct 
**
**      olt_mac_address : The input parameter changes its value to the OLT's actual MAC     
**                        address, whether set explicitly by the input parameter or         
**                        taken from the OLT default (EEPROM) MAC address                   
*/
typedef struct PON_add_olt_result_t
{
	mac_address_t  olt_mac_address;
} PON_add_olt_result_t;



/*== 2 == the end of date struct " PON_olt_init_parameters_t "  ====== */





/* == 3 == the start of data struct " PON_olt_cni_port_mac_configuration_t " ====== */

/* Master/Slave values */
typedef enum 
{
	PON_SLAVE,
	PON_MASTER,
	PON_MASTER_SLAVE_UNKNOWN
} PON_master_slave_t;

extern char *PON_master_slave_s[];

/* PAUSE set threshold: Port activates PAUSE frames when queue is filled with more than specified */
/* threshold. The Enum specifies the only valid values.											  */
/* Hardware default value: PON_PAUSE_SET_THRESHOLD_40K											  */
typedef enum
{
	PON_PAUSE_SET_THRESHOLD_KEEP_CURRENT = PON_VALUE_NOT_CHANGED,	/* Keep current threshold (whether */
																    /* hardware default or previously  */
																	/* modified)					   */
	PON_PAUSE_SET_THRESHOLD_24K	 = 24,	/* 24KBytes */
	PON_PAUSE_SET_THRESHOLD_26K	 = 26,	/* 26KBytes */
	PON_PAUSE_SET_THRESHOLD_32K	 = 32,	/* 32KBytes */
	PON_PAUSE_SET_THRESHOLD_34K	 = 34,	/* 34KBytes */
	PON_PAUSE_SET_THRESHOLD_40K	 = 40,	/* 40KBytes */                                                            
    PON_PAUSE_SET_THRESHOLD_42K	 = 42,	/* 42KBytes this is not a valid value (HW problem) and         */
                                        /* will transform to 40 in setting for backward compatibility  */                 
    /* Values that supported only for PAS5201 or newer */
    PON_PAUSE_SET_THRESHOLD_48K	 = 48,	/* 48KBytes */                                                            
    PON_PAUSE_SET_THRESHOLD_56K	 = 56,	/* 56KBytes */                                                            
    PON_PAUSE_SET_THRESHOLD_60K	 = 60 	/* 60KBytes */                                                            
} PON_pause_set_threshold_t;

/* PAUSE release threshold: Port stops PAUSE frames sending only after queue is filled with less than*/
/* specified threshold. The Enum specifies the only valid values. 									 */
/* Hardware default value: PON_PAUSE_RELEASE_THRESHOLD_32K											 */
typedef enum
{
	PON_PAUSE_RELEASE_THRESHOLD_KEEP_CURRENT = PON_VALUE_NOT_CHANGED,/* Keep current threshold (whether*/
																	 /* hardware default or previously */
																	 /* modified)					   */
	PON_PAUSE_RELEASE_THRESHOLD_16K = 16, /* 16KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_20K = 20, /* 20KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_24K = 24, /* 24KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_28K = 28, /* 28KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_32K = 32, /* 32KBytes */
    PON_PAUSE_RELEASE_THRESHOLD_36K = 36, /* 36KBytes this is not a valid value (HW problem) and       */
                                          /* will transform to 32 in setting for backward compatibility*/
    /* Values that supported only for PAS5201 or newer */
	PON_PAUSE_RELEASE_THRESHOLD_40K = 40, /* 40KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_48K = 48, /* 48KBytes */
	PON_PAUSE_RELEASE_THRESHOLD_52K = 52  /* 52KBytes */

} PON_pause_release_threshold_t;

#define PON_IS_PAUSE_SET_THRESHOLD_VALUE_LEGAL(__set_value__)\
	   ((__set_value__ == PON_PAUSE_SET_THRESHOLD_KEEP_CURRENT) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_24K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_26K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_32K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_34K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_40K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_42K))?(bool)TRUE:(bool)FALSE 

#define PON_IS_PAUSE_SET_THRESHOLD_VALUE_LEGAL_V5(__set_value__)\
	   ((__set_value__ == PON_PAUSE_SET_THRESHOLD_KEEP_CURRENT) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_24K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_26K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_32K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_34K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_40K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_42K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_48K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_56K) ||\
		(__set_value__ == PON_PAUSE_SET_THRESHOLD_60K))?(bool)TRUE:(bool)FALSE 


#define PON_IS_PAUSE_RELEASE_THRESHOLD_VALUE_LEGAL(__release_value__)\
	   ((__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_KEEP_CURRENT) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_16K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_20K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_24K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_28K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_32K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_36K))?(bool)TRUE:(bool)FALSE 

#define PON_IS_PAUSE_RELEASE_THRESHOLD_VALUE_LEGAL_V5(__release_value__)\
	   ((__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_KEEP_CURRENT) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_16K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_20K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_24K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_28K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_32K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_36K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_40K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_48K) ||\
		(__release_value__ == PON_PAUSE_RELEASE_THRESHOLD_52K))?(bool)TRUE:(bool)FALSE 

#ifdef PAS_SOFT_VERSION_V5_3_11
/*
 * For PAS5001 the values are as at the enums PON_pause_set_threshold_t,PON_pause_release_threshold_t
 * For PAS5201 and newer the values are depends if working with external downlink buffer or without
 * Without downlink buffer see PON_pause_set_threshold_t, PON_pause_release_threshold_t
 * with downlink buffer see PON_pause_set_release_threshold_percents_t
 */
typedef struct PON_port_pause_thresholds_t
{
	long      pause_set_threshold; 
	long      pause_release_threshold; 
} PON_port_pause_thresholds_t;

#else
		
/*
 * For PAS5001 the values are as at the enums PON_pause_set_threshold_t,PON_pause_release_threshold_t
 * For PAS5201 and newer the values are depends if working with external downlink buffer or without
 * Without downlink buffer see PON_pause_set_threshold_t, PON_pause_release_threshold_t
 * with downlink buffer see PON_pause_set_release_threshold_percents_t
 */
typedef struct
{
	PON_pause_set_threshold_t      pause_set_threshold;
	PON_pause_release_threshold_t  pause_release_threshold;
} PON_port_pause_thresholds_t;
#endif

/* Minimum difference allowed between set PAUSE and release PAUSE threshlods values.		*/
/* The difference between the PAUSE thresholds composes of pause frame delay, pause frame	*/
/* processing delay, receive path, queue lock ahead and other hardware overheads			*/
#define PON_MIN_PON_PAUSE_THRESHOLDS_DIFFERANCE 6 /* KBytes */

/* The differance with external downlink buffer */
#define PON_MIN_PON_PAUSE_THRESHOLDS_DIFFERANCE_WITH_DLB 8 /* KBytes */

/* PAUSE set release threshold percents, used when working with external downlink buffer */
typedef enum
{
	PON_PAUSE_SET_RELEASE_THRESHOLD_KEEP_CURRENT_PERCENTS = PON_VALUE_NOT_CHANGED,/* Keep current threshold (whether*/
																	              /* hardware default or previously */
																	              /* modified)					   */
	PON_PAUSE_SET_RELEASE_THRESHOLD_85_PERCENTS = 85, /* 85 percents of the external downlink buffer */
	PON_PAUSE_SET_RELEASE_THRESHOLD_90_PERCENTS = 90, /* 90 percents of the external downlink buffer */
	PON_PAUSE_SET_RELEASE_THRESHOLD_95_PERCENTS = 95  /* 95 percents of the external downlink buffer */

} PON_pause_set_release_threshold_percents_t;


#define PON_IS_PAUSE_RELEASE_THRESHOLD_PERCENTS_VALUE_LEGAL(__release_value__)\
	   ((__release_value__ == PON_PAUSE_SET_RELEASE_THRESHOLD_85_PERCENTS) ||\
		(__release_value__ == PON_PAUSE_SET_RELEASE_THRESHOLD_90_PERCENTS) ||\
		(__release_value__ == PON_PAUSE_SET_RELEASE_THRESHOLD_95_PERCENTS))?(bool)TRUE:(bool)FALSE 
		

typedef struct PON_olt_cni_port_advertisement_t
{
	bool				 _1000base_tx_half_duplex; /* PHY MDIO R9.8 								   */
												   /* TRUE - advertised, FALSE - Not advertised	       */
	bool				 _1000base_tx_full_duplex; /* PHY MDIO R9.9 							   	   */
												   /* TRUE - advertised, FALSE - Not advertised		   */
	PON_master_slave_t	 preferable_port_type;	   /* PHY MDIO R9.10, values:						   */
												   /* PON_SLAVE  - 1000BASE-T multiport not advertised */
												   /*				(prefer single port device)		   */
												   /* PON_MASTER - 1000BASE-T multiport advertise 	   */
												   /*				(prefer multi port device)		   */
	bool				 pause;					   /* PHY MDIO R4.10								   */
												   /* ENABLE  - MAC pause implemented,				   */
												   /* DISABLE - MAC pause not implemented			   */
	bool				 asymmetric_pause;		   /* PHY MDIO R4.11								   */
												   /* ENABLE - asymmetric pause implemented,		   */
												   /* DISABLE - no asymmetric pause implemented		   */
} PON_olt_cni_port_advertisement_t;

typedef struct PON_olt_cni_port_mac_configuration_t
{
	PON_master_slave_t				 master;			/* Master / slave mode in port,				*/
														/* PON_MASTER - regular MAC-PHY mode,		*/
														/* PON_SLAVE - MAC disguise as PHY			*/
	bool							  pause;			/* ENABLE  - MAC Pause transmission enable,	*/
														/* DISABLE - MAC Pause transmission disable	*/
	unsigned char					  mdio_phy_address;	/* MDIO PHY address, values:				*/
														/* MDIO_MIN_PHY_ADDRESS - 					*/
														/* MDIO_MAX_PHY_ADDRESS						*/
	bool							  auto_negotiation;	/* ENABLE / DISABLE auto-negotiation in port*/
														/* When disabled, one (and only one) of the */
														/* modes in the relevant					*/
														/* advertisement_details structure has to be*/
														/* set to TRUE								*/
	PON_port_pause_thresholds_t		  pause_thresholds;	/* CNI port PAUSE frames activation 		*/
														/* thresholds configuration, OLT CNI port	*/
														/* queue size is 48KBytes. Not relevant		*/
														/* in case of Host MII port configuration.	*/
	bool							  advertise;		/* ENABLE / DISABLE advertise in port		*/
	PON_olt_cni_port_advertisement_t  advertisement_details;  /* CNI port specific advertisement	*/
														/*configuration.Not relevant in case of Host*/
														/* MII port configuration				    */
} PON_olt_cni_port_mac_configuration_t;

/* == 3 == the end of data struct " PON_olt_cni_port_mac_configuration_t " ====== */



/* == 4 == the start of data struct " PON_olt_update_parameters_t " ====== */

typedef long  PON_rtt_t;  /* C-type representation of RTT measurement (round trip time from    */
						  /* OLT to ONU and back, measured in TQ) */


/* Allowed LLID entries in address table limitations */
typedef enum
{
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_NO_CHANGE = -1,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_0    = 0,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_1    = 1,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_2    = 2,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_4    = 4,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_8    = 8,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_16   = 16,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_32   = 32,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_64   = 64,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_128  = 128,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_256  = 256,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_512  = 512,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022 = 1022,
	PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192 = PAS_ADDRESS_TABLE_SIZE 
} PON_address_table_entries_llid_limitation_t;

#define PON_ADDRESS_TABLE_ENTRY_MIN_LIMITATION (0)
#define PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION (PAS_ADDRESS_TABLE_SIZE-1)

#define IS_ADDRESS_TABLE_ENTRY_LIMITATION_LEGAL(__limitation__)      \
	( ((__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_0)    ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_1)    ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_2)    ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_4)    ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_8)    ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_16)   ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_32)   ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_64)   ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_128)  ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_256)  ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_512)  ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022) ||\
	   (__limitation__ == PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192))  \
	 ?(bool)TRUE:(bool)FALSE)

/* Uplink / downlink network directions */
typedef enum
{  
	PON_DIRECTION_UPLINK,
	PON_DIRECTION_DOWNLINK,
	PON_DIRECTION_UPLINK_AND_DOWNLINK,
    PON_DIRECTION_NO_DIRECTION
} PON_pon_network_traffic_direction_t;

/* Address table config structure
** Parameters:
**      aging_timer               : Address table aging mechanism 
**                                  range: PON_MIN_ADDRESS_TABLE_AGING_TIMER..PON_MAX_ADDRESS_TABLE_AGING_TIMER 
**      removed_when_aged         : Disable : MAC addresses will not removed from OLT address table when it age
**                                  but only when the address table is full                   
**                                  Enable : MAC addresses will be removed from OLT 
**                                  address table after 2 - 3 multiples of Aging timer
**      allow_learning            : address table allow learning from ports
**      discard_llid_unlearned_sa : forward or discard frames from source address that not learned
**      discard_unknown_da        : from where discard unknown frames 
*/
typedef struct PON_address_table_config_t 
{
   bool                                     removed_when_aged;
   long                                     aging_timer;
   PON_pon_network_traffic_direction_t      allow_learning;
   bool                                     discard_llid_unlearned_sa;
   PON_pon_network_traffic_direction_t      discard_unknown_da;

} PON_address_table_config_t;

/* 
** ARP configuration
*/
typedef struct PON_arp_filtering_configuration_t
{
	short int  from_pon_to_firmware;   /* Enable / Disable ARP frames from PON port to OLT firmware    */
								   	   /* (and PAS-SOFT). values:PON_VALUE_NOT_CHANGED, DISABLE, ENABLE*/
	short int  from_cni_to_firmware;   /* Enable / Disable ARP frames from CNI port to OLT firmware    */
									   /* (and PAS-SOFT). values:PON_VALUE_NOT_CHANGED, DISABLE, ENABLE*/
	short int  from_pon_to_cni;		   /* Enable / Disable ARP frames from PON port to CNI port,	   */
									   /* values: PON_VALUE_NOT_CHANGED, DISABLE, ENABLE			   */
	short int  from_cni_to_pon;		   /* Enable / Disable ARP frames from CNI port to PON port,	   */
									   /* values: PON_VALUE_NOT_CHANGED, DISABLE, ENABLE			   */
} PON_arp_filtering_configuration_t;

/*
**HEC configuration (Applies only for PAS5001-N M3)
*/
#if 0   /* V5.3.0 以前版本 */
typedef enum
{
	PON_TX_HEC_LEGACY_MODE  = 0x0,		/* Tx HEC generation as in PAS5001-N					 */
	PON_TX_HEC_802_AH_MODE  = 0x1		/* Tx HEC generation according  to IEEE802.3ah stabdard  */
} PON_tx_hec_config_t;

typedef enum
{
	PON_RX_HEC_NO_HEC_MODE		 = 0x0,		/* No Rx HEC checking								*/	
	PON_RX_HEC_LEGACY_MODE	     = 0x1,		/* Rx HEC check as in PAS5001-N						*/
	PON_RX_HEC_802_AH_MODE		 = 0x2,		/* Rx HEC check according  to IEEE802.3ah stabdard	*/
	PON_RX_HEX_802_LEFACY_MODE   = 0x3		/* Rx HEC check according to both method		    */
} PON_rx_hec_config_t;
#endif

typedef enum
{
    PON_TX_HEC_NO_CHANGE    = -1 ,
	PON_TX_HEC_LEGACY_MODE  = 0x0,		/* Tx HEC generation as in PAS5001-N					 */
	PON_TX_HEC_802_AH_MODE  = 0x1		/* Tx HEC generation according  to IEEE802.3ah stabdard  */
} PON_tx_hec_config_t;

typedef enum
{
    PON_RX_HEC_NO_CHANGE         = -1,
	PON_RX_HEC_NO_HEC_MODE		 = 0x0,		/* No Rx HEC checking								*/	
	PON_RX_HEC_LEGACY_MODE	     = 0x1,		/* Rx HEC check as in PAS5001-N						*/
	PON_RX_HEC_802_AH_MODE		 = 0x2,		/* Rx HEC check according  to IEEE802.3ah stabdard	*/
	PON_RX_HEX_802_LEFACY_MODE   = 0x3		/* Rx HEC check according to both method		    */
} PON_rx_hec_config_t;


typedef struct PON_olt_hec_configuration_t
{
	PON_tx_hec_config_t  tx_hec_configuration;	/* LEGACY/802_3_AH Tx HEC geneartion			*/
	PON_rx_hec_config_t  rx_hec_configuration;  /* No check/LEGACY/802_3AH/BOTH Rx HEC checking	*/

} PON_olt_hec_configuration_t;

#define PON_EMPTY_OLT_HEC_CONFIGURATION_STRUCT(__olt_hec_configuration_struct__) \
	__olt_hec_configuration_struct__.tx_hec_configuration = PON_VALUE_NOT_CHANGED;\
	__olt_hec_configuration_struct__.rx_hec_configuration = PON_VALUE_NOT_CHANGED;


/*
**  VLAN tag filtering (Applies only for PAS5001-N M3)
 */


typedef enum
{
	PON_MULTIPLE_COPY_BROADCAST_DISABLE				  = DISABLE, /* Disable multiple copy broadcast forwarding									 */
	PON_MULTIPLE_COPY_BROADCAST_ENABLE				  = ENABLE,  /* Enable multiple copy broadcast forwarding (full mode)						 */
	PON_MULTIPLE_COPY_BROADCAST_ONLY_UNKNOWN_UNICAST  = 0x2	     /* Enable multiple copy broadcast forwarding for unknown DA unicast frames only */ 
} PON_multiple_copy_broadcast_t;

typedef struct PON_olt_vlan_tag_filtering_configuration_t
{
	short int  untagged_frames_filtering;						/* Disable / Enable Filtering of Untagged frames -		      */
																/* PAS5001-N M3 dropped un-tagged downstream frames		      */
																/* and not transmit them to the PON.						  */
	short int  multiple_copy_broadcast_enable_mode;				/* Disable / Enable Filtering of tagged multicast frames -    */ 
																/* The PAS5001V3  avert tagged downstream frames with         */
																/* multicast or broadcast DA to the PON and not to the CPU,   */
																/* except for special multicast frames (link-constraint,      */
																/* IGMP control and ARP) that are passed to the CPU           */
																/* regardless of this feature.							      */
	short int  filtering_unexpected_tagged_downstream_frames;   /* Disabel / Enable Filtering of unexpected tagged downstream */
																/* frames- The PAS5001V3 block the transmission of downlink   */
																/* VLAN frames that carry VLAN ID different from the one that */
																/* is expected at their LLID destination.					  */
} PON_olt_vlan_tag_filtering_configuration_t;

#define PON_EMPTY_OLT_VLAN_TAG_FILTERING_CONFIGURATION_STRUCT(__olt_vlan_tag_filtering_configuration_struct__)\
	__olt_vlan_tag_filtering_configuration_struct__.untagged_frames_filtering = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_tag_filtering_configuration_struct__.multiple_copy_broadcast_enable_mode = PON_VALUE_NOT_CHANGED;\
	__olt_vlan_tag_filtering_configuration_struct__.filtering_unexpected_tagged_downstream_frames = PON_VALUE_NOT_CHANGED;

/* OLT runtime changeable parameters struct 
** !!Note: each update in the struct should also update PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT macro !!!
**
** These general parameters may be updated in run-time, Each parameter has non-update value.
**
**	max_rtt						: Maximum round trip time between OLT and ONU in the PON. Stated in TQ.
**								  range: PON_VALUE_NOT_CHANGED, 0 - PON_MAX_RTT
**	address_table_aging_timer	: Address table aging mechanism interval time. MAC addresses will be 
**								  removed from OLT address table after 2 - 3 multiples of this timer. 
**								  Stated in milliseconds, value should be multiple of 
**								  PON_ADDRESS_TABLE_AGING_TIMER_RESOLUTION milliseconds.
**								  range: PON_VALUE_NOT_CHANGED, 
**								  PON_MIN_ADDRESS_TABLE_AGING_TIMER - PON_MAX_ADDRESS_TABLE_AGING_TIMER
**	cni_port_maximum_entries	: Maximum number of entries allowed in OLT address table for addresses 
**								  found over the CNI port. Values: PON_VALUE_NOT_CHANGED, enum values
**								  OLT default value: PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192. 
**	arp_filtering_configuration : ARP frames hardware filetring configuration update. 
**								  Values: accroding to struct definition
**	igmp_configuration			: OLT IGMP configuration, see PON_olt_igmp_configuration_t for
**								  details
**    vlan_configuration		: OLT global VLAN configuration, see PON_olt_vlan_configuration_t
**								  for details
**  hec_configuration			: OLT HEC configuration, see PON_olt_hec_configuration_t
**								  for details
**  vlan_tag_filtering_config	: OLT VLAN tag filtering configuration - see PON_olt_vlan_tag_filtering_configuration_t
**								  for details
**  upstream_default_priority	: The VLAN priority field when adding a VLAN tag to an un-tagged uplink frame
**								  range: PON_MIN_UPSTREAM_DEFAULT_PRIORITY-PON_MAX_UPSTREAM_DEFAULT_PRIORITY
*/
typedef struct PON_olt_update_parameters_t
{
	PON_rtt_t									 max_rtt;
	long										 address_table_aging_timer;
	PON_address_table_entries_llid_limitation_t  cni_port_maximum_entries;
	PON_arp_filtering_configuration_t			 arp_filtering_configuration;
	PON_olt_igmp_configuration_t				 igmp_configuration;
	PON_olt_vlan_configuration_t				 vlan_configuration;
	PON_olt_hec_configuration_t					 hec_configuration;
	PON_olt_vlan_tag_filtering_configuration_t   vlan_tag_filtering_config;
	short int									 upstream_default_priority;
    PON_polarity_t                               pon_tbc_polarity;

} PON_olt_update_parameters_t;

/* OLT runtime changeable parameters struct 
** !!Note: each update in the struct should also update PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT macro !!!
**
** These general parameters may be updated in run-time, Each parameter has non-update value.
**
**	max_rtt						: Maximum round trip time between OLT and ONU in the PON. Stated in TQ.
**								  range: PON_VALUE_NOT_CHANGED, 0 - PON_MAX_RTT
**  hec_configuration			: OLT HEC configuration, see PON_olt_hec_configuration_t
**								  for details
**  grant_filtering	            : Frames arriving from a certain LLID during other LLID's receive window are accepted or not
**                                range : PON_VALUE_NOT_CHANGED, DISABLE , ENABLE
**  support_passave_onus        : Supporting or not Passave ONUs' extended capabilities 
**                                range : PON_VALUE_NOT_CHANGED, FALSE , TRUE
*/
typedef struct PON_update_olt_parameters_t
{
	PON_rtt_t									 max_rtt;
	PON_olt_hec_configuration_t					 hec_configuration;
    short int                                    grant_filtering;
    short int                                    support_passave_onus;
} PON_update_olt_parameters_t;

/* == 4 == the end of data struct " PON_olt_update_parameters_t " ====== */


/* == 5 == the start of data struct " PAS_system_parameters_t " ====== */

/* PON initialization struct 
**
** automatic_authorization_policy - ENABLE /DISABLE PAS-SOFT auto-authorization for each registrating ONU
**
*/
/*
typedef struct PAS_pon_initialization_parameters_t
{
	bool		   automatic_authorization_policy;
} PAS_pon_initialization_parameters_t;
*/
/* 
** System parameters struct 
**
** statistics_sampling_cycle - Frequency of statistics sampling into statistics table by PAS-SOFT. 
** Stated in Milliseconds. Values: PON_VALUE_NOT_CHANGED, PON_CONTINUOUS_SAMPLING,
** PON_MIN_STATISTICS_SAMPLING_CYCLE - PON_MAX_STATISTICS_SAMPLING_CYCLE
**
** monitoring_cycle - Frequency of monitoring cycle beginning, stated in Milliseconds.				
** Values: PON_VALUE_NOT_CHANGED, PON_CONTINUOUS_MONITORING,
** PON_MIN_MONITORING_CYCLE - PON_MAX_MONITORING_CYCLE
**
** host_olt_msgs_timeout - Timeout waiting to a response from OLT firmware after sending it a msg. Value
** stated in milliseconds. Values: PON_VALUE_NOT_CHANGED, 
** PON_MIN_RESPONSE_MSG_TIMEOUT - PON_MAX_RESPONSE_MSG_TIMEOUT
**
** olt_reset_timeout - The number of msgs-sending timeouts after which OLT is reset (OLT Reset event)
** Values: PON_VALUE_NOT_CHANGED, PON_NO_TIMEOUT (OLT reset by timeouts mechanism not activated), 
** 1 - MAXINT
**
** automatic_authorization_policy - Auto-authorization policy for each registrating ONU, 
** values: PON_VALUE_NOT_CHANGED / ENABLE / DISABLE PAS-SOFT
*/
typedef struct PAS_system_parameters_t
{
	long int   statistics_sampling_cycle;       
	long int   monitoring_cycle;				
	short int  host_olt_msgs_timeout;
	short int  olt_reset_timeout;
	short int  automatic_authorization_policy;
} PAS_system_parameters_t;

/* == 5 == the end of data struct " PAS_system_parameters_t " ====== */



/* == 6 == the start of data struct " PAS_device_versions_t " ====== */

/* Device components versions structure */

/* MAC types */
typedef enum 
{
	PON_MII,
	PON_GMII,
	PON_TBI,
	PON_UNKNOWN
} PON_mac_t;

extern char *PON_mac_s[];

/* Link rate */
typedef enum 
{
	PON_10M   = 10,	  /* MB/Sec				 */
	PON_100M  = 100,  /* MB/Sec				 */
	PON_1000M = 1000  /* MB/Sec (= 1 GB/Sec) */
} PON_link_rate_t;

/* Duplex mode */
typedef enum 
{
	PON_HALF_DUPLEX,
	PON_FULL_DUPLEX
} PON_duplex_status_t;

extern char *PON_duplex_status_s[];

/* Device components versions structure */
typedef struct PAS_device_versions_t
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
} PAS_device_versions_t;	

typedef struct PON_remote_mgnt_version_t  
{
  short int    major_version;
  short int    minor_version;
} PON_remote_mgnt_version_t;

typedef struct PON_version 
{
    short int major;
    short int minor;
} PON_version;

typedef PON_version PON_version_t;

typedef struct  
{
  bool                    pmc_onu;
  PON_version             hardware_version; 
} PAS_pmc_onu_version_t;


/* == 6 == the end of data struct " PAS_device_versions_t " ====== */


/* == 7 == the start of data struct " PON_oam_uni_port_mac_configuration_t " ====== */

/* MDIO status data 
**
** Collection of PHY register bits as read by the MII/GMII module 
** MDIO register map and valid values are defined in the 802.3 standard (clauses 22, 28, 37, and 40.5) 
*/
typedef struct PON_oam_mii_phy_advertisement_t
{
	bool			_10base_tx_half_duplex;		/* PHY MDIO R4.5 (PHY) / R5.5 (PHY partner)	 */			
												/* TRUE - advertised, FALSE - Not advertised */
	bool			_10base_tx_full_duplex;		/* PHY MDIO R4.6 (PHY) / R5.6 (PHY partner)	 */	
												/* TRUE - advertised, FALSE - Not advertised */
	bool			_100base_tx_half_duplex;	/* PHY MDIO R4.7 (PHY) / R5.7 (PHY partner)	 */	
												/* TRUE - advertised, FALSE - Not advertised */
	bool			_100base_tx_full_duplex;	/* PHY MDIO R4.8 (PHY) / R5.8 (PHY partner)	 */	
												/* TRUE - advertised, FALSE - Not advertised */
	bool			_100base_t4;				/* PHY MDIO R4.9 (PHY) / R5.9 (PHY partner)	 */	
												/* TRUE - advertised, FALSE - Not advertised */
												/*ONU always FALSE -not capable of 100BASE-T4*/
	bool			pause;						/* PHY MDIO R4.10 (PHY) / R5.10 (PHY partner)*/	
												/* ENABLE  - MAC pause implemented,		     */
												/* DISABLE - MAC pause not implemented	     */
	bool			asymmetric_pause;			/* PHY MDIO R4.11 (PHY) / R5.11 (PHY partner)*/
												/* ENABLE - asymmetric pause implemented,	 */
												/* DISABLE - no asymmetric pause implemented */
												
												/* PHY MDIO R4.12 (PHY) / R5.12 (PHY partner)*/
												/* Reserved								     */
} PON_oam_mii_phy_advertisement_t; /* MII PHY TAF data */

typedef struct PON_oam_gmii_phy_advertisement_t
{
	bool					 _1000base_tx_half_duplex;	/* PHY MDIO R9.8 (PHY) /R10.10 (PHY partner) */
														/* TRUE - advertised, FALSE - Not advertised */
	bool					 _1000base_tx_full_duplex;	/* PHY MDIO R9.9 (PHY) /R10.11 (PHY partner) */
														/* TRUE - advertised, FALSE - Not advertised */
	PON_master_slave_t		 preferable_port_type;		/* PHY MDIO R9.10, values:					 */
														/* PON_SLAVE  - prefer single port device	 */
														/* PON_MASTER - prefer multi port device	 */
} PON_oam_gmii_phy_advertisement_t; /* GMII-specific PHY advertisement data */

typedef struct PON_oam_phy_advertisement_t
{
	PON_oam_mii_phy_advertisement_t   mii_phy_advertisement;  
	PON_oam_gmii_phy_advertisement_t  gmii_phy_advertisement;
} PON_oam_phy_advertisement_t; 

typedef struct PON_oam_uni_port_mac_configuration_t
{
	PON_mac_t					 mac_type;				/*UNI Port MAC Type							   */
	PON_link_rate_t				 mii_type_rate;			/*Not including PON_1000M. MII MAC type is	   */ 
														/*composed of rate and duplex				   */
														/*Valid only if (uni_port_mac_type == PON_MII) */
	PON_duplex_status_t			 mii_type_duplex;		/*MII MAC type is composed of rate and duplex  */
														/*Valid only if (uni_port_mac_type == PON_MII) */
	bool						 autonegotiation;		/*ENABLE / DISABLE auto-negotiation in UNI port*/
	PON_master_slave_t			 master;				/*Master / slave mode in UNI port,			   */
														/*PON_MASTER - regular MAC-PHY mode,		   */
														/*PON_SLAVE - MAC disguise as PHY			   */
	bool						 advertise;				/*ENABLE / DISABLE advertise in UNI port	   */
	PON_oam_phy_advertisement_t  advertisement_details; /*Advertising PHY Mode						   */
	unsigned char				 mdio_phy_address;		/*MDIO PHY address, values:					   */
														/*MDIO_MIN_PHY_ADDRESS - MDIO_MAX_PHY_ADDRESS  */
} PON_oam_uni_port_mac_configuration_t;

/* == 7 == the end of data struct " PON_oam_uni_port_mac_configuration_t " ====== */

/* == 8 == the start of data struct " PON_alarm_t " ====== */
#define ALARM_TYPE_MAX_SIZE     32

/* Alarm types */
#if 0   /* V5.3.0 以前版本 */
typedef enum
{  
	PON_ALARM_BER,
	PON_ALARM_FER,
	PON_ALARM_SOFTWARE_ERROR,
	PON_ALARM_LOCAL_LINK_FAULT,	
	PON_ALARM_DYING_GASP,	
	PON_ALARM_CRITICAL_EVENT,
	PON_ALARM_REMOTE_STABLE,
	PON_ALARM_LOCAL_STABLE,
	PON_ALARM_OAM_VENDOR_SPECIFIC,
	PON_ALARM_ERRORED_SYMBOL_PERIOD,
	PON_ALARM_ERRORED_FRAME,
	PON_ALARM_ERRORED_FRAME_PERIOD,
	PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY,
	PON_ALARM_ONU_REGISTRATION_ERROR,
	PON_ALARM_OAM_LINK_DISCONNECTION,
	PON_ALARM_BAD_ENCRYPTION_KEY,
	PON_ALARM_LLID_MISMATCH,
    PON_ALARM_TOO_MANY_ONU_REGISTERING,
    /* From PAS5201 */
    PON_ALARM_PORT_BER,
    PON_ALARM_DEVICE_FATAL_ERROR,
	PON_ALARM_LAST_ALARM
} PON_alarm_t;
#endif

typedef enum
{  
	PON_ALARM_BER,
	PON_ALARM_FER,
	PON_ALARM_SOFTWARE_ERROR,
	PON_ALARM_LOCAL_LINK_FAULT,	
	PON_ALARM_DYING_GASP,	
	PON_ALARM_CRITICAL_EVENT,
	PON_ALARM_REMOTE_STABLE,
	PON_ALARM_LOCAL_STABLE,
	PON_ALARM_OAM_VENDOR_SPECIFIC,
	PON_ALARM_ERRORED_SYMBOL_PERIOD,
	PON_ALARM_ERRORED_FRAME,
	PON_ALARM_ERRORED_FRAME_PERIOD,
	PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY,
	PON_ALARM_ONU_REGISTRATION_ERROR,
	PON_ALARM_OAM_LINK_DISCONNECTION,
	PON_ALARM_BAD_ENCRYPTION_KEY,
	PON_ALARM_LLID_MISMATCH,
    PON_ALARM_TOO_MANY_ONU_REGISTERING,
	 /* From PAS5201 */
    PON_ALARM_PORT_BER,
    PON_ALARM_DEVICE_FATAL_ERROR,
    PON_ALARM_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON,
    PON_ALARM_VIRTUAL_SCOPE_ONU_SIGNAL_DEGRADATION,
#ifdef  PAS_SOFT_VERSION_V5_3_5
	PON_ALARM_VIRTUAL_SCOPE_ONU_EOL,
	PON_ALARM_VIRTUAL_SCOPE_ONU_EOL_DATABASE_IS_FULL,
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
    PON_ALARM_ONU_REGISTERING_WITH_EXISTING_MAC,
#endif
	PON_ALARM_LAST_ALARM
} PON_alarm_t;

/* Return a string corresponding to the alarm type */
extern char *PON_alarm_s[];

#ifdef  PAS_SOFT_VERSION_V5_3_13
/* PON_ALARM_ONU_REGISTERING_WITH_EXISTING_MAC 
**
** Error onus registering with existing MAC. 
** Alarm parameter: 0
** Alarm data type and values: ONU's MAC address
**
** Configuration: No configuration is available for this alarm type
*/
typedef mac_address_t PON_onu_registering_with_existing_mac_alarm_data_t;
#endif

/* == 8 == the end of data struct " PON_alarm_t " ====== */


/* == 9 == the start of data struct " PON_olt_reset_code_t " ====== */

/* 
** OLT reset codes
**
** Scenarios for OLT reset event
*/
typedef enum
{
	PON_OLT_RESET_HOST_TIMEOUT,			  /* Several Host - OLT msgs were timedout (configurable by	    */
										  /* PAS_set_system_parameters function)					    */
	PON_OLT_RESET_OLT_EVENT,			  /* OLT sent 'reset' event, indicating it was physically reset */
	PON_OLT_RESET_OLT_NOT_INITED_RESPONSE /* OLT sent 'Not inited' msg - meaning non-Init command was   */
										  /* sent to uninitialized OLT									*/
} PON_olt_reset_code_t;

extern char *PON_olt_reset_code_s[];

/* == 9 == the end of data struct " PON_olt_reset_code_t " ====== */

/* == 10 == the start of data struct " PON_olt_physical_port_t " ====== */
/* 
** Physical ports definitions
**
*/

/* OLT physical data ports */
/* Note: update PON_OLT_PHYSICAL_PORT_IN_BOUNDS on changes */
typedef enum
{
	OLT_PHYSICAL_PORT_AUTOMATIC,	  /* Default selection of physical port */
	OLT_PHYSICAL_PORT_PON,			  /* PON port						    */
	OLT_PHYSICAL_PORT_SYSTEM,		  /* a.k.a. CNI, Network, upstream port */
	OLT_PHYSICAL_PORT_PON_AND_SYSTEM  /* Both PON and System ports			*/
} PON_olt_physical_port_t;

/* ONU physical ports */
typedef enum PON_onu_physical_port_t
{
	ONU_PHYSICAL_PORT_AUTOMATIC = OLT_PHYSICAL_PORT_AUTOMATIC, /* Default selection of physical port */
	ONU_PHYSICAL_PORT_PON		= OLT_PHYSICAL_PORT_PON,	   /* PON port						     */
	ONU_PHYSICAL_PORT_SYSTEM	= OLT_PHYSICAL_PORT_SYSTEM     /* a.k.a. Network, upstream port      */
} PON_onu_physical_port_t;

/* OLT physical management port */
#define OLT_PHYSICAL_PORT_HOST_MII (OLT_PHYSICAL_PORT_PON_AND_SYSTEM +1)

/* Frame destination OLT port options */
typedef enum
{
	PON_PORT_PON,					   /* PON port													   */
	PON_PORT_SYSTEM,                   /* a.k.a. Network											   */
	PON_PORT_PON_AND_SYSTEM,		   /* Both PON and System ports									   */
	PON_PORT_AUTOMATIC,                /* Default selection of physical port by address table / Both   */
									   /* PON and System ports (if address not found in address table) */
	PON_PORT_AUTOMATIC_INHIBIT_ALL,	   /* Default selection / discard if address not found in address  */
									   /* table)													   */
	PON_PORT_AUTOMATIC_INHIBIT_PON,	   /* Default selection / System port (if address not found in     */
									   /* address table)											   */
	PON_PORT_AUTOMATIC_INHIBIT_SYSTEM  /* Default selection / PON port (if address not found in address*/
									   /* table)													   */
} PON_sent_frame_destination_port_t;



/* == 10 == the end of data struct " PON_olt_physical_port_t " ====== */

/* == 11 == the start of data struct " PON_frame_type_t " ====== */

/* Frame types supported by OLT hardware */
typedef enum
{
	PON_FRAME_TYPE_DEFUALT,		 /* Hardware determines the frame type automatically    */
	PON_FRAME_TYPE_DATA,		 /* Regular Ethernet frame, containing high layers data */
	PON_FRAME_TYPE_PON_CONTROL   /* 60 bytes (net) PON control frame					*/
} PON_frame_type_t;


extern unsigned char *PON_frame_type_s[];

/* == 11 == the end of data struct " PON_frame_type_t " ====== */


/* == 12 == the end of data struct " PON_authorize_mode_t " ====== */

/* Authorize modes */
typedef enum
{  
	PON_DENY_FROM_THE_NETWORK,
	PON_AUTHORIZE_TO_THE_NETWORK,
	PON_AUTHORIZING_TO_THE_NETWORK,
	PON_AUTHORIZE_MODE_LAST_MODE
} PON_authorize_mode_t;

extern char *PON_authorize_mode_s[];

extern char *PON_support_s[];

/* == 12 == the end of data struct " PON_authorize_mode_t " ====== */


/* == 13  == the start of data struct " Encryption definitions " ====== */

/* 
** Encryption definitions
**
*/

typedef enum
{
    PON_ENCRYPTION_KEY_INDEX_0,
    PON_ENCRYPTION_KEY_INDEX_1
} PON_encryption_key_index_t;


/* Encryption types */
typedef enum PON_encryption_type_t
{  
    PON_ENCRYPTION_TYPE_AES,			  /* Advanced Encryption Standard algorithm								 */
    PON_ENCRYPTION_TYPE_TRIPLE_CHURNING,  /* Triple churning algorithm, Supported in PAS5201 \ PAS6301 and above */
} PON_encryption_type_t;


/* Hardware supported encryption types */
typedef enum
{  
	PON_ENCRYPTION_NONE,
	PON_ENCRYPTION_AES
} PON_encryption_t;

/* Hardware supported encryption directions */
typedef enum
{  
	PON_ENCRYPTION_DIRECTION_DOWNLINK				= PON_DIRECTION_DOWNLINK,
	PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK	= PON_DIRECTION_UPLINK_AND_DOWNLINK,
	PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION			= PON_DIRECTION_UPLINK_AND_DOWNLINK + 1,
    PON_ENCRYPTION_DIRECTION_NO_ENCRYPTOIN          = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION /* spelling mistake,for backware compatible */
} PON_encryption_direction_t;

/* Hardware supported encryption key sizes */
#define PON_ENCRYPTION_KEY_SIZE		16  /* Bytes */

/* Encryption key */
typedef unsigned char PON_encryption_key_t[PON_ENCRYPTION_KEY_SIZE];


/* Encryption key update technique */
typedef enum
{  
	PON_ENCRYPTION_UPDATE_PASSAVE	/* Passave specific protocol using OAMPDUs */
} PON_encryption_key_update_t;

/* Encryption management framework */
typedef enum
{  
	PON_ENCRYPTION_MANAGEMENT_PASSAVE,		/* Passave specific protocol using OAMPDUs	*/
	PON_ENCRYPTION_MANAGEMENT_OPEN			/* Open encryption managment protocol		*/ 		
} PON_encryption_management_framework_t;

/* Start encryption acknowledge codes */
typedef enum
{  
	PON_START_ENCRYPTION_SUCCESS,
	PON_START_ENCRYPTION_ERROR
} PON_start_encryption_acknowledge_codes_t;

/* Update encryption key acknowledge codes */
typedef enum
{  
	PON_UPDATE_ENCRYPTION_KEY_SUCCESS,
	PON_UPDATE_ENCRYPTION_KEY_ERROR
} PON_update_encryption_key_acknowledge_codes_t;

/* Stop encryption acknowledge codes */
typedef enum
{  
	PON_STOP_ENCRYPTION_SUCCESS,
	PON_STOP_ENCRYPTION_ERROR
} PON_stop_encryption_acknowledge_codes_t;


/* Encryption error event return codes */ 
/* any change here should be also be done in PON_encryption_result_event_return_codes_s*/
typedef enum
{
    PON_ENCRYPTION_RESULT_EVENT_HPROT_ENCRYPTION_START_SUCCESS  , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_ENCRYPTION_FAILURE        , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_ENCRYPTION_NOT_COMPLETE   , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_UPDATE_KEY_SUCCESS        , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_KEY_NOT_LOADED            , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_UPDATE_ON_UPDATE_KEY      , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_UPDATE_KEY_ABORT          , 
    PON_ENCRYPTION_RESULT_EVENT_HPROT_UPDATE_KEY_NOT_ABORTED    ,
    PON_ENCRYPTION_RESULT_EVENT_HPROT_UPDATE_KEY_HW_COMPLETE     
} PON_encryption_result_event_return_codes_t;

extern char *PON_encryption_result_event_return_codes_s[];

extern char *PON_priority_queue_s[];


/* == 13  == the end of data struct " Encryption definitions " ====== */

/* == 14  == the start of data struct " PAS_olt_test_results_t  " ====== */

/* Test results */
#define TEST_SUCCEEDED			 0
#define TEST_FAILED				 1
#define TEST_SUCCEEDED_PARTIALLY 2

/* Function which returns a string of the corresponding test result */
extern char *PON_test_result_s[];

/* OLT self test results */
typedef enum 
{
	PAS_OLT_TEST_SUCCEEDED			 = TEST_SUCCEEDED,			 
	PAS_OLT_TEST_FAILED				 = TEST_FAILED,				 
	PAS_OLT_TEST_SUCCEEDED_PARTIALLY = TEST_SUCCEEDED_PARTIALLY
} PAS_olt_test_results_t;

/* == 14  == the end of data struct " PAS_olt_test_results_t  " ====== */

/* == 15  == the start of data struct " log flags define  " ====== */

/*
** Log flags (groups)
** Note: When a new flag is added, update Main_init in PAS mudule!
*/
typedef enum
{
	PON_LOG_REDIRECTION_INTERNAL, /* PAS-SOFT Internal handling - printout to stdout  */
	PON_LOG_REDIRECTION_EXTERNAL, /* External redirection                             */
                                 
	PON_REDIRECTION_LOG_LAST	  /* Illegal log indirection, marks the end if the enum					*/
} PON_log_redirection_t;

/*
** Log flags (groups)
** Note: When a new flag is added, update Main_init in PAS mudule!
*/
typedef enum
{
	PON_LOG_FLAG_CRITICAL_ERROR, /* PAS-SOFT compilation error / critical errors during runtime */
								 /* corresponds to CRITICAL_ERROR_DEBUG compilation flag		*/
	PON_LOG_FLAG_OUT,			 /* PAS-SOFT normal operation indications						*/
								 /* corresponds to OUT_DEBUG compilation flag					*/
	PON_LOG_FLAG_COMM,			 /* PAS-SOFT communiaction operations with OLTs					*/
								 /* corresponds to COMM_DEBUG compilation flag					*/
	PON_LOG_FLAG_HWIF,			 /* PAS-SOFT hardware (/host) interface operations				*/
								 /* corresponds to HWIF_DEBUG compilation flag					*/
    PON_LOG_FLAG_ERROR,	         /* PAS-SOFT error debug errors                 				*/
								 /* corresponds to ERROR_DEBUG compilation flag					*/
    PON_LOG_FLAG_TBD_2,			 /* for future use                                              */
    PON_LOG_FLAG_TBD_3,			 /* for future use                                              */
    PON_LOG_FLAG_ALL_PASSOFT,	 /* all PAS-SOFT logs                                           */		 
    PON_LOG_FLAG_FW_NONE,	     /* all logs levels              				                */					 
    PON_LOG_FLAG_FW_NOTE,		 /* OLT FW debug messages              				            */								 
    PON_LOG_FLAG_FW_WARNING,	 /* OLT FW warnings                               				*/
    PON_LOG_FLAG_FW_ERROR,		 /* OLT FW errors           			            	        */
    PON_LOG_FLAG_FW_SEVER,		 /* OLT FW severe errors             				            */
    PON_LOG_FLAG_FW_FATAL,		 /* OLT FW fatal errors             				            */                        
    PON_LOG_FLAG_FW_AUTHENTICATION, /* OLT FW authentication traces             				    */
    PON_LOG_FLAG_FW_MPCP_RX,        /* OLT FW MPCP Rx traces             				            */
    PON_LOG_FLAG_FW_OAM_RX,         /* OLT FW OAM Rx traces             				            */
     
	PON_LOG_FLAG_LAST_FLAG		 /* Illegal log flag, marks the end if the enum					*/
} PON_log_flag_t;


typedef enum
{
	PON_PAS_LOG_FLAG_CRITICAL_ERROR,							 
	PON_PAS_LOG_FLAG_OUT,											
	PON_PAS_LOG_FLAG_COMM,											
	PON_PAS_LOG_FLAG_HWIF,									
	PON_PAS_LOG_FLAG_ERROR,
	PON_PAS_LOG_FLAG_FW,	    				                                
	PON_PAS_LOG_FLAG_LAST_FLAG	
} PON_pas_log_flag_t;

/* == 15  == the end of data struct " log flags define  " ====== */

/* == 16  == the start of data struct " handler functions enum  " ====== */
 
/* Handler functions enum */
typedef enum
{
	PAS_HANDLER_FRAME_RECEIVED,					        /* Ethernet frame originated from OLT's PON / System    */
												        /* port was received								    */
	PAS_HANDLER_ONU_REGISTRATION,				        /* ONU registration to the PON					        */
	PAS_HANDLER_ALARM,							        /* Hardware / Software alarm					        */
	PAS_HANDLER_OLT_RESET,						        /* OLT reset 									        */
	PAS_HANDLER_ONU_DEREGISTRATION,				        /* ONU deregistration (disconnection) from the PON      */
	PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE,           /* Acknowledge for PAS_start_encryption function        */
	PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE,      /*Acknowledge for PAS_update_encryption_key function    */
	PAS_HANDLER_STOP_ENCRYPTION_ACKNOWLEDGE,	        /* Acknowledge for PAS_stop_encryption function	        */
	PAS_HANDLER_LOAD_OLT_BINARY_COMPLETED,		        /* Load OLT binary process completed				    */
	PAS_HANDLER_DBA_EVENT,						        /* Event originating from DBA algorithm			        */
    PAS_HANDLER_RADIUS_MESSAGE_RECEIVED,		        /* RADIUS message originated from OLT received	        */
	PAS_HANDLER_ONU_AUTHORIZATION,				        /* Authorization mode update of a single ONU		    */
    PAS_HANDLER_EVENT_LOG,                              /* Received event log message                           */
    PAS_HANDLER_PON_LOSS,                               /* PON loss                                             */
    PAS_HANDLER_ENCRYPTION_RESULT,                      /* There is a result in the encryption                  */
	PAS_HANDLER_AUTHENTICATION_KEY_RECEIVED,		    /* Authentication key received	  			            */
    PAS_HANDLER_OLT_ADD,								/* OLT add										        */
	PAS_HANDLER_OLT_REMOVE,								/* OLT remove									        */
	PAS_HANDLER_ONU_DENIED,								/* Denied ONU 											*/

#ifdef  PAS_SOFT_VERSION_V5_3_5
	PAS_HANDLER_CONVERT_RSSI_TO_DBM,					/* Convert RSSI value to dBm units						*/
	PAS_HANDLER_GET_OLT_TEMPERATURE,					/* Retrieve OLT temperature 							*/
   	PAS_HANDLER_CNI_LINK,                               /* CNI link                                             */
#ifdef  PAS_SOFT_VERSION_V5_3_11
    PAS_HANDLER_REDUNDANCY_OLT_FAILURE,                 /* redundancy OLT failure                               */
    PAS_HANDLER_REDUNDANCY_SWITCH_OVER,                 /* redundancy switch over                               */
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_12
    PAS_HANDLER_OLT_DOWNLINK_FEC_CHANGED,           	/*If some OLT DOWNLINK FEC changed, trigger this event*/
    PAS_HANDLER_REDUNDANCY_OLT_FAILURE_WITH_EXTEND_INFO,/* redundancy OLT failure with extended infor           */
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
    PAS_HANDLER_MIGRATION_ATTEMPT,                              /* Migration attempt               */
#endif
#endif

    /* for INTERNAL use */
	PAS_HANDLER_REMOTE_EEPROM_REGISTER,					/* read EEPROM register message originated from onu		*/
	PAS_HANDLER_REMOTE_ACCESS_ACKNOWLEDGE,				/* general ack originated from onu						*/
	PAS_HANDLER_REMOTE_MDIO_ACCESS,						/* read MDIO register message originated from onu		*/
	PAS_HANDLER_REMOTE_MEMORY_BLOCK,					/* write memory block message originated from onu		*/
	PAS_HANDLER_PONG,									/* pong message originated from onu						*/
	PAS_HANDLER_REMOTE_OAM_INFORMATION,                 /* oam information message originated from onu			*/
    PAS_HANDLER_REMOTE_STATISTIC,                       /* response of the remote statistics from onu           */
    PAS_HANDLER_LAST_HANDLER,
    
#ifdef  PAS_SOFT_VERSION_V5_3_13
	  PAS_HANDLER_LAST_EXPO_HANDLER = PAS_HANDLER_MIGRATION_ATTEMPT
#elif defined(PAS_SOFT_VERSION_V5_3_12)
	  PAS_HANDLER_LAST_EXPO_HANDLER = PAS_HANDLER_REDUNDANCY_OLT_FAILURE_WITH_EXTEND_INFO
#elif defined(PAS_SOFT_VERSION_V5_3_11)
	  PAS_HANDLER_LAST_EXPO_HANDLER = PAS_HANDLER_REDUNDANCY_SWITCH_OVER
#elif defined(PAS_SOFT_VERSION_V5_3_5)
	  PAS_HANDLER_LAST_EXPO_HANDLER = PAS_HANDLER_CNI_LINK
#else 
	  PAS_HANDLER_LAST_EXPO_HANDLER = PAS_HANDLER_ONU_DENIED
#endif

} PAS_handler_functions_index_t;

/* == 16  == the end of data struct " handler functions enum  " ====== */

/* == 17  == the start of data struct " olt test result  " ====== */

/* Parameters: test result									*/
typedef enum 
{
	PASCOMM_TEST_SUCCEEDED			 = TEST_SUCCEEDED,			 
	PASCOMM_TEST_FAILED				 = TEST_FAILED,				 
	PASCOMM_TEST_SUCCEEDED_PARTIALLY = TEST_SUCCEEDED_PARTIALLY
} PASCOMM_test_results_t;

/* == 17  == the end of data struct " olt test result  " ====== */

/* == 18  == the end of data struct " Plato DBA algorithm  " ====== */
#define PLATO2_ERR_OK                PAS_EXIT_OK
#define PLATO2_ERR_GENERAL_ERROR     PAS_EXIT_ERROR
#define PLATO2_ERR_PARAMETER_ERROR   PAS_PARAMETER_ERROR
#define PLATO2_ERR_DBA_NOT_LOADED    PAS_DBA_DLL_NOT_LOADED
#define PLATO2_ERR_NO_COMMUNICATION  PAS_TIME_OUT
#define PLATO2_ERR_OLT_NOT_EXIST     PAS_OLT_NOT_EXIST
#define PLATO2_ERR_DBA_NOT_RUNNING   PAS_DBA_NOT_RUNNING
#define PLATO2_ERR_ONU_NOT_AVAILABLE PAS_ONU_NOT_AVAILABLE

#define PLATO2_ECODE_NO_ERROR                    0
#define PLATO2_ECODE_UNKNOWN_LLID                1
#define PLATO2_ECODE_TOO_MANY_LLIDS              2
#define PLATO2_ECODE_ILLEGAL_LLID                3
#define PLATO2_ECODE_MIN_GREATER_THAN_MAX        4
#define PLATO2_ECODE_MIN_TOTAL_TOO_LARGE         5
#define PLATO2_ECODE_ILLEGAL_CLASS               6
#define PLATO2_ECODE_ILLEGAL_GR_BW               7
#define PLATO2_ECODE_ILLEGAL_BE_BW               8
#define PLATO2_ECODE_ILLEGAL_BURST               9
#define PLATO2_ECODE_ILLEGAL_CYCLE_LENGTH        10
#define PLATO2_ECODE_ILLEGAL_DISCOVERY_FREQUENCY 11


typedef struct {
  unsigned char class;           /* 0 (lowest priority) ?7 (highest priority) */
  unsigned char delay;           /* 1 (lowest delay) ?2 (high delay) */
  unsigned short max_gr_bw;      /* Max guaranteed bandwidth (1Mbps units)  */
  unsigned short max_gr_bw_fine; /* Max guaranteed bandwidth (64Kbps units)  */
  unsigned short max_be_bw;      /* Max best effort bandwidth (1Mbps units) */
  unsigned short max_be_bw_fine; /* Max best effort bandwidth (64Kbps units) */
} PLATO2_SLA_t;

typedef struct {
  unsigned short cycle_length;        /* Cycle length in 50us units */
  unsigned short discovery_frequency; /* Disocvery frequency in 1ms units */
} PLATO2_conf_t;

/* == 18  == the end of data struct " Plato DBA algorithm  " ====== */

/* == 19  == the start of data struct " alarm details  " ====== */

/* PON_ALARM_BER
**
** High BER value alarm, based on PAS-SOFT internal monitoring mechanism. Alarm specificaitons:  
**
** Alarm source_id:		PON_PAS_SOFT_ID		  |		PON_OLT_ID	 |  PON_MIN_ONU_ID_PER_OLT - 
**							  |				 |  PON_MAX_ONU_ID_PER_OLT	
**				--------------------------|------------------|--------------------------
** alarm parameter:		Illegal source id	  |		 onu_id		 |		0		
** alarm data:			Illegal source id	  |		 PON_ber_t	 |		PON_ber_t
**
** Configuration: 
** 
** source: PON_OLT_ID - configuration applicable for all the ONUs in the specified OLT
** configuration structure: PON_ber_alarm_configuration_t;
*/

typedef PON_ber_t  PON_ber_alarm_data_t;

#if 0
#define BER_THRESHOLD  0.1
#define BER_BYTES 1000 /* PAS default value is:10000 */
#endif

typedef struct PON_ber_alarm_configuration_t
{
	PON_pon_network_traffic_direction_t  direction; /* Traffic direction to monitor for PON_ALARM_BER */
	long double							 ber_threshold; /* Bit error rate measurement threshold above */
														/* which the alarm is raised				  */
														/* Values: 0 - 1							  */
	unsigned long						 minimum_error_bytes_threshold; /* The minimal number of error */
																		/* bytes required for the alarm*/
} PON_ber_alarm_configuration_t;


/* PON_ALARM_FER
**
** High FER value alarm, based on PAS-SOFT internal monitoring mechanism. Alarm specificaitons: 
**
** alarm parameter for OLT originated (PON_OLT_ID source_id) FER alarm:
**	  llid number		   - FER meter (in the OLT) which triggered the alarm
**    PON_ALL_ACTIVE_LLIDS - Indicates global PON port HEC violations
**
** Aalrm source_id:		PON_PAS_SOFT_ID		  |	PON_OLT_ID		  |  PON_MIN_ONU_ID_PER_OLT - 
**							  |				  |  PON_MAX_ONU_ID_PER_OLT	
**				--------------------------|---------------------------|--------------------------
** alarm parameter:		Illegal source id	  |llid, PON_ALL_ACTIVE_LLIDS |		0		
** alarm data:			Illegal source id	  |		 PON_fer_t	  |		PON_fer_t
**
**
** Configuration: 
** 
** source: PON_OLT_ID - configuration applicable for all the ONUs in the specified OLT, and for uplink 
**						global PON port HEC violations 
** configuration structure: PON_fer_alarm_configuration_t;
*/

#if 0
#define FER_THRESHOLD  0.1
#define FER_BYTES 100 /* PAS default value is 1000 */
#endif

typedef PON_fer_t PON_fer_alarm_data_t;

typedef struct PON_fer_alarm_configuration_t
{
	PON_pon_network_traffic_direction_t  direction; /* Traffic direction to monitor for PON_ALARM_FER */
	long double							 fer_threshold; /* Frame error rate measurement threshold     */
														/* above which the alarm is raised			  */
														/* Values: 0 - 1							  */
	unsigned long						 minimum_error_frames_threshold;/*The minimal number of error */
																	   /*frames required for the alarm*/
} PON_fer_alarm_configuration_t;

/* Link parameters default values */
#define PON_MONITORING_BER_THRESHOLD_DEFAULT_VALUE						1
#define PON_MONITORING_MINIMUM_ERROR_BYTES_THRESHOLD_DEFAULT_VALUE		1000
#define PON_MONITORING_FER_THRESHOLD_DEFAULT_VALUE						1
#define PON_MONITORING_MINIMUM_ERROR_FRAMES_THRESHOLD_DEFAULT_VALUE	    100
#define PON_MONITORING_DYING_GASP_THRESHOLD_DEFAULT_VALUE				2500000
#define PON_MONITORING_LLID_MISMATCH_THRESHOLD_DEFAULT_VALUE			1


/* PON_ALARM_SOFTWARE_ERROR
**
** Software internal operation error - in most cases caused by data structures inconsistency.
** Alarm olt_id:    PON_PAS_SOFT_ID
** Alarm source_id: PON_PAS_SOFT_ID
** Alarm parameter: 0
** Alarm data type: PON_software_error_alarm_data_t 
**
** Configuration: No configuration is available for this alarm type
*/

typedef enum
{  
	PON_SOFTWARE_ALARM_SEMAPHORE_ERROR,			
	PON_SOFTWARE_ALARM_DATA_INCONSISTENCY,		
	PON_SOFTWARE_ALARM_MEMORY_ALLOCATION_ERROR	
} PON_software_error_alarm_data_t;

extern unsigned char *Pon_software_error_alarm_s[];


/* PON_ALARM_LOCAL_LINK_FAULT 
**
** 802.3 OAM Link fault alarm. Local device's receive path has detected a fault.
** Alarm parameter: 0
** Alarm data: NULL	
**
** Configuration structure: PON_link_fault_alarm_configuration_t
*/
typedef int PON_link_fault_alarm_data_t;

typedef struct PON_link_fault_alarm_configuration_t
{
	unsigned short		nothing;			/*  nothing */
} PON_link_fault_alarm_configuration_t;

/* PON_ALARM_DYING_GASP
** 
** 802.3 OAM Dying gasp alarm. 
** OAM V1.2 definition: Initiator is about to crash / shutdown
** OAM V2.0 definition: An unrecoverable local failure condition has occurred.
** Alarm parameter: 0
** Alarm data: NULL
**
** Configuration structure: PON_dying_gasp_alarm_configuration_t
*/
typedef int PON_dying_gasp_alarm_data_t;

typedef struct PON_dying_gasp_alarm_configuration_t
{
    unsigned short		nothing;			/*  nothing */
} PON_dying_gasp_alarm_configuration_t;

/* PON_ALARM_CRITICAL_EVENT 
** 
** 802.3 OAM critical event alarm. Indicating that a critical event has occurred.
** Alarm parameter: 0
** Alarm data: NULL
**
** Configuration structure: PON_critical_event_alarm_configuration_t
*/
typedef int PON_critical_event_alarm_data_t;

typedef struct PON_critical_event_alarm_configuration_t
{
    unsigned short		nothing;			/*  nothing */
} PON_critical_event_alarm_configuration_t;


/* PON_ALARM_REMOTE_STABLE
** 
** 802.3 OAM Remote stable alarm. Indicating that Remote DTE has not seen or is unsatisfied with local
** state information
** Alarm parameter: 0
** Alarm data: NULL
**
** Configuration structure: PON_remote_stable_alarm_configuration_t
*/
typedef int PON_remote_stable_alarm_data_t;

typedef struct PON_remote_stable_alarm_configuration_t
{
    unsigned short		nothing;			/*  nothing */
} PON_remote_stable_alarm_configuration_t;

/* PON_ALARM_LOCAL_STABLE
** 
** 802.3 OAM Local stable alarm. Indicating that DTE has not seen or is unsatisfied with remote state 
** information
** Alarm parameter: 0
** Alarm data: NULL
**
** Configuration structure: PON_local_stable_alarm_configuration_t
*/
typedef int PON_local_stable_alarm_data_t;

typedef struct PON_local_stable_alarm_configuration_t
{
    unsigned short		nothing;			/*  nothing */
} PON_local_stable_alarm_configuration_t;

/* PON_ALARM_OAM_VENDOR_SPECIFIC 
** 
** Vendor specific OAM alarm. 
** Alarm parameter: Number of vendor specific alarm (PON_oam_vendor_specific_alarm_t)
** Alarm data: NULL
**
** Configuration structure: PON_oam_vendor_specific_alarm_configuration_t
*/

/* OAM Vendor specific alarms supported 
**
** Note: When new OAM vendor specific alarm is added, its definition has to be added to one of the flag
** types, PON_oam_vendor_specific_alarm_s 
*/
typedef enum 
{
	OAM_ALARM_GLOBAL_EVENT_0 = 0x0,	/* Global event, supported for OAM 1.2 only				 */
	OAM_ALARM_VENDOR_EVENT,			/* Vendor event, supported for OAM 1.2 and OAM 2.0		 */
	OAM_ALARM_VENDOR_ALARM,			/* Vendor alarm, supported for OAM 1.2 and OAM 2.0		 */
	OAM_ALARM_POWER_VOLTAGE_EVENT,	/* Power/Voltage event, supported for OAM 1.2 and OAM 2.0*/
	OAM_ALARM_TEMPERATURE_EVENT,	/* Temperature event, supported for OAM 1.2 and OAM 2.0	 */
	OAM_ALARM_LAST_ALARM			/* Highest value										 */
} PON_oam_vendor_specific_alarm_t;

/* Return a string corresponding to the OAM alarm type */
extern char *PON_oam_vendor_specific_alarm_s[];

typedef struct PON_oam_vendor_specific_alarm_configuration_t
{
    unsigned short		nothing;			/*  nothing */
} PON_oam_vendor_specific_alarm_configuration_t;

/* PON_ALARM_ERRORED_SYMBOL_PERIOD
** 
** 802.3 D2.0 OAM Errored Symbol Period alarm. Indicating that the symbol error count is equal to or 
** greater than a specified threshold for that period.
** Alarm parameter: 0
** Alarm data: as specified in the standard, PON_errored_symbol_period_alarm_data_t
**
** Configuration:
** 
** source: PON_OLT_ID									   - All ONUs in the specified OLT, 
**		   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT - Specific ONU
** configuration structure: PON_errored_symbol_period_alarm_configuration_t;
*/

#define PON_ALARM_ERRORED_SYMBOL_WINDOW_DEFAULT      1000000000 /* MAX: 60,000,000,000 */
#define PON_ALARM_ERRORED_SYMBOL_THRESHOLD_DEFAULT  1 
typedef unsigned short PON_oam_timestamp_t; /* OAM timestamp definition, stated in 100 milliseconds */

/* Representation of 64-bit unsigned integer in ANSI-C  */
/* Integer value is ((msb<<BYTES_IN_SYMBOL)+lsb)        */
typedef struct
{
	unsigned long	msb;
	unsigned long	lsb;
} unsigned__int64;

/* Representation of 64-bit signed integer in ANSI-C  */
/* Integer value is ((msb<<BYTES_IN_SYMBOL)+lsb)      */
typedef struct
{
	long			msb;
	unsigned long	lsb;
} signed__int64;

typedef struct PON_errored_symbol_period_alarm_data_t
{
	PON_oam_timestamp_t  timestamp;
	unsigned__int64      errored_symbol_window;		/* The number of symbols in the period			   */
	unsigned__int64      errored_symbol_threshold;  /* Number of errored symbols in the period that is */
													/* required to be equal to or greater than in order*/
													/* for the alarm to be generated				   */
	unsigned__int64      errored_symbols;			/* The number of symbol errors in the period	   */
	unsigned__int64      error_running_total;		/* The sum of symbol errors accumulated from all   */
													/* Errored Symbol Period Event TLVs that have been */
													/* generated since the OAM sublayer of the Alarm   */
													/* source was reset. This field does not include   */
													/* symbol errors during periods during which the   */
													/* number of symbol errors did not exceed the      */
													/* threshold.									   */
	unsigned long        event_running_total;		/* The number of Errored Symbol Period Event TLVs  */
												    /* that have been generated since the OAM sublayer */
													/* of the Alarm source was reset				   */
} PON_errored_symbol_period_alarm_data_t;

typedef struct PON_errored_symbol_period_alarm_configuration_t
{
	unsigned__int64      errored_symbol_window;		/* The number of symbols in the period			   */
	unsigned__int64      errored_symbol_threshold;  /* Number of errored symbols in the period that is */
													/* required to be equal to or greater than in order*/
													/* for the alarm to be generated				   */
} PON_errored_symbol_period_alarm_configuration_t;

/* PON_ALARM_ERRORED_FRAME 
**
** 802.3 D2.0 OAM Errored Frame alarm. Indicating that errored frame count is equal to or greater than 
** the specified threshold for that period. Errored frames are frames that had transmission errors as
** detected at the Media Access Control sublayer and communicated via the reception_status parameter of
** the MA_DATA.indication service primitive.
** Alarm parameter: 0
** Alarm data: as specified in the standard, PON_errored_frame_alarm_data_t
**
** Configuration:
** 
** source: PON_OLT_ID									   - All ONUs in the specified OLT, 
**		   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT - Specific ONU
** configuration structure: PON_errored_frame_alarm_configuration_t;
*/

#define PON_ALARM_ERRORED_FRAMED_WINDOW  600 /* range : 10--600 */
#define PON_ALARM_ERRORED_FRAMED_THRESHOLD  1

typedef struct PON_errored_frame_alarm_data_t
{
	PON_oam_timestamp_t  timestamp;
	unsigned short		 errored_frame_window;    /* The duration of period in terms of 100 ms intervals */
	unsigned long		 errored_frame_threshold; /* The number of detected errored frames in the period */
												  /* that is required to be equal to or greater than in  */
												  /* order for the alarm to be generated				 */
	unsigned long		 errored_frames;		  /* The number of detected errored frame in the period  */
	unsigned__int64      error_running_total;	  /* The sum of detected errored frames accumulated from */
												  /* all Errored Frame Event TLVs that have been		 */
												  /* generated since the OAM sublayer of the Alarm source*/
												  /* was reset. This field does not include detected     */
												  /* errored frames during periods during which the      */
												  /* number of frame errors did not exceed the threshold.*/
	unsigned long		 event_running_total;	  /* The number of Errored Frame Event TLVs that have    */
												  /* been generated since the OAM sublayer of the Alarm  */
												  /* source was reset									 */
} PON_errored_frame_alarm_data_t;

typedef struct PON_errored_frame_alarm_configuration_t
{
	unsigned short		 errored_frame_window;    /* The duration of period in terms of 100 ms intervals */
	unsigned long		 errored_frame_threshold; /* The number of detected errored frames in the period */
												  /* that is required to be equal to or greater than in  */
												  /* order for the alarm to be generated				 */
} PON_errored_frame_alarm_configuration_t;

/* PON_ALARM_OLT_PORT_BER
**
** High OLT PORT BER value alarm, based on PAS-SOFT internal monitoring mechanism. 
**
*/
typedef PON_ber_t PON_olt_port_ber_alarm_data_t;

typedef struct PON_olt_port_ber_alarm_configuration_t
{
	PON_pon_network_traffic_direction_t  direction; /* Traffic direction to monitor for PON_ALARM_OLT_PORT_BER */
	long double							 ber_threshold; /* Bit error rate measurement threshold above */
														/* which the alarm is raised				  */
														/* Values: 0 - 1							  */
	unsigned long						 minimum_error_bytes_threshold; /* The minimal number of error */
																		/* bytes required for the alarm*/
} PON_olt_port_ber_alarm_configuration_t;

/* PON_ALARM_ERRORED_FRAME_PERIOD
**
** 802.3 D2.0 OAM Errored Frame Period alarm. Indicating that the number of errored frame detected during 
** the specified period exceeded threshold. The period is specified by the number of minFrameSize frames
** that can be received in a time interval on the underlying physical layer. This alarm is generated if 
** the errored frame count is equal to or greater than the specified threshold for that period. Errored 
** frames are frames that had transmission errors as detected at the Media Access Control sublayer and 
** communicated via the reception_status parameter of the MA_DATA.indication service primitive.
** Alarm parameter: 0
** Alarm data: as specified in the standard, PON_errored_frame_period_alarm_data_t
**
** Configuration:
** 
** source: PON_OLT_ID									   - All ONUs in the specified OLT, 
**		   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT - Specific ONU
** configuration structure: PON_errored_frame_period_alarm_configuration_t;
*/

#define PON_ALARM_ERRORED_FRAME_PERIOD_WINDOW  1953125 /* MAX: 117,187,500 */
#define PON_ALARM_ERRORED_FRAME_PERIOD_THRESHOLD  1 

typedef struct PON_errored_frame_period_alarm_data_t
{
	PON_oam_timestamp_t  timestamp;
	unsigned long		 errored_frame_window;   /* The duration of period in terms of frames			 */
	unsigned long		 errored_frame_threshold;/* The number of errored frames in the period that is   */
												 /* required to be equal to or greater than in order for */
												 /* the event to be generated							 */
	unsigned long		 errored_frames;		 /* The number of frame errors in the period			 */
	unsigned__int64      error_running_total;	 /* The sum of frame errors accumulated from all Errored */
												 /* Frame Period Event TLVs that have been generated     */
												 /* since the OAM sublayer of the Alarm source was reset.*/
												 /* This field does not include frame errors during      */
												 /* periods during which the number of frame errors did  */
												 /* not exceed the threshold.							 */
	unsigned long		 event_running_total;	 /* The number of Errored Frame Period Event TLVs that   */
												 /* have been generated since the OAM sublayer of the    */
												 /* Alarm source was reset								 */
} PON_errored_frame_period_alarm_data_t;

typedef struct PON_errored_frame_period_alarm_configuration_t
{
	unsigned long		 errored_frame_window;   /* The duration of period in terms of frames			 */
	unsigned long		 errored_frame_threshold;/* The number of errored frames in the period that is   */
												 /* required to be equal to or greater than in order for */
												 /* the event to be generated							 */

} PON_errored_frame_period_alarm_configuration_t;


/* PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY
**
** 802.3 D2.0 OAM Errored Frame Seconds Summary alarm. Indicating that the number of errored frame seconds
** is equal to or greater than the specified threshold for that period. An errored frame second is a one 
** second interval wherein at least one frame error was detected. Errored frames are frames that had 
** transmission errors as detected at the Media Access Control sublayer and communicated via the 
** reception_status parameter of the MA_DATA.indication service primitive.
** Alarm parameter: 0
** Alarm data: as specified in the standard, PON_errored_frame_seconds_alarm_data_t
**
** Configuration:
** 
** source: PON_OLT_ID									   - All ONUs in the specified OLT, 
**		   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT - Specific ONU
** configuration structure: PON_errored_frame_seconds_alarm_configuration_t;
*/
#define PON_ALARM_ERRORED_FRAME_SECONDS_WINDOW  1
#define PON_ALARM_ERRORED_FRAME_SECONDS_THRESHOLD  600 /* 100 -9000 */

typedef struct PON_errored_frame_seconds_alarm_data_t
{
	PON_oam_timestamp_t  timestamp;
	unsigned short		 errored_frame_seconds_summary_window;    /* The duration of the period in terms*/
																  /* of 100 ms intervals				*/
	unsigned short		 errored_frame_seconds_summary_threshold; /* The number of errored frame seconds*/
																  /* in the period that is required to  */
																  /* be equal to or greater than in     */
																  /* order for the event to be generated*/
	unsigned short		 errored_frame_seconds_summary;			  /* The number of errored frame seconds*/
																  /* in the period						*/
	unsigned long		 error_running_total;					  /* The sum of errored frame seconds   */
																  /* accumulated from all Errored Frame */
																  /*Seconds Summary Event TLVs that have*/
																  /* been generated since the OAM       */
																  /* sublayer of the Alarm source was   */
																  /* reset. This field does not include */
																  /*errored frame seconds during periods*/ 
																  /* during which the number of errored */
																  /* frame seconds did not exceed the   */
																  /* threshold.							*/
	unsigned long		 event_running_total;					  /* The number of Errored Frame Seconds*/
																  /* Summary Event TLVs that have been  */
																  /* generated since the OAM sublayer of*/
																  /* the Alarm source was reset			*/
} PON_errored_frame_seconds_alarm_data_t;

typedef struct PON_errored_frame_seconds_alarm_configuration_t
{
	unsigned short		 errored_frame_seconds_summary_window;    /* The duration of the period in terms*/
																  /* of 100 ms intervals				*/
	unsigned short		 errored_frame_seconds_summary_threshold; /* The number of errored frame seconds*/
																  /* in the period that is required to  */
																  /* be equal to or greater than in     */
																  /* order for the event to be generated*/
} PON_errored_frame_seconds_alarm_configuration_t;

/* PON_ALARM_LLID_MISMATCH 
**
** LLID transmitted frame(s) to the PON when it was not granted.
** Alarm parameter: LLID (values: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT), 
** Alarm data: value of mismatch_dropped field of PON_dropped_frames_per_llid_raw_data_t (unsigned long).
**
** Configuration: 
** 
** source: PON_OLT_ID - configuration applicable for all the LLIDs in the specified OLT
** configuration structure: PON_llid_mismatch_alarm_configuration_t;
*/

typedef unsigned long PON_llid_mismatch_alarm_data_t;

typedef struct PON_llid_mismatch_alarm_configuration_t
{
	unsigned long  llid_mismatch_threshold; /* The minimal number of mismatched frames rate above which */
										    /* PON_ALARM_LLID_MISMATCH alarm is raised. Stated in       */
											/* frames-per-second units								    */
} PON_llid_mismatch_alarm_configuration_t;

#ifdef  PAS_SOFT_VERSION_V5_3_5

/* PON_ALARM_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON 
** 
** Indicating whether ONU laser is permanent on
** Alarm parameter: detect_threshold
** Alarm data: ONU ID
**
** Configuration structure: PON_virtual_scope_onu_laser_always_on_alarm_configuration_t
*/
typedef int PON_virtual_scope_onu_laser_always_on_alarm_data_t;

typedef struct PON_virtual_scope_onu_laser_always_on_alarm_configuration_t
{
    unsigned short	detect_threshold;
} PON_virtual_scope_onu_laser_always_on_alarm_configuration_t;

/* PON_ALARM_VIRTUAL_SCOPE_ONU_SIGNAL_DEGRADATION 
** 
** Indicating whether ONU has a degraded laser signal
** Alarm parameter: detect_threshold
** Alarm data: NULL
**
** Configuration structure: PON_virtual_scope_onu_signal_degradation_alarm_configuration_t
*/
typedef int PON_virtual_scope_onu_signal_degradation_alarm_data_t;

typedef struct PON_virtual_scope_onu_signal_degradation_alarm_configuration_t
{
    unsigned short	detect_threshold;
} PON_virtual_scope_onu_signal_degradation_alarm_configuration_t;


/* PON_ALARM_VIRTUAL_SCOPE_ONU_EOL 
** 
** Indicating whether ONU optics at the end of life
** Alarm parameter: threshold
** Alarm data: NULL
**
** Configuration structure: PON_virtual_scope_onu_eol_alarm_configuration_t
*/
typedef int PON_virtual_scope_onu_eol_alarm_data_t;

typedef struct PON_virtual_scope_onu_eol_alarm_configuration_t
{
    float	threshold;
} PON_virtual_scope_onu_eol_alarm_configuration_t;

#endif

/* 
** Monitoring definitions 
*/

/* OLT monitoring configurations. Each alarm has 'active', several alarms have specific configuration */
/* (e.g. threshold)																					  */

#ifdef  PAS_SOFT_VERSION_V5_3_5
typedef struct
{
	bool												ber_alarm_active;
	PON_ber_alarm_configuration_t						ber_alarm_configuration;
	bool												fer_alarm_active;
	PON_fer_alarm_configuration_t						fer_alarm_configuration;
	bool												software_error_alarm_active;
	bool												local_link_fault_alarm_active;	
	PON_link_fault_alarm_configuration_t				local_link_fault_alarm_configuration;
	bool												dying_gasp_alarm_active;	
	PON_dying_gasp_alarm_configuration_t				dying_gasp_alarm_configuration;
	bool												critical_event_alarm_active;
	PON_critical_event_alarm_configuration_t			critical_event_alarm_configuration;
	bool												remote_stable_alarm_active;
	PON_remote_stable_alarm_configuration_t				remote_stable_alarm_configuration;
	bool												local_stable_alarm_active;
	PON_local_stable_alarm_configuration_t				local_stable_alarm_configuration;
	bool												oam_vendor_specific_alarm_active;
	PON_oam_vendor_specific_alarm_configuration_t		oam_vendor_specific_alarm_configuration;
	bool												errored_symbol_period_alarm_active;
	PON_errored_symbol_period_alarm_configuration_t		errored_symbol_period_alarm_configuration;
	bool												errored_frame_alarm_active;
	PON_errored_frame_alarm_configuration_t				errored_frame_alarm_configuration;
	bool												errored_frame_period_alarm_active;
	PON_errored_frame_period_alarm_configuration_t		errored_frame_period_alarm_configuration;
	bool												errored_frame_seconds_alarm_active;
	PON_errored_frame_seconds_alarm_configuration_t		errored_frame_seconds_alarm_configuration;
	bool												onu_registration_error_alarm_active;
	bool												oam_link_disconnection_alarm_active;
	bool												bad_encryption_key_alarm_active;
	bool												llid_mismatch_alarm_active;
	PON_llid_mismatch_alarm_configuration_t				llid_mismatch_alarm_configuration;
    bool                                                too_many_onus_registering_alarm_active;
	bool												olt_port_ber_alarm_active;
	PON_olt_port_ber_alarm_configuration_t				olt_port_ber_alarm_configuration;
    bool                                                device_fatal_error_alarm_active;
	bool                                                virtual_scope_onu_laser_always_on_alarm_active;
	PON_virtual_scope_onu_laser_always_on_alarm_configuration_t	virtual_scope_onu_laser_always_on_alarm_configuration;
    bool                                                virtual_scope_onu_signal_degradation_alarm_active;
	PON_virtual_scope_onu_signal_degradation_alarm_configuration_t virtual_scope_onu_signal_degradation_alarm_configuration;
    bool                                                virtual_scope_onu_eol_alarm_active;
	PON_virtual_scope_onu_eol_alarm_configuration_t		virtual_scope_onu_eol_alarm_configuration;
#ifdef  PAS_SOFT_VERSION_V5_3_13
    bool                                                onu_registering_with_existing_mac_alarm_active;
#endif
} PON_olt_monitoring_configuration_t;
#else
typedef struct
{
	bool												ber_alarm_active;
	PON_ber_alarm_configuration_t						ber_alarm_configuration;
	bool												fer_alarm_active;
	PON_fer_alarm_configuration_t						fer_alarm_configuration;
	bool												software_error_alarm_active;
	bool												local_link_fault_alarm_active;	
	PON_link_fault_alarm_configuration_t				local_link_fault_alarm_configuration;
	bool												dying_gasp_alarm_active;	
	PON_dying_gasp_alarm_configuration_t				dying_gasp_alarm_configuration;
	bool												critical_event_alarm_active;
	PON_critical_event_alarm_configuration_t			critical_event_alarm_configuration;
	bool												remote_stable_alarm_active;
	PON_remote_stable_alarm_configuration_t				remote_stable_alarm_configuration;
	bool												local_stable_alarm_active;
	PON_local_stable_alarm_configuration_t				local_stable_alarm_configuration;
	bool												oam_vendor_specific_alarm_active;
	PON_oam_vendor_specific_alarm_configuration_t		oam_vendor_specific_alarm_configuration;
	bool												errored_symbol_period_alarm_active;
	PON_errored_symbol_period_alarm_configuration_t		errored_symbol_period_alarm_configuration;
	bool												errored_frame_alarm_active;
	PON_errored_frame_alarm_configuration_t				errored_frame_alarm_configuration;
	bool												errored_frame_period_alarm_active;
	PON_errored_frame_period_alarm_configuration_t		errored_frame_period_alarm_configuration;
	bool												errored_frame_seconds_alarm_active;
	PON_errored_frame_seconds_alarm_configuration_t		errored_frame_seconds_alarm_configuration;
	bool												onu_registration_error_alarm_active;
	bool												oam_link_disconnection_alarm_active;
	bool												bad_encryption_key_alarm_active;
	bool												llid_mismatch_alarm_active;
	PON_llid_mismatch_alarm_configuration_t				llid_mismatch_alarm_configuration;
    bool                                                too_many_onus_registering_alarm_active;
	bool												olt_port_ber_alarm_active;
	PON_olt_port_ber_alarm_configuration_t				olt_port_ber_alarm_configuration;
    bool                                                device_fatal_error_alarm_active;
	bool                                                virtual_scope_onu_laser_always_on_alarm_active;
    bool                                                virtual_scope_onu_signal_degradation_alarm_active;
} PON_olt_monitoring_configuration_t;
#endif

/* == 19  == the end of data struct " alarm details  " ====== */



/* == 20 == the start of data struct " OAM Info  " ====== */
/*
** OAM-standard versions supported
*/

/*            
** OAMPDU configuration - common to all supported standards 
*/
#define MAXIMUM_PDU_SIZE_FIELD_SIZE 11 /* Number of bits allocated for maximum_pdu_size field in      */
									   /* PON_oam_pdu_configuration_t structure	*/

/* OAMPDU configuration field is common to OAM draft V1.2 ,V2.0 ,and V3.3 */
typedef struct
{
/* The largest OAMPDU, in octets, supported by the device. Used for OAMPDU frames size determination. */
/* range: PON_OAM_FRAME_MINIMUM_SIZE - PON_OAM_FRAME_MAXIMUM_SIZE									  */	
	unsigned short  maximum_pdu_size : MAXIMUM_PDU_SIZE_FIELD_SIZE; 
} PON_oam_pdu_configuration_t;

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

/* OAM informaton data flag fields definitions*/

typedef struct
{
	bool	oam_2_0_local_stable;
	bool	oam_2_0_remote_stable;
	bool	oam_2_0_critical_event;
	bool	oam_2_0_dying_gasp;
	bool	oam_2_0_link_fault;
}PON_oam_2_0_flags_t;

/* Vendor Identifier field */
typedef struct
{
	unsigned short  version_identifier;	   /* Identifier which can be used to distinguish between     */
										   /* versions of a particular vendor抯 products. In a case of*/
										   /* Passave ONU - ONU Hardware major & minor versions		  */
	unsigned short  device_identifier;	   /* Identifier which can be used to differentiate a vendor抯*/
										   /* products												  */
	unsigned long   oui;				   /* 24-bit Organizationally Unique Identifier of the vendor.*/
										   /* OUI number list can be found at						  */					  
										   /* standards.ieee.org/regauth/oui/oui.txt				  */
										   /* Passave value defined as								  */
										   /* OAM_INFORMATION_VENDOR_IDENTIFIER_OUI_PASSAVE_VALUE     */
} PON_oam_vendor_identifier_t;

/* OAM TLV containing OAM Information data */
typedef struct
{
	PON_oam_2_0_state_t			 state;
	PON_oam_2_0_configuration_t	 configuration;
	PON_oam_pdu_configuration_t	 pdu_configuration;
	PON_oam_vendor_identifier_t	 vendor_identifier;
} OAM_2_0_tlv_t;


/*
** OAM Flags 
*/
typedef enum 
{
	OAM_2_0_FLAG_LINK_FAULT = 0,			
	OAM_2_0_FLAG_DYING_GASP,			
	OAM_2_0_FLAG_CRITICAL_EVENT,
	OAM_2_0_FLAG_REMOTE_STABLE,
	OAM_2_0_FLAG_LOCAL_STABLE,
	OAM_2_0_FLAG_RESERVE1,	
	OAM_2_0_FLAG_RESERVE2,
	OAM_2_0_FLAG_RESERVE3,
	OAM_2_0_FLAG_RESERVE4,
	OAM_2_0_FLAG_VENDOR_ALARM,
	OAM_2_0_FLAG_POWER_VOLTAGE_EVENT,
	OAM_2_0_FLAG_TEMPERATURE_EVENT,
	OAM_2_0_FLAG_VENDOR_EVENT,
	OAM_2_0_FLAG_RESERVE5,	
	OAM_2_0_FLAG_RESERVE6,
	OAM_2_0_FLAG_RESERVE7,
	OAM_2_0_FLAG_LAST_FLAG 
} OAM_2_0_flag_t;

/*
** OAM Flags field masks
**
*/
#define OAM_2_0_FLAG_LINK_FAULT_MASK									0x0001	/* link fault has been detected in the local device. If a link fault is detected, a 1 Is encoded.										 */
#define OAM_2_0_FLAG_DYING_GASP_MASK									0x0002	/* This flag indicates an unrecoverable failure condition has occurred and is an attempt to alert the far-end to the condition.			 */
#define OAM_2_0_FLAG_CRITICAL_EVENT_MASK								0x0004  /* OAM alarm, indicating that a critical event has occurred.																			 */
#define OAM_2_0_FLAG_REMOTE_STABLE_MASK									0x0008  /* OAM alarm, indicating that Remote DTE has not seen or is unsatisfied with local state information.									 */
#define OAM_2_0_FLAG_LOCAL_STABLE_MASK									0x0010  /* OAM alarm, indicating that Indicating that DTE has not seen or is unsatisfied with remote state information							 */ 
#define OAM_2_0_FLAG_VENDOR_ALARM_MASK									0x0200	/* Vendor specific vendor alarm mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */
#define OAM_2_0_FLAG_POWER_VOLTAGE_EVENT_MASK							0x0400	/* Vendor specific power or voltage event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.   */
#define OAM_2_0_FLAG_TEMPERATURE_EVENT_MASK								0x0800	/* Vendor specific temperature event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.		 */
#define OAM_2_0_FLAG_VENDOR_EVENT_MASK									0x1000	/* Vendor specific vendor event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */

/* 
** OAM state
*/
typedef enum
{
	OAM_STATE_UNSTABLE	= 0x0,	/* Local device either has not seen or is unsatisfied with remote state information */
	OAM_STATE_STABLE	= 0x1	/* Local device has seen and is satisfied with remote state information				*/
} OAM_stable_t;

/* Loopback state definition */
/* Note: when updated - also update OAM_loopback_state_s */
typedef enum
{
	OAM_LOOPBACK_STATE_OFF  = 0x0, /* Local device is not in loopback mode							   */
	OAM_LOOPBACK_STATE_ON   = 0x1, /* Local device抯 lb_timer is non-zero							   */
	OAM_LOOPBACK_STATE_DONE = 0x2, /* Local device is waiting to return to LB_OFF state as the local   */
								   /* device抯 lb_timer has recently expired						   */
	OAM_LOOPBACK_STATE_REQ  = 0x3, /* Local device has requested that the remote device enter loopback */
								   /* mode															   */
	OAM_LOOPBACK_STATE_EXIT = 0x4  /* Remote device has just exited loopback mode					   */
} OAM_loopback_state_t;

extern char *OAM_loopback_state_s[];

typedef struct
{
	OAM_stable_t			stable;
	OAM_loopback_state_t	loopback_state;
} OAM_1_2_state_t;

/* OAM Configuration definitions */
typedef enum
{
	OAM_PASSIVE_MODE	= 0x0,	/* Device is not permitted to send Loopback Control, and Variable     */
								/* Request OAMPDUs													  */
	OAM_ACTIVE_MODE	    = 0x1	/* Device is permitted to send any OAMPDU							  */
} PON_oam_1_2_mode_t;

typedef enum
{
	OAM_UNIDIRECTIONAL_SUPPORT_DISABLE = 0x0,	/* Device is not capable of sending OAMPDUs when the  */
												/* receive path is non-operational					  */
	OAM_UNIDIRECTIONAL_SUPPORT_ENABLE  = 0x1	/* Device is capable of sending OAMPDUs when the      */
												/* receive path is non-operational					  */
} PON_oam_1_2_unidirectional_support_t;

typedef enum
{
	OAM_1_2_LOOPBACK_NOT_SUPPORTED  = 0x0, /* Device is not capable of OAM Loopback mode */
	OAM_1_2_LOOPBACK_SUPPORTED		= 0x1  /* Device is capable of OAM Loopback mode	 */
} PON_oam_1_2_loopback_support_t;

typedef struct
{
	PON_oam_1_2_mode_t				      mode;
	PON_oam_1_2_unidirectional_support_t  unidirectional_support;
	PON_oam_1_2_loopback_support_t		  loopback_support;
} PON_oam_1_2_configuration_t;

#define PON_OAM_CONFIGURATION_COPY(dst_structure, src_structure)\
	dst_structure.mode					 = src_structure.mode;\
	dst_structure.unidirectional_support = src_structure.unidirectional_support;\
	dst_structure.loopback_support		 = src_structure.loopback_support;


/* OAM extension definition */
typedef struct
{
	unsigned long   enterprise_identifier; /* 32-bit IANA Private Enterprise Number.				  */
										   /* Enterprise number list can be found at				  */					  
										   /* www.iana.org/assignments/enterprise-numbers			  */
										   /* Passave value defined as								  */
										   /* OAM_INFORMATION_EXTENSION_PRIVATE_ENTERPRISE_NUMBER_PASSAVE_VALUE */
	unsigned short  device_identifier;	   /* Identifier which can be used to differentiate a vendor抯*/
										   /* products												  */
	unsigned short  version_identifier;	   /* Identifier which can be used to distinguish between     */
										   /* versions of a particular vendor抯 products. In a case of*/
										   /* Passave ONU - ONU Hardware major & minor versions		  */
} PON_oam_1_2_extension_t;

#define PON_OAM_EXTENSION_COPY(dst_structure, src_structure)\
	dst_structure.enterprise_identifier	= src_structure.enterprise_identifier;\
	dst_structure.device_identifier	    = src_structure.device_identifier;\
	dst_structure.version_identifier	= src_structure.version_identifier;


/* OAM TLV containing OAM Information data */
typedef struct
{
	OAM_1_2_state_t				 state;
	PON_oam_1_2_configuration_t	 configuration;
	PON_oam_pdu_configuration_t	 pdu_configuration;
	PON_oam_1_2_extension_t		 extension;
} OAM_1_2_tlv_tuple_t;

/*
** OAM Flags field values
**
** Note: When a new flag is added, its mask has to be added to OAM_internal.h, parsing added to Parse_oam_frame_flags in OAM.c
**
*/
typedef enum /* OAM flags supported by ONU hardware */
{
	OAM_FLAG_LINK_FAULT  = 0x0,			
	OAM_FLAG_DYING_GASP,			
	OAM_FLAG_GLOBAL_EVENT_0,		
	OAM_FLAG_VENDOR_EVENT,		
	OAM_FLAG_VENDOR_ALARM,		
	OAM_FLAG_POWER_VOLTAGE_EVENT,	
	OAM_FLAG_TEMPERATURE_EVENT,	
	OAM_FLAG_RESERVE1,			
	OAM_FLAG_RESERVE2,			
	OAM_FLAG_LAST_FLAG
} OAM_1_2_flag_t;


/* OAM flags structure												    	 */
/* Implementation: ENABLE / DISABLE value in OAM flag index (OAM_1_2_flag_t) */
typedef bool OAM_1_2_flags_t[OAM_FLAG_LAST_FLAG]; 

/* Macro for a loop going over the flags in OAM_1_2_flags_t struct */
#define OAM_FLAGS_BOUNDS_LOOP( __flag_id ) for (__flag_id =0;__flag_id<OAM_FLAG_LAST_FLAG;__flag_id++)

/* Macro for zeroing OAM flags structure */
/* flags - parameter of OAM_1_2_flags_t type */
#define OAM_ZERO_FLAGS_STRUCT( flags ) \
	{\
		OAM_1_2_flag_t _flag__id_;\
		OAM_FLAGS_BOUNDS_LOOP(_flag__id_) flags[_flag__id_] = DISABLE;\
	}


/*
** OAM Flags field masks
**
** Note: When a new flag is added, update Parse_oam_flags function, OAM_1_2_flag_t 
*/
#define OAM_FLAG_LINK_FAULT_MASK																		0x0001	/* link fault has been detected in the local device. If a link fault is detected, a 1 Is encoded.										 */
																												/* If no Local Link Fault has been detected a 0 is encoded.																				 */
#define OAM_FLAG_DYING_GASP_MASK																		0x0002	/* This flag indicates an unrecoverable failure condition has occurred and is an attempt to alert the far-end to the condition.			 */
																												/* A Dying Gasp is encoded as a 1. When no Dying Gasp is being communicated a 0 is encoded.												 */
#define OAM_FLAG_GLOBAL_EVENT_0_MASK																	0x0004	/* Vendor specific global event number 0. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.		 */
#define OAM_FLAG_VENDOR_EVENT_MASK																		0x0008  /* Vendor specific vendor event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */
#define OAM_FLAG_VENDOR_ALARM_MASK																		0x0010  /* Vendor specific vendor alarm mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */
#define OAM_FLAG_POWER_VOLTAGE_EVENT_MASK																0x0020	/* Vendor specific power or voltage event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.   */
#define OAM_FLAG_TEMPERATURE_EVENT_MASK																	0x0040  /* Vendor specific temperature event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.		 */
#define OAM_FLAG_RESERVE1_MASK																			0x0080  /* Vendor specific reserved number 1 event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.  */
#define OAM_FLAG_RESERVE2_MASK



/* Map OAM 2.0 tpyes to OAM 3.3 becuase there is no changes in the defenition */ 
typedef PON_oam_2_0_state_t                     PON_oam_3_3_state_t;

typedef PON_oam_2_0_loopback_support_t          PON_oam_3_3_loopback_support_t;

typedef PON_oam_2_0_unidirectional_support_t    PON_oam_3_3_unidirectional_support_t;

typedef PON_oam_status_2_0_mode_t               PON_oam_status_3_3_mode_t;

typedef PON_oam_2_0_mode_t                      PON_oam_3_3_mode_t;

typedef struct
{
	PON_oam_variable_retrieval_t					 variable_retrieval;
	PON_oam_link_events_t							 link_events;
	PON_oam_3_3_loopback_support_t					 loopback_support;
	PON_oam_3_3_unidirectional_support_t			 unidirectional_support;
	PON_oam_3_3_mode_t								 oam_mode;
} PON_oam_3_3_configuration_t;

/* OAM informaton data flag fields definitions*/

typedef struct
{
	bool    oam_3_3_remote_evaluating;
	bool	oam_3_3_remote_stable;
	bool	oam_3_3_local_stable;
	bool    oam_3_3_local_evaluating;
    bool	oam_3_3_critical_event;
	bool	oam_3_3_dying_gasp;
	bool	oam_3_3_link_fault;

}PON_oam_3_3_flags_t;


/* Vendor Identifier field */
typedef struct
{
	unsigned long   identifier ;	       /* Identifier which can be used to distinguish between     */
										   /* versions of a particular vendor抯 products. In a case of*/
										   /* Passave ONU - ONU Hardware major & minor versions	also  */
	                                	   /* can be used to differentiate a vendor抯 products        */
										   
	unsigned long   oui;				   /* 24-bit Organizationally Unique Identifier of the vendor.*/
										   /* OUI number list can be found at						  */					  
										   /* standards.ieee.org/regauth/oui/oui.txt				  */
										   /* Passave value defined as								  */
										   /* OAM_INFORMATION_VENDOR_IDENTIFIER_OUI_PASSAVE_VALUE     */
} PON_oam_3_3_vendor_identifier_t;

/* OAM TLV containing OAM Information data */
typedef struct
{
	PON_oam_3_3_state_t			        state;
	PON_oam_3_3_configuration_t	        configuration;
	PON_oam_pdu_configuration_t	        pdu_configuration;
	PON_oam_3_3_vendor_identifier_t	    vendor_identifier;
} OAM_3_3_tlv_t;

/*
** OAM Flags 
*/
typedef enum 
{
	OAM_3_3_FLAG_LINK_FAULT = 0,			
	OAM_3_3_FLAG_DYING_GASP,			
	OAM_3_3_FLAG_CRITICAL_EVENT,
	OAM_3_3_FLAG_LOCAL_EVALUATING,
	OAM_3_3_FLAG_LOCAL_STABLE,
	OAM_3_3_FLAG_REMOTE_EVALUATING,
    OAM_3_3_FLAG_REMOTE_STABLE,
	OAM_3_3_FLAG_RESERVE1,	
	OAM_3_3_FLAG_RESERVE2,
	OAM_3_3_FLAG_VENDOR_ALARM,
	OAM_3_3_FLAG_POWER_VOLTAGE_EVENT,
	OAM_3_3_FLAG_TEMPERATURE_EVENT,
	OAM_3_3_FLAG_VENDOR_EVENT,
	OAM_3_3_FLAG_RESERVE3,	
	OAM_3_3_FLAG_RESERVE4,
	OAM_3_3_FLAG_RESERVE5,
	OAM_3_3_FLAG_LAST_FLAG 
} OAM_3_3_flag_t;

/*
** OAM Flags field masks
**
*/
#define OAM_3_3_FLAG_LINK_FAULT_MASK									0x0001	/* link fault has been detected in the local device. If a link fault is detected, a 1 Is encoded.										 */
#define OAM_3_3_FLAG_DYING_GASP_MASK									0x0002	/* This flag indicates an unrecoverable failure condition has occurred and is an attempt to alert the far-end to the condition.			 */
#define OAM_3_3_FLAG_CRITICAL_EVENT_MASK								0x0004  /* OAM alarm, indicating that a critical event has occurred.																			 */

#define OAM_3_3_FLAG_LOCAL_EVALUATING_MASK								0x0008  /* Local Stable and Local Evaluating form a two-bit encoding shown                                                                       */
#define OAM_3_3_FLAG_LOCAL_STABLE_MASK									0x0010  /*   4:3                                                                                                                                 */
                                                                                /* 0x0 = Local DTE Unsatisfied, Discovery can not complete                                                                               */
                                                                                /* 0x1 = Local DTE Discovery process has not completed                                                                                   */
                                                                                /* 0x2 = Local DTE Discovery process has completed                                                                                       */
                                                                                /* 0x3 = Reserved. This value shall not be sent. If the value 0x3 is                                                                     */
                                                                                /* received, it should be ignored and not change the last received                                                                       */
                                                                                /* value.                                                                                                                                */

#define OAM_3_3_FLAG_REMOTE_EVALUATING_MASK 							0x0020  /* 5:6 , When remote_state_valid is set to TRUE, the Remote Stable and                                                                   */
#define OAM_3_3_FLAG_REMOTE_STABLE_MASK									0x0040  /* Remote Evaluating values shall be a copy of the last received Local                                                                   */
                                                                                /* Stable and Local Evaluating values from the remote OAM peer. Otherwise,                                                               */
                                                                                /* the Remote Stable and Remote Evaluating bits shall be set to 0.                                                                       */

#define OAM_3_3_FLAG_VENDOR_ALARM_MASK									0x0200	/* Vendor specific vendor alarm mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */
#define OAM_3_3_FLAG_POWER_VOLTAGE_EVENT_MASK							0x0400	/* Vendor specific power or voltage event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.   */
#define OAM_3_3_FLAG_TEMPERATURE_EVENT_MASK								0x0800	/* Vendor specific temperature event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.		 */
#define OAM_3_3_FLAG_VENDOR_EVENT_MASK									0x1000	/* Vendor specific vendor event mask. Flag is encoded as a 1. When no such indication is being communicated, a 0 is encoded.			 */


/* OAM MDIO PHY status parameters
**
** phy_extended_register_capability - PHY MDIO R1.0, FALSE - PHY extended register capability not support
**													 TRUE  - PHY extended register capability supported
** phy_auto_negotiation_ability -  PHY MDIO R1.3, FALSE - PHY auto-negotiation ability not supported
**												  TRUE  - PHY auto-negotiation ability supported
** remote_fault - PHY MDIO R1.4, FALSE - Remote fault condition not detected
**								 TRUE  - Remote fault condition detected
** auto_negotiation_complete - PHY MDIO R1.5, FALSE - Auto-negotiation process not completed 
**											  TRUE  - Auto-negotiation process completion 
** phy_advertisement - PHY MII and GMII advertisement data, MII: MDIO R4.12:5 - PHY's TAF
**					   GMII: MDIO R9.8-9.10
** link_partner_advertisement -  Link partner PHY MII and GMII advertisement data.
**								 MII: PHY MDIO R5.12:5, PHY's partner TAF
**								 GMII: MDIO R10.10-10.11 (Not including preferable_port_type)
** link_partner_is_auto_negotiation_capable - PHY MDIO R6.0, 
**											  FALSE - PHY link partner isn't auto-negotiation capable
**											  TRUE  - PHY link partner is auto-negotiation capable
** phy_master_slave_manual_mode_setting - PHY MDIO R9.11, PON_SLAVE  - Manual configure as slave
**														  PON_MASTER - Manual configure as master
** phy_master_slave_manual_mode - PHY MDIO R9.12, DISABLE - Automatic PHY master/slave configuration
**												  ENABLE  - Manual PHY master/slave configuration 
** master_slave_configuration_resolution - PHY MDIO R10.14, 
**										   PON_SLAVE  - PHY Master/slave configuration resolved to slave
**										   PON_MASTER - PHY Master/slave configuration resolved to master
** master_slave_configuration_fault - PHY MDIO R10.15, FALSE - Master/slave configuration OK
**													   TRUE  - Master/slave configuration fault detected
*/
typedef struct PON_oam_phy_status_t
{
	bool						 phy_extended_register_capability;					
	bool						 phy_auto_negotiation_ability;					
	bool						 remote_fault;	
	bool						 auto_negotiation_complete;					
	PON_oam_phy_advertisement_t	 phy_advertisement;			
	PON_oam_phy_advertisement_t	 link_partner_advertisement; /* Not including preferable_port_type (which is not available) */ 	
	bool						 link_partner_is_auto_negotiation_capable;
	PON_master_slave_t			 phy_master_slave_manual_mode_setting;
	bool						 phy_master_slave_manual_mode;					
	PON_master_slave_t			 master_slave_configuration_resolution;	
	bool						 master_slave_configuration_fault;
} PON_oam_phy_status_t;

/* 
** Collection of UNI (System) port MAC parameters
**
** uni_port_mac_type - UNI (System) Port MAC type
** uni_port_mii_type_rate - UNI Port MII MAC rate, valid only if uni_port_mac_type == PON_MII
**		values: enumerated values, not including PON_1000M. MII MAC type is	composed of rate and duplex
**		parameters. Valid only if (uni_port_mac_type == PON_MII) 
** uni_port_mii_type_duplex - UNI Port MII MAC duplex,valid only if uni_port_mac_type == PON_MII
**		values: enumerated values. MII MAC type is composed of rate and duplex. Valid only if 
**		(uni_port_mac_type == PON_MII) 
** uni_autonegotiation - ENABLE / DISABLE auto-negotiation in UNI port
** uni_port_link - Link exists at UNI port (values: TRUE / FALSE)	
** uni_master - Master / slave mode in UNI port, values: PON_MASTER - regular MAC-PHY mode
**														 PON_SLAVE  - MAC disguise as PHY 
** auto_negotiation_resolution_error - Whether auto-negotiation process completed successfully.
**		Originating from 'Resolution Error' - MDIO Master state machine.
**		values: TRUE  - Auto-negotiation process completed successfully, link partner is 802.3 compliant
**				FALSE - Error during auto-negotiation process
**
** resolution_master - Resolution Master from MDIO Master state machine, values: PON_MASTER / PON_SLAVE
**
** resolution_pause_Rx - Resolution Pause Rx from MDIO Master state machine, values: ENABLE / DISABLE
**
** resolution_pause_Tx - Resolution Pause Tx from MDIO Master state machine, values: ENABLE / DISABLE
**
*/
typedef struct PON_oam_uni_port_mac_status_t
{
	PON_mac_t			 uni_port_mac_type;
	PON_link_rate_t		 uni_port_mii_type_rate;
	PON_duplex_status_t  uni_port_mii_type_duplex;
	bool				 uni_autonegotiation;	
	bool			 	 uni_port_link;			
	PON_master_slave_t	 uni_master;				
	bool				 auto_negotiation_resolution_error;	
	PON_master_slave_t	 resolution_master;
	bool				 resolution_pause_Rx;
	bool				 resolution_pause_Tx;
} PON_oam_uni_port_mac_status_t;

/*
** OAM Auto-negotiation information
**
** This is Passave specific Auto-negotiation information definition
**
** vendor_code - 24-bit ONU board vendor code casted into long, range: 0 - (2^24-1) 
** model_number - 24-bit ONU board model number casted into long, range: 0 - (2^24-1)
** uni_port_phy_status - PHY status structure, valid only if UNI port MAC type is MII / GMII  
**					     ((uni_port_mac_status.uni_port_mac_type == PON_MII) || 
**					      (uni_port_mac_status.uni_port_mac_type == PON_GMII))
** uni_port_mac_status - UNI (System) port MAC status structure
*/
typedef struct PON_oam_auto_negotiation_data_t
{
	long						   vendor_code;	
	long						   model_number;	
	PON_oam_phy_status_t		   uni_port_phy_status;
	PON_oam_uni_port_mac_status_t  uni_port_mac_status;
} PON_oam_auto_negotiation_data_t;
#if 0

/*
** OAM Auto-negotiation information
**
** This is Passave specific Auto-negotiation information definition
**
** vendor_code - 24-bit ONU board vendor code casted into long, range: 0 - (2^24-1) 
** model_number - 24-bit ONU board model number casted into long, range: 0 - (2^24-1)
** uni_port_phy_status - PHY status structure, valid only if UNI port MAC type is MII / GMII  
**					     ((uni_port_mac_status.uni_port_mac_type == PON_MII) || 
**					      (uni_port_mac_status.uni_port_mac_type == PON_GMII))
** uni_port_mac_status - UNI (System) port MAC status structure
*/
typedef struct PON_oam_auto_negotiation_data_t
{
	long						   vendor_code;	
	long						   model_number;	
	PON_oam_phy_status_t		   uni_port_phy_status;
	PON_oam_uni_port_mac_status_t  uni_port_mac_status;
} PON_oam_auto_negotiation_data_t;
#endif
/*
** OAM-standard versions supported
*/
typedef enum 
{
	OAM_STANDARD_VERSION_1_2  = 0x1, /* OAM standard according to 802.3ah draft 1.2 */
	OAM_STANDARD_VERSION_2_0  = 0x2, /* OAM standard according to 802.3ah draft 2.0 */
    OAM_STANDARD_VERSION_3_3  = 0x3  /* OAM standard according to 802.3ah Standard  */
} OAM_standard_version_t;

typedef struct PON_oam_information_t
{	OAM_standard_version_t			  oam_information_reference_standard;/* The standard according to which the OAM information is received			*/
	void							 *oam_standard_information;			/* 802.3ah D1.2 / D2.0 /D3.3 OAM standard information						*/
																		/* pointer should be casted into (OAM_1_2_tlv_tuple_t*) or (OAM_2_0_tlv_t*) */
																		/* or (OAM_3_3_tlv_t) according to the ONU OAM version	            		*/
	bool							  passave_originated;				/* Is the OAM information originated from Passave ONU, values: TRUE, FALSE	*/
	PON_oam_auto_negotiation_data_t   passave_specific_oam_information; /* Passave vendor specific OAM extension									*/
																	    /* 																			*/
} PON_oam_information_t;


/* == 20 == the end of data struct " OAM Info  " ====== */


/* == 21 == the start of data struct " address table  " ====== */
/* Indication of address table record originated from the OLT CNI port 
#define PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID  PON_PORT_ID_CNI*/

/* Aging (address aging) modes */
typedef enum
{  
	ADDR_DYNAMIC,
	ADDR_STATIC,
	ADDR_DYNAMIC_AND_STATIC /* relevant in sending reset and not when geting address table*/
} PON_address_aging_t;

extern char *PON_address_aging_s ( PON_address_aging_t  address_aging );


#define FSEEK_SUCCESS 0	 /* ANSI */

#define PON_MAX_ADDRESS_TABLE_AGING_TIMER        (86400000) /* 24 hours -> milliseconds  */
#define PON_MIN_ADDRESS_TABLE_AGING_TIMER        (5000)     /* 5 seconds -> milliseconds */
#define PON_ADDRESS_TABLE_AGING_TIMER_RESOLUTION (5000)		/* 5 seconds -> milliseconds */
/* Indication of address table record originated from the OLT CNI port */
#define PAS_ADDRESS_TABLE_RECORD_SYSTEM_LLID  PAS_PORT_ID_CNI
/*Begin: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#define GW10G_PAS_ADDRESS_TABLE_RECORD_SYSTEM_LLID  GW10G_PAS_PORT_ID_CNI
/*End: for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
#define PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID  PON_PORT_ID_CNI

/* Address table aging modes */
typedef PON_address_aging_t		PON_address_record_aging_t;

/* Address table record	*/
typedef struct PON_address_record_t
{
	mac_address_t			    mac_address; /* MAC address											  */
	short int					logical_port;/* The logical port of the MAC address. Values:		  */
											 /* LLID:     PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT */
											 /* CNI port: PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID		  */
	PON_address_record_aging_t  type;		 /* Record type (only static or dynamic)						  */
} PON_address_record_t;

typedef struct PON_address_table_entry_t
{
    bool                  found       ;
    PON_address_record_t  addr_record ;
    
} PON_address_table_entry_t;

/* Address table */
typedef PON_address_record_t    PON_address_table_t[PON_ADDRESS_TABLE_SIZE];

typedef struct PON_address_vlan_record_t
{
	mac_address_t			    mac_address; /* MAC address											  */
	short int					logical_port;/* The logical port of the MAC address. Values:		  */
											 /* LLID:     PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT */
											 /* CNI port: PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID		  */
    short int vid;
	PON_address_record_aging_t  type;		 /* Record type (only static or dynamic)						  */
} PON_address_vlan_record_t;
typedef PON_address_vlan_record_t PON_address_vlan_table_t[PON_ADDRESS_TABLE_SIZE];/*For GPON 获取每个address所对应的vlan*/
/* == 20 == the end of data struct " address table  " ====== */

/* == 21 == the start of data struct " policing definitions  " ====== */

/*
** Policing definitions 
*/
typedef enum
{
	PON_POLICER_DOWNSTREAM_TRAFFIC,		  /* Downstream traffic (destined to a LLID) policer	   */
	PON_POLICER_P2P_TRAFFIC				  /* P2P (peer-to-peer) traffic (sent from a LLID) policer */
} PON_policer_t;

#define PON_POLICING_HIGH_PRIORITY_FRAME	4	     /* Scale according to 802.1p queues standard  */

/* Policing configurations for hardware policer */
#define PON_POLICING_MAXIMAL_BURST_SIZE 8388480

typedef struct PON_policing_struct_t
{
	long  maximum_bandwidth;			   /* Maximum policer bandwidth allowed for an ONU.			   */
										   /* Measured in Kbits/Sec.								   */
									       /* range: 0 - (PON_NETWORK_RATE / KILO)					   */
	long  maximum_burst_size;			   /* Maximal number of bytes in a burst, measured in Bytes.   */
										   /* Range: 0 - PON_POLICING_MAXIMAL_BURST_SIZE. The field    */
										   /* value is truncated to the nearest smaller 128bytes	   */
										   /* multiple value (hardware register counts in 128 bytes    */
										   /* units)						 						   */
	bool  high_priority_frames_preference; /* Enable / disable preference for high priority			   */
										   /* (priority >= PON_POLICING_HIGH_PRIORITY_FRAME) frames.   */
										   /* Values: ENABLE / DISABLE								   */
	bool  short_frames_preference;		   /* Enable / disable preference for short frames.			   */
										   /* Values: ENABLE / DISABLE								   */
} PON_policing_struct_t;

/* 
** P2P (peer-to-peer) policy definitions
*/
#define PON_P2P_POLICY_UNKNOWN_ADDRESSES (-1) /* All unknown MAC addresses			 */
#define PON_P2P_POLICY_ALL_LLIDS         (-1) /* Every LLID, except originating	LLID */

/* 
** Policing config structure
**
** Parameters:
**
**      maximum_bandwidth                 : Maximum policing bandwidth allowed for an ONU.,Measured in Kbits/Sec.            
**                                          range: PON_POLICING_MIN_BANDWIDTH_SIZE - PON_POLICING_MAX_BANDWIDTH_SIZE					   
**      maximum_burst_size                : Maximal number of bytes in a burst, measured in Bytes.
**                                          range: PON_POLICING_MIN_BURST_SIZE - PON_POLICING_MAX_BURST_SIZE. 
**      high_priority_frames_preference   : Enable / disable preference for high priority
**                                          range: DISABLE..ENABLE            
*/ 
typedef struct PON_policing_parameters_t
{
	long  maximum_bandwidth;			  
									      
	long  maximum_burst_size;			  
										  
                                                           
	bool  high_priority_frames_preference;                 
										                   
} PON_policing_parameters_t;

/* == 21 == the end of data struct " policing definitions  " ====== */

/* == 22 == the start of data struct " onu info  " ====== */

/* Device capabilities structure */
typedef struct PAS_physical_capabilities_t
{
	unsigned short int  laser_on_time;	     /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	unsigned short int  laser_off_time;      /* Measured in TQ,relevant to ONU only, range:0 - (2^16-1)*/
	unsigned char		onu_grant_fifo_depth;/* Max number of grant records an ONU can store,		   */
										     /* relevant to ONU only, range: 1 - MAXUCHAR			   */
	short int			agc_lock_time;		 /* PON upstream data AGC lock time, measured in TQ,	   */
											 /* relevant to OLT only, range: 1 - 2^15				   */
	short int			cdr_lock_time;		 /* PON RX signal synchronization time, measured in TQ,	   */
											 /* relevant to OLT only, range: 1 - 2^15				   */
	short int			pon_tx_signal;		 /* PON TX signal                                          */
} PAS_physical_capabilities_t;

/* Structure containing the ONU originated data updated during ONU registration to the network. */
/* This data doesn't include ONU identifications: Id and MAC address							*/
typedef struct
{
	unsigned short int		laser_on_time;	/* Measured in TQ, range: 0 - (2^16-1) */
	unsigned short int		laser_off_time; /* Measured in TQ, range: 0 - (2^16-1) */
	OAM_standard_version_t  oam_version;
	bool					passave_onu;    /* Device contains one of Passave's ONU chips */
	PON_rtt_t				rtt;
#ifdef PAS_SOFT_VERSION_V5_3_9
	    unsigned char           session_id;
#endif
} onu_registration_data_record_t;

#define PON_DEFAULT_TIMEOUT_POLLING_CHECK	10   /* Default polling time for timeout code, stated in Milliseconds */

typedef enum
{
    PON_PASSAVE_ONU_6001_VERSION = 0x6001,
    PON_PASSAVE_ONU_6201_VERSION = 0x6201,
    PON_PASSAVE_ONU_6301_VERSION = 0x6301,
#ifdef  PAS_SOFT_VERSION_V5_3_9
    PON_PASSAVE_ONU_6300_VERSION = 0x6300,	
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
    PON_PASSAVE_ONU_6401_VERSION = 0x6401,
    PON_PASSAVE_ONU_6402_VERSION = 0x6402,
    PON_PASSAVE_ONU_6403_VERSION = 0x6403,
    PON_PASSAVE_ONU_6501_VERSION = 0x6501,
    PON_PASSAVE_ONU_6502_VERSION = 0x6502,
    PON_PASSAVE_ONU_6503_VERSION = 0x6503,
#endif
    /*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    GW10G_PON_PASSAVE_ONU_9001_VERSION = 0x9001,
    GW10G_PON_PASSAVE_ONU_9011_VERSION = 0x9011,
    /*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
    PON_PASSAVE_ONU_NOT_SUPPORTED_VERSION = 0x0,
    PON_PASSAVE_ONU_INIT_VERSION          = -1 /* to support short int function */
}PON_onu_hardware_version_t;

typedef enum
{
    PON_PASSAVE_DEVICE_5001_VERSION = 0x5001,
    PON_PASSAVE_DEVICE_5201_VERSION = 0x5201,
    PON_PASSAVE_DEVICE_6001_VERSION = 0x6001,
    PON_PASSAVE_DEVICE_6201_VERSION = 0x6201,
    PON_PASSAVE_DEVICE_6301_VERSION = 0x6301,
#ifdef  PAS_SOFT_VERSION_V5_3_9
    PON_PASSAVE_DEVICE_6300_VERSION = 0x6300,	
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
    PON_PASSAVE_DEVICE_6401_VERSION = 0x6401,
    PON_PASSAVE_DEVICE_6402_VERSION = 0x6402,
    PON_PASSAVE_DEVICE_6403_VERSION = 0x6403,
    PON_PASSAVE_DEVICE_6501_VERSION = 0x6501,
    PON_PASSAVE_DEVICE_6502_VERSION = 0x6502,
    PON_PASSAVE_DEVICE_6503_VERSION = 0x6503,
#endif

    PON_PASSAVE_DEVICE_NOT_SUPPORTED_VERSION = 0x0,

}PON_device_hardware_version_t;

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

/* ONU parameters record */
typedef struct PAS_onu_parameters_record_t
{
	mac_address_t		mac_address;	/* ONU MAC address, valid unicast MAC address value			   */
	PON_llid_t			llid;			/* ONU id in current version,								   */
									    /* range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT		   */
	PON_rtt_t			rtt;			/* ONU Round Trip Time, measured in TQ						   */
} PAS_onu_parameters_record_t;


/* ONU parameters data structure */
typedef PAS_onu_parameters_record_t PAS_onu_parameters_t [PAS_MAX_ONU_ID_PER_OLT - PAS_MIN_ONU_ID_PER_OLT +1];
typedef PAS_onu_parameters_record_t PON_onu_parameters_t [PON_MAX_ONU_ID_PER_OLT - PON_MIN_ONU_ID_PER_OLT +1];


/* ONU register data structure */
typedef struct
{
	PON_onu_id_t				    onu_id;
	mac_address_t				    onu_mac;
	unsigned short int			    laser_on_time;
	unsigned short int			    laser_off_time;
	PON_authentication_sequence_t   authentication_sequence;
	OAM_standard_version_t		    oam_version;
	bool						    passave_originated;
	PON_rtt_t					    rtt;
    unsigned long int	            critical_events_counter;    
    unsigned long int               timestamp;
    unsigned short int              device_info_retrieved;
	unsigned short int	            device_id;				/* From OAM Information TLV vendor specific inforamtion (device_identifier) */
	unsigned short int	            hw_version;				/* From OAM Information TLV vendor specific inforamtion (version_identifier)*/
	unsigned short int	            sw_version_major;	   
	unsigned short int	            sw_version_minor;
	unsigned short int	            sw_version_build;
    unsigned short int	            maintenance_onu_fw_version;
    unsigned short int	            fw_remote_mgnt_major_version;	   
    unsigned short int	            fw_remote_mgnt_minor_version; 
    unsigned char                   session_id;
    unsigned char                   rdnd_mode;
	bool						    host_request;
} PASCOMM_event_onu_registration_t;


/* == 22 == the end of data struct " onu info  " ====== */

/*
** ONU deregistration reason
*/ 
typedef enum
{  
	PON_ONU_DEREGISTRATION_REPORT_TIMEOUT = 1,     /* Timeout has expired since the last MPCP REPORT   */
											       /* frame received from the ONU					   */
	PON_ONU_DEREGISTRATION_OAM_LINK_DISCONNECTION, /* ONU didn't reply to several OAM Information      */
												   /* frames, hence OAM link is disconnected		   */
	PON_ONU_DEREGISTRATION_HOST_REQUEST,		   /* Response to previous PAS_deregister_onu or       */
												   /* PAS_shutdown_onu API calls					   */
	PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION,	   /* ONU registered twice without deregistering	   */
												   /* This is an error handling for extrem cases.	   */
												   /* If the ONU should be registered, the following   */
												   /* REGISTER_REQ will cause a correct registration.  */
	PON_ONU_DEREGISTRATION_TIMESTAMP_DRIFT,		   /* when a timestamp drift of 16nsec is identified   */
                                                   /* by the HW                                        */
	PON_ONU_DEREGISTRATION_UNKNOWN,                /* unknown reason, (recovery after missing event)   */
	PON_ONU_DEREGISTRATION_RTT_JITTER,             /* RTT jitter is too big between the FW calculation 
    													and the HW calculation                           */
#ifdef   PAS_SOFT_VERSION_V5_3_5
	PON_ONU_DEREGISTRATION_ONU_RATE_LIMITER_DROP,         /* high rate traffic of CPU (Rate limiter to CPU 
                                                             discards OAM frames).                           */
#endif
	PON_ONU_DEREGISTRATION_LAST_CODE
} PON_onu_deregistration_code_t;

/* Return a string corresponding to the ONU deregistration code */
extern char *PON_onu_deregistration_code_s[];

/*====  23  start statistics & monitoring  ========*/
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
typedef enum  
{
	PON_STAT_ONU_BER,			/* Average ONU Bit error rate (upstream) as measured by the OLT                       */  
	PON_STAT_OLT_BER,			/* Average OLT Bit error rate (downstream) as measured by the ONU                     */  
	PON_STAT_ONU_FER,			/* Average ONU frames error rate (upstream) as measured by the OLT                    */  
	PON_STAT_OLT_FER,			/* Average OLT frames error rate (downstream) as measured by the ONU                  */  
	PON_STAT_OLT_PORT_BER,		/* Average OLT PON/System Bit error rate (upstream/downstream) as measured by the OLT */
	PON_STAT_LAST_STAT
} PON_statistics_t;
/* 
** Raw statistics types	(hardware counters) 
** Note: any change should be updated in PON_raw_statistics_s, PON_llid_raw_statistics_b!
*/
typedef enum  
{                                                      
    PON_RAW_STAT_ONU_BER                                            = 0  , /* Average ONU Bit error rate counters (upstream)  */ 
                                                                           /* as measured by the OLT                          */ 
    PON_RAW_STAT_OLT_BER                                            = 1  , /* Average OLT Bit error rate counters (downstream)*/ 
                                                                           /* as measured by the ONU                          */ 
    PON_RAW_STAT_ONU_FER                                            = 2  , /* Average ONU frames error rate counters(upstream)*/ 
                                                                           /* as measured by the OLT                          */ 
    PON_RAW_STAT_LLID_BROADCAST_FRAMES                              = 3  , /* LLID frames counters                            */ 
    PON_RAW_STAT_TOTAL_FRAMES                                       = 4  , /* Total frames transmitted and received per       */ 
                                                                           /* physical port                                   */ 
    PON_RAW_STAT_TOTAL_OCTETS                                       = 5  , /* Total octets (bytes) transmitted and received   */ 
                                                                           /* per physical port                               */ 
    PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID                         = 6  , /* Total octets (bytes) transmitted per LLID       */ 
                                                                           /* through the PON port                            */ 
    PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY                    = 7  , /* Number of frames, received from the UNI port and*/ 
                                                                           /* transmitted to the PON port, per priority       */ 
    PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID                        = 8  , /* Frames transmitted per LLID through the PON port*/ 
    PON_RAW_STAT_DROPPED_FRAMES                                     = 9  , /* Queue-dropped frames to transmit through the PON*/ 
                                                                           /* port                                            */ 
    PON_RAW_STAT_DROPPED_FRAMES_PER_LLID                            = 10 , /* Dropped frames by OLT due to LLID restriction   */ 
                                                                           /* /error                                          */ 
    PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES                            = 11 , /* Dropped frames received (by drop reason type)   */ 
    PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES                            = 12 , /* Total dropped frames to transmit through both   */ 
                                                                           /* ports                                           */ 
    PON_RAW_STAT_SINGLE_COLLISION_FRAMES                            = 13 , /* Not active (not used in full duplex mode)       */ 
    PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES                          = 14 , /* Not active (not used in full duplex mode)       */ 
    PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS                        = 15 , /* FCS check errors of frames received by the PON  */ 
                                                                           /* port                                            */ 
    PON_RAW_STAT_ALIGNMENT_ERRORS                                   = 16 , /* Alignment check errors of frames received by the*/ 
                                                                           /* PON port                                        */ 
    PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID                    = 17 , /* In range length check errors of frames received */ 
                                                                           /* by LLIDs                                        */ 
    PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS                             = 18 , /* In range length check errors of frames received */ 
                                                                           /* by System port                                  */ 
    PON_RAW_STAT_FRAME_TOO_LONG_ERRORS                              = 19 , /* Frame too long check errors of frames received  */ 
                                                                           /* by both ports                                   */ 
    PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID                     = 20 , /* Frame too long check errors of frames received  */ 
                                                                           /* from specified LLID                             */ 
    PON_RAW_STAT_HEC_FRAMES_ERRORS                                  = 21 , /* Header error correction check errors of frames  */ 
                                                                           /* received by the PON port                        */ 
    PON_RAW_STAT_UNSUPPORTED_MPCP_FRAMES                            = 22 , /* Unsupported MPCP type check errors of frames    */ 
                                                                           /* received by the PON port                        */ 
    PON_RAW_STAT_HOST_FRAMES                                        = 23 , /* Frames received and transmitted to the Host     */ 
    PON_RAW_STAT_HOST_OCTETS                                        = 24 , /* Octets received and transmitted to the Host     */ 
    PON_RAW_STAT_HOST_MESSAGES                                      = 25 , /* Traffic load of the Host - Firmware parallel    */ 
                                                                           /* interface                                       */ 
    PON_RAW_STAT_PAUSE_FRAMES                                       = 26 , /* Pause frames received and transmitted by both   */ 
                                                                           /* ports                                           */ 
    PON_RAW_STAT_PAUSE_TIME                                         = 27 , /* Link in Pause mode times                        */ 
    PON_RAW_STAT_REGISTRATION_FRAMES                                = 28 , /* Registration frames counters by both OLT and ONU*/ 
    PON_RAW_STAT_OAM_FRAMES                                         = 29 , /* OAM frames received and transmitted in the PON  */ 
    PON_RAW_STAT_GRANT_FRAMES                                       = 30 , /* Grant frames counters by both OLT and ONU       */ 
    PON_RAW_STAT_REPORT_FRAMES                                      = 31 , /* MPCP 'Report' frames counters collected by both */ 
                                                                           /* OLT and ONU                                     */ 
    PON_RAW_STAT_MULTICAST_FRAMES                                   = 32 , /* Multicast frames received and transmitted by the*/ 
                                                                           /* PON port per ONU                                */ 
    PON_RAW_STAT_BROADCAST_FRAMES                                   = 33 , /* Broadcast frames received and transmitted by the*/ 
                                                                           /* PON port per ONU                                */ 
    PON_RAW_STAT_P2P_FRAMES                                         = 34 , /* P2P frames received, dropped and transmitted by */ 
                                                                           /* the PON port per ONU                            */ 
    PON_RAW_STAT_BRIDGE_FRAMES                                      = 35 , /* Frames bridged by the ONU (at System port)      */ 
    PON_RAW_STAT_PROMISCUOUS_STATUS                                 = 36 , /* PON Promiscuous Status                          */ 
    PON_RAW_STAT_DUPLEX_STATUS                                      = 37 , /* PON Duplex status (Half / Full)                 */ 
    PON_RAW_STAT_RTT                                                = 38 , /* Round trip time from OLT to ONU and back        */ 
    PON_RAW_STAT_RATE_CONTROL_ABILITY                               = 39 , /* Rate Control through lowering the average data  */ 
                                                                           /* rate of the MAC sublayer ability                */ 
    PON_RAW_STAT_MPCP_STATUS                                        = 40 , /* MPCP configuration and traffic counters         */ 
    PON_RAW_STAT_OAM_STATUS                                         = 41 , /* OAM configuration and traffic counters          */ 
    PON_RAW_STAT_OAM_INFORMATION_DATA                               = 42 , /* Fields inside remote and local OAM information  */ 
                                                                           /* frames                                          */ 
    PON_RAW_STAT_OAM_FRAMES_COUNTERS                                = 43 , /* Traffic counters of OAM frames by OAM frame code*/ 
    PON_RAW_STAT_ERRORED_SYMBOL_PERIOD_ALARM                        = 44 , /* Errored Symbol Period alarm configuration and   */ 
                                                                           /* event counters                                  */ 
    PON_RAW_STAT_ERRORED_FRAME_ALARM                                = 45 , /* Errored Frame alarm configuration and event     */ 
                                                                           /* counters                                        */ 
    PON_RAW_STAT_ERRORED_FRAME_PERIOD_ALARM                         = 46 , /* Errored Frame Period alarm configuration and    */ 
                                                                           /* event counters                                  */ 
    PON_RAW_STAT_ERRORED_FRAME_SECONDS_SUMMARY_ALARM                = 47 , /*Errored Frame Seconds Summary alarm              */ 
                                                                           /* configuration and event counters                */ 
    PON_RAW_STAT_STANDARD_OAM_STATUS                                = 48 , /* Standard OAM status                             */ 
    PON_RAW_STAT_STANDARD_OAM_PEER_STATUS                           = 49 , /* Standard OAM peer status                        */ 
    PON_RAW_STAT_STANDARD_OAM_LOOPBACK_STATUS                       = 50 , /* Standard OAM loopback status                    */ 
    PON_RAW_STAT_STANDARD_OAM_STATISTIC                             = 51 , /* Standard OAM statistic                          */ 
    PON_RAW_STAT_STANDARD_OAM_EVENT_CONFIG                          = 52 , /* Standard OAM Event config                       */ 
    PON_RAW_STAT_STANDARD_OAM_LOCAL_EVENT_LOG                       = 53 , /* Standard OAM local event log                    */ 
    PON_RAW_STAT_STANDARD_OAM_REMOTE_EVENT_LOG                      = 54 , /* Standard OAM remote event log                   */ 
    PON_RAW_STAT_STANDARD_OAM_MPCP_STATUS                           = 55 , /* Standard OAM MPCP status                        */ 
    PON_RAW_STAT_STANDARD_OAM_MPCP_STATISTIC                        = 56 , /* Standard OAM MPCP statistic                     */ 
    PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATUS                  = 57 , /* Standard OAM OMP emulation status               */ 
    PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS              = 58 , /* Standard OAM OMP emulation statistics           */ 
    PON_RAW_STAT_STANDARD_OAM_MAU_STATUS                            = 59 , /* Standard OAM MAU status                         */ 
    PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY                = 60 , /* Received frames to CPU per priority             */ 
    PON_RAW_STAT_TRANSMITTED_FRAMES_FROM_CPU_PER_PRIORITY           = 61 , /* Transmitted frames from CPU per priority        */ 
    PON_RAW_STAT_CPU_PORTS_OCTET                                    = 62 , /* CPU ports octets                                */ 
    PON_RAW_STAT_CPU_PORTS_FRAMES                                   = 63 , /* CPU ports frames                                */ 
    PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES                        = 64 , /* Total dropped CPU RX frames                     */ 
    PON_RAW_STAT_ENCRYPT_FRAMES                                     = 65 , /* Encrypt frames                                  */ 
    PON_RAW_STAT_START_END_SYMBOL_FRAMES                            = 66 , /* Start, End symbols frames                       */ 
    PON_RAW_STAT_HOST_MESSAGES_OCTETS                               = 67 , /* Host messages octets                            */
    PON_RAW_STAT_DISCARDED_UNKNOWN_DESTINATION_ADDRESS              = 68 , /* Discarded unknown destination address           */
    PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY                       = 69 , /* Received frames per priority                    */
    PON_RAW_STAT_RECEIVED_OCTETS_PER_PRIORITY                       = 70 , /* Received octets per priority                    */
    PON_RAW_STAT_TRANSMITTED_OCTETS_PER_PRIORITY                    = 71 , /* Transmitted octets per priority                 */
    PON_RAW_STAT_UPLINK_VLAN_DISCARDED_FRAMES                       = 72 , /* Uplink VLAN discarded frames                    */
    PON_RAW_STAT_UPLINK_VLAN_DISCARDED_OCTETS                       = 73 , /* Uplink VLAN discarded octets                    */
    PON_RAW_STAT_DOWNLINK_VLAN_DISCARDED_FRAMES                     = 74 , /* Downlink VLAN discarded frames                  */
    PON_RAW_STAT_DOWNLINK_VLAN_DISCARDED_OCTETS                     = 75 , /* Downlink VLAN discarded octets                  */
    PON_RAW_STAT_FEC_FRAMES                                         = 76 , /* FEC frames                                      */
    PON_RAW_STAT_FEC_BLOCKS_AND_OCTETS                              = 77 , /* FEC blocks and octets                           */
    PON_RAW_STAT_TOTAL_DROPPED_RX_GOOD_FRAMES                       = 78 , /* Total frames dropped because user               */
    PON_RAW_STAT_TOTAL_OCTETS_IN_DROPPED_RX_GOOD_FRAMES             = 79 , /* Total octets in good frames dropped because user*/
    PON_RAW_STAT_EGRESS_DROPPED_FRAMES_PER_PRIORITY                 = 80 ,
    PON_RAW_STAT_DOWNLINK_RECEIVED_FRAMES_OCTETS_PER_VID            = 81 ,
    PON_RAW_STAT_P2P_GLOBAL_FRAMES_DROPPED                          = 82 ,
    PON_RAW_STAT_IPG                                                = 83 ,
    PON_RAW_STAT_TRANSMITTED_GOOD_FRAMES_OCTETS_FROM_PQ             = 84 ,
	PON_RAW_STAT_UNICAST_MULTICAST_PON_FRAMES						= 85 ,
	PON_RAW_STAT_EMPTY                                              = 86 ,
	PON_RAW_STAT_FEC_STATUS		                    				= 87 ,
#ifdef  PAS_SOFT_VERSION_V5_3_5
	PON_RAW_STAT_TRANSMITTED_EGRESS_FRAMES_PER_PRIORITY 		    = 88 ,
#endif
#ifdef  PAS_SOFT_VERSION_V5_3_13
    PON_RAW_STAT_COLLECTED_BY_OLT_PER_LLID_BASIC_BULKED             = 89 ,	/* Get some basic LLID statistics with is collected by OLT, see comments below*/
#endif

    /* B--added by liwei056@2013-9-16 for TK's StatAdapter */
    PON_RAW_STAT_UNICAST_FRAMES                                     = 100 , /* Unicast frames received and transmitted by the*/ 
                                                                           /* PON port per ONU                                */ 
    PON_RAW_STAT_LLID_LOOSE_FRAME_STATISTIC                         = 101 , 
    /* E--added by liwei056@2013-9-16 for TK's StatAdapter */
    
    PON_RAW_STAT_LAST_STAT                             
} PON_raw_statistics_t;

#ifdef  PAS_SOFT_VERSION_V5_3_13
/*
Bulked statistics details
PON_RAW_STAT_COLLECTED_BY_OLT_PER_LLID_BASIC_BULKED include following statistics:

1. PON_RAW_STAT_ONU_BER 
2. PON_RAW_STAT_ONU_FER
3. PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID
4. PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID
5. PON_RAW_STAT_DROPPED_FRAMES_PER_LLID
6. PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS
7. PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID
8. PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID
9. PON_RAW_STAT_MULTICAST_FRAMES
10.PON_RAW_STAT_BROADCAST_FRAMES
*/
#endif


/* Return a string corresponding to the statistics type */
extern char *PON_raw_statistics_s[];

/* Unicast/Multicast PON frames  */
typedef struct PON_fec_status_raw_data_t
{
    short int   number_of_llids_with_fec  ;  /* Number of llids with FEC enable, 
                                                not include broadcast llid */

} PON_fec_status_raw_data_t;

/* Protocol type options */
typedef enum
{
    PON_PROTOCOL_TYPE_ALL,			                /* all protocols					                                  */
    PON_PROTOCOL_TYPE_OLT_MAC_ADDRESS,              /* all ethernet framse with OLT mac address in the destination address*/
    PON_PROTOCOL_TYPE_IGMP,		                    /* IGMP protocol	                                                  */
    PON_PROTOCOL_TYPE_STP,                          /* spanning tree protocol                                             */
	PON_PROTOCOL_TYPE_ARP,                          /* ARP protocol			                                              */
    PON_PROTOCOL_TYPE_802_1X,                       /* 802.1x protocol			                                          */
    PON_PROTOCOL_TYPE_LAST,
									   
} PON_protocol_type_t;


/*====  23  end statistics & monitoring  ========*/

/* 
** Test modes definitions 
*/
/* Previously configured / default test mode not changed */
#define PON_TEST_MODE_NOT_CHANGED  PON_VALUE_NOT_CHANGED

typedef enum 
{
	PAS_MAC_MODE_NOT_CHANGED = PON_TEST_MODE_NOT_CHANGED,   /* MAC mode not changed					   */
	PAS_PON_MAC				 = 0,							/* Regular 802.3ah MAC					   */
    PAS_ETHERNET_MAC										/* OLT - regular Ethernet switch, with no  */
															/*       802.3ah extensions				   */
															/* ONU - Not supported					   */
} PON_mac_test_modes_t;

/*
** Test modes definitions
*/

typedef enum
{
	PON_NO_LOOPBACK  = 0    , /* No Loopback                        */
    PON_LOOPBACK_MAC = 1    , /* Loopback is done inside MAC chip   */
	PON_LOOPBACK_PHY        , /* Loopback is done inside PHY device */
    /* From PAS5201 */
    SYSTEM_LOOPBACK_PHY     , /* Loopback is done inside the system MAC chip */
    SYSTEM_CONNECTION_TEST    /* Loopback is done by the CPU for frames form the system to OLT MAC address */
} PON_loopback_t;

/* 
** Testing definitions 
*/

/* Link test */
#define PON_LINK_TEST_CONTINUOUS  (-1)    /* Start a continuous link test 			     */
#define PON_LINK_TEST_OFF		  OFF     /* Stop an already running continous link test */
#define PON_LINK_TEST_MAX_FRAMES  (32766) /* For specified number of frames test		 */


/* Link test VLAN configuration structure */
typedef struct PON_link_test_vlan_configuration_t
{
	bool				 vlan_frame_enable;	/* The frames in the link test include vlan tag.			  */
											/* Values: DISABLE, ENABLE.									  */
											/* When eanbled - measurment frame size increases by		  */
											/* VLAN_TAG_SIZE bytes.										  */
    PON_vlan_priority_t  vlan_priority;	 /*VLAN priorty range: VLAN_MIN_TAG_PRIORITY-VLAN_MAX_TAG_PRIORITY*/
	short int		     vlan_tag;			/* the vlan tag id of the frame:							  */
											/* Range: VLAN_MIN_TAG_ID - VLAN_MAX_TAG_ID					  */


} PON_link_test_vlan_configuration_t;

/* Link test results structure */
typedef struct PON_link_test_results_t
{

	long int			number_of_sent_frames;				/* Number of measurement frames sent during link  */
															/* test											  */
	long int			number_of_returned_frames;			/* Number of measurement frames received during   */
															/* link test									  */
	long int			number_of_errored_return_frames;	/* Number of measurement error frames received	  */	
															/* during link test								  */
	unsigned long int	minimal_delay;						/* The minimal delay collected from link delay    */
															/* measurements. Stated in TQ units. Valid only if*/
															/* link delay measurement is requested.			  */
	unsigned long int	mean_delay;							/* The mean delay collected from link delay 	  */
															/* measurements. Stated in TQ units. Valid only if*/
															/* link delay measurement is requested.			  */
	unsigned long int   maximal_delay;						/* The maximal delay collected from link delay    */
															/* measurements. Stated in TQ units. Valid only if*/
															/* link delay measurement is requested.			  */
} PON_link_test_results_t;

/* Statistics query methods */
typedef enum
{
	PON_STATISTICS_QUERY_HARDWARE = 2	/* Direct query from hardware counters	*/
} PON_statistics_query_method_t;


#define PON_MAX_MSG_SIZE   1600	   /* Maximal PAS-SOFT <-> OLT FW msg size, measured in bytes */



/*
** OAM definitions
*/
#define PON_ONU_OAM_FRAME_MAXIMUM_SIZE  124 /* Hardware limitation, below the maximum allowed by the */
											/* standard (PON_OAM_FRAME_MAXIMUM_SIZE), measured in    */
											/* Octets (Bytes)										 */

/* OAM V1.2 Identifier */
#define OAM_INFORMATION_EXTENSION_PRIVATE_ENTERPRISE_NUMBER_PASSAVE_VALUE	14590    

/* OAM V2.0 Identifier */
#define OAM_INFORMATION_VENDOR_IDENTIFIER_OUI_PASSAVE_VALUE					0x000CD5 

#define OAM_INFORMATION_EXTENSION_DEVICE_IDENTIFIER_PASSAVE_OLT_VALUE		0x5001 

#define OAM_INFORMATION_EXTENSION_VERSION_IDENTIFIER_TO_PASSAVE_ONU_MAJOR_VERSION(__version_identifier__)  __version_identifier__/0x100;
#define OAM_INFORMATION_EXTENSION_VERSION_IDENTIFIER_TO_PASSAVE_ONU_MINOR_VERSION(__version_identifier__)  __version_identifier__ - ((__version_identifier__/0x100)*0x100);

#define OAM_ONU_OAM_TIMEOUT 5  /* Timeout for time between consecutive OAM frames the ONU requires, */
							   /* measure in Seconds	 */

#define OAM_OUI_SIZE 3		   /* OUI size measured in Bytes */

typedef unsigned char   OAM_oui_t[OAM_OUI_SIZE];

typedef enum
{
	PON_OAM_CODE_OAM_IMFORMATION			= 0x00,	/* Coordination of local and remote OAM information. Sent both by PAS-SOFT and ONU*/
	PON_OAM_CODE_EVENT_NOTIFICATION			= 0x01,	/* Alerts remote device of OAM event(s). Sent by ONU, parsed by PAS-SOFT 		  */
    PON_OAM_CODE_VENDOR_EXTENSION			= 0xFE,	/* Available for System Vendor specific extension 0xB0- 0xFF	*/

} PON_oam_code_t;

#define PAS_LOOPBACK_MODE_NOT_CHANGED  PON_TEST_MODE_NOT_CHANGED /* Loopback mode not changed		   */
#define PAS_NO_LOOPBACK		       	   OFF						 /* Deactivate loopback				   */
#define PAS_MIN_LOOPBACK_TIMEOUT	   1						 /*Min loopback time, measured in 100ms*/
#define PAS_MAX_LOOPBACK_TIMEOUT       0xFFFF				     /*Max loopback time, measured in 100ms*/

/* 
** GPIO definitions
*/
typedef enum
{
	PON_GPIO_LINE_0  = 0,
	PON_GPIO_LINE_1,
	PON_GPIO_LINE_2,
	PON_GPIO_LINE_3,
} PON_gpio_lines_t;

#define PON_GPIO_MIN_LINE  PON_GPIO_LINE_0
#define PON_GPIO_MAX_LINE  PON_GPIO_LINE_3

typedef enum
{
	PON_GPIO_LINE_NOT_CHANGED = PON_VALUE_NOT_CHANGED,
	PON_GPIO_LINE_INPUT		  = 0,
	PON_GPIO_LINE_OUTPUT	  = 1
} PON_gpio_line_io_t;


/* OLT & ONU filled array sizes */
#define PON_MIN_OLT_ID                 0  
#define PON_MAX_OLT_ID                 20  
#define PON_OLT_ID_ARRAY_SIZE          (PON_MAX_OLT_ID+1)         /* Even though minimal value may not be zero */
#define PAS_ONU_ID_PER_OLT_ARRAY_SIZE  (PAS_MAX_ONU_ID_PER_OLT+1) /* Even though minimal value may not be zero */
#define PAS_LLID_PER_OLT_ARRAY_SIZE    (PAS_MAX_LLID_PER_OLT+1)   /* Even though minimal value may not be zero */

#define PON_ONU_ID_PER_OLT_ARRAY_SIZE  (PON_MAX_ONU_ID_PER_OLT+1)   /* Even though minimal value may not be zero */


/* ONU ids list definition */
typedef PON_onu_id_t onu_list_t  [PAS_ONU_ID_PER_OLT_ARRAY_SIZE];
typedef PON_llid_t   llid_list_t [PAS_LLID_PER_OLT_ARRAY_SIZE];

/*
** Used structures
*/

/* LLID parameters (used by Host recovery from OLT process)
**
** mac_address			: LLID MAC address
**
** authorization_mode	: Authorization mode of the LLID, values: enum values
**
** enccryption_mode		: Encryption mode of the LLID, values: enum values
**
** oam_version			: LLID OAM version, values: enum values
*/
typedef struct
{
	mac_address_t				mac_address;
	PON_authorize_mode_t		authorization_mode;
	PON_encryption_direction_t	encryption_mode; 
	OAM_standard_version_t		oam_version;
} PON_llid_parameters_t;

typedef struct
{
	PON_llid_t		llid;		 /* LLID index	 	 */
	mac_address_t	mac_address; /* LLID MAC address */  
} PON_active_llid_record_t;

typedef struct
{
	PON_llid_t				  number_of_active_records;
	PON_active_llid_record_t  llid_records[PAS_LLID_PER_OLT_ARRAY_SIZE];
} PON_active_llids_t;

typedef struct PON_olt_response_parameters_t  
{
  PON_update_olt_parameters_t         update_olt_parameters;
  PON_external_downlink_buffer_size_t external_downlink_buffer_size;
  mac_address_t                       olt_mac_address;

} PON_olt_response_parameters_t;


/* 
** Uplink VLAN config structure
**
** Parameters:
**
**      untagged_frames_authentication_vid : VID to assign untagged frames for authentication
**      authenticated_vid                  : which VID to authenticate Range: VLAN_MIN_TAG_ID..VLAN_UNTAGGED_ID or PON_ALL_FRAMES_AUTHENTICATE
**      discard_untagged                   : discard unauthenticated untagged frames
**      discard_tagged                     : discard unauthenticated tagged frames (not untagged)
**      discard_null_tagged                : discard untagged frames
**      discard_nested                     : discard unauthenticated nested tagged frames
**      vlan_manipulation                  : VLAN manipulation type
**      new_vlan_tag_id                    : New VLAN tag id, Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID
**      vlan_type                          : VLAN type , see PON_olt_vlan_uplink_type_t 
**      vlan_priority                      : VLAN priority, Range: 0..7 or PON_VLAN_ORIGINAL_PRIORITY
*/ 

/*
 *	VLAN types
 */
typedef enum
{
    PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0XFFF       ,
    PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0         

} PON_olt_untagged_frames_authentication_vid_t;

typedef enum
{
    PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION          ,  /* No manipulation         */
    PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED        ,  /* VLAN tag is added       */
    PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED    ,  /* VLAN tag is exchanged   */
    PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED          /* Frame is discarded      */

} PON_olt_vlan_uplink_manipulation_t; 

typedef enum
{
	PON_OLT_VLAN_ETHERNET_TYPE_8100           , /* VLAN type is 0x8100						  */
	PON_OLT_VLAN_ETHERNET_TYPE_9100           , /* VLAN type is 0x9100						  */
	PON_OLT_VLAN_ETHERNET_TYPE_88A8           , /* VLAN type is 0x88a8						  */
    PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE   , /* VLAN type is configurable                  */
    PON_OLT_VLAN_ETHERNET_TYPE_NUM = 4
} PON_olt_vlan_uplink_type_t;

typedef enum
{
	PON_OLT_VLAN_UPLINK_PRIORITY_SOURCE_ORIGINAL_FRAME           , /* From the received frame  */
	PON_OLT_VLAN_UPLINK_PRIORITY_SOURCE_PRIORITY_FIELD             /* From the priority field  */
} PON_olt_vlan_uplink_priority_source_t;


/* 
** Uplink VLAN config structure
**
** Parameters:
**
**      untagged_frames_authentication_vid : VID to assign untagged frames for authentication
**      authenticated_vid                  : which VID to authenticate Range: VLAN_MIN_TAG_ID..VLAN_UNTAGGED_ID or PON_ALL_FRAMES_AUTHENTICATE
**      discard_untagged                   : discard unauthenticated untagged frames
**      discard_tagged                     : discard unauthenticated tagged frames (not untagged)
**      discard_null_tagged                : discard untagged frames
**      discard_nested                     : discard unauthenticated nested tagged frames
**      vlan_manipulation                  : VLAN manipulation type
**      new_vlan_tag_id                    : New VLAN tag id, Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID
**      vlan_type                          : VLAN type , see PON_olt_vlan_uplink_type_t 
**      vlan_priority                      : VLAN priority, Range: 0..7 or PON_VLAN_ORIGINAL_PRIORITY
*/ 
typedef struct PON_olt_vlan_uplink_config_t
{
    PON_olt_untagged_frames_authentication_vid_t      untagged_frames_authentication_vid;
    PON_vlan_tag_t                                    authenticated_vid;
    bool                                              discard_untagged;
    bool                                              discard_tagged;
    bool                                              discard_null_tagged;
    bool                                              discard_nested;
    PON_olt_vlan_uplink_manipulation_t                vlan_manipulation;
    PON_vlan_tag_t                                    new_vlan_tag_id;
    PON_olt_vlan_uplink_type_t                        vlan_type;
    PON_vlan_priority_t                               vlan_priority;

} PON_olt_vlan_uplink_config_t;


/* 
** Downlink VID config structure
**
** Parameters:
**
**      discard_nested    : discard nested tagged frames arriving from this VID
**      destination       : LLID allocation of the frames associated with the VID
**      llid              : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT or  PON_ BROADCAST_LLID
**      vlan_manipulation : VLAN manipulation
**      new_vid           : New VLAN tag id, Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID
**      new_priority      : Priority for tag exchange, Range: 0..7
*/ 
typedef enum
{
	PON_OLT_VLAN_DESTINATION_NONE                                ,
    PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE                       ,
    PON_OLT_VLAN_DESTINATION_LLID_PARAMETER                      ,
    PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE_AND_LLID_PARAMETER
} PON_olt_vlan_destination_t;

typedef enum
{
    PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION            ,    /* No manipulation                                 */
    PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG            ,    /* Remove VLAN tag                                 */
    PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID               ,    /* Exchange VID in VLAN tag with new VID parameter */
    PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY       /* Exchange VID in VLAN tag with new VID parameter 
                                                                          and priority with new priority parameter        */
} PON_olt_vlan_downlink_manipulation_t; 

typedef struct PON_olt_vid_downlink_config_t 
{
    bool                                      discard_nested;
    PON_olt_vlan_destination_t                destination;
    PON_llid_t                                llid;
    PON_olt_vlan_downlink_manipulation_t      vlan_manipulation;
    PON_vlan_tag_t                            new_vid;
    PON_vlan_priority_t                       new_priority;
} PON_olt_vid_downlink_config_t;




#ifdef  PAS_SOFT_5001 
/******************* PAS-SOFT 5001 V4.16.2 start **********/
/* Init	
**
** Init Passave API (PON software inside PAS-SOFT). No other API function may be called prior or during 
** function process time.
**
** Input Parameter:
**	 initialization_parameters				: PON initialization struct, 
**											  see PON_pon_initialization_parameters_t for details
**		automatic_authorization_policy		: ONUs automatic authorization to the network policy
**
** Return codes:
**			PAS_EXIT_OK						: Success
**			PAS_EXIT_ERROR					: General Error
**			PAS_PARAMETER_ERROR				: Function parameter error
*/
extern PON_STATUS PAS_init_v4 ( const PAS_pon_initialization_parameters_t  *initialization_parameters );


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
**		olt_mac_address				    : OLT MAC address
**										  If MAC address is to be set by caller - valid unicast MAC 
**											address value
**										  If OLT default MAC address is used - OLT_DEFAULT_MAC_ADDRESS
**											(0xFF-0xFF-0xFF-0xFF-0xFF-0xFF - all 1s)
**		optics_configuration			: OLT Optics configuration. See PON_olt_optics_configuration_t
**										  for details.
**		discovery_laser_on_time			: Laser on time of the ONUs during the discovery process
**										  ,measured in TQ. 
**										  range: OFF - no constraint, 1 - (2^16-1) - constraint value
**		discovery_laser_off_time		: Laser off time of the ONUs during the discovery process
**										  , measured in TQ. 
**										  range: OFF - no constraint, 1 - (2^16-1) - constraint value
**		igmp_configuration				: OLT IGMP configuration, see PON_olt_igmp_configuration_t for
**										  details
**      vlan_configuration				: OLT global VLAN configuration, see PON_olt_vlan_configuration_t
**										  for details
**		multiple_copy_broadcast_enable	: Enable / Disable multiple LLID VLANs. Values: ENABLE / DISABLE
**		discard_unlearned_addresses		: Discard / forward any frame received from unlearned source 
**										  address of an ONU when ONU entries limit inside address table 
**										  is reached. Values: TRUE / FALSE.
**		_802_1x_authenticator_enable	: Enable / disable 802.1x authenticator functionality 
**		firmware_image					: OLT firmware image file characteristics. See PON_binary_t for 
**										  details. Structure Values:
**										  type - PON_OLT_BINARY_FIRMWARE
**										  source - All supported options
**										  location - According to PON_binary_t guidelines 
**										  size - According to PON_binary_t guidelines 
**										  identification - Not required for current versions
**
** Output Parameter:
**		olt_mac_address				    : The input parameter changes its value to the OLT's actual MAC  
**										  address, whether set explicitly by the input parameter or 
**										  taken from the OLT default (EEPROM) MAC address
**									
** Return codes:
**				All the defined PAS_* return codes
**						
*/
extern PON_STATUS PAS_add_olt_v4
                       ( const short int				  olt_id, 
				 	     mac_address_t					  olt_mac_address,
					     const PON_olt_init_parameters_t  *olt_initialization_parameters );

extern PON_STATUS PAS_test_add_olt_v4
                           ( const short int				   olt_id, 
					         mac_address_t					   olt_mac_address,
					         const PON_olt_init_parameters_t  *olt_initialization_parameters );


/* Remove OLT									
**
** This function is to be called when OLT (card) is removed.
** The function removes all PON software internal OLT references.
**
** Input Parameter:
**			    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Return codes:
**				All the defined PAS_* return codes
**						
*/
extern PON_STATUS PAS_remove_olt_v4 ( const short int  olt_id );


/* Update OLT parameters
**
** Update OLT parameters during run-time. Zero or more OLT parameters may be updated (Each parameter 
** has non-update value).
** The macro PON_EMPTY_OLT_UPDATE_PARAMETERS_STRUCT 'empties' PON_olt_update_parameters_t struct (so each
** parameter has non-update value)
**
** Input Parameters:
**		olt_id						 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		updated_parameters			 : A struct containing parameters to be updated. Parameters
**									   which have PON_VALUE_NOT_CHANGED value are not updated.Every 
**									   parameter which should be updated has non PON_VALUE_NOT_CHANGED
**									   value. 
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_update_olt_parameters_v4 
			  ( const short int						olt_id,
				const PON_olt_update_parameters_t  *updated_parameters );
 
extern PON_STATUS PAS_test_update_olt_parameters_v4 
			  ( const short int						olt_id,
				const PON_olt_update_parameters_t  *updated_parameters );

/* Get OLT parameters 
**
** Query of advanced OLT configuration parameters, corresponding to the ones updated by 
** PAS_update_olt_parameters API function.
**
** Input Parameter:
**		olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameter:
**		olt_parameters			 : A struct containing the OLT parameters 
**        
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_olt_parameters_v4  
                                  ( const PON_olt_id_t			  olt_id,
							        PON_olt_update_parameters_t  *olt_parameters );



/* Get device capabilities
**
** Get several physical capabilities of an OLT / ONU device.
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    device_id : Device id, PON_OLT_ID		                                 - OLT device
**								       PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT   - ONU device
**
** Output Parameters (PAS_physical_capabilities_t struct):
**										 OLT device			ONU device
**										 ----------			-----------
**				laser_on_time		  : Not available (0)	Time takes the ONU laser to turn on
**				laser_off_time		  : Not available (0)   Time takes the ONU laser to turn off
**				onu_grant_fifo_depth  : Not available (0)   (PON_MAX_ONU_REPETITIONS_PER_GRANT_CYCLE*2) for PAS6001
**				agc_lock_time		  : OLT AGC lock time	Not available (0)
**				cdr_lock_time		  : OLT RX Signal sync.	Not available (0)
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_device_capabilities_v4
                                     ( const short int				  olt_id,
					const PON_device_id_t		  device_id,
					PAS_physical_capabilities_t  *device_capabilities );



/* Assign handler function									
**
** Assign (register) a handler function which handles a single event type coming from the PON.
** Handler function must be fast (specially PAS_HANDLER_FRAME_RECEIVED handler function), and may not
** include any calls to PAS5001 system API functions (this header file), except for ONU 
** registration, ONU deregistration and OLT reset handler functions (see the corresponding event 
** definitions for details).
**
** Input Parameters:
**			    handler_function_index  : Event type to be handled by the handler function
**			    handler_function        : Pointer to the handler function (NULL for not assigning a 
**										  function)
**
** Return codes:
**				PAS_EXIT_OK			   : Success
**				PAS_EXIT_ERROR		   : General Error
**				PAS_PARAMETER_ERROR    : Function parameter error (e.g. handler_function_index out of 
**										 range)
**
*/
extern PON_STATUS PAS_test_assign_handler_function_v4
				( const PAS_handler_functions_index_t    handler_function_index, 
				  const void							(*handler_function)() );

extern PON_STATUS PAS_assign_handler_function_v4
				( const PAS_handler_functions_index_t    handler_function_index, 
				  const void							(*handler_function)() );
		


/* Load OLT binary
**
** Load a binary file to an OLT device. 
** Used for firmware upgrade, and DBA algorithms loading (packed as DLLs).
** Note: If several functions are activated simultaneously (e.g. by several applications/threads), the 
** loadings will be performed sequentially by PAS-SOFT. All binary file preparations for reading, as 
** well as post-reading manipulations are done within the function (e.g. open file, close file).
** This function initiates an event which should be handled by PAS_load_olt_binary_completed_handler_t.
** The external file may contain a complete OLT firmware, or a DBA DLL compiled for Passave environment. 
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID  - A single OLT
**		olt_binary		: OLT binary definitions. See PON_binary_t for details
**					
** Return codes:
**				PAS_DBA_ALREADY_RUNNING - DBA algorithm is already running (from a DLL file), a new DBA DLL 
**										  file can't be loaded
**				All the defined PAS_* return codes
**
** Only when the load operation is finished, the function will return.
** If the function returns with an error code which is NOT PAS_PARAMETER_ERROR, PAS_DBA_ALREADY_RUNNING 
** or PAS_OLT_NOT_EXIST errors, there is very little to assume regarding the OLT state. It probably will 
** not function normally.
**
*/
extern PON_STATUS PAS_load_olt_binary_v4
                              ( const short int		 olt_id, 
							    const PON_binary_t  *olt_binary );


/* GPIO access
**
** Set the GPIO (General purpose IO) output lines into new states and read the states of the input lines.
** Currently, two of the PAS5001 four GPIO lines are available for Host manipulation.
**
** Input Parameters:
**		 olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 gpio_line_1_io		: Input / Output / Not changed line. Values: enum values
**		 gpio_line_1_state  : GPIO output line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**		 gpio_line_2_io		: Input / Output / Not changed line. Values: enum values
**		 gpio_line_2_state  : GPIO output line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**
** Output Parameter:
**		 gpio_line_1_state  : GPIO line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**		 gpio_line_2_state  : GPIO line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_gpio_access_v4
                          ( const short int			   olt_id, 
			const PON_gpio_line_io_t   gpio_line_1_io,	
			short int   *gpio_line_1_state,
			const PON_gpio_line_io_t   gpio_line_2_io,	
			short int   *gpio_line_2_state );


/* GPIO access extended
**
** Set the GPIO (General purpose IO) output lines into new states and read the states of the input lines. 
** The extended command supports all four PAS5001 GPIO lines.
**
** Input Parameters:
**		 olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 line_number		: GPIO line number controlled. range: enum values
**		 set_direction		: Determine the direction for the GPIO line. Values: enum values
**		 set_value			: GPIO output line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**		 
** Output Parameters:
**		 direction			: Actual line type: Input / Output. Values: enum values
**		 value				: GPIO line active low / high. Values: OFF, ON
**
** Return codes:
**		 All the defined PAS_* return codes
*/
extern PON_STATUS PAS_gpio_access_extended_v4
                                   ( const PON_olt_id_t			olt_id, 
				 const PON_gpio_lines_t		line_number,
				 const PON_gpio_line_io_t   set_direction,
				 const short int			set_value,
				 PON_gpio_line_io_t		   *direction,
				 bool					   *value );


/* Set link alarm params
**
** Configure alarm event generation thresholds. The alarm event will be invoked when one or more of 
** the conditions specified are met.
** PAS_init function resets all thresholds to their default values. 
** Threshold parameters are applicable for both the OLT and the ONUs of the OLT specified.
**
** Input Parameters:
**			    olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    link_alarm_params	  : PAS_link_alarm_params_t link alarms parameters structure
**													
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_set_link_alarm_params_v4
                                    ( const short int				  olt_id, 
				const PAS_link_alarm_params_t  *link_alarm_params );

/* Reset address table
**
** Reset the content of an OLT address table, automatic address learning will refill the table again.	
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**													
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_reset_address_table_v4 ( const short int	 olt_id );


/* Reset address table extended
**
** Reset the content of an OLT address table according to address type and LLID.
** Automatic MAC address learning will refill the table again
**	
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				llid            : 0 - PON_MAX_LLID_PER_OLT or PON_ALL_ACTIVE_LLIDS or
**                                PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID  
**              address_type	: Dynamic or Static or Dynamic & Static								
** Return codes:
**				All the defined PAS_* return codes
**
*/
/* 
** 802.1x Authenticator configuration 
*/
/* Shared secret max length */
#define PON_MAX_SHARED_SECRET_BYTES	   32 /* Bytes */

typedef struct PON_802_1x_configuration_t
{
	unsigned char shared_secret[PON_MAX_SHARED_SECRET_BYTES]; /* Shared secret key for RADIUS server */
															  /* authentication						 */
} PON_802_1x_configuration_t;

extern PON_STATUS PAS_reset_address_table_extended_v4
                             ( const PON_olt_id_t	        olt_id ,
                               const PON_llid_t           llid   ,
                               const PON_address_aging_t  address_type
                             );


/* Set 802.1X configuration
**
** Configure OLT internal IEEE 802.1x authenticator parameters. OLT internal IEEE 802.1x authenticator 
** default mode is disabled. In a network where IEEE 802.1x is used as the authentication standard, 
** this function is usually used twice: once in order to initialize the OLT internal authenticator, 
** and the second time during System shutdown process.
** Note: Enabling 802.1x authenticator functionality is a precondition for ONUs authentication in IEEE 
** 802.1x networks.
**
** Input Parameters:
**		olt_id						: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      enable_802_1x_authenticator	: Enable / Disable the OLT internal 802.1x authenticator. 
**									  Values: ENABLE, DISABLE
**      p802_1x_configuration		: OLT internal 802.1x authenticator configuration, 
**									  See PON_802_1x_configuration_t struct for details
**
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_set_802_1x_configuration_v4
                                       ( const short int  olt_id,
				 const bool	enable_802_1x_authenticator,
				 const PON_802_1x_configuration_t  *p802_1x_configuration );


/* Get 802.1x configuration
**
** Query OLT 802.1x configuration. This configuration corresponds to the configuration set by the
** PAS_set_802_1x_configuration API function.
**
** Input Parameter:
**		olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      _802_1x_authenticator_enabled	: Is OLT internal 802.1x authenticator enabled. 
**										  Values: ENABLE, DISABLE
**      p802_1x_configuration			: OLT internal 802.1x authenticator configuration, 
**										  See PON_802_1x_configuration_t struct for details
**
** Return codes:
**		All the defined PAS_* return codes
**
*/  
extern PON_STATUS PAS_get_802_1x_configuration_v4 
                                    ( const PON_olt_id_t		    olt_id,
				  bool		 *_802_1x_authenticator_enabled,
				  PON_802_1x_configuration_t   *p802_1x_configuration );

	
/* Set 802.1x parameters API functions
**
** Set the 802.1x authentication parameters
**
** Input Parameters:
**	    olt_id			        	: OLT id, Values: PON_ALL_ACTIVE_OLTS
**      enable_802_1x_authenticator	: Enable / Disable the OLT internal 802.1x authenticator. 
**									  Values: ENABLE, DISABLE
**      parameters_data             : 802.1x parameters data to set
**                                    Values : see the PON_802_1x_configuration_extended_t declaration       
**
** Return codes:
**				All the defined PAS_* return codes
**
*/

typedef struct PON_802_1x_configuration_extended_t 
{
   	unsigned char           shared_secret[PON_MAX_SHARED_SECRET_BYTES]; /* Shared secret key for RADIUS server */
														         	    /* authentication						 */
    unsigned short int      quite_period           ;  /* 0 to 65,535 */
    unsigned short int      tx_period              ;  /* 1 to 65,535 */
    unsigned short int      supp_timeout           ;  /* 1 to 65,535 */
    unsigned short int      server_timeout         ;  /* 1 to 65,535 */
    unsigned char           max_reauthentication   ;  
    unsigned char           max_request            ;  /* 1 to 10 */
} PON_802_1x_configuration_extended_t;

extern PON_STATUS PAS_set_802_1x_configuration_extended_v4
                                                ( const short int                               olt_id, 
                                                  const bool						            enable_802_1x_authenticator,
            					const PON_802_1x_configuration_extended_t     *parameters_data );

extern PON_STATUS PAS_test_802_1x_configuration_extended_v4
                                                ( const short int                               olt_id, 
                                                  const bool						            enable_802_1x_authenticator,
            					 const PON_802_1x_configuration_extended_t     *parameters_data );

/* Get 802.1x parameters API functions
**
** Get the 802.1x authentication parameters
**
** Input Parameters:
**			    olt_id				: OLT id, Values: PON_ALL_ACTIVE_OLTS
**
** Output Parameters:
**      return_code                     : the return codes that return from the FW
**      _802_1x_authenticator_enabled	: Is OLT internal 802.1x authenticator enabled. 
**										  Values: ENABLE, DISABLE
**      parameters_data                 : 802.1x parameters data to set
**                                        Values : see the PON_802_1x_configuration_extended_t declaration       
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_802_1x_configuration_extended_v4
                                                ( const short int                       olt_id, 
                                                  bool			        			    *_802_1x_authenticator_enabled,
                                                  unsigned short int                    *return_code,
								                  PON_802_1x_configuration_extended_t   *parameters_data ); 


/* Set llid VLAN configuration
**
** Add / update VLAN entity handling configuration record. The function overrides the previous 
** configuration. LLID VLAN traffic may be handled by one of three modes. 'No change' is the defualt 
** handling mode of a LLID upon registration. 
** P2P traffic is not affected by LLID VLAN configuration (whether it is the source of the traffic or 
** the destination), Including broadcast LLID P2P traffic.
**
** LLID VLAN handling modes:
** Not change - original VLAN tagged / non-tagged frames are passed both in downlink and in uplink.
** Exchange - original VLAN tags are replaced. This mode replaces the first VLAN tag of every LLID frame 
**		      containing (at least one) VLAN tag to one of the tags specified. The frames to be modified
**			  must have VLAN type of 0x8100. The exchanged downlink VLAN tag is composed of OLT constant 
**			  'downlink tag prefix' concatenated with 'downlink tag suffix' field. VLAN tag priority is 
**			  not changed in this mode.
** Stack - original VLAN tags are stacked: VLAN tag added on uplink, and removed on downlink. Nested mode 
**		   support (several VLAN tags per frame) for this mode is optional. VLAN tag priority is copied  
**         from the original tag, or set to default for previously untagged frames. 
** 
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid					: Configured LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**		handling_mode			: VLAN handling mode, Values: enum values
**		exchange_configuration  : VLAN exchange mode handling configuration, see 
**								  PON_vlan_exchange_configuration_t for details
**		stack_configuration		: VLAN stack mode handling configuration, see 
**								  PON_vlan_stack_configuration_t for details
**		
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_set_llid_vlan_configuration_v4
                             ( const short int							 olt_id, 
			   const PON_llid_t						     llid,
			   const PON_vlan_handling_mode_t			 handling_mode,
			   const PON_vlan_exchange_configuration_t  *exchange_configuration,
			   const PON_vlan_stack_configuration_t     *stack_configuration );

extern PON_STATUS PAS_test_llid_vlan_configuration_v4
                                          ( const short int							  olt_id, 
					const PON_llid_t						  llid,
					const PON_vlan_handling_mode_t			  handling_mode,
					const PON_vlan_exchange_configuration_t  *exchange_configuration,
					const PON_vlan_stack_configuration_t     *stack_configuration );


/* Set LLID address table configuration
**
** Set the maximal number of MAC addresses allowed for a LLID in the OLT address table.
** 
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid				: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**		maximum_entries		: Maximum number of entries allowed in OLT address table. Values: enum values
** 
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_set_llid_address_table_configuration_v4 
						( const short int									 olt_id, 
						  const PON_llid_t									 llid,
						  const PON_address_table_entries_llid_limitation_t  maximum_entries );


/* Get LLID address table configuration
**
** Set the maximal number of MAC addresses allowed for a LLID in the OLT address table.
** 
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid				: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
** Output Parameters:
**		maximum_entries		: Maximum number of entries allowed in OLT address table. Values: enum values
** 
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_get_llid_address_table_configuration_v4 
						( const short int								 olt_id, 
						  const PON_llid_t								 llid,
						  PON_address_table_entries_llid_limitation_t   *maximum_entries );

/* Get llid VLAN configuration
**
** Query LLID VLAN configuration. This configuration corresponds to the configuration set by the
** PAS_set_llid_vlan_configuration API function.
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid					: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**
** Output Parameters:
**		handling_mode			: VLAN handling mode, Values: enum values
**		exchange_configuration  : VLAN exchange mode handling configuration, see 
**								  PON_vlan_exchange_configuration_t for details
**		stack_configuration		: VLAN stack mode handling configuration, see 
**								  PON_vlan_stack_configuration_t for details
**		
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_llid_vlan_configuration_v4
                                             ( const PON_olt_id_t				   olt_id, 
					   const PON_llid_t					   llid,
					   PON_vlan_handling_mode_t		      *handling_mode,
					   PON_vlan_exchange_configuration_t  *exchange_configuration,
					   PON_vlan_stack_configuration_t     *stack_configuration );


/* Set policing parameters
**
** Set the bandwidth policy parameters (restrictions) for a LLID (or for broadcast traffic).
** Each call to the function overrides the previous policy values. Default LLID policies upon registration
** (or reregistration) - disabled.
** Note : Works only on PAS5001
** Input Parameters:
**		 olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid					: LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**								  PON_BROADCAST_LLID
**		 policer_id				: The Policer configured for the LLID. Values: enum values
**		 enable				    : Policing Enable / Disable, values: ENABLE /DISABLE. 
**								  If disabled, policing_struct is ignored
**		 policing_struct		: Policing parameters, PON_policing_struct_t structure
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_set_policing_parameters_v4
                                      ( const short int				  olt_id, 
				const PON_llid_t			  llid,
				const PON_policer_t			  policer_id,
				const bool					  enable,
				const PON_policing_struct_t  *policing_struct );

extern PON_STATUS PAS_test_policing_parameters_v4 
                                      ( const short int				  olt_id, 
				const PON_llid_t			  llid,
				const PON_policer_t			  policer_id,
				const bool					  enable,
				const PON_policing_struct_t  *policing_struct );

/* Get policing parameters 
**
** Query LLID policing configuration. This configuration corresponds to the configuration updated 
** by the PAS_set_policing_parameters API function.
** Note : Works only on PAS5001
** Input Parameters:
**		 olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid					: Queried LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**													   PON_BROADCAST_LLID
**		 policer_id				: Queried policer. Values: enum values
**
** Output Parameters:
**		 enable				    : Policer Enable / Disable, values: ENABLE / DISABLE.
**								  If disabled - policing_struct is ignored
**		 policing_struct		: Policing parameters, see PON_policing_struct_t structure for details
**					
** Return codes:
**		 All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_policing_parameters_v4 
                                ( const PON_olt_id_t	  olt_id, 
				  const PON_llid_t		  llid,
				  const PON_policer_t	  policer_id,
				  bool				     *enable,
				  PON_policing_struct_t  *policing_struct );


/* Shutdown ONU
**
** After ONU has powered up and registered, using this function sets it in an OFF mode, in which it 
** doesn't function as a media converter (uplink or downlink). During OFF mode, the ONU doesn't 
** respond to incoming frames of any type, with the only exception of (ONU hardware) unsupported 
** OAM frames. The ONU handles the unsupported OAM frames by sending them to its host (if exists).
** The function may return after sending request for ONU shutdown, or after the shutdown process has
** completed.
**
** Input Parameters:
**		olt_id						 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id						 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**      wait_for_shutdown_completion : Should the function return after the shutdown process has 
**									   completed: ONU is verified as shutdown and ONU deregistration
**									   event has been raised. Values: TRUE / FALSE
**
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_shutdown_onu_v4
                           ( const short int	 olt_id, 
			 const PON_onu_id_t	 onu_id,
			 const bool			 wait_for_shutdown_completion );

extern PON_STATUS PAS_test_shutdown_onu_v4
                           ( const short int	 olt_id, 
			 const PON_onu_id_t	 onu_id,
			 const bool			 wait_for_shutdown_completion );

/* Get raw statistics
**
** Get hardware counters statistics collected by an OLT / ONU device (a.k.a. the collector).
** See 'Raw statistics types' and 'Raw statistics types data structures details' definitions in 
** PON_expo.h file for raw statistics structures details.
** See API document for statistics_type - statistics_parameter - statistics_data valid sets of values.
** Note: Caller must allocate the returned data structure before calling the function, and cast it to 
** 'void' when calling the function.
**
** Input Parameters:
**		olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		collector_id		  : Statistics collector, values:
**								PON_OLT_AND_ONU_COMBINED_ID							- OLT & ONU devices 
**																					  combined statistics
**								PON_OLT_ID											- OLT device
**							 	PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT     - ONU device
**		raw_statistics_type	  : The raw statistics type requested, range: enumerated values
**		statistics_parameter  : Each statistics type requires a different statistics parameter
**		collecting_method	  : The method used by PAS-SOFT to get the raw statistics, values:
**								PON_STATISTICS_QUERY_HARDWARE		  -	favor direct query of the raw 
**																		statistics from the various 
**																		hardware/s counters
**				
** Output Parameter:
**		statistics_data	      : Raw statistics data structure (PON_*_raw_data_t structures
**								defined in PON_expo.h), allocated by the caller
**		timestamp			  : OLT statistics sample time. Valid only if an OLT or both OLT and ONU are
**								the statistics collector
**
** Return codes:
**				All the defined PAS_* return codes
**
*/ 
extern PON_STATUS PAS_get_raw_statistics_v4
                                 ( const short int						 olt_id, 
			   const short int						 collector_id, 
			   const PON_raw_statistics_t			 raw_statistics_type, 
			   const short int						 statistics_parameter,
			   const PON_statistics_query_method_t	 collecting_method,
			   void								    *statistics_data,
			   PON_timestamp_t						*timestamp );



/* Set test mode
**
** Sets a device in a test mode. The device remains in the test mode until explicitly returned to regular 
** mode (with the exception of ONU loopback modes). 
** Zeroing all test modes returns the device back to normal mode.
**
** Input Parameters:
**	olt_id							  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	device_id						  : Device to be set, values:
**										  PON_OLT_ID - OLT device, 
**										  PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT - ONU device
**	mac_mode						  : MAC test mode, range: enumerated values.Valid for OLT device only
**										but not supported.
**	loopback_mode					  : Loopback activation mode, values (by devices): 
**										OLT MAC - PAS_LOOPBACK_MODE_NOT_CHANGED, PAS_NO_LOOPBACK, ON
**										OLT PHY - PAS_LOOPBACK_MODE_NOT_CHANGED, PAS_NO_LOOPBACK, ON
**										ONU MAC - PAS_LOOPBACK_MODE_NOT_CHANGED, PAS_NO_LOOPBACK, ON
**										ONU PHY - PAS_LOOPBACK_MODE_NOT_CHANGED, PAS_NO_LOOPBACK, 
**											      PAS_MIN_LOOPBACK_TIMEOUT - PAS_MAX_LOOPBACK_TIMEOUT
**	loopback_type					  : Loopback type to activate \ deactivate, range: enumerated values
**	onu_phy_loopback_speed_limitation : Limit the speed of loopback mode. Effective only for ONU MII/GMII 
**										PHY loopback. Values: enum values (specific speed limitation) and
**									    PON_OAM_PHY_LOOPBACK_KEEP_CURRENT_RESOLUTION constant (keep 
**									    speed resolution / Non ONU MII/GMII PHY loopback mode value)
**									
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_test_mode_v4
                            ( const short int			  olt_id, 
			  const PON_device_id_t		  device_id,
		      const PON_mac_test_modes_t  mac_mode,
			  const long int			  loopback_mode,
			  const PON_loopback_t		  loopback_type,
			  const PON_link_rate_t	      onu_phy_loopback_speed_limitation );

extern PON_STATUS PAS_set_test_mode_v4 
                            ( const short int			  olt_id, 
			  const PON_device_id_t		  device_id,
		      const PON_mac_test_modes_t  mac_mode,
			  const long int			  loopback_mode,
			  const PON_loopback_t		  loopback_type,
			  const PON_link_rate_t	      onu_phy_loopback_speed_limitation );


/* Get test mode
**
** Query ONU device active test modes. ONU device test modes can be set by PAS_set_test_mode API function.
**
** Input Parameters:
**		olt_id								: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		device_id							: Queried device (ONU), range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**											
** Output Parameters:						
**		loopback_mode						: Loopback activation mode, values (by devices): 
**											  ONU MAC - PAS_NO_LOOPBACK, ON
**											  ONU PHY - PAS_NO_LOOPBACK, PAS_MIN_LOOPBACK_TIMEOUT - PAS_MAX_LOOPBACK_TIMEOUT
**		loopback_type						: Loopback type in use, Values: enumerated values, OFF
**		onu_phy_loopback_speed_limitation	: Limited speed of loopback mode. Effective only for ONU MII/GMII 
**											  PHY loopback. Values: enum values (specific speed limitation) and
**											  PON_OAM_PHY_LOOPBACK_KEEP_CURRENT_RESOLUTION constant (Use MDIO Master state
**											  machine rate (speed) as loopback speed / Non ONU MII/GMII PHY loopback mode value)
**		 
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_test_mode_v4 
                                ( const PON_olt_id_t	  olt_id, 
			  const PON_device_id_t   device_id,
			  long int			     *loopback_mode,
			  PON_loopback_t		 *loopback_type,
			  PON_link_rate_t	     *onu_phy_loopback_speed_limitation );


/* Get OLT Mode query
**
** Input Parameter:
**		olt_id						 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**		olt_mode					 : OLT mode. Valid values: enum values. 
**									   Only PON_OLT_MODE_CONFIGURED_AND_ACTIVATED value indicates a successful
**									   completion of the recovery process.
**		mac_address					 : OLT MAC address, valid unicast MAC address value
**		initialization_configuration : OLT Initialization parameters struct, note: firmware_image parameter is zeroed
**		dba_mode					 : DBA mode, Valid values: PON_PROVISIONING_MODE_STATIC_GRANTING, 
**									   PON_PROVISIONING_MODE_INTERNAL_DBA, PON_PROVISIONING_MODE_EXTERNAL_DBA 
**		link_test					 : LLID that is being tested in active link test. 
**									   Valid values: PON_OLT_NOT_IN_LINK_TEST, PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**		active_llids				 : List of active LLIDs. Each LLID record includes the LLID index and its MAC address.
**
** Return codes:
**		All the defined PAS_* return codes
*/
extern PON_STATUS PAS_get_olt_mode_query_v4
                                 ( const PON_olt_id_t		    olt_id,
								   PON_olt_mode_t			   *olt_mode,
								   mac_address_t			    mac_address,
								   PON_olt_init_parameters_t   *initialization_configuration, 
								   unsigned short int		   *dba_mode, 
								   PON_llid_t				   *link_test,  
								   PON_active_llids_t		   *active_llids );


/* Reset OLT hw counters
**
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/

extern PON_STATUS PAS_reset_olt_hw_counters_v4 
                ( const PON_olt_id_t   olt_id);


/* Set ONU UNI port MAC configuration
**
** Set new configuration for ONU UNI (System) port MAC. When ONU powers up, it reads the MAC configuration
** from its EEPROM, this command overrides the configuration until the next reset \ shutdown. 
** Default EEPROM configuration may be changed only by re-burning.
**
** Input Parameters:
**		 olt_id							:  OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 onu_id							:  ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
**			mac_configuration
**				mac_type				:  UNI Port MAC Type
**				mii_type_rate			:  UNI Port MAC rate, Valid only if (mac_type == PON_MII)
**				mii_type_duplex			:  UNI Port MAC type, Valid only if (mac_type == PON_MII)
**				autonegotiation			:  Auto-negotiation activation 					
**				master					:  Master \ slave mode
**				advertise				:  Enable \ disable advertise in UNI port
**				advertisement_details	:  Advertising details
**				mdio_phy_address		:  MDIO PHY address
**
** Return codes:
**				All the defined PAS_* return codes, 
**
*/

extern PON_STATUS PAS_test_onu_uni_port_mac_configuration_v4 ( 
									const PON_olt_id_t							 olt_id, 
									const PON_onu_id_t							 onu_id,
									const PON_oam_uni_port_mac_configuration_t  *mac_configuration );

extern PON_STATUS PAS_set_onu_uni_port_mac_configuration_v4 ( 
									const PON_olt_id_t							 olt_id, 
									const PON_onu_id_t							 onu_id,
									const PON_oam_uni_port_mac_configuration_t  *mac_configuration );


/* Get ONU UNI port MAC configuration
**
** Get configuration of ONU UNI (System) port MAC. 
**
** Input Parameters:
**		 olt_id							:  OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 onu_id							:  ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
** Output Parameters:
**			mac_configuration
**				mac_type				:  UNI Port MAC Type
**				mii_type_rate			:  UNI Port MAC rate, Valid only if (mac_type == PON_MII)
**				mii_type_duplex			:  UNI Port MAC type, Valid only if (mac_type == PON_MII)
**				autonegotiation			:  Auto-negotiation activation 					
**				master					:  Master \ slave mode
**				advertise				:  Enable \ disable advertise in UNI port
**				advertisement_details	:  Advertising details
**				mdio_phy_address		:  MDIO PHY address
**
** Return codes:
**				All the defined PAS_* return codes, 
**
*/

extern PON_STATUS PAS_get_onu_uni_port_mac_configuration_v4 ( 
									const PON_olt_id_t					   olt_id, 
									const PON_onu_id_t					   onu_id,
									PON_oam_uni_port_mac_configuration_t  *mac_configuration );


/* Get ONU IGMP table
**
** Query the IGMP addresses set to an ONU.
**
** Input Parameters:
**			olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id					 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**
** Output Parameters:
**			number_of_addresses		 : Number of IGMP MAC addresses 'listened' by the ONU, 
**									   range: 0 - PON_MAX_IGMP_ADDRESSES_PER_ONU
**			mac_addresses			 : Array of IGMP MAC addresses 'listened' by the ONU. 
**									   First number_of_addresses records are filled
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_get_onu_igmp_table_v4 
                                 ( const PON_olt_id_t   olt_id, 
								   const PON_onu_id_t   onu_id, 
								   short int		   *number_of_addresses,
								   mac_address_t		mac_addresses[PON_MAX_IGMP_ADDRESSES_PER_ONU] );


/* Get ONU IGMP GDA table
**
** Query the IGMP GDA addresses set to an ONU.
**
** Input Parameters:
**			olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id					 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**
** Output Parameters:
**			number_of_addresses		 : Number of IGMP MAC addresses 'listened' by the ONU, 
**									   range: 0 - PON_MAX_IGMP_ADDRESSES_PER_ONU
**			gda_addresses			 : Array of IGMP GDA addresses 'listened' by the ONU. 
**
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_get_onu_igmp_gda_table_v4 
                    ( const PON_olt_id_t   olt_id, 
					  const PON_onu_id_t   onu_id, 
					  short int		      *number_of_addresses,
					  unsigned long		   gda_addresses[PON_MAX_IGMP_ADDRESSES_PER_ONU] );


/* Flush ONU IGMP table
**
** Resets the IGMP table for a specific ONU
**
** Input Parameters:
**			olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id					 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**
** 
** Return codes:
**				All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_test_flush_onu_igmp_table_v4 
                                 ( const PON_olt_id_t   olt_id, 
								   const PON_onu_id_t   onu_id);

extern PON_STATUS PAS_flush_onu_igmp_table_v4 
                                 ( const PON_olt_id_t   olt_id, 
								   const PON_onu_id_t   onu_id);



/* Get device versions
**
** Get software and hardware versions of a specified device (OLT / ONU).
** ONU doesn't have a firmware nor a host in current version, so the corresponding versions fields are 
** zeroed.
** Assumption: the caller must allocate the memory for the returned data structure before calling the 
** function, and deallocate it after the function returns.
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    device_id : Device id, PON_OLT_ID                                       - OLT device
**									   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  - ONU device
**
** Output Parameters (PAS_device_versions_t struct):
**							                    OLT device				            ONU device
**							                    ----------				            -----------
**				host_major	            :  PAS-SOFT major version	          ONU host major version
**				host_minor	            :  PAS-SOFT minor version	          ONU host minor version
**				host_compilation        :  PAS-SOFT compilation number        ONU host compilation number
**				firmware_major          :  OLT firmware major version         ONU firmware major version
**				firmware_minor          :  OLT firmware minor version         ONU firmware minor version
**              build_firmware             OLT firmware build version         ONU firmware build version
**              maintenance_firmware       OLT firmware maintenance version   ONU firmware maintenance version
**				hardware_major          :  OLT hardware major version         ONU hardware major version
**				hardware_minor          :  OLT hardware minor version         ONU hardware minor version
**				system_mac              :  OLT CNI port MAC type              ONU UNI port MAC type
**				ports_supported         :  Number of supported LLIDs	      number of LLIDs supported by the ONU
**									       in the PON		
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_device_versions_v4 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_device_id_t	device_id,
									PAS_device_versions_t  *device_versions );


/* Ping
**
** Ping a data frame to LLID. The pinged data can be explicitly parameterized or PAS-SOFT internal 
** generated. Successful return code is returned only after the data returned from the ONU (via a 
** 'Pong' frame) matches the data 'pinged'.
**
** Input Parameters:
**		 olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid			: Pinged LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**		 data_length	: Ping frame data length, measured in bytes. Values:
**						  0									  - PAS-SOFT generates ping data.
**						  1 - PON_ONU_OAM_FRAME_MAXIMUM_SIZE  - The number of octets that will be taken 
**																from 'data' parameter to be used as the pinged frame
**						  If bigger than maximum allowed data for a specific ONU - the data will be clipped. 
**						  If lower than minimum - a zero padding will be added to the pinged data.
**		 data			: Ping frame data, values: != NULL if (data_length > 0)
**
** Output Parameter:
**		 elapsed_time	: Total Ping time (not including PAS_SOFT overhead) 
**						  values: 0			    - Ping time below time measurement resolution	
**								  1 - MAXINT    - Total Ping time, measured in Milliseconds,
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_ping_v4 
                   ( const PON_olt_id_t olt_id, 
					 const PON_llid_t   llid,
					 const short int    data_length,
					 const void        *data,
					 short int		   *elapsed_time );

extern PON_STATUS PAS_ping_v4 
                   ( const PON_olt_id_t olt_id, 
					 const PON_llid_t   llid,
					 const short int    data_length,
					 const void        *data,
					 short int		   *elapsed_time );
#endif

/***********        PAS-SOFT 5001 V4.16.2 end  *****************/


/* GPIO access
**
** Set the GPIO (General purpose IO) output lines into new states and read the states of the input lines.
** Currently, two of the PAS5001 four GPIO lines are available for Host manipulation.
**
** Input Parameters:
**		 olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 gpio_line_1_io		: Input / Output / Not changed line. Values: enum values
**		 gpio_line_1_state  : GPIO output line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**		 gpio_line_2_io		: Input / Output / Not changed line. Values: enum values
**		 gpio_line_2_state  : GPIO output line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**
** Output Parameter:
**		 gpio_line_1_state  : GPIO line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**		 gpio_line_2_state  : GPIO line active high / low. Values: PON_VALUE_NOT_CHANGED, OFF, ON
**
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_gpio_access 
                              ( const PON_olt_id_t			olt_id, 
							    const PON_gpio_lines_t		line_number,
							    const PON_gpio_line_io_t    set_direction,
							    const short int		  	    set_value,
							    PON_gpio_line_io_t		   *direction,
							    bool					   *value );


/* Init	
**
** Init Passave API (PON software inside PAS-SOFT). No other API function may be called prior or during 
** function process time.
**
** Input Parameter:
**
** Return codes:
**			PAS_EXIT_OK						: Success
**			PAS_EXIT_ERROR					: General Error
**			PAS_PARAMETER_ERROR				: Function parameter error
*/
extern PON_STATUS PAS_init ( void );

/* Terminate									
**
** Shutdown Passave API software and all the connected PAS5001 devices.
** The only valid command after terminate is init (PAS_init function),
** other functions will be discarded.
**
** Input Parameter:
**			    None
**
** Return codes:
**				All the defined PAS_* return codes
**						
*/
extern PON_STATUS PAS_terminate ( void );


/*
** Hardware activation commands     							
*/

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
**				All the defined PAS_* return codes
**						
*/
extern PON_STATUS PAS_add_olt 
                    ( const PON_olt_id_t				          olt_id, 
				      const PON_olt_initialization_parameters_t  *olt_initialization_parameters,
                            PON_add_olt_result_t                 *add_olt_result );


/* Reset OLT									
**
** This function is to be called there is a need to reset OLT .
** The function removes all PON software internal OLT references and
** reset the OLT 
**
** Input Parameter:
**			    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Return codes:
**				All the defined PAS_* return codes
**						
*/
extern PON_STATUS PAS_test_reset_olt ( const PON_olt_id_t  olt_id );

extern PON_STATUS PAS_reset_olt ( const PON_olt_id_t  olt_id );

/* Deregister ONU
**
** After ONU has powered up and registered, this function removes the ONU from the network, enabling it
** to reregister at a later time. If the ONU is working well, it will reregister to the network 
** immediately after it detect the deregistration (lack of transmission grants). 
** Note: this command is to be followed by ONU deregistration event (PAS_onu_deregistration_handler_t),
** indicating the (near to) completion of the deregistration process.
** The function may return after sending request for ONU deregistration, or after the deregistration 
** process has completed.
**
** Input Parameters:
**		olt_id							   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id							   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		wait_for_deregistration_completion : Should the function return after the deregistration process has
**											 completed: ONU is verified as deregistered, and ONU 
**											 deregistration event has been raised. Values: TRUE / FALSE
**
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_deregister_onu 
                             ( const PON_olt_id_t  olt_id, 
							   const PON_onu_id_t  onu_id,
							   const bool		   wait_for_deregistration_completion );

extern PON_STATUS PAS_deregister_onu 
                             ( const PON_olt_id_t  olt_id, 
							   const PON_onu_id_t  onu_id,
							   const bool		   wait_for_deregistration_completion );

short int Deregister_onu ( const short int	   olt_id, 
						   const PON_onu_id_t  onu_id,
						   const bool		   wait_for_deregistration_completion, 
                           const bool          send_to_fw );

/* Get ONU mode									
**
** Query the operation (activation) mode of an ONU.
**
** Input Parameters:
**			    olt_id	: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    onu_id  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
**
** Return codes++:
**				PON_ONU_MODE_ON			  : The activation mode of the ONU is On
**				PON_ONU_MODE_OFF		  : The activation mode of the ONU is Off
**				PON_ONU_MODE_PENDING	  : The activation mode of the ONU is Pending (registered and 
**											waiting to be authorized)
**				PAS_* error return codes  : All <0 default return codes
**
** ++When Automatic authorization policy is activated, the activation mode can 
**   be either PON_ONU_MODE_ON or PON_ONU_MODE_OFF
**						
*/
extern PON_STATUS PAS_get_onu_mode 
                           ( const PON_olt_id_t  olt_id, 
							 const PON_onu_id_t  onu_id );

/* Get LLID parameters 
**
** Query LLID configuration parameters.
**
** Input Parameter:
**		olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid					 : Queried LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**
** Output Parameter:
**		llid_parameters			 : A struct containing the LLID configurations, see PON_llid_parameters_t 
**								   definition for details
**        
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_llid_parameters 
                                ( const PON_olt_id_t	 olt_id, 
								  const PON_llid_t		 llid, 
								  PON_llid_parameters_t  *llid_parameters );

/* Authorize ONU
**
** Authorize / deny an ONU to the network after it was authenticated by a system application.
** An ONU not authorized to the network can't send frames in the PON upstream direction, which means 
** most of the ONU related functions are not applicable to it. The function overrides the ONU's previous 
** authorization state, determined either by the default authorization state or by a prior call to this 
** function. Default authorization state for any ONU is determined by automatic_authorization_policy 
** field of PAS_pon_initialization_parameters_t parameterized in the PAS_init function.
** This function may be called within the ONU registration event handler (PAS_onu_registration_handler_t)
** ,but it is recommended that it will be called afterwards, for better network uptime performance.
** ONU authorization event (of type PAS_onu_authorization_handler_t) may override this function updates.
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    onu_id			: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**				authorize_mode	: Authorize / deny to the network. values: enumerated values
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_authorize_onu 
                            ( const PON_olt_id_t		  olt_id, 
							  const PON_onu_id_t		  onu_id,
							  const PON_authorize_mode_t  authorize_mode );

extern PON_STATUS PAS_authorize_onu 
                            ( const PON_olt_id_t		  olt_id, 
							  const PON_onu_id_t		  onu_id,
							  const PON_authorize_mode_t  authorize_mode );


/* Get olt versions
**
** Get software and hardware versions of a specified OLT.
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters (PAS_device_versions_t struct):
**							                    OLT device				    
**							                    ----------				    
**				host_major	            :  PAS-SOFT major version	        
**				host_minor	            :  PAS-SOFT minor version	        
**				host_compilation        :  PAS-SOFT compilation number      
**				firmware_major          :  OLT firmware major version       
**				firmware_minor          :  OLT firmware minor version       
**              build_firmware             OLT firmware build version       
**              maintenance_firmware       OLT firmware maintenance version 
**				hardware_major          :  OLT hardware major version       
**				hardware_minor          :  OLT hardware minor version       
**				system_mac              :  OLT CNI port MAC type            
**				ports_supported         :  Number of supported LLIDs	    
**									       in the PON		
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS  PAS_get_olt_versions (const PON_olt_id_t olt_id, PAS_device_versions_t * device_versions);


/*----------------------------------------------------------*/
/*						return codes						*/
/*----------------------------------------------------------*/
#define PASCOMM_EXIT_OK						EXIT_OK
#define PASCOMM_EXIT_ERROR					EXIT_ERROR
#define PASCOMM_TIME_OUT					TIME_OUT 
#define PASCOMM_NOT_IMPLEMENTED				NOT_IMPLEMENTED
#define PASCOMM_PARAMETER_ERROR				PARAMETER_ERROR
#define PASCOMM_HARDWARE_ERROR				HARDWARE_ERROR
#define PASCOMM_MEDIUM_ERROR				(-2001)
#define PASCOMM_PARSE_MSG_ERROR				(-2002)
#define PASCOMM_PARSE_RESPONSE_MSG_ERROR	(-2003)
#define PASCOMM_OLT_NOT_EXIST				(-2004)
#define PASCOMM_HOST_RESOURCE_ERROR			(-2005)
#define PASCOMM_CONSTRUCT_MSG_ERROR			(-2006)
#define PASCOMM_HARDWARE_NOT_AVAILABLE		(-2007)
#define PASCOMM_ONU_MANAGEMENT_LINK_CLOSE	(-2008)
#define PASCOMM_REMOTE_TIMEOUT				(-2009)
#define PASCOMM_ONU_UNSTABLE_FOR_MANAGEMENT (-2010)
#define PASCOMM_OLT_WAS_RESETED             (-2011)

/* PASCOMM_init_olt
**
** Initialize communication with an OLT.
** The physical characteristics of the OLT are transparent to PASCOMM and PAS modules
**
** Parameters:
**
**		olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Return values:
**		PASCOMM_EXIT_OK					: No error
**		other							: One of the error codes specified below
*/
extern short int PASCOMM_init_olt ( const short int  olt_id);
extern short int Get_device_versions ( const short int			    olt_id, 
								const PON_device_id_t	    device_id,
								PAS_device_versions_t      *device_versions,
                                PON_remote_mgnt_version_t  *remote_mgnt_version,
                                unsigned long int          *critical_events_counter);


/* Get OLT optics parameters
**
** Get the optics parameters for an OLT 
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      optics_configuration : OLT Optics configuration. See PON_olt_optics_configuration_t
**      pon_tx_signal        : PON TX signal, range : DISABLE..ENABLE
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_olt_optics_parameters 
                                    (const PON_olt_id_t                        olt_id,
                                           PON_olt_optics_configuration_t     *optics_configuration,
                                           bool                               *pon_tx_signal );

/* Get onu versions
**
** Get software and hardware versions of a specified ONU.
** ONU doesn't have a firmware nor a host in current version, so the corresponding versions fields are 
** zeroed.
** Assumption: the caller must allocate the memory for the returned data structure before calling the 
** function, and deallocate it after the function returns.
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    device_id : Device id, PON_OLT_ID                                       - OLT device
**									   PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  - ONU device
**
** Output Parameters (PAS_device_versions_t struct):
**							                        ONU device
**							                        -----------
**				host_major	            :     ONU host major version
**				host_minor	            :     ONU host minor version
**				host_compilation        :     ONU host compilation number
**				firmware_major          :     ONU firmware major version
**				firmware_minor          :     ONU firmware minor version
**              build_firmware                ONU firmware build version
**              maintenance_firmware          ONU firmware maintenance version
**				hardware_major          :     ONU hardware major version
**				hardware_minor          :     ONU hardware minor version
**				system_mac              :     ONU UNI port MAC type
**				ports_supported         :     number of LLIDs supported by the ONU
**									       
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS REMOTE_PASONU_get_onu_versions 
                                  ( const PON_olt_id_t  	olt_id, 
									const PON_onu_id_t	    onu_id,
									PAS_device_versions_t  *device_versions );

/*
extern short int PAS_get_device_versions ( const short int			olt_id, 
									const PON_device_id_t	device_id,
									PAS_device_versions_t  *device_versions );
*/

/* Get device capabilities
**
** Get several physical capabilities of an OLT / ONU device.
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    device_id : Device id, PON_OLT_ID		                                 - OLT device
**								       PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT   - ONU device
**
** Output Parameters (PAS_physical_capabilities_t struct):
**										 OLT device			ONU device
**										 ----------			-----------
**				laser_on_time		  : Not available (0)	Time takes the ONU laser to turn on
**				laser_off_time		  : Not available (0)   Time takes the ONU laser to turn off
**				onu_grant_fifo_depth  : Not available (0)   (PON_MAX_ONU_REPETITIONS_PER_GRANT_CYCLE*2) for PAS6001
**				agc_lock_time		  : OLT AGC lock time	Not available (0)
**				cdr_lock_time		  : OLT RX Signal sync.	Not available (0)
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
/*
extern short int PAS_get_device_capabilities ( const short int				  olt_id,
										const PON_device_id_t		  device_id,
										PAS_physical_capabilities_t  *device_capabilities );
*/
 

/* Retrieve ONU registration data as kept in the ONU address table */
extern PON_STATUS PAS_get_onu_registration_data 
                                        ( const PON_olt_id_t   		       olt_id, 
										  const PON_onu_id_t			   onu_id,
										  onu_registration_data_record_t  *onu_registration_data  );

/* Get onu hardware version
**
** Get hardware version of specific onu.
**
** Input Parameters:
**      olt_id                  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id			        : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
** Output Parameters:
**      onu_versions            : onu versions. See PON_onu_versions for details.
** 
** Return codes:
**      All the defined         PAS_* return codes
*/
extern short int PAS_get_onu_version ( const PON_olt_id_t    olt_id,
                                const PON_onu_id_t	  onu_id,
                                PON_onu_versions     *onu_versions );

/* The function belongs to PAS_olt_table, The decleration has moved to PAS_non_api_expo.h */
/* Because the CLI module need to call this function */

extern bool Olt_exists ( const short int olt_id );

extern bool Onu_exist ( const short int     olt_id, 
				 const PON_onu_id_t  onu_id );



/* The functions  Mac_address_exist & Get_onu_mac_address belong to PAS_onu_table,  */
/* The decleration has moved to PAS_non_api_expo.h Because the CLI module need      */
/* to call this function															*/


extern bool Mac_address_exist ( const short int	   olt_id, 
						 const mac_address_t   onu_mac, 
						 PON_onu_id_t         *onu_id_of_mac );

extern short int Get_onu_mac_address ( const short int	    olt_id, 
								const PON_onu_id_t  onu_id,
								mac_address_t       onu_mac );


/* Get ONU parameters
**
** Get a report containing registered ONUs details. Only existent ONUs are placed in the returned 
** structure, ordered by ONU id.
** Assumption: the caller must allocate the memory required for the ONU report before calling the 
** function, and deallocate it after the function returns.
**
**
** Input Parameters:
**		olt_id							   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**		number							   : Returned number of registered ONUs, also the 
**											 number of filled records in the ONU report. 
**											 range: 0 - (PON_MAX_ONU_ID_PER_OLT - PON_MIN_ONU_ID_PER_OLT+1)
**		onu_parameters_record			   : Pointer to a memory structure where the ONUs parameters 
**											 report will be written to, see PAS_onu_parameters_record_t  
**											 definition for details
**			mac_address (per ONU record)   : ONU MAC address
**			llid (per ONU record)		   : LLID
**			rtt	(per ONU record)		   : Round Trip Time
**				
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_onu_parameters 
                                 ( const PON_olt_id_t	  olt_id, 
								   short int			 *number, 
								   PAS_onu_parameters_t   onu_parameters );


short int Get_llid_parameters ( const PON_olt_id_t	            olt_id, 
		 					    const PON_llid_t	            llid, 
		    					PON_llid_parameters_t          *llid_parameters, 
                                onu_registration_data_record_t *registration_data,
                                PON_authentication_sequence_t  *authentication_sequence);


/* Set address table configuration
**
** Set an OLTs' address table configuration
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      address_table_config : Address table config structure, see PON_address_table_config_t
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_address_table_configuration 
                                              (const PON_olt_id_t                    olt_id,
                                               const PON_address_table_config_t      address_table_config );
extern PON_STATUS PAS_set_address_table_configuration 
                                              (const PON_olt_id_t                    olt_id,
                                               const PON_address_table_config_t      address_table_config );

/* Get address table configuration
**
** Get an OLTs' address table configuration 
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      address_table_config : Address table config structure, see PON_address_table_config_t
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_address_table_configuration 
                                              (const PON_olt_id_t                    olt_id,
                                                     PON_address_table_config_t     *address_table_config );


/* Set ONU UNI port MAC configuration
**
** Set new configuration for ONU UNI (System) port MAC. When ONU powers up, it reads the MAC configuration
** from its EEPROM, this command overrides the configuration until the next reset \ shutdown. 
** Default EEPROM configuration may be changed only by re-burning.
**
** Input Parameters:
**		 olt_id							:  OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 onu_id							:  ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
**			mac_configuration
**				mac_type				:  UNI Port MAC Type
**				mii_type_rate			:  UNI Port MAC rate, Valid only if (mac_type == PON_MII)
**				mii_type_duplex			:  UNI Port MAC type, Valid only if (mac_type == PON_MII)
**				autonegotiation			:  Auto-negotiation activation 					
**				master					:  Master \ slave mode
**				advertise				:  Enable \ disable advertise in UNI port
**				advertisement_details	:  Advertising details
**				mdio_phy_address		:  MDIO PHY address
**
** Return codes:
**				All the defined PAS_* return codes, 
**
*/
extern PON_STATUS REMOTE_PASONU_test_onu_uni_port_mac_configuration ( 
									const PON_olt_id_t							 olt_id, 
									const PON_onu_id_t							 onu_id,
									const PON_oam_uni_port_mac_configuration_t  *mac_configuration );

extern PON_STATUS REMOTE_PASONU_set_onu_uni_port_mac_configuration ( 
									const PON_olt_id_t							 olt_id, 
									const PON_onu_id_t							 onu_id,
									const PON_oam_uni_port_mac_configuration_t  *mac_configuration );

/* this api is moved to REMOTE Management Package
extern short int PAS_set_onu_uni_port_mac_configuration ( 
									const short int								 olt_id, 
									const PON_onu_id_t							 onu_id,
									const PON_oam_uni_port_mac_configuration_t  *mac_configuration );
*/

/* Load OLT binary
**
** Load a binary file to an OLT device. 
** Used for firmware upgrade, and DBA algorithms loading (packed as DLLs).
** Note: If several functions are activated simultaneously (e.g. by several applications/threads), the 
** loadings will be performed sequentially by PAS-SOFT. All binary file preparations for reading, as 
** well as post-reading manipulations are done within the function (e.g. open file, close file).
** This function initiates an event which should be handled by PAS_load_olt_binary_completed_handler_t.
** The external file may contain a complete OLT firmware, or a DBA DLL compiled for Passave environment. 
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID  - A single OLT
**		olt_binary		: OLT binary definitions. See PON_binary_t for details
**					
** Return codes:
**				PAS_DBA_ALREADY_RUNNING - DBA algorithm is already running (from a DLL file), a new DBA DLL 
**										  file can't be loaded
**				All the defined PAS_* return codes
**
** Only when the load operation is finished, the function will return.
** If the function returns with an error code which is NOT PAS_PARAMETER_ERROR, PAS_DBA_ALREADY_RUNNING 
** or PAS_OLT_NOT_EXIST errors, there is very little to assume regarding the OLT state. It probably will 
** not function normally.
**
*/
extern PON_STATUS PAS_test_load_olt_binary
                             ( const PON_olt_id_t		   olt_id, 
		  					   const PON_indexed_binary_t  *olt_binary );

extern PON_STATUS PAS_load_olt_binary
                             ( const PON_olt_id_t		   olt_id, 
		  					   const PON_indexed_binary_t  *olt_binary );

/* Provisioning (DBA) Modes , index value range */
#define PON_PROVISIONING_MODE_STATIC_GRANTING 	0			/* Static granting		*/
#define PON_PROVISIONING_MODE_INTERNAL_DBA		1			/* OLT Internal DBA		*/
/* #define PON_PROVISIONING_MODE_EXTERNAL_DBA	2 - MAXUWORD - External DBA number	*/

typedef enum PON_module_state_t
{
	PON_MODULE_STATE_NOT_INIT,
	PON_MODULE_STATE_DURING_INIT,
	PON_MODULE_STATE_RUNNING,
	PON_MODULE_STATE_DURING_CLOSE
} PON_module_state_t;

/* Return PAS-SOFT state (running / init etc.)
**
** Input Parameters:
**				none
**
** Return values:
**				PAS-SOFT state represented by PON_module_state_t enum values
*/
PON_module_state_t Get_pas_soft_state ( void );

/* Set system parameters									
**
** Set (update) Passave API system parameters during runtime. The new parameters stay valid until another
** update / terminate function. Initial system parameters values are set either by PAS_init function 
** parameters or by PAS-SOFT default values.
**
** Input Parameter:
**			system_parameters			         : System parameters struct, 
**												   see PAS_system_parameters_t for details
**				statistics_sampling_cycle		 : Statistics sampling cycle
**				monitoring_cycle				 : Monitoring cycle 
**				host_olt_msgs_timeout			 : Timeout for receiving a response msg from OLT after a 
**												   msg has been sent to it by PAS-SOFT
**				olt_reset_timeout				 : The number of msgs sending timeouts after which OLT 
**												   is reset (applies for every OLT)
**				automatic_authorization_policy   : Auto-authorization policy for each registrating ONU
**
** Return codes:
**				PAS_EXIT_OK						 : Success
**				PAS_EXIT_ERROR					 : General Error
**				PAS_PARAMETER_ERROR				 : Function parameter error
*/
extern PON_STATUS PAS_test_system_parameters ( const PAS_system_parameters_t  *system_parameters );

extern PON_STATUS PAS_set_system_parameters ( const PAS_system_parameters_t  *system_parameters );

/* Get system parameters									
**
** Get Passave API system parameters during runtime. 
**
** Input Parameter:
**
**
** Output Parameters:
**			system_parameters			         : System parameters struct, 
**												   see PAS_system_parameters_t for details
**				statistics_sampling_cycle		 : Statistics sampling cycle
**				monitoring_cycle				 : Monitoring cycle 
**				host_olt_msgs_timeout			 : Timeout for receiving a response msg from OLT after a 
**												   msg has been sent to it by PAS-SOFT
**				olt_reset_timeout				 : The number of msgs sending timeouts after which OLT 
**												   is reset (applies for every OLT)
**				automatic_authorization_policy   : Auto-authorization policy for each registrating ONU
**
** Return codes:
**				All the defined PAS_* return codes
*/

extern PON_STATUS PAS_get_system_parameters ( PAS_system_parameters_t  *system_parameters );

/* Set OLT IGMP snooping mode
**
** Set IGMP snooping mode of a specific OLT 
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      igmp_configuration : IGMP configuration see PON_olt_igmp_configuration_t
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_olt_igmp_snooping_mode 
                                         (const PON_olt_id_t                      olt_id,
                                          const PON_olt_igmp_configuration_t      igmp_configuration );
extern PON_STATUS PAS_set_olt_igmp_snooping_mode 
                                         (const PON_olt_id_t                      olt_id,
                                          const PON_olt_igmp_configuration_t      igmp_configuration );

/* Get OLT IGMP snooping mode
**
** Get IGMP snooping mode of a specific OLT  
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      igmp_configuration : IGMP configuration see PON_olt_igmp_configuration_t
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_olt_igmp_snooping_mode 
                                         (const PON_olt_id_t                      olt_id,
                                                PON_olt_igmp_configuration_t     *igmp_configuration );


/* Set OLT cni port MAC configuration
**
** Set new configuration for OLT CNI (System)  port MAC (and PHY). 
** When OLT powers up, it reads the MAC configuration from its EEPROM, this function overrides the 
** configuration until the next reset or shutdown. Default EEPROM configuration may be changed only 
** by re-burning.
**
** Input Parameters:
**		 olt_id							:  OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 olt_ports_mac_configuration    :  Structure containing a specific OLT port configuration
**		 
** Return codes:
**				All the defined PAS_* return codes, 
**
*/
extern PON_STATUS PAS_test_olt_cni_port_mac_configuration 
                                    ( const PON_olt_id_t 	 				       olt_id,
									  const PON_olt_cni_port_mac_configuration_t  *olt_cni_port_mac_configuration );

extern PON_STATUS PAS_set_olt_cni_port_mac_configuration 
                                    ( const PON_olt_id_t 	 				       olt_id,
									  const PON_olt_cni_port_mac_configuration_t  *olt_cni_port_mac_configuration );

/* Get OLT CNI port MAC configuration 
**
** Query current OLT CNI port MAC configuration. OLT CNI port MAC configuration can be updated by 
** PAS_set_olt_cni_port_mac_configuration API function.
**
** Input Parameter:
**		olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameter:
**		olt_cni_port_mac_configuration  : Structure containing a specific OLT port configuration
**		 
** Return codes:
**		All the defined PAS_* return codes
**
*/

extern PON_STATUS PAS_get_olt_cni_port_mac_configuration 
                                               ( const PON_olt_id_t					   olt_id, 
											     PON_olt_cni_port_mac_configuration_t  *olt_cni_port_mac_configuration );


/* Update OLT parameters
**
** Update OLT parameters during run-time. Zero or more OLT parameters may be updated (Each parameter 
** has non-update value).
** The macro PON_EMPTY_UPDATE_OLT_PARAMETERS_STRUCT 'empties' PON_olt_update_parameters_t struct (so each
** parameter has non-update value)
**
** Input Parameters:
**		olt_id						 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		updated_parameters			 : A struct containing parameters to be updated. Parameters
**									   which have PON_VALUE_NOT_CHANGED value are not updated.Every 
**									   parameter which should be updated has non PON_VALUE_NOT_CHANGED
**									   value. 
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_update_olt_parameters 
			  ( const PON_olt_id_t					olt_id,
				const PON_update_olt_parameters_t  *updated_parameters );

extern PON_STATUS PAS_update_olt_parameters 
			  ( const PON_olt_id_t					olt_id,
				const PON_update_olt_parameters_t  *updated_parameters );

/* Get OLT parameters 
**
** Query of advanced OLT configuration parameters, corresponding to the ones updated by 
** PAS_update_olt_parameters API function.
**
** Input Parameter:
**		olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameter:
**		olt_parameters			 : A struct containing the OLT parameters 
**        
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_olt_parameters  
                                  ( const PON_olt_id_t			    olt_id,
							        PON_olt_response_parameters_t  *olt_parameters );

/* Link test
**
** LLID link test including delay measurement option. The function tests a link to a single LLID over a
** period of time or once, using a variable number of randomally patterned measurement frames. 
** The length of the measurement frames is also parameterized. 
** Two test activation modes supported:
** Defined test capacity - Number of measurement frames used during the test is explicitly defined. 
** The function returns after all the frames were sent and received (or timed out), and includes minimal,
** mean and maximal delay results (if requested). 
** Start / Stop - Activating the test for unspecified time is done using CONTINUOUS number of frames, 
** returned parameters are not valid after activation. Stop test can stop an already active 'Defined test 
** capacity' link test.
** Stopping Link test is done using PAS_LINK_TEST_OFF value for number_of_frames parameter, only then the 
** returned parameters are valid. 
** Consecutive 'starts' / consecutive 'stops' / activating two tests simultaneously for the same OLT are
** all forbidden.
**
** Link test considerations: 
** - During link test, the tested LLID is found in a MAC loopback mode. See PAS_set_test_mode for details.
** - Only a singe Link test can be performed simultaneously for an active OLT
** - Link test can be performed only for LLID which supports OAM version 2.0 or higher
**
** Input Parameters:
**	   olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	   llid						 : Tested LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**	   number_of_frames			 : Activate the test / stop running test. Values: 
**								   PON_LINK_TEST_CONTINUOUS - Start a continuous link test. Results are 
**															  not returned.
**								   PON_LINK_TEST_OFF		- Stop an already running link test. Results 
**															  are returned.
**								   1 - PON_LINK_TEST_MAX_FRAMES	- Perform a complete test using the specified 
**															      number of frames. Results are returned.
**	   frame_size				 : Measurement frames data size, measured in bytes. Not including Preamble, SFD, 
**								   VLAN tag and FCS. When vlan_configuration.vlan_frame_enable parameter is set 
**								   to ENABLE, the actual frame size sent including FCS may reach 
**								   (MAX_VLAN_ETHERNET_FRAME_SIZE_STANDARDIZED + 4 = ) 1522 bytes.
**								   Range: MIN_ETHERNET_FRAME_SIZE - MAX_ETHERNET_FRAME_SIZE_STANDARDIZED
**     link_delay_measurement    : Link test includes / doesn't include delay measurement for each 
**								   measurement frame. Values: DISABLE / ENABLE.
**     
**	   vlan_params:				 : PON_link_test_vlan_configuration_t link test vlan configuration																																						
** Output Parameter:
**	   test_results				 : Link test results struct. Allocated by the caller. Updated only if the 
**								   link test is completed within current function call.
** Return codes:
**	   PAS_NOT_IMPLEMENTED		 : LLID OAM version doesn't support Link test
**	   All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_link_test 
                        ( const PON_olt_id_t						 olt_id, 
						  const PON_llid_t							 llid,
						  const short int					         number_of_frames,
						  const short int							 frame_size,
						  const bool								 link_delay_measurement,
						  const PON_link_test_vlan_configuration_t  *vlan_configuration,	
						  PON_link_test_results_t					*test_results );

extern PON_STATUS PAS_link_test 
                        ( const PON_olt_id_t						 olt_id, 
						  const PON_llid_t							 llid,
						  const short int					         number_of_frames,
						  const short int							 frame_size,
						  const bool								 link_delay_measurement,
						  const PON_link_test_vlan_configuration_t  *vlan_configuration,	
						  PON_link_test_results_t					*test_results );


/* Set loopback mode
**
** This command disables/enables OLT loopback
**
** Input Parameters:
**	olt_id							  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	loopback_type					  : Loopback type to activate \ deactivate, range: enumerated values
**									
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_set_loopback_mode (
	                               const PON_olt_id_t 		olt_id,
                                   const PON_loopback_t    	loopback_type );
	                               
extern PON_STATUS PAS_test_loopback_mode (
	                               const PON_olt_id_t 		olt_id,
                                   const PON_loopback_t    	loopback_type );

/* Set standard onu loopback
**
** This command disables/enables ONU standard MAC loopback
**
** Input Parameters:
**	olt_id			  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	onu_id			  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
**	loopback_mode	  : Enable or disable loopback, range: DISABLE or ENABLE
**									
** Return codes:
**		All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_standard_onu_loopback (
                                   	       const PON_olt_id_t      olt_id,
                                           const PON_onu_id_t      onu_id,
                                           const bool              loopback_mode );

extern PON_STATUS PAS_set_standard_onu_loopback (
                                   	       const PON_olt_id_t      olt_id,
                                           const PON_onu_id_t      onu_id,
                                           const bool              loopback_mode );


/* Set classification rule
**
** Set a classification filter for a specific OLT port
**
** Input Parameters:
**      olt_id                   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      direction                : PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK
**      classification_entity    : Classification entity that can be configured
**      classification_parameter : see PON_olt_classification_t for more details
**      destination              : classification destination
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_classification_rule 
                                      (const PON_olt_id_t                             olt_id,
                                       const PON_pon_network_traffic_direction_t      direction,
                                       const PON_olt_classification_t                 classification_entity,
                                       const void                                    *classification_parameter,
                                       const PON_olt_classifier_destination_t         destination );

extern PON_STATUS PAS_set_classification_rule 
                                      (const PON_olt_id_t                             olt_id,
                                       const PON_pon_network_traffic_direction_t      direction,
                                       const PON_olt_classification_t                 classification_entity,
                                       const void                                    *classification_parameter,
                                       const PON_olt_classifier_destination_t         destination );

/* Get classification rule
**
** Get a classification filter for a specific OLT port 
**
** Input Parameters:
**      olt_id                   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      direction                : PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK
**      classification_entity    : Classification entity that can be configured
**      classification_parameter : see PON_olt_classification_t for more details
** Output Parameters:
**      destination              : classification destination
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_classification_rule 
                                      (const PON_olt_id_t                             olt_id,
                                       const PON_pon_network_traffic_direction_t      direction,
                                       const PON_olt_classification_t                 classification_entity,
                                       const void                                    *classification_parameter,
                                             PON_olt_classifier_destination_t        *destination );

/* Ping
**
** Ping a data frame to LLID. The pinged data can be explicitly parameterized or PAS-SOFT internal 
** generated. Successful return code is returned only after the data returned from the ONU (via a 
** 'Pong' frame) matches the data 'pinged'.
**
** Input Parameters:
**		 olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid			: Pinged LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**		 data_length	: Ping frame data length, measured in bytes. Values:
**						  0									  - PAS-SOFT generates ping data.
**						  1 - PON_ONU_OAM_FRAME_MAXIMUM_SIZE  - The number of octets that will be taken 
**																from 'data' parameter to be used as the pinged frame
**						  If bigger than maximum allowed data for a specific ONU - the data will be clipped. 
**						  If lower than minimum - a zero padding will be added to the pinged data.
**		 data			: Ping frame data, values: != NULL if (data_length > 0)
**
** Output Parameter:
**		 elapsed_time	: Total Ping time (not including PAS_SOFT overhead) 
**						  values: 0			    - Ping time below time measurement resolution	
**								  1 - MAXINT    - Total Ping time, measured in Milliseconds,
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS REMOTE_PASONU_test_ping 
                   ( const PON_olt_id_t olt_id, 
					 const PON_llid_t   llid,
					 const short int    data_length,
					 const void        *data,
					 short int		   *elapsed_time );

extern PON_STATUS REMOTE_PASONU_ping 
                   ( const PON_olt_id_t olt_id, 
					 const PON_llid_t   llid,
					 const short int    data_length,
					 const void        *data,
					 short int		   *elapsed_time );

/* this api is moved to REMPTE Management Package 
extern short int PAS_ping ( const short int    olt_id, 
					 const PON_llid_t   llid,
					 const short int    data_length,
					 const void        *data,
					 short int		   *elapsed_time ); 
*/


/* Get statistics
**
** Get one statistics data computed statistics data. 
** Note: The statistics are buffered in a software statistics table.
**
** Input Parameters:
**			    olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    collector_id		  : Statistics collector, values:
**										PON_OLT_ID			                              - OLT device
**									 	PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT   - ONU device
**				statistics_type		  : The statistics type requested, range: enumerated values
**				statistics_parameter  : Each statistics type requires a different statistics parameter
**										Statistics type     Statistics parameter
**										---------------		--------------------
**										PON_STAT_ONU_BER	ONU id measured
**										PON_STAT_OLT_BER	0
**										PON_STAT_ONU_FER	ONU id measured
**										PON_STAT_OLT_FER	0
**				
** Output Parameter:
**				statistics_data	      : Statistics data measurement
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_statistics
                             ( const PON_olt_id_t		olt_id, 
							   const short int			collector_id, 
							   const PON_statistics_t   statistics_type, 
							   const short int		    statistics_parameter, 
							   long double			   *statistics_data );


/* Get raw statistics
**
** Get hardware counters statistics collected by an OLT / ONU device (a.k.a. the collector).
** See 'Raw statistics types' and 'Raw statistics types data structures details' definitions in 
** PON_expo.h file for raw statistics structures details.
** See API document for statistics_type - statistics_parameter - statistics_data valid sets of values.
** Note: Caller must allocate the returned data structure before calling the function, and cast it to 
** 'void' when calling the function.
**
** Input Parameters:
**		olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		collector_id		  : Statistics collector, values:
**								PON_OLT_AND_ONU_COMBINED_ID							- OLT & ONU devices 
**																					  combined statistics
**								PON_OLT_ID											- OLT device
**							 	PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT     - ONU device
**		raw_statistics_type	  : The raw statistics type requested, range: enumerated values
**		statistics_parameter  : Each statistics type requires a different statistics parameter
**				
** Output Parameter:
**		statistics_data	      : Raw statistics data structure (PON_*_raw_data_t structures
**								defined in PON_expo.h), allocated by the caller
**		timestamp			  : OLT statistics sample time. Valid only if an OLT or both OLT and ONU are
**								the statistics collector
**
** Return codes:
**				All the defined PAS_* return codes
**
*/ 


extern PON_STATUS PAS_get_raw_statistics 
                                 ( const PON_olt_id_t					 olt_id, 
								   const short int						 collector_id, 
								   const PON_raw_statistics_t			 raw_statistics_type, 
								   const short int						 statistics_parameter,
								   void								    *statistics_data,
								   PON_timestamp_t						*timestamp );


/* Get raw statistics async 
**
** The same function as PAS_get_raw_statistics that work only in the asynchronous driver version, 
** the function have a supplemental parameter to inform if the function work asynchronously or not
** Input Parameters:
**		olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		collector_id		  : Statistics collector, values:
**								PON_OLT_AND_ONU_COMBINED_ID							- OLT & ONU devices 
**																					  combined statistics
**								PON_OLT_ID											- OLT device
**							 	PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT     - ONU device
**		raw_statistics_type	  : The raw statistics type requested, range: enumerated values
**		statistics_parameter  : Each statistics type requires a different statistics parameter
**		collecting_method	  : The method used by PAS-SOFT to get the raw statistics, values:
**								PON_STATISTICS_QUERY_HARDWARE		  -	favor direct query of the raw 
**																		statistics from the various 
**																		hardware/s counters
**				
** Output Parameter:
**		statistics_data	      : Raw statistics data structure (PON_*_raw_data_t structures
**								defined in PON_expo.h), allocated by the caller
**		timestamp			  : OLT statistics sample time. Valid only if an OLT or both OLT and ONU are
**								the statistics collector
**      worked_async          : TRUE the function work asynchronously , FALSE the function work synchronously  
**
** Return codes:
**				All the defined PAS_* return codes
**
*/ 
extern PON_STATUS PAS_get_raw_statistics_async ( const short int						 olt_id, 
								         const short int						 collector_id,        
								         const PON_raw_statistics_t			     raw_statistics_type, 
								         const short int						 statistics_parameter,
								         const PON_statistics_query_method_t	 collecting_method,   
								         void								    *statistics_data,     
								         PON_timestamp_t						*timestamp, 
                                         bool                                   *worked_async );     


/* Set downlink buffer configuration
**
** Set downlink datapath buffer configuration for an OLT. Downlink buffer existence is defined in Add OLT API command
**
** Input Parameters:
**      olt_id          : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      priority_limits : priority limits
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_downlink_buffer_configuration 
                                                 (const PON_olt_id_t                               olt_id,
                                                  const PON_downlink_buffer_priority_limits_t      priority_limits );

extern PON_STATUS PAS_set_downlink_buffer_configuration 
                                                 (const PON_olt_id_t                               olt_id,
                                                  const PON_downlink_buffer_priority_limits_t      priority_limits );

/* Get downlink buffer configuration
**
** Get downlink datapath buffer configuration for an OLT. Downlink buffer existence is defined in Add OLT API command 
**
** Input Parameters:
**      olt_id          : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      priority_limits : priority limits
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_downlink_buffer_configuration 
                                                 (const PON_olt_id_t                               olt_id,
                                                        PON_downlink_buffer_priority_limits_t     *priority_limits );
                                         
#if 0
/* Set link alarm params, this function has been superseded by Set alarm configuration
**
** Configure alarm event generation thresholds. The alarm event will be invoked when one or more of 
** the conditions specified are met.
** PAS_init function resets all thresholds to their default values. 
** Threshold parameters are applicable for both the OLT and the ONUs of the OLT specified.
**
** Input Parameters:
**			    olt_id				  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    link_alarm_params	  : PAS_link_alarm_params_t link alarms parameters structure
**													
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern short int PAS_set_link_alarm_params ( const short int				  olt_id, 
								      const PAS_link_alarm_params_t  *link_alarm_params );

#endif

/* Set alarm configuration
**
** Configure event (PAS_alarm_handler_t) generation thresholds, overriding previous configuration. The 
** alarm event will be invoked when all the conditions specified are met. All alarms configurations are 
** disabled by default (which means only alarms without configurations may raise without activating this
** function). Every physical component activation also resets its specific alarm configuration to its 
** default value:
** - PAS_init						- resets all alarm configurations
** - PAS_add_olt					- reset OLT specific alarm configurations
** - PAS_onu_registration_handler_t - event indicates the ONU has its default alarm configurations
**
** Input Parameters:
**		  olt_id		: OLT id, Values: PON_ALL_ACTIVE_OLTS    - Applicable for all active OLTs
**										  PON_MIN_OLT_ID - PON_MAX_OLT_ID - Applicable for a single OLT
**		  source		: Source device for the configured larm, values: 
**							  PON_PAS_SOFT_ID								   - PAS-SOFT 
**							  PON_OLT_ID									   - OLT device / ALL ONUs
**							  PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  - ONU device
**        type			: Alarm type, range: enumerated values
**		  activate		: Enable / disable alarm monitoring for the specified alarm, 
**						  values: DISABLE / ENABLE
**		  configuration	: Relevant alarm configuration(PON_*_alarm_configuration_t casted to (void *) ): 
**						  threshold value, activation mode etc. See API document and 'Alarms details'
**						  definitions in PON_expo.h header for details and castings (different for every
**						  alarm type). NULL in a case of DISABLE value on 'activate' parameter.
**						
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_test_alarm_configuration 
                                    ( const PON_olt_id_t    olt_id, 
									  const short int	    source,
									  const PON_alarm_t     type,
									  const bool			activate,
									  const void		   *configuration );

extern PON_STATUS PAS_set_alarm_configuration 
                                    ( const PON_olt_id_t    olt_id, 
									  const short int	    source,
									  const PON_alarm_t     type,
									  const bool			activate,
									  const void		   *configuration );

/* Get alarm configuration
**
** Query an alarm configuration. Alarm configuration may have its default value, or a value previously
** updated by PAS_set_alarm_configuration API function.
** Note: the proper 'configuration' structure should be allocated by the caller to this function prior
** to the call. The function fills the structure if necessary (not all alarm types have configuration
** structure). After the function returns, the caller must free the allocated 'configuration' structure.
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		source			: Source device for the queried alarm, values: 
**						  PON_PAS_SOFT_ID								   - PAS-SOFT 
**						  PON_OLT_ID									   - OLT device / ALL ONUs
**						  PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  - ONU device
**      type			: Alarm type, range: enumerated values
**						
** Output Parameters:	
**		activated		: Is monitoring for the specified alarm enabled / disabled,
**						  values: DISABLE / ENABLE
**		configuration	: Relevant alarm configuration(PON_*_alarm_configuration_t casted to (void *) ): 
**						  threshold value, activation mode etc. See PAS5001 API document and 
**						  'Alarms details' definitions in PON_expo.h header file for the casting details 
**						  (different for every alarm type). Not changed in the case of DISABLE value of the 
**						  'activated' parameter, or nonexistence alarm type configuration (on-off alarms).
**						
** Return codes:
**		All the defined PAS_* return codes
**
*/
PON_STATUS PAS_get_alarm_configuration 
                                     ( const PON_olt_id_t olt_id, 
									   const short int	  source,
									   const PON_alarm_t  type,
									   bool			      *activated,
									   void		          *configuration );

/* Reset address table
**
** Reset the content of an OLT address table according to address type and LLID.
** Automatic MAC address learning will refill the table again
**	
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				llid            : 0 - PON_MAX_LLID_PER_OLT or PON_ALL_ACTIVE_LLIDS or
**                                PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID  
**              address_type	: Dynamic or Static or Dynamic & Static								
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_reset_address_table
                                           ( const PON_olt_id_t	        olt_id ,
                                             const PON_llid_t           llid   ,
                                             const PON_address_aging_t  address_type
                                           );

extern PON_STATUS PAS_reset_address_table
                                 ( const PON_olt_id_t	      olt_id ,
                                   const PON_llid_t           llid   ,
                                   const PON_address_aging_t  address_type
                                 );



/* Start DBA algorithm
**
** Activate a DBA algorithm for EPON upstream bandwidth allocation, former grant table will be overrided.
** The function activates the last DBA file loaded to the OLT, or the OLT internal default DBA algorithm
** (SLA based bandwidth for all LLIDs).
** Note: This function may be called for external DBA algorithm only after PAS_load_olt_binary was 
** called with a DBA DLL file.
**
** OLT internal DBA parameters values: 
**	+ olt_id					- The configured OLT id
**	+ use_default_dba			- TRUE
**  + initialization_data_size  - 0
**  + initialization_data		- NULL
**
** Input Parameters:
**			    olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				use_default_dba					: Activate OLT internal DBA algorithm or external DBA  
**												  algorithm loaded to the OLT. Values: TRUE / FALSE.
**				initialization_data_size		: Initialization data (buffer) size, measured in bytes.
**												  range: 0 - PON_MAX_DBA_DATA_SIZE
**				initialization_data				: Pointer to a memory address containing the DBA 
**												  initialization data. 
**												  May be NULL (only if initialization_data_size == 0).
**					
** Return codes:
**				PAS_DBA_DLL_NOT_LOADED  - Can't start external DBA algorithm without DLL file loaded
**										  to the OLT
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_test_start_dba_algorithm 
                                  ( const PON_olt_id_t   olt_id,
								    const bool		     use_default_dba,
									const short int	     initialization_data_size,
									const void		    *initialization_data );

extern PON_STATUS PAS_start_dba_algorithm 
                                  ( const PON_olt_id_t   olt_id,
								    const bool		     use_default_dba,
									const short int	     initialization_data_size,
									const void		    *initialization_data );

/* Stop external DBA algorithm
**
** Stop the currently active external upstream DBA algorithm. The OLT default internal DBA algorithm 
** (equal share of upstream bandwidth for each registered ONU) will be activated instead of the external
** DBA algorithm, so the ONUs will not get deregistered due to lack of bandwidth.
** A PAS_set_grants, PAS_start_dba_algorithm or PAS_load_olt_binary (DLL file) may follow this command.
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**					
** Return codes:
**			PAS_DBA_NOT_RUNNING - Error, external DBA algorithm is not currently running on the specified
**								  OLT
**			All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_stop_external_dba_algorithm ( const PON_olt_id_t  olt_id );

extern PON_STATUS PAS_stop_external_dba_algorithm ( const PON_olt_id_t  olt_id );


/* Set policing parameters
**
** Set limiting parameters (restrictions) for a LLID
**
** Input Parameters:
**      olt_id              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid                : LLID index, range: PON_MIN_LLID_PER_OLT- PON_MAX_LLID_PER_OLT,PON_BROADCAST_LLID
**      policer             : Policer type
**      enable              : Policing Enable / Disable, If disabled, policing_struct is ignored
**      policing_params     : Policing parameters, PON_policing_parameters_t structure
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_policing_parameters 
                                       (const PON_olt_id_t               olt_id,
                                        const PON_llid_t                 llid,
                                        const PON_policer_t              policer,
                                        const bool                       enable,
                                        const PON_policing_parameters_t  policing_params     );
extern PON_STATUS PAS_set_policing_parameters 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_llid_t                     llid,
                                           const PON_policer_t                  policer,
                                           const bool                           enable,
                                           const PON_policing_parameters_t      policing_params     );

/* Get policing parameters
**
** Get limiting parameters (restrictions) for a LLID 
**
** Input Parameters:
**      olt_id              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid                : LLID index, range: PON_MIN_LLID_PER_OLT- PON_MAX_LLID_PER_OLT,PON_BROADCAST_LLID
**      policer             : Policer type
** Output Parameters:
**      enable              : Policing Enable / Disable, If disabled, policing_struct is ignored
**      policing_params     : Policing parameters, PON_policing_parameters_t structure
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_policing_parameters 
                                       (const PON_olt_id_t               olt_id,
                                        const PON_llid_t                 llid,
                                        const PON_policer_t              policer,
                                              bool                      *enable,
                                              PON_policing_parameters_t *policing_params     );

/* OLT self test
**
** Activate an OLT self test and return the result		
**
** Input Parameters:
**			    olt_id	  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Return codes:
**				PAS_olt_test_results_t values
**
*/
extern PON_TEST_RESULT PAS_test_olt_self_test ( const PON_olt_id_t  olt_id );

extern PON_TEST_RESULT PAS_olt_self_test ( const PON_olt_id_t  olt_id );



/* Add address table record
**
** Add / update the content of an entry in the address table. 
** If the MAC address already found in the address table, the logical port is updated. 
** This function is generally used for multicast address learning.		
**
** Input Parameters:
**			    olt_id			  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    address_record    : PON_address_record_t struct
**					mac_address	  : Updated / added MAC address, may be unicast or multicast
**					logical_port  : Address record logical port, translates to ONU id / CNI port
**									in the current version,  
**									values: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**									PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID
**					type		  : Ignored, the address (host address) is always set to Static (STATIC)
**													
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_add_address_table_record 
                                       ( const PON_olt_id_t			  olt_id, 
										 const PON_address_record_t  *address_record );

extern PON_STATUS PAS_add_address_table_record 
                                       ( const PON_olt_id_t			  olt_id, 
										 const PON_address_record_t  *address_record );


extern short int Set_logging_configuration ( const PON_log_flag_t		   log_flag,
									  const bool				   mode,
									  const PON_log_redirection_t  redirection,
									  const short int			   logged_olt_id );

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

extern PON_STATUS PAS_test_assign_handler_function
                                     ( const PAS_handler_functions_index_t    handler_function_index, 
									   const void							(*handler_function)(),
									   unsigned short                        *handler_function_id);

extern PON_STATUS PAS_assign_handler_function
                                 ( const PAS_handler_functions_index_t    handler_function_index, 
								   const void							(*handler_function)(), 
								   unsigned short                          *handler_function_id);


/* Start encryption
**
** Start encrypting all downstream and optional upstream traffic destined to and originating from a specific
** LLID. ONU registration encryption key is used as initial encryption key. 
** If the encryption key was updated after ONU registration, the last updated key is used.
** The function returns either after the start encryption process has started in the OLT, or after the 
** process has been completed. If a handler function is assigned for start encryption event 
** (PAS_HANDLER_START_ENCRYPTION_ACKNOWLEDGE value for the handler_function_index parameter) 
** then this function returns after the start encryption process has started (and the function assigned 
** is called when the process ends). If no handler is assigned, this function returns when the process 
** ends (after ONU acknowledgement).
**
** Input Parameters:
**			    olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    llid				: LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**				direction			: Encryption direction. Values: enumerated values.
**					
** Return codes:
**				Handler function is assigned    : All the defined PAS_* return codes
**				Handler function isn't assigned : Start encryption return codes values (should be casted
**												  to PON_start_encryption_acknowledge_codes_t)
**
*/
extern PON_STATUS PAS_test_start_encryption 
                               ( const PON_olt_id_t			       olt_id, 
								 const PON_llid_t				   llid,
								 const PON_encryption_direction_t  direction );

extern PON_STATUS PAS_start_encryption 
                               ( const PON_olt_id_t			       olt_id, 
								 const PON_llid_t				   llid,
								 const PON_encryption_direction_t  direction );



/* Update encryption key
**
** Continue encrypting all downstream traffic destined to, and optional upstream traffic originating from
** a specific LLID using a new encryption key.
** This function may be called only after PAS_start_encryption (may be called several times).
** The function returns either after the key update process has started in the OLT, or after the 
** process has been completed. If a handler function is assigned for key update event 
** (PAS_HANDLER_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE value for the handler_function_index parameter) 
** then the function returns after the key exchange process has started (and the function assigned 
** is called when the process ends). If no handler is assigned, the function returns when the process 
** ends (after LLID acknowledgement).
**
** Input Parameters:
**		olt_id		      : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid		      : LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**		encryption_key    : Updated encryption key, range:(PON_ENCRYPTION_KEY_SIZE*BITS_IN_BYTE)bit value
**		key_update_method : Encryption update technique. Values: enum values.
**					
** Return codes:
**		Handler function is assigned    : All the defined PAS_* return codes
**		Handler function isn't assigned : Update encryption key return codes values (should be 
**										  casted to PON_update_encryption_key_acknowledge_codes_t)
**
*/																			
extern PON_STATUS PAS_test_update_encryption_key 
                                    ( const PON_olt_id_t				 olt_id, 
									  const PON_llid_t					 llid,
									  const PON_encryption_key_t		 encryption_key,
									  const	PON_encryption_key_update_t  key_update_method );

extern PON_STATUS PAS_update_encryption_key 
                                    ( const PON_olt_id_t				 olt_id, 
									  const PON_llid_t					 llid,
									  const PON_encryption_key_t		 encryption_key,
									  const	PON_encryption_key_update_t  key_update_method );

/* Stop encryption
**
** Stop encrypting all downstream and upstream traffic destined to and originating from a specific LLID. 
**
** Input Parameters:
**			    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    llid		   : LLID,   range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_stop_encryption 
                              ( const PON_olt_id_t   olt_id, 
						        const PON_llid_t     llid );

extern PON_STATUS PAS_stop_encryption 
                              ( const PON_olt_id_t   olt_id, 
						        const PON_llid_t     llid );

/* Set encryption configuration
**
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      encryption_type  : See PON_encryption_type_t for details
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_start_encryption_configuration(      
                  const PON_olt_id_t           olt_id,
                  const PON_encryption_type_t  encryption_type );

extern PON_STATUS PAS_set_encryption_configuration(      
                  const PON_olt_id_t           olt_id,
                  const PON_encryption_type_t  encryption_type );

/* Get encryption configuration
**
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      encryption_type  : See PON_encryption_type_t for details
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_encryption_configuration(      
                  const PON_olt_id_t      olt_id,
                  PON_encryption_type_t  *encryption_type );



extern PON_STATUS PLATO2_init( void );

extern PON_STATUS PLATO2_cleanup( void );

/* this function should be called by PAS_HANDLER_DBA_EVENT handler function
   when its olt_id parameter is an OLT which runs PLATO2 DBA */
extern PON_STATUS PLATO2_algorithm_event( short olt_id, unsigned short id, short size,
                                   const char *data );

typedef struct OLT_DBA_version_t
{
    char szDBAname[32];
    char szDBAversion[32];
}OLT_DBA_version_t;

extern PON_STATUS PLATO2_get_info( short olt_id, char *name,
                            unsigned char max_name_size, char *version, 
                            unsigned char max_version_size );           

extern PON_STATUS PLATO2_set_SLA( short olt_id, unsigned char LLID,
                           const PLATO2_SLA_t *SLA, short *DBA_error_code );

extern PON_STATUS PLATO2_get_SLA( short olt_id, unsigned char LLID,
                                 PLATO2_SLA_t *SLA, short *DBA_error_code );
                                 
extern PON_STATUS PLATO2_get_cht_delay_param_validity( bool *delay_param_valid );

extern PON_STATUS PLATO2_set_cht_delay_param_validity( bool delay_param_valid );



/* ONU deregistration handler
**
** An event indicating ONU was deregistered from the network. See API document for event handling 
** guidelines.
** Assumptions: ONU id may be reallocated in the future, either for the same physical ONU or for 
** another ONU.
**
** Input Parameters:
**			    olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				onu_id		: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**				Code		: Reason for the deregistration event. Values: enum values
**
** Return codes:
**				Return codes are not specified for an event
**
*/
typedef PON_STATUS  (*PAS_onu_deregistration_handler_t) 
				  ( const short int						 olt_id,
					const PON_onu_id_t					 onu_id,
					const PON_onu_deregistration_code_t  deregistration_code );

/* Set OLT register
**
** Set value to the OLT register
**
** Input Parameters:
**		olt_id			    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      register_address    : Register address.
**      register_value      : Register value.
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_test_olt_register 
                            ( const PON_olt_id_t   olt_id,
                              const unsigned long  register_address, 
		                      const unsigned long  register_value );

extern PON_STATUS PAS_set_olt_register 
                            ( const PON_olt_id_t   olt_id,
                              const unsigned long  register_address, 
		                      const unsigned long  register_value );

/* Get OLT register
**
** Read an OLT register content.
**
** Input Parameters:
**		olt_id			    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      register_address    : Register address.
**
** Output Parameters:
**      register_value      : Register value.
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/

extern PON_STATUS PAS_get_olt_register 
                            ( const PON_olt_id_t    olt_id,
                              const unsigned long   register_address, 
				              unsigned long        *register_value );
				  

/*----------------------------------------------------------*/
/*						PASCOMM msgs					    */
/*----------------------------------------------------------*/
/* When adding msg - update PSACOMM_msg.c functions and PASCOMM.c send and receive functions */
#if 0 /* V5.3.0以前版本  */

#endif

typedef enum
{
	PASCOMM_MSG_READ_OLT_REGISTERS=1,
	PASCOMM_MSG_OLT_REGISTER_CONTENT,
	PASCOMM_MSG_WRITE_OLT_REGISTERS,
	PASCOMM_MSG_OLT_DIRECT_ACK,
	PASCOMM_MSG_OLT_INIT,
	PASCOMM_MSG_SHUTDOWN_OLT,
	PASCOMM_MSG_SHUTDOWN_ONU,
	PASCOMM_MSG_DEREGISTER_ONU,
	PASCOMM_MSG_SEND_FRAME,
    PASCOMM_MSG_SEND_OAM_FRAME,
	PASCOMM_MSG_OLT_ACK,
	PASCOMM_MSG_GET_OLT_VERSION,
	PASCOMM_MSG_OLT_VERSION,
	PASCOMM_MSG_SET_ADDRESS_RECORD,
	PASCOMM_MSG_DELETE_ADDRESS_RECORD,
	PASCOMM_MSG_GET_ADDRESS_TABLE,
	PASCOMM_MSG_ADDRESS_TABLE,
    PASCOMM_MSG_GET_ADDRESS_TABLE_LLID,
	PASCOMM_MSG_ADDRESS_TABLE_LLID,
	PASCOMM_MSG_SET_GRANT_RECORDS,
	PASCOMM_MSG_SET_STATIC_GRANTING_CONFIG,
	PASCOMM_EVENT_ALARM,							 /*event*/
	PASCOMM_EVENT_FRAME_RECEIVED,					 /*event*/
	PASCOMM_EVENT_ONU_REGISTRATION,					 /*event*/
	PASCOMM_EVENT_OLT_RESET,						 /*event*/
	PASCOMM_EVENT_ONU_DEREGISTRATION,				 /*event*/
	PASCOMM_MSG_SET_OLT_LOAD_BINARY,
	PASCOMM_MSG_OLT_BINARY_DATA,
	PASCOMM_MSG_FINISH_OLT_LOAD_BINARY,
	PASCOMM_MSG_SET_ONU_LOAD_BINARY,
	PASCOMM_MSG_ONU_BINARY_DATA,
	PASCOMM_MSG_FINISH_ONU_LOAD_BINARY,
	PASCOMM_MSG_UPDATE_ONU_IMAGE_INDEX,
	PASCOMM_EVENT_LOAD_OLT_BINARY_COMPLETED,		 /*event*/
	PASCOMM_MSG_ONU_ID_AUTHORIZATION,
	PASCOMM_MSG_OLT_SELF_TEST,
	PASCOMM_MSG_OLT_TEST_RESULTS,
	PASCOMM_MSG_OLT_SET_TEST_MODE,
	PASCOMM_MSG_ONU_SET_LOOPBACK_MODE,
	PASCOMM_MSG_SET_MDIO_REGISTER,	
	PASCOMM_MSG_GET_MDIO_REGISTER,
	PASCOMM_MSG_MDIO_REGISTER_CONTENT,
	PASCOMM_MSG_SET_REMOTE_MDIO_REGISTER,	
	PASCOMM_MSG_GET_REMOTE_MDIO_REGISTER,
	PASCOMM_EVENT_REMOTE_MDIO_REGISTER_CONTENT,		 /*event*/
	PASCOMM_MSG_GET_MAC_ADDRESS,
	PASCOMM_MSG_MAC_ADDRESS,
	PASCOMM_MSG_SET_MAX_RTT,
	PASCOMM_EVENT_ONU_EVENT_NOTIFICATION,            /*event*/ 
	PASCOMM_MSG_VARIABLES_REQUEST,
	PASCOMM_MSG_START_DBA_ALGORITHM,
	PASCOMM_MSG_SEND_DBA_ALGORITHM_MSG,
	PASCOMM_MSG_STOP_EXTERNAL_DBA_ALGORITHM,
	PASCOMM_EVENT_DBA_ALGORITHM_EVENT,				 /*event*/ 
	PASCOMM_MSG_UPDATE_OLT_PARAMS,
	PASCOMM_MSG_GET_OLT_CAPABILITIES,
	PASCOMM_MSG_OLT_CAPABILITIES,
	PASCOMM_MSG_GET_STATISTICS,
	PASCOMM_MSG_GET_REMOTE_STATISTICS,
	PASCOMM_MSG_STATISTICS,
	PASCOMM_EVENT_REMOTE_STATISTICS,				 /*event*/
	PASCOMM_MSG_SET_POLICING_PARAMETERS,
	PASCOMM_MSG_SET_P2P_ACCESS_CONTROL,
	PASCOMM_MSG_START_ENCRYPTION,
	PASCOMM_EVENT_START_ENCRYPTION_ACKNOWLEDGE,	     /*event*/
	PASCOMM_MSG_UPDATE_ENCRYPTION_KEY,
	PASCOMM_EVENT_UPDATE_ENCRYPTION_KEY_ACKNOWLEDGE, /*event*/
	PASCOMM_MSG_STOP_ENCRYPTION,
	PASCOMM_EVENT_STOP_ENCRYPTION_ACKNOWLEDGE,	     /*event*/
	PASCOMM_MSG_CHANGE_OLT_ENCRYPTION_STATE,
	PASCOMM_MSG_FINALIZE_CHANGE_OLT_ENCRYPTION_STATE, 
	PASCOMM_MSG_SET_OLT_ENCRYPTION_KEY,
	PASCOMM_MSG_FINALIZE_SET_OLT_ENCRYPTION_KEY,
	PASCOMM_MSG_SET_LLID_VLAN_CONFIGURATION,
	PASCOMM_MSG_WRITE_EEPROM_REGISTER,
	PASCOMM_MSG_READ_EEPROM_REGISTER,
	PASCOMM_MSG_EEPROM_REGISTER,
	PASCOMM_MSG_WRITE_REMOTE_EEPROM_REGISTER,
	PASCOMM_MSG_READ_REMOTE_EEPROM_REGISTER,
	PASCOMM_EVENT_REMOTE_EEPROM_REGISTER,			 /*event*/
	PASCOMM_MSG_READ_I2C_REGISTER,
	PASCOMM_MSG_I2C_REGISTER,
	PASCOMM_MSG_GPIO_ACCESS,
	PASCOMM_MSG_GPIO_ACCESS_ACKNOWLEDGE,
	PASCOMM_MSG_SET_SPANNING_TREE_CONFIG,
	PASCOMM_MSG_SET_SPANNING_TREE_PORT_CONFIG,
	PASCOMM_MSG_GET_SPANNING_TREE_PORT_CONFIG,
	PASCOMM_MSG_SPANNING_TREE_PORT_CONFIG,			 
	PASCOMM_EVENT_SPANNING_TREE_PORT_STATE_CHANGE,	 /*event*/
	PASCOMM_MSG_GET_ONU_IGMP_TABLE,
	PASCOMM_MSG_ONU_IGMP_TABLE,
	PASCOMM_MSG_FLUSH_ONU_IGMP_TABLE,
	PASCOMM_MSG_RESET_ADDRESS_TABLE,
	PASCOMM_MSG_SET_LLID_P2P_CONFIGURATION,
	PASCOMM_MSG_SET_OLT_CNI_PORT_CONFIGURATION,
	PASCOMM_MSG_SET_LLID_ADDRESS_TABLE_CONFIG,
	PASCOMM_MSG_SET_LOG_LEVEL,
	PASCOMM_EVENT_LOG_MSG,							 /*event*/
	PASCOMM_MSG_LINK_TEST,
	PASCOMM_EVENT_LINK_TEST_RESULTS,				 /*event*/
	PASCOMM_MSG_GET_OAM_INFORMATION,
	PASCOMM_EVENT_OAM_INFORMATION,					 /*event*/	
	PASCOMM_EVENT_ONU_ID_AUTHORIZATION,				 /*event*/		
	PASCOMM_MSG_SET_ALARM_THRESHOLD,
	PASCOMM_MSG_SEND_RADIUS_MSG,
	PASCOMM_EVENT_RADIUS_MSG_RECEIVED,				 /*event*/
	PASCOMM_MSG_SET_802_1X_CONFIGURATION,
	PASCOMM_MSG_REAUTHENTICATE_LLID,
	PASCOMM_MSG_PING_ONU_QUERY,
	PASCOMM_EVENT_PONG,								 /*event*/  
	PASCOMM_MSG_CONNECTION_TEST,
	PASCOMM_MSG_CONNECTION_RESPONSE,
	PASCOMM_MSG_GET_OLT_PARAMS,
	PASCOMM_MSG_OLT_PARAMS,
	PASCOMM_MSG_GET_LLID_PARAMETERS,
	PASCOMM_MSG_LLID_PARAMETERS,
	PASCOMM_MSG_GET_OLT_CNI_PORT_CONFIGURATION,
	PASCOMM_MSG_OLT_CNI_PORT_CONFIGURATION,
	PASCOMM_MSG_GET_TEST_MODE,
	PASCOMM_MSG_TEST_MODE,
	PASCOMM_MSG_GET_STATIC_GRANTING_CONFIGURATION,
	PASCOMM_MSG_STATIC_GRANTING_CONFIGURATION,
	PASCOMM_MSG_GET_POLICING_PARAMETERS,
	PASCOMM_MSG_POLICING_PARAMETERS,
	PASCOMM_MSG_GET_LLID_P2P_CONFIGURATION,
	PASCOMM_MSG_LLID_P2P_CONFIGURATION,
	PASCOMM_MSG_GET_P2P_ACCESS_CONTROL,
	PASCOMM_MSG_P2P_ACCESS_CONTROL,
	PASCOMM_MSG_GET_802_1X_CONFIGURATION,
	PASCOMM_MSG_802_1X_CONFIGURATION,
	PASCOMM_MSG_GET_LLID_VLAN_CONFIGURATION,
	PASCOMM_MSG_LLID_VLAN_CONFIGURATION,
	PASCOMM_MSG_OLT_MODE_QUERY,
	PASCOMM_MSG_OLT_MODE_RESPONSE,
	PASCOMM_MSG_GET_LLID_REPORT,
	PASCOMM_MSG_LLID_REPORT,
 	PASCOMM_MSG_SET_ALARMS_CONFIGURATION,
	PASCOMM_MSG_GET_ALARMS_CONFIGURATION,
	PASCOMM_MSG_ALARMS_CONFIGURATION,
	PASCOMM_EVENT_UPDATAE_ONU_FW_ACK,				/*event*/
	PASCOMM_EVENT_REMOTE_ACCESS_ACKNOWLEDGE,		/*event*/
	PASCOMM_MSG_SET_REMOTE_MEMORY_BLOCK,
	PASCOMM_MSG_GET_REMOTE_MEMORY_BLOCK,	
	PASCOMM_EVENT_REMOTE_MEMORY_BLOCK,				/*event*/
	PASCOMM_MSG_SET_ONU_MAC_LOOPBACK,
	PASCOMM_MSG_SET_ONU_PHY_LOOPBACK,
	PASCOMM_MSG_SET_ONU_UNI_PORT_MAC_CONFIG,
	PASCOMM_MSG_GET_ONU_UNI_PORT_MAC_CONFIG,
	PASCOMM_EVENT_ONU_UNI_PORT_MAC_CONFIG,			/*event*/
	PASCOMM_MSG_GET_ONU_DEVICE_INFORMATION,
	PASCOMM_EVENT_ONU_DEVICE_INFORMATION,			/*event*/
    PASCOMM_EVENT_ENCRYPTION_RESULT     ,           /*event*/ 
    PASCOMM_EVENT_PON_LOSS ,                        /*event*/ 
    PASCOMM_MSG_SET_802_1X_CONFIGURATION_EXTENDED ,   
    PASCOMM_MSG_GET_802_1X_CONFIGURATION_EXTENDED ,   
    PASCOMM_MSG_RESPONSE_802_1X_CONFIGURATION_EXTENDED  ,                 
    PASCOMM_MSG_SET_OLT_PON_TRANSMISSION,
    PASCOMM_MSG_GET_ADDRESS_TABLE_ENTRY,
    PASCOMM_MSG_RESPONSE_ADDRESS_TABLE_ENTRY,
    PASCOMM_MSG_SET_HOST_PROTOCOLS_FORWARDING,
    PASCOMM_MSG_GET_HOST_PROTOCOLS_FORWARDING_STATUS,
    PASCOMM_MSG_RESPONSE_HOST_PROTOCOLS_FORWARDING_STATUS,
    PASCOMM_MSG_ADD_IGMP_FILTER,
    PASCOMM_MSG_REMOVE_IGMP_FILTER,
    PASCOMM_MSG_SET_MPCP_TIMEOUT,
    PASCOMM_MSG_GET_MPCP_TIMEOUT,
    PASCOMM_MSG_RESPONSE_MPCP_TIMEOUT,
    PASCOMM_MSG_SET_OAM_CONFIGURATION,
    PASCOMM_MSG_GET_OAM_CONFIGURATION,
    PASCOMM_MSG_RESPONSE_OAM_CONFIGURATION,
    PASCOMM_MSG_SET_OLT_TLV_ENCRYPTION_STATE,
    PASCOMM_MSG_GET_OLT_TLV_ENCRYPTION_STATE,
    PASCOMM_MSG_RESPONSE_OLT_TLV_ENCRYPTION_STATE,
    PASCOMM_MSG_SET_OLT_REGISTER,
    PASCOMM_MSG_GET_OLT_REGISTER,
    PASCOMM_MSG_RESPONSE_OLT_REGISTER,
    PASCOMM_MSG_SET_TRACES_STATE,
    PASCOMM_MSG_GET_TRACES_STATE,
    PASCOMM_MSG_RESPONSE_TRACES_STATE,
    PASCOMM_MSG_SET_CNI_MANAGEMENT_MODE,
    PASCOMM_MSG_GET_CNI_MANAGEMENT_MODE,
    PASCOMM_MSG_RESPONSE_CNI_MANAGEMENT_MODE,
    PASCOMM_MSG_SEND_CLI_COMMAND,
	PASCOMM_MSG_RESET_OLT_COUNTERS,
#ifdef PAS_SOFT_VERSION_V5_3_9
	PASCOMM_MSG_SET_STANDARD_OAM_TIMEOUT,
	PASCOMM_MSG_GET_STANDARD_OAM_TIMEOUT,
	PASCOMM_MSG_RESPONSE_STANDARD_OAM_TIMEOUT,
#endif
/*****************************************************************************/
/*****************************************************************************/
/*
 *	                               PAS5201 API 
 */
/*****************************************************************************/
/*****************************************************************************/
    
    PASCOMM_MSG_SET_CLASSIFICATION_RULE,
    PASCOMM_MSG_GET_CLASSIFICATION_RULE,
    PASCOMM_MSG_RESPONSE_CLASSIFICATION_RULE,
    PASCOMM_MSG_GET_OLT_OPTICS_PARAMETERS,
    PASCOMM_MSG_RESPONSE_OLT_OPTICS_PARAMETERS,
    PASCOMM_MSG_SET_ADDRESS_TABLE_CONFIGURATION,
    PASCOMM_MSG_GET_ADDRESS_TABLE_CONFIGURATION,
    PASCOMM_MSG_RESPONSE_ADDRESS_TABLE_CONFIGURATION,
    PASCOMM_MSG_GET_DBA_MODE,
    PASCOMM_MSG_RESPONSE_DBA_MODE,
    PASCOMM_MSG_SET_POLICING_THRESHOLDS,
    PASCOMM_MSG_GET_POLICING_THRESHOLDS,
    PASCOMM_MSG_RESPONSE_POLICING_THRESHOLDS,
    PASCOMM_MSG_SET_P2P_GLOBAL_BRIDGING,
    PASCOMM_MSG_GET_P2P_GLOBAL_BRIDGING,
    PASCOMM_MSG_RESPONSE_P2P_GLOBAL_BRIDGING,
    PASCOMM_MSG_SET_LLID_FEC_MODE,
    PASCOMM_MSG_GET_LLID_FEC_MODE,
    PASCOMM_MSG_RESPONSE_LLID_FEC_MODE,
    PASCOMM_MSG_SET_PRIORITY_QUEUE_MAPPING,
    PASCOMM_MSG_GET_PRIORITY_QUEUE_MAPPING,
    PASCOMM_MSG_RESPONSE_PRIORITY_QUEUE_MAPPING,
    PASCOMM_MSG_SET_FRAME_SIZE_LIMITS,
    PASCOMM_MSG_GET_FRAME_SIZE_LIMITS,
    PASCOMM_MSG_RESPONSE_FRAME_SIZE_LIMITS,
    PASCOMM_MSG_SET_VLAN_RECOGNIZING,
    PASCOMM_MSG_GET_VLAN_RECOGNIZING,
    PASCOMM_MSG_RESPONSE_VLAN_RECOGNIZING,
    PASCOMM_MSG_SET_LLID_VLAN_MODE,
    PASCOMM_MSG_GET_LLID_VLAN_MODE,
    PASCOMM_MSG_RESPONSE_LLID_VLAN_MODE,
    PASCOMM_MSG_SET_VID_DOWNLINK_MODE,
    PASCOMM_MSG_GET_VID_DOWNLINK_MODE,
    PASCOMM_MSG_RESPONSE_VID_DOWNLINK_MODE,
    PASCOMM_MSG_DELETE_VID_DOWNLINK_MODE,
    PASCOMM_MSG_SET_OLT_IGMP_SNOOPING_MODE,
    PASCOMM_MSG_GET_OLT_IGMP_SNOOPING_MODE,
    PASCOMM_MSG_RESPONSE_OLT_IGMP_SNOOPING_MODE,
    PASCOMM_MSG_SET_OLT_MLD_FORWARDING_MODE,
    PASCOMM_MSG_GET_OLT_MLD_FORWARDING_MODE,
    PASCOMM_MSG_RESPONSE_OLT_MLD_FORWARDING_MODE,
    PASCOMM_MSG_SET_ENCRYPTION_PREAMBLE_MODE,
    PASCOMM_MSG_GET_ENCRYPTION_PREAMBLE_MODE,
    PASCOMM_MSG_RESPONSE_ENCRYPTION_PREAMBLE_MODE,
    PASCOMM_MSG_SET_FEC_NEGOTIATION_MODE,
    PASCOMM_MSG_GET_FEC_NEGOTIATION_MODE,
    PASCOMM_MSG_RESPONSE_FEC_NEGOTIATION_MODE,
    PASCOMM_MSG_SET_DEBUG_2,
    PASCOMM_MSG_GET_DEBUG_2,
    PASCOMM_MSG_RESPONSE_DEBUG_2,
    PASCOMM_MSG_SET_INTERNAL_DEBUG_1,
    PASCOMM_MSG_GET_INTERNAL_DEBUG_1,
    PASCOMM_MSG_RESPONSE_INTERNAL_DEBUG_1,
    PASCOMM_MSG_SET_DEBUGGING_CONFIGURATION,
    PASCOMM_MSG_GET_DEBUGGING_CONFIGURATION,
    PASCOMM_MSG_RESPONSE_DEBUGGING_CONFIGURATION,
    PASCOMM_MSG_SET_VIRTUAL_SCOPE_MEASUREMENT,
    PASCOMM_MSG_GET_VIRTUAL_SCOPE_MEASUREMENT,
    PASCOMM_MSG_RESPONSE_VIRTUAL_SCOPE_MEASUREMENT,
  	PASCOMM_MSG_GET_OLT_PARAMS_V5,
	PASCOMM_MSG_OLT_PARAMS_V5,
    PASCOMM_MSG_SET_DOWNLINK_BUFFER_CONFIGURATION,
    PASCOMM_MSG_GET_DOWNLINK_BUFFER_CONFIGURATION,
    PASCOMM_MSG_RESPONSE_DOWNLINK_BUFFER_CONFIGURATION,
    PASCOMM_MSG_SET_RAW_DATA,
    PASCOMM_MSG_GET_PORT_MAC_LIMITING,
    PASCOMM_MSG_RESPONSE_PORT_MAC_LIMITING,
    PASCOMM_MSG_SET_DBA_REPORT_FORMAT,
    PASCOMM_MSG_GET_DBA_REPORT_FORMAT,
    PASCOMM_MSG_RESPONSE_DBA_REPORT_FORMAT,
    PASCOMM_MSG_SET_ENCRYPTION_CONFIGURATION,
    PASCOMM_MSG_GET_ENCRYPTION_CONFIGURATION,
    PASCOMM_MSG_RESPONSE_ENCRYPTION_CONFIGURATION,
    PASCOMM_MSG_GET_ADDRESS_TABLE_NEXT_ENTRY,
    PASCOMM_MSG_RESPONSE_ADDRESS_TABLE_NEXT_ENTRY,
	PASCOMM_MSG_SET_MAC_ADDRESS_AUTHENTICATION,
	PASCOMM_MSG_GET_MAC_ADDRESS_AUTHENTICATION,
    PASCOMM_MSG_RESPONSE_MAC_ADDRESS_AUTHENTICATION,
	PASCOMM_MSG_CLEAR_MAC_ADDRESS_AUTHENTICATION_TABLE,
	PASCOMM_MSG_SET_AUTHORIZE_ACCORDING_TO_MAC_LIST,
	PASCOMM_MSG_GET_AUTHORIZE_ACCORDING_TO_MAC_LIST,
    PASCOMM_MSG_RESPONSE_AUTHORIZE_ACCORDING_TO_MAC_LIST,
	PASCOMM_MSG_SET_IGMP_PROXY_MODE,
	PASCOMM_MSG_GET_IGMP_PROXY_MODE,
    PASCOMM_MSG_RESPONSE_IGMP_PROXY_MODE,
	PASCOMM_MSG_GET_IGMP_PROXY_TABLE,
    PASCOMM_MSG_RESPONSE_IGMP_PROXY_TABLE,
	PASCOMM_MSG_SET_IGMP_PROXY_PARAMETERS,
	PASCOMM_MSG_GET_IGMP_PROXY_PARAMETERS,
    PASCOMM_MSG_RESPONSE_IGMP_PROXY_PARAMETERS,
	PASCOMM_MSG_SET_P2P_MODE,
	PASCOMM_MSG_GET_P2P_MODE,
    PASCOMM_MSG_RESPONSE_P2P_MODE,
	PASCOMM_MSG_GET_IGMP_PROXY_GROUP_ADDRESS_SOURCES,
    PASCOMM_MSG_RESPONSE_IGMP_PROXY_GROUP_ADDRESS_SOURCES,
    PASCOMM_EVENT_ONU_DENY,				 /*event*/		
	PASCOMM_EVENT_RF_RELAY,				 /*event*/		
	PASCOMM_MSG_TEST_DOWNLINK_BUFFER,
	PASCOMM_MSG_SET_ADDRESS_TABLE_FULL_HANDLING_MODE,
	PASCOMM_MSG_GET_ADDRESS_TABLE_FULL_HANDLING_MODE,
	PASCOMM_MSG_RESPONSE_ADDRESS_TABLE_FULL_HANDLING_MODE,
	PASCOMM_MSG_SET_LED_SW_CONTROL_MODE,
	PASCOMM_MSG_GET_LED_SW_CONTROL_MODE,
	PASCOMM_MSG_RESPONSE_LED_SW_CONTROL_MODE,
	PASCOMM_MSG_SET_LED_SW_ACTIVATION,
	PASCOMM_MSG_SET_HOST_RATE_LIMITER,
	PASCOMM_MSG_GET_HOST_RATE_LIMITER,
	PASCOMM_MSG_RESPONSE_HOST_RATE_LIMITER,
    PASCOMM_MSG_SET_VIRTUAL_SCOPE_DETECT_ONU_LASER_ALWAYS_ON,
    PASCOMM_MSG_GET_VIRTUAL_SCOPE_DETECT_ONU_LASER_ALWAYS_ON,
    PASCOMM_MSG_RESPONSE_VIRTUAL_SCOPE_DETECT_ONU_LASER_ALWAYS_ON,
    PASCOMM_MSG_SET_VIRTUAL_SCOPE_ADC_CONFIG,
    PASCOMM_MSG_GET_VIRTUAL_SCOPE_ADC_CONFIG,
    PASCOMM_MSG_RESPONSE_VIRTUAL_SCOPE_ADC_CONFIG,
#ifdef PAS_SOFT_VERSION_V5_3_9
    PASCOMM_EVENT_CNI_LINK,                         /*event*/ 
    PASCOMM_MSG_GET_CNI_LINK_STATUS,
    PASCOMM_MSG_RESPONSE_CNI_LINK_STATUS,
    PASCOMM_MSG_SET_IGMP_PARAMETER,
    PASCOMM_MSG_GET_IGMP_PARAMETER,
    PASCOMM_MSG_RESPONSE_IGMP_PARAMETER,
    PASCOMM_MSG_SET_IGMP_VLAN_WHITELIST,
    PASCOMM_MSG_GET_IGMP_VLAN_WHITELIST,
    PASCOMM_MSG_RESPONSE_IGMP_VLAN_WHITELIST,
	PASCOMM_MSG_SET_OAM_FRAME_RATE,
	PASCOMM_MSG_GET_OAM_FRAME_RATE,
	PASCOMM_MSG_RESPONSE_OAM_FRAME_RATE,
#endif

#ifdef PAS_SOFT_VERSION_V5_3_11
	PASCOMM_MSG_SET_REDUNDANCY_CONFIG,
	PASCOMM_MSG_GET_REDUNDANCY_CONFIG,
	PASCOMM_MSG_RESPONSE_REDUNDANCY_CONFIG,
	PASCOMM_MSG_SET_REDUNDANCY_ONU_REGISTER,
	PASCOMM_MSG_GET_LLID_OAM_DB,
	PASCOMM_MSG_RESPONSE_LLID_OAM_DB,
	PASCOMM_MSG_SET_REDUNDANCY_SWITCH_OVER,
	PASCOMM_MSG_GET_REDUNDANCY_ADDRESS_TABLE,
	PASCOMM_MSG_REDUNDANCY_ADDRESS_TABLE,
    PASCOMM_EVENT_REDUNDANCY_OLT_FAILURE,           /*event*/ 
    PASCOMM_EVENT_REDUNDANCY_SWITCH_OVER,           /*event*/ 
    PASCOMM_MSG_SET_RTT_UPDATE_MODE,
    PASCOMM_MSG_GET_RTT_UPDATE_MODE,
    PASCOMM_MSG_RESPONSE_RTT_UPDATE_MODE,
    PASCOMM_MSG_SET_RTT_VALUE,
    PASCOMM_MSG_GET_RTT_VALUE,
    PASCOMM_MSG_RESPONSE_RTT_VALUE,
	PASCOMM_MSG_SET_VIRTUAL_LLID,
#endif

#ifdef PAS_SOFT_VERSION_V5_3_12
	PASCOMM_MSG_SET_LLID_REDUND_STANDBY_TO_ACTIVE,
    PASCOMM_MSG_SET_FLASH_IMAGE_ACTIVE,
    PASCOMM_MSG_GET_FLASH_IMAGE_ACTIVE,
    PASCOMM_MSG_RESPONSE_FLASH_IMAGE_ACTIVE, 
	PASCOMM_MSG_SET_DBA_ALLOCATION_OVERHEAD_MODE,
	PASCOMM_MSG_GET_DBA_ALLOCATION_OVERHEAD_MODE,
	PASCOMM_MSG_RESPONSE_DBA_ALLOCATION_OVERHEAD_MODE,
#endif

#ifdef PAS_SOFT_VERSION_V5_3_13
	PASCOMM_MSG_SET_MIGRATION_CONFIGURATION,
	PASCOMM_MSG_GET_MIGRATION_CONFIGURATION,
	PASCOMM_MSG_RESPONSE_MIGRATION_CONFIGURATION,
	PASCOMM_EVENT_MIGRATION_ATTEMPT,     /* event */
#endif

	PASCOMM_MSG_EVERY_MSG /* This msg represents every legitimate msg */
} PASCOMM_msg_t;

extern char *PASCOMM_msg_s[];

typedef enum
{
   PON_DONT_PASS,
   PON_PASS_DATAPATH,
   PON_PASS_CPU,
   PON_PASS_BOTH
} PON_forwarding_action_t;


/* Set port address table configuration
**
** Set the maximal number of MAC addresses allowed for a LLID in the OLT address table.
** 
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid				: LLID, range: PON_DEFAULT_LLID, PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT ,PON_PORT_ID_CNI 
**		maximum_entries		: Maximum number of entries allowed in OLT address table. 
**                            Values: PAS5001 PON_address_table_entries_llid_limitation_t values
**                                    PAS5201 or newer 0 - 8191
** 
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_test_port_mac_limiting 
						( const PON_olt_id_t    							 olt_id, 
						  const PON_llid_t									 llid,
						  const short int                                    maximum_entries );

extern PON_STATUS PAS_set_port_mac_limiting 
						( const PON_olt_id_t								 olt_id, 
						  const PON_llid_t									 llid,
						  const short int                                    maximum_entries );
/* 
extern short int PAS_set_llid_address_table_configuration 
						( const short int									 olt_id, 
						  const PON_llid_t									 llid,
						  const PON_address_table_entries_llid_limitation_t  maximum_entries );
						  */

/* Get port address table configuration
**
** Get the maximal number of MAC addresses allowed for a LLID
** in PON port or in the CNI port of the OLT address table 
**
** Input Parameters:
**      olt_id          : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid            : LLID, range: PON_DEFAULT_LLID, PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT ,PON_PORT_ID_CNI 
** Output Parameters:
**      maximum_entries : Maximum number of entries allowed in OLT address table
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_port_mac_limiting 
                                            (const PON_olt_id_t      olt_id,
                                             const PON_llid_t        llid,
                                                   short int        *maximum_entries );

/* Remove address table record
**
** Delete the content of an entry in an OLT address table. Only if the MAC address appears in the 
** address table, it will be removed.
** The entry type can be either static or dynamic.
**
** Input Parameters:
**			    olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				mac_address		: Removed MAC address, may be unicast or multicast
**													
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_remove_address_table_record 
                                          ( const PON_olt_id_t   olt_id, 
									        const mac_address_t  mac_address );

extern PON_STATUS PAS_remove_address_table_record 
                                          ( const PON_olt_id_t   olt_id, 
									        const mac_address_t  mac_address );
					                                                                     


/* Get address table
**
** The complete address table of the specified OLT is copied into a memory space allocated by the caller.
** Assumption: the caller must allocate the memory for the address table before calling the function, and
** deallocate it after the function returns.	
**
** Input Parameters:
**			    olt_id								  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				
** Output Parameter:
**				active_records						  : The actual size of the address table, 
**														range: 0 - PON_ADDRESS_TABLE_SIZE 
**				address_table						  : Pointer to a memory address where the address 
**														table should be copied to
**					mac_address	(per address record)  : Address record MAC address, may be unicast or 
**														multicast
**					logical_port (per address record) : Address record logical port, translates to ONU id/
**														CNI port in the current version, 
**														values:PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**														PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID
**					type (per address record)		  : Address record type, range: enum values
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_get_address_table 
                                ( const PON_olt_id_t    olt_id, 
								  short int			   *active_records,	
								  PON_address_table_t   address_table );
								  
/* Get address table for a specific llid
**
** The complete address table of the specified ONU from the OLT address table is copied into a memory space allocated by the caller.
** Assumption: the caller must allocate the memory for the address table before calling the function, and
** deallocate it after the function returns.	
**
** Input Parameters:
**			    olt_id								  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    onu_id			                      : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT  
**				
** Output Parameter:
**				active_records						  : The actual size of the address table, 
**														range: 0 - PON_ADDRESS_TABLE_SIZE 
**				address_table						  : Pointer to a memory address where the address 
**														table should be copied to
**					mac_address	(per address record)  : Address record MAC address, may be unicast or 
**														multicast
**					logical_port (per address record) : Address record logical port, translates to ONU id/
**														CNI port in the current version, 
**														values:PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**														PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID
**					type (per address record)		  : Address record type, range: enum values
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
PON_STATUS PAS_get_address_table_llid 
                ( const PON_olt_id_t    olt_id,
                  const PON_onu_id_t    onu_id,
				  short int			   *active_records,	
				  PON_address_table_t   address_table );

extern short int Remove_olt ( const short int  olt_id,
			const bool send_shutdown_msg_to_olt, 
			const bool reset_olt);

/* Get OAM information
**
** Get OAM information data of an ONU device. 
** Note: oam_standard_information struct is allocated dynamically by the function. It should be freed by
** the caller after the function returns.
**
** Input Parameters:
**			    olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT 
**
** Output Parameters (PON_oam_information_t struct):
**				oam_information
**					oam_information_reference_standard	: The OAM version the ONU supports  
**					oam_standard_information			: Information data according to relevant OAM standard
**					passave_originated				    : Is the OAM information originated from Passave ONU
**					passave_specific_oam_information	: Passave extension information data 
**
** Return codes:
**				All the defined PAS_* return codes, 
**			    PAS_TIME_OUT - when ONU doesn't supply OAM information 
**
*/
extern PON_STATUS PAS_get_oam_information 
                                  ( const PON_olt_id_t      olt_id, 
								    const PON_onu_id_t	    onu_id,
								    PON_oam_information_t  *oam_information );

/* Set oam configuration
**
** Sets the oam limit to be enable\disable
**
** Input Parameters:
**			olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    limitation			    : oam limit. Values: ENABLE, DISABLE 
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_test_oam_configuration (const PON_olt_id_t olt_id,
                                       const bool         ignore_oam_limit );

extern PON_STATUS PAS_set_oam_configuration (const PON_olt_id_t olt_id,
                                      const bool         ignore_oam_limit );



/* Get LLID P2P configuration
**
** Query LLID P2P configuration. This configuration corresponds to the configuration updated 
** by the PAS_set_llid_p2p_configuration API function.
**
** Input Parameters:
**		 olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid					: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**
** Output Parameters:
**		 address_not_found		: Discard / flood downstream frames when address is not found in the  
**								  OLT's address table. Values: ENABLE (flood), DISABLE (discard)
**		 broadcast				: Discard / flood downstream frames with broadcast destination address.
**								  Values: ENABLE (flood), DISABLE (discard)
** Return codes:
**		 All the defined PAS_* return codes
*/
extern PON_STATUS PAS_get_llid_p2p_configuration
                                     ( const PON_olt_id_t   olt_id, 
									   const PON_llid_t     llid,
									   bool				   *address_not_found,
									   bool				   *broadcast );


/* Set LLID P2P configuration
**
** Set P2P (peer-to-peer) behavior for traffic coming from a specific LLID in an EPON network. 
** The default configuration is disabling downstream frames in all cases. 
** P2P configuration zeroes automatically when ONU disconnects from the network, and has to be re-set when
** ONU reconnects to the network.
**
** Input Parameters:
**		 olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid					: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**		 address_not_found		: Discard / flood downstream frames when address is not found in the OLT 
**								  address table. Values: ENABLE (flood), DISABLE (discard)
**		 broadcast				: Discard / flood downstream frames with broadcast destination address.
**								  Values: ENABLE (flood), DISABLE (discard)
** Return codes:
**				All the defined PAS_* return codes
*/
extern PON_STATUS PAS_test_llid_p2p_configuration 
                                         ( const PON_olt_id_t  olt_id, 
										   const PON_llid_t    llid,
										   const bool		   address_not_found,
										   const bool		   broadcast );

extern PON_STATUS PAS_set_llid_p2p_configuration 
                                         ( const PON_olt_id_t  olt_id, 
										   const PON_llid_t    llid,
										   const bool		   address_not_found,
										   const bool		   broadcast );


/* Get P2P access control
**
** Query LLID P2P access control configuration. This configuration corresponds to the configuration 
** updated by the PAS_set_p2p_access_control API function.
**
** Input Parameters:
**		 olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		 llid			: LLID, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**
** Output Parameter:
**		 llids			: Array of access control status with each one of the LLIDs in the OLT
**						  Values: ENABLE  (there is    P2P access permission with the LLID), 
**								  DISABLE (there isn't P2P access permission with the LLID / LLID not registered)
** Return codes:
**		 All the defined PAS_* return codes
**
**/
typedef bool  llid_access_list_t [PAS_LLID_PER_OLT_ARRAY_SIZE];

extern PON_STATUS PAS_get_p2p_access_control 
                                    ( const PON_olt_id_t  olt_id, 
									  const PON_llid_t    llid,
									  llid_access_list_t  llids );

/* Set P2P access control
**
** Enable / Disable P2P traffic between a single LLID and several other LLIDs in the EPON network.
** The P2P traffic is always bidirectional. P2P policy zeroes on default and when ONU disconnects 
** from the network.	
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		configured_llid	: LLID which its P2P access is configured, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**		number_of_llids	: Number of LLIDs to configure the P2P access policy upon. range: PON_P2P_POLICY_ALL_LLIDS,
**						  0 - (PON_MAX_LLID_PER_OLT - PON_MIN_LLID_PER_OLT +1 -1)
**		llids			: Array of LLIDs to configure the P2P access policy upon	
**		access			: P2P access policy enable / disable, values: ENABLE / DISABLE
**					
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_p2p_access_control 
                                     ( const PON_olt_id_t   olt_id, 
									   const PON_llid_t     configured_llid,
									   const short int      number_of_llids,
									   const PON_llid_t     llids[],
									   const bool		    access );

extern PON_STATUS PAS_set_p2p_access_control 
                                     ( const PON_olt_id_t   olt_id, 
									   const PON_llid_t     configured_llid,
									   const short int      number_of_llids,
									   const PON_llid_t     llids[],
									   const bool		    access );


#if 0
/* Set P2P mode
**
** Set point to point mode.
**
** Input Parameters:
**      olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      mode				: point to pont mode. ENABLE\DISABLE
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_set_p2p_mode
                ( const PON_olt_id_t	olt_id,
				  const bool			mode);

/* Get P2P mode
**
** Get point to point mode.
**
** Input Parameters:
**      olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      mode				: point to point mode.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_get_p2p_mode
                ( const PON_olt_id_t   olt_id,
				  bool				  *mode);
#endif

/* Send CLI command
**
** Execute OLT Firmware CLI command.
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      size            : Size of the command string. Measured in bytes
**      command         : Command string to be execute.
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/

extern PON_STATUS PAS_test_send_cli_command 
                ( const PON_olt_id_t     olt_id,
                  const unsigned short   size,
                  const unsigned char   *command );

extern PON_STATUS PAS_send_cli_command 
                ( const PON_olt_id_t     olt_id,
                  const unsigned short   size,
                  const unsigned char   *command );

/* Set OLT pon transmission signal
**
** Sets the olt pon transmission signal
**
** Input Parameters:
**			olt_id					 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    mode					 : PON transmission signal. Values: ENABLE, DISABLE 
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_test_olt_pon_transmission 
                                        (const PON_olt_id_t olt_id,
                                         const bool         mode    );


extern PON_STATUS PAS_set_olt_pon_transmission 
                                        (const PON_olt_id_t olt_id,
                                         const bool         mode    );

/* Get LLID VLAN mode
**
** Get the VLAN handling for uplink frames arriving from a certain LLID 
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid               : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
** Output Parameters:
**      vlan_uplink_config : Uplink VLAN config structure, see definition
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_llid_vlan_mode 
                                 (const PON_olt_id_t                      olt_id,
                                  const PON_llid_t                        llid,
                                        PON_olt_vlan_uplink_config_t     *vlan_uplink_config );


/* Get VID downlink mode
**
** Get the VLAN handling for downlink frames arriving from a certain VID 
** (NULL-tagged and untagged frames are a special case) 
**
** Input Parameters:
**      olt_id              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      vid                 : VLAN ID of arriving frame (for NULL-tag use 0, 
**                            for untagged use 4095) ,Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID or  VLAN_UNTAGGED_ID
** Output Parameters:
**      vid_downlink_config : Downlink VID config structure
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_vid_downlink_mode 
                                    (const PON_olt_id_t                       olt_id,
                                     const PON_vlan_tag_t                     vid,
                                           PON_olt_vid_downlink_config_t     *vid_downlink_config );



/* Set authorize mac address according list mode
**
** Set authorize mac address according list mode
**
** Input Parameters:
**      olt_id					  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      authentication_according_to_list  : authorize according to MAC list. ENABLE\DISABLE
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_set_authorize_mac_address_according_list_mode
                ( const PON_olt_id_t  olt_id,
				  const bool		  authentication_according_to_list);


/* Get authorize mac address according list mode
**
** Get authorize mac address according list mode
**
** Input Parameters:
**      olt_id							  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      authentication_according_to_list  : authorize according to MAC list. ENABLE\DISABLE
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_get_authorize_mac_address_according_list_mode
                ( const PON_olt_id_t   olt_id,
				  bool				  *authentication_according_to_list);


/* Set mac addresses authentication
**
** Set mac addresses for authentication.
**
** Input Parameters:
**      olt_id					  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      authentication_operation  : authentication operation. ENABLE\DISABLE
**      number_of_mac_address	  : number of valid MAC addresses to be authenticate\deny.
**      mac_addresses_list		  : list of ONU MAC addresses. See mac_addresses_list_t for details.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_set_mac_address_authentication 
                ( const PON_olt_id_t	       olt_id,
				  const bool				   authentication_operation,
				  const unsigned char		   number_of_mac_address,
				  mac_addresses_list_t		   mac_addresses_list);



/* Get mac addresses authentication
**
** Get mac addresses for authentication.
**
** Input Parameters:
**      olt_id					  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      number_of_mac_address	  : number of authenticated MAC addresses.
**      mac_addresses_list		  : list of ONU MAC addresses. See mac_addresses_list_t for details.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_get_mac_address_authentication
                ( const PON_olt_id_t	 olt_id,
				  unsigned char			*number_of_mac_address,
				  mac_addresses_list_t   mac_addresses_list);




/* Clear mac addresses authentication table
**
** Clear mac addresses authentication table.
**
** Input Parameters:
**      olt_id					  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_clear_mac_address_authentication_table
                ( const PON_olt_id_t	       olt_id);


/* Set DBA report format
**
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      report_format    : See PON_DBA_report_format_t for details
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_start_dba_report_format(      
                  const PON_olt_id_t             olt_id,
                  const PON_DBA_report_format_t  report_format );

extern PON_STATUS PAS_set_dba_report_format(      
                  const PON_olt_id_t             olt_id,
                  const PON_DBA_report_format_t  report_format );


/* Get DBA report format
**
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      report_format    : See PON_DBA_report_format_t for details
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_dba_report_format(      
                  const PON_olt_id_t        olt_id,
                  PON_DBA_report_format_t  *report_format );


/* Set OLT MLD forwarding mode
**
** Set MLD forwarding mode of a specific OLT 
**
** Input Parameters:
**      olt_id            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      mode              : Mode , range : DISABLE, ENABLE
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_olt_mld_forwarding_mode 
                                        (const PON_olt_id_t                     olt_id,
                                         const disable_enable_t                 mode );
extern PON_STATUS PAS_set_olt_mld_forwarding_mode 
                                        (const PON_olt_id_t                     olt_id,
                                         const disable_enable_t                 mode );

/* Get OLT MLD snooping mode
**
** Get MLD snooping mode of a specific OLT  
**
** Input Parameters:
**      olt_id            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      mode              : Mode , range : DISABLE, ENABLE
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_olt_mld_forwarding_mode 
                                        (const PON_olt_id_t                     olt_id,
                                               disable_enable_t                *mode );

/* Set encryption preamble mode
**
** Set the encryption preamble mode
**
** Input Parameters:
**      olt_id          : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      encryption_mode : Disable/enable encryption preamble mode
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_encryption_preamble_mode 
                                           (const PON_olt_id_t      olt_id,
                                            const bool              encryption_mode );
extern PON_STATUS PAS_set_encryption_preamble_mode 
                                           (const PON_olt_id_t      olt_id,
                                            const bool              encryption_mode );

/* Get encryption preamble mode
**
** Get the encryption preamble mode 
**
** Input Parameters:
**      olt_id          : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      encryption_mode : Disable/enable encryption preamble mode
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_encryption_preamble_mode 
                                           (const PON_olt_id_t      olt_id,
                                                  bool             *encryption_mode );


#define CTC_OUI_DEFINE                    0x111111
#define CTC_VERSION_DEFINE                1
#define CTC_2_1_ONU_VERSION               0x21

#define CTC_OBJECT_CONSTANT_SIZE		  4

#define CTC_ENCRYPTION_KEY_SIZE           3	/* CTC encryption key size measured in Bytes */

#define MAX_OUI_RECORDS                   62/* Max OUI+Version pairs in Extended OAM information frame*/

#define ONU_RESPONSE_SIZE                 100/* used in send file file_name buffer and send file error_msg buffer */

#define	CTC_STACK_ALL_PORTS		0xFF
#define	CTC_MANAGEMENT_OBJECT_ALL_PORTS		0xFFFF


#ifdef PASSOFT_V5_1_20 
#endif

/****  2007/04/29 新增加的********/

/* Set point to point mode
**
** Set point to point mode.
**
** Input Parameters:
**      olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      mode				: point to pont mode. ENABLE\DISABLE
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_set_point_to_point_mode
                ( const PON_olt_id_t	olt_id,
				  const bool			mode);

/* Get point to point mode
**
** Get point to point mode.
**
** Input Parameters:
**      olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      mode				: point to point mode.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_point_to_point_mode
                ( const PON_olt_id_t   olt_id,
				  bool				  *mode);


/* Set address table full handling mode
**
** Set address table full handling mode.
**
** Input Parameters:
**      olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      remove_entry_when_table_full	: TRUE- delete entry in case address table is full.
**										: FALSE- Don't delete entries due to address table full reason.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_set_address_table_full_handling_mode
                ( const PON_olt_id_t   olt_id,
				  const bool		   remove_entry_when_table_full);


/* Get address table full handling mode
**
** Get address table full handling mode.
**
** Input Parameters:
**      olt_id							: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      remove_entry_when_table_full	: TRUE- delete entry in case address table is full.
**										: FALSE- Don't delete entries due to address table full reason.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/

extern PON_STATUS PAS_get_address_table_full_handling_mode
                ( const PON_olt_id_t   olt_id,
				  bool				  *remove_entry_when_table_full);

/* 
** DLB definitions
**
*/

typedef enum
{
	PON_TEST_DOWNLINK_BUFFER_DATA					= 0x0,
	PON_TEST_DOWNLINK_BUFFER_ADDRESS				= 0x1,
	
}PON_test_downlink_buffer_t;

/* Test downlink buffer
**
** Test the downlink buffer
**
** Input Parameters:
**      olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      test_type			: test type. See PON_test_downlink_buffer_t for details.
** 
** Return codes:
**          All the defined         PAS_* return codes
*/


extern PON_STATUS PAS_test_downlink_buffer
                ( const PON_olt_id_t				 olt_id,
				  const PON_test_downlink_buffer_t   test_type);

/* Set LLID FEC mode
**
** Set the downlink FEC mode, either for multicast/broadcast frames, 
** or for a certain, registered, LLID. Uplink FEC mode is enabled automatically 
** when the register request acknowledge contain FEC payload
**
** Input Parameters:
**      olt_id       : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid         : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      downlink_fec : Disable or enable hardware downlink FEC mode for the LLID
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_llid_fec_mode 
                                (const PON_olt_id_t      olt_id,
                                 const PON_llid_t        llid,
                                 const bool              downlink_fec );
extern PON_STATUS PAS_set_llid_fec_mode 
                                (const PON_olt_id_t      olt_id,
                                 const PON_llid_t        llid,
                                 const bool              downlink_fec );

/* Get LLID FEC mode
**
** Get the downlink FEC mode, either for multicast/broadcast frames, or for a certain,
** registered, LLID. Uplink FEC mode is enabled automatically when the register request 
** acknowledge contain FEC payload 
**
** Input Parameters:
**      olt_id       : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid         : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
** Output Parameters:
**      downlink_fec : Disable or enable hardware downlink FEC mode for the LLID
**      uplink_fec   : Disable or enable Uplink FEC mode 
**      last_uplink_frame_fec : Disable or enable last uplink frame 
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_llid_fec_mode 
                                (const PON_olt_id_t    olt_id,
                                 const PON_llid_t      llid,
                                       bool           *downlink_fec, 
                                       bool           *uplink_fec,
                                       bool           *last_uplink_frame_fec );

/****   结束*******/

#ifdef  PASSOFT_V5_1_22 
#endif
/* 在v5.1.22中增加的定义*/
/* 
** LED definitions
**
*/
typedef enum
{
	PON_LED_TYPE_SYSTEM_PORT_TX					= 0x0,
	PON_LED_TYPE_SYSTEM_PORT_RX					= 0x1,
	PON_LED_TYPE_SYSTEM_PORT_RX_ERRORS			= 0x2,
	PON_LED_TYPE_SYSTEM_PORT_LINK				= 0x3,
	PON_LED_TYPE_PON_PORT_TX					= 0x4,
	PON_LED_TYPE_PON_PORT_RX					= 0x5,
	PON_LED_TYPE_PON_PORT_RX_ERRORS			= 0x6,
	PON_LED_TYPE_PON_PORT_LINK					= 0x7,
	
}PON_led_type_t;


typedef enum
{
	AUTO_NEGOTIATION_OFF,
	AUTO_NEGOTIATION_ON
} PON_auto_negotiation_t;

typedef enum 
{
	ADVERTISE_PORT_SINGLE,
	ADVERTISE_PORT_MULTI
} PON_advertise_port_type_t;

typedef enum
{
	PON_ADVERTISE_NOT_CAPABLE,
	PON_ADVERTISE_CAPABLE
} PON_advertise_t;


#include "OnuPmcRemoteManagement.h"
#include "OnuPmcApi.h"

/* Init								
**
** This command initializes the PAS6201 Remote Management package  
**			  
**
** Return codes:
**				All the defined REMOTE_PASONU_* return codes
**
*/

extern PON_STATUS  REMOTE_PASONU_init ( void );


/* Terminate								
**
** This command terminates the PAS6201 Remote Management package. 
**
** Input Parameters:
**			  
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

extern PON_STATUS REMOTE_PASONU_terminate ( void );

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

extern PON_STATUS REMOTE_PASONU_reset_device ( 
                         const PON_olt_id_t        olt_id, 
                         const PON_onu_id_t        onu_id,
                         const PON_reset_reason_t  reason);


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

extern PON_STATUS REMOTE_PASONU_get_device_version( 
                                   const PON_olt_id_t     olt_id, 
                                   const PON_onu_id_t     onu_id,
                                   PON_device_version_t  *versions);


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

extern PON_STATUS REMOTE_PASONU_encryption_set_key ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   const PON_encryption_key_t         encryption_key,
                                   const PON_encryption_key_index_t   encryption_key_index);

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
extern PON_STATUS REMOTE_PASONU_encryption_get_state ( 
                                   const PON_olt_id_t           olt_id, 
                                   const PON_onu_id_t           onu_id,
                                   PON_encryption_direction_t  *direction);                                  

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
extern PON_STATUS REMOTE_PASONU_address_table_get_number_of_entries( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   long                *number_of_entries);

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
extern PON_STATUS REMOTE_PASONU_address_table_ageing_configuration( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const bool           enable,
                                   const unsigned long  max_age);

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
extern PON_STATUS REMOTE_PASONU_address_table_get (
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

extern PON_STATUS REMOTE_PASONU_address_table_set_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   const unsigned long  limit);


/* Address Table getting limit for UNI Learning
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

extern PON_STATUS REMOTE_PASONU_address_table_get_uni_learn_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   unsigned long       *limit);


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


extern PON_STATUS REMOTE_PASONU_address_table_clear( 
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
extern PON_STATUS REMOTE_PASONU_address_table_set_uni_learning( 
                                   const PON_olt_id_t  olt_id, 
                                   const PON_onu_id_t  onu_id,
                                   const bool          enable);

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

extern PON_STATUS REMOTE_PASONU_classifier_l3l4_add_filter ( 
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
extern PON_STATUS REMOTE_PASONU_classifier_l3l4_remove_filter ( 
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
extern PON_STATUS REMOTE_PASONU_classifier_l3l4_get_filter ( 
                                   const PON_olt_id_t                          olt_id,
                                   const PON_onu_id_t                          onu_id,
                                   const PON_pon_network_traffic_direction_t   direction,
                                   const PON_traffic_qualifier_t               traffic,    
                                   const PON_traffic_address_t                 address,    
                                   const unsigned long                         ip_address, 
                                   const unsigned short int                    l4_port_num,
                                   PON_forwarding_action_t                    *action );


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

extern PON_STATUS REMOTE_PASONU_classifier_add_filter ( 
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
extern PON_STATUS REMOTE_PASONU_classifier_get_filter ( 
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

extern PON_STATUS REMOTE_PASONU_classifier_remove_filter ( 
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

extern PON_STATUS REMOTE_PASONU_classifier_add_da_filter ( 
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

extern PON_STATUS REMOTE_PASONU_classifier_add_source_address_filter ( 
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


extern PON_STATUS REMOTE_PASONU_classifier_remove_da_filter ( 
                                   const PON_olt_id_t              olt_id, 
                                   const PON_onu_id_t              onu_id, 
                                   const mac_address_t             address);


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

extern PON_STATUS REMOTE_PASONU_uni_set_port( 
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

extern PON_STATUS REMOTE_PASONU_uni_get_port( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable_cpu, 
                                   bool                *enable_datapath );


/* Clear all statistics
**
** This function clears all (hardware and sofware) of the statistic counters.
**
** Input Parameters:
**	    olt_id		   : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		   : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Return codes:
**	    All the defined REMOTE_PASONU_* return codes
**
*/

extern PON_STATUS REMOTE_PASONU_clear_all_statistics ( const PON_olt_id_t olt_id, const PON_onu_id_t onu_id);

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
PON_STATUS REMOTE_PASONU_set_slow_protocol_limit( 
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
PON_STATUS REMOTE_PASONU_get_slow_protocol_limit( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_onu_id_t   onu_id,
                                   bool                *enable);


	
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
extern PON_STATUS REMOTE_PASONU_uni_set_port_configuration( 
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
extern PON_STATUS REMOTE_PASONU_uni_get_port_configuration( 
                                   const PON_olt_id_t        olt_id, 
                                   const PON_onu_id_t        onu_id,
                                   PON_uni_configuration_t  *uni_configuration );
                                   
                                   
extern short int Set_external_downlink_buffer_size  ( const short int                              olt_id,
     								           const PON_external_downlink_buffer_size_t    external_downlink_buffer_size );

extern short int Get_external_downlink_buffer_size  ( const short int                              olt_id,
							  		                 PON_external_downlink_buffer_size_t   *external_downlink_buffer_size );



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

extern PON_STATUS REMOTE_PASONU_eeprom_mapper_set_parameter ( 
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

extern PON_STATUS REMOTE_PASONU_eeprom_mapper_get_parameter 
                              ( const PON_olt_id_t		      olt_id, 
								const PON_onu_id_t	          onu_id,
								const EEPROM_mapper_param_t   parameter, 
                                void                         *data,
								unsigned long                *size);
								
/* Update ONU firmware
**
** Update ONU device's firmware and initiate an ONU firmware switch process.
** All file preparations for reading, as well as post-reading manipulations are done within the 
** command (e.g. open file, close file). The ONU will deregister itself and reregister back again 
** (using the new firmware) following this command automatically.
**
** Limitations:
** ?ONU firmware can be updated using this command only after ONU has registered for the first time 
**   (upgrade function only)
** ?This command is applicable only for an ONU model designed to support this function by the System 
**   vendor.
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id		    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		onu_firmware	: ONU firmware definitions. See PON_binary_t for details. 
**						  Binary type: always PON_ONU_BINARY_FIRMWARE
**					
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/
extern PON_STATUS REMOTE_PASONU_update_onu_firmware ( const PON_olt_id_t   olt_id, 
					const PON_onu_id_t   onu_id,
					const PON_binary_t  *onu_firmware );


extern PON_STATUS REMOTE_MANAGEMENT_update_onu_firmware (
                                const PON_olt_id_t    olt_id, 
				const PON_onu_id_t    onu_id,
				const PON_binary_t   *onu_firmware);


typedef enum
{
  PON_IMAGE_SELECT_AUTO = -1,
  PON_IMAGE_SELECT_FIRST,
  PON_IMAGE_SELECT_SECOND,
  PON_IMAGE_SELECT_MAX
} PON_image_select_t;
				
/* Update ONU firmware image index
**
** Update ONU device's firmware for a specific image indesx and initiate an ONU firmware switch process.
** All file preparations for reading, as well as post-reading manipulations are done within the 
** command (e.g. open file, close file). The ONU will deregister itself and reregister back again 
** (using the new firmware) following this command automatically.
**
** Limitations:
** ?ONU firmware can be updated using this command only after ONU has registered for the first time 
**   (upgrade function only)
** ?This command is applicable only for an ONU model designed to support this function by the System 
**   vendor.
**
** Input Parameters:
**		olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		onu_firmware		: ONU firmware definitions. See PON_binary_t for details. 
**							  Binary type: always PON_ONU_BINARY_FIRMWARE
**		active_image_index	: Active image to be set. See PON_image_select_t for details.
**					
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

extern PON_STATUS REMOTE_PASONU_update_onu_firmware_image_index
							( const PON_olt_id_t		 olt_id, 
							  const PON_onu_id_t		 onu_id,
							  const PON_binary_t		*onu_firmware,
							  const PON_image_select_t   active_image_index );


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
extern PON_STATUS REMOTE_PASONU_get_burn_image_complete (
                                const PON_olt_id_t   olt_id, 
				const PON_onu_id_t   onu_id,
                                const bool          *complete);


/* Set active flash image
**
** This function sets the active image in the flash. After reset the image that has been set using 
** this function will be loaded into memory
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		active_image_index  : Active image to be set. 0 - 1.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/
extern PON_STATUS REMOTE_PASONU_flash_image_set_active (
                                const PON_olt_id_t   olt_id, 
				const PON_onu_id_t   onu_id,
				const unsigned char  active_image_index);


/* Get active flash image
**
** This function gets the current active image.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		active_image_index  : current active image. 0 - 1.
**
** Return codes:
**		All the defined REMOTE_PASONU_* return codes
**
*/

extern PON_STATUS REMOTE_PASONU_flash_image_get_active (
                                const PON_olt_id_t   olt_id, 
				const PON_onu_id_t   onu_id,
				unsigned char       *active_image_index);
				
	
/* Read MDIO register									
**
** Read an OLT MDIO register from the address specified.
**
** Input Parameters:
**			    olt_id		 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    phy_address  : Physical address, range:	MDIO_MAX_PHY_ADDRESS - MDIO_MAX_PHY_ADDRESS
**				reg_address  : Register address, range: MDIO_MAX_REG_ADDRESS - MDIO_MAX_REG_ADDRESS
**    
** Output Parameters:
**				value		 : Register value, range:  unsigned short int values 
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_LONG_STATUS PAS_read_mdio_register 
                                       ( const PON_olt_id_t    olt_id, 
							             const short int       phy_address, 
							             const short int       reg_address,
							             unsigned short int   *value);

/* Read OLT EEPROM register
**
** Read an OLT EEPROM device register content. 
**
** Input Parameters:
**			    olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				register_address	: EEPROM register number
**
** Output Parameters:
**				data				: EEPROM register content, range: unsigned short int values
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_read_olt_eeprom_register 
                                       ( const PON_olt_id_t		olt_id, 
										 const unsigned short	register_address, 
										 unsigned short int    *data );
/* Get CNI management mode
**
** This API retrieve cni management mode process state in the OLT FW. 
**
** Input Parameters:
**		olt_id  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      mode    : cni management mode.
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/

extern PON_STATUS PAS_get_cni_management_mode 
                                    ( const PON_olt_id_t   olt_id,
		                                    bool          *mode );
		                                
/* Get oam configuration
**
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      limitation		: oam limit. Values: ENABLE, DISABLE 
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_get_oam_configuration
                                    ( const PON_olt_id_t   olt_id,      
                                      bool          *ignore_oam_limit );

#ifndef MAX_PARAM_LENGTH
#define MAX_PARAM_LENGTH                     1000  /* 1024 */
#endif


/* ONU BER raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   or multicast LLIDs including discovery-time data)
*/
typedef struct PON_onu_ber_raw_data_t
{
	unsigned long  error_bytes;			   /* Error bytes received by the OLT (PON port)				*/
	long double	   used_byte_count;		   /* Total bytes containing data received by the OLT which     */
										   /* originated from the measured ONU (not including IDLEs)	*/
	long double	   good_byte_count;        /* Total bytes received by the OLT which	originated from the	*/
										   /* measured ONU (including IDLEs)							*/
} PON_onu_ber_raw_data_t;

/* Unsigned long range: 0 - 0xFFFFFFFF */
#define MINULONG 0
#define MAXULONG (0xFFFFFFFF)

#define PON_RAW_STAT_ONU_BER_ERROR_BYTES_MAX_HARDWARE_VALUE			    MAXULONG
#define PON_RAW_STAT_ONU_BER_GOOD_BYTE_COUNT_MAX_HARDWARE_VALUE			(pow(2,64)-1) 
#define PON_RAW_STAT_ONU_BER_USED_BYTE_COUNT_MAX_HARDWARE_VALUE		    (pow(2,64)-1) 


/* OLT BER raw statistics */
typedef struct PON_olt_ber_raw_data_t
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

#define PON_RAW_STAT_OLT_BER_ERROR_BYTES_MAX_HARDWARE_VALUE			  MAXULONG
#define PON_RAW_STAT_OLT_BER_BYTE_COUNT_MAX_HARDWARE_VALUE			  ((pow(2,64)*BYTES_IN_SYMBOL)-1)


/* ONU FER raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   LLID including discovery-time frames)
*/
typedef struct PON_onu_fer_raw_data_t
{
	unsigned long  received_error;		  /* Error frames received by the OLT and originated from the  */
										  /* ONU measured											   */
	unsigned long  received_ok;			  /* Good frames received by the OLT and originated from the   */
										  /* ONU measured											   */
	unsigned long  firmware_received_ok;  /* Good frames received by the OLT, originated from the ONU  */
										  /* measured, and destined to the OLT firmware (including host*/
										  /* frames)												   */
} PON_onu_fer_raw_data_t;

#define PON_RAW_STAT_ONU_FER_RECEIVED_ERROR_MAX_HARDWARE_VALUE		  MAXULONG
#define PON_RAW_STAT_ONU_FER_RECEIVED_OK_MAX_HARDWARE_VALUE			  MAXULONG
#define PON_RAW_STAT_ONU_FER_FIRMWARE_RECEIVED_OK_MAX_HARDWARE_VALUE  MAXULONG

/* 以下统计类型定义为统计任务使用*/
/* Unicast frames raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**							   PON_BROADCAST_LLID (for multicast LLID)
**							   
*/
typedef struct PON_unicast_frames_raw_data_t
{
	unsigned long  received_ok;	    /* Received frames (by the PON port) which were directed to an  */
									/* active non-broadcast group MAC address					    */
	unsigned long  transmitted_ok;  /* Successfully transmitted frames (through the PON port) which */
									/* were directed to an active non-broadcast group MAC address	*/
} PON_unicast_frames_raw_data_t;

/* Broadcast frames raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, 
**							   PON_BROADCAST_LLID (for multicast LLID)
**							   
*/
typedef struct PON_broadcast_frames_raw_data_t
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
typedef struct PON_multicast_frames_raw_data_t
{
	unsigned long  received_ok;	    /* Received frames (by the PON port) which were directed to an  */
									/* active non-broadcast group MAC address					    */
	unsigned long  transmitted_ok;  /* Successfully transmitted frames (through the PON port) which */
									/* were directed to an active non-broadcast group MAC address	*/
} PON_multicast_frames_raw_data_t;

/* Frame check sequence errors raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   / multicast LLID including discovery-time data)
*/
typedef struct PON_frame_check_sequence_errors_raw_data_t
{
	unsigned long  received;		/* Received frames (PON port) which had an integral number   */
									/* of bytes (octets) in length and didn't pass the FCS check */
} PON_frame_check_sequence_errors_raw_data_t;

/* Alignment errors raw statistics */
typedef struct PON_alignmnet_errors_raw_data_t
{
	unsigned long  received;	/* Received frames (PON port) which didn't have an integral         */
								/* number of bytes (octets) in length and didn't pass the FCS check.*/
								/* Not implemented - thus always 0									*/

} PON_alignmnet_errors_raw_data_t;

/* In range length errors per LLID raw statistics
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID (broadcast
**							   / multicast LLID including discovery-time data)
*/
typedef struct PON_in_range_length_errors_per_llid_raw_data_t
{
	unsigned long  received;	   /* system received - length/type field value was less than the minimum */
								   /* allowed unpadded data size and the number of data octets            */
								   /* received was greater than the minimum unpadded data size            */

} PON_in_range_length_errors_per_llid_raw_data_t;

/* In range length errors raw statistics
**
** Statistics parameter: 0
*/
typedef struct PON_in_range_length_errors_raw_data_t
{
	unsigned long  system_received;	 /* length/type field value was less than the minimum		  */
									 /* allowed unpadded data size and the number of data octets  */
									 /* received was greater than the minimum unpadded data size  */
	unsigned long  pon_received;	 /* length/type field value was less than the minimum		  */
									 /* allowed unpadded data size and the number of data octets  */
									 /* received was greater than the minimum unpadded data size  */
                                     /* added by PAS5201                                          */  

} PON_in_range_length_errors_raw_data_t;


/* Frame too long errors raw statistics
**
** Statistics parameter: 0
*/
typedef struct PON_frame_too_long_errors_raw_data_t
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
typedef struct PON_frame_too_long_errors_per_llid_raw_data_t
{
	unsigned long  received;	/* Received frames (from the specified LLID) which exceeded the */
								/* maximum permitted frame size									*/
} PON_frame_too_long_errors_per_llid_raw_data_t;

/* HEC frames errors raw statistics */
typedef struct PON_hec_frames_errors_raw_data_t
{
	unsigned long  received; /* Received frames (PON port) with HEC (header error correction) errors */
} PON_hec_frames_errors_raw_data_t;

/* Unsupported MPCP frames raw statistics */
typedef struct PON_unsupported_mpcp_frames_raw_data_t
{
	unsigned long  received;			  /* Received frames (PON port) with unsupported MPCP type  */
} PON_unsupported_mpcp_frames_raw_data_t;

/* Single collision frames raw statistics */
typedef struct PON_single_collision_frames_raw_data_t
{
	unsigned long  single_collision_frames;		/* Single collision frames counter - always 0 in EPON */
} PON_single_collision_frames_raw_data_t;

/* Multiple collision frames raw statistics */
typedef struct PON_multiple_collision_frames_raw_data_t
{
	unsigned long  multiple_collision_frames;/* Multiple collision frames counter - always 0 in EPON */
} PON_multiple_collision_frames_raw_data_t;

/* Pause time raw statistics */
typedef struct PON_pause_time_raw_data_t
{
	unsigned long  system_port;				/* 'System port in pause mode' time, 512 nSsec resolution */
											/* (PON_STATISTICS_SAMPLING_TIMESTAMP_CYCLE_LONG cycle)   */
} PON_pause_time_raw_data_t;

/* Registration frames raw statistics */
typedef struct PON_registration_frames_raw_data_t
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
typedef struct PON_oam_frames_raw_data_t
{
	unsigned long  received_ok;					 /* Received OAM frames (PON port)					 */
	unsigned long  transmitted_ok;				 /* Successfully transmitted OAM frames (PON port)	 */
} PON_oam_frames_raw_data_t;


typedef enum
{
	PON_MPCP_MODE_OLT = 0x01,
	PON_MPCP_MODE_ONU = 0x02
} PON_mpcp_mode_t;

typedef enum
{
	PON_MPCP_STATUS_UNREGISTERED = 0x01,
	PON_MPCP_STATUS_REGISTERING	 = 0X02,
	PON_MPCP_STATUS_REGISTERED   = 0X03
} PON_mpcp_registration_status_t;

/* MPCP status raw statistics */
typedef struct PON_mpcp_status_raw_data_t
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
typedef struct PON_host_frames_raw_data_t
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
    unsigned long  pon_ram_dropped;                     /* Frames received that originated from the PON*/
                                                        /* port and were destined for the host, but    */
                                                        /* were dropped because the buffer was full    */
                                                        /* (dropped from outside the queue, all queue  */
                                                        /* RAM is full) ,from PAS5201                  */
	unsigned long  system_dropped;						/* Received frames from System port, destined  */
														/* for the host, which were dropped due to full*/
														/* buffer									   */
    unsigned long  system_ram_dropped;                  /* Frames received that originated from the    */
                                                        /* system port and were destined for the host, */
                                                        /* but were dropped because the buffer was full*/
                                                        /* (dropped from outside the queue, all queue  */
                                                        /* RAM is full)  , from PAS5201                */
    unsigned long  host_rate_limiter_dropped;           /* Frames received that originated from the    */
                                                        /* LLID and were destined for the host, but    */
                                                        /* were dropped because of Host Rate Limiter   */
                                                        /* configuration (exceed  max packet per       */
                                                        /* seconds rate)  , from PAS5201               */
#ifdef   PAS_SOFT_VERSION_V5_3_5
    unsigned long  total_host_destined_frames;          /* Frames that supposed to be send to the host */
                                                        /* but may be discard by the host rate limiter */
#endif
} PON_host_frames_raw_data_t;

/* Host octets raw statistics */
typedef struct PON_host_octets_raw_data_t
{
	unsigned long  received_ok;		/* Received bytes (octets) from the PON port which were destined   */
									/* for the host													   */
	unsigned long  transmitted_ok;	/* Host originated, successfully transmitted (through the PON port)*/
									/* bytes (octets)												   */
} PON_host_octets_raw_data_t;

/* Host messages raw statistics 
**
** Statistics parameter: Host - firmware communication type (PON_comm_type_t values)
** Parallel - Parallel interface queues counters
** Ethernet - Counters of frames traffic between secondary host port and the OLT firmware
** UART		- UART driver framer counters
*/
typedef struct PON_host_messages_raw_data_t
{
	unsigned long  received_from_firmware_ok;	 /* Messages sent from the OLT firmware to the Host    */
	unsigned long  sent_to_firmware_ok;			 /* Messages sent by the Host to the OLT firmware	  */
    unsigned long  received_with_error_from_host;/* Messages received with FCS errors sent from the Host to the OLT firmware, from PAS5201*/
} PON_host_messages_raw_data_t;

/* Host messages octets raw statistics */
typedef struct PON_host_messages_octets_raw_data_t
{
	unsigned long  sent_to_firmware;	            /* Octets in received good frames sent from the Host to the OLT firmware  */
											
	unsigned long  received_with_error_from_host;	/* Octets in received frames with FCS error sent from the Host to the OLT firmware    */

} PON_host_messages_octets_raw_data_t;

/* Pause frames raw statistics */
typedef struct PON_pause_frames_raw_data_t
{
	unsigned long  pon_received_ok;				/* Received Pause frames (PON port)					  */
	unsigned long  system_received_ok;			/* Received Pause frames (System port)				  */
	unsigned long  pon_transmitted_ok;			/* Successfully transmitted Pause frames (PON port)	  */
												/* Valid when in P2P mode only						  */
	unsigned long  system_transmitted_ok;		/* Successfully transmitted Pause frames (System port)*/
} PON_pause_frames_raw_data_t;

/* Grant frames raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_DISCOVERY_LLID for  
**							   discovery grants statistics (received_ok will be zeroed)
*/
typedef struct PON_grant_frames_raw_data_t
{
	unsigned long  received_ok;	        /* Received grant (upstream bandwidth allocation) frames	  */
	unsigned long  transmitted_dba_ok;  /* Successfully transmitted grant frames, originated from DBA */
										/* module													  */
	unsigned long  transmitted_ctrl_ok; /* Successfully transmitted 'REGISTER' frames (broadcast LLID)*/
} PON_grant_frames_raw_data_t;

/* MPCP 'REPORT' frames raw statistics */
typedef struct PON_report_frames_raw_data_t
{
	unsigned long  received_ok;					/* Received MPCP 'report' frames					*/
	unsigned long  transmitted_ok;				/* Successfully transmitted MPCP 'report' frames	*/
} PON_report_frames_raw_data_t;

/* OAM statistics raw statistic */
typedef struct PON_oam_statistic_raw_data_t
{
	unsigned long information_tx;					/* Tx OAMPDU with "information" code															*/
	unsigned long information_rx;					/* Rx OAMPDU with "information" code															*/
	unsigned long unique_event_notification_tx;		/* Tx OAMPDU with "event notification" code														*/
	unsigned long unique_event_notification_rx;		/* Rx OAMPDU with "event notification" code with sequence number different from the last one	*/
	unsigned long duplicate_event_notification_tx;	/* Tx OAMPDU with "event notification code" with sequence number equal to the last one			*/
	unsigned long duplicate_event_notification_rx;	/* Rx OAMPDU with "event notification code" with sequence number equal to the last one			*/
	unsigned long oam_loopback_control_tx;			/* Tx OAMPDU with "loopback control" code														*/
	unsigned long oam_loopback_control_rx;			/* Rx OAMPDU with "loopback control" code														*/
	unsigned long oam_variable_request_tx;			/* Tx OAMPDU with "variable request" code														*/
	unsigned long oam_variable_request_rx;			/* Rx OAMPDU with "variable response" code														*/
	unsigned long oam_variable_response_tx;			/* Tx OAMPDU with "variable response" code														*/
	unsigned long oam_variable_response_rx;			/* Rx OAMPDU with "variable response" code														*/
	unsigned long oam_organization_specific_tx;		/*  Tx OAMPDU with "Organization specific" code													*/
	unsigned long oam_organization_specific_rx;		/*  Rx OAMPDU with "Organization specific" code													*/
    unsigned long unsupported_codes_tx;             /* A count of the number of OAMPDUs transmitted  on this interface with an unsupported op-code  */
    unsigned long unsupported_codes_rx;             /* A count of the number of OAMPDUs received  on this interface with an unsupported op-code     */
	unsigned long frames_lost_due_to_oam_error;		/* number of frames that not sent due to OAM error												*/
} PON_oam_statistic_raw_data_t;

/* MPCP statistics group raw statistics */
typedef struct PON_mpcp_statistic_raw_data_t
{
	unsigned long					mac_ctrl_frames_transmitted;        /*  A count of MPCP frames passed to the MAC sublayer for transmission					                          */
	unsigned long					mac_ctrl_frames_received;           /*  A count of MPCP frames passed by the MAC sublayer to the MAC Control sublayer			                      */
    unsigned long                   discovery_windows_sent;             /*  A count of discovery windows generated. The counter is incremented by one for each generated discovery window */
    unsigned long                   discovery_timeout;                  /*  A count of the number of times a discovery timeout occurs. Increment the counter by one for each discovery    */
                                                                        /*  processing state-machine reset resulting from timeout waiting for message arrival                             */

	unsigned long                   register_request_transmitted_ok;	/* Successfully transmitted registration request */
				                    									/* frames, a count of number of attempts to		 */
	unsigned long                   register_request_received_ok;	    /* Successfully reception registration request frames, */
                                                                        /* a count of the number of times a registration frames reception occurs  */
                                    
	unsigned long                   register_ack_transmitted_ok;		/* A count of the number of times a REGISTER_ACK */
                                                                        /* frames transmission occurs                    */       
	unsigned long                   register_ack_received_ok;	    	/* A count of the number of times a REGISTER_ACK */
                                                                        /* frames reception occurs/                      */


    unsigned long                   transmitted_report;	                /* Successfully transmitted MPCP 'report' frames */
    unsigned long                   received_report;	                /* Successfully received MPCP 'report' frames    */

    unsigned long                   transmited_gate;	                /* A count of the number of times a GATE frames transmission occurs */
    unsigned long                   receive_gate;	                    /* A count of the number of times a GATE frames reception occurs    */
    unsigned long                   register_transmited;	            /* A count of the number of times a REGISTER frames transmission occurs */
    unsigned long                   register_received;	                /* A count of the number of times a REGISTER frames reception occurs */

} PON_mpcp_statistic_raw_data_t;

/* Promiscuous Status raw statistics */
typedef struct PON_promiscuous_status_raw_data_t
{
	short int  status;		 /* Promiscuous mode enabled / disabled in the PON port,				*/
							 /* values: DISABLE/ENABLE, always ENABLE								*/
} PON_promiscuous_status_raw_data_t;


/* Duplex Status raw statistics */
typedef struct PON_duplex_status_raw_data_t
{
	PON_duplex_status_t  status;		   /* Duplex status (PON_HALF_DUPLEX / PON_FULL_DUPLEX)    */
										   /* in the PON port, always PON_FULL_DUPLEX			   */
	PON_duplex_status_t  mac_capabilities; /* MAC capabilities in the PON port,					   */
										   /* values: PON_HALF_DUPLEX / PON_FULL_DUPLEX,		   */
										   /* always PON_FULL_DUPLEX							   */
} PON_duplex_status_raw_data_t;

/* Round trip time raw statistics */
typedef struct PON_rtt_raw_data_t
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
typedef struct PON_p2p_frames_raw_data_t
{
	unsigned long  received_ok;						/* Received P2P frames							   */
	unsigned long  dropped_by_policer;				/* Received P2P frames dropped by Policer		   */
	unsigned long  dropped_by_access_control;		/* Received P2P frames dropped by access control   */
	unsigned long  dropped_due_to_tx_buffer_full;   /* Received P2P frames dropped due to full PON	   */
													/* transmit buffer								   */
	unsigned long  transmitted_ok;					/* Successfully transmitted P2P frames			   */
} PON_p2p_frames_raw_data_t;

/* P2P global frames dropped  */
typedef struct PON_p2p_global_frames_dropped_raw_data_t
{
    
	unsigned long  egress_dropped;         /* P2P frames received that were dropped from 
                                              inside the queue, low priority is dropped 
                                              because higher priority                         */
    unsigned long  ram_dropped;            /* P2P frames that were dropped outside the 
                                              queue, all queue RAM is full and not specific 
                                              priority limit is full                          */

} PON_p2p_global_frames_dropped_raw_data_t;

/* IPG (Inter Packet Gap)*/
typedef struct PON_ipg_raw_data_t
{
    unsigned long short_ipg;               /* Frames that dropped due to short IPG */

} PON_ipg_raw_data_t;

/* Discarded unknown destination address raw statistics */
typedef struct PON_discarded_unknown_destination_address_raw_data_t
{
	unsigned long  frames;	/* Received frames with DA unknown that was discarded   */
	unsigned long  octets;	/* Number of octets in received frames with DA unknown that was discarded     */

} PON_discarded_unknown_destination_address_raw_data_t;

/* Uplink VLAN discarded frames */
typedef struct PON_uplink_vlan_discarded_frames_raw_data_t
{
	unsigned long  tagged_frames	    ;  /* Number of discarded single tagged frames, except NULL tagged,
                                              because 'Discard tagged' parameter was set in LLID VLAN manipulation table */
	unsigned long  untagged_frames	    ;  /* Number of discarded untagged frames 
                                              because 'Discard untagged' parameter was set in LLID VLAN manipulation table */
	unsigned long  null_tagged_frames	;  /* Number of Discarded null tagged frames because 
                                              ' Discard NULL-tagged' was set in LLID VLAN manipulation table */
	unsigned long  nested_tagged_frames	;  /* Number of discarded nested tagged frames because 
                                              'Discard nested' was set in LLID VLAN manipulation table */
	unsigned long  discarded_frames	    ;  /* Number of discarded frames because'VLAN manipulation' parameter was
                                              set to 'Frame is discarded' in LLID VLAN manipulation table */

} PON_uplink_vlan_discarded_frames_raw_data_t;

/* Downlink VLAN discarded frames */
typedef struct PON_downlink_vlan_discarded_frames_raw_data_t
{
	unsigned long  nested_tagged_frames	            ;  /* Number of discarded nested tagged frames because 
                                                          'Discard nested' was set in VID downlink VLAN manipulation table */
	unsigned long  destination_discarded_frames	    ;  /* Number of discarded frames because 'Destination' parameter
                                                          was set to 'None' in VID downlink VLAN manipulation table */

} PON_downlink_vlan_discarded_frames_raw_data_t;

/* FEC frames */
typedef struct PON_fec_frames_raw_data_t
{
	unsigned long  fixed_frames	         ; /*   Fames that were fully corrected by FEC                  */ 
	unsigned long  unfixed_frames	     ; /*   Frames with errors that were not fully corrected by FEC */ 
	unsigned long  good_frames	         ; /*   Frames that did not need FEC correction                 */ 
	unsigned long  wrong_parity_number   ; /*   Frames that has wrong number of parity bytes            */ 

} PON_fec_frames_raw_data_t;


/* FEC blocks and octets */
typedef struct PON_fec_blocks_and_octets_raw_data_t
{
    unsigned long   good_blocks                  ; /* Blocks received that were corrected by FEC                        */ 
    unsigned long   fixed_octets                 ; /* Octets in frames that were fully corrected by FEC                 */ 
    unsigned long   unfixed_octets               ; /* Octets in frames with errors that were not fully corrected by FEC */ 
    unsigned__int64 good_octets                  ; /* Octets in frames that did not need FEC correction                 */ 
    unsigned__int64 rx_overhead_octets           ; /* Size of total received FEC overhead (parity codes + symbols)      */ 
    unsigned__int64 tx_overhead_octets           ; /* Transmitted FEC overhead                                          */ 

} PON_fec_blocks_and_octets_raw_data_t;


/* Total dropped RX good frames*/
typedef struct PON_total_dropped_rx_good_frames_raw_data_t
{
    unsigned long   downlink_dropped    ; /* Total good frames that were received from the system (Downlink)
                                             and were dropped because user configuration of the VLAN modules 
                                             (sum of 135-364, 135-365,135-302 )*/
    unsigned long   uplink_dropped      ; /* Total good frames that were received from the PON (Uplink) and 
                                             were dropped because user configuration of the address table, 
                                             classifier or VLAN modules (sum of 135-354..358, 135-304, 135-399
                                             of all llids ) */
} PON_total_dropped_rx_good_frames_raw_data_t;

     

/* Total octets in dropped RX frames due to errors */
typedef struct PON_total_octets_in_dropped_rx_good_frames_raw_data_t
{
    unsigned__int64 downlink_dropped	 ; /* Total good frames that were received from the system (Downlink)
                                              and were dropped because user configuration of the VLAN modules 
                                              (sum of 135-366, 135-367,135-303 ) */
    unsigned__int64 uplink_dropped       ; /* Total good frames that were received from the PON (Uplink) and 
                                              were dropped because user configuration of the  classifier or VLAN
                                              modules (sum of 135-359..363, 135-305 of all llids) */
} PON_total_octets_in_dropped_rx_good_frames_raw_data_t;


/* Transmited good frames/octets from PQ  */
typedef struct PON_transmitted_good_frames_octets_from_pq_raw_data_t
{
    unsigned long  uplink_pq_frames  ;    /* Transmited good frames from uplink PQ   */
    unsigned long  uplink_pq_octets  ;    /* Transmited good octets from uplink PQ   */
    unsigned long  downlink_pq_frames;    /* Transmited good frames from downlink PQ */
    unsigned long  downlink_pq_octets;    /* Transmited good octets from downlink PQ */

} PON_transmitted_good_frames_octets_from_pq_raw_data_t;


/* Unicast/Multicast PON frames  */
typedef struct PON_unicast_multicast_pon_frames_raw_data_t
{
    unsigned long  rx_multicast_frames  ;    /* Received multicast frames from PON  */
    unsigned long  tx_multicast_frames  ;    /* Transmited multicast frames to PON  */
    unsigned long  rx_unicast_frames	;    /* Received unicast frames from PON	*/
    unsigned long  tx_unicast_frames	;    /* Transmited unicast frames to PON	*/

} PON_unicast_multicast_pon_frames_raw_data_t;


/* Received frames to CPU per priority raw statistics */
typedef struct PON_received_frames_to_cpu_per_priority_raw_data_t
{
    /* Number of frames, received from the physical port and send to CPU, per priority */
	unsigned long  received_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 
	
} PON_received_frames_to_cpu_per_priority_raw_data_t;

/* Encrypt frames raw statistics */
typedef struct PON_encrypt_frames_raw_data_t
{
	unsigned long  encrypt;	    /* The number of encrypted frames transmitted at PON port               */
											
	unsigned long  decrypt;	/* The number of frames received at PON port, with encryption           */
	
} PON_encrypt_frames_raw_data_t;

/* Start, End symbols frames raw statistics */
typedef struct PON_start_end_symbols_frames_raw_data_t
{
	unsigned long  s_symbol;	/* The number of frames received at PON port, with regular start symbol  */
											
	unsigned long  t_symbol;	/* The number of frames received at PON port, with regular end symbol   */
	
} PON_start_end_symbols_frames_raw_data_t;

/* CPU ports frames raw statistics */
typedef struct PON_cpu_ports_frames_raw_data_t
{
	unsigned long  to_cpu;	    /* Number of frames, transmitted from the SYSTEM port to the CPU  */
											
	unsigned long  from_cpu;	/* Number of frames, transmitted to the SYSTEM port from the CPU   */
	
} PON_cpu_ports_frames_raw_data_t;

/* Total dropped CPU RX frames raw statistics */
typedef struct PON_total_dropped_cpu_rx_frames_raw_data_t
{
	unsigned long  total_pon_dropped;	/* Total dropped frames received from  PON to CPU due to queue overflow.  */
											
	unsigned long  total_system_dropped;/* Total dropped frames received from  SYSTEM to CPU due to queue overflow*/
	
} PON_total_dropped_cpu_rx_frames_raw_data_t;


/* OAM frames counters raw statistics */
typedef struct PON_oam_frames_counters_raw_data_t
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


/* Bridge frames raw statistics */
typedef struct PON_bridge_frames_raw_data_t
{
	unsigned long  received;						/* Received and bridged frames (from System port)  */
} PON_bridge_frames_raw_data_t;

/* Dropped frames per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID)
*/
typedef struct PON_dropped_frames_per_llid_raw_data_t
{
	unsigned long  policer_dropped;		  /* Frames, destined to the LLID, dropped by downlink policer   */
	unsigned long  mismatch_dropped;	  /* Frames dropped due to LLID mismatch (frames received when   */
										  /* LLID not granted)										     */
    unsigned long  address_table_dropped; /* Uplink frames that were dropped due to the address table 
                                             entry limiter , from PAS5201                                */
} PON_dropped_frames_per_llid_raw_data_t;

/* Total dropped RX frames raw statistics */
typedef struct PON_total_dropped_rx_frames_raw_data_t
{
	unsigned long  total_control_dropped;	/* (PON received) Frames dropped due to full control queue */
	unsigned long  source_alert_dropped;    /* Frames dropped due to source alert (frame source MAC    */
											/* address == OLT MAC address)					           */
    unsigned long  pon_match           ;    /* Frames dropped due to match address (frame's source MAC */
                                            /* address learned before from the system port)            */
} PON_total_dropped_rx_frames_raw_data_t;

/* Total dropped TX frames raw statistics */
typedef struct PON_total_tx_dropped_frames_raw_data_t
{
	unsigned long    total_pon_dropped;		    /* Total dropped frames (to transmit to the PON) due               */
											    /* to queue overflow								               */
	unsigned__int64  total_pon_dropped_64bit;   /* Total dropped frames (to transmit to the PON) due               */
											    /* to queue overflow, 64 bit value	if available	(from PAS5201) */
	unsigned long    total_system_dropped;	    /* Total dropped frames (to transmit to the System)                */
											    /* due to queue overflow							               */
	unsigned__int64  total_system_dropped_64bit;/* Total dropped frames (to transmit to the System) due            */
											    /* to queue overflow, 64 bit value	if available	(from PAS5201) */   
                                                                                                                        
    unsigned long    pon_ram_dropped;           /* Frames that were destined for the PON but were dropped outside       
                                                   the queue, all queue RAM is full and not specific priority limit     
                                                   is full , from PAS5201                                          */   
    unsigned long    egress_pon_dropped;        /* Frames that were destined for the PON but were dropped from          
                                                   inside the queue, low priority is dropped because higher             
                                                   priority, from PAS5201                                          */   
                                                                                                                        
    unsigned long    system_ram_dropped;        /* Frames that were destined for the system but were dropped outside    
                                                   the queue, all queue RAM is full and not specific priority limit     
                                                   is full , from PAS5201                                          */   
    unsigned long    egress_system_dropped;     /* Frames that were destined for the system but were dropped from       
                                                   inside the queue, low priority is dropped because higher             
                                                   priority, from PAS5201                                          */   
} PON_total_tx_dropped_frames_raw_data_t;

/* OAM information data raw statistics */
typedef struct PON_oam_information_data_raw_data_t
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

/* Transmitted frames from CPU per priority */
typedef struct PON_transmitted_frames_from_cpu_per_priority_raw_data_t
{
    /* Number of frames, transmitted to the physical port from the CPU,per priority */
	unsigned long  transmitted_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 
	
} PON_transmitted_frames_from_cpu_per_priority_raw_data_t;


/* LLID-Broadcast frames raw statistics */
typedef struct PON_llid_Broadcast_frames_raw_data_t
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

/* Total frames raw statistics 
**
** Statistics parameter: OLT_PHYSICAL_PORT_PON / OLT_PHYSICAL_PORT_SYSTEM  - for OLT collector
**						 ONU_PHYSICAL_PORT_PON / ONU_PHYSICAL_PORT_SYSTEM  - for ONU collector 
**
** Note: OLT FER (average OLT frames error rate (downstream) as measured by the ONU) 
** raw statistics data can collected using this raw statistics type
*/
typedef struct PON_total_frames_raw_data_t
{
	unsigned long    received_ok;			   /* Good frames received by the physical port				                                             */
    unsigned__int64  received_ok_64bit;        /* Good frames received by the physical port , 64bit value if available (from PAS5201)  	             */
	unsigned long    received_error;		   /* Error frames received by the physical port			                                             */
    unsigned__int64  received_error_64bit;	   /* Error frames received by the physical port , 64bit value if available (from PAS5201)               */
	unsigned long    transmitted_ok;		   /* Frames transmitted successfully through the physical port                                          */
    unsigned__int64  transmitted_ok_64bit;	   /* Frames transmitted successfully through the physical port, 64bit value if available (from PAS5201) */
    
    unsigned long  total_bad;		       /* Total bad received frames through the physical port      */
} PON_total_frames_raw_data_t;

#define PON_RAW_STAT_TOTAL_FRAMES_RECEIVED_OK_MAX_HARDWARE_VALUE		  MAXULONG
#define PON_RAW_STAT_TOTAL_FRAMES_RECEIVED_ERROR_MAX_HARDWARE_VALUE		  MAXULONG
#define PON_RAW_STAT_TOTAL_FRAMES_TRANSMITTED_OK_MAX_HARDWARE_VALUE		  MAXULONG

/* Total octets raw statistics 
**
** Statistics parameter: OLT_PHYSICAL_PORT_PON / OLT_PHYSICAL_PORT_SYSTEM  - OLT collector
**						 ONU_PHYSICAL_PORT_PON / ONU_PHYSICAL_PORT_SYSTEM  - ONU collector
*/
typedef struct PON_total_octets_raw_data_t
{
	long double    received_ok;		/* Bytes (octets), data and padding, received by the physical port */
	long double    received_error;	/* Error bytes(octets), data and padding, received by the physical */
									/* port															   */
	long double    transmitted_ok;	/* Bytes (octets), data and padding, transmitted successfully	   */
									/* through the physical port									   */
    long double    received_total;	/* Total received octets through the physical port	               */
} PON_total_octets_raw_data_t;

/* Transmitted bytes per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID including discovery frames)
*/
typedef struct PON_transmitted_bytes_per_llid_raw_data_t
{
	long double  transmitted_ok;				/* Transmitted bytes (through the PON port) per LLID */
} PON_transmitted_bytes_per_llid_raw_data_t;

/* (Received from the UNI port and) Transmitted frames per priority raw statistics */
typedef struct PON_transmitted_frames_per_priority_raw_data_t
{
	/* Number of frames, received from the UNI port and transmitted to the PON port, per priority */
	unsigned long  transmitted_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 							  
} PON_transmitted_frames_per_priority_raw_data_t;

/* Transmitted frames per LLID raw statistics 
**
** Statistics parameter range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT, PON_BROADCAST_LLID 
**							   (broadcast / multicast LLID including discovery frames)
*/
typedef struct PON_transmitted_frames_per_llid_raw_data_t
{
	unsigned long  pon_ok;		 /* Transmitted frames (through the PON port) to the LLID			    */
	unsigned long  system_ok;	 /* Transmitted frames (through the System port) originated by the LLID */
} PON_transmitted_frames_per_llid_raw_data_t;

/* Dropped frames raw statistics */
typedef struct PON_dropped_frames_raw_data_t
{
	unsigned long  dropped[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1];	/* Dropped frames (to transmit */
													                    /* to the PON) due to queue    */
																		/* overflow per priority	   */
} PON_dropped_frames_raw_data_t;

/* Egress dropped Frames (Per Priority)  */
typedef struct PON_egress_dropped_frames_per_priority_raw_data_t
{
    /* Frames with priority priority-index (0 to 7) 
       that were to be transmitted to the PON but were dropped due
       to higher priority queue overflow  */
	unsigned long  dropped[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 

} PON_egress_dropped_frames_per_priority_raw_data_t;



/* Downlink received  Frames/octets per VID */
typedef struct PON_downlink_received_frames_octets_per_vid_raw_data_t
{
    
	unsigned long  received_frames;         /* Received frames from CNI with selected VLAN ID */
    unsigned long  received_octets;         /* Received octets from CNI with selected VLAN ID */

} PON_downlink_received_frames_octets_per_vid_raw_data_t;


#ifdef PAS_SOFT_VERSION_V5_3_13
typedef struct PON_collected_by_olt_per_llid_basic_bulked_t
{
	PON_onu_ber_raw_data_t	onu_ber_raw_data;
	PON_onu_fer_raw_data_t	onu_fer_raw_data;
	PON_transmitted_bytes_per_llid_raw_data_t	transmitted_bytes;
	PON_transmitted_frames_per_llid_raw_data_t	transmitted_frames;
	PON_dropped_frames_per_llid_raw_data_t		dropped_frames;
	PON_frame_check_sequence_errors_raw_data_t		frame_check_sequence_errors;
	PON_in_range_length_errors_per_llid_raw_data_t	in_range_length_errors;
	PON_frame_too_long_errors_per_llid_raw_data_t	frame_too_long_errors;
	PON_multicast_frames_raw_data_t	multicast_frames;
	PON_broadcast_frames_raw_data_t	broadcast_frames;
} PON_collected_by_olt_per_llid_basic_bulked_t;
#endif

/*added by liyang @2015-03-30 for pon statistics*/
/* Received frames per priority raw statistics */
typedef struct PON_received_frames_per_priority_raw_data_t
{
    /* Number of frames, received from port, with priority 0-7 */
	unsigned long  received_ok[MAX_PRIORITY_QUEUE - MIN_PRIORITY_QUEUE+1]; 

} PON_received_frames_per_priority_raw_data_t;


/*      结束 */


/* Return pointer to the string "true" or "false" according to the input parameter value
**
** Parameters:
**				parameter - determines the return string value
**								 
** Return values:
**				boolean describing string ("true" / "false")
*/
extern char *PON_boolean_string ( const bool  parameter );

/* Simple macro for making a string out of enum values */
#define enum_s(name) #name


struct CLI_return_code_str_t
{
	short int   return_code_val;
	char	   *return_code_str;
};

#define WRITE_PARAMETER(__printf_type__, __parameter_value__, __parameter_name__) \
        vty_out(vty,"%s: "__printf_type__"%s",#__parameter_name__,__parameter_value__,VTY_NEWLINE);
        
#define WRITE_USHORT_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%hu ",__parameter_value__,__parameter_name__)
	
#define WRITE_STRING_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%s ",__parameter_value__,__parameter_name__)
	
#define WRITE_ENABLE_DISABLE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20]; \
		if (__parameter_value__		 == ENABLE)  strcpy(tmp_str,enum_s(Enable)); \
		else if (__parameter_value__ == DISABLE) strcpy(tmp_str,enum_s(Disable)); \
		else strcpy(tmp_str,"Not valid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}
#define WRITE_LONG_HEX_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("0x%x ", __parameter_value__, __parameter_name__) 

#define ERROR_READING_CLI_SPECIAL_TYPE_COMMAND_PARAMETER(__parameter_name__, __special_type_name__) \
	{ \
		vty_out (vty, "Error reading "#__parameter_name__" parameter (of the type "#__special_type_name__"), aborting%s", VTY_NEWLINE); \
		return (EXIT_ERROR); \
	} 
#define PARSE_COMMAND_PARAMETER(__printf_type__, __string_parameter__, __parameter_address__, __parameter_name__) \
	if(sscanf( __string_parameter__, __printf_type__, __parameter_address__) == EOF ) \
	{   	vty_out (vty, "Error reading "#__parameter_name__" parameter for command from input, aborting%s", VTY_NEWLINE); \
			return (EXIT_ERROR); \
	}

#define PARSE_STRING_COMMAND_PARAMETER(__string_parameter__, __parameter_address__, __parameter_name__)\
	PARSE_COMMAND_PARAMETER("%s ", __string_parameter__, __parameter_address__, __parameter_name__) 
	
#define PARSE_STATISTICS_TYPE_COMMAND_PARAMETER(__string_parameter__, __parameter_address__, __parameter_name__) \
	{	PARSE_STRING_COMMAND_PARAMETER(__string_parameter__, &stati_type, __parameter_name__) \
		if      (strcmp(enum_s(onu-ber),(char*)&stati_type) == 0) __parameter_address__ = PON_STAT_ONU_BER; \
		else if (strcmp(enum_s(olt-ber),(char*)&stati_type) == 0) __parameter_address__ = PON_STAT_OLT_BER; \
		else if (strcmp(enum_s(onu-fer),(char*)&stati_type) == 0) __parameter_address__ = PON_STAT_ONU_FER; \
		else if (strcmp(enum_s(olt-fer),(char*)&stati_type) == 0) __parameter_address__ = PON_STAT_OLT_FER; \
		else if (strcmp(enum_s(olt-fer),(char*)&stati_type) == 0) __parameter_address__ = PON_STAT_OLT_FER; \
		else ERROR_READING_CLI_SPECIAL_TYPE_COMMAND_PARAMETER(__parameter_name__, statistics_type) \
	}


#define PRINT_BUFFER_HEADER \
		vty_out(vty,"%s %s", return_buffer_header, VTY_NEWLINE);

#define WRITE_LONG_DOUBLE_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%Lg ", __parameter_value__, __parameter_name__) 

#define WRITE_ULONG_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%u ",__parameter_value__,__parameter_name__)
	
#define WRITE_SHORT_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%hd ",__parameter_value__,__parameter_name__)
	
#define WRITE_LONG_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%d ",__parameter_value__,__parameter_name__)

#define WRITE_SHORT_HEX_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("0x%hx ", __parameter_value__, __parameter_name__) 

#define WRITE_LONG_HEX_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("0x%x ", __parameter_value__, __parameter_name__) 

#define WRITE_LONG_DOUBLE_PARAMETER(__parameter_value__, __parameter_name__) \
	WRITE_PARAMETER("%Lg ", __parameter_value__, __parameter_name__) 

#define WRITE_POLARITY_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[10];\
		if (__parameter_value__		 == PON_POLARITY_ACTIVE_LOW)	strcpy(tmp_str,"Low"); \
		else if (__parameter_value__ == PON_POLARITY_ACTIVE_HIGH)   strcpy(tmp_str,"High"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}


	
#define WRITE_OLT_PARAMETER \
	WRITE_SHORT_PARAMETER(olt_id,OLT_id)

#define WRITE_ONU_PARAMETER \
	WRITE_SHORT_PARAMETER(onu_id,ONU_id)

#define WRITE_LLID_PARAMETER \
	WRITE_SHORT_PARAMETER(llid,LLID)
	
#define WRITE_PON_NETWORK_TRAFFIC_DIRECTION_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == PON_DIRECTION_DOWNLINK)	strcpy(tmp_str,"Downlink"); \
		else if (__parameter_value__ == PON_DIRECTION_UPLINK)	strcpy(tmp_str,"Uplink"); \
		else if (__parameter_value__ == PON_DIRECTION_UPLINK_AND_DOWNLINK)	strcpy(tmp_str,"Uplink & Downlink"); \
		else if (__parameter_value__ == PON_DIRECTION_NO_DIRECTION)	strcpy(tmp_str,"No direction"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}	

#define WRITE_FULL_MODE_BEHAVIOR_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == ENABLE)		strcpy(tmp_str,"remove-entry");\
		else if (__parameter_value__ == DISABLE)		strcpy(tmp_str,"keep-entry");\
        else    strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_TRUE_FALSE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[10];\
		if (__parameter_value__		 == TRUE)	strcpy(tmp_str,"True"); \
		else if (__parameter_value__ == FALSE)  strcpy(tmp_str,"False"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_PON_NETWORK_TRAFFIC_DIRECTION_FALSE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == PON_DIRECTION_DOWNLINK)	strcpy(tmp_str,"Downlink"); \
		else if (__parameter_value__ == PON_DIRECTION_UPLINK)	strcpy(tmp_str,"Uplink"); \
		else if (__parameter_value__ == PON_DIRECTION_UPLINK_AND_DOWNLINK)	strcpy(tmp_str,"Uplink & Downlink"); \
		else if (__parameter_value__ == PON_DIRECTION_NO_DIRECTION)	strcpy(tmp_str,"False"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}
	
#define WRITE_MAC_TYPE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20]; \
		if (__parameter_value__		 == PON_MII)   strcpy(tmp_str,PON_mac_s[PON_MII]); \
		else if (__parameter_value__ == PON_GMII)  strcpy(tmp_str,PON_mac_s[PON_GMII]); \
        else if (__parameter_value__ == PON_TBI)   strcpy(tmp_str,PON_mac_s[PON_TBI]); \
        else                                       strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_DUPLEX_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20]; \
		if (__parameter_value__		 == PON_HALF_DUPLEX)    strcpy(tmp_str,"Half duplex"); \
		else if (__parameter_value__ == PON_FULL_DUPLEX)    strcpy(tmp_str,"Full duplex"); \
        else                                                strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}
#define WRITE_MASTER_SLAVE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == PON_MASTER)	strcpy(tmp_str,"Master"); \
		else if (__parameter_value__ == PON_SLAVE)	strcpy(tmp_str,"Slave"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_AUTHORIZE_MODE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == PON_AUTHORIZE_TO_THE_NETWORK) strcpy(tmp_str,"Authorize"); \
		else if (__parameter_value__ == PON_DENY_FROM_THE_NETWORK)		strcpy(tmp_str,"Deny"); \
		else if (__parameter_value__ == PON_AUTHORIZING_TO_THE_NETWORK)		strcpy(tmp_str,"Authorizing"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_ENCRYPTION_MODE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == PON_ENCRYPTION_DIRECTION_DOWNLINK)				strcpy(tmp_str,"Downlink"); \
		else if (__parameter_value__ == PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK)	strcpy(tmp_str,"Downlink & Uplink"); \
		else if (__parameter_value__ == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)			strcpy(tmp_str,"No encryption"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_UNTAGGED_FRAMES_AUTHENTICATION_VID_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50];\
		if (__parameter_value__		 == PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0X0)	strcpy(tmp_str,"0X0"); \
		else if (__parameter_value__ == PON_OLT_UNTAGGED_FRAMES_AUTHENTICATION_VID_0XFFF)		strcpy(tmp_str,"0XFFF"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}
	
#define PON_ALL_FRAMES_AUTHENTICATE (-1)

#define WRITE_AUTHENTICATED_VID_WITH_ALL_FRAMES_AUTHENTICATE(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50];\
        if (__parameter_value__		 == PON_ALL_FRAMES_AUTHENTICATE) strcpy(tmp_str,"All frames authenticate"); \
        else sprintf(tmp_str,"%d",__parameter_value__); \
        WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}


#define WRITE_VLAN_UPLINK_MANIPULATION_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == PON_OLT_VLAN_UPLINK_MANIPULATION_NO_MANIPULATION                ) strcpy(tmp_str,"No manipulation"         );\
		else if (__parameter_value__ == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_ADDED              ) strcpy(tmp_str,"Vlan tag is added"              );\
		else if (__parameter_value__ == PON_OLT_VLAN_UPLINK_MANIPULATION_VLAN_TAG_IS_EXCHANGED          ) strcpy(tmp_str,"Vlan tag is exchanged");\
		else if (__parameter_value__ == PON_OLT_VLAN_UPLINK_MANIPULATION_FRAME_IS_DISCARDED             ) strcpy(tmp_str,"Frame is discarded"              );\
        else                                                                               strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}


#define WRITE_VLAN_ETHERNET_TYPE_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == PON_OLT_VLAN_ETHERNET_TYPE_8100           ) strcpy(tmp_str,"0X8100"         );\
		else if (__parameter_value__ == PON_OLT_VLAN_ETHERNET_TYPE_9100           ) strcpy(tmp_str,"0X9100"              );\
		else if (__parameter_value__ == PON_OLT_VLAN_ETHERNET_TYPE_88A8           ) strcpy(tmp_str,"0X88A8");\
		else if (__parameter_value__ == PON_OLT_VLAN_ETHERNET_TYPE_CONFIGURABLE   ) strcpy(tmp_str,"Configurable"              );\
        else                                                                               strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}


#define WRITE_VLAN_PRIORITY_WITH_ORIGINAL_PRIORITY(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50];\
        if (__parameter_value__		 == PON_ALL_FRAMES_AUTHENTICATE) strcpy(tmp_str,"VLAN original priority"); \
        else sprintf(tmp_str,"%d",__parameter_value__); \
        WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_VLAN_DESTINATION_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == PON_OLT_VLAN_DESTINATION_NONE                               ) strcpy(tmp_str,"None"                                     );\
		else if (__parameter_value__ == PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE                      ) strcpy(tmp_str,"Address table"                            );\
		else if (__parameter_value__ == PON_OLT_VLAN_DESTINATION_LLID_PARAMETER                     ) strcpy(tmp_str,"LLID parameter"                           );\
		else if (__parameter_value__ == PON_OLT_VLAN_DESTINATION_ADDRESS_TABLE_AND_LLID_PARAMETER   ) strcpy(tmp_str,"Address table and LLID parameter"   );\
        else                                                                               strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_VLAN_DOWNLINK_MANIPULATION_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == PON_OLT_VLAN_DOWNLINK_MANIPULATION_NO_MANIPULATION             ) strcpy(tmp_str,"No manipulation"             );\
		else if (__parameter_value__ == PON_OLT_VLAN_DOWNLINK_MANIPULATION_REMOVE_VLAN_TAG             ) strcpy(tmp_str,"Remove vlan tag"             );\
		else if (__parameter_value__ == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID                ) strcpy(tmp_str,"Exchange VID"                );\
		else if (__parameter_value__ == PON_OLT_VLAN_DOWNLINK_MANIPULATION_EXCHANGE_VID_AND_PRIORITY   ) strcpy(tmp_str,"Exchange VID and priority"   );\
        else                                                                               strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

#define WRITE_BOOLEAN_FORWARDED_DISCARDED_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[20];\
		if (__parameter_value__		 == TRUE)	strcpy(tmp_str,"Discarded"); \
		else if (__parameter_value__ == FALSE)  strcpy(tmp_str,"Forwarded"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}
	
#define WRITE_UP_DOWN_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
		if (__parameter_value__		 == CTC_STACK_LINK_STATE_DOWN)		strcpy(tmp_str,"Down");\
		else if (__parameter_value__ == CTC_STACK_LINK_STATE_UP)		strcpy(tmp_str,"Up");\
        else    strcpy(tmp_str,"Invalid value"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}


#define WRITE_VID_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[50]; \
        if (__parameter_value__		 == DEFAULT_VLAN_TAG_ID)\
        {		strcpy(tmp_str,"Default VID");\
				WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
        }\
        else  {WRITE_USHORT_PARAMETER(__parameter_value__,__parameter_name__) }\
	}


/* Note: Following macros don't include prefix '{'. In a case its needed, the user has to add it himself */
#define PON_MAC_ADDRESS_LOOP short int __mac_address_counter__=0; for (__mac_address_counter__=0; __mac_address_counter__ < BYTES_IN_MAC_ADDRESS; __mac_address_counter__++) 

#define PON_MAC_ADDRESS_CLI_OUT(mac_address)    PON_MAC_ADDRESS_LOOP {vty_out  (vty,"0x%02x ",mac_address[__mac_address_counter__]);}

#define DBA_INFO_BUFFER_SIZE 50
	
typedef enum  
{
	CLI_PLATO_DBA    = 1000,
	CLI_PLATO2_DBA   = 1001,
	CLI_SOCRATES_DBA = 1002,
	CLI_ARCHIMEDES_DBA = 1003,
	CLI_VENDOR_SPECIFIC_DBA = 1004,
	CLI_LAST_DBA
}CLI_dba_t;

/* CLI DBA modes  */

typedef enum
{
	CLI_STATIC_GRANTING_DBA_MODE,
	CLI_INTERNAL_DBA_MODE,
	CLI_EXTRNAL_DBA_MODE
}CLI_dba_mode_t;

extern char *CLI_dba_s[];

/* CLI DBA modes  */			                                    										 								
/*#include "OnuRemoteMgmtFromPas.h"*/

/***   pas-soft v5.3.0    ******/

/* Set RSTP port enable								
**
** Enable / Disable the port 
**
** Input Parameters:
**	    olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_num	: Port number, starting from port 0 - (N-1)
**		enabled	    : Enable / Disable port
**                    TRUE - port is enabled for RSTP. RSTP protocol includes this port.
**                    FALSE - port is disabled for RSTP. RSTP protocol does not include this port.
**
**
** Return codes:
**		All the defined APP_SERVICES_* return codes
**
*/
extern short int REMOTE_PASONU_rstp_port_set_enable (
                            const PON_olt_id_t		  olt_id, 
                            const PON_onu_id_t		  onu_id,
							const unsigned char       port_num,
							const bool                enabled );


/* Set RSTP port enable								
**
** Enable / Disable the port 
**
** Input Parameters:
**	    olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_num	: Port number, starting from port 0 - (N-1)
**
** Output Parameters:
**		enabled	    : Enable / Disable port
**                    TRUE - port is enabled for RSTP. RSTP protocol includes this port.
**                    FALSE - port is disabled for RSTP. RSTP protocol does not include this port.
**
**
** Return codes:
**		All the defined APP_SERVICES_* return codes
**
*/
extern short int REMOTE_PASONU_rstp_port_get_enable (
                            const PON_olt_id_t		  olt_id, 
                            const PON_onu_id_t		  onu_id,
							const unsigned char       port_num,
							      bool               *enabled );

/* Hardware alarm codes */
typedef enum 
{  
	PON_ALARM_CODE_BAD_KEY,                             
	PON_ALARM_CODE_OAM_LINK_DISCONNECTION,              
    PON_ALARM_CODE_OAM_FLAG,                            
	PON_ALARM_CODE_OAM_ERRORED_SYMBOL_PERIOD,           
	PON_ALARM_CODE_OAM_ERRORED_FRAME,                   
	PON_ALARM_CODE_OAM_ERRORED_FRAME_PERIOD,            
	PON_ALARM_CODE_OAM_ERRORED_FRAME_SECONDS_SUMMARY,   
	PON_ALARM_CODE_TOO_MANY_ONU_REGISTERING ,            
#ifdef PAS_SOFT_VERSION_V5_3_13
	PON_ALARM_CODE_ONU_REGISTERING_WITH_EXISTING_MAC ,            
#endif
    PON_ALARM_CODE_DLB_MAIN_STAT_ERR_EVENT = 0x20,                    
    PON_ALARM_CODE_HPROT_PQ_SEGMENT_OVERFLOW_ALARM_CODE,       
#ifdef PAS_SOFT_VERSION_V5_3_13
#else
	PON_ALARM_CODE_VIRTUAL_SCOPE_ONU_LASER_ALWAYS_ON = 0x22,
#endif
	PON_ALARM_CODE_LAST_ALARM                                  
} PON_hardware_alarm_code_t;

extern char *PON_hardware_alarm_code_s[];


/* for support platoV3 */
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
#ifdef PAS_SOFT_VERSION_V5_3_11
#define PLATO3_ECODE_WRONG_DBA_TYPE              12
#define PLATO3_ECODE_ILLEGAL_DBA_TYPE            13
#define PLATO3_ECODE_UNSUPPORTED_DBA_TYPE        14
#define PLATO3_ECODE_ILLEGAL_WEIGHT              15
#define PLATO3_ECODE_WRONG_WEIGHTS_SUM           16
#define PLATO3_ECODE_ILLEGAL_BE_METHOD           17
#define PLATO3_ECODE_UNSUPPORTED_BE_METHOD       18
#define PLATO3_ECODE_ILLEGAL_MIN_SP              19
#endif

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
#ifdef PLATO_DBA_V3_3
  unsigned short class;             /* 0 (lowest priority) ?7 (highest priority) */
#elif defined PLATO_DBA_V3_1
  unsigned char class;              /* 0 (lowest priority) ?7 (highest priority) */
#endif
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


extern PON_STATUS PLATO3_init( void );

extern PON_STATUS PLATO3_cleanup( void );

/* this function should be called by PAS_HANDLER_DBA_EVENT handler function
   when its olt_id parameter is an OLT which runs PLATO3 DBA */
extern PON_STATUS PLATO3_algorithm_event( short olt_id, unsigned short id, short size,
                                   const char *data );

extern PON_STATUS PLATO3_get_info( short olt_id, char *name,
                            unsigned char max_name_size, char *version, 
                            unsigned char max_version_size );           

extern PON_STATUS PLATO3_set_SLA( short olt_id, unsigned char LLID,
                           const PLATO3_SLA_t *SLA, short *DBA_error_code );

extern PON_STATUS PLATO3_get_SLA( short olt_id, unsigned char LLID,
                                 PLATO3_SLA_t *SLA, short *DBA_error_code );

/* PLATO3 handler functions enum */
typedef enum
{
	PLATO3_HANDLER_UPDATE_LLID_FLAGS,    /*Update llid_flags event*/
    PLATO3_HANDLER_LAST_HANDLER,

} PLATO3_handler_index_t;

#define PON_POLICER_MIN_HIGH_PRIORITY_RESERVED 0 
#define PON_POLICER_MAX_HIGH_PRIORITY_RESERVED 131070

/* 
** Policing high priority frames structure
**
** Parameters:
**
**      priority   : Array of 8 booleans to distinguish if frame with priority X 
**                   are high or low priority 
*/ 

typedef struct PON_high_priority_frames_t  
{
  bool priority[MAX_PRIORITY_QUEUE+1];

} PON_high_priority_frames_t;


/* Get policing parameters
**
** Get limiting parameters (restrictions) for a LLID 
**
** Input Parameters:
**      olt_id              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid                : LLID index, range: PON_MIN_LLID_PER_OLT- PON_MAX_LLID_PER_OLT,PON_BROADCAST_LLID
**      policer             : Policer type
** Output Parameters:
**      enable              : Policing Enable / Disable, If disabled, policing_struct is ignored
**      policing_params     : Policing parameters, PON_policing_parameters_t structure
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_policing_parameters 
                                       (const PON_olt_id_t               olt_id,
                                        const PON_llid_t                 llid,
                                        const PON_policer_t              policer,
                                              bool                      *enable,
                                              PON_policing_parameters_t *policing_params     );

/* Set policing thresholds
**
** Set priorities thresholds for an OLT's policing
**
** Input Parameters:
**      olt_id                    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      policer                   : Policer type
**      high_priority_frames      : see PON_high_priority_frames_t
**      high_priority_reserved : Number of bytes reserved for high priority frames in the policer, 
**                                  The value is truncated to the nearest smaller even value, 
**                                  range : PON_POLICING_MIN_HIGH_PRIORITY_RESERVED..
**                                          PON_POLICING_MAX_HIGH_PRIORITY_RESERVED
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_policing_thresholds 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_policer_t                  policer,
                                           const PON_high_priority_frames_t     high_priority_frames,
                                           const long                           high_priority_reserved );
extern PON_STATUS PAS_set_policing_thresholds 
                                          (const PON_olt_id_t                   olt_id,
                                           const PON_policer_t                  policer,
                                           const PON_high_priority_frames_t     high_priority_frames,
                                           const long                           high_priority_reserved );

/* Passave extensions to 802.3 Ethernet frame definitions */
#define PON_MIN_HW_ETHERNET_FRAME_SIZE				  64   /* Maximal Ethernet frame length supported by   */
												           /* Passave hardware including FCS fields        */


/* Passave extensions to 802.3 Ethernet frame definitions */
#define PON_MAX_HW_ETHERNET_FRAME_SIZE				  1600 /* Maximal Ethernet frame length supported by   */
												           /* Passave hardware including FCS fields        */

/* Set frame size limits
**
** Set the frame size limits of a specific OLT direction
**
** Input Parameters:
**      olt_id                         : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      direction                      : PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK
**      untagged_frames_oversize_limit : Untagged frames oversize limit, including FCS (PON_MIN_HW_ETHERNET_FRAME_SIZE..PON_MAX_HW_ETHERNET_FRAME_SIZE)
**      tagged_frames_oversize_limit   : Tagged frames oversize limit, including FCS (PON_MIN_HW_ETHERNET_FRAME_SIZE..PON_MAX_HW_ETHERNET_FRAME_SIZE)
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_set_frame_size_limits 
                                    (const PON_olt_id_t                             olt_id,
                                     const PON_pon_network_traffic_direction_t      direction,
                                     const short int                                untagged_frames_oversize_limit,
                                     const short int                                tagged_frames_oversize_limit );


/* Get frame size limits
**
** Get the frame size limits of a specific OLT direction 
**
** Input Parameters:
**      olt_id                         : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      direction                      : PON_DIRECTION_UPLINK or PON_DIRECTION_DOWNLINK
** Output Parameters:
**      untagged_frames_oversize_limit : Untagged frames oversize limit, including FCS (PON_MIN_HW_ETHERNET_FRAME_SIZE..PON_MAX_HW_ETHERNET_FRAME_SIZE)
**      tagged_frames_oversize_limit   : Tagged frames oversize limit, including FCS (PON_MIN_HW_ETHERNET_FRAME_SIZE..PON_MAX_HW_ETHERNET_FRAME_SIZE)
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_frame_size_limits 
                                    (const PON_olt_id_t                             olt_id,
                                     const PON_pon_network_traffic_direction_t      direction,
                                           short int                               *untagged_frames_oversize_limit,
                                           short int                               *tagged_frames_oversize_limit );



#define PON_VLAN_ORIGINAL_PRIORITY (-1)

/* Set VLAN recognizing
**
** Set VLAN recognizing
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      extended_vlan_type : Extended VLAN type (default types are 0x8100, 0x9100, 0x88a8)
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_vlan_recognizing 
                                    (const PON_olt_id_t            olt_id,
                                     const unsigned short int      extended_vlan_type );
extern PON_STATUS PAS_set_vlan_recognizing 
                                    (const PON_olt_id_t            olt_id,
                                     const unsigned short int      extended_vlan_type );

/* Get VLAN recognizing
**
** Get VLAN recognizing 
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      extended_vlan_type : Extended VLAN type (default types are 0x8100, 0x9100, 0x88a8)
**
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_vlan_recognizing 
                                    (const PON_olt_id_t            olt_id,
                                           unsigned short int     *extended_vlan_type );


/* Set LLID VLAN mode
**
** Set the VLAN handling for uplink frames arriving from a certain LLID
**
** Input Parameters:
**      olt_id             : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid               : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      vlan_uplink_config : Uplink VLAN config structure, see definition
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_llid_vlan_mode 
                                 (const PON_olt_id_t                      olt_id,
                                  const PON_llid_t                        llid,
                                  const PON_olt_vlan_uplink_config_t      vlan_uplink_config );
extern PON_STATUS PAS_set_llid_vlan_mode 
                                 (const PON_olt_id_t                      olt_id,
                                  const PON_llid_t                        llid,
                                  const PON_olt_vlan_uplink_config_t      vlan_uplink_config );

/* Set VID downlink mode
**
** Set the VLAN handling for downlink frames arriving from a certain VID 
** (NULL-tagged and untagged frames are a special case)
**
** Input Parameters:
**      olt_id              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      vid                 : VLAN ID of arriving frame (for NULL-tag use 0, 
**                            for untagged use 4095) ,Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID or  VLAN_UNTAGGED_ID
**                            or DEFAULT_VLAN_TAG_ID to configure the default
**      vid_downlink_config : Downlink VID config structure
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_test_vid_downlink_mode 
                                    (const PON_olt_id_t                       olt_id,
                                     const PON_vlan_tag_t                     vid,
                                     const PON_olt_vid_downlink_config_t      vid_downlink_config );
PON_STATUS PAS_set_vid_downlink_mode 
                                    (const PON_olt_id_t                       olt_id,
                                     const PON_vlan_tag_t                     vid,
                                     const PON_olt_vid_downlink_config_t      vid_downlink_config );
/* Delete VID downlink mode
**
** Remove a VLAN manipulation configuration for downlink frames destined to a certain VID
**
** Input Parameters:
**      olt_id : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      vid    : VLAN ID of arriving frame (for NULL-tag use 0, for untagged use 4095), 
**               Range: VLAN_MIN_TAG_ID..VLAN_MAX_TAG_ID or  VLAN_UNTAGGED_ID
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_test_delete_vid_downlink_mode 
                                       (const PON_olt_id_t        olt_id,
                                        const PON_vlan_tag_t      vid );
PON_STATUS PAS_delete_vid_downlink_mode 
                                       (const PON_olt_id_t        olt_id,
                                        const PON_vlan_tag_t      vid );

/* pon 接收光信号检测*/

#define WRITE_ADC_POLARITY_PARAMETER(__parameter_value__, __parameter_name__) \
	{	char	tmp_str[10];\
		if (__parameter_value__  == PON_POLARITY_POSITIVE)	strcpy(tmp_str,"Positive"); \
		else if (__parameter_value__ == PON_POLARITY_NEGATIVE)   strcpy(tmp_str,"Negative"); \
		WRITE_STRING_PARAMETER(tmp_str,__parameter_name__); \
	}

/* Virtual scope measurement */

/* mesaurment type */
typedef enum
{
    PON_VIRTUAL_MEASUREMENT_LOCK_TIME,
    PON_VIRTUAL_MEASUREMENT_RSSI
}PON_measurement_type_t;

/* mesaurment status */
typedef enum
{
    PON_VIRTUAL_MEASUREMENT_DONE,
    PON_VIRTUAL_MEASUREMENT_NOT_READY
}PON_measurement_status_t;


/* 
** Lock time configuration structure
**
** Parameters:
**      stability_time   : stability time of the lock signal, measured in TQ
**                         
*/
typedef struct PON_lock_time_configuration_t 
{
    unsigned char stability_time;
} PON_lock_time_configuration_t;


/* 
** Lock time result structure
**
** Parameters:
**      lock_time       : Time from start of grant until lock, measured in TQ
**      lock_loss_time  : Time from lock loss until end of grant, measured in TQ
**                         
*/
typedef struct PON_lock_time_result_t  
{
    unsigned short int lock_time;
    unsigned short int lock_loss_time;
} PON_lock_time_result_t;


/* 
** RSSI configuration structure
**
** Parameters:
**      sampling_time   : Sampling time from LLID's grant start, measured in TQ
**                         
*/
typedef struct  
{
    unsigned short int nothing;
} PON_rssi_configuration_t;

/* Interface type */
typedef enum
{
    PON_ADC_INTERFACE_SPI = 0,
    PON_ADC_INTERFACE_I2C = 1
} PON_adc_interface_t;

/* Optics type */
typedef enum
{
    PON_ADC_OPTICS_WTD,
    PON_ADC_OPTICS_HISENSE_LTE4302M,
    PON_ADC_OPTICS_SUPERXON_SOEB4366_PSGA
} PON_adc_optics_t;

/* Polarity type */
typedef enum
{
    PON_POLARITY_POSITIVE = 0,
    PON_POLARITY_NEGATIVE = 1
} PON_adc_polarity_t;

#define VIRTUAL_SCOPE_DETECT_LASER_ALWAYS_ON_THRESHOLD_MIN  0
#define VIRTUAL_SCOPE_DETECT_LASER_ALWAYS_ON_THRESHOLD_MAX  50

#define VIRTUAL_SCOPE_DETECT_SIGNAL_DEGRADATION_THRESHOLD_MIN  0
#define VIRTUAL_SCOPE_DETECT_SIGNAL_DEGRADATION_THRESHOLD_MAX  50

#define VIRTUAL_SCOPE_DETECT_EOL_THRESHOLD_MIN			0.0F
#define VIRTUAL_SCOPE_DETECT_EOL_THRESHOLD_MAX			30.0F
#define VIRTUAL_SCOPE_DETECT_EOL_THRESHOLD_DEFAULT		4.0F
#define VIRTUAL_SCOPE_DETECT_EOL_REFERENCE_DBM_MEASUREMENT_DAYS_DEFAULT	10


#define VIRTUAL_SCOPE_ADC_BITS_NUMBER_MIN		8
#define VIRTUAL_SCOPE_ADC_BITS_NUMBER_MAX		31
#define VIRTUAL_SCOPE_ADC_BITS_NUMBER_DEFAULT	8

#define VIRTUAL_SCOPE_ADC_VDD_MIN		25
#define VIRTUAL_SCOPE_ADC_VDD_MAX		50
#define VIRTUAL_SCOPE_ADC_VDD_DEFAULT	33

#define VIRTUAL_SCOPE_ADC_NUMBER_OF_LEADING_ZEROS_MIN		0
#define VIRTUAL_SCOPE_ADC_NUMBER_OF_LEADING_ZEROS_MAX		127
#define VIRTUAL_SCOPE_ADC_NUMBER_OF_LEADING_ZEROS_DEFAULT	2

#define VIRTUAL_SCOPE_ADC_NUMBER_OF_TRAILING_ZEROS_MIN		0
#define VIRTUAL_SCOPE_ADC_NUMBER_OF_TRAILING_ZEROS_MAX		31
#define VIRTUAL_SCOPE_ADC_NUMBER_OF_TRAILING_ZEROS_DEFAULT	6

#define VIRTUAL_SCOPE_ADC_CLOCK_MIN 1
#define VIRTUAL_SCOPE_ADC_CLOCK_MAX 18
#define VIRTUAL_SCOPE_ADC_CLOCK_DEFAULT 2

#ifdef PAS_SOFT_VERSION_V5_3_11
#define VIRTUAL_SCOPE_RESISTOR_VALUE_MIN     10
#define VIRTUAL_SCOPE_RESISTOR_VALUE_MAX     1000
#define VIRTUAL_SCOPE_RESISTOR_VALUE_DEFAULT 100

#define VIRTUAL_SCOPE_I2C_DEVICE_ID_MIN     0
#define VIRTUAL_SCOPE_I2C_DEVICE_ID_MAX     255
#define SUPERXON_SOEB4366_PSGA_I2C_DEVICE_ID_DEFAULT 0x51

#define INVALID_DBM	10000
#endif


#ifdef PAS_SOFT_VERSION_V5_3_12
typedef enum
{                                      
    PON_DBA_ALLOCATION_OVERHEAD_AUTO             = 0,    /*Auto Mode - Add Overhead only to PMC ONUs which are 6001 or 6201.*/
    PON_DBA_ALLOCATION_OVERHEAD_LEGACY             = 1  /*Legacy Mode - Always add overhead.*/

} PON_dba_allocation_overhead_mode_t;
#endif

	
#ifdef  PAS_SOFT_VERSION_V5_3_5

/* 
** Lock time result structure
**
** Parameters:
**      range       : RSSI range 
**      sample      : Sample measurement
**      vrssi       : Sample measurement stated in 0.1V units
**      dbm         : Sample measurement stated in dBm units
**                         
*/
typedef struct  
{
    bool               range;
    unsigned short int sample;
	float              vrssi;
	float              dbm;
} PON_rssi_result_t;

/* 
** Virtual Scope ONU EOL (End of Life) entry
**
** Parameters:
**      mac_address_t		: ONU MAC address
**      olt_id              : OLT ID
**      onu_id				: ONU ID
**      reference_dbm       : Reference initial dBm
**      elapsed_sample_days	: Sample days elapsed from EOL enabled
*/
typedef struct  
{
	mac_address_t		mac_address;
	PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;
	float				reference_dbm;
	unsigned short		elapsed_sample_days;
} PON_virtual_scope_onu_eol_t;

/* 
** ADC configuration structure
**
** Parameters:
**      adc_measurement_time   : ADC measurement time, measured in ADC clock
**      adc_clock              : ADC clock,  measured in MHz
**      clock_polarity         : Clock polarity
**      cs_polarity            : CS polarity
*/
typedef struct  
{
	unsigned char         adc_bits_number;
	unsigned char         adc_vdd;
	unsigned char         adc_number_of_leading_zeros;
	unsigned char         adc_number_of_trailing_zeros;
	unsigned char         adc_clock;
	PON_adc_polarity_t    clock_polarity;
	PON_adc_polarity_t    cs_polarity;
	
	unsigned short        adc_measurement_start;
	unsigned short        adc_measurement_time;
	PON_adc_interface_t   adc_interface;	

} PON_adc_config_t;



/* 
** Virtual Scope ONU EOL (End of Life) alarm info
**
** Parameters:
**      mac_address_t		: ONU MAC address
**      olt_id              : OLT ID
**      onu_id				: ONU ID
**      reference_dbm       : Reference initial dBm
**      elapsed_sample_days	: Sample days elapsed from EOL enabled
**      current_dbm         : Current measured dBm
**      threshold           : The alarm threshold
*/
typedef struct  
{
	mac_address_t		mac_address;
	PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;
	float				reference_dbm;
	unsigned short		elapsed_sample_days;
	float				measured_dbm;
	float				alarm_threshold;
} PON_virtual_scope_onu_eol_alarm_info_t;

/* Convert rssi to dbm handler
**
** A hook handler called when the system want to convert RSSI value to dBm units, 
** the handler should supply the convertion formula and to return the value stated in dBm units.
**
** Input Parameters:
**			vrssi			    : RSSI value to convert (stated in volts)
**		    temperature			: Current temperature
**
** Output Parameters:
**			dBm					: The result stated in dBm units
**
** Return codes:
**				Return codes are not specified for an event
**
*/
typedef short int (*PAS_convert_rssi_to_dbm_handler_t)
					(
#ifdef  PAS_SOFT_VERSION_V5_3_9
					PON_olt_id_t	olt_id,
#endif
					const float	vrssi, 
					 const float	temperature, 
					 float			*dbm);


/* Get olt temperature handler
**
** A hook handler used to retrieve the OLT temperature, 
** the handler should return the current OLT temperature
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**		    temperature			: OLT temperature
**
** Return codes:
**				Return codes are not specified for an event
**
*/
typedef short int (*PAS_get_olt_temperature_handler_t)
					(PON_olt_id_t		olt_id, 
					 float				*temperature);

/* CNI link handler
**
** Event indicating a cni link state changed
**
** Input Parameters:
**		olt_id	        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		cni_link		: CNI link status, 
**						  values: PAS_EXIT_OK, PAS_EXIT_ERROR
**			
**				
** Return codes:
**				Return codes are not specified for an event
*/
typedef short int (*PAS_cni_link_handler_t) ( const short int  olt_id, 
											  const bool       status );



/* Set virtual scope detect onu signal degradation frequency
**
** Set frequency for detection of ONU signal degradation  
**
** Input Parameters:
**      detect_frequency    : Detection frequency
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_virtual_scope_detect_onu_signal_degradation_frequency (const unsigned short detect_frequency );

extern PON_STATUS PAS_set_virtual_scope_detect_onu_signal_degradation_frequency (const unsigned short detect_frequency );

/* Get virtual scope detect onu signal degradation frequency
**
** Get frequency for detect ONU signal degradation
**
** Input Parameters:
** Output Parameters:
**      detect_frequency    : Detection frequency
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_detect_onu_signal_degradation_frequency(unsigned short *detect_frequency );


/* Set virtual scope adc config 
**
** Set ADC configuration
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      adc_config           : ADC configuration struct
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_virtual_scope_adc_config
                                            (const PON_olt_id_t        olt_id,
                                             const PON_adc_config_t    adc_config);
extern PON_STATUS PAS_set_virtual_scope_adc_config
                                            (const PON_olt_id_t        olt_id,
                                             const PON_adc_config_t    adc_config);

/* Get virtual_scope_adc_config 
**
** Get ADC configuration 
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      adc_config           : ADC configuration struct
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_adc_config
                                           (const PON_olt_id_t        olt_id,
                                            PON_adc_config_t          *adc_config);



/* Set virtual scope onu eol database entry
**
** Set one entry of ONU EOL (End of Life) database
**
** Input Parameters:
**      mac_address          : ONU MAC address
**      entry                : Database entry struct
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_virtual_scope_onu_eol_database_entry
                          (const mac_address_t                 mac_address,
                           const PON_virtual_scope_onu_eol_t   entry);

extern PON_STATUS PAS_set_virtual_scope_onu_eol_database_entry
                          (const mac_address_t                 mac_address,
                           const PON_virtual_scope_onu_eol_t   entry);


/* Get virtual scope onu eol database entry
**
** Get one entry of ONU EOL (End of Life) database
**
** Input Parameters:
**      mac_address          : ONU MAC address
** Output Parameters:
**      entry                : Database entry struct
**
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_onu_eol_database_entry
                          (const mac_address_t           mac_address,
                           PON_virtual_scope_onu_eol_t   *entry);



/* Get virtual scope onu eol database olt entries
**
** Get all entries (per OLT) of ONU EOL (End of Life) database
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      num_of_entries       : The size of the entries array		
** Output Parameters:
**      num_of_entries		 : Actual number of entries in the entries array
**      entries              : Array of database entry structs
**
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_onu_eol_database_olt_entries
                            (const PON_olt_id_t		      olt_id,
                             unsigned short               *num_of_entries,
                             PON_virtual_scope_onu_eol_t  entries[]);


/* Clear virtual scope onu eol database entry
**
** Remove one entry from ONU EOL (End of Life) database
**
** Input Parameters:
**      mac_address          : ONU MAC address
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_clear_virtual_scope_onu_eol_database_entry
                          (const mac_address_t  mac_address);



/* Clear virtual scope onu eol database olt entries
**
** Remove all entries (per OLT) from ONU EOL (End of Life) database
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_clear_virtual_scope_onu_eol_database_olt_entries
                          (const PON_olt_id_t  olt_id);



/* Clear virtual scope onu eol database olt entries number
**
** Get number of entries (per OLT) in ONU EOL (End of Life) database
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
** Output Parameters:
**      num_of_entries       : Number of entries for the specified OLT
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_onu_eol_database_olt_entries_number
                          (const PON_olt_id_t   olt_id,
						   unsigned short       *num_of_entries);


/* Get virtual scope measurement
**
** Get a virtual scope measurement for an LLID
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid             : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      measurement_type : Measurement type, Range: PON_measurement_type_t
**      configuration    : Relevant measurement configuration
** Output Parameters:
**		result           : Measurement result
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_measurement 
                                              (const PON_olt_id_t                olt_id,
                                               const PON_llid_t                  llid,
                                               const PON_measurement_type_t      measurement_type,
                                               const void *                      configuration, 
                                                     void *                      result         );  

/* Get virtual scope rssi measurement
**
** Get virtual scope rssi measurement for an LLID
**
** Input Parameters:
**      olt_id               : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid                 : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
** Output Parameters:
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_virtual_scope_rssi_measurement
                                            (const PON_olt_id_t                                olt_id,
                                             const PON_llid_t                                  llid,
                                             PON_rssi_result_t                                 *rssi_result);

/* Get CNI link status
**
** This API retrieves cni link status in the OLT.
**
** Input Parameters:
**		olt_id  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      status  : cni link status.
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/

extern PON_STATUS PAS_get_cni_link_status 
                                    ( const PON_olt_id_t   olt_id,
		                              bool                 *status );

extern short int virtual_scope_get_onu_voltage(PON_olt_id_t olt_id, PON_onu_id_t onu_id, float *voltage,unsigned short int *sample, float *dbm);

#endif

/* Read I2C register
**
** Read an OLT I2C device register content. 
**
** Input Parameters:
**			    olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				device				: I2C device number, 
**									  range: I2C_MIN_DEVICE_NUMBER - I2C_MAX_DEVICE_NUMBER
**				register_address	: I2C register address, 
**									  range: I2C_MIN_REGISTER_ADDRESS - I2C_MAX_REGISTER_ADDRESS
**
** Output Parameters:
**				data				: I2C register content, 
**									  range: I2C_MIN_REGISTER_VALUE - I2C_MAX_REGISTER_VALUE
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_read_i2c_register 
                                ( const PON_olt_id_t	olt_id, 
								  const short int	    device, 
								  const short int	    register_address, 
								  short int		       *data );

/* 
** Virtual Scope optics and interface
**
** Parameters:
**      adc_optics             : ADC optics
**      adc_interface          : ADC interface
**      resistor_value        : resistor value (0.1KOhm)
*/
typedef struct  
{
 PON_adc_optics_t      adc_optics; 
 PON_adc_interface_t   adc_interface; 
 unsigned short    resistor_value;
} PON_virtual_scope_optics_and_interface_t;
 

/* 
** Virtual Scope i2c parameters
**
** Parameters:
**      i2c_device_id          : i2c device id
**      calibrated             : ADC measurement could be calibrated or not
**      adc_measurement_start  : adc measurement start  
**      adc_measurement_time   : adc measurement time 
*/
 
typedef struct  
{
    unsigned char         i2c_device_id;
 bool                  calibrated; 
 unsigned short        adc_measurement_start;
 unsigned short        adc_measurement_time;
 
} PON_virtual_scope_i2c_params_t;
 
 
extern PON_STATUS PAS_set_virtual_scope_i2c_params
                                            (const PON_olt_id_t                      olt_id,
                                             const PON_virtual_scope_i2c_params_t    i2c_params);


/* Reset OLT counters
**
**
** Input Parameters:
**		olt_id			: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_reset_olt_counters 
                ( const PON_olt_id_t   olt_id);


/*
** Logging and Debug messages API functions
*/

/* Set logging configuration
**
** Change the current logging configuration of a log flag. Log flag corresponds to a group of log
** msgs sharing a common characteristic: severity, place of creation within the software architecture, 
** interface type and so on. Each log message will be printed to the log output destination only when
** the log flag it belongs to is enabled.
**
** Input Parameters:
**				log_flag			: Log flag (group of log msgs) to be set. Values: enum values
**				mode				: Enable / disable the log msgs belonging to the log_flag group. 
**									  Values: ENABLE, DISABLE
**				redirection			: PAS-SOFT internal handling currently supported. 
**									  Values: enum values
**			    olt_id				: OLT id, Values: PON_ALL_ACTIVE_OLTS
**
** Return codes:
**				All the defined PAS_* return codes
**
*/

extern PON_STATUS PAS_test_logging_configuration 
                                        ( const PON_log_flag_t		   log_flag,
										  const bool				   mode,
										  const PON_log_redirection_t  redirection,
										  const short int			   olt_id );

extern PON_STATUS PAS_set_logging_configuration 
                                        ( const PON_log_flag_t		   log_flag,
										  const bool				   mode,
										  const PON_log_redirection_t  redirection,
										  const short int			   olt_id );

/* Define the PASLOG severity flag as the syslog(3) severities */
typedef enum PASLOG_severity_flag_t 
{
    PASLOG_SEVERITY_IGNORE  = -1 ,
    PASLOG_SEVERITY_FATAL	= 0	 , 
    PASLOG_SEVERITY_ALERT	= 1	 , 
    PASLOG_SEVERITY_CRIT  	= 2  , 
    PASLOG_SEVERITY_ERROR	= 3	 , 
    PASLOG_SEVERITY_WARN	= 4	 ,   
    PASLOG_SEVERITY_NOTICE	= 5	 , 
    PASLOG_SEVERITY_INFO	= 6	 ,   
    PASLOG_SEVERITY_DEBUG	= 7	 , 
    PASLOG_SEVERITY_LAST
} PASLOG_severity_flag_t;

#define FUNCTIONALITY_LENGTH 20
typedef unsigned char PASLOG_functionality_t[FUNCTIONALITY_LENGTH];

/* Basic functionalities */
#define  PASLOG_FUNC_IGNORE					  	"ignore"							
#define  PASLOG_FUNC_GENERAL 					"general"          /* General messages											*/
#define  PASLOG_FUNC_SYS_DUMP 					"sys-dump"          /* Sys Dump messages */
/*#define  PASLOG_FUNC_LAST_FUNC                  ""*/

typedef enum PASLOG_destination_t
{
	PASLOG_DESTINATION_NOT_FOUND	= -1,	/* Destination isn't found for specific configuration */ 
	PASLOG_DESTINATION_FILE			= 0,	/* Print LOG messages into file						  */
	PASLOG_DESTINATION_STDOUT,				/* Print LOG messages into stndard output 			  */
	PASLOG_DESTINATION_SYSLOG,				/* Print LOG messages into syslog daemon			  */
	PASLOG_DESTINATION_MEMORY,				/* Print LOG messages into memory					  */
	PASLOG_DESTINATION_USER_DEFINED,		/* Print LOG messages using user defined function     */
	PASLOG_DESTINATION_CLI,					/* Print LOG messages into cli session                */
	PASLOG_DESTINATION_LAST_DEST
}PASLOG_destination_t;


typedef struct PASLOG_configuration_t
{
	PASLOG_severity_flag_t  severity_flag;
	PASLOG_functionality_t	functionality;
	long int  			    olt_id; 
	long int 			    onu_id;
}PASLOG_configuration_t;

/* Set loggging general configuration
**
** Change the PASLOG genral configuration. The general configuration determine what contents
** the header of each LOG message. It can contain each possibility from module name, file name
** and function name. The default value for all parameters is DISABLE.
**
**/

/* Set paslog configuration
**
** Change the current logging configuration of the new paslog mecanisim. The configuration is bult up
** with 4 parameters: log_flag(priority), log_functionality, olt_id and onu_id and the command set the 
** behavior of each configuration: enable/disable and the output destination (stdout,file, syslog daemon etc)
**
**
** Input Parameters:
**				log_flag			: Log flag (priority) to be set. 
**									  Values: enum values, Ignore flag.
**				log_functionality	: Log functionality (contents of message) to be set
**									  Values: enum values, Ignore functionality.	
**			    olt_id				: OLT id, Values: Ignore OLT id.
**			    onu_id				: ONU id, Values: Ignore OLT id.
**				destination			: The destination of the log for the specific configuratoin
**				dest_parameter		: A parameter which is needed for some of the destination
**				
** Return codes:
**				All the defined PAS_* return codes
**
*/
extern PON_STATUS PAS_test_paslog_configuration
                                      (PASLOG_configuration_t   configuration,
									   bool				        activate,
									   PASLOG_destination_t     destination,
									   void					   *dest_parameter);

extern PON_STATUS PAS_set_paslog_configuration
                                      (PASLOG_configuration_t   configuration,
									   bool				        activate,
									   PASLOG_destination_t     destination,
									   void					   *dest_parameter);


/* Get paslog configuration
**
** Get the current destination configuration of the specific paslog configuration.
**
** Input Parameters:
**				log_flag			: Log flag (priority) to be set. 
**									  Values: enum values, Ignore flag.
**				log_functionality	: Log functionality (contents of message) to be set
**									  Values: enum values, Ignore functionality.	
**			    olt_id				: OLT id, Values: Ignore OLT id.
**			    onu_id				: ONU id, Values: Ignore OLT id.
**
				
** Return codes:
**				All the defined PAS_* return codes
**
*/

extern PON_STATUS PAS_get_paslog_configuration
                                      (PASLOG_configuration_t   configuration,
									   PASLOG_destination_t	   *destination,
									   void				       *dest_parameter);

#define PASCOMM_MAX_CONNECTION_TEST_BUFFER_SIZE 1000
#define PASCOMM_MSG_CONNECTION_TEST_OPCODE	0x48
typedef struct 
{
	unsigned char    char1;
	unsigned char    char2;
	unsigned short   short1;
	unsigned short   short2;
	unsigned long    long1;
	unsigned long    long2;
	unsigned long    long3;
	unsigned long    long4;
	mac_address_t	 mac_address;
	unsigned__int64	 double1;
	long double		 double2;
	unsigned short   buffer_size;
	unsigned char 	 buffer[PASCOMM_MAX_CONNECTION_TEST_BUFFER_SIZE];
} PASCOMM_msg_connection_test_t;

#define PASCOMM_MSG_CONNECTION_RESPONSE_OPCODE	0x48
#define PASCOMM_msg_connection_response_t PASCOMM_msg_connection_test_t

/* Get OLT parameters										  */
/* Physical representation: Opcode, direction: to PASCOMM	  */
#define PASCOMM_MSG_GET_OLT_PARAMS_OPCODE		0x4c

extern int PON_unsigned__int64_2_long_double ( long double *dst, const unsigned__int64 *src);

/* Send msg
**
** This function sends msg to PON communication module and returns with an answer / error
**
** Parameters:
**	  			olt_id					 - OLT index
**			    msg_type				 - The type of the sent msg
**				msg_ptr					 - msg to be sent, allocated by the caller
**			    msg_response_type		 - The type of the response msg
**				msg_response_ptr		 - Response msg, allocated by the caller
**				wait_for_remote_response - If a remote msg is sent (msg destined to an ONU), should the function wait 
**										   for the remote device response msg or just for the send acknowledge. 
**										   Irrelevant for OLT msgs. Values: TRUE, FALSE
**
** Return values:
**				0     : no error
**				other : error codes 
*/
extern short int Send_msg ( const PON_olt_id_t	   olt_id, 
					 const PASCOMM_msg_t   msg_type,  
					 const void			  *msg_ptr, 
					 PASCOMM_msg_t		  *msg_response_type, 
					 void			      *msg_response_ptr,
					 const bool			   wait_for_remote_response );

#ifdef PAS_SOFT_VERSION_V5_3_9

#if 0	/* 20100511 */
#define CTC_2_1_ONU_VERSION               0x21

#define	CTC_STACK_ALL_PORTS		0xFF

/* CTC 2.1 */
#define CTC_MIN_MANAGEMENT_OBJECT_FRAME_NUMBER	0
#define CTC_MAX_MANAGEMENT_OBJECT_FRAME_NUMBER	3

#define CTC_MIN_MANAGEMENT_OBJECT_SLOT_NUMBER	0
#define CTC_MAX_MANAGEMENT_OBJECT_SLOT_NUMBER	62

#define CTC_MANAGEMENT_OBJECT_ALL_SLOTS			63

#define	CTC_MANAGEMENT_OBJECT_PON_PORT_NUMBER		0x0

#define CTC_MIN_MANAGEMENT_OBJECT_ETHERNET_PORT_NUMBER 0x1
#define CTC_MAX_MANAGEMENT_OBJECT_ETHERNET_PORT_NUMBER 0x4F

#define CTC_MIN_MANAGEMENT_OBJECT_VOIP_PORT_NUMBER 0x50
#define CTC_MAX_MANAGEMENT_OBJECT_VOIP_PORT_NUMBER 0x8F

#define CTC_MIN_MANAGEMENT_OBJECT_E1_PORT_NUMBER 0x90
#define CTC_MAX_MANAGEMENT_OBJECT_E1_PORT_NUMBER 0x9F

#define	CTC_MANAGEMENT_OBJECT_ALL_PORTS		0xFFFF

typedef struct
{                                                                                                                   
	PON_binary_source_t			  source;	/* Type of device where the binary currently resides	                */
	void					     *location;	/* Location of the binary, different type for different sources:        */
											/* MEMORY source - Pointer to the beginning of the binary               */
											/* FILE   source - The name of the file containing the binary           */
                                            /* FLASH  source - Not required	                                        */
	long int					  size;		/* Size of the binary, stated in bytes. Required only for MEMORY source */
} CTC_STACK_binary_t;


#define CTC_AUTH_ONU_ID_SIZE            24
#define CTC_AUTH_PASSWORD_SIZE          12

#define CTC_AUTH_REQUEST_DATA_SIZE      0x01 
#define CTC_AUTH_RESPONSE_DATA_SIZE     0x25 
#define CTC_AUTH_NAK_RESPONSE_DATA_SIZE 0x02
#define CTC_AUTH_SUCCESS_DATA_SIZE      0x00 
#define CTC_AUTH_FAILURE_DATA_SIZE      0x01 
#define CTC_AUTH_LOID_DATA_NOT_USED     "NOT_USED" 

typedef enum
{
    CTC_AUTH_TYPE_LOID                = 0x01,   /* ONU_ID+Password mode */
    CTC_AUTH_TYPE_NOT_SUPPORTED       = 0x02    /* ONU not supported or can not accept the required authentication type */
}CTC_STACK_auth_type_t;

typedef enum
{
    CTC_AUTH_FAILURE_ONU_ID_NOT_EXIST = 0x01,   /* ONU_ID is not exist */
    CTC_AUTH_FAILURE_WRONG_PASSWORD   = 0x02    /* ONU_ID is exist but the Password is wrong */
}CTC_STACK_auth_failure_type_t;

typedef enum
{
    CTC_AUTH_MODE_MAC,               /* MAC mode */
    CTC_AUTH_MODE_LOID,              /* ONU_ID+Password mode */
    CTC_AUTH_MODE_HYBRID             /* hybrid mode */
}CTC_STACK_auth_mode_t;

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
	CTC_MANAGEMENT_OBJECT_LEAF_FRAME	= 0x0003,
	CTC_MANAGEMENT_OBJECT_LEAF_LLID		= 0x0004,
	CTC_MANAGEMENT_OBJECT_LEAF_PON_IF	= 0x0005,

	CTC_MANAGEMENT_OBJECT_LEAF_LAST,
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
    POWERING_VOLT_LOW	             = 0x0008,
    ONU_TEMP_HIGH_ALARM		         = 0x0009, 
    ONU_TEMP_LOW_ALARM		         = 0x000A,
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
    FRAME_FAILURE 			         = 0x0301,
    CARD_ALARM    			         = 0x0401,
    SELF_TEST_FAILURE		         = 0x0402,
    ETH_PORT_AUTO_NEG_FAILURE        = 0x0501,
    ETH_PORT_LOS   			         = 0x0502,
    ETH_PORT_CONNECTION_FAILURE      = 0x0503,
    ETH_PORT_LOOPBACK		         = 0x0504,
    ETH_PORT_CONGESTION		         = 0x0505,
    POTS_PORT_FAILURE		         = 0x0601,
    ADSL2_PORT_FAILURE		         = 0x0701,
    E1_PORT_FAILURE			         = 0x0801,
    E1_TIMING_UNLOCK			     = 0x0802,
    E1_LOS           		         = 0x0803,

} CTC_STACK_alarm_id_t;

typedef enum
{
    REPORT_ALARM                     = 0x00,
    CLEAR_REPORT_ALARM               = 0x01,
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
    PON_llid_t                                  llid;
	unsigned char								number_of_llid_queue_config_entries; /* 1..8 */
	CTC_STACK_llid_queue_config_entry_t			llid_queue_config_entries[CTC_MAX_LLID_QUEUE_CONFIG_ENTRIES];
} CTC_STACK_llid_queue_config_t;



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

typedef struct
{
	unsigned char						 port_number;
	CTC_STACK_tag_oper_t				 tag_oper;
    CTC_STACK_multicast_vlan_switching_t multicast_vlan_switching;
} CTC_STACK_multicast_port_tag_oper_t;

typedef  CTC_STACK_multicast_port_tag_oper_t CTC_STACK_multicast_ports_tag_oper_t[MAX_PORT_ENTRIES_NUMBER];

/* Set multicast Tag oper
**
** This function configures if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN

** Input Parameters:
**		olt_id	                 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			     : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**		tag_oper			     : Disable / Enable tag strip or translate tag 
**      multicast_vlan_switching : See CTC_STACK_multicast_vlan_switching_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_multicast_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const unsigned char	                       port_number,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);



/* Get multicast Tag oper
**
** This function retrieves if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN
**
** Input Parameters:
**		olt_id	                 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	                 : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number			     : port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**
** Output Parameters:
**		tag_oper			     : Disable / Enable tag strip or translate tag  
**      multicast_vlan_switching : See CTC_STACK_multicast_vlan_switching_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_multicast_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const unsigned char	                  port_number,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching);


/* Get multicast Tag oper for all ports
**
** This function retrieves if ONU should strip or translate the multicast VLAN TAG of multicast data and
** query frames of downstream multicast flow to the user IPTV VLAN
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries	: Number of valid entries in ports_info
**		ports_info			: See CTC_STACK_multicast_ports_tag_oper_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_multicast_all_port_tag_oper ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_oper_t    ports_info );
#endif

#ifdef SYS_DUMP
/* Sys Dump */
#define PON_SYS_DUMP_ALL	-1	/* Dump all devices			*/
#define PON_SYS_DUMP_NONE	-2	/* Don't dump any device	*/

/* Sys Dump type */
typedef enum
{
    PON_SYS_DUMP_GENERAL_CONFIG		= 0x01,
    PON_SYS_DUMP_OLT_CONFIG			= 0x02,
	PON_SYS_DUMP_ONU_CONFIG			= 0x04
}PON_sys_dump_type_t;

/* Sys Dump
**
** Dump system configuration
**
** Input Parameters:
**		dump_type		: The dump type. The user can select more than one dump type by using OR operator ('|'), 
**							see PON_sys_dump_type_t for details. 
**		olt_id			: OLT to dump, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID,
**							or PON_SYS_DUMP_ALL for all OLTs
**							or PON_SYS_DUMP_NONE for no OLT
**		onu_id			: ONU to dump, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**							or PON_SYS_DUMP_ALL for all ONUs
**							or PON_SYS_DUMP_NONE for no ONU
**
** 
** Return codes:
**			All the defined			PAS_* return codes
*/
extern PON_STATUS PAS_test_sys_dump(const PON_sys_dump_type_t	dump_type, 
							PON_olt_id_t				olt_id, 
							PON_onu_id_t				onu_id);

extern PON_STATUS PAS_sys_dump(const PON_sys_dump_type_t		dump_type, 
						PON_olt_id_t					olt_id, 
						PON_onu_id_t					onu_id);

#endif /* SYS_DUMP */

typedef enum
{
    PON_OAM_FRAME_RATE_10_FRAMES        = 0,
    PON_OAM_FRAME_RATE_100_FRAMES       = 1,
    PON_OAM_FRAME_RATE_NO_LIMITATION    = 2

} PON_oam_frame_rate_t;

/* Set oam frame rate
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid             : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**      frame_rate       : see PON_oam_frame_rate_t for details
**      
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_test_oam_frame_rate
                                    (const PON_olt_id_t          olt_id,
                                     const PON_llid_t            llid,     
                                     const PON_oam_frame_rate_t  frame_rate);
extern PON_STATUS PAS_set_oam_frame_rate
                                    (const PON_olt_id_t          olt_id,
                                     const PON_llid_t            llid,
                                     const PON_oam_frame_rate_t  frame_rate);

/* Get oam frame rate
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      llid             : LLID, Range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**
** Output Parameters:
**      frame_rate       : see PON_oam_frame_rate_t for details
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
extern PON_STATUS PAS_get_oam_frame_rate
                                   (const PON_olt_id_t          olt_id,
                                    const PON_llid_t            llid,
                                    PON_oam_frame_rate_t       *frame_rate);


/* OLT host recovery
**
** (the only) Mandatory API command to be used in a case of PAS5001 host recovery from OLT line card.
** This function performs the PAS5001 host recovery and enables further query functions detailed in this file.
**
** Input Parameter:
**		olt_id						 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**		olt_mode					 : OLT mode. Valid values: enum values. 
**									   Only PON_OLT_MODE_CONFIGURED_AND_ACTIVATED value indicates a successful
**									   completion of the recovery process.
**		initialization_configuration : OLT Initialization parameters struct, note: firmware_image parameter is zeroed
**		dba_mode					 : DBA mode, Valid values: PON_PROVISIONING_MODE_STATIC_GRANTING, 
**									   PON_PROVISIONING_MODE_INTERNAL_DBA, PON_PROVISIONING_MODE_EXTERNAL_DBA 
**		link_test					 : LLID that is being tested in active link test. 
**									   Valid values: PON_OLT_NOT_IN_LINK_TEST, PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT 
**		active_llids				 : List of active LLIDs. Each LLID record includes the LLID index and its MAC address.
**
** Return codes:
**		All the defined PAS_RECOVERY_* return codes
*/
PON_STATUS PAS_test_olt_host_recovery 
                                   ( const PON_olt_id_t		              olt_id,
									 PON_olt_mode_t			              *olt_mode,
									 PON_olt_initialization_parameters_t  *initialization_configuration, 
									 unsigned short int		              *dba_mode, 
									 PON_llid_t				              *link_test,  
									 PON_active_llids_t		              *active_llids );

PON_STATUS PAS_olt_host_recovery 
                                   ( const PON_olt_id_t		              olt_id,
									 PON_olt_mode_t			              *olt_mode,
									 PON_olt_initialization_parameters_t  *initialization_configuration, 
									 unsigned short int		              *dba_mode, 
									 PON_llid_t				              *link_test,
									 PON_active_llids_t		              *active_llids );



typedef enum
{  
	PON_AUTH_MODE_DISABLE_MAC_LIST = 0,
	PON_AUTH_MODE_ENABLE_MAC_LIST  = 1,
	PON_AUTH_MODE_HYBRID           = 2
} PON_authentication_mode_t;

typedef struct
{
    BOOLEAN  fwd_unknown_uc;
    BOOLEAN  fwd_unknown_mc;
    BOOLEAN  fwd_unknown_bc;
}SWAL_ADDTBLE_unknown_da_fwd_mode_t;

/*******************************************************************************************
  Description   : Set forwarding mode for unknown DA frames.
  In params     : 
                  olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
                  onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
                  lport - logical lport number
				  mode  - forwarding mode for unknown DA frames
  Out params    : N/A
  Return value  : standard PASONU API error codes
********************************************************************************************/
extern PON_STATUS REMOTE_PASONU_swal_addrtbl_set_unknown_da_fwd_mode (
                    const PON_olt_id_t                         olt_id, 
                    const PON_onu_id_t                         onu_id,
					const INT8U                                lport,
                    const SWAL_ADDTBLE_unknown_da_fwd_mode_t   mode);

/*******************************************************************************************
  Description   : Get forwarding mode for unknown DA frames.
  In params     : 
                  olt_id	    : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
                  onu_id	    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
                  lport - logical lport number
  Out params    : mode  - pointer to forwarding mode for unknown DA frames
  Return value  : standard PASONU API error codes
********************************************************************************************/
extern PON_STATUS REMOTE_PASONU_swal_addrtbl_get_unknown_da_fwd_mode (
                    const PON_olt_id_t                         olt_id, 
                    const PON_onu_id_t                         onu_id,
					const INT8U                                lport,
                    SWAL_ADDTBLE_unknown_da_fwd_mode_t         *mode);

#if 0	/* 20100511 */
/* Update onu firmware
**
** This function updates onu firmware.
**
** Input Parameters:
**		olt_id	              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		onu_firmware	      : See CTC_STACK_binary_t for details
**      file_name             : string of Vendor.ONU type.firmware version.date
**      standby_start_storage : TRUE  - the default loaded software image when ONU start in future. 
**                              FALSE - restart the software, load and run the software image now.
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/ 
extern PON_STATUS CTC_STACK_update_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const char		            *file_name,
                 const bool                  standby_start_storage );

        
/* Swap onu firmware
**
** This function swaps onu firmware.
**
** Input Parameters:
**		olt_id	              : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				  : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/ 
extern PON_STATUS CTC_STACK_swap_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id);


/* Get auth LOID data table
**
** Get authentication ONU_ID+Password data table
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**          num_of_entries      : num of entries in auth LOID table
**
** Output Parameters:
**			loid_data_table		: See CTC_STACK_auth_loid_data_t for details.
**          num_of_entries      : num of entries in auth LOID table
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_auth_loid_data_table(           
					const PON_olt_id_t			         olt_id, 
					CTC_STACK_auth_loid_data_t          *loid_data_table,
                    unsigned char                       *num_of_entries);


/* Add auth LOID data 
**
** Add authentication ONU_ID+Password data
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			loid_data		    : See CTC_STACK_auth_loid_data_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_add_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);


/* Remove auth LOID data 
**
** Remove authentication ONU_ID+Password data
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			loid_data		    : See CTC_STACK_auth_loid_data_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_remove_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);


/* Set auth mode 
**
** Set authentication mode
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			ctc_auth_mode       : See CTC_STACK_auth_mode_t for details.
**
** Output Parameters:
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_mode_t          ctc_auth_mode);


/* Get auth mode 
**
** Get authentication mode
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**			ctc_auth_mode       : See CTC_STACK_auth_mode_t for details.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					CTC_STACK_auth_mode_t               *ctc_auth_mode);

/* Get optical transceiver diagnosis
**
** This function retrieves ONU optical transceiver diagnosis
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		optical_transceiver_diagnosis	: See CTC_STACK_optical_transceiver_diagnosis_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_optical_transceiver_diagnosis ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );



/* Set service sla
**
** This function sets ONU service SLA
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		activate				: Activate/Deactivate service SLA.
**		service_sla				: See CTC_STACK_service_sla_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const bool									activate,
		   const CTC_STACK_service_sla_t				*service_sla );

		   
/* Get service sla
**
** This function retrieves ONU service SLA
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		activate				: Is service SLA activated or deactivated.
**		service_sla				: See CTC_STACK_service_sla_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_service_sla ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   bool											*activate,
		   CTC_STACK_service_sla_t						*service_sla );


/* Set alarm admin state
**
** This function sets ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		enable					: Enable/Disable alarm.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t   			alarm_id,
			const bool								enable);


/* Get alarm admin state
**
** This function retrieves ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		number_of_alarms		: Size of alarms_state array
**
** Output Parameters:
**		number_of_alarms		: Actual number of elements in alarms_state array
**		alarms_state			: Array of CTC_STACK_alarm_admin_state_t
**									See CTC_STACK_alarm_admin_state_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
typedef struct 
{			
	CTC_management_object_index_t	management_object_index;
	CTC_STACK_alarm_id_t			alarm_id;
	bool							enable;
}CTC_STACK_alarm_admin_state_t;

extern PON_STATUS CTC_STACK_get_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_admin_state_t			alarms_state[] );


/* Set alarm threshold
**
** This function sets ONU alarm threshold
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details 
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		rising_threshold		: Alarm rising threshold
**		falling_threshold		: Alarm falling threshold
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const unsigned long						rising_threshold,
			const unsigned long						falling_threshold );


/* Get alarm threshold
**
** This function retrieves ONU alarm admin state
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		management_object_index	: See CTC_management_object_index_t for details
**		alarm_id				: See CTC_STACK_alarm_id_t for details 
**		number_of_alarms		: Size of alarms_threshold array
**
** Output Parameters:
**		number_of_alarms		: Actual number of elements in alarms_threshold array
**		alarms_state			: Array of CTC_STACK_alarm_threshold_t
**									See CTC_STACK_alarm_threshold_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
typedef struct 
{			
	CTC_management_object_index_t	management_object_index;
	CTC_STACK_alarm_id_t			alarm_id;
	unsigned long					rising_threshold;
	unsigned long					falling_threshold;
}CTC_STACK_alarm_threshold_t;

extern PON_STATUS CTC_STACK_get_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			unsigned short							*number_of_alarms,
			CTC_STACK_alarm_threshold_t				alarms_threshold[] );

			
/* Reset card
**
** This function reset a specific card.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		farme_number		: Card frame number
**		slot_number		: Slot frame number
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_reset_card ( 
			const PON_olt_id_t					olt_id, 
			const PON_onu_id_t					onu_id,
			unsigned char						frame_number,
			unsigned char						slot_number );


/* Get onu version
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id	            : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		onu_version         : ONU version.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_onu_version(
                                    const PON_olt_id_t		   olt_id, 
                                    const PON_onu_id_t		   onu_id,
                                    unsigned char             *onu_version );


/* Set llid queue config
**
** This function sets llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		llid_queue_config   	: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_llid_queue_config_t		    llid_queue_config );

		   
/* Get llid queue config
**
** This function retrieves llid queue config
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		llid_queue_config		: See CTC_STACK_llid_queue_config_t for details 
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_llid_queue_config ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_llid_queue_config_t			   *llid_queue_config );


/* Set multi llid admin control
**
** This function set multi llid admin control
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		num_of_llid_activated	: number of llid activated
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_set_multi_llid_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const unsigned long                    num_of_llid_activated);


/* Authentication response ONU_ID+Password handler
**
** An event of ONU authentication response ONU_ID+Password.
** OLT needs to confirm ONU loid_data.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          loid_data           : See CTC_STACK_auth_loid_data_t for details 
**
** Output Parameters:
**		    auth_success     	: true/false - authentication succeed/failed   
**		    failure_type     	: if authentication failed, see CTC_STACK_auth_failure_type_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_auth_response_loid_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
                    const CTC_STACK_auth_loid_data_t    loid_data,
                    bool                               *auth_success,
					CTC_STACK_auth_failure_type_t      *failure_type );


/* Authorized ONU_ID+Password handler
**
** An event of OLT authorized ONU_ID+Password, meaning if ONU loid_data was confirmed.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          loid_data           : See CTC_STACK_auth_loid_data_t for details 
**		    auth_success     	: true/false - authentication succeed/failed   
**		    failure_type     	: if authentication failed, see CTC_STACK_auth_failure_type_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_authorized_loid_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
                    const CTC_STACK_auth_loid_data_t    loid_data,
                    const bool                          auth_success,
					const CTC_STACK_auth_failure_type_t failure_type );


/* Event Notification handler
**
** An event of Organization Specific Event Notification.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**	    	onu_id			    : ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**          event_value         : See CTC_STACK_event_value_t for details 
**
** Return codes:
**				Return codes are not specified for an event
**
*/

typedef void (*CTC_STACK_event_notification_handler_t) 
				  ( const PON_olt_id_t					olt_id, 
					const PON_onu_id_t					onu_id, 
					const CTC_STACK_event_value_t       event_value );


#endif	/* end 20100511 */


#endif   /* PAS_SOST_VERSION_V5_3_9 */

typedef struct
{
	unsigned long	msb;
	unsigned long	lsb;
} INT64U;
typedef unsigned long OSSRV_semaphore_t;


/*#include "PLATO3_API_expo.h"*/
#include "PON_CTC_STACK_expo.h"


#ifdef  PAS_SOFT_VERSION_V5_3_5
PON_STATUS CTC_STACK_get_fec_ability ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   CTC_STACK_standard_FEC_ability_t  *fec_ability );

PON_STATUS CTC_STACK_set_fec_mode ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   const CTC_STACK_standard_FEC_mode_t  fec_mode);

PON_STATUS CTC_STACK_reset_onu ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id );

PON_STATUS CTC_STACK_start_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid );

PON_STATUS CTC_STACK_stop_encryption ( 
                                   const PON_olt_id_t   olt_id, 
                                   const PON_llid_t     llid );

PON_STATUS CTC_STACK_get_phy_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS CTC_STACK_set_phy_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     state);

PON_STATUS CTC_STACK_get_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *flow_control_enable);

PON_STATUS CTC_STACK_set_ethernet_port_pause ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool			 flow_control_enable);

PON_STATUS CTC_STACK_get_auto_negotiation_admin_state ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   bool				    *state );

PON_STATUS CTC_STACK_set_auto_negotiation_admin_control ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const bool			 state);

PON_STATUS CTC_STACK_set_auto_negotiation_restart_auto_config ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number);

PON_STATUS CTC_STACK_get_ethernet_link_state ( 
           const PON_olt_id_t	    olt_id, 
           const PON_onu_id_t	    onu_id,
		   const unsigned char	    port_number,
		   CTC_STACK_link_state_t  *link_state);

PON_STATUS CTC_STACK_get_ethernet_port_policing ( 
           const PON_olt_id_t						  olt_id, 
           const PON_onu_id_t						  onu_id,
		   const unsigned char						  port_number,
		   CTC_STACK_ethernet_port_policing_entry_t  *port_policing);

PON_STATUS CTC_STACK_set_ethernet_port_policing ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   const CTC_STACK_ethernet_port_policing_entry_t   port_policing);

PON_STATUS CTC_STACK_get_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t								olt_id, 
           const PON_onu_id_t								onu_id,
		   const unsigned char								port_number,
		   CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting );

PON_STATUS CTC_STACK_set_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t										olt_id, 
           const PON_onu_id_t										onu_id,
		   const unsigned char										port_number,
		   const CTC_STACK_ethernet_port_ds_rate_limiting_entry_t	*port_ds_rate_limiting);

PON_STATUS CTC_STACK_get_vlan_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_vlan_configuration_t  *port_configuration);

PON_STATUS CTC_STACK_set_vlan_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_vlan_configuration_t   port_configuration);

PON_STATUS CTC_STACK_get_classification_and_marking ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   CTC_STACK_classification_rules_t   classification_and_marking);

PON_STATUS CTC_STACK_set_classification_and_marking ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const unsigned char							port_number,
		   const CTC_STACK_classification_rule_mode_t   mode,
		   const CTC_STACK_classification_rules_t		classification_and_marking);

PON_STATUS CTC_STACK_delete_classification_and_marking_list ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t    onu_id,
		   const unsigned char   port_number);

PON_STATUS CTC_STACK_get_multicast_vlan ( 
           const PON_olt_id_t			olt_id, 
           const PON_onu_id_t			onu_id,
		   const unsigned char			port_number,
		   CTC_STACK_multicast_vlan_t  *multicast_vlan);

PON_STATUS CTC_STACK_set_multicast_vlan ( 
           const PON_olt_id_t				  olt_id, 
           const PON_onu_id_t				  onu_id,
		   const unsigned char				  port_number,
		   const CTC_STACK_multicast_vlan_t   multicast_vlan);

PON_STATUS CTC_STACK_clear_multicast_vlan ( 
           const PON_olt_id_t    olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number );

PON_STATUS CTC_STACK_set_multicast_control ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const CTC_STACK_multicast_control_t   multicast_control );

PON_STATUS CTC_STACK_clear_multicast_control ( 
           const PON_olt_id_t   olt_id, 
           const PON_onu_id_t	onu_id );

PON_STATUS CTC_STACK_get_multicast_control ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_control_t	*multicast_control );

PON_STATUS CTC_STACK_get_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   unsigned char	    *group_num);

PON_STATUS CTC_STACK_set_multicast_group_num ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char   port_number,
		   const unsigned char	 group_num);

PON_STATUS CTC_STACK_get_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   bool				    *tag_strip);

PON_STATUS CTC_STACK_set_multicast_tag_strip ( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const bool		     tag_strip);

PON_STATUS CTC_STACK_get_multicast_switch ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   CTC_STACK_multicast_protocol_t  *multicast_protocol );

PON_STATUS CTC_STACK_set_multicast_switch ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   const CTC_STACK_multicast_protocol_t   multicast_protocol );

PON_STATUS CTC_STACK_get_fast_leave_ability ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_ability_t		*fast_leave_ability );

PON_STATUS CTC_STACK_get_fast_leave_admin_state ( 
           const PON_olt_id_t					olt_id, 
           const PON_onu_id_t					onu_id,
		   CTC_STACK_fast_leave_admin_state_t	*fast_leave_admin_state );

PON_STATUS CTC_STACK_set_fast_leave_admin_control ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   CTC_STACK_fast_leave_admin_state_t     fast_leave_admin_state );

PON_STATUS CTC_STACK_get_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_vlan_configuration_ports_t   ports_info );


PON_STATUS CTC_STACK_set_qinq_port_configuration ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const unsigned char						   port_number,
		   const CTC_STACK_port_qinq_configuration_t   port_configuration);

PON_STATUS CTC_STACK_set_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_holdover_state_t  parameter);

PON_STATUS CTC_STACK_get_holdover_state ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_holdover_state_t*  parameter);

PON_STATUS CTC_STACK_set_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_global_parameter_config_t  parameter);

PON_STATUS CTC_STACK_get_onu_version(
                                    const PON_olt_id_t		   olt_id, 
                                    const PON_onu_id_t		   onu_id,
                                    unsigned char             *onu_version );

PON_STATUS CTC_STACK_onu_tx_power_supply_control ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
           const CTC_STACK_onu_tx_power_supply_control_t parameter,
           const bool               broadcast);

PON_STATUS CTC_STACK_get_onu_serial_number ( 
           const PON_olt_id_t			   olt_id, 
           const PON_onu_id_t			   onu_id,
		   CTC_STACK_onu_serial_number_t  *onu_serial_number );

PON_STATUS CTC_STACK_init 
                ( const bool							 automatic_mode,
                  const unsigned char					 number_of_records,
                  const CTC_STACK_oui_version_record_t  *oui_version_records_list );

PON_STATUS CTC_STACK_get_ethport_statistic_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_data_t *data);

PON_STATUS CTC_STACK_get_ethport_statistic_state (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_state_t *state);

PON_STATUS CTC_STACK_get_auto_negotiation_local_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS CTC_STACK_get_auto_negotiation_advertised_technology_ability ( 
           const PON_olt_id_t								 olt_id, 
           const PON_onu_id_t								 onu_id,
		   const unsigned char								 port_number,
		   CTC_STACK_auto_negotiation_technology_ability_t  *abilities );

PON_STATUS CTC_STACK_get_multicast_tag_oper ( 
           const PON_olt_id_t	                  olt_id, 
           const PON_onu_id_t	                  onu_id,
		   const unsigned char	                  port_number,
		   CTC_STACK_tag_oper_t                  *tag_oper,
		   CTC_STACK_multicast_vlan_switching_t  *multicast_vlan_switching);

PON_STATUS CTC_STACK_set_multicast_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const unsigned char	                       port_number,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);

PON_STATUS CTC_STACK_get_mxu_mng_global_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_global_parameter_config_t*  parameter);

PON_STATUS CTC_STACK_get_extended_oam_discovery_timing 
                ( unsigned short int  *discovery_timeout );

PON_STATUS CTC_STACK_set_extended_oam_discovery_timing 
                ( unsigned short int  discovery_timeout );

PON_STATUS CTC_STACK_get_encryption_timing 
                ( unsigned char       *update_key_time,
                  unsigned short int  *no_reply_timeout );

PON_STATUS CTC_STACK_set_encryption_timing 
                ( const unsigned char       update_key_time,
                  const unsigned short int  no_reply_timeout );

PON_STATUS CTC_STACK_set_auth_mode(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_mode_t          ctc_auth_mode);

PON_STATUS CTC_STACK_set_alarm_admin_state ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const bool								enable);

PON_STATUS CTC_STACK_set_ethport_statistic_state( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   const CTC_STACK_statistic_state_t state);

PON_STATUS CTC_STACK_set_ethport_statistic_data( 
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number
);

PON_STATUS CTC_STACK_get_ethport_statistic_history_data (
           const PON_olt_id_t	 olt_id, 
           const PON_onu_id_t	 onu_id,
		   const unsigned char	 port_number,
		   CTC_STACK_statistic_data_t *data);

PON_STATUS CTC_STACK_is_init ( bool *init );

PON_STATUS CTC_STACK_assign_handler_function
                ( const CTC_STACK_handler_index_t    handler_function_index, 
				  const void					    (*handler_function)(),
				  unsigned short                     *handler_id );

PON_STATUS CTC_STACK_get_encryption_timing_threshold 
                ( unsigned char   *start_encryption_threshold);

PON_STATUS CTC_STACK_set_encryption_timing_threshold 
                ( const unsigned char   start_encryption_threshold);

PON_STATUS CTC_STACK_get_chipset_id ( 
           const PON_olt_id_t		olt_id, 
           const PON_onu_id_t		onu_id,
		   CTC_STACK_chipset_id_t  *chipset_id );

PON_STATUS CTC_STACK_get_firmware_version ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           unsigned short      *version );

PON_STATUS CTC_STACK_get_onu_capabilities ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_t  *onu_capabilities );

PON_STATUS CTC_STACK_get_onu_capabilities_2 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_2_t  *onu_capabilities );

PON_STATUS CTC_STACK_get_onu_capabilities_3 ( 
           const PON_olt_id_t			  olt_id, 
           const PON_onu_id_t			  onu_id,
		   CTC_STACK_onu_capabilities_3_t  *onu_capabilities );

PON_STATUS CTC_STACK_get_multicast_all_port_tag_strip ( 
           const PON_olt_id_t					   olt_id, 
           const PON_onu_id_t					   onu_id,
		   unsigned char						  *number_of_entries,
		   CTC_STACK_multicast_ports_tag_strip_t   ports_info );

short int CTC_STACK_auth_request ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   CTC_STACK_auth_response_t         *auth_response);

short int CTC_STACK_auth_success ( 
                                         const PON_olt_id_t                 olt_id, 
                                         const PON_onu_id_t                 onu_id);

short int CTC_STACK_auth_failure ( 
                                         const PON_olt_id_t                  olt_id, 
                                         const PON_onu_id_t                  onu_id,
                                         const CTC_STACK_auth_failure_type_t failure_type );

authentication_olt_database_t * CTC_STACK_get_auth_loid_database( const PON_olt_id_t olt_id );

PON_STATUS CTC_STACK_set_alarm_threshold ( 
			const PON_olt_id_t						olt_id, 
			const PON_onu_id_t						onu_id,
			const CTC_management_object_index_t		management_object_index,
			const CTC_STACK_alarm_id_t				alarm_id,
			const unsigned long						alarm_threshold,
			const unsigned long						clear_threshold );

PON_STATUS CTC_STACK_set_multicast_management_object_tag_oper ( 
           const PON_olt_id_t	                       olt_id, 
           const PON_onu_id_t	                       onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_tag_oper_t                  tag_oper,
           const CTC_STACK_multicast_vlan_switching_t  multicast_vlan_switching);

PON_STATUS CTC_STACK_get_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
		    CTC_STACK_mxu_mng_snmp_parameter_config_t*  parameter);

PON_STATUS CTC_STACK_set_mxu_mng_snmp_parameter_config ( 
			const PON_olt_id_t						           olt_id, 
			const PON_onu_id_t						           onu_id,
			const CTC_STACK_mxu_mng_snmp_parameter_config_t  parameter);

PON_STATUS CTC_STACK_get_oui 
                ( unsigned long  *oui );

PON_STATUS CTC_STACK_set_oui 
                ( const unsigned long  oui );

PON_STATUS CTC_STACK_get_version 
                ( unsigned char  *version );

PON_STATUS CTC_STACK_set_version 
                ( const unsigned char  version );

PON_STATUS CTC_STACK_update_onu_firmware ( 
                 const PON_olt_id_t          olt_id, 
                 const PON_onu_id_t          onu_id,
                 const CTC_STACK_binary_t   *onu_firmware,
                 const char		            *file_name);

short int CTC_STACK_activate_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);

short int CTC_STACK_commit_image(
            const PON_olt_id_t                    olt_id, 
            const PON_onu_id_t                    onu_id);

PON_STATUS CTC_STACK_set_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );

PON_STATUS CTC_STACK_get_dba_report_thresholds ( 
                                   const PON_olt_id_t					  olt_id, 
                                   const PON_onu_id_t					  onu_id,
                                   unsigned char 						 *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds );
#if 0
short int CTC_STACK_start_loid_authentication ( 
                                   const PON_olt_id_t                 olt_id, 
                                   const PON_onu_id_t                 onu_id,
                                   bool                              *auth_success);
#endif
PON_STATUS CTC_STACK_get_optical_transceiver_diagnosis ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis );

/* Get firmware version (extended)
**
** This function retrieves ONU firmware version.
**
** Input Parameters:
**		olt_id	            : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		version_buffer_size : Allocated buffer size, up to 127 bytes.
**
** Output Parameters:
**      version_buffer		: Firmware version.
**		version_buffer_size : Actual buffer size, up to 127 bytes.
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
extern PON_STATUS CTC_STACK_get_firmware_version_extended ( 
           const PON_olt_id_t	olt_id, 
           const PON_onu_id_t	onu_id,
           unsigned char		*version_buffer,
           unsigned char		*version_buffer_size);

		   

/* Get all ethernet port ds rate limiting
**
** This function retrieves all ONU ethernet ports downstream rate limiting configuration
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**		number_of_entries		: Number of valid entries in ports info parameter
**		ports_ds_rate_limiting	: See CTC_STACK_ethernet_ports_ds_rate_limiting_t for details
**
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/

extern PON_STATUS CTC_STACK_get_all_ethernet_port_ds_rate_limiting ( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   unsigned char								*number_of_entries,
		   CTC_STACK_ethernet_ports_ds_rate_limiting_t	ports_ds_rate_limiting );



/* Get QinQ VLAN
**
** This function gets Ethernet port QinQ VLAN configuration 
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**		port_number				: port number. CTC_MIN_ETHERNET_PORT_NUMBER - CTC_MAX_ETHERNET_PORT_NUMBER, CTC_STACK_ALL_PORTS
**
** Output Parameters:
**      port_configuration		: See CTC_STACK_port_qinq_configuration_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
extern PON_STATUS CTC_STACK_get_qinq_port_configuration ( 
           const PON_olt_id_t					 olt_id, 
           const PON_onu_id_t					 onu_id,
		   const unsigned char					 port_number,
		   CTC_STACK_port_qinq_configuration_t  *port_configuration);


/* Get QinQ VLAN for all ports
**
** This function gets Ethernet port QinQ VLAN configuration for all Ethernet ports of specified ONU
**
** Input Parameters:
**		olt_id					: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		onu_id					: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**      number_of_entries		: Number of valid entries in ports info 
**      ports_info				: See CTC_STACK_vlan_configuration_ports_t for details.
** Return codes:
**		All the defined CTC_STACK_* return codes
**
*/
extern PON_STATUS CTC_STACK_get_qinq_vlan_all_port_configuration ( 
           const PON_olt_id_t					  olt_id, 
           const PON_onu_id_t					  onu_id,
		   unsigned char						 *number_of_entries,
		   CTC_STACK_qinq_configuration_ports_t   ports_info );  


/* Get discovery state
**
** Get extended OAM discovery state of an ONU.
** The discovery process may succeed or fail.
**
** Input Parameters:
**			olt_id				: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		    onu_id				: ONU id, range: PON_MIN_ONU_ID_PER_OLT - PON_MAX_ONU_ID_PER_OLT
**
** Output Parameters:
**			state			    : One of CTC_STACK_discovery_state_t enum value.
**
** Return codes:
**				Return codes are not specified for an event
**
*/

extern PON_STATUS CTC_STACK_get_discovery_state(           
					const PON_olt_id_t			olt_id, 
					const PON_onu_id_t			onu_id,
					CTC_STACK_discovery_state_t	*state);



#if 1
PON_STATUS CTC_STACK_set_voip_port ( 
           const PON_olt_id_t				olt_id, 
           const PON_onu_id_t				onu_id,
		   const unsigned char				port_number,
		   const CTC_STACK_on_off_state_t   port_state );

PON_STATUS CTC_STACK_get_voip_port ( 
           const PON_olt_id_t		  olt_id, 
           const PON_onu_id_t		  onu_id,
		   const unsigned char		  port_number,
		   CTC_STACK_on_off_state_t  *port_state);

PON_STATUS CTC_STACK_get_voip_all_port ( 
           const PON_olt_id_t		 olt_id, 
           const PON_onu_id_t		 onu_id,
		   unsigned char			*number_of_entries,
		   CTC_STACK_ports_state_t   ports_info );

PON_STATUS CTC_STACK_set_voip_management_object ( 
           const PON_olt_id_t				    olt_id, 
           const PON_onu_id_t				    onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   const CTC_STACK_on_off_state_t       port_state );

PON_STATUS CTC_STACK_get_voip_management_object ( 
           const PON_olt_id_t		            olt_id, 
           const PON_onu_id_t		            onu_id,
		   const CTC_management_object_index_t  management_object_index,
		   CTC_STACK_on_off_state_t            *port_state);

PON_STATUS CTC_STACK_get_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_fax_config_t    	           *voip_fax);

PON_STATUS CTC_STACK_set_voip_fax_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_fax_config_t            voip_fax);

PON_STATUS CTC_STACK_voip_iad_operation( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_operation_type_t                operation_type);

PON_STATUS CTC_STACK_get_voip_iad_oper_status( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_oper_status_t	           *iad_oper_status);

PON_STATUS CTC_STACK_get_voip_global_param_conf ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			CTC_STACK_voip_global_param_conf_t         *global_param);

PON_STATUS CTC_STACK_set_voip_global_param_conf( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_voip_global_param_conf_t     global_param);

PON_STATUS CTC_STACK_get_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_param_config_t    	       *h248_param);

PON_STATUS CTC_STACK_set_h248_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_param_config_t          h248_param);

PON_STATUS CTC_STACK_set_h248_user_tid_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_h248_user_tid_config_t   user_tid_config);

PON_STATUS CTC_STACK_get_h248_user_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
           const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
		   CTC_STACK_h248_user_tid_config_array_t    	       *h248_user_tid_array);

PON_STATUS CTC_STACK_set_h248_rtp_tid_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_h248_rtp_tid_config_t       h248_rtp_tid_config);

PON_STATUS CTC_STACK_get_h248_rtp_tid_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_h248_rtp_tid_info_t    	       *h248_rtp_tid_info);

PON_STATUS CTC_STACK_get_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_sip_param_config_t    	           *sip_param);

PON_STATUS CTC_STACK_set_sip_param_config( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_sip_param_config_t           sip_param);

PON_STATUS CTC_STACK_get_sip_user_param_config(const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
           CTC_STACK_sip_user_param_config_array_t       *sip_user_param_array);


PON_STATUS CTC_STACK_set_sip_user_param_config ( 
           const PON_olt_id_t						   olt_id, 
           const PON_onu_id_t						   onu_id,
		   const CTC_management_object_index_t         management_object_index,
		   const CTC_STACK_sip_user_param_config_t   sip_user_param);

PON_STATUS CTC_STACK_set_sip_digit_map( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   const CTC_STACK_SIP_digit_map_t           sip_digit_map);

PON_STATUS CTC_STACK_get_voip_iad_info( 
           const PON_olt_id_t							olt_id, 
           const PON_onu_id_t							onu_id,
		   CTC_STACK_voip_iad_info_t	     	       *iad_info);

PON_STATUS CTC_STACK_get_voip_pots_status ( 
			const PON_olt_id_t						    olt_id, 
			const PON_onu_id_t						    onu_id,
			const CTC_management_object_index_t		    management_object_index,
     	    unsigned char					           *number_of_entries,
			CTC_STACK_voip_pots_status_array_t       *pots_status_array);
#endif

/* 以下为CTC redundancy相关*/
PON_STATUS CTC_STACK_get_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           CTC_STACK_redundancy_database_t             *redundancy_db);

PON_STATUS CTC_STACK_set_redundancy_database(           
		   const PON_olt_id_t			                olt_id,
		   const PON_onu_id_t			                onu_id,
           const CTC_STACK_redundancy_database_t        redundancy_db);

PON_STATUS ctc_redundancy_init (void);

#endif



/* B--added by liwei056@2010-1-25 for NewVersion5.3.11's redundancy */
#ifdef PAS_SOFT_VERSION_V5_3_11

/*----------------------------------------------------------*/
/*						Return codes						*/
/*----------------------------------------------------------*/
#define REDUNDANCY_MNG_EXIT_OK				EXIT_OK
#define REDUNDANCY_MNG_EXIT_ERROR			EXIT_ERROR
#define REDUNDANCY_MNG_TIME_OUT				TIME_OUT 
#define REDUNDANCY_MNG_NOT_IMPLEMENTED		NOT_IMPLEMENTED
#define REDUNDANCY_MNG_PARAMETER_ERROR		PARAMETER_ERROR
#define REDUNDANCY_MNG_HARDWARE_ERROR		HARDWARE_ERROR
#define REDUNDANCY_MNG_MEMORY_ERROR			MEMORY_ERROR
#define REDUNDANCY_MNG_OLT_NOT_EXIST		PAS_OLT_NOT_EXIST
#define REDUNDANCY_MNG_QUERY_FAILED			PAS_QUERY_FAILED
#define REDUNDANCY_MNG_ONU_NOT_AVAILABLE	PAS_ONU_NOT_AVAILABLE
#define	REDUNDANCY_MNG_ALREADY_EXIST		ENTRY_ALREADY_EXISTS
#define	REDUNDANCY_MNG_TABLE_FULL			TABLE_FULL
#define REDUNDANCY_MNG_OLT_WAS_RESETED   	OLT_WAS_RESETED
#define REDUNDANCY_MNG_NOT_ENABLE   	    -88


#define PON_REDUNDANCY_NO_GPIO	            0xFFFF

#define OLT_ID_REDUNDANCY_LIST_MAX_SIZE	  20
typedef PON_olt_id_t olt_id_list_t[OLT_ID_REDUNDANCY_LIST_MAX_SIZE];

#define GPIO_REDUNDANCY_LIST_MAX_SIZE	  20
typedef unsigned short gpio_list_t[GPIO_REDUNDANCY_LIST_MAX_SIZE];

#define PON_OAM_REMOTE_INFO_TLV_SIZE	    16
typedef unsigned char PON_oam_remote_info_tlv_t[PON_OAM_REMOTE_INFO_TLV_SIZE];

typedef struct
{
	OAM_standard_version_t		    oam_version;
	unsigned short                  oam_state;
	unsigned short                  oam_revision;
	unsigned short                  oam_local_flags;
	unsigned short                  oam_remote_flags;
	unsigned short                  oam_aging_count;
	unsigned short                  oam_link_retry;
	unsigned short                  oam_rtx;
	PON_oam_remote_info_tlv_t       oam_remote_info_tlv;

} PON_llid_oam_db_t;

typedef enum
{                                      
    PON_REDUNDANCY_OLT_FAILURE_HOST_REQUEST = 0,    
    PON_REDUNDANCY_OLT_FAILURE_PON_LOSS     = 1,  
    PON_REDUNDANCY_OLT_RESET                = 2,  /* Inner use */
    PON_REDUNDANCY_OLT_LLID_FAILED          = 64,
    PON_REDUNDANCY_OLT_CardPull             = 98,/*for onu swap by jinhl@2013-04-27*/
    PON_REDUNDANCY_OLT_Removed              = 99,/*for onu swap by jinhl@2013-04-27*/
    PON_REDUNDANCY_OLT_ONUSWAPOVER          = 100/*for onu swap by jinhl@2013-02-22*/
} PON_redundancy_olt_failure_reason_t;

typedef enum
{                                      
    PON_REDUNDANCY_SWITCH_OVER_SUCCEED      = 0,    
    PON_REDUNDANCY_SWITCH_OVER_FAILED       = 1  

} PON_redundancy_switch_over_status_t;


typedef enum
{                                      
    PON_REDUNDANCY_TYPE_1_1             = 0,    
    PON_REDUNDANCY_TYPE_N_M             = 1,
    PON_REDUNDANCY_TYPE_USER            = 2   

} PON_redundancy_type_t;


#ifdef PAS_SOFT_VERSION_V5_3_12
#define REDUNDANCY_OLT_ID_INVALID  0xFF
#define REDUNDANCY_ONU_ID_INVALID 0xFF

#define PON_REDUNDANCY_OLT_FAILURE_LLID_MAX_NUM 					    128
#define PON_REDUNDANCY_ONU_LLID_ALL										0xff

typedef struct
{
	PON_redundancy_olt_failure_reason_t reason;
	short int								llid_n;
	short int* 							llid_list_marker;
} PON_redundancy_olt_failure_info_t;

/*Begin:for onu swap by jinhl@2013-02-22*/
typedef struct
{
	PON_redundancy_olt_failure_reason_t reason;
	short int								llid_n;
	short int 							llid_list_marker[PON_REDUNDANCY_OLT_FAILURE_LLID_MAX_NUM];
} PON_redundancy_msg_olt_failure_info_t;
/*End:for onu swap by jinhl@2013-02-22*/

/* Redundancy specific handler functions enum */
typedef enum
{
    REDUNDANCY_HANDLER_SWITCH_OVER_BETWEEN_PAS_SOFT,         /*switching over between PAS-SOFT is required. */
    REDUNDANCY_HANDLER_SLAVE_OLT_NOT_AVAILABLE,              /*Slave OLT is not available*/
    REDUNDANCY_HANDLER_SWITCH_OVER_SUCCESS,                  /*Swiching over is done successfully*/
    REDUNDANCY_HANDLER_REDUNDANCY_SWITCH_OVER_FAILED,        /*Failed to switch over*/
    REDUNDANCY_HANDLER_SWITCH_OPTICAL_PORT_REQUIRED,         /*Require application to switch on slave OLT optical port*/
    REDUNDANCY_HANDLER_LAST_HANDLER,

} redundancy_handler_index_t;

typedef enum
{
    REDUNDANCY_GET_EXT_INFO_RAW_DATA = 0,
    REDUNDANCY_SET_EXT_INFO_RAW_DATA,
    REDUNDANCY_SYNC_EXT_INFO_TYPE
}redundancy_ext_info_opt_type_t;

typedef struct {
	PON_olt_id_t olt_assign_odd_llid;
	PON_olt_id_t olt_assign_even_llid;
} redundancy_onu_optical_olt_pair_t;

typedef enum
{                                      
    PON_REDUNDANCY_LLID_REDUNDANCY_NORMAL     = 0,
	PON_REDUNDANCY_LLID_REDUNDANCY_SLAVE      = 1,    
    PON_REDUNDANCY_LLID_REDUNDANCY_ACTIVE     = 2,    
    PON_REDUNDANCY_LLID_REDUNDANCY_STANDBY    = 3

} PON_redundancy_llid_redundancy_mode_t;

typedef enum
{
    PON_OLT_REDUNDANCY_STATE_NONE             = 0,   
    PON_OLT_REDUNDANCY_STATE_ASSIGN_ODD_LLID  = 1, /* The OLT which only assign odd llid */
    PON_OLT_REDUNDANCY_STATE_ASSIGN_EVEN_LLID = 2, /* The OLT which only assign even llid */
    PON_OLT_REDUNDANCY_STATE_MASTER           = 3,  
    PON_OLT_REDUNDANCY_STATE_SLAVE            = 4, 
    PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE   = 5,/* state master enters after switch over */
    PON_OLT_REDUNDANCY_STATE_INACTIVE_MASTER  = 6 /* Inner use: state slave enters after switch over */

} PON_redundancy_olt_state_t;

typedef struct
{
	PON_onu_id_t				    onu_id;
	mac_address_t				    onu_mac;
	unsigned short int			    laser_on_time;
	unsigned short int			    laser_off_time;
	PON_authentication_sequence_t   authentication_sequence;
	bool						    passave_originated;
    PON_llid_oam_db_t               llid_oam_db;
    unsigned char                   session_id;
    PON_redundancy_llid_redundancy_mode_t  rdnd_mode; 
    
    /* Inner use */
	PON_rtt_t				        rtt;
    PON_onu_hardware_version_t      hardware_version;             
    short int                       fw_remote_mgnt_major_version; 
    short int                       fw_remote_mgnt_minor_version; 

} PON_redundancy_onu_register_t;

#else

typedef enum
{
    PON_OLT_REDUNDANCY_STATE_NONE             = 0,   
    PON_OLT_REDUNDANCY_STATE_MASTER           = 1,   
    PON_OLT_REDUNDANCY_STATE_SLAVE            = 2,   
    PON_OLT_REDUNDANCY_STATE_INACTIVE_SLAVE   = 3,/* state master enters after switch over */
    PON_OLT_REDUNDANCY_STATE_INACTIVE_MASTER  = 4 /* Inner use: state slave enters after switch over */

} PON_redundancy_olt_state_t;

typedef struct
{
	PON_onu_id_t				    onu_id;
	mac_address_t				    onu_mac;
	unsigned short int			    laser_on_time;
	unsigned short int			    laser_off_time;
	PON_authentication_sequence_t   authentication_sequence;
	bool						    passave_originated;
    PON_llid_oam_db_t               llid_oam_db;
    unsigned char                   session_id;
    
    /* Inner use */
	PON_rtt_t				        rtt;
    PON_onu_hardware_version_t      hardware_version;             
    short int                       fw_remote_mgnt_major_version; 
    short int                       fw_remote_mgnt_minor_version; 

} PON_redundancy_onu_register_t;

#endif


/* Redundancy olt failure handler
**
** Event indicating a redundancy olt failure
**
** Input Parameters:
**		olt_id	        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		reason   		: OLT failure reason
**	
** Return codes:
**				Return codes are not specified for an event
*/
typedef short int (*PAS_redundancy_olt_failure_handler_t) ( const short int                             olt_id, 
											                const PON_redundancy_olt_failure_reason_t   reason);

/* Redundancy switch over handler
**
** Event indicating a redundancy switch over
**
** Input Parameters:
**		olt_id	        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		status   		: switch over status
**				
** Return codes:
**				Return codes are not specified for an event
*/
typedef short int (*PAS_redundancy_switch_over_handler_t) ( const short int                             olt_id, 
											                const PON_redundancy_switch_over_status_t   status);

/*========================= Functions Prototype =============================*/
/* redundancy init
**
** This command initializes the redundancy mng.  
**
** Input Parameters:
**      
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes

*/
extern PON_STATUS redundancy_init (void); 

/* redundancy terminate
**
** This command terminates the redundancy mng.  
**
** Input Parameters:
**      
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_terminate (void); 

/* Is init								
**
** This command returns whether the REDUNDANCY_MNG is initialized
**	
** Input Parameters:	
**	  
** Ouput Parameters:		  
**      init                    : FALSE or TRUE
**
** Return codes:
**				All the defined REDUNDANCY_MNG_* return codes
**
*/
extern PON_STATUS redundancy_is_init ( bool *init );


/* redundancy_master_slave_config
**
**This API is used to configure mapping configuration. If it's N:M redundancy, 
**there's may several master OLT and several slave OLT, GPIO configuration is useless.  
**If it's 1:1 redundancy, master_olt_list[i] and slave_olt_list[i] should be a pair. 
**If the master and slave are in different PAS-SOFT, the one in other PAS-SOFT should be marked as 0xFF. 
**But the number of OLT should be count to make master OLT number equals to slave OLT number.  
**The GPIO list should be specified, the sequence should be the same as the OLT ID in master/slave list. 
**
** Input Parameters:
**
**      master_olt_id_list   :Master OLT list
**      master_olt_id_num    :Number of master OLT
**      slave_olt_id_list    :Slave OLT list
**      slave_olt_id_num     :Number of slave OLT
**      master_gpio_list     :Master OLT's gpio number list
**      slave_gpio_list      :Slave OLT's gpio number list
**
** Output Parameters:        
**
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_master_slave_config (const olt_id_list_t                master_olt_id_list,
                                           const unsigned short               master_olt_id_num,
                                           const olt_id_list_t                slave_olt_id_list,
                                           const unsigned short               slave_olt_id_num,
                                           const gpio_list_t                  master_gpio_list,
                                           const gpio_list_t                  slave_gpio_list);


/* redundancy switch_over
** This function initiates master to slave switch over per user request. 
** It will post semaphore the wake up switch over process thread to start switching over. 
** If slave_olt_id is not 0xFF, the slave OLT id is specified, or the switching over process thread will search a suitable slave olt automatically. 
**
** Input Parameters:
**      master_olt_id        : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      slave_olt_id         : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_switch_over (const PON_olt_id_t              master_olt_id,
                                   const PON_olt_id_t              slave_olt_id);


/* redundancy enable
**
**This API is used to enable redundancy module functionality, include start the backup and switching over task, 
**allocate memory for redundancy DB, etc.
**
** Input Parameters:
**
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_enable(void);


/* redundancy disable
**
**This API is used to disable redundancy module functionality, include stoping the backup and switching over task,
**release memory for redundancy DB, etc.
**
** Input Parameters:
**
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_disable(void);
extern PON_STATUS redundancy_disable_olt(const PON_olt_id_t          olt_id);


/* redundancy set sampling interval
**
**This API is used to sets the sampling interval.
**
** Input Parameters:
**      sample_interval          : configure the interval of Sampling the statuses in the Master OLT, in Seconds unit.
**
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_set_sampling_interval(const unsigned long sample_interval);


/* redundancy_get_sampling_interval
**
**This function gets the sampling interval.
**
** Input Parameters:
**
** Output Parameters:
**      sample_interval          : configure the interval of Sampling the statuses in the Master OLT, in Seconds unit.
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
extern PON_STATUS redundancy_get_sampling_interval(unsigned long *sample_interval);


extern short int Get_redundancy_olt_state 
						( const short int		       olt_id, 
						  PON_redundancy_olt_state_t  *redundancy_state );


/* Redundancy ONU registration
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      onu_register     : see PON_redundancy_onu_register_t for details
**
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_redundancy_onu_register
                                    (const PON_olt_id_t                   olt_id,
                                     const PON_redundancy_onu_register_t  onu_register);

/* Set Redundancy configuration
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      olt_state        : see PON_redundancy_olt_state_t for details
**      gpio_number      : 0-3 or PON_REDUNDANCY_NO_GPIO
**      type             : see PON_redundancy_type_t for details
**      pon_rx_enabled   : enable/disable pon_rx - The slave will disable address table learning if pon_rx_enabled parameter is disabled.
**                         After switchover, address table learning will be restored according to the state configuration before it was defined as slave.
**                         The master behaves according to the state configuration before it was defined as master.
**
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_set_redundancy_config
                                    (const PON_olt_id_t                 olt_id,
                                     const PON_redundancy_olt_state_t   olt_state,
                                     const unsigned short               gpio_number,
                                     const PON_redundancy_type_t        type,
                                     const bool                         pon_rx_enabled);

/* Get Redundancy configuration
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
**      olt_state        : see PON_redundancy_olt_state_t for details
**      gpio_number      : 0-3 or PON_REDUNDANCY_NO_GPIO
**      type             : see PON_redundancy_type_t for details
**      pon_rx_enabled   : enable/disable pon_rx - The slave will disable address table learning if pon_rx_enabled parameter is disabled.
**                         After switchover, address table learning will be restored according to the state configuration before it was defined as slave.
**                         The master behaves according to the state configuration before it was defined as master.
**                    
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_get_redundancy_config
                                    (const PON_olt_id_t            olt_id,
                                     PON_redundancy_olt_state_t   *olt_state,
                                     unsigned short               *gpio_number,
                                     PON_redundancy_type_t        *type,
                                     bool                         *pon_rx_enabled);

/* Redundancy switch over
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**
** Output Parameters:
** 
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_redundancy_switch_over
                                    (const PON_olt_id_t                    olt_id);

/* Get redundancy address table - no learning
**
** The complete address table of the specified OLT is copied into a memory space allocated by the caller.
** Assumption: the caller must allocate the memory for the address table before calling the function, and
** deallocate it after the function returns.	
**
** Input Parameters:
**			    olt_id								  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				
** Output Parameter:
**				active_records						  : The actual size of the address table, 
**														range: 0 - PON_ADDRESS_TABLE_SIZE 
**				address_table						  : Pointer to a memory address where the address 
**														table should be copied to
**					mac_address	(per address record)  : Address record MAC address, may be unicast or 
**														multicast
**					logical_port (per address record) : Address record logical port, translates to ONU id/
**														CNI port in the current version, 
**														values:PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**														PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID
**					type (per address record)		  : Address record type, range: enum values
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
PON_STATUS PAS_get_redundancy_address_table 
                                           ( const PON_olt_id_t            olt_id, 
											 short int			          *active_records,	
											 PON_address_table_t           address_table );

/* Add address table multi records
**
** Add / update the content of multi entries in the address table. 
** If the MAC address already found in the address table, the logical port is updated. 
** This function is generally used for multicast address learning.		
**
** Input Parameters:
**			    olt_id			  : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**			    num_of_records    : Size of the address table
**			    address_table	  : Pointer to a memory address where the address table should be
**					mac_address	(per address record)  : Address record MAC address, may be unicast or 
**														multicast
**					logical_port (per address record) : Address record logical port, translates to ONU id/
**														CNI port in the current version, 
**														values:PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT
**														PON_ADDRESS_TABLE_RECORD_SYSTEM_LLID
**					type (per address record)		  : Address record type, range: enum values
**
** Return codes:
**				All the defined PAS_* return codes
**
*/
PON_STATUS PAS_add_address_table_multi_records
                                       ( const PON_olt_id_t			  olt_id, 
                                         const unsigned short		  num_of_records,
										 PON_address_table_t          address_table );

typedef enum
{
    PON_RTT_FW_UPDATE          = 0,
    PON_RTT_HOST_UPDATE        = 1

} PON_rtt_update_mode_t;

typedef enum
{
    PON_RTT_TABLE_SW           = 0,
    PON_RTT_TABLE_HW           = 1

} PON_rtt_table_t;

typedef struct PON_llid_rtt_value_t
{
    PON_llid_t		llid;		 /* LLID index	 	 */
    short int       rtt_value;  /* LLID RTT value    */  

} PON_llid_rtt_value_t;

typedef  PON_llid_rtt_value_t llid_rtt_value_list_t[PAS_LLID_PER_OLT_ARRAY_SIZE];

typedef enum
{
    PON_ADD_VIRTUAL_LLID       = 0,
    PON_REMOVE_VIRTUAL_LLID    = 1

} PON_virtual_llid_operation_t;

/* Get RTT update mode
**
** Input Parameters:
**      olt_id           : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**      rtt_table       : Updated rtt table, see PON_rtt_table_t for details
**      number_of_entries    : Number of LLID and it's RTT value to set
**      llid_list         : The LLID list, see llid_list_t for details
**      
** Output Parameters:
**       rtt_value_list: The RTT value list, see llid_rtt_value_list_t for details
**
** Return codes:
**          All the defined         PAS_* return codes
*/
PON_STATUS PAS_get_rtt_value
                                   (const PON_olt_id_t        olt_id,
                                    PON_rtt_table_t            rtt_table,
                                    const unsigned short      number_of_entries,
                                    llid_list_t              llid_list,
                                    llid_rtt_value_list_t        rtt_value_list);

short int reset_redundancy_record ( const short int                  olt_id,
					                const PON_redundancy_olt_state_t state );

#endif   /* PAS_SOST_VERSION_V5_3_11 */
/* E--added by liwei056@2010-1-25 for NewVersion5.3.11's redundancy */

/* B--added by liwei056@2010-6-22 for NewVersion5.3.12's redundancy */
#ifdef PAS_SOFT_VERSION_V5_3_12

/* ctc redundancy init
**
** This command initializes the ctc redundancy mng.  
**
** Input Parameters:
**      
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes

*/
PON_STATUS ctc_redundancy_init (void);

/* ctc redundancy terminate
**
** This command terminates the ctc redundancy mng.  
**
** Input Parameters:
**      
** Output Parameters:
** 
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
PON_STATUS ctc_redundancy_terminate (void);

/* redundancy_get_master_slave_list
**
**This API is used to get mapping configuration.
**
** Input Parameters:
**
**
** Output Parameters:        
**
**      master_olt_list            : Master OLT list
**      slave_olt_list             : Slave OLT list
**
** Return codes:
**          All the defined         REDUNDANCY_MNG_* return codes
*/
PON_STATUS redundancy_get_master_slave_list(olt_id_list_t master_olt_list,
                                            olt_id_list_t slave_olt_list);

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
**		All the defined REDUNDANCY_MNG_* return codes
*/
PON_STATUS redundancy_assign_handler_function
                ( const redundancy_handler_index_t    handler_function_index, 
                  const void                        (*handler_function)(),
                  unsigned short                     *handler_id );

/* Delete handler function									
**
** Remove a handler function from the DB.
**
** Input Parameters:
**	    handler_id     : Handler function identification.
**										  
**
** Return codes:
**		All the defined REDUNDANCY_MNG_* return codes
*/
PON_STATUS redundancy_delete_handler_function ( const unsigned short  handler_id );


short int reset_redundancy_olt_record (const short int   olt_id,
                                       const PON_redundancy_olt_state_t state);

bool redundancy_olt_exists (const short int olt_id);

short int get_redundancy_state(const short int  olt_id, 
                               PON_redundancy_olt_state_t  *state );
short int set_redundancy_state(const short int  olt_id, 
                               const PON_redundancy_olt_state_t  state );

short int remove_olt_from_mapping_list(const short int     olt_id);

short int get_pair_master_olt_id(const PON_olt_id_t          olt_id,
                                 PON_olt_id_t               *master_olt_id,
                                 PON_redundancy_olt_state_t  *state);


short int get_pair_slave_olt_id(const PON_olt_id_t   olt_id,
                                PON_olt_id_t    *slave_olt_id,
                                PON_redundancy_olt_state_t  *state);

short int remote_olt_removed_handler(const PON_olt_id_t  olt_id, const short int  remote_olt_id);
short int remote_olt_loosed_handler(const PON_olt_id_t  olt_id, const short int  remote_olt_id);

short int swap_mapping_list(const PON_olt_id_t   master_olt_id, 
                            const PON_olt_id_t slave_olt_id);
short int swap_local_mapping_list(const PON_olt_id_t   master_olt_id, 
                            const PON_olt_id_t slave_olt_id);
short int insert_mapping_pair_to_local_list(const PON_olt_id_t   master_olt_id, 
                                      const PON_olt_id_t slave_olt_id);


/**************************************************************************************
** redundancy_onu_optical_config_olt_pair
** This function is used to add a pair of OLT to ONU optical redundancy system
**
** Input Parameters:
**         olt_pair           : The pair of OLT id.
** 
** Return codes:
**              Return codes are not specified for an event
**
**************************************************************************************/
PON_STATUS redundancy_onu_optical_config_olt_pair(const redundancy_onu_optical_olt_pair_t olt_pair);

/**************************************************************************************
** redundancy_onu_optical_remove_olt_pair
** This function is used to remove a pair of OLT from ONU optical redundancy system
**
** Input Parameters:
**         olt_pair           : The pair of OLT id. 
** 
** Return codes:
**              All the defined         REDUNDANCY_MNG_* return codes
**************************************************************************************/
PON_STATUS redundancy_onu_optical_remove_olt_pair(const redundancy_onu_optical_olt_pair_t olt_pair);

/**************************************************************************************
** redundancy_onu_optical_add_onu
** This function is used to add an ONU to a pair of ONU optical redundancy system OLT
**
** Input Parameters:
**         olt_id           : The olt id.
**         onu_id	        : The ONU id. 
** 
** Return codes:
**              All the defined         REDUNDANCY_MNG_* return codes
**
**************************************************************************************/
PON_STATUS redundancy_onu_optical_add_onu(const PON_olt_id_t olt_id, const PON_onu_id_t onu_id);

PON_STATUS PAS_redundancy_llid_redund_standby_to_active
                                    (const PON_olt_id_t                   olt_id,
									 const short int 					  llid_n,
									 const short int* 					  llid_list_marker);
/*Begin:for onu swap by jinhl@2013-02-22*/
short int get_onu_optical_mapping_list_pair_by_olt_id(PON_olt_id_t olt_id, PON_olt_id_t *standby_olt_id);
short int Get_redundancy_onu_mode(PON_olt_id_t olt_id, PON_onu_id_t onu_id, PON_redundancy_llid_redundancy_mode_t* mode);
short int Set_redundancy_onu_mode(PON_olt_id_t olt_id, PON_onu_id_t onu_id, const PON_redundancy_llid_redundancy_mode_t mode);
short int Set_onu_record_rtt_value(PON_olt_id_t olt_id, PON_onu_id_t onu_id, const PON_rtt_t rtt);
/*End:for onu swap by jinhl@2013-02-22*/
#endif
/* E--added by liwei056@2010-6-22 for NewVersion5.3.12's redundancy */

short int get_onu_register_info_from_fw(const PON_olt_id_t  olt_id, 
                                               const PON_onu_id_t  onu_id,
                                               PON_redundancy_onu_register_t*   onu_register);

short int Get_encryption_mode ( const short int	   olt_id, 
								const PON_llid_t   llid,
								bool			  *encryption_active );

/* Start OLT encryption
**
** Start the process of encrypting all downstream / upstream and downstream directions traffic destined
** to and originating from a specific LLID. The encryption key set by the last 
** PAS_set_olt_encryption_key function is used as the encryption key (both for upstream
** and downstream directions). This command is applicable to non-PAS6001 ONUs, PAS6001 ONU may start LLID
** link encryption process only when using PAS_start_encryption function.
** This function must follow PAS_set_olt_encryption_key function call, and be followed by
** PAS_finalize_start_olt_encryption function call, according to the 'Start encryption sequence
** using non-PAS6001 ONU' diagram in the framework document.
**
** Input Parameters:
**			    olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**				llid		: LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**
** Return codes:
**				All the defined PAS_* return codes
*/
PON_STATUS PAS_start_olt_encryption 
                                ( const PON_olt_id_t	 olt_id, 
								  const PON_llid_t	     llid );


/* Finalize start OLT encryption
**
** Finish the process of encrypting all downstream / upstream and downstream directions traffic destined
** to and originating from a specific LLID.
** This function is applicable to non-PAS6001 ONUs, PAS6001 ONU may complete LLID link encryption process 
** only when using PAS_start_encryption function. This function must follow 
** PAS_start_olt_encryption function call, according to the 'Start encryption sequence using 
** non-PAS6001 ONU' diagram in the framework document.
**
** Input Parameters:
**		olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid		: LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**		status		: LLID confirmation code to the Start DS encryption frame. Values: EXIT_OK, EXIT_ERROR
**
** Return codes:
**		All the defined PAS_* return codes
*/
PON_STATUS PAS_finalize_start_olt_encryption 
                                        ( const PON_olt_id_t   olt_id, 
										  const PON_llid_t	   llid,
										  const short int      status );

/* Stop OLT encryption
**
** Stop encrypting all (the already encrypted) downstream and upstream traffic destined to and originating
** from a specific LLID. This function is applicable to non-PAS6001 ONUs. PAS6001 ONU may stop LLID link
** encryption process only when calling to PAS_stop_encryption function.
** This function is to be used according to the 'Stop encryption sequence using non-PAS6001 ONU' diagram 
** in the framework document: it should be called only after notification to the LLID, active upstream 
** encryption should be stopped prior to the activation of this function.
**
** Input Parameters:
**		olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid		: LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**
** Return codes:
**		All the defined PAS_* return codes
*/
PON_STATUS PAS_stop_olt_encryption 
                                ( const PON_olt_id_t	 olt_id, 
								  const PON_llid_t	     llid );

/* Set OLT encryption key
**
** Set new encryption key to LLID link encryption. The LLID link may be encrypted as well as not encrypted
** at the time the function is called. If LLID link is not encrypted at the time this function is called,
** the next link encryption (PAS_start_olt_encryption) will use the set key.
** This function must be followed by PAS_finalize_set_olt_encryption_key function call,
** according to one the 'Set encryption key sequence' diagrams in the framework document.
** This function can't be called twice without calling to PAS_finalize_set_olt_encryption_key
** function in between.
**
** PAS6001 ONU
** -----------
** This function must be followed by notification of the new encryption key to the LLID (System software 
** responsibility), calling to PAS6001_update_encryption_key function by the ONU system application, 
** and calling to PAS_finalize_set_olt_encryption_key function in order to complete the 
** encryption key update process.
**
** Non-PAS6001 ONU
** ---------------
** According to the 'non-PAS6001 Set encryption key sequence' diagram in the framework document. 
**
** Input Parameters:
**		olt_id			 : OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid			 : LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**		encryption_key   : New encryption key, range:(PON_ENCRYPTION_KEY_SIZE*BITS_IN_BYTE)bit value
**
** Return codes:
**		All the defined PAS_* return codes
*/
PON_STATUS PAS_set_olt_encryption_key 
                                   ( const PON_olt_id_t		     olt_id,           
                                     const PON_llid_t		     llid,             
                                     const PON_encryption_key_t  encryption_key );

/* Finalize set OLT encryption key
**
** Finish the process of setting new encryption key to LLID link. This function should always be called
** after PAS_set_olt_encryption_key function (and after key change notification frame sending
** to the LLID), and according to one of the 'Set encryption key sequence' diagrams in the framework 
** document. After this function is called with EXIT_OK value in status parameter, and returns with no 
** error return code, the new encryption key is ready to be used. If the LLID link is already encrypted - 
** the new key will be used, if not it will be used on the next link encryption 
** (PAS_start_olt_encryption).
** 
** Status parameter
** ----------------
** EXIT_OK value is to be used when the PAS5001 Host application gets a success acknowledge indication 
** from the LLID regarding new key setting. EXIT_ERROR value is to be used when the PAS5001 Host 
** application gets a failure indication from the LLID regarding new key setting, or timeout expired 
** for getting the acknowledge from the LLID.
**
** Input Parameters:
**		olt_id		: OLT id, range: PON_MIN_OLT_ID - PON_MAX_OLT_ID
**		llid		: LLID index, range: PON_MIN_LLID_PER_OLT - PON_MAX_LLID_PER_OLT  
**		status		: LLID confirmation code to the Set encryption key frame. Values: EXIT_OK, EXIT_ERROR
**
** Return codes:
**		All the defined PAS_* return codes
*/
PON_STATUS PAS_finalize_set_olt_encryption_key
                                            ( const PON_olt_id_t  olt_id, 
											  const PON_llid_t	  llid,
											  const short int     status );


/* B--added by liwei056@2010-7-22 for 6900-LocalBus */
/* Physical layer communication (Host <-> hardware) types supported */
typedef enum PON_comm_type_t
{
	PON_COMM_TYPE_PARALLEL,  
	PON_COMM_TYPE_ETHERNET,   
	PON_COMM_TYPE_UART,
	PON_COMM_TYPE_LAST_TYPE
} PON_comm_type_t;

PON_STATUS PAS_set_current_interface(PON_comm_type_t comm_interface);

PON_STATUS PAS_get_current_interface(PON_comm_type_t *comm_interface);

PON_STATUS check_parallel_magic_number(PON_olt_id_t olt_id);
/* E--added by liwei056@2010-7-22 for 6900-LocalBus */

int PAS_set_olt_base_address(unsigned short olt_base_address);



#ifdef  PAS_SOFT_VERSION_V5_3_13
#define PON_MIGRATION_ATTEMPT_EVENT_MIN_ENTRY 1
#define PON_MIGRATION_ATTEMPT_EVENT_MAX_ENTRY 8

typedef struct  
{
  mac_address_t          mac_address;
  PON_llid_t             original_llid; 
  PON_llid_t             new_llid; 
  unsigned short         migration_status;
} PON_migration_info_t;

typedef enum
{                                      
    PON_DUPLICATE_MAC_HANDLE_MODE_STANDARD             = 0,    /*Standard mode - OLT will send broadcast NACK for duplicate MAC registering request (default mode).*/
    PON_DUPLICATE_MAC_HANDLE_MODE_IGNORING             = 1,  /*Ignoring mode - OLT will ignore the duplicate MAC registering request.*/
    PON_DUPLICATE_MAC_HANDLE_MODE_DEREGISTER		= 2  /* Deregister mode - OLT will deregister ONU with the duplicate MAC */

} PON_duplicate_mac_handle_mode_t;
#endif



#endif

