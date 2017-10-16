/***************************************************************
*
*						Module Name:  PonStdOam.c
*
*                       (c) COPYRIGHT  by 
*                        GWTT Com. Ltd.
*                        All rights reserved.
*
*     This software is confidential and proprietary to gwtt Com, Ltd. 
*     No part of this software may be reproduced,
*     stored, transmitted, disclosed or used in any form or by any means
*     other than as expressly provided by the written Software function 
*     Agreement between gwtt and its licensee
*
*   Date: 			2014/07/23
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  14/07/25  |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#define __MODULE__ "OAM_STD_PON" /* File name for log headers */

#define ___FILE___ "" /* File name for log headers */


#include "oam/efm/efm.h"
#include "PonStdOam.h"
#include "PonOam.h"

#include "PonGeneral.h"

/*============================== Data Types =================================*/                             
/*========================== External Variables =============================*/

/*========================== External Functions =============================*/
/*============================== Variables ==================================*/
/*=========================== Internal variables =============================*/
/*============================== Constants ==================================*/
/*================================ Macros ===================================*/

/*======================== Internal Functions prototype =====================*/
/*================================ Functions ================================*/

#if 1
/**************************标准OAM接口****************************/

int PonStdOamDiscovery(short int olt_id, short int llid, mac_address_t link_mac)
{
    ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);

            return efm_oam_set_enable(if_index, EFM_OAM_ENABLE);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

int PonStdOamDisconect(short int olt_id, short int llid, mac_address_t link_mac)
{
    ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);

            return efm_oam_set_enable(if_index, EFM_OAM_DISABLE);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}


static void PonStdOamDiscoverNotify( EFM_OAM_regData_s regData )
{
    ULONG if_index;
 	UINT16 slotNo;
	UINT16 portNo;
	UINT16 subif;
    short int olt_id;
    short int llid;

    if_index = regData.if_index;
	slotNo = PortIndex_GetSlotNo( if_index );
	portNo = PortIndex_GetPortNo( if_index );
    subif  = PortIndex_GetSubIfNo( if_index );

    llid = (short int)subif;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slotNo, (short int)portNo );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
        	if ( OLTAdv_IsExist( olt_id ) )
        	{
            	if( regData.code == EFM_OAM_PEER_REG )
            	{
            	    PON_authentication_sequence_t  onu_auth_seqs = {0}; 

                    Onu_registration_handler(olt_id, llid, regData.mac, onu_auth_seqs, OAM_STANDARD_VERSION_3_3);
            	}
            	else if( regData.code == EFM_OAM_PEER_DEREG )
            	{
            	    if ( OLTAdv_LLIDIsOnline( olt_id, llid ) )
                    {
                	    Onu_deregistration_handler(olt_id, llid, PON_ONU_DEREGISTRATION_OAM_LINK_DISCONNECTION);

                        (void)OLT_DeregisterLLID(olt_id, llid, FALSE);
                    }
            	}
            	else
            	{
            		VOS_ASSERT(0);
            	}
        	}
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return;
}


static int PonStdOamFault2Alarm(short int olt_id, short int llid, EFM_OAM_faultNotify_s *faultData
                    , short int   *alarm_source_id
                    , PON_alarm_t *alarm_type
                    , short int   *alarm_parameter
                    , void        **alarm_data)
{
    int result = 0;

    *alarm_source_id = llid;
    *alarm_parameter = 0;
    *alarm_data = NULL;

    switch ( faultData->code )
    {
        case EFM_OAM_DYING_GASP:
            *alarm_type = PON_ALARM_DYING_GASP;
        break;    
        case EFM_OAM_DYING_GASPRESUME:
            result = VOS_ERROR;
        break;    

        case EFM_OAM_CRITICAL:
            *alarm_type = PON_ALARM_CRITICAL_EVENT;
        break;    
        case EFM_OAM_CRITICALRESUME:
            result = VOS_ERROR;
        break;    

        case EFM_OAM_LINK_FAULT:
            *alarm_type = PON_ALARM_LOCAL_LINK_FAULT;
        break;    
        case EFM_OAM_LINK_FAULTRESUME:
            result = VOS_ERROR;
        break;    

        case EFM_OAM_Errored_Symbol:
            *alarm_type = PON_ALARM_ERRORED_SYMBOL_PERIOD;
        break;    
        case EFM_OAM_Errored_Frame:
            *alarm_type = PON_ALARM_ERRORED_FRAME;
        break;    
        case EFM_OAM_Errored_Period:
            *alarm_type = PON_ALARM_ERRORED_FRAME_PERIOD;
        break;    
        case EFM_OAM_Errored_Seconds:
            *alarm_type = PON_ALARM_ERRORED_FRAME_SECONDS_SUMMARY;
        break;    

        default:
            result = VOS_ERROR;
    }

    return result;
}

static void PonStdOamFaultNotify( EFM_OAM_faultNotify_s faultData )
{
    ULONG if_index;
 	UINT16 slotNo;
	UINT16 portNo;
	UINT16 subif;
    short int olt_id;
    short int llid;

    if_index = faultData.if_index;
	slotNo = PortIndex_GetSlotNo( if_index );
	portNo = PortIndex_GetPortNo( if_index );
    subif  = PortIndex_GetSubIfNo( if_index );

    llid = (short int)subif;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slotNo, (short int)portNo );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
        	if ( OLTAdv_LLIDIsExist( olt_id, llid ) )
        	{
                PON_alarm_t alarm_type;
                short int	alarm_source_id, alarm_parameter;
                void *alarm_data;

                if ( 0 == PonStdOamFault2Alarm(olt_id, llid, &faultData, &alarm_source_id, &alarm_type, &alarm_parameter, &alarm_data) )
                {
        			Alarm_handler( olt_id, alarm_source_id, alarm_type, alarm_parameter, alarm_data );
                }
        	}
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return;
}


int PonStdOamPktRecvHandler(short int olt_id, short int llid, char *oam_pkt, int pkt_len)
{
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            return efm_oam_pktPortRecv(slot, port, subif, oam_pkt, pkt_len);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

int PonStdOamPktSendHandler(unsigned long slot_id, unsigned long port_id, unsigned long sub_if, char *oam_pkt, int pkt_len)
{
    short int olt_id;
    short int llid;

    llid = (short int)sub_if;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slot_id, (short int)port_id );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
            return OLT_SendFrame2CNI(olt_id, (llid == PONBROADCASTLLID) ? -1:llid, oam_pkt, pkt_len);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return VOS_ERROR;
}

int PonStdOamPortIsUp(unsigned long slot_id, unsigned long port_id, unsigned long sub_if, unsigned long *is_up)
{
    short int olt_id;
    short int llid;

    llid = (short int)sub_if;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slot_id, (short int)port_id );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
            if ( OLTAdv_LLIDIsExist( olt_id, llid ) )
            {
                *is_up = TRUE;
            }
            else
            {
                *is_up = FALSE;
            }
            
            return VOS_OK;
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return VOS_ERROR;
}


int PonStdOamGetPeerStatus(short int olt_id, short int llid, int status_code, unsigned long *status_val, unsigned char *status_buf)
{
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);
            return MN_EFM_GetPeerOamTable(if_index, status_code, status_val, status_buf);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}


int PonStdOamGetHeartInfos(short int olt_id, short int llid, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_size, unsigned char *recv_data)
{
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);
            return efm_oam_get_info_pkt_by_if(if_index, send_size, send_data, recv_size, recv_data);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

int PonStdOamGetHeartMode(short int olt_id, short int llid, int *heart_mode)
{
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);
            return efm_oam_get_heart_mode(if_index, heart_mode);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

int PonStdOamSetHeartMode(short int olt_id, short int llid, int heart_mode)
{
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);
            return efm_oam_set_heart_mode(if_index, heart_mode);
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

#endif


#if 1
/**************************扩展OAM接口****************************/

int PonExtOamEvtTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len)
{
    VOS_ASSERT(0);

    return VOS_ERROR;
}

int PonExtOamInfoTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len)
{
    int result;
    int retry_times;
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);
            
            retry_times = 0;    
            while ( EFM_ERROR_STATE == (result = send_efm_orgSpecific_info(if_index, oam_pkt, pkt_len)) )
            {
                if ( ++retry_times < 5 )
                {
                    VOS_TaskDelay(retry_times * 10);
                }
                else
                {
                    break;
                }
            }

            return result;
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

int PonExtOamPktTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len)
{
    int result;
    int retry_times;
	ULONG if_index;
    ULONG slot;
    ULONG port;
    ULONG subif;

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        if ( 0 >= llid )
        {
            llid = PONBROADCASTLLID;
        }

        if ( LLID_ISUNICAST(llid) )
        {
            slot  = GetCardIdxByPonChip(olt_id);
            port  = GetPonPortByPonChip(olt_id);
            subif = llid;

            if_index = IFM_ETH_CREATE_INDEX2(slot, port, subif);

            retry_times = 0;    
            while ( EFM_ERROR_STATE == (result = send_efm_orgSpecific_pkt(if_index, oam_pkt, pkt_len)) )
            {
                if ( ++retry_times < 5 )
                {
                    VOS_TaskDelay(retry_times * 10);
                }
                else
                {
                    break;
                }
            }

            return result;
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return VOS_ERROR;
}

    
static void PonExtOamPktRx( EFM_OAM_orgSpe_s orgData )
{
    ULONG if_index;
 	UINT16 slotNo;
	UINT16 portNo;
	UINT16 subif;
    short int olt_id;
    short int llid;

    if_index = orgData.if_index;
	slotNo = PortIndex_GetSlotNo( if_index );
	portNo = PortIndex_GetPortNo( if_index );
    subif  = PortIndex_GetSubIfNo( if_index );

    llid = (short int)subif;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slotNo, (short int)portNo );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
        	if ( OLTAdv_LLIDIsExist( olt_id, llid ) )
        	{
        	    if ( orgData.len > sizeof(OAM_oui_t) )
                {
            	    PON_OAM_MODULE_receive_frame(olt_id, llid, PON_OAM_CODE_VENDOR_EXTENSION, orgData.data, (unsigned short)(orgData.len - sizeof(OAM_oui_t)), orgData.data + sizeof(OAM_oui_t));
                }   
                else
                {
                    VOS_ASSERT(0);
                }
        	}
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return;
}
    
static void PonExtOamInfoRx( EFM_OAM_orgSpe_s orgData )
{
    ULONG if_index;
 	UINT16 slotNo;
	UINT16 portNo;
	UINT16 subif;
    short int olt_id;
    short int llid;

    if_index = orgData.if_index;
	slotNo = PortIndex_GetSlotNo( if_index );
	portNo = PortIndex_GetPortNo( if_index );
    subif  = PortIndex_GetSubIfNo( if_index );

    llid = (short int)subif;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slotNo, (short int)portNo );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
        	if ( OLTAdv_LLIDIsExist( olt_id, llid ) )
        	{
        	    if ( orgData.len >= 4 + sizeof(OAM_oui_t) )
                {
            	    PON_OAM_MODULE_receive_frame(olt_id, llid, PON_OAM_CODE_OAM_IMFORMATION, orgData.data + 2, (unsigned short)orgData.len, orgData.data);
                }   
                else
                {
                    VOS_ASSERT(0);
                }
        	}
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return;
}

static void PonExtOamEvtRx( EFM_OAM_orgSpe_s orgData )
{
    ULONG if_index;
 	UINT16 slotNo;
	UINT16 portNo;
	UINT16 subif;
    short int olt_id;
    short int llid;

    if_index = orgData.if_index;
	slotNo = PortIndex_GetSlotNo( if_index );
	portNo = PortIndex_GetPortNo( if_index );
    subif  = PortIndex_GetSubIfNo( if_index );

    llid = (short int)subif;
    if ( LLID_ISUNICAST(llid) )
    {
        olt_id = GetPonPortIdxBySlot( (short int)slotNo, (short int)portNo );
        if ( OLT_LOCAL_ISVALID(olt_id) )    
        {
        	if ( OLTAdv_LLIDIsExist( olt_id, llid ) )
        	{
        	    if ( orgData.len > sizeof(OAM_oui_t) )
                {
            	    PON_OAM_MODULE_receive_frame(olt_id, llid, PON_OAM_CODE_EVENT_NOTIFICATION, orgData.data + 2, (unsigned short)orgData.len, orgData.data);
                }   
                else
                {
                    VOS_ASSERT(0);
                }
        	}
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return;
}

#endif


int PonStdOamInit()
{
    /* 初始化上层接口模块 */
    PON_OAM_MODULE_init();

    /* 注册扩展报文接收回调函数 */
    efm_oam_event_register( EFM_OAM_ORGSPE_DATA, (void *)PonExtOamPktRx );
    efm_oam_event_register( EFM_OAM_ORGSPE_INFO, (void *)PonExtOamInfoRx );
    efm_oam_event_register( EFM_OAM_ORGSPE_EVENT, (void *)PonExtOamEvtRx );

    /* 注册"注册/离线"回调函数 */
    efm_oam_event_register( EFM_OAM_REG_INFO, (void *)PonStdOamDiscoverNotify);

    /* 注册事件通知回调函数 */
    efm_oam_event_register( EFM_OAM_FAULT_NOTIFY, (void *)PonStdOamFaultNotify );

    return 0;
}


#ifdef __cplusplus

}

#endif

