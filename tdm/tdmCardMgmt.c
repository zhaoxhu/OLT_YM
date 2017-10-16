/***************************************************************
*
*						Module Name:  tdmCardMgmt.c
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
*   Date: 			2007/11/7
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  07/11/7    |   chenfj          |     create 
**----------|-----------|------------------
**
** modified by chenfj 2008-7-25
**         增加GFA6100 产品支持
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
#include "TdmService.h"

/*extern unsigned long getTdmChipInserted( unsigned char slot_id, unsigned char tdm_id );*/
extern STATUS epon_tdm_cfgdata_retrieve( );

unsigned int  TdmCardStatus = TDMCARD_UNKNOWN;
unsigned char  TdmCardStatusFromTdm;
unsigned int  TdmActivatedFlag = FALSE;
unsigned int  TdmAppUpdateFlag = FALSE;
unsigned int  TdmFpgaUpdateFlag = FALSE;
unsigned int  TdmFpgaLoadingCounter = 0;
unsigned int  TDM_MGMT_DEBUG = V2R1_DISABLE;
unsigned long TDMCardIdx = 5;
unsigned int  TDMStatusCheck=0;
 /* 连续100 次不能读到TDM板状态，则认为TDM 板故障*/
#define  STATUS_CHECK_MAX   200

unsigned char TdmMsgBuf[1600];
unsigned short int TdmMsgLen = 0;
unsigned int TdmFpgaLoadingComp = FALSE;


TdmCommMsgRecord  TdmMsgRecord[MGMT_MSG_SUBTYPE_LAST] = { 0 };

unsigned int   Flag_WaitTdmStatusQuery ;

unsigned char *TdmMsgType[] = 
{
	(unsigned char *)"read version",
	(unsigned char *)"load fgpa",
	(unsigned char *)"load fpga comp",
	(unsigned char *)"tdm activate",
	(unsigned char *)"status query",
	(unsigned char *)"tdm reset",
	(unsigned char *)"unknown"
};

unsigned char  TdmChipDownloadComp[4] = { V2R1_NOTSTARTED,V2R1_NOTSTARTED,V2R1_NOTSTARTED,V2R1_NOTSTARTED};

VOID tdm_interface_update_for_card_insert( ULONG slotno );
VOID tdm_interface_update_for_card_pull( ULONG slotno );
extern void tdmQueueFlush( void );

/* 1 秒定时消息,暂时未用*/
void OnuSecondMsgToTdmMgmt()
{
	unsigned long aulMsg[4] = { MODULE_TDM_CARD_MGMT, TDM_TIMER_OUT, 0, 0};
	return;
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err\r\n"  );*/
		}
}

int OnuSecondMsgHandler()
{
	int i;

	for(i=0; i< (MGMT_MSG_SUBTYPE_LAST-1); i++)
		{
		if( TdmMsgRecord[i].SendFlag == TRUE )
			{
			TdmMsgRecord[i].TimeCount ++;
			if(TdmMsgRecord[i].TimeCount > TDM_COMM_TIMEOUT )
				{
				sys_console_printf("  send %s msg to tdm, but no response received\r\n", TdmMsgType[i]);
				TdmMsgRecord[i].SendFlag = FALSE;
				TdmMsgRecord[i].TimeCount = 0;
				}
			}
		}
	
	return( ROK );
}
#if 0
void  TdmMgmtEntry()
{
	unsigned long aulMsg[4];
	long result;
	unsigned long tdmCardIndex;
	TdmRecvMsgBuf  *BufPtr;

	TdmMsgRecvSemId = VOS_SemBCreate( VOS_SEM_Q_FIFO, VOS_SEM_EMPTY);
	Tdm_mgmt_Task_flag = 1;

	for(;;)
		{
		result = VOS_QueReceive( g_PonUpdate_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR ) 
			{
			ASSERT(0);
			sys_console_printf("error:recv msg from tdm-dev queue fail(TdmMgmtEntry)\r\n" );
			return;
			}
		if( result == VOS_NO_MSG ) continue; 
		
		if((aulMsg[1] != TDM_MSG_RECV_CODE )&& (aulMsg[1] != TDM_TIMER_OUT ))
			tdmCardIndex = aulMsg[2];
		else tdmCardIndex = TDMCardIdx;
		
		switch (aulMsg[1])
			{
			/* 检测到tdm 板在位*/
			case TDM_INSERTED:	
				sys_console_printf("  tdm(slot%d) is to be inserting ...\r\n", (tdmCardIndex +1));
				TdmCardStatus = TDMCARD_NOTLOADING;
				TdmVersionQuery( tdmCardIndex );
				TDMCardIdx = tdmCardIndex;
				break;

			/* 检测到tdm 板不在位*/
			case TDM_PULLED:
				/* 1:    */
				sys_console_printf("  tdm(slot%d) is pulled\r\n", (tdmCardIndex +1));
				TdmCardStatus = TDMCARD_DEL;
				TdmReset( tdmCardIndex );
				
				/*2: 删除voice 相关MAC 地址及vlan 配置*/

				/*3:  */
				SetTdmCardPulled(tdmCardIndex);
				TDMCardIdx = tdmCardIndex;
				
				break;

			/* reset tdm 板*/
			case TDM_RESET:
				sys_console_printf("  tdm(slot%d) is reset\r\n", (tdmCardIndex +1));
				TdmCardStatus = TDMCARD_NOTLOADING;
				TdmReset( tdmCardIndex );

				SetTdmCardPulled(tdmCardIndex);
				TDMCardIdx = tdmCardIndex;
				break;

			/* 激活tdm 板*/
			case TDM_ACTIVATE:
				sys_console_printf("  tdm(slot%d) is to be activating ...\r\n", (tdmCardIndex +1));
				TdmCardStatus = TDMCARD_ACTIVATED;
				TdmRunningEnable(tdmCardIndex);
				TDMCardIdx = tdmCardIndex;
				break;
				
			/* 查询tdm 板状态*/
			case TDM_STATUS_QUERY:

				break;

			/* tdm 升级APP 结束*/
			case TDM_APP_LOAD_COMP:
				result = aulMsg[3];
				TdmAppUpdateFlag = FALSE;
				if( result != ROK )
					TdmCardStatus = TDMCARD_ERROR ;				
				else if( TdmFpgaUpdateFlag == FALSE )
					TdmCardStatus = TDMCARD_UPDATE_COMP;
				break;

			/* tdm 升级APP 结束*/
			case TDM_FPGA_LOAD_COMP:
				TdmFpgaUpdateFlag = FALSE;
				result = aulMsg[3];
				if( result != ROK )
					TdmCardStatus = TDMCARD_ERROR ;
				else if(TdmAppUpdateFlag == FALSE) 
					TdmCardStatus = TDMCARD_UPDATE_COMP;
				break;
			
			/* 从tdm 板接收消息处理*/
			/*case TDM_COMM_RECV_MSG:  */
			case TDM_MSG_RECV_CODE:
				/*BufPtr = (TdmRecvMsgBuf *)aulMsg[3];*/
				TdmMgmtLowLevelMsgHandler( tdmCardIndex,(unsigned char *)aulMsg[3], aulMsg[2]);
				/*VOS_Free((void *)BufPtr);*/
				break;

			/* tdm 管理定时消息*/
			case TDM_TIMER_OUT:
				OnuSecondMsgHandler();
				break;
				
			}

		}
}
#endif
/* tdm 管理任务*/
int TdmMsgHandler(unsigned long *MsgBuf)
{
	unsigned long tdmCardIndex;

	if((MsgBuf[1] != TDM_MSG_RECV_CODE )&& (MsgBuf[1] != TDM_TIMER_OUT ))
		{
		tdmCardIndex = MsgBuf[2];
		TDMCardIdx = tdmCardIndex;
		}
	else tdmCardIndex = TDMCardIdx;
	
	switch (MsgBuf[1])
		{
		/* 检测到tdm 板在位*/
		case TDM_INSERTED:	
			/*TdmCardStatus = TDMCARD_NOTLOADING;
			VOS_TaskDelay(VOS_TICK_SECOND*5);*/
			TdmCardInsert( tdmCardIndex );
			TDMCardIdx = tdmCardIndex;
			break;

		/* 检测到tdm 板不在位*/
		case TDM_PULLED:
			TdmCardPull( tdmCardIndex);
			TDMCardIdx = tdmCardIndex;			

	setEthPortAnAdminStatus( 1, tdmCardIndex, 1, 1);
	setEthPortAnAdminStatus( 1, tdmCardIndex, 2, 1);
	setEthPortAnAdminStatus( 1, tdmCardIndex, 3, 1);

			break;

		/* reset tdm 板*/
		case TDM_RESET:
			ResetTdmCard( tdmCardIndex );
			TDMCardIdx = tdmCardIndex;
			break;

		/* 激活tdm 板*/
		case TDM_ACTIVATE:
			/*TdmCardStatus = TDMCARD_ACTIVATED;*/
			tdmRunNotify( ENUM_CALLTYPE_NOACK, 0 );
			/*
			GetTdmCardStatus( tdmCardIndex);
			if( TdmCardStatus == TDMCARD_ACTIVATED )
				{
				sys_console_printf("  tdm(slot%d) is to be activating ... ok\r\n", (tdmCardIndex));
				SetEthPortVoiceIsolateAll(tdmCardIndex, ETH_FILTER_PERMIT );
				RestoreTdmSgBoardVoiceMacVlan(tdmCardIndex);
				}
			else{
				sys_console_printf("  tdm(slot%d) is to be activating ... err \r\n", (tdmCardIndex));	
				TdmActivatedFlag = FALSE;
				}
			TDMCardIdx = tdmCardIndex;
			*/
			
	setEthPortAnAdminStatus( 1, tdmCardIndex, 1, 2);
	setEthPortAnAdminStatus( 1, tdmCardIndex, 2, 2);
	setEthPortAnAdminStatus( 1, tdmCardIndex, 3, 2);
			break;
			
		/* 查询tdm 板状态*/
		case TDM_STATUS_QUERY:
			GetTdmCardStatus( tdmCardIndex);
			break;
#if 0
		/* tdm 升级APP 结束*/
		case TDM_APP_LOAD_COMP:
			result = MsgBuf[3];
			TdmAppUpdateFlag = FALSE;
			if( result != ROK )
				TdmCardStatus = TDMCARD_ERROR ;				
			else if( TdmFpgaUpdateFlag == FALSE )
				TdmCardStatus = TDMCARD_UPDATE_COMP;
			break;

		/* tdm 升级APP 结束*/
		case TDM_FPGA_LOAD_COMP:
			TdmFpgaUpdateFlag = FALSE;
			result = MsgBuf[3];
			if( result != ROK )
				TdmCardStatus = TDMCARD_ERROR ;
			else if(TdmAppUpdateFlag == FALSE) 
				TdmCardStatus = TDMCARD_UPDATE_COMP;
			break;
		
		/* 从tdm 板接收消息处理*/
		case TDM_COMM_RECV_MSG:
			TdmMgmtLowLevelMsgHandler( tdmCardIndex,(unsigned char *)MsgBuf[3], MsgBuf[2]);
			VOS_Free((char *)MsgBuf[3]);
			break;
			
		case TDM_MSG_RECV_CODE:
			/*BufPtr = (TdmRecvMsgBuf *)MsgBuf[3];*/
			TdmMgmtLowLevelMsgHandler( tdmCardIndex,(unsigned char *)MsgBuf[3], MsgBuf[2]);
			tdmCommMsgFree((char *)MsgBuf[3]);
			break;
#endif
		/* tdm 管理定时消息*/
		case TDM_TIMER_OUT:
			OnuSecondMsgHandler();
			break;
	}
	return( ROK );
}

/*  tdm 板卡管理*/

int GetTdmAppVerFromFlash( unsigned char *TdmAppVer )
{
	if( TdmAppVer == NULL ) return( RERROR );
	
	return( ROK );
}

int GetTdmFPGAVerFromFlash(unsigned char *TdmFPGAVer )
{
	if( TdmFPGAVer == NULL ) return( RERROR );
	
	return( ROK );
}
	
unsigned short int   GetTdmCardInsertedAll( )
{
	return OLTMgmt.InsertedTdmCard;
}

int GetTdmCardInserted( int CardIndex)
{
	if(TdmCardSlotRangeCheck(CardIndex)  != ROK )
		return(RERROR);
	
	CardIndex --;
	
	if(( OLTMgmt.InsertedTdmCard & ( 1 << CardIndex )) == ( 1 << CardIndex ) ){ return CARDINSERT;}
	
	return( CARDNOTINSERT );
}

int SetTdmCardInserted( int CardIndex)
{
	if(TdmCardSlotRangeCheck(CardIndex)  != ROK )
		return(RERROR);
	
	CardIndex --;

	OLTMgmt.InsertedTdmCard = OLTMgmt.InsertedTdmCard | ( 1 << CardIndex );
	OLTMgmt.InsertedTdmCardNum ++;

	if(SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) && SYS_LOCAL_MODULE_ISMASTERACTIVE )
		OLTMgmt.IsSystemStart = FALSE;
	else 
		OLTMgmt.IsSystemStart = TRUE;
	TdmActivatedFlag = FALSE;
	return ( ROK );	
}

int SetTdmCardPulled( int CardIndex)
{
	int loopCount;

	if(TdmCardSlotRangeCheck(CardIndex)  != ROK )
		return(RERROR);
	
	CardIndex --;

	OLTMgmt.InsertedTdmCard = OLTMgmt.InsertedTdmCard &( WORD_FF  - ( 1 << CardIndex ) );
	OLTMgmt.InsertedTdmCardNum --;
	TdmActivatedFlag = FALSE;
	TDMStatusCheck = 0;

	for(loopCount =0; loopCount < TDM_FPGA_MAX; loopCount++)
		TdmChipDownloadComp[loopCount] = V2R1_NOTSTARTED;
	return ( ROK );	
}

#ifdef  _DEVICE_MGMT_TDM_
#endif
/* tdm 管理与设备管理消息交互*/
/* 1 tdm  版卡在位*/
void TdmCardInserted( unsigned long CardIndex)
{
	unsigned long aulMsg[4] = {MODULE_TDM_CARD_MGMT, TDM_INSERTED, 0, 0};

	if(SlotCardIsTdmBoard(CardIndex) != ROK )
		{
		sys_console_printf(" slot %d is not tdm card(tdm card insert) \r\n", (CardIndex ));
		return;
		}
	
	aulMsg[2] = CardIndex;

	if( GetTdmCardInserted( CardIndex) == CARDNOTINSERT )
		{
		/*
		ResetTdmCard( CardIndex );
		VOS_TaskDelay(12*VOS_TICK_SECOND);
		*/
		SetTdmCardInserted( CardIndex );
		TDMStatusCheck = 0;
		}
	else {
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf("    tdm(slot%d) is inserted already\r\n", (CardIndex));
		return;
		}
	
	if( g_PonUpdate_Queue_Id  == 0 ){
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id\r\n" );*/
		return;
		}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK ){
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err\r\n"  );*/
		}
	
}

/*****************************************************
 *
 *    Function: DownloadTdmFpgaImage( unsigned long CardIndex, unsigned char *fpga_in, unsigned char *fpga_out)
 *
 *    input Param:   
 *                 unsigned long CardIndex -- the slot for the tdm card
 *                 unsigned char *fpga_in -- 指定将要加载的TDM FPGA (用1表示); 若其中的值都为0(未指定),
 *                                                         则加载TDM 板上所有FGPA
 *    output param:
 *                 unsigned char *fpga_out -- 返回加载失败的FPGA;
 *
 *    Desc:   
 *
 *    Return:  ROK -- TDM板上FPGA 加载成功
 *                 RERROR -- TDM 板上有FPGA加载失败
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/ 
STATUS DownloadTdmFpgaImage( unsigned long CardIndex, unsigned char *fpga_in, unsigned char *fpga_out)
{
	STATUS ret;
	unsigned char  fpga[TDM_FPGA_PORT_MAX]={0};
	/*unsigned char  ret_fpga[TDM_SG_PORT_MAX]={0};*/
	unsigned char  fpga_flag;
	unsigned char Count;
	unsigned char timecount=20;
	unsigned char  Ok_flag= 0, Err_flag = 0;

	TdmFPGADownloadComp *TdmCardFPGALoadingComp=NULL;

	if(fpga_out == NULL ) return(RERROR);
	for(Count=0; Count<TDM_FPGA_MAX; Count++)
		fpga_out[Count] = ROK;
	
	/*  通过设备管理，获取当前I2C 中记录的FPGA , 或只加载参数中指定的FPGA */
	fpga_flag = 0;
	if( fpga_in != NULL )
		{
		for(Count=0; Count<TDM_FPGA_MAX; Count++)
			{
			fpga[Count] = fpga_in[Count];
			if(fpga[Count] == TDM_FPGA_EXIST )
				fpga_flag++;
			}
		}
	if( fpga_flag == 0 )
		{
		for(Count=0; Count<TDM_FPGA_MAX; Count++)
			{
			fpga[Count] = getTdmChipInserted((CardIndex-1) ,Count);
			if(fpga[Count] ==  TDM_FPGA_EXIST )
				fpga_flag++;
			}
		}
	if(fpga_flag == 0 )
		{
		sys_console_printf("\r\nslot%d: no tdm fpga inserted\r\n", (CardIndex));
		return(ROK);
		}

	TdmFpgaLoadingComp = FALSE;
	TdmCardStatus = TDMCARD_LOADING;
	ret = tdmLoadFpga( ENUM_CALLTYPE_SYN, 0, fpga[0],fpga[1],fpga[2] );

	/* 消息发送错误*/
	if( ret != VOS_OK )
		{
		sys_console_printf("\r\n  send msg to download TDM fpga image err\r\n");
		for(Count=0; Count<TDM_FPGA_MAX;Count++)
			fpga_out[Count] = fpga[Count];
		TdmCardStatus = TDMCARD_ERROR;
		TdmFpgaLoadingComp = FALSE;
		return(RERROR);
		}

	/*  等待fpga image 加载结果 */
	while(( TdmFpgaLoadingComp == FALSE ) &&( timecount > 0 ))
		{
		VOS_TaskDelay(VOS_TICK_SECOND);
		timecount --;
		}

	/* 没有等到消息响应*/
	if(( TdmFpgaLoadingComp == FALSE )  || ( timecount == 0 ))
		{
		sys_console_printf("\r\n Download Firmware image to tdm(slot%d) fpga err, no rsp msg from tdm\r\n", (CardIndex));
		for(Count=0; Count<TDM_FPGA_MAX;Count++)
			fpga_out[Count] = fpga[Count];
		TdmCardStatus = TDMCARD_ERROR;
		TdmFpgaLoadingComp = FALSE;
		return(RERROR);
		}

	/* 收到的响应消息指示错误*/
	TdmCardFPGALoadingComp = ( TdmFPGADownloadComp * )TdmMsgBuf;
	if(TdmCardFPGALoadingComp->MsgCode  !=  0 )
		{
		sys_console_printf("\r\n  Download Firmware image to tdm(slot%d) fpga ", (CardIndex));
		for(Count =0; Count <TDM_FPGA_MAX; Count++)
			{
			if(fpga[Count] == TDM_FPGA_EXIST)
				sys_console_printf("%d ",(Count+1));
			}
		sys_console_printf("... err\r\n");
		for(Count=0; Count<TDM_FPGA_MAX;Count++)
			fpga_out[Count] = fpga[Count];
		TdmCardStatus = TDMCARD_ERROR;
		TdmFpgaLoadingComp = FALSE;
		return(RERROR);
		}

	/* 收到的响应消息指示正确; 但需要判断FPGA 是否全部加载成功*/
	fpga_flag = 0;
	for(Count=0; Count<TDM_FPGA_MAX; Count++)
		{
		if( TdmCardFPGALoadingComp->Result[Count] != ROK )
			{
			fpga_out[Count] = TDM_FPGA_EXIST;
			fpga_flag ++;
			}
		if((fpga[Count] == TDM_FPGA_EXIST) && (fpga_out[Count] == TDM_FPGA_EXIST))
			Err_flag ++;
		if((fpga[Count] == TDM_FPGA_EXIST) && (fpga_out[Count] == ROK ))
			Ok_flag ++;
		}
	
	/* 有fpga 加载成功*/
	if((Ok_flag != 0) ||(fpga_flag == 0 ))
		{
		sys_console_printf("\r\n  Download Firmware image to tdm(slot%d) fpga ", (CardIndex));
		for(Count=0; Count<TDM_FPGA_MAX; Count++)
			{
			if((fpga[Count] ==TDM_FPGA_EXIST ) && (fpga_out[Count] == ROK ))
				{
				sys_console_printf("%d ", (Count+1));
				TdmChipDownloadComp[Count] = V2R1_STARTED;
				}
			}
		sys_console_printf("... ok\r\n");
		}
	
	/* fpga 全部加载成功,返回ROK */
	if(fpga_flag == 0 )
		{
		TdmCardStatus = TDMCARD_LOADCOMP;
		return(ROK );
		}
	else if(Err_flag != 0)
		{			
		sys_console_printf("\r\n  Download Firmware image to tdm(slot%d) fpga ", (CardIndex));
		for(Count =0; Count < TDM_FPGA_MAX; Count++)
			{
			if(fpga_out[Count] == TDM_FPGA_EXIST )
				sys_console_printf("%d ", (Count+1));
			}
		if(TdmFpgaLoadingCounter < TDM_FPGA_LOADING_COUNT)
			sys_console_printf("... err,trying once more\r\n");
		else 
			sys_console_printf("... err\r\n");
		return(RERROR);
		}
	return(ROK);
	
}

void TdmCardInsert( unsigned long CardIndex )
{
	STATUS ret;
	unsigned char TdmAppVerFlash[TDM_APP_VERSION_LEN+1] = "\0";
	unsigned char TdmFPGAVerFlash[TDM_FPGA_VERSION_LEN +1] = "\0";
	unsigned char TdmAppVer[TDM_APP_VERSION_LEN+1] = "\0";
	unsigned char TdmFPGAVer[TDM_FPGA_VERSION_LEN +1] = "\0";
	unsigned short int  AppVerLen=0;
	unsigned short int  FpgaVerLen=0;
	unsigned char loopCount=0;

	unsigned char  fpga[TDM_FPGA_PORT_MAX]={0};
	unsigned char  ret_fpga[TDM_FPGA_PORT_MAX]={0};
	/*unsigned char  fpga_flag;*/

	for(loopCount =0; loopCount < TDM_FPGA_MAX; loopCount++)
		TdmChipDownloadComp[loopCount] = V2R1_NOTSTARTED;
	
	/*ModifyCardNameBySlot(CardIndex);*/
	sys_console_printf("  tdm(slot%d) is to be inserting ...\r\n", (CardIndex));
	/*TdmCardStatus = TDMCARD_NOTLOADING;*/

	/* 1 查询TDM 当前运行版本*/
	loopCount = 0;
	do{
		VOS_TaskDelay(50);
		ret = tdmReadVersion( ENUM_CALLTYPE_SYN, 0, TdmAppVer, &AppVerLen, TdmFPGAVer, &FpgaVerLen );
		loopCount++;
		} while(( ret != VOS_OK ) && ( loopCount < 2));
	
	if( ret != VOS_OK )
		{
		sys_console_printf("\r\nget tdm version err\r\n");
		return;
		}
	
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\nget tdm version ok\r\n");
		sys_console_printf("tdm App version:%s\r\n", TdmAppVer );
		sys_console_printf("tdm FPGA version: %s\r\n",TdmFPGAVer );
		}
	
	/* 2 获取FLASH 中保存的TDM 版本*/
	GetTdmAppVerFromFlash( TdmAppVerFlash );
	GetTdmFPGAVerFromFlash(TdmFPGAVerFlash );

#if 0
	/* 3.1  tdm 板当前fpga 版本与FLASH 中保存版本不一致;
		   发起升级TDM 板fpga 升级
	*/
	if( VOS_MemCmp( TdmFPGAVerFlash, TdmVersionMsg->FpgaVersion, TDM_FPGA_VERSION_LEN ) != 0 )
		{
		TdmFpgaUpdateFlag = TRUE;
		TdmCardStatus = TDMCARD_UPDATEING;
		TdmUpdateAppfile( CardIndex );
		}
	
	/* 3.2  tdm 板当前运行APP 版本与FLASH 中保存版本不一致;
		    发起升级TDM板软件
	*/
	if( VOS_MemCmp( TdmAppVerFlash, TdmVersionMsg->AppVersion, TDM_APP_VERSION_LEN ) != 0 )
		{
		TdmAppUpdateFlag = TRUE;
		TdmCardStatus = TDMCARD_UPDATEING;
		TdmUpdateFpgafile(CardIndex);
		}

	/* 3.3 如果版本不一致，则升级*/
	if( (TdmFpgaUpdateFlag == TRUE ) || (TdmAppUpdateFlag == TRUE))
		return;
	
#endif

	/* 4 downloading tdm fpga */
	TdmFpgaLoadingCounter = 0;
	do{
		TdmFpgaLoadingCounter ++;
		ret = DownloadTdmFpgaImage(CardIndex, fpga, ret_fpga);
		if( ret != ROK )
			{
			for(loopCount =0; loopCount < TDM_FPGA_MAX; loopCount++)
				fpga[loopCount] = ret_fpga[loopCount];
			}
		}while((ret != ROK) && (TdmFpgaLoadingCounter < TDM_FPGA_LOADING_COUNT));

	TdmFpgaLoadingCounter = 0;

	return;

}

extern STATUS tdmBoardAlarmStatus_update( ULONG slotno );	/* added by xieshl 20080202 */

/* 2 TDM板卡不在位*/
void TdmCardPulled ( unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_TDM_CARD_MGMT, TDM_PULLED, 0, 0};

	tdmBoardAlarmStatus_update( CardIndex );	/* added by xieshl 20080202 */
	tdmQueueFlush();
	tdm_interface_update_for_card_pull( CardIndex );		/* added by xieshl 20080219 */
	TdmCardStatus = TDMCARD_UNKNOWN;
	
	aulMsg[2] = CardIndex;
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err\r\n"  );*/
		}
}

void TdmCardPull(unsigned long CardIndex)
{
	/* 1:  复位TDM  */
	sys_console_printf("  tdm(slot%d) is to be pulled\r\n", (CardIndex));
	TdmCardStatus = TDMCARD_UNKNOWN;
	/*tdmReset( ENUM_CALLTYPE_NOACK, 0 );
	TdmReset( CardIndex ); */
	
	/*2: 删除voice 相关MAC 地址及vlan 配置*/
	/*DeleteVoiceVlanByNameAll(CardIndex );*/
	DeleteTdmMacFromSwForTDMPull(CardIndex);

	/*3: PON口对应的ETH口隔离*/
	SetEthPortTdmIsolateAll(CardIndex, TDM_ETH_FILTER_DENY);
	
	/*4:  */
	SetTdmCardPulled(CardIndex);

}

/* 3 复位TDM板*/
void TdmCardReset ( unsigned long CardIndex)
{
	unsigned long aulMsg[4] = { MODULE_TDM_CARD_MGMT, TDM_RESET, 0, 0};
	TdmCardStatus = TDMCARD_UNKNOWN;
	aulMsg[2] = CardIndex;
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err\r\n"  );*/
		}
}
extern LONG bms_board_reset( ULONG slotno, struct vty* vty );
void hwTdmCardReset ( unsigned long CardIndex )
{
	TdmCardStatus = TDMCARD_UNKNOWN;
	bms_board_reset(CardIndex,0);
}
void ResetTdmCard ( unsigned long CardIndex )
{
	/* 1: reset TDM card */
	sys_console_printf("  tdm(slot%d) is reset\r\n", (CardIndex));
	TdmCardStatus = TDMCARD_UNKNOWN;
	/* TdmReset( CardIndex ); */
	tdmReset( ENUM_CALLTYPE_NOACK, 0, 0 );

	/*2: 删除voice 相关MAC 地址及vlan 配置*/
	/*DeleteVoiceVlanByNameAll(CardIndex );*/
	DeleteTdmMacFromSwForTDMPull(CardIndex);

	/*3: PON口对应的ETH口隔离*/
	SetEthPortTdmIsolateAll(CardIndex, TDM_ETH_FILTER_DENY);
	
	/*4:  */
	SetTdmCardPulled(CardIndex);
}

/* 4 激活TDM板*/
void TdmCardActivated(unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_OLT, TDM_ACTIVATE, 0, 0};
	
	aulMsg[2] = CardIndex;

	if(SlotCardIsTdmBoard(CardIndex) != ROK )
	/*if(__SYS_MODULE_TYPE__(CardIndex) != MODULE_E_GFA_SIG )*/
		{
		sys_console_printf(" slot %d is not tdm card(tdm activated) \r\n", (CardIndex));
		return;
		}
#if 0
	if( TdmActivatedFlag == TRUE ) return;
	
	 TdmActivatedFlag = TRUE;
#endif

	if( TdmActivatedFlag == FALSE )
		{		
		if(TDM_MGMT_DEBUG == V2R1_ENABLE)
			sys_console_printf("restore config data to %s board\r\n", GetGFA6xxxTdmNameString());
		epon_tdm_cfgdata_retrieve();
		
		/* 以后有新的tdm 板卡类型，从这里扩展*/

		tdm_interface_update_for_card_insert( CardIndex );	/* added by xieshl 20080219 */
		TdmActivatedFlag = TRUE;
		}

	if( g_PonUpdate_Queue_Id  == 0 ){
		VOS_ASSERT(0);
		sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id(TdmCardActivated())\r\n" );
		return;
		}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err(TdmCardActivated())\r\n" );*/
		}

}

/* 5 tdm板状态查询: TDM 板变为运行状态*/
int  GetTdmCardStatus(  unsigned long CardIndex )
{
	short int port;

	sys_console_printf("  tdm(slot%d) is to be activating ... ok\r\n", (CardIndex));
	DeleteTdmMacFromSwForTDMPull(CardIndex);
	SetEthPortTdmIsolateAll(CardIndex, TDM_ETH_FILTER_PERMIT );
	RestoreTdmBoardMacAndVlan(CardIndex);		

	/* 以下程序用于保证若TDM 槽位发生变化，那么在新的槽位中，
	     删除可能有的ONU 配置数据*/
	for(port = 1; port <= PONPORTPERCARD; port ++)
	{
		short int PonPortIdx;
		short int OnuIdx;
		
		PonPortIdx = GetPonPortIdxBySlot( (short int)CardIndex, port );
		
		if( AllOnuCounter(PonPortIdx) > 0)
		{
			for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
			{
				if( ThisIsValidOnu(PonPortIdx, OnuIdx) ==  ROK )
					DelOnuFromPonPort(PonPortIdx, OnuIdx);
			}
		}						
	}
	
	return( ROK );

}

LONG __tdm_status_callback( UCHAR *pbuf, ULONG size )
{
	tdm_comm_msg_t *pFrm = (tdm_comm_msg_t*)pbuf;
	tdm_pdu_t *pPdu = (tdm_pdu_t*)&pFrm->pdu;
	unsigned char TdmStatus = TDMCARD_UNKNOWN;
	TdmStatus =  pPdu->msgData[0];
	VOS_Free((void *)pbuf);

	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		sys_console_printf("\r\ntdm status %d\r\n",TdmStatus );

	if( TDMStatusCheck >= 2 ) TDMStatusCheck = (TDMStatusCheck >> 1);
	else if(TDMStatusCheck > 0 )  TDMStatusCheck --;
	
	switch (TdmStatus)
		{
		case TDM_INITIALIZING:
			TdmCardStatus = TDMCARD_NOTLOADING;
			break;
		case TDM_LOADING:
			TdmCardStatus = TDMCARD_LOADING;
			break;
		case TDM_LOADED:
			TdmCardStatus = TDMCARD_LOADCOMP;
			break;
		case TDM_RUNNING :			
			if( TdmCardStatus == TDMCARD_LOADCOMP)
				{
				unsigned long aulMsg[4] = {MODULE_TDM_CARD_MGMT, TDM_STATUS_QUERY, 0, 0};

				aulMsg[2] = (unsigned long)TDMCardIdx;
				aulMsg[3] = TDM_RUNNING;

				if( g_PonUpdate_Queue_Id  == 0 )
					{
					VOS_ASSERT(0);
					sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id\r\n" );
					return( RERROR );
					}

				if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
					{
					VOS_ASSERT(0);
					/*sys_console_printf("error: VOS send message err\r\n"  );
					*/
					}
				}
				
			TdmCardStatus = TDMCARD_ACTIVATED;
			break;
		case TDM_UPDATING:
			TdmCardStatus = TDMCARD_UPDATEING;
			break;
		case TDM_ERROR:
			TdmCardStatus = TDMCARD_ERROR;
			break;
		default:
			TdmCardStatus = TDMCARD_UNKNOWN;
		}
	
	return( (int )TdmCardStatus);
	
}

int  TdmCardStatusQuery(unsigned long CardIndex)
{
	unsigned char TdmStatus = TDMCARD_UNKNOWN;
	short int ret;
	/*short int port;*/
	unsigned fpga_flag = 0;
	unsigned char count;
	
	if(SlotCardIsTdmBoard(CardIndex) == ROK )
	/*if(__SYS_MODULE_TYPE__(CardIndex) == MODULE_E_GFA_SIG )*/
		{
		for(count =0; count<TDM_FPGA_MAX; count++)
			{
			if(getTdmChipInserted ( CardIndex-1 , count) == TDM_FPGA_EXIST )
				fpga_flag ++;
			}

		if(fpga_flag == 0 )
			return( TDMCARD_ERROR );
	
		/*ret = tdmStatusQuery( ENUM_CALLTYPE_SYN, 0, &TdmStatus );*/
		ret = tdmStatusQuery( ENUM_CALLTYPE_FUNC, (ULONG)__tdm_status_callback, &TdmStatus );
		TDMStatusCheck++;
		}   
	else return TDMCARD_UNKNOWN;

	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
	sys_console_printf("unanswered status req = %d\r\n",TDMStatusCheck);

	if( TDMStatusCheck <= STATUS_CHECK_MAX )
		return( (int )TdmCardStatus);
	else return( TDMCARD_ERROR);
	
#if 0	
	if( ret != ROK )
		{		
		/*if(TDMStatusCheck < STATUS_CHECK_MAX )*/
			{
			TDMStatusCheck ++;
			}
		/*else  TdmCardStatus = TDMCARD_ERROR;*/
		return( ROK );
		}
	if(( ret == ROK ) && ( TDMStatusCheck != 0 ))
		TDMStatusCheck = 0;
		
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		sys_console_printf("\r\ntdm status %d\r\n",TdmStatus );
	
	switch (TdmStatus)
		{
		case TDM_INITIALIZING:
			TdmCardStatus = TDMCARD_NOTLOADING;
			break;
		case TDM_LOADING:
			TdmCardStatus = TDMCARD_LOADING;
			break;
		case TDM_LOADED:
			TdmCardStatus = TDMCARD_LOADCOMP;
			break;
		case TDM_RUNNING :			
			if( TdmCardStatus == TDMCARD_LOADCOMP)
				{
				sys_console_printf("  tdm(slot%d) is to be activating ... ok\r\n", (TDMCardIdx));
				SetEthPortVoiceIsolateAll(TDMCardIdx, ETH_FILTER_PERMIT );
				RestoreTdmSgBoardVoiceMacVlan(TDMCardIdx);

				/* 以下程序用于保证若TDM 槽位发生变化，那么在新的槽位中，
				     删除可能有的ONU 配置数据*/
				for(port = 1; port <= 4; port ++)
					{
					short int PonPortIdx;
					short int OnuIdx;
					
					PonPortIdx = GetPonPortIdxBySlot( (short int )(get_gfa_tdm_slotno()), port);
					
					if( AllOnuCounter(PonPortIdx) > 0)
						{
						for( OnuIdx = 0; OnuIdx < MAXONUPERPON; OnuIdx ++)
							{
							if( ThisIsValidOnu(PonPortIdx, OnuIdx) ==  ROK )
								DelOnuFromPonPort(PonPortIdx, OnuIdx);
							}
						}						
					}
					
				}
			TdmCardStatus = TDMCARD_ACTIVATED;
			break;
		case TDM_UPDATING:
			TdmCardStatus = TDMCARD_UPDATEING;
			break;
		default:
			TdmCardStatus = TDMCARD_UNKNOWN;
		}
	
	return( (int )TdmCardStatus);
#endif	
}

/* 6  升级tdm 板APP 程序*/
int  TdmUpdateAppfile(unsigned long CardIndex)
{
	sys_console_printf("\r\n  tdm(slot%d) app version is not same, next to update tdm app file\r\n", (CardIndex));
	return( ROK );
}

/* 7  升级tdm 板APP 程序结束*/
int  TdmUpdateAppfileComp(unsigned long CardIndex, unsigned int result )
{
	unsigned long aulMsg[4] = { MODULE_OLT, TDM_APP_LOAD_COMP, 0, 0};

	aulMsg[2] = CardIndex;
	aulMsg[3] = result;

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err(TdmUpdateAppfile)\r\n" );*/
		return( RERROR );
		}

	return( ROK );
}

/* 8  升级tdm 板fpga 程序*/
int  TdmUpdateFpgafile(unsigned long CardIndex)
{

	sys_console_printf("\r\n  tdm(slot%d) fpga version is not same, next to update tdm fpga image\r\n", (CardIndex));
	return( ROK );
}

/* 9  升级tdm 板fpga 程序结束*/
int  TdmUpdateFpgafileComp(unsigned long CardIndex, unsigned int result )
{
	unsigned long aulMsg[4] = { MODULE_OLT, TDM_FPGA_LOAD_COMP, 0, 0};

	aulMsg[2] = CardIndex;
	aulMsg[3] = result;

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
		{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS send message err(TdmUpdateFpgafile)\r\n" );*/
		return( RERROR );
		}

	return( ROK );
}

/* TDM管理与TDM板之间消息交互*/

#if 0
/* 1 查询tdm 当前版本*/
int  TdmVersionQuery(unsigned long CardIndex )
{
	
	int ret = VOS_ERROR;
	unsigned short int pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;
	unsigned char calltype ;
	unsigned long callnotifier =g_PonUpdate_Queue_Id;

	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardVersionQueryMsg)\r\n", (CardIndex +1));
		return (RERROR );
		}
	*/
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_READVER );

	calltype = ENUM_CALLTYPE_MSG;
	ret = tdmCommSendMsg(calltype , callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen );

	if( ret != VOS_OK)
		{		
		sys_console_printf("tdm: send Msg to read tdm version err %d\r\n",ret );
		return( RERROR );	
		}
	
	TdmMsgRecord[MSG_SUBTYPE_READVER].SendFlag = TRUE;
	TdmMsgRecord[MSG_SUBTYPE_READVER].TimeCount =0;
	
	return (ROK );	
}

/* 2 启动tdm 板FPGA 加载*/
int  TdmFPGADownload(unsigned long CardIndex, unsigned char fpga1, unsigned char fpga2, unsigned char fpga3)
{
	
	int ret = VOS_ERROR;
	unsigned short int  pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;
	unsigned char calltype ;
	unsigned long callnotifier =g_PonUpdate_Queue_Id;

	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardFPGALoadingMsg) \r\n", (CardIndex +1));
		return (RERROR );
		}
	*/
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_LOADFPGA);

	if( fpga1 != 0 )
		pdu->msgData[0]=1;
	else pdu->msgData[0]= 0;
	if( fpga2 != 0 )
		pdu->msgData[1]=1;
	else pdu->msgData[1]=0;
	if( fpga3 != 0 )
		pdu->msgData[2]=1;
	else pdu->msgData[2]=0;

	pdulen+=3;

	calltype = ENUM_CALLTYPE_MSG;
	ret = tdmCommSendMsg(calltype , callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen );

	if( ret != VOS_OK)
		{		
		sys_console_printf("tdm:send Msg to load fpga image err %d\r\n",ret );
		return( RERROR );	
		}

	TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].SendFlag = TRUE;
	TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].TimeCount =0; 
	
	return (ROK );	
}

/* 3 启动TDM板进入工作状态*/
int  TdmRunningEnable(unsigned long CardIndex )
{
	int ret = VOS_ERROR;
	unsigned short int  pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;
	unsigned char calltype ;
	unsigned long callnotifier =g_PonUpdate_Queue_Id;

	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardActivatedMsg) \r\n", (CardIndex +1));
		return (RERROR );
		}
	*/
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RUN);

	calltype = ENUM_CALLTYPE_MSG;
	ret = tdmCommSendMsg(calltype , callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen );

	if( ret != VOS_OK)
		{		
		sys_console_printf("tdm: send Msg to enable tdm err %d\r\n",ret );
		return( RERROR );	
		}	
	return (ROK );	
}

/* 4 复位TDM板 */
int  TdmReset(unsigned long CardIndex)
{
	int ret = VOS_ERROR;
	unsigned short int pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;
	unsigned char calltype ;
	unsigned long callnotifier =g_PonUpdate_Queue_Id;

	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardResetMsg) \r\n", (CardIndex +1));
		return (RERROR );
		}
	*/	
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RESET);

	calltype = ENUM_CALLTYPE_MSG;
	ret = tdmCommSendMsg(calltype , callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen );

	if( ret != VOS_OK)
		{		
		sys_console_printf("tdm: send Msg to reset tdm err %d\r\n",ret );
		return( RERROR );	
		}	
	return (ROK );
}

/* 5 查询TDM板当前运行状态*/
int TdmRunningStatus( unsigned long CardIndex )
{
	int ret = VOS_ERROR;
	unsigned short int pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;
	unsigned char calltype ;
	unsigned long callnotifier =g_PonUpdate_Queue_Id;

	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardStatsuQueryMsg) \r\n", (CardIndex +1));
		return (RERROR );
		}
	*/
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc();
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_QUERY);

	calltype = ENUM_CALLTYPE_MSG;
	ret = tdmCommSendMsg(calltype , callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen );

	if( ret != VOS_OK)
		{		
		sys_console_printf("tdm:send Msg to read tdm version err %d\r\n",ret );
		return( RERROR );	
		}

	TdmMsgRecord[MSG_SUBTYPE_QUERY].SendFlag = TRUE;
	TdmMsgRecord[MSG_SUBTYPE_QUERY].TimeCount =0; 
	
	return (ROK );	
}


/* 6 处理TDM 板软件版本信息*/
int  TdmVersionHandler(unsigned long CardIndex, unsigned char *VersionInfo, unsigned int length)
{
	TdmSoftVersion *TdmVersionMsg;
	unsigned char TdmAppVerFlash[10] = "\0";
	unsigned char TdmFPGAVerFlash[20] = "\0";
	unsigned char timeCount;

	TdmVersionMsg = (TdmSoftVersion *)VersionInfo;
	if( VersionInfo == NULL )
		return( RERROR );
	/*
	if( length !=  sizeof( TdmSoftVersion))
		{
		sys_console_printf("the length(=%d) of status msg from TDM is err\r\n", length);
		return( RERROR );
		}	
	*/
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\nRecv tdm TDM version Ack msg \r\n");
		sys_console_printf("reserved: %d\r\n",TdmVersionMsg->Reserved );
		sys_console_printf("msgtype: %d\r\n",TdmVersionMsg->MsgType );
		sys_console_printf("msgSubType: %d\r\n",TdmVersionMsg->MsgSubType );
		sys_console_printf("msgCode: %d\r\n", TdmVersionMsg->MsgCode );
		VOS_MemCpy(TdmAppVerFlash,TdmVersionMsg->AppVersion,TDM_APP_VERSION_LEN );
		VOS_MemCpy(TdmFPGAVerFlash,TdmVersionMsg->FpgaVersion,TDM_FPGA_VERSION_LEN );
		sys_console_printf("App version:%s\r\n", TdmAppVerFlash );
		sys_console_printf("FPGA version: %s\r\n",TdmFPGAVerFlash );
		}
		
	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardVersionHandler)\r\n", (CardIndex +1));
		return (RERROR );
		}
	*/

	/*timeCount = TdmMsgRecord[MSG_SUBTYPE_READVER].TimeCount;*/
	if(TdmVersionMsg->MsgCode != TDM_MSG_OK )
		{
		sys_console_printf("the Ack code(=%d) of status msg from TDM is err\r\n", TdmVersionMsg->MsgCode);
		/*TdmVersionQuery( CardIndex );
		TdmMsgRecord[MSG_SUBTYPE_READVER].TimeCount = timeCount;*/
		return( RERROR );
		}

	TdmMsgRecord[MSG_SUBTYPE_READVER].SendFlag = FALSE;
	TdmMsgRecord[MSG_SUBTYPE_READVER].TimeCount =0; 

	GetTdmAppVerFromFlash( TdmAppVerFlash );
	GetTdmFPGAVerFromFlash(TdmFPGAVerFlash );
#if 0
	/*    tdm 板当前fpga 版本与FLASH 中保存版本不一致;
		   发起升级TDM 板fpga 升级
	*/
	if( VOS_MemCmp( TdmFPGAVerFlash, TdmVersionMsg->FpgaVersion, TDM_FPGA_VERSION_LEN ) != 0 )
		{
		TdmFpgaUpdateFlag = TRUE;
		TdmCardStatus = TDMCARD_UPDATEING;
		TdmUpdateAppfile( CardIndex );
		}
	
	/*   tdm 板当前运行APP 版本与FLASH 中保存版本不一致;
		    发起升级TDM板软件
	*/
	if( VOS_MemCmp( TdmAppVerFlash, TdmVersionMsg->AppVersion, TDM_APP_VERSION_LEN ) != 0 )
		{
		TdmAppUpdateFlag = TRUE;
		TdmCardStatus = TDMCARD_UPDATEING;
		TdmUpdateFpgafile(CardIndex);
		}
#endif
	/* 如果版本一致，则启动FPGA加载*/
	if( (TdmFpgaUpdateFlag == FALSE ) && (TdmAppUpdateFlag == FALSE ))
		{
		unsigned fpga1, fpga2, fpga3, flag=0;
		/* 1 通过设备管理，获取当前I2C 中记录的FPGA */
		fpga1 = getTdmChipInserted ( CardIndex , 0);
		fpga2 = getTdmChipInserted ( CardIndex , 1);
		fpga3 = getTdmChipInserted ( CardIndex , 2);
		
		/* 2 通过板间通道，向TDM 板发送FPGA 加载命令*/
		if(( fpga1==1) || ( fpga2==1) || ( fpga3==1))
			{
			sys_console_printf("\r\n  tdm(slot%d) download image to fpga ", (CardIndex +1));
			if( fpga1 == 1 )sys_console_printf("1 ");
			if( fpga2 == 1 ) sys_console_printf("2 ");
			if( fpga3 == 1 ) sys_console_printf("3 ");
			sys_console_printf("...\r\n");
			}
		else {
			sys_console_printf("\r\n  tdm(slot%d) no fpga to be inserted\r\n",(CardIndex +1));			
			}
		
		TdmFPGADownload( CardIndex, fpga1, fpga2, fpga3);
		TdmFpgaLoadingCounter ++;
		TdmCardStatus = TDMCARD_LOADING;
		}

	return( ROK );
}

/* 7 处理TDM 板FPGA加载命令确认消息*/
int TdmFPGALoadingAckHandler(unsigned long CardIndex, unsigned char *LoadFpgaAckInfo, unsigned int length)
{
	TdmCommGeneralMsg *TdmCardFPGALoadingAck;

	TdmCardFPGALoadingAck = ( TdmCommGeneralMsg * )LoadFpgaAckInfo;
	if( LoadFpgaAckInfo == NULL )
		return( RERROR );
	/*
	if( length !=  sizeof( TdmCommGeneralMsg))
		{
		sys_console_printf("the length(=%d) of FGPAloadingAck msg from TDM is err\r\n", length);
		return( RERROR );
		}
	*/
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\nRecv tdm FPGA loading Ack msg \r\n");
		sys_console_printf("reserved: %d\r\n",TdmCardFPGALoadingAck->Reserved );
		sys_console_printf("msgtype: %d\r\n",TdmCardFPGALoadingAck->MsgType );
		sys_console_printf("msgSubType: %d\r\n",TdmCardFPGALoadingAck->MsgSubType );
		sys_console_printf("msgCode: %d\r\n", TdmCardFPGALoadingAck->MsgCode );
		}
		
	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardFPGALoadingAckHandler)\r\n", (CardIndex +1));
		return (RERROR );
		}
	*/

	if(TdmCardFPGALoadingAck->MsgCode != TDM_MSG_OK )
		{
		sys_console_printf("the Ack code(=%d) of FGPAloadingAck msg from TDM is err\r\n", TdmCardFPGALoadingAck->MsgCode);
		return( RERROR );
		}

	TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].SendFlag = FALSE;
	TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].TimeCount =0; 
	
	return( ROK );
}

	
/* 8 处理TDM 板FPGA加载完成消息*/
int TdmFPGALoadingCompHandler(unsigned long CardIndex, unsigned char *LoadFpgaCompInfo, unsigned int length)
{
	TdmFPGADownloadComp *TdmCardFPGALoadingComp;
	unsigned char fpga1=0;
	unsigned char fpga2=0;
	unsigned char fpga3=0;
	
	TdmCardFPGALoadingComp = ( TdmFPGADownloadComp * )LoadFpgaCompInfo;

	if( LoadFpgaCompInfo == NULL )
		return( RERROR );
	/*
	if( length !=  sizeof( TdmFPGADownloadComp))
		{
		sys_console_printf("the length(=%d) of FGPAloadingComp msg from TDM is err\r\n", length);
		return( RERROR );
		}
	*/
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\nRecv tdm FPGA loading complete msg \r\n");
		sys_console_printf("reserved: %d\r\n",TdmCardFPGALoadingComp->Reserved );
		sys_console_printf("msgtype: %d\r\n",TdmCardFPGALoadingComp->MsgType );
		sys_console_printf("msgSubType: %d\r\n",TdmCardFPGALoadingComp->MsgSubType );
		sys_console_printf("msgCode: %d\r\n", TdmCardFPGALoadingComp->MsgCode );
		sys_console_printf("loading result: %d,%d,%d\r\n",TdmCardFPGALoadingComp->Result[0] ,TdmCardFPGALoadingComp->Result[1],TdmCardFPGALoadingComp->Result[2]);
		}
	/*	
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardFPGALoadingCompHandler)\r\n", (CardIndex +1));
		return (RERROR );
		}
	*/

	if(TdmCardFPGALoadingComp->MsgCode != TDM_MSG_OK )
		{
		sys_console_printf("the Ack code(=%d) of FGPAloadingComp msg from TDM is err\r\n", TdmCardFPGALoadingComp->MsgCode);
		return( RERROR );
		}

	if( TdmCardFPGALoadingComp->Result[0] != 0 )
		fpga1=1;
	if( TdmCardFPGALoadingComp->Result[1] != 0 )
		fpga2=1;
	if( TdmCardFPGALoadingComp->Result[2] != 0 )
		fpga3=1;

	if(( fpga1 != 0 ) || ( fpga2 != 0 ) || ( fpga3 != 0 ) )
		{ /* 有fgpa 加载不成功，可重试一次*/
		if( TdmFpgaLoadingCounter < TDM_FPGA_LOADING_COUNT )
			{
			sys_console_printf("\r\n  tdm(slot%d) download fpga ",(CardIndex+1));
			if( fpga1 == 1) sys_console_printf("1 ");
			if( fpga2 == 1) sys_console_printf("2 ");
			if( fpga3 == 1) sys_console_printf("3 ");
			sys_console_printf("failed,trying once more\r\n");
			TdmFPGADownload( CardIndex, fpga1, fpga2, fpga3);
			TdmFpgaLoadingCounter ++;
			}
		else {
			sys_console_printf("\r\n  tdm(slot%d) download fpga failed\r\n",(CardIndex+1));
			TdmCardStatus = TDMCARD_ERROR;
			TdmFpgaLoadingCounter = 0;
			TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].SendFlag = FALSE;
			TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].TimeCount =0; 
			}
		}
	else {  /* fgpa 加载成功*/
		sys_console_printf("\r\n  tdm(slot%d) download fpga completed\r\n",(CardIndex+1));
		TdmCardStatus = TDMCARD_LOADCOMP;
		TdmFpgaLoadingCounter = 0;
		TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].SendFlag = FALSE;
		TdmMsgRecord[MSG_SUBTYPE_LOADFPGA].TimeCount =0; 
		}
	
	return( ROK );
}

/* 9 处理TDM 板状态查询响应*/
int  TdmStatusRspHandler(unsigned long CardIndex, unsigned char *StatusRsp, unsigned int length)
{
	TdmStatusRsp *TdmStatusQueryRsp;
	unsigned char timeCount;
	
	TdmStatusQueryRsp = ( TdmStatusRsp * )StatusRsp;

	if( StatusRsp == NULL )
		return( RERROR );
	/*
	if( length !=  sizeof( TdmStatusRsp))
		{
		sys_console_printf("the length(=%d) of FGPAloadingComp msg from TDM is err\r\n", length);
		return( RERROR );
		}
	*/
	if( TDM_MGMT_DEBUG == V2R1_ENABLE )
		{
		sys_console_printf("\r\nRecv tdm status Query response msg \r\n");
		sys_console_printf("reserved: %d\r\n",TdmStatusQueryRsp->Reserved );
		sys_console_printf("msgtype: %d\r\n",TdmStatusQueryRsp->MsgType );
		sys_console_printf("msgSubType: %d\r\n",TdmStatusQueryRsp->MsgSubType );
		sys_console_printf("msgCode: %d\r\n", TdmStatusQueryRsp->MsgCode );
		sys_console_printf("current status: %d\r\n",TdmStatusQueryRsp->status );
		}
	/*
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_SIG)
		{
		sys_console_printf(" slot %d is not tdm card(TdmCardStatusRspHandler)\r\n", (CardIndex +1));
		return (RERROR );
		}
	*/
	if(TdmStatusQueryRsp->MsgCode != TDM_MSG_OK )
		{
		sys_console_printf("the Ack code(=%d) of FGPAloadingComp msg from TDM is err\r\n", TdmStatusQueryRsp->MsgCode);
		return( RERROR );
		}

	sys_console_printf("  tdm board status currently is %d\r\n",TdmStatusQueryRsp->status );

	TdmMsgRecord[MSG_SUBTYPE_QUERY].SendFlag = FALSE;
	TdmMsgRecord[MSG_SUBTYPE_QUERY].TimeCount =0; 

	TdmCardStatusFromTdm = TdmStatusQueryRsp->status;	
	return( ROK );
}

void  TdmMgmtLowLevelMsgHandler( unsigned long CardIndex,unsigned char *pBuf, unsigned int length)
{
	TdmCommGeneralMsg *TdmMsgPtr;
	TdmMsgPtr=(TdmCommGeneralMsg*)pBuf;
	
	if( pBuf  == NULL )
		{
		sys_console_printf("TDM mgmt receive msg from tdm board, but is null\r\n");
		return;
		}
	switch (TdmMsgPtr->MsgSubType )
		{
		case MSG_SUBTYPE_READVER:
			TdmVersionHandler( CardIndex, pBuf, length );
			break;
		case MSG_SUBTYPE_LOADFPGA:
			TdmFPGALoadingAckHandler( CardIndex, pBuf, length );
			break;
		case MSG_SUBTYPE_LOADFPGA_DONE:
			TdmFPGALoadingCompHandler( CardIndex, pBuf, length );
			break;
		case MSG_SUBTYPE_RUN:
			TdmFPGALoadingCompHandler( CardIndex, pBuf, length );
			break;
		case MSG_SUBTYPE_QUERY:
			TdmStatusRspHandler( CardIndex, pBuf, length );
			break;			
		case MSG_SUBTYPE_RESET:
			
			break;
		}

}
#endif

/* tdm管理消息分发*/
void  TdmMgmtLowLevelMsgRecv( unsigned char *pBuf, unsigned int length)
{
	TdmFPGADownloadComp *TdmCardFPGALoadingComp;

	TdmCardFPGALoadingComp = (TdmFPGADownloadComp *)pBuf;
	/*
	if((TdmCardFPGALoadingComp->MsgType ==  TDM_MSG_TYPE_DEVMAN )
		&&(TdmCardFPGALoadingComp->MsgType == MSG_SUBTYPE_LOADFPGA_DONE ))
		*/
		{
		VOS_MemCpy( TdmMsgBuf, pBuf, length );
		TdmMsgLen = length;		
		TdmFpgaLoadingComp = TRUE;
		
		if( TDM_MGMT_DEBUG == V2R1_ENABLE )
			{
			sys_console_printf("\r\nRecv tdm FPGA loading complete msg \r\n");
			sys_console_printf("reserved: %d\r\n",TdmCardFPGALoadingComp->Reserved );
			sys_console_printf("msgtype: %d\r\n",TdmCardFPGALoadingComp->MsgType );
			sys_console_printf("msgSubType: %d\r\n",TdmCardFPGALoadingComp->MsgSubType );
			sys_console_printf("msgCode: %d\r\n", TdmCardFPGALoadingComp->MsgCode );
			sys_console_printf("loading result: %d,%d,%d\r\n",TdmCardFPGALoadingComp->Result[0] ,TdmCardFPGALoadingComp->Result[1],TdmCardFPGALoadingComp->Result[2]);
			}
		}
	VOS_Free((void *)pBuf);

	return ;
}

/* added by xieshl 20080219, GFA-SIG拔插时修改其接口名称 */
/* modified by xieshl 20080526, 根据板类型确定其端口名称修改，问题单6715 */
extern LONG ETH_SetDevNameBySlotPort( UCHAR *pucName, ULONG ulSlot, ULONG ulPort );
VOID tdm_interface_update_for_card_insert( ULONG slotno )
{
	int i;
	if(SlotCardIsTdmBoard(slotno) == ROK )
	/*if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_SIG )*/
	{
		for( i=TDM_FPGA_MIN; i<=TDM_FPGA_MAX; i++ )
			ETH_SetDevNameBySlotPort("tdm", slotno, i );
		ETH_SetDevNameBySlotPort("eth", slotno, i );
	}
}
VOID tdm_interface_update_for_card_pull( ULONG slotno )
{
	int i;
	if(SlotCardIsTdmBoard(slotno) == ROK )
	/*if( __SYS_MODULE_TYPE__(slotno) == MODULE_E_GFA_SIG )*/
	{
		for( i=1; i<=PONPORTPERCARD; i++ )
			ETH_SetDevNameBySlotPort("eth", slotno, i );
	}
}

#endif

#ifdef __cplusplus
}
#endif


