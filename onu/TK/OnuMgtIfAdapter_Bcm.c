/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_Bcm.c
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

#if defined(_EPON_10G_BCM_SUPPORT_)            

#ifdef __cplusplus
extern "C"
  {
#endif


#include  "syscfg.h"
#include  "vos/vospubh/vos_base.h"
#include  "vos/vospubh/vos_sem.h"
#include  "includefromPas.h"
#include  "includefromTk.h"
#include  "includefromBcm.h"
#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include  "PonGeneral.h"
#include  "cli/cl_vty.h"
#include "onu/onuOamUpd.h"
#include "onu/onuConfMgt.h"
#include "onu/Onu_oam_comm.h"
#include "ct_manage/CT_Onu_Voip.h"
#include "../onu/OnuPortStatsMgt.h"


#define CMC_CHANNELID_2_IFINDEX(channel_type, channel_id)  ((channel_type) * 100 + (channel_id))


extern PON_onu_address_table_record_t  MAC_Address_Table[PON_ADDRESS_TABLE_SIZE];
extern int ctc_voip_debug ;
#define CTC_VOIP_DEBUG if(ctc_voip_debug) sys_console_printf

/****************************************内部适配*******************************************/

extern CHAR *macAddress_To_Strings(UCHAR *pMacAddr);
static int GWONU_BCM55538_DeregisterOnu(short int olt_id, short int onu_id);

#if 1
/* -------------------ONU基本API------------------- */

static int REMOTE_OK(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_BCM_DEBUG(OLT_BCM_TITLE"REMOTE_OK(%d, %d)'s result(0).\r\n", olt_id, onu_id);

    return OLT_ERR_OK;
}

static int REMOTE_ERROR(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_BCM_DEBUG(OLT_BCM_TITLE"REMOTE_ERROR(%d, %d)'s result(%d).\r\n", olt_id, onu_id, OLT_ERR_NOTEXIST);

    return OLT_ERR_NOTEXIST;
}

static int GWONU_BCM55538_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    int iRlt = 0;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    *status = 0;
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        *status = BcmOnuIsExist(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_OnuIsOnline(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* ONU 手动添加*/
static int GWONU_BCM55538_AddOnuByManual(short int olt_id, short int onu_id, unsigned char *MacAddr)
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
	    GWONU_BCM55538_DeregisterOnu(olt_id, onu_id);
        OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_AddOnuByManual(%d,%d) deregister onu(%s) for onu(%s) on slot %d.\r\n", olt_id, onu_id, macAddress_To_Strings(pSrcMac), macAddress_To_Strings(MacAddr), SYS_LOCAL_MODULE_SLOTNO);
    }   

    return( ROK );
}

/* 删除ONU */
static int GWONU_BCM55538_DelOnuByManual(short int olt_id, short int onu_id)
{	     
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( activeOneLocalPendingOnu(olt_id, onu_id) == RERROR )
	{
	    GWONU_BCM55538_DeregisterOnu(olt_id, onu_id);
	}

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return ROK;
}

static int GWONU_BCM55538_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_BCM55538;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_BCM55538_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_BCM55538;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
/*onu deregister*/
static int GWONU_BCM55538_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = BcmOLT_DeregisterLink(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_DeregisterOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_BCM55538_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned long flags;
    short int llid;
    bcmEmmiLinkInfo link_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info )) )
        {
            if ( 0 == (iRlt = BCM_authorize_llid(olt_id, llid, auth_mode)) )
            {
                switch ( link_info.rate )
                {
                    case bcmEmmiEponRateTenTen:
                        flags = PON_RATE_10_10G;
                    break;    
                    case bcmEmmiEponRateTenOne:
                        flags = PON_RATE_1_10G;
                    break;    
                    case bcmEmmiEponRateOneOne:
                        flags = PON_RATE_NORMAL_1G;
                    break;    
                    default:
                        flags = PON_RATE_NORMAL_1G;
                        VOS_ASSERT(0);
                }
            
                iRlt = pon_pp_add_link(olt_id, llid, link_info.tunnelId.upstreamId, link_info.tunnelId.downstreamId, flags);
            }
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_AuthorizeOnu(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(auth_response);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_auth_request(olt_id, llid, auth_response);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_AuthRequest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_auth_success(olt_id, llid);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_AuthSuccess(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_auth_failure(olt_id, llid, failure_type);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_AuthFailure(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, failure_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
/*traffic service enable*/
/* 5001和5201共用此函数*//*问题单11283*/
static int GWONU_BCM55538_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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
         iRlt = GWONU_BCM55538_DeregisterOnu(olt_id, onu_id);
	    }
    }
    else
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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
        iRlt = GWONU_BCM55538_DeregisterOnu(olt_id, onu_id);
    }
    else 
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P */
static int GWONU_BCM55538_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
#if 0
	short int Llid1, Llid2;
    short int onuid;
	short int onuArray[1];
    bool access;

	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id1);
	ONU_ASSERT(onu_id2);

	Llid1 = GetLlidActivatedByOnuIdx(olt_id, onu_id1);
	Llid2 = GetLlidActivatedByOnuIdx(olt_id, onu_id2);
	if((Llid1 != INVALID_LLID) && (Llid2 != INVALID_LLID))
    {
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, Llid1, &onuid)) )
        {
            if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, Llid2, &onuArray[0])) )
            {
                access = (V2R1_ENABLE == enable) ? TRUE : FALSE;
            	iRlt = BhfOnuSetP2PAccessList( olt_id, onuid, 1, onuArray, access, FALSE );
            }
        }
    }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id1, onu_id2, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P的ONU，是否转发无效MAC */
static int GWONU_BCM55538_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast)
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
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, address_not_found, broadcast, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GWONU_BCM55538_GetSLA(int DIR, ONU_bw_t *BW, BcmSlaQueueParams *SLA)
{
    bcmEmmiSchedulerSolicited *min; /* < Min shaper settings */
    bcmEmmiSchedulerSolicited *max; /* < Max shaper settings */

    VOS_ASSERT(BW);
    VOS_ASSERT(SLA);
    
    min = &SLA->min;
    max = &SLA->max;

    if ( 0 == BW->bw_gr )
    {
        min->burstSize = 256;
        min->weight = 1;
        min->level = 0;
    }
    else
    {
        if ( BW->bw_gr < 64 )
        {
            BW->bw_gr = 64;
        }
        
        min->level = (BW->bw_class < 8) ? BW->bw_class : 7;
        
        if ( OLT_CFG_DIR_UPLINK == DIR )
        {
            min->weight = uplinkBWWeight;
            
            if ( 256 < uplinkBWlimitBurstSize )
            {
                min->burstSize = 256;
            }
            else
            {
                min->burstSize = uplinkBWlimitBurstSize;
            }
        }
        else
        {
            min->weight = downlinkBWWeight;  /* 1-253 */
            
            if ( 256 < downlinkBWlimitBurstSize )
            {
                min->burstSize = 256;
            }
            else
            {
                min->burstSize = downlinkBWlimitBurstSize;
            }
        }
    }

    if ( 0 == BW->bw_be )
    {
        max->burstSize = 256;
        max->weight = 1;
        max->level = 0;
    }
    else
    {
        if ( BW->bw_be < 64 )
        {
            BW->bw_be = 64;
        }
        
        max->level = min->level;
        max->weight = min->weight;
        max->burstSize = min->burstSize;
    }

    min->bandwidth = BW->bw_gr;
    max->bandwidth = BW->bw_be;

    return 0;
}

static int GWONU_BCM55538_GetDefaultSLA(short int olt_id, short int llid, int DIR, BcmSlaQueueParams *SLA)
{
    int iRlt;
    bcmEmmiLinkInfo link_info;

    VOS_ASSERT(SLA);

    if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info)) )
    {
        U32 uiBandWidthUpMax, uiBandWidthDnMax;
        bcmEmmiSchedulerSolicited *min; /* < Min shaper settings */
        bcmEmmiSchedulerSolicited *max; /* < Max shaper settings */
        
        switch ( link_info.rate )
        {
            case bcmEmmiEponRateTenTen:
                uiBandWidthUpMax = 10000000;
                uiBandWidthDnMax = 10000000;
            break;    
            case bcmEmmiEponRateTenOne:
                uiBandWidthUpMax = 1000000;
                uiBandWidthDnMax = 10000000;
            break;    
            case bcmEmmiEponRateOneOne:
                uiBandWidthUpMax = 1000000;
                uiBandWidthDnMax = 1000000;
            break;    
            default:
                uiBandWidthUpMax = 1000000;
                uiBandWidthDnMax = 1000000;
                
                VOS_ASSERT(0);
        }

        min = &SLA->min;
        max = &SLA->max;

        min->level = 0;
        min->weight = 1;
        min->burstSize = 256;
        min->bandwidth = 0;

        max->level = 0;
        max->weight = 1;
        max->burstSize = 256;
        
        if ( OLT_CFG_DIR_DOWNLINK == DIR )
        {
            max->bandwidth = uiBandWidthDnMax;
        }
        else
        {
            max->bandwidth = uiBandWidthUpMax;
        }
    }

    return iRlt;
}

extern int DBA_POLLING_LEVEL;
static int GWONU_BCM55538_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    BcmLinkSlaDbaInfo  SLA;

    if ( 0 == (iRlt = BcmOnuGetSLA(olt_id, llid, &SLA)) )
    {
        if ( 0 == (iRlt = GWONU_BCM55538_GetSLA(OLT_CFG_DIR_UPLINK, BW, &SLA.sla)) )
        {
            BcmSlaDbaParams *DBA = &SLA.dba;
        
            if ( 0 == BW->bw_fixed )
            {
                DBA->dba_flags = bcmEmmiSchedulerFlagsForceReport;
                DBA->polling_level = DBA_POLLING_LEVEL;

                VOS_MemZero(&DBA->tdm, sizeof(DBA->tdm));
            }
            else
            {
                bcmEmmiSchedulerTdm *TDM = &DBA->tdm;
                U32 FixGrantBW;
                U32 FixGrantRate;
                U32 FixGrantLenB;
                U16 FixGrantTQ;

                /* 先由配置，确认Grant尺寸 */
                if ( (0xFFFF << 1) >= (FixGrantLenB = uplinkBWPacketUnitSize) )
                {
                    if ( 4 > (FixGrantTQ = FixGrantLenB >> 1) )
                    {
                        FixGrantTQ = 4;
                    }
                }
                else
                {
                    FixGrantLenB = (0xFFFF << 1);
                    FixGrantTQ = 0xFFFF;
                }
                    
                TDM->length = FixGrantTQ;
                TDM->extraGrantLength = 0;

                if ( 0 == TDM->extraGrantLength )
                {
                    DBA->dba_flags = bcmEmmiSchedulerFlagsForceReport;
                }
                else
                {
                    DBA->dba_flags = bcmEmmiSchedulerFlagsForceReportExtraGrant;
                }
                DBA->polling_level = DBA_POLLING_LEVEL;
                
                /* 再从固定带宽配置，算出Grant周期 */
                FixGrantBW = BW->bw_fixed;
                FixGrantRate = (FixGrantLenB * 1000) / (12 * FixGrantBW);
                if ( 0 == FixGrantRate )
                {
                    FixGrantRate = 1;
                }

                TDM->interval = FixGrantRate;
            }

			/*added by liyang @2015-03-06 for uplink bandwidth */

			SLA.sla.max.bandwidth = SLA.sla.max.bandwidth - SLA.sla.min.bandwidth;
#if 0
			if(SLA.sla.max.bandwidth + SLA.sla.min.bandwidth < 168000)
				SLA.dba.token_size = 1024;
#endif
			
            iRlt = BcmOnuSetSLA(olt_id, llid, &SLA);
        }
    }
        
    return iRlt;
}

static int GWONU_BCM55538_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    BcmSlaQueueParams sla;

    if( (BW->bw_gr > 0)
        && ((PonPortTable[olt_id].DownlinkPoliceMode == V2R1_ENABLE)
        || (OLT_CFG_DIR_INACTIVE & BW->bw_direction)) )
    {
        if ( 0 == (iRlt = GWONU_BCM55538_GetSLA(OLT_CFG_DIR_DOWNLINK, BW, &sla)) )
        {
            iRlt = pon_pp_set_link_sla(olt_id, llid, PON_BRIDGE_PATH_S_DOWNLINK, &sla);
            if (0 == iRlt)
            {
                /* 输出激活带宽 */
                BW->bw_actived = BW->bw_gr;
            }
        }
    }
    else
    {
        if ( 0 == (iRlt = GWONU_BCM55538_GetDefaultSLA(olt_id, llid, OLT_CFG_DIR_DOWNLINK, &sla)) )
        {
            iRlt = pon_pp_set_link_sla(olt_id, llid, PON_BRIDGE_PATH_S_DOWNLINK, &sla);
            if (0 == iRlt)
            {
                /* 输出激活带宽 */
                BW->bw_actived = 0;
            }
        }
    }

    return iRlt;
}

static int  GWONU_BCM55538_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
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
                iRlt = GWONU_BCM55538_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
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

                    iRlt = GWONU_BCM55538_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);
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
                                (void)GWONU_BCM55538_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
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
                
                iRlt = GWONU_BCM55538_SetOnuUplinkBW(olt_id, llid, BW);
                if (0 == iRlt)
                {
                    /* 输出激活带宽 */
                    BW->bw_actived = BW->bw_gr;
                }
#if 0
                else
                {
                    /* B--added by liwei056@2012-8-22 for OnuDefaultBW's ShowRealAllocBW */
                    if ( OLT_CFG_DIR_FORCE & BW->bw_direction )
                    {
                        BW->bw_actived = pstOnuCfg->ActiveUplinkBandwidth + pstOnuCfg->FinalUplinkBandwidth_fixed;
                        iRlt = 0;
                    }
                    /* E--added by liwei056@2012-8-22 for OnuDefaultBW's ShowRealAllocBW */
                }
#endif
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
                
                iRlt = GWONU_BCM55538_SetOnuDownlinkBW(olt_id, llid, BW);
            }
        }
    }   
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int GWONU_BCM55538_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = BcmOnuGetSLA(olt_id, llid, &sla_info->SLA.SLA6)) )
        {
            sla_info->SLA_Ver = 6;
            sla_info->DBA_ErrCode = 0;
        }
    }   
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuSLA(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, sla_info->SLA_Ver, sla_info->DBA_ErrCode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_BCM55538_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int CTCONU_BCM55538_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}


static int GWONU_BCM55538_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = RM_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath );
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetUniPort(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable_cpu, enable_datapath, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_BCM55538_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iPonChipVendorID;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        switch ( iPonChipVendorID = GetOnuChipVendor(olt_id, onu_id) )
        {
            case PONCHIP_VENDOR_PAS:
                iRlt = RM_PASONU_set_slow_protocol_limit( olt_id, llid, enable );
                break;    
            default:
                iRlt = OLT_ERR_NOTSUPPORT;
        }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_BCM55538_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iPonChipVendorID;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        switch ( iPonChipVendorID = GetOnuChipVendor(olt_id, onu_id) )
        {
            case PONCHIP_VENDOR_PAS:
                iRlt = RM_PASONU_get_slow_protocol_limit( olt_id, llid, enable );
                break;
            default:
                iRlt = OLT_ERR_NOTSUPPORT;
        }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_BCM55538_GetOnuB2PMode(short int olt_id, short int onu_id, int *b2p_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 <= (iRlt = BhfBridgeGetOnuDataPathNum(olt_id, llid)) )
        {
            *b2p_mode = (1 < iRlt) ? TRUE : FALSE;
            iRlt = 0;
        }
    }
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetOnuB2PMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int GWONU_BCM55538_SetOnuB2PMode(short int olt_id, short int onu_id, int b2p_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 < (iRlt = BhfBridgeSetOnuDataPathNum(olt_id, llid, (b2p_mode) ? 2 : 1)) )
        {
            iRlt = 0;
        }
    }
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuB2PMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, b2p_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */

static int GWONU_BCM55538_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
#if 0
        if ( 0 == (iRlt = TkLinkGetOnuID(olt_id, llid, &onuid)) )
        {
        	if ( 0 == (iRlt = TkONU_ResetAllCounter ( olt_id, onuid )) )
            {
                TkOLT_ResetAllCounter(olt_id);
            }   
        }   
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_ResetCounters(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_BCM55538_SetPonLoopback(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetPonLoopback(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU加密管理API------------------- */
/* get onu pon Llid parameters */
static int GWONU_BCM55538_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetLLIDParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*onu start encryption */
static int GWONU_BCM55538_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
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
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_StartEncryption(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

/*onu stop encryption*/
static int GWONU_BCM55538_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
#if 0
    	iRlt = TkOLT_DeleteLinkEncryptMode ( olt_id, llid );
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_StopEncryption(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

static int GWONU_BCM55538_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
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
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, key_change_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

static int GWONU_BCM55538_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int iRlt;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
#if 0
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
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuEncryptParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

#endif


#if 1
/* -------------------ONU 地址管理API------------------- */
/*show fdbentry mac*/
static int GWONU_BCM55538_GetOnuMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_onu_address_table_record_t *addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        long int liEntryNum;
        
        if ( NULL == addr_table )
        {
            addr_table = MAC_Address_Table;
        }
        if ( 0 == (iRlt = RM_PASONU_address_table_get(olt_id, llid, &liEntryNum, addr_table)) )
        {
            *EntryNum = (long)liEntryNum;
        }
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*show fdbentry mac*/
/*---modified by wangjiah@2016-07-14 to solve problem #30634*/
static int GWONU_BCM55538_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	//short int addr_num;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
		/*
        addr_num = 0;
        if ( 0 == (iRlt = pon_pp_get_link_addrs(olt_id, llid, &addr_num, addr_table)) )
        {
            *EntryNum = (long)addr_num;
        }
		*/
        iRlt = pon_pp_get_link_addrs(olt_id, llid, EntryNum, addr_table); 
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*set onu max mac*/
/*------modified by wangjiah@2016-11-30 to support arad l2 mac learn limit from OLTV1R15B269-------*/
static int GWONU_BCM55538_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
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
	if ( INVALID_LLID != (llid = GetLlidByLlidIdx(olt_id, onu_id, llid_id)) )
    {
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
                    max_entry = uiSetVal;
                }
            }

		iRlt = ctss_pon_set_l2_mac_learn_limit(olt_id,llid,max_entry);
		if(VOS_ERROR == iRlt)
		{
			sys_console_printf("error, set l2 mac learn limit failer");
		}
    }   

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}

/*get onu pon uni mac config*/
static int GWONU_BCM55538_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
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
					
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuUniMacCfg(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
/*RTT*/
static int GWONU_BCM55538_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    bcmEmmiLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info )) )
        {
			/*Begin-----modified by wangjiah@2016-07-19 to solve issue:30605----------*/
            //*rtt = link_info.rangeValue;
            *rtt = link_info.distance;
			/*End-----modified by wangjiah@2016-07-19 to solve issue:30605----------*/
        }
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuDistance(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GWONU_BCM55538_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    bcmEmmiLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info)) )
        {
            optical_capability->laser_on_time  = link_info.onuLaserOnTime;
            optical_capability->laser_off_time = link_info.onuLaserOffTime;
        }
	}

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOpticalCapability(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
#endif


#if 1
/* -------------------ONU 倒换API---------------- */

static int GWONU_BCM55538_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);

    if ( BcmOnuIsOnline(olt_id, llid) )
    {
        iRlt = 0;
    }
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }
    
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 设备管理API------------------- */

/*get onu pon chip ver*/
static int GWONU_BCM55538_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    short int onuid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
		
    /* iRlt = GetOnuChipVersion(olt_id, onu_id, device_versions); */
	 
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuPonVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int GWONU_BCM55538_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    bcmEmmiLinkInfo link_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = BcmOLT_GetLinkInfo ( olt_id, llid, &link_info)) )
        {
            onu_info->oam_version = OAM_STANDARD_VERSION_3_3;
			/*Begin-----modified by wangjiah@2016-07-19 to solve issue:30605----------*/
            //onu_info->rtt = link_info.rangeValue;
            onu_info->rtt = link_info.distance;
			/*End-----modified by wangjiah@2016-07-19 to solve issue:30605----------*/
            
            onu_info->laser_on_time  = link_info.onuLaserOnTime;
            onu_info->laser_off_time = link_info.onuLaserOffTime;

            onu_info->vendorType     = 0;
            onu_info->productVersion = 0;
            onu_info->productCode    = 0;
            (void)PON_CTC_STACK_get_onu_ponchip_vendor(olt_id, llid, &onu_info->vendorType, &onu_info->productCode, &onu_info->productVersion);

            onu_info->max_links_support = 1;
            onu_info->curr_links_num = 1;
            onu_info->max_cm_support = 0;

            switch ( link_info.rate )
            {
                case bcmEmmiEponRateTenTen:
                    onu_info->pon_rate_flags = PON_RATE_10_10G;
                break;    
                case bcmEmmiEponRateTenOne:
                    onu_info->pon_rate_flags = PON_RATE_1_10G;
                break;    
                case bcmEmmiEponRateOneOne:
                    onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
                break;    
                default:
                    onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
                    VOS_ASSERT(0);
            }
        }
    }   

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int BCM55538_GetI2CAddressByInfoId(short int olt_id, short int onu_id, int info_id, unsigned char *i2cDevAddr, unsigned char *internalDevAddrCount, unsigned char internalDevAddr[])
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

static int GWONU_BCM55538_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = RM_PASONU_eeprom_mapper_get_parameter( olt_id, llid, info_id, data, size );
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_BCM55538_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = RM_PASONU_eeprom_mapper_set_parameter( olt_id, llid, info_id, data, size );
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}


static int GWONU_BCM55538_ResetOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_reset_onu( olt_id, llid );
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_ResetOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int CTCONU_BCM55538_ResetOnu(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_reset_onu( olt_id, llid );
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ResetOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_BCM55538_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = RM_PASONU_get_burn_image_complete( olt_id, llid, complete );
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetBurnImageComplete(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */

static int CTCONU_BCM55538_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_onu_version(olt_id, llid, version);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetCtcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_firmware_version(olt_id, llid, version);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetFirmwareVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_onu_serial_number(olt_id, llid, serial_number);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetSerialNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_chipset_id(olt_id, llid, chipset_id);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetChipsetID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_onu_capabilities(olt_id, llid, caps);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetOnuCap1(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_onu_capabilities_2(olt_id, llid, caps);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetOnuCap2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_onu_capabilities_3(olt_id, llid, caps);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetOnuCap3(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	CTC_STACK_binary_t  f_image;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(file_start);
    VOS_ASSERT(file_len > 0);
    VOS_ASSERT(file_name);

	f_image.source = PON_BINARY_SOURCE_MEMORY;
	f_image.location = file_start;
	f_image.size = file_len;

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_update_onu_firmware(olt_id, llid, &f_image, file_name);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_UpdateOnuFirmware(%d, %d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, file_len, file_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_ActiveOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_activate_image(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ActiveOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_CommitOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_commit_image(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_CommitOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int *timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    unsigned short int tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_extended_oam_discovery_timing(&tm);
        if ( 0 == iRlt )
        {
            *timeout = tm;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_extended_oam_discovery_timing((unsigned short int)timeout);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetEncryptionTiming(short int olt_id, short int onu_id, int *update_time, int *no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar update_key_t;
        ushort no_reply_t;

        iRlt = PON_CTC_STACK_get_encryption_timing(&update_key_t, &no_reply_t);

        if ( 0 == iRlt )
        {
            *update_time = update_key_t;
            *no_reply = no_reply_t;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *update_time, *no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEncryptionTiming(short int olt_id, short int onu_id, int update_time, int no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar update_key_t = update_time;
        ushort no_reply_t  = no_reply;

        iRlt = PON_CTC_STACK_set_encryption_timing(update_key_t, no_reply_t);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, update_time, no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_StartEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_start_encryption(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_StartEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_StopEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_stop_encryption(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_StopEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortLinkstate(short int olt_id, short int onu_id, int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_ethernet_link_state(olt_id, llid, (unsigned char)port_id, enable);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortLinkstate(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = FALSE;

        iRlt = PON_CTC_STACK_get_phy_admin_state(olt_id, llid, lport, &en);
        if ( 0 == iRlt )
        {
            *enable = en;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = enable;

        iRlt = PON_CTC_STACK_set_phy_admin_control(olt_id, llid, lport, en);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = FALSE;

        iRlt = PON_CTC_STACK_get_ethernet_port_pause(olt_id, llid, lport, &en);
        if ( 0 == iRlt )
        {
            *enable = en;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortPauseEnable(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = enable;

        iRlt = PON_CTC_STACK_set_ethernet_port_pause(olt_id, llid, lport, en);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortPauseEnable(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = FALSE;

        iRlt = PON_CTC_STACK_get_auto_negotiation_admin_state(olt_id, llid, lport, &en);
        if ( 0 == iRlt )
        {
            *enable = en;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool en = enable;

        iRlt = PON_CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, en);
        if ( 0 == iRlt )
        {
            if ( TRUE == en )
            {
                /*只有在激活时发restart*/
                (void)PON_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
            }
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortRestartAutoConfig(short int olt_id, short int onu_id, int port_id )
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, (unsigned char)port_id);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortRestartAutoConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetEthPortAnLocalTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(ability);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_auto_negotiation_local_technology_ability(olt_id, llid, (unsigned char)port_id, ability);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortAnLocalTecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetEthPortAnAdvertisedTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(ability);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_auto_negotiation_advertised_technology_ability(olt_id, llid, (unsigned char)port_id, ability);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortAnAdvertisedTecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(policing);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_ethernet_port_policing(olt_id, llid, (unsigned char)port_id, policing);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_ethernet_port_policing(olt_id, llid, (unsigned char)port_id, *policing);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(policing);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_ethernet_port_ds_rate_limiting(olt_id, llid, (unsigned char)port_id, policing);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, (unsigned char)port_id, policing);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, vconf);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, *vconf);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetAllPortVlanConfig(short int olt_id, short int onu_id,  unsigned char *portNum, CTC_STACK_vlan_configuration_ports_t ports_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, portNum, ports_info);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetAllPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *portNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port_id configuration*/
static int CTCONU_BCM55538_SetPortQinqConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_qinq_configuration_t port_configuration )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
		iRlt = PON_CTC_STACK_set_qinq_port_configuration(olt_id, llid, (unsigned char)port_id, port_configuration);
    }

	OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortQinqConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_classification_and_marking(olt_id, llid, (unsigned char)port_id, cam);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rule_mode_t mode, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_classification_and_marking(olt_id, llid, (unsigned char)port_id, mode, cam);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, (unsigned char)port_id);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_vlan(olt_id, llid, (unsigned char)port_id, mv);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_vlan(olt_id, llid, (unsigned char)port_id, *mv);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_ClearEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_clear_multicast_vlan(olt_id, llid, (unsigned char)port_id);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ClearEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int *num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    unsigned char max_number;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_group_num(olt_id, llid, (unsigned char)port_id, &max_number);
        if ( 0 == iRlt )
        {
            *num = max_number;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_group_num(olt_id, llid, (unsigned char)port_id, (unsigned char)num);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    bool strip_mode;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_tag_strip(olt_id, llid, (unsigned char)port_id, &strip_mode);
        if ( 0 == iRlt )
        {
            *strip = strip_mode;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortMulticastTagStrip(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *strip, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_tag_strip(olt_id, llid, (unsigned char)port_id, (bool)strip);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortMulticastTagStrip(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, strip, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetMulticastAllPortTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_get_multicast_all_port_tag_strip(olt_id, llid, number_of_entries, ports_info);
	}
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetMulticastAllPortTagStrip(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

static int CTCONU_BCM55538_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_tag_oper(olt_id, llid, (unsigned char)port_id, oper, sw);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetEthPortMulticastTagOper( short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_tag_oper(olt_id, llid, (unsigned char)port_id, oper, *sw);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_set_multicast_management_object_tag_oper(olt_id, llid, management_object->index, tag_oper, *sw);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetObjMulticastTagOper(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}


static int CTCONU_BCM55538_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_control(olt_id, llid, mc);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_control(olt_id, llid, *mc);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_ClearMulticastControl(short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_clear_multicast_control(olt_id, llid);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ClearMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t *sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_multicast_switch(olt_id, llid, sw);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_switch(olt_id, llid, sw);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetFastLeaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_fast_leave_ability(olt_id, llid, ability);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_Get_FastLeave_Ability(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetFastLeaveAdminState(short int olt_id, short int onu_id, int *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_fast_leave_admin_state(olt_id, llid, state);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetFastLeaveAdminState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *state, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetFastLeaveAdminState(short int olt_id, short int onu_id, int state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_fast_leave_admin_control(olt_id, llid, state);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetFastLeaveAdminState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, state, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetOnuPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

	if(0 == port_id)
	{
		return 0;
	}
	
    if( OnuSupportStatistics(olt_id, onu_id) == VOS_OK )	/* 问题单16117 */
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            iRlt = PON_CTC_STACK_get_ethport_statistic_data(olt_id, llid, (unsigned char)port_id, data);
        }
    }
    else
    {
        iRlt = OLT_ERR_NOTSUPPORT;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuPortStaticData(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(state);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_ethport_statistic_state(olt_id, llid, (unsigned char)port_id, state);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuPortStaticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_ethport_statistic_state(olt_id, llid, (unsigned char)port_id, *state);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_SetOnuPortStaticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}


/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_BCM55538_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_set_alarm_admin_state(olt_id, llid, management_object->index, alarm_id, enable);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetAlarmAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_set_alarm_threshold(olt_id, llid, management_object->index, alarm_id, alarm_threshold, clear_threshold);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetAlarmThreshold(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
    
}

static int CTCONU_BCM55538_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_get_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_set_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}


static int CTCONU_BCM55538_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = PON_CTC_STACK_get_mxu_mng_global_parameter_config(olt_id, llid, mxu_mng);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = PON_CTC_STACK_set_mxu_mng_global_parameter_config(olt_id, llid, *mxu_mng);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetMxuMngSnmpConfig ( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_get_mxu_mng_snmp_parameter_config(olt_id, llid, parameter);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_SetMxuMngSnmpConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_set_mxu_mng_snmp_parameter_config(olt_id, llid, *parameter);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int CTCONU_BCM55538_GetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_holdover_state(olt_id, llid, holdover);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_holdover_state(olt_id, llid, *holdover);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_get_optical_transceiver_diagnosis(olt_id, llid, optical_transceiver_diagnosis);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetOptTransDiag(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_BCM55538_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = PON_CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, FALSE);
    }
    else
        iRlt = PON_CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, TRUE);

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t *fec_ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = PON_CTC_STACK_get_fec_ability(olt_id, llid, fec_ability);
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetFecAbility(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *fec_ability, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */


#if 1
int CTCONU_BCM55538_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t *iad_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(iad_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_voip_iad_info(olt_id, llid, iad_info);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetIADInfo:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

/*H.248协议下IAD的运行状态*/
int CTCONU_BCM55538_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(iad_oper_status);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_voip_iad_oper_status(olt_id, llid, iad_oper_status);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetVoipIadOperStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_voip_iad_operation(olt_id, llid, operation_type);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetVoipIadOperation:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

/*语音模块全局参数配置*/
int CTCONU_BCM55538_GetVoipGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(global_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_voip_global_param_conf(olt_id, llid, global_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetVoipGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetVoipGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_voip_global_param_conf_t global_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(global_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        VOS_MemZero(&global_config, sizeof(CTC_STACK_voip_global_param_conf_t));
        iRlt = PON_CTC_STACK_get_voip_global_param_conf(olt_id, llid, &global_config);
        if ( CTC_STACK_EXIT_OK == iRlt )
        {
            switch(code)
            {
                case voice_global_config_ip_mode:
                    global_config.voice_ip_mode = global_param->voice_ip_mode;
                    break;
                case voice_global_config_ip_addr:
                    global_config.iad_ip_addr = global_param->iad_ip_addr;
                    global_config.iad_net_mask = global_param->iad_net_mask;
                    break;
                case voice_global_config_ip_gw:
                    global_config.iad_def_gw = global_param->iad_def_gw;
                    break;
                case voice_global_config_pppoe_mode:
                    global_config.pppoe_mode = global_param->pppoe_mode;
                    break;
                case voice_global_config_pppoe_username:
                    VOS_MemCpy(global_config.pppoe_user, global_param->pppoe_user, VOIP_PPPOE_USER_SIZE);
                    break;
                case voice_global_config_pppoe_password:
                    VOS_MemCpy(global_config.pppoe_passwd, global_param->pppoe_passwd, VOIP_PPPOE_PASSWD_SIZE);
                    break;
                case voice_global_config_tagged_mode:
                    global_config.tag_flag = global_param->tag_flag;
                    break;
                case voice_global_config_vlan:
                    global_config.cvlan_id = global_param->cvlan_id;
                    global_config.tag_flag = CTC_STACK_VOIP_TAGGED_FLAG_TAG;                    
                    break;
                case voice_global_config_priority:
                    global_config.priority = global_param->priority;
                    break;
                default:
                    VOS_MemCpy(&global_config, global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
                    break;
            }

            iRlt = PON_CTC_STACK_set_voip_global_param_conf(olt_id, llid, global_config);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetVoipGlobalParameter:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(voip_fax);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_voip_fax_config(olt_id, llid, voip_fax);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_voip_fax_config_t ulvoip_fax;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(voip_fax);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulvoip_fax, sizeof(CTC_STACK_voip_fax_config_t));
        iRlt = PON_CTC_STACK_get_voip_fax_config(olt_id, llid, &ulvoip_fax);
        if (iRlt == CTC_STACK_EXIT_OK)
        {
            switch(code)
            {
                case voice_fax_config_t38_enable:
                    ulvoip_fax.t38_enable = voip_fax->t38_enable;
                    break;
                case voice_fax_config_control:
                    ulvoip_fax.fax_control = voip_fax->fax_control;
                    break;
                default:
                    VOS_MemCpy(&ulvoip_fax, voip_fax, sizeof(CTC_STACK_voip_fax_config_t));
                    break;
            }
            iRlt = PON_CTC_STACK_set_voip_fax_config(olt_id, llid, ulvoip_fax);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    
    return iRlt;           
}


int CTCONU_BCM55538_GetVoipPortStatus(short int olt_id, short int onu_id, int port_id, CTC_STACK_voip_pots_status_array *pots_status_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(pots_status_array);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        if(port_id)
            management_object_index.port_number = port_id;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        pots_status_array->number_of_entries = 24;
        iRlt = PON_CTC_STACK_get_voip_pots_status(olt_id, llid, management_object_index, &(pots_status_array->number_of_entries), &(pots_status_array->pots_status_array));
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetVoipPortStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(port_state);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_voip_port(olt_id, llid, (unsigned char)port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);

    return iRlt;   
}

int CTCONU_BCM55538_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_voip_port(olt_id, llid, (unsigned char)port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int CTCONU_BCM55538_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = slot;        
        management_object_index.port_number = port;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        
        iRlt = PON_CTC_STACK_get_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;   
    
}

int CTCONU_BCM55538_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t  management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = slot;        
        management_object_index.port_number = port;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        iRlt = PON_CTC_STACK_set_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;           
    
}


int CTCONU_BCM55538_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_h248_param_config(olt_id, llid, h248_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_h248_param_config_t h248_config;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&h248_config, sizeof(CTC_STACK_h248_param_config_t));
        iRlt = PON_CTC_STACK_get_h248_param_config(olt_id, llid, &h248_config);
        if ( CTC_STACK_EXIT_OK == iRlt )
        {
            switch(code)
            {
                case h248_config_local_port:
                    h248_config.mg_port = h248_param->mg_port;
                    break;
                case h248_config_mgc_addr:
                    h248_config.mgcip = h248_param->mgcip;
                    break;
                case h248_config_mgc_port:
                    h248_config.mgccom_port_num = h248_param->mgccom_port_num;
                    break;
                case h248_config_mgc_backup_addr:
                    h248_config.back_mgcip = h248_param->back_mgcip;
                    break;
                case h248_config_mgc_backup_port:
                    h248_config.back_mgccom_port_num = h248_param->back_mgccom_port_num;
                    break;
                case h248_config_regiser_mode:
                    h248_config.reg_mode = h248_param->reg_mode;
                    break;
                case h248_config_local_domain:
                    VOS_MemCpy(h248_config.mid, h248_param->mid, H248_MID_SIZE);
                    break;
                case h248_config_heartbeat_enable:
                    h248_config.heart_beat_mode = h248_param->heart_beat_mode;
                    break;
                case h248_config_heartbeat_time:
                    h248_config.heart_beat_cycle = h248_param->heart_beat_cycle;
                    break;            
                default:
                    VOS_MemCpy(&h248_config, h248_param, sizeof(CTC_STACK_h248_param_config_t));                    
                    break;
            }
            iRlt = PON_CTC_STACK_set_h248_param_config(olt_id, llid, h248_config);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_GetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63; 
        if(port_id)
            management_object_index.port_number = port_id;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        h248_user_tid_array->number_of_entries = 24;
        iRlt = PON_CTC_STACK_get_h248_user_tid_config(olt_id, llid, management_object_index, &(h248_user_tid_array->number_of_entries), &(h248_user_tid_array->h248_user_tid_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_t *user_tid_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        if(port_id)
            management_object_index.port_number = port_id;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        iRlt = PON_CTC_STACK_set_h248_user_tid_config (olt_id, llid, management_object_index, *user_tid_config);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_rtp_tid_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_h248_rtp_tid_info(olt_id, llid, h248_rtp_tid_info);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(h248_rtp_tid_info);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_h248_rtp_tid_config(olt_id, llid, *h248_rtp_tid_info);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}


int CTCONU_BCM55538_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_sip_param_config(olt_id, llid, sip_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_sip_param_config_t ulsip_param;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_param);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulsip_param, sizeof(CTC_STACK_sip_param_config_t));
        iRlt = PON_CTC_STACK_get_sip_param_config(olt_id, llid, &ulsip_param);
        if (CTC_STACK_EXIT_OK == iRlt)
        {
            switch(code)
            {
                case sip_config_local_port:
                    ulsip_param.mg_port = sip_param->mg_port;
                    break;
                case sip_config_server_ip:
                    ulsip_param.server_ip = sip_param->server_ip;
                    break;
                case sip_config_server_port:
                    ulsip_param.serv_com_port = sip_param->serv_com_port;
                    break;
                case sip_config_server_backup_ip:
                    ulsip_param.back_server_ip = sip_param->back_server_ip;
                    break;
                case sip_config_server_backup_port:
                    ulsip_param.back_serv_com_port = sip_param->back_serv_com_port;
                    break;
                case sip_config_reg_server_ip:
                    ulsip_param.reg_server_ip = sip_param->reg_server_ip;
                    break;
                case sip_config_reg_server_port:
                    ulsip_param.reg_serv_com_port = sip_param->reg_serv_com_port;
                    break;
                case sip_config_reg_server_backup_ip:
                    ulsip_param.back_reg_server_ip = sip_param->back_reg_server_ip;
                    break;
                case sip_config_reg_server_backup_port:
                    ulsip_param.back_reg_serv_com_port = sip_param->back_reg_serv_com_port;
                    break;
                case sip_config_outbound_server_ip:
                    ulsip_param.outbound_server_ip = sip_param->outbound_server_ip;
                    break;
                case sip_config_outbound_server_port:
                    ulsip_param.outbound_serv_com_port = sip_param->outbound_serv_com_port;
                    break;
                case sip_config_reg_interval:
                    ulsip_param.reg_interval = sip_param->reg_interval;
                   break;
                case sip_config_heartbeat_enable:
                    ulsip_param.heart_beat_switch = sip_param->heart_beat_switch;
                    break;
                case sip_config_heartbeat_time:
                    ulsip_param.heart_beat_cycle = sip_param->heart_beat_cycle;
                    break;
                default:
                    VOS_MemCpy(&ulsip_param, sip_param, sizeof(CTC_STACK_sip_param_config_t));
                    break;
            }
            iRlt = PON_CTC_STACK_set_sip_param_config(olt_id, llid, ulsip_param);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }    
        else
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    
    return iRlt;           
}

int CTCONU_BCM55538_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sip_digit_map);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_sip_digit_map(olt_id, llid, *sip_digit_map);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetSipDigitMap:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}


int CTCONU_BCM55538_GetSipUserConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        if(port_id)
            management_object_index.port_number = port_id;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        sip_user_param_array->number_of_entries = 24;
        iRlt = PON_CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &(sip_user_param_array->number_of_entries), &(sip_user_param_array->sip_user_param_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_GetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_BCM55538_SetSipUserConfig(short int olt_id, short int onu_id, int port_id, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object_index;
    unsigned char number_of_entries = 24;
    CTC_STACK_sip_user_param_config_array_t sip_user_param_array;    
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        management_object_index.port_number = port_id;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;       
        iRlt = PON_CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &number_of_entries, &sip_user_param_array);                
        switch(code)
        {
            case sip_user_config_account:
                VOS_MemCpy(sip_user_param->user_name, sip_user_param_array[0].sip_user_param_config.user_name, SIP_USER_NAME_SIZE);                
                VOS_MemCpy(sip_user_param->passwd, sip_user_param_array[0].sip_user_param_config.passwd, SIP_PASSWD_SIZE);                
                break;
            case sip_user_config_username:
                VOS_MemCpy(sip_user_param->sip_port_num, sip_user_param_array[0].sip_user_param_config.sip_port_num, SIP_PORT_NUM_SIZE);
                VOS_MemCpy(sip_user_param->passwd, sip_user_param_array[0].sip_user_param_config.passwd, SIP_PASSWD_SIZE);                
                break;
            case sip_user_config_password:
                VOS_MemCpy(sip_user_param->sip_port_num, sip_user_param_array[0].sip_user_param_config.sip_port_num, SIP_PORT_NUM_SIZE);
                VOS_MemCpy(sip_user_param->user_name, sip_user_param_array[0].sip_user_param_config.user_name, SIP_USER_NAME_SIZE);                
                break;                
            default:
                break;
        }

        iRlt = PON_CTC_STACK_set_sip_user_param_config(olt_id, llid, management_object_index, *sip_user_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_BCM55538_SetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}
#endif

#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

static int CTCONU_BCM55538_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0;
    short int llid = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        unsigned char version = 0;
		int portnum = getOnuEthPortNum(olt_id, onu_id);                    
		CTC_STACK_port_vlan_configuration_t pvc;		
        OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
    
        for ( i=0; i<portnum; i++ )
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

            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetVlanEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_BCM55538_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    short int llid = 0;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    mode &= (~ONU_SEPCAL_FUNCTION);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc);
        if ( 0 == iRlt )
        {
            if ( pvc.mode != mode )
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

                iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_BCM55538_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    ULONG all = 0, untag = 0;
    short int llid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    VOS_MemSet(Port_Info, 0, sizeof(CTC_STACK_vlan_configuration_ports_t));

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        unsigned char version = 0;
        OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
    
        if ( 0 == (iRlt = get_onuconf_vlanPortlist(olt_id, onu_id, vid,  &all, &untag)) )
        {
            if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
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
                        if ( untag & (1<<i/*(Port_Info[i].port_number-1)*/) )
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
                    iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, Port_Info[i].entry);
                    if(iRlt != VOS_OK)
                    {
                        lReturnValue = iRlt;
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                    }
                }
            }
            else
            {
                CTC_STACK_port_vlan_configuration_t pvc;                
                for(i=0;i<portnum;i++)
                {
                    VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                    iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc);
                    if(iRlt == VOS_OK)
                    {
                        int num = pvc.number_of_entries;                    
                        if(version >= CTC_2_1_ONU_VERSION)
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
                        iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_DelVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_BCM55538_SetPortPvid(short int olt_id, short int onu_id, int port_id, int lv)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    short int llid = 0;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc);
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

                iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortPvid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_BCM55538_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    short int llid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        unsigned char version = 0;
        OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
    
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
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
                            /* added by wangxiaoyu 2011-08-29
                                                   * 以tag方式加入VLAN时，去除原来的pvid值，复位为vlan 1
                                                   * pvc.default_vlan &= 0xfffff000;
                                                   * pvc.default_vlan |= 1;*/
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
                    iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, Port_Info[i].entry);
                    if(iRlt != VOS_OK)
                    {
                        lReturnValue = iRlt;                    
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                             
                    }
                }
            }
        }
        else
        {
			CTC_STACK_port_vlan_configuration_t pvc;

			for(i=0;i<portnum;i++)
			{
                if(!(portlist&(1<<i))) 
                    continue;							
				VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                if(PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc) == VOS_OK)
				{
                    int num = pvc.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
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
					iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_AddVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_BCM55538_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0, j = 0, num = 0;
    short int llid = 0;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;
    int portnum = getOnuEthPortNum(olt_id, onu_id);                    
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        unsigned char version = 0;
        OnuGen_Get_CtcVersion(olt_id, onu_id, &version);
        
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
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
                    iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, Port_Info[i].port_number, Port_Info[i].entry);
                    if(iRlt != VOS_OK)
                    {
                        lReturnValue = iRlt;                    
                        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                             
                    }
                }
            }
        }
        else
        {
            CTC_STACK_port_vlan_configuration_t pvc;            
            for(i=0;i<portnum;i++)
            {
                if (!(portlist&(1<<i))) 
                    continue;
                    
                VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc);
                if(iRlt == VOS_OK)
                {
                    int num = pvc.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
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
                    iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_DelVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return lReturnValue;
}

/*vlan transation*/
static int CTCONU_BCM55538_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i;
    short int llid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc)) )
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

            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, num;
    short int llid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc)) )
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

            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_DelVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*vlan aggregation*/
static int CTCONU_BCM55538_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc)) )
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

            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int CTCONU_BCM55538_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = PON_CTC_STACK_get_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, &pvc)) )
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
            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_DelVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*QinQ enable*/
static int CTCONU_BCM55538_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    ULONG vid = 1;
    short int llid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = getOnuConfPortSimpleVar(olt_id, onu_id, port_id, sv_enum_port_default_vid, &vid)) )
        {
            qinq_config.mode=enable;
            qinq_config.default_vlan=vid;
            qinq_config.number_of_entries=0;
            VOS_MemZero(qinq_config.vlan_list, CTC_MAX_VLAN_QINQ_ENTRIES);
            
            iRlt = PON_CTC_STACK_set_qinq_port_configuration(olt_id, llid, (unsigned char)port_id, qinq_config );
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortQinQEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return  iRlt;
}

/*qinq port vlan tag add*/
static int CTCONU_BCM55538_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = get_OnuVlanTagConfig(olt_id, onu_id, port_id, &qinq_config)) )
        {
            if ( CTC_MAX_VLAN_QINQ_ENTRIES/2 > qinq_config.number_of_entries )
            {
                int n = qinq_config.number_of_entries;

                qinq_config.vlan_list[2*n] = cvlan;
                qinq_config.vlan_list[2*n+1] = svlan;
                qinq_config.number_of_entries++;

                iRlt = PON_CTC_STACK_set_qinq_port_configuration(olt_id, llid, (unsigned char)port_id, qinq_config);
            }
            else
                iRlt = OLT_ERR_NORESC;
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_AddQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port vlan tag del*/
static int CTCONU_BCM55538_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, n;
    short int llid;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = get_OnuVlanTagConfig(olt_id, onu_id, port_id, &qinq_config)) )
        {
            for (i = 0; i < qinq_config.number_of_entries; i++)
            {
                if (qinq_config.vlan_list[2*i] == svlan)
                {
                    n = qinq_config.number_of_entries;
                    linux_memmove(qinq_config.vlan_list+2*i, qinq_config.vlan_list+2*i+2, (n-i-1)*2*sizeof(LONG));
                    qinq_config.number_of_entries--;
                    break;
                }

                iRlt = PON_CTC_STACK_set_qinq_port_configuration(olt_id, llid, (unsigned char)port_id, qinq_config);
            }
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_DelQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return  iRlt;
}


static int CTCONU_BCM55538_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool state = mode ? FALSE:TRUE;
        
        if ( 0 == (iRlt = PON_CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, state)) )
        {
            if ( state )
            {
                PON_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
            }
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_ethernet_port_pause(olt_id, llid, (unsigned char)port_id, (bool)en); 
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortFcMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        CTC_STACK_ethernet_port_policing_entry_t ppe;
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

        ppe.bucket_depth = cbs;
        ppe.cir = cir;
        ppe.extra_burst_size = ebs;

        ppe.operation = cir ? CTC_STACK_STATE_ACTIVATE : CTC_STACK_STATE_DEACTIVATE;

        iRlt = PON_CTC_STACK_set_ethernet_port_policing(olt_id, llid, (unsigned char)port_id, ppe);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortIngressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        CTC_STACK_ethernet_port_ds_rate_limiting_entry_t ppe;

        ppe.CIR = rate;
        ppe.PIR = rate;
        ppe.port_number = port_id;
        ppe.state = rate ? TRUE : FALSE;

        iRlt = PON_CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, (unsigned char)port_id, &ppe);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortEgressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
		if ( 0 == qossetid ) /* 删除pon端口上的qos配置*/
		{
			iRlt = PON_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, (unsigned char)port_id);
		}
        else
        {
            qos_classification_rules_t rules;
            
            if ( 0 == (iRlt = getOnuConfQosSet(olt_id, onu_id, qossetid, &rules)) )
            {
                CTC_STACK_classification_rules_t qset;
                qos_classification_rule_t *p;
                int nr = sizeof(rules)/sizeof(qos_classification_rule_t);

                int i=0, c = 1, j; /*pas-soft stack qosset entry index low limit is 1 and upper limmit is 31*/

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

                PON_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, (unsigned char)port_id);

                iRlt = PON_CTC_STACK_set_classification_and_marking(olt_id, llid, (unsigned char)port_id, CTC_CLASSIFICATION_ADD_RULE, qset);
            }
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, (unsigned char)port_id);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_ClrPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



static int CTCONU_BCM55538_SetPortDefaultPriority(short int olt_id, short int onu_id, int port_id, int priority)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        CTC_STACK_port_vlan_configuration_t pvc;
         
        if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, port_id, &pvc)) )
        {
            pvc.default_vlan &= 0x1fff;
            pvc.default_vlan |= priority << 13;
             
            iRlt = PON_CTC_STACK_set_vlan_port_configuration(olt_id, llid, (unsigned char)port_id, pvc);
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortDefaultPriority(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_switch(olt_id, llid, en ? CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING : CTC_STACK_PROTOCOL_CTC);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetIgmpEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM55538_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PON_CTC_STACK_set_multicast_switch(olt_id, llid, en);
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetIgmpAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_BCM55538_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        CTC_STACK_multicast_vlan_t mv;
        int num = 0, vids[ONU_MAX_IGMP_VLAN];

        if ( 0 == vid )
        {
            iRlt = PON_CTC_STACK_clear_multicast_vlan(olt_id, llid, (unsigned char)port_id);
        }
        else
        {
            if ( 0 == (iRlt= getOnuConfPortMulticastVlan(olt_id, onu_id, port_id, &num, vids)) )
            {
                int i, fexist = 0;
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
                    
                   PON_CTC_STACK_clear_multicast_vlan(olt_id, llid, (unsigned char)port_id);

                   mv.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
                   iRlt = PON_CTC_STACK_set_multicast_vlan(olt_id, llid, (unsigned char)port_id, mv);
                }
            }
        }
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_SetPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */

#if 1
/* --------------------CMC管理API------------------- */
static int CMCONU_BCM55538_RegisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
	
    iRlt = CMCCTRL_RegisterCmc(cmc_mac, NULL, 0);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_RegisterCmc(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_UnregisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
	
    iRlt = CMCCTRL_UnRegisterCmc(cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_UnregisterCmc(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_DumpCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_DumpCmc(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmc(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpAlarm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpAlarmList(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpAlarm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLog(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpLogData(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLog(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_ResetCmcBoard(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_ResetCmcBoard(cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_ResetCmcBoard(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcVersion(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetSoftwareVersion(version, len, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcMaxMulticasts(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_multicasts)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetNumOfMulticasts(max_multicasts, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcMaxMulticasts(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *max_multicasts, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_cm)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_GetNumOfCm(max_cm, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcMaxCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *max_cm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short max_cm)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_SetNumOfCm(max_cm, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcMaxCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, max_cm, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_GetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_LocalCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_LocalTime(cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_LocateCmcTime(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_SetCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len)
{
    int iRlt;
    BASE_DateTimeT cmc_time;


    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_SetCustomConfiguration(cfg_id, cfg_data, data_len, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcCustomConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cfg_id, data_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_System_DumpCmcCustomCfg(cmc_mac, cfg_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcCustomConfig(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cfg_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

#if 1
/* --------------------CMC频道管理API------------------- */
static int CMCONU_BCM55538_DumpCmcDownChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpDownstreamChannel(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcDownChannel(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcUpChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamChannel(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcUpChannel(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsEnabled(channel_id, channel_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *docsis30_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsD30Mode(channel_id, docsis30_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *docsis30_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char docsis30_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsD30Mode(channel_id, docsis30_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, docsis30_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsFreq(channel_id, channel_freq, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_AutoAssignDownstreamSettingsFreq(base_freq, step_freq, step_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_AutoAssignUpstreamSettingsFreq(base_freq, step_freq, step_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_width)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsChannelWidth(channel_id, channel_width, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_width, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_width)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsChannelWidth(channel_id, channel_width, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_width, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *annex_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsAnnexMode(channel_id, annex_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *annex_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char annex_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsAnnexMode(channel_id, annex_mode, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, annex_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamChannelType(channel_id, channel_type, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamChannelType(channel_id, channel_type, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *modulation_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsModulation(channel_id, modulation_type, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *modulation_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char modulation_type)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsModulation(channel_id, modulation_type, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, modulation_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_profile)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamSettingsChannelProfile(channel_id, channel_profile, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_profile, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_profile)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamSettingsChannelProfile(channel_id, channel_profile, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_profile, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *interleaver)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsInterleaver(channel_id, interleaver, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *interleaver, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char interleaver)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsInterleaver(channel_id, interleaver, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, interleaver, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetDownstreamSettingsPowerLevel(channel_id, channel_power, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetDownstreamSettingsPowerLevel(channel_id, channel_power, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_GetUpstreamInputPower(channel_id, channel_power, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_SetUpstreamInputPower(channel_id, channel_power, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamInputPower(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcUpChannelSignalQuality(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpUpstreamChannelSignalQuality(cmc_mac, channel_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcUpChannelSignalQuality(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcInterfaceUtilization(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    unsigned short channel_index;

    OLT_LOCAL_ASSERT(olt_id);

    channel_index = CMC_CHANNELID_2_IFINDEX(channel_type, channel_id);
    iRlt = CMCOAM_Channel_DumpCmcInterfaceUtilization(cmc_mac, channel_index, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcInterfaceUtilization(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcInterfaceStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    unsigned short channel_index;

    OLT_LOCAL_ASSERT(olt_id);

    channel_index = CMC_CHANNELID_2_IFINDEX(channel_type, channel_id);
    iRlt = CMCOAM_Channel_DumpCmcInterfaceStatistics(cmc_mac, channel_index, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcInterfaceStatistics(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcMacStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpCmcMacStatistics(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcMacStatistics(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcAllInterface(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Channel_DumpCmcAllInterfaces(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcAllInterface(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------CMC频道组管理API------------------- */
static int CMCONU_BCM55538_DumpAllLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpAllLoadBalancingGrp(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpAllLoadBalancingGrp(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpFullLoadBalancingGrpSettings(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpDownsteam(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpUpsteam(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingDynConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingDynConfig(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingDynConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_SetLoadBalancingDynMethod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char method)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMethod(cmc_mac, method);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynMethod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, method, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynPeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynPeriod(cmc_mac, period);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynPeriod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, period, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynWeightedAveragePeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynWeightedAveragePeriod(cmc_mac, period);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynWeightedAveragePeriod(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, period, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynOverloadThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynOverloadThresold(cmc_mac, percent);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynOverloadThresold(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, percent, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynDifferenceThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynDifferenceThresold(cmc_mac, percent);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynDifferenceThresold(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, percent, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynMaxMoveNumber(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long max_move)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMaxMoveNumber(cmc_mac, max_move);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynMaxMoveNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, max_move, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynMinHoldTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long hold_time)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynMinHoldTime(cmc_mac, hold_time);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynMinHoldTime(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, hold_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynRangeOverrideMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char range_mode)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynRangeOverrideMode(cmc_mac, range_mode);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynRangeOverrideMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, range_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynAtdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynAtdmaDccInitTech(cmc_mac, tech_id);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynAtdmaDccInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynScdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynScdmaDccInitTech(cmc_mac, tech_id);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynScdmaDccInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynAtdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynAtdmaDbcInitTech(cmc_mac, tech_id);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynAtdmaDbcInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetLoadBalancingDynScdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetLoadBalancingDynScdmaDbcInitTech(cmc_mac, tech_id);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetLoadBalancingDynScdmaDbcInitTech(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_CreateLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char grp_method)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_CreateDestroyLoadBalancingGrp(grp_id, grp_method, 1, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_CreateLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, grp_method, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DestroyLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_CreateDestroyLoadBalancingGrp(grp_id, 1, 2, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DestroyLoadBalancingGrp(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_AddLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveDownChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 1, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_AddLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_RemoveLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveDownChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 2, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_RemoveLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_AddLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveUpChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 1, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_AddLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_RemoveLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveUpChannelsToFromLoadBalancingGrp(grp_id, ch_ids, num_of_ch, 2, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_RemoveLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_AddLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveCnuToFromLoadBalancingGrp(grp_id, 1, mac_start, mac_end, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_AddLoadBalancingGrpModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_RemoveLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_AddRemoveCnuToFromLoadBalancingGrp(grp_id, 2, mac_start, mac_end, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_RemoveLoadBalancingGrpModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_AddLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetExcludeCnusFromLoadBalancingGrp(1, mac_start, mac_end, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_AddLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_RemoveLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_SetExcludeCnusFromLoadBalancingGrp(2, mac_start, mac_end, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_RemoveLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_DumpLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpCnuConfig(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpModem(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrpActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpActivedCnu(cmc_mac, grp_id, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpActivedModem(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpExcludeCnusConfiguration(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpExcludeModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpLoadBalancingGrpExcludeActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Group_DumpLoadBalancingGrpExcludeActiveCnus(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpLoadBalancingGrpExcludeActivedModem(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------CM管理API------------------- */
static int CMCONU_BCM55538_DumpAllCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCnuList(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpAllCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuStatus(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCm(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCableModemHistory(cmc_mac, NULL, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpAllCmHistory(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_DumpCableModemHistory(cmc_mac, cm_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmHistory(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_ClearAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Cm_ClearCableModemHistory(cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_ClearAllCmHistory(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_ResetCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_ClearCableModem(stCmMacAddr, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_ResetCm(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_DumpCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuDownstream(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmDownstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_DumpCnuUpstream(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmUpstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_SetCnuDownstream(stCmMacAddr, ch_ids, num_of_ch, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmDownstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_SetCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCOAM_Cm_SetCnuUpstream(stCmMacAddr, ch_ids, num_of_ch, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_SetCmUpstream(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_CreateCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_CreateServiceFlow(stCmMacAddr, cos, tlv_data, (int)tlv_len, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_CreateCmServiceFlow(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, cos, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_ModifyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_ChangeServiceFlow(stCmMacAddr, usfid, dsfid, tlv_data, (int)tlv_len, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_ModifyCmServiceFlow(%d, %d, %lu, %lu, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, usfid, dsfid, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DestroyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Dsx_DestroyServiceFlow(stCmMacAddr, usfid, dsfid, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DestroyCmServiceFlow(%d, %d, %lu, %lu)'s result(%d) on slot %d.\r\n", olt_id, onu_id, usfid, dsfid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_DumpCmClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Qos_DumpCmClassifier(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmClassifier(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    MacAddressT stCmMacAddr;

    OLT_LOCAL_ASSERT(olt_id);

    VOS_MemCpy(stCmMacAddr.addr, cm_mac, 6);
    iRlt = CMCCTRL_Qos_DumpCmServiceFlow(cmc_mac, stCmMacAddr, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmServiceFlow(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------QoS管理API------------------- */
static int CMCONU_BCM55538_DumpCmcClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpPktClassifierConfig(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcClassifier(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpServiceFlowConfig(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcServiceFlow(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcServiceFlowStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpServiceFlowStatistics(sf_ids, num_of_sf, cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcServiceFlowStatistics(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcDownChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpDsBondingGroup(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcDownChannelBondingGroup(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DumpCmcUpChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCOAM_Qos_DumpUsBondingGroup(cmc_mac, dump_buf, dump_len);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DumpCmcUpChannelBondingGroup(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CMCONU_BCM55538_CreateServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_Qos_SetServiceFlowClassName(class_name, 1, tlv_data, (int)tlv_len, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_CreateServiceFlowClassName(%d, %d, %s, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, class_name, tlv_len, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_DestroyServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);

    iRlt = CMCCTRL_Qos_SetServiceFlowClassName(class_name, 2, NULL, 0, cmc_mac);
    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_DestroyServiceFlowClassName(%d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, class_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#if 1
/* --------------------地址管理API------------------- */
static int CMCONU_BCM55538_GetCmcMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = TkAdapter_PAS5201_GetCmcAddrTable(olt_id, llid, cmc_mac, addr_num, addr_table);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmcMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_GetCmMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = TkAdapter_PAS5201_GetCnuAddrTable(olt_id, llid, cmc_mac, cm_mac, addr_num, addr_table);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_GetCmMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CMCONU_BCM55538_ResetCmAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        iRlt = TkAdapter_PAS5201_ResetCnuAddrTable(olt_id, llid, cmc_mac, cm_mac, addr_type);
#else
    iRlt = OLT_ERR_NOTSUPPORT;
#endif
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CMCONU_BCM55538_ResetCmAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, addr_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif

#endif


/* --------------------END------------------------ */

/******************************************外部接口***************************************/
static  const OnuMgmtIFs  s_gwonu55538Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_BCM55538_OnuIsOnline,
    GWONU_BCM55538_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_BCM55538_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_BCM55538_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_BCM55538_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_BCM55538_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    GWONU_BCM55538_SetOnuTrafficServiceMode,
    GWONU_BCM55538_SetOnuPeerToPeer,
    GWONU_BCM55538_SetOnuPeerToPeerForward,
    GWONU_BCM55538_SetOnuBW,
    GWONU_BCM55538_GetOnuSLA,

    GWONU_BCM55538_SetOnuFecMode,
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_BCM55538_SetUniPort,
    GWONU_BCM55538_SetSlowProtocolLimit,
    GWONU_BCM55538_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_BCM55538_GetOnuB2PMode,
    GWONU_BCM55538_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_BCM55538_ResetCounters,
    GWONU_BCM55538_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_BCM55538_GetLLIDParams,
    GWONU_BCM55538_StartEncryption,
    GWONU_BCM55538_StopEncryption,
    GWONU_BCM55538_SetOnuEncryptParams,
    GWONU_BCM55538_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_BCM55538_GetOnuMacAddrTbl,
    GWONU_BCM55538_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_BCM55538_SetOnuMaxMacNum,
    GWONU_BCM55538_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_BCM55538_GetOnuDistance,
    GWONU_BCM55538_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_BCM55538_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_BCM55538_GetOnuPonVer,
    GWONU_BCM55538_GetOnuRegisterInfo,
    GWONU_BCM55538_GetOnuI2CInfo,
    GWONU_BCM55538_SetOnuI2CInfo,
    
    GWONU_BCM55538_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_BCM55538_GetBurnImageComplete,
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
    NULL,           /*GetOnuPortDataByID*/
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
    CMCONU_BCM55538_RegisterCmc,
    CMCONU_BCM55538_UnregisterCmc,
    CMCONU_BCM55538_DumpCmc,
    CMCONU_BCM55538_DumpAlarm,
    CMCONU_BCM55538_DumpLog,
    
    CMCONU_BCM55538_ResetCmcBoard,
    CMCONU_BCM55538_GetCmcVersion,
    CMCONU_BCM55538_GetCmcMaxMulticasts,
    CMCONU_BCM55538_GetCmcMaxCm,
    CMCONU_BCM55538_SetCmcMaxCm,
    
    CMCONU_BCM55538_GetCmcTime,
    CMCONU_BCM55538_SetCmcTime,
    CMCONU_BCM55538_LocalCmcTime,
    CMCONU_BCM55538_SetCmcCustomConfig,
    CMCONU_BCM55538_DumpCmcCustomConfig,
    
    CMCONU_BCM55538_DumpCmcDownChannel,
    CMCONU_BCM55538_DumpCmcUpChannel,
    CMCONU_BCM55538_GetCmcDownChannelMode,
    CMCONU_BCM55538_SetCmcDownChannelMode,
    CMCONU_BCM55538_GetCmcUpChannelMode,

    CMCONU_BCM55538_SetCmcUpChannelMode,
    CMCONU_BCM55538_GetCmcUpChannelD30Mode,
    CMCONU_BCM55538_SetCmcUpChannelD30Mode,
    CMCONU_BCM55538_GetCmcDownChannelFreq,
    CMCONU_BCM55538_SetCmcDownChannelFreq,

    CMCONU_BCM55538_GetCmcUpChannelFreq,
    CMCONU_BCM55538_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_BCM55538_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_BCM55538_SetCmcUpAutoFreq,
    CMCONU_BCM55538_GetCmcUpChannelWidth,
    CMCONU_BCM55538_SetCmcUpChannelWidth,
    CMCONU_BCM55538_GetCmcDownChannelAnnexMode,
    CMCONU_BCM55538_SetCmcDownChannelAnnexMode,
    
    CMCONU_BCM55538_GetCmcUpChannelType,
    CMCONU_BCM55538_SetCmcUpChannelType,
    CMCONU_BCM55538_GetCmcDownChannelModulation,
    CMCONU_BCM55538_SetCmcDownChannelModulation,
    CMCONU_BCM55538_GetCmcUpChannelProfile,
    
    CMCONU_BCM55538_SetCmcUpChannelProfile,
    CMCONU_BCM55538_GetCmcDownChannelInterleaver,
    CMCONU_BCM55538_SetCmcDownChannelInterleaver,
    CMCONU_BCM55538_GetCmcDownChannelPower,    
    CMCONU_BCM55538_SetCmcDownChannelPower,    

    CMCONU_BCM55538_GetCmcUpChannelPower,    
    CMCONU_BCM55538_SetCmcUpChannelPower,    
    CMCONU_BCM55538_DumpCmcUpChannelPower,    
    CMCONU_BCM55538_DumpCmcUpChannelSignalQuality,    
    CMCONU_BCM55538_DumpCmcInterfaceUtilization,    

    CMCONU_BCM55538_DumpCmcInterfaceStatistics,    
    CMCONU_BCM55538_DumpCmcMacStatistics,    
    CMCONU_BCM55538_DumpCmcAllInterface,    
    CMCONU_BCM55538_DumpAllLoadBalancingGrp,    
    CMCONU_BCM55538_DumpLoadBalancingGrp,    

    CMCONU_BCM55538_DumpLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_DumpLoadBalancingGrpUpstream,    
    CMCONU_BCM55538_DumpLoadBalancingDynConfig,    
    CMCONU_BCM55538_SetLoadBalancingDynMethod,    
    CMCONU_BCM55538_SetLoadBalancingDynPeriod,    

    CMCONU_BCM55538_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_BCM55538_SetLoadBalancingDynOverloadThresold,    
    CMCONU_BCM55538_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_BCM55538_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_BCM55538_SetLoadBalancingDynMinHoldTime,    

    CMCONU_BCM55538_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_BCM55538_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_BCM55538_CreateLoadBalancingGrp,    
    CMCONU_BCM55538_DestroyLoadBalancingGrp,    
    CMCONU_BCM55538_AddLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_AddLoadBalancingGrpUpstream,    

    CMCONU_BCM55538_RemoveLoadBalancingGrpUpstream,    
    CMCONU_BCM55538_AddLoadBalancingGrpModem,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpModem,    
    CMCONU_BCM55538_AddLoadBalancingGrpExcludeModem,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_BCM55538_DumpLoadBalancingGrpModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpActivedModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_BCM55538_DumpAllCm,    
    CMCONU_BCM55538_DumpCm,    
    CMCONU_BCM55538_DumpAllCmHistory,    
    CMCONU_BCM55538_DumpCmHistory,    
    CMCONU_BCM55538_ClearAllCmHistory,    

    CMCONU_BCM55538_ResetCm,    
    CMCONU_BCM55538_DumpCmDownstream,    
    CMCONU_BCM55538_DumpCmUpstream,    
    CMCONU_BCM55538_SetCmDownstream,    
    CMCONU_BCM55538_SetCmUpstream,    

    CMCONU_BCM55538_CreateCmServiceFlow,    
    CMCONU_BCM55538_ModifyCmServiceFlow,    
    CMCONU_BCM55538_DestroyCmServiceFlow,    
    CMCONU_BCM55538_DumpCmClassifier,    
    CMCONU_BCM55538_DumpCmServiceFlow,   
    
    CMCONU_BCM55538_DumpCmcClassifier,    
    CMCONU_BCM55538_DumpCmcServiceFlow,    
    CMCONU_BCM55538_DumpCmcServiceFlowStatistics,    
    CMCONU_BCM55538_DumpCmcDownChannelBondingGroup,    
    CMCONU_BCM55538_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_BCM55538_CreateServiceFlowClassName,    
    CMCONU_BCM55538_DestroyServiceFlowClassName,    
    CMCONU_BCM55538_GetCmcMacAddrTbl,    
    CMCONU_BCM55538_GetCmMacAddrTbl,    
    CMCONU_BCM55538_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
#if 0
        /*----------------GPON OMCI----------------------*/
        NULL,
        NULL,
#endif
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,

	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	   
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
    /* --------------------END------------------------ */

    REMOTE_ERROR
};

static const OnuMgmtIFs s_ctconu55538Ifs = {
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_BCM55538_OnuIsOnline,
    GWONU_BCM55538_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_BCM55538_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_BCM55538_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_BCM55538_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_BCM55538_AuthorizeOnu,
    CTCONU_BCM55538_AuthRequest,
    CTCONU_BCM55538_AuthSuccess,
    CTCONU_BCM55538_AuthFailure,
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    CTCONU_BCM55538_SetOnuTrafficServiceMode,
    GWONU_BCM55538_SetOnuPeerToPeer,
    GWONU_BCM55538_SetOnuPeerToPeerForward,
    GWONU_BCM55538_SetOnuBW,
    GWONU_BCM55538_GetOnuSLA,

    CTCONU_BCM55538_SetOnuFecMode,
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_BCM55538_SetUniPort,
    GWONU_BCM55538_SetSlowProtocolLimit,
    GWONU_BCM55538_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    GWONU_BCM55538_GetOnuB2PMode,
    GWONU_BCM55538_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_BCM55538_ResetCounters,
    GWONU_BCM55538_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_BCM55538_GetLLIDParams,
    GWONU_BCM55538_StartEncryption,
    GWONU_BCM55538_StopEncryption,
    GWONU_BCM55538_SetOnuEncryptParams,
    GWONU_BCM55538_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_BCM55538_GetOnuMacAddrTbl,
    GWONU_BCM55538_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_BCM55538_SetOnuMaxMacNum,
    GWONU_BCM55538_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_BCM55538_GetOnuDistance,
    GWONU_BCM55538_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_BCM55538_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    GWONU_BCM55538_GetOnuPonVer,
    GWONU_BCM55538_GetOnuRegisterInfo,
    GWONU_BCM55538_GetOnuI2CInfo,
    GWONU_BCM55538_SetOnuI2CInfo,
    
    CTCONU_BCM55538_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    GWONU_BCM55538_GetBurnImageComplete,
    
    /*CTCONU_BCM55538_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_BCM55538_GetCtcVersion,
    CTCONU_BCM55538_GetFirmwareVersion,
    CTCONU_BCM55538_GetSerialNumber,
    CTCONU_BCM55538_GetChipsetID,

    CTCONU_BCM55538_GetOnuCap1,
    CTCONU_BCM55538_GetOnuCap2,
    CTCONU_BCM55538_GetOnuCap3,
    
    CTCONU_BCM55538_UpdateOnuFirmware,
    CTCONU_BCM55538_ActiveOnuFirmware,
    CTCONU_BCM55538_CommitOnuFirmware,
    
    CTCONU_BCM55538_StartEncrypt,
    CTCONU_BCM55538_StopEncrypt,
    
    CTCONU_BCM55538_GetEthPortLinkstate,
    CTCONU_BCM55538_GetEthPortAdminstatus,
    CTCONU_BCM55538_SetEthPortAdminstatus,
    
    CTCONU_BCM55538_GetEthPortPauseEnable,
    CTCONU_BCM55538_SetEthPortPauseEnable,
    
    CTCONU_BCM55538_GetEthPortAutoNegotinationAdmin,
    CTCONU_BCM55538_SetEthPortAutoNegotinationAdmin,
    CTCONU_BCM55538_SetEthPortRestartAutoConfig,
    CTCONU_BCM55538_GetEthPortAnLocalTecAbility,
    CTCONU_BCM55538_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_BCM55538_GetEthPortPolicing,
    CTCONU_BCM55538_SetEthPortPolicing,
    
    CTCONU_BCM55538_GetEthPortDownstreamPolicing,
    CTCONU_BCM55538_SetEthPortDownstreamPolicing,

    CTCONU_BCM55538_GetEthPortVlanConfig,
    CTCONU_BCM55538_SetEthPortVlanConfig,
    CTCONU_BCM55538_GetAllPortVlanConfig,
    
    CTCONU_BCM55538_GetEthPortClassificationAndMarking,
    CTCONU_BCM55538_SetEthPortClassificationAndMarking,
    CTCONU_BCM55538_ClearEthPortClassificationAndMarking,
    
    CTCONU_BCM55538_GetEthPortMulticastVlan,
    CTCONU_BCM55538_SetEthPortMulticastVlan,
    CTCONU_BCM55538_ClearEthPortMulticastVlan,

    CTCONU_BCM55538_GetEthPortMulticastGroupMaxNumber,
    CTCONU_BCM55538_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_BCM55538_GetEthPortMulticastTagStrip,
    CTCONU_BCM55538_SetEthPortMulticastTagStrip,
    CTCONU_BCM55538_GetMulticastAllPortTagStrip,

    CTCONU_BCM55538_GetEthPortMulticastTagOper,
    CTCONU_BCM55538_SetEthPortMulticastTagOper,
    CTCONU_BCM55538_SetObjMulticastTagOper,
    
    CTCONU_BCM55538_GetMulticastControl,
    CTCONU_BCM55538_SetMulticastControl,
    CTCONU_BCM55538_ClearMulticastControl,
    
    CTCONU_BCM55538_GetMulticastSwitch,
    CTCONU_BCM55538_SetMulticastSwitch,
    
    CTCONU_BCM55538_GetFastLeaveAbility,
    CTCONU_BCM55538_GetFastLeaveAdminState,
    CTCONU_BCM55538_SetFastLeaveAdminState,
    
    CTCONU_BCM55538_GetOnuPortStatisticData,
    CTCONU_BCM55538_GetOnuPortStatisticState,
    CTCONU_BCM55538_SetOnuPortStatisticState,
    
    CTCONU_BCM55538_SetAlarmAdminState,
    CTCONU_BCM55538_SetAlarmThreshold,
    CTCONU_BCM55538_GetDbaReportThresholds,
    CTCONU_BCM55538_SetDbaReportThresholds,
    
    CTCONU_BCM55538_GetMxuMngGlobalConfig,
    CTCONU_BCM55538_SetMxuMngGlobalConfig,
    CTCONU_BCM55538_GetMxuMngSnmpConfig,
    CTCONU_BCM55538_SetMxuMngSnmpConfig,

    CTCONU_BCM55538_GetHoldover,
    CTCONU_BCM55538_SetHoldover,
    CTCONU_BCM55538_GetOptTransDiag,
    CTCONU_BCM55538_SetTxPowerSupplyControl,

    CTCONU_BCM55538_GetFecAbility,

    CTCONU_BCM55538_GetIADInfo,
    CTCONU_BCM55538_GetVoipIadOperStatus,
    CTCONU_BCM55538_SetVoipIadOperation,
    CTCONU_BCM55538_GetVoipGlobalConfig,
    CTCONU_BCM55538_SetVoipGlobalConfig,
    CTCONU_BCM55538_GetVoipFaxConfig,
    CTCONU_BCM55538_SetVoipFaxConfig,
    
    CTCONU_BCM55538_GetVoipPortStatus,
    CTCONU_BCM55538_GetVoipPort,
    CTCONU_BCM55538_SetVoipPort,
    CTCONU_BCM55538_GetVoipPort2,
    CTCONU_BCM55538_SetVoipPort2,
    
    CTCONU_BCM55538_GetH248Config,
    CTCONU_BCM55538_SetH248Config,
    CTCONU_BCM55538_GetH248UserTidConfig,
    CTCONU_BCM55538_SetH248UserTidConfig,
    CTCONU_BCM55538_GetH248RtpTidConfig,
    CTCONU_BCM55538_SetH248RtpTidConfig,
    
    CTCONU_BCM55538_GetSipConfig,
    CTCONU_BCM55538_SetSipConfig,
    CTCONU_BCM55538_SetSipDigitMap,
    CTCONU_BCM55538_GetSipUserConfig,
    CTCONU_BCM55538_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID,
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL,           /* CliCall */

    CTCONU_BCM55538_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    NULL,           /* SetPasFlush */
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_BCM55538_SetVlanEnable,
    CTCONU_BCM55538_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_BCM55538_DelVlan,
    CTCONU_BCM55538_SetPortPvid,

    CTCONU_BCM55538_AddVlanPort,
    CTCONU_BCM55538_DelVlanPort,
    CTCONU_BCM55538_SetVlanTran,
    CTCONU_BCM55538_DelVlanTran,
    CTCONU_BCM55538_SetVlanAgg,
    CTCONU_BCM55538_DelVlanAgg,
    
    CTCONU_BCM55538_SetPortQinQEnable,
    CTCONU_BCM55538_AddQinQVlanTag,
    CTCONU_BCM55538_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_BCM55538_SetPortMode,
    CTCONU_BCM55538_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_BCM55538_SetPortIngressRate,
    CTCONU_BCM55538_SetPortEgressRate,
    
    REMOTE_OK,/*GWONU_SetQosClass,*/
    REMOTE_OK,/*GWONU_ClrQosClass,*/
    REMOTE_OK,/*GWONU_SetQosRule,*/
    REMOTE_OK,/*GWONU_ClrQosRule,*/
    
    CTCONU_BCM55538_SetPortQosRule,
    CTCONU_BCM55538_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_BCM55538_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_BCM55538_SetIgmpEnable,
    CTCONU_BCM55538_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_BCM55538_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    CMCONU_BCM55538_RegisterCmc,
    CMCONU_BCM55538_UnregisterCmc,
    CMCONU_BCM55538_DumpCmc,
    CMCONU_BCM55538_DumpAlarm,
    CMCONU_BCM55538_DumpLog,
    
    CMCONU_BCM55538_ResetCmcBoard,
    CMCONU_BCM55538_GetCmcVersion,
    CMCONU_BCM55538_GetCmcMaxMulticasts,
    CMCONU_BCM55538_GetCmcMaxCm,
    CMCONU_BCM55538_SetCmcMaxCm,
    
    CMCONU_BCM55538_GetCmcTime,
    CMCONU_BCM55538_SetCmcTime,
    CMCONU_BCM55538_LocalCmcTime,
    CMCONU_BCM55538_SetCmcCustomConfig,
    CMCONU_BCM55538_DumpCmcCustomConfig,
    
    CMCONU_BCM55538_DumpCmcDownChannel,
    CMCONU_BCM55538_DumpCmcUpChannel,
    CMCONU_BCM55538_GetCmcDownChannelMode,
    CMCONU_BCM55538_SetCmcDownChannelMode,
    CMCONU_BCM55538_GetCmcUpChannelMode,

    CMCONU_BCM55538_SetCmcUpChannelMode,
    CMCONU_BCM55538_GetCmcUpChannelD30Mode,
    CMCONU_BCM55538_SetCmcUpChannelD30Mode,
    CMCONU_BCM55538_GetCmcDownChannelFreq,
    CMCONU_BCM55538_SetCmcDownChannelFreq,

    CMCONU_BCM55538_GetCmcUpChannelFreq,
    CMCONU_BCM55538_SetCmcUpChannelFreq,
    NULL,           /* GetCmcDownAutoFreq */
    CMCONU_BCM55538_SetCmcDownAutoFreq,
    NULL,           /* GetCmcUpAutoFreq */

    CMCONU_BCM55538_SetCmcUpAutoFreq,
    CMCONU_BCM55538_GetCmcUpChannelWidth,
    CMCONU_BCM55538_SetCmcUpChannelWidth,
    CMCONU_BCM55538_GetCmcDownChannelAnnexMode,
    CMCONU_BCM55538_SetCmcDownChannelAnnexMode,
    
    CMCONU_BCM55538_GetCmcUpChannelType,
    CMCONU_BCM55538_SetCmcUpChannelType,
    CMCONU_BCM55538_GetCmcDownChannelModulation,
    CMCONU_BCM55538_SetCmcDownChannelModulation,
    CMCONU_BCM55538_GetCmcUpChannelProfile,
    
    CMCONU_BCM55538_SetCmcUpChannelProfile,
    CMCONU_BCM55538_GetCmcDownChannelInterleaver,
    CMCONU_BCM55538_SetCmcDownChannelInterleaver,
    CMCONU_BCM55538_GetCmcDownChannelPower,    
    CMCONU_BCM55538_SetCmcDownChannelPower,    

    CMCONU_BCM55538_GetCmcUpChannelPower,    
    CMCONU_BCM55538_SetCmcUpChannelPower,    
    CMCONU_BCM55538_DumpCmcUpChannelPower,    
    CMCONU_BCM55538_DumpCmcUpChannelSignalQuality,    
    CMCONU_BCM55538_DumpCmcInterfaceUtilization,    

    CMCONU_BCM55538_DumpCmcInterfaceStatistics,    
    CMCONU_BCM55538_DumpCmcMacStatistics,    
    CMCONU_BCM55538_DumpCmcAllInterface,    
    CMCONU_BCM55538_DumpAllLoadBalancingGrp,    
    CMCONU_BCM55538_DumpLoadBalancingGrp,    

    CMCONU_BCM55538_DumpLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_DumpLoadBalancingGrpUpstream,    
    CMCONU_BCM55538_DumpLoadBalancingDynConfig,    
    CMCONU_BCM55538_SetLoadBalancingDynMethod,    
    CMCONU_BCM55538_SetLoadBalancingDynPeriod,    

    CMCONU_BCM55538_SetLoadBalancingDynWeightedAveragePeriod,    
    CMCONU_BCM55538_SetLoadBalancingDynOverloadThresold,    
    CMCONU_BCM55538_SetLoadBalancingDynDifferenceThresold,    
    CMCONU_BCM55538_SetLoadBalancingDynMaxMoveNumber,    
    CMCONU_BCM55538_SetLoadBalancingDynMinHoldTime,    

    CMCONU_BCM55538_SetLoadBalancingDynRangeOverrideMode,    
    CMCONU_BCM55538_SetLoadBalancingDynAtdmaDccInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynScdmaDccInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynAtdmaDbcInitTech,    
    CMCONU_BCM55538_SetLoadBalancingDynScdmaDbcInitTech,    

    CMCONU_BCM55538_CreateLoadBalancingGrp,    
    CMCONU_BCM55538_DestroyLoadBalancingGrp,    
    CMCONU_BCM55538_AddLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpDownstream,    
    CMCONU_BCM55538_AddLoadBalancingGrpUpstream,    

    CMCONU_BCM55538_RemoveLoadBalancingGrpUpstream,    
    CMCONU_BCM55538_AddLoadBalancingGrpModem,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpModem,    
    CMCONU_BCM55538_AddLoadBalancingGrpExcludeModem,    
    CMCONU_BCM55538_RemoveLoadBalancingGrpExcludeModem,    

    CMCONU_BCM55538_DumpLoadBalancingGrpModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpActivedModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpExcludeModem,    
    CMCONU_BCM55538_DumpLoadBalancingGrpExcludeActivedModem,    
    NULL,           /* ReserveCmcLoadBalancingGrp */

    CMCONU_BCM55538_DumpAllCm,    
    CMCONU_BCM55538_DumpCm,    
    CMCONU_BCM55538_DumpAllCmHistory,    
    CMCONU_BCM55538_DumpCmHistory,    
    CMCONU_BCM55538_ClearAllCmHistory,    

    CMCONU_BCM55538_ResetCm,    
    CMCONU_BCM55538_DumpCmDownstream,    
    CMCONU_BCM55538_DumpCmUpstream,    
    CMCONU_BCM55538_SetCmDownstream,    
    CMCONU_BCM55538_SetCmUpstream,    

    CMCONU_BCM55538_CreateCmServiceFlow,    
    CMCONU_BCM55538_ModifyCmServiceFlow,    
    CMCONU_BCM55538_DestroyCmServiceFlow,    
    CMCONU_BCM55538_DumpCmClassifier,    
    CMCONU_BCM55538_DumpCmServiceFlow,    
    
    CMCONU_BCM55538_DumpCmcClassifier,    
    CMCONU_BCM55538_DumpCmcServiceFlow,    
    CMCONU_BCM55538_DumpCmcServiceFlowStatistics,    
    CMCONU_BCM55538_DumpCmcDownChannelBondingGroup,    
    CMCONU_BCM55538_DumpCmcUpChannelBondingGroup,    
    
    CMCONU_BCM55538_CreateServiceFlowClassName,    
    CMCONU_BCM55538_DestroyServiceFlowClassName,    
    CMCONU_BCM55538_GetCmcMacAddrTbl,    
    CMCONU_BCM55538_GetCmMacAddrTbl,    
    CMCONU_BCM55538_ResetCmAddrTbl,    
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
  REMOTE_OK,
  REMOTE_OK,
  REMOTE_OK,
  REMOTE_OK,
  REMOTE_OK,
  REMOTE_OK,
#if 0
    /*----------------GPON OMCI----------------------*/
    NULL,
    NULL,
#endif
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,
   REMOTE_OK,

	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	   
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
	REMOTE_OK,
/* --------------------END------------------------ */

    REMOTE_ERROR
};


void GWONU_BCM55538_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_BCM55538_GW, &s_gwonu55538Ifs);
}

void CTCONU_BCM55538_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_BCM55538_CTC, &s_ctconu55538Ifs);
}



#ifdef __cplusplus

}

#endif


#endif

