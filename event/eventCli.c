/****************************************************************************
*
*     Copyright (c) 2006 GWTT Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           GWTT Corporation
*
*     All information contained in this document is GWTT Corporation
*     company private, proprietary, and trade secret.
*
*****************************************************************************/


#ifdef	__cplusplus
extern "C"
{
#endif

/*#include "syscfg.h"

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"*/

#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include "eventMain.h"
#include "lib_gwEponMib.h"
#include "lib_ethPortMib.h"
#include "../cli/Olt_cli.h"
#include "V2R1_product.h"

#define  ONU_TYPE_MASK    EPON_MODULE_YES

extern LONG getTrapBacEnable( ULONG *pEnable );
extern LONG setTrapBacEnable( ULONG enable );
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
extern ULONG ETH_slot_logical_portnum( ULONG slotno );
extern int cl_install_module( struct cl_cmd_module * m );

extern int getEthPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG ethIdx, ULONG *mask );
extern int setEthPortAlarmMask( ULONG devIdx, ULONG brdIdx, ULONG ethIdx, ULONG mask );
extern LONG showOltAlarmStatus( struct vty * vty );
extern int showEventLogImportCmd( struct vty *vty, ULONG devIdx, ULONG level, ULONG alarmId, sysDateAndTime_t *pStartTime, sysDateAndTime_t *pEndTime);
extern int  SearchOnuTypeFullMatch( char *TypeString);
extern LONG almStatusSetOnuTypeAlarmMask( ULONG onuType, ULONG mask );

LONG getOnuEthPortAlarmMask( ULONG *mask );
LONG getOnuTypeAlarmMask(ULONG onu_type,ULONG  *maskvalue);
extern LONG showCurrentAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx );
extern LONG clearDevAlarmStatus( struct vty * vty, ULONG almLevel, ULONG devIdx );
extern LONG almStatusSetDeviceAlarmMask( ULONG devIdx, ULONG mask );
extern int SlotCardIsUplinkBoard(int CardIndex);
extern LONG ctcOnu_alarm_showrun( struct vty * vty );

static char * alarm_dev_mask_bits_to_str( ULONG mask, char *mask_str );
static char * alarm_pon_mask_bits_to_str( ULONG mask, char *mask_str );
static char * alarm_eth_mask_bits_to_str( ULONG mask, char *mask_str );

/*ULONG alarmlogToSyslog_enable=0;*//*add by shixh20090927*/
extern ULONG eventLogOutMode;
ULONG alarm_mask_pon_cni_bits = 1;
extern unsigned long  OptOnuAlarmMask;

extern ULONG eventLogFlashFileFlag;
extern LONG eventLogFlashFileSave();
extern ULONG nvramEventBackupPeriodGet();
extern ULONG nvramEventBackupEnableGet();
extern LONG nvramEventBackupEnableSet( ULONG en );
extern ULONG nvramEventBackupPeriodGet();
extern LONG nvramEventBackupPeriodSet( ULONG period );
extern ULONG nvramEventBackupPeriodDefaultGet();

sysDateAndTime_t * strToSysDateAndTime( char *pStr, sysDateAndTime_t *pDateTime )
{
	ULONG year=0, month=0, day=0, hour=0, minute=0, second=0;
	if( pStr == NULL || pDateTime == NULL )
		return NULL;
	if( VOS_Sscanf(pStr, "%d-%d-%d,%d:%d:%d", &year, &month, &day, &hour, &minute, &second) <= 0 )
	{
		return NULL;
	}
	if( year < 99 )
		pDateTime->year = year + 2000;
	else
		pDateTime->year = year;
	pDateTime->month = (uchar_t)month;
	pDateTime->day = (uchar_t)day;
	pDateTime->hour =(uchar_t) hour;
	pDateTime->minute = (uchar_t)minute;
	pDateTime->second = (uchar_t)second;
	VOS_MemZero(pDateTime->reserver, 4);

	return pDateTime;
}

LONG checkDateAndTime( struct vty *vty, sysDateAndTime_t* pTime )
{
	if( NULL == pTime )
	{
		/* 问题单11641 */
	    	v2r1_printf( vty, "%% Wrong time format!yyyy-mm-dd,hh:mm:ss\r\n" );
		/*VOS_ASSERT(0);*/
		return VOS_ERROR;
	}
	if( (pTime->year > 2079) || (pTime->year < 1980) ||
		(pTime->month > 12) || (pTime->month == 0) ||
		(pTime->day > 31) || (pTime->day == 0) ||
		(pTime->hour > 23) || (pTime->minute > 59) || (pTime->second > 59) )
	{
		v2r1_printf( vty, " Sorry, the time is invalid\r\n" );
		return VOS_ERROR;
	}
	return VOS_OK;
}

/* added by xieshl 20110519, 问题单12345, 为了确保最后4个字节为0，否则网管网管无法解析 */
VOID copyDateAndTime( sysDateAndTime_t* pDstTime, sysDateAndTime_t* pSrcTime )
{
	if( (NULL == pDstTime) || (NULL == pSrcTime) )
	{
		VOS_ASSERT(0);
		return;
	}

	VOS_MemCpy( pDstTime, pSrcTime, sizeof(sysDateAndTime_t) );
	VOS_MemZero(pDstTime->reserver, 4);
}

/*added by wutw 2006/11/27*/
/*modified by wutw 2006/11/29*/
int getTodayTime(sysDateAndTime_t *pDateTime)
{
	/*ULONG   ulRetDate = 0;
	ULONG   ulRetTime = 0;
	ULONG   ulRetMillSec = 0;*/
	sysDateAndTime_t currentTime;
	
	if (pDateTime == NULL)
		return VOS_ERROR;
	VOS_MemZero(&currentTime, sizeof(sysDateAndTime_t));
	if (VOS_OK != eventGetCurTime( &currentTime ) )
		return VOS_ERROR;
	/*if (VOS_OK != VOS_GetCurrentTime( &ulRetDate, &ulRetTime, &ulRetMillSec ))	
		return VOS_ERROR;*/
	VOS_MemCpy( pDateTime, &currentTime, sizeof(sysDateAndTime_t) );
	#if 0
	pDataTime->year = (unsigned short)((ulRetDate & 0xffff0000)>>16);
	pDataTime->month = (unsigned char)((ulRetDate & 0xff00)>>8);
	pDataTime->day =  (unsigned char)(ulRetDate & 0xff);
	#endif
	/*sys_console_printf(" today date: %d-%d-%d %d:%d:%d\r\n",pDateTime->year,
						pDateTime->month,pDateTime->day,pDateTime->hour
						, pDateTime->minute,pDateTime->day);*/
	pDateTime->hour = (unsigned short)0;
	pDateTime->minute = (unsigned char)0;
	pDateTime->second = (unsigned char)0;
				
	return VOS_OK;	
}

/*added by wutw 2006/11/27*/
int getYestodayTime( sysDateAndTime_t *pDateTime, sysDateAndTime_t *pYestodayTime )
{

	unsigned char CurrentDate = 0;
	unsigned char CurrentMonth = 0;
	unsigned short CurrentYear = 0;
	unsigned short bYear = 0;
	unsigned char bMonth = 0;
	unsigned char bDate = 0;
	
	if ((pDateTime == NULL) || (pYestodayTime == NULL ))
		return VOS_ERROR;
	CurrentYear = pDateTime->year;
	CurrentMonth = pDateTime->month;
	CurrentDate = pDateTime->day;
	
	/*检查输入的时间参数是否在合理范围之内,否则返回错误*/
	switch(CurrentMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if(CurrentDate > 31)
				return (-3);
			break;
		case 2:
			{/*检查时间是否超出范围*/
			int leapYear = 0;
			if(CurrentYear%400 == 0)
				leapYear = 1;
			else if(CurrentYear%100 == 0)
				leapYear = 0;
			else if(CurrentYear%4 == 0)
				leapYear = 1;
			else
				leapYear = 0;
			
			if ((leapYear == 0)&&(CurrentDate>28))
				return (-3);
			if ((leapYear == 1)&&(CurrentDate>29))
				return (-3);
			}
		case 4:
		case 6:
		case 9:
		case 11:
			if(CurrentDate > 30)
				return (-3);
			break;
		default:
			return (-2);
	}

	/*当Currentdata为1号时,需要计算上一个月的日期*/
	if(CurrentDate == 1)/*data = 1*/
	{
		switch(CurrentMonth)
		{
			case 1:/*当日期为1月1日时,则上一天应为上一年最后一日*/
				bYear = CurrentYear-1;
				bMonth = 12;/*12 month*/
				bDate = 31;/*day*/
				break;
			case 3:/*当为3月1日时,需要知道当年是否润年,来决定上一日为28或29日*/
				bYear = CurrentYear;
				bMonth = CurrentMonth-1;/*month*/
				if(CurrentYear%400 == 0)
					bDate = 29;
				else if(CurrentYear%100 == 0)
					bDate = 28;
				else if(CurrentYear%4 == 0)
					bDate = 29;
				else
					bDate = 28;
				break;
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				bYear = CurrentYear;
				bMonth = CurrentMonth-1;
				bDate = 30;
				break;
			case 2:
			case 4:
			case 6:
			case 9:
			case 11:
				bYear = CurrentYear;
				bMonth = CurrentMonth-1;
				bDate = 31;
				break;
			default:
				return (-1);
		}
	}
	else
	{
			bYear = CurrentYear;
			bMonth = CurrentMonth;
			bDate = CurrentDate - 1;	
	}
	
	pYestodayTime->year = bYear;
	pYestodayTime->month = bMonth;
	pYestodayTime->day =  bDate;
	pYestodayTime->hour = (unsigned short)0;
	pYestodayTime->minute = (unsigned char)0;
	pYestodayTime->second = (unsigned char)0;
	VOS_MemZero(pYestodayTime->reserver, 4);
	
	/*sys_console_printf(" yestoday date: %d-%d-%d %d:%d:%d\r\n",pYestodayTime->year,
						pYestodayTime->month,pYestodayTime->day,
						pYestodayTime->hour, pYestodayTime->minute,
						pYestodayTime->second);*/
	return VOS_OK;
}

int getTomorrowTime( sysDateAndTime_t *pDateTime, sysDateAndTime_t *pTomorrowTime )
{

	unsigned char CurrentDate = 0;
	unsigned char CurrentMonth = 0;
	unsigned short CurrentYear = 0;
	unsigned char CurrentHour=0;
	unsigned char CurrentMintue=0;
	unsigned char CurrentSecond=0;
	
	if ((pDateTime == NULL) || (pTomorrowTime == NULL ))
		return VOS_ERROR;
	CurrentYear = pDateTime->year;
	CurrentMonth = pDateTime->month;
	CurrentDate = pDateTime->day;
	CurrentHour=pDateTime->hour;
	CurrentMintue=pDateTime->minute;
	CurrentSecond=pDateTime->second;
	
	/*检查输入的时间参数是否在合理范围之内,否则返回错误*/
	switch(CurrentMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if(CurrentDate > 31)
				return (-3);
			break;
		case 2:
			{/*检查时间是否超出范围*/
			int leapYear = 0;
			if(CurrentYear%400 == 0)
				leapYear = 1;
			else if(CurrentYear%100 == 0)
				leapYear = 0;
			else if(CurrentYear%4 == 0)
				leapYear = 1;
			else
				leapYear = 0;
			
			if ((leapYear == 0)&&(CurrentDate>28))
				return (-3);
			if ((leapYear == 1)&&(CurrentDate>29))
				return (-3);
			}
		case 4:
		case 6:
		case 9:
		case 11:
			if(CurrentDate > 30)
				return (-3);
			break;
		default:
			return (-2);
	}

	CurrentDate++;
	switch(CurrentMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if( CurrentDate > 31 )
			{
				CurrentDate = 1;
				CurrentMonth++;
			}
			if( CurrentMonth > 12 )
			{
				CurrentMonth = 1;
				CurrentYear++;
			}
			break;
		case 2:/*当为3月1日时,需要知道当年是否润年,来决定上一日为28或29日*/
			if( (CurrentYear%400 == 0) || (CurrentYear%4 == 0) )
			{
				if( CurrentDate > 29 )
				{
					CurrentDate = 1;
					CurrentMonth++;
				}
			}
			else
			{
				if( CurrentDate > 28 )
				{
					CurrentDate = 1;
					CurrentMonth++;
				}
			}
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if( CurrentDate > 30 )
			{
				CurrentDate = 1;
				CurrentMonth++;
			}
			break;
		default:
			return (-1);
	}
	
	pTomorrowTime->year = CurrentYear;
	pTomorrowTime->month = CurrentMonth;
	pTomorrowTime->day =  CurrentDate;
	pTomorrowTime->hour = (unsigned short)CurrentHour;
	pTomorrowTime->minute = (unsigned char)CurrentMintue;
	pTomorrowTime->second = (unsigned char)CurrentSecond;
	VOS_MemZero(pTomorrowTime->reserver, 4);
	
	/*sys_console_printf(" yestoday date: %d-%d-%d %d:%d:%d\r\n",pYestodayTime->year,
						pYestodayTime->month,pYestodayTime->day,
						pYestodayTime->hour, pYestodayTime->minute,
						pYestodayTime->second);*/
	return VOS_OK;
}

/* modified by xieshl 20110718, 问题单13070 */
extern int GetSlotCardPonPortNum(int module_type, int CardIndex);
extern LONG typesdb_slot_is_master_slot( ULONG slotno );
static BOOL alm_onu_idx_is_illegal( ULONG slotno, ULONG portno, ULONG onuid )
{
	ULONG portnum = 0;
	if( (slotno == 0) || (slotno > SYS_CHASSIS_SWITCH_SLOTNUM) )
		return TRUE;
	if( (onuid == 0) || (onuid > MAXONUPERPON) )
		return TRUE;

	if( SYS_MODULE_IS_PON(slotno) )
		portnum = CARD_MAX_PON_PORTNUM/*GetSlotCardPonPortNum(slotno)*/;
	else
	{
		if( !SYS_CHASSIS_SLOTNO_ISMASTERSLOTNO(slotno) && (__SYS_MODULE_TYPE__(slotno) == MODULE_TYPE_NULL) )
			portnum = CARD_MAX_PON_PORTNUM;
	}
	if( (portno == 0) || (portno > portnum) )
		return TRUE;

	return FALSE;
}

extern int CTC_OnuModel_Translate(unsigned long model, unsigned long *type, char *pModelStr, int *pLen);
extern int CTC_OnuModelStr_2_Type( char *pModelStr, unsigned long *pOnuType );
extern char * CTC_OnuModelType_2_Str( unsigned long onuType );

ULONG  onutypestring_to_int( char *pOnuTypeStr )
{
	ULONG  i = VOS_ERROR;
	/*UCHAR ctc_onu_model_str[80];
	int len = 0;*/
	
	if( pOnuTypeStr == NULL )
		return VOS_ERROR;
	
	for(i = V2R1_ONU_GT811; i < V2R1_ONU_MAX; i++)
	{
		if( VOS_StriCmp(pOnuTypeStr, GetDeviceTypeString(i)) == 0 )
		{
			return i;
		}
	}
#if 0
	/* modified by xieshl 20120927, 因历史原因，GT811和GT811_C是公用的同一类型，都使用V2R1_ONU_GT811，问题单15082/15896 */
	if( CTC_OnuModel_Translate(CTC_GT811_C_MODEL, &i, ctc_onu_model_str, &len) == VOS_OK )
	{
		/*if( VOS_StriCmp(ctc_onu_model_str, onutype) == 0 )*/
		if( i == V2R1_ONU_GT811_C )
			i = V2R1_ONU_GT811;
	}
	else
		i = VOS_ERROR;
#else
	if( CTC_OnuModelStr_2_Type(pOnuTypeStr, &i) == VOS_ERROR )
	{
		i = VOS_ERROR;
	}
	if(i== VOS_ERROR)
	{
		if(GPON_OnuEquipmentStr_2_Type(pOnuTypeStr, &i) == VOS_ERROR)
		{
			i = VOS_ERROR;
		}
	}
#endif

	return  i;
}

CHAR *onutypeint_to_str( ULONG onuType )
{
	CHAR *pOnuTypeStr = CTC_OnuModelType_2_Str(onuType);
	
	if( pOnuTypeStr == NULL )
	{
		/*added by yanjy2016-12*/
		pOnuTypeStr = GPON_OnuType_2_Str(onuType);
		if( pOnuTypeStr == NULL )
			pOnuTypeStr = GetDeviceTypeString(onuType);
	}
	return pOnuTypeStr;
}


/* modified by xieshl 20080505, 问题单#6636 */
/* modified by xieshl 20080526, 问题单#6723 */
static char SHOW_EVENT_LOG_CMD_STR[200];/* = "show alarm log {[device|name] <value>}*1 {[class] <1-5>}*1 {[trap] <1-55>}*1 {[start-time] [yestoday|today|<start_time>]}*1 {[end-time] <end_time>}*1";*/
const char *parameter_str[] = {
		"device",
		"name",
		"class",
		"trap",
		"start-time",
		"end-time" };
const int parameter_num = 6;

/* added by xieshl 20120528, 统一设备索引解析函数，问题单14323 */
LONG alm_device_index_parse( struct vty *vty, CHAR *dev_argv, ULONG *pDevIdx )
{
	ULONG devIdx = 0;
	ULONG slotno, portno, onuid;
	LONG rc = VOS_OK;
	
	if( (dev_argv == NULL) || (pDevIdx == NULL) )
		return VOS_ERROR;
	if( VOS_CheckNumString(dev_argv, VOS_StrLen(dev_argv)) == VOS_OK )
	{
		devIdx = VOS_AtoL(dev_argv );
	}
	else if( PON_ParseSlotPortOnu( dev_argv, &slotno, &portno, &onuid ) == VOS_OK )
	{
		devIdx = MAKEDEVID( slotno, portno, onuid );
	}
	else
		rc = VOS_ERROR;

	if( devIdx > OLT_DEV_ID )
	{
		slotno = GET_PONSLOT(devIdx);
		portno = GET_PONPORT(devIdx);
		onuid = GET_ONUID(devIdx);
		
		if( alm_onu_idx_is_illegal(slotno, portno, onuid) )
			rc = VOS_ERROR;
	}
	
	if( (rc == VOS_ERROR) || (devIdx == 0) )	/* 问题单14323 */
	{
		vty_out( vty, "The device index is error\r\n" );
		rc = VOS_ERROR;
	}
	else
	{
		*pDevIdx = devIdx;
	}
	return rc;
}

QDEFUN( show_event_log,
	show_event_log_cmd,
	/*"show alarm log {[device|name] <device_index>}*1 {[class] <1-5>}*1 {[trap] <1-55>}*1 {[start-time] [yestoday|today|<start_time>]}*1 {[end-time] <end_time>}*1",*/
	SHOW_EVENT_LOG_CMD_STR,
	"Show information\n"
	"Show alarm log information\n"
	"Show system alarm log information\n"
	"Specify the device index\n"
	"Specify the device name\n"
	"Input the device-index(eg. 1-olt device index,12003 or 1/2/3-onu device index) or device name(eg. olt,OLT name,ONU name)\n"
	"Specify the alarm level\n"
	"Input alarm level: 1-vital,2-major,3-minor,4-warning,5-notification\n"
	"Specify alarm trap identification defined by MIB\n"
	"Input the alarm trap ID\n"
	"Specify the alarm start time\n"
	"Select the start time is from 00:00:00 of yestoday\n"
	"Select the start time is from 00:00:00 of today\n"
	"Input the start time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS\n"
	"Specify the alarm end time\n"
	"Input the end time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS\n",
	&eventQueId )
{
	ULONG devIdx = 0;
	ULONG almLevel = 0;
	ULONG almId = 0;
	/*ULONG ulSlot, ulPort, ulOnuid;*/
	ushort_t slot, port, onuid;
	int len = 0;
	char devName[256] = {0};
	sysDateAndTime_t *pStartTime = NULL;
	sysDateAndTime_t *pEndTime = NULL;
	sysDateAndTime_t startTime;
	sysDateAndTime_t endTime;
	int i, j/*, k,value*/;

	if( argc > 1 )
	{
		j = 0;
		for( i=0; i<(argc-1); i++ )
		{
			for( ; j<parameter_num; j++ )
			{
				if( VOS_StriCmp(argv[i], parameter_str[j]) == 0 )
					break;
			}
			if( j >= parameter_num )
				break;
				
			switch( j )
			{
				case 0:		/* device */
					/*当关键字是device时，后面的显示类容可以是OLT的或ONU 的告警*/
					i++;

#if 0	/* modified by xieshl 20120528, 统一设备索引解析函数，问题单14323 /11349 */
					if( VOS_CheckNumString(argv[i], VOS_StrLen(argv[i])) == VOS_OK )
					{
						devIdx = VOS_AtoL(argv[i] );
						if( devIdx > OLT_DEV_ID )
						{
							ulSlot = GET_PONSLOT(devIdx);
							ulPort = GET_PONPORT(devIdx);
							ulOnuid = GET_ONUID(devIdx);
						}
					}
					else if( PON_ParseSlotPortOnu( argv[i], &ulSlot, &ulPort, &ulOnuid ) == VOS_OK )
					{
						devIdx = MAKEDEVID( ulSlot, ulPort, ulOnuid );
					}
					if( (devIdx == 0) || ((devIdx > OLT_DEV_ID) && alm_onu_idx_is_illegal(ulSlot, ulPort, ulOnuid)) )
					{
						vty_out( vty, "The device index is error\r\n" );
						return CMD_WARNING;
					}
#else
					if( alm_device_index_parse(vty, argv[i], &devIdx) == VOS_ERROR )
						return CMD_WARNING;
#endif
					break;
					
				case 1:		/* name */
					i++;
					len = VOS_StrLen(argv[i]);
					if ( len >= MAXDEVICENAMELEN )
					{
						vty_out( vty, "  %% Parameter error. The device name must be less than %d.\r\n", MAXDEVICENAMELEN );
						return CMD_WARNING;
					}
					if( VOS_StrCmp(argv[i], "olt") == 0 )
					{
						devIdx = 1;
					}
					else
					{
						GetOltDeviceName( devName, &len );
						if( VOS_StrCmp(argv[i], devName) == 0 )
						{
							devIdx = 1;
						}
						else if ( GetOnuDeviceIdxByName( argv[i], len, &slot, &port, &onuid ) == VOS_OK )
						{
							devIdx = MAKEDEVID( slot, port, (onuid + 1) );
						}
						else
						{
							vty_out( vty, "  %% Not exist Onu name!\r\n");
							return CMD_WARNING;
						}
					}
					/*sys_console_printf("\r\n name: devidx=%d\r\n", devIdx);*/
					break;
				case 2:		/* class */
					i++;
					almLevel = (ULONG)VOS_AtoL(argv[i]);
					break;
				case 3:		/* trap */
					i++;
					almId = (ULONG)VOS_AtoL(argv[i]);
					break;
				case 4:		/* start-time */
					i++;
					VOS_MemZero( &startTime, sizeof(sysDateAndTime_t) );
					if( VOS_StrCmp( argv[i], "today") == 0 )
					{
						if (VOS_OK != getTodayTime( &startTime ) )
						{
							vty_out( vty, "  %% Get today time error.\r\n");
							return CMD_WARNING;
						}
						pStartTime = &startTime;
					}
					else if( VOS_StrCmp( argv[i], "yestoday") == 0 )
					{
						if (VOS_OK != getTodayTime( &endTime ) )
						{
							vty_out( vty, "  %% Get today time error.\r\n");
							return CMD_WARNING;
						}
						if (VOS_OK != getYestodayTime( &endTime, &startTime ) )
						{
							vty_out( vty, "  %% Get yestoday time error.\r\n");	
							return CMD_WARNING;
						}
						pStartTime = &startTime;
					}
					else
					{
						pStartTime = strToSysDateAndTime( argv[i], &startTime );
						if( checkDateAndTime(vty, pStartTime) != VOS_OK )
							return CMD_WARNING;
					}
					break;
				case 5:		/* end-time */
					i++;
					VOS_MemZero( &endTime, sizeof(sysDateAndTime_t) );
					pEndTime = strToSysDateAndTime( argv[i], &endTime );

					if( checkDateAndTime(vty, pEndTime) != VOS_OK )
						return CMD_WARNING;

					break;
				default:
					break;
			}
		}
	}

	showEventLogCmd( vty, devIdx, almLevel, almId, pStartTime, pEndTime );

	/*vty_out( vty, "\r\n argc=%d\r\n", argc );
	for( i=0; i<argc; i++ )
		vty_out( vty, " argv[%d]=%s\r\n", i, argv[i] );*/
		
    return CMD_SUCCESS;
}
/*add by shixh20090420*/
/* modified by xieshl 20110216, 问题单12065 */
QDEFUN( show_import_event_log,
	show_import_log_cmd,
	"show alarm log important {[device|name] <value>}*1 {[start-time] [yestoday|today|<start_time>]}*1 {[end-time] <end_time>}*1",
	"Show information\n"
	"Show alarm information\n"
	"Show alarm log information\n"
	"Show system important alarm log information\n"
	"Specify the device index\n"
	"Specify the device name\n"
	"Input the device-index(eg. 1,11001,1/1/1) or device name(eg. olt,OLT name,ONU name)\n"
	"Specify the alarm start time\n"
	"Select the start time is from 00:00:00 of yestoday\n"
	"Select the start time is from 00:00:00 of today\n"
	"Input the start time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS\n"
	"Specify the alarm end time\n"
	"Input the end time, format:YYYY-MM-DD,HH:MM:SS or YY-MM-DD,HH:MM:SS\n",
	&eventQueId)
{
	ULONG devIdx = 0;
	/*ULONG ulSlot, ulPort, ulOnuid;*/
	ushort_t slot, port, onuid;
	int len = 0;
	char devName[256] = {0};
	sysDateAndTime_t *pStartTime = NULL;
	sysDateAndTime_t *pEndTime = NULL;
	sysDateAndTime_t startTime;
	sysDateAndTime_t endTime;
	int i, j;

	if( argc > 1 )
	{
		j = 0;
		for( i=0; i<(argc-1); i++ )
		{
			for( ; j<parameter_num; j++ )
			{
				if( VOS_StriCmp(argv[i], parameter_str[j]) == 0 )
					break;
			}
			if( j >= parameter_num )
				break;
				
			switch( j )
			{
				case 0:		/* device */
					/*当关键字是device时，后面的显示类容可以是OLT的或ONU 的告警*/
					i++;
#if 0
					if( VOS_CheckNumString(argv[i], VOS_StrLen(argv[i])) == VOS_OK )
					{
						devIdx = VOS_AtoL(argv[i] );
						if( devIdx > OLT_DEV_ID )
						{
							ulSlot = GET_PONSLOT(devIdx);
							ulPort = GET_PONPORT(devIdx);
							ulOnuid = GET_ONUID(devIdx);
						}
					}
					else if( PON_ParseSlotPortOnu( argv[i], &ulSlot, &ulPort, &ulOnuid ) == VOS_OK )
					{
						devIdx = MAKEDEVID( ulSlot, ulPort, ulOnuid );
					}
					
					if( (devIdx > OLT_DEV_ID) && alm_onu_idx_is_illegal(ulSlot, ulPort, ulOnuid) )
					{
						vty_out( vty, "The device index is error\r\n" );
						return CMD_WARNING;
					}
#else
					if( alm_device_index_parse(vty, argv[i], &devIdx) == VOS_ERROR )
						return CMD_WARNING;
#endif
					break;
					
				case 1:		/* name */
					i++;
					len = VOS_StrLen(argv[i]);
					if ( len >= MAXDEVICENAMELEN )
					{
						vty_out( vty, "  %% Parameter error. The device name must be less than %d.\r\n", MAXDEVICENAMELEN );
						return CMD_WARNING;
					}
					if( VOS_StrCmp(argv[i], "olt") == 0 )
					{
						devIdx = 1;
					}
					else
					{
						GetOltDeviceName( devName, &len );
						if( VOS_StrCmp(argv[i], devName) == 0 )
						{
							devIdx = 1;
						}
						else if ( GetOnuDeviceIdxByName( argv[i], len, &slot, &port, &onuid ) == VOS_OK )
						{
							devIdx = MAKEDEVID( slot, port, (onuid + 1) );
						}
						else
						{
							vty_out( vty, "  %% Not exist Onu name!\r\n");
							return CMD_WARNING;
						}
					}
					/*sys_console_printf("\r\n name: devidx=%d\r\n", devIdx);*/
					break;
				case 2:		/* class */
				case 3:		/* trap */
					i++;
					break;
				case 4:		/* start-time */
					i++;
					VOS_MemZero( &startTime, sizeof(sysDateAndTime_t) );
					if( VOS_StrCmp( argv[i], "today") == 0 )
					{
						if (VOS_OK != getTodayTime( &startTime ) )
						{
							vty_out( vty, "  %% Get today time error.\r\n");
							return CMD_WARNING;
						}
						pStartTime = &startTime;
					}
					else if( VOS_StrCmp( argv[i], "yestoday") == 0 )
					{
						if (VOS_OK != getTodayTime( &endTime ) )
						{
							vty_out( vty, "  %% Get today time error.\r\n");
							return CMD_WARNING;
						}
						if (VOS_OK != getYestodayTime( &endTime, &startTime ) )
						{
							vty_out( vty, "  %% Get yestoday time error.\r\n");	
							return CMD_WARNING;
						}
						pStartTime = &startTime;
					}
					else
					{
						pStartTime = strToSysDateAndTime( argv[i], &startTime );
						if( checkDateAndTime(vty, pStartTime) != VOS_OK )
							return CMD_WARNING;
					}
					break;
				case 5:		/* end-time */
					i++;
					VOS_MemZero( &endTime, sizeof(sysDateAndTime_t) );
					pEndTime = strToSysDateAndTime( argv[i], &endTime );

					if( checkDateAndTime(vty, pEndTime) != VOS_OK )
						return CMD_WARNING;

					break;
				default:
					break;
			}
		}
	}

	if( showEventLogImportCmd( vty, devIdx, 0, 0, pStartTime, pEndTime ) == VOS_OK )
		return CMD_SUCCESS;
	return CMD_WARNING;
}

extern ULONG eventLogOutMode_bak;
QDEFUN( config_alarmlog_out_mode,
       config_alarmlog_out_mode_cmd,
       "config alarm-log out-mode {[all|console|telnet|syslog]}*1",
	"config alarm log display mode\n"
	"config alarm log display mode\n"
	"config alarm log display mode\n"
	"display alarm log in console/telnet,syslog\n"
	"display alarm log in console\n"
	"display alarm log in telnet\n"
	"display alarm log in syslog\n",
     	&eventQueId)
{
	if( argc == 1 )
	{
		if ( VOS_StrCmp( argv[0], "all" ) == 0 )
		{
			eventLogOutMode = EVENT_LOG_OUT_2_ALL;
		}
		else if ( VOS_StrCmp( argv[0], "telnet" ) == 0 )
		{
			eventLogOutMode |= EVENT_LOG_OUT_2_TELNET;
		}
		else if ( VOS_StrCmp( argv[0], "console") == 0 )
		{
			eventLogOutMode |= EVENT_LOG_OUT_2_CONSOLE;
		}
		else if ( VOS_StrCmp( argv[0], "syslog") == 0 )
		{
			eventLogOutMode |= EVENT_LOG_OUT_2_SYSLOG;
		}
		eventLogOutMode_bak = eventLogOutMode;
	}
	else
	{
		vty_out( vty, "alarm log out mode:" );
		if( (eventLogOutMode & EVENT_LOG_OUT_2_ALL) == 0 )
			vty_out( vty, "none\r\n" );
		else
		{
			if( eventLogOutMode & EVENT_LOG_OUT_2_TELNET )
				vty_out( vty, "telnet " );
			if( eventLogOutMode & EVENT_LOG_OUT_2_CONSOLE )
				vty_out( vty, "console " );
			if( eventLogOutMode & EVENT_LOG_OUT_2_SYSLOG )
				vty_out( vty, "syslog " );
			vty_out( vty, "\r\n" );
		}
	}
		
	return CMD_SUCCESS;
}
QDEFUN( undo_alarmlog_out_mode,
       undo_alarmlog_out_mode_cmd,
       "undo alarm-log out-mode [all|console|telnet|syslog]",
	"undo alarm log display mode\n"
	"undo alarm log display mode\n"
	"undo alarm log display mode\n"
	"display alarm log in console/telnet,syslog\n"
	"display alarm log in console\n"
	"display alarm log in telnet\n"
	"display alarm log in syslog\n",
     	&eventQueId)
{
	if( argc == 1 )
	{
		if ( VOS_StrCmp( argv[0], "all" ) == 0 )
		{
			eventLogOutMode = EVENT_LOG_OUT_2_NONE;
		}
		else if ( VOS_StrCmp( argv[0], "telnet" ) == 0 )
		{
			eventLogOutMode &= (~EVENT_LOG_OUT_2_TELNET);
		}
		else if ( VOS_StrCmp( argv[0], "console") == 0 )
		{
			eventLogOutMode &= (~EVENT_LOG_OUT_2_CONSOLE);
		}
		else if ( VOS_StrCmp( argv[0], "syslog") == 0 )
		{
			eventLogOutMode &= (~EVENT_LOG_OUT_2_SYSLOG);
		}
		eventLogOutMode_bak = eventLogOutMode;
	}
	return CMD_SUCCESS;
}

/*add by shixh20090927,alarm log到syslog使能，默认为不打开，问题单8544*/
QDEFUN( alarmLog_to_sysLog_enable,
         alarmLog_to_sysLog_enable_cmd,
         "alarmlog-to-syslog [enable|disable]",
	"config alarm log to syslog enable\n"
	 "enable alarm log to syslog\n"
	  "disable  alarm log to syslog\n",
     	&eventQueId)
{
	if ( !VOS_StrCmp( argv[0], "enable" ) )
	{
		/*alarmlogToSyslog_enable= 1;*/
		eventLogOutMode |= EVENT_LOG_OUT_2_SYSLOG;
	}
	else /*if ( !VOS_StrCmp( argv[0], "disable" ) )*/
	{
		/*alarmlogToSyslog_enable= 0;*/
		eventLogOutMode &= (~EVENT_LOG_OUT_2_SYSLOG);
	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarmLog_to_sysLog_enable,
       show_alarmLog_to_sysLog_enable_cmd,
       "show alarmlog-to-syslog-enable",
	"Show information\n"
	"Show alarm log to syslog enable\n",
     	&eventQueId)
{
	/*if( alarmlogToSyslog_enable== 1 )*/
	if( eventLogOutMode & EVENT_LOG_OUT_2_SYSLOG )
		vty_out(vty,"alarm log to syslog is enable!\r\n");
	else
		vty_out(vty,"alarm log to syslog is disable!\r\n");
	
	return CMD_SUCCESS;
}

/*end by shixh20090927*/
QDEFUN( show_special_date_event_log,
	show_special_date_event_log_cmd,
	"show alarm log [today|yestoday]",
	"Show information\n"
	"Show alarm log information\n"
	"Show system alarm log information\n"
	"Show system alarm log information of today\n"
	"Show system alarm log information of yestoday\n",
	&eventQueId)
{
	ULONG devIdx = 0;
	ULONG almLevel = 0;
	ULONG almId = 0;
	/* modified by xieshl 20100203, 只取today当天或从yestoday 开始至今天范围内的告警日志 */
	sysDateAndTime_t todayTime, yestodayTime;
	sysDateAndTime_t endTime;
	memset( &todayTime, 0, sizeof(sysDateAndTime_t) );
	memset( &yestodayTime, 0, sizeof(sysDateAndTime_t) );
	if (VOS_OK != getTodayTime( &todayTime ) )
	{
		vty_out( vty, "  %% Get system time error.\r\n");
		return CMD_WARNING;
	}
	endTime.year = todayTime.year;
	endTime.month = todayTime.month;
	endTime.day = todayTime.day;
	endTime.hour = 23;
	endTime.minute = 59;
	endTime.second = 59;
	if ( !VOS_StrCmp( argv[ 0 ], "today" ) )
	{
		showEventLogCmd( vty, devIdx, almLevel, almId, &todayTime, &endTime );
	}
	else /*if ( !VOS_StrCmp( argv[ 0 ], "yestoday" ) )*/
	{
		if (VOS_OK == getYestodayTime( &todayTime, &yestodayTime ) )
		{
			showEventLogCmd( vty, devIdx, almLevel, almId, &yestodayTime, &endTime );		
		}
		else
			return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

LONG show_alarm_mask_title( struct vty *vty, LONG titleFlag )
{
	if( titleFlag )
	{
		titleFlag = 0;
		vty_out( vty, " All alarm mask list:\r\n" );
		vty_out( vty, " %-20s%s\r\n", "Mask-name", "Mask-list" );
		vty_out( vty, "-------------------------------\r\n" );
	}
	
	return titleFlag;
}

/* added by xieshl 20110415, PR008808 */
LONG show_all_alarm_mask( struct vty *vty )
{
	ULONG devIdx, brdIdx, portIdx;
	ULONG nextDevIdx, nextBrdIdx, nextPortIdx;
	ULONG mask;
	ULONG slot, pon, onu;
	ULONG onuType;
	CHAR *pOnuTypeStr;
	LONG  titleFlag = 1;
	CHAR str[256];

	if( getFirstDeviceEntryIndex(&nextDevIdx) == VOS_OK )
	{
		do
		{
			devIdx = nextDevIdx;
			mask = 0;
			if( getDeviceAlarmMask(devIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_DEV_ALL;
				if( mask  != 0 )
				{
					titleFlag = show_alarm_mask_title( vty, titleFlag );
					
					if( devIdx == OLT_DEV_ID )
					{
						VOS_StrCpy( str, "olt-device" );
					}
					else
					{
						slot = GET_PONSLOT(devIdx);
						pon = GET_PONPORT(devIdx);
						onu = GET_ONUID(devIdx);

						VOS_Sprintf( str, "onu-device %d/%d/%d", slot, pon, onu ) ;
					}
					vty_out( vty, " %-19s", str );
					vty_out( vty, " %s\r\n", alarm_dev_mask_bits_to_str(mask, str) ) ;
				}
			} 
		}while( getNextDeviceEntryIndex(devIdx, &nextDevIdx) == VOS_OK );
	}

	if( (alarm_mask_pon_cni_bits & 1) == 0 )
	{
		titleFlag = show_alarm_mask_title( vty, titleFlag );
		vty_out( vty, " %-19s %s", "pon-cni", "ber\r\n" );
	}
	if( getFirstPonPortEntryIndex(&nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK )
	{
		do {
			devIdx = nextDevIdx;
			brdIdx = nextBrdIdx;
			portIdx = nextPortIdx;
			mask = 0;
			if( getPonPortAlarmMask(devIdx, brdIdx, portIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_PON_ALL;
				if( mask != 0 )
				{
					titleFlag = show_alarm_mask_title( vty, titleFlag );

					if( devIdx == OLT_DEV_ID )
					{
						VOS_Sprintf( str, "olt-pon %d/%d", brdIdx, portIdx ) ;
					}
					else
					{
						slot = GET_PONSLOT(devIdx);
						pon = GET_PONPORT(devIdx);
						onu = GET_ONUID(devIdx);
						VOS_Sprintf( str, "onu-pon %d/%d/%d", slot, pon, onu ) ;
					}
					vty_out( vty, " %-19s", str );
					vty_out( vty, " %s\r\n", alarm_pon_mask_bits_to_str(mask, str) );
				}
			}
		} while( getNextPonPortEntryIndex(devIdx, brdIdx, portIdx, &nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK );
	}

	if( getFirstEthPortEntryIndex(&nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK )
	{
		do
		{
			if( nextDevIdx != 1 )	/* 这里只处理OLT侧ETH PORT */
				break;
			
			devIdx = nextDevIdx;
			brdIdx = nextBrdIdx;
			portIdx = nextPortIdx;
			mask = 0;
			if( getEthPortAlarmMask(devIdx, brdIdx, portIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_ETH_ALL;
				if( mask != 0 )
				{
					if( devIdx == OLT_DEV_ID )
					{
						titleFlag = show_alarm_mask_title( vty, titleFlag );

						VOS_Sprintf( str, "olt-eth %d/%d", brdIdx, portIdx ) ;
						vty_out( vty, " %-19s", str );
						vty_out( vty, " %s\r\n", alarm_eth_mask_bits_to_str(mask, str) );
					}
				}
			}
		} while( getNextEthPortEntryIndex(devIdx, brdIdx, portIdx, &nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK );
	}

	if( getOnuEthPortAlarmMask(&mask) == VOS_OK )
	{
		if( mask & EVENT_MASK_ETH_ALL )
		{
			titleFlag = show_alarm_mask_title( vty, titleFlag );
			vty_out( vty, " %-19s %s\r\n", "onu-eth", alarm_eth_mask_bits_to_str(mask, str) );
		}
	}

	for( onuType=V2R1_ONU_GT811; onuType<V2R1_ONU_MAX; onuType++ )
	{
		mask = 0;
		if( getOnuTypeAlarmMask( onuType, &mask ) == VOS_OK )
		{
			mask &= EVENT_MASK_DEV_ALL;
			if( mask )
			{
				/*pOnuTypeStr = GetDeviceTypeString(onuType);
				if( VOS_StrCmp(pOnuTypeStr, DEVICE_TYPE_NAME_UNKNOWN_STR) == 0 )
					continue;*/
				pOnuTypeStr = onutypeint_to_str(onuType);
				if( (pOnuTypeStr == NULL) || (VOS_StrCmp(pOnuTypeStr, DEVICE_TYPE_NAME_UNKNOWN_STR) == 0) )
					continue;

				titleFlag = show_alarm_mask_title( vty, titleFlag );

				VOS_Sprintf( str, "onu-type %s", pOnuTypeStr ) ;
				vty_out( vty, " %-19s", str );
				vty_out( vty, " %s\r\n", alarm_dev_mask_bits_to_str(mask, str) ) ;
			}
		}
	}

	if( titleFlag )
	{
		vty_out( vty, " Alarm mask list:null\r\n" );
	}
	vty_out( vty, "\r\n" );
	
	return CMD_SUCCESS;
}


QDEFUN( show_event_config,
	show_event_config_cmd,
	"show alarm configuration",
	"Show alarm information\n"
	"Show alarm configuration information\n"
	"Show alarm log and syn. configuration\n",
	&eventQueId )
{
	ULONG enable = 0;
	char *en_str[] = { "enable", "disable" };

	if( argc >= 1 )
		return CMD_ERR_EXEED_ARGC_MAX;

	getEventLogEnable( &enable );
	vty_out( vty, " alarm-log service is %s\r\n", (enable == EVENTLOG_ENABLE) ? en_str[0] : en_str[1] );

	getTrapBacEnable( &enable );
	vty_out( vty, " trap synchronization service is %s\r\n", (enable == TRAP_BAC_ENABLE) ? en_str[0] : en_str[1] );

	/*enable = alarmlogToSyslog_enable;
	vty_out( vty, " alarmlog-to-syslog %s\r\n", (enable == EVENTLOG_ENABLE) ? en_str[0] : en_str[1] ) ;*/
	vty_out( vty, " alarm-log to-syslog %s\r\n", (eventLogOutMode & EVENT_LOG_OUT_2_SYSLOG) ? en_str[0] : en_str[1] ) ;
	vty_out( vty, " alarm-log to-console %s\r\n", (eventLogOutMode & EVENT_LOG_OUT_2_CONSOLE) ? en_str[0] : en_str[1] ) ;
	vty_out( vty, " alarm-log to-telnet %s\r\n", (eventLogOutMode & EVENT_LOG_OUT_2_TELNET) ? en_str[0] : en_str[1] ) ;

	if( eventLogFlashFileFlag )	/* modied by xieshl 20130320, 支持保存flash */
	{
		vty_out( vty, " config alarm-log flashfile %s\r\n", (nvramEventBackupEnableGet() == 1) ? en_str[0] : en_str[1] );
		vty_out( vty, " config alarm-log flashfile period %d minutes\r\n", nvramEventBackupPeriodGet()/60 );
	}

	if( getEventLogDataFilter() & ALM_FILTER_ONU_ETH_LINK )
	{
		vty_out( vty, " alarm-log filter onu-eth-link\r\n" );
	}
	vty_out( vty, "\r\n" );

	/* modified by xieshl 20110415, PR008808 */
	show_all_alarm_mask( vty );
	
	return CMD_SUCCESS;
}

/* modified by xieshl 20080424, 为了和syslog命令格式一致，同时便于扩展告警日志过滤命令定义，调整该命令 */
QDEFUN( event_log_enable,
	event_log_enable_cmd,
	"config alarm-log [enable|disable]",
	"Config alarm's setting\n"
	"Config alarm log service\n"
	"Set alarm log service enable\n"
	"Set alarm log service disable\n",
	&eventQueId)
{
	if( argc != 1 )
		return CMD_ERR_EXEED_ARGC_MAX;
	else
	{
		/*if( VOS_MemCmp(argv[0], "enable", 6) == 0 )*/
		if( argv[0][0] == 'e' )
			setEventLogEnable( EVENTLOG_ENABLE );
		else /*if( VOS_MemCmp(argv[0], "disable", 7) == 0 )*/
			setEventLogEnable( EVENTLOG_DISABLE );
		/*else
			return CMD_ERR_NO_MATCH;*/
	}  
	return CMD_SUCCESS;
}

QDEFUN( event_show_curalarm,
       event_show_curalarm_cmd,
       "show current alarms {[level] <1-3>}*1 {[device] <device_index>}*1",
	"Show information\n"
	"Show alarm information\n"
	"Show current alarm information\n"
	"Show current alarm level info.\n"
	"Input level:1-vital,2-major,3-minor\n"
	"Specify the device index\n"
	"Input the device-index, eg. 1-olt device index,12003 or 1/2/3-onu device index\n",
       &eventQueId  )
{
	/*ULONG ulSlot, ulPort, ulOnuid;*/
	ULONG devIdx = 0;
	ULONG level = 0;
	int i;
	
	if( argc == 0 )
	{
	}
	else if( (argc == 2) || (argc == 4) )	/* modified by xieshl 20110615, 问题单13025*/
	{
		for( i=0; i<argc; i++ )
		{
			if( VOS_StrCmp(argv[i], "level") == 0 )
			{
				i++;
				level = VOS_AtoL(argv[i] );
			}
			else
			{
				i++;
#if 0	/* modified by xieshl 20120528, 统一设备索引解析函数，问题单14323 */
				if( VOS_CheckNumString(argv[i], VOS_StrLen(argv[i])) == VOS_OK )
				{
					devIdx = VOS_AtoL(argv[i] );
					if( devIdx > OLT_DEV_ID )
					{
						ulSlot = GET_PONSLOT(devIdx);
						ulPort = GET_PONPORT(devIdx);
						ulOnuid = GET_ONUID(devIdx);
					}
				}
				else if( PON_ParseSlotPortOnu( argv[i], &ulSlot, &ulPort, &ulOnuid ) == VOS_OK )
				{
					devIdx = MAKEDEVID( ulSlot, ulPort, ulOnuid );
				}
				else
					devIdx = OLT_DEV_ID + 1;	/* modified by xieshl 20120528, 问题单14323 */

				if( (devIdx > OLT_DEV_ID) && alm_onu_idx_is_illegal(ulSlot, ulPort, ulOnuid) )
				{
					vty_out( vty, "The device index is error\r\n" );
					return CMD_WARNING;
				}
#else
				if( alm_device_index_parse(vty, argv[i], &devIdx) == VOS_ERROR )
					return CMD_WARNING;
#endif
			}
		}
	}
	else
	{
		return CMD_ERR_AMBIGUOUS;
	}

	showCurrentAlarmStatus( vty, level, devIdx );

	return CMD_SUCCESS;
}

QDEFUN( event_clear_curalarm,
       event_clear_curalarm_cmd,
       "clear current alarms {[level] <1-3>}*1 {[device] <device_index>}*1",
	"Clear information\n"
	"Clear alarm information\n"
	"Clear current alarm level info.\n"
	"Input level:1-vital,2-major,3-minor\n"
	"Specify the device index\n"
	"Input the device-index, eg. 1-olt device index,12003 or 1/2/3-onu device index\n",
       &eventQueId )
{
	/*ULONG ulSlot, ulPort, ulOnuid;*/
	ULONG devIdx = 0;
	ULONG level = ALM_LEV_NULL;
	int i;
	
	if( argc == 0 )
	{
		/*level = ALM_LEV_NULL;*/
	}
	else if( (argc == 2) || (argc == 4) )	/* modified by xieshl 20110615, 问题单13025*/
	{
		for( i=0; i<argc; i++ )
		{
			if( VOS_StrCmp(argv[i], "level") == 0 )
			{
				i++;
				level = VOS_AtoL(argv[i] );
			}
			else
			{
				i++;
#if 0	/* modified by xieshl 20120528, 统一设备索引解析函数，问题单14323 */
				if( VOS_CheckNumString(argv[i], VOS_StrLen(argv[i])) == VOS_OK )
				{
					devIdx = VOS_AtoL(argv[i] );
					if( devIdx > OLT_DEV_ID )
					{
						ulSlot = GET_PONSLOT(devIdx);
						ulPort = GET_PONPORT(devIdx);
						ulOnuid = GET_ONUID(devIdx);
					}
				}
				else if( PON_ParseSlotPortOnu( argv[i], &ulSlot, &ulPort, &ulOnuid ) == VOS_OK )
				{
					devIdx = MAKEDEVID( ulSlot, ulPort, ulOnuid );
				}

				if( (devIdx > OLT_DEV_ID) && alm_onu_idx_is_illegal(ulSlot, ulPort, ulOnuid) )
				{
					vty_out( vty, "The device index is error\r\n" );
					return CMD_WARNING;
				}
#else
				if( alm_device_index_parse(vty, argv[i], &devIdx) == VOS_ERROR )
					return CMD_WARNING;
#endif
			}
		}
	}
	else
	{
		return CMD_ERR_AMBIGUOUS;
	}

	clearDevAlarmStatus( vty, level, devIdx );

	return CMD_SUCCESS;
}

/* modified by xieshl 20080424, 为了和syslog命令格式一致，同时便于扩展告警日志过滤命令定义，调整该命令 */
QDEFUN( event_syn_enable,
         event_syn_enable_cmd,
         "config alarm-synchronization [enable|disable]",
	  "Config alarm's setting\n"
	  "Config snmp trap synchronization service\n"
         "Set trap synchronization service enable\n"
         "Set trap synchronization service disable\n",
     	&eventQueId)
{
	if( argc != 1 )
		return CMD_ERR_EXEED_ARGC_MAX;
	else
	{
		/*if( VOS_MemCmp(argv[0], "enable", 6) == 0 )*/
		if( argv[0][0] == 'e' )
			setTrapBacEnable( TRAP_BAC_ENABLE );
		else /*if( VOS_MemCmp(argv[0], "disable", 7) == 0 )*/
			setTrapBacEnable( TRAP_BAC_DISABLE );
		/*else
			return CMD_ERR_NO_MATCH;*/
	}  
	return CMD_SUCCESS;
}

/*extern ULONG eventDebugSwitch_bak;*/
DEFUN( todo_event_debug,
	todo_event_debug_cmd,
	"debug alarm [all|log|oam|sync|trap|status]",
	"Debug information\n"
	"Alarm report infomation\n"
	"Debug all Alarm info.\n"
	"alarm log\n"
	"alarm onu-oam\n"
	"alarm trap-syn\n"
	"alarm trap report\n"
	"current alarm status\n")
{
	ULONG debug = 0;
	if( argc > 1 )
		return CMD_ERR_EXEED_ARGC_MAX;
	else if( argc == 0 )
		debug = EVENT_DEBUGSWITCH_ALL;
	else
	{
    		if( VOS_MemCmp(argv[0], "all", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_ALL;
		else if( VOS_MemCmp(argv[0], "log", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_LOG;
		else if( VOS_MemCmp(argv[0], "oam", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_OAM;
		else if( VOS_MemCmp(argv[0], "syn", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_SYN;
		else if( VOS_MemCmp(argv[0], "trap", 4) == 0 )
			debug = EVENT_DEBUGSWITCH_TRAP;
		else if( VOS_MemCmp(argv[0], "sta", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_STA;
		else
			return CMD_ERR_NO_MATCH;
	}

	/*eventDebugSwitch_bak |= debug;*/
	eventDebugSwitch |= debug;

	return CMD_SUCCESS;
}

DEFUN( undo_event_debug,
	undo_event_debug_cmd,
	"undo debug alarm [all|log|oam|sync|trap|status]",
	"Delete configuration\n"
	"Debug information\n"
	"Alarm report infomation\n"
	"Debug all alarm modules\n"
	"Debug alarm log module\n"
	"Debug alarm onu-oam module\n"
	"Debug alarm trap-syn module\n"
	"Debug alarm trap report module\n"
	"current alarm status\n")
{
	ULONG debug = 0;
	if( argc > 1 )
		return CMD_ERR_EXEED_ARGC_MAX;
	else if( argc == 0 )
		debug = EVENT_DEBUGSWITCH_NONE;
	else
	{
    	if( VOS_MemCmp(argv[0], "all", 3) == 0 )
			debug = EVENT_DEBUGSWITCH_NONE;
		else if( VOS_MemCmp(argv[0], "log", 3) == 0 )
			debug = (~EVENT_DEBUGSWITCH_LOG);
		else if( VOS_MemCmp(argv[0], "oam", 3) == 0 )
			debug = (~EVENT_DEBUGSWITCH_OAM);
		else if( VOS_MemCmp(argv[0], "syn", 3) == 0 )
			debug = (~EVENT_DEBUGSWITCH_SYN);
		else if( VOS_MemCmp(argv[0], "trap", 4) == 0 )
			debug = (~EVENT_DEBUGSWITCH_TRAP);
		else if( VOS_MemCmp(argv[0], "sta", 3) == 0 )
			debug = (~EVENT_DEBUGSWITCH_STA);
		else
			return CMD_ERR_NO_MATCH;
	}  
	/*eventDebugSwitch_bak &= debug;*/
	eventDebugSwitch &= debug;

	return CMD_SUCCESS;
}

extern LONG eraseAllEventLog();
extern LONG eraseTrapBacData();
QDEFUN( erase_event_record,
	erase_event_record_cmd,
	"erase alarm [log|sync]",
	"Erase alarm information\n"
	"Erase alarm record\n"
	"Erase alarm log infomation\n"
	"Erase alarm synchronization infomation\n",
	&eventQueId )
{
	/*if( argc > 1 )
		return CMD_ERR_EXEED_ARGC_MAX;
	else if( argc == 0 )
	{
		eraseAllEventLog();
		eraseTrapBacData();
	}
	else*/
	/*if( SYS_LOCAL_MODULE_ISMASTERACTIVE )*/	/* modified by xieshl 20100524, 问题单10223 */
	{
		if( VOS_MemCmp(argv[0], "log", 3) == 0 )
			eraseAllEventLog();
		else if( VOS_MemCmp(argv[0], "syn", 3) == 0 )
			eraseTrapBacData();
		else
			return CMD_ERR_NO_MATCH;
	}  
    return CMD_SUCCESS;
}
/*add by shixh20090420*/

QDEFUN( erase_event_import_record,
	erase_event_import_record_cmd,
	"erase alarm log important",
	"Erase alarm information\n"
	"Erase alarm record\n"
	"Erase alarm log information\n"
	"Erase alarm import log infomation\n",
	&eventQueId )
{
	/*if( argc !=0)
	{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
	}*/

	/*if( SYS_LOCAL_MODULE_ISMASTERACTIVE )*/	/* modified by xieshl 20100510, 问题单10223 */
		eraseAllEventImportLog();/*add by shixh20090420*/
	
    return CMD_SUCCESS;
}

CHAR * dev_idx_to_str( ULONG devIdx )
{
	static CHAR str[32];
	
	if( devIdx == OLT_DEV_ID )
		return "OLT";
	else
	{
		VOS_Sprintf( str, "ONU%d/%d/%d", GET_PONSLOT(devIdx), GET_PONPORT(devIdx), GET_ONUID(devIdx) );
		return str;
	}
}

/* modified by xieshl 20110415, 统一告警屏蔽项定义，下同 */
static char * alarm_dev_mask_type_to_str( ULONG mask_type )
{
	char *pStr;
	mask_type = (mask_type & EVENT_MASK_DEV_ALL);

	switch( mask_type )
	{
		/*case 0:
			pStr = "null";
			break;*/
		case EVENT_MASK_DEV_ALL:
			pStr = "all";
			break;
		case EVENT_MASK_DEV_POWER:
			pStr = "power";
			break;
		case EVENT_MASK_DEV_FAN:
			pStr = "fan";
			break;
		case EVENT_MASK_DEV_CPU:
			pStr = "cpu";
			break;
		case EVENT_MASK_DEV_TEMPERATURE:
			pStr = "temperature";
			break;
		case EVENT_MASK_DEV_REGISTER:
			pStr = "register";
			break;
		case EVENT_MASK_DEV_PRESENT:
			pStr = "present";
			break;

		case EVENT_MASK_DEV_ETH_LINK:
			pStr = "eth-link";
			break;
		case EVENT_MASK_DEV_ETH_FER:
			pStr = "eth-fer";
			break;
		case EVENT_MASK_DEV_ETH_FLR:
			pStr = "eth-flr";
			break;
		case EVENT_MASK_DEV_ETH_TI:
			pStr = "eth-ti";
			break;
		case EVENT_MASK_DEV_ETH_LOOP:
			pStr = "eth-loop";
			break;
		case EVENT_MASK_DEV_PON_BER:
			pStr = "pon-ber";
			break;
		case EVENT_MASK_DEV_PON_FER:
			pStr = "pon-fer";
			break;
		case EVENT_MASK_DEV_PON_ABNORMAL:
			pStr = "pon-abnormal";
			break;
		case EVENT_MASK_DEV_PON_ABS:
			pStr = "pon-aps";
			break;
		case EVENT_MASK_DEV_PON_LINK:
			pStr = "pon-link";
			break;
		case EVENT_MASK_DEV_ONU_LASER_ON:
			pStr = "laseralwayon";
			break;
		case EVENT_MASK_DEV_PON_POWER_L:
		case EVENT_MASK_DEV_PON_POWER_H:
			pStr = "pon-power";
			break;
		case EVENT_MASK_DEV_PON_LOS:
			pStr = "pon-los";
			break;
		case EVENT_MASK_DEV_PWU_STATUS:
			pStr = "pwu_stat";
			break;
		default:
			pStr = "null";
			break;
	}
	
	return pStr;
}

ULONG alarm_mask_device_parase( int argc, char** argv)
{
	int i;
	ULONG mask = 0;

	if( argc == 0 )
	{
		mask = EVENT_MASK_DEV_ALL;
	}
	else
	{
		for( i=0; i<argc; i++ )
		{
	    		if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ALL)) == 0 )
	    		{
				mask = EVENT_MASK_DEV_ALL;
				break;
			}
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_POWER)) == 0 )
				mask |= EVENT_MASK_DEV_POWER;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_FAN)) == 0 )
				mask |= EVENT_MASK_DEV_FAN;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_CPU)) == 0 )
				mask |= EVENT_MASK_DEV_CPU;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_TEMPERATURE)) == 0 )
				mask |= EVENT_MASK_DEV_TEMPERATURE;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_REGISTER)) == 0 )
				mask |= EVENT_MASK_DEV_REGISTER;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PRESENT)) == 0 )
				mask |= EVENT_MASK_DEV_PRESENT;

			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LINK)) == 0 )
				mask |= EVENT_MASK_DEV_ETH_LINK;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FER)) == 0 )
				mask |= EVENT_MASK_DEV_ETH_FER;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FLR)) == 0 )
				mask |= EVENT_MASK_DEV_ETH_FLR;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_TI)) == 0 )
				mask |= EVENT_MASK_DEV_ETH_TI;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LOOP)) == 0 )
				mask |= EVENT_MASK_DEV_ETH_LOOP;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_BER)) == 0 )
				mask |= EVENT_MASK_DEV_PON_BER;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_FER)) == 0 )
				mask |= EVENT_MASK_DEV_PON_FER;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABNORMAL)) == 0 )
				mask |= EVENT_MASK_DEV_PON_ABNORMAL;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABS)) == 0 )
				mask |= EVENT_MASK_DEV_PON_ABS;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_LINK)) == 0 )
				mask |= EVENT_MASK_DEV_PON_LINK;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ONU_LASER_ON)) == 0 )
				mask |= EVENT_MASK_DEV_ONU_LASER_ON;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_L)) == 0 )
				mask |= (EVENT_MASK_DEV_PON_POWER_L | EVENT_MASK_DEV_PON_POWER_H);
			/*else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_H)) == 0 )
				mask |= EVENT_MASK_DEV_PON_POWER_H;*/
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_LOS)) == 0 )
				mask |= EVENT_MASK_DEV_PON_LOS;
			else if( VOS_StrCmp(argv[i], alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PWU_STATUS)) == 0 )
				mask |= EVENT_MASK_DEV_PWU_STATUS;
		}
	}
	return mask;
}

/* added by xieshl 20070920 便于show run和命令一致处理*/
static char * alarm_dev_mask_bits_to_str( ULONG mask, char *mask_str )
{
#define ALM_DEV_MASK_STR_CAT( M )\
	if( mask & (M) )\
	{\
		if( separator ) VOS_StrCat( pStr, "|" );\
		else separator = 1;\
		VOS_StrCat( pStr, alarm_dev_mask_type_to_str(M) );\
	}
	
	char *pStr;
	LONG separator = 0;
	
	if( mask_str == NULL )
	{
		VOS_ASSERT(0);
		return " ERR";
	}
	
	mask = (mask & EVENT_MASK_DEV_ALL);
	pStr = mask_str;
	*pStr = 0;

	if( mask == 0 )
	{
		VOS_StrCpy( pStr, alarm_dev_mask_type_to_str(0) );
	}
	else if( mask == EVENT_MASK_DEV_ALL )
	{
		VOS_StrCpy( pStr, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ALL) );
	}
	else/* if( mask != 0 )*/
	{
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_POWER );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_FAN );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_CPU );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_TEMPERATURE );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_REGISTER );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PRESENT );

		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ETH_LINK );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ETH_FER );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ETH_FLR );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ETH_TI );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ETH_LOOP );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_BER );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_FER );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_ABNORMAL );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_ABS );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_LINK );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_ONU_LASER_ON );
		if( mask & (EVENT_MASK_DEV_PON_POWER_L | EVENT_MASK_DEV_PON_POWER_H) )
			mask |= EVENT_MASK_DEV_PON_POWER_L;
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_POWER_L );
		/*ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_POWER_H );*/
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PON_LOS );
		ALM_DEV_MASK_STR_CAT( EVENT_MASK_DEV_PWU_STATUS);
		
	}
	return mask_str;
}

static LONG alarm_mask_device_display( struct vty *vty, ULONG devidx )
{
	ULONG mask = 0;
	CHAR mask_str[256];

	if( getDeviceAlarmMask(devidx, &mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	/* modified by xieshl 20070920 便于show run和命令一致处理*/
	VOS_MemZero( mask_str, sizeof(mask_str) );
	
	vty_out( vty, "%s alarm mask:%s\r\n", dev_idx_to_str(devidx), alarm_dev_mask_bits_to_str(mask, mask_str) );

 	return CMD_SUCCESS;
}


QDEFUN( alarm_mask_device_olt,
	alarm_mask_device_olt_cmd,
	/*"alarm-mask olt-device [all|power|fan|cpu|temperature|register|present]",*/		/* modified by xieshl 20121029, 问题单16077 */
	"alarm-mask olt-device [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power|pon-los|pwu_stat]",
	"alarm mask configuration\n"
	"OLT device alarm mask\n"
	"all alarm mask\n"
	"power alarm mask\n"
	"fan alarm mask\n"
	"cpu alarm mask\n"
	"temperature alarm mask\n"
	"register alarm mask\n"
	"present alarm mask\n"
	"eth link alarm mask\n"
	"eth fer alarm mask\n"
	"eth flr alarm mask\n"
	"eth ti alarm mask\n"
	"eth loop alarm mask\n"
	"pon ber alarm mask\n"
	"pon fer alarm mask\n"
	"pon abnormal alarm mask\n"
	"pon aps alarm mask\n"
	"pon link alarm mask\n"
	"Laser Alway On alarm mask\n"
	"Optical Power Low or High alarm mask\n"
	"pon LOS alarm mask\n"
	"power status alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask = 0;
	ULONG devidx = OLT_DEV_ID;

	if( argc > 7 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	mask = alarm_mask_device_parase( argc, argv );

	if( getDeviceAlarmMask(devidx, &old_mask) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	if( setDeviceAlarmMask(devidx, (mask | old_mask)) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_device_olt,
	undo_alarm_mask_device_olt_cmd,
	/*"undo alarm-mask olt-device [all|power|fan|cpu|temperature|register|present]",*/
	"undo alarm-mask olt-device [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power|pon-los|pwu_stat]",
	"delete information\n"
	"alarm mask configuration\n"
	"OLT device alarm mask\n"
	"all alarm mask\n"
	"power alarm mask\n"
	"fan alarm mask\n"
	"cpu alarm mask\n"
	"temperature alarm mask\n"
	"register alarm mask\n"
	"present alarm mask\n"
	"eth link alarm mask\n"
	"eth fer alarm mask\n"
	"eth flr alarm mask\n"
	"eth ti alarm mask\n"
	"eth loop alarm mask\n"
	"pon ber alarm mask\n"
	"pon fer alarm mask\n"
	"pon abnormal alarm mask\n"
	"pon aps alarm mask\n"
	"pon link alarm mask\n"
	"Laser Alway On alarm mask\n"
	"Optical Power Low or High alarm mask\n"
	"pon LOS alarm mask\n"
	"power status alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx = OLT_DEV_ID;

	if( argc > 7 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	mask = alarm_mask_device_parase( argc, argv );
	
	if( getDeviceAlarmMask(devidx, &old_mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_DEV_ALL;
	if( setDeviceAlarmMask(devidx, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_device_olt,
	show_alarm_mask_device_olt_cmd,
	"show alarm-mask olt-device",
	"show information\n"
	"show alarm mask configuration\n"
	"OLT device alarm mask\n",
	&eventQueId )
{
	if( argc != 0 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	return alarm_mask_device_display( vty, OLT_DEV_ID );
}

#if 0	
ULONG alarm_mask_onu_parase( char* pMaskStr )
{
	ULONG mask = 0;

	if( pMaskStr )
	{
		if( VOS_MemCmp(pMaskStr, "all", 3) == 0 )
			mask = EVENT_MASK_DEV_ALL;
		else if( VOS_MemCmp(pMaskStr, "pow", 3) == 0 )
			mask |= EVENT_MASK_ONU_POWER;
		else if( VOS_MemCmp(pMaskStr, "fan", 3) == 0 )
			mask |= EVENT_MASK_ONU_FAN;
		else if( VOS_MemCmp(pMaskStr, "cpu", 3) == 0 )
			mask |= EVENT_MASK_ONU_CPU;
		else if( VOS_MemCmp(pMaskStr, "tem", 3) == 0 )
			mask |= EVENT_MASK_ONU_TEMPERATURE;
		else if( VOS_MemCmp(pMaskStr, "reg", 3) == 0 )
			mask |= EVENT_MASK_ONU_REGISTER;
		else if( VOS_MemCmp(pMaskStr, "pre", 3) == 0 )
			mask |= EVENT_MASK_ONU_PRESENT;
		else if( VOS_MemCmp(pMaskStr, "eth-link", 8) == 0 )
			mask |= EVENT_MASK_ONU_ETH_LINK;
		else if( VOS_MemCmp(pMaskStr, "eth-fer", 7) == 0 )
			mask |= EVENT_MASK_ONU_ETH_FER;
		else if( VOS_MemCmp(pMaskStr, "eth-flr", 7) == 0 )
			mask |= EVENT_MASK_ONU_ETH_FLR;
		else if( VOS_MemCmp(pMaskStr, "eth-ti", 6) == 0 )
			mask |= EVENT_MASK_ONU_ETH_TI;
		else if( VOS_MemCmp(pMaskStr, "eth-loop", 8) == 0 )
			mask |= EVENT_MASK_ONU_ETH_LOOP;
		else if( VOS_MemCmp(pMaskStr, "pon-ber", 7) == 0 )
			mask |= EVENT_MASK_ONU_PON_BER;
		else if( VOS_MemCmp(pMaskStr, "pon-fer", 7) == 0 )
			mask |= EVENT_MASK_ONU_PON_FER;
		else if( VOS_MemCmp(pMaskStr, "pon-abn", 7) == 0 )
			mask |= EVENT_MASK_ONU_PON_ABNORMAL;
		else if( VOS_MemCmp(pMaskStr, "pon-aps", 7) == 0 )
			mask |= EVENT_MASK_ONU_PON_APS;
		else if( VOS_MemCmp(pMaskStr, "pon-link", 8) == 0 )	
			mask |= EVENT_MASK_ONU_PON_LINK;
		else if( VOS_MemCmp(pMaskStr, "laseralwayon", 12) == 0 )
			mask |= EVENT_MASK_ONU_LASER_ALWAYS_ON;
		else if( VOS_MemCmp(pMaskStr, "opticalpowerlow", 15) == 0 )
			mask |= EVENT_MASK_ONU_OPTICAL_POWER_LOW;
		else if( VOS_MemCmp(pMaskStr, "opticalpowerhigh", 16) == 0 )
			mask |= EVENT_MASK_ONU_OPTICAL_POWER_HIGH;
	
	}
	return mask;
}

static int alarm_mask_onu_display( struct vty *vty, ULONG devidx )
{
	ULONG   maskvalue=0;
		
	if(getDeviceAlarmMask( devidx, &maskvalue ) == VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	if( maskvalue == 0 )
	{
		vty_out(vty,"  no alarm mask!\r\n");
		return (CMD_WARNING);
	}
	if( maskvalue == EVENT_MASK_DEV_ALL )
	{
		vty_out(vty,"  all alarm mask!\r\n");
		return CMD_WARNING;
	}
		
	if( maskvalue & EVENT_MASK_ONU_POWER )		vty_out(vty," power");
	if( maskvalue & EVENT_MASK_ONU_FAN )			vty_out(vty," fan");
	if( maskvalue & EVENT_MASK_ONU_CPU )			vty_out(vty," cpu");
	if( maskvalue & EVENT_MASK_ONU_TEMPERATURE )	vty_out(vty," temperature");
	if( maskvalue & EVENT_MASK_ONU_REGISTER )	vty_out(vty," register");
	if( maskvalue & EVENT_MASK_ONU_PRESENT )		vty_out(vty," present");
	if( maskvalue & EVENT_MASK_ONU_ETH_LINK )		vty_out(vty," eth-link");
	if( maskvalue & EVENT_MASK_ONU_ETH_FER )		vty_out(vty," eth-fer");
	if( maskvalue & EVENT_MASK_ONU_ETH_FLR )		vty_out(vty," eth-flr");
	if( maskvalue & EVENT_MASK_ONU_ETH_TI )		vty_out(vty," eth-ti");
	if( maskvalue & EVENT_MASK_ONU_ETH_LOOP )	vty_out(vty," eth-loop");
	if( maskvalue & EVENT_MASK_ONU_PON_BER )		vty_out(vty," pon-ber");
	if( maskvalue & EVENT_MASK_ONU_PON_FER )		vty_out(vty," pon-fer");
	if( maskvalue & EVENT_MASK_ONU_PON_ABNORMAL )	vty_out(vty," pon-abnormal");
	if( maskvalue & EVENT_MASK_ONU_PON_APS )		vty_out(vty," pon-aps");
	if( maskvalue & EVENT_MASK_ONU_PON_LINK )		vty_out(vty," pon-link");
	if( maskvalue & EVENT_MASK_ONU_LASER_ALWAYS_ON )	vty_out(vty," laseralwayson");
	if( maskvalue & EVENT_MASK_ONU_OPTICAL_POWER_LOW )	vty_out(vty," opticalpowerlow");
	if( maskvalue & EVENT_MASK_ONU_OPTICAL_POWER_HIGH )	vty_out(vty," opticalpowerhigh");

	vty_out(vty,"\r\n");
		
	return CMD_SUCCESS;
}
#endif

QDEFUN( alarm_mask_device_onu,
	alarm_mask_device_onu_cmd,
	/*"alarm-mask onu-device <slot/port/onuid> [all|power|fan|cpu|temperature|register|present]",*/
	"alarm-mask onu-device <slot/port/onuid> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power]",
	"alarm mask configuration\n"
	"onu alarm mask\n"
	"onu index\n"
	"all alarm mask\n"
	"onu power alarm mask\n"
	"onu fan alarm mask\n"
	"onu cpu alarm mask\n"
	"onu temperature alarm mask\n"
	"onu register alarm mask\n"
	"onu present alarm mask\n"
	"onu eth link alarm mask\n"
	"onu eth fer alarm mask\n"
	"onu eth flr alarm mask\n"
	"onu eth ti alarm mask\n"
	"onu eth loop alarm mask\n"
	"onu pon ber alarm mask\n"
	"onu pon fer alarm mask\n"
	"onu pon abnormal alarm mask\n"
	"onu pon aps alarm mask\n"
	"onu pon link alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"onu Optical Power Low or High alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx, slot = 0, port = 0, onuId = 0;
	if( (argc == 0) || (argc > 2))
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

 	if( (PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK) || (SlotCardMayBePonBoardByVty(slot, vty) != VOS_OK) )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	devidx = MAKEDEVID( slot, port, onuId );

	mask = alarm_mask_device_parase( argc-1, &argv[1] );
	/*mask = alarm_mask_onu_parase( argv[1] );*/
	
	if( getDeviceAlarmMask(devidx, &old_mask) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	if( setDeviceAlarmMask(devidx, (mask | old_mask)) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_device_onu,
	undo_alarm_mask_device_onu_cmd,
	/*"undo alarm-mask onu-device <slot/port/onuid> [all|power|fan|cpu|temperature|register|present]",*/
	"undo alarm-mask onu-device <slot/port/onuid> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power]",
	"delete information\n"
	"delete alarm mask configuration\n"
	"delete onu alarm mask configuration\n"
	"onuidx  alarm mask\n"
	"all alarm mask\n"
	"onu power alarm mask\n"
	"onu fan alarm mask\n"
	"onu cpu alarm mask\n"
	"onu temperature alarm mask\n"
	"onu register alarm mask\n"
	"onu present alarm mask\n"
	"onu eth link alarm mask\n"
	"onu eth fer alarm mask\n"
	"onu eth flr alarm mask\n"
	"onu eth ti alarm mask\n"
	"onu eth loop alarm mask\n"
	"onu pon ber alarm mask\n"
	"onu pon fer alarm mask\n"
	"onu pon abnormal alarm mask\n"
	"onu pon aps alarm mask\n"
	"onu pon link alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"onu Optical Power Low or High alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx, slot = 0, port = 0, onuId = 0;

	if( (argc == 0) || (argc > 2) )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

 	if( (PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK) || (SlotCardMayBePonBoardByVty(slot, vty) != VOS_OK) )
		return CMD_WARNING;
	devidx = MAKEDEVID( slot, port, onuId );
	CHECK_CMD_ONU_RANGE(vty, onuId-1);

	mask = alarm_mask_device_parase( argc-1, &argv[1] );
	/*mask = alarm_mask_onu_parase( argv[1] );*/

	if( getDeviceAlarmMask(devidx, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_DEV_ALL;
	if( setDeviceAlarmMask(devidx, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_device_onu,
	show_alarm_mask_device_onu_cmd,
	"show alarm-mask onu-device <slot/port/onuid>",
	"show information\n"
	"show alarm mask configuration\n"
	"ONU device alarm mask\n"
	"ONU index\n",
	&eventQueId )
{
	ULONG devidx, slot = 0, port = 0, onuId = 0;
	if( argc != 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
 	if( (PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK) || (SlotCardMayBePonBoardByVty(slot, vty) != VOS_OK) )
		return CMD_WARNING;
	devidx = MAKEDEVID( slot, port, onuId );
	
	/*return alarm_mask_onu_display( vty, devidx );*/
	return alarm_mask_device_display( vty, devidx );
}

static char * alarm_pon_mask_type_to_str( ULONG mask_type )
{
	char *pStr;
	mask_type = (mask_type & EVENT_MASK_PON_ALL);

	switch( mask_type )
	{
		/*case 0:
			pStr = "null";
			break;*/
		case EVENT_MASK_PON_ALL:
			pStr = "all";
			break;
		case EVENT_MASK_PON_BER:
			pStr = "ber";
			break;
		case EVENT_MASK_PON_FER:
			pStr = "fer";
			break;
		case EVENT_MASK_PON_ABNORMAL:
			pStr = "abnormal";
			break;
		case EVENT_MASK_PON_APS:
			pStr = "aps";
			break;
		case EVENT_MASK_PON_LINK:
			pStr = "link";
			break;
		case EVENT_MASK_PON_LASER_ON:
			pStr = "laseralwayon";
			break;
		case EVENT_MASK_PON_POWER_LOW:
		case EVENT_MASK_PON_POWER_HIGH:
			pStr = "power";
			break;
		case EVENT_MASK_PON_LOS:
			pStr = "los";
			break;
		case EVENT_MASK_PON_TEMPERATURE:
			pStr = "temperature";
			break;
		case EVENT_MASK_PON_BIAS_CURRENT:
			pStr = "current";
			break;
		case EVENT_MASK_PON_VOLTAGE:
			pStr = "voltage";
			break;
		default:
			pStr = "null";
			break;
	}
	return pStr;
}

ULONG alarm_mask_pon_parase( int argc, char** argv)
{
	int i;
	ULONG mask = 0;

	if( argc == 1 )
	{
		mask = EVENT_MASK_PON_ALL;
	}
	else
	{
		for( i=0; i<argc; i++ )
		{
			if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_ALL)) == 0 )
			{
				mask = EVENT_MASK_PON_ALL;
				break;
			}
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_BER)) == 0 )
				mask |= EVENT_MASK_PON_BER;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_FER)) == 0 )
				mask |= EVENT_MASK_PON_FER;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_ABNORMAL)) == 0 )
				mask |= EVENT_MASK_PON_ABNORMAL;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_APS)) == 0 )
				mask |= EVENT_MASK_PON_APS;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_LINK)) == 0 )
				mask |= EVENT_MASK_PON_LINK;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_LOS)) == 0 )
				mask |= EVENT_MASK_PON_LOS;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_LASER_ON)) == 0 )
				mask |= EVENT_MASK_PON_LASER_ON;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_POWER_LOW)) == 0 )
				mask |= (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH);
			/*else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_POWER_HIGH)) == 0 )
				mask |= EVENT_MASK_PON_POWER_HIGH;*/

			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_TEMPERATURE)) == 0 )
				mask |= EVENT_MASK_PON_TEMPERATURE;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_BIAS_CURRENT)) == 0 )
				mask |= EVENT_MASK_PON_BIAS_CURRENT;
			else if( VOS_StrCmp(argv[i], alarm_pon_mask_type_to_str(EVENT_MASK_PON_VOLTAGE)) == 0 )
				mask |= EVENT_MASK_PON_VOLTAGE;
		}
	}
	return mask;
}

/* added by xieshl 20070920 便于show run和命令一致处理*/
static CHAR * alarm_pon_mask_bits_to_str( ULONG mask, char *mask_str )
{
#define ALM_PON_MASK_STR_CAT( M )\
	if( mask & M )\
	{\
		if( separator ) VOS_StrCat( pStr, "|" );\
		else separator = 1;\
		VOS_StrCat( pStr, alarm_pon_mask_type_to_str(M) );\
	}
	
	CHAR *pStr;
	LONG separator = 0;
	
	if( mask_str == NULL )
	{
		VOS_ASSERT(0);
		return " ERR";
	}
	
	mask = (mask & EVENT_MASK_PON_ALL);
	pStr = mask_str;
	*pStr = 0;

	if( mask == 0 )
	{
		VOS_StrCpy( pStr, alarm_pon_mask_type_to_str(0) );
	}
	else if( mask == EVENT_MASK_PON_ALL )
	{
		VOS_StrCpy( pStr, alarm_pon_mask_type_to_str(EVENT_MASK_PON_ALL) );
	}
	else
	{
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_BER);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_FER);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_ABNORMAL);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_APS);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_LINK);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_LOS);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_LASER_ON);
		if( mask & (EVENT_MASK_PON_POWER_LOW |EVENT_MASK_PON_POWER_HIGH) )
			ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_POWER_LOW);
		/*ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_POWER_HIGH);*/

		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_TEMPERATURE);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_BIAS_CURRENT);
		ALM_PON_MASK_STR_CAT(EVENT_MASK_PON_VOLTAGE);
	}
	return mask_str;
}

static LONG alarm_mask_pon_display(ULONG devidx, ULONG slot, ULONG port, struct vty *vty)
{
	ULONG mask = 0;
	char mask_str[128];

	if( getPonPortAlarmMask(devidx, slot, port, &mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	/* modified by xieshl 20070920 便于show run和命令一致处理*/
	VOS_MemZero( mask_str, sizeof(mask_str) );
	vty_out( vty, " %s pon%d/%d alarm mask:%s\r\n", dev_idx_to_str(devidx), slot, port, alarm_pon_mask_bits_to_str(mask, mask_str) );

 	return CMD_SUCCESS;
}

QDEFUN( alarm_mask_olt_pon,
	alarm_mask_olt_pon_cmd,
	"alarm-mask olt-pon <slot/port> [all|ber|fer|abnormal|aps|link|los|laseralwayon|power|temperature|current|voltage]",
	"alarm mask configuration\n"
	"OLT PON alarm mask\n"
	"PON port index\n"
	"all alarm mask\n"
	"PON BER alarm mask\n"
	"PON FER alarm mask\n"
	"PON port abnormal alarm mask\n"
	"automatic-protection-switching alarm mask\n"
	"PON port link up/down alarm mask\n"
	"PON port link LOS alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"PON Optical Power Low or High alarm mask\n"
	"PON SFP temperature\n"
	"PON SFP bias current\n"
	"PON SFP voltage\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx = 1, slot = 0, port = 0;
	if( (argc == 0) || (argc > 2))
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	IFM_ParseSlotPort( argv[0], &slot, &port );
	if( SlotCardIsPonBoard(slot) != ROK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}

	mask = alarm_mask_pon_parase( argc, argv );
	if( mask == 0 )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
	
	if( getPonPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Can not read pon %d/%d alarm-mask\r\n", slot, port );
		return CMD_ERR_AMBIGUOUS;
	}
	if( setPonPortAlarmMask(devidx, slot, port, (mask | old_mask)) == VOS_ERROR )
	{
		vty_out( vty, "  %% Set pon %d/%d alarm-mask error\r\n", slot, port );
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_olt_pon,
	undo_alarm_mask_olt_pon_cmd,
	"undo alarm-mask olt-pon <slot/port> [all|ber|fer|abnormal|aps|link|los|laseralwayon|power|temperature|current|voltage]",
	"delete information\n"
	"alarm mask configuration\n"
	"OLT PON alarm mask\n"
	"PON port index\n"
	"all alarm mask\n"
	"PON BER alarm mask\n"
	"PON FER alarm mask\n"
	"PON port abnormal alarm mask\n"
	"automatic-protection-switching alarm mask\n"
	"PON port link up/down alarm mask\n"
	"PON port link LOS alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"PON Optical Power Low or High alarm mask\n"
	"PON SFP temperature\n"
	"PON SFP bias current\n"
	"PON SFP voltage\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx = 1, slot = 0, port = 0;

	if( (argc == 0) || (argc > 2) )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	IFM_ParseSlotPort( argv[0], &slot, &port );
	if( SlotCardIsPonBoard(slot) != ROK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}

	mask = alarm_mask_pon_parase( argc, argv );
	if( mask == 0 )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}

	if( getPonPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Can not read pon %d/%d alarm-mask\r\n", slot, port );
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_PON_ALL;
	if( setPonPortAlarmMask(devidx, slot, port, mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Set pon %d/%d alarm-mask error\r\n", slot, port );
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_olt_pon,
	show_alarm_mask_olt_pon_cmd,
	"show alarm-mask olt-pon <slot/port>",
	"show information\n"
	"show alarm mask configuration\n"
	"OLT PON alarm mask\n"
	"PON port index\n",
	&eventQueId )
{
	ULONG devidx = 1, slot = 0, port = 0;
	if( argc != 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
 	if( IFM_ParseSlotPort( argv[0], &slot, &port )  != VOS_OK )
		return CMD_WARNING;
	
	if(PonCardSlotPortCheckWhenRunningByVty(slot, port,vty) != ROK)
		return(CMD_WARNING);
	if(SlotCardMayBePonBoardByVty(slot,vty) != ROK )
		return(CMD_WARNING);
	
	return alarm_mask_pon_display( devidx, slot, port, vty );
}

QDEFUN( alarm_mask_pon_cni,
       alarm_mask_pon_cni_cmd,
       "alarm-mask pon-cni ber",
	"alarm mask configuration\n"
	"all PONs CNI alarm mask\n"
	"PON BER mask\n",
       &eventQueId)
{
	alarm_mask_pon_cni_bits = 1;
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_pon_cni,
       undo_alarm_mask_pon_cni_cmd,
       "undo alarm-mask pon-cni ber",
	"delete information\n"
	"alarm mask configuration\n"
	"all PONs CNI alarm mask\n"
	"PON BER mask\n",
       &eventQueId)
{
	alarm_mask_pon_cni_bits = 0;
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_pon_cni,
	show_alarm_mask_pon_cni_cmd,
	"show alarm-mask pon-cni",
	"show information\n"
	"show alarm mask configuration\n"
	"all PONs CNI alarm mask\n",
	&eventQueId )
{
	vty_out( vty, "  pon-cni alarm mask: %s\r\n", (alarm_mask_pon_cni_bits & 1) ? "ber" : "null" );
	return CMD_SUCCESS;
}

QDEFUN( alarm_mask_onu_opt,
	alarm_mask_onu_opt_cmd,
	"alarm-mask onu-pon [all|power|temperature|current|voltage] [enable|disable]",
	"alarm mask configuration\n"
	"ONU PON alarm mask\n"
	"all alarm mask include power temperature current and voltage\n"
	"ONU Optical Power Low or High alarm mask\n"
	"ONU SFP temperature\n"
	"ONU SFP bias current\n"
	"ONU SFP voltage\n"
	"enable alarm-mask\n"
	"disable alarm-mask\n",
	&eventQueId )
{
	if( argc != 2 )
	{
		return CMD_WARNING;
	}

	if( VOS_StrCmp(argv[0], "all") == 0 )
	{
		if( VOS_StrCmp(argv[1], "enable") == 0 )
		{
			OptOnuAlarmMask |= (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH | 
					EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_VOLTAGE);
		}
		else if( VOS_StrCmp(argv[1], "disable") == 0 )
		{
			OptOnuAlarmMask &= ~(EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH | 
					EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_VOLTAGE);
		}
	}
	else if( VOS_StrCmp(argv[0], "power") == 0 )
	{
		if( VOS_StrCmp(argv[1], "enable") == 0 )
		{
			OptOnuAlarmMask |= (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH);
		}
		else if( VOS_StrCmp(argv[1], "disable") == 0 )
		{
			OptOnuAlarmMask &= ~(EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH);
		}
	}
	else if( VOS_StrCmp(argv[0], "temperature") == 0 )
	{
		if( VOS_StrCmp(argv[1], "enable") == 0 )
		{
			OptOnuAlarmMask |= EVENT_MASK_PON_TEMPERATURE;
		}
		else if( VOS_StrCmp(argv[1], "disable") == 0 )
		{
			OptOnuAlarmMask &= ~EVENT_MASK_PON_TEMPERATURE;
		}
	}
	else if( VOS_StrCmp(argv[0], "current") == 0 )
	{
		if( VOS_StrCmp(argv[1], "enable") == 0 )
		{
			OptOnuAlarmMask |= EVENT_MASK_PON_BIAS_CURRENT;
		}
		else if( VOS_StrCmp(argv[1], "disable") == 0 )
		{
			OptOnuAlarmMask &= ~EVENT_MASK_PON_BIAS_CURRENT;
		}
	}
	else if( VOS_StrCmp(argv[0], "voltage") == 0 )
	{
		if( VOS_StrCmp(argv[1], "enable") == 0 )
		{
			OptOnuAlarmMask |= EVENT_MASK_PON_VOLTAGE;
		}
		else if( VOS_StrCmp(argv[1], "disable") == 0 )
		{
			OptOnuAlarmMask &= ~EVENT_MASK_PON_VOLTAGE;
		}
	}

	return CMD_SUCCESS;	
}

QDEFUN( show_alarm_mask_onu_opt,
	show_alarm_mask_onu_opt_cmd,
	"show alarm-mask onu-pon ",
	"show information\n"
	"show alarm mask configuration\n"
	"OLT PON alarm mask\n",
	&eventQueId )
{
	if(OptOnuAlarmMask & (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH))
	{
		vty_out( vty, " alarm-mask onu-pon power is enable\r\n");
	}
	else
	{
		vty_out( vty, " alarm-mask onu-pon power is disable\r\n");
	}

	if(OptOnuAlarmMask & EVENT_MASK_PON_TEMPERATURE)
	{
		vty_out( vty, " alarm-mask onu-pon temperature is enable\r\n");
	}
	else
	{
		vty_out( vty, " alarm-mask onu-pon temperature is disable\r\n");
	}
	
	if(OptOnuAlarmMask & EVENT_MASK_PON_BIAS_CURRENT)
	{
		vty_out( vty, " alarm-mask onu-pon current is enable\r\n");
	}
	else
	{
		vty_out( vty, " alarm-mask onu-pon current is disable\r\n");
	}
	
	if(OptOnuAlarmMask & EVENT_MASK_PON_VOLTAGE)
	{
		vty_out( vty, " alarm-mask onu-pon voltage is enable\r\n");
	}
	else
	{
		vty_out( vty, " alarm-mask onu-pon voltage is disable\r\n");
	}
	
	return CMD_SUCCESS;	
}

QDEFUN( alarm_mask_onu_pon,
	alarm_mask_onu_pon_cmd,
	"alarm-mask onu-pon <slot/port/onuid> [all|ber|fer|abnormal|aps|link|laseralwayon|power|temperature|current|voltage]",
	"alarm mask configuration\n"
	"ONU PON alarm mask\n"
	"ONU index\n"
	"all alarm mask\n"
	"PON BER alarm mask\n"
	"PON FER alarm mask\n"
	"PON port abnormal alarm mask\n"
	"automatic-protection-switching alarm mask\n"
	"PON port link up/down alarm mask\n"
	"ONU Laser Alway On alarm mask\n"
	"ONU Optical Power Low or High alarm mask\n"
	"ONU SFP temperature\n"
	"ONU SFP bias current\n"
	"ONU SFP voltage\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx, slot = 0, port = 0, onuId = 0;
	if( (argc == 0) || (argc > 2))
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	
	/* 解决#5409 */
	/*if( (IFM_ParseSlotPort( argv[0], &slot, &port ) != VOS_OK ) ||
		(__SYS_MODULE_TYPE__(slot) != MODULE_E_GFA_EPON) )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}*/
	if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId ) != VOS_OK )
		return CMD_WARNING;
	if(SlotCardIsPonBoard(slot) != ROK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}
	
	devidx = MAKEDEVID( slot, port, onuId );

	mask = alarm_mask_pon_parase( argc, argv );
	if( mask == 0 )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	if( getPonPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Can not read onu %d alarm-mask\r\n", devidx );
		return CMD_WARNING;
	}
	if( setPonPortAlarmMask(devidx, slot, port, (mask | old_mask)) == VOS_ERROR )
	{
		vty_out( vty, "  %% Set onu %d alarm-mask error\r\n", devidx );
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_onu_pon,
	undo_alarm_mask_onu_pon_cmd,
	"undo alarm-mask onu-pon <slot/port/onuid> [all|ber|fer|abnormal|aps|link|laseralwayon|power|temperature|current|voltage]",
	"delete information\n"
	"alarm mask configuration\n"
	"ONU PON alarm mask\n"
	"ONU index\n"
	"all alarm mask\n"
	"PON BER alarm mask\n"
 	"PON FER alarm mask\n"
 	"PON port abnormal alarm mask\n"
	"automatic-protection-switching alarm mask\n"
	"PON port link up/down alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"ONU Optical Power Low or High alarm mask\n"
	"ONU SFP temperature\n"
	"ONU SFP bias current\n"
	"ONU SFP voltage\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx, slot = 0, port = 0, onuId = 0;

	if( (argc == 0) || (argc > 2) )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	/*IFM_ParseSlotPort( argv[0], &slot, &port );*/

	if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId ) != VOS_OK )
		return CMD_WARNING;
	if( SlotCardIsPonBoard(slot) != ROK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}
	
	devidx = MAKEDEVID( slot, port, onuId );

	mask = alarm_mask_pon_parase( argc, argv );
	if( mask == 0 )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}

	if( getPonPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Can not read onu %d alarm-mask\r\n", devidx );
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_PON_ALL;
	if( setPonPortAlarmMask(devidx, slot, port, mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Set onu %d alarm-mask error\r\n", devidx );
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_onu_pon,
	show_alarm_mask_onu_pon_cmd,
	"show alarm-mask onu-pon <slot/port/onuid>",
	"show information\n"
	"show alarm mask configuration\n"
	"OLT PON alarm mask\n"
	"PON port index\n",
	&eventQueId )
{
	ULONG devidx, slot = 0, port = 0, onuId = 0;
	if( argc != 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	/*IFM_ParseSlotPort( argv[0], &slot, &port );*/
 	if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK )
		return CMD_WARNING;
	if( SlotCardIsPonBoard(slot) !=ROK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}

	devidx = MAKEDEVID( slot, port, onuId );
	
	return alarm_mask_pon_display( devidx, slot, port, vty );
}

ULONG alarm_mask_eth_parase( int argc, char** argv)
{
	int i;
	ULONG mask = 0;

	if( argc == 0 )
	{
		mask = EVENT_MASK_ETH_ALL;
	}
	else
	{
		for( i=0; i<argc; i++ )
		{
	    		if( VOS_MemCmp(argv[i], "all", 3) == 0 )
	    		{
				mask = EVENT_MASK_ETH_ALL;
				break;
			}
			else if( VOS_MemCmp(argv[i], "lin", 3) == 0 )
				mask |= EVENT_MASK_ETH_LINK;
			else if( VOS_MemCmp(argv[i], "fer", 3) == 0 )
				mask |= EVENT_MASK_ETH_FER;
			else if( VOS_MemCmp(argv[i], "flr", 3) == 0 )
				mask |= EVENT_MASK_ETH_FLR;
			else if( VOS_MemCmp(argv[i], "ti", 2) == 0 )
				mask |= EVENT_MASK_ETH_TI;
			else if( VOS_MemCmp(argv[i], "lo", 2) == 0 )
				mask |= EVENT_MASK_ETH_LOOP;
			else if( VOS_MemCmp(argv[i], "bcfc", 4) == 0 )
				mask |= EVENT_MASK_ETH_BCFC;
		}
	}
	return mask;
}

/* added by xieshl 20070920 便于show run和命令一致处理*/
static char * alarm_eth_mask_bits_to_str( ULONG mask, char *mask_str )
{
	ULONG separator = 0;

	*mask_str = 0;
	mask = (mask & EVENT_MASK_ETH_ALL);

	if( mask == 0 )
	{
		VOS_StrCpy( mask_str, "null" );
	}
	else if( mask == EVENT_MASK_ETH_ALL )
	{
		VOS_StrCpy( mask_str, "all" );
	}
	else
	{
		if( mask & EVENT_MASK_ETH_LINK )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "link" );
		}
		if( mask & EVENT_MASK_ETH_FER )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "fer" );
		}
		if( mask & EVENT_MASK_ETH_FLR )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "flr" );
		}
		if( mask & EVENT_MASK_ETH_TI )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "ti" );
		}
		if( mask & EVENT_MASK_ETH_LOOP )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "loop" );
		}
		if( mask & EVENT_MASK_ETH_BCFC )
		{
			if( separator )	VOS_StrCat( mask_str, "|" );
			else			separator = 1;
			VOS_StrCat( mask_str, "bcfc" );
		}
	}
	return mask_str;

}

static LONG alarm_mask_eth_display(ULONG devidx, ULONG slot, ULONG port, struct vty *vty)
{
	ULONG mask = 0;
	char mask_str[64];

	/* modified by xieshl 20110418 */
	VOS_MemZero( mask_str, sizeof(mask_str) );
	if( devidx == OLT_DEV_ID )
	{
		if( getEthPortAlarmMask(devidx, slot, port, &mask) == VOS_OK )
		{
			vty_out( vty, "OLT eth%d/%d alarm mask:%s\r\n", slot, port, alarm_eth_mask_bits_to_str(mask, mask_str) );
 			return CMD_SUCCESS;
		}
	}
	else
	{
		if( getOnuEthPortAlarmMask(&mask) == VOS_OK )
		{
			vty_out( vty, "ONU eth port alarm mask:%s\r\n", alarm_eth_mask_bits_to_str(mask, mask_str) );
 			return CMD_SUCCESS;
		}
	}

	vty_out( vty, "  %% Parameter is error.\r\n" );
	return CMD_ERR_AMBIGUOUS;
	
	/*mask = (mask & EVENT_MASK_ETH_ALL);

	vty_out( vty, "DEV:%d, eth:%d/%d alarm mask:", devidx, slot, port );
	if( mask == 0 )
	{
		vty_out( vty, " null\r\n" );
	}
	else if( mask == EVENT_MASK_ETH_ALL )
	{
		vty_out( vty, " all\r\n" );
	}
	else
	{
		if( mask & EVENT_MASK_ETH_LINK )
			vty_out( vty, " link" );
		if( mask & EVENT_MASK_ETH_FER )
			vty_out( vty, " fer" );
		if( mask & EVENT_MASK_ETH_FLR )
			vty_out( vty, " flr" );
		if( mask & EVENT_MASK_ETH_TI )
			vty_out( vty, " ti" );

		vty_out( vty, "\r\n" );
	}*/
	/* end 20070920 */
 	return CMD_SUCCESS;
}

/* added by xieshl 20110707, 检查输入的端口号是否正确，问题单13238 */
LONG olt_eth_idx_check( ULONG slotno, ULONG portno, struct vty *vty )
{
	if( SlotCardIsUplinkBoard(slotno) == VOS_OK )
    {
    	if( (portno == 0) || (portno > ETH_slot_logical_portnum(slotno)) )
    	{
    		if( vty ) vty_out( vty, " %% Error eth %d/%d is not exist.\r\n", slotno, portno  );
    		return VOS_ERROR;
    	}
    }
    else
	{
        if( (0 == SYS_MODULE_IS_UPLINK(slotno)) && (0 == SYS_MODULE_IS_UPLINK_PON(slotno)) )/*modified by yangtongsan 2014-12-04 for问题单23236*/
        {
    		if( vty ) vty_out( vty, "  %% Error slot %d is not uplink.\r\n", slotno  );
    		return VOS_ERROR;
        }
        else
        {
            if (VOS_YES != userport_is_uplink(slotno, portno))
            {
        		if( vty ) vty_out( vty, "  %% Error port %d/%d is not uplink.\r\n", slotno, portno  );
        		return VOS_ERROR;
            }
        }
	}

	return VOS_OK;
}

QDEFUN( alarm_mask_olt_eth,
	alarm_mask_olt_eth_cmd,
	"alarm-mask olt-eth <slot/port> [all|link|fer|flr|ti|loop]",
	"alarm mask configuration\n"
	"OLT ETH alarm mask\n"
	"ETH port index\n"
	"all alarm mask\n"
	"link up/down alarm mask\n"
	"Frame ERROR Rate alarm mask\n"    /*问题单8747*/
	"Frame Loss  Rate alarm mask\n"
	"Traffic Intermit alarm mask\n"
	"eth loop alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx = OLT_DEV_ID, slot = 0, port = 0;
	if( argc != 2 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
#if 0	/* modified by xieshl 20110707, 问题单13238 */
	IFM_ParseSlotPort( argv[0], &slot, &port );
	if( SlotCardIsUplinkBoard(slot) != VOS_OK )	/* 问题单11399 */
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot  );
		return CMD_WARNING;
	}
	/*问题单8746*/
	if((port>4)||(port<1))
		{
		vty_out(vty,"port number out of range,the range is 1-4\r\n");
		return CMD_WARNING;
	}
#else
	if( IFM_ParseSlotPort( argv[0], &slot, &port ) != VOS_OK )
		return CMD_WARNING;
	if( olt_eth_idx_check(slot, port, vty) == VOS_ERROR )
		return CMD_WARNING;
#endif
	mask = alarm_mask_eth_parase( argc-1, &argv[1] );
	
	if( getEthPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		return CMD_WARNING;
	}
	if( setEthPortAlarmMask(devidx, slot, port, (mask | old_mask)) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_olt_eth,
	undo_alarm_mask_olt_eth_cmd,
	"undo alarm-mask olt-eth <slot/port> [all|link|fer|flr|ti|loop]",
	"delete information\n"
	"alarm mask configuration\n"
	"OLT ETH alarm mask\n"
	"ETH port index\n"
	"all alarm mask\n"
	"link up/down alarm mask\n"
	"Frame ERROR Rate alarm mask\n"   /*问题单8747*/
	"Frame Loss Rate alarm mask\n"
	"Traffic Intermit alarm mask\n"
	"eth loop alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	ULONG devidx = 1, slot = 0, port = 0;

	if( argc != 2 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

#if 0	/* modified by xieshl 20110707, 问题单13238 */
	IFM_ParseSlotPort( argv[0], &slot, &port );
	if( SlotCardIsUplinkBoard(slot) != VOS_OK )		/* 问题单11399 */
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}
	/*问题单8746*/
	if((port>4)||(port<1))
		{
		vty_out(vty,"port number out of range,the range is 1-4\r\n");
		return CMD_WARNING;
		}
#else
	if( IFM_ParseSlotPort( argv[0], &slot, &port ) != VOS_OK )
		return CMD_WARNING;
	if( olt_eth_idx_check(slot, port, vty) == VOS_ERROR )
		return CMD_WARNING;
#endif
	
	mask = alarm_mask_eth_parase( argc-1, &argv[1] );

	if( getEthPortAlarmMask(devidx, slot, port, &old_mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_ETH_ALL;
	if( setEthPortAlarmMask(devidx, slot, port, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_olt_eth,
	show_alarm_mask_olt_eth_cmd,
	"show alarm-mask olt-eth <slot/port>",
	"show information\n"
	"show alarm mask configuration\n"
	"OLT ETH alarm mask\n"
	"ETH port index\n",
	&eventQueId )
{
#if 0	/* modified by xieshl 20110707, 问题单13238 */
	ULONG devidx = OLT_DEV_ID, slot = 0, port = 0;
	LONG swport;
	
	if( argc != 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
 	if( IFM_ParseSlotPort( argv[0], &slot, &port )  != VOS_OK )
		return CMD_WARNING;
	
	if( SlotCardIsUplinkBoard(slot) != VOS_OK )
	{
		vty_out( vty, "  %% Error slot %d.\r\n", slot );
		return CMD_WARNING;
	}

	swport = slot_port_2_swport_no(slot, port);
	if( (swport >= 1) && (swport <= MAXETH) )
	{
		if( (__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA6900_GEM_10GE) ||
			(port <= 4) )
		{
			return alarm_mask_eth_display( devidx, slot, port, vty );
		}
	}
	/*问题单8746*/
	vty_out(vty,"port number out of range\r\n");
	return CMD_WARNING;

#else
	ULONG slot = 0, port = 0;
 	if( IFM_ParseSlotPort( argv[0], &slot, &port )  != VOS_OK )
		return CMD_WARNING;
	if( olt_eth_idx_check(slot, port, vty) == VOS_ERROR )
		return CMD_WARNING;
	
	return alarm_mask_eth_display( OLT_DEV_ID, slot, port, vty );
#endif
}

/* added by xieshl 20071105, 所有ONU的所有以太网端口采用相同的配置，不再单独区分 */
static ULONG onuEthPortAlarmMask = 0x80000000;	/*问题单9535*/	/* ONU 以太网端口告警屏蔽开关 */

/*modi by luh 2014-07-18  Q.16592*/
static ULONG onuSwitchAlarmMask = EVENT_MASK_ETH_ALL;
/*static ULONG onuEthPortLinkAlarmLogEnable = 0;*/
LONG getOnuSwitchAlarmMask( ULONG *mask )
{
	if( mask )
	{
		VOS_SemTake( eventSemId, WAIT_FOREVER );
		*mask = (onuSwitchAlarmMask & EVENT_MAST_ONU_SWITCH_ALL);
		VOS_SemGive( eventSemId );
		return VOS_OK;
	}
	return VOS_ERROR;
}
LONG OnuSwitchAlarmMaskSet( ULONG mask )
{
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	onuSwitchAlarmMask = (mask & EVENT_MASK_ETH_ALL);
	VOS_SemGive( eventSemId );
	return VOS_OK;
}

LONG getOnuEthPortAlarmMask( ULONG *mask )
{
	if( mask )
	{
		VOS_SemTake( eventSemId, WAIT_FOREVER );
		*mask = (onuEthPortAlarmMask & EVENT_MASK_ETH_ALL);
		VOS_SemGive( eventSemId );
		return VOS_OK;
	}
	return VOS_ERROR;
}
LONG OnuEthPortAlarmMaskSet( ULONG mask )
{
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	onuEthPortAlarmMask = (mask & EVENT_MASK_ETH_ALL);
	VOS_SemGive( eventSemId );
	return VOS_OK;
}

LONG setOnuEthPortAlarmMask( ULONG mask )
{
	int rc;
	/*VOS_SemTake( eventSemId, WAIT_FOREVER );*/
	rc = setEthPortAlarmMask( 0,0,0, mask );
	/*VOS_SemGive( eventSemId );*/
	return rc;
}

QDEFUN( alarm_mask_onu_eth,
	alarm_mask_onu_eth_cmd,
	"alarm-mask onu-eth [all|link|fer|flr|ti|bcfc]",
	"alarm mask configuration\n"
	"ONU ETH alarm mask\n"
	"all alarm mask\n"
	"link up/down alarm mask\n"
	"Frame error Rate alarm mask\n"
	"Frame Loss  Rate alarm mask\n"
	"Traffic Intermit alarm mask\n"
	"Broadcast flood control\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;
	if( argc > 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	mask = alarm_mask_eth_parase( argc, argv );
	
	if( getOnuEthPortAlarmMask(&old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	if( setOnuEthPortAlarmMask(mask | old_mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_onu_eth,
	undo_alarm_mask_onu_eth_cmd,
	"undo alarm-mask onu-eth [all|link|fer|flr|ti|bcfc]",
	"delete information\n"
	"alarm mask configuration\n"
	"ONU ETH alarm mask\n"
	"all alarm mask\n"
	"link up/down alarm mask\n"
	"Frame ERROR Rate alarm mask\n"
	"Frame Loss Rate alarm mask\n"
	"Traffic Intermit alarm mask\n"
	"Broadcast flood control\n",
	&eventQueId )
{
	ULONG mask = 0, old_mask;

	if( argc > 1 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}

	mask = alarm_mask_eth_parase( argc, argv );

	if( getOnuEthPortAlarmMask(&old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
	mask = (old_mask & (~mask)) & EVENT_MASK_ETH_ALL;
	if( setOnuEthPortAlarmMask(mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_onu_eth,
	show_alarm_mask_onu_eth_cmd,
	"show alarm-mask onu-eth",
	"show information\n"
	"show alarm mask configuration\n"
	"ONU ETH alarm mask\n",
	&eventQueId )
{
#if 0
	ULONG mask = 0;
	char mask_str[64];
	
	if( argc != 0 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	if( getOnuEthPortAlarmMask(&mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
	VOS_MemZero( mask_str, sizeof(mask_str) );
	vty_out( vty, "ONU ETH port alarm mask:%s\r\n", alarm_eth_mask_bits_to_str(mask, mask_str) );
#endif
	alarm_mask_eth_display( 0, 0, 0, vty );
	
	return CMD_SUCCESS;
}

QDEFUN( alarm_mask_onu_switch,
	alarm_mask_onu_switch_cmd,
	"alarm-mask onu-switch",
	"alarm mask configuration\n"
	"ONU switch alarm mask\n",
	&eventQueId )
{
	ULONG mask = EVENT_MAST_ONU_SWITCH_ALL, old_mask = 0;
	
	if( getOnuSwitchAlarmMask(&old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	if( setOnuSwitchAlarmMask(0, 0, 0, mask|old_mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( undo_alarm_mask_onu_switch,
	undo_alarm_mask_onu_switch_cmd,
	"undo alarm-mask onu-switch",
	"delete information\n"
	"alarm mask configuration\n"
	"ONU switch alarm mask\n",
	&eventQueId )
{
	ULONG mask = EVENT_MAST_ONU_SWITCH_ALL, old_mask = 0;

	if( getOnuSwitchAlarmMask(&old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
	mask = old_mask & (~mask);
	if( setOnuSwitchAlarmMask(0, 0, 0, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

QDEFUN( show_alarm_mask_onu_switch,
	show_alarm_mask_onu_switch_cmd,
	"show alarm-mask onu-switch",
	"show information\n"
	"show alarm mask configuration\n"
	"ONU switch alarm mask\n",
	&eventQueId )
{
	ULONG mask = 0;

	if( getOnuSwitchAlarmMask(&mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_ERR_AMBIGUOUS;
	}
    else
    {
		vty_out( vty, "Onu Switch alarm mask:%s\r\n", mask?"ALL":"0" );
    }
	return CMD_SUCCESS;
}

/*add by shixh20091021*/

ULONG onu_type_mask[V2R1_ONU_MAX]/*={0 }*/;
extern short int parseOnuIndexFromDevIdx( const ULONG devIdx, ULONG * pPonIdx, ULONG *pOnuIdx );

LONG setOnuTypeAlarmMask(ULONG onu_type,ULONG mask)
{
#if 0		/* modified by xieshl 20110408 */
	if( (V2R1_ONU_GT811 > onu_type) || (onu_type >= V2R1_ONU_MAX) )
		return VOS_ERROR;

	onu_type_mask[onu_type] = mask;
	return VOS_OK;
#else
	return almStatusSetOnuTypeAlarmMask( onu_type, mask );
#endif
}

LONG getOnuTypeAlarmMask(ULONG onu_type,ULONG  *maskvalue)
{
  	if(maskvalue == NULL)
  	{
  		VOS_ASSERT(0);
		return  VOS_ERROR;
	}
	if( (V2R1_ONU_GT811 > onu_type) || (onu_type >= V2R1_ONU_MAX) )
		return VOS_ERROR;
	
	VOS_SemTake( eventSemId, WAIT_FOREVER );
	*maskvalue = (onu_type_mask[onu_type] & EVENT_MASK_DEV_ALL/*0xfffffffe*/);
	VOS_SemGive( eventSemId );
	
	return VOS_OK;
}
LONG getOnuTypeAlarmMaskByDevIdx( ULONG devIdx, ULONG *pMask )
{
	ULONG onutype = 0;
	/* modified by xieshl 20120927, 解决GT811_C告警屏蔽无效问题，问题单15896 */
	if( getDeviceType(devIdx, &onutype) == VOS_OK)
	{
		if(onutype == V2R1_ONU_CTC)
		{
			ULONG module = 0, type = 0;
			short int onuentry = 0;
			ULONG ponid, onuid;
			onuentry = parseOnuIndexFromDevIdx(devIdx, &ponid, &onuid);
			if( onuentry == VOS_ERROR )
			return VOS_ERROR;

			ONU_MGMT_SEM_TAKE
			module = OnuMgmtTable[onuentry].onu_model;
			ONU_MGMT_SEM_GIVE
			if(CTC_OnuModel_Translate(module, &type, NULL, NULL) == VOS_OK)
			{
				onutype = type;
			}
		}

		/*add by yanjy2016-12*/
		/*问题单33120*/
		else if(onutype == V2R1_ONU_GPON)
		{
			ULONG /*module = 0,*/ type = 0;
			short int onuentry = 0;
			ULONG ponid, onuid;

			 ULONG length = 0;
		        ULONG liv_typeid = 0;		        
		        char equipmentID[MAXEQUIPMENTIDLEN];
		        char onu_module[80] = "";
			
			onuentry = parseOnuIndexFromDevIdx(devIdx, &ponid, &onuid);
			if( onuentry == VOS_ERROR )
			return VOS_ERROR;

		        ONU_MGMT_SEM_TAKE
		        VOS_MemCpy(equipmentID, OnuMgmtTable[onuentry].DeviceInfo.equipmentID, MAXEQUIPMENTIDLEN);
		        ONU_MGMT_SEM_GIVE
		        
		        if(GPON_OnuModel_Translate(equipmentID, &liv_typeid, onu_module, &length) == VOS_OK)
		        {
		        	onutype = liv_typeid;
		        }
            	}

			#if 0
			ONU_MGMT_SEM_TAKE
			module = OnuMgmtTable[onuentry].onu_model;
			ONU_MGMT_SEM_GIVE
			sys_console_printf("\r\nGPON module = %d.\r\n",module);
			if(GPON_OnuModel_Translate(module, &type, NULL, NULL) == VOS_OK)
			{
				onutype = type;
				sys_console_printf("\r\nGPON onutype = %d.\r\n",onutype);
			}	
			#endif
		/*end*/

		
		/*if( onutype == V2R1_ONU_GT811_C )
			onutype = V2R1_ONU_GT811;*/
		return getOnuTypeAlarmMask( onutype, pMask );
	}
	return VOS_ERROR;
}

QDEFUN( alarm_mask_onu,
       alarm_mask_onu_cmd,
       "alarm-mask onu <onu_type> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power]",
	"alarm mask configuration\n"
	"onu alarm mask\n"
	"onu type\n"
	"all alarm mask\n"
	"onu device power alarm mask\n"
	"onu device fan alarm mask\n"
	"onu device cpu alarm mask\n"
	"onu device temperature alarm mask\n"
	"onu device register alarm mask\n"
	"onu device present alarm mask\n"
	"onu eth link alarm mask\n"
	"onu eth fer alarm mask\n"
	"onu eth flr alarm mask\n"
	"onu eth ti alarm mask\n"
	"onu eth loop alarm mask\n"
	"onu pon ber alarm mask\n"
	"onu pon fer alarm mask\n"
	"onu pon abnormal alarm mask\n"
	"onu pon aps alarm mask\n"
	"onu pon link alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"onu Optical Power Low or High alarm mask\n",
	&eventQueId
       )
{
	ULONG mask, old_mask=0;
	ULONG  onu_type;

	if( argc > 2 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	
	onu_type=onutypestring_to_int(argv[0]);
	if(onu_type==VOS_ERROR)
	{
		vty_out(vty, "onu_type is error\r\n");
		return CMD_WARNING;
	}
	
	mask = alarm_mask_device_parase( argc-1, &argv[1] );
	if( getOnuTypeAlarmMask(onu_type, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	if( setOnuTypeAlarmMask(onu_type, (mask | old_mask)) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	
	return CMD_SUCCESS;
}

/*static int alarm_mask_onu_type_display( struct vty *vty, ULONG onutype )
{
	ULONG maskvalue = 0;
	if( (getOnuTypeAlarmMask(onutype,&maskvalue) == VOS_ERROR) || (maskvalue == 0) )
	{
		vty_out(vty,"  no alarm mask!\r\n");
		return (CMD_WARNING);
	}

	if( maskvalue == EVENT_MASK_DEV_ALL )
	{
		vty_out(vty,"  all alarm mask!\r\n");
		return CMD_SUCCESS;
	}
		
 	if( maskvalue & 0x80000000 )	vty_out(vty," power");
 	if( maskvalue & 0x40000000 )	vty_out(vty," fan");
 	if( maskvalue & 0x20000000 )	vty_out(vty," cpu");
 	if( maskvalue & 0x10000000 )	vty_out(vty," temperature");
 	if( maskvalue & 0x08000000 )	vty_out(vty," register");
 	if( maskvalue & 0x04000000 )	vty_out(vty," present");
 	if( maskvalue & 0x02000000 )	vty_out(vty," eth-link");
 	if( maskvalue & 0x01000000 )	vty_out(vty," eth-fer");
 	if( maskvalue & 0x00800000 )	vty_out(vty," eth-flr");
 	if( maskvalue & 0x00400000 )	vty_out(vty," eth-ti");
 	if( maskvalue & 0x00200000 )	vty_out(vty," eth-loop");
 	if( maskvalue & 0x00100000 )	vty_out(vty," pon-ber");
 	if( maskvalue & 0x00080000 )	vty_out(vty," pon-fer");
 	if( maskvalue & 0x00040000 )	vty_out(vty," pon-abnormal");
 	if( maskvalue & 0x00020000 )	vty_out(vty," pon-aps");
 	if( maskvalue & 0x00010000 )	vty_out(vty," pon-link");
 	if( maskvalue & 0x00008000 )	vty_out(vty," laseralwayson");
 	if( maskvalue & 0x00004000 )	vty_out(vty," opticalpowerlow");
 	if( maskvalue & 0x00002000 )	vty_out(vty," opticalpowerhigh");
	vty_out(vty,"\r\n");

	return CMD_SUCCESS;
}*/

QDEFUN( alarm_mask_onu_show,
	alarm_mask_onu_show_cmd,
	"show alarm-mask onu {<onu_type>}*1",
	"show information\n"
	"show alarm mask configuration\n"
	"show onu alarm mask configuration\n"
	"input onu type\n",
	&eventQueId
	)
{
	ULONG  onutype;
	char	    *pOnuTypeStr;
	ULONG   maskvalue=0;
	char mask_str[256];
	
	vty_out(vty, " %-19s%s\r\n", "onu type", "mask");
	
	if(argc==1)
	{
		/*if(SearchOnuTypeFullMatch(argv[0]) == RERROR)
		{
			NotificationOnuTypeString(vty);	
			return(CMD_WARNING);
		}*/
		
		onutype = onutypestring_to_int(argv[0]);
		if(onutype == VOS_ERROR)
		{
			vty_out(vty, "onu_type is error\r\n");
			return CMD_WARNING;
		}

		/*alarm_mask_onu_type_display( vty, onutype );*/
		if( (getOnuTypeAlarmMask(onutype, &maskvalue) == VOS_ERROR) || (maskvalue == 0) )
		{
			return (CMD_WARNING);
		}
		vty_out( vty, " %-18s%s\r\n", argv[0], alarm_dev_mask_bits_to_str(maskvalue, mask_str) );
	}
	else
	{
		for( onutype=V2R1_ONU_GT811; onutype<V2R1_ONU_MAX; onutype++ )
		{
			/*pOnuTypeStr=GetDeviceTypeString( onutype );
			if( VOS_MemCmp( pOnuTypeStr, DEVICE_TYPE_NAME_UNKNOWN_STR, 6) == 0 )
				continue;*/
				
			maskvalue = 0;
			if( getOnuTypeAlarmMask(onutype, &maskvalue) == VOS_ERROR )
			{
				continue;
			}
			if( maskvalue == 0 )
				continue;

			pOnuTypeStr = onutypeint_to_str(onutype);
			if( (pOnuTypeStr == NULL) || (VOS_StrCmp(pOnuTypeStr, DEVICE_TYPE_NAME_UNKNOWN_STR) == 0) )
				continue;

			vty_out( vty, " %-18s%s\r\n", pOnuTypeStr, alarm_dev_mask_bits_to_str(maskvalue, mask_str) );
			/*alarm_mask_onu_type_display( vty, i );*/
		}
	}
	
	return CMD_SUCCESS;
}


QDEFUN( delete_alarm_mask_onu,
       delete_alarm_mask_onu_cmd,
       "undo alarm-mask onu <onu_type> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|pon-power]",
	"delete information\n"
	"delete alarm mask configuration\n"
	"delete onu alarm mask configuration\n"
	"onu alarm mask\n"
	"all alarm mask\n"
	"undo onu device power alarm mask\n"
	"undo onu device fan alarm mask\n"
	"undo onu device cpu alarm mask\n"
	"undo onu device temperature alarm mask\n"
	"undo onu device register alarm mask\n"
	"undo onu device present alarm mask\n"
	"undo onu eth link alarm mask\n"
	"undo onu eth fer alarm mask\n"
	"undo onu eth flr alarm mask\n"
	"undo onu eth ti alarm mask\n"
	"undo onu eth loop alarm mask\n"
	"undo onu pon ber alarm mask\n"
	"undo onu pon fer alarm mask\n"
	"undo onu pon abnormal alarm mask\n"
	"undo onu pon aps alarm mask\n"
	"undo onu pon link alarm mask\n"
	"undo onu Laser Alway On alarm mask\n"
	"undo onu Optical Low or Power High alarm mask\n",
	&eventQueId
       )
{
	ULONG mask, old_mask=0,maskid;
	ULONG  onu_type;

	if( argc > 2 )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	
	onu_type=onutypestring_to_int(argv[0]);
	maskid = alarm_mask_device_parase( argc-1, &argv[1] );
	/*maskid = alarm_mask_onu_parase( argv[1] );*/
	
	if( getOnuTypeAlarmMask(onu_type, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	mask = (~maskid) & old_mask;
	
	setOnuTypeAlarmMask(onu_type, mask );
	
	return CMD_SUCCESS;
}

#if 0
/*add by shixh20091203*/
ULONG alarm_mask_onu_m_parase( int argc, char** argv)
{
	int i=1;
	ULONG mask = 0;

	if( argc == 1 )
	{
		mask = EVENT_MASK_DEV_ALL;
	}
	else
	{
	    	if( VOS_MemCmp(argv[i], "all", 3) == 0 )
			mask = EVENT_MASK_DEV_ALL;
		else if( VOS_MemCmp(argv[i], "pow", 3) == 0 )
			mask |= EVENT_MASK_DEV_POWER;
		else if( VOS_MemCmp(argv[i], "fan", 3) == 0 )
			mask |= EVENT_MASK_DEV_FAN;
		else if( VOS_MemCmp(argv[i], "cpu", 3) == 0 )
			mask |= EVENT_MASK_DEV_CPU;
		else if( VOS_MemCmp(argv[i], "tem", 3) == 0 )
			mask |= EVENT_MASK_DEV_TEMPERATURE;
		else if( VOS_MemCmp(argv[i], "reg", 3) == 0 )
			mask |= EVENT_MASK_DEV_REGISTER;
		else if( VOS_MemCmp(argv[i], "pre", 3) == 0 )
			mask |= EVENT_MASK_DEV_PRESENT;
		else if( VOS_MemCmp(argv[i], "eth-link", 8) == 0 )
			mask |= EVENT_MASK_DEV_ETH_LINK;
		else if( VOS_MemCmp(argv[i], "eth-fer", 7) == 0 )
			mask |= EVENT_MASK_DEV_ETH_FER;
		else if( VOS_MemCmp(argv[i], "eth-flr", 7) == 0 )
			mask |= EVENT_MASK_DEV_ETH_FLR;
		else if( VOS_MemCmp(argv[i], "eth-ti", 6) == 0 )
			mask |= EVENT_MASK_DEV_ETH_TI;
		else if( VOS_MemCmp(argv[i], "eth-loop", 8) == 0 )
			mask |= EVENT_MASK_DEV_ETH_LOOP;
		else if( VOS_MemCmp(argv[i], "pon-ber", 7) == 0 )
			mask |= EVENT_MASK_DEV_PON_BER;
		else if( VOS_MemCmp(argv[i], "pon-fer", 7) == 0 )
			mask |= EVENT_MASK_DEV_PON_FER;
		else if( VOS_MemCmp(argv[i], "pon-abn", 7) == 0 )
			mask |= EVENT_MASK_DEV_PON_ABNORMAL;
		else if( VOS_MemCmp(argv[i], "pon-aps", 7) == 0 )
			mask |= EVENT_MASK_DEV_PON_ABS;
		else if( VOS_MemCmp(argv[i], "pon-link", 8) == 0 )	
			mask |= EVENT_MASK_DEV_PON_LINK;
		else if( VOS_MemCmp(argv[i], "laseralwayon", 12) == 0 )
			mask |= EVENT_MASK_DEV_ONU_LASER_ON;
		else if( VOS_MemCmp(argv[i], "opticalpowerlow", 15) == 0 )
			mask |= EVENT_MASK_DEV_PON_POWER_L;
		else if( VOS_MemCmp(argv[i], "opticalpowerhigh", 16) == 0 )
			mask |= EVENT_MASK_DEV_PON_POWER_H;
	}
	return mask;
}

#if 0
extern ULONG ONU_MASK[20][64];
#else
extern ULONG (*ONU_MASK)[MAXONUPERPON];
#endif

QDEFUN( alarm_mask_onu_m,
       alarm_mask_onu_m_cmd,
       "alarm-mask onuId <slot/port/onuid> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|opticalpowerlow|opticalpowerhigh]",
	"alarm mask configuration\n"
	"onu alarm mask\n"
       "onu index\n"
	"all alarm mask\n"
	"onu device power alarm mask\n"
	"onu device fan alarm mask\n"
	"onu device cpu alarm mask\n"
	"onu device temperature alarm mask\n"
	"onu device register alarm mask\n"
	"onu device present alarm mask\n"
	"onu eth link alarm mask\n"
	"onu eth fer alarm mask\n"
	"onu eth flr alarm mask\n"
	"onu eth ti alarm mask\n"
	"onu eth loop alarm mask\n"
	"onu pon ber alarm mask\n"
	"onu pon fer alarm mask\n"
	"onu pon abnormal alarm mask\n"
	"onu pon aps alarm mask\n"
	"onu pon link alarm mask\n"
	"onu Laser Alway On alarm mask\n"
	"onu Optical Power Low alarm mask\n"
	"onu Optical Power High alarm mask\n",
	&eventQueId
       )
{
	ULONG mask, old_mask=0;
	ULONG devidx, slot, port, onuId;
	
 	if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK )
		return CMD_WARNING;
	devidx = slot * 10000 + port * 1000 + onuId;
	
	mask = alarm_mask_onu_m_parase( argc, argv);

	if(getonu_AlarmMask( devidx, &old_mask )==VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if(setonu_AlarmMask(devidx,(mask|old_mask))==VOS_ERROR)
	{
		return CMD_ERR_AMBIGUOUS;
	}

	return CMD_SUCCESS;
}

QDEFUN( alarm_mask_onu_m_show,
       alarm_mask_onu_m_show_cmd,
       "show alarm-mask onuId {<slot/port/onuid>}*1",
	"show information\n"
	"show alarm mask configuration\n"
	"show onu alarm mask configuration\n"
	"input onu type\n",
       &eventQueId
       )
{
	ULONG  i,j;
	ULONG   maskvalue=0;
	ULONG devidx, slot, port, onuId;
	
 	
	
	/*vty_out(vty,"onu-L-A-O:onu Laser Alway On\r\n");
	vty_out(vty,"onu-O-P-L:onu Optical Power Low\r\n");
	vty_out(vty,"onu-O-P-H:onu Optical Power High\r\n");*/
	vty_out(vty,"onuIdx             mask\r\n");
	
	
	if(argc==1)
	{
		if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK )
			return CMD_WARNING;
		
		devidx = slot * 10000 + port * 1000 + onuId;
		vty_out(vty,"%s%12s", "onu", argv[0]);
#if 0	
		if(getonu_AlarmMask( devidx, &maskvalue )==VOS_ERROR)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}
		if(maskvalue==0)
		{
			vty_out(vty,"  no alarm mask!\r\n");
			return (CMD_WARNING);
		}
		if( maskvalue==EVENT_MASK_DEV_ALL )
	    	{
			vty_out(vty,"	  all alarm mask!\r\n");
			return CMD_WARNING;
		}
		
		if((maskvalue&0x80000000)==0x80000000)
			 vty_out(vty,"  power");
		if((maskvalue&0x40000000)==0x40000000)
			 vty_out(vty,"  fan");
		if((maskvalue&0x20000000)==0x20000000)
			 vty_out(vty,"  cpu");
		if((maskvalue&0x10000000)==0x10000000)
			 vty_out(vty,"  temperature");
		if((maskvalue&0x08000000)==0x08000000)
			 vty_out(vty,"  register");
		if((maskvalue&0x04000000)==0x04000000)
			 vty_out(vty,"  present");
		if((maskvalue&0x02000000)==0x02000000)
			 vty_out(vty,"  eth-link");
		if((maskvalue&0x01000000)==0x01000000)
			 vty_out(vty,"  eth-fer");
		if((maskvalue&0x00800000)==0x00800000)
			 vty_out(vty,"  eth-flr");
		if((maskvalue&0x00400000)==0x00400000)
			 vty_out(vty,"  eth-ti");
		if((maskvalue&0x00200000)==0x00200000)
			 vty_out(vty,"  eth-loop");
		if((maskvalue&0x00100000)==0x00100000)
			 vty_out(vty,"  pon-ber");
		if((maskvalue&0x00080000)==0x00080000)
			 vty_out(vty,"  pon-fer");
		if((maskvalue&0x00040000)==0x00040000)
			 vty_out(vty,"  pon-abnormal");
		if((maskvalue&0x00020000)==0x00020000)
			 vty_out(vty,"  pon-aps");
		if((maskvalue&0x00010000)==0x00010000)
			 vty_out(vty,"  pon-link");
		if((maskvalue&0x00008000)==0x00008000)
			 vty_out(vty,"  onu-L-A-O");
		if((maskvalue&0x00004000)==0x00004000)
			 vty_out(vty,"  onu-O-P-L");
		if((maskvalue&0x00002000)==0x00002000)
			 vty_out(vty,"  onu-O-P-H");

		vty_out(vty,"\r\n");
#else
		alarm_mask_onu_display( vty, devidx );
#endif
	}
	else
	{
		for(i=0;i<MAXPONCHIP;i++)
		{
			slot = GetCardIdxByPonChip(i);
			port = GetPonPortByPonChip(i);
			for(j=0;j<MAXONUPERPON;j++)
			{
				devidx = slot * 10000 + port * 1000 + (j+1);
				vty_out(vty," %s%d/%d/%8d", "onu", slot, port, j+1 );
#if 0
				getonu_AlarmMask(devidx,&maskvalue);
							
				if(maskvalue==0)
				{
					/*vty_out(vty,"   all no mask!\r\n");*/
					continue;
				}
				vty_out(vty," %-16d",devidx);

				displayOnuAlarmMask( vty, maskvalue );
				
							/*if(maskvalue==0)
							{
								continue;
							}
							vty_out(vty," %-16d",devidx);
							if( maskvalue==EVENT_MASK_DEV_ALL )
						    	{
								vty_out(vty,"    all alarm mask!\r\n");
								continue;
							}
							
							if((maskvalue&0x80000000)==0x80000000)
								 vty_out(vty,"	power");
							
							if((maskvalue&0x40000000)==0x40000000)
								 vty_out(vty,"	fan");
							
							if((maskvalue&0x20000000)==0x20000000)
								 vty_out(vty,"	cpu");
							
							if((maskvalue&0x10000000)==0x10000000)
								 vty_out(vty,"	temperature");
							
							if((maskvalue&0x08000000)==0x08000000)
								 vty_out(vty,"	register");
							
							if((maskvalue&0x04000000)==0x04000000)
								 vty_out(vty,"	present");
							
							if((maskvalue&0x02000000)==0x02000000)
								 vty_out(vty,"	eth-link");
							
							if((maskvalue&0x01000000)==0x01000000)
								 vty_out(vty,"	eth-fer");
							
							if((maskvalue&0x00800000)==0x00800000)
								 vty_out(vty,"	eth-flr");
							
							if((maskvalue&0x00400000)==0x00400000)
								 vty_out(vty,"	eth-ti");
							
							if((maskvalue&0x00200000)==0x00200000)
								 vty_out(vty,"	eth-loop");
							
							if((maskvalue&0x00100000)==0x00100000)
								 vty_out(vty,"	pon-ber");
							
							if((maskvalue&0x00080000)==0x00080000)
								 vty_out(vty,"	pon-fer");
							
							if((maskvalue&0x00040000)==0x00040000)
								 vty_out(vty,"	pon-abnormal");

							if((maskvalue&0x00020000)==0x00020000)
								 vty_out(vty,"	pon-aps");

							if((maskvalue&0x00010000)==0x00010000)
								 vty_out(vty,"	pon-link");

							if((maskvalue&0x00008000)==0x00008000)
								 vty_out(vty,"	onu-L-A-O");

							if((maskvalue&0x00004000)==0x00004000)
								 vty_out(vty,"	onu-O-P-L");

							if((maskvalue&0x00002000)==0x00002000)
								 vty_out(vty,"	onu-O-P-H");

								vty_out(vty,"\r\n");*/
#else
				alarm_mask_onu_display( vty, devidx );
#endif
			}
		}
	}
	
	return CMD_SUCCESS;
}
QDEFUN( delete_alarm_mask_onu_m,
       delete_alarm_mask_onu_m_cmd,
       "undo alarm-mask onuId <slot/port/onuid> [all|power|fan|cpu|temperature|register|present|eth-link|eth-fer|eth-flr|eth-ti|eth-loop|pon-ber|pon-fer|pon-abnormal|pon-aps|pon-link|laseralwayon|opticalpowerlow|opticalpowerhigh]",
	"delete information\n"
	"delete alarm mask configuration\n"
	"delete onu alarm mask configuration\n"
	"onuidx  alarm mask\n"
	"all alarm mask\n"
	"undo onu device power alarm mask\n"
	"undo onu device fan alarm mask\n"
	"undo onu device cpu alarm mask\n"
	"undo onu device temperature alarm mask\n"
	"undo onu device register alarm mask\n"
	"undo onu device present alarm mask\n"
	"undo onu eth link alarm mask\n"
	"undo onu eth fer alarm mask\n"
	"undo onu eth flr alarm mask\n"
	"undo onu eth ti alarm mask\n"
	"undo onu eth loop alarm mask\n"
	"undo onu pon ber alarm mask\n"
	"undo onu pon fer alarm mask\n"
	"undo onu pon abnormal alarm mask\n"
	"undo onu pon aps alarm mask\n"
	"undo onu pon link alarm mask\n"
	"undo onu Laser Alway On alarm mask\n"
	"undo onu Optical Power Low alarm mask\n"
	"undo onu Optical Power High alarm mask\n",
	&eventQueId
       )
{
	ULONG mask, old_mask=0,maskid;
	ULONG PonPortIdx,devidx, slot, port, onuId;
	
 	if( PON_ParseSlotPortOnu( argv[0], &slot, &port, &onuId )  != VOS_OK )
		return CMD_WARNING;
	devidx = slot * 10000 + port * 1000 + onuId;
	PonPortIdx=GetPonPortIdxBySlot(slot, port);
	
	maskid = alarm_mask_onu_m_parase( argc, argv);
	
	if( getonu_AlarmMask(devidx, &old_mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	
	/*mask=(~maskid)&(maskid|old_mask);*/mask=(~maskid)&old_mask;


	ONU_MASK[PonPortIdx][onuId-1]=mask;
		
	/*if(setonu_AlarmMask(devidx,mask)==VOS_ERROR)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}*/
	
	return CMD_SUCCESS;
}
#endif


#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
/*add by shixh@20071226*/
ULONG alarm_mask_e1_parase( int argc, char** argv)
{
	int i;
	ULONG mask = 0;
	
	for( i=1; i<argc; i++ )
	{
	    	if( VOS_MemCmp(argv[i], "all", 3) == 0 )
	    	{
			mask = E1_ALM_ALL;
			break;
		}
		else if( VOS_StriCmp(argv[i], "los") == 0 )
			mask |= E1_ALM_LOS;
		else if( VOS_StriCmp(argv[i], "lof") == 0 )
			mask |= E1_ALM_LOF;
		else if( VOS_StriCmp(argv[i], "ais") == 0 )
			mask |= E1_ALM_AIS;
		else if( VOS_StriCmp(argv[i], "rai") == 0 )
			mask |= E1_ALM_RAI;
		else if( VOS_StriCmp(argv[i], "smf") == 0 )
			mask |= E1_ALM_SMF;
		else if( VOS_StriCmp(argv[i], "lofsmf") == 0 )
			mask |= E1_ALM_LOFSMF;
		else if( VOS_StriCmp(argv[i], "crc3") == 0 )
			mask |= E1_ALM_CRC3;
		else if( VOS_StriCmp(argv[i], "crc6") == 0 )
			mask |= E1_ALM_CRC6;
	}
	return mask;
}

char *alarm_e1_mask_to_str( UCHAR mask, char *str )
{
	if( str == NULL )
		return str;
	if( (mask & E1_ALM_ALL) == 0 )
	{
		VOS_StrCpy( str, "NO");
		return str;
	}
	str[0] = 0;
	if( mask & E1_ALM_LOS )
		VOS_StrCat( str,"LOS|");
	if( mask & E1_ALM_LOF )
		VOS_StrCat(str,"LOF|");
	if( mask & E1_ALM_AIS)
		VOS_StrCat(str, "AIS|");
	if( mask & E1_ALM_RAI)
		VOS_StrCat(str, "RAI|");
	if( mask &  E1_ALM_SMF)
		VOS_StrCat(str, "SMF|");
	if( mask &  E1_ALM_LOFSMF)
		VOS_StrCat(str, "LOFSMF|");
	if( mask & E1_ALM_CRC3)
		VOS_StrCat(str, "CRC3|");
	if( mask &  E1_ALM_CRC6)
		VOS_StrCat(str, "CRC6|");

	if( VOS_StrLen(str) > 3 )
		str[VOS_StrLen(str)-1] = 0;
	
	return str;
};

/*add by shixh@20071226*/
DEFUN( alarm_mask_olt_e1,
       alarm_mask_olt_e1_cmd,
       "alarm-mask olt-e1 <port_list> [all|los|lof|ais|rai|smf|lofsmf|crc3|crc6]",
	"alarm mask configuration\n"
	"OLT E1 alarm mask\n"
	"port list eg:1,2,6-9\n"
	"All  alarm mask\n"
	"LOS alarm mask\n"
	"LOF alarm mask\n"
	"AIS alarm mask\n"
 	"RAI alarm mask\n"
 	"SMF alarm mask\n"
 	"LOFSMF alarm mask\n"
 	"CRC3 alarm mask\n"
 	"CRC6 alarm mask\n"
       )
{
	ULONG  e1port;
	ulong idxs[3]={0};
	uchar_t  mask=0,old_mask/*,status*/;
	/*ULONG cur_base_mask;*/
	
	if( (argc > 10) ||(argc < 2) )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
		
	mask=alarm_mask_e1_parase(argc, argv);	/* modified by xieshl 20080401 */

	if( VOS_StriCmp(argv[0], "all") == 0 )
	{
	   	for( e1port=1; e1port<=MAX_E1_PORT_NUM; e1port++ )/*modified by shixh@20080729*/
	   	{
	   		idxs[0]=OLT_DEV_ID;
			idxs[1]=0;
			idxs[2]=e1port;
			
			if( GetE1AlarmMask(e1port,&old_mask) == VOS_ERROR )
			{
				vty_out( vty, "%% Parameter is error.\r\n" );
				return CMD_WARNING;
			}
			
			if(SetE1AlarmMask(idxs,mask | old_mask)==VOS_ERROR)
				return CMD_ERR_AMBIGUOUS;
	 	}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], e1port )
		{
			if( (e1port < 1) || (e1port > MAX_E1_PORT_NUM) )
			{
				vty_out(vty, "out of range,the range is 1~24\r\n");	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		}		
		END_PARSE_PORT_LIST_TO_PORT();*/	/* removed by xieshl 20120906, 解决内存丢失问题，同时非法端口不处理，但不报错，下同 */
                         
		/*mask=alarm_mask_e1_parase(argc, argv);*/
                     
		BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], e1port )
		if( (e1port != 0) && (e1port <= MAX_E1_PORT_NUM) )
		{
			idxs[0]=OLT_DEV_ID;
			idxs[1]=0;
			idxs[2]=e1port;
			
			if(GetE1AlarmMask(e1port,&old_mask)==VOS_ERROR)
			{
				vty_out( vty, "  %% Parameter is error.\r\n" );
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
				
			if( SetE1AlarmMask(idxs, (mask|old_mask))==VOS_ERROR)
			{
				vty_out( vty, " set e1 mask fail!!\r\n" );
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_ERR_AMBIGUOUS );
			}
			/*else
				vty_out( vty, "set success!%x\r\n",mask);*/
		}
		END_PARSE_PORT_LIST_TO_PORT();
		
	}
		
	return CMD_SUCCESS;
}
/*end add by shixh@20071226*/
DEFUN( undo_alarm_mask_olt_e1,
       undo_alarm_mask_olt_e1_cmd,
       "undo alarm-mask olt-e1 <port_list> [all|los|lof|ais|rai|smf|lofsmf|crc3|crc6]",
	"alarm mask configuration\n"
	"alarm mask configuration\n"
	"OLT E1 alarm mask\n"
	"port list eg:1,2,3-6\n"
	"All alarm mask\n"
	"LOS alarm mask\n"
	"LOF alarm mask\n"
	"AIS alarm mask\n"
 	"RAI alarm mask\n"
 	"SMF alarm mask\n"
 	"LOFSMF alarm mask\n"
 	"CRC3 alarm mask\n"
 	"CRC6 alarm mask\n"
       )
{
	ULONG  e1port;
	ulong idxs[3]={0};
	uchar_t  mask=0,old_mask;
	if( (argc>10) || (argc<2) )
	{
		return CMD_ERR_EXEED_ARGC_MAX;
	}
	mask=alarm_mask_e1_parase(argc, argv);	/* modified by xieshl 20080401 */

	if (VOS_StriCmp(argv[0], "all")==0)
	{
		/*mask = alarm_mask_e1_parase(argc, argv);*/
		for(e1port=1;e1port<=MAX_E1_PORT_NUM;e1port++)
	 	{
	 		idxs[0]=OLT_DEV_ID;
			idxs[1]=0;
			idxs[2]=e1port;
			
		 	if(GetE1AlarmMask(e1port,&old_mask)==VOS_ERROR)
			{
				vty_out( vty, "%% Parameter is error.\r\n" );
				return CMD_WARNING;
			}
				
			if(SetE1AlarmMask(idxs, ((old_mask & (~mask)) & E1_ALM_ALL))==VOS_ERROR)
				return CMD_ERR_AMBIGUOUS;
			
	 	}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], e1port )
			if( (e1port<1) || (e1port>MAX_E1_PORT_NUM) )
			{
				vty_out(vty, "out of range,the range is 1~24\r\n");	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/

		/*mask=alarm_mask_e1_parase(argc, argv);*/

		BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], e1port)
		if( (e1port != 0) && (e1port <= MAX_E1_PORT_NUM) )
		{
			idxs[0]=OLT_DEV_ID;
			idxs[1]=0;
			idxs[2]=e1port;
			
			if(GetE1AlarmMask(e1port,&old_mask)==VOS_ERROR)
			{
				vty_out( vty, "  %% Parameter is error.\r\n" );
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_WARNING );
			}
			/*mask = ((old_mask & (~mask)) & E1_ALM_ALL);*/
			if(SetE1AlarmMask(idxs, ((old_mask & (~mask)) & E1_ALM_ALL))==VOS_ERROR)
			{
				RETURN_PARSE_PORT_LIST_TO_PORT( CMD_ERR_AMBIGUOUS );
			}
		}
		END_PARSE_PORT_LIST_TO_PORT();
	
	}
	
	return CMD_SUCCESS;
}

DEFUN( show_alarm_mask_olt_e1,
       show_alarm_mask_olt_e1_cmd,
       "show alarm-mask olt-e1 {<port_list>}*1",
	"show information\n"
	"show alarm mask configuration\n"
	"tdm e1 alarm mask\n"
	"port list eg:1,2,3-6\n"
       )
{
	uchar_t  mask = 0;
	ULONG  e1port;
	char str[64];
	VOS_MemZero(str, sizeof(str));
	
	if( argc == 0 )
	{
		for(e1port=1;e1port<=MAX_E1_PORT_NUM;e1port++)
		{
	       	if(GetE1AlarmMask(e1port,&mask)==VOS_OK)
				vty_out( vty, "E1 port%d alarm mask:%s\r\n", e1port,alarm_e1_mask_to_str(mask,str) );
			else	  
				return CMD_ERR_AMBIGUOUS;
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], e1port )
			if( (e1port<1) || (e1port>MAX_E1_PORT_NUM) )
			{
				vty_out(vty, "out of range,the range is 1~24\r\n");	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/

		/*mask=alarm_mask_e1_parase(argc, argv);*/

		BEGIN_PARSE_PORT_LIST_TO_PORT(argv[0], e1port)
		{
			 if(GetE1AlarmMask(e1port,&mask)==VOS_OK)
				vty_out( vty, "E1 port%d alarm mask:%s\r\n", e1port,alarm_e1_mask_to_str(mask,str) );
			else	
				{
					RETURN_PARSE_PORT_LIST_TO_PORT( CMD_ERR_AMBIGUOUS );
				}
				
				
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	
	return CMD_SUCCESS;
}

DEFUN( alarm_mask_onu_tdm_service,
       alarm_mask_onu_tdm_service_cmd,
       "alarm-mask onu-tdm-service",
	"alarm mask configuration\n"
	"onu tdm service\n"
       )
{
	ULONG  mask = TDM_BASE_ALM_OOS;
	
	/*if( GetTdmMaskBase(&mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	mask |= TDM_ALM_OOS;*/
	if( SetTdmMaskBase(2, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

DEFUN( undo_alarm_mask_onu_tdm_service,
       undo_alarm_mask_onu_tdm_service_cmd,
       "undo alarm-mask onu-tdm-service",
	"alarm mask configuration\n"
	"onu tdm service\n"
	"undo onu tdm service\n"
       )
{
	ULONG  mask = 0;
	
	/*if( GetTdmMaskBase(&mask) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	mask &= ((~TDM_ALM_OOS) & (E1_ALM_ALL | TDM_ALM_OOS));*/
	
	if( SetTdmMaskBase(2, mask) == VOS_ERROR )
	{
		return CMD_ERR_AMBIGUOUS;
 	}
	return CMD_SUCCESS;
}

DEFUN( show_alarm_mask_onu_tdm_service,
       show_alarm_mask_onu_tdm_service_cmd,
       "show alarm-mask onu-tdm-service",
	"alarm mask configuration\n"
	"onu tdm service\n"
	"show onu tdm service\n"
       )
{
	ULONG   mask;
	char  *maskstatus[]={"disable","enable"};
	
	if( GetTdmMaskBase(&mask) == VOS_OK )
	{
		if( mask & TDM_BASE_ALM_OOS )	/* modified by xieshl 20080401 */
			mask = 1;
		else
			mask = 0;
		vty_out( vty, " ONU TDM service alarm mask is:%s\r\n", maskstatus[mask]);
	}
	else
		vty_out( vty, " get ONU TDM service alarm mask fail!!\r\n");
	return CMD_SUCCESS;
}
/*end add by shixh@20071226*/
#endif	/* EPON_MODULE_TDM_SERVICE */

#if 0	/* removed by xieshl 20110418 */
/*add by shixh20091211*/
extern ULONG get_gfa_e1_slotno();
static int all_olt_alarm_mask_device_display(ULONG devidx_olt, struct vty *vty)
{
	ULONG mask_device= 0;
	uchar mask_e1=0,e1port;
	ULONG mask_eth=0;
	ULONG mask_pon=0;
	char mask_str[256];
	ULONG slot=1,port;
	ULONG pon_slot,pon_port;
	
	/*显示OLT device mask*/
	if( getDeviceAlarmMask(devidx_olt, &mask_device) == VOS_ERROR )
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}
	if(mask_device!=0)
		vty_out( vty, "DEV:olt, alarm mask:%s\r\n",  alarm_dev_mask_bits_to_str(mask_device, mask_str) );

	/*显示OLT E1 alarm-mask*/
	
	if (0 != get_gfa_e1_slotno())
	{
		for(e1port=1;e1port<=MAX_E1_PORT_NUM;e1port++)
		{
		       if(GetE1AlarmMask(e1port,&mask_e1)==VOS_OK)
				vty_out( vty, "E1 port%d alarm mask:%s\r\n", e1port,alarm_e1_mask_to_str(mask_e1, mask_str) );
			else	  
				return CMD_ERR_AMBIGUOUS;
		}
	}
	
         /*显示OLT eth alarm-mask*/
	for(port=1;port<5;port++)
	{
		if( getEthPortAlarmMask(devidx_olt, slot, port, &mask_eth) == VOS_ERROR )
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}
		if(mask_eth!=0)
		{
			vty_out( vty, "DEV:olt, eth:%d/%d alarm mask:%s\r\n", slot, port, alarm_eth_mask_bits_to_str(mask_eth, mask_str) );
			break;
		}
	}
	
	/*显示OLT PON alarm-mask*/
	for(pon_slot=4;pon_slot<9;pon_slot++)
	{
		if(SlotCardIsPonBoard(pon_slot)==RERROR)
			continue;
		
		for(pon_port=1;pon_port<5;pon_port++)
		{		
			if(PonCardSlotPortCheckWhenRunningByVty(pon_slot, pon_port,vty) != ROK)
				continue;
			
			if( getPonPortAlarmMask(devidx_olt, pon_slot, pon_port, &mask_pon) == VOS_ERROR )
			{
				vty_out( vty, "  %% Parameter is error.\r\n" );
				return CMD_WARNING;
			}
			if(mask_pon!=0)
				vty_out( vty, "DEV:olt, PON:%d/%d alarm mask:%s\r\n", pon_slot, pon_port, alarm_pon_mask_bits_to_str(mask_pon, mask_str) );
		}
	}
	
	/*这样做不对，后两个的值不唯一*/
	if((mask_device==0)&&(mask_eth==0)&&(mask_pon==0))
		vty_out(vty,"olt no alarmMask!\r\n");
	
 	return CMD_SUCCESS;
}

static int all_onu_alarm_mask_device_display(ULONG devidx, struct vty *vty)
{
	int slot,port;
	ULONG mask_device=0,mask_pon=0;
	char mask_str[256];
	
	slot = GET_PONSLOT(devidx);
	port= GET_PONPORT(devidx);
		
	/*显示ONU DEVICE alarm-mask*/
	getDeviceAlarmMask(devidx, &mask_device);
	if(mask_device!=0)
		vty_out( vty, "DEV:%d alarm mask:%s\r\n", devidx, alarm_dev_mask_bits_to_str(mask_device, mask_str) );
	
		
	/*显示ONU  PON alarm-mask*/
	getPonPortAlarmMask(devidx, slot, port, &mask_pon);
	if(mask_pon!=0)
		vty_out( vty, "DEV:%d, PON:%d/%d alarm mask:%s\r\n", devidx, slot, port, alarm_pon_mask_bits_to_str(mask_pon, mask_str) );

	return CMD_SUCCESS;	
}
#endif

QDEFUN( alarm_mask_show_all,
	alarm_mask_show_all_cmd,
	"show alarm-mask-all",
	"show mask configuration\n"
	"show all olt  onu alarm mask\n",
	&eventQueId)
{
#if 0		/* modified by xieshl 20110418, PR008808 */
	int i,j;
	ULONG devidx,slot,port;
	ULONG mask_eth=0,mask_tdm=0;
	char  *maskstatus[]={"disable","enable"};
	char mask_str[64];

	if (VOS_StrCmp( argv[0], "olt")==0)
	{
		all_olt_alarm_mask_device_display(1,vty);
	}
	else
		{
			for(i=0;i<MAXPONCHIP;i++)
			{
				slot = GetCardIdxByPonChip(i);
				port = GetPonPortByPonChip(i);
				for(j=0;j<64;j++)
				{
					devidx = MAKEDEVID( slot, port, (j+1) );
							
					all_onu_alarm_mask_device_display(devidx,vty);
				}
			}
		
			/*显示ONU ETH alarm-mask*/
			getOnuEthPortAlarmMask(&mask_eth);
			if(mask_eth!=0)
				vty_out( vty, "ONU ETH port alarm mask:%s\r\n", alarm_eth_mask_bits_to_str(mask_eth, mask_str) );
			
			/*显示 ONU TDM sevice*/
			if( GetTdmMaskBase(&mask_tdm) == VOS_OK )
			{
				if( mask_tdm & TDM_BASE_ALM_OOS )	/* modified by xieshl 20080401 */
					mask_tdm = 1;
				else
					mask_tdm = 0;
				
				if(mask_tdm!=0)
					vty_out( vty, "ONU TDM service alarm mask is:%s\r\n", maskstatus[mask_tdm]);
			}
			else
				vty_out( vty, " get ONU TDM service alarm mask fail!!\r\n");
			
			/*显示PON CNI alarm-mask*/
			vty_out( vty, "pon-cni alarm mask: %s\r\n", (alarm_mask_pon_cni_bits & 1) ? "ber" : "null" );
		}
#else
	show_all_alarm_mask( vty );
#endif

   return  CMD_SUCCESS;

}

/* added by xieshl 20080424, 告警日志过滤 */
DEFUN( alarm_log_filter,
       alarm_log_filter_cmd,
       "alarm-log filter {[onu-eth-link]}*1",
	"config filter\n"
	"config alarm log filter\n"
	"filter ONU ETH port linkup/down\n"
       )
{
	addEventLogDataFilterIterm(ALM_FILTER_ONU_ETH_LINK);
	return CMD_SUCCESS;
}

DEFUN( undo_alarm_log_filter,
       undo_alarm_log_filter_cmd,
       "undo alarm-log filter {[onu-eth-link]}*1",
       "Negate a command or set its defaults\n"
	"delete alarm log filter\n"
	"delete alarm log filter\n"
	"delete filter ONU ETH port linkup/down\n"
       )
{
	delEventLogDataFilterIterm(ALM_FILTER_ONU_ETH_LINK);
	return CMD_SUCCESS;
}

DEFUN( show_alarm_log_filter,
       show_alarm_log_filter_cmd,
       "show alarm-log filter",
	"show alarm info.\n"
	"show alarm log info.\n"
	"show alarm log filter\n"
       )
{
	char *pstr;
	if( getEventLogDataFilter() == ALM_FILTER_ONU_ETH_LINK )
		pstr = "onu-eth-link";
	else
		pstr = "none";

	vty_out( vty, "  Alarm log filter:%s\r\n", pstr );
	return CMD_SUCCESS;
}

/* added by xieshl 20110726, 问题单12920 */
extern LONG showPrivateTrapIdCmd( struct vty *vty, alarmLevel_t level );
QDEFUN( event_show_private_trapid,
       event_show_private_trapid_cmd,
       "show alarm trap-id {[level] <0-4>}*1",
	"Show information\n"
	"Show alarm information\n"
	"Show private trap id information\n"
	"Show specified alarm level of trap-id\n"
	"Input level:1-vital,2-major,3-minor,4-warning,0-notification\n",
       &eventQueId  )
{
	alarmLevel_t level = ALM_LEV_MAX;

	if( argc == 0 )
	{
	}
	else if( argc == 2 )
	{
		if( VOS_StrCmp(argv[0], "level") == 0 )
		{
			level = VOS_AtoL(argv[1] );
		}
	}
	else
	{
		return CMD_ERR_AMBIGUOUS;
	}

	showPrivateTrapIdCmd( vty, level );

	return CMD_SUCCESS;
}

/* added by xieshl 20130318, alarm log保存flash配置命令 */
QDEFUN( alarm_log_save_flash,
	alarm_log_save_flash_cmd,
	"save alarm-log flashfile",
	"save alarm log to flash\n"
	"save alarm log to flash\n"
	"save alarm log to flash\n",
	&eventQueId)
{
	eventLogFlashFileSave();
	return CMD_SUCCESS;
}
QDEFUN( alarm_log_save_flash_enable,
	alarm_log_save_flash_enable_cmd,
	"config alarm-log flashfile [enable|disable]",
	"Config system's setting\n"
	"save alarm log to flash\n"
	"save alarm log to flash enable\n"
	"enable\n"
	"disable\n",
	&eventQueId)
{
	ULONG en = 0;
	if( argv[0][0] == 'e' )
		en = 1;
	nvramEventBackupEnableSet( en );
	return CMD_SUCCESS;
}

QDEFUN( alarm_log_save_flash_period,
	alarm_log_save_flash_period_cmd,
	"config alarm-log flashfile period <10-1000000>",
	"Config system's setting\n"
	"save alarm log to flash\n"
	"save alarm log to flash period\n"
	"input alarm log to flash period, unit minute, default 360\n",
	&eventQueId)
{
	ULONG period = VOS_AtoL( argv[0]);
	if( (period < 10) || (period > 1000000) )
		return CMD_WARNING;
	if( nvramEventBackupPeriodSet(period * 60) == VOS_ERROR )
		return CMD_WARNING;
	return CMD_SUCCESS;
}

/*added by yangtongsan 2014-12-17 for 问题单23494*/
QDEFUN( show_alarm_log_save_flash_period,
	show_alarm_log_save_flash_period_cmd,
	"show alarm-log flashfile period",
	"show system's setting\n"
	"alarm-log\n"
	"save alarm log to flash\n"
	"save alarm log to flash period\n",
	&eventQueId)
{
	ULONG enable = 0;
	
	enable = nvramEventBackupPeriodGet();
	vty_out( vty, "alarm-log flashfile period: %d minute\r\n", enable/60);

	return CMD_SUCCESS;
}

#if( EPON_MODULE_SYS_DIAGNOSE == EPON_MODULE_YES )
extern LONG sys_diag_showrun( struct vty * vty );
#endif
LONG event_showrun( struct vty * vty )
{
	ULONG enable = 0;
	ULONG devIdx, brdIdx, portIdx;
	ULONG nextDevIdx, nextBrdIdx, nextPortIdx;
	ULONG mask;
	ULONG slot, pon, onu;
	ULONG onuType;
	CHAR *pOnuTypeStr;
	CHAR str[64];

	vty_out( vty, "!Alarm config\r\n" );
	
	getEventLogEnable( &enable );
	if( enable != EVENTLOG_ENABLE )
		vty_out( vty, " config alarm-log disable\r\n") ;

	getTrapBacEnable( &enable );
	if( enable != TRAP_BAC_ENABLE )
		vty_out( vty, " config alarm-synchronization disable\r\n") ;		/* 问题单9917 */

	/*if( alarmlogToSyslog_enable == 1 )
		vty_out( vty, " alarmlog-to-syslog enable\r\n") ;*/
	if( (eventLogOutMode & EVENT_LOG_OUT_2_ALL) == EVENT_LOG_OUT_2_ALL )
		vty_out( vty, " config alarm-log out-mode all\r\n" );
	else
	{
		if( eventLogOutMode & EVENT_LOG_OUT_2_SYSLOG )
			vty_out( vty, " config alarm-log out-mode syslog\r\n" );
		if( eventLogOutMode & EVENT_LOG_OUT_2_TELNET )
			vty_out( vty, " config alarm-log out-mode telnet\r\n" );
		if( (eventLogOutMode & EVENT_LOG_OUT_2_CONSOLE) == 0 )
			vty_out( vty, " undo alarm-log out-mode console\r\n" );
	}
	
	if( eventLogFlashFileFlag )	/* modified by xieshl 20130319 */
	{
		enable = nvramEventBackupEnableGet();
		if( enable == 0 )
			vty_out( vty, " config alarm-log flashfile disable\r\n" );
		enable = nvramEventBackupPeriodGet();
		if( nvramEventBackupPeriodDefaultGet() != enable )
			vty_out( vty, " config alarm-log flashfile period %d\r\n", enable/60 );
	}

	if((OptOnuAlarmMask & (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH | 
		EVENT_MASK_PON_TEMPERATURE | EVENT_MASK_PON_BIAS_CURRENT | EVENT_MASK_PON_VOLTAGE)) == 0)
	{
		vty_out( vty, " alarm-mask onu-pon all disable\r\n");
	}
	else
	{
		if((OptOnuAlarmMask & (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH)) == 0)
		{
			vty_out( vty, " alarm-mask onu-pon power disable\r\n");
		}
		if((OptOnuAlarmMask & EVENT_MASK_PON_TEMPERATURE) == 0)
		{
			vty_out( vty, " alarm-mask onu-pon temperature disable\r\n");
		}
		if((OptOnuAlarmMask & EVENT_MASK_PON_BIAS_CURRENT) == 0)
		{
			vty_out( vty, " alarm-mask onu-pon current disable\r\n");
		}
		if((OptOnuAlarmMask & EVENT_MASK_PON_VOLTAGE) == 0)
		{
			vty_out( vty, " alarm-mask onu-pon voltage disable\r\n");
		}
	}
	/* added by xieshl 20070920 增加告警屏蔽保存 */
	/* modified by xieshl 20110415, 解决新增加告警屏蔽项无法保存问题 */
	if( getFirstDeviceEntryIndex(&nextDevIdx) == VOS_OK )
	{
		do
		{
			devIdx = nextDevIdx;
			if( getDeviceAlarmMask(devIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_DEV_ALL;
				if( mask  != 0 )
				{
					if( devIdx == OLT_DEV_ID )
					{
						VOS_Sprintf( str, " alarm-mask olt-device " ) ;
					}
					else
					{
						slot = GET_PONSLOT(devIdx);
						pon = GET_PONPORT(devIdx);
						onu = GET_ONUID(devIdx);

						VOS_Sprintf( str, " alarm-mask onu-device %d/%d/%d ", slot, pon, onu ) ;
					}

					if( mask == EVENT_MASK_DEV_ALL )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ALL) );
					else
					{
						if( mask & EVENT_MASK_DEV_POWER )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_POWER) );
						if( mask & EVENT_MASK_DEV_FAN )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_FAN) );
						if( mask & EVENT_MASK_DEV_CPU )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_CPU) );
						if( mask & EVENT_MASK_DEV_TEMPERATURE )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_TEMPERATURE) );
						if( mask & EVENT_MASK_DEV_REGISTER )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_REGISTER) );
						if( mask & EVENT_MASK_DEV_PRESENT )
							vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PRESENT) );

						/*if( devIdx != OLT_DEV_ID )*/		/* modified by xieshl 20121029, 问题单16077 */
						{
							if( mask & EVENT_MASK_DEV_ETH_LINK )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LINK) );
							if( mask & EVENT_MASK_DEV_ETH_FER )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FER) );
							if( mask & EVENT_MASK_DEV_ETH_FLR )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FLR) );
							if( mask & EVENT_MASK_DEV_ETH_TI )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_TI) );
							if( mask & EVENT_MASK_DEV_ETH_LOOP )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LOOP) );
							if( mask & EVENT_MASK_DEV_PON_BER )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_BER) );
							if( mask & EVENT_MASK_DEV_PON_FER )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_FER) );
							if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABNORMAL) );
							if( mask & EVENT_MASK_DEV_PON_ABS )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABS) );
							if( mask & EVENT_MASK_DEV_PON_LINK )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_LINK) );
							if( mask & EVENT_MASK_DEV_ONU_LASER_ON )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ONU_LASER_ON) );
							if( mask & (EVENT_MASK_DEV_PON_POWER_L | EVENT_MASK_DEV_PON_POWER_H) )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_L) );
							/*if( mask & EVENT_MASK_DEV_PON_POWER_H )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_H) );*/
							if( mask & EVENT_MASK_DEV_PON_LOS )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_LOS) );
							if( mask & EVENT_MASK_DEV_PWU_STATUS )
								vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PWU_STATUS) );
						}
					}
				}
			} 
		}while( getNextDeviceEntryIndex(devIdx, &nextDevIdx) == VOS_OK );
	}

	if( getFirstPonPortEntryIndex(&nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK )
	{
		do{
			devIdx = nextDevIdx;
			brdIdx = nextBrdIdx;
			portIdx = nextPortIdx;
			if( getPonPortAlarmMask(devIdx, brdIdx, portIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_PON_ALL;
				if( mask != 0 )
				{
					if( devIdx == OLT_DEV_ID )
					{
						VOS_Sprintf( str, " alarm-mask olt-pon %d/%d ", brdIdx, portIdx );
					}
					else
					{
						slot = GET_PONSLOT(devIdx);
						pon = GET_PONPORT(devIdx);
						onu = GET_ONUID(devIdx);
						VOS_Sprintf( str, " alarm-mask onu-pon %d/%d/%d ", slot, pon, onu );
					}

					if( mask == EVENT_MASK_PON_ALL )
					{
						vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_ALL) );
					}
					else
					{
						if( mask & EVENT_MASK_PON_BER )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_BER) );
						if( mask & EVENT_MASK_PON_FER )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_FER) );
						if( mask & EVENT_MASK_PON_ABNORMAL )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_ABNORMAL) );
						if( mask & EVENT_MASK_PON_APS )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_APS) );
						if( mask & EVENT_MASK_PON_LINK )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_LINK) );
						if( mask & EVENT_MASK_PON_LOS )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_LOS) );
						if( mask & EVENT_MASK_PON_LASER_ON )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_LASER_ON) );
						if( mask & (EVENT_MASK_PON_POWER_LOW | EVENT_MASK_PON_POWER_HIGH) )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_POWER_LOW) );
						/*if( mask & EVENT_MASK_PON_POWER_HIGH )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_POWER_HIGH) );*/

						if( mask & EVENT_MASK_PON_TEMPERATURE )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_TEMPERATURE) );
						if( mask & EVENT_MASK_PON_BIAS_CURRENT )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_BIAS_CURRENT) );
						if( mask & EVENT_MASK_PON_VOLTAGE )
							vty_out( vty,  "%s%s\r\n", str, alarm_pon_mask_type_to_str(EVENT_MASK_PON_VOLTAGE) );

					}
				}
			}
		} while( getNextPonPortEntryIndex(devIdx, brdIdx, portIdx, &nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK );
	}

	if( getFirstEthPortEntryIndex(&nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK )
	{
		do
		{
			if( nextDevIdx != 1 )	/* 这里只处理OLT侧ETH PORT */
				break;
			
			devIdx = nextDevIdx;
			brdIdx = nextBrdIdx;
			portIdx = nextPortIdx;
			if( getEthPortAlarmMask(devIdx, brdIdx, portIdx, &mask) == VOS_OK )
			{
				mask &= EVENT_MASK_ETH_ALL;
				if( mask != 0 )
				{
					if( devIdx == OLT_DEV_ID )
					{
						VOS_Sprintf( str, " alarm-mask olt-eth %d/%d ", brdIdx, portIdx );
						if( (mask & EVENT_MASK_ETH_ALL) == EVENT_MASK_ETH_ALL )
						{
							vty_out( vty, "%s%s\r\n", str, "all" );
						}
						else
						{
							if( mask & EVENT_MASK_ETH_LINK )
								vty_out( vty, "%s%s\r\n", str, "link" );
							if( mask & EVENT_MASK_ETH_FER )
								vty_out( vty, "%s%s\r\n", str, "fer" );
							if( mask & EVENT_MASK_ETH_FLR )
								vty_out( vty, "%s%s\r\n", str, "flr" );
							if( mask & EVENT_MASK_ETH_TI )
								vty_out( vty, "%s%s\r\n", str, "ti" );
							if( mask & EVENT_MASK_ETH_LOOP )	/* modified by xieshl 20121029, 问题单16085 */
								vty_out( vty, "%s%s\r\n", str, "loop" );
						}
					}
					else
					{
						/*slot = devIdx/10000;
						pon = (devIdx%10000)/1000;
						onu = devIdx%1000;*/
					       /*vty_out( vty, "alarm-mask onu-eth %d/%d/%d%s\r\n", slot, pon, onu, alarm_pon_mask_bits_to_str(mask) );*/
					}
				}
			}
		} while( getNextEthPortEntryIndex(devIdx, brdIdx, portIdx, &nextDevIdx, &nextBrdIdx, &nextPortIdx) == VOS_OK );
	}
	/* end 20070920 */

	/* added by xieshl 20071105, 保存ONU以太网端口告警屏蔽 */
	if( getOnuEthPortAlarmMask(&mask) == VOS_OK )
	{
		VOS_Sprintf( str, " alarm-mask onu-eth " );
		if( (mask & EVENT_MASK_ETH_ALL) == EVENT_MASK_ETH_ALL )
		{
			vty_out( vty, "%s%s\r\n", str, "all" );
		}
		else
		{
			if( mask & EVENT_MASK_ETH_LINK )
				vty_out( vty, "%s%s\r\n", str, "link" );
			else		/* 默认为屏蔽 */
				vty_out( vty, " undo%s%s\r\n", str, "link" );
			if( mask & EVENT_MASK_ETH_FER )
				vty_out( vty, "%s%s\r\n", str, "fer" );
			if( mask & EVENT_MASK_ETH_FLR )
				vty_out( vty, "%s%s\r\n", str, "flr" );
			if( mask & EVENT_MASK_ETH_TI )
				vty_out( vty, "%s%s\r\n", str, "ti" );
			if( mask & EVENT_MASK_ETH_BCFC )	/* modified by xieshl 20130129 */
				vty_out( vty, "%s%s\r\n", str, "bcfc" );
		}
	}

	if( getOnuSwitchAlarmMask(&mask) == VOS_OK )
	{
		if( !(mask & EVENT_MAST_ONU_SWITCH_ALL))
		{
			vty_out( vty, " undo alarm-mask onu-switch\r\n");
		}
	}
    
	for( onuType=V2R1_ONU_GT811; onuType<V2R1_ONU_MAX; onuType++ )
	{
		mask = 0;
		if( getOnuTypeAlarmMask( onuType, &mask ) == VOS_OK )
		{
			mask &= EVENT_MASK_DEV_ALL;
			if( mask )
			{
				pOnuTypeStr = onutypeint_to_str(onuType);
				if( VOS_StrCmp(pOnuTypeStr, DEVICE_TYPE_NAME_UNKNOWN_STR) == 0 )
					continue;

				VOS_Sprintf( str, " alarm-mask onu %s ", pOnuTypeStr ) ;

				if( mask == EVENT_MASK_DEV_ALL )
					vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ALL) );
				else
				{
					if( mask & EVENT_MASK_DEV_POWER )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_POWER) );
					if( mask & EVENT_MASK_DEV_FAN )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_FAN) );
					if( mask & EVENT_MASK_DEV_CPU )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_CPU) );
					if( mask & EVENT_MASK_DEV_TEMPERATURE )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_TEMPERATURE) );
					if( mask & EVENT_MASK_DEV_REGISTER )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_REGISTER) );
					if( mask & EVENT_MASK_DEV_PRESENT )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PRESENT) );
					if( mask & EVENT_MASK_DEV_ETH_LINK )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LINK) );
					if( mask & EVENT_MASK_DEV_ETH_FER )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FER) );
					if( mask & EVENT_MASK_DEV_ETH_FLR )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_FLR) );
					if( mask & EVENT_MASK_DEV_ETH_TI )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_TI) );
					if( mask & EVENT_MASK_DEV_ETH_LOOP )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ETH_LOOP) );
					if( mask & EVENT_MASK_DEV_PON_BER )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_BER) );
					if( mask & EVENT_MASK_DEV_PON_FER )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_FER) );
					if( mask & EVENT_MASK_DEV_PON_ABNORMAL )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABNORMAL) );
					if( mask & EVENT_MASK_DEV_PON_ABS )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_ABS) );
					if( mask & EVENT_MASK_DEV_PON_LINK )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_LINK) );
					if( mask & EVENT_MASK_DEV_ONU_LASER_ON )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_ONU_LASER_ON) );
					if( mask & (EVENT_MASK_DEV_PON_POWER_L | EVENT_MASK_DEV_PON_POWER_H) )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_L) );
					/*if( mask & EVENT_MASK_DEV_PON_POWER_H )
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PON_POWER_H) );*/
					if( mask & EVENT_MASK_DEV_PWU_STATUS)
						vty_out( vty,  "%s%s\r\n", str, alarm_dev_mask_type_to_str(EVENT_MASK_DEV_PWU_STATUS) );
				}
			}
		}
	}


#if 0
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	/*add by shixh@20071226,保存信令网关E1端口的告禁屏蔽*/
	for(e1port=1;e1port<25;e1port++)
	{
		if( GetE1AlarmMask(e1port,&maske1) == VOS_OK )
	     	{
	     		if( (maske1 & E1_ALM_ALL) == E1_ALM_ALL )
	     		{
	     			vty_out( vty, " alarm-mask olt-e1 %d all\r\n", e1port );
	     		}
	     		else
	     		{
	     			if( maske1 & E1_ALM_LOS )
	     			{
	     				if( !(E1_ALM_LOS & E1_ALM_DEF) )
	     					vty_out( vty, " alarm-mask olt-e1 %d LOS\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_LOS & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d LOS\r\n", e1port );
				}

				if( maske1 & E1_ALM_LOF )
				{
	     				if( !(E1_ALM_LOF & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d LOF\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_LOF & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d LOF\r\n", e1port );
				}

				if( maske1 & E1_ALM_AIS )
				{
	     				if( !(E1_ALM_AIS & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d AIS\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_AIS & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d AIS\r\n", e1port );
				}

				if( maske1 & E1_ALM_RAI )
				{
	     				if( !(E1_ALM_RAI & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d RAI\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_RAI & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d RAI\r\n", e1port );
				}

				if( maske1 & E1_ALM_SMF )
				{
	     				if( !(E1_ALM_SMF & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d SMF\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_SMF & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d SMF\r\n", e1port );
				}

				if( maske1 & E1_ALM_LOFSMF )
				{
	     				if( !(E1_ALM_LOFSMF & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d LOFSMF\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_LOFSMF & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d LOFSMF\r\n", e1port );
				}

				if( maske1 & E1_ALM_CRC3 )
				{
	     				if( !(E1_ALM_CRC3 & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d CRC3\r\n", e1port );
	     			}
				else
				{
	     				if( E1_ALM_CRC3 & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d CRC3\r\n", e1port );
				}

				if( maske1 & E1_ALM_CRC6 )
				{
	     				if( !(E1_ALM_CRC6 & E1_ALM_DEF) )
		     				vty_out( vty, " alarm-mask olt-e1 %d CRC6\r\n", e1port );
				}
				else
				{
	     				if( E1_ALM_CRC6 & E1_ALM_DEF )
	     					vty_out( vty, " undo alarm-mask olt-e1 %d CRC6\r\n", e1port );
				}
	     		}
	     	}
	}
	/*end add by shixh@20071226*/
#endif	/* EPON_MODULE_TDM_SERVICE */
#endif

	if( (alarm_mask_pon_cni_bits & 1) == 0 )
	vty_out( vty, " undo alarm-mask pon-cni ber\r\n" );

	/* added by xieshl 20080424, 告警日志过滤 */
	mask = getEventLogDataFilter();
	if( mask & ALM_FILTER_ONU_ETH_LINK )
		vty_out( vty, " alarm-log filter onu-eth-link\r\n" );
	
	vty_out( vty, "!\r\n\r\n" );

#if( EPON_MODULE_SYS_DIAGNOSE == EPON_MODULE_YES )
	sys_diag_showrun( vty );
#endif

	return VOS_OK;
}


int event_cli_cmd_install()
{
	VOS_Sprintf( SHOW_EVENT_LOG_CMD_STR, 
		"show alarm log {[device|name] <value>}*1 {[class] <1-5>}*1 {[trap] <1-%d>}*1 {[start-time] [yestoday|today|<start_time>]}*1 {[end-time] <end_time>}*1", trap_private_max-1 );

	install_element ( CONFIG_NODE, &show_event_log_cmd);
	install_element ( VIEW_NODE, &show_event_log_cmd);
	install_element ( CONFIG_NODE, &show_special_date_event_log_cmd);	
	install_element ( VIEW_NODE, &show_special_date_event_log_cmd);	

	install_element ( CONFIG_NODE, &event_show_curalarm_cmd);
	install_element ( CONFIG_NODE, &event_clear_curalarm_cmd);
	install_element ( VIEW_NODE, &event_show_curalarm_cmd);	

	install_element ( CONFIG_NODE, &show_event_config_cmd);
	install_element ( CONFIG_NODE, &event_log_enable_cmd);
	install_element ( CONFIG_NODE, &event_syn_enable_cmd);
	install_element ( CONFIG_NODE, &erase_event_record_cmd);
	install_element ( CONFIG_NODE, &erase_event_import_record_cmd);/*add byshixh20090420*/
	install_element ( CONFIG_NODE, &show_import_log_cmd);
	install_element ( CONFIG_NODE, &alarmLog_to_sysLog_enable_cmd);/*add by shixh20090927*/
	install_element ( CONFIG_NODE, &show_alarmLog_to_sysLog_enable_cmd);

    /*added by luh 2014-07-18*/
    install_element ( VIEW_NODE, &show_import_log_cmd);
 
	install_element ( DEBUG_HIDDEN_NODE, &todo_event_debug_cmd);
	install_element ( DEBUG_HIDDEN_NODE, &undo_event_debug_cmd);
       if (SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
       {
	    install_element ( DEBUG_HIDDEN_NODE, &show_special_date_event_log_cmd);	
	    install_element ( DEBUG_HIDDEN_NODE, &show_event_log_cmd);
    	install_element ( DEBUG_HIDDEN_NODE, &show_import_log_cmd);/*add by luh 2013-08-09*/        
       }
	install_element ( CONFIG_NODE, &alarm_mask_device_olt_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_device_olt_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_device_olt_cmd);
	install_element ( CONFIG_NODE, &alarm_mask_device_onu_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_device_onu_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_device_onu_cmd);

	install_element ( CONFIG_NODE, &alarm_mask_olt_pon_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_olt_pon_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_olt_pon_cmd);
	install_element ( CONFIG_NODE, &alarm_mask_onu_pon_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_onu_pon_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_onu_pon_cmd);

	install_element ( CONFIG_NODE, &alarm_mask_olt_eth_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_olt_eth_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_olt_eth_cmd);

	install_element ( CONFIG_NODE, &alarm_mask_onu_eth_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_onu_eth_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_onu_eth_cmd);
	install_element ( CONFIG_NODE, &alarm_mask_onu_switch_cmd);         /*add by luh 2013-08-13*/
	install_element ( CONFIG_NODE, &undo_alarm_mask_onu_switch_cmd);    /*add by luh 2013-08-13*/
	install_element ( CONFIG_NODE, &show_alarm_mask_onu_switch_cmd);    /*add by luh 2013-08-13*/
	install_element ( VIEW_NODE, &show_alarm_mask_onu_switch_cmd);    /*add by luh 2013-08-13*/
	install_element ( CONFIG_NODE, &alarm_mask_show_all_cmd);/*add by shixh20091214*/
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	install_element ( CONFIG_NODE, &alarm_mask_olt_e1_cmd);	
	install_element ( CONFIG_NODE, &undo_alarm_mask_olt_e1_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_olt_e1_cmd);

	install_element ( CONFIG_NODE, &alarm_mask_onu_tdm_service_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_onu_tdm_service_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_onu_tdm_service_cmd);
#endif

	install_element ( CONFIG_NODE, &alarm_mask_onu_cmd);/*add by shixh20091027*/
	install_element ( CONFIG_NODE, &alarm_mask_onu_show_cmd);
	install_element ( CONFIG_NODE, &delete_alarm_mask_onu_cmd);
	/*install_element ( CONFIG_NODE, &alarm_mask_onu_m_cmd);
	install_element ( CONFIG_NODE, &alarm_mask_onu_m_show_cmd);
	install_element ( CONFIG_NODE, &delete_alarm_mask_onu_m_cmd);*/

	/* added by xieshl 20080424, 告警日志过滤 */
	install_element ( CONFIG_NODE, &alarm_log_filter_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_log_filter_cmd);
	install_element ( CONFIG_NODE, &show_alarm_log_filter_cmd);
	install_element ( VIEW_NODE, &show_alarm_log_filter_cmd);

	install_element ( CONFIG_NODE, &alarm_mask_pon_cni_cmd);
	install_element ( CONFIG_NODE, &undo_alarm_mask_pon_cni_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_pon_cni_cmd);
	install_element ( CONFIG_NODE, &alarm_mask_onu_opt_cmd);
	install_element ( CONFIG_NODE, &show_alarm_mask_onu_opt_cmd);

	install_element( CONFIG_NODE, &event_show_private_trapid_cmd );

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )/* modified by xieshl 20130318, 无nvram时挂接 */
	{
		install_element( CONFIG_NODE, &config_alarmlog_out_mode_cmd );
		install_element( CONFIG_NODE, &undo_alarmlog_out_mode_cmd );
		install_element( VIEW_NODE, &config_alarmlog_out_mode_cmd );
		install_element( VIEW_NODE, &undo_alarmlog_out_mode_cmd );
		
		if( eventLogFlashFileFlag )
		{
			install_element( CONFIG_NODE, &alarm_log_save_flash_cmd );
			install_element( CONFIG_NODE, &alarm_log_save_flash_enable_cmd );
			install_element( CONFIG_NODE, &alarm_log_save_flash_period_cmd );
			install_element( CONFIG_NODE, &show_alarm_log_save_flash_period_cmd );
		}
	}
	return VOS_OK;
}

#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
extern STATUS environment_showrun( struct vty * vty );
#endif
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
extern STATUS powerDownAlarm_showrun( struct vty * vty );
#endif
/*extern LONG ethloop_showrun( struct vty * vty );*/
int event_show_run( struct vty * vty )
{
	event_showrun(vty);
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
	/* begin: added by jianght 20090909 */
	powerDownAlarm_showrun(vty);
	/* end: added by jianght 20090909 */
#endif

#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
	environment_showrun(vty);
#endif
	/*ethloop_showrun( vty );*/

    ctcOnu_alarm_showrun(vty);/*CTC-ONU  alarm show-run modi by luh 2011-11-1*/

	return VOS_OK;
}

/* added by xieshl 20110829, 独立定义showrun模块，问题单12918 */
LONG event_module_init()
{
    struct cl_cmd_module * event_module = NULL;

    event_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_EVENT);
    if ( !event_module )
    {
        ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_MemZero( ( char * ) event_module, sizeof( struct cl_cmd_module ) );

    event_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_EVENT );
    if ( !event_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( event_module );
        return VOS_ERROR;
    }
    VOS_StrCpy( event_module->module_name, "alarm" );

    event_module->init_func = event_cli_cmd_install;
    event_module->showrun_func = event_show_run;
    event_module->next = NULL;
    event_module->prev = NULL;
    cl_install_module( event_module );

    return VOS_OK;
}


#ifdef	__cplusplus
}
#endif/* __cplusplus */
