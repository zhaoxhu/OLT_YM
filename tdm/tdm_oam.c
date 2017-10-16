/***************************************************************
*
*			Module Name:  tdmService.c
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
*   Date: 			2007/11/6
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  07/11/6    |   chenfj          |     create 
**----------|-----------|------------------
**
**   modified by chenfj 2008-7-28
**        增加GFA6100 产品支持
**
*****************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


#include  "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"

#ifdef ONU_OAM_COMM
#endif
/******************************************************************
 *    Function:  SetOnuVoiceEnable(unsigned long OnuDeviceIdx,struct OnuVoiceEnable VoiceEn)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *                              OnuVoiceEnable VoiceEn->VoiceEnable -- ONU 语音模块工作标志；1-工作 0-不工作
 * 						 OnuVoiceEnable VoiceEn->TdmSlot -- TDM板槽位，4-8
 *                              OnuVoiceEnable VoiceEn->SgIdx -- TDM 端口索引；1 -3
 *                              OnuVoiceEnable VoiceEn->LogicOnuIdx -- ONU在TDM端口中的逻辑编号，1-256
 *    Desc:   ONU语音使能设置ONU语音使能及语音MAC，用于CLI及网管调用
 *
 *    Return:    V2R1_TDM_OK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
 int SetOnuVoiceEnable(unsigned long onuDevIdx,  OnuVoiceEnable *VoiceEn)
 {
 	int GetSemId;
	short int PonPortIdx, OnuIdx;
	int slot, port;
	OAM_OnuVoiceEnable  OnuVoiceEn_Send, *OnuVoiceEn_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx = GET_ONUID(onuDevIdx)/*( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( V2R1_TDM_ONU_OFFLINE );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuVoiceEnable );
	VOS_MemSet( (unsigned char *)&OnuVoiceEn_Send,0, PduLength );
	OnuVoiceEn_Send.MsgType = SET_ONU_VOICE_MAC_REQ ;
	if(VoiceEn->VoiceEnable == ENABLE)
		OnuVoiceEn_Send.VoiceEnable = ENABLE;
	else OnuVoiceEn_Send.VoiceEnable = DISABLE;
	VOS_MemCpy( OnuVoiceEn_Send.VoiceSrcMac, VoiceMacOnuFpga, 4);
	VOS_MemCpy( OnuVoiceEn_Send.VoiceDstMac, VoiceMacOltFpga, 5);
	OnuVoiceEn_Send.VoiceSrcMac[4] = (VoiceEn->SgIdx)-1 ;
	OnuVoiceEn_Send.VoiceSrcMac[5] = VoiceEn->LogicOnuIdx-1;
	OnuVoiceEn_Send.VoiceDstMac[5] = (VoiceEn->SgIdx) -1;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_MAC_REQ, (unsigned char *) &OnuVoiceEn_Send, PduLength);

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_MAC_REQ, (unsigned char *) &OnuVoiceEn_Send, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_TDM_ERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf("\r\nset onu voice MAC,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuVoiceEn_Recv = ( OAM_OnuVoiceEnable *)RecvMsgFromOnu;
		if( OnuVoiceEn_Recv->MsgType == SET_ONU_VOICE_MAC_RSP )
			{
			if( OnuVoiceEn_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				return( V2R1_TDM_OK );
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}	

 }

/******************************************************************
 *    Function:  GetOnuVoiceEnable(unsigned long OnuDeviceIdx, unsigned char * VoiceEnable, unsigned char * SrcMac, unsigned char * DestMac)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *    输出参数:
 *               unsigned char * VoiceEnable -- 指示语音模块是否工作，1－工作、0－不工作
 *               unsigned char * SrcMac -- 下行语音MAC地址（ONU FPGA TDM模块源MAC地址）
 *               unsigned char * DestMac -- 上行语音MAC地址（ONU FPGA TDM模块目的MAC地址）
 *
 *    Desc:   查询ONU语音使能及语音MAC，用于调试
 *
 *    Return:    V2R1_TDM_OK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
 int GetOnuVoiceEnable(unsigned long onuDevIdx, unsigned char * VoiceEnable, unsigned char * SrcMac, unsigned char * DestMac)
 {
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int GetSemId;
	OAM_OnuVoiceEnable OnuVoiceEn_Send, *OnuVoiceEn_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(( VoiceEnable == NULL ) || (SrcMac == NULL) || ( DestMac == NULL )) return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuVoiceEnable );
	VOS_MemSet( (unsigned char *)&OnuVoiceEn_Send,0, PduLength );
	OnuVoiceEn_Send.MsgType = GET_ONU_VOICE_MAC_REQ;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);
	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu voice MAC,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuVoiceEn_Recv = (OAM_OnuVoiceEnable *)RecvMsgFromOnu;
		if( OnuVoiceEn_Recv->MsgType == GET_ONU_VOICE_MAC_RSP )
			{
			if( OnuVoiceEn_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{				
				*VoiceEnable = OnuVoiceEn_Recv->VoiceEnable;
				VOS_MemCpy( SrcMac, OnuVoiceEn_Recv->VoiceSrcMac, 6 );
				VOS_MemCpy( DestMac, OnuVoiceEn_Recv->VoiceDstMac, 6 );				
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}
/******************************************************************
 *    Function:  SetOnuVoiceEnable(unsigned long OnuDeviceIdx,struct OnuVoiceEnable VoiceEn)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *                           
 *    Desc:   设置ONU语音VLAN，用于CLI及网管调用
 *
 *    Return:    V2R1_TDM_OK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
 int SetOnuVoiceVlan(unsigned long onuDevIdx, VoiceVlanEnable *VoiceVlan)
 {
 	int GetSemId;
	short int PonPortIdx, OnuIdx;
	int slot, port;
	OAM_VoiceVlanEnable  OnuVoiceVlan_Send, *OnuVoiceVlan_Recv;
	int PduLength;

         slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( V2R1_TDM_ONU_OFFLINE );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_VoiceVlanEnable );
	VOS_MemSet( (unsigned char *)&OnuVoiceVlan_Send,0, PduLength );
	OnuVoiceVlan_Send.MsgType = SET_ONU_VOICE_VLAN_REQ ;
	if(VoiceVlan->Enable == ENABLE)
		OnuVoiceVlan_Send.Enable = ENABLE;
	else  OnuVoiceVlan_Send.Enable = DISABLE;
	OnuVoiceVlan_Send.VlanTag = (VoiceVlan->Priority << 13 ) + VoiceVlan->VlanId;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_VLAN_REQ, (unsigned char *)&OnuVoiceVlan_Send, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_TDM_ERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nset onu voice MAC,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuVoiceVlan_Recv = (OAM_VoiceVlanEnable *)RecvMsgFromOnu;
		if( OnuVoiceVlan_Recv->MsgType  == SET_ONU_VOICE_VLAN_RSP )
			{
			if( OnuVoiceVlan_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				return( V2R1_TDM_OK );
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}	

 }

/******************************************************************
*    Function:  int GetOnuVoiceVlan(unsigned long OnuDeviceIdx, struct VoiceVlanEnable *VoiceVlan)
*
*    输入参数:  unsigned int onuDevIdx -- onu 设备索引
*    输出参数:
*            VoiceVlanEnable *VoiceVlan -- 包含三个字段
*                    unsigned char VoiceVlanEnable - 语音数据是否插入VLAN标签，1 -带tag、0 -不带tag
*                    unsigned char VoicePriority - 语音数据报文的优先级
*                    unsigned short int VlanId - 语音数据报文VLAN id
*
*    Desc:   查询ONU语音VLAN，用于调试
*
*    Return:    V2R1_TDM_OK
*
*    Notes: 
*
*    modified:
*
*********************************************************************/
int GetOnuVoiceVlan(unsigned long onuDevIdx,  VoiceVlanEnable *VoiceVlan)
 {
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int GetSemId;
	OAM_VoiceVlanEnable VoiceVlanEn_Send, *VoiceVlanEn_Recv;
	int PduLength;

         slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(VoiceVlan == NULL )  return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_VoiceVlanEnable );
	VOS_MemSet( (unsigned char *)&VoiceVlanEn_Send, 0, PduLength );
	VoiceVlanEn_Send.MsgType = GET_ONU_VOICE_VLAN_REQ;
	
	/* 将消息通过GW_OAM通信发送到ONU 	
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_VLAN_REQ, (unsigned char *)&VoiceVlanEn_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_VLAN_REQ, (unsigned char *)&VoiceVlanEn_Send, PduLength);
	
	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu voice MAC,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		VoiceVlanEn_Recv = (OAM_VoiceVlanEnable *)RecvMsgFromOnu;
		if( VoiceVlanEn_Recv->MsgType == GET_ONU_VOICE_VLAN_RSP )
			{
			if( VoiceVlanEn_Recv->Result  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
				VoiceVlan->Enable = VoiceVlanEn_Recv->Enable;
				VoiceVlan->Priority = VoiceVlanEn_Recv->VlanTag >> 13;
				VoiceVlan->VlanId = VoiceVlanEn_Recv->VlanTag & 0x1fff;				
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}
/******************************************************************
 *    Function:  SetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList)
 *
 *    输入参数:  
 *           unsigned long OnuDeviceIdx -ONU设备索引，4字节，SLOT * 10000 + PON * 1000 + ONU_id
 *           unsigned char PotsEnable - POTS口使能标志，1－使能，0－关闭
 *           unsigned char PotsNum - 要设置的Pots口个数，
 *           unsigned int PotsList - POTS端口列表；每一个bit对应一个POTS端口，bit0对应POTS1，
 *                             bit1对应POTS2,…… ,Bit15对应POTS16；bit16～31保留；＝1时表示在端
 *                             口上执行PotsEnable标识的操作，＝0表示端口上不执行PotsEnable标
 *                             识的操作。0xff表示所有POTS端口
 *    输出参数: 无
 *                           
 *    Desc:   设置ONU POTS，用于CLI及网管调用。ONU POTS口使能跟OLT侧配置的语音
 *               链接有关，原则上只有配置了语音链接的端口使能打开，其它端
 *               口应关闭
 *
 *    Return:    V2R1_TDM_OK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int SetOnuPotsEnable(unsigned long onuDevIdx,unsigned char PotsEnable,unsigned char PotsNum,unsigned long PotsList)
 {
 	int GetSemId;
	short int PonPortIdx, OnuIdx;
	int slot, port;
	OAM_OnuPotsEnableConfig  OnuPotsEnable_Send;
	OAM_OnuPotsEnableQuery  *OnuPotsEnable_Recv;
	int PduLength;

        slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( V2R1_TDM_ONU_OFFLINE );
	
	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuPotsEnableConfig );
	VOS_MemSet((unsigned char *)&OnuPotsEnable_Send, 0, PduLength );
	OnuPotsEnable_Send.MsgType = SET_ONU_VOICE_POTS_REQ ;
	/* 此处应增加对PotsNum 与PotsList 的检查，看其是否一致*/
	if(PotsEnable == ENABLE)
		OnuPotsEnable_Send.PotsEnable = ENABLE;
	else OnuPotsEnable_Send.PotsEnable = DISABLE;
	OnuPotsEnable_Send.PotsList = PotsList;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_POTS_REQ, (unsigned char *) &OnuPotsEnable_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_POTS_REQ, (unsigned char *) &OnuPotsEnable_Send, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_TDM_ERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nset onu POTS enable,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuPotsEnable_Recv = (OAM_OnuPotsEnableQuery *)RecvMsgFromOnu;
		if( OnuPotsEnable_Recv->MsgType == SET_ONU_VOICE_POTS_RSP )
			{
			if( OnuPotsEnable_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				return( V2R1_TDM_OK );
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}	

 }

/******************************************************************
*    Function:  int GetOnuPotsEnable(unsigned long OnuDeviceIdx,unsigned char PotsNum,unsigned long PotsList, ungined long *PotsEnableList)
*
*    输入参数:  
*           unsigned int onuDevIdx -- onu 设备索引
*           unsigned char PotsNum - 要查询的Pots口个数，
*           unsigned long PotsList -  要查询的Pots端口列表，每一个bit对应一个POTS端口,
*                       bit0对应POTS1，bit1对应POTS2,…… ,Bit15对应POTS16；bit16～31保留； 
*                       0xff－表示所有POTS端口；
*     
*    输出参数:
*            unsigned int *PotsEnableList -POTS端口使能列表；每一个bit对应一个POTS端口。
*                       PotsList中某bit=1,且PotsEnableList中对应bit＝1，表示端口使能；＝0表示
*                       端口关闭
*
*    Desc:   查询ONU POTS使能，用于调试
*
*    Return:    V2R1_TDM_OK
*
*    Notes: 
*
*    modified:
*
*********************************************************************/
int GetOnuPotsEnable(unsigned long onuDevIdx,unsigned char PotsNum,unsigned long PotsList, unsigned long *PotsEnableList)
 {
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int GetSemId;
	OAM_OnuPotsEnableQuery  OnuPotsEnable_Send, *OnuPotsEnable_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(PotsEnableList == NULL )  return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuPotsEnableQuery);
	VOS_MemSet( (unsigned char *)&OnuPotsEnable_Send, 0 , PduLength );
	OnuPotsEnable_Send.MsgType = GET_ONU_VOICE_POTS_REQ;
	OnuPotsEnable_Send.PotsList = PotsList;
	
	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_REQ,(char *) &OnuPotsEnable_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_REQ,(char *) &OnuPotsEnable_Send, PduLength);
		
	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu POTS enable,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuPotsEnable_Recv = (OAM_OnuPotsEnableQuery *)RecvMsgFromOnu;
		if( OnuPotsEnable_Recv->MsgType  == GET_ONU_VOICE_POTS_RSP )
			{
			if( OnuPotsEnable_Recv->Result  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
				*PotsEnableList = OnuPotsEnable_Recv->PotsListEnable;				
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}

/******************************************************************
*    Function:  int GetOnuPotsStatus(unsigned long OnuDeviceIdx,ungined long *PotsStatusList)
*
*    输入参数:  
*           unsigned int onuDevIdx -- onu 设备索引
*     
*    输出参数:
*        ungined long *PotsStatusList -POTS端口摘挂机状态列表；每一个bit对应一个POTS端口。
*                      ＝1，摘机；＝0 挂机
*
*    Desc:   查询ONU 所有POTS摘挂机状态，用于调试
*
*    Return:    V2R1_TDM_OK
*
*    Notes: 
*
*    modified:
*
*********************************************************************/
int GetOnuPotsStatus(unsigned long onuDevIdx, unsigned long *PotsStatusList)
 {
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int GetSemId;
	OAM_OnuPotsStatusQuery  OnuPotsStatus_Send, *OnuPotsStatus_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(PotsStatusList == NULL )  return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuPotsStatusQuery);
	VOS_MemSet( (unsigned char *)&OnuPotsStatus_Send, 0 , PduLength );
	OnuPotsStatus_Send.MsgType = GET_ONU_VOICE_POTS_STATUS_REQ;
	
	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_STATUS_REQ, (char *)&OnuPotsStatus_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_STATUS_REQ, (char *)&OnuPotsStatus_Send, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu POTS status,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuPotsStatus_Recv = (OAM_OnuPotsStatusQuery *)RecvMsgFromOnu;
		if( OnuPotsStatus_Recv->MsgType == GET_ONU_VOICE_POTS_STATUS_RSP )
			{
			if( OnuPotsStatus_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
				*PotsStatusList = OnuPotsStatus_Recv->PotsListStatus;				
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}
/******************************************************************
 *    Function:  int SetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char LoopStatus)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *                              unsigned char LoopStatus -- 环回控制：1－环回、0－停止环回
 *                           
 *    Desc:   该功能主要用作测试，ONU语音环回时，内外环同时起作用，
 *                内环是话路数据直接环回，外环是POTS1-4分别和POTS5-8环、
 *                POTS9-12分别和POTS13-16环
 *
 *    Return:    V2R1_TDM_OK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int SetOnuVoiceLoopbackStatus(unsigned long onuDevIdx, unsigned char LoopStatus)
 {
 	int GetSemId;
	short int PonPortIdx, OnuIdx;
	int slot, port;
	OAM_OnuPotsLoopback  OnuPotsLoop_Send, *OnuPotsLoop_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		return( V2R1_TDM_ONU_OFFLINE );

	if(( LoopStatus != V2R1_ONU_POTS_LOOP_ENABLE ) && ( LoopStatus != V2R1_ONU_POTS_LOOP_DISABLE ))
		return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuPotsLoopback );
	VOS_MemSet( (unsigned char *)&OnuPotsLoop_Send,0, PduLength );
	OnuPotsLoop_Send.MsgType = SET_ONU_VOICE_LOOP_REQ ;
	if(LoopStatus == ENABLE)
		OnuPotsLoop_Send.LoopbackCtrl = ENABLE;
	else OnuPotsLoop_Send.LoopbackCtrl = DISABLE;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_LOOP_REQ, (unsigned char *) &OnuPotsLoop_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_LOOP_REQ, (unsigned char *) &OnuPotsLoop_Send, PduLength);
		
	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_TDM_ERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nset onu POTS loopback,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuPotsLoop_Recv = (OAM_OnuPotsLoopback *)RecvMsgFromOnu;
		if( OnuPotsLoop_Recv->MsgType  == SET_ONU_VOICE_LOOP_RSP )
			{
			if( OnuPotsLoop_Recv->Result ==  V2R1_TDM_ONU_COMM_SUCCESS )
				return( V2R1_TDM_OK );
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}	

 }

/******************************************************************
*    Function:  int GetOnuVoiceLoopbackStatus(unsigned long OnuDeviceIdx, unsigned char *LoopStatus)
*
*    输入参数:  unsigned int onuDevIdx -- onu 设备索引
*    输出参数:
*            unsigned char *LoopStatus -- 环回控制：1－环回、0－停止环回
*
*    Desc:   用于调试
*
*    Return:    V2R1_TDM_OK
*
*    Notes: 
*
*    modified:
*
*********************************************************************/
int GetOnuVoiceLoopbackStatus(unsigned long onuDevIdx, unsigned char *LoopStatus)
 {
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int GetSemId;
	OAM_OnuPotsLoopback  OnuPotsLoop_Send, *OnuPotsLoop_Recv;
	int PduLength;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(LoopStatus == NULL )  return( V2R1_TDM_ERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuPotsLoopback );
	VOS_MemSet( (unsigned char *)&OnuPotsLoop_Send, 0, PduLength );
	OnuPotsLoop_Send.MsgType = GET_ONU_VOICE_LOOP_REQ;
	
	/* 将消息通过GW_OAM通信发送到ONU 	
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)&OnuPotsLoop_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)&OnuPotsLoop_Send, PduLength);
	
	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu POTS loopback status,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuPotsLoop_Recv = (OAM_OnuPotsLoopback *)RecvMsgFromOnu;
		if( OnuPotsLoop_Recv->MsgType == GET_ONU_VOICE_LOOP_RSP )
			{
			if( OnuPotsLoop_Recv->Result  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
				*LoopStatus = OnuPotsLoop_Recv->LoopbackCtrl;		
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}

/******************************************************************
 *    Function:  StartOnuTdmVoiceConfigByIdx(unsigned int onuDevIdx )
 *
 *    Param:  unsigned int onuDevIdx -- onu 设备索引
 *                           
 *    Desc:   查询ONU设备信息扩展属性.
 *               槽位数：非0表示插卡式设备，设备提供的总槽位数；0表示单板设备；
 *               槽位类型：如果槽位数＝0，该字段不存在；如果槽位数＝非0，该字段表示对应槽位的板卡类型，具体值待定；
 *               电源类型：0－48V、1－220V；
 *               电池：是否带电池，1－有、0－没有；
 *               环境：待定
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int GetOnuVoiceDeviceInfo(unsigned long onuDevIdx, unsigned char *SlotNum, unsigned char *SlotInfo, unsigned char *Power, unsigned char *Battery, unsigned char *Surronding)
{
	short int PonPortIdx, OnuIdx;
	int slot, port;
	int GetSemId;
	unsigned char OnuExtAttr[2] = {0};
	unsigned char *OnuExtAttrRsp;

	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if((SlotNum == NULL) || ( SlotInfo == NULL) || (Power == NULL) || (Battery == NULL) ||(Surronding == NULL ))
		return( RERROR );

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) == V2R1_ONU_NOT_EXIST )
		{
		if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
			sys_console_printf("onu %d/%d/%d is not exist\r\n",slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
			sys_console_printf("onu %d%d/%d is off-line\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ONU_OFFLINE );
		}

	OnuExtAttr[0] = SET_ONU_VOICE_EXT_EUQ_REQ;

	/* 将消息通过GW_OAM通信发送到ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)OnuExtAttr, 2);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)OnuExtAttr, 2);
	
	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if( EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu POTS loopback status,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuExtAttrRsp = (unsigned char *)RecvMsgFromOnu;
		if( OnuExtAttrRsp[0] == SET_ONU_VOICE_EXT_EUQ_RSP)
			{
			if( OnuExtAttrRsp[1]  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
					
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
		
	return( ROK );
}

/******************************************************************
 *    Function:  ConfigOnuTdmVoiceService(unsigned int onuDevIdx )
 *
 *    Param:  unsigned int onuDevIdx -- onu 设备索引
 *                           
 *    Desc:   ONU语音业务属性消息是ONU语音MAC、VLAN、POTS口使能的综合，用于ONU重新注册（包括GFA-PON热拔插和PON保护倒换引起的ONU重新注册），或GFA-SG板热拔插的处理情况，以降低OAM收发负荷，提高OAM通道的利用效率，确保系统业务恢复的可靠性和有效性；对应的OAM消息
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/ 
int ConfigOnuTdmVoiceService(unsigned long onuDevIdx )
{
	unsigned long aulMsg[4] = { MODULE_ONU, FC_ONU_TDM_SERVICE_ALL, 0, 0};

	aulMsg[2] = onuDevIdx;
	
	if( VOS_QueSend( g_Onu_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL/*MSG_PRI_URGENT*/) != VOS_OK )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err(SendMsgToOnu)\r\n" );*/
		/*DelOneNodeFromGetEUQ(PonPortIdx, OnuIdx );*/
		return( RERROR );
	}

	return( ROK );
}

int ConfigAllOnuTdmVoiceService(unsigned char TdmSlot, unsigned char SgIdx )
{
	unsigned int i, ret ;
	
	unsigned long OnuCount = 0;
	unsigned long OnuDevIdx;
	
	/*if(__SYS_MODULE_TYPE__(TdmSlot ) != MODULE_E_GFA_SIG)*/
	if(SlotCardIsTdmSgBoard(TdmSlot) != ROK )
		{
		sys_console_printf(" slot %d is not %s card(config onu voice service) \r\n", (TdmSlot ), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_SIG_NAME_STR*/);
		return (RERROR );
		}
	
	/* 取SG 下当前配置的ONU */
	ret = GetAllOnuBelongToSG(TdmSlot, SgIdx, &OnuCount, OnuDevIdx_Array );
	if( ret != VOS_OK )
		{
		sys_console_printf(" Get voice-onu from %s%d/%d err\r\n", GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_SIG_NAME_STR*/, ( TdmSlot), SgIdx );
		return( RERROR );
		}

	for( i = 0; i< OnuCount; i++)
		{
		OnuDevIdx = (OnuDevIdx_Array[i] >> 8);
		ConfigOnuTdmVoiceService(OnuDevIdx);
		}

	return( ROK );
}

/* just for test */
#if 0
STATUS test_onu_voice_vlan_set(unsigned long onuDevIdx,UCHAR a,UCHAR  b,USHORT c)
{	int rc=0;
	VoiceVlanEnable lex;
	lex.Enable=a;
	lex.Priority=b;
	lex.VlanId=c;
	rc=SetOnuVoiceVlan( onuDevIdx, &lex);
	if(rc==V2R1_TDM_OK)
		sys_console_printf("\r\n set onu voice vlan success\r\n");
	if(rc==V2R1_TDM_ERROR)
		sys_console_printf("\r\n set onu voice vlan failure\r\n");
	return rc;
}

STATUS test_onu_voice_service_set (unsigned long onuDevIdx,UCHAR a,UCHAR b,UCHAR c,USHORT d,
										     UCHAR e,UCHAR f, USHORT g,ULONG h)
{	int rc=V2R1_TDM_ERROR;
	OnuVoiceEnable  tex;
	VoiceVlanEnable lex;
	tex.VoiceEnable=a;
	tex.TdmSlot=b;
	tex.SgIdx=c;
	tex.LogicOnuIdx=d;
	lex.Enable=e;
	lex.Priority=f;
	lex.VlanId=g;

	rc=SetOnuVoiceService( onuDevIdx, &tex, &lex, h);
	if (rc==V2R1_TDM_OK)
		sys_console_printf("\r\n set onu voice service success \r\n");
	if(rc==V2R1_TDM_ERROR)
		sys_console_printf("\r\n set onu voice service failure \r\n");
	return rc;

}
#endif

#ifdef  ONU_VOICE_CONFIG_WHEN_REGISTERED
#endif
/******************************************************************
 *    Function:  SetOnuVoiceService(unsigned long onuDevIdx,  OnuVoiceEnable *VoiceEn ,  VoiceVlanEnable *VlanEn, unsigned long PotsEnableList)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *                unsigned char *VoiceEnable -- onu 语音使能，1－工作、0－不工作
 *                unsigned char *SrcMac -- onu 语音源MAC地址
 *                unsigned char *DestMac -- onu 语音目的MAC地址
 *                struct VoiceVlanEnable *VlanEn -- onu vlan 使能
 *                unsigned long PotsEnableList -- onu pots口使能标志
 *    输出参数:
 *                           
 *    Desc:   设置ONU语音业务属性，包括ONU语音MAC、VLAN、POTS口使能的综合
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int SetOnuVoiceService(unsigned long onuDevIdx,  OnuVoiceEnable *VoiceEn ,  VoiceVlanEnable *VlanEn, unsigned long PotsEnableList)
{

	int GetSemId;
	OAM_OnuVoiceService  OnuVoiceConfig_Send, *OnuVoiceConfig_Recv;
	short int PonPortIdx, OnuIdx;
	int slot, port;
	int PduLength;
	
	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP ) 
		return( V2R1_TDM_ONU_OFFLINE );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuVoiceService );
	VOS_MemSet((unsigned char *)&OnuVoiceConfig_Send, 0, PduLength );
	if(VoiceEn->VoiceEnable == ENABLE)
		OnuVoiceConfig_Send.VoiceEnable = ENABLE;
	else OnuVoiceConfig_Send.VoiceEnable = DISABLE;
	VOS_MemCpy( OnuVoiceConfig_Send.VoiceSrcMac, VoiceMacOnuFpga, 4);
	VOS_MemCpy( OnuVoiceConfig_Send.VoiceDstMac, VoiceMacOltFpga, 5);
	OnuVoiceConfig_Send.VoiceSrcMac[4] = (VoiceEn->SgIdx) -1;
	OnuVoiceConfig_Send.VoiceSrcMac[5] = VoiceEn->LogicOnuIdx;
	OnuVoiceConfig_Send.VoiceDstMac[5] = (VoiceEn->SgIdx) -1;

	if(VlanEn->Enable == ENABLE)
		OnuVoiceConfig_Send.VlanEnable = ENABLE;
	else OnuVoiceConfig_Send.VlanEnable = DISABLE;
	OnuVoiceConfig_Send.VlanTag =  (VlanEn->Priority << 13 ) + VlanEn->VlanId;
	
	OnuVoiceConfig_Send.PotsEnableList = PotsEnableList ;
	OnuVoiceConfig_Send.MsgType = SET_ONU_VOICE_SERVICE_REQ;

	/* 将消息通过GW_OAM通信发送到ONU */
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, FC_ONU_TDM_SERVICE, (unsigned char *) &OnuVoiceConfig_Send, PduLength);
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nset onu voice service,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuVoiceConfig_Recv = ( OAM_OnuVoiceService *)RecvMsgFromOnu;
		if( OnuVoiceConfig_Recv->MsgType  == SET_ONU_VOICE_SERVICE_RSP )
			{
			if( OnuVoiceConfig_Recv->Result  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				return( V2R1_TDM_OK );
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}

}


/******************************************************************
 *    Function:  GetOnuVoiceService(unsigned long OnuDeviceIdx, unsigned char *VoiceEnable, unsigned char *SrcMac, unsigned char *DestMac, struct VoiceVlanEnable *VlanEn, unsigned long *PotsEnableList)
 *
 *    输入参数:  unsigned int onuDevIdx -- onu 设备索引
 *    输出参数:
 *                unsigned char *VoiceEnable -- onu 语音使能，1－工作、0－不工作
 *                unsigned char *SrcMac -- onu 语音源MAC地址
 *                unsigned char *DestMac -- onu 语音目的MAC地址
 *                struct VoiceVlanEnable *VlanEn -- onu vlan 使能
 *                unsigned long *PotsEnableList -- onu pots口使能标志
 *                           
 *    Desc:   查询ONU语音业务属性，包括ONU语音MAC、VLAN、POTS口使能的综合，用于调试目的
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/

int GetOnuVoiceService(unsigned long onuDevIdx, unsigned char *VoiceEnable, unsigned char *SrcMac, unsigned char *DestMac,  VoiceVlanEnable *VlanEn, unsigned long *PotsEnableList)
{
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;
	int ret;*/
	int GetSemId;
	OAM_OnuVoiceService  OnuVoiceConfig_Send, *OnuVoiceConfig_Recv;
	int PduLength;
	
	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if(( VoiceEnable == NULL ) || (SrcMac == NULL) || ( DestMac == NULL ) || (VlanEn == NULL ) ||( PotsEnableList == NULL )) return( RERROR );

	/* 创建OAM 消息PDU */
	PduLength = sizeof(OAM_OnuVoiceService );
	VOS_MemSet((unsigned char *)&OnuVoiceConfig_Send, 0, PduLength );
	OnuVoiceConfig_Send.MsgType = GET_ONU_VOICE_SERVICE_REQ;
	
	/* 将消息通过GW_OAM通信发送到ONU */
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, FC_ONU_TDM_SERVICE, (unsigned char *)&OnuVoiceConfig_Send, PduLength);

	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	/* 等待并处理返回的响应消息*/
	if( GetSemId == RERROR )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf("\r\nget onu voice service,but no RSP from onu%d/%d/%d\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}
	else {
		OnuVoiceConfig_Recv = (OAM_OnuVoiceService *)RecvMsgFromOnu;
		if( OnuVoiceConfig_Recv->MsgType == GET_ONU_VOICE_SERVICE_RSP )
			{
			if( OnuVoiceConfig_Recv->Result  ==  V2R1_TDM_ONU_COMM_SUCCESS )
				{
				*VoiceEnable = OnuVoiceConfig_Recv->VoiceEnable;
				VOS_MemCpy( SrcMac, OnuVoiceConfig_Recv->VoiceSrcMac, 6 );
				VOS_MemCpy( DestMac, OnuVoiceConfig_Recv->VoiceDstMac, 6 );

				VlanEn->Enable = OnuVoiceConfig_Recv->VlanEnable;
				VlanEn->Priority = OnuVoiceConfig_Recv->VlanTag >> 13 ;
				VlanEn->VlanId = OnuVoiceConfig_Recv->VlanTag & 0x1fff;

				*PotsEnableList = OnuVoiceConfig_Recv->PotsEnableList;
				
				return( V2R1_TDM_OK );
				}
			else return( V2R1_TDM_ERROR );
			}
		else return( V2R1_TDM_ERROR );
		}
}



/******************************************************************
 *    Function:  ConfigOnuVoiceService(unsigned char TdmSlot, unsigned char SgIdx )
 *
 *    Param:  unsigned char TdmSlot -- tdm 板卡索引
 *                unsigned char SgIdx -- tdm 板信令网关索引
 *                           
 *    Desc:   ONU语音业务属性消息是ONU语音MAC、VLAN、POTS口使能的综合，用于TDM板启动后（包括GFA-SG板热拔插的处理情况)ONU 语音业务配置
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int ConfigOnuVoiceService(unsigned long  onuDevIdx )
{
	short int PonPortIdx, OnuIdx;
	int slot, port;
	/*bool SupportVoice = V2R1_DISABLE ;*/
	int ret;
	unsigned char TdmSlot;
	unsigned char SgIdx;
	unsigned short int LogicOnuId=0;
	/*OAM_OnuVoiceService  OnuVoiceConfig;*/

	 unsigned char EnableFlag=0;
	 unsigned char Priority=0;
	 unsigned short int VlanId=0;

	 unsigned long PotsEnableList=0;	

	 OnuVoiceEnable  VoiceEnableConfig;
	 VoiceVlanEnable  VoiceVlanConfig;	 
	
	slot  = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	port = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000 ) / 1000*/;
	PonPortIdx = GetPonPortIdxBySlot( slot, port );
	OnuIdx =GET_ONUID(onuDevIdx)/* ( onuDevIdx % 1000 )*/-1;

	CHECK_ONU_RANGE

	if( ThisIsValidOnu(PonPortIdx, OnuIdx) == V2R1_ONU_NOT_EXIST )
		{
		if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
			sys_console_printf("onu %d/%d/%d is not exist\r\n",slot, port, (OnuIdx+1));
		return( V2R1_TDM_ERROR );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx ) != ONU_OPER_STATUS_UP )
		{
		if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
			sys_console_printf("onu %d%d/%d is off-line\r\n", slot, port, (OnuIdx+1));
		return( V2R1_TDM_ONU_OFFLINE );
		}

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot, &SgIdx, &LogicOnuId );
	/* 此处ONU 在信令网关中的编号为1-256, 故应减1 */
	if(LogicOnuId > 0 )
		LogicOnuId --;
		
	if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
		{
		switch( ret )
			{
			case TDM_VM_NOT_IN_ANY_SG:
				sys_console_printf("\r\nonu %d/%d/%d not config to any %s\r\n",slot, port, (OnuIdx+1), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_SIG_NAME_STR*/);
				break;
			case TDM_VM_NOT_SUPPORT_VOICE:
				sys_console_printf("\r\nonu %d/%d/%d not support voice\r\n",slot, port, (OnuIdx+1));
				break;
			case TDM_VM_ERR:
				sys_console_printf("\r\nGet onu%d/%d/%d belong to %s err\r\n",slot, port, (OnuIdx+1), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_SIG_NAME_STR*/);
				break;
			case TDM_VM_OK:
				sys_console_printf("\r\nonu %d/%d/%d is configed to %s%d, logic Id is %d\r\n",slot,port,(OnuIdx+1), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_SIG_NAME_STR*/, SgIdx, LogicOnuId );
				break;
			default :
				break;
			}
		}
	if( ret != TDM_VM_OK ) return( ret );
/* 
	OnuVoiceConfig.VoiceEnable = V2R1_ENABLE;
	VOS_MemCpy( OnuVoiceConfig.VoiceSrcMac, VoiceMacOnuFpga, 4);
	VOS_MemCpy( OnuVoiceConfig.VoiceDstMac, VoiceMacOltFpga, 5);
	OnuVoiceConfig.VoiceSrcMac[4] = SgIdx;
	OnuVoiceConfig.VoiceSrcMac[5] = LogicOnuId;
	OnuVoiceConfig.VoiceDstMac[5] = LogicOnuId;
*/
	VoiceEnableConfig.VoiceEnable = ENABLE;
	VoiceEnableConfig.TdmSlot = TdmSlot;
	VoiceEnableConfig.SgIdx = SgIdx;
	VoiceEnableConfig.LogicOnuIdx = LogicOnuId;

	ret = GetTdmSGVoiceVlanEnable(TDMCardIdx,SgIdx,&EnableFlag, &Priority, &VlanId);
	if( ret != TDM_VM_OK ) return( ret );
	if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
		{
		sys_console_printf("\r\nsg %d voice-vlan enable=%d, priority=%d, vid=%d\r\n",SgIdx,EnableFlag,Priority,VlanId);
		}
	/*OnuVoiceConfig.VlanEnable = EnableFlag;
	OnuVoiceConfig.VlanTag =  (Priority << 13 ) + VlanId;
	
	ret = GetOnuPotsLinkAll(onuDevIdx, &PotsEnableList);
	if( ret != TDM_VM_OK ) return( ret );
	if( DEBUG_ONU_TDM_VOICE == V2R1_ENABLE )
		{
		sys_console_printf("\r\nonu %d enabled pots-list=0x%x\r\n",onuDevIdx,PotsEnableList);
		}
	OnuVoiceConfig.PotsEnableList = PotsEnableList ;*/	/* removed by xieshl 20080527 */
	PotsEnableList = -1;				/* 暂不管理POTS口使能 */

	if(EnableFlag == ENABLE)
		VoiceVlanConfig.Enable = ENABLE;
	else VoiceVlanConfig.Enable = DISABLE;
	VoiceVlanConfig.Priority = Priority;
	VoiceVlanConfig.VlanId = VlanId;

	/*V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_SERVICE_REQ, &OnuVoiceConfig, sizeof(OnuVoiceService ));*/

	ret = SetOnuVoiceService(onuDevIdx, &VoiceEnableConfig,  &VoiceVlanConfig , PotsEnableList );
	
	return( ret );
}

#endif

#ifdef __cplusplus
}
#endif

