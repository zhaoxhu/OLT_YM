/***************************************************************
*
*						Module Name:  CardHook.c
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
*   Date: 			2006/05/20
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/05/20 |   chenfj          |     create 
**----------|-----------|------------------
** 1 added by chenfj 2007/01/17
**   增加向ONU 注册和定时(每隔30分钟)发送系统时间
** 2  added by chenfj 2007-02-07
**     增加ONU自动配置
** 3 modified by chenfj 2008-7-9
**         增加GFA6100 产品支持
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"
#include "PonEventHandler.h"
#include "../onu/Onu_manage.h"/*for onu swap by jinhl@2013-04-27*/
#include "ctrlchan_def.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "Tdm_apis.h"
#endif
#include "vos_int.h"
#include "Vos_sysmsg.h"
#include "Cdp_pub.h"

/*
typedef struct tQ_Format{
	void* Pointer; 
	int len;
}tQ_Format_s; 
*/

unsigned long g_Olt_Queue_Id = 0 ;
LONG   g_Olt_Task_Id ;
unsigned int Olt_Task_flag = 0;

/*Begin:GPON保活机制增加任务 by jinhl@2016.09.27*/
unsigned long g_Olt_KPALIVE_Queue_Id = 0 ;
LONG   g_Olt_KPALIVE_Task_Id ;
unsigned int Olt_KPALIVE_Task_flag = 0;
/*End:GPON保活机制增加任务 by jinhl@2016.09.27*/
 
unsigned long g_Pon_Queue_Id=0;
LONG   g_Pon_Task_Id;
unsigned int Pon_Task_flag= 0;

unsigned long g_PonUpdate_Queue_Id=0;	
LONG   g_PonUpdate_Task_Id;
unsigned int PonUpdate_Task_flag = 0;

unsigned long g_Onu_Queue_Id = 0;
LONG  g_Onu_Task_Id=0;
unsigned int Onu_Task_flag = 0;

/*add by shixh20100525*/
unsigned long g_Onu_Mgt_Sync_Queue_Id = 0;
LONG  g_Onu_Mgt_Sync_Task_Id=0;
unsigned int Onu_Mgt_Sync_Task_flag = 0;

unsigned long g_Onu_Equ_Queue_Id = 0;
LONG g_Onu_Equ_Task_Id = 0;
unsigned int Onu_Equ_Task_flag = 0;

unsigned long g_LowLevelComm_Queue_Id = 0;
LONG g_LowLevelComm_Task_Id;
unsigned int LowLevelComm_Task_flag = 0;

unsigned long g_Tdm_mgmt_Queue_Id = 0;
LONG  g_Tdm_mgmt_Task_Id = 0;
unsigned int  Tdm_mgmt_Task_flag = 0;
/*
unsigned char CardSlot_s_1[PRODUCT_MAX_TOTAL_SLOTNUM+1][20] = {
	"Gem(slot1)",
   	"Tdm(slot2)",
   	"Sw1(slot3)",
   	"Pon(slot4)",
   	"Pon(slot5)",
   	"Pon(slot6)",
   	"Pon(slot7)",
   	"Pon(slot8)",
   	"PWU(slot9)",
   	"PWU(slot10)",
   	"PWU(slot11)"
	};

unsigned char *CardSlot_s[PRODUCT_MAX_TOTAL_SLOTNUM+1];

unsigned char *CardSlot_s[] = {
	(unsigned char *)"Card Up_GE( slot 1)",
   	(unsigned char *)"Card Up_TDM( slot 2 )",
   	(unsigned char *)"Card Sw1( slot 3 )",
   	(unsigned char *)"Card Sw2/Pon 5( slot 4 )",
   	(unsigned char *)"Card Pon4( slot 5)",
   	(unsigned char *)"Card Pon3( slot 6)",
   	(unsigned char *)"Card Pon2( slot 7 )",
   	(unsigned char *)"Card POn1( slot 8 )",
   	(unsigned char *)"Card PWU1( slot 9 )",
   	(unsigned char *)"Card PWU2( slot 10 )",
   	(unsigned char *)"Card PWU3( slot 11 )"
	};

unsigned char V2r1TimeOutMsg[16]={0};
*/

extern LONG Onustats_Init(void);
extern int Receive_polling_thread_func1(void *lpPacketRecBuf,unsigned int buf_data_size);
extern int Receive_ponframe_thread_func1(short int olt_id, void *lpPacketRecBuf,unsigned int buf_data_size);
extern int Receive_ponframe_thread_func2(short int olt_id, void *lpPacketRecBuf,unsigned int buf_data_size);
extern int Receive_ponframe_thread_func3(short int olt_id, void *lpPacketRecBuf,unsigned int buf_data_size);
extern short int Get_olt_id_by_mac_address ( const mac_address_t  mac_address );

extern int StatsInit(void);

extern void  init_gwEponMib(void);
extern void  init_gwEponDevMib(void);
extern void  init_ftpManMib(void);
extern void  init_gwSecurityMib(void);
extern void  init_eponMonMib(void);
#ifdef __CTC_TEST
extern void init_voipPortGroup();
#endif
extern void vInitProductMib();
extern LONG EPON_V2R1_CliInit();
extern VOID OnuModule_init();

extern VOID  setup_tree();
extern void  init_gwEponMib(void);
extern void  init_gwEponDevMib(void);
extern void  init_ftpManMib(void);
extern void  init_gwSecurityMib(void );
extern void  init_ethPortMIB(void );
extern short int CommOamMagInfoInit(void );
extern void  init_gwIgmpSnoopAuth(void);
/** Initializes the gwIgmpSnoopAuthTabObj module */
extern void init_ctcIgmpSnoopAuthTabObj(void);
extern void  init_eponMonMib(void);
extern void  init_eponOnuMib(void);
extern long  funOamTransFileInit(void);
extern void  init_gwEponExtMib();
extern void init_gwEponFill();
extern long OltPtyInit(void);
extern void vMnOamInit();
extern void cl_pty_relay_init();

extern int  StatsInit(VOID);
extern short int  monInit(void);
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
extern int  OnuLoopInit(void);
#endif

extern int eventProcInit(void);
extern int initAlarmOam();
extern int CT_RMan_Init();

extern void tdm_recv_frame_handler( const void *pFrame, UINT length );
extern int bcm5325_receive_pkt_callback(ULONG portno, ULONG recv_flag, UCHAR * recv_buf, ULONG recv_len);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
extern void init_eponTdmMib(void);
#endif
extern void gwFtpTask(void);
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
extern int onuAutoLoadInit();
extern void init_eponAutoLoadFtp(void);
#endif
#if ( EPON_MODULE_QOS_ACL_QINQ_MIB == EPON_MODULE_YES )
extern void init_gwEponQinQMib();
extern void init_gwAclMib();
extern void init_gwQosMib();
#endif

extern LONG led_init( void );
extern int InitOltMgmtTableByDefault();
extern int InitPonChipMgmtInfoAll();
extern void ONUmgt_API_Init();
extern STATUS funIgmpTvmInit(void);
extern void PonHotSwapInit();
extern int OnuStatusSyncTimerStart();
extern int onuAgingTimerStart();
extern int ScanOfflineOnuAgingTimer();
extern long funIgmpAuthInit(void);
extern LONG ONU_View_CommandInstall();
extern void PonPowerMeteringInit(void);
extern int ethLoopCheckInit();
extern STATUS onuOamUpdateInit( void );
extern void init_gwUserMgtMib( void );
extern void init_gwEthLoopMib(void);
extern int  InitOnuExtMgmtTable( );


extern LONG OnuMgtSync_OnuStatusReport();
extern LONG OnuMgtSyncDataSend_Register( short int PonPortIdx, short int OnuIdx );
extern int recv_onuSyncMessage(onu_sync_msg_head_t  *pOnuSynData);
extern int CTC_GetOnuDeviceInfo( short int PonPortIdx, short int OnuIdx );
extern void PonGeneralOamDebugInit();
extern VOID ScanPonPortInit();

int  SYS_INIT_COMP = FALSE;

/*  onu timer for test 
void Clr()
{
	VOS_MemSet( V2r1TimeOutMsg, 0,16 );
}

void disp()
{
	int i;

	sys_console_printf("\r\n the data is :");
	for(i=0;i<16;i++){
		sys_console_printf(" %02x", V2r1TimeOutMsg[i]);
		}
	sys_console_printf("\r\n");
}

 end */

typedef int (* PF_RECV_PONFRAME_FUNC)(short int olt_id, void *lpPacketRecBuf,unsigned int buf_data_size);

static PF_RECV_PONFRAME_FUNC s_pfLowLevelReceivePonframe;

#if 0
#define  RecvMsgNumMAX  1000
#define  RecvMsgLengthMAX  2000
unsigned char RevMsg[RecvMsgNumMAX][RecvMsgLengthMAX];
unsigned char (*RevMsg)[RecvMsgLengthMAX];
#else
/* modified by xieshl 20120512, 针对青岛长宽和上海鹏博士发生的部分ONU不注册的问题，怀疑跟
    PON底层通信有关，扩大缓存队列，修改最大帧长，增加统计功能，同时还扩大了
    g_LowLevelComm_Queue_Id长度，防止接收队列溢出造成通信失败 */
#define  RecvMsgNumMAX		MAXLOWLEVELMSGNUM					/* 1000 */
#define  RecvMsgLengthMAX	PON_MAX_HW_ETHERNET_FRAME_SIZE	/* 1600 */
typedef struct {
	UCHAR state;
	UCHAR reserve[3];
	UCHAR buf[RecvMsgLengthMAX];
}low_rx_msg_t;
low_rx_msg_t *RevMsg = NULL;
ULONG pon_comm_rx_over_size_pkts = 0;
ULONG pon_comm_rx_que_over_drop_pkts = 0;
ULONG pon_comm_rx_task_busy_drop_pkts = 0;
ULONG pon_comm_rx_total_pkts = 0;
ULONG pon_comm_rx_error_pkts = 0;
#endif
/* add by tianzhy */
/*unsigned char ctrlMsg[64][2048];*/	/* removed 20070402 */
/*tQ_Format_s MsgBody;
tQ_Format_s *dest, dest1;*/
int LowLevel_count = 0;
/*int ctrl_count = 0;*//*removed by xieshl 20070402*/
enum{
	src_brd_type_sw = 1,
	src_brd_type_pon,
	src_brd_type_tdm
};

#define MAC_PREFIX_CMP( mac1, mac2 ) ( (mac1[0] == mac2[0]) && (mac1[1] == mac2[1]) && (mac1[2] == mac2[2]) )
/*******************************************************************************************
函数名：	parse_frame_src_board_type
功能：	获取源MAC对应的板卡类型
输入：	收到的数据帧，数据帧长度
返回值：	源板卡的类型，1、SW板  2、PON板 3、TDM板
********************************************************************************************/
int parse_frame_src_board_type( void *lpPacketRecBuf, unsigned int buf_data_size )
{
	const char sw_brd_mac_prefix[5] = "\x00\x05\x3b";
	const char pon_brd_mac_prefix[5] = "\x00\x0c\xd5";
	const char tdm_brd_mac_prefix[5] = "\x00\x01\x0b";
	
	int type_ret = -1;

	if( buf_data_size > 12 )
	{
		char * srcMac = (char*)(lpPacketRecBuf)+6;
		if( MAC_PREFIX_CMP(sw_brd_mac_prefix, srcMac) )
			type_ret = src_brd_type_sw;
		else if( MAC_PREFIX_CMP(pon_brd_mac_prefix, srcMac)  )
			type_ret = src_brd_type_pon;
		else if( MAC_PREFIX_CMP(tdm_brd_mac_prefix, srcMac) )
			type_ret = src_brd_type_tdm;
		else
			type_ret = -1;
	}

	return type_ret;
}


/* add by xieshl 20090609，统计板间管理报文 */
int (*emac1_rx_pkt_hookrtn) (void *,unsigned int ) = NULL;
int Receive_polling_thread_func(void *lpPacketRecBuf,unsigned int buf_data_size)
{

	unsigned long aulMsg[4] = { MODULE_CTRLCHAN, FC_LOWLEVEL_COMM, 0, 0 };
	LONG ret = VOS_OK;
	ULONG ulSlot = 0;
	unsigned char *BoardPkt/*, *temp*/;
	low_rx_msg_t *temp;
	int iRes=-1;
	int srcBrdType = -1;

/* add by xieshl 20090609, 检查是否有长度异常报文，并过滤，如果这些异常报文被送到上层
    ，可能会导致系统死机 */
    srcBrdType = parse_frame_src_board_type( lpPacketRecBuf, buf_data_size );    
    if( emac1_rx_pkt_hookrtn )
    {
	if( (*emac1_rx_pkt_hookrtn)(lpPacketRecBuf, buf_data_size ) == VOS_ERROR )
	{
	    pon_comm_rx_error_pkts++;
	    return VOS_ERROR;
	}
    }

#ifdef __TEST_CTRL_CHANNEL
	if( bcm5325e_mirror_enable )
	{
/*		USHORT swPort = slot_port_2_swport_no( 1, 2 );
		if( swPort >= 0 )*/
		bcm5325e_mirror_to_bcm56300( lpPacketRecBuf, buf_data_size, bcm5325e_recv_mirror_port, 0 );
	}
	bcm5325e_recv_pkts_counter++;
#endif
	
	/*if(buf_data_size > RecvMsgLengthMAX)
	{
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf("\r\n  -----low level receive Msg too long ------\r\n");
		buf_data_size=RecvMsgLengthMAX;
	}*/

	ulSlot = SYS_LOCAL_MODULE_SLOTNO;
	/*srcBrdType = parse_frame_src_board_type( lpPacketRecBuf, buf_data_size );*/

	if( srcBrdType == -1 )
	{
/*		sys_console_printf("\r\nsource mac is: %02X:%02X:%02X:%02X:%02X:%02X", ((char*)lpPacketRecBuf)[6],
				((char*)lpPacketRecBuf)[7],((char*)lpPacketRecBuf)[8],((char*)lpPacketRecBuf)[9],((char*)lpPacketRecBuf)[10],((char*)lpPacketRecBuf)[11]);
		sys_console_printf("\r\nboard type val is : %d\r\n", srcBrdType );

		if(EVENT_DEBUG == V2R1_ENABLE)
		{
			int i;
			sys_console_printf("\r\n");
			for(i=0; i<buf_data_size; i++)
			{
				sys_console_printf("%02x ", ((char *)lpPacketRecBuf)[i] );
				if(( i+1) % 16 == 0 )sys_console_printf("\r\n");
			}
			sys_console_printf("\r\n");
		}*/
		pon_comm_rx_error_pkts++;
		return VOS_ERROR;
	}
			
	/*if ( 3 == ulSlot  &&  *((unsigned char *)(lpPacketRecBuf+11)) != 4) *//* MASTER */
	else if( srcBrdType == src_brd_type_pon )
	{
	    if ( NULL != RevMsg )
          {
           	 pon_comm_rx_total_pkts++;
               if(buf_data_size > RecvMsgLengthMAX)
               {
                   /*if(EVENT_DEBUG == V2R1_ENABLE)
                       sys_console_printf("\r\n -----PON low level rx-oam over size %d\r\n", buf_data_size);*/
                   VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "\r\n ----PON low level rx-oam over size %d\r\n", buf_data_size);
                   /*buf_data_size=RecvMsgLengthMAX;*/
                   pon_comm_rx_over_size_pkts++;
                   return VOS_ERROR;
               }

               /*temp = (unsigned char *)&RevMsg[LowLevel_count];
               VOS_MemCpy(temp, lpPacketRecBuf, buf_data_size );*/
               temp = &RevMsg[LowLevel_count];
               if( temp->state )
                   pon_comm_rx_que_over_drop_pkts++;
               else
                    temp->state = 1;
    	        VOS_MemCpy(temp->buf, lpPacketRecBuf, buf_data_size );

    		aulMsg[2] = buf_data_size;
    		aulMsg[3] = (unsigned long)temp;

    		ret = VOS_QueSend( g_LowLevelComm_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
    	       if( ret !=  VOS_OK )
    	       {
                   /*
                   sys_console_printf("\r\n send  PON msg to LowLevel task err\r\n");
                   ASSERT( 0 );
                   */
                   pon_comm_rx_task_busy_drop_pkts++;
                   return( RERROR );
    	       }	
    		LowLevel_count++;
    		if(LowLevel_count >= RecvMsgNumMAX) LowLevel_count = 0;
           }   
	}
	/*else if ( 3 == ulSlot  &&  *((unsigned char *)(lpPacketRecBuf+11)) == 4) */
	else if( srcBrdType == src_brd_type_sw )
	{
	     /*sys_console_printf("\r\n Master CPU get msg from Slave CPU \n");   */

             BoardPkt= VOS_Malloc(2000,MODULE_FEI);/*&ctrlMsg[ctrl_count][0];*/
		if( BoardPkt )
		{
			/* wangysh add 20101208*/
             VOS_MemZero(BoardPkt,2000);
              temp=(char *)(((char *)BoardPkt)+20);
#if 0
		      if( MODULE_E_GFA6900_SW ==SYS_LOCAL_MODULE_TYPE)
		      {
		          VOS_MemCpy(temp, lpPacketRecBuf,12);
		          VOS_MemCpy(temp+12, lpPacketRecBuf+12+10,buf_data_size-12-10);
		          buf_data_size -= 10;
	          }
	          else
	          {
		          VOS_MemCpy(temp, lpPacketRecBuf,buf_data_size);
	          }
#else
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
              buf_data_size = bcmRecPacketDelTag(lpPacketRecBuf, buf_data_size);  
#endif
	          VOS_MemCpy(temp, lpPacketRecBuf,buf_data_size);
#endif
			iRes=bcm5325_receive_pkt_callback(6,0,BoardPkt,buf_data_size+sizeof(sys_ctrlchan_framehead));
		}
	}
	else if( srcBrdType == src_brd_type_tdm )
	{
		void * pPack = NULL;
		aulMsg[0] = MODULE_TDM_COMM;
		aulMsg[1] = FC_TDM_COMM;
		
		pPack = VOS_Malloc( 2048, MODULE_TDM_COMM );
		if( pPack != NULL )
		{

			VOS_MemSet( pPack, 0, 2048 );
			if( buf_data_size <= 1518 )
				VOS_MemCpy( pPack, lpPacketRecBuf, buf_data_size );
			else
			{
				VOS_Free((void *)pPack);
				if(EVENT_DEBUG == V2R1_ENABLE)
					sys_console_printf( "\r\n tdm-recv:too long frame, %d", buf_data_size );
				return RERROR;
			}
			aulMsg[2] = buf_data_size;
			aulMsg[3] = (ULONG)pPack;

			ret = VOS_QueSend( g_LowLevelComm_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
			
			if( ret !=  VOS_OK )
			{
				VOS_Free((void *)pPack);
				/*sys_console_printf("\r\n send %s msg to LowLevel task err IN %s(%d)\r\n", GetGFA6xxxTdmNameString(), __FILE__, __LINE__);*/
				/*ASSERT( 0 );*/
				
				return( RERROR );
			}
		}
		else
		{
			/*sys_console_printf( "\r\nReceive_polling_thread_func: alloc mem error!" );*/
			return RERROR;
		}
	}

	return( ROK);

}

int Receive_ponframe_thread_func(short int olt_id, void *lpPacketRecBuf, unsigned int buf_data_size)
{
    LONG lRlt;
    unsigned long aulMsg[4] = { MODULE_RPU_PON, FC_LOWLEVEL_COMM, 0, 0 };
    low_rx_msg_t *BufferPkt;

    if ( buf_data_size <= RecvMsgLengthMAX )
    {
        if ( NULL != RevMsg )
        {
            pon_comm_rx_total_pkts++;

            BufferPkt = (low_rx_msg_t *)&RevMsg[LowLevel_count];
            if( BufferPkt->state )
            {
            	/*VOS_ASSERT(0);*/
                pon_comm_rx_que_over_drop_pkts++;
            }
			else
                BufferPkt->state = 1;

            VOS_MemCpy(BufferPkt->buf, lpPacketRecBuf, buf_data_size );

            aulMsg[0] = olt_id;
            aulMsg[2] = buf_data_size;
            aulMsg[3] = (unsigned long)BufferPkt;
            
            lRlt = VOS_QueSend( g_LowLevelComm_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
            if( lRlt != VOS_OK )
            {
                pon_comm_rx_task_busy_drop_pkts++;
				BufferPkt->state = 0;
                /*VOS_ASSERT(0);*/ /* 问题单: 16439 modified by duzhk 该问题主要是因为倒换期间因收到太多管理报文等原因导致
        					CDP通道太忙，参考解指导的意见，注释掉断言*/
                return( RERROR );
            }	
            if(++LowLevel_count >= RecvMsgNumMAX) LowLevel_count = 0;
        }   
    }
    else
    {
         VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "\r\n ----PON low level rx-oam over size %d\r\n", buf_data_size);
         pon_comm_rx_over_size_pkts++;
         return VOS_ERROR;
    }
    return ROK;
}

void V2r1_Init()
{
	/*int i;*/
	/* create the semaphore */
	/*sys_console_printf("\r\nOltEntry: create the data semId for olt and pon chip mgmt \r\n");*/
	if( OltMgmtDataSemId == 0 )
		OltMgmtDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	/*OltMgmtDataSemId = semMCreate(SEM_Q_PRIORITY|SEM_INVERSION_SAFE);  */

	if ( PonChipMgmtDataSemId == 0 ) 
		PonChipMgmtDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );

	/* init the data table */
	sys_console_printf("\r\nInitialize PON management Service ... OK\r\n");

    VOS_MemZero(&PON_init_params, sizeof(PON_init_params));

    VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PON_MANAGER );
    OLT_API_Init();

#if(RPU_MODULE_SSH == RPU_YES)
	sshd_module_Init();  /*added by liub 2017-02-25 */

#endif

#ifdef OLT_SYNC_SUPPORT
    if ( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        /* 不直接管理PON卡的PON管理者，是信息收集者 */
        OLT_SYNC_Collector_Init();
    }
    else
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
        {
            /* 带CPU的PON卡管理者，是信息提供者 */
            OLT_SYNC_Offeror_Init();
        }
        else
        {
            /*VOS_ASSERT( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER );*/
        }
    }
#else
    OLT_SYNC_Init();
#endif

	InitOltMgmtTableByDefault();
	InitPonLLIDTable();
	InitPonChipMgmtInfoAll();
	OltMgmtInfoDefault();

	V2R1_SYS_TIME_PERIOD = (30*60);
	/*V2R1_TimeCounter = 0;*/
	
	RecordOLtSysUpTime();
	RecordSysLaunchTime();
	GetDeviceInfoFromHardware();

	/*VOS_TaskDelay(VOS_TICK_SECOND );

	VOS_MemSet( V2r1TimeOutMsg, 0, 16 );*/

	/* create the semId */
	/*sys_console_printf("\r\nPonEntry: create the data semId for pon and llid mgmt \r\n");*/
	if( PonPortDataSemId == 0 )
		PonPortDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	if( PonLLIDDataSemId == 0 )
		PonLLIDDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );
	if( OnuPendingDataSemId == 0 )
		OnuPendingDataSemId = VOS_SemMCreate( VOS_SEM_Q_PRIORITY );

	/* initialize the user data table */
	/*sys_console_printf("  Initialize PON and ONU mgt data ...\r\n");*/
	
	V2R1_AutoProtect_Timer = V2R1_PON_PORT_SWAP_TIMER;

	if( OnuMgmtTableDataSemId == 0 )
		OnuMgmtTableDataSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );	
	if( OnuEncryptSemId == 0 )
		OnuEncryptSemId = VOS_SemMCreate( VOS_SEM_Q_FIFO );	
	
	ONUmgt_API_Init();/*add by shixh20100518*/
	/* ClearOnuMgmtTable(); */
	InitOnuMgmtTableByDefault();
    
    InitMaxOnuTableByDefault();
	/*PonpowerMeteringInit();*/
    funIgmpTvmInit();
    
    if (SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
    	OnuWaitForGetEUQInit();
        
#if( EPON_MODULE_ONU_REG_FILTER	 == EPON_MODULE_YES )
    	InitOnuRegisterFilterQueue();
#endif

        /* B--added by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )
        PonHotSwapInit(); 
#endif
        /* E--added by liwei056@2010-9-29 for 5.3.12's RedundancyUpgrade */

    	/* zhangxinhui, 2010-12-29, 根据代码REVIEW的结果，在主控板上没必要启动
        	** 定时处理任务
        	*/
        V2r1TimerStart();

		/* added by xieshl 20151022 */
		if( SYS_LOCAL_MODULE_TYPE_IS_16EPON || SYS_LOCAL_MODULE_TYPE_IS_8100_EPON)
			ScanPonPortInit();
    }

    if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
         OnuStatusSyncTimerStart();	/* added by xieshl 20110919, 主控和PON板间：ONU状态定时同步 */
    

	/* added by xieshl 20110520, 自动删除不在线ONU PR11109 */
	/* 目前只在主控上启动，删除时，如果PON板在位则通过远程调用删除PON上的ONU，
	    如果PON板不在位则直接在主控上删除。
	    */
	if( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
		onuAgingTimerStart();
	if(SYS_LOCAL_MODULE_TYPE_IS_8000_GPON || SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		onuupdateTimerStart();

    /*added by luh@2015-6-19.添加针对单个onu的oam调试手段*/
    PonGeneralOamDebugInit();
	
}

/*int flag_tdm = 0;*/
void LowLevelComm()
{
	unsigned long aulMsg[4];
	long result;
	low_rx_msg_t *recv_msg;

	LowLevelComm_Task_flag = 0;

    if ( SYS_LOCAL_MODULE_TYPE_IS_PAS_PONCARD_MANAGER )
    {
#if defined(_EPON_10G_PMC_SUPPORT_)            
        /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
        if(SYS_LOCAL_MODULE_TYPE_IS_PAS_10G_PONCARD_MANAGER)
    	{
    	    s_pfLowLevelReceivePonframe = GW10G_Receive_ponframe_thread_func1;
    	}
		else
#endif
		{
	        s_pfLowLevelReceivePonframe = Receive_ponframe_thread_func1;
		}
    }
    else if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER )
    {
#if defined(_EPON_10G_BCM_SUPPORT_)            
        if ( SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
        {
            s_pfLowLevelReceivePonframe = Receive_ponframe_thread_func3;
        }
        else
#endif
        {
            s_pfLowLevelReceivePonframe = Receive_ponframe_thread_func2;
        }
    }
	#if defined(_GPON_BCM_SUPPORT_) 
	else if ( SYS_LOCAL_MODULE_TYPE_IS_GPON_PONCARD_MANAGER )
	{
		s_pfLowLevelReceivePonframe = NULL;
		LowLevelComm_Task_flag = 1;
	}
	#endif
    else
    {
        VOS_ASSERT(0);
        return;
    }
    
	/*
	dest = &dest1;
	V2R1_lowlevel_comm = msgQCreate( 200,200, MSG_Q_PRIORITY );
	*/
	LowLevelComm_Task_flag = 1;
	
	for(;;)
	{
		result = VOS_QueReceive( g_LowLevelComm_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR ) 
		{
			ASSERT(0);
			continue;
		}
		if( result == VOS_NO_MSG ) continue;
		
		if( aulMsg[1] == FC_LOWLEVEL_COMM )
		{
			recv_msg = (low_rx_msg_t*)aulMsg[3];
			if( recv_msg )
			{
				if ( aulMsg[0] == MODULE_CTRLCHAN )
				{
	    				/*sys_console_printf("\r\n LowLevel recvMsg %d  len %d \r\n", aulMsg[3],aulMsg[2]);*/
	    				/*Receive_polling_thread_func1((void *)aulMsg[3], (unsigned int) aulMsg[2] );*/
					Pon_ReceivePollingThreadFunc1(recv_msg->buf, (unsigned int ) aulMsg[2] );
				}
				else
				{
					if ( OLT_LOCAL_ISVALID(aulMsg[0]) )
					{
						(*s_pfLowLevelReceivePonframe)((short int)aulMsg[0], (void *)recv_msg->buf, (unsigned int) aulMsg[2] );
					}
				}
				recv_msg->state = 0;
			}
		}
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
		else if( aulMsg[1] == FC_TDM_COMM && aulMsg[0] == MODULE_TDM_COMM )
		{
/*			if( flag_tdm == 1 )
			{
			unsigned int i;
			sys_console_printf("\r\n");
			for(i = 0; i< aulMsg[2]; i++)
				{
				sys_console_printf("%02x ", ((unsigned char *)aulMsg[3])[i]);
				if((i+1) % 16 == 0 ) sys_console_printf("\r\n");
				}
			}*/
			
			tdm_recv_frame_handler( (void *)aulMsg[3], aulMsg[2] );
		}
#endif
    }
	
}

/* added by zhangxinhui 2010-12-28 
** To Ensure some messages only be implemented on PON card
*/

extern void  Pon_EncryptCLI();
int __PON_SWAP_DELAY = 50;

/* modified by xieshl 20110614, 更改函数名称OltEntry, PonEntry, PonUpdateEntry, OnuEntry, 避免和某些变量重名，
    特别是变量OnuEntry定义特别多，如果变量忘记声明时后果将不堪设想 */
extern int OnuSwapRegHandler(OnuEventData_s *OnuRegData);
void OltTask()
{
	unsigned long aulMsg[4];
	long result;

	if( Olt_Task_flag )
		return;
	
	Olt_Task_flag = 1;
	
	while( 1 )
	{
		result = VOS_QueReceive( g_Olt_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
        {
			VOS_ASSERT(0);
			/*sys_console_printf("error: receiving msg from olt Que failure(OltTask())\r\n" );*/
			VOS_TaskDelay(20);
			continue;/*return;*/
		}
        
		if( result == VOS_NO_MSG ) continue;

        switch(aulMsg[1])
        {
            case FC_V2R1_TIMEOUT:
            PON_BOARD_ENSURE
            {
    			V2r1TimerHandler();
            }
            break;
            case FC_UPDATE_ONU_APP_IMAGE:   /* ONU app image 通过PMC私有通道升级*/
            PON_BOARD_ENSURE 
            {
    			short int PonPortIdx, OnuIdx;
    			unsigned char OnuTypeString[20] = {'\0'};
    			fileName_t  *DataBuf;
    			int length;

    			DataBuf = (fileName_t *)aulMsg[3];

    			PonPortIdx = DataBuf->PonPortIdx ;
    			OnuIdx = DataBuf->OnuIdx;
    			
    			length =  DataBuf->file_len;
    			if( length > 16 ) length = 16;
    			VOS_MemCpy( OnuTypeString, (unsigned char *)DataBuf->file_name, length );

    			UpdateOnuFileImage(PonPortIdx, OnuIdx, OnuTypeString, length );
    			
    			VOS_Free( (void *)DataBuf );
            }
            break;
            case FC_START_ENCRYPT:
            case FC_STOP_ENCRYPT:
            {
    			short int PonPortIdx, OnuIdx;
    			
    			OnuIdx = (short int )aulMsg[3] & 0xffff;
    			PonPortIdx = (short int )(aulMsg[3] >> 16);
    			
    			/*sys_console_printf(" onu %d/%d encrypt direction-%s\r\n", PonPortIdx, OnuIdx, v2r1EncryptDirection[aulMsg[2]]);*/
    			
    			OnuEncrypt(PonPortIdx, OnuIdx, (unsigned int)aulMsg[2]);
            }
            break;
            case FC_ENCRYPT_KEY:
            PON_BOARD_ENSURE 
            {
    			short int PonPortIdx, OnuIdx;
    			
    			OnuIdx = (short int )aulMsg[3] & 0xffff;
    			PonPortIdx = (short int )(aulMsg[3] >> 16);
    			
    			/*sys_console_printf(" onu %d/%d update encrypt key\r\n", PonPortIdx, OnuIdx);*/
    			UpdateOnuEncryptKey(PonPortIdx, OnuIdx);
            }
            break;
            case FC_STARTENCRYPTION_COMP:
            PON_BOARD_ENSURE 
            {
    			OnuStartEncryptionInfo_S *OnuStartEncryptionData;
    			PON_olt_id_t olt_id;
    			short int OnuIdx;
    			PON_onu_id_t llid;
    			PON_start_encryption_acknowledge_codes_t return_code;

    			OnuStartEncryptionData = (OnuStartEncryptionInfo_S *)aulMsg[3];

    			if( OnuStartEncryptionData == NULL ) continue;
    			
    			olt_id = OnuStartEncryptionData->olt_id;
    			llid = OnuStartEncryptionData->llid;
    			return_code = OnuStartEncryptionData->ret_code;
    			VOS_Free( (void *)OnuStartEncryptionData );
    			
    			OnuIdx = GetOnuIdxByLlid(olt_id, llid);
    			if(OnuIdx == RERROR )
    			{
    				continue;
    			}
    				
    			if(( EVENT_ENCRYPT == V2R1_ENABLE ) && ( return_code != ROK))
    			{
    				sys_console_printf("\r\nStart Encryption\r\n");
    				sys_console_printf("    onu%d/%d/%d %s\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1), StartEncryptRetCode[return_code]);
    			}
    			
    			if(return_code != ROK)
    			{
    				PON_llid_parameters_t  llid_parameters;	
    				VOS_MemSet(&llid_parameters,0, sizeof(PON_llid_parameters_t));
					#if 1/*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
					OnuMgt_GetLLIDParams( olt_id, OnuIdx, &llid_parameters );
					if(llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)
    				{
    					if(EVENT_ENCRYPT == V2R1_ENABLE)
    						sys_console_printf("onu%d/%d/%d encrypt error, once more\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1));
    					OnuMgt_StartEncryption( olt_id, OnuIdx, &( OnuMgmtTable[olt_id * MAXONUPERPON + OnuIdx].EncryptDirection ) );					
    				}
					#else
    				PAS_get_llid_parameters( olt_id, llid, &llid_parameters );
    				if(llid_parameters.encryption_mode == PON_ENCRYPTION_DIRECTION_NO_ENCRYPTION)
    				{
    					if(EVENT_ENCRYPT == V2R1_ENABLE)
    						sys_console_printf("onu%d/%d/%d encrypt error, once more\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1));
    					PAS_start_encryption( olt_id, llid, (PON_encryption_direction_t)( OnuMgmtTable[olt_id * MAXONUPERPON + OnuIdx].EncryptDirection -PON_ENCRYPTION_PURE) );					
    				}
					#endif
    			}
			}
            break;
            case FC_STOPENCRYPTION_COMP:
            PON_BOARD_ENSURE 
            {
    			OnuStopEncryptionInfo_S *OnuStopEncryptionData;
    			PON_olt_id_t olt_id;
    			short int OnuIdx;
    			PON_onu_id_t llid;
    			PON_stop_encryption_acknowledge_codes_t return_code;

    			OnuStopEncryptionData = (OnuStopEncryptionInfo_S *)aulMsg[3];
    			
    			if( OnuStopEncryptionData == NULL ) continue;
    			
    			olt_id = OnuStopEncryptionData->olt_id;
    			llid = OnuStopEncryptionData->llid;
    			return_code = OnuStopEncryptionData->ret_code;
    			VOS_Free( (void *)OnuStopEncryptionData );

    			OnuIdx = GetOnuIdxByLlid(olt_id, llid);
    			if(OnuIdx == RERROR )
    			{
    				continue;
    			}

    			if(( EVENT_ENCRYPT == V2R1_ENABLE) && ( return_code != ROK))
    			{
    				sys_console_printf("\r\nStop Encryption\r\n");
    				sys_console_printf("    onu%d/%d/%d %s\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1), StopEncryptRetCode[return_code]);
    			}
            }
            break;
            case FC_UPDATEENCRYPTIONKEY_COMP:
            PON_BOARD_ENSURE 
            {
    			OnuUpdateEncryptkeyInfo_S *OnuUpdateEncryptionKeyData;
    			PON_olt_id_t olt_id;
    			short int OnuIdx;
    			PON_onu_id_t llid;
    			PON_update_encryption_key_acknowledge_codes_t return_code;
    			/*LONG retSem;*/

    			OnuUpdateEncryptionKeyData = (OnuUpdateEncryptkeyInfo_S *)aulMsg[3];
    			
    			if( OnuUpdateEncryptionKeyData == NULL ) continue;
    			
    			olt_id = OnuUpdateEncryptionKeyData->olt_id;
    			llid = OnuUpdateEncryptionKeyData->llid;
    			return_code = OnuUpdateEncryptionKeyData->ret_code;
    			VOS_Free( (void *)OnuUpdateEncryptionKeyData );

    			OnuIdx = GetOnuIdxByLlid(olt_id, llid);
    			if(OnuIdx == RERROR )
    			{
    				continue;
    			}
    				
    			if(( EVENT_ENCRYPT== V2R1_ENABLE ) && ( return_code != ROK))
    			{
    				sys_console_printf("\r\nUpdate Encryption Key\r\n");
    				sys_console_printf("   onu%d/%d/%d %s\r\n", GetCardIdxByPonChip(olt_id), GetPonPortByPonChip(olt_id), (OnuIdx+1), UpdateEncryptKeyRetCode[return_code]);
    			}		
    			
    			/*
        			if(return_code == ROK)
        			{
        				retSem = VOS_SemTake(OnuEncryptSemId, WAIT_FOREVER);
        				OnuMgmtTable[ olt_id * MAXONUPERPON + OnuIdx ].EncryptKeyCounter++;
        				if(retSem == VOS_OK)
        					VOS_SemGive(OnuEncryptSemId);
        			}
        			*/
            }
            break;
            case FC_ONU_PARTNER_REG:
            PON_BOARD_ENSURE 
			{
				short int PonChipIdx_swap, PonChipIdx;
				short int standby_onu_idx, standby_llid;
				int iWaitTimes; 
				int swap_mode = 0;
				int iRlt = -1;
				int curr_slave_isready = PON_OLT_REDUNDANCY_STATE_NONE;
				int copy_flags = OLT_COPYFLAGS_CONFIG;
				unsigned char *MacAddr;

				PonChipIdx      = (aulMsg[2] & 0xFFFF);
				PonChipIdx_swap = (aulMsg[2] >> 16);
				standby_onu_idx = (aulMsg[3] >> 16) ;
				standby_llid    = (aulMsg[3] & 0xFFFF);

				OLT_LOCAL_ASSERT(PonChipIdx);
				OLT_ASSERT(PonChipIdx_swap);
				ONU_ASSERT(standby_onu_idx);
				LLID_ASSERT(standby_llid);

				swap_mode = GetPonPortHotSwapMode(PonChipIdx);
				/* B--added by liwei056@2010-1-29 for Pon-FastSwicthHover */
#if ( EPON_SUBMODULE_PON_FAST_SWAP == EPON_MODULE_YES )

				if (standby_onu_idx >= 0 && swap_mode == V2R1_PON_PORT_SWAP_QUICKLY)
				{
					MacAddr = OnuMgmtTable[PonChipIdx*MAXONUPERPON + standby_onu_idx].DeviceInfo.MacAddr;    

					if ( (0 == OLT_GetRdnState(PonChipIdx_swap, &curr_slave_isready))
							&& (PON_OLT_REDUNDANCY_STATE_SLAVE == curr_slave_isready) )
					{
						/* 等待ONU在备用PON口上的虚注册成功*/ 
						iWaitTimes = 5;
						do
						{            
							VOS_TaskDelay(__PON_SWAP_DELAY);
							if( 0 != OLTAdv_LLIDIsExist(PonChipIdx_swap, standby_llid) )
								break;
						} while(--iWaitTimes > 0);

						OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is %s to wait for sync register from M_pon%d to S_pon%d on the slot%d.\r\n"
								, standby_onu_idx + 1
								, MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]
								, standby_llid
								, (0 < iWaitTimes) ? "successed" : "failed"
								, PonChipIdx, PonChipIdx_swap
								, SYS_LOCAL_MODULE_SLOTNO);

						copy_flags = OLT_COPYFLAGS_SYNC;
						if (0 < iWaitTimes)
						{
							copy_flags |= OLT_COPYFLAGS_WITHLLID;
						}
						iRlt = CopyOnu(PonChipIdx_swap, standby_onu_idx, PonChipIdx, standby_onu_idx, copy_flags);
						OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is %s to sync[%d] register from M_pon%d to S_pon%d on the slot%d.\r\n"
								, standby_onu_idx + 1
								, MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]
								, standby_llid
								, OLT_CALL_ISOK(iRlt) ? "successed" : "failed"
								, copy_flags
								, PonChipIdx, PonChipIdx_swap
								, SYS_LOCAL_MODULE_SLOTNO);
					}   
					else
					{
						/* 注:在快倒换后的1秒内，倒换关系暂时底层无效，期间的ONU新主PON口注册不会同步到新备用PON口上虚注册，导致这里的带宽等实时配置跟踪失败。 */
						/* 但是此时的失败无妨，因为1秒后的倒换配置恢复时，会把新主PON口下的ONU配置都同步到新备用PON下。 */
						/* 所以，此处的备用PON口的备用ONU配置同步，只对倒换结束后的后序新增ONU注册有效。*/
						OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is loosed to sync register from M_pon%d to S_pon%d on the slot%d.\r\n"
								, standby_onu_idx + 1
								, MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]
								, standby_llid
								, PonChipIdx, PonChipIdx_swap
								, SYS_LOCAL_MODULE_SLOTNO);
					}
				}
				else 
#endif
				/* E--added by liwei056@2010-1-29 for Pon-FastSwicthHover */

				/* B--added by wangjiah@2017-04-27 for Pon slow auto-protect*/
					if(standby_onu_idx >= 0 && swap_mode == V2R1_PON_PORT_SWAP_SLOWLY)
					{
						copy_flags |= OLT_COPYFLAGS_WITHINFO;
						MacAddr = OnuMgmtTable[PonChipIdx*MAXONUPERPON + standby_onu_idx].DeviceInfo.MacAddr;    
						iRlt = CopyOnu(OLT_DEVICE_ID(GetCardIdxByPonChip(PonChipIdx_swap),GetPonPortByPonChip(PonChipIdx_swap))/*make CopyOnu operate through master*/, 
								standby_onu_idx, PonChipIdx, standby_onu_idx, copy_flags);
						OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE"Add Onu%d(%02x.%02x.%02x.%02x.%02x.%02x-llid:%d) is %s ( on slot/pon: %d/%d).\r\n"
								, standby_onu_idx + 1
								, MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]
								, standby_llid
								, OLT_CALL_ISOK(iRlt) ? "successed" : "failed"
								, SYS_LOCAL_MODULE_SLOTNO
								, PonChipIdx  
								);
					}
				/* E--added by wangjiah@2017-04-27 for Pon slow auto-protect*/
			}
            break;
		/*Begin:for onu swap by jinhl@2013-04-27*/
		/*针对onu倒换，注册时拷贝设置mac，llid,带宽*/
		case FC_ONUSWAP_PARTNER_REG:
            PON_TOP_MANAGER_ENSURE 
            {
                OnuEventData_s *OnuRegData = NULL;
				OnuRegData = (OnuEventData_s *)aulMsg[3];
				if(NULL != OnuRegData)
				{
				    OnuSwapRegHandler(OnuRegData);
					VOS_Free((void *)OnuRegData);
				}
				
			}
            break;
		/*End:for onu swap by jinhl@2013-04-27*/
	     case FC_ONU_AGINGTIME:
		 	ScanOfflineOnuAgingTimer();
		 	break;
			
            default:
            {
    			SYS_MSG_S *pMsg;
    			pMsg = (SYS_MSG_S *)aulMsg[3];
    			if( pMsg && (AWMC_CLI_BASE == pMsg->usMsgCode) )
    				decode_command_msg_packet( pMsg, VOS_MSG_TYPE_QUE );
    			if( pMsg != NULL )
    				VOS_Free( pMsg );
            }
        }
	}
}


void OltKPTask()
{
	unsigned long aulMsg[4];
	long result;

	if( Olt_KPALIVE_Task_flag )
		return;
	
	Olt_KPALIVE_Task_flag = 1;
	
	while( 1 )
	{
		result = VOS_QueReceive( g_Olt_KPALIVE_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
        {
			VOS_ASSERT(0);
			/*sys_console_printf("error: receiving msg from olt Que failure(OltTask())\r\n" );*/
			VOS_TaskDelay(20);
			continue;/*return;*/
		}
        
		if( result == VOS_NO_MSG ) continue;

        switch(aulMsg[1])
        {
            case FC_V2R1_TIMEOUT:
            PON_BOARD_ENSURE
            {
    			V2r1TimerHandlerForKP();
            }
            break;
			default:
				break;
    	}
	}
}

void PonTask()
{
	unsigned long aulMsg[4];
	long result;
	
	Pon_Task_flag = 1;

	for(;;)
    {
		result = VOS_QueReceive( g_Pon_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR ) 
			{
			/*sys_console_printf("error: receiving msg from pon Que failure(PonTask())\r\n" );*/
			VOS_ASSERT(0);
			VOS_TaskDelay(20);
			continue;
			}
		if( result == VOS_NO_MSG ) continue;

        switch (aulMsg[1])
        {
            case FC_OAM_COMM_RECV:
            PON_BOARD_ENSURE 
            {
    			OamMsgRecv_S  *RecvOamMsg;
    			RecvOamMsg = (OamMsgRecv_S  *)aulMsg[3];
    			if( (RecvOamMsg != NULL) && (RecvOamMsg->content != NULL) )
    			{
    				Ethernet_frame_received_handler_1(RecvOamMsg->olt_id,RecvOamMsg->port, RecvOamMsg->type,
    					RecvOamMsg->llid, RecvOamMsg->length, (void *)RecvOamMsg->content);
    			}
    			else
    				VOS_ASSERT(0);
    			if( RecvOamMsg != NULL )
    			{
    				if( RecvOamMsg->content != NULL )
    					VOS_Free(RecvOamMsg->content );
    				VOS_Free(RecvOamMsg);
    			}
            }
            break;
            case FC_ONU_REGISTER:
            PON_BOARD_ENSURE 
            {
                ONURegisterInfo_S *OnuRegisterData = (ONURegisterInfo_S*)aulMsg[3];
                
                if( OnuRegisterData != NULL )
                {
	                    Pon_event_handler(PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY, OnuRegisterData);

	                    VOS_Free(OnuRegisterData);    
                }
                else
                {
                    VOS_ASSERT( 0 );
                }
            }
            break;
			case FC_EXTOAMDISCOVERY:
			PON_BOARD_ENSURE;
            {
				OnuEventData_s *OnuRegisterExtData = (OnuEventData_s*)aulMsg[3];
                
				if( OnuRegisterExtData  != NULL )
				{
	                    Pon_event_handler(PON_EVT_HANDLER_LLID_END_EXT_OAM_DISCOVERY, OnuRegisterExtData);

						VOS_Free((void*)OnuRegisterExtData);
				}
                else
                {
                    VOS_ASSERT( 0 );
                }
            }
			break;
            case FC_ONU_DEREGISTER:
            PON_BOARD_ENSURE 
            {
                ONUDeregistrationInfo_S *OnuDeregistrationData = (ONUDeregistrationInfo_S*)aulMsg[3];
                
                if( OnuDeregistrationData != NULL )
                {
	                    Pon_event_handler(PON_EVT_HANDLER_LLID_DEREGISTER_EVENT, OnuDeregistrationData);

	                    VOS_Free(OnuDeregistrationData);    
	                }
                else
                {
                    VOS_ASSERT( 0 );
                }
            }
            break;
            case FC_PON_ALARM:
            PON_BOARD_ENSURE 
            {
    			PonAlarmInfo_S  *PonAlarmData;

    			PonAlarmData = ( PonAlarmInfo_S *)aulMsg[3];
    			if( PonAlarmData != NULL )
    			{
    				/*VOS_TaskLock();*/
    				PonAlarmHandler( PonAlarmData );
    				VOS_Free( (void *) PonAlarmData );
    				/*VOS_TaskUnlock();*/
    			}
            }
            break;
            case FC_PON_FILE_LOADED:
            PON_BOARD_ENSURE 
            {
    			PonFileLoadCompleted_S *PonFileLoadResult;

    			PonFileLoadResult = ( PonFileLoadCompleted_S *)aulMsg[3];
    			if( PonFileLoadResult != NULL )
                {
    				LoadFileCompletedHandler( PonFileLoadResult );
    				VOS_Free( (void *)PonFileLoadResult );
    			}
            }
            break;
            case FC_V2R1_TIMEOUT:
            PON_BOARD_ENSURE 
            {
                extern void pon_load_scan_garbage();
                pon_load_scan_garbage();
            }
            break;
            case FC_PON_RESET:
            PON_BOARD_ENSURE 
            {
    			PonResetInfo_S   *PonResetData;

    			PonResetData = ( PonResetInfo_S *)aulMsg[3];
    			if( PonResetData != NULL )
                {
#if 0
    				PonResetHandler( PonResetData );
#else
                	if( PonResetData->code != 0xff )
                	{
    					PonResetHandler( PonResetData );
                	}
					else
					{
						/* 直接复位 */
						Pon_RemoveOlt(PonResetData->olt_id, FALSE, FALSE);	
						PonAbnormalHandler(PonResetData->olt_id, PON_OLT_RESET_OLT_EVENT);

					    if( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
					    {
					        OLT_SYNC_Data(PonResetData->olt_id, FC_PON_RESET, NULL, 0);
					    }
						ClearPonRunningData( PonResetData->olt_id );
						Add_PonPort( PonResetData->olt_id );
					}
#endif
    				VOS_Free( (void *)PonResetData );
    			}
            }
            break;
            case FC_PONLOSS:
            PON_BOARD_ENSURE 
            {
    			short int  PonPortIdx;
    			/*short int  PonPortIdx_Swap;*/

    			PonPortIdx = aulMsg[2];
    			if( aulMsg[3] == ON )
    			{
    				Trap_PonPortSignalLoss(PonPortIdx, V2R1_ENABLE);
    				/*sys_console_printf("\r\n%s/port%d Signal Loss\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));*/
    			}
    			else if( aulMsg[3] == OFF )
    			{
    				Trap_PonPortSignalLoss(PonPortIdx, V2R1_DISABLE);
    				/*sys_console_printf("\r\n%s/port%d Signal ON\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx));*/
#if 0
    				/* modified by chenfj 2007-10-9
    				在检测到PON口失光后，检查PON口是否配置了保护切换，并且其状态为主用；
    				*/
    				if( PonPortSwapEnableQuery( PonPortIdx ) != V2R1_PON_PORT_SWAP_ENABLE ) continue;
    				if( PonPortHotStatusQuery(PonPortIdx) != V2R1_PON_PORT_SWAP_ACTIVE ) continue;
    				if( PonPortSwapPortQuery(PonPortIdx, &PonPortIdx_Swap ) != ROK ) continue;
    				if( GetPonPortAdminStatus(PonPortIdx_Swap) == V2R1_DISABLE ) continue;

    				if( PonPortIsWorking( PonPortIdx_Swap ) != TRUE )
    					{
    					if(PonPortIsWorking(PonPortIdx) != TRUE)
    						{
    						PonPortTable[PonPortIdx].swap_timer  = 0;
    						 PonPortTable[PonPortIdx].swapHappened = TRUE;
    						}
    					continue;
    					}

    				else /*if ( PonPortIsWorking( PonPortIdx_Swap ) == TRUE )*/
    					{
    					HotSwapPort1ToPort2(PonPortIdx, PonPortIdx_Swap);
    					/*
    					if( PonPortIdx < PonPortIdx_Swap )
    						PonPortTable[PonPortIdx_Swap].swapping = V2R1_SWAPPING_NOW;
    					*/
    					}
#endif
    			}
            }
            break;
/* B--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
            case FC_PONSWITCH:
            {
    			PonSwitchInfo_S   *PonSwitchData;

    			PonSwitchData = ( PonSwitchInfo_S *)aulMsg[3];
    			if( PonSwitchData != NULL )
                {
    				PonSwitchHandler( PonSwitchData );
    				VOS_Free( (void *)PonSwitchData );
    			}
            }
            break;
/* E--added by liwei056@2010-1-26 for Pon-FastSwicthHover */
/*Begin:for onu swap by jinhl@2013-02-22*/
            #if ( EPON_SUBMODULE_ONU_OPTIC_SWAP == EPON_MODULE_YES )
            case FC_ONUSWITCH:
            {
    			OnuSwitchInfo_S   *OnuSwitchData = NULL;
                OLT_SWITCH_DEBUG(OLT_SWITCH_TITLE" PonTask rec FC_ONUSWITCH\r\n");
				
    			OnuSwitchData = ( OnuSwitchInfo_S *)aulMsg[3];
    			if( OnuSwitchData != NULL )
                {
    				OnuSwitchHandler( OnuSwitchData );
					VOS_Free( (void *)OnuSwitchData );
    			}
            }
            break;
			#endif
/*End:for onu swap by jinhl@2013-02-22*/
            case FC_ONUDENIED:
            PON_BOARD_ENSURE 
            {
    			OnuRegisterDenied_S  *OnuRegisterDeniedData;

    			OnuRegisterDeniedData = ( OnuRegisterDenied_S *)aulMsg[3];
    			if( OnuRegisterDeniedData != NULL )
    			{
    				OnuRegisterDeniedHandler( OnuRegisterDeniedData );
    				VOS_Free( (void *)OnuRegisterDeniedData );
    			}
            }
            break;
            case FC_SFPLOSS:
            PON_BOARD_ENSURE 
            {
    			short int  PonPortIdx;

    			PonPortIdx = aulMsg[2];
                CheckSlotPonSFPTypeCallback(PonPortIdx, TRUE);
            }
            break;
            default:
                VOS_ASSERT(0);
        }
	}

	pon_terminate();
}

/*int CardStatus1[10];*/

void  PonUpdateTask()
{
	unsigned long aulMsg[4];
	long result = 0;
	/*short int  onuIdx;*/
	/*
	for(result =0; result < 10; result++)
		CardStatus1[result] = 0;
	result = 0;
	*/
	PonUpdate_Task_flag = 1;

	for(;;)
    {
		result = VOS_QueReceive( g_PonUpdate_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
        {
			/*sys_console_printf("error: receiving msg from ponUpdate Que failure(PonUpdateTask())\r\n" );*/
			VOS_ASSERT(0);
			VOS_TaskDelay(20);
			continue;
		}
        
		if( result == VOS_NO_MSG ) continue;

        switch (aulMsg[1])
        {
            case FC_ADD_PONPORT:
            PON_BOARD_ENSURE 
            {
    			/*int ret;*/
    			short int PonPortIdx;
    			short int PonChipType;
                int CardSlot, CardPort;
            
    			PonPortIdx  = (short int) aulMsg[3];
                PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
                
                CardSlot = GetCardIdxByPonChip(PonPortIdx);
                CardPort = GetPonPortByPonChip(PonPortIdx);

#if 1           
                SetPonChipResetFlag(CardSlot, CardPort);
                pon_add_oltEx2(CardSlot, CardPort, PonPortIdx, PonChipType, 0, 1, 1);
#else
    			if(pon_add_olt( PonPortIdx ) != ROK )
    			{
    				if(PonChipType != PONCHIP_PAS5001 )
    				{
    					do{
    					 	PonChipDownloadCounter[PonPortIdx] ++;					
    						if(OLTAdv_IsExist(PonPortIdx) == TRUE )
    							Remove_olt( PonPortIdx, FALSE, FALSE);	
    						ClearPonRunningData( PonPortIdx );
    						Hardware_Reset_olt1(CardSlot, CardPort, 0, 0);
    						VOS_TaskDelay(VOS_TICK_SECOND);
    						Hardware_Reset_olt2(CardSlot, CardPort, 1, 0);
    						ret = pon_add_olt(PonPortIdx );	
    					}while((ret != ROK ) && (PonChipDownloadCounter[PonPortIdx] < (PON_DOWNLOAD_MAX_COUNTER-1) ));
    				}

                    /* B--added by liwei056@2011-1-13 for SyncMasterStatus */
                    if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
                    {
                        OLT_SYNC_Data(PonPortIdx, FC_DEL_PONPORT, NULL, 0);
                    }
                    /* E--added by liwei056@2011-1-13 for SyncMasterStatus */
    			}
    			ClearPonChipResetFlag(CardSlot, CardPort);
#endif                
            }
            break;
            case FC_DEL_PONPORT:
            PON_BOARD_ENSURE 
            {
    			short int PonPortIdx;
    			PonPortIdx =(short int) aulMsg[3];
    			pon_reset_olt( PonPortIdx );
            }
            break;
            case FC_CARD_INSERTED:
            {
    			/*short int CardSlot;
    			
    			CardSlot = (short int )aulMsg[3];
				VOS_TaskDelay( VOS_TICK_SECOND/2);
				PonCardInsert( CardSlot );*/
				PonCardInsert( (aulMsg[2] >> 8), aulMsg[3], (aulMsg[2] & 0xff) );	/* modified by xieshl 20120416, 传入主备倒换标志(aulMsg[2]中低字节)和板卡类型(aulMsg[2]中第2个字节) */
            }
            break;
            case FC_CARD_PULLED:
            {
    			/*short int CardSlot;
    			
    			CardSlot = (short int )aulMsg[3];*/
			PonCardPull( aulMsg[2], aulMsg[3] );
            }
            break;
            case FC_CARD_ACTIVATED:
            {
    			short int CardSlot;
    			
    			CardSlot = (short int )aulMsg[3];
				PonCardActivate( CardSlot,  (short int) aulMsg[2]);
            }
            break;
            case FC_PONPORT_ACTIVATED:
            {
    			short int PonPortIdx;
    			PonPortIdx = (short int )aulMsg[3];
    			ActivePonPort(PonPortIdx);			
            }
            break;
			case FC_PONDEV_RESET:
			{
				short int PonPortIdx;
    			PonPortIdx = (short int )aulMsg[3];
				sys_console_printf("\r\n%s %d &&&&&& pon:%d\r\n",__FUNCTION__,__LINE__,PonPortIdx);
    			OLT_ResetPon(PonPortIdx);		
			}
			break;
			case FC_PONDEV_DISCONNECT:
			{
				extern void gponOltAdp_DevDisconnectDirect();
				gponOltAdp_DevDisconnectDirect();
			}
			break;
    		/* added by chenfj 2006/09/22 */ 
    		/* #2617  问题自动注册限制功能使能，再去使能，结果ONU无法恢复注册*/
    		/* #2619  问题自动注册限制使能后，手动注册ONU时，出现异常*/
            case FC_ACTIVE_PENDING_ONU:
            {
    			short int PonPortIdx;

                PonPortIdx = aulMsg[2];
    			ActivatePendingOnu( PonPortIdx );
            }
            break;
            case FC_ACTIVE_FENDING_ONU_BY_MAC:
            {
    			unsigned char *MacAddr;
    			short int PonPortIdx;
    			PonPortIdx = aulMsg[2];
    			MacAddr = (unsigned char *)aulMsg[3];
    			
    			ActivateOnePendingOnu( PonPortIdx, MacAddr );
    			
    			VOS_Free((void *)MacAddr );
            }
            break;
			case FC_ACTIVE_FENDING_ONU_BY_SN:
            {
    			unsigned char *SN;
    			short int PonPortIdx;
    			PonPortIdx = aulMsg[2];
    			SN = (unsigned char *)aulMsg[3];
    			
    			ActivateOnePendingOnu( PonPortIdx, SN);
    			
    			VOS_Free((void *)SN);
            }
            break;
            case FC_ACTIVE_PENDING_ONU_CONF:
            {
    			short int PonPortIdx;
    			unsigned char Address[GPON_ONU_SERIAL_NUM_STR_LEN], *Address1;

                PonPortIdx = aulMsg[2];
    			Address1 = (unsigned char *)aulMsg[3];
    			if( Address1 != NULL )
    			{
    				VOS_MemCpy(Address,Address1,GPON_ONU_SERIAL_NUM_STR_LEN);
    				
    				ActivatePendingOnu_conf(PonPortIdx, Address );
    				
    				VOS_Free( Address1 );
    			}
            }
            break;
            case FC_ACTIVE_PENDING_ONU_CONF_ALL:
            {
    			short int PonPortIdx;
                
    			PonPortIdx = aulMsg[2];
    			ActivatePendingOnu_conf_all(PonPortIdx);
            }
            break;
            case FC_PONUPDATE:
            {
    			short int PonPortIdx;
                
    			PonPortIdx = aulMsg[2];
    			UpdatePonFirmware(PonPortIdx);
            }
            break;
            default:
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
            if ( (PRODUCT_E_EPON3 == SYS_PRODUCT_TYPE)
                || (PRODUCT_E_GFA6100 == SYS_PRODUCT_TYPE) )
            {
        		/* tdm 设备管理*/
    			TdmMsgHandler( aulMsg );                
            }
            else
#endif
            {
                VOS_ASSERT(0);
            }
        }
	}
}		

#if 0
static VOID OnuTask()
{
	unsigned long aulMsg[4];
	long result;
	short int PonPortIdx, OnuIdx;
	unsigned char MsgType;
	int length;
	unsigned char *pBuf;
	/*short int  ret_PonPort=0;*/

	ONURegisterInfo_S *OnuRegisterData;
	ONUDeregistrationInfo_S  *OnuDeregistrationData;
	ExtOAMDiscovery_t  *ExtOAMDiscovery_Data ;
	int encryptEnable, encryptDir;
	int onuEntry;

	Onu_Task_flag = 1;

	while(1)
	{
		result = VOS_QueReceive( g_Onu_Queue_Id, aulMsg, WAIT_FOREVER );
		if( result == VOS_ERROR )
		{
			VOS_ASSERT(0);
			/*sys_console_printf("error: receiving msg from onu queue failure(OnuTask())\r\n" );*/
			VOS_TaskDelay(20);
			continue;
		}
		if( result == VOS_NO_MSG ) continue; 

		switch( aulMsg[1] )
		{
			/*case FC_GETONUEUQ:
				{
				OnuIdentifier_S *CurNode, *NextNode;
				short int PonPortIdx, OnuIdx;

				CurNode = OnuWaitForGetEUQ.NextNode;
				while( CurNode != NULL )
					{
					PonPortIdx = CurNode->PonPortIdx;
					OnuIdx = CurNode->OnuIdx;
					NextNode = CurNode->NextNode;
					DelOneNodeFromGetEUQ(CurNode->PonPortIdx, CurNode->OnuIdx );
					GetOnuEUQInfo(PonPortIdx, OnuIdx);		
					CurNode = NextNode;
					}
				}
				break;*/
			
			case FC_ONU_STATUS_SYNC_TIMEOUT:	/* added by xieshl 20111019, 主控和PON板间ONU状态同步 */
				OnuMgtSync_OnuStatusReport();
				break;
		
			case CDP_NOTI_FLG_RXDATA:             /*同步消息接收，add by shixh20100608*/
				
				recv_onuSyncMessage((VOID *)aulMsg[3]);

				break;
	
			case FC_ONU_EUQ_INFO:
				/*PON_BOARD_ENSURE;*/
				/* modified by xieshl 20111130, 扩展支持GT811_C读取 */
				OnuIdx = (short int )aulMsg[2] & 0xffff;
				PonPortIdx = aulMsg[2] >> 16;
				pBuf = (unsigned char *)aulMsg[3];
				if( pBuf != NULL )
				{
					int OnuType, OnuEntry;
					
					MsgType = pBuf[0];
					length  = *(int *) &pBuf[4];

					switch( MsgType )
					{
						case  GET_ONU_SYS_INFO_REQ:

							if( IsSupportCTCOnu(PonPortIdx) )
							{
								if(GetOnuRegisterData( PonPortIdx, OnuIdx) != ROK )
								{
									break;
								}
								SetOnuExtOAMStatus(PonPortIdx, OnuIdx );
								CTC_GetOnuDeviceInfo( PonPortIdx, OnuIdx );
								OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
								OnuType = CompareCtcRegisterId( PonPortIdx, OnuIdx, OnuMgmtTable[OnuEntry].onu_model );
								if(OnuType != RERROR)
								{
									ONU_MGMT_SEM_TAKE;
									OnuMgmtTable[OnuEntry].DeviceInfo.type = OnuType;
									ONU_MGMT_SEM_GIVE;
									if( GetOnuEUQInfo(PonPortIdx, OnuIdx) == RERROR )
										break;
								}
								else
								{
									ONU_MGMT_SEM_TAKE;
									if(OnuMgmtTable[OnuEntry].onu_ctc_version >= CTC_2_1_ONU_VERSION)
									{
										OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_ONU_CTC;
									}
									else
										OnuMgmtTable[OnuEntry].DeviceInfo.type = V2R1_DEVICE_UNKNOWN;
									OnuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;
									ONU_MGMT_SEM_GIVE;

									if(OnuType == V2R1_DEVICE_UNKNOWN)
									{
										break;
									}
								}
							}
							else
							{
								if( GetOnuEUQInfo(PonPortIdx, OnuIdx) == RERROR )
									break;
							}

							OnuMgtSyncDataSend_Register( PonPortIdx, OnuIdx );

							break;
							
						case  SET_ONU_SYS_INFO_REQ:
							if( pBuf[2] == SET_ONU_SYS_INFO_NAME )
								SetOnuDeviceNameMsg( PonPortIdx, OnuIdx, &pBuf[8], length );
							else if( pBuf[2] == SET_ONU_SYS_INFO_DESC )
								SetOnuDeviceDescMsg(PonPortIdx, OnuIdx, &pBuf[8], length );
							else if( pBuf[2] == SET_ONU_SYS_INFO_LOCATION )
								SetOnuLocationMsg(PonPortIdx,OnuIdx, &pBuf[8], length);
							break;						
						case  SYNC_ONU_SYS_TIME_REQ:
							SyncSysTimeToOnu(PonPortIdx, OnuIdx );
							break;

						default: 
							break;
					}
					VOS_Free((void *) pBuf );
					/*VOS_TaskDelay(30);*/
				}
				break;
				
			case FC_V2R1_TIMEOUT:
				PON_BOARD_ENSURE;
#if( EPON_MODULE_ONU_REG_FILTER	 == EPON_MODULE_YES )
				HandlerRegisterFilterQueue();
#endif
				/*V2r1TimerHandler();*/
				break;

			case FC_ONU_REGISTER:
				PON_BOARD_ENSURE;
				OnuRegisterData = ( ONURegisterInfo_S *)aulMsg[3];
				if( OnuRegisterData != NULL )
				{
					OnuRegisterHandler( OnuRegisterData );
					VOS_Free( (void *) OnuRegisterData );
				}
				break;
			case FC_ONU_DEREGISTER:
				PON_BOARD_ENSURE;
				OnuDeregistrationData = (ONUDeregistrationInfo_S *)aulMsg[3];
				if( OnuDeregistrationData != NULL )
				{
				    if ( OnuDeregistrationData->deregistration_code > 0 )
                    {
    					OnuDeregisterHandler( OnuDeregistrationData );
                    }
                    else
                    {
    					OnuDeregisterEvent( OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id, -OnuDeregistrationData->deregistration_code, OnuDeregistrationData->event_flags );
                    }

					VOS_Free( (void *)OnuDeregistrationData );
				}
				break;

			case FC_ONU_TDM_SERVICE:
				PON_BOARD_ENSURE;
				OnuIdx = (short int )aulMsg[2] & 0xffff;
				PonPortIdx = (short int)(aulMsg[2] >> 16);

				pBuf = (unsigned char *)aulMsg[3];
				if( pBuf != NULL )
				{
					MsgType = FC_ONU_TDM_SERVICE;
					length  = *(int *) &pBuf[4];				
					EQU_SendMsgToOnu( PonPortIdx, OnuIdx, MsgType, &pBuf[8], length );
					VOS_Free((void *) pBuf );
					VOS_TaskDelay(30);
				}
				break;
				
			case FC_ONU_TDM_SERVICE_ALL:
				PON_BOARD_ENSURE;
				{
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
				ConfigOnuVoiceService(aulMsg[2]);
				/*VOS_TaskDelay(30);*/
#endif
				}
				break;

			case FC_EXTOAMDISCOVERY:
				PON_BOARD_ENSURE;
				ExtOAMDiscovery_Data = (ExtOAMDiscovery_t*)aulMsg[3];
				if( ExtOAMDiscovery_Data  != NULL )
				{
					OnuExtOAMDiscoveryHandler( ExtOAMDiscovery_Data );
					VOS_Free((void*)ExtOAMDiscovery_Data);
				}
				break;

			case FC_V2R1_START_ENCRYPT:
				PON_BOARD_ENSURE;
				PonPortIdx = (short int)aulMsg[2];
				OnuIdx = (short int )aulMsg[3];

				if(( PonPortIdx< 0) ||(PonPortIdx >= MAXPON ))
				{
					VOS_ASSERT(0);
					break;
				}
				if(( OnuIdx < 0 ) || ( OnuIdx >= MAXONUPERPON )) 
				{
					VOS_ASSERT(0);
					break;
				}

				onuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
				ONU_MGMT_SEM_TAKE;
				encryptEnable = OnuMgmtTable[onuEntry].EncryptEnable;
				encryptDir = OnuMgmtTable[onuEntry].EncryptDirection;
				ONU_MGMT_SEM_GIVE;
				if( encryptEnable == V2R1_ENABLE ) 
				{
					OnuEncryptionOperation( PonPortIdx, OnuIdx, encryptDir ) ;
				}
				break;
				
			case CDP_NOTI_FLG_SEND_FINISH:
				{
				 if ( aulMsg[3] != 0)
					CDP_FreeMsg( (void*)aulMsg[3] );
				 else
				 	VOS_ASSERT(0);
				}
				break;
			default:
				break;
		}
	}
}
#endif

void v2r1Version()
{
	extern char * app_creationDate;
	sys_console_printf("\r\n\r\n%s\r\n", typesdb_product_series() );
	sys_console_printf("Application Version: %s \r\n", V2R1_VERSION);
	sys_console_printf("Build Time %s\r\n", app_creationDate/*__TIME__, __DATE__*/ );		/* modified by xieshl 20091023, 区分产品软件build日期 */
	sys_console_printf("%s\r\n", typesdb_product_copyright());
}

int dbgPriO = 95;
void V2r1_CreateTask()
{
    /*short int PonManageType;*/
    LONG lOltQueLen, lOnuQueLen, lPonQueLen;

    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        ULONG ulSize;

#if 1
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/
        /*ulSize = RecvMsgNumMAX * RecvMsgLengthMAX;
        if ( NULL == (RevMsg = g_malloc(ulSize)) )*/
        ulSize = RecvMsgNumMAX * sizeof(low_rx_msg_t);
        if ( NULL == (RevMsg = (low_rx_msg_t*)g_malloc(ulSize)) )
        {
            VOS_ASSERT(0);
        }
#endif
        
    	/* create the physical level communication queue and task */
    	if( g_LowLevelComm_Queue_Id == 0 ) 
    		g_LowLevelComm_Queue_Id = VOS_QueCreate( MAXLOWLEVELMSGNUM , VOS_MSG_Q_FIFO);
    	if( g_LowLevelComm_Queue_Id  == 0 ) {
    		VOS_ASSERT( 0 );
    		}
    	sys_console_printf("\r\n create the LowLevel comm task ... ");
	
    	/*g_LowLevelComm_Task_Id = VOS_TaskCreate( (char *)"tLowComm", (ULONG)TASK_PRIORITY_LOWLEVELCOMM, LowLevelComm, (ULONG *)NULL );*/
    	g_LowLevelComm_Task_Id = VOS_TaskCreateEx( "tLowComm", ( VOS_TASK_ENTRY ) LowLevelComm, TASK_PRIORITY_LOWLEVELCOMM, 60*1024, NULL );
    	while( LowLevelComm_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
    	VOS_QueBindTask( (	VOS_HANDLE)g_LowLevelComm_Task_Id, g_LowLevelComm_Queue_Id );
    	if( g_LowLevelComm_Task_Id != 0 )
    		sys_console_printf(" OK\r\n");
    	else sys_console_printf(" err\r\n");
    }

#if 0	/* modified by xieshl 20160523, 解决ONU提前注册问题，放大消息队列，调整相关任务优先级，防止消息丢失 */
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER ) 
    {
        lOltQueLen = MIN(MAXOLTMSGNUM, (MAXPON * 50));    
        /*lOnuQueLen = MIN(MAXONUMSGNUM, (MAXONU * 2));*/
    }
    else
    {
        lOltQueLen = MAX(MAXOLTMSGNUM, (MAXPON * 10));    
        /*lOnuQueLen = MAX(MAXONUMSGNUM, MAXONU);*/
    }
    lOnuQueLen = MAXONUMSGNUM;          /*modi by lu 2013-3-12*/
    lPonQueLen = lOltQueLen;    
#else
    lOnuQueLen = 8192*2;
	lOltQueLen = 4096;
    lPonQueLen = 4096;
#endif

	/* create the olt mgmt queue and task */
	if( g_Olt_Queue_Id == 0 )  g_Olt_Queue_Id = VOS_QueCreate( lOltQueLen , VOS_MSG_Q_FIFO);
	if( g_Olt_Queue_Id  == 0 ){
		VOS_ASSERT( 0 );
		}

	sys_console_printf(" create the olt mgmt task ...");
	g_Olt_Task_Id = VOS_TaskCreate ( "tOltTask", TASK_PRIORITY_OLT, OltTask ,  NULL );    
	while( Olt_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask(( VOS_HANDLE ) g_Olt_Task_Id, g_Olt_Queue_Id );
	if( g_Olt_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");

	/*Begin:GPON保活机制增加任务 by jinhl@2016.09.27*/

	if( g_Olt_KPALIVE_Queue_Id == 0 )  g_Olt_KPALIVE_Queue_Id = VOS_QueCreate( lOltQueLen , VOS_MSG_Q_FIFO);
	if( g_Olt_KPALIVE_Queue_Id  == 0 ){
		VOS_ASSERT( 0 );
		}
	
	sys_console_printf(" create the olt mgmt KPALIVE task ...");
	g_Olt_KPALIVE_Task_Id = VOS_TaskCreate ( "tOltKPTask", dbgPriO, OltKPTask ,  NULL );    
	while( Olt_KPALIVE_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask(( VOS_HANDLE ) g_Olt_KPALIVE_Task_Id, g_Olt_KPALIVE_Queue_Id );
	if( g_Olt_KPALIVE_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");
	/*End:GPON保活机制增加任务 by jinhl@2016.09.27*/
	

    if ( 1 /* SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER */ )
    {
    	/* create the pon mgmt queue and task */
    	if( g_Pon_Queue_Id == 0 )  g_Pon_Queue_Id = VOS_QueCreate( lPonQueLen , VOS_MSG_Q_FIFO);
    	if( g_Pon_Queue_Id  == 0 ) {
    		VOS_ASSERT( 0 );
    		}
    	
    	sys_console_printf(" create the pon mgmt task ...");
    	g_Pon_Task_Id = VOS_TaskCreate("tPonTask", 62/*TASK_PRIORITY_PON*/, PonTask, NULL );
    	while( Pon_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
    	VOS_QueBindTask( ( VOS_HANDLE )g_Pon_Task_Id, g_Pon_Queue_Id );
    	if( g_Pon_Task_Id != 0 )
    		sys_console_printf(" OK\r\n");
    	else sys_console_printf(" err\r\n");
    }


	/* create the pon update queue and task */
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( lPonQueLen , VOS_MSG_Q_FIFO);
	if( g_PonUpdate_Queue_Id  == 0 ) {
		VOS_ASSERT( 0 );
		}

	sys_console_printf(" create the card mgmt task ...");
	g_PonUpdate_Task_Id = VOS_TaskCreate("tCardMgmt", 61/*TASK_PRIORITY_PONUPDATE*/, PonUpdateTask, NULL );
	while( PonUpdate_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask( ( VOS_HANDLE )g_PonUpdate_Task_Id,  g_PonUpdate_Queue_Id );
	if( g_PonUpdate_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");


/*add by shixh20100525,增加ONU管理信息同步，*/
/* create the onu mangent sync queue and task */
{
/* create the onu queue and task */
	if( g_Onu_Queue_Id == 0 )  g_Onu_Queue_Id = VOS_QueCreate( lOnuQueLen , VOS_MSG_Q_FIFO);
	if( g_Onu_Queue_Id  == 0 ) {
		VOS_ASSERT( 0 );
		}
#if 0	
	sys_console_printf(" create the Onu task ...");
	g_Onu_Task_Id = VOS_TaskCreateEx( "tOnu", NewOnuTask, TASK_PRIORITY_ONU, 81920, NULL );
	/*g_Onu_Task_Id = VOS_TaskCreate("t_Onu", TASK_PRIORITY_ONU, OnuTask, NULL );*/
	while( Onu_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask(( VOS_HANDLE ) g_Onu_Task_Id,  g_Onu_Queue_Id );
	if( g_Onu_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");

	CDP_Create(RPU_TID_CDP_ONU, CDP_NOTI_VIA_QUEUE, g_Onu_Queue_Id, NULL);
#else
    OnuModule_init();
#endif

	sys_console_printf(" create the ONU configuration Manager task ...");
	init_onuConfSyndProcedure();
	sys_console_printf(" OK\r\n");
	
	sys_console_printf(" create the ONU port statistics task ...");
	Onustats_Init();
	sys_console_printf(" OK\r\n");	
}
	

/* 增加TDM 设备管理
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )*/
#if 0
	/* create the onu queue and task */
	sys_console_printf("\r\n create the g_Tdm_mgmt_Queue_Id queue ...\r\n ");
	if( g_Tdm_mgmt_Queue_Id == 0 )  g_Tdm_mgmt_Queue_Id = VOS_QueCreate( MAXTDMMGMTMSGNUM , VOS_MSG_Q_FIFO);
	if( g_Tdm_mgmt_Queue_Id  == 0 ) {
		VOS_ASSERT( 0 );
		sys_console_printf("error: VOS can not create g_Tdm_mgmt_Queue_Id( V2r1_CreateTask() )\r\n");
		}
	else{
		sys_console_printf(" \r\n Create the g_Tdm_mgmt_Queue_Id OK, Queu Id = %d \r\n ", g_Tdm_mgmt_Queue_Id);
		}
	
	sys_console_printf(" \r\n create the g_Tdm_mgmt_Task_Id task ...");
	g_Tdm_mgmt_Task_Id = VOS_TaskCreate("tTdmMgmt", TASK_PRIORITY_TDM_MGMT, TdmMgmtEntry, NULL );
	while( Tdm_mgmt_Task_flag != 1 ) VOS_TaskDelay( VOS_TICK_SECOND /2 );
	VOS_QueBindTask(( VOS_HANDLE ) g_Tdm_mgmt_Task_Id,  g_Tdm_mgmt_Queue_Id );
	if( g_Tdm_mgmt_Task_Id != 0 )
		sys_console_printf(" OK\r\n");
	else sys_console_printf(" err\r\n");


#endif
	
}

#ifdef  V2R1_SWITCH_5325E
extern  LONG BCM5325E_DEBUG_CommandInstall(void);
#endif
/*extern void  initOperateFunctions( void );*/
extern void eventProcInit2();

#if (RPU_MODULE_HOT ==  RPU_YES )
extern ULONG devsm_check_running_state_reset_period;
extern ULONG devsm_check_hot_insert_pull_period;
#endif

#if( EPON_MODULE_POWEROFF_INT_ISR == EPON_MODULE_YES )
/* begin: added by jianght 20090525 */
extern VOID initPowerAlarm();
/* end: added by jianght 20090525 */
#endif
extern LONG init_onuBwBasedMacMib();
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
extern LONG trace_path_init();
#endif

LONG v2r1Entry()
{
	int Product_Type = SYS_PRODUCT_TYPE;

	/*sys_console_printf("\r\n\r\n\r\n===== This is the entry for the EasyPath V2R1 product software =====\r\n");*/

	/*sys_console_printf("\r\n");*/
	VOS_TaskDelay(2);
    
	/* 产品软件识别设备类型, 并据此初始化一些变量*/
	InitProductTypeVar();   

#ifdef  V2R1_SWITCH_5325E
	BrdChannelBcm5325eInit();
	/*BCM5325E_DEBUG_CommandInstall();*/	/* removed by xieshl 20070712 */
#endif

	eventProcInit2();

	/*
     * author wangxy
     * added at 2012-02-16 注册snmp团体字处理函数，针对ONU的集中配置管理作准备工作
     */

	init_snmp_access_record();

    /* if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER ) */
    {
        /* MIB只在主、备板注册 */
    	setup_tree();	
    	init_gwEponMib();
    	init_gwEponExtMib();
    	init_gwUserMgtMib();/*add by shixh20100223*/
    	init_gwEponDevMib();
    	init_ftpManMib();
    	init_gwSecurityMib();
    	init_ethPortMIB();

      	init_gwIgmpSnoopAuth();
    	init_ctcIgmpSnoopAuthTabObj();

    	init_eponMonMib();
    	init_eponOnuMib();
	init_onuBwBasedMacMib();
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
        if ( PRODUCT_E_EPON3 == Product_Type )
        {
        	init_eponTdmMib();
        	init_tdm_process_function();
        }
#endif

#if ( EPON_MODULE_QOS_ACL_QINQ_MIB == EPON_MODULE_YES )
    	init_gwEponQinQMib();/*added by zhengyt 09-08-04*/
    	init_gwAclMib();/*added by zhengyt 09-06-23*/
    	init_gwQosMib();/*added by zhengyt 09-06-25*/
    	init_gwEthLoopMib();	/*added by zhengyt@10-3-18,增加环路检测mib*/
#endif

#if ( EPON_MODULE_OLT_QINQ_MIB == EPON_MODULE_YES )
        init_gwEponOltQinQMib();
#endif

	/*EPON_V2R1_CliInit();*/
    
#ifdef __CTC_TEST
    	/*init_gwGeneralCmdGroup();*/	/* removed by xieshl 20100203, 这个MIB废弃了 */
    	init_gwIgmpSnoopMcCdr();/*added by zhengyt  09-02-17*/
#endif
    	init_gwEponFill();  /*added by zhengyt 09-02-13*/

#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
    	init_eponAutoLoadFtp();
#endif
        
#if( EPON_MODULE_DOCSIS_MANAGE_MIB == EPON_MODULE_YES )
        init_brcmCmcControllerMib();
#endif

    	init_gwdOnuProfileMib();
#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
	init_userTraceMib();
#endif
    	setOltMibTree();

    	/*
         * author wangxy
         * 初始化CTC-ONU管理MIB树
         */
    	setup_tree();
    	init_onuEthPortMib();
    	/*init_gwdOnuQosMib();*/
    	init_CtOnuQosMib();
    	init_c_interfaces();
    	init_onuEthVlanMib();
    	init_onuIgmpMib();
    	init_eponCtcOnuMib();
    	setOnuMibTree();

    	if (V2R1_CTC_STACK)
    	{
    		CT_RMan_Init(); 
#ifdef __CTC_TEST
    		init_voipPortGroup();
#endif
    		/*init_CtcAlarmMib();*/		/* removed by xieshl 20110907, 原MIB已废弃 */
        	/*initOperateFunctions();*/
    	}

#ifdef STATISTIC_TASK_MODULE
        /* 主控板，才统计Eth端口 */
    	StatsInit();	
#endif
    }
#ifdef STATISTIC_TASK_MODULE
	monInit();
#if( EPON_MODULE_ONU_LOOP == EPON_MODULE_YES )
	OnuLoopInit();
#endif
#endif

    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
    {
        /* 只有PON卡管理者，才有OAM应用 */
    	CommOamMagInfoInit();
    	vMnOamInit();/*added by suipl 2006-09-19 for OAM 封装*/
    	/*eventProcInit();*/ 	/* removed by xieshl 20070712 */
    	initAlarmOam();
#ifdef UPDATE_ONU_IMAGE_BY_OAM
    	funOamTransFileInit();
#endif 
    }		

	funIgmpAuthInit();
	
	V2r1_Init();
	V2r1_CreateTask();
	#if 0
	 /*modified for 16GPON by @muqw2015-09-25*/ 
	if(SYS_LOCAL_MODULE_TYPE_IS_8000_GPON)
	#endif
	{
   		pon_init(); 
	}
	
#if ( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
    cmc_init();
#endif

	EPON_V2R1_CliInit();

	ONU_View_CommandInstall();		/* added by xieshl 20111101, 需求9283 */
	InitOnuExtMgmtTable( );
	
	/* modified by xieshl 20110829, 调整模块初始化和命令挂接的顺序 */
#if( EPON_MODULE_PON_OPTICAL_POWER == EPON_MODULE_YES )
	PonPowerMeteringInit();
#endif

	  ethLoopCheckInit();  /*  为了是设备管理跑起来，而注释掉的。140515  mengxsheng  sheng */ 
    /*if( !SYS_LOCAL_MODULE_WORKMODE_ISMASTER )*/	/* 问题单11878 */ 
        onuOamUpdateInit();
#if( EPON_MODULE_ONU_AUTO_LOAD == EPON_MODULE_YES )
	onuAutoLoadInit();
#endif

	v2r1Version();

	OltPtyInit();
	cl_pty_relay_init();

	cl_cdp_pty_client_init();

	sys_console_printf("\r\n");
	VOS_TaskDelay( 2);

/* modified by xieshl 20081231 */
#ifdef V2R1_WATCH_DOG
	V2R1_init_watchdog();
#endif

#if( EPON_MODULE_USER_TRACE == EPON_MODULE_YES )
	trace_path_init();
#endif

	SYS_INIT_COMP = TRUE;
	return 0;
	/*SearchAndInsertPonCard();	*/	
}

/*
xflash_file_read(5,0x200004,0x200000)

start : 0xfe800000
*/

/* added by xieshl 20070705 */
/* modified by xieshl 20070712，解决主备倒换问题 */
extern int ethRcvFromMII(void *rcvBuffP , unsigned int packetSize);

#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
extern int bcmRecPacketPreHandle(char *buf,unsigned int len,int *port, unsigned short  *vlan);
extern int bcmSendPacketPreHandler(char *pBuf,unsigned int len, int port ,unsigned short vlan, char pri);
extern int bcmSendPacketPreHandler2(char *pBuf,unsigned int len);
extern int bcmRecPacketDelTag(char *buf,unsigned int len);

extern void iInit(void (*p_callback)(void * , unsigned int), void* pRecv, void* pSend);
#else
extern void iInit(void (*p_callback)(void * , unsigned int));
#endif
extern void iDeInit(void);

#if 0 /**(RPU_MODULE_HOT ==  RPU_YES )*/
extern VOID gfasw_switchover_rpc_callback( ULONG ulSrcNode, ULONG ulSrcModuleID,
                             VOID * pReceiveData, ULONG ulReceiveDataLen,
                             VOID **ppSendData, ULONG * pulSendDataLen );
VOID sw_switchover_init()
{
	/*if( bms_swBoard_number_get() != 0 )*/ /* 如果只有一个主控SW，则没有SW板间通信，也不会有主备倒换 */
	{
		if ( CDP_SYNC_Register( MODULE_RPU_EPON, gfasw_switchover_rpc_callback ) == VOS_ERROR )
		{
			ASSERT( 0 );
		}
	}
}
#endif

#if (RPU_MODULE_HOT ==  RPU_YES )
extern STATUS dataSyncInit();
#endif
extern LONG OLT_CommandInstall_for_standby();
extern VOID bms_board_cpld_io_set();

static int __cpu_netcomm_init_flag = 1;
LONG epon_v2r1_base_init()
{
	bms_board_cpld_io_set();

	if( __cpu_netcomm_init_flag )
	{
        /* 20150302 zhangxinhui moved */
#if 0
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
        if ( MODULE_E_GFA6900_SW == SYS_LOCAL_MODULE_TYPE  )
        {
        	iInit((void *)ethRcvFromMII, bcmRecPacketPreHandle, bcmSendPacketPreHandler);
        }
        else
        {
        	iInit((void *)ethRcvFromMII, NULL, NULL);
			
			if ( MODULE_E_GFA8000_SW == SYS_LOCAL_MODULE_TYPE  )
				iEmac1SpeedSet(100);  /* add by mengxsh 20140617   暂时放在这里以后会移到bspBoardInit 函数中 sheng	*/
        }
#else
    	iInit((void *)ethRcvFromMII);
#endif
#endif
		VOS_TaskDelay(2);

#ifdef  V2R1_SWITCH_5325E
		/*BrdChannelBcm5325eInit();*/
		BCM5325E_DEBUG_CommandInstall();
#endif

		/* wangysh add 启动点灯任务*/
		led_init();

		eventProcInit(); 
		OLT_CommandInstall_for_standby();

#if (RPU_MODULE_HOT ==  RPU_YES )
		/*sw_switchover_init();*/	/* removed by xieshl 20071102 */
        if ( SYS_IS_DISTRIBUTED )
		    dataSyncInit();
/* wangysh 20101228把下面的变量赋值统一到v2r1Entry()*/
#if 0
	/* added by xieshl 20090428, 下列值为GFA6700主备倒换和热拔插测试出的值，6100采用默认值即可 */
	if( ( SYS_PRODUCT_TYPE ==  PRODUCT_E_EPON3 )||
	    ( SYS_PRODUCT_TYPE == PRODUCT_E_GFA6900 ))
	{
		devsm_check_running_state_reset_period = 1500;				/* default 500 */
		devsm_check_hot_insert_pull_period = /*120*/300;					/* default 60 */
	}
#endif
#endif
		__cpu_netcomm_init_flag = 0;
	}
	return VOS_OK;
}
/* end 20070705 */

/* B--added by liwei056@2012-3-8 for Product's RunInit */
LONG epon_v2r1_run_init()
{   
    /* B--added by liwei056@2012-3-8 for Uplink's PonMonitor */
    if ( IFM_DevChainRegisterApi( Pon_Monitor_Eth, 0 ) != VOS_OK )
    {
        sys_console_printf("Olt Monitor Eth-link Failed!\r\n");
    }
	return VOS_OK;
    /* E--added by liwei056@2012-3-8 for Uplink's PonMonitor */
}
/* E--added by liwei056@2012-3-8 for Product's RunInit */


/* added by chenfj 2008-4-18
    重新启动交换板的管理通道5325E
    */

void ReInitCtrlChannel(void)
{
	LONG lLockKey;
	unsigned int counter = 100;
	/* added by xieshl 20081230, emac1驱动修改后，重新初始化时需要占用较长时间，不关闭看门狗使能会
	导致复位*/
	V2R1_disable_watchdog();
	VOS_TaskLock();
	lLockKey = VOS_IntLock();
	iDeInit();
	while(counter > 1 ) counter--;
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
    if ( MODULE_E_GFA6900_SW == SYS_LOCAL_MODULE_TYPE )
    {
    	iInit((void *)ethRcvFromMII, bcmRecPacketPreHandle, bcmSendPacketPreHandler);
    }
    else
    {
    	iInit((void *)ethRcvFromMII, NULL, NULL);
    }
#else
	iInit((void *)ethRcvFromMII);
#endif
	while(counter <= 100 ) counter++;
	/*RestartPonPortAll();*/
	VOS_IntUnlock(lLockKey);
	VOS_TaskUnlock();
	V2R1_enable_watchdog();
}

void RestartCtrlchannel()
{
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
    if ( MODULE_E_GFA6900_SW == SYS_LOCAL_MODULE_TYPE )
    {
    	iInit((void *)ethRcvFromMII, bcmRecPacketPreHandle, bcmSendPacketPreHandler);
    }
    else
    {
    	iInit((void *)ethRcvFromMII, NULL, NULL);
    }
#else
	iInit((void *)ethRcvFromMII);
#endif
}

#ifdef __cplusplus

}
#endif

