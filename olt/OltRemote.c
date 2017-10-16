/***************************************************************
*
*						Module Name:  OltRemote.c
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
*   Date: 			2011/06/08
*   Author:		liwei056
*   content:
**  History:
**   Date        |    Name       |     Description
**---- ----- |-----------|------------------ 
**  11/06/08  |   liwei056    |     create 
**----------|-----------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"

#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
#include  "ifm/ifm_def.h"
#include  "sys/devsm/devsm_remote.h"


#if 1
/* ----------------远程OLT设备信息查询应用的具体产品实现----------------- */
int devrm_apply_deviceinfo_get(int logical_slot, int logical_port, remote_port_cfg *port_cfgs, void *apply_info, int info_size)
{
    int iRlt = 0;
    remote_apply_devinfo *pLocalDevInfo = (remote_apply_devinfo*)apply_info;

    VOS_ASSERT(apply_info);

    VOS_MemZero(pLocalDevInfo, info_size);
    if ( info_size >= sizeof(remote_apply_devinfo) )
    {
        int iLen;

        pLocalDevInfo->usDevInfoVer = REMOTE_APP_DEVINFO_VERSION;
        pLocalDevInfo->usDevProductType = (USHORT)SYS_PRODUCT_TYPE;
        
        pLocalDevInfo->ucSlotMin = OLTCARD_FIRST;
        pLocalDevInfo->ucSlotMax = OLTCARD_LAST;
        
        pLocalDevInfo->ucSwSlotMin = SWCARD_FIRST;
        pLocalDevInfo->ucSwSlotMax = SWCARD_LAST;

        pLocalDevInfo->ucUplinkSlotMin = UPCARD_FIRST;
        pLocalDevInfo->ucUplinkSlotMax = UPCARD_LAST;
        pLocalDevInfo->ucUplinkSlotPortNum = ETHPORTPERCARD;

        pLocalDevInfo->ucPonSlotMin = PONCARD_FIRST;
        pLocalDevInfo->ucPonSlotMax = PONCARD_LAST;
        pLocalDevInfo->ucPonSlotPortNum = PONPORTPERCARD;

        GetOltDeviceName(pLocalDevInfo->szDeviceName, &iLen);
        pLocalDevInfo->szDeviceName[iLen] = '\0';

        GetOltSWVersion(pLocalDevInfo->szDeviceVersion, &iLen);
        pLocalDevInfo->szDeviceVersion[iLen] = '\0';
    }
    else
    {
        iRlt = REMOTE_ERR_SIZE;
    }
    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"devrm_apply_deviceinfo_get(%d, %d, %d) LocalDevInfo[ver:%d; dev:%d; slot(%d-%d), pon(%d-%d/%d), uplink(%d-%d/%d)]", logical_slot, logical_port, info_size, pLocalDevInfo->usDevInfoVer, pLocalDevInfo->usDevProductType, pLocalDevInfo->ucSlotMin, pLocalDevInfo->ucSlotMax, pLocalDevInfo->ucPonSlotMin, pLocalDevInfo->ucPonSlotMax, pLocalDevInfo->ucPonSlotPortNum, pLocalDevInfo->ucUplinkSlotMin, pLocalDevInfo->ucUplinkSlotMax, pLocalDevInfo->ucUplinkSlotPortNum);

    return iRlt;
}
#endif


#if 1
/* ----------------远程端口信息查询应用的产品实现----------------- */
int devrm_apply_portinfo_get(int logical_slot, int logical_port, remote_port_cfg *port_cfgs, void *apply_info, int info_size)
{
    int iRlt = 0;
    int local_slot;
    int local_port;
    ULONG ulIfIndex;
    ULONG ulIfState;
    remote_apply_portinfo *pLocalPortInfo = (remote_apply_portinfo*)apply_info;

    VOS_ASSERT(port_cfgs);
    VOS_ASSERT(apply_info);

    VOS_MemZero(pLocalPortInfo, info_size);
    if ( info_size >= sizeof(remote_apply_portinfo) )
    {
        local_slot = port_cfgs->ucLocalSlotID;
        local_port = port_cfgs->ucLocalPortID;
        
        if ( SYS_SLOT_IS_VALID_LOCAL(local_slot)
            && (OLTPORTCARD_FIRST <= local_slot)
            && (OLTPORTCARD_LAST >= local_slot) )
        {
            /*char szPortIfName[32];*/
            
            /*VOS_Sprintf(szPortIfName, "%d/%d", local_slot, local_port);*/

			 ulIfIndex = IFM_ETH_CREATE_INDEX(local_slot, local_port);
            
            if ( (local_port >= 1)
                && (local_port <= OLTPORTPERCARD)
                && (VOS_YES == IFM_IndexIsExist(ulIfIndex))/*(IFM_INVALID_IFINDEX != (ulIfIndex = IFM_GetIfindexByNameSlotPort(szPortIfName, &ulIfState))) */)
            {
                pLocalPortInfo->usPortInfoVer = REMOTE_APP_PORTINFO_VERSION;

                if ( 0 == SlotCardMayBePonBoardByVty(local_slot, NULL) )
                {
                    short int PonPortIdx;
                    short int PonPortIdx2;
                    
                    PonPortIdx = GetPonPortIdxBySlot((short int)local_slot, (short int)local_port);
                    if ( (local_port <= PONPORTPERCARD)
                        && OLT_LOCAL_ISVALID(PonPortIdx) )
                    {                       
                        pLocalPortInfo->usPortStatus = DEVRM_PORTSTATUS_NOTEXIST | DEVRM_PORTSTATUS_ENABLE;
                        pLocalPortInfo->sPortType = DEVRM_PORTTYPE_UNKNOWN;

                        if ( ROK == SlotCardIsPonBoard(local_slot) )
                        {
                            if ( getPonChipInserted(local_slot, local_port) )
                            {
                                pLocalPortInfo->sPortType = DEVRM_PORTTYPE_PON;
                                pLocalPortInfo->usPortStatus = DEVRM_PORTSTATUS_EXIST | DEVRM_PORTSTATUS_ENABLE;

                                if ( PonPortIsWorking(PonPortIdx) )
                                {
                                    pLocalPortInfo->usPortStatus |= DEVRM_PORTSTATUS_WORKING;
                                }
                                else
                                {
                                    pLocalPortInfo->usPortStatus |= DEVRM_PORTSTATUS_PENDING;
                                }
                            }
                            else
                            {
                                if ( 0 < GetSlotPonPortPhyRange(local_slot, &PonPortIdx, &PonPortIdx2) )
                                {
                                    if ( (local_port >= PonPortIdx) && (local_port <= PonPortIdx2) )
                                    {
                                        pLocalPortInfo->sPortType = DEVRM_PORTTYPE_PON;
                                    }
                                    else
                                    {
                                        pLocalPortInfo->sPortType = DEVRM_PORTTYPE_UPLINK;
                                    }
                                }
                                else
                                {
                                    pLocalPortInfo->sPortType = DEVRM_PORTTYPE_PON;
                                }
                            }
                        }
                        else
                        {
                            if ( ROK != SlotCardMayBeUplinkBoardByVty(local_slot, NULL) )
                            {
                                pLocalPortInfo->sPortType = DEVRM_PORTTYPE_PON;
                            }
                        }
                    }
                    else
                    {
                        iRlt = REMOTE_ERR_PORT;
                    }
                }
                else if ( ROK == SlotCardIsUplinkBoard(local_slot) )
                {
                    if ( local_port <= ETHPORTPERCARD)
                    {
                        pLocalPortInfo->sPortType = DEVRM_PORTTYPE_UPLINK;
                        pLocalPortInfo->sPortSubType = IFM_IFINDEX_GET_TYPE(ulIfIndex);
                        pLocalPortInfo->usPortStatus = DEVRM_PORTSTATUS_EXIST | DEVRM_PORTSTATUS_ENABLE | DEVRM_PORTSTATUS_WORKING;
                    }
                    else
                    {
                        iRlt = REMOTE_ERR_PORT;
                    }
                }
                else
                {
                    iRlt = REMOTE_ERR_SLOT;
                }
            }
            else
            {
                iRlt = REMOTE_ERR_PORT;
            }
        }
        else
        {
            iRlt = REMOTE_ERR_SLOT;
        }
    }
    else
    {
        iRlt = REMOTE_ERR_SIZE;
    }
    
    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"devrm_apply_portinfo_get(%d, %d, %d)'s result(%d) LocalPortInfo[ver:%d; porttype:%d-%d; portstatus:%d]", logical_slot, logical_port, info_size, iRlt, pLocalPortInfo->usPortInfoVer, pLocalPortInfo->sPortType, pLocalPortInfo->sPortSubType, pLocalPortInfo->usPortStatus);

    return iRlt;
}
#endif


#if 1
/* ----------------远程PON端口管理应用的实现----------------- */
#define REMOTE_APP_PONMANAGE_VERSION  1
#define REMOTE_PON_INFO_MIN  8

/*  远端设备端口PON应用的结构体定义[为保证版本兼容，此结构只能扩展，不能修改] */
typedef struct _remote_apply_poninfo
{
    SHORT sOltLocalVer;
    SHORT sOltRemoteVer;
    
    SHORT sLocalOlt;
    SHORT sRemoteOlt;
}remote_apply_poninfo;

int g_iPonRemoteApplyID = -1;

int olt_rapply_getinfosize(int slot, int port)
{
    int info_size;

    info_size = sizeof(remote_apply_poninfo);
    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_getinfosize(%d, %d, %d)", slot, port, info_size);

    return info_size;
}

int olt_rapply_initinfo(int slot, int port, void *apply_info, int info_size)
{
    int iRlt = OLT_ERR_PARAM;
    remote_apply_poninfo *pPonLocalInfo; 

    if ( NULL != (pPonLocalInfo = apply_info) )
    {
        pPonLocalInfo->sOltLocalVer  = REMOTE_APP_PONMANAGE_VERSION;
        pPonLocalInfo->sOltRemoteVer = 0;
        
        pPonLocalInfo->sLocalOlt  = OLT_ID_INVALID;
        pPonLocalInfo->sRemoteOlt = OLT_ID_INVALID;
        
        iRlt = 0;
    }

    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_initinfo(%d, %d, %d)'s result(%d).", slot, port, info_size, iRlt);

    return iRlt;
}

/* 设备倒换的协商读接口  */
int olt_rapply_getlocalinfo(int logical_slot, int logical_port, remote_port_cfg *port_cfgs, void *apply_info, int info_size)
{
    int local_info_size;
    int remote_info_size;
    remote_apply_poninfo ponInfo;

    VOS_ASSERT(port_cfgs);
    VOS_ASSERT(apply_info);

    local_info_size  = sizeof(ponInfo);
    remote_info_size = info_size;
    
    VOS_MemZero(&ponInfo, local_info_size);
    VOS_MemZero(apply_info, remote_info_size);

    /* 1. 应用版本协商 */
    ponInfo.sOltLocalVer = REMOTE_APP_PONMANAGE_VERSION;
    
    /* 2. 应用消息块尺寸协商 */
    if ( remote_info_size >= local_info_size )
    {
        int local_slot;
        int local_port;
        int remote_slot;
        int remote_port;
        
        /* 3. 端口协商 */
        local_slot = port_cfgs->ucLocalSlotID;
        local_port = port_cfgs->ucLocalPortID;
        if ( (0 == local_slot)
            || (0 == local_port) )
        {
            short int sLogicalOlt, sLocalOlt;

            sLogicalOlt = OLT_NET_ID(logical_slot, logical_port);
            if ( ROK == PonPortSwapPortQuery(sLogicalOlt, &sLocalOlt) )
            {
                local_slot = GetCardIdxByPonChip(sLocalOlt);
                local_port = GetPonPortByPonChip(sLocalOlt);
                
                devsm_remote_port_setlocalport(logical_slot, logical_port, local_slot, local_port, REMOTE_FLAG_NONE);
            }
        }

        if ( OLT_SLOT_ISVALID(local_slot) && OLT_PORT_ISVALID(local_port) )
        {
            ponInfo.sLocalOlt = OLT_NET_ID(local_slot, local_port);
        }
        else
        {
            ponInfo.sLocalOlt = OLT_ID_INVALID;
        }

        remote_slot = port_cfgs->ucRemoteSlotID;
        remote_port = port_cfgs->ucRemotePortID;
        if ( (0 == remote_slot)
            || (0 == remote_port) )
        {
            ponInfo.sRemoteOlt = OLT_ID_INVALID;
        }
        else
        {
            ponInfo.sRemoteOlt = OLT_NET_ID(remote_slot, remote_port);
        }
    }
    else
    {
        ponInfo.sOltRemoteVer = REMOTE_ERR_SIZE;
    
        ponInfo.sLocalOlt  = local_info_size;
        ponInfo.sRemoteOlt = remote_info_size;

        local_info_size = remote_info_size;
    }

    VOS_MemCpy(apply_info, &ponInfo, local_info_size);

    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_getlocalinfo(%d, %d, %d) LocalPonInfo[lver:%d, rver:%d; lolt:%d, rolt:%d]", logical_slot, logical_port, local_info_size, ponInfo.sOltLocalVer, ponInfo.sOltRemoteVer, ponInfo.sLocalOlt, ponInfo.sRemoteOlt);

    return 0;
}

/* 设备倒换的协商写接口  */
int olt_rapply_setremoteinfo(int logical_slot, int logical_port, void *apply_info, int info_size)
{
    int iRlt = 0;
    int local_info_size;
    int remote_info_size;
    remote_apply_poninfo *pPonRemoteInfo; 
    remote_apply_poninfo *pPonLocalInfo; 

    DEVRM_ASSERT_REMOTE_SLOT(logical_slot);
    DEVRM_ASSERT_REMOTE_PORT(logical_port);
    VOS_ASSERT(apply_info);

    if ( info_size < REMOTE_PON_INFO_MIN )
    {
        return REMOTE_ERR_SIZE;
    }
    
    pPonRemoteInfo = (remote_apply_poninfo*)apply_info;
    if ( NULL != (pPonLocalInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(logical_slot, logical_port, g_iPonRemoteApplyID)) )
    {
        short int sRlt;
        short int sRemoteOltID;

        local_info_size  = sizeof(remote_apply_poninfo);
        remote_info_size = info_size;

        /* 1. 远端协商结果判断 */
        if ( 0 < (sRlt = pPonRemoteInfo->sOltLocalVer) )
        {
            /* 2. 本地协商 */
            /* 2.1 应用消息块尺寸协商 */
            if ( remote_info_size == local_info_size )
            {
                /* 2.2 应用版本协商 */
                pPonLocalInfo->sOltRemoteVer = sRlt;

                /* 2.3 应用OLT ID协商 */
                if ( 0 <= (sRemoteOltID = pPonRemoteInfo->sLocalOlt) )
                {
                    short int remote_slot, remote_port;
                    
                    remote_slot = OLT_SLOT_ID(sRemoteOltID);
                    remote_port = OLT_PORT_ID(sRemoteOltID);
                    
                    pPonLocalInfo->sRemoteOlt = sRemoteOltID;
                    devsm_remote_port_setremoteport(logical_slot, logical_port, remote_slot, remote_port, REMOTE_FLAG_NONE);
                }
                
                if ( OLT_ID_INVALID != (sRemoteOltID = pPonRemoteInfo->sRemoteOlt) )
                {
                    short int local_slot, local_port;
                    
                    local_slot = OLT_SLOT_ID(sRemoteOltID);
                    local_port = OLT_PORT_ID(sRemoteOltID);
                    if ( OLT_SLOT_ISVALID(local_slot) && OLT_PORT_ISVALID(local_port) )
                    {
                        sRemoteOltID = GetPonPortIdxBySlot(local_slot, local_port);
                        
                        pPonLocalInfo->sLocalOlt = sRemoteOltID;
                        devsm_remote_port_setlocalport(logical_slot, logical_port, local_slot, local_port, REMOTE_FLAG_NONE);
                    }
                }
            }
            else
            {
                iRlt = REMOTE_ERR_SIZE;
            }
        }
        else
        {
            /* 记录远端协商错误提示 */
            pPonLocalInfo->sOltRemoteVer = sRlt;
            pPonLocalInfo->sLocalOlt     =  pPonLocalInfo->sLocalOlt;
            pPonLocalInfo->sRemoteOlt    =  pPonLocalInfo->sRemoteOlt;
        
            iRlt = sRlt;
        }
        
        devsm_remote_port_release_apply(logical_slot, logical_port, g_iPonRemoteApplyID);
    }
    else
    {
        iRlt = REMOTE_ERR_NOTEXIST;
    }

    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_setremoteinfo(%d, %d, %d)'s result(%d) RemotePonInfo[lver:%d, rver:%d; lolt:%d, rolt:%d]", logical_slot, logical_port, info_size, iRlt, pPonRemoteInfo->sOltLocalVer, pPonRemoteInfo->sOltRemoteVer, pPonRemoteInfo->sLocalOlt, pPonRemoteInfo->sRemoteOlt);

    return iRlt;
}

int olt_rapply_active(int logical_slot, int logical_port, void *apply_info)
{
    int iRlt = 0;
    short int sRemoteOlt;
    short int sLocalOlt;
    remote_apply_poninfo *pPonRemoteInfo; 

    if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)apply_info) )
    {
        if (OLT_ID_INVALID != pPonRemoteInfo->sRemoteOlt)
        {
            /* 有远程OLT ID，可以激活单向远程业务 */
            sRemoteOlt = GetPonPortIdxBySlot(logical_slot, logical_port);    

            if ( ROK == PonPortSwapPortQuery( sRemoteOlt, &sLocalOlt) )
            {
                /* 此OLT有双向业务，尝试激活 */
                if ( OLT_ID_INVALID != pPonRemoteInfo->sLocalOlt )
                {
                    iRlt = EnablePonPortAutoProtectByOnePort(sLocalOlt);
                }
                else
                {
                    iRlt = OLT_ERR_PARTOK;
                }
            }
            else
            {
                /* 此OLT只激活单向业务 */
            }
        }
    }    

    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_active(%d, %d)'s result(%d).", logical_slot, logical_port, iRlt);

    return iRlt;
}

int olt_rapply_deactive(int logical_slot, int logical_port, void *apply_info)
{
    int iRlt = 0;
    remote_apply_poninfo *pPonRemoteInfo;

    /* 设备间的倒换，不能取消PON倒换设置 */
#if 1
    short int sRemoteOlt;
    short int sLocalOlt;

    sRemoteOlt = GetPonPortIdxBySlot(logical_slot, logical_port);    
    if ( ROK == PonPortSwapPortQuery( sRemoteOlt, &sLocalOlt) )
    {
        if ( V2R1_PON_PORT_SWAP_QUICKLY == GetPonPortHotSwapMode(sLocalOlt) )
        {
            /* 通知快倒换远端OLT联系丢失，使其适应去激活状态 */
            iRlt = OLT_RdnLooseOlt(sLocalOlt, sRemoteOlt);        
        }
        else
        {
            /* 慢倒换自适应去激活状态 */
        }
    }   
#endif

    if ( NULL != (pPonRemoteInfo = apply_info) )
    {
        pPonRemoteInfo->sOltRemoteVer = 0;
        
        pPonRemoteInfo->sLocalOlt  = OLT_ID_INVALID;
        pPonRemoteInfo->sRemoteOlt = OLT_ID_INVALID;
    }

    OLT_REMOTE_DEBUG(OLT_REMOTE_TITLE"olt_rapply_deactive(%d, %d)'s result(%d).", logical_slot, logical_port, iRlt);

    return iRlt;
}

static devsm_remote_apply_info s_stOltRemoteApply = {
                        DEVRM_APPLYFLAG_ASYNC
                        ,olt_rapply_getinfosize
                        ,olt_rapply_initinfo
                        ,olt_rapply_getlocalinfo
                        ,olt_rapply_setremoteinfo
                        ,olt_rapply_active
                        ,olt_rapply_deactive
                        };


int devrm_apply_ponmanage_init()
{
    if ( 0 == devsm_remote_apply_register(REMOTE_APPID_PONMANAGE, &s_stOltRemoteApply) )
    {
        if ( 0 == devsm_remote_port_bind_apply(LOGICAL_SLOT_ALL, LOGICAL_PORT_ALL, REMOTE_APPID_PONMANAGE) )
        {
            g_iPonRemoteApplyID = REMOTE_APPID_PONMANAGE;
        }
        else
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        VOS_ASSERT(0);
    }

    return 0;
}
#endif


int olt_remotemange_init()
{
    /* 注册一个远程应用:远程管理PON端口 */
    devrm_apply_ponmanage_init();

    return 0;
}

/* PON远程管理应用 */
int OLTRM_GetRemoteOltID(int logical_slot, int logical_port)
{
    int olt_id = OLT_ID_INVALID;
    remote_apply_poninfo *pPonRemoteInfo; 

    VOS_ASSERT( SYS_SLOT_IS_VALID_REMOTE( logical_slot ) );
    VOS_ASSERT( SYS_PORT_IS_VALID_REMOTE( logical_port ) );
    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER );

    if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(logical_slot, logical_port, g_iPonRemoteApplyID)) )
    {
        olt_id = pPonRemoteInfo->sRemoteOlt;
        devsm_remote_port_release_apply(logical_slot, logical_port, g_iPonRemoteApplyID);
    }
    
    return olt_id;
}

int OLTRM_GetRemoteLogicalOltID(short int remote_olt_id)
{
    int olt_id = OLT_ID_INVALID;
    int lslot;
    int lport;

    OLT_NET_ASSERT(remote_olt_id);
    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER );

    lslot = OLT_SLOT_ID(remote_olt_id);
    lport = OLT_PORT_ID(remote_olt_id);
    if ( 0 == devsm_remote_port_getpartnerport(lslot, lport, &lslot, &lport) )
    {
        olt_id = OLT_NET_ID(lslot, lport);
    }
    
    return olt_id;
}

int OLTRM_RemoteOltIsValid(short int remote_olt_id)
{
    int iRlt = FALSE;
    int lslot;
    int lport;
    remote_apply_poninfo *pPonRemoteInfo; 

    OLT_NET_ASSERT(remote_olt_id);
    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER );

    lslot = OLT_SLOT_ID(remote_olt_id);
    lport = OLT_PORT_ID(remote_olt_id);
    if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(lslot, lport, g_iPonRemoteApplyID)) )
    {
        if ( (0 <= pPonRemoteInfo->sRemoteOlt)
            && (0 <= pPonRemoteInfo->sLocalOlt) )
        {
            /* 只有两端OLT对应上，才允许工作[仅倒换有此要求] */
            iRlt = TRUE;
        }
        devsm_remote_port_release_apply(lslot, lport, g_iPonRemoteApplyID);
    }
    
    return iRlt;
}


#if ( EPON_SUBMODULE_PON_DEVICE_SWAP == EPON_MODULE_YES )
/* PON倒换的远程应用 */
int OLTRM_GetRemoteOltPartner(short int remote_olt_id, short int *partner_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iSlot, iPort;
    
    OLT_NET_ASSERT( remote_olt_id );
    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER );

    iSlot = GetCardIdxByPonChip(remote_olt_id);
    if ( SYS_SLOT_IS_VALID_REMOTE(iSlot) )
    {
        iPort = GetPonPortByPonChip(remote_olt_id);
        if ( SYS_PORT_IS_VALID_REMOTE(iPort) )
        {
            remote_apply_poninfo *pPonRemoteInfo; 
            
            if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(iSlot, iPort, g_iPonRemoteApplyID)) )
            {
                if ( OLT_ID_INVALID != pPonRemoteInfo->sLocalOlt )
                {
                    *partner_id = pPonRemoteInfo->sLocalOlt;
                    iRlt = 0;
                }
                
                devsm_remote_port_release_apply(iSlot, iPort, g_iPonRemoteApplyID);
            }
        }
    }
    
    return iRlt;
}
    
int OLTRM_SetRemoteOltPartner(short int remote_olt_id, short int local_partner_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iSlot, iPort;
    
    OLT_NET_ASSERT( remote_olt_id );
    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER );

    iSlot = GetCardIdxByPonChip(remote_olt_id);
    if ( SYS_SLOT_IS_VALID_REMOTE(iSlot) )
    {
        iPort = GetPonPortByPonChip(remote_olt_id);
        if ( SYS_PORT_IS_VALID_REMOTE(iPort) )
        {
            remote_apply_poninfo *pPonRemoteInfo; 
            
            if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(iSlot, iPort, g_iPonRemoteApplyID)) )
            {               
                pPonRemoteInfo->sLocalOlt = local_partner_id;
                devsm_remote_port_release_apply(iSlot, iPort, g_iPonRemoteApplyID);
            }
            
            devsm_remote_port_setlocalport(iSlot, iPort, GetCardIdxByPonChip(local_partner_id), GetPonPortByPonChip(local_partner_id), DEVRM_FLAGS_USRCFG);
            iRlt = 0;
       }
    }
    
    return iRlt;
}

int OLTRM_ClrRemoteOltPartner(short int remote_olt_id)
{
    int iRlt = OLT_ERR_NOTEXIST;
    int iSlot, iPort;
    
    iSlot = GetCardIdxByPonChip(remote_olt_id);
    if ( SYS_SLOT_IS_VALID_REMOTE(iSlot) )
    {
        iPort = GetPonPortByPonChip(remote_olt_id);
        if ( SYS_PORT_IS_VALID_REMOTE(iPort) )
        {
            remote_apply_poninfo *pPonRemoteInfo; 
            
            if ( NULL != (pPonRemoteInfo = (remote_apply_poninfo*)devsm_remote_port_get_apply(iSlot, iPort, g_iPonRemoteApplyID)) )
            {
                pPonRemoteInfo->sLocalOlt = OLT_ID_INVALID;
                devsm_remote_port_release_apply(iSlot, iPort, g_iPonRemoteApplyID);
            }
                
            devsm_remote_port_clrlocalport(iSlot, iPort);
            iRlt = 0;
        }
    }
    
    return iRlt;
}
#endif



#endif


#ifdef __cplusplus

}

#endif

