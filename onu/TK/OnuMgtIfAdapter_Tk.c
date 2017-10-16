/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_Tk.c
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
*   Date: 			2012/10/08
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  12/10/08  |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "syscfg.h"
#include  "vos/vospubh/vos_base.h"
#include  "vos/vospubh/vos_sem.h"
#include  "includefromPas.h"
#include  "includefromTk.h"
#include  "includefromCmc.h"
#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include  "PonGeneral.h"
#include  "cli/cl_vty.h"
#include "onu/onuOamUpd.h"
#include "onu/onuConfMgt.h"
#include "onu/Onu_oam_comm.h"
#include "ct_manage/CT_Onu_Voip.h"
#include "../onu/OnuPortStatsMgt.h"
#include  "../onu/DrPeng_manage_api.h"


#define CMC_CHANNELID_2_IFINDEX(channel_type, channel_id)  ((channel_type) * 100 + (channel_id))


extern PON_onu_address_table_record_t  MAC_Address_Table[PON_ADDRESS_TABLE_SIZE];
extern int ctc_voip_debug ;
#define CTC_VOIP_DEBUG if(ctc_voip_debug) sys_console_printf

/****************************************内部适配*******************************************/

extern CHAR *macAddress_To_Strings(UCHAR *pMacAddr);
static int GWONU_TK3723_DeregisterOnu(short int olt_id, short int onu_id);

#if 1
/* -------------------ONU基本API------------------- */

static int REMOTE_OK(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_TK_DEBUG(OLT_TK_TITLE"REMOTE_OK(%d, %d)'s result(0).\r\n", olt_id, onu_id);

    return OLT_ERR_OK;
}

static int REMOTE_ERROR(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_TK_DEBUG(OLT_TK_TITLE"REMOTE_ERROR(%d, %d)'s result(%d).\r\n", olt_id, onu_id, OLT_ERR_NOTEXIST);

    return OLT_ERR_NOTEXIST;
}

static int GWONU_TK3723_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    int iRlt = 0;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    *status = 0;
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            *status = OnuIsExist(olt_id, onuid);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_OnuIsOnline(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* ONU 手动添加*/
static int GWONU_TK3723_AddOnuByManual(short int olt_id, short int onu_id, unsigned char *MacAddr)
{
    short int OnuEntry;
    unsigned char *pSrcMac;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(MacAddr);

    OnuEntry = olt_id * MAXONUPERPON + onu_id;    
    pSrcMac  = OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr;
    if ( 0 != VOS_MemCmp(pSrcMac, MacAddr, BYTES_IN_MAC_ADDRESS) )
    {
        /* 强行霸占记录地址 */
	    GWONU_TK3723_DeregisterOnu(olt_id, onu_id);
        OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_AddOnuByManual(%d,%d) deregister onu(%s) for onu(%s) on slot %d.\r\n", olt_id, onu_id, macAddress_To_Strings(pSrcMac), macAddress_To_Strings(MacAddr), SYS_LOCAL_MODULE_SLOTNO);
    }   

    return( ROK );
}

/* 删除ONU */
static int GWONU_TK3723_DelOnuByManual(short int olt_id, short int onu_id)
{	     
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( activeOneLocalPendingOnu(olt_id, onu_id) == RERROR )
	{
	    GWONU_TK3723_DeregisterOnu(olt_id, onu_id);
	}

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return ROK;
}

static int GWONU_TK3723_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_TK3723;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_TK3723_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_TK3723;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}

static int GWONU_BCM55524_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_BCM55524;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_BCM55524_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_BCM55524;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
/*onu deregister*/
static int GWONU_TK3723_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int onuid;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkONU_Deregister(olt_id, onuid);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_DeregisterOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_TK3723_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int onuid;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TK_authorize_onu(olt_id, onuid, auth_mode);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_AuthorizeOnu(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int onuid;
    short int llid;
    TkCtcOnuAuthRequestInfo auth_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(auth_response);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuAuthInfo(olt_id, onuid, (unsigned char)TkCtcOnuAuthenticationTypeLoidPassword /* auth_response->auth_type */, &auth_info)) )
            {
                if ( CTC_AUTH_TYPE_LOID == (auth_response->auth_type = auth_info.type) )
                {
                    VOS_MemCpy(auth_response->loid_data.onu_id, auth_info.x.LoidPassword.loid, CTC_AUTH_ONU_ID_SIZE);
                    VOS_MemCpy(auth_response->loid_data.password, auth_info.x.LoidPassword.password, CTC_AUTH_PASSWORD_SIZE);
                }
                else
                {
                    auth_response->desired_auth_type = auth_info.x.Nak.desiredAuthenticationType;
                }
            }
        }
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_AuthRequest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_TK3723_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int onuid;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetOnuAuthResult(olt_id, onuid, 0);
        }
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_AuthSuccess(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_TK3723_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int onuid;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetOnuAuthResult(olt_id, onuid, (unsigned char)failure_type);
        }
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_AuthFailure(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, failure_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
/*traffic service enable*/
/* 5001和5201共用此函数*//*问题单11283*/
static int GWONU_TK3723_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt = 0;
    short int llid;
    short int OnuEntry;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    /* modified by xieshl 20111010, 避免去激活ONU时造成反复离线注册，问题单13516 */
    /*if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PAS_deregister_onu( olt_id, llid, FALSE );
    }
    else
	 return OLT_ERR_PARAM;*/
	
    OnuEntry = olt_id * MAXONUPERPON + onu_id;
    if( service_mode == V2R1_ENABLE )
    {
        ActivateOnePendingOnu( olt_id, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );	
    }
    else if(service_mode == V2R1_DISABLE)
    {
        /*if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            iRlt = PAS_deregister_onu( olt_id, llid, FALSE );
        }*/
	    iRlt =  GetOnuOperStatus(olt_id, onu_id);
	    if( iRlt == ONU_OPER_STATUS_UP )
	    {
		 OnuMgmtTable[OnuEntry].TrafficServiceEnable = service_mode;
         iRlt = GWONU_TK3723_DeregisterOnu(olt_id, onu_id);
	    }
    }
    else
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt = 0;
    short int llid;
    short int OnuEntry;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OnuEntry = olt_id * MAXONUPERPON + onu_id;
    if( service_mode == V2R1_ENABLE )
    {
        AddMacAuthentication( olt_id, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
    }
    else if( service_mode == V2R1_DISABLE )
    {	
        DeleteMacAuthentication( olt_id, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
        iRlt = GWONU_TK3723_DeregisterOnu(olt_id, onu_id);
    }
    else 
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P */
static int GWONU_TK3723_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int Llid1, Llid2;
    short int onuid;
	short int onuArray[1] = {-1};
    bool access;

	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id1);
	ONU_ASSERT(onu_id2);

	Llid1 = GetLlidActivatedByOnuIdx(olt_id, onu_id1);
	Llid2 = GetLlidActivatedByOnuIdx(olt_id, onu_id2);
	/*modified by liyang @2015-04-30 for Q25510*/	
	if((Llid1 != INVALID_LLID) && (Llid2 != INVALID_LLID))
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, Llid1, &onuid)) )
        {
            if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, Llid2, &onuArray[0])) )
            {
     
                access = (V2R1_ENABLE == enable) ? TRUE : FALSE;
            	iRlt = TkOnuSetP2PAccessList( olt_id, onuid, 1, onuArray, access, FALSE );
            }
        }

    }
	else
	{
		if(Llid2 == INVALID_LLID)
		{
			if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, Llid1, &onuid)) )
	        {
	        	access = (V2R1_ENABLE == enable) ? TRUE : FALSE;
            	iRlt = TkOnuSetP2PAccessList( olt_id, onuid, 1, onuArray, access, FALSE );  
	        }
		}
	}
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id1, onu_id2, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P的ONU，是否转发无效MAC */
static int GWONU_TK3723_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast)
{
    int iRlt = OLT_ERR_NOTEXIST;
#if 0    
	bool address_b, broadcast_b;
	short int llid;

	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	address_b   = (V2R1_ENABLE == address_not_found) ? TRUE : FALSE;
    	broadcast_b = (V2R1_ENABLE == broadcast) ? TRUE : FALSE; 
    	
    	iRlt = PAS_set_llid_p2p_configuration( olt_id, llid, address_b, broadcast_b );
    }   
#else
    iRlt = 0;
#endif
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, address_not_found, broadcast, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GWONU_TK3723_GetSLA(int DIR, ONU_bw_t *BW, TkSlaQueueParams *SLA)
{
    TkSlaShaperParams *min; /* < Min shaper settings */
    TkSlaShaperParams *max; /* < Max shaper settings */

    VOS_ASSERT(BW);
    VOS_ASSERT(SLA);
    
    min = &SLA->min;
    max = &SLA->max;

    if ( 0 == BW->bw_gr )
    {
        min->burst = 0xFFFF;
        min->weight = 0xFFFF;
        min->schedulerLevel = 0xFF;
    }
    else
    {
        if ( BW->bw_gr < 256 )
        {
            BW->bw_gr = 256;
        }
        
        min->schedulerLevel = (BW->bw_class < 6) ? BW->bw_class : 6;
        
        if ( OLT_CFG_DIR_UPLINK == DIR )
        {
            min->weight = uplinkBWWeight;
            
            if ( 256 < uplinkBWlimitBurstSize )
            {
                min->burst = 256;
            }
            else if ( 0 == uplinkBWlimitBurstSize )
            {
                min->burst = 0xFFFF;
            }
            else
            {
                min->burst = uplinkBWlimitBurstSize;
            }
        }
        else
        {
            min->weight = downlinkBWWeight;
            
            if ( 256 < downlinkBWlimitBurstSize )
            {
                min->burst = 256;
            }
            else if ( 0 == downlinkBWlimitBurstSize )
            {
                min->burst = 0xFFFF;
            }
            else
            {
                min->burst = downlinkBWlimitBurstSize;
            }
        }
    }

    if ( 0 == BW->bw_be )
    {
        BW->bw_be = BW->bw_gr;
        
        max->weight = 0xFFFF;
        max->burst = 0xFFFF;
        max->schedulerLevel = 0xFF;
    }
    else
    {
        if ( BW->bw_be < 256 )
        {
            BW->bw_be = 256;
        }
        
        if ( (BW->bw_be - BW->bw_gr) < 256 )
        {
#if 1
            /* 就小不就大 */    
            BW->bw_be = BW->bw_gr;
            
            max->weight = 0xFFFF;
            max->burst = 0xFFFF;
            max->schedulerLevel = 0xFF;
#else
            /* 就大不就小 */    
            BW->bw_gr = 0;
            min->burst = 0xFFFF;
            min->weight = 0xFFFF;
            min->schedulerLevel = 0xFF;
            
            max->schedulerLevel = 2;
            max->weight = 4;
            max->burst  = 255;
#endif
        }
        else
        {
            max->schedulerLevel = (0xFF == min->schedulerLevel) ? 2 : min->schedulerLevel + 1;
            max->weight = (0xFFFF == min->weight) ? 4 : min->weight;
            max->burst  = (0xFFFF == min->burst) ? 256 : min->burst;
        }
    }

    min->bandwidth = BW->bw_gr;
    max->bandwidth = BW->bw_be;

    return 0;
}

static int GWONU_TK3723_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    TkLinkSlaDbaInfo  SLA;
	TkSlaQueueParams DefaultSLA;

    if ( 0 == (iRlt = TkOLT_GetLinkSLA(olt_id, llid, &SLA)) )
    {
        if ( 0 == (iRlt = GWONU_TK3723_GetSLA(OLT_CFG_DIR_UPLINK, BW, &SLA.sla)) )
        {
            TkSlaDbaParams *DBA = &SLA.dba;
        
            if ( 0 == BW->bw_fixed )
            {
                DBA->dbaOptions = TkSlaDbaOptionsForceReport;
                DBA->pollingLevel = 1;

                DBA->tdmRate = 0;
                DBA->tdmGrantLength = 0;
            }
            else
            {
                U32 FixGrantBW;
                U32 FixGrantRate;
                U32 FixGrantLenB;
            
                DBA->dbaOptions = TkSlaDbaOptionsForceReport;
          		DBA->pollingLevel = 1;
				
                /* 先由配置，确认Grant尺寸 */
                FixGrantLenB = uplinkBWPacketUnitSize;
                if ( FixGrantLenB < 84 )
                {
                    FixGrantLenB = 84;
                }
                else if ( FixGrantLenB > 131072 )
                {
                    FixGrantLenB = 131072;
                }

                DBA->tdmGrantLength = FixGrantLenB;
                
                /* 再从固定带宽配置，算出Grant周期 */
                if ( (FixGrantLenB <<= 5) >= (FixGrantBW = BW->bw_fixed) )
                {
                    FixGrantRate = FixGrantLenB / FixGrantBW;
                    
                    if ( FixGrantRate < 1 )
                    {
                        FixGrantRate = 1;
                    }
                    else if ( FixGrantRate > 4000 )
                    {
                        FixGrantRate = 4000;
                    }

                    DBA->tdmRate = FixGrantRate;
                }
                else
                {
                    return TKCOMM_FIXED_BW_ERROR;
                }
            }
           /*modified by liyang 2014-12-05 for Q23006*/
			if(0 != BW->bw_fixed)
			{
				TkBridgeGetDefaultSLA(&DefaultSLA);
				iRlt = TkBridgeSetLinkSLA(olt_id,llid,PON_BRIDGE_PATH_S_UPLINK,&DefaultSLA);

				if(iRlt == TK_EXIT_OK)
				{
					iRlt = TkOLT_SetLinkSLA(olt_id, llid, &SLA);
				}
				
			}
			else
        	{
            	iRlt = TkOLT_SetLinkSLA(olt_id, llid, &SLA);
				if(iRlt == TK_EXIT_OK)
				{
					iRlt = TkBridgeSetLinkSLA(olt_id,llid,PON_BRIDGE_PATH_S_UPLINK,&SLA.sla);
				}
			}
		}
    }
        
    return iRlt;
}

static int GWONU_TK3723_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    TkSlaQueueParams sla;

    if( (BW->bw_gr > 0)
        && ((PonPortTable[olt_id].DownlinkPoliceMode == V2R1_ENABLE)
        || (OLT_CFG_DIR_INACTIVE & BW->bw_direction)) )
    {
        if ( 0 == (iRlt = GWONU_TK3723_GetSLA(OLT_CFG_DIR_DOWNLINK, BW, &sla)) )
        {
            iRlt = TkBridgeSetLinkSLA(olt_id, llid, PON_BRIDGE_PATH_S_DOWNLINK, &sla);
            if (0 == iRlt)
            {
                /* 输出激活带宽 */
                BW->bw_actived = BW->bw_gr;
            }
        }
    }
    else
    {
        iRlt = TkBridgeDftLinkSLA(olt_id, llid, PON_BRIDGE_PATH_S_DOWNLINK);
        if (0 == iRlt)
        {
            /* 输出激活带宽 */
            BW->bw_actived = 0;
        }
    }

    return iRlt;
}

static int  GWONU_TK3723_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    if(!(OLT_CFG_DIR_DO_ACTIVE & BW->bw_direction))
    {
        iRlt = OLT_ERR_OK;
    }       
	else if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        ONUTable_S *pstOnuCfg = &(OnuMgmtTable[olt_id * MAXONUPERPON + onu_id]);

        if ( OLT_CFG_DIR_BOTH == (OLT_CFG_DIR_BOTH & BW->bw_direction) )
        {
            int DownlinkActiveBW;
            unsigned int UplinkClass;
            unsigned int UplinkDelay;
            ONU_bw_t BW_NewDefault;

            /* 尝试设置新的默认带宽，失败也是合理的*/
            iRlt = 0;
            DownlinkActiveBW = -1;
            if ( !(OLT_CFG_DIR_UNDO & BW->bw_direction)
                || (OLT_CFG_DIR_DOWNLINK & pstOnuCfg->BandWidthIsDefault) )
            {
                /* 先设置下行默认带宽 */
                VOS_MemZero(&BW_NewDefault, sizeof(ONU_bw_t));
                BW_NewDefault.bw_direction = OLT_CFG_DIR_DOWNLINK;
                BW_NewDefault.bw_gr = BW->bw_fixed;                
                BW_NewDefault.bw_be = BW->bw_actived;                
                iRlt = GWONU_TK3723_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                if ( 0 == iRlt )
                {
                    /* 保存下行激活带宽 */
                    DownlinkActiveBW = BW_NewDefault.bw_actived;
                    BW->bw_delay = DownlinkActiveBW;
                }
            }

            if ( 0 == iRlt )
            {
                if ( !(OLT_CFG_DIR_UNDO & BW->bw_direction)
                    || (OLT_CFG_DIR_UPLINK & pstOnuCfg->BandWidthIsDefault) )
                {
                    /* 再设置上行默认带宽 */
                    VOS_MemZero(&BW_NewDefault, sizeof(ONU_bw_t));
                    BW_NewDefault.bw_direction = OLT_CFG_DIR_UPLINK;
                    BW_NewDefault.bw_gr        = BW->bw_gr;
                    BW_NewDefault.bw_be        = BW->bw_be;
                    if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
                    {
                        BW_NewDefault.bw_class = OnuConfigDefault.UplinkClass;
                        BW_NewDefault.bw_delay = OnuConfigDefault.UplinkDelay;
                    }
                    else
                    {
                        BW_NewDefault.bw_class = BW->bw_class;
                        BW_NewDefault.bw_delay = BW->bw_delay;
                    }

                    iRlt = GWONU_TK3723_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);
                    if ( 0 == iRlt )
                    {
                        /* 输出上下行激活带宽 */
                        BW->bw_class = BW->bw_gr;
                    }
                    else
                    {
#if 0
                        /* B--added by liwei056@2012-8-22 for OnuDefaultBW's ShowRealAllocBW */
                        if ( OLT_CFG_DIR_FORCE & BW->bw_direction )
                        {
                            BW->bw_class = pstLlidCfg->ActiveUplinkBandwidth + pstLlidCfg->FinalUplinkBandwidth_fixed;
                            iRlt = 0;
                        }
                        else
                        /* E--added by liwei056@2012-8-22 for OnuDefaultBW's ShowRealAllocBW */
#endif
                        {
                            if ( DownlinkActiveBW >= 0 )
                            {
                                /* 上行设置失败，则需恢复旧的下行默认带宽 */
                                VOS_MemZero(&BW_NewDefault, sizeof(ONU_bw_t));
                                BW_NewDefault.bw_direction = OLT_CFG_DIR_DOWNLINK;
                                BW_NewDefault.bw_gr = pstOnuCfg->FinalDownlinkBandwidth_gr;                
                                BW_NewDefault.bw_be = pstOnuCfg->FinalDownlinkBandwidth_be;                
                                (void)GWONU_TK3723_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if ( OLT_CFG_DIR_UPLINK & BW->bw_direction )
            {
                if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
                {
                    /*一个onu同时有基于onuid和mac的带宽配置时，undo mac时应恢复为基于onuid的配置*/
                    /*modi by luh 2012-9-18*/
                    if((!(pstOnuCfg->LlidTable[0].BandWidthIsDefault & OLT_CFG_DIR_UPLINK))
                        &&(!(BW->bw_direction & OLT_CFG_DIR_BY_ONUID)))
                    {
                        BW->bw_fixed = pstOnuCfg->LlidTable[0].UplinkBandwidth_fixed;
                        BW->bw_gr = pstOnuCfg->LlidTable[0].UplinkBandwidth_gr;
                        BW->bw_be = pstOnuCfg->LlidTable[0].UplinkBandwidth_be;
                        BW->bw_class = pstOnuCfg->LlidTable[0].UplinkClass;
                        BW->bw_delay = pstOnuCfg->LlidTable[0].UplinkDelay;
                    }
                }
                
                iRlt = GWONU_TK3723_SetOnuUplinkBW(olt_id, llid, BW);
                if (0 == iRlt)
                {
                    /* 输出激活带宽 */
                    BW->bw_actived = BW->bw_gr;
                }
            }
            else
            {
                if ( OLT_CFG_DIR_UNDO & BW->bw_direction )
                {
                    /*一个onu同时有基于onuid和mac的带宽配置时，undo mac时应恢复为基于onuid的配置*/                    
                    /*modi by luh 2012-9-18*/
                    if((!(pstOnuCfg->LlidTable[0].BandWidthIsDefault & OLT_CFG_DIR_DOWNLINK))
                        &&(!(BW->bw_direction & OLT_CFG_DIR_BY_ONUID)))
                    {
                        BW->bw_gr = pstOnuCfg->LlidTable[0].DownlinkBandwidth_gr;
                        BW->bw_be = pstOnuCfg->LlidTable[0].DownlinkBandwidth_be;
                        BW->bw_class = pstOnuCfg->LlidTable[0].DownlinkClass;
                        BW->bw_delay = pstOnuCfg->LlidTable[0].DownlinkDelay;
                    }
                }
                
                iRlt = GWONU_TK3723_SetOnuDownlinkBW(olt_id, llid, BW);
            }
        }
    }   
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GWONU_TK3723_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkOLT_GetLinkSLA(olt_id, llid, &sla_info->SLA.SLA5)) )
        {
            sla_info->SLA_Ver = 5;
            sla_info->DBA_ErrCode = 0;
        }
    }   
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuSLA(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, sla_info->SLA_Ver, sla_info->DBA_ErrCode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_TK3723_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkFecAbilities Fec_Ability;
        
            if ( 0 == (iRlt = TkONU_GetFECAbility(olt_id, onuid, &Fec_Ability)) )
            {
                if ( 0 != *(U16*)&Fec_Ability )
                {
                    TkFecInfo Fec_Info;

                    VOS_MemZero(&Fec_Info, sizeof(Fec_Info));
                    
                    if ( Fec_Ability.upstreamFecSupported )
                    {
                        Fec_Info.upstreamFecEnabled = TRUE;
                    }
                    
                    if ( Fec_Ability.downstreamFecSupported )
                    {
                        Fec_Info.downstreamFecEnabled = TRUE;
                    }
                    
                    iRlt = TkONU_SetFECMode(olt_id, onuid, &Fec_Info);
                }
                else
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
        }
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int CTCONU_TK3723_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkFecAbilities Fec_Ability;
        
            if ( 0 == (iRlt = TkCTC_GetOnuFECAbility(olt_id, onuid, &Fec_Ability)) )
            {
                if ( 0 != *(U16*)&Fec_Ability )
                {
                    TkFecInfo Fec_Info;
                    U8 bValue = (STD_FEC_MODE_ENABLED == fec_mode) ? TRUE : FALSE;

                    VOS_MemZero(&Fec_Info, sizeof(Fec_Info));
                    
                    if ( Fec_Ability.upstreamFecSupported )
                    {
                        Fec_Info.upstreamFecEnabled = bValue;
                    }
                    
                    if ( Fec_Ability.downstreamFecSupported )
                    {
                        Fec_Info.downstreamFecEnabled = bValue;
                    }
                    
                    iRlt = TkCTC_SetOnuFECMode(olt_id, onuid, &Fec_Info);
                }
                else
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
        }
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}


static int GWONU_TK3723_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TK_PASONU_uni_set_port( olt_id, onuid, enable_cpu, enable_datapath );
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetUniPort(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable_cpu, enable_datapath, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_TK3723_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iPonChipVendorID;
    short int llid, onuid;
    TkLinkOamRate params;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            switch ( iPonChipVendorID = GetOnuChipVendor(olt_id, onu_id) )
            {
                case PONCHIP_VENDOR_PAS:
                    iRlt = TK_PASONU_set_slow_protocol_limit( olt_id, onuid, enable );
                default:
                    if ( enable )
                    {
                        params.maxRate = 3;
                        params.minRate = 10;
                    }
                    else
                    {
#if 0
                        params.maxRate = 25;
                        params.minRate = 10;
#else
                        /* 注意: minRate=0 会造成OAM信道的关闭 */
                        params.maxRate = 0;
                        params.minRate = 10;
#endif
                    }

                    iRlt = TkOLT_SetLinkOamRate( olt_id, llid, &params );
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_TK3723_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iPonChipVendorID;
    short int llid, onuid;
    TkLinkOamRate params;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            switch ( iPonChipVendorID = GetOnuChipVendor(olt_id, onu_id) )
            {
                case PONCHIP_VENDOR_PAS:
                    iRlt = TK_PASONU_get_slow_protocol_limit( olt_id, onuid, enable );
                    break;
                default:
                    if ( 0 == (iRlt = TkOLT_GetLinkOamRate(olt_id, llid, &params)) )
                    {
                        if ( (0 == params.maxRate) || (10 < params.maxRate) )
                        {
                            *enable = FALSE;
                        }
                        else
                        {
                            *enable = TRUE;
                        }
                    }
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_TK3723_GetOnuB2PMode(short int olt_id, short int onu_id, int *b2p_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 <= (iRlt = TkBridgeGetOnuDataPathNum(olt_id, onuid)) )
            {
                *b2p_mode = (1 < iRlt) ? TRUE : FALSE;
                iRlt = 0;
            }
        }
    }
	
	OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOnuB2PMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int GWONU_TK3723_SetOnuB2PMode(short int olt_id, short int onu_id, int b2p_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 < (iRlt = TkBridgeSetOnuDataPathNum(olt_id, onuid, (b2p_mode) ? 2 : 1)) )
            {
                iRlt = 0;
            }
        }
    }
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuB2PMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */

static int GWONU_TK3723_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
        	if ( 0 == (iRlt = TkONU_ResetAllCounter ( olt_id, onuid )) )
            {
                TkOLT_ResetAllCounter(olt_id);
            }   
        }   
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_ResetCounters(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_TK3723_SetPonLoopback(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
        	if( enable == V2R1_ENABLE )
            {
                iRlt = TkONU_EnableLinkLoopback(olt_id, onuid, TkLoopbackLocationMac, TkLoopbackDirectionDownstreamToUpstream);
            }
            else
            {
                iRlt = TkONU_DisableLinkLoopback(olt_id, onuid, TkLoopbackLocationMac, TkLoopbackDirectionDownstreamToUpstream);
            }
        }   
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetPonLoopback(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU加密管理API------------------- */
/* get onu pon Llid parameters */
static int GWONU_TK3723_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	if ( 0 == (iRlt = TkGetLinkMacAddress( olt_id, llid, llid_parameters->mac_address )) )
        {
            TkLinkEncryptionInfo encry_info;
                
            if ( 0 == (iRlt = TkOLT_GetLinkEncryptMode2(olt_id, llid, &encry_info)) )
            {
                if ( TkEncryptModeNone == encry_info.encryptionMode )
                {
                    llid_parameters->encryption_mode = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
                }
                else
                {
                    if ( TkEncryptDirectionBiDirectional == encry_info.encryptionDirection )
                    {
                        llid_parameters->encryption_mode = PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK;
                    }
                    else
                    {
                        llid_parameters->encryption_mode = PON_ENCRYPTION_DIRECTION_DOWNLINK;
                    }
                }
                
                llid_parameters->authorization_mode = PON_AUTHORIZE_TO_THE_NETWORK;
                llid_parameters->oam_version = OAM_STANDARD_VERSION_3_3;
            }   
        }   
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetLLIDParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*onu start encryption */
static int GWONU_TK3723_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int enc_dir;
    short int llid;
    TkLinkEncryptionInfo encry_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(encrypt_dir);

    enc_dir = *encrypt_dir;
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        if ( 0 == (iRlt = TkOLT_GetLinkEncryptMode2(olt_id, llid, &encry_info)) )
        {
            if ( TkEncryptModeNone == encry_info.encryptionMode )
            {
                encry_info.encryptionMode = TkEncryptModeTripleChurning;
            }
            
            if ( 0 == encry_info.keyExchangeTime )
            {
                unsigned int keyExchangeTimeout;

                if ( 0xFFFF < (keyExchangeTimeout = PonPortTable[olt_id].EncryptKeyTime / 1000) )
                {
                    keyExchangeTimeout = 0xFFFF;
                }
                else 
                {
                    if ( 0 == keyExchangeTimeout )
                    {
                        keyExchangeTimeout = EncryptKeyTimeDefault;
                    }
                }

                encry_info.keyExchangeTime = (U16)keyExchangeTimeout;
            }

            if ( PON_ENCRYPTION_DIRECTION_ALL == enc_dir )
            {
                encry_info.encryptionDirection = TkEncryptDirectionBiDirectional;
            }
            else
            {
                encry_info.encryptionDirection = TkEncryptDirectionDownOnly;
            }
            
        	if ( 0 == (iRlt = TkOLT_SetLinkEncryptMode2( olt_id, llid, &encry_info )) )
            {
                /* 标识加密已经启动 */
                *encrypt_dir = 0;
            }   
        }
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_StartEncryption(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

/*onu stop encryption*/
static int GWONU_TK3723_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	iRlt = TkOLT_DeleteLinkEncryptMode ( olt_id, llid );
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_StopEncryption(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

static int GWONU_TK3723_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int enc_dir;
    unsigned int keyExchangeTimeout;
    short int llid;
    TkLinkEncryptionInfo encry_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    enc_dir = *encrypt_dir;
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        if ( 0 == (iRlt = TkOLT_GetLinkEncryptMode2(olt_id, llid, &encry_info)) )
        {
            if ( TkEncryptModeNone == encry_info.encryptionMode )
            {
                encry_info.encryptionMode = TkEncryptModeTripleChurning;
            }
            
            if ( 0 < key_change_time )
            {
                keyExchangeTimeout = key_change_time;
            }
            else
            {
                if ( 0xFFFF < (keyExchangeTimeout = PonPortTable[olt_id].EncryptKeyTime / 1000) )
                {
                    keyExchangeTimeout = 0xFFFF;
                }
                else 
                {
                    if ( 0 == keyExchangeTimeout )
                    {
                        keyExchangeTimeout = EncryptKeyTimeDefault;
                    }
                }
            }
            encry_info.keyExchangeTime = (U16)keyExchangeTimeout;

            if ( PON_ENCRYPTION_DIRECTION_ALL == enc_dir )
            {
                encry_info.encryptionDirection = TkEncryptDirectionBiDirectional;
            }
            else
            {
                encry_info.encryptionDirection = TkEncryptDirectionDownOnly;
            }
            
        	if ( 0 == (iRlt = TkOLT_SetLinkEncryptMode2( olt_id, llid, &encry_info )) )
            {
                /* 标识加密已经启动 */
                *encrypt_dir = 0;
            }   
        }
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, key_change_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

static int GWONU_TK3723_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int iRlt;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        TkLinkEncryptionInfo encry_info;
            
        if ( 0 == (iRlt = TkOLT_GetLinkEncryptMode2(olt_id, llid, &encry_info)) )
        {
            if ( TkEncryptModeNone == encry_info.encryptionMode )
            {
                *encrypt_dir = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
                *encrypt_status = 2;
            }
            else
            {
                if ( TkEncryptDirectionBiDirectional == encry_info.encryptionDirection )
                {
                    *encrypt_dir = PON_ENCRYPTION_DIRECTION_UPLINK_AND_DOWNLINK;
                }
                else
                {
                    *encrypt_dir = PON_ENCRYPTION_DIRECTION_DOWNLINK;
                }
                
                *encrypt_status = 1;
            }

            *key_change_time = encry_info.keyExchangeTime;
        }   
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuEncryptParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

#endif


#if 1
/* -------------------ONU 地址管理API------------------- */
/*show fdbentry mac*/
static int GWONU_TK3723_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid, onuid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            long int liEntryNum;
            
            if ( NULL == addr_table )
            {
                addr_table = MAC_Address_Table;
            }
            if ( 0 == (iRlt = TK_PASONU_address_table_get(olt_id, onuid, &liEntryNum, addr_table)) )
            {
                *EntryNum = (long)liEntryNum;
            }
        }
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*show fdbentry mac*/
static int GWONU_TK3723_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid, onuid;
	short int addr_num;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkAdapter_PAS5201_GetLLIDAddrTable(olt_id, llid, &addr_num, addr_table)) )
        {
            *EntryNum = (long)addr_num;
        }
    }   
	
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*set onu max mac*/
static int GWONU_TK3723_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned int uiSetVal;
	short int llid, onuid;
    unsigned short max_entry;
    int default_flag  = 1;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ID_ASSERT(llid_id);
    VOS_ASSERT(val);
    
#if 1
    if(*val & ONU_NOT_DEFAULT_MAX_MAC_FLAG)
    {
        uiSetVal = (*val) & (~ONU_NOT_DEFAULT_MAX_MAC_FLAG);        
        default_flag = 0;
    }
    else if(*val & ONU_UNDO_MAX_MAC_FLAG)
    {
        uiSetVal = (*val) & (~ONU_UNDO_MAX_MAC_FLAG);        
    }
    else        
    {
    	short int max_mac = 0;
    	ONU_MGMT_SEM_TAKE;
    	max_mac = OnuMgmtTable[olt_id * MAXONUPERPON + onu_id].LlidTable[llid_id].MaxMAC;	
    	ONU_MGMT_SEM_GIVE;

    	if(max_mac & ONU_NOT_DEFAULT_MAX_MAC_FLAG)	
    	{
            return iRlt;
    	}
        uiSetVal = *val;
    }
#endif

	if ( INVALID_LLID != (llid = GetLlidByLlidIdx(olt_id, onu_id, llid_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkDestinationLearnControl mac_ctrl;
            
            /*uiSetVal = *val;*/
            if( 0 == uiSetVal ) 
            {
                max_entry = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
            }
            else
            {
                if( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION )
                {
                    uiSetVal  = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
                    max_entry = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
                }
                else 
                {
                    /* 包含ONU自己的一个MAC */
                    max_entry = uiSetVal + 1;
                }
            }

            mac_ctrl.maxLearningEntries = max_entry;
            if ( 0 == (iRlt = TkBridgeSetOnuDestMacCtrl(olt_id, onuid, PON_MAC_CTL_FLAG_MAX, &mac_ctrl)) )
            {
                *val = uiSetVal;
            }
        }
    }   
	
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*get onu pon uni mac config*/
static int GWONU_TK3723_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_config);

#if 0    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PAS_get_onu_uni_port_mac_configuration_v4(olt_id, llid, mac_config);
    }   
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
					
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuUniMacCfg(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
/*RTT*/
static int GWONU_TK3723_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    TkOltLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuRegisterInfo ( olt_id, llid, NULL, NULL, NULL, &link_info, NULL, NULL, NULL, NULL )) )
        {
            *rtt = link_info.rtt;
        }
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuDistance(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GWONU_TK3723_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);
		
#if 0    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        PAS_physical_capabilities_t device_capabilities;

		iRlt = PAS_get_device_capabilities_v4(olt_id, llid, &device_capabilities);
		if ( iRlt == PAS_EXIT_OK )
        {
            VOS_MemCpy(optical_capability, &device_capabilities, sizeof(ONU_optical_capability_t));
        }      
	}
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOpticalCapability(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
#endif


#if 1
/* -------------------ONU 倒换API---------------- */

static int GWONU_TK3723_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);

    if ( LinkIsOnline(olt_id, llid) )
    {
        iRlt = 0;
    }
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }
    
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 设备管理API------------------- */

/*get onu pon chip ver*/
static int GWONU_TK3723_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    short int onuid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PAS5201_GetOnuChipVersion(olt_id, onuid, device_versions);
        }
    }   
	 
	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuPonVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int GWONU_TK3723_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    unsigned char curr_cm_num;
    unsigned char pon_rate_flags = PON_RATE_NORMAL_1G;
    TkOltLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuRegisterInfo ( olt_id, llid
                                , NULL, NULL, NULL, &link_info
                                , &onu_info->max_links_support
                                , &onu_info->curr_links_num
                                , &onu_info->max_cm_support
                                , &curr_cm_num
                                )) )
        {
            onu_info->oam_version = OAM_STANDARD_VERSION_3_3;
            onu_info->rtt = link_info.rtt;
            
            onu_info->laser_on_time  = 0;
            onu_info->laser_off_time = 0;

            onu_info->vendorType     = link_info.vendorType;
            onu_info->productVersion = link_info.productVersion;
            onu_info->productCode    = link_info.productCode;

            /* 需要OAM查出此ONU是否可工作于下行2G模式 */
            onu_info->pon_rate_flags = pon_rate_flags;
        }
    }   

	OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int TK3723_GetI2CAddressByInfoId(short int olt_id, short int onu_id, int info_id, unsigned char *i2cDevAddr, unsigned char *internalDevAddrCount, unsigned char internalDevAddr[])
{
    int iRlt = OLT_ERR_PARAM;

    /* 每个ONU的地址，都需要针对设置 */
    switch ( info_id )
    {
        case EEPROM_MAPPER_LASER_ON_PERMANENTLY:
            break;
        case EEPROM_MAPPER_POINT_TO_POINT_ENABLE:
            break;
        case EEPROM_MAPPER_EEPROM_MAC_ADDR:
            break;
        case EEPROM_MAPPER_USER_NAME_802_1X:
            break;
        case EEPROM_MAPPER_PASSWORD_802_1X:
            break;
        default:
            NULL;
    }

    return iRlt; 			
}

static int GWONU_TK3723_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 1
            iRlt = TK_PASONU_eeprom_mapper_get_parameter( olt_id, onuid, info_id, data, size );
#else
            unsigned short numBytesToRead;
            unsigned char i2cDevAddr;
            unsigned char internalDevAddrCount;
            unsigned char internalDevAddr[8];

            internalDevAddrCount = sizeof(internalDevAddr);
            if ( 0 == (iRlt = TK3723_GetI2CAddressByInfoId(olt_id, onu_id, info_id, &i2cDevAddr, &internalDevAddrCount, internalDevAddr)) )
            {
                numBytesToRead = *size;
                if ( 0 == (iRlt = TkONU_GetI2cData(olt_id, onuid, i2cDevAddr, internalDevAddrCount, internalDevAddr, &numBytesToRead, data)) )
                {
                    *size = numBytesToRead;
                }
            }
#endif
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_TK3723_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 1
            iRlt = TK_PASONU_eeprom_mapper_set_parameter( olt_id, onuid, info_id, data, size );
#else
            unsigned short numBytesToRead;
            unsigned char i2cDevAddr;
            unsigned char internalDevAddrCount;
            unsigned char internalDevAddr[8];
            
            if ( 0 == (iRlt = TK3723_GetI2CAddressByInfoId(olt_id, onu_id, info_id, &i2cDevAddr, &internalDevAddrCount, internalDevAddr)) )
            {
                iRlt = TkONU_SetI2cData(olt_id, onuid, i2cDevAddr, (unsigned short)size, data);
            }
#endif
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_TK3723_ResetOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkONU_Reset( olt_id, onuid );
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_ResetOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int CTCONU_TK3723_ResetOnu(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ResetOnu( olt_id, onuid );
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ResetOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_TK3723_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    short int onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TK_PASONU_get_burn_image_complete( olt_id, onuid, complete );
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetBurnImageComplete(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */

static int CTCONU_TK3723_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkGetOnuCtcVersion(olt_id, onuid, version);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetCtcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcOnuFirmwareVersionParams version_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            VOS_MemZero(&version_info, sizeof(version_info));
            if ( 0 == (iRlt = TkCTC_GetOnuFirmwareVersion(olt_id, onuid, &version_info)) )
            {
                VOS_MemCpy(version, version_info.version, sizeof(unsigned short));
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetFirmwareVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcOnuSerialNumberParams serial_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    VOS_MemZero(&serial_info,sizeof(TkCtcOnuSerialNumberParams));
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuSerialNumber(olt_id, onuid, &serial_info)) )
            {
                serial_number->onu_model = serial_info.onuModel;
                
                VOS_MemCpy(serial_number->vendor_id, serial_info.vendorId, 4);
                VOS_MemCpy(serial_number->onu_id, serial_info.onuMac, 6);
                VOS_MemCpy(serial_number->hardware_version, serial_info.hwVersion, 8);
                VOS_MemCpy(serial_number->software_version, serial_info.swVersion, 16);
		   		VOS_MemCpy(serial_number->extendedModel, serial_info.extendedModel, 16);/*add by yangzl for CTC3.0*/
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetSerialNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcChipsetIdTlv chip_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuChipInfo(olt_id, onuid, &chip_info)) )
            {
                VOS_MemCpy(chipset_id->vendor_id, &chip_info.vendorId, 2);
                VOS_MemCpy(chipset_id->date, chip_info.designDate, 3);

                chipset_id->chip_model = chip_info.chipModel;
                chipset_id->revision = chip_info.revision;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetChipsetID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcOnuCapsTlv cap_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuCap1(olt_id, onuid, &cap_info)) )
            {
                caps->service_supported = cap_info.services;
                
                caps->ge_ethernet_ports_number = cap_info.numGePorts;
                VOS_MemCpy(&caps->ge_ports_bitmap, &cap_info.gePortMap, 8);
                
                caps->fe_ethernet_ports_number = cap_info.numFePorts;
                VOS_MemCpy(&caps->fe_ports_bitmap, &cap_info.fePortMap, 8);
                
                caps->pots_ports_number = cap_info.numPotsPorts;
                caps->e1_ports_number = cap_info.numE1Ports;
                
                caps->upstream_queues_number = cap_info.numUpstreamQueues;
                caps->max_upstream_queues_per_port = cap_info.numQueuesPerPortUpMax;
                
                caps->downstream_queues_number = cap_info.numDownstreamQueues;
                caps->max_downstream_queues_per_port = cap_info.numQueuesPerPortDnMax;

                caps->battery_backup = cap_info.batteryBackup;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOnuCap1(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcOnuCapabilities21 cap_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuCap2(olt_id, onuid, &cap_info)) )
            {
                int i, n;
                CTC_STACK_onu_capability_interface_type_t *aDstIfTypes;
                TkCtc21OnuInterface *aSrcIfTypes;
            
                caps->onu_type = cap_info.onuType;
                
                caps->multi_llid = cap_info.numLinks;
                
                caps->protection_type = cap_info.protectionType;
                
                caps->num_of_pon_if = cap_info.numPonIFs;
                caps->num_of_slot = cap_info.numSlots;

                VOS_MemZero(caps->interface_type, sizeof(caps->interface_type));
                caps->num_of_infc_type = cap_info.interfacesCount;
                if ( 0 < (n = caps->num_of_infc_type) )
                {
                    aDstIfTypes = caps->interface_type;
                    aSrcIfTypes = cap_info.interfaces;
                    
                    for ( i = 0; i < n; i++ )
                    {
                        aDstIfTypes[i].interface_type = aSrcIfTypes[i].type;
                        aDstIfTypes[i].num_of_port = aSrcIfTypes[i].numPorts;
                    }
                }

                caps->battery_backup = cap_info.batteryBackup;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOnuCap2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcOnuCapabilities30 cap_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuCap3(olt_id, onuid, &cap_info)) )
            {
                caps->onu_ipv6_aware = cap_info.ipv6;
                
                caps->onu_power_supply_control = cap_info.power_ctrl;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOnuCap3(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(file_start);
    VOS_ASSERT(file_len > 0);
    VOS_ASSERT(file_name);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkUpgradeCtcOnuFirmware(olt_id, onuid, file_start, file_len, file_name);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_UpdateOnuFirmware(%d, %d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, file_len, file_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_ActiveOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ActiveOnuImage(olt_id, onuid);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ActiveOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_CommitOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_CommitOnuImage(olt_id, onuid);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_CommitOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int *timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

#if 0    
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_extended_oam_discovery_timing(&tm);
        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *timeout = tm;
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

#if 0    
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_set_extended_oam_discovery_timing(timeout);
        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEncryptionTiming(short int olt_id, short int onu_id, int *update_time, int *no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkEncryptionInfo encrypt_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = TkOLT_GetLinkEncryptMode(olt_id, llid, &encrypt_info);
        if (0 == iRlt)
        {
            *update_time = encrypt_info.keyExchangeTimeout;
            *no_reply = encrypt_info.mode;
        }
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *update_time, *no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEncryptionTiming(short int olt_id, short int onu_id, int update_time, int no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkOnuEncrypt(olt_id, onuid, -1, -1, TkEncryptModeTripleChurning, (unsigned short)update_time);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, update_time, no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_StartEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkOnuEncrypt(olt_id, onuid, -1, -1, TkEncryptModeTripleChurning, 0);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_StartEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_StopEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkOnuEncrypt(olt_id, onuid, -1, -1, TkEncryptModeNone, 0);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_StopEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortLinkstate(short int olt_id, short int onu_id,  int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                *enable = port_info.linked;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortLinkstate(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                *enable = port_info.phyEnabled;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;
    TkCtcEthernetPortCtrl port_ctrl;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                if ( port_info.phyEnabled != enable )
                {
                    port_ctrl.phyEnable = (U8)enable;
                    
                    port_ctrl.flowControlEnable = port_info.flowControlEnabled;
                    port_ctrl.autoNegoEnable = port_info.autoNegoEnabled;
                    port_ctrl.restartAutoNego = 0;
                    
                    iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                *enable = port_info.flowControlEnabled;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortPauseEnable(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;
    TkCtcEthernetPortCtrl port_ctrl;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                if ( port_info.flowControlEnabled != enable )
                {
                    port_ctrl.flowControlEnable = (U8)enable;
                    
                    port_ctrl.phyEnable = port_info.phyEnabled;
                    port_ctrl.autoNegoEnable = port_info.autoNegoEnabled;
                    port_ctrl.restartAutoNego = 0;
                    
                    iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortPauseEnable(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                *enable = port_info.autoNegoEnabled;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_OK;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;
    TkCtcEthernetPortCtrl port_ctrl;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                if ( port_info.autoNegoEnabled != enable )
                {
                    port_ctrl.autoNegoEnable = (U8)enable;
                    
                    port_ctrl.phyEnable = port_info.phyEnabled;
                    port_ctrl.flowControlEnable = port_info.flowControlEnabled;
                    port_ctrl.restartAutoNego = 0;
                    
                    iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortRestartAutoConfig(short int olt_id, short int onu_id,  int port_id )
{
    int iRlt = OLT_ERR_OK;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;
    TkCtcEthernetPortCtrl port_ctrl;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                port_ctrl.phyEnable = port_info.phyEnabled;
                port_ctrl.flowControlEnable = port_info.flowControlEnabled;
                port_ctrl.autoNegoEnable = port_info.autoNegoEnabled;
                port_ctrl.restartAutoNego = TRUE;
                
                iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortRestartAutoConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEthPortAnLocalTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(ability);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                int i, n;
            
                if ( 0 < (ability->number_of_abilities = port_info.abilitiesCount) )
                {
                    if ( MAX_TECHNOLOGY_ABILITY >= (n = ability->number_of_abilities) )
                    {
                        for (i=0; i<n; i++)
                        {
                            ability->technology[i] = port_info.abilities[i];
                        }
                    }
                    else
                    {
                        iRlt = OLT_ERR_PARAM;
                    }
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortAnLocalTecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEthPortAnAdvertisedTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcEthernetPortInfo port_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(ability);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                int i, n;
            
                if ( 0 < (ability->number_of_abilities = port_info.advertisedCount) )
                {
                    if ( MAX_TECHNOLOGY_ABILITY >= (n = ability->number_of_abilities) )
                    {
                        for (i=0; i<n; i++)
                        {
                            ability->technology[i] = port_info.advertised[i];
                        }
                    }
                    else
                    {
                        iRlt = OLT_ERR_PARAM;
                    }
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortAnAdvertisedTecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcPolicingParam police_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(policing);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortPolicing(olt_id, onuid, (short int)port_id, &police_info)) )
            {
                VOS_MemZero(policing, sizeof(*policing));
            
                policing->operation = police_info.policingEnabled;

                VOS_MemCpy(((U8*)&policing->cir) + 1, &police_info.cir, sizeof(U24));
                VOS_MemCpy(((U8*)&policing->bucket_depth) + 1, &police_info.policeBucket, sizeof(U24));
                VOS_MemCpy(((U8*)&policing->extra_burst_size) + 1, &police_info.extraBucket, sizeof(U24));
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcPolicingParam police_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            police_info.policingEnabled = policing->operation;

            VOS_MemCpy(&police_info.cir, ((U8*)&policing->cir) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.policeBucket, ((U8*)&policing->bucket_depth) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.extraBucket, ((U8*)&policing->extra_burst_size) + 1, sizeof(U24));
            iRlt = TkCTC_SetEthPortPolicing(olt_id, onuid, (short int)port_id, &police_info);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcDownstreamRateLimitingParam police_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(policing);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetEthPortDownRateLimit(olt_id, onuid, (short int)port_id, &police_info)) )
            {
                VOS_MemZero(policing, sizeof(*policing));
            
                policing->port_number = (U8)port_id;
                policing->state = police_info.enabled;

                VOS_MemCpy(((U8*)&policing->CIR) + 1, &police_info.cir, sizeof(U24));
                VOS_MemCpy(((U8*)&policing->PIR) + 1, &police_info.pir, sizeof(U24));
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcDownstreamRateLimitingParam police_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            police_info.enabled = policing->state;

            VOS_MemCpy(&police_info.cir, ((U8*)&policing->CIR) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.pir, ((U8*)&policing->PIR) + 1, sizeof(U24));
            iRlt = TkCTC_SetEthPortDownRateLimit(olt_id, onuid, (short int)port_id, &police_info);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, vconf);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, vconf);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int CTCONU_TK3723_GetAllPortVlanConfig(short int olt_id, short int onu_id,  unsigned char *portNum, CTC_STACK_vlan_configuration_ports_t ports_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_all_ethport_vlan_config(olt_id, onuid, portNum, ports_info);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetAllPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *portNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port_id configuration*/
static int CTCONU_TK3723_SetPortQinqConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_qinq_configuration_t   port_configuration )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

#if 0    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {

		uchar lport = port_id;

		iRlt = CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, port_configuration);

        	if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

     }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif

	OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortQinqConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_TK3723_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_port_class_config(olt_id, onuid, (short int)port_id, cam);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rule_mode_t  mode, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_set_port_class_config(olt_id, onuid, (short int)port_id, mode, cam);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ClearPortClass(olt_id, onuid, (short int)port_id);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned short vlan_nums;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            vlan_nums = CTC_MAX_MULTICAST_VLAN_ENTRIES;
            if ( 0 == (iRlt = TkCTC_GetPortMulticastVlan(olt_id, onuid, (short int)port_id, &vlan_nums, mv->vlan_id)) )
            {
                mv->num_of_vlan_id = (unsigned char)vlan_nums;
                mv->vlan_operation = CTC_MULTICAST_VLAN_OPER_LIST;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned short vlan_nums;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            switch ( mv->vlan_operation )
            {
                case CTC_MULTICAST_VLAN_OPER_ADD:
                    vlan_nums = mv->num_of_vlan_id;
                    iRlt = TkCTC_AddPortMulticastVlan(olt_id, onuid, (short int)port_id, vlan_nums, mv->vlan_id);
                    break;
                case CTC_MULTICAST_VLAN_OPER_DELETE:
                    vlan_nums = mv->num_of_vlan_id;
                    iRlt = TkCTC_DeletePortMulticastVlan(olt_id, onuid, (short int)port_id, vlan_nums, mv->vlan_id);
                    break;
                case CTC_MULTICAST_VLAN_OPER_CLEAR:
                    iRlt = TkCTC_ClearPortMulticastVlan(olt_id, onuid, (short int)port_id);
                    break;
                default:
                    iRlt = OLT_ERR_PARAM;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_ClearEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ClearPortMulticastVlan(olt_id, onuid, (short int)port_id);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ClearEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int *num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char max_number;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetPortMulticastGroupNumberMax(olt_id, onuid, (short int)port_id, &max_number)) )
            {
                *num = max_number;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetPortMulticastGroupNumberMax(olt_id, onuid, (short int)port_id, (unsigned char)num);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char strip_mode;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetPortMulticastTagStrip(olt_id, onuid, (short int)port_id, &strip_mode, NULL, NULL)) )
            {
                *strip = strip_mode;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortMulticastTagStrip(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *strip, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetPortMulticastTagStrip(olt_id, onuid, (short int)port_id, (unsigned char)strip, 0, NULL);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortMulticastTagStrip(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, strip, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char strip_mode;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            sw->number_of_entries = CTC_MAX_MULTICAST_VLAN_SWITCHING_ENTRIES;
            if ( 0 == (iRlt = TkCTC_GetPortMulticastTagStrip(olt_id, onuid, (short int)port_id, &strip_mode, &sw->number_of_entries, sw->entries)) )
            {
                *oper = strip_mode;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetEthPortMulticastTagOper( short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetPortMulticastTagStrip(olt_id, onuid, (short int)port_id, (unsigned char)oper, sw->number_of_entries, sw->entries);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_multicast_control(olt_id, onuid, mc);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_set_multicast_control(olt_id, onuid, mc);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_ClearMulticastControl(short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ClearOnuMulticastGroups(olt_id, onuid);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ClearMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t *sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char switch_mode;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuMulticastSwitchMode(olt_id, onuid, &switch_mode)) )
            {
                switch (switch_mode)
                {
                    case TkCtcOnuMulticastSwitchingModeIgmpMldSnooping:
                    case TkCtcOnuMulticastSwitchingModeIgmpSnooping:
                    case TkCtcOnuMulticastSwitchingModePassThrough:
                        *sw = CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING;
                        break;
                    case TkCtcOnuMulticastSwitchingModeCtcControllableIgmpMld:
                    case TkCtcOnuMulticastSwitchingModeCtcControllableIgmp:
                        *sw = CTC_STACK_PROTOCOL_CTC;
                        break;
                    default:
                        iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char switch_mode;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            switch ( sw )
            {
                case CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING:
                    switch_mode = TkCtcOnuMulticastSwitchingModeIgmpMldSnooping;
                    break;
                case CTC_STACK_PROTOCOL_CTC:
                    switch_mode = TkCtcOnuMulticastSwitchingModeCtcControllableIgmpMld;
                    break;
                default:
                    return OLT_ERR_PARAM;
            }
            
            iRlt = TkCTC_SetOnuMulticastSwitchMode(olt_id, onuid, switch_mode);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetFastLeaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char ability_bitmap;
    unsigned char ability_num;
    CTC_STACK_fast_leave_ability_mode_t *piAbility;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuFastLeaveAbility(olt_id, onuid, &ability_bitmap)) )
            {
                ability_num = 0;
            
                if ( TkCtcFastLeaveAbilityBitmapNone != ability_bitmap )
                {
                    piAbility = ability->modes;
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapNonFastLeaveInSnooping )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_IGMP_SNOOPING_MODE;
                        ability_num++;
                    }
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapFastLeaveInSnooping )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_IGMP_SNOOPING_MODE;
                        ability_num++;
                    }
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapNonFastLeaveInCtcControlled )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_MC_CONTROL_MODE;
                        ability_num++;
                    }
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapFastLeaveInCtcControlled )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MC_CONTROL_MODE;
                        ability_num++;
                    }
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapNonFastLeaveInMldSnooping )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_NON_FAST_LEAVE_IN_MLD_SNOOPING_MODE;
                        ability_num++;
                    }
                    
                    if ( ability_bitmap & TkCtcFastLeaveAbilityBitmapFastLeaveInMldSnooping )
                    {
                        *piAbility++ = CTC_STACK_FAST_LEAVE_ABILITY_ONU_SUPPORT_FAST_LEAVE_IN_MLD_SNOOPING_MODE;
                        ability_num++;
                    }
                }
            
                ability->num_of_modes = ability_num;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_Get_FastLeave_Ability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetFastLeaveAdminState(short int olt_id, short int onu_id, int *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char admin_state;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuFastLeaveState(olt_id, onuid, &admin_state)) )
            {
                *state = admin_state;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetFastLeaveAdminState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *state, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetFastLeaveAdminState(short int olt_id, short int onu_id, int state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned char admin_state;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            admin_state = (unsigned char)state;
            iRlt = TkCTC_SetOnuFastLeaveState(olt_id, onuid, admin_state);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetFastLeaveAdminState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_GetOnuPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    if( OnuSupportStatistics(olt_id, onu_id) == VOS_OK )	/* 问题单16117 */
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
            {
                iRlt = TkAdapter_PASCTC_get_ethport_statistic_data(olt_id, onuid, (short int)port_id, data);
                if (iRlt != 0)
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
        }
    }
    else
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOnuPortStaticData(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned short monitor_state;
    unsigned long  monitor_period;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(state);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetPortMonitorState(olt_id, onuid, (short int)port_id, &monitor_state, &monitor_period);

            if (0 == iRlt)
            {
                state->status = monitor_state;
                state->circle = monitor_period;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_GetOnuPortStaticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

static int CTCONU_TK3723_SetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    unsigned short monitor_state;
    unsigned long  monitor_period;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            monitor_state = state->status;
            monitor_period = state->circle;
            
            iRlt = TkCTC_SetPortMonitorState(olt_id, onuid, (short int)port_id, monitor_state, monitor_period);

            if (0 != iRlt)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"GWONU_TK3723_SetOnuPortStaticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

#if 0
/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_TK3723_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    #define CTC_MANAGEMENT_OBJECT_LEAF_NONE 0xFFFF
    #define TkCtc21OnuInstanceLabelPort 1
     #define TkCtc21OnuPortTypeEthernet 1
	 
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 1
            TkCtcOnuPortLabelBase ctc_obj;

            if ( 0 == (iRlt = TkCTC_GetTkObjFromCtcObj(olt_id, onuid, management_object, &ctc_obj)) )
            {/*add by yangzl@2016-9-13*/
            	  if(management_object->leaf == CTC_MANAGEMENT_OBJECT_LEAF_NONE)
            	  	{
            	  		ctc_obj.x.Ctc21.label = TkCtc21OnuInstanceLabelPort;
              	       ctc_obj.x.Ctc21.type = TkCtc21OnuPortTypeEthernet;
            	  	}
                iRlt = TkCTC_SetObjAlarmState(olt_id, onuid, &ctc_obj, alarm_id, enable ? TK_CTC_ENABLE : TK_CTC_DISABLE);
            }
#else
            TkCtc30EventStatusEntry alarm_state;

            if ( CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE == management_object_index.port_type )
            {
                alarm_state.eventEntry.objectType = TkCtcOnuObjectTypePonIf;
                alarm_state.eventEntry.objectInstance = 0;
            }
            else
            {
                alarm_state.eventEntry.objectType = TkCtcOnuObjectTypePort;
                alarm_state.eventEntry.objectInstance = management_object_index.port_number;
            }

            alarm_state.eventEntry.alarmID = (U16)alarm_id;
            alarm_state.eventStatus = enable;

            iRlt = TkCTC_SetOnuAlarmState(olt_id, onuid, 1, &alarm_state);
#endif
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetAlarmAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
#endif
int tk_opt_810h_debug = 0;
int CTCONU_TK3723_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object,
                                                                                                                 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

         if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
         {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkCtcOnuPortLabelBase ctc_obj;

            if ( 0 == (iRlt = TkCTC_GetTkObjFromCtcObj(olt_id, onuid, management_object, &ctc_obj)) )
            {
                                     switch( alarm_id )
                                     {
                                               case  ETH_PORT_LOOPBACK:
                                        ctc_obj.x.Ctc21.label = 1;/*TkCtc21OnuInstanceLabelPort;*/
                                   ctc_obj.x.Ctc21.type = 1;/*TkCtc21OnuPortTypeEthernet;*/
                                   iRlt = TkCTC_SetObjAlarmState(olt_id, onuid, &ctc_obj, alarm_id, enable ? TK_CTC_ENABLE : TK_CTC_DISABLE);
                                                        if(tk_opt_810h_debug) sys_console_printf("patch onu=%d-%d alarm-id=0x%04x %d ok\r\n", olt_id, onuid, alarm_id, enable);
                                                        break;
                                    case RX_POWER_HIGH_ALARM:
                                    case RX_POWER_LOW_ALARM:
                                    case TX_POWER_HIGH_ALARM:
                                    case TX_POWER_LOW_ALARM:
                                    case TX_BIAS_HIGH_ALARM: 
                                    case TX_BIAS_LOW_ALARM:
                                    case VCC_HIGH_ALARM:
                                    case VCC_LOW_ALARM: 
                                    case TEMP_HIGH_ALARM:
                                    case TEMP_LOW_ALARM:
								enable = 0; 
                                                        /*if( enable == 0 )*/
                                                        {
                                            iRlt = TkCTC_SetObjAlarmState(olt_id, onuid, &ctc_obj, alarm_id, TK_CTC_DISABLE);
                                                                 if(tk_opt_810h_debug) sys_console_printf("patch(%d-%d) onu=%d-%d alarm-id=0x%04x %d disable\r\n", 
                                                                           management_object->leaf, management_object->index.port_type,
                                                                           olt_id, onuid, alarm_id, enable);
                                                        }
                                                        break;
                                               default:
                                   iRlt = TkCTC_SetObjAlarmState(olt_id, onuid, &ctc_obj, alarm_id, enable ? TK_CTC_ENABLE : TK_CTC_DISABLE);
                                                        break;
                                     }
            }
              }
         }

    OLT_TK_DEBUG(OLT_TK_TITLE"patch_CTCONU_TK3723_SetAlarmAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

         return iRlt;
}

static int CTCONU_TK3723_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc30EventThresholdEntry tk_alarm_threshold;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 1
            TkCtcOnuPortLabelBase ctc_obj;
            unsigned long thresholds[2];

            if ( 0 == (iRlt = TkCTC_GetTkObjFromCtcObj(olt_id, onuid, management_object, &ctc_obj)) )
            {
                thresholds[0] = alarm_threshold;
                thresholds[1] = clear_threshold;
                
                iRlt = TkCTC_SetObjAlarmThreshold(olt_id, onuid, &ctc_obj, alarm_id, thresholds);
            }
#else
            if ( CTC_MANAGEMENT_OBJECT_PORT_TYPE_NONE == management_object_index.port_type )
            {
                tk_alarm_threshold.eventEntry.objectType = TkCtcOnuObjectTypePonIf;
                tk_alarm_threshold.eventEntry.objectInstance = 0;
            }
            else
            {
                tk_alarm_threshold.eventEntry.objectType = TkCtcOnuObjectTypePort;
                tk_alarm_threshold.eventEntry.objectInstance = management_object_index.port_number;
            }

            tk_alarm_threshold.eventEntry.alarmID = (U16)alarm_id;
            tk_alarm_threshold.setThreshold   = alarm_threshold;
            tk_alarm_threshold.clearThreshold = clear_threshold;

            iRlt = TkCTC_SetOnuAlarmThreshold(olt_id, onuid, 1, &tk_alarm_threshold);
#endif
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetAlarmThreshold(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
    
}

static int CTCONU_TK3723_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcDbaInfo dba_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
    	    if ( 0 == (iRlt = TkCTC_GetDBA_ReportThresholds(olt_id, onuid, &dba_info)) )
            {
                if ( 0 < (*number_of_queue_sets = dba_info.qSetsCount) )
                {
                    int i, n;
                    CTC_STACK_onu_queue_threshold_report_t *pstQueue;
                    TkCtcDbaQueueSet *pstQueSet;

                    n = *number_of_queue_sets;
                    pstQueue = queues_sets_thresholds->queue;
                    pstQueSet = dba_info.qSets;
                    for ( i = 0; i < n; i++ )
                    {
                        if ( 0 == pstQueSet->reported )
                        {
                            pstQueue->state = FALSE;
                        }
                        else
                        {
                            pstQueue->state = TRUE;
                            pstQueue->threshold = pstQueSet->thresholds[0];
                        }

                        pstQueue++;
                        pstQueSet++;
                    }
                }
            }   
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_TK3723_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtcDbaInfo dba_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            VOS_MemZero(&dba_info, sizeof(dba_info));
        
            if ( 0 < (dba_info.qSetsCount = *number_of_queue_sets) )
            {
                int i, n;
                CTC_STACK_onu_queue_threshold_report_t *pstQueue;
                TkCtcDbaQueueSet *pstQueSet;

                n = *number_of_queue_sets;
                pstQueue = queues_sets_thresholds->queue;
                pstQueSet = dba_info.qSets;
                for ( i = 0; i < n; i++ )
                {
                    if ( pstQueue->state )
                    {
                        pstQueSet->reported = 1;
                        pstQueSet->thresholds[0] = pstQueue->threshold;
                    }
                }
            }
        
    	    iRlt = TkCTC_SetDBA_ReportThresholds(olt_id, onuid, &dba_info);
    	}
	}
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}


static int CTCONU_TK3723_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetMxuGlobalParams(olt_id, onuid, (TkCtc21OnuMxuGlobalParameters*)mxu_mng);
    	}
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetMxuGlobalParams(olt_id, onuid, (TkCtc21OnuMxuGlobalParameters*)mxu_mng);
    	}
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetMxuMngSnmpConfig ( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21OnuMxuSnmpParameters tk_parameter;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetMxuSnmpParams(olt_id, onuid, &tk_parameter)) )
            {
                parameter->snmp_ver = tk_parameter.snmpVersion;
                parameter->trap_host_ip = tk_parameter.trapHostIP;
                parameter->trap_port = tk_parameter.trapPort;
                parameter->snmp_port = tk_parameter.snmpPort;

                VOS_MemCpy(parameter->security_name, tk_parameter.securityName, SECURITY_NAME_SIZE);
                VOS_MemCpy(parameter->community_for_read, tk_parameter.communityForRead, COMMUNITY_FOR_READ_SIZE);
                VOS_MemCpy(parameter->community_for_write, tk_parameter.communityForWrite, COMMUNITY_FOR_WRITE_SIZE);
            }
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_TK3723_SetMxuMngSnmpConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21OnuMxuSnmpParameters tk_parameter;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            tk_parameter.snmpVersion = parameter->snmp_ver;
            tk_parameter.trapHostIP = parameter->trap_host_ip;
            tk_parameter.trapPort = parameter->trap_port;
            tk_parameter.snmpPort = parameter->snmp_port;

            VOS_MemCpy(tk_parameter.securityName, parameter->security_name, SECURITY_NAME_SIZE);
            VOS_MemCpy(tk_parameter.communityForRead, parameter->community_for_read, COMMUNITY_FOR_READ_SIZE);
            VOS_MemCpy(tk_parameter.communityForWrite, parameter->community_for_write, COMMUNITY_FOR_WRITE_SIZE);
            
            iRlt = TkCTC_SetMxuSnmpParams(olt_id, onuid, &tk_parameter);
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int CTCONU_TK3723_GetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkOnuHoldoverParams hold_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetPonPortHoldover(olt_id, onuid, 0, &hold_info)) )
            {
                holdover->holdover_time = hold_info.holdoverInterval;
                
                if (  0 == hold_info.holdoverFlags  )
                {
                    holdover->holdover_state = CTC_STACK_HOLDOVER_STATE_ACTIVATED;
                }
                else
                {
                    holdover->holdover_state = CTC_STACK_HOLDOVER_STATE_DEACTIVATED;
                }
            }
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkOnuHoldoverParams hold_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            hold_info.holdoverInterval = holdover->holdover_time;
            
            if ( CTC_STACK_HOLDOVER_STATE_ACTIVATED == holdover->holdover_state )
            {
                hold_info.holdoverFlags = 0;
            }
            else
            {
                hold_info.holdoverFlags = 1;
            }
            
            iRlt = TkCTC_SetPonPortHoldover(olt_id, onuid, 0, &hold_info);
        }
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 1
	        iRlt = TkCTC_GetOpticalDiagInfo(olt_id, onuid, (TkCtc21OpticalTransceiverDiagnosis*)optical_transceiver_diagnosis);
#else
            iRlt = OLT_ERR_NOTSUPPORT;
#endif
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetOptTransDiag(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

/*modified by liyang @2015-05-18 for tk ctc set tx power supply control frame format error*/
static int CTCONU_TK3723_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
	int iRlt = OLT_ERR_NOTEXIST;
	short int llid, port_id,onuid;
	unsigned char version;
	TkCtcOnuTxPowerSupplyControl tk_tx_info;

	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	VOS_MemSet(&tk_tx_info,0,sizeof(tk_tx_info));

	{
		tk_tx_info.action = parameter->action;
		VOS_MemCpy(tk_tx_info.onuId,parameter->onu_sn, BYTES_IN_MAC_ADDRESS);
		tk_tx_info.opticalTxId = parameter->optical_id;
		port_id = (short int)TkChip_GetOltPonPortID(olt_id);   
		iRlt = TkCTC_SetPonPortTxPowerControl(olt_id, port_id, &tk_tx_info);
	}
	
	OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int CTCONU_TK3723_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t *fec_ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkFecAbilities tk_fec_cap;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = TkCTC_GetOnuFECAbility(olt_id, onuid, &tk_fec_cap)) )
            {
                if ( tk_fec_cap.upstreamFecSupported || tk_fec_cap.downstreamFecSupported )
                {
                    *fec_ability = STD_FEC_ABILITY_SUPPORTED;
                }
                else
                {
                    *fec_ability = STD_FEC_ABILITY_UNSUPPORTED;
                }
            }
    	}
	}

    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_GetFecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *fec_ability, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */


#if 1
int CTCONU_TK3723_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t *iad_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21IadInformation tk_iad_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(iad_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipIadInfo(olt_id, onuid, &tk_iad_info);
            if (0 == iRlt)
            {
                VOS_MemCpy(iad_info->mac_address, tk_iad_info.iadMac, BYTES_IN_MAC_ADDRESS);
                VOS_MemCpy(iad_info->sw_version, tk_iad_info.iadSoftwareVer, VOIP_SW_VERSION_SIZE);
                VOS_MemCpy(iad_info->sw_time, tk_iad_info.iadSoftwareTime, VOIP_SW_TIME_SIZE);
                iad_info->voip_protocol = tk_iad_info.protocolSupport;
                iad_info->user_count = tk_iad_info.voipUserCount;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetIADInfo:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

/*H.248协议下IAD的运行状态*/
int CTCONU_TK3723_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(iad_oper_status);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipH248IadStatus(olt_id, onuid, (TkCtc21IadOperationStatus*)iad_oper_status);
            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetVoipIadOperStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetVoipIadOperation(olt_id, onuid, (TkCtc21IadOperation)operation_type);
            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetVoipIadOperation:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

/*语音模块全局参数配置*/
int CTCONU_TK3723_GetVoipGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21VoipGlobalParameters tk_global_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(global_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipGlobalParams(olt_id, onuid, &tk_global_param);
            if (0 == iRlt)
            {
                VOS_MemCpy(global_param->pppoe_user, tk_global_param.pppoeUsername, VOIP_PPPOE_USER_SIZE);
                VOS_MemCpy(global_param->pppoe_passwd, tk_global_param.pppoePassword, VOIP_PPPOE_PASSWD_SIZE);
                
                global_param->voice_ip_mode = tk_global_param.voiceIPMode;
                global_param->pppoe_mode = tk_global_param.pppoeMode;
                global_param->tag_flag = tk_global_param.taggingMode;
                global_param->priority = tk_global_param.voicePri;
                
                global_param->cvlan_id = tk_global_param.voiceCvlan;
                global_param->svlan_id = tk_global_param.voiceSvlan;
                
                global_param->iad_ip_addr = tk_global_param.iadIPAddr;
                global_param->iad_net_mask = tk_global_param.iadNetMask;
                global_param->iad_def_gw = tk_global_param.iadDefaultGW;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetVoipGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetVoipGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21VoipGlobalParameters tk_global_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(global_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipGlobalParams(olt_id, onuid, &tk_global_param);
            if ( 0 == iRlt )
            {
                switch(code)
                {
                    case voice_global_config_ip_mode:
                        tk_global_param.voiceIPMode = (U8)global_param->voice_ip_mode;
                        break;
                    case voice_global_config_ip_addr:
                        tk_global_param.iadIPAddr = global_param->iad_ip_addr;
                        tk_global_param.iadNetMask = global_param->iad_net_mask;
                        break;
                    case voice_global_config_ip_gw:
                        tk_global_param.iadDefaultGW = global_param->iad_def_gw;
                        break;
                    case voice_global_config_pppoe_mode:
                        tk_global_param.pppoeMode = (U8)global_param->pppoe_mode;
                        break;
                    case voice_global_config_pppoe_username:
                        VOS_MemCpy(tk_global_param.pppoeUsername, global_param->pppoe_user, VOIP_PPPOE_USER_SIZE);
                        break;
                    case voice_global_config_pppoe_password:
                        VOS_MemCpy(tk_global_param.pppoeUsername, global_param->pppoe_passwd, VOIP_PPPOE_PASSWD_SIZE);
                        break;
                    case voice_global_config_tagged_mode:
                        tk_global_param.taggingMode = (U8)global_param->tag_flag;
                        break;
                    case voice_global_config_vlan:
                        tk_global_param.voiceCvlan = global_param->cvlan_id;
                        tk_global_param.taggingMode = CTC_STACK_VOIP_TAGGED_FLAG_TAG;                    
                        break;
                    case voice_global_config_priority:
                        tk_global_param.voicePri = (U8)global_param->priority;
                        break;
                    default:
                        VOS_MemCpy(tk_global_param.pppoeUsername, global_param->pppoe_user, VOIP_PPPOE_USER_SIZE);
                        VOS_MemCpy(tk_global_param.pppoePassword, global_param->pppoe_passwd, VOIP_PPPOE_PASSWD_SIZE);
                        
                        tk_global_param.voiceIPMode = global_param->voice_ip_mode;
                        tk_global_param.pppoeMode = global_param->pppoe_mode;
                        tk_global_param.taggingMode = global_param->tag_flag;
                        tk_global_param.voicePri = global_param->priority;
                        
                        tk_global_param.voiceCvlan = global_param->cvlan_id;
                        tk_global_param.voiceSvlan = global_param->svlan_id;
                        
                        tk_global_param.iadIPAddr = global_param->iad_ip_addr;
                        tk_global_param.iadNetMask = global_param->iad_net_mask;
                        tk_global_param.iadDefaultGW = global_param->iad_def_gw;
                        break;
                }

                iRlt = TkCTC_SetVoipGlobalParams(olt_id, llid, &tk_global_param);
                if ( iRlt != 0 )
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetVoipGlobalParameter:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21FaxModemConfiguration tk_fax_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(voip_fax);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetFoipGlobalParams(olt_id, onuid, &tk_fax_param);
            if ( 0 == iRlt )
            {
                voip_fax->t38_enable  = tk_fax_param.voiceT38Enable;
                voip_fax->fax_control = tk_fax_param.voiceFaxModemControl;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21FaxModemConfiguration tk_fax_param;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(voip_fax);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetFoipGlobalParams(olt_id, onuid, &tk_fax_param);
            if ( 0 == iRlt )
            {
                switch(code)
                {
                    case voice_fax_config_t38_enable:
                        tk_fax_param.voiceT38Enable = (U8)voip_fax->t38_enable;
                        break;
                    case voice_fax_config_control:
                        tk_fax_param.voiceFaxModemControl = (U8)voip_fax->fax_control;
                        break;
                    default:
                        tk_fax_param.voiceT38Enable = (U8)voip_fax->t38_enable;
                        tk_fax_param.voiceFaxModemControl = (U8)voip_fax->fax_control;
                        break;
                }
                iRlt = TkCTC_SetFoipGlobalParams(olt_id, onuid, &tk_fax_param);
                if (iRlt != 0)
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    
    return iRlt;           
}


int CTCONU_TK3723_GetVoipPortStatus(short int olt_id, short int onu_id, int port_id, CTC_STACK_voip_pots_status_array *pots_status_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipPortStatus(olt_id, onuid, (short int)port_id, (TkCtc21PotsStatus*)&pots_status_array->pots_status_array[0].pots_status);
            if (0 == iRlt)
            {
                pots_status_array->number_of_entries = 1;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetVoipPortStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    bool tk_port_state;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipPortState(olt_id, onuid, port_id, &tk_port_state);
            if ( 0 == iRlt )
            {
                *port_state = tk_port_state;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);

    return iRlt;   
}

int CTCONU_TK3723_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( CTC_STACK_STATE_DEACTIVATE == port_state )
            {
                iRlt = TkCTC_DisablePort(olt_id, onuid, port_id);
            }
            else
            {
                iRlt = TkCTC_EnablePort(olt_id, onuid, port_id);
            }
            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int CTCONU_TK3723_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21H248Parameters tk_h248_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipH248Params(olt_id, onuid, &tk_h248_param);
            if (0 == iRlt)
            {
                h248_param->mg_port = tk_h248_param.mGPortNumber;
                h248_param->mgcip = tk_h248_param.mgcIP;
                h248_param->mgccom_port_num = tk_h248_param.mgcComPort;
                h248_param->back_mgcip = tk_h248_param.backupMgcIP;
                h248_param->back_mgccom_port_num = tk_h248_param.backupMgcComPort;
                h248_param->active_mgc = tk_h248_param.useActiveMgc;
                h248_param->reg_mode = tk_h248_param.regMode;
                h248_param->heart_beat_mode = tk_h248_param.heartbeatMode;
                h248_param->heart_beat_cycle = tk_h248_param.heartbeatCycle;
                h248_param->heart_beat_count = tk_h248_param.heartbeatCount;

                VOS_MemCpy(h248_param->mid, tk_h248_param.mid, H248_MID_SIZE);
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21H248Parameters tk_h248_param;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipH248Params(olt_id, onuid, &tk_h248_param);
            if ( 0 == iRlt )
            {
                switch(code)
                {
                    case h248_config_local_port:
                        tk_h248_param.mGPortNumber = h248_param->mg_port;
                        break;
                    case h248_config_mgc_addr:
                        tk_h248_param.mgcIP = h248_param->mgcip;
                        break;
                    case h248_config_mgc_port:
                        tk_h248_param.mgcComPort = h248_param->mgccom_port_num;
                        break;
                    case h248_config_mgc_backup_addr:
                        tk_h248_param.backupMgcIP = h248_param->back_mgcip;
                        break;
                    case h248_config_mgc_backup_port:
                        tk_h248_param.backupMgcComPort = h248_param->back_mgccom_port_num;
                        break;
                    case h248_config_regiser_mode:
                        tk_h248_param.regMode = h248_param->reg_mode;
                        break;
                    case h248_config_local_domain:
                        VOS_MemCpy(tk_h248_param.mid, h248_param->mid, H248_MID_SIZE);
                        break;
                    case h248_config_heartbeat_enable:
                        tk_h248_param.heartbeatMode = h248_param->heart_beat_mode;
                        break;
                    case h248_config_heartbeat_time:
                        tk_h248_param.heartbeatCycle = h248_param->heart_beat_cycle;
                        break;            
                    default:
                        tk_h248_param.mGPortNumber = h248_param->mg_port;
                        tk_h248_param.mgcIP = h248_param->mgcip;
                        tk_h248_param.mgcComPort = h248_param->mgccom_port_num;
                        tk_h248_param.backupMgcIP = h248_param->back_mgcip;
                        tk_h248_param.backupMgcComPort = h248_param->back_mgccom_port_num;
                        tk_h248_param.useActiveMgc = h248_param->active_mgc;
                        tk_h248_param.regMode = h248_param->reg_mode;
                        tk_h248_param.heartbeatMode = h248_param->heart_beat_mode;
                        tk_h248_param.heartbeatCycle = h248_param->heart_beat_cycle;
                        tk_h248_param.heartbeatCount = h248_param->heart_beat_count;

                        VOS_MemCpy(tk_h248_param.mid, h248_param->mid, H248_MID_SIZE);
                        break;
                }
                
                iRlt = TkCTC_SetVoipH248Params(olt_id, onuid, &tk_h248_param);
                if ( iRlt != 0 )
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_GetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipPortH248Tid(olt_id, onuid, (short int)port_id, h248_user_tid_array->h248_user_tid_array[0].h248_user_tid_config.user_tid_name);
            if (0 == iRlt)
            {
                h248_user_tid_array->number_of_entries = 1;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_t *user_tid_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetVoipPortH248Tid(olt_id, onuid, (short int)port_id, user_tid_config->user_tid_name);
            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_rtp_tid_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipH248RtpTid(olt_id, onuid, (TkCtc21H248RtpTidInformation*)h248_rtp_tid_info);
            if ( 0 != iRlt )
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21H248RtpTidConfiguration tk_rtp_tid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_rtp_tid_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            tk_rtp_tid.numRtpTids = h248_rtp_tid_info->num_of_rtp_tid;
            tk_rtp_tid.rtpTidMode = (U8)h248_rtp_tid_info->rtp_tid_mode;
            tk_rtp_tid.rtpTidLength = h248_rtp_tid_info->rtp_tid_digit_length;
            
            VOS_MemCpy(tk_rtp_tid.prefix, h248_rtp_tid_info->rtp_tid_prefix, H248_RTP_TID_PREFIX_SIZE);
            VOS_MemCpy(&tk_rtp_tid.digitBegin, h248_rtp_tid_info->rtp_tid_digit_begin, H248_RTP_TID_DIGIT_SIZE);
        
            iRlt = TkCTC_SetVoipH248RtpTid(olt_id, onuid, &tk_rtp_tid);
            if ( iRlt != 0 )
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}


int CTCONU_TK3723_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21SipParameters tk_sip_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipSipParams(olt_id, onuid, &tk_sip_param);
            if ( 0 == iRlt )
            {
                sip_param->mg_port = tk_sip_param.mGPortNumber;
                sip_param->server_ip = tk_sip_param.activeSipIP;
                sip_param->serv_com_port = tk_sip_param.activeSipComPort;
                sip_param->back_server_ip = tk_sip_param.backupSipIP;
                sip_param->back_serv_com_port = tk_sip_param.backupSipComPort;
                sip_param->active_proxy_server = tk_sip_param.activeSipProxyServer;
                sip_param->reg_server_ip = tk_sip_param.activeSipRegistrationServerIP;
                sip_param->reg_serv_com_port = tk_sip_param.activeSipRegistrationServerComPort;
                sip_param->back_reg_server_ip = tk_sip_param.backupSipRegIP;
                sip_param->back_reg_serv_com_port = tk_sip_param.backupSipRegComPort;
                sip_param->outbound_server_ip = tk_sip_param.outboundServerIP;
                sip_param->outbound_serv_com_port = tk_sip_param.outboundServerPort;
                sip_param->reg_interval = tk_sip_param.sipRegistrationInterval;
                sip_param->heart_beat_switch = tk_sip_param.disableHeartbeatSwitch;
                sip_param->heart_beat_cycle = tk_sip_param.heartbeatCycle;
                sip_param->heart_beat_count = tk_sip_param.heartbeatCount;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21SipParameters tk_sip_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipSipParams(olt_id, onuid, &tk_sip_param);
            if ( 0 == iRlt )
            {
                switch(code)
                {
                    case sip_config_local_port:
                        tk_sip_param.mGPortNumber = sip_param->mg_port;
                        break;
                    case sip_config_server_ip:
                        tk_sip_param.activeSipIP = sip_param->server_ip;
                        break;
                    case sip_config_server_port:
                        tk_sip_param.activeSipComPort = sip_param->serv_com_port;
                        break;
                    case sip_config_server_backup_ip:
                        tk_sip_param.backupSipIP = sip_param->back_server_ip;
                        break;
                    case sip_config_server_backup_port:
                        tk_sip_param.backupSipComPort = sip_param->back_serv_com_port;
                        break;
                    case sip_config_reg_server_ip:
                        tk_sip_param.activeSipRegistrationServerIP = sip_param->reg_server_ip;
                        break;
                    case sip_config_reg_server_port:
                        tk_sip_param.activeSipRegistrationServerComPort = sip_param->reg_serv_com_port;
                        break;
                    case sip_config_reg_server_backup_ip:
                        tk_sip_param.backupSipRegIP = sip_param->back_reg_server_ip;
                        break;
                    case sip_config_reg_server_backup_port:
                        tk_sip_param.backupSipRegComPort = sip_param->back_reg_serv_com_port;
                        break;
                    case sip_config_outbound_server_ip:
                        tk_sip_param.outboundServerIP = sip_param->outbound_server_ip;
                        break;
                    case sip_config_outbound_server_port:
                        tk_sip_param.outboundServerPort = sip_param->outbound_serv_com_port;
                        break;
                    case sip_config_reg_interval:
                        tk_sip_param.sipRegistrationInterval = sip_param->reg_interval;
                       break;
                    case sip_config_heartbeat_enable:
                        tk_sip_param.disableHeartbeatSwitch = sip_param->heart_beat_switch;
                        break;
                    case sip_config_heartbeat_time:
                        tk_sip_param.heartbeatCycle = sip_param->heart_beat_cycle;
                        break;
                    default:
                        tk_sip_param.mGPortNumber = sip_param->mg_port;
                        tk_sip_param.activeSipIP = sip_param->server_ip;
                        tk_sip_param.activeSipComPort = sip_param->serv_com_port;
                        tk_sip_param.backupSipIP = sip_param->back_server_ip;
                        tk_sip_param.backupSipComPort = sip_param->back_serv_com_port;
                        tk_sip_param.activeSipProxyServer = sip_param->active_proxy_server;
                        tk_sip_param.activeSipRegistrationServerIP = sip_param->reg_server_ip;
                        tk_sip_param.activeSipRegistrationServerComPort = sip_param->reg_serv_com_port;
                        tk_sip_param.backupSipRegIP = sip_param->back_reg_server_ip;
                        tk_sip_param.backupSipRegComPort = sip_param->back_reg_serv_com_port;
                        tk_sip_param.outboundServerIP = sip_param->outbound_server_ip;
                        tk_sip_param.outboundServerPort = sip_param->outbound_serv_com_port;
                        tk_sip_param.sipRegistrationInterval = sip_param->reg_interval;
                        tk_sip_param.disableHeartbeatSwitch = sip_param->heart_beat_switch;
                        tk_sip_param.heartbeatCycle = sip_param->heart_beat_cycle;
                        tk_sip_param.heartbeatCount = sip_param->heart_beat_count;
                        break;
                }
                iRlt = TkCTC_SetVoipSipParams(olt_id, onuid, &tk_sip_param);
                if (iRlt != 0)
                {
                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    
    return iRlt;           
}

int CTCONU_TK3723_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_digit_map);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_SetVoipSipDigitMap(olt_id, onuid, (unsigned short)sip_digit_map->digital_map_length, sip_digit_map->digital_map);

            if ( iRlt != 0 )
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetSipDigitMap:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}


int CTCONU_TK3723_GetSipUserConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipPortSipUserParams(olt_id, onuid, (short int)port_id, (TkCtc21SipUserParameters*)&sip_user_param_array->sip_user_param_array[0].sip_user_param_config);
            if (0 == iRlt)
            {
                sip_user_param_array->number_of_entries = 1;
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_GetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_TK3723_SetSipUserConfig(short int olt_id, short int onu_id, int port_id, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    TkCtc21SipUserParameters sip_user_config;    
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_GetVoipPortSipUserParams(olt_id, onuid, (short int)port_id, &sip_user_config);
            if (0 == iRlt)
            {
                switch(code)
                {
                    case sip_user_config_account:
                        VOS_MemCpy(sip_user_config.account, sip_user_param->sip_port_num, SIP_PORT_NUM_SIZE);
                        break;
                    case sip_user_config_username:
                        VOS_MemCpy(sip_user_config.name, sip_user_param->user_name, SIP_USER_NAME_SIZE);                
                        break;
                    case sip_user_config_password:
                        VOS_MemCpy(sip_user_config.password, sip_user_param->passwd, SIP_PASSWD_SIZE);                
                        break;                
                    default:
                        VOS_MemCpy(sip_user_config.account, sip_user_param->sip_port_num, SIP_PORT_NUM_SIZE);
                        VOS_MemCpy(sip_user_config.name, sip_user_param->user_name, SIP_USER_NAME_SIZE);                
                        VOS_MemCpy(sip_user_config.password, sip_user_param->passwd, SIP_PASSWD_SIZE);                
                }
            }
            else
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

            iRlt = TkCTC_SetVoipPortSipUserParams(olt_id, onuid, (short int)port_id, &sip_user_config);
            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_TK3723_SetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}
#endif

#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

static int CTCONU_TK3723_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0;
    short int llid = 0, onuid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char version = 0;
            int portnum = getOnuEthPortNum(olt_id, onu_id);                    
            CTC_STACK_port_vlan_configuration_t pvc;
            OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
            
            for(i=0;i<portnum;i++)
            {
                VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                if(version >= CTC_2_1_ONU_VERSION)
                {
                    pvc.mode = enable?ONU_CONF_VLAN_MODE_TRUNK:ONU_CONF_VLAN_MODE_TRANSPARENT;
            
                    pvc.default_vlan = (0x8100<<16)|1;
                    pvc.number_of_entries = 0;
                }
                else
                {
                    pvc.mode = enable?ONU_CONF_VLAN_MODE_TAG:ONU_CONF_VLAN_MODE_TRANSPARENT;
            
                    pvc.vlan_list[0] = (0x8100<<16)|1;
                    pvc.number_of_entries = 1;
                }
                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                if(iRlt != VOS_OK)
                {
                    lReturnValue = iRlt;                    				                                
                    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
                }
            }                    
        }   
        else
        {
            lReturnValue = VOS_ERROR;                                                                    
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                    
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetVlanEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_TK3723_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    short int llid = 0, onuid = 0;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    mode &= (~ONU_SEPCAL_FUNCTION);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            if ( 0 == iRlt )
            {
            	/*del by luh@2015-10-13*/
                /*if ( pvc.mode != mode )*/
                {
                    pvc.number_of_aggregation_tables = 0;
                    pvc.number_of_entries = 0;
                    pvc.mode = mode;
                    pvc.default_vlan = (0x8100<<16)|1;

    				if(pvc.mode == ONU_CONF_VLAN_MODE_TAG)
    				{
    					pvc.number_of_entries = 1;
    					pvc.vlan_list[0] = 1;
    				}

    				if(pvc.mode == ONU_CONF_VLAN_MODE_AGG)
    				{
    				    pvc.number_of_aggregation_tables = 1;
    				    pvc.vlan_aggregation_tables[0].number_of_aggregation_entries = 0;
    				    pvc.vlan_aggregation_tables[0].target_vlan = 0;
    				}

                    iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
                    if(iRlt != VOS_OK)
                    {
                        lReturnValue = iRlt;                                                                        
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);         
                    }
                }
            }
            else
            {
                lReturnValue = iRlt;                                                                
                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);                     
            }
        }
        else
        {
            lReturnValue = VOS_ERROR;                                                                
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);                     
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);                     
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_TK3723_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    ULONG all = 0, untag = 0;
    short int llid = 0, onuid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char version = 0;
            OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
                    
            if ( 0 == (iRlt = get_onuconf_vlanPortlist(olt_id, onu_id, vid,  &all, &untag)) )
            {

#if 0 /*deleted by liyang @2015-06-03 for get all ethport vlan by one cmd */
                if ( 0 == (iRlt = TkAdapter_PASCTC_get_all_ethport_vlan_config(olt_id, onuid, &portNum, Port_Info)) )
                {
                    for ( i=0; i<portnum; i++ )
                    {
                        num = Port_Info[i].entry.number_of_entries;
                        if(version >= CTC_2_1_ONU_VERSION)
                        {                        
                            for ( j=0; j<num; j++ )
                            {
                                if ( (Port_Info[i].entry.vlan_list[j] & 0xfff) == vid )
                                {
                                    linux_memmove(Port_Info[i].entry.vlan_list+j, Port_Info[i].entry.vlan_list+j+1, sizeof(long)*(num-1-j));
                                    Port_Info[i].entry.number_of_entries--;
                                }
                            }
                            
                            /*端口默认vlan恢复为 1 */
                            if ( untag & (1<<i) )
                            {
                                /* pvc.default_vlan &= 0xfffff000;*/
                                /* pvc.default_vlan |= 1; */
                                Port_Info[i].entry.default_vlan = (0x8100<<16)|1;
                            }
                        }
                        else
                        {
                            if((Port_Info[i].entry.vlan_list[0]&0xfff) == vid)
                            {
                                Port_Info[i].entry.vlan_list[0] = 1;
                            }
                            else
                            {
                                continue;
                            }                        
                        }
                        iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &Port_Info[i].entry);
                        if(iRlt != VOS_OK)
                        {
                            lReturnValue = iRlt;                                                                            
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                        }
                    }
                }
                else
#endif
                {
                    CTC_STACK_port_vlan_configuration_t pvc;                
                    for(i=0;i<portnum;i++)
                    {
                        VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                        iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                        if(iRlt == VOS_OK)
                        {
                            int num = pvc.number_of_entries;                    
                            if(version >= CTC_2_1_ONU_VERSION)
                            {
                            	if(pvc.mode == CTC_VLAN_MODE_TRUNK)
                        		{
	                                for(j=0; j<num; j++)
	                                {
	                                    if((pvc.vlan_list[j]&0xfff) == vid)
	                                    {
	                                        linux_memmove(pvc.vlan_list+j, pvc.vlan_list+j+1, sizeof(long)*(num-1-j));
	                                        pvc.number_of_entries--;
	                                    }
	                                }
	                                /*端口默认vlan恢复为 1 */
	                                if(untag & (1<<i))
	                                {
	                                    pvc.default_vlan = (0x8100<<16)|1;
	                                }
                        		}
								else if(pvc.mode == CTC_VLAN_MODE_TAG)
								{
	                                if((pvc.vlan_list[0]&0xfff) == vid)
	                                {
	                                    pvc.vlan_list[0] = 1;
	                                }
	                                else
	                                {
	                                    continue;
	                                }								
								}
								else
								{
									continue;
								}
                            }
                            else
                            {
                                if((pvc.vlan_list[0]&0xfff) == vid)
                                {
                                    pvc.vlan_list[0] = 1;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                            if(iRlt != VOS_OK)
                            {
                                lReturnValue = iRlt;                                                                                
                                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                            }
                        }
                        else
                        {
                            lReturnValue = iRlt;                                                                            
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                                 
                        }
                    }
                
                }
            }
        }
        else
        {
            lReturnValue = VOS_ERROR;                                                                            
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                                 
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                            
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                                 
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_DelVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_TK3723_SetPortPvid(short int olt_id, short int onu_id, int port_id, int lv)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    short int llid = 0, onuid = 0;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            if ( 0 == iRlt )
            {
                unsigned char version = 0;
                OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
                if(version >= CTC_2_1_ONU_VERSION)                
                {            
        		 	if(pvc.mode != ONU_CONF_VLAN_MODE_TAG)
        		 	{
        			    pvc.default_vlan &= 0xf000;
        			    pvc.default_vlan |= lv;
        		 	}
        			else
        			{
        				pvc.vlan_list[0] = lv;
        				pvc.number_of_entries = 1;
        			}
                    iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
                    if(iRlt != VOS_OK)
                    {
                        lReturnValue = iRlt;                                                                                                
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, lv, __LINE__);         
                    }
                }
                else
                {
                    iRlt = OLT_ERR_OK;
                }
            }
            else
            {
                lReturnValue = iRlt;                                                                                        
                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, lv, __LINE__);                     
            }
        }
        else
        {
            lReturnValue = VOS_ERROR;                                                                                        
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, port_id, lv, __LINE__);                     
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                                        
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, port_id, lv, __LINE__);                         
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortPvid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_TK3723_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    short int llid = 0, onuid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char version = 0;
            OnuGen_Get_CtcVersion(olt_id, onu_id, &version);

#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_all_ethport_vlan_config(olt_id, onuid, &portNum, Port_Info)) )
            {
                for ( i=0; i<portnum; i++ )
                {
                    if(portlist&(1<<i))
                    {
                        num = Port_Info[i].entry.number_of_entries;
                        if(version >= CTC_2_1_ONU_VERSION)
                        {
                            /*untag port member added by wangxiaoyu 2011-08-10, auto change port pvid to vid*/
                            if(tag == 2)
                            {
                                /* pvc.default_vlan &= 0xfffff000; pvc.default_vlan |= vid;*/
                                Port_Info[i].entry.default_vlan = (0x8100<<16)|vid;

                                /*如果已加为tag的，从tag中删除*/
                                for(j=0; j<num; j++)
                                {
                                    if((Port_Info[i].entry.vlan_list[j]&0xfff) == vid)
                                    {
                                        linux_memmove(&Port_Info[i].entry.vlan_list[j], &Port_Info[i].entry.vlan_list[j+1], (num-j-1)*sizeof(ULONG));
                                        Port_Info[i].entry.number_of_entries--;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                if(vid == (Port_Info[i].entry.default_vlan&0xfff))
                                    Port_Info[i].entry.default_vlan = (0x8100<<16)|1;

                                for(j=0; j<num; j++)
                                {
                                     if((Port_Info[i].entry.vlan_list[j]&0xfff) == vid)
                                         break;
                                }

                                if(j == num)
                                {
                                    Port_Info[i].entry.vlan_list[num] = vid;
                                    Port_Info[i].entry.number_of_entries++;
                                }
                            }
                        }
                        else
                        {
                            int iv = 0;
                            if(tag == 2)
                            {
                                Port_Info[i].entry.vlan_list[0] = vid;
                                Port_Info[i].entry.number_of_entries = 1;
                            }
                            else
                            {
                                return OLT_ERR_OK;
                            }
                        
                        }
                        iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &Port_Info[i].entry);
                        if(iRlt != VOS_OK)
                        {
                            lReturnValue = iRlt;                                                                                                    
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                        }
                    }
                }
            }
            else
#endif 
            {
                CTC_STACK_port_vlan_configuration_t pvc;
                
                for(i=0;i<portnum;i++)
                {
                    if(!(portlist&(1<<i))) 
                        continue;                           
                    VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                    iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                    if(iRlt == VOS_OK)
                    {
                        int num = pvc.number_of_entries;
                        if(version >= CTC_2_1_ONU_VERSION)
                        {
                        	if(pvc.mode == CTC_VLAN_MODE_TRUNK)
                    		{
	                            if(tag == 2)
	                            {
	                                int iv = 0;
	                                /* pvc.default_vlan &= 0xfffff000; pvc.default_vlan |= vid;*/
	                                pvc.default_vlan = (0x8100<<16)|vid;
	                                /*如果已加为tag的，从tag中删除*/
	                                for(iv=0; iv<num; iv++)
	                                {
	                                    if((pvc.vlan_list[iv]&0xfff) == vid)
	                                    {
	                                        linux_memmove(&pvc.vlan_list[iv], &pvc.vlan_list[iv+1], (num-iv-1)*sizeof(ULONG));
	                                        pvc.number_of_entries--;
	                                        break;
	                                    }
	                                }
	                            }
	                            else
	                            {
	                                int iv = 0;
	                                if(vid == (pvc.default_vlan&0xfff))
	                                    pvc.default_vlan = (0x8100<<16)|1;
	                
	                                for(iv=0; iv<num; iv++)
	                                {
	                                     if((pvc.vlan_list[iv]&0xfff) == vid)
	                                         break;
	                                }
	                
	                                if(iv == num)
	                                {
	                                    pvc.vlan_list[num] = vid;
	                                    pvc.number_of_entries++;
	                                }
	                            }
                    		}
							else if(pvc.mode == CTC_VLAN_MODE_TAG)
							{
	                            if(tag == 2)
	                            {
	                                pvc.vlan_list[0] = vid;
	                                pvc.number_of_entries = 1;
	                            }
	                            else
	                            {
	                                return VOS_OK;
	                            }							
							}
							else
							{
								continue;
							}
                        }
                        else
                        {
                            int iv = 0;
                            if(tag == 2)
                            {
                                pvc.vlan_list[0] = vid;
                                pvc.number_of_entries = 1;
                            }
                            else
                            {
                                return VOS_OK;
                            }
                        }
                        iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                        if(iRlt != VOS_OK)
                        {
                            lReturnValue = iRlt;                                                                                                    
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                        }
                    }
                    else
                    {
                        lReturnValue = iRlt;                                                                                                
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                    }
                }
            
            }
        }
        else
        {
            lReturnValue = VOS_ERROR;                                                                                                
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                                                
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);             
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_AddVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_TK3723_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    short int llid = 0, onuid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char version = 0;
            OnuGen_Get_CtcVersion(olt_id, onu_id, &version);

#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_all_ethport_vlan_config(olt_id, onuid, &portNum, Port_Info)) )
            {
                for ( i=0; i<portnum; i++ )
                {
                    if(portlist&(1<<i))
                    {
                        num = Port_Info[i].entry.number_of_entries;
                        if(version >= CTC_2_1_ONU_VERSION)
                        {                        
                            for(j=0; j<num; j++)
                            {
                                if((Port_Info[i].entry.vlan_list[j]&0xfff) == vid)
                                {
                                    linux_memmove(Port_Info[i].entry.vlan_list+j, Port_Info[i].entry.vlan_list+j+1,sizeof(long)*(num-1-j));
                                    Port_Info[i].entry.number_of_entries--;
                                    break;
                                }
                            }
                            /*added by wangxiaoyu 2009-08-10*/
                            /* auto change port pvid to vid 1 when the port default vid eq 'vid'*/
                            if((Port_Info[i].entry.default_vlan & 0xfff) == vid)
                            {
        						/* pvc.default_vlan &= 0xfffff000;pvc.default_vlan |= 1; */
                                Port_Info[i].entry.default_vlan = (0x8100<<16)|1;
                            }
                        }
                        else
                        {
                            if((Port_Info[i].entry.vlan_list[0] & 0xfff) == vid)
                            {
                                Port_Info[i].entry.vlan_list[0] = 1;
                            }
                            else
                            {
                                continue;
                            }                        
                        }
                        iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &Port_Info[i].entry);
                        if(iRlt != VOS_OK)
                        {
                            lReturnValue = iRlt;                                                                                                                        
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                        }
                    }
                }
            }
            else
#endif
            {
                CTC_STACK_port_vlan_configuration_t pvc;            
                for(i=0;i<portnum;i++)
                {
                    if (!(portlist&(1<<i))) 
                        continue;
                        
                    VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                    iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                    if(iRlt == VOS_OK)
                    {
                        int num = pvc.number_of_entries;
                        if(version >= CTC_2_1_ONU_VERSION)
                        {
                        	if(pvc.mode == CTC_VLAN_MODE_TRUNK)
                    		{
	                            for(j=0; j<num; j++)
	                            {
	                                if((pvc.vlan_list[j]&0xfff) == vid)
	                                {
	                                    linux_memmove(pvc.vlan_list+j, pvc.vlan_list+j+1,sizeof(long)*(num-1-j));
	                                    pvc.number_of_entries--;
	                                    break;
	                                }
	                            }
	                            
	                            /* auto change port pvid to vid 1 when the port default vid eq 'vid'*/
	                            if((pvc.default_vlan & 0xfff) == vid)
	                            {
	                                pvc.default_vlan = (0x8100<<16)|1;
	                            }
                    		}
							else if(pvc.mode == CTC_VLAN_MODE_TAG)
							{
	                            if((pvc.vlan_list[0] & 0xfff) == vid)
	                            {
	                                pvc.vlan_list[0] = 1;
	                            }
	                            else
	                            {
	                                continue;
	                            }							
							}
							else
							{
								continue;
							}
                        }
                        else
                        {
                            if((pvc.vlan_list[0] & 0xfff) == vid)
                            {
                                pvc.vlan_list[0] = 1;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, i+1, &pvc);
                        if(iRlt != VOS_OK)
                        {
                            lReturnValue = iRlt;                                                                                                                        
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                        }
                    }
                    else
                    {
                        lReturnValue = iRlt;                                                                                                                    
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                             
                    }
                }
            
            }
        }
        else
        {
            lReturnValue = VOS_ERROR;                                                                                                                    
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                             
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                                                                                                    
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                                 
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_DelVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return lReturnValue;
}

/*vlan transation*/
static int CTCONU_TK3723_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i;
    short int llid, onuid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc)) )
#else
            if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, (USHORT)port_id, &pvc)) )
#endif
            {
                pvc.mode = CTC_VLAN_MODE_TRANSLATION;

                for (i = 0; i < ONU_MAX_VLAN_TRANS; i++)
                {
                    if (pvc.vlan_list[2*i] == 0)
                    {
                        pvc.vlan_list[2 * i] = inVid;
                        pvc.vlan_list[2 * i + 1] = newVid;
                        sys_console_printf("GWONU_SET_VlanTransation invid=%d,newvid=%d\r\n", inVid, newVid);
                        pvc.number_of_entries++;
                        break;
                    }
                }

                if (pvc.number_of_entries == ONU_MAX_VLAN_TRANS)
                    sys_console_printf("the VlanTransation number is 8,out of range!\r\n");
                /*pvc.number_of_entries++;*/

                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, num;
    short int llid, onuid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc)) )
#else
            if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, (USHORT)port_id, &pvc)) )
#endif
            {
                pvc.mode = CTC_VLAN_MODE_TRANSLATION;
                num = pvc.number_of_entries;

                for (i = 0; i < num/*ONU_MAX_VLAN_TRANS*/; i++)
                {
                    if (pvc.vlan_list[2 * i] == inVid)
                    {
                        linux_memmove(&pvc.vlan_list[2*i], &pvc.vlan_list[2*(i+1)], sizeof(ULONG)*2*(num-i-1));
#if 0
                        pvc.vlan_list[2 * i] = 0;
                        pvc.vlan_list[2 * i + 1] = 0;
#endif
                        sys_console_printf("GWONU_DEL_VlanTransation invid=%d\r\n", inVid);
                        pvc.number_of_entries--;
                        break;
                    }
                }

                /*pvc.number_of_entries--;*/

                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_DelVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*vlan aggregation*/
static int CTCONU_TK3723_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid, onuid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc)) )
#else
            if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, (USHORT)port_id, &pvc)) )
#endif
            {
                pvc.mode = CTC_VLAN_MODE_AGGREGATION;

                for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                {
                    if (pvc.vlan_aggregation_tables[i].target_vlan == 0)
                    {
                        pvc.vlan_aggregation_tables[i].target_vlan = targetVid;

                        for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                        {
                            if(inVid[j])
                            {
                                pvc.vlan_aggregation_tables[i].vlan_aggregation_list[j] = inVid[j];
                                pvc.vlan_aggregation_tables[i].number_of_aggregation_entries++;
                            }
                        }
                    }
                }

                pvc.number_of_aggregation_tables++;

                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int CTCONU_TK3723_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid, onuid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
#if 0
            if ( 0 == (iRlt = TkAdapter_PASCTC_get_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc)) )
#else
            if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, (USHORT)port_id, &pvc)) )
#endif
            {
                pvc.mode = CTC_VLAN_MODE_AGGREGATION;

                for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                {
                    if (pvc.vlan_aggregation_tables[i].target_vlan == targetVid)
                    {
                        for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                        {
                            pvc.vlan_aggregation_tables[i].vlan_aggregation_list[j] = 0;
                        }
                        pvc.vlan_aggregation_tables[i].number_of_aggregation_entries = 0;
                    }
                }
#if 0
                pvc.number_of_aggregation_tables--;
#else
                if(pvc.number_of_aggregation_tables)/*modi 2013-3-21 防止越界，导致pon板挂死*/
                    pvc.number_of_aggregation_tables--;

#endif
                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_DelVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*QinQ enable*/
static int CTCONU_TK3723_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    ULONG vid = 1;
    short int llid, onuid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = getOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_default_vid, &vid)) )
            {
                qinq_config.mode=enable;
                qinq_config.default_vlan=vid;
                qinq_config.number_of_entries=0;
                VOS_MemZero(qinq_config.vlan_list, CTC_MAX_VLAN_QINQ_ENTRIES);
                
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortQinQEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return  iRlt;
}

/*qinq port vlan tag add*/
static int CTCONU_TK3723_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = get_OnuVlanTagConfig(olt_id, onu_id, port_id, &qinq_config)) )
            {
                if ( CTC_MAX_VLAN_QINQ_ENTRIES/2 > qinq_config.number_of_entries )
                {
                    int n = qinq_config.number_of_entries;

                    qinq_config.vlan_list[2*n] = cvlan;
                    qinq_config.vlan_list[2*n+1] = svlan;
                    qinq_config.number_of_entries++;

                    iRlt = OLT_ERR_NOTSUPPORT;
                }
                else
                    iRlt = OLT_ERR_NORESC;
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_AddQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port vlan tag del*/
static int CTCONU_TK3723_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG  cvlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, n;
    short int llid, onuid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            if ( 0 == (iRlt = get_OnuVlanTagConfig(olt_id, onu_id, port_id, &qinq_config)) )
            {
                for (i = 0; i < qinq_config.number_of_entries; i++)
                {
                    if (qinq_config.vlan_list[2*i] == cvlan)
                    {
                        n = qinq_config.number_of_entries;
                        linux_memmove(qinq_config.vlan_list+2*i, qinq_config.vlan_list+2*i+2, (n-i-1)*2*sizeof(LONG));
                        qinq_config.number_of_entries--;
                        break;
                    }

                    iRlt = OLT_ERR_NOTSUPPORT;
                }
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_DelQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return  iRlt;
}


static int CTCONU_TK3723_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkCtcEthernetPortInfo port_info;
            TkCtcEthernetPortCtrl port_ctrl;
            U8 enable = mode ? FALSE:TRUE;

            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                if ( port_info.autoNegoEnabled != enable )
                {
                    port_ctrl.autoNegoEnable = enable;
                    port_ctrl.restartAutoNego = enable;
                    
                    port_ctrl.phyEnable = port_info.phyEnabled;
                    port_ctrl.flowControlEnable = port_info.flowControlEnabled;
                    
                    iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
                }
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkCtcEthernetPortInfo port_info;
            TkCtcEthernetPortCtrl port_ctrl;
            U8 enable = en ? TRUE:FALSE;

            if ( 0 == (iRlt = TkCTC_GetEthPortInfo(olt_id, onuid, (short int)port_id, &port_info)) )
            {
                if ( port_info.flowControlEnabled!= enable )
                {
                    port_ctrl.autoNegoEnable = port_info.autoNegoEnabled;
                    port_ctrl.restartAutoNego = FALSE;
                    
                    port_ctrl.phyEnable = port_info.phyEnabled;
                    port_ctrl.flowControlEnable = port_info.flowControlEnabled;
                    
                    iRlt = TkCTC_SetEthPort(olt_id, onuid, (short int)port_id, &port_ctrl);
                }
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortFcMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}


static int CTCONU_TK3723_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkCtcPolicingParam police_info;
            ULONG cir = rate;
            ULONG cbs = cir*1000/8;
#if 0
            ULONG ebs = cir*1000/8*2;
#else
            ULONG ebs = 1522; /*CTC规范要求，ebs要小于等于以太网帧最大字节数*/
#endif

            /*added by luh @2015-1-15 协议中定义为3个字节，如果超过3个字节则取最大值*/                
            if(cbs & 0xFF000000)
            {
                cbs = 0xFFFFFF;
            }

            police_info.policingEnabled = cir ? CTC_STACK_STATE_ACTIVATE : CTC_STACK_STATE_DEACTIVATE;

            VOS_MemCpy(&police_info.cir, ((U8*)&cir) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.policeBucket, ((U8*)&cbs) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.extraBucket, ((U8*)&ebs) + 1, sizeof(U24));
            iRlt = TkCTC_SetEthPortPolicing(olt_id, onuid, (short int)port_id, &police_info);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortIngressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            TkCtcDownstreamRateLimitingParam police_info;
            ULONG cir = rate;

            police_info.enabled = rate ? TRUE : FALSE;

            VOS_MemCpy(&police_info.cir, ((U8*)&cir) + 1, sizeof(U24));
            VOS_MemCpy(&police_info.pir, &police_info.cir, sizeof(U24));
            iRlt = TkCTC_SetEthPortDownRateLimit(olt_id, onuid, (short int)port_id, &police_info);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortEgressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
    		if ( 0 == qossetid ) /* 删除pon端口上的qos配置*/
    		{
    			iRlt =  TkCTC_ClearPortClass(olt_id, onuid, (short int)port_id);
    		}
            else
            {
                qos_classification_rules_t rules;
                
                if ( 0 == (iRlt = getOnuConfQosSet(olt_id, onu_id, qossetid, &rules)) )
                {
                    CTC_STACK_classification_rules_t qset;
                    qos_classification_rule_t *p;
                    int nr = sizeof(rules)/sizeof(qos_classification_rule_t);
					/*modified by liyang @2015-04-22 for Q25412*/
                    int i=0, c = 0, j; /*pas-soft stack qosset entry index low limit is 1 and upper limmit is 31*/

                    VOS_MemZero(&qset, sizeof(CTC_STACK_classification_rules_t));

                    for(i=0; i<nr; i++)
                    {
                        p = &rules[i];
                        if(p->valid)
                        {
                            int nc = sizeof(p->entries)/sizeof(qos_class_rule_entry);
                            int ic = 0;

                            qset[c].valid = p->valid;
                            qset[c].num_of_entries = p->num_of_entries;
                            qset[c].queue_mapped = p->queue_mapped;
                            qset[c].priority_mark = p->priority_mark;

                            for(j=0; j<nc; j++)
                            {
                                if(p->entrymask & (1<<j))
                                {
                                    qset[c].entries[ic].field_select = p->entries[j].field_select;
                                    qset[c].entries[ic].validation_operator = p->entries[j].validation_operator;
                                    switch(p->entries[j].field_select)
                                    {
                                    case FIELD_SEL_DA_MAC:
                                    case FIELD_SEL_SA_MAC:
                                        VOS_MemCpy(&qset[c].entries[ic].value.mac_address, &p->entries[j].value.mac_address, sizeof(mac_address_t));
                                        break;
                                    case FIELD_SEL_DEST_IPV6:
                                    case FIELD_SEL_SRC_IPV6:
                                        VOS_MemCpy(&qset[c].entries[ic].value.ipv6_match_value, &p->entries[j].value.ipv6_match_value, sizeof(PON_ipv6_addr_t));
                                        break;
                                    default:
                                        qset[c].entries[ic].value.match_value = p->entries[j].value.match_value;

                                    }
                                    ic++;
                                }
                            }

                            c++;
                        }
                    }

                    TkCTC_ClearPortClass(olt_id, onuid, (short int)port_id);

			
                    iRlt = TkAdapter_PASCTC_set_port_class_config(olt_id, onuid, (short int)port_id, CTC_CLASSIFICATION_ADD_RULE, qset);
	
				}
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            iRlt = TkCTC_ClearPortClass(olt_id, onuid, (short int)port_id);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_ClrPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



static int CTCONU_TK3723_SetPortDefaultPriority(short int olt_id, short int onu_id, int port_id, int priority)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            CTC_STACK_port_vlan_configuration_t pvc;
             
            if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, port_id, &pvc)) )
            {
                pvc.default_vlan &= 0x1fff;
                pvc.default_vlan |= priority << 13;
                 
                iRlt = TkAdapter_PASCTC_set_ethport_vlan_config(olt_id, onuid, (short int)port_id, &pvc);
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortDefaultPriority(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char switch_mode;

            if ( en )
            {
                switch_mode = TkCtcOnuMulticastSwitchingModeIgmpMldSnooping;
            }
            else
            {
                switch_mode = TkCtcOnuMulticastSwitchingModeCtcControllableIgmpMld;
            }
            
            iRlt = TkCTC_SetOnuMulticastSwitchMode(olt_id, onuid, switch_mode);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetIgmpEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_TK3723_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            unsigned char switch_mode;
            
            switch ( en )
            {
                case CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING:
                    switch_mode = TkCtcOnuMulticastSwitchingModeIgmpMldSnooping;
                    break;
                case CTC_STACK_PROTOCOL_CTC:
                    switch_mode = TkCtcOnuMulticastSwitchingModeCtcControllableIgmpMld;
                    break;
                default:
                    return OLT_ERR_PARAM;
            }
            
            iRlt = TkCTC_SetOnuMulticastSwitchMode(olt_id, onuid, switch_mode);
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetIgmpAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_TK3723_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
            CTC_STACK_multicast_vlan_t mv;
            int num = 0, vids[ONU_MAX_IGMP_VLAN];

            if ( 0 == vid )
            {
                iRlt = TkCTC_ClearPortMulticastVlan(olt_id, onuid, (short int)port_id);
            }
            else
            {
                if ( 0 == (iRlt= getOnuConfPortMulticastVlan(olt_id, onu_id, port_id, &num, vids)) )
                {
                    int i, fexist = 0;
                    unsigned short vlan_nums;    
                    
                    for(i=0; i<num; i++)
                    {
                        if(vids[i] == vid)
                            fexist = 1;
                        mv.vlan_id[i] = vids[i];
                    }

                    if(i < ONU_MAX_IGMP_VLAN)
                    {
                        if(!fexist)
                        {
                            mv.vlan_id[i] = vid;
                            mv.num_of_vlan_id = num+1;
                        }
                        else
                            mv.num_of_vlan_id = num;
                        
                        TkCTC_ClearPortMulticastVlan(olt_id, onuid, (short int)port_id);

                        mv.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
                        vlan_nums = mv.num_of_vlan_id;
                        iRlt = TkCTC_AddPortMulticastVlan(olt_id, onuid, (short int)port_id, vlan_nums, mv.vlan_id);
                    }
                }
            }
        }
    }
    
    OLT_TK_DEBUG(OLT_TK_TITLE"CTCONU_TK3723_SetPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */

#if 1
/* --------------------CMC管理API------------------- */
static int CMCONU_TK3723_RegisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
	
    iRlt = CMCCTRL_RegisterCmc(cmc_mac, NULL, 0);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_RegisterCmc(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_UnregisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
	
    iRlt = CMCCTRL_UnRegisterCmc(cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_UnregisterCmc(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_DumpCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_DumpCmc(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmc(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpAlarm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpAlarmList(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpAlarm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLog(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpLogData(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLog(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_ResetCmcBoard(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_ResetCmcBoard(cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_ResetCmcBoard(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcVersion(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetSoftwareVersion(version, len, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcMaxMulticasts(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_multicasts)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetNumOfMulticasts(max_multicasts, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcMaxMulticasts(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *max_multicasts, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_cm)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetNumOfCm(max_cm, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcMaxCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *max_cm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short max_cm)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_SetNumOfCm(max_cm, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcMaxCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, max_cm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_GetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt;
    BASE_DateTimeT cmc_time;

    OLT_LOCAL_ASSERT(olt_id);

    if ( 0 == (iRlt = CMCOAM_System_GetTime(cmc_mac, &cmc_time)) )
    {
        time->tm_year = cmc_time.year;
        time->tm_mon  = cmc_time.month;
        time->tm_mday = cmc_time.day;
        time->tm_wday = cmc_time.wday;
        time->tm_hour = cmc_time.hour;
        time->tm_min  = cmc_time.min;
        time->tm_sec  = cmc_time.sec;
    }
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt;
    BASE_DateTimeT cmc_time;

    OLT_LOCAL_ASSERT(olt_id);

    cmc_time.year  = time->tm_year;
    cmc_time.month = time->tm_mon;
    cmc_time.day   = time->tm_mday;
    cmc_time.wday  = time->tm_wday;
    cmc_time.hour  = time->tm_hour;
    cmc_time.min   = time->tm_min;
    cmc_time.sec   = time->tm_sec;

    iRlt = CMCOAM_System_SetTime(cmc_mac, &cmc_time);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_LocalCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_LocalTime(cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_LocateCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_SetCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len)
{
    int iRlt;
    BASE_DateTimeT cmc_time;


    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_SetCustomConfiguration(cfg_id, cfg_data, data_len, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcCustomConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cfg_id, data_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpCmcCustomCfg(cmc_mac, cfg_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcCustomConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cfg_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

#if 1
/* --------------------CMC频道管理API------------------- */
static int CMCONU_TK3723_DumpCmcDownChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpDownstreamChannel(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcDownChannel(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcUpChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamChannel(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcUpChannel(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *docsis30_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsD30Mode(channel_id, docsis30_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *docsis30_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char docsis30_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsD30Mode(channel_id, docsis30_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, docsis30_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_AutoAssignDownstreamSettingsFreq(base_freq, step_freq, step_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_AutoAssignUpstreamSettingsFreq(base_freq, step_freq, step_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_width)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsChannelWidth(channel_id, channel_width, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_width, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_width)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsChannelWidth(channel_id, channel_width, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_width, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *annex_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsAnnexMode(channel_id, annex_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *annex_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char annex_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsAnnexMode(channel_id, annex_mode, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, annex_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamChannelType(channel_id, channel_type, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamChannelType(channel_id, channel_type, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *modulation_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsModulation(channel_id, modulation_type, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *modulation_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char modulation_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsModulation(channel_id, modulation_type, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, modulation_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_profile)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsChannelProfile(channel_id, channel_profile, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_profile, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_profile)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsChannelProfile(channel_id, channel_profile, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_profile, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *interleaver)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsInterleaver(channel_id, interleaver, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *interleaver, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char interleaver)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsInterleaver(channel_id, interleaver, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, interleaver, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsPowerLevel(channel_id, channel_power, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsPowerLevel(channel_id, channel_power, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamInputPower(channel_id, channel_power, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamInputPower(channel_id, channel_power, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamInputPower(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcUpChannelSignalQuality(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamChannelSignalQuality(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcUpChannelSignalQuality(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcInterfaceUtilization(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    unsigned short channel_index;

    OLT_LOCAL_ASSERT(olt_id);

    channel_index = CMC_CHANNELID_2_IFINDEX(channel_type, channel_id);
    iRlt = CMCOAM_Channel_DumpCmcInterfaceUtilization(cmc_mac, channel_index, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcInterfaceUtilization(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcInterfaceStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    unsigned short channel_index;

    OLT_LOCAL_ASSERT(olt_id);

    channel_index = CMC_CHANNELID_2_IFINDEX(channel_type, channel_id);
    iRlt = CMCOAM_Channel_DumpCmcInterfaceStatistics(cmc_mac, channel_index, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcInterfaceStatistics(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcMacStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpCmcMacStatistics(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcMacStatistics(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcAllInterface(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpCmcAllInterfaces(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcAllInterface(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------CMC频道组管理API------------------- */
static int CMCONU_TK3723_DumpAllLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpAllLoadBalancingGrp(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpAllLoadBalancingGrp(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpFullLoadBalancingGrpSettings(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpDownsteam(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpUpsteam(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingDynConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingDynConfig(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingDynConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_SetLoadBalancingDynMethod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char method)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMethod(cmc_mac, method);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynMethod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, method, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynPeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynPeriod(cmc_mac, period);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynPeriod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, period, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynWeightedAveragePeriod(cmc_mac, period);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, period, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynOverloadThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynOverloadThresold(cmc_mac, percent);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynOverloadThresold(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, percent, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynDifferenceThresold(cmc_mac, percent);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, percent, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long max_move)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMaxMoveNumber(cmc_mac, max_move);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, max_move, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynMinHoldTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long hold_time)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMinHoldTime(cmc_mac, hold_time);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynMinHoldTime(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, hold_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char range_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynRangeOverrideMode(cmc_mac, range_mode);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, range_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynAtdmaDccInitTech(cmc_mac, tech_id);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynScdmaDccInitTech(cmc_mac, tech_id);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynAtdmaDbcInitTech(cmc_mac, tech_id);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynScdmaDbcInitTech(cmc_mac, tech_id);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_CreateLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char grp_method)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_CreateDestroyLoadBalancingGrp(grp_id, grp_method, 1, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_CreateLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, grp_method, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DestroyLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_CreateDestroyLoadBalancingGrp(grp_id, 1, 2, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DestroyLoadBalancingGrp(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_AddLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveDownChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 1, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_AddLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_RemoveLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveDownChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 2, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_RemoveLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_AddLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveUpChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 1, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_AddLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_RemoveLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveUpChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 2, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_RemoveLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_AddLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveCnuToFromLoadBalancingGrp(grp_id, 1, mac_start, mac_end, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_AddLoadBalancingGrpModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_RemoveLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveCnuToFromLoadBalancingGrp(grp_id, 2, mac_start, mac_end, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_RemoveLoadBalancingGrpModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_AddLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetExcludeCnusFromLoadBalancingGrp(1, mac_start, mac_end, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_AddLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetExcludeCnusFromLoadBalancingGrp(2, mac_start, mac_end, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_DumpLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpCnuConfig(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpModem(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrpActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpActivedCnu(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpActivedModem(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpExcludeCnusConfiguration(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpExcludeActiveCnus(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------CM管理API------------------- */
static int CMCONU_TK3723_DumpAllCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCnuList(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpAllCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuStatus(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCableModemHistory(cmc_mac, NULL, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpAllCmHistory(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCableModemHistory(cmc_mac, cm_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmHistory(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_ClearAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_ClearCableModemHistory(cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_ClearAllCmHistory(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_ResetCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_ClearCableModem(stCmMacAddr, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_ResetCm(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_DumpCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuDownstream(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmDownstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuUpstream(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmUpstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_SetCnuDownstream(stCmMacAddr, ch_ids, num_of_ch, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmDownstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_SetCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_SetCnuUpstream(stCmMacAddr, ch_ids, num_of_ch, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_SetCmUpstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_CreateCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_CreateServiceFlow(stCmMacAddr, cos, tlv_data, (int)tlv_len, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_CreateCmServiceFlow(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cos, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_ModifyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_ChangeServiceFlow(stCmMacAddr, usfid, dsfid, tlv_data, (int)tlv_len, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_ModifyCmServiceFlow(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, usfid, dsfid, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DestroyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_DestroyServiceFlow(stCmMacAddr, usfid, dsfid, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DestroyCmServiceFlow(%d, %d, %lu, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, usfid, dsfid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_DumpCmClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Qos_DumpCmClassifier(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmClassifier(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Qos_DumpCmServiceFlow(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmServiceFlow(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------QoS管理API------------------- */
static int CMCONU_TK3723_DumpCmcClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpPktClassifierConfig(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcClassifier(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpServiceFlowConfig(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcServiceFlow(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcServiceFlowStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpServiceFlowStatistics(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcServiceFlowStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcDownChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpDsBondingGroup(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcDownChannelBondingGroup(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DumpCmcUpChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpUsBondingGroup(cmc_mac, dump_buf, dump_len);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DumpCmcUpChannelBondingGroup(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_TK3723_CreateServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_Qos_SetServiceFlowClassName(class_name, 1, tlv_data, (int)tlv_len, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_CreateServiceFlowClassName(%d, %d, %s, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, class_name, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_DestroyServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_Qos_SetServiceFlowClassName(class_name, 2, NULL, 0, cmc_mac);
    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_DestroyServiceFlowClassName(%d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, class_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------地址管理API------------------- */
static int CMCONU_TK3723_GetCmcMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = TkAdapter_PAS5201_GetCmcAddrTable(olt_id, llid, cmc_mac, addr_num, addr_table);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmcMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_GetCmMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = TkAdapter_PAS5201_GetCnuAddrTable(olt_id, llid, cmc_mac, cm_mac, addr_num, addr_table);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_GetCmMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_TK3723_ResetCmAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = TkAdapter_PAS5201_ResetCnuAddrTable(olt_id, llid, cmc_mac, cm_mac, addr_type);
    }

    OLT_TK_DEBUG(OLT_TK_TITLE"CMCONU_TK3723_ResetCmAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, addr_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#endif


/* --------------------END------------------------ */

/******************************************外部接口***************************************/
static  const OnuMgmtIFs  s_gwonu3723Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_TK3723_OnuIsOnline,
    GWONU_TK3723_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_TK3723_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_TK3723_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_TK3723_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_TK3723_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    GWONU_TK3723_SetOnuTrafficServiceMode,
    GWONU_TK3723_SetOnuPeerToPeer,
    GWONU_TK3723_SetOnuPeerToPeerForward,
    GWONU_TK3723_SetOnuBW,
    GWONU_TK3723_GetOnuSLA,

    NULL,           /* SetOnuFecMode */
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_SetUniPort,
    GWONU_TK3723_SetSlowProtocolLimit,
    GWONU_TK3723_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_TK3723_GetOnuB2PMode,
    GWONU_TK3723_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_TK3723_ResetCounters,
    NULL,           /* SetPonLoopback */
#endif

#if 1
/* -------------------ONU加密管理API-------------------- */
    GWONU_TK3723_GetLLIDParams,
    GWONU_TK3723_StartEncryption,
    GWONU_TK3723_StopEncryption,
    GWONU_TK3723_SetOnuEncryptParams,
    GWONU_TK3723_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_TK3723_GetOnuMacAddrTbl,
    GWONU_TK3723_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_TK3723_SetOnuMaxMacNum,
    GWONU_TK3723_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_TK3723_GetOnuDistance,
    GWONU_TK3723_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_TK3723_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_TK3723_GetOnuPonVer,
    GWONU_TK3723_GetOnuRegisterInfo,
    GWONU_TK3723_GetOnuI2CInfo,
    GWONU_TK3723_SetOnuI2CInfo,
    
    GWONU_TK3723_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    GWONU_SetOnuDeviceName,
    GWONU_SetOnuDeviceDesc,
    GWONU_SetOnuDeviceLocation,
    NULL,
#endif

#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
    NULL,           /* GetCtcVersion */
    NULL,           /* GetFirmwareVersion */
    NULL,           /* GetSerialNumber */
    NULL,           /* GetChipsetID */

    NULL,           /* GetOnuCap1 */
    NULL,           /* GetOnuCap2 */
    NULL,           /* GetOnuCap3 */

    NULL,           /* UpdateOnuFirmware */
    NULL,           /* ActiveOnuFirmware */
    NULL,           /* CommitOnuFirmware */
    
    NULL,           /* StartEncrypt */
    NULL,           /* StopEncrypt */
    
    NULL,           /* GetEthPortLinkState */
    NULL,           /* GetEthPortAdminStatus */
    GWONU_SetEthPortAdminStatus,
    
    NULL,           /* GetEthPortPause */
    GWONU_SetEthPortPause,
    
    NULL,           /* GetEthPortAutoNegotiation */
    NULL,           /* SetEthPortAutoNegotiation */
    GWONU_SetEthPortAutoNegotiationRestart,
    NULL,           /* GetEthPortAnLocalTecAbility */
    NULL,           /* GetEthPortAnAdvertisedTecAbility */
    
    NULL,           /* GetEthPortPolicing */
    NULL,           /* SetEthPortPolicing */
    
    NULL,           /* GetEthPortDsPolicing */
    NULL,           /* SetEthPortDsPolicing */

    NULL,           /* GetEthPortVlanConfig */
    NULL,           /* SetEthPortVlanConfig */
    NULL,           /* GetAllPortVlanConfig */
    
    NULL,           /* GetEthPortClassificationAndMark */
    NULL,           /* SetEthPortClassificationAndMark */
    NULL,           /* ClearEthPortClassificationAndMarking */
    
    NULL,           /* GetEthPortMulticatVlan */
    NULL,           /* SetEthPortMulticatVlan */
    NULL,           /* ClearEthPortMulticastVlan */

    NULL,           /* GetEthPortMulticastGroupMaxNumber */
    GWONU_SetEthPortMulticastGroupMaxNumber,

    NULL,           /* GetEthPortMulticastTagStrip */
    GWONU_SetEthPortMulticastTagStrip,
    NULL,           /* GetAllPortMulticastTagStrip */
    
    NULL,           /* GetEthPortMulticastTagOper */
    GWONU_SetEthPortMulticastTagOper,
    NULL,           /* SetObjMulticastTagOper */
    
    NULL,           /* GetMulticatControl */
    NULL,           /* SetMulticatControl */
    NULL,           /* ClearMulticastControl */

    NULL,           /* GetMulticatSwitch */
    NULL,           /* SetMulticatSwitch */
    
    NULL,           /* GetMulticastFastLeaveAbility */
    NULL,           /* GetMulticastFastLeave */
    NULL,           /* SetMulticastFastLeave */

    NULL,           /* GetOnuPortStatisticData */
    NULL,           /* GetOnuPortStatisticState */
    NULL,           /* SetOnuPortStatisticState */

    NULL,           /* SetObjAlarmAdminState */
    NULL,           /* SetObjAlarmThreshold */
    NULL,           /* GetDbaReportThreshold */
    NULL,           /* SetDbaReportThreshold */
  
    NULL,           /* GetMngGlobalConfig */
    NULL,           /* SetMngGlobalConfig */
    NULL,           /* GetMngSnmpConfig */
    NULL,           /* SetMngSnmpConfig */
    
    NULL,           /* GetHoldOver */
    NULL,           /* SetHoldOver */
    NULL,           /* GetOptTransDiag */
    NULL,           /* SetTxPowerSupplyControl */

    NULL,           /* GetFecAbility */
    
    NULL,           /* GetIADInfo */
    NULL,           /* GetVoipIadOperation */
    NULL,           /* SetVoipIadOperation */
    NULL,           /* GetVoipGlobalConfig */
    NULL,           /* SetVoipGlobalConfig */
    NULL,           /* GetVoipFaxConfig */
    NULL,           /* SetVoipFaxConfig */

    NULL,           /* GetVoipPotsStatus */
    NULL,           /* GetVoipPort */
    NULL,           /* SetVoipPort */
    NULL,           /* GetVoipPort2 */
    NULL,           /* SetVoipPort2 */

    NULL,           /* GetH248Config */
    NULL,           /* SetH248Config */
    NULL,           /* GetH248UserTidConfig */
    NULL,           /* SetH248UserTidConfig */
    NULL,           /* GetH248RtpTidConfig */
    NULL,           /* SetH248RtpTidConfig */

    NULL,           /* GetSipConfig */
    NULL,           /* SetSipConfig */
    NULL,           /* SetSipDigitMap */
    NULL,           /* GetSipUserConfig */
    NULL,           /* SetSipUserConfig */
    NULL,           /* GetOnuPortDataByID*/
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    GWONU_CliCall,
    
    GWONU_SetMgtReset,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    NULL,           /* SetPasFlush */
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    GWONU_SetVlanEnable,
    GWONU_SetVlanMode,
    GWONU_AddVlan,
    GWONU_DelVlan,
    GWONU_SetPortPvid,

    GWONU_AddVlanPort,
    GWONU_DelVlanPort,
    GWONU_SetVlanTran,
    GWONU_DelVlanTran,
    GWONU_SetVlanAgg,
    GWONU_DelVlanAgg,

    GWONU_SetPortQinQEnable,
    GWONU_AddQinQVlanTag,
    GWONU_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,

    GWONU_SetPortMode,
    GWONU_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    GWONU_SetPortIngressRate,
    GWONU_SetPortEgressRate,
    
    GWONU_SetQosClass,
    GWONU_ClrQosClass,
    GWONU_SetQosRule,
    GWONU_ClrQosRule,
    
    GWONU_SetPortQosRule,
    GWONU_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    GWONU_SetPortDefPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    GWONU_SetIgmpEnable,
    GWONU_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    GWONU_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    CMCONU_TK3723_RegisterCmc,
    CMCONU_TK3723_UnregisterCmc,
    CMCONU_TK3723_DumpCmc,
    CMCONU_TK3723_DumpAlarm,
    CMCONU_TK3723_DumpLog,
    
    CMCONU_TK3723_ResetCmcBoard,
    CMCONU_TK3723_GetCmcVersion,
    CMCONU_TK3723_GetCmcMaxMulticasts,
    CMCONU_TK3723_GetCmcMaxCm,
    CMCONU_TK3723_SetCmcMaxCm,
    
    CMCONU_TK3723_GetCmcTime,
    CMCONU_TK3723_SetCmcTime,
    CMCONU_TK3723_LocalCmcTime,
    CMCONU_TK3723_SetCmcCustomConfig,
    CMCONU_TK3723_DumpCmcCustomConfig,
    
    CMCONU_TK3723_DumpCmcDownChannel,
    CMCONU_TK3723_DumpCmcUpChannel,
    CMCONU_TK3723_GetCmcDownChannelMode,
    CMCONU_TK3723_SetCmcDownChannelMode,
    CMCONU_TK3723_GetCmcUpChannelMode,

    CMCONU_TK3723_SetCmcUpChannelMode,
    CMCONU_TK3723_GetCmcUpChannelD30Mode,
    CMCONU_TK3723_SetCmcUpChannelD30Mode,
    CMCONU_TK3723_GetCmcDownChannelFreq,
    CMCONU_TK3723_SetCmcDownChannelFreq,

    CMCONU_TK3723_GetCmcUpChannelFreq,
    CMCONU_TK3723_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_TK3723_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_TK3723_SetCmcUpAutoFreq,
    CMCONU_TK3723_GetCmcUpChannelWidth,
    CMCONU_TK3723_SetCmcUpChannelWidth,
    CMCONU_TK3723_GetCmcDownChannelAnnexMode,
    CMCONU_TK3723_SetCmcDownChannelAnnexMode,
    
    CMCONU_TK3723_GetCmcUpChannelType,
    CMCONU_TK3723_SetCmcUpChannelType,
    CMCONU_TK3723_GetCmcDownChannelModulation,
    CMCONU_TK3723_SetCmcDownChannelModulation,
    CMCONU_TK3723_GetCmcUpChannelProfile,
    
    CMCONU_TK3723_SetCmcUpChannelProfile,
    CMCONU_TK3723_GetCmcDownChannelInterleaver,
    CMCONU_TK3723_SetCmcDownChannelInterleaver,
    CMCONU_TK3723_GetCmcDownChannelPower,    
    CMCONU_TK3723_SetCmcDownChannelPower,    

    CMCONU_TK3723_GetCmcUpChannelPower,    
    CMCONU_TK3723_SetCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelSignalQuality,    
    CMCONU_TK3723_DumpCmcInterfaceUtilization,    

    CMCONU_TK3723_DumpCmcInterfaceStatistics,    
    CMCONU_TK3723_DumpCmcMacStatistics,    
    CMCONU_TK3723_DumpCmcAllInterface,    
    CMCONU_TK3723_DumpAllLoadBalancingGrp,    
    CMCONU_TK3723_DumpLoadBalancingGrp,    

    CMCONU_TK3723_DumpLoadBalancingGrpDownstream,    
    CMCONU_TK3723_DumpLoadBalancingGrpUpstream,    
    CMCONU_TK3723_DumpLoadBalancingDynConfig,    
    CMCONU_TK3723_SetLoadBalancingDynMethod,    
    CMCONU_TK3723_SetLoadBalancingDynPeriod,    

    CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_TK3723_SetLoadBalancingDynOverloadThresold,    
    CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_TK3723_SetLoadBalancingDynMinHoldTime,    

    CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_TK3723_CreateLoadBalancingGrp,    
    CMCONU_TK3723_DestroyLoadBalancingGrp,    
    CMCONU_TK3723_AddLoadBalancingGrpDownstream,    
    CMCONU_TK3723_RemoveLoadBalancingGrpDownstream,    
    CMCONU_TK3723_AddLoadBalancingGrpUpstream,    

    CMCONU_TK3723_RemoveLoadBalancingGrpUpstream,    
    CMCONU_TK3723_AddLoadBalancingGrpModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpModem,    
    CMCONU_TK3723_AddLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_TK3723_DumpLoadBalancingGrpModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpActivedModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_TK3723_DumpAllCm,    
    CMCONU_TK3723_DumpCm,    
    CMCONU_TK3723_DumpAllCmHistory,    
    CMCONU_TK3723_DumpCmHistory,    
    CMCONU_TK3723_ClearAllCmHistory,    

    CMCONU_TK3723_ResetCm,    
    CMCONU_TK3723_DumpCmDownstream,    
    CMCONU_TK3723_DumpCmUpstream,    
    CMCONU_TK3723_SetCmDownstream,    
    CMCONU_TK3723_SetCmUpstream,    

    CMCONU_TK3723_CreateCmServiceFlow,    
    CMCONU_TK3723_ModifyCmServiceFlow,    
    CMCONU_TK3723_DestroyCmServiceFlow,    
    CMCONU_TK3723_DumpCmClassifier,    
    CMCONU_TK3723_DumpCmServiceFlow,    
    
    CMCONU_TK3723_DumpCmcClassifier,    
    CMCONU_TK3723_DumpCmcServiceFlow,    
    CMCONU_TK3723_DumpCmcServiceFlowStatistics,    
    CMCONU_TK3723_DumpCmcDownChannelBondingGroup,    
    CMCONU_TK3723_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_TK3723_CreateServiceFlowClassName,    
    CMCONU_TK3723_DestroyServiceFlowClassName,    
    CMCONU_TK3723_GetCmcMacAddrTbl,    
    CMCONU_TK3723_GetCmMacAddrTbl,    
    CMCONU_TK3723_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
        /*----------------GPON OMCI----------------------*/
        NULL,
        NULL,
#endif
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	DRPENG_SetOnuPortIsolation,
	DRPENG_GetOnuPortIsolation,
	DRPENG_GetOnuMacAddressTable,
	DRPENG_SetOnuPortSaveConfiguration,
	DRPENG_SetOnuLoopDetectionTime,

	DRPENG_GetOnuLoopDetectionTime,
	DRPENG_SetOnuPortMode,
	DRPENG_GetOnuPortMode,
	DRPENG_SetOnuPortStormStatus,
	DRPENG_GetOnuPortStormStatus,

	DRPENG_SetOnuDeviceLocation,
	DRPENG_GetOnuDeviceLocation,
	DRPENG_SetOnuDeviceDescription,
	DRPENG_GetOnuDeviceDescription,
	DRPENG_SetOnuDeviceName,

	DRPENG_GetOnuDeviceName,
	DRPENG_GetOnuPortMacAddressNumber,
	DRPENG_GetOnuPortLocationByMAC,
	DRPENG_GetOnuSupportExtendAttribute,
    /* --------------------END------------------------ */

    REMOTE_ERROR
};

static  const OnuMgmtIFs  s_gwonu55524Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_TK3723_OnuIsOnline,
    GWONU_TK3723_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_TK3723_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_BCM55524_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_TK3723_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_TK3723_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    GWONU_TK3723_SetOnuTrafficServiceMode,
    GWONU_TK3723_SetOnuPeerToPeer,
    GWONU_TK3723_SetOnuPeerToPeerForward,
    GWONU_TK3723_SetOnuBW,
    GWONU_TK3723_GetOnuSLA,

    GWONU_TK3723_SetOnuFecMode,
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_SetUniPort,
    GWONU_TK3723_SetSlowProtocolLimit,
    GWONU_TK3723_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_TK3723_GetOnuB2PMode,
    GWONU_TK3723_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_TK3723_ResetCounters,
    GWONU_TK3723_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_TK3723_GetLLIDParams,
    GWONU_TK3723_StartEncryption,
    GWONU_TK3723_StopEncryption,
    GWONU_TK3723_SetOnuEncryptParams,
    GWONU_TK3723_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_TK3723_GetOnuMacAddrTbl,
    GWONU_TK3723_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_TK3723_SetOnuMaxMacNum,
    GWONU_TK3723_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_TK3723_GetOnuDistance,
    GWONU_TK3723_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_TK3723_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_TK3723_GetOnuPonVer,
    GWONU_TK3723_GetOnuRegisterInfo,
    GWONU_TK3723_GetOnuI2CInfo,
    GWONU_TK3723_SetOnuI2CInfo,
    
    GWONU_TK3723_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    GWONU_SetOnuDeviceName,
    GWONU_SetOnuDeviceDesc,
    GWONU_SetOnuDeviceLocation,
    NULL,
#endif

#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
    NULL,           /* GetCtcVersion */
    NULL,           /* GetFirmwareVersion */
    NULL,           /* GetSerialNumber */
    NULL,           /* GetChipsetID */

    NULL,           /* GetOnuCap1 */
    NULL,           /* GetOnuCap2 */
    NULL,           /* GetOnuCap3 */

    NULL,           /* UpdateOnuFirmware */
    NULL,           /* ActiveOnuFirmware */
    NULL,           /* CommitOnuFirmware */
    
    NULL,           /* StartEncrypt */
    NULL,           /* StopEncrypt */
    
    NULL,           /* GetEthPortLinkState */
    NULL,           /* GetEthPortAdminStatus */
    GWONU_SetEthPortAdminStatus,
    
    NULL,           /* GetEthPortPause */
    GWONU_SetEthPortPause,
    
    NULL,           /* GetEthPortAutoNegotiation */
    NULL,           /* SetEthPortAutoNegotiation */
    GWONU_SetEthPortAutoNegotiationRestart,
    NULL,           /* GetEthPortAnLocalTecAbility */
    NULL,           /* GetEthPortAnAdvertisedTecAbility */
    
    NULL,           /* GetEthPortPolicing */
    NULL,           /* SetEthPortPolicing */
    
    NULL,           /* GetEthPortDsPolicing */
    NULL,           /* SetEthPortDsPolicing */

    NULL,           /* GetEthPortVlanConfig */
    NULL,           /* SetEthPortVlanConfig */
    NULL,           /* GetAllPortVlanConfig */
    
    NULL,           /* GetEthPortClassificationAndMark */
    NULL,           /* SetEthPortClassificationAndMark */
    NULL,           /* ClearEthPortClassificationAndMarking */
    
    NULL,           /* GetEthPortMulticatVlan */
    NULL,           /* SetEthPortMulticatVlan */
    NULL,           /* ClearEthPortMulticastVlan */

    NULL,           /* GetEthPortMulticastGroupMaxNumber */
    GWONU_SetEthPortMulticastGroupMaxNumber,

    NULL,           /* GetEthPortMulticastTagStrip */
    GWONU_SetEthPortMulticastTagStrip,
    NULL,           /* GetAllPortMulticastTagStrip */
    
    NULL,           /* GetEthPortMulticastTagOper */
    GWONU_SetEthPortMulticastTagOper,
    NULL,           /* SetObjMulticastTagOper */
    
    NULL,           /* GetMulticatControl */
    NULL,           /* SetMulticatControl */
    NULL,           /* ClearMulticastControl */

    NULL,           /* GetMulticatSwitch */
    NULL,           /* SetMulticatSwitch */
    
    NULL,           /* GetMulticastFastLeaveAbility */
    NULL,           /* GetMulticastFastLeave */
    NULL,           /* SetMulticastFastLeave */

    NULL,           /* GetOnuPortStatisticData */
    NULL,           /* GetOnuPortStatisticState */
    NULL,           /* SetOnuPortStatisticState */

    NULL,           /* SetObjAlarmAdminState */
    NULL,           /* SetObjAlarmThreshold */
    NULL,           /* GetDbaReportThreshold */
    NULL,           /* SetDbaReportThreshold */
  
    NULL,           /* GetMngGlobalConfig */
    NULL,           /* SetMngGlobalConfig */
    NULL,           /* GetMngSnmpConfig */
    NULL,           /* SetMngSnmpConfig */
    
    NULL,           /* GetHoldOver */
    NULL,           /* SetHoldOver */
    NULL,           /* GetOptTransDiag */
    NULL,           /* SetTxPowerSupplyControl */

    NULL,           /* GetFecAbility */
    
    NULL,           /* GetIADInfo */
    NULL,           /* GetVoipIadOperation */
    NULL,           /* SetVoipIadOperation */
    NULL,           /* GetVoipGlobalConfig */
    NULL,           /* SetVoipGlobalConfig */
    NULL,           /* GetVoipFaxConfig */
    NULL,           /* SetVoipFaxConfig */

    NULL,           /* GetVoipPotsStatus */
    NULL,           /* GetVoipPort */
    NULL,           /* SetVoipPort */
    NULL,           /* GetVoipPort2 */
    NULL,           /* SetVoipPort2 */

    NULL,           /* GetH248Config */
    NULL,           /* SetH248Config */
    NULL,           /* GetH248UserTidConfig */
    NULL,           /* SetH248UserTidConfig */
    NULL,           /* GetH248RtpTidConfig */
    NULL,           /* SetH248RtpTidConfig */

    NULL,           /* GetSipConfig */
    NULL,           /* SetSipConfig */
    NULL,           /* SetSipDigitMap */
    NULL,           /* GetSipUserConfig */
    NULL,           /* SetSipUserConfig */
    NULL,           /* GetOnuPortDataByID*/
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    GWONU_CliCall,
    
    GWONU_SetMgtReset,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    NULL,           /* SetPasFlush */
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    GWONU_SetVlanEnable,
    GWONU_SetVlanMode,
    GWONU_AddVlan,
    GWONU_DelVlan,
    GWONU_SetPortPvid,

    GWONU_AddVlanPort,
    GWONU_DelVlanPort,
    GWONU_SetVlanTran,
    GWONU_DelVlanTran,
    GWONU_SetVlanAgg,
    GWONU_DelVlanAgg,

    GWONU_SetPortQinQEnable,
    GWONU_AddQinQVlanTag,
    GWONU_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,

    GWONU_SetPortMode,
    GWONU_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    GWONU_SetPortIngressRate,
    GWONU_SetPortEgressRate,
    
    GWONU_SetQosClass,
    GWONU_ClrQosClass,
    GWONU_SetQosRule,
    GWONU_ClrQosRule,
    
    GWONU_SetPortQosRule,
    GWONU_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    GWONU_SetPortDefPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    GWONU_SetIgmpEnable,
    GWONU_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    GWONU_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    CMCONU_TK3723_RegisterCmc,
    CMCONU_TK3723_UnregisterCmc,
    CMCONU_TK3723_DumpCmc,
    CMCONU_TK3723_DumpAlarm,
    CMCONU_TK3723_DumpLog,
    
    CMCONU_TK3723_ResetCmcBoard,
    CMCONU_TK3723_GetCmcVersion,
    CMCONU_TK3723_GetCmcMaxMulticasts,
    CMCONU_TK3723_GetCmcMaxCm,
    CMCONU_TK3723_SetCmcMaxCm,
    
    CMCONU_TK3723_GetCmcTime,
    CMCONU_TK3723_SetCmcTime,
    CMCONU_TK3723_LocalCmcTime,
    CMCONU_TK3723_SetCmcCustomConfig,
    CMCONU_TK3723_DumpCmcCustomConfig,
    
    CMCONU_TK3723_DumpCmcDownChannel,
    CMCONU_TK3723_DumpCmcUpChannel,
    CMCONU_TK3723_GetCmcDownChannelMode,
    CMCONU_TK3723_SetCmcDownChannelMode,
    CMCONU_TK3723_GetCmcUpChannelMode,

    CMCONU_TK3723_SetCmcUpChannelMode,
    CMCONU_TK3723_GetCmcUpChannelD30Mode,
    CMCONU_TK3723_SetCmcUpChannelD30Mode,
    CMCONU_TK3723_GetCmcDownChannelFreq,
    CMCONU_TK3723_SetCmcDownChannelFreq,

    CMCONU_TK3723_GetCmcUpChannelFreq,
    CMCONU_TK3723_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_TK3723_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_TK3723_SetCmcUpAutoFreq,
    CMCONU_TK3723_GetCmcUpChannelWidth,
    CMCONU_TK3723_SetCmcUpChannelWidth,
    CMCONU_TK3723_GetCmcDownChannelAnnexMode,
    CMCONU_TK3723_SetCmcDownChannelAnnexMode,
    
    CMCONU_TK3723_GetCmcUpChannelType,
    CMCONU_TK3723_SetCmcUpChannelType,
    CMCONU_TK3723_GetCmcDownChannelModulation,
    CMCONU_TK3723_SetCmcDownChannelModulation,
    CMCONU_TK3723_GetCmcUpChannelProfile,
    
    CMCONU_TK3723_SetCmcUpChannelProfile,
    CMCONU_TK3723_GetCmcDownChannelInterleaver,
    CMCONU_TK3723_SetCmcDownChannelInterleaver,
    CMCONU_TK3723_GetCmcDownChannelPower,    
    CMCONU_TK3723_SetCmcDownChannelPower,    

    CMCONU_TK3723_GetCmcUpChannelPower,    
    CMCONU_TK3723_SetCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelSignalQuality,    
    CMCONU_TK3723_DumpCmcInterfaceUtilization,    

    CMCONU_TK3723_DumpCmcInterfaceStatistics,    
    CMCONU_TK3723_DumpCmcMacStatistics,    
    CMCONU_TK3723_DumpCmcAllInterface,    
    CMCONU_TK3723_DumpAllLoadBalancingGrp,    
    CMCONU_TK3723_DumpLoadBalancingGrp,    

    CMCONU_TK3723_DumpLoadBalancingGrpDownstream,    
    CMCONU_TK3723_DumpLoadBalancingGrpUpstream,    
    CMCONU_TK3723_DumpLoadBalancingDynConfig,    
    CMCONU_TK3723_SetLoadBalancingDynMethod,    
    CMCONU_TK3723_SetLoadBalancingDynPeriod,    

    CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_TK3723_SetLoadBalancingDynOverloadThresold,    
    CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_TK3723_SetLoadBalancingDynMinHoldTime,    

    CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_TK3723_CreateLoadBalancingGrp,    
    CMCONU_TK3723_DestroyLoadBalancingGrp,    
    CMCONU_TK3723_AddLoadBalancingGrpDownstream,    
    CMCONU_TK3723_RemoveLoadBalancingGrpDownstream,    
    CMCONU_TK3723_AddLoadBalancingGrpUpstream,    

    CMCONU_TK3723_RemoveLoadBalancingGrpUpstream,    
    CMCONU_TK3723_AddLoadBalancingGrpModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpModem,    
    CMCONU_TK3723_AddLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_TK3723_DumpLoadBalancingGrpModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpActivedModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_TK3723_DumpAllCm,    
    CMCONU_TK3723_DumpCm,    
    CMCONU_TK3723_DumpAllCmHistory,    
    CMCONU_TK3723_DumpCmHistory,    
    CMCONU_TK3723_ClearAllCmHistory,    

    CMCONU_TK3723_ResetCm,    
    CMCONU_TK3723_DumpCmDownstream,    
    CMCONU_TK3723_DumpCmUpstream,    
    CMCONU_TK3723_SetCmDownstream,    
    CMCONU_TK3723_SetCmUpstream,    

    CMCONU_TK3723_CreateCmServiceFlow,    
    CMCONU_TK3723_ModifyCmServiceFlow,    
    CMCONU_TK3723_DestroyCmServiceFlow,    
    CMCONU_TK3723_DumpCmClassifier,    
    CMCONU_TK3723_DumpCmServiceFlow,   
    
    CMCONU_TK3723_DumpCmcClassifier,    
    CMCONU_TK3723_DumpCmcServiceFlow,    
    CMCONU_TK3723_DumpCmcServiceFlowStatistics,    
    CMCONU_TK3723_DumpCmcDownChannelBondingGroup,    
    CMCONU_TK3723_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_TK3723_CreateServiceFlowClassName,    
    CMCONU_TK3723_DestroyServiceFlowClassName,    
    CMCONU_TK3723_GetCmcMacAddrTbl,    
    CMCONU_TK3723_GetCmMacAddrTbl,    
    CMCONU_TK3723_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if 0
        /*----------------GPON OMCI----------------------*/
        NULL,
        NULL,
#endif
   	DRPENG_SetOnuPortIsolation,
	DRPENG_GetOnuPortIsolation,
	DRPENG_GetOnuMacAddressTable,
       DRPENG_SetOnuPortSaveConfiguration,
       DRPENG_SetOnuLoopDetectionTime,

	DRPENG_GetOnuLoopDetectionTime,
	DRPENG_SetOnuPortMode,
	DRPENG_GetOnuPortMode,
	DRPENG_SetOnuPortStormStatus,
	DRPENG_GetOnuPortStormStatus,

	DRPENG_SetOnuDeviceLocation,
	DRPENG_GetOnuDeviceLocation,
	DRPENG_SetOnuDeviceDescription,
	DRPENG_GetOnuDeviceDescription,
	DRPENG_SetOnuDeviceName,

	DRPENG_GetOnuDeviceName,
	DRPENG_GetOnuPortMacAddressNumber,
	DRPENG_GetOnuPortLocationByMAC,
	DRPENG_GetOnuSupportExtendAttribute,
    /* --------------------END------------------------ */

    REMOTE_ERROR
};

static const OnuMgmtIFs s_ctconu3723Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_TK3723_OnuIsOnline,
    GWONU_TK3723_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_TK3723_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_TK3723_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_TK3723_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_TK3723_AuthorizeOnu,
    CTCONU_TK3723_AuthRequest,
    CTCONU_TK3723_AuthSuccess,
    CTCONU_TK3723_AuthFailure,
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    CTCONU_TK3723_SetOnuTrafficServiceMode,
    GWONU_TK3723_SetOnuPeerToPeer,
    GWONU_TK3723_SetOnuPeerToPeerForward,
    GWONU_TK3723_SetOnuBW,
    GWONU_TK3723_GetOnuSLA,

    CTCONU_TK3723_SetOnuFecMode,
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_SetUniPort,
    GWONU_TK3723_SetSlowProtocolLimit,
    GWONU_TK3723_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_TK3723_GetOnuB2PMode,
    GWONU_TK3723_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_TK3723_ResetCounters,
    GWONU_TK3723_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_TK3723_GetLLIDParams,
    GWONU_TK3723_StartEncryption,
    GWONU_TK3723_StopEncryption,
    GWONU_TK3723_SetOnuEncryptParams,
    GWONU_TK3723_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_TK3723_GetOnuMacAddrTbl,
    GWONU_TK3723_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_TK3723_SetOnuMaxMacNum,
    GWONU_TK3723_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_TK3723_GetOnuDistance,
    GWONU_TK3723_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_TK3723_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_TK3723_GetOnuPonVer,
    GWONU_TK3723_GetOnuRegisterInfo,
    GWONU_TK3723_GetOnuI2CInfo,
    GWONU_TK3723_SetOnuI2CInfo,
    
    CTCONU_TK3723_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    /*CTCONU_TK3723_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_TK3723_GetCtcVersion,
    CTCONU_TK3723_GetFirmwareVersion,
    CTCONU_TK3723_GetSerialNumber,
    CTCONU_TK3723_GetChipsetID,

    CTCONU_TK3723_GetOnuCap1,
    CTCONU_TK3723_GetOnuCap2,
    CTCONU_TK3723_GetOnuCap3,
    
    CTCONU_TK3723_UpdateOnuFirmware,
    CTCONU_TK3723_ActiveOnuFirmware,
    CTCONU_TK3723_CommitOnuFirmware,
    
    CTCONU_TK3723_StartEncrypt,
    CTCONU_TK3723_StopEncrypt,
    
    CTCONU_TK3723_GetEthPortLinkstate,
    CTCONU_TK3723_GetEthPortAdminstatus,
    CTCONU_TK3723_SetEthPortAdminstatus,
    
    CTCONU_TK3723_GetEthPortPauseEnable,
    CTCONU_TK3723_SetEthPortPauseEnable,
    
    CTCONU_TK3723_GetEthPortAutoNegotinationAdmin,
    CTCONU_TK3723_SetEthPortAutoNegotinationAdmin,
    CTCONU_TK3723_SetEthPortRestartAutoConfig,
    CTCONU_TK3723_GetEthPortAnLocalTecAbility,
    CTCONU_TK3723_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_TK3723_GetEthPortPolicing,
    CTCONU_TK3723_SetEthPortPolicing,
    
    CTCONU_TK3723_GetEthPortDownstreamPolicing,
    CTCONU_TK3723_SetEthPortDownstreamPolicing,

    CTCONU_TK3723_GetEthPortVlanConfig,
    CTCONU_TK3723_SetEthPortVlanConfig,
    CTCONU_TK3723_GetAllPortVlanConfig,
    
    CTCONU_TK3723_GetEthPortClassificationAndMarking,
    CTCONU_TK3723_SetEthPortClassificationAndMarking,
    CTCONU_TK3723_ClearEthPortClassificationAndMarking,
    
    CTCONU_TK3723_GetEthPortMulticastVlan,
    CTCONU_TK3723_SetEthPortMulticastVlan,
    CTCONU_TK3723_ClearEthPortMulticastVlan,

    CTCONU_TK3723_GetEthPortMulticastGroupMaxNumber,
    CTCONU_TK3723_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_TK3723_GetEthPortMulticastTagStrip,
    CTCONU_TK3723_SetEthPortMulticastTagStrip,
    NULL,           /* GetAllPortMulticastTagStrip */

    CTCONU_TK3723_GetEthPortMulticastTagOper,
    CTCONU_TK3723_SetEthPortMulticastTagOper,
    NULL,           /* SetObjMulticastTagOper */
    
    CTCONU_TK3723_GetMulticastControl,
    CTCONU_TK3723_SetMulticastControl,
    CTCONU_TK3723_ClearMulticastControl,
    
    CTCONU_TK3723_GetMulticastSwitch,
    CTCONU_TK3723_SetMulticastSwitch,
    
    CTCONU_TK3723_GetFastLeaveAbility,
    CTCONU_TK3723_GetFastLeaveAdminState,
    CTCONU_TK3723_SetFastLeaveAdminState,
    
    CTCONU_TK3723_GetOnuPortStatisticData,
    CTCONU_TK3723_GetOnuPortStatisticState,
    CTCONU_TK3723_SetOnuPortStatisticState,
    
    CTCONU_TK3723_SetAlarmAdminState,
    CTCONU_TK3723_SetAlarmThreshold,
    CTCONU_TK3723_GetDbaReportThresholds,
    CTCONU_TK3723_SetDbaReportThresholds,
    
    CTCONU_TK3723_GetMxuMngGlobalConfig,
    CTCONU_TK3723_SetMxuMngGlobalConfig,
    CTCONU_TK3723_GetMxuMngSnmpConfig,
    CTCONU_TK3723_SetMxuMngSnmpConfig,

    CTCONU_TK3723_GetHoldover,
    CTCONU_TK3723_SetHoldover,
    CTCONU_TK3723_GetOptTransDiag,
    CTCONU_TK3723_SetTxPowerSupplyControl,

    CTCONU_TK3723_GetFecAbility,

    CTCONU_TK3723_GetIADInfo,
    CTCONU_TK3723_GetVoipIadOperStatus,
    CTCONU_TK3723_SetVoipIadOperation,
    CTCONU_TK3723_GetVoipGlobalConfig,
    CTCONU_TK3723_SetVoipGlobalConfig,
    CTCONU_TK3723_GetVoipFaxConfig,
    CTCONU_TK3723_SetVoipFaxConfig,
    
    CTCONU_TK3723_GetVoipPortStatus,
    CTCONU_TK3723_GetVoipPort,
    CTCONU_TK3723_SetVoipPort,
    NULL,           /* GetVoipPort2 */
    NULL,           /* SetVoipPort2 */
    
    CTCONU_TK3723_GetH248Config,
    CTCONU_TK3723_SetH248Config,
    CTCONU_TK3723_GetH248UserTidConfig,
    CTCONU_TK3723_SetH248UserTidConfig,
    CTCONU_TK3723_GetH248RtpTidConfig,
    CTCONU_TK3723_SetH248RtpTidConfig,
    
    CTCONU_TK3723_GetSipConfig,
    CTCONU_TK3723_SetSipConfig,
    CTCONU_TK3723_SetSipDigitMap,
    CTCONU_TK3723_GetSipUserConfig,
    CTCONU_TK3723_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID,
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL,           /* CliCall */

    CTCONU_TK3723_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    NULL,           /* SetPasFlush */
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_TK3723_SetVlanEnable,
    CTCONU_TK3723_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_TK3723_DelVlan,
    CTCONU_TK3723_SetPortPvid,

    CTCONU_TK3723_AddVlanPort,
    CTCONU_TK3723_DelVlanPort,
    CTCONU_TK3723_SetVlanTran,
    CTCONU_TK3723_DelVlanTran,
    CTCONU_TK3723_SetVlanAgg,
    CTCONU_TK3723_DelVlanAgg,
    
    CTCONU_TK3723_SetPortQinQEnable,
    CTCONU_TK3723_AddQinQVlanTag,
    CTCONU_TK3723_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_TK3723_SetPortMode,
    CTCONU_TK3723_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_TK3723_SetPortIngressRate,
    CTCONU_TK3723_SetPortEgressRate,
    
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    
    CTCONU_TK3723_SetPortQosRule,
    CTCONU_TK3723_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_TK3723_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_TK3723_SetIgmpEnable,
    CTCONU_TK3723_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_TK3723_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    CMCONU_TK3723_RegisterCmc,
    CMCONU_TK3723_UnregisterCmc,
    CMCONU_TK3723_DumpCmc,
    CMCONU_TK3723_DumpAlarm,
    CMCONU_TK3723_DumpLog,
    
    CMCONU_TK3723_ResetCmcBoard,
    CMCONU_TK3723_GetCmcVersion,
    CMCONU_TK3723_GetCmcMaxMulticasts,
    CMCONU_TK3723_GetCmcMaxCm,
    CMCONU_TK3723_SetCmcMaxCm,
    
    CMCONU_TK3723_GetCmcTime,
    CMCONU_TK3723_SetCmcTime,
    CMCONU_TK3723_LocalCmcTime,
    CMCONU_TK3723_SetCmcCustomConfig,
    CMCONU_TK3723_DumpCmcCustomConfig,
    
    CMCONU_TK3723_DumpCmcDownChannel,
    CMCONU_TK3723_DumpCmcUpChannel,
    CMCONU_TK3723_GetCmcDownChannelMode,
    CMCONU_TK3723_SetCmcDownChannelMode,
    CMCONU_TK3723_GetCmcUpChannelMode,

    CMCONU_TK3723_SetCmcUpChannelMode,
    CMCONU_TK3723_GetCmcUpChannelD30Mode,
    CMCONU_TK3723_SetCmcUpChannelD30Mode,
    CMCONU_TK3723_GetCmcDownChannelFreq,
    CMCONU_TK3723_SetCmcDownChannelFreq,

    CMCONU_TK3723_GetCmcUpChannelFreq,
    CMCONU_TK3723_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_TK3723_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_TK3723_SetCmcUpAutoFreq,
    CMCONU_TK3723_GetCmcUpChannelWidth,
    CMCONU_TK3723_SetCmcUpChannelWidth,
    CMCONU_TK3723_GetCmcDownChannelAnnexMode,
    CMCONU_TK3723_SetCmcDownChannelAnnexMode,
    
    CMCONU_TK3723_GetCmcUpChannelType,
    CMCONU_TK3723_SetCmcUpChannelType,
    CMCONU_TK3723_GetCmcDownChannelModulation,
    CMCONU_TK3723_SetCmcDownChannelModulation,
    CMCONU_TK3723_GetCmcUpChannelProfile,
    
    CMCONU_TK3723_SetCmcUpChannelProfile,
    CMCONU_TK3723_GetCmcDownChannelInterleaver,
    CMCONU_TK3723_SetCmcDownChannelInterleaver,
    CMCONU_TK3723_GetCmcDownChannelPower,    
    CMCONU_TK3723_SetCmcDownChannelPower,    

    CMCONU_TK3723_GetCmcUpChannelPower,    
    CMCONU_TK3723_SetCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelSignalQuality,    
    CMCONU_TK3723_DumpCmcInterfaceUtilization,    

    CMCONU_TK3723_DumpCmcInterfaceStatistics,    
    CMCONU_TK3723_DumpCmcMacStatistics,    
    CMCONU_TK3723_DumpCmcAllInterface,    
    CMCONU_TK3723_DumpAllLoadBalancingGrp,    
    CMCONU_TK3723_DumpLoadBalancingGrp,    

    CMCONU_TK3723_DumpLoadBalancingGrpDownstream,    
    CMCONU_TK3723_DumpLoadBalancingGrpUpstream,    
    CMCONU_TK3723_DumpLoadBalancingDynConfig,    
    CMCONU_TK3723_SetLoadBalancingDynMethod,    
    CMCONU_TK3723_SetLoadBalancingDynPeriod,    

    CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_TK3723_SetLoadBalancingDynOverloadThresold,    
    CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_TK3723_SetLoadBalancingDynMinHoldTime,    

    CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_TK3723_CreateLoadBalancingGrp,    
    CMCONU_TK3723_DestroyLoadBalancingGrp,    
    CMCONU_TK3723_AddLoadBalancingGrpDownstream,    
    CMCONU_TK3723_RemoveLoadBalancingGrpDownstream,    
    CMCONU_TK3723_AddLoadBalancingGrpUpstream,    

    CMCONU_TK3723_RemoveLoadBalancingGrpUpstream,    
    CMCONU_TK3723_AddLoadBalancingGrpModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpModem,    
    CMCONU_TK3723_AddLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_TK3723_DumpLoadBalancingGrpModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpActivedModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_TK3723_DumpAllCm,    
    CMCONU_TK3723_DumpCm,    
    CMCONU_TK3723_DumpAllCmHistory,    
    CMCONU_TK3723_DumpCmHistory,    
    CMCONU_TK3723_ClearAllCmHistory,    

    CMCONU_TK3723_ResetCm,    
    CMCONU_TK3723_DumpCmDownstream,    
    CMCONU_TK3723_DumpCmUpstream,    
    CMCONU_TK3723_SetCmDownstream,    
    CMCONU_TK3723_SetCmUpstream,    

    CMCONU_TK3723_CreateCmServiceFlow,    
    CMCONU_TK3723_ModifyCmServiceFlow,    
    CMCONU_TK3723_DestroyCmServiceFlow,    
    CMCONU_TK3723_DumpCmClassifier,    
    CMCONU_TK3723_DumpCmServiceFlow,    
    
    CMCONU_TK3723_DumpCmcClassifier,    
    CMCONU_TK3723_DumpCmcServiceFlow,    
    CMCONU_TK3723_DumpCmcServiceFlowStatistics,    
    CMCONU_TK3723_DumpCmcDownChannelBondingGroup,    
    CMCONU_TK3723_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_TK3723_CreateServiceFlowClassName,    
    CMCONU_TK3723_DestroyServiceFlowClassName,    
    CMCONU_TK3723_GetCmcMacAddrTbl,    
    CMCONU_TK3723_GetCmMacAddrTbl,    
    CMCONU_TK3723_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
#if 0
    /*----------------GPON OMCI----------------------*/
    NULL,
    NULL,
#endif
	DRPENG_SetOnuPortIsolation,
	DRPENG_GetOnuPortIsolation,
	DRPENG_GetOnuMacAddressTable,
       DRPENG_SetOnuPortSaveConfiguration,
       DRPENG_SetOnuLoopDetectionTime,

	DRPENG_GetOnuLoopDetectionTime,
	DRPENG_SetOnuPortMode,
	DRPENG_GetOnuPortMode,
	DRPENG_SetOnuPortStormStatus,
	DRPENG_GetOnuPortStormStatus,

	DRPENG_SetOnuDeviceLocation,
	DRPENG_GetOnuDeviceLocation,
	DRPENG_SetOnuDeviceDescription,
	DRPENG_GetOnuDeviceDescription,
	DRPENG_SetOnuDeviceName,

	DRPENG_GetOnuDeviceName,
	DRPENG_GetOnuPortMacAddressNumber,
	DRPENG_GetOnuPortLocationByMAC,
	DRPENG_GetOnuSupportExtendAttribute,
/* --------------------END------------------------ */

    REMOTE_ERROR
};

static const OnuMgmtIFs s_ctconu55524Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_TK3723_OnuIsOnline,
    GWONU_TK3723_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_TK3723_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_BCM55524_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_TK3723_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_TK3723_AuthorizeOnu,
    CTCONU_TK3723_AuthRequest,
    CTCONU_TK3723_AuthSuccess,
    CTCONU_TK3723_AuthFailure,
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    CTCONU_TK3723_SetOnuTrafficServiceMode,
    GWONU_TK3723_SetOnuPeerToPeer,
    GWONU_TK3723_SetOnuPeerToPeerForward,
    GWONU_TK3723_SetOnuBW,
    GWONU_TK3723_GetOnuSLA,

    CTCONU_TK3723_SetOnuFecMode,
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_TK3723_SetUniPort,
    GWONU_TK3723_SetSlowProtocolLimit,
    GWONU_TK3723_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_TK3723_GetOnuB2PMode,
    GWONU_TK3723_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_TK3723_ResetCounters,
    GWONU_TK3723_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_TK3723_GetLLIDParams,
    GWONU_TK3723_StartEncryption,
    GWONU_TK3723_StopEncryption,
    GWONU_TK3723_SetOnuEncryptParams,
    GWONU_TK3723_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_TK3723_GetOnuMacAddrTbl,
    GWONU_TK3723_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_TK3723_SetOnuMaxMacNum,
    GWONU_TK3723_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_TK3723_GetOnuDistance,
    GWONU_TK3723_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_TK3723_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_TK3723_GetOnuPonVer,
    GWONU_TK3723_GetOnuRegisterInfo,
    GWONU_TK3723_GetOnuI2CInfo,
    GWONU_TK3723_SetOnuI2CInfo,
    
    CTCONU_TK3723_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    GWONU_TK3723_GetBurnImageComplete,
    
    /*CTCONU_TK3723_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_TK3723_GetCtcVersion,
    CTCONU_TK3723_GetFirmwareVersion,
    CTCONU_TK3723_GetSerialNumber,
    CTCONU_TK3723_GetChipsetID,

    CTCONU_TK3723_GetOnuCap1,
    CTCONU_TK3723_GetOnuCap2,
    CTCONU_TK3723_GetOnuCap3,
    
    CTCONU_TK3723_UpdateOnuFirmware,
    CTCONU_TK3723_ActiveOnuFirmware,
    CTCONU_TK3723_CommitOnuFirmware,
    
    CTCONU_TK3723_StartEncrypt,
    CTCONU_TK3723_StopEncrypt,
    
    CTCONU_TK3723_GetEthPortLinkstate,
    CTCONU_TK3723_GetEthPortAdminstatus,
    CTCONU_TK3723_SetEthPortAdminstatus,
    
    CTCONU_TK3723_GetEthPortPauseEnable,
    CTCONU_TK3723_SetEthPortPauseEnable,
    
    CTCONU_TK3723_GetEthPortAutoNegotinationAdmin,
    CTCONU_TK3723_SetEthPortAutoNegotinationAdmin,
    CTCONU_TK3723_SetEthPortRestartAutoConfig,
    CTCONU_TK3723_GetEthPortAnLocalTecAbility,
    CTCONU_TK3723_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_TK3723_GetEthPortPolicing,
    CTCONU_TK3723_SetEthPortPolicing,
    
    CTCONU_TK3723_GetEthPortDownstreamPolicing,
    CTCONU_TK3723_SetEthPortDownstreamPolicing,

    CTCONU_TK3723_GetEthPortVlanConfig,
    CTCONU_TK3723_SetEthPortVlanConfig,
    CTCONU_TK3723_GetAllPortVlanConfig,
    
    CTCONU_TK3723_GetEthPortClassificationAndMarking,
    CTCONU_TK3723_SetEthPortClassificationAndMarking,
    CTCONU_TK3723_ClearEthPortClassificationAndMarking,
    
    CTCONU_TK3723_GetEthPortMulticastVlan,
    CTCONU_TK3723_SetEthPortMulticastVlan,
    CTCONU_TK3723_ClearEthPortMulticastVlan,

    CTCONU_TK3723_GetEthPortMulticastGroupMaxNumber,
    CTCONU_TK3723_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_TK3723_GetEthPortMulticastTagStrip,
    CTCONU_TK3723_SetEthPortMulticastTagStrip,
    NULL,           /* GetAllPortMulticastTagStrip */

    CTCONU_TK3723_GetEthPortMulticastTagOper,
    CTCONU_TK3723_SetEthPortMulticastTagOper,
    NULL,           /* SetObjMulticastTagOper */
    
    CTCONU_TK3723_GetMulticastControl,
    CTCONU_TK3723_SetMulticastControl,
    CTCONU_TK3723_ClearMulticastControl,
    
    CTCONU_TK3723_GetMulticastSwitch,
    CTCONU_TK3723_SetMulticastSwitch,
    
    CTCONU_TK3723_GetFastLeaveAbility,
    CTCONU_TK3723_GetFastLeaveAdminState,
    CTCONU_TK3723_SetFastLeaveAdminState,
    
    CTCONU_TK3723_GetOnuPortStatisticData,
    CTCONU_TK3723_GetOnuPortStatisticState,
    CTCONU_TK3723_SetOnuPortStatisticState,
    
    CTCONU_TK3723_SetAlarmAdminState,
    CTCONU_TK3723_SetAlarmThreshold,
    CTCONU_TK3723_GetDbaReportThresholds,
    CTCONU_TK3723_SetDbaReportThresholds,
    
    CTCONU_TK3723_GetMxuMngGlobalConfig,
    CTCONU_TK3723_SetMxuMngGlobalConfig,
    CTCONU_TK3723_GetMxuMngSnmpConfig,
    CTCONU_TK3723_SetMxuMngSnmpConfig,

    CTCONU_TK3723_GetHoldover,
    CTCONU_TK3723_SetHoldover,
    CTCONU_TK3723_GetOptTransDiag,
    CTCONU_TK3723_SetTxPowerSupplyControl,

    CTCONU_TK3723_GetFecAbility,

    CTCONU_TK3723_GetIADInfo,
    CTCONU_TK3723_GetVoipIadOperStatus,
    CTCONU_TK3723_SetVoipIadOperation,
    CTCONU_TK3723_GetVoipGlobalConfig,
    CTCONU_TK3723_SetVoipGlobalConfig,
    CTCONU_TK3723_GetVoipFaxConfig,
    CTCONU_TK3723_SetVoipFaxConfig,
    
    CTCONU_TK3723_GetVoipPortStatus,
    NULL,           /* GetVoipPort */
    NULL,           /* SetVoipPort */
    NULL,           /* GetVoipPort2 */
    NULL,           /* SetVoipPort2 */
    
    CTCONU_TK3723_GetH248Config,
    CTCONU_TK3723_SetH248Config,
    CTCONU_TK3723_GetH248UserTidConfig,
    CTCONU_TK3723_SetH248UserTidConfig,
    CTCONU_TK3723_GetH248RtpTidConfig,
    CTCONU_TK3723_SetH248RtpTidConfig,
    
    CTCONU_TK3723_GetSipConfig,
    CTCONU_TK3723_SetSipConfig,
    CTCONU_TK3723_SetSipDigitMap,
    CTCONU_TK3723_GetSipUserConfig,
    CTCONU_TK3723_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID,
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL,           /* CliCall */

    CTCONU_TK3723_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    NULL,           /* SetPasFlush */
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_TK3723_SetVlanEnable,
    CTCONU_TK3723_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_TK3723_DelVlan,
    CTCONU_TK3723_SetPortPvid,

    CTCONU_TK3723_AddVlanPort,
    CTCONU_TK3723_DelVlanPort,
    CTCONU_TK3723_SetVlanTran,
    CTCONU_TK3723_DelVlanTran,
    CTCONU_TK3723_SetVlanAgg,
    CTCONU_TK3723_DelVlanAgg,
    
    CTCONU_TK3723_SetPortQinQEnable,
    CTCONU_TK3723_AddQinQVlanTag,
    CTCONU_TK3723_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_TK3723_SetPortMode,
    CTCONU_TK3723_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_TK3723_SetPortIngressRate,
    CTCONU_TK3723_SetPortEgressRate,
    
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    
    CTCONU_TK3723_SetPortQosRule,
    CTCONU_TK3723_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_TK3723_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_TK3723_SetIgmpEnable,
    CTCONU_TK3723_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_TK3723_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    CMCONU_TK3723_RegisterCmc,
    CMCONU_TK3723_UnregisterCmc,
    CMCONU_TK3723_DumpCmc,
    CMCONU_TK3723_DumpAlarm,
    CMCONU_TK3723_DumpLog,
    
    CMCONU_TK3723_ResetCmcBoard,
    CMCONU_TK3723_GetCmcVersion,
    CMCONU_TK3723_GetCmcMaxMulticasts,
    CMCONU_TK3723_GetCmcMaxCm,
    CMCONU_TK3723_SetCmcMaxCm,
    
    CMCONU_TK3723_GetCmcTime,
    CMCONU_TK3723_SetCmcTime,
    CMCONU_TK3723_LocalCmcTime,
    CMCONU_TK3723_SetCmcCustomConfig,
    CMCONU_TK3723_DumpCmcCustomConfig,
    
    CMCONU_TK3723_DumpCmcDownChannel,
    CMCONU_TK3723_DumpCmcUpChannel,
    CMCONU_TK3723_GetCmcDownChannelMode,
    CMCONU_TK3723_SetCmcDownChannelMode,
    CMCONU_TK3723_GetCmcUpChannelMode,

    CMCONU_TK3723_SetCmcUpChannelMode,
    CMCONU_TK3723_GetCmcUpChannelD30Mode,
    CMCONU_TK3723_SetCmcUpChannelD30Mode,
    CMCONU_TK3723_GetCmcDownChannelFreq,
    CMCONU_TK3723_SetCmcDownChannelFreq,

    CMCONU_TK3723_GetCmcUpChannelFreq,
    CMCONU_TK3723_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_TK3723_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_TK3723_SetCmcUpAutoFreq,
    CMCONU_TK3723_GetCmcUpChannelWidth,
    CMCONU_TK3723_SetCmcUpChannelWidth,
    CMCONU_TK3723_GetCmcDownChannelAnnexMode,
    CMCONU_TK3723_SetCmcDownChannelAnnexMode,
    
    CMCONU_TK3723_GetCmcUpChannelType,
    CMCONU_TK3723_SetCmcUpChannelType,
    CMCONU_TK3723_GetCmcDownChannelModulation,
    CMCONU_TK3723_SetCmcDownChannelModulation,
    CMCONU_TK3723_GetCmcUpChannelProfile,
    
    CMCONU_TK3723_SetCmcUpChannelProfile,
    CMCONU_TK3723_GetCmcDownChannelInterleaver,
    CMCONU_TK3723_SetCmcDownChannelInterleaver,
    CMCONU_TK3723_GetCmcDownChannelPower,    
    CMCONU_TK3723_SetCmcDownChannelPower,    

    CMCONU_TK3723_GetCmcUpChannelPower,    
    CMCONU_TK3723_SetCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelPower,    
    CMCONU_TK3723_DumpCmcUpChannelSignalQuality,    
    CMCONU_TK3723_DumpCmcInterfaceUtilization,    

    CMCONU_TK3723_DumpCmcInterfaceStatistics,    
    CMCONU_TK3723_DumpCmcMacStatistics,    
    CMCONU_TK3723_DumpCmcAllInterface,    
    CMCONU_TK3723_DumpAllLoadBalancingGrp,    
    CMCONU_TK3723_DumpLoadBalancingGrp,    

    CMCONU_TK3723_DumpLoadBalancingGrpDownstream,    
    CMCONU_TK3723_DumpLoadBalancingGrpUpstream,    
    CMCONU_TK3723_DumpLoadBalancingDynConfig,    
    CMCONU_TK3723_SetLoadBalancingDynMethod,    
    CMCONU_TK3723_SetLoadBalancingDynPeriod,    

    CMCONU_TK3723_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_TK3723_SetLoadBalancingDynOverloadThresold,    
    CMCONU_TK3723_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_TK3723_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_TK3723_SetLoadBalancingDynMinHoldTime,    

    CMCONU_TK3723_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_TK3723_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_TK3723_CreateLoadBalancingGrp,    
    CMCONU_TK3723_DestroyLoadBalancingGrp,    
    CMCONU_TK3723_AddLoadBalancingGrpDownstream,    
    CMCONU_TK3723_RemoveLoadBalancingGrpDownstream,    
    CMCONU_TK3723_AddLoadBalancingGrpUpstream,    

    CMCONU_TK3723_RemoveLoadBalancingGrpUpstream,    
    CMCONU_TK3723_AddLoadBalancingGrpModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpModem,    
    CMCONU_TK3723_AddLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_TK3723_DumpLoadBalancingGrpModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpActivedModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_TK3723_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_TK3723_DumpAllCm,    
    CMCONU_TK3723_DumpCm,    
    CMCONU_TK3723_DumpAllCmHistory,    
    CMCONU_TK3723_DumpCmHistory,    
    CMCONU_TK3723_ClearAllCmHistory,    

    CMCONU_TK3723_ResetCm,    
    CMCONU_TK3723_DumpCmDownstream,    
    CMCONU_TK3723_DumpCmUpstream,    
    CMCONU_TK3723_SetCmDownstream,    
    CMCONU_TK3723_SetCmUpstream,    

    CMCONU_TK3723_CreateCmServiceFlow,    
    CMCONU_TK3723_ModifyCmServiceFlow,    
    CMCONU_TK3723_DestroyCmServiceFlow,    
    CMCONU_TK3723_DumpCmClassifier,    
    CMCONU_TK3723_DumpCmServiceFlow,    
    
    CMCONU_TK3723_DumpCmcClassifier,    
    CMCONU_TK3723_DumpCmcServiceFlow,    
    CMCONU_TK3723_DumpCmcServiceFlowStatistics,    
    CMCONU_TK3723_DumpCmcDownChannelBondingGroup,    
    CMCONU_TK3723_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_TK3723_CreateServiceFlowClassName,    
    CMCONU_TK3723_DestroyServiceFlowClassName,    
    CMCONU_TK3723_GetCmcMacAddrTbl,    
    CMCONU_TK3723_GetCmMacAddrTbl,    
    CMCONU_TK3723_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
#if 0
    /*----------------GPON OMCI----------------------*/
    NULL,
    NULL,
#endif

	DRPENG_SetOnuPortIsolation,
	DRPENG_GetOnuPortIsolation,
	DRPENG_GetOnuMacAddressTable,
       DRPENG_SetOnuPortSaveConfiguration,
       DRPENG_SetOnuLoopDetectionTime,

	DRPENG_GetOnuLoopDetectionTime,
	DRPENG_SetOnuPortMode,
	DRPENG_GetOnuPortMode,
	DRPENG_SetOnuPortStormStatus,
	DRPENG_GetOnuPortStormStatus,

	DRPENG_SetOnuDeviceLocation,
	DRPENG_GetOnuDeviceLocation,
	DRPENG_SetOnuDeviceDescription,
	DRPENG_GetOnuDeviceDescription,
	DRPENG_SetOnuDeviceName,

	DRPENG_GetOnuDeviceName,
	DRPENG_GetOnuPortMacAddressNumber,
	DRPENG_GetOnuPortLocationByMAC,
	DRPENG_GetOnuSupportExtendAttribute,
/* --------------------END------------------------ */

    REMOTE_ERROR
};


void GWONU_TK3723_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_TK3723_GW, &s_gwonu3723Ifs);
}

void GWONU_BCM55524_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_BCM55524_GW, &s_gwonu55524Ifs);
}


void CTCONU_TK3723_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_TK3723_CTC, &s_ctconu3723Ifs);
}

void CTCONU_BCM55524_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_BCM55524_CTC, &s_ctconu55524Ifs);
}



#ifdef __cplusplus

}

#endif
