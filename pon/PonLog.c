/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  PonLog.c -  C file for PON general interface 
**
**  This file was written by liwei056, 03/09/2012
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 03/09/2012 |	creation	      | liwei056
*/

#include "PonLog.h"
#include "OltGeneral.h"


extern int g_iLogLevelPonSoft; 
/*B--added by liyang@2015-4-25 for syslog olt&onu filter */
extern short int g_sLogPonSoftLLID;
extern short int g_sLogPonSoftOltID;
extern unsigned char  g_ucLogPonSoftEnable;
/*E--added by liyang@2015-4-25 for syslog olt&onu filter */

#ifdef GW_LOG
extern void VOS_SysLog( char iLog_Type, char iLog_Level, char * szFmt, ... );
#else
extern long sys_console_buf_out( char * buf, int len );
#endif


char* gw_vsprintf2(int *o_strlen, const char* a_fmt, va_list a_args)
{
    int		size	= 1024;
    char*	buffer  = (char*)VOS_Malloc(size, MODULE_RPU_PON);
    
    while (1) {
	int n = vsnprintf(buffer, size, a_fmt, a_args);
	
	/* If that worked, return */
	if (n > -1 && n < size)
    {
        *o_strlen = n;
	    return buffer;
    }   
	
	/* Else try again with more space. */
	if (n > -1)     /* ISO/IEC 9899:1999 */
	    size = n + 1;    
	else            /* twice the old size */
	    size *= 2;      
	
	buffer = (char*)VOS_Realloc(buffer, size, MODULE_RPU_PON);
    }
}

char* gw_sprintf2(int *o_strlen, const char* a_fmt, ...)
{
    char*	buffer;
    va_list	args;

    va_start(args, a_fmt);
    buffer = gw_vsprintf2(o_strlen, a_fmt, args);
    va_end(args);

    return buffer;
}

char* get_filename_from_filepath(char* path)
{
	int i;
	char* p = path;
	
	for (i = strlen(path) -1; i >= 0; i--)
		if (path[i] == '\\' || path[i] == '/')
			return (p+i+1);

	return p;
}



static void GWLOG_print_args
                     ( char                    *file_name,
					   char					   *func_name,
					   PONLOG_severity_flag_t    severity_flag,
					   const char *             functionality,
					   long int   			    olt_id,
					   long int    	  		    onu_id,
					   const char*				format,
					   va_list					a_args)
{
    if ( (severity_flag < 0)
        || (severity_flag >= PONLOG_SEVERITY_LAST)|| (g_sLogPonSoftOltID != olt_id) || (g_sLogPonSoftLLID != onu_id)||(g_ucLogPonSoftEnable == FALSE) )
    {
        return;
    }

    if ( severity_flag <= g_iLogLevelPonSoft )
    {
        int iStrLen;
        char *pszLogOut;
        char *pszLogStr = gw_vsprintf2(&iStrLen, format, a_args);

        if (pszLogStr != NULL)
        {
#ifdef GW_LOG
#if 1
            VOS_SysLog(LOG_TYPE_PON, severity_flag, "%s() %s [olt(%d), onu(%d)].", func_name, pszLogStr, olt_id, onu_id);
#else
            VOS_SysLog(LOG_TYPE_PON, severity_flag, "File:[%s] Fun:[%s()] RunAt[%s] Log:[%s] Target[olt(%d), onu(%d)].", extract_filename_from_filepath(file_name), func_name, functionality, pszLogStr, olt_id, onu_id);
#endif
#else
            if ( NULL != (pszLogOut = gw_sprintf2(&iStrLen, "\r\nFile:[%s] Fun:[%s()] RunAt[%s] Log:[%s] Target[olt(%d), onu(%d)].", get_filename_from_filepath(file_name), func_name, functionality, pszLogStr, olt_id, onu_id)) )
            {
                sys_console_buf_out(pszLogOut, iStrLen);
                VOS_Free(pszLogOut);
            }
#endif
            VOS_Free(pszLogStr);
        }
    }
    
    return;
}

int PONLOG_print(const bool               show_full_info,
					   char					   *file_name,
					   char					   *func_name,
					   PONLOG_severity_flag_t    severity_flag,
					   const char *             functionality,
					   long int   			    olt_id,
					   long int    	  		    onu_id,
					   const char*				format,
										  ...)
{  
#ifdef ERROR_DEBUG
    va_list va;
    va_start(va, format);
    GWLOG_print_args(file_name, func_name, severity_flag, functionality, olt_id, onu_id, format, va);
    va_end(va);	
#endif
	
    return 0;
}

bool PONLOG_check_configuration_for_print
					  (PONLOG_severity_flag_t   severity_flag,
					   const char *             functionality,
					   long int   			    olt_id,
					   long int    	  		    onu_id)
{
    if ( severity_flag <= g_iLogLevelPonSoft )
    {
		return TRUE;
    }

	return FALSE;
}




