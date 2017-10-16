/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_GPon.c
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
*   Date: 			2015/10/08
*   Author:		luhao
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  15/10/08  |   luhao    |     create 
**----------|-----------|------------------
***************************************************************/

/*#if defined(_GPON_BCM_SUPPORT_)            */

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
#include  "../../driver/pon/gpon/gponadp/include/GponOnuAdp.h"


extern PON_onu_address_table_record_t  MAC_Address_Table[PON_ADDRESS_TABLE_SIZE];
extern int gponOnuAdp_GetOnuStatus(int oltId, int onuId, GPONADP_ONU_STATUS_E *status);
extern int gponOnuAdp_DeregOnu(int oltId, int onuId);
extern int gponOnuAdp_EnableOnu(int oltId, int onuId);
extern int gponOnuAdp_RemoveOnu(int oltId, int onuId);
extern int gponOnuAdp_SetUpBW(int oltId, int onuId, GPONADP_ONU_BW bw);
extern int gponOnuAdp_SetFec(int oltId, int onuId, GPONADP_ONU_STATE_E mode);
#if 0
extern int gponOnuAdp_SetVlan(int oltId, int onuId,  int port, GPONADP_ONU_port_vlan_configuration_t config);
#else
extern int gponOnuAdp_SetVlanMode(int oltId, int onuId,  int port, GPONADP_ONU_port_vlan_configuration_t config);
#endif
extern int gponOnuAdp_GetVlan(int oltId, int onuId,  int port, GPONADP_ONU_port_vlan_configuration_t *config);

extern int gponOnuAdp_AddVlanByPort(int oltId, int onuId,  int port, short int vlan, int tag);
extern int gponOnuAdp_DelVlanByPort(int oltId, int onuId,  int port, short int vlan, int tag);


extern int gponOnuAdp_GetOnuCfg(int olt_id, int onu_id, GPONOnuConfig *onuConfig);
/*extern int gponOnuAdp_SetOnuCfg(int olt_id, int onu_id, GPONOnuConfig onuConfig);*/
extern int gponOnuAdp_ResetOnu(int olt_id, int onu_id);
extern int gponOnuAdp_GetOperationalState(short int olt_id, short int onu_id, int *operationalState);
extern int gponOnuAdp_SetUniPortAdminState(short int olt_id, short int onu_id, short int port_id, int state);
extern int gponOnuAdp_GetUniPortAdminState(short int olt_id, short int onu_id, short int port_id, int *state);
extern int gponOnuAdp_GetUniPortOperState(short int olt_id, short int onu_id, short int port_id, int *state);
extern int gponOnuAdp_SetUniPortMode(short int olt_id, short int onu_id, short int port_id, GPONADP_ONU_PORT_MODE mode);
extern int gponOnuAdp_GetUniPortMode(short int olt_id, short int onu_id, short int port_id, GPONADP_ONU_PORT_MODE *mode);
extern int gponOnuAdp_GetOnuOptPower(int olt_id, int onu_id, GPONOnuConfig *onuConfig);
extern int gponOnuAdp_SetQinQVlan(int oltId, int onuId,  int port, short int cvlan, short int svlan);
extern int gponOnuAdp_DelQinQVlan(int oltId, int onuId,  int port, short int svlan);

static int GPONONU_BCM68620_SetOnuMacLimit(short int olt_id, short int onu_id, int mac_limit);
#define STATICTEMPLATE  0   
#define DYNAMICTEMPLATE 1 



/****************************************内部适配*******************************************/
static int GPONONU_BCM68620_DeregisterOnu(short int olt_id, short int onu_id);

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
int GPONONU_OnuIsValid(short int olt_id, short int onu_id, int *status)
{
    int onuMgtIdx;
    unsigned char *MacAddr;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    onuMgtIdx = olt_id*MAXONUPERPON+onu_id;
    ONU_MGMT_SEM_TAKE;
    if( VOS_StrLen(OnuMgmtTable[onuMgtIdx].DeviceInfo.DeviceSerial_No) == 0)
    {
        *status = 0;
    }
    else
    {
        *status = 1;
    }   
    ONU_MGMT_SEM_GIVE;

    /* OLT_GW_DEBUG(OLT_GW_TITLE"GW_OnuIsValid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, 0, SYS_LOCAL_MODULE_SLOTNO); */
    
    return 0;
}

static int GPONONU_BCM68620_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    int iRlt = 0;
    short int llid;
    GPONADP_ONU_STATUS_E onu_status;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    *status = 0;
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        if(VOS_OK == gponOnuAdp_GetOnuStatus(olt_id, llid, &onu_status) && onu_status == GPONADP_ONU_STATUS_ACTIVE)
        {
            *status = TRUE;
        }
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_OnuIsOnline(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *status, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/* 删除ONU */
static int GPONONU_BCM68620_DelOnuByManual(short int olt_id, short int onu_id)
{	     
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( activeOneLocalPendingOnu(olt_id, onu_id) == RERROR )
	{
	    GPONONU_BCM68620_DeregisterOnu(olt_id, onu_id);
	}

	OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_DelOnuByManual(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, 0, SYS_LOCAL_MODULE_SLOTNO);

    return ROK;
}
static int GPONONU_BCM68620_AddOnuByManual(short int olt_id, short int onu_id, unsigned char *sn)
{
    short int OnuEntry;
    unsigned char *pSrcMac;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sn);

    OnuEntry = olt_id * MAXONUPERPON + onu_id;    
    if ( 0 != VOS_StrCmp(OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, sn) )
    {
        /* 强行霸占记录地址 */
	    GPONONU_BCM68620_DeregisterOnu(olt_id, onu_id);
        OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AddOnuByManual(%d,%d) deregister on slot %d.\r\n", olt_id, onu_id, SYS_LOCAL_MODULE_SLOTNO);
    }   

    return( ROK );
}

static int GPONONU_BCM68620_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    *chip_type   = PONCHIP_GPON;
    *remote_type = ONU_MANAGE_GPON;
    return OLT_ERR_OK;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
/*onu deregister*/
static int GPONONU_BCM68620_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        /*iRlt = gponOnuAdp_DeregOnu(olt_id, llid);*/
	    iRlt = gponOnuAdp_ResetOnu(olt_id, llid);		
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_DeregisterOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned long flags;
    short int llid;
    bcmEmmiLinkInfo link_info;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    iRlt = 0;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AuthorizeOnu(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_AuthRequest ( short int olt_id, short int onu_id, CTC_STACK_auth_response_t *auth_response)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(auth_response);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = 0;
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AuthRequest(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GPONONU_BCM68620_AuthSuccess ( short int olt_id, short int onu_id)
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = 0;
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AuthSuccess(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GPONONU_BCM68620_AuthFailure (short int olt_id, short int onu_id, CTC_STACK_auth_failure_type_t failure_type )
{
    int iRlt = CTC_STACK_NOT_IMPLEMENTED;
    short int llid;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
	    iRlt = 0;
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AuthFailure(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, failure_type, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}


#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
/*traffic service enable*/
/* 5001和5201共用此函数*//*问题单11283*/
static int GPONONU_BCM68620_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
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
         iRlt = GPONONU_BCM68620_DeregisterOnu(olt_id, onu_id);
	    }
    }
    else
    {
        VOS_ASSERT(0);
        iRlt = OLT_ERR_PARAM;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuTrafficServiceMode(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int test_bw(int slot, int port, int onuid, int AssureBw, int fix, int MaxBw, int Type)
{
    short int PonPortIdx = GetPonPortIdxBySlot(slot, port);
    short int OnuIdx = onuid-1;
    short int llid = GetLlidActivatedByOnuIdx(PonPortIdx, OnuIdx);
    short int iRlt = 0;
    GPONADP_ONU_BW bw;
    
    VOS_MemZero(&bw, sizeof(GPONADP_ONU_BW));

    /*所配置的带宽颗粒度均为64k*/
    bw.bwAssured = (AssureBw/1000)*1024;
    bw.bwFixed = (fix/1000)*1024;
    bw.bwMax = (MaxBw/1000)*1024;
    bw.tcontType = Type;
    iRlt = gponOnuAdp_SetUpBW(PonPortIdx, llid, bw);    
    
    return iRlt;

}

static int GPONONU_BCM68620_SetOnuUplinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt = 0;
    GPONADP_ONU_BW bw;
    unsigned int bwAssured = 0;
    unsigned int bwMax = 0;
    unsigned int bwFixed = 0;
    VOS_MemZero(&bw, sizeof(GPONADP_ONU_BW));
#if 1
    /*所配置的带宽颗粒度均为64k，采用四舍五入法，配置相邻精度*/
    bwAssured = (BW->bw_gr/64)*64;
    if(BW->bw_gr%64>32)
        bwAssured += 64;
        
    bwMax = (BW->bw_be/64)*64;        
    if(BW->bw_be%64>32)  
        bwMax += 64;   
        
    if(BW->bw_fixed)
    {
        bwFixed = (BW->bw_fixed/64)*64;
        /*固定带宽必须保证大于等于256k*/
        if(BW->bw_fixed%64>32)  
            bwFixed += 64; 
		else if( bwFixed == 0)
			bwFixed += 256;
            
        if(BW->bw_be == 0 && BW->bw_gr == 0)
        {
            /*type 1:只包含固定带宽，且最大带宽等于固定带宽*/
            bw.tcontType = GPONADP_ONU_TCONT_TYPE_1;
            bw.bwAssured = bwFixed;
            bw.bwFixed =  bwFixed;
            bw.bwMax = bwFixed;            
        }
        else
        {
        	if(bwMax < (bwAssured + bwFixed))
    		{
    			OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuUplinkBW(%d, %d)'s err for  bwMax < (bwAssured + bwFixed) on slot %d.\r\n", olt_id, llid,  SYS_LOCAL_MODULE_SLOTNO);
    			return OLT_ERR_PARAM;
    		}
            /*type 5:包含固定带宽，且同时包含保证带宽和最大带宽，满足max >= fix + be，fix带宽大于64kB*/
            bw.tcontType = GPONADP_ONU_TCONT_TYPE_5;
            bw.bwAssured = bwAssured;
            bw.bwFixed =  bwFixed;
            bw.bwMax = bwMax;
        }
    }
    else
    {
    	
    	if(bwMax == bwAssured)
		{
			bw.tcontType = GPONADP_ONU_TCONT_TYPE_2;
	        bw.bwAssured = bwAssured;
			bw.bwMax = bwMax; 
		}
		else if(bwMax > bwAssured)
		{
			bw.tcontType = GPONADP_ONU_TCONT_TYPE_3;
	        bw.bwAssured = bwAssured;
			bw.bwMax = bwMax; 
		}
       
        bw.bwFixed =  0;
               
    } 
#else
    /*所配置的带宽颗粒度均为64k*/
    bw.bwAssured = (BW->bw_gr/64)*64;
    bw.bwFixed =  256/*(BW->bw_gr/64)*64*/;
    bw.bwMax = (BW->bw_be/64)*64;
    bw.tcontType = GPONADP_ONU_TCONT_TYPE_5;
#endif    
    iRlt = gponOnuAdp_SetUpBW(olt_id, llid, bw);    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuUplinkBW(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_SetOnuDownlinkBW(short int olt_id, short int llid, ONU_bw_t *BW)
{
    int iRlt = 0;
 
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuDownlinkBW(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, llid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int  GPONONU_BCM68620_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
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
                iRlt = GPONONU_BCM68620_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
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

                    iRlt = GPONONU_BCM68620_SetOnuUplinkBW(olt_id, llid, &BW_NewDefault);
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
                                (void)GPONONU_BCM68620_SetOnuDownlinkBW(olt_id, llid, &BW_NewDefault);
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
                
                iRlt = GPONONU_BCM68620_SetOnuUplinkBW(olt_id, llid, BW);
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
                
                iRlt = GPONONU_BCM68620_SetOnuDownlinkBW(olt_id, llid, BW);
            }
        }
    }   
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuBW(%d, %d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, BW->bw_actived, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt;
}
static int GPONONU_BCM68620_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid, onuid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {    
        GPONADP_ONU_STATE_E fec_state = fec_mode?GPONADP_ONU_STATE_ENABLE:GPONADP_ONU_STATE_DISABLE;    
        iRlt = gponOnuAdp_SetFec(olt_id, llid, fec_state);
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuFecMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	
	return iRlt;	
}

#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */




#endif


#if 1
/* -------------------ONU加密管理API------------------- */


#endif


#if 1
/* -------------------ONU 地址管理API------------------- */
/*show fdbentry mac*/
static int GPONONU_BCM68620_GetOnuMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_onu_address_table_record_t *addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        long int liEntryNum;
        
    }   
	
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GWONU_BCM55538_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*show fdbentry mac*/
static int GPONONU_BCM68620_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	iRlt = gponOnuAdp_GetMacAddrTbl(olt_id, llid, EntryNum, addr_table);
    }   
	
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GWONU_BCM68620_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*为获取gpon onu 中mac地址vlan而做*/
static int GPONONU_BCM68620_GetOltMacAddrVlanTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_vlan_table_t addr_table)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
	
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	iRlt = gponOnuAdp_GetMacAddrVlanTbl(olt_id, llid, EntryNum, addr_table);
    }   
	
	OLT_GPON_DEBUG(OLT_GPON_TITLE"GPONONU_BCM68620_GetOltMacAddrVlanTbl(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;			
}

/*set onu max mac*/
static int GPONONU_BCM68620_SetOnuMaxMacNum(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
{
    int iRlt = OLT_ERR_NOTEXIST;
    unsigned int uiSetVal;
	short int llid, onuid;
    unsigned short max_entry;
    int default_flag  = 1;
    /*tOgCmOnuConfig *onuConfig = NULL;*/

    
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
    if (INVALID_LLID != ( llid = GetLlidByOnuIdx(olt_id,onu_id)))
    {
        iRlt = GPONONU_BCM68620_SetOnuMacLimit(olt_id, onu_id, uiSetVal);
    }

    OLT_GPON_DEBUG(OLT_GPON_TITLE"GPONONU_BCM68620_SetOnuMaxMacNum(%d, %d, %d)'s result(%d)"
        " on slot %d.\r\n", olt_id, onu_id, uiSetVal, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
#if 0
    onuConfig = VOS_Malloc(sizeof(tOgCmOnuConfig), MODULE_ONU);
    if(!onuConfig)
        return iRlt;
    
    VOS_MemZero(onuConfig, sizeof(tOgCmOnuConfig));
    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_MAC_LIMIT;        
    if(VOS_OK == (iRlt = OnuMgt_GetGponOnuCfg(olt_id, onu_id, onuConfig)))
    {
        OLT_BCM_DEBUG("onuConfig->macLimit = %d \r\n", onuConfig->macLimit);
    }
    VOS_MemZero(onuConfig, sizeof(tOgCmOnuConfig));
    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_MAC_LIMIT;        
    onuConfig->macLimit = uiSetVal;
    iRlt = OnuMgt_SetGponOnuCfg(olt_id, onu_id, onuConfig);	
    VOS_Free(onuConfig);
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, SYS_LOCAL_MODULE_SLOTNO);
#endif
    return iRlt;
}

/*get onu pon uni mac config*/
static int GPONONU_BCM_GetOnuUniMacCfg(short int olt_id, short int onu_id, PON_oam_uni_port_mac_configuration_t *mac_config)
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
static int GPONONU_BCM68620_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt = OLT_ERR_NOTEXIST;
	short int llid;
    /*tOgCmOnuConfig *onuConfig = NULL;*/
		
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);
#if 0 
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        onuConfig = VOS_Malloc(sizeof(tOgCmOnuConfig), MODULE_ONU);
        if(!onuConfig)
            return iRlt;
        
        VOS_MemZero(onuConfig, sizeof(tOgCmOnuConfig));
        onuConfig->bitMask2 |= fCMAPI_ONU_CONFIG_MASK_DISTANCE;        
        if(VOS_OK == (iRlt = OnuMgt_GetGponOnuCfg(olt_id, onu_id, onuConfig)))
        {
            *rtt = onuConfig->distance;
            OLT_BCM_DEBUG("onuConfig->macLimit = %d \r\n", onuConfig->distance);
        }
    
        VOS_Free(onuConfig);
    }   
#endif
	OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetOnuDistance(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
	return iRlt;
}

static int GPONONU_BCM_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
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

static int GPONONU_BCM_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
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
static int GPONONU_BCM_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions)
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

static int GPONONU_BCM_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
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
            onu_info->rtt = link_info.rangeValue;
            
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

static int GPONONU_BCM68620_ResetOnu(short int olt_id, short int onu_id, int en)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_ResetOnu( olt_id, llid );
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_ResetOnu(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt; 			
}
#endif


#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */

static int GPONONU_BCM_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        /*iRlt = gponOnuAdp_GetSn(olt_id, llid, serial_number);*/
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM55538_GetSerialNumber(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_GetEthPortLinkstate(short int olt_id, short int onu_id, int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_GetUniPortOperState(olt_id, llid, (unsigned char)port_id, enable);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetEthPortLinkstate(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GPONONU_BCM68620_GetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int* enable)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_GetUniPortAdminState(olt_id, llid, port_id, enable);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_SetEthPortAdminstatus(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_SetUniPortAdminState(olt_id, llid, port_id, enable);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetEthPortAdminstatus(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}


static int GPONONU_BCM_GetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int* enable)
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

static int GPONONU_BCM_SetEthPortPauseEnable(short int olt_id, short int onu_id, int port_id, int enable)
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


static int GPONONU_BCM68620_GetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int *enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    int lReturn_code = 0;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(enable);
#if 0/*自协商不该有get*/
    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        int state = 0;

        iRlt = gponOnuAdp_GetUniPortMode(olt_id, llid, lport, &state);
        if ( 0 == iRlt )
        {
            *enable = (state==1)?1:0;
        }
        else
        {
            /*GPON 如果没有绑定profile，则返回默认值*/
            *enable = 0;
        }
        
    }
#endif

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, *enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturn_code;
}

static int GPONONU_BCM68620_SetEthPortAutoNegotinationAdmin(short int olt_id, short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        uchar lport = port_id;
        int state = enable?1:2;

        iRlt = gponOnuAdp_SetUniPortMode(olt_id, llid, lport, state);
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetEthPortAutoNegotinationAdmin(%d, %d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, enable, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_GetOptTransDiag ( short int olt_id, short int onu_id,
		   CTC_STACK_optical_transceiver_diagnosis_t	*optical_transceiver_diagnosis )
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
	GPONOnuConfig *onuConfig = NULL;
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	if(NULL == optical_transceiver_diagnosis)
	{
		VOS_ASSERT(0);
		return iRlt;
	}
	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
		
	    onuConfig = VOS_Malloc(sizeof(GPONOnuConfig), MODULE_ONU);
	    if(!onuConfig)
	        return iRlt;
    
	    VOS_MemZero(onuConfig, sizeof(GPONOnuConfig));
	    if(VOS_OK == (iRlt = gponOnuAdp_GetOnuOptPower(olt_id, llid, onuConfig)))
    	{
    		optical_transceiver_diagnosis->rx_power = onuConfig->rxpower;
			optical_transceiver_diagnosis->tx_power = onuConfig->txpower;
			optical_transceiver_diagnosis->supply_voltage = 3;
			optical_transceiver_diagnosis->transceiver_temperature = 20;
			optical_transceiver_diagnosis->tx_bias_current = 1;
    	}
		if(onuConfig)
	    {
	        VOS_Free(onuConfig);
	        onuConfig = NULL;
	    }
	}

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetOptTransDiag(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

	return iRlt;
}

static int GPONONU_BCM_SetEthPortRestartAutoConfig(short int olt_id, short int onu_id, int port_id )
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

static int GPONONU_BCM_GetEthPortAnLocalTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
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

static int GPONONU_BCM_GetEthPortAnAdvertisedTecAbility(short int  olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
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


static int GPONONU_BCM_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
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

static int GPONONU_BCM_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
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

static int GPONONU_BCM_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t * policing)
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

static int GPONONU_BCM_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
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


static int GPONONU_BCM68620_GetEthPortVlanConfig(short int olt_id, short int onu_id,  int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int i = 0;
    short int llid;
    GPONADP_ONU_port_vlan_configuration_t vlan_config;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        VOS_MemZero(&vlan_config, sizeof(GPONADP_ONU_port_vlan_configuration_t));
        iRlt = gponOnuAdp_GetVlan(olt_id, llid, port_id, &vlan_config);
        if(iRlt == VOS_OK)
    	{
    	    vconf->default_vlan = vlan_config.default_vlan;
    	    vconf->mode = vlan_config.mode;
    	    vconf->number_of_entries = vlan_config.number_of_entries;
    	    for(i=0;i<vconf->number_of_entries;i++)
    	    {
        	    vconf->vlan_list[i] = vlan_config.vlan_list[i];
    	    }
    	}
    	
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        GPONADP_ONU_port_vlan_configuration_t vlan_conf;
        int i = 0;
        VOS_MemZero(&vlan_conf, sizeof(GPONADP_ONU_port_vlan_configuration_t));
        vlan_conf.mode = vconf->mode;
        vlan_conf.default_vlan = vconf->default_vlan;

        if(vconf->number_of_entries > GPONADP_ONU_VLAN_FILTER_ENTRIES)
            vconf->number_of_entries = GPONADP_ONU_VLAN_FILTER_ENTRIES;
		
        for(i = 0; i<vconf->number_of_entries;i++)
        {
            vlan_conf.vlan_list[i] = vconf->vlan_list[i];
        }
        vlan_conf.number_of_entries = vconf->number_of_entries;

		#if 0
        iRlt = gponOnuAdp_SetVlan(olt_id, llid, (unsigned char)port_id, vlan_conf);
		#else
		iRlt = gponOnuAdp_SetVlanMode(olt_id, llid, (unsigned char)port_id, vlan_conf);
		#endif
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM_GetAllPortVlanConfig(short int olt_id, short int onu_id,  unsigned char *portNum, CTC_STACK_vlan_configuration_ports_t ports_info)
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


static int GPONONU_BCM_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *strip )
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

static int GPONONU_BCM_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int strip )
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

static int GPONONU_BCM_GetMulticastAllPortTagStrip ( short int olt_id, short int onu_id, unsigned char *number_of_entries, CTC_STACK_multicast_ports_tag_strip_t ports_info )
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

static int GPONONU_BCM_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
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

static int GPONONU_BCM_SetEthPortMulticastTagOper( short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
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

static int GPONONU_BCM_SetObjMulticastTagOper ( short int olt_id,short int onu_id, CTC_management_object_t *management_object, CTC_STACK_tag_oper_t tag_oper, CTC_STACK_multicast_vlan_switching_t *sw )
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


static int GPONONU_BCM_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc)
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

static int GPONONU_BCM_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
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

static int GPONONU_BCM_ClearMulticastControl(short int olt_id, short int onu_id )
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

static int GPONONU_BCM_GetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t *sw)
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

static int GPONONU_BCM_SetMulticastSwitch(short int olt_id, short int onu_id, CTC_STACK_multicast_protocol_t sw)
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

static int GPONONU_BCM_GetFastLeaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
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

static int GPONONU_BCM_GetFastLeaveAdminState(short int olt_id, short int onu_id, int *state)
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

static int GPONONU_BCM_SetFastLeaveAdminState(short int olt_id, short int onu_id, int state)
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


static int GPONONU_BCM68620_GetOnuPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_GetEthPortStats(olt_id, llid, (unsigned char)port_id, data);
    }
   

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_68620_GetOnuPortStatisticData(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_GetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_OK;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(state);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	if(gponOnuAdp_isEthPortStatsEn(olt_id, llid, (unsigned char)port_id))
		{
			state->status = CTC_STACK_STATISTIC_STATE_ENABLE;
		}
		else
		{
			state->status = CTC_STACK_STATISTIC_STATE_DISABLE;
		}
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_68620_GetOnuPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_SetOnuPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    	if(CTC_STACK_STATISTIC_STATE_ENABLE == state->status)
		{
	        iRlt = gponOnuAdp_EnableEthPortStats(olt_id, llid, (unsigned char)port_id);
		}
		else
		{
			iRlt = gponOnuAdp_DisableEthPortStats(olt_id, llid, (unsigned char)port_id);
		}
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_68620_SetOnuPortStatisticState(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;

}


/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
static int GPONONU_BCM_SetAlarmAdminState(short int olt_id, short int onu_id, CTC_management_object_t *management_object,
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

static int GPONONU_BCM_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
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

static int GPONONU_BCM_GetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
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

static int GPONONU_BCM_SetDbaReportThresholds ( short int olt_id, short int onu_id, unsigned char *number_of_queue_sets, CTC_STACK_onu_queue_set_thresholds_t  *queues_sets_thresholds )
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


static int GPONONU_BCM68620_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    char onuSn[GPONADP_ONU_SERIAL_NUMBER_LEN];
    int snLen;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if (parameter->onu_sn[0] == 0xff 
     && parameter->onu_sn[1] == 0xff
     && parameter->onu_sn[2] == 0xff
     && parameter->onu_sn[3] == 0xff
     && parameter->onu_sn[4] == 0xff
     && parameter->onu_sn[5] == 0xff)
    {
        /* 不支持all操作 */
        return OLT_ERR_PARAM;
    }

    iRlt = GetOnuSerialNo(olt_id, onu_id, onuSn, &snLen);
    if (iRlt == RERROR)
    {
        return OLT_ERR_PARAM;
    }

    llid = GetLlidByOnuIdx(olt_id, onu_id);

    if (parameter->action == 0)
    {
        /*enable*/
        iRlt = gponOnuAdp_TxPowerEnableByOnuSn(olt_id, llid, onuSn, 1);
    }
    else 
    {
        /* disable */
        iRlt = gponOnuAdp_TxPowerEnableByOnuSn(olt_id, llid, onuSn, 0);
        
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_SetTxPowerSupplyControl(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*begin: 支持设置gpon onu的管理ip，mod by liuyh, 2017-4-25*/
static int CTCONU_BCM68620_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    int vlanMode;
    short int default_vlanId = 0;
    uint32 onuIPAddress = 0x00000000;
    uint32 onuIPMask = 0x00000000;
    uint32 onuDefaultGateway = 0x00000000;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = gponOnuAdp_GetIpAddress(olt_id, llid, &onuIPAddress, &onuIPMask, &onuDefaultGateway);
        if (VOS_OK != iRlt)
        {
            OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_GetMxuMngGlobalConfig: gponOnuAdp_GetIpAddress(%d, %d)'s " \
                "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
        }

        iRlt = gponOnuAdp_GetIpHostVlan(olt_id, llid, &vlanMode, &default_vlanId);
        if (VOS_OK != iRlt)
        {
            OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_GetMxuMngGlobalConfig: gponOnuAdp_GetIpHostVlan(%d, %d)'s " \
                "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
        }
    }

    mxu_mng->mng_ip = onuIPAddress;
    mxu_mng->mng_mask = onuIPMask;
    mxu_mng->mng_gw = onuDefaultGateway;
    mxu_mng->data_cvlan = default_vlanId;
    mxu_mng->data_svlan = 0;
    mxu_mng->data_priority = 0;

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_GetMxuMngGlobalConfig(%d, %d)'s " \
        "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int CTCONU_BCM68620_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;
    int vlanMode;
    short int default_vlan;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
	{
        iRlt = gponOnuAdp_SetIpAddress(olt_id, llid, mxu_mng->mng_ip, mxu_mng->mng_mask, mxu_mng->mng_gw);
        if (VOS_OK == iRlt)
        {
            /* 设置vlan参数，只支持设置cvlan，不支持设置svlan和pri */
            vlanMode = GPONADP_ONU_VLANMODE_TRUNK;
            default_vlan = mxu_mng->data_cvlan;
            
            if (default_vlan == 0)
            {
                default_vlan = 1; /* 缺省带vlan 1 */
            }            

            iRlt = gponOnuAdp_SetIpHostVlan(olt_id, llid, vlanMode, default_vlan);   
            if (VOS_OK != iRlt)
            {
                OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_SetMxuMngGlobalConfig" \
                    ":gponOnuAdp_SetIpHostVlanMode(%d, %d)'s result(%d) on slot %d.\r\n", \
                    olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
            }
        }        
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"CTCONU_BCM68620_SetMxuMngGlobalConfig(%d, %d)'s " \
        "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

void Test_SetOnuMngIp(short int olt_id, short int onu_id, uint32 onuIPAddress, uint32 onuIPMask, uint32 onuDefaultGateway, short int cvlanId)
{
    int iRlt = OLT_ERR_NOTEXIST;
    CTC_STACK_mxu_mng_global_parameter_config_t mxu_mng = {0};

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    mxu_mng.mng_ip = onuIPAddress;
    mxu_mng.mng_mask = onuIPMask;
    mxu_mng.mng_gw = onuDefaultGateway;
    mxu_mng.data_cvlan = cvlanId;

    iRlt = CTCONU_BCM68620_SetMxuMngGlobalConfig(olt_id, onu_id, &mxu_mng);
    if (OLT_ERR_OK != iRlt)
    {
        sys_console_printf("CTCONU_BCM68620_SetMxuMngGlobalConfig: onu(%d, %d)'s result(%d)," \
            " IP 0x%06x, Mask 0x%06x, Gw 0x%06x, CVLAN %d.", olt_id, onu_id, iRlt, onuIPAddress, onuIPMask, 
            onuDefaultGateway, cvlanId);
    }

    return;
}
/*end: mod by liuyh, 2017-4-25*/

static int GPONONU_BCM_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t *fec_ability)
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
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */

static int GPONONU_BCM68620_SetVlanEnable(short int olt_id, short int onu_id, int enable)
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
        iRlt = GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
        if(iRlt != VOS_OK)
        {            
            lReturnValue = iRlt;
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
        }
        
        /*只有模式切换时才要进行实际操作，进行vlan删除。否则模式不变时，不进行vlan删除*/
        if((enable && pvc.mode != ONU_CONF_VLAN_MODE_TRUNK)
        ||(!enable && pvc.mode != ONU_CONF_VLAN_MODE_TRANSPARENT))
        {
            for ( i=0; i<portnum; i++ )
            {
    			VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
    			pvc.mode = enable?ONU_CONF_VLAN_MODE_TRUNK:ONU_CONF_VLAN_MODE_TRANSPARENT;

    			pvc.default_vlan = (0x8100<<16)|1;
			pvc.number_of_entries = 0;


                iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
                if(iRlt != VOS_OK)
                {            
                    lReturnValue = iRlt;
                    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
                }
            }
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, enable, __LINE__);         
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetVlanEnable(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int GPONONU_BCM68620_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
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
        iRlt = GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, port_id, &pvc);
        if(iRlt != VOS_OK)
        {            
            lReturnValue = iRlt;
            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);         
        }
        else if(pvc.mode != mode)
        {
#if 0        
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

            iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, (unsigned char)port_id, &pvc);
            if(iRlt != VOS_OK)
            {
                lReturnValue = iRlt;                
                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);         
            }
#else
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

            iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, (unsigned char)port_id, &pvc);
            if(iRlt != VOS_OK)
            {
                lReturnValue = iRlt;                
                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);         
            }
#endif
        }
        else
        {
            /*相同vlan模式修改，不做实际操作*/
            /*do nothing*/
        }
    
    }
    else
    {
        lReturnValue = VOS_ERROR;        
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, port_id, mode, __LINE__);         
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetVlanMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int GPONONU_BCM68620_DelVlan(short int olt_id, short int onu_id, int vid)
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
            CTC_STACK_port_vlan_configuration_t pvc;                
            for(i=0;i<portnum;i++)
            {
                if(!(all&(1<<i)))
                    continue;
                    
#if 0                    
                VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                iRlt = GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
                if(iRlt == VOS_OK)
                {
                    int num = pvc.number_of_entries;                    
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
                    
                    iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
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
#else              
                /*需要通知ocs模块要删除的vlan是untag还是tag vlan*/
                if(untag&(1<<i))/*恢复默认vlan 1 by jinhl*/
                    /*iRlt = gponOnuAdp_DelVlanByPort(olt_id, llid, i+1, vid, 2);*/
					iRlt = gponOnuAdp_AddVlanByPort(olt_id, llid, i+1, 1, 2);
                else
                    iRlt = gponOnuAdp_DelVlanByPort(olt_id, llid, i+1, vid, 1);
                    
                if(iRlt != VOS_OK)
                {
                    lReturnValue = iRlt;                        
                    OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);         
                }
                    
#endif
            }
        
        }   
    }
    else
    {
        lReturnValue = VOS_ERROR;
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                                 
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_DelVlan(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int GPONONU_BCM68620_SetPortPvid(short int olt_id, short int onu_id, int port_id, int lv)
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
        iRlt = GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, llid, (unsigned char)port_id, &pvc);
        if ( 0 == iRlt )
        {
#if 0        
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

            iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, (unsigned char)port_id, &pvc);
            if(iRlt != VOS_OK)
            {
                lReturnValue = iRlt;                
                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", iRlt,__FUNCTION__, olt_id, onu_id, port_id, lv, __LINE__);                         
            }
#else
            iRlt = gponOnuAdp_AddVlanByPort(olt_id, llid, port_id, lv, 2);
			lReturnValue = iRlt;/*32643,对于GPON ONU添加8个以上VLAN时，应当不可添加并提示by jinhl@2016.08.26*/
#endif
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
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetPortPvid(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int GPONONU_BCM68620_AddVlanPort(short int olt_id, short int onu_id, int vid, int portlist, int tag)
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
    
        {
			CTC_STACK_port_vlan_configuration_t pvc;

			for(i=0;i<portnum;i++)
			{
                if(!(portlist&(1<<i))) 
                    continue;							
#if 0				
				VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                if(GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc) == VOS_OK)
				{
                    num = pvc.number_of_entries;				
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
                    
					iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
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
#else
                if(GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc) == VOS_OK)
				{
                    num = pvc.number_of_entries;				
                    if(tag == 2)
                    {
                        int iv = 0;
                        /*如果已加为tag的，从tag中删除*/
                        for(iv=0; iv<num; iv++)
                        {
                            if((pvc.vlan_list[iv]&0xfff) == vid)
                            {
                                iRlt = gponOnuAdp_DelVlanByPort(olt_id, llid, i+1, vid, 1);
                                break;
                            }
                        }

                        /*不再重复添加untag vlan*/
                        if(vid == (pvc.default_vlan&0xfff))
                        {
                            continue;
                        }
                        
                        
                    }
                    else
                    {
                        int iv = 0;
                        /*端口的default vlan如果为要配置的vlan，要修改为默认vlan*/
                        if(vid == (pvc.default_vlan&0xfff))
                        {
                            iRlt = gponOnuAdp_AddVlanByPort(olt_id, llid, i+1, 1, 2);
                        }
                        
                        for(iv=0; iv<num; iv++)
                        {
                             if((pvc.vlan_list[iv]&0xfff) == vid)
                                 break;
                        }                        
                        /*端口已经存在的vlan，不再添加*/
                        if(iv < num)
                        {
                            continue;
                        }
                    }
				}

                iRlt = gponOnuAdp_AddVlanByPort(olt_id, llid, i+1, vid, tag);
				lReturnValue = iRlt;/*32643,对于GPON ONU添加8个以上VLAN时，应当不可添加并提示by jinhl@2016.08.26*/
#endif
			}
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;                
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                       
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_AddVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return lReturnValue;
}

static int GPONONU_BCM68620_DelVlanPort(short int olt_id, short int onu_id, int vid, int portlist)
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
        
        {
            CTC_STACK_port_vlan_configuration_t pvc;            
            for(i=0;i<portnum;i++)
            {
                if (!(portlist&(1<<i))) 
                    continue;
                    
                VOS_MemSet(&pvc, 0, sizeof(CTC_STACK_port_vlan_configuration_t));
                iRlt = GPONONU_BCM68620_GetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
#if 0                
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
                    iRlt = GPONONU_BCM68620_SetEthPortVlanConfig(olt_id, onu_id, i+1, &pvc);
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
#else
                if(iRlt == VOS_OK)
                {
                    int num = pvc.number_of_entries;
                    
                    for(j=0; j<num; j++)
                    {
                        if((pvc.vlan_list[j]&0xfff) == vid)
                        {
                            iRlt = gponOnuAdp_DelVlanByPort(olt_id, llid, i+1, vid, 1);
                            break;
                        }
                    }
                    
                    /* auto change port pvid to vid 1 when the port default vid eq 'vid'*/
                    if((pvc.default_vlan & 0xfff) == vid)
                    {
                        iRlt = gponOnuAdp_AddVlanByPort(olt_id, llid, i+1, 1, 2);
                    }
                    
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
#endif
            }
        
        }
    }
    else
    {
        lReturnValue = VOS_ERROR;
        OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d, %d), line %d!\r\n", lReturnValue,__FUNCTION__, olt_id, onu_id, i+1, vid, __LINE__);                         
    }
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_DelVlanPort(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, vid, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return lReturnValue;
}

static int GPONONU_BCM68620_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt = OLT_ERR_NOTEXIST;
    ULONG vid = 1;
    short int llid;
    uchar lport = port_id;

    OLT_PAS_DEBUG(OLT_PAS_TITLE"CTCONU_PAS5201_SetPortQinQEnable(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    
    return  VOS_OK;
}

/*qinq port vlan tag add*/
static int GPONONU_BCM68620_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_SetQinQVlan(olt_id, llid, port_id, cvlan, svlan);
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GPONONU_BCM68620_AddQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

/*qinq port vlan tag del*/
static int GPONONU_BCM68620_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
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
            iRlt = gponOnuAdp_DelQinQVlan(olt_id, llid, port_id, (short int)svlan);
        }
    }
    
    OLT_PAS_DEBUG(OLT_PAS_TITLE"GPONONU_BCM68620_DelQinQVlanTag(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return  iRlt;
}

static int GPONONU_BCM68620_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt = OLT_ERR_NOTEXIST;
    GPONADP_ONU_PORT_MODE glmode = 0;
	int llid = 0;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
    
#if 1       
        if(mode)
        {
        
            switch(mode)
            {
                case 10:
                    glmode = GPONADP_ONU_10BASET_FDUPLEX;
                    break;
                case 11:
                    glmode = GPONADP_ONU_10BASET_HDUPLEX;
                    break;                    
                case 8:
                    glmode = GPONADP_ONU_100BASET_FDUPLEX;
                    break;
                case 9:
                    glmode = GPONADP_ONU_100BASET_HDUPLEX;
                    break;
                case 12:
                    glmode = GPONADP_ONU_GBITETH_FDUPLEX;
                    break;
                default:
                    iRlt = VOS_ERROR;
                    break;
            } 
            
            if(iRlt == OLT_ERR_NOTEXIST)
                iRlt = gponOnuAdp_SetUniPortMode(olt_id, llid, port_id, glmode);
        
        }
        else
        {
            /*设置为自协商*/
            iRlt = gponOnuAdp_SetUniPortMode(olt_id, llid, port_id, GPONADP_ONU_AUTONE);
            
        }
#endif
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetPortMode(%d, %d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, port_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

static int GPONONU_BCM68620_SetMulticastTemplate( short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int ONU_Multicast_Prof_Structlen = 0;
   short int ONU_StaticMcast_Prof_Structlen = 1;
   short int ONU_DynamicMcast_Prof_Structlen = 2;
   GPONADP_ONU_StaticMcast_Prof_t static_cfg;
   GPONADP_ONU_DynamicMcast_Prof_t dynamic_cfg;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);

    ONU_Multicast_Prof_Structlen = sizeof(CTC_GPONADP_ONU_Multicast_Prof_t);
    ONU_StaticMcast_Prof_Structlen= sizeof(GPONADP_ONU_StaticMcast_Prof_t);
    ONU_DynamicMcast_Prof_Structlen = sizeof(GPONADP_ONU_DynamicMcast_Prof_t);

    if(ONU_Multicast_Prof_Structlen != ONU_StaticMcast_Prof_Structlen || ONU_Multicast_Prof_Structlen != ONU_DynamicMcast_Prof_Structlen)
		return OLT_ERR_NOTEXIST;
	
     if(stateflag == STATICTEMPLATE)
     {
     	   VOS_MemSet( &static_cfg,0,ONU_StaticMcast_Prof_Structlen);
	   VOS_MemCpy(&static_cfg,parameter,ONU_StaticMcast_Prof_Structlen);
	      iRlt = gponOnuAdp_staticMcastProfSet(prof1,prof2,static_cfg);
     }
    else if(stateflag == DYNAMICTEMPLATE)
    	{
      	   VOS_MemSet( &dynamic_cfg,0,ONU_DynamicMcast_Prof_Structlen);
	   VOS_MemCpy(&dynamic_cfg,parameter,ONU_DynamicMcast_Prof_Structlen);

	      iRlt = gponOnuAdp_dynamicMcastProfSet(prof1,prof2,dynamic_cfg);  		
    	}
   OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetMulticastTemplate(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
static int GPONONU_BCM68620_GetMulticastTemplate( short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt = OLT_ERR_NOTEXIST;
   short int ONU_Multicast_Prof_Structlen = 0;
   short int ONU_StaticMcast_Prof_Structlen = 1;
   short int ONU_DynamicMcast_Prof_Structlen = 2;
   GPONADP_ONU_StaticMcast_Prof_t static_cfg;
   GPONADP_ONU_DynamicMcast_Prof_t dynamic_cfg;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);

    ONU_Multicast_Prof_Structlen = sizeof(CTC_GPONADP_ONU_Multicast_Prof_t);
    ONU_StaticMcast_Prof_Structlen= sizeof(GPONADP_ONU_StaticMcast_Prof_t);
    ONU_DynamicMcast_Prof_Structlen = sizeof(GPONADP_ONU_DynamicMcast_Prof_t);

    if(ONU_Multicast_Prof_Structlen != ONU_StaticMcast_Prof_Structlen || ONU_Multicast_Prof_Structlen != ONU_DynamicMcast_Prof_Structlen)
		return OLT_ERR_NOTEXIST;
	
     if(stateflag == STATICTEMPLATE)
     {
     	   VOS_MemSet( &static_cfg,0,sizeof(GPONADP_ONU_StaticMcast_Prof_t));
	    iRlt = gponOnuAdp_staticMcastProfGet(prof1,prof2,&static_cfg);
	   if(iRlt != 0)
	  	return OLT_ERR_NOTEXIST;
	   VOS_MemCpy(parameter,&static_cfg,sizeof(GPONADP_ONU_StaticMcast_Prof_t));
     }
    else if(stateflag == DYNAMICTEMPLATE)
    	{
      	   VOS_MemSet( &dynamic_cfg,0,sizeof(GPONADP_ONU_DynamicMcast_Prof_t));
	    iRlt = gponOnuAdp_dynamicMcastProfGet(prof1,prof2,&dynamic_cfg);  	
	   if(iRlt != 0)
	  	return OLT_ERR_NOTEXIST;
	   VOS_MemCpy(parameter,&dynamic_cfg,sizeof(GPONADP_ONU_DynamicMcast_Prof_t));
    	}
     OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetMulticastTemplate(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
static int GPONONU_BCM68620_SetMcastOperProfile(short int olt_id, short int onu_id, int prof,CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
   int iRlt = OLT_ERR_NOTEXIST;
   short int CTC_McastOper_Prof_Structlen = 0;
   short int GPON_McastOper_Prof_Structlen = 1;
   GPONADP_ONU_McastOper_Prof_t cfg;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
   CTC_McastOper_Prof_Structlen = sizeof(CTC_GPONADP_ONU_McastOper_Prof_t);
   GPON_McastOper_Prof_Structlen = sizeof(GPONADP_ONU_McastOper_Prof_t);
   if(CTC_McastOper_Prof_Structlen != GPON_McastOper_Prof_Structlen )
		return OLT_ERR_NOTEXIST;
   
   VOS_MemSet( &cfg,0,GPON_McastOper_Prof_Structlen);
   VOS_MemCpy(&cfg,parameter,GPON_McastOper_Prof_Structlen);
  iRlt =  gponOnuAdp_McastOperProfSet(prof,cfg);

  OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetMcastOperProfile(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
static int GPONONU_BCM68620_GetMcastOperProfile(short int olt_id, short int onu_id, int prof,CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
   int iRlt = OLT_ERR_NOTEXIST;
   short int CTC_McastOper_Prof_Structlen = 0;
   short int GPON_McastOper_Prof_Structlen = 1;
   GPONADP_ONU_McastOper_Prof_t cfg;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
   VOS_ASSERT(parameter);

   CTC_McastOper_Prof_Structlen = sizeof(CTC_GPONADP_ONU_McastOper_Prof_t);
   GPON_McastOper_Prof_Structlen = sizeof(GPONADP_ONU_McastOper_Prof_t);
   if(CTC_McastOper_Prof_Structlen != GPON_McastOper_Prof_Structlen )
		return OLT_ERR_NOTEXIST;
   
   VOS_MemSet( &cfg,0,GPON_McastOper_Prof_Structlen);
    iRlt =  gponOnuAdp_McastOperProfGet(prof,&cfg);
   if(iRlt != 0)
	  	return OLT_ERR_NOTEXIST;
    VOS_MemCpy(parameter,&cfg,GPON_McastOper_Prof_Structlen);
	
  OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetMcastOperProfile(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
    return iRlt;
}
static int GPONONU_BCM68620_SetUniPortAssociateMcastProf(short int olt_id, short int onu_id,short int port_id,unsigned char stateflag,int profIdx)
{
	 int iRlt = OLT_ERR_NOTEXIST;
	  OLT_LOCAL_ASSERT(olt_id);
	 ONU_ASSERT(onu_id);

	 if(stateflag == 0)
 	{
 	      iRlt = gponOnuAdp_SetUniPortAssociateStaticMcastProf(olt_id,  onu_id, port_id, profIdx);
	}
	 else if(stateflag == 1)
	 {
	 	iRlt = gponOnuAdp_SetUniPortAssociateDynamicMcastProf(olt_id, onu_id,  port_id, profIdx);
	 }
	 else if(stateflag == 2)
 	{
 		iRlt = gponOnuAdp_SetUniPortAssociateMcastOperProf(olt_id, onu_id, port_id, profIdx);
 	}
	 else
	 	return OLT_ERR_NOTEXIST;
	 
	 OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetUniPortAssociateMcastProf(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	   return iRlt;

}
static int GPONONU_BCM68620_GetUniPortAssociateMcastProf(short int olt_id, short int onu_id,short int port_id,CTC_GPONADP_ONU_Profile_t *ProfIdx)
{
	 int iRlt = OLT_ERR_NOTEXIST;
	 int staticProfIdx, dynamicProfIdx, operProfIdx;
	 
	 OLT_LOCAL_ASSERT(olt_id);
	 ONU_ASSERT(onu_id);
	 VOS_ASSERT(ProfIdx);
	
 	  iRlt = gponOnuAdp_GetUniPortAssociateMcastProfIdx(olt_id, onu_id, port_id, &staticProfIdx, &dynamicProfIdx, &operProfIdx);
	  if(iRlt == 0)
	  	{
	  		 ProfIdx->dynamicProfIdx = dynamicProfIdx;
			 ProfIdx->staticProfIdx = staticProfIdx;
			 ProfIdx->operProfIdx = operProfIdx;
	  	}
	 
	 OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetUniPortAssociateMcastProf(%d, %d)'s result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, SYS_LOCAL_MODULE_SLOTNO);
	   return iRlt;

}

static int GPONONU_BCM68620_SetOnuMacLimit(short int olt_id, short int onu_id, int mac_limit)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int uiSetVal = mac_limit;
	short int llid;
    GPONOnuConfig onuConfig;
    unsigned int port_num;
    
    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    llid = GetLlidActivatedByOnuIdx(olt_id, onu_id);
	if ( INVALID_LLID != llid )
    {
        if (uiSetVal > GPONADP_ONU_MACLIMIT_MAX)
        {            
            OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuMacLimit(%d, %d)'s "
                "err: max mac %d greater than 255 on slot %d.\r\n", olt_id, onu_id, 
                uiSetVal, SYS_LOCAL_MODULE_SLOTNO);    
            return OLT_ERR_PARAM;
        }

        iRlt = gponOnuAdp_SetMacLimit(olt_id, llid, uiSetVal);
        if (iRlt != OLT_ERR_OK)
        {
            /* not return */
            OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuMacLimit(%d, %d)'s err: "
                "SetMacLimit result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, 
                SYS_LOCAL_MODULE_SLOTNO);    
        }

        /* 设备级配置端口级生效，以规避ONU设备级mac限制不生效问题*/
        memset(&onuConfig, 0, sizeof(onuConfig));

        iRlt = gponOnuAdp_GetOnuCfg(olt_id, llid, &onuConfig);
        if (iRlt == OLT_ERR_OK)
        {
            for (port_num = 1; port_num <= onuConfig.totalEthernetUNINumber; port_num++)
            {
                (void)gponOnuAdp_SetUniPortMacLimit(olt_id, llid, port_num, uiSetVal);   
            }            
        }        
    }  
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }
    
    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_SetOnuMacLimit(%d, %d)'s "
        "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, 
        SYS_LOCAL_MODULE_SLOTNO);
        
    return iRlt; 			
}

static int GPONONU_BCM68620_GetOnuMacLimit(short int olt_id, short int onu_id, int *mac_limit)
{
    int iRlt = OLT_ERR_NOTEXIST;
    GPONOnuConfig onuConfig;
    short int llid;

    OLT_LOCAL_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	llid = GetLlidActivatedByOnuIdx(olt_id, onu_id);
	if ( INVALID_LLID != llid )
    {
        memset(&onuConfig, 0, sizeof(onuConfig));

        iRlt = gponOnuAdp_GetOnuCfg(olt_id, llid, &onuConfig);
        if (iRlt == OLT_ERR_OK)
        {
            *mac_limit = onuConfig.macLimit;
        }
    }
    else
    {
        iRlt = OLT_ERR_NOTEXIST;
    }

    OLT_BCM_DEBUG(OLT_BCM_TITLE"GPONONU_BCM68620_GetOnuMacLimit(%d, %d)'s "
        "result(%d) on slot %d.\r\n", olt_id, onu_id, iRlt, 
        SYS_LOCAL_MODULE_SLOTNO);
    
    return iRlt; 			
}

static int GPONONU_BCM_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int en)
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


static int GPONONU_BCM_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
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

static int GPONONU_BCM_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
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


static int GPONONU_BCM_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
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

static int GPONONU_BCM_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
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



static int GPONONU_BCM_SetPortDefaultPriority(short int olt_id, short int onu_id, int port_id, int priority)
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


static int GPONONU_BCM_SetIgmpEnable(short int olt_id, short int onu_id, int en)
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

static int GPONONU_BCM_SetIgmpAuth(short int olt_id, short int onu_id, int en)
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
#define ONU_REGISTER_DEBUG  if(EVENT_REGISTER == V2R1_ENABLE )sys_console_printf

extern ULONG ulGPON_GEMPORT_BASE;
int GPONONU_GetDeviceInformation(short int PonPortIdx, short int OnuIdx)
{
    int ret = VOS_ERROR;
    GPONOnuConfig *onuConfig = NULL;
    int OnuEntry = 0;
	int gemvid[64] = {0};
	short int buf_len = 0;
	char buf[128] = {0};
	ULONG gemvidnum = 0;	
	int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	char series_number[GPON_ONU_SERIAL_NUM_STR_LEN];
	UCHAR SerialNoAfterSIXDigit[BYTES_IN_MAC_ADDRESS];
	UCHAR series_tmp[BYTES_IN_MAC_ADDRESS];
	UCHAR icount;
	char equipmentID[MAXEQUIPMENTIDLEN];
	char pModel[80]="";
	ULONG pLen=0;
	ULONG typeid=0;
    CHECK_ONU_RANGE;
    
	#if 1
    onuConfig = VOS_Malloc(sizeof(GPONOnuConfig), MODULE_ONU);
    if(!onuConfig)
        return ret;
    
    VOS_MemZero(onuConfig, sizeof(GPONOnuConfig));
    if(VOS_OK == (ret = gponOnuAdp_GetOnuCfg(PonPortIdx, llid, onuConfig)))
    {
        OnuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
        ONU_MGMT_SEM_TAKE;
        buf_len = VOS_StrLen(onuConfig->vendorID)/2; 
        if(buf_len && VOS_OK == VOS_ConvertHexAsciiStrToBinOctets(onuConfig->vendorID, buf, &buf_len))
        {
            VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.VendorInfo, buf, buf_len);            
            OnuMgmtTable[OnuEntry].DeviceInfo.VendorInfo[buf_len] = '\0';
            OnuMgmtTable[OnuEntry].DeviceInfo.VendorInfoLen = buf_len;  
            VOS_MemCpy(OnuMgmtTable[OnuEntry].device_vendor_id, buf, 4);            
        }
        ONU_REGISTER_DEBUG("VendorInfo(%s)(%s)\r\n", onuConfig->vendorID, buf);

        if(onuConfig->onuImageInstance0Valid)
        {        
            OnuMgmtTable[OnuEntry].ImageValidIndex = 1;
        }
        else if(onuConfig->onuImageInstance1Valid)
        {
            OnuMgmtTable[OnuEntry].ImageValidIndex = 2;        
        }

        buf_len = VOS_StrLen(onuConfig->onuImageInstance0Version)/2; 
        if(buf_len && VOS_OK == VOS_ConvertHexAsciiStrToBinOctets(onuConfig->onuImageInstance0Version, buf, &buf_len))
        {       
            VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion, buf, buf_len);
            OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen = 29;                        
            OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion[buf_len] = '\0';
        }               
        ONU_REGISTER_DEBUG("onuImageInstance0Version(%s)(%s), valid = %d\r\n", onuConfig->onuImageInstance0Version, buf, onuConfig->onuImageInstance0Valid);

        buf_len = VOS_StrLen(onuConfig->onuImageInstance1Version)/2; 
        if(buf_len && VOS_OK == VOS_ConvertHexAsciiStrToBinOctets(onuConfig->onuImageInstance1Version, buf, &buf_len))
        {       
            VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion1, buf, buf_len);
            OnuMgmtTable[OnuEntry].DeviceInfo.SwVersionLen = 29;                        
            OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion1[buf_len] = '\0';
        }  
        ONU_REGISTER_DEBUG("onuImageInstance1Version(%s)(%s), valid = %d\r\n", onuConfig->onuImageInstance1Version, buf, onuConfig->onuImageInstance1Valid);

        VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.equipmentID, onuConfig->equipmentID, MAXEQUIPMENTIDLEN);
        OnuMgmtTable[OnuEntry].DeviceInfo.equipmentIDLen = MAXEQUIPMENTIDLEN;
        ONU_REGISTER_DEBUG("equipmentID(%s)\r\n", onuConfig->equipmentID);

        VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.DevicePassward, onuConfig->password, MAXDEVICEPWDLEN);
        OnuMgmtTable[OnuEntry].DeviceInfo.DevicePasswardLen = MAXDEVICEPWDLEN;
        ONU_REGISTER_DEBUG("passward(%s)\r\n", onuConfig->password);

        OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number = onuConfig->totalEthernetUNINumber;
        OnuMgmtTable[OnuEntry].POTS_ports_number = onuConfig->totalPOTSUNInumber;
        OnuMgmtTable[OnuEntry].TcontNum = onuConfig->totalTCONTNumber;
        OnuMgmtTable[OnuEntry].GEMPortNum = onuConfig->totalGEMPortNumber;
        OnuMgmtTable[OnuEntry].OmccVersion = onuConfig->omccVersion;
        OnuMgmtTable[OnuEntry].PmEnable = onuConfig->enablePm;	  		
        OnuMgmtTable[OnuEntry].GemPortId = ulGPON_GEMPORT_BASE+(llid-1)*16;
        OnuMgmtTable[OnuEntry].BerInterval = onuConfig->berIntervalConfiguration;
        ONU_REGISTER_DEBUG("FE_Ethernet_ports_number(%d), POTS_ports_number(%d)\r\n", onuConfig->totalEthernetUNINumber, onuConfig->totalPOTSUNInumber);
        ONU_REGISTER_DEBUG("TcontNum(%d), GEMPortNum(%d)\r\n", onuConfig->totalTCONTNumber, onuConfig->totalGEMPortNumber);
        
        OnuMgmtTable[OnuEntry].ONUMeteringTable.recvOpticalPower = onuConfig->rxpower;
        OnuMgmtTable[OnuEntry].ONUMeteringTable.transOpticalPower = onuConfig->txpower;
        OnuMgmtTable[OnuEntry].RTT = onuConfig->distance;        
        ONU_REGISTER_DEBUG("recvOpticalPower(%d), transOpticalPower(%d), RTT(%d)\r\n", onuConfig->rxpower, onuConfig->txpower, onuConfig->distance);
        
		
		if(MAC_ADDR_IS_ZERO(onuConfig->onuMacAddress))
		{
			VOS_StrnCpy(series_number, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN-1);
			VOS_MemCpy(equipmentID, OnuMgmtTable[OnuEntry].DeviceInfo.equipmentID, MAXEQUIPMENTIDLEN);
			GPON_OnuModel_Translate(equipmentID, &typeid, pModel, &pLen);

			if( typeid == V2R1_ONU_GPON ) /*其他厂商onu*/
			{
				for(icount = 0;icount < 6;icount += 1)
				{
					series_number[GPON_ONU_SERIAL_NUM_STR_LEN-1]=0;
					if(series_number[10+icount] >= 'A' && series_number[10+icount] <= 'F')
						SerialNoAfterSIXDigit[icount] = series_number[10+icount]-'A' + 10;
					else if(series_number[10+icount] >= 'a' && series_number[10+icount] <= 'f')
						SerialNoAfterSIXDigit[icount] = series_number[10+icount]-'a' + 10;
					else
						SerialNoAfterSIXDigit[icount] = series_number[10+icount] - '0';
				 }
				VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, SerialNoAfterSIXDigit,BYTES_IN_MAC_ADDRESS);
			}
			else
			{
				SerialNoAfterSIXDigit[0]=0x00;
				SerialNoAfterSIXDigit[1]=0x0F;
				SerialNoAfterSIXDigit[2]=0xE9;
				for(icount = 0;icount < 6;icount += 1)
		  		{
					series_number[GPON_ONU_SERIAL_NUM_STR_LEN-1]=0;
					if(series_number[10+icount] >= 'A' && series_number[10+icount] <= 'F')
						series_tmp[icount] = series_number[10+icount]-'A' + 10;
					else if(series_number[10+icount] >= 'a' && series_number[10+icount] <= 'f')
						series_tmp[icount] = series_number[10+icount]-'a' + 10;
					else
						series_tmp[icount] = series_number[10+icount] - '0';
		    	}
				
				SerialNoAfterSIXDigit[3] = ((series_tmp[0]) << 4) | series_tmp[1];	
				SerialNoAfterSIXDigit[4] = ((series_tmp[2]) << 4) | series_tmp[3];
				SerialNoAfterSIXDigit[5] = ((series_tmp[4]) << 4) | series_tmp[5];

				VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, SerialNoAfterSIXDigit,BYTES_IN_MAC_ADDRESS);
			}
		}
		else
		{
			VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, onuConfig->onuMacAddress, 6);
		}

        ONU_MGMT_SEM_GIVE;    
    }
    
    if(onuConfig)
    {
        VOS_Free(onuConfig);
        onuConfig = NULL;
    }
	#else
	OnuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number = 1;
    OnuMgmtTable[OnuEntry].POTS_ports_number = 0;
    OnuMgmtTable[OnuEntry].TcontNum = 16;
    OnuMgmtTable[OnuEntry].GEMPortNum = 16;
   		
    OnuMgmtTable[OnuEntry].GemPortId = 200+(llid-1)*16;
	ONU_MGMT_SEM_GIVE;  
	return VOS_OK;
	#endif
    return ret;
}


static int GPONONU_BCM_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
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
            iRlt = PON_CTC_STACK_clear_multicast_vlan(olt_id, onu_id, (unsigned char)port_id);
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
                    
                   PON_CTC_STACK_clear_multicast_vlan(olt_id, onu_id, (unsigned char)port_id);

                   mv.vlan_operation = CTC_MULTICAST_VLAN_OPER_ADD;
                   iRlt = PON_CTC_STACK_set_multicast_vlan(olt_id, onu_id, (unsigned char)port_id, mv);
                }
            }
        }
    }


    return iRlt;
}

#endif

#if 0
/*----------------OMCI----------------*/
int GPONONU_BCM68620_GetOnuCfg(int olt_id, int onu_id, tGponOnuConfig *onuConfig)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
	    VOS_MemZero(onuConfig, sizeof(tOgCmOnuConfig));
    
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_VENDOR_ID;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_VERSION;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_EQUIPMENT_ID;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_TOTAL_GEM_PORT_NUMBER;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_TOTAL_TCONT_NUMBER;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_TOTAL_ETHERNET_UNI_NUMBER;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_TOTAL_POTS_UNI_NUMBER;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_MAC_ADDRESS;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_RX_POWER;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_TX_POWER;            
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_IMAGE_INSTANCE0_VERSION; 
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_IMAGE_INSTANCE0_VALID; 
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_IMAGE_INSTANCE1_VERSION; 
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ONU_IMAGE_INSTANCE1_VALID; 

	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_BATTERY_BACKUP;    	
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_PASSWORD;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_OMCC_VERSION;
	    onuConfig->bitMask1 |= fCMAPI_ONU_CONFIG_MASK_ENABLE_PM;
	    
	    onuConfig->bitMask2 |= fCMAPI_ONU_CONFIG_MASK_DISTANCE;

        iRlt = gponOnuAdp_GetOnuCfg(olt_id, llid, onuConfig);
    }
    return iRlt;
}

int GPONONU_BCM68620_SetOnuCfg(int olt_id, int onu_id, tOgCmOnuConfig *onuConfig)
{
    int iRlt = OLT_ERR_NOTEXIST;
    short int llid = 0;

    if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onu_id)) )
    {
        iRlt = gponOnuAdp_SetOnuCfg(olt_id, llid, *onuConfig);
    }
    return iRlt;
}

#endif

/* --------------------END------------------------ */

/******************************************外部接口***************************************/
static const OnuMgmtIFs s_gpononuIfs = {
#if 1
/* -------------------ONU基本API------------------- */
    GPONONU_OnuIsValid,
    GPONONU_BCM68620_OnuIsOnline,      /*GWONU_BCM55538_OnuIsOnline,*/
    REMOTE_OK,      /*GWONU_BCM55538_AddOnuByManual,*/
    REMOTE_OK,      /*ModifyOnuByManual*/    
    GPONONU_BCM68620_DelOnuByManual,      /*GWONU_BCM55538_DelOnuByManual,*/
    GPONONU_BCM68620_AddOnuByManual,
    GWONU_CmdIsSupported,

    GWONU_CopyOnu,
	REMOTE_OK,      /*CTCONU_BCM55538_GetIFType,*/
	GWONU_SetIFType,/* SetIFType */
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    GPONONU_BCM68620_DeregisterOnu,      /*GWONU_BCM55538_DeregisterOnu,*/
    REMOTE_OK,      /* SetMacAuthMode */
    REMOTE_OK,      /* DelBindingOnu */
#if 0
    REMOTE_OK,      /* AddPendingOnu */
    REMOTE_OK,      /* DelPendingOnu */
    REMOTE_OK,      /* DelConfPendingOnu */
#endif
    GPONONU_BCM68620_AuthorizeOnu,      /*GWONU_BCM55538_AuthorizeOnu,*/
    GPONONU_BCM68620_AuthRequest,      /*CTCONU_BCM55538_AuthRequest,*/
    GPONONU_BCM68620_AuthSuccess,      /*CTCONU_BCM55538_AuthSuccess,*/
    GPONONU_BCM68620_AuthFailure,      /*CTCONU_BCM55538_AuthFailure,*/
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    REMOTE_OK,      /*CTCONU_BCM55538_SetOnuTrafficServiceMode,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuPeerToPeer,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuPeerToPeerForward,*/
    GPONONU_BCM68620_SetOnuBW,      /*GWONU_BCM55538_SetOnuBW,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuSLA,*/

    GPONONU_BCM68620_SetOnuFecMode,      /*CTCONU_BCM55538_SetOnuFecMode,*/
    REMOTE_OK,           /* GetOnuVlanMode */
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    REMOTE_OK,      /*GWONU_BCM55538_SetUniPort,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetSlowProtocolLimit,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetSlowProtocolLimit,*/
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

   	REMOTE_OK,      /*GWONU_GetBWInfo,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuB2PMode,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuB2PMode,*/
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    REMOTE_OK,      /*GWONU_BCM55538_ResetCounters,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetPonLoopback,*/
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    REMOTE_OK,      /*GWONU_BCM55538_GetLLIDParams,*/
    REMOTE_OK,      /*GWONU_BCM55538_StartEncryption,*/
    REMOTE_OK,      /*GWONU_BCM55538_StopEncryption,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuEncryptParams,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuEncryptParams,*/
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    NULL,           /*UpdateEncryptionKey*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuMacAddrTbl,*/
    GPONONU_BCM68620_GetOltMacAddrTbl,      /*GetOltMacAddrTbl,*/
    GPONONU_BCM68620_GetOltMacAddrVlanTbl,      /*GetOltMacAddrVlanTbl,*/
    GPONONU_BCM68620_SetOnuMaxMacNum,      /*GWONU_BCM55538_SetOnuMaxMacNum,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuUniMacCfg,*/
    REMOTE_OK,      /*GWONU_GetOnuMacCheckFlag,*/
    REMOTE_OK,      /*GWONU_GetAllEthPortMacCounter,*/
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    GPONONU_BCM68620_GetOnuDistance,      /*GWONU_BCM55538_GetOnuDistance,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOpticalCapability,*/
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuLLID,*/
#endif

#if 1
/* -------------------ONU 设备管理API------------------- */
    NULL,           /* GetOnuVer */
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuPonVer,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuRegisterInfo,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetOnuI2CInfo,*/
    REMOTE_OK,      /*GWONU_BCM55538_SetOnuI2CInfo,*/
    
    GPONONU_BCM68620_ResetOnu,      /*CTCONU_BCM55538_ResetOnu,*/
    REMOTE_OK,      /* SetOnuSWUpdateMode */
    GPONONU_OnuSwUpdate,       /*CTCONU_OnuSwUpdate,*/
    REMOTE_OK,      /*GWONU_OnuGwCtcSwConvert,*/
    REMOTE_OK,      /*GWONU_BCM55538_GetBurnImageComplete,*/
    
    /*CTCONU_BCM55538_SetOnuDeviceName, cortina onu act as gwd onu for this variable 2011-10-19*/
    REMOTE_OK,      /*GWONU_SetOnuDeviceName,*/
    REMOTE_OK,      /* SetOnuDeviceDesc */
    REMOTE_OK,      /* SetOnuDeviceLocation */
    REMOTE_OK,      /*GWONU_GetOnuAllPortStatisticData,*/
#endif

#if 1
/* --------------------ONU CTC-PROTOCOL API------------------- */
    REMOTE_OK,       /*CTCONU_BCM55538_GetCtcVersion,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetFirmwareVersion,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetSerialNumber,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetChipsetID,*/

    REMOTE_OK,       /*CTCONU_BCM55538_GetOnuCap1,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetOnuCap2,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetOnuCap3,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_UpdateOnuFirmware,*/
    REMOTE_OK,       /*CTCONU_BCM55538_ActiveOnuFirmware,*/
    REMOTE_OK,       /*CTCONU_BCM55538_CommitOnuFirmware,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_StartEncrypt,*/
    REMOTE_OK,       /*CTCONU_BCM55538_StopEncrypt,*/
    
    GPONONU_BCM68620_GetEthPortLinkstate,       /*CTCONU_BCM55538_GetEthPortLinkstate,*/
    GPONONU_BCM68620_GetEthPortAdminstatus,       /*CTCONU_BCM55538_GetEthPortAdminstatus,*/
    GPONONU_BCM68620_SetEthPortAdminstatus,       /*CTCONU_BCM55538_SetEthPortAdminstatus,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortPauseEnable,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortPauseEnable,*/
    
    GPONONU_BCM68620_GetEthPortAutoNegotinationAdmin,       /*CTCONU_BCM55538_GetEthPortAutoNegotinationAdmin,*/
    GPONONU_BCM68620_SetEthPortAutoNegotinationAdmin,       /*CTCONU_BCM55538_SetEthPortAutoNegotinationAdmin,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortRestartAutoConfig,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortAnLocalTecAbility,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortAnAdvertisedTecAbility,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortPolicing,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortPolicing,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortDownstreamPolicing,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortDownstreamPolicing,*/

    GPONONU_BCM68620_GetEthPortVlanConfig,       /*CTCONU_BCM55538_GetEthPortVlanConfig,*/
    GPONONU_BCM68620_SetEthPortVlanConfig,       /*CTCONU_BCM55538_SetEthPortVlanConfig,*/
    REMOTE_ERROR,       /*CTCONU_BCM55538_GetAllPortVlanConfig,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortClassificationAndMarking,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortClassificationAndMarking,*/
    REMOTE_OK,       /*CTCONU_BCM55538_ClearEthPortClassificationAndMarking,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortMulticastVlan,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortMulticastVlan,*/
    REMOTE_OK,       /*CTCONU_BCM55538_ClearEthPortMulticastVlan,*/

    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortMulticastGroupMaxNumber,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortMulticastGroupMaxNumber,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortMulticastTagStrip,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortMulticastTagStrip,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetMulticastAllPortTagStrip,*/

    REMOTE_OK,       /*CTCONU_BCM55538_GetEthPortMulticastTagOper,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetEthPortMulticastTagOper,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetObjMulticastTagOper,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetMulticastControl,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetMulticastControl,*/
    REMOTE_OK,       /*CTCONU_BCM55538_ClearMulticastControl,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetMulticastSwitch,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetMulticastSwitch,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_GetFastLeaveAbility,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetFastLeaveAdminState,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetFastLeaveAdminState,*/
    
    GPONONU_BCM68620_GetOnuPortStatisticData,       /*CTCONU_BCM55538_GetOnuPortStatisticData,*/
    GPONONU_BCM68620_GetOnuPortStatisticState,       /*CTCONU_BCM55538_GetOnuPortStatisticState,*/
    GPONONU_BCM68620_SetOnuPortStatisticState,       /*CTCONU_BCM55538_SetOnuPortStatisticState,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_SetAlarmAdminState,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetAlarmThreshold,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetDbaReportThresholds,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetDbaReportThresholds,*/
    
    CTCONU_BCM68620_GetMxuMngGlobalConfig,       /*CTCONU_BCM55538_GetMxuMngGlobalConfig,*/
    CTCONU_BCM68620_SetMxuMngGlobalConfig,       /*CTCONU_BCM55538_SetMxuMngGlobalConfig,*/
    REMOTE_OK,       /*CTCONU_BCM55538_GetMxuMngSnmpConfig,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetMxuMngSnmpConfig,*/

    REMOTE_OK,       /*CTCONU_BCM55538_GetHoldover,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetHoldover,*/
    GPONONU_BCM68620_GetOptTransDiag,/*GPONONU_BCM68620_GetOptTransDiag,*/
    GPONONU_BCM68620_SetTxPowerSupplyControl,/*CTCONU_BCM55538_SetTxPowerSupplyControl,*/

    REMOTE_OK,       /*CTCONU_BCM55538_GetFecAbility,*/

    NULL,       /*CTCONU_BCM55538_GetIADInfo,*/
    NULL,       /*CTCONU_BCM55538_GetVoipIadOperStatus,*/
    NULL,       /*CTCONU_BCM55538_SetVoipIadOperation,*/
    NULL,       /*CTCONU_BCM55538_GetVoipGlobalConfig,*/
    NULL,       /*CTCONU_BCM55538_SetVoipGlobalConfig,*/
    NULL,       /*CTCONU_BCM55538_GetVoipFaxConfig,*/
    NULL,       /*CTCONU_BCM55538_SetVoipFaxConfig,*/
    
    NULL,       /*CTCONU_BCM55538_GetVoipPortStatus,*/
    NULL,       /*CTCONU_BCM55538_GetVoipPort,*/
    NULL,       /*CTCONU_BCM55538_SetVoipPort,*/
    NULL,       /*CTCONU_BCM55538_GetVoipPort2,*/
    NULL,       /*CTCONU_BCM55538_SetVoipPort2,*/
    
    NULL,       /*CTCONU_BCM55538_GetH248Config,*/
    NULL,       /*CTCONU_BCM55538_SetH248Config,*/
    NULL,       /*CTCONU_BCM55538_GetH248UserTidConfig,*/
    NULL,       /*CTCONU_BCM55538_SetH248UserTidConfig,*/
    NULL,       /*CTCONU_BCM55538_GetH248RtpTidConfig,*/
    NULL,       /*CTCONU_BCM55538_SetH248RtpTidConfig,*/
    
    NULL,       /*CTCONU_BCM55538_GetSipConfig,*/
    NULL,       /*CTCONU_BCM55538_SetSipConfig,*/
    NULL,       /*CTCONU_BCM55538_SetSipDigitMap,*/
    NULL,       /*CTCONU_BCM55538_GetSipUserConfig,*/
    NULL,       /*CTCONU_BCM55538_SetSipUserConfig,*/
    NULL,       /*CTCONU_Onustats_GetOnuPortDataByID,*/
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    NULL,       /* CliCall */

    GPONONU_BCM68620_ResetOnu,       /*CTCONU_BCM55538_ResetOnu,*/
    NULL,       /*GWONU_SetMgtConfig,*/
    NULL,       /*GWONU_SetMgtLaser,*/
    NULL,       /*GWONU_SetTemperature,*/
    NULL,           /* SetPasFlush */
    
    NULL,       /*GWONU_SetAtuAgingTime,*/
    NULL,       /*GWONU_SetAtuLimit,*/
    
    NULL,       /*GWONU_SetPortLinkMon,*/
    NULL,       /*GWONU_SetPortModeMon,*/
    NULL,       /*GWONU_SetPortIsolate,*/

    GPONONU_BCM68620_SetVlanEnable,       /*CTCONU_BCM55538_SetVlanEnable,*/
    GPONONU_BCM68620_SetVlanMode,       /*CTCONU_BCM55538_SetVlanMode,*/
    REMOTE_OK,     /* AddVlan */
    GPONONU_BCM68620_DelVlan,       /*CTCONU_BCM55538_DelVlan,*/
    GPONONU_BCM68620_SetPortPvid,       /*CTCONU_BCM55538_SetPortPvid,*/

    GPONONU_BCM68620_AddVlanPort,       /*CTCONU_BCM55538_AddVlanPort,*/
    GPONONU_BCM68620_DelVlanPort,       /*CTCONU_BCM55538_DelVlanPort,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetVlanTran,*/
    REMOTE_OK,       /*CTCONU_BCM55538_DelVlanTran,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetVlanAgg,*/
    REMOTE_OK,       /*CTCONU_BCM55538_DelVlanAgg,*/
    
    GPONONU_BCM68620_SetPortQinQEnable,       /*CTCONU_BCM55538_SetPortQinQEnable,*/
    GPONONU_BCM68620_AddQinQVlanTag,       /*CTCONU_BCM55538_AddQinQVlanTag,*/
    GPONONU_BCM68620_DelQinQVlanTag,       /*CTCONU_BCM55538_DelQinQVlanTag,*/

    REMOTE_OK,       /*GWONU_SetPortVlanFrameTypeAcc,*/
    REMOTE_OK,       /*GWONU_SetPortIngressVlanFilter,*/
    
    GPONONU_BCM68620_SetPortMode,       /*CTCONU_BCM55538_SetPortMode,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetPortFcMode,*/
    REMOTE_OK,       /*GWONU_SetPortAtuLearn,*/
    REMOTE_OK,       /*GWONU_SetPortAtuFlood,*/
    REMOTE_OK,       /*GWONU_SetPortLoopDetect,*/
    REMOTE_OK,       /*GWONU_SetPortStatFlush,*/
    
    REMOTE_OK,       /*GWONU_SetIngressRateLimitBase,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetPortIngressRate,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetPortEgressRate,*/
    
    REMOTE_OK/*GWONU_SetQosClass*/,
    REMOTE_OK/*GWONU_ClrQosClass*/,
    REMOTE_OK/*GWONU_SetQosRule*/,
    REMOTE_OK/*GWONU_ClrQosRule*/,
    
    NULL,       /*CTCONU_BCM55538_SetPortQosRule,*/
    NULL,       /*CTCONU_BCM55538_ClrPortQosRule,*/
    NULL,       /*GWONU_SetPortQosRuleType,*/
    
    NULL,       /*CTCONU_BCM55538_SetPortDefaultPriority,*/
    NULL,       /*GWONU_SetPortNewPriority,*/
    NULL,       /*GWONU_SetQosPrioToQueue,*/
    NULL,       /*GWONU_SetQosDscpToQueue,*/
    
    NULL,       /*GWONU_SetPortUserPriorityEnable,*/
    NULL,       /*GWONU_SetPortIpPriorityEnable,*/
    NULL,       /*GWONU_SetQosAlgorithm,*/
    NULL,       /*GWONU_SET_QosMode,*/
    NULL,       /*GWONU_SET_Rule,*/
    
    REMOTE_OK,       /*CTCONU_BCM55538_SetIgmpEnable,*/
    REMOTE_OK,       /*CTCONU_BCM55538_SetIgmpAuth,*/
    NULL,       /*GWONU_SetIgmpHostAge,*/
    NULL,       /*GWONU_SetIgmpGroupAge,*/
    NULL,       /*GWONU_SetIgmpMaxResTime,*/
    
    NULL,       /*GWONU_SetIgmpMaxGroup,*/
    NULL,       /*GWONU_AddIgmpGroup,*/
    NULL,       /*GWONU_DeleteIgmpGroup,*/
    NULL,       /*GWONU_SetPortIgmpFastLeave,*/
    NULL,       /*CTCONU_BCM55538_SetPortMulticastVlan,*/

    NULL,       /*GWONU_SetPortMirrorFrom,*/
    NULL,       /*GWONU_SetPortMirrorTo,*/
    NULL,       /*GWONU_DeleteMirror,*/
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
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif
GPONONU_BCM68620_SetMulticastTemplate, 
GPONONU_BCM68620_GetMulticastTemplate,
GPONONU_BCM68620_SetMcastOperProfile,
GPONONU_BCM68620_GetMcastOperProfile,
GPONONU_BCM68620_SetUniPortAssociateMcastProf,
GPONONU_BCM68620_GetUniPortAssociateMcastProf,

/* --------------------END------------------------ */
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
/*--------------------GPON OMCI 管理API------------------------------*/
   NULL, /*GPONONU_BCM68620_GetOnuCfg,*/
   NULL, /*GPONONU_BCM68620_SetOnuCfg,*/
#endif

    REMOTE_ERROR
};



void GPON_BCM_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_BCM_GPON, &s_gpononuIfs);
}



#ifdef __cplusplus

}

#endif


/*#endif*/

