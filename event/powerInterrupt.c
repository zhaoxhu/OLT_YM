
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "ppc405EP.h"
#include  "bsp_cpld.h"


#define PRODUCT_MAX_TOTAL_POWER_COUNT	4		/* 最多支持电源板数 */
ULONG  sys_max_power_count = 1;		/* 产品实际支持电源板槽位数 */

#define INT_LVL_EXT_IRQ_0	25
#define INT_LVL_EXT_IRQ_1	26
#define INT_LVL_EXT_IRQ_2	27
#define INT_LVL_EXT_IRQ_3    28
#define INT_LVL_EXT_IRQ_4    29
#define INT_LVL_EXT_IRQ_5    30
#define INT_LVL_EXT_IRQ_6    31

#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
UCHAR pwuIntIrq[PRODUCT_MAX_TOTAL_POWER_COUNT + 1] = { 0 };
#else

/* Universal Interrupt Controller  */
#define UIC0_SR			(UIC_DCR_BASE + 0)
#define UIC0_ER			(UIC_DCR_BASE + 2)
#define UIC0_CR			(UIC_DCR_BASE + 3)
#define UIC0_PR			(UIC_DCR_BASE + 4)
#define UIC0_TR			(UIC_DCR_BASE + 5)
#define UIC0_MSR		(UIC_DCR_BASE + 6)
#define UIC0_VR			(UIC_DCR_BASE + 7)
#define UIC0_VCR			(UIC_DCR_BASE + 8)

#define UINT32_BIT_LEN			32
UINT32 UIC0_SR_Offset[PRODUCT_MAX_TOTAL_POWER_COUNT + 1] = { 0 };
#endif


#define PWU_INT_ENABLE		1
#define PWU_INT_DISABLE		0

#define PWU_ISR_CONNECT		1
#define PWU_ISR_DISCONNECT	0

UCHAR pwuIntEnable[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 };

#define PWU_ALM_CLEAR	0
#define PWU_ALM_ON  1
UCHAR PWU_ALARM[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 };
UCHAR PWU_ALARM2[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 };

UCHAR pwu_status[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 };
UCHAR pwu_status_mask[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 1,1,1,1 };
UCHAR powerId2slotno[ PRODUCT_MAX_TOTAL_POWER_COUNT ];
UCHAR slotno2powerId[ PRODUCT_MAX_TOTAL_SLOTNUM + 1 ];

UCHAR powerId2rspwuId[ PRODUCT_MAX_TOTAL_POWER_COUNT ];
UCHAR rspwuIdcount = 0;


/* 6100和6700电源GPIO起始号 */
UCHAR PowerBrdGpioStart = 0;
LONG debugPowerBrdInterrupt = 0;

#define POWERBRDINT_INFO	0x01/* 函数执行的错误信息 */
#define POWERBRD_PRINT(x)      if (debugPowerBrdInterrupt & POWERBRDINT_INFO) sys_console_printf x
#define POWERBRD_LOGMSG(x) if (debugPowerBrdInterrupt & POWERBRDINT_INFO) logMsg x


/****************************************************************/
void ISR_powerInterrupte(UCHAR powerId);
void ISR_powerInterrupte0(void);
void ISR_powerInterrupte1(void);
void ISR_powerInterrupte2(void);
BOOL PWU_GpioLevelRead(UCHAR powerId);
BOOL checkPWU48VSupply(UCHAR powerId);
BOOL isLastPowerBrdInterrupt(UCHAR powerId);
BOOL isPWU48V(UCHAR powerId);
BOOL readPowerBrdGpio(UCHAR powerBrdIdx);
void PWU_PowerOff( ULONG powerId );
void PWU_PowerOffClear(ULONG slotno);
void PWU_IntDisable(UCHAR powerId);
void PWU_IntEnable(UCHAR powerId);
STATUS powerInterruptConnect(UCHAR powerId);

LONG enable_pwu_alarm( int enable );

/*************************** extern ************************************/
extern LONG device_chassis_is_slot_inserted( ULONG slotno );
extern int read_gpio(int GPIOx);
extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern int intLock (void);
extern void intUnlock( int lockKey );
void showStatus( struct vty * vty );
void showPwuStatus( struct vty * vty );


/* ********************** def by wangysh 09.10.27 ********************************/
#define PWD_TRAP_QUEUE_NUM 100
#define PWD_TASK_PRIO 1
#define PWD_WD_TIMER 1000
ULONG pwdTrapQueId = 0 ;
ULONG pwdTrapTaskId = 0;
ULONG pwdWdTimerId = 0;

ULONG interrupt_counter[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 } ;
ULONG flag_when_interrupt[ PRODUCT_MAX_TOTAL_POWER_COUNT ] = { 0 };

ULONG interpt_limit = 5;
ULONG cl_interrupt_status = 1;
ULONG service_interrupt_status = 0;
ULONG pwuAlmSemB = 0;

/* 消息类型*/
#define MSG_PWD_OFF 1                   /*掉电*/
#define MSG_PWD_TIMER 2                  /*定时期消息*/

/*状态机输入*/
#define PWD_ON         1          /* 带电插入*/
#define PWD_PULLOUT   2         /* 不带电拔出*/
#define PWD_INSERT 3

/* PWU状态机 */
#define PWU_STAT_NULL			0
#define PWU_STAT_INSERT		1
#define PWU_STAT_RUN	                2

#define PWU_STAT_RUN_STR	 "RUNNING"
#define PWU_STAT_NULL_STR	 "EMPTY"
#define PWU_STAT_INSERT_STR "INSERT"

/*#define POWERID_ASSERT( powerid ) VOS_ASSERT( powerid <= 2 );*/
#define POWERID_ASSERT( powerid ) VOS_ASSERT( powerid < sys_max_power_count );

/* ********************** end by wangysh 09.10.27 ********************************/

extern STATUS intConnect
(
 VOIDFUNCPTR * vector,     /* interrupt vector to attach to */
 VOIDFUNCPTR   routine,    /* routine to be called */
 int           parameter   /* parameter to be passed to routine */
 );

#define SWPOWERID(i,j) \
do{\
	if( i == 2 )\
	    j = 0;\
	else if( i == 3 )\
	    j = 1;\
	else\
		j = i;\
}while(0);
#define INSERT_48_BIT(i) (1<<(3+i))
#define INSERT_220_BIT(i) (1<<(5+i))
#define ALARM48_BIT(i) (1<<(2*i))
#define CLR_ALM48_BIT(i) (~(1<<(2*i)))
#define ALARM220_BIT(i)(1<<(2*i))
#define CLR_ALM220_BIT(i)(~(1<<(2*i)))
#define ALARM_CLEAR_48_BIT(i) (1<<(4+2*i))
#define CLEAR_48_BIT(i) (~(1<<(4+2*i)))
#define ALARM_CLEAR_220_BIT(i)(1<<(4+2*i))
#define CLEAR_220_BIT(i)(~(1<<(4+2*i)))
#define OPENER_48_BIT(i) (1<<(2*i))
#define OPENER_220_BIT(i)(1<<(1+2*i))
#define PARTNER(one,theOther) \
do{\
	switch(one)\
	{\
		case 18:\
			theOther=20;\
		break;\
		case 19:\
			theOther=21;\
		break;\
		case 20:\
			theOther=18;\
		break;\
		case 21:\
			theOther=19;\
		break;\
		default:\
		break;\
	}\
}while(0);


/*BEGIN: add for Pwu Status Alarm by @muqw 2017-5-6*/

typedef struct pwu_module_info{
	ULONG pwu_id;            /*电源ID*/
	ULONG pwu_moduletype;    /*电源板卡类型*/
	ULONG pwu_mode;		     /*电源供电模式*/
	ULONG pwu_alarmed;       /*以bitmap方式记录告警类型，同类型只报一次*/
	ULONG pwu_temper;            /*电源温度*/
	LONG  pwu_voltage;           /*电压*/
	LONG  pwu_current;           /*电流*/
	ULONG reserved;          /*reserved*/
}PWU_MODULE_INFO;

PWU_MODULE_INFO  pwu_module_info_t[PRODUCT_MAX_TOTAL_POWER_COUNT] = {0};


#define FPGA_PWU220_FAULT_1_ALM7 0x0080    /*18,20槽位AC电源未上电告警*/
#define FPGA_PWU220_FAULT_2_ALM9 0x0200    /*19,21槽位AC电源未上电告警*/

#define FPGA_PWU48_FAULT_1_ALM13 0x0800    /*18槽位DC电源未上电告警*/
#define FPGA_PWU48_FAULT_2_ALM14 0x1000    /*19槽位DC电源未上电告警*/

#define FPGA_PWU48_ALARM_1_ALM4  0x0008    /*18槽位DC电源电压超出范围告警*/
#define FPGA_PWU48_ALARM_2_ALM5  0x0010    /*19槽位DC电源电压超出范围告警*/

USHORT  PWU220_FAULT_PWUID_2_ERRNO[PRODUCT_MAX_TOTAL_POWER_COUNT] = {FPGA_PWU220_FAULT_1_ALM7, FPGA_PWU220_FAULT_2_ALM9, FPGA_PWU220_FAULT_1_ALM7, FPGA_PWU220_FAULT_2_ALM9};
USHORT  PWU48_FAULT_PWUID_2_ERRNO[PRODUCT_MAX_TOTAL_POWER_COUNT]  = {FPGA_PWU48_FAULT_1_ALM13, FPGA_PWU48_FAULT_2_ALM14, FPGA_PWU48_FAULT_1_ALM13, FPGA_PWU48_FAULT_2_ALM14};
USHORT  PWU48_ALARM_PWUID_2_ERRNO[PRODUCT_MAX_TOTAL_POWER_COUNT]  = {FPGA_PWU48_ALARM_1_ALM4,  FPGA_PWU48_ALARM_2_ALM5,  FPGA_PWU48_ALARM_1_ALM4,  FPGA_PWU48_ALARM_2_ALM5 };

#define RS485_PWU220_VOLT_REG   0x20
#define RS485_PWU220_CUR_REG    0x0A
#define RS485_PWU220_STAT_REG   0x0C
#define RS485_PWU220_TEMP_REG   0x0B
#define RS485_PWU220_SLOT_REG   0x1F       /*标记转换板电源槽位   18/20--0x16   19/21--0x15*/

#define RS485_PWU220_FAN_ALARM   0x4000    /*风扇异常*/       /*FAN_STAT*/
#define RS485_PWU220_LOW_ALARM   0x0020    /*输入欠压*/       /*AC_LOW_LINE_STAT*/
#define RS485_PWU220_LOW_FAULT   0x0002    /*输入电压过低*/   /*ACF_STAT*/
#define RS485_PWU220_HIGH_ALARM  0x0400    /*输入过压*/

#define READ_PWU_STATUS_ALARM(alm1, alm2, alarm)\
do{\
	ReadCPLDReg((UCHAR*)CPLD1_PWU_STATUS_ALM1_REG_8000, &alm1);\
	ReadCPLDReg((UCHAR*)CPLD1_PWU_STATUS_ALM2_REG_8000, &alm2);\
	alarm = (alm2<<8)|alm1;\
}while(0);

#define ACF_STAT_ABNORMAL   0x00000001     /*AC输入电压异常（ACF_STAT） */
#define RFA_STAT_ABNORMAL   0x00000002     /*电源工作状态异常（RFA_STAT）*/
#define FAN_STAT_ABNORMAL   0x00000004     /*风扇工作异常（FAN_STAT）*/
#define CL_STAT_ABNORMAL    0x00000008     /*电源处于过流工作状态（CL_STAT）*/
#define VOP_R_ABNORMAL      0x00000010     /*电压输出异常(VOP_R)*/ 
#define HVSD_STAT_ABNORMAL  0x00000020     /*电压输入过压异常(HVSD_STAT)*/
#define I_R_ABNORMAL        0x00000040     /*电流均流异常(I_R) */
#define AC_LOWLINE_ABNORMAL 0x00000080     /*电源处于low line工作状态*/
#define REG_STAT_ABNORMAL   0x00000100     /*电源盘的寄存器无法访问*/
#define DC_VOLT_OUT_RANGE   0x00000200     /*DC电源电压超出范围*/
#define PWU_POWEROFF        0x00000400     /*电源未上电*/
#define AC_TEMP_ABNORMAL    0x00000800     /*AC电源温度异常*/
#define PWU_MAX_ABNORMAL    0x00001000     /*电源告警最大告警ID*/

/*根据告警严重级别由高到低排列*/
ULONG AlarmidOrderdByImportance[] = {0x00000001,
									 0x00000002,
									 0x00000004,
									 0x00000008,
									 0x00000010,
									 0x00000020,
									 0x00000100,
									 0x00000200,
									 0x00000800,
									 0x00000040,
									 0x00000400,
									 0x00000080,
									 0x00001000
		 							};

#define PWU_TYPE_AC220  1
#define PWU_TYPE_DC48   2


#define DOUBLE_AC_VOLT_LOW_THRESHOLD  -50.3
#define DOUBLE_AC_VOLT_HIGH_THRESHOLD  53.5

#define DC_AC_VOLT_LOW_THRESHOLD     -45.0
#define DC_AC_VOLT_HIGH_THRESHOLD     54.0

FLOAT PWU_VOLT_LOW_THRESHOLD = 0;
FLOAT PWU_VOLT_HIGH_THRESHOLD = 0;

ULONG PWU_CHECK_TIMER_DEFAULT = 60;
ULONG PWU_CHECK_TIMER = 60; /*轮询一次时间间隔10*60S*/
ULONG PWU_DEBUG=0;

FLOAT DOUBLE_AC_CUR_LOW_THRESHOLD = 0.5;

#define PWU_STATUS_DEBUG(x)      if(PWU_DEBUG) sys_console_printf x


extern LONG temperatureAlarmValueHigh;
extern LONG temperatureAlarmValueLow;


/*END: add for Pwu Status Alarm by @muqw 2017-5-6*/


extern int logMsg( char * fmt, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 );
extern ULONG (*devsm_module_pwu_runningstate_get_callback)(ULONG slotno);

BOOL brdOn(UCHAR powerBrdIdx)
{
	UCHAR val;
	UCHAR slotno,partner;
	UCHAR powerId;

	slotno = powerId2slotno[powerBrdIdx];
	SWPOWERID(powerBrdIdx,powerId);
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 )
	{	
		ReadCPLDReg((UCHAR*)0xd,&val);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48)
		{
			if( val & OPENER_48_BIT(powerId) )
			{
				return TRUE;
			}
			else if( val & OPENER_220_BIT(powerId) )
			{
				return TRUE;
			}
			else
				return FALSE;
		}
		else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220)
		{
			if( val & OPENER_220_BIT(powerId) )
			{
				PARTNER(slotno,partner)
				if( device_chassis_is_slot_inserted(partner) )
					return TRUE;
				else
					return FALSE;
			}
			else
				return FALSE;
		}
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000) /*****add by   mengxsh  20140923 *****/ 
	{	
		ReadCPLDReg((UCHAR*)(FPGA_BASEADDR_8000+0xd),&val);
		if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU48) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48))
		{
			if( val & OPENER_48_BIT(powerId) )
			{
				return TRUE;
			}
			else if( val & OPENER_220_BIT(powerId) )
			{
				return TRUE;
			}
			else
				return FALSE;
		}
		else if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU220) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220))
		{
			if( val & OPENER_220_BIT(powerId) )
			{
				PARTNER(slotno,partner)
				if( device_chassis_is_slot_inserted(partner) )
					return TRUE;
				else
					return FALSE;
			}
			else
				return FALSE;
		}
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100)
	{	
		ReadCPLDReg((UCHAR*)(0x02),&val);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_PWU48) 
		{
			if(!( val & (1 << (4+powerId)) ))
			{
				return TRUE;
			}
			else
				return FALSE;
		}
		else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8100_PWU220) 
		{
			if(!( val & (1 << (4+powerId)) ))
			{
				return TRUE;
			}
			else
				return FALSE;
		}
	}
	else
	{		
		return readPowerBrdGpio(powerBrdIdx);
	}
	return FALSE;
}

/* added by xieshl 20120313, 读电源板实际状态，需求13344 */
ULONG pwu_brd_runningstate_get(ULONG slotno)
{
	UCHAR powerBrdIdx = 0;
	ULONG state = MODULE_EMPTY;

	if (SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
	{
		if( CDSMS_CHASSIS_SLOT_INSERTED(slotno) )
		{
			powerBrdIdx = slotno2powerId[slotno];
			if( brdOn(powerBrdIdx) )
				state = SYS_MODULE_RUNNINGSTATE(slotno);
			else
				state = MODULE_INSERTED;
		}
	}
	return state;
}

void pwdTrapProc( ULONG powerId )
{	
	ULONG loop;
	POWERID_ASSERT( powerId )
	
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
		for( loop = 0 ; loop < 1000000 ; loop ++ );
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
		for( loop = 0 ; loop < 10000 ; loop ++ );
		
	if ( !brdOn( powerId ) )
	{
		if ( isLastPowerBrdInterrupt( powerId ) )
		{
			devPowerOff_EventReport( OLT_DEV_ID );
		}
		else
		{
			POWERBRD_PRINT( ( "\r\n pwd %d power off \r\n",powerId ));
			PWU_PowerOff( powerId );
			pwu_status[ powerId ] = PWU_STAT_INSERT;
			pwu_status_mask[ powerId ] = 1 << PWU_STAT_INSERT;
		}
	}
	else
	{
		if( cl_interrupt_status )
			PWU_IntEnable( powerId );
	}
	return;
}

ULONG alarm6900(ULONG powerId)
{
	UCHAR val;
	UCHAR slotno;
	UCHAR j;

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)
	{
		slotno = powerId2slotno[powerId];
		SWPOWERID(powerId,j)
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48)
		{
			ReadCPLDReg((UCHAR*)0xb,&val);
			if( val && ALARM48_BIT(j))
			{
				write_cpld((UCHAR*)0xb,val&CLR_ALM48_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
		else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220)
		{
			ReadCPLDReg((UCHAR*)0xc,&val);
			if( val && ALARM220_BIT(j))
			{
				write_cpld((UCHAR*)0xc,val&CLR_ALM220_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}		
	}
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (0 == powerId)
		{
			val = read_gpio( 29 );
			return ( val == 0 ) ? FALSE : TRUE ;
		}
		else if (1 == powerId)
		{
			val = read_gpio( 18 );
			return ( val == 0 ) ? FALSE : TRUE ;
		}
	}
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
	{
		return FALSE;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
	{
#if 1  /*****add by   mengxsh  20141013 *****/
		slotno = powerId2slotno[powerId];
		SWPOWERID(powerId,j)
		if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU48) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48))
		{
			ReadCPLDReg((UCHAR*)(FPGA_BASEADDR_8000+0xb),&val);
			if( val && ALARM48_BIT(j))
			{
				write_cpld((UCHAR*)(FPGA_BASEADDR_8000+0xb),val&CLR_ALM48_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
		else if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU220) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220))
		{
			ReadCPLDReg((UCHAR*)(FPGA_BASEADDR_8000+0xc),&val);
			if( val && ALARM220_BIT(j))
			{
				write_cpld((UCHAR*)(FPGA_BASEADDR_8000+0xc),val&CLR_ALM220_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
#endif
	}

	return FALSE;
}

ULONG alarm6900Clr(ULONG powerId)
{
	UCHAR val;
	UCHAR slotno;
	UCHAR j;

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)
	{
		slotno = powerId2slotno[powerId];
		SWPOWERID(powerId,j)
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48)
		{
			ReadCPLDReg((UCHAR*)0xb,&val);
			if( val && ALARM_CLEAR_48_BIT(j))
			{
				write_cpld((UCHAR*)0xb,val&CLEAR_48_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
		else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220)
		{
			ReadCPLDReg((UCHAR*)0xc,&val);
			if( val && ALARM_CLEAR_220_BIT(j))
			{
				write_cpld((UCHAR*)0xc,val&CLEAR_220_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}		
	}
	else if(( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) || ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S))
	{
		if (0 == powerId)
		{
			val = read_gpio( 29 );
			return ( val == 0 ) ? TRUE : FALSE ;
		}
		else if (1 == powerId)
		{
			val = read_gpio( 18 );
			return ( val == 0 ) ? TRUE : FALSE ;
		}
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
	{
#if 1  /*****add by   mengxsh  20141013 *****/
		slotno = powerId2slotno[powerId];
		SWPOWERID(powerId,j)
		if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU48) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU48))
		{
			ReadCPLDReg((UCHAR*)(FPGA_BASEADDR_8000+0xb),&val);
			if( val && ALARM_CLEAR_48_BIT(j))
			{
				write_cpld((UCHAR*)(FPGA_BASEADDR_8000+0xb),val&CLEAR_48_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
		else if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_PWU220)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_PWU220))
		{
			ReadCPLDReg((UCHAR*)(FPGA_BASEADDR_8000+0xc),&val);
			if( val && ALARM_CLEAR_220_BIT(j))
			{
				write_cpld((UCHAR*)(FPGA_BASEADDR_8000+0xc),val&CLEAR_220_BIT(j));
				return TRUE;
			}
			else
				return FALSE;
		}
#endif
	}
	
	return FALSE;
}

void PWU_PowerError( ULONG powerId )
{
	if(PWU_ALARM2[powerId]==0)
	{
		/*sys_console_printf("\r\n slot %d alarm\r\n",powerId2slotno[powerId]);*/
		PWU_ALARM2[powerId]=1;
	}
}

void PWU_PowerErrorClear( ULONG powerId )
{
	if(PWU_ALARM2[powerId]==1)
	{
		/*sys_console_printf("\r\n slot %d alarm clear\r\n",powerId2slotno[powerId]);*/
		PWU_ALARM2[powerId]=0;
	}
}


/*BEGIN: add for Pwu Status Alarm by @muqw 2017-5-6*/

STATUS PWU_READ_STATUS_VALID(ULONG powerid , USHORT *status)
{
    char buf[32];
	int array[4];
	int physlot = 0, rspwslot[4];
	int rscount = 0;
	int len = 0;
	int i = 0;
	unsigned short var;

	rscount = rs485Get(array);

	for(i = 0; i < rscount; i++)
	{
		physlot = array[i];

		/*获取电源转换板寄存器里的ID数*/
		rspwslot[i] = 0;
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(physlot, RS485_PWU220_SLOT_REG, buf);
		if(len==1)
		{
			rspwslot[i] = ((int)*((unsigned char *)buf));
		}

		
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(physlot, RS485_PWU220_STAT_REG, buf);
		if(len==2)
		{
			var = *((unsigned short *)buf);
		}
		else
		{
			continue;
		}

		if(0x16 == rspwslot[i]) /*slot 18/20对应powerid 0/2*/
		{
			status[0] = var;
			status[2] = var;
		}
		else if(0x15 == rspwslot[i])/*slot 19/21对应powerid 1/3*/
		{
			status[1] = var;
			status[3] = var;
		}

	}

	return VOS_OK;
	
}


STATUS PWU_READ_TEMP_VALID(ULONG powerid , UCHAR *temp)
{
    char buf[32];
	int array[4];
	int physlot = 0, rspwslot[4];
	int rscount = 0;
	int len = 0;
	int i = 0;
	char var;

	rscount = rs485Get(array);

	for(i = 0; i < rscount; i++)
	{	
		physlot = array[i];

		/*获取电源转换板寄存器里的ID数*/
		rspwslot[i] = 0;
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(physlot, RS485_PWU220_SLOT_REG, buf);
		if(len==1)
		{
			rspwslot[i] = ((int)*((unsigned char *)buf));
		}
	
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(physlot, RS485_PWU220_TEMP_REG, buf);
		if(len==1)
		{
			var = *((unsigned char *)buf);
		}
		else
		{
			continue;
		}


		if(0x16 == rspwslot[i]) /*slot 18/20对应powerid 0/2*/
		{
			temp[0] = var;
			temp[2] = var;
		}
		else if(0x15 == rspwslot[i])/*slot 19/21对应powerid 1/3*/
		{
			temp[1] = var;
			temp[3] = var;
		}

	}

	return VOS_OK;
	
}


STATUS PWU_READ_VOLT_CUR_VALID(ULONG powerid , LONG *voltage, LONG *current)
{
    char buf[32];
	int array[4];
	int len = 0;
	int i = 0, j = 0;
	float val;
	unsigned short vol,cap;
	unsigned long tmp1[5],tmp2[5];
	int physlot = 0, rspwslot[4];
	int rscount = 0, err_count = 0 ;
	int vol_ave = 0, cur_ave = 0;  

	rscount = rs485Get(array);
	
	PWU_STATUS_DEBUG(("\r\n %s %d rscount:%d \r\n",__FUNCTION__, __LINE__, rscount));

	for(i = 0; i < rscount; i++)
	{
		physlot = array[i];

		/*获取电源转换板寄存器里的ID数*/
		rspwslot[i] = 0;
		VOS_MemZero(buf,32);
		len = ep1000_read_reg(physlot, RS485_PWU220_SLOT_REG, buf);
		if(len==1)
		{
			rspwslot[i] = (int)*((unsigned char *)buf);
		}

		err_count = 0;
	
		for(j = 0; j < 5; j++)
		{
			tmp1[j] = 0;
			VOS_MemZero(buf,32);
			len = ep1000_read_reg(physlot, RS485_PWU220_VOLT_REG, buf);
			if(len==2)
			{
				vol = *((unsigned short *)buf);
				val = vol/4;    /*真实值需除以400，为保留小数点后两位精度，将真实值扩大100倍*/
				tmp1[j] = (unsigned long)val;
			}
			else
			{
				err_count++;
			}
		}

		if(err_count == 5)
		{
			return VOS_ERROR;
		}
		
		vol_ave = (LONG)ExcludeMaxMin_Then_GetAverage(tmp1, 5);

		if(0x16 == rspwslot[i]) /*slot 18/20对应powerid 0/2*/
		{
			voltage[0] = vol_ave;
			voltage[2] = vol_ave;
		}
		else if(0x15 == rspwslot[i])/*slot 19/21对应powerid 1/3*/
		{
			voltage[1] = vol_ave;
			voltage[3] = vol_ave;
		}
		

		err_count = 0;
		
		for(j = 0; j < 5; j++)
		{
			tmp2[j] = 0;
			VOS_MemZero(buf,32);
			len = ep1000_read_reg(physlot, RS485_PWU220_CUR_REG, buf);
			if(len==2)
			{
				cap = *((unsigned short *)buf);
				val = cap*10;     /*真实值需除以10，为保留小数点后两位精度，将真实值扩大100倍*/
				tmp2[j] = (unsigned long)val;
			}
			else
			{
				err_count++;
			}
		}

		if(err_count == 5)
		{
			return VOS_ERROR;
		}

		cur_ave = (LONG)ExcludeMaxMin_Then_GetAverage(tmp2, 5);


		if(0x16 == rspwslot[i]) /*slot 18/20对应powerid 0/2*/
		{
			current[0] = cur_ave;
			current[2] = cur_ave;
		}
		else if(0x15 == rspwslot[i])/*slot 19/21对应powerid 1/3*/
		{
			current[1] = cur_ave;
			current[3] = cur_ave;
		}
	}

	return VOS_OK;
	
}

ULONG ulpwualarmid[PRODUCT_MAX_TOTAL_POWER_COUNT] = {0, 0, 0, 0};
ULONG ulpwuclearalarmid[PRODUCT_MAX_TOTAL_POWER_COUNT] = {0, 0, 0, 0};

LONG PWU_Status_Alarm_Report(ULONG slotno, ULONG new_alarmid, ULONG old_alarmid)
{
	int i,j;
	ULONG ulalarmid_new = 0;
	ULONG ulalarmid_old = 0;
	ULONG ulalarmid_report = 0;

	ULONG powerid = slotno2powerId[slotno];

	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		return VOS_OK;
	}

	ulalarmid_new = new_alarmid;
	ulalarmid_old = old_alarmid;

	if(ulalarmid_new == ulalarmid_old)
	{
		return VOS_OK;
	}
	
	for(i = 0; i < sizeof(ULONG)*8; i++)
	{
		if(AlarmidOrderdByImportance[i] < PWU_MAX_ABNORMAL)
		{
			PWU_STATUS_DEBUG((" \r\n i:%d new:0x%x old:0x%x Order:0x%x ",i, new_alarmid, old_alarmid, AlarmidOrderdByImportance[i]));

			PWU_STATUS_DEBUG((" new&order:%d, old&order:%d  \r\n",(ulalarmid_new & AlarmidOrderdByImportance[i]),(ulalarmid_old & AlarmidOrderdByImportance[i])));
			
			if((ulalarmid_new & AlarmidOrderdByImportance[i]) && (!(ulalarmid_old & AlarmidOrderdByImportance[i])))
			{
				/*上报告警*/
				PWUStatusAbnormal_EventReport(OLT_DEV_ID, slotno, AlarmidOrderdByImportance[i]);
				ulpwualarmid[powerid] = AlarmidOrderdByImportance[i];
				PWU_STATUS_DEBUG((" \r\n slot:%d PWUStatusAbnormal_EventReport i:%d alarmid:0x%x \r\n", slotno, i, AlarmidOrderdByImportance[i]));
				break;
			}
			else if((!(ulalarmid_new & AlarmidOrderdByImportance[i])) && (ulalarmid_old & AlarmidOrderdByImportance[i]))
			{
				/*上报告警清除*/
				PWUStatusAbnormalClear_EventReport(OLT_DEV_ID, slotno, AlarmidOrderdByImportance[i]);
				ulpwuclearalarmid[powerid] = AlarmidOrderdByImportance[i];
				PWU_STATUS_DEBUG((" \r\n slot:%d PWUStatusAbnormalClear_EventReport i%d  alarmid:0x%x \r\n", slotno, i, AlarmidOrderdByImportance[i]));
				
				if(0 == ulalarmid_new)
				{
					break;
				}
			}
		}
	}

	return VOS_OK;
}

/*该函数每10s钟调用一次*/
void PWU_Status_Timer_Check()
{
	ULONG status = 0, slotno = 0;
	ULONG powerid = 0;
	ULONG ac_count = 0, dc_count = 0;
	USHORT alarm = 0, stat[4] = {0,0,0,0};
	
	LONG  voltage[4] = {0,0,0,0}, current[4] = {0,0,0,0};
	ULONG alarmid[4] = {0,0,0,0};
	UCHAR temper[4] = {0,0,0,0};
	static ULONG regstat_count[4] = {0,0,0,0};
	LONG tickbefore = 0, tickafter = 0;
	UCHAR alm1 = 0, alm2 = 0;
	int i;

	
	if(SYS_LOCAL_MODULE_RUNNINGSTATE != MODULE_RUNNING)
	{
		return;
	}


	if(PWU_CHECK_TIMER_DEFAULT == PWU_CHECK_TIMER)
	{
		PWU_CHECK_TIMER--;
	}
	else if(0 == PWU_CHECK_TIMER)
	{
		PWU_CHECK_TIMER = PWU_CHECK_TIMER_DEFAULT;
		return;
	}
	else if(PWU_CHECK_TIMER--)
	{
		return;
	}


	for(powerid = 0; powerid < PRODUCT_MAX_TOTAL_POWER_COUNT; powerid++)
	{
		status = pwu_status[powerid]; 
		slotno = powerId2slotno[powerid];
	
		switch(status)
		{
			case PWU_STAT_NULL:
			case PWU_STAT_INSERT:

				alarmid[powerid] |= PWU_POWEROFF;	
				break;
				
			case PWU_STAT_RUN:
				
				if(SYS_MODULE_TYPE_IS_DC48_PWU(__SYS_MODULE_TYPE__(slotno)))
				{
					pwu_module_info_t[powerid].pwu_moduletype = PWU_TYPE_DC48;
					dc_count++;

					READ_PWU_STATUS_ALARM(alm1,alm2,alarm);
					
					if((~alarm) & PWU48_ALARM_PWUID_2_ERRNO[powerid])
					{
						alarmid[powerid] |= DC_VOLT_OUT_RANGE;
					}
				}
				else if(SYS_MODULE_TYPE_IS_AC220_PWU(__SYS_MODULE_TYPE__(slotno)))
				{
					pwu_module_info_t[powerid].pwu_moduletype = PWU_TYPE_AC220;
					ac_count++;
					
					if(VOS_OK == PWU_READ_VOLT_CUR_VALID(powerid, voltage, current))
					{
						pwu_module_info_t[powerid].pwu_voltage = voltage[powerid];
						pwu_module_info_t[powerid].pwu_current = current[powerid];
					}
					else
					{
						alarmid[powerid] |= REG_STAT_ABNORMAL;
					}

					if(VOS_OK == PWU_READ_STATUS_VALID(powerid, stat))
					{
						if(stat[powerid] & RS485_PWU220_LOW_FAULT)
						{
							alarmid[powerid] |= ACF_STAT_ABNORMAL;
						}
						else if(stat[powerid] & RS485_PWU220_LOW_ALARM)
						{
							alarmid[powerid] |= AC_LOWLINE_ABNORMAL;
						}
						else if(stat[powerid] & RS485_PWU220_FAN_ALARM)
						{
							alarmid[powerid] |= FAN_STAT_ABNORMAL;
						}
						else if(stat[powerid] & RS485_PWU220_HIGH_ALARM)
						{
							alarmid[powerid] |= HVSD_STAT_ABNORMAL;
						}
					}
					else
					{
						alarmid[powerid] |= REG_STAT_ABNORMAL;
					}

					if(VOS_OK == PWU_READ_TEMP_VALID(powerid, temper))
					{
						pwu_module_info_t[powerid].pwu_temper = temper[powerid];
						if(temper[powerid] > temperatureAlarmValueHigh || temper[powerid] < temperatureAlarmValueLow)
						{
							alarmid[powerid] |= AC_TEMP_ABNORMAL;
						}
					}
					else
					{
						alarmid[powerid] |= REG_STAT_ABNORMAL;
					}
				}
				break;
				
			default:
				break;
			
		}
	
	}


	/*双AC电源进行电压及稳流检测*/
	if(4 == ac_count)
	{
		PWU_VOLT_LOW_THRESHOLD = DOUBLE_AC_VOLT_LOW_THRESHOLD;
		PWU_VOLT_HIGH_THRESHOLD = DOUBLE_AC_VOLT_HIGH_THRESHOLD;

		/*一组电源两个槽位只进行一次比较*/
		for(powerid = 0; powerid < PRODUCT_MAX_TOTAL_POWER_COUNT/2; powerid++)
		{
			/*双AC电源进行电压检测*/
			if((voltage[powerid] > PWU_VOLT_HIGH_THRESHOLD*100) ||(voltage[powerid] < PWU_VOLT_LOW_THRESHOLD*100))
			{
				alarmid[powerid]   |= VOP_R_ABNORMAL;
				alarmid[powerid+2] |= VOP_R_ABNORMAL;
			}

			if(current[powerid] < DOUBLE_AC_CUR_LOW_THRESHOLD*100)
			{
				alarmid[powerid]   |= I_R_ABNORMAL;
				alarmid[powerid+2] |= I_R_ABNORMAL;
			}
		}

		#if 0  /*修改稳流检测条件，只比较电流值 @muqw 2017-5-27*/
		/*当AC电源功率大于400W时进行稳流检测*/
		if((voltage[0]*current[0] + voltage[1]*current[1]) >= 400*10000)
		{
			if(abs(current[0]-current[1])/(current[0]<current[1]?current[0]:current[1]) > 0.03)
			{
				alarmid[0] |=  I_R_ABNORMAL;
				alarmid[1] |=  I_R_ABNORMAL;
				alarmid[2] |=  I_R_ABNORMAL;
				alarmid[3] |=  I_R_ABNORMAL;
			}
		}		
		#endif

	}
	else if(2 == ac_count)
	{
		PWU_VOLT_LOW_THRESHOLD = DC_AC_VOLT_LOW_THRESHOLD;
		PWU_VOLT_HIGH_THRESHOLD = DC_AC_VOLT_HIGH_THRESHOLD;
		
		/*DC+AC电源进行电压检测*/ /*一组电源两个槽位只进行一次比较*/
		for(powerid = 0; powerid < PRODUCT_MAX_TOTAL_POWER_COUNT/2; powerid++)
		{
			slotno = powerId2slotno[powerid];
			if((SYS_MODULE_TYPE_IS_AC220_PWU(__SYS_MODULE_TYPE__(slotno)))
				&&(SYS_MODULE_TYPE_IS_AC220_PWU(__SYS_MODULE_TYPE__(slotno+2))))
			{
				/*双AC电源进行电压检测*/
				if((voltage[powerid] > PWU_VOLT_HIGH_THRESHOLD*100) ||(voltage[powerid] < PWU_VOLT_LOW_THRESHOLD*100))
				{
					alarmid[powerid]   |= VOP_R_ABNORMAL;
					alarmid[powerid+2] |= VOP_R_ABNORMAL;
				}
			}
		}
	}
	else
	{
		/*DC电源无法进行相关检测*/
	}

	for(powerid = 0; powerid < PRODUCT_MAX_TOTAL_POWER_COUNT; powerid++)
	{
		slotno = powerId2slotno[powerid];

		/*电源寄存器状态有可能会读取失败, 暂不告警 @muqw 2017-5-27*/
		if(alarmid[powerid]&REG_STAT_ABNORMAL)
		{		
			alarmid[powerid] &= ~REG_STAT_ABNORMAL;
		}

		if(VOS_OK == PWU_Status_Alarm_Report(slotno, alarmid[powerid], pwu_module_info_t[powerid].pwu_alarmed))
		{
			pwu_module_info_t[powerid].pwu_alarmed = alarmid[powerid];
		}
		
	}
	
	return;

}

VOID pwu_module_info_get(CHAR *pwu_info, ULONG slotno, ULONG len)
{
	ULONG powerid = slotno2powerId[slotno];
	
	VOS_MemCpy((CHAR *)pwu_info, (CHAR *)&(pwu_module_info_t[powerid].pwu_alarmed), len);

	return ;
}

VOID pwu_module_info_set(CHAR *pwu_info, ULONG slotno, ULONG len)
{
	ULONG powerid = slotno2powerId[slotno];
	
	VOS_MemCpy((CHAR *)&(pwu_module_info_t[powerid].pwu_alarmed), (CHAR *)pwu_info, len);

	return ;
}

/*END: add for Pwu Status Alarm by @muqw 2017-5-6*/

void pwdTimerProc( ULONG powerId ,ULONG event )
{
	ULONG status = 0;
	POWERID_ASSERT( powerId );

	status = pwu_status[ powerId ];

	switch( status )
	{
		case PWU_STAT_NULL :
		{
			switch( event )
			{
				case PWD_ON:
					pwu_status[ powerId ] = PWU_STAT_RUN;
					pwu_status_mask[ powerId ] = 1 << PWU_STAT_RUN;
					if( cl_interrupt_status )
					{
						PWU_PowerOffClear( powerId );
						PWU_IntEnable( powerId );
					}
				break;
				case PWD_INSERT:
					pwu_status[ powerId ] = PWU_STAT_INSERT;
					pwu_status_mask[ powerId ] = 1 << PWU_STAT_INSERT;
				break;
				default:
					VOS_ASSERT( 0 );
				break;
			}
		}
		break;
		case PWU_STAT_INSERT :
			switch( event )
			{
				case PWD_PULLOUT:
					pwu_status[ powerId ] = PWU_STAT_NULL;
					pwu_status_mask[ powerId ] = 1 << PWU_STAT_NULL;
				break;
				case PWD_ON:
					pwu_status[ powerId ] = PWU_STAT_RUN;
					pwu_status_mask[ powerId ] = 1 << PWU_STAT_RUN;
					if( cl_interrupt_status )
					{
						PWU_PowerOffClear( powerId );
						PWU_IntEnable( powerId );
					}
				break;
				default:
					VOS_ASSERT( 0 );
			}
		break;
		case PWU_STAT_RUN :
			switch( event )
			{
				case PWD_PULLOUT:
					pwu_status[ powerId ] = PWU_STAT_NULL;
					pwu_status_mask[ powerId ] = 1 << PWU_STAT_NULL;
					if( cl_interrupt_status )
					{
						PWU_PowerOff( powerId );
					}
				break;
				case PWD_INSERT:
				pwu_status[ powerId ] = PWU_STAT_INSERT;
				pwu_status_mask[ powerId ] = 1 << PWU_STAT_INSERT;
				if( cl_interrupt_status && ((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)|| (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000) ||
					(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)))
				{
					PWU_PowerOff( powerId );
				}
				break;
				case PWD_ON:
					if(alarm6900(powerId))
					{
						PWU_PowerError(powerId);
					}
					if(alarm6900Clr(powerId))
					{
						PWU_PowerErrorClear(powerId);
					}
				break;
			}
		break;
		default:
			VOS_ASSERT( 0 );
		break;
	}
	
}

void pwdMsgProcess()
{
	LONG lRet = VOS_ERROR;
	ULONG aulMsg [ 4 ] = {0};
	ULONG msgType = 0 ;
	ULONG powerId = 0;
	ULONG event = 0;
	
	while( 1 )
	{
		lRet = VOS_QueReceive( pwdTrapQueId , aulMsg , WAIT_FOREVER );
		if( VOS_ERROR == lRet )
		{
			VOS_ASSERT( 0 );
			VOS_TaskDelay( 100 );
			continue;
		}

		msgType = aulMsg[1];
		powerId = aulMsg[2];
		event = aulMsg[3];
		
		switch( msgType )
		{
			case MSG_PWD_OFF:
				pwdTrapProc( powerId );
			break;
			case MSG_PWD_TIMER:
				pwdTimerProc( powerId , event );
			break;
			default:
				break;
		}
	}
}



/*添加中断和gpio切换的方式*/
BOOL readPowerBrdGpio(UCHAR powerBrdIdx)
{
	int gpioValue = 0;

	POWERID_ASSERT( powerBrdIdx )
	
	gpioValue = read_gpio( (int)(powerBrdIdx + PowerBrdGpioStart) );

	if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
		return ( gpioValue == 0 ) ? TRUE : FALSE ;
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
		return ( gpioValue == 0 ) ? FALSE : TRUE ;
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 ){}
	else if(( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) || ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S))
	{
		if (0 == powerBrdIdx)
		{
			gpioValue = read_gpio( 7 );
			return ( gpioValue == 0 ) ? FALSE : TRUE ;
		}
		else if (1 == powerBrdIdx)
		{
			gpioValue = read_gpio( 19 );
			return ( gpioValue == 0 ) ? FALSE : TRUE ;
		}
	}
	return FALSE;
}

void isPWUupdown( ULONG slotNo )
{
	ULONG aulMsg[4] = { 0 };
	ULONG powerId = 0 ;
#if 0
	if( 0 == pwuAlmSemB )
		return ;
		
	if ( VOS_ERROR == VOS_SemTake( pwuAlmSemB , NO_WAIT ) )
		return;
		
	VOS_SemGive( pwuAlmSemB );
#endif

	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		if( slotNo < 4 || slotNo >5 )
			return ;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
	{
		if( slotNo < 9 || slotNo > 11 )
			return ;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000))
	{
		if( slotNo < 18 || slotNo > 21 )
			return ;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if( slotNo < 5 || slotNo > 6 )
			return ;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
	{
		if( slotNo < 3 || slotNo > 4 )
			return ;
	}

	powerId = slotno2powerId[ slotNo ];
	if( device_chassis_is_slot_inserted( slotNo ) )
	{
		if ( isPWU48V( powerId ) )
		{
			pwu_status[ powerId ] = PWU_STAT_INSERT;
		}
		else
		{
			/*避免isr里的误报,导致此处读gpio时认为是一个掉电*/
			if( flag_when_interrupt[ powerId ] )
			{
				VOS_TaskDelay(10);
				flag_when_interrupt[ powerId ] = 0;
			}
			if( brdOn( powerId ) )
			{
				if( pwu_status_mask[ powerId ] & ( 1 << PWU_STAT_RUN ) && (SYS_PRODUCT_TYPE != PRODUCT_E_GFA6900) && 
					(SYS_PRODUCT_TYPE != PRODUCT_E_GFA6900M) && (SYS_PRODUCT_TYPE != PRODUCT_E_GFA6900S)&&(SYS_PRODUCT_TYPE != PRODUCT_E_GFA8000))
					return ;
				
				aulMsg[1] = MSG_PWD_TIMER;
				aulMsg[2] = powerId;
				aulMsg[3] = PWD_ON;
			}
			else
			{
				if( pwu_status_mask[ powerId ] & ( 1 << PWU_STAT_INSERT ) )
					return ;

				aulMsg[1] = MSG_PWD_TIMER;
				aulMsg[2] = powerId;
				aulMsg[3] = PWD_INSERT;
			}
		}
	}
	else
	{
		if( pwu_status_mask[ powerId ] & ( 1 << PWU_STAT_NULL ) )
			return ;
		aulMsg[1] = MSG_PWD_TIMER;
		aulMsg[2] = powerId;
		aulMsg[3] = PWD_PULLOUT;
	}
	if( 0 != pwdTrapQueId &&  0 != aulMsg[1]  )
	{
		if ( VOS_ERROR == VOS_QueSend( pwdTrapQueId , aulMsg , NO_WAIT, MSG_PRI_NORMAL ) )
		{
			VOS_ASSERT( 0 );
		}
	}
}

void wdTimerCallBack()
{
	int i ;
	int lockKey;
	
	lockKey = intLock();
	for( i = 0 ; i < sys_max_power_count ; i ++ )
	{
		if( interrupt_counter[ i ] > interpt_limit )
		{
			if( cl_interrupt_status )
				PWU_IntEnable( i );
		}
		interrupt_counter[ i ] = 0 ;
	}
	if ( VOS_ERROR == wdStart ( pwdWdTimerId , PWD_WD_TIMER , wdTimerCallBack , NULL ) )
	{
		logMsg("\r\n start wd timer error ",0,0,0,0,0,0);
	}
	intUnlock(lockKey);
	return;
}


/*初始化要放到系统启动之后*/
VOID initPowerAlarm()
{
	int i;
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

	/* 初始化先把中断关闭 */
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		sys_max_power_count = 2;	
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
        pwuIntIrq[0] = pabIntIrqs[5];
        pwuIntIrq[1] = pabIntIrqs[6];
#else
		UIC0_SR_Offset[0] = UINT32_BIT_LEN - INT_LVL_EXT_IRQ_5 - 1;
		UIC0_SR_Offset[1] = UINT32_BIT_LEN - INT_LVL_EXT_IRQ_6 - 1;
#endif
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		sys_max_power_count = 3;
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
        pwuIntIrq[0] = pabIntIrqs[0];
        pwuIntIrq[1] = pabIntIrqs[1];
        pwuIntIrq[2] = pabIntIrqs[2];
#else
		UIC0_SR_Offset[0] = UINT32_BIT_LEN - INT_LVL_EXT_IRQ_0 - 1;
		UIC0_SR_Offset[1] = UINT32_BIT_LEN - INT_LVL_EXT_IRQ_1 - 1;
		UIC0_SR_Offset[2] = UINT32_BIT_LEN - INT_LVL_EXT_IRQ_2 - 1;
#endif
		
		/*PWU_IntDisable(2);*/
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000))
	{
		sys_max_power_count = 4;
	}
	else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
	{
		sys_max_power_count = 2;
	}
	else
	{
		sys_max_power_count = 1;
		return;
	}

	/*PWU_IntDisable(0);
	PWU_IntDisable(1);*/

	for( i = 0; i < sys_max_power_count; i++ )
	{
		PWU_IntDisable(i);
		
		pwuIntEnable[ i ] = PWU_INT_DISABLE;
		pwu_status[ i ] = PWU_STAT_NULL;
		PWU_ALARM[ i ] = PWU_ALM_CLEAR;
	}

	VOS_MemZero( slotno2powerId, sizeof(slotno2powerId) );
	VOS_MemZero( powerId2slotno, sizeof(powerId2slotno) );

	VOS_MemZero( pwu_module_info_t, sizeof(PWU_MODULE_INFO)*PRODUCT_MAX_TOTAL_POWER_COUNT );  /*add by @muqw 2017-4-26*/

	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		slotno2powerId[4] = 0;
		slotno2powerId[5] = 1;

		powerId2slotno[0] = 4;
		powerId2slotno[1] = 5;

		PowerBrdGpioStart = 22;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		slotno2powerId[9] = 0;
		slotno2powerId[10] = 1;
		slotno2powerId[11] = 2;

		powerId2slotno[0] = 9;
		powerId2slotno[1] = 10;
		powerId2slotno[2] = 11;

		PowerBrdGpioStart = 17;
		cl_interrupt_status = 0;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000))
	{
		slotno2powerId[18] = 0;
		slotno2powerId[19] = 1;
		slotno2powerId[20] = 2;
		slotno2powerId[21] = 3;

		powerId2slotno[0] = 18;
		powerId2slotno[1] = 19;
		powerId2slotno[2] = 20;
		powerId2slotno[3] = 21;
		
		enable_pwu_alarm(cl_interrupt_status);	/* modified by xieshl 20120313, 6900上默认打开电源检测 */
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		slotno2powerId[5] = 0;
		slotno2powerId[6] = 1;

		powerId2slotno[0] = 5;
		powerId2slotno[1] = 6;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
	{
		slotno2powerId[3] = 0;
		slotno2powerId[4] = 1;

		powerId2slotno[0] = 3;
		powerId2slotno[1] = 4;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100)
	{
		slotno2powerId[3] = 0;
		slotno2powerId[4] = 1;

		powerId2slotno[0] = 3;
		powerId2slotno[1] = 4;
	}

	devsm_module_pwu_runningstate_get_callback = pwu_brd_runningstate_get;	/* modified by xieshl 20120313, 需求13344 */

}

VOID startPowerAlarm()	/* modified by xieshl 20100128, 启动和基本信息初始化分开，否则，在disable时网管上看到的信息可能错误 */
{
	int i;
	if( 0 == cl_interrupt_status )
		return ;
		
	if ( service_interrupt_status )
		return ;
		
	service_interrupt_status = 1;
	/*pwuAlmSemB = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY );
	if( pwuAlmSemB == 0 )
	{
		sys_console_printf("\r\n Creat pwu Alarm Sem Error !");
	}*/

	pwdTrapQueId = VOS_QueCreate( PWD_TRAP_QUEUE_NUM , VOS_MSG_Q_FIFO );
	if ( 0 == pwdTrapQueId )
	{
		sys_console_printf("Creat pwdTrapQue Error !\r\n");
		VOS_ASSERT( 0 );
		return ;
	}

	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 || SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		pwdWdTimerId = wdCreate();
		if ( 0 == pwdWdTimerId )
		{
			sys_console_printf("Creat pwdWdTimer Error !\r\n");
			VOS_ASSERT( 0 );
		}

		if ( VOS_ERROR == wdStart( pwdWdTimerId , PWD_WD_TIMER , wdTimerCallBack , NULL ) )
		{
			sys_console_printf("start wdTimer Error !\r\n");
			VOS_ASSERT( 0 );
		}
	}
	pwdTrapTaskId = VOS_TaskCreate( "tPwuTask" , PWD_TASK_PRIO , pwdMsgProcess , NULL );
	if( pwdTrapTaskId == 0 )
	{
		sys_console_printf("Creat pwuTask Error !\r\n");
		VOS_ASSERT( 0 );
		return ;
	}

	VOS_QueBindTask( ( VOS_HANDLE ) pwdTrapTaskId , pwdTrapQueId );	

	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 || SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		for( i = 0 ; i < sys_max_power_count ; i ++ )
		{
			if (VOS_OK != powerInterruptConnect( i ) )
			{
				sys_console_printf("\r\n power %d is failed to enable.", i );
			}
		}
	}
	
	/*VOS_SemGive( pwuAlmSemB );*/

}

void PWU_IntEnable(UCHAR powerId)
{
	if( powerId >= sys_max_power_count/*PRODUCT_MAX_TOTAL_POWER_COUNT*/ )
		return;
	
	if (PWU_INT_ENABLE == pwuIntEnable[ powerId ])
	{
		return;
	}

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 || SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
        cpuIrqEnable(pwuIntIrq[ powerId ]);
#else
    	volatile UINT32 uic0_er = 0;
    		
		uic0_er = sysDcrInLong(UIC0_ER);
		uic0_er |= (1 << UIC0_SR_Offset[ powerId ]);
		sysDcrOutLong(UIC0_ER, uic0_er);
#endif
	}
    
	pwuIntEnable[ powerId ] = PWU_INT_ENABLE;
}

/*锁中断*/
void PWU_IntDisable(UCHAR powerId)
{
	int lockKey;

	if( powerId >= sys_max_power_count/*PRODUCT_MAX_TOTAL_POWER_COUNT*/ )
		return;

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 || SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		lockKey = intLock();	
		pwuIntEnable[ powerId ] = PWU_INT_DISABLE;

#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
        cpuIrqDisable(pwuIntIrq[ powerId ]);
        cpuIrqClear(pwuIntIrq[ powerId ]);
#else
        {
        	volatile UINT32 uic0_er = 0, uic0_sr = 0;

    		uic0_er = sysDcrInLong(UIC0_ER);
    		uic0_er &= ~(1 << UIC0_SR_Offset[ powerId ]);
    		sysDcrOutLong(UIC0_ER, uic0_er);

    		uic0_sr = sysDcrInLong(UIC0_SR);
    		uic0_sr |= 1 << UIC0_SR_Offset[ powerId ];
    		sysDcrOutLong(UIC0_SR, uic0_sr);
        }
#endif

		intUnlock( lockKey );
	}
	else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) || 
		(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000) 
		|| (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))/*add by yanjy2107-1*//*for 8100pwu-alarm disable 33162*/
	{
		pwuIntEnable[ powerId ] = PWU_INT_DISABLE;
	}
    
	return;
}

#if 0
extern LONG getModuleHardwareVersion( ULONG slotno, char * version );

/* 支持GPIO中断的电源板卡是否在位，在位返回true，否则false */
BOOL isPowerBrdInsert(UCHAR powerId)
{
	UCHAR slotno = 0;
	CHAR version[16] = "";
	BOOL ret = FALSE;

	POWERID_ASSERT( powerId )
	
	slotno = powerId2slotno[ powerId ];

	if ( MODULE_E_GFA_PWU220 == __SYS_MODULE_TYPE__(slotno) )
	{
		ret = TRUE;
	} 
	else if ( MODULE_E_GFA_PWU48 == __SYS_MODULE_TYPE__(slotno) )
	{
		if (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
		{
			memset(version, 0, sizeof(version));
			getModuleHardwareVersion( (ULONG)slotno, version );

			if (version[1] >= '2')
			{
				ret = TRUE;
			} 
		}
		else
			ret = TRUE;
	}
	return ret;
}
#endif
/* 支持GPIO中断的电源板卡是否在位并供电中，在位并供电中返回true，否则false */
BOOL isPowerBrdRun(UCHAR powerId )
{
	POWERID_ASSERT( powerId )
	if( isPWU48V( powerId ) )
	{
		return TRUE;
	}
	return brdOn( powerId );	
}

/* 如果是最后一个电源板卡，返回true */
BOOL isLastPowerBrdInterrupt( UCHAR powerId )
{
	ULONG i;

	POWERID_ASSERT( powerId )
	
	for( i = 0 ; i < sys_max_power_count ; i ++ )
	{
		if( i == powerId )
			continue;
		if( isPowerBrdRun( i ) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/* UCHAR PowerIdx 范围：0-2 */
void ISR_powerInterrupte( UCHAR powerId )
{
	ULONG lockKey;
	ULONG aulMsg[4] = { 0 };
	
	lockKey = intLock();

	/*if( powerId > 2 )*/
	if( powerId >= sys_max_power_count )
	{
		intUnlock( lockKey );
		return;
	}
	
	if (PWU_INT_DISABLE == pwuIntEnable[ powerId ])
	{
		POWERBRD_LOGMSG( ( "pwu %d alm disable \r\n", powerId, 0, 0, 0, 0, 0 ) );
		intUnlock( lockKey );
		return;
	}

	/* 系统没初始化完毕，不触发电源中断 */
	if (SYS_LOCAL_MODULE_RUNNINGSTATE != MODULE_RUNNING)
	{
		POWERBRD_LOGMSG( ("system not running pwu %d  \r\n", powerId, 0, 0, 0, 0, 0 ) );
		intUnlock( lockKey );
		return;
	}

	PWU_IntDisable( powerId );
	
	flag_when_interrupt[ powerId ] = 1;

	if(  ( ++interrupt_counter[ powerId ] )  <= interpt_limit )
	{
		aulMsg[1] = MSG_PWD_OFF;
		aulMsg[2] = powerId;
		
		if( 0 != pwdTrapQueId )
		{
			if ( VOS_ERROR == VOS_QueSend( pwdTrapQueId , aulMsg, NO_WAIT , MSG_PRI_NORMAL) )
			{
				if( cl_interrupt_status)
				{
					PWU_IntEnable( powerId );
					intUnlock( lockKey );
				}
				POWERBRD_LOGMSG( ( "send msg from int isr failed \r\n",0,0,0,0,0,0 ) );
			}
		}
	}
	else
	{
		PWU_IntDisable( powerId );
	}
	intUnlock( lockKey );
	return;
}

void ISR_powerInterrupte0(void)
{
	ISR_powerInterrupte(0);
	return;
}

void ISR_powerInterrupte1(void)
{
	ISR_powerInterrupte(1);
	return;
}

void ISR_powerInterrupte2(void)
{
	ISR_powerInterrupte(2);
	return;
}

/* UCHAR powerBrdIdx范围：0-2 */
STATUS powerInterruptConnect(UCHAR powerId )
{
	STATUS rc = VOS_ERROR;
	volatile UINT32 uic0_cr = 0, uic0_pr = 0, uic0_tr = 0;
    unsigned char *pabIntIrqs = CDSMS_LOCAL_MODULE_EXT_INT_BYTEARRAY;

	POWERID_ASSERT( powerId )

	PWU_IntDisable( powerId );

	switch (powerId)
	{
		case 0:
			if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
				rc = intConnect((VOIDFUNCPTR *)
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
				pabIntIrqs[5]
#else
				INT_LVL_EXT_IRQ_5
#endif
                , (VOIDFUNCPTR)ISR_powerInterrupte0, 0);
			else if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
				rc = intConnect((VOIDFUNCPTR *)
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
				pabIntIrqs[0]
#else
				INT_LVL_EXT_IRQ_0
#endif
                , (VOIDFUNCPTR)ISR_powerInterrupte0, 0);
			break;
		case 1:
			if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
				rc = intConnect((VOIDFUNCPTR *)
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
				pabIntIrqs[6]
#else
				INT_LVL_EXT_IRQ_6
#endif
				, (VOIDFUNCPTR)ISR_powerInterrupte1, 0);
			else if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
				rc = intConnect((VOIDFUNCPTR *)
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
				pabIntIrqs[1]
#else
				INT_LVL_EXT_IRQ_1
#endif
				, (VOIDFUNCPTR)ISR_powerInterrupte1, 0);
			break;
		case 2:
			if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
			{
			}
			else if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
				rc = intConnect((VOIDFUNCPTR *)
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
				pabIntIrqs[2]
#else
				INT_LVL_EXT_IRQ_2
#endif
                , (VOIDFUNCPTR)ISR_powerInterrupte2, 0);
			break;
		default:
			break;
	}

	if( rc == VOS_OK )
	{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
		if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
            cpuIrqConfig(pwuIntIrq[ powerId ], 1, 1);	
        else
            cpuIrqConfig(pwuIntIrq[ powerId ], 1, 0);	
#else
		/* non-critical */
		uic0_cr = sysDcrInLong(UIC0_CR);
		uic0_cr &= ~(1 << UIC0_SR_Offset[ powerId ]);
		sysDcrOutLong(UIC0_CR, uic0_cr);

		/* rising edge */
		uic0_pr = sysDcrInLong(UIC0_PR);
		if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
			uic0_pr |= (1 << UIC0_SR_Offset[ powerId ]);
		else
			uic0_pr &= ~(1 << UIC0_SR_Offset[ powerId ]);
		sysDcrOutLong(UIC0_PR, uic0_pr);


		/* Edge-sensitive */
		uic0_tr = sysDcrInLong(UIC0_TR);
		uic0_tr |= (1 << UIC0_SR_Offset[ powerId ]);
		sysDcrOutLong(UIC0_TR, uic0_tr);
#endif

	}
	else
	{
		POWERBRD_PRINT( ("Power %d interrupt_load: interrupt connect error\r\n", powerId ) );
	}
	return rc;
}

/* 旧版48V电源板卡只要在位，就算是供电中 */
/* 在位返回true，不在位返回false */
BOOL isPWU48V(UCHAR powerId )
{
	UCHAR slotno = 0;
	CHAR version[16];

	POWERID_ASSERT( powerId )

	slotno = powerId2slotno[ powerId ];

	if (SYS_PRODUCT_TYPE != PRODUCT_E_EPON3)	/* 6100不存在新旧48V电源问题 */
		return FALSE;

	if ( MODULE_E_GFA_PWU48 == __SYS_MODULE_TYPE__(slotno) )
	{
		memset(version, 0, sizeof(version));
		getModuleHardwareVersion( (ULONG)slotno, version );

		if (version[1] == '1')
		{
			return TRUE;
		}
		else if ( (version[1] < '0') || (version[1] > '9') )
		{
			/* 读不出版本的情况下，就当成旧版的48V PWU */
			POWERBRD_PRINT( ( "Get Slot%d Version Error!    version=%s\r\n", slotno, version ) );
			return TRUE;
		}
	}

	return FALSE;
}

/* 判断其他两个槽位是否有旧版的48V电源板卡 */
/* 返回false说明另外两个槽位没有旧版48V电源供电 */
BOOL checkPWU48VSupply(UCHAR powerId)
{
	ULONG i = 0 ;
	POWERID_ASSERT( powerId )

	for( i = 0 ; i < sys_max_power_count ; i ++)
	{
		if( powerId == i )
			continue;
		if ( TRUE == isPWU48V( i ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* added by xieshl 20120313, 电源掉电告警检测使能，需求13344 */
LONG enable_pwu_alarm( int enable )
{
	ULONG i = 0;

	for (i = 0; i < sys_max_power_count; i++)
	{
		if ( enable )
		{
			cl_interrupt_status = 1;
			
			startPowerAlarm();
			
			if (PWU_INT_DISABLE == pwuIntEnable[ i ])
			{
				PWU_IntEnable( i );
			}
		} 
		else
		{
			cl_interrupt_status = 0;
			
			if ( PWU_INT_ENABLE == pwuIntEnable[ i ] )
			{
				PWU_IntDisable( i );
			}
		}
	}
	return VOS_OK;
}

/* 调试开关 */
DEFUN  (
		pwu_power_alarm_debug,
		pwu_power_alarm_debug_cmd,
		"debug pwu-alarm",
		 "Debug information\n"
		"power on or power off alarm debug enable\n"
		)
{
	debugPowerBrdInterrupt = POWERBRDINT_INFO;
	return CMD_SUCCESS;
}

DEFUN  (
		pwu_power_alarm_undo_debug,
		pwu_power_alarm_undo_debug_cmd,
		"undo debug pwu-alarm",
	     "Delete configuration\n"
		 "Debug information\n"
		"power on or powen off alarm debug disable\n"
		)
{
	debugPowerBrdInterrupt = 0;
	return CMD_SUCCESS;
}

DEFUN  (
		pwu_alarm,
		pwu_alarm_cmd,
		"pwu-alarm [disable|enable]",
		"pwu power off alarm\n"
		"disable pwu alarm\n"
		"enable pwu alarm\n"
		)
{
	int enable = 0;

	if ( VOS_StrCmp(argv[0],"enable") == 0 )
		enable = 1;

	/* modified by xieshl 20120313, 需求13344 */
	if( enable_pwu_alarm(enable) == VOS_OK )
		return( CMD_SUCCESS );
	return CMD_WARNING;
}

DEFUN  (
		pwu_alarm_show,
		pwu_alarm_show_cmd,
		"show pwu-alarm",
		"Show information\n"
		"Show power alarm information\n"
		)
{
	showStatus( vty );
	return( CMD_SUCCESS );
}


/*BEGIN: add for Pwu Status Alarm by @muqw 2017-5-6*/
DEFUN  (
		pwu_status_show,
		pwu_status_show_cmd,
		"show pwu-status",
		"Show information\n"
		"Show power status alarm information\n"
		)
{
	showPwuStatus( vty );
	return( CMD_SUCCESS );
}


ULONG g_ulAlarmID = 0;

DEFUN( pwu_status_alarm_test,
	pwu_status_alarm_test_cmd,
	"pwu-status-test <18-21> [alarm|clear]{[in_volt]}*1 {[pwu_stat]}*1 {[pwu_fan]}*1 {[cur_limit]}*1 {[ac_volt]}*1 {[high_volt]}*1 {[out_cur]}*1 {[low_line]}*1 {[pwu_reg]}*1 {[dc_volt]}*1 {[power_off]}*1 {[temp_high]}*1",
	"Test pwu atatus alarm\n"
	"Please input pwu slotno\n"
	"Please choose alarm or clear\n"
	"Set Alarm info in_volt\n"
	"Set Alarm info pwu_stat\n"
	"Set Alarm info pwu_fan\n"
	"Set Alarm info cur_limit\n"
	"Set Alarm info ac_volt\n"
	"Set Alarm info high_volt\n"
	"Set Alarm info out_cur\n"
	"Set Alarm info low_line\n"
	"Set Alarm info pwu_reg\n"
	"Set Alarm info dc_volt\n"
	"Set Alarm info power_off\n"
	"Set Alarm info temp_high\n")
{
	ULONG ulAlarmID = 0;
	ULONG ulSlotno  = 0;
	LONG  alarmtag = -1;
	int i, j, rv;
	
	if(argc <= 2)
	{
		vty_out(vty, "%% Please Input test alarm Info.\r\n");
		return CMD_ERR_AMBIGUOUS;
	}
	else
	{
		ulSlotno = VOS_AtoI(argv[0]);

		if( ulSlotno < 18 || ulSlotno > 21)
		{
			vty_out(vty, "%% Slot is not correct.\r\n");
			return CMD_WARNING;
		}

		
		for(i = 2; i < argc; i++ )
		{
			if( VOS_StrCmp( argv[i], "in_volt" ) ==0 )
			{
				ulAlarmID |= ACF_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "pwu_stat") == 0 )
			{
				ulAlarmID |= RFA_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "pwu_fan") == 0 )
			{
				ulAlarmID |= FAN_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "cur_limit") == 0 )
			{
				ulAlarmID |= CL_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "ac_volt") == 0 )
			{
				ulAlarmID |= VOP_R_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "high_volt") == 0 )
			{
				ulAlarmID |= HVSD_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "out_cur") == 0 )
			{
				ulAlarmID |= I_R_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "low_line") == 0 )
			{
				ulAlarmID |= AC_LOWLINE_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "pwu_reg") == 0 )
			{
				ulAlarmID |= REG_STAT_ABNORMAL;
			}
			else if( VOS_StrCmp(argv[i], "dc_volt") == 0 )
			{
				ulAlarmID |= DC_VOLT_OUT_RANGE;
			}
			else if( VOS_StrCmp(argv[i], "power_off") == 0 )
			{
				ulAlarmID |= PWU_POWEROFF;
			}
			else if( VOS_StrCmp(argv[i], "temp_high") == 0 )
			{
				ulAlarmID |= AC_TEMP_ABNORMAL;
			}
		}
		

		if(g_ulAlarmID != 0)
		{
			if(0 == VOS_StrCmp("alarm", argv[1]))
			{
				for(j = 0; j < 32; j++)
				{
					if((ulAlarmID & (1<j)) && (g_ulAlarmID & (1<j)))
					{
						vty_out(vty, " alarmid has already reported.\r\n");
						return CMD_WARNING;
					}
				}
				alarmtag = 1;

			}
			else if(0 == VOS_StrCmp("clear", argv[1]))
			{
				alarmtag = 0;
				for(j = 0; j < 32; j++)
				{
					if((ulAlarmID & (1<j)) && (!(g_ulAlarmID & (1<j))))
					{
						vty_out(vty, " alarmid is not exist.\r\n");
						return CMD_WARNING;
					}
				}
			}

		}
		else 
		{
			if(0 == VOS_StrCmp("clear", argv[1]))
			{
				vty_out(vty, " alarmid is not exist.\r\n");
				return CMD_WARNING;
			}
		}
		g_ulAlarmID = ulAlarmID;

		if(1 == alarmtag)
		{
			if(VOS_OK != PWUStatusAbnormal_EventReport(OLT_DEV_ID, ulSlotno, ulAlarmID))
			{
				vty_out(vty, "%% alarm report fail.\r\n");
				return CMD_WARNING;
			}
		}
		else if(0 == alarmtag)
		{
			if(VOS_OK != PWUStatusAbnormalClear_EventReport(OLT_DEV_ID, ulSlotno, ulAlarmID))
			{
				vty_out(vty, "%% alarm clear report fail.\r\n");
				return CMD_WARNING;
			}
		}

		return CMD_SUCCESS;
	}
}

/*END: add for Pwu Status Alarm by @muqw 2017-5-6*/

DEFUN  (
		pwu_alarm_set,
		pwu_alarm_set_cmd,
		"pwu-alarm limit <1-100>",
		"pwu power off alarm\n"
		"the number of power off interrupt,system can proceed every 10(s)\n"
		"the range\n"
		)
{	
	interpt_limit = VOS_AtoI( argv[ 0 ] );
	return CMD_SUCCESS ;
}
STATUS powerDownAlarm_showrun( struct vty * vty )
{
	vty_out( vty, "!Power alarm config\r\n" );
	/*电源告警6900默认是打开的*/
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000))
	{
		if( !cl_interrupt_status )
		{
			vty_out( vty, " pwu-alarm disable\r\n" );
		}
	}
	else
	{
		if( cl_interrupt_status )
		{
			vty_out( vty, " pwu-alarm enable\r\n" );
		}
	}
	
	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}

STATUS powerBrdPowerOnAndOffAlarm_Command_Install()
{
	install_element ( DEBUG_HIDDEN_NODE, &pwu_power_alarm_debug_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &pwu_power_alarm_undo_debug_cmd );
	install_element ( CONFIG_NODE, &pwu_alarm_cmd );
	install_element ( CONFIG_NODE, &pwu_alarm_show_cmd );
	install_element ( VIEW_NODE, &pwu_alarm_show_cmd );
	install_element ( CONFIG_NODE, &pwu_alarm_set_cmd );
	/*install_element ( CONFIG_NODE, &pwu_show_cmd );*/
	install_element ( DEBUG_HIDDEN_NODE, &pwu_status_show_cmd );
	install_element ( DEBUG_HIDDEN_NODE, &pwu_status_alarm_test_cmd );
	
	return VOS_OK;
}

void PWU_PowerOff(ULONG powerId )
{
	ULONG slotno;
	POWERID_ASSERT( powerId );
	slotno = powerId2slotno[ powerId ];
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
		return;
	if (PWU_ALM_CLEAR == PWU_ALARM[ powerId ])
	{
		PWUPowerOff_EventReport( OLT_DEV_ID, slotno , __SYS_MODULE_TYPE__(slotno) );
		/*if (VOS_OK != PWUPowerOff_EventReport( 1, slotno , __SYS_MODULE_TYPE__(slotno)))
		{
			POWERBRD_PRINT( ( "PWU_PowerOff(%d)::PWUPowerOff_EventReport()     error!\r\n", slotno ) );
		}*/
		PWU_ALARM[ powerId ] = PWU_ALM_ON;
	}
	else
	{
		POWERBRD_PRINT( ( "PWU_PowerOff(%d)::PWU_ALARM[%d]=%d  error!\r\n", slotno, slotno, PWU_ALARM[ powerId ] ) );
	}
}

void PWU_PowerOffClear(ULONG powerId )
{
	ULONG slotno;
	POWERID_ASSERT( powerId );
	slotno = powerId2slotno[ powerId ];
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
		return;
	if (PWU_ALM_ON == PWU_ALARM[ powerId ])
	{
		PWUPowerOn_EventReport( OLT_DEV_ID, slotno , __SYS_MODULE_TYPE__(slotno) );
		/*if (VOS_OK != PWUPowerOn_EventReport( 1, slotno , __SYS_MODULE_TYPE__(slotno)))
		{
			POWERBRD_PRINT( ( "PWU_PowerOffClear(%d)::PWUPowerOn_EventReport()     error!\r\n", slotno ) );
		}*/
		PWU_ALARM[ powerId ] = PWU_ALM_CLEAR;
	}
	else
	{
		POWERBRD_PRINT( ( "PWU_PowerOffClear(%d)::PWU_ALARM[%d]=%d     error!\r\n", powerId, powerId, PWU_ALARM[ powerId ] ) );
	}
}

UCHAR * pwdStat2Str( UCHAR stat )
{
	switch ( stat )
	{
		case PWU_STAT_RUN:
			return PWU_STAT_RUN_STR;		
		break;
		case PWU_STAT_NULL:
			return PWU_STAT_NULL_STR;
		break;
		case PWU_STAT_INSERT:
			return PWU_STAT_INSERT_STR;
		break;
		default:
			return "UNKNOWN";
	}
}

LONG getPwdStatus( ULONG powerIdx )	/* added by xieshl 20100128,  问题单9766 */
{
	LONG status = VOS_ERROR;
	if( (powerIdx <= sys_max_power_count) && (powerIdx != 0) )
	{
		switch ( pwu_status[powerIdx - 1] )
		{
			case PWU_STAT_RUN:
				status = 3;
				break;
			case PWU_STAT_NULL:
				status = 1;
				break;
			case PWU_STAT_INSERT:
				status = 2;
				break;
			default:
				break;
		}
	}
	return status;
}


void showStatus( struct vty * vty )
{
	ULONG i,slotNo;
	vty_out( vty,"\r\nSlot RunningState  AlarmAbility  AlarmState \r\n");
	vty_out( vty,"\r\n");
	
	for( i = 0 ; i < sys_max_power_count ; i ++ )
	{
		slotNo = powerId2slotno[ i ];
		vty_out( vty,"%3d  %-12s  %-12s  %-5s\r\n",slotNo , 
											  pwdStat2Str( pwu_status[ i ] ) , 
											  ( PWU_INT_ENABLE == pwuIntEnable[ i ] ) ? "ENABLE" : "DISABLE" ,
											  ( PWU_ALARM[ i ] == PWU_ALM_CLEAR ) ? "CLR" : "ALM" );
	}
	vty_out( vty,"\r\n");
	return ;
}

/*BEGIN: add for Pwu Status Alarm by @muqw 2017-5-6*/
CHAR *PwuTypeString(ULONG slotno)
{
	if(SYS_MODULE_TYPE_IS_AC220_PWU(__SYS_MODULE_TYPE__(slotno)))
	{
		return "PWU220";
	}
	else if(SYS_MODULE_TYPE_IS_DC48_PWU(__SYS_MODULE_TYPE__(slotno)))
	{
		return "PWU48";
	}
	else
	{
		return "UNKOWN";
	}
}



void showPwuStatus( struct vty * vty )
{
	ULONG i,slotNo;
	vty_out( vty,"\r\nSlot  PowerType  AlarmID   Temper    Volt    Curr \r\n");
	vty_out( vty,"\r\n");
	
	for( i = 0 ; i < sys_max_power_count ; i ++ )
	{
		slotNo = powerId2slotno[ i ];
		vty_out( vty,"%3d    %-8s   0x%-6x   %-4d  %4d.%-2d  %2d.%-2d \r\n",slotNo , 
											  PwuTypeString(slotNo), 
											  pwu_module_info_t[i].pwu_alarmed,
											  pwu_module_info_t[i].pwu_temper,
											  pwu_module_info_t[i].pwu_voltage/100, pwu_module_info_t[i].pwu_voltage%100,
											  pwu_module_info_t[i].pwu_current/100, pwu_module_info_t[i].pwu_current%100
											  );
	}
	vty_out( vty,"\r\n");
	return ;
}


LONG getPwdModuleType( ULONG powerIdx )
{
	ULONG slotno = 0;
	LONG moduletype = VOS_ERROR;
	
	POWERID_ASSERT( powerIdx );
	slotno = powerId2slotno[powerIdx-1];
	
	if( (powerIdx <= sys_max_power_count) && (powerIdx != 0) )
	{
		if(SYS_MODULE_TYPE_IS_AC220_PWU(__SYS_MODULE_TYPE__(slotno)))
		{
			moduletype = 1;
		}
		else if(SYS_MODULE_TYPE_IS_DC48_PWU(__SYS_MODULE_TYPE__(slotno)))
		{
			moduletype = 2;
		}
	}
	
	return moduletype;
}

LONG getPwdVoltage( ULONG powerIdx )
{
	return pwu_module_info_t[powerIdx-1].pwu_voltage;
}

LONG getPwdCurrent( ULONG powerIdx )
{
	return pwu_module_info_t[powerIdx-1].pwu_current;
}


/*为保留小数点以后两位的精度，乘以100后再输出  @muqw 201-5-27*/
LONG getPwdVoltageHighThreshold( ULONG powerIdx )
{
	return PWU_VOLT_HIGH_THRESHOLD*100;
}

LONG getPwdVoltageLowThreshold( ULONG powerIdx )
{
	return PWU_VOLT_LOW_THRESHOLD*100;
}

LONG getPwuTemperatureAverage(ULONG powerIdx)
{
	return pwu_module_info_t[powerIdx-1].pwu_temper;
}

LONG getPwdAlarmInfo( ULONG powerIdx )
{
	return pwu_module_info_t[powerIdx-1].pwu_alarmed;;
}

LONG getPwdAlarmID( ULONG powerIdx )
{
	return ulpwualarmid[powerIdx-1];
}

LONG getPwdClearAlarmID( ULONG powerIdx )
{
	return ulpwuclearalarmid[powerIdx-1];
}



/*END: add for Pwu Status Alarm by @muqw 2017-5-6*/


#endif	/* EPON_MODULE_POWEROFF_INT_ISR */

#ifdef __cplusplus
}
#endif

