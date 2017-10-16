#ifdef __cplusplus
extern "C"
{
#endif

#include "Syscfg.h"

#if( RPU_MODULE_IGMP_TVM == RPU_YES )
#include "Igmp_tvm.h"
#include "device/Device_manage.h"
#include "GwttOam/oam_gw.h"
#include "msgcode.h"

#define once_send_num  14
#define CRC_CHECK_INTERVAL_DEFAULT  120

ULONG  g_ulIgmp_TVM_Enabled = VOS_NO;
USHORT TVM_MAX_GROUPS = 128;
long glIgmpTvmDebug = 0;
long tvm_timer_period = CRC_CHECK_INTERVAL_DEFAULT;
long tvm_timer = CRC_CHECK_INTERVAL_DEFAULT;
TVM_Cont_Head_t *TVM_Cont_Head_Info = NULL;
unsigned long OnuTVMCommSemId = 0;

extern  int FTPC_CalCrc32( unsigned char * buffer, int length );
extern VOID get_ipdotstring_from_long( CHAR *ipaddr, int ip );
extern long get_long_from_ipdotstring(char* ipaddr);
extern ULONG DEV_GetPhySlot(VOID);
extern int  PonPortIsWorking( short int PonPortIdx );
extern short int Comm_Oam_ErrParse(short int iRes);
extern int GetCardIdxByPonChip( short int PonChipIdx );
extern int GetPonPortByPonChip( short int PonChipIdx );
extern ULONG DEV_GetPhySlot(VOID);
extern ULONG DEV_IsMySelfMaster(VOID);

typedef int (* ctss_igmp_tvm_enable_f)(unsigned int PonPortIdx,int enable_mode); 
typedef int (* ctss_igmp_tvm_add_f)(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void *vty); 
typedef int (* ctss_igmp_tvm_delItem_f)(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, void *vty);
typedef int (* ctss_igmp_tvm_delAll_f)(unsigned int PonPortIdx, void *vty);
typedef int (* ctss_igmp_tvm_delVid_f)(unsigned int PonPortIdx, USHORT mcastVid, void *vty);
typedef int (* ctss_igmp_tvm_sync_f)(unsigned int PonPortIdx, long syncvalue);
typedef int (* ctss_igmp_tvm_stop_f)(unsigned int PonPortIdx,void *vty);

typedef struct stIgmpTvmMgmtIFs {
	ctss_igmp_tvm_enable_f igmp_tvm_enable_t;
	ctss_igmp_tvm_add_f  igmp_tvm_add_t;
	ctss_igmp_tvm_delItem_f  igmp_tvm_delitem_t;
	ctss_igmp_tvm_delAll_f   igmp_tvm_delall_t;
	ctss_igmp_tvm_delVid_f  igmp_tvm_delvid_t;
	ctss_igmp_tvm_sync_f    igmp_tvm_sync_t;
	ctss_igmp_tvm_stop_f    igmp_tvm_stop_t;
}IgmpTvmMgmtIFs;
/*
const IgmpTvmMgmtIFs *pM_rpcIfs_tvm=NULL, 
const IgmpTvmMgmtIFs *pS_rpcIfs_tvm=NULL;
*/

struct stIgmpTvmMgmtIFs  *pM_rpcIfs_tvm;
struct stIgmpTvmMgmtIFs  *pS_rpcIfs_tvm;

#define IGMP_TVM_DEBUG(x)    if(VOS_YES == glIgmpTvmDebug) {sys_console_printf x;} 

/* usMsgMode */
#define IGMPTVMTOLIC_REQACK             0x0001              /*本消息需要应答*/
#define IGMPTVMTOLIC_ACK                    0x0002              /*应答消息, 操作成功*/
#define IGMPTVMTOLIC_NAK                    0x0003              /*应答消息, 操作失败*/
#ifdef _DISTRIBUTE_PLATFORM_
#define IGMPTVMTOLIC_ACK_END		0x0004
#endif

ULONG RPC_Tvm_CmdIDNumber = 0;

/* usMsgType */
#define IGMPTVMTOLIC_EXECUTE_CMD        0x0001          /* 主控板通知lic执行相应的命令 */
#ifdef _DISTRIBUTE_PLATFORM_
#define IGMPTVMTOLIC_EXECUTE_CMD_CONT		0x0002
#endif

LONG IgmpTvmSyncTimerId=0;

const IgmpTvmMgmtIFs  *IgmpTvmRpcIfs = NULL; 


#define IGMPTVM_PONPORTIDX_ALL    -1

#define IGMPTVM_ASSERT(PonPortIdx)       VOS_ASSERT( ((PonPortIdx) >= 0) && ((PonPortIdx) < MAXPON) )

#define IGMPTVM_PONPORTIDX_ALL    -1
#if 0
#define IGMPTVM_API_CALL(oltid, fun, params) (( (NULL != IgmpTvmRpcIfs) && (NULL != IgmpTvmRpcIfs->fun) ) ? (*IgmpTvmRpcIfs->fun) params : VOS_ERROR)
#else
#define IGMPTVM_API_CALL(oltid, fun, params) ( ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (GetCardIdxByPonChip(oltid) != SYS_LOCAL_MODULE_SLOTNO)&& SYS_MODULE_SLOT_ISHAVECPU(GetCardIdxByPonChip(oltid))) ? (*pM_rpcIfs_tvm->fun) params : (*pS_rpcIfs_tvm->fun) params)
#endif

static int RPC_IgmpTvm_Enable(unsigned int PonPortIdx,int enable_mode)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_enable,PonPortIdx,enable_mode,0,0,0);
		
   	return iRlt;
}
static int IgmpTvm_OK2(unsigned int para1, int para2)
{
	return VOS_OK;
}

static int IgmpTvm_OK5(unsigned int PonPortIdx, ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void * vty)
{
	return VOS_OK;
}

static int RPC_IgmpTvm_AddItem(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void *vty)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_add,PonPortIdx,0,mulIps,mulIpe,mcastVid);
		
   	return iRlt;
}

static int RPC_IgmpTvm_DelItem(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, void *vty)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_delitem,PonPortIdx,0,mulIps,mulIpe,0);
		
   	return iRlt;
}
static int IgmpTvm_OK4(unsigned int PonPortIdx, ULONG mulIps, ULONG mulIpe, void * vty)
{
	return VOS_OK;
}

static int RPC_IgmpTvm_DelAll(unsigned int PonPortIdx, void *vty)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_delall,PonPortIdx,0,0,0,0);
		
   	return iRlt;
}
static int RPC_IgmpTvm_DelVid(unsigned int PonPortIdx,USHORT mcastVid, void *vty)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_delvid,PonPortIdx,0,0,0,mcastVid);
		
   	return iRlt;
}

static int IgmpTvm_OK3(unsigned int para1, USHORT mcastVid, void* para2)
{
	return VOS_OK;
}

static int IgmpTvm_OKa2(unsigned int para1, void* para2)
{
	return VOS_OK;
}

static int RPC_IgmpTvm_Sync(unsigned int PonPortIdx,long syncvalue)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_sync,PonPortIdx,syncvalue,0,0,0);
		
   	return iRlt;
}

static int IgmpTvm_OKb2(unsigned int para1, long para2)
{
	return VOS_OK;
}

static int RPC_IgmpTvm_Stop(unsigned int PonPortIdx,void *vty)
{
	int iRlt;
	
   	iRlt = Igmp_Tvm_RPC_CALL(igmp_tvm_stop,PonPortIdx,0,0,0,0);
		
   	return iRlt;
}

static const IgmpTvmMgmtIFs M_tvm_rpcIfs = {
	RPC_IgmpTvm_Enable,
	RPC_IgmpTvm_AddItem,
	RPC_IgmpTvm_DelItem,
	RPC_IgmpTvm_DelAll,
	RPC_IgmpTvm_DelVid,
	RPC_IgmpTvm_Sync,
	RPC_IgmpTvm_Stop
};

static const IgmpTvmMgmtIFs S_tvm_rpcIfs = {
	IgmpTvm_OK2,
	IgmpTvm_OK5,
	IgmpTvm_OK4,
	IgmpTvm_OKa2,
	IgmpTvm_OK3,
	IgmpTvm_OKb2,
	IgmpTvm_OKa2
};
typedef struct RPC_IgmpTvm_MsgHead
{
    USHORT      usSrcSlot;
    USHORT      usDstSlot;
    ULONG        ulSrcModuleID;
    ULONG        ulDstModuleID;
    USHORT      usMsgMode;
    USHORT      usMsgType;
    ULONG	 ulCmdID;
    USHORT 	 ulCmdType;
    USHORT      ulDstPort;
    ULONG        ResResult;
    ULONG	 para1;
    ULONG        mulIps;
    ULONG        mulIpe;
    USHORT      vid;
    ULONG	 pSendBufLen;
    UCHAR	*pSendBuf;
}RPC_IgmpTvm_MsgHead_S;


int Igmp_Tvm_RPC_CALL(IgmpTvm_Cmd_Type_t  cmd_type,short int PonPortIdx,ULONG para1,
				ULONG mulIps, ULONG mulIpe, USHORT vid)
{
	RPC_IgmpTvm_MsgHead_S *pstSendMsg = NULL;
	RPC_IgmpTvm_MsgHead_S *pstRevData = NULL;
	ULONG ulCmdID = 0;
	ULONG rc = VOS_ERROR , pstSendMsgLen , ulRevLen;
	USHORT ulSlot,ulPort;

	ulSlot = GetCardIdxByPonChip(PonPortIdx);
	ulPort = GetPonPortByPonChip(PonPortIdx);
	/*sys_console_printf("in func: Igmp_Tvm_RPC_CALL. ulSlot is %d\r\n",ulSlot);*/
	if(SYS_MODULE_ISMASTERSTANDBY(SYS_LOCAL_MODULE_SLOTNO))
	{
		/*sys_console_printf("%%The card is MasterStandby card,needn't send cmd to lic.\r\n");*/
		return VOS_OK;
	}

	/*if(SYS_MODULE_IS_PON(SYS_LOCAL_MODULE_SLOTNO))
	{
		sys_console_printf("%%The card is pon card,can't send cmd to lic.\r\n");
		return VOS_ERROR;
	}*/

	/*if ( !DEV_IsMySelfMaster() )
	{
		sys_console_printf("%%The card is slave card,can't send cmd to lic.\r\n");
		return VOS_ERROR;
	}*/

	if ( !SYS_MODULE_IS_READY(ulSlot) )
	{
		return VOS_OK;
	}

	if(!SYS_MODULE_IS_PON(ulSlot))	
	{
		return VOS_OK;
	}
	
	pstSendMsg = ( RPC_IgmpTvm_MsgHead_S* ) CDP_SYNC_AllocMsg( sizeof( RPC_IgmpTvm_MsgHead_S ), MODULE_RPU_IGMPSNOOP );
	if ( NULL == pstSendMsg )
	{
		ASSERT( 0 );
		return VOS_ERROR;
	}
	
	VOS_MemZero((VOID *)pstSendMsg, sizeof( RPC_IgmpTvm_MsgHead_S ) );
	
	pstSendMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
       pstSendMsg->usDstSlot =  GetCardIdxByPonChip(PonPortIdx);
	pstSendMsg->ulSrcModuleID = MODULE_RPU_IGMPSNOOP;
	pstSendMsg->ulDstModuleID = MODULE_RPU_IGMPSNOOP;
	pstSendMsg->usMsgMode = IGMPTVMTOLIC_REQACK;
	pstSendMsg->usMsgType = IGMPTVMTOLIC_EXECUTE_CMD;
	pstSendMsg->ulDstPort = ulPort-1;
	ulCmdID = RPC_Tvm_CmdIDNumber++;
	pstSendMsg->ulCmdID = ulCmdID;
	pstSendMsg->ulCmdType = cmd_type;
	pstSendMsg->para1 = para1;
	pstSendMsg->mulIps = mulIps;
	pstSendMsg->mulIpe = mulIpe;
	pstSendMsg->vid = vid;
	pstSendMsg->pSendBufLen = 0;
	
	pstSendMsgLen = sizeof( RPC_IgmpTvm_MsgHead_S );

	if ( VOS_OK == CDP_SYNC_Call( MODULE_RPU_IGMPSNOOP,ulSlot, MODULE_RPU_IGMPSNOOP, 0,
			pstSendMsg, pstSendMsgLen, ( VOID** ) &pstRevData,&ulRevLen, 3000 ) )  
	{
		if ( pstRevData != NULL)
		{
			if ( pstRevData->usSrcSlot != ulSlot )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			if ( pstRevData->usMsgMode != IGMPTVMTOLIC_ACK_END )
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}

			if ( ulCmdID !=  pstRevData->ulCmdID)
			{
				ASSERT( 0 );
				CDP_SYNC_FreeMsg( pstRevData );
				return VOS_ERROR;
			}
			
			if(pstRevData->ResResult == 1)
			{
				rc = VOS_OK;
			}
			else
				rc = VOS_ERROR;
			/*sys_console_printf("result is %d\r\n",pstRevData->ResResult);*/
			CDP_SYNC_FreeMsg( pstRevData );
			
			pstRevData = NULL;
		}
		else
		{
			ASSERT( 0 );
		}
		return rc;
	}
	else
	{
		/*ASSERT( 0 );*/	/* 配置数据恢复过程中拔插板卡，不可避免会产生这个断言，先去掉，问题单25708 */
		return VOS_ERROR;
	}
	return rc;
}

int Igmp_Tvm_Enable(int enable_mode )
{
	Tvm_Pon_Onu_t *Tvm_Pon_Temp=NULL;

	if( enable_mode == VOS_YES) 
	{
		g_ulIgmp_TVM_Enabled = VOS_YES;
		tvm_timer_period = CRC_CHECK_INTERVAL_DEFAULT;
	}
	else
    	{
        	g_ulIgmp_TVM_Enabled = VOS_NO;
		tvm_timer_period = 120;
	}
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		Tvm_Pon_Temp =(Tvm_Pon_Onu_t *)VOS_Malloc(sizeof(Tvm_Pon_Onu_t),MODULE_RPU_IGMPSNOOP );
		if( NULL == Tvm_Pon_Temp )
	    	{
			/*vty_out(vty, "Failed to apply for memory when it sends the status of enable to the ONUs.\r\n" );*/
			VOS_ASSERT(0);
	        	return VOS_ERROR;
	    	}
		Tvm_Pon_Temp->Calcus = 0;
		Tvm_Pon_Temp->OnuId = 0;
		Tvm_Pon_Temp->PonId = 0;
		if ( VOS_YES == g_ulIgmp_TVM_Enabled )
		{
			Tvm_Pon_Temp->Enable = TVM_ENABLE;
		}
		else
		{
			Tvm_Pon_Temp->Enable = TVM_DISABLE;
		}
		 if(VOS_OK != Send_To_IgmpQue_Tvm((void *)Tvm_Pon_Temp,AWMC_IPMC_PRIVATE_SNOOP_TVM_ABLE))
		{
			VOS_Free(Tvm_Pon_Temp);
			sys_console_printf("Failed to send the status of enable to the ONUs .\r\n" );
			return VOS_ERROR;
		}
	}
	
	return VOS_OK;
	
}

LONG Set_Igmp_Tvm_Enable(unsigned int PonPortIdx,int enable_mode)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_enable_t, (PonPortIdx,enable_mode) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_enable_t, (olt_id,enable_mode))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		Igmp_Tvm_Enable(enable_mode);
	}
    
	return iRlt;

}

VOID IgmpTvm_CMD2LIC_RPC_Callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                               VOID * pReceiveData, ULONG ulReceiveDataLen,
                               VOID **ppSendData, ULONG * pulSendDataLen )
{
    RPC_IgmpTvm_MsgHead_S * RecvpMsg = NULL;
    RPC_IgmpTvm_MsgHead_S *SendpMsg = NULL;
    LONG rc = -1;
    LONG ulSrcSlot = -1 ;
    void *pBuf = NULL;
	
    if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
        return ;

    if ( NULL == pReceiveData )
    {
        VOS_ASSERT( 0 );
        return ;
    }
   
    RecvpMsg = (RPC_IgmpTvm_MsgHead_S *)pReceiveData;

    ulSrcSlot = RecvpMsg->usSrcSlot;

    if( RecvpMsg->usMsgType == IGMPTVMTOLIC_EXECUTE_CMD )
    {
		switch( RecvpMsg->ulCmdType )
		{
			 case igmp_tvm_enable :
				rc = Set_Igmp_Tvm_Enable(RecvpMsg->ulDstPort,RecvpMsg->para1);
				break;

			case igmp_tvm_add :
				rc = Add_Igmp_Tvm_Item(RecvpMsg->ulDstPort,RecvpMsg->mulIps,RecvpMsg->mulIpe,RecvpMsg->vid,pBuf);
				break;

			case igmp_tvm_delitem:
				rc = Del_Igmp_Tvm_Item(RecvpMsg->ulDstPort,RecvpMsg->mulIps,RecvpMsg->mulIpe,pBuf);
				break;

			case igmp_tvm_delall:
				rc = Del_Igmp_Tvm_All(RecvpMsg->ulDstPort,pBuf);
				break;

			case igmp_tvm_delvid:
				rc = Del_Igmp_Tvm_Vid(RecvpMsg->ulDstPort,  RecvpMsg->vid, pBuf);
				break;

			case igmp_tvm_sync:
				rc = Set_Igmp_Tvm_Sync(RecvpMsg->ulDstPort, RecvpMsg->para1);
				break;
			case igmp_tvm_stop:
				rc = Stop_Igmp_Tvm(RecvpMsg->ulDstPort, pBuf);
				break;
			default:
				break;
		}
    }

		SendpMsg = ( RPC_IgmpTvm_MsgHead_S  * ) CDP_SYNC_AllocMsg( sizeof( RPC_IgmpTvm_MsgHead_S), MODULE_RPU_IGMPSNOOP);
		if ( SendpMsg == NULL )
		{
			RPC_Tvm_CmdIDNumber = 0;
			VOS_ASSERT( 0 );
			return ;
		}
		
		SendpMsg->usSrcSlot = ( USHORT ) DEV_GetPhySlot();
		SendpMsg->usDstSlot = ulSrcSlot;
		SendpMsg->ulSrcModuleID = MODULE_RPU_IGMPSNOOP;
		SendpMsg->ulDstModuleID = MODULE_RPU_IGMPSNOOP;
		SendpMsg->usMsgMode = IGMPTVMTOLIC_ACK_END;
		SendpMsg->ulCmdType = RecvpMsg->ulCmdType;
		SendpMsg->pSendBufLen = 0 ;
		if(rc == VOS_OK)	
		{
			SendpMsg->ResResult = 1;
		}
		else
		{
			SendpMsg->ResResult = -1;
		}
		SendpMsg->usMsgType = IGMPTVMTOLIC_EXECUTE_CMD;
		SendpMsg->ulCmdID = RecvpMsg->ulCmdID;
		
		*ppSendData = SendpMsg;
		*pulSendDataLen = sizeof(RPC_IgmpTvm_MsgHead_S);

	    return ;
}

int IgmptvmTimerCallback()
{
	if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && VOS_YES == g_ulIgmp_TVM_Enabled )
	{
	    	if(tvm_timer > 0)
	    	{
	    		if(1==tvm_timer)
	    		{
	    			tvmTimerCallback();
	    			tvm_timer = tvm_timer_period;
	    		}
	    		tvm_timer--;
	    	}
	}
	
	return VOS_OK;

}

int IgmpTvmTimerStart()
{
	LONG interval = 1000;
	
	IgmpTvmSyncTimerId = VOS_TimerCreate( MODULE_RPU_IGMPSNOOP, (unsigned int )0, interval, (void *)IgmptvmTimerCallback, (void *)NULL/*&PonPortIdx*/, VOS_TIMER_LOOP );
	if( IgmpTvmSyncTimerId == VOS_ERROR){
		sys_console_printf("\r\nstart igmp tvm  timer error !\r\n ");
		return VOS_ERROR;
	}
	else
	{
		return IgmpTvmSyncTimerId ;
	}
}

STATUS  TvmInsert_Poncard_Insert_Callback_2(ULONG ulFlag,	ULONG ulChID, ULONG ulDstNode, ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
	return VOS_OK;
}

STATUS funIgmpTvmInit(void)
{
	if(NULL == TVM_Cont_Head_Info)
	{
		TVM_Cont_Head_Info = (TVM_Cont_Head_t *)VOS_Malloc(sizeof(TVM_Cont_Head_t),MODULE_RPU_IGMPSNOOP);
		if( NULL == TVM_Cont_Head_Info )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
		TVM_Cont_Head_Info->Through_Vlan_Group_head = NULL;
		TVM_Cont_Head_Info->TVMCOUNT = 0;
		TVM_Cont_Head_Info->Calculates = 0;
	}
	OnuTVMCommSemId = VOS_SemBCreate(VOS_SEM_Q_FIFO,VOS_SEM_FULL);
	if( 0 == OnuTVMCommSemId )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		CommOltMsgRvcCallbackInit(GW_OPCODE_TVM_RESPONSE, funIgmpTvmOamReqCallBack);
	}
	
	if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA6900_SW || (SYS_LOCAL_MODULE_TYPE == MODULE_E_GFA8000_SW)))
	{
		CDP_Create( RPU_TID_CDP_IGMP_TVM,  CDP_NOTI_VIA_FUNC, 0, TvmInsert_Poncard_Insert_Callback_2) ;
	}
	else
	{
		CDP_Create( RPU_TID_CDP_IGMP_TVM,  CDP_NOTI_VIA_FUNC, 0, TvmInsert_Poncard_Insert_Callback) ;
	}
	
	( VOID ) CDP_SYNC_Register( MODULE_RPU_IGMPSNOOP, IgmpTvm_CMD2LIC_RPC_Callback );

	pM_rpcIfs_tvm = &M_tvm_rpcIfs;
	pS_rpcIfs_tvm = &S_tvm_rpcIfs;
/*
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
		IgmpTvmRpcIfs = &S_tvm_rpcIfs ;
	else
		IgmpTvmRpcIfs = &M_tvm_rpcIfs ;
*/
	if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		IgmpTvmTimerStart();
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
		Igmp_tvm_cli_init();
	else
		Igmp_tvm_cli_init_pon();
	
	return(VOS_OK);
}


QDEFUN( igmpsnoop_tvm_enable_func,
        igmpsnooping_tvm_able_cmd,
        "igmp-snooping-tvm [enable|disable]",
        "IGMP snooping through vlan multicast config\n"
        "IGMP Snooping tvm function\n"
        "Enable IGMP Snooping tvm\n"
        "Disable IGMP Snooping tvm\n",
        &g_ulIgmp_Snoop_MSQ_id  )
{
	if ( 0 == VOS_StrCmp( argv[ 0 ], "enable" ) )
	{
		if( VOS_YES == g_lIgmp_Snoop_Enable )
		{
      			if ( VOS_YES == g_ulIgmp_TVM_Enabled )
        		{
            			vty_out( vty, "igmp-snooping-tvm is already enabled.\r\n" );
            			return CMD_WARNING;
        		}
			Set_Igmp_Tvm_Enable(-1,VOS_YES);
        		vty_out( vty, " Igmp snooping-tvm Enable!\r\n");
        		/*vty_out( vty, " Please check if all the pon ports are in the special vlan !\r\n");*/
		}
		else
		{
			vty_out(vty,"You should firstly enable the Igmp-snooping !\r\n");
            		return CMD_WARNING;
		}
	}
	else
    	{
       	if ( VOS_NO == g_ulIgmp_TVM_Enabled )
       	{
            		vty_out( vty, " igmp-snooping-tvm is already disabled.\r\n" );
            		return CMD_WARNING;
       	}
        	Set_Igmp_Tvm_Enable(-1,VOS_NO);
        	vty_out( vty, " Igmp snooping-tvm Disable!\r\n");
	}
	
	return CMD_SUCCESS;
	
}

STATUS  Tvm_Table_Crc()
{
	int k=0;
	Tvm_Pkt_t *Tvm_Pkt_temp=NULL, *Tvm_Pkt_temp2 = NULL;
	Through_Vlan_Group_t  *Thr_Vlan_temp=NULL;
	STATUS ret = VOS_OK;
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;

	IGMP_TVM_DEBUG( ("\r\nNow the global group Number in the table is %d .\r\n", TVM_Cont_Head_Info->TVMCOUNT) );
	
	if(0 == TVM_Cont_Head_Info->TVMCOUNT || NULL == TVM_Cont_Head_Info->Through_Vlan_Group_head)
	{
		TVM_Cont_Head_Info->Calculates = 0;
	}
	else
	{
		Tvm_Pkt_temp = (Tvm_Pkt_t *)VOS_Malloc((TVM_Cont_Head_Info->TVMCOUNT)*sizeof(Tvm_Pkt_t), MODULE_RPU_IGMPSNOOP);
		Tvm_Pkt_temp2 = Tvm_Pkt_temp;
		if(NULL == Tvm_Pkt_temp)
		{
			VOS_ASSERT(0);
	        	return VOS_ERROR;	
		}
		VOS_MemSet(Tvm_Pkt_temp, 0,(TVM_Cont_Head_Info->TVMCOUNT)*sizeof(Tvm_Pkt_t) );
		
		VOS_SemTake(OnuTVMCommSemId , WAIT_FOREVER);
		while( Thr_Vlan_temp )
		{
			if( k >= TVM_Cont_Head_Info->TVMCOUNT )
			{
				VOS_ASSERT(0);
				ret = VOS_ERROR;	
				break;
			}
			Tvm_Pkt_temp->IVid = VOS_HTONS(Thr_Vlan_temp->IVid); 
			Tvm_Pkt_temp->GROUPS = VOS_HTONL(Thr_Vlan_temp->GROUPS);
			Tvm_Pkt_temp->GROUPE = VOS_HTONL(Thr_Vlan_temp->GROUPE);
			Tvm_Pkt_temp->llid = VOS_HTONS(Thr_Vlan_temp->llid);
			Tvm_Pkt_temp->PonId = VOS_HTONS(Thr_Vlan_temp->PonId);
			Tvm_Pkt_temp->ulIfIndex = VOS_HTONL(Thr_Vlan_temp->ulIfIndex);
			
			Thr_Vlan_temp = Thr_Vlan_temp->next;
			Tvm_Pkt_temp++;
			k++;
		}
		if( ret == VOS_OK )
		{
			TVM_Cont_Head_Info->Calculates = FTPC_CalCrc32((UCHAR *)Tvm_Pkt_temp2,TVM_Cont_Head_Info->TVMCOUNT*sizeof(Tvm_Pkt_t));
		}
		VOS_SemGive(OnuTVMCommSemId);

		VOS_Free(Tvm_Pkt_temp2);
	}

	return ret;
}

int Igmp_Tvm_Add(ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void *vty)
{
	Through_Vlan_Group_t  *Thr_Vlan_temp;
	LONG lRet = VOS_OK;

	if(VOS_OK != AddIgmpSnoopTvmItem(mulIps,mulIpe,mcastVid,vty))
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
	 		vty_out( vty, "Failed to add the group.\r\n");
		return VOS_ERROR;
	}
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		lRet = Tvm_Table_Crc();
	
		if (VOS_OK != lRet)
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
				vty_out( vty, "Executing error when it calculates the CRC.\r\n");
			return VOS_ERROR;
		}

		if(VOS_YES == g_ulIgmp_TVM_Enabled)
		{
			Thr_Vlan_temp = (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
			if( NULL == Thr_Vlan_temp )
		    	{
		        	/*vty_out(vty, "Failed to apply for memory when it sends the groups you want to add to the ONUs.\r\n" );*/
				VOS_ASSERT(0);
		        	return VOS_ERROR;
		    	}
				
			Thr_Vlan_temp->GROUPS = mulIps;
			Thr_Vlan_temp->GROUPE = mulIpe;
			Thr_Vlan_temp->IVid = mcastVid;
			Thr_Vlan_temp->PonId = 0;
			Thr_Vlan_temp->llid = 0;
			Thr_Vlan_temp->ulIfIndex = 0;
			Thr_Vlan_temp->next = NULL;

			 if(VOS_OK != Send_To_IgmpQue_Tvm((void *)Thr_Vlan_temp,AWMC_IPMC_PRIVATE_SNOOP_TVM_ADD))
			 {
			 	VOS_Free(Thr_Vlan_temp);
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty, "Failed to send the groups you want to add to the ONUs.\r\n" );
				return VOS_ERROR;
			 }
		}
	}
	
	return VOS_OK;
	
}

int Add_Igmp_Tvm_Item(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, USHORT mcastVid, void *vty)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_add_t, (PonPortIdx, mulIps,  mulIpe,  mcastVid, vty) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_add_t, (olt_id, mulIps, mulIpe, mcastVid, vty))) )
			{
				break;
			}
		}
	}

	/*if ( VOS_OK == iRlt )*/
    	{
		iRlt = Igmp_Tvm_Add(mulIps, mulIpe, mcastVid, (struct vty*)vty);
	}
    
	return iRlt;

}

QDEFUN(
    igmpsnooping_tvm_add_func,
    igmpsnooping_tvm_add_cmd,
    "igmp-snooping-tvm add <A.B.C.D> <A.B.C.D> <1-4094>",
    "IGMP snooping through vlan multicast config\n"
    "Config IGMP snooping through vlan multicast to add items\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n"
    "Please input multicast vlan id\n",
    &g_ulIgmp_Snoop_MSQ_id )
{
 
 	ULONG mulIps = 0;
	ULONG mulIpe = 0;
	unsigned int  mcastVid = 0;	
	
	mulIps= get_long_from_ipdotstring( argv[ 0 ] );
	mulIps = VOS_NTOHL(mulIps);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIps ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 0] );
		return CMD_WARNING;
    	}

	mulIpe = get_long_from_ipdotstring( argv[ 1 ] );
	mulIpe = VOS_NTOHL(mulIpe);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIpe ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[1] );
		return CMD_WARNING;
    	}
	if(mulIps > mulIpe)
	{
		vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}	
	mcastVid = ( USHORT ) VOS_AtoL( argv[ 2 ] );

	Add_Igmp_Tvm_Item(-1,mulIps,mulIpe, mcastVid, (void *)vty);

	/*vty_out( vty, " Please check if all the pon ports are in the special vlan!\r\n");*/
	return CMD_SUCCESS;

}

int Igmp_Tvm_DelItem(ULONG mulIps, ULONG mulIpe, void *vty)
{
	Through_Vlan_Group_t  *Thr_Vlan_temp;
	LONG lRet = VOS_OK;
	lRet = DelIgmpSnoopTvmItem( mulIps, mulIpe,vty);
	if (lRet != VOS_OK)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out( vty, "Failed to delete the groups.\r\n");
		return VOS_ERROR;
	}
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{	
		lRet=Tvm_Table_Crc();

		if (VOS_OK != lRet)
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
				vty_out( vty, "Executing error when it calculates the CRC.\r\n");
			return VOS_ERROR;
		}

		if(VOS_YES == g_ulIgmp_TVM_Enabled)
		{
			Thr_Vlan_temp= (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
			if( NULL == Thr_Vlan_temp )
		    	{
		        	/*vty_out(vty, "Failed to apply for memory when it sends the groups you want to delete to the ONUs.\r\n" );*/
				VOS_ASSERT(0);
		        	return VOS_ERROR;
		    	}
				
			Thr_Vlan_temp->GROUPS = mulIps;
			Thr_Vlan_temp->GROUPE = mulIpe;
			Thr_Vlan_temp->IVid = 0;
			Thr_Vlan_temp->PonId = 0;
			Thr_Vlan_temp->llid = 0;
			Thr_Vlan_temp->ulIfIndex = 0;
			Thr_Vlan_temp->next = NULL;
			
			 if(VOS_OK != Send_To_IgmpQue_Tvm((void *)Thr_Vlan_temp,AWMC_IPMC_PRIVATE_SNOOP_TVM_DEL))
			 {
			 	VOS_Free(Thr_Vlan_temp);
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty, "Failed to send the groups you want to delete to the ONUs.\r\n" );
				return VOS_ERROR;
			 }
		}
	}
	return VOS_OK;
}

int Del_Igmp_Tvm_Item(unsigned int PonPortIdx,ULONG mulIps, ULONG mulIpe, void *vty)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_delitem_t, (PonPortIdx, mulIps,  mulIpe, vty) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_delitem_t, (olt_id, mulIps, mulIpe, vty))) )
			{
				break;
			}
		}
	}

	/*if ( VOS_OK == iRlt )*/
    	{
		iRlt = Igmp_Tvm_DelItem(mulIps, mulIpe, vty);
	}
    
	return iRlt;

}

QDEFUN(
    igmpsnooping_tvm_del_func,
    igmpsnooping_tvm_del_cmd,
    "igmp-snooping-tvm del <A.B.C.D> <A.B.C.D>",
    "IGMP snooping through vlan multicast config\n"
    "Config IGMP snooping through vlan multicast to delete items\n"
    "Please input group-ip start(224.0.1.0-239.255.255.255)\n"
    "Please input group-ip end(224.0.1.0-239.255.255.255)\n",
    &g_ulIgmp_Snoop_MSQ_id )
{
 	ULONG mulIps = 0;
	ULONG mulIpe = 0 ;
	
	/*argv[0]*/
	mulIps= get_long_from_ipdotstring( argv[ 0 ] );
	mulIps = VOS_NTOHL(mulIps);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIps ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[ 0] );
		return CMD_WARNING;
    	}

	/*argv[1]*/
	mulIpe = get_long_from_ipdotstring( argv[ 1 ] );
	mulIpe = VOS_NTOHL(mulIpe);
    	if ( VOS_OK != Igmp_Snoop_Addr_Check( mulIpe ) )
 	{
 		vty_out( vty, "Group address %s invalid.\r\n", argv[1] );
		return CMD_WARNING;
    	}
	if(mulIps > mulIpe)
	{
		vty_out(vty, "\r\nThe second multicast address should not be smaller then the first one !!\r\n");
		return CMD_WARNING;
	}
	Del_Igmp_Tvm_Item(-1, mulIps, mulIpe, (void *)vty);

	return CMD_SUCCESS;

}

int Igmp_Tvm_DelAll(struct vty *vty)
{
 	LONG lRet = VOS_OK;
	Through_Vlan_Group_t  *Thr_Vlan_temp;

	lRet = DelIgmpSnoopTvmAll();
	if (VOS_OK != lRet)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out( vty, "Failed to delete all the groups.\r\n");
		return VOS_ERROR;
	}
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{	
		lRet=Tvm_Table_Crc();

		if (VOS_OK != lRet)
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
				vty_out( vty, "Executing error when it calculates the CRC.\r\n");
			return VOS_ERROR;
		}

		if(VOS_YES == g_ulIgmp_TVM_Enabled)
		{		
			Thr_Vlan_temp = (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
			if( NULL == Thr_Vlan_temp )
			{
			      	/*vty_out(vty, "Failed to apply for memory when it sends the message about 'delete all the groups'  to the ONUs.\r\n" );*/
				VOS_ASSERT(0);
			      	return VOS_ERROR;
			}

			Thr_Vlan_temp->GROUPS = 0;
			Thr_Vlan_temp->GROUPE = 0;
			Thr_Vlan_temp->IVid = 0;
			Thr_Vlan_temp->PonId = 0;
			Thr_Vlan_temp->llid = 0;
			Thr_Vlan_temp->ulIfIndex = 0;
			Thr_Vlan_temp->next = NULL;
			
			 if(VOS_OK != Send_To_IgmpQue_Tvm((void *)Thr_Vlan_temp,AWMC_IPMC_PRIVATE_SNOOP_TVM_DEL))
			 {
			 	VOS_Free(Thr_Vlan_temp);
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty,"Failed to send the message about 'delete all the groups' to the ONUs.\r\n");
				return VOS_ERROR;
			 }
		}
	}
	return VOS_OK;
}

int Del_Igmp_Tvm_All(unsigned int PonPortIdx,void *vty)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_delall_t, (PonPortIdx, vty) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_delall_t, (olt_id, vty))) )
			{
				break;
			}
		}
	}

	/*if ( VOS_OK == iRlt )*/
    	{
		iRlt = Igmp_Tvm_DelAll( (struct vty*)vty);
	}
    
	return iRlt;

}

QDEFUN(
    igmpsnooping_tvm_delall_func,
    igmpsnooping_tvm_delall_cmd,
    "igmp-snooping-tvm del all",
    "IGMP snooping through vlan multicast config\n"
    "Config IGMP snooping through vlan multicast to delete items\n"
    "Delete the table\n",
    &g_ulIgmp_Snoop_MSQ_id )
{

	Del_Igmp_Tvm_All(-1, (void *) vty);
	return CMD_SUCCESS;

}

int Igmp_Tvm_DelVid(USHORT mcastVid, struct vty *vty)
{
 	LONG lRet = VOS_OK;
	Through_Vlan_Group_t  *Thr_Vlan_temp;
	lRet = DelIgmpSnoopTvmVid(mcastVid,vty);
	if (VOS_OK != lRet)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out( vty, "Failed to delete the groups contained in the VlanId you give .\r\n");
		return VOS_ERROR;
	}
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		lRet = Tvm_Table_Crc();

		if (VOS_OK != lRet)
		{
			if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
				vty_out( vty, "Executing error when it calculates the CRC.\r\n");
			return VOS_ERROR;
		}
		if(VOS_YES == g_ulIgmp_TVM_Enabled)
		{		
			Thr_Vlan_temp = (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
			if( NULL == Thr_Vlan_temp )
			{
			      /*vty_out(vty, "Failed to apply for memory when it sends the message about 'delete all the groups'  to the ONUs.\r\n" );*/
				VOS_ASSERT(0);
			      	return VOS_ERROR;
			}

			Thr_Vlan_temp->GROUPS = 0;
			Thr_Vlan_temp->GROUPE = 0;
			Thr_Vlan_temp->IVid = mcastVid;
			Thr_Vlan_temp->PonId = 0;
			Thr_Vlan_temp->llid = 0;
			Thr_Vlan_temp->ulIfIndex = 0;
			Thr_Vlan_temp->next	= NULL;
			
			 if(VOS_OK != Send_To_IgmpQue_Tvm((void *)Thr_Vlan_temp,AWMC_IPMC_PRIVATE_SNOOP_TVM_DEL))
			 {
			 	VOS_Free(Thr_Vlan_temp);
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty,"Failed to send the VlanId in which you want to delete the groups to the ONUs .\r\n");
				return VOS_ERROR;
			 }
		}
	}
	return VOS_OK;
}

int Del_Igmp_Tvm_Vid(unsigned int PonPortIdx,USHORT mcastVid,void *vty)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_delvid_t, (PonPortIdx, mcastVid, vty) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_delvid_t, (olt_id, mcastVid, vty))) )
			{
				break;
			}
		}
	}

	/*if ( VOS_OK == iRlt )*/
    	{
		iRlt = Igmp_Tvm_DelVid(mcastVid, (struct vty*)vty);
	}
    
	return iRlt;

}

QDEFUN(
    igmpsnooping_tvm_delvid_func,
    igmpsnooping_tvm_delvid_cmd,
    "igmp-snooping-tvm del <1-4094>",
    "IGMP snooping through vlan multicast config\n"
    "Config IGMP snooping through vlan multicast to delete items\n"
    "Delete all the multicast groups in the vlan\n",
    &g_ulIgmp_Snoop_MSQ_id )
{
	unsigned int  mcastVid = 0;	
	mcastVid = ( USHORT ) VOS_AtoL( argv[ 0 ] );
	
	Del_Igmp_Tvm_Vid(-1,mcastVid,(struct vty*)vty);
	
	return CMD_SUCCESS;

}

QDEFUN (
    igmpsnooping_tvm_item_show_func,
    igmpsnooping_tvm_show_cmd,
    "show igmp-snooping-tvm {<1-4094>}*1",
    DescStringCommonShow
    "IGMP Snooping through vlan multicast table\n"
    "Please input multicast vlan id\n",
    &g_ulIgmp_Snoop_MSQ_id )
{
	Through_Vlan_Group_t  * Thr_Vlan_temp;
	char buftemp[32];
	int Counter_tvm = 1;
	unsigned int  mcastVid = 0;	
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	if(VOS_YES == g_ulIgmp_TVM_Enabled)
	{
		vty_out(vty,"\r\n Igmp-snooping tvm is enabled!\r\n");
	}
	else
	{
		vty_out(vty,"\r\n Igmp-snooping tvm is disabled!\r\n");
	}
	vty_out( vty, "\r\nNo.    GroupStart      GroupEnd        IVid\r\n");
	vty_out( vty, "---------------------------------------------\r\n");
	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
	if(1 == argc)
	{
		mcastVid = ( USHORT ) VOS_AtoL( argv[ 0 ] );
		while(NULL != Thr_Vlan_temp)
		{
			if(mcastVid ==Thr_Vlan_temp->IVid)
			{
				VOS_MemZero(buftemp, sizeof(buftemp));
				get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
				vty_out( vty, " %-4d  %-14s ", Counter_tvm, buftemp);
				VOS_MemZero(buftemp, sizeof(buftemp));
				get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
				vty_out( vty, " %-14s  %-8d\r\n", buftemp, Thr_Vlan_temp->IVid);
				Counter_tvm++;
			}
			Thr_Vlan_temp=Thr_Vlan_temp->next;
		}
	}
	else
	{
		while(NULL != Thr_Vlan_temp)
		{	
			VOS_MemZero(buftemp, sizeof(buftemp));
			get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
			vty_out( vty, " %-4d  %-14s ",Counter_tvm,buftemp);
			VOS_MemZero(buftemp, sizeof(buftemp));
			get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
			vty_out( vty, " %-14s  %-8d\r\n",buftemp, Thr_Vlan_temp->IVid);
			Counter_tvm++;
			Thr_Vlan_temp = Thr_Vlan_temp->next;
		}
	}
	
	VOS_SemGive(OnuTVMCommSemId);
	
	return CMD_SUCCESS;
	
}

QDEFUN( IgmpSnoop_Tvm_Debug_Func,
        Igmpsnooping_tvm_debug_cmd,
        "debug igmp-snooping tvm",
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Tvm\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
	glIgmpTvmDebug = VOS_YES;

	return CMD_SUCCESS;
	
}


QDEFUN( undo_IgmpSnoop_Tvm_Debug_Func,
        undo_Igmpsnooping_tvm_debug_cmd,
        "undo debug igmp-snooping tvm",
        NO_STR
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Tvm\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
	glIgmpTvmDebug = VOS_NO;

	return CMD_SUCCESS;
	
}

int Set_Igmp_Tvm_Sync(unsigned int PonPortIdx,long syncvalue)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_sync_t, (PonPortIdx, syncvalue) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_sync_t, (olt_id, syncvalue))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		tvm_timer_period = syncvalue ;
	}
    
	return iRlt;

}

QDEFUN( Igmp_Tvm_CheckInterval_Func,
        Igmp_Tvm_CheckInterval_Cmd,
        "igmp-snooping-tvm sync-interval [<10-120>|default]",
        "IGMP Snooping Tvm command\n"
        "Set synchronization Interval time\n"
        "Synchronization Interval time value,the range is <10-120> seconds\n"
        "Set to default value (30 seconds)\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
	if ( 0 == VOS_StrCmp( argv[ 0 ], "default" ) )
	{
		Set_Igmp_Tvm_Sync(-1, CRC_CHECK_INTERVAL_DEFAULT); 
	}
	else
	{
		Set_Igmp_Tvm_Sync(-1, VOS_AtoL( argv[ 0 ] ));
	}
	
	return VOS_OK;

}

QDEFUN( show_Igmp_Tvm_CheckInterval_Func,
        show_Igmp_Tvm_Interval_Cmd,
        "show igmp-snooping-tvm sync-interval",
        DescStringCommonShow
        "IGMP Snooping Tvm command\n"
        "Synchronization Interval time value\n",
        &g_ulIgmp_Snoop_MSQ_id
      )
{
	vty_out(vty,"\r\n  Synchronization Interval time is : %d seconds.\r\n",tvm_timer_period);
	return VOS_OK;
}

DEFUN( Pon_show_Igmp_Tvm_CheckInterval_Func,
        Pon_show_Igmp_Tvm_Interval_Cmd,
        "show igmp-snooping-tvm sync-interval",
        DescStringCommonShow
        "IGMP Snooping Tvm command\n"
        "Synchronization Interval time value\n"
      )
{
	vty_out(vty,"\r\n  Synchronization Interval time is : %d seconds.\r\n",tvm_timer_period);
	return VOS_OK;
}

DEFUN( Pon_IgmpSnoop_Tvm_Debug_Func,
        Pon_Igmpsnooping_tvm_debug_cmd,
        "debug igmp-snooping tvm",
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Tvm\n"
      )
{
	glIgmpTvmDebug = VOS_YES;

	return CMD_SUCCESS;
	
}

DEFUN( Pon_undo_IgmpSnoop_Tvm_Debug_Func,
        Pon_undo_Igmpsnooping_tvm_debug_cmd,
        "undo debug igmp-snooping tvm",
        NO_STR
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Tvm\n"
      )
{
	glIgmpTvmDebug = VOS_NO;

	return CMD_SUCCESS;
	
}

extern ULONG g_ulIgmpAuthDebug;

DEFUN( Pon_IgmpSnoop_Auth_Debug_Func,
        Pon_Igmpsnooping_Auth_debug_cmd,
        "debug igmp-snooping auth",
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Auth\n"
      )
{
	g_ulIgmpAuthDebug = VOS_YES;

	return CMD_SUCCESS;
	
}

DEFUN( Pon_undo_IgmpSnoop_Auth_Debug_Func,
        Pon_undo_Igmpsnooping_Auth_debug_cmd,
        "undo debug igmp-snooping auth",
        NO_STR
        "debug command\n"
        "IGMP Snooping debug command\n"
        "IGMP Snooping debug Auth\n"
      )
{
	g_ulIgmpAuthDebug = VOS_NO;

	return CMD_SUCCESS;
	
}

DEFUN (
    Pon_igmpsnooping_tvm_item_show_func,
    Pon_igmpsnooping_tvm_show_cmd,
    "show igmp-snooping-tvm {<1-4094>}*1",
    DescStringCommonShow
    "IGMP Snooping through vlan multicast table\n"
    "Please input multicast vlan id\n"
    )
{
	Through_Vlan_Group_t  * Thr_Vlan_temp;
	char buftemp[32];
	int Counter_tvm = 1;
	unsigned int  mcastVid = 0;	
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	if(VOS_YES == g_ulIgmp_TVM_Enabled)
	{
		vty_out(vty,"\r\n Igmp-snooping tvm is enabled!\r\n");
	}
	else
	{
		vty_out(vty,"\r\n Igmp-snooping tvm is disabled!\r\n");
	}
	vty_out( vty, "\r\nNo.    GroupStart      GroupEnd        IVid\r\n");
	vty_out( vty, "---------------------------------------------\r\n");
	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
	if(1 == argc)
	{
		mcastVid = ( USHORT ) VOS_AtoL( argv[ 0 ] );
		while(NULL != Thr_Vlan_temp)
		{
			if(mcastVid ==Thr_Vlan_temp->IVid)
			{
				VOS_MemZero(buftemp, sizeof(buftemp));
				get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
				vty_out( vty, " %-4d  %-14s ", Counter_tvm, buftemp);
				VOS_MemZero(buftemp, sizeof(buftemp));
				get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
				vty_out( vty, " %-14s  %-8d\r\n", buftemp, Thr_Vlan_temp->IVid);
				Counter_tvm++;
			}
			Thr_Vlan_temp=Thr_Vlan_temp->next;
		}
	}
	else
	{
		while(NULL != Thr_Vlan_temp)
		{	
			VOS_MemZero(buftemp, sizeof(buftemp));
			get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
			vty_out( vty, " %-4d  %-14s ",Counter_tvm,buftemp);
			VOS_MemZero(buftemp, sizeof(buftemp));
			get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
			vty_out( vty, " %-14s  %-8d\r\n",buftemp, Thr_Vlan_temp->IVid);
			Counter_tvm++;
			Thr_Vlan_temp = Thr_Vlan_temp->next;
		}
	}
	
	VOS_SemGive(OnuTVMCommSemId);
	
	return CMD_SUCCESS;
	
}

int Igmp_tvm_cli_init( VOID )
{
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_able_cmd);
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_add_cmd);
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_del_cmd);
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_delvid_cmd);
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_delall_cmd);
	 install_element( CONFIG_NODE, &igmpsnooping_tvm_show_cmd);
	 install_element( DEBUG_HIDDEN_NODE, &Igmpsnooping_tvm_debug_cmd);
	 install_element( DEBUG_HIDDEN_NODE, &undo_Igmpsnooping_tvm_debug_cmd);
	 install_element(CONFIG_NODE, &Igmp_Tvm_CheckInterval_Cmd);
	 install_element(CONFIG_NODE, &show_Igmp_Tvm_Interval_Cmd);
	 return VOS_OK;	
}

int Igmp_tvm_cli_init_pon( VOID )
{
	 install_element( CONFIG_NODE, &Pon_igmpsnooping_tvm_show_cmd);
	 install_element( DEBUG_HIDDEN_NODE, &Pon_Igmpsnooping_tvm_debug_cmd);
	 install_element( DEBUG_HIDDEN_NODE, &Pon_undo_Igmpsnooping_tvm_debug_cmd);
	 install_element(CONFIG_NODE, &Pon_show_Igmp_Tvm_Interval_Cmd);
	 install_element( DEBUG_HIDDEN_NODE, &Pon_Igmpsnooping_Auth_debug_cmd);
	 install_element( DEBUG_HIDDEN_NODE, &Pon_undo_Igmpsnooping_Auth_debug_cmd);
	 return VOS_OK;	
}

STATUS DelIgmpSnoopTvmAll()
{
	Through_Vlan_Group_t  *Thr_Vlan_temp=NULL,*Thr_Vlan_temp2=NULL;
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	
	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
	
	while(NULL != Thr_Vlan_temp)
	{
		Thr_Vlan_temp2 = Thr_Vlan_temp;
		Thr_Vlan_temp = Thr_Vlan_temp->next;
		VOS_Free(Thr_Vlan_temp2);
		TVM_Cont_Head_Info->TVMCOUNT--;
	}
	
	TVM_Cont_Head_Info->Through_Vlan_Group_head=NULL;
	
	VOS_SemGive(OnuTVMCommSemId);
	
	return VOS_OK;
	
}


STATUS DelIgmpSnoopTvmVid(unsigned int  mcastVid , struct vty * vty)
{
	int count = 0;
	Through_Vlan_Group_t  *Thr_Vlan_temp = NULL,*Thr_Vlan_Pre = NULL,*Thr_Vlan_temp2 = NULL;
	
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	
	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
	
	while(Thr_Vlan_temp)
	{
		if(Thr_Vlan_temp->IVid == mcastVid)
		{
			if(NULL != Thr_Vlan_Pre)
			{
				Thr_Vlan_Pre->next = Thr_Vlan_temp->next;
			}
			else
			{
				TVM_Cont_Head_Info->Through_Vlan_Group_head = Thr_Vlan_temp->next;
			}
			Thr_Vlan_temp2 = Thr_Vlan_temp;
			Thr_Vlan_temp = Thr_Vlan_temp->next;
			VOS_Free(Thr_Vlan_temp2);
			TVM_Cont_Head_Info->TVMCOUNT--;
			count++;
		}
		else
		{
			Thr_Vlan_Pre=Thr_Vlan_temp;
			Thr_Vlan_temp = Thr_Vlan_temp->next;
		}
	}
		
	VOS_SemGive(OnuTVMCommSemId);

	if( 0 == count )
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out(vty, "The VlanId you input doesn't contain any groups.\r\n");
		return VOS_ERROR;
	}
	
	return VOS_OK;
	
}

STATUS  Send_To_IgmpQue_Tvm(void * pointer ,short int tvmtype) 
{
 	SYS_MSG_S *pstMsg = NULL;
 	ULONG aulMsg[ 4 ] = { 0 };
	int rc=VOS_ERROR;
	
    	pstMsg = ( SYS_MSG_S * ) VOS_Malloc( sizeof( SYS_MSG_S ), MODULE_RPU_IGMPSNOOP );
    	if( NULL == pstMsg )
    	{
    		VOS_ASSERT(0);
        	return VOS_ERROR;
    	}
    	VOS_MemZero( pstMsg, sizeof( SYS_MSG_S ) );
    	pstMsg->ptrMsgBody     =  pointer; 
    	pstMsg->usMsgCode      = tvmtype;
    	pstMsg->ucMsgType      = MSG_NOTIFY;
    	pstMsg->ulDstModuleID  = MODULE_RPU_IGMPSNOOP;
    	pstMsg->ulSrcModuleID  = MODULE_RPU_IGMPSNOOP;
    	pstMsg->ulSrcSlotID    = DEV_GetPhySlot();
    	pstMsg->ulDstSlotID    = DEV_GetPhySlot();
    	pstMsg->ucMsgBodyStyle = MSG_BODY_MULTI;
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
	{
	      aulMsg[ 0 ] = pstMsg->ulSrcModuleID;
	      aulMsg[ 1 ] = 0; 
	      aulMsg[ 2 ] = 0;
	      aulMsg[ 3 ] = (ULONG) pstMsg;

		if( VOS_OK !=  VOS_QueSend( g_ulIgmp_Snoop_MSQ_id, aulMsg, NO_WAIT, MSG_PRI_NORMAL ) )
		{
			VOS_Free( pstMsg );
			return VOS_ERROR;
		}
		else
			return VOS_OK;
	}
	else
	{
		switch(SYS_MSG_MSG_CODE( pstMsg ))
		{
			case AWMC_IPMC_PRIVATE_SNOOP_TVM_ABLE:
				rc = Igmp_Tvm_Recv_Enable(pstMsg);
				if(rc == VOS_OK)
					IGMP_SNOOP_FREE(pstMsg->ptrMsgBody);
				IGMP_SNOOP_FREE( pstMsg );
				break;
			case AWMC_IPMC_PRIVATE_SNOOP_TVM_ADD:
				rc = Igmp_Tvm_Recv_Add(pstMsg);
				if(rc == VOS_OK)
					IGMP_SNOOP_FREE(pstMsg->ptrMsgBody);
				IGMP_SNOOP_FREE( pstMsg );
				break;
			case AWMC_IPMC_PRIVATE_SNOOP_TVM_DEL:
				rc =Igmp_Tvm_Recv_Del(pstMsg);
				if(rc == VOS_OK)
					IGMP_SNOOP_FREE(pstMsg->ptrMsgBody);
				IGMP_SNOOP_FREE( pstMsg );
				break;

			default:
                     	ASSERT( 0 );
                     	rc = VOS_ERROR;
				break;
		}
	}
	return rc;
}


STATUS DelIgmpSnoopTvmItem(ULONG mulips,ULONG mulipe,struct vty * vty)
{
	STATUS rc = VOS_ERROR;
	Through_Vlan_Group_t  *Thr_Vlan_temp = NULL,*Thr_Vlan_new = NULL, *Thr_Vlan_pre = NULL ;
	
	Thr_Vlan_temp=TVM_Cont_Head_Info->Through_Vlan_Group_head;	
	if(mulips > mulipe)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out(vty, "\r\nThe second multicast address should not be smaller then the first one !!\r\n");
		return VOS_ERROR;
	}
	
	
	if( NULL == Thr_Vlan_temp )
	{
		return VOS_ERROR;
	}

	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
		
	while(Thr_Vlan_temp)
	{
		if( (Thr_Vlan_temp->GROUPS <= mulips) && (mulipe <=Thr_Vlan_temp->GROUPE) )
		{

			if(Thr_Vlan_temp->GROUPS == mulips)
			{
				if(Thr_Vlan_temp->GROUPE == mulipe)/*两边都重合*/
				{
					if(Thr_Vlan_pre != NULL)/*该结点不是头结点*/
					{
						Thr_Vlan_pre->next = Thr_Vlan_temp->next;
					}
					else/*该结点是头结点*/
					{
						TVM_Cont_Head_Info->Through_Vlan_Group_head = Thr_Vlan_temp->next;
					}
					VOS_Free(Thr_Vlan_temp);
					TVM_Cont_Head_Info->TVMCOUNT--;
					rc = VOS_OK;
					break;
				}
				else/*只有左边重合*/
				{
					Thr_Vlan_temp->GROUPS = mulipe+1;
					rc = VOS_OK;
					break;
				}
			}
			else if(Thr_Vlan_temp->GROUPE== mulipe)/*只有右边重合*/
			{
				Thr_Vlan_temp->GROUPE = mulips-1;
				rc = VOS_OK;
				break;
			}
			else/*两边都不重合*/
			{
				Thr_Vlan_new = (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
				if(Thr_Vlan_new==NULL)
				{
				       VOS_ASSERT(0);
					break;
				}
				Thr_Vlan_new->next = Thr_Vlan_temp->next;
				Thr_Vlan_temp->next = Thr_Vlan_new;
				Thr_Vlan_new->GROUPS =  mulipe+1;
				Thr_Vlan_new->GROUPE =  Thr_Vlan_temp->GROUPE;
				Thr_Vlan_new->IVid = Thr_Vlan_temp->IVid;
				Thr_Vlan_new->PonId = 0;
				Thr_Vlan_new->llid = 0;
				Thr_Vlan_new->ulIfIndex = 0;
				Thr_Vlan_temp->GROUPE=mulips-1;
				TVM_Cont_Head_Info->TVMCOUNT++;
				rc = VOS_OK;
				break;
			}
		}
		else
		{
			if(mulips < Thr_Vlan_temp->GROUPS || NULL == Thr_Vlan_temp->next)
			{
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty,"The groups don't exist or conflict with the groups, please check it !\r\n");
				break;
			}		
		}

		Thr_Vlan_pre= Thr_Vlan_temp ;
		Thr_Vlan_temp = Thr_Vlan_temp->next ;
	}

	VOS_SemGive(OnuTVMCommSemId);

	return rc;	
	
}

STATUS AddIgmpSnoopTvmItem(ULONG   mulips,ULONG   mulipe, unsigned int vid, struct vty * vty)
{
	Through_Vlan_Group_t  *Thr_Vlan_temp = NULL,*Thr_Vlan_new = NULL, *Thr_Vlan_pre = NULL , *Thr_Vlan_next = NULL,*Thr_Vlan_pre2 = NULL ;
	INT connect_flag = 0 ;
	STATUS ret = VOS_OK;
	
	Thr_Vlan_temp=TVM_Cont_Head_Info->Through_Vlan_Group_head;	
	if(mulips > mulipe)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out(vty,"The second multicast address should not be smaller then the first one !!\r\n");
		return VOS_ERROR;
	}
	if(TVM_Cont_Head_Info->TVMCOUNT >= TVM_MAX_GROUPS)
	{
		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
			vty_out(vty,"The maximal number of groups is %d , it has exceeded the limit !\r\n",TVM_MAX_GROUPS);
		return VOS_ERROR;
	}
	VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
	
	if(NULL == Thr_Vlan_temp)
	{
		connect_flag = HEAD_IS_NULL ;/*现在表中没有结点*/
	}
	else
	{
		while(NULL != Thr_Vlan_temp && 0 == connect_flag)
		{
			if((Thr_Vlan_temp->GROUPS<=mulips && mulips<=Thr_Vlan_temp->GROUPE )||
			   (Thr_Vlan_temp->GROUPS<=mulipe  && mulipe<=Thr_Vlan_temp->GROUPE )||
			   (mulips<=Thr_Vlan_temp->GROUPS  &&  Thr_Vlan_temp->GROUPE<=mulipe) )
			{
				if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
					vty_out(vty, "Conflict! Please Input again .\r\n" );
				VOS_SemGive(OnuTVMCommSemId);
		 		return VOS_ERROR;
			}
			else
			{
				if(mulipe < Thr_Vlan_temp->GROUPS)/*应放置在两个已存在结点之间的情况*/
				{
					Thr_Vlan_next = Thr_Vlan_temp;
					if(NULL != Thr_Vlan_pre2)
					{
						Thr_Vlan_pre = Thr_Vlan_pre2;
						if(mulips-1 == Thr_Vlan_pre->GROUPE  && vid == Thr_Vlan_pre->IVid)
						{
							if(mulipe+1 == Thr_Vlan_next->GROUPS && vid == Thr_Vlan_pre->IVid)
							{
								connect_flag = CONNECT_BOTH;/*两边都合并*/
							}
							else
							{
								connect_flag = CONNECT_LEFT;/*左边合并*/
							}
						}
						else if(mulipe+1 == Thr_Vlan_next->GROUPS && vid == Thr_Vlan_next->IVid)
						{
							connect_flag = CONNECT_RIGHT;/*右边合并*/
						}
						else
						{
							connect_flag = CONNECT_NOT;/*两边均不需要合并*/
						}
					}
					else/*应放置在头节点之前*/
					{
						if(mulipe+1 == Thr_Vlan_temp->GROUPS && vid == Thr_Vlan_temp->IVid)
						{
							connect_flag = CONNECT_RIGHT;
						}
						else
						{
							connect_flag = CONNECT_NOT;
						}
						Thr_Vlan_pre = NULL;
					}
				}
			}
			Thr_Vlan_pre2 = Thr_Vlan_temp ;
			Thr_Vlan_temp = Thr_Vlan_temp->next ;
		}
			
		if(0 == connect_flag && NULL == Thr_Vlan_temp )/*应放置在所有结点之后*/
		{
			if(mulips-1 == Thr_Vlan_pre2->GROUPE && vid == Thr_Vlan_pre2->IVid)
			{
				connect_flag = CONNECT_LEFT;
			}
			else
			{
				connect_flag = CONNECT_NOT;
			}
			Thr_Vlan_pre = Thr_Vlan_pre2;
			Thr_Vlan_next = NULL ;
		}
		
	}

	switch(connect_flag)
	{
		case CONNECT_BOTH:
			Thr_Vlan_pre->GROUPE = Thr_Vlan_next->GROUPE;
			Thr_Vlan_pre->next = Thr_Vlan_next->next;
			VOS_Free(Thr_Vlan_next);
			TVM_Cont_Head_Info->TVMCOUNT--;
			break;
			
		case CONNECT_LEFT:
			Thr_Vlan_pre->GROUPE = mulipe;
			break;
			
		case CONNECT_RIGHT:
			Thr_Vlan_next->GROUPS = mulips;
			break;

		case HEAD_IS_NULL:
		case CONNECT_NOT:
			Thr_Vlan_new = (Through_Vlan_Group_t *)VOS_Malloc(sizeof(Through_Vlan_Group_t), MODULE_RPU_IGMPSNOOP);
			if(NULL == Thr_Vlan_new)
			{
				VOS_ASSERT(0);
			      	ret = VOS_ERROR;	
				break;
			}
			if(NULL != Thr_Vlan_pre)
			{
				Thr_Vlan_pre->next = Thr_Vlan_new;
			}
			else
			{
				TVM_Cont_Head_Info->Through_Vlan_Group_head = Thr_Vlan_new;
			}
			Thr_Vlan_new->next = Thr_Vlan_next;
			Thr_Vlan_new->GROUPS =  mulips;
			Thr_Vlan_new->GROUPE =  mulipe;
			Thr_Vlan_new->IVid = vid;
			Thr_Vlan_new->PonId = 0;
			Thr_Vlan_new->llid = 0;
			Thr_Vlan_new->ulIfIndex = 0;	
			TVM_Cont_Head_Info->TVMCOUNT++;
			break;
		default:
			if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
				vty_out(vty, "Executing error when it adds the groups !!\r\n" );
			ret = VOS_ERROR;
			break;

	}

	VOS_SemGive(OnuTVMCommSemId);
	
	return ret;
	
}


STATUS Igmp_Tvm_Recv_Enable(SYS_MSG_S *pstMsg)
{
	Tvm_Pon_Onu_t *Tvm_Pon_Temp = NULL;
	INT length, iRes = VOS_ERROR;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	
	Tvm_Pon_Temp = (Tvm_Pon_Onu_t *)pstMsg->ptrMsgBody;
	if(Tvm_Pon_Temp == NULL)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	if(TVM_ENABLE == Tvm_Pon_Temp->Enable)
	{
		if( (iRes = Igmp_Tvm_Table_Send(0, 0,TVM_BROADCAST)) != VOS_OK)
		{
	        	IGMP_TVM_DEBUG( ("Igmp_Tvm_Table_Send  failed (in Igmp_Tvm_Recv_Enable)\r\n") );
		}
	}
	else if(TVM_DISABLE == Tvm_Pon_Temp->Enable)
	{
		Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t), MODULE_RPU_IGMPSNOOP);
		if(NULL == Tvm_Pkt_Head_temp)
		{
			/*IGMP_TVM_DEBUG( "Igmp_Tvm_Recv_Enable Alloc msg failed\r\n " );*/
			VOS_ASSERT(0);
		      	iRes = VOS_ERROR;	
		}
		else
		{
			VOS_MemZero(Tvm_Pkt_Head_temp, sizeof(Tvm_Pkt_Head_t));
			Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
			Tvm_Pkt_Head_temp->type = TVM_DEL;
			Tvm_Pkt_Head_temp->CRC = 0;
			Tvm_Pkt_Head_temp->count =0;

			length = sizeof(Tvm_Pkt_Head_t);
			
			iRes = Comm_Tvm_Frame_Send(0,0, (char *)Tvm_Pkt_Head_temp, length, TVM_BROADCAST);

			VOS_Free(Tvm_Pkt_Head_temp);
		}
	}
	if( iRes != VOS_OK )
	{
		iRes = VOS_ERROR;
	}
	return iRes;
}


STATUS Igmp_Tvm_Recv_Add(SYS_MSG_S *pstMsg)
{
	ULONG mulIps = 0,mulIpe = 0;
	USHORT IVid = 0;
	INT length, iRes = VOS_OK ;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	Tvm_Pkt_t *Tvm_Pkt_temp = NULL;
	struct Through_Vlan_Group *Thr_Vlan_temp = NULL;
	char *buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t), MODULE_RPU_IGMPSNOOP);
	if(NULL == buftemp)
	{
		/*IGMP_TVM_DEBUG( "Igmp_Tvm_Recv_Add Alloc msg failed\r\n " );*/
		VOS_ASSERT(0);
	      	return VOS_ERROR;	
	}
	
	Thr_Vlan_temp = (struct Through_Vlan_Group*)pstMsg->ptrMsgBody;
	if(Thr_Vlan_temp == NULL)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	IVid = Thr_Vlan_temp->IVid;
	mulIpe =Thr_Vlan_temp->GROUPE;
	mulIps = Thr_Vlan_temp->GROUPS;
	
	if(VOS_YES == glIgmpTvmDebug)
	{
		sys_console_printf( "\r\nIn Function : Igmp_tvm_recv_Add \r\n" );
		sys_console_printf( "Thr_Vlan_temp->IVid is %d \r\n",Thr_Vlan_temp->IVid );
		VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
		get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
		sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",buftemp);
		VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
		get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
		sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",buftemp);
	}
	
	VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
	
	Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
	Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
	Tvm_Pkt_Head_temp->type = TVM_ADD;
	Tvm_Pkt_Head_temp->CRC = 0;
	Tvm_Pkt_Head_temp->count =1;
	Tvm_Pkt_temp=(Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t));
	Tvm_Pkt_temp->IVid = VOS_HTONS(IVid);
	Tvm_Pkt_temp->GROUPS = VOS_HTONL(mulIps);
	Tvm_Pkt_temp->GROUPE = VOS_HTONL(mulIpe);
	Tvm_Pkt_temp->llid = 0;
	Tvm_Pkt_temp->PonId = 0;
	Tvm_Pkt_temp->ulIfIndex = 0;
	
	length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t);
	
	iRes=Comm_Tvm_Frame_Send(0,0,buftemp,length,TVM_BROADCAST);

	VOS_Free(buftemp);

	if( iRes != VOS_OK )
		iRes = VOS_ERROR;
	return iRes;	

}


STATUS Igmp_Tvm_Recv_Del(SYS_MSG_S *pstMsg)
{
	ULONG mulIps = 0,mulIpe = 0;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	INT length, iRes = VOS_OK ;
	unsigned int  mcastVid = 0;	
	Tvm_Pkt_t *Tvm_Pkt_temp = NULL;
	struct Through_Vlan_Group *Thr_Vlan_temp = NULL;
	char *buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t), MODULE_RPU_IGMPSNOOP);
	if(NULL == buftemp)
	{
		/*IGMP_TVM_DEBUG( "Igmp_Tvm_Recv_Del Alloc msg failed\r\n " );*/
		VOS_ASSERT(0);
	      	return VOS_ERROR;	
	}
	
	Thr_Vlan_temp = (struct Through_Vlan_Group*)pstMsg->ptrMsgBody;
	if(Thr_Vlan_temp == NULL)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	mulIpe =Thr_Vlan_temp->GROUPE;
	mulIps = Thr_Vlan_temp->GROUPS;
	mcastVid = Thr_Vlan_temp->IVid;
	
	if(VOS_YES == glIgmpTvmDebug)
	{
		sys_console_printf( "\r\nIn Function : Igmp_tvm_recv_Del \r\n" );
		sys_console_printf( "Thr_Vlan_temp->IVid is %d \r\n",Thr_Vlan_temp->IVid );
		VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
		get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPS);
		sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",buftemp);
		VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
		get_ipdotstring_from_long(buftemp,Thr_Vlan_temp->GROUPE);
		sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",buftemp);
	}
	
	VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t));
	
	Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
	Tvm_Pkt_temp =(Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t));
	if( (0 != Thr_Vlan_temp->GROUPS) && (0 != Thr_Vlan_temp->GROUPE) && (0 == Thr_Vlan_temp->IVid) )/*正常删除*/
	{
		Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
		Tvm_Pkt_Head_temp->type = TVM_DEL;
		Tvm_Pkt_Head_temp->CRC = 0;
		Tvm_Pkt_Head_temp->count =1;
		Tvm_Pkt_temp->IVid = 0;
		Tvm_Pkt_temp->GROUPS = VOS_HTONL(mulIps);
		Tvm_Pkt_temp->GROUPE = VOS_HTONL(mulIpe);
		Tvm_Pkt_temp->llid = 0;
		Tvm_Pkt_temp->PonId = 0;
		Tvm_Pkt_temp->ulIfIndex = 0;
		length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t);
	}
	else if( (0 == Thr_Vlan_temp->GROUPS) && (0 == Thr_Vlan_temp->GROUPE) && (0 == Thr_Vlan_temp->IVid) )/*删除全部*/
	{
		Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
		Tvm_Pkt_Head_temp->type = TVM_DEL;
		Tvm_Pkt_Head_temp->CRC = 0;
		Tvm_Pkt_Head_temp->count =0;	
		length = sizeof(Tvm_Pkt_Head_t);
	}
	else if( (0 == Thr_Vlan_temp->GROUPS) && (0 == Thr_Vlan_temp->GROUPE) && (0 != Thr_Vlan_temp->IVid) )/*按照VLAN ID 删除*/
	{
		Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
		Tvm_Pkt_Head_temp->type = TVM_DEL_VID;
		Tvm_Pkt_Head_temp->CRC = 0;
		Tvm_Pkt_Head_temp->count =1;
		Tvm_Pkt_temp->IVid = mcastVid;
		Tvm_Pkt_temp->GROUPS = 0;
		Tvm_Pkt_temp->GROUPE = 0;
		Tvm_Pkt_temp->llid = 0;
		Tvm_Pkt_temp->PonId = 0;
		Tvm_Pkt_temp->ulIfIndex = 0;
		length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t);
	}
	else
	{
		VOS_Free(buftemp);
		return VOS_ERROR;
	}

	iRes = Comm_Tvm_Frame_Send(0,  0,  buftemp,length , TVM_BROADCAST);
	if( VOS_OK != iRes)
	{
		iRes = VOS_ERROR;
	}
	VOS_Free(buftemp);

	return iRes;
	
}


STATUS Igmp_Tvm_Recv_Mod(SYS_MSG_S *pstMsg)
{
	unsigned short usPonId = 0 , usOnuId = 0;
	Tvm_Pon_Onu_t *Tvm_Pon_Onu_temp = NULL;
	STATUS ret = VOS_ERROR;
	
	Tvm_Pon_Onu_temp = (Tvm_Pon_Onu_t*)pstMsg->ptrMsgBody;
	if(Tvm_Pon_Onu_temp == NULL)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	usPonId =Tvm_Pon_Onu_temp->PonId;
	usOnuId =Tvm_Pon_Onu_temp->OnuId;
	ret = Igmp_Tvm_Table_Send(usPonId,usOnuId,TVM_UNICAST);

	IGMP_TVM_DEBUG( ("Igmp_Tvm_Recv_Mod %s: usPonId=%d,usOnuId=%d\r\n", ((ret == VOS_OK) ? "OK" : "Err"), usPonId,usOnuId) );
	
	return ret;
	
}


STATUS Igmp_Tvm_Recv_Cal(SYS_MSG_S *pstMsg)
{
	int Calculates=0, length, iRes = VOS_OK;
	Tvm_Pon_Onu_t *Tvm_Pon_Onu_temp = NULL;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;

	char *buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t), MODULE_RPU_IGMPSNOOP);
	if(NULL == buftemp)
	{
		/*IGMP_TVM_DEBUG( "Igmp_Tvm_Recv_Cal Alloc msg failed\r\n " );*/
		VOS_ASSERT(0);
	      	return VOS_ERROR;	
	}
	

	Tvm_Pon_Onu_temp = (Tvm_Pon_Onu_t*)pstMsg->ptrMsgBody;
	if(Tvm_Pon_Onu_temp == NULL)
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}

	Calculates = Tvm_Pon_Onu_temp->Calcus;
	
	IGMP_TVM_DEBUG( ("Calculates is %d \r\n",Calculates) );
	
	Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
	if( VOS_YES == g_ulIgmp_TVM_Enabled)
	{
		Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
	}
	else
	{
		Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
	}
	Tvm_Pkt_Head_temp->type = TVM_CRC;
	Tvm_Pkt_Head_temp->CRC = Calculates;
	Tvm_Pkt_Head_temp->count = 0;

	length = sizeof(Tvm_Pkt_Head_t);
	
	iRes = Comm_Tvm_Frame_Send(0,0,buftemp,length,TVM_BROADCAST);

	VOS_Free(buftemp);


	if( VOS_OK != iRes)
	{
		iRes = VOS_ERROR;
	}
	
	return iRes;	

}

STATUS Igmp_Tvm_Table_Send( ULONG PonPortIdx, ULONG OnuIdx, INT flag)
{
	INT k = 0,length = 0, iRes = VOS_OK ,Tvm_Count = 0, degree = 0;
	char bufchar[32], *buftemp = NULL;
	Through_Vlan_Group_t * Thr_Vlan_temp;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	Tvm_Pkt_t *Tvm_Pkt_temp = NULL;
	
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	Tvm_Count = TVM_Cont_Head_Info->TVMCOUNT;

	IGMP_TVM_DEBUG( ("\r\nIn Function : Igmp_Tvm_Table_Send \r\n") );

	if(0 == Tvm_Count)
	{
		IGMP_TVM_DEBUG( ("The Number of groups is ZERO , it doesn't send the table to the onus .\r\n") );
		return VOS_OK;	
	}
	else
	{
		buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num, MODULE_RPU_IGMPSNOOP);
		if(NULL == buftemp)
		{
			/*IGMP_TVM_DEBUG( "Through_VLAN_Oam_Pkt Alloc msg failed\r\n " );*/
			VOS_ASSERT(0);
		      	return VOS_ERROR;	
		}
	
		VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
		while(Tvm_Count > once_send_num)
		{
			VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num);
			
			Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
			if( VOS_YES == g_ulIgmp_TVM_Enabled)
			{
				Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
			}
			else
			{
				Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
			}
			Tvm_Pkt_Head_temp->type = TVM_ADD;
			Tvm_Pkt_Head_temp->CRC = 0;
			Tvm_Pkt_Head_temp->count = once_send_num;

			for(k=1; k<=once_send_num; k++)
			{
				Tvm_Pkt_temp=(Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*(k-1));
				Tvm_Pkt_temp->IVid = VOS_HTONS(Thr_Vlan_temp->IVid); 
				Tvm_Pkt_temp->GROUPS = VOS_HTONL(Thr_Vlan_temp->GROUPS);
				Tvm_Pkt_temp->GROUPE = VOS_HTONL(Thr_Vlan_temp->GROUPE);
				Tvm_Pkt_temp->llid = VOS_HTONS(Thr_Vlan_temp->llid);
				Tvm_Pkt_temp->PonId = VOS_HTONS(Thr_Vlan_temp->PonId);
				Tvm_Pkt_temp->ulIfIndex = VOS_HTONL(Thr_Vlan_temp->ulIfIndex);
				if(VOS_YES == glIgmpTvmDebug)
				{
					sys_console_printf( "\r\nThe sequence number of this group is %d \r\n",k+once_send_num*degree);
					sys_console_printf( "Tvm_Pkt_temp->IVid is %d \r\n",Tvm_Pkt_temp->IVid );
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPS);
					sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",bufchar);
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPE);
					sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",bufchar);
				}
				Thr_Vlan_temp = Thr_Vlan_temp->next;
			}
			
			degree++;

			length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num;
		
			iRes = Comm_Tvm_Frame_Send(PonPortIdx,  OnuIdx,  buftemp,length , flag);

			Tvm_Count = Tvm_Count -once_send_num;
		}

		/*下面发最后一段*/
		if(Tvm_Count > 0)
		{
			VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*Tvm_Count);

			Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
			if( VOS_YES == g_ulIgmp_TVM_Enabled)
			{
				Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
			}
			else
			{
				Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
			}
			Tvm_Pkt_Head_temp->type = TVM_ADD;
			Tvm_Pkt_Head_temp->CRC = 0;
			Tvm_Pkt_Head_temp->count = Tvm_Count;
			
			for(k=1; k<=Tvm_Count; k++)
			{
				Tvm_Pkt_temp = (Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*(k-1));
				Tvm_Pkt_temp->IVid  = VOS_HTONS(Thr_Vlan_temp->IVid); 
				Tvm_Pkt_temp->GROUPS = VOS_HTONL(Thr_Vlan_temp->GROUPS);
				Tvm_Pkt_temp->GROUPE = VOS_HTONL(Thr_Vlan_temp->GROUPE);
				Tvm_Pkt_temp->llid = VOS_HTONS(Thr_Vlan_temp->llid);
				Tvm_Pkt_temp->PonId = VOS_HTONS(Thr_Vlan_temp->PonId);
				Tvm_Pkt_temp->ulIfIndex = VOS_HTONL(Thr_Vlan_temp->ulIfIndex);
				if(VOS_YES == glIgmpTvmDebug)
				{
					sys_console_printf( "\r\nThe sequence number of this group is %d \r\n",k+once_send_num*degree);
					sys_console_printf( "Tvm_Pkt_temp->IVid is %d \r\n",Tvm_Pkt_temp->IVid );
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPS);
					sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",bufchar);
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPE);
					sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",bufchar);
				}
				Thr_Vlan_temp = Thr_Vlan_temp->next;
			}
					
			length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*Tvm_Count;

			iRes = Comm_Tvm_Frame_Send(PonPortIdx,  OnuIdx,  buftemp,length , flag);
		}

		VOS_SemGive(OnuTVMCommSemId);
		VOS_Free(buftemp);
	}

	if( VOS_OK != iRes)
	{
		iRes = VOS_ERROR;
	}
	
	return iRes;	
	
}


 VOID tvmTimerCallback()
{
  	SYS_MSG_S *pQueMsg = NULL;
 	ULONG aulMsg[ 4 ] = { 0 };
	Tvm_Pon_Onu_t *Tvm_Pon_Onu_temp = NULL;

	/*
	if(g_ulIgmp_TVM_Enabled == VOS_NO  || TVM_Cont_Head_Info->Calculates == 0 )
		return ;
	*/
	
    	pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_RPU_IGMPSNOOP);
    	if(NULL == pQueMsg )
    	{
		/*IGMP_TVM_DEBUG("PQueMsg Alloc msg failed (in tvmTimerCallback)\r\n" );*/
		VOS_ASSERT(0);
		return ;
	}
	Tvm_Pon_Onu_temp = (Tvm_Pon_Onu_t*)VOS_Malloc(sizeof(Tvm_Pon_Onu_t), MODULE_RPU_IGMPSNOOP);
	if(NULL == Tvm_Pon_Onu_temp)
	{
	      /*IGMP_TVM_DEBUG( "Tvm_Pon_Onu_temp Alloc msg failed(in tvmTimerCallback)\r\n" );*/
		  VOS_Free(pQueMsg);
 		VOS_ASSERT(0);
       	return ;	
	}
	Tvm_Pon_Onu_temp->PonId = 0;
	Tvm_Pon_Onu_temp->OnuId = 0;
	Tvm_Pon_Onu_temp->Calcus = TVM_Cont_Head_Info->Calculates;
	
	VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S));
	
    	pQueMsg->ptrMsgBody     = Tvm_Pon_Onu_temp;
    	pQueMsg->usMsgCode      = AWMC_IPMC_PRIVATE_SNOOP_TVM_CAL;
	pQueMsg->ucMsgType      = MSG_NOTIFY;
    	pQueMsg->ulDstModuleID  = MODULE_RPU_IGMPSNOOP;
    	pQueMsg->ulSrcModuleID  = MODULE_RPU_IGMPSNOOP;
    	pQueMsg->ulSrcSlotID    = DEV_GetPhySlot();
    	pQueMsg->ulDstSlotID    = DEV_GetPhySlot(); 
    	pQueMsg->ucMsgBodyStyle = MSG_BODY_MULTI;
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
	{
	      aulMsg[ 0 ] = pQueMsg->ulSrcModuleID;
	      aulMsg[ 1 ] = 0; 
	      aulMsg[ 2 ] = 0;
	      aulMsg[ 3 ] = (ULONG) pQueMsg;

		if(VOS_QueSend(g_ulIgmp_Snoop_MSQ_id, aulMsg, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
		{
	           	VOS_Free(pQueMsg);
			VOS_Free(Tvm_Pon_Onu_temp);
		}
	}
	else
	{
		Igmp_Tvm_Recv_Cal(pQueMsg);
		VOS_Free(pQueMsg->ptrMsgBody);
		VOS_Free( pQueMsg );
	}

}

void funIgmpTvmOamReqCallBack(unsigned short usPonId,  
                                      unsigned short usOnuId, unsigned short llid,  unsigned short usDatLen, 
                                      unsigned char *pDatBuf,  unsigned char *pSessId)
{
    	SYS_MSG_S *pQueMsg = NULL;
 	ULONG aulMsg[ 4 ] = { 0 };
	Tvm_Pon_Onu_t *Tvm_Pon_Onu_temp = NULL;
	
    	pQueMsg = (SYS_MSG_S*)VOS_Malloc(sizeof(SYS_MSG_S), MODULE_RPU_IGMPSNOOP);
    	if(NULL == pQueMsg)
    	{
		/*IGMP_TVM_DEBUG("PQueMsg Alloc msg failed (in funIgmpTvmOamReqCallBack)\r\n" );*/
		VOS_ASSERT(0);
        	VOS_Free(pDatBuf);
      		VOS_Free(pSessId);
		return ;
	}
	Tvm_Pon_Onu_temp = (Tvm_Pon_Onu_t*)VOS_Malloc(sizeof(Tvm_Pon_Onu_t), MODULE_RPU_IGMPSNOOP);
	if(NULL == Tvm_Pon_Onu_temp)
    	{
	      /*IGMP_TVM_DEBUG( "Tvm_Pon_Onu_temp Alloc msg failed(in funIgmpTvmOamReqCallBack)\r\n" );*/
		VOS_ASSERT(0);
        	VOS_Free(pDatBuf);
      		VOS_Free(pSessId);
		VOS_Free(pQueMsg);
		return ;
	}
	Tvm_Pon_Onu_temp->PonId = usPonId;
	Tvm_Pon_Onu_temp->OnuId = usOnuId;
	Tvm_Pon_Onu_temp->Calcus = 0;
	
	VOS_MemZero(pQueMsg, sizeof(SYS_MSG_S));
	
    	pQueMsg->ptrMsgBody     = Tvm_Pon_Onu_temp;
    	pQueMsg->usMsgCode      = AWMC_IPMC_PRIVATE_SNOOP_TVM_MOD;
	pQueMsg->ucMsgType      = MSG_NOTIFY;
    	pQueMsg->ulDstModuleID  = MODULE_RPU_IGMPSNOOP;
    	pQueMsg->ulSrcModuleID  = MODULE_RPU_IGMPSNOOP;
    	pQueMsg->ulSrcSlotID    = *((ULONG*)pSessId);
    	pQueMsg->ulDstSlotID    = *((ULONG*)(pSessId+4) ); 
    	pQueMsg->ucMsgBodyStyle = MSG_BODY_MULTI;
	if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
	{
	      aulMsg[ 0 ] = pQueMsg->ulSrcModuleID;
	      aulMsg[ 1 ] = 0; 
	      aulMsg[ 2 ] = 0;
	      aulMsg[ 3 ] = (ULONG) pQueMsg;

		if(VOS_QueSend(g_ulIgmp_Snoop_MSQ_id, aulMsg, NO_WAIT, MSG_PRI_NORMAL) != VOS_OK)
		{
		      IGMP_TVM_DEBUG( ("Failed when it sends the message about send groups in the table to the ONU (Unicast).\r\n") );
	           	VOS_Free(pQueMsg);
			VOS_Free(Tvm_Pon_Onu_temp);
		}
	}
	else
	{
		Igmp_Tvm_Recv_Mod(pQueMsg);
		VOS_Free(pQueMsg->ptrMsgBody);
		VOS_Free( pQueMsg );
	}
	VOS_Free(pDatBuf);
    	VOS_Free(pSessId);
		
}

STATUS Igmp_snoop_Stop_Tvm(struct vty *vty)
{
	USHORT  length = 0, iRes=VOS_OK;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	
	g_ulIgmp_TVM_Enabled = VOS_NO;
	tvm_timer_period = 120;
	
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
	{
		char *buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t), MODULE_RPU_IGMPSNOOP);
		if(NULL == buftemp)
		{
			/*IGMP_TVM_DEBUG( "Through_VLAN_Oam_Pkt Alloc msg failed\r\n " );*/
			VOS_ASSERT(0);
		      	return VOS_ERROR;	
		}
		
		VOS_MemZero(buftemp, sizeof(buftemp));

		Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
		Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
		Tvm_Pkt_Head_temp->type = TVM_DEL;
		Tvm_Pkt_Head_temp->CRC = 0;
		Tvm_Pkt_Head_temp->count = 0;
		
		length = sizeof(Tvm_Pkt_Head_t);
		
		iRes = Comm_Tvm_Frame_Send(0,0,buftemp,length,TVM_BROADCAST);

		VOS_Free(buftemp);
		if( VOS_OK != iRes)
		{
			iRes = VOS_OK;
		}
	}
	return iRes;
}

int Stop_Igmp_Tvm(unsigned int PonPortIdx,void *vty)
{
	int iRlt,olt_id;

	if ( IGMPTVM_PONPORTIDX_ALL != PonPortIdx )
	{   
		IGMPTVM_ASSERT(PonPortIdx);
		iRlt = IGMPTVM_API_CALL( PonPortIdx, igmp_tvm_stop_t, (PonPortIdx, vty) );
	}
	else
	{

		for ( olt_id = 0; olt_id < MAXPON; olt_id += PONPORTPERCARD )
		{
			if ( VOS_OK != (iRlt = IGMPTVM_API_CALL( olt_id, igmp_tvm_stop_t, (olt_id, vty))) )
			{
				break;
			}
		}
	}

	if ( VOS_OK == iRlt )
    	{
		Igmp_snoop_Stop_Tvm((struct vty *) vty);
	}
    	else
    	{
    		if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER && vty != NULL)
		vty_out(vty,"Stop Igmp Snoop Tvm Failed !\r\n");
	}
	return iRlt;

}
STATUS Igmp_Tvm_Debug_Head(char *buftemp)
{
	ULONG  tvmcalcu = 0;
	USHORT   tvmable = 0, tvmtype= 0, tvmcount = 0;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	
	Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
	tvmable = Tvm_Pkt_Head_temp->enable;
	tvmtype = Tvm_Pkt_Head_temp->type;
	tvmcalcu = Tvm_Pkt_Head_temp->CRC;
	tvmcount = Tvm_Pkt_Head_temp->count;
	sys_console_printf("\r\nThe number about Enable is %d\r\n",tvmable);
	sys_console_printf("The number about type is %d\r\n",tvmtype);
	sys_console_printf("The number about calcu is %d\r\n",tvmcalcu);
	sys_console_printf("The number about count is %d\r\n",tvmcount);
	
	return VOS_OK;
}

STATUS Register_Tvm_Send()
{
	if(VOS_YES == g_ulIgmp_TVM_Enabled)
	{
		tvmTimerCallback();
	}
	return VOS_OK;
}

STATUS CliIgmpTvmShowRun(struct vty *vty)
{
	char buftemps[32], buftempe[32];
	Through_Vlan_Group_t  * Thr_Vlan_temp = NULL;
    if ( NULL != TVM_Cont_Head_Info )
	{
		Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;

		VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);

		vty_out(vty,"\r\n");
		while(NULL != Thr_Vlan_temp)
		{	
			VOS_MemZero(buftemps, sizeof(buftemps));
			VOS_MemZero(buftempe, sizeof(buftempe));
			get_ipdotstring_from_long(buftemps,Thr_Vlan_temp->GROUPS);
			get_ipdotstring_from_long(buftempe,Thr_Vlan_temp->GROUPE);
			vty_out( vty, " igmp-snooping-tvm add %s %s %d\r\n",buftemps,buftempe,Thr_Vlan_temp->IVid);
			Thr_Vlan_temp = Thr_Vlan_temp->next;
		}

		if( CRC_CHECK_INTERVAL_DEFAULT != tvm_timer_period )
			vty_out(vty," igmp-snooping-tvm sync-interval %d\r\n", tvm_timer_period);
		
		VOS_SemGive(OnuTVMCommSemId);
	}
	return VOS_OK;
	
}

int IgmpTvm_Poncard_Insert( LONG slotno, LONG module_type  )
{
	INT k = 0,length = 0, iRes = VOS_OK ,Tvm_Count = 0, degree = 0;
	char bufchar[32], *buftemp = NULL;
	Through_Vlan_Group_t * Thr_Vlan_temp;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	Tvm_Pkt_t *Tvm_Pkt_temp = NULL;
	if (module_type <= MODULE_TYPE_UNKNOW)
		return VOS_OK;
#if 0		
	if(!SYS_MODULE_TYPE_IS_6900_EPON(module_type) &&  !SYS_MODULE_TYPE_IS_8000_EPON(module_type) )
		return VOS_OK;
#else
    if(!SYS_MODULE_TYPE_IS_CPU_PON(module_type))
        return VOS_OK;
#endif
	Thr_Vlan_temp = TVM_Cont_Head_Info->Through_Vlan_Group_head;
	Tvm_Count = TVM_Cont_Head_Info->TVMCOUNT;
	
	if(0 == Tvm_Count)
	{
		length = 0;
		buftemp = NULL;
		Tvm_Insert_Cdp_Send(slotno, buftemp, length);
	}
	else
	{
		buftemp = (char *)VOS_Malloc(sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num, MODULE_RPU_IGMPSNOOP);
		if(NULL == buftemp)
		{
			VOS_ASSERT(0);
		      	return VOS_ERROR;	
		}
	
		VOS_SemTake(OnuTVMCommSemId, WAIT_FOREVER);
		while(Tvm_Count > once_send_num)
		{
			VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num);
			
			Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
			if( VOS_YES == g_ulIgmp_TVM_Enabled)
			{
				Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
			}
			else
			{
				Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
			}
			Tvm_Pkt_Head_temp->type = TVM_ADD;
			Tvm_Pkt_Head_temp->CRC = 0;
			Tvm_Pkt_Head_temp->count = once_send_num;

			for(k=1; k<=once_send_num; k++)
			{
				Tvm_Pkt_temp=(Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*(k-1));
				Tvm_Pkt_temp->IVid = VOS_HTONS(Thr_Vlan_temp->IVid); 
				Tvm_Pkt_temp->GROUPS = VOS_HTONL(Thr_Vlan_temp->GROUPS);
				Tvm_Pkt_temp->GROUPE = VOS_HTONL(Thr_Vlan_temp->GROUPE);
				Tvm_Pkt_temp->llid = VOS_HTONS(Thr_Vlan_temp->llid);
				Tvm_Pkt_temp->PonId = VOS_HTONS(Thr_Vlan_temp->PonId);
				Tvm_Pkt_temp->ulIfIndex = VOS_HTONL(Thr_Vlan_temp->ulIfIndex);
				if(VOS_YES == glIgmpTvmDebug)
				{
					sys_console_printf( "\r\nThe sequence number of this group is %d \r\n",k+once_send_num*degree);
					sys_console_printf( "Tvm_Pkt_temp->IVid is %d \r\n",Tvm_Pkt_temp->IVid );
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPS);
					sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",bufchar);
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPE);
					sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",bufchar);
				}
				Thr_Vlan_temp = Thr_Vlan_temp->next;
			}
			
			degree++;

			length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*once_send_num;
		
			iRes = Tvm_Insert_Cdp_Send(slotno, buftemp, length);

			Tvm_Count = Tvm_Count -once_send_num;
		}

		/*下面发最后一段*/
		if(Tvm_Count > 0)
		{
			VOS_MemZero(buftemp, sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*Tvm_Count);

			Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)buftemp;
			if( VOS_YES == g_ulIgmp_TVM_Enabled)
			{
				Tvm_Pkt_Head_temp->enable = TVM_ENABLE;
			}
			else
			{
				Tvm_Pkt_Head_temp->enable = TVM_DISABLE;
			}
			Tvm_Pkt_Head_temp->type = TVM_ADD;
			Tvm_Pkt_Head_temp->CRC = 0;
			Tvm_Pkt_Head_temp->count = Tvm_Count;
			
			for(k=1; k<=Tvm_Count; k++)
			{
				Tvm_Pkt_temp = (Tvm_Pkt_t *)(buftemp+sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*(k-1));
				Tvm_Pkt_temp->IVid  = VOS_HTONS(Thr_Vlan_temp->IVid); 
				Tvm_Pkt_temp->GROUPS = VOS_HTONL(Thr_Vlan_temp->GROUPS);
				Tvm_Pkt_temp->GROUPE = VOS_HTONL(Thr_Vlan_temp->GROUPE);
				Tvm_Pkt_temp->llid = VOS_HTONS(Thr_Vlan_temp->llid);
				Tvm_Pkt_temp->PonId = VOS_HTONS(Thr_Vlan_temp->PonId);
				Tvm_Pkt_temp->ulIfIndex = VOS_HTONL(Thr_Vlan_temp->ulIfIndex);
				if(VOS_YES == glIgmpTvmDebug)
				{
					sys_console_printf( "\r\nThe sequence number of this group is %d \r\n",k+once_send_num*degree);
					sys_console_printf( "Tvm_Pkt_temp->IVid is %d \r\n",Tvm_Pkt_temp->IVid );
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPS);
					sys_console_printf("Thr_Vlan_temp->GROUPS is %-14s\r\n",bufchar);
					VOS_MemZero(bufchar, sizeof(bufchar));
					get_ipdotstring_from_long(bufchar,Thr_Vlan_temp->GROUPE);
					sys_console_printf("Thr_Vlan_temp->GROUPE is %-14s\r\n",bufchar);
				}
				Thr_Vlan_temp = Thr_Vlan_temp->next;
			}
					
			length = sizeof(Tvm_Pkt_Head_t)+sizeof(Tvm_Pkt_t)*Tvm_Count;

			iRes = Tvm_Insert_Cdp_Send(slotno, buftemp, length);
		}

		VOS_SemGive(OnuTVMCommSemId);
		VOS_Free(buftemp);
	}

	if( VOS_OK != iRes)
	{
		iRes = VOS_ERROR;
	}
	
	return iRes;	

}

STATUS  TvmInsert_Poncard_Insert_Callback(ULONG ulFlag, ULONG ulChID, ULONG ulDstNode,	 ULONG ulDstChId, VOID  *pData, ULONG ulDataLen)
{
       TvmInsertCDPMsgHead_t * revBuf = NULL ;
	Tvm_Pkt_t *Tvm_Pkt_temp = NULL, *Tvm_Pkt_temp2 = NULL;
	Tvm_Pkt_Head_t *Tvm_Pkt_Head_temp = NULL;
	int i=0, lRet;
	
	switch( ulFlag )
	{
		case CDP_NOTI_FLG_RXDATA: /* 收到数据*/
			if( pData == NULL )
			{
				VOS_ASSERT( 0 );
				return VOS_ERROR;
			}
			/* sys_console_printf("\r\nIn Function :  Loop_Poncard_Insert_CDP_Callback\r\n"); */
			revBuf=(TvmInsertCDPMsgHead_t *)(( SYS_MSG_S * )pData + 1);

			if(ulDstChId != RPU_TID_CDP_IGMP_TVM|| ulChID != RPU_TID_CDP_IGMP_TVM)
			{
				CDP_FreeMsg(pData);
				return VOS_ERROR;
			}

			g_ulIgmp_TVM_Enabled = revBuf->enable;
			tvm_timer_period = revBuf->sync;

			Tvm_Pkt_Head_temp = (Tvm_Pkt_Head_t *)(revBuf+1);
			if(revBuf->count > 0)
			{
				Tvm_Pkt_temp2= (Tvm_Pkt_t *)(Tvm_Pkt_Head_temp + 1);
				
				for(i=0; i<Tvm_Pkt_Head_temp->count; i++)
				{
					Tvm_Pkt_temp = Tvm_Pkt_temp2;
					lRet = AddIgmpSnoopTvmItem(Tvm_Pkt_temp->GROUPS, Tvm_Pkt_temp->GROUPE, Tvm_Pkt_temp->IVid, NULL);
					if(lRet != VOS_OK)
					{
						sys_console_printf("AddIgmpSnoopTvmItem failed !\r\n");
					}
					Tvm_Pkt_temp2 = (Tvm_Pkt_t *)(Tvm_Pkt_temp+1);
				}

				lRet = Tvm_Table_Crc();
			}
			CDP_FreeMsg(pData);
			break;
		case CDP_NOTI_FLG_SEND_FINISH:/*异步发送时*/
			CDP_FreeMsg(pData);		/*异步发送失败暂不处理，但需要释放消息*/
			break;
		default:
			ASSERT(0);
			CDP_FreeMsg(pData);
			break;
	}

	return VOS_OK;
}

int Tvm_Insert_Cdp_Send(LONG slotno, char *buf, USHORT length)
{
	unsigned int ulLen = 0;
    	SYS_MSG_S      *pMsg       = NULL;
	TvmInsertCDPMsgHead_t *TvmCdpHead = NULL;
	
       ulLen=sizeof(TvmInsertCDPMsgHead_t)+sizeof(SYS_MSG_S)+length;
	pMsg=(SYS_MSG_S*)CDP_AllocMsg(ulLen, MODULE_RPU_IGMPSNOOP);

       if( NULL == pMsg )
       {
            VOS_ASSERT(0);
            return  VOS_ERROR;
       }
       VOS_MemZero((CHAR *)pMsg, ulLen );

       SYS_MSG_SRC_ID( pMsg )       = MODULE_RPU_IGMPSNOOP;
        SYS_MSG_DST_ID( pMsg )       = MODULE_RPU_IGMPSNOOP;
        SYS_MSG_MSG_TYPE( pMsg )     = MSG_NOTIFY;
        SYS_MSG_FRAME_LEN( pMsg )    = ulLen;
        SYS_MSG_BODY_STYLE( pMsg )   = MSG_BODY_INTEGRATIVE;
        SYS_MSG_BODY_POINTER( pMsg ) = pMsg + 1;
        SYS_MSG_SRC_SLOT( pMsg )     = DEV_GetPhySlot();
        SYS_MSG_DST_SLOT( pMsg ) = SYS_MASTER_ACTIVE_SLOTNO;
		
	TvmCdpHead= (TvmInsertCDPMsgHead_t * ) ( pMsg + 1 );
	TvmCdpHead->enable = g_ulIgmp_TVM_Enabled;
	TvmCdpHead->sync = tvm_timer_period;
	if(length != 0)
		TvmCdpHead->count = (length-sizeof(Tvm_Pkt_Head_t))/sizeof(Tvm_Pkt_t);
	else
		TvmCdpHead->count = 0;
	
	if(0 != TvmCdpHead->count && buf != NULL)
		VOS_MemCpy((void *)(TvmCdpHead+1), buf, length);
		
       if ( VOS_OK !=  CDP_Send( RPU_TID_CDP_IGMP_TVM, slotno, RPU_TID_CDP_IGMP_TVM, /*CDP_MSG_TM_ASYNC*/ 0,\
                 (VOID *)pMsg, ulLen, MODULE_RPU_IGMPSNOOP ) )
	{
		VOS_ASSERT(0); 
		sys_console_printf("\r\n%%Send the igmp snoop tvm configuration to the new poncard Failed !\r\n");
		CDP_FreeMsg(pMsg);
		return VOS_ERROR;
	}
	/*else
	 {
		sys_console_printf("Send the igmp snoop tvm configuration to the new poncard successed ! \r\n");
	 }*/

	return VOS_OK;

}


#endif

#ifdef __cplusplus
}
#endif


