/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  PonLog.h -  Header file for LOG interface 
**
**  This file was written by liwei056, 03/09/2012
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 03/09/2012 |	creation	      | liwei056
*/

#if !defined(__PONLOG_H__)
#define __PONLOG_H__

#if defined(__cplusplus)
extern "C" {
#endif


#ifndef bool
#define bool unsigned char /* TRUE / FALSE */
#endif
 

#define FUNCTIONALITY_LENGTH       20
#define PONLOG_MAX_FUNCTIONALITIES 20
typedef unsigned char PONLOG_functionality_t[FUNCTIONALITY_LENGTH];

/* Basic functionalities */
#define PONLOG_FUNC_IGNORE                   "ignore"							
#define PONLOG_FUNC_GENERAL                  "general"          /* General messages											*/
#define PONLOG_FUNC_SYS_DUMP                 "sys-dump"          /* Sys Dump messages */

/* EPON functionalities */								    								    	
#define PONLOG_FUNC_ENCRYPTION_FLOW	        "encryption"	    
#define PONLOG_FUNC_REGISTRATION_FLOW        "registration"	    
#define PONLOG_FUNC_DATA_FLOW                "data-flow"        /* PON-SOFT DataFlow operations messages							*/
#define PONLOG_FUNC_QOS_FLOW                 "qos-flow"         /* PON-SOFT QosFlow operations messages							*/
#define PONLOG_FUNC_API_CALLED               "api-called"       /* PON-SOFT API used messages								*/
#define PONLOG_FUNC_EVENT_FLOW               "event-flow"       /* PON-SOFT events handlers messages						    */
#define PONLOG_FUNC_STATISTICS               "statistics"       /* Statistics & Monitoring messages (periodic only)			*/
#define PONLOG_FUNC_FW                       "fw"               /* Firmware messages										    */
#define PONLOG_FUNC_AUTHENTICATION_FLOW	    "authentication"	    
#define PONLOG_FUNC_ALARM             	    "alarm"	    

/* General constant definition for not changing value / parameter */
#define PON_VALUE_NOT_CHANGED		 (-1)
#define PON_UNKNOWN_VALUE_STRING	 "unknown"
#define PON_NOT_CHANGED_IP_STRING	 "255.255.255.255"

#define PONLOG_VALUE_NOT_CHANGED		PON_VALUE_NOT_CHANGED
#define PONLOG_NOT_CHANGED_IP_STRING	PON_NOT_CHANGED_IP_STRING

#define PONLOG_OLT_IRRELEVANT (-1)
#define PONLOG_ONU_IRRELEVANT (-1)


/* Assumption: Each module defines __MODULE__ as a string containing the module name */
#define __PONLOG_MODULE_FORMAT__	    __MODULE__" "
#define __PONLOG_FILE_FORMAT__       __FILE__" "
#define __PONLOG_FUNCTION_FORMAT__   PONLOG_FUNCTION_NAME


#define PONLOG_EPON_register_functionality(x)   


/* Define the PONLOG severity flag as the syslog(3) severities */
typedef enum PONLOG_severity_flag_t 
{
    PONLOG_SEVERITY_IGNORE   = -1 ,
    PONLOG_SEVERITY_FATAL    = 0	 , 
    PONLOG_SEVERITY_ALERT    = 1	 , 
    PONLOG_SEVERITY_CRIT     = 2  , 
    PONLOG_SEVERITY_ERROR    = 3	 , 
    PONLOG_SEVERITY_WARN     = 4	 ,   
    PONLOG_SEVERITY_NOTICE	= 5	 , 
    PONLOG_SEVERITY_INFO     = 6	 ,   
    PONLOG_SEVERITY_DEBUG    = 7	 , 
    PONLOG_SEVERITY_LAST
} PONLOG_severity_flag_t;


int PONLOG_print(const bool               show_full_info,
					   char					   *file_name,
					   char					   *func_name,
					   PONLOG_severity_flag_t    severity_flag,
					   const char *             functionality,
					   long int   			    olt_id,
					   long int    	  		    onu_id,
					   const char*				format,
										  ...);

bool PONLOG_check_configuration_for_print
					  (PONLOG_severity_flag_t   severity_flag,
					   const char *             functionality,
					   long int   			    olt_id,
					   long int    	  		    onu_id);

short int PON_general_hex_bytes_to_string(
							unsigned char *data, 
							int data_len, 
							char *output_buffer, 
							int output_buffer_len);

/* MACROS FOR TRACE FUNCTIONS */
/* Macros for Errors */
/* Assumption : user will define PONLOG_FUNCTION_NAME in his scope */

#ifdef ERROR_DEBUG

#define PONLOG_ERROR_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__);										

#define PONLOG_ERROR_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										

#define PONLOG_ERROR_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				     PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_ERROR_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_ERROR_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_ERROR_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);

#define PONLOG_ERROR_6(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__);

#define PONLOG_ERROR_7(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__);

#define PONLOG_ERROR_8(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__);

#define PONLOG_ERROR_9(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__);

#define PONLOG_ERROR_14(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__, __arg11__, __arg12__, __arg13__, __arg14__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_ERROR, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__, __arg11__, __arg12__, __arg13__, __arg14__);

/* CRITICAL_ERROR_DEBUG */
#define PONLOG_CRITICAL_ERROR_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_CRIT, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__);										

#define PONLOG_CRITICAL_ERROR_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_CRIT, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										

#define PONLOG_CRITICAL_ERROR_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_CRIT, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_CRITICAL_ERROR_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_CRIT, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_CRITICAL_ERROR_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
					 PONLOG_SEVERITY_CRIT, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);										


#define PONLOG_DEBUG_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, __onu_id__, __str__);										

#define PONLOG_DEBUG_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										
                                                           
#define PONLOG_DEBUG_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_DEBUG_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_DEBUG_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_DEBUG_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_DEBUG, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);

#else

#define PONLOG_ERROR_0(__functionality__,__olt_id__,__onu_id__,__str__)

#define PONLOG_ERROR_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)

#define PONLOG_ERROR_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)

#define PONLOG_ERROR_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)

#define PONLOG_ERROR_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)

#define PONLOG_ERROR_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, arg5)

#define PONLOG_ERROR_6(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__)

#define PONLOG_ERROR_7(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__)

#define PONLOG_ERROR_8(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__)

#define PONLOG_ERROR_9(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__)

#define PONLOG_ERROR_14(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__, __arg11__, __arg12__, __arg13__, __arg14__)


#define PONLOG_CRITICAL_ERROR_0(__functionality__,__olt_id__,__onu_id__,__str__)

#define PONLOG_CRITICAL_ERROR_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)

#define PONLOG_CRITICAL_ERROR_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)

#define PONLOG_CRITICAL_ERROR_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)

#define PONLOG_CRITICAL_ERROR_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)


#define PONLOG_DEBUG_0(__functionality__,__olt_id__,__onu_id__,__str__)

#define PONLOG_DEBUG_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)

#define PONLOG_DEBUG_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)

#define PONLOG_DEBUG_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)

#define PONLOG_DEBUG_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)

#define PONLOG_DEBUG_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, arg5)
#endif


#define PONLOG_INFO_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, __onu_id__, __str__);										

#define PONLOG_INFO_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										
                                                           
#define PONLOG_INFO_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_INFO_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_INFO_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_INFO_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);

#define PONLOG_INFO_6(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__);

#define PONLOG_INFO_7(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__);

#define PONLOG_INFO_8(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__);

#define PONLOG_INFO_9(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__);

#define PONLOG_INFO_10(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__)	\
		PONLOG_print(TRUE,  __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__);

#define PONLOG_INFO_14(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__, __arg11__, __arg12__, __arg13__, __arg14__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_INFO, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__, __arg8__, __arg9__, __arg10__, __arg11__, __arg12__, __arg13__, __arg14__);


#define PONLOG_INFO_API_CALLED_0(__olt_id__,__onu_id__)	\
		PONLOG_INFO_1(PONLOG_FUNC_API_CALLED, __olt_id__, __onu_id__, "%s API Called.\n", PONLOG_FUNCTION_NAME)

#define PONLOG_INFO_API_CALLED_1(__olt_id__,__onu_id__, __str__, __arg1__)	\
		PONLOG_INFO_2(PONLOG_FUNC_API_CALLED, __olt_id__, __onu_id__, "%s API Called - "__str__"\n", PONLOG_FUNCTION_NAME, __arg1__)

#define PONLOG_INFO_API_CALLED_2(__olt_id__,__onu_id__, __str__, __arg1__,__arg2__)	\
		PONLOG_INFO_3(PONLOG_FUNC_API_CALLED, __olt_id__, __onu_id__, "%s API Called - "__str__"\n", PONLOG_FUNCTION_NAME, __arg1__,__arg2__)

#define PONLOG_INFO_API_CALLED_3(__olt_id__,__onu_id__, __str__, __arg1__,__arg2__,__arg3__)	\
		PONLOG_INFO_4(PONLOG_FUNC_API_CALLED, __olt_id__, __onu_id__, "%s API Called - "__str__"\n", PONLOG_FUNCTION_NAME, __arg1__,__arg2__,__arg3__)


#define PONLOG_EVENT_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, __onu_id__, __str__);										

#define PONLOG_EVENT_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										
                                                           
#define PONLOG_EVENT_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_EVENT_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_EVENT_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_EVENT_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);
										
#define PONLOG_EVENT_6(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__);
										
#define PONLOG_EVENT_7(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_NOTICE, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__);


#define PONLOG_WARN_0(__functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, __onu_id__, __str__);										

#define PONLOG_WARN_1(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										
                                                           
#define PONLOG_WARN_2(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_WARN_3(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_WARN_4(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_WARN_5(__functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  PONLOG_SEVERITY_WARN, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);


#define PONLOG_0(__severity__, __functionality__,__olt_id__,__onu_id__,__str__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__);										

#define PONLOG_1(__severity__, __functionality__,__olt_id__,__onu_id__,__str__, __arg1__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__);										

#define PONLOG_2(__severity__, __functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__);										

#define PONLOG_3(__severity__, __functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__);										

#define PONLOG_4(__severity__, __functionality__,__olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__);
										
#define PONLOG_5(__severity__, __functionality__, __olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__);
										
#define PONLOG_6(__severity__, __functionality__, __olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__);
										
#define PONLOG_7(__severity__, __functionality__, __olt_id__,__onu_id__,__str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__)	\
		PONLOG_print(TRUE, __PONLOG_FILE_FORMAT__, __PONLOG_FUNCTION_FORMAT__,  \
				  __severity__, __functionality__, (PON_olt_id_t)__olt_id__, (PON_onu_id_t)__onu_id__, __str__, __arg1__, __arg2__, __arg3__, __arg4__, __arg5__, __arg6__, __arg7__);



#if defined(__cplusplus)
}
#endif

#endif /* __PONLOG_H__ */
