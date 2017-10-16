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

#include "lib_eponMonMib.h"
#include "bmsp/product_info/Bms_product_info.h"
#include "sys/main/sys_main.h"
#include "statistics/statistics.h"

#include  "Tdm_comm.h"
#include  "Tdm_apis.h"
#include  "TdmService.h"

#include "../e1/E1_apis.h"
#include "../e1/E1_oam.h"
/* begin: added by jianght 20090327  */
#include "../e1/E1_MIB.h"
#include  "eth_type.h"
/* end: added by jianght 20090327 */

unsigned char VoiceMacOnuFpga[6] = {0x00,0x0f,0xe9,0x04,0x00,0x00 };
unsigned char VoiceMacOltFpga[6] = {0x00,0x0f,0xe9,0x04,0x10,0x00 };
unsigned long OnuDevIdx_Array[MAXONUPERSG];

unsigned char 	E1MacOnuFpga[6] = {0x00,0x0f,0xe9,0x04,0x03,0x80 };  /* 0x80 ~ 0x87 */
unsigned char E1MacOltFpga[6] = {0x00,0x0f,0xe9,0x04,0x03,0x00 };    /* 0x0 ~ 0x7 */

unsigned char TdmStyleMac[4] = { 0x00,0x0f,0xe9,0x04};
unsigned short int  TdmMaxMac = 0x100f;
/*
unsigned char *V2R1_VoiceVlanName[]=
	{
		(unsigned char *)"unknown",
		(unsigned char *)"TDMVLAN1",
		(unsigned char *)"TDMVLAN2",
		(unsigned char *)"TDMVLAN3"
	};
*/

#if 0   /* deleted by chenfj 2008-7-31 */

#ifndef  VLAN_PORT_BASED
#define  VLAN_PORT_BASED  1
#endif

unsigned char MacAddr[6] = { 0x00,0x0f,0xe9,0x00,0x01,0x05};
unsigned int VID= 0x2;
unsigned int  PonPort = 0x5;

typedef unsigned char   bcm_mac_t[6];

int  AddStaticMacToSW(void)
{
	cli_l2_entry_t l2entry;
		
	VOS_MemSet( &l2entry, 0, sizeof(cli_l2_entry_t ));
	VOS_MemCpy( l2entry.mac, MacAddr, BYTES_IN_MAC_ADDRESS );
	l2entry.vid = VID;
	
	l2entry.l2_static = 1;
	l2entry.tgid_port = PonPortToSwLogPort[PonPort];
	
	bms_l2_add(0,&l2entry);
	return( ROK );
}

int DelStaticMacFromSW(void )
{

	return(bms_l2_delete(0,VID,MacAddr, 0));
}

int GetStaticMacFromSW( int flag)
{
	int i;
	int ret;
	cli_l2_addr_t  L2_Addr;
	
	for(i = 0; i< 8192; i++)
		{
		ret = bms_l2_entry_get(0,i,&L2_Addr, flag);
		if( ret == 0 )
			{
			sys_console_printf("entry:%d, vid=%d,mac address %02x:%02x:%02x:%02x:%02x:%02x\r\n",i,L2_Addr.vid, L2_Addr.mac[0],L2_Addr.mac[1],L2_Addr.mac[2],L2_Addr.mac[3],L2_Addr.mac[4],L2_Addr.mac[5] );
			}

		}
	return( ROK );
}
#endif


/**************************************************************
	����һ��MAC �Ƿ�����ڽ���оƬ�ĵ�ַ���У�
	�����ڣ�������������VLAN
***************************************************************/
int FindMacFromBCM56300( unsigned char *Mac, int *vid)
{
	int i;
	int index_min,index_max;
	int ret;
	cli_l2_addr_t  L2_Addr;
	
	if(bms_l2_idx_get( 0, &index_min, &index_max ) != ROK) return(RERROR);
	
	for(i = index_min; i<= index_max; i++)
		{
		ret = bms_l2_entry_get(0,i,&L2_Addr, 1);
		if( ret == 0 )
			{
			if( VOS_MemCmp(Mac, L2_Addr.mac, BYTES_IN_MAC_ADDRESS ) == 0)
				{
				*vid = L2_Addr.vid;
				return( ROK );
				}
			}

		}
	return( ROK );
}

int FindTdmStyleMacFromBCM56300( unsigned char *TdmMac, int *vid)
{
	int i;
	int index_min,index_max;
	int ret;
	cli_l2_addr_t  L2_Addr;

	if(bms_l2_idx_get( 0, &index_min, &index_max ) != ROK) return(RERROR);
	
	for(i = index_min; i<= index_max; i++)
		{
		ret = bms_l2_entry_get(0,i,&L2_Addr, 1);
		if( ret == 0 )
			{
			if( VOS_MemCmp(VoiceMacOnuFpga, L2_Addr.mac, (BYTES_IN_MAC_ADDRESS-2) ) == 0)
				{
				if(L2_Addr.mac[4] <= VoiceMacOltFpga[4])
					{
					*vid = L2_Addr.vid;
					return( ROK );
					}
				}
			/*
			if( VOS_MemCmp(VoiceMacOltFpga, L2_Addr.mac, (BYTES_IN_MAC_ADDRESS-1) ) == 0)
				{
				*vid = L2_Addr.vid;
				return( ROK );
				}
			*/
			}

		}
	return( ROK );

}

int  AddTdmMacToBCM56300(unsigned int slot, unsigned int port, unsigned char *MacAddress, unsigned int vlanid )
{
	cli_l2_entry_t l2entry = {0};
	short int PonPortIdx;
	

	PonPortIdx = GetPonPortIdxBySlot(slot, port);
	CHECK_PON_RANGE
		
	VOS_MemSet( &l2entry, 0, sizeof(cli_l2_entry_t ));
	VOS_MemCpy( l2entry.mac, MacAddress, BYTES_IN_MAC_ADDRESS );
	l2entry.vid = vlanid;
	
	l2entry.l2_static = 1;
	l2entry.tgid_port = PonPortToSwLogPort[PonPortIdx];
	
	bms_l2_add(0,&l2entry);
	return( ROK );
}

/*********   ���ڲ���ONU OAM ͨ��************

STATUS  test_onu_voice_enable_set (unsigned long onuDevIdx,UCHAR a,UCHAR  b,  UCHAR c,UCHAR d)
{	int  rc;
 	OnuVoiceEnable  lex;
	lex.VoiceEnable=a;
	lex.TdmSlot=b;
	lex.SgIdx=c;
	lex.LogicOnuIdx=d;
	rc=SetOnuVoiceEnable( onuDevIdx,&lex);
	if(rc==V2R1_TDM_ERROR) 
		sys_console_printf("\r\n set onu voice enable failure\r\n");
	else 
		sys_console_printf("\r\nset onu voice enable success\r\n");
	return rc;
	
}
*/
#ifdef TDM_STARTUP
#endif

#if  0    /* deleted by chenfj 2008-7-31 */
/*��������VLAN */
int  CreateVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx,  unsigned short int VoiceVid)
{
	int ret;
	short int Vid;
	unsigned char VoiceVlanInfo[V2R1_VLAN_INFO_LEN] = {0};
	unsigned char count = 5;
	
	ret = MN_Snmp_Vlan_Create(V2R1_VoiceVlanName[SgIdx],VoiceVid, &Vid, VLAN_PORT_BASED, 1, VoiceVlanInfo);
	if( ret != VOS_OK )
		{
		sys_console_printf("\r\ncrate voice vlan %d err\r\n",VoiceVid);
		return( RERROR );
		}
	
	while(((ret = MN_Vlan_Check_exist( VoiceVid )) != VOS_OK ) && ( count != 0 ))
		{
		VOS_TaskDelay(10);
		count --;
		}
	
	return( ret );
}

/* ɾ������VLAN */
int  DeleteVoiceVlan( unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid )
{
	int ret;
	unsigned char VoiceVlanInfo[V2R1_VLAN_INFO_LEN] = {0};
	unsigned char VoiceVlanName[V2R1_VLAN_NAME_LEN] = {0};
	unsigned short int Vid;
	unsigned char count = 5;
	
	/* ����VID ɾ��VLAN */
	ret = MN_Vlan_Check_exist(VoiceVid );
	if( ret == VOS_OK )
		{
		MN_Vlan_Delete(VoiceVid, 1, VoiceVlanInfo);
		}
	/* ����vlan ����ɾ��vlan */
	VOS_MemCpy( VoiceVlanName, V2R1_VoiceVlanName[SgIdx], VOS_StrLen(V2R1_VoiceVlanName[SgIdx]));
	
	ret = MN_Vlan_Get_Vid( &Vid, VoiceVlanName, 1,  VoiceVlanInfo);
	if( ret == VOS_OK )
		{
		MN_Vlan_Delete(Vid, 1, VoiceVlanInfo);
		}
	
	while(((ret = MN_Vlan_Check_exist( VoiceVid )) == VOS_OK ) && ( count != 0 ))
		{
		VOS_TaskDelay(10);
		count --;
		}
	
	return( ROK );
}

/* ɾ������VLAN */
int  DeleteVoiceVlanByName( unsigned char TdmSlot, unsigned char SgIdx )
{
	int ret;
	unsigned char VoiceVlanInfo[V2R1_VLAN_INFO_LEN] = {0};
	unsigned char VoiceVlanName[V2R1_VLAN_NAME_LEN] = {0};
	unsigned short int Vid;
	unsigned char count = 5;
	
	/* ����vlan ����ɾ��vlan */
	VOS_MemCpy( VoiceVlanName, V2R1_VoiceVlanName[SgIdx], VOS_StrLen(V2R1_VoiceVlanName[SgIdx]));
	
	ret = MN_Vlan_Get_Vid( &Vid, VoiceVlanName, 1,  VoiceVlanInfo);
	if( ret == VOS_OK )
		{
		MN_Vlan_Delete(Vid, 1, VoiceVlanInfo);
		}
	
	while(((ret =MN_Vlan_Check_exist( Vid ))== VOS_OK ) && ( count != 0 ))
		{
		VOS_TaskDelay(10);
		count --;
		}
	
	return( ROK );
}

/* ɾ������VLAN */
int  DeleteVoiceVlanByNameAll( unsigned char TdmSlot)
{
	unsigned long fpga, fpgaIdx;

	if(SlotCardIsTdmBoard(TdmSlot) != ROK ) 
		{
		sys_console_printf(" slot %d is not tdm card(delete voice vlan) \r\n", (TdmSlot));
		return (RERROR );
		}
	
	for(fpgaIdx = TDM_SG_MIN; fpgaIdx <= TDM_SG_MAX; fpgaIdx++)
		{
		if(getTdmChipInserted(TdmSlot-1, fpgaIdx-1) == TDM_FPGA_EXIST)
			DeleteVoiceVlanByName(TdmSlot, fpgaIdx);
		}
	/*
	fpga1 = getTdmChipInserted ( TdmSlot-1, TDM_SG_INDEX1-1);
	fpga2 = getTdmChipInserted ( TdmSlot-1, TDM_SG_INDEX2-1);
	fpga3 = getTdmChipInserted ( TdmSlot-1, TDM_SG_INDEX3-1);

	if(fpga1 == 1 )
		DeleteVoiceVlanByName(TdmSlot, TDM_SG_INDEX1);
	if(fpga2 == 1 )
		DeleteVoiceVlanByName(TdmSlot, TDM_SG_INDEX2);
	if(fpga3 == 1 )
		DeleteVoiceVlanByName(TdmSlot, TDM_SG_INDEX3);
	*/
	return( ROK );
}


/* ������VLAN ����Ӷ˿�*/
int  AddPortToVoiceVlan(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VlanId )
{
	
	int ret;
	int portList[13] = {0};
	unsigned char i;
	for(i=0; i<13;i++)
		portList[i] = 0;

	for(i=(PONCARD_FIRST); i<=(PONCARD_LAST);i++)
		{
		if((SlotCardIsTdmBoard(i) == ROK ) && ( i == TdmSlot ))
			{
			portList[i-1] = 0x80000000 >> ( SgIdx-1);
			}
		else if(SlotCardIsSwBoard(i) == ROK)
			{
			continue;
			}
		else/* if( __SYS_MODULE_TYPE__(i+1) == MODULE_E_GFA_EPON )*/
			{
			portList[i] = 0xf0000000;
			}
		}
/*	
	sys_console_printf("voice vlan %d port list \r\n", VlanId);
	for(i=0; i<13;i++)
		{
		sys_console_printf("0x%04x \r\n",portList[i]);
		}
*/	
	ret = MN_Vlan_Set_TaggedPorts((ULONG ) VlanId, (char *)&portList[0], 52 );
	if( ret != VOS_OK )
		{
		sys_console_printf("\r\nadd port to vlan %d err\r\n",VlanId);
			/*return( RERROR );*/
		}
	else{


		}

	return( ret );

}
#endif


#ifdef  _TDM_BOARD_SG_VOICE_OPERATION_
#endif
/********************************************************
*                                                                                                     *
*        ע: ���º�����ʹ����TDM ������ ����    *
*                                                                                                     *
*********************************************************/


/******************************************************************
 *    Function:  AddVoiceMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx)
 *
 *    Param:  unsigned char TdmSlot -- tdm �忨����; ȡֵ4 - 8
 *                unsigned char SgIdx -- tdm ��������������; ȡֵ: 1- 3
 *                unsigned int OnuDevIdx -- ONU �豸����
 *                unsigned short int VoiceVid -- ����VLAN; 
 *                unsigned short int LogicOnuIdx -- ONU �������������߼���ţ�ȡֵ1 �� 256
 *                           
 *    Desc:   ��	����оƬBCM56300 �����Ӿ�̬����MAC ( ��������ONU ) 
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int AddVoiceMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx)
{
	unsigned int slot, port;
	unsigned char VoiceMac[BYTES_IN_MAC_ADDRESS];

	VOS_MemCpy( VoiceMac, VoiceMacOnuFpga, BYTES_IN_MAC_ADDRESS );
	VoiceMac[4] = SgIdx - 1;
	VoiceMac[5] = LogicOnuIdx -1;
	slot =GET_PONSLOT(OnuDevIdx)/* OnuDevIdx / 10000*/;
	port = GET_PONPORT(OnuDevIdx)/*(OnuDevIdx % 10000 ) / 1000*/;
	AddTdmMacToBCM56300(slot, port, &VoiceMac[0],VoiceVid );
	return( ROK );
}

/* ��	����оƬBCM56300 �����Ӿ�̬����MAC (��������ONU )*/
int AddVoiceMacToSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int VoiceVid, unsigned int OnuCount, unsigned long *SgOnuIdx)
{
	unsigned short int LogicOnuIdx;
	unsigned int OnuDevIdx;
	unsigned int slot, port;
	unsigned char VoiceMac[BYTES_IN_MAC_ADDRESS];
	short int i;

	if(SgOnuIdx == NULL ) return( RERROR );
	
	VOS_MemCpy( VoiceMac, VoiceMacOnuFpga, BYTES_IN_MAC_ADDRESS );
	for( i = 0; i< OnuCount; i++)
		{
		LogicOnuIdx =(unsigned short int ) (SgOnuIdx[i] & 0xff );
		OnuDevIdx = (SgOnuIdx[i] >> 8);
		VoiceMac[4] = SgIdx - 1;
		VoiceMac[5] = LogicOnuIdx;
		slot = GET_PONSLOT(OnuDevIdx)/*OnuDevIdx / 10000*/;
		port =GET_PONPORT(OnuDevIdx)/* (OnuDevIdx % 10000 ) / 1000*/;
		AddTdmMacToBCM56300(slot, port, &VoiceMac[0],VoiceVid );
		}	

	/* ����OLT fpga ��̬MAC */
	VOS_MemCpy( VoiceMac, VoiceMacOltFpga, BYTES_IN_MAC_ADDRESS );
	VoiceMac[5] = SgIdx - 1;
	slot = TdmSlot;
	port = SgIdx;
	AddTdmMacToBCM56300(slot, port, &VoiceMac[0],VoiceVid );

	return( ROK );

}

/* ɾ������оƬ�о�̬����MAC (����ONU ) */
int DeleteVoiceMacFromSw(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicOnuIdx)
{
	unsigned char VoiceMac[BYTES_IN_MAC_ADDRESS];
	int VoiceVid;
	int ret;

	VOS_MemCpy( VoiceMac, VoiceMacOnuFpga, BYTES_IN_MAC_ADDRESS );
	VoiceMac[5]  =LogicOnuIdx-1;
	VoiceMac[4] = SgIdx - 1;

	ret = FindMacFromBCM56300(VoiceMac, &VoiceVid );
	if( ret == ROK )
		bms_l2_delete(0,VoiceVid,VoiceMac, 0);
	
	return( ret );	
}

/* ɾ������оƬ�����о�̬����MAC (ָ��TDM�ӿ�������ONU ) */
int DeleteVoiceMacFromSwAll(unsigned char TdmSlot, unsigned char SgIdx, unsigned int OnuCount, unsigned long *SgOnuIdx)
{
	unsigned char VoiceMac[BYTES_IN_MAC_ADDRESS];
	short int i;
	int ret;
	int VoiceVid;
	
	/* ���������BCM56300 ��ԭ�е�ONU  fpga ��̬����MAC  */
	VOS_MemCpy( VoiceMac, VoiceMacOnuFpga, BYTES_IN_MAC_ADDRESS );
	for( i = 0; i< OnuCount; i++)
		{
		VoiceMac[5]  = (SgOnuIdx[i] & 0xff );
		VoiceMac[4] = SgIdx - 1;

		ret = FindMacFromBCM56300(VoiceMac, &VoiceVid );
		if( ret == ROK )
			bms_l2_delete(0,VoiceVid,VoiceMac, 0);
		}

	/* �����������BCM56300 ��ԭ�е�TDM fpga ����MAC */
	VOS_MemCpy( VoiceMac, VoiceMacOltFpga, BYTES_IN_MAC_ADDRESS );
	VoiceMac[5] =  SgIdx - 1;
	ret = FindMacFromBCM56300(VoiceMac, &VoiceVid );
	if( ret == ROK )
		bms_l2_delete(0,VoiceVid,VoiceMac, 0);

	return( ROK );
}

/* �ָ�һ����������������vlan ����������ONU ��̬MAC */
int RestoreSWVoiceMacAndVlan(unsigned char TdmSlot, unsigned char SgIdx)
{
	unsigned int ret;
	unsigned long OnuCount = 0;
	VoiceVlanEnable OnuVoiceVlan;	
	unsigned short int VoiceVid;


	/* ȡSG ��ǰ���õ�ONU */
	ret = GetAllOnuBelongToSG(TdmSlot, SgIdx, &OnuCount, OnuDevIdx_Array );
	if( ret != VOS_OK )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf(" Get voice-onu from %s%d/%d err\r\n",GetGFA6xxxTdmNameString(),(TdmSlot), SgIdx );
		return( RERROR );
		}

	/* ȡSG �µ�ǰ������VLAN ����*/
	VOS_MemSet((void *) &OnuVoiceVlan, 0 , sizeof(VoiceVlanEnable));
	ret = GetTdmSGVoiceVlanEnable(TdmSlot, SgIdx, &(OnuVoiceVlan.Enable), &(OnuVoiceVlan.Priority), &(OnuVoiceVlan.VlanId));
	if( ret != VOS_OK )
		{
		if(EVENT_DEBUG == V2R1_ENABLE)
		sys_console_printf(" Get voice VLAN from %s%d/%d err\r\n",GetGFA6xxxTdmNameString(),(TdmSlot), SgIdx );
		return( RERROR );
		}
#if 0
	/* ���������BCM56300 ��ԭ�е�fpga ��̬����MAC  */
	DeleteVoiceMacFromSwAll( TdmSlot, SgIdx, OnuCount, OnuDevIdx_Array );

	/* �����������BCM56300 ��ԭ�е�TDM ����VLAN �Ͷ˿�. ע: ɾ��vlan ��ͬʱ��vlan �µĶ˿�Ҳ��ɾ��*/
	DeleteVoiceVlanByName(TdmSlot, SgIdx);

	/* ��TDM ���еĵ�ǰ���ݣ������µ�����VLAN */
	if( OnuVoiceVlan.Enable == V2R1_ENABLE )
		{
		VoiceVid = OnuVoiceVlan.VlanId;
		if( VoiceVid > V2R1_DEFAULT_VLAN)
			{
			ret = CreateVoiceVlan( TdmSlot, SgIdx, OnuVoiceVlan.VlanId );
			}
		else{
			/* ΪĬ��VLAN */
			VoiceVid = V2R1_DEFAULT_VLAN;
			ret = ROK;
			}
		}
	else{
		/* ΪĬ��VLAN */
		VoiceVid = V2R1_DEFAULT_VLAN;
		ret =  ROK;
		}
	if( ret !=  ROK ) 
		return( RERROR );

	/* ������VLAN ����Ӷ˿�*/
	if((OnuVoiceVlan.Enable == V2R1_ENABLE ) && (VoiceVid  > V2R1_DEFAULT_VLAN))
		AddPortToVoiceVlan(TdmSlot, SgIdx, VoiceVid );
	else{
		
		}
#endif

	/* ��TDM ���еĵ�ǰ�������ý�����ľ�̬MAC*/
	if( OnuVoiceVlan.Enable == V2R1_ENABLE )
		VoiceVid = OnuVoiceVlan.VlanId;
	else VoiceVid = V2R1_DEFAULT_VLAN;
	if( MN_Vlan_Check_exist( (unsigned long )VoiceVid ) == VOS_OK )
		AddVoiceMacToSwAll(TdmSlot, SgIdx, VoiceVid, OnuCount, OnuDevIdx_Array);
	else{
		/*if(EVENT_DEBUG == V2R1_ENABLE)*/
		sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",VoiceVid);
		}

	return( ROK );
}

/* �ָ�TDM ��������������������vlan ����������ONU ��̬MAC */
int  RestoreTdmSgBoardVoiceMacVlan( unsigned char TdmSlot)
{
	/*unsigned long fpga1 = 0,fpga2 = 0,fpga3 = 0;*/
	unsigned long fpgaIdx;

	if(SlotCardIsTdmSgBoard(TdmSlot) != ROK)
	/*if(__SYS_MODULE_TYPE__(TdmSlot ) != MODULE_E_GFA_TDM)*/
		{
		sys_console_printf(" slot %d is not %s card(restore config data from SW) \r\n", (TdmSlot), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_TDM_NAME_STR*/);
		return (RERROR );
		}

	for(fpgaIdx = 0; fpgaIdx < TDM_FPGA_MAX; fpgaIdx++)
		{
		 if(getTdmChipInserted ((TdmSlot-1), fpgaIdx) == TDM_FPGA_EXIST )
			{
			RestoreSWVoiceMacAndVlan(TdmSlot ,(fpgaIdx+1) );
			if(OLTMgmt.IsSystemStart == FALSE )
				ConfigAllOnuTdmVoiceService(TdmSlot ,(fpgaIdx+1));
			}
		}
	/*
	fpga1 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX1-1);
	fpga2 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX2-1);
	fpga3 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX3-1);

	if( fpga1 == 1 )
		{
		RestoreSWVoiceMacAndVlan(TdmSlot ,TDM_SG_INDEX1 );
		if(OLTMgmt.IsSystemStart == FALSE )
			ConfigAllOnuTdmVoiceService(TdmSlot ,TDM_SG_INDEX1 );
		}
	if( fpga2 == 1 )
		{
		RestoreSWVoiceMacAndVlan(TdmSlot ,TDM_SG_INDEX2 );
		if(OLTMgmt.IsSystemStart == FALSE )
			ConfigAllOnuTdmVoiceService(TdmSlot ,TDM_SG_INDEX2 );
		}
	if( fpga3 == 1 )
		{
		RestoreSWVoiceMacAndVlan(TdmSlot ,TDM_SG_INDEX3 );
		if(OLTMgmt.IsSystemStart == FALSE )
			ConfigAllOnuTdmVoiceService(TdmSlot ,TDM_SG_INDEX3 );
		}
	*/
	return( ROK );
}


int test1EthPortIsolate(unsigned int PonPortIdx1, unsigned int PonPortIdx2, unsigned int en_flag)
{
	bms_isolate_set(PonPortToSwPort[PonPortIdx1], PonPortToSwPort[PonPortIdx2], en_flag);
	return( ROK );
}

#ifdef  _TDM_BOARD_E1_LINK_OPERATION_
#endif
/********************************************************
*                                                                                                     *
*        ע: ���º�����ʹ����TDM ��E1 ����           *
*                                                                                                     *
*********************************************************/

/******************************************************************
 *    Function: int AddOnuE1LinkMacToSw(unsigned char TdmSlot, unsigned char SgIdx,  unsigned int OnuDevIdx, unsigned short int VoiceVid, unsigned short int LogicOnuIdx)
 *
 *    Param:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned char E1Idx -- ��һ��E1���е�˳���ţ�ȡֵ: 1 - MAX_E1_PER_FPGA
 *                unsigned int OnuDevIdx -- ONU �豸����
 *                unsigned short int E1_slotPort -- E1 ���Ӷ�Ӧ��ONU���E1����
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   ��	����оƬBCM56300 �����Ӿ�̬E1-link MAC (ֻ���ONU ��)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int AddOnuE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned char E1Idx, unsigned int OnuDevIdx, /*unsigned short int E1_slotPort,*/ unsigned short int E1LinkVid)
{
	unsigned int slot, port;
	unsigned char E1linkMac[BYTES_IN_MAC_ADDRESS]= {0};

	if(SlotCardIsTdmE1Board(TdmSlot) != ROK)
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("slot%d is not TDM-E1 board\r\n");
		return(VOS_ERROR);
		}
	if(TdmPortRangeCheck(fpgaIdx) != ROK)
		return(VOS_ERROR);
	
	if(TdmFpgaE1NumRangeCheck(E1Idx) != ROK)
		return(VOS_ERROR);

	if(MN_Vlan_Check_exist(E1LinkVid) != VOS_OK)
		{
		/*if(TDM_MGMT_DEBUG == V2R1_ENABLE)*/
		sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",E1LinkVid);
		return(VOS_ERROR);
		}
		
	VOS_MemCpy( E1linkMac, E1MacOnuFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	E1linkMac[5] += (E1Idx - 1);
	slot =GET_PONSLOT(OnuDevIdx)/* OnuDevIdx / 10000*/;
	port = GET_PONPORT(OnuDevIdx)/*(OnuDevIdx % 10000 ) / 1000*/;
	AddTdmMacToBCM56300(slot, port, &E1linkMac[0],E1LinkVid );
	/*
	VOS_MemCpy( E1linkMac, E1MacOltFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	E1linkMac[5] += (E1Idx - 1);
	AddTdmMacToBCM56300(TdmSlot, fpgaIdx, &E1linkMac[0],E1LinkVid );
	*/
	return( VOS_OK );
}

/* �ӽ���оƬ56300 ��ɾ��һ��E1 ���Ӷ�Ӧ��MAC ��ַ(ֻ���ONU ��) */
int DelOnuE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx, unsigned char E1Idx, unsigned OnuDevIdx/*, unsigned short int E1_slotPort*/ )
{
	unsigned char E1linkMac[BYTES_IN_MAC_ADDRESS]= {0};
	int E1LinkVid;
	int ret;

	if(SlotCardIsTdmE1Board(TdmSlot) != ROK)
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("slot%d is not TDM-E1 board\r\n");
		return(VOS_ERROR);
		}
	if(TdmPortRangeCheck(fpgaIdx) != ROK)
		return(VOS_ERROR);

	if(TdmFpgaE1NumRangeCheck(E1Idx) != ROK)
		return(VOS_ERROR);

	VOS_MemCpy( E1linkMac, E1MacOnuFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	E1linkMac[5] += (E1Idx - 1);
	
	ret = FindMacFromBCM56300(E1linkMac, &E1LinkVid );
	if( ret == ROK )
		bms_l2_delete(0,E1LinkVid,E1linkMac, 0);
	/*
	VOS_MemCpy( E1linkMac, E1MacOltFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	E1linkMac[5] += (E1Idx - 1);
	
	ret = FindMacFromBCM56300(E1linkMac, &E1LinkVid );
	if( ret == ROK )
		bms_l2_delete(0,E1LinkVid,E1linkMac, 0);
	*/
	return( ROK );
}

/* begin: added by jianght 20090804 */
extern ULONG IFM_GetIfindexByNameSlotPort( CHAR * szName , ULONG * pulState );
extern LONG IFM_config( ULONG ulIfindex, ULONG ulCode, VOID * pData, CHAR ** szErrInfo );
/* ����OLT�˿��Ƿ���Э�� 1=enable,2=disable*/
int setEthPortAutoNegotiation(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned char enable)
{
	char argv0[64];
	char ifName[30 + 1];
	unsigned long ulIfindex = 0, ulState = 0, ulAuto = 0;

	if (1 != enable && 2 != enable)
	{
		return VOS_ERROR;
	}

	VOS_MemZero( argv0, sizeof(argv0));
	VOS_MemZero( ifName, sizeof(ifName));
	/*
	sprintf(argv0, "%d/%d", TdmSlot, fpgaIdx);
	VOS_Snprintf( ifName, sizeof(ifName) - 1, "%s", argv0 );

	ulIfindex = IFM_GetIfindexByNameSlotPort( ifName, &ulState );
	*/
	ulIfindex = IFM_ETH_CREATE_INDEX(TdmSlot, fpgaIdx);
	if (VOS_NO == IFM_IndexIsExist(ulIfindex))
	{
		sys_console_printf("Can't find slot %d, port %d!\r\n", TdmSlot, fpgaIdx);
		return VOS_ERROR;
	}
	if (1 == enable)
	{
		ulAuto = AUTO_ENABLE;
	} 
	else
	{
		ulAuto = AUTO_DISABLE;
	}

	if (VOS_OK != IFM_config( ulIfindex, IFM_CONFIG_ETH_AUTONEGOTATION, &ulAuto, NULL ))
	{
		sys_console_printf("setEthPortAutoNegotiation(%d,%d,%d)   is Failed!\r\n", TdmSlot, fpgaIdx, enable);
		return VOS_ERROR;
	}

	return( VOS_OK );
}
/* end: added by jianght 20090804 */

/******************************************************************
 *    Function:  int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid)
 *
 *    Param:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
 *                unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
 *                unsigned short int E1LinkVid -- E1-link vlan-id, 1-4094
 *                            
 *    Desc:   �򽻻�оƬBCM56300 ������һƬfpga ��̬E1-link MAC (ֻ���tdm ��)
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
*********************************************************************/
int AddFpgaE1LinkMacToSw(unsigned char TdmSlot, unsigned char fpgaIdx, unsigned short int E1LinkVid)
{
	unsigned char E1linkMac[BYTES_IN_MAC_ADDRESS]= {0};
	unsigned char E1Idx;

	/* begin: added by jianght 20090804 */
	/* �رն˿���Э�� */
	/*if (VOS_ERROR == setEthPortAutoNegotiation(TdmSlot, fpgaIdx, 2))
	{
		sys_console_printf("set port=%d AN fail!\r\n", fpgaIdx);
	} */
	/* end: added by jianght 20090804 */

	if(SlotCardIsTdmE1Board(TdmSlot) != ROK)
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("slot%d is not TDM-E1 board\r\n");
		return(VOS_ERROR);
		}
	if(TdmPortRangeCheck(fpgaIdx) != ROK)
		return(VOS_ERROR);
	
	if(MN_Vlan_Check_exist(E1LinkVid) != VOS_OK)
		{
		/*if(TDM_MGMT_DEBUG == V2R1_ENABLE)*/
		sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",E1LinkVid);
		return(VOS_ERROR);
		}

	VOS_MemCpy( E1linkMac, E1MacOltFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	
	for(E1Idx =0; E1Idx < MAX_E1_PER_FPGA; E1Idx++)
		{
		E1linkMac[5] = E1Idx;
		AddTdmMacToBCM56300(TdmSlot, fpgaIdx, &E1linkMac[0],E1LinkVid );
		}
	
	return( VOS_OK );
}

/* �ӽ���оƬ56300 ��ɾ��һƬfpga E1 ���Ӷ�Ӧ��MAC ��ַ(ֻ���tdm ��) */
int DelFpgaE1LinkMacFromSw(unsigned char TdmSlot, unsigned fpgaIdx )
{
	unsigned char E1linkMac[BYTES_IN_MAC_ADDRESS]= {0};
	int E1LinkVid;
	int ret;
	unsigned char E1Idx;

	if(SlotCardIsTdmE1Board(TdmSlot) != ROK)
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("slot%d is not TDM-E1 board\r\n");
		return(VOS_ERROR);
		}
	if(TdmPortRangeCheck(fpgaIdx) != ROK)
		return(VOS_ERROR);

	VOS_MemCpy( E1linkMac, E1MacOltFpga, BYTES_IN_MAC_ADDRESS );
	E1linkMac[4]+= fpgaIdx-1;
	
	for(E1Idx=0; E1Idx < MAX_E1_PER_FPGA; E1Idx++)
		{
		E1linkMac[5] = E1Idx;
	
		ret = FindMacFromBCM56300(E1linkMac, &E1LinkVid );
		if( ret == ROK )
			bms_l2_delete(0,E1LinkVid,E1linkMac, 0);
		}

	return( ROK );
}

 /*  ���ڻָ�����*/
/******************************************************************
*  function type: int RestoreOneFpgaE1LinkToOnu(unsigned char TdmSlot, unsigned char fpgaIdx)
*
*  Parameter:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
*                     unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
*
*  Description :   ��ȡһ��TDM fpga �����õ�����E1 link����ͨ��gw-oam ��Ϣ���õ�ONU ��
*
*  Return:    ROK
*
*  Notes:  
*
*******************************************************************/
int  RestoreOneFpgaE1LinkToOnu(unsigned char TdmSlot, unsigned char fpgaIdx)
{
	AllE1Info pAllE1Info;
	FpgaE1Info   *pE1Info;
	ULONG  idxs[2], sw_idx[3];
	STATUS ret;
	int e1LinkCount,fpgaId;
	OAM_OnuE1Info pOAM_OnuE1Info;
	e1PortTable_t e1PortTable;
	
	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	if(SlotCardIsTdmE1Board(idxs[1])  != VOS_OK)
		return(VOS_ERROR);
	if(TdmPortRangeCheck(fpgaIdx) != VOS_OK)
		return(VOS_ERROR);
	
	ret = tdm_getAllE1Info(idxs, (fpgaIdx-1), &pAllE1Info );
	if(ret != VOS_OK)
		return(VOS_ERROR);

	if(pAllE1Info.type != (fpgaIdx-1)) return(VOS_ERROR);
	fpgaId=0;
/*	do{
		pE1Info =  &(pAllE1Info.fpgaE1Info[fpgaId]);
		fpgaId++;
		}while((pE1Info->fpgaIndex  != (fpgaIdx-1)) && (fpgaId <= TDM_FPGA_MAX));
	if(fpgaId > TDM_FPGA_MAX) return(VOS_ERROR);*/
	pE1Info = &(pAllE1Info.fpgaE1Info[fpgaId]);
/*
	if (1 == pAllE1Info.type)
	{
		sys_console_printf("\r\npAllE1Info:\r\n");

		for (i = 0; i < sizeof(AllE1Info); i++)
		{
			if ( !(i % 16) )
			{
				sys_console_printf("\r\n");
			}
			sys_console_printf( "%02x ", *(char *)((char *)&pAllE1Info + i) );
		}

		sys_console_printf("\r\npE1Info:\r\n");

		for (i = 0; i < sizeof(FpgaE1Info); i++)
		{
			if ( !(i % 16) )
			{
				sys_console_printf("\r\n");
			}
			sys_console_printf( "%02x ", *(char *)((char *)pE1Info + i) );
		}
	}
*/
	pOAM_OnuE1Info.MsgType = SET_ONU_E1_ALL_REQ;
	pOAM_OnuE1Info.Result = 0;
	pOAM_OnuE1Info.E1PortTotalCount = 1;
	pOAM_OnuE1Info.oam_OnuE1Info[0].reserve2 =0;
	pOAM_OnuE1Info.oam_OnuE1Info[0].reserve3 = 0;
		
	for(e1LinkCount =0; e1LinkCount < pE1Info->fpgaValidE1Count; e1LinkCount++)
		{
		pOAM_OnuE1Info.oam_OnuE1Info[0].E1SlotIdx = pE1Info->eachE1Info[e1LinkCount].onuE1SlotId;
		pOAM_OnuE1Info.oam_OnuE1Info[0].E1PortIdx = pE1Info->eachE1Info[e1LinkCount].onuE1Id+1;
		if(pE1Info->eachE1Info[e1LinkCount].eponE1LocalEnable == V2R1_ENABLE)
			pOAM_OnuE1Info.oam_OnuE1Info[0].E1Enable =  V2R1_ENABLE;
		else pOAM_OnuE1Info.oam_OnuE1Info[0].E1Enable =  V2R1_DISABLE -V2R1_DISABLE;
		VOS_MemCpy(pOAM_OnuE1Info.oam_OnuE1Info[0].DesMac, E1MacOltFpga,BYTES_IN_MAC_ADDRESS);
		VOS_MemCpy(pOAM_OnuE1Info.oam_OnuE1Info[0].SrcMac, E1MacOnuFpga,BYTES_IN_MAC_ADDRESS);
		pOAM_OnuE1Info.oam_OnuE1Info[0].DesMac[4] += pE1Info->fpgaIndex;
		pOAM_OnuE1Info.oam_OnuE1Info[0].DesMac[5] += pE1Info->eachE1Info[e1LinkCount].e1Index;
		pOAM_OnuE1Info.oam_OnuE1Info[0].SrcMac[4] += pE1Info->fpgaIndex;
		pOAM_OnuE1Info.oam_OnuE1Info[0].SrcMac[5] += pE1Info->eachE1Info[e1LinkCount].e1Index;
		if(pE1Info->eponVlanEnable == V2R1_ENABLE)
			pOAM_OnuE1Info.oam_OnuE1Info[0].VlanEable = V2R1_ENABLE;
		else pOAM_OnuE1Info.oam_OnuE1Info[0].VlanEable = V2R1_DISABLE -V2R1_DISABLE;
		pOAM_OnuE1Info.oam_OnuE1Info[0].VlanTag = pE1Info->eponVlanPri;
		pOAM_OnuE1Info.oam_OnuE1Info[0].VlanTag = (pOAM_OnuE1Info.oam_OnuE1Info[0].VlanTag << 13) + pE1Info->eponVlanId;
		pOAM_OnuE1Info.oam_OnuE1Info[0].ClockControl = pE1Info->eachE1Info[e1LinkCount].eponE1TxClock;
		pOAM_OnuE1Info.oam_OnuE1Info[0].LoopControl = 0/*pE1Info->eachE1Info[e1LinkCount].eponE1Loop*/;
		pOAM_OnuE1Info.oam_OnuE1Info[0].AlarmStat = 0;

		sw_idx[0] = (ULONG)pE1Info->eachE1Info[e1LinkCount].onuDevId;
		sw_idx[1] = (ULONG)pE1Info->eachE1Info[e1LinkCount].onuE1SlotId;
		sw_idx[2] = (ULONG)pE1Info->eachE1Info[e1LinkCount].onuE1Id;
		if (VOS_OK != sw_e1PortTable_get( sw_idx, &e1PortTable ))
			{
			sys_console_printf("RestoreOneFpgaE1LinkToOnu()::onuDevIdx=%d slot=%d port=%d    error!\r\n", sw_idx[0], sw_idx[1], sw_idx[2]);
			pOAM_OnuE1Info.oam_OnuE1Info[e1LinkCount].AlarmMask = 0;
			}
		else
			{
			pOAM_OnuE1Info.oam_OnuE1Info[e1LinkCount].AlarmMask = e1PortTable.eponE1PortAlarmMask/* ONU���mask������TDM��mask */;
			}
		SetOnuE1All(pE1Info->eachE1Info[e1LinkCount].onuDevId, &pOAM_OnuE1Info);
		}

	return(VOS_OK);
}

/******************************************************************
*  function type: int RestoreOneFpgaE1LinkMacAndVlanToSw(unsigned char TdmSlot, unsigned char fpgaIdx)
*
*  Parameter:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
*                     unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
*
*  Description :   �ڽ���оƬ�лָ�TDM ��ĳ��e1 ������E1 ������ص�MAC  ��VLAN
*
*  Return:    ROK
*
*  Notes:  
*
*******************************************************************/
int RestoreOneFpgaE1LinkMacAndVlanToSw(unsigned char TdmSlot, unsigned char fpgaIdx)
{
	AllE1Info pAllE1Info;
	FpgaE1Info   *pE1Info;
	ULONG  idxs[2];
	STATUS ret;
	int e1LinkCount,fpgaId;
	
	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	if(SlotCardIsTdmE1Board(idxs[1])  != ROK)
		return(RERROR);
	if(TdmPortRangeCheck(fpgaIdx) != ROK)
		return(RERROR);
	
	ret = tdm_getAllE1Info(idxs, (fpgaIdx-1), &pAllE1Info );
	if(ret != ROK)
		{
		sys_console_printf("1 Get E1-link Info error from tdm-fpga %d/%d\r\n", TdmSlot, fpgaIdx );
		return(RERROR);
		}
	if(pAllE1Info.type != (fpgaIdx-1)) 
		{
		sys_console_printf("2 Get E1-link Info error from tdm-fpga %d/%d\r\n", TdmSlot, fpgaIdx );
		return(RERROR);
		}
	fpgaId=0;
	do{
		pE1Info =  &(pAllE1Info.fpgaE1Info[fpgaId]);
		fpgaId++;
		}while((pE1Info->fpgaIndex  != (fpgaIdx-1)) && (fpgaId <= TDM_FPGA_MAX));
	if(fpgaId > TDM_FPGA_MAX)
		{
		sys_console_printf("3 Get E1-link Info error from tdm-fpga %d/%d\r\n", TdmSlot, fpgaIdx );
		return(RERROR);
		}
	
	for(e1LinkCount =0; e1LinkCount < pE1Info->fpgaValidE1Count; e1LinkCount++)
		{
		if(e1LinkCount >= MAX_E1_PER_FPGA) break;
		if(pE1Info->eponVlanEnable == V2R1_ENABLE)
			AddOnuE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx, (pE1Info->eachE1Info[e1LinkCount].e1Index+1),pE1Info->eachE1Info[e1LinkCount].onuDevId, pE1Info->eponVlanId );
		else 
			AddOnuE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx, (pE1Info->eachE1Info[e1LinkCount].e1Index+1),pE1Info->eachE1Info[e1LinkCount].onuDevId,V2R1_DEFAULT_VLAN);
		}
	
	if(pE1Info->eponVlanEnable == V2R1_ENABLE)
		AddFpgaE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx,pE1Info->eponVlanId );
	else 
		AddFpgaE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx,V2R1_DEFAULT_VLAN);
	
	return(ROK);
	
}

/******************************************************************
*  function type: int RestoreOnePonE1LinkMacAndVlanToSw(unsigned long slotIdx, unsigned long  portIdx)
*
*  Parameter:  unsigned long slotIdx -- pon �忨����; ȡֵPONCARD_FIRST - PONCARD_LAST
*                    unsigned long  portIdx --   pon �˿�����; ȡֵ: 1 - PONPORTPERCARD
*
*  Description :   �ڽ���оƬ�лָ�ĳ��PON������������E1 ���Ӷ�Ӧ�ľ�̬MAC  ��VLAN
*
*  Return:    ROK
*
*  Notes:  
*
*******************************************************************/
int RestoreOnePonE1LinkMacAndVlanToSw(unsigned long slotIdx, unsigned long  portIdx)
{
	AllE1Info pAllE1Info;
	FpgaE1Info   *pE1Info;
	ULONG  idxs[2];
	STATUS ret;
	int e1LinkCount,fpgaIdx;

	if(PonCardSlotPortCheckByVty(slotIdx, portIdx, NULL) != ROK) 
		return(RERROR);
	if(SlotCardIsPonBoard(slotIdx) != ROK)
		return(RERROR);
	
	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	if(SlotCardIsTdmE1Board(idxs[1])  != ROK)
		return(RERROR);

	/* ȡ�������õ�E1 ����*/
	ret = tdm_getAllE1Info(idxs, 0xff, &pAllE1Info );
	if(ret != ROK)
		return(RERROR);
	if(pAllE1Info.type != 0xff) return(RERROR);

	for(fpgaIdx=TDM_FPGA_MIN; fpgaIdx<=TDM_FPGA_MAX; fpgaIdx++)
		{
		pE1Info =  &(pAllE1Info.fpgaE1Info[fpgaIdx-1]);
		if( getTdmChipInserted((unsigned char)(get_gfa_e1_slotno() -1),(unsigned char)(pE1Info->fpgaIndex)) != TDM_FPGA_EXIST)
			continue;
			
		for(e1LinkCount =0; e1LinkCount < pE1Info->fpgaValidE1Count; e1LinkCount++)
			{
			if(e1LinkCount >= MAX_E1_PER_FPGA) break;
			if((slotIdx *10 + portIdx) == (pE1Info->eachE1Info[e1LinkCount].onuDevId /1000))
				{
				if(pE1Info->eponVlanEnable == V2R1_ENABLE)
					AddOnuE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx, (pE1Info->eachE1Info[e1LinkCount].e1Index+1),pE1Info->eachE1Info[e1LinkCount].onuDevId, pE1Info->eponVlanId );
				else 
					AddOnuE1LinkMacToSw(get_gfa_e1_slotno(),fpgaIdx, (pE1Info->eachE1Info[e1LinkCount].e1Index+1),pE1Info->eachE1Info[e1LinkCount].onuDevId,V2R1_DEFAULT_VLAN);
				}
			}
		}
	return(ROK);

}

/******************************************************************
*  function type: RestoreTdmE1BoradMacVlan(unsigned char TdmSlot)
*
*  Parameter:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
*
*  Description :   �ڽ���оƬ�лָ�TDM ������E1 ������ص�MAC  ��VLAN; ���ж��Ƿ�
*                        ��ϵͳ��������Ϊϵͳ����������ONU ����tdm-e1 ����OAM ��Ϣ��
*                       ��ֻ�ǵ�����������������ONU����tdm-e1 ����OAM ��Ϣ
*
*  Return:    ROK
*
*  Notes:  �˺���Ӧ���ڶ�����TDM ������E1 ���Ӷ�Ӧ�ľ�̬MAC ��ַ�Ļָ�,
*             ��TDM �忨����ʱ����
*
*******************************************************************/
int RestoreTdmE1BoradMacVlan(unsigned char TdmSlot)
{
	unsigned long fpgaIdx;
	unsigned char flag;

	if(SlotCardIsTdmE1Board(TdmSlot) != ROK)
		{
		sys_console_printf(" slot %d is not %s card(restore config data from SW) \r\n", (TdmSlot), GetGFA6xxxTdmNameString()/*MODULE_E_EPON3_TDM_NAME_STR*/);
		return (RERROR );
		}

	for(fpgaIdx = TDM_FPGA_MIN; fpgaIdx <= TDM_FPGA_MAX; fpgaIdx++)
		{
		flag=0;
		if(getTdmChipInserted ((TdmSlot-1), (fpgaIdx-1)) == TDM_FPGA_EXIST )
			{
			flag=1;
			RestoreOneFpgaE1LinkMacAndVlanToSw(TdmSlot , fpgaIdx );
			if(OLTMgmt.IsSystemStart == FALSE )
				RestoreOneFpgaE1LinkToOnu(TdmSlot , fpgaIdx);
			VOS_TaskDelay(1);
			}
		if(flag != 1 )
			sys_console_printf("tdm fpga %d/%d is not inserted\r\n", TdmSlot, fpgaIdx );
		}
	return( ROK );
}

/* ������������*/
/******************************************************************
*  function type: SetTdmE1LinkVlanEnable(unsigned char TdmSlot,unsigned char fpgaIdx, unsigned char Priority, unsigned short int VlanId)
*
*  Parameter:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
*				 unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
*                    unsigned char Priority -- ҵ�����ȼ���0��7
*                     unsigned short int E1LinkVid -- vid, 1-4094
*
*  Description :   ���ж�֮ǰ�Ƿ���ʹ��e1-link VLAN; 
*                       ֮����TDM-E1 ��������vlan ;
*                       �ڽ���оƬ�н�e1-link ��̬MAC �޸ĵ��µ�VLAN ��
*                       �����Ѵ��ڵ�����e1-link ONU �����޸�VLAN GW-OAM ��Ϣ
*
*  Return:    ROK
*
*  Notes:  �˺���Ӧ����ʹ��E1-link VLAN
*
*******************************************************************/
int SetTdmE1LinkVlanEnable(unsigned char TdmSlot,unsigned char fpgaIdx, unsigned char Priority, unsigned short int E1LinkVid)
{
	unsigned short int VlanId_old = 0;
	unsigned short int VlanEn_old = V2R1_DISABLE;
	ULONG idxs[3];
	e1VlanTable_row_entry pEntry;
	OAM_OnuE1Vlan pOAM_OnuE1Vlan;
	AllE1Info pAllE1Info;
	int Counter=0;
	FpgaE1Info *pFpgaE1Info;	
	int ret = E1_VM_OK;

	if(MN_Vlan_Check_exist((unsigned long )E1LinkVid ) != VOS_OK )
		{
		/*if(TDM_MGMT_DEBUG == V2R1_ENABLE)*/
		sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",E1LinkVid);
		return( E1_VM_ERR );
		}
		
	idxs[0]=1;
	idxs[1]= TdmSlot;
	idxs[2] = fpgaIdx-1;
	
	if((TdmCardSlotPortCheckByVty(TdmSlot,fpgaIdx,NULL) != ROK ) ||
		(Priority > 7) || (E1LinkVid < 1) || (E1LinkVid > 4094))
		ret = E1_VM_ERR;
	
	else if(  tdm_e1VlanTable_get(idxs, &pEntry) == VOS_ERROR )
		ret = E1_VM_ERR;
	
	else if( pEntry.eponVlanEnable== V2R1_ENABLE )
	{
		/* ��VID �����ȼ���ͬ���򷵻�*/
		if((pEntry.eponVlanPri== Priority) && (pEntry.eponVlanId == E1LinkVid ))
			return(E1_VM_VID_EXIST);
		VlanId_old = pEntry.eponVlanId;
	}

	if( ret != E1_VM_OK )
		return( E1_VM_ERR );

	VlanEn_old = pEntry.eponVlanEnable;

	/*  e1-link VLAN ��δʹ�ܣ���ʹ�ܣ���VID �����ȼ�ֻҪ��һ���仯������Ҫ����Ϣ�޸�*/
	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = fpgaIdx-1;

	/* ����TDM-E1 board */
	if ( VOS_OK != tdm_e1VlanTable_rowset( idxs, V2R1_ENABLE, Priority, E1LinkVid ) )
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("config e1-link vlan enable to TDM err\r\n");
		return E1_VM_ERR;
		}

	/* ��֮ǰ������E1-link, ����������E1-link ��GW-OAM��Ϣ������������vlan*/
	ret = tdm_getAllE1Info(idxs, (fpgaIdx-1), &pAllE1Info );
	if(ret != VOS_OK)
		return(E1_VM_ERR);

	if(pAllE1Info.type != (fpgaIdx-1)) return(E1_VM_ERR);
	do{
		pFpgaE1Info = (FpgaE1Info *)&pAllE1Info.fpgaE1Info[Counter];
		Counter++;
		}while((Counter <= TDM_FPGA_MAX) &&(pFpgaE1Info->fpgaIndex != (fpgaIdx-1)));
	if(Counter > TDM_FPGA_MAX) return(E1_VM_ERR);

	pOAM_OnuE1Vlan.MsgType = SET_ONU_E1_VLAN_REQ;
	pOAM_OnuE1Vlan.Result = 0;
	pOAM_OnuE1Vlan.E1PortTotalCount =1;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].reserve3=0;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].reserve2=0;

	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanEable = V2R1_ENABLE;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag = (unsigned short int)Priority;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag = (pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag << 13) + E1LinkVid;
	
	for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
		{
		pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].E1SlotIdx = pFpgaE1Info->eachE1Info[Counter].onuE1SlotId;
		pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].E1PortIdx = pFpgaE1Info->eachE1Info[Counter].onuE1Id + 1;
		SetOnuE1Vlan(pFpgaE1Info->eachE1Info[Counter].onuDevId, &pOAM_OnuE1Vlan);
		}

	/*TODO:
	    ��Ӵ�SG ���صĶ˿���ָ����VLAN
	*/
	if((( VlanEn_old == V2R1_ENABLE ) && ( VlanId_old  != E1LinkVid )) || ( VlanEn_old == V2R1_DISABLE ))
		{
		Counter =0;
		do{
			pFpgaE1Info = (FpgaE1Info *)&pAllE1Info.fpgaE1Info[Counter];
			Counter++;
			}while((Counter <= TDM_FPGA_MAX) &&(pFpgaE1Info->fpgaIndex != (fpgaIdx-1)));
		if(Counter > TDM_FPGA_MAX) return(E1_VM_ERR);

		for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
			{			
			DelOnuE1LinkMacFromSw(TdmSlot, fpgaIdx,(1+pFpgaE1Info->eachE1Info[Counter].e1Index), pFpgaE1Info->eachE1Info[Counter].onuDevId);
			}
		for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
			{			
			AddOnuE1LinkMacToSw(TdmSlot, fpgaIdx,(1+pFpgaE1Info->eachE1Info[Counter].e1Index), pFpgaE1Info->eachE1Info[Counter].onuDevId, E1LinkVid);
			}

		DelFpgaE1LinkMacFromSw(TdmSlot, fpgaIdx);
		AddFpgaE1LinkMacToSw(TdmSlot, fpgaIdx,E1LinkVid);
		}
	
	return ret;
}

/******************************************************************
*  function type: SetTdmE1LinkVlanDisable(unsigned char TdmSlot,unsigned char fpgaIdx)
*
*  Parameter:  unsigned char TdmSlot -- tdm �忨����; ȡֵTDMCARD_FIRST - TDMCARD_LAST
*				 unsigned char fpgaIdx -- tdm ������e1 ������; ȡֵ: TDM_FPGA_MIN - TDM_FPGA_MAX
*
*  Description :   ���ж�֮ǰ�Ƿ���ʹ��e1-link VLAN; 
*                       ֮����TDM-E1 ����ȡ��vlan ;
*                       �ڽ���оƬ�н�e1-link ��̬MAC �޸ĵ�Ĭ��VLAN ��
*                       �����Ѵ��ڵ�����e1-link ONU �����޸�VLAN GW-OAM ��Ϣ
*
*  Return:    ROK
*
*  Notes:  �˺���Ӧ����ȥʹ��E1-link VLAN
*
*******************************************************************/
int SetTdmE1LinkVlanDisable(unsigned char TdmSlot,unsigned char fpgaIdx)
{

	ULONG idxs[3];
	e1VlanTable_row_entry pEntry;
	OAM_OnuE1Vlan pOAM_OnuE1Vlan;
	AllE1Info pAllE1Info;
	int Counter=0;
	FpgaE1Info *pFpgaE1Info;	
	int ret = E1_VM_OK;

	idxs[0]=1;
	idxs[1]= TdmSlot;
	idxs[2] = fpgaIdx-1;
	
	if(TdmCardSlotPortCheckByVty(TdmSlot,fpgaIdx,NULL) != ROK )
		ret = E1_VM_ERR;
	
	else if(  tdm_e1VlanTable_get(idxs, &pEntry) == VOS_ERROR )
		ret = E1_VM_ERR;
	
	else if( pEntry.eponVlanEnable== V2R1_DISABLE )
	{
		/* û��ʹ��VLAN�� ����*/
		return(E1_VM_OK);
	}

	if( ret != E1_VM_OK )
		return( E1_VM_ERR );

	/*  e1-link VLAN ��δʹ�ܣ���ʹ�ܣ���VID �����ȼ�ֻҪ��һ���仯������Ҫ����Ϣ�޸�*/
	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = fpgaIdx-1;

	/* ����TDM-E1 board */
	if ( VOS_OK != tdm_e1VlanTable_rowset( idxs,V2R1_DISABLE, pEntry.eponVlanPri, pEntry.eponVlanId ) )
		{
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("config e1-link vlan disable to %s err\r\n", GetGFA6xxxTdmNameString());
		return E1_VM_ERR;
		}

	/* ��֮ǰ������E1-link, ����������E1-link ��GW-OAM��Ϣ��ȥʹ��vlan*/
	ret = tdm_getAllE1Info(idxs, (fpgaIdx-1), &pAllE1Info );
	if(ret != VOS_OK)
		return(E1_VM_ERR);

	if(pAllE1Info.type != (fpgaIdx-1)) return(E1_VM_ERR);
	do{
		pFpgaE1Info = (FpgaE1Info *)&pAllE1Info.fpgaE1Info[Counter];
		Counter++;
		}while((Counter <= TDM_FPGA_MAX) &&(pFpgaE1Info->fpgaIndex != (fpgaIdx-1)));
	if(Counter > TDM_FPGA_MAX) return(E1_VM_ERR);

	pOAM_OnuE1Vlan.MsgType = SET_ONU_E1_VLAN_REQ;
	pOAM_OnuE1Vlan.Result = 0;
	pOAM_OnuE1Vlan.E1PortTotalCount =1;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].reserve3=0;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].reserve2=0;

	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanEable = V2R1_DISABLE-V2R1_DISABLE;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag = pEntry.eponVlanPri;
	pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag = (pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].VlanTag << 13) + pEntry.eponVlanId;
		
	for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
		{
		pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].E1SlotIdx = pFpgaE1Info->eachE1Info[Counter].onuE1SlotId;
		pOAM_OnuE1Vlan.oam_OnuE1Vlan[0].E1PortIdx = pFpgaE1Info->eachE1Info[Counter].onuE1Id + 1;
		SetOnuE1Vlan(pFpgaE1Info->eachE1Info[Counter].onuDevId, &pOAM_OnuE1Vlan);
		}

	/*TODO:
	    ����TDM-FPGA ��e1 ���Ӷ�Ӧ�ľ�̬MAC ���õ�Ĭ��vlan ��
	*/
	Counter =0;
	do{
		pFpgaE1Info = (FpgaE1Info *)&pAllE1Info.fpgaE1Info[Counter];
		Counter++;
		}while((Counter <= TDM_FPGA_MAX) &&(pFpgaE1Info->fpgaIndex != (fpgaIdx-1)));
	if(Counter > TDM_FPGA_MAX) return(E1_VM_ERR);

	for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
		{			
		DelOnuE1LinkMacFromSw(TdmSlot, fpgaIdx,(pFpgaE1Info->eachE1Info[Counter].e1Index+1), pFpgaE1Info->eachE1Info[Counter].onuDevId);
		}
	for(Counter=0;Counter<pFpgaE1Info->fpgaValidE1Count; Counter++)
		{			
		AddOnuE1LinkMacToSw(TdmSlot, fpgaIdx,(pFpgaE1Info->eachE1Info[Counter].e1Index+1), pFpgaE1Info->eachE1Info[Counter].onuDevId, V2R1_DEFAULT_VLAN);
		}

	DelFpgaE1LinkMacFromSw(TdmSlot, fpgaIdx);
	AddFpgaE1LinkMacToSw(TdmSlot, fpgaIdx,V2R1_DEFAULT_VLAN);
			
	return ret;
}

#ifdef __TDM_BOARD_OPERATION__
#endif
/********************************************************
*                                                                                                     *
*        ע: ���º�����ʹ������������TDM ��    *
*                                                                                                     *
*********************************************************/

/* ����һ��TDM �˿�������ETH�˿�֮���Ƿ����*/
int SetEthPortTdmIsolate( unsigned char TdmSlot, unsigned char tdmPort, unsigned char en_flag )
{
	unsigned char i, j;
	int TdmPort;
	short int PonPortIdx;
	
	
	if(( en_flag != TDM_ETH_FILTER_PERMIT ) && ( en_flag != TDM_ETH_FILTER_DENY ))
		return( RERROR );

	TdmPort = GetPonPortIdxBySlot(TdmSlot, tdmPort);
	for(i=PONCARD_FIRST; i<=PONCARD_LAST; i++)
	{
		if((SlotCardIsTdmBoard(i) == ROK) && ( i == TdmSlot ))
		/*if(( __SYS_MODULE_TYPE__(i+1) == MODULE_E_GFA_TDM ) && ((i+1) == TdmSlot ))*/
		{
			continue;
		}
		else if(SlotCardIsSwBoard(i) == ROK)
		/*else if( __SYS_MODULE_TYPE__(i+1) == MODULE_E_GFA_SW )*/
		{
			continue;
		}
		else/* if( __SYS_MODULE_TYPE__(i+1) == MODULE_E_GFA_EPON )*/
		{
			for(j=1; j<=PONPORTPERCARD; j++)
			{
				PonPortIdx = GetPonPortIdxBySlot(i, j);
				bms_isolate_set(PonPortToSwPort[TdmPort], PonPortToSwPort[PonPortIdx], en_flag);
				bms_isolate_set(PonPortToSwPort[PonPortIdx], PonPortToSwPort[TdmPort], en_flag);
			}
		}
	}
	
	return( ROK);

}

/* ����TDM���Ӧ��ETH �˿�������PON�ڶ�Ӧ��ETH ��֮���Ƿ����
en_flag  0-ETH_FILTER_PERMIT ����
      	  1-ETH_FILTER_DENY  ����
*/
int SetEthPortTdmIsolateAll( unsigned char TdmSlot, unsigned char en_flag )
{
	/*unsigned long fpga1 = 0,fpga2 = 0,fpga3 = 0;*/
	unsigned long fpgaIdx;

	for(fpgaIdx = 0; fpgaIdx < TDM_FPGA_MAX; fpgaIdx++)
		{
		if(getTdmChipInserted(TdmSlot-1,fpgaIdx) == TDM_FPGA_EXIST)
			SetEthPortTdmIsolate(TdmSlot, (fpgaIdx+1),en_flag );
		}
	/*
	fpga1 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX1-1);
	fpga2 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX2-1);
	fpga3 = getTdmChipInserted ( TdmSlot-1 , TDM_SG_INDEX3-1);

	if( fpga1 == 1 )
		SetEthPortVoiceIsolate(TdmSlot, TDM_SG_INDEX1,en_flag );
	if( fpga2 == 1 )
		SetEthPortVoiceIsolate(TdmSlot, TDM_SG_INDEX2,en_flag );
	if( fpga3 == 1 )
		SetEthPortVoiceIsolate(TdmSlot, TDM_SG_INDEX3,en_flag );
	*/
	return( ROK );
}

/********************************************************
*	�ָ�һ��ETH �˿�������ONU ��(tdm) ��̬MAC 
*
*         �������������eth �˿��ڴ�down ��Ϊup ʱ����
*         �����eth �˿ڶ�Ӧ��PON ���µ�ONU ��(tdm)MAC ��ַ,
*         ��������eth �˿ڶ�Ӧ��TDM �˿��µ�(tdm) MAC ��ַ��
*         ����д�ص�BCM56300
*    �������:
*         unsigned char eth_slot -- ETH�˿����ڵĲ�λ����Χ:4-8
*         unsigned char eth_port -- ETH�˿ںţ���Χ:1-4
********************************************************/
int RestoreEthPortMacAndVlan(unsigned char eth_slot, unsigned char eth_port)
{
	unsigned int ret;
	unsigned long OnuCount = 0, CountAll=0;
	unsigned char count =0;
	VoiceVlanEnable OnuVoiceVlan;	
	unsigned long VoiceOnu[MAXONUPERPONNOLIMIT];
	unsigned char TdmSlot, FpgaIdx;
	unsigned long slot, port;
	unsigned short int i;
	unsigned char VoiceMac[BYTES_IN_MAC_ADDRESS];

	if(PonCardSlotPortCheckByVty(eth_slot, eth_port, NULL) != ROK)
	/*if(( eth_slot < (PON5+1)) ||(eth_slot > (PON1+1)) || (eth_port < 1 ) || ( eth_port > 4 ))*/
		return( RERROR );
	
	TdmSlot = get_gfa_tdm_slotno();

	if(SlotCardIsTdmSgBoard(TdmSlot) == ROK ) /* TDM -sg */
		{
		/* tdm ���Ӧ��ETH �˿�*/
		if( eth_slot == TdmSlot )
			{
			if( eth_port > TDM_FPGA_MAX ) return( RERROR );
			VOS_MemSet((void *) &OnuVoiceVlan, 0 , sizeof(VoiceVlanEnable));
			ret = GetTdmSGVoiceVlanEnable(eth_slot, eth_port, &(OnuVoiceVlan.Enable), &(OnuVoiceVlan.Priority), &(OnuVoiceVlan.VlanId));
			if( ret != VOS_OK )
				{
				if(EVENT_DEBUG == V2R1_ENABLE)
				sys_console_printf(" Get voice VLAN from %s%d/%d err\r\n",GetGFA6xxxTdmNameString(), eth_slot, eth_port );
				return( RERROR );
				}
			if(( OnuVoiceVlan.Enable == ENABLE ) && ( OnuVoiceVlan.VlanId > V2R1_DEFAULT_VLAN))
				{
				/*
				if((MN_Vlan_Check_exist( OnuVoiceVlan.VlanId )) != VOS_OK )
					{
					ret = VOS_ERROR;
					ret = CreateVoiceVlan( eth_slot, eth_port, OnuVoiceVlan.VlanId );
					AddPortToVoiceVlan(eth_slot, eth_port, OnuVoiceVlan.VlanId );
					}
				*/
				ret = MN_Vlan_Check_exist( OnuVoiceVlan.VlanId );
				if( ret == VOS_OK )
					{
					/* ����OLT fpga ��̬MAC */
					VOS_MemCpy( VoiceMac, VoiceMacOltFpga, BYTES_IN_MAC_ADDRESS );
					VoiceMac[5] = eth_port - 1;
					slot = eth_slot;
					port = eth_port;
					AddTdmMacToBCM56300(slot, port, &VoiceMac[0],OnuVoiceVlan.VlanId );
					}
				else{
					/*if(EVENT_DEBUG == V2R1_ENABLE)*/
					sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",OnuVoiceVlan.VlanId);
					}
				}
			else{
				/* ����OLT fpga ��̬MAC  to  Ĭ��VLAN  */
				VOS_MemCpy( VoiceMac, VoiceMacOltFpga, BYTES_IN_MAC_ADDRESS );
				VoiceMac[5] = eth_port - 1;
				slot = eth_slot;
				port = eth_port;
				AddTdmMacToBCM56300(slot, port, &VoiceMac[0], V2R1_DEFAULT_VLAN);
				}
			return( ROK );					
			}
		
		
		/* PON ���Ӧ��ETH �˿�*/
		for(FpgaIdx = TDM_FPGA_MIN; FpgaIdx <= TDM_FPGA_MAX; FpgaIdx++)
			{
			if( getTdmChipInserted ((TdmSlot-1),(FpgaIdx -1)) == TDM_FPGA_EXIST)
				{
				/* ȡSG ��ǰ���õ�����ONU */
				 OnuCount = 0;
				ret = GetAllOnuBelongToSG(TdmSlot, FpgaIdx, &OnuCount, OnuDevIdx_Array );
				if( ret != VOS_OK )
					{
					if(EVENT_DEBUG == V2R1_ENABLE)
					sys_console_printf(" Get voice ONU from %s%d/%d err\r\n",GetGFA6xxxTdmNameString(),(TdmSlot), FpgaIdx );
					continue;
					}
				
				if( OnuCount != 0 )
					{
					count = 0;
					CountAll = 0;
					for( i=0; i < OnuCount; i++)
						{
						/*slot = (OnuDevIdx_Array[i] >> 8 ) / 10000;
						port = ((OnuDevIdx_Array[i] >> 8 ) % 10000) /1000;*/
                                          slot =GET_PONSLOT((OnuDevIdx_Array[i] >> 8 ) );
						port =GET_PONPORT((OnuDevIdx_Array[i] >> 8 ));
                                            
						if((slot == eth_slot ) && (port == eth_port ) && ( CountAll < (MAXONUPERPON-1)))
							{
							VoiceOnu[count] = OnuDevIdx_Array[i];
							count++;
							CountAll ++;
							}
						}
					
					/* ȡSG �µ�ǰ������VLAN ����*/
					if( count != 0 )
						{
						VOS_MemSet((void *) &OnuVoiceVlan, 0, sizeof(VoiceVlanEnable));
						ret = GetTdmSGVoiceVlanEnable(TdmSlot, FpgaIdx, &(OnuVoiceVlan.Enable), &(OnuVoiceVlan.Priority), &(OnuVoiceVlan.VlanId));
						if( ret != VOS_OK )
							{
							if(EVENT_DEBUG == V2R1_ENABLE)
							sys_console_printf(" Get voice VLAN from %s%d/%d err\r\n",GetGFA6xxxTdmNameString(),(TdmSlot), FpgaIdx );
							continue;
							}
						
						if(( OnuVoiceVlan.Enable == V2R1_ENABLE ) && ( OnuVoiceVlan.VlanId > V2R1_DEFAULT_VLAN))
							{
							/*
							if((MN_Vlan_Check_exist( OnuVoiceVlan.VlanId )) != VOS_OK )
								{
								ret = VOS_ERROR;
								ret = CreateVoiceVlan( TdmSlot, SgIdx, OnuVoiceVlan.VlanId );
								AddPortToVoiceVlan(TdmSlot, SgIdx, OnuVoiceVlan.VlanId );
								}
							*/
							ret = MN_Vlan_Check_exist( OnuVoiceVlan.VlanId );
							if( ret == VOS_OK )
								AddVoiceMacToSwAll(TdmSlot, FpgaIdx, OnuVoiceVlan.VlanId , count, VoiceOnu);
							else{
								/*if(EVENT_DEBUG == V2R1_ENABLE)*/
								sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",OnuVoiceVlan.VlanId);
								}
							}
						else
							AddVoiceMacToSwAll(TdmSlot, FpgaIdx, V2R1_DEFAULT_VLAN , count, VoiceOnu);
						}

					}
				}			
			}
		}
	
	else if(SlotCardIsTdmE1Board(TdmSlot) == ROK)  /* tdm-e1 */
		{
		ULONG idxs[3];
		e1VlanTable_row_entry pEntry;
		
		idxs[0]=1;
		idxs[1]= TdmSlot;
		idxs[2] = eth_port -1;
	
		/* tdm ���Ӧ��ETH �˿�*/
		if( eth_slot == TdmSlot )
			{
			if( eth_port > TDM_FPGA_MAX ) return( RERROR );
			if(tdm_e1VlanTable_get(idxs, &pEntry) == VOS_ERROR) return(RERROR);
			if( pEntry.eponVlanEnable== V2R1_ENABLE )
				{
				AddFpgaE1LinkMacToSw(TdmSlot, eth_port, pEntry.eponVlanId);
				}
			else{
				AddFpgaE1LinkMacToSw(TdmSlot, eth_port,V2R1_DEFAULT_VLAN);
				}
			}

		/* PON ���Ӧ��ETH �˿�*/
		else{
			RestoreOnePonE1LinkMacAndVlanToSw(eth_slot, eth_port);
			}
		}

	return( ROK );
}

/************************************
 �ָ�TDM ������(tdm) MAC ��VLAN 
*************************************/
int  RestoreTdmBoardMacAndVlan(unsigned char CardIndex)
{
	if(SlotCardIsTdmSgBoard(CardIndex) == ROK)
		return(RestoreTdmSgBoardVoiceMacVlan(CardIndex));
	else if(SlotCardIsTdmE1Board(CardIndex) == ROK)
		return(RestoreTdmE1BoradMacVlan(CardIndex));

	return(RERROR);
}

/* ɾ������оƬ������(tdm) MAC (����TDM�ӿ�������ONU, ����TDM���Ȱβ�) */
int DeleteTdmMacFromSwForTDMPull(unsigned char TdmSlot)
{
	int i;
	int index_min, index_max;
	int ret;
	cli_l2_addr_t  L2_Addr;
	/*int vid;*/

	if(bms_l2_idx_get( 0, &index_min, &index_max ) != ROK) return(RERROR);
	
	for(i = index_min; i<= index_max; i++)
		{
		ret = bms_l2_entry_get(0,i,&L2_Addr, 1);
		if( ret == 0 )
			{
			/*
			if( VOS_MemCmp(VoiceMacOltFpga, L2_Addr.mac, (BYTES_IN_MAC_ADDRESS-1) ) == 0)
				{
				bms_l2_delete(0,L2_Addr.vid,L2_Addr.mac, 0);
				continue;
				}
			*/
			if( VOS_MemCmp(TdmStyleMac, L2_Addr.mac, (BYTES_IN_MAC_ADDRESS-2) ) == 0)
				{
				if(*(unsigned short int *)&(L2_Addr.mac[4]) <= TdmMaxMac )
					bms_l2_delete(0,L2_Addr.vid,L2_Addr.mac, 0);
				}
			}

		}
	return( ROK );

}

#endif

#ifdef __cplusplus
}
#endif


