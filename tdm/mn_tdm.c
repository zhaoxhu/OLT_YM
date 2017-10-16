/***************************************************************
*
*						Module Name:  mn_tdm.c
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
**   modified by chenfj 2008-7-30
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
#include  "onu/ExtBoardType.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"
#include "TdmService.h"
#include "mn_tdm.h"
#include "gwEponSys.h"
#include "lib_gwEponMib.h"

/*
#define	ENABLE_POTS_PORT	1
#define	DISABLE_POTS_PORT	0

#define	VOICE_VLAN_ENABLE	  1
#define	VOICE_VLAN_DISABLE	2

#define	MIN_SG_IDX	1
#define	MAX_SG_IDX	 3

#define  MAX_ONU_POTS  32 
#define  MIN_ONU_POTS   1
*/


enum{
	OAM_CODE_SET_VLAN=102,
	OAM_CODE_GET_VLAN,
	OAM_CODE_SET_POTS,
	OAM_CODE_GET_POTS,
	OAM_CODE_GET_POTS_HANGOFF,
	OAM_CODE_SET_LOOP,
	OAM_CODE_GET_LOOP
};


/* added by chenfj */
typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	ULONG devIdx; 
}__attribute__((packed)) OnuPotsLinkAll_ReqPdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	ULONG devIdx; 
	UCHAR potslist[ONU_POTS_NUM_MAX];
}__attribute__((packed)) OnuPotsLinkAll_RspPdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR SgIdx; 
	unsigned short int  OnuCount;
	ULONG OnuIdx[MAXONUPERSG];
}__attribute__((packed)) SgOnuTableAll_RspPdu;

typedef struct{
	UCHAR reserved1;
	UCHAR reserved2;
	UCHAR sgIndex;
}PACKED get_onu_pdu;

typedef struct{
	UCHAR reserved1;
	UCHAR reserved2;
	ULONG onuDevIdx;
}PACKED get_pots_pdu;


extern UCHAR g_tdmDebugFlag;
/*
#define PDU_CHECK( x ) \
	if( x == NULL )\
	{\
		if( g_tdmDebugFlag & TDM_DEBUG_INFO )\
		{\
			sys_console_printf( "\r\nTDM sending queue is full, abort TDM request\r\n" );\
			sys_console_printf( "file:%s, line:%d\r\n",__FILE__, __LINE__  );\
		}\
		return VOS_ERROR;\
	}
*/



/* 输入参数: idxs[0]-SG索引，idxs[1]-ONU设备索引 */

/*  deleted by chenfj 2008-7-30
	这个函数怎么创建了一行后, 立马又删除了?
	查了一下, 这个函数没有被调用, 所以注释掉吧
	*/
#if 0
STATUS mn_tdmOnuAdd( ULONG *idxs )
{
	int rc;
	OnuVoiceEnable voiceEn;
	USHORT lport = 0;
	if( (rc = tdm_tdmOnuTable_set(LEAF_EPONTDMONUROWSTATUS, idxs, 4, &lport)) == VOS_OK )
	{
		voiceEn.VoiceEnable = 1;
		voiceEn.TdmSlot = get_gfa_tdm_slotno();
		voiceEn.SgIdx = idxs[0];
		voiceEn.LogicOnuIdx = lport;
		rc = SetOnuVoiceEnable(idxs[1], &voiceEn);
		if( rc != V2R1_TDM_ERROR )	/* modified by xieshl 20080411 */
			rc = VOS_OK;

		tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_DESTROY, &lport );
	}
	return rc;
}
#endif
STATUS mn_tdmOnuDel( ULONG *idxs )
{
	int rc;
	OnuVoiceEnable voiceEn;
	USHORT lport = 0;
	if( (rc = tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_DESTROY, &lport)) == VOS_OK )
	{
		voiceEn.VoiceEnable = 2;
		voiceEn.TdmSlot = get_gfa_sg_slotno();
		voiceEn.SgIdx = idxs[0];
		voiceEn.LogicOnuIdx = lport;
		rc = SetOnuVoiceEnable(idxs[1], &voiceEn);
		if( rc != V2R1_TDM_ERROR )	/* modified by xieshl 20080411 */
			rc = VOS_OK;

		onuTdmServiceAlarmStatus_update(idxs[1]);
	}
	return rc;
}

/* chenfj 2008-7-30
     这个函数也没有被调用
     */
STATUS	mn_tdmOnuRowStatusSet( ULONG idxs[2] , ULONG setval )
{
	STATUS rc = VOS_ERROR;
	USHORT logicOnu;
	if( setval == RS_CREATEANDGO ||setval == RS_CREATEANDWAIT )
		rc = AddOnuToSG(get_gfa_sg_slotno(), idxs[0], idxs[1], &logicOnu);
	else if( setval == RS_DESTROY )
		rc = mn_tdmOnuDel( idxs );
	else
		rc = VOS_OK;

	return rc;
}


/*******************************************************************************************
函数名：	mn_tdmPotsLinkRowStatusSet
功能：	设置POTS连接并使能ONU上的POTS端口
输入：	sgIndex -- 信令网关的编号
        		sgPortIndex -- 逻辑端口号
        		satus --行变量的状态值
输出：	无
返回值：VOS_OK -- 操作正常 VOS_ERROR -- 操作异常
*******************************************************************************************/
STATUS	mn_tdmPotsLinkRowStatusSet( ULONG sgIndex, ULONG sgPortIndex, ULONG status )
{

	/*potslinktable_row_entry entry;*/
	STATUS rc = VOS_ERROR;

	ULONG idxs[2] = { 0 };

	idxs[0] = sgIndex;
	idxs[1] = sgPortIndex;

	switch( status )
	{
		case RS_CREATEANDWAIT: 
		case RS_CREATEANDGO:
			/* modified by xieshl 20080602, 解决DA网管批量配置语音连接时响应时间太长问题,
			    添加空行的操作越应超前处理，否则在检查数值有效性时会导致大量无谓的
			    GET操作*/
			/*rc = tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&status );*/
			rc = VOS_OK;
			break;
		case RS_ACTIVE:
			if( VOS_OK == tdm_potsLinkTable_set(LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&status ) )
			{
				rc = VOS_OK;
				/*rc =  ( VOS_OK == SetOnuPotsEnable( entry.devIdx, ENABLE_POTS_PORT, 1, (1<<(entry.potsIdx-1)) ) )?VOS_OK:VOS_ERROR;*/
			}
			break;
		case RS_DESTROY:
				rc = deleteOnuPotsLinkFromSgByLogicPort( idxs[0], idxs[1] );
			break;
	}

	return rc;
}

/*解决GT861类型的onu添加语音连接时，onu板卡范围是2-5，对应的pots口范围是1-32，2008-7-1*/
STATUS mn_tdmPotsLinkRowSet( ULONG sgIndex, ULONG sgPortIndex, ULONG ulonuDev,ULONG ulonuBoard,ULONG ulonuPots,ULONG ulphoneCode,UCHAR ulLinkDesc[32],ulong  ulpotslinkStatus)
{
	ulong_t  status;
	/*potslinktable_row_entry entry;*/
	STATUS rc = VOS_ERROR;

	ULONG idxs[2] = { 0 };

	idxs[0] = sgIndex;
	idxs[1] = sgPortIndex;
	status =ulpotslinkStatus;

	switch( status )
	{
		case RS_CREATEANDWAIT: 
		case RS_CREATEANDGO:
			/* modified by xieshl 20080602, 解决DA网管批量配置语音连接时响应时间太长问题,
			    添加空行的操作越应超前处理，否则在检查数值有效性时会导致大量无谓的
			    GET操作*/
			/*rc = tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&status );*/
			rc = VOS_OK;
			break;
		case RS_ACTIVE:
			if( VOS_OK == tdm_potsLinkTable_rowset( idxs , ulonuDev,  ulonuBoard,  ulonuPots ,  ulphoneCode,  ulLinkDesc,  (uchar)ulpotslinkStatus))
			{
				rc = VOS_OK;
				/*rc =  ( VOS_OK == SetOnuPotsEnable( entry.devIdx, ENABLE_POTS_PORT, 1, (1<<(entry.potsIdx-1)) ) )?VOS_OK:VOS_ERROR;*/
			}
			break;
		case RS_DESTROY:
				rc = deleteOnuPotsLinkFromSgByLogicPort( idxs[0], idxs[1] );
			break;
	}

	return rc;
}

/*******************************************************************************************
函数名：mn_tdmSgVoiceVlanSet
功能：  设置语音VLAN使能
输入：  sgIndex -- 信令网关的编号
		setval -- 操作值 1：使能 2：去使能
输出：  无
返回值：VOS_OK:正常 VOS_ERROR:异常
***************************************************************************************************/
STATUS	mn_tdmSgVoiceVlanSet( ULONG sgIndex, ULONG setval )
{
	
	ULONG onuIdxs[MAXONUPERSG];
	STATUS	rc = VOS_ERROR;
	UINT i=0;
	ULONG onuNum = 0;

	VoiceVlanEnable vven = { 0 };
	sgtable_row_entry entry = { 0 };
	
	if( GetAllOnuBelongToSG( get_gfa_sg_slotno(), sgIndex, &onuNum, onuIdxs ) == VOS_OK )
	{

		if( tdm_sgTable_set( LEAF_EPONSGVLANENABLE, sgIndex, setval ) == VOS_OK )
		{
			/*TODO:
				操作56300上的VLAN(添加/删除)
			*/
			tdm_sgTable_get( sgIndex, &entry );
			
			for( ; i<onuNum; i++ )
			{
				vven.Enable = entry.vlanen;
				vven.Priority = entry.vlanpri;
				vven.VlanId = entry.vlanid;
				SetOnuVoiceVlan( onuIdxs[i]>>8, &vven );
			}

			rc = VOS_OK;

		}
		
	}
	
	return rc;
}


#ifdef TDM_VM_API
#endif
/*语音管理API实现*/

BOOL onuIsOnLine( ULONG onuIdx )
{
	
	ULONG onuStatus;
	
	bool ret = FALSE;
	
	if((getDeviceOperStatus( onuIdx, &onuStatus ) == VOS_OK) && (onuStatus == ONU_OPER_STATUS_UP ))
		ret = TRUE;
	
	return ret;
		
}

int SetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char Priority, unsigned short int VlanId)
{
	sgtable_row_entry entry = {0};
	ULONG onuNum=0;
	ULONG devIdxs[MAXONUPERSG] = {0};
	unsigned short int VlanId_old = 0;
	unsigned short int VlanEn_old = V2R1_DISABLE;
	
	int ret = TDM_VM_OK;

	if(MN_Vlan_Check_exist((unsigned long )VlanId ) != VOS_OK )
		{
		sys_console_printf("\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n",VlanId);
		return( TDM_VM_ERR );
		}
	
	if((TdmCardSlotPortCheckByVty(TdmSlot,SgIdx,NULL) != ROK ) ||
	/*if( SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 ||*/
		Priority > 7 || VlanId < 1 || VlanId > 4094 )
		ret = TDM_VM_ERR;
	
	else if( tdm_sgTable_get( SgIdx, &entry ) == VOS_ERROR )
		ret = TDM_VM_ERR;
	
	else if( entry.vlanen == V2R1_ENABLE )
	{
		VlanId_old = entry.vlanid ;
		/* 若VID 及优先级相同，则返回*/
		if(/*( entry.sgifIdx == SgIdx) && */(entry.vlanpri == Priority) && (entry.vlanid == VlanId ))
			return(TDM_VM_VID_EXIT);
	}

	if( ret != TDM_VM_OK )
		return( TDM_VM_ERR );

	VlanEn_old = entry.vlanen;

	/*  语音VLAN 或未使能，或使能，但VID 及优先级只要有一个变化，都需要发消息修改*/
	{
		entry.vlanid = VlanId;
		entry.vlanpri = Priority;
		entry.vlanen = V2R1_ENABLE;
		if( tdm_sgTable_rowset( SgIdx, &entry.mastere1 ) == VOS_ERROR )
			return(TDM_VM_ERR);
	}
	
	if( GetAllOnuBelongToSG(TdmSlot, SgIdx, &onuNum, devIdxs  ) == VOS_OK)
	{
		int i=0;
		VoiceVlanEnable vve;
		for(; i<onuNum; i++ )
		{
			if(onuIsOnLine(devIdxs[i]>>8) != TRUE ) continue;
			vve.Enable = V2R1_ENABLE;
			vve.Priority = Priority;
			vve.VlanId = VlanId;
			if( SetOnuVoiceVlan( devIdxs[i]>>8, &vve ) != V2R1_TDM_OK )
				SetOnuVoiceVlan( devIdxs[i]>>8, &vve );
		}
	}

	/*TODO:
	    添加此SG 网关的端口至指定的VLAN
	*/
	
	if((( VlanEn_old == V2R1_ENABLE ) && ( VlanId_old  != VlanId )) || ( VlanEn_old == V2R1_DISABLE ))
		{
		/*
		DeleteVoiceMacFromSwAll(TdmSlot,SgIdx,onuNum,devIdxs);
		if(VlanId_old > V2R1_DEFAULT_VLAN )
			DeleteVoiceVlan(TdmSlot, SgIdx, VlanId_old);
		
		if( VlanId > V2R1_DEFAULT_VLAN )
			{
			CreateVoiceVlan(TdmSlot, SgIdx,  VlanId);
			AddPortToVoiceVlan(TdmSlot, SgIdx, VlanId );
			}
		*/
		DeleteVoiceMacFromSwAll(TdmSlot,SgIdx,onuNum,devIdxs);
		AddVoiceMacToSwAll(TdmSlot, SgIdx, VlanId,onuNum,devIdxs);
		}
	
	return ret;
}

int SetTdmSGVoiceVlanDisable(unsigned char TdmSlot,unsigned char SgIdx)
{
	sgtable_row_entry entry = { 0 };
	int ret = TDM_VM_OK;

	ULONG onuNum=0;
	ULONG devIdxs[MAXONUPERSG] = {0};
	/*
	unsigned short int VlanId_old = 0;
	unsigned short int VlanEn_old = V2R1_DISABLE;
	*/
	
	if(TdmCardSlotPortCheckByVty(TdmSlot,SgIdx,NULL) != ROK ) 
	/*if( SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 )*/
		ret = TDM_VM_ERR;
	
	else if( tdm_sgTable_get( SgIdx, &entry ) == VOS_ERROR )
		ret = TDM_VM_ERR;
	
	else if( entry.vlanen == V2R1_ENABLE )
	{
		if( tdm_sgTable_set(LEAF_EPONSGVLANENABLE, SgIdx, V2R1_DISABLE ) == VOS_ERROR )
			return(TDM_VM_ERR );
		
		if( GetAllOnuBelongToSG(TdmSlot, SgIdx, &onuNum, devIdxs  ) == VOS_OK)
		{
			int i=0;
			VoiceVlanEnable vve;
			for(; i<onuNum; i++ )
			{
				if(onuIsOnLine(devIdxs[i]>>8) != TRUE ) continue;
				vve.Enable = DISABLE;
				vve.Priority = entry.vlanpri;
				vve.VlanId = entry.vlanid;
				if( SetOnuVoiceVlan( devIdxs[i]>>8, &vve ) != V2R1_TDM_OK )
					SetOnuVoiceVlan(devIdxs[i]>>8, &vve);
			}
		}
		
		/*TODO:
		    删除此SG 网关的端口至指定的VLAN
		*/
		DeleteVoiceMacFromSwAll(TdmSlot, SgIdx, onuNum,devIdxs);
		/*
		DeleteVoiceVlanByName(TdmSlot, SgIdx);
		*/
		
		AddVoiceMacToSwAll(TdmSlot, SgIdx, V2R1_DEFAULT_VLAN,onuNum,devIdxs);
				
	}

	if( ret != TDM_VM_OK )
		return( TDM_VM_ERR ); 	
		
	return ret;
}

int GetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char *enableFlag, 
									unsigned char *Priority, unsigned short int *VlanId)
{
	sgtable_row_entry entry = {0};
	int ret = TDM_VM_OK;
	
	if(TdmCardSlotPortCheckByVty(TdmSlot,SgIdx,NULL) != ROK ) 
	/*if( SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 )*/
		ret = TDM_VM_ERR;
	else if( tdm_sgTable_get( SgIdx, &entry ) == VOS_ERROR )
		ret = TDM_VM_ERR;
	else
	{
		*enableFlag = entry.vlanen;
		*Priority = entry.vlanpri;
		*VlanId = entry.vlanid;
	}
	
	
	return ret;
}

#ifdef	ONUTOSG
int SetOnuVoiceVlanEnable(unsigned long OnuDeviceIdx,unsigned short int vid, unsigned char priority)
{
	short int ponid, onuid, vtag;
	char msgbuf[80]="";
	int ret = TDM_VM_OK;
	
	if( vid<1 || vid>4094 || priority > 7 )
		ret = TDM_VM_ERR;
			
	else if( parseOnuIndexFromDevIdx( OnuDeviceIdx, &ponid, &onuid ) == -1 )
		ret = TDM_VM_ERR;
	
	else
	{
		
		/*设置VLAN标签*/
		msgbuf[0] = OAM_CODE_SET_VLAN;
		msgbuf[1] = 0;
		msgbuf[1]= ENABLE;
		vtag = priority;
		*(short int*)(msgbuf+3) = (vtag<<13)|vid;
		
		if( !onuIsOnLine( OnuDeviceIdx ) )
			ret = TDM_VM_ERR;
			
		else if( EQU_SendMsgToOnu( ponid, onuid, OAM_CODE_SET_VLAN, msgbuf, 5 ) == VOS_ERROR )
			ret = TDM_VM_ERR;
	}

			
	return ret;	
}

int SetOnuVoiceVlanDisable(unsigned long OnuDeviceIdx /*,unsigned short int vid, unsigned char priority*/)
{
	short int ponid, onuid, vtag;
	char msgbuf[80]="";
	int ret = TDM_VM_OK;
	
	if( vid<1 || vid>4094 || priority > 7 )
		ret = TDM_VM_ERR;
			
	else if( parseOnuIndexFromDevIdx( OnuDeviceIdx, &ponid, &onuid ) == -1 )
		ret = TDM_VM_ERR;
	
	else
	{
		
		/*设置VLAN标签*/
		msgbuf[0] = OAM_CODE_SET_VLAN;
		msgbuf[1] = 0;
		msgbuf[2]= DISABLE;
		
		/*
		vtag = priority;
		*(short int*)(msgbuf+3) = (vtag<<13)|vid;
		*/
		if( !onuIsOnLine( OnuDeviceIdx ) )
			ret = TDM_VM_ERR;
			
		else if( EQU_SendMsgToOnu( ponid, onuid, OAM_CODE_SET_VLAN, msgbuf, 5 ) == VOS_ERROR )
			ret = TDM_VM_ERR;
	}

			
	return ret;	
}


int setEponSgVlanEnable( USHORT sgIfIdx, UCHAR enable )
{
	sgtable_row_entry entry;
	int ret = TDM_VM_OK;
	
	if((TdmPortRangeCheck(sgIfIdx) != ROK )|| enable != V2R1_ENABLE || enable != V2R1_DISABLE )
	/*if( sgIfIdx < MIN_SG_IDX || sgIfIdx > MAX_SG_IDX || enable != 1 || enable != 2 )*/
		ret = TDM_VM_ERR;
		
	else if( tdm_sgTable_get( sgIfIdx, &entry ) == VOS_OK )
	{
		if( tdm_sgTable_set( LEAF_EPONSGVLANENABLE, sgIfIdx, enable ) == VOS_ERROR )
			reports( "setEponSgVlanEnable", "tdm_sgTable_set call fail!!!" );
		else
		{
			tdmonutable_row_entry onuEntry;
			ULONG idxs[2] = { 0, 0 };
			
			/*设置交换芯片中的VLAN信息*/
			SetSWVoiceVlanEnable( 6, sgIfIdx, entry.vlanid, entry.vlanpri );
			
			/*找到已经配置的语音ONU，将连接ONU的PON端口对应的交换端口加入VLAN
			  如果ONU在线，下发OAM使能ONU上的语音VLAN
			*/
			while( tdm_tdmOnuTable_getNext( idxs, &onuEntry ) == VOS_OK )
			{
				addPonPortToSwVlan( onuEntry.devIdx );
				
				if( onuIsOnLine( onuEntry.devIdx ) )
				{
					/*
					if( enable == 1 )
						SetOnuVoiceVlanEnable( onuEntry.devIdx, entry.vlanid, entry.vlanpri );
					else
						SetOnuVoiceVlanDisable( onuEntry.devIdx );
						*/
					VoiceVlanEnable vven;
					vven.Enable = enable;
					vven.Priority = entry.vlanpri;
					vven.VlanId = entry.vlanid;
					
					if ( SetOnuVoiceVlan( onuEntry.devIdx, &vven ) == VOS_OK )
						ret = TDM_VM_OK;
					else
						ret = TDM_VM_ERR;
				}

				idxs[0] = onuEntry.sgIdx;
				idxs[1] = onuEntry.devIdx;
					
			}
		}
	}
	
	return ret;
}

#endif

int AddNewOnuToSg(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu)
{

	ULONG idxs[2] = {0};
	USHORT port = 0;

	int ret = VOS_ERROR;

	idxs[0] = SgIdx;
	idxs[1] = OnuDeviceIdx;

	if( tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_CREATEANDGO, &port ) == VOS_OK )
	{

		sgtable_row_entry sgEntry;
		/*激活刚添加的表项
		tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_ACTIVE, &port );*/

		if( onuIsOnLine( OnuDeviceIdx ) )
		{
			ConfigOnuTdmVoiceService( OnuDeviceIdx );
		}

		/*todo: 调用平台API，添加语音MAC地址*/
		/*VOS_ASSERT( 0 );*/
		if( tdm_sgTable_get( SgIdx, &sgEntry ) == VOS_OK )
		{
			if( sgEntry.vlanen == V2R1_ENABLE )
				AddVoiceMacToSw( TdmSlot, SgIdx, OnuDeviceIdx, sgEntry.vlanid, port );
			else 
				AddVoiceMacToSw( TdmSlot, SgIdx, OnuDeviceIdx, V2R1_DEFAULT_VLAN, port );
		}
		*logicOnu = port;
		ret = VOS_OK;
	}

	return ret;
}

int AddOnuToSG(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu)
{
	int ret = TDM_VM_OK;
	unsigned long slot, port;
	short int onuIdx;

	slot = GET_PONSLOT(OnuDeviceIdx)/*OnuDeviceIdx / 10000*/;
	port = GET_PONPORT(OnuDeviceIdx)/*(OnuDeviceIdx % 10000 ) /1000*/;
	onuIdx =GET_ONUID(OnuDeviceIdx)/* (OnuDeviceIdx % 1000 )*/ - 1;

	if(ThisIsValidOnu(GetPonPortIdxBySlot(slot, port), onuIdx ) != ROK )
		return TDM_VM_NOT_EXIST;

	if((TdmCardSlotPortCheckByVty(TdmSlot,SgIdx,NULL) != ROK ) ||(logicOnu == NULL))
	/*if( TdmSlot < OLT_MFUNC_BRD ||TdmSlot > OLT_PON_BRD4 || SgIdx < 1 || SgIdx > 3 || logicOnu == NULL)*/
		ret = TDM_VM_ERR;
	else
	{
		bool fVoice = FALSE;
		*logicOnu = 0;
		if( OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice )
		{

			UCHAR tdmslot = 0, sgidx=0;
			USHORT lonuid = 0;

			if( GetOnuBelongToSG( OnuDeviceIdx, &tdmslot, &sgidx, &lonuid ) == TDM_VM_OK )
			{
				if( TdmSlot == tdmslot && SgIdx == sgidx )
					ret = TDM_VM_IN_THIS_SG;
				else
					ret = TDM_VM_IN_ANOTHER_SG;
			}
			else
				ret = AddNewOnuToSg(TdmSlot, SgIdx, OnuDeviceIdx, logicOnu);
		}
		else
			ret = TDM_VM_NOT_SUPPORT_VOICE;
	}

	return ret;
}

#if 0
int DeleteOnuFromSG( unsigned long OnuDeviceIdx )
{	
	int ret = TDM_VM_OK;

	STATUS rc = VOS_ERROR;

	ULONG idxs[2] = { 0, 0 };

	UCHAR tdmslot=0, sgidx=0;
	USHORT logicOnu = 0;

	/*if( GetOnuBelongToSG( OnuDeviceIdx, &tdmslot, &sgidx, &logicOnu ) == VOS_OK )*/
	if( onuIsOnLine( OnuDeviceIdx ) )
		ConfigOnuTdmVoiceService( OnuDeviceIdx );
			
	/*调用平台的API，删除语音MAC*/
	DeleteVoiceMacFromSw( tdmslot, sgidx, logicOnu );
			
	idxs[0] = sgidx;
	idxs[1] = OnuDeviceIdx;

	rc = tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_DESTROY, &logicOnu );

	ret = (rc == VOS_OK )?TDM_VM_OK:TDM_VM_NOT_IN_ANY_SG;

	return ret;
}
#endif

int DeleteOnuFromSG( unsigned long OnuDeviceIdx )
{
	int ret = TDM_VM_OK;

	STATUS rc = VOS_ERROR;

	ULONG idxs[2] = { 0, 0 };

	UCHAR tdmslot=0, sgidx=0;
	USHORT logicOnu = 0,  logicOnu1 = 0;

	ret = GetOnuBelongToSG( OnuDeviceIdx, &tdmslot, &sgidx, &logicOnu );

	if( ret == TDM_VM_OK )
		{
		idxs[0] = sgidx;
		idxs[1] = OnuDeviceIdx;

		rc = tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_DESTROY, &logicOnu1 );
		if(  ret !=  VOS_OK ) 
			return( TDM_VM_ERR );

		if( onuIsOnLine(OnuDeviceIdx) == TRUE )
			{
			VoiceVlanEnable VoiceVlan;
			OnuVoiceEnable VoiceEn;
				
			/* 删除ONU 上语音VLAN  */
			VoiceVlan.Enable =DISABLE;
			GetTdmSGVoiceVlanEnable(tdmslot, sgidx, &VoiceVlan.Enable, &VoiceVlan.Priority, &VoiceVlan.VlanId);
			if(  VoiceVlan.Enable == V2R1_ENABLE )
				{
				VoiceVlan.Enable = DISABLE;
				SetOnuVoiceVlan( OnuDeviceIdx,  &VoiceVlan);
				}

			/* 去使能所有ONU POTS 口*/
			/*SetOnuPotsEnable(OnuDeviceIdx, DISABLE, ONU_POTS_NUM_MAX, ONU_POTS_ALL);*/	/* removed by xieshl 20080527 */

			/* ONU 上语音去使能*/
			VoiceEn.VoiceEnable = DISABLE;
			VoiceEn.SgIdx = sgidx;
			VoiceEn.TdmSlot = tdmslot;
			VoiceEn.LogicOnuIdx = logicOnu;
			SetOnuVoiceEnable(OnuDeviceIdx, &VoiceEn);

			}

		/* 删除BCM56300 静态语音MAC */
		DeleteVoiceMacFromSw(tdmslot, sgidx, logicOnu );

		return( TDM_VM_OK );
		}
	
	else return( ret );
	
	/*	
		case TDM_VM_ERR :	
		case TDM_VM_NOT_SUPPORT_VOICE :
		case TDM_VM_NOT_IN_ANY_SG:
	*/

}

int DeleteOnuFromSGBySgIdx( unsigned long OnuDeviceIdx, unsigned char tdmslot, unsigned char sgidx)
{
	int ret = TDM_VM_OK, ret1 = TDM_VM_OK;

	/*STATUS rc = VOS_ERROR;*/

	ULONG idxs[2] = { 0, 0 };
	unsigned char ret_tdmslot=0,  ret_sgidx=0;
	USHORT logicOnu = 0, logicOnu1=0;

	ret = GetOnuBelongToSG( OnuDeviceIdx, &ret_tdmslot, &ret_sgidx, &logicOnu );

	/*  不管ONU是否配置，都做一次删除操所；用于兼容当前状态为not-ready */
	idxs[0] = sgidx;
	idxs[1] = OnuDeviceIdx;

	ret1 = tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs, RS_DESTROY, &logicOnu1 );

	if(( ret == TDM_VM_OK ) && ( ret1 == TDM_VM_OK ))
		{
		/* */
		bool fVoice = FALSE;
		if((tdmslot != ret_tdmslot ) || ( sgidx != ret_sgidx ))
			return(TDM_VM_IN_ANOTHER_SG);
		
		if(( onuIsOnLine(OnuDeviceIdx) == TRUE ) &&(( OnuIsSupportVoice(OnuDeviceIdx,&fVoice)== ROK) &&( fVoice == TRUE)))
			{
			VoiceVlanEnable VoiceVlan;
			OnuVoiceEnable VoiceEn;

			/* ONU 上语音去使能*/
			VoiceEn.VoiceEnable = DISABLE;
			VoiceEn.SgIdx = sgidx;
			VoiceEn.TdmSlot = tdmslot;
			VoiceEn.LogicOnuIdx = logicOnu;
			SetOnuVoiceEnable(OnuDeviceIdx, &VoiceEn);
				
			/* 删除ONU 上语音VLAN  */
			VoiceVlan.Enable = V2R1_DISABLE;
			GetTdmSGVoiceVlanEnable(tdmslot, sgidx, &VoiceVlan.Enable, &VoiceVlan.Priority, &VoiceVlan.VlanId);
			if(  VoiceVlan.Enable == V2R1_ENABLE )
				{
				VoiceVlan.Enable = DISABLE;
				SetOnuVoiceVlan( OnuDeviceIdx,  &VoiceVlan);
				}

			/* 去使能所有ONU POTS 口*/
			/*SetOnuPotsEnable(OnuDeviceIdx, DISABLE, ONU_POTS_NUM_MAX, ONU_POTS_ALL);*/	/* removed by xieshl 20080527 */
			}

		/* 删除BCM56300 静态语音MAC */
		/* modified by chenfj 2008-2-29
		     在去使能ONU语音业务及清除地址表中静态语音MAC 之间加检测机制,
		     以确保清除语音MAC 之前, ONU上关闭了语音业务; 这样就可以保证在交换
		     芯片的地址表中不会增加垃圾表项
		    */
		/* modified by chenfj 2008-8-5
		    执行条件中, 当ONU在线时, 增加ONU 是否支持语音业务的判断;
		    循环条件中, 增加执行次数的限制
		  */
		{
		BOOL online_status = FALSE;
		bool fVoice=FALSE;
		unsigned char OamCounter = 0;
		unsigned char VoiceEnable = V2R1_ENABLE;
		unsigned char SrcMac[BYTES_IN_MAC_ADDRESS], DestMac[BYTES_IN_MAC_ADDRESS];
		
		OnuIsSupportVoice(OnuDeviceIdx, &fVoice);
		do{
			OamCounter++;
			online_status = onuIsOnLine(OnuDeviceIdx);
			if((online_status == TRUE ) &&(fVoice == TRUE))
				{
				VOS_TaskDelay(ONU_OAM_COMM_WAIT_SEND);
				GetOnuVoiceEnable(OnuDeviceIdx, &VoiceEnable,SrcMac, DestMac);
				}
			else VoiceEnable = DISABLE;
				
			}while((VoiceEnable == ENABLE) && (OamCounter < MAX_GETEUQINFO_COUNTER));
		}
		/*VOS_TaskDelay(ONU_OAM_COMM_WAIT_SEND*ONU_OAM_COMM_WAIT_SEND);*/
		DeleteVoiceMacFromSw(tdmslot, sgidx, logicOnu );

		onuTdmServiceAlarmStatus_update(OnuDeviceIdx);
	
		return( TDM_VM_OK );
		}
	/* modified by chenfj 2008-8-5
	    当删除一个没有配置的语音ONU 时, 
	    返回的提示信息是'not in any tdm'
	    而不是'%executing failed'
	    */
	else if(ret == TDM_VM_NOT_IN_ANY_SG)
		return(ret);
	else if( ret1 == TDM_VM_OK )
		return( ret );
	else if( ret == TDM_VM_OK )
		return( ret1 );
	else return ( TDM_VM_ERR );
	
	
	/*
		case TDM_VM_ERR :	
		case TDM_VM_NOT_SUPPORT_VOICE :
		case TDM_VM_NOT_IN_ANY_SG:
	*/

}

int DeleteAllOnuFromSG(unsigned char TdmSlot ,unsigned char SgIdx)
{
	
	int ret = TDM_VM_OK;
	
	if(TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )
	/*if( SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 )*/
		ret = TDM_VM_ERR;
	
	else
	{
		ULONG idxs[2] = { 0, 0 };
		/*
		while( tdm_tdmOnuTable_getNext( idxs, &entry ) == VOS_OK )
		{
			if( entry.sgIdx == SgIdx )
				DeleteOnuFromSG( entry.devIdx );

			idxs[0] = entry.sgIdx;
			idxs[1] = entry.devIdx;
		}
		*/
		ULONG onucounter = 0;
		ULONG onuIdxs[MAXONUPERSG]= {0};
		
		if( GetAllOnuBelongToSG( TdmSlot, SgIdx, &onucounter, onuIdxs ) == VOS_OK )
		{
			int i=0;
			for( ; i<onucounter; i++ )
			{
				idxs[0] = SgIdx;
				idxs[1] = onuIdxs[i]>>8;
				mn_tdmOnuDel( idxs );
				
				/* 删除BCM56300 静态语音MAC */
				DeleteVoiceMacFromSw(TdmSlot, SgIdx, (onuIdxs[i] & 0xff )+1);

				if( onuIsOnLine(onuIdxs[i]>>8) == TRUE )
					{
					VoiceVlanEnable VoiceVlan;
					OnuVoiceEnable VoiceEn;
						
					/* 删除ONU 上语音VLAN  */
					VoiceVlan.Enable = V2R1_DISABLE;
					GetTdmSGVoiceVlanEnable(TdmSlot, SgIdx, &VoiceVlan.Enable, &VoiceVlan.Priority, &VoiceVlan.VlanId);
					if(  VoiceVlan.Enable == V2R1_ENABLE )
						{
						VoiceVlan.Enable = V2R1_DISABLE;
						SetOnuVoiceVlan( onuIdxs[i]>>8,  &VoiceVlan);
						}

					/* 去使能所有ONU POTS 口*/
					/*SetOnuPotsEnable(onuIdxs[i]>>8, DISABLE, ONU_POTS_NUM_MAX, ONU_POTS_ALL);*/

					/* ONU 上语音去使能*/
					VoiceEn.VoiceEnable = DISABLE;
					VoiceEn.SgIdx = SgIdx;
					VoiceEn.TdmSlot = TdmSlot;
					VoiceEn.LogicOnuIdx = (onuIdxs[i] & 0xff ) +1;
					SetOnuVoiceEnable(onuIdxs[i]>>8, &VoiceEn);

					}
			}
		}
		
	}

	return ret;	
}

int GetOnuBelongToSG(unsigned long OnuDeviceIdx, unsigned char *TdmSlot ,unsigned char *SgIdx,unsigned short int *LogicOnuId)
{
	int ret = TDM_VM_OK;
	
	if( TdmSlot == NULL || SgIdx == NULL || LogicOnuId == NULL )
		ret = TDM_VM_ERR;
	else
	{	/*
		bool fVoice = FALSE;
	
		if( !(OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice == TRUE))
			ret = TDM_VM_NOT_SUPPORT_VOICE;
		else 
		*/
		{
			
			UCHAR i=TDM_FPGA_MIN;
			for( ; i<=TDM_FPGA_MAX; i++ )
			{
				
				ULONG onunum = 0;
				ULONG onuIdxs[MAXONUPERSG]= { 0 };
				if( GetAllOnuBelongToSG( get_gfa_sg_slotno(), i, &onunum, onuIdxs ) == VOS_OK )
				{
					UINT j=0;
					for( ; j<onunum; j++ )
					{
						if( OnuDeviceIdx == (onuIdxs[j]>>8) )
						{
							/*
							tdmonutable_row_entry entry;
							ULONG idxs[2] = { 0, 0 };

							idxs[0] = i;
							idxs[1] = OnuDeviceIdx;
							if( tdm_tdmOnuTable_get( idxs, &entry ) == VOS_OK )
							{
								*TdmSlot = get_gfa_tdm_slotno();
								*SgIdx = i;
								*LogicOnuId = entry.logiconuIdx;
								break;
							}
							*/
							*TdmSlot = get_gfa_sg_slotno();
							*SgIdx = i;
							*LogicOnuId = (onuIdxs[j]&0xff)+ 1;
							break;
						}
					}
					
					if( j < onunum )
						break;
				}
			}

			if( i > TDM_FPGA_MAX )
				ret = TDM_VM_NOT_IN_ANY_SG;	
			
		}
/*		
		else if( tdm_tdmOnuTable_getNext( idxs, &entry ) == VOS_ERROR )
			ret = TDM_VM_NOT_IN_ANY_SG;
		else
		{
			STATUS rc = VOS_OK;
			
			do{
				if( entry.devIdx == OnuDeviceIdx )
				{
					*TdmSlot = get_gfa_tdm_slotno();
					*SgIdx = entry.sgIdx;
					*LogicOnuId = entry.logiconuIdx;
					break;
				}
				idxs[0] = entry.sgIdx;
				idxs[1] = entry.devIdx;
			}while( (rc = tdm_tdmOnuTable_getNext( idxs, &entry ) )!= VOS_ERROR );

			ret = ( rc == VOS_OK )?TDM_VM_OK:TDM_VM_NOT_IN_ANY_SG;
		}
*/		
	}	
	
	return ret;
}

int GetAllOnuBelongToSG(unsigned char TdmSlot,unsigned char SgIdx, unsigned long *OnuNumber, unsigned long *OnuDeviceIdx)
{
	int ret = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	get_onu_pdu *p = NULL;
	UCHAR calltype = ENUM_CALLTYPE_SYN;
	tdm_pdu_t *pdu = NULL;
	SgOnuTableAll_RspPdu *PduData = NULL;
	
	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK ) ||
	/*if((SgIdx < MIN_SG_IDX) ||( SgIdx > MAX_SG_IDX) ||(TdmSlot < OLT_MFUNC_BRD )||(TdmSlot > OLT_PON_BRD4)||*/
		(OnuNumber == NULL) ||(OnuDeviceIdx == NULL))
		return(ret);

	pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLONU);
	p = (get_onu_pdu*)pdu->msgData;
	p->reserved1= 0;
	p->reserved2= 0;
	p->sgIndex = SgIdx;

	pdulen = headlen+sizeof( OnuPotsLinkAll_ReqPdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		ret = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*(FUNC_PRC)getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLONU, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, &PduData) )
				ret = VOS_ERROR;
			*OnuNumber = PduData->OnuCount;
			VOS_MemCpy((unsigned char *)OnuDeviceIdx, (unsigned char *)&PduData->OnuIdx[0], (*OnuNumber ) * 4 );

			tdmCommMsgFree( pRecv );
				
			if( g_tdmDebugFlag & TDM_DEBUG_INFO )
			{
				UCHAR i;
				if( *OnuNumber == 0 )
					sys_console_printf("No onu being configed to Sg%d\r\n", SgIdx );
				else sys_console_printf("There are %d onu being configed to Sg%d\r\n", (*OnuNumber ), SgIdx );
				
				for(i=0; i< (*OnuNumber); i++ )
				{
					sys_console_printf("%5d    %5d\r\n", (OnuDeviceIdx[i] >> 8),( (OnuDeviceIdx[i] & 0xff) +1) );
				}
			}
		}					
	}
	return ret;	

}


int GetOnuPotsLinkByDevIdx(unsigned long OnuDeviceIdx, unsigned char PotsBoard, unsigned char PotsPort, BOOL *EnableFlag, unsigned char *TdmSlot, unsigned char *SgIdx, unsigned short int *LogicPort)
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = {0};

	if( getLocation( OnuDeviceIdx, &lct,CONV_YES ) == VOS_ERROR || PotsBoard < 1 || PotsBoard > MAX_ONU_BRD_NUM ||PotsPort < ONU_POTS_NUM_MIN || PotsPort > ONU_POTS_NUM_MAX )
		ret = TDM_VM_ERR;
	else
	{
		bool fVoice = FALSE;
		if( OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice == TRUE )
		{
			UCHAR tdmslot=0, sgIndex=0;
			USHORT logicOnu = 0;
			if( GetOnuBelongToSG( OnuDeviceIdx, &tdmslot, &sgIndex, &logicOnu ) == TDM_VM_OK )
			{
				ULONG potsEnList = 0;

				*TdmSlot = tdmslot;
				*SgIdx = sgIndex;
				
				if( GetOnuPotsLinkAll( OnuDeviceIdx, &potsEnList ) == VOS_OK )
				{
					if( potsEnList & (1<<(PotsPort-1)) )
					{
						potsporttable_row_entry entry;
						ULONG idxs[3]={ 0, 0, 0 };
						idxs[0] = OnuDeviceIdx;
						idxs[1] = PotsBoard;
						idxs[2] = PotsPort;
						if( tdm_potsPortTable_get( idxs, &entry ) == VOS_OK )
						{
							*LogicPort = entry.logicalport;
							*EnableFlag = 1;
						}
						else
							ret = TDM_VM_ERR;
						
					}
					else
						*EnableFlag = 2;
					
				}
				else
					ret = TDM_VM_ERR;
			
			}
			else
				ret = TDM_VM_NOT_IN_ANY_SG;
		}
		else
			ret = TDM_VM_NOT_SUPPORT_VOICE;			
	}
		
	
	return ret;
	
}
/*
add by zhengyt  2008-1-30 增加该函数用于判断pots link
*/
int GetOnuPotsLogicalPort (ULONG Onudevidx, ULONG OnuBoard,ULONG OnuPort,USHORT *Logicalport)
{
	int ret = TDM_VM_OK;
	ULONG idxs[3]={0};
	potsporttable_row_entry  entry;
	
	if(Onudevidx==0||OnuBoard==0||OnuPort> ONU_POTS_NUM_MAX
		||OnuPort<ONU_POTS_NUM_MIN)
			ret=TDM_VM_ERR;

	idxs[0]=Onudevidx;
	idxs[1]=OnuBoard;
	idxs[2]=OnuPort;

	/*sys_console_printf("onuboard=%d,  OnuPort=%d\r\n",OnuBoard,OnuPort);*/

	if(OnuBoard!=1)
		{
			idxs[1]=1;
			/*sys_console_printf("1:dev=%d,board=%d,port=%d\r\n",idxs[0],idxs[1],idxs[2]);*/
		}

	/*sys_console_printf("2:dev=%d,board=%d,port=%d\r\n",idxs[0],idxs[1],idxs[2]);*/

	if( tdm_potsPortTable_get( idxs, &entry ) == VOS_OK )
		{
			*Logicalport=entry.logicalport;
			ret=TDM_VM_OK;
		}

	return ret;

}



int AddOnuPotsLinkToSg(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, 
						unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort)
{
	int ret = TDM_VM_OK;
	
	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK) ||
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 ||
		SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || */
		LogicPort > MAXIMUM_LOGICPORT)
		ret = TDM_VM_ERR;
	else
	{
		bool fVoice = FALSE;
		if( OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice == TRUE )
		{
			USHORT logicOnuId;			
			UCHAR newTdmSlot, newSgIndex;
			BOOL enFlag = FALSE;
			USHORT logicPort = 0;
			
			ret = GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &newTdmSlot, &newSgIndex, &logicPort );
			if( ret == TDM_VM_OK )
			{
				if( newTdmSlot != TdmSlot || newSgIndex != SgIdx )
					ret = TDM_VM_IN_ANOTHER_SG;
				else if( enFlag == 1 && logicPort == LogicPort )
					ret = TDM_VM_EXIST_LINK_POT;
				else if( enFlag == 1 && logicPort != LogicPort )
					ret = TDM_VM_EXIST_ANOTHER_LINK_POT;
				else
				{
					ULONG setval = 0, newPotsBoard = Potsboard, newPotsPort = PotsPort;
					ULONG idxs[2] = { 0, 0 };
					idxs[0] = SgIdx;
					idxs[1] = LogicPort;
					setval = 5;
					if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&setval ) == VOS_OK )
					{
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUDEV, idxs, (UCHAR*)&OnuDeviceIdx );
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUBOARD, idxs, (UCHAR*)&newPotsBoard );
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUPOTS, idxs, (UCHAR*)&newPotsPort );
						setval = RS_ACTIVE;
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs,  (UCHAR*)&setval);

						/*if( onuIsOnLine( OnuDeviceIdx ) )
						{
							ret = (SetOnuPotsEnable( OnuDeviceIdx, ENABLE, 1, 1<<(PotsPort-1) ) == VOS_OK )?TDM_VM_OK:TDM_VM_ERR;
						}*/		/* removed by xieshl 20080527 */
					}
					else
						ret = TDM_VM_ERR;
					
				}
			}
			else if(ret == TDM_VM_NOT_IN_ANY_SG && AddNewOnuToSg(TdmSlot, SgIdx, OnuDeviceIdx,&logicOnuId) == TDM_VM_OK)
			{
					ULONG setval = 0, newPotsBoard = Potsboard, newPotsPort = PotsPort;
					ULONG idxs[2] = { 0, 0 };
					idxs[0] = SgIdx;
					idxs[1] = LogicPort;
					setval = 5;
					if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&setval ) == VOS_OK )
					{
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUDEV, idxs, (UCHAR*)&OnuDeviceIdx );
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUBOARD, idxs, (UCHAR*)&newPotsBoard );
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUPOTS, idxs, (UCHAR*)&newPotsPort );
						setval = RS_ACTIVE;
						tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs,  (UCHAR*)&setval);

						/*通过OAM使能ONU上的POT口*/
						/*VOS_ASSERT( 0 );*/
						/*if( onuIsOnLine( OnuDeviceIdx ) )
						{
						SetOnuPotsEnable( OnuDeviceIdx, ENABLE, 1, 1<<(PotsPort-1) ) ;
						}*/		/* removed by xieshl 20080527 */
						ret = TDM_VM_OK;
					}
					else
						ret = TDM_VM_ERR;				
			}
			
		}
		else
			ret = TDM_VM_NOT_SUPPORT_VOICE;
	}
	
	return ret;
}

#if 0
int AddOnuPotsLinkToSg_other(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, 
						unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort)
{
	int ret = TDM_VM_OK;
	
	if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 ||
		SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX ||
		LogicPort > MAXIMUM_LOGICPORT)
		ret = TDM_VM_ERR;

	else 
		{
			bool fVoice = FALSE;
			if( OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK )
				{
					USHORT logicOnuId;			
					UCHAR newTdmSlot, newSgIndex;
					BOOL enFlag = FALSE;
					USHORT logicPort = 0;

					ret = GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &newTdmSlot, &newSgIndex, &logicPort );
					if( ret == TDM_VM_OK )
						{
							if( newTdmSlot != TdmSlot || newSgIndex != SgIdx )
								ret = TDM_VM_IN_ANOTHER_SG;
							else if( enFlag == 1 && logicPort == LogicPort )
								ret = TDM_VM_EXIST_LINK_POT;
							else if( enFlag == 1 && logicPort != LogicPort )
								ret = TDM_VM_EXIST_ANOTHER_LINK_POT;
							else
								{
									ULONG setval = 0, newPotsBoard = Potsboard, newPotsPort = PotsPort;
									ULONG idxs[2] = { 0, 0 };
									idxs[0] = SgIdx;
									idxs[1] = LogicPort;
									setval = 5;

									tdm_potsLinkTable_rowset( idxs , OnuDeviceIdx,  Potsboard,  PotsPort, ULONG phonecode, UCHAR linkdesc [ 32 ], UCHAR potsLinkStatus)
									
								}
						}

					
				}
		}
}
#endif

int deleteOnuPotsLinkFromSgByLogicPort( ULONG sgIndex, ULONG logicPort )
{
	int ret = TDM_VM_OK;

	if((TdmPortRangeCheck(sgIndex) != ROK)|| logicPort > MAXIMUM_LOGICPORT)
	/*if( sgIndex < MIN_SG_IDX || sgIndex > MAX_SG_IDX || logicPort > MAXIMUM_LOGICPORT)*/
		ret = TDM_VM_ERR;
	else
	{
		ULONG idxs[2] = {0}, setval = RS_DESTROY;
		idxs[0] = sgIndex;
		idxs[1] = logicPort;

		if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&setval ) == VOS_ERROR )
			ret = TDM_VM_ERR;

		/*TODO:
		向ONU写POTS口使能关闭
		
		
		if((ret == TDM_VM_OK ) && (onuIsOnLine( OnuDeviceIdx )) 
			SetOnuPotsEnable( OnuDeviceIdx, DISABLE, 1, 1<<(PotsPort-1) );.
		*/
	}

	return ret;
}

int DeleteOnuPotsLinkFromSg(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort)
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = { 0 };

	bool fVoice = FALSE;

	if( getLocation( OnuDeviceIdx, &lct, CONV_YES ) != VOS_OK || Potsboard < 1 || Potsboard > MAX_ONU_BRD_NUM || PotsPort < ONU_POTS_NUM_MIN ||PotsPort > ONU_POTS_NUM_MAX )
		ret = TDM_VM_ERR;
	else if( !(OnuIsSupportVoice(OnuDeviceIdx, &fVoice)== VOS_OK && fVoice) )
		ret = TDM_VM_NOT_SUPPORT_VOICE;
	else
	{
		potsporttable_row_entry potsPortEntry;
		ULONG idxs[3] = {0};
		idxs[0] = OnuDeviceIdx;
		idxs[1] = Potsboard;
		idxs[2] = PotsPort;

		if( tdm_potsPortTable_get( idxs, &potsPortEntry ) == VOS_OK )
		{
			ret = deleteOnuPotsLinkFromSgByLogicPort( potsPortEntry.sgifIdx, potsPortEntry.logicalport );
		}
		else
			ret = TDM_VM_ERR;

	}

	return ret;
}

int GetOnuPotsLinkBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, 
							BOOL *EnableFlag, unsigned long *OnuDeviceIdx, unsigned char *PotsBoard, unsigned char *PotsPort)
{
	int ret = TDM_VM_OK;

	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 || SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX*/
		|| LogicPort > MAXIMUM_LOGICPORT)
		ret = TDM_VM_ERR;
	else
	{
		potslinktable_row_entry entry;
		ULONG idxs[2]={ 0, 0 };
		idxs[0] = SgIdx;
		idxs[1] = LogicPort;
		if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_OK )
		{
			*EnableFlag = ( entry.linkrowstatus == 1 )?TRUE:FALSE;
			*OnuDeviceIdx = entry.devIdx;
			*PotsBoard = entry.brdIdx;
			*PotsPort = entry.potsIdx;
		}
		else
			ret = TDM_VM_NOT_EXIST_LINK_POT;
	}

	return ret;
}

/*modified by wangxiaoyu 2007-12-11, static -> global*/
long atobcd( const UCHAR *p )
{
	long ret = 0;
	UCHAR val = 0;
	int i=0, len=0;

	/*begin:
	added by wangxiaoyu 2008-03-21
	filter the leading 'zero' or whitespace
	*/
	while(*p == '0'|| *p == ' ')
		p++;
	/*end*/

	/*begin:added by wangxiaoyu 2008-01-30*/
	len = VOS_StrLen(p);
	if(len > 8)
		len = 8;
	/*end*/

	for( ; i<len; i++ )
	{
		val = *(p+i);
		if( val >= '0' && val <='9' )
			val = val-'0';
		else if( val >= 'a' && val <= 'f' )
			val = 10+val-'a';
		else if (val >='A' && val <= 'F') 
			val = 10+val-'A';
		else
			break;
		
		ret |= ((long)val)<<((len-1-i)*4);	/*modified by wangxiaoyu 2008-01-30 7-->len-1*/
	}

	return ret;
}
	
int SetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, 
													unsigned char PotsPort, unsigned char *PhonuNumber )
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = { 0 };

	BOOL enFlag = FALSE;
	UCHAR tdmSlot = 0, sgIndex=0;
	USHORT lport = 0;

	if( getLocation( OnuDeviceIdx, &lct, CONV_YES ) == VOS_ERROR ||Potsboard < 1 || Potsboard > MAX_ONU_BRD_NUM ||
					PotsPort < ONU_POTS_NUM_MIN || PotsPort > ONU_POTS_NUM_MAX )
					ret = TDM_VM_ERR;

	else if( GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &tdmSlot, &sgIndex, &lport ) == TDM_VM_OK )
	{
		long setval = atobcd( PhonuNumber );
		ULONG idxs[2] = { 0, 0 };
		idxs[0] = sgIndex;
		idxs[1] = lport;
		if( setval == -1 || tdm_potsLinkTable_set( LEAF_EPONPOTSLINKPHONECODE, idxs, (UCHAR*)&setval ) ==  VOS_ERROR )
			ret = TDM_VM_ERR;
	}
	else
		ret = TDM_VM_NOT_EXIST_LINK_POT;

	return ret;
}

int SetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,
															unsigned char *PhonuNumber)
{
	int ret =  TDM_VM_OK;

	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 || SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX*/
		|| LogicPort > MAXIMUM_LOGICPORT)
		ret = TDM_VM_ERR;
	else
	{
		potslinktable_row_entry entry;

		ULONG idxs[2] = {0};
		idxs[0] = SgIdx;
		idxs[1] = LogicPort;
		
		if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_OK )
		{
			long setval = atobcd( PhonuNumber );
			if( setval == -1 ||tdm_potsLinkTable_set( LEAF_EPONPOTSLINKPHONECODE, idxs, (UCHAR*)&setval ) == VOS_ERROR )
				ret = TDM_VM_ERR;
		}
		else
			ret = TDM_VM_NOT_EXIST_LINK_POT;
	}

	return ret;
}


/*modified by wangxiaoyu 2007-12-11, static -> global*/
/*modified by wangxiaoyu 2008-01-30, 去除了清除字符串末尾'0'的代码*/
int bcdtoa( const ULONG code, UCHAR * const phoneNum )
{
	int ret  = VOS_OK;

	if( phoneNum == NULL )
		ret = VOS_ERROR;
	else
	{
		/*char *p = NULL;*/
		sprintf( phoneNum, "%-8x", code );
		
	}

	return ret;
}

int GetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard,
													unsigned char PotsPort,unsigned char *PhonuNumber)
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = { 0 };

	UCHAR tdmSlot = 0, sgIndex = 0;
	USHORT lport = 0;
	BOOL enFlag = 0;
	bool fVoice = FALSE;

	if( getLocation( OnuDeviceIdx, &lct, CONV_YES ) == VOS_ERROR || Potsboard < 1 || Potsboard > MAX_ONU_BRD_NUM ||
							PotsPort < ONU_POTS_NUM_MIN || PotsPort > ONU_POTS_NUM_MAX )
							ret = TDM_VM_ERR;
	else if( !(OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice == TRUE))
		ret = TDM_VM_NOT_SUPPORT_VOICE;
	else if( GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &tdmSlot, &sgIndex, &lport ) != TDM_VM_OK )
		ret = TDM_VM_NOT_EXIST_LINK_POT;
	else
	{
		potslinktable_row_entry entry;
		ULONG idxs[2] = {0};
		idxs[0] = sgIndex;
		idxs[1] = lport;

		if(!( tdm_potsLinkTable_get( idxs, &entry ) == VOS_OK && bcdtoa( entry.phonecode, PhonuNumber ) == VOS_OK) )
			ret = TDM_VM_ERR;
			
	}

	return ret;
}

int GetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char *PhonuNumber)
{
	int ret = TDM_VM_OK;

	potslinktable_row_entry entry = { 0 };
	ULONG idxs[2] = {0};
	idxs[0] = SgIdx;
	idxs[1] = LogicPort;

	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 || SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX*/
			|| LogicPort > MAXIMUM_LOGICPORT )
			ret = TDM_VM_ERR;

	else if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_ERROR )
		ret = TDM_VM_NOT_EXIST_LINK_POT;
	else if( bcdtoa( entry.phonecode, PhonuNumber ) == VOS_ERROR )
		ret = TDM_VM_ERR;

	return ret;
}

int SetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description )
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = { 0 };

	BOOL enFlag = FALSE;
	UCHAR tdmSlot = 0, sgIndex=0;
	USHORT lport = 0;

	if( getLocation( OnuDeviceIdx, &lct, CONV_YES ) == VOS_ERROR ||Potsboard < 1 || Potsboard > MAX_ONU_BRD_NUM ||
					PotsPort < ONU_POTS_NUM_MIN || PotsPort > ONU_POTS_NUM_MAX )
					ret = TDM_VM_ERR;

	else if( GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &tdmSlot, &sgIndex, &lport ) == TDM_VM_OK )
	{
		ULONG idxs[2] = { 0, 0 };
		idxs[0] = sgIndex;
		idxs[1] = lport;
		if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKDESC, idxs, Description ) ==  VOS_ERROR )
			ret = TDM_VM_ERR;
	}
	else
		ret = TDM_VM_NOT_EXIST_LINK_POT;

	return ret;	
}

int SetOnuPotsLinkDescBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description )
{
	int ret =  TDM_VM_OK;

	if(TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 || SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX || LogicPort > MAXIMUM_LOGICPORT )*/
		ret = TDM_VM_ERR;
	else
	{
		potslinktable_row_entry entry;

		ULONG idxs[2] = {0};
		idxs[0] = SgIdx;
		idxs[1] = LogicPort;
		
		if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_OK )
		{
			if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKPHONECODE, idxs, Description ) == VOS_ERROR )
				ret = TDM_VM_ERR;
		}
		else
			ret = TDM_VM_NOT_EXIST_LINK_POT;
	}

	return ret;
	
}

int GetOnuPotsLinkDescBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description)
{
	int ret = TDM_VM_OK;

	potslinktable_row_entry entry = { 0 };
	ULONG idxs[2] = {0};
	idxs[0] = SgIdx;
	idxs[1] = LogicPort;

	if((TdmCardSlotPortCheckByVty(TdmSlot, SgIdx, NULL) != ROK )||
	/*if( TdmSlot < OLT_MFUNC_BRD || TdmSlot > OLT_PON_BRD4 || SgIdx < MIN_SG_IDX || SgIdx > MAX_SG_IDX ||*/
			LogicPort > MAXIMUM_LOGICPORT )
			ret = TDM_VM_ERR;

	else if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_ERROR )
		ret = TDM_VM_NOT_EXIST_LINK_POT;
	else
		memcpy( Description, entry.linkdesc, 32 );
	
	return ret;	
}

int GetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description )
{
	int ret = TDM_VM_OK;

	LOCATIONDES lct = { 0 };

	UCHAR tdmSlot = 0, sgIndex = 0;
	USHORT lport = 0;
	BOOL enFlag = 0;
	bool fVoice = FALSE;

	if( getLocation( OnuDeviceIdx, &lct, CONV_YES ) == VOS_ERROR || Potsboard < 1 || Potsboard > MAX_ONU_BRD_NUM ||
							PotsPort < ONU_POTS_NUM_MIN || PotsPort > ONU_POTS_NUM_MAX )
							ret = TDM_VM_ERR;
	else if( !(OnuIsSupportVoice( OnuDeviceIdx, &fVoice ) == VOS_OK && fVoice == TRUE) )
		ret = TDM_VM_NOT_SUPPORT_VOICE;
	else if( GetOnuPotsLinkByDevIdx( OnuDeviceIdx, Potsboard, PotsPort, &enFlag, &tdmSlot, &sgIndex, &lport ) != TDM_VM_OK )
		ret = TDM_VM_NOT_EXIST_LINK_POT;
	else
	{
		potslinktable_row_entry entry;
		ULONG idxs[2] = {0};
		idxs[0] = sgIndex;
		idxs[1] = lport;

		if( tdm_potsLinkTable_get( idxs, &entry ) == VOS_ERROR )
			ret = TDM_VM_ERR;
		else
			memcpy( Description, entry.linkdesc, 32 );
			
	}

	return ret;	
}

int GetOnuPotsLinkAll(unsigned long OnuDeviceIdx, unsigned long *PotsEnableList)
{
	int rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	OnuPotsLinkAll_ReqPdu * p = NULL;
	UCHAR calltype = ENUM_CALLTYPE_SYN;
	tdm_pdu_t *pdu = NULL;

	if( PotsEnableList == NULL ) return( rc);
	
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLPOTS );
	p = (OnuPotsLinkAll_ReqPdu*)pdu->msgData;
	p->tableIdx = 0;
	p->leafIdx = 0;
	p->devIdx= OnuDeviceIdx;

	pdulen = headlen+sizeof( OnuPotsLinkAll_ReqPdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*(FUNC_PRC)getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLPOTS, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, PotsEnableList ) )
				rc = VOS_ERROR;			
			
			tdmCommMsgFree( pRecv );
		}					
	}
	return rc;	
	
}

STATUS process_rtn_sg_onu_get_all(const char *pBuf, USHORT len, void *ret_ptr)
{
	tdm_pdu_t * pdu;
	if( pBuf == NULL ) 
		{
		ret_ptr = NULL;
		return( VOS_ERROR );
		}
	pdu = (tdm_pdu_t*) pBuf;
	*(UCHAR**)ret_ptr = pdu->msgData;
	return VOS_OK;
}

STATUS process_rtn_pots_port_get_all( const char *pBuf, USHORT len, void * pData )
{
	OnuPotsLinkAll_RspPdu *PduData;
	tdm_pdu_t * pdu = (tdm_pdu_t*)pBuf;
	int i;
	int rc;

	UINT * PotsPortList = (UINT*)pData;


	rc = VOS_ERROR;

	if( PotsPortList == NULL )
		return rc;
	
	if(pBuf == NULL ) 
		{
		*PotsPortList = 0;
		return( rc );
		}

	PduData = (OnuPotsLinkAll_RspPdu*)(pdu->msgData);

	*(UINT*)PotsPortList = 0;
	for(i = 0; i< ONU_POTS_NUM_MAX; i++)
		{
		if((PduData->potslist[i]) == 1 ) 
			*PotsPortList += (1 << i );
		}

	if( g_tdmDebugFlag & TDM_DEBUG_INFO )
	{
		sys_console_printf( "\r\n" );
		reportd( "table idx:", (int)PduData->tableIdx);
		reportd( "field idx:", (int)PduData->leafIdx);
		reportd( "dev idx:", (int)PduData->devIdx);
		sys_console_printf( "port list:");
		for( i = 0; i< ONU_POTS_NUM_MAX; i++)
			{
			if(PduData->potslist[i] == 1 )
				sys_console_printf("1 ");
			else sys_console_printf("2 ");
			if(( i+1 ) % 16 == 0)
				sys_console_printf("\r\n");
			}
		sys_console_printf( "\r\n" );
	}
		
	rc = VOS_OK;
	
	return( rc );

}

extern int SetOnuVoiceLoopbackStatus(unsigned long onuDevIdx, unsigned char LoopStatus);
static int mn_tdmOnuVoiceLoopback( ULONG *idxs, ULONG ctrl )
{
	int rc;
	tdmonutable_row_entry  entry;
	USHORT lport;
	UCHAR onu_voice_loop_en = 0;

	VOS_MemZero( &entry, sizeof(tdmonutable_row_entry) );
	rc = tdm_tdmOnuTable_get( idxs, &entry );
	if( (rc == VOS_OK) && (entry.rowstatus == 1) )
	{
		/* modified by xieshl 20080407 */
		/*if( entry.potsloopenable != 1 )
			return rc;*/
		if( ctrl == 1 )
			onu_voice_loop_en = 1;
		if( SetOnuVoiceLoopbackStatus(idxs[1], onu_voice_loop_en) == 0 )
		{
			/*return*/ tdm_tdmOnuTable_set( LEAF_EPONTDMONUPOTSLOOPBACKCTRL, idxs , ctrl, &lport );
			return VOS_OK;
		}
	}
	return VOS_ERROR;
}

int mn_tdmOnuVoiceLoopbackStart( ULONG *idxs )
{
	return mn_tdmOnuVoiceLoopback( idxs, 1 );
}
int mn_tdmOnuVoiceLoopbackStop( ULONG *idxs )
{
	return mn_tdmOnuVoiceLoopback( idxs, 2 );
}

#endif

#ifdef __cplusplus
}
#endif


