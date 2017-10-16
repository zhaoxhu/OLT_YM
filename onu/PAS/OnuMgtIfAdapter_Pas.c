/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_Pas.c
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
**  10/05/1    |   shixh    |     create 
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

int g_iOnuBwSetFirstFailed  = 0;
int g_iOnuBwSetSecondFailed = 0;
int g_iOnuBwSetFailedDelay  = 200;

int ctc_voip_debug = 0;
#define CTC_VOIP_DEBUG if(ctc_voip_debug) sys_console_printf

extern PON_onu_address_table_record_t  MAC_Address_Table[8192];
/*extern USHORT onu_mac_check_counter[SYS_MAX_PON_PORTNUM][MAXONUPERPON];*/

/****************************************�ڲ�����*******************************************/

#if 1
/* -------------------ONU����API------------------- */

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

static int GWONU_PAS5001_OnuIsOnline(short int olt_id, short int onu_id, int *status)
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_OnuIsOnline(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* ONU �ֶ����*//*add by shixh20100908*/
static int GWONU_PAS5001_AddOnuByManual(short int olt_id, short int onu_id, unsigned char *MacAddr)
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
        /* ǿ�а�ռ��¼��ַ */
        if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {
            PAS_deregister_onu( olt_id, llid, FALSE );
        }
        OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_AddOnuByManual(%d,%d) deregister onu(%s) for onu(%s) on slot %d.\r\n", olt_id, onu_id, macAddress_To_Strings(pSrcMac), macAddress_To_Strings(MacAddr), SYS_LOCAL_MODULE_SLOTNO);
    }   

    return( ROK );
}

/* ɾ��ONU */
static int GWONU_PAS5001_DelOnuByManual(short int olt_id, short int onu_id)
{	
    short int OnuEntry;
    short int llid;
      
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( activeOneLocalPendingOnu(olt_id, onu_id) == RERROR )
	{
	    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	    {
	        PAS_deregister_onu( olt_id, llid, FALSE );
	    }
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return ROK;
}

static int GWONU_PAS5001_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS5001;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int GWONU_PAS5201_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS5201;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_PAS5201_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS5201;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}

static int GWONU_PAS5204_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS5204;
    *remote_type = ONU_MANAGE_GW;
    return OLT_ERR_OK;
}

static int CTCONU_PAS5204_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_PAS5204;
    *remote_type = ONU_MANAGE_CTC;
    return OLT_ERR_OK;
}
#endif


#if 1
/* -------------------ONU ��֤����API------------------- */
/*onu deregister*/
static int GWONU_PAS5001_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PAS_deregister_onu( olt_id, llid, FALSE );
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_DeregisterOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS5001_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( auth_mode )
        {
            iRlt = PAS_authorize_onu(olt_id, llid, PON_AUTHORIZE_TO_THE_NETWORK);
        }
        else
        {
            iRlt = PAS_authorize_onu(olt_id, llid, PON_DENY_FROM_THE_NETWORK);
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_AuthorizeOnu(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_PAS5201_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_auth_request(olt_id, llid, auth_response);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_AuthRequest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

	OLT_ASSERT(olt_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_auth_success(olt_id, llid);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_AuthSuccess(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
    
	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_auth_failure(olt_id, llid, failure_type);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_AuthFailure(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, failure_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
#endif


#if 1
/* -------------------ONU ҵ�����API------------------- */
/*traffic service enable*/
/* 5001��5201���ô˺���*//*���ⵥ11283*/
static int GWONU_PAS5001_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt = 0;
    short int llid;
    short int OnuEntry;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    /* modified by xieshl 20111010, ����ȥ����ONUʱ��ɷ�������ע�ᣬ���ⵥ13516 */
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
	            iRlt = PAS_deregister_onu( olt_id, llid, FALSE );
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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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
            iRlt = PAS_deregister_onu( olt_id, llid, FALSE );
        }
    }
    else 
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P, 5001��5201����*/
static int GWONU_PAS5001_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
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
    	iRlt = PAS_set_p2p_access_control( olt_id, Llid1, 1, llidArray, access );
    }
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id1, onu_id2, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* P2P��ONU���Ƿ�ת����ЧMAC��5001��5201����*/  /*���ⵥ11315*/
static int GWONU_PAS5001_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast)
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
    	
    	iRlt = PAS_set_llid_p2p_configuration( olt_id, llid, address_b, broadcast_b );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, address_not_found, broadcast, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GWONU_platoDBA2_0_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    PLATO2_SLA_t  SLA;
    short DBA_error_code;
    int bw_val;

    /*added by chenfj 2006-12-21 
    	   ���ô���ʱ������class, delay, ���֤���������ô������
       */
    /*
          SLA.class = 2;
          SLA.delay = 1;
       */
    if( BW->bw_class > PRECEDENCE_OF_FLOW_7 )
    	SLA.class = OnuConfigDefault.UplinkClass;
    else
    	SLA.class = BW->bw_class;

    if(( BW->bw_delay == V2R1_DELAY_HIGH )
    	|| ( BW->bw_delay == V2R1_DELAY_LOW ))
    	SLA.delay = BW->bw_delay;
    else
    	SLA.delay = OnuConfigDefault.UplinkDelay;

    /* modified by chenfj 2007-7-16
            ����һ�������������,���ڲ��Դ�����
       */
    bw_val = (BW->bw_gr * Bata_ratio) / 1000 ;
    SLA.max_gr_bw =  bw_val / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    SLA.max_gr_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) SLA.max_gr_bw_fine ++;

    /*added by chenfj 2006-12-21 
    	   ���ô���ʱ������class, delay, ���֤���������ô������
       */
    /*
          SLA.max_be_bw = SLA.max_gr_bw + 1;   
          SLA.max_be_bw_fine = SLA.max_gr_bw_fine  ; 
       */

    /* modified by chenfj 2007-7-16
            ����һ�������������,���ڲ��Դ�����
       */
    bw_val = (BW->bw_be * Bata_ratio)/1000 ;
    SLA.max_be_bw = bw_val  / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    SLA.max_be_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) SLA.max_be_bw_fine++;

    if( SLA.max_be_bw < SLA.max_gr_bw )
    {
    	SLA.max_be_bw = SLA.max_gr_bw;
    	SLA.max_be_bw_fine = SLA.max_gr_bw_fine;
    }
    
    /*sys_console_printf("onu %d/%d bandwidth setting,gr=%dM%dKB, be=%dM%dKB\r\n", PonPortIdx, (OnuIdx+1), SLA.max_gr_bw,(SLA.max_gr_bw_fine*BITS_PER_SECOND_64K), SLA.max_be_bw, (SLA.max_be_bw_fine * BITS_PER_SECOND_64K ) );*/
    /*
    SLA.max_gr_bw =  PonPortTable[PonPortIdx].DefaultOnuBW / BITS_PER_SECOND_1M;				
    SLA.max_gr_bw_fine = 0;  
    SLA.max_be_bw = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr / BITS_PER_SECOND_1M;
    Bw = OnuMgmtTable[ PonPortIdx * MAXONUPERPON + OnuIdx ].LlidTable[0].UplinkBandwidth_gr % BITS_PER_SECOND_1M;
    SLA.max_be_bw_fine =  Bw / BITS_PER_SECOND_64K;; 
    */
    iRlt = PLATO2_set_SLA(olt_id, llid, (PLATO2_SLA_t *)&SLA, &DBA_error_code) ;
    if( iRlt != PAS_EXIT_OK )
    {
    	iRlt = PLATO2_set_SLA(olt_id, llid, (PLATO2_SLA_t *)&SLA, &DBA_error_code) ;
    }

    if (PAS_EXIT_OK == iRlt)
    {
        /* B--added by liwei056@2010-1-13 for D9425 */
        if ( DBA_error_code != PLATO2_ECODE_NO_ERROR )
        {
            /* B--modified by liwei056@2010-11-30 for D11310 */
            iRlt = OLT_ERR_DBA_BEGIN - DBA_error_code;
            /* E--modified by liwei056@2010-11-30 for D11310 */
        }
        /* E--added by liwei056@2010-1-13 for D9425 */
    }
        
    return iRlt;
}

static int GWONU_platoDBA3_3_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    PLATO3_SLA_t  V3_SLA;
    short DBA_error_code = 0;
    int bw_val;

    if( BW->bw_class > PRECEDENCE_OF_FLOW_7 )
    	V3_SLA.class = OnuConfigDefault.UplinkClass;
    else
    	V3_SLA.class = BW->bw_class;
    	
    V3_SLA.fixed_packet_size = uplinkBWPacketUnitSize;

    bw_val = BW->bw_fixed;			
    V3_SLA.fixed_bw = bw_val / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V3_SLA.fixed_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.fixed_bw_fine++;

    bw_val = (BW->bw_gr * Bata_ratio) / 1000 ;
    V3_SLA.max_gr_bw = bw_val / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V3_SLA.max_gr_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.max_gr_bw_fine++;

    bw_val = (BW->bw_be * Bata_ratio) / 1000 ;
    V3_SLA.max_be_bw = bw_val  / BITS_PER_SECOND_1M; 
    bw_val = bw_val % BITS_PER_SECOND_1M;
    V3_SLA.max_be_bw_fine = bw_val / BITS_PER_SECOND_64K;  
    if( ( bw_val % BITS_PER_SECOND_64K ) != 0 ) V3_SLA.max_be_bw_fine++;

    if(V3_SLA.max_be_bw == ( V3_SLA.fixed_bw + V3_SLA.max_gr_bw ))
    {
    	if( V3_SLA.max_be_bw_fine < (V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine ) )
    	{
    		V3_SLA.max_be_bw_fine = V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine;
    	}
    }
    else if(V3_SLA.max_be_bw < ( V3_SLA.fixed_bw + V3_SLA.max_gr_bw ))
    {
    	V3_SLA.max_be_bw = V3_SLA.fixed_bw + V3_SLA.max_gr_bw;
    	V3_SLA.max_be_bw_fine = V3_SLA.fixed_bw_fine + V3_SLA.max_gr_bw_fine;
    }

    iRlt = PLATO3_set_SLA(olt_id, llid, (PLATO3_SLA_t *)&V3_SLA, &DBA_error_code) ;
    if( iRlt != PAS_EXIT_OK )
    {
        g_iOnuBwSetFirstFailed++;

        if ( g_iOnuBwSetFailedDelay > 0 )
        {
            VOS_TaskDelay( (ULONG)((VOS_TICK_SECOND * g_iOnuBwSetFailedDelay) / 1000) );
        }
        
        /* ��������2�Σ����ܱ�֤���ɹ��ĳɹ�? (�¸����Ĵ���) */
    	iRlt = PLATO3_set_SLA(olt_id, llid, (PLATO3_SLA_t *)&V3_SLA, &DBA_error_code) ;
    }

    if (PAS_EXIT_OK == iRlt)
    {
        /* B--added by liwei056@2010-1-13 for D9425 */
        if ( DBA_error_code != PLATO3_ECODE_NO_ERROR )
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

static int GWONU_PAS5001_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt;
    PON_policer_t policer_id;
    PON_policing_struct_t  policing_struct;

    policer_id = PON_POLICER_DOWNSTREAM_TRAFFIC;

    if ( BW->bw_gr > 0 )
    {
        policing_struct.maximum_bandwidth = BW->bw_gr;
    }
    else
    {
        policing_struct.maximum_bandwidth = BW->bw_be;
    }
    
    if(policing_struct.maximum_bandwidth > MAX_DOWNLINK_SPEED)
    	policing_struct.maximum_bandwidth = MAX_DOWNLINK_SPEED;

#if 0
    Real_bw = BW->bw_gr;
    Config_bw = Real_bw /(1000000 - Real_bw)  ;
    sys_console_printf("\r\n config bw %d \r\n", Config_bw);
    Config_bw = Config_bw * 1000000;			
    sys_console_printf("\r\n config bw %d %d \r\n", Config_bw, Real_bw );
#endif

    /* modified by chenfj 2006/11/28
              ���ⵥ#3267: ��ONU�ڵ���ѧ����OLT�ϵ�ǧ�׿ڷ�������ԴMAC,onu�ղ���OLT�������İ� */
    policing_struct.maximum_burst_size = 5580000; /* 0~8388480 */
    policing_struct.high_priority_frames_preference = DISABLE;
    policing_struct.short_frames_preference = DISABLE;

    if( (policing_struct.maximum_bandwidth > 0)
        && ((PonPortTable[olt_id].DownlinkPoliceMode /* downlinkBWlimit */ == V2R1_ENABLE)
        || (OLT_CFG_DIR_INACTIVE & BW->bw_direction)) )
    {
    	iRlt = PAS_set_policing_parameters_v4( olt_id, llid,  policer_id, ENABLE, &policing_struct );
        if (0 == iRlt)
        {
            /* ���������� */
            BW->bw_actived = BW->bw_gr;
        }
    }
    else
    {
        iRlt = PAS_set_policing_parameters_v4( olt_id, llid,  policer_id, DISABLE, &policing_struct );
        if (0 == iRlt)
        {
            /* ���������� */
            BW->bw_actived = 0;
        }
    }

    return iRlt;
}

static int GWONU_PAS5201_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
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
    	iRlt = PAS_set_policing_parameters(olt_id, llid, policer_id, ENABLE, policing_param);
        if (0 == iRlt)
        {
            /* ���������� */
            BW->bw_actived = BW->bw_gr;
        }
    }
    else
    {
        iRlt = PAS_set_policing_parameters(olt_id, llid, policer_id, DISABLE, policing_param);
        if (0 == iRlt)
        {
            /* ���������� */
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

            /* ���������µ�Ĭ�ϴ���ʧ��Ҳ�Ǻ����*/
            iRlt = 0;
            DownlinkActiveBW = -1;
            if ( !(OLT_CFG_DIR_UNDO & BW->bw_direction)
                || (OLT_CFG_DIR_DOWNLINK & pstOnuCfg->BandWidthIsDefault) )
            {
                /* ����������Ĭ�ϴ��� */
                VOS_MemZero(&BW_NewDefault, sizeof(ONU_bw_t));
                BW_NewDefault.bw_direction = OLT_CFG_DIR_DOWNLINK;
                BW_NewDefault.bw_gr = BW->bw_fixed;                
                BW_NewDefault.bw_be = BW->bw_actived;                
                if ( 0 == chip_id )
                {
                    iRlt = GWONU_PAS5201_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                }
                else
                {
                    iRlt = GWONU_PAS5001_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                }
                if ( 0 == iRlt )
                {
                    /* �������м������ */
                    DownlinkActiveBW = BW_NewDefault.bw_actived;
                    BW->bw_delay = DownlinkActiveBW;
                }
            }

            if ( 0 == iRlt )
            {
                if ( !(OLT_CFG_DIR_UNDO & BW->bw_direction)
                    || (OLT_CFG_DIR_UPLINK & pstOnuCfg->BandWidthIsDefault) )
                {
                    /* ����������Ĭ�ϴ��� */
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

#ifdef PLATO_DBA_V3
                    iRlt = GWONU_platoDBA3_3_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);
#else
                    iRlt = GWONU_platoDBA2_0_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);
#endif
                    if ( 0 == iRlt )
                    {
                        /* ��������м������ */
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
                                /* ��������ʧ�ܣ�����ָ��ɵ�����Ĭ�ϴ��� */
                                VOS_MemZero(&BW_NewDefault, sizeof(ONU_bw_t));
                                BW_NewDefault.bw_direction = OLT_CFG_DIR_DOWNLINK;
                                BW_NewDefault.bw_gr = pstOnuCfg->FinalDownlinkBandwidth_gr;                
                                BW_NewDefault.bw_be = pstOnuCfg->FinalDownlinkBandwidth_be;                
                                if ( 0 == chip_id )
                                {
                                    (void)GWONU_PAS5201_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                                }
                                else
                                {
                                    (void)GWONU_PAS5001_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
                                }
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
                    /*һ��onuͬʱ�л���onuid��mac�Ĵ�������ʱ��undo macʱӦ�ָ�Ϊ����onuid������*/
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
                
#ifdef PLATO_DBA_V3
                iRlt = GWONU_platoDBA3_3_SetOnuUplinkBW(olt_id, llid, BW);
#else
                iRlt = GWONU_platoDBA2_0_SetOnuUplinkBW(olt_id, llid, BW);
#endif
                if (0 == iRlt)
                {
                    /* ���������� */
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
                    /*һ��onuͬʱ�л���onuid��mac�Ĵ�������ʱ��undo macʱӦ�ָ�Ϊ����onuid������*/                    
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
                
                if ( 0 == chip_id )
                {
                    iRlt = GWONU_PAS5201_SetOnuDownlinkBW(olt_id, llid, BW);
                }
                else
                {
                    iRlt = GWONU_PAS5001_SetOnuDownlinkBW(olt_id, llid, BW);
                }
            }
        }
    }   
    
    return iRlt;
}

static int  GWONU_PAS5001_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
{
    int iRlt;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(BW);

    iRlt = GWONU_PAS_SetOnuBW(olt_id, onu_id, BW, 1);
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int  GWONU_PAS5201_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5201_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int  GWONU_platoDBA2_0_GetSLA(short int olt_id, short int llid, ONU_SLA_INFO_t *sla_info)
{
    sla_info->SLA_Ver = 2;
	return PLATO2_get_SLA( olt_id, (unsigned char)llid, &(sla_info->SLA.SLA2), &sla_info->DBA_ErrCode);
}

static int GWONU_platoDBA3_0_GetSLA(short int olt_id, short int llid, ONU_SLA_INFO_t *sla_info)
{
    sla_info->SLA_Ver = 3;
	return PLATO3_get_SLA( olt_id, (unsigned char)llid, &(sla_info->SLA.SLA3), &sla_info->DBA_ErrCode);
}

static int GWONU_PAS5001_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
        
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);
    
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#ifdef PLATO_DBA_V3
    	iRlt = GWONU_platoDBA3_0_GetSLA( olt_id, llid, sla_info );
#else
    	iRlt = GWONU_platoDBA2_0_GetSLA( olt_id, llid, sla_info );
#endif
    }   
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_GetOnuSLA(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, sla_info->SLA_Ver, sla_info->DBA_ErrCode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS5201_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
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
    		sRlt = PAS_set_llid_fec_mode(olt_id, GW1G_PAS_BROADCAST_LLID, ENABLE);
    		iRlt = PAS_set_llid_fec_mode(olt_id, llid, ENABLE);
    	}
    	else
    	{
    		PON_fec_status_raw_data_t fec_status;
    		PON_timestamp_t           timestamp; 
    			
    		iRlt = PAS_set_llid_fec_mode(olt_id, llid, DISABLE);
    		sRlt = PAS_get_raw_statistics( olt_id,
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
    				sRlt = PAS_set_llid_fec_mode(olt_id, GW1G_PAS_BROADCAST_LLID, DISABLE);
                }      
            }
    	}
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5201_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

static int CTCONU_PAS5201_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
        CTC_STACK_standard_FEC_ability_t Fec_Ability;

        iRlt = CTC_STACK_get_fec_ability(olt_id, llid, &Fec_Ability);    
    	if ( (iRlt != 0)
            || (Fec_Ability != STD_FEC_ABILITY_SUPPORTED) )
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            iRlt = CTC_STACK_set_fec_mode(olt_id, llid, (CTC_STACK_standard_FEC_mode_t)fec_mode);    
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

/*get onu vlan mode*/
static int GWONU_PAS5201_GetOnuVlanMode(short int olt_id, short int onu_id, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(vlan_uplink_config);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	iRlt = PAS_get_llid_vlan_mode( olt_id, llid, vlan_uplink_config );
    }   
	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5201_GetOnuVlanMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;	
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS5001_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_uni_set_port( olt_id, llid, enable_cpu, enable_datapath );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetUniPort(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable_cpu, enable_datapath, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS5001_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_set_slow_protocol_limit( olt_id, llid, enable);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS5001_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_get_slow_protocol_limit( olt_id, llid, enable);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetSlowProtocolLimit(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

#endif


#if 1
/* -------------------ONU ���ͳ�ƹ���API------------------- */

static int GWONU_PAS5001_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	if ( 0 == (iRlt = REMOTE_PASONU_clear_all_statistics ( olt_id, llid )) )
        {
            /* B--added by liwei056@2011-7-11 for D12785 */
            PAS_reset_olt_counters(olt_id);
            /* E--added by liwei056@2011-7-11 for D12785 */
        }   
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_ResetCounters(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS5201_SetPonLoopback(short int olt_id, short int onu_id, int enable)
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

    	iRlt = PAS_set_standard_onu_loopback( olt_id, llid, loop_mode );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5201_SetPonLoopback(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

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
/* -------------------ONU���ܹ���API------------------- */
/* get onu pon Llid parameters */
static int GWONU_PAS5001_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	iRlt = PAS_get_llid_parameters ( olt_id, llid, llid_parameters );
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetLLIDParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*onu start encryption */
static int GWONU_PAS5001_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
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
    	if ( 0 == (iRlt = PAS_start_encryption ( olt_id, llid, (PON_encryption_direction_t)(enc_dir - PON_ENCRYPTION_PURE) )) )
        {
            /* ��ʶ�����Ѿ����� */
            *encrypt_dir = 0;
        }   
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_StartEncryption(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

/*onu stop encryption*/
static int GWONU_PAS5001_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	 
	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    			
    	iRlt = PAS_stop_encryption ( olt_id, llid );
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_StopEncryption(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

static int GWONU_PAS5001_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
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

        	if ( 0 == (iRlt = PAS_get_llid_parameters ( olt_id, llid, &llid_parameters )) )
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
                        iRlt = PAS_stop_encryption ( olt_id, llid );
                    }
                    
                    if ( PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION != new_dir )
                    {
                        if ( (PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION != old_dir)
                            && (0 == iRlt) )
                        {
                            extern int EncryptKeyDelayTime;
                        
                            /* ���ܷ���仯������ʱ0.5��*/
                  	        VOS_TaskDelay(EncryptKeyDelayTime); 
                        }
                        
                    	if ( 0 == (iRlt = PAS_start_encryption ( olt_id, llid, new_dir )) )
                        {
                            /* ��ʶ�����Ѿ����� */
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

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enc_dir, key_change_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}	

static int GWONU_PAS5001_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
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

                /* �˲��������ٻ�ȡ */
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
            
        	if ( 0 == (iRlt = PAS_get_llid_parameters ( olt_id, llid, &llid_parameters )) )
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
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuEncryptParams(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
					
	return iRlt;			
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS5001_UpdateEncryptionKey(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method)
{
    int iRlt;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
		iRlt = PAS_update_encryption_key(olt_id, llid, encryption_key, key_update_method);
	}
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_UpdateEncryptionKey(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU ��ַ����API------------------- */
/*show fdbentry mac*/
static int GWONU_PAS5001_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table)
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
        if ( 0 == (iRlt = REMOTE_PASONU_address_table_get(olt_id, llid, &liEntryNum, addr_table)) )
        {
            *EntryNum = (long)liEntryNum;
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*show fdbentry mac*/
static int GWONU_PAS5001_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
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
        if ( 0 == (iRlt = PAS_get_address_table_llid(olt_id, llid, &addr_num, addr_table)) )
        {
            *EntryNum = (long)addr_num;
        }
    }   
	
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*set onu max mac*/
static int GWONU_PAS5001_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
{
    static int s_iPas5001MacAdrLimits[] = {0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1022, 8191}; 
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned int uiSetVal;
    short int llid;
    short int max_entry;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ID_ASSERT(llid_id);
    VOS_ASSERT(val);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
#if 1
        int i, iSub1, iSub2;

        i = ARRAY_SIZE(s_iPas5001MacAdrLimits) - 1;
        uiSetVal = *val;
        if ( (0 < uiSetVal) && (uiSetVal <= s_iPas5001MacAdrLimits[i]) )
        {
            for (--i; i>=0; i--)
            {
                if ( uiSetVal >= s_iPas5001MacAdrLimits[i] )
                {
                    iSub1 = uiSetVal - s_iPas5001MacAdrLimits[i];
                    iSub2 = s_iPas5001MacAdrLimits[i+1] - uiSetVal;

                    if (iSub1 >= iSub2)
                    {
                        i++;
                    }

                    break;
                }
            }
        }

        if ( i > 0 )
        {
            max_entry = s_iPas5001MacAdrLimits[i];
            if ( 0 < uiSetVal )
            {
                uiSetVal = s_iPas5001MacAdrLimits[i];
            }
            
            if ( 0 == (iRlt = PAS_set_port_mac_limiting(olt_id, llid, max_entry)) )
            {
                *val = uiSetVal;
            }
        }
        else
        {
            iRlt = OLT_ERR_PARAM;
        }
#else        
    	if(( uiSetVal > (PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022+2)) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_8192 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022;
    	if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_512) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_1022 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_512;
    	if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_256 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_512 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_256;
    	if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_128 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_256 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_128;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_64 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_128 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_64;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_32 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_64 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_32;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_16 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_32 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_16;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_8 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_16))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_8;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_4 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_8 ))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_4;
    	else if(( uiSetVal >= PON_ADDRESS_TABLE_ENTRY_LIMITATION_2 ) &&( uiSetVal < PON_ADDRESS_TABLE_ENTRY_LIMITATION_4))
    		max_entry = PON_ADDRESS_TABLE_ENTRY_LIMITATION_2;
    	else
            max_entry =(PON_address_table_entries_llid_limitation_t) uiSetVal;
#endif        
    }
    
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GWONU_PAS5201_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
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
        /*del by luh 2013-8-14*/
        /*uiSetVal = *val;*/ 
        if( 0 == uiSetVal ) 
        {
            max_entry = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
        }
        else
        {
            if( uiSetVal >= /*PAS_ADDRESS_TABLE_SIZE*/PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION )	/* modified by xieshl 20121030, ���ⵥ16118 */
            {
                uiSetVal  = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
                max_entry = PON_ADDRESS_TABLE_ENTRY_MAX_LIMITATION;
            }
            else 
            {
                /* ����ONU�Լ���һ��MAC */
                max_entry = uiSetVal + 1;
            }
        }
		/*added by liyang @2015-05-20 for clear mac addr before setting limit  Q25860*/
		if(0 == (iRlt = PAS_reset_address_table(olt_id, llid, ADDR_DYNAMIC)))
		{
			if ( 0 == (iRlt = PAS_set_port_mac_limiting(olt_id, llid, max_entry)) )
        	{
            	*val = uiSetVal;
        	}
		}
		 
        
    }   
	
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5201_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*get onu pon uni mac config*/
static int GWONU_PAS5001_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_config);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PAS_get_onu_uni_port_mac_configuration_v4(olt_id, llid, mac_config);
    }   
					
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuUniMacCfg(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU ��·����API------------------- */
/*RTT*/
static int GWONU_PAS5001_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	onu_registration_data_record_t onu_registration_data;
        
		iRlt = PAS_get_onu_registration_data(olt_id, llid, &onu_registration_data);
		if ( iRlt == PAS_EXIT_OK )
        {
			*rtt = onu_registration_data.rtt * 16 / 10;
        }      
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuDistance(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GWONU_PAS5001_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        PAS_physical_capabilities_t device_capabilities;

		iRlt = PAS_get_device_capabilities_v4(olt_id, llid, &device_capabilities);
		if ( iRlt == PAS_EXIT_OK )
        {
            VOS_MemCpy(optical_capability, &device_capabilities, sizeof(ONU_optical_capability_t));
        }      
	}

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOpticalCapability(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}
#endif


#if 1
/* -------------------ONU ����API---------------- */

static int GWONU_PAS5001_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);

    if ( 0 <= (iRlt = PAS_get_onu_mode(olt_id, llid)) )
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
    
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuLLID(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU �豸����API------------------- */
/*get onu ver*/
static int GWONU_PAS5001_GetOnuVer(short int olt_id, short int onu_id, PON_onu_versions *onu_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_versions);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = PAS_get_onu_version ( olt_id, llid, onu_versions );
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}	

/*get onu pon chip ver*/
static int GWONU_PAS5001_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
		
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = PAS_get_device_versions_v4 ( olt_id, llid, (PAS_device_versions_t*)device_versions )) )
        {
            device_versions->pon_vendors = PON_VENDOR_PMC;
        }
    }   
	 
	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuPonVer(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int GWONU_PAS5001_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
	int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	onu_registration_data_record_t onu_registration_data;
	PON_onu_versions onu_version_info;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if ( 0 == (iRlt = PAS_get_onu_version ( olt_id, llid, &onu_version_info )) )
        {
            if ( 0 == (iRlt = PAS_get_onu_registration_data ( olt_id, llid, &onu_registration_data )) )
            {
                onu_info->oam_version = onu_registration_data.oam_version;
                onu_info->rtt = onu_registration_data.rtt;
                
                onu_info->laser_on_time  = onu_registration_data.laser_on_time;
                onu_info->laser_off_time = onu_registration_data.laser_off_time;

                onu_info->vendorType     = OnuVendorTypesPmc;
                onu_info->productVersion = onu_version_info.hardware_version;
                onu_info->productCode    = onu_info->productVersion;
            
                onu_info->max_links_support = 1;
                onu_info->curr_links_num = 1;
            
                onu_info->max_cm_support = 0;
                onu_info->pon_rate_flags = PON_RATE_NORMAL_1G;
            }
        }
    }   

	OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuRegisterInfo(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

static int GWONU_PAS5001_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_eeprom_mapper_get_parameter( olt_id, llid, info_id, data, size );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS5001_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_eeprom_mapper_set_parameter( olt_id, llid, info_id, data, size );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int GWONU_PAS5001_ResetOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_reset_device( olt_id, llid, PON_RESET_SW );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_RemoteReset(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

static int CTCONU_PAS5201_ResetOnu(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_reset_onu( olt_id, llid );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_RemoteReset(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int GWONU_PAS5001_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = REMOTE_PASONU_get_burn_image_complete( olt_id, llid, complete );
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GWONU_PAS5001_Get_Burn_Image_Complete(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

#endif


#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */

static int CTCONU_PAS5201_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_onu_version(olt_id, llid, version);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetCtcVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_firmware_version(olt_id, llid, version);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetFirmwareVersion(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *version, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_onu_serial_number(olt_id, llid, serial_number);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetSerialNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_chipset_id(olt_id, llid, chipset_id);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetChipsetID(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_onu_capabilities(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetOnuCap1(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_onu_capabilities_2(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetOnuCap2(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_onu_capabilities_3(olt_id, llid, caps);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetOnuCap3(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_UpdateOnuFirmware( short int olt_id, short int onu_id, void *file_start, int file_len, char *file_name )
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
        iRlt = CTC_STACK_update_onu_firmware(olt_id, llid, &f_image, file_name);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_UpdateOnuFirmware(%d, %d, %d, %s)'s result(%d) on slot %d.\r\n", olt_id, onu_id, file_len, file_name, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_ActiveOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_activate_image(olt_id, llid);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_ActiveOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_CommitOnuFirmware( short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_commit_image(olt_id, llid);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_CommitOnuFirmware(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}



static int CTCONU_PAS5201_GetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int *timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    unsigned short int tm;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetExtentOamDiscoveryTimeout(short int olt_id, short int onu_id, int timeout)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_set_extended_oam_discovery_timing((unsigned short int)timeout);
        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetExtentOamDiscoveryTimeout(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, timeout, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEncryptionTiming(short int olt_id, short int onu_id, int *update_time, int *no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar update_key_t;
        ushort no_reply_t;

        iRlt = CTC_STACK_get_encryption_timing(&update_key_t, &no_reply_t);

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

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEncryptionTiming(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEncryptionTiming(short int olt_id, short int onu_id, int update_time, int no_reply)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar update_key_t = update_time;
        ushort no_reply_t = no_reply;

        iRlt = CTC_STACK_set_encryption_timing(update_key_t, no_reply_t);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEncryptionTiming(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, update_time, no_reply, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_StartEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_start_encryption(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_StartEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_StopEncrypt(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_stop_encryption(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_StopEncrypt(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortLinkstate(short int olt_id, short int onu_id,  int port, int *ls )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_ethernet_link_state(olt_id, llid, lport, ls);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortLinkstate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEthPortAdminstatus(short int olt_id, short int onu_id,  int port, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = CTC_STACK_get_phy_admin_state(olt_id, llid, lport, &en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *enable = en;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortAdminstatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortAdminstatus(short int olt_id, short int onu_id,  int port, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if(GetOnuOperStatus(olt_id, onu_id) == ONU_OPER_STATUS_UP)
    {
        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {

            uchar lport = port;
            bool en = enable;

            iRlt = CTC_STACK_set_phy_admin_control(olt_id, llid, lport, en);

            if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortAdminstatus(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortPauseEnable(short int olt_id, short int onu_id,  int port, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = CTC_STACK_get_ethernet_port_pause(olt_id, llid, lport, &en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *enable = en;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortPauseEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortPauseEnable(short int olt_id, short int onu_id,  int port, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = enable;

        iRlt = CTC_STACK_set_ethernet_port_pause(olt_id, llid, lport, en);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortPauseEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id,  int port, int *an )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool lan = FALSE;

        iRlt = CTC_STACK_get_auto_negotiation_admin_state(olt_id, llid, lport, &lan);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
            *an = lan;

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortAutoNegotinationAdmin(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id,  int port, int an )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port;

        iRlt = CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, an);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else if(an == TRUE)/*ֻ���ڼ���ʱ��restart*/
        {
            CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortAutoNegotinationAdmin(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortRestartAutoConfig(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortRestartAutoConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEthPortAnLocalTecAbility(short int  olt_id, short int onu_id, int port, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_auto_negotiation_local_technology_ability(olt_id, llid, port, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortAnLocalTecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEthPortAnAdvertisedTecAbility(short int  olt_id, short int onu_id, int port, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_auto_negotiation_advertised_technology_ability(olt_id, llid, port, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortAnAdvertisedTecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_policing_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool en = FALSE;

        iRlt = CTC_STACK_get_ethernet_port_policing(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_ethernet_port_policing(olt_id, llid, lport, *policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t * policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_ethernet_port_ds_rate_limiting(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id,  int port, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, lport, policing);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port, CTC_STACK_port_vlan_configuration_t * vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, vconf);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortVlanConfig(short int olt_id, short int onu_id,  int port, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, *vconf);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}

static int CTCONU_PAS5201_GetAllPortVlanConfig(short int olt_id, short int onu_id,  unsigned char* portNum, CTC_STACK_vlan_configuration_ports_t   ports_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, portNum, ports_info);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetAllPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, portNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port configuration*/
static int CTCONU_PAS5201_SetPortQinqConfig(short int olt_id, short int onu_id, int port, CTC_STACK_port_qinq_configuration_t   port_configuration )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
        {

		uchar lport = port;

		iRlt = CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, port_configuration);

        	if (iRlt != 0)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }

     }

	OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortQinqConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_classification_and_marking(olt_id, llid, lport, cam);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port,
        CTC_STACK_classification_rule_mode_t  mode, CTC_STACK_classification_rules_t cam )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_classification_and_marking(olt_id, llid, lport, mode, cam);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_multicast_vlan(olt_id, llid, lport, mv);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_multicast_vlan(olt_id, llid, lport, *mv);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_ClearEthPortMulticastVlan(short int olt_id, short int onu_id,  int port )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_clear_multicast_vlan(olt_id, llid, lport);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_ClearEthPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port, int *num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar  lport = port, gnum=0;

        iRlt = CTC_STACK_get_multicast_group_num(olt_id, llid, lport, &gnum);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            *num = gnum;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port, int num )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port, gnum=num;

        iRlt = CTC_STACK_set_multicast_group_num(olt_id, llid, lport, gnum);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port, int *strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool fstrip = FALSE;

        iRlt = CTC_STACK_get_multicast_tag_strip(olt_id, llid, lport, &fstrip);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
        else
        {
            *strip = fstrip;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port, int strip )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;
        bool fstrip = strip;

        iRlt = CTC_STACK_set_multicast_tag_strip(olt_id, llid, lport, fstrip);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetMulticastAllPortTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_get_multicast_all_port_tag_strip(olt_id, llid, number_of_entries, ports_info);
	}
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetMulticastAllPortTagStrip(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
    
}

static int CTCONU_PAS5201_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_get_multicast_tag_oper(olt_id, llid, lport, oper, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        uchar lport = port;

        iRlt = CTC_STACK_set_multicast_tag_oper(olt_id, llid, lport, oper, *sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_set_multicast_management_object_tag_oper(olt_id, llid, management_object->index, tag_oper, *sw);
	}
	
	return iRlt;
}


static int CTCONU_PAS5201_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_multicast_control(olt_id, llid, mc);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
static int CTCONU_PAS5201_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_set_multicast_control(olt_id, llid, *mc);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_ClearMulticastControl(short int olt_id, short int onu_id )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_clear_multicast_control(olt_id, llid);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_ClearMulticastControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t *sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_multicast_switch(olt_id, llid, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t sw)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_set_multicast_switch(olt_id, llid, sw);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetMulticastSwitch(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetFastLeaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_fast_leave_ability(olt_id, llid, ability);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetFastLeaveAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetFastLeaveAdminState(short int olt_id, short int onu_id, int *fla)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_get_fast_leave_admin_state(olt_id, llid, fla);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetFastLeaveAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetFastLeaveAdminState(short int olt_id, short int onu_id, int fla)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {

        iRlt = CTC_STACK_set_fast_leave_admin_control(olt_id, llid, fla);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetFastLeaveAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port_id;

        if( OnuSupportStatistics(olt_id, onu_id) == VOS_OK )	/* ���ⵥ16117 */
            iRlt = CTC_STACK_get_ethport_statistic_data(olt_id, llid, lport, data);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetPortStatisticData(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

static int CTCONU_PAS5201_GetPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port_id;

        iRlt = CTC_STACK_get_ethport_statistic_state(olt_id, llid, lport, state);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

static int CTCONU_PAS5201_SetPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = (uchar)port_id;

        iRlt = CTC_STACK_set_ethport_statistic_state(olt_id, llid, lport, *state);

        if (iRlt != 0)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}

int ctctest_statistic(int slot, int pon, int onu, int port, int vcode, ... )
{

    int arg, arg1;
    int rc = 0;

    CTC_STACK_statistic_state_t s;
    CTC_STACK_statistic_data_t d;

    va_list va_ptr;

    short int ponid = GetPonPortIdxBySlot(slot, pon);
    short int onuid = onu-1, llid = INVALID_LLID;

	short int PonChipType = RERROR;
	short int PonPortIdx = RERROR;
    va_start(va_ptr, vcode);

    PonPortIdx = GetPonPortIdxBySlot ( slot, pon );
    PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
    if(ponid != -1 && ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(ponid, onuid)) ) )
    {
#if defined(_EPON_10G_PMC_SUPPORT_)            
        if(OLT_PONCHIP_ISPAS10G(PonChipType))
    	{
	        switch(vcode)
	        {
	        case 1:
	            rc = GW10GAdp_CTC_STACK_get_ethport_statistic_state(ponid, llid, port, &s);
	            break;
	        case 2:
	            arg = va_arg(va_ptr, int);
	            arg1 = va_arg(va_ptr, int);
	            s.status = arg;
	            s.circle = arg1;
	            /*rc = GW10GAdp_CTC_STACK_set_ethport_statistic_state(ponid, llid, port, s);*/
	            break;
	        case 3:
	           /* rc = GW10GAdp_CTC_STACK_get_ethport_statistic_data(ponid, llid, port, &d);*/
	            break;
	        case 4:
	            rc = GW10GAdp_CTC_STACK_set_ethport_statistic_data(ponid, llid, port);
	            break;
	        case 5:
	           /* rc = GW10GAdp_CTC_STACK_get_ethport_statistic_history_data(ponid, llid, port, &d);*/
	            break;
	        default:
	            break;
	        }
    	}
		else
#endif
        if(OLT_PONCHIP_ISPAS1G(PonChipType))
		{
	        switch(vcode)
	        {
	        case 1:
	            rc = CTC_STACK_get_ethport_statistic_state(ponid, llid, port, &s);
	            break;
	        case 2:
	            arg = va_arg(va_ptr, int);
	            arg1 = va_arg(va_ptr, int);
	            s.status = arg;
	            s.circle = arg1;
	            rc = CTC_STACK_set_ethport_statistic_state(ponid, llid, port, s);
	            break;
	        case 3:
	            rc = CTC_STACK_get_ethport_statistic_data(ponid, llid, port, &d);
	            break;
	        case 4:
	            rc = CTC_STACK_set_ethport_statistic_data(ponid, llid, port);
	            break;
	        case 5:
	            rc = CTC_STACK_get_ethport_statistic_history_data(ponid, llid, port, &d);
	            break;
	        default:
	            break;
	        }

		}
    }

    va_end(va_ptr);

    return rc;
}


/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int CTCONU_PAS5201_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_set_alarm_admin_state(olt_id, llid, management_object->index, alarm_id, enable);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetAlarmAdminState(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_set_alarm_threshold(olt_id, llid, management_object->index, alarm_id, alarm_threshold, clear_threshold);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetAlarmThreshold(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_get_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets,
                                   CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_set_dba_report_thresholds(olt_id, llid, number_of_queue_sets, queues_sets_thresholds);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetDbaReportThresholds(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;
}


static int CTCONU_PAS5201_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = CTC_STACK_get_mxu_mng_global_parameter_config(olt_id, llid, mxu_mng);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = CTC_STACK_set_mxu_mng_global_parameter_config(olt_id, llid, *mxu_mng);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetMxuMngGlobalConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_GetMxuMngSnmpConfig( short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_get_mxu_mng_snmp_parameter_config(olt_id, llid, parameter);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int CTCONU_PAS5201_SetMxuMngSnmpConfig(short int olt_id, short int onu_id,CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_set_mxu_mng_snmp_parameter_config(olt_id, llid, *parameter);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetMxuMngSnmpConfig(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


static int CTCONU_PAS5201_GetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_holdover_state(olt_id, llid, holdover);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetHoldover( short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_holdover_state(olt_id, llid, *holdover);
    }

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetHoldover(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

	OLT_ASSERT(olt_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_get_optical_transceiver_diagnosis(olt_id, llid, optical_transceiver_diagnosis);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetOptTransDiag(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
     
}

static int CTCONU_PAS5201_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, FALSE);
    }
    else
        iRlt = CTC_STACK_onu_tx_power_supply_control(olt_id, llid, *parameter, TRUE);

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t  *fec_ability)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	
	OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = CTC_STACK_get_fec_ability(olt_id, llid, fec_ability);
	}

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_GetFecAbility(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */


#if 1
int CTCONU_PAS5201_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t* iad_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_voip_iad_info(olt_id, llid, iad_info);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetIADInfo:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

/*H.248Э����IAD������״̬*/
int CTCONU_PAS5201_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_voip_iad_oper_status(olt_id, llid, iad_oper_status);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipIadOperStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

int CTCONU_PAS5201_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_voip_iad_operation(olt_id, llid, operation_type);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetVoipIadOperation:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

/*����ģ��ȫ�ֲ�������*/
int CTCONU_PAS5201_GetGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_voip_global_param_conf(olt_id, llid, global_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }

    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS5201_SetGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_voip_global_param_conf_t global_config;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        VOS_MemZero(&global_config, sizeof(CTC_STACK_voip_global_param_conf_t));
        iRlt = CTC_STACK_get_voip_global_param_conf(olt_id, llid, &global_config);
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

            iRlt = CTC_STACK_set_voip_global_param_conf(olt_id, llid, global_config);
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

    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetGlobalConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int testGlobalParameterConfig(short int slot, short int port, short int onuid, ULONG vid)
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
        iRlt = CTC_STACK_get_voip_global_param_conf(olt_id, llid, &ulglobal_param);
        ulglobal_param.cvlan_id = vid;
        iRlt = CTC_STACK_set_voip_global_param_conf(olt_id, llid, ulglobal_param);
        CTC_VOIP_DEBUG("\r\n%d = CTCONU_Set_GlobalParameterConfig:olt_id = %d, onu_id = %d, llid = %d, vid = %d\r\n", iRlt, olt_id, onu_id, llid, vid);    
    }
    return VOS_OK;
}

int CTCONU_PAS5201_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_voip_fax_config(olt_id, llid, voip_fax);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_STACK_voip_fax_config_t ulvoip_fax;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulvoip_fax, sizeof(CTC_STACK_voip_fax_config_t));
        iRlt = CTC_STACK_get_voip_fax_config(olt_id, llid, &ulvoip_fax);
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
            iRlt = CTC_STACK_set_voip_fax_config(olt_id, llid, ulvoip_fax);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetVoipFaxConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    
    return iRlt;           
}


int CTCONU_PAS5201_GetVoipPortStatus(short int olt_id, short int onu_id, int port, CTC_STACK_voip_pots_status_array *pots_status_array)
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
        iRlt = CTC_STACK_get_voip_pots_status(olt_id, llid, management_object_index, &(pots_status_array->number_of_entries), &(pots_status_array->pots_status_array));
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
#endif    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipPortStatus:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
    
}

int CTCONU_PAS5201_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_voip_port(olt_id, llid, port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;   
}

int CTCONU_PAS5201_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_voip_port(olt_id, llid, port_id, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetVoipPort:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state)
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
        
        iRlt = CTC_STACK_get_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;   
    
}

int CTCONU_PAS5201_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state)
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
        iRlt = CTC_STACK_set_voip_management_object(olt_id, llid, management_object_index, port_state);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetVoipPort2:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);
    return iRlt;           
    
}

int CTCONU_PAS5201_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_h248_param_config(olt_id, llid, h248_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS5201_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    CTC_STACK_h248_param_config_t h248_config;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&h248_config, sizeof(CTC_STACK_h248_param_config_t));
        iRlt = CTC_STACK_get_h248_param_config(olt_id, llid, &h248_config);
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
            iRlt = CTC_STACK_set_h248_param_config(olt_id, llid, h248_config);
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

    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetH248Config:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    

    return iRlt;           
}

int CTCONU_PAS5201_GetH248UserTidConfig(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
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
        iRlt = CTC_STACK_get_h248_user_tid_config(olt_id, llid, management_object_index, &(h248_user_tid_array->number_of_entries), &(h248_user_tid_array->h248_user_tid_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
#endif    
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetH248UserTidConfig(short int olt_id, short int onu_id, int port, CTC_STACK_h248_user_tid_config_t *user_tid_config)
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
        iRlt = CTC_STACK_set_h248_user_tid_config (olt_id, llid, management_object_index, *user_tid_config);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetH248UserTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_h248_rtp_tid_info(olt_id, llid, h248_rtp_tid_info);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)

{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_h248_rtp_tid_config(olt_id, llid, *h248_rtp_tid_info);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetH248RtpTidConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int CTCONU_PAS5201_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_sip_param_config(olt_id, llid, sip_param);
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    CTC_STACK_sip_param_config_t ulsip_param;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&ulsip_param, sizeof(CTC_STACK_sip_param_config_t));
        iRlt = CTC_STACK_get_sip_param_config(olt_id, llid, &ulsip_param);
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
            iRlt = CTC_STACK_set_sip_param_config(olt_id, llid, ulsip_param);
            if (iRlt != CTC_STACK_EXIT_OK)
            {
                iRlt = OLT_ERR_NOTSUPPORT;
            }
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetSipConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_sip_digit_map(olt_id, llid, *sip_digit_map);

        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }
    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetSipDigitMap:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}


int sip_slot_5201 = 63;
int sip_no_5201 = 24;

int CTCONU_PAS5201_GetSipUserConfig(short int olt_id, short int onu_id, int port, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
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
        management_object_index.slot_number = sip_slot_5201;        
        if(port)
            management_object_index.port_number = port;
        else
            management_object_index.port_number = 0xffff;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;
        sip_user_param_array->number_of_entries = sip_no_5201;
        iRlt = CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &(sip_user_param_array->number_of_entries), &(sip_user_param_array->sip_user_param_array));        
        if (iRlt != CTC_STACK_EXIT_OK)
        {
            iRlt = OLT_ERR_NOTSUPPORT;
        }

    }
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_GetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
    return iRlt;           
}

int CTCONU_PAS5201_SetSipUserConfig(short int olt_id, short int onu_id, int port, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
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
        management_object_index.slot_number = sip_slot_5201;        
        management_object_index.port_number = port;
        management_object_index.port_type = CTC_MANAGEMENT_OBJECT_PORT_TYPE_VOIP_PORT;       
        iRlt = CTC_STACK_get_sip_user_param_config(olt_id, llid, management_object_index, &number_of_entries, &sip_user_param_array);                
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
    CTC_VOIP_DEBUG("%d = CTCONU_PAS5201_SetSipUserConfig:olt_id = %d, onu_id = %d, llid = %d\r\n", iRlt, olt_id, onu_id, llid);    
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
		iRlt = CTC_STACK_set_auth_mode(olt_id, ctc_auth_mode);

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
             iRlt=CTC_STACK_set_alarm_admin_state(olt_id, onu_id,management_object,EQUIPMENT_ALARM,1 );

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
/* -------------------ONU Զ�̹���API------------------- */

static int CTCONU_PAS5201_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;
    int i = 0;
    short int llid = 0;
    unsigned char portNum = 0;
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
			iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetVlanEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return lReturnValue;
}

static int CTCONU_PAS5201_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;    
    short int llid = 0;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    mode &= (~ONU_SEPCAL_FUNCTION);
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc);
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

                iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
                if(iRlt != VOS_OK)
                {
                    lReturnValue = iRlt;                
                    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, lport, mode, __LINE__);         
                }
            }
        }
        else
        {
            lReturnValue = iRlt;        
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, lport, mode, __LINE__);			
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;        
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, lport, mode, __LINE__);            
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_PAS5201_DelVlan(short int olt_id, short int onu_id, int vid)
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
            if ( 0 == (iRlt = CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
            {
                for ( i=0; i<portnum; i++ )
                {
                    num = Port_Info[i].entry.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
                    {
                    	if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TRUNK)
                		{
	                        for ( j=0; j<num; j++ )
	                        {
	                            if ( (Port_Info[i].entry.vlan_list[j] & 0xfff) == vid )
	                            {
	                                linux_memmove(Port_Info[i].entry.vlan_list+j, Port_Info[i].entry.vlan_list+j+1, sizeof(long)*(num-1-j));
	                                Port_Info[i].entry.number_of_entries--;
	                            }
	                        }
	                        
	                        /*�˿�Ĭ��vlan�ָ�Ϊ 1 */
	                        if ( untag & (1<<i/*(Port_Info[i].port_number-1)*/) )
	                        {
	                            Port_Info[i].entry.default_vlan = (0x8100<<16)|1;
	                        }
                		}
						else if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TAG)
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
						else
						{
							continue;
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
                    iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, Port_Info[i].entry);
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
                    iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc);
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
                            /*�˿�Ĭ��vlan�ָ�Ϊ 1 */
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
                        iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_DelVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_PAS5201_SetPortPvid(short int olt_id, short int onu_id, int port_id, int lv)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int lReturnValue = VOS_OK;    
    short int llid = 0;
    uchar lport = port_id;
    CTC_STACK_port_vlan_configuration_t pvc;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc);
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

                iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
                if(iRlt != VOS_OK)
                {
                    lReturnValue = iRlt;                                    
                    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, lport, lv, __LINE__);         
                }
            }
            else
            {
                iRlt = OLT_ERR_OK;
            }
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                                    
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, lport, lv, __LINE__);             
    }
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortPvid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_PAS5201_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
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
    
        if ( 0 == (iRlt = CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
        {
            for ( i=0; i<portnum; i++ )
            {
                if(portlist&(1<<i))
                {
                    num = Port_Info[i].entry.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
                    {
                    	if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TRUNK)
                		{

		                    /*untag port member added by wangxiaoyu 2011-08-10, auto change port pvid to vid*/
	                        if(tag == 2)
	                        {
                            /* pvc.default_vlan &= 0xfffff000; pvc.default_vlan |= vid;*/
	                            Port_Info[i].entry.default_vlan = (0x8100<<16)|vid;

	                            /*����Ѽ�Ϊtag�ģ���tag��ɾ��*/
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
	                                                   * ��tag��ʽ����VLANʱ��ȥ��ԭ����pvidֵ����λΪvlan 1
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
                    	else if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TAG)
						{
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
						else
						{
                            return OLT_ERR_OK;
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
                    iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, Port_Info[i].entry);
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
				iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc);
                if(iRlt == VOS_OK)
				{
                    int num = pvc.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
                    {
                        if(tag == 2)
                        {
                            int iv = 0;
                            /* pvc.default_vlan &= 0xfffff000; pvc.default_vlan |= vid;*/
                            pvc.default_vlan = (0x8100<<16)|vid;
                            /*����Ѽ�Ϊtag�ģ���tag��ɾ��*/
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
					iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_AddVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int CTCONU_PAS5201_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
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
    
        if ( 0 == (iRlt = CTC_STACK_get_vlan_all_port_configuration(olt_id, llid, &portNum, Port_Info)) )
        {
            for ( i=0; i<portnum; i++ )
            {
                if(portlist&(1<<i))
                {
                    num = Port_Info[i].entry.number_of_entries;
                    if(version >= CTC_2_1_ONU_VERSION)
                    {
                    	if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TRUNK)
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
						else if(Port_Info[i].entry.mode == CTC_VLAN_MODE_TAG)
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
						else
						{
							continue;
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
                    iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, Port_Info[i].entry);
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
                iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, i+1, &pvc);
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
                    iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, i+1, pvc);
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
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_DelVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return lReturnValue;
}
void test_CTCONU_PAS5201_DelVlanPort(short int slot ,short int port, short int onu, short int vid)
{
    short int olt_id = GetPonPortIdxBySlot(slot, port);
    short int onu_id = onu-1;
    int deviceIndex = MAKEDEVID(GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id+1);
    short int iRlt = -1;
    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, 1, vid, __LINE__);         
}
/*vlan transation*/
static int CTCONU_PAS5201_SetVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
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
        if ( 0 == (iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_DelVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
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
        if ( 0 == (iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_DelVlanTran(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*vlan aggregation*/
static int CTCONU_PAS5201_SetVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
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
        if ( 0 == (iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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

            iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}

static int CTCONU_PAS5201_DelVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
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
        if ( 0 == (iRlt = CTC_STACK_get_vlan_port_configuration(olt_id, llid, lport, &pvc)) )
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
            if(pvc.number_of_aggregation_tables)/*modi 2013-3-21 ��ֹԽ�磬����pon�����*/
                pvc.number_of_aggregation_tables--;
#endif
            iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, onu_id, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_DelVlanAgg(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*QinQ enable*/
static int CTCONU_PAS5201_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
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
            
            iRlt = CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config );
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortQinQEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return  iRlt;
}

/*qinq port vlan tag add*/
static int CTCONU_PAS5201_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
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

                iRlt = CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config);
            }
            else
                iRlt = OLT_ERR_NORESC;
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_AddQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port vlan tag del*/
static int CTCONU_PAS5201_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
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
                if (qinq_config.vlan_list[2*i] == svlan)
                {
                    n = qinq_config.number_of_entries;
                    linux_memmove(qinq_config.vlan_list+2*i, qinq_config.vlan_list+2*i+2, (n-i-1)*2*sizeof(LONG));
                    qinq_config.number_of_entries--;
                    break;
                }

                iRlt = CTC_STACK_set_qinq_port_configuration(olt_id, llid, lport, qinq_config);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_DelQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return  iRlt;
}


static int CTCONU_PAS5201_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        bool state = mode ? FALSE:TRUE;
        
        if ( 0 == (iRlt = CTC_STACK_set_auto_negotiation_admin_control(olt_id, llid, lport, state)) )
        {
            if ( state )
            {
                CTC_STACK_set_auto_negotiation_restart_auto_config(olt_id, llid, lport);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_ethernet_port_pause(olt_id, llid, (unsigned char)port_id, (bool)en);    
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortFcMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
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
        ULONG ebs = 1522; /*CTC�淶Ҫ��ebsҪС�ڵ�����̫��֡����ֽ���*/
#endif

        /*added by luh @2015-1-15 Э���ж���Ϊ3���ֽڣ��������3���ֽ���ȡ���ֵ*/                
        if(cbs & 0xFF000000)
        {
            cbs = 0xFFFFFF;
        }

        ppe.bucket_depth = cbs;
        ppe.cir = cir;
        ppe.extra_burst_size = ebs;

        ppe.operation = cir ? CTC_STACK_STATE_ACTIVATE : CTC_STACK_STATE_DEACTIVATE;

        iRlt = CTC_STACK_set_ethernet_port_policing(olt_id, llid, lport, ppe);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortIngressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
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

        iRlt = CTC_STACK_set_ethernet_port_ds_rate_limiting(olt_id, llid, lport, &ppe);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortEgressRate(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;

		if ( 0 == qossetid ) /* ɾ��pon�˿��ϵ�qos����*/
		{
			iRlt = CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);
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

                CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);

                iRlt = CTC_STACK_set_classification_and_marking(olt_id, llid, lport, CTC_CLASSIFICATION_ADD_RULE, qset);
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;

        iRlt = CTC_STACK_delete_classification_and_marking_list(olt_id, llid, lport);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_ClrPortQosRule(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_SetPortDefaultPriority(short int olt_id, short int onu_id, int port_id, int priority)
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
             
            iRlt = CTC_STACK_set_vlan_port_configuration(olt_id, llid, lport, pvc);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortDefaultPriority(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int CTCONU_PAS5201_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_multicast_switch(olt_id, llid, en ? CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING : CTC_STACK_PROTOCOL_CTC);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetIgmpEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = CTC_STACK_set_multicast_switch(olt_id, llid, en);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetIgmpAuth(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_PAS5201_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
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
            iRlt = CTC_STACK_clear_multicast_vlan(olt_id, onu_id, lport);
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
                    
                   CTC_STACK_clear_multicast_vlan(olt_id, onu_id, lport);

                   mv.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
                   iRlt = CTC_STACK_set_multicast_vlan(olt_id, onu_id, lport, mv);
                }
            }
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortMulticastVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

#endif

/* --------------------END------------------------ */


/******************************************�ⲿ�ӿ�***************************************/
static  const OnuMgmtIFs  s_gwonu5001Ifs = {
#if 1
/* -------------------ONU����API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS5001_OnuIsOnline,
    GWONU_PAS5001_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/
    GWONU_PAS5001_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_PAS5001_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU ��֤����API------------------- */
    GWONU_PAS5001_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS5001_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU ҵ�����API------------------- */
    GWONU_PAS5001_SetOnuTrafficServiceMode,
    GWONU_PAS5001_SetOnuPeerToPeer,
    GWONU_PAS5001_SetOnuPeerToPeerForward,
    GWONU_PAS5001_SetOnuBW,
    GWONU_PAS5001_GetOnuSLA,

    NULL,           /* SetOnuFecMode */
    NULL,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_SetUniPort,
    GWONU_PAS5001_SetSlowProtocolLimit,
    GWONU_PAS5001_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU ���ͳ�ƹ���API--------------- */
    GWONU_PAS5001_ResetCounters,
    NULL,         /* SetPonLoopback */
#endif

#if 1
/* -------------------ONU���ܹ���API-------------------- */
    GWONU_PAS5001_GetLLIDParams,
    GWONU_PAS5001_StartEncryption,
    GWONU_PAS5001_StopEncryption,
    GWONU_PAS5001_SetOnuEncryptParams,
    GWONU_PAS5001_GetOnuEncryptParams,
    
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU ��ַ����API------------------- */
    GWONU_PAS5001_GetOnuMacAddrTbl,
    GWONU_PAS5001_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS5001_SetOnuMaxMacNum,
    GWONU_PAS5001_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU ��·����API------------------- */
    GWONU_PAS5001_GetOnuDistance,
    GWONU_PAS5001_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU ����API---------------- */
    GWONU_PAS5001_SetOnuLLID,
#endif

#if 1
/* -------------------ONU �豸����API------------------- */
    GWONU_PAS5001_GetOnuVer,
    GWONU_PAS5001_GetOnuPonVer,
    GWONU_PAS5001_GetOnuRegisterInfo,
    GWONU_PAS5001_GetOnuI2CInfo,
    GWONU_PAS5001_SetOnuI2CInfo,
    
    GWONU_PAS5001_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_GetBurnImageComplete,
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
/* -------------------ONU Զ�̹���API------------------- */
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
/* --------------------ONU CMCЭ�����API------------------- */
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
    NULL,           /* GetCmcDownChannelMode */
    NULL,           /* SetCmcDownChannelMode */
    NULL,           /* GetCmcUpChannelMode */
    
    NULL,           /* SetCmcUpChannelMode */
    NULL,           /* GetCmcUpChannelD30Mode */
    NULL,           /* SetCmcUpChannelD30Mode */
    NULL,           /* GetCmcDownChannelFreq */
    NULL,           /* SetCmcDownChannelFreq */

    NULL,           /* GetCmcUpChannelFreq */
    NULL,           /* SetCmcUpChannelFreq */
    NULL,           /* GetCmcDownAutoFreq */
    NULL,           /* SetCmcDownAutoFreq */
    NULL,           /* GetCmcUpAutoFreq */
    
    NULL,           /* SetCmcUpAutoFreq */
    NULL,           /* GetCmcUpChannelWidth */
    NULL,           /* SetCmcUpChannelWidth */
    NULL,           /* GetCmcDownChannelAnnexMode */
    NULL,           /* SetCmcDownChannelAnnexMode */
    
    NULL,           /* GetCmcUpChannelType */
    NULL,           /* SetCmcUpChannelType */
    NULL,           /* GetCmcDownChannelModulation */
    NULL,           /* SetCmcDownChannelModulation */
    NULL,           /* GetCmcUpChannelProfile */
    
    NULL,           /* SetCmcUpChannelProfile */
    NULL,           /* GetCmcDownChannelInterleaver */
    NULL,           /* SetCmcDownChannelInterleaver */
    NULL,           /* GetCmcDownChannelPower */
    NULL,           /* SetCmcDownChannelPower */

    NULL,           /* GetCmcUpChannelPower */
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

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSISӦ�ù���API------------------- */
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

    REMOTE_ERROR,
};

static  const OnuMgmtIFs  s_gwonu5201Ifs = {
#if 1
/* -------------------ONU����API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS5001_OnuIsOnline,
    GWONU_PAS5001_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/
    GWONU_PAS5001_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_PAS5201_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU ��֤����API------------------- */
    GWONU_PAS5001_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS5001_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU ҵ�����API------------------- */
    GWONU_PAS5001_SetOnuTrafficServiceMode,
    GWONU_PAS5001_SetOnuPeerToPeer,
    GWONU_PAS5001_SetOnuPeerToPeerForward,
    GWONU_PAS5201_SetOnuBW,
    GWONU_PAS5001_GetOnuSLA,

    GWONU_PAS5201_SetOnuFecMode,
    GWONU_PAS5201_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_SetUniPort,
    GWONU_PAS5001_SetSlowProtocolLimit,
    GWONU_PAS5001_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU ���ͳ�ƹ���API--------------- */
    GWONU_PAS5001_ResetCounters,
    GWONU_PAS5201_SetPonLoopback,
#endif

#if 1
/* -------------------ONU���ܹ���API------------------- */
    GWONU_PAS5001_GetLLIDParams,
    GWONU_PAS5001_StartEncryption,
    GWONU_PAS5001_StopEncryption,
    GWONU_PAS5001_SetOnuEncryptParams,
    GWONU_PAS5001_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU ��ַ����API------------------- */
    GWONU_PAS5001_GetOnuMacAddrTbl,
    GWONU_PAS5001_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS5201_SetOnuMaxMacNum,
    GWONU_PAS5001_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU ��·����API------------------- */
    GWONU_PAS5001_GetOnuDistance,
    GWONU_PAS5001_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU ����API---------------- */
    GWONU_PAS5001_SetOnuLLID,
#endif

#if 1
/* -------------------ONU �豸����API------------------- */
    GWONU_PAS5001_GetOnuVer,
    GWONU_PAS5001_GetOnuPonVer,
    GWONU_PAS5001_GetOnuRegisterInfo,
    GWONU_PAS5001_GetOnuI2CInfo,
    GWONU_PAS5001_SetOnuI2CInfo,
    
    GWONU_PAS5001_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_GetBurnImageComplete,
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
/* -------------------ONU Զ�̹���API------------------- */
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
/* --------------------ONU CMCЭ�����API------------------- */
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
    NULL,           /* GetCmcDownChannelMode */
    NULL,           /* SetCmcDownChannelMode */
    NULL,           /* GetCmcUpChannelMode */
    
    NULL,           /* SetCmcUpChannelMode */
    NULL,           /* GetCmcUpChannelD30Mode */
    NULL,           /* SetCmcUpChannelD30Mode */
    NULL,           /* GetCmcDownChannelFreq */
    NULL,           /* SetCmcDownChannelFreq */

    NULL,           /* GetCmcUpChannelFreq */
    NULL,           /* SetCmcUpChannelFreq */
    NULL,           /* GetCmcDownAutoFreq */
    NULL,           /* SetCmcDownAutoFreq */
    NULL,           /* GetCmcUpAutoFreq */
    
    NULL,           /* SetCmcUpAutoFreq */
    NULL,           /* GetCmcUpChannelWidth */
    NULL,           /* SetCmcUpChannelWidth */
    NULL,           /* GetCmcDownChannelAnnexMode */
    NULL,           /* SetCmcDownChannelAnnexMode */
    
    NULL,           /* GetCmcUpChannelType */
    NULL,           /* SetCmcUpChannelType */
    NULL,           /* GetCmcDownChannelModulation */
    NULL,           /* SetCmcDownChannelModulation */
    NULL,           /* GetCmcUpChannelProfile */
    
    NULL,           /* SetCmcUpChannelProfile */
    NULL,           /* GetCmcDownChannelInterleaver */
    NULL,           /* SetCmcDownChannelInterleaver */
    NULL,           /* GetCmcDownChannelPower */
    NULL,           /* SetCmcDownChannelPower */

    NULL,           /* GetCmcUpChannelPower */
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

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSISӦ�ù���API------------------- */
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

    REMOTE_ERROR,
};

static const OnuMgmtIFs s_gwonu5204Ifs = {
#if 1
/* -------------------ONU����API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS5001_OnuIsOnline,
    GWONU_PAS5001_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_PAS5001_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	GWONU_PAS5204_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU ��֤����API------------------- */
    GWONU_PAS5001_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS5001_AuthorizeOnu,
    NULL,           /* AuthRequest */
    NULL,           /* AuthSucess */
    NULL,           /* AuthFail */    
#endif

#if 1
/* -------------------ONU ҵ�����API------------------- */
    GWONU_PAS5001_SetOnuTrafficServiceMode,
    GWONU_PAS5001_SetOnuPeerToPeer,
    GWONU_PAS5001_SetOnuPeerToPeerForward,
    GWONU_PAS5201_SetOnuBW,
    GWONU_PAS5001_GetOnuSLA,

    GWONU_PAS5201_SetOnuFecMode,
    GWONU_PAS5201_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_SetUniPort,
    GWONU_PAS5001_SetSlowProtocolLimit,
    GWONU_PAS5001_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU ���ͳ�ƹ���API--------------- */
    GWONU_PAS5001_ResetCounters,
    GWONU_PAS5201_SetPonLoopback,
#endif

#if 1
/* -------------------ONU���ܹ���API------------------- */
    GWONU_PAS5001_GetLLIDParams,
    GWONU_PAS5001_StartEncryption,
    GWONU_PAS5001_StopEncryption,
    GWONU_PAS5001_SetOnuEncryptParams,
    GWONU_PAS5001_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU ��ַ����API------------------- */
    GWONU_PAS5001_GetOnuMacAddrTbl,
    GWONU_PAS5001_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS5201_SetOnuMaxMacNum,
    GWONU_PAS5001_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU ��·����API------------------- */
    GWONU_PAS5001_GetOnuDistance,
    GWONU_PAS5001_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU ����API---------------- */
    GWONU_PAS5001_SetOnuLLID,
#endif

#if 1
/* -------------------ONU �豸����API------------------- */
    GWONU_PAS5001_GetOnuVer,
    GWONU_PAS5001_GetOnuPonVer,
    GWONU_PAS5001_GetOnuRegisterInfo,
    GWONU_PAS5001_GetOnuI2CInfo,
    GWONU_PAS5001_SetOnuI2CInfo,
    
    GWONU_PAS5001_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GWONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_GetBurnImageComplete,
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
/* -------------------ONU Զ�̹���API------------------- */
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
/* --------------------ONU CMCЭ�����API------------------- */
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

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSISӦ�ù���API------------------- */
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

    REMOTE_ERROR,
};

static const OnuMgmtIFs s_ctconu5201Ifs = {
#if 1
/* -------------------ONU����API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS5001_OnuIsOnline,
    GWONU_PAS5001_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_PAS5001_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_PAS5201_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU ��֤����API------------------- */
    GWONU_PAS5001_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS5001_AuthorizeOnu,
    CTCONU_PAS5201_AuthRequest,
    CTCONU_PAS5201_AuthSuccess,
    CTCONU_PAS5201_AuthFailure,        
#endif

#if 1
/* -------------------ONU ҵ�����API------------------- */
    CTCONU_PAS5201_SetOnuTrafficServiceMode,
    GWONU_PAS5001_SetOnuPeerToPeer,
    GWONU_PAS5001_SetOnuPeerToPeerForward,
    GWONU_PAS5201_SetOnuBW,
    GWONU_PAS5001_GetOnuSLA,

    CTCONU_PAS5201_SetOnuFecMode,
    GWONU_PAS5201_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_SetUniPort,
    GWONU_PAS5001_SetSlowProtocolLimit,
    GWONU_PAS5001_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU ���ͳ�ƹ���API--------------- */
    GWONU_PAS5001_ResetCounters,
    GWONU_PAS5201_SetPonLoopback,
#endif

#if 1
/* -------------------ONU���ܹ���API------------------- */
    GWONU_PAS5001_GetLLIDParams,
    GWONU_PAS5001_StartEncryption,
    GWONU_PAS5001_StopEncryption,
    GWONU_PAS5001_SetOnuEncryptParams,
    GWONU_PAS5001_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU ��ַ����API------------------- */
    GWONU_PAS5001_GetOnuMacAddrTbl,
    GWONU_PAS5001_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS5201_SetOnuMaxMacNum,
    GWONU_PAS5001_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU ��·����API------------------- */
    GWONU_PAS5001_GetOnuDistance,
    GWONU_PAS5001_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU ����API---------------- */
    GWONU_PAS5001_SetOnuLLID,
#endif

#if 1
/* -------------------ONU �豸����API------------------- */
    GWONU_PAS5001_GetOnuVer,
    GWONU_PAS5001_GetOnuPonVer,
    GWONU_PAS5001_GetOnuRegisterInfo,
    GWONU_PAS5001_GetOnuI2CInfo,
    GWONU_PAS5001_SetOnuI2CInfo,
    
    CTCONU_PAS5201_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    /*CTCONU_PAS5201_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_PAS5201_GetCtcVersion,
    CTCONU_PAS5201_GetFirmwareVersion,
    CTCONU_PAS5201_GetSerialNumber,
    CTCONU_PAS5201_GetChipsetID,

    CTCONU_PAS5201_GetOnuCap1,
    CTCONU_PAS5201_GetOnuCap2,
    CTCONU_PAS5201_GetOnuCap3,
    
    CTCONU_PAS5201_UpdateOnuFirmware,
    CTCONU_PAS5201_ActiveOnuFirmware,
    CTCONU_PAS5201_CommitOnuFirmware,
    
    CTCONU_PAS5201_StartEncrypt,
    CTCONU_PAS5201_StopEncrypt,
    
    CTCONU_PAS5201_GetEthPortLinkstate,
    CTCONU_PAS5201_GetEthPortAdminstatus,
    CTCONU_PAS5201_SetEthPortAdminstatus,
    
    CTCONU_PAS5201_GetEthPortPauseEnable,
    CTCONU_PAS5201_SetEthPortPauseEnable,
    
    CTCONU_PAS5201_GetEthPortAutoNegotinationAdmin,
    CTCONU_PAS5201_SetEthPortAutoNegotinationAdmin,
    CTCONU_PAS5201_SetEthPortRestartAutoConfig,
    CTCONU_PAS5201_GetEthPortAnLocalTecAbility,
    CTCONU_PAS5201_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_PAS5201_GetEthPortPolicing,
    CTCONU_PAS5201_SetEthPortPolicing,
    
    CTCONU_PAS5201_GetEthPortDownstreamPolicing,
    CTCONU_PAS5201_SetEthPortDownstreamPolicing,

    CTCONU_PAS5201_GetEthPortVlanConfig,
    CTCONU_PAS5201_SetEthPortVlanConfig,
    CTCONU_PAS5201_GetAllPortVlanConfig,
    
    CTCONU_PAS5201_GetEthPortClassificationAndMarking,
    CTCONU_PAS5201_SetEthPortClassificationAndMarking,
    CTCONU_PAS5201_ClearEthPortClassificationAndMarking,
    
    CTCONU_PAS5201_GetEthPortMulticastVlan,
    CTCONU_PAS5201_SetEthPortMulticastVlan,
    CTCONU_PAS5201_ClearEthPortMulticastVlan,

    CTCONU_PAS5201_GetEthPortMulticastGroupMaxNumber,
    CTCONU_PAS5201_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_PAS5201_GetEthPortMulticastTagStrip,
    CTCONU_PAS5201_SetEthPortMulticastTagStrip,
    CTCONU_PAS5201_GetMulticastAllPortTagStrip,
    
    CTCONU_PAS5201_GetEthPortMulticastTagOper,
    CTCONU_PAS5201_SetEthPortMulticastTagOper,
    CTCONU_PAS5201_SetObjMulticastTagOper,
    
    CTCONU_PAS5201_GetMulticastControl,
    CTCONU_PAS5201_SetMulticastControl,
    CTCONU_PAS5201_ClearMulticastControl,
    
    CTCONU_PAS5201_GetMulticastSwitch,
    CTCONU_PAS5201_SetMulticastSwitch,
    
    CTCONU_PAS5201_GetFastLeaveAbility,
    CTCONU_PAS5201_GetFastLeaveAdminState,
    CTCONU_PAS5201_SetFastLeaveAdminState,

    CTCONU_PAS5201_GetPortStatisticData,
    CTCONU_PAS5201_GetPortStatisticState,
    CTCONU_PAS5201_SetPortStatisticState,

    CTCONU_PAS5201_SetAlarmAdminState,
    CTCONU_PAS5201_SetAlarmThreshold,
    CTCONU_PAS5201_GetDbaReportThresholds,
    CTCONU_PAS5201_SetDbaReportThresholds,
    
    CTCONU_PAS5201_GetMxuMngGlobalConfig,
    CTCONU_PAS5201_SetMxuMngGlobalConfig,
    CTCONU_PAS5201_GetMxuMngSnmpConfig,
    CTCONU_PAS5201_SetMxuMngSnmpConfig,
    
    CTCONU_PAS5201_GetHoldover,
    CTCONU_PAS5201_SetHoldover,
    CTCONU_PAS5201_GetOptTransDiag,
    CTCONU_PAS5201_SetTxPowerSupplyControl,
    
    CTCONU_PAS5201_GetFecAbility,

    CTCONU_PAS5201_GetIADInfo,
    CTCONU_PAS5201_GetVoipIadOperStatus,
    CTCONU_PAS5201_SetVoipIadOperation,
    CTCONU_PAS5201_GetGlobalConfig,
    CTCONU_PAS5201_SetGlobalConfig,
    CTCONU_PAS5201_GetVoipFaxConfig,
    CTCONU_PAS5201_SetVoipFaxConfig,

    CTCONU_PAS5201_GetVoipPortStatus,
    CTCONU_PAS5201_GetVoipPort,
    CTCONU_PAS5201_SetVoipPort,
    CTCONU_PAS5201_GetVoipPort2,
    CTCONU_PAS5201_SetVoipPort2,

    CTCONU_PAS5201_GetH248Config,
    CTCONU_PAS5201_SetH248Config,
    CTCONU_PAS5201_GetH248UserTidConfig,
    CTCONU_PAS5201_SetH248UserTidConfig,
    CTCONU_PAS5201_GetH248RtpTidConfig,
    CTCONU_PAS5201_SetH248RtpTidConfig,
    
    CTCONU_PAS5201_GetSipConfig,
    CTCONU_PAS5201_SetSipConfig,
    CTCONU_PAS5201_SetSipDigitMap,
    CTCONU_PAS5201_GetSipUserConfig,
    CTCONU_PAS5201_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID,
#endif

#if 1
/* -------------------ONU Զ�̹���API------------------- */
    NULL,           /* CliCall */

    CTCONU_PAS5201_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    GWONU_SetPasFlush,
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_PAS5201_SetVlanEnable,
    CTCONU_PAS5201_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_PAS5201_DelVlan,
    CTCONU_PAS5201_SetPortPvid,
    
    CTCONU_PAS5201_AddVlanPort,
    CTCONU_PAS5201_DelVlanPort,
    CTCONU_PAS5201_SetVlanTran,
    CTCONU_PAS5201_DelVlanTran,
    CTCONU_PAS5201_SetVlanAgg,
    CTCONU_PAS5201_DelVlanAgg,
    
    CTCONU_PAS5201_SetPortQinQEnable,
    CTCONU_PAS5201_AddQinQVlanTag,
    CTCONU_PAS5201_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_PAS5201_SetPortMode,
    CTCONU_PAS5201_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_PAS5201_SetPortIngressRate,
    CTCONU_PAS5201_SetPortEgressRate,
    
    REMOTE_OK/*GWONU_SetQosClass*/,
    REMOTE_OK/*GWONU_ClrQosClass*/,
    REMOTE_OK/*GWONU_SetQosRule*/,
    REMOTE_OK/*GWONU_ClrQosRule*/,
    
    CTCONU_PAS5201_SetPortQosRule,
    CTCONU_PAS5201_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_PAS5201_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_PAS5201_SetIgmpEnable,
    CTCONU_PAS5201_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_PAS5201_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMCЭ�����API------------------- */
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

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSISӦ�ù���API------------------- */
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

    REMOTE_ERROR,
};

static const OnuMgmtIFs s_ctconu5204Ifs = {
#if 1
/* -------------------ONU����API------------------- */
    GWONU_OnuIsValid,
    GWONU_PAS5001_OnuIsOnline,
    GWONU_PAS5001_AddOnuByManual,
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GWONU_PAS5001_DelOnuByManual,
    REMOTE_OK,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	CTCONU_PAS5204_GetIFType,
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU ��֤����API------------------- */
    GWONU_PAS5001_DeregisterOnu,
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GWONU_PAS5001_AuthorizeOnu,
    CTCONU_PAS5201_AuthRequest,
    CTCONU_PAS5201_AuthSuccess,
    CTCONU_PAS5201_AuthFailure,        
#endif

#if 1
/* -------------------ONU ҵ�����API------------------- */
    CTCONU_PAS5201_SetOnuTrafficServiceMode,
    GWONU_PAS5001_SetOnuPeerToPeer,
    GWONU_PAS5001_SetOnuPeerToPeerForward,
    GWONU_PAS5201_SetOnuBW,
    GWONU_PAS5001_GetOnuSLA,

    CTCONU_PAS5201_SetOnuFecMode,
    GWONU_PAS5201_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_SetUniPort,
    GWONU_PAS5001_SetSlowProtocolLimit,
    GWONU_PAS5001_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	GWONU_GetBWInfo,
    NULL,           /* GetOnuB2PMode */
    NULL,           /* SetOnuB2PMode */
#endif

#if 1
/* -------------------ONU ���ͳ�ƹ���API--------------- */
    GWONU_PAS5001_ResetCounters,
    GWONU_PAS5201_SetPonLoopback,
#endif

#if 1
/* -------------------ONU���ܹ���API------------------- */
    GWONU_PAS5001_GetLLIDParams,
    GWONU_PAS5001_StartEncryption,
    GWONU_PAS5001_StopEncryption,
    GWONU_PAS5001_SetOnuEncryptParams,
    GWONU_PAS5001_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU ��ַ����API------------------- */
    GWONU_PAS5001_GetOnuMacAddrTbl,
    GWONU_PAS5001_GetOltMacAddrTbl,
    NULL,/*GetOltMacAddrVlanTbl*/
    GWONU_PAS5201_SetOnuMaxMacNum,
    GWONU_PAS5001_GetOnuUniMacCfg,
    GWONU_GetOnuMacCheckFlag,
    GWONU_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU ��·����API------------------- */
    GWONU_PAS5001_GetOnuDistance,
    GWONU_PAS5001_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU ����API---------------- */
    GWONU_PAS5001_SetOnuLLID,
#endif

#if 1
/* -------------------ONU �豸����API------------------- */
    GWONU_PAS5001_GetOnuVer,
    GWONU_PAS5001_GetOnuPonVer,
    GWONU_PAS5001_GetOnuRegisterInfo,
    GWONU_PAS5001_GetOnuI2CInfo,
    GWONU_PAS5001_SetOnuI2CInfo,
    
    CTCONU_PAS5201_ResetOnu,
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    CTCONU_OnuSwUpdate,
    GWONU_OnuGwCtcSwConvert,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    GWONU_PAS5001_GetBurnImageComplete,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    /*CTCONU_PAS5201_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    GWONU_SetOnuDeviceName,
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    GWONU_GetOnuAllPortStatisticData,
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    CTCONU_PAS5201_GetCtcVersion,
    CTCONU_PAS5201_GetFirmwareVersion,
    CTCONU_PAS5201_GetSerialNumber,
    CTCONU_PAS5201_GetChipsetID,

    CTCONU_PAS5201_GetOnuCap1,
    CTCONU_PAS5201_GetOnuCap2,
    CTCONU_PAS5201_GetOnuCap3,
    
    CTCONU_PAS5201_UpdateOnuFirmware,
    CTCONU_PAS5201_ActiveOnuFirmware,
    CTCONU_PAS5201_CommitOnuFirmware,
    
    CTCONU_PAS5201_StartEncrypt,
    CTCONU_PAS5201_StopEncrypt,
    
    CTCONU_PAS5201_GetEthPortLinkstate,
    CTCONU_PAS5201_GetEthPortAdminstatus,
    CTCONU_PAS5201_SetEthPortAdminstatus,
    
    CTCONU_PAS5201_GetEthPortPauseEnable,
    CTCONU_PAS5201_SetEthPortPauseEnable,
    
    CTCONU_PAS5201_GetEthPortAutoNegotinationAdmin,
    CTCONU_PAS5201_SetEthPortAutoNegotinationAdmin,
    CTCONU_PAS5201_SetEthPortRestartAutoConfig,
    CTCONU_PAS5201_GetEthPortAnLocalTecAbility,
    CTCONU_PAS5201_GetEthPortAnAdvertisedTecAbility,
    
    CTCONU_PAS5201_GetEthPortPolicing,
    CTCONU_PAS5201_SetEthPortPolicing,
    
    CTCONU_PAS5201_GetEthPortDownstreamPolicing,
    CTCONU_PAS5201_SetEthPortDownstreamPolicing,

    CTCONU_PAS5201_GetEthPortVlanConfig,
    CTCONU_PAS5201_SetEthPortVlanConfig,
    CTCONU_PAS5201_GetAllPortVlanConfig,
    
    CTCONU_PAS5201_GetEthPortClassificationAndMarking,
    CTCONU_PAS5201_SetEthPortClassificationAndMarking,
    CTCONU_PAS5201_ClearEthPortClassificationAndMarking,
    
    CTCONU_PAS5201_GetEthPortMulticastVlan,
    CTCONU_PAS5201_SetEthPortMulticastVlan,
    CTCONU_PAS5201_ClearEthPortMulticastVlan,

    CTCONU_PAS5201_GetEthPortMulticastGroupMaxNumber,
    CTCONU_PAS5201_SetEthPortMulticastGroupMaxNumber,
    
    CTCONU_PAS5201_GetEthPortMulticastTagStrip,
    CTCONU_PAS5201_SetEthPortMulticastTagStrip,
    CTCONU_PAS5201_GetMulticastAllPortTagStrip,
    
    CTCONU_PAS5201_GetEthPortMulticastTagOper,
    CTCONU_PAS5201_SetEthPortMulticastTagOper,
    CTCONU_PAS5201_SetObjMulticastTagOper,
    
    CTCONU_PAS5201_GetMulticastControl,
    CTCONU_PAS5201_SetMulticastControl,
    CTCONU_PAS5201_ClearMulticastControl,
    
    CTCONU_PAS5201_GetMulticastSwitch,
    CTCONU_PAS5201_SetMulticastSwitch,
    
    CTCONU_PAS5201_GetFastLeaveAbility,
    CTCONU_PAS5201_GetFastLeaveAdminState,
    CTCONU_PAS5201_SetFastLeaveAdminState,

    CTCONU_PAS5201_GetPortStatisticData,
    CTCONU_PAS5201_GetPortStatisticState,
    CTCONU_PAS5201_SetPortStatisticState,
    
    CTCONU_PAS5201_SetAlarmAdminState,
    CTCONU_PAS5201_SetAlarmThreshold,
    CTCONU_PAS5201_GetDbaReportThresholds,
    CTCONU_PAS5201_SetDbaReportThresholds,
    
    CTCONU_PAS5201_GetMxuMngGlobalConfig,
    CTCONU_PAS5201_SetMxuMngGlobalConfig,
    CTCONU_PAS5201_GetMxuMngSnmpConfig,
    CTCONU_PAS5201_SetMxuMngSnmpConfig,
    
    CTCONU_PAS5201_GetHoldover,
    CTCONU_PAS5201_SetHoldover,
    CTCONU_PAS5201_GetOptTransDiag,
    CTCONU_PAS5201_SetTxPowerSupplyControl,

    CTCONU_PAS5201_GetFecAbility,

    CTCONU_PAS5201_GetIADInfo,
    CTCONU_PAS5201_GetVoipIadOperStatus,
    CTCONU_PAS5201_SetVoipIadOperation,
    CTCONU_PAS5201_GetGlobalConfig,
    CTCONU_PAS5201_SetGlobalConfig,
    CTCONU_PAS5201_GetVoipFaxConfig,
    CTCONU_PAS5201_SetVoipFaxConfig,

    CTCONU_PAS5201_GetVoipPortStatus,
    CTCONU_PAS5201_GetVoipPort,
    CTCONU_PAS5201_SetVoipPort,
    CTCONU_PAS5201_GetVoipPort2,
    CTCONU_PAS5201_SetVoipPort2,

    CTCONU_PAS5201_GetH248Config,
    CTCONU_PAS5201_SetH248Config,
    CTCONU_PAS5201_GetH248UserTidConfig,
    CTCONU_PAS5201_SetH248UserTidConfig,
    CTCONU_PAS5201_GetH248RtpTidConfig,
    CTCONU_PAS5201_SetH248RtpTidConfig,
    
    CTCONU_PAS5201_GetSipConfig,
    CTCONU_PAS5201_SetSipConfig,
    CTCONU_PAS5201_SetSipDigitMap,
    CTCONU_PAS5201_GetSipUserConfig,
    CTCONU_PAS5201_SetSipUserConfig,
    CTCONU_Onustats_GetOnuPortDataByID,
#endif

#if 1
/* -------------------ONU Զ�̹���API------------------- */
    NULL,           /* CliCall */

    CTCONU_PAS5201_ResetOnu,
    GWONU_SetMgtConfig,
    GWONU_SetMgtLaser,
    GWONU_SetTemperature,
    GWONU_SetPasFlush,
    
    GWONU_SetAtuAgingTime,
    GWONU_SetAtuLimit,
    
    GWONU_SetPortLinkMon,
    GWONU_SetPortModeMon,
    GWONU_SetPortIsolate,

    CTCONU_PAS5201_SetVlanEnable,
    CTCONU_PAS5201_SetVlanMode,
    REMOTE_OK,     /* AddVlan */
    CTCONU_PAS5201_DelVlan,
    CTCONU_PAS5201_SetPortPvid,
    
    CTCONU_PAS5201_AddVlanPort,
    CTCONU_PAS5201_DelVlanPort,
    CTCONU_PAS5201_SetVlanTran,
    CTCONU_PAS5201_DelVlanTran,
    CTCONU_PAS5201_SetVlanAgg,
    CTCONU_PAS5201_DelVlanAgg,
    
    CTCONU_PAS5201_SetPortQinQEnable,
    CTCONU_PAS5201_AddQinQVlanTag,
    CTCONU_PAS5201_DelQinQVlanTag,

    GWONU_SetPortVlanFrameTypeAcc,
    GWONU_SetPortIngressVlanFilter,
    
    CTCONU_PAS5201_SetPortMode,
    CTCONU_PAS5201_SetPortFcMode,
    GWONU_SetPortAtuLearn,
    GWONU_SetPortAtuFlood,
    GWONU_SetPortLoopDetect,
    GWONU_SetPortStatFlush,
    
    GWONU_SetIngressRateLimitBase,
    CTCONU_PAS5201_SetPortIngressRate,
    CTCONU_PAS5201_SetPortEgressRate,
    
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    REMOTE_OK,
    
    CTCONU_PAS5201_SetPortQosRule,
    CTCONU_PAS5201_ClrPortQosRule,
    GWONU_SetPortQosRuleType,
    
    CTCONU_PAS5201_SetPortDefaultPriority,
    GWONU_SetPortNewPriority,
    GWONU_SetQosPrioToQueue,
    GWONU_SetQosDscpToQueue,
    GWONU_SetPortUserPriorityEnable,
    GWONU_SetPortIpPriorityEnable,
    GWONU_SetQosAlgorithm,
    GWONU_SET_QosMode,
    GWONU_SET_Rule,
    
    CTCONU_PAS5201_SetIgmpEnable,
    CTCONU_PAS5201_SetIgmpAuth,
    GWONU_SetIgmpHostAge,
    GWONU_SetIgmpGroupAge,
    GWONU_SetIgmpMaxResTime,
    GWONU_SetIgmpMaxGroup,
    GWONU_AddIgmpGroup,
    GWONU_DeleteIgmpGroup,
    GWONU_SetPortIgmpFastLeave,
    CTCONU_PAS5201_SetPortMulticastVlan,

    GWONU_SetPortMirrorFrom,
    GWONU_SetPortMirrorTo,
    GWONU_DeleteMirror,
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMCЭ�����API------------------- */
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
    NULL,           /* GetCmcDownChannelMode */
    NULL,           /* SetCmcDownChannelMode */
    NULL,           /* GetCmcUpChannelMode */
    
    NULL,           /* SetCmcUpChannelMode */
    NULL,           /* GetCmcUpChannelD30Mode */
    NULL,           /* SetCmcUpChannelD30Mode */
    NULL,           /* GetCmcDownChannelFreq */
    NULL,           /* SetCmcDownChannelFreq */

    NULL,           /* GetCmcUpChannelFreq */
    NULL,           /* SetCmcUpChannelFreq */
    NULL,           /* GetCmcDownAutoFreq */
    NULL,           /* SetCmcDownAutoFreq */
    NULL,           /* GetCmcUpAutoFreq */
    
    NULL,           /* SetCmcUpAutoFreq */
    NULL,           /* GetCmcUpChannelWidth */
    NULL,           /* SetCmcUpChannelWidth */
    NULL,           /* GetCmcDownChannelAnnexMode */
    NULL,           /* SetCmcDownChannelAnnexMode */
    
    NULL,           /* GetCmcUpChannelType */
    NULL,           /* SetCmcUpChannelType */
    NULL,           /* GetCmcDownChannelModulation */
    NULL,           /* SetCmcDownChannelModulation */
    NULL,           /* GetCmcUpChannelProfile */
    
    NULL,           /* SetCmcUpChannelProfile */
    NULL,           /* GetCmcDownChannelInterleaver */
    NULL,           /* SetCmcDownChannelInterleaver */
    NULL,           /* GetCmcDownChannelPower */
    NULL,           /* SetCmcDownChannelPower */

    NULL,           /* GetCmcUpChannelPower */
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

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSISӦ�ù���API------------------- */
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

    REMOTE_ERROR,
};


void GWONU_Pas5001_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS5001_GW, &s_gwonu5001Ifs);
}

void GWONU_Pas5201_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS5201_GW, &s_gwonu5201Ifs);
}

void GWONU_Pas5204_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS5204_GW, &s_gwonu5204Ifs);
}


void CTCONU_Pas5201_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS5201_CTC, &s_ctconu5201Ifs);
}

void CTCONU_Pas5204_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_PAS5204_CTC, &s_ctconu5204Ifs);
}



#ifdef __cplusplus

}

#endif
