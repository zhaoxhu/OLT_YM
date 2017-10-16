/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_8411.c
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
*   Date: 			2010/05/13
*   Author:		shixh
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  12/11/21    |   jinhl    |     create 
**----------|-----------|------------------
***************************************************************/
#if defined(_EPON_10G_PMC_SUPPORT_)            

/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "syscfg.h"
#include  "vos/vospubh/vos_base.h"
#include  "vos/vospubh/vos_sem.h"
#include  "includefromPas.h"
#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include  "PonGeneral.h"
#include  "cli/cl_vty.h"
#include "onu/onuOamUpd.h"
#include "onu/onuConfMgt.h"
#include "onu/Onu_oam_comm.h"
#include "ct_manage/CT_Onu_Voip.h"
#include "../onu/OnuPortStatsMgt.h"

extern int g_iOnuBwSetFirstFailed;
extern int g_iOnuBwSetSecondFailed;
extern int g_iOnuBwSetFailedDelay;

extern int ctc_voip_debug ;
#define CTC_VOIP_DEBUG if(ctc_voip_debug) sys_console_printf
extern PON_onu_address_table_record_t  MAC_Address_Table[8192];
/*extern USHORT onu_mac_check_counter[SYS_MAX_PON_PORTNUM][MAXONUPERPON];*/

/****************************************内部适配*******************************************/

#if 1
/* -------------------ONU基本API------------------- */

static int REMOTE_OK(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"REMOTE_OK(%d, %d)'s result(0).\r\n", olt_id, onu_id);

    return OLT_ERR_OK;
}

static int REMOTE_ERROR(short int olt_id, short int onu_id)
{
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"REMOTE_ERROR(%d, %d)'s result(%d).\r\n", olt_id, onu_id, OLT_ERR_NOTEXIST);

    return OLT_ERR_NOTEXIST;
}

static int GWONU_PAS8411_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    int iRlt = 0;
    short int llid_id;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    if ( INVALID_LLID != (llid_id = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        *status = 1;
    }
    else
    {
        *status = 0;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_OnuIsOnline(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* ONU 手动添加*//*add by shixh20100908*/
static int GWONU_PAS8411_AddOnuByManual(short int olt_id, short int onu_id, unsigned char *MacAddr)
{
    short int OnuEntry;
    short int llid;
    unsigned char *pSrcMac;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(MacAddr);

    OnuEntry = olt_id * MAXONUPERPON + onu_id;    
    pSrcMac  = OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr;
    if ( 0 != VOS_MemCmp(pSrcMac, MacAddr, BYTES_IN_MAC_ADDRESS) )
    {
        /* 强行霸占记录地址 */
        if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            GW10G_PAS_deregister_onu( olt_id, llid, FALSE );
        }
        OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_AddOnuByManual(%d,%d) deregister onu(%s) for onu(%s) on slot %d.\r\n", olt_id, onu_id, macAddress_To_Strings(pSrcMac), macAddress_To_Strings(MacAddr), SYS_LOCAL_MODULE_SLOTNO);
    }   

    return( ROK );
}

/* 删除ONU */
static int GWONU_PAS8411_DelOnuByManual(short int olt_id, short int onu_id)
{	
    short int OnuEntry;
    short int llid;
      
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( activeOneLocalPendingOnu(olt_id, onu_id) == RERROR )
	{
	    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	    {
	        GW10G_PAS_deregister_onu( olt_id, llid, FALSE );
	    }
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return ROK;
}


static int GWONU_PAS8411_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS8411;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_PAS8411_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS8411;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
/*onu deregister*/
static int GWONU_PAS8411_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_PAS_deregister_onu( olt_id, llid, FALSE );
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_DeregisterOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GWONU_PAS8411_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( auth_mode )
        {
            iRlt = GW10G_PAS_authorize_onu(olt_id, llid, PON_AUTHORIZE_TO_THE_NETWORK);
        }
        else
        {
            iRlt = GW10G_PAS_authorize_onu(olt_id, llid, PON_DENY_FROM_THE_NETWORK);
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_AuthorizeOnu(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_PAS8411_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_auth_request(olt_id, llid, auth_response);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_AuthRequest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

static int CTCONU_PAS8411_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

	OLT_ASSERT(olt_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_auth_success(olt_id, llid);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_AuthSuccess(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS8411_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
    
	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_auth_failure(olt_id, llid, failure_type);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_AuthFailure(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, failure_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
/*traffic service enable*/

static int GWONU_PAS8411_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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
	        if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	        {
	            iRlt = GW10G_PAS_deregister_onu( olt_id, llid, FALSE );
	        }
	        else
	            return OLT_ERR_PARAM;
	    }
    }
    else
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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

        if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            iRlt = GW10G_PAS_deregister_onu( olt_id, llid, FALSE );
        }
    }
    else 
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P, 5001和5201共用*/
static int GWONU_PAS8411_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int Llid1, Llid2;
	short int llidArray[2];
    bool access;

	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id1);
	ONU_ASSERT(onu_id2);

	Llid1 = GetLlidActivatedByOnuIdx(olt_id, onu_id1);
	Llid2 = GetLlidActivatedByOnuIdx(olt_id, onu_id2);
	if((Llid1 != INVALID_LLID) && (Llid2 != INVALID_LLID))
    {
    	llidArray[0] = Llid2;
        access = (V2R1_ENABLE == enable) ? TRUE : FALSE;
    	iRlt = GW10GAdp_set_p2p_access_control( olt_id, Llid1, 1, llidArray, access );
    }
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id1, onu_id2, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P的ONU，是否转发无效MAC，5001和5201共用*/  /*问题单11315*/
static int GWONU_PAS8411_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast)
{
    int iRlt = OLT_ERR_NOTEXIST;
	bool address_b, broadcast_b;
	short int llid;

	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	address_b   = (V2R1_ENABLE == address_not_found) ? TRUE : FALSE;
    	broadcast_b = (V2R1_ENABLE == broadcast) ? TRUE : FALSE; 
    	
    	iRlt = GW10GAdp_set_llid_p2p_configuration( olt_id, llid, address_b, broadcast_b );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, address_not_found, broadcast, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int GWONU_platoDBA4_3_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    PLATO4_SLA_t  V4_SLA;
    short DBA_error_code = 0;
    int bw_val;

    if( BW->bw_class > PRECEDENCE_OF_FLOW_7 )
    	V4_SLA.class = OnuConfigDefault.UplinkClass;
    else
    	V4_SLA.class = BW->bw_class;
    	
    V4_SLA.fixed_packet_size = uplinkBWPacketUnitSize;

    bw_val = BW->bw_fixed;			
    V4_SLA.fixed_bw = bw_val / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V4_SLA.fixed_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V4_SLA.fixed_bw_fine++;

    bw_val = BW->bw_gr;/*bw_val = (BW->bw_gr * Bata_ratio) / 1000 ;*//*当超过5G时越界，暂时去掉for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
    V4_SLA.max_gr_bw = bw_val / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V4_SLA.max_gr_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V4_SLA.max_gr_bw_fine++;

    bw_val = BW->bw_be;/*bw_val = (BW->bw_be * Bata_ratio) / 1000 ;*//*当超过5G时越界，暂时去掉for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
    V4_SLA.max_be_bw = bw_val  / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V4_SLA.max_be_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V4_SLA.max_be_bw_fine++;

    if(V4_SLA.max_be_bw == ( V4_SLA.fixed_bw + V4_SLA.max_gr_bw ))
    {
    	if( V4_SLA.max_be_bw_fine < (V4_SLA.fixed_bw_fine + V4_SLA.max_gr_bw_fine ) )
    	{
    		V4_SLA.max_be_bw_fine = V4_SLA.fixed_bw_fine + V4_SLA.max_gr_bw_fine;
    	}
    }
    else if(V4_SLA.max_be_bw < ( V4_SLA.fixed_bw + V4_SLA.max_gr_bw ))
    {
    	V4_SLA.max_be_bw = V4_SLA.fixed_bw + V4_SLA.max_gr_bw;
    	V4_SLA.max_be_bw_fine = V4_SLA.fixed_bw_fine + V4_SLA.max_gr_bw_fine;
    }

    iRlt = PLATO4_set_SLA(olt_id, llid, (PLATO4_SLA_t *)&V4_SLA, &DBA_error_code) ;
    if( iRlt != PAS_EXIT_OK )
    {
        g_iOnuBwSetFirstFailed++;

        if ( g_iOnuBwSetFailedDelay > 0 )
        {
            VOS_TaskDelay( (ULONG)((VOS_TICK_SECOND * g_iOnuBwSetFailedDelay) / 1000) );
        }
        
        /* 连续设置2次，就能保证不成功的成功? (陈福军的代码) */
    	iRlt = PLATO4_set_SLA(olt_id, llid, (PLATO4_SLA_t *)&V4_SLA, &DBA_error_code) ;
    }

    if (PAS_EXIT_OK == iRlt)
    {
        /* B--added by liwei056@2010-1-13 for D9425 */
        if ( DBA_error_code != PLATO4_ECODE_NO_ERROR )
        {
            /* B--modified by liwei056@2010-11-30 for D11310 */
            iRlt = OLT_ERR_DBA_BEGIN - DBA_error_code;
            /* E--modified by liwei056@2010-11-30 for D11310 */
        }
        /* E--added by liwei056@2010-1-13 for D9425 */
    }
    else
    {
        g_iOnuBwSetSecondFailed++;
    }
        
    return iRlt;
}


static int GWONU_PAS8411_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    PON_policer_t  policer_id;
    PON_policing_parameters_t  policing_param;

    policer_id = PON_POLICER_DOWNSTREAM_TRAFFIC;

    if ( BW->bw_gr > 0 )
    {
        policing_param.maximum_bandwidth = BW->bw_gr;
    }
    else
    {
        policing_param.maximum_bandwidth = BW->bw_be;
    }
    
    /* B--modified by liwei056@2010-11-30 for D11311 */
	if ( V2R1_ENABLE == PonPortTable[olt_id].DownlinkPoliceMode /* downlinkBWlimit */ )
    {
        policing_param.maximum_burst_size = downlinkBWlimitBurstSize; /* 0~16777215 */
        policing_param.high_priority_frames_preference = downlinkBWlimitPreference;
    }
    else
    {
        policing_param.maximum_burst_size = ONU_DOWNLINK_POLICE_BURSESIZE_DEFAULT;
        policing_param.high_priority_frames_preference = DISABLE;
    }
    /* E--modified by liwei056@2010-11-30 for D11311 */
    
    if( policing_param.maximum_bandwidth > PAS5201_MAX_DOWNLINK_SPEED )
    	policing_param.maximum_bandwidth = PAS5201_MAX_DOWNLINK_SPEED;

    if( (policing_param.maximum_bandwidth > 0)
        && ((PonPortTable[olt_id].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_ENABLE)
        || (OLT_CFG_DIR_INACTIVE & BW->bw_direction)) )
    {
    	iRlt = GW10GAdp_set_policing_parameters(olt_id, llid, policer_id, ENABLE, policing_param);
        if (0 == iRlt)
        {
            /* 输出激活带宽 */
            BW->bw_actived = BW->bw_gr;
        }
    }
    else
    {
        iRlt = GW10GAdp_set_policing_parameters(olt_id, llid, policer_id, DISABLE, policing_param);
        if (0 == iRlt)
        {
            /* 输出激活带宽 */
            BW->bw_actived = 0;
        }
    }

    return iRlt;
}

static int  GWONU_PAS_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW, int chip_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
           
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
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
                iRlt = GWONU_PAS8411_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                
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

                    iRlt = GWONU_platoDBA4_3_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);


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
                                
                                (void)GWONU_PAS8411_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                                
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
                

                iRlt = GWONU_platoDBA4_3_SetOnuUplinkBW(olt_id, llid, BW);


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
                
                iRlt = GWONU_PAS8411_SetOnuDownlinkBW(olt_id, llid, BW);
                
            }
        }
    }   
    
    return iRlt;
}


static int  GWONU_PAS8411_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(BW);
    
    if(OLT_CFG_DIR_DO_ACTIVE & BW->bw_direction)
    {
        iRlt = GWONU_PAS_SetOnuBW(olt_id, onu_id, BW, 0);
    }
    else        
    {
        iRlt = 0;
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS8411_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        sla_info->SLA_Ver = 4;
	    iRlt = PLATO4_get_SLA( olt_id, (unsigned char)llid, &(sla_info->SLA.SLA4), &sla_info->DBA_ErrCode);

    }   
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_GetOnuSLA(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, sla_info->SLA_Ver, sla_info->DBA_ErrCode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS8411_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int sRlt;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	if( fec_mode == STD_FEC_MODE_ENABLED )
    	{
    		sRlt = GW10G_PAS_set_llid_fec_mode(olt_id, PON_BROADCAST_LLID, ENABLE);
    		iRlt = GW10G_PAS_set_llid_fec_mode(olt_id, llid, ENABLE);
    	}
    	else
    	{
    		PON_fec_status_raw_data_t fec_status;
    		PON_timestamp_t           timestamp; 
    			
    		iRlt = GW10G_PAS_set_llid_fec_mode(olt_id, llid, DISABLE);
    		sRlt = GW10GAdp_get_raw_statistics( olt_id,
                                   	PON_OLT_ID,
                                   	PON_RAW_STAT_FEC_STATUS,
                                   	0,
                                  	&fec_status,
                                   	&timestamp);
    		if( PAS_EXIT_OK == sRlt )
            {
        		if( 0 == fec_status.number_of_llids_with_fec ) 
                {
                    /* No ONUs with FEC disable broadcast LLID */
    				sRlt = GW10G_PAS_set_llid_fec_mode(olt_id, PON_BROADCAST_LLID, DISABLE);
                }      
            }
    	}
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int CTCONU_PAS8411_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        CTC_STACK_standard_FEC_ability_t Fec_Ability;

        iRlt = GW10G_CTC_STACK_get_fec_ability(olt_id, llid, &Fec_Ability);    
    	if ( (iRlt != 0)
            || (Fec_Ability != STD_FEC_ABILITY_SUPPORTED) )
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            iRlt = GW10G_CTC_STACK_set_fec_mode(olt_id, llid, (CTC_STACK_standard_FEC_mode_t)fec_mode);    
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

/*get onu vlan mode*/
static int GWONU_PAS8411_GetOnuVlanMode(short int olt_id, short int onu_id, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(vlan_uplink_config);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	iRlt = GW10GAdp_get_llid_vlan_mode( olt_id, llid, vlan_uplink_config );
    }   
	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuVlanMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;	
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS8411_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    
        iRlt = GW10G_REMOTE_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetUniPort(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS8411_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    
        iRlt = GW10G_REMOTE_PASONU_set_slow_protocol_limit( olt_id, llid, enable);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetSlowProtocolLimit(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */

static int GWONU_PAS8411_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
	   
    	if ( 0 == (iRlt = GW10G_REMOTE_PASONU_clear_all_statistics ( olt_id, llid )) )
        {
            /* B--added by liwei056@2011-7-11 for D12785 */
            GW10G_PAS_reset_olt_counters(olt_id);
            /* E--added by liwei056@2011-7-11 for D12785 */
        }  
		
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_ResetCounters(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GWONU_PAS8411_SetPonLoopback(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
#if 1
    short int llid;
    bool loop_mode = FALSE;
  	/*PON_address_table_config_t  address_table_config;*/
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	if( enable == V2R1_ENABLE )
		loop_mode = TRUE;

	/*VOS_MemZero( &address_table_config, sizeof(address_table_config) );
	if( PAS_get_address_table_configuration(olt_id, &address_table_config) == PAS_EXIT_OK )
	{
		address_table_config.allow_learning = PON_DIRECTION_UPLINK_AND_DOWNLINK;
		PAS_set_address_table_configuration(olt_id, address_table_config);
	}*/

    	iRlt = GW10G_PAS_set_standard_onu_loopback( olt_id, llid, loop_mode );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetPonLoopback(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

#else
#define LOOP_PON_LINK_TEST_CONTINUOUS  (-1)    /* Start a continuous link test 			     */
#define LOOP_PON_LINK_TEST_OFF		  0     /* Stop an already running continous link test */
	short int llid;
	PON_link_test_vlan_configuration_t vlan_configuratiion;
	PON_link_test_results_t  test_results;
	short int frame_size = 100;
	short int test_mode = LOOP_PON_LINK_TEST_OFF;
	
	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
		VOS_MemZero( &vlan_configuratiion, sizeof(PON_link_test_vlan_configuration_t));
		VOS_MemZero( &test_results, sizeof(test_results));
		vlan_configuratiion.vlan_frame_enable = FALSE;
		vlan_configuratiion.vlan_priority = 0;
		vlan_configuratiion.vlan_tag = 0;
	    	if( enable == V2R1_ENABLE )
			test_mode = LOOP_PON_LINK_TEST_CONTINUOUS;
		
		iRlt = PAS_link_test ( olt_id, llid,  LOOP_PON_LINK_TEST_CONTINUOUS, frame_size, TRUE, &vlan_configuratiion, &test_results );
	}
#endif
    return iRlt;
}


#endif


#if 1
/* -------------------ONU加密管理API------------------- */
/* get onu pon Llid parameters */
static int GWONU_PAS8411_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	iRlt = GW10G_PAS_get_llid_parameters ( olt_id, llid, llid_parameters );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetLLIDParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*onu start encryption */
static int GWONU_PAS8411_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int enc_dir;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(encrypt_dir);

    enc_dir = *encrypt_dir;
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	if ( 0 == (iRlt = GW10G_PAS_start_encryption ( olt_id, llid, (PON_encryption_direction_t)(enc_dir - PON_ENCRYPTION_PURE) )) )
        {
            /* 标识加密已经启动 */
            *encrypt_dir = 0;
        }   
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_StartEncryption(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

/*onu stop encryption*/
static int GWONU_PAS8411_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	iRlt = GW10G_PAS_stop_encryption ( olt_id, llid );
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_StopEncryption(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

static int GWONU_PAS8411_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
{
    int iRlt = 0;
    int enc_dir;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( NULL != encrypt_dir )
    {
        enc_dir = *encrypt_dir;
    	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {    			
            PON_llid_parameters_t llid_parameters;

        	if ( 0 == (iRlt = GW10G_PAS_get_llid_parameters ( olt_id, llid, &llid_parameters )) )
            {
                PON_encryption_direction_t new_dir;
                PON_encryption_direction_t old_dir;
                
                if ( PON_ENCRYPTION_PURE < enc_dir )
                {
                    new_dir = (PON_encryption_direction_t)(enc_dir - PON_ENCRYPTION_PURE);
                }
                else
                {
                    new_dir = PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION;
                }
                
                if ( new_dir != (old_dir = llid_parameters.encryption_mode) )
                {
                    if ( PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION != old_dir )
                    {
                        iRlt = GW10G_PAS_stop_encryption ( olt_id, llid );
                    }
                    
                    if ( PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION != new_dir )
                    {
                        if ( (PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION != old_dir)
                            && (0 == iRlt) )
                        {
                            extern int EncryptKeyDelayTime;
                        
                            /* 加密方向变化，需延时0.5秒*/
                  	        VOS_TaskDelay(EncryptKeyDelayTime); 
                        }
                        
                    	if ( 0 == (iRlt = GW10G_PAS_start_encryption ( olt_id, llid, new_dir )) )
                        {
                            /* 标识加密已经启动 */
                            *encrypt_dir = 0;
                        }   
                    }
                }   
            }
        }   
        else
        {
            iRlt = OLT_ERR_NOTEXIST;
        }
    }
    else
    {
        enc_dir = -1;
    }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, key_change_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

static int GWONU_PAS8411_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int iRlt;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

#if 0
    if ( NULL != encrypt_dir )
    {
    	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {    			
            PON_llid_parameters_t llid_parameters;
            
        	if ( 0 == (iRlt = PAS_get_llid_parameters ( olt_id, llid, &llid_parameters )) )
            {
                if ( PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION == llid_parameters.encryption_mode )
                {
                    *encrypt_dir = PON_ENCRYPTION_PURE;
                }
                else
                {
                    *encrypt_dir = llid_parameters.encryption_mode + PON_ENCRYPTION_PURE;
                }

                /* 此参数不必再获取 */
                encrypt_dir = NULL;
            }   
        }
    }
#else
    iRlt = GWONU_GetOnuEncryptParams(olt_id, onu_id, encrypt_dir, key_change_time, encrypt_status);
    if ( NULL != encrypt_status )
    {
    	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {    			
            PON_llid_parameters_t llid_parameters;
            
        	if ( 0 == (iRlt = GW10G_PAS_get_llid_parameters ( olt_id, llid, &llid_parameters )) )
            {
                if ( PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION == llid_parameters.encryption_mode )
                {
                    *encrypt_status = 2;
                }
                else
                {
                    *encrypt_status = 1;
                }
            }   
        }
    }
#endif
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuEncryptParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS8411_UpdateEncryptionKey(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method)
{
    int iRlt;
    short int llid;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
		iRlt = GW10G_PAS_update_encryption_key(olt_id, llid, encryption_key, key_update_method);
	}
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_UpdateEncryptionKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* -------------------ONU 地址管理API------------------- */
/*show fdbentry mac*/
static int GWONU_PAS8411_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        long int liEntryNum;
        
        if ( NULL == addr_table )
        {
            addr_table = MAC_Address_Table;
        }
		
        if ( 0 == (iRlt = GW10G_REMOTE_PASONU_address_table_get(olt_id, llid, &liEntryNum, addr_table)) )
        {
            *EntryNum = (long)liEntryNum;
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}


/*show fdbentry mac*/
static int GWONU_PAS8411_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	short int addr_num;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( NULL == addr_table )
        {
            addr_table = Mac_addr_table;
        }
        if ( 0 == (iRlt = GW10GAdp_get_address_table_llid(olt_id, llid, &addr_num, addr_table)) )
        {
            *EntryNum = (long)addr_num;
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*set onu max mac*/

static int GWONU_PAS8411_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned int uiSetVal;
    short int llid;
    short int max_entry;
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

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
    	PON_address_table_config_t address_table_config;
        
        iRlt = PAS_get_address_table_configuration( olt_id, &address_table_config );
        if( iRlt == PAS_EXIT_OK )
        {
			address_table_config.discard_llid_unlearned_sa = V2R1_discard_llid_unlearned_sa;
			address_table_config.discard_unknown_da        = V2R1_discard_unknown_da;
			PAS_set_address_table_configuration( olt_id, address_table_config );
        }
#endif
        /*uiSetVal = *val;*/
        if( 0 == uiSetVal ) 
        {
            max_entry = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
        }
        else
        {
            if( uiSetVal >= /*PON_ADDRESS_TABLE_SIZE*/PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION )	/* modified by xieshl 20121030, 问题单16118 */
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
        
        if ( 0 == (iRlt = GW10G_PAS_set_port_mac_limiting(olt_id, TM_PORT_PON_10G, llid, max_entry)) )
        {
            *val = uiSetVal;
        }
    }   
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*get onu pon uni mac config*/
static int GWONU_PAS8411_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_config);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10GAdp_get_onu_uni_port_mac_configuration(olt_id, llid, mac_config);
    }   
					
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuUniMacCfg(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
/*RTT*/
static int GWONU_PAS8411_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	GW10G_onu_registration_data_record_t onu_registration_data;
        
		iRlt = GW10G_PAS_get_onu_registration_data(olt_id, llid, &onu_registration_data);
		if ( iRlt == PAS_EXIT_OK )
        {
			*rtt = onu_registration_data.rtt * 16 / 10;
        }      
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuDistance(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GWONU_PAS8411_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        PAS_physical_capabilities_t device_capabilities;

		/*need to do*/
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOpticalCapability(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
#endif


#if 1
/* -------------------ONU 倒换API---------------- */

static int GWONU_PAS8411_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);

    if ( 0 <= (iRlt = GW10G_PAS_get_onu_mode(olt_id, llid)) )
    {
        switch (iRlt)
        {
            case PON_ONU_MODE_ON:
            case PON_ONU_MODE_PENDING:
                iRlt = 0;
                break;
            case PON_ONU_MODE_OFF:    
                iRlt = OLT_ERR_NOTEXIST;
                break;
            default:
                iRlt = OLT_ERR_NOTEXIST;
        }
    }
    
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 设备管理API------------------- */
/*get onu ver*/
static int GWONU_PAS8411_GetOnuVer(short int olt_id, short int onu_id, PON_onu_versions *onu_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_versions);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_PAS_get_onu_version ( olt_id, llid, onu_versions );
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

/*get onu pon chip ver*/
static int GWONU_PAS8411_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        /*need to do*/
    }   
	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuPonVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}


static int GWONU_PAS8411_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
	int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	GW10G_onu_registration_data_record_t onu_registration_data;
	PON_onu_versions onu_version_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = GW10G_PAS_get_onu_version ( olt_id, llid, &onu_version_info )) )
        {
            if ( 0 == (iRlt = GW10G_PAS_get_onu_registration_data ( olt_id, llid, &onu_registration_data )) )
            {
                onu_info->oam_version = onu_registration_data.oam_version;
                onu_info->rtt = onu_registration_data.rtt;
                
                onu_info->laser_on_time  = onu_registration_data.laser_on_time;
                onu_info->laser_off_time = onu_registration_data.laser_off_time;

                onu_info->vendorType     = OnuVendorTypesPmc;
                onu_info->productVersion = onu_version_info.hardware_version;
                onu_info->productCode    = onu_info->productVersion;
            
                onu_info->max_links_support = 1;

                switch ( onu_registration_data.onu_type )
                {
                    case GW10G_PON_SYMMETRIC_10G_ONU:
                        onu_info->pon_rate_flags = PON_RATE_10_10G;
                    break;    
                    case GW10G_PON_ASYMMETRIC_10G_ONU:
                        onu_info->pon_rate_flags = PON_RATE_1_10G;
                    break;    
                    case GW10G_PON_1G_ONU:
                        onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
                    break;    
                    default:
                        onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
                        VOS_ASSERT(0);
                }
            }
        }
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}


static int GWONU_PAS8411_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_REMOTE_PASONU_eeprom_mapper_get_parameter( olt_id, llid, info_id, data, size );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS8411_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_REMOTE_PASONU_eeprom_mapper_set_parameter( olt_id, llid, info_id, data, size );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS8411_ResetOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    
        iRlt = GW10G_REMOTE_PASONU_reset_device( olt_id, llid, PON_RESET_SW );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_RemoteReset(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int CTCONU_PAS8411_ResetOnu(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_reset_onu( olt_id, llid );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_RemoteReset(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS8411_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    
        iRlt = GW10G_REMOTE_PASONU_get_burn_image_complete( olt_id, llid, complete );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS8411_Get_Burn_Image_Complete(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */

static int CTCONU_PAS8411_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_onu_version(olt_id, llid, version);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetCtcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_firmware_version(olt_id, llid, version);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetFirmwareVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_onu_serial_number(olt_id, llid, serial_number);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetSerialNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_chipset_id(olt_id, llid, chipset_id);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetChipsetID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_onu_capabilities(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetOnuCap1(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_onu_capabilities_2(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetOnuCap2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_onu_capabilities_3(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetOnuCap3(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name )
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
        iRlt = GW10G_CTC_STACK_update_onu_firmware(olt_id, llid, &f_image, file_name);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_UpdateOnuFirmware(%d, %d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, file_len, file_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_ActiveOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_activate_image(olt_id, llid);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_ActiveOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_CommitOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_commit_image(olt_id, llid);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_CommitOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int *timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_extended_oam_discovery_timing(&tm);
        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *timeout = tm;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_set_extended_oam_discovery_timing(timeout);
        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEncryptionTiming(short int olt_id, short int onu_id, int *update_time, int *no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar update_key_t;
        ushort no_reply_t;

        iRlt = GW10G_CTC_STACK_get_encryption_timing(&update_key_t, &no_reply_t);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            *update_time = update_key_t;
            *no_reply = no_reply_t;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEncryptionTiming(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEncryptionTiming(short int olt_id, short int onu_id, int update_time, int no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar update_key_t = update_time;
        ushort no_reply_t = no_reply;

        iRlt = GW10G_CTC_STACK_set_encryption_timing(update_key_t, no_reply_t);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, update_time, no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_StartEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_start_encryption(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_StartEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_StopEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_stop_encryption(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_StopEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortLinkstate(short int olt_id, short int onu_id,  int port, int *ls )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_get_ethernet_link_state(olt_id, llid, lport, ls);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortLinkstate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEthPortAdminstatus(short int olt_id, short int onu_id,  int port, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = GW10G_CTC_STACK_get_phy_admin_state(olt_id, llid, lport, &en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *enable = en;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortAdminstatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortAdminstatus(short int olt_id, short int onu_id,  int port, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {

            uchar lport = port;
            bool en = enable;

            iRlt = GW10G_CTC_STACK_set_phy_admin_control(olt_id, llid, lport, en);

            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortAdminstatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortPauseEnable(short int olt_id, short int onu_id,  int port, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = GW10G_CTC_STACK_get_ethernet_port_pause(olt_id, llid, lport, &en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *enable = en;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortPauseEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortPauseEnable(short int olt_id, short int onu_id,  int port, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = enable;

        iRlt = GW10G_CTC_STACK_set_ethernet_port_pause(olt_id, llid, lport, en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortPauseEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id,  int port, int *an )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool lan = FALSE;

        iRlt = GW10G_CTC_STACK_get_auto_negotiation_admin_state(olt_id, llid, lport, &lan);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *an = lan;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortAutoNegotinationAdmin(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id,  int port, int an )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, an);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else if(an == TRUE)/*只有在激活时发restart*/
        {
            GW10G_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortAutoNegotinationAdmin(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortRestartAutoConfig(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortRestartAutoConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEthPortAnLocalTecAbility(short int  olt_id, short int onu_id, int port, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_auto_negotiation_local_technology_ability(olt_id, llid, port, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortAnLocalTecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEthPortAnAdvertisedTecAbility(short int  olt_id, short int onu_id, int port, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_auto_negotiation_advertised_technology_ability(olt_id, llid, port, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortAnAdvertisedTecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_policing_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = GW10G_CTC_STACK_get_ethernet_port_policing(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_ethernet_port_policing(olt_id, llid, lport, *policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = GW10G_CTC_STACK_get_ethernet_port_ds_rate_limiting(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port, CTC_STACK_port_vlan_configuration_t * vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, vconf);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortVlanConfig(short int olt_id, short int onu_id,  int port, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, *vconf);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
static int CTCONU_PAS8411_GetAllPortVlanConfig(short int olt_id, short int onu_id,  unsigned char* portNum, CTC_STACK_vlan_configuration_ports_t   ports_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, portNum, ports_info);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetAllPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, portNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port configuration*/
static int CTCONU_PAS8411_SetPortQinqConfig(short int olt_id, short int onu_id, int port, CTC_STACK_port_qinq_configuration_t   port_configuration )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {

		uchar lport = port;

		iRlt = GW10G_CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, port_configuration);

        	if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

     }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortQinqConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS8411_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_get_classification_and_marking(olt_id, llid, lport, cam);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port,
        CTC_STACK_classification_rule_mode_t  mode, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_classification_and_marking(olt_id, llid, lport, mode, cam);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_get_multicast_vlan(olt_id, llid, lport, mv);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_multicast_vlan(olt_id, llid, lport, *mv);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_ClearEthPortMulticastVlan(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_clear_multicast_vlan(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_ClearEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port, int *num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar  lport = port, gnum=0;

        iRlt = GW10G_CTC_STACK_get_multicast_group_num(olt_id, llid, lport, &gnum);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            *num = gnum;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port, int num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port, gnum=num;

        iRlt = GW10G_CTC_STACK_set_multicast_group_num(olt_id, llid, lport, gnum);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port, int *strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool fstrip = FALSE;

        iRlt = GW10G_CTC_STACK_get_multicast_tag_strip(olt_id, llid, lport, &fstrip);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            *strip = fstrip;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port, int strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool fstrip = strip;

        iRlt = GW10G_CTC_STACK_set_multicast_tag_strip(olt_id, llid, lport, fstrip);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetMulticastAllPortTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_get_multicast_all_port_tag_strip(olt_id, llid, number_of_entries, ports_info);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetMulticastAllPortTagStrip(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
    
}

static int CTCONU_PAS8411_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool fstrip = FALSE;

        iRlt = GW10G_CTC_STACK_get_multicast_tag_oper(olt_id, llid, lport, oper, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetEthPortMulticastTagOper( short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = GW10G_CTC_STACK_set_multicast_tag_oper(olt_id, llid, lport, oper, *sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object,
		   CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_set_multicast_management_object_tag_oper(olt_id, llid, management_object->index, tag_oper, *sw);
	}
	
	return iRlt;
}


static int CTCONU_PAS8411_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_multicast_control(olt_id, llid, mc);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_set_multicast_control(olt_id, llid, *mc);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_ClearMulticastControl(short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_clear_multicast_control(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_ClearMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t *sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_multicast_switch(olt_id, llid, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_set_multicast_switch(olt_id, llid, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetFastLeaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_fast_leave_ability(olt_id, llid, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetFastLeaveAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetFastLeaveAdminState(short int olt_id, short int onu_id, int *fla)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_get_fast_leave_admin_state(olt_id, llid, fla);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetFastLeaveAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetFastLeaveAdminState(short int olt_id, short int onu_id, int fla)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = GW10G_CTC_STACK_set_fast_leave_admin_control(olt_id, llid, fla);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetFastLeaveAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetPortStatisticData(short int olt_id, short int onu_id, int port,CTC_STACK_statistic_data_t *data)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port;

        if( OnuSupportStatistics(olt_id, onu_id) == VOS_OK )	/* 问题单16117 */
            iRlt = GW10GAdp_CTC_STACK_get_ethport_statistic_data(olt_id, llid, lport, (void*)data);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetPortStatisticData(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

static int CTCONU_PAS8411_GetPortStatisticState(short int olt_id, short int onu_id, int port, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port;

        iRlt = GW10GAdp_CTC_STACK_get_ethport_statistic_state(olt_id, llid, lport, (void*)state);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

static int CTCONU_PAS8411_SetPortStatisticState(short int olt_id, short int onu_id, int port,CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port;

        iRlt = GW10GAdp_CTC_STACK_set_ethport_statistic_state(olt_id, llid, lport, state->status);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}




/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_PAS8411_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object, CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_set_alarm_admin_state(olt_id, llid, management_object->index, alarm_id, enable);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetAlarmAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS8411_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_set_alarm_threshold(olt_id, llid, management_object->index, alarm_id, alarm_threshold, clear_threshold);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetAlarmThreshold(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

static int CTCONU_PAS8411_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_get_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS8411_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_set_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}


static int CTCONU_PAS8411_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = GW10G_CTC_STACK_get_mxu_mng_global_parameter_config(olt_id, llid, mxu_mng);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = GW10G_CTC_STACK_set_mxu_mng_global_parameter_config(olt_id, llid, *mxu_mng);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_GetMxuMngSnmpConfig ( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_get_mxu_mng_snmp_parameter_config(olt_id, llid, parameter);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS8411_SetMxuMngSnmpConfig(short int olt_id, short int onu_id,CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_set_mxu_mng_snmp_parameter_config(olt_id, llid, *parameter);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int CTCONU_PAS8411_GetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_holdover_state(olt_id, llid, holdover);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_holdover_state(olt_id, llid, *holdover);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_get_optical_transceiver_diagnosis(olt_id, llid, optical_transceiver_diagnosis);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetOptTransDiag(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}

static int CTCONU_PAS8411_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = GW10G_CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, FALSE);
    }
    else
        iRlt = GW10G_CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, TRUE);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t  *fec_ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = GW10G_CTC_STACK_get_fec_ability(olt_id, llid, fec_ability);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_GetFecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */


#if 1
int CTCONU_PAS8411_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t* iad_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_voip_iad_info(olt_id, llid, iad_info);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetIADInfo:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

/*H.248协议下IAD的运行状态*/
int CTCONU_PAS8411_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_voip_iad_oper_status(olt_id, llid, iad_oper_status);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetVoipIadOperStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

int CTCONU_PAS8411_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_voip_iad_operation(olt_id, llid, operation_type);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetVoipIadOperation:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

/*语音模块全局参数配置*/
int CTCONU_PAS8411_GetGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_voip_global_param_conf(olt_id, llid, global_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS8411_SetGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_voip_global_param_conf_t global_config;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        VOS_MemZero(&global_config, sizeof(CTC_STACK_voip_global_param_conf_t));
        iRlt = GW10G_CTC_STACK_get_voip_global_param_conf(olt_id, llid, &global_config);
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

            iRlt = GW10G_CTC_STACK_set_voip_global_param_conf(olt_id, llid, global_config);
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

    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

static int testGlobalParameterConfig(short int slot, short int port, short int onuid, ULONG vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_STACK_voip_global_param_conf_t ulglobal_param;
    short int olt_id = GetPonPortIdxBySlot(slot, port);
    short int onu_id = onuid-1;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        VOS_MemZero(&ulglobal_param, sizeof(CTC_STACK_voip_global_param_conf_t));
        iRlt = GW10G_CTC_STACK_get_voip_global_param_conf(olt_id, llid, &ulglobal_param);
        ulglobal_param.cvlan_id = vid;
        iRlt = GW10G_CTC_STACK_set_voip_global_param_conf(olt_id, llid, ulglobal_param);
        CTC_VOIP_DEBUG("\r\n%d = CTCONU_Set_GlobalParameterConfig:olt_id = %d, onu_id = %d, llid = %d, vid = %d\r\n", iRlt, olt_id, onu_id, llid, vid);    
    }
    return VOS_OK;
}

int CTCONU_PAS8411_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_voip_fax_config(olt_id, llid, voip_fax);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_STACK_voip_fax_config_t ulvoip_fax;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulvoip_fax, sizeof(CTC_STACK_voip_fax_config_t));
        iRlt = GW10G_CTC_STACK_get_voip_fax_config(olt_id, llid, &ulvoip_fax);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
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
            iRlt = GW10G_CTC_STACK_set_voip_fax_config(olt_id, llid, ulvoip_fax);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    
    return iRlt;           
}


int CTCONU_PAS8411_GetVoipPortStatus(short int olt_id, short int onu_id, int port, CTC_STACK_voip_pots_status_array *pots_status_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_management_object_index_t management_object_index;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
#if 1
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        if(port)
            management_object_index.port_number = port;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        pots_status_array->number_of_entries = 24;
        iRlt = GW10G_CTC_STACK_get_voip_pots_status(olt_id, llid, management_object_index, &(pots_status_array->number_of_entries), &(pots_status_array->pots_status_array));
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
#endif    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetVoipPortStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

int CTCONU_PAS8411_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_voip_port(olt_id, llid, port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;   
}

int CTCONU_PAS8411_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_voip_port(olt_id, llid, port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
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
        
        iRlt = GW10G_CTC_STACK_get_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;   
    
}

int CTCONU_PAS8411_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
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
        iRlt = GW10G_CTC_STACK_set_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;           
    
}

int CTCONU_PAS8411_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_h248_param_config(olt_id, llid, h248_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS8411_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_h248_param_config_t h248_config;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&h248_config, sizeof(CTC_STACK_h248_param_config_t));
        iRlt = GW10G_CTC_STACK_get_h248_param_config(olt_id, llid, &h248_config);
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
            iRlt = GW10G_CTC_STACK_set_h248_param_config(olt_id, llid, h248_config);
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

    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS8411_GetH248UserTidConfig(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_management_object_index_t management_object_index;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
#if 1
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63; 
        if(port)
            management_object_index.port_number = port;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        h248_user_tid_array->number_of_entries = 24;
        iRlt = GW10G_CTC_STACK_get_h248_user_tid_config(olt_id, llid, management_object_index, &(h248_user_tid_array->number_of_entries), &(h248_user_tid_array->h248_user_tid_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
#endif    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetH248UserTidConfig(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_t *user_tid_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_management_object_index_t  management_object_index;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = 63;        
        if(port)
            management_object_index.port_number = port;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        iRlt = GW10G_CTC_STACK_set_h248_user_tid_config (olt_id, llid, management_object_index, *user_tid_config);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_h248_rtp_tid_info(olt_id, llid, h248_rtp_tid_info);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)

{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_h248_rtp_tid_config(olt_id, llid, *h248_rtp_tid_info);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int CTCONU_PAS8411_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_sip_param_config(olt_id, llid, sip_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_STACK_sip_param_config_t ulsip_param;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulsip_param, sizeof(CTC_STACK_sip_param_config_t));
        iRlt = GW10G_CTC_STACK_get_sip_param_config(olt_id, llid, &ulsip_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
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
            iRlt = GW10G_CTC_STACK_set_sip_param_config(olt_id, llid, ulsip_param);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_sip_digit_map(olt_id, llid, *sip_digit_map);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetSipDigitMap:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int sip_slot_8411 = 63;
int sip_no_8411 = 24;

int CTCONU_PAS8411_GetSipUserConfig(short int olt_id, short int onu_id, int port, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_management_object_index_t management_object_index;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = sip_slot_8411;        
        if(port)
            management_object_index.port_number = port;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        sip_user_param_array->number_of_entries = sip_no_8411;
        iRlt = GW10G_CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &(sip_user_param_array->number_of_entries), &(sip_user_param_array->sip_user_param_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_GetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS8411_SetSipUserConfig(short int olt_id, short int onu_id, int port, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_management_object_index_t management_object_index;
    unsigned char number_of_entries = 24;
    CTC_STACK_sip_user_param_config_array_t sip_user_param_array;    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    

        VOS_MemZero(&management_object_index, sizeof(CTC_management_object_index_t));
        management_object_index.frame_number = 0;
        management_object_index.slot_number = sip_slot_8411;        
        management_object_index.port_number = port;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;       
        iRlt = GW10G_CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &number_of_entries, &sip_user_param_array);                
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

        iRlt = CTC_STACK_set_sip_user_param_config(olt_id, llid, management_object_index, *sip_user_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS8411_SetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}
#endif

/*onu auth*/
#if 0
static int CTCONU_Set_auth_loid_data_add(short int olt_id, short int onu_id,CTC_STACK_auth_loid_data_t   loid_data )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
        
		iRlt = CTC_STACK_add_auth_loid_data(olt_id, llid);

        	if (iRlt != 0)
                {
                iRlt = OLT_ERR_NOTSUPPORT;
                }

        }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_Set_commit_image(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


PON_STATUS CTC_STACK_add_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);

PON_STATUS CTC_STACK_remove_auth_loid_data(           
					const PON_olt_id_t			         olt_id, 
					const CTC_STACK_auth_loid_data_t     loid_data);


#endif

static int CTCONU_Set_auth_mode(short int olt_id, short int onu_id, CTC_STACK_auth_mode_t   ctc_auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
		iRlt = GW10G_CTC_STACK_set_auth_mode(olt_id, ctc_auth_mode);

    	if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTC_STACK_set_auth_mode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


/*onu alarm*/
static int CTCONU_Set_onualarm_equipment(short int olt_id, short int onu_id, USHORT flag)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_management_object_index_t management_object;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	VOS_MemZero(&management_object, sizeof(management_object));

       management_object.frame_number = 0;
	management_object.port_number = 0xFFFFFFFF;
	management_object.slot_number = 0;
	management_object.port_type =CTC_MANAGEMENT_OBJECT_PORT_TYPE_ETHERNET_PORT;

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
             iRlt=GW10G_CTC_STACK_set_alarm_admin_state(olt_id, onu_id,management_object,EQUIPMENT_ALARM,1 );

        	if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

        }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_Set_onualarm_equipment(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

static int CTCONU_PAS8411_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i;
    short int llid;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
        {
            for ( i=0; i<portNum; i++ )
            {
                Port_Info[i].entry.mode = enable ? ONU_CONF_VLAN_MODE_TRUNK : ONU_CONF_VLAN_MODE_TRANSPARENT;

                /* added by wangxiaoyu 2011-09-21 */
                /* vlan使能时， 将所有端口默认加入vlan 1 */
                /* pvc.default_vlan &= ~0xfff; */
                /* pvc.default_vlan |= 1; */
                Port_Info[i].entry.default_vlan = (0x8100<<16)|1;
                Port_Info[i].entry.number_of_entries = 0;

                iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, Port_Info[i].port_number, Port_Info[i].entry);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetVlanEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    mode &= (~ONU_SEPCAL_FUNCTION);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc);
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

                iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j, num;
    ULONG all, untag;
    short int llid;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemSet(Port_Info, 0, sizeof(CTC_STACK_vlan_configuration_ports_t));
                
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = get_onuconf_vlanPortlist(olt_id, onu_id, vid,  &all, &untag)) )
        {
            if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
            {
                for ( i=0; i<portNum; i++ )
                {
                    num = Port_Info[i].entry.number_of_entries;
                    
                    for ( j=0; j<num; j++ )
                    {
                        if ( (Port_Info[i].entry.vlan_list[j] & 0xfff) == vid )
                        {
                            linux_memmove(Port_Info[i].entry.vlan_list+j, Port_Info[i].entry.vlan_list+j+1, sizeof(long)*(num-1-j));
                            Port_Info[i].entry.number_of_entries--;
                        }
                    }
                    
                    /*端口默认vlan恢复为 1 */
                    if ( untag & (1<<(Port_Info[i].port_number-1)) )
                    {
                        /* pvc.default_vlan &= 0xfffff000;*/
                        /* pvc.default_vlan |= 1; */
                        Port_Info[i].entry.default_vlan = (0x8100<<16)|1;
                    }
                    
                    iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, Port_Info[i].port_number, Port_Info[i].entry);
                }
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_DelVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetPortPvid(short int olt_id, short int onu_id, int port_id, int lv)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc);
        if ( 0 == iRlt )
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

            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortPvid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j, num;
    short int llid;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
        {
            for ( i=0; i<portNum; i++ )
            {
                if(portlist&(1<<(Port_Info[i].port_number-1)))
                {
                    num = Port_Info[i].entry.number_of_entries;

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

                    iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, Port_Info[i].port_number, Port_Info[i].entry);
                }
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_AddVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j, num;
    short int llid;
    unsigned char portNum = 0;
    CTC_STACK_vlan_configuration_ports_t Port_Info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
        {
            for ( i=0; i<portNum; i++ )
            {
                if(portlist&(1<<(Port_Info[i].port_number-1)))
                {
                    num = Port_Info[i].entry.number_of_entries;
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
                    
                    iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, Port_Info[i].port_number, Port_Info[i].entry);
                }
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_DelVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

/*vlan transation*/
static int CTCONU_PAS8411_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, num;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_DelVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*vlan aggregation*/
static int CTCONU_PAS8411_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int CTCONU_PAS8411_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, j;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 0
        if ( 0 == (iRlt = GW10G_CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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
            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_DelVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*QinQ enable*/
static int CTCONU_PAS8411_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    ULONG vid = 1;
    short int llid;
    uchar lport = port_id;
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
            
            iRlt = GW10G_CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config );
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortQinQEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return  iRlt;
}

/*qinq port vlan tag add*/
static int CTCONU_PAS8411_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    uchar lport = port_id;
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

                iRlt = GW10G_CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config);
            }
            else
                iRlt = OLT_ERR_NORESC;
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_AddQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port vlan tag del*/
static int CTCONU_PAS8411_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG  cvlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i, n;
    short int llid;
    uchar lport = port_id;
    CTC_STACK_port_qinq_configuration_t  qinq_config;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
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

                iRlt = GW10G_CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_DelQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return  iRlt;
}


static int CTCONU_PAS8411_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    uchar lport = port_id;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        bool state = mode ? FALSE:TRUE;
        
        if ( 0 == (iRlt = GW10G_CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, state)) )
        {
            if ( state )
            {
                GW10G_CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_ethernet_port_pause(olt_id, llid, (unsigned char)port_id, (bool)en);    
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortFcMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
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

        iRlt = GW10G_CTC_STACK_set_ethernet_port_policing(olt_id, llid, lport, ppe);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortIngressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        CTC_STACK_ethernet_port_ds_rate_limiting_entry_t ppe;

        ppe.CIR = rate;
        ppe.PIR = rate;
        ppe.port_number = port_id;
        ppe.state = rate ? TRUE : FALSE;

        iRlt = GW10G_CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, lport, &ppe);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortEgressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8411_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;

		if ( 0 == qossetid ) /* 删除pon端口上的qos配置*/
		{
			iRlt = GW10G_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);
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

                GW10G_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);

                iRlt = GW10G_CTC_STACK_set_classification_and_marking(olt_id, llid, lport, CTC_CLASSIFICATION_ADD_RULE, qset);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;

        iRlt = GW10G_CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_ClrPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



static int CTCONU_PAS8411_SetPortDefaultPriority(short int olt_id, short int onu_id, int port_id, int priority)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        CTC_STACK_port_vlan_configuration_t pvc;
         
        if ( 0 == (iRlt = get_OnuConf_Ctc_portVlanConfig(olt_id, onu_id, port_id, &pvc)) )
        {
            pvc.default_vlan &= 0x1fff;
            pvc.default_vlan |= priority << 13;
             
            iRlt = GW10G_CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortDefaultPriority(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS8401_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_multicast_switch(olt_id, llid, en ? CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING : CTC_STACK_PROTOCOL_CTC);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8401_SetIgmpEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = GW10G_CTC_STACK_set_multicast_switch(olt_id, llid, en);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetIgmpAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS8411_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        CTC_STACK_multicast_vlan_t mv;
        int num = 0, vids[ONU_MAX_IGMP_VLAN];

        if ( 0 == vid )
        {
            iRlt = GW10G_CTC_STACK_clear_multicast_vlan(olt_id, onu_id, lport);
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
                    
                   GW10G_CTC_STACK_clear_multicast_vlan(olt_id, onu_id, lport);

                   mv.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
                   iRlt = GW10G_CTC_STACK_set_multicast_vlan(olt_id, onu_id, lport, mv);
                }
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS8411_SetPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

/* --------------------END------------------------ */


/******************************************外部接口***************************************/

static const OnuMgmtIFs s_gwonu8411Ifs = 
{
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS8411_OnuIsOnline,
    GWONU_PAS8411_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/
    GWONU_PAS8411_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_PAS8411_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_PAS8411_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS8411_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    GWONU_PAS8411_SetOnuTrafficServiceMode,
    GWONU_PAS8411_SetOnuPeerToPeer,
    GWONU_PAS8411_SetOnuPeerToPeerForward,
    GWONU_PAS8411_SetOnuBW,
    GWONU_PAS8411_GetOnuSLA,

    GWONU_PAS8411_SetOnuFecMode,
    GWONU_PAS8411_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_SetUniPort,
    GWONU_PAS8411_SetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_PAS8411_ResetCounters,
    GWONU_PAS8411_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_PAS8411_GetLLIDParams,
    GWONU_PAS8411_StartEncryption,
    GWONU_PAS8411_StopEncryption,
    GWONU_PAS8411_SetOnuEncryptParams,
    GWONU_PAS8411_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_PAS8411_GetOnuMacAddrTbl,
    GWONU_PAS8411_GetOltMacAddrTbl,
    GWONU_PAS8411_SetOnuMaxMacNum,
    GWONU_PAS8411_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_PAS8411_GetOnuDistance,
    GWONU_PAS8411_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_PAS8411_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    GWONU_PAS8411_GetOnuVer,
    GWONU_PAS8411_GetOnuPonVer,
    GWONU_PAS8411_GetOnuRegisterInfo,
    GWONU_PAS8411_GetOnuI2CInfo,
    GWONU_PAS8411_SetOnuI2CInfo,
    
    GWONU_PAS8411_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_GetBurnImageComplete,
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
    GWONU_SetPasFlush,
    
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
    REMOTE_OK,/*GWONU_SetVlanTran,*/
    REMOTE_OK,/*GWONU_DelVlanTran,*/
    REMOTE_OK,/*GWONU_SetVlanAgg,*/
    REMOTE_OK,/*GWONU_DelVlanAgg,*/

    REMOTE_OK,/*GWONU_SetPortQinQEnable,*/
    REMOTE_OK,/*GWONU_AddQinQVlanTag,*/
    REMOTE_OK,/*GWONU_DelQinQVlanTag,*/

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
    
    REMOTE_OK/*GWONU_SetQosClass*/,
    REMOTE_OK/*GWONU_ClrQosClass*/,
    REMOTE_OK/*GWONU_SetQosRule*/,
    REMOTE_OK/*GWONU_ClrQosRule*/,
    
    REMOTE_OK,/*GWONU_SetPortQosRule,*/
    REMOTE_OK,/*GWONU_ClrPortQosRule,*/
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
    REMOTE_OK,/*GWONU_SetPortMulticastVlan,*/

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    NULL,           /* RegisterCMC */
    NULL,           /* UnregisterCMC */
    NULL,           /* DumpCmc */
    NULL,           /* DumpCmcAlarms */
    NULL,           /* DumpCmcLogs */
    
    NULL,           /* ResetCmcBoard */
    NULL,           /* GetCmcVersion */
    NULL,           /* GetCmcMaxMulticasts */
    NULL,           /* GetCmcMaxCm */
    NULL,           /* SetCmcMaxCm */
    
    NULL,           /* GetCmcTime */
    NULL,           /* SetCmcTime */
    NULL,           /* LocalCmcTime */
    NULL,           /* SetCmcCustomConfig */
    NULL,           /* DumpCmcCustomConfig */

    NULL,           /* DumpCmcDownChannel */
    NULL,           /* DumpCmcUpChannel */
    NULL,           /* SetCmcDownChannelMode */
    NULL,           /* SetCmcUpChannelMode */
    NULL,           /* SetCmcUpChannelD30Mode */
    
    NULL,           /* SetCmcDownChannelFreq */
    NULL,           /* SetCmcUpChannelFreq */
    NULL,           /* SetCmcDownAutoFreq */
    NULL,           /* SetCmcUpAutoFreq */
    NULL,           /* SetCmcUpChannelWidth */
    
    NULL,           /* SetCmcDownChannelAnnexMode */
    NULL,           /* SetCmcUpChannelType */
    NULL,           /* SetCmcDownChannelModulation */
    NULL,           /* SetCmcUpChannelProfile */
    NULL,           /* SetCmcDownChannelInterleaver */

    NULL,           /* SetCmcDownChannelPower */
    NULL,           /* SetCmcUpChannelPower */
    NULL,           /* DumpCmcUpChannelPower */
    NULL,           /* DumpCmcUpChannelSignalQuality */
    NULL,           /* DumpCmcInterfaceUtilization */
    
    NULL,           /* DumpCmcInterfaceStatistics */
    NULL,           /* DumpCmcMacStatistics */
    NULL,           /* DumpCmcAllInterface */
    NULL,           /* DumpCmcAllLoadBalancingGrp */
    NULL,           /* DumpCmcLoadBalancingGrp */
    
    NULL,           /* DumpCmcLoadBalancingGrpDownstream */
    NULL,           /* DumpCmcLoadBalancingGrpUpstream */
    NULL,           /* DumpCmcLoadBalancingDynConfig */
    NULL,           /* SetCmcLoadBalancingDynMethod */
    NULL,           /* SetCmcLoadBalancingDynDynPeriod */
    
    NULL,           /* SetCmcLoadBalancingDynWeightedAveragePeriod */
    NULL,           /* SetCmcLoadBalancingDynOverloadThresold */
    NULL,           /* SetCmcLoadBalancingDynDifferenceThresold */
    NULL,           /* SetCmcLoadBalancingDynMaxMoveNumber */
    NULL,           /* SetCmcLoadBalancingDynMinHoldTime */
    
    NULL,           /* SetCmcLoadBalancingDynRangeOverrideMode */
    NULL,           /* SetCmcLoadBalancingDynAtdmaDccInitTech */
    NULL,           /* SetCmcLoadBalancingDynScdmaDccInitTech */
    NULL,           /* SetCmcLoadBalancingDynAtdmaDbcInitTech */
    NULL,           /* SetCmcLoadBalancingDynScdmaDbcInitTech */

    NULL,           /* CreateCmcLoadBalancingGrp */
    NULL,           /* DestroyCmcLoadBalancingGrp */
    NULL,           /* AddCmcLoadBalancingGrpDownstream */
    NULL,           /* RemoveCmcLoadBalancingGrpDownstream */
    NULL,           /* AddCmcLoadBalancingGrpUpstream */
    
    NULL,           /* RemoveCmcLoadBalancingGrpUpstream */
    NULL,           /* AddCmcLoadBalancingGrpModem */
    NULL,           /* RemoveCmcLoadBalancingGrpModem */
    NULL,           /* AddCmcLoadBalancingGrpExcludeModem */
    NULL,           /* RemoveCmcLoadBalancingGrpExcludeModem */

    NULL,           /* DumpCmcLoadBalancingGrpModem */
    NULL,           /* DumpCmcLoadBalancingGrpActivedModem */
    NULL,           /* DumpCmcLoadBalancingGrpExcludeModem */
    NULL,           /* DumpCmcLoadBalancingGrpExcludeActivedModem */
    NULL,           /* ReserveCmcLoadBalancingGrp */

    NULL,           /* DumpAllCm */
    NULL,           /* DumpCm */
    NULL,           /* DumpAllCmHistory */
    NULL,           /* DumpCmHistory */
    NULL,           /* ClearAllCmHistory */

    NULL,           /* ResetCM */
    NULL,           /* DumpCmDownstream */
    NULL,           /* DumpCmUpstream */
    NULL,           /* SetCmDownstream */
    NULL,           /* SetCmUpstream */

    NULL,           /* CreateCmServiceFlow */
    NULL,           /* ModifyCmServiceFlow */
    NULL,           /* DestroyCmServiceFlow */
    NULL,           /* DumpCmClassifier */
    NULL,           /* DumpCmServiceFlow */

    NULL,           /* DumpCmcClassifier */
    NULL,           /* DumpCmcServiceFlow */
    NULL,           /* DumpCmcServiceFlowStatistics */
    NULL,           /* DumpCmcDownChannelBondingGroup */
    NULL,           /* DumpCmcUpChannelBondingGroup */   
    
    NULL,           /* CreateCmcServiceFlowClassName */
    NULL,           /* DestroyCmcServiceFlowClassName */   
    NULL,           /* GetCmcMacAddrTbl */   
    NULL,           /* GetCmMacAddrTbl */   
    NULL,           /* ResetCmAddrTbl */   
#endif
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL,
   NULL,
   NULL,
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
    /* --------------------END------------------------ */

    REMOTE_ERROR
};

static const OnuMgmtIFs s_ctconu8411Ifs = 
{
#if 1
/* -------------------ONU基本API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS8411_OnuIsOnline,
    GWONU_PAS8411_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/
    GWONU_PAS8411_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_PAS8411_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GWONU_PAS8411_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS8411_AuthorizeOnu,
    CTCONU_PAS8411_AuthRequest,
    CTCONU_PAS8411_AuthSuccess,
    CTCONU_PAS8411_AuthFailure,
	/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    CTCONU_PAS8411_SetOnuTrafficServiceMode,
    GWONU_PAS8411_SetOnuPeerToPeer,
    GWONU_PAS8411_SetOnuPeerToPeerForward,
    GWONU_PAS8411_SetOnuBW,
    GWONU_PAS8411_GetOnuSLA,

    CTCONU_PAS8411_SetOnuFecMode,
    GWONU_PAS8411_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_SetUniPort,
    GWONU_PAS8411_SetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    GWONU_PAS8411_ResetCounters,
    GWONU_PAS8411_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    GWONU_PAS8411_GetLLIDParams,
    GWONU_PAS8411_StartEncryption,
    GWONU_PAS8411_StopEncryption,
    GWONU_PAS8411_SetOnuEncryptParams,
    GWONU_PAS8411_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    GWONU_PAS8411_GetOnuMacAddrTbl,
    GWONU_PAS8411_GetOltMacAddrTbl,
    REMOTE_OK,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS8411_SetOnuMaxMacNum,
    GWONU_PAS8411_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GWONU_PAS8411_GetOnuDistance,
    GWONU_PAS8411_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    GWONU_PAS8411_SetOnuLLID,
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    GWONU_PAS8411_GetOnuVer,
    GWONU_PAS8411_GetOnuPonVer,
    GWONU_PAS8411_GetOnuRegisterInfo,
    GWONU_PAS8411_GetOnuI2CInfo,
    GWONU_PAS8411_SetOnuI2CInfo,
    
    CTCONU_PAS8411_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS8411_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    /*CTCONU_PAS5201_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_PAS8411_GetCtcVersion,
    CTCONU_PAS8411_GetFirmwareVersion,
    CTCONU_PAS8411_GetSerialNumber,
    CTCONU_PAS8411_GetChipsetID,

    CTCONU_PAS8411_GetOnuCap1,
    CTCONU_PAS8411_GetOnuCap2,
    CTCONU_PAS8411_GetOnuCap3,
    
    CTCONU_PAS8411_UpdateOnuFirmware,
    CTCONU_PAS8411_ActiveOnuFirmware,
    CTCONU_PAS8411_CommitOnuFirmware,
    
    CTCONU_PAS8411_StartEncrypt,
    CTCONU_PAS8411_StopEncrypt,
    
    CTCONU_PAS8411_GetEthPortLinkstate,
    CTCONU_PAS8411_GetEthPortAdminstatus,
    CTCONU_PAS8411_SetEthPortAdminstatus,
    
    CTCONU_PAS8411_GetEthPortPauseEnable,
    CTCONU_PAS8411_SetEthPortPauseEnable,
    
    CTCONU_PAS8411_GetEthPortAutoNegotinationAdmin,
    CTCONU_PAS8411_SetEthPortAutoNegotinationAdmin,
    CTCONU_PAS8411_SetEthPortRestartAutoConfig,
    CTCONU_PAS8411_GetEthPortAnLocalTecAbility,
    CTCONU_PAS8411_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_PAS8411_GetEthPortPolicing,
    CTCONU_PAS8411_SetEthPortPolicing,
    
    CTCONU_PAS8411_GetEthPortDownstreamPolicing,
    CTCONU_PAS8411_SetEthPortDownstreamPolicing,

    CTCONU_PAS8411_GetEthPortVlanConfig,
    CTCONU_PAS8411_SetEthPortVlanConfig,
    CTCONU_PAS8411_GetAllPortVlanConfig,
    
    CTCONU_PAS8411_GetEthPortClassificationAndMarking,
    CTCONU_PAS8411_SetEthPortClassificationAndMarking,
    CTCONU_PAS8411_ClearEthPortClassificationAndMarking,
    
    CTCONU_PAS8411_GetEthPortMulticastVlan,
    CTCONU_PAS8411_SetEthPortMulticastVlan,
    CTCONU_PAS8411_ClearEthPortMulticastVlan,

    CTCONU_PAS8411_GetEthPortMulticastGroupMaxNumber,
    CTCONU_PAS8411_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_PAS8411_GetEthPortMulticastTagStrip,
    CTCONU_PAS8411_SetEthPortMulticastTagStrip,
    CTCONU_PAS8411_GetMulticastAllPortTagStrip,
    
    CTCONU_PAS8411_GetEthPortMulticastTagOper,
    CTCONU_PAS8411_SetEthPortMulticastTagOper,
    CTCONU_PAS8411_SetObjMulticastTagOper,
    
    CTCONU_PAS8411_GetMulticastControl,
    CTCONU_PAS8411_SetMulticastControl,
    CTCONU_PAS8411_ClearMulticastControl,
    
    CTCONU_PAS8411_GetMulticastSwitch,
    CTCONU_PAS8411_SetMulticastSwitch,
    
    CTCONU_PAS8411_GetFastLeaveAbility,
    CTCONU_PAS8411_GetFastLeaveAdminState,
    CTCONU_PAS8411_SetFastLeaveAdminState,

    CTCONU_PAS8411_GetPortStatisticData,
    CTCONU_PAS8411_GetPortStatisticState,
    CTCONU_PAS8411_SetPortStatisticState,

    CTCONU_PAS8411_GetIADInfo,
    CTCONU_PAS8411_GetVoipIadOperStatus,
    CTCONU_PAS8411_SetVoipIadOperation,
    CTCONU_PAS8411_GetGlobalConfig,
    CTCONU_PAS8411_SetGlobalConfig,
    CTCONU_PAS8411_GetVoipFaxConfig,
    CTCONU_PAS8411_SetVoipFaxConfig,
    
    CTCONU_PAS8411_SetAlarmAdminState,
    CTCONU_PAS8411_SetAlarmThreshold,
    CTCONU_PAS8411_GetDbaReportThresholds,
    CTCONU_PAS8411_SetDbaReportThresholds,
    
    CTCONU_PAS8411_GetMxuMngGlobalConfig,
    CTCONU_PAS8411_SetMxuMngGlobalConfig,
    CTCONU_PAS8411_GetMxuMngSnmpConfig,
    CTCONU_PAS8411_SetMxuMngSnmpConfig,

    CTCONU_PAS8411_GetHoldover,
    CTCONU_PAS8411_SetHoldover,
    CTCONU_PAS8411_GetOptTransDiag,
    CTCONU_PAS8411_SetTxPowerSupplyControl,

    CTCONU_PAS8411_GetFecAbility,

    CTCONU_PAS8411_GetVoipPortStatus,
    CTCONU_PAS8411_GetVoipPort,
    CTCONU_PAS8411_SetVoipPort,
    CTCONU_PAS8411_GetVoipPort2,
    CTCONU_PAS8411_SetVoipPort2,

    CTCONU_PAS8411_GetH248Config,
    CTCONU_PAS8411_SetH248Config,
    CTCONU_PAS8411_GetH248UserTidConfig,
    CTCONU_PAS8411_SetH248UserTidConfig,
    CTCONU_PAS8411_GetH248RtpTidConfig,
    CTCONU_PAS8411_SetH248RtpTidConfig,
    
    CTCONU_PAS8411_GetSipConfig,
    CTCONU_PAS8411_SetSipConfig,
    CTCONU_PAS8411_SetSipDigitMap,
    CTCONU_PAS8411_GetSipUserConfig,
    CTCONU_PAS8411_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID, 
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL,           /* CliCall */

    CTCONU_PAS8411_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    GWONU_SetPasFlush,
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_PAS8411_SetVlanEnable,
    CTCONU_PAS8411_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_PAS8411_DelVlan,
    CTCONU_PAS8411_SetPortPvid,

    CTCONU_PAS8411_AddVlanPort,
    CTCONU_PAS8411_DelVlanPort,
    CTCONU_PAS8411_SetVlanTran,
    CTCONU_PAS8411_DelVlanTran,
    CTCONU_PAS8411_SetVlanAgg,
    CTCONU_PAS8411_DelVlanAgg,
    
    CTCONU_PAS8411_SetPortQinQEnable,
    CTCONU_PAS8411_AddQinQVlanTag,
    CTCONU_PAS8411_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_PAS8411_SetPortMode,
    CTCONU_PAS8411_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_PAS8411_SetPortIngressRate,
    CTCONU_PAS8411_SetPortEgressRate,
    
    REMOTE_OK/*GWONU_SetQosClass*/,
    REMOTE_OK/*GWONU_ClrQosClass*/,
    REMOTE_OK/*GWONU_SetQosRule*/,
    REMOTE_OK/*GWONU_ClrQosRule*/,
    
    CTCONU_PAS8411_SetPortQosRule,
    CTCONU_PAS8411_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_PAS8411_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_PAS8401_SetIgmpEnable,
    CTCONU_PAS8411_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_PAS8411_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    NULL,           /* RegisterCMC */
    NULL,           /* UnregisterCMC */
    NULL,           /* DumpCmc */
    NULL,           /* DumpCmcAlarms */
    NULL,           /* DumpCmcLogs */
    
    NULL,           /* ResetCmcBoard */
    NULL,           /* GetCmcVersion */
    NULL,           /* GetCmcMaxMulticasts */
    NULL,           /* GetCmcMaxCm */
    NULL,           /* SetCmcMaxCm */
    
    NULL,           /* GetCmcTime */
    NULL,           /* SetCmcTime */
    NULL,           /* LocalCmcTime */
    NULL,           /* SetCmcCustomConfig */
    NULL,           /* DumpCmcCustomConfig */

    NULL,           /* DumpCmcDownChannel */
    NULL,           /* DumpCmcUpChannel */
    NULL,           /* SetCmcDownChannelMode */
    NULL,           /* SetCmcUpChannelMode */
    NULL,           /* SetCmcUpChannelD30Mode */
    
    NULL,           /* SetCmcDownChannelFreq */
    NULL,           /* SetCmcUpChannelFreq */
    NULL,           /* SetCmcDownAutoFreq */
    NULL,           /* SetCmcUpAutoFreq */
    NULL,           /* SetCmcUpChannelWidth */
    
    NULL,           /* SetCmcDownChannelAnnexMode */
    NULL,           /* SetCmcUpChannelType */
    NULL,           /* SetCmcDownChannelModulation */
    NULL,           /* SetCmcUpChannelProfile */
    NULL,           /* SetCmcDownChannelInterleaver */

    NULL,           /* SetCmcDownChannelPower */
    NULL,           /* SetCmcUpChannelPower */
    NULL,           /* DumpCmcUpChannelPower */
    NULL,           /* DumpCmcUpChannelSignalQuality */
    NULL,           /* DumpCmcInterfaceUtilization */
    
    NULL,           /* DumpCmcInterfaceStatistics */
    NULL,           /* DumpCmcMacStatistics */
    NULL,           /* DumpCmcAllInterface */
    NULL,           /* DumpCmcAllLoadBalancingGrp */
    NULL,           /* DumpCmcLoadBalancingGrp */
    
    NULL,           /* DumpCmcLoadBalancingGrpDownstream */
    NULL,           /* DumpCmcLoadBalancingGrpUpstream */
    NULL,           /* DumpCmcLoadBalancingDynConfig */
    NULL,           /* SetCmcLoadBalancingDynMethod */
    NULL,           /* SetCmcLoadBalancingDynDynPeriod */
    
    NULL,           /* SetCmcLoadBalancingDynWeightedAveragePeriod */
    NULL,           /* SetCmcLoadBalancingDynOverloadThresold */
    NULL,           /* SetCmcLoadBalancingDynDifferenceThresold */
    NULL,           /* SetCmcLoadBalancingDynMaxMoveNumber */
    NULL,           /* SetCmcLoadBalancingDynMinHoldTime */
    
    NULL,           /* SetCmcLoadBalancingDynRangeOverrideMode */
    NULL,           /* SetCmcLoadBalancingDynAtdmaDccInitTech */
    NULL,           /* SetCmcLoadBalancingDynScdmaDccInitTech */
    NULL,           /* SetCmcLoadBalancingDynAtdmaDbcInitTech */
    NULL,           /* SetCmcLoadBalancingDynScdmaDbcInitTech */

    NULL,           /* CreateCmcLoadBalancingGrp */
    NULL,           /* DestroyCmcLoadBalancingGrp */
    NULL,           /* AddCmcLoadBalancingGrpDownstream */
    NULL,           /* RemoveCmcLoadBalancingGrpDownstream */
    NULL,           /* AddCmcLoadBalancingGrpUpstream */
    
    NULL,           /* RemoveCmcLoadBalancingGrpUpstream */
    NULL,           /* AddCmcLoadBalancingGrpModem */
    NULL,           /* RemoveCmcLoadBalancingGrpModem */
    NULL,           /* AddCmcLoadBalancingGrpExcludeModem */
    NULL,           /* RemoveCmcLoadBalancingGrpExcludeModem */

    NULL,           /* DumpCmcLoadBalancingGrpModem */
    NULL,           /* DumpCmcLoadBalancingGrpActivedModem */
    NULL,           /* DumpCmcLoadBalancingGrpExcludeModem */
    NULL,           /* DumpCmcLoadBalancingGrpExcludeActivedModem */
    NULL,           /* ReserveCmcLoadBalancingGrp */

    NULL,           /* DumpAllCm */
    NULL,           /* DumpCm */
    NULL,           /* DumpAllCmHistory */
    NULL,           /* DumpCmHistory */
    NULL,           /* ClearAllCmHistory */

    NULL,           /* ResetCM */
    NULL,           /* DumpCmDownstream */
    NULL,           /* DumpCmUpstream */
    NULL,           /* SetCmDownstream */
    NULL,           /* SetCmUpstream */

    NULL,           /* CreateCmServiceFlow */
    NULL,           /* ModifyCmServiceFlow */
    NULL,           /* DestroyCmServiceFlow */
    NULL,           /* DumpCmClassifier */
    NULL,           /* DumpCmServiceFlow */

    NULL,           /* DumpCmcClassifier */
    NULL,           /* DumpCmcServiceFlow */
    NULL,           /* DumpCmcServiceFlowStatistics */
    NULL,           /* DumpCmcDownChannelBondingGroup */
    NULL,           /* DumpCmcUpChannelBondingGroup */   
    
    NULL,           /* CreateCmcServiceFlowClassName */
    NULL,           /* DestroyCmcServiceFlowClassName */   
    NULL,           /* GetCmcMacAddrTbl */   
    NULL,           /* GetCmMacAddrTbl */   
    NULL,           /* ResetCmAddrTbl */   
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL,
   NULL,
   NULL,
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

/* --------------------END------------------------ */

    REMOTE_ERROR
};


void GWONU_Pas8411_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS8411_GW, &s_gwonu8411Ifs);
}

void CTCONU_Pas8411_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS8411_CTC, &s_ctconu8411Ifs);
}


#ifdef __cplusplus

}

#endif

/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif

