/***************************************************************
*
*						Module Name:  OltSync.c
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
*   Date: 			2010/08/18
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  10/08/18  |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif


#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"

#include  "ponEventHandler.h"

#include  "../monitor/monitor.h"

#include "vos/vospubh/cdp_pub.h"
#include "vos/vospubh/cdp_syn.h"

#define OLT_SYNC_HANDLE_QUEUE_FULL_LEN   1000
#define OLT_SYNC_HANDLE_QUEUE_BUSY_LEN   800

#define OLT_SYNC_TASK_EVENT_DATA     1
#define OLT_SYNC_TASK_EVENT_TIMEOUT  2

#define OLT_SYNC_RESEND_TIMESPAN        5
#define OLT_SYNC_RESEND_TIMESPAN_MAX    10

#define OLT_SYNC_EVENT_CONNECT      0x08
#define OLT_SYNC_EVENT_DISCONNECT   0x10

#define OLT_SYNC_STATUS_CONN_CLOSE      0x01
#define OLT_SYNC_STATUS_DATA_RESEND     0x02
#define OLT_SYNC_STATUS_CONN_CLEAR      0x08

typedef enum
{
    OLT_SYNC_CMD_UPDATE = 0,
    OLT_SYNC_CMD_CLEARDATA,
    OLT_SYNC_CMD_CONNECT,
    OLT_SYNC_CMD_DISCONNECT,
   
    OLT_SYNC_CMD_MAX    
}OLT_SYNC_CMD;

typedef enum
{
    OLT_SYNC_ROLE_OFFEROR = 0,
    OLT_SYNC_ROLE_COLLECTOR,
   
    OLT_SYNC_ROLE_MAX    
}OLT_SYNC_ROLE;

typedef struct stOltSyncMsgHead
{
    CDP_SYNC_Header_S    sync;
    short int            slot;
    short int            port;
    short int            obj_id;
    int                  src_id;
    int                  dst_id;
    int                  sync_cmd;
    char                 sync_data[0];
}OLT_SYNC_MSG_HEAD;


#ifdef OLT_SYNC_SUPPORT

typedef struct stOltSyncNode
{
    struct stOltSyncNode *pNext;
   
    INT32           src_id;
    INT32           dst_id;
    
    UINT16          Events;
    UINT16          Status;

    UINT16          TimerCurr;
    UINT16          TimerLen;

    INT32           sync_cmd;
    VOID           *sync_data;
    ULONG           sync_len;

    UINT16          retry_time;
    UINT16          retry_limit;

    UINT16          sync_num;
    UINT16          update_num;
    UINT16          ok_num;
    UINT16          loose_num;
    UINT16          send_fail;
    UINT16          recv_fail;

    OLT_SYNC_OBJECT Object;
}OLT_SYNC_NODE;

typedef int (*PF_OLT_SYNC_ONRECV)(void *sync_data, unsigned long sync_datalen);
typedef int (*PF_OLT_SYNC_ONTIMER)(unsigned long ulTimeSecLen);

typedef struct stOltSyncObjectList
{       
    ULONG node_sem;         
    int   node_id;
    OLT_SYNC_NODE *node_list;

    PF_OLT_SYNC_ONRECV   onRecvData;
    PF_OLT_SYNC_ONTIMER  onTimer;  
    
    int iSyncTimerLooseNum;
    int iSyncDataLooseNum;
    int iSyncErrorLooseNum;

    int iSyncExitFlag;    
    int iSyncQueue;
    int iSyncTimer;
    int iSyncTask;
}OLT_SYNC_OBJECT_LIST;

static OLT_SYNC_OBJECT_LIST  synobj_list[OLT_SYNC_ROLE_MAX];


#if 1
void OLT_SYNC_Handle(unsigned long aulMsg[4])
{
    VOS_ASSERT((0 <= aulMsg[0]) && (aulMsg[0] < OLT_SYNC_ROLE_MAX));

    switch (aulMsg[1])
    {
        case OLT_SYNC_TASK_EVENT_DATA:
            VOS_ASSERT(aulMsg[2]);
            VOS_ASSERT(aulMsg[3]);
            (*synobj_list[aulMsg[0]].onRecvData)(aulMsg[2], aulMsg[3]);
            CDP_FreeMsg(aulMsg[2]);
            break;
        case OLT_SYNC_TASK_EVENT_TIMEOUT:
            (*synobj_list[aulMsg[0]].onTimer)(aulMsg[2]);           
            break;
        default:
            VOS_ASSERT(0);
    }

    return;
}

void OLT_SYNC_Task_Run( ULONG ulArg0, ULONG ulArg1, ULONG ulArg2,
                      ULONG ulArg3, ULONG ulArg4, ULONG ulArg5, ULONG ulArg6,
                      ULONG ulArg7, ULONG ulArg8, ULONG ulArg9 )
{
    unsigned long aulMsg[4];
    long result;
    OLT_SYNC_ROLE emSyncRole = ulArg0;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    
    synobj_list[emSyncRole].iSyncTaskExitFlag = 0;
    for(;;)
    {
        result = VOS_QueReceive( synobj_list[emSyncRole].iSyncQueue, aulMsg, WAIT_FOREVER );
        if( result == VOS_ERROR )
        {
        	continue;
        }
        if( result == VOS_NO_MSG ) continue;

        if ( 0 == synobj_list[emSyncRole].iSyncTaskExitFlag )
        {
           	OLT_SYNC_Handle(aulMsg);
        }
        else
        {
            break;
        }
    }

    VOS_SemDelete(synobj_list[emSyncRole].node_sem);
    VOS_TimerDelete(MODULE_OLT, synobj_list[emSyncRole].iSyncTimer);
    synobj_list[emSyncRole].iSyncTask = 0;
            
    return;
}

void OLT_SYNC_Timer_CB(VOID *pArg)
{
    unsigned long aulMsg[4] = {0, OLT_SYNC_TASK_EVENT_TIMEOUT, 0, 0};
    LONG lQueueLen;
    OLT_SYNC_ROLE emSyncRole;

    VOS_ASSERT(pArg);
    emSyncRole = *(int*)pArg;
    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));

    lQueueLen = VOS_QueNum(synobj_list[emSyncRole].iSyncQueue);
    if ( lQueueLen > OLT_SYNC_HANDLE_QUEUE_BUSY_LEN )
    {
        if ( (OLT_SYNC_HANDLE_QUEUE_FULL_LEN == lQueueLen)
            || (OLT_SYNC_ROLE_COLLECTOR == emSyncRole) )
        {
            /* 同步处理较慢: 采集慢会累积定时消息；收集慢会累积同步数据包 */
            /* 因此，采集任务要尽可能多得处理定时消息；收集任务要尽可能少得处理定时消息 */
            synobj_list[emSyncRole].iSyncTimerLooseNum++;
            return;
        }
    }

    aulMsg[0] = emSyncRole;
    aulMsg[2] = synobj_list[emSyncRole].iSyncTimerLooseNum;
    if( VOS_QueSend( synobj_list[emSyncRole].iSyncQueue, aulMsg, NO_WAIT, MSG_PRI_NORMAL ) != VOS_OK )
    {
        synobj_list[emSyncRole].iSyncTimerLooseNum++;
    }
    else
    {
        synobj_list[emSyncRole].iSyncTimerLooseNum = 1;
    }

    return;
}

int OLT_SYNC_IPC_CB(VOID *pData, ULONG ulLen)
{
    unsigned long aulMsg[4] = { 0, OLT_SYNC_TASK_EVENT_DATA, 0, 0};
    LONG lQueueLen;
    OLT_SYNC_MSG_HEAD *pMsgHead = pData;
    OLT_SYNC_ROLE emSyncRole;

    VOS_ASSERT(pMsgHead);
    VOS_ASSERT(ulLen >= sizeof(OLT_SYNC_MSG_HEAD));
    
    emSyncRole = (pMsgHead->dst_id >> 16) & 0xFFFF;
    if ((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX))
    {
        lQueueLen = VOS_QueNum(synobj_list[emSyncRole].iSyncQueue);
        if ( lQueueLen > OLT_SYNC_HANDLE_QUEUE_BUSY_LEN )
        {
            if ( (OLT_SYNC_HANDLE_QUEUE_FULL_LEN == lQueueLen)
                || (OLT_SYNC_CMD_UPDATE == pMsgHead->sync_cmd) )
            {
                /* 同步的接收处理慢于采集的发送处理，则丢同步数据包 */
                CDP_FreeMsg (pData);
                synobj_list[emSyncRole].iSyncDataLooseNum++;
                
                return VOS_ERROR;
            }
        }

        aulMsg[0] = emSyncRole;
        aulMsg[2] = pData;
        aulMsg[3] = ulLen;
        if( VOS_QueSend( synobj_list[emSyncRole].iSyncQueue, aulMsg, NO_WAIT, MSG_PRI_URGENT ) != VOS_OK )
        {
            CDP_FreeMsg (pData);
            synobj_list[emSyncRole].iSyncDataLooseNum++;
            
        	VOS_ASSERT(0);
            return VOS_ERROR;
       }
    }
    else
    {
        /* 丢掉不合理的包 */
        CDP_FreeMsg (pData);
        synobj_list[emSyncRole].iSyncErrorLooseNum++;

        VOS_ASSERT(0);
        return VOS_ERROR;
    }

    return VOS_OK;
}

VOID OLT_IPC_CB (ULONG ulFlag, ULONG ulChID, ULONG ulNode,
       ULONG ulChannelID, VOID *pData, ULONG ulLen)
{
    switch( ulFlag )
    {
        case CDP_NOTI_FLG_RXDATA: /* Receive data*/
             (void)OLT_SYNC_IPC_CB( pData, ulLen );             
             break;
        case CDP_NOTI_FLG_SEND_FINISH:/*Async send data*/
             CDP_FreeMsg (pData);
             break;
        default: break;
    }
    
    return;
}

int OLT_SYNC_Task_Create(OLT_SYNC_ROLE emSyncRole)
{
    ULONG args[ VOS_MAX_TASK_ARGS ] = {0};
    ULONG ulSyncSem;
    int   iSyncQueue, iSyncTask, iSyncTimer;
    CHAR  *aszTaskName[OLT_SYNC_ROLE_MAX][] = {"tOltSyncOfferor", "tOltSyncCollector"};
    OLT_SYNC_OBJECT_LIST *object_list; 

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    
    iSyncQueue = VOS_QueCreate(OLT_SYNC_HANDLE_QUEUE_FULL_LEN, VOS_MSG_Q_PRIORITY);
    if( iSyncQueue  == 0 )
    {
    	VOS_ASSERT( 0 );
    	return VOS_ERROR;
    }

    args[0] = emSyncRole;
    iSyncTask = (int)VOS_TaskCreate(aszTaskName[emSyncRole], TASK_PRIORITY_BELOW_NORMAL, OLT_SYNC_Task_Run, args);
    if( 0 == iSyncTask )
    {
        VOS_QueDelete(iSyncQueue);
        
    	VOS_ASSERT( 0 );
    	return VOS_ERROR;
    }

	VOS_QueBindTask( (VOS_HANDLE)iSyncTask, iSyncQueue );

    /* add a timer, one second, loop */
    iSyncTimer = VOS_TimerCreate(MODULE_OLT, 0, 1000, OLT_SYNC_Timer_CB, (VOID*)&emSyncRole, VOS_TIMER_LOOP);
    if ( VOS_ERROR == iSyncTimer )
    {
        VOS_TaskDelete(iSyncTask);
        
    	VOS_ASSERT( 0 );
    	return VOS_ERROR;
    }
   
    if ( 0 == (ulSyncSem = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL)) )
    {
        VOS_TimerDelete(MODULE_OLT, iSyncTimer);
        VOS_TaskDelete(iSyncTask);
        
    	VOS_ASSERT( 0 );
    	return VOS_ERROR;
    }

    /* 创建OLT同步数据的CDP通道 */
    (void)CDP_Create(RPU_TID_CDP_OLT, CDP_NOTI_VIA_FUNC, 0, OLT_IPC_CB);

    object_list = &synobj_list[emSyncRole];    
    object_list->iSyncTask     = iSyncTask;
    object_list->iSyncQueue    = iSyncQueue;
    object_list->iSyncTimer    = iSyncTimer;
    object_list->iSyncExitFlag = 0;

    object_list->iSyncDataLooseNum  = 0;
    object_list->iSyncTimerLooseNum = 1;

    object_list->node_id   = 0;
    object_list->node_num  = 0;
    object_list->node_list = NULL;
    object_list->node_sem  = ulSyncSem;

    return 0;
}

static int OLT_SYNC_Send_Data(OLT_SYNC_ROLE emSyncRole, OLT_SYNC_NODE *sync_node, int sync_cmd, void *sync_data, unsigned long sync_datalen)
{
    int iRlt = OLT_ERR_OK;
    OLT_SYNC_MSG_HEAD *pMsgHead;
    OLT_SYNC_OBJECT   *sync_obj;
    ULONG ulMsgLen;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(sync_node);

    sync_obj = &sync_node->Object;
    ulMsgLen = sizeof(OLT_SYNC_MSG_HEAD) + sync_datalen;
    pMsgHead = ( OLT_SYNC_MSG_HEAD * ) CDP_AllocMsg( ulMsgLen, MODULE_OLT );              
    if(NULL == pMsgHead)
    {
        return OLT_ERR_MALLOC;
    }

    pMsgHead->slot     = sync_obj->dst_slot;
    pMsgHead->port     = sync_obj->dst_port;
    pMsgHead->obj_id   = sync_obj->obj_id;

    pMsgHead->src_id   = sync_node->src_id;
    pMsgHead->dst_id   = sync_node->dst_id;

    if ( sync_datalen > 0 )
        VOS_MemCpy(pMsgHead->sync_data, sync_data, sync_datalen);

    if ( pMsgHead->slot != SYS_LOCAL_MODULE_SLOTNO )
    {
        /* 板间同步，走CDP板间信道 */
        /* CDP的同步发送，可以保证同步不占满板间信道的带宽 */
        if ( VOS_OK != CDP_Send(RPU_TID_CDP_OLT, pMsgHead->slot, RPU_TID_CDP_OLT, CDP_MSG_TM_SYNC, pMsgHead, ulMsgLen, MODULE_OLT) )
        {
            sync_node->send_fail++;
            
            iRlt = OLT_ERR_TIMEOUT;
        }

        /* CDP的同步发送，必须手动释放消息块 */
    	CDP_FreeMsg( pMsgHead );
    }
    else
    {
        /* 板内同步，直接LPC信道 */
        if ( VOS_OK != OLT_SYNC_IPC_CB(pMsgHead, ulMsgLen) )
        {
            sync_node->send_fail++;
            
            iRlt = OLT_ERR_TIMEOUT;
        }
    }
   
    return iRlt;
}

static int OLT_SYNC_Send(OLT_SYNC_ROLE emSyncRole, OLT_SYNC_NODE *sync_node, int sync_cmd, void *sync_data, unsigned long sync_datalen)
{
    int iRlt;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(sync_node);

    sync_node->sync_cmd = sync_cmd;
    if ( NULL != sync_data )
    {
        if (NULL != sync_node->sync_data)
        {
            VOS_Free(sync_node->sync_data);
        }
        
        sync_node->sync_data = sync_data;
        sync_node->sync_len  = sync_datalen;
    }
    
    sync_node->Status  &= ~OLT_SYNC_STATUS_DATA_RESEND;
    if ( 0 != (iRlt = OLT_SYNC_Send_Data(emSyncRole, sync_node, sync_cmd, sync_data, sync_datalen)) )
    {
        sync_node->Status |= OLT_SYNC_STATUS_DATA_RESEND;

        sync_node->retry_limit = (sync_node->sync_num % OLT_SYNC_RESEND_TIMESPAN) + 1;
        sync_node->retry_time  = sync_node->retry_limit;
    }

    return iRlt;
}


static int OLT_SYNC_Remove_Object(OLT_SYNC_ROLE emSyncRole, int sync_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    OLT_SYNC_OBJECT_LIST *pSyncList;
    OLT_SYNC_NODE *pPrevNode;
    OLT_SYNC_NODE *pCurrNode;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(sync_id > 0);

    pSyncList = &synobj_list[emSyncRole];
    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pCurrNode = pSyncList->node_list;
    pPrevNode = NULL;
    while ( pCurrNode != NULL )
    {
        if ( sync_id == pCurrNode->src_id )
        {
            if ( 0 == (pCurrNode->Status | OLT_SYNC_STATUS_CONN_CLOSE) )
            {
                (void)OLT_SYNC_Send(emSyncRole, pCurrNode, OLT_SYNC_CMD_DISCONNECT, NULL, 0);
            }

            if ( NULL != pPrevNode )
            {
                pPrevNode->pNext = pCurrNode->pNext;
            }
            else
            {
                pSyncList->node_list = pCurrNode->pNext;
            }

            if ( NULL != pCurrNode->sync_data )
            {
                VOS_Free(pCurrNode->sync_data);
            }
            VOS_Free(pCurrNode);

            iRlt = 0;
            break;
        }

        pPrevNode = pCurrNode;
        pCurrNode = pCurrNode->pNext;
    }
    VOS_SemGive(pSyncList->node_sem);

    return iRlt;
}

static int OLT_SYNC_Notify_Object(OLT_SYNC_ROLE emSyncRole, int sync_id, unsigned int sync_event)
{
    int iRlt = OLT_ERR_NOTEXIST;
    OLT_SYNC_OBJECT_LIST *pSyncList;
    OLT_SYNC_NODE *pCurrNode;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(sync_id > 0);

    pSyncList = &synobj_list[emSyncRole];
    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pCurrNode = pSyncList->node_list;
    while ( pCurrNode != NULL )
    {
        if ( sync_id == pCurrNode->src_id )
        {
            pCurrNode->Events |= sync_event;    

            iRlt = 0;
            break;
        }

        pCurrNode = pCurrNode->pNext;
    }
    VOS_SemGive(pSyncList->node_sem);

    return iRlt;
}

static void OLT_SYNC_Update(OLT_SYNC_ROLE emSyncRole, OLT_SYNC_NODE *sync_node)
{
    int iRlt;
    VOID *pData;
    ULONG ulDataLen;

    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(sync_node);

    if ( OLT_SYNC_ROLE_OFFEROR == emSyncRole )
    {
        /* 采集刷新 */
        if ( NULL != sync_node->Object.pfOffer )
        {
            sync_node->sync_num++;
            if ( 0 == (*sync_node->Object.pfOffer)(&pData, &ulDataLen) )
            {
                if (NULL != sync_node->sync_data)
                {
                    if ((NULL != pData)
                        && (0 != ulDataLen))
                    {
                        if ((sync_node->sync_len == ulDataLen)
                            && (0 == VOS_MemCpy(sync_node->sync_data, pData, ulDataLen)))
                        {
                            /* 数据无变化，无需同步 */
                            VOS_Free(pData);
                            pData = NULL;
                        }
                    }
                }
            
                if (NULL != pData)
                {
                    /* 定时同步 */
                    sync_node->update_num++;
                    (void)OLT_SYNC_Send(emSyncRole, sync_node, OLT_SYNC_CMD_UPDATE, pData, ulDataLen);
                }
            }
            else
            {
                sync_node->loose_num++;
            }
        }
    }
    else
    {
        /* 刷新 收集 */
        if ( NULL != sync_node->Object.pfCollect )
        {
            /* 通知对端的Offer采集数据 */
            (void)OLT_SYNC_Send(emSyncRole, sync_node, OLT_SYNC_CMD_UPDATE, NULL, 0);
        }
    }

    return;
}

static int OLT_SYNC_Timer_Handle(OLT_SYNC_ROLE emSyncRole, unsigned long ulTimeSecLen)
{
    int iUpdateNum;
    OLT_SYNC_OBJECT_LIST *pSyncList;
    OLT_SYNC_NODE *pSynNode;
    
    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));
    VOS_ASSERT(ulTimeSecLen > 0);

    iUpdateNum = 0;
    pSyncList = &synobj_list[emSyncRole];
    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pSynNode = pSyncList->node_list;
    while ( pSynNode != NULL )
    {
        do{
            if ( 0 != pSynNode->Events )
            {
                /* 此同步对象的事件处理 */
                if ( OLT_SYNC_EVENT_UPDATE_FORBID | pSynNode->Events )
                {
                    break;
                }
                
                if ( OLT_SYNC_EVENT_UPDATE_CLEAR | pSynNode->Events )
                {
                    pSynNode->Events &= ~OLT_SYNC_EVENT_UPDATE_CLEAR;
                    (void)OLT_SYNC_Send(emSyncRole, pSynNode, OLT_SYNC_CMD_CLEARDATA, NULL, 0);
                    break;
                }

                if ( OLT_SYNC_EVENT_UPDATE_ATONCE | pSynNode->Events )
                {
                    pSynNode->Events &= ~OLT_SYNC_EVENT_UPDATE_ATONCE;

                    /* 即时更新数据 */
                    OLT_SYNC_Update(emSyncRole, pSynNode);
                    iUpdateNum++;
                }
            }
            else if ( 0 != pSynNode->Status )
            {
                /* 同步包的重发处理 */
                if (OLT_SYNC_STATUS_DATA_RESEND | pSynNode->Status)
                {
                    if ( (pSynNode->retry_time -= ulTimeSecLen) <= 0 ) 
                    {
                        if ( 0 == OLT_SYNC_Send_Data(emSyncRole, &pSynNode->Object, pSynNode->sync_cmd, pSynNode->sync_data , pSynNode->sync_len) )
                        {
                            pSynNode->Status &= ~OLT_SYNC_STATUS_DATA_RESEND;
                        }
                        else
                        {
                            pSynNode->retry_limit *= 2;
                            pSynNode->retry_time = pSynNode->retry_limit;

                            if (pSynNode->retry_limit > OLT_SYNC_RESEND_TIMESPAN_MAX)
                            {
                                pSynNode->Status &= ~OLT_SYNC_STATUS_DATA_RESEND;
                                pSynNode->loose_num++;
                            }
                        }
                    }
                }     

                /* 同步对象的连接处理 */
                if ( OLT_SYNC_STATUS_CONN_CLOSE | pSynNode->Status )
                {

                }
            }
            else
            {
                /* 此同步对象的定时处理 */
                if (( 0 < pSynNode->TimerLen )
                    && (pSynNode->TimerLen <= (pSynNode->TimerCurr += ulTimeSecLen)))
                {
                    pSynNode->TimerCurr = 0;

                    /* 定时更新数据 */
                    OLT_SYNC_Update(emSyncRole, pSynNode);
                    iUpdateNum++;
                }
            }
        }while(0);

        pSynNode = pSynNode->pNext;
    }
    VOS_SemGive(pSyncList->node_sem);

    return iUpdateNum;
}

static int OLT_SYNC_Recv_Data(OLT_SYNC_ROLE emSyncRole, void *pData, unsigned long ulDataLen)
{
    int iRlt = OLT_ERR_NOTEXIST;
    OLT_SYNC_MSG_HEAD *pMsgHead;
    OLT_SYNC_OBJECT_LIST *pSyncList;
    OLT_SYNC_NODE *pCurrNode;

    VOS_ASSERT(pData);
    VOS_ASSERT(ulDataLen >= sizeof(OLT_SYNC_MSG_HEAD));
    VOS_ASSERT((0 <= emSyncRole) && (emSyncRole < OLT_SYNC_ROLE_MAX));

    pMsgHead  = (OLT_SYNC_MSG_HEAD *)pData;
    pSyncList = &synobj_list[emSyncRole];
    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pCurrNode = pSyncList->node_list;
    while ( pCurrNode != NULL )
    {
        if ( pMsgHead->dst_id == pCurrNode->src_id )
        {           
            switch (pMsgHead->sync_cmd)
            {
                case OLT_SYNC_CMD_UPDATE:
                    if (OLT_SYNC_ROLE_OFFEROR == emSyncRole)
                    {
                        pCurrNode->Events |= OLT_SYNC_EVENT_UPDATE_ATONCE;
                    }
                    else
                    {
                        pCurrNode->update_num++;
                        if ( 0 < (ulDataLen -= sizeof(OLT_SYNC_MSG_HEAD)) )
                        {
                            if ( NULL != pCurrNode->Object.pfCollect )
                            {
                                if ( 0 == (*pCurrNode->Object.pfCollect)(pMsgHead->sync_data, ulDataLen) )
                                {
                                    pCurrNode->ok_num++;
                                }
                                else
                                {
                                    pCurrNode->loose_num++;
                                }
                            }
                        }
                        else
                        {
                            pCurrNode->recv_fail++;
                            VOS_ASSERT(0);
                        }
                    }
                    break;
                case OLT_SYNC_CMD_CLEARDATA:

                    break;
                case OLT_SYNC_CMD_CONNECT:
                    if (OLT_SYNC_ROLE_OFFEROR == emSyncRole)
                    {
                        pCurrNode->Events |= OLT_SYNC_EVENT_CONNECT;
                    }
                    pCurrNode->dst_id = pMsgHead->src_id;
                    pCurrNode->Status &= ~OLT_SYNC_STATUS_CONN_CLOSE;
                    break;
                case OLT_SYNC_CMD_DISCONNECT:
                    pCurrNode->dst_id = 0;
                    pCurrNode->Status |= OLT_SYNC_STATUS_CONN_CLOSE;
                    break;
                default:
                    pCurrNode->recv_fail++;
                    VOS_ASSERT(0);
            }

            iRlt = 0;
            break;
        }

        pCurrNode = pCurrNode->pNext;
    }
    VOS_SemGive(pSyncList->node_sem);

    return iRlt;
}
#endif


#if 1
static int OLT_SYNC_Offeror_Recv_Data(void *pData, unsigned long ulDataLen)
{
    return OLT_SYNC_Recv_Data(OLT_SYNC_ROLE_OFFEROR, pData, ulDataLen);
}

static int OLT_SYNC_Collector_Recv_Data(void *pData, unsigned long ulDataLen)
{
    return OLT_SYNC_Recv_Data(OLT_SYNC_ROLE_COLLECTOR, pData, ulDataLen);
}

static int OLT_SYNC_Offeror_Timer_Handle(unsigned long ulTimeSecLen)
{
    return OLT_SYNC_Timer_Handle(OLT_SYNC_ROLE_OFFEROR, ulTimeSecLen);
}

static int OLT_SYNC_Collector_Timer_Handle(unsigned long ulTimeSecLen)
{
    return OLT_SYNC_Timer_Handle(OLT_SYNC_ROLE_COLLECTOR, ulTimeSecLen);
}
#endif


void OLT_SYNC_Offeror_Init()
{
    OLT_SYNC_OBJECT_LIST *pSyncWorker = &synobj_list[OLT_SYNC_ROLE_OFFEROR];

    if (pSyncWorker->iSyncTask == 0)
    {   
        /* 标明OLT同步数据的采集端身份及操作 */
        pSyncWorker->onRecvData = OLT_SYNC_Offeror_Recv_Data;
        pSyncWorker->onTimer    = OLT_SYNC_Offeror_Timer_Handle;

        /* 创建OLT同步数据的收集任务 */
        OLT_SYNC_Task_Create(OLT_SYNC_ROLE_OFFEROR);
    }
}

void OLT_SYNC_Collector_Init()
{
    OLT_SYNC_OBJECT_LIST *pSyncWorker = &synobj_list[OLT_SYNC_ROLE_COLLECTOR];

    if (pSyncWorker->iSyncTask == 0)
    {   
        /* 标明OLT同步数据的收集端身份及操作 */
        pSyncWorker->onRecvData = OLT_SYNC_Collector_Recv_Data;
        pSyncWorker->onTimer    = OLT_SYNC_Collector_Timer_Handle;
        
        /* 创建OLT同步数据的收集任务 */
        OLT_SYNC_Task_Create(OLT_SYNC_ROLE_COLLECTOR);
    }
}

int OLT_SYNC_Add_Offeror(OLT_SYNC_OBJECT *pstOfferor, UINT16 nCircleTimeSec)
{
    OLT_SYNC_NODE *pNewNode;
    OLT_SYNC_OBJECT_LIST *pSyncList;

    VOS_ASSERT(pstOfferor);
    VOS_ASSERT(nCircleTimeSec > 0);

    if (NULL == pstOfferor->pfOffer)
    {
        return OLT_ERR_PARAM;
    }

    pSyncList = &synobj_list[OLT_SYNC_ROLE_OFFEROR];
    if (pSyncList->iSyncTask == 0)
    {
        OLT_SYNC_Offeror_Init();
    }

    if ( NULL == (pNewNode = VOS_Malloc(sizeof(OLT_SYNC_NODE), MODULE_OLT)) )
    {
        return OLT_ERR_MALLOC;
    }

    VOS_MemZero(pNewNode, sizeof(OLT_SYNC_NODE));
    VOS_MemCpy(&pNewNode->Object, pstOfferor, sizeof(OLT_SYNC_OBJECT));
    pNewNode->Events   = OLT_SYNC_EVENT_UPDATE_ATONCE;
    pNewNode->Status   = OLT_SYNC_STATUS_CONN_CLOSE;
    pNewNode->TimerLen = nCircleTimeSec;

    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pNewNode->src_id     = (OLT_SYNC_ROLE_OFFEROR << 16) | (USHORT)(++pSyncList->node_id);
    pNewNode->pNext      = pSyncList->node_list;
    pSyncList->node_list = pNewNode;
    VOS_SemGive(pSyncList->node_sem);

    return pNewNode->src_id;
}

int OLT_SYNC_Add_Collector(OLT_SYNC_OBJECT *pstCollector, UINT16 nUpdateTimeSec)
{
    OLT_SYNC_NODE *pNewNode;
    OLT_SYNC_OBJECT_LIST *pSyncList;

    VOS_ASSERT(pstCollector);
    VOS_ASSERT(nUpdateTimeSec >= 0);
    
    if (NULL == pstCollector->pfCollect)
    {
        return OLT_ERR_PARAM;
    }

    pSyncList = &synobj_list[OLT_SYNC_ROLE_COLLECTOR];
    if (pSyncList->iSyncTask == 0)
    {
        OLT_SYNC_Collector_Init();
    }

    if ( NULL == (pNewNode = VOS_Malloc(sizeof(OLT_SYNC_NODE), MODULE_OLT)) )
    {
        return OLT_ERR_MALLOC;
    }

    VOS_MemZero(pNewNode, sizeof(OLT_SYNC_NODE));
    VOS_MemCpy(&pNewNode->Object, pstCollector, sizeof(OLT_SYNC_OBJECT));
    pNewNode->Events   = OLT_SYNC_EVENT_UPDATE_ATONCE;
    pNewNode->Status   = OLT_SYNC_STATUS_CONN_CLOSE;
    pNewNode->TimerLen = nUpdateTimeSec;

    VOS_SemTake(pSyncList->node_sem, WAIT_FOREVER);
    pNewNode->src_id     = (OLT_SYNC_ROLE_COLLECTOR << 16) | (USHORT)(++pSyncList->node_id);
    pNewNode->pNext      = pSyncList->node_list;
    pSyncList->node_list = pNewNode;
    VOS_SemGive(pSyncList->node_sem);

    return pNewNode->src_id;
}


int OLT_SYNC_Notify_Offeror(int sync_id, unsigned int sync_event)
{
    return OLT_SYNC_Notify_Object(OLT_SYNC_ROLE_OFFEROR, sync_id, sync_event);
}

int OLT_SYNC_Notify_Collector(int sync_id, unsigned int sync_event)
{
    return OLT_SYNC_Notify_Object(OLT_SYNC_ROLE_COLLECTOR, sync_id, sync_event);
}


int OLT_SYNC_Remove_Offeror(int sync_id)
{
    return OLT_SYNC_Remove_Object(OLT_SYNC_ROLE_OFFEROR, sync_id);
}

int OLT_SYNC_Remove_Collector(int sync_id)
{
    return OLT_SYNC_Remove_Object(OLT_SYNC_ROLE_COLLECTOR, sync_id);
}


#else
extern int  OLT_RPC_AsyncCall( CDP_SYNC_Header_S *sync_head );

int OLT_SYNC_Data(short int olt_id, int sync_cmd, void *sync_data, unsigned long sync_datalen)
{
    int iRlt = OLT_ERR_OK;
    OLT_SYNC_MSG_HEAD *pMsgHead;
    ULONG ulMsgLen;
    ULONG ulDstSlot;
    int slot, port;

    OLT_ASSERT(olt_id);

    /* 得到OLT所在的PON板槽位 */
    if ( RERROR == (slot = GetCardIdxByPonChip(olt_id)) )
    {
        return OLT_ERR_PARAM;
    }

    /* 得到OLT所在的PON板端口号 */
    if ( RERROR == (port = GetPonPortByPonChip(olt_id)) )
    {
        return OLT_ERR_PARAM;
    }
        
    if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
    {
        /* 主控板发同步消息到PON板 */
        if ( SYS_MODULE_IS_REMOTE(slot) )
        {
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
            ulDstSlot = (ULONG)LSLOT_TO_CCSLOT(slot); 

            if ( OLT_ID_INVALID == (olt_id = OLTRM_GetRemoteLogicalOltID(olt_id)) )
            {
                return OLT_ERR_NOTEXIST;
            }
            
            /* 定位为远端的逻辑端口 */    
            slot = OLT_SLOT_ID(olt_id);
            port = OLT_PORT_ID(olt_id);
#else
            return OLT_ERR_NOTSUPPORT;
#endif
        }
        else
        {
            ulDstSlot = slot;
        }
    }
    else
    {
        /* PON板发同步消息到主控板 */
        ulDstSlot = SYS_MASTER_ACTIVE_SLOTNO;
    }
    
    ulMsgLen = sizeof(OLT_SYNC_MSG_HEAD) + sync_datalen;
    pMsgHead = ( OLT_SYNC_MSG_HEAD * ) CDP_AllocMsg( ulMsgLen, MODULE_OLT );              
    if(NULL == pMsgHead)
    {
        return OLT_ERR_MALLOC;
    }

    pMsgHead->slot = slot;
    pMsgHead->port = port;
    pMsgHead->src_id = SYS_LOCAL_MODULE_SLOTNO;
    pMsgHead->dst_id = slot;
    pMsgHead->obj_id = 0;
    pMsgHead->sync_cmd = sync_cmd;

    if ( sync_datalen > 0 )
        VOS_MemCpy(pMsgHead->sync_data, sync_data, sync_datalen);

    /* 板间同步，走CDP板间信道 */
    /* CDP的同步发送，可以保证同步不占满板间信道的带宽 */
    iRlt = CDP_Send(RPU_TID_CDP_OLT, ulDstSlot, RPU_TID_CDP_OLT, CDP_MSG_TM_SYNC, pMsgHead, ulMsgLen, MODULE_OLT);
    OLT_SYNC_DEBUG(OLT_SYNC_SND_TITLE"Slot(%d) send sync-msg(%d) to Slot(%d) result(%d).", SYS_LOCAL_MODULE_SLOTNO, sync_cmd, ulDstSlot, iRlt );

    /* CDP的同步发送，必须手动释放消息块 */
    CDP_FreeMsg( pMsgHead );
   
    return iRlt;
}

static int OLT_SYNC_IPC_CB(VOID *pData, ULONG ulLen)
{
    int iRlt = 0;
    short int olt_id, obj_id;
    OLT_SYNC_MSG_HEAD *pMsgHead = pData;

    VOS_ASSERT(pMsgHead);

    olt_id = GetPonPortIdxBySlot(pMsgHead->slot, pMsgHead->port);
    obj_id = pMsgHead->obj_id;

    if ( 0 == pMsgHead->sync.ulDstMod )
    {
        VOS_ASSERT(ulLen >= sizeof(OLT_SYNC_MSG_HEAD));
        OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) receive sync-msg(%d) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->sync_cmd, pMsgHead->src_id );
        switch (pMsgHead->sync_cmd)
        {
            case FC_ADD_PONPORT:
                OLT_LOCAL_ASSERT(olt_id);
                if ( PONCHIP_LOADCOMP != PonChipMgmtTable[olt_id].operStatus )
                {
                    PonChipMgmtTable[olt_id].operStatus = PONCHIP_LOADCOMP;
                    OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_ADD_PONPORT) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->slot );
                }

                break;
            case FC_DEL_PONPORT:
                OLT_LOCAL_ASSERT(olt_id);
                if ( PONCHIP_ERR != PonChipMgmtTable[olt_id].operStatus )
                {
                    PonChipMgmtTable[olt_id].operStatus = PONCHIP_ERR;
                    PonPortTable[olt_id].PortWorkingStatus = PONPORT_DOWN;
                    OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_DEL_PONPORT) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->slot );
                }

                break;
            case FC_PON_RESET:
                OLT_LOCAL_ASSERT(olt_id);
                if ( PONCHIP_ERR != PonChipMgmtTable[olt_id].operStatus )
                {
                    PonChipMgmtTable[olt_id].operStatus = PONCHIP_ERR;
                    PonPortTable[olt_id].PortWorkingStatus = PONPORT_DOWN;
                    OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_PON_RESET) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->slot );

                    /* B--added by liwei056@2011-11-22 for D13853 */
                    OLTAdv_NotifyOltInvalid(olt_id);
                    /* E--added by liwei056@2011-11-22 for D13853 */
                }

                break;
            case FC_PONPORT_ACTIVATED:
                OLT_LOCAL_ASSERT(olt_id);
                if ( PONPORT_DOWN == PonPortTable[olt_id].PortWorkingStatus )
                {
                    PonPortTable[olt_id].PortWorkingStatus = PONPORT_UPDATE;
                    PonChipMgmtTable[olt_id].operStatus = PONCHIP_LOADCOMP;
        			PonPortActivated(olt_id);
                    OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_PONPORT_ACTIVATED) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->slot );
                }

                break;
            case FC_PONSWITCH:
                {
                    PonSwitchInfo_S *PonSwitchData = (PonSwitchInfo_S*)pMsgHead->sync_data;

                    if ( sizeof(PonSwitchInfo_S) == ulLen - sizeof(OLT_SYNC_MSG_HEAD) )
                    {
                        sendPonSwitchEventMsg(olt_id, PonSwitchData->event_id, PonSwitchData->event_code, PonSwitchData->event_source, PonSwitchData->slot_source, PonSwitchData->new_status, PonSwitchData->event_seq, PonSwitchData->event_flags);
                        OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_PONSWITCH) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->src_id );
                    }
                }
                break;
            case FC_ONUSWITCH:
			   /*for onu swap by jinhl@2013-02-22*/
			   
			   OLT_LOCAL_ASSERT(olt_id);
			   OLT_SYNC_DEBUG("\r\nsync FC_ONUSWITCH,olt_id:%d,Max_Pon:%d\r\n",olt_id,MAXPON);
               #if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
               {
                    OnuSwitchInfo_S *OnuSwitchData = (OnuSwitchInfo_S*)pMsgHead->sync_data;

                    if ( sizeof(OnuSwitchInfo_S) == ulLen - sizeof(OLT_SYNC_MSG_HEAD) )
                    {
                        sendOnuSwitchEventMsg(olt_id, OnuSwitchData->event_id, OnuSwitchData->fail_info, OnuSwitchData->event_source, OnuSwitchData->event_seq, OnuSwitchData->event_flags);
                        OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_OnuSWITCH) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->slot );
                    }
                }
			   #endif
                break;
            case FC_TEST_PONPORT:
                iRlt = 0;
                OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_TEST_PONPORT) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->src_id );
                break;
			case FC_PONDEV_RESET:
				OLT_LOCAL_ASSERT(olt_id);
				iRlt = 0;
                OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) handle sync-msg(FC_PONDEV_RESET) from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, pMsgHead->src_id );
				PonDevReset(olt_id);
				break;
				
            default:
                iRlt = OLT_ERR_PARAM;
        }

        CDP_FreeMsg (pData);
    }
    else
    {
        CDP_SYNC_Header_S *sync_head = &pMsgHead->sync;
    
        VOS_ASSERT(ulLen > sizeof(CDP_SYNC_Header_S));
        OLT_SYNC_DEBUG(OLT_SYNC_RCV_TITLE"Slot(%d) receive async_call-msg from Slot(%d).", SYS_LOCAL_MODULE_SLOTNO, sync_head->ulSrcSlot );
        iRlt = OLT_RPC_AsyncCall( sync_head );
    }

    return iRlt;
}

static VOID OLT_IPC_CB (ULONG ulFlag, ULONG ulChID, ULONG ulNode,
       ULONG ulChannelID, VOID *pData, ULONG ulLen)
{
    switch( ulFlag )
    {
        case CDP_NOTI_FLG_RXDATA: /* Receive data*/
             (void)OLT_SYNC_IPC_CB( pData, ulLen );             
             break;
        case CDP_NOTI_FLG_SEND_FINISH:/*Async send data*/
             CDP_FreeMsg (pData);
             break;
        default: break;
    }
    
    return;
}

void OLT_SYNC_Init()
{
    /* 创建OLT同步数据的CDP通道 */
    (void)CDP_Create(RPU_TID_CDP_OLT, CDP_NOTI_VIA_FUNC, 0, OLT_IPC_CB);
}

#endif /* OLT_SYNC_SUPPORT */


#ifdef __cplusplus
}
#endif


