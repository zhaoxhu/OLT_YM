/***************************************************************
*
*      Module Name:  DrPeng_manage_api.c
*
*                       (c) COPYRIGHT  by 
*                        GWD Com. Ltd.
*                        All rights reserved.
*
*     This software is confidential and proprietary to gwtt Com, Ltd. 
*     No part of this software may be reproduced,
*     stored, transmitted, disclosed or used in any form or by any means
*     other than as expressly provided by the written Software function 
*     Agreement between gwtt and its licensee
*
*   Date:    2017/03/23
*   Author:  yangzl
*   content:
**  History:
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "Onu_oam_comm.h"
#include  "DrPeng_manage_api.h"

#if 1

int DRPENG_SetOnuPortSaveConfiguration(short int olt_id, short int onu_id, short int port_id,unsigned char action )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt = DRPENG_STACK_SetOnuPortSaveConfiguration( olt_id, onu_id, port_id, action);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuPortSaveConfiguration(%d, %d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,port_id, action, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuLoopDetectionTime(olt_id,onu_id, port_downtime,restart_port_times);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuLoopDetectionTime(%d, %d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,port_downtime, restart_port_times, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuLoopDetectionTime(olt_id,onu_id, port_downtime,restart_port_times);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuLoopDetectionTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuPortMode(olt_id,onu_id, port_id,mode);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuPortMode(%d, %d,%d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id,mode,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char* mode )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuPortMode(olt_id,onu_id, port_id,mode);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuPortMode(%d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuPortStormStatus(olt_id,onu_id, port_id,status);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuPortStormStatus(%d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuPortStormStatus(olt_id,onu_id, port_id,status);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuPortStormStatus(%d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuPortIsolation(short int olt_id, short int onu_id, unsigned char status )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuPortIsolation(olt_id,onu_id,status);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuPortIsolation(%d, %d,%d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, status,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuPortIsolation(short int olt_id, short int onu_id, unsigned char* status )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuPortIsolation(olt_id,onu_id,status);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuPortIsolation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuDeviceLocation(olt_id,onu_id,device_location,device_location_len);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuDeviceLocation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location )
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned char device_location_len = 0;
   
    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuDeviceLocation(olt_id,onu_id,device_location,&device_location_len);

    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuDeviceLocation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* name,unsigned char  name_len )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuDeviceDescription(olt_id,onu_id,name,name_len);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuDeviceDescription(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* device_description )
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned char device_description_len = 0;
    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuDeviceDescription(olt_id,onu_id,device_description,&device_description_len);

    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuDeviceDescription(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_SetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name,unsigned char  device_name_len )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_SetOnuDeviceName(olt_id,onu_id,device_name,device_name_len);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_SetOnuDeviceName(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int DRPENG_GetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_name )
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned char device_name_len = 0;
    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuDeviceName(olt_id,onu_id,device_name,&device_name_len);

    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuDeviceName(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuMacAddressTable(short int olt_id, short int onu_id,  unsigned char mac_type,OnuPortLacationInfor_S* table )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuMacAddressTable(olt_id,onu_id,mac_type,table);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuMacAddressTable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuPortMacAddressNumber(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number )
{
    int iRlt = OLT_ERR_NOTEXIST;

    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuPortMacAddressNumber(olt_id,onu_id,port_id,mac_address_number);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuPortMacAddressNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuPortLocationByMAC(short int olt_id, short int onu_id, mac_address mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor )
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned char  result_get;
    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuPortLocationByMAC(olt_id,onu_id,mac,vlan_id,&result_get,port_location_infor);
    if(0 == iRlt)
    		port_location_infor->result_get = result_get;
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuPortLocationByMAC(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int DRPENG_GetOnuSupportExtendAttribute(short int olt_id, short int onu_id, char *SupportAttribute)
{
    int iRlt = OLT_ERR_NOTEXIST;
   char SupportAttribute_len = 0;
    OLT_LOCAL_ASSERT(olt_id);
    
     iRlt =  DRPENG_STACK_GetOnuSupportExtendAttribute(olt_id,onu_id,SupportAttribute,&SupportAttribute_len);
	 
    OLT_PAS_DEBUG(OLT_TK_TITLE"DRPENG_GetOnuSupportExtendAttribute(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id,iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

#ifdef __cplusplus

}

#endif


