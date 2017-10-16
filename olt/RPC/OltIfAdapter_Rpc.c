/***************************************************************
*
*						Module Name:  OltIfAdapter_Rpc.c
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
*   Date: 			2010/05/11
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  10/05/1    |   liwei056    |     create 
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
#include "cli/cli.h"
#include "man/cli/cl_sys.h"
#include "man/cli/cl_vect.h"
#include "sys/devsm/devsm_remote.h"

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "../../access_identifier/access_id.h"

extern PON_address_record_t Mac_addr_table[PON_ADDRESS_TABLE_SIZE];
extern PON_address_vlan_record_t Mac_addr_vlan_table[PON_ADDRESS_TABLE_SIZE];

extern OLT_onu_table_t  olt_onu_global_table;


/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#define RPC_OLT_CMDBUF_LEN (1024)


/* 板间通信使用以太帧，帧长不小于64字节[这里每次占用32字节，不浪费] */
typedef union
{
    long int liVal;
    long int liValPair[2];
    long int liVals[4];
    ULONG  ulVal;
    ULONG  ulValPair[2];
    ULONG  ulVals[8];
    int    iVal;
    int    iValPair[2];
    int    iVals[8];
    unsigned int uiVal;
    unsigned int uiValPair[2];
    unsigned int uiVals[8];   
    bool   bVal;
    bool   bValPair[2];
    bool   bVals[32];
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
}OLT_VALUE_S;

typedef struct stOltCmdHead
{
    int cmd;
    short int olt_id;
    short int llid;
    UCHAR params[0];
}OLT_CMD_HEAD;

typedef struct stOltCmd
{
    int cmd;
    short int olt_id;
    short int llid;
    OLT_VALUE_S params;
}OLT_CMD;

typedef struct stOltRlt
{
    int rlt;
    UCHAR values[0];
}OLT_RLT;

typedef struct stOltEvt
{
    int evt;
    short int olt_id;
    short int llid;
    int param1;
    int param2;   
}OLT_EVT;


/*-----------------------------RPC系统接口----------------------------------------*/
#define OLT_RPC_CMD_NEW(pRpcCmd) ( NULL != (pRpcCmd = CDP_SYNC_AllocMsg(sizeof(OLT_CMD), MODULE_OLT)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define OLT_RPC_CMD_NEW_EX(pRpcCmd, size) ( NULL != (pRpcCmd = CDP_SYNC_AllocMsg(sizeof(OLT_CMD_HEAD) + (size), MODULE_OLT)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define OLT_RPC_RLT_NEW(pRpcRlt, size) ( NULL != (pRpcRlt = CDP_SYNC_AllocMsg((size), MODULE_OLT)) ) ? OLT_ERR_OK : OLT_ERR_MALLOC 
#define OLT_RPC_RLT_FREE(pRpcRlt)      CDP_SYNC_FreeMsg(pRpcRlt)

int OLT_Rpc_Call(short int olt_id, short int llid, int *olt_slot, int olt_cmd, VOID *pSndValue, ULONG ulSndLen, VOID *pRcvData, ULONG ulRcvLen, ULONG ulTimeOut, ULONG ulCallFlags)
{
    ULONG ulRet;
    int iRlt = OLT_ERR_OK;
    VOID *pTmpRcvData = NULL;
    ULONG ulTmpRcvLen = 0;
    ULONG ulSendLen;
    ULONG ulDstSlot;
    int   iDstSlot, iDstPort;
    OLT_CMD_HEAD *pRpcCmd;
    OLT_RLT *pCmdResult;
    short int dst_id; 

    OLT_ASSERT(olt_id);
    VOS_ASSERT(olt_slot);
    OLT_CMD_ASSERT(olt_cmd);

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

    /* 统一的RPC不支持接口的优化处理 */
#ifdef OLT_RPC_OPTIMIZE
    if ( OLTAdv_CmdIsSupported(olt_id, olt_cmd) )
#endif
    {
        if ( OLT_RPC_CALLFLAG_OUTPUTASINPUT & ulCallFlags )
        {
            /* 输出参数合并为输入参数 */
            VOS_ASSERT(pSndValue);
            ulSndLen += ulRcvLen;
        }
        
        if (ulSndLen <= sizeof(OLT_VALUE_S))
        {
            iRlt = OLT_RPC_CMD_NEW(pRpcCmd);
            ulSendLen = sizeof(OLT_CMD);
        }
        else
        {
            iRlt = OLT_RPC_CMD_NEW_EX(pRpcCmd, ulSndLen);
            ulSendLen = ulSndLen + sizeof(OLT_CMD_HEAD);
        }
        
        if ( OLT_ERR_OK == iRlt )
        {
            /* 统一的RPC输入参数处理 */
            pRpcCmd->cmd    = olt_cmd;
            
            /* OltID在目标板的定位转换 */
            pRpcCmd->olt_id = dst_id;
            pRpcCmd->llid   = llid;

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
                pstCdpSyncHead->ulSrcMod = MODULE_OLT;
                pstCdpSyncHead->ulDstMod = MODULE_OLT;
                pstCdpSyncHead->usType = 0;
                pstCdpSyncHead->usId   = 0;
            
                ulRet = CDP_Send(RPU_TID_CDP_OLT, ulDstSlot, RPU_TID_CDP_OLT, CDP_MSG_TM_SYNC, pstCdpSyncHead, ulSendLen + sizeof(CDP_SYNC_Header_S), MODULE_OLT);
                pTmpRcvData = NULL;

                /* CDP的同步发送，必须手动释放消息块 */
                CDP_FreeMsg( pstCdpSyncHead );
            }
            else
            {
                ulRet = CDP_SYNC_Call(MODULE_OLT, ulDstSlot, MODULE_OLT, 0, pRpcCmd, ulSendLen, &pTmpRcvData, &ulTmpRcvLen, ulTimeOut);
            }
            
        	if ( VOS_OK == ulRet )
            {
                if ( NULL != (pCmdResult = (OLT_RLT *)pTmpRcvData) )
                {
                    if ( ulTmpRcvLen >= sizeof(OLT_RLT) )
                    {
                        /* 统一的RPC返回值处理 */
                        iRlt = pCmdResult->rlt;

                        /* 统一的RPC输出参数处理 */
                        if ( NULL != pRcvData )
                        {
                            if ( 0 < (ulTmpRcvLen -= sizeof(OLT_RLT)) )
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
                                    *(OLT_RLT**)pRcvData = pCmdResult;
                                    VOS_ASSERT(ulRcvLen == sizeof(OLT_RLT*));
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
    }

    VOS_SysLog(LOG_TYPE_OLT, LOG_DEBUG, "OLT_Rpc_Call(oltid:%d)'s result(%d) send cmd[%d] to pon%d/%d from slot%d", olt_id, iRlt, olt_cmd, iDstSlot, iDstPort, SYS_LOCAL_MODULE_SLOTNO);

    return iRlt;
}

int OLT_Rpc_Callback( ULONG ulSrcSlot, ULONG ulSrcMod, VOID *pRcvData, ULONG ulRcvDataLen, VOID **ppSendData, ULONG *pulSendDataLen )   
{
    int iRet;
    OLT_CMD *pRpcCmd;
    OLT_RLT *pRpcResult = NULL;
    ULONG ulResultLen;
    short int olt_id, slot_id;
	int sAddrNumTemp = 0;
    
    VOS_ASSERT(pRcvData);
    VOS_ASSERT(ulRcvDataLen >= sizeof(OLT_CMD));
    VOS_ASSERT(ppSendData);
    VOS_ASSERT(pulSendDataLen);

    /* 其它板向业务板发送RPC，目前主要用于配置 */
    pRpcCmd = (OLT_CMD*)pRcvData;
    olt_id = pRpcCmd->olt_id;
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

    switch ( pRpcCmd->cmd )
    {
#if 1
/* -------------------OLT基本API------------------- */
        case OLT_CMD_OLT_IS_EXIST_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_IsExist( olt_id, pRpcResult + 1 );
                /* OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_IsExist(%d)'s result(%d) from slot %d.\r\n", pRpcCmd->olt_id, iRet, ulSrcSlot); */
            }
          
            break;
        case OLT_CMD_GET_CHIPTYPEID_2:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *pTypeID = pRpcResult + 1;
                    
                iRet = OLT_GetChipTypeID( olt_id, pTypeID );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetChipTypeID(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *pTypeID, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_CHIPTYPENAME_3:
            ulResultLen = sizeof(OLT_RLT) + OLT_CHIP_NAMELEN;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                char *pcszTmp = pRpcResult + 1;
                
                iRet = OLT_GetChipTypeName( olt_id, pcszTmp );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetChipTypeName(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_RESET_PON_4:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResetPon( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResetPon(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_REMOVE_OLT_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RemoveOlt( olt_id, pRpcCmd->params.bVals[0], pRpcCmd->params.bVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RemoveOlt(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.bVals[0], pRpcCmd->params.bVals[1], iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_COPY_OLT_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_CopyOlt( olt_id, pRpcCmd->llid, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_CopyOlt(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_IS_SUPPORT_CMD_7:
            ulResultLen = sizeof(OLT_RLT) + sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                short int *psCmdId = pRpcResult + 1;

                *psCmdId = pRpcCmd->llid;
                iRet = OLT_CmdIsSupported( olt_id, psCmdId );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_CmdIsSupported(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *psCmdId, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_PON_DEBUG_MODE_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetDebugMode( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetDebugMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_INIT_PARAMS_9:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetInitParams( olt_id, pRpcCmd->params.usVals[0], pRpcCmd->params.usVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetInitParams(%d, %d, 0x%x)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.usVals[0], pRpcCmd->params.usVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_SYSTEM_PARAMS_10:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetSystemParams( olt_id, pRpcCmd->params.liVals[0], pRpcCmd->params.liVals[1], pRpcCmd->params.liVals[2], pRpcCmd->params.liVals[3] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.liVals[0], pRpcCmd->params.liVals[1], pRpcCmd->params.liVals[2], pRpcCmd->params.liVals[3], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_PON_I2C_EXTINFO_11:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPonI2CExtInfo( olt_id, &pRpcCmd->params );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPonI2CExtInfo(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_PON_I2C_EXTINFO_12:
            ulResultLen = sizeof(OLT_RLT) + sizeof(eeprom_gfa_epon_ext_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetPonI2CExtInfo( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetPonI2CExtInfo(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_CARD_I2C_INFO_13:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetCardI2CInfo( olt_id, &pRpcCmd->params );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetCardI2CInfo(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_CARD_I2C_INFO_14:
            ulResultLen = sizeof(OLT_RLT) + sizeof(board_sysinfo_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetCardI2CInfo( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetCardI2CInfo(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
		/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_WRITE_MDIO_REGISTER_15:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_WriteMdioRegister( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2]);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_WriteMdioRegister(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_READ_MDIO_REGISTER_16:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ReadMdioRegister( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ReadMdioRegister(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_READ_I2C_REGISTER_17:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ReadI2CRegister( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ReadI2CRegister(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
		}
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_READ_GPIO_19:
			ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *value = (bool*)(pRpcResult + 1);
            
                iRet = OLT_ReadGpio( olt_id, (int)pRpcCmd->llid, value );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ReadGpio(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *value, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_WRITE_GPIO_20:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_WriteGpio( olt_id, (int)pRpcCmd->llid, pRpcCmd->params.bVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_WriteGpio(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.bVal, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_SEND_CHIP_CLI_21:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SendChipCli( olt_id, (unsigned short)pRpcCmd->llid, pRpcCmd->params.aucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SendChipCli(%d, %d, %s)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.aucVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_DEVICE_NAME_22:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetDeviceName( olt_id, &pRpcCmd->params, pRpcCmd->llid);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetDeviceName(%d, %s, %d)'s result(%d) from slot %d.\r\n", olt_id, &pRpcCmd->params, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;
		/*reset pon chip by jinhl*/
		case OLT_CMD_RESET_PON_CHIP_23:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResetPonChip( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResetPonChip(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
#endif

            
#if 1
/* -------------------OLT PON管理API------------------- */
        case OLT_CMD_GET_VERSION_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(PON_device_versions_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetVersion( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetVersion(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_DBA_VERSION_2:
            ulResultLen = sizeof(OLT_RLT) + sizeof(OLT_DBA_version_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_DBA_version_t *pDBAver = pRpcResult + 1;
                
                iRet = OLT_GetDBAVersion( olt_id, pDBAver);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetDBAVersion(%d, %s, %s)'s result(%d) from slot %d.\r\n", olt_id, pDBAver->szDBAname, pDBAver->szDBAversion, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_CHK_VERSION_3:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *pbIsOK = pRpcResult + 1;
                
                iRet = OLT_ChkVersion( olt_id, pbIsOK);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ChkVersion(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *pbIsOK, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_CHK_DBA_VERSION_4:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *pbIsOK = pRpcResult + 1;
                
                iRet = OLT_ChkDBAVersion( olt_id, pbIsOK);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ChkDBAVersion(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *pbIsOK, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_CNI_LINK_STATUS_5:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetCniLinkStatus( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetCniLinkStatus(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;

        case OLT_CMD_GET_PON_WORKSTATUS_6:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piStatus = (int*)(pRpcResult + 1);
                
                iRet = OLT_GetPonWorkStatus( olt_id, piStatus );
                /* OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetPonWorkStatus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *piStatus, iRet, ulSrcSlot); */
            }
          
            break;	
        case OLT_CMD_SET_ADMIN_STATUS_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAdminStatus( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAdminStatus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_ADMIN_STATUS_8:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piStatus = (int*)(pRpcResult + 1);
                
                iRet = OLT_GetAdminStatus( olt_id, piStatus );
                /* OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetAdminStatus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *piStatus, iRet, ulSrcSlot); */
            }
          
            break;
        case OLT_CMD_SET_VLAN_TPID_9:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetVlanTpid( olt_id, pRpcCmd->params.usVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetVlanTpid(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.usVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_VLAN_QINQ_10:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_vlan_qinq_t *pQinQ = &pRpcCmd->params;
                    
                iRet = OLT_SetVlanQinQ( olt_id, pQinQ );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetVlanQinQ(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pQinQ->qinq_direction, pQinQ->qinq_objectid, iRet, ulSrcSlot);
            }
          
            break;

        case OLT_CMD_SET_FRAME_SIZELIMIT_11:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPonFrameSizeLimit( olt_id, pRpcCmd->params.sVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPonFrameSizeLimit(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.sVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_FRAME_SIZELIMIT_12:
            ulResultLen = sizeof(OLT_RLT) + sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetPonFrameSizeLimit( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetPonFrameSizeLimit(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *(short int *)(pRpcResult + 1), iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_OAM_IS_LIMIT_13:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *pIsLimit = pRpcResult + 1;
                
                iRet = OLT_OamIsLimit( olt_id, pIsLimit);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_OamIsLimit(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *pIsLimit, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_UPDATE_PON_PARAMS_14:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_UpdatePonParams( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_UpdatePonParams(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_PPPOE_RELAY_MODE_15:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPPPoERelayMode( olt_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPPPoERelayMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1], iRet, ulSrcSlot);
            }
          
            break;
            
        case OLT_CMD_SET_PPPOE_RELAY_PARAMS_16:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int param1, param2;

                param1 = pRpcCmd->params.acVal;
                param2 = VOS_StrLen(pRpcCmd->params.acVal);
                iRet = OLT_SetPPPoERelayParams( olt_id, pRpcCmd->llid, param1, param2 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPPPoERelayParams(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, param1, param2, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_DHCP_RELAY_MODE_17:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetDhcpRelayMode( olt_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetDhcpRelayMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.sVals[0], pRpcCmd->params.sVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_IGMP_AUTH_MODE_18:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( pRpcCmd->params.iVal >= 0 )
                {
                    iRet = OLT_SetIgmpAuthMode( OLT_ID_ALL, pRpcCmd->params.iVal );
                }
                else
                {
                    iRet = OLT_SetIgmpAuthMode( olt_id, -pRpcCmd->params.iVal );
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetIgmpAuthMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SEND_FRAME_2_PON_19:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                ulRcvDataLen -= sizeof(OLT_CMD_HEAD);   
                iRet = OLT_SendFrame2PON( olt_id, pRpcCmd->llid, &pRpcCmd->params, ulRcvDataLen );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SendFrame2PON(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, ulRcvDataLen, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SEND_FRAME_2_CNI_20:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                ulRcvDataLen -= sizeof(OLT_CMD_HEAD);   
                iRet = OLT_SendFrame2CNI( olt_id, pRpcCmd->llid, &pRpcCmd->params, ulRcvDataLen );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SendFrame2CNI(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, ulRcvDataLen, iRet, ulSrcSlot);
            }
          
            break;
            
		/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_GET_VID_DOWNLINK_MODE_21:
			ulResultLen = sizeof(OLT_RLT) + sizeof(PON_olt_vid_downlink_config_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_GetVidDownlinkMode( olt_id, pRpcCmd->params.iVal,  (PON_olt_vid_downlink_config_t*)(pRpcResult + 1));
                
            }
          
            break;
		case OLT_CMD_DEL_VID_DOWNLINK_MODE_22:
			ulResultLen = sizeof(OLT_RLT) ;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_DelVidDownlinkMode( olt_id, pRpcCmd->params.iVal);
                
            }
          
            break;
		case OLT_CMD_GET_OLT_PARAMETERS_23:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(PON_olt_response_parameters_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_GetOltParameters( olt_id, (PON_olt_response_parameters_t *)(pRpcResult + 1));
                
            }
			break;
		}
        case OLT_CMD_SET_OLT_IGMP_SNOOPING_MODE_24:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_SetOltIgmpSnoopingMode( olt_id, (PON_olt_igmp_configuration_t*)&pRpcCmd->params);
                
            }
			break;
		}

		case OLT_CMD_GET_OLT_IGMP_SNOOPING_MODE_25:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(PON_olt_igmp_configuration_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_GetOltIgmpSnoopingMode( olt_id, (PON_olt_igmp_configuration_t *)(pRpcResult + 1));
                
            }
			break;
		}
        
		case OLT_CMD_SET_OLT_MLD_FORWARDING_MODE_26:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_SetOltMldForwardingMode( olt_id, pRpcCmd->params.iVal);
                
            }
			break;
		}
		case OLT_CMD_GET_OLT_MLD_FORWARDING_MODE_27:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_GetOltMldForwardingMode( olt_id, pRpcResult + 1);
                
            }
			break;
		}
		case OLT_CMD_SET_DBA_REPORT_FORMAT_28:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_SetDBAReportFormat( olt_id, pRpcCmd->params.iVal);
                
            }
			break;
		}
		case OLT_CMD_GET_DBA_REPORT_FORMAT_29:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_GetDBAReportFormat( olt_id, pRpcResult + 1);
                
            }
			break;
		}
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		/*Begin:for onu swap by jinhl@2013-04-27*/
		case OLT_CMD_UPDATE_PROV_BWINFO_30:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                                
                iRet = OLT_UpdateProvBWInfo( olt_id);
                
            }
			break;
		}
		/*End:for onu swap by jinhl@2013-04-27*/
#endif


#if 1
/* -------------------OLT LLID 管理API------------------- */
        case OLT_CMD_LLID_IS_EXIST_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *status = pRpcResult + 1;

                iRet = OLT_LLIDIsExist( olt_id, pRpcCmd->llid, status );
                /* OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_LLIDIsExist(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *status, iRet, ulSrcSlot); */
            }
          
            break;	
        case OLT_CMD_LLID_DEREGISTER_2:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_DeregisterLLID( olt_id, pRpcCmd->llid, pRpcCmd->params.bVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_DeregisterLLID(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_MAC_3:
            ulResultLen = sizeof(OLT_RLT) + sizeof(mac_address_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetLLIDMac( olt_id, pRpcCmd->llid, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDMac(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_REGINFO_4:
            ulResultLen = sizeof(OLT_RLT) + sizeof(onu_registration_info_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetLLIDRegisterInfo( olt_id, pRpcCmd->llid, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDRegisterInfo(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_AUTHORIZE_LLID_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_AuthorizeLLID( olt_id, pRpcCmd->llid, pRpcCmd->params.ucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_AuthorizeLLID(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.ucVal, iRet, ulSrcSlot);
            }
            
        	break;
            
        case OLT_CMD_SET_LLID_SLA_6:
            ulResultLen = sizeof(OLT_RLT) + sizeof(LLID_SLA_INFO_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                LLID_SLA_INFO_t *SLA = pRpcResult + 1;

                VOS_MemCpy(SLA, &pRpcCmd->params, sizeof(LLID_SLA_INFO_t));
                iRet = OLT_SetLLIDSLA( olt_id, pRpcCmd->llid, SLA );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDSLA(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_SLA_7:
            ulResultLen = sizeof(OLT_RLT) + sizeof(LLID_SLA_INFO_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetLLIDSLA( olt_id, pRpcCmd->llid, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDSLA(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_LLID_POLICE_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetLLIDPolice( olt_id, pRpcCmd->llid, &pRpcCmd->params );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDPolice(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_POLICE_9:
            ulResultLen = sizeof(OLT_RLT) + sizeof(LLID_POLICE_INFO_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                LLID_POLICE_INFO_t *police = pRpcResult + 1;

                police->path = pRpcCmd->params.iVal;
                iRet = OLT_GetLLIDPolice( olt_id, pRpcCmd->llid, police );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDPolice(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_LLID_DBA_TYPE_10:
            ulResultLen = sizeof(OLT_RLT) + sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                short int *dbs_error = pRpcResult + 1;

                *dbs_error = pRpcCmd->params.iVals[1];
                iRet = OLT_SetLLIDdbaType( olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], dbs_error );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDdbaType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], *dbs_error, iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_GET_LLID_DBA_TYPE_11:
            ulResultLen = sizeof(OLT_RLT) + sizeof(short int) * 2;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int type;
                short int *dba_type = pRpcResult + 1;
                short int *dbs_error = dba_type + 1;
                
                if ( 0 == (iRet = OLT_GetLLIDdbaType( olt_id, pRpcCmd->llid, &type, dbs_error )) )
                {
                    *dba_type = (short int)type;
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDdbaType(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *dba_type, *dbs_error, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_LLID_DBA_FLAG_12:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetLLIDdbaFlags( olt_id, pRpcCmd->llid, pRpcCmd->params.usVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDdbaFlags(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.usVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_DBA_FLAG_13:
            ulResultLen = sizeof(OLT_RLT) + sizeof(unsigned short);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                unsigned short *dba_flag = pRpcResult + 1;
                
                iRet = OLT_GetLLIDdbaFlags( olt_id, pRpcCmd->llid, dba_flag );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDdbaFlags(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *dba_flag, iRet, ulSrcSlot);
            }
          
            break;	
#endif


#if 1
/* -------------------OLT ONU 管理API------------------- */
        case OLT_CMD_GET_ALL_ONUNUM_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piNumber = pRpcResult + 1;
                
                iRet = OLT_GetOnuNum( olt_id, pRpcCmd->params.iVal, piNumber);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuNum(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *piNumber, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_ALL_ONU_2:
        {
            int iOnuNum = 0;
            OLT_onu_table_t *pTbl = &olt_onu_global_table;

            ulResultLen = sizeof(OLT_RLT);
            iRet = OLT_GetAllOnus( olt_id, pTbl );
            if ( 0 == iRet )
            {
                ulResultLen += sizeof(pTbl->onu_num);
                if ( 0 < (iOnuNum = pTbl->onu_num) )
                {
					/*modified by wangjiahe@2016-09-09*/
                    ulResultLen += iOnuNum * sizeof(/*PON_onu_parameters_t*/PAS_onu_parameters_record_t);
                }
            }
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_onu_table_t *pTbl2 = (OLT_onu_table_t*)(pRpcResult+1);

                if ( 0 < (pTbl2->onu_num = iOnuNum) )
                {
                    VOS_MemCpy(pTbl2->onus, pTbl->onus, ulResultLen - sizeof(OLT_RLT) - sizeof(pTbl->onu_num));
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetAllOnus(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, iOnuNum, iRet, ulSrcSlot);
        }

        break;
        case OLT_CMD_CLEAR_ALL_ONU_3:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ClearAllOnus( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ClearAllOnus(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_RESUME_ALL_ONU_STATUS_4:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResumeAllOnuStatus( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_ALLONU_AUTH_MODE_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuAuthMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuAuthMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_ONU_AUTH_MODE_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuAuthMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuAuthMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_ONU_MAC_AUTH_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetMacAuth( olt_id, (int)(pRpcCmd->params.aucVal[0]), &(pRpcCmd->params.aucVal[1]));
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetMacAuth(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, (bool)(pRpcCmd->params.aucVal[0]), iRet, ulSrcSlot);
            }
          
            break;
	 
        case OLT_CMD_SET_ALLONU_BIND_MODE_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuBindMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuBindMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_CHK_ONU_REG_CTRL_9:
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                short int *psBindOltId;
                unsigned char *pbMacAddr;

                pbMacAddr   = pRpcCmd->params.aucVal;
                psBindOltId = pRpcResult + 1;
                iRet = OLT_ChkOnuRegisterControl( olt_id, pRpcCmd->llid, pbMacAddr, psBindOltId );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ChkOnuRegisterControl(%d, %d, [%02x-%02x-%02x-%02x-%02x-%02x], %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pbMacAddr[0], pbMacAddr[1], pbMacAddr[2], pbMacAddr[3], pbMacAddr[4], pbMacAddr[5], *psBindOltId, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_SET_ALLONU_DEFAULT_BW_10:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                ONU_bw_t *bw = (ONU_bw_t*)(&pRpcCmd->params);
                
                iRet = OLT_SetAllOnuDefaultBW( olt_id, bw );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuDefaultBW(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, bw->bw_gr, bw->bw_be, bw->bw_fixed, bw->bw_actived, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_MODE_11:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuDownlinkPoliceMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuDownlinkPoliceMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_ONU_DOWNLINK_POLICE_MODE_12:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuDownlinkPoliceMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuDownlinkPoliceMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_PARAM_13:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuDownlinkPoliceParam( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuDownlinkPoliceParam(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_ALLONU_UPLINK_DBA_PARAM_14:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuUplinkDBAParam( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuUplinkDBAParam(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_ONU_DOWNLINK_PRI2COS_15:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuDownlinkPri2CoSQueueMap( olt_id, &pRpcCmd->params);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuDownlinkPri2CoSQueueMap(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_ACTIVE_PENDING_ONU_16:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ActivePendingOnu( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ActivePendingOnu(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_ACTIVE_ONE_PENDING_ONU_17:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ActiveOnePendingOnu( olt_id, pRpcCmd->params.aucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ActiveOnePendingOnu(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_ACTIVE_CONFLICT_PENDING_ONU_18:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ActiveConfPendingOnu( OLT_ID_ALL, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ActiveConfPendingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_ACTIVE_ONE_CONFLICT_PENDING_ONU_19:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ActiveOneConfPendingOnu( OLT_ID_ALL, pRpcCmd->llid, pRpcCmd->params.aucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ActiveOneConfPendingOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_PENDING_ONU_20:
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(pendingOnuList_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetPendingOnuList( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetPendingOnuList(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_GET_UPDATING_ONU_21:		/* added by xieshl 20110110, 问题单11796 */
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(onuUpdateStatusList_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetUpdatingOnuList( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetUpdatingOnuList(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_GET_UPDATED_ONU_22:
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(onuUpdateStatusList_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetUpdatedOnuList( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetUpdatedOnuList(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_GET_UPDATING_COUNTER_23:
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(onu_updating_counter_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetOnuUpdatingStatusBySlot( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuUpdatingStatusBySlot(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_SET_UPDATE_ONU_MSG_24:
        {
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuUpdateMsg( olt_id, &pRpcCmd->params );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuUpdateMsg(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }
        break;
        case OLT_CMD_GET_UPDATE_WAITING_25:
        {
            ulResultLen = sizeof(OLT_RLT) + sizeof(onu_update_waiting_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetOnuUpdateWaiting( olt_id, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuUpdateWaiting(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        }	
        break;
        case OLT_CMD_SET_ALLONU_AUTH_MODE2_26:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuAuthMode2( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuAuthMode2(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;
        case OLT_CMD_SET_ALLONU_BW_PARAM_27:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAllOnuBWParams( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAllOnuBWParams(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
            
        	break;
        case OLT_CMD_SET_ONU_P2P_MODE_28:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuP2PMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuP2PMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;
        case OLT_CMD_GET_ONU_B2P_MODE_29:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *mode = (int*)(pRpcResult + 1);
            
                iRet = OLT_GetOnuB2PMode( olt_id, mode );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuB2PMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *mode, iRet, ulSrcSlot);
            }
            
        	break;
        case OLT_CMD_SET_ONU_B2P_MODE_30:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuB2PMode( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuB2PMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;
            
        /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_GET_ONU_MODE_31:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetOnuMode( olt_id, pRpcCmd->llid);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
            
        	break;
		case OLT_CMD_GET_MAC_ADDRESS_AUTHENTICATION_32:
		{
			mac_addresses_list_t   mac_addresses_list;
			unsigned char addr_num = 0;
			ulResultLen = sizeof(OLT_RLT);
	        iRet = OLT_GetMACAddressAuthentication( olt_id, &addr_num, mac_addresses_list );
	        if ( (addr_num > 0) )
	        {
	            ulResultLen += addr_num * sizeof(mac_address_t);
	        }
	        if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
	        {
	            if ( OLT_ERR_OK == iRet )
	            {
	                if ( (0 < (iRet = addr_num))  )
	                {
	                    VOS_MemCpy(pRpcResult+1, mac_addresses_list, ulResultLen - sizeof(OLT_RLT));
	                }
	            }
	        }
			break;
		}
				
		case OLT_CMD_SET_AUTHORIZE_MAC_Aaccording_LIST_33:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAuthorizeMacAddressAccordingListMode( olt_id, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAuthorizeMacAddressAccordingListMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;
		}
		case OLT_CMD_GET_AUTHORIZE_MAC_Aaccording_LIST_34:
		{
			ulResultLen = sizeof(OLT_RLT)+ sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetAuthorizeMacAddressAccordingListMode( olt_id, pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetAuthorizeMacAddressAccordingListMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *(pRpcResult+1), iRet, ulSrcSlot);
            }
            
        	break;
		}
		case OLT_CMD_GET_DOWNLINK_BUFFER_CONFIG_35:
		{
			ulResultLen = sizeof(OLT_RLT)+ sizeof(PON_downlink_buffer_priority_limits_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetDownlinkBufferConfiguration( olt_id, pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetDownlinkBufferConfiguration(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
            
        	break;
		}
		
			
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
        /*Begin:for onu swap by jinhl@2013-02-22*/
		case OLT_CMD_RESUME_LLID_STATUS_37:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResumeLLIDStatus( olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResumeLLIDStatus(%d, LLID:%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SEARCH_FREE_ONUIDX_38:
			ulResultLen = sizeof(OLT_RLT)+ sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SearchFreeOnuIdx( olt_id, (char *)&(pRpcCmd->params), pRpcResult+1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SearchFreeOnuIdx(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
			break;
		case OLT_CMD_GET_ONUIDX_BYMAC_39:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetActOnuIdxByMac( olt_id, (char *)&(pRpcCmd->params));
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuIdxByMac(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
			break;
        /*End:for onu swap by jinhl@2013-02-22*/
        case OLT_CMD_BROADCAST_CLICOMMAND_40:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_BroadCast_CliCommand( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_BroadCast_CliCommand(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;

        case OLT_CMD_SET_AUTH_Entry_41:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAuthEntry( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAuthEntry(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;

        case OLT_CMD_SET_ONU_DEFAULT_MAX_MAC_42:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuDefaultMaxMac( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuDefaultMaxMac(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;

		case OLT_CMD_CTC_SET_PORT_STATS_TIME_OUT_43:
			ulResultLen = sizeof(OLT_RLT);
        	if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
        	{
            	iRet = OLT_SetCTCOnuPortStatsTimeOut( olt_id,pRpcCmd->params.iVals[0], pRpcCmd->params.uiVals[1] );
            	OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetCTCOnuPortStatsTimeOut(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id,  pRpcCmd->params.iVals[0], pRpcCmd->params.uiVals[1], iRet, ulSrcSlot);
        	}
			
			break;      

        case OLT_CMD_SET_MAX_ONU_44:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetMaxOnu( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetMaxOnu(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            
        	break;

        case OLT_CMD_GET_ONU_CONFIG_DEL_STATUS_45:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                char *name = (char *)(pRpcCmd->params.aucVal); 
                int  *status = pRpcResult + 1;
                iRet = OLT_GetOnuConfDelStatus( olt_id, name, status );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOnuConfDelStatus(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
            
        	break;
		case OLT_CMD_SET_ONU_TX_PORWER_SUPPLY_CONTROL_46:
			 ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
				short int onu_id = *(short int *)(&pRpcCmd->params.aucVal[0]);
				CTC_STACK_onu_tx_power_supply_control_t *parameter = (CTC_STACK_onu_tx_power_supply_control_t * )(&pRpcCmd->params.aucVal[sizeof(short int)]);
				 	
				iRet = OLT_SetCTCONUTxPowerSupplyControl( olt_id, onu_id,parameter);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetCTCONUTxPowerSupplyControl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, onu_id, iRet, ulSrcSlot);
            }
            
        	break;
        case OLT_CMD_SET_GPON_AUTH_Entry_47:                
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                gpon_onu_auth_entry_t* entry = (gpon_onu_auth_entry_t*)(&pRpcCmd->params.aucVal[0]);
                iRet = OLT_SetGponAuthEntry( olt_id, entry);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetGponAuthEntry(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
        break;
#endif


#if 1
/* -------------------OLT 加密管理API------------------- */
        case OLT_CMD_SET_ENCRYPT_MODE_1:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetEncryptMode( olt_id, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetEncryptMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
		/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_SET_ENCRYPTION_PREAMBLE_MODE_2:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetEncryptionPreambleMode( olt_id, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetEncryptionPreambleMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
		}
		case OLT_CMD_GET_ENCRYPTION_PREAMBLE_MODE_3:
		{
			ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetEncryptionPreambleMode( olt_id, pRpcResult + 1);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetEncryptionPreambleMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *(pRpcResult + 1), iRet, ulSrcSlot);
            }
          
            break;
		}
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
        case OLT_CMD_GET_LLID_ENCRYPT_MODE_4:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *mode = pRpcResult + 1;
                
                iRet = OLT_GetLLIDEncryptMode( olt_id, pRpcCmd->llid, mode );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDEncryptMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *mode, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_START_LLID_ENCRYPT_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_StartLLIDEncrypt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_StartLLIDEncrypt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_FINISH_LLID_ENCRYPT_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_FinishLLIDEncrypt( olt_id, pRpcCmd->llid, pRpcCmd->params.sVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_FinishLLIDEncrypt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_STOP_LLID_ENCRYPT_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_StopLLIDEncrypt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_StopLLIDEncrypt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_LLID_ENCRYPT_KEY_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetLLIDEncryptKey( olt_id, pRpcCmd->llid, pRpcCmd->params.aucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDEncryptKey(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_FINISH_LLID_ENCRYPT_KEY_9:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_FinishLLIDEncryptKey( olt_id, pRpcCmd->llid, pRpcCmd->params.sVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_FinishLLIDEncryptKey(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
#endif


#if 1
/* -------------------OLT 地址表管理API------------------- */
        case OLT_CMD_SET_MAC_AGINGTIME_1:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetMacAgingTime( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetMacAgingTime(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_ADDR_TBL_CFG_2:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_addr_table_cfg_t *pCfg = &pRpcCmd->params;
                
                iRet = OLT_SetAddressTableConfig( olt_id, pCfg );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAddressTableConfig(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pCfg->removed_when_full, pCfg->discard_llid_unlearned_sa, pCfg->discard_unknown_da, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_ADDR_TBL_CFG_3:
            ulResultLen = sizeof(OLT_RLT) + sizeof(OLT_addr_table_cfg_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_addr_table_cfg_t *pCfg = pRpcResult + 1;
                
                iRet = OLT_GetAddressTableConfig( olt_id, pCfg );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetAddressTableConfig(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pCfg->removed_when_full, pCfg->discard_llid_unlearned_sa, pCfg->discard_unknown_da, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_ADDR_TBL_4:
        {
            short int sAddrNum = 0;
            short int sIsOnlyNum;
            PON_address_record_t *pMacAddrTbl;

            if ( 0 == (sIsOnlyNum = pRpcCmd->llid) )
            {
                pMacAddrTbl = Mac_addr_table;
            }
            else
            {
                pMacAddrTbl = NULL;
            }

            ulResultLen = sizeof(OLT_RLT);
            iRet = OLT_GetMacAddrTbl( olt_id, &sAddrNum, pMacAddrTbl );
            if ( (sAddrNum > 0) && (pMacAddrTbl != NULL) )
            {
            	if(sAddrNum > PON_ADDRESS_TABLE_SIZE)
					sAddrNumTemp = PON_ADDRESS_TABLE_SIZE;
				else
					sAddrNumTemp = sAddrNum;
				
				ulResultLen += sAddrNumTemp * sizeof(PON_address_record_t);
			}
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( OLT_ERR_OK == iRet )
                {
                    if ( (0 < (iRet = sAddrNum)) && (pMacAddrTbl != NULL) )
                    {
                        VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(OLT_RLT));
                    }
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetMacAddrTbl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, sAddrNum, iRet, ulSrcSlot);
        }
        break;
        case OLT_CMD_ADD_ADDR_TBL_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( pRpcCmd->llid >= 0 )
                {
                    iRet = OLT_AddMacAddrTbl( OLT_ID_ALL, pRpcCmd->llid, &pRpcCmd->params );
                }
                else
                {
                    iRet = OLT_AddMacAddrTbl( olt_id, -pRpcCmd->llid, &pRpcCmd->params );
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_AddMacAddrTbl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_DEL_ADDR_TBL_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( pRpcCmd->llid >= 0 )
                {
                    iRet = OLT_DelMacAddrTbl( OLT_ID_ALL, pRpcCmd->llid, &pRpcCmd->params );
                }
                else
                {
                    iRet = OLT_DelMacAddrTbl( olt_id, -pRpcCmd->llid, &pRpcCmd->params );
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_DelMacAddrTbl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_REMOVE_MAC_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RemoveMac( olt_id, pRpcCmd->params.aucVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RemoveMac(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_RESET_ADDR_TBL_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResetAddrTbl( olt_id, pRpcCmd->llid, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResetAddrTbl(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_ONU_MAC_THRESHOLD_9:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuMacThreshold( olt_id, pRpcCmd->params.ulVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuMacThreshold(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
            }
          
            break;
       case OLT_CMD_SET_ONU_MAC_CHECK_ENABLE_10:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuMacCheckEnable( olt_id, pRpcCmd->params.ulVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuMacCheckEnable(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
            }
	     break;
       case OLT_CMD_SET_ONU_MAC_CHECK_PERIOD_11:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOnuMacCheckPeriod( olt_id, pRpcCmd->params.ulVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOnuMacCheckPeriod(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.ulVal, iRet, ulSrcSlot);
            }
            break;
        /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		case OLT_CMD_SET_ADDRESS_TABLE_FULL_HANDLING_MODE_12:
		{
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetAddressTableFullHandlingMode( olt_id, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAddressTableFullHandlingMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
            break;
		}
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
        case OLT_CMD_GET_LLID_BY_USER_MAC_ADDRESS_13:
        {
            ulResultLen = sizeof(OLT_RLT)+sizeof(short int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                unsigned short *onu_llid = pRpcResult + 1;            
                iRet = OLT_GetLlidByUserMacAddress( olt_id, pRpcCmd->params.aucVal, onu_llid);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLlidByUserMacAddress(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
            break;
        }

		case OLT_CMD_GET_ADDR_VLAN_TBL_14:/*for GPON by jinhl*/
        {
            short int sAddrNum = 0;
            short int sIsOnlyNum;
            PON_address_vlan_record_t *pMacAddrTbl;

            if ( 0 == (sIsOnlyNum = pRpcCmd->llid) )
            {
                pMacAddrTbl = Mac_addr_vlan_table;
            }
            else
            {
                pMacAddrTbl = NULL;
            }

            ulResultLen = sizeof(OLT_RLT);
            iRet = OLT_GetMacAddrVlanTbl( olt_id, &sAddrNum, pMacAddrTbl );
            if ( (sAddrNum > 0) && (pMacAddrTbl != NULL) )
            {
            	if(sAddrNum > PON_ADDRESS_TABLE_SIZE)
					sAddrNumTemp = PON_ADDRESS_TABLE_SIZE;
				else
					sAddrNumTemp = sAddrNum;
				
				ulResultLen += sAddrNumTemp * sizeof(PON_address_vlan_record_t);
			}
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( OLT_ERR_OK == iRet )
                {
                    if ( (0 < (iRet = sAddrNum)) && (pMacAddrTbl != NULL) )
                    {
                        VOS_MemCpy(pRpcResult+1, pMacAddrTbl, ulResultLen - sizeof(OLT_RLT));
                    }
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetMacAddrVlanTbl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, sAddrNum, iRet, ulSrcSlot);
        }
        break;
#endif


#if 1
/* -------------------OLT 光路管理API------------------- */
        case OLT_CMD_GET_OPTICAL_CAPABILITY_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(OLT_optical_capability_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_optical_capability_t *pCap = pRpcResult + 1;
                
                iRet = OLT_GetOpticalCapability( olt_id, pCap);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pCap->agc_lock_time, pCap->cdr_lock_time, pCap->laser_on_time, pCap->laser_off_time, pCap->pon_tx_signal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_OPTICS_DETAIL_2:
            ulResultLen = sizeof(OLT_RLT) + sizeof(OLT_optics_detail_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                OLT_optics_detail_t *pDetail = pRpcResult + 1;
                
                iRet = OLT_GetOpticsDetail( olt_id, pDetail);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOpticsDetail(%d, %d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pDetail->pon_optics_params.agc_lock_time, pDetail->pon_optics_params.cdr_lock_time, pDetail->pon_optics_params.discovery_laser_on_time, pDetail->pon_optics_params.discovery_laser_off_time, pDetail->pon_tx_signal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_MAX_RTT_3:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPonRange( olt_id, pRpcCmd->params.uiVals[0], pRpcCmd->params.uiVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPonRange(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.uiVals[0], pRpcCmd->params.uiVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_OPTICAL_TX_MODE_4:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOpticalTxMode( olt_id, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOpticalTxMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_OPTICAL_TX_MODE_5:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *tx_mode = pRpcResult + 1;
                
                iRet = OLT_GetOpticalTxMode( olt_id, tx_mode );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetOpticalTxMode(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *tx_mode, iRet, ulSrcSlot);
            }
          
            break;

        case OLT_CMD_SET_VIRTUAL_SCOPE_ADC_CONFIG_6:
			{
				ulResultLen = sizeof(OLT_RLT);
	            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
	            {
	                iRet = OLT_SetVirtualScopeAdcConfig( olt_id, (PON_adc_config_t*)pRpcCmd->params.aucVal);
	            }
				break;
			}
        case OLT_CMD_GET_VIRTUAL_SCOPE_MEASUREMENT_7:
			{
				ulResultLen = sizeof(OLT_RLT);
				if(pRpcCmd->params.iVals[2] > 0)
				{
				    ulResultLen += pRpcCmd->params.iVals[2];
				}
	            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
	            {
				    if(pRpcCmd->params.iVals[1] > 0)
			    	{
			    	    iRet = OLT_GetVirtualScopeMeasurement(olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], (void *)&pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[1], pRpcResult + 1, pRpcCmd->params.iVals[2]);
			    	}
					else
					{
					    iRet = OLT_GetVirtualScopeMeasurement(olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], NULL, 0, pRpcResult + 1, pRpcCmd->params.iVals[2]);
					}
	            }
				break;
			}
        case OLT_CMD_GET_VIRTUAL_SCOPE_RSSI_MEASUREMENT_8:
            ulResultLen = sizeof(OLT_RLT) + sizeof(PON_rssi_result_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                PON_rssi_result_t *pstRlt = pRpcResult + 1;
            
                iRet = OLT_GetVirtualScopeRssiMeasurement( olt_id, pRpcCmd->llid, pstRlt );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetVirtualScopeRssiMeasurement(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, (int)pstRlt->dbm, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_GET_VIRTUAL_SCOPE_ONU_VOLTAGE_9:
            ulResultLen = sizeof(OLT_RLT) + sizeof(float)*2 + sizeof(unsigned short);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                float *pfVals = pRpcResult + 1;
                    
                iRet = OLT_GetVirtualScopeOnuVoltage( olt_id, pRpcCmd->llid, pfVals, pfVals + 2, pfVals + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetVirtualScopeOnuVoltage(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, (int)pfVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_VIRTUAL_LLID_10:
			{
				ulResultLen = sizeof(OLT_RLT);
	            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
	            {
	                iRet = OLT_SetVirtualLLID( olt_id, pRpcCmd->llid, pRpcCmd->params.iVal );
	                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetVirtualLLID(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
	            }
	            
	        	break;
			}	
			
        case OLT_CMD_SET_OPTICAL_TX_MODE2_11:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetOpticalTxMode2( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetOpticalTxMode2(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;
#endif


#if 1
/* -------------------OLT 监控统计管理API------------------- */
        case OLT_CMD_GET_RAW_STAT_1:
        {
            OLT_raw_stat_item_t *stat_item = &pRpcCmd->params;

            ulResultLen = sizeof(OLT_RLT) + sizeof(OLT_raw_stat_item_t) + stat_item->statistics_data_size;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                VOS_MemCpy(pRpcResult + 1, stat_item, sizeof(OLT_raw_stat_item_t));
                stat_item = pRpcResult + 1;
                stat_item->statistics_data = stat_item + 1; 
                iRet = OLT_GetRawStatistics( olt_id, stat_item );
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetRawStatistics(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRet, ulSrcSlot);
        }
        break;
        case OLT_CMD_RESET_COUNTER_2:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResetCounters( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResetCounters(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_BER_ALARM_3:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetBerAlarm( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetBerAlarm(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_FER_ALARM_4:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetFerAlarm( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetFerAlarm(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_PON_BER_ALARM_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPonBerAlarm( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPonBerAlarm(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_PON_FER_ALARM_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetPonFerAlarm( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetPonFerAlarm(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_BER_ALARM_PARAMS_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetBerAlarmParams( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetBerAlarmParams(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;
        case OLT_CMD_SET_FER_ALARM_PARAMS_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetFerAlarmParams( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetFerAlarmParams(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;
		/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
        case OLT_CMD_SET_ALARM_Config_9:
			ulResultLen = sizeof(OLT_RLT);
	    	if(OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen))
	    	{
	    		iRet = OLT_SetAlarmConfig(olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1],pRpcCmd->params.iVals[2],(void*)&pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[4]);
	    		OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetAlarmConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
	    	}

	    	break;
		case OLT_CMD_GET_STATISTICS_10:
			ulResultLen = sizeof(OLT_RLT) + sizeof(long double);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                             
                iRet = OLT_GetStatistics( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcResult + 1);
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetStatistics(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            break;
		case OLT_CMD_OLT_SELF_TEST_11:
		{
			ulResultLen = sizeof(OLT_RLT);
                     
            iRet = OLT_OltSelfTest( olt_id);
			break;
                
        }
		case OLT_CMD_LINK_TEST_12:
		{
			ulResultLen = sizeof(OLT_RLT)+ sizeof(PON_link_test_results_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_LinkTest( olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], (PON_link_test_vlan_configuration_t *)&pRpcCmd->params.iVals[3],(PON_link_test_results_t*)(pRpcResult+1));
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_LinkTest(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
		}
		/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
        case OLT_CMD_SET_LLID_FEC_MODE_13:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetLLIDFecMode( olt_id, pRpcCmd->llid, pRpcCmd->params.bVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDFecMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.bVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_LLID_FEC_MODE_14:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool) * 3;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *pbOutParams = pRpcResult + 1;
                
                iRet = OLT_GetLLIDFecMode( olt_id, pRpcCmd->llid, pbOutParams, pbOutParams + 1, pbOutParams + 2 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDFecMode(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pbOutParams[0], pbOutParams[1], pbOutParams[2], iRet, ulSrcSlot);
            }
          
            break;	
#endif


#if 1
/* -------------------OLT 倒换API------------------- */
        case OLT_CMD_GET_HOTSWAP_CAP_1:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piMode = (int*)(pRpcResult + 1);
                
                iRet = OLT_GetHotSwapCapability( olt_id, piMode );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetHotSwapCapability(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *piMode, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_HOTSWAP_MODE_2:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int) * 3;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piMode = (int*)(pRpcResult + 1);
                int *piStatus = piMode + 2;
                short int *psPartnerOltId = (short int*)piMode++;

                *psPartnerOltId++ = 0;
                iRet = OLT_GetHotSwapMode( olt_id, psPartnerOltId, piMode, piStatus );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, *psPartnerOltId, *piMode, *piStatus, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_HOTSWAP_MODE_3:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {                
                iRet = OLT_SetHotSwapMode( olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetHotSwapMode(%d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_FORCE_HOTSWAP_PON_4:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ForceHotSwap( olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ForceHotSwap(%d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_HOTSWAP_PARAM_5:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetHotSwapParam( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[4], pRpcCmd->params.iVals[5] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetHotSwapParam(%d, %d, %d, %d, %d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3], pRpcCmd->params.iVals[4], pRpcCmd->params.iVals[5], iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_RDN_ONU_REGISTER_6:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                PON_redundancy_onu_register_t *pOnuInfo = &pRpcCmd->params;
                    
                iRet = OLT_RdnOnuRegister( olt_id, pOnuInfo );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnOnuRegister(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pOnuInfo->onu_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_RDN_CONFIG_7:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetRdnConfig( olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[1], pRpcCmd->params.iVals[2], pRpcCmd->params.iVals[3] );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetRdnConfig(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVals[0], pRpcCmd->params.iVals[2], iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_RDN_SWITCHOVER_8:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RdnSwitchOver( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnSwitchOver(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_RDN_OLT_ISEXIST_9:
            ulResultLen = sizeof(OLT_RLT) + sizeof(bool);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                bool *status = pRpcResult + 1;    
            
                iRet = OLT_RdnIsExist( olt_id, status );
                /* OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnIsExist(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *status, iRet, ulSrcSlot); */
            }
          
            break;	
        case OLT_CMD_RESET_OLT_RDN_RECORD_10:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ResetRdnRecord( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_ResetRdnRecord(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_GET_RDN_OLT_STATE_11:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *state = pRpcResult + 1;    
            
                iRet = OLT_GetRdnState( olt_id, state );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetRdnState(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *state, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SET_RDN_OLT_STATE_12:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetRdnState( olt_id, pRpcCmd->params.iVal );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetRdnState(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_RDN_ADDR_TBL_13:
        {
            short int sAddrNum = 0;

            ulResultLen = sizeof(OLT_RLT);
            iRet = OLT_GetRdnAddrTbl( olt_id, &sAddrNum, Mac_addr_table );
            if ( OLT_ERR_OK == iRet )
            {
                ulResultLen += sAddrNum * sizeof(PON_address_record_t);
            }
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if (  OLT_ERR_OK == iRet  )
                {
                    if ( 0 < (iRet = sAddrNum) )
                    {
                        VOS_MemCpy(pRpcResult+1, Mac_addr_table, ulResultLen-sizeof(OLT_RLT));
                    }
                }
            }
            OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetRdnAddrTbl(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, sAddrNum, iRet, ulSrcSlot);
        }
        break;
        case OLT_CMD_REMOVE_RDN_OLT_14:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RemoveRdnOlt( olt_id );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RemoveRdnOlt(%d)'s result(%d) from slot %d.\r\n", olt_id, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_RDN_LLID_DB_15:
            ulResultLen = sizeof(OLT_RLT) + sizeof(CTC_STACK_redundancy_database_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetLLIDRdnDB( olt_id, pRpcCmd->llid, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDRdnDB(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
            
        case OLT_CMD_SET_RDN_LLID_DB_16:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetLLIDRdnDB( olt_id, pRpcCmd->llid, &pRpcCmd->params );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetLLIDRdnDB(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_REMOVE_RDN_OLT_17:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RdnRemoveOlt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnRemoveOlt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_SWAP_RDN_OLT_18:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RdnSwapOlt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnSwapOlt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_ADD_RDN_OLT_19:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_AddSwapOlt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_AddSwapOlt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_LOOSE_RDN_OLT_20:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_RdnLooseOlt( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnLooseOlt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
            
		/*Begin:for onu swap by jinhl@2013-02-22*/
		case OLT_CMD_RDN_LLID_ADD_21:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                
                iRet = OLT_RdnLLIDAdd( olt_id, pRpcCmd->llid );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnLLIDAdd(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_GET_RDNLLID_MODE_22:
			ulResultLen = sizeof(OLT_RLT) + sizeof(PON_redundancy_llid_redundancy_mode_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                PON_redundancy_llid_redundancy_mode_t *mode = pRpcResult + 1;
                iRet = OLT_GetRdnLLIDMode( olt_id, pRpcCmd->llid, mode );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetRdnLLIDMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, *mode, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_SET_RDNLLID_MODE_23:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetRdnLLIDMode( olt_id, pRpcCmd->llid, pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetRdnLLIDMode(%d, %d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_SET_RDNLLID_STDBYTOACT_24:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_SetRdnLLIDStdbyToAct( olt_id, pRpcCmd->llid, &pRpcCmd->params.iVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetRdnLLIDStdbyToAct(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_SET_RDNLLID_RTT_25:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                
                iRet = OLT_SetRdnLLIDRtt( olt_id, pRpcCmd->llid, pRpcCmd->params.liVal);
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetRdnLLIDRtt(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
		/*End:for onu swap by jinhl@2013-02-22*/
        
        case OLT_CMD_RDN_IS_READY_26:
            ulResultLen = sizeof(OLT_RLT) + sizeof(int);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                int *piIsReady = pRpcResult + 1;
                
                iRet = OLT_RdnIsReady( olt_id, piIsReady );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_RdnIsReady(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, *piIsReady, iRet, ulSrcSlot);
            }
          
            break;	
        case OLT_CMD_GET_RDN_LLID_REGINFO_27:
            ulResultLen = sizeof(OLT_RLT) + sizeof(PON_redundancy_onu_register_t);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_GetLLIDRdnRegisterInfo( olt_id, pRpcCmd->llid, pRpcResult + 1 );
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_GetLLIDRdnRegisterInfo(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->llid, iRet, ulSrcSlot);
            }
          
            break;	
#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
        case OLT_CMD_DUMP_ALL_CMC_1:
            ulResultLen = sizeof(OLT_RLT) + pRpcCmd->params.usVal;
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                char *buf = pRpcResult + 1;
                unsigned short buf_len = pRpcCmd->params.usVal;
                
                if ( 0 == (iRet = OLT_DumpAllCmc( olt_id, buf, &buf_len )) )
                {
                    iRet = buf_len;
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_DumpAllCmc(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, buf_len, iRet, ulSrcSlot);
            }
          
            break;	
		case OLT_CMD_SET_ALL_CMC_SVLAN_2:
			ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                if ( pRpcCmd->params.iVal >= 0 )
                {
                    iRet = OLT_SetCmcServiceVID( OLT_ID_ALL, pRpcCmd->params.iVal );
                }
                else
                {
                    iRet = OLT_SetCmcServiceVID( olt_id, -1 - pRpcCmd->params.iVal );
                }
                OLT_RPC_DEBUG(OLT_RPC_RCV_TITLE"OLT_SetCmcServiceVID(%d, %d)'s result(%d) from slot %d.\r\n", olt_id, pRpcCmd->params.iVal, iRet, ulSrcSlot);
            }
          
            break;	
#endif


        default:
            ulResultLen = sizeof(OLT_RLT);
            if ( OLT_ERR_OK == OLT_RPC_RLT_NEW(pRpcResult, ulResultLen) )
            {
                iRet = OLT_ERR_NOTSUPPORT;
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
            *pulSendDataLen = sizeof(OLT_RLT);
        }
    }
    else
    {
        *ppSendData     = NULL;
        *pulSendDataLen = 0;
    }

    VOS_SysLog(LOG_TYPE_OLT, LOG_DEBUG, "OLT_Rpc_Callback(oltid:%d)'s result(%d) on slot(%d), recv cmd[%d] from slot[%d] ", olt_id, iRet, SYS_LOCAL_MODULE_SLOTNO, pRpcCmd->cmd, ulSrcSlot);
	
    return iRet;
}



/*-----------------------------内部适配----------------------------------------*/


#if 1
/* -------------------OLT基本API------------------- */

static int RPC_Error(short int olt_id)
{
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_Error(%d)'s result(%d).\r\n", olt_id, OLT_ERR_NOTEXIST);

    VOS_ASSERT(0);

    return OLT_ERR_NOTEXIST;
}

static int RPC_IsExist(short int olt_id, bool *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_OLT_IS_EXIST_1, NULL, 0, status, sizeof(bool), 0, 0);
    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_IsExist(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_GetChipTypeID(short int olt_id, int *type)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(type);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_CHIPTYPEID_2, NULL, 0, type, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetChipTypeID(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetChipTypeName(short int olt_id, char typename[OLT_CHIP_NAMELEN])
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(typename);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_CHIPTYPENAME_3, NULL, 0, typename, OLT_CHIP_NAMELEN, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetChipTypeName(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetPon(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESET_PON_4, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetPon(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetPonChip(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

	/*modified by wangjiah@2016-09-01: 
	 * Increase timeout from 10s to 15s 8xep because resetting pon chip need more than 10s */
    //iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESET_PON_CHIP_23, NULL, 0, NULL, 0, 0, 0);
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESET_PON_CHIP_23, NULL, 0, NULL, 0, 15000, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetPonChip(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveOlt(short int olt_id, bool send_shutdown_msg_to_olt, bool reset_olt)
{
    int iRlt;
    int cfg_slot;
    bool CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = send_shutdown_msg_to_olt;
    CmdParam[1] = reset_olt;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_REMOVE_OLT_5, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveOlt(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, send_shutdown_msg_to_olt, reset_olt, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_CopyOlt(short int olt_id, short int dst_olt_id, int copy_flags)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(dst_olt_id);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只拷贝OLT的配置，无需到从实接口处拷贝 */
        iRlt = GW_CopyOlt(olt_id, dst_olt_id, copy_flags);
    }
    else
    {
        iRlt = OLT_Rpc_Call(olt_id, dst_olt_id, &cfg_slot, OLT_CMD_COPY_OLT_6, &copy_flags, sizeof(int), NULL, 0, 20000, OLT_RPC_CALLFLAG_ASYNC);
    }   
    
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CopyOlt(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, dst_olt_id, copy_flags, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_CmdIsSupported(short int olt_id, short int *cmd)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(cmd);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只获取命令状态，无需从实接口处获取 */
        iRlt = GW_CmdIsSupported(olt_id, cmd);
    }
    else
    {
        iRlt = OLT_Rpc_Call(olt_id, *cmd, &cfg_slot, OLT_CMD_IS_SUPPORT_CMD_7, NULL, 0, NULL, 0, 0, 0);
    }    

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_CmdIsSupported(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *cmd, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetDebugMode(short int olt_id, int debug_flags, int debug_mode)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = debug_flags;
    CmdParam[1] = debug_mode;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_PON_DEBUG_MODE_8, CmdParam, sizeof(CmdParam), NULL, 0, 0, OLT_RPC_CALLFLAG_ASYNC);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetDebugMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, debug_flags, debug_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetInitParams(short int olt_id, unsigned short host_olt_manage_type, unsigned short host_olt_manage_address)
{
    int iRlt;
    int cfg_slot;
    unsigned short CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = host_olt_manage_type;
    CmdParam[1] = host_olt_manage_address;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_INIT_PARAMS_9, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetInitParams(%d, %d, 0x%x)'s result(%d) to slot %d.\r\n", olt_id, host_olt_manage_type, host_olt_manage_address, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetSystemParams(short int olt_id, long int statistics_sampling_cycle, long int monitoring_cycle, short int host_olt_msg_timeout, short int olt_reset_timeout)
{
    int iRlt;
    int cfg_slot;
    long int CmdParam[4];

    OLT_ASSERT(olt_id);

    CmdParam[0] = statistics_sampling_cycle;
    CmdParam[1] = monitoring_cycle;
    CmdParam[2] = host_olt_msg_timeout;
    CmdParam[3] = olt_reset_timeout;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_SYSTEM_PARAMS_10, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetSystemParams(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, statistics_sampling_cycle, monitoring_cycle, host_olt_msg_timeout, olt_reset_timeout, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(pon_info);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_PON_I2C_EXTINFO_11, pon_info, sizeof(eeprom_gfa_epon_ext_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonI2CExtInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetPonI2CExtInfo(short int olt_id, eeprom_gfa_epon_ext_t *pon_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(pon_info);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_PON_I2C_EXTINFO_12, NULL, 0, pon_info, sizeof(eeprom_gfa_epon_ext_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPonI2CExtInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(board_info);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_CARD_I2C_INFO_13, board_info, sizeof(board_sysinfo_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCardI2CInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCardI2CInfo(short int olt_id, board_sysinfo_t *board_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(board_info);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_CARD_I2C_INFO_14, NULL, 0, board_info, sizeof(board_sysinfo_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCardI2CInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_WriteMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int value )
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];
    OLT_ASSERT(olt_id);

    CmdParam[0] = phy_address;
	CmdParam[1] = reg_address;
	CmdParam[2] = value;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_WRITE_MDIO_REGISTER_15, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_WriteMdioRegister(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, phy_address, reg_address, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ReadMdioRegister(short int olt_id, short int phy_address, short int reg_address, unsigned short int *value )
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];
    OLT_ASSERT(olt_id);

    CmdParam[0] = phy_address;
	CmdParam[1] = reg_address;
	
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_READ_MDIO_REGISTER_16, CmdParam, sizeof(CmdParam), value, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ReadMdioRegister(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, phy_address, reg_address, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_ReadI2CRegister(short int olt_id, short int device, short int register_address, short int *data )
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];
    OLT_ASSERT(olt_id);

    CmdParam[0] = device;
	CmdParam[1] = register_address;
	
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_READ_I2C_REGISTER_17, CmdParam, sizeof(CmdParam), data, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ReadI2CRegister(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, device, register_address, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

static int RPC_ReadGpio(short int olt_id, int func_id, bool *value)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(value);
	
    iRlt = OLT_Rpc_Call(olt_id, (short int)func_id, &cfg_slot, OLT_CMD_READ_GPIO_19, NULL, 0, value, sizeof(*value), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ReadGpio(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, func_id, *value, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_WriteGpio(short int olt_id, int func_id, bool value)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, (short int)func_id, &cfg_slot, OLT_CMD_WRITE_GPIO_20, &value, sizeof(value), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_WriteGpio(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, func_id, value, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SendChipCli(short int olt_id, unsigned short size, unsigned char *command)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, (short int)size, &cfg_slot, OLT_CMD_SEND_CHIP_CLI_21, command, (ULONG)size, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SendChipCli(%d, %d, %s)'s result(%d) to slot %d.\r\n", olt_id, size, command, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_SetDeviceName(short int olt_id, char* pValBuf, int valLen)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, valLen, &cfg_slot, OLT_CMD_SET_DEVICE_NAME_22, pValBuf, valLen, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetDeviceName(%d, %s, %d)'s result(%d) to slot %d.\r\n", olt_id, pValBuf, valLen, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT PON管理API------------------- */

static int RPC_GetOltVersions(short int olt_id, PON_device_versions_t *device_versions)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(device_versions);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_VERSION_1, NULL, 0, device_versions, sizeof(PON_device_versions_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltVersions(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetDBAVersion(short int olt_id, OLT_DBA_version_t *dba_version)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(dba_version);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_DBA_VERSION_2, NULL, 0, dba_version, sizeof(OLT_DBA_version_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetDBAVersion(%d, %s, %s)'s result(%d) to slot %d.\r\n", olt_id, dba_version->szDBAname, dba_version->szDBAversion, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ChkVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_CHK_VERSION_3, NULL, 0, is_compatibled, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ChkVersion(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *is_compatibled, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ChkDBAVersion(short int olt_id, bool *is_compatibled)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(is_compatibled);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_CHK_DBA_VERSION_4, NULL, 0, is_compatibled, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ChkDBAVersion(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *is_compatibled, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetCniLinkStatus(short int olt_id, bool *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_CNI_LINK_STATUS_5, NULL, 0, status, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetCniLinkStatus(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetPonWorkStatus(short int olt_id, int *work_status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(work_status);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只获取工作状态，无需从实接口处获取 */
        iRlt = GW_GetPonWorkStatus(olt_id, work_status);
    }
    else
    {
        iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_PON_WORKSTATUS_6, NULL, 0, work_status, sizeof(int), 0, 0);
    }

    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPonWorkStatus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *work_status, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_SetAdminStatus(short int olt_id, int admin_status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ADMIN_STATUS_7, &admin_status, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAdminStatus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, admin_status, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetAdminStatus(short int olt_id, int *admin_status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(admin_status);

    if ( OLT_ISLOCAL(olt_id) )
    {
        /* 只获取管理状态，无需从实接口处获取 */
        iRlt = GW_GetAdminStatus(olt_id, admin_status);
    }
    else
    {
        iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ADMIN_STATUS_8, NULL, 0, admin_status, sizeof(int), 0, 0);
    }

    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetAdminStatus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *admin_status, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_SetVlanTpid(short int olt_id, unsigned short int vlan_tpid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_VLAN_TPID_9, &vlan_tpid, sizeof(unsigned short int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVlanTpid(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, vlan_tpid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetVlanQinQ(short int olt_id, OLT_vlan_qinq_t *vlan_qinq_config)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(vlan_qinq_config);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_VLAN_QINQ_10, vlan_qinq_config, sizeof(OLT_vlan_qinq_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVlanQinQ(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, vlan_qinq_config->qinq_direction, vlan_qinq_config->qinq_objectid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonFrameSizeLimit(short int olt_id, short int jumbo_length)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_FRAME_SIZELIMIT_11, &jumbo_length, sizeof(short int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonFrameSizeLimit(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, jumbo_length, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetPonFrameSizeLimit(short int olt_id, short int *jumbo_length)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_FRAME_SIZELIMIT_12, NULL, 0, jumbo_length, sizeof(short int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPonFrameSizeLimit(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *jumbo_length, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_OamIsLimit(short int olt_id, bool *oam_limit)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(oam_limit);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_OAM_IS_LIMIT_13, NULL, 0, oam_limit, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OamIsLimit(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *oam_limit, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_UpdatePonParams(short int olt_id, int max_range, int mac_agetime)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = max_range;
    CmdParam[1] = mac_agetime;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_UPDATE_PON_PARAMS_14, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_UpdatePonParams(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, max_range, mac_agetime, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPPPoERelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt;
    int cfg_slot;
    short int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = set_mode;
    CmdParam[1] = relay_mode;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_PPPOE_RELAY_MODE_15, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPPPoERelayMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, set_mode, relay_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPPPoERelayParams(short int olt_id, short int param_id, int param_value1, int param_value2)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    switch (param_id)
    {
        case PPPOE_RELAY_PARAMID_STRHEAD:
            iRlt = OLT_Rpc_Call(olt_id, param_id, &cfg_slot, OLT_CMD_SET_PPPOE_RELAY_PARAMS_16, (VOID*)param_value1, param_value2+1, NULL, 0, 0, 0);
            break;
        default:
            VOS_ASSERT(0);
            iRlt = OLT_ERR_PARAM;
    }

    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPPPoERelayParams(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, param_id, param_value1, param_value2, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetDhcpRelayMode(short int olt_id, int set_mode, int relay_mode)
{
    int iRlt;
    int cfg_slot;
    short int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = set_mode;
    CmdParam[1] = relay_mode;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_DHCP_RELAY_MODE_17, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetDhcpRelayMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, set_mode, relay_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetIgmpAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_IGMP_AUTH_MODE_18, &auth_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetIgmpAuthMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, auth_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SendFrame2PON(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SEND_FRAME_2_PON_19, eth_frame, frame_len, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SendFrame2PON(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, frame_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SendFrame2CNI(short int olt_id, short int llid, void *eth_frame, int frame_len)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SEND_FRAME_2_CNI_20, eth_frame, frame_len, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SendFrame2CNI(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, frame_len, iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_GetVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid, PON_olt_vid_downlink_config_t *vid_downlink_config)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_VID_DOWNLINK_MODE_21, &vid, sizeof(int), vid_downlink_config, sizeof(PON_olt_vid_downlink_config_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetVidDownlinkMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelVidDownlinkMode(short int olt_id, PON_vlan_tag_t vid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_DEL_VID_DOWNLINK_MODE_22, &vid, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelVidDownlinkMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, vid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOltParameters(short int olt_id, PON_olt_response_parameters_t *olt_parameters)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OLT_PARAMETERS_23, NULL, 0, olt_parameters, sizeof(PON_olt_response_parameters_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltParameters(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_OLT_IGMP_SNOOPING_MODE_24, igmp_configuration, sizeof(PON_olt_igmp_configuration_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOltIgmpSnoopingMode(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOltIgmpSnoopingMode(short int olt_id, PON_olt_igmp_configuration_t *igmp_configuration)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OLT_IGMP_SNOOPING_MODE_25, NULL, 0, igmp_configuration, sizeof(PON_olt_igmp_configuration_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltIgmpSnoopingMode(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_SetOltMldForwardingMode(short int olt_id, disable_enable_t mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_OLT_MLD_FORWARDING_MODE_26, &mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOltMldForwardingMode(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_GetOltMldForwardingMode(short int olt_id, disable_enable_t *mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OLT_MLD_FORWARDING_MODE_27, NULL, 0, mode, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOltMldForwardingMode(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 
static int RPC_SetDBAReportFormat(short int olt_id, PON_DBA_report_format_t report_format)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_DBA_REPORT_FORMAT_28, &report_format, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetDBAReportFormat(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 

static int RPC_GetDBAReportFormat(short int olt_id, PON_DBA_report_format_t *report_format)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_DBA_REPORT_FORMAT_29, NULL, 0, report_format, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetDBAReportFormat(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

/*Begin:for onu swap by jinhl@2013-04-27*/
static int RPC_UpdateProvBWInfo(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_UPDATE_PROV_BWINFO_30, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_UpdateProvBWInfo(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 
/*End:for onu swap by jinhl@2013-04-27*/
#endif


#if 1
/* -------------------OLT LLID管理API------------------- */

static int RPC_LLIDIsExist(short int olt_id, short int llid, bool *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(status);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_LLID_IS_EXIST_1, NULL, 0, status, sizeof(bool), 0, 0);
    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_LLIDIsExist(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, *status, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_DeregisterLLID(short int olt_id, short int llid, bool iswait)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_LLID_DEREGISTER_2, &iswait, sizeof(bool), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DeregisterLLID(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDMac(short int olt_id, short int llid, mac_address_t onu_mac)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_mac);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_MAC_3, NULL, 0, onu_mac, sizeof(mac_address_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDMac(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDRegisterInfo(short int olt_id, short int llid, onu_registration_info_t *onu_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_info);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_REGINFO_4, NULL, 0, onu_info, sizeof(onu_registration_info_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDRegisterInfo(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AuthorizeLLID(short int olt_id, short int llid, bool auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_AUTHORIZE_LLID_5, &auth_mode, sizeof(auth_mode), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AuthorizeLLID(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, auth_mode, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_SLA_6, SLA, sizeof(LLID_SLA_INFO_t), SLA, sizeof(LLID_SLA_INFO_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDSLA(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDSLA(short int olt_id, short int llid, LLID_SLA_INFO_t *SLA)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(SLA);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_SLA_7, NULL, 0, SLA, sizeof(LLID_SLA_INFO_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDSLA(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_POLICE_8, police, sizeof(LLID_POLICE_INFO_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDPolice(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDPolice(short int olt_id, short int llid, LLID_POLICE_INFO_t *police)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(police);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_POLICE_9, &police->path, sizeof(int), police, sizeof(LLID_POLICE_INFO_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDPolice(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLLIDdbaType(short int olt_id, short int llid, int dba_type, short int *dba_error)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_error);

    CmdParam[0] = dba_type;
    CmdParam[1] = *dba_error;
    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_DBA_TYPE_10, CmdParam, sizeof(CmdParam), dba_error, sizeof(short int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDdbaType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, dba_type, *dba_error, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDdbaType(short int olt_id, short int llid, int *dba_type, short int *dba_error)
{
    int iRlt;
    int cfg_slot;
    short int CmdRlt[2];

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_type);
    VOS_ASSERT(dba_error);

    if ( 0 == (iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_DBA_TYPE_11, NULL, 0, CmdRlt, sizeof(CmdRlt), 0, 0)) )
    {
        *dba_type  = CmdRlt[0];
        *dba_error = CmdRlt[1];
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDdbaType(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, *dba_type, *dba_error, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLLIDdbaFlags(short int olt_id, short int llid, unsigned short dba_flags)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_DBA_FLAG_12, &dba_flags, sizeof(unsigned short), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDdbaFlags(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, dba_flags, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDdbaFlags(short int olt_id, short int llid, unsigned short *dba_flags)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(dba_flags);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_DBA_FLAG_13, NULL, 0, dba_flags, sizeof(unsigned short), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDdbaFlags(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, *dba_flags, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT ONU 管理API------------------- */

static int RPC_GetOnuNum(short int olt_id, int onu_flags, int *onu_number)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_number);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ALL_ONUNUM_1, &onu_flags, sizeof(int), onu_number, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuNum(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_flags, *onu_number, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetAllOnus(short int olt_id, OLT_onu_table_t *onu_table)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_table);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ALL_ONU_2, NULL, 0, onu_table, sizeof(OLT_onu_table_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetAllOnus(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_table->onu_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ClearAllOnus(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_CLEAR_ALL_ONU_3, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ClearAllOnus(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResumeAllOnuStatus(short int olt_id, int resume_reason, int resume_mode)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = resume_reason;
    CmdParam[1] = resume_mode;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESUME_ALL_ONU_STATUS_4, CmdParam, sizeof(CmdParam), NULL, 0, 0, OLT_RPC_CALLFLAG_ASYNC);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResumeAllOnuStatus(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, resume_reason, resume_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_AUTH_MODE_5, &auth_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuAuthMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, auth_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuAuthMode(short int olt_id, int auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_AUTH_MODE_6, &auth_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuAuthMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, auth_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetMacAuth(short int olt_id, int mode, mac_address_t mac)
{
    int iRlt;
    int cfg_slot;
    unsigned char CmdParam[7];

    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);

    CmdParam[0] = (unsigned char)mode;
    VOS_MemCpy(&CmdParam[1], mac, 6);
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_MAC_AUTH_7, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMacAuth(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuBindMode(short int olt_id, int bind_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_BIND_MODE_8, &bind_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuBindMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, bind_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ChkOnuRegisterControl(short int olt_id, short int llid, mac_address_t mac_address, short int *bind_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(mac_address);
    VOS_ASSERT(bind_olt_id);
    
    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_CHK_ONU_REG_CTRL_9, mac_address, BYTES_IN_MAC_ADDRESS, bind_olt_id, sizeof(short int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ChkOnuRegisterControl(%d, %d, [%02x-%02x-%02x-%02x-%02x-%02x], %d)'s result(%d) to slot %d.\r\n", olt_id, llid, mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5], *bind_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuDefaultBW(short int olt_id, ONU_bw_t *default_bw)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(default_bw);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_DEFAULT_BW_10, default_bw, sizeof(ONU_bw_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuDefaultBW(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, default_bw->bw_gr, default_bw->bw_be, default_bw->bw_fixed, default_bw->bw_actived, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuDownlinkPoliceMode(short int olt_id, int police_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_MODE_11, &police_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuDownlinkPoliceMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, police_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuDownlinkPoliceMode(short int olt_id, int police_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_DOWNLINK_POLICE_MODE_12, &police_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDownlinkPoliceMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, police_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuDownlinkPoliceParam(short int olt_id, int BwBurstSize, int BwPreference, int BwWeightSize)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);

    CmdParam[0] = BwBurstSize;
    CmdParam[1] = BwPreference;
    CmdParam[2] = BwWeightSize;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_DOWNLINK_POLICE_PARAM_13, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuDownlinkPoliceParam(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, BwBurstSize, BwPreference, BwWeightSize, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuUplinkDBAParam(short int olt_id, int BwFixedPktSize, int BwBurstSize, int BwWeightSize)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);

    CmdParam[0] = BwFixedPktSize;
    CmdParam[1] = BwBurstSize;
    CmdParam[2] = BwWeightSize;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_UPLINK_DBA_PARAM_14, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuUplinkDBAParam(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, BwFixedPktSize, BwBurstSize, BwWeightSize, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuDownlinkPri2CoSQueueMap(short int olt_id, OLT_pri2cosqueue_map_t *map)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(map);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_DOWNLINK_PRI2COS_15, map, sizeof(OLT_pri2cosqueue_map_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDownlinkPri2CoSQueueMap(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

/* active pending onu */
static int RPC_ActivePendingOnu(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_ACTIVE_PENDING_ONU_16, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ActivePendingOnu(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ActiveOnePendingOnu(short int olt_id, unsigned char *mac)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_ACTIVE_ONE_PENDING_ONU_17, mac,GPON_ONU_SERIAL_NUM_STR_LEN, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ActiveOnePendingOnu(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ActiveConfPendingOnu(short int olt_id, short int conf_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, conf_olt_id, &cfg_slot, OLT_CMD_ACTIVE_CONFLICT_PENDING_ONU_18, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ActiveConfPendingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, conf_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ActiveOneConfPendingOnu(short int olt_id, short int conf_olt_id, mac_address_t mac)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);

    iRlt = OLT_Rpc_Call(olt_id, conf_olt_id, &cfg_slot, OLT_CMD_ACTIVE_ONE_CONFLICT_PENDING_ONU_19, mac, GPON_ONU_SERIAL_NUM_STR_LEN, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ActiveOneConfPendingOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, conf_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetPendingOnuList(short int olt_id, pendingOnuList_t *onuList)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_PENDING_ONU_20, NULL, 0, onuList, sizeof(pendingOnuList_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetPendingOnuList(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetUpdatingOnuList(short int olt_id, pendingOnuList_t *onuList)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_UPDATING_ONU_21, NULL, 0, onuList, sizeof(onuUpdateStatusList_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetUpdatingOnuList(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetUpdatedOnuList(short int olt_id, pendingOnuList_t *onuList)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_UPDATED_ONU_22, NULL, 0, onuList, sizeof(onuUpdateStatusList_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetUpdatedOnuList(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetUpdatingStatusOnu(short int olt_id, onu_updating_counter_t *onuList)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onuList);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_UPDATING_COUNTER_23, NULL, 0, onuList, sizeof(onu_updating_counter_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetUpdatingStatusOnu(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetUpdateOnuMsg(short int olt_id, onu_update_msg_t *pMsg)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(pMsg);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_UPDATE_ONU_MSG_24, pMsg, sizeof(onu_update_msg_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetUpdateOnuMsg(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetUpdateWaiting(short int olt_id, onu_update_waiting_t *pList)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(pList);
    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_UPDATE_WAITING_25, NULL, 0, pList, sizeof(onu_update_waiting_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetUpdateWaiting(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuAuthMode2(short int olt_id, int auth_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_AUTH_MODE2_26, &auth_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuAuthMode2(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, auth_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAllOnuBWParams(short int olt_id, int uplink_bwradio, int dwlink_bwradio)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = uplink_bwradio;
    CmdParam[1] = dwlink_bwradio;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALLONU_BW_PARAM_27, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAllOnuBWParams(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, uplink_bwradio, dwlink_bwradio, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuP2PMode(short int olt_id, int p2p_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_P2P_MODE_28, &p2p_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuP2PMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, p2p_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOnuB2PMode(short int olt_id, int *b2p_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ONU_B2P_MODE_29, NULL, 0, b2p_mode, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuB2PMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *b2p_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuB2PMode(short int olt_id, int b2p_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_B2P_MODE_30, &b2p_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuB2PMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, b2p_mode, iRlt, cfg_slot);

    return iRlt;
}


/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_GetOnuMode(short int olt_id, short int onu_id)
{
    int iRlt;
    int cfg_slot;
    

    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, onu_id, &cfg_slot, OLT_CMD_GET_ONU_MODE_31, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMACAddressAuthentication(short int olt_id, unsigned char	*number_of_mac_address, mac_addresses_list_t mac_addresses_list)
{
    int iRlt;
    int cfg_slot;
    

    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_MAC_ADDRESS_AUTHENTICATION_32, NULL, 0, mac_addresses_list, sizeof(mac_addresses_list_t), 0, 0);
	if (iRlt >= 0)
	{
	   	*number_of_mac_address = iRlt;
	}


	
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMACAddressAuthentication(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *number_of_mac_address, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_SetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	authentication_according_to_list)
{
    int iRlt;
    int cfg_slot;
    
    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_AUTHORIZE_MAC_Aaccording_LIST_33, &authentication_according_to_list, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAuthorizeMacAddressAccordingListMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, authentication_according_to_list, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetAuthorizeMacAddressAccordingListMode (short int olt_id, bool	*authentication_according_to_list)
{
    int iRlt;
    int cfg_slot;
    
    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_AUTHORIZE_MAC_Aaccording_LIST_34, NULL, 0, authentication_according_to_list, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetAuthorizeMacAddressAccordingListMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *authentication_according_to_list, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetDownlinkBufferConfiguration(short int olt_id, PON_downlink_buffer_priority_limits_t *priority_limits)
{
    int iRlt;
    int cfg_slot;
    
    OLT_ASSERT(olt_id);

    
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_DOWNLINK_BUFFER_CONFIG_35, NULL, 0, priority_limits, sizeof(PON_downlink_buffer_priority_limits_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetDownlinkBufferConfiguration(%d)'s result(%d) to slot %d.\r\n", olt_id,  iRlt, cfg_slot);

    return iRlt;
}

/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

static int RPC_ResumeLLIDStatus(short int olt_id, short int llid, int resume_reason, int resume_mode)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = resume_reason;
    CmdParam[1] = resume_mode;
    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_RESUME_LLID_STATUS_37, CmdParam, sizeof(CmdParam), NULL, 0, 0, OLT_RPC_CALLFLAG_ASYNC);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResumeLLIDStatus(%d, llid:%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, resume_reason, resume_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SearchFreeOnuIdx(short int olt_id, unsigned char *MacAddress, short int *reg_flag)
{
    int onuIdx;
    int cfg_slot;
    

    OLT_ASSERT(olt_id);
    
    onuIdx = OLT_Rpc_Call(olt_id, 0, &cfg_slot, OLT_CMD_SEARCH_FREE_ONUIDX_38, MacAddress, BYTES_IN_MAC_ADDRESS, reg_flag, sizeof(short int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SearchFreeOnuIdx(%d)'s result(%d) to slot %d.\r\n", olt_id, onuIdx, cfg_slot);

    return onuIdx;
}


/*Begin:for onu swap by jinhl@2013-04-27*/
static int RPC_GetActOnuIdxByMac(short int olt_id, unsigned char *MacAddress)
{
    int onuIdx;
    int cfg_slot;
    

    OLT_ASSERT(olt_id);
    
    onuIdx = OLT_Rpc_Call(olt_id, 0, &cfg_slot, OLT_CMD_GET_ONUIDX_BYMAC_39, MacAddress, BYTES_IN_MAC_ADDRESS, 0, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuIdxByMac(%d)'s result(%d) to slot %d.\r\n", olt_id, onuIdx, cfg_slot);

    return onuIdx;
}
/*End:for onu swap by jinhl@2013-04-27*/

static int RPC_BroadCast_CliCommand(short int olt_id, int action_code)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_BROADCAST_CLICOMMAND_40, &action_code, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_Broadcast_CliCommand(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, action_code, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAuthEntry(short int olt_id, int code)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_AUTH_Entry_41, &code, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAuthEntry(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, code, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetGponAuthEntry(short int olt_id, gpon_onu_auth_entry_t *entry)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_GPON_AUTH_Entry_47, entry, sizeof(gpon_onu_auth_entry_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetGponAuthEntry(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOnuDefaultMaxMac(short int olt_id, int max_mac)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_DEFAULT_MAX_MAC_42, &max_mac, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuDefaultMaxMac(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, max_mac, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCTCOnuPortStatsTimeOut(short int olt_id, ONU_PORTSTATS_TIMER_NAME_E timer_name,LONG timeout)
{
	int iRlt;
    int cfg_slot;
	int CmdParam[2];

	CmdParam[0] = timer_name;
    CmdParam[1] = timeout;

    OLT_ASSERT(olt_id);
  
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot,OLT_CMD_CTC_SET_PORT_STATS_TIME_OUT_43,CmdParam, sizeof(CmdParam),NULL, 0, 0, 0); 
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCTCOnuPortStatsTimeOut(%d,%d,%d)'s result(%d) to slot %d.\r\n", olt_id, timer_name,timeout,iRlt,cfg_slot);

    return iRlt;
}
static int RPC_SetMaxOnu(short int olt_id, int max_onu)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_MAX_ONU_44, &max_onu, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMaxOnu(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, max_onu, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_GetOnuConfDelStatus(short int olt_id, char *name, int *status)
{
    int iRlt;
    int cfg_slot;
    int len = 0;
    OLT_ASSERT(olt_id);

    len = strlen(name);
    if(len>20)
        len = 20;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ONU_CONFIG_DEL_STATUS_45, name, len, status, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOnuConfDelStatus(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCTCOnuTxPowerSupplyControl(short int olt_id, short int onu_id, CTC_STACK_onu_tx_power_supply_control_t * parameter)
{
    int iRlt;
    int cfg_slot;
    int len = 0;
	char cmd_buffer[sizeof(short int)+sizeof(CTC_STACK_onu_tx_power_supply_control_t)+sizeof(bool)];
	
    OLT_ASSERT(olt_id);

	*((short int*)(&cmd_buffer[0])) = onu_id;
	VOS_MemCpy(&cmd_buffer[sizeof(short int)],parameter,sizeof(CTC_STACK_onu_tx_power_supply_control_t));
 
	iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot,OLT_CMD_SET_ONU_TX_PORWER_SUPPLY_CONTROL_46,cmd_buffer, sizeof(cmd_buffer),NULL, 0, 0, 0); 
	
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetONUTxPowerSupplyControl(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}


#endif


#if 1
/* -------------------OLT 加密管理API------------------- */

static int RPC_SetEncryptMode(short int olt_id, int encrypt_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ENCRYPT_MODE_1, &encrypt_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEncryptMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, encrypt_mode, iRlt, cfg_slot);

    return iRlt;
}
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_SetEncryptionPreambleMode(short int olt_id, bool encrypt_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ENCRYPTION_PREAMBLE_MODE_2, &encrypt_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetEncryptionPreambleMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, encrypt_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetEncryptionPreambleMode(short int olt_id, bool *encrypt_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ENCRYPTION_PREAMBLE_MODE_3, NULL, 0, encrypt_mode, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetEncryptionPreambleMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *encrypt_mode, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/


static int RPC_GetLLIDEncryptMode(short int olt_id, short int llid, bool *encrypt_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(encrypt_mode);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_ENCRYPT_MODE_4, NULL, 0, encrypt_mode, sizeof(bool), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDEncryptMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, *encrypt_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_StartLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_START_LLID_ENCRYPT_5, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StartLLIDEncrypt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_FinishLLIDEncrypt(short int olt_id, short int llid, short int status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_FINISH_LLID_ENCRYPT_6, &status, sizeof(short int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_FinishLLIDEncrypt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_StopLLIDEncrypt(short int olt_id, short int llid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_STOP_LLID_ENCRYPT_7, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_StopLLIDEncrypt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLLIDEncryptKey(short int olt_id, short int llid, PON_encryption_key_t key)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(key);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_ENCRYPT_KEY_8, key, sizeof(PON_encryption_key_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDEncryptKey(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_FinishLLIDEncryptKey(short int olt_id, short int llid, short int status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_FINISH_LLID_ENCRYPT_KEY_9, &status, sizeof(short int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_FinishLLIDEncryptKey(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT 地址表管理API------------------- */

static int RPC_SetMacAgingTime(short int olt_id, int aging_time)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_MAC_AGINGTIME_1, &aging_time, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetMacAgingTime(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, aging_time, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ADDR_TBL_CFG_2, addrtbl_cfg, sizeof(OLT_addr_table_cfg_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAddressTableConfig(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetAddressTableConfig(short int olt_id, OLT_addr_table_cfg_t *addrtbl_cfg)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_ADDR_TBL_CFG_3, NULL, 0, addrtbl_cfg, sizeof(OLT_addr_table_cfg_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetAddressTableConfig(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, addrtbl_cfg->removed_when_full, addrtbl_cfg->discard_llid_unlearned_sa, addrtbl_cfg->discard_unknown_da, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetMacAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    VOID *pRcvBuf;
    short int sIsOnlyNum;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);

    if ( NULL != addr_tbl )
    {
        sIsOnlyNum = 0;
        pRcvBuf = addr_tbl;
        iRcvBufLen = sizeof(PON_address_table_t);
    }
    else
    {
        sIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
    
    if( 0 <= (iRlt = OLT_Rpc_Call(olt_id, sIsOnlyNum, &cfg_slot, OLT_CMD_GET_ADDR_TBL_4, NULL, 0, pRcvBuf, iRcvBufLen, 0, 0)) )
    {
        *addr_num = (short int)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMacAddrTbl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *addr_num, iRlt, cfg_slot);

    return iRlt;
}


static int RPC_GetMacAddrVlanTbl(short int olt_id, short int *addr_num, PON_address_vlan_table_t addr_tbl)
{
    int iRlt;
    int cfg_slot;
    int iRcvBufLen;
    VOID *pRcvBuf;
    short int sIsOnlyNum;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);

    if ( NULL != addr_tbl )
    {
        sIsOnlyNum = 0;
        pRcvBuf = addr_tbl;
        iRcvBufLen = sizeof(PON_address_vlan_table_t);
    }
    else
    {
        sIsOnlyNum = 1;
        pRcvBuf = NULL;
        iRcvBufLen = 0;
    }
    
    if( 0 <= (iRlt = OLT_Rpc_Call(olt_id, sIsOnlyNum, &cfg_slot, OLT_CMD_GET_ADDR_VLAN_TBL_14, NULL, 0, pRcvBuf, iRcvBufLen, 0, 0)) )
    {
        *addr_num = (short int)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetMacAddrVlanTbl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    iRlt = OLT_Rpc_Call(olt_id, addr_num, &cfg_slot, OLT_CMD_ADD_ADDR_TBL_5, addr_tbl, ((addr_num > 0) ? addr_num : -addr_num) * sizeof(PON_address_record_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddMacAddrTbl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_DelMacAddrTbl(short int olt_id, short int addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num != 0);
    VOS_ASSERT(addr_tbl);

    iRlt = OLT_Rpc_Call(olt_id, addr_num, &cfg_slot, OLT_CMD_DEL_ADDR_TBL_6, addr_tbl, ((addr_num > 0) ? addr_num : -addr_num) * sizeof(PON_address_record_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DelMacAddrTbl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveMac(short int olt_id, mac_address_t mac)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_REMOVE_MAC_7, mac, sizeof(mac_address_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveMac(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetAddrTbl(short int olt_id, short int llid, int addr_type)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_RESET_ADDR_TBL_8, &addr_type, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetAddrTbl(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, addr_type, iRlt, cfg_slot);

    return iRlt;
}

/*设置ONU MAC 的最大门限*/
static int RPC_SetOnuMacThreshold(short int olt_id, ULONG  mac_threshold)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(mac_threshold != 0);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_MAC_THRESHOLD_9, &mac_threshold, sizeof(ULONG), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuMacThreshold(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, mac_threshold, iRlt, cfg_slot);

    return iRlt;
}

/*add by sxh20120210*/
static int RPC_SetOnuMacCheckEnable(short int olt_id, ULONG  enable)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    /*VOS_ASSERT(enable != 0);*/	/* removed by xieshl 20120329, 问题单14843 */

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_MAC_CHECK_ENABLE_10, &enable, sizeof(ULONG), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuMacCheckEnable(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, enable, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_SetOnuMacCheckPeriod(short int olt_id, ULONG  period)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(period != 0);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ONU_MAC_CHECK_PERIOD_11, &period, sizeof(ULONG), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOnuMacCheckPeriod(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, period, iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_SetAddressTableFullHandlingMode(short int olt_id, bool remove_entry_when_table_full)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
	
	iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ADDRESS_TABLE_FULL_HANDLING_MODE_12, &remove_entry_when_table_full, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAddressTableFullHandlingMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, remove_entry_when_table_full, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_GetLlidByUserMacAddress(short int olt_id, mac_address_t mac, short int *llid)
{

    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_LLID_BY_USER_MAC_ADDRESS_13, mac, sizeof(mac_address_t), llid, sizeof(short int), 0, 0);
}
#endif


#if 1
/* -------------------OLT 光路管理API------------------- */

static int RPC_GetOpticalCapability(short int olt_id, OLT_optical_capability_t *optical_capability)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(optical_capability);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OPTICAL_CAPABILITY_1, NULL, 0, optical_capability, sizeof(OLT_optical_capability_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOpticalCapability(%d, %d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, optical_capability->agc_lock_time, optical_capability->cdr_lock_time, optical_capability->laser_on_time, optical_capability->laser_off_time, optical_capability->pon_tx_signal, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOpticsDetail(short int olt_id, OLT_optics_detail_t *optics_params) 
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(optics_params);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OPTICS_DETAIL_2, NULL, 0, optics_params, sizeof(OLT_optics_detail_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOpticsDetail(%d, %d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, optics_params->pon_optics_params.agc_lock_time, optics_params->pon_optics_params.cdr_lock_time, optics_params->pon_optics_params.discovery_laser_on_time, optics_params->pon_optics_params.discovery_laser_off_time, optics_params->pon_tx_signal, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonRange(short int olt_id, unsigned int max_range, unsigned int max_rtt)
{
    int iRlt;
    int cfg_slot;
    unsigned int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = max_range;
    CmdParam[1] = max_rtt;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_MAX_RTT_3, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonRange(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, max_range, max_rtt, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetOpticalTxMode(short int olt_id, int tx_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_OPTICAL_TX_MODE_4, &tx_mode, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOpticalTxMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, tx_mode, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetOpticalTxMode(short int olt_id, int *tx_mode)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(tx_mode);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_OPTICAL_TX_MODE_5, NULL, 0, tx_mode, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetOpticalTxMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *tx_mode, iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */
static int RPC_SetVirtualScopeAdcConfig(short int olt_id, PON_adc_config_t *adc_config)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_VIRTUAL_SCOPE_ADC_CONFIG_6, adc_config, sizeof(PON_adc_config_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVirtualScopeAdcConfig(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 

static int RPC_GetVirtualScopeMeasurement(short int olt_id, short int llid, PON_measurement_type_t measurement_type, 
	void *configuration, short int config_len, void *result, short int res_len)
{
    int iRlt;
    int cfg_slot;
    char cmd_buffer[RPC_OLT_CMDBUF_LEN];
    short int slen = 0;
	
    OLT_ASSERT(olt_id);

	*(int*)(cmd_buffer+slen) = measurement_type;
    slen += sizeof(int);

	*(int*)(cmd_buffer+slen) = config_len;
    slen += sizeof(int);

	*(int*)(cmd_buffer+slen) = res_len;
    slen += sizeof(int);

	if(0 < config_len)
	{
	    VOS_MemCpy(cmd_buffer+slen, configuration, res_len);
	    slen += res_len;
	}
    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_VIRTUAL_SCOPE_MEASUREMENT_7, cmd_buffer, slen, result, res_len, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetVirtualScopeMeasurement(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 

static int RPC_GetVirtualScopeRssiMeasurement(short int olt_id, short int llid, PON_rssi_result_t *rssi_result)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(rssi_result);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_VIRTUAL_SCOPE_RSSI_MEASUREMENT_8, NULL, 0, rssi_result, sizeof(*rssi_result), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetVirtualScopeRssiMeasurement(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, (int)rssi_result->dbm, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetVirtualScopeOnuVoltage(short int olt_id, short int llid, float *voltage, unsigned short int *sample, float *dbm)
{
    int iRlt;
    int cfg_slot;
    char cmd_buffer[sizeof(float)*2 + sizeof(unsigned short)];
	
    OLT_ASSERT(olt_id);

    if ( 0 == (iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_VIRTUAL_SCOPE_ONU_VOLTAGE_9, NULL, 0, cmd_buffer, sizeof(cmd_buffer), 0, 0)) )
    {
        VOS_MemCpy(voltage, &cmd_buffer[0], sizeof(float));
        VOS_MemCpy(dbm, &cmd_buffer[sizeof(float)], sizeof(float));
        VOS_MemCpy(sample, &cmd_buffer[sizeof(float) * 2], sizeof(unsigned short));
    }
    
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetVirtualScopeMeasurement(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, (int)*dbm, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetVirtualLLID(short int olt_id, short int llid, PON_virtual_llid_operation_t operation)
{
    int iRlt;
    int cfg_slot;
    
    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_VIRTUAL_LLID_10, &operation, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetVirtualLLID(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, operation, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 change stucture by jinhl @2013-01-21 */

static int RPC_SetOpticalTxMode2(short int olt_id, int tx_mode, int tx_reason)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = tx_mode;
    CmdParam[1] = tx_reason;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_OPTICAL_TX_MODE2_11, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetOpticalTxMode2(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, tx_mode, tx_reason, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT 监控统计管理API------------------- */

static int RPC_GetRawStatistics(short int olt_id, OLT_raw_stat_item_t *stat_item)
{
    int iRlt;
    int cfg_slot;
    OLT_RLT *pCmdResult = NULL;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(stat_item);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_RAW_STAT_1, stat_item, sizeof(OLT_raw_stat_item_t), &pCmdResult, sizeof(OLT_RLT*), 0, OLT_RPC_CALLFLAG_MANUALFREERLT);
    if (NULL != pCmdResult)
    {
        OLT_raw_stat_item_t *stat_rlt = pCmdResult->values;

        /* B--added by liwei056@2010-11-30 for D11323 */
        stat_rlt->statistics_data = stat_rlt + 1;
        /* E--added by liwei056@2010-11-30 for D11323 */

        stat_item->timestam = stat_rlt->timestam;

        VOS_ASSERT(stat_item->statistics_data_size == stat_rlt->statistics_data_size);
        VOS_MemCpy(stat_item->statistics_data, stat_rlt->statistics_data, stat_item->statistics_data_size);
        OLT_RPC_RLT_FREE(pCmdResult);
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetRawStatistics(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, stat_item->collector_id, stat_item->raw_statistics_type, stat_item->statistics_parameter, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ResetCounters(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESET_COUNTER_2, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetCounters(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);

    CmdParam[0] = alarm_switch;
    CmdParam[1] = alarm_thresold;
    CmdParam[2] = alarm_min_error_bytes;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_BER_ALARM_3, &CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetBerAlarm(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
        
    CmdParam[0] = alarm_switch;
    CmdParam[1] = alarm_thresold;
    CmdParam[2] = alarm_min_error_frames;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_FER_ALARM_4, &CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetFerAlarm(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonBerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);

    CmdParam[0] = alarm_switch;
    CmdParam[1] = alarm_thresold;
    CmdParam[2] = alarm_min_error_bytes;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_PON_BER_ALARM_5, &CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonBerAlarm(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_bytes, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetPonFerAlarm(short int olt_id, int alarm_switch, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
        
    CmdParam[0] = alarm_switch;
    CmdParam[1] = alarm_thresold;
    CmdParam[2] = alarm_min_error_frames;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_PON_FER_ALARM_6, &CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetPonFerAlarm(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_switch, alarm_thresold, alarm_min_error_frames, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetBerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_bytes)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = alarm_thresold;
    CmdParam[1] = alarm_min_error_bytes;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_BER_ALARM_PARAMS_7, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetBerAlarmParams(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_thresold, alarm_min_error_bytes, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetFerAlarmParams(short int olt_id, int alarm_thresold, int alarm_min_error_frames)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);

    CmdParam[0] = alarm_thresold;
    CmdParam[1] = alarm_min_error_frames;

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_FER_ALARM_PARAMS_8, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetFerAlarmParams(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, alarm_thresold, alarm_min_error_frames, iRlt, cfg_slot);

    return iRlt;
}

/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static int RPC_SetAlarmConfig(short int olt_id, short int source, PON_alarm_t type, bool activate, void	*configuration, int length)
{
    int iRlt;
    int cfg_slot;
    char cmd_buffer[RPC_OLT_CMDBUF_LEN ];
    int slen = 0;
	
    OLT_ASSERT(olt_id);

    
	*(int*)(cmd_buffer+slen) = source;
    slen += sizeof(int);
    *(int*)(cmd_buffer+slen) = type;
    slen += sizeof(PON_alarm_t);
	*(int*)(cmd_buffer+slen) = activate;
    slen += sizeof(bool);
	VOS_MemCpy(cmd_buffer+slen, configuration, length);
	slen += length;
	*(int*)(cmd_buffer+slen) = length;
    slen += sizeof(int);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALARM_Config_9, cmd_buffer, slen, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetAlarmConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, source, type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetStatistics(short int olt_id, short int collector_id, PON_statistics_t statistics_type, short int statistics_parameter, long double *statistics_data)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
   
    CmdParam[0] = collector_id;
	CmdParam[1] = statistics_type;
	CmdParam[2] = statistics_parameter;
	
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_STATISTICS_10, CmdParam, sizeof(CmdParam), statistics_data, sizeof(long double), 0, 0);
    
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetStatistics(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, collector_id, statistics_type, statistics_parameter, iRlt, cfg_slot);

    return iRlt;
}
static int RPC_OltSelfTest(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_OLT_SELF_TEST_11, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_OltSelfTest(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
} 

static int RPC_LinkTest(short int olt_id, PON_onu_id_t onu_id, short int number_of_frames, short int frame_size, bool link_delay_measurement, PON_link_test_vlan_configuration_t *vlan_configuration, PON_link_test_results_t *test_results)
{
    int iRlt;
    int cfg_slot;
    char cmd_buffer[RPC_OLT_CMDBUF_LEN];
	int slen = 0;
	
    OLT_ASSERT(olt_id);

    *(int*)(cmd_buffer+slen) = number_of_frames;
    slen += sizeof(int);
	*(int*)(cmd_buffer+slen) = frame_size;
    slen += sizeof(int);
	*(int*)(cmd_buffer+slen) = link_delay_measurement;
    slen += sizeof(int);
	VOS_MemCpy(cmd_buffer+slen, vlan_configuration, sizeof(PON_link_test_vlan_configuration_t));
	slen += sizeof(PON_link_test_vlan_configuration_t);
    
    iRlt = OLT_Rpc_Call(olt_id, onu_id, &cfg_slot, OLT_CMD_LINK_TEST_12, cmd_buffer, slen, test_results, sizeof(PON_link_test_results_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_LinkTest(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_id, iRlt, cfg_slot);

    return iRlt;
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

static int RPC_SetLLIDFecMode(short int olt_id, short int llid, bool downlink_fec)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_LLID_FEC_MODE_13, &downlink_fec, sizeof(bool), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDFecMode(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, downlink_fec, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDFecMode(short int olt_id, short int llid, bool *downlink_fec, bool *uplink_fec, bool *uplink_lastframe_fec)
{
    int iRlt;
    int cfg_slot;
    bool CmdRlt[3];

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(downlink_fec);
    VOS_ASSERT(uplink_fec);
    VOS_ASSERT(uplink_lastframe_fec);

    if ( 0 == (iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_LLID_FEC_MODE_14, NULL, 0, CmdRlt, sizeof(CmdRlt), 0, 0)) )
    {
        *downlink_fec         = CmdRlt[0];
        *uplink_fec           = CmdRlt[1];
        *uplink_lastframe_fec = CmdRlt[2];
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDFecMode(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, *downlink_fec, *uplink_fec, *uplink_lastframe_fec, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if 1
/* -------------------OLT 倒换API------------------- */

static int RPC_GetHotSwapCapability(short int olt_id, int *swap_cap)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(swap_cap);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_HOTSWAP_CAP_1, NULL, 0, swap_cap, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetHotSwapCapability(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *swap_cap, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetHotSwapMode(short int olt_id, short int *partner_olt_id, int *swap_mode, int *swap_status)
{
    int iRlt;
    int cfg_slot;
    int CmdRlt[3];

    OLT_ASSERT(olt_id);
    VOS_ASSERT(partner_olt_id);
    VOS_ASSERT(swap_mode);
    VOS_ASSERT(swap_status);

    if ( 0 == (iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_HOTSWAP_MODE_2, NULL, 0, CmdRlt, sizeof(CmdRlt), 0, 0)) )
    {
        *partner_olt_id = CmdRlt[0];
        *swap_mode      = CmdRlt[1];
        *swap_status    = CmdRlt[2];
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetHotSwapMode(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, *partner_olt_id, *swap_mode, *swap_status, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetHotSwapMode(short int olt_id, short int partner_olt_id, int swap_mode, int swap_status, int swap_flags)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[3];

    OLT_ASSERT(olt_id);
    OLT_ID_ASSERT(partner_olt_id);

    CmdParam[0] = swap_mode;
    CmdParam[1] = swap_status;
    CmdParam[2] = swap_flags;

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_SET_HOTSWAP_MODE_3, CmdParam, sizeof(CmdParam), NULL, 0, 20000, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetHotSwapMode(%d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, swap_mode, swap_status, swap_flags, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_ForceHotSwap(short int olt_id, short int partner_olt_id, int swap_status, int swap_flags)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[2];

    OLT_ASSERT(olt_id);
    OLT_ID_ASSERT(partner_olt_id);

    CmdParam[0] = swap_status;
    CmdParam[1] = swap_flags;

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_FORCE_HOTSWAP_PON_4, CmdParam, sizeof(CmdParam), NULL, 0, 20000, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ForceHotSwap(%d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, swap_status, swap_flags, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetHotSwapParam(short int olt_id, int swap_enable, int swap_time, int rpc_mode, int swap_triggers, int trigger_param1, int trigger_param2)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[6];

    OLT_ASSERT(olt_id);

    CmdParam[0] = swap_enable;
    CmdParam[1] = swap_time;
    CmdParam[2] = rpc_mode;
    CmdParam[3] = swap_triggers;
    CmdParam[4] = trigger_param1;
    CmdParam[5] = trigger_param2;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_HOTSWAP_PARAM_5, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetHotSwapParam(%d, %d, %d, %d, %d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, swap_enable, swap_time, rpc_mode, swap_triggers, trigger_param1, trigger_param2, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnOnuRegister(short int olt_id, PON_redundancy_onu_register_t *onu_reg_info)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(onu_reg_info);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RDN_ONU_REGISTER_6, onu_reg_info, sizeof(PON_redundancy_onu_register_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnOnuRegister(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, onu_reg_info->onu_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetRdnConfig(short int olt_id, int rdn_status, int gpio_num, int rdn_type, int rx_enable)
{
    int iRlt;
    int cfg_slot;
    int CmdParam[4];

    OLT_ASSERT(olt_id);

    CmdParam[0] = rdn_status;
    CmdParam[1] = gpio_num;
    CmdParam[2] = rdn_type;
    CmdParam[3] = rx_enable;
    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_RDN_CONFIG_7, CmdParam, sizeof(CmdParam), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetRdnConfig(%d, %d, %d)'s result(%d) to slot %d.\r\n", olt_id, rdn_status, rdn_type, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnSwitchOver(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RDN_SWITCHOVER_8, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnSwitchOver(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnIsExist(short int olt_id, bool *status)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(status);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RDN_OLT_ISEXIST_9, NULL, 0, status, sizeof(bool), 0, 0);
    /* OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnIsExist(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *status, iRlt, cfg_slot); */

    return iRlt;
}

static int RPC_ResetRdnRecord(short int olt_id, int rdn_state)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RESET_OLT_RDN_RECORD_10, &rdn_state, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_ResetRdnRecord(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, rdn_state, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetRdnState(short int olt_id, int *state)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(state);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_RDN_OLT_STATE_11, NULL, 0, state, sizeof(int), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetRdnState(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *state, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetRdnState(short int olt_id, int state)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_RDN_OLT_STATE_12, &state, sizeof(int), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetRdnState(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, state, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetRdnAddrTbl(short int olt_id, short int *addr_num, PON_address_table_t addr_tbl)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(addr_num);
    VOS_ASSERT(addr_tbl);

    if( 0 <= (iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_GET_RDN_ADDR_TBL_13, NULL, 0, addr_tbl, sizeof(PON_address_table_t), 0, 0)) )
    {
        *addr_num = (short int)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetRdnAddrTbl(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *addr_num, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RemoveRdnOlt(short int olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_REMOVE_RDN_OLT_14, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RemoveRdnOlt(%d)'s result(%d) to slot %d.\r\n", olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_RDN_LLID_DB_15, NULL, 0, rdn_db, sizeof(CTC_STACK_redundancy_database_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDRdnDB(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetLLIDRdnDB(short int olt_id, short int llid, CTC_STACK_redundancy_database_t *rdn_db)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(rdn_db);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_RDN_LLID_DB_16, rdn_db, sizeof(CTC_STACK_redundancy_database_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetLLIDRdnDB(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnRemoveOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_REMOVE_RDN_OLT_17, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnRemoveOlt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_SWAP_RDN_OLT_18, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnSwapOlt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_AddSwapOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_ADD_RDN_OLT_19, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_AddSwapOlt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_RdnLooseOlt(short int olt_id, short int partner_olt_id)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    OLT_ASSERT(partner_olt_id);

    iRlt = OLT_Rpc_Call(olt_id, partner_olt_id, &cfg_slot, OLT_CMD_LOOSE_RDN_OLT_20, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnLooseOlt(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, partner_olt_id, iRlt, cfg_slot);

    return iRlt;
}

/*Begin:for onu swap by jinhl@2013-02-22*/
static int RPC_RdnLLIDAdd(short int olt_id, short int llid)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
	

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_RDN_LLID_ADD_21, NULL, 0, NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnLLIDAdd(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_GetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t* mode)
{
    int iRlt;
    int cfg_slot;
	

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
	VOS_ASSERT(mode);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_RDNLLID_MODE_22, NULL, 0, mode, sizeof(PON_redundancy_llid_redundancy_mode_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetRdnLLIDMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}   

static int RPC_SetRdnLLIDMode(short int olt_id, short int llid, PON_redundancy_llid_redundancy_mode_t mode)
{
    int iRlt;
    int cfg_slot;
	

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
	VOS_ASSERT(mode);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_RDNLLID_MODE_23, &mode, sizeof(PON_redundancy_llid_redundancy_mode_t), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetRdnLLIDMode(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}   
static int RPC_SetRdnLLIDStdbyToAct(short int olt_id, short int llid_n, short int* llid_list_marker)
{
    int iRlt;
    int cfg_slot;
	

    OLT_ASSERT(olt_id);
    
	VOS_ASSERT(llid_list_marker);
    iRlt = OLT_Rpc_Call(olt_id, llid_n, &cfg_slot, OLT_CMD_SET_RDNLLID_STDBYTOACT_24, llid_list_marker, llid_n*sizeof(int), NULL, 0, 0, 0);
	
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetRdnLLIDStdbyToAct(%d)'s result(%d) to slot %d.\r\n", olt_id,  iRlt, cfg_slot);

    return iRlt;
}   

static int RPC_SetRdnLLIDRtt(short int olt_id, short int llid, PON_rtt_t rtt)
{
    int iRlt;
    int cfg_slot;
	

    OLT_ASSERT(olt_id);
    	

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_SET_RDNLLID_RTT_25, &rtt, sizeof(rtt), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetRdnLLIDRtt(%d)'s result(%d) to slot %d.\r\n", olt_id,  iRlt, cfg_slot);

    return iRlt;
}   
/*End:for onu swap by jinhl@2013-02-22*/


static int RPC_RdnIsReady(short int olt_id, int *iIsReady)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(iIsReady);

    if ( OLT_ISLOCAL(olt_id) )
    {
        if ( 0 == (iRlt = GW_RdnIsReady(olt_id, iIsReady)) )
        {
            if ( TRUE == *iIsReady )
            {
                /* 获取倒换就绪状态，必需从实接口处获取 */
                iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RDN_IS_READY_26, NULL, 0, iIsReady, sizeof(int), 0, 0);
            }
            else
            {
                /* 只获取未就绪倒换状态，无需从实接口处获取 */
                cfg_slot = SYS_LOCAL_MODULE_SLOTNO;
            }
        }
    }
    else
    {       
        iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_RDN_IS_READY_26, NULL, 0, iIsReady, sizeof(int), 0, 0);
    }
    
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_RdnIsReady(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *iIsReady, iRlt, cfg_slot);
    
    return iRlt;
}

static int RPC_GetLLIDRdnRegisterInfo(short int olt_id, short int llid, PON_redundancy_onu_register_t *onu_reginfo)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    LLID_ASSERT(llid);
    VOS_ASSERT(onu_reginfo);

    iRlt = OLT_Rpc_Call(olt_id, llid, &cfg_slot, OLT_CMD_GET_RDN_LLID_REGINFO_27, NULL, 0, onu_reginfo, sizeof(PON_redundancy_onu_register_t), 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_GetLLIDRdnRegisterInfo(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, llid, iRlt, cfg_slot);

    return iRlt;
}

#endif


#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */

static int RPC_DumpAllCmc(short int olt_id, char *dump_buf, unsigned short *dump_len)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);
    VOS_ASSERT(dump_buf);
    VOS_ASSERT(dump_len);

    if ( 0 <= (iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_DUMP_ALL_CMC_1, dump_len, sizeof(*dump_len), dump_buf, *dump_len, 0, 0)) )
    {
        *dump_len = (unsigned short)iRlt;
        iRlt = 0;
    }
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_DumpAllCmc(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, *dump_len, iRlt, cfg_slot);

    return iRlt;
}

static int RPC_SetCmcServiceVID(short int olt_id, int svlan)
{
    int iRlt;
    int cfg_slot;

    OLT_ASSERT(olt_id);

    iRlt = OLT_Rpc_Call(olt_id, -1, &cfg_slot, OLT_CMD_SET_ALL_CMC_SVLAN_2, &svlan, sizeof(svlan), NULL, 0, 0, 0);
    OLT_RPC_DEBUG(OLT_RPC_SND_TITLE"RPC_SetCmcServiceVID(%d, %d)'s result(%d) to slot %d.\r\n", olt_id, svlan, iRlt, cfg_slot);

    return iRlt;
}

#endif


#undef OLT_RPC_CMD_NEW 
#undef OLT_RPC_RLT_NEW 



/*-----------------------------外部接口----------------------------------------*/

static const OltMgmtIFs s_rpcIfs = {
#if 1
/* -------------------OLT基本API------------------- */
    RPC_IsExist,
    RPC_GetChipTypeID,
    RPC_GetChipTypeName,
    RPC_ResetPon,
    RPC_RemoveOlt,
    
    RPC_CopyOlt,
    RPC_CmdIsSupported,
    RPC_SetDebugMode,
    RPC_SetInitParams,
    RPC_SetSystemParams,    

    RPC_SetPonI2CExtInfo,
    RPC_GetPonI2CExtInfo,
    RPC_SetCardI2CInfo,
    RPC_GetCardI2CInfo,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_WriteMdioRegister,

    RPC_ReadMdioRegister,
    RPC_ReadI2CRegister,
    RPC_Error,            /*GpioAccess*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_ReadGpio,
    RPC_WriteGpio,

    RPC_SendChipCli,
    RPC_SetDeviceName,
    RPC_ResetPonChip,    /*ResetPonChip*/
#endif

#if 1
/* -------------------OLT PON管理API--------------- */
    RPC_GetOltVersions,
    RPC_GetDBAVersion,
    RPC_ChkVersion,
    RPC_ChkDBAVersion,
    RPC_GetCniLinkStatus,

    RPC_GetPonWorkStatus,
    RPC_SetAdminStatus,
    RPC_GetAdminStatus,
    RPC_SetVlanTpid,
    RPC_SetVlanQinQ,

    RPC_SetPonFrameSizeLimit,
    RPC_GetPonFrameSizeLimit,
    RPC_OamIsLimit,
    RPC_UpdatePonParams,
    RPC_SetPPPoERelayMode,    

    RPC_SetPPPoERelayParams,    
    RPC_SetDhcpRelayMode,    
    RPC_SetIgmpAuthMode,    
    RPC_SendFrame2PON,    
    RPC_SendFrame2CNI,    

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_GetVidDownlinkMode,
    RPC_DelVidDownlinkMode,
    RPC_GetOltParameters,
    RPC_SetOltIgmpSnoopingMode,
    RPC_GetOltIgmpSnoopingMode,

    RPC_SetOltMldForwardingMode,
    RPC_GetOltMldForwardingMode,
    RPC_SetDBAReportFormat,
    RPC_GetDBAReportFormat,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-04-27*/
    RPC_UpdateProvBWInfo,
    /*End:for onu swap by jinhl@2013-04-27*/
#endif

#if 1
/* -------------------OLT LLID管理API------------------- */
    RPC_LLIDIsExist,
    RPC_DeregisterLLID,
    RPC_GetLLIDMac,
    RPC_GetLLIDRegisterInfo,
    RPC_AuthorizeLLID,

    RPC_SetLLIDSLA,
    RPC_GetLLIDSLA,
    RPC_SetLLIDPolice,
    RPC_GetLLIDPolice,
    RPC_SetLLIDdbaType,
    
    RPC_GetLLIDdbaType,
    RPC_SetLLIDdbaFlags,
    RPC_GetLLIDdbaFlags,
    RPC_Error,            /* GetLLIDHeartbeatOam */
    RPC_Error,            /* SetLLIDHeartbeatOam */
#endif

#if 1
/* -------------------OLT ONU 管理API-------------- */
    RPC_GetOnuNum,
    RPC_GetAllOnus,
    RPC_ClearAllOnus,
    RPC_ResumeAllOnuStatus,
    RPC_SetAllOnuAuthMode,

    RPC_SetOnuAuthMode,
    RPC_SetMacAuth,
    RPC_SetAllOnuBindMode,
    RPC_ChkOnuRegisterControl,
    RPC_SetAllOnuDefaultBW,    

    RPC_SetAllOnuDownlinkPoliceMode,
    RPC_SetOnuDownlinkPoliceMode,
    RPC_SetAllOnuDownlinkPoliceParam,
    RPC_SetAllOnuUplinkDBAParam,
    RPC_SetOnuDownlinkPri2CoSQueueMap,

    RPC_ActivePendingOnu,
    RPC_ActiveOnePendingOnu,
    RPC_ActiveConfPendingOnu,
    RPC_ActiveOneConfPendingOnu,
    RPC_GetPendingOnuList,

    RPC_GetUpdatingOnuList,
    RPC_GetUpdatedOnuList,
    RPC_GetUpdatingStatusOnu,
    RPC_SetUpdateOnuMsg,
    RPC_GetUpdateWaiting,
    
    RPC_SetAllOnuAuthMode2,
    RPC_SetAllOnuBWParams,
    RPC_SetOnuP2PMode,
    RPC_GetOnuB2PMode,
    RPC_SetOnuB2PMode,

    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_GetOnuMode,
    RPC_GetMACAddressAuthentication,
    RPC_SetAuthorizeMacAddressAccordingListMode,
    RPC_GetAuthorizeMacAddressAccordingListMode,
    RPC_GetDownlinkBufferConfiguration,

    RPC_Error,            /*GetOamInformation*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    /*Begin:for onu swap by jinhl@2013-02-22*/
    RPC_ResumeLLIDStatus,
    RPC_SearchFreeOnuIdx,
    RPC_GetActOnuIdxByMac,
    /*End:for onu swap by jinhl@2013-02-22*/
    RPC_BroadCast_CliCommand,

    RPC_SetAuthEntry,
    RPC_SetGponAuthEntry,
    RPC_SetOnuDefaultMaxMac,    
    RPC_SetCTCOnuPortStatsTimeOut,
    RPC_SetMaxOnu,
    RPC_GetOnuConfDelStatus,
    RPC_SetCTCOnuTxPowerSupplyControl,
#endif

#if 1
/* -------------------OLT 加密管理API----------- */
    RPC_SetEncryptMode,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_SetEncryptionPreambleMode,
    RPC_GetEncryptionPreambleMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_GetLLIDEncryptMode,
    RPC_StartLLIDEncrypt,

    RPC_FinishLLIDEncrypt,
    RPC_StopLLIDEncrypt,
    RPC_SetLLIDEncryptKey,
    RPC_FinishLLIDEncryptKey,
#endif

#if 1
/* -------------------OLT 地址表管理API-------- */
    RPC_SetMacAgingTime,
    RPC_SetAddressTableConfig,
    RPC_GetAddressTableConfig,
    RPC_GetMacAddrTbl,
    RPC_AddMacAddrTbl,
    
    RPC_DelMacAddrTbl,
    RPC_RemoveMac,
    RPC_ResetAddrTbl,
    RPC_SetOnuMacThreshold,
    RPC_SetOnuMacCheckEnable,
    
    RPC_SetOnuMacCheckPeriod,
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_SetAddressTableFullHandlingMode,
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_GetLlidByUserMacAddress,
    RPC_GetMacAddrVlanTbl,/*GetMacAddrVlanTbl*/
#endif
    
#if 1
/* -------------------OLT 光路管理API----------- */
    RPC_GetOpticalCapability,
    RPC_GetOpticsDetail,
    RPC_SetPonRange,
    RPC_SetOpticalTxMode,
    RPC_GetOpticalTxMode,

    RPC_SetVirtualScopeAdcConfig,
    RPC_GetVirtualScopeMeasurement,
    RPC_GetVirtualScopeRssiMeasurement,
    RPC_GetVirtualScopeOnuVoltage,
    RPC_SetVirtualLLID,
    
    RPC_SetOpticalTxMode2,
#endif
    
#if 1
/* -------------------OLT 监控统计管理API---- */
    RPC_GetRawStatistics,
    RPC_ResetCounters,
    RPC_SetBerAlarm,
    RPC_SetFerAlarm,
    RPC_SetPonBerAlarm,

    RPC_SetPonFerAlarm,
    RPC_SetBerAlarmParams,    
    RPC_SetFerAlarmParams,    
    /*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    RPC_SetAlarmConfig,
    RPC_Error,            /*GetAlarmConfig*/
    
    RPC_GetStatistics,
    RPC_Error,            /*OltSelfTest*/
    RPC_Error,            /*LinkTest*/
    RPC_SetLLIDFecMode,
    RPC_GetLLIDFecMode,

    RPC_Error,            /*SysDump*/
    /*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#endif
    
#if 1
/* -------------------OLT 倒换API---------------- */
    RPC_GetHotSwapCapability,
    RPC_GetHotSwapMode,
    RPC_SetHotSwapMode,
    RPC_ForceHotSwap,
    RPC_SetHotSwapParam,
    
    RPC_RdnOnuRegister,
    RPC_SetRdnConfig,
    RPC_RdnSwitchOver,
    RPC_RdnIsExist,
    RPC_ResetRdnRecord,

    RPC_GetRdnState,
    RPC_SetRdnState,
    RPC_GetRdnAddrTbl,
    RPC_RemoveRdnOlt,
    RPC_GetLLIDRdnDB,

    RPC_SetLLIDRdnDB,
    RPC_RdnRemoveOlt,
    RPC_RdnSwapOlt,
    RPC_AddSwapOlt,
    RPC_RdnLooseOlt,
    
    /*Begin:for onu swap by jinhl@2013-02-22*/
    RPC_RdnLLIDAdd,
    RPC_GetRdnLLIDMode,
    RPC_SetRdnLLIDMode,
    RPC_SetRdnLLIDStdbyToAct,
    RPC_SetRdnLLIDRtt,
    /*End:for onu swap by jinhl@2013-02-22*/
    
    RPC_RdnIsReady,
    RPC_GetLLIDRdnRegisterInfo,
#endif

#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
/* --------------------OLT CMC管理API------------------- */
    RPC_DumpAllCmc,
    RPC_SetCmcServiceVID,
#endif

    RPC_Error                  /* LastFun */
};


#if 1
/* ----------------异步RPC服务任务池----------------- */

#define RPC_TASKPOOL_SIZE  16

static cl_vector s_RpcTaskParamPool;
static cl_vector s_RpcTaskSemPool;
static cl_vector s_RpcTaskFreePool;
ULONG OLT_RPC_POOL;
ULONG s_ulRpcTaskPoolTop;
ULONG s_ulRpcTaskPoolFree;
static ULONG s_ulRpcTaskPoolMutex;

#define RPCPOOL_LOCK()   VOS_SemTake(s_ulRpcTaskPoolMutex, WAIT_FOREVER)
#define RPCPOOL_UNLOCK() VOS_SemGive(s_ulRpcTaskPoolMutex)

static int OLT_RPC_SyncCall( CDP_SYNC_Header_S *sync_msg )
{
    int iRlt;
    VOID *pReturnData;
    ULONG ulReturnDataLen;

    pReturnData = NULL;
    
    switch ( sync_msg->ulSrcMod )
    {
        case MODULE_OLT:
            iRlt = OLT_Rpc_Callback( sync_msg->ulSrcSlot, sync_msg->ulSrcMod, (VOID*)(sync_msg + 1), sync_msg->ulDataLen, &pReturnData, &ulReturnDataLen );

            break;
        case MODULE_ONU:
            iRlt = ONU_Rpc_Callback( sync_msg->ulSrcSlot, sync_msg->ulSrcMod, (VOID*)(sync_msg + 1), sync_msg->ulDataLen, &pReturnData, &ulReturnDataLen );

            break;
        default:
            iRlt = OLT_ERR_NOTSUPPORT;
    }
    
    if ( NULL != pReturnData )
    {
        OLT_RPC_RLT_FREE(pReturnData);
    }

    return iRlt;
}

static VOID OLT_RPC_Main( ULONG ulArg1, ULONG ulArg2, ULONG ulArg3, ULONG ulArg4, ULONG ulArg5, ULONG ulArg6, ULONG ulArg7, ULONG ulArg8, ULONG ulArg9, ULONG ulArg10 )
{
    ULONG ulTaskIdx;
    ULONG ulTaskSemID;
    CDP_SYNC_Header_S *sync_msg;

    ulTaskIdx   = ulArg1;
    ulTaskSemID = ulArg2;
    do{
        /* 空闲任务等待处理消息 */    
        VOS_SemTake(ulTaskSemID, WAIT_FOREVER);
        
        /* 取得处理消息 */    
        RPCPOOL_LOCK();
        sync_msg = (CDP_SYNC_Header_S*)cl_vector_slot(s_RpcTaskParamPool, ulTaskIdx);
        cl_vector_set_index(s_RpcTaskParamPool, ulTaskIdx, NULL);
        RPCPOOL_UNLOCK();
        
        /* 处理RPC调用消息 */    
        if ( 0 != sync_msg )
        {
            OLT_RPC_SyncCall(sync_msg);
            CDP_FreeMsg(sync_msg);
        }
        else
        {
            VOS_ASSERT(0);
        }
        
        /* 任务空闲入池 */    
        RPCPOOL_LOCK();
        cl_vector_set_index(s_RpcTaskFreePool, s_ulRpcTaskPoolFree++, (void*)ulTaskIdx);
        RPCPOOL_UNLOCK();
    }while( TRUE == OLT_RPC_POOL );
    
    /* 任务释放*/    
    RPCPOOL_LOCK();
    --s_ulRpcTaskPoolFree;
    RPCPOOL_UNLOCK();

    return;
}

static int OLT_RPC_TaskNew( CDP_SYNC_Header_S *sync_msg )
{
    ULONG ulTaskIdx;
    ULONG ulTaskSemID;
    ULONG ulArgv[10] = {0};
    CHAR  ucTaskName[20];

    do{
        ulTaskSemID = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_EMPTY);
        if ( 0 == ulTaskSemID )
        {
            break;
        }
    
        VOS_Snprintf(ucTaskName, 18, "tOltAsynC%d", s_ulRpcTaskPoolTop);
        ulArgv[0] = s_ulRpcTaskPoolTop;
        ulArgv[1] = ulTaskSemID;
        ulTaskIdx = VOS_TaskCreate(ucTaskName, TASK_PRIORITY_NORMAL + 10, OLT_RPC_Main, ulArgv);
        if ( 0 == ulTaskIdx )
        {
            VOS_SemDelete(ulTaskSemID);
            ulTaskSemID = 0;

            break;
        }

        cl_vector_set_index(s_RpcTaskSemPool, s_ulRpcTaskPoolTop, (void*)ulTaskSemID);
        cl_vector_set_index(s_RpcTaskParamPool, s_ulRpcTaskPoolTop, (void*)sync_msg);
        if ( 0 != sync_msg )
        {
            /* 新任务不入池，直接干活 */
        	VOS_SemGive(ulTaskSemID);
        }
        else
        {
            /* 新建的空闲任务，直接入池 */
            cl_vector_set_index(s_RpcTaskFreePool, s_ulRpcTaskPoolFree++, (void*)s_ulRpcTaskPoolTop);
        }
        
        s_ulRpcTaskPoolTop++;
    }while(0);

    return ulTaskSemID;
}

int OLT_RPC_AsyncCall( CDP_SYNC_Header_S *sync_head )
{
    ULONG ulTaskIdx;
    ULONG ulTaskSemID;
    OLT_CMD_HEAD *pstCmdHead = (OLT_CMD_HEAD*)(sync_head + 1);

    if ( TRUE == OLT_RPC_POOL )
    {
        RPCPOOL_LOCK();
        if ( s_ulRpcTaskPoolFree > 0 )
        {
            /* 任务池有空闲任务可用，空闲任务出池 */
            ulTaskIdx   = (ULONG)cl_vector_slot(s_RpcTaskFreePool, --s_ulRpcTaskPoolFree);
            VOS_ASSERT(ulTaskIdx < s_ulRpcTaskPoolTop);
            ulTaskSemID = (ULONG)cl_vector_slot(s_RpcTaskSemPool, ulTaskIdx);
            VOS_ASSERT(0 < ulTaskSemID);

            /* 为空闲任务设置处理参数 */
            cl_vector_set_index(s_RpcTaskParamPool, ulTaskIdx, (void*)sync_head);
            
            /* 通知空闲任务干活 */
        	VOS_SemGive(ulTaskSemID);
        }
        else
        {
            /* 任务池无空闲任务可用，需添加新任务干活 */
            ulTaskSemID = OLT_RPC_TaskNew(sync_head);
        }
        RPCPOOL_UNLOCK();
    }
    else
    {
        ulTaskSemID = 0;
    }

    if ( 0 == ulTaskSemID )
    {
        /* 任务池失效，恢复单任务处理*/
        /* 有可能挂死CDP主任务，建议还是丢掉此次处理较好 */
        /* OLT_RPC_SyncCall(sync_head); */
        CDP_FreeMsg(sync_head);

        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"OLT_RPC_AsyncCall(%d->%d, cmd:%d, olt:%d, llid:%d) failed to enter async-task.", sync_head->ulSrcSlot, sync_head->ulDstSlot, pstCmdHead->cmd, pstCmdHead->olt_id, pstCmdHead->llid );
        return OLT_ERR_NORESC;
    }
    else
    {
        OLT_SYNC_DEBUG(OLT_SYNC_TITLE"OLT_RPC_AsyncCall(%d->%d, cmd:%d, olt:%d, llid:%d) run in async-task.", sync_head->ulSrcSlot, sync_head->ulDstSlot, pstCmdHead->cmd, pstCmdHead->olt_id, pstCmdHead->llid );
    }

    return OLT_ERR_OK;
}

int OLT_RPC_TaskPool_Enable()
{
    int hTask;

    RPCPOOL_LOCK();
    
    s_ulRpcTaskPoolTop  = 0;
    s_ulRpcTaskPoolFree = 0;
   
    s_RpcTaskSemPool = cl_vector_init(RPC_TASKPOOL_SIZE);
    VOS_ASSERT(s_RpcTaskSemPool);
    s_RpcTaskParamPool = cl_vector_init(RPC_TASKPOOL_SIZE);
    VOS_ASSERT(s_RpcTaskParamPool);
    s_RpcTaskFreePool = cl_vector_init(RPC_TASKPOOL_SIZE);
    VOS_ASSERT(s_RpcTaskFreePool);
    
    /* 任务池预存5个空闲任务 */
    hTask = OLT_RPC_TaskNew(NULL);
	VOS_ASSERT(hTask);
    hTask = OLT_RPC_TaskNew(NULL);
	VOS_ASSERT(hTask);
    hTask = OLT_RPC_TaskNew(NULL);
	VOS_ASSERT(hTask);
    hTask = OLT_RPC_TaskNew(NULL);
	VOS_ASSERT(hTask);
    hTask = OLT_RPC_TaskNew(NULL);
	VOS_ASSERT(hTask);

    /* 任务池开始生效 */
    OLT_RPC_POOL = TRUE;

    RPCPOOL_UNLOCK();

    return 0;
}

int OLT_RPC_TaskPool_Disable()
{
    ULONG ulTaskID;
    ULONG ulTaskSemID;
    VOID *pTaskParam;

    RPCPOOL_LOCK();
    
    /* 任务池开始失效 */
    OLT_RPC_POOL = FALSE;

    while ( 0 < s_ulRpcTaskPoolTop )
    {
        pTaskParam = cl_vector_slot(s_RpcTaskParamPool, s_ulRpcTaskPoolTop);
        if ( NULL != pTaskParam )
        {
            cl_vector_set_index(s_RpcTaskParamPool, s_ulRpcTaskPoolTop, NULL);
            CDP_FreeMsg(pTaskParam);
        }

        ulTaskSemID = (ULONG)cl_vector_slot(s_RpcTaskSemPool, s_ulRpcTaskPoolTop);
        VOS_SemDelete(ulTaskSemID);

        --s_ulRpcTaskPoolTop;
    }

    RPCPOOL_UNLOCK();

    while ( 0 < s_ulRpcTaskPoolFree )
    {
        VOS_TaskDelay(VOS_TICK_SECOND / 10);
    }

    cl_vector_free(s_RpcTaskSemPool);
    cl_vector_free(s_RpcTaskParamPool);
    cl_vector_free(s_RpcTaskFreePool);

    return 0;
}

int OLT_RPC_TaskPool_Config(int pool_enabled)
{
    int iRlt = OLT_ERR_NOTNEED_CFG;

    if ( pool_enabled != OLT_RPC_POOL )
    {
        if ( pool_enabled )
        {
            iRlt = OLT_RPC_TaskPool_Enable();
        }
        else
        {
            iRlt = OLT_RPC_TaskPool_Disable();
        }
    }

    return iRlt;
}

int OLT_RPC_TaskPool_Init()
{
    OLT_RPC_POOL = 0;

    s_ulRpcTaskPoolMutex = VOS_SemBCreate(VOS_SEM_Q_PRIORITY, VOS_SEM_FULL);
    VOS_ASSERT(s_ulRpcTaskPoolMutex);

    OLT_RPC_TaskPool_Config(TRUE);

    return 0;
}
#endif


void OLT_RPC_Support()
{
    OLT_RegisterAdapter(OLT_ADAPTER_RPC, &s_rpcIfs);

    if( VOS_OK != CDP_SYNC_Register( MODULE_OLT, ( CDP_SYNC_RECEIVE_NOTIFY ) OLT_Rpc_Callback ) )
    {
        VOS_ASSERT( 0 );
    }

    /* RPC异步调用任务池  */
    OLT_RPC_TaskPool_Init();
}


#ifdef __cplusplus

}

#endif


