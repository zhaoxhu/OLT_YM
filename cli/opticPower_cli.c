
/*modle name:opticPower_cli.c*/
/*creat by shixh@20080901*/
#ifdef __cplusplus
extern "C" {
#endif




#include "OltGeneral.h"
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )

#include "gwEponSys.h"
#include "gwEponMibData.h"

#include "lib_eponOpticPowerMon.h"


DEFUN  (
    optical_power_threshold_onu_config,
    optical_power_threshold_onu_config_cmd,
    "optical-power-threshold onu <Tx_high> <Tx_low> <Rx_high> <Rx_low>",
    "config optical power threshold\n"
    "config onu optical power threshold\n"
    "transmission power high\n"
     "transmission power low\n"
     "receiver power high\n"
     "receiver power low\n"
    )
{
int  field;
long  tx_high_val,tx_low_val,rx_high_val,rx_low_val;

if(argc!=4)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

tx_high_val = VOS_AtoL(argv[0]);
tx_low_val = VOS_AtoL(argv[1]);
rx_high_val = VOS_AtoL(argv[2]);
rx_low_val = VOS_AtoL(argv[3]);

setOpticalPowerThreshold(field_recv_oppower_low, rx_low_val);
setOpticalPowerThreshold(field_recv_oppower_high, rx_high_val);
setOpticalPowerThreshold(field_trans_oppower_low, tx_low_val);
setOpticalPowerThreshold(field_trans_oppower_high, tx_high_val);

return VOS_OK;
}

DEFUN  (
    show_optical_power_threshold_onu,
    show_optical_power_threshold_onu_cmd,
    "show optical-power-threshold onu",
    "show optical power information\n"
    "show optical power threshold\n"
    )
{
long  tx_high_val,tx_low_val,rx_high_val,rx_low_val;
rx_high_val=getOpticalPowerThreshold(field_recv_oppower_high);
vty_out(vty, "receiver high is:%d\r\n",rx_high_val);
rx_low_val=getOpticalPowerThreshold(field_recv_oppower_low);
vty_out(vty, "receiver low is:%d\r\n",rx_low_val);
tx_high_val=getOpticalPowerThreshold(field_trans_oppower_high);
vty_out(vty, "transmission high is:%d\r\n",tx_high_val);
tx_low_val=getOpticalPowerThreshold(field_trans_oppower_low);
vty_out(vty, "transmission low is:%d\r\n",tx_low_val);
return VOS_OK;

}


DEFUN  (
    optical_power_threshold_olt_config,
    optical_power_threshold_olt_config_cmd,
    "optical-power-threshold olt <Tx_high> <Tx_low> <Rx_high> <Rx_low>",
    "config optical power threshold\n"
    "config onu optical power threshold\n"
    "transmission power high\n"
    "transmission power low\n"
    "receiver power high\n"
    "receiver power low\n"
    )
{
int  field;
long  tx_high_val,tx_low_val,rx_high_val,rx_low_val;

if(argc!=4)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

tx_high_val = VOS_AtoL(argv[0]);
tx_low_val = VOS_AtoL(argv[1]);
rx_high_val = VOS_AtoL(argv[2]);
rx_low_val = VOS_AtoL(argv[3]);

setOpticalPowerThreshold(field_olt_recv_oppower_low, rx_low_val);
setOpticalPowerThreshold(field_olt_recv_oppower_high, rx_high_val);
setOpticalPowerThreshold(field_olt_trans_oppower_low, rx_high_val);
setOpticalPowerThreshold(field_olt_trans_oppower_high, tx_low_val);

return VOS_OK;
}

DEFUN  (
    optical_power_temperature_onu_config,
    optical_power_temperature_onu_config_cmd,
    "optical-temperature onu <high> <low>",
    "config optical temperature\n"
    "config onu optical temperature\n"
    "module temperature high\n"
    "module temperature low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_pon_tempe_high, high_val);
setOpticalPowerThreshold(field_pon_tempe_low, low_val);

return VOS_OK;
}
DEFUN  (
    optical_power_temperature_olt_config,
    optical_power_temperature_olt_config_cmd,
    "optical-temperature olt <high> <low>",
    "config optical temperature\n"
    "config olt optical temperature\n"
    "module temperature high\n"
    "module temperature low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_olt_tempe_high, high_val);
setOpticalPowerThreshold(field_olt_tempe_low, low_val);

return VOS_OK;
}
DEFUN  (
    optical_power_voltage_onu_config,
    optical_power_voltage_onu_config_cmd,
    "optical-voltage onu <high> <low>",
    "config optical voltage\n"
    "config onu optical voltage\n"
    "applied voltage high\n"
    "applied voltage low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_pon_vol_high, high_val);
setOpticalPowerThreshold(field_pon_vol_low, low_val);

return VOS_OK;
}
DEFUN  (
    optical_power_voltage_olt_config,
    optical_power_voltage_olt_config_cmd,
    "optical-voltage olt <high> <low>",
    "config optical voltage\n"
    "config olt optical voltage\n"
    "applied voltage high\n"
    "applied voltage low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_olt_vol_high, high_val);
setOpticalPowerThreshold(field_olt_vol_low, low_val);

return VOS_OK;
}
DEFUN  (
    optical_power_current_onu_config,
    optical_power_current_onu_config_cmd,
    "optical-current onu <high> <low>",
    "config optical current\n"
    "config onu optical current\n"
    "bias current high\n"
    "bias current low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_pon_cur_high, high_val);
setOpticalPowerThreshold(field_pon_cur_low, low_val);

return VOS_OK;
}
DEFUN  (
    optical_power_current_olt_config,
    optical_power_current_olt_config_cmd,
    "optical-current olt <high> <low>",
    "config optical current\n"
    "config olt optical current\n"
    "bias current high\n"
    "bias current low\n"
    )
{
int  field;
long  high_val,low_val;

if(argc!=2)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

high_val = VOS_AtoL(argv[0]);
low_val = VOS_AtoL(argv[1]);

setOpticalPowerThreshold(field_olt_cur_high, high_val);
setOpticalPowerThreshold(field_olt_cur_low, low_val);

return VOS_OK;
}
DEFUN  (
    onu_laser_always_on_threshold_config,
    onu_laser_always_on_threshold_config_cmd,
    "onu-laser-always-on-threshold <power>",
    "onu laser always on threshold\n"
    "power value\n"
    )
{
int  field;
long  threshold;

if(argc!=1)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

threshold = VOS_AtoL(argv[0]);

setOpticalPowerThreshold(field_olt_laser_thresh, threshold);

return VOS_OK;
}

DEFUN  (
    optical_mon_dead_zone_config,
     optical_mon_dead_zone_config_cmd,
    "optical-mon-dead-zone <power> <tempertaure> <voltage> <current>",
    "config optical monitor dead zone\n"
    "power value\n"
    "temperature value\n"
    "voltage value\n"
    "current value\n"
    )
{
int  field;
long  power,temperature,voltage,current;

if(argc!=4)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}

power = VOS_AtoL(argv[0]);
temperature = VOS_AtoL(argv[1]);
voltage = VOS_AtoL(argv[2]);
current = VOS_AtoL(argv[3]);

setOpticalPowerDeadZone(field_power_dead_zone, power);
setOpticalPowerDeadZone(field_tempe_dead_zone, temperature);
setOpticalPowerDeadZone(field_vol_dead_zone, voltage);
setOpticalPowerDeadZone(field_cur_dead_zone, current);

return VOS_OK;
}
DEFUN  (
    optical_monitor_enable,
    optical_monitor_enable_cmd,
    "optical-monitor-enable [enable|disable]",
    "optical montior enable\n"
    "enable\n"
    "disable\n"
    )
{
int  field;
long  threshold;

if(argc!=1)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}
 if (VOS_StrCmp( argv[0], "enable")==0)
 	{
 	setOpticalPowerThreshold(field_olt_monitor_enable, 1);
 	}
 else
 	{
 	setOpticalPowerThreshold(field_olt_monitor_enable, 2);
 	}

return VOS_OK;
}
DEFUN  (
    optical_monitor_interval,
    optical_monitor_interval_cmd,
    "optical-monitor-interval <value>",
    "config optical montior enable\n"
    "the period of the monitor,stated in second\n"
    )
{
int  field;
long  interval;

if(argc!=1)
{
	vty_out(vty, " %% Parameter err\r\n");
	return( CMD_WARNING );
}
interval = VOS_AtoL(argv[0]);

setOpticalPowerThreshold(field_olt_mon_interval, interval);
 	
return VOS_OK;
}

LONG opticalPower_CommandInstall()
{
	install_element ( CONFIG_NODE, &optical_power_threshold_onu_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_threshold_olt_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_temperature_onu_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_temperature_olt_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_voltage_onu_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_voltage_olt_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_current_onu_config_cmd);
	install_element ( CONFIG_NODE, &optical_power_current_olt_config_cmd);
	install_element ( CONFIG_NODE, &onu_laser_always_on_threshold_config_cmd);
	install_element ( CONFIG_NODE, &optical_mon_dead_zone_config_cmd);
	install_element ( CONFIG_NODE, &optical_monitor_enable_cmd);
	install_element ( CONFIG_NODE, &optical_monitor_interval_cmd);
	install_element ( CONFIG_NODE, &show_optical_power_threshold_onu_cmd);

	return VOS_OK;
}

LONG  optical_power_show_run( struct vty * vty )
{

long  tx_high_val,tx_low_val,rx_high_val,rx_low_val;
long  tx_high_olt_val,tx_low_olt_val,rx_high_olt_val,rx_low_olt_val;
long  high_val,low_val;
long  high_olt_val,low_olt_val;
long  high_vol_val,low_vol_val;
long  high_vol_olt_val,low_vol_olt_val;
long  high_cur_val,low_cur_val;
long  high_cur_olt_val,low_cur_olt_val;
long laser_on;
long  montior,montior_interval;
long  power,temperature,voltage,current;

vty_out( vty, "!optical power and optical monitior config\r\n" );
rx_low_val=getOpticalPowerThreshold(field_recv_oppower_low);
rx_high_val=getOpticalPowerThreshold(field_recv_oppower_high);
tx_high_val=getOpticalPowerThreshold(field_trans_oppower_high);
tx_low_val=getOpticalPowerThreshold(field_trans_oppower_low);

rx_low_olt_val=getOpticalPowerThreshold(field_olt_recv_oppower_low);
rx_high_olt_val=getOpticalPowerThreshold(field_olt_recv_oppower_high);
tx_low_olt_val=getOpticalPowerThreshold(field_olt_trans_oppower_low);
tx_high_olt_val=getOpticalPowerThreshold(field_olt_trans_oppower_high);

low_val=getOpticalPowerThreshold(field_pon_tempe_low);
high_val=getOpticalPowerThreshold(field_pon_tempe_high);

low_olt_val=getOpticalPowerThreshold(field_olt_tempe_low);
high_olt_val=getOpticalPowerThreshold(field_olt_tempe_high);

low_vol_val=getOpticalPowerThreshold(field_pon_vol_low);
high_vol_val=getOpticalPowerThreshold(field_pon_vol_high);

low_vol_olt_val=getOpticalPowerThreshold(field_olt_vol_low);
high_vol_olt_val=getOpticalPowerThreshold(field_olt_vol_high);

low_cur_val=getOpticalPowerThreshold(field_pon_cur_low);
high_cur_val=getOpticalPowerThreshold(field_pon_cur_high);

low_cur_olt_val=getOpticalPowerThreshold(field_olt_cur_low);
high_cur_olt_val=getOpticalPowerThreshold(field_olt_cur_high);

laser_on=getOpticalPowerThreshold(field_olt_laser_thresh);

montior=getOpticalPowerThreshold(field_olt_monitor_enable);
	
montior_interval=getOpticalPowerThreshold(field_olt_mon_interval);

power=getOpticalPowerDeadZone(field_power_dead_zone);
temperature=getOpticalPowerDeadZone(field_tempe_dead_zone);
voltage=getOpticalPowerDeadZone(field_vol_dead_zone);
current=getOpticalPowerDeadZone(field_cur_dead_zone);

vty_out( vty,"optical-power-threshold onu %d %d %d %d\r\n", tx_high_val,tx_low_val,rx_high_val,rx_low_val);
vty_out( vty,"optical-power-threshold olt %d %d %d %d\r\n", tx_high_olt_val,tx_low_olt_val,rx_high_olt_val,rx_low_olt_val);

vty_out( vty,"optical-temperature onu %d %d\r\n", high_val,low_val);
vty_out( vty,"optical-temperature olt %d %d\r\n", high_olt_val,low_olt_val);

vty_out( vty,"optical-voltage onu %d %d\r\n", high_vol_val,low_vol_val);
vty_out( vty,"optical-voltage olt %d %d\r\n", high_vol_olt_val,low_vol_olt_val);

vty_out( vty,"optical-current onu %d %d\r\n", high_cur_val,low_cur_val);
vty_out( vty,"optical-current olt %d %d\r\n", high_cur_olt_val,low_cur_olt_val);

vty_out( vty,"onu-laser-always-on-threshold %d\r\n", laser_on);

vty_out( vty,"optical-mon-dead-zone %d %d %d %d\r\n", power,temperature,voltage,current);

vty_out( vty,"optical-monitor-interval %d\r\n", montior_interval);
if(montior==1)
	vty_out( vty,"optical-monitor-enable enable \r\n");
else
	vty_out( vty,"optical-monitor-enable disable\r\n");
vty_out( vty, "!\r\n" );
return VOS_OK;
}

#endif
#ifdef __cplusplus
}
#endif
