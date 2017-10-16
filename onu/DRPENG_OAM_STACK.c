/***************************************************************
*
*      Module Name:  DRPENG_OAM_STACK.c
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
#include  "DRPENG_OAM_STACK.h"
/*#include "onuConfMgt.h"
#include "onuOamUpd.h"
#include "Onu_oam_comm.h"
#include "ct_manage/CT_Onu_Voip.h"
#include  "gwEponSys.h"*/

#if 1

#define SEND_COMPARSER_BUFFER						sent_oam_pdu_data + (EUQ_MAX_OAM_PDU - sent_length)
#define RECEIVED_LENGTH_BUFFER (unsigned short)(received_length - received_total_length)
#define RECEIVED_COMPARSER_BUFFER received_oam_pdu_data + received_total_length
#if 0
static void DataPrintf( uchar_t *pOctBuf, ulong_t bufLen )
{
    int i, j;
    char str[64];
    uchar_t cc;

    if( bufLen >= 2048 )
    {
        sys_console_printf("\r\n buffer is to long %d\r\n", bufLen );
        return;
    }

    j=0;
    for( i=0; i<bufLen; i++ )
    {
        cc = ((pOctBuf[i] >> 4) & 0xf);
        if( cc<=9 )
            str[j] = cc+'0';
        else if( (cc>=0x0a) && (cc<=0x0f) )
            str[j] = cc + 0x37;        /*'A' -- 'F'*/
        else
            str[j] = '-';
        j++;
        cc = (pOctBuf[i] & 0xf );
        if( cc<=9 )
            str[j] = cc+'0';
        else if( (cc>=0x0a) && (cc<=0x0f) )
            str[j] = cc + 0x37;
        else
            str[j] = '-';
        j++;
        str[j++] = ' ';

        if( j >= 60 )
        {
            str[j] = 0;
            sys_console_printf( "%s\r\n", str );
            j = 0;
        }
    }
    if( (j != 0) && (j <= 60) )
    {
        str[j] = 0;
        sys_console_printf( "%s\r\n", str );
    }
}
#endif

#if 1
short int DRPENG_COMPOSE_OUI
		  (  unsigned char    OUI[],
		      unsigned char		  *send_buffer,
		      unsigned short		  *send_buf_length)
{
    if(NULL == send_buffer || NULL == OUI)
		return OLT_ERR_PARAM;
	
    memcpy(send_buffer,OUI,OAM_OUI_SIZE);
    (*send_buf_length) = OAM_OUI_SIZE;

    return OLT_ERR_OK;
}
short int DRPENG_COMPOSE_opcode
		  ( const unsigned char    opcode,
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
    send_buffer[0] = opcode;
    (*send_buf_length) = EXTEND_OPCODE_LEN;

    return OLT_ERR_OK;
}
short int DRPENG_COMPOSE_BRANCH_LEAF
		  ( const unsigned char    expected_branch,
		     const unsigned short    expected_leaf,
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
    send_buffer[0] = expected_branch;
    send_buffer[1] = (unsigned char)((expected_leaf>>8)&0xff);
    send_buffer[2] = (unsigned char)(expected_leaf & 0xff);
    (*send_buf_length) = EXTEND_BRANCH_LEAF_LEN;

    return OLT_ERR_OK;
}
short int DRPENG_COMPOSE_PORT_TLV
		  ( CTC_management_object_index_t  * management_objects,
			 unsigned char		  *send_buffer,
			 unsigned short 	  *send_buf_length)
{
	short int sent_length = 0,comparser_result,port_id;
	unsigned char Width = 0x04;
	if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( CTC_2_1_MANAGEMENT_OBJECT_BRANCH, management_objects->port_type,send_buffer, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	send_buffer += sent_length;

	port_id = management_objects->port_number;
	send_buffer[0] = Width;
	send_buffer[1] = (unsigned char)(management_objects->port_type & 0xff);
	send_buffer[4] = (unsigned char)(port_id & 0xff);
	(*send_buf_length) = PORT_TLV_LEN;

	return OLT_ERR_OK;
}

short int DRPENG_COMPOSE_UCHAR
		  ( unsigned char type, 
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    unsigned char Width = 0x01;
    if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
    send_buffer[0] =   Width;
    send_buffer[1] =   type;
    (*send_buf_length) = Width + VARIABLE_WIDTH_LEN;

    return OLT_ERR_OK;
}
short int DRPENG_COMPOSE_DEVICE_NAME
		  ( unsigned char *device_name,
		  	unsigned char device_name_len,
			 unsigned char		  *send_buffer,
			 unsigned short 	  *send_buf_length)
{
	unsigned char Width = device_name_len;
	if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	if(device_name_len > 255)
		return OLT_ERR_PARAM;
	send_buffer[0] =   Width;
	memcpy(&send_buffer[1] ,device_name,device_name_len);
	(*send_buf_length) = Width + VARIABLE_WIDTH_LEN;

	return OLT_ERR_OK;
}

short int DRPENG_COMPOSE_PORT_LOCATION_BY_MAC
		  ( mac_address    mac,
		     const short int vlan_id,
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    unsigned char Width = 0x08;
    if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
    send_buffer[0] =   Width;
    memcpy(&send_buffer[1],mac,sizeof(mac_address));
    send_buffer[7] = (unsigned char)((vlan_id>>8)&0xff);
    send_buffer[8] = (unsigned char)(vlan_id & 0xff);
    (*send_buf_length) = Width+ VARIABLE_WIDTH_LEN;

    return OLT_ERR_OK;
}

short int DRPENG_COMPOSE_PORT_STORM
		  ( OnuPortStorm_S * status,
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    unsigned char Width = 0x05;
    unsigned int storm_rate = 0;
	
    if(NULL == send_buffer || NULL == status)
		return OLT_ERR_PARAM;
	
    storm_rate = status->storm_rate;
    send_buffer[0] =   Width;
    send_buffer[1] =   status->storm_type;
    send_buffer[2] =   status->storm_mode;
    send_buffer[3]     = (unsigned char)((storm_rate >>(OAM_BYTES_IN_WORD*OAM_BITS_IN_BYTE))& 0xff);
    send_buffer[4]   = (unsigned char)((storm_rate >>OAM_BITS_IN_BYTE)& 0xff);
    send_buffer[5]   = (unsigned char)(storm_rate & 0xff);
    (*send_buf_length) = Width + VARIABLE_WIDTH_LEN;

    return OLT_ERR_OK;
}

short int DRPENG_COMPOSE_LOOP_DETECTION_TIME
		  ( unsigned short port_downtime,
	            unsigned short restart_port_times,
		     unsigned char		  *send_buffer,
		     unsigned short		  *send_buf_length)
{
    unsigned char Width = 0x04;
    if(NULL == send_buffer)
		return OLT_ERR_PARAM;
	
    send_buffer[0] =   Width;
    send_buffer[1] =   (unsigned char)((port_downtime >>OAM_BITS_IN_BYTE)& 0xff);
    send_buffer[2] =   (unsigned char)(port_downtime & 0xff);
    send_buffer[3]     = (unsigned char)((restart_port_times >>OAM_BITS_IN_BYTE)& 0xff);
    send_buffer[4]   = (unsigned char)(restart_port_times & 0xff);
    (*send_buf_length) = Width + VARIABLE_WIDTH_LEN;

    return OLT_ERR_OK;
}

short int DRPENG_PARSE_branch_leaf
		  (const unsigned char		  *recevie_buffer, 
		    unsigned short                 recv_buffer_size,
		    const unsigned char    expected_branch,
		    const unsigned short    expected_leaf,
		     unsigned short		  *left_length)
{
    unsigned short recevie_expected_leaf = 0;
    unsigned char recevie_expected_branch = 0;
    unsigned short * pbuffer = NULL;
    if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
    CHECK_BUFFER_SIZE(EXTEND_BRANCH_LEAF_LEN,recv_buffer_size)
    if(recevie_buffer[0] == 0x37)
    {
    	CHECK_BUFFER_SIZE(EXTEND_BRANCH_LEAF_LEN+PORT_TLV_LEN,recv_buffer_size)
    	recevie_expected_branch = recevie_buffer[PORT_TLV_LEN];
	pbuffer = &recevie_buffer[PORT_TLV_LEN+1];
	recevie_expected_leaf = *pbuffer;
	*left_length = PORT_TLV_LEN + EXTEND_BRANCH_LEAF_LEN;
    }
    else
    {
    	recevie_expected_branch = recevie_buffer[0];
	pbuffer = &recevie_buffer[1];
	recevie_expected_leaf = *pbuffer;
	*left_length =  EXTEND_BRANCH_LEAF_LEN;
    }
    if(recevie_expected_branch != expected_branch || recevie_expected_leaf != expected_leaf)
    {
    	 return OLT_ERR_PARAM;
    }
  	 
    return OLT_ERR_OK;
}
short int DRPENG_PARSE_get_result
		  (const unsigned char		  *recevie_buffer, 
		    unsigned short                 recv_buffer_size,
		     unsigned short		  *left_length)
{
    if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
    CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
    if(recevie_buffer[0] == 0x80)
    	 return OLT_ERR_OK;
    else
	return 	OLT_ERR_NOTSUPPORT;
}
short int DRPENG_PARSE_port_mode
		  (const unsigned char		  *recevie_buffer, 
		    unsigned short                 recv_buffer_size,
		     unsigned char		  *mode)
{
    if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
    CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
    if(recevie_buffer[0] != 0x01)
		return OLT_ERR_PARAM;
    CHECK_BUFFER_SIZE(recevie_buffer[0],recv_buffer_size-VARIABLE_WIDTH_LEN)
    *mode = recevie_buffer[1];
	return OLT_ERR_OK;
}

short int DRPENG_PARSE_mac_address_number
		  (const unsigned char		  *recevie_buffer, 
			unsigned short				   recv_buffer_size,
			 unsigned short		  *number)
{
	unsigned short *pbuffer =NULL;
	if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
	CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
	if(recevie_buffer[0] != 0x02)
		return OLT_ERR_PARAM;
	CHECK_BUFFER_SIZE(recevie_buffer[0],recv_buffer_size-VARIABLE_WIDTH_LEN)
	pbuffer = &recevie_buffer[1];
	*number = *pbuffer;
	return OLT_ERR_OK;
}

short int DRPENG_PARSE_OnuPortLocationByMAC
		  (const unsigned char		  *recevie_buffer, 
			unsigned short				   recv_buffer_size,
			 unsigned char * result_get,
			OnuPortLacationEntry_S *port_location_infor)
{
	unsigned char recevie_result = 0;
	unsigned short *pbuffer =NULL;
	if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
	CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
	if(recevie_buffer[0] != 0x0b)
		return OLT_ERR_PARAM;
	CHECK_BUFFER_SIZE(recevie_buffer[0],recv_buffer_size-VARIABLE_WIDTH_LEN)
	recevie_result = recevie_buffer[1];
	* result_get = recevie_result;
	if(recevie_result == NOTFOUND)
		return OLT_ERR_OK;
	memcpy(port_location_infor->mac,&recevie_buffer[2],ONU_MAC_LEN);
	pbuffer = &recevie_buffer[8];
	port_location_infor->vlan_id = *pbuffer;
	port_location_infor->port_id = recevie_buffer[10];
	port_location_infor->mac_type = recevie_buffer[11];
	return OLT_ERR_OK;
}

short int DRPENG_PARSE_loopDetectionTime
		  (const unsigned char		  *recevie_buffer, 
		    unsigned short                 recv_buffer_size,
		     unsigned short  *port_downtime,
		    unsigned short  *restart_port_times)
{
   unsigned char Width = 0;
   unsigned short *pbuffer =NULL;
    if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
   CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
   Width = recevie_buffer[0];
   if(Width != 0x04)
		return OLT_ERR_PARAM;
   CHECK_BUFFER_SIZE(Width,recv_buffer_size-VARIABLE_WIDTH_LEN)
    pbuffer = &recevie_buffer[1];
   *port_downtime = pbuffer[0];
   *restart_port_times = pbuffer[1];

   return OLT_ERR_OK;
}

short int DRPENG_PARSE_storm_status
		  (const unsigned char		  *recevie_buffer, 
			unsigned short				   recv_buffer_size,
			 OnuPortStorm_S  *status)
{
   unsigned char Width = 0;
   unsigned int bit1,bit2,bit3;
	if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
   CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
   Width = recevie_buffer[0];
   if(Width != 0x05)
		return OLT_ERR_PARAM;
   CHECK_BUFFER_SIZE(Width,recv_buffer_size-VARIABLE_WIDTH_LEN)
   status->storm_type = recevie_buffer[1];
   status->storm_mode = recevie_buffer[2];
   bit1 = recevie_buffer[3];
   bit2 = recevie_buffer[4];
   bit3 = recevie_buffer[5];
   status->storm_rate =  (bit1<<16)|(bit2<<8)|(bit3);
   return OLT_ERR_OK;
}

short int DRPENG_PARSE_DEVICE_STRING
		  (const unsigned char		  *recevie_buffer, 
			unsigned short				   recv_buffer_size,
			 unsigned char  *device,
			unsigned char  *device_len)
{
   unsigned char Width = 0;
	if(NULL == recevie_buffer|| NULL == device)
		return OLT_ERR_PARAM;
	
   CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
   Width = recevie_buffer[0];
   *device_len = Width;
   CHECK_BUFFER_SIZE(Width,recv_buffer_size-VARIABLE_WIDTH_LEN)
   memcpy(device,&recevie_buffer[1],Width);
   return OLT_ERR_OK;
}

short int DRPENG_PARSE_SupportExtendAttribute
		  ( const unsigned char		  *recevie_buffer, 
		      unsigned short                 recv_buffer_size,
			unsigned char    SupportAttribute[],
			char *SupportAttribute_len)
{
	unsigned char Width,result;
	if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;
	
	CHECK_BUFFER_SIZE(VARIABLE_WIDTH_LEN,recv_buffer_size)
	Width = recevie_buffer[0];
	if(Width > 0x11)
		return OLT_ERR_PARAM;
	CHECK_BUFFER_SIZE(Width,recv_buffer_size-VARIABLE_WIDTH_LEN)
	result = recevie_buffer[1];
	if(result == GET_SUCCESS)
	{
		memcpy(SupportAttribute,&recevie_buffer[2],Width-1);
		*SupportAttribute_len = Width-1;
		return OLT_ERR_OK;
	}
	else
	{
		*SupportAttribute_len = 0;
		return OLT_ERR_PARAM;
	}
	 
}

short int DRPENG_PARSE_MacAddressTable
		  (  unsigned char 	  *recevie_buffer, 
			  unsigned short				 recv_buffer_size,
			OnuPortLacationInfor_S* table)
{
	unsigned short number_entry_tlv = 0,received_left_length = 0;
	short int result;
	unsigned char Width = 0;
	unsigned int vlan_id_high = 0,vlan_id_low = 0;
	unsigned int total_number_entry = 0;
	unsigned int * pbuffer_int = NULL;
	unsigned short *pbuffer_short = NULL;
	unsigned char * pbuffer_char = NULL;
	unsigned short i = 0,j = 0,k=0;
	if(NULL == recevie_buffer)
		return OLT_ERR_PARAM;

	CHECK_BUFFER_SIZE(EXTEND_BRANCH_LEAF_LEN+VARIABLE_WIDTH_LEN,recv_buffer_size)
	Width = recevie_buffer[EXTEND_BRANCH_LEAF_LEN];
	number_entry_tlv = (Width-6)/10;
	CHECK_BUFFER_SIZE(Width,recv_buffer_size-EXTEND_BRANCH_LEAF_LEN-VARIABLE_WIDTH_LEN)

	pbuffer_short = &recevie_buffer[EXTEND_BRANCH_LEAF_LEN+1];
	total_number_entry = *pbuffer_short;
	if(TOTAL_NUMBER_MAC_ADDRESS_ENTRY < total_number_entry)
	{
		return OLT_ERR_PARAM;
	}
	table->number_entry = total_number_entry;

	for(i = 0;i < (total_number_entry/TLV_NUMBER_MAC_ADDRESS_ENTRY+1); i ++)
	{
		if(recevie_buffer[i*ONE_TLV_MAX_LENGTH] == 0xc7 && recevie_buffer[i*ONE_TLV_MAX_LENGTH+1] == 0xc0 && recevie_buffer[i*ONE_TLV_MAX_LENGTH+2] == 0x03)
		{
			pbuffer_short = &recevie_buffer[i*ONE_TLV_MAX_LENGTH + 6];
			number_entry_tlv = *pbuffer_short;
			pbuffer_char = &recevie_buffer[i*ONE_TLV_MAX_LENGTH+8];
			if(TLV_NUMBER_MAC_ADDRESS_ENTRY < number_entry_tlv)
			{
				return OLT_ERR_PARAM;
			}
			else if(number_entry_tlv == TLV_NUMBER_MAC_ADDRESS_ENTRY)
			{
				if(ONE_TLV_MAX_LENGTH > recv_buffer_size)
					return OLT_ERR_OK;
				for(j = 0; j <number_entry_tlv; j+=1,k+=1)
				{
					if(k == TOTAL_NUMBER_MAC_ADDRESS_ENTRY)
						return OLT_ERR_OK;
					memcpy(table->OnuPortMacTable[k].mac,&pbuffer_char[j*10],ONU_MAC_LEN);
					vlan_id_high = pbuffer_char[j*10+ONU_MAC_LEN];
					vlan_id_low = pbuffer_char[j*10+ONU_MAC_LEN+1];
					table->OnuPortMacTable[k].vlan_id = (vlan_id_high<<8)|vlan_id_low;
					table->OnuPortMacTable[k].port_id = pbuffer_char[j*10+ONU_MAC_LEN+2];
					table->OnuPortMacTable[k].mac_type= pbuffer_char[j*10+ONU_MAC_LEN+3];
				}
				recv_buffer_size -= ONE_TLV_MAX_LENGTH;
			}
			else if(number_entry_tlv < TLV_NUMBER_MAC_ADDRESS_ENTRY/*last one*/)
			{
				if(recv_buffer_size < (number_entry_tlv*10+10))
					return OLT_ERR_OK;
				for(j = 0; j <number_entry_tlv; j+=1,k+=1)
				{
					if(k == TOTAL_NUMBER_MAC_ADDRESS_ENTRY)
						return OLT_ERR_OK;
					memcpy(table->OnuPortMacTable[k].mac,&pbuffer_char[j*10],ONU_MAC_LEN);
					vlan_id_high = pbuffer_char[j*10+ONU_MAC_LEN];
					vlan_id_low = pbuffer_char[j*10+ONU_MAC_LEN+1];
					table->OnuPortMacTable[k].vlan_id = (vlan_id_high<<8)|vlan_id_low;
					table->OnuPortMacTable[k].port_id = pbuffer_char[j*10+ONU_MAC_LEN+2];
					table->OnuPortMacTable[k].mac_type= pbuffer_char[j*10+ONU_MAC_LEN+3];
				}
				break;/*直接跳出循环*/
			}
		}
		else
			break;
	}
	 return OLT_ERR_OK;
}


#endif
/*----------------------------Dr.Peng 扩展OAM接口--------------------------*/
int DRPENG_STACK_SetOnuPortSaveConfiguration(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id,
	const unsigned char action )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_SAVE_CONFIG;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_UCHAR
							( action,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuPortSaveConfiguration: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuPortSaveConfiguration(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuLoopDetectionTime(
	const short int olt_id,
	const short int onu_id, 
	const unsigned short port_downtime,
	const unsigned short restart_port_times)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_LOOP_DETECTION_TIME;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_LOOP_DETECTION_TIME
							( port_downtime,restart_port_times,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuLoopDetectionTime: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuLoopDetectionTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuLoopDetectionTime(
	const short int olt_id,
	const short int onu_id, 
	unsigned short  *port_downtime,
	unsigned short  *restart_port_times)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_LOOP_DETECTION_TIME;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuLoopDetectionTime: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/

		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_loopDetectionTime
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,port_downtime,restart_port_times);
		CHECK_COMPARSER_RESULT(result)
		
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuLoopDetectionTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuPortMode(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id,
	const unsigned char mode )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_MODE;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_UCHAR
							( mode,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuPortMode: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuPortMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuPortMode(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id,
	unsigned char  *mode )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_MODE;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuPortMode: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/

		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_port_mode
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,mode);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuPortMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuPortStormStatus(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id,
	OnuPortStorm_S * status )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_STORM;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_STORM
							( status,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuPortStormStatus: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuPortStormStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuPortStormStatus(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id,
	OnuPortStorm_S  *status )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_STORM;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuPortStormStatus: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_storm_status
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,status);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuPortStormStatus(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuPortIsolation(
	const short int olt_id,
	const short int onu_id, 
	const unsigned char status )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_ISOLATION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_UCHAR
							( status,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		/*DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuPortIsolation: length = %d \r\n", received_length);
		DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuPortIsolation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuPortIsolation(
	const short int olt_id,
	const short int onu_id, 
	unsigned char  *status )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_ISOLATION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		/*DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuPortIsolation: length = %d \r\n", received_length);
		DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_port_mode
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,status);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuPortIsolation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuDeviceLocation(
	const short int olt_id,
	const short int onu_id, 
	unsigned char *device_location,
	unsigned char  device_location_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_LOCATION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_DEVICE_NAME
							( device_location,device_location_len,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuDeviceLocation: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuDeviceLocation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuDeviceLocation(
	const short int olt_id,
	const short int onu_id, 
	unsigned char  *device_location,
	unsigned char  *device_location_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_LOCATION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuDeviceLocation: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_DEVICE_STRING
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,device_location,device_location_len );
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuDeviceLocation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuDeviceDescription(
	const short int olt_id,
	const short int onu_id, 
	unsigned char *name,
	unsigned char name_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_DESCRIPTION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_DEVICE_NAME
							( name,name_len,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuDeviceDescription: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuDeviceDescription(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuDeviceDescription(
	const short int olt_id,
	const short int onu_id, 
	unsigned char* device_description,
	unsigned char  *device_description_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_DESCRIPTION;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuDeviceDescription: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_DEVICE_STRING
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,device_description,device_description_len );
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuDeviceDescription(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_SetOnuDeviceName(
	const short int olt_id,
	const short int onu_id, 
	unsigned char *device_name ,
	unsigned char device_name_len )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_SET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_NAME;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_DEVICE_NAME
							( device_name,device_name_len,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_SetOnuDeviceName: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_get_result
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_SetOnuDeviceName(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}
int DRPENG_STACK_GetOnuDeviceName(
	const short int olt_id,
	const short int onu_id, 
	unsigned char* device_name, 
	unsigned char *device_name_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_DEVICE_NAME;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuDeviceName: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_DEVICE_STRING
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,device_name,device_name_len );
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuDeviceName(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_GetOnuMacAddressTable(
	const short int olt_id,
	const short int onu_id, 
	const unsigned char mac_type,
	OnuPortLacationInfor_S* table )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_MAC_TABLE;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_UCHAR
							( mac_type,SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuMacAddressTable: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
									( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		/*received_total_length += received_left_length;此处不偏移*/

		result = (short int)DRPENG_PARSE_MacAddressTable
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,table);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuMacAddressTable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_GetOnuPortMacAddressNumber(
	const short int olt_id,
	const short int onu_id, 
	const short int port_id, 
	unsigned short  *mac_address_number )
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_MAC_NUMBER;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	CTC_management_object_index_t   management_objects;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	
	management_objects.port_number  = port_id;
	management_objects.slot_number	   = 0x00;
	management_objects.frame_number	   = 0x00;
	management_objects.port_type 	   = CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;
	
	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_PORT_TLV
							( &management_objects, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuPortMacAddressNumber: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;
		result = (short int)DRPENG_PARSE_mac_address_number
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,mac_address_number);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuPortMacAddressNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_GetOnuPortLocationByMAC(
	const short int olt_id,
	const short int onu_id, 
	mac_address mac,
	const short int vlan_id,
	unsigned char * result_get,
	OnuPortLacationEntry_S *port_location_infor)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,sent_length = EUQ_MAX_OAM_PDU,received_left_length = 0;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_PORT_LOCATION_BY_MAC;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	if(NULL == port_location_infor)
	      return VOS_ERROR;
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_PORT_LOCATION_BY_MAC
							( mac, vlan_id, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENG_STACK_GetOnuPortLocationByMAC: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_OnuPortLocationByMAC
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,result_get,port_location_infor);
		CHECK_COMPARSER_RESULT(result)
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENG_STACK_GetOnuPortLocationByMAC(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}

int DRPENG_STACK_GetOnuSupportExtendAttribute(
	const short int olt_id,
	const short int onu_id, 
	char SupportAttribute[],
	char *SupportAttribute_len)
{
	int                             result = OLT_ERR_UNKNOEWN;
	unsigned short     received_length = 0,received_left_length = 0,sent_length = EUQ_MAX_OAM_PDU;
	short int                  comparser_result;
	unsigned char       OUI[OAM_OUI_SIZE] = {0x0c,0x7c,0x7d};/*鹏博士OAM拓展OUI*/
	unsigned char       sent_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char       received_oam_pdu_data[EUQ_MAX_OAM_PDU];
	unsigned char	       expected_opcode =  DRPENGONU_OPCODE_OAM_EXT_GET_REQUEST;
	unsigned char	       expected_branch =  OAM_VENDOR_EXT_EX_VAR_BRANCH_ATTR;
	unsigned short     expected_leaf	=  DRPENG_EX_VAR_ONU_SUPPORT_ATTRIBUTE;
	unsigned short	max_length=EUQ_MAX_OAM_PDU,total_length=0, received_total_length=OAM_BYTES_EXTEND_OPCODE;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	if(NULL == SupportAttribute)
	      return VOS_ERROR;
	
	VOS_MemZero(sent_oam_pdu_data, EUQ_MAX_OAM_PDU);
	VOS_MemZero(received_oam_pdu_data, EUQ_MAX_OAM_PDU);

	comparser_result = (short int)DRPENG_COMPOSE_OUI
							( OUI, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;
	
	comparser_result = (short int)DRPENG_COMPOSE_opcode
							( expected_opcode, SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	sent_length = max_length - total_length;

	comparser_result = (short int)DRPENG_COMPOSE_BRANCH_LEAF
							( expected_branch, expected_leaf,
							  SEND_COMPARSER_BUFFER, &sent_length );
	CHECK_COMPARSER_RESULT(comparser_result)
	total_length += sent_length;
	total_length += VARIABLE_WIDTH_LEN;/*Variable Width占一个字节*/
	
	result = DrPeng_Oam_Session_Send(olt_id, onu_id, 0, OAM_SYNC, expected_branch, expected_leaf, NULL, sent_oam_pdu_data, total_length, received_oam_pdu_data, &received_length);
	if(result == VOS_OK)
	{
		DEBUG_OAM_SESSION_PRINTF("DRPENGONU_GetOnuSupportExtendAttributeInformation: length = %d \r\n", received_length);
		/*DataPrintf(received_oam_pdu_data,received_length);*/
		
		result = (short int)DRPENG_PARSE_branch_leaf
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,expected_branch,expected_leaf ,&received_left_length);
		CHECK_COMPARSER_RESULT(result)
		received_total_length += received_left_length;

		result = (short int)DRPENG_PARSE_SupportExtendAttribute
										( RECEIVED_COMPARSER_BUFFER ,RECEIVED_LENGTH_BUFFER,SupportAttribute,SupportAttribute_len );
		CHECK_COMPARSER_RESULT(result)	
	}	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"DRPENGONU_GetOnuSupportExtendAttributeInformation(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, result, SYS_LOCAL_MODULE_SLOTNO);

	return result;	
}


#endif

#ifdef __cplusplus

}

#endif


