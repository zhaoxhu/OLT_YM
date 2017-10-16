#ifndef _DRPENG_MANAGE_API_H
#define _DRPENG_MANAGE_API_H

typedef unsigned char mac_address [6];
int DRPENG_SetOnuPortSaveConfiguration(short int olt_id, short int onu_id, short int port_id,unsigned char action );
int DRPENG_SetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times );
int DRPENG_GetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times );
int DRPENG_SetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode );
int DRPENG_GetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char* mode );
int DRPENG_SetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status );
int DRPENG_GetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status );
int DRPENG_SetOnuPortIsolation(short int olt_id, short int onu_id, unsigned char status );
int DRPENG_GetOnuPortIsolation(short int olt_id, short int onu_id, unsigned char* status );
int DRPENG_SetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len );
int DRPENG_GetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location );
int DRPENG_SetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name,unsigned char  name_len );
int DRPENG_GetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* device_description);
int DRPENG_SetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name,unsigned char  device_name_len );
int DRPENG_GetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name );
int DRPENG_GetOnuMacAddressTable(short int olt_id, short int onu_id,  unsigned char mac_type,OnuPortLacationInfor_S* table );
int DRPENG_GetOnuPortMacAddressNumber(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number );
int DRPENG_GetOnuPortLocationByMAC(short int olt_id, short int onu_id, mac_address mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor );
int DRPENG_GetOnuSupportExtendAttribute(short int olt_id, short int onu_id, char *SupportAttribute);


#endif
