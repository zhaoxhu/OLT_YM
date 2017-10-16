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
**        ����GFA6100 ��Ʒ֧��
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *                              OnuVoiceEnable VoiceEn->VoiceEnable -- ONU ����ģ�鹤����־��1-���� 0-������
 * 						 OnuVoiceEnable VoiceEn->TdmSlot -- TDM���λ��4-8
 *                              OnuVoiceEnable VoiceEn->SgIdx -- TDM �˿�������1 -3
 *                              OnuVoiceEnable VoiceEn->LogicOnuIdx -- ONU��TDM�˿��е��߼���ţ�1-256
 *    Desc:   ONU����ʹ������ONU����ʹ�ܼ�����MAC������CLI�����ܵ���
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

	/* ����OAM ��ϢPDU */
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

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_MAC_REQ, (unsigned char *) &OnuVoiceEn_Send, PduLength);

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_MAC_REQ, (unsigned char *) &OnuVoiceEn_Send, PduLength);

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *    �������:
 *               unsigned char * VoiceEnable -- ָʾ����ģ���Ƿ�����1��������0��������
 *               unsigned char * SrcMac -- ��������MAC��ַ��ONU FPGA TDMģ��ԴMAC��ַ��
 *               unsigned char * DestMac -- ��������MAC��ַ��ONU FPGA TDMģ��Ŀ��MAC��ַ��
 *
 *    Desc:   ��ѯONU����ʹ�ܼ�����MAC�����ڵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuVoiceEnable );
	VOS_MemSet( (unsigned char *)&OnuVoiceEn_Send,0, PduLength );
	OnuVoiceEn_Send.MsgType = GET_ONU_VOICE_MAC_REQ;

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *                           
 *    Desc:   ����ONU����VLAN������CLI�����ܵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_VoiceVlanEnable );
	VOS_MemSet( (unsigned char *)&OnuVoiceVlan_Send,0, PduLength );
	OnuVoiceVlan_Send.MsgType = SET_ONU_VOICE_VLAN_REQ ;
	if(VoiceVlan->Enable == ENABLE)
		OnuVoiceVlan_Send.Enable = ENABLE;
	else  OnuVoiceVlan_Send.Enable = DISABLE;
	OnuVoiceVlan_Send.VlanTag = (VoiceVlan->Priority << 13 ) + VoiceVlan->VlanId;

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_MAC_REQ, (unsigned char *)&OnuVoiceEn_Send, PduLength);

	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_VLAN_REQ, (unsigned char *)&OnuVoiceVlan_Send, PduLength);

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
*    �������:  unsigned int onuDevIdx -- onu �豸����
*    �������:
*            VoiceVlanEnable *VoiceVlan -- ���������ֶ�
*                    unsigned char VoiceVlanEnable - ���������Ƿ����VLAN��ǩ��1 -��tag��0 -����tag
*                    unsigned char VoicePriority - �������ݱ��ĵ����ȼ�
*                    unsigned short int VlanId - �������ݱ���VLAN id
*
*    Desc:   ��ѯONU����VLAN�����ڵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_VoiceVlanEnable );
	VOS_MemSet( (unsigned char *)&VoiceVlanEn_Send, 0, PduLength );
	VoiceVlanEn_Send.MsgType = GET_ONU_VOICE_VLAN_REQ;
	
	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 	
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_VLAN_REQ, (unsigned char *)&VoiceVlanEn_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_VLAN_REQ, (unsigned char *)&VoiceVlanEn_Send, PduLength);
	
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    �������:  
 *           unsigned long OnuDeviceIdx -ONU�豸������4�ֽڣ�SLOT * 10000 + PON * 1000 + ONU_id
 *           unsigned char PotsEnable - POTS��ʹ�ܱ�־��1��ʹ�ܣ�0���ر�
 *           unsigned char PotsNum - Ҫ���õ�Pots�ڸ�����
 *           unsigned int PotsList - POTS�˿��б�ÿһ��bit��Ӧһ��POTS�˿ڣ�bit0��ӦPOTS1��
 *                             bit1��ӦPOTS2,���� ,Bit15��ӦPOTS16��bit16��31��������1ʱ��ʾ�ڶ�
 *                             ����ִ��PotsEnable��ʶ�Ĳ�������0��ʾ�˿��ϲ�ִ��PotsEnable��
 *                             ʶ�Ĳ�����0xff��ʾ����POTS�˿�
 *    �������: ��
 *                           
 *    Desc:   ����ONU POTS������CLI�����ܵ��á�ONU POTS��ʹ�ܸ�OLT�����õ�����
 *               �����йأ�ԭ����ֻ���������������ӵĶ˿�ʹ�ܴ򿪣�������
 *               ��Ӧ�ر�
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
	
	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuPotsEnableConfig );
	VOS_MemSet((unsigned char *)&OnuPotsEnable_Send, 0, PduLength );
	OnuPotsEnable_Send.MsgType = SET_ONU_VOICE_POTS_REQ ;
	/* �˴�Ӧ���Ӷ�PotsNum ��PotsList �ļ�飬�����Ƿ�һ��*/
	if(PotsEnable == ENABLE)
		OnuPotsEnable_Send.PotsEnable = ENABLE;
	else OnuPotsEnable_Send.PotsEnable = DISABLE;
	OnuPotsEnable_Send.PotsList = PotsList;

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_POTS_REQ, (unsigned char *) &OnuPotsEnable_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_POTS_REQ, (unsigned char *) &OnuPotsEnable_Send, PduLength);

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
*    �������:  
*           unsigned int onuDevIdx -- onu �豸����
*           unsigned char PotsNum - Ҫ��ѯ��Pots�ڸ�����
*           unsigned long PotsList -  Ҫ��ѯ��Pots�˿��б�ÿһ��bit��Ӧһ��POTS�˿�,
*                       bit0��ӦPOTS1��bit1��ӦPOTS2,���� ,Bit15��ӦPOTS16��bit16��31������ 
*                       0xff����ʾ����POTS�˿ڣ�
*     
*    �������:
*            unsigned int *PotsEnableList -POTS�˿�ʹ���б�ÿһ��bit��Ӧһ��POTS�˿ڡ�
*                       PotsList��ĳbit=1,��PotsEnableList�ж�Ӧbit��1����ʾ�˿�ʹ�ܣ���0��ʾ
*                       �˿ڹر�
*
*    Desc:   ��ѯONU POTSʹ�ܣ����ڵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuPotsEnableQuery);
	VOS_MemSet( (unsigned char *)&OnuPotsEnable_Send, 0 , PduLength );
	OnuPotsEnable_Send.MsgType = GET_ONU_VOICE_POTS_REQ;
	OnuPotsEnable_Send.PotsList = PotsList;
	
	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_REQ,(char *) &OnuPotsEnable_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_REQ,(char *) &OnuPotsEnable_Send, PduLength);
		
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
*    �������:  
*           unsigned int onuDevIdx -- onu �豸����
*     
*    �������:
*        ungined long *PotsStatusList -POTS�˿�ժ�һ�״̬�б�ÿһ��bit��Ӧһ��POTS�˿ڡ�
*                      ��1��ժ������0 �һ�
*
*    Desc:   ��ѯONU ����POTSժ�һ�״̬�����ڵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuPotsStatusQuery);
	VOS_MemSet( (unsigned char *)&OnuPotsStatus_Send, 0 , PduLength );
	OnuPotsStatus_Send.MsgType = GET_ONU_VOICE_POTS_STATUS_REQ;
	
	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_STATUS_REQ, (char *)&OnuPotsStatus_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_POTS_STATUS_REQ, (char *)&OnuPotsStatus_Send, PduLength);

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *                              unsigned char LoopStatus -- ���ؿ��ƣ�1�����ء�0��ֹͣ����
 *                           
 *    Desc:   �ù�����Ҫ�������ԣ�ONU��������ʱ�����⻷ͬʱ�����ã�
 *                �ڻ��ǻ�·����ֱ�ӻ��أ��⻷��POTS1-4�ֱ��POTS5-8����
 *                POTS9-12�ֱ��POTS13-16��
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuPotsLoopback );
	VOS_MemSet( (unsigned char *)&OnuPotsLoop_Send,0, PduLength );
	OnuPotsLoop_Send.MsgType = SET_ONU_VOICE_LOOP_REQ ;
	if(LoopStatus == ENABLE)
		OnuPotsLoop_Send.LoopbackCtrl = ENABLE;
	else OnuPotsLoop_Send.LoopbackCtrl = DISABLE;

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, SET_ONU_VOICE_LOOP_REQ, (unsigned char *) &OnuPotsLoop_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, SET_ONU_VOICE_LOOP_REQ, (unsigned char *) &OnuPotsLoop_Send, PduLength);
		
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
*    �������:  unsigned int onuDevIdx -- onu �豸����
*    �������:
*            unsigned char *LoopStatus -- ���ؿ��ƣ�1�����ء�0��ֹͣ����
*
*    Desc:   ���ڵ���
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuPotsLoopback );
	VOS_MemSet( (unsigned char *)&OnuPotsLoop_Send, 0, PduLength );
	OnuPotsLoop_Send.MsgType = GET_ONU_VOICE_LOOP_REQ;
	
	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 	
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)&OnuPotsLoop_Send, PduLength);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)&OnuPotsLoop_Send, PduLength);
	
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    Param:  unsigned int onuDevIdx -- onu �豸����
 *                           
 *    Desc:   ��ѯONU�豸��Ϣ��չ����.
 *               ��λ������0��ʾ�忨ʽ�豸���豸�ṩ���ܲ�λ����0��ʾ�����豸��
 *               ��λ���ͣ������λ����0�����ֶβ����ڣ������λ������0�����ֶα�ʾ��Ӧ��λ�İ忨���ͣ�����ֵ������
 *               ��Դ���ͣ�0��48V��1��220V��
 *               ��أ��Ƿ����أ�1���С�0��û�У�
 *               ����������
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

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU 
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);
	V2R1_SendMsgToONU(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)OnuExtAttr, 2);
	GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);
	*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, GET_ONU_VOICE_LOOP_REQ, (unsigned char *)OnuExtAttr, 2);
	
	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    Param:  unsigned int onuDevIdx -- onu �豸����
 *                           
 *    Desc:   ONU����ҵ��������Ϣ��ONU����MAC��VLAN��POTS��ʹ�ܵ��ۺϣ�����ONU����ע�ᣨ����GFA-PON�Ȱβ��PON�������������ONU����ע�ᣩ����GFA-SG���Ȱβ�Ĵ���������Խ���OAM�շ����ɣ����OAMͨ��������Ч�ʣ�ȷ��ϵͳҵ��ָ��Ŀɿ��Ժ���Ч�ԣ���Ӧ��OAM��Ϣ
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
	
	/* ȡSG �µ�ǰ���õ�ONU */
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *                unsigned char *VoiceEnable -- onu ����ʹ�ܣ�1��������0��������
 *                unsigned char *SrcMac -- onu ����ԴMAC��ַ
 *                unsigned char *DestMac -- onu ����Ŀ��MAC��ַ
 *                struct VoiceVlanEnable *VlanEn -- onu vlan ʹ��
 *                unsigned long PotsEnableList -- onu pots��ʹ�ܱ�־
 *    �������:
 *                           
 *    Desc:   ����ONU����ҵ�����ԣ�����ONU����MAC��VLAN��POTS��ʹ�ܵ��ۺ�
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

	/* ����OAM ��ϢPDU */
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

	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU */
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, FC_ONU_TDM_SERVICE, (unsigned char *) &OnuVoiceConfig_Send, PduLength);
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    �������:  unsigned int onuDevIdx -- onu �豸����
 *    �������:
 *                unsigned char *VoiceEnable -- onu ����ʹ�ܣ�1��������0��������
 *                unsigned char *SrcMac -- onu ����ԴMAC��ַ
 *                unsigned char *DestMac -- onu ����Ŀ��MAC��ַ
 *                struct VoiceVlanEnable *VlanEn -- onu vlan ʹ��
 *                unsigned long *PotsEnableList -- onu pots��ʹ�ܱ�־
 *                           
 *    Desc:   ��ѯONU����ҵ�����ԣ�����ONU����MAC��VLAN��POTS��ʹ�ܵ��ۺϣ����ڵ���Ŀ��
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

	/* ����OAM ��ϢPDU */
	PduLength = sizeof(OAM_OnuVoiceService );
	VOS_MemSet((unsigned char *)&OnuVoiceConfig_Send, 0, PduLength );
	OnuVoiceConfig_Send.MsgType = GET_ONU_VOICE_SERVICE_REQ;
	
	/* ����Ϣͨ��GW_OAMͨ�ŷ��͵�ONU */
	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_OAM_COMM_WAIT_SEND);*/
	GetSemId = EQU_SendMsgToOnu(PonPortIdx, OnuIdx, FC_ONU_TDM_SERVICE, (unsigned char *)&OnuVoiceConfig_Send, PduLength);

	/*GetSemId = VOS_SemTake(OnuEUQRecvSemId, ONU_RESPONSE_TIMEOUT);*/

	/* �ȴ��������ص���Ӧ��Ϣ*/
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
 *    Param:  unsigned char TdmSlot -- tdm �忨����
 *                unsigned char SgIdx -- tdm ��������������
 *                           
 *    Desc:   ONU����ҵ��������Ϣ��ONU����MAC��VLAN��POTS��ʹ�ܵ��ۺϣ�����TDM�������󣨰���GFA-SG���Ȱβ�Ĵ������)ONU ����ҵ������
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
	/* �˴�ONU �����������еı��Ϊ1-256, ��Ӧ��1 */
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
	PotsEnableList = -1;				/* �ݲ�����POTS��ʹ�� */

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

