#include  "OltGeneral.h"
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "PonEventHandler.h"
#include  "../cli/Pon_cli.h"
#include  "lib_eponOpticPowerMon.h"
#include "Math.h"
#include "gwEponMibData.h"
#include "includeFromPas.h"
#include "vos/vospubh/cdp_syn.h"
#include "vos/vospubh/vos_sysmsg.h"
#include "vos/vospubh/vos_ctype.h"
#include	"vos/vospubh/cdp_pub.h" 
#include "i2c.h"
#include "../ct_manage/CT_Onu_event.h"
#include "ifm_api.h"
#include "bsp_cpld.h"

long Optical_Timer_Callback();
ULONG sfp_debug_switch = 0;
/*#define SFP_RSSI_DEBUG(d, x) if(sfp_debug_switch & d) sys_console_printf x  */
#define SFP_RSSI_DEBUG(d, x) if(sfp_debug_switch & d) VOS_SysLog x;
#define SFP_OFFSET 4
#define SFP_10GE_OFFSET 8
#define SFP_8100_10GE_OFFSET 2

#define GFA8100_UPLINK_SFP_STATE 0x00000009
#define GFA8100_UPLINK_SFP_GPON_STATE 0x01000000+0x0c

ULONG		gMQidOpticalPower = 0;
ULONG		OpticalPowerTaskId = 0;
unsigned int 	gOpticalPowerInitStatus = 0;
LONG PonpoweringTimerId=0;
LONG UplinkLosCheckTimerId=0;
#define OPTICALPOWER_TASK_PRIO   251

ULONG Pon_Power_Interval_Times_Default = 1 /*3 */; 
		/*modified by duzhk 2011-09-19  每一个检测周期都向主控板同步一次*/
ULONG Pon_Power_Interval_Times = 0;

#define OPT_TX_PWR_QU_DEF	1
#define OPT_BIAS_CUR_QU_DEF	2
int opt_tx_pwr_qunit = OPT_TX_PWR_QU_DEF;		/* 量化单位0.1uW */
int opt_biascur_qunit = OPT_BIAS_CUR_QU_DEF;	/* 量化单位2uA */

/* added by xieshl 20101104, 增加上联光模块检测校准类型判断，参见SFF-8472 */
#define NOT_SUPPORT_SFF8472		0
#define EXTERNALLY_CALIBRATED		1
#define INTERNALLY_CALIBRATED		2
int optical_print_debug = 0;
#define  OPTICAL_DEBUGOUT  if(optical_print_debug)sys_console_printf

typedef enum
{
	 optical_power_enable ,
	 /*optical_power_disable ,*/
  /*optical-power [enable|disable]*/

	 optical_power_interval ,
  /* <1-86400> */

	 pon_adc_negative_polarity_stp_type ,
	 undo_pon_adc_negative_polarity_stp_type,
  /* pon-adc negative-polarity sfp-type <name>*/

	 optical_power_calibration_olt ,
	 optical_power_calibration_onu ,
  /* <rxCali> <txCali> */

	 optical_power_threshold_deadzone,
	 /*optical_power_threshold_deadzone_temperature ,
	 optical_power_threshold_deadzone_voltage ,
	 optical_power_threshold_deadzone_current ,*/
  /*optical-power threshold-deadzone <power> <tempertaure> <voltage> <current> */

	 optical_power_alarm_threshold_olt ,
	 optical_power_alarm_threshold_onu ,
	 optical_power_alarm_threshold_uplink ,
  /* <Tx_high> <Tx_low> <Rx_high> <Rx_low> */ 

	  optical_bias_current_alarm_threshold_olt , 
	  optical_bias_current_alarm_threshold_onu ,
	  optical_bias_current_alarm_threshold_uplink ,
 /* [olt|onu|uplink] <high> <low>*/

	 optical_temperature_alarm_threshold_olt	 ,
	 optical_temperature_alarm_threshold_onu ,	
	 optical_temperature_alarm_threshold_uplink ,
  /*optical-temperature alarm-threshold [olt|onu|uplink] <high> <low>*/
	 optical_voltage_alarm_threshold_olt ,
	 optical_voltage_alarm_threshold_onu ,	
	 optical_voltage_alarm_threshold_uplink,
  /*optical-voltage alarm-threshold [olt|onu|uplink] <high> <low>*/
	 onu_laser_always_on_enable,
	 /*onu_laser_always_on_disable,*/
	 onu_laser_always_on_alarm_times,
	 onu_laser_always_on_alarm_clear_times,
	 onu_laser_always_on_alarm_threshold,

	show_optical_power_olt_rx_instant,
	show_optical_power_olt_rx_history,
	show_onu_laser_always_on,	
	show_optical_power_XcvrInfoArr,
	debug_optical_power_command,
	show_optical_power_pon_sfp_type,
	show_online_sfp_pon,
	get_sfp_online_state,
	
	/*uplink_los_check_interval,
	optical_power_alarm_threshold_uplink,
	optical_bias_current_alarm_threshold_uplink,
	optical_temperature_alarm_threshold_uplink,
	optical_voltage_alarm_threshold_uplink*/
}Optical_Cmd_Type_t;


typedef int (* ctss_optical_power_enable_f)(unsigned int PonPortIdx,int enable_mode, int onu_enable_mode, int Flag); 
typedef int (* ctss_optical_power_interval_f)(unsigned int PonPortIdx,int interval); 
typedef int (* ctss_pon_adc_negative_polarity_stp_type_f)(unsigned int PonPortIdx,UCHAR *String_Name);
typedef int (* ctss_undo_pon_adc_negative_polarity_stp_type_f)(unsigned int PonPortIdx,UCHAR *String_Name);
typedef int (* ctss_optical_power_calibration_olt_f)(unsigned int PonPortIdx,long rx_calibration, long tx_calibration); 
typedef int (* ctss_optical_power_calibration_onu_f)(unsigned int PonPortIdx,long rx_calibration, long tx_calibration); 
typedef int (* ctss_optical_power_threshold_deadzone_f)(unsigned int PonPortIdx,long power_deadzone, long temperature_deadzone,long supply_voltage_deadzone,long bias_current_zone);
typedef int (* ctss_optical_power_alarm_threshold_olt_f)(unsigned int PonPortIdx,long tx_power_low,long tx_power_high,long rx_power_low,long rx_power_high,int flag); 
typedef int (* ctss_optical_power_alarm_threshold_onu_f)(unsigned int PonPortIdx,long tx_power_low,long tx_power_high,long rx_power_low,long rx_power_high); 
typedef int (* ctss_optical_power_alarm_threshold_uplink_f)(unsigned int PonPortIdx,long tx_power_low,long tx_power_high,long rx_power_low,long rx_power_high,int flag); 
typedef int (* ctss_optical_bias_current_alarm_threshold_olt_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag); 
typedef int (* ctss_optical_bias_current_alarm_threshold_onu_f)(unsigned int PonPortIdx,long low_val,long high_val); 
typedef int (* ctss_optical_biasCurrent_alarm_threshold_uplink_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag); 
typedef int (* ctss_optical_temperature_alarm_threshold_olt_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag);
typedef int (* ctss_optical_temperature_alarm_threshold_onu_f)(unsigned int PonPortIdx,long low_val,long high_val);
typedef int (* ctss_optical_temperature_alarm_threshold_uplink_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag);
typedef int (* ctss_optical_voltage_alarm_threshold_olt_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag);
typedef int (* ctss_optical_voltage_alarm_threshold_onu_f)(unsigned int PonPortIdx,long low_val,long high_val);
typedef int (* ctss_optical_voltage_alarm_threshold_uplink_f)(unsigned int PonPortIdx,long low_val,long high_val,int flag);
typedef int (* ctss_onu_laser_always_on_enable_f)(unsigned int PonPortIdx,int enable_mode); 
typedef int (* ctss_onu_laser_always_on_alarm_times_f)(unsigned int PonPortIdx,int alarm_times); 
typedef int (* ctss_onu_laser_always_on_alarm_clear_times_f)(unsigned int PonPortIdx,int clear_times); 
typedef int (* ctss_onu_laser_always_on_alarm_threshold_f)(unsigned int PonPortIdx,int alarm_threshold); 
typedef int (* ctss_show_optical_power_olt_rx_instant_f)(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length,void *pointer); 
typedef int (* ctss_show_optical_power_olt_rx_history_f)(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length, void *pBuf); 
typedef int (* ctss_show_onu_laser_always_on_f)(ULONG ulSlot, ULONG ulPort,UCHAR *TypeFlag,void *pBuf);
typedef int (* ctss_show_optical_power_XcvrInfoArr_f)(short int ulSlot, short int ulPort, UCHAR *TypeFlag, void*pBuf );
typedef int (* ctss_debug_optical_power_command_f)(unsigned int PonPortIdx,long enable_mode, long DebugType);
typedef int (* ctss_show_optical_power_pon_sfp_type_f)(short int ulSlot, short int ulPort, void*pBuf );
typedef int (* ctss_show_online_sfp_pon_f)(ULONG ulSlot, ULONG ulPort, void*pBuf,USHORT pBufLen,struct vty *vty );
typedef int (* ctss_get_sfp_online_state_f)(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType);

#define OPTICALPOWER_ASSERT(PonPortIdx)       VOS_ASSERT( ((PonPortIdx) >= 0) && ((PonPortIdx) < MAXPON) )

#define OPTICAL_PONPORTIDX_ALL    -1


ULONG opticalpower_semid = 0;
typedef struct stOpticalPowerMgmtIFs {
	ctss_optical_power_enable_f optical_power_enable_t;
	ctss_optical_power_interval_f optical_power_interval_t;
	ctss_pon_adc_negative_polarity_stp_type_f pon_adc_negative_polarity_stp_type_t;
	ctss_undo_pon_adc_negative_polarity_stp_type_f undo_pon_adc_negative_polarity_stp_type_t;
	ctss_optical_power_calibration_olt_f  optical_power_calibration_olt_t;
	ctss_optical_power_calibration_onu_f optical_power_calibration_onu_t;
	ctss_optical_power_threshold_deadzone_f optical_power_threshold_deadzone_t;
	ctss_optical_power_alarm_threshold_olt_f optical_power_alarm_threshold_olt_t;
	ctss_optical_power_alarm_threshold_onu_f optical_power_alarm_threshold_onu_t;
	ctss_optical_power_alarm_threshold_uplink_f optical_power_alarm_threshold_uplink_t;
	ctss_optical_bias_current_alarm_threshold_olt_f optical_bias_current_alarm_threshold_olt_t;
	ctss_optical_bias_current_alarm_threshold_onu_f optical_bias_current_alarm_threshold_onu_t;
	ctss_optical_biasCurrent_alarm_threshold_uplink_f optical_biasCurrent_alarm_threshold_uplink_t;
	ctss_optical_temperature_alarm_threshold_olt_f optical_temperature_alarm_threshold_olt_t;
	ctss_optical_temperature_alarm_threshold_onu_f optical_temperature_alarm_threshold_onu_t;
	ctss_optical_temperature_alarm_threshold_uplink_f optical_temperature_alarm_threshold_uplink_t;
	ctss_optical_voltage_alarm_threshold_olt_f optical_voltage_alarm_threshold_olt_t;
	ctss_optical_voltage_alarm_threshold_onu_f optical_voltage_alarm_threshold_onu_t;
	ctss_optical_voltage_alarm_threshold_uplink_f optical_voltage_alarm_threshold_uplink_t;
	ctss_onu_laser_always_on_enable_f onu_laser_always_on_enable_t;
	ctss_onu_laser_always_on_alarm_times_f onu_laser_always_on_alarm_times_t;
	ctss_onu_laser_always_on_alarm_clear_times_f onu_laser_always_on_alarm_clear_times_t;
	ctss_onu_laser_always_on_alarm_threshold_f onu_laser_always_on_alarm_threshold_t;
	ctss_show_optical_power_olt_rx_instant_f show_optical_power_olt_rx_instant_t;
	ctss_show_optical_power_olt_rx_history_f show_optical_power_olt_rx_history_t;
	ctss_show_onu_laser_always_on_f  show_onu_laser_always_on_t;
	ctss_show_optical_power_XcvrInfoArr_f show_optical_power_XcvrInfoArr_t;
	ctss_debug_optical_power_command_f debug_optical_power_command_t;
	ctss_show_optical_power_pon_sfp_type_f show_optical_power_pon_sfp_type_t;
	ctss_show_online_sfp_pon_f show_online_sfp_pon_f;
	ctss_get_sfp_online_state_f  get_sfp_online_state_t; 
}OpticalPowerMgmtIFs;

/*const OpticalPowerMgmtIFs  *OptPowRpcIfs = NULL; */

/*#define OPTICALPOWER_API_CALL(oltid, fun, params) (( (NULL != OptPowRpcIfs) && (NULL != OptPowRpcIfs->fun) ) ? (*OptPowRpcIfs->fun) params : VOS_ERROR)*/
OpticalPowerMgmtIFs *pM_rpcIfs;
OpticalPowerMgmtIFs *pS_rpcIfs;
#define OPTICALPOWER_API_CALL(oltid, fun, params) ( ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (GetCardIdxByPonChip(oltid) != SYS_LOCAL_MODULE_SLOTNO) && SYS_MODULE_SLOT_ISHAVECPU(GetCardIdxByPonChip(oltid))) ? (*pM_rpcIfs->fun) params : (*pS_rpcIfs->fun) params)


#define PON_ALARM_STATE_LOS    				(1<<0)
#define PON_ALARM_STATE_RXPOW_L    		(1<<1)   /*GW_RX_POWER_LOW_ALARM*/
#define PON_ALARM_STATE_RXPOW_H    		(1<<2)   /*GW_RX_POWER_HIGH_ALARM*/
#define PON_ALARM_STATE_TXPOW_L 			(1<<3)  /*GW_TX_POWER_LOW_ALARM*/
#define PON_ALARM_STATE_TXPOW_H   		(1<<4)  /*GW_TX_POWER_HIGH_ALARM*/
#define PON_ALARM_STATE_TEMP_L  			(1<<5)  /*GW_TEMP_LOW_ALARM*/
#define PON_ALARM_STATE_TEMP_H 			(1<<6)   /*GW_TEMP_HIGH_ALARM*/
#define PON_ALARM_STATE_VOLT_L   			(1<<7)  /*GW_VCC_LOW_ALARM*/
#define PON_ALARM_STATE_VOLT_H   			(1<<8)  /*GW_VCC_HIGH_ALARM*/
#define PON_ALARM_STATE_BIAS_L   			(1<<9)  /*GW_TX_BIAS_LOW_ALARM*/
#define PON_ALARM_STATE_BIAS_H   			(1<<10)  /*GW_TX_BIAS_HIGH_ALARM*/
#define PON_ALARM_STATE_LASER_ALWAYS_ON   (1<<11)

#define UPLINK_ALARM_STATE_LOS			(1<<0)
#define UPLINK_ALARM_STATE_RXPOW_L   		(1<<1)
#define UPLINK_ALARM_STATE_RXPOW_H  	 	(1<<2)
#define UPLINK_ALARM_STATE_TXPOW_L   		(1<<3)
#define UPLINK_ALARM_STATE_TXPOW_H  	 	(1<<4)
#define UPLINK_ALARM_STATE_TEMP_L   		(1<<5)
#define UPLINK_ALARM_STATE_TEMP_H   		(1<<6)
#define UPLINK_ALARM_STATE_VOLT_L   		(1<<7)
#define UPLINK_ALARM_STATE_VOLT_H   		(1<<8)
#define UPLINK_ALARM_STATE_BIAS_L   		(1<<9)
#define UPLINK_ALARM_STATE_BIAS_H   		(1<<10)

typedef struct Optical_Insert_CdpHead{
	eponOpticalPowerThresholds_t *eponOpticalPowerThresholds_s;
	eponOpticalPowerDeadZone_t *eponOpticalPowerDeadZone_s;
	long olt_rx_optical_power_calibration ;
	long olt_tx_optical_power_calibration ;
	long onu_rx_optical_power_calibration;
	long onu_tx_optical_power_calibration ;
	LONG onuLaser_alwaysOn_Enable;
	ULONG onuLaser_alwaysOn_alarm_timeCounter ;
	ULONG onuLaser_alwaysOn_clear_timeCounter;
	ULONG onuLaser_alwaysOn_alarm_threshold ;
	ULONG sfp_debug_switch;
	int sfp_type_num;
}Optical_Insert_CdpHead_s;

typedef struct Online_Sfp_Info_Get{
	USHORT Online;
	UCHAR Vendorname[16];
	UCHAR NameEnd;
	UCHAR VendorPN[16];
	UCHAR PnEend;
	UCHAR VendorSN[16];
	UCHAR SnEnd;
	UCHAR DataCode[8];
	UCHAR DataEnd;
	UCHAR OpticalPower;
	UCHAR Reserved;
	UCHAR Diagnostic_Monitoring_Type;
	UCHAR Wavelength[2];
	UCHAR WaveEnd;
}Online_Sfp_Info_Get_t;

eponOpticalPowerThresholds_t eponOpticalPowerThresholds;
eponOpticalPowerDeadZone_t eponOpticalPowerDeadZone;

extern ULONG PowerMeteringSemId;

long olt_rx_optical_power_default = -1000;
long uplink_rx_optical_power_default = -500;

long olt_rx_optical_power_calibration = 0;
long olt_tx_optical_power_calibration = 0;
long onu_rx_optical_power_calibration = 0;
long onu_tx_optical_power_calibration = 0;

/*unsigned int Optical_Scope_Timer_Counter = 0; */
/* 下一个等待光功率检测的PON 口*/
short int  Optical_Scope_PonPortIdx =0;
short int  Optical_Scope_PonPortIdx_Onu =0;
short int  Optical_Scope_UplinkPortIdx = 1;
/* PON 光功率检测周期*/
unsigned int Optical_Scope_Sample_default = 5;
/* 每个PON 口光功率检测周期*/
/*unsigned int Optical_Scope_Sample_Interval_1 = 5;*/
/*
extern int i2c_read_all(int device, int sfp_sel, unsigned char i2c_addr, unsigned char reg, unsigned char * value);
extern int i2c_read_all2(int device,int sfp_sel,unsigned char i2c_addr, unsigned char reg, unsigned char* value);
extern int i2c_read_all4(int device,int sfp_sel,unsigned char i2c_addr, unsigned char reg, unsigned char* value);
extern int i2c_write_all(int device,int sfp_sel,unsigned char i2c_addr, unsigned char reg, unsigned char val);
*/
extern int onuOpticalParaAlm_EventReport(USHORT paraType, ulong_t almFlag, ulong_t devIdx, ulong_t brdIdx, ulong_t portIdx, long rtVal);
extern int oltOpticalRxLowClear_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, long oltrxPower);
extern int oltOpticalRxHigh_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx,long oltrxPower);
extern int oltOpticalRxHighClear_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, long oltrxPower);
extern int oltOpticalRxLow_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, long oltrxPower);
extern int BroadcastOamFrameToOnu( short int PonPortIdx, int length, unsigned char *content );
extern int UnicastOamFrameToOnu( short int PonPortIdx, short int llid, int length, unsigned char *content );
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG IFM_ParseSlotPort( CHAR * szName, ULONG * pulSlot, ULONG * pulPort );
extern STATUS getDeviceName( const ulong_t devIdx, char* pValBuf, ulong_t *pValLen );
extern VOID OpticalPower_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen );
/*extern LONG RPU_SendCmd2OpticalPower(Optical_Cmd_Type_t  cmd_type,short int PonPortIdx,UCHAR *strings,ULONG length,ULONG para1,
				ULONG para2, ULONG para3, ULONG para4);*/
extern int parse_pon_command_parameter( struct vty *vty, ULONG *pulSlot, ULONG *pulPort , ULONG *pulOnuId, INT16 *pi16PonId );
extern int  OltPonpowerAlwaysOnDetection(void);
extern STATUS OpticalToPonPdpInit();
extern int UplinkSFPRecvPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPRecvPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long rxPower);
extern int UplinkSFPTransPowerLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPTransPowerHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long txPower);
extern int UplinkSFPVoltageLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPVoltageHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long voltage);
extern int UplinkSFPBiasCurrentLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPBiasCurrentHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long current);
extern int UplinkSFPTemperatureLow_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureLowClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureHigh_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int UplinkSFPTemperatureHighClear_EventReport( ulong devIdx,ulong_t brdIdx, ulong_t ethIdx,  long temperature);
extern int ponPortLaserAlwaysOn_EventReport( ulong_t brdIdx, ulong_t portIdx, ulong_t onudevidx, LONG voltage);
extern int ponPortLaserAlwaysOnClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx , LONG dbm);
extern ULONG DEV_IsMySelfMaster(VOID);
extern ULONG DEV_GetPhySlot(VOID);
extern LONG OpticalPower_PRC_CALL(Optical_Cmd_Type_t  cmd_type,short int PonPortIdx, UCHAR *strings,ULONG length,ULONG para1,
				ULONG para2, ULONG para3, ULONG para4,ULONG para5,VOID *pRcvData, ULONG ulRcvLen);
extern LONG Sfp_Type_Add(UCHAR *String_Name);
extern int ReadPonPortRecevPower( short int PonPortIdx ,short int OnuIdx , short int Flag, long *val);
extern ULONG Show_OpticalPower_OltRx_Instant_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,ULONG pBufLen, struct vty *vty);
extern ULONG Show_OpticalPower_XcvrInfoArr_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf, ULONG pBufLen,UCHAR *flag, struct vty *vty);
extern ULONG Show_OpticalPower_OnuLaser_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf, ULONG pBufLen,UCHAR *flag, struct vty *vty);
extern ULONG Show_OpticalPower_OltRx_History_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,ULONG pBufLen, struct vty *vty);
extern ULONG Sfp_Type_Delete(UCHAR *String_Name);
extern int setOltOpticalPowerCalibration( long rx_calibration, long tx_calibration );
extern  int checkPonPortIndex( struct vty *vty, ULONG  ulSlot, ULONG ulPort );
extern int SetOpticalPowerDebug(long enable_mode, long DebugType);
extern int InitSFPTypeString();

int CheckPonPortTemperature(short int PonPortIdx, /*long tem,*/long Temperature_cur);
int CheckPonPortWorkVoltage(short int PonPortIdx,/*long vol,*/long workVoltage_cur);
int CheckPonPortBiasCurrent(short int PonPortIdx, /*long bias,*/long BiasCurrent_cur);
int CheckPonPortTransOpticalPower(short int PonPortIdx, long TransOpticalPower_cur);
int ClearOpticalPowerAlamWhenOltDetectionDisable(short int PonPortIdx,short int OnuIdx);
int ClearOpticalPowerAlarmWhenPonPortDown(short int PonPortIdx);
int ReadPonPortTransPower( short int PonPortIdx, long *val );
int ReadPonPortTemperature( short int PonPortIdx, long *val);
int ReadPonPortVoltage( short int  PonPortIdx, long *val);
int ReadPonPortBias( short int PonPortIdx, long *val );
int ReadUplinkPortTransPower( short int slotno, int sfp_sel, long *val);
int ReadUplinkPortRecevPower( short int slotno, int sfp_sel, long *val);
int ReadUplinkPortTemperature( short int slotno, int sfp_sel, long *val);
int ReadUplinkPortVoltage( short int slotno, int sfp_sel, long *val);
int ReadUplinkPortBias( short int slotno, int sfp_sel, long *val);
static LONG SetPonPortOpticalMonitorEnable(long val);
int Check_Sfp_Online_12Epon_Uplink(short int slotno, short int sfp_sel);
LONG ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss(int PonPortIdx);
LONG SetPonPortOpticalMonitorInterval(int val);
static int setOnuOpticalPowerCalibration( long rx_calibration, long tx_calibration );
int setUplinkPortSFPUpdated(short int slotno,int sfp_sel);
int ClearAllUplinkAlarmWhenLOS(short int slotno, short int sfp_sel);
int CheckPonSFPVendorSpecificIsGwd(short int PonPortIdx);
int ponSFPType_read(short int PonPortIdx, char *SFPType);
UCHAR UplinkSFPDiagnosticMonitorType(short int slotno,short int sfp_sel, int Flag );
int Check_Sfp_Online_6100(short int slotno, short int sfp_sel);
int ReadRatefromUplinkSfp(unsigned int slotno,unsigned int portno, ULONG* rate);
int Onu_Tx_Power_Supply_Control(ULONG ulSlot , ULONG ulPort ,ULONG *onuList ,ULONG length,ULONG action);
LONG onu_tx_power_supply_control(ULONG ulSlot , ULONG ulPort ,ULONG *onuList ,ULONG length,ULONG action );

LONG GetUplinkPortTransOpticalPowerLowthrd(int Flag);
LONG SetUplinkPortTransOpticalPowerLowthrd(long val, int Flag);
LONG GetUplinkPortTransOpticalPowerHighthrd(int Flag);
LONG SetUplinkPortTransOpticalPowerHighthrd(long val, int Flag);
LONG GetUplinkPortRecvOpticalPowerLowthrd(int Flag);
LONG SetUplinkPortRecvOpticalPowerLowthrd(long val, int Flag );
LONG GetUplinkPortRecvOpticalPowerHighthrd(int Flag);
LONG SetUplinkPortRecvOpticalPowerHighthrd(long val ,int Flag);
LONG GetUplinkPortTemperatureLowthrd(int Flag);
LONG SetUplinkPortTemperatureLowthrd(long val, int Flag);
LONG GetUplinkPortTemperatureHighthrd(int Flag);
LONG SetUplinkPortTemperaturreHighthrd(long val, int Flag);
LONG GetUplinkPortVoltageLowthrd(int Flag);
LONG SetUplinkPortVoltageLowthrd(long val, int Flag);
LONG GetUplinkPortVoltageHighthrd(int Flag);
LONG SetUplinkPortVoltageHighthrd(long val, int Flag);
LONG GetUplinkPortBiasCurrentLowthrd(int Flag);
LONG SetUplinkPortBiasCurrentLowthrd(long val, int Flag);
LONG GetUplinkPortBiasCurrentHighthrd(int Flag);
LONG SetUplinkPortBiasCurrentHighthrd(long val, int Flag);
int GetGponSfpOnlineState(short int PonPortIdx,bool* state);
#define OPT_POWER_PON_MAX_NUM	SYS_MAX_PON_PORTNUM
#define READ_I2C_COUNT	5
LONG PonRecvOpticalPower_Buffer[OPT_POWER_PON_MAX_NUM][MAXONUPERPONNOLIMIT][READ_I2C_COUNT];	             /*接收光功率，unit 0.1dbm */
UCHAR PonRecvOpticalPower_Buffer_Index[OPT_POWER_PON_MAX_NUM][MAXONUPERPONNOLIMIT];
UCHAR PonRecvOpticalPower_Buffer_Flag[OPT_POWER_PON_MAX_NUM][MAXONUPERPONNOLIMIT];

LONG PonTransOpticalPower_Buffer[OPT_POWER_PON_MAX_NUM][READ_I2C_COUNT];	                /*发送光功率, unit 0.1dbm */
UCHAR PonTransOpticalPower_Buffer_Index[OPT_POWER_PON_MAX_NUM];
UCHAR PonTransOpticalPower_Buffer_Flag[OPT_POWER_PON_MAX_NUM];

LONG PonTemperature_Buffer[OPT_POWER_PON_MAX_NUM][READ_I2C_COUNT];	                      /*模块温度, unit C */
UCHAR PonTemperature_Buffer_Index[OPT_POWER_PON_MAX_NUM];
UCHAR PonTemperature_Buffer_Flag[OPT_POWER_PON_MAX_NUM];

LONG PonVoltageApplied_Buffer[OPT_POWER_PON_MAX_NUM][READ_I2C_COUNT];	          /*模块电压,unit 0.1V */
UCHAR PonVoltageApplied_Buffer_Index[OPT_POWER_PON_MAX_NUM];	
UCHAR PonVoltageApplied_Buffer_Flag[OPT_POWER_PON_MAX_NUM];	

LONG PonBiasCurrent_Buffer[OPT_POWER_PON_MAX_NUM][READ_I2C_COUNT];	              /*偏置电流,unit 1mA */
UCHAR PonBiasCurrent_Buffer_Index[OPT_POWER_PON_MAX_NUM];	  
UCHAR PonBiasCurrent_Buffer_Flag[OPT_POWER_PON_MAX_NUM];	

#define MAXUPLINKPORT 8   /*问题单13257  是否可以这样设置 ?????? */
#define SystemMaxUplinkPortNum 20*MAXUPLINKPORT
LONG UplinkTransOpticalPower_Buffer[SystemMaxUplinkPortNum][READ_I2C_COUNT];	                /*发送光功率, unit 0.1dbm */
UCHAR UplinkTransOpticalPower_Buffer_Index[SystemMaxUplinkPortNum];
UCHAR UplinkTransOpticalPower_Buffer_Flag[SystemMaxUplinkPortNum];

LONG UplinkRecvOpticalPower_Buffer[SystemMaxUplinkPortNum][READ_I2C_COUNT];	             /*接收光功率，unit 0.1dbm */
UCHAR UplinkRecvOpticalPower_Buffer_Index[SystemMaxUplinkPortNum];
UCHAR UplinkRecvOpticalPower_Buffer_Flag[SystemMaxUplinkPortNum];

LONG UplinkTemperature_Buffer[SystemMaxUplinkPortNum][READ_I2C_COUNT];	                      /*模块温度, unit C */
UCHAR UplinkTemperature_Buffer_Index[SystemMaxUplinkPortNum];
UCHAR UplinkTemperature_Buffer_Flag[SystemMaxUplinkPortNum];

LONG UplinkVoltageApplied_Buffer[SystemMaxUplinkPortNum][READ_I2C_COUNT];	          /*模块电压,unit 0.1V */
UCHAR UplinkVoltageApplied_Buffer_Index[SystemMaxUplinkPortNum];	
UCHAR UplinkVoltageApplied_Buffer_Flag[SystemMaxUplinkPortNum];	

LONG UplinkBiasCurrent_Buffer[SystemMaxUplinkPortNum][READ_I2C_COUNT];	              /*偏置电流,unit 1mA */
UCHAR UplinkBiasCurrent_Buffer_Index[SystemMaxUplinkPortNum];	  
UCHAR UplinkBiasCurrent_Buffer_Flag[SystemMaxUplinkPortNum];	

UplinkPortMeteringInfo_S UplinkPortMeteringInfo[SystemMaxUplinkPortNum];

/*
SYS_CHASSIS_SWITCH_SLOTNUM      在6900上大小为14 
*/
UCHAR Sfp_Uplink_Online_Flag[ SystemMaxUplinkPortNum ];
UCHAR Sfp_Pon_Online_Flag[OPT_POWER_PON_MAX_NUM]; 
	/* 暂时先设置的比较大一点，其实6900上只在PON 上实现，对6700 也没有这么大 */

typedef enum{
	PON_SFP_TYPE_DEFAULT =0,
	PON_SFP_TYPE_HISENSE,
	PON_SFP_TYPE_WTD,
}PonSFPType;

#define  SFP_TYPE_Vendor_len  10
typedef struct sfp_type{
	UCHAR defaultType;
	char type_name[SFP_TYPE_Vendor_len+1];
	struct sfp_type *pNext;
}sfp_type_vendor;
sfp_type_vendor *HeadSfpType=NULL;

#define OnuLaser_AlwaysOn_Alarm_Threshold_Default		(-280)
#define OnuLaser_AlwaysOn_Alarm_TimeCounter_Default	(3)
#define OnuLaser_AlwaysOn_Clear_TimeCounter_Default	(5)

LONG  onuLaser_alwaysOn_alarm_threshold/* = OnuLaser_AlwaysOn_Alarm_Threshold_Default */;
UCHAR onuLaser_alwaysOn_alarm_flag[OPT_POWER_PON_MAX_NUM][2];
LONG onuLaser_alwaysOn_alarm_record[OPT_POWER_PON_MAX_NUM][2];
ULONG onuLaser_alwaysOn_alarm_timeCounter/* = OnuLaser_AlwaysOn_Alarm_TimeCounter_Default*/;
ULONG onuLaser_alwaysOn_clear_timeCounter/* = OnuLaser_AlwaysOn_Clear_TimeCounter_Default*/;
UCHAR onuLaser_alwaysOn_check_support[OPT_POWER_PON_MAX_NUM];

LONG onuLaser_alwaysOn_Enable = V2R1_DISABLE;

LONG onu_OpticalPower_Enable = V2R1_DISABLE;

typedef enum
{
	ALWAYSON_NO_ALARM =0,
	ALWAYSON_GENERATION_ALARM = 1,
	ALWAYSON_CLEAR_ALARM  = 2
} AlwaysOn_Alarm_Type_t;

typedef enum
{
	Less_Or_Equal_AlwaysOn_Alarm_Threshold = 0,
	Bigger_Than_AlwaysOn_Alarm_Threshold = 1
}Comp_With_AlwaysOn_Alarm_Threshold;

LONG PonAlwaysOnTest = 0;

typedef struct RPC_Optical_MsgHead
{
    USHORT      usSrcSlot;
    USHORT      usDstSlot;
    ULONG        ulSrcModuleID;
    ULONG        ulDstModuleID;
    USHORT      usMsgMode;
    USHORT      usMsgType;
    ULONG	 ulCmdID;
    USHORT 	 ulCmdType;
    USHORT      ulDstPort;
    ULONG        ResResult;
    LONG	 	 parameter[5];
    ULONG	 pSendBufLen;
    UCHAR	*pSendBuf;
}RPC_Optical_MsgHead_S;


typedef struct RPC_Optical_Msg_ResHead
{
    USHORT      usSrcSlot;
    USHORT      usDstSlot;
    ULONG        ulSrcModuleID;
    ULONG        ulDstModuleID;
    USHORT      usMsgMode;
    USHORT      usMsgType;
    ULONG	 ulCmdID;
    ULONG 	 ulCmdType;
    ULONG        ResResult;
    ULONG	 pSendBufLen;
    UCHAR	*pSendBuf;
}RPC_Optical_Msg_ResHead_S;

typedef struct Optical_Power_Rx_History
{
	USHORT OnuIdx;
	USHORT NotExistFlag;
	LONG Recv_Buffer[READ_I2C_COUNT];
}Optical_Power_Rx_History_t;

typedef struct Optical_Power_Rx_Instant
{
	ULONG OnuIdx;
	USHORT NotExistFlag;
	USHORT OnuStatus;
	LONG Recv_Instant_Buffer;
}Optical_Power_Rx_Instant_t;

ULONG RPC_Optical_CmdIDNumber = 0;

/* usMsgMode */
#define OPTICALTOLIC_REQACK             0x0001              /*本消息需要应答*/
#define OPTICALTOLIC_ACK                    0x0002              /*应答消息, 操作成功*/
#define OPTICALTOLIC_NAK                    0x0003              /*应答消息, 操作失败*/
#ifdef _DISTRIBUTE_PLATFORM_
#define OPTICALTOLIC_ACK_END		0x0004
#endif

/* usMsgType */
#define OPTICALTOLIC_EXECUTE_CMD        0x0001          /* 主控板通知lic执行相应的命令 */
#ifdef _DISTRIBUTE_PLATFORM_
#define OPTICALTOLIC_EXECUTE_CMD_CONT		0x0002
#endif


typedef struct{
	USHORT PonId;
	USHORT laseralwaysonsupport;
	LONG transOpticalPower;	/*发送光功率, unit 0.1dbm */
	LONG ponTemperature;	/*模块温度, unit C */
	LONG ponVoltageApplied;	/*模块电压,unit 0.1V */
	LONG ponBiasCurrent;	/*偏置电流,unit 1mA */
	int 	  PonOnlineFlag;
	/*ULONG Counter;*/
	ULONG  sfpMeterSupport;
	LONG OnuLaserData; 
	ULONG  pSendBufLen;
	USHORT txAlarmStatus;
	UCHAR rxAlarmStatus[MAXONUPERPONNOLIMIT];
	/*UCHAR *pSendBuf;*/  /*deleted by duzhk 2011.2.16*/
} OpticalToPonMsgHead_t;

typedef struct{
	ULONG OnuId;
	long TransOpticalPower;	/*发送光功率*/
	long PonTemperature;	/*模块温度*/
	long PonVoltageApplied;	/*模块电压*/
	long PonBiasCurrent;	/*偏置电流*/
	long OnuRecvOpticalPower;	/*接收光功率*/
	long OltRecvOpticalPower;
	UCHAR recvPowerFlag;
	UCHAR onu_power_support_flag;
	USHORT AlarmStatus;
}Optical_Msg_PerOnu_t;

UCHAR Onu_OpticalInfo_Change_Flag[OPT_POWER_PON_MAX_NUM][MAXONUPERPONNOLIMIT];
UCHAR Onu_OpticalInfo_Support_Change_Flag[OPT_POWER_PON_MAX_NUM][MAXONUPERPONNOLIMIT];


union byte_short_selfdef{
		unsigned char sbyte[2];
		unsigned short int sshort;
};

#define FAST	register
#define LONG_MAX2	2147483647L
#define LONG_MIN2	(-2147483647L-1L)

long strtol_2
    (
    const char * nptr,		/* string to convert */
    /*char **      endptr,*/	/* ptr to final string */
    FAST int     base,		/* radix */
    long *value
    )
{
    FAST const   char *s = nptr;
    FAST ulong_t acc;
    FAST int 	 c;
    FAST ulong_t cutoff;
    FAST int     neg = 0;
    FAST int     any;
    FAST int     cutlim;
    FAST int     i=0, num=0;

    num = VOS_StrLen(nptr);

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do 
        {
    	c = *s++;
        } while (isspace (c));

    if (c == '-') 
        {
    	neg = 1;
    	c = *s++;
        } 
    else if (c == '+')
    	c = *s++;

    if (((base == 0) || (base == 16)) &&
        (c == '0') && 
        ((*s == 'x') || (*s == 'X'))) 
        {
    	c = s[1];
    	s += 2;
    	base = 16;
        }

    if (base == 0)
    	base = (c == '0' ? 8 : 10);

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */

    cutoff = (neg ? -(ulong_t) LONG_MIN2 : LONG_MAX2);
    cutlim = cutoff % (ulong_t) base;
    cutoff /= (ulong_t) base;

    for (acc = 0, any = 0, i=1;; c = *s++, i++) 
        {
    	if (isdigit (c))
    	    c -= '0';
    	else if (isalpha (c))
    	    c -= (isupper(c) ? 'A' - 10 : 'a' - 10);
    	else
    	{
    	    if(i <= num-neg && c != '.' )
		any = -1;
    	    break;
    	}
		
    	if (c >= base)
    	{
    	    if(i <= num-neg)
		any = -1;
    	    break;
    	}
		
    	if ((any < 0) || (acc > cutoff) || (acc == cutoff) && (c > cutlim))
    	    any = -1;
    	else 
            {
    	    any = 1;
    	    acc *= base;
    	    acc += c;
    	    }
        }

    if (any < 0) 
    {
    	acc = (neg ? LONG_MIN2 : LONG_MAX2);
	*value = acc ;
	return VOS_ERROR;
    	/*errno = ERANGE;*/
    } 
    else if (neg)
    	acc = -acc;

    /*if (endptr != 0)
    	*endptr = (any ? (char *) (s - 1) : (char *) nptr);*/

    *value = acc ;
    return VOS_OK;
    /*return (acc);*/
}

long atol_2
    (
    const register char * s,		/* pointer to string */
    long *value
    )
{
    return strtol_2 (s, /*(char **) NULL,*/ 10, value);
}

LONG VOS_AtoL_2(const CHAR *szString, long *value)
{
    return (LONG)atol_2((const char *)szString, value);
}
/*功能:将值分离出整数部分*/
/*缺陷:当val=-10到0时，是想要得到val的整数部分为-0的，即-1(-0.9)到-0.1(0)*
* 但是由于0的特殊性，前面的负号和正号做除法的时候是不能区分的
* 所以在这里做了一个简单的近似:四舍五入*/
int decimal2_integer_part( int val )
{
	if(val <0 && val >-10)
	{
		if( val > -5 )
			return 0;
		else
			return -1;
	}
	return val/10;
}
/*功能:得到小数部分*/
/*注意:小数部分都是正数*/
int decimal2_fraction_part( int val )
{
	int a = val%10;
	
	if(val <0 && val >-10)
	{
		return 0;
	}
	
	return (((a) >= 0) ? (a) : -(a)) ;
}

#define CHECK_LRET3(l) {if(l!=VOS_OK) {sys_console_printf("\nParameter error, please check it !\n"); return l;}}

static int RPC_OpticalPower_Enable(unsigned int PonPortIdx,int enable_mode, int onu_enable_mode, int Flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_enable,PonPortIdx,NULL,0,enable_mode,onu_enable_mode,Flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_Debug_OpticalPower(unsigned int PonPortIdx,long enable_mode, long DebugType)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(debug_optical_power_command,PonPortIdx,NULL,0,enable_mode,DebugType,0,0,0,NULL,0);
		
   	return iRlt;
}
static int RPC_OpticalPower_Interval(unsigned int PonPortIdx,int interval)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_interval,PonPortIdx,NULL,0,interval,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_StpType_Add(unsigned int PonPortIdx,UCHAR *String_Name)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(pon_adc_negative_polarity_stp_type,PonPortIdx,String_Name,SFP_TYPE_Vendor_len,0,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_StpType_Del(unsigned int PonPortIdx,UCHAR *String_Name)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(undo_pon_adc_negative_polarity_stp_type,PonPortIdx,String_Name,SFP_TYPE_Vendor_len,0,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Calibration_Olt(unsigned int PonPortIdx,long rx_calibration, long tx_calibration)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_calibration_olt,PonPortIdx,NULL,0,rx_calibration,tx_calibration,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Calibration_Onu(unsigned int PonPortIdx,long rx_calibration, long tx_calibration)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_calibration_onu,PonPortIdx,NULL,0,rx_calibration,tx_calibration,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Deadzone(unsigned int PonPortIdx,long power_deadzone,
	long temperature_deadzone,long supply_voltage_deadzone,long bias_current_zone)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_threshold_deadzone,PonPortIdx,NULL,0,
		power_deadzone,temperature_deadzone,supply_voltage_deadzone,bias_current_zone,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_alarm_threshold_olt(unsigned int PonPortIdx,long tx_power_low,
	long tx_power_high,long rx_power_low, long rx_power_high,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_alarm_threshold_olt,PonPortIdx,NULL,0,
		tx_power_low,tx_power_high,rx_power_low,rx_power_high,(ULONG)flag,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_alarm_threshold_Uplink(unsigned int PonPortIdx,long tx_power_low,
	long tx_power_high,long rx_power_low, long rx_power_high,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_alarm_threshold_uplink,PonPortIdx,NULL,0,
		tx_power_low,tx_power_high,rx_power_low,rx_power_high,(ULONG)flag,NULL,0);
		
   	return iRlt;
}


static int RPC_OpticalPower_alarm_threshold_onu(unsigned int PonPortIdx,long tx_power_low,
	long tx_power_high,long rx_power_low, long rx_power_high)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_power_alarm_threshold_onu,PonPortIdx,NULL,0,
		tx_power_low,tx_power_high,rx_power_low,rx_power_high,0,NULL,0);
		
   	return iRlt;
}



static int RPC_OpticalPower_BiasCurrent_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_bias_current_alarm_threshold_olt,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_BiasCurrent_Threshold_Uplink(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_bias_current_alarm_threshold_uplink,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_BiasCurrent_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_bias_current_alarm_threshold_onu,PonPortIdx,NULL,0,low_val,high_val,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Temperature_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_temperature_alarm_threshold_olt,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Temperature_Threshold_Uplink(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_temperature_alarm_threshold_uplink,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Temperature_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_temperature_alarm_threshold_onu,PonPortIdx,NULL,0,low_val,high_val,0,0,0,NULL,0);
		
   	return iRlt;
}


static int RPC_OpticalPower_Voltage_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_voltage_alarm_threshold_onu,PonPortIdx,NULL,0,low_val,high_val,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Voltage_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_voltage_alarm_threshold_olt,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OpticalPower_Voltage_Threshold_Uplnk(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(optical_voltage_alarm_threshold_uplink,PonPortIdx,NULL,0,low_val,high_val,(ULONG)flag,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OnuLaserAlwaysOn_Enable(unsigned int PonPortIdx,int enable_mode)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(onu_laser_always_on_enable,PonPortIdx,NULL,0,enable_mode,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OnuLaserAlwaysOn_AlarmTimes(unsigned int PonPortIdx,int alarm_times)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(onu_laser_always_on_alarm_times,PonPortIdx,NULL,0,alarm_times,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OnuLaserAlwaysOn_ClearTimes(unsigned int PonPortIdx,int clear_times)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(onu_laser_always_on_alarm_clear_times,PonPortIdx,NULL,0,clear_times,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_OnuLaserAlwaysOn_AlarmThreshold(unsigned int PonPortIdx,int alarm_threshold)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(onu_laser_always_on_alarm_threshold,PonPortIdx,NULL,0,alarm_threshold,0,0,0,0,NULL,0);
		
   	return iRlt;
}

static int RPC_Optical_Power_Instant(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length, void *pBuf/*void *pointer*/)
{
	int iRlt,OnuNum,pBufLen;
	unsigned int PonPortIdx;
       /*UCHAR *pBuf;*/

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	OnuNum = length/sizeof(ULONG);
	pBufLen = OnuNum*sizeof(Optical_Power_Rx_Instant_t);
	/*pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero( pBuf, pBufLen);*/
	
	iRlt = OpticalPower_PRC_CALL(show_optical_power_olt_rx_instant,PonPortIdx,(UCHAR *)onuList,length,ulSlot,ulPort,0,0,0,pBuf,pBufLen);

	/*if(VOS_OK == iRlt)
	{
		Show_OpticalPower_OltRx_Instant_FromPon( ulSlot, ulPort, pBuf, pBufLen,(struct vty *)pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}
/*olt接收光功率的值，当前值和历史值(存历史的5次)*/
static int get_optical_power_meter_instant(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length, void *pBuf  )
{
	short int PonPortIdx, OnuIdx;
	short int j, Llid,OnuStatus;
	long val,ret =-1;
	Optical_Power_Rx_Instant_t *recv_instant, *recv_instant2;
       int number = 0;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		sys_console_printf("  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
	}
	
	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		/*sys_console_printf("can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );*/
		return VOS_ERROR;
	}
	number = length/sizeof(ULONG);
	recv_instant = (Optical_Power_Rx_Instant_t *)pBuf;
	for(j = 0 ; j < number ; j++)
 	{
		OnuIdx = onuList[j] -1;
		if(OnuIdx >= MAXONUPERPON) 
		{
			sys_console_printf("Onu %d must be less than %d!\r\n",OnuIdx+1,MAXONUPERPON+1);
			continue;
		}
		recv_instant2 = recv_instant;
		recv_instant->OnuIdx = OnuIdx;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
			/*sys_console_printf(" onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuId);*/
			recv_instant->OnuIdx = OnuIdx;
			recv_instant->NotExistFlag = 1;
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}
		OnuStatus = GetOnuOperStatus(PonPortIdx, OnuIdx) ;
		recv_instant->OnuIdx = OnuIdx;
		recv_instant->OnuStatus = OnuStatus;
		if(OnuStatus != ONU_OPER_STATUS_UP)
		{
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}
		Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if(Llid == INVALID_LLID)
		{
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}
		/*上面的都是判断ONU是否正常工作，接下来才要去根据索引读接收的光功率值*/
		ret = ReadPonPortRecevPower(PonPortIdx,OnuIdx,0,&val);
		if(ret == VOS_ERROR)
			recv_instant->Recv_Instant_Buffer = -1000; 
		else
			recv_instant->Recv_Instant_Buffer = val; 
		
		recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
	}
		
	return VOS_OK;
}

static int get_optical_power_meter_history(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length, void *pBuf  )
{
	short int PonPortIdx, OnuIdx;
	short int i,j, k/*, ulOnuId*/;
       int number = 0;
	Optical_Power_Rx_History_t *recv_history, *recv_history2;

	/*if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	
	{
		sys_console_printf(" %% slot %d is not inserted\r\n", ulSlot);
		return( RERROR );
	}
	
	if(SlotCardIsPonBoard(ulSlot) != ROK )	
	{
		sys_console_printf(" %% slot %d is not pon card\r\n", ulSlot);
		return( RERROR );
	}

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)
	{
		sys_console_printf("  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(RERROR);
	}*/

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		sys_console_printf("  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
	}
	/* 对历史数据没必要判断这个条件
	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		sys_console_printf("can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
		return VOS_ERROR;
	}*/
	number = length/sizeof(ULONG);
	recv_history = (Optical_Power_Rx_History_t *)pBuf;
	for(j = 0 ; j < number ; j++)
 	{
		OnuIdx = onuList[j] -1;
		if(OnuIdx >= MAXONUPERPON) 
		{
			sys_console_printf("Onu %d must be less than %d!\r\n",OnuIdx+1,MAXONUPERPON+1);
			continue;
		}	
		recv_history2 = recv_history;
		recv_history->OnuIdx = OnuIdx;
		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
		{
			/*sys_console_printf(" onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, OnuIdx+1);*/
			recv_history->OnuIdx = OnuIdx;
			recv_history->NotExistFlag = 1;
			recv_history = (Optical_Power_Rx_History_t *)(recv_history2+1);
			continue;
		}

		k = PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx];
		
		for(i=0;i<READ_I2C_COUNT;i++)
		{
			if( k >=READ_I2C_COUNT)
				k = 0;
			recv_history->Recv_Buffer[i] = PonRecvOpticalPower_Buffer[PonPortIdx][OnuIdx][k];
			k++;
		}
		recv_history = (Optical_Power_Rx_History_t *)(recv_history2+1);
	}
		
	return VOS_OK;
}

static int RPC_Optical_Power_History(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length, void *pBuf/*void *pointer*/)
{
	int iRlt,OnuNum,pBufLen;
	unsigned int PonPortIdx;
       /*UCHAR *pBuf;*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	OnuNum = length/sizeof(ULONG);
	pBufLen = OnuNum*sizeof(Optical_Power_Rx_History_t);
	/*pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero( pBuf, pBufLen);*/
	
	iRlt = OpticalPower_PRC_CALL(show_optical_power_olt_rx_history,PonPortIdx,(UCHAR *)onuList,length,ulSlot,ulPort,0,0,0,pBuf,pBufLen);

	/*if(VOS_OK == iRlt)
	{
		Show_OpticalPower_OltRx_History_FromPon( ulSlot, ulPort, pBuf, pBufLen,(struct vty *)pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}

static int RPC_OnuLaser_AlwaysOn(ULONG ulSlot, ULONG ulPort, UCHAR *TypeFlag, void *pBuf/*void *pointer*/)
{
	int iRlt,pBufLen;
	unsigned int PonPortIdx;
       /*UCHAR *pBuf;*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	pBufLen = PONPORTPERCARD*sizeof(LONG);
	/*pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);*/
	
	iRlt = OpticalPower_PRC_CALL(show_onu_laser_always_on,PonPortIdx,TypeFlag,6,ulSlot,ulPort,0,0,0,pBuf,pBufLen);
	
	/*if(VOS_OK == iRlt)
	{
		Show_OpticalPower_OnuLaser_FromPon( ulSlot, ulPort, pBuf, pBufLen,TypeFlag,(struct vty *)pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}

static int get_Onu_laser_Always_on_Value(ULONG ulSlot, ULONG ulPort, UCHAR *TypeFlag, void*pBuf  )
{
	int i;
	LONG record_t[13*16];/*4 means PONPORTPERCARD; This variable should be bigger then MAXPON . */
	VOS_MemZero(record_t, MAXPON*sizeof(LONG));
	if(VOS_MemCmp(TypeFlag,"status",  6) == 0)
	{
		for(i =0;i<MAXPON;i++ )
		{
			record_t[i] = onuLaser_alwaysOn_alarm_record[i][0];
		}
	}
	else
	{
		for(i =0;i<MAXPON;i++ )
		{
			record_t[i] = onuLaser_alwaysOn_alarm_record[i][0];
		}
	}

	VOS_MemCpy(pBuf, (UCHAR *)record_t, MAXPON*sizeof(LONG));
	
	return VOS_OK;
}


LONG Fetch_OnuLaser_AlwaysOn(ULONG ulSlot, ULONG ulPort,UCHAR *TypeFlag,void *pointer)
{
	int iRlt, olt_id;
	unsigned int PonPortIdx;

	if(ulSlot == 0 && ulPort == 0)
		PonPortIdx = OPTICAL_PONPORTIDX_ALL;
	else
	{
		PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

		if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
			return RERROR;
	}

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx /*||SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/ )
	{   
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_onu_laser_always_on_t, (ulSlot, ulPort, TypeFlag, pointer) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
		 	ulSlot = GetCardIdxByPonChip(olt_id);
			ulPort = GetPonPortByPonChip(olt_id);
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id /*PonPortIdx*/, show_onu_laser_always_on_t, (ulSlot, ulPort, TypeFlag, (void *)((LONG *)pointer+olt_id)) )))
			{
				break;
			}
		}
	}

	return iRlt;

}

static int RPC_Online_Sfp_Pon(ULONG ulSlot, ULONG ulPort, void *pBuf/*void *pointer*/,USHORT pBufLen,struct vty *vty)
{
	int iRlt/*,i*/;
	unsigned int PonPortIdx;
       /*_XCVR_DATA_  *pXcvrArr;*/
	UCHAR *buffer;

	buffer = pBuf;
	/*pXcvrArr = &XcvrInfoArr[0];*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	/*pBufLen = 16*sizeof(UCHAR)*(pXcvrArr[16].cLen+2)+1;*/
	iRlt = OpticalPower_PRC_CALL(show_online_sfp_pon,PonPortIdx,NULL,0,ulSlot,ulPort,0,0,0,pBuf,pBufLen);
	/*if(VOS_OK == iRlt)
	{
		Show_OpticalPower_OnuLaser_FromPon( ulSlot, ulPort, pBuf, pBufLen,TypeFlag,(struct vty *)pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}

static int get_Online_Sfp_Pon(ULONG ulSlot, ULONG ulPort, void*pBuf, USHORT pBufLen, struct vty *vty )
{
	PON_gpio_line_io_t  direction;
	bool value = 0;
	short int PonPortIdx = 0, i = 0 ;
	ULONG ulDevAddr;
	UCHAR GetlDevAddr[32],*buffer;
	int j,k,Ret,times, Flag = 0;
	_XCVR_DATA_  *pXcvrArr;
	union byte_short_selfdef GetAddr;
	Online_Sfp_Info_Get_t *Sfp_Info;
	long lRet, Gfa6100_not_online = 0 ;
	
	ulDevAddr = A0H_1010000X;
	pXcvrArr = &XcvrInfoArr[0];
	buffer = pBuf;
	Sfp_Info = (Online_Sfp_Info_Get_t *)(buffer+1);

	buffer[0] = MAXPON;
	
	for(i = 0; i< MAXPON; i++)
	{		
		if( i + 1 < ulPort )
		{
			Sfp_Info = Sfp_Info+1;
			continue;
		}
			
		VOS_MemZero(GetlDevAddr,32);

		if(i >= PONPORTPERCARD)
		{
			buffer[0] = PONPORTPERCARD;
			return VOS_OK;
		}

		PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), i+1);

		CHECK_PON_RANGE

		Gfa6100_not_online = 0;
		if( SYS_LOCAL_MODULE_TYPE_IS_SFP_FIXED_PON )
		{
			lRet = 0;
			value = 0;
		}
		else
     		{
#if 0
    		/* for 10G EPON of PMC8411 by jinhl @2012-11-12 */
			lRet = OLT_GpioAccess(PonPortIdx, PON_GPIO_LINE_3, PON_GPIO_LINE_INPUT, 0,   /*在前一个参数为PON_GPIO_LINE_INPUT 的情况下此参数无效*/ 
	 								&direction, &value );
#else
			if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
			{
				lRet = GetGponSfpOnlineState(PonPortIdx, &value);
			}
			else
			{
	         	 	lRet = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &value);
			}

#endif
		}      

		if( SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			if(value == 0)
			{
				Ret = i2c_data_get(ulSlot, i, pXcvrArr[13].cAddr, GetlDevAddr, pXcvrArr[13].cLen);
				if(Ret != 0)
				{
					VOS_MemCpy(Sfp_Info->Vendorname, GetlDevAddr, pXcvrArr[13].cLen);
					Sfp_Info->NameEnd = '\0';
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Ret = i2c_data_get(ulSlot, i, pXcvrArr[16].cAddr, GetlDevAddr, pXcvrArr[16].cLen);
				if(Ret != 0)
				{
					VOS_MemCpy(Sfp_Info->VendorPN, GetlDevAddr, pXcvrArr[16].cLen);
					Sfp_Info->PnEend = '\0';
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Ret = i2c_data_get(ulSlot, i, pXcvrArr[18].cAddr, GetlDevAddr, pXcvrArr[18].cLen);
				if(Ret != 0)
				{
					VOS_MemCpy(Sfp_Info->Wavelength, GetlDevAddr, pXcvrArr[18].cLen);
					Sfp_Info->WaveEnd= '\0';
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Ret = i2c_data_get(ulSlot, i, pXcvrArr[24].cAddr, GetlDevAddr, pXcvrArr[24].cLen);
				if(Ret != 0)
				{
					VOS_MemCpy(Sfp_Info->VendorSN, GetlDevAddr, pXcvrArr[24].cLen);
					Sfp_Info->SnEnd = '\0';
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Ret = i2c_data_get(ulSlot, i, pXcvrArr[25].cAddr, GetlDevAddr, pXcvrArr[25].cLen);
				if(Ret != 0)
				{
					VOS_MemCpy(Sfp_Info->DataCode, GetlDevAddr, pXcvrArr[25].cLen);
					Sfp_Info->DataEnd = '\0';
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Ret = i2c_data_get(ulSlot, i, pXcvrArr[26].cAddr, GetlDevAddr, pXcvrArr[26].cLen);
				if(Ret != 0)
				{
					Sfp_Info->Diagnostic_Monitoring_Type=GetlDevAddr[0];
						
					VOS_MemZero(GetlDevAddr,32);
				}
				else
				{
					Sfp_Info->Online = 0;	
					Sfp_Info = Sfp_Info+1;
					continue;
				}

				Sfp_Info->Online = 1;

				if(VOS_OK == CheckPonSFPVendorSpecificIsGwd(PonPortIdx))
					Sfp_Info->OpticalPower = 1;
				else 
					Sfp_Info->OpticalPower = 0;
			}
		}
		else
		{
			if( 0 == lRet )
			{
			 	if(value == 0)/*低电平*/
			 	{
			 		k =0;
					for(j=0; j < pXcvrArr[13].cLen ; )
					{
					    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[13].cAddr+j, &(GetAddr.sshort));
						if( (0 != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( SYS_LOCAL_MODULE_TYPE_IS_SFP_FIXED_PON )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
						
							times++;
							if(times>6)
							{
#if 0
							if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
							{
								Flag = 1;
								times = 0;
								break;
							}
							else
							{
								j++;
								times = 0;
							}
#else
							Flag = 1;
							j++;
							times = 0;
							Sfp_Info->Online = 0;
							Sfp_Info = Sfp_Info+1;
							break;
#endif
							}
						}
						else
						{
							j++;
							GetlDevAddr[k] = GetAddr.sbyte[1];
							k++;
							if(k>pXcvrArr[13].cLen)
								break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}

					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					VOS_MemCpy(Sfp_Info->Vendorname, GetlDevAddr, pXcvrArr[13].cLen);
					Sfp_Info->NameEnd = '\0';
					
					VOS_MemZero(GetlDevAddr,32);

					k =0;
					for(j=0; j < pXcvrArr[16].cLen ; )
					{
					    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[16].cAddr+j, &(GetAddr.sshort));
						if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( SYS_LOCAL_MODULE_TYPE_IS_SFP_FIXED_PON )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
							
							times++;
							if(times>6)
							{
								j++;
								times = 0;
								Sfp_Info->Online = 0;
								Sfp_Info = Sfp_Info+1;
								Flag = 1;
								break;
							}
						}
						else
						{
							j++;
							GetlDevAddr[k] = GetAddr.sbyte[1];
							k++;
							if(k>pXcvrArr[16].cLen)
								break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}
					
					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					VOS_MemCpy(Sfp_Info->VendorPN, GetlDevAddr, pXcvrArr[16].cLen);
					Sfp_Info->PnEend = '\0';
					
					VOS_MemZero(GetlDevAddr,32);
	                                k =0;
					for(j=0; j < pXcvrArr[18].cLen ; )
					{			
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[18].cAddr+j, &(GetAddr.sshort));
						if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
							
							times++;
							if(times>6)
							{
								j++;
								times = 0;
								Sfp_Info->Online = 0;
								Sfp_Info = Sfp_Info+1;
								Flag = 1;
								break;
							}
						}
						else
						{
							j++;
							GetlDevAddr[k] = GetAddr.sbyte[1];
							k++;
							if(k>pXcvrArr[18].cLen)
								break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}
					
					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					VOS_MemCpy(Sfp_Info->Wavelength, GetlDevAddr, pXcvrArr[18].cLen);
					Sfp_Info->WaveEnd= '\0';
					
					VOS_MemZero(GetlDevAddr,32);
					k =0;
					for(j=0; j < pXcvrArr[24].cLen ; )
					{
					    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[24].cAddr+j, &(GetAddr.sshort));
						if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
							
							times++;
							if(times>6)
							{
								j++;
								times = 0;
								Sfp_Info->Online = 0;
								Sfp_Info = Sfp_Info+1;
								Flag=1;
								break;
							}
						}
						else
						{
							j++;
							GetlDevAddr[k] = GetAddr.sbyte[1];
							k++;
							if(k>pXcvrArr[24].cLen)
								break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}
					
					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					VOS_MemCpy(Sfp_Info->VendorSN, GetlDevAddr, pXcvrArr[24].cLen);
					Sfp_Info->SnEnd = '\0';
					VOS_MemZero(GetlDevAddr,32);

					k =0;
					for(j=0; j < pXcvrArr[25].cLen ; )
					{
					    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[25].cAddr+j, &(GetAddr.sshort));
						if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
							
							times++;
							if(times>6)
							{
								j++;
								times = 0;
								Sfp_Info->Online = 0;
								Sfp_Info = Sfp_Info+1;
								Flag=1;
								break;
							}
						}
						else
						{
							j++;
							GetlDevAddr[k] = GetAddr.sbyte[1];
							k++;
							if(k>pXcvrArr[25].cLen)
								break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}
					
					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					VOS_MemCpy(Sfp_Info->DataCode, GetlDevAddr, pXcvrArr[25].cLen);
					Sfp_Info->DataEnd = '\0';

					VOS_MemZero(GetlDevAddr,32);


					while(1)
					{
						Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[26].cAddr, &(GetAddr.sshort));
						if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
						{
							if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
							{
								Sfp_Info->Online= 0;
								Gfa6100_not_online = 1;
								break;
							}
							
							times++;
							if(times>6)
							{
								times = 0;
								Sfp_Info->Online = 0;
								Sfp_Info = Sfp_Info+1;
								Flag=1;
								break;
							}
						}
						else
						{
							GetlDevAddr[0] = GetAddr.sbyte[1];
							break;
						}
					}

					if( 1 == Gfa6100_not_online )
					{
						Sfp_Info = Sfp_Info+1;
						continue;
					}
					
					if(Flag == 1)
					{
						Flag = 0;
						continue;
					}
					
					times=0;
					Sfp_Info->Online = 1;
					Sfp_Info->Diagnostic_Monitoring_Type=GetlDevAddr[0];
					VOS_MemZero(GetlDevAddr,32);

					if(VOS_OK == CheckPonSFPVendorSpecificIsGwd(PonPortIdx))
						Sfp_Info->OpticalPower = 1;
					else 
						Sfp_Info->OpticalPower = 0;
				}
				else
				{
					Sfp_Info->Online= 0;
				}
			}
		}
		Sfp_Info = Sfp_Info + 1;
		
	}
	
	return VOS_OK;
}

LONG Fetch_Online_Sfp_Pon(ULONG ulSlot, ULONG ulPort,void *pointer,USHORT pBufLen,struct vty *vty)
{
	int iRlt, olt_id, i ;
	unsigned int PonPortIdx;
	_XCVR_DATA_  *pXcvrArr;
	UCHAR *buffer;
	Online_Sfp_Info_Get_t *Sfp_Info;
	UCHAR temp0[16], *temp1, *temp2; 
	buffer = pointer;
	Sfp_Info = (Online_Sfp_Info_Get_t *)(buffer+1);
	pXcvrArr = &XcvrInfoArr[0];

	if(ulSlot == 0 && ulPort == 0)
		PonPortIdx = OPTICAL_PONPORTIDX_ALL;
	else
	{
		PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

		if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
			return RERROR;
	}

	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (!SYS_LOCAL_MODULE_TYPE_IS_8100_PON))
	{
		if( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
		{
			VOS_MemZero(pointer,pBufLen);
			iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_online_sfp_pon_f, (ulSlot, ulPort, pointer,pBufLen,vty) );
			if(iRlt == VOS_OK)
			{
				if(vty != NULL)
				{
					for(i=0; i<buffer[0];i++)
					{	
						if(Sfp_Info->Online != 0)
						{

							VOS_Sprintf(temp0,"%d/%d",ulSlot,i+1);
							
							if(Sfp_Info->OpticalPower == 1)
								temp1 = "Support ";
							else
								temp1 = "Not Support ";
							
							if(FALSE == HasOnuOnlineOrRegistering(GetPonPortIdxBySlot((short int)(ulSlot), (short int)(i+1))))
								temp2 = "not working ";
							else
								temp2 = "working ";
							
							vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info->Vendorname,
									Sfp_Info->VendorPN, Sfp_Info->VendorSN,Sfp_Info->Diagnostic_Monitoring_Type,Sfp_Info->Wavelength[0]*256+Sfp_Info->Wavelength[1],Sfp_Info->DataCode, temp1, temp2);
						}
						Sfp_Info = Sfp_Info+1;
					}
				}
			}
		}
		else
		{
			for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
			{
			 	ulSlot = GetCardIdxByPonChip(olt_id);
				ulPort = GetPonPortByPonChip(olt_id);
				VOS_MemZero(pointer,pBufLen);
				
				if ( !SYS_MODULE_IS_READY(ulSlot) )
				{
					continue;
				}

				if( RERROR == SlotCardIsPonBoard(ulSlot) )	
				{
					continue;
				}

				if( SYS_MODULE_IS_UPLINK_PON(ulSlot) )
					ulPort = 5;
				else
					ulPort = 1;
			
				if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id /*PonPortIdx*/, show_online_sfp_pon_f, (ulSlot, ulPort, (void *)pointer,pBufLen,vty))))
				{
				}
				else
				{
					Sfp_Info = (Online_Sfp_Info_Get_t *)(buffer+1);
					if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
					{
						if(vty != NULL)
						{
							for(i=0; i<buffer[0];i++)
							{	
								if(Sfp_Info->Online != 0)
								{
									VOS_Sprintf(temp0,"%d/%d",ulSlot,i+1);
									
									if(Sfp_Info->OpticalPower == 1)
										temp1 = "Support ";
									else
										temp1 = "Not Support ";
									
									if(FALSE == HasOnuOnlineOrRegistering(GetPonPortIdxBySlot((short int)(ulSlot), (short int)(i+1))))
										temp2 = "not working ";
									else
										temp2 = "working ";
									
									vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info->Vendorname,
									Sfp_Info->VendorPN, Sfp_Info->VendorSN,Sfp_Info->Diagnostic_Monitoring_Type,Sfp_Info->Wavelength[0]*256+Sfp_Info->Wavelength[1],Sfp_Info->DataCode, temp1, temp2);
								}
								Sfp_Info = Sfp_Info+1;
							}
						}
					}
				}
			}
		}
	}
	else if ( SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
	{
		if( OPTICAL_PONPORTIDX_ALL == PonPortIdx )
		{
			ulSlot = SYS_LOCAL_MODULE_SLOTNO;
		
			if( SYS_MODULE_IS_UPLINK_PON(ulSlot) && !SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
				ulPort = 5;
			else
				ulPort = 1;

			VOS_MemZero(pointer,pBufLen);
			iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_online_sfp_pon_f, (ulSlot, ulPort, pointer,pBufLen,vty) );
			if(iRlt == VOS_OK)
			{
				Sfp_Info = (Online_Sfp_Info_Get_t *)(buffer+1);

				if(vty != NULL)
				{
					for(i=0; i<buffer[0];i++)
					{	
						if(Sfp_Info->Online != 0)
						{
							VOS_Sprintf(temp0,"%d/%d",ulSlot,i+1);
							
							if(Sfp_Info->OpticalPower == 1)
								temp1 = "Support ";
							else
								temp1 = "Not Support ";
							
							if(FALSE == HasOnuOnlineOrRegistering(GetPonPortIdxBySlot((short int)(ulSlot), (short int)(i+1))))
								temp2 = "not working ";
							else
								temp2 = "working ";
							
							vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info->Vendorname,
							Sfp_Info->VendorPN, Sfp_Info->VendorSN,Sfp_Info->Diagnostic_Monitoring_Type,Sfp_Info->Wavelength[0]*256+Sfp_Info->Wavelength[1],Sfp_Info->DataCode, temp1, temp2);
						}
						Sfp_Info = Sfp_Info+1;
					}
				}
				
			}
		}
		else
		{
			if(SYS_LOCAL_MODULE_SLOTNO != ulSlot)
			{
				return VOS_OK;
			}
			else
			{
				VOS_MemZero(pointer,pBufLen);
				iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_online_sfp_pon_f, (ulSlot, ulPort, pointer,pBufLen,vty) );
				if(iRlt == VOS_OK)
				{
					Sfp_Info = (Online_Sfp_Info_Get_t *)(buffer+1);

					if(vty != NULL)
					{
						for(i=0; i<buffer[0];i++)
						{	
							if(Sfp_Info->Online != 0)
							{
								VOS_Sprintf(temp0,"%d/%d",ulSlot,i+1);
								
								if(Sfp_Info->OpticalPower == 1)
									temp1 = "Support ";
								else
									temp1 = "Not Support ";
								
								if(FALSE == HasOnuOnlineOrRegistering(GetPonPortIdxBySlot((short int)(ulSlot), (short int)(i+1))))
									temp2 = "not working ";
								else
									temp2 = "working ";
								
								vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info->Vendorname,
								Sfp_Info->VendorPN, Sfp_Info->VendorSN,Sfp_Info->Diagnostic_Monitoring_Type,Sfp_Info->Wavelength[0]*256+Sfp_Info->Wavelength[1],Sfp_Info->DataCode, temp1, temp2);
							}
							Sfp_Info = Sfp_Info+1;
						}
					}
					
				}
			}
		}
		
	}

	return iRlt;

}

static int RPC_Get_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType)
{
	int iRlt;
	
	/*OPTICALPOWER_ASSERT(PonPortIdx);*/
	
   	iRlt = OpticalPower_PRC_CALL(get_sfp_online_state, PonPortIdx,NULL,0,GetType,0,0,0,0,pBuf,pBufLen );
		
   	return iRlt;
}
/*begin:added by yanjy,2017-2*/
/*专门读取pon口光模块是否在位，在位返回VOS_OK,不在位返回VOS_ERROR*/
/*调用:在检查pon Los和pon Los clear时，需要先检查光模块是否匹配(在检查之前要判断光模块状态)*
*防止在快速插拔光模块时打印异常i2c信息*/
int check_sfp_online_state(unsigned int PonPortIdx)
{
	int ret = 0;
	int value = 0;
	
 	if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
	{
		ret = GetGponSfpOnlineState(PonPortIdx, &value);
	}
	else
	{
     	 	ret = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &value);
	}
		if(ret == 0)
		{
		 	if(value == 0)/*低电平*/
		 	{
				return VOS_OK;
			}
			else
			{
				return VOS_ERROR;
			}
		}	
	return VOS_ERROR;
}
/*end:added by yanjy*/
static int Get_Pon_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType )
{
	PON_gpio_line_io_t  direction;
	bool value = 0;
	OpticalToPonMsgHead_t *pQueueMsg = pBuf ;
	char SFPTypeString[SFP_TYPE_Vendor_len];
	long ret, val;
	
	if( SYS_LOCAL_MODULE_TYPE_IS_SFP_FIXED_PON )
    {
		pQueueMsg->PonOnlineFlag = 0 ;
    }   
	else
	{
#if 0
	    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if(OLT_GpioAccess(PonPortIdx, PON_GPIO_LINE_3, PON_GPIO_LINE_INPUT, 0, /*在前一个参数为PON_GPIO_LINE_INPUT 的情况下此参数无效*/ 
		 								&direction, &value ) == 0 )
#else
     		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			ret = GetGponSfpOnlineState(PonPortIdx, &value);
		}
		else
		{
         	 	ret = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &value);
		}
#endif
		if(ret == 0)
		{
		 	if(value == 0)/*低电平*/
		 	{
				pQueueMsg->PonOnlineFlag = 0; /*State->value = 0;*/
				Sfp_Pon_Online_Flag[PonPortIdx] = 1;
			}
			else
			{
				pQueueMsg->PonOnlineFlag = 1 ;
				return VOS_OK;
			}
		}
	}

	if(!SYS_LOCAL_MODULE_TYPE_IS_8000_GPON && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
	{
		if(ponSFPType_read(PonPortIdx, SFPTypeString) != ROK || CheckPonSFPVendorSpecificIsGwd(PonPortIdx) == VOS_ERROR )
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=0;
			SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"PonPortIdx=%d,read sfp type err,not support rssi\r\n",PonPortIdx));
			return VOS_OK;
		}
	}

	PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=1;
	pQueueMsg->sfpMeterSupport = 1 ;

	ret = ReadPonPortTransPower( PonPortIdx, &val);
	if(ret == VOS_OK)
	{
		if( PonTransOpticalPower_Buffer_Flag[PonPortIdx]==1)
			CheckPonPortTransOpticalPower( PonPortIdx, val );
		PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = val;	/*刷新OLT sfp光模块参数信息表*/
	}
	ret = ReadPonPortTemperature( PonPortIdx, &val);
	if(ret == VOS_OK)
	{
		if( PonTemperature_Buffer_Flag[PonPortIdx]==1)
			CheckPonPortTemperature(PonPortIdx, val );
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature = val;
	}
	
	ret = ReadPonPortBias( PonPortIdx, &val);
	if(ret == VOS_OK)
	{
		if( PonBiasCurrent_Buffer_Flag[PonPortIdx]==1)
			CheckPonPortBiasCurrent( PonPortIdx, val );
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = val;
	}
	
	ret = ReadPonPortVoltage( PonPortIdx, &val );
	if(ret == VOS_OK)
	{
		if( PonVoltageApplied_Buffer_Flag[PonPortIdx]==1)
			CheckPonPortWorkVoltage( PonPortIdx, val );
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = val;
	}

	pQueueMsg->laseralwaysonsupport = onuLaser_alwaysOn_check_support[PonPortIdx];
	pQueueMsg->transOpticalPower = PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower;
	pQueueMsg->ponTemperature = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature;
	pQueueMsg->ponVoltageApplied = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied;
	pQueueMsg->ponBiasCurrent = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent;
		
	return VOS_OK;
	
}

static int Get_Uplink_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType )
{
	int portno;
	UplinkPortMeteringInfo_S *pQueueMsg = pBuf ;
	long ret=0, val = 0;
	
	portno = SYS_LOCAL_MODULE_SLOTNO*MAXUPLINKPORT+PonPortIdx;

	if( Check_Sfp_Online_6100( SYS_LOCAL_MODULE_SLOTNO, PonPortIdx ) == FALSE)
	{
		/*sys_console_printf(" It is not online .\r\n");*/
		return VOS_OK;
	}
	
	pQueueMsg->powerMeteringSupport = UplinkSFPDiagnosticMonitorType(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, 1);
	if( pQueueMsg->powerMeteringSupport == NOT_SUPPORT_SFF8472 )
		return VOS_OK;

	/*if( !checkUplinkPortSFPUpdated( SYS_LOCAL_MODULE_SLOTNO, PonPortIdx) )
		return VOS_OK;*/

	ret =ReadUplinkPortTemperature(SYS_LOCAL_MODULE_SLOTNO,PonPortIdx, &val);  
	if(ret == VOS_OK)
	{
		UplinkPortMeteringInfo[portno].Temperature=val;
		pQueueMsg->Temperature = val;
	}
	
	ret=ReadUplinkPortVoltage(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, &val); 
	if(ret == VOS_OK)
	{
		UplinkPortMeteringInfo[portno].Voltage=val;
		pQueueMsg->Voltage = val ;
	}
	
	ret=ReadUplinkPortBias(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, &val);
	if(ret == VOS_OK)
	{
		UplinkPortMeteringInfo[portno].BiasCurrent= val ;
		pQueueMsg->BiasCurrent = val ;
	}

	ret=ReadUplinkPortTransPower(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, &val ); 
	if(ret == VOS_OK)
	{
		UplinkPortMeteringInfo[portno].transOpticalPower= val ;
		pQueueMsg->transOpticalPower = val ;
	}
	
	ret=ReadUplinkPortRecevPower(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, &val );  
	if(ret == VOS_OK)
	{
		UplinkPortMeteringInfo[portno].recvOpticalPower= val ;
		pQueueMsg->recvOpticalPower = val;
	}
	
	setUplinkPortSFPUpdated(SYS_LOCAL_MODULE_SLOTNO,PonPortIdx);
	
	return VOS_OK;
	
}
 
static int Get_12Epon_Uplink_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType )
{
	int slotno, sfp_sel, portno;
	UCHAR *temp1, *temp2; 
	long ret=0, val = 0;
	unsigned short int data[32];
	UCHAR supportflag = 0, CheckFlag = 1;
	Online_Sfp_Info_Get_t  *Sfp_Info = pBuf ;
	
	slotno = SYS_LOCAL_MODULE_SLOTNO;
	sfp_sel = PonPortIdx;
	portno = SYS_LOCAL_MODULE_SLOTNO*MAXUPLINKPORT+PonPortIdx;
	if( Check_Sfp_Online_12Epon_Uplink( SYS_LOCAL_MODULE_SLOTNO, PonPortIdx) == FALSE )
	{
		Sfp_Info->Online = 0;
		return VOS_ERROR;
	}
	
	Sfp_Info->Online = 1;
				
	ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[13].cAddr, (char *)data,XcvrInfoArr[13].cLen);
	VOS_MemCpy(Sfp_Info->Vendorname, data,XcvrInfoArr[13].cLen);
	Sfp_Info->NameEnd = '\0';
				
	ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[16].cAddr, (char *)data,XcvrInfoArr[16].cLen);
	VOS_MemCpy(Sfp_Info->VendorPN, data,XcvrInfoArr[16].cLen);
	Sfp_Info->PnEend = '\0';

	ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[18].cAddr, (char *)data,XcvrInfoArr[18].cLen);
	VOS_MemCpy(Sfp_Info->Wavelength, data,XcvrInfoArr[18].cLen);
	Sfp_Info->WaveEnd= '\0';
				
	ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[24].cAddr, (char *)data,XcvrInfoArr[24].cLen);
	VOS_MemCpy(Sfp_Info->VendorSN, data,XcvrInfoArr[24].cLen);
	Sfp_Info->SnEnd = '\0';
				
	ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[25].cAddr, (char *)data,XcvrInfoArr[25].cLen);
	VOS_MemCpy(Sfp_Info->DataCode, data,XcvrInfoArr[25].cLen);
	Sfp_Info->DataEnd= '\0';

	ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[26].cAddr, (char *)data,XcvrInfoArr[26].cLen);
	VOS_MemCpy(Sfp_Info->Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
	
	supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
		if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
	if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
		Sfp_Info->Reserved = 1;
	else
		Sfp_Info->Reserved = 0;
						
	return VOS_OK;
	
}
static int Get_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType )
{
	int (* Func )(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType );
	long lRet ;
	if( GetType == 0)
		Func =  Get_Pon_Sfp_Online_State;
	else if( GetType == 1)
		Func = Get_Uplink_Sfp_Online_State;
	else if( GetType == 2)
		Func = Get_12Epon_Uplink_Sfp_Online_State ;
	
	lRet = (*Func)( PonPortIdx, pBuf,  pBufLen,  GetType );
	
	return lRet;
}

LONG Fetch_Sfp_Online_State(unsigned int PonPortIdx, void *pBuf, USHORT pBufLen, int GetType)  /* 0: PON, 1:UPLINK; 2:12EPON show sfp_online uplink dispaly */
{
	int lRlt;

	lRlt = OPTICALPOWER_API_CALL( PonPortIdx, get_sfp_online_state_t, (PonPortIdx, pBuf, pBufLen, GetType ) );

	return lRlt;

}
int GetPonPortSfpState(short int PonPortIdx)
{
    int ret = VOS_ERROR;
	USHORT length = sizeof(OpticalToPonMsgHead_t);
	OpticalToPonMsgHead_t *pQueueMsg = VOS_Malloc(length, MODULE_RPU_PON_MON);
	if(pQueueMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	
	VOS_MemZero(pQueueMsg, length);
	
	ret = Fetch_Sfp_Online_State( PonPortIdx, (VOID *)pQueueMsg, length, 0);
		
	if(VOS_OK== ret) 
	{	
		if( pQueueMsg->PonOnlineFlag == 0) 
            		ret = SFP_ONLINE;
	}
    else
    		ret = VOS_ERROR;
    
    VOS_Free(pQueueMsg);
    return ret;
        
}
static int RPC_optical_power_XcvrInfoArr(short int ulSlot, short int ulPort, UCHAR *TypeFlag, void *pBuf/*void *pointer*/)
{
	int iRlt,pBufLen;
	unsigned int PonPortIdx;
       /*UCHAR *pBuf;*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	pBufLen = 128;
	/*pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);*/
				
	iRlt = OpticalPower_PRC_CALL(show_optical_power_XcvrInfoArr,PonPortIdx, TypeFlag,4,ulSlot,ulPort,0,0,0,pBuf,pBufLen);
	
	/*if(VOS_OK == iRlt)
	{
		Show_OpticalPower_XcvrInfoArr_FromPon( ulSlot, ulPort, pBuf, pBufLen,TypeFlag,(struct vty *)pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}

static int RPC_optical_power_PonSfpType(short int ulSlot, short int ulPort, void *pBuf /*void *pointer*/)
{
	int iRlt,pBufLen/*,i*/;
	unsigned int PonPortIdx;
       /*UCHAR *pBuf;*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	pBufLen = SFP_TYPE_Vendor_len;
	/*pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);*/
				
	iRlt = OpticalPower_PRC_CALL(show_optical_power_pon_sfp_type,PonPortIdx, NULL,0,ulSlot,ulPort,0,0,0,pBuf,pBufLen);

	/*for(i = 0;i<SFP_TYPE_Vendor_len;i++)
	{
		if(pBuf[i]==0x20 || pBuf[i] == 0x43)
		{
			pBuf[i] = 0x0;
		}
	}
	if(VOS_OK == iRlt)
	{
		Show_Pon_Sfp_Type(pBuf,1,pointer);
	}
	else
	{
		Show_Pon_Sfp_Type(pBuf,0,pointer);
	}
	VOS_Free( pBuf);*/
   	return iRlt;
}

static int get_optical_power_XcvrInfoArr(short int ulSlot, short int ulPort, UCHAR *TypeFlag, void*pBuf  )
{
	short int PonPortIdx;
	ULONG ulDevAddr;
	UCHAR GetlDevAddr[128];
	int i,j,k,Ret,m,OffLine_Test;
	_XCVR_DATA_  *pXcvrArr;
	union byte_short_selfdef GetAddr;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	ulSlot = GetCardIdxByPonChip(PonPortIdx);
	ulPort = GetPonPortByPonChip(PonPortIdx);

	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 1 板在位检查*/
	{
		sys_console_printf(" %% slot %d is not inserted\r\n", ulSlot);
		return( RERROR );
	}
	
	if(SlotCardIsPonBoard(ulSlot) != ROK )		/* 2 pon 板类型检查*/
	{
		sys_console_printf(" %% slot %d is not pon card\r\n", ulSlot);
		return( RERROR );
	}

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)	/* 3  pon chip is inserted  */
	{
		sys_console_printf("  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(RERROR);
	}

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		sys_console_printf("  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
	}
	
	if(VOS_MemCmp(TypeFlag,"info",  4) == 0)
	{
		ulDevAddr = A0H_1010000X;
		pXcvrArr = &XcvrInfoArr[0];
	}
	else
	{
		ulDevAddr = A2H_1010001X;
		pXcvrArr = &XcvrDiagArr[0];
	}

	k = 0;
	OffLine_Test = 0;
	for(i=0; pXcvrArr[i].pcName != NULL ; i++)
	{
		if(k<pXcvrArr[i].cAddr)
		{
			for(m=0;m<pXcvrArr[i].cAddr-k;m++)
				GetlDevAddr[k] = 0x20;	
		}
		k = pXcvrArr[i].cAddr ;
		for(j=0; j<pXcvrArr[i].cLen; j++)
		{
		    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			Ret = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr[i].cAddr+j, &(GetAddr.sshort));
			if((PAS_EXIT_OK  != Ret)/*||((PAS_EXIT_OK  == ret)&&((*(&data))==0))*/)
			{
				OffLine_Test++;
				if(OffLine_Test > 4)
					return VOS_ERROR;
			}
			else
			{
				/*if((*(&data+1))==0x20)
				{
					break;
				}
				else*/ 
					/*GetlDevAddr[k] = *(((unsigned char *)&data)+1);*/
				GetlDevAddr[k] = GetAddr.sbyte[1];
			}
			if(k>128)
			{
				VOS_ASSERT(0);
				return VOS_ERROR;
			}
			k++;
		}
		/*sys_console_printf("i = %d, j= %d, pXcvrArr[i].cLen is %d",i,j,pXcvrArr[i].cLen);*/
	}
	
	VOS_MemCpy(pBuf, GetlDevAddr, 128);
	
	return VOS_OK;
}

extern int  ponSfp_readVendorID( short int PonPortIdx, char *sfpType);
static int get_optical_power_PonSfpType(short int ulSlot, short int ulPort, void*pBuf  )
{
	short int PonPortIdx,iRet = -1;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	ulSlot = GetCardIdxByPonChip(PonPortIdx);
	ulPort = GetPonPortByPonChip(PonPortIdx);

	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 1 板在位检查*/
	{
		sys_console_printf(" %% slot %d is not inserted\r\n", ulSlot);
		return( RERROR );
	}
	
	if(SlotCardIsPonBoard(ulSlot) != ROK )		/* 2 pon 板类型检查*/
	{
		sys_console_printf(" %% slot %d is not pon card\r\n", ulSlot);
		return( RERROR );
	}

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)	/* 3  pon chip is inserted  */
	{
		sys_console_printf("  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(RERROR);
	}

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		sys_console_printf("  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
	}
	
	iRet = ponSfp_readVendorID(PonPortIdx,pBuf);
	
	return iRet;
}

int Onu_Tx_Power_Supply_Control(ULONG ulSlot , ULONG ulPort ,ULONG *onuList ,ULONG length,ULONG action)
{
	unsigned int PonPortIdx = 0;
	short int OnuEntry = 0;
	int i = 0, ret = -1;
	ULONG OnuIdx = 0;
	CTC_STACK_onu_tx_power_supply_control_t parameter;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	if(onuList[0] == 0xff)
	{
		parameter.action = (USHORT)action;
		parameter.optical_id = 0;
		parameter.onu_sn[0] = 0xff;
		parameter.onu_sn[1] = 0xff;
		parameter.onu_sn[2] = 0xff;
		parameter.onu_sn[3] = 0xff;
		parameter.onu_sn[4] = 0xff;
		parameter.onu_sn[5] = 0xff;
#if 0
		ret = CTC_STACK_onu_tx_power_supply_control((PON_olt_id_t)PonPortIdx ,(PON_onu_id_t)onuList[0],parameter,TRUE);
#endif 
		ret = OLT_SetCTCONUTxPowerSupplyControl((PON_olt_id_t)PonPortIdx ,0,&parameter);
	}
	else
	{
		for(i = 0; i < (length/sizeof(ULONG)); i++)
		{
			VOS_MemZero( &parameter, sizeof(CTC_STACK_onu_tx_power_supply_control_t ) );
			OnuIdx = onuList[i] - 1;
			if(OnuIdx >= MAXONUPERPON) 
			{
				sys_console_printf("Onu %d must be less than %d!\r\n",OnuIdx+1,MAXONUPERPON+1);
				continue;
			}
			OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx ;
			VOS_MemCpy(parameter.onu_sn, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, BYTES_IN_MAC_ADDRESS);
			if((parameter.onu_sn[0] == 0xff) && (parameter.onu_sn[1] == 0xff) && (parameter.onu_sn[2] == 0xff)
				&& (parameter.onu_sn[3] == 0xff) &&( parameter.onu_sn[4] == 0xff) && (parameter.onu_sn[5] == 0xff))
			{
				parameter.onu_sn[0] = 0;
				parameter.onu_sn[1] = 0;
				parameter.onu_sn[2] = 0;
				parameter.onu_sn[3] = 0;
				parameter.onu_sn[4] = 0;
				parameter.onu_sn[5] = 0;
			}
			parameter.action = (USHORT)action;
			parameter.optical_id = 0;	
#if 0
			ret = CTC_STACK_onu_tx_power_supply_control((PON_olt_id_t)PonPortIdx ,(PON_onu_id_t)OnuIdx,parameter,TRUE);
#endif
			ret = OLT_SetCTCONUTxPowerSupplyControl((PON_olt_id_t)PonPortIdx ,(PON_onu_id_t)OnuIdx,&parameter);
		}
	}
}

LONG Fetch_Optical_Power_XcvrData(ULONG ulSlot, ULONG ulPort,UCHAR *TypeFlag,void *pointer)
{
	int iRlt;

	unsigned int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_optical_power_XcvrInfoArr_t, (ulSlot, ulPort, TypeFlag, pointer) );

	return iRlt;

}

LONG Fetch_Optical_Power_PonSfpType(ULONG ulSlot, ULONG ulPort,void *pointer)
{
	int iRlt;

	unsigned int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_optical_power_pon_sfp_type_t, (ulSlot, ulPort, pointer) );

	return iRlt;

}

LONG Fetch_Optical_Power_Instant(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length,void *pointer)
{
	int iRlt;

	unsigned int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;
	
	iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_optical_power_olt_rx_instant_t, (ulSlot, ulPort, onuList, length,pointer) );

	return iRlt;

}

LONG Fetch_Optical_Power_History(ULONG ulSlot, ULONG ulPort, ULONG *onuList, ULONG length,void *pointer)
{
	int iRlt;

	unsigned int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;
	
	iRlt = OPTICALPOWER_API_CALL( PonPortIdx, show_optical_power_olt_rx_history_t, (ulSlot, ulPort, onuList, length,pointer) );

	return iRlt;

}

LONG Set_OnuLaserAlwaysOn_AlarmThreshold(unsigned int PonPortIdx,int alarm_threshold)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, onu_laser_always_on_alarm_threshold_t, (PonPortIdx,alarm_threshold) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, onu_laser_always_on_alarm_threshold_t, (olt_id,alarm_threshold))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		onuLaser_alwaysOn_alarm_threshold = alarm_threshold ;
	}
    
	return iRlt;

}

LONG Set_OnuLaserAlwaysOn_ClearTimes(unsigned int PonPortIdx,int clear_times)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, onu_laser_always_on_alarm_clear_times_t, (PonPortIdx,clear_times) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, onu_laser_always_on_alarm_clear_times_t, (olt_id,clear_times))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		onuLaser_alwaysOn_clear_timeCounter = clear_times ;
	}
    
	return iRlt;

}


LONG Set_OnuLaserAlwaysOn_AlarmTimes(unsigned int PonPortIdx,int alarm_times)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, onu_laser_always_on_alarm_times_t, (PonPortIdx,alarm_times) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, onu_laser_always_on_alarm_times_t, (olt_id,alarm_times))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		onuLaser_alwaysOn_alarm_timeCounter = alarm_times ;
	}
    
	return iRlt;

}


LONG Set_OnuLaserAlwaysOn_Enable(unsigned int PonPortIdx,int enable_mode)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, onu_laser_always_on_enable_t, (PonPortIdx,enable_mode) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, onu_laser_always_on_enable_t, (olt_id,enable_mode))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOnuLaserAlwaysOnEnable(enable_mode);
		if(enable_mode == V2R1_DISABLE)
		{
			if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER ) 
			{
				for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++ )
					ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( PonPortIdx );
			}
		}
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Voltage_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt,olt_id;
	int slotno = 0;
/*	
	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		slotno = GetCardIdxByPonChip(PonPortIdx);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_10G_8EPON)
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
		
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_voltage_alarm_threshold_olt_t, (PonPortIdx,low_val,high_val,flag) );
	}
	else
	{*/
		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_voltage_alarm_threshold_olt_t, (olt_id,low_val,high_val,flag))) )
			{
				break;
			}
		}
/*	}*/

	if ( VOS_OK == iRlt )
    	{
		SetPonPortWorkVoltageLowThrd(low_val,flag);
		SetPonPortWorkVoltageHighThrd(high_val,flag);
	}
    
	return iRlt;

}


LONG Set_OpticalPower_Voltage_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_voltage_alarm_threshold_onu_t, (PonPortIdx,low_val,high_val) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_voltage_alarm_threshold_onu_t, (olt_id,low_val,high_val))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOnuWorkVoltageLowThrd(low_val);
		SetOnuWorkVoltageHighThrd(high_val);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Voltage_Threshold_Uplink(long low_val,long high_val,int flag)
{
	int iRlt,olt_id;

	for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
	{
		if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_voltage_alarm_threshold_uplink_t, (olt_id,low_val,high_val,flag))) )
		{
			break;
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetUplinkPortVoltageLowthrd(low_val,flag);
		SetUplinkPortVoltageHighthrd(high_val,flag);
	}
    
	return iRlt;

}


LONG Set_OpticalPower_Temperature_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_temperature_alarm_threshold_onu_t, (PonPortIdx,low_val,high_val) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_temperature_alarm_threshold_onu_t, (olt_id,low_val,high_val))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOnuTemperatureLowThrd(low_val);
		SetOnuTemperatureHighThrd(high_val);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Temperature_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt,olt_id;
	int slotno = 0;

/*	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		slotno = GetCardIdxByPonChip(PonPortIdx);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_10G_8EPON)
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
	
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_temperature_alarm_threshold_olt_t, (PonPortIdx,low_val,high_val,flag) );
	}
	else
	{*/
		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_temperature_alarm_threshold_olt_t, (olt_id,low_val,high_val,flag))) )
			{
				break;
			}
		}
/*	}*/

	if ( VOS_OK == iRlt )
    	{
		SetPonPortTemperatureLowThrd(low_val,flag);
		SetPonPortTemperatureHighThrd(high_val,flag);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Temperature_Threshold_Uplink(long low_val,long high_val,int flag)
{
	int iRlt,olt_id;

	for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
	{
		if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_temperature_alarm_threshold_uplink_t, (olt_id,low_val,high_val,flag))) )
		{
			break;
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetUplinkPortTemperatureLowthrd(low_val,flag);
		SetUplinkPortTemperaturreHighthrd(high_val,flag);
	}
    
	return iRlt;

}


LONG Set_OpticalPower_BiasCurrent_Threshold_Onu(unsigned int PonPortIdx,long low_val,long high_val)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_bias_current_alarm_threshold_onu_t, (PonPortIdx,low_val,high_val) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_bias_current_alarm_threshold_onu_t, (olt_id,low_val,high_val))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOnuBiasCurrentLowThrd(low_val);
		SetOnuBiasCurrentHighThrd(high_val);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_BiasCurrent_Threshold_Olt(unsigned int PonPortIdx,long low_val,long high_val,int flag)
{
	int iRlt,olt_id;
	int slotno = 0;
	
/*	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		slotno = GetCardIdxByPonChip(PonPortIdx);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_10G_8EPON)
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
		
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_bias_current_alarm_threshold_olt_t, (PonPortIdx,low_val,high_val,flag) );
	}
	else
	{*/
		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_bias_current_alarm_threshold_olt_t, (olt_id,low_val,high_val,flag))) )
			{
				break;
			}
		}
	/*}*/

	if ( VOS_OK == iRlt )
    	{
		SetPonPortBiasCurrentLowThrd(low_val,flag);
		SetPonPortBiasCurrentHighThrd(high_val,flag);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_BiasCurrent_Threshold_Uplink(long low_val,long high_val,int flag)
{
	int iRlt,olt_id;

	for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
	{
		if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_biasCurrent_alarm_threshold_uplink_t, (olt_id,low_val,high_val,flag))) )
		{
			break;
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetUplinkPortBiasCurrentLowthrd(low_val,flag);
		SetUplinkPortBiasCurrentHighthrd(high_val,flag);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_alarm_threshold_onu(unsigned int PonPortIdx,long tx_power_low,
	long tx_power_high,long rx_power_low, long rx_power_high)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_alarm_threshold_onu_t, (PonPortIdx,
			tx_power_low,tx_power_high,rx_power_low,rx_power_high) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_alarm_threshold_onu_t, (olt_id,
				tx_power_low,tx_power_high,rx_power_low,rx_power_high))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOnuTransOpticalPowerLowThrd(tx_power_low);
		SetOnuTransOpticalPowerHighThrd(tx_power_high);
		SetOnuRecvOpticalPowerLowThrd(rx_power_low);
		SetOnuRecvOpticalPowerHighThrd(rx_power_high);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_alarm_threshold_olt(unsigned int PonPortIdx,long tx_power_low,
	long tx_power_high,long rx_power_low, long rx_power_high,int flag)
{
	int iRlt,olt_id;
	int slotno = 0;
	
	/*if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		slotno = GetCardIdxByPonChip(PonPortIdx);
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_10G_8EPON)
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
		
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_alarm_threshold_olt_t, (PonPortIdx,
			tx_power_low,tx_power_high,rx_power_low,rx_power_high,flag) );
	}
	else
	{*/
		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_alarm_threshold_olt_t, (olt_id,
				tx_power_low,tx_power_high,rx_power_low,rx_power_high,flag))) )
			{
				break;
			}
		}
	/*}*/

	if ( VOS_OK == iRlt )
    	{
		SetPonPortTransOpticalPowerLowThrd(tx_power_low,flag);
		SetPonPortTransOpticalPowerHighThrd(tx_power_high,flag);	
		SetPonPortRecvOpticalPowerLowThrd(rx_power_low,flag);
		SetPonPortRecvOpticalPowerHighThrd(rx_power_high,flag);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_alarm_Threshold_Uplink(long tx_power_low,long tx_power_high,
	long rx_power_low, long rx_power_high,int flag)
{
	int iRlt,olt_id;

	for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
	{
		if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_alarm_threshold_uplink_t, (olt_id,tx_power_low,
			tx_power_high,rx_power_low,rx_power_high,flag))) )
		{
			break;
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetUplinkPortTransOpticalPowerLowthrd(tx_power_low, flag);
		SetUplinkPortTransOpticalPowerHighthrd(tx_power_high, flag);	
		SetUplinkPortRecvOpticalPowerLowthrd(rx_power_low, flag );
		SetUplinkPortRecvOpticalPowerHighthrd(rx_power_high, flag );
	}
    
	return iRlt;

}


static void setOpticalPowerDeadZone( int field, long val)
{
	switch( field )
	{
		case field_power_dead_zone:
			eponOpticalPowerDeadZone.powerVarDeadZone = val;
			break;
		case field_tempe_dead_zone:
			eponOpticalPowerDeadZone.temVarDeadZone = val;
			break;
		case field_vol_dead_zone:
			eponOpticalPowerDeadZone.volVarDeadZone  = val;
			break;
		case field_cur_dead_zone:
			eponOpticalPowerDeadZone.curVarDeadZone = val;
			break;
		default:
			break;
	}	
}

LONG getOpticalPowerDeadZone( int field)
{
	long ret = -1;	/* modified by xieshl 20080823 */
	switch( field )
	{
		case field_power_dead_zone:
			ret = eponOpticalPowerDeadZone.powerVarDeadZone;
			break;
		case field_tempe_dead_zone:
			ret = eponOpticalPowerDeadZone.temVarDeadZone;
			break;
		case field_vol_dead_zone:
			ret = eponOpticalPowerDeadZone.volVarDeadZone;
			break;
		case field_cur_dead_zone:
			ret = eponOpticalPowerDeadZone.curVarDeadZone;
			break;
		default:
			break;
	}
	return(ret);
}

LONG Set_OpticalPower_Deadzone(unsigned int PonPortIdx,long power_deadzone,
	long temperature_deadzone,long supply_voltage_deadzone,long bias_current_zone)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_threshold_deadzone_t, (PonPortIdx,
			power_deadzone,temperature_deadzone,supply_voltage_deadzone,bias_current_zone) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_threshold_deadzone_t, (olt_id,
				power_deadzone,temperature_deadzone,supply_voltage_deadzone,bias_current_zone))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		setOpticalPowerDeadZone(field_power_dead_zone,power_deadzone);
		setOpticalPowerDeadZone(field_tempe_dead_zone,temperature_deadzone);
		setOpticalPowerDeadZone(field_vol_dead_zone,supply_voltage_deadzone);
		setOpticalPowerDeadZone(field_cur_dead_zone,bias_current_zone);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Enable(unsigned int PonPortIdx,int enable_mode, int onu_enable_mode, int Flag)
{
	int iRlt,olt_id;
#if 0
	if( Flag == 1 )  /* 主要保证网管重复配置使能不要把通过命令行配置的关于ONU 的配置覆盖*/
	{
		if( ( enable_mode == V2R1_ENABLE ) && ( V2R1_ENABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) ))
			return VOS_OK;

		if( ( enable_mode == V2R1_DISABLE ) && ( V2R1_DISABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) ))
			return CMD_WARNING;
	}
#endif

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_enable_t, (PonPortIdx,enable_mode,onu_enable_mode, Flag) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_enable_t, (olt_id,enable_mode,onu_enable_mode, Flag) )) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetPonPortOpticalMonitorEnable(enable_mode);
		SetOnuOpticalPowerEnable( onu_enable_mode );/*这里该函数要放在SetPonPortOpticalMonitorEnable 之后调用*/
	}
    
	return iRlt;

}

LONG Set_Debug_OpticalPower(unsigned int PonPortIdx,long enable_mode, long DebugType)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, debug_optical_power_command_t, (PonPortIdx,enable_mode,DebugType) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, debug_optical_power_command_t, (olt_id,enable_mode,DebugType))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetOpticalPowerDebug(enable_mode,DebugType);
	}
    
	return iRlt;

}



LONG Set_OpticalPower_Interval(unsigned int PonPortIdx,int interval)
{
	int iRlt,olt_id;

	if( interval < 30 || interval > 86400)
		return VOS_ERROR;
	
	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_interval_t, (PonPortIdx,interval) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_interval_t, (olt_id,interval))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		SetPonPortOpticalMonitorInterval(interval);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_SfpType_Add(unsigned int PonPortIdx,UCHAR *String_Name)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, pon_adc_negative_polarity_stp_type_t, (PonPortIdx,String_Name) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, pon_adc_negative_polarity_stp_type_t, (olt_id,String_Name))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		Sfp_Type_Add(String_Name);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_SfpType_Del(unsigned int PonPortIdx,UCHAR *String_Name)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, undo_pon_adc_negative_polarity_stp_type_t, (PonPortIdx,String_Name) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, undo_pon_adc_negative_polarity_stp_type_t, (olt_id,String_Name))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		Sfp_Type_Delete(String_Name);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Calibration_Olt(unsigned int PonPortIdx,long rx_calibration, long tx_calibration)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_calibration_olt_t, (PonPortIdx, rx_calibration, tx_calibration) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_calibration_olt_t, (olt_id,rx_calibration, tx_calibration))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		setOltOpticalPowerCalibration(rx_calibration, tx_calibration);
	}
    
	return iRlt;

}

LONG Set_OpticalPower_Calibration_Onu(unsigned int PonPortIdx,long rx_calibration, long tx_calibration)
{
	int iRlt,olt_id;

	if ( OPTICAL_PONPORTIDX_ALL != PonPortIdx )
	{   
		OPTICALPOWER_ASSERT(PonPortIdx);
		iRlt = OPTICALPOWER_API_CALL( PonPortIdx, optical_power_calibration_onu_t, (PonPortIdx, rx_calibration, tx_calibration) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = OPTICALPOWER_API_CALL( olt_id, optical_power_calibration_onu_t, (olt_id,rx_calibration, tx_calibration))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		setOnuOpticalPowerCalibration(rx_calibration, tx_calibration);
	}
    
	return iRlt;

}

LONG onu_tx_power_supply_control(ULONG ulSlot , ULONG ulPort ,ULONG *onuList ,ULONG length,ULONG action )
{
	int iRlt;
	unsigned int PonPortIdx;

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));

	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;
	
	iRlt = Onu_Tx_Power_Supply_Control(ulSlot, ulPort, onuList,  length, action);
	return iRlt;
}
 
static int OpticalPower_OK2(unsigned int para1, int para2)
{
	return OLT_ERR_OK;
}

static int OpticalPower_OK2c(unsigned int para1, UCHAR * para2)
{
	return OLT_ERR_OK;
}

static int OpticalPower_OK3(unsigned int para1, long para2, long para3)
{
	return OLT_ERR_OK;
}

static int OpticalPower_OK4(unsigned int para1, long para2, long para3, int para4)
{
	return OLT_ERR_OK;
}

static int OpticalPower_OK5(unsigned int para1, long para2, long para3, long para4, long para5)
{
	return OLT_ERR_OK;
}

static int OpticalPower_OK6(unsigned int para1, long para2, long para3, long para4, long para5,int para6)
{
	return OLT_ERR_OK;
}

static int OpticalPower_Enable(unsigned int para1, int para2, int para3, int para4)
{
    CtcEventCfgData_t pAlmMsg;
    pAlmMsg.alarm_id = RX_POWER_HIGH_ALARM;
    pAlmMsg.flag = 0;
    CTC_eventReportMsgSend(&pAlmMsg );
	return OLT_ERR_OK;
}
static int OpticalPower_Deadzone(unsigned int para1, long para2, long para3, long para4, long para5)
{
    CtcEventCfgData_t pAlmMsg;
    pAlmMsg.flag = 1;
    pAlmMsg.alarm_id = RX_POWER_HIGH_ALARM;
    CTC_eventReportMsgSend(&pAlmMsg );
    pAlmMsg.alarm_id = TX_BIAS_HIGH_ALARM;
    CTC_eventReportMsgSend(&pAlmMsg );
    pAlmMsg.alarm_id = VCC_HIGH_ALARM;
    CTC_eventReportMsgSend(&pAlmMsg );
    pAlmMsg.alarm_id = TEMP_HIGH_ALARM;
    CTC_eventReportMsgSend(&pAlmMsg );
    
	return OLT_ERR_OK;
}

static int OpticalPower_alarm_threshold_onu(unsigned int para1, long para2, long para3, long para4, long para5)
{
    CtcEventCfgData_t almMsg;
    almMsg.alarm_id = RX_POWER_HIGH_ALARM;
    almMsg.flag = 1;
    CTC_eventReportMsgSend(&almMsg );
	return OLT_ERR_OK;
}
static int OpticalPower_BiasCurrent_Threshold_Onu(unsigned int para1, long para2, long para3)
{
    CtcEventCfgData_t almMsg;
    almMsg.alarm_id = TX_BIAS_HIGH_ALARM;
    almMsg.flag = 1;
    CTC_eventReportMsgSend(&almMsg );
	return OLT_ERR_OK;
}
static int OpticalPower_Temperature_Threshold_Onu(unsigned int para1, long para2, long para3)
{
    CtcEventCfgData_t almMsg;
    almMsg.alarm_id = TEMP_LOW_ALARM;
    almMsg.flag = 1;
    CTC_eventReportMsgSend(&almMsg );
	return OLT_ERR_OK;
}
static int OpticalPower_Voltage_Threshold_Onu(unsigned int para1, long para2, long para3)
{
    CtcEventCfgData_t almMsg;
    almMsg.alarm_id = VCC_LOW_ALARM;
    almMsg.flag = 1;
    CTC_eventReportMsgSend(&almMsg );
	return OLT_ERR_OK;
}

static const OpticalPowerMgmtIFs M_rpcIfs = {
	RPC_OpticalPower_Enable,
	RPC_OpticalPower_Interval,
	RPC_OpticalPower_StpType_Add,
	RPC_OpticalPower_StpType_Del,
	RPC_OpticalPower_Calibration_Olt,
	RPC_OpticalPower_Calibration_Onu,
	RPC_OpticalPower_Deadzone,
	RPC_OpticalPower_alarm_threshold_olt,
	RPC_OpticalPower_alarm_threshold_onu,
	RPC_OpticalPower_alarm_threshold_Uplink,
	RPC_OpticalPower_BiasCurrent_Threshold_Olt,
	RPC_OpticalPower_BiasCurrent_Threshold_Onu,
	RPC_OpticalPower_BiasCurrent_Threshold_Uplink,
	RPC_OpticalPower_Temperature_Threshold_Olt,
	RPC_OpticalPower_Temperature_Threshold_Onu,
	RPC_OpticalPower_Temperature_Threshold_Uplink,
	RPC_OpticalPower_Voltage_Threshold_Olt,
	RPC_OpticalPower_Voltage_Threshold_Onu,
	RPC_OpticalPower_Voltage_Threshold_Uplnk,
	RPC_OnuLaserAlwaysOn_Enable,
	RPC_OnuLaserAlwaysOn_AlarmTimes,
	RPC_OnuLaserAlwaysOn_ClearTimes,
	RPC_OnuLaserAlwaysOn_AlarmThreshold,
	RPC_Optical_Power_Instant,
	RPC_Optical_Power_History,
	RPC_OnuLaser_AlwaysOn,
	RPC_optical_power_XcvrInfoArr,
	RPC_Debug_OpticalPower,
	RPC_optical_power_PonSfpType,
	RPC_Online_Sfp_Pon,
	RPC_Get_Sfp_Online_State,
};
#define Size_M_rpcIfs sizeof(M_rpcIfs)
static const OpticalPowerMgmtIFs S_rpcIfs = {
	OpticalPower_Enable,
	OpticalPower_OK2,
	OpticalPower_OK2c,
	OpticalPower_OK2c,
	OpticalPower_OK3,
	OpticalPower_OK3,
	OpticalPower_Deadzone,
	OpticalPower_OK6,
	OpticalPower_alarm_threshold_onu,
	OpticalPower_OK6,
	OpticalPower_OK4,
	OpticalPower_BiasCurrent_Threshold_Onu,
	OpticalPower_OK4,
	OpticalPower_OK4,
	OpticalPower_Temperature_Threshold_Onu,
	OpticalPower_OK4,
	OpticalPower_OK4,
	OpticalPower_Voltage_Threshold_Onu,
	OpticalPower_OK4,
	OpticalPower_OK2,
	OpticalPower_OK2,
	OpticalPower_OK2,
	OpticalPower_OK2,
	get_optical_power_meter_instant,
	get_optical_power_meter_history,
	get_Onu_laser_Always_on_Value,
	get_optical_power_XcvrInfoArr,
	OpticalPower_OK3,
	get_optical_power_PonSfpType,
	get_Online_Sfp_Pon,
	Get_Sfp_Online_State,
};
#define Size_S_rpcIfs sizeof(S_rpcIfs)
/*add by yangzl for R268 optical-power@2016-6-8*/
int  AdcConfig_BCM(short int PonPortIdx)
{
	UCHAR adc_start_time = 10;
	UCHAR adc_time_len = 106;
	UCHAR adc_clk_polarity= PON_POLARITY_POSITIVE;
	UCHAR adc_clock=16;
	UCHAR adc_bits_number = 10;
	UCHAR cs_polarity = PON_POLARITY_POSITIVE;

	PON_adc_config_t  adc_config;
	short int PonChipType;
	short int ret;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

	if(OLT_PONCHIP_ISPAS5001(PonChipType)) return( RERROR );
	
	adc_config.adc_bits_number = adc_bits_number;
	adc_config.adc_vdd = 33;
	adc_config.adc_number_of_leading_zeros = 23;
	adc_config.adc_number_of_trailing_zeros =0;
	adc_config.adc_clock = adc_clock;
	adc_config.clock_polarity = adc_clk_polarity;
	adc_config.cs_polarity = cs_polarity;
	adc_config.adc_measurement_start = adc_start_time;
	adc_config.adc_measurement_time = adc_time_len;
	adc_config.adc_interface = PON_ADC_INTERFACE_I2C;
	ret = OLT_SetVirtualScopeAdcConfig( PonPortIdx, &adc_config);
	if(ret != PAS_EXIT_OK )
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
			sys_console_printf("pon%d/%d Adc Config err %d\r\n", GetCardIdxByPonChip(PonPortIdx), (GetPonPortByPonChip(PonPortIdx)), ret );
		return( RERROR );
	}
	return( ROK );
}

int  AdcConfig_default(short int PonPortIdx)
{
	UCHAR adc_start_time = 0;
	UCHAR adc_time_len = 50;
	UCHAR adc_clk_polarity= PON_POLARITY_POSITIVE;
	UCHAR adc_clock=16;
	UCHAR adc_bits_number = 10;
	UCHAR cs_polarity = PON_POLARITY_POSITIVE;

	PON_adc_config_t  adc_config;
	short int PonChipType;
	short int ret;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

	if(OLT_PONCHIP_ISPAS5001(PonChipType)) return( RERROR );
	
	adc_config.adc_bits_number = adc_bits_number;
	adc_config.adc_vdd = 33;
	adc_config.adc_number_of_leading_zeros = 23;
	adc_config.adc_number_of_trailing_zeros =0;
	adc_config.adc_clock = adc_clock;
	adc_config.clock_polarity = adc_clk_polarity;
	adc_config.cs_polarity = cs_polarity;
	adc_config.adc_measurement_start = adc_start_time;
	adc_config.adc_measurement_time = adc_time_len;
	adc_config.adc_interface = PON_ADC_INTERFACE_I2C;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_SetVirtualScopeAdcConfig( PonPortIdx, &adc_config);
	if(ret != PAS_EXIT_OK )
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
			sys_console_printf("pon%d/%d Adc Config err %d\r\n", GetCardIdxByPonChip(PonPortIdx), (GetPonPortByPonChip(PonPortIdx)), ret );
		return( RERROR );
	}
	return( ROK );
}
/* 武邮SFP
adc_start_time = 0
adc_time_len = 40
adc_clk_polarity= 1
cs_polarity = 1
adc_clock=16
adc_bits_number = 10
AdcConfig 5
*/
int  AdcConfig_WTD(short int PonPortIdx)
{
	PON_adc_config_t  adc_config;
	short int PonChipType;
	short int ret;

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);

	if(PonChipType != PONCHIP_PAS5201) return( RERROR );
	
	adc_config.adc_bits_number = 10;
	adc_config.adc_vdd = 33;
	adc_config.adc_number_of_leading_zeros = 23;
	adc_config.adc_number_of_trailing_zeros =0;
	adc_config.adc_clock = 16;
	adc_config.clock_polarity = PON_POLARITY_NEGATIVE;
	adc_config.cs_polarity = PON_POLARITY_NEGATIVE;
	adc_config.adc_measurement_start = 0;
	adc_config.adc_measurement_time = 40;
	adc_config.adc_interface = PON_ADC_INTERFACE_I2C;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_SetVirtualScopeAdcConfig( PonPortIdx, &adc_config);
	if(ret != PAS_EXIT_OK )
	{
		if(EVENT_DEBUG == V2R1_ENABLE )
			sys_console_printf("pon%d/%d Adc Config err %d\r\n", GetCardIdxByPonChip(PonPortIdx), (GetPonPortByPonChip(PonPortIdx)), ret );
		return( RERROR );
	}
	return( ROK );
}
/*
static LONG sfp_i2c_read( ULONG device, ULONG sfp_sel, ULONG i2c_addr, ULONG reg, UCHAR * value, UCHAR len )
{
	int ret = FALSE;
	if( value == NULL )
	{
		VOS_ASSERT(0);
		return ret;
	}
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
	{
		if( sfp_sel == 0 )
			i2c_addr += 8;
		else
			i2c_addr += 6;
	}
	if( len == 1 )
		ret = i2c_read_all( device, sfp_sel, i2c_addr, reg, value );
	else if( len == 2 )
		ret = i2c_read_all2( device, sfp_sel, i2c_addr, reg, value );
	else if( len == 4 )
		ret = i2c_read_all4( device, sfp_sel, i2c_addr, reg, value );
	
	return ret;
}

static LONG sfp_i2c_write( ULONG device, ULONG sfp_sel, ULONG i2c_addr, ULONG reg, UCHAR * value, UCHAR len )
{
	int ret = FALSE;
	int i;
	if( (value == NULL) || (len > 4) )
	{
		VOS_ASSERT(0);
		return ret;
	}
	if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100 )
	{
		if( sfp_sel == 0 )
			i2c_addr += 8;
		else
			i2c_addr += 6;
	}
	for( i=0; i<len; i++ )
	{
		ret = i2c_write_all( device, sfp_sel, i2c_addr, reg, value[i] );
		if( !ret )
			break;
	}
	return ret;
}
*/
extern int i2c_data_get( UINT slot_id, UINT type, UINT reg, UCHAR *pdata, UINT len);
static LONG sfp_i2c_read( ULONG device, ULONG sfp_sel, ULONG reg, UCHAR * value, UCHAR len )
{
	int ret = FALSE;
	int offset = sfp_sel%SFP_OFFSET;
/* wangysh add 先判断光模块是否在位*/
	if( !SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON )
	{
		if( !SYS_LOCAL_MODULE_ISMASTERACTIVE || !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		    return ret;
	}
	
	if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) )
	{
		if( FALSE == i2c_data_get( g_PhysicalSlot[device],I2C_BASE_GPIO+offset,0,value,I2C_RW_LEN) )
			return ret;
	
		if( *value & 1<<offset )
			return ret;
	}
	
	ret = i2c_data_get(  g_PhysicalSlot[device], I2C_BASE_SFP+sfp_sel, reg, value,len);
	
	return ret;
}
extern int i2c_data_set( UINT slot_id, UINT type, UINT reg, UCHAR *pdata, UINT len);
static LONG sfp_i2c_write( ULONG device, ULONG sfp_sel, ULONG reg, UCHAR * value, UCHAR len )
{
	int ret = FALSE;

	ret = i2c_data_set( device, I2C_BASE_SFP+sfp_sel, reg, value,len);

	return ret;
}

int UplinkSFPType_read(short int sfp_sel,char *SFPType)
{
	_XCVR_DATA_ *pXcvrArr;
	int i;
	char data;
	short int ret;
	
	data=0;	
	pXcvrArr = XcvrInfoArr;
	if(SFPType == NULL)
		return(RERROR);

	data=0;
	for(i=0; i<SFP_TYPE_Vendor_len; i++)
	{
		ret=sfp_i2c_read( 1, sfp_sel, pXcvrArr[13].cAddr+i, &data, 1 );
		if((TRUE  != ret)||((TRUE  == ret)&&((*(&data))==0)))
		{
			return VOS_ERROR;
		}
		else
		{
			if((*(&data))==0x20)
			{
				break;
			}
			else 
				SFPType[i] = *(&data);
		}
	}
	SFPType[i]='\0';
	return(ROK);		
}

UCHAR UplinkSFPDiagnosticMonitorType(short int slotno,short int sfp_sel, int Flag )
{
	UCHAR  type = NOT_SUPPORT_SFF8472;
	char data = 0;
	_XCVR_DATA_ *pXcvrArr = XcvrInfoArr;
	UCHAR value,a,b;
	ULONG offset, portno ;
	
	
	offset = sfp_sel%SFP_OFFSET;
	portno = slotno*MAXUPLINKPORT+sfp_sel;

	if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
	{
		if( FALSE == i2c_data_get( g_PhysicalSlot[slotno],I2C_BASE_GPIO+offset,0,&value,I2C_RW_LEN) )
			return type;

		if( value & 1<<offset )
		{	
			/* 光模块 不在位*/
			if( 1 == Flag ) /*表示需要进行判断是否消除告警*/
			{
				if( Sfp_Uplink_Online_Flag[ portno ] == 1 )
				{
					ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
				}
			}
			Sfp_Uplink_Online_Flag[ portno ] = 0;

			return type;
		}
		else
		{
			/* 光模块在位*/
			Sfp_Uplink_Online_Flag[ portno ] = 1;
		}
	}
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		if( TRUE == i2c_data_get( g_PhysicalSlot[slotno],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) 
		{
			if( value & 1<<sfp_sel)
			{	
				/* 光模块 不在位*/
				if( 1 == Flag ) /*表示需要进行判断是否消除告警*/
				{
					if( Sfp_Uplink_Online_Flag[ portno ] == 1 )
					{
						ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
					}
				}
				Sfp_Uplink_Online_Flag[ portno ] = 0;

				return type;
			}
			else
			{
				/* 光模块在位*/
				Sfp_Uplink_Online_Flag[ portno ] = 1;
			}

		}

		if(TRUE ==  i2c_data_get2( slotno, sfp_sel+1,0x50, pXcvrArr[26].cAddr, &data, 1) )
		{
			if( data & (1<<6) )	/* 0x58 == 01011000 */
			{
				if( data & (1<<4) )
					type = EXTERNALLY_CALIBRATED;
				else
					type = INTERNALLY_CALIBRATED;
			}
		}

		return type;
	}
	

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &value);
		else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		{
			ReadCPLDReg(GFA8100_UPLINK_SFP_GPON_STATE, &value);
			a = value & 0x03;//取bit0、1
			b = value & 0x0C;//取bit2、3
			/*c = value & 0xf0;*/
			value = (a<<2) | (b>>2);//组合在一起，使之与EPON的端口号一致
			/*epon低四位代表的端口号顺序为19.20.17.18。GPON低四位代表的端口号为17.18.19.20(从bit0开始)*/
			/*这样就是使GPON的端口号和epon一致。如果以后会用到value的高四位，那就定义C将value补全*/
			/*value = value | c;*/
		}
	
		if(sfp_sel < 2)
		{
			if(value & 1<<(sfp_sel+2))
			{
				if( 1 == Flag ) /*表示需要进行判断是否消除告警*/
				{
					if( Sfp_Uplink_Online_Flag[ portno ] == 1 )
					{
						ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
					}
				}
				Sfp_Uplink_Online_Flag[ portno ] = 0;

				return type;
			}
			else
			{
				/* 光模块在位*/
				Sfp_Uplink_Online_Flag[ portno ] = 1;
			}
		
			if( i2c_data_get( slotno, I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[26].cAddr, &data, 1) )
			{
				if( data & (1<<6) )	/* 0x58 == 01011000 */
				{
					if( data & (1<<4) )
						type = EXTERNALLY_CALIBRATED;
					else
						type = INTERNALLY_CALIBRATED;
				}
			}
		}
		else
		{
			if(value & 1<<(sfp_sel-2))
			{
				if( 1 == Flag ) /*表示需要进行判断是否消除告警*/
				{
					if( Sfp_Uplink_Online_Flag[ portno ] == 1 )
					{
						ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
					}
				}
				Sfp_Uplink_Online_Flag[ portno ] = 0;

				return type;
			}
			else
			{
				/* 光模块在位*/
				Sfp_Uplink_Online_Flag[ portno ] = 1;
			}
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)//两个偏移地址不一样，所以分开
			{
				if( i2c_data_get( slotno, sfp_sel-2, pXcvrArr[26].cAddr, &data, 1) )
				{
					if( data & (1<<6) )	/* 0x58 == 01011000 */
					{
						if( data & (1<<4) )
							type = EXTERNALLY_CALIBRATED;
						else
							type = INTERNALLY_CALIBRATED;
					}
				}
			}
			
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{
				if( i2c_data_get( slotno, 0xd0+sfp_sel-2, pXcvrArr[26].cAddr, &data, 1) )
				{
					if( data & (1<<6) )	/* 0x58 == 01011000 */
					{
						if( data & (1<<4) )
							type = EXTERNALLY_CALIBRATED;
						else
							type = INTERNALLY_CALIBRATED;
					}
				}
			}
		}
	}
	else 
	{
		if( i2c_data_get( slotno, sfp_sel, pXcvrArr[26].cAddr, &data, 1) )
		{
			if( data & (1<<6) )	/* 0x58 == 01011000 */
			{
				if( data & (1<<4) )
					type = EXTERNALLY_CALIBRATED;
				else
					type = INTERNALLY_CALIBRATED;
			}
		}
	}
	return type;
}

int CheckUplinkSupportPowerMetering(short int slotno, short int sfp_sel)
{
	/*char SFPTypeString[SFP_TYPE_Vendor_len];*/
	int portno;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	/*if(UplinkSFPType_read(sfp_sel, SFPTypeString) != ROK)
	{
		UplinkPortMeteringInfo[sfp_sel].powerMeteringSupport=0;
		SFP_RSSI_DEBUG(4096,("SFP type err:sfp_sel=%d,%d\r\n",sfp_sel+1,UplinkPortMeteringInfo[sfp_sel].powerMeteringSupport));
		return VOS_ERROR;
	}
	else
	{
		UplinkPortMeteringInfo[sfp_sel].powerMeteringSupport=1;
		SFP_RSSI_DEBUG(4096,("sfp_sel=%d,%d\r\n",sfp_sel+1,UplinkPortMeteringInfo[sfp_sel].powerMeteringSupport));
	}*/
	UplinkPortMeteringInfo[portno].powerMeteringSupport = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 1);
	if( UplinkPortMeteringInfo[portno].powerMeteringSupport == NOT_SUPPORT_SFF8472 )
		return VOS_ERROR;
	return VOS_OK;
}
int ponSFPType_read(short int PonPortIdx, char *SFPType)
{
	short int DevAddr, data;
	short int ret;
	int i;
	_XCVR_DATA_ *pXcvrArr;
	
	DevAddr = A0H_1010000X;	
	pXcvrArr = &XcvrInfoArr[13];/*厂商信息*/
	
	if(SFPType == NULL)
		return(RERROR);
	data=0;
	for(i=0; i<SFP_TYPE_Vendor_len; i++)
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_ReadI2CRegister(PonPortIdx,DevAddr, pXcvrArr->cAddr+i, &data);
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"PonPortIdx=%d %d %d\r\n",PonPortIdx, (*(&data+1)), (*(&data))));
		if((PAS_EXIT_OK  != ret)||((PAS_EXIT_OK  == ret)&&((*(&data))==0)))
		{
			return VOS_ERROR;
		}
		else
		{
			if(/*(*(&data+1))==0x20*/data == 0x20)/*空格*/
			{
				break;
			}
			else 
				SFPType[i] = *(((unsigned char *)&data)+1);
			SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"SFPType[%d]%c\r\n",i,SFPType[i]));
		}
	}
	SFPType[i]='\0';
	SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"%s\r\n",SFPType));
	return(ROK);		
}

/*功能:读取GPON的在位状态*/
/*输入:ponport索引；****************
**输出:在位状态值；1->不在位，0->在位*/
int GetGponSfpOnlineState(short int PonPortIdx,bool *state)
{
	UINT slot,port;
	UCHAR value,value1,offset;

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	offset = (port-1)%8;

	if( SYS_LOCAL_MODULE_TYPE_IS_GPON )
	{
		ReadCPLDReg( CPLD1_RESET_REG2_8000,&value);/*低八位*/
		ReadCPLDReg( CPLD1_RESET_REG3_8000,&value1);/*高八位*/

		if((port-1)/8)
		{
			if(value1 & (1 << offset))
			{
				*state = 1;
				return VOS_OK;
			}
		}
		else 
		{
			if(value & (1 << offset))
			{
				*state = 1;
				return VOS_OK;
			}
		}		
	}

	*state = 0;
	
	return VOS_OK;
}

int CheckPonSFPVendorSpecificIsGwd(short int PonPortIdx)
{
	short int ret;
	char *pGwdStr = SFPOUITYPE_GWD;
	int i;
	short int DevAddr = A0H_1010000X;
	UCHAR data[2] = {0,0};
	
	for(i=0; i<SFPOUITYPELEN; i++)
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		ret = OLT_ReadI2CRegister( PonPortIdx, DevAddr, 0x60+i, (short int *)data );
		if( (PAS_EXIT_OK  != ret) ||(data[1] != pGwdStr[i]) )
		{
			return VOS_ERROR;
		}
	}
	return VOS_OK;
}

int InitSFPTypeString()
{
	HeadSfpType = VOS_Malloc(sizeof(sfp_type_vendor), MODULE_RPU_PON_MON);
	if( HeadSfpType == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	HeadSfpType->defaultType = 1;
	VOS_StrCpy( HeadSfpType->type_name, SFP_TYPE_Vendor_Name_WTD);
	HeadSfpType->pNext = NULL;
	return VOS_OK;	
}

extern int pon_SFP_MonitorType_read(short int PonPortIdx, char *MonType);
int CheckSFPTypeSupportPowerMetering(short int PonPortIdx)
{
	char SFPTypeString[SFP_TYPE_Vendor_len];
	sfp_type_vendor *curNode=NULL;
       /*  char MonType = 0;*/
	
	if(ponSFPType_read(PonPortIdx, SFPTypeString) != ROK)
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=0;
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"PonPortIdx=%d,read sfp type err,not support rssi\r\n",PonPortIdx));
		return VOS_ERROR;
	}
	else
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=1;
		/*SFP_RSSI_DEBUG(512,("PonPortIdx=%d,%d\r\n",PonPortIdx,PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport));*/
	}
	
	/*if( CheckPonSFPVendorSpecificIsGwd(PonPortIdx) == VOS_ERROR )
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=0;
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"PonPortIdx=%d,read sfp vendor specific err,not support rssi\r\n",PonPortIdx));
		return VOS_ERROR;
	}
	*/
        /*if(pon_SFP_MonitorType_read(PonPortIdx, &MonType) != ROK)
                   return VOS_ERROR;
         if(MonType&0x40)
         {
                   PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=1;
         }
         else
         {
                   PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport=0;
                   SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"PonPortIdx=%d,read sfp type err,not support rssi\r\n",PonPortIdx));
                   return VOS_ERROR;
         }*/
	
	curNode = HeadSfpType;
	if( PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport )
	{	
		VOS_SemTake(opticalpower_semid, WAIT_FOREVER);
		while(curNode)
		{
			if( VOS_StrnCmp(SFPTypeString,curNode->type_name, SFP_TYPE_Vendor_len) == 0)
			{
				if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType != PON_POLARITY_NEGATIVE )
				{
					PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType = PON_POLARITY_NEGATIVE;
					AdcConfig_WTD(PonPortIdx);
					SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"AdcConfig_WTD PonPortIdx=%d\r\n",PonPortIdx));
					SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"\r\nsfptype IN if=%s,curname=%s,PonPortIdx=%d\r\n",SFPTypeString,curNode->type_name,PonPortIdx));
				}
				VOS_SemGive(opticalpower_semid);
				return VOS_OK;
			}
			SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"\r\nsfptype=%s,curname=%s,PonPortIdx=%d\r\n",SFPTypeString,curNode->type_name,PonPortIdx));
			curNode=curNode->pNext;
			/*if((curNode==NULL)&&(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType==PON_POLARITY_NEGATIVE))
			{
				return VOS_OK;
			}*/
		}
		if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType != PON_POLARITY_POSITIVE )
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType = PON_POLARITY_POSITIVE;
			AdcConfig_default(PonPortIdx);
			SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"AdcConfig_default PonPortIdx=%d\r\n",PonPortIdx));
		}
		VOS_SemGive(opticalpower_semid);
	}
	
	return(VOS_OK);
}
int IsSupportRSSI(short int PonPortIdx, struct vty *vty)
{
	int slot, port;

	CHECK_PON_RANGE
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	return(ponSfp_IsSupportRSSI(slot, port, vty));
}

int SFPIsOnline( int slotno, int portno )
{
	short int PonPortIdx;
	int status;
	
	if(SlotCardIsPonBoard(slotno) != ROK)
	{
		return (RERROR);
	}

	PonPortIdx = GetPonPortIdxBySlot(slotno, portno);

	status = GetPonchipWorkingStatus( PonPortIdx );
	if( (status != PONCHIP_UP ) && (status != PONCHIP_TESTING))
		return (RERROR);

	/*return ponSfp_IsOnline(PonPortIdx);*/
	return ROK;
}

 /*int PonSFPSupportRSSI(short int PonPortIdx)
{
	CHECK_PON_RANGE

	if((PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport == V2R1_ENABLE )
		&& (PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm == V2R1_DISABLE))
		return(ROK);
	else
		return(RERROR);
}
*/
int CheckSpePonSFPSupportRSSI(short int PonPortIdx)
{

	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
		return VOS_ERROR;

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"PON %d not working\r\n", PonPortIdx) );
		return VOS_ERROR;
	}

	if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO ) == MODULE_E_GFA6100_MAIN )
	{
		if(2 == GetCardIdxByPonChip(PonPortIdx) )
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
			PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = 0;
			return VOS_ERROR;
		}	
	}
		
	if(IsSupportRSSI(PonPortIdx, NULL) != ROK)
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = 0;
		SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"PON %d not support RSSI\r\n", PonPortIdx) );
		return VOS_ERROR;
	}
	if(CheckSFPTypeSupportPowerMetering(PonPortIdx)!=ROK)
	{
		return VOS_ERROR;
	}

#if 0
	if(ponSfp_IsOnline(PonPortIdx) != ROK)
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
		SFP_RSSI_DEBUG(1, ("PON %d not online\r\n", PonPortIdx) );
		return VOS_ERROR;
	}

		/* 光模块类型检查*/
		if(ponSfp_IsGwdType(PonPortIdx, NULL) != ROK)   /* not match */
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
			SFP_RSSI_DEBUG(1, ("PON %d sfp type mismatch\r\n", PonPortIdx) );
			return VOS_ERROR;
		}
		else
		{  /* match */
			PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_ENABLE;
		}
#endif
	SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"PON %d support RSSI\r\n", PonPortIdx) );

	return VOS_OK;
}

short int FindOpticalScopePonPort(void)
{
	int ret;
	short int PonPortIdx = Optical_Scope_PonPortIdx;
	short int i;
    short int slot = 0;

	for( i=0; i<MAXPON; i++ )
	{
		PonPortIdx++;
		if( PonPortIdx >= MAXPON )
			PonPortIdx = 0;
        /*问题单19167， modified by luh 2013-12-04*/
        slot =  GetCardIdxByPonChip(PonPortIdx);
		if( SYS_MODULE_SLOT_ISHAVECPU(slot) && SYS_LOCAL_MODULE_SLOTNO != slot)
			continue;
		
		if( CheckSpePonSFPSupportRSSI(PonPortIdx) == VOS_OK )
		{
			ret = PonPortHotStatusQuery( PonPortIdx );
			if( ret != V2R1_PON_PORT_SWAP_PASSIVE ) 
			{
				Optical_Scope_PonPortIdx = PonPortIdx;
				onuLaser_alwaysOn_check_support[PonPortIdx] = V2R1_ENABLE;
				return(PonPortIdx);
			}
		}
		onuLaser_alwaysOn_check_support[PonPortIdx] = V2R1_DISABLE;
	}
	return VOS_ERROR;

	/*for(PonPortIdx=Optical_Scope_PonPortIdx; PonPortIdx<MAXPON; PonPortIdx++)
	{
		if(CheckSpePonSFPSupportRSSI(PonPortIdx)!=VOS_OK)
			continue;

		ret = PonPortHotStatusQuery( PonPortIdx );
		if( (ret == RERROR) || (ret == V2R1_PON_PORT_SWAP_PASSIVE) ) 
		{
			continue;
		}

		Optical_Scope_PonPortIdx=PonPortIdx+1;
		if(Optical_Scope_PonPortIdx>=MAXPON) 
			Optical_Scope_PonPortIdx=0;	
		return(PonPortIdx);
	}
	for(PonPortIdx=0; PonPortIdx<Optical_Scope_PonPortIdx; PonPortIdx++)
		{
		if(CheckSpePonSFPSupportRSSI(PonPortIdx)!=VOS_OK)
			continue;	
		else 
			
		Optical_Scope_PonPortIdx=PonPortIdx+1;
		if(Optical_Scope_PonPortIdx>=MAXPON) 
			Optical_Scope_PonPortIdx=0;	
		return(PonPortIdx);
		}
	return VOS_ERROR;*/
}
short int FindOpticalScopePonOnuPort(void)
{
	int ret;
	short int PonPortIdx = Optical_Scope_PonPortIdx_Onu;
	short int i;

	for( i=0; i<MAXPON; i++ )
	{
		PonPortIdx++;
		if( PonPortIdx >= MAXPON )
			PonPortIdx = 0;

		if(PonPortIsWorking(PonPortIdx) == TRUE)
		{
			if(TRUE == HasOnuOnlineOrRegistering(PonPortIdx))
			{
				Optical_Scope_PonPortIdx_Onu = PonPortIdx;
				return(PonPortIdx);
			}
		}
	}
	
	return VOS_ERROR;

}


LONG GetUplinkLosCheckInterval(void)
{
	return (getOpticalPowerThreshold(field_upport_mon_interval, 0));
}
LONG SetUplinkLosCheckInterval(int val)
{
	extern LONG UplinkLosCheckTimerId;
	if( (val < POWER_METERING_INTERVAL_MIN) || (val > POWER_METERING_INTERVAL_MAX) )
		return RERROR;

	if( getOpticalPowerThreshold(field_upport_mon_interval, 0) != val )
	{
		if( UplinkLosCheckTimerId )
			VOS_TimerChange( MODULE_RPU_PON_MON, UplinkLosCheckTimerId, val * 1000 );
	
		setOpticalPowerThreshold(field_upport_mon_interval, val,0);
	}
	return(ROK);
}

LONG GetUplinkPortTransOpticalPowerLowthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_trans_oppower_low, Flag));
}
LONG SetUplinkPortTransOpticalPowerLowthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_trans_oppower_low,val,Flag);
	return(ROK);
}

LONG GetUplinkPortTransOpticalPowerHighthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_trans_oppower_high, Flag));
}
LONG SetUplinkPortTransOpticalPowerHighthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_trans_oppower_high,val, Flag );
	return(ROK);
}

LONG GetUplinkPortRecvOpticalPowerLowthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_recv_oppower_low, Flag));
}
LONG SetUplinkPortRecvOpticalPowerLowthrd(long val, int Flag )
{
	setOpticalPowerThreshold(field_upport_recv_oppower_low,val, Flag );
	return(ROK);
}

LONG GetUplinkPortRecvOpticalPowerHighthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_recv_oppower_high,Flag));
}
LONG SetUplinkPortRecvOpticalPowerHighthrd(long val ,int Flag)
{
	setOpticalPowerThreshold(field_upport_recv_oppower_high,val, Flag );
	return(ROK);
}

LONG GetUplinkPortTemperatureLowthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_tempe_low,Flag));
}
LONG SetUplinkPortTemperatureLowthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_tempe_low,val,Flag);
	return(ROK);
}
LONG GetUplinkPortTemperatureHighthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_tempe_high,Flag));
}
LONG SetUplinkPortTemperaturreHighthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_tempe_high,val,Flag);
	return(ROK);
}

LONG GetUplinkPortVoltageLowthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_vol_low,Flag));
}
LONG SetUplinkPortVoltageLowthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_vol_low,val, Flag);
	return(ROK);
}

LONG GetUplinkPortVoltageHighthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_vol_high,Flag));
}
LONG SetUplinkPortVoltageHighthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_vol_high,val, Flag );
	return(ROK);
}

LONG GetUplinkPortBiasCurrentLowthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_cur_low,Flag));
}
LONG SetUplinkPortBiasCurrentLowthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_cur_low,val, Flag);
	return(ROK);
}

LONG GetUplinkPortBiasCurrentHighthrd(int Flag)
{
	return(getOpticalPowerThreshold(field_upport_cur_high,Flag));
}
LONG SetUplinkPortBiasCurrentHighthrd(long val, int Flag)
{
	setOpticalPowerThreshold(field_upport_cur_high,val, Flag);
	return(ROK);
}
int ReInitWhenUplinkRecoverFromLOS(short int slotno, short int sfp_sel)
{
	int portno;
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	UplinkPortMeteringInfo[portno].AlarmStatus=0;
	UplinkPortMeteringInfo[portno].LosAlarmFlag=2;
	UplinkPortMeteringInfo[portno].transOpticalPower=-50;
	UplinkPortMeteringInfo[portno].recvOpticalPower=-50;
	UplinkPortMeteringInfo[portno].Temperature=40;
	UplinkPortMeteringInfo[portno].Voltage=33;
	UplinkPortMeteringInfo[portno].BiasCurrent=15;
	return VOS_OK;
}

int ClearUplinkThirdClassAlarm(short int slotno,short int sfp_sel)
{
	long val=0;
	int portno, Flag = 0;
	short int portoffset = 1;
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||  (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = 17;
	}
	
	val=GetUplinkPortRecvOpticalPowerLowthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_RXPOW_L)!=0)
	{
		UplinkSFPRecvPowerLowClear_EventReport( 1, slotno,sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_RXPOW_L);
	}
	val=GetUplinkPortRecvOpticalPowerHighthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_RXPOW_H)!=0)
	{
		UplinkSFPRecvPowerHighClear_EventReport( 1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_RXPOW_H);
	}
	val=GetUplinkPortTransOpticalPowerLowthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_TXPOW_L)!=0)
	{
		UplinkSFPTransPowerLowClear_EventReport( 1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_TXPOW_L);
	}
	val=GetUplinkPortTransOpticalPowerHighthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_TXPOW_H)!=0)
	{
		UplinkSFPTransPowerHighClear_EventReport( 1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_TXPOW_H);
	}
	return VOS_OK;	
}

int ClearAllUplinkAlarmWhenLOS(short int slotno, short int sfp_sel)
{
	long val = 0;
	int portno, Flag = 0;
	short int portoffset = 1;
	
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||  (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET))
		Flag = 1;
	else
		Flag = 0;

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = 17;
	}
	
	portno = slotno*MAXUPLINKPORT + sfp_sel;

	val = GetUplinkPortTemperatureLowthrd( Flag );
	if( (UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TEMP_L) != 0 )
	{
		UplinkSFPTemperatureLowClear_EventReport(1, slotno, sfp_sel+portoffset, val );
		UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_TEMP_L);
	}
	val = GetUplinkPortTemperatureHighthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_TEMP_H)!=0)
	{
		UplinkSFPTemperatureHighClear_EventReport(1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_TEMP_H);
	}
	val = GetUplinkPortBiasCurrentLowthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_BIAS_L)!=0)
	{
		UplinkSFPBiasCurrentLowClear_EventReport(1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_BIAS_L);
	}
	val=GetUplinkPortBiasCurrentHighthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_BIAS_H)!=0)
	{
		UplinkSFPBiasCurrentHighClear_EventReport(1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_BIAS_H);
	}
	val=GetUplinkPortVoltageLowthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_VOLT_L)!=0)
	{
		UplinkSFPVoltageLowClear_EventReport(1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_VOLT_L);
	}
	val=GetUplinkPortVoltageHighthrd( Flag );
	if((UplinkPortMeteringInfo[portno].AlarmStatus&UPLINK_ALARM_STATE_VOLT_H)!=0)
	{
		UplinkSFPVoltageHighClear_EventReport(1, slotno, sfp_sel+portoffset,val);
		UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_VOLT_H);
	}
		
	ClearUplinkThirdClassAlarm(slotno, sfp_sel);
	
	return VOS_OK;
}

extern int ponPortLosAlarm_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx);
extern int ponPortLosAlarmClear_EventReport( ulong_t devIdx, ulong_t brdIdx, ulong_t ponIdx);	

int ReadRatefromUplinkSfp(unsigned int slotno,unsigned int portno, ULONG* rate)
{
	int ret;
	UCHAR value = 0;
	*rate = 0;

	if( TRUE == i2c_data_get( slotno,I2C_BASE_GPIO+portno-1,0,&value,1) ) 
	{
		if( value & 1<<(portno-1) )
		{
			/*sys_console_printf("Can't get rate from sfp\n");*/
			return VOS_ERROR;
		}
	}
	else
	{
		return VOS_ERROR;
	}
	
	ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+portno-1, XcvrInfoArr[5].cAddr,(unsigned char *)&value, 1);
	
	if(ret == TRUE)
	{
		if(value > 0x09 && value < 0x10)
		{
			*rate = 1000; 
		}
		else if(value > 0x60 && value < 0x70)
		{
			*rate = 10000;
		}
		else
		{
			return VOS_ERROR;
		}
	}
	else
	{
		return VOS_ERROR;
	}

	return VOS_OK;
}


int CheckUplinkLOSState(short int slotno,short int sfp_sel)
{
	char data,Losalarmflag;
	short int ret;
	_XCVR_DATA_ *pXcvrArr;
	int portno;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	pXcvrArr = XcvrDiagArr;
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	Losalarmflag=UplinkPortMeteringInfo[portno].LosAlarmFlag;	
	ret = sfp_i2c_read(slotno, sfp_sel+SFP_OFFSET, pXcvrArr[39].cAddr, &data, 1);
	SFP_RSSI_DEBUG(0x100, (LOG_OPTICAL_POWER,LOG_INFO,"ret=%d sfp_sel=%d data=%x\r\n",ret,sfp_sel,data));
	if(ret==TRUE)
	{
		if(data==0x02)
		{
			if((Losalarmflag!=1)&&(Losalarmflag!=2))
			{
				ClearAllUplinkAlarmWhenLOS(slotno, sfp_sel);
				ponPortLosAlarm_EventReport( 1, slotno, sfp_sel+1 );
				UplinkPortMeteringInfo[portno].AlarmStatus|=UPLINK_ALARM_STATE_LOS;
				SFP_RSSI_DEBUG(0x100, (LOG_OPTICAL_POWER,LOG_INFO,"data=%x\r\n",data));
			}
			UplinkPortMeteringInfo[portno].LosAlarmFlag=1;
		}
		else 
		{
			if(Losalarmflag==1)
			{
				ReInitWhenUplinkRecoverFromLOS(slotno, sfp_sel);
				ponPortLosAlarmClear_EventReport( 1, slotno, sfp_sel+1 );
			}
			UplinkPortMeteringInfo[portno].AlarmStatus&=(~UPLINK_ALARM_STATE_LOS);
			UplinkPortMeteringInfo[portno].LosAlarmFlag=0;
		}	
		return VOS_OK;
	}
	else
	{
		return VOS_ERROR;
		/*sys_console_printf("slot is %d, port is %d\r\n",slotno,sfp_sel+1);*/
	}
}

long GponTranslateOpticalPower( unsigned short int OpticalPower)
{
	double miliwatt;
	long ret = 0;
	#define TWO_POW_16_MAX (65536)

	if(OpticalPower >= TWO_POW_16_MAX / 2)
	{
		ret = ((double)(OpticalPower - TWO_POW_16_MAX) * 0.002)*10 ;	
	}
	else
	{
		ret = ((double)(OpticalPower) * 0.002)*10;
	}
		
	return ret;
}

/* modified by xieshl 20160531, 支持不同量化单位的光模块 */
long TranslateOpticalPower_QUnit( unsigned short int OpticalPower, int qunit)
{
	double miliwatt;
	long ret = 0;

	miliwatt = (double)(qunit * (int)OpticalPower / 10000.0);

	if( miliwatt !=0 )
		ret = (long)((100.0*log10(miliwatt)) + 0.5);

	return ret;
}
/*功率转换，默认量化单位0.1uw*/
long TranslateOpticalPower( unsigned short int OpticalPower)
{
	double miliwatt;
	long ret = 0;

	miliwatt = (double)(OpticalPower / 10000.0);

	if( miliwatt !=0 )
		ret = (long)((100.0*log10(miliwatt)) + 0.5);

	return ret;
}

long uplinkTranslateOpticalPower( unsigned short int OpticalPower)
{
	long ret = uplink_rx_optical_power_default;
	double miliwatt;

	miliwatt = (OpticalPower / 10000.0);
	if( miliwatt !=0 )
		ret = (long)((100.0*log10(miliwatt)) + 0.5);
	return ret;
}


/*温度转换*/
long TranslateTemperature( short int Temperature)
{
	signed char flag=0;

	if(Temperature & 0x8000)
	{
		Temperature = (~Temperature)+1;
		if((Temperature & 0xff) >= 127 )
			flag =-1;
		Temperature = Temperature *(-1);
	}
	else
	{
		if((Temperature & 0xff) >= 127 )
			flag = 1;
	}
	return(long)(Temperature/256 + flag);
}

/* modified by xieshl 20160531, 支持不同量化单位的光模块 */
long TranslateBiasCurrent_QUnit( unsigned short int BiasCurrent, int qunit )
{
	return(long)(((BiasCurrent * qunit + 500) / 1000));
}
/* 偏置电流转换，默认量化单位2uA */
long TranslateBiasCurrent( unsigned short int BiasCurrent)
{
	/*unsigned char flag =0;
	if((((BiasCurrent * 2) / 100) % 10 ) >= 5)
		flag = 1;
	return(long)(((BiasCurrent * 2) / 1000)+flag);*/
	return(long)(((BiasCurrent * OPT_BIAS_CUR_QU_DEF + 500) / 1000));
}

/*工作电压转换*/
long TranslateWorkVoltage(unsigned short int WorkVoltage)
{
	/*unsigned char flag =0;
	if(((WorkVoltage / 100) % 10 ) >= 5)
		flag = 1;
	return(long)((WorkVoltage/1000)+flag);*/
	return(long)((WorkVoltage+500)/1000);
}

/* 1 PON口光模块发射光功率*/
int ReadsfpTxPower(short int PonPortIdx, long *power)
{
	int i;
	int ret;
	_XCVR_DATA_  *pXcvrArr;
	short int data1, data2;

	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) || (power == NULL) )
	{
		return VOS_ERROR;
	}
		
	pXcvrArr = XcvrDiagArr;

	i = 37;
	data1=0;
	data2=0;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr, &data1);
	ret += OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr+1, &data2);
	SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"\n pon Transpower data=%x%02x\r\n",data1,data2));
	if(ret == PAS_EXIT_OK)
	{
		if( opt_tx_pwr_qunit != OPT_TX_PWR_QU_DEF )
			*power = TranslateOpticalPower_QUnit((data1 << 8 ) + data2, opt_tx_pwr_qunit);
		else
			*power = TranslateOpticalPower((data1 << 8 ) + data2);
		return VOS_OK;
	}
	return VOS_ERROR;
}

/*2 PON口光模块工作温度*/
int ReadsfpTemperature(short int PonPortIdx,long *temperature)
{
	int ret;
	int i;
	_XCVR_DATA_  *pXcvrArr;
	short int  data1, data2;
	
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) || (temperature == NULL) )
	{
		return VOS_ERROR;
	}
	
	pXcvrArr = XcvrDiagArr;
	
	i = 34;
	data1=0;
	data2=0;
    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr, (short int *)&data1);
	ret += OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr+1, (short int *)&data2);
	SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"\n pon tempwerature data=%x%02x\r\n",data1,data2));
	if(PAS_EXIT_OK == ret)
	{		
		*temperature=TranslateTemperature((data1<<8 )+data2);
		return VOS_OK;
	}
	return VOS_ERROR;
}

/*3 PON口工作电压*/
int ReadsfpVoltage(short int PonPortIdx,long *voltage)
{
	int i;
	int ret;
	_XCVR_DATA_  *pXcvrArr;
	unsigned short int  data1, data2;

	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) || (voltage == NULL) )
	{
		return VOS_ERROR;
	}
	
	pXcvrArr = XcvrDiagArr;

	i = 35;
	data1 = 0;
	data2 = 0;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_ReadI2CRegister(PonPortIdx,A2H_1010001X, pXcvrArr[i].cAddr, &data1);
	ret += OLT_ReadI2CRegister(PonPortIdx,A2H_1010001X, pXcvrArr[i].cAddr+1, &data2);
	if( ret == PAS_EXIT_OK)
	{
		*voltage=TranslateWorkVoltage((data1<<8 )+data2);
		return VOS_OK;
	}
	return VOS_ERROR;
}

/*4PON口偏置电流*/
int ReadsfpBias(short int PonPortIdx,long *bias)
{
	int i;
	int ret;
	_XCVR_DATA_  *pXcvrArr;
	unsigned short int  data1, data2;

	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) || (bias == NULL) )
	{
		return VOS_ERROR;
	}
 
	pXcvrArr = XcvrDiagArr;
 
	i = 36;
	data1=0;
	data2=0;
	/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret=OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr, &data1);
	ret=OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X, pXcvrArr[i].cAddr+1, &data2);
	
	if(PAS_EXIT_OK == ret)
	{
		if( opt_biascur_qunit != OPT_BIAS_CUR_QU_DEF )
			*bias=TranslateBiasCurrent_QUnit((data1<<8 )+data2, opt_biascur_qunit);
		else
			*bias=TranslateBiasCurrent((data1<<8 )+data2);
		return VOS_OK;
	}
	return VOS_ERROR;
}

/* 5读接收ONU 光功率*/
int  ReadPowerMeteringOnu( short int PonPortIdx, short int OnuIdx,long *power)
{
	short int Llid,i;
	short int ret = PAS_EXIT_OK;
	PON_rssi_result_t         rssi_result;
	short int data1, data2;
	unsigned short int rx_pwr_ad;
	UINT slot,port;
	float rx_pwr4=0, rx_pwr3=0, rx_pwr2=0, rx_pwr1=0 /*0.44807*/, rx_pwr0=0;
	double rx_pwr;

	/*#define I2C_BASE_SFP 0x00*/
    #define I2C_BASE_SFP2 0xc0
	/*double miliwatt = 0;*/
	
	_XCVR_DATA_  *pXcvrArr;

	i = 38;
	data1=0;
	data2=0;
	
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) || (power== NULL) )
	{
		return VOS_ERROR;
	}	
	
	Llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(Llid == INVALID_LLID ) 
		return(RERROR );
	
	pXcvrArr = XcvrDiagArr;

	slot = GetCardIdxByPonChip(PonPortIdx);
    port = GetPonPortByPonChip(PonPortIdx);
    /* B--modified by liwei056@2014-1-7 for D19321 */
#if 0
	#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	ret = OLT_GetVirtualScopeMeasurement(PonPortIdx, Llid, PON_VIRTUAL_MEASUREMENT_RSSI, NULL, 0, &rssi_result, sizeof(PON_rssi_result_t));
	#else
	ret = PAS_get_virtual_scope_measurement(PonPortIdx, Llid, PON_VIRTUAL_MEASUREMENT_RSSI,NULL,&rssi_result );
	#endif
#else    
	/*if(!SYS_LOCAL_MODULE_TYPE_IS_8000_GPON)*/
	{
		ret = OLT_GetVirtualScopeRssiMeasurement(PonPortIdx, Llid, &rssi_result);
	}
#endif
    /* E--modified by liwei056@2014-1-7 for D19321 */

	VOS_TaskDelay(1);
	
	if(ret == PAS_EXIT_OK)
	{
		VOS_TaskDelay(10);
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[20].cAddr,(unsigned char *)&rx_pwr4,4);
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[21].cAddr,(unsigned char *)&rx_pwr3,4);
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[22].cAddr,(unsigned char *)&rx_pwr2,4);
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[23].cAddr,(unsigned char *)&rx_pwr1,4);
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[24].cAddr,(unsigned char *)&rx_pwr0,4);
			ret = i2c_data_get(slot,I2C_BASE_SFP2 + port - 1,pXcvrArr[38].cAddr,(unsigned char *)&rx_pwr_ad,2);

			if(ret == TRUE)
			{
				rx_pwr = (((rx_pwr4 * rx_pwr_ad + rx_pwr3) * rx_pwr_ad + rx_pwr2) * rx_pwr_ad + rx_pwr1) * rx_pwr_ad + rx_pwr0;
				*power=TranslateOpticalPower(rx_pwr);
				return VOS_OK;
			}
			
		}
		else
		{
			ret = OLT_ReadI2CRegister(PonPortIdx, A2H_1010001X,pXcvrArr[i].cAddr,&data1);
			ret += OLT_ReadI2CRegister(PonPortIdx,A2H_1010001X,pXcvrArr[i].cAddr+1,&data2);
		
		
		
		if(ret == PAS_EXIT_OK)
		{			
			/*miliwatt = (double)((( data1 << 8 ) + data2) / 10000.0);
			if(miliwatt == 0)
				return VOS_ERROR;*/
			if( opt_tx_pwr_qunit != OPT_TX_PWR_QU_DEF )
 				*power=TranslateOpticalPower(( data1 << 7 ) + data2);
			else
 				*power=TranslateOpticalPower(( data1 << 8 ) + data2);
			SFP_RSSI_DEBUG(1, (LOG_OPTICAL_POWER,LOG_INFO,"ReadPowerMeteringOnu %d, %d,rec_power=%d, 0x%x%02x\r\n", PonPortIdx, OnuIdx,*power, data1, data2));
				return VOS_OK;
		}
		}
	}
	return VOS_ERROR;
}

#if 0
int ComputeTimeInterverl()
{
	ULONG start_tick,end_tick,time;
	int i;
	int data=0;
	start_tick=VOS_GetTick();
	for(i=1;i<5000;i++) 
		{
		i2c_read_all2(1,0,1,110,(unsigned char *)&data);
		}
	end_tick=VOS_GetTick();
	time=end_tick-start_tick;
	sys_console_printf("\n time = %d\r\n",time);
	return VOS_OK;
}
#endif

/*上联口光模块发送光功率*/
int ReadUplinkPortSFPTransPower(short int slotno, int sfp_sel, long *val)
{
	int ret;
	_XCVR_DATA_  *pXcvrArr;
	short int data;
	
	pXcvrArr = XcvrDiagArr;
	data=0;
	*val=-50;

	if( (sfp_sel<0)||(sfp_sel>3))
	{
		return VOS_ERROR;
	}
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)/*偏移地址不一样,add by yanjy,2016.8,GPON 地址*/
		ret = i2c_data_get(slotno,0xd0+sfp_sel, pXcvrArr[37].cAddr,(unsigned char *)&data, 2);
		else 
		ret = i2c_data_get(slotno,sfp_sel, pXcvrArr[37].cAddr,(unsigned char *)&data, 2);
	}
	else
	{
		ret=sfp_i2c_read(slotno, SFP_OFFSET+sfp_sel, pXcvrArr[37].cAddr, (unsigned char *)&data, 2);
		/*SFP_RSSI_DEBUG(256,("\nret=%d,Transpower data=%x\r\n",ret,data));*/
	}
	if(ret == TRUE)
	{
		*val=TranslateOpticalPower(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;
}
/*上联口光模块接收光功率*/
/*int ReadUplinkPortSFPRecvPower(int sfp_sel)
{
	int ret;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data;
	long val;
	
	pXcvrArr=XcvrDiagArr;
	data=0;
	val=-50;
	if( (sfp_sel<0)||(sfp_sel>3))
	{
		return VOS_ERROR;
	}	
	ret=sfp_i2c_read(1,sfp_sel,1,pXcvrArr[38].cAddr,(unsigned char *)&data, 2);
	if(ret == TRUE)
	{
		if(data<0x20)
		{
			data=0x20;
		}
		val=TranslateOpticalPower(data);
	}
	return (val);	
}*/
/* 需要外校准 */
int ReadUplinkPortSFPRecvPower(short int slotno, int sfp_sel, long *val)
{
	int ret;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int rx_pwr_ad;
	float rx_pwr4=0, rx_pwr3=0, rx_pwr2=0, rx_pwr1=0 /*0.44807*/, rx_pwr0=0;
	/*float rx_pwr1=0.44807;*/
	/*long val = uplink_rx_optical_power_default;*/
	double rx_pwr;
	
	pXcvrArr=XcvrDiagArr;
	rx_pwr_ad=0;
	*val=-50;
	if( (sfp_sel<0)||(sfp_sel>3))
	{
		return VOS_ERROR;
	}
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		{
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[20].cAddr,(unsigned char *)&rx_pwr4, 4 );
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[21].cAddr,(unsigned char *)&rx_pwr3, 4 );
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[22].cAddr,(unsigned char *)&rx_pwr2, 4 );		
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[23].cAddr,(unsigned char *)&rx_pwr1, 4 );
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[24].cAddr,(unsigned char *)&rx_pwr0, 4 );
			ret=i2c_data_get( slotno , 0xd0+sfp_sel, pXcvrArr[38].cAddr,(unsigned char *)&rx_pwr_ad, 2 );
		}
		else if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
		{
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[20].cAddr,(unsigned char *)&rx_pwr4, 4 );
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[21].cAddr,(unsigned char *)&rx_pwr3, 4 );
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[22].cAddr,(unsigned char *)&rx_pwr2, 4 );		
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[23].cAddr,(unsigned char *)&rx_pwr1, 4 );
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[24].cAddr,(unsigned char *)&rx_pwr0, 4 );
			ret=i2c_data_get( slotno , sfp_sel, pXcvrArr[38].cAddr,(unsigned char *)&rx_pwr_ad, 2 );
		}
	}
	else
	{
		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[20].cAddr,(unsigned char *)&rx_pwr4, 4 );

		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[21].cAddr,(unsigned char *)&rx_pwr3, 4 );
		
		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[22].cAddr,(unsigned char *)&rx_pwr2, 4 );
		
		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[23].cAddr,(unsigned char *)&rx_pwr1, 4 );
		
		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[24].cAddr,(unsigned char *)&rx_pwr0, 4 );

		ret=sfp_i2c_read( slotno , SFP_OFFSET+sfp_sel, pXcvrArr[38].cAddr,(unsigned char *)&rx_pwr_ad, 2 );
	}
	if(ret == TRUE)
	{
		if( UplinkPortMeteringInfo[slotno*MAXUPLINKPORT+sfp_sel].powerMeteringSupport == EXTERNALLY_CALIBRATED )
		{
				rx_pwr = (((rx_pwr4 * rx_pwr_ad + rx_pwr3) * rx_pwr_ad + rx_pwr2) * rx_pwr_ad + rx_pwr1) * rx_pwr_ad + rx_pwr0;
		}
		else
			rx_pwr = rx_pwr_ad;
		
		*val = uplinkTranslateOpticalPower(rx_pwr);
		return VOS_OK;
	}
	else
		return VOS_ERROR;	
}


/* 上联口光模块工作温度*/
int ReadUplinkPortSFPTemperature(short int slotno,int sfp_sel, long *val)	
{
	int ret;
	_XCVR_DATA_ *pXcvrArr;
	short int data;

	pXcvrArr=XcvrDiagArr;
	data=0;
	*val=40;
	if( (sfp_sel<0)||(sfp_sel>=MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		ret = i2c_data_get(slotno,0xd0+sfp_sel, pXcvrArr[34].cAddr,(unsigned char *)&data, 2);
		else
		ret = i2c_data_get(slotno,sfp_sel, pXcvrArr[34].cAddr,(unsigned char *)&data, 2);	
	}
	else
	{
		ret=sfp_i2c_read(slotno,SFP_OFFSET+sfp_sel, pXcvrArr[34].cAddr,(unsigned char *)&data, 2);
		/*SFP_RSSI_DEBUG(256,("\nret=%d,temperature data=%x  \r\n",ret,data));*/
	}
	if(ret == TRUE)
	{
		*val=TranslateTemperature(data);	
		return VOS_OK;
	}
	else
		return VOS_ERROR;
}

/* 上联口光模块工作电压*/
int ReadUplinkPortSFPVoltage(short int slotno, int sfp_sel, long *val)
{
	int ret;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data;

	pXcvrArr=XcvrDiagArr;
	data=0;
	*val=33;

	if( (sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		ret = i2c_data_get(slotno,0xd0+sfp_sel,pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
		else
		ret = i2c_data_get(slotno,sfp_sel,pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
	}
	else
	{
		ret=sfp_i2c_read(slotno,SFP_OFFSET+sfp_sel, pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
	}
	if(ret == TRUE)
	{
		*val=TranslateWorkVoltage(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;	
}
/* 上联口光模块偏置电流*/
int ReadUplinkPortSFPBiasCurrent(short int slotno, int sfp_sel, long *val)
{
	int ret;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data;
	
	pXcvrArr=XcvrDiagArr;
	data=0;
	*val=15;

	if((sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		ret = i2c_data_get(slotno,0xd0+sfp_sel, pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
		else
		ret = i2c_data_get(slotno,sfp_sel, pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
	}
	else
	{
		ret=sfp_i2c_read(slotno, SFP_OFFSET + sfp_sel,pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
	}
	if(ret == TRUE)
	{
		*val=TranslateBiasCurrent(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;		
}


static int setUplinkPortSFPUpdatedFlag[20];
int setUplinkPortSFPUpdated(short int slotno,int sfp_sel)
{
	UCHAR val[2] = {0, 0};
	
	if( (sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		return bcm_port_phy_set(USER_PORT_2_SWITCH_UNIT(slotno,sfp_sel),USER_PORT_2_SWITCH_PORT(slotno, sfp_sel),8,0x50006F,val);
	}
	
	return sfp_i2c_write( slotno, SFP_OFFSET+sfp_sel, 111, val, 1 );
}

int checkUplinkPortSFPUpdated( int slotno, int sfp_sel)
{
	int i, ret;
	UCHAR val[2];
	
	if( (sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return FALSE;
	}

	if( setUplinkPortSFPUpdatedFlag[slotno] == 0 )
	{
		setUplinkPortSFPUpdated(slotno, sfp_sel);
		setUplinkPortSFPUpdatedFlag[slotno] = 1;
	}
	for( i=0; i<2; i++ )
	{
		val[0] = 0;
		ret = sfp_i2c_read( slotno, SFP_OFFSET+sfp_sel, 111, val, 1 );
		if( ret && ((val[0] & 0xf8) == 0xf8) )/*现在第111 字节只使用了五个比特*/
		{
			SFP_RSSI_DEBUG(0x100,(LOG_OPTICAL_POWER,LOG_INFO,"uplink %d/%d sfp monitor data updated Success !\r\n", slotno, sfp_sel+1));
			return ret;
		}
		VOS_TaskDelay(20);
	}
	SFP_RSSI_DEBUG(0x100,(LOG_OPTICAL_POWER,LOG_INFO,"uplink %d/%d sfp monitor data updated err\r\n", slotno, sfp_sel+1));
	return FALSE;
}

/*上联口光功率检测接口函数*/
int GetUplinkPortTransOpticalPower(short int slotno, int sfp_sel)
{
	int power;

	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	power = UplinkPortMeteringInfo[slotno*MAXUPLINKPORT + sfp_sel].transOpticalPower ;
	VOS_SemGive(PowerMeteringSemId);

	return power;	
}
int GetUplinkPortRecvOpticalPower(short int slotno, int sfp_sel)
{
	int power = uplink_rx_optical_power_default;

	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	power = UplinkPortMeteringInfo[slotno*MAXUPLINKPORT + sfp_sel].recvOpticalPower;
	VOS_SemGive(PowerMeteringSemId);

	return power;	
}
int GetUplinkPortTemperature(short int slotno, int sfp_sel)
{
	int tem,portno;
	
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	portno = slotno*MAXUPLINKPORT + sfp_sel;

	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	tem = UplinkPortMeteringInfo[portno].Temperature;
	VOS_SemGive(PowerMeteringSemId);

	return tem;	
}
int GetUplinkPortVoltage(short int slotno, int sfp_sel)
{
	int vol;

	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	vol = UplinkPortMeteringInfo[slotno*MAXUPLINKPORT + sfp_sel].Voltage;
	VOS_SemGive(PowerMeteringSemId);

	return vol;	
}
int GetUplinkPortBiasCurrent(short int slotno, int sfp_sel)
{
	int bias;

	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	bias = UplinkPortMeteringInfo[slotno*MAXUPLINKPORT + sfp_sel].BiasCurrent;
	VOS_SemGive(PowerMeteringSemId);

	return bias;	
}

/*PON功率检测接口函数*/
int GetPonPortTransOpticalPower(short int PonPortIdx)
{
	int power;
	CHECK_PON_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );	
	power = (PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower /*+ olt_tx_optical_power_calibration*/);
	VOS_SemGive(PowerMeteringSemId);
	return power;
}

int GetPonPortRecvOpticalPower(short int PonPortIdx, short int OnuIdx)
{
	int power;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );

	if(  PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[ OnuIdx ] == 1)
		power = PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx] /*+olt_rx_optical_power_calibration*/;
	else
		power = -1000;
	
	VOS_SemGive(PowerMeteringSemId);
	return power;
}

int GetPonPortTemperature(short int PonPortIdx)
{
	int tem;
	CHECK_PON_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	tem=PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature;
	VOS_SemGive(PowerMeteringSemId);
	return tem;
}

int GetPonPortWorkVoltage(short int PonPortIdx)
{
	int vol;
	CHECK_PON_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	vol = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied;
	VOS_SemGive(PowerMeteringSemId);
	return vol;
}

int GetPonPortBiasCurrent(short int PonPortIdx)
{
	int bias;
	CHECK_PON_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	bias=PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent;
	VOS_SemGive(PowerMeteringSemId);
	return bias;
}
long getOltRxOpticalPowerCalibration()
{
	return olt_rx_optical_power_calibration;
}
long getOltTxOpticalPowerCalibration()
{
	return olt_tx_optical_power_calibration;
}
int setOltOpticalPowerCalibration( long rx_calibration, long tx_calibration )
{
	olt_rx_optical_power_calibration = rx_calibration;
	olt_tx_optical_power_calibration = tx_calibration;
	return VOS_OK;
}
long getOnuRxOpticalPowerCalibration()
{
	return onu_rx_optical_power_calibration;
}
long getOnuTxOpticalPowerCalibration()
{
	return onu_tx_optical_power_calibration;
}
static int setOnuOpticalPowerCalibration( long rx_calibration, long tx_calibration )
{
	onu_rx_optical_power_calibration = rx_calibration;
	onu_tx_optical_power_calibration = tx_calibration;
	return VOS_OK;
}
LONG GetPonPortOpticalMonitorInterval(void)
{
	return (getOpticalPowerThreshold(field_olt_mon_interval,0));
}

LONG SetPonPortOpticalMonitorInterval(int val)
{
	extern LONG PonpoweringTimerId;
	if( (val < POWER_METERING_INTERVAL_MIN) || (val > POWER_METERING_INTERVAL_MAX) )
		return RERROR;

	if( getOpticalPowerThreshold(field_olt_mon_interval,0) != val )
	{
		if( PonpoweringTimerId )
			VOS_TimerChange( MODULE_RPU_PON_MON, PonpoweringTimerId, val * 1000 );
	
		setOpticalPowerThreshold(field_olt_mon_interval, val,0);
	}
	return(ROK);
}
LONG GetPonPortOpticalMonitorEnable(void)
{
	return (getOpticalPowerThreshold(field_olt_monitor_enable,0));
}

LONG ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss(int PonPortIdx)
{
#if 0
	for(i = 0; i<MAXPON; i++)
	{
		if(onuLaser_alwaysOn_alarm_record[i][0] != 0)
		{
			ponPortLaserAlwaysOnClear_EventReport( GetCardIdxByPonChip( i), GetPonPortByPonChip(i), 0, 0 );
			onuLaser_alwaysOn_alarm_record[i][0] = 0;
			onuLaser_alwaysOn_alarm_record[i][1] = 0;
			onuLaser_alwaysOn_alarm_flag[i][0] =0;
			onuLaser_alwaysOn_alarm_flag[i][1] =0;
		}
	}
#else
	if( PonPortIdx >= MAXPON)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	
	if(onuLaser_alwaysOn_alarm_record[PonPortIdx][0] != 0)
	{
		onuLaser_alwaysOn_alarm_record[PonPortIdx][0] = 0;
		onuLaser_alwaysOn_alarm_record[PonPortIdx][1] = 0;
		onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] =0;
		onuLaser_alwaysOn_alarm_flag[PonPortIdx][1] =0;
		Optical_Timer_Callback();
		ponPortLaserAlwaysOnClear_EventReport( GetCardIdxByPonChip( PonPortIdx), GetPonPortByPonChip(PonPortIdx), 0, 0 );
	}
#endif
	return VOS_OK;

}

long ClearAllAlarmsForOpticalPower( )
{
	int sfp_sel, max_uplink_sfp, slotno, OnuIdx ;
	LONG max_uplink_slotno = 1, PonPortIdx;

	if( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
	{
		if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
			max_uplink_slotno = 1;
		else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)/* wangysh mod 20110303 6700 support slot2 uplink */
	       {   
#ifdef _SUPPORT_8GE_6700_
	              if (SYS_CHASSIS_IS_V2_8GE)
			    max_uplink_slotno = 2;
	              else
	                  max_uplink_slotno = 1;
#else
			max_uplink_slotno = 1;
#endif
	       }
		else
			max_uplink_slotno = SYS_CHASSIS_SWITCH_SLOTNUM;

		for( slotno=1; slotno<=max_uplink_slotno; slotno++ )
		{
			max_uplink_sfp = GetMAXUplinkPort(slotno);
			if( max_uplink_sfp == 0 )
				continue;
			
			for( sfp_sel=0; sfp_sel<max_uplink_sfp; sfp_sel++ )
			{
				ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
			}
		}
	}
	
	if( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
	{
		for(PonPortIdx=0; PonPortIdx< MAXPON; PonPortIdx++ )
		{
			ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( PonPortIdx );
			
			ClearOpticalPowerAlarmWhenPonPortDown( PonPortIdx );
			
			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
			{
				if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP)
					continue;

				ClearOpticalPowerAlamWhenOltDetectionDisable( PonPortIdx, OnuIdx );
			}
		}
	}

	return VOS_OK;

}
	
int ClearAllAlarmsForOpticalPower_CardOut( LONG ulSLot, LONG module_type )
{
	int sfp_sel, max_uplink_sfp, slotno, OnuIdx ;
	LONG max_uplink_slotno = 1, PonPortIdx;
	int slot /*,port*/ ;
	
	if( ! SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
		return VOS_OK;
	if (module_type <= MODULE_TYPE_UNKNOW)
		return VOS_OK;
	
	if( SYS_MODULE_IS_UPLINK( ulSLot ) || SYS_MODULE_IS_UPLINK_PON(ulSLot) )
	{
		if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
			max_uplink_slotno = 1;
		else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)/* wangysh mod 20110303 6700 support slot2 uplink */
	       {   
#ifdef _SUPPORT_8GE_6700_
	              if (SYS_CHASSIS_IS_V2_8GE)
			    max_uplink_slotno = 2;
	              else
	                  max_uplink_slotno = 1;
#else
			max_uplink_slotno = 1;
#endif
	       }
		else
			max_uplink_slotno = SYS_CHASSIS_SWITCH_SLOTNUM;

		for( slotno=1; slotno<=max_uplink_slotno; slotno++ )
		{
			if( slotno != ulSLot )
				continue;
			
			max_uplink_sfp = GetMAXUplinkPort(slotno);
			if( max_uplink_sfp == 0 )
				continue;
			
			for( sfp_sel=0; sfp_sel<max_uplink_sfp; sfp_sel++ )
			{
				ClearAllUplinkAlarmWhenLOS( slotno, sfp_sel );
			}
		}
		
	}

	if( ROK == SlotCardIsPonBoard(ulSLot) )
	{
		for(PonPortIdx=0; PonPortIdx< MAXPON; PonPortIdx++ )
		{
			slot = GetCardIdxByPonChip(PonPortIdx);
			/*port = GetPonPortByPonChip(PonPortIdx);*/
			
			if( slot != ulSLot )
				continue;
			
			ClearOpticalPowerAlarmWhenPonPortDown( PonPortIdx );
			
			for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
			{
				ClearOpticalPowerAlamWhenOnuOffline( PonPortIdx, OnuIdx );
			}

			if(onuLaser_alwaysOn_alarm_record[PonPortIdx][0] != 0)
			{
				onuLaser_alwaysOn_alarm_record[PonPortIdx][0] = 0;
				onuLaser_alwaysOn_alarm_record[PonPortIdx][1] = 0;
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] =0;
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][1] =0;
				ponPortLaserAlwaysOnClear_EventReport( GetCardIdxByPonChip( PonPortIdx), GetPonPortByPonChip(PonPortIdx), 0, 0 );
			}
		}
	}
	/*ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( );*/

	return VOS_OK;

}

int Check_Sfp_Online_6100(short int slotno, short int sfp_sel)
{
	char a = 0;
	int lRet1, lRet2;
	short int temp;
	temp = sfp_sel%SFP_OFFSET;
	
	lRet1 = i2c_read(g_PhysicalSlot[slotno], temp, 20, &a, 1 );
	lRet2 = i2c_read(g_PhysicalSlot[slotno], temp + SFP_OFFSET, 20, &a, 1 );
	return  lRet1&&lRet2 ;
}

int Check_Sfp_Online_10GE(short int slotno )
{
	char a = 0;
	int lRet1, lRet2;
	short int temp;
	
	lRet1 = i2c_read( g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, 148, &a, 1);
	lRet2 = i2c_read(g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, 150, &a, 1 );
	return  lRet1&&lRet2 ;
}


int Check_Sfp_Online_12Epon_Uplink(short int slotno, short int sfp_sel)
{
	unsigned char a = 0xFF,b,c;
	int lRet1/*, lRet2*/;
	int temp;

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			{
				ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &a);
			}
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{
				ReadCPLDReg(GFA8100_UPLINK_SFP_GPON_STATE, &a);
				a = a & 0x0f;
				b = a & 0x03;//取bit0、1
				c = a & 0x0C;//取bit2、3
			
				a = (b<<2) | (c>>2);
			}
		/*ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &a);*/
		if(sfp_sel < 2)
		{
			if ( a & (1 << (sfp_sel+2)) )
		      {
		          lRet1 = FALSE;
		      }
		      else
		      {
		          lRet1 = TRUE;
		      }
		}
		else
		{
			if ( a & (1 << (sfp_sel-2)) )
		      {
		          lRet1 = FALSE;
		      }
		      else
		      {
		          lRet1 = TRUE;
		      }
		}
	}
	else
	{
	    temp = sfp_sel%SFP_OFFSET;
	    /* B--modified by liwei056@2014-1-6 for D19412 */
#if 1
	    ReadCPLDReg((volatile UCHAR*)0x09, &a);
	    if ( a & (1 << temp) )
	    {
	        lRet1 = FALSE;
	    }
	    else
	    {
	        lRet1 = TRUE;
	    }
#else    
		lRet1 = i2c_read(g_PhysicalSlot[slotno], temp, 20, &a, 1 );
		/*lRet2 = i2c_read(g_PhysicalSlot[slotno], temp + SFP_OFFSET, 20, &a, 1 );*/
#endif
	    /* E--modified by liwei056@2014-1-6 for D19412 */
	}
	
	return  lRet1/*&&lRet2*/ ;
}
/*int Check_Sfp_Online_6900_10GE(short int slotno, short int sfp_sel)
{
	char a = 0;
	int lRet1, lRet2;
	if( sfp_sel != 4 ||  __SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA6900_GEM_10GE )
		return FALSE;
	
	lRet1 = i2c_read(g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, 20, &a, 1 );
	lRet2 = i2c_read(g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, 21, &a, 1 );
	return  lRet1&&lRet2 ;
}
*/
static LONG SetPonPortOpticalMonitorEnable(long val)
{
       int i = 0;
	if((val == V2R1_ENABLE) || (val == V2R1_DISABLE))
       {   
           if(val == V2R1_DISABLE)
           {
                     SetOnuLaserAlwaysOnEnable(val);
			/* SetOnuOpticalPowerEnable(val);*/
			
			ClearAllAlarmsForOpticalPower( );

			VOS_MemZero( Sfp_Pon_Online_Flag, sizeof(Sfp_Pon_Online_Flag) );
			VOS_MemZero( Sfp_Uplink_Online_Flag, sizeof(Sfp_Uplink_Online_Flag) );
           }
	    setOpticalPowerThreshold(field_olt_monitor_enable, val,0);
       }
       return(ROK);
}

LONG GetPonPortRecvOpticalPowerLowThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_recv_oppower_low, flag));
}
 LONG SetPonPortRecvOpticalPowerLowThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_recv_oppower_low, val,flag);
	return(ROK);
}
LONG GetPonPortRecvOpticalPowerHighThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_recv_oppower_high,flag));
}
 LONG SetPonPortRecvOpticalPowerHighThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_recv_oppower_high, val,flag);
	return(ROK);
}
LONG GetPonPortTransOpticalPowerLowThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_trans_oppower_low,flag));
}
 LONG SetPonPortTransOpticalPowerLowThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_trans_oppower_low, val,flag);
	return(ROK);
}
LONG GetPonPortTransOpticalPowerHighThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_trans_oppower_high,flag));
}
 LONG SetPonPortTransOpticalPowerHighThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_trans_oppower_high, val,flag);
	return(ROK);
}
LONG GetPonPortTemperatureLowThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_tempe_low,flag));
}
 LONG SetPonPortTemperatureLowThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_tempe_low, val,flag);
	return(ROK);
}
LONG GetPonPortTemperatureHighThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_tempe_high,flag));
}
LONG SetPonPortTemperatureHighThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_tempe_high, val,flag);
	return(ROK);
}
LONG GetPonPortWorkVoltageLowThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_vol_low,flag));
}
LONG SetPonPortWorkVoltageLowThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_vol_low,val,flag);
	return(ROK);
}
LONG GetPonPortWorkVoltageHighThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_vol_high,flag));
}
LONG SetPonPortWorkVoltageHighThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_vol_high,val,flag);
	return(ROK);
}
LONG GetPonPortBiasCurrentLowThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_cur_low,flag));
}
LONG SetPonPortBiasCurrentLowThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_cur_low,val,flag);
	return(ROK);
}
LONG GetPonPortBiasCurrentHighThrd(int flag)
{
	return(getOpticalPowerThreshold(field_olt_cur_high,flag));
}
LONG SetPonPortBiasCurrentHighThrd(long val,int flag)
{
	setOpticalPowerThreshold(field_olt_cur_high,val,flag);
	return(ROK);
}

LONG GetOnuRecvOpticalPowerLowThrd()
{
	return(getOpticalPowerThreshold(field_recv_oppower_low,0));
}
LONG SetOnuRecvOpticalPowerLowThrd(long val)
{
	setOpticalPowerThreshold(field_recv_oppower_low,val,0);
	return(ROK);
}

LONG GetOnuRecvOpticalPowerHighThrd()
{
	return(getOpticalPowerThreshold(field_recv_oppower_high,0));
}

LONG SetOnuRecvOpticalPowerHighThrd(long val)
{
	setOpticalPowerThreshold(field_recv_oppower_high,val,0);
	return(ROK);
}
LONG GetOnuTransOpticalPowerLowThrd()
{
	return(getOpticalPowerThreshold(field_trans_oppower_low,0));
}
 LONG SetOnuTransOpticalPowerLowThrd(long val)
{
	setOpticalPowerThreshold(field_trans_oppower_low,val,0);
	return(ROK);
}
LONG GetOnuTransOpticalPowerHighThrd()
{
	return(getOpticalPowerThreshold(field_trans_oppower_high,0));
}
 LONG SetOnuTransOpticalPowerHighThrd(long val)
{
	setOpticalPowerThreshold(field_trans_oppower_high,val,0);
	return(ROK);
}
LONG GetOnuTemperatureLowThrd()
{
	return(getOpticalPowerThreshold(field_pon_tempe_low,0));
}
LONG SetOnuTemperatureLowThrd(long val)
{
	setOpticalPowerThreshold(field_pon_tempe_low,val,0);
	return(ROK);
}
LONG GetOnuTemperatureHighThrd()
{
	return(getOpticalPowerThreshold(field_pon_tempe_high,0));
}
LONG SetOnuTemperatureHighThrd(long val)
{
	setOpticalPowerThreshold(field_pon_tempe_high,val,0);
	return(ROK);
}
LONG GetOnuWorkVoltageLowThrd()
{
	return(getOpticalPowerThreshold(field_pon_vol_low,0));
}
LONG SetOnuWorkVoltageLowThrd(long val)
{
	setOpticalPowerThreshold(field_pon_vol_low,val,0);
	return(ROK);
}
LONG GetOnuWorkVoltageHighThrd()
{
	return(getOpticalPowerThreshold(field_pon_vol_high,0));
}
LONG SetOnuWorkVoltageHighThrd(long val)
{
	setOpticalPowerThreshold(field_pon_vol_high,val,0);
	return(ROK);
}
LONG GetOnuBiasCurrentLowThrd()
{
	return(getOpticalPowerThreshold(field_pon_cur_low,0));
}
 LONG SetOnuBiasCurrentLowThrd(long val)
{
	setOpticalPowerThreshold(field_pon_cur_low,val,0);
	return(ROK);
}
LONG GetOnuBiasCurrentHighThrd()
{
	return(getOpticalPowerThreshold(field_pon_cur_high,0));
}
 LONG SetOnuBiasCurrentHighThrd(long val)
{
	setOpticalPowerThreshold(field_pon_cur_high,val,0);
	return(ROK);
}
LONG GetPonPortOpticalAlwaysOnEnable(void)
{
	return (getOpticalPowerThreshold(field_olt_mon_laser_thresh, 0));
}

LONG SetPonPortOpticalAlwaysOnEnable(long val)
{
	setOpticalPowerThreshold(field_olt_mon_laser_thresh,val,0);
	return(ROK);
}


/*OLT端发送光功率告警检测*/
int CheckPonPortTransOpticalPower(short int PonPortIdx, long TransOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long TransOpticalPowerLowThrd;
	long TransOpticalPowerHighThrd;
	ULONG slot, port;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}
	/*告警门限值是不是要设置为无符号的数值，而校准值是可以有正有负的*/
	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);
	TransOpticalPowerLowThrd = GetPonPortTransOpticalPowerLowThrd(flag);
	TransOpticalPowerHighThrd = GetPonPortTransOpticalPowerHighThrd(flag);

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower>(TransOpticalPowerHighThrd))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_H) &&
		(TransOpticalPower_cur < (TransOpticalPowerHighThrd - OpticalPowerDeadZone)))
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport( GW_TX_POWER_HIGH_ALARM, V2R1_DISABLE,1, slot, port, TransOpticalPowerHighThrd );	
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_TXPOW_H);

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerNormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower<(TransOpticalPowerLowThrd))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_L) &&
		(TransOpticalPower_cur > (TransOpticalPowerLowThrd + OpticalPowerDeadZone)) )
	{
		/*清除低告警*/	
		onuOpticalParaAlm_EventReport( GW_TX_POWER_LOW_ALARM, V2R1_DISABLE, 1, slot, port, TransOpticalPowerLowThrd );	
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_TXPOW_L);

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerNormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower<(TransOpticalPowerHighThrd))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_H) == 0) &&
		(TransOpticalPower_cur > (TransOpticalPowerHighThrd)) )
	{
		/*产生高告警*/	
		onuOpticalParaAlm_EventReport( GW_TX_POWER_HIGH_ALARM, V2R1_ENABLE, 1, slot, port, TransOpticalPowerHighThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_TXPOW_H;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerAbnormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower>(TransOpticalPowerLowThrd))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_L) == 0) &&
		(TransOpticalPower_cur < (TransOpticalPowerLowThrd)) )
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport( GW_TX_POWER_LOW_ALARM, V2R1_ENABLE, 1, slot, port, TransOpticalPowerLowThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_TXPOW_L;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerAbnormal(PonPortIdx);
#endif
	}
	return ROK;
}

/*OLT端接收功率告警检测*/
int CheckPonPortRecvOpticalPower(short int PonPortIdx, short int OnuIdx, /*long power,*/long RecvOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long RecvOpticalPowerLowThrd;
	long RecvOpticalPowerHighThrd;

	ULONG slot, port, onu_devidx;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_ONU_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	onu_devidx =MAKEDEVID(slot,port,(OnuIdx+1)) /*slot*10000+port*1000+OnuIdx+1*/;

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}
	
	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);
	RecvOpticalPowerLowThrd = GetPonPortRecvOpticalPowerLowThrd(flag);
	RecvOpticalPowerHighThrd = GetPonPortRecvOpticalPowerHighThrd(flag);

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;
		
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx] >(RecvOpticalPowerHighThrd ))*/
		(pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_H) &&
		(RecvOpticalPower_cur < (RecvOpticalPowerHighThrd-OpticalPowerDeadZone)) )
	{
		/*清除高告警*/
		oltOpticalRxHighClear_EventReport( slot, port, onu_devidx, RecvOpticalPowerHighThrd );
		pPonEntry->rxAlarmStatus[OnuIdx] &= (~PON_ALARM_STATE_RXPOW_H);

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerNormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx] <(RecvOpticalPowerLowThrd))*/
		(pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_L) &&
		(RecvOpticalPower_cur > (RecvOpticalPowerLowThrd+OpticalPowerDeadZone)) )
	{
		/*清除低告警*/
		oltOpticalRxLowClear_EventReport( slot, port, onu_devidx, RecvOpticalPowerLowThrd );
		pPonEntry->rxAlarmStatus[OnuIdx] &= (~PON_ALARM_STATE_RXPOW_L);

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerNormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]<(RecvOpticalPowerHighThrd))*/
		((pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_H) == 0) &&
		(RecvOpticalPower_cur > (RecvOpticalPowerHighThrd)) )
	{
		/*产生高告警*/
		oltOpticalRxHigh_EventReport( slot, port, onu_devidx, RecvOpticalPowerHighThrd );
		pPonEntry->rxAlarmStatus[OnuIdx] |= PON_ALARM_STATE_RXPOW_H;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerAbnormal(PonPortIdx);
#endif
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]>(RecvOpticalPowerLowThrd))*/
		((pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_L) == 0) &&
		(RecvOpticalPower_cur < (RecvOpticalPowerLowThrd)) )
	{
		/*产生低告警*/
		oltOpticalRxLow_EventReport( slot, port, onu_devidx, RecvOpticalPowerLowThrd );
		pPonEntry->rxAlarmStatus[OnuIdx] |= PON_ALARM_STATE_RXPOW_L;

#if ( EPON_SUBMODULE_PON_SWAP_TRIGGER_OPTICPOWER == EPON_MODULE_YES )
		PonOpticalPowerAbnormal(PonPortIdx);
#endif
	}
	return ROK;
}

/*OLT端光模块工作温度告警检测*/	
int CheckPonPortTemperature(short int PonPortIdx, /*long tem,*/long Temperature_cur)
{
	long TemperatureDeadZone;
	long TemperatureLowThrd;
	long TemperatureHighThrd;

	unsigned long slot, port;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	TemperatureDeadZone = getOpticalPowerDeadZone(field_tempe_dead_zone);
	TemperatureLowThrd = GetPonPortTemperatureLowThrd(flag);
	TemperatureHighThrd = GetPonPortTemperatureHighThrd(flag);

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;

	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature>(TemperatureHighThrd ))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_H) &&
		(Temperature_cur < (TemperatureHighThrd -TemperatureDeadZone)))
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport( GW_TEMP_HIGH_ALARM, V2R1_DISABLE, 1, slot, port,TemperatureHighThrd );
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_TEMP_H);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature<(TemperatureLowThrd ))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_L) &&
		(Temperature_cur > (TemperatureLowThrd +TemperatureDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport( GW_TEMP_LOW_ALARM, V2R1_DISABLE, 1, slot, port, TemperatureLowThrd );
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_TEMP_L);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature<(TemperatureHighThrd ))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_H) == 0) &&
		(Temperature_cur > (TemperatureHighThrd )))
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport( GW_TEMP_HIGH_ALARM, V2R1_ENABLE, 1, slot, port,TemperatureHighThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_TEMP_H;
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature>(TemperatureLowThrd ))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_L) == 0) &&
		(Temperature_cur < (TemperatureLowThrd)))
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport( GW_TEMP_LOW_ALARM, V2R1_ENABLE, 1, slot, port, TemperatureLowThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_TEMP_L;
	}
	return VOS_OK;
}

/*OLT端光模块工作电压告警检测*/
int CheckPonPortWorkVoltage(short int PonPortIdx,/*long vol,*/long workVoltage_cur)
{
	long workVoltageDeadZone;
	long workVoltageLowThrd;
	long workVoltageHighThrd;

	unsigned long slot, port;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	workVoltageDeadZone = getOpticalPowerDeadZone(field_vol_dead_zone);
	workVoltageLowThrd = GetPonPortWorkVoltageLowThrd(flag);
	workVoltageHighThrd = GetPonPortWorkVoltageHighThrd(flag);

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;

	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied>(workVoltageHighThrd))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_H) &&
		(workVoltage_cur<(workVoltageHighThrd -workVoltageDeadZone)) )
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport( GW_VCC_HIGH_ALARM, V2R1_DISABLE, 1, slot, port, workVoltageHighThrd );
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_VOLT_H);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied<(workVoltageLowThrd ))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_L) &&
		(workVoltage_cur>(workVoltageLowThrd +workVoltageDeadZone)) )
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport( GW_VCC_LOW_ALARM, V2R1_DISABLE, 1, slot, port, workVoltageLowThrd );
		PonPortTable[PonPortIdx].PonPortmeteringInfo.txAlarmStatus &= (~PON_ALARM_STATE_VOLT_L);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied<(workVoltageHighThrd ))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_H) == 0) &&
		(workVoltage_cur > (workVoltageHighThrd)) )
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport( GW_VCC_HIGH_ALARM, V2R1_ENABLE, 1, slot, port,workVoltageHighThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_VOLT_H;
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied>(workVoltageLowThrd))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_L) == 0) &&
		(workVoltage_cur < (workVoltageLowThrd)) )
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport( GW_VCC_LOW_ALARM, V2R1_ENABLE, 1, slot, port, workVoltageLowThrd );
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_VOLT_L;
	}
	return VOS_OK;
}

/*OLT端工作电流告警检测*/
int CheckPonPortBiasCurrent(short int PonPortIdx, /*long bias,*/long BiasCurrent_cur)
{
	long BiasCurrentDeadZone;
	long BiasCurrentLowThrd;
	long BiasCurrentHighThrd;

	unsigned long slot, port;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	BiasCurrentDeadZone = getOpticalPowerDeadZone(field_cur_dead_zone);
	BiasCurrentLowThrd = GetPonPortBiasCurrentLowThrd(flag);
	BiasCurrentHighThrd = GetPonPortBiasCurrentHighThrd(flag);

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;

	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent>(BiasCurrentHighThrd ))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_H) &&
		(BiasCurrent_cur < (BiasCurrentHighThrd -BiasCurrentDeadZone)))
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_DISABLE,1, slot, port,BiasCurrentHighThrd);
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_BIAS_H);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent<(BiasCurrentLowThrd))*/
		(pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_L) &&
		(BiasCurrent_cur > (BiasCurrentLowThrd +BiasCurrentDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_DISABLE,1, slot, port, BiasCurrentLowThrd);
		pPonEntry->txAlarmStatus &= (~PON_ALARM_STATE_BIAS_L);
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent<(BiasCurrentHighThrd ))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_H) == 0) &&
		(BiasCurrent_cur > (BiasCurrentHighThrd )))
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_ENABLE,1, slot, port,BiasCurrentHighThrd);
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_BIAS_H;
	}
	if( /*(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent>(BiasCurrentLowThrd +BiasCurrentDeadZone))*/
		((pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_L) == 0) &&
		(BiasCurrent_cur < (BiasCurrentLowThrd)))
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_ENABLE,1, slot, port, BiasCurrentLowThrd);
		pPonEntry->txAlarmStatus |= PON_ALARM_STATE_BIAS_L;
	}
	return VOS_OK;
}

/*ONU端功率检测接口函数*/
int GetOnuTransOpticalPower(short int PonPortIdx,short int OnuIdx)
{
	int power;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	power= (OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.transOpticalPower /*+ onu_tx_optical_power_calibration*/ );
	VOS_SemGive(PowerMeteringSemId);
	return power;
}

int GetOnuRecvOpticalPower(short int PonPortIdx, short int OnuIdx)
{
	int power;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	power= (OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.recvOpticalPower /*+ onu_rx_optical_power_calibration*/);
	VOS_SemGive(PowerMeteringSemId);	
	return power;
}

int GetOnuTemperature(short int PonPortIdx, short int OnuIdx)
{
	int tem;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	tem= OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.ponTemperature;
	VOS_SemGive(PowerMeteringSemId);
	return tem;
}

int GetOnuWorkVoltage(short int PonPortIdx, short int OnuIdx)
{
	int vol;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	vol= OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.ponVoltageApplied;
	VOS_SemGive(PowerMeteringSemId);	
	return vol;
}

int GetOnuBiasCurrent(short int PonPortIdx,short int OnuIdx)
{
	int bias;
	CHECK_ONU_RANGE
	VOS_SemTake(PowerMeteringSemId, WAIT_FOREVER );
	bias= OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.ponBiasCurrent;
	VOS_SemGive(PowerMeteringSemId);	
	return bias;
}



int CheckOnuTransOpticalPower(short int PonPortIdx, short int OnuIdx,/*long power,*/ long TransOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long TransOpticalPowerLowThrd;
	long TransOpticalPowerHighThrd;
	ULONG Onu_deviceIdx;
	int OnuEntry;
	ONUMeteringTable_S *pEntry;
	
	CHECK_ONU_RANGE

	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	if( OnuEntry > MAXONU )
		return VOS_ERROR;
	
	/*Onu_deviceIdx = (GetCardIdxByPonChip(PonPortIdx))*10000 + (GetPonPortByPonChip(PonPortIdx))*1000 +(OnuIdx+1);*/
        Onu_deviceIdx=MAKEDEVID(GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));

	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);
	TransOpticalPowerLowThrd = GetOnuTransOpticalPowerLowThrd();
	TransOpticalPowerHighThrd = GetOnuTransOpticalPowerHighThrd();
	
	pEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;
	
	if(	/*(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower>(TransOpticalPowerHighThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_H) &&
		(TransOpticalPower_cur < (TransOpticalPowerHighThrd-OpticalPowerDeadZone)))
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport( GW_TX_POWER_HIGH_ALARM, V2R1_DISABLE, Onu_deviceIdx, 1, 1, TransOpticalPowerHighThrd );
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_TXPOW_H);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower<(TransOpticalPowerLowThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_L) &&
		(TransOpticalPower_cur > (TransOpticalPowerLowThrd+OpticalPowerDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,TransOpticalPowerLowThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_TXPOW_L);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower<(TransOpticalPowerHighThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_H) == 0) &&
		( TransOpticalPower_cur > (TransOpticalPowerHighThrd)) )
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_TX_POWER_HIGH_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,TransOpticalPowerHighThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_TXPOW_H;
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower>(TransOpticalPowerLowThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_L) == 0) &&
		(TransOpticalPower_cur < (TransOpticalPowerLowThrd)) )
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_POWER_LOW_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,TransOpticalPowerLowThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_TXPOW_L;
	}
	return VOS_OK;
}

int CheckOnuRecvOpticalPower(short int PonPortIdx, short int OnuIdx, /*long power,*/long RecvOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long RecvOpticalPowerLowThrd;
	long RecvOpticalPowerHighThrd;
	int OnuEntry;
	unsigned long Onu_deviceIdx;
	ONUMeteringTable_S *pEntry;
	
	CHECK_ONU_RANGE
		
	OnuEntry= PonPortIdx*MAXONUPERPON + OnuIdx;
	if( PonPortIdx > MAXPON )
		return VOS_ERROR;

	/*Onu_deviceIdx = (GetCardIdxByPonChip(PonPortIdx))*10000 + (GetPonPortByPonChip(PonPortIdx))*1000 +(OnuIdx+1);*/
        Onu_deviceIdx=MAKEDEVID(GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
    
	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);
	RecvOpticalPowerLowThrd = GetOnuRecvOpticalPowerLowThrd();
	RecvOpticalPowerHighThrd = GetOnuRecvOpticalPowerHighThrd();

	pEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;

	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower>(RecvOpticalPowerHighThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_H) &&
		(RecvOpticalPower_cur < (RecvOpticalPowerHighThrd-OpticalPowerDeadZone)) )
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport(GW_RX_POWER_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,RecvOpticalPowerHighThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_RXPOW_H);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower<(RecvOpticalPowerLowThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_L) &&
		(RecvOpticalPower_cur > (RecvOpticalPowerLowThrd+OpticalPowerDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_RX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,RecvOpticalPowerLowThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_RXPOW_L);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower<(RecvOpticalPowerHighThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_H) == 0) &&
		(RecvOpticalPower_cur > (RecvOpticalPowerHighThrd)))
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_RX_POWER_HIGH_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,RecvOpticalPowerHighThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_RXPOW_H;
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower>(RecvOpticalPowerLowThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_L) == 0) &&
		(RecvOpticalPower_cur < (RecvOpticalPowerLowThrd)))
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_RX_POWER_LOW_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,RecvOpticalPowerLowThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_RXPOW_L;
	}
	return VOS_OK;
}

int CheckOnuTemperature(short int PonPortIdx, short int OnuIdx, /*long tem,*/long Temperature_cur)
{
	long TemperatureDeadZone;
	long TemperatureLowThrd;
	long TemperatureHighThrd;
	short int OnuEntry;
	unsigned long Onu_deviceIdx;
	ONUMeteringTable_S *pEntry;
	
	CHECK_ONU_RANGE
		
	OnuEntry= PonPortIdx*MAXONUPERPON + OnuIdx;
	if( OnuEntry > MAXONU )
		return VOS_ERROR;

	/*Onu_deviceIdx = (GetCardIdxByPonChip(PonPortIdx))*10000 + (GetPonPortByPonChip(PonPortIdx))*1000 +(OnuIdx+1);*/
        Onu_deviceIdx=MAKEDEVID(GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));

	TemperatureDeadZone = getOpticalPowerDeadZone(field_tempe_dead_zone);
	TemperatureLowThrd = GetOnuTemperatureLowThrd();
	TemperatureHighThrd = GetOnuTemperatureHighThrd();

	pEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;

	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature>(TemperatureHighThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_TEMP_H) &&
		(Temperature_cur < (TemperatureHighThrd-TemperatureDeadZone)) )
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport(GW_TEMP_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,TemperatureHighThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_TEMP_H);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature<(TemperatureLowThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_TEMP_L) &&
		(Temperature_cur > (TemperatureLowThrd+TemperatureDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_TEMP_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,TemperatureLowThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_TEMP_L);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature<(TemperatureHighThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_TEMP_H) == 0) &&
		(Temperature_cur > (TemperatureHighThrd)) )
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_TEMP_HIGH_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,TemperatureHighThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_TEMP_H;
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature>(TemperatureLowThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_TEMP_L) == 0) &&
		(Temperature_cur < (TemperatureLowThrd)))
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_TEMP_LOW_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,TemperatureLowThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_TEMP_L;
	}
	return VOS_OK;
}

int CheckOnuWorkVoltage(short int PonPortIdx, short int OnuIdx, /*long vol,*/long WorkVoltage_cur)
{
	long WorkVoltageDeadZone;
	long WorkVoltageLowThrd;
	long WorkVoltageHighThrd;
	short int OnuEntry;
	unsigned long Onu_deviceIdx;
	ONUMeteringTable_S *pEntry;
	
	CHECK_ONU_RANGE
	OnuEntry= PonPortIdx*MAXONUPERPON + OnuIdx;
	if( OnuEntry > MAXONU )
		return VOS_ERROR;

	/*Onu_deviceIdx = (GetCardIdxByPonChip(PonPortIdx))*10000 + (GetPonPortByPonChip(PonPortIdx))*1000 +(OnuIdx+1);*/
        Onu_deviceIdx=MAKEDEVID(GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));

	WorkVoltageDeadZone = getOpticalPowerDeadZone(field_vol_dead_zone);
	WorkVoltageLowThrd = GetOnuWorkVoltageLowThrd();
	WorkVoltageHighThrd = GetOnuWorkVoltageHighThrd();

	pEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;

	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied>(WorkVoltageHighThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_VOLT_H) &&
		(WorkVoltage_cur < (WorkVoltageHighThrd-WorkVoltageDeadZone)) )
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport(GW_VCC_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,WorkVoltageHighThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_VOLT_H);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied<(WorkVoltageLowThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_VOLT_L) &&
		(WorkVoltage_cur > (WorkVoltageLowThrd+WorkVoltageDeadZone)))
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_VCC_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,WorkVoltageLowThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_VOLT_L);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied<(WorkVoltageHighThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_VOLT_H) == 0) &&
		(WorkVoltage_cur > (WorkVoltageHighThrd)))
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_VCC_HIGH_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,WorkVoltageHighThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_VOLT_H;
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied>(WorkVoltageLowThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_VOLT_L) == 0) &&
		(WorkVoltage_cur < (WorkVoltageLowThrd)))
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_VCC_LOW_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,WorkVoltageLowThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_VOLT_L;
	}
	return VOS_OK;
}

int CheckOnuBiasCurrent(short int PonPortIdx, short int OnuIdx, /*long bias,*/long BiasCurrent_cur)
{
	long BiasCurrentDeadZone;
	long BiasCurrentLowThrd;
	long BiasCurrentHighThrd;
	short int OnuEntry;
	unsigned long Onu_deviceIdx;
	ONUMeteringTable_S *pEntry;
	
	CHECK_ONU_RANGE
	OnuEntry= PonPortIdx*MAXONUPERPON + OnuIdx;
	if( OnuEntry > MAXONU )
		return VOS_ERROR;

	/*Onu_deviceIdx = (GetCardIdxByPonChip(PonPortIdx))*10000 + (GetPonPortByPonChip(PonPortIdx))*1000 +(OnuIdx+1);*/
        Onu_deviceIdx=MAKEDEVID(GetCardIdxByPonChip(PonPortIdx),GetPonPortByPonChip(PonPortIdx),(OnuIdx+1));
    
	BiasCurrentDeadZone = getOpticalPowerDeadZone(field_cur_dead_zone);
	BiasCurrentLowThrd = GetOnuBiasCurrentLowThrd();
	BiasCurrentHighThrd = GetOnuBiasCurrentHighThrd();

	pEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;

	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent>(BiasCurrentHighThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_BIAS_H) &&
		(BiasCurrent_cur < (BiasCurrentHighThrd-BiasCurrentDeadZone)) )
	{
		/*清除高告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,BiasCurrentHighThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_BIAS_H);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent<(BiasCurrentLowThrd))*/
		(pEntry->AlarmStatus & PON_ALARM_STATE_BIAS_L) &&
		(BiasCurrent_cur > (BiasCurrentLowThrd+BiasCurrentDeadZone)) )
	{
		/*清除低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,BiasCurrentLowThrd);
		pEntry->AlarmStatus &= (~PON_ALARM_STATE_BIAS_L);
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent<(BiasCurrentHighThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_BIAS_H) == 0) &&
		(BiasCurrent_cur > BiasCurrentHighThrd) )
	{
		/*产生高告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,BiasCurrentHighThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_BIAS_H;
	}
	if( /*(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent>(BiasCurrentLowThrd))*/
		((pEntry->AlarmStatus & PON_ALARM_STATE_BIAS_L) == 0) &&
		(BiasCurrent_cur < BiasCurrentLowThrd) )
	{
		/*产生低告警*/
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_ENABLE,Onu_deviceIdx,1,1,BiasCurrentLowThrd);
		pEntry->AlarmStatus |= PON_ALARM_STATE_BIAS_L;
	}
	return VOS_OK;
}
int checkUplinkPortSpeed(short int slotno,short int sfp_sel)
{
	int Flag = 0;
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;

	return Flag;
}
/*上联口光功率检测告警处理*/
int CheckUplinkTransPower(short int slotno, short int sfp_sel,/*long tran,*/long TransOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long TransOpticalPowerLowThrd;
	long TransOpticalPowerHighThrd;
	int portno, Flag=0,portoffset = 0;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;
	#if 0
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;
	#endif
	Flag = checkUplinkPortSpeed(slotno, sfp_sel);

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = sfp_sel+16;
	}
	else
	{
		portoffset = sfp_sel;
	}

	
	portno = slotno*MAXUPLINKPORT + sfp_sel;
	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);
	TransOpticalPowerLowThrd = GetUplinkPortTransOpticalPowerLowthrd( Flag );
	TransOpticalPowerHighThrd = GetUplinkPortTransOpticalPowerHighthrd( Flag );
	/*UplinkPortMeteringInfo[sfp_sel].transOpticalPower=tran;*/

/*	if( (UplinkPortMeteringInfo[portno].AlarmStatus & 
		(	UPLINK_ALARM_STATE_LOS | 
			UPLINK_ALARM_STATE_BIAS_H | 
			UPLINK_ALARM_STATE_BIAS_L | 
			UPLINK_ALARM_STATE_VOLT_H | 
			UPLINK_ALARM_STATE_VOLT_L | 
			UPLINK_ALARM_STATE_TEMP_H | 
			UPLINK_ALARM_STATE_TEMP_L )) == 0 )
	{*/
		if( /*(UplinkPortMeteringInfo[sfp_sel].transOpticalPower>(TransOpticalPowerHighThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TXPOW_H) &&
			(TransOpticalPower_cur < (TransOpticalPowerHighThrd - OpticalPowerDeadZone)))
		{
			/*清除高告警*/
			UplinkSFPTransPowerHighClear_EventReport( 1, slotno, portoffset+1, TransOpticalPowerHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_TXPOW_H);   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].transOpticalPower<(TransOpticalPowerLowThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TXPOW_L) &&
			(TransOpticalPower_cur > (TransOpticalPowerLowThrd + OpticalPowerDeadZone)))
		{
			/*清除低告警*/
			UplinkSFPTransPowerLowClear_EventReport(  1, slotno,portoffset+1, TransOpticalPowerLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_TXPOW_L);   				
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].transOpticalPower<(TransOpticalPowerHighThrd))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TXPOW_H) == 0) &&
			(TransOpticalPower_cur > TransOpticalPowerHighThrd) )
		{
			/*产生高告警*/
			UplinkSFPTransPowerHigh_EventReport(  1, slotno, portoffset+1, TransOpticalPowerHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_TXPOW_H;   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].transOpticalPower>(TransOpticalPowerLowThrd))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TXPOW_L) == 0) &&
			(TransOpticalPower_cur < TransOpticalPowerLowThrd) )
		{
			/*产生低告警*/
			UplinkSFPTransPowerLow_EventReport(  1, slotno, portoffset+1, TransOpticalPowerLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_TXPOW_L;   
		}
	
	return VOS_OK;
}

int CheckUplinkRecvPower(short int slotno, short int sfp_sel,/*long recv,*/ long RecvOpticalPower_cur)
{
	long OpticalPowerDeadZone;
	long RecvOpticalPowerLowThrd;
	long RecvOpticalPowerHighThrd;
	int portno, Flag = 0,portoffset = 0;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	/*added by yanjy*/
	/*有的光模块在没有UP的时候接受光功率为0，且此时会告警。避免这种情况*/
	if(0 == RecvOpticalPower_cur)
		return VOS_OK;
	

	#if 0
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;
	#endif
	Flag = checkUplinkPortSpeed(slotno, sfp_sel);

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = sfp_sel+16;
	}
	else
	{
		portoffset = sfp_sel;
	}
	
	portno = slotno*MAXUPLINKPORT +sfp_sel;
	OpticalPowerDeadZone = getOpticalPowerDeadZone(field_power_dead_zone);

	RecvOpticalPowerLowThrd = GetUplinkPortRecvOpticalPowerLowthrd( Flag );

	RecvOpticalPowerHighThrd = GetUplinkPortRecvOpticalPowerHighthrd( Flag );
	/*UplinkPortMeteringInfo[sfp_sel].recvOpticalPower=recv;*/

	/*if( (UplinkPortMeteringInfo[portno].AlarmStatus &
		(	UPLINK_ALARM_STATE_LOS | 
			UPLINK_ALARM_STATE_BIAS_H | 
			UPLINK_ALARM_STATE_BIAS_L | 
			UPLINK_ALARM_STATE_VOLT_H | 
			UPLINK_ALARM_STATE_VOLT_L | 
			UPLINK_ALARM_STATE_TEMP_H |
			UPLINK_ALARM_STATE_TEMP_L )) == 0 )
	{*/
		if( /*(UplinkPortMeteringInfo[sfp_sel].recvOpticalPower>(RecvOpticalPowerHighThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_RXPOW_H) &&
			(RecvOpticalPower_cur < (RecvOpticalPowerHighThrd - OpticalPowerDeadZone)) )
		{
			/*清除高告警*/
			UplinkSFPRecvPowerHighClear_EventReport( 1, slotno, portoffset+1, RecvOpticalPowerHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_RXPOW_H);   
		}

		if( /*(UplinkPortMeteringInfo[sfp_sel].recvOpticalPower<(RecvOpticalPowerLowThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_RXPOW_L) &&
			(RecvOpticalPower_cur > (RecvOpticalPowerLowThrd+OpticalPowerDeadZone)))
		{
			/*清除低告警*/
			UplinkSFPRecvPowerLowClear_EventReport( 1, slotno, portoffset+1, RecvOpticalPowerLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_RXPOW_L);
		}

		if( /*(UplinkPortMeteringInfo[sfp_sel].recvOpticalPower<(RecvOpticalPowerHighThrd))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_RXPOW_H) == 0) &&
			(RecvOpticalPower_cur > RecvOpticalPowerHighThrd) )
		{
			/*产生高告警*/
			UplinkSFPRecvPowerHigh_EventReport( 1, slotno, portoffset+1, RecvOpticalPowerHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_RXPOW_H;
			SFP_RSSI_DEBUG(0x100, (LOG_OPTICAL_POWER,LOG_INFO,"sfp=%d, %d\r\n", sfp_sel, UplinkPortMeteringInfo[sfp_sel].AlarmStatus) );
		}

		if( /*(UplinkPortMeteringInfo[sfp_sel].recvOpticalPower>(RecvOpticalPowerLowThrd))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_RXPOW_L) == 0) &&
			(RecvOpticalPower_cur < RecvOpticalPowerLowThrd) )
		{
			/*产生低告警*/
			UplinkSFPRecvPowerLow_EventReport( 1, slotno, portoffset+1, RecvOpticalPowerLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_RXPOW_L;
		}

	
	return VOS_OK;
}

int CheckUplinkTemperature(short int slotno,short int sfp_sel,/*long tem,*/long Temperature_cur)
{
	long TemperatureDeadZone;
	long TemperatureLowThrd;
	long TemperatureHighThrd;
	int portno, Flag = 0,portoffset = 0;

	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	#if 0
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;
	#endif
	Flag = checkUplinkPortSpeed(slotno, sfp_sel);

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = sfp_sel+16;
	}
	else
	{
		portoffset = sfp_sel;
	}
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	TemperatureDeadZone = getOpticalPowerDeadZone(field_tempe_dead_zone);
	TemperatureLowThrd = GetUplinkPortTemperatureLowthrd(Flag);
	TemperatureHighThrd = GetUplinkPortTemperatureHighthrd(Flag);
	/*UplinkPortMeteringInfo[sfp_sel].Temperature=tem;*/

	/*if( (UplinkPortMeteringInfo[portno].AlarmStatus &
		(	UPLINK_ALARM_STATE_LOS |
			UPLINK_ALARM_STATE_BIAS_H |
			UPLINK_ALARM_STATE_BIAS_L | 
			UPLINK_ALARM_STATE_VOLT_H |
			UPLINK_ALARM_STATE_VOLT_L )) == 0 )
	{*/
		if( /*(UplinkPortMeteringInfo[sfp_sel].Temperature>(TemperatureHighThrd ))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TEMP_H) &&
			(Temperature_cur < (TemperatureHighThrd -TemperatureDeadZone)) )
		{
			/*清除高告警*/
			UplinkSFPTemperatureHighClear_EventReport( 1, slotno,portoffset+1, TemperatureHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_TEMP_H);
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Temperature<(TemperatureLowThrd ))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TEMP_L) &&
			(Temperature_cur>(TemperatureLowThrd +TemperatureDeadZone)) )
		{
			/*清除低告警*/
			UplinkSFPTemperatureLowClear_EventReport(  1, slotno,portoffset+1, TemperatureLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_TEMP_L);
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Temperature<(TemperatureHighThrd ))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TEMP_H) == 0) &&
			(Temperature_cur > TemperatureHighThrd) )
		{
			/*产生高告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPTemperatureHigh_EventReport( 1, slotno,portoffset+1, TemperatureHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_TEMP_H;
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Temperature>(TemperatureLowThrd ))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_TEMP_L) == 0) &&
			(Temperature_cur < TemperatureLowThrd) )
		{
			/*产生低告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPTemperatureLow_EventReport( 1, slotno, portoffset+1, TemperatureLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_TEMP_L;
		}
	
	return VOS_OK;
}
int CheckUplinkBiasCurrent(short int slotno, short int sfp_sel,/*long bias,*/long BiasCurrent_cur )
{
	long BiasCurrentDeadZone;
	long BiasCurrentLowThrd;
	long BiasCurrentHighThrd;
	int portno, Flag = 0,portoffset = 0;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	#if 0
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;
	#endif
	Flag = checkUplinkPortSpeed(slotno, sfp_sel);

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = sfp_sel+16;
	}
	else
	{
		portoffset = sfp_sel;
	}

	
	portno = slotno*MAXUPLINKPORT + sfp_sel;
	BiasCurrentDeadZone = getOpticalPowerDeadZone(field_cur_dead_zone);
	BiasCurrentLowThrd = GetUplinkPortBiasCurrentLowthrd( Flag );
	BiasCurrentHighThrd = GetUplinkPortBiasCurrentHighthrd( Flag );
	/*UplinkPortMeteringInfo[sfp_sel].BiasCurrent=bias;*/

	/*if( (UplinkPortMeteringInfo[portno].AlarmStatus &
		(	UPLINK_ALARM_STATE_LOS | 
			UPLINK_ALARM_STATE_VOLT_H | 
			UPLINK_ALARM_STATE_VOLT_L | 
			UPLINK_ALARM_STATE_TEMP_H |
			UPLINK_ALARM_STATE_TEMP_L )) == 0 )
	{*/
		if( /*(UplinkPortMeteringInfo[sfp_sel].BiasCurrent>(BiasCurrentHighThrd ))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_BIAS_H) &&
			(BiasCurrent_cur < (BiasCurrentHighThrd -BiasCurrentDeadZone)) )
		{
			/*清除高告警*/
			UplinkSFPBiasCurrentHighClear_EventReport(  1, slotno,portoffset+1, BiasCurrentHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_BIAS_H);   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].BiasCurrent<(BiasCurrentLowThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_BIAS_L) &&
			(BiasCurrent_cur > (BiasCurrentLowThrd +BiasCurrentDeadZone)) )
		{
			/*清除低告警*/
			UplinkSFPBiasCurrentLowClear_EventReport(  1, slotno, portoffset+1, BiasCurrentLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_BIAS_L);   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].BiasCurrent<(BiasCurrentHighThrd ))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_BIAS_H) == 0) &&
			(BiasCurrent_cur > BiasCurrentHighThrd) )
		{
			/*产生高告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPBiasCurrentHigh_EventReport( 1, slotno, portoffset+1, BiasCurrentHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_BIAS_H;   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].BiasCurrent>(BiasCurrentLowThrd +BiasCurrentDeadZone))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_BIAS_L) == 0) &&
			(BiasCurrent_cur < BiasCurrentLowThrd) )
		{
			/*产生低告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPBiasCurrentLow_EventReport( 1, slotno,portoffset+1, BiasCurrentLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_BIAS_L;   
		}
	
	return VOS_OK;
}
int CheckUplinkVoltage(short int slotno, short int sfp_sel,/*long vol,*/long workVoltage_cur)
{
	long workVoltageDeadZone;
	long workVoltageLowThrd;
	long workVoltageHighThrd;
	int portno, Flag=0,portoffset = 0;
	if( (sfp_sel < 0) || (sfp_sel>=MAXUPLINKPORT) )
		return VOS_ERROR;

	#if 0
	if(( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
		 ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		 || (SYS_LOCAL_MODULE_TYPE_IS_8100_PON && ((sfp_sel == 0)  || (sfp_sel == 1))))
		Flag = 1;
	else
		Flag = 0;
	#endif
	Flag = checkUplinkPortSpeed(slotno, sfp_sel);

	
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		portoffset = sfp_sel+16;
	}
	else
	{
		portoffset = sfp_sel;
	}
	
	portno = slotno*MAXUPLINKPORT + sfp_sel;
	workVoltageDeadZone = getOpticalPowerDeadZone(field_vol_dead_zone);
	workVoltageLowThrd = GetUplinkPortVoltageLowthrd(Flag);
	workVoltageHighThrd = GetUplinkPortVoltageHighthrd(Flag);
	/*UplinkPortMeteringInfo[sfp_sel].Voltage=vol;*/

	/*if( (UplinkPortMeteringInfo[portno].AlarmStatus &
		(	UPLINK_ALARM_STATE_LOS | 
			UPLINK_ALARM_STATE_BIAS_H | 
			UPLINK_ALARM_STATE_BIAS_L | 
			UPLINK_ALARM_STATE_TEMP_H |
			UPLINK_ALARM_STATE_TEMP_L )) == 0 )
	{*/
		if( /*(UplinkPortMeteringInfo[sfp_sel].Voltage>(workVoltageHighThrd))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_VOLT_H) &&
			(workVoltage_cur < (workVoltageHighThrd - workVoltageDeadZone)) )
		{
			/*清除高告警*/
			UplinkSFPVoltageHighClear_EventReport( 1, slotno,portoffset+1, workVoltageHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_VOLT_H);   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Voltage<(workVoltageLowThrd ))*/
			(UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_VOLT_L) &&
			(workVoltage_cur > (workVoltageLowThrd + workVoltageDeadZone)) )
		{
			/*清除低告警*/
			UplinkSFPVoltageLowClear_EventReport( 1, slotno,portoffset+1, workVoltageLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus &= (~UPLINK_ALARM_STATE_VOLT_L);   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Voltage<(workVoltageHighThrd ))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_VOLT_H) == 0) &&
			(workVoltage_cur > workVoltageHighThrd) )
		{
			/*产生高告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPVoltageHigh_EventReport(  1, slotno, portoffset+1,workVoltageHighThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_VOLT_H;   
		}
		if( /*(UplinkPortMeteringInfo[sfp_sel].Voltage>(workVoltageLowThrd))*/
			((UplinkPortMeteringInfo[portno].AlarmStatus & UPLINK_ALARM_STATE_VOLT_L) == 0) &&
			(workVoltage_cur < workVoltageLowThrd) )
		{
			/*产生低告警*/
			ClearUplinkThirdClassAlarm(slotno, sfp_sel);
			UplinkSFPVoltageLow_EventReport( 1, slotno, portoffset+1, workVoltageLowThrd );
			UplinkPortMeteringInfo[portno].AlarmStatus |= UPLINK_ALARM_STATE_VOLT_L;   
		}
	
	return VOS_OK;
}
/*ONU离线时的处理*/
int ClearOpticalPowerAlamWhenOnuOffline(short int PonPortIdx,short int OnuIdx)
{
	unsigned long slot, port, Onu_deviceIdx;
	int OnuEntry;
	PonPortmeteringInfo_S *pPonEntry;
	ONUMeteringTable_S *pOnuEntry;
	int flag = 0;

	CHECK_ONU_RANGE
		
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	Onu_deviceIdx =MAKEDEVID(slot,port,(OnuIdx+1))/* slot*10000+port*1000+OnuIdx+1*/;
	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	if( OnuEntry > MAXONU )
		return VOS_ERROR;
	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;
	pOnuEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;

	/* 清除OLT 上从ONU 接收光告警*/
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]<GetPonPortRecvOpticalPowerLowThrd())*/
	if( pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_L )
	{
		oltOpticalRxLowClear_EventReport(slot, port, Onu_deviceIdx, GetPonPortRecvOpticalPowerLowThrd(flag));
	}
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]>GetPonPortRecvOpticalPowerHighThrd())*/
	if( pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_H )
	{
		oltOpticalRxHighClear_EventReport(slot, port, Onu_deviceIdx,GetPonPortRecvOpticalPowerHighThrd(flag));
	}
	pPonEntry->rxAlarmStatus[OnuIdx] = 0;
	
	/* 清除ONU 上报的告警*/
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower<GetOnuTransOpticalPowerLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_L )
	{
		onuOpticalParaAlm_EventReport(GW_TX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTransOpticalPowerLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower>GetOnuTransOpticalPowerHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_H )
	{
		onuOpticalParaAlm_EventReport(GW_TX_POWER_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTransOpticalPowerHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower<GetOnuRecvOpticalPowerLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_L )
	{
		onuOpticalParaAlm_EventReport(GW_RX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuRecvOpticalPowerLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower>GetOnuRecvOpticalPowerHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_H )
	{
		onuOpticalParaAlm_EventReport(GW_RX_POWER_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuRecvOpticalPowerHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature<GetOnuTemperatureLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TEMP_L )
	{
		onuOpticalParaAlm_EventReport(GW_TEMP_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTemperatureLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature>GetOnuTemperatureHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TEMP_H )
	{
		onuOpticalParaAlm_EventReport(GW_TEMP_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTemperatureHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied<GetOnuWorkVoltageLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_VOLT_L )
	{
		onuOpticalParaAlm_EventReport(GW_VCC_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuWorkVoltageLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied>GetOnuWorkVoltageHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_VOLT_H )
	{
		onuOpticalParaAlm_EventReport(GW_VCC_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuWorkVoltageHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent<GetOnuBiasCurrentLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_BIAS_L )
	{
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuBiasCurrentLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent>GetOnuBiasCurrentHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_BIAS_H )
	{
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuBiasCurrentHighThrd());
	}
	pOnuEntry->AlarmStatus = 0;

	return(ROK);

}

int ClearOpticalPowerAlamWhenOnuDetectionDisable(short int PonPortIdx,short int OnuIdx)
{
	unsigned long slot, port, Onu_deviceIdx;
	int OnuEntry;
	ONUMeteringTable_S *pOnuEntry;

	CHECK_ONU_RANGE
		
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	/*Onu_deviceIdx = slot*10000+port*1000+OnuIdx+1;*/
	Onu_deviceIdx = MAKEDEVID(slot, port, OnuIdx+1);
	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

	if( OnuEntry > MAXONU )
		return VOS_ERROR;
	pOnuEntry = &OnuMgmtTable[OnuEntry].ONUMeteringTable;
	
	/* 清除ONU 上报的告警*/
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower<GetOnuTransOpticalPowerLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_L )
	{
		onuOpticalParaAlm_EventReport(GW_TX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTransOpticalPowerLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower>GetOnuTransOpticalPowerHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TXPOW_H )
	{
		onuOpticalParaAlm_EventReport(GW_TX_POWER_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTransOpticalPowerHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower<GetOnuRecvOpticalPowerLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_L )
	{
		onuOpticalParaAlm_EventReport(GW_RX_POWER_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuRecvOpticalPowerLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower>GetOnuRecvOpticalPowerHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_RXPOW_H )
	{
		onuOpticalParaAlm_EventReport(GW_RX_POWER_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuRecvOpticalPowerHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature<GetOnuTemperatureLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TEMP_L )
	{
		onuOpticalParaAlm_EventReport(GW_TEMP_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTemperatureLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature>GetOnuTemperatureHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_TEMP_H )
	{
		onuOpticalParaAlm_EventReport(GW_TEMP_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuTemperatureHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied<GetOnuWorkVoltageLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_VOLT_L )
	{
		onuOpticalParaAlm_EventReport(GW_VCC_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuWorkVoltageLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied>GetOnuWorkVoltageHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_VOLT_H )
	{
		onuOpticalParaAlm_EventReport(GW_VCC_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuWorkVoltageHighThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent<GetOnuBiasCurrentLowThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_BIAS_L )
	{
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuBiasCurrentLowThrd());
	}
	/*if(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent>GetOnuBiasCurrentHighThrd())*/
	if( pOnuEntry->AlarmStatus & PON_ALARM_STATE_BIAS_H )
	{
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_DISABLE,Onu_deviceIdx,1,1,GetOnuBiasCurrentHighThrd());
	}
	pOnuEntry->AlarmStatus = 0;
	
	return(ROK);

}

int ClearOpticalPowerAlamWhenOltDetectionDisable(short int PonPortIdx,short int OnuIdx)
{
	unsigned long slot, port, Onu_deviceIdx;
	int OnuEntry;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;

	CHECK_ONU_RANGE
		
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	/* Onu_deviceIdx = slot*10000+port*1000+OnuIdx+1; */
	Onu_deviceIdx = MAKEDEVID(slot, port, OnuIdx+1);
	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	if( OnuEntry > MAXONU )
		return VOS_ERROR;
	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;
	
	/* 清除OLT 上从ONU 接收光告警*/
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]<GetPonPortRecvOpticalPowerLowThrd())*/
	if( pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_L )
	{
		oltOpticalRxLowClear_EventReport(slot, port, Onu_deviceIdx, GetPonPortRecvOpticalPowerLowThrd(flag));
	}
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]>GetPonPortRecvOpticalPowerHighThrd())*/
	if( pPonEntry->rxAlarmStatus[OnuIdx] & PON_ALARM_STATE_RXPOW_H )
	{
		oltOpticalRxHighClear_EventReport(slot, port, Onu_deviceIdx,GetPonPortRecvOpticalPowerHighThrd(flag));
	}
	pPonEntry->rxAlarmStatus[OnuIdx] = 0;
	
	return(ROK);

}

/*新加入ONU或ONU重新注册时的处理*/
int ReInitWhenNewOnuAddedOrOldOnuReregister(short int PonPortIdx,short int OnuIdx)
{
	unsigned long slot, port;
	short int OnuEntry;

	CHECK_ONU_RANGE
		
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);
	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

	PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx]=-1000;
	OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = -100;
	OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = 30;
	OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature = 50;
	OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied = 30;
	OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent = 20;
	return(ROK);
}


int ClearOpticalPowerAlarmWhenPonPortDown(short int PonPortIdx)
{
	unsigned long slot, port;
	PonPortmeteringInfo_S *pPonEntry;
	int flag = 0;
	
	CHECK_PON_RANGE
		
	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	VOS_ASSERT( MAXPON > PonPortIdx );

	if(__SYS_MODULE_TYPE__(slot) == MODULE_E_GFA8000_10G_8EPON)
	{
		flag = 1;
	}
	else
	{
		flag = 0;
	}

	pPonEntry = &PonPortTable[PonPortIdx].PonPortmeteringInfo;

	/*清除OLT上报的告警*/	
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower<GetPonPortTransOpticalPowerLowThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_L )
		onuOpticalParaAlm_EventReport(GW_TX_POWER_LOW_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortTransOpticalPowerLowThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower>GetPonPortTransOpticalPowerHighThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_TXPOW_H )
		onuOpticalParaAlm_EventReport(GW_TX_POWER_HIGH_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortTransOpticalPowerHighThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature< GetPonPortTemperatureLowThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_L )
		onuOpticalParaAlm_EventReport(GW_TEMP_LOW_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortTemperatureLowThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature>GetPonPortTemperatureHighThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_TEMP_H )
		onuOpticalParaAlm_EventReport(GW_TEMP_HIGH_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortTemperatureHighThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent <GetPonPortBiasCurrentLowThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_L )
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_LOW_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortBiasCurrentLowThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent >GetPonPortBiasCurrentHighThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_BIAS_H )
		onuOpticalParaAlm_EventReport(GW_TX_BIAS_HIGH_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortBiasCurrentHighThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied<GetPonPortWorkVoltageLowThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_L )
		onuOpticalParaAlm_EventReport(GW_VCC_LOW_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortWorkVoltageLowThrd(flag));
	/*if(PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied>GetPonPortWorkVoltageHighThrd())*/
	if( pPonEntry->txAlarmStatus & PON_ALARM_STATE_VOLT_H )
		onuOpticalParaAlm_EventReport(GW_VCC_HIGH_ALARM,V2R1_DISABLE,1, slot, port, GetPonPortWorkVoltageHighThrd(flag));
	pPonEntry->txAlarmStatus = 0;
	
	return(ROK);
}

LONG ExcludeMaxMin_Then_GetAverage(long *point , USHORT number)
{
	int counter, MaxIndex = 0 , MinIndex = 0 , Max, Min, summation = 0;

	if(number < 3)
		return VOS_ERROR ;
	
	Max = Min = point[0];
	
	for(counter = 0; counter < number ; counter++)
	{
		if(point[counter] > Max)
		{
			Max = point[counter];
			MaxIndex = counter ;
		}
		else if(point[counter] < Min)
		{
			Min = point[counter];
			MinIndex = counter ;
		}
		summation += point[counter];
	}

	return (summation - (point[MaxIndex] + point[MinIndex]))/(number - 2) ;
}


int ReadPonPortTransPower( short int PonPortIdx, long *val )
{
	long transpower=30,ret = -1;
	if( (ret = ReadsfpTxPower(PonPortIdx, &transpower)) == VOS_OK )
	{
		PonTransOpticalPower_Buffer[PonPortIdx][PonTransOpticalPower_Buffer_Index[PonPortIdx]]=transpower;
		PonTransOpticalPower_Buffer_Index[PonPortIdx]++;	
		if( PonTransOpticalPower_Buffer_Index[PonPortIdx]>=READ_I2C_COUNT )
			{
				PonTransOpticalPower_Buffer_Index[PonPortIdx]=0;
				PonTransOpticalPower_Buffer_Flag[PonPortIdx]=1;
			}
		if( PonTransOpticalPower_Buffer_Flag[PonPortIdx]==1)
		{
			/*transpower=transOpticalPower_buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			transpower = ExcludeMaxMin_Then_GetAverage(&PonTransOpticalPower_Buffer[PonPortIdx][0],READ_I2C_COUNT);
		}
	}
	*val = transpower;	
	return ret;
}

int ReadPonPortRecevPower( short int PonPortIdx ,short int OnuIdx , short int Flag, long *val)
{
	long recpower=-1000, ret = -1, temp = 0 ;
	
	if((ret = ReadPowerMeteringOnu(PonPortIdx, OnuIdx,&recpower))== VOS_OK)
	{
		if(Flag == 1)/*表示需要返回平均值，否则返回即时读取的数值*/
		{
			PonRecvOpticalPower_Buffer[PonPortIdx][OnuIdx][PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx]]=recpower;

			if( recpower != temp  )  /*modified by duzhk 2011-12-09 */
				PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx]++;

			if( PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx]>=READ_I2C_COUNT)
			{
				PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx]=0;
				PonRecvOpticalPower_Buffer_Flag[PonPortIdx][OnuIdx]=1;/*是否可以取平均的标志*/
			}
			if( PonRecvOpticalPower_Buffer_Flag[PonPortIdx][OnuIdx]==1)
			{
				recpower= ExcludeMaxMin_Then_GetAverage(&PonRecvOpticalPower_Buffer[PonPortIdx][OnuIdx][0],READ_I2C_COUNT);
				/*	recvOpticalPower_buffer[PonPortIdx][OnuIdx][READ_I2C_COUNT/2] */
			}
		}
	}	
	*val = recpower;
	return ret;
}
int ReadPonPortTemperature( short int PonPortIdx, long *val)
{
	long temperature=50, ret = -1;
	if((ret = ReadsfpTemperature(PonPortIdx,&temperature))==VOS_OK)
	{
		PonTemperature_Buffer[PonPortIdx][PonTemperature_Buffer_Index[PonPortIdx]]=temperature;
		PonTemperature_Buffer_Index[PonPortIdx]++;
		if( PonTemperature_Buffer_Index[PonPortIdx]>=READ_I2C_COUNT)
		{
			PonTemperature_Buffer_Index[PonPortIdx]=0;
			PonTemperature_Buffer_Flag[PonPortIdx]=1;
		}
		if( PonTemperature_Buffer_Flag[PonPortIdx]==1)
		{
			/*temperature=ponTemperature_buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			temperature = ExcludeMaxMin_Then_GetAverage(&PonTemperature_Buffer[PonPortIdx][0],READ_I2C_COUNT);
		}
	}
	*val = temperature;
	return ret;
}
int ReadPonPortVoltage( short int  PonPortIdx, long *val)
{
	long voltage=30, ret = -1;
	if((ret = ReadsfpVoltage(PonPortIdx, &voltage))==VOS_OK)
	{
		PonVoltageApplied_Buffer[PonPortIdx][PonVoltageApplied_Buffer_Index[PonPortIdx]]=voltage;
		PonVoltageApplied_Buffer_Index[PonPortIdx]++;
		if( PonVoltageApplied_Buffer_Index[PonPortIdx]>=READ_I2C_COUNT)
		{
			PonVoltageApplied_Buffer_Index[PonPortIdx]=0;
			PonVoltageApplied_Buffer_Flag[PonPortIdx]=1;
		}
		if( PonVoltageApplied_Buffer_Flag[PonPortIdx]==1)
		{
			/*voltage=ponVoltageApplied_buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			voltage = ExcludeMaxMin_Then_GetAverage(&PonVoltageApplied_Buffer[PonPortIdx][0],READ_I2C_COUNT);
		}
	}
	*val = voltage;
	return ret;
}
int ReadPonPortBias( short int PonPortIdx, long *val )
{
	long bias=20, ret =-1;
	if((ret = ReadsfpBias(PonPortIdx, &bias))==VOS_OK)
	{
		PonBiasCurrent_Buffer[PonPortIdx][PonBiasCurrent_Buffer_Index[PonPortIdx]]=bias;
		PonBiasCurrent_Buffer_Index[PonPortIdx]++;
		if( PonBiasCurrent_Buffer_Index[PonPortIdx]>=READ_I2C_COUNT)
		{
			PonBiasCurrent_Buffer_Index[PonPortIdx]=0;
			PonBiasCurrent_Buffer_Flag[PonPortIdx]=1;
		}
		if( PonBiasCurrent_Buffer_Flag[PonPortIdx]==1)
		{
			/*bias=PonBiasCurrent_Buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			bias = ExcludeMaxMin_Then_GetAverage(&PonBiasCurrent_Buffer[PonPortIdx][0],READ_I2C_COUNT);
		}		
	}
	*val = bias;
	return ret;
}
int GetMAXUplinkPort(int slotno)
{
	int max_link_port;

    if ( SYS_MODULE_IS_UPLINK(slotno) )
    {
    	if ( SYS_MODULE_IS_UPLINK_10GE(slotno) )
    	{
    		max_link_port = 8;
    		if ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 )
    		{
    			if ((5==slotno) || (6==slotno) || (9==slotno) || (10==slotno)) 
    				max_link_port = 5;
    			else
    				max_link_port = 4;
    		}
		else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000 )
		{
			max_link_port = 5;
		}
    	else if( PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE) )
    		max_link_port = 1;
		if( (MODULE_E_GFA8000_8XET == __SYS_MODULE_TYPE__( slotno ))  || (MODULE_E_GFA8000_8XETA1== __SYS_MODULE_TYPE__( slotno ))  )
			max_link_port = 8;
			if( MODULE_E_GFA8000_4XET == __SYS_MODULE_TYPE__( slotno ) )
				max_link_port = 4;
    	}
    	else 
    		max_link_port = 4;
    }
#if defined(_EPON_10G_PMC_SUPPORT_)            
	else if(SYS_MODULE_IS_6900_10G_EPON(slotno))
		max_link_port = 5;
#endif
	else if( SYS_MODULE_IS_UPLINK_PON(slotno) )
		max_link_port = 4;
	else 
		max_link_port = 0;
	
	return max_link_port;      /*问题单13370*/
	
	/*if( SlotCardIsUplinkBoard(slotno) == ROK )
		return ETH_slot_logical_portnum( slotno );*/		/* modified by xieshl 20110712, 问题单13257 */
	/*return 0;*/
}

int GetMAXUplinkPort_BeCardOutEd(int slotno)
{
	int max_link_port;

    if ( SYS_MODULE_IS_UPLINK(slotno) )
    {
    	if ( SYS_MODULE_IS_UPLINK_10GE(slotno) )
    	{
    		max_link_port = 8;
    		if ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 )
    		{
    			if ((5==slotno) || (6==slotno) || (9==slotno) || (10==slotno)) 
    				max_link_port = 5;
    			else
    				max_link_port = 4;
    		}
		else if( SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000 )
		{
			max_link_port = 5;
		}
		else if( PRODUCT_IS_HM_Series(SYS_PRODUCT_TYPE) )
    			max_link_port = 1;
		if(( MODULE_E_GFA8000_8XET == __SYS_MODULE_TYPE__( slotno ) )|| (MODULE_E_GFA8000_8XETA1== __SYS_MODULE_TYPE__( slotno )) )
			max_link_port = 8;
			if( MODULE_E_GFA8000_4XET == __SYS_MODULE_TYPE__( slotno ) )
				max_link_port = 4;
    	}
    	else 
    		max_link_port = 4;
    }
#if defined(_EPON_10G_PMC_SUPPORT_)            
	else if(SYS_MODULE_IS_6900_10G_EPON(slotno))
		max_link_port = 5;
#endif
	else if( SYS_MODULE_IS_UPLINK_PON(slotno) )
		max_link_port = 4;
	else 
		max_link_port = 0;
	
	return max_link_port;      /*问题单13370*/
	
	/*if( SlotCardIsUplinkBoard(slotno) == ROK )
		return ETH_slot_logical_portnum( slotno );*/		/* modified by xieshl 20110712, 问题单13257 */
	/*return 0;*/
}

/*extern LONG getnext_uplink_sys_optical( int cur_port,int *next_port,unsigned int *n_slot );
int ReadUplinkPortSFPParametre()
{
	LONG val;
	int cur_port = 0 , cur_slot = 0, *next_port , *next_slot ;

	while(VOS_OK == getnext_uplink_sys_optical(cur_port,  next_port,next_slot))
	{
		cur_port = *next_port ;
		cur_slot = *next_slot ;
		if( CheckUplinkSupportPowerMetering(next_port) == VOS_ERROR )
			continue;
		if( !checkUplinkPortSFPUpdated( cur_port) )
			continue;
		
		val=ReadUplinkPortSFPTemperature(cur_port);
		CheckUplinkTemperature( cur_port,val );
		UplinkPortMeteringInfo[cur_port].Temperature=val;

		val=ReadUplinkPortSFPVoltage(cur_port);
		CheckUplinkVoltage( cur_port,val );
		UplinkPortMeteringInfo[cur_port].Voltage=val;

		val=ReadUplinkPortSFPBiasCurrent(cur_port);
		CheckUplinkBiasCurrent( cur_port,val );
		UplinkPortMeteringInfo[cur_port].BiasCurrent=val;
			
		val=ReadUplinkPortSFPTransPower(cur_port);
		CheckUplinkTransPower(cur_port,val);
		UplinkPortMeteringInfo[cur_port].transOpticalPower=val;
				
		val=ReadUplinkPortSFPRecvPower(cur_port);
		CheckUplinkRecvPower(cur_port,val );
		UplinkPortMeteringInfo[cur_port].recvOpticalPower=val;			

		setUplinkPortSFPUpdated(cur_port);

	}
	return VOS_OK;
}
*/
extern int ReadUplinkPortSFPTransPower_10GE(short int slotno, int sfp_sel, long *val);

int ReadUplinkPortTransPower( short int slotno, int sfp_sel, long *val)
{
	long transpower=30,ret = -1;
	int portno;
	int (*Func)(short int slotno,int sfp_sel, long *val) ;
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(SYS_LOCAL_MODULE_TYPE_IS_8100_PON  && (sfp_sel < 2)))
		Func = ReadUplinkPortSFPTransPower_10GE ;
	else
		Func = ReadUplinkPortSFPTransPower;
	
	if( (ret = (*Func)(slotno, sfp_sel, &transpower)) == VOS_OK )
	{
		UplinkTransOpticalPower_Buffer[portno][UplinkTransOpticalPower_Buffer_Index[portno]]=transpower;
		UplinkTransOpticalPower_Buffer_Index[portno]++;	
		if( UplinkTransOpticalPower_Buffer_Index[portno]>=READ_I2C_COUNT )
			{
				UplinkTransOpticalPower_Buffer_Index[portno]=0;
				UplinkTransOpticalPower_Buffer_Flag[portno]=1;
			}
		if( UplinkTransOpticalPower_Buffer_Flag[portno]==1)
		{
			/*transpower=transOpticalPower_buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			transpower = ExcludeMaxMin_Then_GetAverage(&UplinkTransOpticalPower_Buffer[portno][0],READ_I2C_COUNT);
		}
	}
	*val = transpower;	
	return ret;
}

extern int ReadUplinkPortSFP_RxPower_10GE(short int slotno, int sfp_sel, long *val);

int ReadUplinkPortRecevPower( short int slotno, int sfp_sel, long *val)
{
	long recpower=-1000, ret = -1 ;
	int portno;
	int (*Func)(short int slotno,int sfp_sel, long *val);
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(SYS_LOCAL_MODULE_TYPE_IS_8100_PON  && (sfp_sel < 2)))
		Func = ReadUplinkPortSFP_RxPower_10GE ;
	else
		Func = ReadUplinkPortSFPRecvPower;
	
	if((ret = (*Func)(slotno, sfp_sel, &recpower))== VOS_OK)
	{
			UplinkRecvOpticalPower_Buffer[portno][UplinkRecvOpticalPower_Buffer_Index[portno]]=recpower;
			UplinkRecvOpticalPower_Buffer_Index[portno]++;
			if( UplinkRecvOpticalPower_Buffer_Index[portno]>=READ_I2C_COUNT)
			{
				UplinkRecvOpticalPower_Buffer_Index[portno]=0;
				UplinkRecvOpticalPower_Buffer_Flag[portno]=1;
			}
			if( UplinkRecvOpticalPower_Buffer_Flag[portno]==1)
			{
				recpower= ExcludeMaxMin_Then_GetAverage(&UplinkRecvOpticalPower_Buffer[portno][0],READ_I2C_COUNT);
				/*	recvOpticalPower_buffer[PonPortIdx][OnuIdx][READ_I2C_COUNT/2] */
			}
	}	
	*val = recpower;
	return ret;
}

extern int ReadUplinkPortSFPTemperature_10GE(short int slotno,int sfp_sel, long *val)	;

int ReadUplinkPortTemperature( short int slotno, int sfp_sel, long *val)
{
	long temperature=50, ret = -1;
	int portno = 0;
	int (*Func)(short int slotno,int sfp_sel, long *val);
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		||  (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(SYS_LOCAL_MODULE_TYPE_IS_8100_PON  && (sfp_sel < 2)))
		Func = ReadUplinkPortSFPTemperature_10GE ;
	else
		Func = ReadUplinkPortSFPTemperature;
	
	if((ret = (*Func)(slotno,sfp_sel, &temperature))==VOS_OK)
	{
		UplinkTemperature_Buffer[portno][UplinkTemperature_Buffer_Index[portno]]=temperature;
		UplinkTemperature_Buffer_Index[portno]++;
		if( UplinkTemperature_Buffer_Index[portno]>=READ_I2C_COUNT)
		{
			UplinkTemperature_Buffer_Index[portno]=0;
			UplinkTemperature_Buffer_Flag[portno]=1;
		}
		if( UplinkTemperature_Buffer_Flag[portno]==1)
		{
			temperature = ExcludeMaxMin_Then_GetAverage(&UplinkTemperature_Buffer[portno][0],READ_I2C_COUNT);
		}
	}
	*val = temperature;
	return ret;
}

extern int ReadUplinkPortSFPVoltage_10GE(short int slotno, int sfp_sel, long *val);

int ReadUplinkPortVoltage( short int slotno, int sfp_sel, long *val)
{
	long voltage=30, ret = -1;
	int portno;
	int (*Func)(short int slotno,int sfp_sel, long *val);
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(SYS_LOCAL_MODULE_TYPE_IS_8100_PON  && (sfp_sel < 2)))
		Func = ReadUplinkPortSFPVoltage_10GE ;
	else
		Func = ReadUplinkPortSFPVoltage;
		
	if((ret = (*Func)(slotno, sfp_sel, &voltage))==VOS_OK)
	{
		UplinkVoltageApplied_Buffer[portno][UplinkVoltageApplied_Buffer_Index[portno]]=voltage;
		UplinkVoltageApplied_Buffer_Index[portno]++;
		if( UplinkVoltageApplied_Buffer_Index[portno]>=READ_I2C_COUNT)
		{
			UplinkVoltageApplied_Buffer_Index[portno]=0;
			UplinkVoltageApplied_Buffer_Flag[portno]=1;
		}
		if( UplinkVoltageApplied_Buffer_Flag[portno]==1)
		{
			/*voltage=ponVoltageApplied_buffer[PonPortIdx][READ_I2C_COUNT/2];*/
			voltage = ExcludeMaxMin_Then_GetAverage(&UplinkVoltageApplied_Buffer[portno][0],READ_I2C_COUNT);
		}
	}
	*val = voltage;
	return ret;
}

extern int ReadUplinkPortSFPBiasCurrent_10GE(short int slotno, int sfp_sel, long *val);

int ReadUplinkPortBias( short int slotno, int sfp_sel, long *val)
{
	long bias=20, ret =-1;
	int portno;
	int (*Func)(short int slotno,int sfp_sel, long *val);
	
	portno = slotno*MAXUPLINKPORT+sfp_sel;
	if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
		&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))
		|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
		||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)||(SYS_LOCAL_MODULE_TYPE_IS_8100_PON  && (sfp_sel < 2)))
		Func = ReadUplinkPortSFPBiasCurrent_10GE ;
	else
		Func = ReadUplinkPortSFPBiasCurrent;

	if((ret = (*Func)(slotno, sfp_sel, &bias))==VOS_OK)
	{
		UplinkBiasCurrent_Buffer[portno][UplinkBiasCurrent_Buffer_Index[portno]]=bias;
		UplinkBiasCurrent_Buffer_Index[portno]++;
		if( UplinkBiasCurrent_Buffer_Index[portno]>=READ_I2C_COUNT)
		{
			UplinkBiasCurrent_Buffer_Index[portno]=0;
			UplinkBiasCurrent_Buffer_Flag[portno]=1;
		}
		if( UplinkBiasCurrent_Buffer_Flag[portno]==1)
		{
			bias=ExcludeMaxMin_Then_GetAverage(&UplinkBiasCurrent_Buffer[portno][0],READ_I2C_COUNT);
		}	
	}
	*val = bias;
	return ret;
}

int ReadUplinkPortSFPParametre()
{
	int sfp_sel = 0, max_uplink_sfp = 0, slotno = 0;
	long val = 0, ret = -1, lRet = 0;
	int portno = 0, times = 0, counter = 0;
	LONG max_uplink_slotno = 1;
	ULONG ulIfindex = 0;
	LONG status = -1;
	UCHAR value = 0;
	char data = 0;
	/*CHAR * * ppError;
	struct net_device * pstNetDev = NULL;
	ULONG ulSwSlot, ulSwport;*/
	
	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
		max_uplink_slotno = 1;
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)/* wangysh mod 20110303 6700 support slot2 uplink */
       {   
#ifdef _SUPPORT_8GE_6700_
              if (SYS_CHASSIS_IS_V2_8GE)
		    max_uplink_slotno = 2;
              else
                  max_uplink_slotno = 1;
#else
		max_uplink_slotno = 1;
#endif
       }
	else
		max_uplink_slotno = SYS_CHASSIS_SWITCH_SLOTNUM;

	slotno = Optical_Scope_UplinkPortIdx;
	counter = 0;
	
	for( times = 1; times <= max_uplink_slotno; times++ )
	{
		if( !SYS_MODULE_IS_UPLINK( slotno) )
		{
			slotno++;
		
			if( slotno > max_uplink_slotno)
				slotno =1;
			
			continue;
		}
		max_uplink_sfp = GetMAXUplinkPort(slotno);

		if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
			max_uplink_sfp = 2;
		
		if( max_uplink_sfp == 0 )
		{
			slotno++;
		
			if( slotno > max_uplink_slotno)
				slotno =1;
			
			continue;
		}
		counter++;
		
		if(counter == 3)
		{
			Optical_Scope_UplinkPortIdx = slotno ;
			return VOS_OK;
		}
		
		for( sfp_sel=0; sfp_sel<max_uplink_sfp; sfp_sel++ )
		{
			portno = slotno*MAXUPLINKPORT+sfp_sel;

			if((GetOltType() == V2R1_OLT_GFA6900M) 
				&& __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
				&& sfp_sel != 0 )
				continue;
			
			if( (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
				&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) )
				) ||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET)
				||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1))
			{
#if 0
				if( Check_Sfp_Online_6900_10GE( slotno, sfp_sel ) == FALSE)
				{
					UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472 ;
					continue;
				}
				else
					UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
#else
				if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
				{
					if( TRUE == i2c_data_get( g_PhysicalSlot[slotno],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 15611*/
					{
#if 0
						if( value & 1<<sfp_sel )
						{
							UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
							continue;
						}
						else
							UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
#endif
						if( value & 1<<sfp_sel )
						{
							UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
							continue;
						}
						
						if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET))
						{
							if( i2c_data_get( slotno, I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[26].cAddr, &data, 1) )
							{
								if( data & (1<<6) )	/* 0x58 == 01011000 */
								{
									if( data & (1<<4) )
										UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
									else
										UplinkPortMeteringInfo[portno].powerMeteringSupport = INTERNALLY_CALIBRATED;
								}
								else
								{
									UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
									continue;
								}
							}
							else
							{
								UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
								continue;
							}
						}
						else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
						{
							if(TRUE ==  i2c_data_get2( slotno, sfp_sel+1,0x50, XcvrInfoArr[26].cAddr, &data, 1))/*偏移为0*/
							{
						
								if( data & (1<<6) )	/* 0x58 == 01011000 */
								{
									if( data & (1<<4) )
										UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
									else
										UplinkPortMeteringInfo[portno].powerMeteringSupport = INTERNALLY_CALIBRATED;
								}
								else
								{
									UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
								}
							}
							else
							{
								UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
							}
						
						}
						else
						{
							UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
						}
							
					}
					else
					{
						UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
						continue;
					}
				}
				else
				{
					if( FALSE == Check_Sfp_Online_10GE( slotno ))
					{
						UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
						continue;
					}
					else
						UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
				}
#endif
				
			}
			else
			{
				if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN 
				|| __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA_SW)
				{
					if( Check_Sfp_Online_6100( slotno, sfp_sel ) == FALSE)
					{
						UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
						continue;
					}
				}
				
				if( CheckUplinkSupportPowerMetering( (short int)slotno, (short int)sfp_sel) == VOS_ERROR )
					continue;
			/*	if( !checkUplinkPortSFPUpdated( slotno, sfp_sel) )
					continue;  */
			}
			
			ret =ReadUplinkPortTemperature(slotno,sfp_sel, &val);  
			if(ret == VOS_OK)
			{
				if( UplinkTemperature_Buffer_Flag[portno]==1)
					CheckUplinkTemperature(slotno, sfp_sel,val );
				UplinkPortMeteringInfo[portno].Temperature=val;
			}
			
			ret=ReadUplinkPortVoltage(slotno, sfp_sel, &val); 
			if(ret == VOS_OK)
			{
				if( UplinkVoltageApplied_Buffer_Flag[portno]==1)
					CheckUplinkVoltage(slotno, sfp_sel,val );
				UplinkPortMeteringInfo[portno].Voltage=val;
			}
			
			ret=ReadUplinkPortBias(slotno, sfp_sel, &val);
			if(ret == VOS_OK)
			{
				if( UplinkBiasCurrent_Buffer_Flag[portno]==1)
					CheckUplinkBiasCurrent(slotno, sfp_sel,val );
				UplinkPortMeteringInfo[portno].BiasCurrent=val;
			}
			
			ret=ReadUplinkPortTransPower(slotno, sfp_sel, &val); 
			if(ret == VOS_OK)
			{
				if( UplinkTransOpticalPower_Buffer_Flag[portno]==1)
					CheckUplinkTransPower(slotno, sfp_sel,val);
				UplinkPortMeteringInfo[portno].transOpticalPower=val;
			}
			
			SFP_RSSI_DEBUG(0x100,(LOG_OPTICAL_POWER,LOG_INFO,"RSSI: uplink %d/%d sfp temp=%d,volt=%d,bias=%d,txp=%d\r\n", slotno, sfp_sel+1, 
				UplinkPortMeteringInfo[portno].Temperature, UplinkPortMeteringInfo[portno].Voltage,
				UplinkPortMeteringInfo[portno].BiasCurrent, UplinkPortMeteringInfo[portno].transOpticalPower));

			ret=ReadUplinkPortRecevPower(slotno, sfp_sel, &val);  
			if(ret == VOS_OK)
			{
				/* lRet = slot_port_2_swport( slotno, sfp_sel+1, &ulSwSlot, &ulSwport );*/
					lRet = getEthPortOperStatus(1, slotno, sfp_sel+1, &status); 
					/*ulIfindex = IFM_ETH_CREATE_INDEX( ulSwSlot, ulSwport );
					lRet = IFM_find_netdev( ulIfindex, &pstNetDev, ppError );*/
	   		 		
					if( lRet == VOS_OK && status == 1 /* IFM_STATE_UP*/ ) 
					{
						if( UplinkRecvOpticalPower_Buffer_Flag[portno]==1)
							CheckUplinkRecvPower(slotno, sfp_sel,val );
					}
					UplinkPortMeteringInfo[portno].recvOpticalPower=val;	
			}
#if 0
			if( __SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA6900_GEM_10GE || sfp_sel != 4)
				setUplinkPortSFPUpdated(slotno,sfp_sel);
#else
			if(GetOltType() != V2R1_OLT_GFA6900M )
			{
				if( (__SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA6900_GEM_10GE || sfp_sel != 4) &&
					(__SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA8000_8XET) && (__SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA8000_4XET)&& 
					(__SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA8000_8XETA1))
				{
					setUplinkPortSFPUpdated(slotno,sfp_sel);
				}
			}
			else
			{
				if( __SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA6900_GEM_10GE )
				{
					setUplinkPortSFPUpdated(slotno,sfp_sel);
				}
			}
#endif
		}

		slotno++;
		
		if( slotno > max_uplink_slotno)
			slotno =1;
		
	}
	return VOS_OK;
}

/*ULONG Uplink_12Epon_Check_Flag[4];*/

int ReadUplinkPortSFPParametre_12EPON()
{
	int sfp_sel, max_uplink_sfp, slotno;
	long val, ret = -1, lRet;
	int portno;
	ULONG ulIfindex ;
	LONG status = -1;

	max_uplink_sfp = 4;
	sfp_sel = 0;

	slotno = SYS_LOCAL_MODULE_SLOTNO;
	
	for( sfp_sel; sfp_sel<max_uplink_sfp; sfp_sel++ )
	{
		portno = slotno*MAXUPLINKPORT+sfp_sel;

		if(Check_Sfp_Online_12Epon_Uplink( slotno, sfp_sel) == FALSE )
		{
			 UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
			 Sfp_Uplink_Online_Flag[ portno ] = 0;
			continue;
		}

		 Sfp_Uplink_Online_Flag[ portno ] = 1;
		 
		if( CheckUplinkSupportPowerMetering( (short int)slotno, (short int)sfp_sel) == VOS_ERROR )
			continue;

		if(!SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
		{
			if( !checkUplinkPortSFPUpdated( slotno, sfp_sel) )
				continue;
		}
	
		ret =ReadUplinkPortTemperature(slotno,sfp_sel, &val);  
		if(ret == VOS_OK)
		{
			/*这里是主控代表6900M 和6900S 的情况*/
			if( SYS_LOCAL_MODULE_ISMASTERACTIVE && UplinkTemperature_Buffer_Flag[portno]==1)
					CheckUplinkTemperature(slotno, sfp_sel,val );
			UplinkPortMeteringInfo[portno].Temperature=val;
		}
		
		ret=ReadUplinkPortVoltage(slotno, sfp_sel, &val); 
		if(ret == VOS_OK)
		{
			if( SYS_LOCAL_MODULE_ISMASTERACTIVE && UplinkVoltageApplied_Buffer_Flag[portno]==1)
					CheckUplinkVoltage(slotno, sfp_sel,val );
			UplinkPortMeteringInfo[portno].Voltage=val;
		}
		
		ret=ReadUplinkPortBias(slotno, sfp_sel, &val);
		if(ret == VOS_OK)
		{
			if( SYS_LOCAL_MODULE_ISMASTERACTIVE && UplinkBiasCurrent_Buffer_Flag[portno]==1)
					CheckUplinkBiasCurrent(slotno, sfp_sel,val );
			UplinkPortMeteringInfo[portno].BiasCurrent=val;
		}
		
		ret=ReadUplinkPortTransPower(slotno, sfp_sel, &val); 
		if(ret == VOS_OK)
		{
			if( SYS_LOCAL_MODULE_ISMASTERACTIVE && UplinkTransOpticalPower_Buffer_Flag[portno]==1)
					CheckUplinkTransPower(slotno, sfp_sel,val);
			UplinkPortMeteringInfo[portno].transOpticalPower=val;
		}
		
		ret=ReadUplinkPortRecevPower(slotno, sfp_sel, &val);  
		if(ret == VOS_OK)
		{
			if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
				lRet = getEthPortOperStatus(1, slotno, sfp_sel+1, &status); 
				if( lRet == VOS_OK && status == 1 /* IFM_STATE_UP*/ ) 
				{
					if( UplinkRecvOpticalPower_Buffer_Flag[portno]==1)
						CheckUplinkRecvPower(slotno, sfp_sel,val );
				}
			}
			UplinkPortMeteringInfo[portno].recvOpticalPower=val;	
		}

		if(!SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
		{
			setUplinkPortSFPUpdated(slotno,sfp_sel);
		}
		/*Uplink_12Epon_Check_Flag[sfp_sel] = 1;*/

	}

	return VOS_OK;
}


LONG onuLaser_alwaysOn_Interval = 3;

int UplinkSFPLOSState_Report()
{
	/*int sfp_sel;
	int max_link_port;
	ULONG slotno = 1;*/
	static LONG onuLaserCheckTimer = 0;
	/*int sfp_sel, max_uplink_sfp, slotno;
	LONG max_uplink_slotno = 1;*/

/*
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEO)
		max_link_port = 4;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_GEM)
		max_link_port = 2;
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6100_GEM)
		max_link_port = 2;
	else
		return VOS_OK;
			
	for(sfp_sel=0;sfp_sel<max_link_port;sfp_sel++)
	{
		CheckUplinkLOSState(slotno, sfp_sel);
		if( sfp_debug_switch & 0x100 )
		{
			sysDateAndTime_t dt;
			eventGetCurTime( &dt );
			sys_console_printf("RSSI: uplink %d  %d:%d:%d\r\n", sfp_sel+1,dt.hour, dt.minute, dt.second);
		}			
	}
*/

	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		if( onuLaserCheckTimer > onuLaser_alwaysOn_Interval )
		{
			if(V2R1_ENABLE == onuLaser_alwaysOn_Enable)
				OltPonpowerAlwaysOnDetection();

			onuLaserCheckTimer = 0;
		}
		else
			onuLaserCheckTimer++;

		if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
 			return VOS_OK;
	}
	
	/*if( (SYS_PRODUCT_TYPE == PRODUCT_E_EPON3) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100) )
		max_uplink_slotno = 1;
	else
	{
		max_uplink_slotno = SYS_CHASSIS_SWITCH_SLOTNUM ;
	}
	
	for( slotno=1; slotno<=max_uplink_slotno; slotno++ )
	{
		max_uplink_sfp = GetMAXUplinkPort(slotno);

		if( max_uplink_sfp == 0 )
			continue;
		
		for( sfp_sel=0; sfp_sel<max_uplink_sfp; sfp_sel++ )
		{
			if(VOS_ERROR == CheckUplinkLOSState(slotno,sfp_sel))
				continue;
		}
	}*/

/*	while(VOS_OK == getnext_uplink_sys(cur_port,  next_port))
	{
		cur_port = *next_port ;
		CheckUplinkLOSState(cur_port);
		if( sfp_debug_switch & 0x100 )
		{
			sysDateAndTime_t dt;
			eventGetCurTime( &dt );
			sys_console_printf("RSSI: uplink %d  %d:%d:%d\r\n", cur_port+1,dt.hour, dt.minute, dt.second);
		}		
	}*/

	return VOS_OK;
}

/* added by xieshl 20110418, CTC 互通ONU光功率检测 */
LONG ctcOnuPowerMetering( short int PonPortIdx, short int OnuIdx )
{
	int ret;
	short int llid;
	CTC_STACK_optical_transceiver_diagnosis_t optical;
	OpticalScope_RspMsg_t  RecvOamMsg;

	if( !V2R1_CTC_STACK )
		return VOS_OK;

	SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER, LOG_INFO, "\n power metering ctc onu %d/%d/%d\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), (OnuIdx+1)) );
			
	llid = GetLlidByOnuIdx(PonPortIdx,OnuIdx);
	if(llid == INVALID_LLID ) 
		return VOS_ERROR;

	VOS_MemZero( &optical, sizeof(CTC_STACK_optical_transceiver_diagnosis_t) );
	/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	ret = OnuMgt_GetOptTransDiag( PonPortIdx, OnuIdx, &optical );
	if( ret == CTC_STACK_EXIT_OK )
	{
		VOS_MemZero( &RecvOamMsg, sizeof(RecvOamMsg) );

		RecvOamMsg.OamHeader.GwOpcode = GW_OPCODE_EUQ_INFO_RESPONSE;
		RecvOamMsg.OamBody.subGwOpcode = GET_ONU_VIRTUAL_SCOPE_OPTICAL_RSP;
		RecvOamMsg.OamBody.sendPower = optical.tx_power;
		RecvOamMsg.OamBody.recvPower = optical.rx_power;
		RecvOamMsg.OamBody.temperature = optical.transceiver_temperature;
		RecvOamMsg.OamBody.workVoltage = optical.supply_voltage;
		RecvOamMsg.OamBody.biasCurrent = optical.tx_bias_current;

		GwOamMsg_OpticalScope_Handler( PonPortIdx, OnuIdx, sizeof(RecvOamMsg), &RecvOamMsg );


		/*SFP_RSSI_DEBUG(1,("\nonu %d-%d tx-power=%ld,rx-power=%ld\r\n",PonPortIdx, OnuIdx+1,TranslateOpticalPower(optical.tx_power),
						TranslateOpticalPower(optical.rx_power)) );
		SFP_RSSI_DEBUG(1,("  temperature=%ld,voltage=%ld,bias=%ld\r\n",PonPortIdx, OnuIdx+1,
						TranslateTemperature(optical.transceiver_temperature),
						TranslateWorkVoltage(optical.supply_voltage) ,TranslateBiasCurrent(optical.tx_bias_current)) );*/
	}
	return VOS_OK;
}

int  Main_OltPonpowermetering(void)
{
	short int PonPortIdx = 0, OnuIdx = 0;
	int LastCheckId[4] = {0};
	long val = 0,ret = -1, i = 0, j = 0, flag = 0, temp = 0 ;
	ULONG check_num = 0;
	/*ComputeTimeInterverl();*/
	/*判断本板是否为主控板，本板是否运行*/
	/*if( !(SYS_LOCAL_MODULE_ISMASTERACTIVE || SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO)) )
		return VOS_OK;*/
	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
		return(RERROR);	
	
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ReadUplinkPortSFPParametre();

		if( PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE) )
			return VOS_OK;
	}

	if( SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON )
	{
		ReadUplinkPortSFPParametre_12EPON();
	}

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
		check_num = 1;
	else
		check_num = 4;

	VOS_MemZero( LastCheckId, sizeof(LastCheckId) );
	VOS_MemSet((UCHAR *)LastCheckId, 0xFF, sizeof( LastCheckId ) );
	
	for( i = 0; i < check_num; i++  )
	{
		PonPortIdx = FindOpticalScopePonPort();/*找到下一个等待检测的PON口*/

		if((PonPortIdx < 0 ) || (PonPortIdx > MAXPON ))
			return(RERROR);
		
		LastCheckId[ flag ] = PonPortIdx ;

		for( j = 0; j< check_num; j++ )
		{
			if(flag != j)
			{
				if( LastCheckId[j] == PonPortIdx)
					return VOS_OK;
			}
		}

		flag++;

		if(flag >= check_num )
			flag = 0;

		/*if( sfp_debug_switch & 1 )
		{
			sysDateAndTime_t dt;
			eventGetCurTime( &dt );
			sys_console_printf("RSSI: pon%d/%d metering %d:%d:%d\r\n", GetCardIdxByPonChip(PonPortIdx), (GetPonPortByPonChip(PonPortIdx) ),
				dt.hour, dt.minute, dt.second);
		}*/
		/*SFP_RSSI_DEBUG(1,("power metering PON %d start\r\n", PonPortIdx) );*/

		ret = ReadPonPortTransPower( PonPortIdx, &val);
		if(ret == VOS_OK)
		{
			val = val + olt_tx_optical_power_calibration;

			if( PonTransOpticalPower_Buffer_Flag[PonPortIdx]==1)
				CheckPonPortTransOpticalPower( PonPortIdx, val );
			PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = val;	/*刷新OLT sfp光模块参数信息表*/
		}
		ret = ReadPonPortTemperature( PonPortIdx, &val);
		if(ret == VOS_OK)
		{
			if( PonTemperature_Buffer_Flag[PonPortIdx]==1)
				CheckPonPortTemperature(PonPortIdx, val );
			PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature = val;
		}
		
		ret = ReadPonPortBias( PonPortIdx, &val);
		if(ret == VOS_OK)
		{
			if( PonBiasCurrent_Buffer_Flag[PonPortIdx]==1)
				CheckPonPortBiasCurrent( PonPortIdx, val );
			PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = val;
		}
		
		ret = ReadPonPortVoltage( PonPortIdx, &val );
		if(ret == VOS_OK)
		{
			if( PonVoltageApplied_Buffer_Flag[PonPortIdx]==1)
				CheckPonPortWorkVoltage( PonPortIdx, val );
			PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = val;
		}
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"olt-pon%d tx-p=%d,temp=%d,bias=%d,volt=%d\r\n", PonPortIdx, PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower,
			PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature, PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent,
			PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied) );

		for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
			int onuType = 0;
			/*查找在线的ONU*/
			if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP)
				continue;

		/* added by xieshl 20110418, CTC 互通ONU光功率检测 */

		ret = ReadPonPortRecevPower(PonPortIdx,OnuIdx,1, &val);
		if(ret == VOS_OK)
		{
			if( val != temp)
			{
				val = val+olt_rx_optical_power_calibration;
				
				if( PonRecvOpticalPower_Buffer_Flag[PonPortIdx][OnuIdx]==1)
					CheckPonPortRecvOpticalPower(PonPortIdx, OnuIdx, val );
				if(abs(PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx] -val) > 0.01)
					Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;
				/*刷新OLT接收功率信息*/
				PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[OnuIdx] = 1;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuIdx] = val;
			}
		}
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"rx-p(ONU%d)=%d\r\n", OnuIdx+1, val) );	
	
	      /*if( V2R1_CTC_STACK )
			{
				if( GetOnuType(PonPortIdx, OnuIdx, &onuType) == VOS_ERROR )
					continue;
				if( onuType == V2R1_ONU_CTC )
				{
					ctcOnuPowerMetering( PonPortIdx, OnuIdx );
				}
			}*/
		}
#if 0
		if( GetOnuOpticalPowerEnable() == V2R1_ENABLE )
			PollingPonPortOpticalScope(PonPortIdx);
#endif
	}
	
	return VOS_OK;
}

int  Main_OnuPonpowermetering(void)
{
	short int PonPortIdx, OnuIdx;
	int LastCheckId[16];
	long val,ret = -1, i, j, flag = 0, temp = 0 ;
	ULONG check_num ;
	
	if(GetOnuOpticalPowerEnable() != V2R1_ENABLE)
		return(RERROR);	

      if(SYS_LOCAL_MODULE_ISMASTERACTIVE && !SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	  	return(ROK);
	
	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
		check_num = 2;
	else
		check_num = 8;

	VOS_MemZero( LastCheckId, sizeof(LastCheckId) );
	VOS_MemSet((UCHAR *)LastCheckId, 0xFF, sizeof( LastCheckId ) );

	flag = 0;
	
	for( i = 0; i < check_num; i++  )
	{
		PonPortIdx = FindOpticalScopePonOnuPort();/*找到下一个等待检测的PON口*/

		if((PonPortIdx < 0 ) || (PonPortIdx > MAXPON ))
			return(RERROR);
		
		LastCheckId[ flag ] = PonPortIdx ;

		for( j = 0; j< check_num; j++ )
		{
			if(flag != j)
			{
				if( LastCheckId[j] == PonPortIdx)
					return VOS_OK;
			}
		}

		flag++;

		if(flag >= check_num )
			flag = 0;

		for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
		{
			int onuType = 0;
			/*查找在线的ONU*/
			if(GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP)
				continue;
			
			if( V2R1_CTC_STACK )
			{
				if( GetOnuType(PonPortIdx, OnuIdx, &onuType) == VOS_ERROR )
					continue;
				if( (onuType == V2R1_ONU_CTC) || (V2R1_ONU_GPON == onuType) )
				{
					ctcOnuPowerMetering( PonPortIdx, OnuIdx );
				}
				else
				{
					UnicastOamMsg_GetOpticalScopeToOnu(PonPortIdx, OnuIdx);
				}
			}
		}
		
		if( GetOnuOpticalPowerEnable() == V2R1_ENABLE && !SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
			PollingPonPortOpticalScope(PonPortIdx);
		
	}
	
	return VOS_OK;
	
}


DEFUN( OnuLaser_AlwaysOn_Alarm_Threshold_Func,
        OnuLaser_AlwaysOn_Alarm_Threshold_Cmd,
        "config onu-laser-always-on alarm-threshold [<threshold>|default]",
        CONFIG_STR
	"onu laser always on\n"
	"lowest alarm threshold\n"
	"inputalarm threshold value, from -32 to -10 dbm\n"
        "set to default value\n"
      )
{
	LONG thre_val=0;
	if(0 == VOS_StrCmp( argv[0],"default"))
	{
		onuLaser_alwaysOn_alarm_threshold = OnuLaser_AlwaysOn_Alarm_Threshold_Default;
              thre_val = OnuLaser_AlwaysOn_Alarm_Threshold_Default;
	}
	else
	{
		thre_val = VOS_AtoL( argv[ 0 ] );
		if(thre_val < -32 || thre_val > -10)
		{
			vty_out(vty,"The range should be from -32 dbm to -10 dbm, please input again .\r\n");
			return CMD_WARNING;
		}
		thre_val = thre_val*10;
	}

	/*if(VOS_OK == RPU_SendCmd2OpticalPower(onu_laser_always_on_alarm_threshold,-1,NULL,0,thre_val,0,0,0))
	{
		onuLaser_alwaysOn_alarm_threshold = thre_val ;
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/

	Set_OnuLaserAlwaysOn_AlarmThreshold(-1,thre_val);
	
	return CMD_SUCCESS;

}

/*LONG Set_OnuLaserAlwaysOn_AlarmTimes(int alarm_times)
{
	onuLaser_alwaysOn_alarm_timeCounter = alarm_times;
}*/

DEFUN( OnuLaser_AlwaysOn_Alarm_Times_Func,
        OnuLaser_AlwaysOn_Alarm_Times_Cmd,
        "config onu-laser-always-on alarm-times [<2-20>|default]",
	CONFIG_STR
	"onu laser always on\n"
	"always on alarm times\n"
	"input alarm times value\n"
        "set to default value\n"
      )
{
	long alarm_times = 0;
	if(0 == VOS_StrCmp( argv[0],"default"))
	{
		alarm_times = OnuLaser_AlwaysOn_Alarm_TimeCounter_Default ;
	}
	else
	{
		alarm_times = VOS_AtoL( argv[ 0 ] ) ;
		
		if( alarm_times < 2 || alarm_times >20 )
		{
			vty_out(vty, " The value you input is illegal !\r\n");
			return CMD_WARNING;
		}
	}

	/*if(VOS_OK == RPU_SendCmd2OpticalPower(onu_laser_always_on_alarm_times,-1,NULL,0,alarm_times,0,0,0))
	{
		onuLaser_alwaysOn_alarm_timeCounter = alarm_times ;
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/

	Set_OnuLaserAlwaysOn_AlarmTimes(-1,alarm_times);
	
	return CMD_SUCCESS;
}

DEFUN( OnuLaser_AlwaysOn_AlarmClear_Times_Func,
        OnuLaser_AlwaysOn_AlarmClear_Times_Cmd,
        "config onu-laser-always-on alarm-clear-times [<1-20>|default]",
	CONFIG_STR
	"onu laser always on\n"
	"always on alarm clear times\n"
	"alarm times value\n"
        "set to default value\n"
      )
{
	long clear_times = 0;
	if(0 == VOS_StrCmp( argv[0],"default"))
	{
		clear_times = OnuLaser_AlwaysOn_Clear_TimeCounter_Default ;
	}
	else
	{
		clear_times = VOS_AtoL( argv[ 0 ] ) ;
	}

	/*if(VOS_OK == RPU_SendCmd2OpticalPower(onu_laser_always_on_alarm_clear_times,-1,NULL,0,clear_times,0,0,0))
	{
		onuLaser_alwaysOn_clear_timeCounter = clear_times ;
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/

	Set_OnuLaserAlwaysOn_ClearTimes(-1,clear_times);
	
	return CMD_SUCCESS;
}

long GetOnuLaserAlwaysOnThred()
{
	return decimal2_integer_part(onuLaser_alwaysOn_alarm_threshold);
}

DEFUN( Show_OnuLaser_AlwaysOn_Func,
        Show_OnuLaser_AlwaysOn_Cmd,
        "show onu-laser-always-on {[config|status]}*1",
        SHOW_STR
	"onu laser always on\n"
	"show config\n"
	"show onu laser always on alarm\n")
{
	int i;
	USHORT slot, port;
	
	if( (argc == 1) && (argv[0][0] == 'c') )
	{
		vty_out(vty," onu laser always on is %s\r\n", (V2R1_ENABLE == onuLaser_alwaysOn_Enable) ? "enable" : "disable" );
		vty_out(vty," alarm threshold: %d\r\n", decimal2_integer_part(onuLaser_alwaysOn_alarm_threshold) );
		vty_out(vty," alarm times: %d\r\n", onuLaser_alwaysOn_alarm_timeCounter);
		vty_out(vty," alarm clear times: %d\r\n", onuLaser_alwaysOn_clear_timeCounter);
	}
	else
	{
		if(MAXPON > OPT_POWER_PON_MAX_NUM)
		{
			vty_out(vty, "Error, MAXPON is bigger then  OPT_POWER_PON_MAX_NUM !\r\n");
			return CMD_WARNING;
		}
		
		for( i = 0; i<MAXPON; i++ )
		{			
			if(onuLaser_alwaysOn_alarm_record[i][0] != 0)
			{
				slot = GetCardIdxByPonChip(i);
				port = GetPonPortByPonChip(i);
				
				vty_out(vty,"\r\n onu %d/%d/0 laser always on, rxp=%d.%d\r\n", slot, port,
						decimal2_integer_part( onuLaser_alwaysOn_alarm_record[i][0] ),
						decimal2_fraction_part( onuLaser_alwaysOn_alarm_record[i][0] ));
			}
		}
#if 0
		pBufLen = MAXPON*sizeof(LONG);
		pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
		if(pBuf == NULL)
		{
			VOS_ASSERT( 0 );
			return VOS_ERROR;
		}
		VOS_MemZero(pBuf, pBufLen);
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(show_onu_laser_always_on,-1,"status",6,0,0,0,0))
		{
			
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/

		iRlt = Fetch_OnuLaser_AlwaysOn(0, 0, "status",(void *)pBuf/*(void *)vty*/);

		if(VOS_OK == iRlt)
		{
			Show_OpticalPower_OnuLaser_FromPon( 0/*ulSlot*/, 0 /*ulPort*/, pBuf, pBufLen,"status"/*TypeFlag*/,vty/*(struct vty *)pointer*/);
		}

		VOS_Free( pBuf);
#endif
	}

	return CMD_SUCCESS;
}

DEFUN(config_show_optical_always_onl_value,
	config_show_optical_always_onl_value_cmd,
	"show onu-laser-always-on value <slot/port>",
	SHOW_STR
	"onu laser always on\n"
	"pon real-time monitor value\n"
	"pon slot/port\n"
	)
{
	unsigned long ulSlot, ulPort;
	short int PonPortIdx;
	UCHAR *pBuf;
	int iRlt,pBufLen;
	
	if( IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort ) != VOS_OK )
		return CMD_WARNING;

	PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return CMD_WARNING;

	if(V2R1_DISABLE == onuLaser_alwaysOn_Enable)
	{
		vty_out(vty,"onu laser always on detection is disabled!\r\n");
		return CMD_SUCCESS;
	}
	if( onuLaser_alwaysOn_check_support[PonPortIdx] == V2R1_DISABLE )
	{
		vty_out(vty,"optical power monitor not support!\r\n");
		return CMD_SUCCESS;
	}

	/*if(VOS_OK == RPU_SendCmd2OpticalPower(show_onu_laser_always_on,PonPortIdx,"value ",6,0,0,0,0))
	{
		
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/
	pBufLen = MAXPON*sizeof(LONG);
	pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);
	iRlt = Fetch_OnuLaser_AlwaysOn(ulSlot, ulPort,"value",(void *)pBuf/*(void *)vty*/);
	if(VOS_OK == iRlt)
	{
		Show_OpticalPower_OnuLaser_FromPon( ulSlot, ulPort, pBuf, pBufLen,"value"/*TypeFlag*/,vty/*(struct vty *)pointer*/);
	}
	VOS_Free( pBuf);

	return CMD_SUCCESS;
}

int Show_Pon_Sfp_Type(char *SFPTypeString,short int getflag,void *vty)
{
	int sfpstring[SFP_TYPE_Vendor_len];
	/*int i=0;*/
	
	VOS_MemZero(sfpstring, SFP_TYPE_Vendor_len);
	
	vty_out( vty, "optical module type:" );
	if(getflag == 1)
	{
		/*sys_console_printf("size : %d\r\n", VOS_StrLen( SFPTypeString));
		for( i = 0; i<=10; i++)
			sys_console_printf("%02x ",SFPTypeString[i]);*/
		
		if( VOS_StrLen( SFPTypeString) > 0 )
		{
			VOS_StrCpy(sfpstring, SFPTypeString);/*问题单: 16974 modified by duzhk 2013-02-06*/
			vty_out( vty, "%s\r\n", sfpstring );
		}
		else
			vty_out( vty, " \r\n", sfpstring );
	}
	else if(getflag == 0)
		vty_out( vty, "unknown\r\n" );
	else
		vty_out(vty,"error\r\n");

	return VOS_OK;
}

DEFUN( Show_Sfp_Type_Func,
        Show_Sfp_Type_Cmd,
        "show pon-sfp-type",
        SHOW_STR
	"optical module type\n"
      )
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	UCHAR *pBuf;
	/*char SFPTypeString[SFP_TYPE_Vendor_len];*/
	int iRlt,pBufLen,i;
	
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	pBufLen = SFP_TYPE_Vendor_len;
	pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);
	
	iRlt = Fetch_Optical_Power_PonSfpType(ulSlot,ulPort,(void*)pBuf /*(void*)vty*/);

	for(i = 0;i<SFP_TYPE_Vendor_len;i++)
	{
		if(pBuf[i]==0x20 || pBuf[i] == 0x43)
		{
			pBuf[i] = 0x0;
		}
	}
	if(VOS_OK == iRlt)
	{
		Show_Pon_Sfp_Type(pBuf,1,vty/*pointer*/);
	}
	else
	{
		Show_Pon_Sfp_Type(pBuf,0,vty/*pointer*/);
	}
	VOS_Free( pBuf);
	
	return CMD_SUCCESS;
}


/*
DEFUN( Show_Sfp_Test_Func,
        Show_Sfp_Test_Cmd,
        "show pon-sfp test",
        SHOW_STR
	"optical module type\n"
      )
{
	unsigned long ulSlot, ulPort, ulOnuId;
	short int PonPortIdx;
	char SFPTypeString[SFP_TYPE_Vendor_len+6];
	if( parse_pon_command_parameter( vty, &ulSlot, &ulPort , &ulOnuId, &PonPortIdx) != VOS_OK )
		return CMD_WARNING;

	vty_out( vty, "optical module type:" );
	if(ponSFPTest_read(PonPortIdx, SFPTypeString) == ROK)
		vty_out( vty, "%s\r\n", SFPTypeString );
	else
		vty_out( vty, "unknown\r\n" );

	return CMD_SUCCESS;
}
*/


LONG CheckOnuLaserAlwaysOn( short int PonPortIdx, LONG lDbm )
{
	if( lDbm > onuLaser_alwaysOn_alarm_threshold )
	{
		onuLaser_alwaysOn_alarm_record[PonPortIdx][1] =  lDbm ;/*record 的第二列 存储最近一次检测到的强发光光功率*/
		/*llid==0 时隙收到的光功率大于设定的临界值*/
		if( onuLaser_alwaysOn_alarm_flag[PonPortIdx][1]  == 0 )
		{
			/*flag 的第一列表示连续的周期个数，第二列表示是否已经报告强发光告警*/
			onuLaser_alwaysOn_alarm_flag[PonPortIdx][0]++;
			/*连续AlwaysOn_Alarm_Times 个周期收到的光功率都大于设定的临界值*/
			if( onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] >= onuLaser_alwaysOn_alarm_timeCounter )
			{
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] = 0;
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][1] = 1;
				
				/*产生告警*/
				/*record 的第一列存储 已检测到的 强发光告警的光功率数值*/
				
				onuLaser_alwaysOn_alarm_record[PonPortIdx][0] = lDbm;
				ponPortLaserAlwaysOn_EventReport( GetCardIdxByPonChip( PonPortIdx), GetPonPortByPonChip(PonPortIdx), 0, lDbm );
				Optical_Timer_Callback();
			}
		}
		else
		{
			onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] = 0;
		}
	}
	else
	{
		onuLaser_alwaysOn_alarm_record[PonPortIdx][1] =  0 ;/*存储最近一次检测到的强发光光功率*/
		if(onuLaser_alwaysOn_alarm_flag[PonPortIdx][1]  == 1)
		{
			onuLaser_alwaysOn_alarm_flag[PonPortIdx][0]++;
			if(onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] >= onuLaser_alwaysOn_clear_timeCounter)
			{
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] = 0;
				onuLaser_alwaysOn_alarm_flag[PonPortIdx][1] = 0;
				
				/*消除告警*/
				onuLaser_alwaysOn_alarm_record[PonPortIdx][0] = 0;
				ponPortLaserAlwaysOnClear_EventReport( GetCardIdxByPonChip( PonPortIdx), GetPonPortByPonChip(PonPortIdx), 0, 0 );
			}
		}
		else
		{
			onuLaser_alwaysOn_alarm_flag[PonPortIdx][0] = 0;
		}
	}
	return VOS_OK;
}
/*
ULONG PonPower_AlwaysOn_Test_Switch = 0;
*/


extern PON_STATUS PAS_set_virtual_llid(const PON_olt_id_t olt_id, const PON_llid_t llid, const PON_virtual_llid_operation_t operation);

int  OltPonpowerAlwaysOnDetection(void)
{
	short int PonPortIdx/*, OnuIdx*/;
	PON_onu_id_t       virtual_onu_id = PON_VIRTUAL_LLID;
    	short int          return_result;
	unsigned short int sample;
	float              voltage;
	float              dbm;
	/*ULONG testflag = 0;*/
	LONG val=0;
	/*char temp[128];*/

	/*判断本板是否为主控板，本板是否运行*/
	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER || !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO))
		return VOS_OK;
	
	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
		return(RERROR);	

	/*PonPortIdx = FindOpticalScopePonPort();*/      /*找到下一个等待检测的PON口*/
	/*PonPortIdx = 4;*/
	for(PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
	{
		if( onuLaser_alwaysOn_check_support[PonPortIdx] == 0 )
		{
			if(CheckSpePonSFPSupportRSSI(PonPortIdx)!=VOS_OK)
			{
				onuLaser_alwaysOn_check_support[PonPortIdx] = V2R1_DISABLE;
				continue;
			}
			onuLaser_alwaysOn_check_support[PonPortIdx] = V2R1_ENABLE;
		}
		if( onuLaser_alwaysOn_check_support[PonPortIdx] != V2R1_ENABLE )
			continue;
	
		/*Add virtual_llid*/
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		return_result = OLT_SetVirtualLLID(PonPortIdx, virtual_onu_id,PON_ADD_VIRTUAL_LLID);
		if (return_result != PAS_EXIT_OK)
		{ 
			SFP_RSSI_DEBUG(0x10,(LOG_OPTICAL_POWER,LOG_INFO,"onu_laser_always_on check start failed PON %d ONU %d Error:%d\r\n",
	                       PonPortIdx, virtual_onu_id, return_result));
			return (return_result);
		}
		 
		/*Measure virtual_llid*/
		return_result = OLT_GetVirtualScopeOnuVoltage(PonPortIdx, virtual_onu_id, &voltage, &sample, &dbm);
		
		if (return_result != PAS_EXIT_OK)
		{ 
			SFP_RSSI_DEBUG(0x10,(LOG_OPTICAL_POWER,LOG_INFO,"onu_laser_always_on check failed PON:%d ONU:%d (%d%s)\r\n",
	                       PonPortIdx, virtual_onu_id, return_result, (return_result == PAS_TIME_OUT) ? " TIMEOUT" : ""));
		}
		else
		{
			/* calc threshold and send alarm if needed */
			val = (LONG)(dbm * 10);
			CheckOnuLaserAlwaysOn( PonPortIdx, val );
		}
	
		/*Remove virtual_llid*/
		/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		return_result = OLT_SetVirtualLLID(PonPortIdx, virtual_onu_id, PON_REMOVE_VIRTUAL_LLID);
		if (return_result != PAS_EXIT_OK)
		{ 
			SFP_RSSI_DEBUG(0x10,(LOG_OPTICAL_POWER,LOG_INFO,"onu_laser_always_on check end failed PON %d ONU %d Error:%d%\n",
	                       PonPortIdx, virtual_onu_id, return_result));
			return (return_result);
		}
		if((val > 1000) || val < -1000 ) val = 0;
		SFP_RSSI_DEBUG(0x10,(LOG_OPTICAL_POWER,LOG_INFO,"onu_laser_always_on check ok for PON:%d ONU:%d, %d(0.1dbm)\r\n", PonPortIdx, virtual_onu_id, val));
	}

	return VOS_OK;
}

void  PollingPonPortOpticalScope(short int PonPortIdx)
{	
	
/*	Optical_Scope_Timer_Counter++;
	if( Optical_Scope_Timer_Counter >= Optical_Scope_Sample_Interval_1 )
		Optical_Scope_Timer_Counter=0;
*/	
	if( GetOnuOpticalPowerEnable()!= V2R1_ENABLE) 
			return;
	if(PonPortIdx != RERROR)
	{
		BroadcastOamMsg_GetOpticalScopeToOnu(PonPortIdx);		
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"broadcast power-req OAM to PON %d\r\n", PonPortIdx) );
	}
}

static void ComposeGetOpticalOamMsg (OpticalScope_GetMsg_t *OamMsg)
{
	unsigned char OamMsgDstMac[BYTES_IN_MAC_ADDRESS] = {0x01,0x80,0xc2,0x00,0x00,0x02};
	unsigned char OamMsgSrcMac[BYTES_IN_MAC_ADDRESS] = {0x00,0x0f,0xe9,0x00,0x00,0x00};
	
	VOS_MemCpy( OamMsg->OamHeader.DA, OamMsgDstMac, BYTES_IN_MAC_ADDRESS );
	VOS_MemCpy( OamMsg->OamHeader.SA, OamMsgSrcMac, BYTES_IN_MAC_ADDRESS );
	OamMsg->OamHeader.Type = 0x8809;
	OamMsg->OamHeader.SubType = 0x03;
	OamMsg->OamHeader.Flag = 0x0050;
	OamMsg->OamHeader.Code = 0xfe;
	
	OamMsg->OamHeader.OUI[0] = 0x00;
	OamMsg->OamHeader.OUI[1] = 0x0f;
	OamMsg->OamHeader.OUI[2] = 0xe9;
	OamMsg->OamHeader.GwOpcode = GW_OPCODE_EUQ_INFO_REQUESET;
	OamMsg->OamHeader.SendSerNo = 0;
	OamMsg->OamHeader.WholePktLen = sizeof(OpticalScope_GetMsg_t);
	OamMsg->OamHeader.PayLoadOffset = 0;
	OamMsg->OamHeader.PayLoadLength = sizeof(OpticalScope_GetMsg_t);
	VOS_MemZero(OamMsg->OamHeader.SessionID, 8);
	
	OamMsg->OamBody.subGwOpcode = GET_ONU_VIRTUAL_SCOPE_OPTICAL_REQ;
}

/*  向ONU 广播OAM 消息，查询ONU 光模块参数*/
int BroadcastOamMsg_GetOpticalScopeToOnu(short int PonPortIdx)
{
	OpticalScope_GetMsg_t  OamMsg;
	CHECK_PON_RANGE
	VOS_MemZero( &OamMsg, sizeof(OpticalScope_GetMsg_t));

	ComposeGetOpticalOamMsg(&OamMsg);

	BroadcastOamFrameToOnu( PonPortIdx, sizeof(OpticalScope_GetMsg_t), (UCHAR*)&OamMsg );

	return(ROK);
}

/*added by wangjiah@2016-09-22 : begin*/
/* Unicast oam msg to get optical params*/
int UnicastOamMsg_GetOpticalScopeToOnu(short int PonPortIdx, short int OnuIdx)
{
	OpticalScope_GetMsg_t  OamMsg;
	short int llid = INVALID_LLID;
	VOS_MemZero( &OamMsg, sizeof(OpticalScope_GetMsg_t));

	CHECK_PON_RANGE

	llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	if( llid == INVALID_LLID) return RERROR; 
	
	ComposeGetOpticalOamMsg(&OamMsg);

	UnicastOamFrameToOnu( PonPortIdx, llid, sizeof(OpticalScope_GetMsg_t), (UCHAR*)&OamMsg );

	return(ROK);
}
/*added by wangjiah@2016-09-22 : end*/

/* 判断是否为查询ONU 光模块参数相应消息*/
int  ReceivedFrameIsGwOAM_OpticalScope_Rsp( unsigned char *content )
{
	OpticalScope_RspMsg_t  *RecvMsg;
	
	if( content == NULL ) return (FALSE );
	
	RecvMsg = (OpticalScope_RspMsg_t *)content;
	if((RecvMsg->OamHeader.GwOpcode == GW_OPCODE_EUQ_INFO_RESPONSE) && (RecvMsg->OamBody.subGwOpcode == GET_ONU_VIRTUAL_SCOPE_OPTICAL_RSP))
		return(TRUE);

	return(FALSE);
}
/* 对查询ONU 光模块参数相应消息的处理*/
int GwOamMsg_OpticalScope_Handler(short int PonPortIdx, short int OnuIdx, short int Length, unsigned char *content)
{
	OpticalScope_RspMsg_t  *RecvOamMsg;
	short int OnuEntry;

	if((Length == 0) ||(content == NULL )) 
		return(RERROR);
	CHECK_ONU_RANGE

	if( GetOnuOpticalPowerEnable() == V2R1_DISABLE )
		return VOS_ERROR;
		
	SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"recv power-rep OAM from ONU %d-%d\r\n", PonPortIdx, OnuIdx+1) );

	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	RecvOamMsg = (OpticalScope_RspMsg_t *)content;
	if((RecvOamMsg->OamHeader.GwOpcode == GW_OPCODE_EUQ_INFO_RESPONSE) && (RecvOamMsg->OamBody.subGwOpcode == GET_ONU_VIRTUAL_SCOPE_OPTICAL_RSP))
	{
		if(RecvOamMsg->OamBody.sendPower == -1 ||
			RecvOamMsg->OamBody.recvPower == -1 ||
			RecvOamMsg->OamBody.temperature == -1 ||
			RecvOamMsg->OamBody.workVoltage == -1 ||
			RecvOamMsg->OamBody.biasCurrent == -1 ||
			RecvOamMsg->OamBody.sendPower == 0 ||
			RecvOamMsg->OamBody.recvPower == 0 ||
			RecvOamMsg->OamBody.temperature == 0 ||
			RecvOamMsg->OamBody.workVoltage == 0 ||
			RecvOamMsg->OamBody.biasCurrent == 0 )
		{
			if( 1 == OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag
			    && Onu_OpticalInfo_Support_Change_Flag[PonPortIdx][OnuIdx] > 3 )
			{
				OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag = 0;
				Onu_OpticalInfo_Support_Change_Flag[PonPortIdx][OnuIdx] = 0;
			}
			else 
				Onu_OpticalInfo_Support_Change_Flag[PonPortIdx][OnuIdx]++;
		}
		else/*在oam报文中得到了值，此时将flag置位1*/
		{
			OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag = 1;
			Onu_OpticalInfo_Support_Change_Flag[PonPortIdx][OnuIdx] = 0;
		}
	}
	if( OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag==1 )
	{
		/*CheckOnuTransOpticalPower(PonPortIdx, OnuIdx,TranslateOpticalPower(RecvOamMsg->OamBody.sendPower));
		CheckOnuRecvOpticalPower(PonPortIdx, OnuIdx, TranslateOpticalPower(RecvOamMsg->OamBody.recvPower));
		CheckOnuTemperature(PonPortIdx, OnuIdx, TranslateTemperature(RecvOamMsg->OamBody.temperature));
		CheckOnuWorkVoltage(PonPortIdx, OnuIdx, TranslateWorkVoltage(RecvOamMsg->OamBody.workVoltage));
		CheckOnuBiasCurrent(PonPortIdx, OnuIdx,TranslateBiasCurrent(RecvOamMsg->OamBody.biasCurrent));*/

		/*if(abs(OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower -TranslateOpticalPower(RecvOamMsg->OamBody.recvPower)) > 
																	onu_rx_optical_power_calibration + 0.1)*/
		{
			Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;
		}
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		{
			OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = GponTranslateOpticalPower(RecvOamMsg->OamBody.recvPower)
																	+onu_rx_optical_power_calibration;
		}
		else
		{
			OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = TranslateOpticalPower(RecvOamMsg->OamBody.recvPower)
																	+onu_rx_optical_power_calibration;
		}
		CheckOnuRecvOpticalPower(PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower);															+ onu_rx_optical_power_calibration;
		
		/*if(abs(OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower -TranslateOpticalPower(RecvOamMsg->OamBody.sendPower)) > 
																	onu_tx_optical_power_calibration + 0.1)*/
		{
			Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;
		}
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		{
			OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = GponTranslateOpticalPower(RecvOamMsg->OamBody.sendPower)
																	+ onu_tx_optical_power_calibration;
		}
		else
		{
			OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = TranslateOpticalPower(RecvOamMsg->OamBody.sendPower)
																	+ onu_tx_optical_power_calibration;
		}
		CheckOnuTransOpticalPower(PonPortIdx, OnuIdx,OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower);

		/*if(abs(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature -TranslateTemperature(RecvOamMsg->OamBody.temperature)) > 0.1)*/
			Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature = RecvOamMsg->OamBody.temperature;			
		else
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature = TranslateTemperature(RecvOamMsg->OamBody.temperature);
		CheckOnuTemperature(PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature );
		
		/*if(abs(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied  -TranslateWorkVoltage(RecvOamMsg->OamBody.workVoltage)) > 0.1)*/
			Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;
		
		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied = 10 * RecvOamMsg->OamBody.workVoltage;			
		else
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied = TranslateWorkVoltage(RecvOamMsg->OamBody.workVoltage);
		CheckOnuWorkVoltage(PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied );
		
		/*if(abs(OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent  -TranslateBiasCurrent(RecvOamMsg->OamBody.biasCurrent)) > 0.1)*/
			Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuIdx]++;

		if(SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent = RecvOamMsg->OamBody.biasCurrent;
		else		
			OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent = TranslateBiasCurrent(RecvOamMsg->OamBody.biasCurrent);
		CheckOnuBiasCurrent(PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent );
		/*SFP_RSSI_DEBUG(8,("\nOnuEntry=%d\r\n",OnuEntry));*/
		
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO,"onu %d-%d rcepower=%ld(0x%x)\r\n",PonPortIdx, OnuIdx+1,TranslateOpticalPower(RecvOamMsg->OamBody.recvPower),RecvOamMsg->OamBody.recvPower));
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO," tx-power%ld\r\n", TranslateOpticalPower(RecvOamMsg->OamBody.sendPower)));
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO," temp=%ld\r\n", TranslateTemperature(RecvOamMsg->OamBody.temperature)));
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO," volt=%ld\r\n", TranslateWorkVoltage(RecvOamMsg->OamBody.workVoltage)));
		SFP_RSSI_DEBUG(1,(LOG_OPTICAL_POWER,LOG_INFO," bias=%ld\r\n", TranslateBiasCurrent(RecvOamMsg->OamBody.biasCurrent)));
	}
	return(ROK);
}

int PonPortTransPowerGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *tpowr )
{
	short int  slot, port;
	short int PonPortIdx, OnuIdx;
	
	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(tpowr== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{ /* Onu pon port  */
		/*added by yanjy.2017-03*/
		/*when tern off onu optical-power,mib can read value.29894*/
		if( V2R1_ENABLE != GetOnuOpticalPowerEnable())
			return RERROR;
		OnuIdx = OnuIdx-1;
		PonPortIdx = GetPonPortIdxBySlot(slot, port);
		CHECK_ONU_RANGE
		if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.onu_power_support_flag != 1)
			*tpowr = -1000;
		else
			*tpowr = GetOnuTransOpticalPower(PonPortIdx,OnuIdx);	
			
	}
	else
	{ /* get olt pon port  */
	
		if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
			return(RERROR);	
		
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport == 0)
			*tpowr = -1000;
		else
			*tpowr = GetPonPortTransOpticalPower(PonPortIdx );		
	}
	return( ROK);
}

int PonPortRecvPowerGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *powr )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(powr== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
	{ /* Onu pon port  */
		
		if( V2R1_ENABLE != GetOnuOpticalPowerEnable())
			return RERROR;
		
		OnuIdx = OnuIdx-1;
		PonPortIdx = GetPonPortIdxBySlot(slot, port);
		CHECK_ONU_RANGE
		if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.onu_power_support_flag != 1)
			*powr = -1000;
		else
			*powr = GetOnuRecvOpticalPower(PonPortIdx,OnuIdx);	
			
	}
	else
	{ /* get olt pon port  */
		if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
			return(RERROR);	
		
		OnuIdx = OnuIdx-1;/*added by wangying 问题单29661*/
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport == 0)
			*powr = -1000;
		else
			*powr = GetPonPortRecvOpticalPower(PonPortIdx, OnuIdx );		
	}
	return( ROK );
}

int PonPortTempGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *temp )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(temp== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
	{ /* Onu pon port  */
		if( V2R1_ENABLE != GetOnuOpticalPowerEnable())
			return RERROR;
		
		OnuIdx = OnuIdx-1;
		PonPortIdx = GetPonPortIdxBySlot(slot, port);
		CHECK_ONU_RANGE
		if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.onu_power_support_flag != 1)
			*temp = -1000;
		else	
			*temp = GetOnuTemperature(PonPortIdx,OnuIdx);	
	}
	else
	{ /* get olt pon port  */
		if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
			return(RERROR);	
		
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport == 0)
			*temp = -1000;
		else
			*temp = GetPonPortTemperature(PonPortIdx );			
	}
	return( ROK );
}

int PonPortVoltageGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *vol )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(vol== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
	{ /* Onu pon port  */
	
		if( V2R1_ENABLE != GetOnuOpticalPowerEnable())
			return (RERROR);
		
		OnuIdx = OnuIdx-1;
		PonPortIdx = GetPonPortIdxBySlot(slot, port);
		CHECK_ONU_RANGE
		if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.onu_power_support_flag != 1)
			*vol = -1000;
		else
			*vol = GetOnuWorkVoltage(PonPortIdx,OnuIdx);	
	}
	else
	{ /* get olt pon port  */
		if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
			return(RERROR);	
		
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport == 0)
			*vol = -1000;
		else
			*vol = GetPonPortWorkVoltage(PonPortIdx );
	}
	return( ROK );
}

int PonPortCurrentGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *cur )
{
	short int  slot, port, OnuIdx;
	short int PonPortIdx;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(cur== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
	{ 
		if( V2R1_ENABLE != GetOnuOpticalPowerEnable())
			return (RERROR);
		
		OnuIdx = OnuIdx-1;
		PonPortIdx = GetPonPortIdxBySlot(slot, port);
		CHECK_ONU_RANGE
		if(OnuMgmtTable[PonPortIdx*MAXONUPERPON+OnuIdx].ONUMeteringTable.onu_power_support_flag != 1)
			*cur = -1000;
		else
			*cur = GetOnuBiasCurrent(PonPortIdx,OnuIdx);
	}
	else
	{ /* get olt pon port  */
		if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
			return(RERROR);	
		PonPortIdx = GetPonPortIdxBySlot( brdIdx, portIdx );
		CHECK_PON_RANGE
		if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport == 0)
			*cur = -1000;
		else
			*cur = GetPonPortBiasCurrent(PonPortIdx );
	}
	return( ROK );
}

int UplinkPortBiasCurrentGet(ULONG brdIdx,ULONG  ethIdx,LONG *val)
{
	int max_link_port,sfp_sel;

	sfp_sel=ethIdx-1;
	max_link_port=GetMAXUplinkPort(brdIdx);
	if( sfp_sel<0 || sfp_sel >= max_link_port )
	{
		return VOS_ERROR;
	}
	if(UplinkPortMeteringInfo[brdIdx*MAXUPLINKPORT + sfp_sel].powerMeteringSupport == 0)
	{
		*val = -1000;
	}
	else
	{
		*val=GetUplinkPortBiasCurrent(brdIdx,sfp_sel);
	}

	return VOS_OK;
}

int UplinkSfpWorkVoltageGet(ULONG brdIdx,ULONG  ethIdx,LONG *val)
{
	int max_link_port,sfp_sel;

	sfp_sel=ethIdx-1;
	max_link_port=GetMAXUplinkPort(brdIdx);
	if( sfp_sel<0 || sfp_sel >= max_link_port )
	{
		return VOS_ERROR;
	}
	if(UplinkPortMeteringInfo[brdIdx*MAXUPLINKPORT + sfp_sel].powerMeteringSupport == 0)
	{
		*val = -1000;
	}
	else
	{
		*val=GetUplinkPortVoltage(brdIdx,sfp_sel);
	}

	return VOS_OK;
}

int UplinkSfpModuleTemperatureGet(ULONG brdIdx,ULONG  ethIdx,LONG *val)
{
	int max_link_port,sfp_sel;

	sfp_sel=ethIdx-1;
	max_link_port=GetMAXUplinkPort(brdIdx);
	if( sfp_sel<0 || sfp_sel >= max_link_port )
	{
		return VOS_ERROR;
	}
	if(UplinkPortMeteringInfo[brdIdx*MAXUPLINKPORT + sfp_sel].powerMeteringSupport == 0)
	{
		*val = -1000;
	}
	else
	{
		*val=*val=GetUplinkPortTemperature(brdIdx,sfp_sel);
	}

	return VOS_OK;
}

int UplinkSfpReceiverPowerGet(ULONG brdIdx,ULONG  ethIdx,LONG *val)
{
	int max_link_port,sfp_sel;

	sfp_sel=ethIdx-1;
	max_link_port=GetMAXUplinkPort(brdIdx);
	if( sfp_sel<0 || sfp_sel >= max_link_port )
	{
		return VOS_ERROR;
	}
	if(UplinkPortMeteringInfo[brdIdx*MAXUPLINKPORT + sfp_sel].powerMeteringSupport == 0)
	{
		*val = -1000;
	}
	else
	{
		*val=GetUplinkPortRecvOpticalPower(brdIdx,sfp_sel);
	}

	return VOS_OK;
}

int UplinkSfpTransmissionPowerGet(ULONG brdIdx,ULONG  ethIdx,LONG *val)
{
	int max_link_port,sfp_sel;

	sfp_sel=ethIdx-1;
	max_link_port=GetMAXUplinkPort(brdIdx);
	if( sfp_sel<0 || sfp_sel >= max_link_port )
	{
		return VOS_ERROR;
	}
	if(UplinkPortMeteringInfo[brdIdx*MAXUPLINKPORT + sfp_sel].powerMeteringSupport == 0)
	{
		*val = -1000;
	}
	else
	{
		*val=GetUplinkPortTransOpticalPower(brdIdx,sfp_sel);
	}

	return VOS_OK;
}

int getPortAlwaysOnEnable(ULONG devIdx, ULONG brdIdx, ULONG ponIdx, LONG *val)
{
	/*STATUS rc = VOS_OK;

	LOCATIONDES lct;

	if(getLocation(devIdx, &lct, CONV_YES) == VOS_OK)
	{
		rc = PonPortAlwaysOnEnGet(LOC2DevIdx(&lct), brdIdx, ponIdx, val);	
	}
	else 
		rc = VOS_ERROR;*/

	*val = onuLaser_alwaysOn_Enable;

	return VOS_OK;
}

int setPortAlwaysOnEnable(ULONG devIdx, ULONG brdIdx, ULONG ponIdx, LONG val)
{
	/*int i=0;*/

	/*LOCATIONDES lct;

	if(getLocation(devIdx, &lct, CONV_YES) == VOS_OK)
	{
		rc = PonPortAlwaysOnEnSet(LOC2DevIdx(&lct), brdIdx, ponIdx, val);	
	}
	else 
		rc = VOS_ERROR;*/
	if(GetPonPortOpticalMonitorEnable() == V2R1_ENABLE)
		Set_OnuLaserAlwaysOn_Enable(-1,val);
	
	return VOS_OK;
}

int PonPortSigMonEnGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *cur )
{
	short int  slot, port, OnuIdx;
	
	*cur = GetPonPortOpticalMonitorEnable();
	return( ROK );

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(cur== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{ /* Onu pon port  */
		return( RERROR);
		}
	else{ /* get olt pon port  */
		*cur = GetPonPortOpticalMonitorEnable();
		return( ROK );
		}

}

int PonPortSigMonEnSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val )
{
	short int  slot, port, OnuIdx;

	/* SetPonPortOpticalMonitorEnable(val );	*/ /*问题单: 13670 */
	Set_OpticalPower_Enable(-1, val,GetOnuOpticalPowerEnable(), 1);
	return( ROK );

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{ /* Onu pon port  */
		return( RERROR);
		}
	else{ /* get olt pon port  */
		SetPonPortOpticalMonitorEnable(val );	
		return( ROK );
		}

}

int PonPortSigMonIntervalGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *interval )
{
	short int  slot, port, OnuIdx;

	*interval = GetPonPortOpticalMonitorInterval();
	return( ROK );

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(interval== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{ /* Onu pon port  */
		return( RERROR);
		}
	else{ /* get olt pon port  */
		*interval = GetPonPortOpticalMonitorInterval();
		return( ROK );
		}

}

int PonPortSigMonIntervalSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val )
{
	short int  slot, port, OnuIdx;
	long lRet ;
	lRet = Set_OpticalPower_Interval( -1, val );
	/*SetPonPortOpticalMonitorInterval(val );*/
	return lRet;

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{ /* Onu pon port  */
		return( RERROR);
		}
	else{ /* get olt pon port  */
		SetPonPortOpticalMonitorInterval(val );	
		return( ROK );
		}
}

int PonPortAlwaysOnEnGet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG *en )
{
	short int  slot, port, OnuIdx;

	*en = GetPonPortOpticalAlwaysOnEnable();
	return( ROK );

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(en== NULL ) return( RERROR );
	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{
		return( RERROR);
		}
	else{ 
		*en = GetPonPortOpticalAlwaysOnEnable();
		return( ROK );
		}
}

int PonPortAlwaysOnEnSet( DeviceIndex_S DevIdx, ULONG brdIdx, ULONG  portIdx, LONG val )
{
	short int  slot, port, OnuIdx;

	SetPonPortOpticalAlwaysOnEnable(val );
	return( ROK );

	slot = DevIdx.slot;
	port = DevIdx.port;
	OnuIdx = DevIdx.onuId;

	if(!( (slot == 0 ) && ( port == 0 ) && ( OnuIdx == 1) ))
		{
		return( RERROR);
		}
	else{ 
		SetPonPortOpticalAlwaysOnEnable(val );
		return( ROK );
		}
}


/*  配置光功率检测使能及周期*/
DEFUN(config_optical_power_enable,
	config_optical_power_enable_cmd,
	"optical-power [enable|disable]",
	CONFIG_STR
	"enable\n"
	"disable\n"
	)
{
	/*if (VOS_StrCmp( argv[0], "enable")==0)
	{
		if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_enable,-1,NULL,0,0,0,0,0))
		{
			SetPonPortOpticalMonitorEnable(V2R1_ENABLE);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}
	}
	else
	{
		if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_disable,-1,NULL,0,0,0,0,0))
		{
			if(V2R1_DISABLE == onuLaser_alwaysOn_Enable)
				SetPonPortOpticalMonitorEnable(V2R1_DISABLE);
			else
				vty_out(vty," You should firstly disable the optical power always on!\r\n");
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}
	}*/
#if 0
	if( (VOS_StrCmp( argv[0], "enable")==0) && ( V2R1_ENABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) ))
	{
		vty_out(vty,"The optical power has been enabled !\r\n");
		return CMD_WARNING;
	}
	else if( (VOS_StrCmp( argv[0], "disable")==0) && ( V2R1_DISABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) ))
	{
		vty_out(vty,"The optical power has been disabled !\r\n");
		return CMD_WARNING;
	}
#endif
	if (VOS_StrCmp( argv[0], "enable")==0)
		Set_OpticalPower_Enable(-1, V2R1_ENABLE, GetOnuOpticalPowerEnable(), 0) ;
	else
		Set_OpticalPower_Enable(-1, V2R1_DISABLE, GetOnuOpticalPowerEnable(), 0) ;
		
	return(CMD_SUCCESS);
}

DEFUN(config_optical_power_onu_enable,
	config_optical_power_onu_enable_cmd,
	"optical-power onu [enable|disable]",
	CONFIG_STR
	"onu's config\n"
	"enable\n"
	"disable\n"
	)
{
#if 0
	if( V2R1_DISABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) )
	{
		if(VOS_StrCmp( argv[0], "enable")==0)
			vty_out(vty," Please enable the optical power for olt firstly !\r\n");
		else
			vty_out(vty," The optical power for olt doesn't enable !\r\n");
		
		return CMD_WARNING;
	}
	else
	{
		if( (GetOnuOpticalPowerEnable() == V2R1_ENABLE) && (VOS_StrCmp( argv[0], "enable")==0))
		{
			vty_out(vty," The optical power for onu has been enabled !\r\n");
			return CMD_WARNING;
		}
		else if( (GetOnuOpticalPowerEnable() == V2R1_DISABLE) && (VOS_StrCmp( argv[0], "disable")==0))
		{
			vty_out(vty," The optical power for onu has been disabled !\r\n");
			return CMD_WARNING;
		}
	}
#endif

	if (VOS_StrCmp( argv[0], "enable")==0)
		Set_OpticalPower_Enable(-1, GetPonPortOpticalMonitorEnable(), V2R1_ENABLE, 0) ;
	else
		Set_OpticalPower_Enable(-1, GetPonPortOpticalMonitorEnable(), V2R1_DISABLE, 0) ;
		
	return(CMD_SUCCESS);
}

int GetOnuLaserAlwaysOnEnable( )
{
	return onuLaser_alwaysOn_Enable;
}

int SetOnuLaserAlwaysOnEnable(int enable_mode)
{
	onuLaser_alwaysOn_Enable = enable_mode;
	return VOS_OK;
}

int SetOnuOpticalPowerEnable(int enable_mode)
{
	int PonPortIdx, OnuIdx , OnuEntry = 0;
#if 0
	if( V2R1_DISABLE == getOpticalPowerThreshold( field_olt_monitor_enable ) )
	{	
		onu_OpticalPower_Enable = V2R1_DISABLE;
		if( enable_mode == V2R1_DISABLE)
			return VOS_OK;
		else
			return VOS_ERROR;
	}
#endif
	onu_OpticalPower_Enable = enable_mode;

	if( enable_mode == V2R1_DISABLE )
	{
		if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		{
			for(PonPortIdx=0; PonPortIdx< MAXPON; PonPortIdx++ )
			{			
				for(OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx++)
				{/*遍历所有pon口下的所有ONU*/
					if(GetOnuOperStatus(PonPortIdx, OnuIdx ) == ONU_OPER_STATUS_UP)
						ClearOpticalPowerAlamWhenOnuDetectionDisable( PonPortIdx, OnuIdx );

					OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

					if(OnuEntry < MAXONU)
						OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag = 0;
				}
			}
		}
	}
	
	return VOS_OK;
}

int GetOnuOpticalPowerEnable( )
{
	return onu_OpticalPower_Enable;
}

DEFUN(config_optical_power_alwayson_enable,
	config_optical_power_alwayson_enable_cmd,
	"config onu-laser-always-on [enable|disable]",
	CONFIG_STR
	"onu laser always on detection\n"
	"enable\n"
	"disable\n"
	)
{
	if (VOS_StrCmp( argv[0], "enable")==0)
	{
		if(GetPonPortOpticalMonitorEnable() == V2R1_ENABLE)
		{
			/*if(VOS_OK == RPU_SendCmd2OpticalPower(onu_laser_always_on_enable,-1,NULL,0,0,0,0,0))
			{
				SetOnuLaserAlwaysOnEnable(V2R1_ENABLE) ;
			}
			else
			{
				vty_out(vty,"Send the command to Pon card failed !\r\n");
			}*/
			Set_OnuLaserAlwaysOn_Enable(-1,V2R1_ENABLE);
		}
		else
			vty_out(vty," You should firstly enable the optical power !\r\n");
	}
	else
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(onu_laser_always_on_disable,-1,NULL,0,0,0,0,0))
		{
			SetOnuLaserAlwaysOnEnable(V2R1_DISABLE);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OnuLaserAlwaysOn_Enable(-1,V2R1_DISABLE);
	}
	return(CMD_SUCCESS);
}

DEFUN(config_optical_power_interval,
	config_optical_power_interval_cmd,
	"optical-power interval <30-86400>",
	CONFIG_STR
	"optical power interval\n"
	"optical power interval, unit:second\n"
	)
{
	/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_interval,-1,NULL,0,VOS_AtoL(argv[0]),0,0,0))
	{
		SetPonPortOpticalMonitorInterval((int)VOS_AtoL(argv[0]));
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/
	Set_OpticalPower_Interval(-1,(int)VOS_AtoL(argv[0]));
	return(CMD_SUCCESS);
}

DEFUN(config_optical_power_interval_def,
	config_optical_power_interval_cmd_def,
	"optical-power interval default",
	CONFIG_STR
	"optical power interval\n"
	"default value: 60,  unit:second\n"
	)
{
	Set_OpticalPower_Interval(-1,POWER_METERING_INTERVAL_DEFAULT);
	return(CMD_SUCCESS);
}

DEFUN(config_optical_power_show,
	config_optical_power_show_cmd,
	"show optical-power config",
	SHOW_STR
	"show optical-power config\n"
	"show optical-power config\n"
	)
{
	if(GetPonPortOpticalMonitorEnable() == V2R1_ENABLE)
		vty_out(vty," optical power for olt is enable\r\n");
	else
		vty_out(vty," optical power for olt is disable\r\n");

	if( GetOnuOpticalPowerEnable() == V2R1_ENABLE)
		vty_out(vty," optical power for onu is enable\r\n");
	else
		vty_out(vty," optical power for onu is disable\r\n");
	
	vty_out(vty," optical power interval is %d(s)\r\n", GetPonPortOpticalMonitorInterval());
	vty_out(vty," onu laser always on is %s\r\n", (V2R1_ENABLE == onuLaser_alwaysOn_Enable) ? "enable" : "disable" );
	
	return(CMD_SUCCESS);
}
/*配置上联口LOS 信号的检测周期*/
DEFUN(config_uplink_loscheck_interval,
	config_uplink_loscheck_interval_cmd,
	"uplink-los-check interval <1-60>",
	CONFIG_STR
	"uplink los check interval\n"
	"uplink los check interval, unit:second\n"
	)
{
	SetUplinkLosCheckInterval((int)VOS_AtoL(argv[0]));
	
	return(CMD_SUCCESS);
}

DEFUN(config_uplink_loscheck_show,
	config_uplink_loscheck_show_cmd,
	"show uplink-los-check interval",
	SHOW_STR
	"show uplink los check interval\n"
	)
{
		vty_out(vty," optical uplink los check interval is %d(s)\r\n", GetUplinkLosCheckInterval());

	return(CMD_SUCCESS);
}

/* 配置告警门限deadZone ,  OLT 侧和ONU 侧通用*/
DEFUN(config_power_alarm_thrld_deadzone,
	config_power_alarm_thrld_deadzone_cmd,
	"optical-power threshold-deadzone <power> <tempertaure> <voltage> <current>",
	CONFIG_STR
	"alarm threshold deadzone(must be greater than 0)\n"
    	"power deadzone(unit:0.1dbm)\n"
    	"temperature deadzone(unit:C)\n"
    	"voltage deadzone(unit:0.1V)\n"
    	"current deadzone(unit:1mA)\n"
	)
{
	int power_deadzone, temperature_deadzone,supply_voltage_deadzone,bias_current_zone;
	long lRet = VOS_ERROR;
#if 0
	power_deadzone = (int)VOS_AtoL(argv[0]);
	temperature_deadzone = (int)VOS_AtoL(argv[1]);
	supply_voltage_deadzone = (int)VOS_AtoL(argv[2]);
	bias_current_zone = (int)VOS_AtoL(argv[3]);
#else
	lRet = VOS_AtoL_2(argv[0], &power_deadzone);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[1], &temperature_deadzone);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &supply_voltage_deadzone);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[3], &bias_current_zone);
	CHECK_LRET3( lRet );
#endif
/*added by yanjy,2017-3*/
/*deadzone必须大于零35649*/
	if((power_deadzone < 0) || (temperature_deadzone < 0) || (supply_voltage_deadzone < 0) || (bias_current_zone < 0) )
	{
		vty_out(vty, " %% Parameter err\r\n");
		return( CMD_WARNING );
	}

	if(((GetPonPortTransOpticalPowerLowThrd(0)+ power_deadzone) >= GetPonPortTransOpticalPowerHighThrd(0))
		||((GetPonPortTransOpticalPowerLowThrd(1)+ power_deadzone) >= GetPonPortTransOpticalPowerHighThrd(1))
		|| ((GetPonPortRecvOpticalPowerLowThrd(0)+ power_deadzone) >= GetPonPortRecvOpticalPowerHighThrd(0)) 
		|| ((GetPonPortRecvOpticalPowerLowThrd(1)+ power_deadzone) >= GetPonPortRecvOpticalPowerHighThrd(1))
		||((GetOnuTransOpticalPowerLowThrd()+ power_deadzone) >= GetOnuTransOpticalPowerHighThrd())
		|| ((GetOnuRecvOpticalPowerLowThrd()+ power_deadzone) >= GetOnuRecvOpticalPowerHighThrd()) ) 
	{
		vty_out(vty,"\r\n tx-optical-power alarm threshold: DeadZone should be less than high threshold -low threshold\r\n");
		return(CMD_WARNING);
	}

	if(((GetPonPortTemperatureLowThrd(0)+ temperature_deadzone) >= GetPonPortTemperatureHighThrd(0))
		||((GetPonPortTemperatureLowThrd(1)+ temperature_deadzone) >= GetPonPortTemperatureHighThrd(1))
		||((GetOnuTemperatureLowThrd()+ temperature_deadzone) >= GetOnuTemperatureHighThrd()) )
	{
		vty_out(vty,"\r\n SFP temperature alarm threshold: DeadZone should be less than high threshold -low threshold\r\n");
		return(CMD_WARNING);
	}

	if(((GetPonPortWorkVoltageLowThrd(0)+ supply_voltage_deadzone) >= GetPonPortWorkVoltageHighThrd(0))
		||((GetPonPortWorkVoltageLowThrd(1)+ supply_voltage_deadzone) >= GetPonPortWorkVoltageHighThrd(1))
		||((GetOnuWorkVoltageLowThrd()+ supply_voltage_deadzone) >= GetOnuWorkVoltageHighThrd()) )
	{
		vty_out(vty,"\r\n SFP supply voltage alarm threshold: DeadZone should be less than high threshold -low threshold\r\n");
		return(CMD_WARNING);
	}
 
	if(((GetPonPortBiasCurrentLowThrd(0)+ bias_current_zone) >= GetPonPortBiasCurrentHighThrd(0))
		||((GetPonPortBiasCurrentLowThrd(1)+ bias_current_zone) >= GetPonPortBiasCurrentHighThrd(1))
		||((GetOnuBiasCurrentLowThrd()+ bias_current_zone) >= GetOnuBiasCurrentHighThrd()) )
	{
		vty_out(vty,"\r\n SFP bias current alarm threshold: DeadZone should be less than high threshold -low threshold\r\n");
		return(CMD_WARNING);
	}
	 
	/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_threshold_deadzone,-1,NULL,0,
		power_deadzone,temperature_deadzone,supply_voltage_deadzone,bias_current_zone))
	{
		setOpticalPowerDeadZone(field_power_dead_zone,power_deadzone);
		setOpticalPowerDeadZone(field_tempe_dead_zone,temperature_deadzone);
		setOpticalPowerDeadZone(field_vol_dead_zone,supply_voltage_deadzone);
		setOpticalPowerDeadZone(field_cur_dead_zone,bias_current_zone);
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/

	Set_OpticalPower_Deadzone(-1, power_deadzone, temperature_deadzone, supply_voltage_deadzone, bias_current_zone);

	return(CMD_SUCCESS);	
}
/*显示告警门限deadzone*/
DEFUN(config_power_alarm_thrld_deadzone_show,
	config_power_alarm_thrld_deadzone_show_cmd,
	"show optical-power threshold-deadzone",
	SHOW_STR
	"optical power\n"
	"optical power alarm threshold deadzone\n"
	)
{
	LONG val;
	val = getOpticalPowerDeadZone(field_power_dead_zone);
	vty_out(vty," power alarm threshold deadzone:%d.%d dbm\r\n", decimal2_integer_part(val), decimal2_fraction_part(val) );
	val = getOpticalPowerDeadZone(field_tempe_dead_zone);
	vty_out(vty," temperature alarm threshold deadzone:%d C\r\n", val );
	val = getOpticalPowerDeadZone(field_vol_dead_zone);
	vty_out(vty," supply voltage alarm threshold deadzone:%d.%d V\r\n", decimal2_integer_part(val), decimal2_fraction_part(val) );
	val = getOpticalPowerDeadZone(field_cur_dead_zone);
	vty_out(vty," bias current alarm threshold deadzone:%d mA\r\n", val );

	return(CMD_SUCCESS);	
}

/*配置功率校正值*/
DEFUN(config_optical_power_calibration,
	config_optical_power_calibration_cmd,
	"optical-power calibration [olt|onu] <rxCali> <txCali>",
	"optical-power\n"
	"optical power calibration\n"
	"OLT optical-power calibration\n"
	"ONU optical-power calibration\n"
	"input rx-power-calibration, unit:0.1dbm\n"
	"input tx-power-calibration, unit:0.1dbm\n"
	)
{
	long rx, tx;
	long lRet = VOS_OK;
#if 0
	rx = VOS_AtoL( argv[1] );
	tx = VOS_AtoL( argv[2] );
#else
	lRet = VOS_AtoL_2(argv[1], &rx);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &tx);
	CHECK_LRET3( lRet );
#endif

	if( argv[0][1] == 'l' )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_calibration_olt,-1,NULL, 0,rx, tx,0,0))
		{
			setOltOpticalPowerCalibration( rx, tx );
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Calibration_Olt(-1,rx,tx);
	}
	else
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_calibration_onu,-1,NULL,0,rx,tx,0,0))
		{
			setOnuOpticalPowerCalibration( rx, tx );
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Calibration_Onu(-1,rx,tx);
	}	
		
	return(CMD_SUCCESS);
}
DEFUN(show_optical_power_calibration,
	show_optical_power_calibration_cmd,
	"show optical-power calibration",
	SHOW_STR
	"optical power\n"
	"optical-power calibration\n"
	)
{
	vty_out( vty, "OLT optical-power calibration:rx=%d.%d dbm,tx=%d.%d dbm\r\n", 
			decimal2_integer_part(olt_rx_optical_power_calibration), decimal2_fraction_part(olt_rx_optical_power_calibration), 
			decimal2_integer_part(olt_tx_optical_power_calibration), decimal2_fraction_part(olt_tx_optical_power_calibration) );
	vty_out( vty, "ONU optical-power calibration:rx=%d.%d dbm,tx=%d.%d dbm\r\n", 
			decimal2_integer_part(onu_rx_optical_power_calibration), decimal2_fraction_part(onu_rx_optical_power_calibration), 
			decimal2_integer_part(onu_tx_optical_power_calibration), decimal2_fraction_part(onu_tx_optical_power_calibration) );
	return(CMD_SUCCESS);
}

/* 配置OLT|ONU侧告警门限*/
DEFUN  (
    optical_power_threshold_config,
    optical_power_threshold_config_cmd,
    "optical-power alarm-threshold [olt|olt-10GE|onu|uplink|uplink-10GE] <Tx_high> <Tx_low> <Rx_high> <Rx_low>",
    "optical power\n"
    "optical power alarm threshold\n"
    "olt\n"
    "olt 10GE\n"
    "onu\n"
    "uplink\n"
    "uplink 10GE\n"
    "tx power high(unit:0.1dbm)\n"
    "tx power low(unit:0.1dbm)\n"
    "rx power high(unit:0.1dbm)\n"
    "rx power low(unit:0.1dbm)\n" )
{
	long  tx_power_low,tx_power_high,rx_power_low,rx_power_high;
	long lRet = VOS_ERROR;
#if 0
	tx_power_high = VOS_AtoL(argv[1]);
	tx_power_low = VOS_AtoL(argv[2]);
	rx_power_high = VOS_AtoL(argv[3]);
	rx_power_low = VOS_AtoL(argv[4]);
#else
	lRet = VOS_AtoL_2(argv[1], &tx_power_high);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &tx_power_low);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[3], &rx_power_high);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[4], &rx_power_low);
	CHECK_LRET3( lRet );
#endif

	if((tx_power_low + getOpticalPowerDeadZone(field_power_dead_zone))>= tx_power_high)
	{
		vty_out(vty,"\r\n tx-optical-power alarm threshold: high threshold be greater than low threshold + DeadZone\r\n");
		return(CMD_WARNING);
	}
	if((rx_power_low + getOpticalPowerDeadZone(field_power_dead_zone))>= rx_power_high)
	{
		vty_out(vty,"\r\n rx-optical-power alarm threshold: high threshold be greater than low threshold + DeadZone\r\n");
		return(CMD_WARNING);
	}

	if( VOS_StrCmp(argv[0], "olt") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_alarm_threshold_olt,-1,NULL,0,tx_power_low,tx_power_high,rx_power_low,rx_power_high))
		{
			SetPonPortTransOpticalPowerLowThrd(tx_power_low);
			SetPonPortTransOpticalPowerHighThrd(tx_power_high);
			
			SetPonPortRecvOpticalPowerLowThrd(rx_power_low);
			SetPonPortRecvOpticalPowerHighThrd(rx_power_high);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_alarm_threshold_olt(-1,tx_power_low,tx_power_high,rx_power_low,rx_power_high,0);
	}
	else if( VOS_StrCmp(argv[0], "olt-10ge") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_alarm_threshold_olt,-1,NULL,0,tx_power_low,tx_power_high,rx_power_low,rx_power_high))
		{
			SetPonPortTransOpticalPowerLowThrd(tx_power_low);
			SetPonPortTransOpticalPowerHighThrd(tx_power_high);
			
			SetPonPortRecvOpticalPowerLowThrd(rx_power_low);
			SetPonPortRecvOpticalPowerHighThrd(rx_power_high);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_alarm_threshold_olt(-1,tx_power_low,tx_power_high,rx_power_low,rx_power_high,1);
	}
	else if( VOS_StrCmp(argv[0], "onu") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_alarm_threshold_onu,-1,NULL,0,tx_power_low,tx_power_high,rx_power_low,rx_power_high))
		{
			SetOnuTransOpticalPowerLowThrd(tx_power_low);
			SetOnuTransOpticalPowerHighThrd(tx_power_high);
			
			SetOnuRecvOpticalPowerLowThrd(rx_power_low);
			SetOnuRecvOpticalPowerHighThrd(rx_power_high);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_alarm_threshold_onu(-1,tx_power_low,tx_power_high,rx_power_low,rx_power_high);
	}
	else if( VOS_StrCmp(argv[0], "uplink") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_power_alarm_threshold_uplink,NULL,tx_power_low,tx_power_high,rx_power_low,rx_power_high))
		{
			SetUplinkPortTransOpticalPowerLowthrd(tx_power_low, 0 );
			SetUplinkPortTransOpticalPowerHighthrd(tx_power_high, 0 );
			
			SetUplinkPortRecvOpticalPowerLowthrd(rx_power_low, 0 );
			SetUplinkPortRecvOpticalPowerHighthrd(rx_power_high, 0 );
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_alarm_Threshold_Uplink(tx_power_low,tx_power_high,rx_power_low,rx_power_high,0);
	}
	else if( VOS_StrCmp(argv[0], "uplink-10ge") == 0 )
	{
	/*
		SetUplinkPortTransOpticalPowerLowthrd(tx_power_low, 1 );
		SetUplinkPortTransOpticalPowerHighthrd(tx_power_high, 1 );
			
		SetUplinkPortRecvOpticalPowerLowthrd(rx_power_low, 1 );
		SetUplinkPortRecvOpticalPowerHighthrd(rx_power_high, 1 );
	*/
		Set_OpticalPower_alarm_Threshold_Uplink(tx_power_low,tx_power_high,rx_power_low,rx_power_high,1);
	}
	
	return VOS_OK;
}

DEFUN  (
    optical_temperature_threshold_config,
    optical_temperature_threshold_config_cmd,
    "optical-temperature alarm-threshold [olt|olt-10GE|onu|uplink|uplink-10GE] <high> <low>",
    "optical temperature\n"
    "SFP temperature alarm threshold\n"
    "olt\n"
    "olt 10GE\n"
    "onu\n"
    "uplink\n"
    "uplink 10GE\n"
    "SFP temperature high(unit:C)\n"
    "SFP temperature low(unit:C)\n"
    )
{

	long  high_val,low_val;
	long lRet= VOS_ERROR;
#if 0
	high_val = VOS_AtoL(argv[1]);
	low_val = VOS_AtoL(argv[2]);
#else
	lRet = VOS_AtoL_2(argv[1], &high_val);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &low_val);
	CHECK_LRET3( lRet );
#endif

	if((low_val + getOpticalPowerDeadZone(field_tempe_dead_zone))>= high_val)
	{
		vty_out(vty,"\r\n SFP temperature alarm threshold: high threshold be greater than low threshold + DeadZone\r\n");
		return(CMD_WARNING);
	}

	if( VOS_StrCmp(argv[0], "olt") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_temperature_alarm_threshold_olt,-1,NULL,0,low_val,high_val,0,0))
		{
			SetPonPortTemperatureLowThrd(low_val);
			SetPonPortTemperatureHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Temperature_Threshold_Olt(-1,low_val,high_val,0);
	}
	else if( VOS_StrCmp(argv[0], "olt-10ge") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_temperature_alarm_threshold_olt,-1,NULL,0,low_val,high_val,0,0))
		{
			SetPonPortTemperatureLowThrd(low_val);
			SetPonPortTemperatureHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Temperature_Threshold_Olt(-1,low_val,high_val,1);
	}
	else if(VOS_StrCmp(argv[0],"onu")==0)
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_temperature_alarm_threshold_onu,-1,NULL,0,low_val,high_val,0,0))
		{
			SetOnuTemperatureLowThrd(low_val);
			SetOnuTemperatureHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Temperature_Threshold_Onu(-1,low_val,high_val);
	}
	else if(VOS_StrCmp(argv[0],"uplink")==0)
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_temperature_alarm_threshold_uplink,NULL,low_val,high_val,0,0))
		{*/
			Set_OpticalPower_Temperature_Threshold_Uplink(low_val,high_val,0);
		/*}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
	}
	else if(VOS_StrCmp(argv[0],"uplink-10ge")==0)
	{
		Set_OpticalPower_Temperature_Threshold_Uplink(low_val,high_val,1);
	}
	return VOS_OK;
}

DEFUN  (
    optical_supply_voltage_config,
    optical_supply_voltage_config_cmd,
    "optical-voltage alarm-threshold [olt|olt-10GE|onu|uplink|uplink-10GE] <high> <low>",
    "optical voltage\n"
    "SFP optical voltage\n"
    "olt\n"
    "olt 10GE\n"
    "onu\n"
    "uplink\n"
    "uplink 10GE\n"
    "SFP supply voltage high(unit:0.1V)\n"
    "SFP supply voltage low(unit:0.1V)\n"
    )
{

	long  high_val,low_val;
	long lRet = VOS_ERROR;
#if 0
	high_val = VOS_AtoL(argv[1]);
	low_val = VOS_AtoL(argv[2]);
#else
	lRet = VOS_AtoL_2(argv[1], &high_val);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &low_val);
	CHECK_LRET3( lRet );
#endif

	if((low_val + getOpticalPowerDeadZone(field_vol_dead_zone))>= high_val)
	{
		vty_out(vty,"\r\n SFP supply voltage alarm threshold: high threshold be greater than low threshold + DeadZone\r\n");
		return(CMD_WARNING);
	}

	if( VOS_StrCmp(argv[0], "olt") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_voltage_alarm_threshold_olt,-1,NULL,0,low_val,high_val,0,0))
		{
			SetPonPortWorkVoltageLowThrd(low_val);
			SetPonPortWorkVoltageHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Voltage_Threshold_Olt(-1,low_val,high_val,0);
	}
	else if( VOS_StrCmp(argv[0], "olt-10ge") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_voltage_alarm_threshold_olt,-1,NULL,0,low_val,high_val,0,0))
		{
			SetPonPortWorkVoltageLowThrd(low_val);
			SetPonPortWorkVoltageHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Voltage_Threshold_Olt(-1,low_val,high_val,1);
	}
	else if( VOS_StrCmp(argv[0], "onu") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_voltage_alarm_threshold_onu,-1,NULL,0,low_val,high_val,0,0))
		{
			SetOnuWorkVoltageLowThrd(low_val);
			SetOnuWorkVoltageHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_Voltage_Threshold_Onu(-1,low_val,high_val);
	}
	else if( VOS_StrCmp(argv[0], "uplink") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_voltage_alarm_threshold_uplink,NULL,low_val,high_val,0,0))
		{*/
			Set_OpticalPower_Voltage_Threshold_Uplink(low_val,high_val,0);
		/*}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
	}
	else if( VOS_StrCmp(argv[0], "uplink-10ge") == 0 )
	{
		Set_OpticalPower_Voltage_Threshold_Uplink(low_val,high_val,1);

	}
	return VOS_OK;
}

DEFUN  (
    optical_bias_current_config,
    optical_bias_current_config_cmd,
    "optical-bias-current alarm-threshold [olt|olt-10GE|onu|uplink|uplink-10GE] <high> <low>",
    "optical current\n"
    "SFP optical current\n"
    "olt\n"
    "olt 10GE\n"
    "onu\n"
    "uplink\n"
    "uplink-10GE\n"
    "SFP bias-current high(unit:mA)\n"
    "SFP bias-current low(unit:mA)\n"
    )
{
	long  high_val,low_val;
	long lRet = VOS_ERROR;
#if 0
	high_val = VOS_AtoL(argv[1]);
	low_val = VOS_AtoL(argv[2]);
#else
	lRet = VOS_AtoL_2(argv[1], &high_val);
	CHECK_LRET3( lRet );
	lRet = VOS_AtoL_2(argv[2], &low_val);
	CHECK_LRET3( lRet );
#endif

	if((low_val + getOpticalPowerDeadZone(field_cur_dead_zone))>= high_val)
	{
		vty_out(vty,"\r\n SFP current alarm threshold: high threshold be greater than low threshold + DeadZone\r\n");
		return(CMD_WARNING);
	}

	if( VOS_StrCmp(argv[0], "olt") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_bias_current_alarm_threshold_olt,-1,NULL,0,low_val,high_val,0,0))
		{
			SetPonPortBiasCurrentLowThrd(low_val);
			SetPonPortBiasCurrentHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_BiasCurrent_Threshold_Olt(-1,low_val,high_val,0);
	}
	else if( VOS_StrCmp(argv[0], "olt-10ge") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_bias_current_alarm_threshold_onu,-1,NULL,0,low_val,high_val,0,0))
		{
			SetOnuBiasCurrentLowThrd(low_val);
			SetOnuBiasCurrentHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_BiasCurrent_Threshold_Olt(-1,low_val,high_val,1);
	}
	else if( VOS_StrCmp(argv[0], "onu") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_bias_current_alarm_threshold_onu,-1,NULL,0,low_val,high_val,0,0))
		{
			SetOnuBiasCurrentLowThrd(low_val);
			SetOnuBiasCurrentHighThrd(high_val);
		}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
		Set_OpticalPower_BiasCurrent_Threshold_Onu(-1,low_val,high_val);
	}
	else if( VOS_StrCmp(argv[0], "uplink") == 0 )
	{
		/*if(VOS_OK == RPU_SendCmd2OpticalPower(optical_bias_current_alarm_threshold_uplink,NULL,low_val,high_val,0,0))
		{*/
			Set_OpticalPower_BiasCurrent_Threshold_Uplink(low_val,high_val,0);
		/*}
		else
		{
			vty_out(vty,"Send the command to Pon card failed !\r\n");
		}*/
	}
	else if( VOS_StrCmp(argv[0], "uplink-10ge") == 0 )
	{
		Set_OpticalPower_BiasCurrent_Threshold_Uplink(low_val,high_val,1);
	}
	return CMD_SUCCESS;
}

/*显示OLT侧和ONU侧告警门限*/
DEFUN(optical_power_alarm_thrld_show,
	optical_power_alarm_thrld_show_cmd,
	"show optical-power alarm-threshold [olt|olt-10GE|onu|uplink|upLink-10GE]",
	SHOW_STR
	"optical power\n"
	"power metering alarm threshold\n"
	"olt\n"
	"olt 10GE\n"
	"onu\n"
	"uplink\n"
	"uplink 10GE\n"
	)
{
	int tx_power_low, tx_power_high;
	int rx_power_low, rx_power_high;
	int temperature_low, temperature_high;
	int voltage_low, voltage_high;
	int bias_current_low, bias_current_high;

	if( VOS_StrCmp(argv[0], "olt") == 0 )
	{
		tx_power_low = GetPonPortTransOpticalPowerLowThrd(0);
		tx_power_high = GetPonPortTransOpticalPowerHighThrd(0);

		rx_power_low = GetPonPortRecvOpticalPowerLowThrd(0);
		rx_power_high = GetPonPortRecvOpticalPowerHighThrd(0);

		temperature_low = GetPonPortTemperatureLowThrd(0);
		temperature_high = GetPonPortTemperatureHighThrd(0);

		voltage_low = GetPonPortWorkVoltageLowThrd(0);
		voltage_high = GetPonPortWorkVoltageHighThrd(0);

		bias_current_low = GetPonPortBiasCurrentLowThrd(0);
		bias_current_high = GetPonPortBiasCurrentHighThrd(0);
	}
	else if(VOS_StrCmp(argv[0], "olt-10ge") == 0)
	{
		tx_power_low = GetPonPortTransOpticalPowerLowThrd(1);
		tx_power_high = GetPonPortTransOpticalPowerHighThrd(1);

		rx_power_low = GetPonPortRecvOpticalPowerLowThrd(1);
		rx_power_high = GetPonPortRecvOpticalPowerHighThrd(1);

		temperature_low = GetPonPortTemperatureLowThrd(1);
		temperature_high = GetPonPortTemperatureHighThrd(1);

		voltage_low = GetPonPortWorkVoltageLowThrd(1);
		voltage_high = GetPonPortWorkVoltageHighThrd(1);

		bias_current_low = GetPonPortBiasCurrentLowThrd(1);
		bias_current_high = GetPonPortBiasCurrentHighThrd(1);
	}
	else if( VOS_StrCmp(argv[0], "onu") == 0 )
	{
		tx_power_low = GetOnuTransOpticalPowerLowThrd();
		tx_power_high = GetOnuTransOpticalPowerHighThrd();

		rx_power_low = GetOnuRecvOpticalPowerLowThrd();
		rx_power_high = GetOnuRecvOpticalPowerHighThrd();

		temperature_low = GetOnuTemperatureLowThrd();
		temperature_high = GetOnuTemperatureHighThrd();

		voltage_low = GetOnuWorkVoltageLowThrd();
		voltage_high = GetOnuWorkVoltageHighThrd();

		bias_current_low = GetOnuBiasCurrentLowThrd();
		bias_current_high = GetOnuBiasCurrentHighThrd();
	}
	else if( VOS_StrCmp(argv[0], "uplink") == 0 )
	{
		tx_power_low = GetUplinkPortTransOpticalPowerLowthrd(0);
		tx_power_high = GetUplinkPortTransOpticalPowerHighthrd(0);

		rx_power_low = GetUplinkPortRecvOpticalPowerLowthrd(0);
		rx_power_high = GetUplinkPortRecvOpticalPowerHighthrd(0);
		
		temperature_low = GetUplinkPortTemperatureLowthrd(0);
		temperature_high = GetUplinkPortTemperatureHighthrd(0);

		voltage_low = GetUplinkPortVoltageLowthrd(0);
		voltage_high = GetUplinkPortVoltageHighthrd(0);

		bias_current_low = GetUplinkPortBiasCurrentLowthrd(0);
		bias_current_high = GetUplinkPortBiasCurrentHighthrd(0);
	}
	else if( VOS_StrCmp(argv[0], "uplink-10ge") == 0 )
	{
		tx_power_low = GetUplinkPortTransOpticalPowerLowthrd(1);
		tx_power_high = GetUplinkPortTransOpticalPowerHighthrd(1);

		rx_power_low = GetUplinkPortRecvOpticalPowerLowthrd(1);
		rx_power_high = GetUplinkPortRecvOpticalPowerHighthrd(1);
		
		temperature_low = GetUplinkPortTemperatureLowthrd(1);
		temperature_high = GetUplinkPortTemperatureHighthrd(1);

		voltage_low = GetUplinkPortVoltageLowthrd(1);
		voltage_high = GetUplinkPortVoltageHighthrd(1);

		bias_current_low = GetUplinkPortBiasCurrentLowthrd(1);
		bias_current_high = GetUplinkPortBiasCurrentHighthrd(1);
	}

	vty_out(vty,"\r\n tx power alarm low:%d.%d dbm\r\n", decimal2_integer_part(tx_power_low), decimal2_fraction_part(tx_power_low) );
	vty_out(vty," tx power alarm high:%d.%d dbm\r\n", decimal2_integer_part(tx_power_high), decimal2_fraction_part(tx_power_high) );

	vty_out(vty," rx power alarm low:%d.%d dbm\r\n", decimal2_integer_part(rx_power_low), decimal2_fraction_part(rx_power_low) );
	vty_out(vty," rx power alarm high:%d.%d dbm\r\n", decimal2_integer_part(rx_power_high), decimal2_fraction_part(rx_power_high) );

	vty_out(vty," temperature alarm low:%d C\r\n", temperature_low);
	vty_out(vty," temperature alarm high:%d C\r\n", temperature_high);
	
	vty_out(vty," supply voltage alarm low:%d.%d V\r\n", decimal2_integer_part(voltage_low), decimal2_fraction_part(voltage_low) );
	vty_out(vty," supply voltage alarm high:%d.%d V\r\n", decimal2_integer_part(voltage_high), decimal2_fraction_part(voltage_high));

	vty_out(vty," bias-current alarm low:%d mA\r\n", bias_current_low);
	vty_out(vty," bias-current alarm high:%d mA\r\n", bias_current_high);
	
	return(CMD_SUCCESS);
}

#if 0
ULONG showUplinkSFPTemperature(short int sfp_sel,short slotno,struct vty *vty)
{
	int ret,max_link_port;
	max_link_port=GetMAXUplinkPort(slotno);
	if( (sfp_sel < 0) || (sfp_sel >= max_link_port) )
	{
		vty_out( vty, "\r\n SFPId=%d out of rang\r\n",sfp_sel );
		return CMD_WARNING;
	}
	if((ret=GetUplinkPortTemperature(slotno,sfp_sel))==ERROR)
	{
		vty_out(vty,"get temperature info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	vty_out(vty,"  %s: %d Celsius\r\n",XcvrDiagArr[34].pcName, ret);
	return CMD_SUCCESS;
}
ULONG showUplinkSFPVoltage(short int sfp_sel,short slotno,struct vty *vty)
{
	int ret,max_link_port;
	max_link_port=GetMAXUplinkPort(slotno);
	if(sfp_sel<0||sfp_sel>max_link_port-1)
	{
		sys_console_printf("\r\n SFPId=%d out of rang\r\n",sfp_sel);
		return CMD_WARNING;
	}
	if((ret=GetUplinkPortVoltage(slotno, sfp_sel))==ERROR)
		{
		vty_out(vty,"get voltage info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
		}
	vty_out(vty, "  %s: %d.%d V\r\n", XcvrDiagArr[35].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}
ULONG showUplinkSFPBiasCurrent(short int sfp_sel,short slotno,struct vty *vty)
{
	int ret,max_link_port;
	max_link_port=GetMAXUplinkPort(slotno);
	if(sfp_sel<0||sfp_sel>max_link_port-1)
	{
		sys_console_printf("\r\n SFPId=%d out of rang\r\n",sfp_sel);
		return CMD_WARNING;
	}
	if((ret=GetUplinkPortBiasCurrent(slotno, sfp_sel))==ERROR)
		{
		vty_out(vty,"get bias-current info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
		}
	vty_out(vty, "  %s: %d(mA)\r\n", XcvrDiagArr[36].pcName, ret );
	return CMD_SUCCESS;
}
ULONG showUplinkSFPTransOpticalPower(short int sfp_sel,short slotno,struct vty *vty)
{
	int ret,max_link_port;
	max_link_port=GetMAXUplinkPort(slotno);
	if(sfp_sel<0||sfp_sel>max_link_port-1)
	{
		sys_console_printf("\r\n SFPId=%d out of rang\r\n",sfp_sel);
		return CMD_WARNING;
	}
	if((ret=GetUplinkPortTransOpticalPower(slotno, sfp_sel))==ERROR)
		{
		vty_out(vty,"get transpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
		}
	vty_out( vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[37].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}
ULONG showUplinkSFPRecvOpticalPower(short int sfp_sel,short slotno,struct vty *vty)
{
	int ret,max_link_port;
	max_link_port=GetMAXUplinkPort(slotno);
	if(sfp_sel<0||sfp_sel>max_link_port-1)
	{
		sys_console_printf("\r\n SFPId=%d out of rang\r\n",sfp_sel);
		return CMD_WARNING;
	}
	if((ret=GetUplinkPortRecvOpticalPower(slotno, sfp_sel))==ERROR)
		{
		vty_out(vty,"get recvpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
		}
	if( ret != uplink_rx_optical_power_default )
		vty_out( vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[38].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	else
		vty_out( vty, "  %s: --\r\n", XcvrDiagArr[38].pcName );
	return CMD_SUCCESS;
}
#endif

extern int SlotCardIsUplinkBoard(int CardIndex);
int show_uplink_sfp_transceiver_meter( struct vty *vty, ULONG ulSlot, ULONG ulPort )
{
	int sfp_sel,max_link_port;
	int portno;
	int ret, flag = 0;
	UCHAR value,a,b;
	int length;
	UplinkPortMeteringInfo_S *pQueueMsg;
	int PonPortIdx = GetPonPortIdxBySlot( ulSlot, ulPort );
	
	sfp_sel=ulPort;
	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		sfp_sel=sfp_sel-17;
	}
	else
	{
		sfp_sel=sfp_sel-1;
	}
	max_link_port=GetMAXUplinkPort(ulSlot);
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 板在位检查*/
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	if( !(SlotCardIsUplinkBoard(ulSlot) == ROK 
		|| SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON
		|| SYS_MODULE_IS_UPLINK_PON(ulSlot)) )	/*板类型检查*/
	{
		vty_out(vty," %% slot %d is not uplink board\r\n", ulSlot);
		return( CMD_WARNING );
	}

	if( !SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON )
	{
		if( (sfp_sel < 0) ||(sfp_sel > max_link_port-1) )
		{
			vty_out( vty, "\r\n SFPId=%d out of rang \r\n",sfp_sel+1);
			return CMD_WARNING;
		}
	}

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		//ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &value);
			if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			{
				ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &value);
			}
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{
				ReadCPLDReg(GFA8100_UPLINK_SFP_GPON_STATE, &value);
				a = value & 0x03;//取bit0、1
				b = value & 0x0C;//取bit2、3
				
				value = (a<<2) | (b>>2);
			}
		if(sfp_sel < 2)
		{
			if(value & 1 << (sfp_sel+2))
			{
				vty_out(vty,"uplink %d/%d sfp is not online .\r\n",ulSlot,sfp_sel+17 );
				return CMD_WARNING;
			}
		}
		else
		{
			if(value & 1 << (sfp_sel-2))
			{
				vty_out(vty,"uplink %d/%d sfp is not online .\r\n",ulSlot,sfp_sel+17 );
				return CMD_WARNING;
			}
		}

	      portno = ulSlot*MAXUPLINKPORT +sfp_sel;
		if( UplinkPortMeteringInfo[portno].powerMeteringSupport == NOT_SUPPORT_SFF8472 )
		{
			vty_out(vty,"uplink %d/%d sfp is online, but don`t support optical power detection.\r\n",ulSlot,sfp_sel+17 );
			return CMD_WARNING;
		}
		/*else
			vty_out(vty," Diagnostic monitor type: %s calibrated\r\n",  
				((UplinkPortMeteringInfo[portno].powerMeteringSupport == EXTERNALLY_CALIBRATED) ? "externally" : "internally") );*/
	}
	else
	{
		if( (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) 
			&& !SYS_MODULE_IS_UPLINK_PON(ulSlot) )
		{
			if( TRUE == i2c_data_get( g_PhysicalSlot[ulSlot],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 12949*/
			{
				if( value & 1<<sfp_sel )
				{		
					vty_out(vty,"uplink %d/%d sfp is not online .\r\n",ulSlot,sfp_sel+1 );
					return CMD_WARNING;
				}
			}
			flag = 1;
		}
		
		portno = ulSlot*MAXUPLINKPORT +sfp_sel;
		if( UplinkPortMeteringInfo[portno].powerMeteringSupport == NOT_SUPPORT_SFF8472 )
		{
			if( flag == 1 )
			{
				vty_out(vty,"uplink %d/%d sfp is online, but don`t support optical power detection.\r\n",ulSlot,sfp_sel+1 );
				return CMD_WARNING;
			}
			else
			{
				if( SYS_MODULE_IS_UPLINK_PON(ulSlot) )
				{
					length = sizeof(UplinkPortMeteringInfo_S);
					pQueueMsg = VOS_Malloc(length, MODULE_RPU_PON_MON);
					if(pQueueMsg == NULL)
					{
						VOS_ASSERT( 0 );
						return VOS_ERROR;
					}
					
					VOS_MemZero(pQueueMsg, length);
					
					ret = Fetch_Sfp_Online_State( PonPortIdx, (VOID *)pQueueMsg, length, 1 );  /* 1:UPLINK */
						
					if(VOS_OK== ret) 
					{	
						if( pQueueMsg->powerMeteringSupport != NOT_SUPPORT_SFF8472 )
						{
							VOS_MemCpy( (VOID *)(&(UplinkPortMeteringInfo[portno])), (VOID *)pQueueMsg, sizeof(UplinkPortMeteringInfo_S) );
							VOS_Free( pQueueMsg );
						}
						else
						{
							vty_out(vty,"uplink %d/%d sfp is not online or not support Diagnostic monitor.\r\n",ulSlot,sfp_sel+1 );
							VOS_Free( pQueueMsg );
							return CMD_WARNING;
						}
					}
					else
					{
						sys_console_printf("\r\n  Get sfp online state failed!\r\n");
						VOS_Free( pQueueMsg );
						return VOS_ERROR;
					}
				}
				else
				{
					vty_out(vty,"uplink %d/%d sfp is not online .\r\n",ulSlot,sfp_sel+1 );
					return CMD_WARNING;
				}
			}
			
		}
		else
			vty_out(vty," Diagnostic monitor type: %s calibrated\r\n",  
				((UplinkPortMeteringInfo[portno].powerMeteringSupport == EXTERNALLY_CALIBRATED) ? "externally" : "internally") );
	}
	
	if((ret=GetUplinkPortTemperature(ulSlot, sfp_sel))==ERROR)
	{
		vty_out(vty,"get temperature info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	vty_out(vty,"  %s: %d Celsius\r\n",XcvrDiagArr[34].pcName, ret);

	if((ret=GetUplinkPortVoltage(ulSlot,sfp_sel))==ERROR)
	{
		vty_out(vty,"get voltage info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d.%d V\r\n", XcvrDiagArr[35].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );

	if((ret=GetUplinkPortBiasCurrent(ulSlot,sfp_sel))==ERROR)
	{
		vty_out(vty,"get bias-current info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d mA\r\n", XcvrDiagArr[36].pcName, ret );

	if((ret=GetUplinkPortTransOpticalPower(ulSlot,sfp_sel))==ERROR)
	{
		vty_out(vty,"get transpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	if( /*ret != -50   &&*/ ret != uplink_rx_optical_power_default && decimal2_integer_part(ret) > -30 && ret !=0)
	{	
		if(ret < 0 && ret >-10)
		{
			vty_out( vty, "  %s: -%d.%d dbm\r\n", XcvrDiagArr[37].pcName, 0, abs(ret%10) );
		}
		else
		{
			vty_out( vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[37].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
		}
	}
	else
		vty_out( vty, "  %s: --\r\n", XcvrDiagArr[37].pcName );

	
	if((ret=GetUplinkPortRecvOpticalPower(ulSlot,sfp_sel))==ERROR)
	{
		vty_out(vty,"get recvpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
		return CMD_WARNING;
	}
	/*有光模块在不UP时，接收为0。add by yanjy*/
	if( /*ret != -50   &&*/ ret != uplink_rx_optical_power_default && decimal2_integer_part(ret) > -40 && ret !=0)  /*问题单: 15888 */
	{	
		if(ret < 0 && ret >-10)
		{
			vty_out( vty, "  %s: -%d.%d dbm\r\n", XcvrDiagArr[38].pcName, 0, abs(ret%10) );
		}
		else
		{
			vty_out( vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[38].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
		}
	}
	else
		vty_out( vty, "  %s: --\r\n", XcvrDiagArr[38].pcName );

	return CMD_SUCCESS;
}

extern int show_uplink_sfp_transceiver_meter_10GE( struct vty *vty, ULONG ulSlot, ULONG ulPort, int read );
DEFUN(uplink_transceiver_meter_show,
	uplink_transceiver_meter_show_cmd,
	"show optical-power uplink <slot/port> ",
	SHOW_STR
	"Uplink Optical transceiver info\n"
	"Olt UPLINK\n"
	"input slot/port\n"
	)
{
	unsigned long ulSlot, ulPort;
	LONG lRet;
	int Flag;
	
	lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if( lRet != VOS_OK )
			return CMD_WARNING;
		
	if(GetPonPortOpticalMonitorEnable() == V2R1_DISABLE)
	{
		vty_out(vty," optical power is disable\r\n");
		return CMD_WARNING;
	}

	if( SYS_MODULE_ISMASTERSTANDBY( SYS_LOCAL_MODULE_SLOTNO ) )
		return CMD_SUCCESS;

	if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		if(ulPort >20 || ulPort <17)
		{
			vty_out(vty," Uplink port range is 17-20\r\n");
			return CMD_WARNING;
		}
	}
	
	
		if(!(((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW) && (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && 
		(( ulSlot == 5 || ulSlot == 6 || ulSlot == 9 ||ulSlot == 10 ) &&  ulPort == 5)) ||  ((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) && 
		(((__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && ulPort == 5) ||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XETA1)
		|| (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XET)
		||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_4XET)))))
		{
			
			Flag = 1;/*10GE---1*/
			
		}
		else
			Flag = 0;
	
	
		
	
	if(((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW) && (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && 
		(( ulSlot == 5 || ulSlot == 6 || ulSlot == 9 ||ulSlot == 10 ) &&  ulPort == 5)) ||  ((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) && 
		(((__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && ulPort == 5) ||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XETA1)
		|| (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XET)
		||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_4XET))))
	{
		show_uplink_sfp_transceiver_meter_10GE( vty, ulSlot, ulPort, Flag );
	}
	else
	{	
		show_uplink_sfp_transceiver_meter( vty, ulSlot, ulPort );
	}
	return CMD_SUCCESS;
}
DEFUN(uplink_transceiver_meter_show_10GE_thred,
	uplink_transceiver_meter_show_cmd_10GE_thred,
	"show optical-power uplink-10GE-alarm-threshold <slot/port> ",
	SHOW_STR
	"Uplink Optical transceiver info\n"
	"Olt UPLINK 10GE alarm-threshold\n"
	"input slot/port, 10GE port\n"
	)
{
	unsigned long ulSlot, ulPort;
	LONG lRet;
	lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if( lRet != VOS_OK )
			return CMD_WARNING;

	if( SYS_MODULE_ISMASTERSTANDBY( SYS_LOCAL_MODULE_SLOTNO ) )
		return CMD_SUCCESS;
	
	if(GetPonPortOpticalMonitorEnable() == V2R1_DISABLE)
	{
		vty_out(vty," optical power is disable\r\n");
		return CMD_WARNING;
	}
	if(( __SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA6900_GEM_10GE) &&(__SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA8000_8XETA1)&& (__SYS_MODULE_TYPE__(ulSlot) != MODULE_E_GFA8000_8XET))
	{
		vty_out(vty, " The slot is not for 10GE.\r\n");
		return CMD_WARNING;
	}
	
	if(((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW) && (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && 
		(( ulSlot == 5 || ulSlot == 6 || ulSlot == 9 ||ulSlot == 10 ) &&  ulPort == 5)) ||  ((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) && 
		(((__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA6900_GEM_10GE) && ulPort == 5) || (__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XETA1)||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_4XET))))
	{
		 show_uplink_sfp_transceiver_meter_10GE_Alarm( vty, ulSlot, ulPort );
	}
	else
	{	
		vty_out(vty, "The port is not 10GE port !\r\n"  );
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
	
}
#define I2C_BASE_10G_SFP			0x60
#define I2C_BASE_1G_SFP_16GPON			0xd0
_XCVR_DATA_ XFPInfo[] = {
	{ 0,  "Identifier", 				1, 0 },  		/*  0  */
	{ 1,  "Signal Conditioner Control", 1, 0 },
	/*{ 2,  "Threshold Values used for Alarm and Warning Flags", 	56, 0 },*/
	{ 2, "Temp High Alarm",	2, 0 },  
	{ 4, "Temp Low Alarm",	2, 0 },
	{ 6, "Temp High Warning",	2, 0 },
	{ 8, "Temp Low Warning",	2, 0 },                 /*  5 */
	{ 10, "Reserved A/D Flag Thresholds",	8, 0 },
	{ 18, "Bias High Alarm",	2, 0 },
	{ 20, "Bias Low Alarm",	2, 0 },
	{ 22, "Bias High Warning",	2, 0 },
	{ 24, "Bias Low Warning",	2, 0 },               /* 10 */
	{ 26, "TX Power High Alarm",	2, 0 },
	{ 28, "TX Power Low Alarm",	2, 0 },
	{ 30, "TX Power High Warning",	2, 0 },
	{ 32, "TX Power Low Warning",	2, 0 },
	{ 34, "RX Power High Alarm",	2, 0 },        /* 15 */
	{ 36, "RX Power Low Alarm",	2, 0 },
	{ 38, "RX Power High Warning",	2, 0 },
	{ 40, "RX Power Low Warning",	2, 0 },
	{ 42, "Voltage High Alarm",	2, 0 },
	{ 44, "Voltage Low Alarm",	2, 0 },              /* 20 */
	{ 46, "Voltage High Warning",	2, 0 },
	{ 48, "Voltage Low Warning",	2, 0 },
	{ 50, "AUX 2 High Alarm",	2, 0 },
	{ 52, "AUX 2 Low Alarm",	2, 0 },
	{ 54, "AUX 2 High Warning",	2, 0 },             /* 25  */
	{ 56, "AUX 2 Low Warning",	2, 0 },
	{ 58,  "Optional VPS Control Registers",  2, 0 },
	{ 60, "Reserved", 				10, 0 },
	{ 70, "BER Reporting", 			2, 0 },		/* 29  */
	{ 72, "Wavelength Control Registers", 	4, 0 },
	{ 76, "FEC control Register",		4, 0 },
	{ 80, "Flags and Interrupt Control",	16, 0 },
	/*{ 96, "A/D readout",		14, 0 },*/
	{ 96, "Temperature",		2, 0 },            /* 33 */			/*{ 97, "Temperature LSB",		1, 0 }, */  
	{ 98, "Reserved",		2, 0 },                   /* 34  */
	{ 100, "Bias-current",		2, 0 },           	/* 35 */     		/*{ 101, "TX Bias LSB",		1, 0 },*/
	{ 102, "TX Optical-power",		2, 0 },    	/*36  */			/*{ 103, "TX Power LSB",		1, 0 },*/       
	{ 104, "RX Optical-Power",		2, 0 },    	/* 37 */      		/*{ 105, "RX Power LSB",		1, 0 },*/
	{ 106, "Working-voltage",		2, 0 },	/* 38  */			/*{ 107, "AUX 1 LSB",		1, 0 },*/
	{ 108, "AUX 2",		2, 0 },          		/* 39  */ 		/*{ 109, "AUX 2 LSB ",		1, 0 },*/
	{ 110, "General Control/Status bits",	2, 0 },		/*  40  */
	{ 112, "Reserved",		6, 0 },
	{ 118, "Serial Interface Read/Write Error Checking", 		1, 0 },
	{ 119, "Password Change Entry Area (Optional)", 			4, 1 },   /* 43  */
	{ 123, "Password Entry Area (optional)", 				4, 0 },
	{ 127, "Page Select Byte", 			1, 0 },			/*  45  */
	/* defult : Serial ID */
	{ 128, "Identifier",		1, 0 },
	{ 129, "Ext. Identifier",		1, 0 },
	{ 130, "Connector",		1, 0 },             /* 48  */
	{ 131, "Transceiver",		8, 0 },
	{ 139, "Encoding",		1, 0 },
	{ 140, "BR-Min ",		1, 0 },
	{ 141, "BR-Max",		1, 0 },           
	{ 142, "Length(SMF)-km",		1, 0  },    /* 53*/
 	{ 143, "Length (E-50μm)",		1, 0 },
	{ 144, "Length (50 μm)",		1, 0 },
	{ 145, "Length (62.5 μm)",	1, 0 },
	{ 146, "Length (Copper)",		1, 0 },
	{ 147, "Device Tech",		1, 0 },          /* 58  */
	{ 148, "Vendor name",		16, 1 },
	{ 164, "CDR Support ",		1, 0 },
	{ 165, "Vendor OUI",		3, 0 },
	{ 168, "Vendor PN",		16, 1 },
	{ 184, "Vendor rev",		2, 1 },         /* 63 */
	{ 186, "Wavelength",		2, 0 },
	{ 188, "Wavelength Tolerance",		2, 0 },
	{ 190, "Max Case Temp",		1, 0 },
	{ 191, "CC_BASE",		1, 0 },
	/* Extended Id Field */
	{ 192, "Power Supply",		4, 0 },        /* 68 */
	{ 196, "Vendor SN",		16, 1 },
	{ 212, "Date code",		8, 1 },
	{ 220, "Diagnostic Monitoring Type",		1, 0 },
	{ 221, "Enhanced Options",		1, 0 },
	{ 222, "Aux Monitoring",		1, 0 },       /* 73 */
	{ 223, "CC_EXT",		1, 0 },
	/* Vendor Specific ID Fidlds */
	{ 224, "Vendor Specific",		31, 0 },
	{ 255, NULL,					1, 0 }		/*  76  */
};
ULONG  Test_10GE = 0;
int ReadUplinkPortSFPTemperature_10GE(short int slotno,int sfp_sel, long *val)	
{
	int ret = 0;
	_XCVR_DATA_ *pXcvrArr = NULL;
	short int data = 0;
	data=0;
	*val=40;
	if( (sfp_sel<0)||(sfp_sel>=MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		pXcvrArr=XFPInfo;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[33].cAddr,(unsigned char *)&data, 2);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_8100_10GE_OFFSET+sfp_sel, pXcvrArr[34].cAddr,(unsigned char *)&data, 2);		
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)/*add by yanjy-2016-09*/
	{
		pXcvrArr=XcvrDiagArr;
		ret =i2c_data_get2( slotno, sfp_sel+1,0x51, pXcvrArr[34].cAddr, (unsigned char *)&data,2);
	}
	else
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[34].cAddr,(unsigned char *)&data, 2);
	}
	/*ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[33].cAddr, (unsigned char *)&data,2);*/
	if(ret == TRUE)
	{
		*val=TranslateTemperature(data);	
		return VOS_OK;
	}
	else
		return VOS_ERROR;
}
int ReadUplinkPortSFPVoltage_10GE(short int slotno, int sfp_sel, long *val)
{
	int ret = 0;
	_XCVR_DATA_ *pXcvrArr = NULL;
	unsigned short int data = 0;
	data=0;
	*val=33;
	if( (sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		pXcvrArr=XFPInfo;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[38].cAddr,(unsigned char *)&data, 2);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		pXcvrArr=XcvrDiagArr;	
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_8100_10GE_OFFSET+sfp_sel, pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		ret =i2c_data_get2( slotno, sfp_sel+1,0x51, pXcvrArr[35].cAddr, (unsigned char *)&data,2);
	}
	else
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
	}
	/*ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[38].cAddr, (unsigned char *)&data,2);*/
	if(ret == TRUE)
	{
		*val=TranslateWorkVoltage(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;	
}
int ReadUplinkPortSFPBiasCurrent_10GE(short int slotno, int sfp_sel, long *val)
{
	int ret = 0;
	_XCVR_DATA_ *pXcvrArr = NULL;
	unsigned short int data = 0;
	data=0;
	*val=15;
	if((sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		pXcvrArr=XFPInfo;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[35].cAddr,(unsigned char *)&data, 2);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		pXcvrArr=XcvrDiagArr;	
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_8100_10GE_OFFSET+sfp_sel, pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		/*ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[35].cAddr,(unsigned char *)&data, 2);*/
		ret =i2c_data_get2( slotno, sfp_sel+1,0x51, pXcvrArr[36].cAddr, (unsigned char *)&data,2);
	}
	else
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
	}
	/*ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[35].cAddr, (unsigned char *)&data,2);*/
	if(ret == TRUE)
	{
		*val=TranslateBiasCurrent(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;		
}
int ReadUplinkPortSFPTransPower_10GE(short int slotno, int sfp_sel, long *val)
{
	int ret = 0;
	_XCVR_DATA_  *pXcvrArr = NULL;
	short int data = 0;
	data=0;
	*val=-50;
/*
	if( (sfp_sel<0)||(sfp_sel>3))
	{
		return VOS_ERROR;
	}*/	
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		pXcvrArr=XFPInfo;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[36].cAddr,(unsigned char *)&data, 2);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		pXcvrArr=XcvrDiagArr;	
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_8100_10GE_OFFSET+sfp_sel, pXcvrArr[37].cAddr,(unsigned char *)&data, 2);
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		ret =i2c_data_get2( slotno, sfp_sel+1,0x51, pXcvrArr[37].cAddr, (unsigned char *)&data,2);
	}
	else
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[37].cAddr,(unsigned char *)&data, 2);
	}
	/*ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[36].cAddr, (unsigned char *)&data,2);*/
	if( Test_10GE == 1)
		sys_console_printf( " Tx data is %x\n",data );
	/*SFP_RSSI_DEBUG(256,("\nret=%d,Transpower data=%x\r\n",ret,data));*/
	if(ret == TRUE)
	{
		*val=TranslateOpticalPower(data);
		return VOS_OK;
	}
	else
		return VOS_ERROR;
}
int ReadUplinkPortSFP_RxPower_10GE(short int slotno, int sfp_sel, long *val)
{
	int ret = 0;
	_XCVR_DATA_  *pXcvrArr = NULL;
	short int data = 0;
	*val=-50;
/*
	if( (sfp_sel<0)||(sfp_sel>3))
	{
		return VOS_ERROR;
	}*/			
	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		pXcvrArr=XFPInfo;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP, pXcvrArr[37].cAddr,(unsigned char *)&data, 2);
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
	{
		pXcvrArr=XcvrDiagArr;	
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_8100_10GE_OFFSET+sfp_sel, pXcvrArr[38].cAddr,(unsigned char *)&data, 2);
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		ret =i2c_data_get2( slotno, sfp_sel+1,0x51, pXcvrArr[38].cAddr, (unsigned char *)&data,2);
	}
	else
	{
		pXcvrArr=XcvrDiagArr;
		ret = i2c_data_get(slotno,I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[38].cAddr,(unsigned char *)&data, 2);
	}
	/*ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, pXcvrArr[37].cAddr, (unsigned char *)&data,2);*/
	if( Test_10GE == 1)
		sys_console_printf( " Rx data is %x\n",data );
	/*SFP_RSSI_DEBUG(256,("\nret=%d,Transpower data=%x\r\n",ret,data));*/
	if(ret == TRUE)
	{
		*val=TranslateOpticalPower(data);
		
		return VOS_OK;
	}
	else
		return VOS_ERROR;
}
int show_uplink_sfp_transceiver_meter_10GE( struct vty *vty, ULONG ulSlot, ULONG ulPort, int read )
{
	int sfp_sel,max_link_port;
	int portno;
	int flag = 0;
	long ret=0;
	UCHAR value;
	char data = 0;
	
	sfp_sel=ulPort;
	sfp_sel=sfp_sel-1;
	max_link_port=GetMAXUplinkPort(ulSlot);
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 板在位检查*/
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	if(SlotCardIsUplinkBoard(ulSlot) != ROK )		/*板类型检查*/
	{
		vty_out(vty," %% slot %d is not uplink board\r\n", ulSlot);
		return( CMD_WARNING );
	}
	if( (sfp_sel < 0) ||(sfp_sel > max_link_port-1) )
	{
		vty_out( vty, "\r\n SFPId=%d out of rang \r\n",sfp_sel+1);
		return CMD_WARNING;
	}
#if 0
	if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW)
	{
		if( TRUE == i2c_data_get( g_PhysicalSlot[ulSlot],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 12949*/
		{
			if( value & 1<<sfp_sel )
			{		
				vty_out(vty,"uplink %d/%d sfp is not online .\r\n",ulSlot,sfp_sel+1 );
				return CMD_WARNING;
			}
		}
		flag = 1;
	}
#endif
	portno = ulSlot*MAXUPLINKPORT +sfp_sel;
#if 0
	if( Check_Sfp_Online_6900_10GE( ulSlot, sfp_sel ) == FALSE)
	{
		vty_out(vty,"uplink %d/%d sfp is not online.\r\n",ulSlot,sfp_sel+1 );
		UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
		return CMD_WARNING;
	}
#else
	if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
	{
		if( TRUE == i2c_data_get( g_PhysicalSlot[ulSlot],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 15611*/
		{
			if( value & 1<<sfp_sel )
			{
				UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
			}
			else
			{
				if((__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XET)||(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_4XET))
				{
					if( i2c_data_get( ulSlot, I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[26].cAddr, &data, 1) )
					{
						if( data & (1<<6) )	/* 0x58 == 01011000 */
						{
							if( data & (1<<4) )
								UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
							else
								UplinkPortMeteringInfo[portno].powerMeteringSupport = INTERNALLY_CALIBRATED;
						}
						else
						{
							UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
						}
					}
					else
					{
						UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
					}
				}
				else if(__SYS_MODULE_TYPE__(ulSlot) == MODULE_E_GFA8000_8XETA1)/*add by yanjy-2016-09*/
				{
					if(TRUE==  i2c_data_get2( ulSlot, sfp_sel+1,0x50, XcvrInfoArr[26].cAddr, &data, 1))
					{
						
							if( data & (1<<6) )	/* 0x58 == 01011000 */
							{
								if( data & (1<<4) )
									UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
								else
									UplinkPortMeteringInfo[portno].powerMeteringSupport = INTERNALLY_CALIBRATED;
							}
							else
							{
								UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
							}
					}
					else
					{
						UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
					}
						
				}
				else
				{
					UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
				}
			}
		}
		else
		{
			UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
		}
	}				
	else
	{
		if( FALSE == Check_Sfp_Online_10GE( ulSlot ))
		{
			UplinkPortMeteringInfo[portno].powerMeteringSupport = NOT_SUPPORT_SFF8472;
		}
		else
			UplinkPortMeteringInfo[portno].powerMeteringSupport = EXTERNALLY_CALIBRATED;
	}
						
#endif
	if( UplinkPortMeteringInfo[portno].powerMeteringSupport == NOT_SUPPORT_SFF8472 )
	{
		vty_out(vty,"uplink %d/%d sfp is not online or not support Diagnostic monitor\r\n",ulSlot,sfp_sel+1 );
		return CMD_WARNING;
	}
	
	if(read == 0)
	{
		if((ret=GetUplinkPortTemperature(ulSlot, sfp_sel))==ERROR)
		{
			vty_out(vty,"get temperature info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
			return CMD_WARNING;
		}
	}	
	else
		ReadUplinkPortSFPTemperature_10GE( ulSlot, sfp_sel, &ret );
	
	vty_out(vty,"  %s: %d Celsius\r\n",XFPInfo[33].pcName, ret);
	
	if(read == 0)
	{
		if((ret=GetUplinkPortVoltage(ulSlot,sfp_sel))==ERROR)
		{
			vty_out(vty,"get voltage info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
			return CMD_WARNING;
		}
	}
	else
		ReadUplinkPortSFPVoltage_10GE( ulSlot, sfp_sel, &ret );
	vty_out(vty, "  %s: %d.%d V\r\n", XFPInfo[38].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	
	if(read == 0)
	{
		if((ret=GetUplinkPortBiasCurrent(ulSlot,sfp_sel))==ERROR)
		{
			vty_out(vty,"get bias-current info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
			return CMD_WARNING;
		}
	}
	else
		ReadUplinkPortSFPBiasCurrent_10GE( ulSlot, sfp_sel, &ret );
	vty_out(vty, "  %s: %d mA\r\n", XFPInfo[35].pcName, ret );
	
	if(read == 0)
	{
		if((ret=GetUplinkPortTransOpticalPower(ulSlot,sfp_sel))==ERROR)
		{
			vty_out(vty,"get transpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
			return CMD_WARNING;
		}
	}
	else
		ReadUplinkPortSFPTransPower_10GE( ulSlot, sfp_sel, &ret );	
	if( ret != uplink_rx_optical_power_default && ret != 0 )
	{
		if(ret < 0 && ret >-10)
		{
			vty_out( vty, "  %s: -%d.%d dbm\r\n", XFPInfo[36].pcName, 0, abs(ret%10) );
		}
		else
		{
			vty_out( vty, "  %s: %d.%d dbm\r\n", XFPInfo[36].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
		}
	}
	
	if(read == 0)
	{
		if((ret=GetUplinkPortRecvOpticalPower(ulSlot,sfp_sel))==ERROR)
		{
			vty_out(vty,"get recvpower info from uplink sfp %d err(code=%d)\r\n",sfp_sel,ret);
			return CMD_WARNING;
		}
	}
	else
		ReadUplinkPortSFP_RxPower_10GE( ulSlot, sfp_sel, &ret );
	if( ret != uplink_rx_optical_power_default && ret != 0 && decimal2_integer_part(ret) > -30  )
	{
		if(ret < 0 && ret >-10)
		{
			vty_out( vty, "  %s: -%d.%d dbm\r\n", XFPInfo[37].pcName, 0, abs(ret%10) );
		}
		else
		{
			vty_out( vty, "  %s: %d.%d dbm\r\n", XFPInfo[37].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
		}
	}
	else
		vty_out( vty, "  %s: --\r\n", XFPInfo[37].pcName );
	
	return CMD_SUCCESS;
}
int ReadUplinkPortSFPTemperature_10GE_Alarm(struct vty *vty, short int slotno,int sfp_sel, long *val)	
{
	int ret, i;
	_XCVR_DATA_ *pXcvrArr;
	short int data;
	pXcvrArr=XFPInfo;
	data=0;
	*val=40;

	if(vty == NULL )
		return VOS_ERROR;
	
	if( (sfp_sel<0)||(sfp_sel>=MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	

	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		for( i= 0; i < 4; i++ )
		{
			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, pXcvrArr[2+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[2+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[0+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)/*add by yanjy-2016-09*/
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			/*ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);*/
			ret =i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x51, pXcvrArr[0+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[0+i].pcName, *val);
			}
		}
	}


	return VOS_OK;
	
}

int ReadUplinkPortSFPVoltage_10GE_Alarm(struct vty *vty, short int slotno, int sfp_sel, long *val)
{
	int ret, i;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data;

	pXcvrArr=XFPInfo;
	data=0;
	*val=33;

	if(vty == NULL )
		return VOS_ERROR;
	
	if( (sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	

	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		for( i= 0; i < 4; i++ )
		{
			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, pXcvrArr[19+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateWorkVoltage(data);	
				vty_out(vty,"  %s: %d.%d V\r\n",pXcvrArr[19+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[4+i].cAddr,(unsigned char *)&data, 2);
			if(ret == TRUE)
			{
				*val=TranslateWorkVoltage(data);	
				vty_out(vty,"  %s: %d.%d V\r\n",pXcvrArr[4+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			/*ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);*/
			ret =i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x51, pXcvrArr[4+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[4+i].pcName, *val);
			}
		}
	}

	return VOS_OK;
}
int ReadUplinkPortSFPBiasCurrent_10GE_Alarm(struct vty *vty, short int slotno, int sfp_sel, long *val)
{
	int ret, i;
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data;
	
	pXcvrArr=XFPInfo;
	data=0;
	*val=15;
	if(vty == NULL )
		return VOS_ERROR;
	
	if((sfp_sel<0)||(sfp_sel>= MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	

	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		for( i= 0; i < 4; i++ )
		{
			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, pXcvrArr[7+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateBiasCurrent(data);	
				vty_out(vty,"  %s: %d mA\r\n",pXcvrArr[7+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[8+i].cAddr,(unsigned char *)&data, 2);
			if(ret == TRUE)
			{
				*val=TranslateBiasCurrent(data);	
				vty_out(vty,"  %s: %d mA\r\n",pXcvrArr[8+i].pcName, *val);
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			/*ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);*/
			ret =i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x51, pXcvrArr[8+i].cAddr, (unsigned char *)&data,2);
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[8+i].pcName, *val);
			}
		}
	}
	return VOS_OK;
	
}
int ReadUplinkPortSFPTransPower_10GE_Alarm( struct vty *vty, short int slotno, int sfp_sel, long *val)
{
	int ret, i;
	_XCVR_DATA_  *pXcvrArr;
	short int data;
	
	pXcvrArr = XFPInfo;
	data=0;
	*val=-50;
	if(vty == NULL )
		return VOS_ERROR;
	
	if( (sfp_sel<0)||(sfp_sel>MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	

	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		for( i= 0; i < 4; i++ )
		{
			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, pXcvrArr[11+i].cAddr, (unsigned char *)&data,2);
			if( Test_10GE == 1)
				sys_console_printf( " Tx data is %x\n",data );
			/*SFP_RSSI_DEBUG(256,("\nret=%d,Transpower data=%x\r\n",ret,data));*/
			if(ret == TRUE)
			{
				*val=TranslateOpticalPower(data);
				vty_out( vty, "  %s: %d.%d dbm\r\n", pXcvrArr[11+i].pcName, decimal2_integer_part(*val), decimal2_fraction_part(*val) );
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[12+i].cAddr,(unsigned char *)&data, 2);
			if( Test_10GE == 1)
				sys_console_printf( " Tx data is %x\n",data );
			/*SFP_RSSI_DEBUG(256,("\nret=%d,Transpower data=%x\r\n",ret,data));*/
			if(ret == TRUE)
			{
				*val=TranslateOpticalPower(data);
				vty_out( vty, "  %s: %d.%d dbm\r\n", pXcvrArr[12+i].pcName, decimal2_integer_part(*val), decimal2_fraction_part(*val) );
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			/*ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);*/
			ret =i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x51, pXcvrArr[12+i].cAddr, (unsigned char *)&data,2);
			if( Test_10GE == 1)
				sys_console_printf( " Rx data is %x\n",data );
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[12+i].pcName, *val);
			}
		}
	}
	return VOS_OK;
	
}
int ReadUplinkPortSFP_RxPower_10GE_Alarm( struct vty *vty, short int slotno, int sfp_sel, long *val)
{
	int ret, i;
	_XCVR_DATA_  *pXcvrArr;
	short int data;
	
	pXcvrArr = XFPInfo;
	data=0;
	*val=-50;
	
	if(vty == NULL )
		return VOS_ERROR;
	
	if( (sfp_sel<0)||(sfp_sel>MAXUPLINKPORT))
	{
		return VOS_ERROR;
	}	

	if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
	{
		for( i= 0; i < 4; i++ )
		{
			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, pXcvrArr[11+i].cAddr, (unsigned char *)&data,2);
			if( Test_10GE == 1)
				sys_console_printf( " Rx data is %x\n",data );
			if(ret == TRUE)
			{
				*val=TranslateOpticalPower(data);
				vty_out( vty, "  %s: %d.%d dbm\r\n", pXcvrArr[11+i].pcName, decimal2_integer_part(*val), decimal2_fraction_part(*val) );
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[16+i].cAddr,(unsigned char *)&data, 2);
			if( Test_10GE == 1)
				sys_console_printf( " Rx data is %x\n",data );
			if(ret == TRUE)
			{
				*val=TranslateOpticalPower(data);
				vty_out( vty, "  %s: %d.%d dbm\r\n", pXcvrArr[16+i].pcName, decimal2_integer_part(*val), decimal2_fraction_part(*val) );
			}
		}
	}
	else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
	{
		pXcvrArr=XcvrDiagArr;
		for( i= 0; i < 4; i++ )
		{
			/*ret = sfp_i2c_read(g_PhysicalSlot[slotno],I2C_BASE_10G_SFP+SFP_10GE_OFFSET+sfp_sel, pXcvrArr[0+i].cAddr,(unsigned char *)&data, 2);*/
			ret =i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x51, pXcvrArr[16+i].cAddr, (unsigned char *)&data,2);
			if( Test_10GE == 1)
				sys_console_printf( " Rx data is %x\n",data );
			if(ret == TRUE)
			{
				*val=TranslateTemperature(data);	
				vty_out(vty,"  %s: %d Celsius\r\n",pXcvrArr[16+i].pcName, *val);
			}
		}
	}
	return VOS_OK;
	
}
int show_uplink_sfp_transceiver_meter_10GE_Alarm( struct vty *vty, ULONG ulSlot, ULONG ulPort )
{
	int sfp_sel,max_link_port;
	int portno;
	int flag = 0;
	long ret=0;
	UCHAR value;
	
	sfp_sel=ulPort;
	sfp_sel=sfp_sel-1;
	max_link_port=GetMAXUplinkPort(ulSlot);
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 板在位检查*/
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( CMD_WARNING );
	}
	
	if(SlotCardIsUplinkBoard(ulSlot) != ROK )		/*板类型检查*/
	{
		vty_out(vty," %% slot %d is not uplink board\r\n", ulSlot);
		return( CMD_WARNING );
	}
	if( (sfp_sel < 0) ||(sfp_sel > max_link_port-1) )
	{
		vty_out( vty, "\r\n SFPId=%d out of rang \r\n",sfp_sel+1);
		return CMD_WARNING;
	}
#if 0
	if( Check_Sfp_Online_6900_10GE( ulSlot, sfp_sel ) == FALSE)
	{
		vty_out(vty,"uplink %d/%d sfp is not online.\r\n",ulSlot,sfp_sel+1 );
		return VOS_ERROR;
	}
#else
	if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
	{
		if( TRUE == i2c_data_get( g_PhysicalSlot[ulSlot],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 15611*/
		{
			if( value & 1<<sfp_sel )
			{
				vty_out(vty,"uplink %d/%d sfp is not online.\r\n",ulSlot,sfp_sel+1 );
				return VOS_ERROR;
			}
		}
	}
#endif
	ReadUplinkPortSFPTemperature_10GE_Alarm( vty, ulSlot, sfp_sel, &ret );
	
	ReadUplinkPortSFPVoltage_10GE_Alarm( vty, ulSlot, sfp_sel, &ret );
	
	ReadUplinkPortSFPBiasCurrent_10GE_Alarm( vty, ulSlot, sfp_sel, &ret );
	
	ReadUplinkPortSFPTransPower_10GE_Alarm( vty, ulSlot, sfp_sel, &ret );
	
	ReadUplinkPortSFP_RxPower_10GE_Alarm( vty, ulSlot, sfp_sel, &ret );
	return CMD_SUCCESS;
}

#if 0
/* 下面的API 与命令行cli 相关，用于显示OLT PON 自身光模块参数*/
/*** 1 显示温度***/
ULONG showSFPTemperature(short int PonPortIdx, struct vty *vty)
{
	int ret;
	CHECK_PON_RANGE
	if((ret = GetPonPortTemperature(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get temperature info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)),(GetPonPortByPonChip(PonPortIdx)), ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d Celsius\r\n", XcvrDiagArr[34].pcName, ret);
	return CMD_SUCCESS;
}

/*** 2 显示工作电压***/
ULONG showSFPVoltage(short int PonPortIdx,struct vty *vty)
{
	int ret;

	CHECK_PON_RANGE
	if((ret = GetPonPortWorkVoltage(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get working-voltage info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)),(GetPonPortByPonChip(PonPortIdx)), ret);
		return CMD_WARNING;
	}
		
	vty_out(vty, "  %s: %d.%d V\r\n", XcvrDiagArr[35].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}

/*** 3 显示偏置电流***/
ULONG showSFPBias(short int PonPortIdx, struct vty *vty)
{
	int ret;
	CHECK_PON_RANGE
	if((ret=GetPonPortBiasCurrent(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get bias-current info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)),(GetPonPortByPonChip(PonPortIdx)), ret);
		return CMD_WARNING;
	}
	
	vty_out(vty, "  %s: %d(mA)\r\n", XcvrDiagArr[36].pcName, ret);
	return CMD_SUCCESS;
}

/*** 4 显示发射光功率***/
ULONG showSFPPower(short int PonPortIdx, struct vty *vty)
{
	int ret;
	CHECK_PON_RANGE

	if((ret=GetPonPortTransOpticalPower(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"No tx-power info pon%d/%d (err-code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)),(GetPonPortByPonChip(PonPortIdx)), ret);
		return CMD_WARNING;
	}
		
	vty_out(vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[37].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}
#endif

 int checkPonPortIndex( struct vty *vty, ULONG  ulSlot, ULONG ulPort )
{
	int PonPortIdx;
	
	if(PonCardSlotPortCheckWhenRunningByVty(ulSlot, ulPort, vty) != ROK)
		return(RERROR);

	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	/* 1 板在位检查*/
	{
		vty_out(vty," %% slot %d is not inserted\r\n", ulSlot);
		return( RERROR );
	}
	
	if(SlotCardIsPonBoard(ulSlot) != ROK )		/* 2 pon 板类型检查*/
	{
		vty_out(vty," %% slot %d is not pon card\r\n", ulSlot);
		return( RERROR );
	}

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)	/* 3  pon chip is inserted  */
	{
		vty_out(vty,"  %% pon%d/%d is not inserted\r\n", ulSlot, ulPort);
		return(RERROR);
	}

	/*if( ponSfp_IsSupportRSSI(ulSlot, ulPort,vty) != ROK)
	{
		vty_out(vty,"  %% pon%d/%d not support optical-power metering\r\n", ulSlot, ulPort);
		return(RERROR);
	}

	if( SFPIsOnline(ulSlot, ulPort) != ROK)
	{
		vty_out(vty,"  %% pon%d/%d no SFP module on-line\r\n", ulSlot, ulPort);
		return(RERROR);
	}*/

	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
	{
		vty_out(vty," %% Can not find pon%d/%d\r\n", ulSlot, ulPort );	/* modified by xieshl 20101220, 问题单11521 */
		return RERROR;
	}
	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		vty_out(vty,"  %% pon%d/%d is not working\r\n", ulSlot, ulPort);
		return(RERROR);
	}
	return PonPortIdx;
}

/*  显示OLT PON 口光模块参数*/
#if 0
static int show_sfp_transceiver_info( struct vty *vty, int argc, char **argv, ULONG ulSlot, ULONG ulPort )
{
	ULONG ulDevAddr;
	int gwRet;
	short int	ucValue;
	unsigned char	cValueArr[16]={0};
	int i,j;
	_XCVR_DATA_  *pXcvrArr;
	unsigned char *pString;

	int PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	/*CHECK_PON_RANGE*/	/* 问题单10755 */
	if( PonPortIdx == VOS_ERROR )
		return CMD_WARNING;

	if(VOS_StrCmp(argv[0], "info") == 0)
	{
		ulDevAddr = A0H_1010000X;
		pXcvrArr = &XcvrInfoArr[0];
	}
	else
	{
		ulDevAddr = A2H_1010001X;
		pXcvrArr = &XcvrDiagArr[0];
	}

	for(i=0; pXcvrArr[i].pcName != NULL ; i++)
	{
		vty_out(vty, "  %22s : ", pXcvrArr[i].pcName);
		if(0 == pXcvrArr[i].cType)
		{
			for(j=0; j<pXcvrArr[i].cLen; j++)
			{
				gwRet = PAS_read_i2c_register(PonPortIdx,ulDevAddr, pXcvrArr[i].cAddr+j, &ucValue);
				if(PAS_EXIT_OK == gwRet)
					vty_out(vty, "%02x", ucValue);
				else
					vty_out(vty, "--");
			}
			vty_out(vty, "\r\n");
		}
		else
		{
			union byte_short_selfdef GetAddr;
			pString = &cValueArr[0];
			for(j=0; j<pXcvrArr[i].cLen; j++)
			{
				gwRet = PAS_read_i2c_register(PonPortIdx,ulDevAddr, pXcvrArr[i].cAddr+j, &(GetAddr.sshort));
				if(PAS_EXIT_OK  != gwRet)
					cValueArr[j] = 0x20;
				else 
					cValueArr[j] = GetAddr.sbyte[1];
			}
			cValueArr[j] = '\0';
			vty_out(vty, "%s\r\n", pString);
		}
	}

	return CMD_SUCCESS;
}
#endif

DEFUN(pon_online_sfp_fun,
	pon_online_sfp_cmd,
	"show sfp-online uplink",
	SHOW_STR
	"online optical module info\n"
	"olt uplink\n"
	)
{

	int sfp_sel, max_uplink_sfp, slotno, offset, PonPortIdx;
	long /*val,*/ ret = -1;
	LONG max_uplink_slotno = 1, length = 0;
	UCHAR value, supportflag = 0, CheckFlag = 1, wavelength[2] = {0};
	_XCVR_DATA_ *pXcvrArr;
	unsigned short int data[32];
	Online_Sfp_Info_Get_t Sfp_Info, *pQueueMsg;
	ULONG ulUpOrDown = 0;
	UCHAR temp0[16], *temp1, *temp2; 
	VOS_MemZero(&Sfp_Info, sizeof(Online_Sfp_Info_Get_t));
	
	pXcvrArr=XcvrInfoArr;
	if( SYS_MODULE_ISMASTERSTANDBY( SYS_LOCAL_MODULE_SLOTNO ) )
		return CMD_SUCCESS;
	
	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
		max_uplink_slotno = 1;
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)/* wangysh mod 20110303 6700 support slot2 uplink */
       {   
#ifdef _SUPPORT_8GE_6700_
              if (SYS_CHASSIS_IS_V2_8GE)
		    max_uplink_slotno = 2;
              else
                  max_uplink_slotno = 1;
#else
		max_uplink_slotno = 1;
#endif
       }
	else
		max_uplink_slotno = SYS_CHASSIS_SWITCH_SLOTNUM;

	vty_out(vty,"  Pon Port  Vendor Name      Vendor PN        Vendor SN     Diagnostic Monitoring Type   Wavelength    DataCode  Optical Power  State\r\n");
			vty_out(vty,"  ------------------------------------------------------------------------------------------------------------------------------------------\r\n");
	
	for( slotno=1; slotno<=max_uplink_slotno; slotno++ )
	{
		max_uplink_sfp = GetMAXUplinkPort(slotno);
		if( max_uplink_sfp == 0 )
			continue;
		if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN )
		{
			max_uplink_sfp = 2 ;
		}
#if 0
		if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
			max_uplink_sfp = 4 ;  
		/* 问题单14049   现在版本暂时不支持10 GE 光模块信息的读取 */
#endif	
		if(!SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
		{
			for( sfp_sel=0; sfp_sel<max_uplink_sfp; sfp_sel++ )
			{		
				if((GetOltType() == V2R1_OLT_GFA6900M) 
					&& __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
					&& sfp_sel != 0)
					continue;
					
				if((__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE 
					&& ( (GetOltType() != V2R1_OLT_GFA6900M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA6900M && sfp_sel == 0 ) ))/*||
					(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_GEM_10GE 
					&& ( (GetOltType() != V2R1_OLT_GFA8000M && sfp_sel == 4 ) || (GetOltType() == V2R1_OLT_GFA8000M && sfp_sel == 0 ) ))*/
					|| (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XET) || (__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_4XET))
				{
#if 0
					if( Check_Sfp_Online_6900_10GE( slotno, sfp_sel ) == FALSE)
					{
						CheckFlag = 0;
						Sfp_Info.Online = 0 ;
						continue;
					}
#else
					if(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW || __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
					{
						if( TRUE == i2c_data_get( g_PhysicalSlot[slotno],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /* modified by duzhk for 15611*/
						{
							if( value & 1<<sfp_sel )
							{
								CheckFlag = 0;
								Sfp_Info.Online = 0 ;
								continue;
							}
							else
							{
								CheckFlag = 1;
								Sfp_Info.Online = 1 ;
							}
						}
						else
						{
							continue;
						}
					}

					
					else
					{
						if( FALSE == Check_Sfp_Online_10GE( slotno ))
						{
							CheckFlag = 0;
							Sfp_Info.Online = 0 ;
							continue;
						}
						else
						{	
							CheckFlag = 1;
							Sfp_Info.Online = 1;
						}
					}
#endif

					if(CheckFlag == 1)
					{	
						if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA6900_GEM_10GE)
						{
							ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[59].cAddr, data,XFPInfo[59].cLen);
							VOS_MemCpy(Sfp_Info.Vendorname, data,XFPInfo[59].cLen);
							Sfp_Info.NameEnd = '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[62].cAddr, data,XFPInfo[62].cLen);
							VOS_MemCpy(Sfp_Info.VendorPN, data,XFPInfo[62].cLen);
							Sfp_Info.PnEend = '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[64].cAddr, data,XFPInfo[64].cLen);
							VOS_MemCpy(Sfp_Info.Wavelength, data,XFPInfo[64].cLen);
							wavelength[0] = (UCHAR)((((int)(( Sfp_Info.Wavelength[0] << 8 ) + Sfp_Info.Wavelength[1]))/20)/256);
							wavelength[1] = (UCHAR)((((int)(( Sfp_Info.Wavelength[0] << 8 ) + Sfp_Info.Wavelength[1]))/20)%256);
							VOS_MemCpy(Sfp_Info.Wavelength,wavelength,2);
							Sfp_Info.WaveEnd= '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[69].cAddr, data,XFPInfo[69].cLen);
							VOS_MemCpy(Sfp_Info.VendorSN, data,XFPInfo[69].cLen);
							Sfp_Info.SnEnd = '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[70].cAddr, data,XFPInfo[70].cLen);
							VOS_MemCpy(Sfp_Info.DataCode, data,XFPInfo[70].cLen);
							Sfp_Info.DataEnd= '\0';

							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP, XFPInfo[71].cAddr, data,XFPInfo[71].cLen);
							VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XFPInfo[71].cLen);
						}
						else
						{
							ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
							VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
							Sfp_Info.NameEnd = '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
							VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
							Sfp_Info.PnEend = '\0';

							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
							VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
							Sfp_Info.WaveEnd= '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
							VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
							Sfp_Info.SnEnd = '\0';
							
							ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
							VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
							Sfp_Info.DataEnd= '\0';

		                			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
							VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
						}
						IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+1), &ulUpOrDown);
						VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+1);
				
#if 0
					supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
					if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
					if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
						temp1 = "Support ";
					else
						temp1 = "Not Support ";
#else
						temp1 = "Support ";
#endif
						if(ulUpOrDown == 0)
							temp2 = "not working ";
						else
							temp2 = "working ";
									
						vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
											Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
					}				
				}


				else if(__SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA8000_8XETA1)
				{
										
						if( TRUE == i2c_data_get( g_PhysicalSlot[slotno],I2C_BASE_GPIO+sfp_sel,0,&value,I2C_RW_LEN) ) /*暂时定为0x50*/
						{
							if( value & 1<<sfp_sel )
							{
								CheckFlag = 0;
								Sfp_Info.Online = 0 ;
								continue;
							}
							else
							{
								CheckFlag = 1;
								Sfp_Info.Online = 1 ;
							}
						}
						else
						{
							continue;
						}

					if( CheckFlag == 1)
					{
						Sfp_Info.Online = 1;
						
						ret = i2c_data_get2( g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
						VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
						Sfp_Info.NameEnd = '\0';
						
						ret = i2c_data_get2(  g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
						VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
						Sfp_Info.PnEend = '\0';

						ret = i2c_data_get2(  g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
						VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
						Sfp_Info.WaveEnd= '\0';
						
						ret = i2c_data_get2(  g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
						VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
						Sfp_Info.SnEnd = '\0';
						
						ret = i2c_data_get2(  g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
						VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
						Sfp_Info.DataEnd= '\0';

	                 			ret = i2c_data_get2(  g_PhysicalSlot[slotno], sfp_sel+1,0x50, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
						VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
					    
						IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+1), &ulUpOrDown);

						VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+1);

						supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
						if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
						if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
							temp1 = "Support ";
						else
							temp1 = "Not Support ";
								
						if(ulUpOrDown == 0)
							temp2 = "not working ";
						else
							temp2 = "working ";
									
						vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
										Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
						
					}

				}
				
				else
				{
					offset = sfp_sel%SFP_OFFSET;

					CheckFlag = 1;

					if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6100_MAIN 
					|| __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA_SW)
					{
						if( Check_Sfp_Online_6100( slotno, sfp_sel ) == FALSE)
						{
							CheckFlag = 0;
							UplinkPortMeteringInfo[slotno*MAXUPLINKPORT+sfp_sel].powerMeteringSupport = NOT_SUPPORT_SFF8472;
							continue;
						}
					}
				
					if((__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW|| __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW) 
						&&  !SYS_MODULE_IS_SWITCH_PON(slotno) )
					{
						if( FALSE == i2c_data_get(g_PhysicalSlot[slotno],I2C_BASE_GPIO+offset,0,&value,I2C_RW_LEN) )
							continue;
						if( value & 1<<offset )
							CheckFlag = 0;
					}
					if( SYS_MODULE_IS_UPLINK_PON(slotno) )
					{
						PonPortIdx = GetPonPortIdxBySlot( slotno, sfp_sel+1 );
						length = sizeof(Online_Sfp_Info_Get_t);
						pQueueMsg = VOS_Malloc(length, MODULE_RPU_PON_MON);
						if(pQueueMsg == NULL)
						{
							VOS_ASSERT( 0 );
							return VOS_ERROR;
						}
						
						VOS_MemZero(pQueueMsg, length);
						
						ret = Fetch_Sfp_Online_State( PonPortIdx, (VOID *)pQueueMsg, length, 2 );
							
						if(VOS_OK== ret) 
						{	
							if( pQueueMsg->Online == 0) 
							{
								VOS_Free( pQueueMsg );
								continue;
							}
							else
							{
								VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+1);
								IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+1), &ulUpOrDown);
								if(ulUpOrDown == 0)
									temp2 = "not working ";
								else
									temp2 = "working ";
								if( pQueueMsg->Reserved == 1)
									temp1 = "Support ";
								else
									temp1 = "Not Support ";
								
								vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, pQueueMsg->Vendorname,
										pQueueMsg->VendorPN, pQueueMsg->VendorSN,pQueueMsg->Diagnostic_Monitoring_Type,pQueueMsg->Wavelength[0]*256+pQueueMsg->Wavelength[1],pQueueMsg->DataCode, temp1, temp2);
							}
						}
						VOS_Free( pQueueMsg );
						continue;
					}
					
					if( CheckFlag == 1)
					{
						Sfp_Info.Online = 1;
						
						ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
						VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
						Sfp_Info.NameEnd = '\0';
						
						ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
						VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
						Sfp_Info.PnEend = '\0';

						ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
						VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
						Sfp_Info.WaveEnd= '\0';
						
						ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
						VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
						Sfp_Info.SnEnd = '\0';
						
						ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
						VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
						Sfp_Info.DataEnd= '\0';

	                 			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
						VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
					    
						IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+1), &ulUpOrDown);

						VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+1);

						supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
						if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
						if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
							temp1 = "Support ";
						else
							temp1 = "Not Support ";
								
						if(ulUpOrDown == 0)
							temp2 = "not working ";
						else
							temp2 = "working ";
									
						vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
										Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
						
					}
				}
			}
		}
		else
		{	
			/*8100的EPON和GPON读取在为信息的寄存器是不一样的，另外，千兆口的偏移地址也是不一样的*/
			/*epon的在为信息寄存器地址为0x09,gpon为0x100000c.epon千兆口偏移地址为0，GPON为0xd0*/
		    if(SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			{
				ReadCPLDReg(GFA8100_UPLINK_SFP_STATE, &value);

			   for( sfp_sel=0; sfp_sel<2; sfp_sel++ )
			   {
				  if( !(value & 1<<sfp_sel) )
				  {
					Sfp_Info.Online = 1;
					
					ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
					VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
					Sfp_Info.NameEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
					VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
					Sfp_Info.PnEend = '\0';

					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
					VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
					Sfp_Info.WaveEnd= '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
					VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
					Sfp_Info.SnEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
					VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
					Sfp_Info.DataEnd= '\0';

                 			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_SFP+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
					VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
				    
					IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+19), &ulUpOrDown);

					VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+19);

					supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel+2, 0) ; 
#if 0
					if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
					if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
						temp1 = "Support ";
					else
						temp1 = "Not Support ";
							
					if(ulUpOrDown == 0)
						temp2 = "not working ";
					else
						temp2 = "working ";
								
					vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
									Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
					
				}
			}

			for( sfp_sel=0; sfp_sel<2; sfp_sel++ )
			{
				if( !(value & 1<<(sfp_sel+2)) )
				{
					Sfp_Info.Online = 1;
					
					ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
					VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
					Sfp_Info.NameEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
					VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
					Sfp_Info.PnEend = '\0';

					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
					VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
					Sfp_Info.WaveEnd= '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
					VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
					Sfp_Info.SnEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
					VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
					Sfp_Info.DataEnd= '\0';

                 			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
					VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
				    
					IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+17), &ulUpOrDown);

					VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+17);

					supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
					if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
					if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
						temp1 = "Support ";
					else
						temp1 = "Not Support ";
							
					if(ulUpOrDown == 0)
						temp2 = "not working ";
					else
						temp2 = "working ";
								
					vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
									Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
					
				}
			}
				
			}
			
			else if(SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
			{
				/*gpon的寄存器地址和epon不一样，分开读*/
				ReadCPLDReg(GFA8100_UPLINK_SFP_GPON_STATE, &value);

				for( sfp_sel=0; sfp_sel<2; sfp_sel++ )
				{
				if( !(value & 1<<sfp_sel) )
				{
					Sfp_Info.Online = 1;
					
					ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
					VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
					Sfp_Info.NameEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
					VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
					Sfp_Info.PnEend = '\0';

					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
					VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
					Sfp_Info.WaveEnd= '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
					VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
					Sfp_Info.SnEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
					VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
					Sfp_Info.DataEnd= '\0';

                 			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_10G_SFP+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
					VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
				    
					IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+17), &ulUpOrDown);

					VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+17);

					supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel, 0) ; 
#if 0
					if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
					if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
						temp1 = "Support ";
					else
						temp1 = "Not Support ";
							
					if(ulUpOrDown == 0)
						temp2 = "not working ";
					else
						temp2 = "working ";
								
					vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
									Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
					
				}
			}

			for( sfp_sel=0; sfp_sel<2; sfp_sel++ )
			{
				if( !(value & 1<<(sfp_sel+2)) )
				{
					Sfp_Info.Online = 1;
					
					ret = i2c_data_get( g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[13].cAddr, data,XcvrInfoArr[13].cLen);
					VOS_MemCpy(Sfp_Info.Vendorname, data,XcvrInfoArr[13].cLen);
					Sfp_Info.NameEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[16].cAddr, data,XcvrInfoArr[16].cLen);
					VOS_MemCpy(Sfp_Info.VendorPN, data,XcvrInfoArr[16].cLen);
					Sfp_Info.PnEend = '\0';

					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[18].cAddr, data,XcvrInfoArr[18].cLen);
					VOS_MemCpy(Sfp_Info.Wavelength, data,XcvrInfoArr[18].cLen);
					Sfp_Info.WaveEnd= '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[24].cAddr, data,XcvrInfoArr[24].cLen);
					VOS_MemCpy(Sfp_Info.VendorSN, data,XcvrInfoArr[24].cLen);
					Sfp_Info.SnEnd = '\0';
					
					ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[25].cAddr, data,XcvrInfoArr[25].cLen);
					VOS_MemCpy(Sfp_Info.DataCode, data,XcvrInfoArr[25].cLen);
					Sfp_Info.DataEnd= '\0';

                 			ret = i2c_data_get(  g_PhysicalSlot[slotno], I2C_BASE_1G_SFP_16GPON+sfp_sel, XcvrInfoArr[26].cAddr, data,XcvrInfoArr[26].cLen);
					VOS_MemCpy(Sfp_Info.Diagnostic_Monitoring_Type, data,XcvrInfoArr[26].cLen);
				    
					IFM_GetIfStatusApi(userSlot_userPort_2_Ifindex(slotno, sfp_sel+19), &ulUpOrDown);

					VOS_Sprintf(temp0,"%d/%d",slotno,sfp_sel+19);

					supportflag = UplinkSFPDiagnosticMonitorType(slotno, sfp_sel+2, 0) ; 
#if 0
					if(UplinkPortMeteringInfo[slotno*MAXUPLINKPORT +sfp_sel].powerMeteringSupport != NOT_SUPPORT_SFF8472 )
#else
					if( NOT_SUPPORT_SFF8472 != supportflag )
#endif
						temp1 = "Support ";
					else
						temp1 = "Not Support ";
							
					if(ulUpOrDown == 0)
						temp2 = "not working ";
					else
						temp2 = "working ";
								
					vty_out(vty,"  %-9s %-16s %-16s %-13s %-25x %-4d          %-9s %-14s %-12s\r\n", temp0, Sfp_Info.Vendorname,
									Sfp_Info.VendorPN, Sfp_Info.VendorSN,Sfp_Info.Diagnostic_Monitoring_Type,Sfp_Info.Wavelength[0]*256+Sfp_Info.Wavelength[1],Sfp_Info.DataCode, temp1, temp2);
					
				}
			}
		}	
		}
	}

	return VOS_OK;
	
}

#define PonPower_Max_SlotNum  (14+2)   /* 就是这样子 */

static long SlotParseList(CHAR * list,ULONG *slot, int slotNum)
{
	char *token,*p,ch;
	ULONG startId,endId,id,counter;
	token=NULL;
	ch=id=startId=endId=counter=0;
	
	if(NULL==list)
	{
		return VOS_ERROR;
	}
	else
	{
		p=(char *)list;
	}
	do
	{
		token=StrToken(&p,",-",&ch);
		if(token)
		{
			if(ch==',')
			{
				if(0!=vos_isdigit(*token))
				{
					id=atoi(token);
					if( id < slotNum-1 && id >= 1 )
					{
						slot[id]=id;
						counter++;
					}
					else
					{
						return VOS_ERROR;
					}
				}
			}
			else if(ch=='-')
			{
				if(0!=vos_isdigit(*token))
				{
					startId=atoi(token);
					if(startId<1||startId>=slotNum-1)
					{
						return VOS_ERROR;
					}
					token=StrToken(&p,",",&ch);
					if(NULL==token)
					{
						return VOS_ERROR;
					}	
					if(0!=vos_isdigit(*token))
					{
						endId=atoi(token);
						if(endId>=slotNum-1 ||endId<1)
						{
							return VOS_ERROR;
						}
					}
					else
					{
						return VOS_ERROR;
					}
					if(startId>endId)
					{
						ULONG _temp;
						_temp=startId;
						startId=endId;
						endId=_temp;
					}
					for(id = startId;(id<slotNum-1)&&(id<=endId); id++)
					{
						slot[id]=id;
						counter++;
					}
				}
				else
				{
					return VOS_ERROR;
				}
			}
			else if(ch == '\0')
			{
				if(0!=vos_isdigit(*token))
				{
					id=atoi(token);
					if(id<1||id>=slotNum-1)
					{
						return VOS_ERROR;
					}
					slot[id]=id;
					counter++;
				}
				break;
			}
			else
			{
				return VOS_ERROR;
			}
		}
		else
		{
			if(0!=vos_isdigit(*p))
			{
				id=atoi(p);
				if(id<1 ||id>=slotNum-1)
				{
					return VOS_ERROR;
				}
				slot[id] = id;
				counter++;
			}
			else
			{
				return VOS_ERROR;
			}
		}		
	}
	while(ch==',');
	return counter;
}


static long parseSlotList(CHAR * list,ULONG *slotList, int slotNum)
{
	ULONG i,j,ret,len,counter;
	ULONG *slot=NULL;
	
	if(list == NULL)
		return VOS_ERROR;
		
	i=j=ret=len=counter=0;
	len=strlen(list);
	if(0==len)
	{
		return VOS_ERROR;
	}
	/*判断输入合法性*/
	while(counter<len)
	{
		if( (list[counter]>='0'&&list[counter]<='9') ||(list[counter]==',')||(list[counter]=='-'))
		{
			counter++;
		}
		else
		{
			return VOS_ERROR;
		}
	}

	slot = VOS_Malloc(slotNum*sizeof(ULONG),MODULE_RPU_PON_MON);
	if(NULL == slot)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	VOS_MemZero(slot,slotNum*sizeof(ULONG));

	ret=SlotParseList(list,slot, slotNum);
	if(( ret<1 )||( ret >= slotNum-1))
	{
		VOS_Free(slot);
		slot=NULL;
		return VOS_ERROR;
	}

	for(i=1,j=0;(j<ret)&&(i<slotNum-1);i++)
	{
		if(slot[i])
		{
			slotList[j++]=slot[i];
		}
	}
	slotList[ret]=0;

	VOS_Free(slot);

	return ret;
}

DEFUN(pon_online_sfp_pon_fun,
	pon_online_sfp_pon_cmd,
	"show sfp-online pon {<slotlist>}*1",
	SHOW_STR
	"online optical module info\n"
	"pon card\n"
	"pon card slot list number"
	)
{
	unsigned int ulSlot = 0, ulPort = 0, pBufLen ;
	/*nsigned int PortNum, i, PonPortIdx,counter = 0;*/
	UCHAR *pBuf, *buffer ; /* 应该与PonPower_Max_SlotNum 一致*/
	ULONG *SlotList, *SlotList2, *list ;
	long ret = VOS_ERROR, Flag = 0;
	UCHAR tmp[16], portlist_str[256];

	if(argc == 1)
	{
		VOS_MemZero(portlist_str, sizeof(portlist_str));
		
		SlotList = VOS_Malloc( PonPower_Max_SlotNum*sizeof(ULONG), MODULE_RPU_PON_MON);
		if( NULL == SlotList)
		{
			VOS_ASSERT(0);
			return CMD_SUCCESS;
		}
		SlotList2 = list = SlotList;
		VOS_MemZero(SlotList, PonPower_Max_SlotNum*sizeof(ULONG));
		
		ret=parseSlotList( argv[ 0 ], SlotList, PonPower_Max_SlotNum );
		if(VOS_ERROR == ret)
		{
			vty_out( vty, "  %% Input a invalid slot list.\r\n" );
			VOS_Free( list );
			return CMD_WARNING ;
		}

		while( *SlotList2 )
		{
			ulSlot = *SlotList2 ;

			if( RERROR == SlotCardIsPonBoard(ulSlot) )
			{
				if(PonCardSlotRangeCheckByVty(ulSlot,vty) != ROK)
				{
					vty_out( vty, "  %% Input a invalid slot list.\r\n" );
					VOS_Free( list );
					return CMD_WARNING;
				}
				VOS_MemZero(tmp, sizeof(tmp));
				VOS_Sprintf( tmp, "%s%d", ((portlist_str[0] == 0) ? "" : ","),ulSlot );
				VOS_StrCat( portlist_str, tmp );
				Flag++;
			}
#if 0
			else
			{
				PortNum = Slot_Get_PortNum( ulSlot );
				for( i = 1 ; i <= PortNum ; i++ )
				{
					PonPortIdx = GetPonPortIdxBySlot( ulSlot, i );
					if( Sfp_Pon_Online_Flag[PonPortIdx] == 1 )
					{
						counter++;
						break;
					}
				}
			}
#endif
			SlotList2++;
		}

		if( Flag > 0 )
		{
			vty_out(vty, "  Slot %s is not pon card !\r\n", portlist_str );
			/*VOS_Free( list );
			return CMD_WARNING ;*/
		}

		if(Flag != ret /* && counter != 0*/)
		{
			vty_out(vty,"  Pon Port  Vendor Name      Vendor PN        Vendor SN     Diagnostic Monitoring Type   Wavelength    DataCode  Optical Power  State\r\n");
			vty_out(vty,"  ------------------------------------------------------------------------------------------------------------------------------------------\r\n");
		}
		
		pBufLen = 16*sizeof(Online_Sfp_Info_Get_t)+1;
		pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
		if(pBuf == NULL)
		{
			VOS_ASSERT( 0 );
			VOS_Free( list );
			return CMD_WARNING;
		}
		VOS_MemZero(pBuf, pBufLen);
		buffer = pBuf;
			
		while( *SlotList )
		{
			ulSlot = *SlotList ;

			if( RERROR == SlotCardIsPonBoard(ulSlot) )
			{
				SlotList++;
				continue;
			}
			if( SYS_MODULE_IS_UPLINK_PON(ulSlot) && !SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
				ulPort = 5;
			else
				ulPort = 1;
			
			Fetch_Online_Sfp_Pon(ulSlot,ulPort,(VOID *)pBuf,pBufLen,vty );

			SlotList++;
		}

		VOS_Free( list );
		VOS_Free( pBuf);
	}
	else
	{
		Flag = 0;
		
		for(ulSlot = PONCARD_FIRST; ulSlot <= PONCARD_LAST; ulSlot++ )
		{
			if( ROK == SlotCardIsPonBoard(ulSlot) )
			{
				Flag++;
			}
		}
		
		if(Flag != 0)
		{
			vty_out(vty,"  Pon Port  Vendor Name      Vendor PN        Vendor SN     Diagnostic Monitoring Type   Wavelength    DataCode  Optical Power  State\r\n");
			vty_out(vty,"  ------------------------------------------------------------------------------------------------------------------------------------------\r\n");
		}
		else
		{
			vty_out(vty, "  There is no pon card !\r\n");
			return CMD_WARNING ;
		}
		
		pBufLen = 16*sizeof(Online_Sfp_Info_Get_t)+1;
		pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
		if(pBuf == NULL)
		{
			VOS_ASSERT( 0 );
			return CMD_WARNING;
		}
		VOS_MemZero(pBuf, pBufLen);
		buffer = pBuf;

		ulSlot = ulPort = 0;
		Fetch_Online_Sfp_Pon(ulSlot,ulPort,(VOID *)pBuf,pBufLen,vty );
		
		VOS_Free( pBuf);
	}
	
	return CMD_SUCCESS;
	
}

DEFUN(pon_show_transceiver_info,
	pon_show_transceiver_info_cmd,
	"show optical-power [info|diag]",
	SHOW_STR
	"Optical transceiver info\n"
	"Optical transceiver vendor info\n"
	"Optical transceiver diag parameters\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuid;
	LONG lRet;
	int PonPortIdx;
	UCHAR *pBuf = NULL;
	int iRlt,pBufLen;
	
	lRet = PON_GetSlotPortOnu( ( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if( (PonPortIdx = checkPonPortIndex(vty, ulSlot, ulPort)) == RERROR )	/* modified by xieshl 20101227, 问题单11521 */
		return CMD_WARNING;

	if( Sfp_Pon_Online_Flag[ PonPortIdx ] == 0 )
	{
		vty_out(vty," The optical module is not online !\r\n");
		return VOS_OK;
	}
	/*(VOS_OK == RPU_SendCmd2OpticalPower(show_optical_power_XcvrInfoArr,PonPortIdx,argv[1],4,ulSlot,ulPort,0,0))
	{
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/
	pBufLen = 128;
	pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);
	
	iRlt = Fetch_Optical_Power_XcvrData(ulSlot,ulPort,argv[0],(void *)pBuf/*(void *)vty*/);

	if(VOS_OK == iRlt)
	{
		Show_OpticalPower_XcvrInfoArr_FromPon( ulSlot, ulPort, pBuf, pBufLen,argv[0],vty/*(struct vty *)pointer*/);
	}
	VOS_Free( pBuf);

	return CMD_SUCCESS;
	
	/*unsigned long ulSlot, ulPort, ulOnuid;
	LONG lRet;
	
	lRet = PON_GetSlotPortOnu( ( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	return show_sfp_transceiver_info( vty, argc, argv, ulSlot, ulPort );*/
}
DEFUN(config_show_transceiver_info,
	config_show_transceiver_info_cmd,
	"show optical-power <slot/port> [info|diag]",
	SHOW_STR
	"Optical transceiver info\n"
	"input slot/port\n"
	"Optical transceiver vendor info\n"
	"Optical transceiver diag parameters\n"
	)
{
	unsigned long ulSlot, ulPort;
	LONG lRet;
	int PonPortIdx;
	UCHAR *pBuf = NULL;
	int iRlt,pBufLen,online=0;
	UCHAR value,value1,offset;
	
	lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if( (PonPortIdx = checkPonPortIndex(vty, ulSlot, ulPort)) == RERROR )	/* modified by xieshl 20101227, 问题单11521 */
		return CMD_WARNING;
	
	/*(VOS_OK == RPU_SendCmd2OpticalPower(show_optical_power_XcvrInfoArr,PonPortIdx,argv[1],4,ulSlot,ulPort,0,0))
	{
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/
	pBufLen = 128;
	pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
	if(pBuf == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero(pBuf, pBufLen);
	
	/**add by yanjy 2016-10*
	*当为8100GPON时，先判断光模块在不在位。
	*/
	offset = (ulPort-1)%8;

	if( SYS_LOCAL_MODULE_TYPE_IS_8100_GPON )
	{
		ReadCPLDReg( CPLD1_RESET_REG2_8000,&value);
		ReadCPLDReg( CPLD1_RESET_REG3_8000,&value1);

		if((ulPort-1)/8)
		{
			if(value1 & (1 << offset))
			{
				online = 1;
			}
		}
		else 
		{
			if(value & (1 << offset))
			{
				online = 1;
			}
		}

		if(0 == online)
			iRlt = Fetch_Optical_Power_XcvrData(ulSlot,ulPort,argv[1],(void *)pBuf/*(void *)vty*/);
			
	}

	else
	{
		iRlt = Fetch_Optical_Power_XcvrData(ulSlot,ulPort,argv[1],(void *)pBuf/*(void *)vty*/);
	}

	if(VOS_OK == iRlt)
	{
		Show_OpticalPower_XcvrInfoArr_FromPon( ulSlot, ulPort, pBuf, pBufLen,argv[1],vty/*(struct vty *)pointer*/);
	}
	VOS_Free( pBuf);

	return CMD_SUCCESS;
	/*return show_sfp_transceiver_info( vty, argc-1, &argv[1], ulSlot, ulPort );*/
}


/* 显示OLT PON 口光模块参数测量结果*/
static int show_sfp_transceiver_meter( struct vty *vty, ULONG ulSlot, ULONG ulPort )
{
	int PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort )/*, PonPortIdx2*/;
	int ret, length;
	/*PON_gpio_line_io_t  direction;
	Sfp_Online_State_t *SfpState = NULL ;*/
	OpticalToPonMsgHead_t *pQueueMsg;
	
	if( PonPortIdx == VOS_ERROR )
		return CMD_WARNING;

	if(GetPonPortOpticalMonitorEnable() == V2R1_DISABLE)   /*问题单13645 */
	{
		vty_out(vty," optical power is disable\r\n");
		return CMD_WARNING;
	}

	if( Sfp_Pon_Online_Flag[ PonPortIdx ] == 0 || PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)    /*问题单 12949 modified by duzhk  2011-10-13 */
	{
		if( __SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO ) == MODULE_E_GFA6100_MAIN )
		{
			if(2 == GetCardIdxByPonChip(PonPortIdx) )
			{
				PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = 0;
				vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
				return VOS_ERROR;
			}	
		}
		
		length = sizeof(OpticalToPonMsgHead_t);
		pQueueMsg = VOS_Malloc(length, MODULE_RPU_PON_MON);
		if(pQueueMsg == NULL)
		{
			VOS_ASSERT( 0 );
			return VOS_ERROR;
		}
		
		VOS_MemZero(pQueueMsg, length);
		
		ret = Fetch_Sfp_Online_State( PonPortIdx, (VOID *)pQueueMsg, length, 0 );
			
		if(VOS_OK== ret) 
		{	
			if( pQueueMsg->PonOnlineFlag == 1 ) /* 表示返回为高电平 */
			{
				vty_out(vty," The Optical Module is not online !\r\n");
				VOS_Free( pQueueMsg );
				return VOS_ERROR;
			}

			if( pQueueMsg->sfpMeterSupport == 0 )
			{
				vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
				VOS_Free( pQueueMsg );
				return CMD_WARNING;
			}
		}
		else
		{
			sys_console_printf("Fetch Sfp Online State Failed !\r\n");
			VOS_Free( pQueueMsg );
			return CMD_WARNING;
		}

		onuLaser_alwaysOn_check_support[PonPortIdx] = pQueueMsg->laseralwaysonsupport ;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = pQueueMsg->transOpticalPower;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature= pQueueMsg->ponTemperature;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = pQueueMsg->ponVoltageApplied ;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = pQueueMsg->ponBiasCurrent;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = pQueueMsg->sfpMeterSupport;
		Sfp_Pon_Online_Flag[PonPortIdx] = pQueueMsg->PonOnlineFlag ;

		VOS_Free( pQueueMsg );
		
	}
	
	if((ret = GetPonPortTemperature(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get temperature info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)+1),(GetPonPortByPonChip(PonPortIdx)+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d Celsius\r\n", XcvrDiagArr[34].pcName, ret);

	if((ret = GetPonPortWorkVoltage(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get working-voltage info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)+1),(GetPonPortByPonChip(PonPortIdx)+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d.%d V\r\n", XcvrDiagArr[35].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );

	if((ret=GetPonPortBiasCurrent(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"get bias-current info from pon %d/%d err(code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)+1),(GetPonPortByPonChip(PonPortIdx)+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d(mA)\r\n", XcvrDiagArr[36].pcName, ret);

	if((ret=GetPonPortTransOpticalPower(PonPortIdx)) == RERROR)
	{
		vty_out(vty,"No tx-power info pon%d/%d (err-code=%d)\r\n", (GetCardIdxByPonChip(PonPortIdx)+1),(GetPonPortByPonChip(PonPortIdx)+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty, "  %s: %d.%d dbm\r\n", XcvrDiagArr[37].pcName, decimal2_integer_part(ret), decimal2_fraction_part(ret) );

	return CMD_SUCCESS;
}

DEFUN(pon_transceiver_meter_show,
	pon_transceiver_meter_show_cmd,
	"show optical-power olt",
	SHOW_STR
	"Optical transceiver info\n"
	"Olt PON\n"
	)
{
	ULONG ulSlot, ulPort, ulOnuid;
	LONG lRet;
	
	lRet = PON_GetSlotPortOnu(( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	return show_sfp_transceiver_meter( vty, ulSlot, ulPort );
}

DEFUN(olt_pon_transceiver_meter_show,
	olt_pon_transceiver_meter_show_cmd,
	"show optical-power olt <slot/port>",
	SHOW_STR
	"Optical transceiver info\n"
	"Olt PON\n"
	"input slot/port\n"
	)
{
	unsigned long ulSlot, ulPort;
	LONG lRet;

	lRet = IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	return show_sfp_transceiver_meter( vty, ulSlot, ulPort );
}

#if 0
/*ONU端光模块参数的显示，用于CLI*/
/*** 1 显示温度***/
ULONG showOnuSFPTemperature(short int PonPortIdx, short int OnuIdx, struct vty *vty)
{
	int ret;
	ULONG slotno = GetCardIdxByPonChip(PonPortIdx);
	ULONG portno = GetPonPortByPonChip(PonPortIdx);
	
	CHECK_ONU_RANGE

	if((ret = GetOnuTemperature(PonPortIdx,OnuIdx)) == RERROR)
	{
		vty_out(vty,"get temperature info from onu %d/%d/%d err(code=%d)\r\n", slotno, portno,(OnuIdx+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty," onu %d/%d/%d temperature: %d C\r\n", slotno, portno, (OnuIdx+1), ret);
	return CMD_SUCCESS;
}

/*** 2 显示工作电压***/
ULONG showOnuSFPVoltage(short int PonPortIdx,short int OnuIdx,struct vty *vty)
{
	int ret;
	ULONG slotno = GetCardIdxByPonChip(PonPortIdx);
	ULONG portno = GetPonPortByPonChip(PonPortIdx);

	CHECK_PON_RANGE
	if((ret = GetOnuWorkVoltage(PonPortIdx,OnuIdx)) == RERROR)
	{
		vty_out(vty,"get working-voltage info from onu %d/%d/%d err(code=%d)\r\n", slotno, portno, (OnuIdx+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty," onu %d/%d/%d working-voltage: %d.%d V\r\n", slotno, portno, (OnuIdx+1), 
			decimal2_integer_part(ret), decimal2_fraction_part(ret) );	
	return CMD_SUCCESS;
}

/*** 3 显示偏置电流***/
ULONG showOnuSFPBias(short int PonPortIdx, short int OnuIdx,struct vty *vty)
{
	int ret;
	ULONG slotno = GetCardIdxByPonChip(PonPortIdx);
	ULONG portno = GetPonPortByPonChip(PonPortIdx);

	CHECK_PON_RANGE
	if((ret = GetOnuBiasCurrent(PonPortIdx,OnuIdx)) == RERROR)
	{
		vty_out(vty,"get bias-current info from onu %d/%d/%d err(code=%d)\r\n", slotno, portno, (OnuIdx+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty," onu %d/%d/%d bias-current: %d(mA)\r\n", slotno, portno, (OnuIdx+1), ret);
	return CMD_SUCCESS;
}

/*** 4 显示发射光功率***/
ULONG showOnuSFPTransPower(short int PonPortIdx, short int OnuIdx,struct vty *vty)
{
	int ret;
	ULONG slotno = GetCardIdxByPonChip(PonPortIdx);
	ULONG portno = GetPonPortByPonChip(PonPortIdx);
	
	CHECK_PON_RANGE

	if((ret = GetOnuTransOpticalPower(PonPortIdx,OnuIdx)) == RERROR)
	{
		vty_out(vty,"No tx-power info onu%d/%d/%d (err-code=%d)\r\n", slotno, portno,(OnuIdx+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty," onu %d/%d/%d tx-power: %d.%d dbm\r\n", slotno, portno, (OnuIdx+1), 
			decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}
/*** 5显示接收光功率***/
ULONG showOnuSFPRcePower(short int PonPortIdx, short int OnuIdx,struct vty *vty)
{
	int ret;
	ULONG slotno = GetCardIdxByPonChip(PonPortIdx);
	ULONG portno = GetPonPortByPonChip(PonPortIdx);
	
	CHECK_PON_RANGE

	if((ret = GetOnuRecvOpticalPower(PonPortIdx,OnuIdx)) == RERROR)
	{
		vty_out(vty,"No rx-power info onu%d/%d/%d (err-code=%d)\r\n", slotno, portno, (OnuIdx+1), ret);
		return CMD_WARNING;
	}
	vty_out(vty," onu%d/%d/%d rx-power: %d.%d dbm\r\n", slotno, portno, (OnuIdx+1), 
			decimal2_integer_part(ret), decimal2_fraction_part(ret) );
	return CMD_SUCCESS;
}
#endif

/* 显示ONU 光模块参数上报结果*/
static int show_onu_sfp_transceiver_meter( struct vty *vty, int argc, char **argv, ULONG ulSlot, ULONG ulPort )
{
	int ret;
	short int PonPortIdx, OnuIdx;
	short int Llid;
	short int count=0, ulOnuId,OnuEntry;
	/*unsigned char content=0;*/
	
	PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return CMD_WARNING;
	
	if(GetOnuOpticalPowerEnable() != V2R1_ENABLE)
	{
		vty_out(vty," optical power for onu is disable\r\n");
		return VOS_ERROR;
	}   /* 问题单 13666 */
	
	BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
 	{
 		count++;
	}
	END_PARSE_ONUID_LIST_TO_ONUID();
	if (count != 0)
	{
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
		{
	 		OnuIdx = (ulOnuId-1);
			OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;			
	 		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	 		{
				/*vty_out(vty, " onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuId);*/
				continue;
			}
			if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
			{
				vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);
				continue;
			}

			Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if(Llid == INVALID_LLID)
				continue;
			if( OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag!=1 )	
			{
				vty_out(vty, " onu %d/%d/%d do not support power-metering report\r\n", ulSlot, ulPort, ulOnuId);
				continue;				
			}
			/*vty_out(vty, "%d flag=%d\r\n", OnuEntry,OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag);	*/		

			if((ret = GetOnuTemperature(PonPortIdx, OnuIdx)) == RERROR)
			{
				vty_out(vty,"get temperature info from onu %d/%d/%d err(code=%d)\r\n", ulSlot, ulPort, ulOnuId, ret);
				continue;
			}
			vty_out( vty, " onu %d/%d/%d\r\n", ulSlot, ulPort, ulOnuId );
			vty_out( vty, "  temperature: %d C\r\n", ret );

			if((ret = GetOnuWorkVoltage(PonPortIdx, OnuIdx)) == RERROR)
			{
				vty_out(vty,"get working-voltage info from onu %d/%d/%d err(code=%d)\r\n", ulSlot, ulPort, ulOnuId, ret);
				continue;
			}
			vty_out( vty, "  working-voltage: %d.%d V\r\n", decimal2_integer_part(ret), decimal2_fraction_part(ret) );	

			if((ret = GetOnuBiasCurrent(PonPortIdx, OnuIdx)) == RERROR)
			{
				vty_out(vty,"get bias-current info from onu %d/%d/%d err(code=%d)\r\n", ulSlot, ulPort, ulOnuId, ret);
				continue;
			}
			vty_out( vty, "  bias-current: %d(mA)\r\n",  ret );

			if((ret = GetOnuTransOpticalPower(PonPortIdx, OnuIdx)) == RERROR)
			{
				vty_out(vty,"No tx-power info onu%d/%d/%d (err-code=%d)\r\n", ulSlot, ulPort, ulOnuId, ret);
				continue;
			}
			vty_out( vty, "  tx-power: %d.%d dbm\r\n", decimal2_integer_part(ret), decimal2_fraction_part(ret) );

			if((ret = GetOnuRecvOpticalPower(PonPortIdx, OnuIdx)) == RERROR)
			{
				vty_out(vty,"No rx-power info onu%d/%d/%d (err-code=%d)\r\n", ulSlot, ulPort, ulOnuId, ret);
				continue;
			}
			vty_out( vty, "  rx-power: %d.%d dbm\r\n\r\n", decimal2_integer_part(ret), decimal2_fraction_part(ret) );
		}
		END_PARSE_ONUID_LIST_TO_ONUID();
	}
	return CMD_SUCCESS;
}

DEFUN(olt_rx_onu_transceiver_meter_show,
	olt_rx_onu_transceiver_meter_show_cmd,
	"show optical-power onu <slot/port> <onuid_list>",
	SHOW_STR
	"Optical onu-transceiver info\n"
	"Onu PON\n"
	"input slot/port\n"
	"the range for onuId is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
	)
{
	unsigned long ulSlot, ulPort;

	if( IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort ) != VOS_OK )
		return CMD_WARNING;
	return show_onu_sfp_transceiver_meter( vty, argc-1, &argv[1], ulSlot, ulPort );
}

DEFUN(pon_rx_onu_transceiver_meter_show,
	pon_rx_onu_transceiver_meter_show_cmd,
	"show optical-power onu <onuid_list>",
	SHOW_STR
	"Optical onu-transceiver info\n"
	"Onu PON\n"
	"input slot/port\n"
	"the range for onuId is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
	)
{
	ULONG ulSlot, ulPort, ulOnuid;
	LONG lRet;

	lRet = PON_GetSlotPortOnu(( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	return show_onu_sfp_transceiver_meter( vty, argc, &argv[0], ulSlot, ulPort );
}

/* 显示OLT PON 光模块接收ONU 光功率测量结果*/
int show_optical_power_meter( struct vty *vty, int argc, char **argv, ULONG ulSlot, ULONG ulPort )
{
	short int PonPortIdx, OnuIdx;
	short int /*Llid,*/onu_status;
	short int /*count=0,*/ ulOnuId;
	long val,OnuEntry;
	int TempVal=0;
	
	PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return CMD_WARNING;

	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
	{
		vty_out(vty," optical power is disable\r\n");
		return VOS_ERROR;
	}   /* 问题单 13270 */
	
	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
		return CMD_WARNING;
	}
	
		vty_out(vty," OnuIdx    Status       rx-power(dbm)    running/down time\r\n" );
		vty_out(vty," ---------------------------------------------------------\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
 		{
	 		OnuIdx = ulOnuId -1;
	 		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	 		{
				/* vty_out(vty, " onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuId); */
				continue;
			}
			/*if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
			{
				continue;
			}
			Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if(Llid == INVALID_LLID)
				continue;*/
			TempVal = getOpticalPowerThreshold(field_olt_monitor_enable,0);
			onu_status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (onu_status < 0) || (onu_status > 5) )
				onu_status = 0;
			val = GetPonPortRecvOpticalPower(PonPortIdx, OnuIdx);
			vty_out(vty," %d/%d/%-2d", ulSlot, ulPort, ulOnuId);
			vty_out(vty,"    %-10s", OnuCurrentStatus[onu_status]);
			if(val == -1000 || onu_status != ONU_OPER_STATUS_UP ||TempVal != V2R1_ENABLE)
				vty_out(vty,"       -        ");
			else
				vty_out(vty,"   %3d.%d        ",decimal2_integer_part(val), decimal2_fraction_part(val) );
			/*if(onu_status == 1)
				vty_out(vty,"    --------");
			else*/
			{
				ULONG /*CurTicks,*/ SysUpDelayed;
				ULONG days, hours,minutes, seconds;
				OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
				/*CurTicks = VOS_GetTick();
				SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

				seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
				SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
				days = SysUpDelayed /V2R1_ONE_DAY;
				hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
				minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
				seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

				vty_out(vty,"    %04u:%02u:%02u:%02u ",days, hours, minutes, seconds );
			}
			vty_out(vty,"\r\n");

		}
		END_PARSE_ONUID_LIST_TO_ONUID()
	return CMD_SUCCESS;
}


int show_optical_power_meter_instant( struct vty *vty, int argc, char **argv, ULONG ulSlot, ULONG ulPort )
{
	short int PonPortIdx, OnuIdx;
	short int Llid,onu_status;
	short int /*count=0,*/ ulOnuId;
	long val,OnuEntry,ret = -1;
	
	PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return CMD_WARNING;

	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
		return CMD_WARNING;
	}

		vty_out(vty," OnuIdx    Status       rx-power(dbm)    running-time\r\n" );
		vty_out(vty," -----------------------------------------------------\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
 		{
	 		OnuIdx = ulOnuId -1;
	 		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	 		{
				/*vty_out(vty, " onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuId);*/
				continue;
			}
			if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
			{
				continue;
			}
			Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if(Llid == INVALID_LLID)
				continue;
			
			onu_status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (onu_status < 0) || (onu_status > 5) )
				onu_status = 0;
			ret = ReadPonPortRecevPower(PonPortIdx,OnuIdx,0,&val);
			vty_out(vty," %d/%d/%-2d", ulSlot, ulPort, ulOnuId);
			vty_out(vty,"    %-10s", OnuCurrentStatus[onu_status]);
			if(val == -1000 || ret == VOS_ERROR)
				vty_out(vty,"       -        ");
			else
				vty_out(vty,"   %3d.%d        ",decimal2_integer_part(val), decimal2_fraction_part(val) );
			/*if(onu_status == 1)
				vty_out(vty,"    --------");
			else*/
			{
				ULONG /*CurTicks,*/ SysUpDelayed;
				ULONG days, hours,minutes, seconds;
				OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
				/*CurTicks = VOS_GetTick();
				SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

				seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
				SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
				days = SysUpDelayed /V2R1_ONE_DAY;
				hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
				minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
				seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

				vty_out(vty,"    %04u:%02u:%02u:%02u ",days, hours, minutes, seconds );
			}
			vty_out(vty,"\r\n");

		}
		END_PARSE_ONUID_LIST_TO_ONUID()
	return CMD_SUCCESS;
}
/* 显示OLT PON 光模块接收ONU 光功率测量结果*/
int show_optical_power_meter_history( struct vty *vty, int argc, char **argv, ULONG ulSlot, ULONG ulPort )
{
	short int PonPortIdx, OnuIdx;
	short int /*Llid,*/onu_status,k;
	short int /*count=0,*/ ulOnuId;
	long val,i,OnuEntry;
	
	PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort );
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return CMD_WARNING;

	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
		return CMD_WARNING;
	}

		/*vty_out(vty," OnuIdx  rx-power(dbm)\r\n" );*/
		vty_out(vty," OnuIdx    Status       rx-power(dbm)                            DownTime\r\n" );
		vty_out(vty," -----------------------------------------------------------------------------\r\n");
		BEGIN_PARSE_ONUID_LIST_TO_ONUID( argv[0], ulOnuId )
 		{
	 		OnuIdx = ulOnuId -1;
	 		if(ThisIsValidOnu(PonPortIdx, OnuIdx) != ROK )
	 		{
				/*vty_out(vty, " onu %d/%d/%d is not exist\r\n", ulSlot, ulPort, ulOnuId);*/
				continue;
			}
			/*if(GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP)
			{
				vty_out(vty, " onu %d/%d/%d is off-line\r\n", ulSlot, ulPort, ulOnuId);
				continue;
			}
			Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
			if(Llid == INVALID_LLID)
				continue;*/
			
			onu_status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
			if( (onu_status < 0) || (onu_status > 5) )
				onu_status = 0;
			
			vty_out(vty," %d/%d/%-2d", ulSlot, ulPort, ulOnuId);
			
			vty_out(vty,"    %-10s", OnuCurrentStatus[onu_status]);

			k = PonRecvOpticalPower_Buffer_Index[PonPortIdx][OnuIdx];
			
			for(i=0;i<READ_I2C_COUNT;i++)
			{
				if( k >=READ_I2C_COUNT)
					k = 0;
				val = PonRecvOpticalPower_Buffer[PonPortIdx][OnuIdx][k];
				vty_out(vty,"   %3d.%d", decimal2_integer_part(val), decimal2_fraction_part(val) );
				k++;
			}
			if(onu_status == 1)
				vty_out(vty,"      -");
			else
			{
				ULONG /*CurTicks,*/ SysUpDelayed;
				ULONG days, hours,minutes, seconds;
				OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
				/*CurTicks = VOS_GetTick();
				SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;

				seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
				SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
				days = SysUpDelayed /V2R1_ONE_DAY;
				hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
				minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
				seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;

				vty_out(vty,"    %04u:%02u:%02u:%02u ",days, hours, minutes, seconds );
			}
			vty_out(vty,"\r\n");

		}
		END_PARSE_ONUID_LIST_TO_ONUID()
	return CMD_SUCCESS;
}

DEFUN(pon_optical_power_meter_show,
	pon_optical_power_meter_show_cmd,
	"show optical-power olt-rx <onuid_list>",
	SHOW_STR
	"show optical power metering\n"
	"Olt PON\n"
	"optical power received from onu,e.g 1,3-5,7, etc. the range for onuId is 1-"INT_TO_STR(MAXONUPERPON)"\n"
	)
{
	unsigned long ulSlot, ulPort, ulOnuid_1;
	LONG lRet;

	lRet = PON_GetSlotPortOnu(( ULONG ) ( vty->index ), &ulSlot, &ulPort, &ulOnuid_1 );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	return show_optical_power_meter( vty, argc, argv, ulSlot, ulPort );
}

DEFUN(config_show_optical_power_meter,
	config_show_optical_power_meter_cmd,
	"show optical-power olt-rx <slot/port> <onuid_list> {[average|instant|history]}*1",
	SHOW_STR
	"show optical power metering\n"
	"Olt PON\n"
	"pon slot/port\n"
	"optical power received from onu,e.g 1,3-5,7, etc. the range for onuId is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)"\n"
	"average power\n"
	"instant power\n"
	"history power\n"
	)
{
	LONG ret = 0,_i = 0;
	unsigned long ulSlot = 0, ulPort = 0;
	ULONG * _pulIfArray = NULL , OnuNum = 0, PonPortIdx = 0;
	UCHAR *pBuf = NULL;
	int iRlt = 0,pBufLen = 0;
	
	if( IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort ) != VOS_OK )
		return CMD_WARNING;

	if( (PonPortIdx = checkPonPortIndex(vty, ulSlot, ulPort)) == RERROR )	/* modified by xieshl 20101227, 问题单11521 */
		return CMD_WARNING;

  	_pulIfArray = V2R1_Parse_OnuId_List(argv[1]);
	if(_pulIfArray != NULL)
	{
		for(_i=0;_pulIfArray[_i]!=0;_i++)
		{
			OnuNum++;
		}
	}
	else
		return CMD_WARNING;

	if( argc < 3 )
		ret = show_optical_power_meter( vty, argc-1, &argv[1], ulSlot, ulPort );
	else
	{
		if( argv[2][0] == 'a' )
			ret = show_optical_power_meter( vty, argc-1, &argv[1], ulSlot, ulPort );
		else if( argv[2][0] == 'i' )
		{
			/*if(VOS_OK == RPU_SendCmd2OpticalPower(show_optical_power_olt_rx_instant,PonPortIdx,(UCHAR *)_pulIfArray,OnuNum*sizeof(ULONG),ulSlot,ulPort,0,0))
			{
				vty_out(vty,"Send the command to Pon card successed !\r\n");
			}
			else
			{
				vty_out(vty,"Send the command to Pon card failed !\r\n");
			}*/
			if(OnuNum > 10)
			{
				vty_out(vty,"The number of the onuid in the onuid_list shouldn't be bigger then ten !\r\n");
				VOS_Free(_pulIfArray);

				return CMD_WARNING;
			}
			
			pBufLen = OnuNum*sizeof(Optical_Power_Rx_Instant_t);
			pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
			if(pBuf == NULL)
			{
				VOS_Free(_pulIfArray);
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
			VOS_MemZero( pBuf, pBufLen);
			
			iRlt = Fetch_Optical_Power_Instant(ulSlot, ulPort, _pulIfArray, OnuNum*sizeof(ULONG),(void *)pBuf);

			if(VOS_OK == iRlt)
			{
				Show_OpticalPower_OltRx_Instant_FromPon( ulSlot, ulPort, pBuf, pBufLen,vty);
			}
			else
			{
				vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
			}
			VOS_Free( pBuf);
			/*ret = show_optical_power_meter_instant( vty, argc-1, &argv[1], ulSlot, ulPort );*/
		}
		else
		{	/*ret = show_optical_power_meter_history( vty, argc-1, &argv[1], ulSlot, ulPort );*/
			/*if(VOS_OK == RPU_SendCmd2OpticalPower(show_optical_power_olt_rx_history,PonPortIdx,(UCHAR *)_pulIfArray,OnuNum*sizeof(ULONG),ulSlot,ulPort,0,0))
			{
				vty_out(vty,"Send the command to Pon card successed !\r\n");
			}
			else
			{
				vty_out(vty,"Send the command to Pon card failed !\r\n");
			}*/
	
			pBufLen = OnuNum*sizeof(Optical_Power_Rx_History_t);
			pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
			if(pBuf == NULL)
			{
				VOS_Free(_pulIfArray);
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
			VOS_MemZero( pBuf, pBufLen);
			
			iRlt = Fetch_Optical_Power_History(ulSlot, ulPort, _pulIfArray, OnuNum*sizeof(ULONG),(void *)pBuf);

			if(VOS_OK == iRlt)
			{
				Show_OpticalPower_OltRx_History_FromPon( ulSlot, ulPort, pBuf, pBufLen, vty );
			}
			VOS_Free( pBuf);
		}	
	}
	VOS_Free(_pulIfArray);
	return ret;
}

ULONG Show_OpticalPower_OltRx_Instant_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,ULONG pBufLen,struct vty *vty)
{
	Optical_Power_Rx_Instant_t *recv_instant, *recv_instant2;
	int j,number,OnuIdx;
	short int PonPortIdx;
	short int Llid,onu_status;

	long val,OnuEntry;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return VOS_ERROR;

	recv_instant = (Optical_Power_Rx_Instant_t *)pBuf;
	number = pBufLen/sizeof(Optical_Power_Rx_Instant_t);
	
	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE)
	{
		vty_out(vty," optical power is disable\r\n");
		return VOS_ERROR;
	}
	
	vty_out(vty,"\r\n OnuIdx    Status       rx-power(dbm)     running-time\r\n" );
	vty_out(vty," -----------------------------------------------------\r\n");
	for(j = 0 ; j < number ; j++)
	{
		recv_instant2 = recv_instant;
		OnuIdx = recv_instant->OnuIdx;

	 	if(recv_instant->NotExistFlag != 0 )
	 	{
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}

		if(recv_instant->OnuStatus != ONU_OPER_STATUS_UP)
		{
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}
		Llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
		if(Llid == INVALID_LLID)
		{
			recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
			continue;
		}
		onu_status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
		if( (onu_status < 0) || (onu_status > 5) )
			onu_status = 0;
		
		vty_out(vty," %d/%d/%-2d", ulSlot, ulPort, OnuIdx+1);
		
		vty_out(vty,"    %-10s", OnuCurrentStatus[onu_status]);

		val = recv_instant->Recv_Instant_Buffer;
		if(val == -1000)
			vty_out(vty,"       -        ");
		else
			vty_out(vty,"    %3d.%d        ", decimal2_integer_part(val), decimal2_fraction_part(val) );

		/*if(onu_status == 1)
			sys_console_printf("      -");
		else*/
		{
			ULONG /*CurTicks,*/ SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;
			vty_out(vty,"    %04u:%02u:%02u:%02u ",days, hours, minutes, seconds );
		}
		vty_out(vty,"\r\n");
		recv_instant = (Optical_Power_Rx_Instant_t *)(recv_instant2+1);
	}
	return VOS_OK;
}


ULONG Show_OpticalPower_XcvrInfoArr_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,
	ULONG pBufLen,UCHAR *flag, struct vty * vty)
{
	ULONG ulDevAddr;
	_XCVR_DATA_  *pXcvrArr;
	short int	PonPortIdx;
	short int	ucValue;
	int i,j;
	unsigned char	cValueArr[16]={0};
	unsigned char *pString;
	/*unsigned char GetlDevAddr[128];*/
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
		return RERROR;

	if(VOS_MemCmp(flag, "info", 4) == 0)
	{
		ulDevAddr = A0H_1010000X;
		pXcvrArr = &XcvrInfoArr[0];
	}
	else
	{
		ulDevAddr = A2H_1010001X;
		pXcvrArr = &XcvrDiagArr[0];
	}
	vty_out(vty,"\r\n");
	for(i=0; pXcvrArr[i].pcName != NULL ; i++)
	{
		vty_out(vty,"  %22s : ", pXcvrArr[i].pcName);
		if(0 == pXcvrArr[i].cType)
		{
			for(j=0; j<pXcvrArr[i].cLen; j++)
			{
				ucValue = pBuf[pXcvrArr[i].cAddr+j];
				vty_out(vty,"%02x", ucValue);
			}
		}
		else
		{
			pString = &cValueArr[0];
			VOS_MemZero(cValueArr,sizeof(cValueArr));
			for(j=0; j<pXcvrArr[i].cLen; j++)
			{
				if(pBuf[pXcvrArr[i].cAddr+j] )
				{
					if(pBuf[pXcvrArr[i].cAddr+j] != 0x20)
					{
						*pString = pBuf[pXcvrArr[i].cAddr+j];
						pString++;
					}
				}
			}
			/*sys_console_printf("\r\ni = %d, j= %d, pXcvrArr[i].cLen is %d",i,j,pXcvrArr[i].cLen);*/
			cValueArr[j] = '\0';
			vty_out(vty,"%s", cValueArr);
		}
		vty_out(vty,"\r\n");
	}
	return VOS_OK;
}

ULONG Show_OpticalPower_OnuLaser_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,
	ULONG pBufLen,UCHAR *flag, struct vty * vty)
{
	int i, count = 0,PonPortIdx= 0;
	LONG record_t[13*16];/*4 means PONPORTPERCARD; This variable should be bigger then MAXPON . */
	int slot,port;
	VOS_MemCpy( record_t, pBuf, MAXPON*sizeof(LONG));
	
	if(VOS_MemCmp(flag, "status", 6) == 0)  /*此时传入的slot和port都为0 */
	{
		for( i = 0; i<MAXPON; i++ )
		{
			if(record_t[i] != 0)
			{
				count++;

				slot = GetCardIdxByPonChip(i);
				port = GetPonPortByPonChip(i);

				if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
				{
					vty_out(vty,"\r\n onu %d/%d/0 laser always on, rxp=%d.%d\r\n", slot, port,
							decimal2_integer_part(record_t[i]),
							decimal2_fraction_part(record_t[i]));
				}
				else
				{
					vty_out(vty,"\r\n onu %d/%d/0 laser always on, rxp=%d.%d\r\n", slot, port,
						decimal2_integer_part(record_t[i]),
						decimal2_fraction_part(record_t[i]));
				}
			}
		}
		if( count == 0 )
		{
			/*sys_console_printf(" Slot %d no onu laser always on\r\n",ulSlot);*/
		}
	}
	else
	{
		vty_out(vty,"\r\n");
		if(V2R1_DISABLE == onuLaser_alwaysOn_Enable)
		{
			vty_out(vty,"onu laser always on detection is disabled!\r\n");
			return CMD_SUCCESS;
		}

		PonPortIdx = GetPonPortIdxBySlot(ulSlot, ulPort);   /* problem num: 18024 */
		
		if( onuLaser_alwaysOn_check_support[PonPortIdx] == V2R1_DISABLE )
		{
			vty_out(vty,"optical power monitor not support!\r\n");
			return CMD_SUCCESS;
		}

#if 0       /* 问题单: 16962  */
		if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && record_t[ulPort-1] != 0)
		{
			vty_out(vty,"Pon %d/%d optical always on value : %d.%d\r\n",ulSlot,ulPort,
					decimal2_integer_part(record_t[ulPort-1]), 
					decimal2_fraction_part(record_t[ulPort-1]) );
		}
		else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && record_t[PonPortIdx] != 0)
		{
			vty_out(vty,"Pon %d/%d optical always on value : %d.%d\r\n",ulSlot,ulPort,
					decimal2_integer_part(record_t[PonPortIdx]), 
					decimal2_fraction_part(record_t[PonPortIdx]) );
		}
		else
			vty_out(vty,"Pon %d/%d optical always on value : 0 \r\n",ulSlot,ulPort);
#else
		if(record_t[ulPort-1] != 0 )
		{
			vty_out(vty,"Pon %d/%d optical always on value : %d.%d\r\n",ulSlot,ulPort,
					decimal2_integer_part(record_t[ulPort-1]), 
					decimal2_fraction_part(record_t[ulPort-1]) );
		}
		else
			vty_out(vty,"Pon %d/%d optical always on value : 0 \r\n",ulSlot,ulPort);
#endif

	}
	
	return VOS_OK;
}


ULONG Show_OpticalPower_OltRx_History_FromPon(USHORT ulSlot, USHORT ulPort,UCHAR *pBuf,ULONG pBufLen,struct vty* vty)
{
	Optical_Power_Rx_History_t *recv_history, *recv_history2;
	int i,j,number,OnuIdx;
	short int PonPortIdx;
	short int /*Llid,*/onu_status;
	long val,OnuEntry;
	
	PonPortIdx = GetPonPortIdxBySlot((short int)(ulSlot), (short int)(ulPort));
	if( (PonPortIdx< 0) || (PonPortIdx >= MAXPON) )
		return VOS_ERROR;

	if(PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport==0)
	{
		vty_out(vty,"can't get pon %d/%d sfp power-metering info\r\n", ulSlot, ulPort );
		return CMD_WARNING;
	}

	recv_history = (Optical_Power_Rx_History_t *)pBuf;
	number = pBufLen/sizeof(Optical_Power_Rx_History_t);
	
	vty_out(vty,"\r\n OnuIdx    Status       rx-power(dbm)                            DownTime\r\n" );
	vty_out(vty," -----------------------------------------------------------------------------\r\n");
	for(j = 0 ; j < number ; j++)
	{
		recv_history2 = recv_history;
		OnuIdx = recv_history->OnuIdx;

	 	if(recv_history->NotExistFlag != 0 )
	 	{
			recv_history = (Optical_Power_Rx_History_t *)(recv_history2+1);
			continue;
		}

		onu_status = GetOnuOperStatus_Ext(PonPortIdx, OnuIdx );
		if( (onu_status < 0) || (onu_status > 5) )
			onu_status = 0;
		
		vty_out(vty," %d/%d/%-2d", ulSlot, ulPort, OnuIdx+1);
		
		vty_out(vty,"    %-10s", OnuCurrentStatus[onu_status]);
		for(i=0;i<READ_I2C_COUNT;i++)
		{
			val = recv_history->Recv_Buffer[i];
			vty_out(vty,"   %3d.%d", decimal2_integer_part(val), decimal2_fraction_part(val) );
		}
		if(onu_status == 1)
			vty_out(vty,"      -");
		else
		{
			ULONG /*CurTicks,*/ SysUpDelayed;
			ULONG days, hours,minutes, seconds;
			OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
			/*CurTicks = VOS_GetTick();
			SysUpDelayed = CurTicks - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			seconds = SysUpDelayed / VOS_Ticks_Per_Seconds;*/
			SysUpDelayed = time(0) - OnuMgmtTable[OnuEntry].DeviceInfo.SysUptime;
			days = SysUpDelayed /V2R1_ONE_DAY;
			hours = (SysUpDelayed % V2R1_ONE_DAY ) / V2R1_ONE_HOUR;
			minutes = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) / V2R1_ONE_MINUTE;
			seconds = ((SysUpDelayed % V2R1_ONE_DAY) % V2R1_ONE_HOUR ) % V2R1_ONE_MINUTE;
			vty_out(vty,"    %04u:%02u:%02u:%02u ",days, hours, minutes, seconds );
		}
		vty_out(vty,"\r\n");
		recv_history = (Optical_Power_Rx_History_t *)(recv_history2+1);
	}
	return VOS_OK;
}

LONG Sfp_Type_Add(UCHAR *String_Name)
{
	sfp_type_vendor *curNode;
	curNode = HeadSfpType;
	VOS_SemTake(opticalpower_semid, WAIT_FOREVER);
	while( curNode )
	{
		if(VOS_StrnCmp( String_Name, curNode->type_name, SFP_TYPE_Vendor_len) !=0 )
		{
			curNode=curNode->pNext;
			continue;
		}
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
			sys_console_printf("SFPType %s has been added to sfptype list\r\n", String_Name );
		VOS_SemGive(opticalpower_semid);
		return VOS_ERROR;
	}
		
	curNode = VOS_Malloc(sizeof(sfp_type_vendor), MODULE_RPU_PON_MON);
	if( curNode == NULL )
	{
		VOS_ASSERT(0);
		VOS_SemGive(opticalpower_semid);
		return VOS_ERROR;
	}
	curNode->defaultType = 0;
	VOS_StrCpy( curNode->type_name, String_Name );
	curNode->pNext = HeadSfpType;
	HeadSfpType = curNode;
	VOS_SemGive(opticalpower_semid);
	return VOS_OK;
}

ULONG Sfp_Type_Delete(UCHAR *String_Name)
{
	sfp_type_vendor *curNode, *pre;

	curNode = HeadSfpType;
	semTake(opticalpower_semid, WAIT_FOREVER);
	pre = curNode;
	while( curNode )
	{
		if( VOS_StrnCmp(String_Name, curNode->type_name, SFP_TYPE_Vendor_len) == 0 )
			break;
		pre = curNode;
		curNode = curNode->pNext;
	}

	if( curNode == NULL )
	{
		sys_console_printf("delete sfptype %s doesn't exist\r\n", String_Name);
		semGive(opticalpower_semid);
		return CMD_WARNING;
	}
	if( pre == curNode )
	{
		HeadSfpType = HeadSfpType->pNext;
		VOS_Free( curNode );
	}
	else
	{
		pre->pNext = curNode->pNext;
		VOS_Free( curNode );
	}
	semGive(opticalpower_semid);
	
	return CMD_SUCCESS;

}


DEFUN(config_ponsfptype_to_sfptable,
	config_ponsfptype_to_sfptable_cmd,
	"pon-adc negative-polarity sfp-type <name>",
	"sfp time sequence config\n"
	"negative polarity\n"
	"sfp name or vendor id\n"
	"input sfp name\n")
{
	
	if( VOS_StrLen(argv[0]) > SFP_TYPE_Vendor_len )
	{
		vty_out(vty,"input name length out of range\r\n");
		return CMD_WARNING;
	}
	
	/*if(VOS_OK == RPU_SendCmd2OpticalPower(pon_adc_negative_polarity_stp_type,-1,argv[0],SFP_TYPE_Vendor_len,0,0,0,0))
	{
		Sfp_Type_Add(argv[0]);
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}
	*/
	Set_OpticalPower_SfpType_Add(-1,argv[0]);
	return CMD_SUCCESS;
}

DEFUN(undo_config_ponsfptype_to_sfptable,
	undo_config_ponsfptype_to_sfptable_cmd,
	"undo pon-adc negative-polarity sfp-type <name>",
	"undo pon adc config\n" 
	"sfp time sequence config\n"
	"negative polarity\n"
	"sfp type or vendor id\n"
	"input sfp type\n")
{
	if( VOS_StrLen(argv[0]) > SFP_TYPE_Vendor_len )
	{
		vty_out(vty,"input name length out of range\r\n");
		return CMD_WARNING;
	}

	/*if(VOS_OK == RPU_SendCmd2OpticalPower(undo_pon_adc_negative_polarity_stp_type,-1,argv[0],SFP_TYPE_Vendor_len,0,0,0,0))
	{
		Sfp_Type_Delete(argv[0]);
	}
	else
	{
		vty_out(vty,"Send the command to Pon card failed !\r\n");
	}*/
	Set_OpticalPower_SfpType_Del(-1,argv[0]);

	return CMD_SUCCESS;
}

/*DEFUN(config_optical_power_measurepara,
	config_optical_power_measurepara_cmd,
	"config optical-power-measure <slot/port> trigger <0-65535> timelen <0-65535>",
	CONFIG_STR
	"ONU optical-power measure\n"
	"input the pon slot/port\n"
	"ONU optical-power measure's trigger time\n"
	"input the trigger time, unit:ms\n"
	"ONU optical-power measure's trigger timelen\n"
	"input the trigger timespan, unit:ms\n"
	)
{
	 int adc_start_time;
	 int adc_time_len;
	unsigned long ulSlot, ulPort;
	short int PonPortIdx;
	if( IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort ) != VOS_OK )
		return CMD_WARNING;
	if (0 > (PonPortIdx = checkPonPortIndex( vty, ulSlot, ulPort )) )
   	 {
    		return(CMD_WARNING);
    	 }   
	CHECK_PON_RANGE

    	adc_start_time = VOS_AtoL( argv[1] );
   	adc_time_len = VOS_AtoL( argv[2] );

	return(CMD_SUCCESS);
}*/

DEFUN(show_ponsfptype_to_sfptable,
	show_ponsfptype_to_sfptable_cmd,
	"show pon-adc negative-polarity sfp-type",
	SHOW_STR
	"sfp time sequence config\n"
	"negative polarity\n"
	"sfp type or vendor id\n")
{
	/*char SFPTypeName[SFP_TYPE_Vendor_len];*/
	sfp_type_vendor *curNode;

	curNode = HeadSfpType;
	semTake(opticalpower_semid, WAIT_FOREVER);
	while( curNode )
	{
		if( curNode->defaultType )
			vty_out( vty," %s  default\r\n", curNode->type_name );
		else
			vty_out( vty," %s\r\n", curNode->type_name );
		curNode = curNode->pNext;
	}
	semGive(opticalpower_semid);
	return CMD_SUCCESS;
}

int SetOpticalPowerDebug(long enable_mode, long DebugType)
{
	if(enable_mode == V2R1_ENABLE)
	{
		if(DebugType == 1 ||DebugType == 0x100 || DebugType == 0x10)
		{
			sfp_debug_switch |= DebugType;
		}
		else
			return VOS_ERROR;
	}
	else
	{
		if(DebugType == 1 ||DebugType == 0x100 || DebugType == 0x10)
		{
			sfp_debug_switch &= (~DebugType);
		}
		else
			return VOS_ERROR;
	}
	
	return VOS_OK;

}

DEFUN(debug_optical_power,
	debug_optical_power_cmd,
	"debug optical-power [pon|uplink|onu-laser]",
	DEBUG_STR
	"debug optical power\n"
	"pon sfp monitor\n"
	"uplink sfp monitor\n"
	"onu laser always on\n")
{
	long DebugType = 0;
	
	if( argv[0][0] == 'p' )
		DebugType = 1;
	else if( argv[0][0] == 'u' )
		DebugType = 0x100;
	else
		DebugType = 0x10;

	if(VOS_OK == Set_Debug_OpticalPower(-1,V2R1_ENABLE, DebugType))
		return CMD_SUCCESS;
	else
		return CMD_WARNING;
}

DEFUN(undo_debug_optical_power,
	undo_debug_optical_power_cmd,
	"undo debug optical-power [pon|uplink|onu-laser]",
	"undo\n"
	DEBUG_STR
	"debug optical power\n"
	"pon sfp monitor\n"
	"uplink sfp monitor\n"
	"onu laser always on\n")
{
	long DebugType = 0;
	
	if( argv[0][0] == 'p' )
		DebugType = 1;
	else if( argv[0][0] == 'u' )
		DebugType = 0x100;
	else
		DebugType = 0x10;

	if(VOS_OK == Set_Debug_OpticalPower(-1,V2R1_DISABLE, DebugType))
		return CMD_SUCCESS;
	else
		return CMD_WARNING;
}

DEFUN(config_onu_tx_power_supply_control,
	config_onu_tx_power_supply_control_cmd,
	"optical-power onu-tx-power-supply-control <slot/port> [all|<onuid_list>] <0-65535>",
	"set optical power metering opteration\n"
	"set optical power metering onu_tx_power control\n"
	"pon slot/port\n"
	"all onu\n"
	"onu ID,the range is 1-"INT_TO_STR(MAXONUPERPONNOLIMIT)" \n"
	"0:re-enable TX power supply;1-65534:duration shutdown time,unit:second;65535:permanently shutdown\n"
	)
{
	LONG ret = 0,_i = 0;
	unsigned long ulSlot = 0, ulPort = 0;
	ULONG * _pulIfArray = NULL , OnuNum = 0, PonPortIdx = 0;
	ULONG action = -1;
	ULONG pulIfArray = 0;

	action = VOS_AtoL(argv[2]);
	
	if( IFM_ParseSlotPort( argv[0], &ulSlot, &ulPort ) != VOS_OK )
		return CMD_WARNING;

	if( (PonPortIdx = checkPonPortIndex(vty, ulSlot, ulPort)) == RERROR )	
		return CMD_WARNING;

	if(argv[1][0] == 'a')
	{
		pulIfArray = 0xff;
		OnuNum = 1;
		ret = onu_tx_power_supply_control(ulSlot, ulPort, &pulIfArray, OnuNum*sizeof(ULONG), action);
	}
	else
	{
	  	_pulIfArray = V2R1_Parse_OnuId_List(argv[1]);
		if(_pulIfArray != NULL)
		{
			for(_i=0;_pulIfArray[_i]!=0;_i++)
			{
				OnuNum++;
			}
		}
		else
			return CMD_WARNING;

		if(OnuNum > 10)
		{
			VOS_Free(_pulIfArray);
			vty_out(vty,"The number of the onuid in the onuid_list shouldn't be bigger then ten !\r\n");
			return CMD_WARNING;
		}	
		ret = onu_tx_power_supply_control(ulSlot, ulPort, _pulIfArray, OnuNum*sizeof(ULONG), action);
	
		VOS_Free(_pulIfArray);
	}
	
	return ret;
}

/*
DEFUN(debug_optical_power,
	debug_optical_power_cmd,
	"debug optical-power [pon|uplink|onu-laser]",
	DEBUG_STR
	"debug optical power\n"
	"pon sfp monitor\n"
	"uplink sfp monitor\n"
	"onu laser always on\n")
{
	if( argv[0][0] == 'p' )
		sfp_debug_switch |= 1;
	else if( argv[0][0] == 'u' )
		sfp_debug_switch |= 0x100;
	else
		sfp_debug_switch |= 0x10;
	return CMD_SUCCESS;
}

DEFUN(undo_debug_optical_power,
	undo_debug_optical_power_cmd,
	"undo debug optical-power [pon|uplink|onu-laser]",
	"undo\n"
	DEBUG_STR
	"debug optical power\n"
	"pon sfp monitor\n"
	"uplink sfp monitor\n"
	"onu laser always on\n")
{
	if( argv[0][0] == 'p' )
		sfp_debug_switch &= (~1);
	else if( argv[0][0] == 'u' )
		sfp_debug_switch &= (~0x100);
	else
		sfp_debug_switch &= (~0x10);
	return CMD_SUCCESS;
}
*/
	
int PonPortMeteringCliInit(void)
{
	install_element ( PON_PORT_NODE, &pon_show_transceiver_info_cmd);
	install_element ( CONFIG_NODE, &config_show_transceiver_info_cmd);
	install_element ( VIEW_NODE, &config_show_transceiver_info_cmd);
	install_element ( PON_PORT_NODE, &pon_transceiver_meter_show_cmd );
	install_element ( CONFIG_NODE, &olt_pon_transceiver_meter_show_cmd);	
	install_element ( VIEW_NODE, &olt_pon_transceiver_meter_show_cmd);	
	install_element ( PON_PORT_NODE, &pon_optical_power_meter_show_cmd);
	install_element ( CONFIG_NODE, &config_show_optical_power_meter_cmd);
	install_element ( VIEW_NODE, &config_show_optical_power_meter_cmd);
	
	install_element (CONFIG_NODE,&config_show_optical_always_onl_value_cmd);	
	install_element (VIEW_NODE,&config_show_optical_always_onl_value_cmd);	
/*	install_element ( PON_PORT_NODE, &pon_show_onu_transceiver_meter_cmd );*/
	install_element ( CONFIG_NODE, &olt_rx_onu_transceiver_meter_show_cmd);	
	install_element ( VIEW_NODE, &olt_rx_onu_transceiver_meter_show_cmd);	
	install_element ( PON_PORT_NODE, &pon_rx_onu_transceiver_meter_show_cmd);

	install_element(CONFIG_NODE, &uplink_transceiver_meter_show_cmd);
	install_element(VIEW_NODE, &uplink_transceiver_meter_show_cmd);
	install_element(DEBUG_HIDDEN_NODE, &uplink_transceiver_meter_show_cmd_10GE_thred);
	install_element(CONFIG_NODE, &config_uplink_loscheck_interval_cmd);
	install_element(CONFIG_NODE, &config_uplink_loscheck_show_cmd);
	
	install_element ( CONFIG_NODE, &config_optical_power_enable_cmd);
	install_element ( CONFIG_NODE, &config_optical_power_onu_enable_cmd);	
	install_element ( CONFIG_NODE, &config_optical_power_alwayson_enable_cmd);
	install_element ( CONFIG_NODE, &config_optical_power_interval_cmd);
	install_element ( CONFIG_NODE, &config_optical_power_interval_cmd_def);
	install_element ( CONFIG_NODE, &config_optical_power_show_cmd);
	install_element ( VIEW_NODE, &config_optical_power_show_cmd);

	/* 告警门限*/
	/* deadzone */
	install_element ( CONFIG_NODE, &config_power_alarm_thrld_deadzone_cmd);
	install_element ( CONFIG_NODE, &config_power_alarm_thrld_deadzone_show_cmd);
	install_element ( VIEW_NODE, &config_power_alarm_thrld_deadzone_show_cmd);

	/* optical power threshold*/
	install_element ( CONFIG_NODE, &optical_power_threshold_config_cmd);
	install_element ( CONFIG_NODE, &optical_temperature_threshold_config_cmd);
	install_element ( CONFIG_NODE, &optical_supply_voltage_config_cmd);
	install_element ( CONFIG_NODE, &optical_bias_current_config_cmd);	

	install_element ( CONFIG_NODE, &optical_power_alarm_thrld_show_cmd);
	install_element ( VIEW_NODE, &optical_power_alarm_thrld_show_cmd);
	
	install_element ( CONFIG_NODE, &config_optical_power_calibration_cmd);
	install_element ( CONFIG_NODE, &show_optical_power_calibration_cmd);
	install_element ( VIEW_NODE, &show_optical_power_calibration_cmd);

#ifdef _CTC_TEST
	install_element ( CONFIG_NODE, &show_onu_laser_always_on_cmd);
#endif
	/*install_element ( CONFIG_NODE, &config_optical_power_measurepara_cmd);*/

	install_element ( CONFIG_NODE, &config_ponsfptype_to_sfptable_cmd);
	install_element ( CONFIG_NODE, &undo_config_ponsfptype_to_sfptable_cmd);
	install_element ( CONFIG_NODE, &show_ponsfptype_to_sfptable_cmd);

	install_element(CONFIG_NODE,&OnuLaser_AlwaysOn_Alarm_Threshold_Cmd);
	install_element(CONFIG_NODE,&OnuLaser_AlwaysOn_Alarm_Times_Cmd);
	install_element(CONFIG_NODE,&OnuLaser_AlwaysOn_AlarmClear_Times_Cmd);
	install_element(CONFIG_NODE,&Show_OnuLaser_AlwaysOn_Cmd);
	install_element(VIEW_NODE,&Show_OnuLaser_AlwaysOn_Cmd);
	install_element(PON_PORT_NODE, &Show_Sfp_Type_Cmd);
	install_element(DEBUG_HIDDEN_NODE, &debug_optical_power_cmd);
	install_element(DEBUG_HIDDEN_NODE, &undo_debug_optical_power_cmd);
	install_element(CONFIG_NODE, &config_onu_tx_power_supply_control_cmd);

	install_element ( CONFIG_NODE, &pon_online_sfp_pon_cmd);
	install_element ( VIEW_NODE, &pon_online_sfp_pon_cmd);
		
	if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		install_element ( CONFIG_NODE, &pon_online_sfp_cmd);
		install_element ( VIEW_NODE, &pon_online_sfp_cmd);
	}	
	return(ROK);
}

int  optical_power_show_run( struct vty * vty )
{
	long  tx_high_val,tx_low_val,rx_high_val,rx_low_val;
	long  high_val,low_val;
	/*long laser_on;*/
	long  monitor,monitor_interval;
	long  power,temperature,voltage,current;
	sfp_type_vendor *curNode;

	vty_out( vty, "!optical power monitor config\r\n" );
	curNode = HeadSfpType;
	semTake(opticalpower_semid, WAIT_FOREVER);
	while( curNode )
	{
		if( curNode->defaultType == 0 )
			vty_out( vty," pon-adc negative-polarity sfp-type %s\r\n", curNode->type_name );
		curNode = curNode->pNext;
	}
	semGive(opticalpower_semid);
	
	monitor=getOpticalPowerThreshold(field_olt_monitor_enable,0);
		
	if(monitor  != POWER_METERING_ENABLE_DEFAULT)
	{
		/*vty_out( vty, "!optical power and optical monitior config\r\n" );*/
		rx_low_val=getOpticalPowerThreshold(field_olt_recv_oppower_low,0);
		rx_high_val=getOpticalPowerThreshold(field_olt_recv_oppower_high,0);
		tx_low_val=getOpticalPowerThreshold(field_olt_trans_oppower_low,0);
		tx_high_val=getOpticalPowerThreshold(field_olt_trans_oppower_high,0);
		if( (RECV_OPTICAL_POWER_LOW != rx_low_val) || (RECV_OPTICAL_POWER_HIGH != rx_high_val) ||
			(TRAN_OPTICAL_POWER_LOW != tx_low_val) || (TRAN_OPTICAL_POWER_HIGH != tx_high_val) )
		{
			vty_out( vty," optical-power alarm-threshold olt %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
		}

		rx_low_val=getOpticalPowerThreshold(field_olt_recv_oppower_low,1);
		rx_high_val=getOpticalPowerThreshold(field_olt_recv_oppower_high,1);
		tx_low_val=getOpticalPowerThreshold(field_olt_trans_oppower_low,1);
		tx_high_val=getOpticalPowerThreshold(field_olt_trans_oppower_high,1);
		if( (RECV_OPTICAL_POWER_LOW_10GE != rx_low_val) || (RECV_OPTICAL_POWER_HIGH_10GE != rx_high_val) ||
			(TRAN_OPTICAL_POWER_LOW_10GE != tx_low_val) || (TRAN_OPTICAL_POWER_HIGH_10GE != tx_high_val) )
		{
			vty_out( vty," optical-power alarm-threshold olt-10ge %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
		}
		
		rx_low_val=getOpticalPowerThreshold(field_upport_recv_oppower_low,0);
		rx_high_val=getOpticalPowerThreshold(field_upport_recv_oppower_high,0);
		tx_low_val=getOpticalPowerThreshold(field_upport_trans_oppower_low,0);
		tx_high_val=getOpticalPowerThreshold(field_upport_trans_oppower_high,0);
		if( (UPLINK_RECV_OPTICAL_POWER_LOW != rx_low_val) || (UPLINK_RECV_OPTICAL_POWER_HIGH != rx_high_val) ||
			(UPLINK_TRAN_OPTICAL_POWER_LOW != tx_low_val) || (UPLINK_TRAN_OPTICAL_POWER_HIGH != tx_high_val) )
		{
			vty_out( vty," optical-power alarm-threshold uplink %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
		}

		rx_low_val=getOpticalPowerThreshold(field_upport_recv_oppower_low,1);
		rx_high_val=getOpticalPowerThreshold(field_upport_recv_oppower_high,1);
		tx_low_val=getOpticalPowerThreshold(field_upport_trans_oppower_low,1);
		tx_high_val=getOpticalPowerThreshold(field_upport_trans_oppower_high,1);
		if( (UPLINK_RECV_OPTICAL_POWER_LOW_10GE != rx_low_val) || (UPLINK_RECV_OPTICAL_POWER_HIGH_10GE != rx_high_val) ||
			(UPLINK_TRAN_OPTICAL_POWER_LOW_10GE != tx_low_val) || (UPLINK_TRAN_OPTICAL_POWER_HIGH_10GE != tx_high_val) )
		{
			vty_out( vty," optical-power alarm-threshold uplink-10GE %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_tempe_low,0);
		high_val=getOpticalPowerThreshold(field_olt_tempe_high,0);
		if( (PON_MODULE_TEMPERATURE_LOW != low_val) || (PON_MODULE_TEMPERATURE_HIGH != high_val) )
		{
			vty_out( vty," optical-temperature alarm-threshold olt %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_tempe_low,1);
		high_val=getOpticalPowerThreshold(field_olt_tempe_high,1);
		if( (PON_MODULE_TEMPERATURE_LOW_10GE!= low_val) || (PON_MODULE_TEMPERATURE_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-temperature alarm-threshold olt-10ge %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_upport_tempe_low,0);
		high_val=getOpticalPowerThreshold(field_upport_tempe_high,0);
		if( (UPLINK_MODULE_TEMPERATURE_LOW != low_val) || (UPLINK_MODULE_TEMPERATURE_HIGH != high_val) )
		{
			vty_out( vty," optical-temperature alarm-threshold uplink %d %d\r\n", high_val,low_val);
		}
		low_val=getOpticalPowerThreshold(field_upport_tempe_low,1);
		high_val=getOpticalPowerThreshold(field_upport_tempe_high,1);
		if( (UPLINK_MODULE_TEMPERATURE_LOW_10GE != low_val) || (UPLINK_MODULE_TEMPERATURE_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-temperature alarm-threshold uplink-10GE %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_vol_low,0);
		high_val=getOpticalPowerThreshold(field_olt_vol_high,0);
		if( (PON_MODULE_VOLTAGE_LOW != low_val) || (PON_MODULE_VOLTAGE_HIGH != high_val) )
		{
			vty_out( vty," optical-voltage alarm-threshold olt %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_vol_low,1);
		high_val=getOpticalPowerThreshold(field_olt_vol_high,1);
		if( (PON_MODULE_VOLTAGE_LOW_10GE != low_val) || (PON_MODULE_VOLTAGE_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-voltage alarm-threshold olt-10ge %d %d\r\n", high_val,low_val);
		}
		
		low_val=getOpticalPowerThreshold(field_upport_vol_low,0);
		high_val=getOpticalPowerThreshold(field_upport_vol_high,0);
		if( (UPLINK_MODULE_VOLTAGE_LOW != low_val) || (UPLINK_MODULE_VOLTAGE_HIGH != high_val) )
		{
			vty_out( vty," optical-voltage alarm-threshold uplink %d %d\r\n", high_val,low_val);
		}
		low_val=getOpticalPowerThreshold(field_upport_vol_low,1);
		high_val=getOpticalPowerThreshold(field_upport_vol_high, 1);
		if( (UPLINK_MODULE_VOLTAGE_LOW_10GE != low_val) || (UPLINK_MODULE_VOLTAGE_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-voltage alarm-threshold uplink-10GE %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_cur_low,0);
		high_val=getOpticalPowerThreshold(field_olt_cur_high,0);
		if( (PON_MODULE_BIAS_CURRENT_LOW != low_val) || (PON_MODULE_BIAS_CURRENT_HIGH != high_val) )
		{
			vty_out( vty," optical-bias-current alarm-threshold olt %d %d\r\n", high_val,low_val);
		}

		low_val=getOpticalPowerThreshold(field_olt_cur_low,1);
		high_val=getOpticalPowerThreshold(field_olt_cur_high,1);
		if( (PON_MODULE_BIAS_CURRENT_LOW_10GE != low_val) || (PON_MODULE_BIAS_CURRENT_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-bias-current alarm-threshold olt-10ge %d %d\r\n", high_val,low_val);
		}
		
		low_val=getOpticalPowerThreshold(field_upport_cur_low,0);
		high_val=getOpticalPowerThreshold(field_upport_cur_high,0);
		if( (UPLINK_MODULE_BIAS_CURRENT_LOW != low_val) || (UPLINK_MODULE_BIAS_CURRENT_HIGH != high_val) )
		{
			vty_out( vty," optical-bias-current alarm-threshold uplink %d %d\r\n", high_val,low_val);
		}
		low_val=getOpticalPowerThreshold(field_upport_cur_low,1);
		high_val=getOpticalPowerThreshold(field_upport_cur_high,1);
		if( (UPLINK_MODULE_BIAS_CURRENT_LOW_10GE != low_val) || (UPLINK_MODULE_BIAS_CURRENT_HIGH_10GE != high_val) )
		{
			vty_out( vty," optical-bias-current alarm-threshold uplink-10GE %d %d\r\n", high_val,low_val);
		}
		
		/* modified by xieshl 20110810, 问题单13112 */
		if( (olt_rx_optical_power_calibration != 0) || (olt_tx_optical_power_calibration != 0) )
			vty_out( vty, " optical-power calibration olt %d %d\r\n", olt_rx_optical_power_calibration, olt_tx_optical_power_calibration );
		
		vty_out( vty," optical-power enable\r\n");

	}
	
	if( GetOnuOpticalPowerEnable() == V2R1_ENABLE )
	{
		rx_low_val=getOpticalPowerThreshold(field_recv_oppower_low,0);
		rx_high_val=getOpticalPowerThreshold(field_recv_oppower_high,0);
		tx_high_val=getOpticalPowerThreshold(field_trans_oppower_high,0);
		tx_low_val=getOpticalPowerThreshold(field_trans_oppower_low,0);
		if( (PON_RECV_OPTICAL_POWER_LOW != rx_low_val) || (PON_RECV_OPTICAL_POWER_HIGH != rx_high_val) ||
			(PON_TRAN_OPTICAL_POWER_LOW != tx_low_val) || (PON_TRAN_OPTICAL_POWER_HIGH != tx_high_val) )
		{
			vty_out( vty," optical-power alarm-threshold onu %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
		}
		
		
		low_val=getOpticalPowerThreshold(field_pon_tempe_low,0);
		high_val=getOpticalPowerThreshold(field_pon_tempe_high,0);
		if( (PON_MODULE_TEMPERATURE_LOW != low_val) || (PON_MODULE_TEMPERATURE_HIGH != high_val) )
		{
			vty_out( vty," optical-temperature alarm-threshold onu %d %d\r\n", high_val,low_val);
		}
		low_val=getOpticalPowerThreshold(field_pon_vol_low,0);
		high_val=getOpticalPowerThreshold(field_pon_vol_high,0);
		if( (PON_MODULE_VOLTAGE_LOW != low_val) || (PON_MODULE_VOLTAGE_HIGH != high_val) )
		{
			vty_out( vty," optical-voltage alarm-threshold onu %d %d\r\n", high_val,low_val);
		}
		
		low_val=getOpticalPowerThreshold(field_pon_cur_low,0);
		high_val=getOpticalPowerThreshold(field_pon_cur_high,0);
		if( (PON_MODULE_BIAS_CURRENT_LOW != low_val) || (PON_MODULE_BIAS_CURRENT_HIGH != high_val) )
		{
			vty_out( vty," optical-bias-current alarm-threshold onu %d %d\r\n", high_val,low_val);	/* modified by xieshl 20110810, 问题单13112 */
		}
		
		/* modified by xieshl 20120925, 问题单15890 */
		if( (onu_rx_optical_power_calibration != 0) || (onu_tx_optical_power_calibration != 0) )
			vty_out( vty, " optical-power calibration onu %d %d\r\n", onu_rx_optical_power_calibration, onu_tx_optical_power_calibration );
		vty_out( vty," optical-power onu enable\r\n");
	}
	
	/*laser_on=getOpticalPowerThreshold(field_olt_laser_thresh);
	vty_out( vty," onu-laser-always-on-threshold %d\r\n", laser_on);*/

	if(V2R1_ENABLE == onuLaser_alwaysOn_Enable)
	{
		if(OnuLaser_AlwaysOn_Alarm_Threshold_Default != onuLaser_alwaysOn_alarm_threshold)
		{
			vty_out(vty," config onu-laser-always-on alarm-threshold %d\r\n", decimal2_integer_part(onuLaser_alwaysOn_alarm_threshold) );
		}

		if(OnuLaser_AlwaysOn_Alarm_TimeCounter_Default != onuLaser_alwaysOn_alarm_timeCounter)
		{
			vty_out(vty," config onu-laser-always-on alarm-times %d\r\n",onuLaser_alwaysOn_alarm_timeCounter);
		}

		if(OnuLaser_AlwaysOn_Clear_TimeCounter_Default != onuLaser_alwaysOn_clear_timeCounter)
		{
			vty_out(vty," config onu-laser-always-on alarm-clear-times %d\r\n",onuLaser_alwaysOn_clear_timeCounter);
		}
		vty_out( vty, " config onu-laser-always-on enable\r\n" );
	}
	
	monitor_interval=GetPonPortOpticalMonitorInterval();
	if( POWER_METERING_INTERVAL_DEFAULT != monitor_interval )
	{
		vty_out( vty," optical-power interval %d\r\n", monitor_interval);
	}

	power=getOpticalPowerDeadZone(field_power_dead_zone);
	temperature=getOpticalPowerDeadZone(field_tempe_dead_zone);
	voltage=getOpticalPowerDeadZone(field_vol_dead_zone);
	current=getOpticalPowerDeadZone(field_cur_dead_zone);
	if( (POWER_ALARM_DEADZONE_DEFAULT != power) || (VOLTAGE_ALARM_DEADZONE_DEFAULT != voltage) ||
		(CURRENT_ALARM_DEADZONE_DEFAULT != current) || (TEMERATURE_ALARM_DEADZONE_DEFAULT != temperature) )
	{
		vty_out( vty," optical-power threshold-deadzone %d %d %d %d\r\n", power,temperature,voltage,current);
	}

	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}


/* added by xieshl 20110829, 独立定义showrun模块，问题单12918 */
LONG OpticalPower_module_init()
{
    struct cl_cmd_module * opticalpower_module = NULL;

    opticalpower_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_PON_MON);
    if ( !opticalpower_module )
    {
        ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_MemZero( ( char * ) opticalpower_module, sizeof( struct cl_cmd_module ) );

    opticalpower_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_PON_MON );
    if ( !opticalpower_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( opticalpower_module );
        return VOS_ERROR;
    }
    VOS_StrCpy( opticalpower_module->module_name, "optical-power" );

    opticalpower_module->init_func = PonPortMeteringCliInit;
    opticalpower_module->showrun_func = optical_power_show_run;
    opticalpower_module->next = NULL;
    opticalpower_module->prev = NULL;
    cl_install_module( opticalpower_module );

    return VOS_OK;
}



/*
LONG Optical_Power_CdpSync_Call(RPC_Optical_MsgHead_S *pstSendMsg,ULONG pstSendMsgLen, 
					short int PonPortIdx)
{
	RPC_Optical_Msg_ResHead_S *pstRevData = NULL;
	ULONG ulRevLen = 0,rc = VOS_ERROR;
	USHORT ulSlot,ulPort;
	UCHAR SfpInfo[4],OnuLaserValue_t[6];
	ulSlot = GetCardIdxByPonChip(PonPortIdx);
	ulPort = GetPonPortByPonChip(PonPortIdx);
	
	if( __SYS_MODULE_TYPE__(ulSlot) == MODULE_TYPE_NULL )	
	{

			return  rc ;
	}
	
	if(SlotCardIsPonBoard(ulSlot) != ROK )		
	{
		
			return  rc ;
	}

	if(getPonChipInserted((unsigned char)(ulSlot),(unsigned char)(ulPort)) != PONCHIP_EXIST)	
	{
		
			return  rc ;
	}

	if(PonPortIsWorking(PonPortIdx) != TRUE)
	{
		
		return(RERROR);
	}
	
	switch( pstSendMsg->ulCmdType )
	{
		case show_optical_power_XcvrInfoArr:
			VOS_MemCpy(SfpInfo,(UCHAR *)(pstSendMsg+1),pstSendMsg->pSendBufLen);
			break;
		case show_onu_laser_always_on:
			VOS_MemCpy(OnuLaserValue_t,(UCHAR *)(pstSendMsg+1),pstSendMsg->pSendBufLen);
			break;
		default:
			break;
	}
	
	if ( VOS_OK == CDP_SYNC_Call( MODULE_RPU_PON_MON,ulSlot, MODULE_RPU_PON_MON, 0,
			pstSendMsg, pstSendMsgLen, ( VOID** ) &pstRevData,&ulRevLen, 3000 ) )  
	{
		if ( pstRevData != NULL)
		{
			if ( pstRevData->usSrcSlot != ulSlot )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			if ( pstRevData->usMsgMode != OPTICALTOLIC_ACK_END )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			
			if(pstRevData->ResResult == 1)
			{
				switch( pstRevData->ulCmdType )
				{
					case show_optical_power_olt_rx_history:
						Show_OpticalPower_OltRx_History_FromPon(ulSlot,ulPort ,(UCHAR *)(pstRevData+1),pstRevData->pSendBufLen);
						break;
					case show_optical_power_olt_rx_instant:
						Show_OpticalPower_OltRx_Instant_FromPon(ulSlot,ulPort ,(UCHAR *)(pstRevData+1),pstRevData->pSendBufLen);
						break;
					case show_optical_power_XcvrInfoArr:
						Show_OpticalPower_XcvrInfoArr_FromPon(ulSlot,ulPort ,(UCHAR *)(pstRevData+1),pstRevData->pSendBufLen,SfpInfo);
						break;
					case show_onu_laser_always_on:
						Show_OpticalPower_OnuLaser_FromPon(ulSlot,ulPort ,(UCHAR *)(pstRevData+1),pstRevData->pSendBufLen,OnuLaserValue_t);
						break;
					default:
						break;
				}
				rc = VOS_OK;
			}
			else
				rc = VOS_ERROR;
			
			CDP_SYNC_FreeMsg( pstRevData );
			
			pstRevData = NULL;
		}
		else
		{
			ASSERT( 0 );
		}
		return rc;
	}
	else
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}
	
	return rc;
}
*/
LONG OpticalPower_PRC_CALL(Optical_Cmd_Type_t  cmd_type,short int PonPortIdx, UCHAR *strings,ULONG length,ULONG para1,
				ULONG para2, ULONG para3, ULONG para4,ULONG para5,VOID *pRcvData, ULONG ulRcvLen)
{
	RPC_Optical_MsgHead_S *pstSendMsg = NULL;
	RPC_Optical_Msg_ResHead_S *pstRevData = NULL;
	ULONG ulCmdID = 0;
	ULONG rc = VOS_ERROR , pstSendMsgLen , ulRevLen;
	USHORT ulSlot,ulPort;

	ulSlot = GetCardIdxByPonChip(PonPortIdx);
	ulPort = GetPonPortByPonChip(PonPortIdx);

	if(SYS_MODULE_ISMASTERSTANDBY(SYS_LOCAL_MODULE_SLOTNO)/*||SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/)
	{
		/*sys_console_printf("%%The card is MasterStandby card,needn't send cmd to lic.\r\n");*/
		return VOS_OK;
	}

	if( SYS_LOCAL_MODULE_SLOTNO == ulSlot )
		return VOS_OK;

	/*if(SYS_MODULE_IS_PON(SYS_LOCAL_MODULE_SLOTNO))
	{
		sys_console_printf("%%The card is pon card,can't send cmd to lic.\r\n");
		return VOS_ERROR;
	}*/

	/*if ( !DEV_IsMySelfMaster() )
	{
		sys_console_printf("%%The card is slave card,can't send cmd to lic.\r\n");
		return VOS_ERROR;
	}*/

	if ( !SYS_MODULE_IS_READY(ulSlot) )
	{
		return VOS_OK;
	}

	if(!SYS_MODULE_IS_PON(ulSlot))	
	{
		return VOS_OK;
	}
	
	pstSendMsg = ( RPC_Optical_MsgHead_S* ) CDP_SYNC_AllocMsg( sizeof( RPC_Optical_MsgHead_S )+length, MODULE_RPU_PON_MON );
	if ( NULL == pstSendMsg )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}
	
	VOS_MemZero((VOID *)pstSendMsg, sizeof( RPC_Optical_MsgHead_S )+length );
	
	pstSendMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
       pstSendMsg->usDstSlot =  GetCardIdxByPonChip(PonPortIdx);
	pstSendMsg->ulSrcModuleID = MODULE_RPU_PON_MON;
	pstSendMsg->ulDstModuleID = MODULE_RPU_PON_MON;
	pstSendMsg->usMsgMode = OPTICALTOLIC_REQACK;
	pstSendMsg->usMsgType = OPTICALTOLIC_EXECUTE_CMD;
	pstSendMsg->ulDstPort = ulPort-1;/*ulPort 总是为1，把ulDstPort 设置为0*/
	ulCmdID = RPC_Optical_CmdIDNumber++;
	pstSendMsg->ulCmdID = ulCmdID;
	pstSendMsg->ulCmdType = cmd_type;
	pstSendMsg->parameter[0] = para1;
	pstSendMsg->parameter[1] = para2;
	pstSendMsg->parameter[2] = para3;
	pstSendMsg->parameter[3] = para4;
	pstSendMsg->parameter[4] = para5;
	pstSendMsg->pSendBufLen = length;
	VOS_MemCpy( (VOID *)(pstSendMsg+1), strings, length );
	pstSendMsgLen = sizeof( RPC_Optical_MsgHead_S )+length;

	if ( VOS_OK == CDP_SYNC_Call( MODULE_RPU_PON_MON,ulSlot, MODULE_RPU_PON_MON, 0,
			pstSendMsg, pstSendMsgLen, ( VOID** ) &pstRevData,&ulRevLen, 90000 ) )  
	{
		if ( pstRevData != NULL)
		{
			if ( pstRevData->usSrcSlot != ulSlot )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			
			if ( pstRevData->usMsgMode != OPTICALTOLIC_ACK_END )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}

			if(ulCmdID != pstRevData->ulCmdID)
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			
			if(pstRevData->ResResult == 1)
			{
				if(pRcvData != NULL)
				{
					if(pstRevData->pSendBufLen != 0)
					{
						if(pstRevData->pSendBufLen <= ulRcvLen)
						{
							VOS_MemCpy(pRcvData, (UCHAR *)(pstRevData+1), pstRevData->pSendBufLen);
						}
						else
						{
							CDP_SYNC_FreeMsg( pstRevData );
							return VOS_ERROR;
						}
					}
				}
				
				rc = VOS_OK;
			}
			else
				rc = VOS_ERROR;
			
			CDP_SYNC_FreeMsg( pstRevData );
			
			pstRevData = NULL;
		}
		else
		{
			ASSERT( 0 );
			/*sys_console_printf("test 1\r\n");*/
		}
		return rc;
	}
	else
	{
		ASSERT( 0 );
		/*sys_console_printf("test 2\r\n");*/
		return VOS_ERROR;
	}
	return rc;
}

/*
LONG RPU_SendCmd2OpticalPower(Optical_Cmd_Type_t  cmd_type,short int PonPortIdx, UCHAR *strings,ULONG length,ULONG para1,
				ULONG para2, ULONG para3, ULONG para4)
{
	RPC_Optical_MsgHead_S *pstSendMsg = NULL;
	ULONG ulCmdID = 0, i = 0;
	ULONG rc = VOS_ERROR , pstSendMsgLen = 0;
	USHORT ulSlot , SendulSlot;
	USHORT SuccessFlag[64],*sFlag;
	if ( !DEV_IsMySelfMaster() )
	{
		sys_console_printf("%%The card is slave card,can't send cmd to lic.\r\n");
		return VOS_ERROR;
	}
	VOS_MemZero(SuccessFlag, 64);
	sFlag = SuccessFlag;
	pstSendMsg = ( RPC_Optical_MsgHead_S* ) CDP_SYNC_AllocMsg( sizeof( RPC_Optical_MsgHead_S )+length, MODULE_RPU_PON_MON );
	if ( NULL == pstSendMsg )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}
	VOS_MemZero((VOID *)pstSendMsg, sizeof( RPC_Optical_MsgHead_S )+length );
	pstSendMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
	if(PonPortIdx != -1)
		pstSendMsg->usDstSlot =  GetCardIdxByPonChip(PonPortIdx);
	else
		pstSendMsg->usDstSlot = -1;
	pstSendMsg->ulSrcModuleID = MODULE_RPU_PON_MON;
	pstSendMsg->ulDstModuleID = MODULE_RPU_PON_MON;
	pstSendMsg->usMsgMode = OPTICALTOLIC_REQACK;
	pstSendMsg->usMsgType = OPTICALTOLIC_EXECUTE_CMD;
	ulCmdID = RPC_Optical_CmdIDNumber++;
	pstSendMsg->ulCmdID = ulCmdID;
	pstSendMsg->ulCmdType = cmd_type;
	pstSendMsg->ulDstPort = PonPortIdx;
	pstSendMsg->parameter[0] = para1;
	pstSendMsg->parameter[1] = para2;
	pstSendMsg->parameter[2] = para3;
	pstSendMsg->parameter[3] = para4;
	pstSendMsg->pSendBufLen = length;
	VOS_MemCpy( (VOID *)(pstSendMsg+1), strings, length );
	pstSendMsgLen = sizeof( RPC_Optical_MsgHead_S )+length;
	if(PonPortIdx != -1)
		rc = Optical_Power_CdpSync_Call( pstSendMsg, pstSendMsgLen, PonPortIdx);
	else
	{
		SendulSlot = -1 ;
		for(i=0;i<MAXPON;i++)
		{
			ulSlot = GetCardIdxByPonChip(i);
			if(SendulSlot != ulSlot)
			{
				SendulSlot = ulSlot;
				rc = Optical_Power_CdpSync_Call( pstSendMsg, pstSendMsgLen, i);
				if(rc == VOS_OK)
				{
					*sFlag = SendulSlot ;
					sFlag++;
				}
			}
		}
		sys_console_printf("\r\nSend Success Slot : ");
		for(i = 0; i<32;i++)
		{
			if(SuccessFlag[i] != 0)
				sys_console_printf(" %d ",SuccessFlag[i]);
		}
		sys_console_printf("\r\n");
		return VOS_OK;
	}
    return rc;
}
*/

int CTC_eventReportMsgSend( CtcEventCfgData_t *pAlmMsg )
{
	int rc = VOS_ERROR;
	ULONG ulMsg[4] = {MODULE_EVENT, FC_EVENT_CFG_PON_TO_ONU, 0, 0};
	SYS_MSG_S * pstMsg = NULL;
	ULONG msgLen = sizeof(SYS_MSG_S) + sizeof(CtcEventCfgData_t);

	if( pAlmMsg == NULL )
		return rc;
	
	pstMsg = (SYS_MSG_S *)VOS_Malloc( msgLen, MODULE_EVENT ); 
	if( pstMsg == NULL )
	{
		VOS_ASSERT(0);
		return rc;
	}

	VOS_MemZero( pstMsg, msgLen );
	pstMsg->ulSrcModuleID = MODULE_EVENT;
	pstMsg->ulDstModuleID = MODULE_EVENT;
	pstMsg->ulSrcSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ulDstSlotID = SYS_LOCAL_MODULE_SLOTNO;
	pstMsg->ucMsgType = MSG_REQUEST;
	pstMsg->ucMsgBodyStyle = MSG_BODY_INTEGRATIVE;  
	pstMsg->ptrMsgBody = (VOID *)(pstMsg + 1);
	pstMsg->usFrameLen = sizeof(CtcEventCfgData_t);
	pstMsg->usMsgCode = FC_EVENT_CFG_PON_TO_ONU;

	VOS_MemCpy( pstMsg->ptrMsgBody, pAlmMsg, sizeof(eventMsg_t) );

	ulMsg[3] = (ULONG)pstMsg;
	rc = VOS_QueSend( ctcEventQueId, ulMsg, NO_WAIT, MSG_PRI_NORMAL);
	if( rc != VOS_OK )
	{
		VOS_Free((void*)pstMsg);
	}
	return rc;	
}

VOID OpticalPower_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen )
{
    RPC_Optical_MsgHead_S * RecvpMsg = NULL;
    RPC_Optical_Msg_ResHead_S *SendpMsg = NULL;
    LONG rc = -1;
    LONG ulSrcSlot = -1 ;
    UCHAR *pBuf = NULL;
    ULONG pBufLen = 0,OnuNum = 0;
    _XCVR_DATA_  *pXcvrArr;
	
    if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
        return ;

    if ( NULL == pReceiveData )
    {
        VOS_ASSERT( 0 );
        return ;
    }
   
    RecvpMsg = (RPC_Optical_MsgHead_S *)pReceiveData;

    ulSrcSlot = RecvpMsg->usSrcSlot;

    if( RecvpMsg->usMsgType == OPTICALTOLIC_EXECUTE_CMD )
    {
		switch( RecvpMsg->ulCmdType )
		{
			 case optical_power_enable :
				/*rc = SetPonPortOpticalMonitorEnable(V2R1_ENABLE);*/
				rc = Set_OpticalPower_Enable(RecvpMsg->ulDstPort,RecvpMsg->parameter[0], RecvpMsg->parameter[1], 0 );
				break;
			 /*case optical_power_disable :
				if(V2R1_DISABLE == onuLaser_alwaysOn_Enable)
					rc = SetPonPortOpticalMonitorEnable(V2R1_DISABLE);
				break;*/
		  /*optical-power [enable|disable]*/

			 case optical_power_interval :
				/*rc = SetPonPortOpticalMonitorInterval(RecvpMsg->parameter[0]);*/
				rc = Set_OpticalPower_Interval(RecvpMsg->ulDstPort,RecvpMsg->parameter[0]);
				break;
		  /* <1-86400> */

			 case pon_adc_negative_polarity_stp_type :
				/*rc = Sfp_Type_Add((UCHAR *)(RecvpMsg+1));*/
				rc = Set_OpticalPower_SfpType_Add(RecvpMsg->ulDstPort,(UCHAR *)(RecvpMsg+1));
				break;
			case undo_pon_adc_negative_polarity_stp_type :
				/*rc = Sfp_Type_Delete((UCHAR *)(RecvpMsg+1));*/
				rc = Set_OpticalPower_SfpType_Add(RecvpMsg->ulDstPort,(UCHAR *)(RecvpMsg+1));
				break;
		  /* pon-adc negative-polarity sfp-type <name>*/

			 case optical_power_calibration_olt :
				/*rc = setOltOpticalPowerCalibration( RecvpMsg->parameter[0], RecvpMsg->parameter[1] );*/
				rc = Set_OpticalPower_Calibration_Olt(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;
			 case optical_power_calibration_onu :
				/*rc = setOnuOpticalPowerCalibration( RecvpMsg->parameter[0], RecvpMsg->parameter[1] );*/
				rc = Set_OpticalPower_Calibration_Onu(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;
		  /* <rxCali> <txCali> */

			 case optical_power_threshold_deadzone :
				/*setOpticalPowerDeadZone(field_power_dead_zone,RecvpMsg->parameter[0]);
				setOpticalPowerDeadZone(field_tempe_dead_zone,RecvpMsg->parameter[1]);
				setOpticalPowerDeadZone(field_vol_dead_zone,RecvpMsg->parameter[2]);
				setOpticalPowerDeadZone(field_cur_dead_zone,RecvpMsg->parameter[3]);
				rc = CMD_SUCCESS;*/
				rc = Set_OpticalPower_Deadzone(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									RecvpMsg->parameter[2],RecvpMsg->parameter[3]);
				break;
		  /*optical-power threshold-deadzone <power> <tempertaure> <voltage> <current> */

			 case optical_power_alarm_threshold_olt :
				/*rc = SetPonPortTransOpticalPowerLowThrd(RecvpMsg->parameter[0]);
				rc = SetPonPortTransOpticalPowerHighThrd(RecvpMsg->parameter[1]);
				rc = SetPonPortRecvOpticalPowerLowThrd(RecvpMsg->parameter[2]);
				rc = SetPonPortRecvOpticalPowerHighThrd(RecvpMsg->parameter[3]);*/
				rc = Set_OpticalPower_alarm_threshold_olt(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									RecvpMsg->parameter[2],RecvpMsg->parameter[3],(int)(RecvpMsg->parameter[4]));
				break;
			 case optical_power_alarm_threshold_onu :
				/*rc = SetOnuTransOpticalPowerLowThrd(RecvpMsg->parameter[0]);
				rc = SetOnuTransOpticalPowerHighThrd(RecvpMsg->parameter[1]);
				
				rc = SetOnuRecvOpticalPowerLowThrd(RecvpMsg->parameter[2]);
				rc = SetOnuRecvOpticalPowerHighThrd(RecvpMsg->parameter[3]);*/
				rc = Set_OpticalPower_alarm_threshold_onu(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									RecvpMsg->parameter[2],RecvpMsg->parameter[3]);
				break;
			case optical_power_alarm_threshold_uplink :
				 /*rc = SetUplinkPortTransOpticalPowerLowthrd(pMsg->parameter[0]);
				rc = SetUplinkPortTransOpticalPowerHighthrd(pMsg->parameter[1]);
				
				rc = SetUplinkPortRecvOpticalPowerLowthrd(pMsg->parameter[2]);
				rc = SetUplinkPortRecvOpticalPowerHighthrd(pMsg->parameter[3]);
				break;*/
		  /* <Tx_high> <Tx_low> <Rx_high> <Rx_low> */ 
				rc = Set_OpticalPower_alarm_Threshold_Uplink(RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									RecvpMsg->parameter[2],RecvpMsg->parameter[3],(int)(RecvpMsg->parameter[4]));
				 break;
			  case optical_bias_current_alarm_threshold_olt :
				/*rc = SetPonPortBiasCurrentLowThrd(RecvpMsg->parameter[0]);
				rc = SetPonPortBiasCurrentHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_BiasCurrent_Threshold_Olt(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break; 
			  case optical_bias_current_alarm_threshold_onu :
				/*rc = SetOnuBiasCurrentLowThrd(RecvpMsg->parameter[0]);
				rc = SetOnuBiasCurrentHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_BiasCurrent_Threshold_Onu(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;
			  case optical_bias_current_alarm_threshold_uplink :
				/*rc = SetUplinkPortBiasCurrentLowthrd(pMsg->parameter[0]);
				rc = SetUplinkPortBiasCurrentHighthrd(pMsg->parameter[1]);
				break;*/
		 /* [olt|onu|uplink] <high> <low>*/
				rc = Set_OpticalPower_BiasCurrent_Threshold_Uplink(RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break; 
			 case optical_temperature_alarm_threshold_olt :
				/*rc = SetPonPortTemperatureLowThrd(RecvpMsg->parameter[0]);
				rc = SetPonPortTemperatureHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_Temperature_Threshold_Olt(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break;
			 case optical_temperature_alarm_threshold_onu :
				/*rc = SetOnuTemperatureLowThrd(RecvpMsg->parameter[0]);
				rc = SetOnuTemperatureHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_Temperature_Threshold_Onu(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;	
			 case optical_temperature_alarm_threshold_uplink :
				/*rc = SetUplinkPortTemperatureLowthrd(pMsg->parameter[0]);
				rc = SetUplinkPortTemperaturreHighthrd(pMsg->parameter[1]);
				break;*/
		  /*optical-temperature alarm-threshold [olt|onu|uplink] <high> <low>*/
				rc = Set_OpticalPower_Temperature_Threshold_Uplink(RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break;
			 case optical_voltage_alarm_threshold_olt :
				/*rc = SetPonPortWorkVoltageLowThrd(RecvpMsg->parameter[0]);
				rc = SetPonPortWorkVoltageHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_Voltage_Threshold_Olt(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break;
			 case optical_voltage_alarm_threshold_onu :
				/*rc = SetOnuWorkVoltageLowThrd(RecvpMsg->parameter[0]);
				rc = SetOnuWorkVoltageHighThrd(RecvpMsg->parameter[1]);*/
				rc = Set_OpticalPower_Voltage_Threshold_Onu(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;	
			 case optical_voltage_alarm_threshold_uplink :
				/*rc = SetUplinkPortVoltageLowthrd(pMsg->parameter[0]);
				rc = SetUplinkPortVoltageHighthrd(pMsg->parameter[1]);
				break; */
		  /*optical-voltage alarm-threshold [olt|onu|uplink] <high> <low>*/
				rc = Set_OpticalPower_Voltage_Threshold_Uplink(RecvpMsg->parameter[0],RecvpMsg->parameter[1],
									(int)(RecvpMsg->parameter[2]));
				break;
			case onu_laser_always_on_enable:
				/*if(GetPonPortOpticalMonitorEnable() == V2R1_ENABLE)
				{
					onuLaser_alwaysOn_Enable = V2R1_ENABLE ;
					rc = CMD_SUCCESS;
				}
				*/
				if(RecvpMsg->parameter[0] == V2R1_ENABLE && GetPonPortOpticalMonitorEnable() == V2R1_DISABLE)
				{
					rc = VOS_ERROR;
				}
				else
					rc = Set_OnuLaserAlwaysOn_Enable(RecvpMsg->ulDstPort,RecvpMsg->parameter[0]);
				break;
			 /*case onu_laser_always_on_disable:
				onuLaser_alwaysOn_Enable = V2R1_DISABLE ;
				rc = CMD_SUCCESS;
				break;*/
			 case onu_laser_always_on_alarm_times: 
				/*onuLaser_alwaysOn_alarm_timeCounter = RecvpMsg->parameter[0] ;
				rc = CMD_SUCCESS;*/
				rc = Set_OnuLaserAlwaysOn_AlarmTimes(RecvpMsg->ulDstPort,RecvpMsg->parameter[0]);
				break;
			 case onu_laser_always_on_alarm_clear_times:
				/*onuLaser_alwaysOn_clear_timeCounter = RecvpMsg->parameter[0] ;
				rc = CMD_SUCCESS;*/
				rc = Set_OnuLaserAlwaysOn_ClearTimes(RecvpMsg->ulDstPort,RecvpMsg->parameter[0]);
				break;
			 case onu_laser_always_on_alarm_threshold:
				/*onuLaser_alwaysOn_alarm_threshold = RecvpMsg->parameter[0] ;
				rc = CMD_SUCCESS;*/
				rc = Set_OnuLaserAlwaysOn_AlarmThreshold(RecvpMsg->ulDstPort,RecvpMsg->parameter[0]);
				break;
			case show_optical_power_olt_rx_history:
				OnuNum = RecvpMsg->pSendBufLen/sizeof(ULONG);
				pBufLen = OnuNum*sizeof(Optical_Power_Rx_History_t);
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero( pBuf, pBufLen);
				rc =Fetch_Optical_Power_History( RecvpMsg->parameter[0], RecvpMsg->parameter[1],
											 (ULONG *)(RecvpMsg+1),RecvpMsg->pSendBufLen,(void *)pBuf);
				break;
			case show_optical_power_olt_rx_instant:
				OnuNum = RecvpMsg->pSendBufLen/sizeof(ULONG);
				pBufLen = OnuNum*sizeof(Optical_Power_Rx_Instant_t);
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero( pBuf, pBufLen);
				rc = Fetch_Optical_Power_Instant( RecvpMsg->parameter[0], RecvpMsg->parameter[1],
											 (ULONG *)(RecvpMsg+1),RecvpMsg->pSendBufLen,(void *)pBuf);
				break;
			case show_optical_power_XcvrInfoArr:
				pBufLen = 128;
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero( pBuf, pBufLen);
				rc = Fetch_Optical_Power_XcvrData(RecvpMsg->parameter[0], RecvpMsg->parameter[1],
					(UCHAR *)(RecvpMsg+1), (void *)pBuf);
				break;
			case show_onu_laser_always_on:
				pBufLen = MAXPON*sizeof(LONG);
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero(pBuf, pBufLen);
				rc = Fetch_OnuLaser_AlwaysOn(RecvpMsg->parameter[0], RecvpMsg->parameter[1],
					(UCHAR *)(RecvpMsg+1), (void *)pBuf);
				break;
			case debug_optical_power_command :
				rc = Set_Debug_OpticalPower(RecvpMsg->ulDstPort,RecvpMsg->parameter[0],RecvpMsg->parameter[1]);
				break;
			case show_optical_power_pon_sfp_type:
				pBufLen = SFP_TYPE_Vendor_len;
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero( pBuf, pBufLen);
				rc = Fetch_Optical_Power_PonSfpType(RecvpMsg->parameter[0], RecvpMsg->parameter[1],(void *)pBuf);
				break;
			case show_online_sfp_pon:
				pXcvrArr = &XcvrInfoArr[0];
				pBufLen = MAXPON*sizeof(Online_Sfp_Info_Get_t)+1;
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero(pBuf, pBufLen);
				rc = Fetch_Online_Sfp_Pon(RecvpMsg->parameter[0], RecvpMsg->parameter[1], (void *)pBuf,pBufLen,NULL);
				break;
			case get_sfp_online_state:
				if( RecvpMsg->parameter[0] == 1 )
					pBufLen = sizeof( UplinkPortMeteringInfo_S );
				else if( RecvpMsg->parameter[0] == 0 )
					pBufLen = sizeof(OpticalToPonMsgHead_t);
				else if( RecvpMsg->parameter[0] == 2 )
					pBufLen = sizeof( Online_Sfp_Info_Get_t );
				
				pBuf = VOS_Malloc(pBufLen, MODULE_RPU_PON_MON);
				if(pBuf == NULL)
				{
					VOS_ASSERT( 0 );
					return ;
				}
				VOS_MemZero(pBuf, pBufLen);
				rc = Fetch_Sfp_Online_State( RecvpMsg->ulDstPort, (void *)pBuf, pBufLen, RecvpMsg->parameter[0] );
				break;
			default:
				break;
		}
    }

		SendpMsg = ( RPC_Optical_Msg_ResHead_S  * ) CDP_SYNC_AllocMsg( sizeof( RPC_Optical_Msg_ResHead_S)+pBufLen, MODULE_RPU_PON_MON);
		if ( SendpMsg == NULL )
		{
			RPC_Optical_CmdIDNumber = 0;
			VOS_ASSERT( 0 );
			return ;
		}
		
		SendpMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
		SendpMsg->usDstSlot = ulSrcSlot;
		SendpMsg->ulSrcModuleID = MODULE_RPU_PON_MON;
		SendpMsg->ulDstModuleID = MODULE_RPU_PON_MON;
		SendpMsg->usMsgMode = OPTICALTOLIC_ACK_END;
		SendpMsg->ulCmdType = RecvpMsg->ulCmdType;
		SendpMsg->pSendBufLen = 0 ;
		if(rc == VOS_OK)	
		{
			SendpMsg->ResResult = 1;
			switch( RecvpMsg->ulCmdType )
			{
				case show_optical_power_olt_rx_history:
				case show_optical_power_olt_rx_instant:
				case show_optical_power_XcvrInfoArr:
				case show_onu_laser_always_on:
				case show_optical_power_pon_sfp_type:
				case show_online_sfp_pon:
				case get_sfp_online_state:
					VOS_MemCpy((VOID *)(SendpMsg+1), pBuf , pBufLen);
					SendpMsg->pSendBufLen = pBufLen;
					VOS_Free(pBuf );
					break;
				default:
					pBufLen = 0;
					break;
			}
		}
		else
		{
			SendpMsg->ResResult = -1;
			if( pBuf != NULL )
				VOS_Free(pBuf );
			pBufLen = 0;
		}
		SendpMsg->usMsgType = OPTICALTOLIC_EXECUTE_CMD;
		SendpMsg->ulCmdID = RecvpMsg->ulCmdID;
		
		*ppSendData = SendpMsg;
		*pulSendDataLen = sizeof(RPC_Optical_Msg_ResHead_S)+pBufLen;

	    return ;
}



LONG GetNum_NotEqual_Zero(UCHAR *array, LONG number)
{
	LONG counter , sum = 0;
	for(counter = 0; counter < number ; counter++)
	{
		if(0 != array[counter])
			sum++;
	}

	return sum;
}

enum OpticalPower_pdpType
{
	PON_MODULE_INFORMATION,
	UPLINK_MODULE_INFORMATION
};

typedef struct optical_12Epon_Uplink_Aux{
	USHORT PonPortIdx;
	USHORT BiasFlag;
	USHORT VoltFlag;
	USHORT TempFlag;
	USHORT RecvFlag;
	USHORT TranFlag;
	USHORT PortStatus;
	USHORT SFP_Online_Flag;
}optical_12Epon_Uplink_Aux_t;

long Optical_Timer_Callback()
{
	Optical_Msg_PerOnu_t *Optical_Msg_Onu,*Optical_Msg_Onu_temp;
	OpticalToPonMsgHead_t *pQueueMsg;
	UplinkPortMeteringInfo_S *uplink_temp;
	optical_12Epon_Uplink_Aux_t *Uplink_Aux;
	SYS_MSG_S *pMsg       = NULL;
	ULONG ulLen = 0,ulDatBufLen;
	USHORT PonPortIdx;
	int OnuId,Counter = 0,temp = 0;
	int OnuEntry, OnuEntryBase;
	PON_gpio_line_io_t  direction;
	bool value = 0;
	int portno;
	long status, lRet ;
	
	if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER || !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		return VOS_OK;

	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE 
		&& GetOnuOpticalPowerEnable() != V2R1_ENABLE)/*当该功能模块未开启时，不用发送*/
		return(RERROR);
	
	for(PonPortIdx=0; PonPortIdx<MAXPON; PonPortIdx++)
	{
		/*if(CheckSpePonSFPSupportRSSI(PonPortIdx)!=VOS_OK)
			continue; */
		if( (SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON) && PonPortIdx < 4 
			&& GetPonPortOpticalMonitorEnable() == V2R1_ENABLE )
		{
			/*if(Uplink_12Epon_Check_Flag[PonPortIdx] == 0)
				continue;*/
			
			ulLen=sizeof(UplinkPortMeteringInfo_S)+sizeof(SYS_MSG_S)+sizeof(optical_12Epon_Uplink_Aux_t);
			pMsg=(SYS_MSG_S*)PDP_AllocMsg(ulLen, MODULE_RPU_PON_MON);
		 	if( NULL == pMsg )
		       {
		       	VOS_ASSERT(0);
		              return (VOS_ERROR);
		       }
			VOS_MemZero((CHAR *)pMsg, ulLen );

			SYS_MSG_SRC_ID( pMsg )       = MODULE_RPU_PON_MON;
		       SYS_MSG_DST_ID( pMsg )       = MODULE_RPU_PON_MON;
		       SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
			SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
			SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
			SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
			SYS_MSG_SRC_SLOT( pMsg )  = SYS_LOCAL_MODULE_SLOTNO;
			SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
			SYS_MSG_MSG_CODE(pMsg)  = UPLINK_MODULE_INFORMATION;
			
			uplink_temp = (UplinkPortMeteringInfo_S* ) ( pMsg + 1 );

			portno = SYS_LOCAL_MODULE_SLOTNO*MAXUPLINKPORT+PonPortIdx;
			
			VOS_MemCpy( (VOID*)uplink_temp, (VOID*)(&(UplinkPortMeteringInfo[portno])), sizeof(UplinkPortMeteringInfo_S) );

			Uplink_Aux = (optical_12Epon_Uplink_Aux_t *)(uplink_temp+1);
			Uplink_Aux->PonPortIdx = PonPortIdx;
			Uplink_Aux->BiasFlag = UplinkBiasCurrent_Buffer_Flag[portno];
			Uplink_Aux->VoltFlag = UplinkVoltageApplied_Buffer_Flag[portno];
			Uplink_Aux->TempFlag = UplinkTemperature_Buffer_Flag[portno];
			Uplink_Aux->RecvFlag = UplinkRecvOpticalPower_Buffer_Flag[portno];
			Uplink_Aux->TranFlag = UplinkTransOpticalPower_Buffer_Flag[portno];
			Uplink_Aux->SFP_Online_Flag = Sfp_Uplink_Online_Flag[ portno ] ;
			lRet = getEthPortOperStatus(1, SYS_LOCAL_MODULE_SLOTNO, PonPortIdx+1, &status); 
			if( lRet == VOS_OK )
				Uplink_Aux->PortStatus = (USHORT)status;
			
			PDP_Send(RPU_TID_PDP_OPTICAL, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_PDP_OPTICAL, (VOID *)pMsg, ulLen);
			PDP_FreeMsg(pMsg);
			/*Uplink_12Epon_Check_Flag[PonPortIdx] = 0;*/
		}
		else if( GetPonPortOpticalMonitorEnable() == V2R1_ENABLE || GetOnuOpticalPowerEnable() == V2R1_ENABLE )
		{
#if 0
		    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			if(OLT_GpioAccess(PonPortIdx, PON_GPIO_LINE_3, PON_GPIO_LINE_INPUT, 0, &direction, &value ) == 0)
#else
	             if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
			{
				lRet = GetGponSfpOnlineState(PonPortIdx, &value);
			}
			else
			{
	         	 	lRet = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &value);
			}
#endif
			if(lRet == 0)
			{
				if(1 == value)
				{
					if( 1 == Sfp_Pon_Online_Flag[PonPortIdx] )
					{
						ClearOpticalPowerAlarmWhenPonPortDown( PonPortIdx ) ;
						Sfp_Pon_Online_Flag[ PonPortIdx ] = 0 ;
					}
					else
					{
						continue;
					}
				}
				else
				{
					Sfp_Pon_Online_Flag[ PonPortIdx ] = 1;
				}
			}
			else
				continue;

			Counter = GetNum_NotEqual_Zero(Onu_OpticalInfo_Change_Flag[PonPortIdx],MAXONUPERPON);
			if(Counter > OPT_POWER_PON_MAX_NUM)
			{
				VOS_ASSERT(0);
		              return (VOS_ERROR);
			}
			ulDatBufLen = Counter*sizeof(Optical_Msg_PerOnu_t);
			ulLen=sizeof(OpticalToPonMsgHead_t)+ulDatBufLen+sizeof(SYS_MSG_S);
			pMsg=(SYS_MSG_S*)PDP_AllocMsg(ulLen, MODULE_RPU_PON_MON);
		 	if( NULL == pMsg )
		       {
		       	VOS_ASSERT(0);
		              return (VOS_ERROR);
		       }
			VOS_MemZero((CHAR *)pMsg, ulLen );

			SYS_MSG_SRC_ID( pMsg )       = MODULE_RPU_PON_MON;
		        SYS_MSG_DST_ID( pMsg )       = MODULE_RPU_PON_MON;
		        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
			SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
			SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
			SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
			SYS_MSG_SRC_SLOT( pMsg )  = SYS_LOCAL_MODULE_SLOTNO;
			SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
			SYS_MSG_MSG_CODE(pMsg)  = PON_MODULE_INFORMATION;
				
			pQueueMsg= (OpticalToPonMsgHead_t* ) ( pMsg + 1 );
			pQueueMsg->PonId = PonPortIdx;
			pQueueMsg->laseralwaysonsupport = onuLaser_alwaysOn_check_support[PonPortIdx];
			pQueueMsg->transOpticalPower = PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower;
			pQueueMsg->ponTemperature = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature;
			pQueueMsg->ponVoltageApplied = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied;
			pQueueMsg->ponBiasCurrent = PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent;
			pQueueMsg->txAlarmStatus = PonPortTable[PonPortIdx].PonPortmeteringInfo.txAlarmStatus;
			VOS_MemCpy(pQueueMsg->rxAlarmStatus , PonPortTable[PonPortIdx].PonPortmeteringInfo.rxAlarmStatus, MAXONUPERPONNOLIMIT);
			
			if( GetPonPortOpticalMonitorEnable() != V2R1_ENABLE )
				pQueueMsg->sfpMeterSupport = 0;
			else
				pQueueMsg->sfpMeterSupport = PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport;

			pQueueMsg->PonOnlineFlag = Sfp_Pon_Online_Flag[PonPortIdx];

			pQueueMsg->OnuLaserData = onuLaser_alwaysOn_alarm_record[PonPortIdx][0];

			Optical_Msg_Onu_temp = (Optical_Msg_PerOnu_t*)(pQueueMsg+1);
			temp =0;
			OnuEntryBase = PonPortIdx*MAXONUPERPON;
			for(OnuId=0;OnuId<MAXONUPERPON;OnuId++)
			{
				Optical_Msg_Onu =Optical_Msg_Onu_temp ; 
				OnuEntry = OnuEntryBase + OnuId;
				if(Onu_OpticalInfo_Change_Flag[PonPortIdx][OnuId] != 0)
				{
					temp++;
					if(temp > Counter)
					{
						/*VOS_ASSERT(0);
						sys_console_printf("\r\nPlease check the function : Optical_Timer_Callback\r\n");  */
						break;
					}
					Optical_Msg_Onu->OnuId =OnuId;

					if( GetPonPortOpticalMonitorEnable() != V2R1_ENABLE )
						Optical_Msg_Onu->recvPowerFlag = 0;
					else
						Optical_Msg_Onu->recvPowerFlag = PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[OnuId];

					Optical_Msg_Onu->OltRecvOpticalPower = PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuId];
					Optical_Msg_Onu->OnuRecvOpticalPower =  OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower;
					Optical_Msg_Onu->PonBiasCurrent = OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent;
					Optical_Msg_Onu->PonTemperature = OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature;
					Optical_Msg_Onu->PonVoltageApplied = OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied;
					Optical_Msg_Onu->TransOpticalPower = OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower;
					Optical_Msg_Onu->AlarmStatus = OnuMgmtTable[OnuEntry].ONUMeteringTable.AlarmStatus;
					
					if( GetOnuOpticalPowerEnable() != V2R1_ENABLE )
						Optical_Msg_Onu->onu_power_support_flag = 0;
					else
						Optical_Msg_Onu->onu_power_support_flag = OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag;

					Optical_Msg_Onu_temp =  (Optical_Msg_PerOnu_t*)(Optical_Msg_Onu+1);
				}
			}
			if(temp > 0)
				VOS_MemZero(Onu_OpticalInfo_Change_Flag[PonPortIdx], MAXONUPERPONNOLIMIT*sizeof(UCHAR) /*sizeof(Onu_OpticalInfo_Change_Flag[PonPortIdx] )*/ );/*2011.2.15*/

			pQueueMsg->pSendBufLen = ulDatBufLen;

			PDP_Send(RPU_TID_PDP_OPTICAL, SYS_MASTER_ACTIVE_SLOTNO, RPU_TID_PDP_OPTICAL, (VOID *)pMsg, ulLen);
			PDP_FreeMsg(pMsg);
			VOS_TaskDelay(5);
		}
	}

	return VOS_OK;
	
}


STATUS OpticalToPonPdpMsgRxMaster(
    ULONG ulChID,        /* PDP通道号 ，哪个通道进行的报告*/
    ULONG ulSrcNodeID,   /* 数据的源节点号*/
    ULONG ulSrcVpChId,   /* 数据的源PDP通道号*/
    VOID * pData,        /* 接收到的数据 */
    ULONG ulLen        /* 数据的长度*/
)
{
	ULONG rc=VOS_ERROR;
	Optical_Msg_PerOnu_t *Optical_Msg_Onu,*Optical_Msg_Onu_Temp;
	OpticalToPonMsgHead_t * pQueueMsg;
	UplinkPortMeteringInfo_S *uplink_temp;
	optical_12Epon_Uplink_Aux_t *Uplink_Aux;
	unsigned long  Counter = 0,OnuEntry;
	USHORT PonPortIdx,OnuId,i, temp,sfp_sel;
	SYS_MSG_S *pMsg = NULL;
	int portno = 0, src_slot;
	
	if( pData == NULL )
	{
		VOS_ASSERT( 0 );
		return ;
	}
			
	if(GetPonPortOpticalMonitorEnable() != V2R1_ENABLE
		&& GetOnuOpticalPowerEnable() != V2R1_ENABLE )/*当该功能模块未开启时，不用发送*/
	{
		VOS_ASSERT(0);
		PDP_FreeMsg(pData);
		return(RERROR);
	}
	
	if(ulSrcVpChId==RPU_TID_PDP_OPTICAL&&SYS_MASTER_ACTIVE_SLOTNO!=ulSrcNodeID&&ulChID==RPU_TID_PDP_OPTICAL)
	{
		pMsg = (SYS_MSG_S * )pData;

		switch(SYS_MSG_MSG_CODE(pMsg))
		{
			case PON_MODULE_INFORMATION:
				pQueueMsg= (OpticalToPonMsgHead_t* ) (pMsg+ 1 );
				Counter = pQueueMsg->pSendBufLen/sizeof(Optical_Msg_PerOnu_t);
				if(Counter > OPT_POWER_PON_MAX_NUM)
				{
					VOS_ASSERT(0);
					PDP_FreeMsg(pData);
			              return (VOS_ERROR);
				}
				PonPortIdx = GetPonPortIdxBySlot(SYS_MSG_SRC_SLOT( pMsg ), pQueueMsg->PonId+1);
				if(PonPortIdx != VOS_ERROR && PonPortIdx < MAXPON)
				{
				onuLaser_alwaysOn_check_support[PonPortIdx] = pQueueMsg->laseralwaysonsupport ;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = pQueueMsg->transOpticalPower;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature= pQueueMsg->ponTemperature;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = pQueueMsg->ponVoltageApplied ;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = pQueueMsg->ponBiasCurrent;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = pQueueMsg->sfpMeterSupport;
				Sfp_Pon_Online_Flag[PonPortIdx] = pQueueMsg->PonOnlineFlag ;
				PonPortTable[PonPortIdx].PonPortmeteringInfo.txAlarmStatus = pQueueMsg->txAlarmStatus;
				VOS_MemCpy(PonPortTable[PonPortIdx].PonPortmeteringInfo.rxAlarmStatus, pQueueMsg->rxAlarmStatus , MAXONUPERPONNOLIMIT);
				
				temp = GetPonPortIdxBySlot(SYS_MSG_SRC_SLOT( pMsg ), pQueueMsg->PonId+1);

				if( (temp >= 0) && (temp < OPT_POWER_PON_MAX_NUM ) )
					onuLaser_alwaysOn_alarm_record[temp][0] =  pQueueMsg->OnuLaserData;
				
				Optical_Msg_Onu_Temp = (Optical_Msg_PerOnu_t*)(pQueueMsg+1);
				
				for( i= 0; i < Counter; i++)
				{
					Optical_Msg_Onu = Optical_Msg_Onu_Temp;
					OnuId = Optical_Msg_Onu->OnuId;
					OnuEntry = PonPortIdx*MAXONUPERPON + OnuId;

						if(OnuEntry >= 0 && OnuEntry < MAXONU)
						{						
					if( Optical_Msg_Onu->recvPowerFlag == 1)
					{
						PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[OnuId] = 1;
						PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuId] = Optical_Msg_Onu->OltRecvOpticalPower;
					}
					else
					{
						 PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[OnuId] = 0;
						PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[OnuId] = -1000;
					}
					OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag = Optical_Msg_Onu->onu_power_support_flag;

					if(OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag == 1 && ( GetOnuOpticalPowerEnable() == V2R1_ENABLE ) )
					{
						if(Optical_Msg_Onu->TransOpticalPower != 0)
						{
							/*CheckOnuTransOpticalPower(PonPortIdx, OnuId,Optical_Msg_Onu->TransOpticalPower);*/
							OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = Optical_Msg_Onu->TransOpticalPower;
						}
						
						if(Optical_Msg_Onu->OnuRecvOpticalPower != 0)
						{
							/*CheckOnuRecvOpticalPower(PonPortIdx, OnuId, Optical_Msg_Onu->OnuRecvOpticalPower);*/
							OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = Optical_Msg_Onu->OnuRecvOpticalPower;
						}

						if(Optical_Msg_Onu->PonTemperature != 0)
						{
							/*CheckOnuTemperature(PonPortIdx, OnuId, Optical_Msg_Onu->PonTemperature);*/
							OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature = Optical_Msg_Onu->PonTemperature ;
						}

						if(Optical_Msg_Onu->PonVoltageApplied != 0)
						{
							/*CheckOnuWorkVoltage(PonPortIdx, OnuId, Optical_Msg_Onu->PonVoltageApplied);*/
							OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied= Optical_Msg_Onu->PonVoltageApplied;
						}

						if(Optical_Msg_Onu->PonBiasCurrent != 0 )
						{
							/*CheckOnuBiasCurrent(PonPortIdx, OnuId,Optical_Msg_Onu->PonBiasCurrent);*/
							OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent = Optical_Msg_Onu->PonBiasCurrent ;
						}

						OnuMgmtTable[OnuEntry].ONUMeteringTable.AlarmStatus = Optical_Msg_Onu->AlarmStatus;
						
					}
						}
						else
						{
							OPTICAL_DEBUGOUT("OnuEntry = %d\r\n", OnuEntry);
						}
					Optical_Msg_Onu_Temp =(Optical_Msg_PerOnu_t*) (Optical_Msg_Onu+1);
					}
				}
				else
				{
					OPTICAL_DEBUGOUT("PonPortIdx = %d, Entry = %d\r\n", PonPortIdx, OnuEntry);
				}
				rc=VOS_OK;
				break;
			case UPLINK_MODULE_INFORMATION:
				uplink_temp = (UplinkPortMeteringInfo_S*)(pMsg+ 1 );
				Uplink_Aux = (optical_12Epon_Uplink_Aux_t *)(uplink_temp+1);
				
				sfp_sel = Uplink_Aux->PonPortIdx;
				src_slot = SYS_MSG_SRC_SLOT( pMsg );
				portno = src_slot*MAXUPLINKPORT+sfp_sel;

				/*VOS_MemCpy( (VOID*)(&(UplinkPortMeteringInfo[portno])), (VOID*)uplink_temp, sizeof(UplinkPortMeteringInfo_S) );  
				问题单: 15318;  这样就覆盖了AlarmStatus*/
				if(portno >= 0 && portno < SystemMaxUplinkPortNum)
				{
					UplinkPortMeteringInfo[portno].powerMeteringSupport = uplink_temp->powerMeteringSupport;
					UplinkPortMeteringInfo[portno].LosAlarmFlag = uplink_temp->LosAlarmFlag;
				UplinkPortMeteringInfo[portno].SFPType = uplink_temp->SFPType;
				UplinkPortMeteringInfo[portno].resved = uplink_temp->resved;
				UplinkPortMeteringInfo[portno].recvOpticalPower = uplink_temp->recvOpticalPower;
				UplinkPortMeteringInfo[portno].transOpticalPower = uplink_temp->transOpticalPower;
				UplinkPortMeteringInfo[portno].Voltage = uplink_temp->Voltage;
				UplinkPortMeteringInfo[portno].BiasCurrent = uplink_temp->BiasCurrent;

				if( 0 == Uplink_Aux->SFP_Online_Flag && Sfp_Uplink_Online_Flag[ portno ] == 1 )
				{
					ClearAllUplinkAlarmWhenLOS( src_slot, sfp_sel );
				}

				Sfp_Uplink_Online_Flag[ portno ] = Uplink_Aux->SFP_Online_Flag;
				
				UplinkBiasCurrent_Buffer_Flag[portno] = Uplink_Aux->BiasFlag ;
				UplinkVoltageApplied_Buffer_Flag[portno] = Uplink_Aux->VoltFlag;
				UplinkTemperature_Buffer_Flag[portno] = Uplink_Aux->TempFlag;
				UplinkRecvOpticalPower_Buffer_Flag[portno] = Uplink_Aux->RecvFlag;
				UplinkTransOpticalPower_Buffer_Flag[portno] = Uplink_Aux->TranFlag;

				if(uplink_temp->powerMeteringSupport != NOT_SUPPORT_SFF8472 )
				{
					if( UplinkTemperature_Buffer_Flag[portno] == 1 )
						CheckUplinkTemperature( src_slot, sfp_sel, uplink_temp->Temperature );
			
					if( UplinkVoltageApplied_Buffer_Flag[portno] == 1 )
						CheckUplinkVoltage( src_slot, sfp_sel,uplink_temp->Voltage );
				
					if( UplinkBiasCurrent_Buffer_Flag[portno] == 1 )
						CheckUplinkBiasCurrent( src_slot, sfp_sel, uplink_temp->BiasCurrent );
					
			
					if( UplinkTransOpticalPower_Buffer_Flag[portno] == 1 )
						CheckUplinkTransPower( src_slot, sfp_sel, uplink_temp->transOpticalPower );
					
					if( Uplink_Aux->PortStatus == 1 /* IFM_STATE_UP*/ ) 
					{
						if( UplinkRecvOpticalPower_Buffer_Flag[portno]==1)
								CheckUplinkRecvPower( src_slot, sfp_sel, uplink_temp->recvOpticalPower );
						}
					}
				}
				else
				{
					OPTICAL_DEBUGOUT("portno = %d\r\n", portno);
				}
				rc=VOS_OK;
				break;
			default:
				ASSERT(0);
				rc = VOS_ERROR;
				break;
		}
	}
	else
	{
		rc=VOS_ERROR;
	}
	
	PDP_FreeMsg(pData);
	
	return rc;
}

STATUS OpticalPowerSlave()
{
	return VOS_OK;
}

STATUS OpticalToPonPdpInit()
{
    ULONG rc=VOS_ERROR;
    
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)    /*如果是6900的主控板*/
        rc=PDP_Create(RPU_TID_PDP_OPTICAL,OpticalToPonPdpMsgRxMaster);
    else   /*如果是6900的pon板*/
        rc=PDP_Create(RPU_TID_PDP_OPTICAL,OpticalPowerSlave);
    if(rc!=VOS_OK)
    {
       ASSERT(0);
       return rc;
    }
	
    return rc;
}

int UplinkLosTimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_RPU_PON_MON, FC_V2R1_TIMEOUT_UPLINKLOS, 0, 0};
	if(VOS_QueNum(gMQidOpticalPower) > 10) 
		return VOS_OK;
	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		if( VOS_QueSend( gMQidOpticalPower, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("  error: VOS send 1second timer-out to statistics-Queue message err\r\n"  );*/
		}
	}
	return VOS_OK;
}

int UplinkLosTimerStart()
{
	LONG interval = GetUplinkLosCheckInterval() * 1000;
	
	UplinkLosCheckTimerId = VOS_TimerCreate( MODULE_RPU_PON_MON, (unsigned int )0, interval, (void *)UplinkLosTimerCallback, (void *)NULL/*&PonPortIdx*/, VOS_TIMER_LOOP );
	if( UplinkLosCheckTimerId == VOS_ERROR){
		sys_console_printf("\r\nstart ponpowering timer err \r\n ");
		return( RERROR );
		}
	else{
		/*sys_console_printf("\r\nstart uplinkcheck timer OK \r\n ");*/
		return( UplinkLosCheckTimerId );
		}
	
}

int PonpoweringTimerCallback()
{
	unsigned long aulMsg[4] = { MODULE_RPU_PON_MON, FC_V2R1_TIMEOUT, 0, 0};
	if(VOS_QueNum(gMQidOpticalPower) > 10) 
		return VOS_OK;
	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) /*&& !SYS_LOCAL_MODULE_ISMASTERACTIVE*/ )
	{
		if( VOS_QueSend( gMQidOpticalPower, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("  error: VOS send 1second timer-out to statistics-Queue message err\r\n"  );*/
		}
	}
	return VOS_OK;
}

int PonpoweringTimerStart()
{
	LONG interval = GetPonPortOpticalMonitorInterval() * 1000;
	
	PonpoweringTimerId = VOS_TimerCreate( MODULE_RPU_PON_MON, (unsigned int )0, interval, (void *)PonpoweringTimerCallback, (void *)NULL/*&PonPortIdx*/, VOS_TIMER_LOOP );
	if( PonpoweringTimerId == VOS_ERROR){
		sys_console_printf("\r\nstart ponpowering timer err \r\n ");
		return( RERROR );
		}
	else{
		/*sys_console_printf("\r\nstart ponpowering timer OK \r\n ");*/
		return( PonpoweringTimerId );
		}
}
/*extern void CheckPonSFPTypeValid();*/
int PonpoweringTimerHandler()
{
	Pon_Power_Interval_Times++;

	/*CheckPonSFPTypeValid();*/

	Main_OltPonpowermetering();
	Main_OnuPonpowermetering();

	if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
	{
		if(Pon_Power_Interval_Times >= Pon_Power_Interval_Times_Default)
		{
			Optical_Timer_Callback();
			Pon_Power_Interval_Times=0;
		}
	}
	return VOS_OK;
}

int PonpowerMeter_Event( VOID * unused, unsigned long event, IFM_DEVNOTIFYMSG * ptr )
{
    LONG lRet;
    ULONG olt_id=0;
    ULONG ulSlot=0,ulPort=0;

    if ( NULL == ptr )
    {
        ASSERT( 0 );
        return 0;
    }

    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
    {
        return 0;
    }
	
    if( !SYS_IS_ETH_IF(ptr->ulIfIndex))
    {
    	return 0;
    }
	
    if( event !=  NETDEV_DOWN) 
    {
        return 0;
    }

    ulSlot = IFM_ETH_GET_SLOT( ptr->ulIfIndex );
    ulPort = IFM_ETH_GET_PORT( ptr->ulIfIndex );
	
    if( !SYS_MODULE_IS_PON( ulSlot ) )  /* problem num: 18015 */
		return 0;

    olt_id = GetPonPortIdxBySlot( ulSlot, ulPort );
	if(olt_id != VOS_ERROR)
        ClearOnuLaserAlwaysOnAlarmsWhenPon_Loss( olt_id );

    return 0;
}

/*VOID (*check_environment_hook_rtn) (VOID) = NULL;*/
void OpticalPowerTask()
{
	LONG 	lRes = VOS_OK;
	ULONG aulMsg[4] = {0, 0, 0, 0};
/*#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
	LONG other_timer_count = 0;
#endif*/
	

	while( !SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
	{
		VOS_TaskDelay( VOS_TICK_SECOND);
	}

	VOS_TaskDelay( VOS_TICK_SECOND * 60 );
	/*历史统计采样超时消息*/

	if(IFM_DevChainRegisterApi((IFM_NOTIFY_FUNC)PonpowerMeter_Event,0) != VOS_OK)
	{
		ASSERT(0);
	}
	
	PonpoweringTimerStart();
	UplinkLosTimerStart();

	while(1)
	{
		lRes = VOS_QueReceive(gMQidOpticalPower, aulMsg, WAIT_FOREVER);
		if ( lRes == VOS_ERROR || lRes == VOS_NO_MSG  )   
		{
			VOS_ASSERT(0);
			VOS_TaskDelay( 50 );
			continue;
		}

	/*if(SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
		continue;*/
	
		if( aulMsg[1] == FC_V2R1_TIMEOUT )
		{
#if 0
			/*8xep可能导致任务挂死，暂时先屏蔽*/
			if(!SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
			{
				PonpoweringTimerHandler();
			}
#endif
			PonpoweringTimerHandler();
		}
		else if( aulMsg[1] == FC_V2R1_TIMEOUT_UPLINKLOS )
		{
#if 0
			/*8xep可能导致任务挂死，暂时先屏蔽*/
			if(!SYS_LOCAL_MODULE_TYPE_IS_8000_10G_EPON)
			{
				UplinkSFPLOSState_Report();
			}
#endif
			UplinkSFPLOSState_Report();
/*#if( EPON_MODULE_ENVIRONMENT_MONITOR == EPON_MODULE_YES )
			if ( check_environment_hook_rtn )
			{
				other_timer_count++;
				if( other_timer_count >= 2 )
				{
					(*check_environment_hook_rtn)();
					other_timer_count = 0;
				}
			}
#endif*/	/* removed by xieshl 20121016, 由设备管理统一处理 */
		}
	}
	return;
}


int Get_Sfp_Type_Number()
{
	sfp_type_vendor *curNode;
	int counter = 0;

	curNode = HeadSfpType;
	semTake(opticalpower_semid, WAIT_FOREVER);
	while( curNode )
	{
		counter++;
		curNode=curNode->pNext;
	}
	semGive(opticalpower_semid);
	return counter;
}

int  OpticalPower_Poncard_Insert_Callback_2(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	return VOS_OK;
}

int  OpticalPower_Poncard_Insert_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	Optical_Insert_CdpHead_s *CdpHead = NULL;
	eponOpticalPowerThresholds_t  *OpticalThresholds;
	eponOpticalPowerDeadZone_t *PowerDeadZone;
	sfp_type_vendor *curNode = NULL;
	
	int i=0;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
			if( pData == NULL )
			{
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
			
			CdpHead=(Optical_Insert_CdpHead_s *)(( SYS_MSG_S * )pData + 1);

			if(ulDstChId != RPU_TID_CDP_OPTICALPOWER|| ulChID != RPU_TID_CDP_OPTICALPOWER)
			{
				CDP_FreeMsg(pData);
				return VOS_ERROR;
			}
			olt_rx_optical_power_calibration = CdpHead->olt_rx_optical_power_calibration ;
			olt_tx_optical_power_calibration = CdpHead->olt_tx_optical_power_calibration ;
			onu_rx_optical_power_calibration = CdpHead->onu_rx_optical_power_calibration ;
			onu_tx_optical_power_calibration = CdpHead->onu_tx_optical_power_calibration ;
			onuLaser_alwaysOn_Enable = CdpHead->onuLaser_alwaysOn_Enable ;
			onuLaser_alwaysOn_alarm_timeCounter = CdpHead->onuLaser_alwaysOn_alarm_timeCounter;
			onuLaser_alwaysOn_clear_timeCounter = CdpHead->onuLaser_alwaysOn_clear_timeCounter;
			onuLaser_alwaysOn_alarm_threshold = CdpHead->onuLaser_alwaysOn_alarm_threshold ;
			sfp_debug_switch = CdpHead->sfp_debug_switch;
			

			OpticalThresholds= (eponOpticalPowerThresholds_t *)(CdpHead+1);
			VOS_MemCpy(&eponOpticalPowerThresholds, OpticalThresholds, sizeof(eponOpticalPowerThresholds_t));

			PowerDeadZone= (eponOpticalPowerDeadZone_t *)(OpticalThresholds+1);
			VOS_MemCpy(&eponOpticalPowerDeadZone, PowerDeadZone, sizeof(eponOpticalPowerDeadZone_t));	

			if(CdpHead->sfp_type_num != 0)
			{
				curNode = (sfp_type_vendor *)(PowerDeadZone+1);
				for(i=0; i<CdpHead->sfp_type_num; i++)
				{
					Sfp_Type_Add(curNode->type_name);
					curNode = curNode+1;
				}
			}
			CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

	return VOS_OK;
}


int OpticalPower_Poncard_Insert( LONG slotno, LONG module_type  )
{
       Optical_Insert_CdpHead_s *CdpHead = NULL;
	eponOpticalPowerThresholds_t  *OpticalThresholds;
	eponOpticalPowerDeadZone_t *PowerDeadZone;
	sfp_type_vendor *curNode = NULL, *curNode2;
	int number, i,j, PonPortIdx, OnuEntry;
	int ulLen = 0, counter = 0;
    	SYS_MSG_S      *pMsg       = NULL;

	/*number = SYS_MODULE_SLOT_PORT_NUM( slotno );*/
	/* 这个宏在这是还未起作用*/
	if (module_type <= MODULE_TYPE_UNKNOW)
		return VOS_OK;
	if( SYS_MODULE_IS_UPLINK(slotno) )
	{
		for( i=MAXUPLINKPORT*slotno; i< MAXUPLINKPORT*(slotno+1); i++ )/*每个上联口对应一项*/
		{
			UplinkPortMeteringInfo[i].AlarmStatus = 0;
			UplinkPortMeteringInfo[i].LosAlarmFlag = 2;
			UplinkPortMeteringInfo[i].powerMeteringSupport = NOT_SUPPORT_SFF8472;
			UplinkPortMeteringInfo[i].transOpticalPower = -50;
			UplinkPortMeteringInfo[i].recvOpticalPower = -50;
			UplinkPortMeteringInfo[i].Temperature = 40;
			UplinkPortMeteringInfo[i].Voltage = 33;
			UplinkPortMeteringInfo[i].BiasCurrent = 15;
		}
		
		return VOS_OK;
	}
	
	if( !SYS_MODULE_TYPE_IS_CPU_PON(module_type) )
		return VOS_OK;

	if( SYS_MODULE_TYPE_IS_4EPON(module_type) )
		number = 4;
	else if( SYS_MODULE_TYPE_IS_8EPON(module_type) )
		number = 8;
	else if( SYS_MODULE_TYPE_IS_12EPON(module_type) )
		number = 12;
	else if( SYS_MODULE_TYPE_IS_16EPON(module_type) )
		number = 16;
#if defined(_EPON_10G_PMC_SUPPORT_)            
	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	else if( SYS_MODULE_TYPE_IS_6900_10G_EPON(module_type) )
	{
		number = 1;
	}
	/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
	else
		number = 0;
	
	for( j = 1; j <= number; j++ )
	{
		PonPortIdx = GetPonPortIdxBySlot( slotno, j);

		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = 0;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType=PON_POLARITY_POSITIVE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = PON_OPTICAL_POWER_TRANSMIT_DEFAULT;	/*发送光功率*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature = 50;		/*模块温度*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = 30;	/*模块电压*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = 20;		/*偏置电流*/

		PonPortTable[PonPortIdx].PonPortmeteringInfo.txAlarmStatus = 0;
		
		for( i=0; i<MAXONUPERPONNOLIMIT; i++ )
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[i] = -1000;
			PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[i] = 0;
			PonPortTable[PonPortIdx].PonPortmeteringInfo.rxAlarmStatus[i] = 0;

			OnuEntry = PonPortIdx*MAXONUPERPONNOLIMIT + i;

			if(OnuEntry < MAXONU )
			{
				OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = -200;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = 30;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.ponTemperature = 50;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.ponVoltageApplied = 30;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.ponBiasCurrent = 20;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.onu_power_support_flag = 0;
				OnuMgmtTable[OnuEntry].ONUMeteringTable.AlarmStatus = 0;
			}
		}	
	}
	
	counter = Get_Sfp_Type_Number();
	if(counter == 1 && VOS_MemCmp(HeadSfpType->type_name, SFP_TYPE_Vendor_Name_WTD, SFP_TYPE_Vendor_len) == 0 
		&& HeadSfpType->defaultType == 1)
	{
		counter = 0;
	}
	
       ulLen=sizeof(Optical_Insert_CdpHead_s)+sizeof(SYS_MSG_S)+sizeof(eponOpticalPowerThresholds_t)
	   	+sizeof(eponOpticalPowerDeadZone_t)+counter*sizeof(sfp_type_vendor);
	
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_RPU_PON_MON);

	/*sys_console_printf("\r\n\r\nIn Function : EthPortLoop_Poncard_Insert\r\n\r\n");*/
       if( NULL == pMsg )
       {
            VOS_ASSERT(0);
            return  VOS_ERROR;
       }
       VOS_MemZero((CHAR *)pMsg, ulLen );

       SYS_MSG_SRC_ID( pMsg )       = MODULE_RPU_PON_MON;
        SYS_MSG_DST_ID( pMsg )       = MODULE_RPU_PON_MON;
        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
        SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;

       CdpHead= (Optical_Insert_CdpHead_s * ) ( pMsg + 1 );
	CdpHead->olt_rx_optical_power_calibration =olt_rx_optical_power_calibration;
	CdpHead->olt_tx_optical_power_calibration =olt_tx_optical_power_calibration;
	CdpHead->onu_rx_optical_power_calibration =onu_rx_optical_power_calibration;
	CdpHead->onu_tx_optical_power_calibration =onu_tx_optical_power_calibration;
	CdpHead->onuLaser_alwaysOn_Enable =onuLaser_alwaysOn_Enable;
	CdpHead->onuLaser_alwaysOn_alarm_timeCounter =onuLaser_alwaysOn_alarm_timeCounter;
	CdpHead->onuLaser_alwaysOn_clear_timeCounter = onuLaser_alwaysOn_clear_timeCounter;
	CdpHead->onuLaser_alwaysOn_alarm_threshold = onuLaser_alwaysOn_alarm_threshold;
	CdpHead->sfp_debug_switch =sfp_debug_switch;
	CdpHead->sfp_type_num = counter;
		
	OpticalThresholds= (eponOpticalPowerThresholds_t *)(CdpHead+1);
	VOS_MemCpy(OpticalThresholds, &eponOpticalPowerThresholds, sizeof(eponOpticalPowerThresholds_t));

	PowerDeadZone= (eponOpticalPowerDeadZone_t *)(OpticalThresholds+1);
	VOS_MemCpy(PowerDeadZone, &eponOpticalPowerDeadZone, sizeof(eponOpticalPowerDeadZone_t));	

	if(counter != 0)
	{
		curNode = HeadSfpType;
		curNode2 = (sfp_type_vendor *)(PowerDeadZone+1);
		semTake(opticalpower_semid, WAIT_FOREVER);
		while(curNode != NULL)
		{
			curNode2->defaultType = curNode->defaultType;
			VOS_MemCpy( curNode2->type_name, curNode->type_name, SFP_TYPE_Vendor_len);
			curNode2->pNext = curNode2->pNext;
			curNode2 = curNode2+1;
			curNode = curNode->pNext;
		}
		semGive(opticalpower_semid);
	}
	
       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_OPTICALPOWER, slotno, RPU_TID_CDP_OPTICALPOWER, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_RPU_PON_MON ) )
	{
		VOS_ASSERT(0); 
		sys_console_printf("\r\n%%Send the optical power configuration to the new poncard Failed !\r\n");
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}
	/*else
	 {
		sys_console_printf("Send the optical power configuration to the new poncard successed ! \r\n");
	 }*/

	return VOS_OK;

}

void PonPowerMeteringInit(void)
{
	short int PonPortIdx,i;
	char *pTaskName = "tOpticalPower";

	/* modified by xieshl 20160531, 8XEP光模块发送光功率和偏置电流的量化单位和1G光模块不同，
	    但没有在光模块中找到量化单位的存储位置，暂时先这么改，以后可根据光模块类型完善 */
	if( (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA8000_10G_8EPON) || (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA8000_10G_8EPON_M) )
	{
		opt_tx_pwr_qunit = 2;	/* LHT4301 量化单位0.2uW */
		opt_biascur_qunit = 4;	/* 量化单位4uA */
	}
	
    if (gOpticalPowerInitStatus != 0)
	{
		sys_console_printf(" OpticalPowerInit task is already running!\r\n");
		return ;
	}

	gMQidOpticalPower = VOS_QueCreate(500, VOS_MSG_Q_PRIORITY);
	if (gMQidOpticalPower==0)  
	{
		return ;
	}

	OpticalPowerTaskId = VOS_TaskCreate( pTaskName, (ULONG)OPTICALPOWER_TASK_PRIO, (VOS_TASK_ENTRY)OpticalPowerTask, NULL);
	if(OpticalPowerTaskId == 0)
	{
		if( gMQidOpticalPower ) VOS_QueDelete(gMQidOpticalPower);
		sys_console_printf("It creats the opticalpower task failed !!\r\n");
		return ;
	}
	VOS_QueBindTask( (void*)OpticalPowerTaskId, gMQidOpticalPower );

	gOpticalPowerInitStatus = 1;
	sys_console_printf(" create the OpticalPower task ... OK\r\n");
	/*return (STATS_OK);*/

	( VOID ) CDP_SYNC_Register( MODULE_RPU_PON_MON, OpticalPower_CMD2LIC_RPC_Callback );/*主要负责命令的RPC 执行*/
	
	OpticalToPonPdpInit();/*该PDP 通道负责定时从PON Card 上向主控同步它上面的更新*/

	if(Size_S_rpcIfs != Size_M_rpcIfs)  
		VOS_ASSERT(0);

	pM_rpcIfs = &M_rpcIfs;
	pS_rpcIfs = &S_rpcIfs;
	/*if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		OptPowRpcIfs = &S_rpcIfs ;
	else
		OptPowRpcIfs = &M_rpcIfs ;*/

	if(MAXPON > OPT_POWER_PON_MAX_NUM)
	{
		sys_console_printf("Error, MAXPON is bigger then  OPT_POWER_PON_MAX_NUM !\r\n");
		return ;
	}
	
	if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_SW || SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA8000_SW ))
	{
		CDP_Create( RPU_TID_CDP_OPTICALPOWER,  CDP_NOTI_VIA_FUNC, 0, OpticalPower_Poncard_Insert_Callback_2) ;
	}
	else
	{
		/*通过该CDP  通道，热插拔时，主控向PON 发送数据，下面的函数负责接收*/
		CDP_Create( RPU_TID_CDP_OPTICALPOWER,  CDP_NOTI_VIA_FUNC, 0, OpticalPower_Poncard_Insert_Callback) ;
	}
	
	for( PonPortIdx= 0; PonPortIdx< MAXPON; PonPortIdx++ )/*每个PON 对应一项*/
	{
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfpMeteringSupport = 0;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPInitType=PON_POLARITY_POSITIVE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.powerMeteringSupport = V2R1_DISABLE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
		PonPortTable[PonPortIdx].PonPortmeteringInfo.transOpticalPower = PON_OPTICAL_POWER_TRANSMIT_DEFAULT;	/*发送光功率*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponTemperature = 50;		/*模块温度*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponVoltageApplied = 30;	/*模块电压*/
		PonPortTable[PonPortIdx].PonPortmeteringInfo.ponBiasCurrent = 20;		/*偏置电流*/

		PonPortTable[PonPortIdx].PonPortmeteringInfo.txAlarmStatus = 0;
		for( i=0; i<MAXONUPERPONNOLIMIT; i++ )
		{
			PonPortTable[PonPortIdx].PonPortmeteringInfo.recvOpticalPower[i] = -1000;
			PonPortTable[PonPortIdx].PonPortmeteringInfo.recvPowerFlag[i] = 0;
			PonPortTable[PonPortIdx].PonPortmeteringInfo.rxAlarmStatus[i] = 0;
		}
	}

	for( i = 0; i<MAXONU; i++ )/*每个ONU 对应一项*/
	{	
		OnuMgmtTable[i].ONUMeteringTable.recvOpticalPower = -200;
		OnuMgmtTable[i].ONUMeteringTable.transOpticalPower = 30;
		OnuMgmtTable[i].ONUMeteringTable.ponTemperature = 50;
		OnuMgmtTable[i].ONUMeteringTable.ponVoltageApplied = 30;
		OnuMgmtTable[i].ONUMeteringTable.ponBiasCurrent = 20;
		OnuMgmtTable[i].ONUMeteringTable.onu_power_support_flag = 0;
		OnuMgmtTable[i].ONUMeteringTable.AlarmStatus = 0;
	}
	for(i=0;i<MAXUPLINKPORT*SYS_CHASSIS_SWITCH_SLOTNUM;i++)/*每个上联口对应一项*/
	{
		UplinkPortMeteringInfo[i].AlarmStatus = 0;
		UplinkPortMeteringInfo[i].LosAlarmFlag = 2;
		UplinkPortMeteringInfo[i].powerMeteringSupport = NOT_SUPPORT_SFF8472;
		UplinkPortMeteringInfo[i].transOpticalPower = -50;
		UplinkPortMeteringInfo[i].recvOpticalPower = -50;
		UplinkPortMeteringInfo[i].Temperature = 40;
		UplinkPortMeteringInfo[i].Voltage = 33;
		UplinkPortMeteringInfo[i].BiasCurrent = 15;
	}
	VOS_MemSet(PonRecvOpticalPower_Buffer, 0, sizeof(PonRecvOpticalPower_Buffer)); 
	VOS_MemSet(PonRecvOpticalPower_Buffer_Index, 0, sizeof(PonRecvOpticalPower_Buffer_Index)); 
	VOS_MemSet(PonRecvOpticalPower_Buffer_Flag, 0, sizeof(PonRecvOpticalPower_Buffer_Flag)); 
	
	VOS_MemSet(PonTransOpticalPower_Buffer, 0, sizeof(PonTransOpticalPower_Buffer)); 
	VOS_MemSet(PonTransOpticalPower_Buffer_Index, 0, sizeof(PonTransOpticalPower_Buffer_Index)); 
	VOS_MemSet(PonTransOpticalPower_Buffer_Flag, 0, sizeof(PonTransOpticalPower_Buffer_Flag)); 
	
	VOS_MemSet(PonTemperature_Buffer, 0, sizeof(PonTemperature_Buffer)); 
	VOS_MemSet(PonTemperature_Buffer_Index, 0, sizeof(PonTemperature_Buffer_Index)); 
	VOS_MemSet(PonTemperature_Buffer_Flag, 0, sizeof(PonTemperature_Buffer_Flag));
	
	VOS_MemSet(PonVoltageApplied_Buffer, 0, sizeof(PonVoltageApplied_Buffer)); 
	VOS_MemSet(PonVoltageApplied_Buffer_Index, 0, sizeof(PonVoltageApplied_Buffer_Index)); 
	VOS_MemSet(PonVoltageApplied_Buffer_Flag, 0, sizeof(PonVoltageApplied_Buffer_Flag)); 
	
	VOS_MemSet(PonBiasCurrent_Buffer, 0, sizeof(PonBiasCurrent_Buffer)); 
	VOS_MemSet(PonBiasCurrent_Buffer_Index, 0, sizeof(PonBiasCurrent_Buffer_Index)); 
	VOS_MemSet(PonBiasCurrent_Buffer_Flag, 0, sizeof(PonBiasCurrent_Buffer_Flag)); 

	VOS_MemSet(UplinkRecvOpticalPower_Buffer, 0, sizeof(UplinkRecvOpticalPower_Buffer)); 
	VOS_MemSet(UplinkRecvOpticalPower_Buffer_Index, 0, sizeof(UplinkRecvOpticalPower_Buffer_Index)); 
	VOS_MemSet(UplinkRecvOpticalPower_Buffer_Flag, 0, sizeof(UplinkRecvOpticalPower_Buffer_Flag)); 
	
	VOS_MemSet(UplinkTransOpticalPower_Buffer, 0, sizeof(UplinkTransOpticalPower_Buffer)); 
	VOS_MemSet(UplinkTransOpticalPower_Buffer_Index, 0, sizeof(UplinkTransOpticalPower_Buffer_Index)); 
	VOS_MemSet(UplinkTransOpticalPower_Buffer_Flag, 0, sizeof(UplinkTransOpticalPower_Buffer_Flag)); 
	
	VOS_MemSet(UplinkTemperature_Buffer, 0, sizeof(UplinkTemperature_Buffer)); 
	VOS_MemSet(UplinkTemperature_Buffer_Index, 0, sizeof(UplinkTemperature_Buffer_Index)); 
	VOS_MemSet(UplinkTemperature_Buffer_Flag, 0, sizeof(UplinkTemperature_Buffer_Flag));
	
	VOS_MemSet(UplinkVoltageApplied_Buffer, 0, sizeof(UplinkVoltageApplied_Buffer)); 
	VOS_MemSet(UplinkVoltageApplied_Buffer_Index, 0, sizeof(UplinkVoltageApplied_Buffer_Index)); 
	VOS_MemSet(UplinkVoltageApplied_Buffer_Flag, 0, sizeof(UplinkVoltageApplied_Buffer_Flag)); 
	
	VOS_MemSet(UplinkBiasCurrent_Buffer, 0, sizeof(UplinkBiasCurrent_Buffer)); 
	VOS_MemSet(UplinkBiasCurrent_Buffer_Index, 0, sizeof(UplinkBiasCurrent_Buffer_Index)); 
	VOS_MemSet(UplinkBiasCurrent_Buffer_Flag, 0, sizeof(UplinkBiasCurrent_Buffer_Flag)); 

	/* ONU 侧光模块收发光功率告警门限初始化*/
	eponOpticalPowerThresholds.recvOPlow = PON_RECV_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.recvOPhigh = PON_RECV_OPTICAL_POWER_HIGH;
	eponOpticalPowerThresholds.tranOPlow = PON_TRAN_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.tranOPhigh=PON_TRAN_OPTICAL_POWER_HIGH;
	/* olt 侧光模块收发光功率告警门限初始化*/	
	eponOpticalPowerThresholds.OltRecvOPlow[0] = RECV_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.OltRecvOPhigh[0] = RECV_OPTICAL_POWER_HIGH;	
	eponOpticalPowerThresholds.OltTranOPlow[0] = TRAN_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.OltTranOPhigh[0] = TRAN_OPTICAL_POWER_HIGH;
	eponOpticalPowerThresholds.OltRecvOPlow[1] = RECV_OPTICAL_POWER_LOW_10GE;
	eponOpticalPowerThresholds.OltRecvOPhigh[1] = RECV_OPTICAL_POWER_HIGH_10GE;	
	eponOpticalPowerThresholds.OltTranOPlow[1] = TRAN_OPTICAL_POWER_LOW_10GE;
	eponOpticalPowerThresholds.OltTranOPhigh[1] = TRAN_OPTICAL_POWER_HIGH_10GE;
	
	eponOpticalPowerThresholds.ponTemLow = PON_MODULE_TEMPERATURE_LOW;
	eponOpticalPowerThresholds.ponTemHigh = PON_MODULE_TEMPERATURE_HIGH;
	eponOpticalPowerThresholds.OltTemLow[0] = PON_MODULE_TEMPERATURE_LOW;
	eponOpticalPowerThresholds.OltTemHigh[0] = PON_MODULE_TEMPERATURE_HIGH;
	eponOpticalPowerThresholds.OltTemLow[1] = PON_MODULE_TEMPERATURE_LOW_10GE;
	eponOpticalPowerThresholds.OltTemHigh[1] = PON_MODULE_TEMPERATURE_HIGH_10GE;
	
	eponOpticalPowerThresholds.ponVolLow = PON_MODULE_VOLTAGE_LOW;
	eponOpticalPowerThresholds.ponVolHigh = PON_MODULE_VOLTAGE_HIGH;
	eponOpticalPowerThresholds.OltVolLow[0] = PON_MODULE_VOLTAGE_LOW;
	eponOpticalPowerThresholds.OltVolHigh[0] = PON_MODULE_VOLTAGE_HIGH;
	eponOpticalPowerThresholds.OltVolLow[1] = PON_MODULE_VOLTAGE_LOW_10GE;
	eponOpticalPowerThresholds.OltVolHigh[1] = PON_MODULE_VOLTAGE_HIGH_10GE;
	
	eponOpticalPowerThresholds.ponBiasCurLow = PON_MODULE_BIAS_CURRENT_LOW;
	eponOpticalPowerThresholds.ponBiasCurHigh = PON_MODULE_BIAS_CURRENT_HIGH;
	eponOpticalPowerThresholds.OltBiasCurLow[0] = PON_MODULE_BIAS_CURRENT_LOW;
	eponOpticalPowerThresholds.OltBiasCurHigh[0] = PON_MODULE_BIAS_CURRENT_HIGH;
	eponOpticalPowerThresholds.OltBiasCurLow[1] = PON_MODULE_BIAS_CURRENT_LOW_10GE;
	eponOpticalPowerThresholds.OltBiasCurHigh[1] = PON_MODULE_BIAS_CURRENT_HIGH_10GE;
	
	eponOpticalPowerThresholds.OltLaserAlwaysOnThresh = 0;
	eponOpticalPowerThresholds.OltMonLaserAlwaysOnThresh = POWER_METERING_ENABLE_DEFAULT;
	eponOpticalPowerThresholds.OltMonitorEnable = POWER_METERING_ENABLE_DEFAULT;
	eponOpticalPowerThresholds.OltMonInterval = POWER_METERING_INTERVAL_DEFAULT;

	eponOpticalPowerDeadZone.curVarDeadZone = CURRENT_ALARM_DEADZONE_DEFAULT;
	eponOpticalPowerDeadZone.powerVarDeadZone = POWER_ALARM_DEADZONE_DEFAULT;
	eponOpticalPowerDeadZone.temVarDeadZone = TEMERATURE_ALARM_DEADZONE_DEFAULT;
	eponOpticalPowerDeadZone.volVarDeadZone = VOLTAGE_ALARM_DEADZONE_DEFAULT;
	
	/*上联口光模块告警门限初始化*/
	eponOpticalPowerThresholds.upportrecvOPhigh[0] = UPLINK_RECV_OPTICAL_POWER_HIGH;
	eponOpticalPowerThresholds.upportrecvOPlow[0] = UPLINK_RECV_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.upporttransOPhigh[0] = UPLINK_TRAN_OPTICAL_POWER_HIGH;
	eponOpticalPowerThresholds.upporttransOPlow[0] = UPLINK_TRAN_OPTICAL_POWER_LOW;
	eponOpticalPowerThresholds.upportTemhigh[0] = UPLINK_MODULE_TEMPERATURE_HIGH;
	eponOpticalPowerThresholds.upportTemlow[0] = UPLINK_MODULE_TEMPERATURE_LOW;
	eponOpticalPowerThresholds.upportVolhigh[0] = UPLINK_MODULE_VOLTAGE_HIGH;
	eponOpticalPowerThresholds.upportVollow[0] = UPLINK_MODULE_VOLTAGE_LOW;
	eponOpticalPowerThresholds.upportCurhigh[0] = UPLINK_MODULE_BIAS_CURRENT_HIGH;
	eponOpticalPowerThresholds.upportCurlow[0] = UPLINK_MODULE_BIAS_CURRENT_LOW;
	eponOpticalPowerThresholds.UplinkMonInterval = 5;
	eponOpticalPowerThresholds.upportrecvOPhigh[1] = UPLINK_RECV_OPTICAL_POWER_HIGH_10GE;
	eponOpticalPowerThresholds.upportrecvOPlow[1] = UPLINK_RECV_OPTICAL_POWER_LOW_10GE;
	eponOpticalPowerThresholds.upporttransOPhigh[1] = UPLINK_TRAN_OPTICAL_POWER_HIGH_10GE;
	eponOpticalPowerThresholds.upporttransOPlow[1] = UPLINK_TRAN_OPTICAL_POWER_LOW_10GE;
	eponOpticalPowerThresholds.upportTemhigh[1] = UPLINK_MODULE_TEMPERATURE_HIGH_10GE;
	eponOpticalPowerThresholds.upportTemlow[1] = UPLINK_MODULE_TEMPERATURE_LOW_10GE;
	eponOpticalPowerThresholds.upportVolhigh[1] = UPLINK_MODULE_VOLTAGE_HIGH_10GE;
	eponOpticalPowerThresholds.upportVollow[1] = UPLINK_MODULE_VOLTAGE_LOW_10GE;
	eponOpticalPowerThresholds.upportCurhigh[1] = UPLINK_MODULE_BIAS_CURRENT_HIGH_10GE;
	eponOpticalPowerThresholds.upportCurlow[1] = UPLINK_MODULE_BIAS_CURRENT_LOW_10GE;

	onuLaser_alwaysOn_alarm_threshold = OnuLaser_AlwaysOn_Alarm_Threshold_Default;
	VOS_MemZero( onuLaser_alwaysOn_alarm_flag, sizeof(onuLaser_alwaysOn_alarm_flag) );
	VOS_MemZero( onuLaser_alwaysOn_alarm_record, sizeof(onuLaser_alwaysOn_alarm_record) );
	VOS_MemZero( Sfp_Pon_Online_Flag, sizeof(Sfp_Pon_Online_Flag) );
	VOS_MemZero( Sfp_Uplink_Online_Flag, sizeof(Sfp_Uplink_Online_Flag) );
	onuLaser_alwaysOn_alarm_timeCounter = OnuLaser_AlwaysOn_Alarm_TimeCounter_Default;
	onuLaser_alwaysOn_clear_timeCounter = OnuLaser_AlwaysOn_Clear_TimeCounter_Default;
	VOS_MemZero( onuLaser_alwaysOn_check_support, sizeof(onuLaser_alwaysOn_check_support) );/*是不是支持强发光检测标志*/
	VOS_MemZero(Onu_OpticalInfo_Change_Flag,sizeof(Onu_OpticalInfo_Change_Flag));
	VOS_MemZero(setUplinkPortSFPUpdatedFlag,sizeof(setUplinkPortSFPUpdatedFlag));
	InitSFPTypeString();/*与命令pon-adc negative-polarity sfp-type <name> 相关*/
	opticalpower_semid = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	if( opticalpower_semid == 0 )
	{
		/*sys_console_printf( "initialize EthLoop SemId fail!\r\n" );*/
		VOS_ASSERT(0);
	}

	OpticalPower_module_init();
}

#endif	/*EPON_MODULE_PON_OPTICAL_POWER*/
