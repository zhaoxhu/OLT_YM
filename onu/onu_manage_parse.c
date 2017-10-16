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
#include "gwEponSys.h"
#include "onu/onuConfMgt.h"

#if 1
static int check_onu_register_parameter(short int PonPort_id, short int llid, unsigned char *mac_address, unsigned int event_flag)
{
    short int onu_idx = 0;
    int OnuEntry = 0;
    short int local_slot, local_port;
    
	if( (!OLT_LOCAL_ISVALID(PonPort_id) ) || (!LLID_ISVALID(llid)) )
	{
        ONU_REGISTER_DEBUG("\r\n ONU Reg. unknown LLID (pon=%d,llid=%d)\r\n", PonPort_id, llid );
		return( RERROR );
	}

    /*new added by luh 2014-07-04, 解决PON LOS以及PON LOS CLEAR 漏报问题*/
	/* modified by xieshl 20111103, 解决PON LOS clear告警漏报问题，如果ONU是第一个注册则上报一次，问题单13210 */
	if( PonPortTable[PonPort_id].SignalLossFlag == V2R1_ENABLE )
	{
		Trap_PonPortSignalLoss( PonPort_id, V2R1_DISABLE );
	}
    
    local_slot = GetCardIdxByPonChip( PonPort_id );
	local_port = GetPonPortByPonChip( PonPort_id );  
    if( (!OLT_SLOT_ISVALID(local_slot)) || (!OLT_PORT_ISVALID(local_port)) )
	{
        ONU_REGISTER_DEBUG("\r\n ONU Reg. unknown slot%d, port%d (pon=%d,llid=%d)\r\n", local_slot, local_port, PonPort_id, llid );
		VOS_ASSERT(0);
		return( RERROR );
	}

    /* 确保此LLID已经无其它ONU占用 */
    while ( RERROR != (onu_idx = GetOnuIdxByLlid(PonPort_id, llid)) )
    {
        int OnuStatus;
        bool DoubleRegFlag;
        
        OnuEntry = PonPort_id * MAXONUPERPON + onu_idx;

        ONU_MGMT_SEM_TAKE;
        OnuStatus = OnuMgmtTable[OnuEntry].OperStatus;
        DoubleRegFlag = MAC_ADDR_IS_EQUAL(mac_address, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr);
        ONU_MGMT_SEM_GIVE;

        if ( DoubleRegFlag )
        {
            /* 注册的LLID已被ONU自己占用 */
            if( ONU_OPER_STATUS_DOWN == OnuStatus )
            {
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d resume onu%d's real registering status by %s-register.\r\n", PonPort_id, onu_idx+1, (event_flag & ONU_EVENTFLAG_REAL) ? "real" : "virtual");
                break;
            }
            else
            {
                if (event_flag & ONU_EVENTFLAG_REAL)
                {
                    if ( ONU_OPER_STATUS_UP == OnuStatus )
                    {
                        ONUDeregistrationInfo_S stDeregEvt;

                        /* 真实注册的LLID已被ONU自己占用，则需重新注册此LLID */
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event by double-register(llid%d).\r\n", PonPort_id, onu_idx+1, llid);
                        stDeregEvt.olt_id = PonPort_id;
                        stDeregEvt.onu_id = llid;
                        stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
                        stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
                        OnuDeregisterHandler(&stDeregEvt);
                    }
                    /* B--added by liwei056@2012-7-23 for D15483 */
                    else
                    {
                        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d clear onu%d's virtual registering status by double-register(llid%d).\r\n", PonPort_id, onu_idx+1, llid);
                        ClearOnuRunningData( PonPort_id, onu_idx, 0 );
                    }
                    /* E--added by liwei056@2012-7-23 for D15483 */
                    
                }
                else
                {
                    /* 虚拟注册的LLID已经UP，则此虚拟注册无效 */
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual register event for onu%d have register the llid by real-register.\r\n", PonPort_id, llid, onu_idx+1);
            		return( RERROR );
                }
            }
        }
        else
        {
            /* 注册的LLID被其他ONU占用 */
            if (event_flag & ONU_EVENTFLAG_REAL)
            {
                /* 真实注册的LLID被别的ONU占用，则需抢夺此LLID */
                if( ONU_OPER_STATUS_UP == OnuStatus )
                {
                    ONUDeregistrationInfo_S stDeregEvt;

                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event by real-register(llid%d).\r\n", PonPort_id, onu_idx+1, llid);
                    stDeregEvt.olt_id = PonPort_id;
                    stDeregEvt.onu_id = llid;
                    stDeregEvt.deregistration_code = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
                    stDeregEvt.event_flags = ONU_EVENTFLAG_VIRTAL;
                    OnuDeregisterHandler(&stDeregEvt);
                }
                else
                {
                    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d clear onu%d's virtual registering status by real-register(llid%d).\r\n", PonPort_id, onu_idx+1, llid);
                    ClearOnuRunningData( PonPort_id, onu_idx, 0 );
                }
            }
            else
            {
                /* 虚拟注册的LLID被别的ONU占用，则此虚拟注册无效 */
                OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual register event for onu%d have occupied the llid by real-register.\r\n", PonPort_id, llid, onu_idx+1);
        		return( RERROR );
            }
        }
    }    

    return VOS_OK;
}
static int check_onu_pending_queue(short int PonPort_id, short int llid, unsigned char *mac_address)
{
	/*3、PonChipIdx = GetPonChipIdx( PonPort_id );*/
	/*判断此ONU是否在PENDING-CONF 队列中，如果在，立刻退出注册处理流程*/
    short int slot = GetCardIdxByPonChip( PonPort_id );
	short int port = GetPonPortByPonChip( PonPort_id );     
	if(PONid_ONUMAC_BINDING == V2R1_ENABLE)
	{
		if(Check_Onu_Is_In_PendingConf_Queue(PonPort_id, mac_address) == ROK)
		{
			sys_console_printf( "\r\n find onu-pon binding:pon%d/%d,mac %02x%02x.%02x%02x.%02x%02x conflict pending\r\n",
						slot, port, mac_address[0], mac_address[1], mac_address[2],mac_address[3], mac_address[4], mac_address[5] );
			return VOS_ERROR;
		}
	}   
    
    return VOS_OK;
}
static int check_ponport_valid(short int PonPort_id, short int llid, unsigned char *mac_address, short int *PonChipVer)
{
	short int PonChipType = GetPonChipTypeByPonPort(PonPort_id );

	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
        *PonChipVer = PonChipType;
        PonChipType = PONCHIP_PAS;
	}
    
	/* 5、modified by xieshl 20110509, PON板上在进行合法性检查时，同时删除其它PON下该mac地址的ONU */
	if( checkOnuRegisterControl( PonChipType, PonPort_id, llid, mac_address ) == VOS_ERROR )
		return VOS_ERROR;
    
    return VOS_OK;
}
static int check_onu_table(short int PonPort_id, short int llid, unsigned char *mac_address, short int PonChipType, short int *OnuIdx, short int *llidIdx)
{
    UCHAR traffic_enable = 0;
    UCHAR oam_status = 0;
    int OnuEntry = 0;
    short int reg_flag = 0;
    short int PonChipVer = GetPonChipTypeByPonPort(PonPort_id);    
    short int slot = GetCardIdxByPonChip(PonPort_id);
    short int port = GetPonPortByPonChip(PonPort_id);
	short int onu_idx = SearchFreeOnuIdxForRegister(PonPort_id, mac_address, &reg_flag);
    short int llid_idx = 0;
    
	if( onu_idx == RERROR )	/* 注册满 */
	{
		/* modify by chenfj 2006/09/21 */
		/*#2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
		ONU_REGISTER_DEBUG("\r\n no g_free entry for this onu(pon=%d/%d,llid=%d\r\n", slot, port, llid );
		
		/* LOG info : too many onu registered */

		if( PonChipType == PONCHIP_PAS )
		{
		    OLT_AuthorizeLLID(PonPort_id, llid, FALSE);
			/*PAS_authorize_onu(PonPort_id, llid, PON_DENY_FROM_THE_NETWORK );*/
			/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );*/
		}
		
		/* added by chenfj 2006/09/22 */ 
		/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
		/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
		
		AddPendingOnu( PonPort_id, -1, llid, mac_address, PENDING_REASON_CODE_ONU_FULL);

		/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
		Trap_PonPortFull(PonPort_id);
		
		return( RERROR );
	}
	else
	{
		OnuEntry = PonPort_id * MAXONUPERPON + onu_idx;
		if( OnuEntry < 0 || OnuEntry > MAXONU )
		{
			VOS_ASSERT(0);
			AddPendingOnu( PonPort_id, -1, llid, mac_address, 0);
			return RERROR;
		}
		ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d is %s-register, entry is %d \r\n", slot, port, (onu_idx+1), (reg_flag ? "new" : "re"), OnuEntry);

    	ONU_MGMT_SEM_TAKE;
    	oam_status = OnuMgmtTable[OnuEntry].ExtOAM;
    	traffic_enable = OnuMgmtTable[OnuEntry].TrafficServiceEnable;
    	ONU_MGMT_SEM_GIVE;
        
        /* B--added by liwei056@2011-1-30 for CodeCheck  */
        /* CTC发现未成功之前，都是GW接口 */
        if( oam_status != V2R1_ENABLE )	
        /* E--added by liwei056@2011-1-30 for CodeCheck */
        {
            /* add 20100823,ONU管理接口挂接*/
            ONU_SetupIFsByType(PonPort_id, onu_idx, PonChipVer, ONU_MANAGE_GW);  /* modied by shixh 20101117  问题单11170 */
            if( EVENT_REGISTER == V2R1_ENABLE )
            {
                sys_console_printf("\r\n onu %d/%d/%d get the LOCAL-GW-ONU service at the OnuRegister time.\r\n", slot, port, (onu_idx + 1) );
            }   
        }
        
		if( reg_flag == 0 )		/* 0-re-register, 1-new register, 2-replaced */
		{
			ONU_MGMT_SEM_TAKE;
            
			/* B--added by liwei056@2011-3-14 for CodeCheck */
			if( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
			{               
                /* B--added by liwei056@2012-11-19 for Tk's Multi-llid ONU's Supported */
                if ( RERROR == (llid_idx = GetOnuLLIDIdx( PonPort_id, onu_idx, llid )) )
                {
    			    llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else
                /* E--added by liwei056@2012-11-19 for Tk's Multi-llid ONU's Supported */
                {
                    /* 重新发现Link，需要补充Link的离线处理 */
    			    OnuClientData_s stDeregEvt;

    			    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for double-register.\r\n", PonPort_id, onu_idx+1);

                    VOS_MemZero(&stDeregEvt, sizeof(stDeregEvt));
                    stDeregEvt.PonPortIdx = PonPort_id;
                    stDeregEvt.OnuIdx = onu_idx;
                    stDeregEvt.LLIDIdx = llid_idx;
                    stDeregEvt.Data = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
    			    OnuEvent_Deregister(stDeregEvt);

                    /* 强制离线后，继续新的注册 */
                    OnuMgmtTable[OnuEntry].llidNum++;
                }
			}
			/* E--added by liwei056@2011-3-14 for CodeCheck */
            else
            {
                /* 只有重新注册的ONU 才需要判断业务是否激活，其他情况下会恢复默认modi by luh 2012-3-15*/
            	/* 判断ONU业务是否去激活，若去激活，则不让ONU注册*/
            	if( traffic_enable == V2R1_DISABLE )
            	{
            		ONU_REGISTER_DEBUG(" onu %d/%d/%d service deactive\r\n", slot, port, (onu_idx + 1));
            		
            		AddPendingOnu( PonPort_id, onu_idx, llid, mac_address, PENDING_REASON_CODE_TRAFFIC_DISABLE);
            		/*REMOTE_PASONU_uni_set_port( PonPort_id, onu_id, 1, 0 );*/
            		/*SetOnuOperStatus(PonPort_id,(OnuEntry % MAXONUPERPON), ONU_OPER_STATUS_DOWN);*/
                    ONU_MGMT_SEM_GIVE;
            		return( RERROR );					
            	}		

                if ( RERROR == (llid_idx = GetOnuLLIDIdx( PonPort_id, onu_idx, llid )) )
                {
    			    llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else
                {
                    /* 重新发现Link，只恢复LLID相关配置，暂无其它处理 */
                }
            }
            
			ONU_MGMT_SEM_GIVE;
		}
		else			/* 1-new register, 2-replaced */
		{
			/* modified by xieshl 20110512, 覆盖旧ONU时上报告警 */
			if( reg_flag == 2 )	/* 0-re-register, 1-new register, 2-replaced */
			{
				ONU_MGMT_SEM_TAKE;
				onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
				ONU_MGMT_SEM_GIVE;

				clearDeviceAlarmStatus( MAKEDEVID(slot, port, (onu_idx+1)) );
				InitOneOnuByDefault(PonPort_id, onu_idx);
			}

			ONU_MGMT_SEM_TAKE;
            OnuMgmtTable[OnuEntry].llidNum = 1;
	    	VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, mac_address, BYTES_IN_MAC_ADDRESS );

            /* add by chenfj 2006-10-30  */
            /*added by chenfj 2006-12-21 
                        设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
                    */
			GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_UPLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_be);
			GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_DOWNLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_be);
            OnuMgmtTable[OnuEntry].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
            OnuMgmtTable[OnuEntry].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
            OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
            OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;
            OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC = MaxMACDefault;
			ONU_MGMT_SEM_GIVE;
		}
	}    

    *OnuIdx  = onu_idx;
    *llidIdx = llid_idx;
        
    return VOS_OK;
}
static int check_gonu_table(short int PonPort_id, short int llid, unsigned char *mac_address, unsigned char *sn, short int PonChipType, short int *OnuIdx, short int *llidIdx)
{
    UCHAR traffic_enable = 0;
    UCHAR oam_status = 0;
    int OnuEntry = 0;
    short int reg_flag = 0;
    short int PonChipVer = GetPonChipTypeByPonPort(PonPort_id);    
    short int slot = GetCardIdxByPonChip(PonPort_id);
    short int port = GetPonPortByPonChip(PonPort_id);
	short int onu_idx = SearchFreeGponOnuIdxForRegister(PonPort_id, llid, sn, &reg_flag);
    short int llid_idx = 0;
    
	if( onu_idx == RERROR )	/* 注册满 */
	{
		/* modify by chenfj 2006/09/21 */
		/*#2606 问题在手工添加64个ONU后，再实际注册新ONU时，出现异常*/
		ONU_REGISTER_DEBUG("\r\n no g_free entry for this onu(pon=%d/%d,llid=%d\r\n", slot, port, llid );
		
		/* LOG info : too many onu registered */

		if( PonChipType == PONCHIP_PAS )
		{
		    OLT_AuthorizeLLID(PonPort_id, llid, FALSE);
			/*PAS_authorize_onu(PonPort_id, llid, PON_DENY_FROM_THE_NETWORK );*/
			/*PAS_shutdown_onu( PonPort_id, onu_id, TRUE );*/
		}
		
		/* added by chenfj 2006/09/22 */ 
		/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
		/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
		
		AddPendingOnu( PonPort_id, -1, llid, sn, PENDING_REASON_CODE_ONU_FULL);

		/* added by chenfj 2009-5-14 , 当这个PON 口下有多于 64个ONU 时，产生告警*/
		Trap_PonPortFull(PonPort_id);
		
		return( RERROR );
	}
	else
	{
		OnuEntry = PonPort_id * MAXONUPERPON + onu_idx;
		if( OnuEntry < 0 || OnuEntry > MAXONU )
		{
			VOS_ASSERT(0);
			AddPendingOnu( PonPort_id, -1, llid, sn, 0);
			return RERROR;
		}
		ONU_REGISTER_DEBUG("\r\n onu %d/%d/%d is %s-register, entry is %d \r\n", slot, port, (onu_idx+1), (reg_flag ? "new" : "re"), OnuEntry);

    	ONU_MGMT_SEM_TAKE;
    	oam_status = OnuMgmtTable[OnuEntry].ExtOAM;
    	traffic_enable = OnuMgmtTable[OnuEntry].TrafficServiceEnable;
    	ONU_MGMT_SEM_GIVE;
        
        /* B--added by liwei056@2011-1-30 for CodeCheck  */
        /* CTC发现未成功之前，都是GW接口 */
        if( oam_status != V2R1_ENABLE )	
        /* E--added by liwei056@2011-1-30 for CodeCheck */
        {
            /* add 20100823,ONU管理接口挂接*/
            ONU_SetupIFsByType(PonPort_id, onu_idx, PonChipVer, ONU_MANAGE_GPON);  /* modied by shixh 20101117  问题单11170 */
            if( EVENT_REGISTER == V2R1_ENABLE )
            {
                sys_console_printf("\r\n onu %d/%d/%d get the LOCAL-GW-ONU service at the OnuRegister time.\r\n", slot, port, (onu_idx + 1) );
            }   
        }
        
		if( reg_flag == 0 )		/* 0-re-register, 1-new register, 2-replaced */
		{
			ONU_MGMT_SEM_TAKE;
            
			/* B--added by liwei056@2011-3-14 for CodeCheck */
			if( ONU_OPER_STATUS_UP == OnuMgmtTable[OnuEntry].OperStatus )
			{               
                /* B--added by liwei056@2012-11-19 for Tk's Multi-llid ONU's Supported */
                if ( RERROR == (llid_idx = GetOnuLLIDIdx( PonPort_id, onu_idx, llid )) )
                {
    			    llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else
                /* E--added by liwei056@2012-11-19 for Tk's Multi-llid ONU's Supported */
                {
                    /* 重新发现Link，需要补充Link的离线处理 */
    			    OnuClientData_s stDeregEvt;

    			    OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d simulate onu%d's deregister event for double-register.\r\n", PonPort_id, onu_idx+1);

                    VOS_MemZero(&stDeregEvt, sizeof(stDeregEvt));
                    stDeregEvt.PonPortIdx = PonPort_id;
                    stDeregEvt.OnuIdx = onu_idx;
                    stDeregEvt.LLIDIdx = llid_idx;
                    stDeregEvt.Data = PON_ONU_DEREGISTRATION_DOUBLE_REGISTRATION;
    			    OnuEvent_Deregister(stDeregEvt);

                    /* 强制离线后，继续新的注册 */
                    OnuMgmtTable[OnuEntry].llidNum++;
                }
			}
			/* E--added by liwei056@2011-3-14 for CodeCheck */
            else
            {
                /* 只有重新注册的ONU 才需要判断业务是否激活，其他情况下会恢复默认modi by luh 2012-3-15*/
            	/* 判断ONU业务是否去激活，若去激活，则不让ONU注册*/
            	if( traffic_enable == V2R1_DISABLE )
            	{
            		ONU_REGISTER_DEBUG(" onu %d/%d/%d service deactive\r\n", slot, port, (onu_idx + 1));
            		
            		AddPendingOnu( PonPort_id, onu_idx, llid, sn, PENDING_REASON_CODE_TRAFFIC_DISABLE);
            		/*REMOTE_PASONU_uni_set_port( PonPort_id, onu_id, 1, 0 );*/
            		/*SetOnuOperStatus(PonPort_id,(OnuEntry % MAXONUPERPON), ONU_OPER_STATUS_DOWN);*/
                    ONU_MGMT_SEM_GIVE;
            		return( RERROR );					
            	}		

                if ( RERROR == (llid_idx = GetOnuLLIDIdx( PonPort_id, onu_idx, llid )) )
                {
    			    llid_idx = OnuMgmtTable[OnuEntry].llidNum++;
                }
                else
                {
                    /* 重新发现Link，只恢复LLID相关配置，暂无其它处理 */
                }
            }
            
			ONU_MGMT_SEM_GIVE;
		}
		else			/* 1-new register, 2-replaced */
		{
			/* modified by xieshl 20110512, 覆盖旧ONU时上报告警 */
			if( reg_flag == 2 )	/* 0-re-register, 1-new register, 2-replaced */
			{
				ONU_MGMT_SEM_TAKE;
				onuDeletingNotify_EventReport( OLT_DEV_ID, slot, port, (onu_idx+1), OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr );
				ONU_MGMT_SEM_GIVE;

				clearDeviceAlarmStatus( MAKEDEVID(slot, port, (onu_idx+1)) );
				InitOneOnuByDefault(PonPort_id, onu_idx);
			}

			ONU_MGMT_SEM_TAKE;
            OnuMgmtTable[OnuEntry].llidNum = 1;
	    	VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, mac_address, BYTES_IN_MAC_ADDRESS );

            /* add by chenfj 2006-10-30  */
			/*added by chenfj 2006-12-21 
                        设置带宽时，增加class, delay, 最大保证带宽，最大可用带宽参
                    */
			GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_UPLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].UplinkBandwidth_be);
			GetOnuDefaultBWByPonRate(PonPort_id, onu_idx, OLT_CFG_DIR_DOWNLINK, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkBandwidth_be);

            OnuMgmtTable[OnuEntry].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
            OnuMgmtTable[OnuEntry].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
            OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
            OnuMgmtTable[OnuEntry].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;
            OnuMgmtTable[OnuEntry].LlidTable[0].MaxMAC = MaxMACDefault;
			ONU_MGMT_SEM_GIVE;
		}
	}    

    *OnuIdx  = onu_idx;
    *llidIdx = llid_idx;
        
    return VOS_OK;
}

#endif

#if 1
/* ****************************************************************
  * 解析onu 事件消息，并进行各项检查并获取onu 索引。
  * 获取成功进入状态机，否则退出处理流程
  ****************************************************************/
static int parse_onu_register_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
	ONURegisterInfo_S *OnuRegisterData;
    int ret = VOS_ERROR;
    short int PonPort_id;
    short int onu_id;
    int slot, port;    
	unsigned int event_flags;
	unsigned char  mac_address[BYTES_IN_MAC_ADDRESS];
	unsigned char authenticatioin_sequence[PON_AUTHENTICATION_SEQUENCE_SIZE];
	OAM_standard_version_t	  supported_oam_standard;
    short int PonChipType;
    int OnuEntry = 0;
    short int OnuIdx = 0;
    short int llidIdx = 0;
	UCHAR icount;
	UCHAR SerialNoAfterSIXDigit[BYTES_IN_MAC_ADDRESS];
	
	UCHAR series_tmp[BYTES_IN_MAC_ADDRESS];
	char equipmentID[MAXEQUIPMENTIDLEN];
	char pModel[80]="";
	ULONG pLen=0;
	ULONG typeid=0;

    if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
    {
        gONURegisterInfo_S *gOnuRegisterData = ( gONURegisterInfo_S *)aulMsg[3];
        char series_number[GPON_ONU_SERIAL_NUM_STR_LEN];
        if( gOnuRegisterData != NULL )        
        {
            event_flags = gOnuRegisterData->event_flags;	
            PonPort_id = gOnuRegisterData->olt_id;
            onu_id = gOnuRegisterData->onu_id;
			
			/*begin: ID, 37748，mod by liub，2017-5-15. 其他厂商mac按sn号后六位填0的规则设置，私有GW按文档要求*/
#if 1
				SerialNoAfterSIXDigit[0]=0x00;
				SerialNoAfterSIXDigit[1]=0x0F;
				SerialNoAfterSIXDigit[2]=0xE9;
				for(icount = 0;icount < 6;icount += 1)
		  		{
		            VOS_StrnCpy(series_number, gOnuRegisterData->series_number, GPON_ONU_SERIAL_NUM_STR_LEN-1);
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

#else
			VOS_StrnCpy(series_number, gOnuRegisterData->series_number, GPON_ONU_SERIAL_NUM_STR_LEN-1);
			
#endif
			/*end: mod by liub，2017-5-15*/
			
        	VOS_MemCpy(mac_address, SerialNoAfterSIXDigit, BYTES_IN_MAC_ADDRESS );
            VOS_MemCpy(authenticatioin_sequence, gOnuRegisterData->authenticatioin_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE);
            supported_oam_standard = gOnuRegisterData->supported_oam_standard;
            VOS_Free(gOnuRegisterData);    
        }
        else
        {
            VOS_ASSERT( 0 );
            return( RERROR );	
        }
		extern int GW_PonLinkFlag(short int olt_id, bool flag);
		GW_PonLinkFlag(PonPort_id,1);
        /*参数合法性检查，包括ponid、llid、slot、port*/
        ret = check_onu_register_parameter(PonPort_id, onu_id, mac_address, event_flags);	
        if(ret != VOS_OK)
            return ret;
        ret = check_gonu_table(PonPort_id, onu_id, mac_address, series_number, PonChipType, &OnuIdx, &llidIdx);
        if(ret != VOS_OK)
            return ret;

        slot = GetCardIdxByPonChip( PonPort_id );
        port = GetPonPortByPonChip( PonPort_id );  
        
        OnuEntry = (PonPort_id)*MAXONUPERPON + OnuIdx;
        ONU_MGMT_SEM_TAKE;
        OnuMgmtTable[OnuEntry].LlidTable[llidIdx].EntryStatus = LLID_ENTRY_ACTIVE;
        OnuMgmtTable[OnuEntry].LlidTable[llidIdx].Llid = onu_id;
        OnuMgmtTable[OnuEntry].llidNum = 1;
        OnuMgmtTable[OnuEntry].IsGponOnu = 1;
        
        /* 只有第一个LLID，才认为是ONU注册；否则认为是增加LLID链路 */
        OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DORMANT;
		VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, SerialNoAfterSIXDigit, BYTES_IN_MAC_ADDRESS );
        VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, series_number, GPON_ONU_SERIAL_NUM_STR_LEN-1);
        OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_NoLen = GPON_ONU_SERIAL_NUM_STR_LEN;
        
        VOS_MemCpy( OnuMgmtTable[OnuEntry].SequenceNo, OnuRegisterData->authenticatioin_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
        OnuMgmtTable[OnuEntry].OAM_Ver = OnuRegisterData->supported_oam_standard;
        OnuMgmtTable[OnuEntry].LLID  = onu_id;
        OnuMgmtTable[OnuEntry].Index.slot = slot;
        OnuMgmtTable[OnuEntry].Index.port = port;
        OnuMgmtTable[OnuEntry].Index.subNo = /* MAXLLID + */ onu_id;
        OnuMgmtTable[OnuEntry].Index.Onu_FE = 0;
		OnuMgmtTable[OnuEntry].onu_ctc_version = 0x21;       
        if(OnuMgmtTable[OnuEntry].UsedFlag != ONU_USED_FLAG)
        {
            OnuMgmtTable[OnuEntry].UsedFlag = ONU_USED_FLAG;
        }
        ONU_MGMT_SEM_GIVE;
        
    }
    else
    {
        OnuRegisterData = ( ONURegisterInfo_S *)aulMsg[3]; 
        if( OnuRegisterData != NULL )
        {
            event_flags = OnuRegisterData->event_flags;	
            PonPort_id = OnuRegisterData->olt_id;
            onu_id = OnuRegisterData->onu_id;
            VOS_MemCpy(mac_address, OnuRegisterData->mac_address, BYTES_IN_MAC_ADDRESS);
            VOS_MemCpy(authenticatioin_sequence, OnuRegisterData->authenticatioin_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE);
            supported_oam_standard = OnuRegisterData->supported_oam_standard;
            VOS_Free(OnuRegisterData);    
        }
        else
        {
            VOS_ASSERT( 0 );
            return( RERROR );	
        }


        /*参数合法性检查，包括ponid、llid、slot、port*/
        ret = check_onu_register_parameter(PonPort_id, onu_id, mac_address, event_flags);	
        if(ret != VOS_OK)
            return ret;
        
        /*Onu 冲突队列检查*/
        ret = check_onu_pending_queue(PonPort_id, onu_id, mac_address);   
        if(ret != VOS_OK)
            return ret;

        /*Mac 地址合法性以及各个pon口下该Mac地址ONU 检查*/
        ret = check_ponport_valid(PonPort_id, onu_id, mac_address, &PonChipType);
        if(ret != VOS_OK)
            return ret;

        /*Onu 管理表检查，给Onu分配索引0~63*/
    	ret = check_onu_table(PonPort_id, onu_id, mac_address, PonChipType, &OnuIdx, &llidIdx);	    
        if(ret != VOS_OK)
            return ret;

        slot = GetCardIdxByPonChip( PonPort_id );
    	port = GetPonPortByPonChip( PonPort_id );  
        
        OnuEntry = (PonPort_id)*MAXONUPERPON + OnuIdx;
        
    	ONU_MGMT_SEM_TAKE;
    	OnuMgmtTable[OnuEntry].LlidTable[llidIdx].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
    	OnuMgmtTable[OnuEntry].LlidTable[llidIdx].Llid = onu_id;
        if ( 1 == OnuMgmtTable[OnuEntry].llidNum )
        {
            /* 只有第一个LLID，才认为是ONU注册；否则认为是增加LLID链路 */
        	OnuMgmtTable[OnuEntry].OperStatus = ONU_OPER_STATUS_DORMANT;
            VOS_MemCpy(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, mac_address, 6);
        	VOS_MemCpy( OnuMgmtTable[OnuEntry].SequenceNo, OnuRegisterData->authenticatioin_sequence, PON_AUTHENTICATION_SEQUENCE_SIZE );
        	OnuMgmtTable[OnuEntry].OAM_Ver = OnuRegisterData->supported_oam_standard;
        	OnuMgmtTable[OnuEntry].LLID  = onu_id;
        	OnuMgmtTable[OnuEntry].Index.slot = slot;
        	OnuMgmtTable[OnuEntry].Index.port = port;
        	OnuMgmtTable[OnuEntry].Index.subNo = /* MAXLLID + */ onu_id;
        	OnuMgmtTable[OnuEntry].Index.Onu_FE = 0;
            
            if(OnuMgmtTable[OnuEntry].UsedFlag != ONU_USED_FLAG)
            {
                OnuMgmtTable[OnuEntry].UsedFlag = ONU_USED_FLAG;
            }
        }
        OnuMgmtTable[OnuEntry].LlidTable[llidIdx].EntryStatus =  LLID_ENTRY_ACTIVE;  
    	ONU_MGMT_SEM_GIVE;
    }
    
    OnuClientData->PonPortIdx = PonPort_id;
    OnuClientData->OnuIdx = OnuIdx;
    OnuClientData->LLIDIdx = llidIdx;
    
    return VOS_OK;
}

/* ****************************************************************
  * 解析onu 事件消息，并进行各项检查并获取onu 索引。
  * 获取成功进入状态机，否则退出处理流程
  ****************************************************************/
static int parse_onu_discovery_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    ExtOAMDiscovery_t  *ExtOAMDiscovery_Data = (ExtOAMDiscovery_t*)aulMsg[3];
    unsigned int   event_flags;
	PON_olt_id_t   olt_id;
	PON_onu_id_t   llid;
	CTC_STACK_discovery_state_t  result;
	unsigned char Number_of_records; 
	CTC_STACK_oui_version_record_t  onu_version_records_list[MAX_OUI_RECORDS];
    short int lPonPortIdx = 0, lOnuIdx = 0, lLLIDIdx = 0;
    unsigned char local_mac[6];
	unsigned char SN[17];
    int OnuEntry = 0;
    short int brdIdx, portIdx; 
    
    if( ExtOAMDiscovery_Data  != NULL )
    {
        event_flags = ExtOAMDiscovery_Data->event_flags;
        olt_id = ExtOAMDiscovery_Data->olt_id;
        llid   = ExtOAMDiscovery_Data->llid;
        result = ExtOAMDiscovery_Data->result;
        if ( 0 < (Number_of_records = ExtOAMDiscovery_Data->Number_of_records) )
        {
            VOS_MemCpy(onu_version_records_list, ExtOAMDiscovery_Data->onu_version_records_list, MAX_OUI_RECORDS);
        }
        VOS_Free((void*)ExtOAMDiscovery_Data);
    }  
    else
    {
        VOS_ASSERT( 0 );
        return( RERROR );
    }
    
    /*check onu discovery data*/
    
    lPonPortIdx = olt_id;
    lOnuIdx = GetOnuIdxByLlid(olt_id, llid);
    
	if( !OLT_LOCAL_ISVALID(lPonPortIdx) )
		return ( RERROR );

	if( !ONU_IDX_ISVALID(lOnuIdx) )
		return ( RERROR );
    
	brdIdx = GetCardIdxByPonChip(lPonPortIdx);
	portIdx = GetPonPortByPonChip(lPonPortIdx);
    OnuEntry = lPonPortIdx * MAXONUPERPON + lOnuIdx;

    if(1/*SYS_LOCAL_MODULE_TYPE_IS_GPON*/)
    {
        ONU_MGMT_SEM_TAKE;
        VOS_MemCpy(local_mac, OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, 6);
		VOS_MemCpy(SN, OnuMgmtTable[OnuEntry].DeviceInfo.DeviceSerial_No, 17);
        ONU_MGMT_SEM_GIVE;
        
        if ( event_flags & ONU_EVENTFLAG_VIRTAL )
        {
        if ( V2R1_ENABLE == OnuMgmtTable[OnuEntry].ExtOAM )
        {
            /* 虚拟发现的LLID已经发现完成，则此虚拟发现无效 */
            OLT_SYNC_DEBUG(OLT_SYNC_TITLE"pon%d loose llid%d's virtual ext-discovery event for onu%d have finish the discovery by real-register.\r\n", lPonPortIdx, llid, lOnuIdx+1);
    		return RERROR;
        }
    	}
    
    	if( (result != CTC_DISCOVERY_STATE_COMPLETE) /*|| (0 == Number_of_records) */)
    	{
		/*sys_console_printf("%s/port%d onu%d CTC Ext OAM discovery failed\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx), (OnuIdx+1) );*/
		ClearOnuExtOAMStatus( lPonPortIdx,  lOnuIdx);
		ONU_MGMT_SEM_TAKE;
    		OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_OTHER;
    		ONU_MGMT_SEM_GIVE;
    		sys_console_printf("\r\n onu%d/%d/%d CTC Ext OAM discovery failed(%d)\r\n", brdIdx, portIdx, lOnuIdx + 1, result );
			if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
				AddPendingOnu(lPonPortIdx, lOnuIdx, llid, SN, PENDING_REASON_CODE_ExtOAM_FAIL);
			else
				AddPendingOnu(lPonPortIdx, lOnuIdx, llid, local_mac, PENDING_REASON_CODE_ExtOAM_FAIL);
            return VOS_ERROR;
        }
    }
    
    lLLIDIdx = GetOnuLLIDIdx(lPonPortIdx, lOnuIdx, llid);
	if( !LLID_IDX_ISVALID(lLLIDIdx) )
		return ( RERROR );

    OnuClientData->PonPortIdx = lPonPortIdx;
    OnuClientData->OnuIdx = lOnuIdx;
    OnuClientData->LLIDIdx = lLLIDIdx;
    
    return VOS_OK;
}

/* ****************************************************************
  * 解析onu 事件消息，并进行各项检查并获取onu 索引。
  * 获取成功进入状态机，否则退出处理流程
  ****************************************************************/
static int parse_onu_in_progress_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    if(aulMsg[3])
    {
        OnuClientData->DataBuffer = (int*)aulMsg[3];
    }
    OnuClientData->PonPortIdx = (short int)aulMsg[0];
    OnuClientData->OnuIdx = aulMsg[2] >> 16;
    OnuClientData->LLIDIdx = aulMsg[2] & 0xffff;
    return VOS_OK;
}
static int parse_onu_timeout_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    OnuClientData->PonPortIdx = aulMsg[2];
    OnuClientData->OnuIdx = aulMsg[3] >> 8;
    OnuClientData->LLIDIdx = aulMsg[3] & 0xFF;
    return VOS_OK;
}

/* ****************************************************************
  * 解析onu 事件消息，并进行各项检查并获取onu 索引。
  * 获取成功进入状态机，否则退出处理流程，并更
  * 新Pending 队列。
  ****************************************************************/
static int parse_onu_deregister_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    unsigned int  event_flags;
	PON_olt_id_t  olt_id;
	PON_onu_id_t  onu_id;
    short int lOnuIdx, lLLIDIdx;
	PON_onu_deregistration_code_t  deregistration_code;    
    ONUDeregistrationInfo_S * OnuDeregistrationData = (ONUDeregistrationInfo_S *)aulMsg[3];

    if( OnuDeregistrationData != NULL )
    {
        event_flags = OnuDeregistrationData->event_flags;
        olt_id = OnuDeregistrationData->olt_id;
        deregistration_code = OnuDeregistrationData->deregistration_code;
        
        /*B-----区分实注册(deregistration_code>0)和虚注册deregistration_code<0-----------------------------------------*/
        if(deregistration_code > 0)
        {
            onu_id = OnuDeregistrationData->onu_id;/*当deregistration_code>0时，为llid；deregistration_code<0时为OnuIdx。*/        
        	if( (!OLT_LOCAL_ISVALID(olt_id) ) || (!LLID_ISVALID(onu_id)) )
        	{
                VOS_Free( (void *)OnuDeregistrationData );      /*added by luh 2013-2-20  用户内存存在泄露可能*/          
        		ONU_REGISTER_DEBUG("\r\n ONU DeReg. unknown LLID (pon=%d, llid=%d)\r\n", olt_id, onu_id );
        		return( RERROR );
        	}
        	ONU_REGISTER_DEBUG("\r\n onu %d/%d/llid(%d) deregistration, reason_code(%d), event_flags(%d).\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), onu_id, deregistration_code, event_flags );
			/*extern int gponOltAdp_PonLink(int oltId, int enable);
			gponOltAdp_PonLink(olt_id,0); */       	
        	/* modified by chenfj 2007/4/6 */
        	/* 问题单#3994: 徐州一台OLT上发现，有时ONU离线消息到不了软件应用层，造成ONU计数错误。*/
        	/* 将这个数据处理提到前面来*/
        	/* pon data table */
        	lOnuIdx = GetOnuIdxByLlid( olt_id, onu_id );
        	if( lOnuIdx == RERROR ) 
        	{
        		/* llid未记录的ONU，必定在Pending队列里 */
        		UpdatePendingOnu(olt_id, onu_id, 0);
        		UpdatePendingConfOnu(olt_id, onu_id, 0);
                VOS_Free( (void *)OnuDeregistrationData );       /*added by luh 2013-2-20  用户内存存在泄露可能*/                    
        		ONU_REGISTER_DEBUG(" onu deregister ERR(get onuid failed)\r\n");
        		return VOS_ERROR;
        	}
            
            	/* mod by luh@2015-04-03  	从pending队列离线的onu有特殊处理，此处只处理在线onu上报loss告警*/
        	if( PonPortTable[olt_id].SignalLossFlag == V2R1_DISABLE )
        	{
        		if( OnRegOnuCounter(olt_id) <= 1 )
        		{
        			Trap_PonPortSignalLoss( olt_id, V2R1_ENABLE );
        		}
        	}
            lLLIDIdx = GetOnuLLIDIdx(olt_id, lOnuIdx, onu_id);
        	if( !LLID_IDX_ISVALID(lLLIDIdx) )
            {
                VOS_Free( (void *)OnuDeregistrationData );      /*added by luh 2013-2-20  用户内存存在泄露可能*/          
                VOS_ASSERT(0);
        		ONU_REGISTER_DEBUG(" onu deregister ERR(get llidIdx failed)\r\n");
        		return ( RERROR );
            }   
        }
        else
        {
            deregistration_code = -deregistration_code;
            lOnuIdx = OnuDeregistrationData->onu_id;
            lLLIDIdx = 0;
        }
        VOS_Free( (void *)OnuDeregistrationData );
    }
    else
    {
        VOS_ASSERT( 0 );
        return( RERROR );
    }

    OnuClientData->PonPortIdx = olt_id;
    OnuClientData->OnuIdx = lOnuIdx;
    OnuClientData->LLIDIdx = lLLIDIdx;
    OnuClientData->Data = deregistration_code;
    
    return VOS_OK;
}
static int parse_onu_setbandwidth_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    OnuClientData->PonPortIdx = (aulMsg[2]>>16)&0xffff;
    OnuClientData->OnuIdx = aulMsg[2]&0xffff;
    OnuClientData->Data = aulMsg[3];
    return VOS_OK;
}
#endif
/* ****************************************************************
  * 解析onu 事件消息，并进行各项检查并获取onu 索引。
  * 获取成功进入状态机，否则退出处理流程
  ****************************************************************/
int parse_onu_event_data(ULONG aulMsg[4], OnuClientData_s *OnuClientData)
{
    int ret = VOS_ERROR;
    switch(aulMsg[1])
    {
        case FC_ONU_REGISTER:
            ret = parse_onu_register_data(aulMsg, OnuClientData);
            break;

        case FC_EXTOAMDISCOVERY:
            ret = parse_onu_discovery_data(aulMsg, OnuClientData);
            break;

        case FC_ONU_REGISTER_INPROCESS:
            ret = parse_onu_in_progress_data(aulMsg, OnuClientData);
            break;
                        
        case FC_ONU_REGISTER_TIMEOUT:            
            ret = parse_onu_timeout_data(aulMsg, OnuClientData);
            break;
        case FC_ONU_DEREGISTER:
            ret = parse_onu_deregister_data(aulMsg, OnuClientData);
            break;
        case FC_ONU_REGISTER_SETBANDWIDTH:
            ret = parse_onu_setbandwidth_data(aulMsg, OnuClientData);
            break;
        default:
            break; 
    }

    return ret;
}

#ifdef __cplusplus
}
#endif
