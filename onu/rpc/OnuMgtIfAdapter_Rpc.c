/***************************************************************
*
*						Module Name:  OnuMgtIfAdapter_Rpc.c
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

#include "syscfg.h"
#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/cdp_syn.h"
#include "vos_global.h"
#include "vos/vospubh/vos_types.h"
#include "vos/vospubh/vos_string.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/cdp_pub.h"
#include "sys/devsm/devsm_remote.h"

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include 	"OnuGeneral.h"
#include "ct_manage/CT_Onu_Voip.h"

#include  "onu/onuConfMgt.h"

#define NAMELEN 256
#define SUPPORT_ATTRIBUTE_LEN 17

/* 板间通信使用以太帧，帧长不小于64字节[这里每次占用32字节，不浪费] */
typedef union
{
    ULONG  ulVal;
    ULONG  ulValPair[2];
    ULONG  ulVals[8];
    int    iVal;
    int    iValPair[2];
    int    iVals[8];
    unsigned int uiVal;
    unsigned int uiValPair[2];
    unsigned int uiVals[8];   
    USHORT usVal;
    USHORT usValPair[2];
    USHORT usVals[16];
    SHORT  sVal;
    SHORT  sValPair[2];
    SHORT  sVals[16];
    UCHAR  ucVal;
    UCHAR  ucValPair[2];
    UCHAR  aucVal[32];
    CHAR   cVal;
    CHAR   cValPair[2];
    CHAR   acVal[32];
    bool   bVal;
    bool   bValPair[2];
    bool   bVals[32];
}ONU_VALUE_S;

typedef struct stOnuCmdHead
{
    int cmd;
    short int olt_id;
    short int llid;
    UCHAR params[0];
}ONU_CMD_HEAD;

typedef struct stOnuCmd
{
    int cmd;
    short int olt_id;
    short int llid;
    ONU_VALUE_S params;
}ONU_CMD;

typedef struct stOnuRlt
{
    int rlt;
    UCHAR values[0];
}ONU_RLT;

typedef struct stOnuEvt
{
    int evt;
    short int olt_id;
    short int llid;
    int param1;
    int param2;   
}ONU_EVT;


extern PON_onu_address_table_record_t  MAC_Address_Table[8192];
extern char cli_recvbuf[64*1024];
/*-------------------------------------RPC系统接口--------------------------------------------*/
#define ONU_RPC_CMD_NEW(pRpcCmd) ( NULL != (pRpcCmd = CDP_SYNC_AllocMsg(sizeof(ONU_CMD), MODULE_RPU_ONU)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define ONU_RPC_CMD_NEW_EX(pRpcCmd, size) ( NULL != (pRpcCmd = CDP_SYNC_AllocMsg(sizeof(ONU_CMD_HEAD) + (size), MODULE_RPU_ONU)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define ONU_RPC_RLT_NEW(pRpcRlt, size) ( NULL != (pRpcRlt = CDP_SYNC_AllocMsg((size), MODULE_RPU_ONU)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define ONU_RPC_RLT_FREE(pRpcRlt)      CDP_SYNC_FreeMsg(pRpcRlt)


int ONU_Rpc_Call(short int olt_id, short int onu_id,int *olt_slot, int onu_cmd, VOID *pSndValue, ULONG ulSndLen, VOID *pRcvData, ULONG ulRcvLen, ULONG ulTimeOut, ULONG ulCallFlags)
{
    ULONG ulRet;
    int iRlt = OLT_ERR_OK;
    VOID *pTmpRcvData = NULL;
    ULONG ulTmpRcvLen = 0;
    ULONG ulSendLen;
    ULONG ulDstSlot;
    int   iDstSlot, iDstPort;
    ONU_CMD_HEAD *pRpcCmd;
    ONU_RLT *pCmdResult;
    short int dst_id; 

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(olt_slot);
    ONU_CMD_ASSERT(onu_cmd);

    if ( 0 == (iDstSlot = OLT_SLOT_ID(olt_id)) )
    {
        VOS_ASSERT(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER);
    
        /* 得到OLT所在的PON板端口号 */
        if ( RERROR == (iDstPort = GetPonPortByPonChip(olt_id)) )
        {
            return OLT_ERR_PARAM;
        }

        /* 全局OLT ID到本地PON板OLT ID的转换 */
        dst_id = iDstPort - 1;

        /* 得到OLT所在的PON板槽位 */
        if ( RERROR == (iDstSlot = GetCardIdxByPonChip(olt_id)) )
        {
            return OLT_ERR_PARAM;
        }

        ulDstSlot = iDstSlot;
    }
    else
    {
        iDstPort = OLT_PORT_ID(olt_id);
        
        if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
        {
            if ( SYS_MODULE_IS_LOCAL(iDstSlot) )
            {
                /* 主控PON板发出的RPC无需中转 */
                ulDstSlot = iDstSlot;
                
                /* 全局OLT ID到本地PON板OLT ID的转换 */
                dst_id = iDstPort - 1;
            }
            else
            {
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
                ulDstSlot = (ULONG)LSLOT_TO_CCSLOT(iDstSlot);               
            
                /* 全局OLT ID到远端目标PON板OLT ID的转换 */
                dst_id = OLTRM_GetRemoteOltID(iDstSlot, iDstPort);
                if ( OLT_ID_INVALID == dst_id )
                {
                    return OLT_ERR_NOTEXIST;
                }
#else
                return OLT_ERR_NOTSUPPORT;
#endif
            }
        }   
        else
        {
            if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
            {
                /* PON板发出的RPC都经由主控板来中转 */
                ulDstSlot = SYS_MASTER_ACTIVE_SLOTNO;
                
                /* 全局OLT ID中转 */
                dst_id = olt_id;
            }
            else
            {
                /* 备用主控或其它业务板，禁止RPC */
                return OLT_ERR_NOTEXIST;
            }        
        }
    }
    *olt_slot = (int)ulDstSlot;

    if ( OLT_RPC_CALLFLAG_OUTPUTASINPUT & ulCallFlags )
    {
        /* 输出参数合并为输入参数 */
        VOS_ASSERT(pSndValue);
        ulSndLen += ulRcvLen;
    }

    if (ulSndLen <= sizeof(ONU_VALUE_S))
    {
        iRlt = ONU_RPC_CMD_NEW(pRpcCmd);
        ulSendLen = sizeof(ONU_CMD);
    }
    else
    {
        iRlt = ONU_RPC_CMD_NEW_EX(pRpcCmd, ulSndLen);
        ulSendLen = ulSndLen + sizeof(ONU_CMD_HEAD);
    }
		
    if ( OLT_ERR_OK == iRlt )
    {
        /* 统一的RPC输入参数处理 */
        pRpcCmd->cmd    = onu_cmd;

        /* OltID在目标板的定位转换 */
        pRpcCmd->olt_id = dst_id;
        pRpcCmd->llid   = onu_id;	

        if ( NULL != pSndValue )
        {
            if ( OLT_RPC_CALLFLAG_OUTPUTASINPUT & ulCallFlags )
            {
                /* 输出参数合并到输入参数 */
                ulSndLen -= ulRcvLen;
                VOS_MemCpy(&pRpcCmd->params, pSndValue, ulSndLen);
                VOS_MemCpy(&pRpcCmd->params[ulSndLen], pRcvData, ulRcvLen);

                pRcvData = NULL;
            }
            else
            {
                VOS_MemCpy(&pRpcCmd->params, pSndValue, ulSndLen);
            }
        }
        
        if (0 == ulTimeOut)
        {
            ulTimeOut = 10000;
        }

        if ( OLT_RPC_CALLFLAG_ASYNC & ulCallFlags )
        {
            CDP_SYNC_Header_S *pstCdpSyncHead = ((CDP_SYNC_Header_S*)pRpcCmd) - 1;

            pstCdpSyncHead->ulSrcSlot = SYS_LOCAL_MODULE_SLOTNO;
            pstCdpSyncHead->ulDstSlot = ulDstSlot;
            pstCdpSyncHead->ulSrcMod = MODULE_ONU;
            pstCdpSyncHead->ulDstMod = MODULE_ONU;
            pstCdpSyncHead->usType = 0;
            pstCdpSyncHead->usId   = 0;
            ulRet = CDP_Send(RPU_TID_CDP_OLT, ulDstSlot, RPU_TID_CDP_OLT, CDP_MSG_TM_SYNC, pstCdpSyncHead, ulSendLen + sizeof(CDP_SYNC_Header_S), MODULE_ONU);
            pTmpRcvData = NULL;

            /* CDP的同步发送，必须手动释放消息块 */
            CDP_FreeMsg( pstCdpSyncHead );
        }
        else
        {
            ulRet = CDP_SYNC_Call(MODULE_ONU, ulDstSlot, MODULE_ONU, 0, pRpcCmd, ulSendLen, &pTmpRcvData, &ulTmpRcvLen, ulTimeOut);
        }

        if ( VOS_OK == ulRet )
        {
            if ( NULL != (pCmdResult = (ONU_RLT *)pTmpRcvData) )
            {
                if ( ulTmpRcvLen >= sizeof(ONU_RLT) )
                {
                    /* 统一的RPC返回值处理 */
                    iRlt = pCmdResult->rlt;

                    /* 统一的RPC输出参数处理 */
                    if ( NULL != pRcvData )
                    {
                        if ( 0 < (ulTmpRcvLen -= sizeof(ONU_RLT)) )
                        {
                            if ( !(ulCallFlags & OLT_RPC_CALLFLAG_MANUALFREERLT) )
                            {
                                if ( ulRcvLen >= ulTmpRcvLen )
                                {
                                    VOS_MemCpy(pRcvData, pCmdResult->values, ulTmpRcvLen);
                                }
                                else
                                {
                                    iRlt = OLT_ERR_PARAM;
                                }
                                
                                CDP_SYNC_FreeMsg(pTmpRcvData);
                            }
                            else
                            {
                                *(ONU_RLT**)pRcvData = pCmdResult;
                                VOS_ASSERT(ulRcvLen == sizeof(ONU_RLT*));
                            }
                        }
                        else
                        {
                            /* 仅返回结果值 */
                            CDP_SYNC_FreeMsg(pTmpRcvData);
                        }
                    }
                    else
                    {
                        /* 仅需要结果值 */
                        CDP_SYNC_FreeMsg(pTmpRcvData);
                    }
                }
                else
                {
                    CDP_SYNC_FreeMsg(pTmpRcvData);
                }
            }
            else
            {
                if ( !(OLT_RPC_CALLFLAG_ASYNC & ulCallFlags) )
                {
                    iRlt = OLT_ERR_MALLOC;
                }
            }
        }
        else
        {
            switch (ulRet)
            {
                case CDP_SYNC_OUT_TIMEOUT:
                    iRlt = OLT_ERR_TIMEOUT;
                    break;
                case CDP_SYNC_NO_BUF:
                    iRlt = OLT_ERR_MALLOC;
                    break;
                default:
                    iRlt = OLT_ERR_UNKNOEWN;
            }
        }
    }

    VOS_SysLog(LOG_TYPE_ONU, LOG_DEBUG, "ONU_Rpc_Call(oltid:%d, onuid:%d)'s result(%d) send cmd[%d] to pon%d/%d from slot%d", olt_id, onu_id,iRlt, onu_cmd, iDstSlot, iDstPort, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}
int debug_rpc = 0;
int ONU_Rpc_Callback( ULONG ulSrcSlot, ULONG ulSrcMod, VOID *pRcvData, ULONG ulRcvDataLen, VOID **ppSendData, ULONG *pulSendDataLen )   
{
    int iRet;
    ONU_CMD *pRpcCmd;
    /*ONU_EVT *pRpcEvt;*/
    ONU_RLT *pRpcResult = NULL;
    ULONG ulResultLen;
    short int olt_id, onu_id, slot_id;
   
    VOS_ASSERT(pRcvData);
    VOS_ASSERT(ulRcvDataLen >= sizeof(ONU_CMD));
    VOS_ASSERT(ppSendData);
    VOS_ASSERT(pulSendDataLen);

    /* 主控板向业务板发送RPC，目前主要用于配置 */
    pRpcCmd = (ONU_CMD*)pRcvData;
    olt_id = pRpcCmd->olt_id;
    onu_id = pRpcCmd->llid;
    if ( 0 != (slot_id = OLT_SLOT_ID(olt_id)) )
    {
        /* PON板发出的全局OLT ID处理请求的接收中转处 */
        VOS_ASSERT(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER);
        
        if (SYS_MODULE_IS_LOCAL(slot_id))
        {
            /* 全局OLT ID在本地PON处理的中转 */
            olt_id = GetPonPortIdxBySlot(slot_id, OLT_PORT_ID(olt_id));
            OLT_LOCAL_ASSERT(olt_id);
        }
        else
        {
            /* 全局OLT ID在远程PON处理的中转 */
        }
    }
	
    /*added by luh@2015-03-11, 配置恢复过程中不允许对ONU 进行操作*/
    if(getOnuConfRestoreFlag(olt_id, onu_id))
    {
        iRet = OLT_ERR_UNKNOEWN;
    }
    else
    {
    switch ( pRpcCmd->cmd )
    {
#if 1
/* -------------------ONU基本API------------------- */
        case ONU_CMD_ONU_IS_VALID_1:
            ulResultLen = sizeof(ONU_RLT) + sizeof(int);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_OnuIsValid( olt_id, onu_id, pRpcResult + 1 );
            }
            
            break;
        case ONU_CMD_ONU_IS_ONLINE_2:
            ulResultLen = sizeof(ONU_RLT) + sizeof(int);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_OnuIsOnline( olt_id, onu_id, pRpcResult + 1 );
            }
            
            break;
        case ONU_CMD_ADD_ONU_MANURAL_3:
        	ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_AddOnuByManual( olt_id, onu_id, pRpcCmd->params.acVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddOnuByManual(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
        	break;
        case  ONU_CMD_DEL_ONU_MANURAL_4:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_DelOnuByManual( olt_id, onu_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelOnuByManual(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
        	break;
        case  ONU_CMD_IS_SUPPORT_CMD_5:
            ulResultLen = sizeof(ONU_RLT) + sizeof(short int);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                short int *psCmdId = pRpcResult + 1;

                *psCmdId = pRpcCmd->params.sVal;
                    
                iRet = OnuMgt_CmdIsSupported( olt_id, onu_id, psCmdId );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CmdIsSupported(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *psCmdId, iRet, ulSrcSlot);
            }   
        	break;
        case ONU_CMD_COPY_ONU_6:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_CopyOnu(olt_id, onu_id, (short int)pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CopyOnu(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }   
        }
        break;	
        case  ONU_CMD_GET_IFTYPE_7:
            ulResultLen = sizeof(ONU_RLT) + sizeof(int) * 2;
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *aiParams = pRpcResult + 1;
                    
                iRet = OnuMgt_GetIFType( olt_id, onu_id, &aiParams[0], &aiParams[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetIFType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, aiParams[0], aiParams[1], iRet, ulSrcSlot);
            }   
        	break;
        case  ONU_CMD_SET_IFTYPE_8:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_SetIFType( olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIFType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }   
        	break;
        case ONU_CMD_MODIFY_ONU_MANURAL_9:
        	ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_ModifyOnuByManual( olt_id, onu_id, pRpcCmd->params.acVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ModifyOnuByManual(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }               
            break;
        case ONU_CMD_ADD_GPON_ONU_MANURAL_10:
        	ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_AddGponOnuByManual( olt_id, onu_id, pRpcCmd->params.acVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddOnuByManual(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
        	break;
#endif

			
#if 1
/* -------------------ONU 认证管理API------------------- */
        case  ONU_CMD_DEREGISTERT_ONU_1:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_DeregisterOnu( olt_id, onu_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DeregisterOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }  
            
            break;
        case ONU_CMD_SET_MAC_AUTH_MODE_2:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_SetMacAuthMode(olt_id, onu_id, (int)(pRpcCmd->params.aucVal[0]), &(pRpcCmd->params.aucVal[1]));	                    
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMacAuthMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
        }   
        
        break;
        case ONU_CMD_DEL_BIND_ONU_3:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet =  OnuMgt_DelBindingOnu(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelBindingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
        }
        
        break;	
#if 0
        case ONU_CMD_GW_ADD_PENDING_ONU_4:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_AddPendingOnu( olt_id, onu_id, pRpcCmd->params.acVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddPendingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
            
            break;
        case  ONU_CMD_GW_DEL_PENDING_ONU_5:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_DelPendingOnu( olt_id, onu_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelPendingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
            
            break;
        case  ONU_CMD_DEL_CONF_PENDING_ONU_6:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_DelConfPendingOnu( olt_id, onu_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelConfPendingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }   
            break;
#endif
        case  ONU_CMD_AUTHORIZE_ONU_7:
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_AuthorizeOnu( olt_id, onu_id, pRpcCmd->params.ucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AuthorizeOnu(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ucVal, iRet, ulSrcSlot);
            }   
            break;
#endif


#if 1
/* -------------------ONU 业务管理API------------------- */
    case ONU_CMD_SET_TRAFFIC_SERVICE_MODE_1:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuTrafficServiceMode( olt_id, onu_id, pRpcCmd->params.iVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuTrafficServiceMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
        }        
        
        break;
    case ONU_CMD_SET_ONU_PEER_TO_PEER_2:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuPeerToPeer( olt_id, onu_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1], iRet, ulSrcSlot);
        }   
        
        break;	
    case ONU_CMD_SET_ONU_PEER_TO_PEER_FORWARD_3:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuPeerToPeerForward( olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
        }   
        
        break;
    case  ONU_CMD_SET_ONU_BW_4:
        ulResultLen = sizeof(ONU_RLT) + sizeof(ONU_bw_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            ONU_bw_t *BW = pRpcResult + 1;

            VOS_MemCpy(BW, &pRpcCmd->params, sizeof(ONU_bw_t));
            iRet =  OnuMgt_SetOnuBW( olt_id, onu_id, BW );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuBW(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, iRet, ulSrcSlot);
        }   
        
        break;
    case  ONU_CMD_GET_ONU_SLA_5:
        ulResultLen = sizeof(ONU_RLT) + sizeof(ONU_SLA_INFO_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            ONU_SLA_INFO_t *SLA = pRpcResult + 1;

            iRet =  OnuMgt_GetOnuSLA( olt_id, onu_id, SLA );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuSLA(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, SLA->SLA_Ver, SLA->DBA_ErrCode, iRet, ulSrcSlot);
        }   
        
        break;
    case ONU_CMD_SET_FEC_MODE_6:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuFecMode( olt_id, onu_id, pRpcCmd->params.iVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuFecMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
        }
        
        break;
    case ONU_CMD_GET_ONU_VLAN_MODE_7:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(PON_olt_vlan_uplink_config_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet =  OnuMgt_GetOnuVlanMode( olt_id, onu_id, pRpcResult + 1 );	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuVlanMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }   
    
    break;
	/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	case ONU_CMD_SET_UNI_PORT_8:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetUniPort( olt_id, onu_id, pRpcCmd->params.bVals[0], pRpcCmd->params.bVals[1] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetUniPort(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.bVals[0], pRpcCmd->params.bVals[1], iRet, ulSrcSlot);
        }   
        
        break;
	case ONU_CMD_SET_SLOW_PROTOCOL_LIMIT_9:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetSlowProtocolLimit( olt_id, onu_id, pRpcCmd->params.bVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetSlowProtocolLimit(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.bVal, iRet, ulSrcSlot);
        }   
        
        break;
	case ONU_CMD_GET_SLOW_PROTOCOL_LIMIT_10:
        ulResultLen = sizeof(ONU_RLT) + sizeof(bool);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            bool *pbVal = (bool*)(pRpcResult + 1);
        
            iRet = OnuMgt_GetSlowProtocolLimit( olt_id, onu_id, pbVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetSlowProtocolLimit(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *pbVal, iRet, ulSrcSlot);
        }   
        
        break;
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    case ONU_CMD_GET_ONU_BWINFO_11:
    {
    	typedef struct bwInfo
    	{
    	    PonPortBWInfo_S ponBW;
    		ONUBWInfo_S onuBW;
    	}bwInfo_S;
    	
    	bwInfo_S *realInfo = NULL;
    	ulResultLen = sizeof(ONU_RLT) + sizeof(bwInfo_S);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            realInfo = (bwInfo_S *)(pRpcResult + 1);
                            
            iRet = OnuMgt_GetBWInfo( olt_id, onu_id, &(realInfo->ponBW),&(realInfo->onuBW));
            
        }
    	break;
    }
    /*End:for onu swap by jinhl@2013-04-27*/

	case ONU_CMD_GET_B2P_MODE_12:
        ulResultLen = sizeof(ONU_RLT) + sizeof(int);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            int *piVal = (int*)(pRpcResult + 1);
        
            iRet = OnuMgt_GetOnuB2PMode( olt_id, onu_id, piVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuB2PMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *piVal, iRet, ulSrcSlot);
        }   
        
        break;
	case ONU_CMD_SET_B2P_MODE_13:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuB2PMode( olt_id, onu_id, pRpcCmd->params.iVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuB2PMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
        }   
        
        break;
#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */
    case ONU_CMD_RESET_COUNTER_1:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_ResetCounters( olt_id, onu_id );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ResetCounters(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }
        
        break;

    case ONU_CMD_SET_PON_LOOPBACK_2:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetPonLoopback( olt_id, onu_id, pRpcCmd->params.iVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPonLoopback(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal , iRet, ulSrcSlot);
        }
        
        break;
#endif

			
#if 1
/* -------------------OLT 加密管理API----------- */
    case ONU_CMD_GET_LLID_PARAMS_1:
        ulResultLen = sizeof(ONU_RLT) + sizeof(PON_llid_parameters_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetLLIDParams( olt_id, onu_id, pRpcResult + 1 );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetLLIDParams(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }
        
        break;
    case ONU_CMD_START_ENCRYPTION_2:
        ulResultLen = sizeof(ONU_RLT) + sizeof(int);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            int *pEncDir = pRpcResult + 1;

            *pEncDir = pRpcCmd->params.iVal;               
            iRet = OnuMgt_StartEncryption( olt_id, onu_id, pEncDir );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_StartEncryption(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
        }
        
        break;
    case ONU_CMD_STOP_ENCRYPTION_3:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_StopEncryption( olt_id, onu_id );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_StopEncryption(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }        
        
        break;
    case ONU_CMD_SET_ONU_ENCRYPT_PARAMS_4:
    {
        int *pEncDir;
        
        if ( 0 < pRpcCmd->params.iVal )
        {
            ulResultLen = sizeof(ONU_RLT) + sizeof(int);
            pEncDir = 1;
        }
        else
        {
            ulResultLen = sizeof(ONU_RLT);
            pEncDir = NULL;
        }
        
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            if ( NULL != pEncDir )
            {
                pEncDir  = pRpcResult + 1;
                *pEncDir = pRpcCmd->params.iVal;               
            }

            iRet = OnuMgt_SetOnuEncryptParams( olt_id, onu_id, pEncDir, pRpcCmd->params.iVals[1] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
        }
    }
    
    break;
    case ONU_CMD_GET_ONU_ENCRYPT_PARAMS_5:
        ulResultLen = sizeof(ONU_RLT) + sizeof(int) * 3;
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            int *pEncPara  = pRpcResult + 1;
            iRet = OnuMgt_GetOnuEncryptParams( olt_id, onu_id, &pEncPara[0], &pEncPara[1], &pEncPara[2] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuEncryptParams(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pEncPara[0], pEncPara[1], pEncPara[2], iRet, ulSrcSlot);
        }        
        
        break;
	/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	case ONU_CMD_UPDATE_ENCRYPTION_KEY_6:
		ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_UpdateEncryptionKey(olt_id, onu_id, &pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_UpdateEncryptionKey(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
    	}

    	break;
	/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU 地址表管理API-------- */
    case ONU_CMD_GET_ONU_MAC_ADDTBL_1:
    {
        LONG lNum = 0;
        int iIsOnlyNum;
        PON_onu_address_table_record_t *pMacAddrTbl;

        if ( 0 == (iIsOnlyNum = pRpcCmd->params.iVal) )
        {
            pMacAddrTbl = MAC_Address_Table;
        }
        else
        {
            pMacAddrTbl = NULL;
        }
        
        ulResultLen = sizeof(ONU_RLT);
        iRet = OnuMgt_GetOnuMacAddrTbl( olt_id, onu_id, &lNum, pMacAddrTbl );
        if ( (lNum > 0) && (pMacAddrTbl != NULL) )
        {
            ulResultLen += lNum * sizeof(PON_onu_address_table_record_t);
        }
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            if ( OLT_ERR_OK == iRet )
            {
                if ( (0 < (iRet = lNum)) && (pMacAddrTbl != NULL) )
                {
                    VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(ONU_RLT));
                }
            }
        }
        OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, lNum, iRet, ulSrcSlot);
    }   
    
    break;	
    case ONU_CMD_GET_OLT_MAC_ADDTBL_2:
    {
        short int lNum = 0;
        int iIsOnlyNum;
        PON_address_record_t *pMacAddrTbl;

        if ( 0 == (iIsOnlyNum = pRpcCmd->params.iVal) )
        {
            pMacAddrTbl = Mac_addr_table;
        }
        else
        {
            pMacAddrTbl = NULL;
        }
        
        ulResultLen = sizeof(ONU_RLT);
        iRet = OnuMgt_GetOltMacAddrTbl( olt_id, onu_id, &lNum, pMacAddrTbl );
        if ( (lNum > 0) && (pMacAddrTbl != NULL) )
        {
            ulResultLen += lNum * sizeof(PON_address_record_t);
        }
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            if ( OLT_ERR_OK == iRet )
            {
                if ( (0 < (iRet = lNum)) && (pMacAddrTbl != NULL) )
                {
                    VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(ONU_RLT));
                }
            }
        }
        OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, lNum, iRet, ulSrcSlot);
    }   
    
    break;	
    case ONU_CMD_SET_MAX_MAC_3:
        ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned int);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            unsigned int *puiValue = pRpcResult + 1;

            *puiValue = pRpcCmd->params.uiVals[1];
            iRet = OnuMgt_SetOnuMaxMac( olt_id, onu_id, (short int)(pRpcCmd->params.iVals[0]), puiValue );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuMaxMac(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.uiVals[1], iRet, ulSrcSlot);
        }   
        
        break;		
    case ONU_CMD_GET_ONU_UNI_CONFIG_4:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(PON_oam_uni_port_mac_configuration_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet =  OnuMgt_GetOnuUniMacCfg( olt_id, onu_id, pRpcResult + 1 );	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuUniMacCfg(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }
    
    break;	
    case ONU_CMD_GET_ONU_MAC_CHECK_FLAG_5:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(ULONG);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet =  OnuMgt_GetOnuMacCheckFlag( olt_id, onu_id, pRpcResult + 1 );	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuMacCheckFlag(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }
    
    break;	

    case ONU_CMD_GET_ONU_ALL_PORT_MAC_COUNTER_6:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(OnuEthPortCounter_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet =  OnuMgt_GetAllEthPortMacCounter( olt_id, onu_id, pRpcResult + 1 );                        
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetAllEthPortMacCounter(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }
    
    break;  

	case ONU_CMD_GET_OLT_MAC_VLAN_ADDTBL_7:
    {
        short int lNum = 0;
        int iIsOnlyNum;
        PON_address_vlan_record_t *pMacAddrTbl;

        if ( 0 == (iIsOnlyNum = pRpcCmd->params.iVal) )
        {
            pMacAddrTbl = Mac_addr_vlan_table;
        }
        else
        {
            pMacAddrTbl = NULL;
        }
        
        ulResultLen = sizeof(ONU_RLT);
        iRet = OnuMgt_GetOltMacAddrVlanTbl( olt_id, onu_id, &lNum, pMacAddrTbl );
        if ( (lNum > 0) && (pMacAddrTbl != NULL) )
        {
            ulResultLen += lNum * sizeof(PON_address_vlan_record_t);
        }
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            if ( OLT_ERR_OK == iRet )
            {
                if ( (0 < (iRet = lNum)) && (pMacAddrTbl != NULL) )
                {
                    VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(ONU_RLT));
                }
            }
        }
        OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOltMacAddrVlanTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, lNum, iRet, ulSrcSlot);
    }   
    
    break;	
    
#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
    case ONU_CMD_GET_ONU_DISTANCE_1:	
        ulResultLen = sizeof(ONU_RLT)+sizeof(int);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetOnuDistance( olt_id, onu_id, pRpcResult + 1 );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuDistance(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }        
        
        break;
    case ONU_CMD_GET_OPTICAL_CAPABILITY_2:	
        ulResultLen = sizeof(ONU_RLT)+sizeof(ONU_optical_capability_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetOpticalCapability( olt_id, onu_id, pRpcResult + 1 );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOpticalCapability(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }        
        
        break;
#endif


#if 1
/* -------------------ONU 倒换API---------------- */
	case ONU_CMD_SET_ONU_LLID_1:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_SetOnuLLID(olt_id, onu_id, pRpcCmd->params.sVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuLLID(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.sVal, iRet, ulSrcSlot);
            }   
        }
    
        break;	
#endif


#if 1
/* -------------------ONU 设备管理API------------------- */
    case ONU_CMD_GET_ONU_VER_1:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(PON_onu_versions);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetOnuVer(olt_id, onu_id, pRpcResult + 1);	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuVer(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }
    
    break;	
    case ONU_CMD_GET_PON_VER_2:
    {
        ulResultLen = sizeof(ONU_RLT)+sizeof(PON_device_versions_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetOnuPonVer( olt_id, onu_id, pRpcResult + 1 );	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuPonVer(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
    }
    
    break;	
    case  ONU_CMD_GET_ONU_INFO_3:
        ulResultLen = sizeof(ONU_RLT)+sizeof(onu_registration_info_t);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_GetOnuRegisterInfo( olt_id, onu_id, pRpcResult + 1 );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuRegisterInfo(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }   
        
        break;
    case ONU_CMD_GET_ONU_I2C_INFO_4:
        {
            int info_id = (int)pRpcCmd->params.ulVals[0];
            unsigned long buffer_size = pRpcCmd->params.ulVals[1];

            ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned long) + buffer_size;
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                unsigned long *size = pRpcResult + 1;
                void *data = size + 1;

                *size = buffer_size;
                iRet = OnuMgt_GetOnuI2CInfo( olt_id, onu_id, info_id, data, size );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRet, ulSrcSlot);
            }        
        }
    
        break;	
    case ONU_CMD_SET_ONU_I2C_INFO_5:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            int info_id = (int)pRpcCmd->params.ulVals[0];
            unsigned long size = pRpcCmd->params.ulVals[1];
            void *data = (void*)&pRpcCmd->params.ulVals[2];

            iRet = OnuMgt_SetOnuI2CInfo( olt_id, onu_id, info_id, data, size );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRet, ulSrcSlot);
        }        
        
        break;	

    case ONU_CMD_RESET_ONU_6:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_ResetOnu( olt_id, onu_id );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ResetOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }        
        
        break;	
    case ONU_CMD_SET_ONU_SW_UPDATE_MODE_7:
    {
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_SetOnuSWUpdateMode( olt_id, onu_id, pRpcCmd->params.iVal );	                    
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuSWUpdateMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
        }   
    }
    
    break;	
    case  ONU_CMD_ONU_SW_UPDATE_8:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_OnuSwUpdate( olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.uiVals[1] );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_OnuSwUpdate(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.uiVals[1], iRet, ulSrcSlot);
        }   
        
        break;
    case  ONU_CMD_GW_CTC_SW_CONVERT_9:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OnuMgt_OnuGwCtcSwConvert( olt_id, onu_id, pRpcCmd->params.acVal );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_OnuGwCtcSwConvert(%d, %d, %s)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.acVal, iRet, ulSrcSlot);
        }   
        
        break;
	/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	 case ONU_CMD_Get_Burn_Image_Complete_10:
        ulResultLen = sizeof(ONU_RLT) + sizeof(bool);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            
            iRet = OnuMgt_GetBurnImageComplete( olt_id, onu_id, pRpcResult + 1 );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuEncryptParams(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
        }        
        
        break;
	/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    case ONU_CMD_SET_ONU_DEVICE_NAME_11:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            char *pszStr;
            int iStrLen;

            pszStr  = pRpcCmd->params.acVal;
            iStrLen = ulRcvDataLen - sizeof(ONU_CMD_HEAD);
            iRet = OnuMgt_SetOnuDeviceName( olt_id, onu_id, pszStr, iStrLen );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuDeviceName(%d, %d, %s, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pszStr, iStrLen, iRet, ulSrcSlot);
        }        
        
        break;		
    case ONU_CMD_SET_ONU_DEVICE_DESC_12:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            char *pszStr;
            int iStrLen;

            pszStr  = pRpcCmd->params.acVal;
            iStrLen = ulRcvDataLen - sizeof(ONU_CMD_HEAD);
            iRet = OnuMgt_SetOnuDeviceDesc( olt_id, onu_id, pszStr, iStrLen );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuDeviceDesc(%d, %d, %s, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pszStr, iStrLen, iRet, ulSrcSlot);
        }        
        
        break;	
    case ONU_CMD_SET_ONU_DEVICE_LOCATION_13:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            char *pszStr;
            int iStrLen;

            pszStr  = pRpcCmd->params.acVal;
            iStrLen = ulRcvDataLen - sizeof(ONU_CMD_HEAD);
            iRet = OnuMgt_SetOnuDeviceLocation( olt_id, onu_id, pszStr, iStrLen );
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuDeviceLocation(%d, %d, %s, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pszStr, iStrLen, iRet, ulSrcSlot);
        }        
        
        break;	
            
         case ONU_CMD_GET_ONU_ALL_PORT_STATISTIC_DATA_14:
            {
                ulResultLen = sizeof(ONU_RLT) + sizeof(OnuStatisticData_S) ;
                if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
                {
                    iRet = OnuMgt_GetOnuAllPortStatisticData( olt_id, onu_id, pRpcResult+1);
                    OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuAllPortStatisticData(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
                }
            }
            break;
            
#endif


#if 1
/* -------------------ONU CTC-PROTOCOL--------------- */
     case ONU_CMD_CTC_GET_CTC_VERSION_1:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *pcVer = (unsigned char *)(pRpcResult + 1);
         
             iRet = OnuMgt_GetCtcVersion(olt_id, onu_id, pcVer);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCtcVersion(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *pcVer, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_FIRMWARE_VERSION_2:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned short);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned short *psVer = (unsigned short *)(pRpcResult + 1);
         
             iRet = OnuMgt_GetFirmwareVersion(olt_id, onu_id, psVer);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetFirmwareVersion(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *psVer, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_SERIAL_NUMBER_3:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_onu_serial_number_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetSerialNumber(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetSerialNumber(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_CHIPSET_ID_4:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_chipset_id_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetChipsetID(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetChipsetID(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }

         break;
         
     case ONU_CMD_CTC_GET_ONU_CAP1_5:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_onu_capabilities_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetOnuCap1(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuCap1(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_ONU_CAP2_6:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_onu_capabilities_2_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetOnuCap2(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuCap2(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_ONU_CAP3_7:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_onu_capabilities_3_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetOnuCap3(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuCap3(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }

         break;

    case ONU_CMD_CTC_START_ENCRYPTION_11:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_StartEncrypt(olt_id, onu_id);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_StartEncrypt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_STOP_ENCRYPTION_12:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_StopEncrypt(olt_id, onu_id);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_StopEncrypt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
    	}
    	break;

    case ONU_CMD_CTC_GET_ETHPORT_LINKSTATE_13:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetEthPortLinkState(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortLinkState(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
    case ONU_CMD_CTC_GET_ETHPORT_ADMIN_STATUS_14:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortAdminStatus(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortAdminStatus(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_ADMIN_STATUS_15:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortAdminStatus(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortAdminStatus(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;

    case ONU_CMD_CTC_GET_ETHPORT_PAUSE_16:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortPause(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortPause(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_PAUSE_17:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortPause(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortPause(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
        
     case ONU_CMD_CTC_GET_ETHPORT_AUTO_NEGOTIATION_ADMIN_18:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetEthPortAutoNegotiationAdmin(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortAutoNegotiationAdmin(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_ADMIN_19:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetEthPortAutoNegotiationAdmin(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortAutoNegotiationAdmin(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_RESTART_20:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetEthPortAutoNegotiationRestart(olt_id, onu_id, pRpcCmd->params.iVal);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortAutoNegotiationRestart(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_GET_ETHPORT_AN_LOCAL_TECABILITY_21:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_auto_negotiation_technology_ability_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetEthPortAnLocalTecAbility(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortAnLocalTecAbility(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_GET_ETHPORT_AN_ADVERTISED_TECABILITY_22:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_auto_negotiation_technology_ability_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetEthPortAnAdvertisedTecAbility(olt_id, onu_id, pRpcCmd->params.iVal,pRpcResult+1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortAnAdvertisedTecAbility(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
         }

         break;

    case ONU_CMD_CTC_GET_ETHPORT_POLICING_23:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_ethernet_port_policing_entry_t);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortPolicing(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuEthPortPolicing(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_POLICING_24:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortPolicing(olt_id, onu_id, pRpcCmd->params.iValPair[0], (CTC_STACK_ethernet_port_policing_entry_t*)&pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuEthPortPolicing(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_GET_ETHPORT_DOWNSTREAM_POLICING_25:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_ethernet_port_ds_rate_limiting_entry_t);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortDownstreamPolicing(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_DOWNSTREAM_POLICING_26:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortDownstreamPolicing(olt_id, onu_id, pRpcCmd->params.iValPair[0], (CTC_STACK_ethernet_port_ds_rate_limiting_entry_t*)&pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;

    case ONU_CMD_CTC_GET_ETHPORT_VCONF_27:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_port_vlan_configuration_t);
    	if(debug_rpc) sys_console_printf("ulResultLen = %d ,address = 0x%0x\r\n", ulResultLen, &ulResultLen);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortVlanConfig(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthportVlanConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_VCONF_28:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortVlanConfig(olt_id, onu_id, pRpcCmd->params.iValPair[0], (CTC_STACK_port_vlan_configuration_t*)&pRpcCmd->params.iValPair[1]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
        
    case ONU_CMD_CTC_GET_ETHPORT_CLASSIFICATIOANDMARK_30:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_classification_rules_t);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortClassificationAndMarking(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_CLASSIFICATIOANDMARK_31:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortClassificationAndMarking(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], &pRpcCmd->params.iVals[2]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_CLR_ETHPORT_CLASSIFICATIOANDMARK_32:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_ClearEthPortClassificationAndMarking(olt_id, onu_id, pRpcCmd->params.iVals[0]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;

    case ONU_CMD_CTC_GET_ETHPORT_MULTICAST_VLAN_33:
    	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_multicast_vlan_t);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_GetEthPortMulticastVlan(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_SET_ETHPORT_MULTICAST_VLAN_34:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_SetEthPortMulticastVlan(olt_id, onu_id, pRpcCmd->params.iVals[0], ((CTC_STACK_multicast_vlan_t*)&pRpcCmd->params.iVals[1]));
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;
    case ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_VLAN_35:
    	ulResultLen = sizeof(ONU_RLT);
    	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	{
    		iRet = OnuMgt_ClearEthPortMulticastVlan(olt_id, onu_id, pRpcCmd->params.iVals[0]);
    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClearEthPortMulticastVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
    	}

    	break;

     case ONU_CMD_CTC_GET_ETHPORT_MULTICAST_MAX_GROUP_36:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetEthPortMulticastGroupMaxNumber(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_ETHPORT_MULTICAST_MAX_GROUP_37:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetEthPortMulticastGroupMaxNumber(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;

     case ONU_CMD_CTC_GET_ETHPORT_MULTICAST_TAG_STRIP_38:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetEthPortMulticastTagStrip(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_ETHPORT_MULTICAST_TAG_STRIP_39:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetEthPortMulticastTagStrip(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
     	}

     	break;
        
     case ONU_CMD_CTC_GET_ETHPORT_IGMP_TAG_OPER_41:
    	 ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_multicast_vlan_switching_t)+sizeof(CTC_STACK_tag_oper_t);
    	 if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	 {
    		 CTC_STACK_tag_oper_t * pOper = (CTC_STACK_tag_oper_t *)(pRpcResult+1);
    		 CTC_STACK_multicast_vlan_switching_t *psw = (CTC_STACK_multicast_vlan_switching_t*)(pOper+1);

    		 iRet = OnuMgt_GetEthPortMulticastTagOper(olt_id, onu_id, pRpcCmd->params.iVal, pOper, psw );
    		 OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetEthPortMulticastTagOper(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
    	 }
    	 break;
     case ONU_CMD_CTC_SET_ETHPORT_IGMP_TAG_OPER_42:
    	 ulResultLen = sizeof(ONU_RLT);
    	 if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
    	 {
    		 iRet = OnuMgt_SetEthPortMulticastTagOper(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], (CTC_STACK_multicast_vlan_switching_t*)&pRpcCmd->params.iVals[2] );
    		 OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortMulticastTagOper(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
    	 }
    	 break;

    case ONU_CMD_CTC_GET_ETHPORT_MULTICAST_CONTROL_44:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_multicast_control_t);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetMulticastControl(olt_id, onu_id, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetMulticastControl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_ETHPORT_MULTICAST_CONTROL_45:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetMulticastControl(olt_id, onu_id, (CTC_STACK_multicast_control_t*)&pRpcCmd->params.iVals[0]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMulticastControl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_CONTROL_46:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_ClearMulticastControl(olt_id, onu_id);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClearMulticastControl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}

     	break;

     case ONU_CMD_CTC_GET_MULTICAST_SWITCH_47:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_multicast_protocol_t);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetMulticastSwitch(olt_id, onu_id, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuMulticastSwitch(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_MULTICAST_SWITCH_48:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetMulticastSwitch(olt_id, onu_id, pRpcCmd->params.iValPair[0]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuMulticastSwitch(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}

     	break;

     case ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_ABILITY_49:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_fast_leave_ability_t);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetMulticastFastleaveAbility(olt_id, onu_id, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetMulticastFastleaveAbility(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_50:
     	ulResultLen = sizeof(ONU_RLT)+sizeof(int);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetMulticastFastleave(olt_id, onu_id, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuMulticastFastleave(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id,  iRet, ulSrcSlot);
     	}

     	break;
     case ONU_CMD_CTC_SET_MULTICAST_FASTLEAVE_51:
     	ulResultLen = sizeof(ONU_RLT);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_SetMulticastFastleave(olt_id, onu_id, pRpcCmd->params.iValPair[0]);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuMulticastFastleave(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}

     	break;
         
     case ONU_CMD_CTC_GET_PORT_STATISTIC_DATA_52:
        {
            int port_number = (unsigned char)pRpcCmd->params.iVal;

            ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_statistic_data_t) ;
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_GetOnuPortStatisticData( olt_id, onu_id, port_number, pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuPortStatisticData(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, port_number, iRet, ulSrcSlot);
            }
        }
        break;
     case ONU_CMD_CTC_GET_PORT_STATISTIC_STATE_53:
        {
            int port_number = (unsigned char)pRpcCmd->params.iVal;

            ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_statistic_state_t) ;
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_GetOnuPortStatisticState( olt_id, onu_id, port_number, pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuPortStatisticState(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, port_number, iRet, ulSrcSlot);
            }        
        }

        break;	
     case ONU_CMD_CTC_SET_PORT_STATISTIC_STATE_54:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_SetOnuPortStatisticState( olt_id, onu_id, pRpcCmd->params.iVal, &pRpcCmd->params.aucVal[4]);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuPortStatisticState(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }        
        }

        break;	
     case ONU_CMD_CTC_SET_ALARM_ADMIN_STATE_55:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                CTC_STACK_alarm_id_t *piAlarmId = (CTC_STACK_alarm_id_t*)&pRpcCmd->params.aucVal[sizeof(bool)];
                    
                iRet = OnuMgt_SetAlarmAdminState( olt_id, onu_id, &pRpcCmd->params.aucVal[sizeof(bool)+sizeof(CTC_STACK_alarm_id_t)], *piAlarmId, pRpcCmd->params.bVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetAlarmAdminState(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *piAlarmId, iRet, ulSrcSlot);
            }        
        }

        break;	
     case ONU_CMD_CTC_SET_ALARM_THRESOLD_56:
        {
            ulResultLen = sizeof(ONU_RLT);
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                CTC_STACK_alarm_id_t *piAlarmId = (CTC_STACK_alarm_id_t*)&pRpcCmd->params.aucVal[sizeof(unsigned long)*2];
                    
                iRet = OnuMgt_SetAlarmThreshold( olt_id, onu_id, &pRpcCmd->params.aucVal[sizeof(unsigned long)*2+sizeof(CTC_STACK_alarm_id_t)], *piAlarmId, pRpcCmd->params.ulValPair[0], pRpcCmd->params.ulValPair[1]);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetAlarmThreshold(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *piAlarmId, iRet, ulSrcSlot);
            }        
        }

        break;	

     case ONU_CMD_CTC_GET_MNG_GLOBAL_CONFIG_59:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_mxu_mng_global_parameter_config_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetMxuMngGlobalConfig(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_CTC_SET_MNG_GLOBAL_CONFIG_60:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMxuMngGlobalConfig(olt_id, onu_id, &pRpcCmd->params);
         }
         break;
     case ONU_CMD_CTC_GET_MNG_SNMP_CONFIG_61:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_mxu_mng_snmp_parameter_config_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetMxuMngSnmpConfig(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_CTC_SET_MNG_SNMP_CONFIG_62:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMxuMngSnmpConfig(olt_id, onu_id, &pRpcCmd->params);
         }
         break;

     case ONU_CMD_CTC_GET_HOLDOVER_63:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_holdover_state_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             CTC_STACK_holdover_state_t *pHoldOver = pRpcResult + 1;
         
             iRet = OnuMgt_GetHoldOver(olt_id, onu_id, pHoldOver);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetHoldOver(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pHoldOver->holdover_state, pHoldOver->holdover_time, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_CTC_SET_HOLDOVER_64:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             CTC_STACK_holdover_state_t *pHoldOver = pRpcCmd->params.aucVal;

             iRet = OnuMgt_SetHoldOver(olt_id, onu_id, pHoldOver);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetHoldOver(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pHoldOver->holdover_state, pHoldOver->holdover_time, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_CTC_GET_OPTIC_TRAN_DIAG_65:
         ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_STACK_optical_transceiver_diagnosis_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetOptTransDiag(olt_id, onu_id, pRpcResult + 1);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOptTransDiag(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_CTC_SET_ONU_POWER_SUPPLY_CONTROL_66:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             CTC_STACK_onu_tx_power_supply_control_t *pv = pRpcCmd->params.aucVal;
             iRet = OnuMgt_SetTxPowerSupplyControl(olt_id, onu_id, pv);
         }
         break;

	case ONU_CMD_CTC_GET_ONU_FEC_ABILITY_67:
        ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_standard_FEC_ability_t);
     	if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
     	{
     		iRet = OnuMgt_GetFecAbility(olt_id, onu_id, pRpcResult+1);
     		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetFecAbility(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
     	}
		break;

#if 1
     case ONU_CMD_GET_IADINFO_68:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_voip_iad_info_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetIADInfo(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_GET_VOIP_IAD_OPER_69:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_voip_iad_oper_status_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipIadOperStatus(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_VOIP_IAD_OPER_70:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetVoipIadOperation(olt_id, onu_id, pRpcCmd->params.iVal);
         }        
         break;
     case ONU_CMD_GET_VOIP_GLOBAL_CONFIG_71:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_voip_global_param_conf_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipGlobalConfig(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_VOIP_GLOBAL_CONFIG_72:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_voip_global_param_conf_t *pv = (CTC_STACK_voip_global_param_conf_t *)(p+sizeof(int));            
             iRet = OnuMgt_SetVoipGlobalConfig(olt_id, onu_id, pRpcCmd->params.iVals[0], pv);
         }        
         break;
     case ONU_CMD_GET_VOIP_FAX_CONFIG_73:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_voip_fax_config_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipFaxConfig(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_VOIP_FAX_CONFIG_74:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_voip_fax_config_t *pv = (CTC_STACK_voip_fax_config_t *)(p+sizeof(int));            
             iRet = OnuMgt_SetVoipFaxConfig(olt_id, onu_id, pRpcCmd->params.iVals[0], pv);
         }        
         break;

     case ONU_CMD_GET_VOIP_POTS_STATUS_75:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_voip_pots_status_array);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipPortStatus(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
         }
         break; 
     case ONU_CMD_GET_VOIP_PORT_OPER_76:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_on_off_state_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipPort(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_VOIP_PORT_OPER_77:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetVoipPort(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1]);             
         }        
         break;
     case ONU_CMD_GET_VOIP_PORT2_OPER_78:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_on_off_state_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetVoipPort2(olt_id, onu_id, pRpcCmd->params.iValPair[0], pRpcCmd->params.iValPair[1], pRpcResult+1);
         }
         break;
     case ONU_CMD_SET_VOIP_PORT2_OPER_79:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetVoipPort2(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
         }        
         break; 
        
     case ONU_CMD_GET_H248_CONFIG_80:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_h248_param_config_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetH248Config(olt_id, onu_id, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_H248_CONFIG_81:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_h248_param_config_t *pv = (CTC_STACK_h248_param_config_t *)(p+sizeof(int));
             iRet = OnuMgt_SetH248Config(olt_id, onu_id, pRpcCmd->params.iVals[0], pv);
         }        
         break;
     case ONU_CMD_GET_H248_USERTID_CONFIG_82:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_h248_user_tid_config_array);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetH248UserTidConfig(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_H248_USERTID_CONFIG_83:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_h248_user_tid_config_t *pv = (CTC_STACK_h248_user_tid_config_t *)(p+sizeof(int)*2);            
             iRet = OnuMgt_SetH248UserTidConfig(olt_id, onu_id, pRpcCmd->params.iVal, pv);
         }        
         break;
     case ONU_CMD_GET_H248_RTPTID_CONFIG_84:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_h248_rtp_tid_info_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetH248RtpTidConfig(olt_id, onu_id, pRpcResult+1); 
         }
         break; 
     case ONU_CMD_SET_H248_RTPTID_CONFIG_85:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_h248_rtp_tid_config_t *pv = (CTC_STACK_h248_rtp_tid_config_t *)p;
             iRet = OnuMgt_SetH248RtpTidConfig(olt_id, onu_id, pv);
         }        
         break;

     case ONU_CMD_GET_SIP_CONFIG_86:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_sip_param_config_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetSipConfig(olt_id, onu_id, pRpcResult+1);  
         }
         break; 
     case ONU_CMD_SET_SIP_CONFIG_87:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_sip_param_config_t *pv = (CTC_STACK_sip_param_config_t *)(p+sizeof(int));
             iRet = OnuMgt_SetSipConfig(olt_id, onu_id, pRpcCmd->params.iValPair[0], pv);
         }        
         break;
     case ONU_CMD_SET_SIP_DIGITMAP_88:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_SIP_digit_map_t pv;
             
             pv.digital_map_length = pRpcCmd->params.ulVal;
             pv.digital_map = (unsigned char *)(p+sizeof(int));

             iRet = OnuMgt_SetSipDigitMap(olt_id, onu_id, &pv);
         }        
         break;
     case ONU_CMD_GET_SIP_USERTID_CONFIG_89:
         ulResultLen = sizeof(ONU_RLT)+sizeof(CTC_STACK_sip_user_param_config_array);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetSipUserConfig(olt_id, onu_id, pRpcCmd->params.iVal, pRpcResult+1);
         }
         break; 
     case ONU_CMD_SET_SIP_USERTID_CONFIG_90:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             CTC_STACK_sip_user_param_config_t *pv = (CTC_STACK_sip_user_param_config_t *)(p+2*sizeof(int));
             iRet = OnuMgt_SetSipUserConfig(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pv);
         }        
         break;
	 case ONU_CMD_CTC_GET_PORT_STATS_DATA_91:
	 	{
            int port_number = (unsigned char)pRpcCmd->params.iVal;

            ulResultLen = sizeof(ONU_RLT) + sizeof(OnuPortStats_ST) ;
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OnuMgt_GetCTCOnuPortStatsData( olt_id, onu_id, port_number, pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCTCOnuPortStatsData(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, port_number, iRet, ulSrcSlot);
            }
        }
        break;

#endif
#endif


#if 1
/* -------------------ONU 远程管理API------------------- */
     case ONU_CMD_CLI_CALL_1:
         {
             int iStrLen;
             USHORT len = 0;
             char *pszStr;
             char *pRecv = NULL;

             pszStr  = pRpcCmd->params.acVal;
             iStrLen = ulRcvDataLen - sizeof(ONU_CMD_HEAD);
             
             iRet = OnuMgt_CliCall(olt_id, onu_id, pszStr, iStrLen, &pRecv, &len);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CliCall(%d, %d)[%s]'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.acVal, iRet, ulSrcSlot);
             ulResultLen = sizeof(ONU_RLT) + len;
             if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
             {
                 if ( 0 == iRet )
                 {
                     if ( (NULL != pRecv) && (len > 0) )
                     {
    					 VOS_MemCpy(pRpcResult->values, pRecv, len);
                     }
                 }
             }
         }
         break;
     case ONU_CMD_SET_MGT_RESET_2:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMgtReset(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMgtReset(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_SET_MGT_CONFIG_3:

         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMgtConfig(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMgtConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_SET_MGT_LASER_4:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMgtLaser(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMgtLaser(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }

         break;
     case ONU_CMD_SET_TEMPERATURE_5:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetTemperature(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetTemperature(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PAS_FLUSH_6:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPasFlush(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPasFlush(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_ATU_AGING_TIME_7:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetAtuAgingTime(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetAtuAgingTime(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_ATU_LIMIT_8:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetAtuLimit(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetAtuLimit(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_PORT_LINK_MON_9:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortLinkMon(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortLinkMon(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_MODE_MON_10:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortModeMon(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortModeMon(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_PORT_ISOLATE_11:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortIsolate(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortIsolate(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_VLAN_ENABLE_12:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetVlanEnable(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetVlanEnable(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_VLAN_MODE_13:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetVlanMode(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetVlanMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_ADD_VLAN_14:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddVlan(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_DEL_VLAN_15:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DelVlan(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_PVID_16:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortPvid(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortPvid(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_ADD_VLAN_PORT_17:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddVlanPort(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddVlanPort(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_DEL_VLAN_PORT_18:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DelVlanPort(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelVlanPort(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
    case ONU_CMD_SET_VLAN_TRANSATION_19:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetEthPortVlanTran(olt_id, onu_id, pRpcCmd->params.iVals[0],pRpcCmd->params.ulVals[1], pRpcCmd->params.ulVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortVlanTran(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
      case ONU_CMD_DEL_VLAN_TRANSATION_20:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DelEthPortVlanTran(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.ulVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelEthPortVlanTran(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
       case ONU_CMD_SET_VLAN_AGGREGATION_21:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetEthPortVlanAgg(olt_id, onu_id,(int)pRpcCmd->params.usVals[0], &pRpcCmd->params.usVals[1], pRpcCmd->params.usVals[9]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetEthPortVlanAgg(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break;
        case ONU_CMD_DEL_VLAN_AGGREGATION_22:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DelEthPortVlanAgg(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.ulVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelEthPortVlanAgg(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_QINQ_ENABLE_23:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortQinQEnable(olt_id, onu_id, pRpcCmd->params.iVals[0],pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortQinQEnable(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_ADD_QINQ_TAG_24:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddQinQVlanTag(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.ulVals[1], pRpcCmd->params.ulVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddQinQVlanTag(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_DEL_QINQ_TAG_25:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DelQinQVlanTag(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.ulVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DelQinQVlanTag(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_PORT_VLAN_FRAME_ACC_26:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortVlanFrameTypeAcc(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortVlanFrameTypeAcc(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_INGRESS_VLAN_FILTER_27:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortIngressVlanFilter(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortIngressVlanFilter(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;


     case ONU_CMD_SET_PORT_MODE_28:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortMode(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_FC_MODE_29:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortFcMode(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortFcMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_ATU_LEARN_30:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortAtuLearn(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortAtuLearn(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_ATU_FLOOD_31:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortAtuFlood(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortAtuFlood(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_LOOP_DETECT_32:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortLoopDetect(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortLoopDetect(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_STAT_FLUSH_33:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortStatFlush(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortStatFlush(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_INGRESS_RATE_LIMIT_BASE_34:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIngressRateLimitBase(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIngressRateLimitBase(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_INGRESS_RATE_35:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortIngressRate(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[4]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortIngressRate(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_EGRESS_RATE_36:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortEgressRate(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortEgressRate(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_QOS_CLASS_37:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetQosClass(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[4], &pRpcCmd->params.iVals[6], pRpcCmd->params.iVals[5]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetQosClass(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_CLR_QOS_CLASS_38:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ClrQosClass(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClrQosClass(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_QOS_RULE_39:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetQosRule(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetQosRule(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_CLR_QOS_RULE_40:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ClrQosRule(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClrQosRule(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_PORT_QOS_RULE_41:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortQosRule(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortQosRule(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_CLR_PORT_QOS_RULE_42:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ClrPortQosRule(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClrPortQosRule(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_QOS_RULE_TYPE_43:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortQosRuleType(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortQosRuleType(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
         
     case ONU_CMD_SET_PORT_DEF_PRIORITY_44:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortDefPriority(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortDefPriority(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_NEW_PRIORITY_45:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortNewPriority(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortNewPriority(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_SET_QOS_PRIORITY_TO_QUEUE_46:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetQosPrioToQueue(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetQosPrioToQueue(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_SET_QOS_DSCP_TO_QUEUE_47:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetQosDscpToQueue(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetQosDscpToQueue(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_SET_PORT_QOS_USERPRI_ENABLE_48:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortUserPriorityEnable(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortUserPriorityEnable(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_SET_PORT_QOS_IPPRI_ENABLE_49:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortIpPriorityEnable(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortIpPriorityEnable(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_SET_QOS_ALGORITHM_50:
        ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetQosAlgorithm(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetQosAlgorithm(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_QOS_MODE_51:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = GWONU_SET_QosMode(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"GWONU_SET_QosMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
        break;
     case ONU_CMD_SET_RULE_52:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *p = (char*)&pRpcCmd->params;
             gw_rule_t *rule = (gw_rule_t *)(p+sizeof(int)*2);
             iRet = GWONU_SET_Rule(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], *rule);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"GWONU_SET_Rule(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
        break;

     case ONU_CMD_SET_IGMP_ENABLE_53:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpEnable(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpEnable(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_IGMP_AUTH_54:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpAuth(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpAuth(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_IGMP_HOST_AGE_55:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpHostAge(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpHostAge(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_IGMP_GROUP_AGE_56:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpGroupAge(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpGroupAge(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_IGMP_MAX_RESTIME_57:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpMaxResTime(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpMaxResTime(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;

     case ONU_CMD_SET_IGMP_MAX_GROUP_58:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetIgmpMaxGroup(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetIgmpMaxGroup(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_ADD_IGMP_GROUP_59:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddIgmpGroup(olt_id, onu_id, pRpcCmd->params.iVal, pRpcCmd->params.ulVals[1], pRpcCmd->params.ulVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddIgmpGroup(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_DEL_IGMP_GROUP_60:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DeleteIgmpGroup(olt_id, onu_id, pRpcCmd->params.ulVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DeleteIgmpGroup(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_MULTICAST_FASTLEAVE_61:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortIgmpFastLeave(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortIgmpFastLeave(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
     case ONU_CMD_SET_PORT_MULTICAST_VLAN_62:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortMulticastVlan(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortMulticastVlan(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
         
     case ONU_CMD_SET_PORT_MIRROR_FROM_63:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortMirrorFrom(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortMirrorFrom(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;    
     case ONU_CMD_SET_PORT_MIRROR_TO_64:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetPortMirrorTo(olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetPortMirrorTo(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_DEL_MIRROR_65:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DeleteMirror(olt_id, onu_id, pRpcCmd->params.iVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DeleteMirror(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break;
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC协议管理API------------------- */
     case ONU_CMD_CMC_REGISTER_CMC_1:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_RegisterCmc(olt_id, onu_id, pRpcCmd->params.aucVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_RegisterCmc(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_UNREGISTER_CMC_2:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_UnregisterCmc(olt_id, onu_id, pRpcCmd->params.aucVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_UnregisterCmc(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_3:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmc( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmc(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_ALARM_4:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcAlarms( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcAlarms(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_LOG_5:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLogs( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLogs(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_RESET_CMC_BOARD_6:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ResetCmcBoard(olt_id, onu_id, pRpcCmd->params.aucVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ResetCmcBoard(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_VERSION_7:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.ucVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned char buf_len = pRpcCmd->params.ucVal;
            
             if ( 0 == (iRet = OnuMgt_GetCmcVersion( olt_id, onu_id, &pRpcCmd->params.aucVal[1], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcVersion(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_MAXMULTICAST_8:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned short);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned short *max_multicasts = pRpcResult + 1;
            
             iRet = OnuMgt_GetCmcMaxMulticasts( olt_id, onu_id, pRpcCmd->params.aucVal, max_multicasts );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcMaxMulticasts(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *max_multicasts, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_MAXCM_9:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned short);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned short *max_cm = pRpcResult + 1;
            
             iRet = OnuMgt_GetCmcMaxCm( olt_id, onu_id, pRpcCmd->params.aucVal, max_cm );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcMaxCm(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, *max_cm, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_MAXCM_10:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcMaxCm( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.usVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcMaxCm(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break; 

     case ONU_CMD_CMC_GET_CMC_TIME_11:
         ulResultLen = sizeof(ONU_RLT) + sizeof(struct tm);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetCmcTime( olt_id, onu_id, pRpcCmd->params.aucVal, pRpcResult + 1 );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_TIME_12:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcTime( olt_id, onu_id, &pRpcCmd->params.aucVal[sizeof(struct tm)], &pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_LOCAL_CMC_TIME_13:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_LocalCmcTime( olt_id, onu_id, pRpcCmd->params.aucVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_LocalCmcTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_CUSTOM_CONFIG_14:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcCustomConfig( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], &pRpcCmd->params.aucVal[9], pRpcCmd->params.usVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcCustomConfig(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_CUSTOM_CONFIG_15:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcCustomConfig( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcDownChannel(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_16:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcDownChannel( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcDownChannel(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_17:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcUpChannel( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcUpChannel(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODE_18:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelMode( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODE_19:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelMode( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_MODE_20:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelMode( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_MODE_21:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelMode( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_D30MODE_22:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelD30Mode( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_D30MODE_23:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelD30Mode( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_FREQ_24:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned long) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned long *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_FREQ_25:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[5], pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 

     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_FREQ_26:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned long) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned long *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_FREQ_27:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[5], pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_AUTO_FREQ_29:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownAutoFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[9], pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1], pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1], pRpcCmd->params.aucVal[8], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_SET_CMC_UP_AUTO_FREQ_31:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpAutoFreq( olt_id, onu_id, &pRpcCmd->params.aucVal[9], pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1], pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1], pRpcCmd->params.aucVal[8], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_WIDTH_32:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned long) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned long *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelWidth( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_WIDTH_33:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelWidth( olt_id, onu_id, &pRpcCmd->params.aucVal[5], pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_ANNEXMODE_34:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelAnnexMode( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_ANNEXMODE_35:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelAnnexMode( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_TYPE_36:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelType( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_TYPE_37:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelType( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODULATION_38:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelModulation( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODULATION_39:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelModulation( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_PROFILE_40:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelProfile( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_PROFILE_41:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelProfile( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_INTERLEAVER_42:
         ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             unsigned char *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelInterleaver( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_INTERLEAVER_43:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelInterleaver( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_POWER_44:
         ulResultLen = sizeof(ONU_RLT) + sizeof(short int) * CMC_MAX_DS_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             short int *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcDownChannelPower( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_POWER_45:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcDownChannelPower( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], pRpcCmd->params.sVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], pRpcCmd->params.sVal, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_GET_CMC_UP_CHANNEL_POWER_46:
         ulResultLen = sizeof(ONU_RLT) + sizeof(short int) * CMC_MAX_US_CH;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             short int *buf = pRpcResult + 1;
             
             iRet = OnuMgt_GetCmcUpChannelPower( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], buf );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], *buf, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_CMC_UP_CHANNEL_POWER_47:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcUpChannelPower( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], pRpcCmd->params.sVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], pRpcCmd->params.sVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_POWER_48:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcUpChannelPower( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_SIGNAL_QUALITY_49:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcUpChannelSignalQuality( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcUpChannelSignalQuality(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_UTILITY_50:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcInterfaceUtilization( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcInterfaceUtilization(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_STATISTICS_51:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcInterfaceStatistics( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcInterfaceStatistics(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_IF_MAC_STATISTICS_52:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcMacStatistics( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcMacStatistics(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_IF_ALL_STATISTICS_53:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcAllInterface( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcAllInterface(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_GROUP_ALL_54:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcAllLoadBalancingGrp( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcAllLoadBalancingGrp(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_GROUP_55:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrp( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 

     case ONU_CMD_CMC_DUMP_GROUP_DOWN_CHANNEL_56:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpDownstream( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_GROUP_UP_CHANNEL_57:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpUpstream( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_GROUP_DYNAMIC_CONFIG_58:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingDynConfig( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingDynConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_METHOD_59:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynMethod( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynMethod(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_PERIOD_60:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynPeriod( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynPeriod(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_WEIGHT_PERIOD_61:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_OVERLOAD_THRESOLD_62:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynOverloadThresold( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynOverloadThresold(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_DIFF_THRESOLD_63:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynDifferenceThresold( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_MAX_MOVE_64:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_MIN_HOLD_65:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynMinHoldTime( olt_id, onu_id, &pRpcCmd->params.aucVal[4], pRpcCmd->params.ulVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynMinHoldTime(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_RANGE_MODE_66:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DCC_TECH_67:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DCC_TECH_68:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DBC_TECH_69:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DBC_TECH_70:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_CREATE_GROUP_71:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_CreateCmcLoadBalancingGrp( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CreateCmcLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DESTROY_GROUP_72:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DestroyCmcLoadBalancingGrp( olt_id, onu_id, &pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DestroyCmcLoadBalancingGrp(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_ADD_GROUP_DOWN_CHANNEL_73:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddCmcLoadBalancingGrpDownstream( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddCmcLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_RMV_GROUP_DOWN_CHANNEL_74:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_RemoveCmcLoadBalancingGrpDownstream( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_RemoveCmcLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_ADD_GROUP_UP_CHANNEL_75:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddCmcLoadBalancingGrpUpstream( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddCmcLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_RMV_GROUP_UP_CHANNEL_76:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_RemoveCmcLoadBalancingGrpUpstream( olt_id, onu_id, &pRpcCmd->params.aucVal[2], pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[8] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_RemoveCmcLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], pRpcCmd->params.aucVal[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_ADD_GROUP_MODEM_77:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddCmcLoadBalancingGrpModem( olt_id, onu_id, &pRpcCmd->params.aucVal[13], pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[7] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddCmcLoadBalancingGrpModem(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_RMV_GROUP_MODEM_78:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_RemoveCmcLoadBalancingGrpModem( olt_id, onu_id, &pRpcCmd->params.aucVal[13], pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[1], &pRpcCmd->params.aucVal[7] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_RemoveCmcLoadBalancingGrpModem(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_ADD_GROUP_EXCLUDE_MODEM_79:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_AddCmcLoadBalancingGrpExcludeModem( olt_id, onu_id, &pRpcCmd->params.aucVal[12], &pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[6] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_AddCmcLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_RMV_GROUP_EXCLUDE_MODEM_80:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem( olt_id, onu_id, &pRpcCmd->params.aucVal[12], &pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[6] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_DUMP_GROUP_MODEM_81:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpModem( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpModem(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_GROUP_ACTIVE_MODEM_82:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpActivedModem( olt_id, onu_id, &pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpActivedModem(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_MODEM_83:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpExcludeModem( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpExcludeModem(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_ACTIVE_MODEM_84:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_RSV_GROUP_85:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
            iRet = OLT_ERR_NOTSUPPORT;
         }
         break; 
         
     case ONU_CMD_CMC_DUMP_ALL_MODEM_86:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpAllCm( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpAllCm(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_87:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCm( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCm(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_ALL_MODEM_HISTORY_88:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpAllCmHistory( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpAllCmHistory(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_HISTORY_89:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmHistory( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmHistory(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_CLEAR_ALL_MODEM_HISTORY_90:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ClearAllCmHistory( olt_id, onu_id, &pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ClearAllCmHistory(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_RESET_MODEM_91:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ResetCm( olt_id, onu_id, &pRpcCmd->params.aucVal[6], &pRpcCmd->params.aucVal[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ResetCm(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_DOWN_CHANNEL_92:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmDownstream( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmDownstream(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_UP_CHANNEL_93:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmUpstream( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmUpstream(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_MODEM_DOWN_CHANNEL_94:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmDownstream( olt_id, onu_id, &pRpcCmd->params.aucVal[7], pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[14] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmDownstream(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_SET_MODEM_UP_CHANNEL_95:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetCmUpstream( olt_id, onu_id, &pRpcCmd->params.aucVal[7], pRpcCmd->params.aucVal[1], pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[14] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetCmUpstream(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[0], iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_CREATE_MODEM_SERVICE_FLOW_96:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_CreateCmServiceFlow( olt_id, onu_id, &pRpcCmd->params.aucVal[9], pRpcCmd->params.aucVal[3], pRpcCmd->params.aucVal[2], &pRpcCmd->params.aucVal[15], pRpcCmd->params.usVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CreateCmServiceFlow(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.aucVal[2], pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_MODIFY_MODEM_SERVICE_FLOW_97:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ModifyCmServiceFlow( olt_id, onu_id, &pRpcCmd->params.aucVal[18], pRpcCmd->params.aucVal[12], pRpcCmd->params.ulVals[1], pRpcCmd->params.ulVals[2], &pRpcCmd->params.aucVal[24], (unsigned short)pRpcCmd->params.ulVals[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ModifyCmServiceFlow(%d, %d, %lu, %lu, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVals[1], pRpcCmd->params.ulVals[2], pRpcCmd->params.ulVals[0], pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DESTROY_MODEM_SERVICE_FLOW_98:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DestroyCmServiceFlow( olt_id, onu_id, &pRpcCmd->params.aucVal[14], &pRpcCmd->params.aucVal[8], pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DestroyCmServiceFlow(%d, %d, %lu, %lu)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.ulVals[0], pRpcCmd->params.ulVals[1], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_CLASSIFIER_99:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmClassifier( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmClassifier(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_MODEM_SERVICE_FLOW_100:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmServiceFlow( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmServiceFlow(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_DUMP_CMC_CLASSIFIER_101:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
             unsigned short num_of_sf = pRpcCmd->params.usVals[1];
             unsigned long  array_len = num_of_sf * sizeof(unsigned long);
                
             if ( 0 == (iRet = OnuMgt_DumpCmcClassifier( olt_id, onu_id, &pRpcCmd->params.aucVal[array_len+4], num_of_sf, &pRpcCmd->params.aucVal[4], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcClassifier(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, num_of_sf, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_102:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
             unsigned short num_of_sf = pRpcCmd->params.usVals[1];
             unsigned long  array_len = num_of_sf * sizeof(unsigned long);
                
             if ( 0 == (iRet = OnuMgt_DumpCmcServiceFlow( olt_id, onu_id, &pRpcCmd->params.aucVal[array_len+4], num_of_sf, &pRpcCmd->params.aucVal[4], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcServiceFlow(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, num_of_sf, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_STATISTICS_103:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
             unsigned short num_of_sf = pRpcCmd->params.usVals[1];
             unsigned long  array_len = num_of_sf * sizeof(unsigned long);
                
             if ( 0 == (iRet = OnuMgt_DumpCmcServiceFlowStatistics( olt_id, onu_id, &pRpcCmd->params.aucVal[array_len+4], num_of_sf, &pRpcCmd->params.aucVal[4], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcServiceFlowStatistics(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, num_of_sf, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_GROUP_104:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcDownChannelBondingGroup( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcDownChannelBondingGroup(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_GROUP_105:
         ulResultLen = sizeof(ONU_RLT) + pRpcCmd->params.usVal;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             char *buf = pRpcResult + 1;
             unsigned short buf_len = pRpcCmd->params.usVal;
            
             if ( 0 == (iRet = OnuMgt_DumpCmcUpChannelBondingGroup( olt_id, onu_id, &pRpcCmd->params.aucVal[2], buf, &buf_len )) )
             {
                 iRet = buf_len;
             }
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DumpCmcUpChannelBondingGroup(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, buf_len, iRet, ulSrcSlot);
         }
         break; 
         
     case ONU_CMD_CMC_CREATE_SERVICE_FLOW_CLASS_NAME_106:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_CreateCmcServiceFlowClassName( olt_id, onu_id, &pRpcCmd->params.aucVal[4], &pRpcCmd->params.aucVal[10], &pRpcCmd->params.aucVal[10+pRpcCmd->params.usVals[1]], pRpcCmd->params.usVals[0] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_CreateCmcServiceFlowClassName(%d, %d, %s, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, &pRpcCmd->params.aucVal[10], pRpcCmd->params.usVal, iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_DESTROY_SERVICE_FLOW_CLASS_NAME_107:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DestroyCmcServiceFlowClassName( olt_id, onu_id, &pRpcCmd->params.aucVal[0], &pRpcCmd->params.aucVal[6] );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DestroyCmcServiceFlowClassName(%d, %d, %s)'s result(%d) from slot %d.\r\n", olt_id, onu_id, &pRpcCmd->params.aucVal[6], iRet, ulSrcSlot);
         }
         break; 
     case ONU_CMD_CMC_GET_CMC_MAC_ADDTBL_108:
         {
            unsigned short nNum = 0;
            unsigned short nIsOnlyNum;
            PON_address_record_t *pMacAddrTbl;

            if ( 0 == (nIsOnlyNum = pRpcCmd->params.usVal) )
            {
                pMacAddrTbl = Mac_addr_table;
            }
            else
            {
                pMacAddrTbl = NULL;
            }

            ulResultLen = sizeof(ONU_RLT);
            iRet = OnuMgt_GetCmcMacAddrTbl( olt_id, onu_id, &pRpcCmd->params.aucVal[2], &nNum, pMacAddrTbl );
            if ( (nNum > 0) && (pMacAddrTbl != NULL) )
            {
                ulResultLen += nNum * sizeof(PON_address_record_t);
            }
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( OLT_ERR_OK == iRet )
                {
                    if ( (0 < (iRet = nNum)) && (pMacAddrTbl != NULL) )
                    {
                        VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(ONU_RLT));
                    }
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmcMacAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, nNum, iRet, ulSrcSlot);
         }   
    
         break; 
     case ONU_CMD_CMC_GET_CM_MAC_ADDTBL_109:
         {
            unsigned short nNum = 0;
            unsigned short nIsOnlyNum;
            PON_address_record_t *pMacAddrTbl;

            if ( 0 == (nIsOnlyNum = pRpcCmd->params.usVal) )
            {
                pMacAddrTbl = Mac_addr_table;
            }
            else
            {
                pMacAddrTbl = NULL;
            }

            ulResultLen = sizeof(ONU_RLT);
            iRet = OnuMgt_GetCmMacAddrTbl( olt_id, onu_id, &pRpcCmd->params.aucVal[8], &pRpcCmd->params.aucVal[2], &nNum, pMacAddrTbl );
            if ( (nNum > 0) && (pMacAddrTbl != NULL) )
            {
                ulResultLen += nNum * sizeof(PON_address_record_t);
            }
            if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( OLT_ERR_OK == iRet )
                {
                    if ( (0 < (iRet = nNum)) && (pMacAddrTbl != NULL) )
                    {
                        VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(ONU_RLT));
                    }
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetCmMacAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, nNum, iRet, ulSrcSlot);
         }   
    
         break; 
     case ONU_CMD_CMC_RESET_CM_MAC_ADDTBL_110:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_ResetCmAddrTbl( olt_id, onu_id, &pRpcCmd->params.aucVal[10], &pRpcCmd->params.aucVal[4], pRpcCmd->params.iVal );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_ResetCmAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
         }
         break; 
     
#endif
	case ONU_CMD_CMC_Set_Multicast_Template_112:
         ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMulticastTemplate( olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], &pRpcCmd->params.aucVal[9],pRpcCmd->params.aucVal[8]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMulticastTemplate(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	 case ONU_CMD_CMC_Get_Multicast_Template_113:
        ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_GPONADP_ONU_Multicast_Prof_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetMulticastTemplate( olt_id, onu_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcResult+1,pRpcCmd->params.aucVal[8]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetMulticastTemplate(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	 case ONU_CMD_CMC_Set_Mcast_OperProfile_114:
        ulResultLen = sizeof(ONU_RLT) ;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetMcastOperProfile( olt_id, onu_id, pRpcCmd->params.iVals[0], &pRpcCmd->params.iVals[1]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetMcastOperProfile(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	  case ONU_CMD_CMC_Get_Mcast_OperProfile_115:
        ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_GPONADP_ONU_McastOper_Prof_t) ;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetMcastOperProfile( olt_id, onu_id, pRpcCmd->params.iVals[0],pRpcResult+1 );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetMcastOperProfile(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	 case ONU_CMD_CMC_SetUniPortAssociateMcastProf_116:
        ulResultLen = sizeof(ONU_RLT);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_SetUniPortAssociateMcastProf( olt_id, onu_id, pRpcCmd->params.sVals[2],pRpcCmd->params.acVal[6],pRpcCmd->params.iVals[0]);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetUniPortAssociateMcastProf(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	 case ONU_CMD_CMC_GetUniPortAssociateMcastProf_117:
        ulResultLen = sizeof(ONU_RLT) + sizeof(CTC_GPONADP_ONU_Profile_t);
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetUniPortAssociateMcastProf( olt_id, onu_id, pRpcCmd->params.sVals[0],pRpcResult+1 );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetUniPortAssociateMcastProf(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	case ONU_CMD_Set_Port_Isolation_118:
        ulResultLen = sizeof(ONU_RLT) ;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DrPengSetOnuPortIsolation( olt_id, onu_id, pRpcCmd->params.cVal);
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetOnuPortIsolation(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
         break; 
	 case ONU_CMD_Get_Port_Isolation_119:
        ulResultLen = sizeof(ONU_RLT) + sizeof(unsigned char) ;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_DrPengGetOnuPortIsolation( olt_id, onu_id,pRpcResult+1 );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuPortIsolation(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
	  break;
	 case ONU_CMD_Get_Onu_Mac_Entry_120:
        ulResultLen = sizeof(ONU_RLT) + sizeof(OnuPortLacationInfor_S) ;
         if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
         {
             iRet = OnuMgt_GetOnuMacEntry( olt_id, onu_id,pRpcCmd->params.iVals[0],pRpcResult+1 );
             OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetOnuMacEntry(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
         }
	  break;
	  case ONU_CMD_Set_Onu_Port_Save_Config_121:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuPortSaveConfig( olt_id, onu_id,pRpcCmd->params.sValPair[0],pRpcCmd->params.acVal[2]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuPortSaveConfig(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Set_Onu_Loop_DetectionTime_122:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuLoopDetectionTime( olt_id, onu_id,pRpcCmd->params.sValPair[0],pRpcCmd->params.sValPair[1]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuLoopDetectionTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	   case ONU_CMD_Get_Onu_Loop_DetectionTime_123:
	  ulResultLen = sizeof(ONU_RLT)+sizeof(short)*2;
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
	   	   unsigned short *pResult = pRpcResult + 1;
		   iRet = OnuMgt_DrPengGetOnuLoopDetectionTime( olt_id, onu_id,pResult,pResult+1);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuLoopDetectionTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Set_Onu_Port_Mode_124:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuPortMode( olt_id, onu_id,pRpcCmd->params.sValPair[0],pRpcCmd->params.acVal[2]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuPortMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	 case ONU_CMD_Get_Onu_Port_Mode_125:
	  ulResultLen = sizeof(ONU_RLT)+sizeof(char);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengGetOnuPortMode( olt_id, onu_id,pRpcCmd->params.sVal,pRpcResult+1);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuPortMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Set_Onu_Port_StormStatus_126:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuPortStormStatus( olt_id, onu_id,pRpcCmd->params.sVal,&(pRpcCmd->params.sValPair[1]));
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuPortStormStatus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	 case ONU_CMD_Get_Onu_Port_StormStatus_127:
	  ulResultLen = sizeof(ONU_RLT)+sizeof(OnuPortStorm_S);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengGetOnuPortStormStatus( olt_id, onu_id,pRpcCmd->params.sVal,pRpcResult+1);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuPortStormStatus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Set_Onu_DeviceLocation_128:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuDeviceLocation( olt_id, onu_id,&(pRpcCmd->params.aucVal[1]),pRpcCmd->params.aucVal[0]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuDeviceLocation(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Get_Onu_DeviceLocation_129:
	  ulResultLen = sizeof(ONU_RLT)+NAMELEN;
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
	   	   unsigned char *pResult = pRpcResult+1;
		   iRet = OnuMgt_DrPengGetOnuDeviceLocation( olt_id, onu_id,pResult);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuDeviceLocation(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Set_Onu_DeviceDescription_130:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuDeviceDescription( olt_id, onu_id,&(pRpcCmd->params.aucVal[1]),pRpcCmd->params.aucVal[0]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuDeviceDescription(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Get_Onu_DeviceDescription_131:
	  ulResultLen = sizeof(ONU_RLT)+NAMELEN;
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
	   	   unsigned char *pResult = pRpcResult+1;
		   iRet = OnuMgt_DrPengGetOnuDeviceDescription( olt_id, onu_id,pResult);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuDeviceDescription(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	   case ONU_CMD_Set_Onu_DeviceName_132:
	  ulResultLen = sizeof(ONU_RLT);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengSetOnuDeviceName( olt_id, onu_id,&(pRpcCmd->params.aucVal[1]),pRpcCmd->params.aucVal[0]);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengSetOnuDeviceName(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Get_Onu_DeviceName_133:
	  ulResultLen = sizeof(ONU_RLT)+NAMELEN;
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
	   	   unsigned char *pResult = pRpcResult+1;
		   iRet = OnuMgt_DrPengGetOnuDeviceName( olt_id, onu_id,pResult);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuDeviceName(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	   case ONU_CMD_Get_Onu_PortMacNumber_134:
	  ulResultLen = sizeof(ONU_RLT)+sizeof(short);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
	   	   unsigned short *pResult = pRpcResult+1;
		   iRet = OnuMgt_DrPengGetOnuPortMacNumber( olt_id, onu_id,pRpcCmd->params.sVal,pResult);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuPortMacNumber(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	   case ONU_CMD_Get_Onu_PortLocationByMAC_135:
	  ulResultLen = sizeof(ONU_RLT)+sizeof(OnuPortLacationEntry_S);
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengGetOnuPortLocationByMAC( olt_id, onu_id,pRpcCmd->params.aucVal,pRpcCmd->params.sVals[3],pRpcResult+1);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuPortLocationByMAC(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  case ONU_CMD_Get_Onu_ExtendAttribute_136:
	  ulResultLen = sizeof(ONU_RLT)+SUPPORT_ATTRIBUTE_LEN;
	   if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
	   {
		   iRet = OnuMgt_DrPengGetOnuExtendAttribute( olt_id, onu_id,pRpcResult+1);
		   OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_DrPengGetOnuExtendAttribute(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
	   }
	  break;
	  
#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
    case ONU_CMD_SPLIT_LINE_OMCI_GET_ONU_CFG:
        ulResultLen = sizeof(ONU_RLT)+sizeof(tOgCmOnuConfig);
        if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
        {
            iRet = OnuMgt_GetGponOnuCfg(olt_id, onu_id, (tOgCmOnuConfig*)pRpcResult+1);
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_GetGponOnuCfg(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);            
        }    
        break;
        
    case ONU_CMD_SPLIT_LINE_OMCI_SET_ONU_CFG:
        ulResultLen = sizeof(ONU_RLT);
        if(OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen))
        {
            iRet = OnuMgt_SetGponOnuCfg(olt_id, onu_id, (tOgCmOnuConfig*)&pRpcCmd->params.iVals[0]);
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OnuMgt_SetGponOnuCfg(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);            
        }
        break;
#endif        
    default:
        ulResultLen = sizeof(ONU_RLT);
        if ( OLT_ERR_OK == ONU_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        {
            iRet = OLT_ERR_NOTSUPPORT;
        }
    }
    }
    
    if ( pRpcResult != NULL )
    {
        pRpcResult->rlt = iRet;
        
        *ppSendData     = pRpcResult;

        if ( iRet >= 0 )
        {
            *pulSendDataLen = ulResultLen;
        }
        else
        {
            /* 失败返回结果即可 */
            *pulSendDataLen = sizeof(ONU_RLT);
        }
    }
    else
    {
        *ppSendData     = NULL;
        *pulSendDataLen = 0;
    }

    VOS_SysLog(LOG_TYPE_ONU, LOG_DEBUG, "ONU_Rpc_Callback(oltid:%d, onuid:%d)'s result(%d) on slot(%d), recv cmd[%d] from slot[%d] ", olt_id, onu_id, iRet, SYS_LOCAL_MODULE_SLOTNO, pRpcCmd->cmd, ulSrcSlot);
	
    return iRet;
}

/*-----------------------------------内部适配----------------------------------------------*/


#if 1
/* -------------------ONU基本API------------------- */

static int RPC_OnuError(short int olt_id, short int onu_id)
{
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuError(%d, %d)'s result(%d).\r\n", olt_id, onu_id, OLT_ERR_NOTEXIST);

    VOS_ASSERT(0);

    return OLT_ERR_NOTEXIST;
}

static int RPC_OnuIsValid(short int olt_id, short int onu_id, int *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只获取ONU内存状态，无需从实接口处获取 */
        iRlt = GWONU_OnuIsValid(olt_id, onu_id, status);
    }
    else
    {
        iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ONU_IS_VALID_1, NULL, 0, status, sizeof(int), 0, 0);
    }

    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuIsValid(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *status, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_OnuIsOnline(short int olt_id, short int onu_id, int *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(status);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ONU_IS_ONLINE_2, NULL, 0, status, sizeof(int), 0, 0);
    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuIsOnline(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *status, iRlt, cfg_slot); */

    return iRlt;
}

/* pending onu*/
static int RPC_AddOnuByManual(short int olt_id, short int onu_id,  unsigned char *MacAddr)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_ADD_ONU_MANURAL_3, MacAddr, 6, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddOnuByManual(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}
static int RPC_ModifyOnuByManual(short int olt_id, short int onu_id,  unsigned char *MacAddr)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_MODIFY_ONU_MANURAL_9, MacAddr, 6, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ModifyOnuByManual(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}

static int RPC_DelOnuByManual(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_ONU_MANURAL_4, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelOnuByManual(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_AddGponOnuByManual(short int olt_id, short int onu_id,  unsigned char *sn)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_ADD_GPON_ONU_MANURAL_10, sn, GPON_ONU_SERIAL_NUM_STR_LEN, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddGponOnuByManual(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}

static int RPC_OnuCmdIsSupported(short int olt_id, short int onu_id, short int *cmd)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(cmd);
    
    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只获取命令状态，无需从实接口处获取 */
        iRlt = GWONU_CmdIsSupported(olt_id, onu_id, cmd);
    }
    else
    {
        iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_IS_SUPPORT_CMD_5, cmd, sizeof(short int), NULL, 0, 0, 0);
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuCmdIsSupported(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *cmd, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_CopyOnu(short int olt_id, short int onu_id, short int dst_olt_id, short int dst_onu_id, int copy_flags)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    OLT_ASSERT(dst_olt_id);
    ONU_ASSERT(onu_id);
    ONU_ASSERT(dst_onu_id);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只拷贝ONU的配置，无需从实接口处拷贝 */
        iRlt = GWONU_CopyOnu(olt_id, onu_id, dst_olt_id, dst_onu_id, copy_flags);
    }
    else
    {
        CmdParam[0] = dst_olt_id;
        CmdParam[1] = dst_onu_id;
        CmdParam[2] = copy_flags;
        iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_COPY_ONU_6, CmdParam, sizeof(CmdParam), NULL, 0, 20000, OLT_RPC_CALLFLAG_ASYNC);
    }
    
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CopyOnu(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, dst_olt_id, dst_onu_id, copy_flags, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetIFType(short int olt_id, short int onu_id, int *chip_type, int *remote_type)
{
    int iRlt;
    int cfg_slot;
    int CmdRlt[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chip_type);
    VOS_ASSERT(remote_type);
    
    if ( 0 == (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_IFTYPE_7, NULL, 0, CmdRlt, sizeof(CmdRlt), 0, 0)) )
    {
        *chip_type   = CmdRlt[0];
        *remote_type = CmdRlt[1];
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetIFType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *chip_type, *remote_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIFType(short int olt_id, short int onu_id, int chip_type, int remote_type)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = chip_type;
    CmdParam[1] = remote_type;    
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IFTYPE_8, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIFType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, chip_type, remote_type, iRlt, cfg_slot);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 认证管理API------------------- */
/*onu deregister*/
static int RPC_DeregisterOnu(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEREGISTERT_ONU_1, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DeregisterOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}

static int RPC_SetMacAuthMode(short int olt_id, short int onu_id, int auth_mode, mac_address_t auth_mac)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[7];
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(auth_mac);

    CmdParam[0] = (unsigned char)auth_mode;
    VOS_MemCpy(&CmdParam[1], auth_mac, 6);
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_MAC_AUTH_MODE_2, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMacAuthMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

/* delete bind onu */
static int RPC_DelBindingOnu(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code, int code_param)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = deregistration_code;
    CmdParam[1] = code_param;
    iRlt = ONU_Rpc_Call(olt_id, onu_id,&cfg_slot, ONU_CMD_DEL_BIND_ONU_3, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelBindingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

#if 0
/*add pending onu*/
static int RPC_AddPendingOnu(short int olt_id, short int onu_id, unsigned char *MacAddr)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(MacAddr);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ADD_PENDING_ONU_4, MacAddr, 6, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddPendingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelPendingOnu(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_DEL_PENDING_ONU_5, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelPendingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelConfPendingOnu(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id,onu_id, &cfg_slot, ONU_CMD_DEL_CONF_PENDING_ONU_6, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelConfPendingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}
#endif

static int RPC_AuthorizeOnu(short int olt_id, short int onu_id, bool auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id,onu_id, &cfg_slot, ONU_CMD_AUTHORIZE_ONU_7, &auth_mode, sizeof(auth_mode), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AuthorizeOnu(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, auth_mode, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 业务管理API------------------- */

static int RPC_SetOnuTrafficServiceMode(short int olt_id, short int onu_id, int service_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_TRAFFIC_SERVICE_MODE_1, &service_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuTrafficServiceMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, service_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuPeerToPeer(short int olt_id, short int onu_id1, short int onu_id2, short int enable)
{
    int iRlt;
    int cfg_slot;
    short int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id1);
    ONU_ASSERT(onu_id2);

    CmdParam[0] = onu_id2;	
    CmdParam[1] = enable;	
    iRlt = ONU_Rpc_Call(olt_id, onu_id1, &cfg_slot, ONU_CMD_SET_ONU_PEER_TO_PEER_2, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPeerToPeer(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id1, onu_id2, enable, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuPeerToPeerForward(short int olt_id, short int onu_id, int address_not_found, int broadcast )
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];
 
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    CmdParam[0] = address_not_found;	
    CmdParam[1] = broadcast;	

    iRlt = ONU_Rpc_Call(olt_id, onu_id,&cfg_slot, ONU_CMD_SET_ONU_PEER_TO_PEER_FORWARD_3, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPeerToPeerForward(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, address_not_found, broadcast, iRlt, cfg_slot);

    return iRlt;
}

static int  RPC_SetOnuBW(short int olt_id, short int onu_id, ONU_bw_t *BW)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(BW);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_BW_4, BW, sizeof(ONU_bw_t), BW, sizeof(ONU_bw_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuBW(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, BW->bw_direction, BW->bw_gr, iRlt, cfg_slot);

    return iRlt;
}

static int  RPC_GetOnuSLA(short int olt_id, short int onu_id, ONU_SLA_INFO_t *sla_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(sla_info);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_SLA_5, NULL, 0, sla_info, sizeof(ONU_SLA_INFO_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuSLA(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, sla_info->SLA_Ver, sla_info->DBA_ErrCode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuFecMode(short int olt_id, short int onu_id, int fec_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id,&cfg_slot, ONU_CMD_SET_FEC_MODE_6, &fec_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuFecMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, fec_mode, iRlt, cfg_slot);

    return iRlt;
}

/*get onu vlan mode*/
static int RPC_GetOnuVlanMode(short int olt_id, short int onu_id, PON_olt_vlan_uplink_config_t *vlan_uplink_config)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(vlan_uplink_config);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_VLAN_MODE_7, NULL, 0, vlan_uplink_config, sizeof(PON_olt_vlan_uplink_config_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuVlanMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetUniPort(short int olt_id, short int onu_id, bool enable_cpu, bool enable_datapath)
{
    int iRlt;
    int cfg_slot;
	bool CmdParam[2];
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	CmdParam[0] = enable_cpu;	
    CmdParam[1] = enable_datapath;	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_UNI_PORT_8, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetUniPort(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, enable_cpu, enable_datapath, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetSlowProtocolLimit(short int olt_id, short int onu_id, bool enable)
{
    int iRlt;
    int cfg_slot;
		
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_SLOW_PROTOCOL_LIMIT_9, &enable, sizeof(bool), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetSlowProtocolLimit(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, enable, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetSlowProtocolLimit(short int olt_id, short int onu_id, bool *enable)
{
    int iRlt;
    int cfg_slot;
		
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_SLOW_PROTOCOL_LIMIT_10, NULL, 0, enable, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetSlowProtocolLimit(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *enable, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/


/*Begin:for onu swap by jinhl@2013-04-27*/
static int RPC_GetOnuBWInfo(short int olt_id, short int onu_id, PonPortBWInfo_S *ponBW, ONUBWInfo_S *onuBW)
{
    int iRlt;
    int cfg_slot;
	struct 
	{
	    PonPortBWInfo_S ponBW;
		ONUBWInfo_S onuBW;
	}bwInfo;
	

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

	if( (NULL == ponBW) || (NULL == onuBW))
	{
	    VOS_ASSERT(0);
		return VOS_ERROR;
	}


    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_BWINFO_11, NULL, 0, &bwInfo, sizeof(bwInfo), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuBWInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);
	if(VOS_OK == iRlt)
	{
	    VOS_MemCpy(ponBW, &(bwInfo.ponBW), sizeof(PonPortBWInfo_S));
		VOS_MemCpy(onuBW, &(bwInfo.onuBW), sizeof(ONUBWInfo_S));
	}

    return iRlt;
} 
/*End:for onu swap by jinhl@2013-04-27*/

static int RPC_GetOnuB2PMode(short int olt_id, short int onu_id, int *b2p_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id,&cfg_slot, ONU_CMD_GET_B2P_MODE_12, NULL, 0, b2p_mode, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuB2PMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *b2p_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuB2PMode(short int olt_id, short int onu_id, int b2p_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id,&cfg_slot, ONU_CMD_SET_B2P_MODE_13, &b2p_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuB2PMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, b2p_mode, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 监控统计管理API------------------- */

static int RPC_ResetCounters(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_RESET_COUNTER_1, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetCounters(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonLoopback(short int olt_id, short int onu_id, int enable)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PON_LOOPBACK_2, &enable, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonLoopback(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, enable, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 加密管理API----------- */

static int RPC_GetLLIDParams(short int olt_id, short int onu_id, PON_llid_parameters_t *llid_parameters)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(llid_parameters);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_LLID_PARAMS_1, NULL, 0, llid_parameters, sizeof(PON_llid_parameters_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDParams(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}
 
static int  RPC_StartEncryption(short int olt_id, short int onu_id, int *encrypt_dir)
{
    int iRlt;
    int cfg_slot;
    int iEncDir;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(encrypt_dir);

    iEncDir = *encrypt_dir;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_START_ENCRYPTION_2, encrypt_dir, sizeof(int), encrypt_dir, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StartEncryption(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iEncDir, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_StopEncryption(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_STOP_ENCRYPTION_3, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StopEncryption(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int key_change_time)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];
    int CmdRltSize;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( NULL == encrypt_dir )
    {
        CmdParam[0] = -1;	
        CmdRltSize  = 0;
    }
    else
    {
        CmdParam[0] = *encrypt_dir;
        CmdRltSize  = sizeof(int);
    }
    CmdParam[1] = key_change_time;	

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_ENCRYPT_PARAMS_4, CmdParam, sizeof(CmdParam), encrypt_dir, CmdRltSize, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuEncryptParams(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, CmdParam[0], CmdParam[1], iRlt, cfg_slot);

    return iRlt;
}
  
static int  RPC_GetOnuEncryptParams(short int olt_id, short int onu_id, int *encrypt_dir, int *key_change_time, int *encrypt_status)
{
    int iRlt;
    int cfg_slot;
    int CmdRlt[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( 0 == (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_ENCRYPT_PARAMS_5, NULL, 0, CmdRlt, sizeof(CmdRlt), 0, 0)) )
    {
        if ( NULL != encrypt_dir )
        {
            *encrypt_dir = CmdRlt[0];
        }

        if ( NULL != key_change_time )
        {
            *key_change_time = CmdRlt[1];
        }
	 if( NULL != encrypt_status )
	 	*encrypt_status = CmdRlt[2];
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuEncryptParams(%d,%d,%d,%d,%d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, CmdRlt[0], CmdRlt[1], CmdRlt[2], iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int  RPC_UpdateEncryptionKey(short int olt_id, short int onu_id, PON_encryption_key_t encryption_key, PON_encryption_key_update_t key_update_method)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(PON_encryption_key_t)+sizeof(PON_encryption_key_update_t)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(CmdParam, encryption_key, sizeof(PON_encryption_key_t));
    VOS_MemCpy(&CmdParam[sizeof(PON_encryption_key_t)], &key_update_method, sizeof(PON_encryption_key_update_t));

    if ( 0 == (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_UPDATE_ENCRYPTION_KEY_6, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0)) )
    {
        
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_UpdateEncryptionKey(%d,%d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif


#if 1
/* -------------------ONU 地址表管理API-------- */
/*show fdbentry mac*/
static int RPC_GetOnuMacAddrTbl(short int olt_id, short int onu_id, long *EntryNum, PON_onu_address_table_record_t *addr_table )
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    int iIsOnlyNum;
    VOID *pRcvBuf;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

    if ( NULL != addr_table )
    {
        iIsOnlyNum = 0;
        pRcvBuf = addr_table;
        iRcvBufLen = sizeof(MAC_Address_Table);
    }
    else
    {
        iIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_MAC_ADDTBL_1, &iIsOnlyNum, sizeof(int), pRcvBuf, iRcvBufLen, 0, 0);
    if (iRlt >= 0)
    {
    	*EntryNum = iRlt;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuMacAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOltMacAddrTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_table_t addr_table )
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    int iIsOnlyNum;
    VOID *pRcvBuf;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

    if ( NULL != addr_table )
    {
        iIsOnlyNum = 0;
        pRcvBuf = addr_table;
        iRcvBufLen = sizeof(PON_address_table_t);
    }
    else
    {
        iIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_OLT_MAC_ADDTBL_2, &iIsOnlyNum, sizeof(int), pRcvBuf, iRcvBufLen, 0, 0);
    if (iRlt >= 0)
    {
    	*EntryNum = iRlt;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltMacAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOltMacAddrVlanTbl(short int olt_id, short int onu_id, short int *EntryNum, PON_address_vlan_table_t addr_table )
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    int iIsOnlyNum;
    VOID *pRcvBuf;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(EntryNum);

    if ( NULL != addr_table )
    {
        iIsOnlyNum = 0;
        pRcvBuf = addr_table;
        iRcvBufLen = sizeof(PON_address_vlan_table_t);
    }
    else
    {
        iIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_OLT_MAC_VLAN_ADDTBL_7, &iIsOnlyNum, sizeof(int), pRcvBuf, iRcvBufLen, 0, 0);
    if (iRlt >= 0)
    {
    	*EntryNum = iRlt;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltMacAddrVlanTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *EntryNum, iRlt, cfg_slot);

    return iRlt;
}


/*set onu max mac*/
static int RPC_SetOnuMaxMac(short int olt_id, short int onu_id, short int llid_id, unsigned int *val)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid_id);
    VOS_ASSERT(val);

    CmdParam[0] = llid_id;	
    CmdParam[1] = *val;	

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_MAX_MAC_3, CmdParam, sizeof(CmdParam), val, sizeof(unsigned int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuMaxMacNum(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, llid_id, *val, iRlt, cfg_slot);

    return iRlt;
}

/*get onu uni mac config*/
static int RPC_GetOnuUniMacCfg(short int olt_id, short int onu_id , PON_oam_uni_port_mac_configuration_t *mac_config )
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(mac_config);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_UNI_CONFIG_4, NULL, 0, mac_config, sizeof(PON_oam_uni_port_mac_configuration_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuUniMacCfg(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOnuMacCheckFlag(short int olt_id, short int onu_id ,ULONG  *flag)
{
    int iRlt;
    int cfg_slot;
    ULONG  CheckFlag=0;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(flag);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_MAC_CHECK_FLAG_5, NULL, 0, &CheckFlag, sizeof(ULONG), 0, 0);
    if (iRlt >= 0)
    {
    	*flag =CheckFlag;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuMacCheckFlag(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}
static int RPC_GetAllEthPortMacCounter(short int olt_id, short int onu_id ,OnuEthPortCounter_t *data)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_ALL_PORT_MAC_COUNTER_6, NULL, 0, data, sizeof(OnuEthPortCounter_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetAllEthPortMacCounter(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;

}

#endif


#if 1
/* -------------------ONU 光路管理API------------------- */
/*RTT*/
static int RPC_GetOnuDistance(short int olt_id, short int onu_id, int *rtt)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(rtt);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_DISTANCE_1, NULL, 0, rtt, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuDistance(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *rtt, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOpticalCapability(short int olt_id, short int onu_id, ONU_optical_capability_t *optical_capability)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(optical_capability);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_OPTICAL_CAPABILITY_2, NULL, 0, optical_capability, sizeof(ONU_optical_capability_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOpticalCapability(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, optical_capability->pon_tx_signal, iRlt, cfg_slot);

    return iRlt;
}
#endif


#if 1
/* -------------------ONU 倒换API---------------- */

static int RPC_SetOnuLLID(short int olt_id, short int onu_id, short int llid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    LLID_ASSERT(llid);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_LLID_1, &llid, sizeof(short int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuLLID(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, llid, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU 设备管理API------------------- */

static int RPC_GetOnuVer(short int olt_id, short int onu_id, PON_onu_versions *onu_versions)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_versions);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_VER_1, NULL, 0, onu_versions, sizeof(PON_onu_versions), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuVer(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

/*get onu pon ver*/
static int RPC_GetOnuPonVer(short int olt_id, short int onu_id, PON_device_versions_t *device_versions )
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(device_versions);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_PON_VER_2, NULL, 0, device_versions, sizeof(PON_device_versions_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPonVer(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOnuRegisterInfo(short int olt_id, short int onu_id, onu_registration_info_t *onu_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(onu_info);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_INFO_3, NULL, 0, onu_info, sizeof(onu_registration_info_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuRegisterInfo(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}	

static int RPC_GetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long *size)
{
    int iRlt;
    int cfg_slot;
    unsigned long ulRltLen;
    unsigned long CmdParam[2];
    ONU_RLT *pCmdResult = NULL;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    CmdParam[0] = info_id;
    CmdParam[1] = *size;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_I2C_INFO_4, CmdParam, sizeof(CmdParam), &pCmdResult, sizeof(ONU_RLT*), 0, OLT_RPC_CALLFLAG_MANUALFREERLT);
    if ( NULL != pCmdResult )
    {
        if ( *size > (ulRltLen = *((unsigned long*)(pCmdResult->values))) )
        {
            *size = ulRltLen;
        }

        VOS_MemCpy(data, &pCmdResult->values[sizeof(unsigned long)], *size);
        ONU_RPC_RLT_FREE(pCmdResult);
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, *size, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuI2CInfo(short int olt_id, short int onu_id, int info_id, void *data, unsigned long size)
{
    int iRlt;
    int cfg_slot;
    unsigned long CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(data);

    CmdParam[0] = info_id;
    CmdParam[1] = size;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_I2C_INFO_5, CmdParam, sizeof(CmdParam), data, size, 0, OLT_RPC_CALLFLAG_OUTPUTASINPUT);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuI2CInfo(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, info_id, *(char*)data, size, iRlt, cfg_slot);

    return iRlt;
}


/*onu remote reset*/
static int RPC_ResetOnu(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_RESET_ONU_6, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuSWUpdateMode(short int olt_id, short int onu_id, int update_mode)
{
    int iRlt;
    int cfg_slot;
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_SW_UPDATE_MODE_7, &update_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuSWUpdateMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, update_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_OnuSwUpdate(short int olt_id, short int onu_id, int update_flags, unsigned int update_filetype)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    CmdParam[0] = update_flags;	
    CmdParam[1] = update_filetype;	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ONU_SW_UPDATE_8, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuSwUpdate(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, update_flags, update_filetype, iRlt, cfg_slot);

    return iRlt;
}	

static int RPC_OnuSwGwCtcConvert(short int olt_id, short int onu_id, char file_id[ONU_TYPE_LEN + 4])
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GW_CTC_SW_CONVERT_9, file_id, ONU_TYPE_LEN + 4, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OnuSwGwCtcConvert(%d, %d, %s)'s result(%d) to slot %d.\r\n", olt_id, onu_id, file_id, iRlt, cfg_slot);

    return iRlt;
}	

/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int RPC_GetBurnImageComplete(short int olt_id, short int onu_id, bool *complete)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Burn_Image_Complete_10, NULL, 0, complete, sizeof(complete), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_Get_Burn_Image_Complete(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}	
/*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/


static int RPC_SetOnuDeviceName(short int olt_id, short int onu_id, char *Name, int NameLen)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Name);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_DEVICE_NAME_11, Name, NameLen, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDeviceName(%d, %d, %s, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, Name, NameLen, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuDeviceDesc(short int olt_id, short int onu_id,char *Desc, int DescLen)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Desc);

    iRlt = ONU_Rpc_Call(olt_id,onu_id, &cfg_slot, ONU_CMD_SET_ONU_DEVICE_DESC_12, Desc, DescLen, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDeviceDesc(%d, %d, %s, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, Desc, DescLen, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuDeviceLocation(short int olt_id, short int onu_id, char *Location, int LocationLen)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(Location);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ONU_DEVICE_LOCATION_13, Location, LocationLen, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDeviceLocation(%d, %d, %s, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, Location, LocationLen, iRlt, cfg_slot);

    return iRlt;
}
/*获取ONU的端口统计数据*/
static int RPC_GetOnuAllPortStatisticData(short int olt_id, short int onu_id, OnuStatisticData_S* data)
{
    int iRlt;
    int cfg_slot;


    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_ONU_ALL_PORT_STATISTIC_DATA_14, 0, 0, data,sizeof(OnuStatisticData_S), 0, 0);
 
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuAllPortStatisticData(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------ONU CTC-PROTOCOL API--------- */

static int RPC_GetCtcVersion( short int olt_id, short int onu_id, unsigned char *version )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_CTC_VERSION_1, NULL, 0, version, sizeof(unsigned char), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCtcVersion(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *version, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetFirmwareVersion( short int olt_id, short int onu_id, unsigned short int *version )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(version);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_FIRMWARE_VERSION_2, NULL, 0, version, sizeof(unsigned short int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetFirmwareVersion(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *version, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetSerialNumber( short int olt_id, short int onu_id, CTC_STACK_onu_serial_number_t *serial_number )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(serial_number);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_SERIAL_NUMBER_3, NULL, 0, serial_number, sizeof(CTC_STACK_onu_serial_number_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetSerialNumber(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetChipsetID( short int olt_id, short int onu_id, CTC_STACK_chipset_id_t *chipset_id )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(chipset_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_CHIPSET_ID_4, NULL, 0, chipset_id, sizeof(CTC_STACK_chipset_id_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetChipsetID(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetOnuCap1( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_t *caps )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ONU_CAP1_5, NULL, 0, caps, sizeof(CTC_STACK_onu_capabilities_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuCap1(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOnuCap2( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_2_t *caps )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ONU_CAP2_6, NULL, 0, caps, sizeof(CTC_STACK_onu_capabilities_2_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuCap2(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOnuCap3( short int olt_id, short int onu_id, CTC_STACK_onu_capabilities_3_t *caps )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(caps);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ONU_CAP3_7, NULL, 0, caps, sizeof(CTC_STACK_onu_capabilities_3_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuCap3(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_StartEncrypt(short int olt_id, short int onu_id)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_START_ENCRYPTION_11, NULL, 0, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StartEncrypt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_StopEncrypt(short int olt_id, short int onu_id)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_STOP_ENCRYPTION_12, NULL, 0, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StopEncrypt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortLinkState(short int olt_id, short int onu_id, int port_id, int *ls )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_LINKSTATE_13, &port_id, sizeof(int), ls, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortLinkState(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int *enable)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_ADMIN_STATUS_14, &port_id, sizeof(int), enable, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortAdminStatus(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortAdminStatus(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt, cfg_slot;
    int cmd_para[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    cmd_para[0] = port_id;
    cmd_para[1] = enable;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_ADMIN_STATUS_15, cmd_para, sizeof(cmd_para), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortAdminStatus(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id,  iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortPause(short int olt_id, short int onu_id, int port_id, int *enable)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_PAUSE_16, &port_id, sizeof(int), enable, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortPause(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortPause(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt, cfg_slot;
    int cmd_para[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    cmd_para[0] = port_id;
    cmd_para[1] = enable;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_PAUSE_17, cmd_para, sizeof(cmd_para), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortPause(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int *an )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_AUTO_NEGOTIATION_ADMIN_18, &port_id, sizeof(int), an, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortAutoNegotiationAdmin(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortAutoNegotiationAdmin(short int olt_id, short int onu_id, int port_id, int an )
{
    int iRlt, cfg_slot;
    int cmd_para[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    cmd_para[0] = port_id;
    cmd_para[1] = an;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_ADMIN_19, cmd_para, sizeof(cmd_para), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortAutoNegotiationAdmin(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id,  iRlt, cfg_slot);

    return iRlt;
}
static int RPC_SetEthPortAutoNegotiationRestart(short int olt_id, short int onu_id, int port_id )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_AUTO_NEGOTIATION_RESTART_20, &port_id, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortAutoNegotiationRestart(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortAnLocalTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_AN_LOCAL_TECABILITY_21, &port_id, sizeof(int), ability, sizeof(CTC_STACK_auto_negotiation_technology_ability_t),  0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortAnLocalTecAbility(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortAnAdvertisedTecAbility(short int olt_id, short int onu_id, int port_id, CTC_STACK_auto_negotiation_technology_ability_t *ability)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_AN_ADVERTISED_TECABILITY_22, &port_id, sizeof(int), ability, sizeof(CTC_STACK_auto_negotiation_technology_ability_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortAnAdvertisedTecAbility(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_POLICING_23, &port_id, sizeof(int), policing, sizeof(CTC_STACK_ethernet_port_policing_entry_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CTCONU_Get_Ethport_Policing(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_policing_entry_t *policing)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(*policing)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], policing, sizeof(*policing));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_POLICING_24, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortPolicing(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_DOWNSTREAM_POLICING_25, &port_id, sizeof(int), policing, sizeof(CTC_STACK_ethernet_port_ds_rate_limiting_entry_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_SetEthPortDownstreamPolicing(short int olt_id, short int onu_id, int port_id, CTC_STACK_ethernet_port_ds_rate_limiting_entry_t *policing)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(*policing)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], policing, sizeof(*policing));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_DOWNSTREAM_POLICING_26, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortDownstreamPolicing(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_VCONF_27, &port_id, sizeof(int), vconf, sizeof(CTC_STACK_port_vlan_configuration_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortVlanConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortVlanConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_port_vlan_configuration_t *vconf)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(*vconf)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], vconf, sizeof(*vconf));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_VCONF_28, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortVlanConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id,  iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, CTC_STACK_classification_rules_t cam )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_CLASSIFICATIOANDMARK_30, &port_id, sizeof(int), cam, sizeof(CTC_STACK_classification_rules_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id, int mode, CTC_STACK_classification_rules_t cam)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(mode) + sizeof(CTC_STACK_classification_rules_t)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], &mode, sizeof(mode));
    VOS_MemCpy(&CmdParam[sizeof(port_id)+sizeof(mode)], cam, sizeof(CTC_STACK_classification_rules_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_CLASSIFICATIOANDMARK_31, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClearEthPortClassificationAndMarking(short int olt_id, short int onu_id, int port_id)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_CLR_ETHPORT_CLASSIFICATIOANDMARK_32, &port_id, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClearEthPortClassificationAndMarking(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_MULTICAST_VLAN_33, &port_id, sizeof(int), mv, sizeof(CTC_STACK_multicast_vlan_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortMulticastVlan(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortMulticastVlan(short int olt_id, short int onu_id,  int port_id, CTC_STACK_multicast_vlan_t *mv )
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(*mv)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], mv, sizeof(*mv));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_MULTICAST_VLAN_34, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortMulticastVlan(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClearEthPortMulticastVlan(short int olt_id, short int onu_id, int port_id )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_VLAN_35, &port_id, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClearEthPortMulticastVlan(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int *num )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_MULTICAST_MAX_GROUP_36, &port_id, sizeof(int), num, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortMulticastGroupMaxNumber(short int olt_id, short int onu_id, int port_id, int num )
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = num;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_MULTICAST_MAX_GROUP_37, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortMulticastGroupMaxNumber(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int *strip )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_MULTICAST_TAG_STRIP_38, &port_id, sizeof(int), strip, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortMulticastTagStrip(short int olt_id, short int onu_id, int port_id, int strip )
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = strip;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_MULTICAST_TAG_STRIP_39, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortMulticastTagStrip(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t *oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt, cfg_slot;

    char buf[256] = "";

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_IGMP_TAG_OPER_41, &port_id, sizeof(int), buf, sizeof(buf), 0, 0);

    if(iRlt == OLT_ERR_OK)
    {
    	*oper = *(int*)buf;
    	*sw = *(CTC_STACK_multicast_vlan_switching_t*)(buf+sizeof(int));
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortMulticastTagOper(short int olt_id, short int onu_id, int port_id, CTC_STACK_tag_oper_t oper, CTC_STACK_multicast_vlan_switching_t *sw )
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(oper) + sizeof(*sw)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], &oper, sizeof(oper));
    VOS_MemCpy(&CmdParam[sizeof(port_id)+sizeof(oper)], sw, sizeof(*sw));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_IGMP_TAG_OPER_42, &port_id, sizeof(int), CmdParam, sizeof(CmdParam), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortMulticastTagOper(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ETHPORT_MULTICAST_CONTROL_44, NULL, 0, mc, sizeof(CTC_STACK_multicast_control_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMulticastControl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMulticastControl(short int olt_id, short int onu_id, CTC_STACK_multicast_control_t *mc )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ETHPORT_MULTICAST_CONTROL_45, mc, sizeof(CTC_STACK_multicast_control_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMulticastControl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClearMulticastControl(short int olt_id, short int onu_id )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_CLR_ETHPORT_MULTICAST_CONTROL_46, NULL, 0, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClearMulticastControl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMulticastSwitch(short int olt_id, short int onu_id, int *sw)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_MULTICAST_SWITCH_47, NULL, 0, sw, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMulticastSwitch(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMulticastSwitch(short int olt_id, short int onu_id, int sw)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_MULTICAST_SWITCH_48, &sw, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMulticastSwitch(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMulticastFastleaveAbility(short int olt_id, short int onu_id, CTC_STACK_fast_leave_ability_t *ability)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_ABILITY_49, NULL, 0, ability, sizeof(CTC_STACK_fast_leave_ability_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMulticastFastleaveAbility(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMulticastFastleave(short int olt_id, short int onu_id, int *fla)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_MULTICAST_FASTLEAVE_50, NULL, 0, fla, sizeof(int), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMulticastFastleave(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMulticastFastleave(short int olt_id, short int onu_id, int fla)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_MULTICAST_FASTLEAVE_51, &fla, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMulticastFastleave(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}


/*获取ONU的端口统计数据*/
static int RPC_GetPortStatisticData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
    int iRlt;
    int cfg_slot;


    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_PORT_STATISTIC_DATA_52, &port_id, sizeof(port_id), data, sizeof(CTC_STACK_statistic_data_t), 0, 0);
 
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPortStatisticData(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCTCOnuPortStatsData(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_data_t *data)
{
	int iRlt;
    int cfg_slot;


    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
  
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_PORT_STATS_DATA_91,&port_id, sizeof(port_id), data, sizeof(OnuPortStats_ST), 0, 0);
 
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCTCOnuPortStatsData(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;

}

/*获取ONU的端口统计状态*/
static int RPC_GetPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt;
    int cfg_slot;


    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_PORT_STATISTIC_STATE_53, &port_id, sizeof(port_id), state, sizeof(CTC_STACK_statistic_state_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPortStatisticState(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

/*设置ONU的端口统计状态*/
static int RPC_SetPortStatisticState(short int olt_id, short int onu_id, int port_id, CTC_STACK_statistic_state_t *state)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(port_id)+sizeof(CTC_STACK_statistic_state_t)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));
    VOS_MemCpy(&CmdParam[sizeof(port_id)], state, sizeof(CTC_STACK_statistic_state_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_PORT_STATISTIC_STATE_54, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortStatisticState(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAlarmAdminState(short int olt_id, short int onu_id,  CTC_management_object_t *management_object,
												 CTC_STACK_alarm_id_t alarm_id, bool enable)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(CTC_STACK_alarm_id_t)+sizeof(bool)+sizeof(CTC_management_object_t)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &enable, sizeof(enable));
    VOS_MemCpy(&CmdParam[sizeof(enable)], &alarm_id, sizeof(alarm_id));
    VOS_MemCpy(&CmdParam[sizeof(alarm_id) + sizeof(enable)], management_object, sizeof(CTC_management_object_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ALARM_ADMIN_STATE_55, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAlarmAdminState(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, alarm_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAlarmThreshold (short int olt_id, short int onu_id, CTC_management_object_t *management_object,
			CTC_STACK_alarm_id_t alarm_id, unsigned long alarm_threshold, unsigned long	clear_threshold )
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(CTC_STACK_alarm_id_t) + sizeof(unsigned long)*2 + sizeof(CTC_management_object_t)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &alarm_threshold, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[sizeof(unsigned long)], &clear_threshold, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[sizeof(unsigned long)*2], &alarm_id, sizeof(alarm_id));
    VOS_MemCpy(&CmdParam[sizeof(alarm_id) + sizeof(unsigned long)*2], management_object, sizeof(CTC_management_object_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ALARM_THRESOLD_56, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAlarmThreshold(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, alarm_id, iRlt, cfg_slot);

    return iRlt;
}


/*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
static int RPC_GetMxuMngGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_MNG_GLOBAL_CONFIG_59, NULL, 0, mxu_mng, sizeof(*mxu_mng), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMxuMngGlobalParameter(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMxuMngGlobalConfig(short int olt_id, short int onu_id, RPC_CTC_mxu_mng_global_parameter_config_t *mxu_mng)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_MNG_GLOBAL_CONFIG_60, mxu_mng, sizeof(*mxu_mng), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMxuMngGlobalParameter(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMxuMngSnmpConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_MNG_SNMP_CONFIG_61, NULL, 0, parameter, sizeof(*parameter), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMxuMngSnmpConfig(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMxuMngSnmpConfig(short int olt_id, short int onu_id, CTC_STACK_mxu_mng_snmp_parameter_config_t *parameter)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_MNG_SNMP_CONFIG_62, parameter, sizeof(*parameter), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMxuMngSnmpConfig(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(holdover);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_HOLDOVER_63, NULL, 0, holdover, sizeof(CTC_STACK_holdover_state_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetHoldOver(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetHoldOver(short int olt_id, short int onu_id, CTC_STACK_holdover_state_t *holdover)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_HOLDOVER_64, holdover, sizeof(CTC_STACK_holdover_state_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetHoldOver(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, holdover->holdover_state, holdover->holdover_time, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOptTransDiag( short int olt_id, short int onu_id, CTC_STACK_optical_transceiver_diagnosis_t	*parameter )
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_OPTIC_TRAN_DIAG_65, NULL, 0, parameter, sizeof(*parameter), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOptTransDiag(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t *parameter)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_SET_ONU_POWER_SUPPLY_CONTROL_66, parameter, sizeof(*parameter), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetTxPowerSupplyControl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetFecAbility(short int olt_id, short int onu_id, CTC_STACK_standard_FEC_ability_t *fec_ability)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CTC_GET_ONU_FEC_ABILITY_67, NULL, 0, fec_ability, sizeof(*fec_ability),  0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetFecAbility(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id,  iRlt, cfg_slot);

	return iRlt;
}


#if 1
static int RPC_GetIADInfo(short int olt_id, short int onu_id, CTC_STACK_voip_iad_info_t *iad_info)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_IADINFO_68, NULL, 0, iad_info, sizeof(CTC_STACK_voip_iad_info_t), 0, 0);

    return iRlt;
}

/*H.248协议下IAD的运行状态*/
static int RPC_GetVoipIadOperStatus(short int olt_id, short int onu_id, CTC_STACK_voip_iad_oper_status_t *iad_oper_status)
{
    int iRlt, cfg_slot;
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_IAD_OPER_69, NULL, 0, iad_oper_status, sizeof(CTC_STACK_voip_iad_oper_status_t), 0, 0);

    return iRlt;
}

static int RPC_SetVoipIadOperation(short int olt_id, short int onu_id, CTC_STACK_operation_type_t operation_type)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VOIP_IAD_OPER_70, &operation_type, sizeof(CTC_STACK_operation_type_t), NULL, 0, 0, 0);

    return iRlt;
}

/*语音模块全局参数配置*/
static int RPC_GetVoipGlobalConfig(short int olt_id, short int onu_id, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_GLOBAL_CONFIG_71, NULL, 0, global_param, sizeof(CTC_STACK_voip_global_param_conf_t), 0, 0);

    return iRlt;
}

static int RPC_SetVoipGlobalConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_global_param_conf_t *global_param)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(code) + sizeof(*global_param)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &code, sizeof(code));    
    VOS_MemCpy(&CmdParam[sizeof(code)], global_param, sizeof(*global_param));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VOIP_GLOBAL_CONFIG_72, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_GetVoipFaxConfig(short int olt_id, short int onu_id, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_FAX_CONFIG_73, NULL, 0, voip_fax, sizeof(CTC_STACK_voip_fax_config_t), 0, 0);

    return iRlt;
}

static int RPC_SetVoipFaxConfig(short int olt_id, short int onu_id, int code, CTC_STACK_voip_fax_config_t *voip_fax)
{
    int iRlt, cfg_slot;    
    unsigned char CmdParam[sizeof(code) + sizeof(*voip_fax)];
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    VOS_MemCpy(&CmdParam[0], &code, sizeof(code));    
    VOS_MemCpy(&CmdParam[sizeof(code)], voip_fax, sizeof(*voip_fax));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VOIP_FAX_CONFIG_74, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_GetVoipPortStatus(short int olt_id, short int onu_id, int port_id, CTC_STACK_voip_pots_status_array *pots_status_array)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_POTS_STATUS_75, &port_id, sizeof(port_id), pots_status_array, sizeof(CTC_STACK_voip_pots_status_array), 0, 0);

    return iRlt;
}

static int RPC_GetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t  *port_state)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_PORT_OPER_76, &port_id, sizeof(int), port_state, sizeof(CTC_STACK_on_off_state_t), 0, 0);

    return iRlt;    
}

static int RPC_SetVoipPort(short int olt_id, short int onu_id, int port_id, CTC_STACK_on_off_state_t port_state)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = port_state;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VOIP_PORT_OPER_77, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_GetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t *port_state)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = slot;
    CmdParam[1] = port;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_VOIP_PORT2_OPER_78, CmdParam, sizeof(CmdParam), port_state, sizeof(CTC_STACK_on_off_state_t), 0, 0);

    return iRlt;
}

static int RPC_SetVoipPort2(short int olt_id, short int onu_id, int slot, int port, CTC_STACK_on_off_state_t port_state)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    CmdParam[0] = slot;
    CmdParam[1] = port;
    CmdParam[2] = port_state;

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VOIP_PORT2_OPER_79, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}


static int RPC_GetH248Config(short int olt_id, short int onu_id, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_H248_CONFIG_80, NULL, 0, h248_param, sizeof(CTC_STACK_h248_param_config_t), 0, 0);

    return iRlt;
}

static int RPC_SetH248Config(short int olt_id, short int onu_id, int code, CTC_STACK_h248_param_config_t *h248_param)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(code) + sizeof(*h248_param)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &code, sizeof(code));    
    VOS_MemCpy(&CmdParam[sizeof(code)], h248_param, sizeof(*h248_param));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_H248_CONFIG_81, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_GetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_array *h248_user_tid_array)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_H248_USERTID_CONFIG_82, &port_id, sizeof(port_id), h248_user_tid_array, sizeof(CTC_STACK_h248_user_tid_config_array), 0, 0);

    return iRlt;
}

static int RPC_SetH248UserTidConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_h248_user_tid_config_t *user_tid_config)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(*user_tid_config)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));    
    VOS_MemCpy(&CmdParam[sizeof(port_id)], user_tid_config, sizeof(*user_tid_config));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_H248_USERTID_CONFIG_83, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_GetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_info_t *h248_rtp_tid_info)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_H248_RTPTID_CONFIG_84, NULL, 0, h248_rtp_tid_info, sizeof(CTC_STACK_h248_rtp_tid_info_t), 0, 0);

    return iRlt;
}

static int RPC_SetH248RtpTidConfig(short int olt_id, short int onu_id, CTC_STACK_h248_rtp_tid_config_t *h248_rtp_tid_info)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_H248_RTPTID_CONFIG_85, h248_rtp_tid_info, sizeof(CTC_STACK_h248_rtp_tid_config_t), NULL, 0, 0, 0);

    return iRlt;
}


static int RPC_GetSipConfig(short int olt_id, short int onu_id, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_SIP_CONFIG_86, NULL, 0, sip_param, sizeof(CTC_STACK_sip_param_config_t), 0, 0);

    return iRlt;
}

static int RPC_SetSipConfig(short int olt_id, short int onu_id, int code, CTC_STACK_sip_param_config_t *sip_param)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(code) + sizeof(*sip_param)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &code, sizeof(code));    
    VOS_MemCpy(&CmdParam[sizeof(code)], sip_param, sizeof(*sip_param));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_SIP_CONFIG_87, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}

static int RPC_SetSipDigitMap(short int olt_id, short int onu_id, CTC_STACK_SIP_digit_map_t *sip_digit_map)
{
    int iRlt, cfg_slot;
    unsigned long ulParamSize;
    unsigned char CmdParam[2050];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( sizeof(CmdParam) - sizeof(ulParamSize) >= (ulParamSize = sip_digit_map->digital_map_length) )
    {
        VOS_MemCpy(&CmdParam[0], &ulParamSize, sizeof(ulParamSize));
        VOS_MemCpy(&CmdParam[sizeof(ulParamSize)], sip_digit_map->digital_map, ulParamSize);
        
        iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_SIP_DIGITMAP_88, CmdParam, ulParamSize + sizeof(ulParamSize), NULL, 0, 0, 0);
    }
    else
    {
        iRlt = OLT_ERR_PARAM;
    }

    return iRlt;
}

static int RPC_GetSipUserConfig(short int olt_id, short int onu_id, int port_id, CTC_STACK_sip_user_param_config_array *sip_user_param_array)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_GET_SIP_USERTID_CONFIG_89, &port_id, sizeof(port_id), sip_user_param_array, sizeof(CTC_STACK_sip_user_param_config_array), 0, 0);

    return iRlt;
}

static int RPC_SetSipUserConfig(short int olt_id, short int onu_id, int port_id, int code, CTC_STACK_sip_user_param_config_t *sip_user_param)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(port_id) + sizeof(code) + sizeof(*sip_user_param)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &port_id, sizeof(port_id));    
    VOS_MemCpy(&CmdParam[sizeof(port_id)], &code, sizeof(code));    
    VOS_MemCpy(&CmdParam[sizeof(port_id) + sizeof(code)], sip_user_param, sizeof(*sip_user_param));    

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_SIP_USERTID_CONFIG_90, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    return iRlt;
}
#endif
#endif


#if 1
/* -------------------ONU 远程管理API------------------- */

static int RPC_CliCall(short int olt_id, short int onu_id, char *cli_str, int cli_len, char **rlt_str, unsigned short int *rlt_len)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    cli_recvbuf[0] = '\0';
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CLI_CALL_1, cli_str, cli_len, cli_recvbuf, sizeof(cli_recvbuf), 0, 0);
    if ( 0 == iRlt )
    {
        if ( cli_recvbuf[0] == '\0' )
        {
            VOS_StrCpy(cli_recvbuf, "\r\n");
        }

        *rlt_str = cli_recvbuf;
        *rlt_len = VOS_StrLen(cli_recvbuf);
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CliCall(%d, %d)[%s]'s result(%d) to slot %d.\r\n", olt_id, onu_id, cli_str, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMgtReset(short int olt_id, short int onu_id, int lv)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_MGT_RESET_2, &lv, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMgtReset(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, lv, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMgtConfig(short int olt_id, short int onu_id, int lv)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_MGT_CONFIG_3, &lv, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMgtConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, lv, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMgtLaser(short int olt_id, short int onu_id, int lv)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_MGT_LASER_4, &lv, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMgtLaser(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, lv, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetTemperature(short int olt_id, short int onu_id, int temp)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_TEMPERATURE_5, &temp, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetTemperature(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, temp, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPasFlush(short int olt_id, short int onu_id, int act)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PAS_FLUSH_6, &act, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPasFlush(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, act, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetAtuAgingTime(short int olt_id, short int onu_id, int aging)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ATU_AGING_TIME_7, &aging, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAtuAgingTime(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, aging, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAtuLimit(short int olt_id, short int onu_id, int limit)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_ATU_LIMIT_8, &limit, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAtuLimit(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, limit, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortLinkMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_LINK_MON_9, &mon, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortLinkMon(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, mon, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortModeMon(short int olt_id, short int onu_id, int mon)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MODE_MON_10, &mon, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortModeMon(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, mon, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortIsolate(short int olt_id, short int onu_id, int enable)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_ISOLATE_11, &enable, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortIsolate(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, enable, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetVlanEnable(short int olt_id, short int onu_id, int enable)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VLAN_ENABLE_12, &enable, sizeof(int), NULL, 0, 1200000, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVlanEnable(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, enable, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetVlanMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VLAN_MODE_13, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVlanMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ADD_VLAN_14, &vid, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddVlan(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelVlan(short int olt_id, short int onu_id, int vid)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_VLAN_15, &vid, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelVlan(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortPvid(short int olt_id, short int onu_id, int port_id, int pvid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = pvid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_PVID_16, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortPvid(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_AddVlanPort(short int olt_id, short int onu_id, int vid, int portmask, int tag)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = vid;
    CmdParam[1] = portmask;
    CmdParam[2] = tag;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ADD_VLAN_PORT_17, CmdParam, sizeof(CmdParam), NULL, 0, 1200000, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddVlanPort(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelVlanPort(short int olt_id, short int onu_id, int vid, int portmask)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = vid;
    CmdParam[1] = portmask;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_VLAN_PORT_18, CmdParam, sizeof(CmdParam), NULL, 0, 1200000, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelVlanPort(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid, ULONG newVid)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = inVid;
    CmdParam[2] = newVid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VLAN_TRANSATION_19, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortVlanTran(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelEthPortVlanTran(short int olt_id, short int onu_id, int port_id, ULONG inVid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = inVid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_VLAN_TRANSATION_20, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelEthPortVlanTran(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetEthPortVlanAgg(short int olt_id, short int onu_id, int port_id, USHORT inVid[8], USHORT targetVid)
{
    int iRlt, cfg_slot;
    USHORT CmdParam[10];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = (USHORT)port_id;
    VOS_MemCpy(&CmdParam[1], inVid, sizeof(USHORT)*8);
    CmdParam[9] = targetVid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_VLAN_AGGREGATION_21, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEthPortVlanAgg(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelEthPortVlanAgg(short int olt_id, short int onu_id, int port_id, ULONG targetVid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = targetVid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_VLAN_AGGREGATION_22, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelEthPortVlanAgg(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortQinQEnable(short int olt_id,short int onu_id, int port_id, int enable )
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QINQ_ENABLE_23, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortQinQEnable(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG cvlan, ULONG svlan)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = cvlan;
    CmdParam[2] = svlan;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ADD_QINQ_TAG_24, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddQinQVlanTag(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelQinQVlanTag(short int olt_id, short int onu_id, int port_id, ULONG svlan)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = svlan;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_QINQ_TAG_25, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelQinQVlanTag(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortVlanFrameTypeAcc(short int olt_id, short int onu_id, int port_id, int acc)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = acc;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_VLAN_FRAME_ACC_26, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortVlanFrameTypeAcc(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortIngressVlanFilter(short int olt_id, short int onu_id, int port_id, int enable)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_INGRESS_VLAN_FILTER_27, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortIngressVlanFilter(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortMode(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MODE_28, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortFcMode(short int olt_id, short int onu_id, int port_id, int fc)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = fc;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_FC_MODE_29, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortFcMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortAtuLearn(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = portlist;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_ATU_LEARN_30, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortAtuLearn(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, portlist, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortAtuFlood(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = portlist;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_ATU_FLOOD_31, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortAtuFlood(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, portlist, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortLoopDetect(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = portlist;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_LOOP_DETECT_32, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortLoopDetect(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, portlist, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortStatFlush(short int olt_id, short int onu_id, int portlist, int enable)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = portlist;
    CmdParam[1] = enable;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_STAT_FLUSH_33, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortStatFlush(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, portlist, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetIngressRateLimitBase(short int olt_id, short int onu_id, int uv)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_INGRESS_RATE_LIMIT_BASE_34, &uv, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIngressRateLimitBase(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, uv, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortIngressRate(short int olt_id, short int onu_id, int port_id, int type, int rate, int action, int burstmode)
{
    int iRlt, cfg_slot;
    int CmdParam[5];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = type;
    CmdParam[2] = rate;
    CmdParam[3] = action;
    CmdParam[4] = burstmode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_INGRESS_RATE_35, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortIngressRate(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortEgressRate(short int olt_id, short int onu_id, int port_id, int rate)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = rate;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_EGRESS_RATE_36, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortEgressRate(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetQosClass(short int olt_id, short int onu_id, int qossetid,  int ruleid, int classid, int field, int oper, char *val, int len)
{
    int iRlt, cfg_slot;
    char CmdParam[sizeof(int) * 6 + sizeof(qos_value_t)];
    int *piParam = (int*)CmdParam;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(len <= sizeof(qos_value_t));

    *piParam++ = qossetid;
    *piParam++ = ruleid;
    *piParam++ = classid;
    *piParam++ = field;
    *piParam++ = oper;
    *piParam++ = len;
    VOS_MemCpy(piParam, val, len);
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QOS_CLASS_37, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetQosClass(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, qossetid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClrQosClass(short int olt_id, short int onu_id, int qossetid, int ruleid, int classid)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = qossetid;
    CmdParam[1] = ruleid;
    CmdParam[2] = classid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CLR_QOS_CLASS_38, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClrQosClass(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, qossetid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid, int queue, int prio)
{
    int iRlt, cfg_slot;
    int CmdParam[4];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = qossetid;
    CmdParam[1] = ruleid;
    CmdParam[2] = queue;
    CmdParam[3] = prio;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QOS_RULE_39, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetQosRule(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, qossetid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClrQosRule(short int olt_id, short int onu_id, int qossetid, int ruleid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = qossetid;
    CmdParam[1] = ruleid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CLR_QOS_RULE_40, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClrQosRule(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, qossetid, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = qossetid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_QOS_RULE_41, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortQosRule(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClrPortQosRule(short int olt_id, short int onu_id, int port_id, int qossetid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = qossetid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CLR_PORT_QOS_RULE_42, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClrPortQosRule(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortQosRuleType(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_QOS_RULE_TYPE_43, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortQosRuleType(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortDefPriority(short int olt_id, short int onu_id, int port_id, int prio)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = prio;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_DEF_PRIORITY_44, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortDefPriority(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortNewPriority(short int olt_id, short int onu_id, int port_id, int oldprio, int newprio)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = oldprio;
    CmdParam[2] = newprio;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_NEW_PRIORITY_45, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortNewPriority(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetQosPrioToQueue(short int olt_id, short int onu_id, int prio, int queue)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = prio;
    CmdParam[1] = queue;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QOS_PRIORITY_TO_QUEUE_46, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetQosPrioToQueue(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, prio, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetQosDscpToQueue(short int olt_id, short int onu_id, int dscpnum, int queue)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = dscpnum;
    CmdParam[1] = queue;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QOS_DSCP_TO_QUEUE_47, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetQosDscpToQueue(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, dscpnum, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortUserPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_QOS_USERPRI_ENABLE_48, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortUserPriorityEnable(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortIpPriorityEnable(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_QOS_IPPRI_ENABLE_49, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortIpPriorityEnable(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetQosAlgorithm(short int olt_id, short int onu_id, int uv)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_QOS_ALGORITHM_50, &uv, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetQosAlgorithm(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, uv, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GWONU_Set_QosMode(short int olt_id, short int onu_id, int direct, unsigned char mode)
{
    int iRlt = VOS_ERROR, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    CmdParam[0] = direct;
    CmdParam[1] = mode;        
        
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_SET_QOS_MODE_51, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    return iRlt;
}
static int RPC_GWONU_Set_Rule(short int olt_id, short int onu_id, int direct, int code, gw_rule_t rule)
{
    int iRlt = VOS_ERROR, cfg_slot;
    unsigned char CmdParam[sizeof(direct)+sizeof(code)+sizeof(rule)];
    
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    
    VOS_MemCpy(&CmdParam[0], &direct, sizeof(direct));
    VOS_MemCpy(&CmdParam[4], &code, sizeof(code));
    VOS_MemCpy(&CmdParam[8], &rule, sizeof(rule));
                
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot,ONU_CMD_SET_RULE_52, CmdParam, sizeof(direct)+sizeof(code)+sizeof(rule), NULL, 0, 0, 0);
    return iRlt;    
}

static int RPC_SetIgmpEnable(short int olt_id, short int onu_id, int en)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_ENABLE_53, &en, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpEnable(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, en, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpAuth(short int olt_id, short int onu_id, int en)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_AUTH_54, &en, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpAuth(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, en, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpHostAge(short int olt_id, short int onu_id, int age)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_HOST_AGE_55, &age, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpHostAge(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, age, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpGroupAge(short int olt_id, short int onu_id, int age)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_GROUP_AGE_56, &age, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpGroupAge(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, age, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpMaxResTime(short int olt_id, short int onu_id, int tm)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_MAX_RESTIME_57, &tm, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpMaxResTime(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, tm, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpMaxGroup(short int olt_id, short int onu_id, int number)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_IGMP_MAX_GROUP_58, &number, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpMaxGroup(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, number, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddIgmpGroup(short int olt_id, short int onu_id, int portlist, ULONG addr, ULONG vid)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = portlist;
    CmdParam[1] = addr;
    CmdParam[1] = vid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_ADD_IGMP_GROUP_59, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddIgmpGroup(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, portlist, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DeleteIgmpGroup(short int olt_id, short int onu_id, ULONG addr)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_IGMP_GROUP_60, &addr, sizeof(ULONG), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DeleteIgmpGroup(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, addr, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortIgmpFastLeave(short int olt_id, short int onu_id, int port_id, int mode)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MULTICAST_FASTLEAVE_61, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortIgmpFastLeave(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortMulticastVlan(short int olt_id, short int onu_id, int port_id, int vid)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = vid;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MULTICAST_VLAN_62, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortMulticastVlan(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetPortMirrorFrom(short int olt_id, short int onu_id, int port_id, int mode, int type)
{
    int iRlt, cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = mode;
    CmdParam[2] = type;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MIRROR_FROM_63, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortMirrorFrom(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPortMirrorTo(short int olt_id, short int onu_id, int port_id, int type)
{
    int iRlt, cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    CmdParam[0] = port_id;
    CmdParam[1] = type;
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SET_PORT_MIRROR_TO_64, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPortMirrorTo(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, port_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DeleteMirror(short int olt_id, short int onu_id, int type)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_DEL_MIRROR_65, &type, sizeof(int), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DeleteMirror(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, type, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC协议管理API------------------- */

#if 1
/* --------------------CMC管理API------------------- */
static int RPC_RegisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_REGISTER_CMC_1, cmc_mac, sizeof(mac_address_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RegisterCmc(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_UnregisterCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_UNREGISTER_CMC_2, cmc_mac, sizeof(mac_address_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_UnregisterCmc(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmc(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_3, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmc(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpAlarms(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_ALARM_4, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpAlarms(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLogs(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_LOG_5, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLogs(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetCmcBoard(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RESET_CMC_BOARD_6, cmc_mac, sizeof(mac_address_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetCmcBoard(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcVersion(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *version, unsigned char *len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], len, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_VERSION_7, CmdParam, sizeof(CmdParam), version, *len, 0, 0)) )
    {
        *len = (unsigned char)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcVersion(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcMaxMulticasts(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_multicasts)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_MAXMULTICAST_8, cmc_mac, sizeof(mac_address_t), max_multicasts, sizeof(*max_multicasts), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcMaxMulticasts(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *max_cm)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_MAXCM_9, cmc_mac, sizeof(mac_address_t), max_cm, sizeof(*max_cm), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcMaxCm(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *max_cm, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcMaxCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short max_cm)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &max_cm, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_MAXCM_10, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcMaxCm(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, max_cm, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_TIME_11, cmc_mac, sizeof(mac_address_t), time, sizeof(*time), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcTime(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, struct tm *time)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(struct tm)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], time, sizeof(struct tm));
    VOS_MemCpy(&CmdParam[sizeof(struct tm)], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_TIME_12, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcTime(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_LocalCmcTime(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_LOCAL_CMC_TIME_13, cmc_mac, sizeof(mac_address_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcMaxCm(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, unsigned char *cfg_data, unsigned short data_len)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char) * 66];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( data_len > 64 )
    {
        VOS_ASSERT(0);
        return OLT_ERR_PARAM;
    }

    VOS_MemCpy(&CmdParam[0], &data_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &cfg_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[9], cfg_data, data_len);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_CUSTOM_CONFIG_14, CmdParam, data_len + 9, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcCustomConfig(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcCustomConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char cfg_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &cfg_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_CUSTOM_CONFIG_15, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcCustomConfig(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, cfg_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}
#endif

#if 1
/* --------------------CMC频道管理API------------------- */
static int RPC_DumpCmcDownChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_16, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcDownChannel(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcUpChannel(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_17, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcUpChannel(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODE_18, CmdParam, sizeof(CmdParam), channel_mode, sizeof(unsigned char) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &channel_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODE_19, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_MODE_20, CmdParam, sizeof(CmdParam), channel_mode, sizeof(unsigned char) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &channel_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_MODE_21, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *docsis30_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_D30MODE_22, CmdParam, sizeof(CmdParam), docsis30_mode, sizeof(unsigned char) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *docsis30_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelD30Mode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char docsis30_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &docsis30_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_D30MODE_23, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelD30Mode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, docsis30_mode, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_FREQ_24, CmdParam, sizeof(CmdParam), channel_freq, sizeof(unsigned long) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[5], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_FREQ_25, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelFreq(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_freq)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_FREQ_26, CmdParam, sizeof(CmdParam), channel_freq, sizeof(unsigned long) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_freq, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_freq)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[5], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_FREQ_27, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelFreq(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_freq, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetCmcDownAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(unsigned long) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &base_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &step_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[8], &step_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[9], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_AUTO_FREQ_29, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpAutoFreq(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long base_freq, unsigned long step_freq, unsigned char step_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(unsigned long) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &base_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &step_freq, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[8], &step_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[9], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_AUTO_FREQ_31, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpAutoFreq(%d, %d, %lu, %lu, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, base_freq, step_freq, step_mode, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long *channel_width)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_WIDTH_32, CmdParam, sizeof(CmdParam), channel_width, sizeof(unsigned long) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_width, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelWidth(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned long channel_width)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_width, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[5], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_WIDTH_33, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelWidth(%d, %d, %d, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_width, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *annex_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_ANNEXMODE_34, CmdParam, sizeof(CmdParam), annex_mode, sizeof(unsigned char) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *annex_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelAnnexMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char annex_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &annex_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_ANNEXMODE_35, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelAnnexMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, annex_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_type)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_TYPE_36, CmdParam, sizeof(CmdParam), channel_type, sizeof(unsigned char) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelType(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_type)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &channel_type, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_TYPE_37, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *modulation_type)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_MODULATION_38, CmdParam, sizeof(CmdParam), modulation_type, sizeof(unsigned char) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *modulation_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelModulation(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char modulation_type)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &modulation_type, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_MODULATION_39, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelModulation(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, modulation_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *channel_profile)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_PROFILE_40, CmdParam, sizeof(CmdParam), channel_profile, sizeof(unsigned char) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_profile, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelProfile(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char channel_profile)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &channel_profile, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_PROFILE_41, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelProfile(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_profile, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char *interleaver)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_INTERLEAVER_42, CmdParam, sizeof(CmdParam), interleaver, sizeof(unsigned char) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *interleaver, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelInterleaver(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, unsigned char interleaver)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &interleaver, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_INTERLEAVER_43, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelInterleaver(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, interleaver, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_DOWN_CHANNEL_POWER_44, CmdParam, sizeof(CmdParam), channel_power, sizeof(short int) * CMC_MAX_DS_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcDownChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(short int)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_power, sizeof(short int));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_DOWN_CHANNEL_POWER_45, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcDownChannelPower(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int *channel_power)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_UP_CHANNEL_POWER_46, CmdParam, sizeof(CmdParam), channel_power, sizeof(short int) * CMC_MAX_US_CH, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *channel_power, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, short int channel_power)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) + sizeof(short int)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &channel_power, sizeof(short int));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_CMC_UP_CHANNEL_POWER_47, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, channel_power, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcUpChannelPower(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_POWER_48, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcUpChannelPower(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcUpChannelSignalQuality(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_SIGNAL_QUALITY_49, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcUpChannelSignalQuality(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcInterfaceUtilization(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_type, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_UTILITY_50, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcInterfaceUtilization(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcInterfaceStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char channel_type, unsigned char channel_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &channel_type, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], &channel_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_IF_CHANNEL_STATISTICS_51, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcInterfaceStatistics(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, channel_type, channel_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_DumpCmcMacStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_IF_MAC_STATISTICS_52, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcMacStatistics(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcAllInterface(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_IF_ALL_STATISTICS_53, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcAllInterface(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}
#endif

#if 1
/* --------------------CMC频道组管理API------------------- */
static int RPC_DumpAllLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_GROUP_ALL_54, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpAllLoadBalancingGrp(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_GROUP_55, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_DOWN_CHANNEL_56, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_UP_CHANNEL_57, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingDynConfig(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_DYNAMIC_CONFIG_58, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingDynConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetLoadBalancingDynMethod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char method)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &method, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_METHOD_59, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynMethod(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, method, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynPeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &period, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_PERIOD_60, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynPeriod(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, period, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynWeightedAveragePeriod(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long period)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &period, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_WEIGHT_PERIOD_61, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynWeightedAveragePeriod(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, period, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynOverloadThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &percent, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_OVERLOAD_THRESOLD_62, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynOverloadThresold(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, percent, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynDifferenceThresold(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char percent)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &percent, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_DIFF_THRESOLD_63, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynDifferenceThresold(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, percent, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynMaxMoveNumber(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long max_move)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &max_move, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_MAX_MOVE_64, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynMaxMoveNumber(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, max_move, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynMinHoldTime(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned long hold_time)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned long)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &hold_time, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_MIN_HOLD_65, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynMinHoldTime(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, hold_time, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynRangeOverrideMode(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char range_mode)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &range_mode, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_RANGE_MODE_66, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynRangeOverrideMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, range_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynAtdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &tech_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DCC_TECH_67, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynAtdmaDccInitTech(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynScdmaDccInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &tech_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DCC_TECH_68, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynScdmaDccInitTech(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynAtdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &tech_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_ATDMA_DBC_TECH_69, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynAtdmaDbcInitTech(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLoadBalancingDynScdmaDbcInitTech(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char tech_id)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &tech_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_GROUP_DYNAMIC_SCDMA_DBC_TECH_70, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLoadBalancingDynScdmaDbcInitTech(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, tech_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_CreateLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char grp_method)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &grp_method, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_CREATE_GROUP_71, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CreateLoadBalancingGrp(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, grp_method, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DestroyLoadBalancingGrp(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DESTROY_GROUP_72, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DestroyLoadBalancingGrp(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_AddLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 18];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_ADD_GROUP_DOWN_CHANNEL_73, CmdParam, num_of_ch + 8, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveLoadBalancingGrpDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 18];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RMV_GROUP_DOWN_CHANNEL_74, CmdParam, num_of_ch + 8, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveLoadBalancingGrpDownstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 6];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_ADD_GROUP_UP_CHANNEL_75, CmdParam, num_of_ch + 8, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveLoadBalancingGrpUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned char) * 6];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RMV_GROUP_UP_CHANNEL_76, CmdParam, num_of_ch + 8, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveLoadBalancingGrpUpstream(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_AddLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 3 + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], mac_start, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[7], mac_end, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[13], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_ADD_GROUP_MODEM_77, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddLoadBalancingGrpModem(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 3 + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], mac_start, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[7], mac_end, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[13], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RMV_GROUP_MODEM_78, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveLoadBalancingGrpModem(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], mac_start, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[6], mac_end, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[12], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_ADD_GROUP_EXCLUDE_MODEM_79, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t mac_start, mac_address_t mac_end)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 3];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], mac_start, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[6], mac_end, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[12], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RMV_GROUP_EXCLUDE_MODEM_80, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveLoadBalancingGrpExcludeModem(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_DumpLoadBalancingGrpModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_MODEM_81, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpModem(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrpActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char grp_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &grp_id, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_ACTIVE_MODEM_82, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpActivedModem(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, grp_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrpExcludeModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_MODEM_83, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpExcludeModem(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpLoadBalancingGrpExcludeActivedModem(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_GROUP_EXCLUDE_ACTIVE_MODEM_84, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpLoadBalancingGrpExcludeActivedModem(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}
#endif

#if 1
/* --------------------CM管理API------------------- */
static int RPC_DumpAllCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_ALL_MODEM_86, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpAllCm(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_87, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCm(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_ALL_MODEM_HISTORY_88, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpAllCmHistory(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_HISTORY_89, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmHistory(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClearAllCmHistory(short int olt_id, short int onu_id, mac_address_t cmc_mac)
{
    int iRlt, cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_CLEAR_ALL_MODEM_HISTORY_90, cmc_mac, sizeof(mac_address_t), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClearAllCmHistory(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetCm(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[6], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RESET_MODEM_91, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetCm(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_DOWN_CHANNEL_92, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmDownstream(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_UP_CHANNEL_93, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmUpstream(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmDownstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned char) * 17];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[7], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[13], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_MODEM_DOWN_CHANNEL_94, CmdParam, num_of_ch + 13, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmDownstream(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmUpstream(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char num_of_ch, unsigned char *ch_ids)
{
    int iRlt, cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned char) * 5];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &num_of_ch, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[1], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[7], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[13], ch_ids, num_of_ch);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SET_MODEM_UP_CHANNEL_95, CmdParam, num_of_ch + 13, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmUpstream(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, num_of_ch, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_CreateCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned char cos, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    int cfg_slot;
    unsigned long ulParamLen;
    unsigned char CmdParam[2048];
    unsigned char *pCmdParam;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ulParamLen = sizeof(mac_address_t) * 2 + sizeof(unsigned char) * (tlv_len + 1) + sizeof(unsigned short);
    if ( ulParamLen > sizeof(CmdParam) )
    {
        if ( NULL == (pCmdParam = VOS_Malloc(ulParamLen, MODULE_RPU_ONU)) )
        {
            return OLT_ERR_MALLOC;
        }
    }
    else
    {
        pCmdParam = CmdParam;
    }
    
    VOS_MemCpy(&CmdParam[0], &tlv_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &cos, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[3], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[9], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[15], tlv_data, tlv_len);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_CREATE_MODEM_SERVICE_FLOW_96, pCmdParam, ulParamLen, NULL, 0, 0, 0);

    if ( pCmdParam != CmdParam )
    {
        VOS_Free(pCmdParam);
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CreateCmServiceFlow(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, cos, tlv_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ModifyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    int cfg_slot;
    unsigned long ulParamLen;
    unsigned long ulParamTmp;
    unsigned char CmdParam[2048];
    unsigned char *pCmdParam;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ulParamLen = sizeof(mac_address_t) * 2 + sizeof(unsigned long) * 3 + sizeof(unsigned char) * tlv_len;
    if ( ulParamLen > sizeof(CmdParam) )
    {
        if ( NULL == (pCmdParam = VOS_Malloc(ulParamLen, MODULE_RPU_ONU)) )
        {
            return OLT_ERR_MALLOC;
        }
    }
    else
    {
        pCmdParam = CmdParam;
    }

    ulParamTmp = tlv_len;
    VOS_MemCpy(&CmdParam[0], &ulParamTmp, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &usfid, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[8], &dsfid, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[12], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[18], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[24], tlv_data, tlv_len);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_MODIFY_MODEM_SERVICE_FLOW_97, pCmdParam, ulParamLen, NULL, 0, 0, 0);

    if ( pCmdParam != CmdParam )
    {
        VOS_Free(pCmdParam);
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ModifyCmServiceFlow(%d, %d, %lu, %lu, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, usfid, dsfid, tlv_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DestroyCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned long usfid, unsigned long dsfid)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned long) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &usfid, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[4], &dsfid, sizeof(unsigned long));
    VOS_MemCpy(&CmdParam[8], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[14], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DESTROY_MODEM_SERVICE_FLOW_98, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DestroyCmServiceFlow(%d, %d, %lu, %lu)'s result(%d) to slot %d.\r\n", olt_id, onu_id, usfid, dsfid, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_DumpCmClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_CLASSIFIER_99, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmClassifier(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_MODEM_SERVICE_FLOW_100, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmServiceFlow(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}
#endif


#if 1
/* --------------------QoS管理API------------------- */
static int RPC_DumpCmcClassifier(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt, cfg_slot;
    unsigned long ulParamLen;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) * 2 + sizeof(unsigned long) * 8];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ulParamLen = num_of_sf * sizeof(unsigned long);
    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &num_of_sf, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[4], sf_ids, ulParamLen);
    VOS_MemCpy(&CmdParam[ulParamLen + 4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_CLASSIFIER_101, CmdParam, ulParamLen + 10, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcClassifier(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcServiceFlow(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt, cfg_slot;
    unsigned long ulParamLen;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) * 2 + sizeof(unsigned long) * 8];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ulParamLen = num_of_sf * sizeof(unsigned long);
    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &num_of_sf, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[4], sf_ids, ulParamLen);
    VOS_MemCpy(&CmdParam[ulParamLen + 4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_102, CmdParam, ulParamLen + 10, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcServiceFlow(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcServiceFlowStatistics(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short num_of_sf, unsigned long *sf_ids, char *dump_buf, unsigned short *dump_len)
{
    int iRlt, cfg_slot;
    unsigned long ulParamLen;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short) * 2 + sizeof(unsigned long) * 8];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    ulParamLen = num_of_sf * sizeof(unsigned long);
    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], &num_of_sf, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[4], sf_ids, ulParamLen);
    VOS_MemCpy(&CmdParam[ulParamLen + 4], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_SERVICE_FLOW_STATISTICS_103, CmdParam, ulParamLen + 10, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcServiceFlowStatistics(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, num_of_sf, *dump_len, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_DumpCmcDownChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_DOWN_CHANNEL_GROUP_104, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcDownChannelBondingGroup(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DumpCmcUpChannelBondingGroup(short int olt_id, short int onu_id, mac_address_t cmc_mac, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], dump_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));

    if ( 0 <= (iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DUMP_CMC_UP_CHANNEL_GROUP_105, CmdParam, sizeof(CmdParam), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpCmcUpChannelBondingGroup(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_CreateCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name, char *tlv_data, unsigned short tlv_len)
{
    int iRlt;
    int cfg_slot;
    unsigned short usParamLen;
    unsigned char CmdParam[sizeof(mac_address_t) + 512 + sizeof(unsigned short) * 2];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( 255 < (usParamLen = (unsigned short)VOS_StrLen(class_name) + 1) )
    {
        return OLT_ERR_PARAM;
    }

    if ( 255 < tlv_len )
    {
        return OLT_ERR_PARAM;
    }
    
    VOS_MemCpy(&CmdParam[0], &tlv_len, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[1], &usParamLen, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[4], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[10], class_name, usParamLen);
    VOS_MemCpy(&CmdParam[10 + usParamLen], tlv_data, tlv_len);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_CREATE_SERVICE_FLOW_CLASS_NAME_106, CmdParam, (ULONG)(usParamLen + tlv_len + 10), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CreateCmcServiceFlowClassName(%d, %d, %s, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, class_name, tlv_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DestroyCmcServiceFlowClassName(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned char *class_name)
{
    int iRlt;
    int cfg_slot;
    unsigned long ulParamLen;
    unsigned char CmdParam[sizeof(mac_address_t) + 255];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    if ( 255 < (ulParamLen = VOS_StrLen(class_name) + 1) )
    {
        return OLT_ERR_PARAM;
    }

    VOS_MemCpy(&CmdParam[0], cmc_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[6], class_name, ulParamLen);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_DESTROY_SERVICE_FLOW_CLASS_NAME_107, CmdParam, ulParamLen + 6, NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DestroyCmcServiceFlowClassName(%d, %d, %s)'s result(%d) to slot %d.\r\n", olt_id, onu_id, class_name, iRlt, cfg_slot);

    return iRlt;
}
#endif

#if 1
/* --------------------地址管理API------------------- */
static int RPC_GetCmcMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    VOID *pRcvBuf;
    unsigned short nIsOnlyNum;
    unsigned char CmdParam[sizeof(mac_address_t) + sizeof(unsigned short)];
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(addr_num);

    if ( NULL != addr_table )
    {
        nIsOnlyNum = 0;
        pRcvBuf = addr_table;
        iRcvBufLen = sizeof(PON_address_table_t);
    }
    else
    {
        nIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
    
    VOS_MemCpy(&CmdParam[0], &nIsOnlyNum, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cmc_mac, sizeof(mac_address_t));
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CMC_MAC_ADDTBL_108, CmdParam, sizeof(CmdParam), pRcvBuf, iRcvBufLen, 0, 0);
    if (iRlt >= 0)
    {
    	*addr_num = iRlt;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmcMacAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCmMacAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, unsigned short *addr_num, PON_address_table_t addr_table)
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    VOID *pRcvBuf;
    unsigned short nIsOnlyNum;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(unsigned short)];
	
    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(addr_num);

    if ( NULL != addr_table )
    {
        nIsOnlyNum = 0;
        pRcvBuf = addr_table;
        iRcvBufLen = sizeof(PON_address_table_t);
    }
    else
    {
        nIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
    
    VOS_MemCpy(&CmdParam[0], &nIsOnlyNum, sizeof(unsigned short));
    VOS_MemCpy(&CmdParam[2], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[8], cmc_mac, sizeof(mac_address_t));
	
    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GET_CM_MAC_ADDTBL_109, CmdParam, sizeof(CmdParam), pRcvBuf, iRcvBufLen, 0, 0);
    if (iRlt >= 0)
    {
    	*addr_num = iRlt;
    	iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCmMacAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, *addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetCmAddrTbl(short int olt_id, short int onu_id, mac_address_t cmc_mac, mac_address_t cm_mac, int addr_type)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(mac_address_t) * 2 + sizeof(int)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    VOS_MemCpy(&CmdParam[0], &addr_type, sizeof(int));
    VOS_MemCpy(&CmdParam[4], cm_mac, sizeof(mac_address_t));
    VOS_MemCpy(&CmdParam[10], cmc_mac, sizeof(mac_address_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_RESET_CM_MAC_ADDTBL_110, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetCmAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, addr_type, iRlt, cfg_slot);

    return iRlt;
}

#endif

#endif


#if 1
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
/*--------------------- GPON OMCI-------------------------*/
int RPC_GetGponOnuCfg(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SPLIT_LINE_OMCI_GET_ONU_CFG, NULL, 0, onuConfig, sizeof(tGponOnuConfig), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetGponOnuCfg(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
    return iRlt;    
}
int RPC_SetGponOnuCfg(short int olt_id, short int onu_id, tGponOnuConfig *onuConfig)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);


    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_SPLIT_LINE_OMCI_SET_ONU_CFG, onuConfig, sizeof(tGponOnuConfig), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetGponOnuCfg(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
    
    return iRlt;    
}
#endif

static int RPC_SetMulticastTemplate(short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[sizeof(CTC_GPONADP_ONU_Multicast_Prof_t)  + sizeof(int) * 2 + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
    VOS_MemCpy(&CmdParam[0], &prof1, sizeof(int));
    VOS_MemCpy(&CmdParam[4], &prof2, sizeof(int));
    VOS_MemCpy(&CmdParam[8], &stateflag, sizeof(unsigned char));
    VOS_MemCpy(&CmdParam[9], parameter, sizeof(CTC_GPONADP_ONU_Multicast_Prof_t));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_Set_Multicast_Template_112, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMulticastTemplate(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
    
    return iRlt;
}
static int RPC_GetMulticastTemplate(short int olt_id, short int onu_id, int prof1,  int prof2,  CTC_GPONADP_ONU_Multicast_Prof_t *parameter,unsigned char stateflag)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[ sizeof(int) * 2 + sizeof(unsigned char)];

    OLT_ASSERT(olt_id);
    ONU_ASSERT(onu_id);
    VOS_ASSERT(parameter);
	
    VOS_MemCpy(&CmdParam[0], &prof1, sizeof(int));
    VOS_MemCpy(&CmdParam[4], &prof2, sizeof(int));
    VOS_MemCpy(&CmdParam[8], &stateflag, sizeof(unsigned char));

    iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_Get_Multicast_Template_113, CmdParam, sizeof(CmdParam), parameter , sizeof(CTC_GPONADP_ONU_Multicast_Prof_t), 0, 0);

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMulticastTemplate(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
    
    return iRlt;
}

static int RPC_SetMcastOperProfile(short int olt_id, short int onu_id, int prof,CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[ sizeof(int) + sizeof(CTC_GPONADP_ONU_McastOper_Prof_t)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(parameter);
	
	VOS_MemCpy(&CmdParam[0], &prof, sizeof(int));
	VOS_MemCpy(&CmdParam[4], parameter, sizeof(CTC_GPONADP_ONU_McastOper_Prof_t));

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_Set_Mcast_OperProfile_114, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMcastOperProfile(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_GetMcastOperProfile(short int olt_id, short int onu_id, int prof,CTC_GPONADP_ONU_McastOper_Prof_t *parameter)
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[ sizeof(int)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(parameter);
	
	VOS_MemCpy(&CmdParam[0], &prof, sizeof(int));

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_Get_Mcast_OperProfile_115, CmdParam, sizeof(CmdParam), parameter, sizeof(CTC_GPONADP_ONU_McastOper_Prof_t), 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMcastOperProfile(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_SetUniPortAssociateMcastProf( short int olt_id, short int onu_id,short int portid ,unsigned char stateflag,int profIdx)
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[ sizeof(int) + sizeof(short int) + sizeof(unsigned char)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], &profIdx, sizeof(int));
	VOS_MemCpy(&CmdParam[4], &portid, sizeof(short int));
	VOS_MemCpy(&CmdParam[6], &stateflag, sizeof(unsigned char));

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_SetUniPortAssociateMcastProf_116, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetUniPortAssociateMcastProf(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}	
static int RPC_GetUniPortAssociateMcastProf( short int olt_id, short int onu_id, short int port_id,CTC_GPONADP_ONU_Profile_t *ProfIdx)
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(short int)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], &port_id, sizeof(short int));

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_CMC_GetUniPortAssociateMcastProf_117, CmdParam, sizeof(CmdParam), ProfIdx, sizeof(CTC_GPONADP_ONU_Profile_t), 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetUniPortAssociateMcastProf(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}	
static int RPC_SetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char state)
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Port_Isolation_118, &state, sizeof(char), NULL, 0, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPortIsolation(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_GetOnuPortIsolation( short int olt_id, short int onu_id,unsigned char * parameter)
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_ASSERT(parameter);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Port_Isolation_119, NULL, 0, parameter, sizeof(unsigned char), 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPortIsolation(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_GetOnuMacEntry(short int olt_id, short int onu_id, ULONG mactype ,OnuPortLacationInfor_S *table)
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(ULONG)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], &mactype, sizeof(int));

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_Mac_Entry_120, CmdParam, sizeof(CmdParam), table, sizeof(OnuPortLacationInfor_S), 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuMacEntry(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}	

static int RPC_SetOnuPortSaveConfig(short int olt_id, short int onu_id, short int port_id,unsigned char action )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(short)+sizeof(char)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], &port_id, sizeof(short));
	VOS_MemCpy(&CmdParam[2], &action, sizeof(char));
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_Port_Save_Config_121, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPortSaveConfig(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}	

static int RPC_SetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short port_downtime,unsigned short restart_port_times )
{
	int iRlt;
	int cfg_slot;
	unsigned short CmdParam[2];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	CmdParam[0] = port_downtime;
	CmdParam[1] = restart_port_times;
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_Loop_DetectionTime_122, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuLoopDetectionTime(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}	

static int RPC_GetOnuLoopDetectionTime(short int olt_id, short int onu_id, unsigned short *port_downtime,unsigned short *restart_port_times )
{
	int iRlt;
	int cfg_slot;
	unsigned short CmdParam[2];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_Loop_DetectionTime_123, NULL, 0, CmdParam, sizeof(CmdParam), 0, 0);
	*port_downtime = CmdParam[0];
	*restart_port_times = CmdParam[1];
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuLoopDetectionTime(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_SetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char mode )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(short)+sizeof(char)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], &port_id, sizeof(short));
	VOS_MemCpy(&CmdParam[2], &mode, sizeof(char));
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_Port_Mode_124, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPortMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_GetOnuPortMode(short int olt_id, short int onu_id, unsigned short port_id,unsigned char *mode )
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_Port_Mode_125, &port_id, sizeof(short), mode, sizeof(char), 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPortMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_SetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(OnuPortStorm_S)+sizeof(short)];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	VOS_MemCpy(&CmdParam[0], &port_id, sizeof(short));
	VOS_MemCpy(&CmdParam[2], status, sizeof(OnuPortStorm_S));
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_Port_StormStatus_126, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuPortStormStatus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_GetOnuPortStormStatus(short int olt_id, short int onu_id, unsigned short port_id,OnuPortStorm_S * status )
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_Port_StormStatus_127, &port_id, sizeof(short), status, sizeof(OnuPortStorm_S), 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPortStormStatus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_PengSetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[NAMELEN];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_MemSet(CmdParam,0,sizeof(CmdParam));

	CmdParam[0] = device_location_len;
	VOS_MemCpy(&CmdParam[1],device_location,device_location_len);
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_DeviceLocation_128, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_PengSetOnuDeviceLocation(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_GetOnuDeviceLocation(short int olt_id, short int onu_id, unsigned char* device_location)
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_DeviceLocation_129, NULL, 0, device_location,NAMELEN , 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuDeviceLocation(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_SetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[NAMELEN];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_MemSet(CmdParam,0,sizeof(CmdParam));

	CmdParam[0] = device_location_len;
	VOS_MemCpy(&CmdParam[1],device_location,device_location_len);
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_DeviceDescription_130, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDeviceDescription(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_GetOnuDeviceDescription(short int olt_id, short int onu_id, unsigned char* device_location )
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_DeviceDescription_131, NULL, 0, device_location, NAMELEN, 0, 0);

	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuDeviceDescription(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_PengSetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_location,unsigned char  device_location_len )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[NAMELEN];

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	VOS_MemSet(CmdParam,0,sizeof(CmdParam));

	CmdParam[0] = device_location_len;
	VOS_MemCpy(&CmdParam[1],device_location,device_location_len);
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Set_Onu_DeviceName_132, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDeviceName(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_PengGetOnuDeviceName(short int olt_id, short int onu_id, unsigned char* device_location)
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_DeviceName_133, NULL, 0, device_location, NAMELEN, 0, 0);
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuDeviceName(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_GetOnuPortMacNumber(short int olt_id, short int onu_id,  short int port_id, unsigned short  *mac_address_number )
{
	int iRlt;
	int cfg_slot;

	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_PortMacNumber_134, &port_id, sizeof(short), mac_address_number, sizeof(short), 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPortMacNumber(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}
static int RPC_GetOnuPortLocationByMAC(short int olt_id, short int onu_id, mac_address_t mac, short int vlan_id,OnuPortLacationEntry_S *port_location_infor )
{
	int iRlt;
	int cfg_slot;
	unsigned char CmdParam[sizeof(mac_address_t)+sizeof(short)];
	
	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	VOS_MemCpy(&CmdParam[0], mac, sizeof(mac_address_t));
	VOS_MemCpy(&CmdParam[sizeof(mac_address_t)], &vlan_id, sizeof(short));
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_PortLocationByMAC_135, CmdParam, sizeof(CmdParam), port_location_infor, sizeof(OnuPortLacationEntry_S), 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuPortLocationByMAC(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

static int RPC_GetOnuExtendAttribute(short int olt_id, short int onu_id, char *SupportAttribute)
{
	int iRlt;
	int cfg_slot;
	
	OLT_ASSERT(olt_id);
	ONU_ASSERT(onu_id);
	
	iRlt = ONU_Rpc_Call(olt_id, onu_id, &cfg_slot, ONU_CMD_Get_Onu_ExtendAttribute_136, NULL, 0, SupportAttribute, SUPPORT_ATTRIBUTE_LEN, 0, 0);
	
	OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuExtendAttribute(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);
	
	return iRlt;
}

/* --------------------------END-------------------------- */


/*************************************外部接口**********************************************/
static const  OnuMgmtIFs  sonu_rpcIfs = {
#if 1
/* -------------------ONU基本API------------------- */
    RPC_OnuIsValid,  
    RPC_OnuIsOnline,  
    RPC_AddOnuByManual,
    RPC_ModifyOnuByManual,    
    RPC_DelOnuByManual,
    RPC_AddGponOnuByManual,
    RPC_OnuCmdIsSupported,

    RPC_CopyOnu,
    RPC_GetIFType,
    RPC_SetIFType,
#endif

#if 1
/* -------------------ONU 认证管理API------------------- */
    RPC_DeregisterOnu,
    RPC_SetMacAuthMode,
    RPC_DelBindingOnu,
#if 0
    RPC_AddPendingOnu,
    RPC_DelPendingOnu,
    RPC_DelConfPendingOnu,
#endif
    RPC_AuthorizeOnu,
    RPC_OnuError,   /* AuthRequest */
    RPC_OnuError,   /* AuthSucess */
    RPC_OnuError,   /* AuthFail */
#endif

#if 1
/* -------------------ONU 业务管理API------------------- */
    RPC_SetOnuTrafficServiceMode,
    RPC_SetOnuPeerToPeer,
    RPC_SetOnuPeerToPeerForward,
    RPC_SetOnuBW,
    RPC_GetOnuSLA,

    RPC_SetOnuFecMode,
    RPC_GetOnuVlanMode,
    /*Begin: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    RPC_SetUniPort,
    RPC_SetSlowProtocolLimit,
    RPC_GetSlowProtocolLimit,
    /*End: for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/

    RPC_GetOnuBWInfo,
    RPC_GetOnuB2PMode,
    RPC_SetOnuB2PMode,
#endif

#if 1
/* -------------------ONU 监控统计管理API--------------- */
    RPC_ResetCounters,
    RPC_SetPonLoopback,
#endif

#if 1
/* -------------------ONU加密管理API------------------- */
    RPC_GetLLIDParams,
    RPC_StartEncryption,
    RPC_StopEncryption,
    RPC_SetOnuEncryptParams,
    RPC_GetOnuEncryptParams,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_UpdateEncryptionKey,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif

#if 1
/* -------------------ONU 地址管理API------------------- */
    RPC_GetOnuMacAddrTbl,
    RPC_GetOltMacAddrTbl,
    RPC_GetOltMacAddrVlanTbl,/*GetOltMacAddrVlanTbl*/
    RPC_SetOnuMaxMac,
    RPC_GetOnuUniMacCfg,
    RPC_GetOnuMacCheckFlag,
    RPC_GetAllEthPortMacCounter,
#endif

#if 1
/* -------------------ONU 光路管理API------------------- */
    RPC_GetOnuDistance,
    RPC_GetOpticalCapability,
#endif

#if 1
/* -------------------ONU 倒换API---------------- */
    RPC_SetOnuLLID,
#endif
    
#if 1
/* -------------------ONU 设备管理API------------------- */
    RPC_GetOnuVer,
    RPC_GetOnuPonVer,
    RPC_GetOnuRegisterInfo,
    RPC_GetOnuI2CInfo,
    RPC_SetOnuI2CInfo,
    
    RPC_ResetOnu,
    RPC_SetOnuSWUpdateMode,
    RPC_OnuSwUpdate,
    RPC_OnuSwGwCtcConvert,
    RPC_GetBurnImageComplete,
    
    RPC_SetOnuDeviceName,
    RPC_SetOnuDeviceDesc,
    RPC_SetOnuDeviceLocation,
    RPC_GetOnuAllPortStatisticData,    
#endif
   
#if 1
/* -------------------ONU CTC-PROTOCOL API---------- */
    RPC_GetCtcVersion,
    RPC_GetFirmwareVersion,
    RPC_GetSerialNumber,
    RPC_GetChipsetID,
    
    RPC_GetOnuCap1,
    RPC_GetOnuCap2,
    RPC_GetOnuCap3,

    RPC_OnuError,   /* UpdateOnuFirmware */
    RPC_OnuError,   /* ActiveOnuFirmware */
    RPC_OnuError,   /* CommitOnuFirmware */
    
    RPC_StartEncrypt,
    RPC_StopEncrypt,
    
    RPC_GetEthPortLinkState,
    RPC_GetEthPortAdminStatus,
    RPC_SetEthPortAdminStatus,
    
    RPC_GetEthPortPause,
    RPC_SetEthPortPause,
    
    RPC_GetEthPortAutoNegotiationAdmin,
    RPC_SetEthPortAutoNegotiationAdmin,
    RPC_SetEthPortAutoNegotiationRestart,
    RPC_GetEthPortAnLocalTecAbility,
    RPC_GetEthPortAnAdvertisedTecAbility,
    
    RPC_GetEthPortPolicing,
    RPC_SetEthPortPolicing,
    
    RPC_GetEthPortDownstreamPolicing,
    RPC_SetEthPortDownstreamPolicing,

    RPC_GetEthPortVlanConfig,
    RPC_SetEthPortVlanConfig,
    RPC_OnuError,   /* GetAllPortVlanConfig */
    
    RPC_GetEthPortClassificationAndMarking,
    RPC_SetEthPortClassificationAndMarking,
    RPC_ClearEthPortClassificationAndMarking,
    
    RPC_GetEthPortMulticastVlan,
    RPC_SetEthPortMulticastVlan,
    RPC_ClearEthPortMulticastVlan,

    RPC_GetEthPortMulticastGroupMaxNumber,
    RPC_SetEthPortMulticastGroupMaxNumber,
    
    RPC_GetEthPortMulticastTagStrip,
    RPC_SetEthPortMulticastTagStrip,
    RPC_OnuError,   /* GetAllPortMulcastTagStrip */

    RPC_GetEthPortMulticastTagOper,
    RPC_SetEthPortMulticastTagOper,
    RPC_OnuError,   /* SetObjMulticastTagOper */
    
    RPC_GetMulticastControl,
    RPC_SetMulticastControl,
    RPC_ClearMulticastControl,
    
    RPC_GetMulticastSwitch,
    RPC_SetMulticastSwitch,
    
    RPC_GetMulticastFastleaveAbility,
    RPC_GetMulticastFastleave,
    RPC_SetMulticastFastleave,
    
    RPC_GetPortStatisticData,
    RPC_GetPortStatisticState,
    RPC_SetPortStatisticState,

    RPC_SetAlarmAdminState,
    RPC_SetAlarmThreshold,
    RPC_OnuError,   /* GetDbaReportThreshold */
    RPC_OnuError,   /* SetDbaReportThreshold */
    
    RPC_GetMxuMngGlobalConfig,
    RPC_SetMxuMngGlobalConfig,
    RPC_GetMxuMngSnmpConfig,
    RPC_SetMxuMngSnmpConfig,
    
    RPC_GetHoldOver,
    RPC_SetHoldOver,
    RPC_GetOptTransDiag,
    RPC_SetTxPowerSupplyControl,

    RPC_GetFecAbility,

    RPC_GetIADInfo,
    RPC_GetVoipIadOperStatus,
    RPC_SetVoipIadOperation,
    RPC_GetVoipGlobalConfig,
    RPC_SetVoipGlobalConfig,
    RPC_GetVoipFaxConfig,
    RPC_SetVoipFaxConfig,
    
    RPC_GetVoipPortStatus,
    RPC_GetVoipPort,
    RPC_SetVoipPort,
    RPC_GetVoipPort2,
    RPC_SetVoipPort2,

    RPC_GetH248Config,
    RPC_SetH248Config,
    RPC_GetH248UserTidConfig,
    RPC_SetH248UserTidConfig,
    RPC_GetH248RtpTidConfig,
    RPC_SetH248RtpTidConfig,
    
    RPC_GetSipConfig,
    RPC_SetSipConfig,
    RPC_SetSipDigitMap,
    RPC_GetSipUserConfig,
    RPC_SetSipUserConfig,
    RPC_GetCTCOnuPortStatsData,
#endif

#if 1
/* -------------------ONU 远程管理API------------------- */
    RPC_CliCall,
    RPC_SetMgtReset,
    RPC_SetMgtConfig,
    RPC_SetMgtLaser,
    RPC_SetTemperature,
    RPC_SetPasFlush,

    RPC_SetAtuAgingTime,
    RPC_SetAtuLimit,

    RPC_SetPortLinkMon,
    RPC_SetPortModeMon,
    RPC_SetPortIsolate,

    RPC_SetVlanEnable,
    RPC_SetVlanMode,
    RPC_AddVlan,
    RPC_DelVlan,
    RPC_SetPortPvid,

    RPC_AddVlanPort,
    RPC_DelVlanPort,
    RPC_SetEthPortVlanTran,
    RPC_DelEthPortVlanTran,
    RPC_SetEthPortVlanAgg,
    RPC_DelEthPortVlanAgg,

    RPC_SetPortQinQEnable,
    RPC_AddQinQVlanTag,
    RPC_DelQinQVlanTag,

    RPC_SetPortVlanFrameTypeAcc,
    RPC_SetPortIngressVlanFilter,

    RPC_SetPortMode,
    RPC_SetPortFcMode,
    RPC_SetPortAtuLearn,
    RPC_SetPortAtuFlood,
    RPC_SetPortLoopDetect,
    RPC_SetPortStatFlush,

    RPC_SetIngressRateLimitBase,
    RPC_SetPortIngressRate,
    RPC_SetPortEgressRate,

    RPC_SetQosClass,
    RPC_ClrQosClass,
    RPC_SetQosRule,
    RPC_ClrQosRule,

    RPC_SetPortQosRule,
    RPC_ClrPortQosRule,
    RPC_SetPortQosRuleType,

    RPC_SetPortDefPriority,
    RPC_SetPortNewPriority,
    RPC_SetQosPrioToQueue,
    RPC_SetQosDscpToQueue,
    
    RPC_SetPortUserPriorityEnable,
    RPC_SetPortIpPriorityEnable,
    RPC_SetQosAlgorithm,
    RPC_GWONU_Set_QosMode,
    RPC_GWONU_Set_Rule,
    
    RPC_SetIgmpEnable,
    RPC_SetIgmpAuth,
    RPC_SetIgmpHostAge,
    RPC_SetIgmpGroupAge,
    RPC_SetIgmpMaxResTime,
    
    RPC_SetIgmpMaxGroup,
    RPC_AddIgmpGroup,
    RPC_DeleteIgmpGroup,
    RPC_SetPortIgmpFastLeave,
    RPC_SetPortMulticastVlan,

    RPC_SetPortMirrorFrom,
    RPC_SetPortMirrorTo,
    RPC_DeleteMirror,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU CMC协议管理API------------------- */
    RPC_RegisterCmc,
    RPC_UnregisterCmc,
    RPC_DumpCmc,
    RPC_DumpAlarms,
    RPC_DumpLogs,

    RPC_ResetCmcBoard,
    RPC_GetCmcVersion,
    RPC_GetCmcMaxMulticasts,
    RPC_GetCmcMaxCm,
    RPC_SetCmcMaxCm,

    RPC_GetCmcTime,
    RPC_SetCmcTime,
    RPC_LocalCmcTime,
    RPC_SetCmcCustomConfig,
    RPC_DumpCmcCustomConfig,

    RPC_DumpCmcDownChannel,
    RPC_DumpCmcUpChannel,
    RPC_GetCmcDownChannelMode,
    RPC_SetCmcDownChannelMode,
    RPC_GetCmcUpChannelMode,
    
    RPC_SetCmcUpChannelMode,
    RPC_GetCmcUpChannelD30Mode,
    RPC_SetCmcUpChannelD30Mode,
    RPC_GetCmcDownChannelFreq,
    RPC_SetCmcDownChannelFreq,

    RPC_GetCmcUpChannelFreq,
    RPC_SetCmcUpChannelFreq,
    RPC_OnuError,   /* GetCmcDownAutoFreq */
    RPC_SetCmcDownAutoFreq,
    RPC_OnuError,   /* GetCmcUpAutoFreq */

    RPC_SetCmcUpAutoFreq,
    RPC_GetCmcUpChannelWidth,
    RPC_SetCmcUpChannelWidth,
    RPC_GetCmcDownChannelAnnexMode,
    RPC_SetCmcDownChannelAnnexMode,

    RPC_GetCmcUpChannelType,
    RPC_SetCmcUpChannelType,
    RPC_GetCmcDownChannelModulation,
    RPC_SetCmcDownChannelModulation,
    RPC_GetCmcUpChannelProfile,
    
    RPC_SetCmcUpChannelProfile,
    RPC_GetCmcDownChannelInterleaver,
    RPC_SetCmcDownChannelInterleaver,
    RPC_GetCmcDownChannelPower,    
    RPC_SetCmcDownChannelPower,    

    RPC_GetCmcUpChannelPower,    
    RPC_SetCmcUpChannelPower,    
    RPC_DumpCmcUpChannelPower,    
    RPC_DumpCmcUpChannelSignalQuality,    
    RPC_DumpCmcInterfaceUtilization,    

    RPC_DumpCmcInterfaceStatistics,    
    RPC_DumpCmcMacStatistics,    
    RPC_DumpCmcAllInterface,    
    RPC_DumpAllLoadBalancingGrp,    
    RPC_DumpLoadBalancingGrp,    

    RPC_DumpLoadBalancingGrpDownstream,    
    RPC_DumpLoadBalancingGrpUpstream,    
    RPC_DumpLoadBalancingDynConfig,    
    RPC_SetLoadBalancingDynMethod,    
    RPC_SetLoadBalancingDynPeriod,    

    RPC_SetLoadBalancingDynWeightedAveragePeriod,    
    RPC_SetLoadBalancingDynOverloadThresold,    
    RPC_SetLoadBalancingDynDifferenceThresold,    
    RPC_SetLoadBalancingDynMaxMoveNumber,    
    RPC_SetLoadBalancingDynMinHoldTime,    

    RPC_SetLoadBalancingDynRangeOverrideMode,    
    RPC_SetLoadBalancingDynAtdmaDccInitTech,    
    RPC_SetLoadBalancingDynScdmaDccInitTech,    
    RPC_SetLoadBalancingDynAtdmaDbcInitTech,    
    RPC_SetLoadBalancingDynScdmaDbcInitTech,    

    RPC_CreateLoadBalancingGrp,    
    RPC_DestroyLoadBalancingGrp,    
    RPC_AddLoadBalancingGrpDownstream,    
    RPC_RemoveLoadBalancingGrpDownstream,    
    RPC_AddLoadBalancingGrpUpstream,    

    RPC_RemoveLoadBalancingGrpUpstream,    
    RPC_AddLoadBalancingGrpModem,    
    RPC_RemoveLoadBalancingGrpModem,    
    RPC_AddLoadBalancingGrpExcludeModem,    
    RPC_RemoveLoadBalancingGrpExcludeModem,    

    RPC_DumpLoadBalancingGrpModem,    
    RPC_DumpLoadBalancingGrpActivedModem,    
    RPC_DumpLoadBalancingGrpExcludeModem,    
    RPC_DumpLoadBalancingGrpExcludeActivedModem,    
    RPC_OnuError,   /* ReserveCmcLoadBalancingGrp */

    RPC_DumpAllCm,
    RPC_DumpCm,
    RPC_DumpAllCmHistory,
    RPC_DumpCmHistory,
    RPC_ClearAllCmHistory,

    RPC_ResetCm,
    RPC_DumpCmDownstream,
    RPC_DumpCmUpstream,
    RPC_SetCmDownstream,
    RPC_SetCmUpstream,

    RPC_CreateCmServiceFlow,
    RPC_ModifyCmServiceFlow,
    RPC_DestroyCmServiceFlow,
    RPC_DumpCmClassifier,
    RPC_DumpCmServiceFlow,
    
    RPC_DumpCmcClassifier,
    RPC_DumpCmcServiceFlow,
    RPC_DumpCmcServiceFlowStatistics,
    RPC_DumpCmcDownChannelBondingGroup,
    RPC_DumpCmcUpChannelBondingGroup,
    
    RPC_CreateCmcServiceFlowClassName,
    RPC_DestroyCmcServiceFlowClassName,
    RPC_GetCmcMacAddrTbl,
    RPC_GetCmMacAddrTbl,
    RPC_ResetCmAddrTbl,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------ONU DOCSIS应用管理API------------------- */
#endif

#if 0
    NULL,       /*RPC_GetGponOnuCfg,*/
    NULL,       /*RPC_SetGponOnuCfg,*/
#endif
RPC_SetMulticastTemplate,
RPC_GetMulticastTemplate,
RPC_SetMcastOperProfile,
RPC_GetMcastOperProfile,
RPC_SetUniPortAssociateMcastProf,
RPC_GetUniPortAssociateMcastProf,

/* --------------------------Dr.Peng-------------------------- */
RPC_SetOnuPortIsolation,
RPC_GetOnuPortIsolation,
RPC_GetOnuMacEntry,
RPC_SetOnuPortSaveConfig,
RPC_SetOnuLoopDetectionTime,

RPC_GetOnuLoopDetectionTime,
RPC_SetOnuPortMode,
RPC_GetOnuPortMode,
RPC_SetOnuPortStormStatus,
RPC_GetOnuPortStormStatus,

RPC_PengSetOnuDeviceLocation,
RPC_GetOnuDeviceLocation,
RPC_SetOnuDeviceDescription,
RPC_GetOnuDeviceDescription,
RPC_PengSetOnuDeviceName,

RPC_PengGetOnuDeviceName,
RPC_GetOnuPortMacNumber,
RPC_GetOnuPortLocationByMAC,
RPC_GetOnuExtendAttribute,

/* --------------------------END-------------------------- */

    RPC_OnuError
};


void ONU_RPC_Support()
{
    ONU_RegisterAdapter(ONU_ADAPTER_RPC, &sonu_rpcIfs);

    if( VOS_OK != CDP_SYNC_Register( MODULE_ONU, ( CDP_SYNC_RECEIVE_NOTIFY ) ONU_Rpc_Callback ) )
    {
        VOS_ASSERT( 0 );
    }
}


#ifdef __cplusplus

}

#endif

