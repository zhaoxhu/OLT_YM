#ifdef __cplusplus
extern "C" {
#endif

#include  "OltGeneral.h"
#include "gwEponSys.h"
/*#include  "PonGeneral.h"*/
#include  "OnuGeneral.h"

#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
#include "cli/cli.h"
#include "eventMain.h"
#include "i2c.h"
/*#define OLT_ENVIRONMENT_MONITOR*/

#define MAX_ALARM_TEMPERATURE            /*200*/125/* because 126 is not a valid value */
#define MIN_ALARM_TEMPERATURE            -50
#define DEFAULT_MAX_ALARM_TEMPERATURE    60/* because liy said to increase high alarm temperature */
#define DEFAULT_MIN_ALARM_TEMPERATURE    -10
#define TEMPERATURE_ALARM_ENABLE         1
#define TEMPERATURE_ALARM_DISABLE        2
#define FAN_ALARM_ENABLE                 1
#define FAN_ALARM_DISABLE                2
#define FAN_ALARM_DEFAULT_MIN_REV        20
#define FAN_6900_ALARM_DEFAULT_MIN_REV   100

#define TEMPERATURE_ALARM_MODE_INTEGRATED		1
#define TEMPERATURE_ALARM_MODE_DISTRIBUTION	2

int temp_debug = 0;

#define TEMP_DEBUG(x) {if(temp_debug) sys_console_printf(x);}

 UCHAR fanDebug = 0;
 UCHAR temperatureDebug = 0;

 UCHAR temperatureAlarmEnable = TEMPERATURE_ALARM_DISABLE;/* 温度告警使能，0无效，1有效 */
 UCHAR fanAlarmEnable         = FAN_ALARM_DISABLE;/* 风扇告警使能，0无效，1有效 */

 LONG temperatureAlarmValueHigh = DEFAULT_MAX_ALARM_TEMPERATURE;/* 网管设置的温度告警上限值 */
 LONG temperatureAverage        = 0;/* 网管设置的温度告警上限值 */
 LONG temperatureAlarmValueLow  = DEFAULT_MIN_ALARM_TEMPERATURE;/* 网管设置的温度告警下限值 */
 LONG temperatureAlarmMode = TEMPERATURE_ALARM_MODE_DISTRIBUTION;
 ULONG fanMinRev                = FAN_ALARM_DEFAULT_MIN_REV;/* 6100风扇最小转速，一秒钟内的转速 */
 ULONG fanMibRev[8]             = {0, 0, 0, 0, 0, 0, 0, 0};/* 提供给网管的转速值 */
 UCHAR fanAlarm[8]              = {0, 0, 0, 0, 0, 0, 0, 0};/* 置1是有告警，下标1-3对应6100的3个风扇，下标1-4对应6700的4个风扇 */

int temperature_gpio = 0;
int fan_num_total = 4;
#define FAN_SPEED_FAST 1
#define FAN_SPEED_SLOW 2
#define FAN_MAX_SLOT 20
UCHAR g_fan_speed[FAN_MAX_SLOT];
ULONG ot_alm[8]={0};
ULONG fanfail_alm[8]={0};
/* wangysh add 6900*/

#define TEMP_CHAN1_REG 0x00
#define TEMP_CHAN2_REG 0x01
#define STATUS_REG     0x02
#define MASK_REG       0x03
#define GOBAL_CONFIG_REG   0x04
#define TEMP_CHAN1_EXT_REG 0x05
#define TEMP_CHAN2_EXT_REG 0x06
#define CHAN1_ALERT_LMT_REG 0x08
#define CHAN2_ALERT_LMT_REG 0x09
#define CHAN1_OT_LMT_REG 0x0a
#define CHAN2_OT_LMT_REG 0x0b
#define CHAN1_THERM_LMT_REG 0x0c
#define CHAN2_THERM_LMT_REG 0x0d
#define FAN1_CONFIG1_REG 0x10
#define FAN1_CONFIG2A_REG 0x11
#define FAN1_CONFIG2B_REG 0x12
#define FAN1_CONFIG3_REG 0x13
#define FAN2_CONFIG1_REG 0x14
#define FAN2_CONFIG2A_REG 0x15
#define FAN2_CONFIG2B_REG 0x16
#define FAN2_CONFIG3_REG 0x17
#define FAN1_TACH_R_REG 0x20
#define FAN2_TACH_R_REG 0x21
#define FAN1_TACH_CONFIG_REG 0x22
#define FAN2_TACH_CONFIG_REG 0x23
#define FAN1_PULSE_REG 0x24
#define FAN2_PULSE_REG 0x25
#define FAN1_CYCLE_REG 0x26
#define FAN2_CYCLE_REG 0x27
#define CHAN1_MIN_TEMP_REG 0x28
#define CHAN2_MIN_TEMP_REG 0x29
#define DEVICE_REG 0x3d
#define MANUFACTURE_REG 0x3e
#define DEV_VER_REG 0x3f

#define TEMPQUEMAXNUM 50
#define MAXFANNUM 6
#define CHAN1 1
#define CHAN2 2
#define SLOTOFFSET 15

#define CHIP_CLK_FRE 1000
#define HI_THRESHOLD 50
#define LO_THRESHOLD -10
#define NOR_RANGE 20
#define TEMP_GPIO 25
#define TEMP_DEBUG(x) {if(temp_debug) sys_console_printf(x);}

#define FANID2SLOTID(x,y) \
do{\
	if (SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900 || SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)\
	{\
	 if((x)==2||(x)==1)\
	    (y) = 15;\
	 else if( (x)==4 || (x) == 3 )\
	    (y) = 16;\
	 else if( (x)==6 || (x) == 5 )\
	    (y) = 17;\
	}\
	else if (SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900M)\
	{\
	 if((x)==2||(x)==1)\
	    (y) = 4;\
	}\
	else if ((SYS_PRODUCT_TYPE==PRODUCT_E_GFA6900S) || (SYS_PRODUCT_TYPE==PRODUCT_E_GFA8100))\
	{\
	    (y) = 2;\
	}\
}while(0);

#define FANID2TACHREG(x,reg) \
do{\
	if(x%2)\
	    reg = FAN1_TACH_R_REG;\
	else\
		reg = FAN2_TACH_R_REG;\
}while(0);

#define FAN_6900_POLLING( i )\
	for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)\
		if( SYS_MODULE_IS_FAN(i) )

typedef enum fan_mode{
	MODE_AUTO,
	MODE_MANUAL,
	MODE_DISABLE
}FANMODE;

/* wangysh end 6900*/


extern void ReadCPLDReg( unsigned char * RegAddr, unsigned char * pucIntData );
extern LONG getLocalFirmwareVersion( char * version );
extern VOID (*check_environment_hook_rtn) (VOID);
extern STATUS getDeviceAlarmMask( ULONG devIdx, ULONG *mask );
STATUS validateParamIsFigure(CHAR *param);
extern int Read_Temperature( int gpio );
extern LONG devsm_board_set_temperature( ULONG slotno, LONG temp );
extern LONG devsm_board_get_temperature( ULONG slotno );
extern VOID devsm_fan_set_insert_fast(ULONG slotno);
int fan_ctrl_i2c_write(int device,unsigned char reg, unsigned char value);
void init_fan_config();
/* modified by xieshl 20121211, 问题单16144 */
ULONG FANID2SLOTNO( ULONG fan_id )
{
	ULONG slotno = 0;
	if( (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100) )
		slotno = SYS_MASTER_ACTIVE_SLOTNO;
	else
		FANID2SLOTID( fan_id, slotno );
	
	return slotno;
}
ULONG FANID2FANNO( ULONG fan_id )
{
	if( (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100) )
		return fan_id;
	else if( (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 || SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000) ||
		(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M) )
		return ( (fan_id % 2) ? 1 : 2 );
	else if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
		return fan_id;
	else 
		return fan_id;
}
ULONG FANSLOT2ID( ULONG slotno, ULONG fanno )
{
	ULONG fan_id = 0;
	switch( SYS_PRODUCT_TYPE )
	{
		case PRODUCT_E_EPON3:
		case PRODUCT_E_GFA6100:
			fan_id = fanno;
			break;
		case PRODUCT_E_GFA6900:
		case PRODUCT_E_GFA8000:
			if( (slotno >= 15) && (slotno <= 17) )
				fan_id = ((slotno-15) << 1) + fanno;
			break;
		case PRODUCT_E_GFA6900M:
		case PRODUCT_E_GFA6900S:
		case PRODUCT_E_GFA8000M:
		case PRODUCT_E_GFA8000S:
		case PRODUCT_E_GFA8100:
			fan_id = fanno;/*1;*/
			break;
		default:
			break;
	}
	return fan_id;
}


/* 取得风扇告警状态和转速 fanIdx:1~4,  fanAlarmStat为TRUE有告警，False无告警*/
STATUS getFanAlarmStat(UCHAR fanIdx, BOOL *fanAlarmStat, ULONG *rev)
{
	if ( 0 == fanIdx || fan_num_total < fanIdx )
	{
		/*sys_console_printf("getFanAlarmStat()::fanIdx=%d error\r\n", fanIdx);*/
		return VOS_ERROR;
	}

	/* 如果使能关闭，则不报告警 */
	if ( FAN_ALARM_DISABLE == fanAlarmEnable )
	{
		*fanAlarmStat = FALSE;
		*rev = 0;
		return VOS_OK;
	}

	if ( NULL != fanAlarmStat )
	{
		if ( fanAlarm[fanIdx] )
		{
			*fanAlarmStat = TRUE;
		} 
		else
		{
			*fanAlarmStat = FALSE;
		}
	}

	if ( NULL != rev )
	{
		*rev = fanMibRev[fanIdx];
	}

	return VOS_OK;
}

ULONG getFanAlarmMinRev()
{
	return fanMinRev;
}

STATUS setFanAlarmMinRev(ULONG rev)
{
	if( (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900) || (SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900M) || 
		(SYS_PRODUCT_TYPE ==  PRODUCT_E_GFA6900S) || SYS_PRODUCT_TYPE ==   PRODUCT_E_GFA8000)
	{
		if( rev < 0 )
			return VOS_ERROR;
	}
	else
	{
		if ( rev < 20 || rev > 120)
		{
			/*sys_console_printf("setFanAlarmMinRev()::rev=%ld error\r\n", rev);*/
			return VOS_ERROR;
		}
	}
	fanMinRev = rev;
	return VOS_OK;
}

int getFanAlarmEnable()
{
	return fanAlarmEnable;
}

STATUS setFanAlarmEnable(UCHAR enable)
{
	char fw[8] = {0};

	if ( FAN_ALARM_ENABLE != enable && FAN_ALARM_DISABLE != enable )
	{
		return VOS_ERROR;
	}
	if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
	{
		getLocalCPLDVersion(fw);

		if( fw[3] < '5' )
		{
			fanAlarmEnable = FAN_ALARM_DISABLE;
			return VOS_ERROR;
		}
	}

	fanAlarmEnable = enable;
	return VOS_OK;
}

int getTemperatureAlarmEnable()
{
	return temperatureAlarmEnable;
}

STATUS setTemperatureAlarmEnable(UCHAR enable)
{
	if ( TEMPERATURE_ALARM_DISABLE != enable && TEMPERATURE_ALARM_ENABLE != enable )
	{
		return VOS_ERROR;
	}

	temperatureAlarmEnable = enable;
	return VOS_OK;
}

LONG getTemperatureAlarmValueHigh()
{
	return temperatureAlarmValueHigh;
}

LONG getTemperatureAverage()
{
	return temperatureAverage;
}

LONG getTemperatureAlarmValueLow()
{
	return temperatureAlarmValueLow;
}

STATUS setTemperatureAlarmValue(struct vty *vty, LONG low, LONG high)
{
	if (low < MIN_ALARM_TEMPERATURE || low > MAX_ALARM_TEMPERATURE || high < MIN_ALARM_TEMPERATURE || high > MAX_ALARM_TEMPERATURE)
	{
		vty_out( vty, "the value been input is out of range, low=%d, high=%d\r\n", low, high);
		return VOS_ERROR;
	}

	if (low >= high)
	{
		vty_out( vty, "the low value is larger than high value, low=%d, high=%d\r\n", low, high);
		return VOS_ERROR;
	}

	temperatureAlarmValueLow = low;
	temperatureAlarmValueHigh = high;
	/*这里还应该加上对风扇控制芯片的设置*/
	/*add by zhaozhig */
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 ||
		SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M || SYS_PRODUCT_TYPE ==   PRODUCT_E_GFA8000) )
	{
		ULONG i = 0;
		FAN_6900_POLLING(i)
		{
            /*added by luh 2-14-01-13*/
        	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
        	{
        		if (i == 4)
        		{
        			i = 15;
        		}
        	}
            
			fan_ctrl_i2c_write (i,0xc,temperatureAlarmValueHigh);
			fan_ctrl_i2c_write (i,0xd,temperatureAlarmValueHigh);
		}
	}
	return VOS_OK;
}

STATUS setTemperatureAlarmLowValue( LONG low )
{
	if (low < MIN_ALARM_TEMPERATURE || low > MAX_ALARM_TEMPERATURE)
	{
		/*sys_console_printf("the value been input is out of range, low=%d\r\n", low);*/
		return VOS_ERROR;
	}

	if (low >= temperatureAlarmValueHigh)
	{

		/*sys_console_printf("the low value is larger than high value, low=%d, temperatureAlarmValueHigh=%d\r\n", low, temperatureAlarmValueHigh);*/
		return VOS_ERROR;
	}

	temperatureAlarmValueLow = low;
	return VOS_OK;
}

STATUS setTemperatureAlarmHighValue( LONG high )
{
	if (high < MIN_ALARM_TEMPERATURE || high > MAX_ALARM_TEMPERATURE)
	{
		/*sys_console_printf("the value been input is out of range, high=%d\r\n", high);*/
		return VOS_ERROR;
	}

	if (temperatureAlarmValueLow >= high)
	{
		/*sys_console_printf("the low value is larger than high value, high=%d, temperatureAlarmValueLow=%d\r\n", high, temperatureAlarmValueLow);*/
		return VOS_ERROR;
	}

	temperatureAlarmValueHigh = high;
	return VOS_OK;
}



#define FAN_RECORD             2 /* 存放记录的次数 */
#define FAN_TOTAL              3 /* 6100风扇个数 */
/*6100 cpld reg addr*/
#define FAN_1_REG_HIGH_ADDR    0x10
#define FAN_1_REG_LOW_ADDR     0x11
#define FAN_2_REG_HIGH_ADDR    0x12
#define FAN_2_REG_LOW_ADDR     0x13
#define FAN_3_REG_HIGH_ADDR    0x15
#define FAN_3_REG_LOW_ADDR     0x16
/*6700 cpld reg addr*/
#define FAN_VALID_ADDR    0x16
#define FAN_ALARM_ADDR    0x0B
/* wangysh add 6900*/
#define FAN_6900M_ALARM_ADDR 0x08
#define FAN_8100_ALARM_ADDR 0x07
#define FAN_8100_16GP_ALARM_ADDR 0x2f
int fan_ctrl_i2c_read(int device,unsigned char reg, unsigned char* value)
{
	return i2c_data_get(device,I2C_BASE_FAN,reg,value,I2C_RW_LEN);
}

int fan_ctrl_i2c_write(int device,unsigned char reg, unsigned char value)
{
	return i2c_data_set(device,I2C_BASE_FAN,reg,&value,I2C_RW_LEN);
}

ULONG read_rpm( ULONG i,ULONG * rpm)
{
	ULONG j,reg;
	UCHAR val;
	
	FANID2SLOTID(i,j)
	FANID2TACHREG(i,reg)
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (j == 4)
		{
			j = 15;
		}
	}
	fan_ctrl_i2c_read(j, reg, &val);
	
	if(val==0xff)
	    *rpm=0;	
	else if( val != 0 )
		*rpm = 4000*60/val;
	else
		*rpm = 3000;
	return VOS_OK;
}
VOID read_alarm_status(ULONG i)
{
	ULONG j;
	UCHAR val;
	FANID2SLOTID(i,j)
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (j == 4)
		{
			j = 15;
		}
	}
	fan_ctrl_i2c_read(j,0x2,&val);
	/* fan fail alm*/
	if(i%2)
	{
		if( val&2 )
			fanfail_alm[i]=VOS_YES;
		else
			fanfail_alm[i]=VOS_NO;
	}
	else
	{
		if( val&1 )
			fanfail_alm[i]=VOS_YES;
		else
			fanfail_alm[i]=VOS_NO;
	}
#if 0
	/* ot alm*/
	if(val && 1<<4 )
	{
		ot_alm[i]=VOS_YES;
	}
#endif
	return;
}
/*判断风扇转速函数 index:0-2 or 3; rev:风扇每秒转速输出*/
static STATUS checkFanRev(UCHAR index, ULONG *revPtr)
{
	UCHAR  val;
	ULONG revValue = 0;
	static ULONG rev[FAN_TOTAL + 1] = {0};
	static UCHAR count = 0, isValid = 0, fanAlarmStat[FAN_TOTAL + 2] = {0}/* 1告警，0无告警 */;

	if (index > (UCHAR)fan_num_total)
	{
		/*sys_console_printf("checkFanRev()::index=%x error!\r\n", index);*/
		return VOS_ERROR;
	}

	if (NULL == revPtr)
	{
		/*sys_console_printf("checkFanRev()::rev=NULL!\r\n", index);*/
		return VOS_ERROR;
	}

	if ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
	{
			count++;
			revValue = 0;

			if (fanDebug)
			{
				sys_console_printf("CPLD REG 6100 Fan Index:%x\r\n", index);
			}

			switch(index)
			{
			case 1:
				ReadCPLDReg( (unsigned char *)FAN_1_REG_HIGH_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_1_REG_HIGH_ADDR, val);
				}

				revValue |= (ULONG)( ( val & 0x1F ) << 4 );/* 取低5位 */
				ReadCPLDReg( (unsigned char *)FAN_1_REG_LOW_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_1_REG_LOW_ADDR, val);
				}

				revValue |= (ULONG)( val & 0x0F );/* 取低4位 */
				break;
			case 2:
				ReadCPLDReg( (unsigned char *)FAN_2_REG_HIGH_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_2_REG_HIGH_ADDR, val);
				}

				revValue |= (ULONG)( ( val & 0x1F ) << 4 );/* 取低5位 */
				ReadCPLDReg( (unsigned char *)FAN_2_REG_LOW_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_2_REG_LOW_ADDR, val);
				}

				revValue |= (ULONG)( val & 0x0F );/* 取低4位 */
				break;
			case 3:
				ReadCPLDReg( (unsigned char *)FAN_3_REG_HIGH_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_3_REG_HIGH_ADDR, val);
				}

				revValue |= (ULONG)( ( val & 0x1F ) << 4 );/* 取低5位 */
				ReadCPLDReg( (unsigned char *)FAN_3_REG_LOW_ADDR, &val );

				if (fanDebug)
				{
					sys_console_printf("CPLD REG address: 0x%02x, value:0x%02x\r\n", FAN_3_REG_LOW_ADDR, val);
				}

				revValue |= (ULONG)( val & 0x0F );/* 取低4位 */
			    break;
			default:
				return VOS_ERROR;
			}

			if (count < 3)
			{
				rev[index] = revValue;
				return VOS_ERROR;
			}
			else
			{
				rev[index] = rev[index] > revValue ? rev[index] : revValue;/* 取2个中较大的值 */
				*revPtr = rev[index] / 2;/* 脉冲数转换成每秒转速 */

				if (count == 5)
				{
					if (fanDebug)
					{
						sys_console_printf("6100 alarm per second impulse to be set: %d\r\n", fanMinRev * 2);

						for (val = 0; val < fan_num_total; val++)
						{
							sys_console_printf("6100 Fan Index:%x, Rev impulse:%d\r\n", val, rev[val]);
						}
						sys_console_printf("\r\n");
					}

					count = 0;
				}
			}
	} 
	else if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		/*if (fanDebug)
		{
			sys_console_printf("6700 count=0x%02x\r\n", count);
		}*/

		/* 判断风扇板是否在位有效 */
		if (count < 2)
		{
			count++;

			ReadCPLDReg( (unsigned char *)FAN_VALID_ADDR, &val );

			if (val & 0x08)
			{
				if (fanDebug)
				{
					sys_console_printf("6700 fan cpld valid reg: 0x%02x, isValid=0x%02x\r\n", val, isValid);
				}

				isValid++;
			}

			return VOS_ERROR;
		}

		if (isValid == 0)
		{
			/* 风扇板是否在位无效 */
			count = 0;
			*revPtr = 0;/* 有告警，转速为每秒0转 */
			return VOS_OK;
		}

		ReadCPLDReg( (unsigned char *)FAN_ALARM_ADDR, &val );

		if (fanDebug)
		{
			sys_console_printf("6700 fan cpld alarm reg: 0x%02x\r\n", val);
		}

		if ( val & ( 0x01 << (index + 3) ) )/* 无告警，转速为每秒120转 */
		{
			*revPtr = 120;
			fanAlarmStat[index] = 0;
		} 
		else
		{
			*revPtr = 0;/* 有告警，转速为每秒0转 */
			fanAlarmStat[index] = 1;

			/* 重新判断板在位寄存器 */
			if ( fanAlarmStat[1] && fanAlarmStat[2] && fanAlarmStat[3] && fanAlarmStat[4] )
			{
				count = 0;
				isValid = 0;

				for (val = 1; val <= fan_num_total; val++)
				{
					fanAlarmStat[val] = 0;
				}
			}
		}
	}
	else if( PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE) || PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE) )
	{
		ULONG rpm=0;
#if 1
		read_rpm(index,&rpm);
		read_alarm_status(index);
#endif
		*revPtr = rpm;
	}
	else if( PRODUCT_IS_HS_Series(SYS_PRODUCT_TYPE) )
	{
		UCHAR rpm=0;
		UCHAR val = 0;
		ULONG add = 0;

		if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100)
		{
			/*风扇暂时没有异常*/
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			{
				ReadCPLDReg( (unsigned char *)FAN_8100_ALARM_ADDR, &val );
				if(val&(0x1<<(index-1)))
				{
					*revPtr = 9600;
					fanfail_alm[index] = VOS_NO;
				}
				else
				{
					*revPtr = 0;
					fanfail_alm[index] = VOS_YES;
				}
			}
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{
				ReadCPLDReg( (unsigned char *)(0x200000+FAN_8100_16GP_ALARM_ADDR), &val );
				if(val&(0x1<<(index-1+4)))
				{
					*revPtr = 9000;
					fanfail_alm[index] = VOS_NO;
				}
				else
				{
					*revPtr = 0;
					fanfail_alm[index] = VOS_YES;
				}
			}

		}
		else
		{
			/*没有风扇控制芯片，如果转，转速为120*/
			/*先读cpld风扇是否有异常*/
			ReadCPLDReg( (unsigned char *)FAN_6900M_ALARM_ADDR, &val );
			if (1&(val>>5))
			{
				/*没有异常*/
				*revPtr = 120;
				fanfail_alm[index] = VOS_NO;
			}
			else
			{
				i2c_data_get(15,I2C_BASE_GPIO+index-1,0,&rpm,I2C_RW_LEN);
				if ( 0 == rpm)
				{
					*revPtr = 120;
					fanfail_alm[index] = VOS_NO;
				}
				else
				{
					*revPtr = 0;
					fanfail_alm[index] = VOS_YES;
				}
			}
		}
		
	}

	return VOS_OK;
}

/* begin: added by jianght 20090803 */
/* 风扇告警被屏蔽了，返回true；没被屏蔽，返回false */
BOOL checkFanAlarmMask(void)
{
	ulong_t mask = 0;

	if( getDeviceAlarmMask( 1, &mask ) == VOS_OK )
	{
		if (mask & EVENT_MASK_DEV_FAN)
		{
			return TRUE;/* 屏蔽了 */
		} 
		else
		{
			return FALSE;/* 没屏蔽 */
		}
	}

	return FALSE;/* 异常情况，则认为没被屏蔽 */
}
/* end: added by jianght 20090803 */

STATUS fanCheck()
{
	ULONG rev;
	UCHAR index;
	ULONG slot;
	
 	if( !SYS_LOCAL_MODULE_ISMASTERACTIVE )
		return VOS_OK;
	if(!fan_num_total)
		return VOS_OK;

	if (FAN_ALARM_DISABLE == fanAlarmEnable)
	{
		/*disable后，清除已经存在的告警*/
		/* 若原来有告警，则清除告警 */
		for (index = 1; index <= (UCHAR)fan_num_total; index++)
		{
			fanMibRev[index] = 0;

			if ( fanAlarm[index] )
			{
				if ( VOS_ERROR == devFanAlarmClear_EventReport( OLT_DEV_ID, index) )
				{
					/*sys_console_printf("fanCheck()::devFanAlarm_EventReport()  Failed !!! \r\n");*/
					return VOS_ERROR;
				}

				fanAlarm[index] = 0;
			}
		}

		return VOS_OK;
	}

	for (index = 1; index <= (UCHAR)fan_num_total; index++)
	{
		if( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
		{
			FANID2SLOTID(index, slot)
			if(!SYS_MODULE_IS_FAN(slot) )
			{
				if ( fanAlarm[index] )
				{
					devFanAlarmClear_EventReport( OLT_DEV_ID, index);
					fanAlarm[index] = 0;
					fanMibRev[index] = 0;
				}
				continue;
			}
		}
		
		if ( VOS_OK == checkFanRev(index, &rev) )
		{
			fanMibRev[index] = rev;/* 提供给网管的转速值 */

			if ( !checkFanAlarmMask() && ( fanAlarm[index] == 0 ) && ((fanfail_alm[index])||(rev < fanMinRev)) )/* 发出告警 */
			{
				fanAlarm[index] = 1;

				if ( VOS_ERROR == devFanAlarm_EventReport( OLT_DEV_ID, index) )
				{
					/*sys_console_printf("fanCheck()::devFanAlarm_EventReport()  Failed !!! \r\n");*/
					return VOS_ERROR;
				}
			}
			else if ( ( ( fanAlarm[index] != 0 ) && ( VOS_NO == fanfail_alm[index] )&&(rev >= fanMinRev) ) || (checkFanAlarmMask() && fanAlarm[index] != 0) )/* 清除告警 */
			{
				fanAlarm[index] = 0;

				if ( VOS_ERROR == devFanAlarmClear_EventReport( OLT_DEV_ID, index) )
				{
					/*sys_console_printf("fanCheck()::devFanAlarm_EventReport()  Failed !!! \r\n");*/
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}


#define TEMPERATURE_SWING	100 /* 相邻两次温度取值不超过100 */
#define MAXNUM              3/* 取10次温度的平均值 */

/* begin: added by jianght 20090803 */
/* 温度告警被屏蔽了，返回true；没被屏蔽，返回false */
BOOL checkTemperatureAlarmMask(void)
{
	ulong_t mask = 0;

	if( getDeviceAlarmMask( 1, &mask ) == VOS_OK )
	{
		if (mask & EVENT_MASK_DEV_TEMPERATURE)
		{
			return TRUE;/* 屏蔽了 */
		} 
		else
		{
			return FALSE;/* 没屏蔽 */
		}
	}

	return FALSE;/* 异常情况，则认为没被屏蔽 */
}
/* end: added by jianght 20090803 */

/* add by wangysh 6900*/


int read_temp( int slot,int channel,LONG * temp )
{
	UCHAR i/*,j*/,reg_1,reg_2;

	if( temp == NULL)
		return VOS_ERROR;
	switch(channel)
	{
		case CHAN1:
			reg_1 = TEMP_CHAN1_REG;
		    reg_2 = TEMP_CHAN1_EXT_REG;
		break;
		case CHAN2:
			reg_1 = TEMP_CHAN2_REG;
		    reg_2 = TEMP_CHAN2_EXT_REG;
		break;
		default:
		break;
	}
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (4 == slot)
		{
			slot = 15;
		}
	}
	if ((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)||(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
	{
		/* 没有温控芯片*/
		*temp = 0;
		return VOS_OK;
	}
	
	if(VOS_NO == fan_ctrl_i2c_read(slot,reg_1,&i))
		return VOS_ERROR;
	/*if(VOS_NO == fan_ctrl_i2c_read(slot,reg_2,&j))
		return VOS_ERROR;
	*temp = i+(j>>5)*0.125;*/	/* modified by xieshl 20121016, 只取整数部分就够了*/
	*temp = i;
	return VOS_OK;
}

/* wangysh add  temperature collection 20110106*/
/*LONG board_temp_set(LONG slotno, FLOAT temp )
{
	if( SYS_SLOTNO_IS_ILLEGAL(slotno) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	devsm_module[slotno].temperature = temp;
	return VOS_OK;
}
FLOAT board_temp_get(LONG slotno )
{
	if( SYS_SLOTNO_IS_ILLEGAL(slotno) )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	return devsm_module[slotno].temperature;
}*/	/* moved to devsm_aux */

LONG fan_temp()
{
	int i;
	LONG temp=0;
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		return VOS_OK;
	FAN_6900_POLLING(i)
	{
		if( VOS_ERROR == read_temp(i,CHAN2,&temp) )
			return VOS_ERROR;
		devsm_board_set_temperature(i,temp);
	}
	return VOS_OK;
}
LONG temperature()
{
	LONG temp;
	UCHAR tem_val;
	LONG rv = VOS_OK;
	if( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
	{
		fan_temp();
		temp = Read_Temperature(temperature_gpio);
		/*wangysh 记录交换板温度2011-01-06*/
		/*devsm_board_set_temperature(SYS_LOCAL_MODULE_SLOTNO,temp);*/	/* removed by xieshl 20111121, 为防止显示结果跟environment不一致，应统一用temperatureAverage，问题单13919 */
		if(SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
		{
			rv = i2c_data_get(SYS_LOCAL_MODULE_SLOTNO, 0xF0, 0x00, &tem_val, 1);
			if(1 == rv)
			{
				temp = tem_val;
			}
		}
	}
	else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)||(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3))
	{
		temp = Read_Temperature(temperature_gpio);
	}
	return temp;
}

/* modified by xieshl 20121016, 读本板温度和温度告警检查分开，slave板上只负责读，告警统一由主控
    (master active)检查，问题单15596 */
#if 0
STATUS temperatureCheck()
{
	static UCHAR alarmHigh = 0, alarmLow = 0;/* 记录是否已经产生告警 0无告警，1有告警 */
	static LONG preValue = 0xFFFFFFFF, sum = 0;
	static ULONG count = 0;
	LONG value;

	if (TEMPERATURE_ALARM_DISABLE == temperatureAlarmEnable)
	{
	    temperatureAverage = 0;/* 使能关闭时为0 */
		devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */
	    preValue = 0xFFFFFFFF;
		sum = 0;
		count = 0;
		   
		/* 若disable后，存在告警，则清除 */
		if (1 == alarmHigh)
		{
			/* 低于温度上限，告警恢复 */
			if (VOS_ERROR == temperature_high_alarm(FALSE, temperatureAverage))
			{
				return VOS_ERROR;
			}

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %ld  and high clear alarm\r\n", temperatureAverage);
			}*/

			alarmHigh = 0;
		}

		if (1 == alarmLow)
		{
			/* 高于温度下线，告警恢复 */
			if(VOS_ERROR==temperature_low_alarm(FALSE, temperatureAverage))
			{
				return VOS_ERROR;
			}

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %ld  and low clear alarm\r\n", temperatureAverage);
			}*/

			alarmLow = 0;
		}

		return VOS_OK;
	}

	if (0xFFFFFFFF == preValue)
	{
		value = temperature();

		if (temperatureDebug)
		{
			sys_console_printf("@the first raw temperature is %ld \r\n", value);
		}

		if ( value > MAX_ALARM_TEMPERATURE || value < MIN_ALARM_TEMPERATURE )/* 初始温度若过大，则放弃此次温度值 */
		{
			return VOS_OK;
		}

		if (0 == count)
		{
			sum = value;/*sum放最小值*/
		} 
		else if (count > 0)
		{
			sum = sum < value ? sum : value;
		}

		count++;

		if (count >= MAXNUM)
		{
			preValue = sum /*/ MAXNUM*/;/* 取得初始前次温度值 */
			temperatureAverage = preValue;
			devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */

			if (temperatureDebug)
			{
				sys_console_printf("the first avr temperature is %ld , first sum is %ld, the count is %ld\r\n", temperatureAverage, sum, count);
			}

	            sum = 0;
	            count = 0;
		}

		return VOS_OK;
	}

	value = temperature();
	/*需判断温度告警管脚*/

	if (temperatureDebug)
	{
		sys_console_printf("the raw temperature is %ld \r\n", value);
	}

	if ( value > MAX_ALARM_TEMPERATURE || value < MIN_ALARM_TEMPERATURE )/* 原始温度若过大，则放弃此次温度值 */
	{
		return VOS_OK;
	}

	count++;

	if (value > preValue)
	{
		/* 变化太大的采样，用上次的值 */
		if ( (value - preValue) > TEMPERATURE_SWING )
		{
			sum += preValue;
		}
		else
		{
			sum += value;
			preValue = value;
		}
	}
	else if (value < preValue)
	{
		if ( (preValue - value) > TEMPERATURE_SWING )
		{
			sum += preValue;
		} 
		else
		{
			sum += value;
			preValue = value;
		}
	}
	else
	{
		sum += value;
	}

	if (count >= MAXNUM)
	{
		value    = sum / MAXNUM;
		preValue = value;
		sum      = 0;
		count    = 0;
		temperatureAverage = value;
		devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */

		if (temperatureDebug)
		{
			sys_console_printf("the avr temperature is %ld \r\n", temperatureAverage);
		}

		/* begin: added by jianght 20090803 */
		if (checkTemperatureAlarmMask())
		{
			if (1 == alarmHigh)
			{
				/* 低于温度上限，告警恢复 */
				if (VOS_ERROR == temperature_high_alarm(FALSE, value))
				{
					return VOS_ERROR;
				}
				alarmHigh = 0;
			}

			if (1 == alarmLow)
			{
				/* 高于温度下线，告警恢复 */
				if(VOS_ERROR==temperature_low_alarm(FALSE, value))
				{
					return VOS_ERROR;
				}
				alarmLow = 0;
			}

			return VOS_OK;
		}
		/* end: added by jianght 20090803 */

		if (value > temperatureAlarmValueHigh && 0 == alarmHigh)
		{
			/* 之前有低温告警，则先清除 */
			if (alarmLow)
			{
				/* 高于温度下线，告警恢复 */
				if(VOS_ERROR == temperature_low_alarm(FALSE, value))
				{
				return VOS_ERROR;
				}

				alarmLow = 0;
			}

			/* 高于温度上限，产生告警 */
			if ( !checkTemperatureAlarmMask() && VOS_ERROR == temperature_high_alarm(TRUE, value) )
			{
				return VOS_ERROR;
			}

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %d  and send high alarm\r\n", value);
			}*/

			alarmHigh = 1;
		}
		else if (value <= temperatureAlarmValueHigh && 1 == alarmHigh)
		{
			/* 低于温度上限，告警恢复*/
			if (VOS_ERROR == temperature_high_alarm(FALSE, value))
			{
				return VOS_ERROR;
			} 

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %d  and high clear alarm\r\n", value);
			}*/

			alarmHigh = 0;
		}

		if (value < temperatureAlarmValueLow && 0 == alarmLow)
		{
			/* 之前有高温告警，则先清除 */
			if (alarmHigh)
			{
				/* 低于温度上限，告警恢复*/
				if (VOS_ERROR == temperature_high_alarm(FALSE, value))
				{
				return VOS_ERROR;
				} 

				alarmHigh = 0;
			}

			/* 低于温度下线，产生告警 */
			if( !checkTemperatureAlarmMask() && (VOS_ERROR==temperature_low_alarm(TRUE, value)))
			{
				return VOS_ERROR;
			}

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %d  and send low alarm\r\n", value);
			}*/

			alarmLow = 1;
		}
		else if (value >= temperatureAlarmValueLow && 1 == alarmLow)
		{
			/* 高于温度下线，告警恢复 */
			if(VOS_ERROR==temperature_low_alarm(FALSE, value))
			{
				return VOS_ERROR;
			}

			/*if (temperatureDebug)
			{
				sys_console_printf("the temperature is %d  and low clear alarm\r\n", value);
			}*/

			alarmLow = 0;
		}
	}

	return VOS_OK;
}
#else
LONG temperature_high_alarm( ULONG slotno, BOOL alarm_flag, LONG temp_val )
{
	LONG rc = VOS_ERROR;
	if( temperatureAlarmMode == TEMPERATURE_ALARM_MODE_DISTRIBUTION )
	{
		if( alarm_flag )
			rc = boardTemperatureHigh_EventReport( OLT_DEV_ID, slotno, temp_val );
		else
			rc = boardTemperatureHighClear_EventReport( OLT_DEV_ID, slotno, temp_val );
	}
	else
	{
		if( alarm_flag )
			rc = deviceTemperatureHigh_EventReport( OLT_DEV_ID, temp_val, temperatureAlarmValueHigh );
		else
			rc = deviceTemperatureHighClear_EventReport( OLT_DEV_ID, temp_val, temperatureAlarmValueHigh );
	}
	return rc;
}
LONG temperature_low_alarm( ULONG slotno, BOOL alarm_flag, LONG temp_val )
{
	LONG rc = VOS_ERROR;
	if( temperatureAlarmMode == TEMPERATURE_ALARM_MODE_DISTRIBUTION )
	{
		if( alarm_flag )
			rc = boardTemperatureHigh_EventReport( OLT_DEV_ID, slotno, temp_val );
		else
			rc = boardTemperatureHighClear_EventReport( OLT_DEV_ID, slotno, temp_val );
	}
	else
	{
		if( alarm_flag )
			rc = deviceTemperatureLow_EventReport( OLT_DEV_ID, temp_val, temperatureAlarmValueLow );
		else
			rc = deviceTemperatureLowClear_EventReport( OLT_DEV_ID, temp_val, temperatureAlarmValueLow );
	}
	return rc;
}

/* 读本板温度 */
STATUS temperatureCheck()
{
	static LONG preValue = 0xFFFFFFFF, sum = 0;
	static ULONG count = 0;
	LONG value;

	if (TEMPERATURE_ALARM_DISABLE == temperatureAlarmEnable)
	{
		temperatureAverage = 0;/* 使能关闭时为0 */
		/*devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );*/	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */
		devsm_module[SYS_LOCAL_MODULE_SLOTNO].temperature = temperatureAverage;
		preValue = 0xFFFFFFFF;
		sum = 0;
		count = 0;

		return VOS_OK;
	}

	if (0xFFFFFFFF == preValue)
	{
		value = temperature();

		if (temperatureDebug)
		{
			sys_console_printf("@the first raw temperature is %ld \r\n", value);
		}

		if ( value > MAX_ALARM_TEMPERATURE || value < MIN_ALARM_TEMPERATURE )/* 初始温度若过大，则放弃此次温度值 */
		{
			return VOS_OK;
		}

		if (0 == count)
		{
			sum = value;/*sum放最小值*/
		} 
		else if (count > 0)
		{
			sum = sum < value ? sum : value;
		}

		count++;

		if (count >= MAXNUM)
		{
			preValue = sum /*/ MAXNUM*/;/* 取得初始前次温度值 */
			temperatureAverage = preValue;
			/*devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );*/	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */
			devsm_module[SYS_LOCAL_MODULE_SLOTNO].temperature = temperatureAverage;

			if (temperatureDebug)
			{
				sys_console_printf("the first avr temperature is %ld , first sum is %ld, the count is %ld\r\n", temperatureAverage, sum, count);
			}

	            sum = 0;
	            count = 0;
		}

		return VOS_OK;
	}

	value = temperature();
	/*需判断温度告警管脚*/

	if (temperatureDebug)
	{
		sys_console_printf("the raw temperature is %ld \r\n", value);
	}

	if ( value > MAX_ALARM_TEMPERATURE || value < MIN_ALARM_TEMPERATURE )/* 原始温度若过大，则放弃此次温度值 */
	{
		return VOS_OK;
	}

	count++;

	if (value > preValue)
	{
		/* 变化太大的采样，用上次的值 */
		if ( (value - preValue) > TEMPERATURE_SWING )
		{
			sum += preValue;
		}
		else
		{
			sum += value;
			preValue = value;
		}
	}
	else if (value < preValue)
	{
		if ( (preValue - value) > TEMPERATURE_SWING )
		{
			sum += preValue;
		} 
		else
		{
			sum += value;
			preValue = value;
		}
	}
	else
	{
		sum += value;
	}

	if (count >= MAXNUM)
	{
		value    = sum / MAXNUM;
		preValue = value;
		sum      = 0;
		count    = 0;
		temperatureAverage = value;
		devsm_board_set_temperature( SYS_LOCAL_MODULE_SLOTNO, temperatureAverage );	/* modifieded by xieshl 20111121, 6700无法显示问题，问题单13919 */

		if (temperatureDebug)
		{
			sys_console_printf("the avr temperature is %ld \r\n", temperatureAverage);
		}
	}

	return VOS_OK;
}

/* 温度告警检查 */
LONG temperatureAlarmCheck( ULONG slotno, LONG value )
{
	static ULONG alarmHigh = 0, alarmLow = 0;/* 记录是否已经产生告警 0无告警，1有告警 */
	ULONG alm_flag = 0;
	LONG rc = VOS_OK;

	if( SYS_SLOTNO_IS_ILLEGAL(slotno) )
		return VOS_ERROR;
	if( checkTemperatureAlarmMask() )
		return rc;
	
	alm_flag = (1<<(slotno-1));

	/* 若disable后，存在告警，则清除 */
	if( (TEMPERATURE_ALARM_DISABLE == temperatureAlarmEnable) && (!checkTemperatureAlarmMask()) )
	{
		if( alarmHigh & alm_flag )
		{
			/* 低于温度上限，告警恢复 */
			rc = temperature_high_alarm( slotno, FALSE, temperatureAverage);
			alarmHigh &= (~alm_flag);
		}

		if( alarmLow & alm_flag )
		{
			/* 高于温度下线，告警恢复 */
			rc = temperature_low_alarm(slotno, FALSE, temperatureAverage);
			alarmLow &= (~alm_flag);
		}
		return rc;
	}

	/* begin: added by jianght 20090803 */
	if( checkTemperatureAlarmMask() )
	{
		if( alarmHigh & alm_flag )
		{
			/* 低于温度上限，告警恢复 */
			rc = temperature_high_alarm(slotno, FALSE, value);
			alarmHigh &= (~alm_flag);
		}

		if( alarmLow & alm_flag )
		{
			/* 高于温度下线，告警恢复 */
			rc = temperature_low_alarm(slotno, FALSE, value);
			alarmLow &= (~alm_flag);
		}
		return rc;
	}
	/* end: added by jianght 20090803 */

	if( (devsm_module[slotno].temperature == value) && (value == 0) )
		return rc;

	if( (value > temperatureAlarmValueHigh) && (0 == (alarmHigh & alm_flag)) )
	{
		/* 之前有低温告警，则先清除 */
		if( alarmLow & alm_flag )
		{
			/* 高于温度下线，告警恢复 */
			rc = temperature_low_alarm(slotno, FALSE, value);
			alarmLow &= (~alm_flag);
		}

		/* 高于温度上限，产生告警 */
		rc = temperature_high_alarm(slotno, TRUE, value);
		alarmHigh |= alm_flag;
	}
	else if( (value < temperatureAlarmValueHigh) && (alarmHigh & alm_flag) )
	{
		/* 低于温度上限，告警恢复*/
		rc = temperature_high_alarm(slotno, FALSE, value);
		alarmHigh &= (~alm_flag);
	}

	if( (value < temperatureAlarmValueLow) && (0 == (alarmLow & alm_flag)) )
	{
		/* 之前有高温告警，则先清除 */
		if( alarmHigh & alm_flag )
		{
			/* 低于温度上限，告警恢复*/
			rc = temperature_high_alarm(slotno, FALSE, value);
			alarmHigh &= (~alm_flag);
		}

		/* 低于温度下线，产生告警 */
		rc = temperature_low_alarm(slotno, TRUE, value);
		alarmLow |= alm_flag;
	}
	else if( (value > temperatureAlarmValueLow) && (alarmLow & alm_flag) )
	{
		/* 高于温度下线，告警恢复 */
		rc = temperature_low_alarm(slotno, FALSE, value);
		alarmLow &= (~alm_flag);
	}

	return rc;
}
#endif

VOID environment_check_callback()
{
	int i;
	if( SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
	{
		temperatureCheck();
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			fanCheck();
			if( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
			{
				for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)
				{
					isPWUupdown(i);
				}
				PWU_Status_Timer_Check();  /*add for Pwu Stutas Alarm by @muqw 2017-4-6*/
			}
		}
	}
}

LONG temperature_vty_show(struct vty * vty,LONG slotno)
{
	LONG temp_1/*,temp_2,temp_3*/;
	if(SYS_SLOTNO_IS_ILLEGAL(slotno))
		return VOS_ERROR;
	temp_1 = devsm_board_get_temperature(slotno);
	/*temp_2 = temp_1*10;
	temp_3 = temp_2%10;*/
	if( temp_1 != 126 )
	{
		vty_out(vty,"  %-4d%-20s%d.%01d\r\n",slotno,
			typesdb_module_type2name( devsm_module[ slotno ].pub_info.module_type ),
			temp_1, /*temp_3*/0);
	}
	else
	{
		vty_out(vty,"  %-4d%-20s%s\r\n",slotno,
			typesdb_module_type2name( devsm_module[ slotno ].pub_info.module_type ),
			"-");
	}
	return VOS_OK;
}

DEFUN(show_all_board_temp,
		show_all_board_temp_cmd,
		"show temperature {<1-21>}*1",
		DescStringCommonShow
		"board temprature\n"
		"input the slotId\n"
)
{
	int slotno;
	if( argc == 1 )
	{
		slotno = VOS_AtoI(argv[0]);
		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE&&slotno != SYS_LOCAL_MODULE_SLOTNO)
		{
			vty_out(vty,"slave card can only check itself\r\n");
		}
		else
		{
			if( SYS_MODULE_SLOT_ISHAVECPU(slotno) || SYS_MODULE_IS_FAN(slotno))
			{
				vty_out(vty,"\r\nSlot  type                Temperature\r\n\r\n");
				temperature_vty_show(vty,slotno);
			}
			else
			{
				vty_out( vty, "there is no need to check temperature on card %d\r\n",slotno);
			}
		}
	}
	else
	{
		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
		{
			vty_out(vty,"\r\nSlot  type                Temperature\r\n\r\n");
			temperature_vty_show(vty,SYS_LOCAL_MODULE_SLOTNO);
		}
		else
		{
			/* zhangxinhui, 2011-02-10 moved from for loop to here */
			vty_out(vty,"\r\nSlot  type                Temperature\r\n\r\n");
			for(slotno=1; slotno<=SYS_CHASSIS_SLOTNUM; slotno++)
			{
				if( SYS_MODULE_SLOT_ISHAVECPU(slotno) || SYS_MODULE_IS_FAN(slotno) )
				{
					temperature_vty_show(vty,slotno);
				}
			}
		}
	}
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;
}
DEFUN  (
		show_temperature_monitor,
		show_temperature_monitor_cmd,
		"show environment-monitor",
		DescStringCommonShow
		"show temperature\n"
		)
{
	int i,j, ret = getTemperatureAlarmEnable();

	if (ret == TEMPERATURE_ALARM_ENABLE )
	{
		vty_out( vty, " temperature monitor: enable\r\n");

		vty_out( vty, " temperature value: %d\r\n", temperatureAverage);
		vty_out( vty, " alarm threshold high: %d\r\n", temperatureAlarmValueHigh);
		vty_out( vty, " alarm threshold low: %d\r\n", temperatureAlarmValueLow);
	} 
	else
	{
		vty_out( vty, " the temperature monitor is disable\r\n");
	}

	ret = getFanAlarmEnable();

	if (ret == FAN_ALARM_ENABLE)
	{
		vty_out( vty, " fan monitor: enable\r\n");
		vty_out( vty, " fan alarm threshold low: %d\r\n", fanMinRev);

		for ( i = 1; i <= fan_num_total; i++ )
		{
#if 0	/* modified by xieshl 20120229, 解决6700/6100不能显示风扇状态问题 */
			FANID2SLOTID(i,j);
			if(!SYS_MODULE_IS_6900_FAN(j))
				continue;
			if ( fanAlarm[i] )
			{
				vty_out( vty, "  fan %d/%d : %-10s  speed (RPM) : %-20d\r\n", FANID2SLOTNO(i), FANID2FANNO(i), "ABNORMAL", fanMibRev[i]/* 每秒的转速 */);
			} 
			else
			{
				vty_out( vty, "  fan %d/%d : %-10s  speed (RPM) : %-20d\r\n", FANID2SLOTNO(i), FANID2FANNO(i), "NORMAL", fanMibRev[i]/* 每秒的转速 */);
			}
#else
			if ( PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE) )
			{
				FANID2SLOTID(i,j);
				if( SYS_MODULE_IS_FAN(j) )
				{
					vty_out( vty, "  fan %d/%d : %-10s  speed (RPM) : %-20d\r\n", FANID2SLOTNO(i), FANID2FANNO(i), 
						(fanAlarm[i] ? "ABNORMAL" : "NORMAL"), fanMibRev[i]/* 每秒的转速 */);
				}
			}
			else if ( PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE) )
			{
				FANID2SLOTID(i,j);
				if( SYS_MODULE_IS_FAN(j) )
				{
					vty_out( vty, "  fan %d/%d : %-10s  speed (RPM) : %-20d\r\n", 4, FANID2FANNO(i), 
						(fanAlarm[i] ? "ABNORMAL" : "NORMAL"), fanMibRev[i]/* 每秒的转速 */);
				}
			}
			else if ( PRODUCT_IS_HS_Series(SYS_PRODUCT_TYPE) )
			{
				FANID2SLOTID(i,j);
				if( SYS_MODULE_IS_FAN(j) )
				{
					vty_out( vty, "  fan %d/%d : %-10s  speed (RPM) : %-20d\r\n", 2, i, 
						(fanAlarm[i] ? "ABNORMAL" : "NORMAL"), fanMibRev[i]/* 每秒的转速 */);
				}
			}
			else
			{
				vty_out( vty, "  fan%d:%s rev:%d\r\n", i, (fanAlarm[i] ? "abnormal" : "normal"), fanMibRev[i]/* 每秒的转速 */);
			}
#endif
		}
	} 
	else
	{
		vty_out( vty, " fan monitor is disable\r\n");
	}	

	return CMD_SUCCESS;
}

DEFUN  (
		set_temperature_alarm_threshold,
		set_temperature_alarm_threshold_cmd,
		"temperature alarm-threshold <high> {<low>}*1",
		"set alarm temperature\n"
		"set alarm temperature threshold\n"
		"Input the alarm low temperature -50~100 Celsius degree\n"
		"Input the alarm high temperature -50~100 Celsius degree\n"
		)
{
	int ret;

	/*if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)*/
	if(!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
		return CMD_SUCCESS;
	if ( TEMPERATURE_ALARM_ENABLE != getTemperatureAlarmEnable() )
	{
		vty_out( vty, "The temperature alarm is disable.Please enable temperature alarm first!\r\n");
		return CMD_WARNING;
	}
		
	if (1 == argc)
	{
		if ( VOS_OK != validateParamIsFigure(argv[0]) )
		{
			vty_out( vty, "command executing failed!\r\n");
			return CMD_WARNING;
		}

		ret = setTemperatureAlarmValue( vty, temperatureAlarmValueLow, VOS_AtoL(argv[0]) );
	}
	else
	{
		if ( ( VOS_OK != validateParamIsFigure(argv[0]) ) || ( VOS_OK != validateParamIsFigure(argv[1]) ) )
		{
			vty_out( vty, "command executing failed!\r\n");
			return CMD_WARNING;
		}

		ret = setTemperatureAlarmValue( vty, VOS_AtoL(argv[1]), VOS_AtoL(argv[0]) );
	}

	if ( VOS_ERROR == ret )
	{
		vty_out( vty, "command executing failed!\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN  (
		set_fan_monitor_enable,
		set_fan_monitor_enable_cmd,
		"fan-monitor [enable|disable]",
		"set fan monitor enable\n"
		"fan monitor enable\n"
		"fan monitor disable\n"
		)
{
	if ( VOS_StrCmp( (char*)argv[0], "enable") == 0 )
	{
		if( setFanAlarmEnable( FAN_ALARM_ENABLE ) == VOS_ERROR )
		{
			vty_out( vty, "Sorry, please update the CPLD version.\r\n" );
		}
		else
		{
			vty_out( vty, "The fan monitor is enable now.\r\n" );
		}
	}
	else /*if ( VOS_StrCmp( (char*)argv[0], "disable") == 0 )*/
	{
		if ( VOS_OK == setFanAlarmEnable( FAN_ALARM_DISABLE ) )
		{
			vty_out( vty, "The fan monitor is disable now.\r\n" );
		}
	}
	return CMD_SUCCESS;
}

DEFUN  (
		set_temperature_monitor_enable,
		set_temperature_monitor_enable_cmd,
		"temperature-monitor [enable|disable]",
		"set temperature monitor enable\n"
		"temperature monitor enable\n"
		"temperature monitor disable\n"
		)
{
	if ( VOS_StrCmp( (char*)argv[0], "enable") == 0 )
	{
		if ( VOS_OK == setTemperatureAlarmEnable( TEMPERATURE_ALARM_ENABLE ) )
		{
			vty_out( vty, "The temperature monitor is enable now.\r\n" );
		}
	}
	else if ( VOS_StrCmp( (char*)argv[0], "disable") == 0 )
	{
		if ( VOS_OK == setTemperatureAlarmEnable( TEMPERATURE_ALARM_DISABLE ) )
		{
			vty_out( vty, "The temperature monitor is disable now.\r\n" );
		}
	}

	return CMD_SUCCESS;
}

DEFUN  (
		set_fan_rev_threshold,
		set_fan_rev_threshold_cmd,
		"fan alarm-threshold <0-3200>",
		"set fan alarm\n"
		"set fan alarm threshold\n"
		"Input the fan alarm per second\n"
		)
{
	int ret;

		if ( FAN_ALARM_ENABLE != getFanAlarmEnable() )
		{
			vty_out( vty, "The fan alarm is disable.Please enable fan alarm first!\r\n");
			return CMD_WARNING;
		}
		ret = setFanAlarmMinRev(VOS_AtoL(argv[0]));
	return CMD_SUCCESS;
}

/* for test,读寄存器的值 */
DEFUN  (
		show_fan_reg_value,
		show_fan_reg_value_cmd,
		"show fan-reg-value",
		"show fan register value\n"
		"show fan register value\n"
		)
{
	UCHAR validReg, alarmReg;

	ReadCPLDReg( (unsigned char *)FAN_VALID_ADDR, &validReg );
	ReadCPLDReg( (unsigned char *)FAN_ALARM_ADDR, &alarmReg );

	vty_out( vty, "6700 The Value of Valid Reg Addr 0x16 is 0x%02x\r\n", validReg);
	vty_out( vty, "6700 The Value of Alarm Reg Addr 0x0B is 0x%02x\r\n", alarmReg);

	/*vty_out( vty, "6700 The static variable fan alarm state is 0x%02x\r\n", fanAlarm);*/

	return CMD_SUCCESS;
}

/* 调试开关 */
DEFUN  (
		set_environment_debug,
		set_environment_debug_cmd,
		"environment debug {<debug>}*1",
		"show environment debug\n"
		"show environment debug info\n"
		"0 - undo debug, 1 - fan Debug, 2 - temperature Debug\n"
		)
{
	if (1 == argc)
	{
		if (0 == VOS_AtoL(argv[0]))
		{
			fanDebug = 0;
			temperatureDebug = 0;
		} 
		else if (1 == VOS_AtoL(argv[0]))
		{
			fanDebug = 1;
		} 
		else if (2 == VOS_AtoL(argv[0]))
		{
			temperatureDebug = 1;
		}
		else
		{
			vty_out( vty, "command executing failed!\r\n");
			return CMD_WARNING;
		}
	}
	else if (0 == argc)
	{
		fanDebug = 1;
		temperatureDebug = 1;
	}

	return CMD_SUCCESS;
}

DEFUN(fan_ctrl,
			fan_ctrl_cmd,
			"fan-speed <slot> [slow|fast]",
			"fan speed control\n"
			"slot number\n"
			"slow speed\n"
			"fast speed\n"
)
{
	int slot=VOS_AtoI(argv[0]);
	UCHAR val;
    UCHAR control;
	if( !PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE) && !PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE) )
	{
		return CMD_SUCCESS;
	}
    if(slot<0 ||slot>FAN_MAX_SLOT)
    {
		vty_out(vty,"\r\n  % Illegal slot number. \r\n",slot, slot);
        return CMD_WARNING;
    }
	if( !VOS_StrCmp("fast",argv[1]) )
	{
		val = 0x70;
        g_fan_speed[slot] = FAN_SPEED_FAST;
	}
	else if( !VOS_StrCmp("slow",argv[1]) )
	{
		val = 0x3c;	
        g_fan_speed[slot] = FAN_SPEED_SLOW;
	}    
	#if 0
	if(slot < 15 || slot > 17 )
	{
		vty_out(vty,"\r\n  % Wrong slot number \r\n");
		return CMD_SUCCESS;
	}
	else if(!SYS_MODULE_IS_6900_FAN(slot))
	{
		vty_out(vty,"\r\n  % Slot %d is empty \r\n",slot);
		return CMD_SUCCESS;
	}
	#else
	if(!SYS_MODULE_IS_FAN(slot) )
	{
		vty_out(vty,"\r\n  % Slot %d is not fan ,or no fan on slot %d. \r\n",slot, slot);
		return CMD_SUCCESS;
	}
	#endif
    
    
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		return CMD_SUCCESS;

    /*added by luh 2014-01-13*/
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (slot == 4)
		{
			slot = 15;
		}
	}
    else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)||(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
    {
		if (slot == 2)
		{
			return;
		}
    }

	fan_ctrl_i2c_read(slot,0x10,&control);
	if(!(control&1<<7))
	{
		vty_out(vty,"  %% Error,please config fan mode to manual first!\r\n");
		return CMD_SUCCESS;
	}
	control = 0x2;
	fan_ctrl_i2c_write(slot,0x11,control);
	fan_ctrl_i2c_write(slot,0x15,control);
/*	else if( !VOS_StrCmp("",argv[1]) )
	{
		val = 0x20;
	}
*/
	fan_ctrl_i2c_write(slot,0x26,val);
	fan_ctrl_i2c_write(slot,0x27,val);
	
	return CMD_SUCCESS;
}
#if 1
int fanMode = MODE_MANUAL;
void pwu_mode(int slot)
{
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		return;
    }
    /*added by luh 2014-01-13*/
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (slot == 4)
		{
			slot = 15;
		}
	}
    else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)||(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
    {
		if (slot == 2)
		{
			return;
		}
    }
    
	fan_ctrl_i2c_write( slot,0x4,0x78);
/*	fan_ctrl_i2c_write(slot,0x10,0xb2);
	fan_ctrl_i2c_write(slot,0x14,0xb2);*/
	/*modi by luh 2013-07-12, 转换fan手动模式时，转速调整为快速*/
#if 0	
	fan_ctrl_i2c_write(slot,0x26,0x3c);
	fan_ctrl_i2c_write(slot,0x27,0x3c);
#else
	fan_ctrl_i2c_write(slot,0x11,0x2);
	fan_ctrl_i2c_write(slot,0x15,0x2);

	fan_ctrl_i2c_write(slot,0x26,0x70);
	fan_ctrl_i2c_write(slot,0x27,0x70);
#endif
}
void rpm_auto(int slot)
{
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		return;
    }
    /*added by luh 2014-01-13*/
	if (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M)
	{
		if (slot == 4)
		{
			slot = 15;
		}
	}
    else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)||(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100))
    {
		if (slot == 2)
		{
			return;
		}
    }
	fan_ctrl_i2c_write( slot,0x4,0x78);
	fan_ctrl_i2c_write( slot,0x4,56);
	fan_ctrl_i2c_write( slot,0xd,47);
	fan_ctrl_i2c_write( slot,0xb,46);
	fan_ctrl_i2c_write( slot,0x3,0);
	fan_ctrl_i2c_write (slot,0x10,0x36);
	fan_ctrl_i2c_write (slot,0x14,0x36);
	fan_ctrl_i2c_write (slot,0x11,0x13);
	fan_ctrl_i2c_write (slot,0x15,0x13);
	fan_ctrl_i2c_write( slot,0x12,0x30);	
	fan_ctrl_i2c_write( slot,0x16,0x30);
	fan_ctrl_i2c_write (slot,0x22,0X77);
	fan_ctrl_i2c_write (slot,0x23,0X77);
	fan_ctrl_i2c_write (slot,0x29,28);
}
#endif

DEFUN(fan_mode,
			fan_mode_cmd,
			"fan-mode {[manual|auto]}*1",
			"fan mode ctrl\n"
			"ctrl fan speed manually\n"
			"ctrl fan speed automatically\n"
)
{
	int i;
	
	if( argc == 0 )	/* 添加一个回车显示当前的风扇模式added by mengxsh 20150506 */
	{
		if( fanMode == MODE_AUTO)
			vty_out(vty," fan-mode is %s.\r\n","auto");
		else
		{	
			vty_out( vty, " fan-mode is %s.\r\n", "manual" );
		}
		
	}
	else
	{
		if( !VOS_StrCmp("auto",argv[0]) )
		{
			fanMode = MODE_AUTO;
			FAN_6900_POLLING(i)
				rpm_auto(i);
		}
		else
		{
			fanMode = MODE_MANUAL;
			FAN_6900_POLLING(i)
			{
				pwu_mode(i);
			}
	        init_fan_config();
	    }
	}
	
	
	return CMD_SUCCESS;
}

STATUS alarm_Command_Install()
{
	install_element ( CONFIG_NODE, &show_temperature_monitor_cmd );
	install_element ( CONFIG_NODE, &show_all_board_temp_cmd );
	install_element ( CONFIG_NODE, &set_temperature_alarm_threshold_cmd );
	install_element ( CONFIG_NODE, &set_temperature_monitor_enable_cmd );
	install_element ( CONFIG_NODE, &fan_ctrl_cmd );	
	install_element ( CONFIG_NODE, &fan_mode_cmd );		
	install_element ( DEBUG_HIDDEN_NODE, &set_environment_debug_cmd );
	
	/*added by mengxsh 20150408*/
	install_element ( VIEW_NODE, &show_temperature_monitor_cmd );
	install_element ( VIEW_NODE, &show_all_board_temp_cmd );

	if( fan_num_total )
	{
		install_element ( CONFIG_NODE, &set_fan_monitor_enable_cmd );
		install_element ( CONFIG_NODE, &set_fan_rev_threshold_cmd );
		install_element ( DEBUG_HIDDEN_NODE, &show_fan_reg_value_cmd );
	}

	return VOS_OK;
}

STATUS alarm_Environment_Init()
{
	int i;
	  
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )

	if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
	{
		temperature_gpio = 25;
		fan_num_total = 4;
		temperatureAlarmMode = TEMPERATURE_ALARM_MODE_INTEGRATED;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		temperature_gpio = 3;	/* 只读主板 */
		fan_num_total = 3;
		temperatureAlarmMode = TEMPERATURE_ALARM_MODE_INTEGRATED;
	}
	else if(PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE))
	{		
		fanMinRev = FAN_6900_ALARM_DEFAULT_MIN_REV;
		if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		{
			if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900)
			{
				temperature_gpio = 25;
				fan_num_total = 6;	
			}
			else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)  /*****add by   mengxsh  20140917 *****/ 
			{
				temperature_gpio = 0;
				fan_num_total = 6;
#if	0		/* removed by mengxsh 20150403*/	
				/***BEGIN**add by	mengxsh  20140923 *****/
				for(i=15; i<=17; i++)/*  在gfa8000 中将风速调整为最大   */
				{
					devsm_fan_set_insert_fast(i);
				}
				/***END**add by   mengxsh  20140923 *****/
#endif
			}
			
		}
		else
		{
			if( SYS_LOCAL_MODULE_TYPE_IS_CPU_PON )
			{
				if(SYS_LOCAL_MODULE_TYPE_IS_8000_GPON)
				{
					temperature_gpio = 5;
				}
				else
				{
	                /*modi by luh 2013-09-09，修改新pon板温度的gpio*/
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)                
	                unsigned char bCPLDType = 0;                
	                /* 根据CPLD版本，采用对应的触发方式 */
	                ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
	                
	                if ( 3 > bCPLDType )
	                {
	                    /*旧cpu*/
	                    temperature_gpio = 31;
	                }
	                else
	                {
	                    /*新4EPON 板cpu*/
	                    temperature_gpio = 0;
	                }                
#else
					temperature_gpio = 31;
#endif
				}
			}
			fan_num_total = 0;
		}
	}
	else if(PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE))
	{		
		fanMinRev = FAN_6900_ALARM_DEFAULT_MIN_REV;
		if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)               
            unsigned char bCPLDType = 0;                
            /* 根据CPLD版本，采用对应的触发方式 */
            ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
            
            if ( 3 > bCPLDType )
            {
                /*旧cpu*/
                temperature_gpio = 31;
            }
            else
            {
                /*新4EPON 板cpu*/
                temperature_gpio = 0;
            }                
#else
			temperature_gpio = 31;
#endif
			fan_num_total = 2;
		}
		else
		{
			if( SYS_LOCAL_MODULE_TYPE_IS_SWITCH_PON )
			{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)               
                unsigned char bCPLDType = 0;                
                /* 根据CPLD版本，采用对应的触发方式 */
                ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
                
                if ( 3 > bCPLDType )
                {
                    /*旧cpu*/
                    temperature_gpio = 31;
                }
                else
                {
                    /*新4EPON 板cpu*/
                    temperature_gpio = 0;
                }                
#else
				temperature_gpio = 31;
#endif
			}
			fan_num_total = 0;
		}
		
	}
	else if(PRODUCT_IS_HS_Series(SYS_PRODUCT_TYPE))
	{		
		fanMinRev = FAN_6900_ALARM_DEFAULT_MIN_REV;
		if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
			{
				temperature_gpio = 5;
				fan_num_total = 3;
			}
			else
			{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)               
		        unsigned char bCPLDType = 0;                
		        /* 根据CPLD版本，采用对应的触发方式 */
		        ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
		        
		        if ( 3 > bCPLDType )
		        {
		            /*旧cpu*/
		            temperature_gpio = 31;
		        }
		        else
		        {
		            /*新4EPON 板cpu*/
		            temperature_gpio = 0;
		        }                
#else
				temperature_gpio = 31;
#endif
				fan_num_total = 4;
			}
		}
		else
		{
			if( SYS_LOCAL_MODULE_TYPE_IS_SWITCH_PON )
			{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)                
                unsigned char bCPLDType = 0;                
                /* 根据CPLD版本，采用对应的触发方式 */
                ReadCPLDReg((volatile UCHAR*)0, &bCPLDType);
                
                if ( 3 > bCPLDType )
                {
                    /*旧cpu*/
                    temperature_gpio = 31;
                }
                else
                {
                    /*新4EPON 板cpu*/
                    temperature_gpio = 0;
                }                
#else
				temperature_gpio = 31;
#endif
			}
			fan_num_total = 0;
		}
		temperatureAlarmMode = TEMPERATURE_ALARM_MODE_INTEGRATED;
	}

	if( fan_num_total )
		setFanAlarmEnable(FAN_ALARM_ENABLE);
	setTemperatureAlarmEnable(TEMPERATURE_ALARM_ENABLE);
    init_fan_config();
	alarm_Command_Install();

 
	check_environment_hook_rtn = environment_check_callback;


#endif
	return VOS_OK;
}
void init_fan_config()
{
    int i;
    VOS_MemZero(g_fan_speed, FAN_MAX_SLOT);
    for(i=0;i<FAN_MAX_SLOT;i++)
        g_fan_speed[i]=FAN_SPEED_FAST;    
}
void restore_fan_config(USHORT slot)
{
    if(fanMode == MODE_AUTO)
        rpm_auto(slot);
    else
    {
        if(g_fan_speed[slot] == FAN_SPEED_FAST)
            pwu_mode(slot);
    }
}
void load_fan_config()
{
    int i;
	FAN_6900_POLLING(i)
	{
        if(fanMode == MODE_AUTO)
            rpm_auto(i);
        else
        {
            if(g_fan_speed[i] == FAN_SPEED_FAST)
                pwu_mode(i);
        }
	}
}
ULONG fan_alarm_default_min_rev_get()
{
	if ( PRODUCT_IS_H_Series(SYS_PRODUCT_TYPE) )
	{
		return FAN_6900_ALARM_DEFAULT_MIN_REV;
	}
	else
	{
		return FAN_ALARM_DEFAULT_MIN_REV;
	}
}
STATUS environment_showrun( struct vty * vty )
{
#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
	UCHAR value;
	LONG temperature1, temperature2;
	ULONG rev;

	vty_out( vty, "!Environment config\r\n" );
	value = getTemperatureAlarmEnable();
	if (TEMPERATURE_ALARM_ENABLE != value)
	{
		vty_out( vty, " temperature-monitor disable\r\n") ;
	}

	temperature1 = getTemperatureAlarmValueHigh();
	temperature2 = getTemperatureAlarmValueLow();
	if ( DEFAULT_MAX_ALARM_TEMPERATURE != temperature1 || DEFAULT_MIN_ALARM_TEMPERATURE != temperature2 )
	{
		vty_out( vty, " temperature alarm-threshold %ld %ld\r\n", temperature1, temperature2 );
	}

	if( fan_num_total )
	{
		value = getFanAlarmEnable();
		if (FAN_ALARM_ENABLE != value)
		{
			vty_out( vty, " fan-monitor disable\r\n") ;
		}
		rev = getFanAlarmMinRev();
		if (fan_alarm_default_min_rev_get() != rev)
		{
			vty_out( vty, " fan alarm-threshold %ld\r\n", rev );
		}
		/*
		if( fanMode == MODE_MANUAL )
			vty_out(vty," fan-mode %s\r\n","manual");
		else
			vty_out(vty," fan-mode %s\r\n","auto");
		*/
		if( fanMode == MODE_AUTO)
			vty_out(vty," fan-mode %s\r\n","auto");
        else
        {
            int i;
    		FAN_6900_POLLING(i)
    		{
    			if(g_fan_speed[i] == FAN_SPEED_SLOW)
                    vty_out(vty, " fan-speed %d slow\r\n", i);
    		}
        }
	}
#endif
	vty_out( vty, "!\r\n\r\n" );
	return VOS_OK;
}

/*判断输入参数是否为纯数字*/
STATUS validateParamIsFigure(CHAR *param)
{
	UCHAR strLen, i = 0;

	strLen = VOS_StrLen(param);

	if (param[i] == '-')/* 符号跳过 */
	{
		i++;
	}

	for (; i < strLen; i++)
	{
		if ( (param[i] < '0') || (param[i] > '9') )
		{
			return VOS_ERROR;
		}
	}

	return VOS_OK;
}

#endif	/*#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )*/
#ifdef	__cplusplus
}
#endif/* __cplusplus */

