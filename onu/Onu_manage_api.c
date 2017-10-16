#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "onuConfMgt.h"
#include  "V2R1_product.h"
#include  "ponEventHandler.h"
#include  "ct_manage/CT_RMan_EthPort.h"
#include  "Onu_manage.h"
#include  "Onu_oam_comm.h"
ULONG OnuEventCallBackfuctionSemId = 0;
static OnuEventList_t s_OnuEvent_FunctionList = {0,0,0};
OnuEventList_t *g_OnuEvent_FunctionList = &s_OnuEvent_FunctionList;
char *g_onu_oper_status_str[] = 
{
    "NULL",
    "UP",
    "DOWN",
    "PENDING",
    "DORMANT",
    "POWERDOWN"
};
char *g_onu_register_status_str[] =
{
    "IDLE",
    "DISCOVERY",
    "IN_PROGRESS",
    "FINISH",
    "FAIL"
};

extern int VOS_ConvertHexAsciiStrToBinOctets(char *strP, char *bufP, short int *bufLenP);
#if 1
/*----------------------GPON support API -------------------*/

int OnuEvent_ShowCapabilities(struct vty *vty, short int PonPortIdx, short int OnuIdx)
{
    int ret = VOS_ERROR;
    int OnuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
    CHECK_ONU_RANGE;

    ONU_MGMT_SEM_TAKE;    
    vty_out( vty, "ONU Capabilities:\r\n" );
    vty_out( vty, "  Number of POTS ports   : %d\r\n", OnuMgmtTable[OnuEntry].POTS_ports_number);  
    vty_out( vty, "  Number of ETH ports    : %d\r\n", OnuMgmtTable[OnuEntry].FE_Ethernet_ports_number);  
    vty_out( vty, "  Number of GEM ports    : %d\r\n", OnuMgmtTable[OnuEntry].GEMPortNum);  
    vty_out( vty, "  Number of T-CONTs      : %d\r\n", OnuMgmtTable[OnuEntry].TcontNum);  
    vty_out( vty, "  Battery backup         : %s\r\n", "Not support");        
    ONU_MGMT_SEM_GIVE; 

    vty_out( vty, "\r\n");        
    return ret;
}

int OnuEvent_ShowDeviceInfomation(struct vty *vty, short int PonPortIdx, short int OnuIdx)
{
    int ret = VOS_ERROR;
    int OnuEntry = PonPortIdx*MAXONUPERPON+OnuIdx;
	short int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
	
    CHECK_ONU_RANGE;
    ONU_MGMT_SEM_TAKE;    
    {
        vty_out( vty, "Device information:\r\n" );

        vty_out( vty, "  Serial_number/Passward        : %s/%s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, OnuMgmtTable[OnuEntry].DeviceInfo.DevicePassward);
        vty_out( vty, "  Mac address                   : %02x%02x:%02x%02x:%02x%02x\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[0], OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[1],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[2],
        OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[3],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[4],OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr[5]);  
        vty_out( vty, "  Equipment-ID                  : %s\r\n", OnuMgmtTable[OnuEntry].DeviceInfo.equipmentID);
        vty_out( vty, "  Vendor-ID                     : %c%c%c%c\r\n", OnuMgmtTable[OnuEntry].device_vendor_id[0], OnuMgmtTable[OnuEntry].device_vendor_id[1],
        OnuMgmtTable[OnuEntry].device_vendor_id[2],OnuMgmtTable[OnuEntry].device_vendor_id[3]);
        vty_out( vty, "  OMCC Version                  : %02x\r\n", OnuMgmtTable[OnuEntry].OmccVersion);
        
        vty_out( vty, "  Main Software Version         : %s(%s)\r\n", VOS_StrLen(OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion)?OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion:"-", OnuMgmtTable[OnuEntry].ImageValidIndex == 1?"valid":"invalid");  
        vty_out( vty, "  Secondary Software Version    : %s(%s)\r\n", VOS_StrLen(OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion1)?OnuMgmtTable[OnuEntry].DeviceInfo.SwVersion1:"-", OnuMgmtTable[OnuEntry].ImageValidIndex == 2?"valid":"invalid");  
        vty_out( vty, "  Performance Monitoring        : %s\r\n", OnuMgmtTable[OnuEntry].PmEnable?"ENABLE":"DISABLE");
        vty_out( vty, "  GemPort/Ber interval          : %d/%d\r\n", OnuMgmtTable[OnuEntry].GemPortId, OnuMgmtTable[OnuEntry].BerInterval);
        if(OnuMgmtTable[OnuEntry].RTT>10)
            vty_out( vty, "  RTT                           : %d\r\n", OnuMgmtTable[OnuEntry].RTT);
        else
            vty_out( vty, "  RTT                           : <10Km\r\n");        
    }
    ONU_MGMT_SEM_GIVE; 
    
    vty_out( vty, "\r\n");        

    return ret;
}

#endif
#if 1
extern int ReleaseOnuUplinkBW( short int PonPortIdx, short int OnuIdx );
extern int ReleaseOnuDownlinkBW( short int PonPortIdx, short int OnuIdx );
extern int CTC_STACK_start_loid_authentication (short int  olt_id, short int  onu_id);
extern int CTC_GetOnuDeviceInfo( short int PonPortIdx, short int OnuIdx );
extern STATUS isHaveAuthMacAddress( ULONG slot, ULONG pon, const char* mac,  ULONG *onu );
extern STATUS getOnuAuthEnable( ULONG slot, ULONG port, ULONG *enable );
extern LONG OnuMgtSyncDataSend_Register( short int PonPortIdx, short int OnuIdx );
extern int ClearArpWithOnuNotPresent();
static int OnuEvent_RemoveAllOnu(short int PonPortIdx);
static int OnuEvent_RemoveOneOnu(short int PonPortIdx, short int OnuIdx);
static int OnuEvent_Register_Set_Slave_BW(const short int PonPortIdx, const short int OnuIdx);
extern int gAutoClearArpflag;
int OnuEvent_UpDate_RunningBandWidth( short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    OnuEvent_UpDate_RunningUpBandWidth(PonPortIdx, OnuIdx);
    OnuEvent_UpDate_RunningDownBandWidth(PonPortIdx, OnuIdx);

    OnuMgmtTable[OnuEntry].ActiveUplinkBandwidth = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr;
    OnuMgmtTable[OnuEntry].ActiveDownlinkBandwidth = OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_gr;
    return VOS_OK;
}

int CheckOnuActionIsValid( short int PonPortIdx, short int OnuIdx, int direction)
{
    int ret = VOS_OK;
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    ONU_bw_t Bw;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return ret;
	}
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    if(OLT_CFG_DIR_BOTH == direction)
    {
        if((OnuEvent_GetUpLinkBandWidthByMac(local_mac, &Bw) == VOS_OK)
           && (OnuEvent_GetDownLinkBandWidthByMac(local_mac, &Bw) == VOS_OK))
           ret = VOS_ERROR;
    }
    if(OLT_CFG_DIR_UPLINK == direction)
    {
        if(OnuEvent_GetUpLinkBandWidthByMac(local_mac, &Bw) == VOS_OK)
        {
            ret = VOS_ERROR;
        }
    }
    else
    {
        if(OnuEvent_GetDownLinkBandWidthByMac(local_mac, &Bw) == VOS_OK)
        {
            ret = VOS_ERROR;
        }
    }
    return ret;
}


int OnuEvent_UpDate_RunningUpBandWidth( short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    OnuLLIDTable_S *pLlidCfg;
    ONU_bw_t Bw;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    
    VOS_MemZero(&Bw, sizeof(ONU_bw_t));
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    
    if(OnuEvent_GetUpLinkBandWidthByMac(local_mac, &Bw) != VOS_OK)
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr = pLlidCfg->UplinkBandwidth_gr;
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_be = pLlidCfg->UplinkBandwidth_be;
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed = pLlidCfg->UplinkBandwidth_fixed;
        OnuMgmtTable[OnuEntry].FinalUplinkClass = pLlidCfg->UplinkClass;
        OnuMgmtTable[OnuEntry].FinalUplinkDelay = pLlidCfg->UplinkDelay;
        OnuMgmtTable[OnuEntry].BandWidthIsDefault = pLlidCfg->BandWidthIsDefault;
    	ONU_MGMT_SEM_GIVE;            
    }   
    else
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr = Bw.bw_gr;
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_be = Bw.bw_be;
        OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed = Bw.bw_fixed;
        OnuMgmtTable[OnuEntry].FinalUplinkClass = Bw.bw_class;
        OnuMgmtTable[OnuEntry].FinalUplinkDelay = Bw.bw_delay;
        OnuMgmtTable[OnuEntry].BandWidthIsDefault &= ~OLT_CFG_DIR_UPLINK;
    	ONU_MGMT_SEM_GIVE;            
    }
    return VOS_OK;
}

int OnuEvent_UpDate_RunningDownBandWidth( short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    OnuLLIDTable_S *pLlidCfg;
    ONU_bw_t Bw;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    
    VOS_MemZero(&Bw, sizeof(ONU_bw_t));
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    
    if(OnuEvent_GetDownLinkBandWidthByMac(local_mac, &Bw) != VOS_OK)
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        
        OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_gr = pLlidCfg->DownlinkBandwidth_gr;
        OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_be = pLlidCfg->DownlinkBandwidth_be;
        OnuMgmtTable[OnuEntry].FinalDownlinkClass = pLlidCfg->DownlinkClass;
        OnuMgmtTable[OnuEntry].FinalDownlinkDelay = pLlidCfg->DownlinkDelay;
        OnuMgmtTable[OnuEntry].BandWidthIsDefault = pLlidCfg->BandWidthIsDefault;
    	ONU_MGMT_SEM_GIVE;            
    }
    else
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        
        OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_gr = Bw.bw_gr;
        OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_be = Bw.bw_be;
        OnuMgmtTable[OnuEntry].FinalDownlinkClass = Bw.bw_class;
        OnuMgmtTable[OnuEntry].FinalDownlinkDelay = Bw.bw_delay;
        OnuMgmtTable[OnuEntry].BandWidthIsDefault &= ~OLT_CFG_DIR_DOWNLINK;
    	ONU_MGMT_SEM_GIVE;            
    }
    return VOS_OK;
}

int OnuEvent_Active_UplinkRunningBandWidth(short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    OnuLLIDTable_S *pLlidCfg;
    int brdIdx = GetCardIdxByPonChip( PonPortIdx );
    int portIdx = GetPonPortByPonChip( PonPortIdx );
    ONU_bw_t Bw;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    
    VOS_MemZero(&Bw, sizeof(ONU_bw_t));
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    
    UpdateProvisionedBWInfo( PonPortIdx );
    if(OnuEvent_GetUpLinkBandWidthByMac(local_mac, &Bw) != VOS_OK)
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        if ( OLT_CFG_DIR_UPLINK & pLlidCfg->BandWidthIsDefault )
        {
            /* 拷贝默认带宽 */
			GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &Bw.bw_gr, &Bw.bw_be);
            Bw.bw_fixed = 0;
            Bw.bw_class = OnuConfigDefault.UplinkClass;
            Bw.bw_delay = OnuConfigDefault.UplinkDelay;
            
            Bw.bw_direction = OLT_CFG_DIR_UNDO | OLT_CFG_DIR_UPLINK;
        }
        else
        {
            /* 拷贝指定带宽 */
            Bw.bw_gr    = pLlidCfg->UplinkBandwidth_gr;
            Bw.bw_be    = pLlidCfg->UplinkBandwidth_be;
            Bw.bw_fixed = pLlidCfg->UplinkBandwidth_fixed;
            Bw.bw_class = pLlidCfg->UplinkClass;
            Bw.bw_delay = pLlidCfg->UplinkDelay;
            
            Bw.bw_direction = OLT_CFG_DIR_UPLINK;
        }
    	ONU_MGMT_SEM_GIVE;            
    }   
	if((Bw.bw_gr+Bw.bw_fixed) > ( PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed) )
	{
    	ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Request UpLink BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
		return VOS_ERROR;
	}
    Bw.bw_direction |= OLT_CFG_DIR_DO_ACTIVE;
    
    if(VOS_OK != OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw))
    {
    	ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Active Uplink-BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
        return VOS_ERROR;
    }
    else
        return VOS_OK;

}
int OnuEvent_Active_DownlinkRunningBandWidth(short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    OnuLLIDTable_S *pLlidCfg;
    int brdIdx = GetCardIdxByPonChip( PonPortIdx );
    int portIdx = GetPonPortByPonChip( PonPortIdx );
    ONU_bw_t Bw;
    
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    
    VOS_MemZero(&Bw, sizeof(ONU_bw_t));
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    
    UpdateProvisionedBWInfo( PonPortIdx );
    if(OnuEvent_GetDownLinkBandWidthByMac(local_mac, &Bw) != VOS_OK)
    {
    	ONU_MGMT_SEM_TAKE;
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);            
        if ( OLT_CFG_DIR_DOWNLINK & pLlidCfg->BandWidthIsDefault )
        {
            /* 拷贝默认带宽 */
            Bw.bw_gr    = OnuConfigDefault.DownlinkBandwidth;
            Bw.bw_be    = OnuConfigDefault.DownlinkBandwidthBe;
            Bw.bw_fixed = 0;
            Bw.bw_class = OnuConfigDefault.DownlinkClass;
            Bw.bw_delay = OnuConfigDefault.DownlinkDelay;
            
            Bw.bw_direction = OLT_CFG_DIR_UNDO | OLT_CFG_DIR_DOWNLINK;
        }
        else
        {
            /* 拷贝指定带宽 */
            Bw.bw_gr        = pLlidCfg->DownlinkBandwidth_gr;
            Bw.bw_be        = pLlidCfg->DownlinkBandwidth_be;
            Bw.bw_class     = pLlidCfg->DownlinkClass;
            Bw.bw_delay     = pLlidCfg->DownlinkDelay;
            
            Bw.bw_direction = OLT_CFG_DIR_DOWNLINK;
        }
    	ONU_MGMT_SEM_GIVE;            
    }   
    Bw.bw_direction |= OLT_CFG_DIR_DO_ACTIVE;    
    if(VOS_OK != OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw))
    {
    	ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Active DownLink-BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
        return VOS_ERROR;
    }
    else
        return VOS_OK;
}

/*****************************************************
 *
 *    Function:  OnuEvent_RequestOnuBW( short int PonPortIdx, short int OnuIdx)
 *
 *    Param:    PonPortIdx/OnuIdx-Onu索引
 *                 
 *    Desc:   
 *
 *    Return:    VOS_OK/VOS_ERROR
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int OnuEvent_Request_BandWidth( short int PonPortIdx, short int OnuIdx, ONU_bw_t Bw[2])
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    OnuLLIDTable_S *pLlidCfg;
    int brdIdx = GetCardIdxByPonChip( PonPortIdx );
    int portIdx = GetPonPortByPonChip( PonPortIdx );
    unsigned int uplink_fix = 0;
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    VOS_MemZero(Bw, sizeof(ONU_bw_t)*2);
    if(SYS_LOCAL_MODULE_TYPE_IS_EPON)
    {
        ONU_MGMT_SEM_TAKE;
        VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
        ONU_MGMT_SEM_GIVE;


    	if( MAC_ADDR_IS_INVALID(local_mac) )
    		return VOS_ERROR;
    	UpdateProvisionedBWInfo( PonPortIdx );
        if(OnuEvent_GetUpLinkBandWidthByMac(local_mac, &Bw[0]) != VOS_OK)
        {
        	ONU_MGMT_SEM_TAKE;
            pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
            Bw[0].bw_direction = OLT_CFG_DIR_UPLINK;        
            if ( OLT_CFG_DIR_UPLINK & pLlidCfg->BandWidthIsDefault )
            {
                /* 拷贝默认带宽 */
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, &pLlidCfg->UplinkBandwidth_gr, &pLlidCfg->UplinkBandwidth_be);
                pLlidCfg->UplinkBandwidth_fixed = 0;
                pLlidCfg->UplinkClass = OnuConfigDefault.UplinkClass;
                pLlidCfg->UplinkDelay = OnuConfigDefault.UplinkDelay;
                Bw[0].bw_direction |= OLT_CFG_DIR_UNDO;
            }
            
            Bw[0].bw_gr    = pLlidCfg->UplinkBandwidth_gr;
            Bw[0].bw_be    = pLlidCfg->UplinkBandwidth_be;
            Bw[0].bw_fixed = pLlidCfg->UplinkBandwidth_fixed;
            Bw[0].bw_class = pLlidCfg->UplinkClass;
            Bw[0].bw_delay = pLlidCfg->UplinkDelay;
        	ONU_MGMT_SEM_GIVE;            
        }   
        /*modified by luh 2012-10-25 Onu注册应该以当前激活带宽为判断条件，只要还有剩余带宽就应该允许onu注册*/
        uplink_fix = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed;
        if((Bw[0].bw_gr+Bw[0].bw_fixed + PonPortTable[PonPortIdx].ActiveBW - uplink_fix) > ( PonPortTable[PonPortIdx].MaxBW/* + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed*/) )
    	{
        	ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Request UpLink BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
    		return VOS_ERROR;
    	}
    	
        if(OnuEvent_GetDownLinkBandWidthByMac(local_mac, &Bw[1]) != VOS_OK)
        {
        	ONU_MGMT_SEM_TAKE;  
            pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);  
            Bw[1].bw_direction = OLT_CFG_DIR_DOWNLINK;        
            if ( OLT_CFG_DIR_DOWNLINK & pLlidCfg->BandWidthIsDefault )
            {
                /* 拷贝默认带宽 */
				GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, &pLlidCfg->DownlinkBandwidth_gr, &pLlidCfg->DownlinkBandwidth_be);
                pLlidCfg->DownlinkClass = OnuConfigDefault.DownlinkClass;
                pLlidCfg->DownlinkDelay = OnuConfigDefault.DownlinkDelay;
                Bw[1].bw_direction |= OLT_CFG_DIR_UNDO;
            }
            Bw[1].bw_gr        = pLlidCfg->DownlinkBandwidth_gr;
            Bw[1].bw_be        = pLlidCfg->DownlinkBandwidth_be;
            Bw[1].bw_class     = pLlidCfg->DownlinkClass;
            Bw[1].bw_delay     = pLlidCfg->DownlinkDelay;
        	ONU_MGMT_SEM_GIVE;
        }
    }
    else if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
    {
    	UpdateProvisionedBWInfo( PonPortIdx );
        ONU_MGMT_SEM_TAKE;
        /*copy uplink bandwidth*/
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);
        Bw[0].bw_direction = OLT_CFG_DIR_UPLINK;        
        if ( OLT_CFG_DIR_UPLINK & pLlidCfg->BandWidthIsDefault )
        {
            /* 拷贝默认带宽 */
            pLlidCfg->UplinkBandwidth_gr = OnuConfigDefault.UplinkBandwidth_GPON;
            pLlidCfg->UplinkBandwidth_be = OnuConfigDefault.UplinkBandwidthBe_GPON;
            pLlidCfg->UplinkBandwidth_fixed = 0;
            pLlidCfg->UplinkClass = OnuConfigDefault.UplinkClass;
            pLlidCfg->UplinkDelay = OnuConfigDefault.UplinkDelay;
            Bw[0].bw_direction |= OLT_CFG_DIR_UNDO;
        }
        
        Bw[0].bw_gr    = pLlidCfg->UplinkBandwidth_gr;
        Bw[0].bw_be    = pLlidCfg->UplinkBandwidth_be;
        Bw[0].bw_fixed = pLlidCfg->UplinkBandwidth_fixed;
        Bw[0].bw_class = pLlidCfg->UplinkClass;
        Bw[0].bw_delay = pLlidCfg->UplinkDelay;
        
        /*copy downlink bandwidth*/
        pLlidCfg = &(OnuMgmtTable[OnuEntry].LlidTable[0]);  
        Bw[1].bw_direction = OLT_CFG_DIR_DOWNLINK;        
        if ( OLT_CFG_DIR_DOWNLINK & pLlidCfg->BandWidthIsDefault )
        {
            /* 拷贝默认带宽 */
            pLlidCfg->DownlinkBandwidth_gr = OnuConfigDefault.DownlinkBandwidth_GPON;
            pLlidCfg->DownlinkBandwidth_be = OnuConfigDefault.DownlinkBandwidthBe_GPON;
            pLlidCfg->DownlinkClass = OnuConfigDefault.DownlinkClass;
            pLlidCfg->DownlinkDelay = OnuConfigDefault.DownlinkDelay;
            Bw[1].bw_direction |= OLT_CFG_DIR_UNDO;
        }
        Bw[1].bw_gr        = pLlidCfg->DownlinkBandwidth_gr;
        Bw[1].bw_be        = pLlidCfg->DownlinkBandwidth_be;
        Bw[1].bw_class     = pLlidCfg->DownlinkClass;
        Bw[1].bw_delay     = pLlidCfg->DownlinkDelay;
        ONU_MGMT_SEM_GIVE;       

        /*modified by luh 2012-10-25 Onu注册应该以当前激活带宽为判断条件，只要还有剩余带宽就应该允许onu注册*/
        uplink_fix = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed;
        if((Bw[0].bw_gr+Bw[0].bw_fixed + PonPortTable[PonPortIdx].ActiveBW - uplink_fix) > ( PonPortTable[PonPortIdx].MaxBW/* + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr + OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed*/) )
        {
            ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Request UpLink BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
            return VOS_ERROR;
        }
        
    }
    else
    {
        /*do nothing*/
    }
	
    return VOS_OK;
}

/*由PON 模块申请的带宽向ONU 恢复，参数的检查由外部完成*/
int OnuEvent_Active_BandWidth (short int PonPortIdx, short int OnuIdx, ONU_bw_t Bw[2])
{ 
    int brdIdx = GetCardIdxByPonChip( PonPortIdx );
    int portIdx = GetPonPortByPonChip( PonPortIdx );
    
    Bw[0].bw_direction |= OLT_CFG_DIR_DO_ACTIVE;
    Bw[1].bw_direction |= OLT_CFG_DIR_DO_ACTIVE;
    if(VOS_OK != OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw[0])|| VOS_OK != OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw[1]))
    {
    	ONU_REGISTER_DEBUG("\r\nOnu(%d/%d/%d) Active BandWidth Fail!\r\n", brdIdx, portIdx, OnuIdx+1);
        return VOS_ERROR;
    }
    else
        return VOS_OK;
}
int OnuEvent_Config_BandWidth (short int PonPortIdx, short int OnuIdx, ULONG direction)
{ 
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    ONU_bw_t Bw;
    int ret = 0;
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    VOS_MemZero(&Bw, sizeof(ONU_bw_t));
    Bw.bw_direction = direction;

	if( MAC_ADDR_IS_INVALID(local_mac) )
	{
		return VOS_ERROR;
	}

    if((Bw.bw_direction & OLT_CFG_DIR_BOTH) == OLT_CFG_DIR_BOTH)
    {        
        ONU_MGMT_SEM_TAKE;
        Bw.bw_gr = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr;
        Bw.bw_be = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_be;
        ONU_MGMT_SEM_GIVE;
        Bw.bw_direction &= ~OLT_CFG_DIR_BY_ONUID;
        Bw.bw_fixed   = ONU_DEFAULT_BW;
        Bw.bw_actived = ONU_DEFAULT_BE_BW;
        ret = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw);        
    }
    else
    {
        if(Bw.bw_direction & OLT_CFG_DIR_UPLINK)
        {
            /*OnuEvent_UpDate_RunningUpBandWidth(PonPortIdx, OnuIdx);*/
            ONU_MGMT_SEM_TAKE;           
            Bw.bw_gr    = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_gr;
            Bw.bw_be    = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_be;
            Bw.bw_fixed = OnuMgmtTable[OnuEntry].FinalUplinkBandwidth_fixed;
            Bw.bw_class = OnuMgmtTable[OnuEntry].FinalUplinkClass;
            Bw.bw_delay = OnuMgmtTable[OnuEntry].FinalUplinkDelay;
            if ( OLT_CFG_DIR_UPLINK & OnuMgmtTable[OnuEntry].BandWidthIsDefault )
            {
                /* 拷贝默认带宽 */
                Bw.bw_direction = OLT_CFG_DIR_UNDO|OLT_CFG_DIR_UPLINK;
            }
            else
            {
                /* 拷贝指定带宽 */
                Bw.bw_direction = OLT_CFG_DIR_UPLINK;                
            }
            ONU_MGMT_SEM_GIVE;
            ret = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw);        
        }
        else if(Bw.bw_direction & OLT_CFG_DIR_DOWNLINK)
        {
            /*OnuEvent_UpDate_RunningDownBandWidth(PonPortIdx, OnuIdx);                        */
            ONU_MGMT_SEM_TAKE;
            Bw.bw_gr        = OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_gr;
            Bw.bw_be        = OnuMgmtTable[OnuEntry].FinalDownlinkBandwidth_be;
            Bw.bw_class     = OnuMgmtTable[OnuEntry].FinalDownlinkClass;
            Bw.bw_delay     = OnuMgmtTable[OnuEntry].FinalDownlinkDelay;
            if ( OLT_CFG_DIR_DOWNLINK & OnuMgmtTable[OnuEntry].BandWidthIsDefault )
            {
                /* 拷贝默认带宽 */
                Bw.bw_direction = OLT_CFG_DIR_UNDO | OLT_CFG_DIR_DOWNLINK;
            }
            else
            {
                /* 拷贝指定带宽 */
                Bw.bw_direction = OLT_CFG_DIR_DOWNLINK;
            }
            ONU_MGMT_SEM_GIVE;
            ret = OnuMgt_SetOnuBW(PonPortIdx, OnuIdx, &Bw);                
        }
    }
    if(ret == VOS_OK)
        OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
    return VOS_OK;
}

/*****************************************************
 *
 *    Function:  OnuEvent_FreeOnuBW( short int PonPortIdx, short int OnuIdx )
 *
 *    Param:    PonPortIdx/OnuIdx-Onu索引
 *                 
 *    Desc:   
 *
 *    Return:    VOS_OK/VOS_ERROR
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int OnuEvent_Free_BandWidth( short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    char local_mac[6];
    ONU_bw_t Bw[2];
	if( OnuEntry < 0 || OnuEntry > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
    VOS_MemZero(Bw, sizeof(ONU_bw_t)*2);
    /*moved by luh @2015-11-11*/
#if 0    
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;

	if( MAC_ADDR_IS_INVALID(local_mac) )
		return VOS_ERROR;
#endif
    
    if(VOS_OK == ReleaseOnuUplinkBW(PonPortIdx, OnuIdx))
    {
        /*释放pon的带宽*/
    }
    else
        {
            return VOS_ERROR;
        }
        if(VOS_OK == ReleaseOnuDownlinkBW(PonPortIdx, OnuIdx))
        {
            /*释放pon 的带宽*/
        }
        else
        {
            return VOS_ERROR;
        }

    return VOS_OK;
}
#endif

#if 1
/*******************************************************************
  * Onu 事件注册。事件类型包括注册成功、去注册以
  * 及其他情况的Onu离线事件处理函数注册。code取值
  * ONU_EVENT_CODE_REGISTER/ONU_EVENT_CODE_DEREGISTER/
  * ONU_EVENT_CODE_REMOVE 及它们的任意组合。
  *******************************************************************/
int reigster_onuevent_callback(int code, g_OnuEventFuction_callback function)
{
    OnuEventList_t *cur_OnuEventNode = NULL;
    OnuEventList_t *pre_OnuEventNode = NULL;
    if((ONU_EVENT_CODE_MIN >= code)||(ONU_EVENT_CODE_MAX <= code)||(function == NULL))
    {
        return VOS_ERROR;
    }
    
    if( VOS_SemTake(OnuEventCallBackfuctionSemId, 5000) == VOS_OK)
    {
        cur_OnuEventNode = g_OnuEvent_FunctionList->next;
        while(cur_OnuEventNode) 
        {
            pre_OnuEventNode = cur_OnuEventNode;
            cur_OnuEventNode = cur_OnuEventNode->next;
        }
        cur_OnuEventNode = (OnuEventList_t *)VOS_Malloc(sizeof(OnuEventList_t), MODULE_ONU);
        cur_OnuEventNode->code = code;
        cur_OnuEventNode->func = function;
        cur_OnuEventNode->next = NULL;

        if(pre_OnuEventNode == NULL)
        {
            g_OnuEvent_FunctionList->next = cur_OnuEventNode;
        }
        else
        {
            pre_OnuEventNode->next = cur_OnuEventNode;
        }
        VOS_SemGive(OnuEventCallBackfuctionSemId);
    }
    return VOS_OK;
}

/*******************************************************************
  * Onu 事件回调函数，对Onu 发生的事件进行处理。
  * 主要是通知其他模块Onu当前的状态
  *******************************************************************/
void handler_onuevent_callback(int code, OnuEventData_s data)
{
    OnuEventList_t *OnuEventNode = NULL;

    if((ONU_EVENT_CODE_MIN >= code)||(ONU_EVENT_CODE_MAX <= code))
        return;
    
    if( VOS_SemTake(OnuEventCallBackfuctionSemId, 5000) == VOS_OK)
    {    
        OnuEventNode = g_OnuEvent_FunctionList->next;
        while(OnuEventNode) 
        {
            if(OnuEventNode->code == code)
                (*(OnuEventNode->func))(data);
            OnuEventNode = OnuEventNode->next;
        }
        VOS_SemGive(OnuEventCallBackfuctionSemId);
    }
}
int OnuEvent_InprogressMsg_Send(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, char *pdata)
{
    int ret = VOS_OK;
    unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_REGISTER_INPROCESS, 0,0 };
    	
    aulMsg[0] = PonPortIdx;
    aulMsg[2] = (OnuIdx << 16) | (LLIDIdx & 0xFFFF);
    aulMsg[3] = (int)pdata;

    ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
    if( ret !=  VOS_OK )
        ret = RERROR; 
    return ret;
}
int OnuEvent_SetBandWidthMsg_Send(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, ULONG direction)
{
    int ret = VOS_OK;
    unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_REGISTER_SETBANDWIDTH, 0,0 };
    	
    aulMsg[2] = (PonPortIdx<<16)|OnuIdx;
    aulMsg[3] = direction;
    ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
    if( ret !=  VOS_OK )
        ret = RERROR; 
    return ret;
}
int OnuEvent_ClearRunningDataMsg_Send(short int PonPortIdx)
{
    int ret = VOS_OK;
    unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_EVENT_PONPORT_RESET, 0,0 };
    	
    aulMsg[2] = PonPortIdx;
    ret = VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
    if( ret !=  VOS_OK )
        ret = RERROR; 
    return ret;
}

int OnuEvent_Clear_OnuRunningDataBySlot(ULONG aulMsg[4])
{
    short int slot = (short int )aulMsg[2];
    short int port = 0;
    short int PonPortIdx = 0;
    
    for(port=1;port<=PONPORTPERCARD;port++)
    {
   		PonPortIdx = GetPonPortIdxBySlot(slot, port);
   		if (PonPortIdx == VOS_ERROR)
            continue;
        OnuEvent_RemoveAllOnu(PonPortIdx);
    }
    return VOS_OK;
}

int OnuEvent_Clear_OnuRunningDataByPort(ULONG aulMsg[4])
{
    short int PonPortIdx  = (short int )aulMsg[2];
    if (PonPortIdx != VOS_ERROR)
       OnuEvent_RemoveAllOnu(PonPortIdx);
    return VOS_OK;
}

void OnuEvent_EQU_handler(ULONG aulMsg[4])
{
	unsigned char MsgType;
	int length;
    
    short int OnuIdx = (short int )aulMsg[2] & 0xffff;
    short int PonPortIdx = aulMsg[2] >> 16;
    unsigned char *pBuf = (unsigned char *)aulMsg[3];
    if( pBuf != NULL )
    {
        int OnuType, OnuEntry;
        					
        MsgType = pBuf[0];
        length  = *(int *) &pBuf[4];

        switch( MsgType )
        {
            case  GET_ONU_SYS_INFO_REQ:
                if( IsSupportCTCOnu(PonPortIdx) )
                {
                    if(GetOnuRegisterData( PonPortIdx, OnuIdx) != ROK )
                    {
                        break;
                    }
                    SetOnuExtOAMStatus(PonPortIdx, OnuIdx );
                    CTC_GetOnuDeviceInfo( PonPortIdx, OnuIdx );
                    OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
                    OnuType = CompareCtcRegisterId( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].onu_model );
                    if(OnuType != RERROR)
                    {
                        ONU_MGMT_SEM_TAKE;
                        OnuMgmtTable[OnuEntry].DeviceInfo.type = OnuType;
                        ONU_MGMT_SEM_GIVE;
                        if( GetOnuEUQInfo(PonPortIdx, OnuIdx) == RERROR )
                            break;
                    }
                    else
                    {
                        ONU_MGMT_SEM_TAKE;
                        if(OnuMgmtTable[OnuEntry].onu_ctc_version >= CTC_2_1_ONU_VERSION)
                        {
                            OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_CTC;
                        }
                        else
                            OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;
                        OnuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;
                        ONU_MGMT_SEM_GIVE;

                        if(OnuType == V2R1_DEVICE_UNKNOWN)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    if( GetOnuEUQInfo(PonPortIdx, OnuIdx) == RERROR )
                        break;
                }
                OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );
                break;
							
            case  SET_ONU_SYS_INFO_REQ:
                if( pBuf[2] == SET_ONU_SYS_INFO_NAME )
                    SetOnuDeviceNameMsg( PonPortIdx, OnuIdx, &pBuf[8], length );
                else if( pBuf[2] == SET_ONU_SYS_INFO_DESC )
                    SetOnuDeviceDescMsg(PonPortIdx, OnuIdx, &pBuf[8], length );
                else if( pBuf[2] == SET_ONU_SYS_INFO_LOCATION )
                    SetOnuLocationMsg(PonPortIdx,OnuIdx, &pBuf[8], length);
                break;						
            case  SYNC_ONU_SYS_TIME_REQ:
                SyncSysTimeToOnu(PonPortIdx, OnuIdx );
                break;

            default: 
                break;
        }
        VOS_Free((void *) pBuf );
        /*VOS_TaskDelay(30);*/
    }   
}

/************************************************************
  * Onu 认证，受认证使能、认证模式控制，在
  * CTC 协议栈打开的情况下支持loid及混合认证。
  * 否则，只支持基于Mac 地址的认证
  ************************************************************/
int OnuEvent_Authentication(short int PonPortIdx, short int OnuIdx)
{
    int ret = 0;
    int auth_result = ONU_AUTH_SUCESS;
    int brdIdx = GetCardIdxByPonChip( PonPortIdx );
    int portIdx = GetPonPortByPonChip( PonPortIdx );
    int llid = GetLlidByOnuIdx(PonPortIdx, OnuIdx);
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    ULONG Auth_enable = 0, TableIdx = 0;
    ULONG Auth_mode = 0;
    unsigned char ctc_version = 0;
    char series_number[GPON_ONU_SERIAL_NUM_STR_LEN]={0};
	CHAR Passward[GPON_ONU_PASSWARD_STR_LEN];
    char mac[6]={0};
	gpon_onu_auth_t *plist;
    
    getOnuAuthEnable(brdIdx, portIdx, &Auth_enable);
    mn_getCtcOnuAuthMode(brdIdx, portIdx, &Auth_mode);
    if(Auth_enable != V2R1_ONU_AUTHENTICATION_ENABLE)
    {
    	ONU_REGISTER_DEBUG("\r\n Onu(%d/%d/%d) register authentication is Disable!\r\n", brdIdx, portIdx, OnuIdx+1);
        return ONU_AUTH_SUCESS;
    }
    /*added by luh 2012-11-9;缓存认证需要的mac地址和支持的ctc版本*/
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(series_number, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No,GPON_ONU_SERIAL_NUM_STR_LEN);
	VOS_MemCpy(Passward, OnuMgmtTable[OnuEntry].DeviceInfo.DevicePassward,GPON_ONU_PASSWARD_STR_LEN);
    VOS_MemCpy(mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    ctc_version = OnuMgmtTable[OnuEntry].onu_ctc_version;
    ONU_MGMT_SEM_GIVE;

    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
    {
    	if(Auth_mode == mn_ctc_auth_mode_mac || Auth_mode == mn_gpon_auth_mode_sn)
    	{
            plist = gonu_auth_entry_list_seach(brdIdx, portIdx, series_number);
	 		if( plist == NULL) 
            	auth_result = ONU_AUTH_FAIL;	
    	}
		else if(Auth_mode == mn_gpon_auth_mode_sn_and_pwd)
		{
			plist = gonu_auth_entry_list_seach(brdIdx, portIdx, series_number);
			if( plist == NULL) 
            	auth_result = ONU_AUTH_FAIL;
			else
			{
				if(VOS_StrCmp(Passward,plist->authEntry.password) != 0)
				{
					auth_result = ONU_AUTH_FAIL;
				}
			
			}
		}
    
    /*B-基于loid 以及混合模式的认证，由此完成，认证不通过的进入pending队列。*
        *PS: 可选支持互通onu的静默机制。new add by luh 2012-3-1*/
		return auth_result;
    }
	else if(V2R1_CTC_STACK)
    {
        /*基于Mac 地址的认证*/
        if(Auth_mode == mn_ctc_auth_mode_mac)
        {
			ret = isHaveAuthMacAddress(brdIdx, portIdx, mac, &TableIdx );
	 		if( ret == VOS_ERROR ) 
                auth_result = ONU_AUTH_FAIL;
        }
        /*基于Loid表的认证*/
        else if(mn_ctc_auth_mode_loid == Auth_mode || mn_ctc_auth_mode_loid_no_pwd == Auth_mode)
        {
            if(ctc_version>= CTC_2_1_ONU_VERSION)
            {
                if(CTC_STACK_start_loid_authentication(PonPortIdx, OnuIdx)!=VOS_OK)
                    auth_result = ONU_AUTH_FAIL;
            }
            else
            {
                auth_result = ONU_AUTH_FAIL;
            }
        }
        /*混合认证*/
        else if(mn_ctc_auth_mode_hybrid == Auth_mode || mn_ctc_auth_mode_hybrid_no_pwd == Auth_mode)
        {
            /*先进行mac 认证，如果通不过再进行loid 认证。2012-3-5 add by luh */
			ret = isHaveAuthMacAddress(brdIdx, portIdx, mac, &TableIdx );
	 		if( ret == VOS_ERROR ) 
	 		{
                if(ctc_version>= CTC_2_1_ONU_VERSION)
                {
                    if(CTC_STACK_start_loid_authentication(PonPortIdx, OnuIdx) != VOS_OK)
                        auth_result = ONU_AUTH_FAIL;
    	 		}
                else
                {
                    auth_result = ONU_AUTH_FAIL;
                }
	 		}
        }    
    }
    else
    {
        /*基于Mac 地址的认证*/
        if(Auth_mode == mn_ctc_auth_mode_mac)
        {
            ret = isHaveAuthMacAddress(brdIdx, portIdx, mac, &TableIdx );
	 		if( ret == VOS_ERROR ) 
                auth_result = ONU_AUTH_FAIL;
        }
    }
    ONU_REGISTER_DEBUG("\r\n Onu(%d/%d/%d) register auth-%s, mode(%d)!\r\n", brdIdx, portIdx, OnuIdx+1, (auth_result == ONU_AUTH_SUCESS?"sucess":"fail"), Auth_mode);

    /*End-new add by luh 2012-3-1*/
    return auth_result;
}
static int OnuEvent_RemoveAllOnu(short int PonPortIdx)
{
    int OnuIdx = 0;
	for( OnuIdx=0; OnuIdx<MAXONUPERPON; OnuIdx++)
	{
		OnuEvent_RemoveOneOnu( PonPortIdx, OnuIdx);
	}
    return VOS_OK;
}
static int OnuEvent_RemoveOneOnu(short int PonPortIdx, short int OnuIdx)
{
    OnuEventData_s data;
    int OnuStatus = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
    VOS_MemZero(&data, sizeof(OnuEventData_s));
    if(OnuStatus != ONU_OPER_STATUS_DOWN)	
    {
        if(OnuStatus  == ONU_OPER_STATUS_UP )	
        {
            /*2012-11-15；重启pon板和复位pon口均进行处理，由主控板上报trap，防止重复上报*/
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            Trap_OnuDeregister( PonPortIdx, OnuIdx, PON_ONU_DEREGISTRATION_HOST_REQUEST, 1 );	
            data.PonPortIdx = PonPortIdx;
            data.OnuIdx = OnuIdx;
            handler_onuevent_callback(ONU_EVENT_CODE_REMOVE, data);
        }
        ClearOnuRunningData( PonPortIdx, OnuIdx, 0);
	}
    return VOS_OK;
}
#endif

#if 1
UCHAR OnuEvent_Get_OnuAbility(short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    UCHAR OnuAbility;

    ONU_MGMT_SEM_TAKE;
    OnuAbility = OnuMgmtTable[OnuEntry].OnuAbility;
    ONU_MGMT_SEM_GIVE;
    return OnuAbility;
}

int OnuSupportStatistics(short int PonPortIdx, short int OnuIdx)
{
    UCHAR OnuAbility;
    int ret = VOS_ERROR;
    
    OnuAbility = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);    
    if(OnuAbility&0x80)
        ret = VOS_OK;
    return ret;
}
/*added by luh , onu是否支持snmp透传能力*/
int OnuSupportSnmpTrans(short int PonPortIdx, short int OnuIdx)
{
    UCHAR OnuAbility;
    int ret = VOS_ERROR;
    
    OnuAbility = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);    
    if(OnuAbility&0x40)
        ret = VOS_OK;
    return ret;
    
}
/*added by luh, onu是否支持端口mac地址统计，能力的第五个比特位*/
int OnuSupportEthPortMacCounter(short int PonPortIdx, short int OnuIdx)
{
    UCHAR OnuAbility;
    int ret = VOS_ERROR;
    
    OnuAbility = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);    
    if(OnuAbility&0x08)
        ret = VOS_OK;
    return ret;
}

int OnuSupportSnmpPrivateStatistic(short int PonPortIdx, short int OnuIdx)
{
    UCHAR OnuAbility;
    int ret = VOS_ERROR;
    
    OnuAbility = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);    
    if(OnuAbility&0x20)
        ret = VOS_OK;
    return ret;    
}
int OnuSupportSnmpPonStatistic(short int PonPortIdx, short int OnuIdx)
{
    UCHAR OnuAbility;
    int ret = VOS_ERROR;
    
    OnuAbility = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);    
    if(OnuAbility&0x04)
        ret = VOS_OK;
    return ret;    
}   
int OnuEvent_Set_OnuAbility(short int PonPortIdx, short int OnuIdx, UCHAR OnuAbility)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[OnuEntry].OnuAbility = OnuAbility;
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;
}


int OnuGeneral_Get_EquipmentID(short int PonPortIdx, short int OnuIdx, unsigned char *equipment)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(equipment, OnuMgmtTable[OnuEntry].DeviceInfo.equipmentID, OnuMgmtTable[OnuEntry].DeviceInfo.equipmentIDLen); 
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;    
}

int OnuGen_Get_CtcVersion(short int PonPortIdx, short int OnuIdx, unsigned char *ver)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

    ONU_MGMT_SEM_TAKE;
    *ver = OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].onu_ctc_version; /*yangzl*/
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;    
}
int OnuGen_Set_CtcVersion(short int PonPortIdx, short int OnuIdx, unsigned char ver)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[PonPortIdx * MAXONUPERPON + OnuIdx].onu_ctc_version = ver; 
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;    
}
int OnuEvent_Get_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    ULONG last_time;

    ONU_MGMT_SEM_TAKE;
    last_time = OnuMgmtTable[OnuEntry].LlidTable[LLIDIdx].OnuEventRegisterTimeout;
    ONU_MGMT_SEM_GIVE;
    return last_time;
}
int OnuEvent_Set_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, int TimeOut)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[OnuEntry].LlidTable[LLIDIdx].OnuEventRegisterTimeout = TimeOut;
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;
}
int OnuEvent_Clear_RegisterTimer(short int PonPortIdx, short int OnuIdx, short int LLIDIdx)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    
    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[OnuEntry].LlidTable[LLIDIdx].OnuEventRegisterTimeout = 0;
    ONU_MGMT_SEM_GIVE;
    
    return VOS_OK;
}

int OnuEvent_Set_RegisterStatus(short int PonPortIdx, short int OnuIdx, short int LLIDIdx, int status)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[OnuEntry].LlidTable[LLIDIdx].OnuhandlerStatus = status;
    ONU_MGMT_SEM_GIVE;
    return VOS_OK;
}
int OnuEvent_Get_RegisterStatus(short int PonPortIdx, short int OnuIdx, short int LLIDIdx)
{
    int status = 0;
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    ONU_MGMT_SEM_TAKE;
    status = OnuMgmtTable[OnuEntry].LlidTable[LLIDIdx].OnuhandlerStatus;
    ONU_MGMT_SEM_GIVE;
    
    return status;
}


UCHAR OnuEvent_Get_WaitOnuEUQMsgFlag(short int PonPortIdx, short int OnuIdx)
{
    UCHAR status = 0;
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    ONU_MGMT_SEM_TAKE;
    status = OnuMgmtTable[OnuEntry].WaitOnuEUQMsgFlag;
    ONU_MGMT_SEM_GIVE;
    
    return status;
}
int OnuEvent_Set_WaitOnuEUQMsgFlag(short int PonPortIdx, short int OnuIdx, UCHAR status)
{
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    ONU_MGMT_SEM_TAKE;
    OnuMgmtTable[OnuEntry].WaitOnuEUQMsgFlag = status;
    ONU_MGMT_SEM_GIVE;
    
    return VOS_OK;
}
#endif

#if 1
int test_onu_reg_timeout = 200; /* 因gpon多端口onu mibupload时间较长，增大注册超时时间 */
int test_onu_auth_timeout = 10;
int OnuEvent_OK(OnuClientData_s OnuClientData)
{
    /*modi by luh 2013-3-12*/
    /*OnuEvent_Clear_RegisterTimer(OnuClientData.PonPortIdx, OnuClientData.OnuIdx);    */
    OnuEvent_Set_RegisterTimer(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx, test_onu_reg_timeout);    
    return VOS_OK;
}
int OnuEvent_ERROR(OnuClientData_s OnuClientData)
{
    char *pdata = (char *)OnuClientData.DataBuffer;
    int llid = GetLlidByLlidIdx(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx);
    char local_mac[6];
	UCHAR SN[17];
    int OnuEntry = (OnuClientData.PonPortIdx) * MAXONUPERPON + OnuClientData.OnuIdx;    

    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, 17);
    ONU_MGMT_SEM_GIVE;
    
    ONU_REGISTER_DEBUG("OnuEvent_ERROR: PonPortIdx = %d, OnuIdx = %d\r\n", OnuClientData.PonPortIdx, OnuClientData.OnuIdx);    
    OnuEvent_Clear_RegisterTimer(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, OnuClientData.LLIDIdx);
    
    if(pdata)
    {
        VOS_Free(pdata);
    }
    
    if(llid == INVALID_LLID)
        return VOS_ERROR;
    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
    	AddPendingOnu(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, llid, SN, 0); 
	else
	{
		if( MAC_ADDR_IS_INVALID(local_mac))
			return VOS_ERROR;
    	AddPendingOnu(OnuClientData.PonPortIdx, OnuClientData.OnuIdx, llid, local_mac, 0); 
	}
    return VOS_OK;
}

/*int OnuEvent_SetBandWidth(OnuClientData_s OnuClientData)
{
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    short int slot = GetCardIdxByPonChip(PonPortIdx);
    short int port = GetPonPortByPonChip(PonPortIdx);
    int direction = OnuClientData.Data;
    int direction_flag = 0;
    char *direction_str[] = 
    {
        "NULL"
        "Both UPLINK and DOWNLINK",
        "UPLINK",
        "DOWNLINK",
    };
                
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);    
    if((direction & OLT_CFG_DIR_BOTH) == OLT_CFG_DIR_BOTH)
        direction_flag = 1;
    else
    {
        if(direction & OLT_CFG_DIR_UPLINK)
            direction_flag = 2;
        else if(direction & OLT_CFG_DIR_DOWNLINK)
            direction_flag = 3;
    }
    if(OnuEvent_Config_BandWidth(PonPortIdx, OnuIdx, direction) != VOS_OK)
        sys_console_printf("Config Onu %d/%d/%d %s BandWidth error!\r\n", slot, port, OnuIdx+1, direction_str[direction_flag]);
    return VOS_OK;
}*/

int abiDbg = 1;
int OnuEvent_Register_GetEQUInfo(OnuClientData_s OnuClientData)
{
    int ret = VOS_OK;
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    short int firstllid = 0;
    short int llid = 0;
    int OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;    
    char mac_address[6];
    
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);     
    
    llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(mac_address, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
    firstllid = OnuMgmtTable[OnuEntry].LLID;
    ONU_MGMT_SEM_GIVE;

    if (firstllid == llid)
    {
        short int slot = GetCardIdxByPonChip(PonPortIdx);
        short int port = GetPonPortByPonChip(PonPortIdx);
        
        if(GetOnuRegisterData(PonPortIdx, OnuIdx) != ROK ) 
        {
            AddPendingOnu(PonPortIdx, OnuIdx, llid, mac_address, 0); 
            return (RERROR );
        }

        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[OnuEntry].onu_model = 0;
        ONU_MGMT_SEM_GIVE;
        
        OnuEvent_Set_RegisterTimer( PonPortIdx, OnuIdx, LLIDIdx, test_onu_auth_timeout );
        if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,0,0) == ROK )
        {
            ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send OK!\r\n", slot, port, OnuIdx+1 );
        }
        else
        {
            ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send ERR!\r\n", slot, port, OnuIdx+1 );
        }
    }
    else
    {
        /* ONU增加新的LLID，无需重新获取设备信息 */
        OnuEvent_InprogressMsg_Send( PonPortIdx, OnuIdx, LLIDIdx, NULL );
    }
    
    return ret;
}
int OnuEvent_Register_Discovery(OnuClientData_s OnuClientData)
{
	int ret = VOS_OK;
	int brdIdx = 0, portIdx = 0, onuIdx = 0;
	int OnuEntry;
	int gwonu_flag = 0;/*是否是支持私有OAM 设备信息获取, 1为支持，0为不支持*/
	int llid;
	short int PonPortIdx = OnuClientData.PonPortIdx;
	short int OnuIdx = OnuClientData.OnuIdx;
	short int LLIDIdx = OnuClientData.LLIDIdx;
	short int firstllid;
	short int OnuType;
	UCHAR OltType;
	short int PonChipType;
	char local_mac[6];
	onu_local_info_t local_info;
	onu_local_info_all_t info;

	OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;

	brdIdx = GetCardIdxByPonChip( PonPortIdx );
	portIdx = GetPonPortByPonChip( PonPortIdx );
	onuIdx = OnuIdx + 1;

	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	firstllid = OnuMgmtTable[OnuEntry].LLID;
	ONU_MGMT_SEM_GIVE;
	OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);        
	llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);

	VOS_MemZero(&local_info,sizeof(onu_local_info_t));
	VOS_MemZero(&info,sizeof(onu_local_info_all_t));

	if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100)
	{
		OltType = 0x61;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_EPON3)
	{
		OltType = 0x67;
	}
	else if((SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900) 
			|| (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900S)
			|| (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900M))
	{
		OltType = 0x69;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8000)
	{
		OltType = 0x80;
	}
	else if(SYS_PRODUCT_TYPE == PRODUCT_E_GFA8100)
	{
		OltType = 0x81;
	}

	local_info.ucOltType = (UCHAR)OltType;
	local_info.ucOltSlot = (UCHAR)brdIdx;
	local_info.ucOltPort = (UCHAR)portIdx;
	local_info.ucOnuId = (UCHAR)onuIdx;
	info.info_type = 0x0001; /*bit位置1表示携带ONU位置信息,表示没携带*/
	info.info_Id = 1; /*0-保留；1-ONU位置信息*/
	info.info_len = sizeof(onu_local_info_t);
	info.info = local_info;

	if(SYS_LOCAL_MODULE_TYPE_IS_EPON)
	{

		if (llid == firstllid)
		{
			if(GetOnuRegisterData( PonPortIdx, OnuIdx) != ROK )
			{
				AddPendingOnu( PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_GET_ONU_REGISTER_INFO_FAIL); 
				/*Onu_deregister( PonPortIdx,OnuIdx);*/
				return( RERROR );
			}


			/*handler function , Body*/
			SetOnuExtOAMStatus( PonPortIdx, OnuIdx );

			/* 扩展发现成功，则尝试挂接CTC接口 */
			PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
			ONU_SetupIFsByType(PonPortIdx, OnuIdx, PonChipType, ONU_MANAGE_CTC);

			/* 获取ONU 设备信息*/
			if ( 0 != CTC_GetOnuDeviceInfo( PonPortIdx, OnuIdx ) )
			{
				/* CTC信息获取失败，则认为GW接口 */
				ONU_SetupIFsByType(PonPortIdx, OnuIdx, PonChipType, ONU_MANAGE_GW);

				gwonu_flag = 1;
				OnuEvent_Set_RegisterTimer( PonPortIdx, OnuIdx, LLIDIdx, test_onu_auth_timeout);  
				if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) == ROK )
				{
					ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send OK!\r\n", brdIdx, portIdx, onuIdx);
				}
				else
				{
					if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) !=ROK)
						ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send ERR!\r\n", brdIdx, portIdx, onuIdx);
				}       
			}
			else
			{
				OnuEvent_Set_RegisterTimer( PonPortIdx, OnuIdx, LLIDIdx, test_onu_auth_timeout);  
				OnuType = CompareCtcRegisterId( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].onu_model );
				if(OnuType != RERROR)
				{
					gwonu_flag = 1;
					ONU_MGMT_SEM_TAKE;
					OnuMgmtTable[OnuEntry].DeviceInfo.type = OnuType;
					ONU_MGMT_SEM_GIVE;
					/* modified by wangxiaoyu 2011-10-20, move to CTC_GetOnuDeviceInfo for get cortina onu name */
					/*ret = GetOnuEUQInfo( PonPortIdx, OnuIdx );*/
					if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) == ROK )
					{
						ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send OK!\r\n", brdIdx, portIdx, onuIdx);
					}
					else
					{
						if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) !=ROK)
							ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send ERR!\r\n", brdIdx, portIdx, onuIdx);
					}       
				}
				else
				{
					/*ULONG onu_model = 0;*/
					unsigned char verdor_id[4]={0};
					unsigned char ctc_version = 0;
					ONU_MGMT_SEM_TAKE;
					/*onu_model = OnuMgmtTable[OnuEntry].onu_model;*/
					ctc_version = OnuMgmtTable[OnuEntry].onu_ctc_version;
					VOS_MemCpy(verdor_id, OnuMgmtTable[OnuEntry].device_vendor_id, 4);        
					ONU_MGMT_SEM_GIVE;
					/*modified by luh 2012-11-21, 放开私有OAM 设备信息获取条件，不再只限制811C*/
					if(ctc_version >= CTC_2_1_ONU_VERSION)
					{
						if(!VOS_MemCmp(verdor_id, CTC_ONU_VENDORID, 4) /*&& onu_model == CTC_GT811_C_MODEL*/)
						{
							gwonu_flag = 1;
							if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) == ROK )
							{
								ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send OK!\r\n", brdIdx, portIdx, onuIdx);
							}
							else
							{
								if(EQU_SendMsgToOnu_ASYNC(PonPortIdx, OnuIdx, GET_ONU_SYS_INFO_REQ,(unsigned char *)&info,sizeof(onu_local_info_all_t)) !=ROK)
									ONU_REGISTER_DEBUG(" onu %d/%d/%d Get device info Msg Send ERR!\r\n", brdIdx, portIdx, onuIdx);
							}       
						}
					}
				}
			}
		}
		else
		{
			/* 增加LLID的扩展发现，暂无处理 */
			gwonu_flag = 0;
		}
		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery Complete\r\n", brdIdx, portIdx, onuIdx );        
	}
	else if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
	{             
		/* 扩展发现成功，则尝试挂接CTC接口 */
		int iRlt = 0;
		UCHAR ability = 0;
		PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		ONU_SetupIFsByType(PonPortIdx, OnuIdx, 2, ONU_MANAGE_GPON);
		OnuEvent_Set_RegisterTimer( PonPortIdx, OnuIdx, LLIDIdx, test_onu_auth_timeout);                      
		if(VOS_OK != (iRlt = GPONONU_GetDeviceInformation(PonPortIdx, OnuIdx)))
		{
			sys_console_printf("\r\n onu%d/%d/%d get device information failed(%d)\r\n", brdIdx, portIdx, onuIdx, iRlt);                
		}
		if(abiDbg)
		{
			ability = OnuEvent_Get_OnuAbility(PonPortIdx, OnuIdx);
			ability |= 0x80;
			OnuEvent_Set_OnuAbility(PonPortIdx, OnuIdx, ability);
		}
		sys_console_printf("\r\n onu%d/%d/%d Ext OAM discovery Complete\r\n", brdIdx, portIdx, onuIdx );                
	}
	else
	{
		VOS_ASSERT(0);
	}

	if(!gwonu_flag)/*非GW ONU 状态机需要驱动，GW ONU 状态机由获取设备信息成功后驱动*/
		OnuEvent_InprogressMsg_Send(PonPortIdx, OnuIdx, LLIDIdx, NULL);
	return ret;
}

int OnuEvent_Register_Finish(OnuClientData_s OnuClientData)
{
	short int PonPortIdx = OnuClientData.PonPortIdx;
	short int OnuIdx = OnuClientData.OnuIdx;
	short int LLIDIdx = OnuClientData.LLIDIdx;
	short int firstllid;
	int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
	int ret = 0;
	int vendorType;
	UCHAR local_mac[6];
	UCHAR SN[GPON_ONU_SERIAL_NUM_STR_LEN];
	UCHAR SerialNoAfterSIXDigit[BYTES_IN_MAC_ADDRESS];
	int reg_flag, traffic_enable;
	OnuEventData_s data;
	OnuRegisterDenied_S RegisterDenied_Data;  
	int OnuType;
	int brdIdx = GetCardIdxByPonChip( PonPortIdx );
	int portIdx = GetPonPortByPonChip( PonPortIdx );
	int llid = GetLlidByLlidIdx(PonPortIdx, OnuIdx, LLIDIdx);
	short int PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	int onuIdx = OnuIdx + 1;
	char* EQUData = OnuClientData.DataBuffer;
    RPC_CTC_mxu_mng_global_parameter_config_t stOnuMngIp;      
    
	VOS_MemZero(&data, sizeof(OnuEventData_s));
	VOS_MemZero(&RegisterDenied_Data, sizeof(OnuRegisterDenied_S));
    
	ONU_MGMT_SEM_TAKE;
	VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
	VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, GPON_ONU_SERIAL_NUM_STR_LEN);
	reg_flag = OnuMgmtTable[OnuEntry].RegisterFlag == ONU_FIRST_REGISTER_FLAG?1:0;  
	traffic_enable = OnuMgmtTable[OnuEntry].TrafficServiceEnable;
	firstllid = OnuMgmtTable[OnuEntry].LLID;

	/*begin: 增加支持ONU管理IP的配置加载, mod by liuyh, 2017-5-12*/
    stOnuMngIp.mxu_mng.mng_ip = OnuMgmtTable[OnuEntry].mngIp.ip;
    stOnuMngIp.mxu_mng.mng_mask = OnuMgmtTable[OnuEntry].mngIp.mask;
    stOnuMngIp.mxu_mng.mng_gw = OnuMgmtTable[OnuEntry].mngIp.gw;
    stOnuMngIp.mxu_mng.data_cvlan = OnuMgmtTable[OnuEntry].mngIp.cVlan;
    stOnuMngIp.mxu_mng.data_svlan = OnuMgmtTable[OnuEntry].mngIp.sVlan;
    stOnuMngIp.mxu_mng.data_priority = OnuMgmtTable[OnuEntry].mngIp.pri;
	/*end: mod by liuyh, 2017-5-12*/    
	ONU_MGMT_SEM_GIVE;

	data.PonPortIdx = PonPortIdx;
	data.OnuIdx = OnuIdx;
	data.llid = llid;
	VOS_MemCpy(data.onu_mac, local_mac, 6); 
	OnuEvent_Set_RegisterTimer( PonPortIdx, OnuIdx, LLIDIdx, test_onu_auth_timeout);

	if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
	{

		ONU_MGMT_SEM_TAKE;
		OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_GPON;
		ONU_MGMT_SEM_GIVE;

		if(OnuEvent_Authentication(PonPortIdx, OnuIdx) != ONU_AUTH_SUCESS)
		{     
			short int icount;
			AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_ONU_AUTH_FAIL);
			RegisterDenied_Data.olt_id = PonPortIdx;
			/*removed by liub 2017-5-15. 发送认证失败消息，mac地址可直接用管理表里获取*/
#if 0
			for(icount = 0;icount < 6;icount += 1)
			{
				SerialNoAfterSIXDigit[icount] = SN[10+icount] - '0';
			}
			VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, SerialNoAfterSIXDigit, BYTES_IN_MAC_ADDRESS );
#else
			VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, local_mac, BYTES_IN_MAC_ADDRESS );
#endif
			/*modified for GPON alarm Q:29778 yangzl@2016-7-7*/
			OnuRegisterDeniedHandler( &RegisterDenied_Data);
			return VOS_ERROR;
		}

		ONU_MGMT_SEM_TAKE;
		{
			/*GPON 只比较gwd这3个字节*/
			if(get_ctcOnuOtherVendorAccept()||!VOS_MemCmp(OnuMgmtTable[OnuEntry].device_vendor_id, CTC_ONU_VENDORID,
						/*sizeof(OnuMgmtTable[OnuEntry].device_vendor_id)*/3))
			{
				/*do nothing*/
			}
			else
			{
				AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_ONU_DENY);                    
				ONU_MGMT_SEM_GIVE;
				return VOS_ERROR;
			}
		}
		ONU_MGMT_SEM_GIVE;

		/*4、申请并设置带宽*/
		{
			ONU_bw_t Bw[2];
			if(VOS_OK == (ret = OnuEvent_Request_BandWidth(PonPortIdx, OnuIdx, Bw)))
			{
				if(VOS_OK != OnuEvent_Active_BandWidth(PonPortIdx, OnuIdx, Bw))
				{
					OnuEvent_Free_BandWidth(PonPortIdx, OnuIdx);
					AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_SET_BW_FAIL);
					return VOS_ERROR;
				}
			}
			else
			{
				llidActBWExceeding_EventReport( (unsigned long)onuIdToOnuIndex(PonPortIdx, OnuIdx+1), 1, 1, 1 );             
				AddPendingOnu(PonPortIdx, OnuIdx, llid, SN, PENDING_REASON_CODE_REQUEST_BW_FAIL);
				return VOS_ERROR;
			}
		}
		/*    恢复ONU 上行业务优先级配置*/
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
		RestoreUplinkVlanManipulationToPon(PonPortIdx, OnuIdx );
#endif

		/*5、更新onu的状态及上线时间，*/
		updateOnuOperStatusAndUpTime( OnuEntry, ONU_OPER_STATUS_UP );

		/*恢复onumax-mac配置*/
		SetOnuMaxMacNum( PonPortIdx, OnuIdx, 0, OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC );

        /*begin: 增加支持ONU管理IP的配置加载, mod by liuyh, 2017-5-12*/
        if (stOnuMngIp.mxu_mng.data_cvlan != 0)
        {
            /* 只有配置过才加载管理IP */
            stOnuMngIp.needSaveOlt = TRUE; 
            ret = OnuMgt_SetMxuMngGlobalConfig(PonPortIdx, OnuIdx, &stOnuMngIp);   
        }        
	    /*end: mod by liuyh, 2017-5-12*/

		/*7、modified by xieshl 20111129, 增加主备同步 */
		OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );

		/*8、send register trap to NMS */
		Trap_OnuRegister( PonPortIdx, OnuIdx );

		/*9、注册回调函数，通知其他模块，ONU 注册成功*/
		handler_onuevent_callback(ONU_EVENT_CODE_REGISTER, data);

	}
	else  if(SYS_LOCAL_MODULE_TYPE_IS_EPON)
	{
		if ( llid == firstllid )
		{
			/*1、onu 认证，失败进入pending队列，状态机变为fail状态*/
			if(OnuEvent_Authentication(PonPortIdx, OnuIdx) != ONU_AUTH_SUCESS)
			{
				if(EQUData)
				{
					VOS_Free(EQUData);
				}        
				AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_ONU_AUTH_FAIL);
				RegisterDenied_Data.olt_id = PonPortIdx;
				VOS_MemCpy( RegisterDenied_Data.Onu_MacAddr, local_mac, BYTES_IN_MAC_ADDRESS );
				OnuRegisterDeniedHandler( &RegisterDenied_Data);
				return VOS_ERROR;
			}
		}

		/*2、 modified by xieshl 20110613, 上层API需要获取LLID，注册处理这样调用有风险，而且效率低，同时这本来是底层驱动回调的函数，直接调pas驱动即可 */
		/* ONU授权入网后，一些配置才能下发 */
		if ( llid == firstllid )
		{
			ret = OnuMgt_AuthorizeOnu(PonPortIdx, OnuIdx, TRUE);
			ONU_REGISTER_DEBUG("\r\n Authorize this onu(%d/%d/%d) ... %s\r\n", brdIdx, portIdx, onuIdx, (ret == 0 ? "ok" : "err") );
		}
		else
		{
			ret = OLT_AuthorizeLLID(PonPortIdx, llid, TRUE);
			ONU_REGISTER_DEBUG("\r\n Authorize this llid(%d/%d/%d:%d) ... %s\r\n", brdIdx, portIdx, onuIdx, llid, (ret == 0 ? "ok" : "err") );
		}
		if( ret != 0 )
		{
			if(EQUData)
			{
				VOS_Free(EQUData);
			}        
			AddPendingOnu( PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_ONU_AUOR_FAIL); 
			return( RERROR );
		}

		if ( llid == firstllid )
		{
			if(EQUData)
			{
				GetOnuEUQInfoHandle_New(PonPortIdx, OnuIdx, EQUData);
			}

			if(V2R1_CTC_STACK)
			{
				OnuType = CompareCtcRegisterId( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].onu_model );
				if(OnuType == RERROR)
				{
					ONU_MGMT_SEM_TAKE;
					if(OnuMgmtTable[OnuEntry].onu_ctc_version >= 0x20/*CTC_2_1_ONU_VERSION*/)
					{
						if(get_ctcOnuOtherVendorAccept()||!VOS_MemCmp(OnuMgmtTable[OnuEntry].device_vendor_id, CTC_ONU_VENDORID,
									sizeof(OnuMgmtTable[OnuEntry].device_vendor_id)))
						{
							OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_CTC;
						}
						else
						{
							AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_ONU_DENY);                    
							OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;                        
							ONU_MGMT_SEM_GIVE;
							return VOS_ERROR;
						}
					}
					else
					{
						/*added by luh@2016-6-1 for Q.28035*/
						AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, 0);                    
						OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;                        
						ONU_MGMT_SEM_GIVE;
						return VOS_ERROR;
					}

					if ( 0 < OnuMgmtTable[OnuEntry].cmMax )
					{
						OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_CMC;
					}

					OnuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;
					ONU_MGMT_SEM_GIVE;
					/*if(OnuType == V2R1_DEVICE_UNKNOWN)
					  OnuMgt_SetOnuTrafficServiceMode(PonPortIdx, OnuIdx, V2R1_DISABLE);	*/
				}
			}

			/*3、管理接口挂接*/
			if( GetOnuVendorType( PonPortIdx, OnuIdx ) == ONU_VENDOR_CT )
			{
				vendorType = ONU_MANAGE_CTC;
			}
			else
			{
				vendorType = ONU_MANAGE_GW;
			}
			ONU_SetupIFsByType(PonPortIdx, OnuIdx, PonChipType, vendorType);
		}


		/*4、申请并设置带宽*/
		{
			ONU_bw_t Bw[2];
			if(VOS_OK == (ret = OnuEvent_Request_BandWidth(PonPortIdx, OnuIdx, Bw)))
			{
				if(VOS_OK != OnuEvent_Active_BandWidth(PonPortIdx, OnuIdx, Bw))
				{
					OnuEvent_Free_BandWidth(PonPortIdx, OnuIdx);
					AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_SET_BW_FAIL);
					return VOS_ERROR;
				}
			}
			else
			{
				llidActBWExceeding_EventReport( (unsigned long)onuIdToOnuIndex(PonPortIdx, OnuIdx+1), 1, 1, 1 );             
				AddPendingOnu(PonPortIdx, OnuIdx, llid, local_mac, PENDING_REASON_CODE_REQUEST_BW_FAIL);
				return VOS_ERROR;
			}
		}

		if ( llid == firstllid )
		{
			/*5、更新onu的状态及上线时间，*/
			updateOnuOperStatusAndUpTime( OnuEntry, ONU_OPER_STATUS_UP );

			/*6、Onu 配置下发*/
#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
			OnuMgt_SetSlowProtocolLimit(PonPortIdx, OnuIdx, DISABLE);
#else
			REMOTE_PASONU_set_slow_protocol_limit(PonPortIdx, llid, DISABLE);
#endif

			/* 限制ONU在OLT上占用的MAC地址表资源 */
			SetOnuMaxMacNum( PonPortIdx, OnuIdx, 0, OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC );
			OnuMgt_StopEncrypt(PonPortIdx, OnuIdx);
#ifdef ONU_PEER_TO_PEER
			{
				int ret;
				short int OnuIdx1;
				short int OnuNeedP2P;

				OnuNeedP2P = 0;
				EnableOnuPeerToPeerForward( PonPortIdx, OnuIdx );
				for( OnuIdx1= 0; OnuIdx1 < MAXONUPERPON; OnuIdx1++)
				{
					ret = GetOnuPeerToPeer( PonPortIdx, OnuIdx, OnuIdx1 );
					if( ret == V2R1_ENABLE )
					{
						EnableOnuPeerToPeer( PonPortIdx, OnuIdx,OnuIdx1);
						OnuNeedP2P = 1;
					}
				}

				if ( OnuNeedP2P )
				{
					OLT_SetOnuP2PMode(PonPortIdx, TRUE);
				}
			}
#endif	

			/* added by chenfj 2008-3-28
			   恢复ONU 上行业务优先级配置*/
#if( EPON_MODULE_PON_LLID_VLAN == EPON_MODULE_YES )
			RestoreUplinkVlanManipulationToPon(PonPortIdx, OnuIdx );
#endif

			if(IsCtcOnu(PonPortIdx, OnuIdx))
			{
				/*  设置CTC-ONU 组播协议模式*/
				VOS_TaskDelay(1);
				CTC_SetOnuMulticastSwitchProtocol(PonPortIdx, OnuIdx );

				/*如果之前升级成功，则执行commit操作*/
				if(getCtcOnuCommitFlag(PonPortIdx, OnuIdx))
				{				
					/*for 10G EPON of PMC8411 change stucture by jinhl @2013-01-16 */
					ret = OnuMgt_CommitOnuFirmware(PonPortIdx, OnuIdx);
					if(ret != CTC_STACK_EXIT_OK)
					{
						ONU_REGISTER_DEBUG("CTC:commit new image fail(%d)!\r\n", ret );
					}
					else
					{
						ONU_REGISTER_DEBUG("CTC:commit new image ok!\r\n", ret );
					}
					clearCtcOnuCommitFlag(PonPortIdx, OnuIdx);
				}

				{
					CTC_STACK_holdover_state_t holdover;
					holdover.holdover_state = CTC_STACK_HOLDOVER_STATE_ACTIVATED;
					holdover.holdover_time = 200;
					ret = OnuMgt_SetHoldOver(PonPortIdx, OnuIdx, &holdover);
					if(ret != CTC_STACK_EXIT_OK)
					{
						ONU_REGISTER_DEBUG("CTC:set holdover time fail(%d)!\r\n", ret );
					}
				}

                /*begin: 增加支持ONU管理IP的配置加载, mod by liuyh, 2017-5-12*/
                if (stOnuMngIp.mxu_mng.data_cvlan != 0)
                {
                    /* 只有配置过才加载管理IP */
                    stOnuMngIp.needSaveOlt = TRUE; 
                    ret = OnuMgt_SetMxuMngGlobalConfig(PonPortIdx, OnuIdx, &stOnuMngIp);   
                }        
        	    /*end: mod by liuyh, 2017-5-12*/
			}

			/*7、modified by xieshl 20111129, 增加主备同步 */
			OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );

			/*8、send register trap to NMS */
			Trap_OnuRegister( PonPortIdx, OnuIdx );

			/*9、注册回调函数，通知其他模块，ONU 注册成功*/
			handler_onuevent_callback(ONU_EVENT_CODE_REGISTER, data);

		}
	}
	else
	{
		VOS_ASSERT(0);
	}
	OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);        
	return VOS_OK;
}
extern int GWGPON_DelSwitchVlanQinQ(short int olt_id, short int onu_id);

int OnuEvent_Deregister(OnuClientData_s OnuClientData)
{
    /*handler function , Body*/
    short int PonPortIdx = OnuClientData.PonPortIdx;
    short int OnuIdx = OnuClientData.OnuIdx;
    short int LLIDIdx = OnuClientData.LLIDIdx;
    short int llidRegState;
    short int llidNum;
	PON_onu_id_t  onu_id;
	PON_onu_deregistration_code_t  deregistration_code = OnuClientData.Data;
    int         iRlt;
    ULONG       slotno, portno;
    ULONG       onuno, onuEntry;
    char        local_mac[6];
    OnuEventData_s data;
   
	slotno = GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);
    onuno  = OnuIdx + 1;
    onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    if( onuEntry >= MAXONU )
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    
    /*2012-11-8，add by luh; 去注册处理之后，超时应该清空*/
    OnuEvent_Clear_RegisterTimer(PonPortIdx, OnuIdx, LLIDIdx);  
   
    ONU_MGMT_SEM_TAKE;
    onu_id = OnuMgmtTable[onuEntry].LlidTable[LLIDIdx].Llid;
    llidRegState = OnuMgmtTable[onuEntry].LlidTable[LLIDIdx].OnuhandlerStatus;
    llidNum = OnuMgmtTable[onuEntry].llidNum;
    VOS_MemCpy(local_mac, OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;

	
#ifdef BCM_DRV_646
	if(SYS_LOCAL_MODULE_TYPE_IS_8000_GPON)
		GWGPON_DelSwitchVlanQinQ(PonPortIdx, OnuIdx);
#endif

    if ( 1 == llidNum )
    {
        /* B--added by liwei056@2010-10-18 for RedundancySwitch-BUG */
        iRlt = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
    	ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d deregister at the OnuStatus(%s).\r\n", slotno, portno, onuno, g_onu_oper_status_str[iRlt] );
          
        if ( iRlt != ONU_OPER_STATUS_UP )
        {
    		/* UP的ONU，必定不在Pending队列里 */
    		UpdatePendingOnu(PonPortIdx, onu_id, 0);
    		UpdatePendingConfOnu(PonPortIdx, onu_id, 0);

            /* 没有上报过注册事件的ONU，其离线事件也不能上报 */
        	if ( iRlt == ONU_OPER_STATUS_DOWN )
            {
            	ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d deregister ERR(onu device status=%d)\r\n", slotno, portno, onuno, iRlt);
        		return( RERROR );
            }
        }
        /* E--added by liwei056@2010-10-18 for RedundancySwitch-BUG */
        
        data.PonPortIdx = PonPortIdx;
        data.OnuIdx = OnuIdx;
        data.llid = onu_id;
        VOS_MemCpy(data.onu_mac, OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, 6);    
        handler_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, data);
        
    	/* add by shixh20100823,取消ONU管理接口挂接*/
    	ONU_SetupIFs(PonPortIdx, OnuIdx, ONU_ADAPTER_NULL);
        OnuMgt_SetOnuLLID(PonPortIdx, OnuIdx, INVALID_LLID);
           

    	/* added by chenfj 2006/09/22 */ 
    	/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
    	/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/

#if 0
    	/*add by shixh20100921,6700应该在这是删除，6900要挪到主控板上删除，这样才能保证PON板和主控的pending onu 一致*/
    	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )    /*add by shixh20101124*/
        {
            DelPendingOnu( PonPortIdx, llid );
            DelPendingOnu_conf( PonPortIdx, llid );
        }
#endif
    	
    	/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
    	VOS_SemTake( OnuPendingDataSemId, WAIT_FOREVER );
    	iRlt = (PonPortTable[PonPortIdx].PendingOnu_Conf.Next == NULL) && (PonPortTable[PonPortIdx].PendingOnu.Next == NULL);
    	VOS_SemGive( OnuPendingDataSemId );
    	if( iRlt )
    	{
    		Trap_PonPortFullClear(PonPortIdx);
    	}		
    	
#if 0
    	if(( reason_code < PON_ONU_DEREGISTRATION_REPORT_TIMEOUT) ||(reason_code >= PON_ONU_DEREGISTRATION_LAST_CODE ))
    	{
    		if( EVENT_REGISTER == V2R1_ENABLE )
    			sys_console_printf("  error: pon%d llid%d deregister reason %d\r\n",PonPortIdx, llid, reason_code );
    	}
#endif

#if 1    
    	if( PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_UP )
    	{
    		DecreasePonPortRunningData( PonPortIdx,  OnuIdx);
    	}
#else
        OnuEvent_Free_BandWidth(PonPortIdx, OnuIdx);
#endif

    	/*1 send ONU Not present Trap */
    	Trap_OnuDeregister( PonPortIdx, OnuIdx, deregistration_code, 0 );

    	/*RecordOnuUpTime( onuEntry );*/
    	
    	/*2 if ONU existing alarm , send alarm clear Trip to NMS */

    	/*3 Clear the onu mgmt table running data */
    	/*sys_console_printf(" onu status %d \r\n", GetOnuOperStatus(PonPort_id, OnuIdx));*/

    	/*OnuEncryptionOperation(PonPort_id, OnuIdx,  PON_ENCRYPTION_PURE);*/
    	
    	/*if(GetOnuOperStatus(PonPort_id, OnuIdx ) == ONU_OPER_STATUS_UP )
    		{
    		if( EVENT_REGISTER == V2R1_ENABLE )
    			sys_console_printf("\r\n   onu %d/%d/%d Deregistered\r\n", slotno, portno, onuno, onu_id );
    		}*/
    	if(GetOnuOperStatus_1(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_DOWN )
    		PonPortTable[PonPortIdx].OnuDeregisterMsgCounter ++;
    	
    	Trap_OnuPonPortBER( PonPortIdx, OnuIdx, 0, V2R1_BER_CLEAR );
    	Trap_OnuPonPortFER( PonPortIdx, OnuIdx, 0, V2R1_FER_CLEAR );
    	/* modified by xieshl 20101220, 问题单11740 */
    	updateOnuOperStatusAndUpTime(onuEntry, ONU_OPER_STATUS_DOWN);
    	/* modified for 共进测试chenfj 2007-8-27*/
#ifndef GONGJIN_VERSION
    	ClearOnuRunningData(PonPortIdx, OnuIdx, 0);
#else 
    	InitOneOnuByDefault(PonPortIdx, OnuIdx );

    	ONU_MGMT_SEM_TAKE;
    	OnuMgmtTable[onuEntry].RegisterFlag = NOT_ONU_FIRST_REGISTER_FLAG;
    	ONU_MGMT_SEM_GIVE;
#endif
	ONU_MGMT_SEM_TAKE;
	OnuMgmtTable[onuEntry].DrPengSupportAttribute.get_flag = 0;/*delete DrPeng get flag .add by yangzl@2017-6-5*/
	ONU_MGMT_SEM_GIVE;

    	if( OnuMgmtTable[onuEntry].NeedDeleteFlag == TRUE )
    	{
    		InitOneOnuByDefault(PonPortIdx, OnuIdx);		
    	}

        /* modified by xieshl 20111129, 增加主备同步 */
        OnuMgtSyncDataSend_Deregister( PonPortIdx, OnuIdx );
    }
    else
    {
    	ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d's llid(%d) deregister at the llidStatus(%s).\r\n", slotno, portno, onuno, onu_id, g_onu_register_status_str[llidRegState] );

        /* 清除离线LLID */
        ClearOnuLlidRunningData(PonPortIdx, OnuIdx, LLIDIdx);

        if ( ONU_FINISH == llidRegState )
        {
            /* 使得带宽刷新消息，在任务队列里尽量在所有Link的离线消息之后 */
            VOS_TaskDelay(VOS_TICK_SECOND); 
            
            /* 通知需重新映射优先级(重新分配带宽，丢LLID不丢带宽) */
            OnuEvent_SetBandWidthMsg_Send(PonPortIdx, OnuIdx, -1, OLT_CFG_DIR_BOTH);
        }
        else
        {
    		/* 注册完成的LLID，必定不在Pending队列里 */
    		UpdatePendingOnu(PonPortIdx, onu_id, 0);
    		UpdatePendingConfOnu(PonPortIdx, onu_id, 0);
        }
    }
    
	/*OltOamPtyOnuLoseNoti(PonPortIdx, onuno);	*/
    return VOS_OK;
}
#endif

#if 1
int OnuEvent_Register_Notify_OnuMgtModule( OnuEventData_s data )
{
    short int PonPortIdx = data.PonPortIdx;
    short int OnuIdx = data.OnuIdx;
    short int slot = GetCardIdxByPonChip(PonPortIdx);    
    short int port = GetPonPortByPonChip(PonPortIdx);
	unsigned char  isNeedToRestoreConfFile = TRUE;

	GetNeedToRestoreConfFile(PonPortIdx, OnuIdx, &isNeedToRestoreConfFile);
    /*modi by lu 2014-05-08, 集中式设备备用主控不能处理配置恢复过程*/
    if(((SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && !SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
        || slot == SYS_LOCAL_MODULE_SLOTNO) && isNeedToRestoreConfFile /* added by wangjiah,no need to restore while quick swaping*/)
    {
		OnuEvent_Register_Set_Slave_BW(PonPortIdx, OnuIdx + 1);/*Set slave bandwidth as onu register when pon protect is set*/
    	addOnuToRestoreQueue(PonPortIdx, OnuIdx, ONU_CONF_RES_ACTION_DO, OnuProfile_Part_CTC);
		ONU_REGISTER_DEBUG("\r\nadd olt %d onu %d to restore queue, conf:%s\r\n",PonPortIdx, OnuIdx + 1,getOnuConfNamePtrByPonId(PonPortIdx, OnuIdx));
    }

	SetNeedToRestoreConfFile(PonPortIdx, OnuIdx, TRUE);/*Set true for not-quick swaping, then onu can restore conf when re-register*/
    /*不论是master还是slave都需要清一下告警，熄灭告警灯2013-07-12*/
    /*只有注册成功才能清除告警，2014-03-12*/
    if(ONU_OPER_STATUS_UP == GetOnuOperStatus(PonPortIdx, OnuIdx))
        clearIllegalOnuAlarmStatus(MAKEDEVID(slot, port, OnuIdx+1));

    return VOS_OK;
}

/*added by wangjiah@2017-01-18 for decrease quick swaping time*/
/* 需要将ONU的带宽配置拷贝设置到备用OLT上 */
static int OnuEvent_Register_Set_Slave_BW(const short int olt_id,const short int onu_id)
{
	PLATO3_SLA_t up_bw;
	short   error_code;
	PON_policing_parameters_t down_bw;
	bool   enFlag;
	short int                    result;
    PON_redundancy_olt_state_t   state;
    PON_redundancy_type_t       redundancy_type;
    
    bool set_slave_needed = FALSE;
    short int slave_olt_id = REDUNDANCY_OLT_ID_INVALID;   
   
    result = get_redundancy_state (olt_id, &state);
	if (result != REDUNDANCY_MNG_EXIT_OK)
    {
        ONU_REGISTER_DEBUG("Error getting redundancy state from OLT %d \n", olt_id);
        return (result);
    }
    /*Check if need to set slave OLT or local DB*/
    if(state == PON_OLT_REDUNDANCY_STATE_MASTER)
    {
        result = get_olt_redundancy_mapping_type(&redundancy_type);
        if (result != REDUNDANCY_MNG_EXIT_OK)
        {
            ONU_REGISTER_DEBUG("Error getting redundancy type from Redundancy MNG\n");
            return (result);
        }
        if(redundancy_type == PON_REDUNDANCY_TYPE_1_1) 
        {
            /*check if slave OLT exist*/
            result = get_pair_slave_olt_id(olt_id, &slave_olt_id, &state);
            if (result != REDUNDANCY_MNG_EXIT_OK)
            {
                ONU_REGISTER_DEBUG("Error getting redundancy state from OLT %d \n", olt_id);
                return (result);
            }

            if((slave_olt_id != REDUNDANCY_OLT_ID_INVALID) && (state == PON_OLT_REDUNDANCY_STATE_SLAVE))
            {
                set_slave_needed = TRUE;
            }
		}
    }

	if(set_slave_needed)
	{
		/* 获得ONU上行带宽设置 */
		result = OLTAdv_GetLLIDSLA(olt_id, onu_id, &up_bw, &error_code);
		if ( (EXIT_OK != result) || (PLATO3_ECODE_NO_ERROR != (result = error_code)) )
		{
			ONU_REGISTER_DEBUG("Error, PLATO3_get_SLA failed (code %d) for ONU %d of OLT %d\n", 
					result, onu_id, olt_id);
			return (result);
		}

		/* 获得ONU下行带宽设置 */
		result = OLTAdv_GetLLIDPolice(olt_id, onu_id, PON_POLICER_DOWNSTREAM_TRAFFIC, &enFlag, &down_bw);
		if ( EXIT_OK != result )
		{
			ONU_REGISTER_DEBUG("Error, PAS_get_policing_parameters failed (code %d) for ONU %d of OLT %d\n", result, onu_id, olt_id);
			return (result);
		}

		/* ONU上行带宽设置拷贝 */
		result = OLTAdv_SetLLIDSLA(slave_olt_id, onu_id, &up_bw, &error_code);
		ONU_REGISTER_DEBUG("OLTAdv_SetLLIDSLA(%d,%d,(%d ~ %d),%d), result(%d)\r\n",slave_olt_id, onu_id,up_bw.max_gr_bw,up_bw.max_be_bw,error_code, result);
		if ( (EXIT_OK != result) || (PLATO3_ECODE_NO_ERROR != (result = error_code)) )
		{
			ONU_REGISTER_DEBUG("Error, PLATO3_set_SLA failed (code %d) for ONU %d of OLT %d\n", result, onu_id, slave_olt_id);
			return (result);
		}

		/* ONU下行带宽设置拷贝 */
		result = OLTAdv_SetLLIDPolice(slave_olt_id, onu_id, PON_POLICER_DOWNSTREAM_TRAFFIC, enFlag, down_bw);
		if ( EXIT_OK != result )
		{
			ONU_REGISTER_DEBUG("Error, PAS_set_policing_parameters failed (code %d) for ONU %d of OLT %d\n", 
					result, onu_id, slave_olt_id);
			return (result);
		}
	}
	return result;
}

int OnuEvent_Register_Notify_PonModule( OnuEventData_s data )
{
	short int PonPortIdx = data.PonPortIdx;
	short int OnuIdx = data.OnuIdx;
	short int llid = data.llid;
	short int PonChipIdx_swap = -1;
	short int standby_onu_idx = -1;
	int ret = 0;
	short int slot = GetCardIdxByPonChip(PonPortIdx);
	/*Begin:for onu swap by jinhl@2013-04-27*/
	/*若为单mac双光口的onu保护倒换，注册时添加对端虚注册*/
	int swap_mode = 0;
	int swap_status = 0;
	int brdIdx = GetCardIdxByPonChip( PonPortIdx );
	int portIdx = GetPonPortByPonChip( PonPortIdx );
	int onuIdx = OnuIdx + 1;
	bool exist = FALSE;
	/*在板子启动时，主控上会将存在onu列表的内容调用进入此函数但llid为-1*/
	if(INVALID_LLID == llid )
		return VOS_OK;
	ret = OLTAdv_GetHotSwapMode(PonPortIdx, &PonChipIdx_swap, &swap_mode, &swap_status);
	ONU_SWITCH_DEBUG("\r\nNotify_PonModule,slot:%d,PonPortIdx:%d,OnuIdx:%d,llid:%d,prectype:%d\r\n",slot,PonPortIdx,OnuIdx,llid,GetOnuProtectType( PonPortIdx, OnuIdx ));
	if((OLT_ERR_OK == ret) && (V2R1_PON_PORT_SWAP_ONU == swap_mode ))
	{
		if(!SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
			return VOS_OK;
		ret = OLT_IsExist(PonChipIdx_swap, &exist);
		if( (ONU_PROTECT_TYPE_C == GetOnuProtectType( PonPortIdx, OnuIdx )) && /*((VOS_OK == ret) && exist)*/PonPortIsWorking(PonChipIdx_swap) )
		{
			ONU_SWITCH_DEBUG("\r\n This llid(%d/%d/%d:%d) support onu redundancy\r\n", brdIdx, portIdx, onuIdx, llid );
#if 1
			{
				unsigned long aulMsg[4] = { MODULE_OLT, FC_ONUSWAP_PARTNER_REG, 0, 0 };
				OnuEventData_s *OnuRegData = NULL;

				OnuRegData= (OnuEventData_s *)VOS_Malloc( sizeof(OnuEventData_s), MODULE_OLT);
				if( OnuRegData == NULL )
				{
					sys_console_printf("Error, Malloc buffer not satified %s %d\r\n",__FILE__,__LINE__);
					VOS_ASSERT( 0 );
					return( RERROR );
				}
				VOS_MemCpy((void *)OnuRegData, (void *)&data, sizeof(OnuEventData_s));
				aulMsg[2] = sizeof(OnuEventData_s);
				aulMsg[3] = (int)OnuRegData;

				ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
				if( ret !=  VOS_OK )
				{
					VOS_ASSERT( 0 );
					VOS_Free((void *)OnuRegData);
				}
			}
#endif
		}
		return VOS_OK;
	}
	/*End:for onu swap by jinhl@2013-04-27*/
#if 0 
	if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		return VOS_OK;
#else
	/*if((SYS_LOCAL_MODULE_ISMASTERACTIVE&&!SYS_MODULE_SLOT_ISHAVECPU(slot))|| slot!=SYS_LOCAL_MODULE_SLOTNO)
	  return VOS_OK;*/
#endif
	if((SYS_LOCAL_MODULE_WORKMODE_ISMASTER && !SYS_MODULE_IS_CPU_PON(slot)) || slot == SYS_LOCAL_MODULE_SLOTNO)
	{    
#if ( EPON_MODULE_PON_PORT_HOT_SWAP == EPON_MODULE_YES )
		/* clear pon port swap counter */	
		if( PonPortSwapEnableQuery( PonPortIdx ) == V2R1_PON_PORT_SWAP_ENABLE )
		{
			if( PonPortHotStatusQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_PASSIVE )
			{
				/* modified by wangjiah@2017-04-27 sync configuration when swap_mode is quick or slow */
				swap_mode = GetPonPortHotSwapMode(PonPortIdx);
				if ( V2R1_PON_PORT_SWAP_QUICKLY == swap_mode || V2R1_PON_PORT_SWAP_SLOWLY == swap_mode )
				{
					PonPortSwapPortQuery(PonPortIdx, &PonChipIdx_swap);
				}
			}
		}
		if (PonChipIdx_swap >= 0)
		{
			/* 需要等待备用PON口上此ONU的虚注册成功后，再同步带宽等设置 */
			if ( PonPortIsNotSwaping(PonPortIdx) )
			{
				/* 倒换期间的注册，无需同步 */
				unsigned long aulMsg[4] = { MODULE_PON, FC_ONU_PARTNER_REG, 0, 0 };

				/* 备用ONU记录的端口下索引必须与主用一致，且它们的onu注册ID和MAC必须相同 */
				/* 使得从主用ONU记录可以得到其备用ONU记录 */
				standby_onu_idx = OnuIdx;

				aulMsg[2] = (PonChipIdx_swap << 16 ) | (PonPortIdx & 0xFFFF);  
				aulMsg[3] = (standby_onu_idx << 16 ) | (OnuIdx & 0xFFFF);

				ret = VOS_QueSend( g_Olt_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
				if( ret !=  VOS_OK )
				{
					VOS_ASSERT( 0 );
				}	
			}
		}
#endif            
	}
	return ret;
}
int OnuEvent_Register_Notify_ArpModule( OnuEventData_s data )
{
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && gAutoClearArpflag)    
        ClearArpWithOnuNotPresent();
    return VOS_OK;
}

int OnuEvent_Register_Notify_Statis( OnuEventData_s data )
{
	short int PonPortIdx = data.PonPortIdx;
    short int OnuIdx = data.OnuIdx;
	short int llid = data.llid;
	int onuIdx = OnuIdx + 1;
	int portNum = 0;
	int i = 0;
	CTC_STACK_statistic_state_t state;
	if(!SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER)
		return VOS_OK;
	
	portNum = getOnuEthPortNum(PonPortIdx, OnuIdx);
	
	state.status = CTC_STACK_STATISTIC_STATE_ENABLE;
	for(i = 1; i <= portNum; i++)
	{
		OnuMgt_SetOnuPortStatisticState(PonPortIdx, OnuIdx, i, &state);
	}
	
}
int OnuEvent_Deregister_Notify_OnuMgtModule( OnuEventData_s data )
{
    short int PonPortIdx = data.PonPortIdx;
    short int OnuIdx = data.OnuIdx;
    short int slot = GetCardIdxByPonChip(PonPortIdx);
#if 0	
    if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        return VOS_OK;
#else
    /*if((SYS_LOCAL_MODULE_ISMASTERACTIVE&&!SYS_MODULE_SLOT_ISHAVECPU(slot))|| slot!=SYS_LOCAL_MODULE_SLOTNO)
		return VOS_OK;*/
#endif
    /*modi by luh 2014-11-11,先这么改吧，后续再完善宏定义*/
    if((SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && !SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
        || slot == SYS_LOCAL_MODULE_SLOTNO)
    {
	    delOnuFromRestoreQueue(PonPortIdx, OnuIdx);
    }
    return VOS_OK;
}
int OnuEvent_Deregister_Notify_OnuPtyModule( OnuEventData_s data )
{
    short int PonPortIdx = data.PonPortIdx;
    short int OnuIdx = data.OnuIdx;
    /*moved by luh 2012-11-21, 问题单16369*/
    /*if(!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        return VOS_OK;*/
    
    OltOamPtyOnuLoseNoti(PonPortIdx, OnuIdx+1);
    return VOS_OK;
}
int OnuEvent_Deregister_Notify_ArpModule( OnuEventData_s data )
{
    short int PonPortIdx = data.PonPortIdx;
    short int slot = GetCardIdxByPonChip(PonPortIdx);   
    short int port = GetPonPortByPonChip(PonPortIdx);
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && gAutoClearArpflag)    
        ClearArpWithOnuNotPresent();
    return VOS_OK;
}
int OnuEvent_Register_Notify_PonEvtModule( OnuEventData_s data )
{
    int iRlt;
    short int PonPortIdx  = data.PonPortIdx;
    short int PonPortLLID = data.llid;
    
    if( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )    
        iRlt = sendOnuExtOamOverMsg(PonPortIdx, PonPortLLID, data.onu_mac, CTC_DISCOVERY_STATE_COMPLETE);

    return VOS_OK;
}
int OnuEvent_Deregister_Notify_PonPPModule( OnuEventData_s data )
{
    int iRlt;
    short int PonPortIdx  = data.PonPortIdx;
    short int PonPortLLID = data.llid;
    
    if( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )    
        iRlt = pon_pp_remove_link(PonPortIdx, PonPortLLID);

    return VOS_OK;
}
#endif
void OnuEvent_Data_init()
{
    if(OnuEventCallBackfuctionSemId == 0)
    	OnuEventCallBackfuctionSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );	
    
    /*g_OnuEvent_FunctionList->code = 0;
    g_OnuEvent_FunctionList->func = NULL;
    g_OnuEvent_FunctionList->next = NULL;*/
}

void OnuEvent_Callback_init()
{   
    /* 1.begin:register event handler function*/
    reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, OnuEvent_Register_Notify_OnuMgtModule);
    reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, OnuEvent_Register_Notify_PonModule);
    reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, OnuEvent_Register_Notify_ArpModule);    
	reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, OnuEvent_Register_Notify_Statis);    
    /*end*/

    /* 2.begin:deregister event handler function*/
    reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, OnuEvent_Deregister_Notify_OnuMgtModule);
    reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, OnuEvent_Deregister_Notify_OnuPtyModule);
    reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, OnuEvent_Deregister_Notify_ArpModule);    
    if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
    {
        reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, OnuEvent_Register_Notify_PonEvtModule);
        reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, OnuEvent_Deregister_Notify_PonPPModule);    
    }
    /*end*/
    
    /* 3.begin:remove onu event handler function*/
    reigster_onuevent_callback(ONU_EVENT_CODE_REMOVE, OnuEvent_Deregister_Notify_OnuPtyModule);
    reigster_onuevent_callback(ONU_EVENT_CODE_REMOVE, OnuEvent_Deregister_Notify_OnuMgtModule);    
    /*end*/    
}
void OnuEvent_init()
{
    OnuEvent_Data_init();
    OnuEvent_Callback_init();
}

#ifdef __cplusplus
}
#endif

